/**
  ******************************************************************************
  * @file    lfo.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and function of the lfo.
  ******************************************************************************
**/

#include "dsp/lfo.h"

/* ========== Constructor ==========*/
void setupLfo(Lfo *lfo, float sr) {
	lfo->sr 				= sr;
	lfo->f 					= DEFAULT_RATE_LFO;
	lfo->sample_value 		= 0.0f;
	lfo->phase_increment	= lfo->f/lfo->sr;
	lfo->phase_value 		= 0.0f;
	lfo->waveform 			= DEFAULT_WF_LFO;
}

/* ========== Parameters ==========*/
void setLfoWaveform(Lfo *lfo, int waveform) {
	lfo->waveform = waveform;
}

void setLfoFrequency(Lfo *lfo, float f) {
	lfo->f = f;
}

/* ========== Processing ==========*/
void getLfoAudioBlock(Lfo *lfo, float *out_buffer) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		out_buffer[i] = getLfoSample(lfo);
	}
}

float getLfoSample(Lfo *lfo) {
	switch (lfo->waveform) {
		case TRIANGLE:
			lfo->sample_value = 4.0f * fabs(lfo->phase_value - 0.5f) - 1.0;
			break;
		case SAWTOOTH:
			lfo->sample_value = 2.0f * lfo->phase_value - 1.0f;
			break;
		case SQUARE:
			lfo->sample_value = (lfo->phase_value > 0.5f) - (lfo->phase_value < 0.5f);
			break;
		default:
			lfo->sample_value = 0.0f;
	}
	lfo->phase_increment = lfo->f / lfo->sr;
	lfo->phase_value += lfo->phase_increment;
	lfo->phase_value -= (int) lfo->phase_value;
	return lfo->sample_value;
}
