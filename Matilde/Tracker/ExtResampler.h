#ifndef	EXTRESAMPLER_H__
#define	EXTRESAMPLER_H__

#include	 <dsplib.h>

class	CExtResamplerParams : public CResamplerParams
{
public:
	float	DestAmp;
};

class	CExtResamplerState : public CResamplerState
{
public:
};

void	EXTDSP_Resample( float *pout, int numsamples, CExtResamplerState &state, CExtResamplerParams const &params );

#endif
