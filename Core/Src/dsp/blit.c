/**
  ******************************************************************************
  * @file    blit.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and function of the blit.
  ******************************************************************************
**/

#include "dsp/blit.h"

/* ========== Init functions ========== */
void createBlitTable(Blit *blit) {
	float step = 0.0039f; // 1/256 -> (1/n_sinc)
	float temp = 0.0f;
	float totalSum = 0.0f;

	for (int i = 0; i < 256; i++) {
		totalSum = 0.0f;
		temp = step * i;

		for (int x = 0; x < 16; x++) {
			blit->blits_matrix[i][x] = x == 8 ?
				(!temp ? (2.0f * 3.14f * 0.45f) : (sinf(2.0f * 3.14f * 0.45f * (-temp)) / (-temp))) :
				(sinf(2.0f * 3.14f * 0.45f * (x - 8.0f - temp)) / (x - 8.0f - temp));
			blit->blits_matrix[i][x] *= (0.51f - 0.49f * (cosf(2.0f * 3.14f * (x - temp) / 16.0f)));

			totalSum += blit->blits_matrix[i][x];
		}

		// Normalize (Sum of all sample = 1)
		for (int x = 0; x < 16; x++) {
			blit->blits_matrix[i][x] /= totalSum;
		}
	}
}

void setupBlit(Blit *blit, float sr) {
    blit->sr 				= sr;
	blit->sp 				= 1.0f / sr;
	blit->p_edge 			= 0.0f;
	blit->n_edge 			= 0.0f;
	blit->sub_offset1 		= 0.0f;
	blit->sub_offset2 		= 0.0f;
	blit->alpha_coeff 		= 0.999f;
	blit->decrement_step 	= 0.0f;
	blit->leakiness			= 0.0f;
	blit->leakiness_tri 	= 0.0f;

		//Accumulators
	blit->acc_tri 		= 0.0f;
	blit->acc_saw 		= 0.0f;
	blit->acc_square 	= 0.0f;

		// Blits utils
	blit->sample_cont 					= 0;
	blit->passed_neg 					= false;
	blit->index 						= 0;
	memset(&blit->p_blit, 0, sizeof(blit->p_blit));
	memset(&blit->n_blit, 0, sizeof(blit->n_blit));
	memset(&blit->blits_matrix, 0, sizeof(blit->blits_matrix));
	createBlitTable(blit);

	//alpha = exp(-(LEAKY_INTEGRATOR_BASE_FREQUENCY / sr) * MathConstants<double>::twoPi);
	//leakMod   = alpha - exp(-(LEAKY_INTEGRATOR_MOD_FREQUENCY/sr) * MathConstants<double>::twoPi);
	//leakNoise = alpha - exp(-(LEAKY_INTEGRATOR_NOISE_FREQUENCY/sr) * MathConstants<double>::twoPi);
	//leakTrNoi = alpha - exp(-(LEAKY_INTEGRATOR_TRI_NOISE_FREQUENCY/sr) * MathConstants<double>::twoPi);
	//leakTrMod = alpha - exp(-(LEAKY_INTEGRATOR_TRI_MOD_FREQUENCY / sr) * MathConstants<double>::twoPi);
}

/* ========== Utils functions ========== */
void getPositiveBlit(Blit *blit) {
	int blit_index = 0;
    blit->sub_offset1 = blit->p_edge - (int)blit->p_edge;
	blit_index = blit->sub_offset1 * 256;
	uint8_t j = blit->index;
	float *temp_blit = blit->blits_matrix[blit_index];
	for (int i = 0; i < 16; i++) {
		blit->p_blit[j] += temp_blit[i];
		j++;
	}
}

void getNegativeBlit(Blit *blit) {
	int blit_index = 0;
	blit->sub_offset2 = blit->n_edge - (int)blit->n_edge;
	blit_index = blit->sub_offset2 * 256;
	uint8_t j = blit->index;
	float *temp_blit = blit->blits_matrix[blit_index];
	for (int i = 0; i < 16; i++) {
		blit->n_blit[j] -= temp_blit[i];
		j++;
	}
}

bool negativeEdgeCrossed(Blit *blit)
{
	int threshold = (int)blit->n_edge;
	bool cross = !blit->passed_neg && (blit->sample_cont >= threshold);
	if (cross) blit->passed_neg = true;
	return cross;
}

void updateLeakiness(Blit *blit)  // DA IMPLEMENTARE
{
	// blit->leakiness = 0.0;
	// blit->leakiness_tri = 0.0;
}


void clearBlitAccumulators(Blit *blit) {
	blit->acc_tri = 	0.0f;
	blit->acc_saw = 	0.0f;
	blit->acc_square = 	0.0f;
	blit->sample_cont = 0;
}

/* ========== Wave functions ========== */
float getBlitSample(Blit *blit, float f, int waveform) {
	float temp_sample = 0.0f;
	blit->decrement_step = f * 1.0f/blit->sr;

	switch (waveform) {
		case TRIANGLE:
			temp_sample = getTriSample(blit, f);
			break;
		case SAWTOOTH:
			temp_sample = getSawSample(blit, f);
			break;
		case SQUARE:
			temp_sample = getSquareSample(blit, f);
			break;
	}

	blit->p_blit[blit->index] = 0.0f;
	blit->n_blit[blit->index] = 0.0f;
	blit->sample_cont++;
	blit->index++;
	return temp_sample;
}

float getTriSample(Blit *blit, float f) {
	// blit->acc_tri = blit->acc_tri * (blit->alpha_coeff - blit->leakiness_tri) + getSquareSample(blit, f) * 8.0f * f * blit->sp;
	blit->acc_tri = blit->acc_tri * (blit->alpha_coeff - blit->leakiness_tri) + getSquareSample(blit, f) * 4.0f * f * blit->sp;
	return blit->acc_tri;
}

float getSawSample(Blit *blit, float f) {
	blit-> n_edge = (blit->sr / f) + blit->sub_offset2;

	if (blit->sample_cont >= (int)blit->n_edge)
	{
		blit->sample_cont = 0;
		getNegativeBlit(blit);

		//decrementStep = -1.0 * f * sp;
	}

	blit->acc_saw = blit->acc_saw * (blit->alpha_coeff - blit->leakiness) + blit->n_blit[blit->index]+ blit->decrement_step; // - decrementStep;
	return blit->acc_saw ;
}

float getSquareSample(Blit *blit, float f) {
	blit->p_edge = (blit->sr) / f + blit->sub_offset1;
	blit->n_edge = (blit->p_edge + blit->sub_offset1) * 0.5f;

	if (blit->sample_cont >= (int)blit->p_edge) {
		blit->passed_neg = false;
		blit->sample_cont = 0;
		getPositiveBlit(blit);
	}

	//if (sampleCont == int(nEdge)) getNegativeBlit();
	if (negativeEdgeCrossed(blit)) getNegativeBlit(blit);
	blit->acc_square = blit->acc_square * (blit->alpha_coeff - blit->leakiness) + blit->p_blit[blit->index] + blit->n_blit[blit->index];
	//accSquare = pBlit[index] + nBlit[index];
	return blit->acc_square;
}
