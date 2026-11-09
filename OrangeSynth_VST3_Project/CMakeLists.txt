cmake_minimum_required(VERSION 3.22)

project(OrangeSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
)

FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(OrangeSynthData SOURCES
    FactoryPresets/FactoryPresets.json
)

juce_add_plugin(OrangeSynth
    COMPANY_NAME "Orange Audio"
    BUNDLE_ID "com.orangeaudio.orangesynth"
    PLUGIN_MANUFACTURER_CODE OrAu
    PLUGIN_CODE Orng
    FORMATS VST3 Standalone
    PRODUCT_NAME "ORANGE"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(OrangeSynth)

target_sources(OrangeSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(OrangeSynth PRIVATE Source)

target_link_libraries(OrangeSynth PRIVATE
    OrangeSynthData
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
