/**
  ******************************************************************************
  * @file    adsr.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and function of the adsr.
  ******************************************************************************
**/

#include "dsp/adsr.h"

/* ========== Constructor ========== */
void setupAdsr(Adsr *adsr, float sr) {
	adsr->sr = sr;
	adsr->attack = DEFAULT_ATTACK;
	adsr->release = DEFAULT_RELEASE;
	adsr->sample_value = 0.0f;
	adsr->step = 0.0f;
	adsr->env = 0.0f;
	adsr->reset_voice = 0;
	adsr->state = ADSR_IDLE;
}

/* ========== Setters ========== */
void setAdsrAttack(Adsr *adsr, float attack) {
	adsr->attack = attack * 0.000244f + DEFAULT_ATTACK;
}
void setAdsrRelease(Adsr *adsr, float release) {
	adsr->release = release * 0.000244f + DEFAULT_RELEASE;
}

/* =========== Midi ============ */
void adsrNoteOn(Adsr *adsr) {
	adsr->step = 1/(adsr->attack * adsr->sr);	// How much to increment the envelope every sample
	adsr->state = ADSR_ATTACK;
}
void adsrNoteOff(Adsr *adsr) {
	adsr->step = 1/(adsr->release * adsr->sr);	// How much to decrement the envelope every sample
	adsr->state = ADSR_RELEASE;
}

/* ======== Processing ========= */
void getAdsrAudioBlock(Adsr *adsr, float *out_buffer) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		out_buffer[i] *= getAdsrEnvelope(adsr);
	}
}
float getAdsrEnvelope(Adsr *adsr) {
	switch(adsr->state) {
		case ADSR_IDLE:
			adsr->env = 0.0f;
		break;
		case ADSR_ATTACK:
			adsr->env += adsr->step;
			if (adsr->env >= 1.0f) {
				adsr->state = ADSR_SUSTAIN;
			}
		break;
		case ADSR_RELEASE:
			adsr->env -= adsr->step;
			if (adsr->env <= 0.0f) {
				adsr->state = ADSR_IDLE;
				adsr->reset_voice = 1;
			}
		break;
		case ADSR_SUSTAIN:
			adsr->env = 1.0f;
		break;
		default:
			adsr->env = 0.0f;
	}
	return adsr->env;
}
