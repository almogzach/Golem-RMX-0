/*
 * HiHat.c
 *
 *  Created on: 18.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */



#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/DSPAudio/HiHat.h"
#include "Soundengine/DSPAudio/squareRootLut.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"
#include "IO/adcPots.h"
#include "Soundengine/FxBus.h"
#include "Soundengine/DSPAudio/lxr0_declick.h"

INDTCMZ HiHatVoice hatVoice;
// MODIFIED: separate open hi-hat voice (voice 7). Kept in normal RAM so the
// DTCM layout of the original firmware is unchanged.
HiHatVoice hatVoice2;
uint8_t hatVoice2_choke = 1;

//---------------------------------------------------
void HiHat_setPan(const uint8_t pan)
{
	//hatVoice.panL = squareRootLut[127-pan];
	//hatVoice.panR = squareRootLut[pan];
	hatVoice.pan = pan;
}
//---------------------------------------------------
void HiHat2_setPan(const uint8_t pan)
{
	hatVoice2.pan = pan;
}
//---------------------------------------------------
static void hh_init(HiHatVoice* h)
{
	SnapEg_init(&h->snapEg);
	h->pan = 0;
	h->vol = 0.8f;
	h->cvFreq = 0.5f;

	h->decayClosed = 0.002f;
	h->decayOpen = 0.002f;

	transient_init(&h->transGen);

	h->fmModAmount1 = 0.5f;
	h->fmModAmount2 = 0.5f;

	setDistortionShape(&h->distortion, 0.f);

	h->modOsc.freq = 440;
	h->modOsc.waveform = SINE;
	h->modOsc.fmMod = 0;
	h->modOsc.midiFreq = 70<<8 | 64 ;
	h->modOsc.pitchMod = 1.0f;
	h->modOsc.modNodeValue = 1;
	h->modOsc.cvFreq = -1.f;

	h->modOsc2.freq = 440;
	h->modOsc2.waveform = NOISE;//SINE;
	h->modOsc2.fmMod = 0;
	h->modOsc2.midiFreq = 70<<8 | 64;
	h->modOsc2.pitchMod = 1.0f;
	h->modOsc2.modNodeValue = 1;
	h->modOsc2.cvFreq = -1.f;

	h->osc.freq = 440;
#if LXR0_SAMPLES
	osc_setSampleTrim(&h->osc, 5);	// GOLEM-RMX-0: both hats use voice 6's sample trims
#endif
	h->osc.waveform = 1;
	h->osc.fmMod = 1;
	h->osc.midiFreq = 70<<8 | 64;
	h->osc.pitchMod = 1.0f;
	h->osc.modNodeValue = 1;
	h->osc.cvFreq = -1.f;

	h->volumeMod = 1;

	slopeEg2_init(&h->oscVolEg);

	SVF_init(&h->filter);

	lfo_init(&h->lfo);
}

void HiHat_init()
{
	hh_init(&hatVoice);
	hh_init(&hatVoice2);	// MODIFIED: voice 7
	hatVoice2_choke = 1;
}

//---------------------------------------------------
static void hh_updatePan(HiHatVoice* h)
{
	h->triggeredPanL = sqrtf((127-h->pan)/127.f);
	h->triggeredPanR = sqrtf(h->pan/127.f);
}
void HiHat_updatePan()
{
	hh_updatePan(&hatVoice);
}
//---------------------------------------------------
void HiHat_setCv1VOct(float oct)
{
	// CV pitch input of voice 6 (closed hat) only
	hatVoice.osc.cvFreq		= oct;		// the main fm oscillator
	hatVoice.modOsc.cvFreq	= oct;		// the mod osc
	hatVoice.modOsc2.cvFreq	= oct;		// the mod osc
	hatVoice.transGen.cvFreq = oct;
}
//---------------------------------------------------
static void hh_trigger(HiHatVoice* h, uint8_t vel, uint8_t isOpen, const uint8_t note)
{
#if LXR0_DECLICK
	// RMX-0: the new note starts from a closed envelope, as it would from silence.
	// GOLEM-RMX-0: each hat has its own declick slot (5 = closed, 6 = open / voice 7)
	if(lxr0Declick_arm(h == &hatVoice2 ? 6 : 5))
		h->oscVolEg.value = 0;
#endif

	//update velocity modulation
#if OLD_LFO_ROUTING
	modNode_updateValue(&velocityModulators[5],vel/127.f);
#else
	h->velocity = vel/127.f;
#endif
	float offset = 1;
	if(h->transGen.waveform==1) //offset mode
	{
		offset -= h->transGen.volume;
	}
	if(h->osc.waveform == SINE)
		h->osc.phase = (0x3ff<<20)*offset;//voiceArray[voiceNr].osc.startPhase ;
	else if(h->osc.waveform > SINE && h->osc.waveform <= REC)
		h->osc.phase = (0xff<<22)*offset;
	else
		h->osc.phase = 0;

	h->modOsc.phase = 0;
	h->modOsc2.phase = 0;
	OSC_RESTART_SAMPLE(h->osc); // RMX-0
	OSC_RESTART_SAMPLE(h->modOsc);
	OSC_RESTART_SAMPLE(h->modOsc2);

	osc_setBaseNote(&h->osc,note);
	osc_setBaseNote(&h->modOsc,note);
	osc_setBaseNote(&h->modOsc2,note);

	h->isOpen = isOpen;
	h->oscVolEg.decay = isOpen?h->decayOpen:h->decayClosed;

	slopeEg2_trigger(&h->oscVolEg);
	h->velo = vel/127.f;
	transient_trigger(&h->transGen);

	SnapEg_trigger(&h->snapEg);

	hh_updatePan(h);
}
//---------------------------------------------------
// MODIFIED: fast fade-out of the open hat when the closed hat chokes it
static void hh_choke(HiHatVoice* h)
{
	if(h->oscVolEg.state != EG_STOPPED)
	{
		h->oscVolEg.state = EG_D;
		h->oscVolEg.decay = slopeEg2_calcDecay(0);
	}
}
//---------------------------------------------------
void HiHat_trigger( uint8_t vel, uint8_t isOpen, const uint8_t note)
{
	lfo_retrigger(5);

	if(isOpen)
	{
		// MODIFIED: voice 7 is its own voice
		hh_trigger(&hatVoice2, vel, 1, note);
	}
	else
	{
		hh_trigger(&hatVoice, vel, 0, note);
		if(hatVoice2_choke)
		{
			hh_choke(&hatVoice2);
		}
	}
}
//---------------------------------------------------
static void __attribute__((noinline)) hh_calcAsync(HiHatVoice* h)
{
	//calc the osc  vol eg
	h->egValueOscVol = slopeEg2_calc(&h->oscVolEg);

	//calc snap EG if transient sample 0 is activated
	if(h->transGen.waveform == 0)
	{
		const float snapVal = SnapEg_calc(&h->snapEg, h->transGen.pitch);
		h->osc.pitchMod = 1 + snapVal*h->transGen.volume;
	}

	osc_setFreq(&h->osc);
	osc_setFreq(&h->modOsc);
	osc_setFreq(&h->modOsc2);
}
// MODIFIED: no longer IN_ITCM_RAM, so the second voice does not grow the 16 KB ITCM
void HiHat_calcAsync( )
{
	hh_calcAsync(&hatVoice);
	hh_calcAsync(&hatVoice2);
}
//---------------------------------------------------
static void hh_calcSyncBlock(HiHatVoice* h, int16_t* buf, const uint8_t size)
{
	//2 buffers for the mod oscs
	int16_t mod1[size],mod2[size];
	//calc next mod osc samples, scaled with mod amount
	calcNextOscSampleBlock(&h->modOsc,mod1,size, h->fmModAmount1, h->pwm);
	calcNextOscSampleBlock(&h->modOsc2,mod2,size,  h->fmModAmount2, h->pwm);

	//combine both mod oscs to 1 modulation signal
	bufferTool_addBuffersSaturating(mod1,mod2,size);

	calcNextOscSampleFmBlock(&h->osc,mod1,buf,size,0.5f, h->pwm) ;

#if USE_REVERB
	if(fxBus.fxType != FX_REVERB)
	{
#endif
		SVF_calcBlockZDF(&h->filter,h->filterType,buf,size);

#if USE_REVERB
	}
	else
	{
		SVF_calcBlockCheapZDF(&h->filter,h->filterType,buf,size);
	}
#endif
	//calc transient sample
	transient_calcBlock(&h->transGen,mod1,size);

	uint8_t j;
	if(h->volumeMod)
	{
		h->combinedVolume = h->vol * h->velo * h->egValueOscVol;
	}
	else
	{
		h->combinedVolume = h->vol * h->egValueOscVol;
	}

	int32_t* x = (int32_t*)buf;
	int32_t* y = (int32_t*)mod1;

	//add filter to buffer
	for(uint8_t i=0;i<size/2;i++)
	{
		x[i] = __QADD16(x[i],y[i]);
	}

	for(j=0;j<size;j++)
	{
		//add volume
		buf[j] *=  h->combinedVolume ;
	}

	calcDistBlock(&h->distortion,buf,size);
}

void HiHat_calcSyncBlock(int16_t* buf, const uint8_t size)
{
	hh_calcSyncBlock(&hatVoice, buf, size);
}

// MODIFIED: voice 7 (open hat)
void HiHat2_calcSyncBlock(int16_t* buf, const uint8_t size)
{
	hh_calcSyncBlock(&hatVoice2, buf, size);
}
//---------------------------------------------------

