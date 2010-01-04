/*
 *		Dsp Classes
 *
 *			Written by George Nicolaidis aka Geonik
 */

#ifndef inc_dspcFft
#define inc_dspcFft

#include "Buffer.h"


/*	CFft
 *
 *		Fast Fourier Transform
 *
 *		Based on code by ReliableSoftware
 */

struct CFft {

	CBuffer	 cBuf;

	int		 iPoints;
	double	 f1oPoints;
	double	 fHzPerPoint;
	int		 iLogPoints;
	int		 iSqrtPoints;
	double	 f1oSqrtPoints;
	int		 iSampleRate;
	double	 f1oSampleRate;

	int		*aBitRev;
	float	*aFft;
	float  **aExp;
	float	*aAbs;

					 CFft();
					~CFft();
	void			 Init(int points,int const sr);
	void			 ReInit_Points(int const points);
	void			 Free();
    double			 GetIntensity(int const i);
	double			 GetFrequency(int const point);
    int				 HzToPoint(int const freq); 

	inline void		 FillFft();
	inline void		 Transform();
	inline void		 MapIntensities(double g);
	inline void		 ScaleFft();
	inline void		 InvTransform();
	inline void		 GetSamples(float *pb, int ns);
};


CFft::CFft() {
 
		// Constructor

	}


CFft::~CFft() {
	
		// Destructor
	
	Free(); }


void CFft::Init(int points,int const sr) {

		// Allocate and initialize the buffers

	iPoints = points;
	f1oPoints = 1.0 / (double)iPoints;
	iSqrtPoints = (int)sqrt((double)iPoints);
	f1oSqrtPoints = 1.0 / (float)iSqrtPoints;
	iSampleRate = sr;
	f1oSampleRate = 1.0 / (float)sr;
	fHzPerPoint = (float)sr / (float)iPoints;

	// Make the buffer
	cBuf.Init(iPoints);
	cBuf.Clear();

	// Calc LogPoints
	iLogPoints = 0;
	points--;
	while(points != 0) { points >>= 1; iLogPoints++; }

	aBitRev = new int [iPoints];

	aFft = new float [iPoints * 2];		// Re of point i at 2*i, Im at 2*i+1
	aExp = new float*[iLogPoints+1];
	aAbs = new float [iPoints];

	// Precompute complex exponentials
	int _2_l = 2;
	for (int l=1; l <= iLogPoints; l++) {
		aExp[l] = new float [iPoints*2];

		for (int i=0; i < iPoints; i++) {
			aExp[l][i*2]   = (float)( cos (2. * PI * i / _2_l));
			aExp[l][i*2+1] = (float)(-sin (2. * PI * i / _2_l)); }
		_2_l *= 2; }

	// Set up bit reverse mapping
	int rev = 0;
	int halfPoints = iPoints/2;
	for (int i=0; i < iPoints - 1; i++) {
		aBitRev[i] = rev * 2;					// Double for complex numbers
		int mask = halfPoints;
		while (rev >= mask) {
			rev -= mask; // turn off this bit
			mask >>= 1;	}
		rev += mask; }
	aBitRev[iPoints-1] = (iPoints-1)*2; }


void CFft::ReInit_Points(int const points) {

		// Number of points has changed

	Free();
	Init(points,iSampleRate); }


void CFft::Free() {

		// Cleanup everything

	cBuf.Free();

	if(aBitRev) {
		delete[] aBitRev;
		aBitRev = NULL; }

	if(aAbs) {
		delete[] aAbs;
		aAbs = NULL; }

	if(aExp) {
		for(int l=1; l <= iLogPoints; l++) delete[] aExp[l];
		delete[] aExp;
		aExp = NULL; }

	if(aFft) {
		delete[] aFft;
		aFft = NULL; } }


double CFft::GetIntensity(int const i) {

		// Return the intensity (absolute value) of point i

	return sqrt(aFft[i*2]*aFft[i*2] + aFft[i*2+1]*aFft[i*2+1]) * f1oSqrtPoints; }


inline void CFft::MapIntensities(double g) {
	
		// Map the intensities of each point in the table aAbs
		// Scale them by g
	
	float *a = aAbs;
	float *f = aFft;
	int    c = iPoints;
	g *= f1oSqrtPoints;
	while(c--) {
		double const r = *f++;
		double const i = *f++;
		*a++ = (float)(g * sqrt(r*r + i*i)); } }


double CFft::GetFrequency(int const point) {

		// Return the frequency of point
	
	return fHzPerPoint * ((double)point + 0.5); }


int CFft::HzToPoint(int const freq) { 

		// Find the point that is closest to frequency freq

    return (int)(iPoints * freq * f1oSampleRate); }


inline void CFft::FillFft() {

		// Copies circular buffer to the aFft array

	float	 *b = cBuf.pBuffer + cBuf.iWritePos;
	int		 *r = aBitRev;

	int c = cBuf.iLength - cBuf.iWritePos;

	while(c--) {
		int const p = *r++;
	    aFft[p  ] = *b++;
		aFft[p+1] = 0; }

	c = cBuf.iWritePos;
	b = cBuf.pBuffer;

	while(c--) {
		int const p = *r++;
	    aFft[p  ] = *b++;
		aFft[p+1] = 0; } }


inline void CFft::GetSamples(float *pb, int ns) {

		// Get ns samples from the aFft array
		// ns Must be <= than iPoints

	float *s = aFft;
	while(ns--) {
		*pb++ = *s++; s++; } }


inline void CFft::Transform () {

		// FFT transform of the aFft array

	int step = 1;
	for(int level=1; level <= iLogPoints; level++) {
		int const increm = step * 2;
		for (int j = 0; j < step; j++) {
			double const Ur = aExp[level][j*2];
			double const Ui = aExp[level][j*2+1];
			for (int i = j; i < iPoints; i += increm) {
				double		 Tr = Ur;
				double		 Ti = Ui;
				int const	 c = (i+step)*2;
				double const t = Tr * aFft[c] - Ti * aFft[c+1];
				Ti = Tr * aFft[c+1] + Ti * aFft[c];
				Tr = t;
				aFft[c]		 = (float)(aFft[i*2]   - Tr);
				aFft[c+1]	 = (float)(aFft[i*2+1] - Ti);
				aFft[i*2]	+= (float)Tr;
				aFft[i*2+1]	+= (float)Ti; } }
		step *= 2; } }


inline void CFft::InvTransform () {

		// Inverse FFT transform of the aFft array (not working)

	int step = 1;
	for(int level=1; level <= iLogPoints; level++) {
		int const increm = step * 2;
		for (int j = 0; j < step; j++) {
			double const Ur =  aExp[level][j*2];
			double const Ui = -aExp[level][j*2+1];			// Inverse
			for (int i = j; i < iPoints; i += increm) {
				double		 Tr = Ur;
				double		 Ti = Ui;
				int const	 c = (i+step)*2;
				double const t = Tr * aFft[c] - Ti * aFft[c+1];
				Ti = Tr * aFft[c+1] + Ti * aFft[c];
				Tr = t;
				aFft[c]		 = (float)(aFft[i*2]   - Tr);
				aFft[c+1]	 = (float)(aFft[i*2+1] - Ti);
				aFft[i*2]	+= (float)Tr;
				aFft[i*2+1]	+= (float)Ti; } }
		step *= 2; } }


inline void CFft::ScaleFft() {
	
		// Map the intensities of each point in the table aAbs
		// Scale them by g
	
	float *f = aFft;
	int c = iPoints >> 2;
	double const g = f1oPoints;
	while(c--) {
		*f++ *= (float)g;		// Faster
		*f++ *= (float)g;
		*f++ *= (float)g;
		*f++ *= (float)g;
		*f++ *= (float)g;
		*f++ *= (float)g;
		*f++ *= (float)g;
		*f++ *= (float)g; } }


#endif
