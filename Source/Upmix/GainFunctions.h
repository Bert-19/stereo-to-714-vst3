#pragma once

#include <JuceHeader.h>
#include "UpmixConfig.h"
#include <cmath>

namespace upmix
{

inline float wrapAngleDeg (float degrees)
{
    while (degrees > 180.0f)  degrees -= 360.0f;
    while (degrees < -180.0f) degrees += 360.0f;
    return degrees;
}

inline float smoothstep (float edge0, float edge1, float x)
{
    const float t = juce::jlimit (0.0f, 1.0f, (x - edge0) / (edge1 - edge0 + 1.0e-6f));
    return t * t * (3.0f - 2.0f * t);
}

inline float angularGaussian (float thetaDeg, float centerDeg, float widthDeg)
{
    const float d = wrapAngleDeg (thetaDeg - centerDeg);
    const float w = juce::jmax (widthDeg, 1.0e-3f);
    return std::exp (-0.5f * (d / w) * (d / w));
}

inline float directWeight (float psi, float azimuthDeg, float speakerAzimuthDeg, float widthDeg)
{
    return (1.0f - psi) * angularGaussian (azimuthDeg, speakerAzimuthDeg, widthDeg);
}

inline float centerWeight (float psi, float azimuthDeg, CenterMode mode)
{
    const float frontFocus = angularGaussian (azimuthDeg, 0.0f, mode == CenterMode::movie ? 18.0f : 32.0f);
    const float pointSource = 1.0f - psi;
    const float modeGain = mode == CenterMode::movie ? 1.0f : 0.45f;
    return frontFocus * pointSource * modeGain;
}

inline float skyGate (float psi, HeightMode mode)
{
    const float threshold = mode == HeightMode::conservative ? 0.45f : 0.25f;
    return smoothstep (threshold, 1.0f, psi);
}

inline float skyDirectionGain (float azimuthDeg, float speakerAzimuthDeg)
{
    return angularGaussian (azimuthDeg, speakerAzimuthDeg, 55.0f);
}

inline float highFrequencyWeight (float frequencyHz, float sampleRate)
{
    const float hfPivot = 4500.0f;
    const float norm = frequencyHz / juce::jmax (sampleRate * 0.5f, 1.0f);
    const float pivot = hfPivot / juce::jmax (sampleRate * 0.5f, 1.0f);
    return smoothstep (pivot * 0.5f, pivot * 1.5f, norm);
}

} // namespace upmix
