#include <JuceHeader.h>
#include "../Source/Upmix/UpmixEngine.h"

#include <iostream>

namespace
{
bool bufferIsSilent (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            if (buffer.getSample (ch, sample) != 0.0f)
                return false;

    return true;
}
} // namespace

int main()
{
    juce::AudioBuffer<float> input (2, 32);
    input.clear();

    for (int sample = 0; sample < input.getNumSamples(); ++sample)
    {
        input.setSample (0, sample, 0.25f);
        input.setSample (1, sample, -0.25f);
    }

    juce::AudioBuffer<float> output (upmix::kNum714Channels, input.getNumSamples());
    output.clear();

    for (int ch = 0; ch < output.getNumChannels(); ++ch)
        for (int sample = 0; sample < output.getNumSamples(); ++sample)
            output.setSample (ch, sample, 1.0f);

    upmix::UpmixEngine engine;
    engine.process (input, output);

    if (! bufferIsSilent (output))
    {
        std::cerr << "Unprepared processing should return silence.\n";
        return 1;
    }

    engine.prepare (48000.0, input.getNumSamples());
    engine.process (input, output);

    std::cout << "UpmixEngine tests passed.\n";
    return 0;
}
