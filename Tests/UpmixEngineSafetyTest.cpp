#include <JuceHeader.h>
#include "Upmix/UpmixEngine.h"

#include <cmath>
#include <iostream>

namespace
{
bool expect (bool condition, const char* message)
{
    if (! condition)
        std::cerr << "FAILED: " << message << '\n';

    return condition;
}

bool bufferIsSilent (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (std::abs (buffer.getSample (ch, sample)) > 1.0e-7f)
                return false;

    return true;
}

void fillBuffer (juce::AudioBuffer<float>& buffer, float value)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample (ch, sample, value);
}

bool unpreparedProcessClearsOutput()
{
    upmix::UpmixEngine engine;
    juce::AudioBuffer<float> input (2, 64);
    juce::AudioBuffer<float> output (upmix::kNum714Channels, 64);

    fillBuffer (input, 0.25f);
    fillBuffer (output, 1.0f);

    engine.process (input, output);

    return expect (bufferIsSilent (output), "unprepared process should clear output");
}

bool monoInputClearsOutput()
{
    upmix::UpmixEngine engine;
    engine.prepare (48000.0, 64);

    juce::AudioBuffer<float> input (1, 64);
    juce::AudioBuffer<float> output (upmix::kNum714Channels, 64);

    fillBuffer (input, 0.25f);
    fillBuffer (output, 1.0f);

    engine.process (input, output);

    return expect (bufferIsSilent (output), "mono input should clear output");
}

bool preparedBypassCopiesStereo()
{
    upmix::UpmixEngine engine;
    engine.prepare (48000.0, 64);

    upmix::UpmixParams params;
    params.bypass = true;
    engine.setParams (params);

    juce::AudioBuffer<float> input (2, 64);
    juce::AudioBuffer<float> output (upmix::kNum714Channels, 64);

    for (int sample = 0; sample < input.getNumSamples(); ++sample)
    {
        input.setSample (0, sample, 0.1f * static_cast<float> (sample));
        input.setSample (1, sample, -0.05f * static_cast<float> (sample));
    }

    fillBuffer (output, 1.0f);
    engine.process (input, output);

    for (int sample = 0; sample < input.getNumSamples(); ++sample)
    {
        if (! expect (std::abs (output.getSample (0, sample) - input.getSample (0, sample)) <= 1.0e-7f,
                      "bypass should copy left input")
            || ! expect (std::abs (output.getSample (1, sample) - input.getSample (1, sample)) <= 1.0e-7f,
                         "bypass should copy right input"))
            return false;
    }

    for (int ch = 2; ch < output.getNumChannels(); ++ch)
        for (int sample = 0; sample < output.getNumSamples(); ++sample)
            if (! expect (std::abs (output.getSample (ch, sample)) <= 1.0e-7f,
                          "bypass should clear non-stereo outputs"))
                return false;

    return true;
}
} // namespace

int main()
{
    const bool ok = unpreparedProcessClearsOutput()
                    && monoInputClearsOutput()
                    && preparedBypassCopiesStereo();

    if (ok)
        std::cout << "All UpmixEngine safety tests passed.\n";

    return ok ? 0 : 1;
}
