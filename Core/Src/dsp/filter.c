/**
  ******************************************************************************
  * @file    filter.c
  * @author  Bianchi Davide
  * @brief   This file contains the whole structure and function of the filter.
  ******************************************************************************
**/

#include "dsp/filter.h"

/* ========== Constructor ==========*/

void setupFilter(Filter *f, float sr) {
	f->sr 		= sr;
	f->sp 		= 1.0f/sr;
	f->cutoff 	= DEFAULT_CUTOFF_RATE;
	f->k 		= DEFAULT_RESONANCE;
	f->g		= (2*M_PI*f->cutoff)*(1/sr)*0.5f;
	f->Glp		= f->g/(1+f->g);
	f->Gtot		= f->g*f->g*f->g*f->g; //powf(f->g, 4.0f);
	f->coeff 	= 1;

	memset(&f->v, 0, sizeof(f->v));
	memset(&f->s, 0, sizeof(f->s));
	memset(&f->y, 0, sizeof(f->y));

}


/* ========== Parameters ==========*/

void updateFilterCutoff(Filter *f, float cutoff) {
	f->cutoff 	= cutoff;
	f->g		= (2*M_PI*cutoff)*(1/f->sr)*0.5f;
	f->Glp 		= f->g/(1+f->g);
	f->Gtot 	= f->g*f->g*f->g*f->g; // powf(f->g, 4.0f);

}


void setFilterResonance(Filter *f, float resonance) {
	f->k = resonance * 0.000488;	// [0, 2]
	f->coeff  = 1 + f->k *2.0f;
}

/*========== Processing ==========*/
void getFilterAudioBlock(Filter *filter, float *fm_buffer, float *out_buffer) {
	for(int i = 0; i < BUFFER_SIZE; i++) {
		updateFilterCutoff(filter, fm_buffer[i]);
		out_buffer[i] = getFilterSample(filter, out_buffer[i]);
	}
}

float getFilterSample(Filter *f, float x) {
	// Pre-calculus
	float S = f->g*f->g*f->g*f->s[0] + f->g*f->g*f->s[1] + f->g*f->s[2] + f->s[3];
	float u = (x - f->k*S)/(1+f->k*f->Gtot);
	u = tanhf(u);

	// Forward path
	// Filter 1
	f->v[0] = (u - f->s[0])* f->Glp;
	f->y[0] = (f->v[0] + f->s[0]);
	f->s[0] = (f->y[0] + f->v[0]);

	// Filter 2
	f->v[1] = (f->y[0] - f->s[1])* f->Glp;
	f->y[1] = (f->v[1] + f->s[1]);
	f->s[1] = (f->y[1] + f->v[1]);

	// Filter 3
	f->v[2] = (f->y[1] - f->s[2])* f->Glp;
	f->y[2] = (f->v[2] + f->s[2]);
	f->s[2] = (f->y[2] + f->v[2]);

	// Filter 4
	f->v[3] = (f->y[2] - f->s[3])* f->Glp;
	f->y[3] = (f->v[3] + f->s[3]);
	f->s[3] = (f->y[3] + f->v[3]);

	return f->y[3] * f->coeff;
}






