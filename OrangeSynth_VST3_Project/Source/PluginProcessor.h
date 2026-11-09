#pragma once
#include <JuceHeader.h>

class OrangeSynthAudioProcessor : public juce::AudioProcessor
{
public:
    OrangeSynthAudioProcessor();
    ~OrangeSynthAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ORANGE"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    juce::StringArray getPresetNames() const;
    void loadPreset(int index);
    bool saveUserPreset(const juce::String& presetName);
    juce::String getPresetFolder() const;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct Sound : public juce::SynthesiserSound { bool appliesToNote(int) override { return true; } bool appliesToChannel(int) override { return true; } };

    struct Voice : public juce::SynthesiserVoice
    {
        bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<juce::SynthesiserSound*>(s) != nullptr; }
        void startNote(int midiNote, float velocity, juce::SynthesiserSound*, int) override;
        void stopNote(float, bool allowTailOff) override;
        void pitchWheelMoved(int) override {}
        void controllerMoved(int, int) override {}
        void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;
        void setParams(OrangeSynthAudioProcessor* p) { owner = p; }
        OrangeSynthAudioProcessor* owner = nullptr;
        double phase1=0, phase2=0, phase3=0;
        float env=0, velocity=0, releaseStep=0;
        bool releasing=false;
        float filterState=0;
        int note=60;
        float osc(double phase, int wave) const;
    };

    juce::Synthesiser synth;
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> reverbBuffer;
    juce::StringArray presetNames;
    juce::var factoryBank;
    void refreshPresetNames();
    void applyPresetObject(const juce::var& obj);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrangeSynthAudioProcessor)
};
