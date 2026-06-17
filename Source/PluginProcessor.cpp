#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr const char* kHeightMode = "heightMode";
constexpr const char* kCenterMode = "centerMode";
constexpr const char* kWidth = "width";
constexpr const char* kHeightAmount = "heightAmount";
constexpr const char* kSurroundMix = "surroundMix";
constexpr const char* kLfeLevel = "lfeLevel";
constexpr const char* kBypass = "bypass";
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout StereoTo714AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        kHeightMode,
        "Height Mode",
        juce::StringArray { "Conservative", "Aggressive" },
        0));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        kCenterMode,
        "Center Mode",
        juce::StringArray { "Movie", "Music" },
        0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kWidth, 1 },
        "Width",
        juce::NormalisableRange<float> { 0.0f, 2.0f, 0.01f },
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kHeightAmount, 1 },
        "Height Amount",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.01f },
        0.65f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kSurroundMix, 1 },
        "Surround Mix",
        juce::NormalisableRange<float> { 0.0f, 1.5f, 0.01f },
        1.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { kLfeLevel, 1 },
        "LFE Level",
        juce::NormalisableRange<float> { 0.0f, 1.5f, 0.01f },
        0.707f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        kBypass,
        "Bypass",
        false));

    return layout;
}

juce::AudioChannelSet StereoTo714AudioProcessor::get714OutputLayout()
{
    return juce::AudioChannelSet::create7point1point4();
}

StereoTo714AudioProcessor::StereoTo714AudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", get714OutputLayout(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

StereoTo714AudioProcessor::~StereoTo714AudioProcessor() = default;

const juce::String StereoTo714AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StereoTo714AudioProcessor::acceptsMidi() const
{
    return false;
}

bool StereoTo714AudioProcessor::producesMidi() const
{
    return false;
}

bool StereoTo714AudioProcessor::isMidiEffect() const
{
    return false;
}

double StereoTo714AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StereoTo714AudioProcessor::getNumPrograms()
{
    return 1;
}

int StereoTo714AudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoTo714AudioProcessor::setCurrentProgram (int) {}

const juce::String StereoTo714AudioProcessor::getProgramName (int)
{
    return {};
}

void StereoTo714AudioProcessor::changeProgramName (int, const juce::String&) {}

upmix::UpmixParams StereoTo714AudioProcessor::readParams() const
{
    upmix::UpmixParams p;

    p.heightMode = *apvts.getRawParameterValue (kHeightMode) < 0.5f
                       ? upmix::HeightMode::conservative
                       : upmix::HeightMode::aggressive;

    p.centerMode = *apvts.getRawParameterValue (kCenterMode) < 0.5f
                       ? upmix::CenterMode::movie
                       : upmix::CenterMode::music;

    p.width = *apvts.getRawParameterValue (kWidth);
    p.heightAmount = *apvts.getRawParameterValue (kHeightAmount);
    p.surroundMix = *apvts.getRawParameterValue (kSurroundMix);
    p.lfeLevel = *apvts.getRawParameterValue (kLfeLevel);
    p.bypass = *apvts.getRawParameterValue (kBypass) >= 0.5f;

    return p;
}

void StereoTo714AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    upmixEngine.prepare (sampleRate, samplesPerBlock);
    prepared = true;
    setLatencySamples (upmixEngine.getLatencySamples());
}

void StereoTo714AudioProcessor::releaseResources()
{
    upmixEngine.reset();
    prepared = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StereoTo714AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != get714OutputLayout())
        return false;

    return true;
}
#endif

void StereoTo714AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused (midiMessages);

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    if (! prepared || totalNumInputChannels < 2 || totalNumOutputChannels < upmix::kNum714Channels)
    {
        buffer.clear();
        return;
    }

    juce::AudioBuffer<float> input (2, buffer.getNumSamples());
    input.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
    input.copyFrom (1, 0, buffer, 1, 0, buffer.getNumSamples());

    upmixEngine.setParams (readParams());
    upmixEngine.process (input, buffer);
}

bool StereoTo714AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* StereoTo714AudioProcessor::createEditor()
{
    return new StereoTo714AudioProcessorEditor (*this);
}

void StereoTo714AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void StereoTo714AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoTo714AudioProcessor();
}
