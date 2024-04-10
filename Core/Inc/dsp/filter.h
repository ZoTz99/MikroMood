/**
  ******************************************************************************
  * @file    filter.h
  * @author  Bianchi Davide
  * @brief   This file contains all the prototypes for the filter.c
  ******************************************************************************
**/

#include "parameters.h"

#ifndef INC_DSP_FILTER_H_
#define INC_DSP_FILTER_H_

/* ========== Base structure ========== */
typedef struct {
	float sr;
	float sp;
	float cutoff;
	float k;
	float g;
	float Glp;
	float Gtot;
	float coeff;

	float v[4];
	float s[4];
	float y[4];
} Filter;

/* ========== Exported functions ========== */
void setupFilter		(Filter *filter, float sr);
void updateFilterCutoff	(Filter *filter, float cutoff);
void setFilterResonance	(Filter *filter, float resonance);
void getFilterAudioBlock(Filter *filter, float *fm_buffer, float *out_buffer);
float getFilterSample	(Filter *filter, float x);

#endif /* INC_DSP_FILTER_H_ */
