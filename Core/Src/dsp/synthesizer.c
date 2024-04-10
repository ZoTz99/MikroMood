/**
  ******************************************************************************
  * @file    synthesizer.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and component of the synth.
  * 	     It is a wrapper that receives the controls change from the outside
  * 	     and call the functions of the component in the inside to update
  * 	     the states.
  ******************************************************************************
**/

#include "dsp/synthesizer.h"

/* ========== Constructor ==========*/
void setupSynthesizer(Synthesizer *synth, float sr) {
	// Setup Components
	setupOsc(&synth->osc1, sr);
	setupOsc(&synth->osc2, sr);
	setupLfo(&synth->lfo, sr);
	setupMixer(&synth->mixer, sr);
	setupAdsr(&synth->adsr, sr);
	setupFilter(&synth->filter, sr);

	// Setup Buffers
	memset(&synth->buffer_osc1, 	0, sizeof(synth->buffer_osc1));
	memset(&synth->buffer_osc2, 	0, sizeof(synth->buffer_osc2));
	memset(&synth->am_buffer, 		0, sizeof(synth->am_buffer));
	memset(&synth->fm_buffer_osc1, 	0, sizeof(synth->fm_buffer_osc1));
	memset(&synth->fm_buffer_osc2, 	0, sizeof(synth->fm_buffer_osc2));
	memset(&synth->fm_buffer_filter,0, sizeof(synth->fm_buffer_filter));

	// Setup Variables
	synth->sr 					= sr;
	synth->gain      			= DEFAULT_GAIN;
	synth->gain_osc1			= DEFAULT_GAIN;
	synth->gain_osc2			= DEFAULT_GAIN;
	synth->mute_osc1 			= DEFAULT_MUTE_OSC_1;
	synth->mute_osc2 			= DEFAULT_MUTE_OSC_2;
	synth->velocity 			= DEFAULT_VELOCITY;
	synth->pitch_bend 			= DEFAULT_PITCH_WHEEL;
	synth->detune_osc2 			= DEFAULT_DETUNE;
	synth->octave_osc1 			= DEFAULT_OCTAVE;
	synth->octave_osc2  		= DEFAULT_OCTAVE;
	synth->hertz_note			= DEFAULT_HERTZ_NOTE;
	synth->filter_cutoff		= DEFAULT_CUTOFF_RATE;
	synth->mod_wheel 			= DEFAULT_MODULATION_WHEEL;
	synth->is_tremolo_mod_on	= DEFAULT_OSC_MODULATION;
	synth->is_vibrato_mod_on	= DEFAULT_OSC_MODULATION;
	synth->is_filter_mod_on 	= DEFAULT_FILTER_MODULATION;
	synth->is_gain_enabled		= DEFAULT_GAIN_ENABLER;
	synth->note_counter 		= 0;
	synth->midi_note 			= 69;
}


/* ========== Processing ========== */

void getSynthAudioBlock(Synthesizer *synth, int16_t *out_buffer) {
	uint16_t *p_buffer = out_buffer;
	uint16_t dac_sample;
	for(int i = 0; i < BUFFER_SIZE; i++) {
		// Frequency Modulations
		float mod_sample = getLfoSample(&synth->lfo);
		float mod_amount = synth->mod_wheel * jmap(mod_sample, -1.0f, 1.0f, -0.5f, +0.5f); // [0.5, 1.5];
		float bend_val = (synth->pitch_bend >= 0.5) ? jmap(synth->pitch_bend, 0.5f, 1.0f, 1.0f, 2.0f) : jmap(synth->pitch_bend, 0.0f, 0.5f, 0.5f, 1.0f);

		float fm_osc1 = synth->hertz_note * bend_val + (synth->is_vibrato_mod_on ? synth->hertz_note*bend_val*mod_amount : 0.0f);
		float fm_osc2 = fm_osc1 * synth->detune_osc2;
		fm_osc1	*= synth->octave_osc1;
		fm_osc2	*= synth->octave_osc2;
		float fm_filter = synth->filter_cutoff + (synth->is_filter_mod_on ? synth->filter_cutoff*mod_amount : 0.0f);

		// Oscillator buffers
		setOscFrequency(&synth->osc1, fm_osc1);
		setOscFrequency(&synth->osc2, fm_osc2);
		float sample = getOscSample(&synth->osc1)*synth->gain_osc1*synth->mute_osc1 + getOscSample(&synth->osc2)*synth->gain_osc2*synth->mute_osc2;

		// Filter
		updateFilterCutoff(&synth->filter, fm_filter);
		sample = getFilterSample(&synth->filter, sample);

		// ADSR
		sample *= getAdsrEnvelope(&synth->adsr);

		if((&synth->adsr)->reset_voice == 1) {
			(&synth->adsr)->reset_voice = 0;
			clearOscAccumulators(&synth->osc1);
			clearOscAccumulators(&synth->osc2);
		}

		// Final Gain
		float am_sample = mod_sample * synth->mod_wheel;
		sample = (sample + (synth->is_tremolo_mod_on ? am_sample*sample: 0.0f))*(synth->gain * synth->is_gain_enabled);

		dac_sample = (uint16_t) ((int16_t) ((32767.0f) * sample)); // Conversion Float -> Int
		*p_buffer++ = dac_sample; // Left Channel Sample
		*p_buffer++ = dac_sample; // Right Channel Sample

	}
}


void applyGain(Synthesizer *synth, float *out_buffer) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		out_buffer[i] = (out_buffer[i] + (synth->is_tremolo_mod_on ? synth->am_buffer[i]*out_buffer[i]: 0.0f))*(synth->gain * synth->is_gain_enabled);
	}
}

/* ========== MIDI Parameters Functions ==========*/
void synthesizerNoteOn(Synthesizer *synth, float hertz_note, float velocity) {
	//synth->midi_note = midi_note;
	synth->hertz_note = hertz_note;
	synth->velocity = velocity;
	synth->note_counter++;
	adsrNoteOn(&synth->adsr);
}

void synthesizerNoteOff(Synthesizer *synth, float hertz_note, float velocity) {
	synth->velocity = velocity;	// Feature di più note da aggiungere dopo
	synth->note_counter--;
	if(synth->note_counter <= 0) {
		adsrNoteOff(&synth->adsr);
		//clearOscAccumulators(&synth->osc1);
		//clearOscAccumulators(&synth->osc2);
	}
}

void synthesizerControllerChange(Synthesizer *synth, uint8_t controller_id, float controller_value) {
	switch(controller_id) {
		case 1:		// Modulation Wheel
			synth->mod_wheel = controller_value;
		break;
		case 7:		// Channel Volume
			synth->chn_vol = controller_value;			// Not Implemented
		break;
		case 10:	// Pan Controller
			synth->pan_ctrl = controller_value; 		// Not Implemented
		break;
		case 64:	// Sustain Pedal
			synth->sustain_pedal = controller_value;	// Not Implemented
		break;
		default:
			;
	}
}

void synthesizerPitchBend(Synthesizer *synth, float pitch_bend) {
	synth->pitch_bend = pitch_bend;
}

/* ========== Setters ==========*/
// FAI ADSR E SMOOTHED

void setOctaveOsc1(Synthesizer *synth, uint16_t new_value) {	// |..|.><.|.><.|.><.|.><.|
	if(new_value < 1229) {
		synth->octave_osc1 = 0.25;
	} else if(new_value > 1229 && new_value < 2048) {
		synth->octave_osc1 = 0.5;
	} else if(new_value > 2048 && new_value < 2867) {
		synth->octave_osc1 = 1.0;
	} else if(new_value > 2867 && new_value < 3686) {
		synth->octave_osc1 = 2;
	} else {
		synth->octave_osc1 = 4;
	}
}

void setOctaveOsc2(Synthesizer *synth, uint16_t new_value) {
	if(new_value < 1229) {
			synth->octave_osc2 = 0.25;
		} else if(new_value > 1229 && new_value < 2048) {
			synth->octave_osc2 = 0.5;
		} else if(new_value > 2048 && new_value < 2867) {
			synth->octave_osc2 = 1.0;
		} else if(new_value > 2867 && new_value < 3686) {
			synth->octave_osc2 = 2;
		} else {
			synth->octave_osc2 = 4;
		}
}

void setDetuneOsc2(Synthesizer *synth, uint16_t new_value) {
	synth->detune_osc2 = (new_value*0.000244) * 0.83 + 0.67;		// [0.67 1.5]
}
void setFilterCutoff(Synthesizer *synth, uint16_t new_value) {
	synth->filter_cutoff = (new_value*0.000244)*SAMPLE_RATE/2 + 10; // [10 20010]
}
void setGain(Synthesizer *synth, uint16_t new_value) {
	synth->gain = (new_value*0.000244);								// [0, 1]
}

/* ========== Parameters ==========*/
void parametersChangedAnalog(Synthesizer *synth, uint16_t* new_values)
{
	// Oscillators
	setOctaveOsc1(synth, new_values[0]);
	setOctaveOsc2(synth, new_values[1]);
	setDetuneOsc2(synth, new_values[2]);

	// Mixer
	setGainOsc1(&synth->mixer, new_values[3]);
	setGainOsc2(&synth->mixer, new_values[4]);
	synth->gain_osc1 = new_values[3] * 0.000244f;
	synth->gain_osc2 = new_values[4] * 0.000244f;

	// LFO
	setLfoFrequency(&synth->lfo, (new_values[5]*0.000244f)*200+0.05f);

	// Filter
	setFilterCutoff(synth, new_values[6]);
	setFilterResonance(&synth->filter, new_values[7]);

	//ADSR
	setAdsrAttack(&synth->adsr, new_values[8]);
	setAdsrRelease(&synth->adsr, new_values[9]);

	// Gain
	new_values[10] = 100;	// Standard value because volume is very high

	setGain(synth, new_values[10]);
}

void parametersChangedDigital(Synthesizer *synth, uint16_t GPIO_Pin)
{
	// All'avvio della mia applicazione dovrei fare uno scanning di tutti gli input in modo che parta da uno stato corretto e aggiornato
	switch(GPIO_Pin) {
		// Oscillators
		case GPIO_PIN_0:
			setOscWaveform(&synth->osc1, 0);	// SETTARE GLI INPUT DI QUESTI MULTISWITCH SOLO COME RISING E NON COME FALLING DATO CHE VOGLIO CHE LO STATO CAMBI SOLO QUANDO IL SEGNALE SI ALZA E NON QUANDO SI ABBASSA
		break;
		case GPIO_PIN_1:
			setOscWaveform(&synth->osc1, 1);	// SETTARE GLI INPUT DI QUESTI MULTISWITCH SOLO COME RISING E NON COME FALLING DATO CHE VOGLIO CHE LO STATO CAMBI SOLO QUANDO IL SEGNALE SI ALZA E NON QUANDO SI ABBASSA
		break;
		case GPIO_PIN_2:
			setOscWaveform(&synth->osc1, 2);	// SETTARE GLI INPUT DI QUESTI MULTISWITCH SOLO COME RISING E NON COME FALLING DATO CHE VOGLIO CHE LO STATO CAMBI SOLO QUANDO IL SEGNALE SI ALZA E NON QUANDO SI ABBASSA
		break;
		case GPIO_PIN_3:
			setOscWaveform(&synth->osc2, 0);
		break;
		case GPIO_PIN_4:
			setOscWaveform(&synth->osc2, 1);
		break;
		case GPIO_PIN_5:
			setOscWaveform(&synth->osc2, 2);
		break;

		case GPIO_PIN_6:
			setMuteOsc1(&synth->mixer, HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6));
			synth->mute_osc1 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6);
		break;
		case GPIO_PIN_7:
			setMuteOsc2(&synth->mixer, HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7));
			synth->mute_osc2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_7);
		break;
		case GPIO_PIN_8:
			setLfoWaveform(&synth->lfo, HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8));
		break;
		case GPIO_PIN_9:
			synth->is_tremolo_mod_on = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_9);
		break;
		case GPIO_PIN_10:
			synth->is_vibrato_mod_on = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10);
		break;
		case GPIO_PIN_11:
			synth->is_filter_mod_on = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_11);
		break;
		case GPIO_PIN_12:
			synth->is_gain_enabled = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12);
		break;
		default:
			;
	}
}

/* ========== Private function ========== */
float jmap(float source_value, float source_min, float source_max, float target_min, float target_max) {
	return target_min + ((target_max - target_min) * (source_value - source_min)) / (source_max - source_min);
}



/* ========== Deprecati ========== */
/*
void getSynthAudioBlock(Synthesizer *synth, float *out_buffer) {

	// Frequency buffers
	getFrequencyBuffers(synth);
	// Oscillator buffers
	getOscAudioBlock(&synth->osc1, synth->fm_buffer_osc1, synth->buffer_osc1);
	getOscAudioBlock(&synth->osc2, synth->fm_buffer_osc2, synth->buffer_osc2);
	// Mixer
	getMixerAudioBlock(&synth->mixer, out_buffer, synth->buffer_osc1, synth->buffer_osc2);
	// Filter
	getFilterAudioBlock(&synth->filter, synth->fm_buffer_filter, out_buffer);
	// ADSR
	getAdsrAudioBlock(&synth->adsr, out_buffer);
	// Final Gain
	applyGain(synth, out_buffer);

}


void getFrequencyBuffers(Synthesizer *synth) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		// Hertz
		float mod_sample = getLfoSample(&synth->lfo);	// [-1, +1]
		//float mod_amount = synth->mod_wheel * mod_sample * 24.0;
		float mod_amount = synth->mod_wheel * jmap(mod_sample, -1.0f, 1.0f, -0.5f, +0.5f); // [0.5, 1.5] // mod_sample * 1.5f + 0.5f;   // [0.5, 1.5]
		float bend_val = (synth->pitch_bend >= 0.5) ? jmap(synth->pitch_bend, 0.5f, 1.0f, 1.0f, 2.0f) : jmap(synth->pitch_bend, 0.0f, 0.5f, 0.5f, 1.0f);
		synth->am_buffer[i]			= mod_sample * synth->mod_wheel;
		synth->fm_buffer_osc1[i] 	= synth->hertz_note * bend_val + (synth->is_vibrato_mod_on ? synth->hertz_note*bend_val*mod_amount : 0.0f);
		synth->fm_buffer_osc2[i] 	= synth->fm_buffer_osc1[i] * synth->detune_osc2;
		synth->fm_buffer_osc1[i] 	= synth->fm_buffer_osc1[i] * synth->octave_osc1;
		synth->fm_buffer_osc2[i] 	= synth->fm_buffer_osc2[i] * synth->octave_osc2;
		synth->fm_buffer_filter[i]	= synth->filter_cutoff + (synth->is_filter_mod_on ? synth->filter_cutoff*mod_amount : 0.0f);

		// MIDI

		//float mod_sample = getLfoSample(&synth->lfo);
		//float mod_amount = synth->mod_wheel * mod_sample * 24.0;
		//synth->am_buffer[i]			= mod_sample;
		//synth->fm_buffer_osc1[i] 	= synth->hertz_note + synth->pitch_bend + (synth->is_vibrato_mod_on ? mod_amount : 0.0);
		//synth->fm_buffer_osc2[i] 	= synth->fm_buffer_osc1[i] + synth->detune_osc2;
		//synth->fm_buffer_osc1[i] 	= midiToHertz(synth->fm_buffer_osc1[i]) * synth->octave_osc1;
		//synth->fm_buffer_osc2[i] 	= midiToHertz(synth->fm_buffer_osc2[i]) * synth->octave_osc2;
		//synth->fm_buffer_filter[i]	= 10000; //midiToHertz(hertzToMidi(synth->filter_cutoff) + (synth->is_filter_mod_on ? mod_amount : 0.0));

	}
}
*/





