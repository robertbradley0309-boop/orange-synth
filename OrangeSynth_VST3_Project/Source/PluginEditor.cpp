#include "PluginEditor.h"

OrangeSynthAudioProcessorEditor::OrangeSynthAudioProcessorEditor(OrangeSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 500);

    brand.setText("ORANGE", juce::dontSendNotification);
    brand.setFont(juce::Font(32.0f, juce::Font::bold));
    brand.setColour(juce::Label::textColourId, juce::Colour(0xfff07824));
    addAndMakeVisible(brand);

    presetBox.setTextWhenNothingSelected("Select Preset");
    addAndMakeVisible(presetBox);

    previous.setButtonText("<");
    next.setButtonText(">");
    save.setButtonText("SAVE");

    addAndMakeVisible(previous);
    addAndMakeVisible(next);
    addAndMakeVisible(save);

    refreshPresets();

    presetBox.onChange = [this]
    {
        auto index = presetBox.getSelectedItemIndex();
        if (index >= 0)
            processor.loadPreset(presetBox.getItemText(index));
    };

    previous.onClick = [this]
    {
        auto index = presetBox.getSelectedItemIndex();
        if (index > 0)
            presetBox.setSelectedItemIndex(index - 1);
    };

    next.onClick = [this]
    {
        auto index = presetBox.getSelectedItemIndex();
        if (index < presetBox.getNumItems() - 1)
            presetBox.setSelectedItemIndex(index + 1);
    };

    // Preset saving button temporarily disabled to avoid unsupported modal dialog.
    save.onClick = [] {};

    const char* ids[] = {
        "wave", "oscMix", "sub", "detune", "cutoff",
        "resonance", "attack", "decay", "sustain", "release",
        "drive", "reverb", "lfoRate", "lfoDepth", "level"
    };

    const char* names[] = {
        "WAVE", "OSC MIX", "SUB", "DETUNE", "CUTOFF",
        "RESONANCE", "ATTACK", "DECAY", "SUSTAIN", "RELEASE",
        "DRIVE", "REVERB", "LFO RATE", "LFO DEPTH", "LEVEL"
    };

    for (int i = 0; i < 15; ++i)
    {
        auto c = std::make_unique<Control>();

        c->label.setText(names[i], juce::dontSendNotification);
        c->label.setJustificationType(juce::Justification::centred);
        c->label.setColour(juce::Label::textColourId, juce::Colour(0xff555555));
        addAndMakeVisible(c->label);

        c->slider.setSliderStyle(juce::Slider::Rotary);
        c->slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 18);
        c->slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff07824));
        c->slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff07824));
        c->slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff333333));

        addAndMakeVisible(c->slider);

        attachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                processor.parameters, ids[i], c->slider));

        controls.push_back(std::move(c));
    }
}

void OrangeSynthAudioProcessorEditor::refreshPresets()
{
    presetBox.clear();

    auto presets = processor.getPresetNames();

    for (int i = 0; i < presets.size(); ++i)
        presetBox.addItem(presets[i], i + 1);

    if (presetBox.getNumItems() > 0)
        presetBox.setSelectedItemIndex(0);
}

void OrangeSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xfffafafa));

    g.setColour(juce::Colour(0xfff07824));
    g.drawRect(getLocalBounds(), 3);

    g.setColour(juce::Colour(0xffeeeeee));
    g.drawLine(20.0f, 85.0f, 740.0f, 85.0f, 2.0f);
}

void OrangeSynthAudioProcessorEditor::resized()
{
    brand.setBounds(25, 15, 220, 55);

    presetBox.setBounds(270, 25, 220, 32);
    previous.setBounds(500, 25, 35, 32);
    next.setBounds(540, 25, 35, 32);
    save.setBounds(625, 25, 100, 32);

    const int startX = 20;
    const int startY = 110;
    const int width = 144;
    const int height = 180;

    for (int i = 0; i < controls.size(); ++i)
    {
        int row = i / 5;
        int col = i % 5;

        int x = startX + col * width;
        int y = startY + row * height;

        controls[i]->label.setBounds(x, y, 130, 25);
        controls[i]->slider.setBounds(x, y + 25, 130, 140);
    }
}
