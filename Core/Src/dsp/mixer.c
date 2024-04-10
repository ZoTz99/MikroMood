/**
  ******************************************************************************
  * @file    mixer.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and function of the mixer.
  ******************************************************************************
**/

#include "dsp/mixer.h"

/* ========== Constructor ==========*/
void setupMixer(Mixer *mixer, float sr) {
	mixer->sr = sr;
	mixer->gain_osc1 = DEFAULT_GAIN;
	mixer->gain_osc2 = DEFAULT_GAIN;
	mixer->mute_osc1 = DEFAULT_MUTE_OSC_1;
	mixer->mute_osc2 = DEFAULT_MUTE_OSC_2;
}

/* ========== Parameters ==========*/
void setMuteOsc1(Mixer *mixer, bool mute1) {
	mixer->mute_osc1 = mute1;
	setGainOsc1(mixer, mixer->gain_osc1);
}

void setMuteOsc2(Mixer *mixer, bool mute2) {
	mixer->mute_osc2 = mute2;
	setGainOsc2(mixer, mixer->gain_osc2);
}

void setGainOsc1(Mixer *mixer, float gain1) {
	mixer->gain_osc1 = gain1 * 0.000244f * (mixer->mute_osc1);
}

void setGainOsc2(Mixer *mixer, float gain2) {
	mixer->gain_osc2 = gain2 * 0.000244f * (mixer->mute_osc2);
}

/* ========== Processing ==========*/
void getMixerAudioBlock(Mixer *mixer, float *out_buffer, float *buffer_osc1, float *buffer_osc2) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		buffer_osc1[i] *= mixer->gain_osc1;
		buffer_osc2[i] *= mixer->gain_osc2;
		out_buffer[i] = (buffer_osc1[i] + buffer_osc2[i]);
	}
}


