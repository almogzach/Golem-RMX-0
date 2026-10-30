/*
 * FxBus2.c
 *
 * Second FX bus ("FX B") for the LXR V2 Eurorack module.
 * MODIFIED VERSION - not official Erica Synths firmware.
 *
 * Mirrors the structure of FxBus.c (bus A) for the effects that keep their
 * own state: Drive, Ring and Compressor. Bus A is left unchanged.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * This file is part of a modified LXR firmware, licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Soundengine/FxBus2.h"
#include "Soundengine/DSPAudio/mixer.h"
#include "Parameters.h"

FxBus2 fxBus2;

static void FxBus2_updateParameter(void);

void FxBus2_init(void)
{
	fxBus2.fxType = FX2_OFF;
	fxBus2.audioRouting = 0;
	fxBus2.dryWet = 0;
	for(uint8_t i=0;i<4;i++) fxBus2.parameter[i] = 0;

	Distortion_init(&fxBus2.distortion);
	Ringmod_init(&fxBus2.ringmod);
	Compressor_init(&fxBus2.compressor);
}

void FxBus2_process(int16_t *fxBuffer, int16_t* outL, int16_t* outR, int16_t* outL2, int16_t* outR2, uint32_t size)
{
	int16_t* out;
	int16_t* out2 = NULL;
	// < MIXER_ROUTING_DAC1_L -> stereo 1 or 2
	const uint8_t channels = fxBus2.audioRouting < MIXER_ROUTING_DAC1_L ? 2 : 1;

	// same output mapping as bus A (FxBus.c)
	switch(fxBus2.audioRouting)
	{
	default:
	case MIXER_ROUTING_DAC1_STEREO:
		out = outL2;
		out2 = outR2;
		break;
	case MIXER_ROUTING_DAC2_STEREO:
		out = outL;
		out2 = outR;
		break;
	case MIXER_ROUTING_DAC1_L:
		out = outL2;
		break;
	case MIXER_ROUTING_DAC1_R:
		out = outR2;
		break;
	case MIXER_ROUTING_DAC2_L:
		out = outL;
		break;
	case MIXER_ROUTING_DAC2_R:
		out = outR;
		break;
	}

	switch(fxBus2.fxType)
	{
	default:
	case FX2_OFF:
		if(channels == 2)
		{
			int32_t* stereoIn = (int32_t*)fxBuffer;
			int32_t* stereoOut = (int32_t*)out;
			for(uint8_t i=0;i<size;i++)
			{
				*stereoOut = __QADD16(*stereoOut, *stereoIn);
				stereoOut++;
				stereoIn++;
			}
		}
		else
		{
			int32_t* in = (int32_t*)fxBuffer;
			for(uint8_t i=0;i<size;i++)
			{
				*out = __QADD16(*out, *in);
				out += 2;
				in++;
			}
		}
		break;

	case FX2_DISTORTION:
		Distortion_process(&fxBus2.distortion, fxBuffer, out, out2, size, channels, fxBus2.dryWet);
		break;

	case FX2_RING:
		Ringmod_process(&fxBus2.ringmod, fxBuffer, out, out2, size, channels, fxBus2.dryWet);
		break;

	case FX2_COMPRESSOR:
		Compressor_process(&fxBus2.compressor, fxBuffer, out, out2, size, channels, fxBus2.dryWet);
		break;
	}
}

// same parameter mapping as FxBus_updateParameter() for bus A
static void FxBus2_updateParameter(void)
{
	switch(fxBus2.fxType)
	{
	case FX2_DISTORTION:
		Distortion_setDrive(&fxBus2.distortion, fxBus2.parameter[0]);
		Distortion_setParam1(&fxBus2.distortion, fxBus2.parameter[1]);
		Distortion_setParam2(&fxBus2.distortion, fxBus2.parameter[2]);
		Distortion_setParam3(&fxBus2.distortion, fxBus2.parameter[3]);
		break;

	case FX2_RING:
		Ringmod_setFreq(&fxBus2.ringmod, fxBus2.parameter[0]);
		break;

	case FX2_COMPRESSOR:
		Compressor_setAttack(&fxBus2.compressor, fxBus2.parameter[0]);
		Compressor_setRelease(&fxBus2.compressor, fxBus2.parameter[1]);
		Compressor_setTreshold(&fxBus2.compressor, fxBus2.parameter[2]);
		Compressor_setGain(&fxBus2.compressor, fxBus2.parameter[3]);
		break;

	case FX2_OFF:
	default:
		break;
	}
}

void FxBus2_setFromParam(uint16_t paramNr, uint8_t value)
{
	switch(paramNr)
	{
	case PAR_FX2_TYPE:
		fxBus2.fxType = value < FX2_NUM_TYPES ? value : FX2_OFF;
		FxBus2_updateParameter();
		break;

	case PAR_FX2_ROUTING:
		// bus B can only output to the 6 hardware outs, never to an FX bus
		fxBus2.audioRouting = value <= MIXER_ROUTING_DAC2_R ? value : MIXER_ROUTING_DAC1_STEREO;
		break;

	case PAR_FX2_DRY_WET:
		fxBus2.dryWet = (value > 100 ? 100 : value) / 100.f;
		break;

	case PAR_FX2_RATIO:
		Compressor_setRatio(&fxBus2.compressor, value);
		break;

	case PAR_FX2_P1:
	case PAR_FX2_P2:
	case PAR_FX2_P3:
	case PAR_FX2_P4:
		fxBus2.parameter[paramNr - PAR_FX2_P1] = value / 127.f;
		FxBus2_updateParameter();
		break;

	default:
		break;
	}
}

void FxBus2_loadFromParameters(const uint8_t *params)
{
	// routing, mix and ratio first, type last so the type switch picks up current values
	FxBus2_setFromParam(PAR_FX2_ROUTING, params[PAR_FX2_ROUTING]);
	FxBus2_setFromParam(PAR_FX2_DRY_WET, params[PAR_FX2_DRY_WET]);
	FxBus2_setFromParam(PAR_FX2_RATIO, params[PAR_FX2_RATIO]);
	for(uint16_t p = PAR_FX2_P1; p <= PAR_FX2_P4; p++)
	{
		fxBus2.parameter[p - PAR_FX2_P1] = params[p] / 127.f;
	}
	FxBus2_setFromParam(PAR_FX2_TYPE, params[PAR_FX2_TYPE]);
}
