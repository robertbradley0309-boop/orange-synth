#include "PluginEditor.h"

OrangeSynthAudioProcessorEditor::OrangeSynthAudioProcessorEditor(OrangeSynthAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p)
{
    setSize(760,500);
    brand.setText("ORANGE",juce::dontSendNotification); brand.setFont(juce::Font(juce::FontOptions(34.0f,juce::Font::bold)));
    brand.setColour(juce::Label::textColourId,juce::Colour(0xfff07824)); addAndMakeVisible(brand);
    subtitle.setText("POLY SYNTHESIZER  /  FACTORY BANK 200",juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId,juce::Colour(0xff777777)); addAndMakeVisible(subtitle);
    presetLabel.setText("PRESET",juce::dontSendNotification); presetLabel.setColour(juce::Label::textColourId,juce::Colour(0xff555555)); addAndMakeVisible(presetLabel);
    addAndMakeVisible(presets); refreshPresets();
    presets.onChange=[this]{ processor.loadPreset(presets.getSelectedItemIndex()); };
    addAndMakeVisible(prev); addAndMakeVisible(next); addAndMakeVisible(save);
    prev.onClick=[this]{presets.setSelectedItemIndex(juce::jmax(0,presets.getSelectedItemIndex()-1));};
    next.onClick=[this]{presets.setSelectedItemIndex(juce::jmin(presets.getNumItems()-1,presets.getSelectedItemIndex()+1));};
    save.onClick = [] {};
    };
    const char* ids[]={"wave","oscMix","sub","detune","cutoff","resonance","attack","decay","sustain","release","drive","reverb","lfoRate","lfoDepth","level"};
    const char* names[]={"WAVE","OSC MIX","SUB","DETUNE","CUTOFF","RESONANCE","ATTACK","DECAY","SUSTAIN","RELEASE","DRIVE","REVERB","LFO RATE","LFO DEPTH","LEVEL"};
    for(int i=0;i<15;++i) {
        auto c=std::make_unique<Control>();
        c->label.setText(names[i],juce::dontSendNotification); c->label.setJustificationType(juce::Justification::centred);
        c->label.setColour(juce::Label::textColourId,juce::Colour(0xff555555)); addAndMakeVisible(c->label);
        c->slider.setSliderStyle(i==0?juce::Slider::Rotary:juce::Slider::RotaryHorizontalVerticalDrag);
        c->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,58,18);
        c->slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xfff07824));
        c->slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xfff07824));
        c->slider.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xff333333));
        addAndMakeVisible(c->slider);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters,ids[i],c->slider));
        controls.push_back(std::move(c));
    }
}
void OrangeSynthAudioProcessorEditor::refreshPresets()
{
    presets.clear(); auto names=processor.getPresetNames();
    for(int i=0;i<names.size();++i) presets.addItem(names[i],i+1);
    if(names.size()>0) presets.setSelectedItemIndex(0,juce::dontSendNotification);
}
void OrangeSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xfffafafa));
    g.setColour(juce::Colour(0xfff07824)); g.drawRoundedRectangle(8,8,getWidth()-16,getHeight()-16,12,2.0f);
    g.setColour(juce::Colour(0xffeeeeee)); g.drawLine(24,112,getWidth()-24,112,1.0f);
    g.setColour(juce::Colour(0xfffff0e4)); g.fillRoundedRectangle(22,128,getWidth()-44,66,10);
}
void OrangeSynthAudioProcessorEditor::resized()
{
    brand.setBounds(24,18,260,48); subtitle.setBounds(27,66,300,22);
    presetLabel.setBounds(350,26,65,20); presets.setBounds(410,24,245,28);
    prev.setBounds(660,24,38,28); next.setBounds(702,24,38,28); save.setBounds(640,151,90,30);
    const int left=28, top=220, cellW=140, cellH=130;
    for(int i=0;i<(int)controls.size();++i) {
        int row=i/5, col=i%5;
        controls[i]->label.setBounds(left+col*cellW,top+row*cellH,cellW,20);
        controls[i]->slider.setBounds(left+col*cellW+14,top+row*cellH+18,cellW-28,104);
    }
}
