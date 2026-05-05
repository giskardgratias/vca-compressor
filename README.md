# VCA Compressor — VST3 Plugin for Cubase

A transparent, musical VCA-style stereo compressor built with the Steinberg VST3 SDK.

## Features

| Parameter   | Range              | Description                                          |
|-------------|--------------------|----------------------------------------------------- |
| Threshold   | -60 … 0 dBFS       | Level above which compression starts                 |
| Ratio       | 1:1 … ∞:1          | Compression slope (≥20 treated as limiting)          |
| Attack      | 0.1 … 300 ms       | Time to reach compression target                     |
| Release     | 10 … 3000 ms       | Time to return to unity gain                         |
| Makeup      | 0 … +30 dB         | Output level compensation                            |
| Knee        | 0 … 20 dB          | Soft-knee width around threshold                     |
| Mix         | 0 … 100 %          | Parallel compression (New York style)                |
| SC HPF      | Off / 60 / 100 / 200 Hz | Sidechain high-pass filter (de-ess pumping)    |
| Bypass      | on / off           | True bypass                                          |
| GR Meter    | read-only          | Gain reduction sent to host automation lane          |

## DSP Architecture

```
Audio In (L/R)
    │
    ├──→ [Sidechain HPF] → [RMS Detector] → [Gain Computer] → [Attack/Release]
    │                       (stereo linked)    (soft knee)       (VCA ballistics)
    │                                               │
    └──────────────── Mix ───────────── × gain ────┘
                     (dry)              × makeup
                                           │
                                       Audio Out (L/R)
```

- **Feed-forward RMS detection** — clean, predictable compression
- **Stereo-linked**: gain determined by the louder channel, applied to both
- **Log-domain gain computer** with optional soft knee
- **Per-sample ballistics** — no block-boundary artefacts
- **Parallel compression** via dry/wet Mix knob

---

## Build Instructions

### Prerequisites

1. **C++ compiler** — MSVC 2019+ (Windows), Clang / GCC 9+ (macOS/Linux)
2. **CMake** ≥ 3.15
3. **VST3 SDK** — clone from Steinberg:
   ```
   git clone --recursive https://github.com/steinbergmedia/vst3sdk.git
   ```

### Windows (MSVC)

```bat
mkdir build && cd build
cmake .. -DVST3_SDK_ROOT=C:/path/to/vst3sdk -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The `.vst3` bundle lands in `build/VST3/Release/`.
Copy it to `C:\Program Files\Common Files\VST3\`.

### macOS (Xcode / Clang)

```bash
mkdir build && cd build
cmake .. -DVST3_SDK_ROOT=~/vst3sdk -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Copy `build/VST3/Release/VCA Compressor.vst3` to `/Library/Audio/Plug-Ins/VST3/`.

### Linux (GCC / Clang)

```bash
mkdir build && cd build
cmake .. -DVST3_SDK_ROOT=~/vst3sdk -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Copy to `~/.vst3/` or `/usr/lib/vst3/`.

---

## Project Structure

```
VCACompressor/
├── CMakeLists.txt
└── source/
    ├── version.h                    ← UIDs, version strings
    ├── VCACompressorIDs.h           ← Parameter IDs & ranges
    ├── VCACompressorDSP.h/.cpp      ← Pure DSP (no VST3 deps)
    ├── VCACompressorProcessor.h/.cpp← IAudioProcessor
    ├── VCACompressorController.h/.cpp← IEditController
    └── entry.cpp                    ← Factory registration
```

---

## Customisation Tips

- **Change UIDs** in `version.h` before releasing (use `uuidgen` / `guidgen`).
- **Add VSTGUI** for a custom UI: set `SMTG_ADD_VSTGUI ON` in CMakeLists and
  implement `createView()` in `VCACompressorController`.
- **Lookahead** can be added in `VCACompressorDSP` by buffering the input
  `n` samples and delaying audio while running detection ahead.
- **Stereo Mid/Side** mode: transform L/R → M/S before detection for
  mastering-style compression.
