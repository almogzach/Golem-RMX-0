/*
 * randomizer.c
 *
 * Parameter randomization for the LXR V2 Eurorack module.
 * MODIFIED VERSION - not official Erica Synths firmware.
 *
 * Which parameters belong to a voice is read from that voice's menu pages
 * (menuPages[VOICEx_PAGE]), so the randomizer always matches what the user can
 * edit on that voice, including the separate open hi-hat (voice 7).
 *
 * Runs in the UI, not in the audio interrupt. After changing the edit buffer it
 * re-sends the kit the same way loading a kit does.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * This file is part of a modified LXR firmware, licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include <string.h>
#include "menu/randomizer.h"
#include "menu/menu.h"
#include "Parameters.h"

#ifndef RANDOMIZER_HOST_TEST
#include "Preset/PresetManager.h"
#include "Soundengine/DSPAudio/random.h"
#endif

#define NUM_VOICE_PAGES 7

static uint8_t randomizer_undoBuffer[END_OF_SOUND_PARAMETERS];
static uint8_t randomizer_undoValid = 0;

static uint8_t voiceParams_mask[END_OF_SOUND_PARAMETERS];
static uint8_t voiceParams_ready = 0;

//-----------------------------------------------------------------------------
static void voiceParams_build(void)
{
	memset(voiceParams_mask, 0, sizeof(voiceParams_mask));
	for(uint8_t v = 0; v < NUM_VOICE_PAGES; v++)
	{
		for(uint8_t sub = 0; sub < NUM_SUB_PAGES; sub++)
		{
			for(uint8_t slot = 0; slot < 8; slot++)
			{
				const uint16_t p = pgm_read_word(&menuPages[VOICE1_PAGE + v][sub].bot1 + slot);
				if(p == PAR_NONE || p >= END_OF_SOUND_PARAMETERS) continue;
				// voice 7's LFO page edits voice 6's LFO
				if((v == 6) && ((p == PAR_FREQ_LFO6) || (p == PAR_OFFSET_LFO6) || (p == PAR_WAVE_LFO6) || (p == PAR_RETRIGGER_LFO6)))
					continue;
				voiceParams_mask[p] |= (uint8_t)(1 << v);
			}
		}
	}
	voiceParams_ready = 1;
}
//-----------------------------------------------------------------------------
uint8_t voiceParams_getVoiceMask(uint16_t paramNr)
{
	if(paramNr >= END_OF_SOUND_PARAMETERS) return 0;
	if(!voiceParams_ready) voiceParams_build();
	return voiceParams_mask[paramNr];
}
//-----------------------------------------------------------------------------
uint8_t voiceParams_isMorphed(uint16_t paramNr, uint8_t morphTargetKitValid7)
{
	// routing and choke always stay as the current kit sets them
	if((paramNr >= PAR_AUDIO_OUT1) && (paramNr <= PAR_AUDIO_OUT6)) return 0;
	if((paramNr == PAR_HH7_AUDIO_OUT) || (paramNr == PAR_HH7_CHOKE) || (paramNr == PAR_HH7_VALID)) return 0;

	// voice 7 settings: only morph toward a kit that was saved with its own voice 7
	if((paramNr >= PAR_HH7_COARSE) && (paramNr <= PAR_HH7_CHOKE) && !morphTargetKitValid7) return 0;

	const uint8_t mv = activeParameters[PAR_MORPH_VOICE];
	if(mv == 0 || mv > NUM_VOICE_PAGES) return 1;		// all voices: unchanged original behaviour
	return (voiceParams_getVoiceMask(paramNr) >> (mv - 1)) & 1;
}

//-----------------------------------------------------------------------------
void randomizer_init(void)
{
	activeParameters[PAR_RAND_AMOUNT] = 50;
	activeParameters[PAR_RAND_TARGET] = RANDOMIZER_TARGET_KIT;
	activeParameters[PAR_RAND_GO] = 0;
	activeParameters[PAR_RAND_UNDO] = 0;
	activeParameters[PAR_MORPH_VOICE] = 0;	// morph all voices, as before
	randomizer_undoValid = 0;
	voiceParams_build();
}
//-----------------------------------------------------------------------------
// full value range of a parameter from its display type. returns 0 if the type
// is not something the randomizer should touch (mod targets, note names...)
static uint8_t dtypeRange(uint16_t paramNr, uint8_t* min, uint8_t* max, uint8_t* isDiscrete)
{
	const uint16_t dtype = parameter_dtypes[paramNr];
	*isDiscrete = 0;
	*min = 0;

	switch(dtype & 0xFF)
	{
	case DTYPE_0B127:
	case DTYPE_PM63:	*max = 127;							return 1;
	case DTYPE_0B100:	*max = 100;							return 1;
	case DTYPE_PM100:	*max = 200;							return 1;
	case DTYPE_0B255:	*max = 255;							return 1;
	case DTYPE_0B15:	*max = 15;							return 1;
	case DTYPE_1B16:	*min = 1; *max = 16;				return 1;
	case DTYPE_1B64:	*min = 1; *max = 64;				return 1;
	case DTYPE_ON_OFF:
	case DTYPE_0b1:		*max = 1; *isDiscrete = 1;			return 1;
	case DTYPE_MENU:
	{
		const uint8_t n = menu_getMaxEntriesForMenu((uint8_t)(dtype >> 8));
		if(n < 2) return 0;
		*max = (uint8_t)(n - 1);
		*isDiscrete = 1;
		return 1;
	}
	default:
		return 0;
	}
}
//-----------------------------------------------------------------------------
uint8_t randomizer_getBounds(uint16_t paramNr, uint8_t* min, uint8_t* max, uint8_t* isDiscrete)
{
	if(paramNr == PAR_NONE || paramNr >= END_OF_SOUND_PARAMETERS) return 0;

	// never touched: the FX buses, morph and the CV mod matrix are not part of a voice's sound
	if((paramNr >= PAR_FX_TYPE) && (paramNr <= PAR_MORPH_KIT_DEST)) return 0;
	if((paramNr >= PAR_CV_DEST_1) && (paramNr <= PAR_CV_SRC_18)) return 0;
	if((paramNr >= PAR_FX2_TYPE) && (paramNr <= PAR_FX2_P4)) return 0;

	switch(paramNr)
	{
	// never touched: levels, routing, choke and velocity behaviour stay as the user set them
	case PAR_VOL1: case PAR_VOL2: case PAR_VOL3: case PAR_VOL4: case PAR_VOL5: case PAR_VOL6: case PAR_HH7_VOL:
	case PAR_AUDIO_OUT1: case PAR_AUDIO_OUT2: case PAR_AUDIO_OUT3: case PAR_AUDIO_OUT4: case PAR_AUDIO_OUT5: case PAR_AUDIO_OUT6:
	case PAR_HH7_AUDIO_OUT: case PAR_HH7_CHOKE: case PAR_HH7_VALID:
	case PAR_VOLUME_MOD_ON_OFF1: case PAR_VOLUME_MOD_ON_OFF2: case PAR_VOLUME_MOD_ON_OFF3:
	case PAR_VOLUME_MOD_ON_OFF4: case PAR_VOLUME_MOD_ON_OFF5: case PAR_VOLUME_MOD_ON_OFF6: case PAR_HH7_VOLUME_MOD:
		return 0;
	default:
		break;
	}

	if(!dtypeRange(paramNr, min, max, isDiscrete)) return 0;

	// musical limits on top of the full range
	switch(paramNr)
	{
	// short attacks: these are drums
	case PAR_VELOA1: case PAR_VELOA2: case PAR_VELOA3: case PAR_VELOA4: case PAR_VELOA5: case PAR_VELOA6: case PAR_HH7_ATTACK:
		*min = 0; *max = 25; break;

	// decays: drum length, closed hat short, open hat longer
	case PAR_VELOD1: case PAR_VELOD2: case PAR_VELOD3:	*min = 25; *max = 100; break;
	case PAR_VELOD4: case PAR_VELOD5:					*min = 15; *max = 90; break;
	case PAR_VELOD6_CLOSED:								*min = 5;  *max = 55; break;
	case PAR_HH7_DECAY:									*min = 35; *max = 110; break;

	// pitch ranges per voice type (value = note number)
	case PAR_COARSE1:									*min = 20; *max = 55; break;
	case PAR_COARSE2: case PAR_COARSE3:					*min = 30; *max = 80; break;
	case PAR_COARSE4:									*min = 40; *max = 85; break;
	case PAR_COARSE5:									*min = 50; *max = 110; break;
	case PAR_COARSE6: case PAR_HH7_COARSE:				*min = 60; *max = 120; break;
	case PAR_FINE1: case PAR_FINE2: case PAR_FINE3: case PAR_FINE4: case PAR_FINE5: case PAR_FINE6: case PAR_HH7_FINE:
		*min = 48; *max = 80; break;

	// filters: keep them open enough to hear, no self-oscillation
	case PAR_FILTER_FREQ_1: case PAR_FILTER_FREQ_2: case PAR_FILTER_FREQ_3:
	case PAR_FILTER_FREQ_4: case PAR_FILTER_FREQ_5: case PAR_FILTER_FREQ_6: case PAR_HH7_FILTER_FREQ:
		*min = 30; *max = 127; break;
	case PAR_RESO_1: case PAR_RESO_2: case PAR_RESO_3: case PAR_RESO_4: case PAR_RESO_5: case PAR_RESO_6: case PAR_HH7_RESO:
		*min = 0; *max = 100; break;

	// drive and distortion: some grit, not wall-of-noise
	case PAR_DRIVE1: case PAR_DRIVE2: case PAR_DRIVE3:
	case PAR_SNARE_DISTORTION: case PAR_CYMBAL_DISTORTION: case PAR_HAT_DISTORTION: case PAR_HH7_DISTORTION:
	case PAR_FILTER_DRIVE_1: case PAR_FILTER_DRIVE_2: case PAR_FILTER_DRIVE_3:
	case PAR_FILTER_DRIVE_4: case PAR_FILTER_DRIVE_5: case PAR_FILTER_DRIVE_6: case PAR_HH7_FILTER_DRIVE:
		*min = 0; *max = 80; break;

	// sample-rate reduction: mostly full rate
	case PAR_VOICE_DECIMATION1: case PAR_VOICE_DECIMATION2: case PAR_VOICE_DECIMATION3:
	case PAR_VOICE_DECIMATION4: case PAR_VOICE_DECIMATION5: case PAR_VOICE_DECIMATION6: case PAR_HH7_DECIMATION:
		*min = 100; *max = 127; break;

	// pan: stay near the centre
	case PAR_PAN1: case PAR_PAN2: case PAR_PAN3: case PAR_PAN4: case PAR_PAN5: case PAR_PAN6: case PAR_HH7_PAN:
		*min = 40; *max = 88; break;

	// transient (click) level
	case PAR_TRANS1_VOL: case PAR_TRANS2_VOL: case PAR_TRANS3_VOL:
	case PAR_TRANS4_VOL: case PAR_TRANS5_VOL: case PAR_TRANS6_VOL: case PAR_HH7_TRANS_VOL:
		*min = 0; *max = 110; break;

	default:
		break;
	}
	return 1;
}
//-----------------------------------------------------------------------------
uint8_t randomizer_isRandomized(uint16_t paramNr, uint8_t voice)
{
	uint8_t min, max, isDiscrete;

	// voice 7 has no LFO of its own: its LFO page edits voice 6's LFO, leave that alone
	if(voice == 6)
	{
		if( (paramNr == PAR_FREQ_LFO6) || (paramNr == PAR_OFFSET_LFO6) ||
			(paramNr == PAR_WAVE_LFO6) || (paramNr == PAR_RETRIGGER_LFO6) )
			return 0;
	}
	return randomizer_getBounds(paramNr, &min, &max, &isDiscrete);
}
//-----------------------------------------------------------------------------
uint8_t randomizer_mutate(uint8_t current, uint8_t min, uint8_t max, uint8_t isDiscrete, uint8_t amount, uint32_t rnd)
{
	if(amount > 100) amount = 100;
	const uint32_t span = (uint32_t)(max - min) + 1;
	const uint8_t target = (uint8_t)(min + ((rnd >> 8) % span));

	if(isDiscrete)
	{
		// switch to a random choice with a probability equal to the amount
		return ((rnd & 0xFF) % 100) < amount ? target : current;
	}

	// move from the current value toward a random value inside the musical range
	const int32_t diff = (int32_t)target - (int32_t)current;
	int32_t v = (int32_t)current + (diff * amount + (diff >= 0 ? 50 : -50)) / 100;
	if(v < 0) v = 0;
	if(v > 255) v = 255;
	return (uint8_t)v;
}
//-----------------------------------------------------------------------------
#ifndef RANDOMIZER_HOST_TEST
static void randomizer_voice(uint8_t voice, uint8_t amount, uint8_t* done)
{
	for(uint8_t sub = 0; sub < NUM_SUB_PAGES; sub++)
	{
		for(uint8_t slot = 0; slot < 8; slot++)
		{
			const uint16_t paramNr = pgm_read_word(&menuPages[VOICE1_PAGE + voice][sub].bot1 + slot);
			if(paramNr == PAR_NONE || paramNr >= END_OF_SOUND_PARAMETERS) continue;
			if(done[paramNr >> 3] & (1 << (paramNr & 7))) continue;	// already changed (shared params)
			if(!randomizer_isRandomized(paramNr, voice)) continue;

			uint8_t min, max, isDiscrete;
			randomizer_getBounds(paramNr, &min, &max, &isDiscrete);
			activeParameters[paramNr] = randomizer_mutate(activeParameters[paramNr], min, max, isDiscrete, amount, GetRngValue());
			done[paramNr >> 3] |= (uint8_t)(1 << (paramNr & 7));
		}
	}
}
//-----------------------------------------------------------------------------
void randomizer_run(uint8_t target, uint8_t amount)
{
	uint8_t done[(END_OF_SOUND_PARAMETERS + 7) / 8];
	memset(done, 0, sizeof(done));

	memcpy(randomizer_undoBuffer, activeParameters, END_OF_SOUND_PARAMETERS);
	randomizer_undoValid = 1;

	if(target >= RANDOMIZER_TARGET_KIT)
	{
		for(uint8_t v = 0; v < NUM_VOICE_PAGES; v++)
			randomizer_voice(v, amount, done);
	}
	else
	{
		randomizer_voice(target, amount, done);
	}

	// apply exactly like a kit load: updates every voice, FX bus B and voice 7
	preset_sendDrumsetParameters();
	menu_repaintAll();
}
//-----------------------------------------------------------------------------
uint8_t randomizer_undo(void)
{
	if(!randomizer_undoValid) return 0;
	memcpy(activeParameters, randomizer_undoBuffer, END_OF_SOUND_PARAMETERS);
	randomizer_undoValid = 0;
	preset_sendDrumsetParameters();
	menu_repaintAll();
	return 1;
}
#endif
