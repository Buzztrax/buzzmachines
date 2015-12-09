#include	"ExtResampler.h"

#ifdef WIN32
typedef	unsigned __int64		u_llong;
typedef	signed __int64		llong;
#else
typedef unsigned long long 	u_llong;
typedef signed long long 	llong;
#endif

static	void	resample_with_reverse( float *pout, int numsamples, CExtResamplerState &state, CExtResamplerParams const &params )
{
	if( state.Active )
	{
		llong	d=((llong(params.StepInt)<<RS_STEP_FRAC_BITS)+params.StepFrac);
		llong	p=((llong(state.PosInt)<<RS_STEP_FRAC_BITS)+state.PosFrac);

		if( p+d*numsamples>=0 )
		{
			DSP_Resample( pout, numsamples, state, params  );
		}
		else
		{
			while( state.Active && numsamples>0 )
			{
				int	n;
				
				if( (params.LoopBegin!=-1) && (p>=(llong(params.LoopBegin)<<RS_STEP_FRAC_BITS)) )
					n=int((p-(llong(params.LoopBegin)<<RS_STEP_FRAC_BITS))/-d);
				else
					n=int(p/-d);

				if( numsamples<n )
					n=numsamples;

				if( n>0 )
				{
					DSP_Resample( pout, n, state, params  );
					numsamples-=n;
					if( params.LoopBegin!=-1 && (((llong(state.PosInt)<<RS_STEP_FRAC_BITS)+state.PosFrac)<=(llong(params.LoopBegin)<<RS_STEP_FRAC_BITS)) )
					{
                      // FIXME:
                         // there is no SetPos in resample.h
						//state.SetPos(params.numSamples);
                        state.PosInt=params.numSamples;
                        state.PosFrac=0;
					}
				}
				else
				{
					state.Active=false;
				}
			}
		}
	}
}

void	EXTDSP_Resample( float *pout, int numsamples, CExtResamplerState &state, CExtResamplerParams const &params )
{
	if( state.Active )
	{
		if( params.AmpMode==RSA_LINEAR_INTP )
		{
			int	n;

			n=int((params.DestAmp-state.Amp)/params.AmpStep);

			if( numsamples<n )
				n=numsamples;

			if( n>0 )
			{
				resample_with_reverse( pout, n, state, params );
				numsamples-=n;
			}

			if( numsamples>0 )
			{
				state.Amp=params.DestAmp;
				CExtResamplerParams	p=params;
				p.AmpMode=RSA_CONSTANT;
				p.AmpStep=0.0f;
				resample_with_reverse( pout, numsamples, state, p );
			}
		}
		else
		{
			resample_with_reverse( pout, numsamples, state, params );
		}
	}
}

