// SRF_2PFilter.cpp: implementation of the C2PFilter class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <memory.h>
#include "SRF_2PFilter.h"

using namespace SurfDSPLib;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const C2PFilter::BIQUAD	C2PFilter::ProtoCoef[FILTER_SECTIONS]=
{
	{
		/* Section 1 */		
		1.0f,
		0,
		0,
		1.0f,
		0.765367f,
		1.0f,
	},

	{
		/* Section 2 */		
		1.0f,
		0,
		0,
		1.0f,
		1.847759f,
		1.0f,
	}
};

void	C2PFilter::Reset()
{
	for( int i=0; i<2; i+=1 )
	{
		int	 j;
		for( j=0; j<2*FILTER_SECTIONS; j+=1 )
			iirs[i].history[j]=0;
		for( j=0; j<4*FILTER_SECTIONS+1; j+=1 )
			iirs[i].coef[j]=0;
	}

	m_eMode=FILTER_LOWPASS;
	Resonance = 1.0f;						/* preset Resonance */
	Cutoff = 5000;						/* preset Filter cutoff (Hz) */
	CutoffTarget = 5000;
	CutoffAdd = 0;
	Force = true;				// make sure filter gets set up first time

	m_oBypass=true;

	m_fSampleRate=44100;
	m_fSampleRateSquared=m_fSampleRate*m_fSampleRate;

	RecalcWP();
}

C2PFilter::C2PFilter()
{
	Reset();
}

C2PFilter::~C2PFilter()
{
}

void	C2PFilter::prewarp(	float *r1, float *r2, const float &a1, const float &a2 )
{
	*r2 = a2 / (m_fWP * m_fWP);		// could turn one of these divides into a multiply
	*r1 = a1 / m_fWP;
}

void	C2PFilter::bilinear(
	const float &a0, const float &a1, const float &a2,
	const float &b0, const float &b1, const float &b2,
    float *k,           /* overall gain factor */
    float *coef         /* pointer to 4 iir coefficients */
)
{
    float ad, bd;
	ad = (4.f * a2 * m_fSampleRateSquared + 2.f * a1 * m_fSampleRate + a0);		// could get rid of a few of these multiplies
    bd = (4.f * b2 * m_fSampleRateSquared + 2.f * b1 * m_fSampleRate + b0);
    *k *= ad/bd;
    *coef++ = ((2.f * b0 - 8.f * b2 * m_fSampleRateSquared) / bd);			/* beta1 */
    *coef++ = ((4.f * b2 * m_fSampleRateSquared - 2.f * b1 * m_fSampleRate + b0) / bd); /* beta2 */
    *coef++ = ((2.f * a0 - 8.f * a2 * m_fSampleRateSquared) / ad);			/* alpha1 */
    *coef = ((4.f * a2 * m_fSampleRateSquared - 2.f * a1 * m_fSampleRate + a0) / ad);	/* alpha2 */
}

void	C2PFilter::szxform( const C2PFilter::BIQUAD &Coef, const float &i_b1,		/* denominator coefficients */
    float *k,								/* overall gain factor */
    float *coef)							/* pointer to 4 iir coefficients */
{
	float	a1, a2;
	float	b1, b2;
	prewarp( &a1, &a2, Coef.a1, Coef.a2 );
	prewarp( &b1, &b2, i_b1, Coef.b2 );
	bilinear( Coef.a0, a1, a2, Coef.b0, b1, b2, k, coef);
}

/*
 * --------------------------------------------------------------------
 * 
 * iir_filter - Perform IIR filtering sample by sample on floats
 * 
 * Implements cascaded direct form II second order sections.
 * Requires FILTER structure for history and coefficients.
 * The length in the filter structure specifies the number of sections.
 * The size of the history array is 2*iir->length.
 * The size of the coefficient array is 4*iir->length + 1 because
 * the first coefficient is the overall scale factor for the filter.
 * Returns one output sample for each input sample.  Allocates history
 * array if not previously allocated.
 * 
 * float iir_filter(float input,FILTER *iir)
 * 
 *     float input        new float input sample
 *     FILTER *iir        pointer to FILTER structure
 * 
 * Returns float value giving the current output.
 * 
 * Allocation errors cause an error message and a call to exit.
 * --------------------------------------------------------------------
 */
float	C2PFilter::iir_filter( float input, FILTER &iir )
{
    unsigned int i;
    float *hist1_ptr,*hist2_ptr;
	float *coef_ptr;
    float output,new_hist,history1,history2;
	float		*coef;
	unsigned	nInd;
	float	k, r;

	if( CutoffAdd!=0.0f ||  Force )
	{
		bool	doit=false;

		if(CutoffAdd > 0.0)
		{
			Cutoff += CutoffAdd;
			if(Cutoff >= CutoffTarget)
			{
				Cutoff = CutoffTarget;
				CutoffAdd = 0;
			}
			RecalcWP();
			doit = true;
		}
		else if(CutoffAdd < 0.0)
		{
			Cutoff += CutoffAdd;
			if(Cutoff <= CutoffTarget)
			{
				Cutoff = CutoffTarget;
				CutoffAdd = 0;
			}
			RecalcWP();
			doit = true;
		}

		if( doit||Force )
		{
			if( Cutoff<475 )
			{
				r = (1.0f / Resonance) * ( ((475.0f-Cutoff)/475.0f)*10.0f);
				if(r > 1.0f)
					r = 1.0f;
			}
			else
				r = (1.0f/Resonance);
			k = 1.0f;
			coef = iir.coef + 1;	
			for (nInd = 0; nInd < FILTER_SECTIONS; nInd++)
			{
				szxform( ProtoCoef[nInd], ProtoCoef[nInd].b1 * r, &k, coef);
				coef += 4;							
			}
			iir.coef[0] = k;

			Force = false;
		}
	}

    coef_ptr = iir.coef;                /* coefficient pointer */

    hist1_ptr = iir.history;            /* first history */
    hist2_ptr = hist1_ptr + 1;           /* next history */

    output =(float) (input * (*coef_ptr++));

    for (i = 0 ; i < FILTER_SECTIONS; i++)
	{
        history1 = *hist1_ptr;							/* history values */
        history2 = *hist2_ptr;

        output = (float) (output - history1 * (*coef_ptr++));
        new_hist = (float) (output - history2 * (*coef_ptr++));    /* poles */

        output = (float) (new_hist + history1 * (*coef_ptr++));
        output = (float) (output + history2 * (*coef_ptr++));      /* zeros */

        *hist2_ptr++ = *hist1_ptr;
        *hist1_ptr++ = new_hist;
        hist1_ptr++;
        hist2_ptr++;
    }

	if( m_eMode==FILTER_LOWPASS )
		return(output);
	else
		return(input-output);
}

void	C2PFilter::Filter_Mono( float *pOut, float *pIn, int iCount )
{
	if( m_oBypass )
	{
		if( pOut!=pIn )
			memcpy( pOut, pIn, iCount*sizeof(float) );

		return;
	}

	while( iCount-- )
	{
		*pOut++ = iir_filter( *pIn++, iirs[0] );
	}
}

void	C2PFilter::Filter_Stereo( float *pOut, float *pIn, int iCount )
{
	if( m_oBypass )
	{
		if( pOut!=pIn )
			memcpy( pOut, pIn, iCount*sizeof(float)*2 );

		return;
	}

	while( iCount-- )
	{
		*pOut++ = iir_filter( *pIn++, iirs[0] );
		*pOut++ = iir_filter( *pIn++, iirs[1] );
	}
}

void	C2PFilter::SetCutOff( float fFreq )
{
	if( fFreq>m_fSampleRate/2 )
		fFreq=float(m_fSampleRate/2);
	if( fFreq<20 )
		fFreq=20;

	CutoffTarget=fFreq;

	if( m_iInertia == 0 )
	{
		Cutoff = CutoffTarget;
		CutoffAdd = 0.0;
		RecalcWP();
	}
	else
	{
		CutoffAdd = (CutoffTarget-Cutoff)/m_iInertia;
		if( CutoffAdd>20.0f )
			CutoffAdd=20.0f;
		else if( CutoffAdd<-20.0f )
			CutoffAdd=-20.0f;
	}

	Force = true;
}

void	C2PFilter::SetResonance( float fRez )
{
	Resonance = fRez;
	Force = true;
}
