#include "Upmix/UpmixConfig.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace
{
bool expectIndex (juce::AudioChannelSet::ChannelType type, int expectedIndex)
{
    const auto layout = juce::AudioChannelSet::create7point1point4();
    return layout.getChannelIndexForType (type) == expectedIndex;
}
} // namespace

int main()
{
    static_assert (upmix::kNum714Channels == 12);

    if (! expectIndex (juce::AudioChannelSet::left, upmix::chL))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::right, upmix::chR))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::centre, upmix::chC))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::LFE, upmix::chLFE))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::leftSurroundSide, upmix::chLs))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::rightSurroundSide, upmix::chRs))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::topFrontLeft, upmix::chLtf))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::topFrontRight, upmix::chRtf))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::topRearLeft, upmix::chLtr))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::topRearRight, upmix::chRtr))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::leftSurroundRear, upmix::chLrs))
        return 1;
    if (! expectIndex (juce::AudioChannelSet::rightSurroundRear, upmix::chRrs))
        return 1;

    static_assert (upmix::kSpeakers[upmix::chLFE].azimuthDeg == 0.0f);
    static_assert (! upmix::kSpeakers[upmix::chLFE].isHeight);
    static_assert (upmix::kSpeakers[upmix::chLtf].isHeight);
    static_assert (upmix::kSpeakers[upmix::chLrs].azimuthDeg == +150.0f);
    static_assert (! upmix::kSpeakers[upmix::chLrs].isHeight);

    return 0;
}
