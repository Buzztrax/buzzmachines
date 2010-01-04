/*
 *		Dsp Classes
 *
 *			Written by George Nicolaidis aka Geonik
 */

#ifndef inc_dspcvolume
#define inc_dspcVolume

#include "DspClasses.h"


/*	CRms
 *
 *		RMS volume detection
 */

struct CRms {
	double	fC1;
	double	fC2;
	double	fQ;

	CRms() {
		fQ		= 0; }

	void Configure(int const averaging=10,double const samplesPerSec=g_iSampleRate) {
		double b = 2.0 - cos((double)averaging * 2 * kPi / samplesPerSec);
		fC2		 = b - sqrt(b * b - 1.0);
		fC1		 = 1.0 - fC2; }

	void Clear() {
		fQ		 = 0; }

	void SetRms(double const newq) {					// Set amplitude
		fQ		 = newq*newq; }

#pragma optimize ("a", on)

	double WorkSamples(float *pb, int const ns) {		// Returns square of mean amp
		double const c1 = fC1;
		double const c2 = fC2;
		double		 q  = fQ;

		if (ns >= 4) {
			int cnt = ns >> 2;
			do {
				q = c1 * pb[0] * pb[0] + c2 * q;
				q = c1 * pb[1] * pb[1] + c2 * q;
				q = c1 * pb[2] * pb[2] + c2 * q;
				q = c1 * pb[3] * pb[3] + c2 * q;
				pb += 4; } while(--cnt); }
		int cnt = ns & 3;
		while(cnt--) {
			double const v = *pb++;
			q = c1 * v * v + c2 * q; }

		return (fQ = q); }

#pragma optimize ("a", off)

 };


#endif
