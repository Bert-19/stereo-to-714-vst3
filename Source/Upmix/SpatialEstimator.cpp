#include "SpatialEstimator.h"
#include <JuceHeader.h>
#include <cmath>

namespace upmix
{

float SpatialEstimator::binFrequency (int binIndex, double sr)
{
    return static_cast<float> (binIndex * sr / static_cast<double> (kFftSize));
}

void SpatialEstimator::prepare (double newSampleRate)
{
    sampleRate = newSampleRate;

    ildToTheta.resize (static_cast<size_t> (kNumBins));
    ipdToTheta.resize (static_cast<size_t> (kNumBins));
    ildWeight.resize (static_cast<size_t> (kNumBins));
    ipdWeight.resize (static_cast<size_t> (kNumBins));

    for (int k = 0; k < kNumBins; ++k)
    {
        const float freq = binFrequency (k, sampleRate);

        ildToTheta[static_cast<size_t> (k)] = 12.0f;
        ipdToTheta[static_cast<size_t> (k)] = (freq < 1500.0f ? 55.0f : 85.0f) / juce::MathConstants<float>::pi;

        const float lowBand = juce::jlimit (0.0f, 1.0f, (800.0f - freq) / 800.0f);
        ildWeight[static_cast<size_t> (k)] = lowBand;
        ipdWeight[static_cast<size_t> (k)] = 1.0f - lowBand;
    }
}

void SpatialEstimator::reset() {}

void SpatialEstimator::analyseBin (int binIndex,
                                   std::complex<float> xl,
                                   std::complex<float> xr,
                                   float& thetaDeg,
                                   float& psi,
                                   float& gamma) const
{
    const float pl = std::norm (xl);
    const float pr = std::norm (xr);
    const auto cross = xl * std::conj (xr);

    const float denom = std::sqrt (pl * pr) + 1.0e-8f;
    gamma = juce::jlimit (0.0f, 1.0f, std::abs (cross) / denom);
    psi = 1.0f - gamma;

    const float ild = 10.0f * std::log10 ((pl + 1.0e-8f) / (pr + 1.0e-8f));
    const float ipd = std::atan2 (cross.imag(), cross.real());

    const auto idx = static_cast<size_t> (binIndex);
    const float thetaIld = juce::jlimit (-kThetaMaxDeg, kThetaMaxDeg, ild * ildToTheta[idx]);
    const float thetaIpd = juce::jlimit (-kThetaMaxDeg, kThetaMaxDeg, ipd * ipdToTheta[idx]);
    thetaDeg = ildWeight[idx] * thetaIld + ipdWeight[idx] * thetaIpd;
}

} // namespace upmix
