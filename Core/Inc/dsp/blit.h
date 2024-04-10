/**
  ******************************************************************************
  * @file    blit.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the blit.c
  ******************************************************************************
**/

#include "parameters.h"

#ifndef INC_DSP_BLIT_H_
#define INC_DSP_BLIT_H_

/* ========== Base structure ========== */
typedef struct {
	// Base variable
	float sr;
	float sp;
	float p_edge;
	float n_edge;
	float sub_offset1;
	float sub_offset2;
	float alpha_coeff;
	float decrement_step;
	float leakiness;
	float leakiness_tri;

	//Accumulators
	float acc_tri;
	float acc_saw;
	float acc_square;

	// Blits utils
	int sample_cont;
	bool passed_neg;
	unsigned char index;
	float p_blit[256];
	float n_blit[256];
	float blits_matrix[256][16];
} Blit;

/* ========== Exported functions ========== */
void setupBlit   			(Blit *blit, float sr);
void createBlitTable		(Blit *blit);
void getPositiveBlit		(Blit *blit);
void getNegativeBlit		(Blit *blit);
bool negativeEdgeCrossed	(Blit *blit);
void clearBlitAccumulators	(Blit *blit);
void updateLeakiness		(Blit *blit);

float getBlitSample		(Blit *blit, float f, int waveform);
float getTriSample		(Blit *blit, float f);
float getSawSample		(Blit *blit, float f);
float getSquareSample	(Blit *blit, float f);

#endif /* INC_DSP_BLIT_H_ */

