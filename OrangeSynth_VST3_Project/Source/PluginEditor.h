#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrangeSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OrangeSynthAudioProcessorEditor(OrangeSynthAudioProcessor&);
    ~OrangeSynthAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    OrangeSynthAudioProcessor& processor;
    juce::ComboBox presets;
    juce::TextButton prev{"<"}, next{">"}, save{"SAVE PRESET"};
    juce::Label brand, subtitle, presetLabel;
    struct Control { juce::Slider slider; juce::Label label; };
    std::vector<std::unique_ptr<Control>> controls;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
    void refreshPresets();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrangeSynthAudioProcessorEditor)
};
