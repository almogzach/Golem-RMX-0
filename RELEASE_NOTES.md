# Golem-RMX-0 v1.0

Update file: `LXRE_GRMX0_V1.0.img`. Based on Erica Synths' LXR Eurorack firmware v1.51.
Unofficial: not made, approved or supported by Erica Synths or by the RMX-0 author.

This release merges **[RMX-0](https://github.com/mechanical-dance/RMX-0) by
[mechanical-dance](https://github.com/mechanical-dance)** into **Golem** (v1.51.5). Thank you to mechanical-dance for
RMX-0, and to **Erica Synths** for opening the LXR firmware.

## From RMX-0
- **LFOs:** eleven new shapes, including smooth random and one-shot envelopes; click-free retriggers; pan that moves
  sounding notes; a restart menu (SHIFT + EDIT + PERF).
- **MIDI:** MIDI input on the OP HH jack, with a channel and note per voice, voice links (unison, chord, poly) for
  drums 1–3, and glide and legato per drum.
- **Samples:** your own WAVs as oscillator waveforms, with start and end trims. Reinstall them after every update.
- **Reverb:** a global stereo reverb with a send per voice.

## Joining the two
- Voice 7 (Golem's separate open hat) gets RMX-0's declick, pan-follow, samples and MIDI, plus its own reverb send.
- The reverb page sits on the FX button cycle: FX A → FX B → Reverb → Random. Its output and return level are on the
  page's 2nd screen, because the Perform page keeps Golem's morph controls.
- The randomizer and morph leave voice links, glide, sample trims and the reverb's bus settings alone.
- Kits from stock 1.51, Golem and RMX-0 all load: RMX-0 kits keep their settings, Golem kits get the reverb sends at 0.

## Everything from Golem
FX bus B, the separate voice 7 with choke switch, the randomizer, per-voice morph, the morph chain with CV kit
selection, the delay CV fix and the mod-target fix.
