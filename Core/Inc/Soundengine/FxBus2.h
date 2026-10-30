/*
 * FxBus2.h
 *
 * Second FX bus ("FX B") for the LXR V2 Eurorack module.
 * MODIFIED VERSION - not official Erica Synths firmware.
 *
 * Bus B offers Drive, Ring and Compressor. The delay stays on bus A only,
 * because it needs the single 64 KB delay memory (FX_BUFFER) in DTCM.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * This file is part of a modified LXR firmware, licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_SOUNDENGINE_FXBUS2_H_
#define INC_SOUNDENGINE_FXBUS2_H_

#include "stm32f7xx_hal.h"
#include "Soundengine/DSPAudio/FX/Distortion.h"
#include "Soundengine/DSPAudio/FX/Ringmod.h"
#include "Soundengine/DSPAudio/FX/Compressor.h"

enum FxBus2TypeEnum
{
	FX2_OFF,
	FX2_DISTORTION,
	FX2_RING,
	FX2_COMPRESSOR,
	FX2_NUM_TYPES
};

typedef struct Fx2Struct
{
	uint8_t fxType;
	uint8_t audioRouting;
	float dryWet;
	float parameter[4];

	MasterDistortion distortion;
	Ringmod ringmod;
	Compressor compressor;
} FxBus2;

extern FxBus2 fxBus2;

void FxBus2_init(void);
void FxBus2_process(int16_t *fxBuffer, int16_t* outL, int16_t* outR, int16_t* outL2, int16_t* outR2, uint32_t size);

/** apply one Bus B parameter. paramNr is the full parameter number (PAR_FX2_*), value is the raw 0..127 menu value */
void FxBus2_setFromParam(uint16_t paramNr, uint8_t value);
/** apply all Bus B parameters from a parameter array (used after a kit is loaded) */
void FxBus2_loadFromParameters(const uint8_t *params);

#endif /* INC_SOUNDENGINE_FXBUS2_H_ */
