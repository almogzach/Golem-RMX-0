# Golem-RMX-0 (LXR firmware mod)

Unofficial custom firmware for the Erica Synths LXR Eurorack drum module. It combines two community firmwares,
both built on Erica Synths' open-source LXR firmware v1.51:

- **Golem** by Zach ([almogzach](https://github.com/almogzach)): a second FX bus, a fully separate open hi-hat
  (voice 7), a parameter randomizer, per-voice morph and a morph chain with CV kit selection.
- **[RMX-0](https://github.com/mechanical-dance/RMX-0) by [mechanical-dance](https://github.com/mechanical-dance)**: new LFO shapes, click-free
  retriggers, pan that follows sounding notes, a restart menu, MIDI input with voice links and glide, user samples
  and a global reverb.

> **Unofficial, modified firmware.** Golem-RMX-0 is not official Erica Synths firmware and is not made, approved or
> supported by Erica Synths, or by the RMX-0 author. Install it at your own risk.
> The start-up screen shows **`GOLEM-RMX0 v1.0`** / **`LXR firmware mod`** so you can always tell it apart.

## Credits

- **[Erica Synths](https://www.ericasynths.lv)** made the LXR Eurorack module and published its firmware under the
  GPL-3.0 ([erica-synths/lxr-eurorack-app](https://github.com/erica-synths/lxr-eurorack-app)). Everything this
  firmware does on the module starts from their code, and their full history is kept in this repository.
- **[mechanical-dance](https://github.com/mechanical-dance)** wrote **[RMX-0](https://github.com/mechanical-dance/RMX-0)**: the LFO, MIDI,
  voice-link, glide, sample, reverb, pan, declick and restart-menu features here are their work, merged into Golem.
  Their user guide is the reference for those features: [RMX-0 docs](https://github.com/mechanical-dance/RMX-0/tree/main/docs).
- **Julian Schmidt (Sonic Potions)** created the original LXR drum machine and firmware that Erica Synths' LXR
  firmware is based on.
- **Brendan Clarke** wrote [LXR02Open](https://github.com/brendanclarke/LXR02Open), which RMX-0 credits for showing
  samples running from the module's flash and for inspiring several of its LFO shapes.
- **Golem** and the merge: Zach ([almogzach](https://github.com/almogzach)).

## What RMX-0 brings
Details for each feature are in the [RMX-0 docs](https://github.com/mechanical-dance/RMX-0/tree/main/docs). What differs in this merge is noted.

### R1. LFOs ([RMX-0 guide](https://github.com/mechanical-dance/RMX-0/blob/main/docs/lfo.md))
- Eleven new shapes after the original eight: smooth random (`nse`), exponential triangle (`xtr`), one-shot
  versions of the main shapes (`si1 tr1 su1 sd1 xu1 xd1 xt1`), a one-shot gate (`sq1`) and random-per-hit (`rn1`).
  One-shots work as extra envelopes.
- **Click-free retriggers:** retriggering a sounding voice fades the old note out over 3 ms. *Golem-RMX-0: also on
  voice 7.*
- **Pan follows sounding notes:** the pan knob, LFOs, CV and link width move notes already playing.
  *Golem-RMX-0: also on voice 7.*
- **Restart menu:** SHIFT + EDIT + PERF, then turn the encoder right to fill the bar. Any button leaves.
  Holding the encoder as the bar fills enters the updater.

### R2. MIDI, voice links and glide ([MIDI](https://github.com/mechanical-dance/RMX-0/blob/main/docs/midi.md), [links](https://github.com/mechanical-dance/RMX-0/blob/main/docs/voice-links.md), [glide](https://github.com/mechanical-dance/RMX-0/blob/main/docs/glide-legato.md))
- The **OP HH** trigger jack can work as a MIDI input (TRS type A). Set it in global settings (SHIFT + LOAD/SAVE):
  `mid`, a global channel `ch`, a channel per voice `t1`–`t7` and a note per voice `n1`–`n7`.
- **Voice links:** drums 1–3 play as one instrument in unison, chord or poly mode, with detune and stereo width.
- **Glide and legato** per drum, with three glide curves.
- *Golem-RMX-0:* `t7`/`n7` play Golem's separate voice 7. Link and glide settings are left alone by the randomizer
  and by morphing.

### R3. Samples ([RMX-0 guide](https://github.com/mechanical-dance/RMX-0/blob/main/docs/samples.md))
- Put WAV files in `/samples` and `/loops` on the card, then **LOAD/SAVE** until `Load:Samples` and press the
  encoder. Up to 120 files, 1.5 MB of flash.
- Samples appear after `Pwm` on any voice's waveform (`s0`, `s1`, …), with `sta`/`end` trims on the envelope
  page's 2nd screen.
- **Every firmware update erases installed samples.** Reinstall them after flashing.
- *Golem-RMX-0:* voice 7 can play samples too; voices 6 and 7 share one pair of trims. The randomizer never
  changes trims.

### R4. Global reverb ([RMX-0 guide](https://github.com/mechanical-dance/RMX-0/blob/main/docs/reverb.md))
- A stereo reverb on its own bus, next to the FX buses, saved with the kit.
- Each voice's **Mix** page has a `rev` send. *Golem-RMX-0: voice 7 has its own send on its Mix page (RMX-0's hats
  share one).* Kits without that setting give voice 7 the same send as voice 6.
- *Golem-RMX-0:* the reverb page is on the FX button cycle: **pre** pre-delay, **dec** decay, **ton** tone,
  **ref** reflection; turn the encoder to its 2nd screen for **out** (output) and the return level. The Perform page
  keeps Golem's morph controls, so the return level isn't on a Perform pot here.
- A load guard bypasses the reverb if the module runs out of processing time, and brings it back when there is room.

## What Golem brings
### G1. A second FX bus (FX B)
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

### G2. The open hi-hat (voice 7) is a separate voice
Voice 7 has its own complete sound (oscillator, waveform/sample, PWM, FM, transient, filter, envelope) and its own
Mix page (volume, pan, sample-rate reduction, drive, output including **FX** and **FXB**).
A new **chk (Choke)** setting on voice 7's Mix page turns the closed-hat choke on or off.
Voice 7 has no LFO of its own, and the CV pitch input reaches voice 6 only.

### G3. Parameter randomizer
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

### G4. Per-voice morph
The Performance page has a new **mvc** (morph voice) control. `all` morphs the whole kit as before; `v1`–`v7`
morphs only that voice toward the same voice in the morph target kit. Outputs and choke never morph.
Voice 7's own settings take part in morphing toward kits saved with Golem.

### G5. Morph chain
- **msr** picks the morph source kit (the edit buffer by default), **mtg** the target. Changes wait until the morph
  knob is at the silent end, so switching kits never glitches.
- **aut** auto-advances: each time the morph reaches one end, the kit on that side steps to the next kit in the
  **lo**–**hi** range, so sweeping back and forth walks through a chain of kits.
- **scv**/**tcv** (global settings, sub-page 2) let CV inputs choose the source and target kits.
- The delay no longer glitches when a CV moves its time, and a mod-target bug that read past the end of a table
  is fixed.

## Navigation
- **SHIFT + FX**, then press again to cycle: FX A → **Fx B** → **Reverb** → **Random** → FX A.
- Performance page: **mrp** (morph), **mtg** (morph target), **sr** (sample rate), **mvc** (morph voice) |
  **msr** (morph source), **aut** (auto-advance), **lo**/**hi** (kit range).
- Global settings, sub-page 2: **scv**/**tcv** pick the CV inputs that select the morph source and target kits.

## Your existing kits are safe
- Kits from the official firmware load with FX B off, voice 7 as a copy of the shared hat with choke on, the reverb
  sends at 0 and the RMX-0 settings at their defaults.
- Kits saved with **Golem** load with the reverb sends at 0.
- Kits saved with **RMX-0** load with all their RMX-0 settings in place; FX B starts off and voice 7 starts as a
  copy of voice 6.
- Kits saved with Golem-RMX-0 still load on the official 1.51 firmware, which ignores the extra settings.

## Install
1. Download `LXRE_GRMX0_V1.0.img` from the [Releases](../../releases) page. (The module's updater only accepts files
   whose names start with `LXRE`.)
2. Copy it to the top folder of a FAT32 SD card and delete any other `LXRE*.img` files from the card.
3. With the module off, insert the card, then **press and hold the encoder** (the menu knob next to the display)
   while switching the power on. Keep holding until the updater screen appears.
4. Wait for the success message. The module restarts and shows `GOLEM-RMX0 v1.0`.
5. Reinstall your samples (LOAD/SAVE → `Load:Samples`) if you use them.

**Going back:** flash the official 1.51 file from Erica Synths (or a Golem or RMX-0 release) the same way. The
updater (bootloader) is never changed by an update, so you can always return.

## Notes
- CPU: this firmware runs one extra hi-hat voice, two FX buses and a reverb. Heavy settings on all three are the
  most demanding case. The reverb's load guard steps in first; if you still hear crackles, turn FX B off.
- Build: same as the official firmware (see below). The update file is
  `build/release/LXRE_GRMX0_V<version>.img`.
- In the source, Golem changes are marked `MODIFIED`, RMX-0's `lxr-0`, and code joining the two `GOLEM-RMX-0`.
  RMX-0's features can be switched off one by one in `Core/Inc/lxr0.h`.

## Licence
GPL-3.0-only with Erica Synths' additional terms in [NOTICE.md](NOTICE.md), unchanged from the original. RMX-0 is
also GPL-3.0-only. "Erica Synths", "LXR" and "LXR-02" are trademarks of Erica Synths and are used here only to say
which module this firmware is for.

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
| FLASH | 0x08008000 | 480 KB | This firmware |
| FLASH | 0x08080000 | 1.5 MB | Samples (sectors 6–11), installed from the SD card; erased by a firmware update |
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
