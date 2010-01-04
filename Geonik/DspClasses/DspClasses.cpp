/*
 *		Dsp Classes : Implementation
 *
 *			Written by George Nicolaidis aka Geonik
 */

#include "DspClasses.h"

#include <math.h>
#include <stdio.h>


//	DspAdd : Add input signal to output signal

void DspAdd(float *pout, float *pin, int const ns) {

	if (ns >= 4) {

		int c = ns >> 2;
		do {

			*pout++ += *pin++;
			*pout++ += *pin++;
			*pout++ += *pin++;
			*pout++ += *pin++; } while(--c); }

	int c = ns & 3;

	while(c--) {

		*pout++ += *pin++; } }


//	DspRemoveDcOffset : Centers signal around zero

void DspRemoveDcOffset(float * const b, int const ns) {

	int c = ns;
	double i = 0.;
	float *b1 = b;
	do {
		i += *b1++;
		} while(--c);
	i = i / ns;
	c = ns; b1 = b;
	do {
		*b1++ -= (float)i; } while(--c); }


//	DspNormalize : Scales signal to new amplitude

void DspNormalize(float * const b, int const ns, double const a) {

	int		 c = ns;
	double	 m = 0.;
	float	*b1 = b;
	do {
		double f = *b1++; f *= f;
		if(f > m) m = f; } while(--c);
	m = a / sqrt(m);
	c = ns; b1 = b;
	do {
		*b1++ *= (float)m; } while(--c); }


//	DspNormalDist : Mean m, Standard deviation s
//
//		Credits: Everett Carter

double DspNormalDist(const double m, const double s) {

	double v1, v2, w;

	do {
		v1 = DspFastRand();
		v2 = DspFastRand();
		w  = v1 * v1 + v2 * v2;
	} while(w >= 1.0);					// Loops 2 times on average

	w = sqrt((-2.0 * log(w)) / w);
	return(m + (v1 * w) * s); }


//	DspMidiNoteToStr : Returns an ASCII string decribing the midi note

const char dsp_aNoteNames[12*3] = "C.\0C#\0D.\0D#\0E.\0F.\0F#\0G.\0G#\0A.\0A#\0B.";

const char *DspMidiNoteToStr(int const n) {

	static char b[32]; static unsigned p = 0; p += 4; if(p >= 32) p = 0;
	int const oct  = n / 12;
	int const note = n % 12;
	sprintf(b+p, "%s%1x", &dsp_aNoteNames[note * 3], oct);
	return b+p; }


//	Frequency table for equal-tempered scale

#pragma warning (disable:4305)
const float dsp_aEqualTemperedScale[128] = {

		//   C        C#       D        D#       E        F        F#       G        G#       A        A#       B
/* 0 */		8.18,    8.66,    9.18,    9.72,   10.30,   10.91,   11.56,   12.25,   12.98,   13.75,   14.57,   15.43,
/* 1 */	   16.35,   17.32,   18.35,   19.45,   20.60,   21.83,   23.12,   24.50,   25.96,   27.50,   29.14,   30.87,
/* 2 */	   32.70,   34.65,   36.71,   38.89,   41.20,   43.65,   46.25,   49.00,   51.91,   55.00,   58.27,   61.74,
/* 3 */	   65.41,   69.30,   73.42,   77.78,   82.41,   87.31,   92.50,   98.00,  103.83,  110.00,  116.54,  123.47,
/* 4 */	  130.81,  138.59,  146.83,  155.56,  164.81,  174.61,  185.00,  196.00,  207.65,  220.00,  233.08,  246.94,
/* 5 */	  261.63,  277.18,  293.66,  311.13,  329.63,  349.23,  369.99,  392.00,  415.30,  440.00,  466.16,  493.88,
/* 6 */	  523.25,  554.37,  587.33,  622.25,  659.26,  698.46,  739.99,  783.99,  830.61,  880.00,  932.33,  987.77,
/* 7 */	 1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760.00, 1864.66, 1975.53,
/* 8 */	 2093.00, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.00, 3729.31, 3951.07,
/* 9 */	 4186.01, 4434.92, 4698.64, 4978.03, 5274.04, 5587.65, 5919.91, 6271.93, 6644.88, 7040.00, 7458.62, 7902.13,
/* A */	 8372.02, 8869.84, 9397.27, 9956.06,10548.08,11175.30,11839.82,12543.85 

};

#pragma warning (default:4305)

double DspCalcNoteFreq(const float *scale, int const note, double const det) {

	double	i, f = modf(det, &i); 
	int		n = note; n += DspFastD2I(i);
	if(f < 0) { n--; f += 1; }
		 if(n > 126) n = 126;
	else if(n < 0)	 n = 0;

	double freq = scale[n];
	freq *= 1. - f;
	freq += f * scale[n+1];

	return freq; }


