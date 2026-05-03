# Rhythm Offset Calibrator

Standalone C++ desktop app for rhythm timing calibration.

## Stack

- C++20
- SDL2
- Dear ImGui
- miniaudio
- CMake

## Features

- BPM control from 40 to 300
- Selectable beat sounds: kick, snare, clap, hi-hat, tick
- Moving calibration marker with centered judgement line
- Space bar timing capture
- Latest, average, median, stddev, and recommended offset stats

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The CMake project fetches third-party dependencies automatically.

## Run

```powershell
.\build\Release\RhythmCalibrator.exe
```
