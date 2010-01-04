/*
 *		Dsp Classes : FastSqrt
 *
 *		Credits: Paul Lalonde, Robert Dawson
 */

#include <stdio.h>
#include <math.h>

#define kMostSigOffset	1
#define kTableSize		16384

#define kMantissaShifts	7

#define kExpBias		1023       /* Exponents are always positive     */
#define kExpShifts		20         /* Shifs exponent to least sig. bits */
#define kExpLsb			0x00100000 /* 1 << kExpShifts                   */
#define kMantissaMask	0x000FFFFF /* Mask to extract mantissa          */

static int aSqrt[kTableSize];

void DspFastSqrtInit() {

	int           i;
    double        f;
    unsigned int *fi = (unsigned int *)&f + kMostSigOffset;
    
    for (i = 0; i < kTableSize/2; i++) {
		f = 0;
		*fi = (i << kMantissaShifts) | (kExpBias << kExpShifts);
		f = sqrt(f);
		aSqrt[i] = *fi & kMantissaMask;

		f = 0;
		*fi = (i << kMantissaShifts) | ((kExpBias + 1) << kExpShifts);
		f = sqrt(f);
		aSqrt[i + kTableSize/2] = *fi & kMantissaMask; } }


double DspFastSqrt(const double f) {

	unsigned int  e;
	unsigned int  i = *((unsigned int *)&f + kMostSigOffset);

	e = (i >> kExpShifts) - kExpBias;
	i &= kMantissaMask;
	if (e & 1) i |= kExpLsb;
	e >>= 1;
	i = (aSqrt[i >> kMantissaShifts]) | ((e + kExpBias) << kExpShifts);
	double r; *((unsigned int *)&r + kMostSigOffset) = i;
	return r; }

