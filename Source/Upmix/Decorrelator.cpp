#include "Decorrelator.h"
#include <JuceHeader.h>
#include <cmath>

namespace upmix
{

void Decorrelator::prepare (double sampleRate)
{
    static constexpr std::array<float, 8> surroundDelayMs { 7.0f, 11.0f, 15.0f, 19.0f, 23.0f, 27.0f, 31.0f, 35.0f };
    static constexpr std::array<int, 8> surroundChannels { chLs, chRs, chLrs, chRrs, chLtf, chRtf, chLtr, chRtr };

    for (auto& phases : binPhase)
        phases.assign (static_cast<size_t> (kNumBins), 0.0f);

    for (size_t i = 0; i < surroundChannels.size(); ++i)
    {
        const int ch = surroundChannels[i];
        const float delaySamples = static_cast<float> (surroundDelayMs[i] * 0.001 * sampleRate);

        for (int k = 0; k < kNumBins; ++k)
        {
            const float phase = -2.0f * juce::MathConstants<float>::pi * static_cast<float> (k) * delaySamples
                                / static_cast<float> (kFftSize);
            binPhase[static_cast<size_t> (ch)][static_cast<size_t> (k)] = phase;
        }
    }
}

void Decorrelator::reset() {}

std::complex<float> Decorrelator::processBin (int channelIndex, int binIndex, std::complex<float> value) const
{
    const auto& phases = binPhase[static_cast<size_t> (channelIndex)];
    const float phase = phases[static_cast<size_t> (binIndex)];
    return value * std::polar (1.0f, phase);
}

} // namespace upmix
