/*
 *		Dsp Classes
 *
 *			Written by George Nicolaidis aka Geonik
 */

#ifndef inc_dspcDelay
#define inc_dspcDelay

#include "DspClasses.h"


/*	CDelay
 *
 *		Simple delay
 */

struct CDelay {

	float	*pBuffer;
	int		 iLength;
	int		 iMaxLength;
	int		 iPos;

	CDelay() {
		pBuffer = NULL;
		iLength = 0; }

	~CDelay() {
		Free(); }

	virtual void Alloc(int const maxlen) {			// Init(): Allocate delay line
		if(pBuffer)
			assert(maxlen <= iMaxLength);
		else
			pBuffer	= new float[maxlen];
		iLength		= maxlen;
		iMaxLength	= maxlen;
		iPos		= 0; }							//  Warning, no more Clear()

	virtual void Free() {
		iLength = iPos = 0;
		if(pBuffer) delete[] pBuffer;
		pBuffer = NULL; }

	virtual void Clear() {							// Clear(): Zero buffer contents
		memset(pBuffer,0,iLength*sizeof(float)); }

	virtual void SetDelay(int const len) {			// Delay length in samples
		iLength = len;
		if(iLength > iMaxLength) iLength = iMaxLength;
		if(iLength < 2) iLength = 2;
		if(iPos >= iLength) iPos = 0; }

	virtual void SetFrequency(double const freq) {	// Set self-frequency of delay line
		SetDelay((int)(((double)g_iSampleRate / freq) - 0.5)); }

	virtual void ScaleBuffer(double const s) {
		int ns = iLength;
		float *pb = pBuffer;
		do {
			*pb++ *= (float)s;
		} while(--ns); }

	virtual double Work(double const fNew) {		// Work(): Very slow
		double fTemp = pBuffer[iPos];
		pBuffer[iPos++] = (float)fNew;
		if(iPos >= iLength) iPos = 0;
		return fTemp; }
 };


/*	CLiDelay
 *
 *		Delay with lineary interpolated fractional length
 */

struct CLiDelay : public CDelay {
	double	fAlpha;
	double	fAlpha_1m;

	CLiDelay() : CDelay() { }
	
	void SetDelay(int const len) {					// SetDelay(): Integer length
		CDelay::SetDelay(len);
		fAlpha		= 0.0;
		fAlpha_1m	= 1.0; }

	void SetDelay(double const len) {				// SetDelay(): Fractional length
		int const i = DspFastD2I(ceil(len));
		CDelay::SetDelay(i-1);
		fAlpha		= i - len;
		fAlpha_1m	= 1.0 - fAlpha; }

	void SetFrequency(double const freq) {			// Set self-frequency of delay line
		SetDelay((double)g_iSampleRate / freq); }

	double Work(double const fNew) {				// Work(): Slow, untested
		double fTemp = pBuffer[iPos] * fAlpha_1m;
		pBuffer[iPos++] = (float)fNew;
		if(iPos >= iLength) iPos = 0;
		fTemp += pBuffer[iPos] * fAlpha;
		return fTemp; } };


/*	CCircBuffer
 *
 *		Circular buffer
 */

struct CCircBuffer {
	float	*pBuffer;
	int		 iLength;
	int		 iPos;

	CCircBuffer() {
		pBuffer = NULL;
		iLength = 0; }

	~CCircBuffer() {
		Free(); }

	void Alloc(int const len) {				// Alloc(): Allocate buffer
		assert(!pBuffer);
		pBuffer	= new float[len];
		iLength		= len;
		iPos		= 0; }

	void Clear() {							// Clear(): Zero buffer contents
		memset(pBuffer,0,iLength*sizeof(float)); }

	void Free() {
		iLength = iPos = 0;
		if(pBuffer) delete[] pBuffer;
		pBuffer = NULL; }

	void AddData(float *s, int ns) {		// New contents for the buffer
	//	assert(ns <= iLength);
		int	c = min(ns,iLength - iPos);	ns -= c;
		float *b = pBuffer + iPos; iPos += c;
		while(c--) *b++ = *s++;
		if(ns > 0) {
			iPos = ns;
			b = pBuffer;
			while(ns--) *b++ = *s++; } }

	void IterateAll(void *f(const int c, const float s)) {
		float *b = pBuffer + iPos;
		int c;
		for(c = 0; c < iLength - iPos; c++) f(c,*b++);
		for(b = pBuffer; c < iLength; c++) f(c,*b++); } };

#endif