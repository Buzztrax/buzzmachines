/*
 *		Dsp Classes
 *
 *			Written by George Nicolaidis aka Geonik
 */

#pragma once

#include <math.h>


//	Useful constants

double const kPi = 3.14159265358979323846;


//	Calling application should have this global defined

extern int g_iSampleRate;


//	Useful inline functions

inline int DspFastD2I(double n) {
	double const d2i = (1.5 * (1 << 26) * (1 << 26));
	union {
	  int i;
	  double n;
	} u;
	u.n = n + d2i;
	return (u.i);
	//double n_ = n + d2i;
	//return *(int *)(void *)&n_;
}


inline double DspFastRand() {
	static long stat = 0x16BA2118;
	stat = stat * 1103515245 + 12345;
	return (double)stat * (1.0 / 0x80000000); }


inline double DspFastRand(double a)			{ return a * DspFastRand(); }


inline double DspDbToAmplitude(double db)	{ return pow(10,db*(1/20.0)); }

inline double DspAmplitudeToDb(double a)	{ return 20*log10(a); }


inline double DspCalcLogStep(double const from, double const to, double const invtime) { return pow(to / from, invtime); }

inline double DspCalcLinStep(double const from, double const to, double const invtime) { return (to-from) * invtime; }


template <class T> inline void	DspSwap  (T &a, T &b)		{ T t = a; a = b; b = t; }
template <class T> inline void	DspClamp (T &x, T a, T b)	{ x = (x<a ? a : x>b ? b : x); }
template <class T> inline T		DspMinMax(T x, T a, T b)	{ return (x<a ? a : x>b ? b : x); }


//	Performance counting

#pragma warning(disable:4035)
inline unsigned long __fastcall DspCpuCounter() {
#if 0
  __asm {
		_emit 0x0f		; rdtsc
		_emit 0x31 }
#else
  // FIXME:
  return 0UL;
#endif
}
#pragma warning(default:4035)

struct PerfSession { unsigned long perf;
	PerfSession()			{ perf = DspCpuCounter(); }
	unsigned long Elapsed()	{ return DspCpuCounter() - perf; } };


//	Declarations of functions in DspClasses.cpp

void		 DspAdd(float *pout, float *pin, int const ns);
void		 DspRemoveDcOffset(float * const pb, int const ns);

void		 DspNormalize(float * const pb, int const ns, double const a = 1.);
double		 DspNormalDist(const double m, const double s);

const char	*DspMidiNoteToStr(int const n);

double		 DspCalcNoteFreq(const float *scale, int const note, double const det);


//	Functions in WinBase.cpp

//__int64		 DspGetCpuTickCount();


//	Functions in Sqrt.cpp

void		 DspFastSqrtInit();
double		 DspFastSqrt(const double x);


//	Functions in InvSqrt.cpp

void		 DspFastInvSqrtInit();
double		 DspFastInvSqrt(float const x);


//	Declarations of Data

extern const char	dsp_aNoteNames[];
extern const float	dsp_aEqualTemperedScale[128];


// fpu control
// http://www.christian-seiler.de/projekte/fpmath/

#ifdef WIN32
static int __old_fpu_state=0;

inline void DspFPUConfigRoundDown(void) {
__old_fpu_state = _control87(0,0);
_control87(_RC_DOWN,_MCW_RC);
}

inline void DspFPUConfigReset(void) {
  _control87(__old_fpu_state,_MCW_RC);
}
#else
#include <fpu_control.h>

static fpu_control_t __old_fpu_state=_FPU_DEFAULT;

inline void DspFPUConfigRoundDown(void) {
  fpu_control_t fpu_state;
  
  _FPU_GETCW(__old_fpu_state);
  fpu_state=__old_fpu_state|_FPU_RC_DOWN;
  _FPU_SETCW(fpu_state);
}

inline void DspFPUConfigReset(void) {
  _FPU_SETCW(__old_fpu_state);
  __old_fpu_state=_FPU_DEFAULT;
}

#endif
