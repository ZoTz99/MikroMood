/**
  ******************************************************************************
  * @file    mixer.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the mixer.c
  ******************************************************************************
**/

#include "parameters.h"

#ifndef INC_DSP_MIXER_H_
#define INC_DSP_MIXER_H_

/* ========== Base structure ========== */
typedef struct {
	float sr;
	float gain_osc1;
	float gain_osc2;
	bool mute_osc1;
	bool mute_osc2;
} Mixer;

/* ========== Exported functions ========== */
void setupMixer(Mixer *mixer, float sr);
void setMuteOsc1(Mixer *mixer, bool mute1);
void setMuteOsc2(Mixer *mixer, bool mute2);
void setGainOsc1(Mixer *mixer, float gain1);
void setGainOsc2(Mixer *mixer, float gain2);
void getMixerAudioBlock(Mixer *mixer, float *out_buffer, float *buffer_osc1, float *buffer_osc2);

#endif /* INC_DSP_MIXER_H_ */
