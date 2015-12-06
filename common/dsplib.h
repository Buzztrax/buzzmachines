#ifndef __BUZZ_DSPLIB_H
#define __BUZZ_DSPLIB_H

/*

	How to use dsplib in Buzz machines:

		Add dsplib.lib to library modules list of your project and #include
		this file. After that you can simply call the functions you need.

		Note: Some of these functions may not be implemented yet. You will
		get a link error if you try to use them.

	Descriptions for the abbrvs. used in this file:

		ps - pointer to samples
		pin - pointer to input samples
		pout - pointer to output samples
		n - number of samples
		a - amplitude scaling factor
		la - left a
		ra - right a
		M - mono
		2 - to
		S - stereo



*/

typedef unsigned long dword;

#ifdef _MSC_VER
#if defined(DI_EXPORT)
    #define DI __declspec(dllexport)
#else
    #define DI __declspec(dllimport)
#endif
#if defined(_M_X86)
#define DSPCALL __fastcall
#else
#define DSPCALL
#endif
#else
#define DI
#define DSPCALL
#endif

// initialization

// machines that link dsplib statically need to call DSP_Init()
DI void DSPCALL DSP_Init(int const samplerate);

// basic stuff

DI void DSPCALL DSP_Zero(float *ps, dword const n);

DI void DSPCALL DSP_Copy(float *pout, float const *pin, dword const n);
DI void DSPCALL DSP_Copy(float *pout, float const *pin, dword const n, float const a);

DI void DSPCALL DSP_CopyM2S(float *pout, float const *pin, dword const n);
DI void DSPCALL DSP_CopyM2S(float *pout, float const *pin, dword const n, float const a);
DI void DSPCALL DSP_CopyM2S(float *pout, float const *pin, dword const n, float const la, float const ra);

DI void DSPCALL DSP_CopyS2MOneChannel(float *pout, float const *pin, dword const n, float const a);

DI void DSPCALL DSP_Add(float *pout, float const *pin, dword const n);
DI void DSPCALL DSP_Add(float *pout, float const *pin, dword const n, float const a);

DI void DSPCALL DSP_AddM2S(float *pout, float const *pin, dword const n);
DI void DSPCALL DSP_AddM2S(float *pout, float const *pin, dword const n, float const a);
DI void DSPCALL DSP_AddM2S(float *pout, float const *pin, dword const n, float const la, float const ra);

DI void DSPCALL DSP_AddS2S(float *pout, float const *pin, dword const n);
DI void DSPCALL DSP_AddS2S(float *pout, float const *pin, dword const n, float const a);
DI void DSPCALL DSP_AddS2S(float *pout, float const *pin, dword const n, float const la, float const ra);

DI void DSPCALL DSP_AddS2MOneChannel(float *pout, float const *pin, dword const n, float const a);
DI void DSPCALL DSP_AddS2SOneChannel(float *pout, float const *pin, dword const n, float const a);

DI void DSPCALL DSP_Amp(float *ps, dword const n, float const a);

// second order butterworth filters

#include "bw.h"

DI void DSPCALL DSP_BW_Reset(CBWState &s);	// clears past inputs & outputs

DI void DSPCALL DSP_BW_InitLowpass(CBWState &s, float const f);
DI void DSPCALL DSP_BW_InitHighpass(CBWState &s, float const f);
DI void DSPCALL DSP_BW_InitBandpass(CBWState &s, float const f, float const bw);
DI void DSPCALL DSP_BW_InitBandreject(CBWState &s, float const f, float const bw);

DI bool DSPCALL DSP_BW_Work(CBWState &s, float *ps, dword const n, int const mode);
DI bool DSPCALL DSP_BW_WorkStereo(CBWState &s, float *ps, dword const n, int const mode);

// resampler

#include "resample.h"

DI void DSPCALL DSP_Resample(float *pout, int numsamples, CResamplerState &state, CResamplerParams const &params);

#undef DI

#endif
