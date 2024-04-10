/**
  ******************************************************************************
  * @file    parameters.h
  * @author  Bianchi Davide
  * @brief   This file contains all the default values used in the synthesizer
  ******************************************************************************
**/

#ifndef INC_PARAMETERS_H_
#define INC_PARAMETERS_H_

// Utils
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include <string.h>
#include <math.h>


enum AdsrState {
    ADSR_IDLE,
    ADSR_ATTACK,
    ADSR_SUSTAIN,
	ADSR_RELEASE
};

enum Waveform {
	TRIANGLE,
	SAWTOOTH,
	SQUARE
};

#define BUFFER_SIZE 	32		// I2S_BUFFER_SIZE/4
#define I2S_BUFFER_SIZE 128
#define SAMPLE_RATE		48828.0f

//Valori parametri oscillatori
#define DEFAULT_MUTE_OSC_1      false	// Osc1 not muted
#define DEFAULT_MUTE_OSC_2      true    // Osc2 muted
#define DEFAULT_WF		        1 		// Triangolare
#define DEFAULT_OCTAVE	    	1
#define DEFAULT_DETUNE          0.0f
#define DEFAULT_GAIN	        0.3f
#define DEFAULT_HERTZ_NOTE		220.0f
//Valori parametri LFO
#define DEFAULT_WF_LFO		    0
#define DEFAULT_RATE_LFO        3.0f
#define DEFAULT_GAIN_LFO	    1.0f
//Valori parametri filtro
#define DEFAULT_CUTOFF_RATE     10000.0
#define MAX_CUTOFF_RATE         20000.0
#define DEFAULT_RESONANCE       0.2f
//Valori parametri Loudness + Filter ADSR
#define DEFAULT_ATTACK          0.0001f
#define DEFAULT_RELEASE			0.0001f
//Valori parametri Enabler Generici
#define DEFAULT_OSC_MODULATION      false
#define DEFAULT_FILTER_MODULATION   false
#define DEFAULT_GAIN_ENABLER        true
//Valori parametri modulazioni
#define DEFAULT_MODULATION_WHEEL    0.0f
#define DEFAULT_MAX_MOD_AMOUNT      0.999f
#define DEFAULT_MIN_MOD_AMOUNT      0.001f
//Valori parametri Controlli Generici
#define DEFAULT_GLIDE_RATE		    0.001f
#define DEFAULT_PITCH_WHEEL         0.5f
#define DEFAULT_VELOCITY			1.0f

#endif /* INC_PARAMETERS_H_ */

