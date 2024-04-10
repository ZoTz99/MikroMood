/**
  ******************************************************************************
  * @file    midi_decoder.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the midi_decoder.c
  ******************************************************************************
**/

#include "usb_host.h"
#include "usbh_core.h"
#include "dsp/synthesizer.h"
#include "driver/usbh_midi.h"

#ifndef INC_DRIVER_MIDI_DRIVER_H_
#define INC_DRIVER_MIDI_DRIVER_H_

static float hertz_notes[88] = 	{
									27.50, 29.14, 30.87, 32.70, 34.65, 36.71, 38.89, 41.20, 43.65, 46.25, 49.00, 51.91, 						// ottava 1
									55.00, 58.27, 61.74, 65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.83, 						// ottava 2
									110.00, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 			// ottava 3
									220.00, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 			// ottava 4
									440.00, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 			// ottava 5
									880.00, 932.33, 987.77, 1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 	// ottava 6
									1760.00, 1864.66, 1975.53, 2093.00, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, // ottava 7
									3520.00, 3729.31, 3951.07, 4186.01, 4434.92, 4698.63, 4978.03, 5274.04, 5587.65, 5919.91, 6271.93, 6644.88  // ottava 8
								};
/* ========== Exported functions ========== */
void midiDecode(USBH_HandleTypeDef *phost, Synthesizer *synth, uint8_t *midi_rx_buffer);
void midiDecodeNoteOff(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2);
void midiDecodeNoteOn(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2);
void midiDecodeControllerChange(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2);
void midiDecodePitchBend(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2);

#endif /* INC_DRIVER_MIDI_DRIVER_H_ */
