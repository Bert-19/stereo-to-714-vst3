#include "../Upmix/UpmixEngine.h"

#include <cmath>
#include <iostream>
#include <string>

namespace
{

constexpr int kBlockSize = 64;
constexpr double kSampleRate = 48000.0;

bool nearlyEqual (float actual, float expected, float tolerance = 1.0e-6f)
{
    return std::abs (actual - expected) <= tolerance;
}

float processBlock (upmix::UpmixEngine& engine,
                    const juce::AudioBuffer<float>& input,
                    juce::AudioBuffer<float>& output)
{
    output.clear();
    engine.process (input, output);

    float energy = 0.0f;
    for (int ch = 0; ch < output.getNumChannels(); ++ch)
        for (int sample = 0; sample < output.getNumSamples(); ++sample)
            energy += std::abs (output.getSample (ch, sample));

    return energy;
}

bool expect (bool condition, const std::string& message)
{
    if (! condition)
        std::cerr << "FAIL: " << message << '\n';

    return condition;
}

bool bypassOutputsLatencyCompensatedDryStereo()
{
    upmix::UpmixEngine engine;
    engine.prepare (kSampleRate, kBlockSize);

    upmix::UpmixParams params;
    params.bypass = true;
    engine.setParams (params);

    juce::AudioBuffer<float> input (2, kBlockSize);
    juce::AudioBuffer<float> output (upmix::kNum714Channels, kBlockSize);

    const auto latency = engine.getLatencySamples();
    const auto totalSamples = latency + kBlockSize;
    bool passed = true;

    for (int offset = 0; offset < totalSamples; offset += kBlockSize)
    {
        input.clear();

        if (offset == 0)
        {
            input.setSample (0, 0, 1.0f);
            input.setSample (1, 0, -0.5f);
        }

        processBlock (engine, input, output);

        for (int sample = 0; sample < kBlockSize; ++sample)
        {
            const auto globalSample = offset + sample;
            const auto expectedL = globalSample == latency ? 1.0f : 0.0f;
            const auto expectedR = globalSample == latency ? -0.5f : 0.0f;

            passed &= expect (nearlyEqual (output.getSample (0, sample), expectedL),
                              "bypass left channel is not delayed dry audio");
            passed &= expect (nearlyEqual (output.getSample (1, sample), expectedR),
                              "bypass right channel is not delayed dry audio");

            for (int ch = 2; ch < upmix::kNum714Channels; ++ch)
                passed &= expect (nearlyEqual (output.getSample (ch, sample), 0.0f),
                                  "bypass emitted non-stereo output");
        }
    }

    return passed;
}

bool bypassConsumesStaleProcessedAudioBeforeReEnable()
{
    upmix::UpmixEngine engine;
    engine.prepare (kSampleRate, kBlockSize);

    upmix::UpmixParams params;
    engine.setParams (params);

    juce::AudioBuffer<float> input (2, kBlockSize);
    juce::AudioBuffer<float> output (upmix::kNum714Channels, kBlockSize);
    float seededEnergy = 0.0f;

    for (int block = 0; block < 96; ++block)
    {
        for (int sample = 0; sample < kBlockSize; ++sample)
        {
            const auto value = std::sin (static_cast<float> (block * kBlockSize + sample) * 0.05f);
            input.setSample (0, sample, value);
            input.setSample (1, sample, -value * 0.75f);
        }

        seededEnergy += processBlock (engine, input, output);
    }

    bool passed = expect (seededEnergy > 1.0f, "test did not seed processed engine output");

    params.bypass = true;
    engine.setParams (params);
    input.clear();

    for (int block = 0; block < 256; ++block)
        processBlock (engine, input, output);

    params.bypass = false;
    engine.setParams (params);

    const auto energyAfterReEnable = processBlock (engine, input, output);
    passed &= expect (energyAfterReEnable < 1.0e-4f,
                      "re-enabling after bypass emitted stale pre-bypass audio");

    return passed;
}

} // namespace

int main()
{
    bool passed = true;
    passed &= bypassOutputsLatencyCompensatedDryStereo();
    passed &= bypassConsumesStaleProcessedAudioBeforeReEnable();

    if (! passed)
        return 1;

    std::cout << "UpmixEngine tests passed\n";
    return 0;
}
