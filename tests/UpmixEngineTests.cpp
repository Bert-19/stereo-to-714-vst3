#include <JuceHeader.h>
#include "Upmix/UpmixEngine.h"

#include <cmath>
#include <iostream>
#include <string>

namespace
{
bool nearlyEqual (float actual, float expected)
{
    return std::abs (actual - expected) <= 1.0e-6f;
}

bool expectSample (const std::string& label, int sampleIndex, float actual, float expected)
{
    if (nearlyEqual (actual, expected))
        return true;

    std::cerr << label << " sample " << sampleIndex << ": expected " << expected
              << ", got " << actual << '\n';
    return false;
}

bool bypassOutputMatchesReportedLatency()
{
    upmix::UpmixEngine engine;
    engine.prepare (48000.0, 64);

    upmix::UpmixParams params;
    params.bypass = true;
    engine.setParams (params);

    const auto latency = engine.getLatencySamples();
    const auto totalSamples = latency + 16;
    const auto blockSize = 64;

    std::vector<float> leftOut (static_cast<size_t> (totalSamples), 0.0f);
    std::vector<float> rightOut (static_cast<size_t> (totalSamples), 0.0f);

    for (int offset = 0; offset < totalSamples; offset += blockSize)
    {
        const auto numSamples = std::min (blockSize, totalSamples - offset);
        juce::AudioBuffer<float> input (2, numSamples);
        juce::AudioBuffer<float> output (upmix::kNum714Channels, numSamples);

        input.clear();
        output.clear();

        if (offset == 0)
        {
            input.setSample (0, 0, 1.0f);
            input.setSample (1, 0, -0.5f);
        }

        engine.process (input, output);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto globalSample = offset + sample;
            leftOut[static_cast<size_t> (globalSample)] = output.getSample (0, sample);
            rightOut[static_cast<size_t> (globalSample)] = output.getSample (1, sample);

            for (int ch = 2; ch < upmix::kNum714Channels; ++ch)
            {
                if (! expectSample ("bypass non-stereo channel " + std::to_string (ch),
                                    globalSample,
                                    output.getSample (ch, sample),
                                    0.0f))
                {
                    return false;
                }
            }
        }
    }

    for (int sample = 0; sample < totalSamples; ++sample)
    {
        const auto expectedLeft = sample == latency ? 1.0f : 0.0f;
        const auto expectedRight = sample == latency ? -0.5f : 0.0f;

        if (! expectSample ("bypass left", sample, leftOut[static_cast<size_t> (sample)], expectedLeft))
            return false;

        if (! expectSample ("bypass right", sample, rightOut[static_cast<size_t> (sample)], expectedRight))
            return false;
    }

    return true;
}
} // namespace

int main()
{
    if (! bypassOutputMatchesReportedLatency())
        return 1;

    return 0;
}
