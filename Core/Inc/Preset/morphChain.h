/*
 * morphChain.h
 *
 * Morph source selection, glitch guard, auto-advance and CV kit selection
 * for the LXR V2 Eurorack module.
 * MODIFIED VERSION ("Golem") - not official Erica Synths firmware.
 *
 * The morph blends a SOURCE kit (msr) toward a TARGET kit (mtg). The values the
 * user (or a CV) asks for are kept in activeParameters[PAR_MORPH_SOURCE / _TARGET];
 * the kits actually used ("effective") only change on the side that is silent:
 *   - the target changes only while the morph sits at the source end,
 *   - the source changes only while the morph sits at the target end,
 * so changing kits never makes the sound jump.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * This file is part of a modified LXR firmware, licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_PRESET_MORPHCHAIN_H_
#define INC_PRESET_MORPHCHAIN_H_

#include <stdint.h>

#define MORPHCHAIN_AT_SOURCE(m)	((m) <= 2)		/**< morph value (0..255) counts as fully at the source */
#define MORPHCHAIN_AT_TARGET(m)	((m) >= 253)	/**< morph value (0..255) counts as fully at the target */
#define MORPHCHAIN_NUM_KITS		128

/** call when a kit is loaded into the edit buffer: it becomes the morph source */
void morphChain_setLoadedKit(uint8_t kit);

/** the kit currently in the edit buffer */
uint8_t morphChain_loadedKit(void);

/** effective (sounding) source and target kits */
uint8_t morphChain_sourceKit(void);
uint8_t morphChain_targetKit(void);

/** 1 if the effective source is the loaded kit, so the edit buffer (with unsaved edits) is used */
uint8_t morphChain_sourceIsEditBuffer(void);

/**
 * Apply requested source/target changes on the silent side, and auto-advance if enabled.
 * morph: current morph value 0..255. Returns 1 if the effective source or target changed
 * (the kit then has to be re-sent).
 */
uint8_t morphChain_sync(uint8_t morph);

/**
 * Map a CV (0..1) onto kits lo..hi with a dead zone around each kit, so a noisy or
 * slowly moving CV does not flicker between neighbours. Returns the kit to request.
 */
uint8_t morphChain_cvToKit(float cv, uint8_t current, uint8_t lo, uint8_t hi);

/** next kit after 'kit' inside lo..hi, wrapping; a kit outside the range goes to lo */
uint8_t morphChain_nextKit(uint8_t kit, uint8_t lo, uint8_t hi);

/** read the morph CV jacks (if assigned) and update the requested kits. Call once per control-rate tick. */
void morphChain_cvTick(void);

#endif /* INC_PRESET_MORPHCHAIN_H_ */
