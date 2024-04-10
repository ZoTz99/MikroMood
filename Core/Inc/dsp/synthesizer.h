/**
  ******************************************************************************
  * @file    synthesizer.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the synthesizer.c
  ******************************************************************************
**/

#include "parameters.h"
#include "dsp/lfo.h"
#include "dsp/osc.h"
#include "dsp/mixer.h"
#include "dsp/filter.h"
#include "dsp/adsr.h"

#ifndef INC_DSP_SYNTHESIZER_H_
#define INC_DSP_SYNTHESIZER_H_

/* ========== Base structure ========== */
typedef struct {
	// Main Components
	Lfo lfo;
	Osc osc1;
	Osc osc2;
	Mixer mixer;
	Filter filter;
	Adsr adsr;

	// Buffers
	float buffer_osc1[BUFFER_SIZE];
	float buffer_osc2[BUFFER_SIZE];
	float am_buffer[BUFFER_SIZE];
	float fm_buffer_osc1[BUFFER_SIZE];
	float fm_buffer_osc2[BUFFER_SIZE];
	float fm_buffer_filter[BUFFER_SIZE];

	// Frequency and Slider value
	float sr;
	float gain;
	float velocity;

	// Modified by MIDI
	float midi_note;
	float hertz_note;
	float mod_wheel;
	float chn_vol;
	float pan_ctrl;
	float sustain_pedal;
	int note_counter;

	// Pitch adjustment
	float pitch_bend;
	float detune_osc2;
	float octave_osc1;
    float octave_osc2;

    // Filter cutoff
    float filter_cutoff;

    // Mixer
    float gain_osc1;
	float gain_osc2;
	float mute_osc1;
	float mute_osc2;

    // Modulations
    //float mod_amount = DEFAULT_MODULATION_WHEEL;
	bool is_tremolo_mod_on;
	bool is_vibrato_mod_on;
	bool is_filter_mod_on;
	bool is_gain_enabled;
} Synthesizer;

/* ========== Exported functions ========== */
// Constructor
void setupSynthesizer(Synthesizer *synth, float sr);
// "Getters"
void getFrequencyBuffers(Synthesizer *synth);
void getSynthAudioBlock(Synthesizer *synth, int16_t *out_buffer);
// "Setters"
void setOctaveOsc1(Synthesizer *synth, uint16_t new_value);
void setOctaveOsc2(Synthesizer *synth, uint16_t new_value);
void setDetuneOsc2(Synthesizer *synth, uint16_t new_value);
void setFilterCutoff(Synthesizer *synth, uint16_t new_value);
void setGain(Synthesizer *synth, uint16_t new_value);
void applyGain(Synthesizer *synth, float *out_buffer);
// MIDI Parameters Functions
void synthesizerNoteOn(Synthesizer *synth, float hertz_note, float velocity);
void synthesizerNoteOff(Synthesizer *synth, float hertz_note, float velocity);
void synthesizerControllerChange(Synthesizer *synth, uint8_t controller_id, float controller_value);
void synthesizerPitchBend(Synthesizer *synth, float pitch_bend);
// Parameters
void parametersChangedAnalog(Synthesizer *synth, uint16_t* new_values);
void parametersChangedDigital(Synthesizer *synth, uint16_t GPIO_Pin);

/* ========== Private function ========== */
float jmap(float source_value, float source_min, float source_max, float target_min, float target_max);
#endif /* INC_DSP_SYNTHESIZER_H_ */
