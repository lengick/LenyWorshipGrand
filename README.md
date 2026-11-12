# LenyWorship Grand — clean project

Standalone JUCE/CMake project. This repository is intentionally independent from any previous LenWorship codebase.

## What is included
- New VST3 + Standalone project targeting Windows x64.
- The supplied LenyWorship interface image embedded as the plugin background.
- Six controls matching the artwork: Volume, Pad, Delay, Cutoff, Reverb and Pad Sustain.
- A synthesized warm pad engine with slow attack, long release, stereo detune and low-pass control.
- A sample-based piano engine using JUCE's sampler.
- MIDI note input and sustain-controller support in the piano sampler.

## Important: Grand Piano sample library
A truly realistic acoustic grand piano requires licensed, high-quality recordings. No fake piano samples are included. Use your own legally licensed WAV sample set.

For this first prototype, put one WAV file per MIDI key in a folder and name it using note + octave, for example:
`A0.wav`, `C1.wav`, `C#1.wav`, `D1.wav`, ... `C8.wav`.

In the plugin, click **LOAD PIANO SAMPLES** and select that folder. The prototype maps each file to its named MIDI note. This is a first playable sample-loader, not yet a finished multi-velocity production piano. The next sound-engine milestone is velocity-layered multisampling, pedal-up sympathetic resonance, una corda/soft pedal, and optimized sample streaming.

## Build on Windows 10/11 x64
Requirements:
- Visual Studio 2022 with **Desktop development with C++** workload
- CMake 3.22+
- Git
- Internet on the first configure/build, because CMake downloads JUCE 8.0.6

From this folder in PowerShell:
```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target LenyWorshipGrand_VST3
```

The VST3 bundle will be under:
`build/LenyWorshipGrand_artefacts/Release/VST3/LenyWorship Grand.vst3`

The project also defines a Standalone target for local testing.

## Compatibility
The target format is VST3 64-bit. It is intended for FL Studio and other Windows DAWs that host VST3 plugins. Actual compatibility must be confirmed after compiling and testing in each host.

## Current prototype limitations
- Piano samples are currently one sample per named key; no velocity layers yet.
- Pad sustain-pedal note-off behavior is being refined; the piano sampler already receives MIDI CC64.
- The Cutoff parameter affects the pad only.
- This source has not yet been compiled in this environment. Build/test status must not be represented as successful until verified on Windows.
