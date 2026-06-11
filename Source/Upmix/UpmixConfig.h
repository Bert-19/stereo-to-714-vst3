#pragma once

#include <array>
#include <cmath>

namespace upmix
{

inline constexpr int kFftOrder = 11;
inline constexpr int kFftSize = 1 << kFftOrder;
inline constexpr int kHopSize = 512;
inline constexpr int kNumBins = kFftSize / 2 + 1;
inline constexpr int kNum714Channels = 12;
inline constexpr float kLfeCutoffHz = 120.0f;
inline constexpr float kThetaMaxDeg = 150.0f;

enum Channel714 : int
{
    chL = 0,
    chR,
    chC,
    chLFE,
    chLs,
    chRs,
    chLrs,
    chRrs,
    chLtf,
    chRtf,
    chLtr,
    chRtr
};

enum class HeightMode { conservative = 0, aggressive };
enum class CenterMode { movie = 0, music };

struct SpeakerLayout
{
    float azimuthDeg;
    bool isHeight;
};

inline constexpr std::array<SpeakerLayout, kNum714Channels> kSpeakers { {
    { +30.0f, false },
    { -30.0f, false },
    {   0.0f, false },
    {   0.0f, false },
    { +110.0f, false },
    { -110.0f, false },
    { +150.0f, false },
    { -150.0f, false },
    { +45.0f, true },
    { -45.0f, true },
    { +135.0f, true },
    { -135.0f, true },
} };

struct UpmixParams
{
    HeightMode heightMode = HeightMode::conservative;
    CenterMode centerMode = CenterMode::movie;
    float width = 1.0f;
    float heightAmount = 0.65f;
    float surroundMix = 1.0f;
    float lfeLevel = 0.707f;
    bool bypass = false;
};

} // namespace upmix
