/**
  ******************************************************************************
  * @file    adsr.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the adsr.c
  ******************************************************************************
**/

#include "parameters.h"

#ifndef INC_DSP_ADSR_H_
#define INC_DSP_ADSR_H_

/* ========== Base structure ========== */
typedef struct {
	float sr;
	float attack;
	float release;
	float sample_value;
	float step;
	float env;
	uint8_t reset_voice;
	enum AdsrState state;
} Adsr;

/* ========== Exported functions ========== */
void setupAdsr(Adsr *adsr, float sr);
void setAdsrAttack(Adsr *adsr, float attack);
void setAdsrRelease(Adsr *adsr, float release);
void adsrNoteOn(Adsr *adsr);
void adsrNoteOff(Adsr *adsr);
void getAdsrAudioBlock(Adsr *adsr, float *out_buffer);
float getAdsrEnvelope(Adsr *adsr);

#endif /* INC_DSP_ADSR_H_ */
