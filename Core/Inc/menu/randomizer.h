/*
 * randomizer.h
 *
 * Parameter randomization for the LXR V2 Eurorack module.
 * MODIFIED VERSION - not official Erica Synths firmware.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * This file is part of a modified LXR firmware, licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_MENU_RANDOMIZER_H_
#define INC_MENU_RANDOMIZER_H_

#include <stdint.h>

#define RANDOMIZER_TARGET_KIT 7		/**< PAR_RAND_TARGET value 0..6 = voice 1..7, 7 = whole kit */

/** set the default amount and target (call once at start-up) */
void randomizer_init(void);

/**
 * Which voices a sound parameter belongs to, as a bit mask (bit 0 = voice 1 ... bit 6 = voice 7),
 * read from each voice's menu pages. Shared by the randomizer and the per-voice morph.
 * Voice 7's LFO page edits voice 6's LFO, so those LFO parameters count for voice 6 only.
 */
uint8_t voiceParams_getVoiceMask(uint16_t paramNr);

/** 1 if a sound parameter takes part in morphing with the current morph voice setting */
uint8_t voiceParams_isMorphed(uint16_t paramNr, uint8_t morphTargetKitValid7);

/**
 * Randomize the sound parameters of one voice (0..6) or the whole kit (RANDOMIZER_TARGET_KIT).
 * amount 0..100: low values mutate the current sound, 100 = new sound within musical limits.
 * Saves an undo copy first and sends the new values to the sound engine.
 */
void randomizer_run(uint8_t target, uint8_t amount);

/** restore the sound from before the last randomize. Returns 1 if there was something to undo */
uint8_t randomizer_undo(void);

/**
 * Pure helpers, exposed for testing on the host.
 * randomizer_isRandomized(): 1 if paramNr may be changed when randomizing voice (0..6)
 * randomizer_getBounds(): musical range for a parameter; returns 0 if the parameter is excluded
 * randomizer_mutate(): new value for one parameter, given a random number
 */
uint8_t randomizer_isRandomized(uint16_t paramNr, uint8_t voice);
uint8_t randomizer_getBounds(uint16_t paramNr, uint8_t* min, uint8_t* max, uint8_t* isDiscrete);
uint8_t randomizer_mutate(uint8_t current, uint8_t min, uint8_t max, uint8_t isDiscrete, uint8_t amount, uint32_t rnd);

#endif /* INC_MENU_RANDOMIZER_H_ */
