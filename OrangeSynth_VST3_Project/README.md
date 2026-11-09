# ORANGE — VST3 Synthesizer project

ORANGE is an original polyphonic synth project for Windows / FL Studio. It has a bright white GUI with orange outlines, 15 sound controls, 200 factory presets, and a user preset folder.

## Important: what is included
This download is the complete JUCE/C++ source project and preset bank. It is **not a precompiled `.vst3` binary**. A Windows build is required to create the plugin binary.

## Build on Windows
1. Install **Visual Studio 2022 Community** and select the **Desktop development with C++** workload.
2. Install **CMake** (3.22 or newer) and **Git**.
3. Extract this ZIP to a normal folder, for example `C:\Audio\OrangeSynth`.
4. Open a terminal in that folder and run:

   ```powershell
   cmake -B build -S . -A x64
   cmake --build build --config Release
   ```

   CMake downloads JUCE 8.0.6 during configuration, so an internet connection is needed the first time.
5. The VST3 will be produced in the build tree under the `OrangeSynth_artefacts\Release\VST3` folder. Copy `ORANGE.vst3` to:
   `C:\Program Files\Common Files\VST3`
6. Restart FL Studio and use **Options → Manage plugins → Find installed plugins**.

## Add your own presets
The 200 factory presets are embedded in the plugin at build time. User presets are saved as individual JSON files in:

`Documents\OrangeSynth\Presets`

Use the **SAVE PRESET** button in the plugin. The files are plain JSON, so you can back them up, copy them to another PC, or edit them. A user preset is automatically listed the next time the plugin opens.

## Sound engine
- Three oscillator layers (main, detuned, sub)
- Sine, saw, square and triangle oscillator shapes
- Polyphonic MIDI input
- Low-pass tone shaping, resonance and drive
- Attack / decay / sustain / release controls
- LFO movement and global reverb

## Preset styles
The factory bank contains original sounds in the direction of rolling rave/house, UK garage, speed garage, bassline, punchy club synths, trance/rave, acid/techno, house and donk. Artist/style names are used only as broad creative references; the presets are original.

## Notes
This is a starter version 1.0 synth engine. It is suitable for building/testing and sound-design iteration, but it is not a Serum replacement and does not yet include sample import, a wavetable editor, preset tagging/search, or advanced modulation routing.
