# Golem (LXR firmware mod)

Unofficial custom firmware for the Erica Synths LXR Eurorack module.

> **Unofficial, modified firmware.** Golem is not official Erica Synths firmware and is not approved by,
> affiliated with or supported by Erica Synths. Install it at your own risk.
> The start-up screen shows **`GOLEM v1.51.3`** / **`LXR firmware mod`** so you can always tell it apart.

Golem is based on the official LXR V2 Eurorack firmware **v1.51** from
[erica-synths/lxr-eurorack-app](https://github.com/erica-synths/lxr-eurorack-app), with the full original history
kept in this repository. It adds four features and leaves everything else as it was.

## What Golem adds

### 1. A second FX bus (FX B)
Different voices can use different effects at the same time.

- Every voice's **Output** setting (Mix page) has a new choice, **FXB**, after **FX**.
- **FX A** is unchanged: Drive, Ring, Compressor or Delay.
- **FX B** offers Drive, Ring or Compressor, with its own output, dry/wet and 4 parameters.
  Delay stays on FX A only, because the module has memory for one delay.

| FX B type | P1 | P2 | P3 | P4 | Ratio |
|---|---|---|---|---|---|
| `drv` | drive | color | tone | volume | unused |
| `rng` | frequency | – | – | – | unused |
| `cmp` | attack | release | threshold | gain | used |

### 2. The open hi-hat (voice 7) is a separate voice
Voice 7 has its own complete sound (oscillator, waveform/sample, PWM, FM, transient, filter, envelope) and its own
Mix page (volume, pan, sample-rate reduction, drive, output including **FX** and **FXB**).
A new **chk (Choke)** setting on voice 7's Mix page turns the closed-hat choke on or off.
Voice 7 has no LFO of its own, and the CV pitch input reaches voice 6 only.

### 3. Parameter randomizer
A **Random** page for creating sounds and kits quickly.

| Control | What it does |
|---|---|
| **amt** | 0–100. Low values make variations of the current sound; 100 makes a new one. |
| **tgt** | `v1`–`v7` for one voice, or `kit` for all seven. Set to the voice you were editing. |
| **rnd** | Turn to `go!` to roll. It returns to `---`, so each further turn rolls another variation. |
| **und** | Turn to `go!` to restore the sound from before the last roll. |

It changes pitch, waveforms, envelopes, FM, transient, filter, drive, sample rate, pan and LFOs within musical
limits (drum-length decays, no filter self-oscillation, moderate drive). It never changes volumes, outputs,
FX routing, choke, velocity response, the FX buses, morph or the CV mod matrix.

### 4. Per-voice morph
The Performance page has a new **mvc** (morph voice) control. `all` morphs the whole kit as before; `v1`–`v7`
morphs only that voice toward the same voice in the morph target kit. Outputs and choke never morph.
Voice 7's own settings take part in morphing toward kits saved with Golem.

## Navigation
- **SHIFT + FX**, then press again to cycle: FX A → **Fx B** → **Random** → FX A.
- Performance page: **mrp** (morph), **mtg** (morph target), **sr** (sample rate), **mvc** (morph voice).

## Your existing kits are safe
- Kits saved with official firmware load with FX B off, and voice 7 starts as a copy of the old shared hat with
  choke on, so they sound the same until you change something.
- Kits saved with Golem still load on official 1.51, which ignores the extra settings.

## Install
1. Download `LXRE_V2_V1.51.3.img` from the [Releases](../../releases) page. (The module's updater only accepts files
   whose names start with `LXRE`, so the file keeps that name.)
2. Copy it to the top folder of a FAT32 SD card and delete any other `LXRE*.img` files from the card.
3. With the module off, insert the card, then **press and hold the encoder** (the menu knob next to the display)
   while switching the power on. Keep holding until the updater screen appears.
4. Wait for the success message. The module restarts with Golem.

**Going back:** flash the official 1.51 file from Erica Synths the same way. The updater (bootloader) is never
changed by an update, so you can always return.

## Notes
- CPU: Golem computes one extra hi-hat voice and up to two effects. Heavy settings on both FX buses at once are
  the most demanding case; if you hear crackles, turn FX B off.
- Build: same as the official firmware (see below). Tested with Arm GNU Toolchain GCC 15.
- Source changes are marked `MODIFIED` in the code.

## Licence
GPL-3.0-only with the additional terms in [NOTICE.md](NOTICE.md), unchanged from the original. "Erica Synths"
and "LXR" are trademarks of Erica Synths and are used here only to say which module this firmware is for.

---

*The original README for the official firmware follows.*

## Original README: LXR V2 Eurorack firmware (STM32F765VG)

Firmware for the LXR V2 Eurorack drum module: an STM32F765 (Cortex-M7, 216 MHz) running the drum
voices, FX bus, trigger and CV inputs, MIDI, SD-card kit storage and the LCD/encoder UI.

> **Licence and trademarks:** the firmware source code is free software under the GNU GPL version 3
> ([LICENSE](LICENSE)). The names **"Erica Synths" and "LXR"** and the **LXR hardware** are the property
> of Erica Synths and are **not** covered by this licence. Building and installing your own firmware is
> at your own risk: Erica Synths provides no support for self-built or modified firmware.
> See [NOTICE.md](NOTICE.md).

## Building

You need [CMake](https://cmake.org/download/) 3.21+, [Ninja](https://ninja-build.org/), the
[Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
(`arm-none-eabi-gcc`, tested with 14.3.rel1) and [Python 3](https://www.python.org/downloads/), all on `PATH`.

- macOS: `brew install cmake ninja && brew install --cask gcc-arm-embedded`. Don't use
  `brew install arm-none-eabi-gcc`: it lacks the C library and the build fails with it.
- Windows: `winget install Kitware.CMake Ninja-build.Ninja Python.Python.3.12` plus Arm's installer
  (tick "Add path to environment variable").
- Linux: `sudo apt install cmake ninja-build python3` plus Arm's toolchain, with its `bin` folder on `PATH`.

Then, in this folder:

```sh
cmake --preset release
cmake --build --preset release
```

The update file is written to `build/release/LXRE_V2_V<version>.img`. The version is set by
`project(VERSION ...)` in [CMakeLists.txt](CMakeLists.txt) and shown on the start-up screen.

| Option | Effect |
|---|---|
| `cmake --preset debug` | Debug build (`-O3 -g3`, `DEBUG` defined) in `build/debug/` |
| `-DARM_TOOLCHAIN_DIR=<bin folder>` | Use a toolchain that isn't on `PATH` |
| `-DLXR_ITCM_OPT=-O2` | Lower optimisation for code in the 16 KB ITCM, if a change or compiler makes it overflow at `-O3` |
| `-DLXR_GENERATE_LISTING=ON` | Also write a disassembly listing (`.list`) |

To start from scratch (e.g. after switching compilers), delete the `build/` folder.

## Installing the update

1. Copy `LXRE_V2_V<version>.img` to the top folder of a FAT32 SD card and delete any older `LXRE*.img`
   files from it.
2. With the module powered off, insert the card, hold down the encoder button and power on. Keep
   holding until "LXR Updater" appears.
3. Wait for "Update OK!". The module restarts with the new firmware.

The updater (bootloader) is never changed by an update, so a failed or interrupted update can simply be
repeated. The module's updater only looks for files named `LXRE*.img`, so desktop LXR update files on
the same card are ignored.

## Programming with a debug probe

Not needed for updates. With an ST-Link and [OpenOCD](https://openocd.org/) on `PATH`:

```sh
cmake --build --preset release --target flash        # firmware only, bootloader untouched
```

For blank devices, configure with the Eurorack bootloader, either from source
(`-DLXR_BOOTLOADER_DIR=path/to/LXR_V2_STM32F765_Bootloader`, built with `LXR_BOOTLOADER_EURORACK`) or
prebuilt (`-DLXR_BOOTLOADER_HEX=path/to/LXR_V2_Eurorack_Bootloader.hex`). The build then also writes
`LXRE_V2_V<version>_full.hex/.bin` (whole flash from 0x08000000) and enables the `flash_full` target.

> **Warning:** `flash_full` overwrites the bootloader and asks you to type `yes` first (or set
> `LXR_CONFIRM_BOOTLOADER_OVERWRITE=yes`). If it is interrupted, the module can only be recovered with a
> debug probe.

## Memory map

| Region | Address | Size | Contents |
|---|---|---|---|
| FLASH | 0x08000000 | 32 KB | Bootloader |
| FLASH | 0x08008000 | 992 KB | This firmware |
| ITCM | 0x00000000 | 16 KB | `IN_ITCM_RAM` DSP code |
| DTCM | 0x20000000 | 128 KB | `INDTCM`/`INDTCMZ` data, DMA buffers, stack |
| RAM | 0x20020000 | 384 KB | `.data`, `.bss`, heap |

## Source layout

Originally an STM32CubeIDE project; the CubeMX `.ioc` was removed because regenerating code from it
overwrites application code. Pins, clocks and peripherals live in `Core/Inc/main.h`, `Core/Src/main.c`
and `Core/Src/stm32f7xx_hal_msp.c`. Source files are listed explicitly in the `CMakeLists.txt` files
(`Core/` is split into `lxr_platform`, `lxr_io`, `lxr_ui`, `lxr_preset`, `lxr_dsp`, `lxr_engine` and
`lxr_midi`); add new files there.

## Licence

Copyright (C) 2012-2026 Erica Synths. Licensed under the [GNU GPL version 3](LICENSE) (GPL-3.0-only)
with the additional permission and terms in [NOTICE.md](NOTICE.md): modified firmware must be marked as
modified and must not be presented as official Erica Synths firmware. The trademarks and the hardware
are not licensed. Third-party components (STMicroelectronics, Arm, ChaN FatFs) keep their own licences,
see [NOTICE.md](NOTICE.md#third-party-components).
