/*
 *		Dsp Classes : Implementation
 *
 *			Written by George Nicolaidis aka Geonik
 */

#include <math.h>


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
		double const f = *b1++; f*=f;
		if(f > m) m = f;
		} while(--c);
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
