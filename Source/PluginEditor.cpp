#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
void styleLabel (juce::Label& label)
{
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
}

void styleSlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff4fc3f7));
    slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff4fc3f7));
}
} // namespace

StereoTo714AudioProcessorEditor::StereoTo714AudioProcessorEditor (StereoTo714AudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    titleLabel.setFont (juce::FontOptions (22.0f).withStyle ("Bold"));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    for (auto* label : { &heightModeLabel, &centerModeLabel, &widthLabel,
                         &heightAmountLabel, &surroundMixLabel, &lfeLevelLabel })
    {
        styleLabel (*label);
        addAndMakeVisible (*label);
    }

    heightModeBox.addItemList ({ "Conservative", "Aggressive" }, 1);
    centerModeBox.addItemList ({ "Movie", "Music" }, 1);
    addAndMakeVisible (heightModeBox);
    addAndMakeVisible (centerModeBox);

    for (auto* slider : { &widthSlider, &heightAmountSlider, &surroundMixSlider, &lfeLevelSlider })
    {
        styleSlider (*slider);
        addAndMakeVisible (*slider);
    }

    bypassButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (bypassButton);

    auto& apvts = processorRef.getApvts();

    heightModeAttachment = std::make_unique<ComboAttachment> (apvts, "heightMode", heightModeBox);
    centerModeAttachment = std::make_unique<ComboAttachment> (apvts, "centerMode", centerModeBox);
    widthAttachment = std::make_unique<SliderAttachment> (apvts, "width", widthSlider);
    heightAmountAttachment = std::make_unique<SliderAttachment> (apvts, "heightAmount", heightAmountSlider);
    surroundMixAttachment = std::make_unique<SliderAttachment> (apvts, "surroundMix", surroundMixSlider);
    lfeLevelAttachment = std::make_unique<SliderAttachment> (apvts, "lfeLevel", lfeLevelSlider);
    bypassAttachment = std::make_unique<ButtonAttachment> (apvts, "bypass", bypassButton);

    setSize (560, 360);
}

StereoTo714AudioProcessorEditor::~StereoTo714AudioProcessorEditor() = default;

void StereoTo714AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a2e));

    g.setColour (juce::Colour (0xff16213e));
    g.fillRoundedRectangle (getLocalBounds().reduced (12).toFloat(), 10.0f);
}

void StereoTo714AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);
    titleLabel.setBounds (bounds.removeFromTop (34));

    auto modeRow = bounds.removeFromTop (56);
    heightModeLabel.setBounds (modeRow.removeFromLeft (80));
    heightModeBox.setBounds (modeRow.removeFromLeft (150).reduced (4));
    modeRow.removeFromLeft (12);
    centerModeLabel.setBounds (modeRow.removeFromLeft (80));
    centerModeBox.setBounds (modeRow.removeFromLeft (150).reduced (4));

    bounds.removeFromTop (8);
    bypassButton.setBounds (bounds.removeFromTop (28).removeFromRight (120));

    auto sliderRow = bounds.removeFromTop (180);
    const int sliderWidth = sliderRow.getWidth() / 4;

    auto placeSlider = [] (juce::Rectangle<int> area, juce::Label& label, juce::Slider& slider)
    {
        auto block = area.reduced (6);
        label.setBounds (block.removeFromTop (20));
        slider.setBounds (block);
    };

    placeSlider (sliderRow.removeFromLeft (sliderWidth), widthLabel, widthSlider);
    placeSlider (sliderRow.removeFromLeft (sliderWidth), heightAmountLabel, heightAmountSlider);
    placeSlider (sliderRow.removeFromLeft (sliderWidth), surroundMixLabel, surroundMixSlider);
    placeSlider (sliderRow, lfeLevelLabel, lfeLevelSlider);
}
