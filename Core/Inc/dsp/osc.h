/**
  ******************************************************************************
  * @file    osc.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the osc.c
  ******************************************************************************
**/

#include "parameters.h"
#include "dsp/blit.h"
#include "dsp/lfo.h"

#ifndef INC_DSP_OSC_H_
#define INC_DSP_OSC_H_

/* ========== Base structure ========== */
typedef struct {
	Blit blit;
	Lfo lfo;
	float sr;
	float f;
	float sample_value;
	enum Waveform waveform;
} Osc;

/* ========== Exported functions ========== */
void setupOsc				(Osc *osc, float sr);
void setOscWaveform			(Osc *osc, int waveform);
void setOscFrequency		(Osc *osc, float frequency);
void getOscAudioBlock		(Osc *osc, float *fm_buffer, float *out_buffer);
float getOscSample			(Osc *osc);
void clearOscAccumulators	(Osc *osc);

#endif /* INC_DSP_OSC_H_ */


