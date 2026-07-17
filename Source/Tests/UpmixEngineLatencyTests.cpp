#include "../Upmix/UpmixEngine.h"

#include <cmath>
#include <iostream>
#include <string>

namespace
{

constexpr int kBlockSize = 64;
constexpr double kSampleRate = 48000.0;

bool expect (bool condition, const std::string& message)
{
    if (! condition)
        std::cerr << "FAIL: " << message << '\n';

    return condition;
}

void processImpulseBlock (upmix::UpmixEngine& engine,
                          int blockStart,
                          juce::AudioBuffer<float>& input,
                          juce::AudioBuffer<float>& output)
{
    input.clear();
    output.clear();

    if (blockStart == 0)
    {
        input.setSample (0, 0, 1.0f);
        input.setSample (1, 0, 1.0f);
    }

    engine.process (input, output);
}

bool processedImpulsePeakMatchesReportedLatency()
{
    upmix::UpmixEngine engine;
    engine.prepare (kSampleRate, kBlockSize);

    upmix::UpmixParams params;
    params.width = 0.0f;
    params.heightAmount = 0.0f;
    params.surroundMix = 0.0f;
    params.lfeLevel = 0.0f;
    engine.setParams (params);

    juce::AudioBuffer<float> input (2, kBlockSize);
    juce::AudioBuffer<float> output (upmix::kNum714Channels, kBlockSize);

    const auto expectedLatency = engine.getLatencySamples();
    const auto totalSamples = expectedLatency + upmix::kFftSize;
    int peakSample = -1;
    float peak = 0.0f;

    for (int blockStart = 0; blockStart < totalSamples; blockStart += kBlockSize)
    {
        processImpulseBlock (engine, blockStart, input, output);

        for (int sample = 0; sample < kBlockSize; ++sample)
        {
            const auto value = std::abs (output.getSample (upmix::chC, sample));
            if (value > peak)
            {
                peak = value;
                peakSample = blockStart + sample;
            }
        }
    }

    bool passed = expect (peak > 0.25f, "processed impulse did not produce a usable center-channel peak");
    passed &= expect (peakSample == expectedLatency,
                      "processed impulse peak did not match reported latency; peak="
                          + std::to_string (peakSample)
                          + " latency="
                          + std::to_string (expectedLatency));

    return passed;
}

} // namespace

int main()
{
    if (! processedImpulsePeakMatchesReportedLatency())
        return 1;

    std::cout << "UpmixEngine latency tests passed\n";
    return 0;
}
