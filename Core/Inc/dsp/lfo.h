/**
  ******************************************************************************
  * @file    lfo.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the lfo.c
  ******************************************************************************
**/

#include "parameters.h"

#ifndef INC_DSP_LFO_H_
#define INC_DSP_LFO_H_

/* ========== Base structure ========== */
typedef struct {
	float sr;
	float f;
	float sample_value;
	float phase_increment;
	float phase_value;
	enum Waveform waveform;
} Lfo;

/* ========== Exported functions ========== */
void 	setupLfo    	(Lfo *lfo, float sr);
void 	setLfoFrequency	(Lfo *lfo, float f);
void 	setLfoWaveform	(Lfo *lfo, int waveform);
void 	getLfoAudioBlock(Lfo *lfo, float *out_buffer);
float 	getLfoSample	(Lfo *lfo);

#endif /* INC_DSP_LFO_H_ */

