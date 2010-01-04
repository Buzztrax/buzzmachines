/*
 *		DspClasses / Filter.h
 *
 *		Based on code by Oskari Tammelin and Perry Cook
 *
 *			Written by George Nicolaidis
 */

#pragma once

/*	FilterQ
 *
 *		One pole resonant filter
 */

struct FilterQ {

	double	fCutOff;
	double	fResonance;
	double	fLowpass;
	double	fBandpass;
	double	fHighpass;

	void Clear() {

		fLowpass	= 0;
		fBandpass	= 0;
		fHighpass	= 0; }

#pragma optimize ("a", on)

	void Work(float *pout, int const numsamples) {

		double L = fLowpass;
		double B = fBandpass;
		double H = fHighpass;
		double f = fCutOff;

		double const q = fResonance;

		int c = numsamples;

		do {

			L += f * B;
			H = *pout - L - q * B;
			B += f * H;
			*pout++ = (float)L;

		} while(--c);

		fHighpass = H;
		fBandpass = B;
		fLowpass = L; }

#pragma optimize ("a", off)
};

/*	Filter2p2q
 *
 *		2-pole 2-resonance filter
 */

struct Filter2p2q {

	double	fCutOff;
	double	fBpRes;
	double	fLpRes;

	double	fLopass[2];
	double	fBdpass[2];
	double	fHipass[2];

	Filter2p2q() {

		Clear();

		fCutOff		= 1.0;
		fBpRes		= 1.0;
		fLpRes		= 1.0; }

	void Clear() {

		fLopass[0] = 0;
		fBdpass[0] = 0;
		fHipass[0] = 0;
		fLopass[1] = 0;
		fBdpass[1] = 0;
		fHipass[1] = 0; }

#pragma optimize ("a", on)

	void WorkSamples(float *pout, int const numsamples) {

		double L1 = fLopass[0];
		double B1 = fBdpass[0];
		double H1 = fHipass[0];
		double L2 = fLopass[1];
		double B2 = fBdpass[1];
		double H2 = fHipass[1];

		double f = fCutOff;

		double const bq = fBpRes;
		double const lq = fLpRes;

		int c = numsamples;

		do {

			L1 += f * B1;
			H1 = *pout - L1 - bq * B1;		// first pole input / BP resonance
			B1 += f * H1;
			L2 += f * B2;
			H2 = L1 - lq * L2 - B2;			// second pole input / LP resonance
			B2 += f * H2;
			*pout++ = (float)L2;			// output

		} while(--c);

		fLopass[0] = L1;
		fBdpass[0] = B1;
		fHipass[0] = H1;
		fLopass[1] = L2;
		fBdpass[1] = B2;
		fHipass[1] = H2; }

#pragma optimize ("a", off)
};

/*	Filter1p
 *
 *		One pole filter
 */

struct Filter1p {

	double	fCoef;
	double	fGain;
	double	fNormGain;
	double	fLastOut;

	Filter1p() {

		fCoef		= 0.9;
		fGain		= 1.0;
		fNormGain	= 0.1;
		fLastOut	= 0.0; }

	void Clear() {

		fLastOut	= 0.0; }

	void SetPole(double const fNewCoef) {

		fCoef = fNewCoef;
		if(fCoef > 0)	fNormGain = fGain * (1.0 - fCoef);
		else			fNormGain = fGain * (1.0 + fCoef); }

	void SetGain(double const fNewGain) {

		fGain = fNewGain;
		if(fCoef > 0)	fNormGain = fGain * (1.0 - fCoef);
		else			fNormGain = fGain * (1.0 + fCoef); }

	double Work(double const fNew) {

		fLastOut = (fNew * fNormGain) + (fCoef * fLastOut);
		return fLastOut; }

	void WorkSamples(float *inb, int const numsamples) {

		double const ng = fNormGain;
		double const c  = fCoef;
		double		 ls = fLastOut;

		int ns=numsamples; do {

			double s = *inb;
			ls = s*ng + ls*c;
			*inb++ = (float)ls; } while(--ns);

		fLastOut = ls; }
};

/*	FilterBiQuad
 *
 *		2-pole, 2-zero filter
 */

struct FilterBiQuad {

	double	fZeroC[2];
	double	fPoleC[2];
	double	fIn[2];

	FilterBiQuad() {

		fZeroC[0]	= 0;
		fZeroC[1]	= 0;
		fPoleC[0]	= 0;
		fPoleC[1]	= 0;
		fIn[0]		= 0;
		fIn[1]		= 0; }

	void Clear() {

		fIn[0]		= 0;
		fIn[1]		= 0; }

	void SetFreqAndReson(double freq, double reson) {

		fPoleC[1] = -(reson*reson);
		fPoleC[0] = 2.0*reson* cos((2*kPi) * freq / g_iSampleRate); }

	void WorkSamplesEqualGain(float *pin,int const ns) {

		int			 numsamples = ns;
		double const pc0 = fPoleC[0];
		double const pc1 = fPoleC[1];
		double		 i0  = fIn[0];
		double		 i1  = fIn[1];

		do {

			double t = *pin;
			t += i0*pc0;
			t += i1*pc1;
			*pin++ = (float)(t - i1);
			i1 = i0;
			i0 = t; 
		
		} while(--numsamples);

		fIn[0] = i0;
		fIn[1] = i1; }

	void WorkSamplesZeroZeroes(float *pin,int const ns) {

		int numsamples = ns;

		double const pc0 = fPoleC[0];
		double const pc1 = fPoleC[1];
		double		 i0  = fIn[0];
		double		 i1  = fIn[1];

		do {

			double t = *pin;
			t += i0*pc0;
			t += i1*pc1;
			*pin++ = (float)(t);
			i1 = i0;
			i0 = t;
	
		} while(--numsamples);

		fIn[0] = i0;
		fIn[1] = i1; }
};

/*	FilterBw
 *
 *		Second order butterworth filter
 */

struct FilterBw {

	float	fA[5];
	float	fY[2];

	FilterBw() {

		Clear(); }

	void Clear() {

		fY[0] = fY[1] = 0; }

	void SetBandpass(double const f, double const w) {

		double const pidsr = kPi / (double)g_iSampleRate;
		double const c = 1.0 / tan(pidsr * w);
		double const d = 2.0 * cos(2.0 * pidsr * f);

		fA[0] = (float)(1.0 / (1.0 + c));
		fA[1] = 0;
		fA[2] = (float)(-fA[0]);
		fA[3] = (float)(- c * d * fA[0]);
		fA[4] = (float)((c - 1.0) * fA[0]); }

#pragma optimize ("a", on)

	void WorkSamples(float *pout, int ns) {

		double const a  = fA[0];
		double const a1 = fA[1];
		double const a2 = fA[2];
		double const a3 = fA[3];
		double const a4 = fA[4];

		double y1 = fY[1];
		double y0 = fY[0];

		do {

			double const t = *pout - a3 * y0 - a4 * y1;
			double const y = a * t + a1 * y0 + a2 * y1;
			y1 = y0;
			y0 = t;
			*pout++ = (float)y;

		} while(--ns);

		fY[1] = (float)y1;
		fY[0] = (float)y0; }

	void WorkSamplesDest(float const *pin, float *pout, int ns) {

		double const a  = fA[0];
		double const a1 = fA[1];
		double const a2 = fA[2];
		double const a3 = fA[3];
		double const a4 = fA[4];

		double y1 = fY[1];
		double y0 = fY[0];

		do {

			double const t = *pin++ - a3 * y0 - a4 * y1;
			double const y = a * t + a1 * y0 + a2 * y1;
			y1 = y0;
			y0 = t;
			*pout++ = (float)y;

		} while(--ns);

		fY[1] = (float)y1;
		fY[0] = (float)y0; }

#pragma optimize ("a", off)
};

/*	FilterResonance
 *
 *		Two step IIR resonant filter
 */

struct FilterResonance {

	double			 fY1,fY2,fX1;		// Coeffs
	double			 fPrevY[2];			// Past Inputs
	double			 fPrevX[2];			// Past Outputs

	FilterResonance() {

		Clear(); }

	void Clear() {

		fPrevY[0] = 0;
		fPrevY[1] = 0;
		fPrevX[0] = 0; }

	void Set(double const f, double const r) {

		double const p2T = 2.0 * kPi / g_iSampleRate;

		fY1 = 2.0 * r * cos(f * p2T);
		fY2 = - r * r;
		fX1 = - cos(f * p2T);
	
		Clear(); }

#pragma optimize ("a", on)

	void WorkSamples(float *pb, int ns) {

		double const x1c = fX1;
		double const y1c = fY1;
		double const y2c = fY2;

		double x1 = fPrevX[0];
		double y1 = fPrevY[0];
		double y2 = fPrevY[1];

		do {

			double const x = *pb;
			double const y = y1c * y1 + y2c * y2 + x + x1c * x1;
			y2 = y1; y1 = y; x1 = x;
			*pb++ = (float)y;

		} while(--ns);

		fPrevX[0] = x1;
		fPrevY[0] = y1;
		fPrevY[1] = y2; }

#pragma optimize ("a", off)
};


