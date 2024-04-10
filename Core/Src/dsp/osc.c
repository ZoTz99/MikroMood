/**
  ******************************************************************************
  * @file    osc.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and function of the osc.
  ******************************************************************************
**/

#include "dsp/osc.h"

/* ========== Constructor ==========*/
void setupOsc(Osc *osc, float sr) {
	osc->sr = sr;
	osc->f = 0;
	osc->sample_value = 0;
	osc->waveform = DEFAULT_WF;
	setupBlit(&osc->blit, sr);
	setupLfo(&osc->lfo, sr);
}

/* ========== Parameters ==========*/
void setOscWaveform(Osc *osc, int waveform) {
	osc->waveform = waveform;
	setLfoWaveform(&osc->lfo, waveform);
}

void setOscFrequency(Osc *osc, float frequency) {
	osc->f = frequency;
	setLfoFrequency(&osc->lfo, frequency);
}

/* ========== Processing ==========*/
void getOscAudioBlock(Osc *osc, float *fm_buffer, float *out_buffer) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		setOscFrequency(osc, fm_buffer[i]);
		out_buffer[i] = getOscSample(osc);
	}
}

float getOscSample(Osc *osc) {
	if(osc->f <= 20) {
		osc->sample_value = getLfoSample(&osc->lfo);
	} else {
		osc->sample_value = getBlitSample(&osc->blit, osc->f, osc->waveform);
	}
	return osc->sample_value;
}

void clearOscAccumulators(Osc *osc) {
	clearBlitAccumulators(&osc->blit);
}


