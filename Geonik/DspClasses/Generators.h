/*
 *		Dsp Classes	: Generators
 *
 *			Written by George Nicolaidis aka Geonik
 */

#pragma once


//	MagicCircle : Sine generator

struct MagicCircle {

	double	fFactor;
	double	fX;
	double	fY;

	void SetFrequency(double freq) {

		fFactor = 2.0*sin(kPi*freq/g_iSampleRate); }

	void Reset(double amp=1.0, double y=0.0) {

		fX = amp;
		fY = y; }

	void WorkSamples(float *pout, int ns) {

		double const e = fFactor;
		double		 x = fX;
		double		 y = fY;
		do {
			x += e*y;
			y -= e*x;
			*pout++ = (float)y; }
		while(--ns);
		fX = x;
		fY = y; }

	void WorkSamplesAdd(float *pout, int ns) {

		double const e = fFactor;
		double		 x = fX;
		double		 y = fY;
		do {
			x += e*y;
			y -= e*x;
			*pout++ += (float)y; }
		while(--ns);
		fX = x;
		fY = y; }
 };

