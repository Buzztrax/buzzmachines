/*
 *		Dsp Classes : FastInvSqrt
 *
 *		Credits: Ken Turkowski
 */

#include "../DspClasses/DspClasses.h"

#include <math.h>
#include <stdio.h>


#define kLookupBits    6   /* Number of mantissa bits for lookup */
#define kExpPos       23   /* Position of the exponent */
#define kExpBias     127   /* Bias of exponent */

#define kLoopupPos   (kExpPos-kLookupBits)  /* Position of mantissa lookup */
#define kSeedPos     (kExpPos-8)            /* Position of mantissa seed */
#define kTableSize   (2 << kLookupBits)     /* Number of entries in table */
#define kLoopupMask  (kTableSize - 1)       /* Mask for table input */

#define mGetExp(a)			(((a) >> kExpPos) & 0xFF)
#define mSetExp(a)			((a) << kExpPos)
#define mGetMantissa(a)		(((a) >> kLoopupPos) & kLoopupMask)
#define mSetMantissaSeed(a)	(((unsigned long)(a)) << kSeedPos)

static unsigned char aInvSqrt[kTableSize];

union flint {
    unsigned long    i;
    float            f; };


void DspFastInvSqrtInit() {

    register long f;
    register unsigned char *h;

	h = aInvSqrt;
	for (f = 0, h = aInvSqrt; f < kTableSize; f++) {
	    union flint fi, fo;
		fi.i = ((kExpBias-1) << kExpPos) | (f << kLoopupPos);
		fo.f = (float)(1.0 / sqrt(fi.f));
		*h++ = (char)(((fo.i + (1<<(kSeedPos-2))) >> kSeedPos) & 0x000000FF); }
	aInvSqrt[kTableSize / 2] = 0xFF; }


double DspFastInvSqrt(float const x) {

	register unsigned long a = *(unsigned long *)(&x);

	a = mSetExp(((3*kExpBias-1) - mGetExp(a)) >> 1) | mSetMantissaSeed(aInvSqrt[mGetMantissa(a)]);

	register double	r = *(float *)(&a);
	r *= 1.5 - 0.5 * r * r * x;
	r *= 1.5 - 0.5 * r * r * x;
	return r; }
