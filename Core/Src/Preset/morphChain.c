/*
 * morphChain.c
 *
 * Morph source selection, glitch guard, auto-advance and CV kit selection
 * for the LXR V2 Eurorack module. See morphChain.h.
 * MODIFIED VERSION ("Golem") - not official Erica Synths firmware.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 * This file is part of a modified LXR firmware, licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Preset/morphChain.h"
#include "menu/menu.h"
#include "Parameters.h"

#ifndef MORPHCHAIN_HOST_TEST
#include "Preset/PresetManager.h"
#include "IO/adcPots.h"
#else
extern uint8_t preset_morphValue;
float adc_getCvValue(uint8_t idx);
void preset_morph(uint8_t morph, uint8_t forceResetIndex);
#endif

static uint8_t morphChain_loaded = 0;		// kit in the edit buffer
static uint8_t morphChain_src = 0;			// effective source kit
static uint8_t morphChain_tgt = 0;			// effective target kit
static uint8_t morphChain_ready = 0;

//-----------------------------------------------------------------------------
static uint8_t clampKit(uint8_t k)
{
	return k < MORPHCHAIN_NUM_KITS ? k : (MORPHCHAIN_NUM_KITS - 1);
}
//-----------------------------------------------------------------------------
static void getRange(uint8_t* lo, uint8_t* hi)
{
	uint8_t a = clampKit(activeParameters[PAR_MORPH_LO]);
	uint8_t b = clampKit(activeParameters[PAR_MORPH_HI]);
	if(a > b) { uint8_t t = a; a = b; b = t; }
	*lo = a;
	*hi = b;
}
//-----------------------------------------------------------------------------
void morphChain_setLoadedKit(uint8_t kit)
{
	kit = clampKit(kit);
	morphChain_loaded = kit;
	morphChain_src = kit;
	activeParameters[PAR_MORPH_SOURCE] = kit;
	if(!morphChain_ready)
	{
		morphChain_tgt = clampKit(activeParameters[PAR_MORPH_TARGET]);
		morphChain_ready = 1;
	}
}
//-----------------------------------------------------------------------------
uint8_t morphChain_loadedKit(void)			{ return morphChain_loaded; }
uint8_t morphChain_sourceKit(void)			{ return morphChain_src; }
uint8_t morphChain_targetKit(void)			{ return morphChain_ready ? morphChain_tgt : clampKit(activeParameters[PAR_MORPH_TARGET]); }
uint8_t morphChain_sourceIsEditBuffer(void)	{ return morphChain_src == morphChain_loaded; }
//-----------------------------------------------------------------------------
uint8_t morphChain_nextKit(uint8_t kit, uint8_t lo, uint8_t hi)
{
	if(lo > hi) { uint8_t t = lo; lo = hi; hi = t; }
	if(kit < lo || kit >= hi) return lo;
	return (uint8_t)(kit + 1);
}
//-----------------------------------------------------------------------------
uint8_t morphChain_sync(uint8_t morph)
{
	uint8_t changed = 0;
	uint8_t lo, hi;
	getRange(&lo, &hi);

	if(!morphChain_ready)
	{
		morphChain_src = clampKit(activeParameters[PAR_MORPH_SOURCE]);
		morphChain_tgt = clampKit(activeParameters[PAR_MORPH_TARGET]);
		morphChain_ready = 1;
		changed = 1;
	}

	const uint8_t autoOn = activeParameters[PAR_MORPH_AUTO] != 0;

	if(MORPHCHAIN_AT_SOURCE(morph))
	{
		// the target is silent: it may change now
		if(autoOn && (activeParameters[PAR_MORPH_TGT_CV] == 0))
		{
			const uint8_t n = morphChain_nextKit(morphChain_src, lo, hi);
			if(n != morphChain_src) activeParameters[PAR_MORPH_TARGET] = n;
		}
		const uint8_t req = clampKit(activeParameters[PAR_MORPH_TARGET]);
		if(req != morphChain_tgt)
		{
			morphChain_tgt = req;
			changed = 1;
		}
	}
	else if(MORPHCHAIN_AT_TARGET(morph))
	{
		// the source is silent: it may change now
		if(autoOn && (activeParameters[PAR_MORPH_SRC_CV] == 0))
		{
			const uint8_t n = morphChain_nextKit(morphChain_tgt, lo, hi);
			if(n != morphChain_tgt) activeParameters[PAR_MORPH_SOURCE] = n;
		}
		const uint8_t req = clampKit(activeParameters[PAR_MORPH_SOURCE]);
		if(req != morphChain_src)
		{
			morphChain_src = req;
			changed = 1;
		}
	}
	return changed;
}
//-----------------------------------------------------------------------------
uint8_t morphChain_cvToKit(float cv, uint8_t current, uint8_t lo, uint8_t hi)
{
	if(lo > hi) { uint8_t t = lo; lo = hi; hi = t; }
	if(cv < 0.f) cv = 0.f;
	if(cv > 1.f) cv = 1.f;
	const uint8_t span = (uint8_t)(hi - lo);
	if(span == 0) return lo;

	const float pos = cv * span;	// 0..span
	if(current >= lo && current <= hi)
	{
		// dead zone: stay on the current kit until the CV is clearly nearer a neighbour
		float d = pos - (float)(current - lo);
		if(d < 0.f) d = -d;
		if(d <= 0.65f) return current;
	}
	uint8_t k = (uint8_t)(pos + 0.5f);
	if(k > span) k = span;
	return (uint8_t)(lo + k);
}
//-----------------------------------------------------------------------------
void morphChain_cvTick(void)
{
	uint8_t lo, hi;
	getRange(&lo, &hi);

	const uint8_t srcJack = activeParameters[PAR_MORPH_SRC_CV];
	const uint8_t tgtJack = activeParameters[PAR_MORPH_TGT_CV];

	if(srcJack >= 1 && srcJack <= 5)
	{
		activeParameters[PAR_MORPH_SOURCE] =
			morphChain_cvToKit(adc_getCvValue((uint8_t)(srcJack - 1)), activeParameters[PAR_MORPH_SOURCE], lo, hi);
	}
	if(tgtJack >= 1 && tgtJack <= 5)
	{
		activeParameters[PAR_MORPH_TARGET] =
			morphChain_cvToKit(adc_getCvValue((uint8_t)(tgtJack - 1)), activeParameters[PAR_MORPH_TARGET], lo, hi);
	}

	// applies pending kit changes (and auto-advance) as soon as their side is silent
	if(morphChain_sync(preset_morphValue))
	{
		preset_morph(preset_morphValue, 1);
	}
}
