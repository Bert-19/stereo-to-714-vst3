#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class StereoTo714AudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit StereoTo714AudioProcessorEditor (StereoTo714AudioProcessor&);
    ~StereoTo714AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    StereoTo714AudioProcessor& processorRef;

    juce::Label titleLabel { {}, "Stereo to 7.1.4" };
    juce::Label heightModeLabel { {}, "Height" };
    juce::Label centerModeLabel { {}, "Center" };
    juce::Label widthLabel { {}, "Width" };
    juce::Label heightAmountLabel { {}, "Height Amt" };
    juce::Label surroundMixLabel { {}, "Surround" };
    juce::Label lfeLevelLabel { {}, "LFE" };

    juce::ComboBox heightModeBox;
    juce::ComboBox centerModeBox;

    juce::Slider widthSlider;
    juce::Slider heightAmountSlider;
    juce::Slider surroundMixSlider;
    juce::Slider lfeLevelSlider;

    juce::ToggleButton bypassButton { "Bypass" };

    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ComboAttachment> heightModeAttachment;
    std::unique_ptr<ComboAttachment> centerModeAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;
    std::unique_ptr<SliderAttachment> heightAmountAttachment;
    std::unique_ptr<SliderAttachment> surroundMixAttachment;
    std::unique_ptr<SliderAttachment> lfeLevelAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StereoTo714AudioProcessorEditor)
};
