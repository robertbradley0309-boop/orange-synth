#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    float getP(juce::AudioProcessorValueTreeState& s, const char* id)
    {
        if (auto* p = s.getRawParameterValue(id))
            return p->load();

        return 0.0f;
    }

    juce::File userPresetDir()
    {
        auto dir = juce::File::getSpecialLocation(
            juce::File::userDocumentsDirectory)
            .getChildFile("OrangeSynth").getChildFile("Presets");

        dir.createDirectory();
        return dir;
    }
}

OrangeSynthAudioProcessor::OrangeSynthAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (int i = 0; i < 12; ++i)
    {
        auto* v = new Voice();
        v->setParams(this);
        synth.addVoice(v);
    }

    synth.addSound(new Sound());

    auto parsed = juce::JSON::parse(
        juce::String::fromUTF8(
            BinaryData::FactoryPresets_json,
            BinaryData::FactoryPresets_jsonSize));

    factoryBank = parsed;
    refreshPresetNames();
    loadPreset(0);
}

juce::AudioProcessorValueTreeState::ParameterLayout
OrangeSynthAudioProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto add = [&p](const char* id, const char* name,
                    float lo, float hi, float def)
    {
        p.push_back(std::make_unique<P>(
            juce::ParameterID{id, 1},
            name,
            juce::NormalisableRange<float>(lo, hi),
            def));
    };

    p.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"wave", 1},
        "Wave",
        juce::StringArray{"Sine", "Saw", "Square", "Triangle"},
        1));

    add("oscMix", "Osc Mix", 0, 1, 0.65f);
    add("sub", "Sub", 0, 1, 0.25f);
    add("detune", "Detune", 0, 1, 0.18f);
    add("cutoff", "Cutoff", 0, 1, 0.62f);
    add("resonance", "Resonance", 0, 1, 0.18f);
    add("attack", "Attack", 0.001f, 2, 0.01f);
    add("decay", "Decay", 0.01f, 3, 0.3f);
    add("sustain", "Sustain", 0, 1, 0.6f);
    add("release", "Release", 0.01f, 4, 0.25f);
    add("drive", "Drive", 0, 1, 0.2f);
    add("reverb", "Reverb", 0, 1, 0.12f);
    add("lfoRate", "LFO Rate", 0.05f, 12, 0.2f);
    add("lfoDepth", "LFO Depth", 0, 1, 0.1f);
    add("level", "Level", 0, 1, 0.8f);

    return {p.begin(), p.end()};
}

bool OrangeSynthAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || l.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

void OrangeSynthAudioProcessor::prepareToPlay(double sr, int)
{
    synth.setCurrentPlaybackSampleRate(sr);

    juce::dsp::Reverb::Parameters rp;
    rp.roomSize = 0.45f;
    rp.damping = 0.55f;
    rp.wetLevel = 0.2f;
    rp.dryLevel = 0.8f;

    reverb.setParameters(rp);
}

void OrangeSynthAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    auto wet = getP(parameters, "reverb");

    if (wet > 0.001f)
    {
        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = 0.45f;
        rp.damping = 0.48f;
        rp.wetLevel = wet * 0.45f;
        rp.dryLevel = 1.0f - wet * 0.18f;

        reverb.setParameters(rp);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> ctx(block);

        reverb.process(ctx);
    }

    buffer.applyGain(getP(parameters, "level"));
}

void OrangeSynthAudioProcessor::Voice::startNote(
    int midiNote, float vel, juce::SynthesiserSound*, int)
{
    note = midiNote;
    velocity = vel;
    env = 0;
    releasing = false;
    filterState = 0;
    phase1 = phase2 = phase3 = 0;
}

void OrangeSynthAudioProcessor::Voice::stopNote(float, bool tail)
{
    if (tail)
    {
        releasing = true;

        releaseStep = std::max(
            0.00001f,
            env / (float)(owner->getSampleRate()
            * getP(owner->parameters, "release")));
    }
    else
    {
        clearCurrentNote();
        env = 0;
    }
}

float OrangeSynthAudioProcessor::Voice::osc(double ph, int wave) const
{
    const double t = ph - std::floor(ph);

    switch (wave)
    {
        case 0:
            return std::sin(
                juce::MathConstants<double>::twoPi * t);

        case 1:
            return (float)(2.0 * t - 1.0);

        case 2:
            return t < 0.5 ? 1.0f : -1.0f;

        default:
            return (float)(1.0 - 4.0 * std::abs(t - 0.5));
    }
}

void OrangeSynthAudioProcessor::Voice::renderNextBlock(
    juce::AudioBuffer<float>& out, int start, int count)
{
    if (!isVoiceActive() || owner == nullptr)
        return;

    auto& p = owner->parameters;

    const float sr = (float)owner->getSampleRate();

    const int wave = (int)getP(p, "wave");

    const float mix = getP(p, "oscMix");
    const float sub = getP(p, "sub");
    const float det = getP(p, "detune");

    const float cutoff = getP(p, "cutoff");
    const float res = getP(p, "resonance");

    const float a = getP(p, "attack");
    const float d = getP(p, "decay");
    const float s = getP(p, "sustain");

    const float drive = getP(p, "drive");
    const float lfoRate = getP(p, "lfoRate");
    const float lfoDepth = getP(p, "lfoDepth");

    const double base =
        juce::MidiMessage::getMidiNoteInHertz(note) / sr;

    const double detuneRatio =
        std::pow(2.0, (det * 22.0) / 1200.0);

    const float coeff =
        1.0f - std::exp(
            -2.0f * juce::MathConstants<float>::pi
            * (35.0f + cutoff * 18000.0f) / sr);

    for (int i = 0; i < count; ++i)
    {
        if (!releasing)
        {
            if (env < 1.0f && a > 0)
                env = std::min(1.0f, env + 1.0f / (a * sr));

            else if (env > s)
                env = std::max(
                    s,
                    env + (s - env) / (std::max(0.01f, d) * sr * 0.35f));
        }
        else
        {
            env = std::max(0.0f, env - releaseStep);

            if (env <= 0.0001f)
            {
                clearCurrentNote();
                break;
            }
        }

        const double lfoPhase =
            (double)i / sr * lfoRate
            * juce::MathConstants<double>::twoPi;

        const float lfo = (float)std::sin(lfoPhase) * lfoDepth;

        const float o1 = osc(phase1, wave);
        const float o2 = osc(phase2, (wave + 1) % 4);
        const float o3 = osc(phase3, 2);

        phase1 += base;
        phase2 += base * detuneRatio;
        phase3 += base * 0.5;

        phase1 -= std::floor(phase1);
        phase2 -= std::floor(phase2);
        phase3 -= std::floor(phase3);

        float sig =
            (o1 * (1.0f - mix * 0.45f)
            + o2 * mix * 0.55f
            + o3 * sub * 0.65f)
            / (1.0f + sub * 0.65f);

        const float fc =
            juce::jlimit(0.001f, 0.98f, coeff * (1.0f + lfo * 0.7f));

        filterState += fc * (sig - filterState);

        sig = std::tanh(filterState * (1.0f + drive * 9.0f));

        sig *= env * velocity * 0.22f;

        for (int ch = 0; ch < out.getNumChannels(); ++ch)
            out.addSample(ch, start + i, sig);
    }
}

void OrangeSynthAudioProcessor::getStateInformation(
    juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();

    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    copyXmlToBinary(*xml, dest);
}

void OrangeSynthAudioProcessor::setStateInformation(
    const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(
        getXmlFromBinary(data, size));

    if (xml && xml->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(
            juce::ValueTree::fromXml(*xml));
    }
}

juce::StringArray OrangeSynthAudioProcessor::getPresetNames() const
{
    juce::StringArray result;

    if (auto* arr = factoryBank.getProperty("presets", {}).getArray())
    {
        for (auto& v : *arr)
            result.add(v.getProperty("name", "Preset").toString());
    }

    auto dir = userPresetDir();

    for (auto f : dir.findChildFiles(
        juce::File::findFiles, false, "*.json"))
    {
        result.add("[USER] " + f.getFileNameWithoutExtension());
    }

    return result;
}

void OrangeSynthAudioProcessor::refreshPresetNames()
{
}

void OrangeSynthAudioProcessor::applyPresetObject(const juce::var& obj)
{
    auto params = obj.getProperty("params", {});

    const char* ids[] = {
        "wave", "oscMix", "sub", "detune", "cutoff",
        "resonance", "attack", "decay", "sustain", "release",
        "drive", "reverb", "lfoRate", "lfoDepth", "level"
    };

    for (auto* id : ids)
    {
        if (params.hasProperty(id))
        {
            if (auto* ranged =
                dynamic_cast<juce::RangedAudioParameter*>(
                    parameters.getParameter(id)))
            {
                auto value = (float)params.getProperty(id, juce::var());

                ranged->setValueNotifyingHost(
                    ranged->convertTo0to1(value));
            }
        }
    }
}

void OrangeSynthAudioProcessor::loadPreset(int index)
{
    auto names = getPresetNames();

    if (index < 0 || index >= names.size())
        return;

    if (index < 200)
    {
        if (auto* arr = factoryBank.getProperty("presets", {}).getArray())
        {
            if (index < arr->size())
                applyPresetObject(arr->getReference(index));
        }
    }
    else
    {
        auto f = userPresetDir().getChildFile(
            names[index].fromFirstOccurrenceOf("] ", false, false) + ".json");

        auto v = juce::JSON::parse(f);

        if (v.isObject())
            applyPresetObject(v);
    }
}

bool OrangeSynthAudioProcessor::saveUserPreset(const juce::String& name)
{
    auto safe = name.trim().replaceCharacters(
        "\\/:*?\"<>|", "_________");

    if (safe.isEmpty())
        return false;

    juce::DynamicObject::Ptr params = new juce::DynamicObject();

    const char* ids[] = {
        "wave", "oscMix", "sub", "detune", "cutoff",
        "resonance", "attack", "decay", "sustain", "release",
        "drive", "reverb", "lfoRate", "lfoDepth", "level"
    };

    for (auto* id : ids)
    {
        if (auto* ranged =
            dynamic_cast<juce::RangedAudioParameter*>(
                parameters.getParameter(id)))
        {
            params->setProperty(
                id, ranged->convertFrom0to1(ranged->getValue()));
        }
    }

    juce::DynamicObject::Ptr root = new juce::DynamicObject();

    root->setProperty("name", safe);
    root->setProperty("category", "User");
    root->setProperty("params", juce::var(params.get()));

    return userPresetDir().getChildFile(safe + ".json")
        .replaceWithText(
            juce::JSON::toString(juce::var(root.get()), true));
}

juce::String OrangeSynthAudioProcessor::getPresetFolder() const
{
    return userPresetDir().getFullPathName();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OrangeSynthAudioProcessor();
}
