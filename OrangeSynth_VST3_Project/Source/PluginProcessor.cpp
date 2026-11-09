void OrangeSynthAudioProcessor::applyPresetObject(const juce::var& obj)
{
    auto params = obj.getProperty("params", juce::var());

    const char* ids[] = {
        "wave", "oscMix", "sub", "detune", "cutoff",
        "resonance", "attack", "decay", "sustain", "release",
        "drive", "reverb", "lfoRate", "lfoDepth", "level"
    };

    for (auto* id : ids)
    {
        if (params.hasProperty(id))
        {
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameters.getParameter(id)))
            {
                auto value = (float)params.getProperty(id, juce::var());
                ranged->setValueNotifyingHost(ranged->convertTo0to1(value));
            }
        }
    }
}
