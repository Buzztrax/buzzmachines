/*
 *		Dsp Classes	: Buzz Oscillators
 *
 *			Written by George Nicolaidis aka Geonik
 */

#ifndef inc_dspcBuzzOsc
#define inc_dspcBuzzOsc

#include "../DspClasses/DspClasses.h"


/*	CBuzzOsc
 *
 *		Bandlimited oscillators built in Buzz
 */

struct CBuzzOsc {

	int			 iOscTable;
	const short	*pOscTable;
	int			 iLevel;
	const short	*pSubTable;
	int			 iOscMask;
	double		 fPos;
	double		 fRate;

	CBuzzOsc() {

		fPos = 0; }


	void SetTable(CMachineInterface *p, int i) {

		pOscTable = p->pCB->GetOscillatorTable(iOscTable = i); }


	void SetFrequency(double freq) {

		fRate = 2048.0 * freq / g_iSampleRate;

		if((iOscTable != OWF_SINE) && (fRate >= 0.5))
			iLevel = (int)ceil(log(fRate) * (1.0 / log(2.0)));
			else iLevel = 0;

		pSubTable	= pOscTable + GetOscTblOffset(iLevel);
		fRate		= fRate / (1 << iLevel);
		iOscMask	= (2048 >> iLevel) - 1; }


	void Work(float *pout, int ns) {

		DspFPUConfigRoundDown();			
		short const	*ptbl	= pSubTable;
		int const	 omask	= iOscMask;
		double const d2i	= (1.5 * (1 << 26) * (1 << 26));
		double		 pos	= fPos;
		double		 step	= fRate;
		do {
		    union {
		      double res;
		      int    ipos;
		    } type_pun;
		    type_pun.res = pos + d2i;
			double const frac = pos - type_pun.ipos;
			double const s1	  = ptbl[type_pun.ipos & omask];
			double const s2   = ptbl[(type_pun.ipos + 1) & omask];
			*pout++ = (float)((s1 + (s2 - s1) * frac));
			pos += step;
		} while(--ns);
		fPos = pos;
		DspFPUConfigReset(); }
 };


/*	CPwPulse
 *
 *		Bandlimited pulse with pulse width control
 */

struct CPwPulse : public CBuzzOsc {
	double	 fPWidth;
	int		 iDistance;
	double	 fDistance;

	CPwPulse() : CBuzzOsc() {
		fPWidth = 0.5; }

	void Init(CMachineInterface *p) {
		SetTable(p, OWF_SAWTOOTH); }

	void SetPWidth(double w) {
		fPWidth = w; }

	void Update() {
		double d = fPWidth * (2048 >> iLevel);
		iDistance = DspFastD2I(d);
		fDistance = d - iDistance; }

#pragma optimize ("a", on)

	void WorkSamples(float *pout, int ns) {
		DspFPUConfigRoundDown();
		short const	*ptbl	= pSubTable;
		int const	 omask	= iOscMask;
		int const	 dist	= iDistance;
		double const df		= fDistance;
		double const d2i	= (1.5 * (1 << 26) * (1 << 26));
		double		 pos	= fPos;
		double		 step	= fRate;
		do {
		    union {
		      double res;
		      int    ipos;
		    } type_pun;
		    type_pun.res = pos + d2i;
			double const frac = pos - type_pun.ipos;
			double const s1	  = ptbl[type_pun.ipos & omask];
			double const s2   = ptbl[(type_pun.ipos + 1) & omask];
			double const s3	  = ptbl[(type_pun.ipos + dist) & omask];
			double const s4   = ptbl[(type_pun.ipos + dist + 1 )& omask];
			*pout++ = (float)(s1 - s3 + (s2-s1)*frac - (s4-s3)*(frac+df));
			pos += step;
		} while(--ns);
		fPos = pos;
		DspFPUConfigReset(); }

#pragma optimize ("a", off)

 };

#endif
