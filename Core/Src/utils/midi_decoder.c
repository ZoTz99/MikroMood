/**
  ******************************************************************************
  * @file    midi_decoder.c
  * @author  Bianchi Davide
  * @brief   This file contains all the functions that elaborate the midi packet
  * 		 to be used with the synth.
  ******************************************************************************
**/

#include "utils/midi_decoder.h"

/* ======= MIDI Functions Wrapper ======*/
void midiDecode(USBH_HandleTypeDef *phost, Synthesizer *synth, uint8_t *midi_rx_buffer)
{
	uint16_t number_of_packets;
	uint8_t *ptr = midi_rx_buffer;
	midi_package_t packet;

	number_of_packets = USBH_MIDI_GetLastReceivedDataSize(phost) / 4; // Each USB midi package is 4 bytes long

	while (number_of_packets--)
	{
		packet.usb_byte = *ptr;
		ptr++;
		packet.status_byte = *ptr;
		ptr++;
		packet.data_byte_1 = *ptr;
		ptr++;
		packet.data_byte_2 = *ptr;
		ptr++;

		switch(packet.status_byte & 0xF0) {
			case 0x80:	// NoteOff
				midiDecodeNoteOff(synth, packet.data_byte_1, packet.data_byte_2);
			break;
			case 0x90:	// NoteOn
				midiDecodeNoteOn(synth, packet.data_byte_1, packet.data_byte_2);
			break;
			case 0xB0:	// Controller Change
				midiDecodeControllerChange(synth, packet.data_byte_1, packet.data_byte_2);
			break;
			case 0xE0:	// Pitch Bend
				midiDecodePitchBend(synth, packet.data_byte_1, packet.data_byte_2);
			break;
			default:
				;
		}
	}
}

/* ========== MIDI Functions ==========*/
void midiDecodeNoteOff(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2) {
	//float midi_note = data_byte_1;
	float hertz_note = hertz_notes[data_byte_1-21];
	float velocity = data_byte_2 * 0.007874; 			// data_byte_2 / 127.0;   [0, 1]
	synthesizerNoteOff(synth, hertz_note, velocity);
}

void midiDecodeNoteOn(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2) {
	// float midi_note = data_byte_1;
	float hertz_note = hertz_notes[data_byte_1-21];
	float velocity = data_byte_2 * 0.007874; 			// [0, 1]
	synthesizerNoteOn(synth, hertz_note, velocity);
}

void midiDecodeControllerChange(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2) {
	uint32_t controller_id = data_byte_1;
	float controller_value = data_byte_2 * 0.007874; 	// [0, 1]
	synthesizerControllerChange(synth, controller_id, controller_value);
}

void midiDecodePitchBend(Synthesizer *synth, uint8_t data_byte_1, uint8_t data_byte_2) {
	uint16_t packet = (data_byte_1 | (data_byte_2 << 7));
	float pitch_bend = packet * 0.00006104; 			// [0, 1]
	synthesizerPitchBend(synth, pitch_bend);
}



