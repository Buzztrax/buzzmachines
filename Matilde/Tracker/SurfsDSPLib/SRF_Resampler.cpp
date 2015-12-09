#include <limits>
#include	<stdlib.h>
#include	"SRF_DSP.h"

using namespace SurfDSPLib;

const long	MAXFRACTION=0xFFFFFF;
const int		SHIFTFRACTION=24;

#define	POINTERADD(p,n,mu)		(u_long(p)+((n)*(mu)))
#define	POINTERSUB(p1,p2,mu)	((u_long(p1)-u_long(p2))/(mu))

static inline float	fscale( float r, long i ) {
	//i=(*(long *)&r)+((i&0xFF)<<23);
	//return *(float *)&i;
	return r * (1 << i);
}

static inline stereofloat fscale( stereofloat r, long i ) {
	stereofloat z;
	/*long t = (*(long *)&r.l)+((i&0xFF)<<23);
	z.l = *(float *)&t;
	t = (*(long *)&r.r)+((i&0xFF)<<23);
	z.r = *(float *)&t;*/
	z.l = r.l * (1 << i);
	z.r = r.r * (1 << i);
	return z;
}

static	u_char	gSampleSizes[16]=
{
	1, // SMP_SIGNED8=0,
	1, // SMP_SIGNED8SWAPPED=1,
	4, // SMP_FLOAT=2,
	2, // SMP_SIGNED16=3,
	3, // SMP_SIGNED24=4,
	4, // SMP_SIGNED32=5,

	0,	// 6 - unused
	0,	// 7 - unused

	2, // SMP_SIGNED8_STEREO=SMP_SIGNED8|SMP_FLAG_STEREO,
	2, // SMP_SIGNED8SWAPPED_STEREO=SMP_SIGNED8SWAPPED|SMP_FLAG_STEREO,
	8, // SMP_FLOAT_STEREO=SMP_FLOAT|SMP_FLAG_STEREO,
	4, // SMP_SIGNED16_STEREO=SMP_SIGNED16|SMP_FLAG_STEREO,
	6, // SMP_SIGNED24_STEREO=SMP_SIGNED24|SMP_FLAG_STEREO,
	8, // SMP_SIGNED32_STEREO=SMP_SIGNED32|SMP_FLAG_STEREO,

};

void	CResampler::CLocation::AdvanceLocation( int i )
{
	m_pStart=(void *)POINTERADD(m_pStart,i,gSampleSizes[m_eFormat]);
}

void	CResampler::CLocation::AdvanceEnd( int i )
{
	m_pEnd=(void *)POINTERADD(m_pEnd,i,gSampleSizes[m_eFormat]);
}

long	CResampler::CLocation::GetLength()
{
	return POINTERSUB(m_pEnd,m_pStart,gSampleSizes[m_eFormat]);
}

void	CResampler::Reset()
{
	m_Location.m_pStart=NULL;
	m_iFraction=0;
	m_iDelaySamples=0;
	m_pDoneCallback=NULL;
	m_iRampTime=0;
	m_fLastLeftSample=0;
	m_fLastLeftSampleRamp=0;
	m_fLastRightSample=0;
	m_fLastRightSampleRamp=0;
	m_oPingPongLoop=false;
	m_oForward=true;
}

llong	CResampler::MultiplyFreq( int i )
{
	return llong(m_iFreq)*i;
}

llong	CResampler::GetSamplesToEnd()
{
	llong	t;

	if( m_iFreq>0 )
	{
		t=POINTERSUB(m_Location.m_pEnd,m_Location.m_pStart,gSampleSizes[m_Location.m_eFormat])-1-m_iPosition-(m_oPingPongLoop?1:0);
		t<<=SHIFTFRACTION;
		t-=m_iFraction;
		t+=MAXFRACTION;
		t/=m_iFreq;
	}
	else
	{
		t=m_iPosition;
		t<<=SHIFTFRACTION;
		t+=m_iFraction;
		t=-t/m_iFreq;
	}

	return t+1;
}

void	CResampler::SetFrequency( float f )
{
	// ae: limit for max freq here
	if (f >= 64.0f) f = 64.0f - std::numeric_limits<float>::epsilon();
	m_iFreq=u_long(f*(1<<SHIFTFRACTION));
}

bool	CResampler::Active()
{
	return m_Location.m_pStart && m_iFreq;
}

void CopyStereoChannel(float* pDest, float* pSrc, int iCount, int iDestOfs, int iSrcOfs) {
	pDest += iDestOfs;
	pSrc += iSrcOfs;
	while (iCount--) {
		*pDest++ = *pSrc++;
	}
}

void	CResampler::ResampleToFloatBuffer_Raw( float *pDest, int iCount )
{
	if( m_Location.m_eFiltering==FILTER_SPLINE )
	{
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleTToFloatBuffer_Spline<float, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_FLOAT_STEREO )
			pDest=ResampleTToFloatBuffer_Spline<float, float, stereofloat>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleTToFloatBuffer_Spline<short, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleTToFloatBuffer_Spline<short, float, stereofloat>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24 )
			pDest=ResampleTToFloatBuffer_Spline<S24, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24_STEREO )
			pDest=ResampleTToFloatBuffer_Spline<S24, float, stereofloat>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32 )
			pDest=ResampleTToFloatBuffer_Spline<int, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32_STEREO )
			pDest=ResampleTToFloatBuffer_Spline<int, float, stereofloat>( pDest, iCount );
		else
			pDest=ResampleTToFloatBuffer_Spline<char, float, float>( pDest, iCount );
	}
	else if( m_Location.m_eFiltering==FILTER_LINEAR )
	{
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleTToFloatBuffer_Filter<float, float, float>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_FLOAT_STEREO )
			pDest=ResampleTToFloatBuffer_Filter<float, float, stereofloat>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleTToFloatBuffer_Filter<short, float, float>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleTToFloatBuffer_Filter<short, float, stereofloat>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24 )
			pDest=ResampleTToFloatBuffer_Filter<S24, float, float>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24_STEREO )
			pDest=ResampleTToFloatBuffer_Filter<S24, float, stereofloat>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32 )
			pDest=ResampleTToFloatBuffer_Filter<int, float, float>(pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32_STEREO )
			pDest=ResampleTToFloatBuffer_Filter<int, float, stereofloat>(pDest, iCount );
		else
			pDest=ResampleTToFloatBuffer_Filter<char, float, float>(pDest, iCount );
	}
	else
	{
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleTToFloatBuffer_Normal<float, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_FLOAT_STEREO )
			pDest=ResampleTToFloatBuffer_Normal<float, float, stereofloat>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleTToFloatBuffer_Normal<short, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleTToFloatBuffer_Normal<short, float, stereofloat>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24 )
			pDest=ResampleTToFloatBuffer_Normal<S24, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24_STEREO )
			pDest=ResampleTToFloatBuffer_Normal<S24, float, stereofloat>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32 )
			pDest=ResampleTToFloatBuffer_Normal<int, float, float>( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32_STEREO )
			pDest=ResampleTToFloatBuffer_Normal<int, float, stereofloat>( pDest, iCount );
		else
			pDest=ResampleTToFloatBuffer_Normal<char, float, float>( pDest, iCount );
	}
	m_fMaybeLastLeftSample=pDest[-1];
}

void	CResampler::ResampleToStereoFloatBuffer_Raw( float *pDest, int iCount )
{
	if( m_Location.m_eFiltering==FILTER_SPLINE )
	{
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=(float*)ResampleTToFloatBuffer_Spline<float, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_FLOAT_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Spline<float, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=(float*)ResampleTToFloatBuffer_Spline<short, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Spline<short, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24 ) {
			pDest=(float*)ResampleTToFloatBuffer_Spline<S24, stereofloat, float>((stereofloat*) pDest, iCount );
		} else if( m_Location.m_eFormat==SMP_SIGNED24_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Spline<S24, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32 )
			pDest=(float*)ResampleTToFloatBuffer_Spline<int, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Spline<int, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else {
			pDest=(float*)ResampleTToFloatBuffer_Spline<char, stereofloat, float>((stereofloat*) pDest, iCount );
		}
	}
	else if( m_Location.m_eFiltering==FILTER_LINEAR )
	{
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=(float*)ResampleTToFloatBuffer_Filter<float, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_FLOAT_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Filter<float, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=(float*)ResampleTToFloatBuffer_Filter<short, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Filter<short, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24 )
			pDest=(float*)ResampleTToFloatBuffer_Filter<S24, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Filter<S24, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32 )
			pDest=(float*)ResampleTToFloatBuffer_Filter<int, stereofloat, float>((stereofloat*) pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Filter<int, stereofloat, stereofloat>((stereofloat*) pDest, iCount );
		else {
			pDest=(float*)ResampleTToFloatBuffer_Filter<char, stereofloat, float>((stereofloat*) pDest, iCount );
		}
	}
	else
	{
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=(float*)ResampleTToFloatBuffer_Normal<float, stereofloat, float>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_FLOAT_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Normal<float, stereofloat, stereofloat>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=(float*)ResampleTToFloatBuffer_Normal<short, stereofloat, float>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Normal<short, stereofloat, stereofloat>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24 )
			pDest=(float*)ResampleTToFloatBuffer_Normal<S24, stereofloat, float>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED24_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Normal<S24, stereofloat, stereofloat>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32 )
			pDest=(float*)ResampleTToFloatBuffer_Normal<int, stereofloat, float>((stereofloat*)  pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED32_STEREO )
			pDest=(float*)ResampleTToFloatBuffer_Normal<int, stereofloat, stereofloat>((stereofloat*)  pDest, iCount );
		else {
			pDest=(float*)ResampleTToFloatBuffer_Normal<char, stereofloat, float>((stereofloat*)  pDest, iCount );
		}
	}
	m_fMaybeLastRightSample=pDest[-1];
	m_fMaybeLastLeftSample=pDest[-2];
}

void	CResampler::AddFadeOut( float *pDest, int iCount )
{
	if( m_fLastLeftSampleRamp!=0.0f )
	{
		int	n=int(-m_fLastLeftSample/m_fLastLeftSampleRamp);
		bool	reset=true;

		if( n>iCount )
		{
			n=iCount;
			reset=false;
		}

		while( n>0 )
		{
			*pDest += m_fLastLeftSample;
			m_fLastLeftSample+=m_fLastLeftSampleRamp;
			n-=1;
			pDest+=1;
		}

		if( reset )
		{
			m_fLastLeftSampleRamp=0;
		}
	}
}

void	CResampler::AddFadeOutStereo( float *pDest, int iCount )
{
	if( m_fLastLeftSampleRamp!=0.0f && m_fLastRightSampleRamp!=0.0f )
	{
		int	n=int(-m_fLastLeftSample/m_fLastLeftSampleRamp);
		bool	reset=true;

		if( n>iCount )
		{
			n=iCount;
			reset=false;
		}

		while( n>0 )
		{
			*pDest += m_fLastLeftSample;
			m_fLastLeftSample+=m_fLastLeftSampleRamp;
			pDest+=1;
			*pDest += m_fLastRightSample;
			m_fLastRightSample+=m_fLastRightSampleRamp;
			pDest+=1;
			n-=1;
		}

		if( reset )
		{
			m_fLastLeftSampleRamp=0;
			m_fLastRightSampleRamp=0;
		}
	}
}

void	CResampler::ResampleToFloatBuffer( float *pDest, int iCount )
{
	if( m_Location.m_pStart && m_iFreq )
	{
		int		oldfreq=m_iFreq;

		if( m_oPingPongLoop && !m_oForward )
			m_iFreq=-m_iFreq;

		while( iCount>0 && Active() )
		{
			if( m_iDelaySamples )
			{
				if( m_iDelaySamples<iCount )
				{
					iCount-=m_iDelaySamples;
					ZeroFloat( pDest, m_iDelaySamples );
					pDest+=m_iDelaySamples;

					m_iDelaySamples=0;
				}
				else
				{
					m_iDelaySamples-=iCount;
					return;
				}
			}

			llong	iSampCount=GetSamplesToEnd();

			if( iSampCount>iCount )
				iSampCount=iCount;

			if( iSampCount )
				ResampleToFloatBuffer_Raw( pDest, int(iSampCount) );

			AddFadeOut( pDest, int(iSampCount) );
			pDest+=iSampCount;

			if( (m_iPosition>=m_Location.GetLength()-(m_oPingPongLoop?1:0)) || (m_iPosition<0) )
			{
				if( m_Loop.m_pStart )
				{
					if( m_oPingPongLoop )
					{
						if( m_iPosition>=0 )
						{
							m_iPosition-=m_Location.GetLength()-1;
							int	t=(m_iPosition<<SHIFTFRACTION)|m_iFraction;

							m_iFraction=-t;
							m_iPosition=m_Loop.GetLength()-1+(m_iFraction>>SHIFTFRACTION);
							m_iFraction&=MAXFRACTION;
						}
						else
						{
							int	t=(m_iPosition<<SHIFTFRACTION)|m_iFraction;
							m_iFraction=-t;
							m_iPosition=m_iFraction>>SHIFTFRACTION;
							m_iFraction&=MAXFRACTION;
						}

						m_iFreq=-m_iFreq;
						m_oForward=!m_oForward;

					}
					else
					{
						if( m_iPosition>=0 )
							m_iPosition-=m_Location.GetLength();
						else
							m_iPosition+=m_Loop.GetLength()-1;
					}
					m_Location.m_pStart=m_Loop.m_pStart;
					m_Location.m_pEnd=m_Loop.m_pEnd;
					m_Location.m_eFormat=m_Loop.m_eFormat;
					m_Location.m_eFiltering=m_Loop.m_eFiltering;
				}
				else
				{
					Stop();
				}

				if( m_pDoneCallback )
					m_pDoneCallback( m_iCBData );
			}

			iCount-=int(iSampCount);
		}
		ZeroFloat( pDest, iCount );

		m_iFreq=oldfreq;
	}

	AddFadeOut( pDest, iCount );
}

void	CResampler::Stop()
{
	if( m_Location.m_pStart )
	{
		m_fLastLeftSample=m_fMaybeLastLeftSample;
		m_fMaybeLastLeftSample=0;
		m_fLastRightSample=m_fMaybeLastRightSample;
		m_fMaybeLastRightSample=0;

		if( m_iRampTime )
		{
			m_fLastLeftSampleRamp=-m_fLastLeftSample/m_iRampTime;
			m_fLastRightSampleRamp=-m_fLastRightSample/m_iRampTime;
		}
		else
		{
			m_fLastLeftSampleRamp=0;
			m_fLastRightSampleRamp=0;
		}

		m_Location.m_pStart=NULL;
	}
}

void	CResampler::ResampleToStereoFloatBuffer( float *pDest, int iCount )
{
	if( m_Location.m_pStart && m_iFreq )
	{
		int		oldfreq=m_iFreq;

		if( m_oPingPongLoop && !m_oForward )
			m_iFreq=-m_iFreq;

		while( iCount>0 && Active() )
		{
			if( m_iDelaySamples )
			{
				if( m_iDelaySamples<iCount )
				{
					iCount-=m_iDelaySamples;
					ZeroFloat( pDest, m_iDelaySamples*2 );
					pDest+=m_iDelaySamples*2;

					m_iDelaySamples=0;
				}
				else
				{
					m_iDelaySamples-=iCount;
					return;
				}
			}

			llong	iSampCount=GetSamplesToEnd();

			if( iSampCount>iCount )
				iSampCount=iCount;

			if( iSampCount )
				ResampleToStereoFloatBuffer_Raw( pDest, int(iSampCount) );

			AddFadeOutStereo( pDest, int(iSampCount) );
			pDest+=iSampCount*2;

			if( (m_iPosition>=m_Location.GetLength()-(m_oPingPongLoop?1:0)) || (m_iPosition<0) )
			{
				if( m_Loop.m_pStart )
				{
					if( m_oPingPongLoop )
					{
						if( m_iPosition>=0 )
						{
							m_iPosition-=m_Location.GetLength()-1;
							int	t=(m_iPosition<<SHIFTFRACTION)|m_iFraction;

							m_iFraction=-t;
							m_iPosition=m_Loop.GetLength()-1+(m_iFraction>>SHIFTFRACTION);
							m_iFraction&=MAXFRACTION;
						}
						else
						{
							int	t=(m_iPosition<<SHIFTFRACTION)|m_iFraction;
							m_iFraction=-t;
							m_iPosition=m_iFraction>>SHIFTFRACTION;
							m_iFraction&=MAXFRACTION;
						}

						m_iFreq=-m_iFreq;
						m_oForward=!m_oForward;

					}
					else
					{
						if( m_iPosition>=0 )
							m_iPosition-=m_Location.GetLength();
						else
							m_iPosition+=m_Loop.GetLength()-1;
					}
					m_Location.m_pStart=m_Loop.m_pStart;
					m_Location.m_pEnd=m_Loop.m_pEnd;
					m_Location.m_eFormat=m_Loop.m_eFormat;
					m_Location.m_eFiltering=m_Loop.m_eFiltering;
				}
				else
				{
					Stop();
				}

				if( m_pDoneCallback )
					m_pDoneCallback( m_iCBData );
			}

			iCount-=int(iSampCount);
		}
		ZeroFloat( pDest, iCount*2 );

		m_iFreq=oldfreq;
	}

	AddFadeOutStereo( pDest, iCount );
}

void	CResampler::Skip_Raw( int iCount )
{
	llong	i;
	int	pos;

	i=llong(m_iFreq)*iCount;
	m_iFraction+=u_long(i&MAXFRACTION);
	pos=int((m_iFraction>>SHIFTFRACTION)+(i>>SHIFTFRACTION));
	m_iFraction&=MAXFRACTION;
	m_iPosition+=pos;
}

void	CResampler::Skip( int iCount )
{
	if( m_Location.m_pStart && m_iFreq )
	{
		int		oldfreq=m_iFreq;

		if( m_oPingPongLoop && !m_oForward )
			m_iFreq=-m_iFreq;

		while( iCount>0 && Active() )
		{
			if( m_iDelaySamples )
			{
				if( m_iDelaySamples<iCount )
				{
					iCount-=m_iDelaySamples;
					m_iDelaySamples=0;
				}
				else
				{
					m_iDelaySamples-=iCount;
					return;
				}
			}

			llong	iSampCount=GetSamplesToEnd();
			if( iSampCount>iCount )
				iSampCount=iCount;

			if( iSampCount )
				Skip_Raw( int(iSampCount) );

			if( (m_iPosition>=m_Location.GetLength()-(m_oPingPongLoop?1:0)) || (m_iPosition<0) )
			{
				if( m_Loop.m_pStart )
				{
					if( m_oPingPongLoop )
					{
						if( m_iPosition>=0 )
						{
							m_iPosition-=m_Location.GetLength()-1;
							int	t=(m_iPosition<<SHIFTFRACTION)|m_iFraction;

							m_iFraction=-t;
							m_iPosition=m_Loop.GetLength()-1+(m_iFraction>>SHIFTFRACTION);
							m_iFraction&=MAXFRACTION;
						}
						else
						{
							int	t=(m_iPosition<<SHIFTFRACTION)|m_iFraction;
							m_iFraction=-t;
							m_iPosition=m_iFraction>>SHIFTFRACTION;
							m_iFraction&=MAXFRACTION;
						}

						m_iFreq=-m_iFreq;
						m_oForward=!m_oForward;

					}
					else
					{
						if( m_iPosition>=0 )
							m_iPosition-=m_Location.GetLength();
						else
							m_iPosition+=m_Loop.GetLength()-1;
					}
					m_Location.m_pStart=m_Loop.m_pStart;
					m_Location.m_pEnd=m_Loop.m_pEnd;
					m_Location.m_eFormat=m_Loop.m_eFormat;
					m_Location.m_eFiltering=m_Loop.m_eFiltering;
				}
				else
				{
					m_fLastLeftSample=m_fMaybeLastLeftSample;
					m_fMaybeLastLeftSample=0;
					m_fLastRightSample=m_fMaybeLastRightSample;
					m_fMaybeLastRightSample=0;

					if( m_iRampTime )
					{
						m_fLastLeftSampleRamp=-m_fLastLeftSample/m_iRampTime;
						m_fLastRightSampleRamp=-m_fLastRightSample/m_iRampTime;
					}
					else
					{
						m_fLastLeftSampleRamp=0;
						m_fLastRightSampleRamp=0;
					}

					m_Location.m_pStart=NULL;
				}

				if( m_pDoneCallback )
					m_pDoneCallback( m_iCBData );
			}

			iCount-=int(iSampCount);
		}
		m_iFreq=oldfreq;
	}
}
