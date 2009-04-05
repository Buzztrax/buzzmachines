#include	<stdlib.h>
#include	"SRF_DSP.h"

using namespace SurfDSPLib;

const long	MAXFRACTION=0xFFFFFF;
const int		SHIFTFRACTION=24;

#define	POINTERADD(p,n,sh)		(u_long(p)+((n)<<(sh)))
#define	POINTERSUB(p1,p2,sh)	((u_long(p1)-u_long(p2))>>(sh))
#define	READFLOAT(p,n)			(((float *)(p))[n])
#define	READSIGNED8(p,n)		(*((char *)((u_long(p)+(n))^1)))
#define	READSIGNED16(p,n)		(((short *)(p))[n])

#define	READLEFTSIGNED16(p,n)	(((short *)(p))[(n)<<1])
#define	READRIGHTSIGNED16(p,n)	(((short *)(p))[((n)<<1)+1])
#define	READSTEREOSIGNED16(p,n)	((int(READLEFTSIGNED16(p,n))+int(READRIGHTSIGNED16(p,n)))>>1)

static inline int	absint( int i )
{
	return i>=0?i:-i;
}

static inline float	fscale( float r, long i )
{
	//i=(*(long *)&r)+(i<<23);
	i=(*(long *)((void *)&r))+((i&0xFF)<<23);
	return *(float *)((void *)&i);
}

static	u_char	gSampleSizes[8]=
{
	0,
	0,
	2,
	1,
	1,
	1,
	3,
	2,
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
	m_iFreq=u_long(f*(1<<SHIFTFRACTION));
}

bool	CResampler::Active()
{
	return m_Location.m_pStart && m_iFreq;
}

#ifndef	BUZZ
float	*	CResampler::ResampleFloatToFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		*pDest++ = READFLOAT(m_Location.m_pStart,m_iPosition);
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleFloatToStereoFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		float	d=READFLOAT(m_Location.m_pStart,m_iPosition);

		*pDest++ = d;
		*pDest++ = d;
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}
#endif

float	*	CResampler::ResampleSigned16ToFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		*pDest++ = READSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleSigned16ToStereoFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		float	d=READSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);

		*pDest++ = d;
		*pDest++ = d;
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleStereoSigned16ToFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		*pDest++ = READSTEREOSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleStereoSigned16ToStereoFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		*pDest++ = READLEFTSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		*pDest++ = READRIGHTSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

#ifndef	BUZZ
float	*	CResampler::ResampleSigned8ToFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		*pDest++ = READSIGNED8(m_Location.m_pStart,m_iPosition)*(1.0f/128.0f);
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleSigned8ToStereoFloatBuffer_Normal( float *pDest, int iCount )
{
	while( iCount-- )
	{
		float	d=READSIGNED8(m_Location.m_pStart,m_iPosition)*(1.0f/128.0f);

		*pDest++ = d;
		*pDest++ = d;
		m_iFraction+=m_iFreq;
		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleFloatToFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;
	
	while( (iCount>0) && (m_iPosition<iLast) )
	{
		float	s=READFLOAT(m_Location.m_pStart,m_iPosition);
		float	d=READFLOAT(m_Location.m_pStart,m_iPosition+1);

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( iCount>0 )
	{
		float	s=READFLOAT(m_Location.m_pStart,m_iPosition);
		float	d;

		if( m_Loop.m_pStart )
			d=READFLOAT(m_Loop.m_pStart,0);
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleFloatToStereoFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;
	
	while( (iCount>0) && (m_iPosition<iLast) )
	{
		float	s=READFLOAT(m_Location.m_pStart,m_iPosition);
		float	d=READFLOAT(m_Location.m_pStart,m_iPosition+1);

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			float	t=d*m_iFraction+s;
			*pDest++ = t;
			*pDest++ = t;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( iCount>0 )
	{
		float	s=READFLOAT(m_Location.m_pStart,m_iPosition);
		float	d;

		if( m_Loop.m_pStart )
			d=READFLOAT(m_Loop.m_pStart,0);
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			float	t=d*m_iFraction+s;

			*pDest++ = t;
			*pDest++ = t;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}
#endif

float	*	CResampler::ResampleSigned16ToFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;

	while( (iCount>0) && (m_iPosition<iLast) && (m_iPosition>=0) )
	{
		float	s=READSIGNED16(m_Location.m_pStart,m_iPosition)*(1/32768.0f);
		float	d=READSIGNED16(m_Location.m_pStart,m_iPosition+1)*(1/32768.0f);

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( m_iPosition<0 )
		iCount=iCount;

	if( iCount>0 )
	{
		float	s=READSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		float	d;

		if( m_Loop.m_pStart )
			d=READSIGNED16(m_Loop.m_pStart,0)*(1.0f/32768.0f);
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleSigned16ToStereoFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;

	while( (iCount>0) && (m_iPosition<iLast) && (m_iPosition>=0) )
	{
		float	s=READSIGNED16(m_Location.m_pStart,m_iPosition)*(1/32768.0f);
		float	d=READSIGNED16(m_Location.m_pStart,m_iPosition+1)*(1/32768.0f);

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			float	t=d*m_iFraction+s;
			*pDest++ = t;
			*pDest++ = t;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( m_iPosition<0 )
		iCount=iCount;

	if( iCount>0 )
	{
		float	s=READSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		float	d;

		if( m_Loop.m_pStart )
			d=READSIGNED16(m_Loop.m_pStart,0)*(1.0f/32768.0f);
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			float	t=d*m_iFraction+s;
			*pDest++ = t;
			*pDest++ = t;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleStereoSigned16ToFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;

	while( (iCount>0) && (m_iPosition<iLast) && (m_iPosition>=0) )
	{
		float	s=READSTEREOSIGNED16(m_Location.m_pStart,m_iPosition)*(1/32768.0f);
		float	d=READSTEREOSIGNED16(m_Location.m_pStart,m_iPosition+1)*(1/32768.0f);

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( m_iPosition<0 )
		iCount=iCount;

	if( iCount>0 )
	{
		float	s=READSTEREOSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		float	d;

		if( m_Loop.m_pStart )
			d=READSTEREOSIGNED16(m_Loop.m_pStart,0)*(1.0f/32768.0f);
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleStereoSigned16ToStereoFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;

	while( (iCount>0) && (m_iPosition<iLast) && (m_iPosition>=0) )
	{
		float	ls=READLEFTSIGNED16(m_Location.m_pStart,m_iPosition)*(1/32768.0f);
		float	ld=READLEFTSIGNED16(m_Location.m_pStart,m_iPosition+1)*(1/32768.0f);
		float	rs=READRIGHTSIGNED16(m_Location.m_pStart,m_iPosition)*(1/32768.0f);
		float	rd=READRIGHTSIGNED16(m_Location.m_pStart,m_iPosition+1)*(1/32768.0f);

		ld=(ld-ls)*(1.0f/(1<<SHIFTFRACTION));
		rd=(rd-rs)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = ld*m_iFraction+ls;
			*pDest++ = rd*m_iFraction+rs;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( m_iPosition<0 )
		iCount=iCount;

	if( iCount>0 )
	{
		float	ls=READLEFTSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		float	ld;
		float	rs=READRIGHTSIGNED16(m_Location.m_pStart,m_iPosition)*(1.0f/32768.0f);
		float	rd;

		if( m_Loop.m_pStart )
		{
			ld=READLEFTSIGNED16(m_Loop.m_pStart,0)*(1.0f/32768.0f);
			rd=READRIGHTSIGNED16(m_Loop.m_pStart,0)*(1.0f/32768.0f);
		}
		else
		{
			ld=0;
			rd=0;
		}

		ld=(ld-ls)*(1.0f/(1<<SHIFTFRACTION));
		rd=(rd-rs)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = ld*m_iFraction+ls;
			*pDest++ = rd*m_iFraction+rs;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

#ifndef	BUZZ
float	*	CResampler::ResampleSigned8ToFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;

	while( (iCount>0) && (m_iPosition<iLast) )
	{
		float	s=READSIGNED8(m_Location.m_pStart,m_iPosition)/128.0f;
		float	d=READSIGNED8(m_Location.m_pStart,m_iPosition+1)/128.0f;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( iCount>0 )
	{
		float	s=READSIGNED8(m_Location.m_pStart,m_iPosition)/128.0f;
		float	d;

		if( m_Loop.m_pStart )
			d=READSIGNED8(m_Loop.m_pStart,0)/128.0f;
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			*pDest++ = d*m_iFraction+s;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleSigned8ToStereoFloatBuffer_Filter( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;

	while( (iCount>0) && (m_iPosition<iLast) )
	{
		float	s=READSIGNED8(m_Location.m_pStart,m_iPosition)/128.0f;
		float	d=READSIGNED8(m_Location.m_pStart,m_iPosition+1)/128.0f;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			float	t=d*m_iFraction+s;
			*pDest++ = t;
			*pDest++ = t;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	if( iCount>0 )
	{
		float	s=READSIGNED8(m_Location.m_pStart,m_iPosition)/128.0f;
		float	d;

		if( m_Loop.m_pStart )
			d=READSIGNED8(m_Loop.m_pStart,0)/128.0f;
		else
			d=0;

		d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

		while( (m_iFraction<=MAXFRACTION) && iCount-- )
		{
			float	t=d*m_iFraction+s;
			*pDest++ = t;
			*pDest++ = t;
			m_iFraction+=m_iFreq;
		}

		m_iPosition+=m_iFraction>>SHIFTFRACTION;
		m_iFraction&=MAXFRACTION;
	}

	return pDest;
}

float	*	CResampler::ResampleFloatToFloatBuffer_Spline( float *pDest, int iCount )
{
	return ResampleFloatToFloatBuffer_Filter( pDest, iCount );
}
#endif

float	*	CResampler::ResampleSigned16ToFloatBuffer_Spline( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;
	long	iLoopPos=0;

	float	p0,p1,p2,p3;
	int		n0,n1,n2,n3;

	n1=m_iPosition;
	p1=READSIGNED16(m_Location.m_pStart,n1)*(1.0f/32768.0f);
	n0=n1-1;
	if( n0<0 )
		n0=0;
	p0=READSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
	n2=m_iPosition+1;
	if( n2>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			p2=READSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
			p2=0;
	}
	else
		p2=READSIGNED16(m_Location.m_pStart,n2)*(1.0f/32768.0f);
	n3=n2+1;
	if( n3>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			p3=READSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
			p3=0;
	}
	else
		p3=READSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);

	if( m_iFreq>0 )
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	q0=-p0+fscale(p1,1)+p1-fscale(p2,1)-p2+p3;
			float	q1=fscale(p0,1)-fscale(p1,2)-p1+fscale(p2,2)-p3;
			float	q2=-p0+p2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				*pDest++ = ((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt-- )
			{
				p0=p1;
				p1=p2;
				p2=p3;
				n3+=1;
				if( n3>=iLast )
				{
					if( m_Loop.m_pStart )
					{
						p3=READSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
						p3=0;
				}
				else
					p3=READSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}
	else
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	q0=-p0+fscale(p1,1)+p1-fscale(p2,1)-p2+p3;
			float	q1=fscale(p0,1)-fscale(p1,2)-p1+fscale(p2,2)-p3;
			float	q2=-p0+p2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				*pDest++ = ((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt++ )
			{
				p3=p2;
				p2=p1;
				p1=p0;
				n0-=1;
				if( n0<0 )
				{
					if( m_Loop.m_pStart )
					{
						p0=READSIGNED16(m_Loop.m_pEnd,n0)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
						p0=0;
				}
				else
					p0=READSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}

	return pDest;
}

float	*	CResampler::ResampleStereoSigned16ToFloatBuffer_Spline( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;
	long	iLoopPos=0;

	float	p0,p1,p2,p3;
	int		n0,n1,n2,n3;

	n1=m_iPosition;
	p1=READSTEREOSIGNED16(m_Location.m_pStart,n1)*(1.0f/32768.0f);
	n0=n1-1;
	if( n0<0 )
		n0=0;
	p0=READSTEREOSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
	n2=m_iPosition+1;
	if( n2>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			p2=READSTEREOSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
			p2=0;
	}
	else
		p2=READSTEREOSIGNED16(m_Location.m_pStart,n2)*(1.0f/32768.0f);
	n3=n2+1;
	if( n3>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			p3=READSTEREOSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
			p3=0;
	}
	else
		p3=READSTEREOSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);

	if( m_iFreq>0 )
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	q0=-p0+fscale(p1,1)+p1-fscale(p2,1)-p2+p3;
			float	q1=fscale(p0,1)-fscale(p1,2)-p1+fscale(p2,2)-p3;
			float	q2=-p0+p2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				*pDest++ = ((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt-- )
			{
				p0=p1;
				p1=p2;
				p2=p3;
				n3+=1;
				if( n3>=iLast )
				{
					if( m_Loop.m_pStart )
					{
						p3=READSTEREOSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
						p3=0;
				}
				else
					p3=READSTEREOSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}
	else
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	q0=-p0+fscale(p1,1)+p1-fscale(p2,1)-p2+p3;
			float	q1=fscale(p0,1)-fscale(p1,2)-p1+fscale(p2,2)-p3;
			float	q2=-p0+p2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				*pDest++ = ((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt++ )
			{
				p3=p2;
				p2=p1;
				p1=p0;
				n0-=1;
				if( n0<0 )
				{
					if( m_Loop.m_pStart )
					{
						p0=READSTEREOSIGNED16(m_Loop.m_pEnd,n0)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
						p0=0;
				}
				else
					p0=READSTEREOSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}

	return pDest;
}

#ifndef	BUZZ
float	*	CResampler::ResampleSigned8ToFloatBuffer_Spline( float *pDest, int iCount )
{
	return ResampleSigned8ToFloatBuffer_Filter( pDest, iCount );
}

float	*	CResampler::ResampleFloatToStereoFloatBuffer_Spline( float *pDest, int iCount )
{
	return ResampleFloatToStereoFloatBuffer_Filter( pDest, iCount );
}
#endif

float	*	CResampler::ResampleSigned16ToStereoFloatBuffer_Spline( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;
	long	iLoopPos=0;

	float	p0,p1,p2,p3;
	int		n0,n1,n2,n3;

	n1=m_iPosition;
	p1=READSIGNED16(m_Location.m_pStart,n1)*(1.0f/32768.0f);
	n0=n1-1;
	if( n0<0 )
		n0=0;
	p0=READSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
	n2=m_iPosition+1;
	if( n2>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			p2=READSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
			p2=0;
	}
	else
		p2=READSIGNED16(m_Location.m_pStart,n2)*(1.0f/32768.0f);
	n3=n2+1;
	if( n3>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			p3=READSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
			p3=0;
	}
	else
		p3=READSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);

	if( m_iFreq>0 )
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	q0=-p0+fscale(p1,1)+p1-fscale(p2,1)-p2+p3;
			float	q1=fscale(p0,1)-fscale(p1,2)-p1+fscale(p2,2)-p3;
			float	q2=-p0+p2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				float	d=((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
				*pDest++ = d;
				*pDest++ = d;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt-- )
			{
				p0=p1;
				p1=p2;
				p2=p3;
				n3+=1;
				if( n3>=iLast )
				{
					if( m_Loop.m_pStart )
					{
						p3=READSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
						p3=0;
				}
				else
					p3=READSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}
	else
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	q0=-p0+fscale(p1,1)+p1-fscale(p2,1)-p2+p3;
			float	q1=fscale(p0,1)-fscale(p1,2)-p1+fscale(p2,2)-p3;
			float	q2=-p0+p2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				float	d=((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
				*pDest++ = d;
				*pDest++ = d;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt++ )
			{
				p3=p2;
				p2=p1;
				p1=p0;
				n0-=1;
				if( n0<0 )
				{
					if( m_Loop.m_pStart )
					{
						p0=READSIGNED16(m_Loop.m_pEnd,n0)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
						p0=0;
				}
				else
					p0=READSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}

	return pDest;
}

float	*	CResampler::ResampleStereoSigned16ToStereoFloatBuffer_Spline( float *pDest, int iCount )
{
	long	iLast=m_Location.GetLength()-1;
	long	iLoopPos=0;

	float	lp0,lp1,lp2,lp3;
	float	rp0,rp1,rp2,rp3;
	int		n0,n1,n2,n3;

	n1=m_iPosition;
	lp1=READLEFTSIGNED16(m_Location.m_pStart,n1)*(1.0f/32768.0f);
	rp1=READRIGHTSIGNED16(m_Location.m_pStart,n1)*(1.0f/32768.0f);
	n0=n1-1;
	if( n0<0 )
		n0=0;
	lp0=READLEFTSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
	rp0=READRIGHTSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
	n2=m_iPosition+1;
	if( n2>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			lp2=READLEFTSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			rp2=READRIGHTSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
		{
			lp2=0;
			rp2=0;
		}
	}
	else
	{
		lp2=READLEFTSIGNED16(m_Location.m_pStart,n2)*(1.0f/32768.0f);
		rp2=READRIGHTSIGNED16(m_Location.m_pStart,n2)*(1.0f/32768.0f);
	}
	n3=n2+1;
	if( n3>=iLast )
	{
		if( m_Loop.m_pStart )
		{
			lp3=READLEFTSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			rp3=READRIGHTSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
			iLoopPos+=1;
		}
		else
		{
			lp3=0;
			rp3=0;
		}
	}
	else
	{
		lp3=READLEFTSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
		rp3=READRIGHTSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
	}

	if( m_iFreq>0 )
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	lq0=-lp0+fscale(lp1,1)+lp1-fscale(lp2,1)-lp2+lp3;
			float	lq1=fscale(lp0,1)-fscale(lp1,2)-lp1+fscale(lp2,2)-lp3;
			float	lq2=-lp0+lp2;
			float	rq0=-rp0+fscale(rp1,1)+rp1-fscale(rp2,1)-rp2+rp3;
			float	rq1=fscale(rp0,1)-fscale(rp1,2)-rp1+fscale(rp2,2)-rp3;
			float	rq2=-rp0+rp2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				*pDest++ = ((fTime*lq0+lq1)*fTime+lq2)*fTime*0.5f+lp1;
				*pDest++ = ((fTime*rq0+rq1)*fTime+rq2)*fTime*0.5f+rp1;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt-- )
			{
				lp0=lp1;
				lp1=lp2;
				lp2=lp3;
				rp0=rp1;
				rp1=rp2;
				rp2=rp3;
				n3+=1;
				if( n3>=iLast )
				{
					if( m_Loop.m_pStart )
					{
						lp3=READLEFTSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
						rp3=READRIGHTSIGNED16(m_Loop.m_pStart,iLoopPos)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
					{
						lp3=0;
						rp3=0;
					}
				}
				else
				{
					lp3=READLEFTSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
					rp3=READRIGHTSIGNED16(m_Location.m_pStart,n3)*(1.0f/32768.0f);
				}
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}
	else
	{
		while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
		{
			float	lq0=-lp0+fscale(lp1,1)+lp1-fscale(lp2,1)-lp2+lp3;
			float	lq1=fscale(lp0,1)-fscale(lp1,2)-lp1+fscale(lp2,2)-lp3;
			float	lq2=-lp0+lp2;
			float	rq0=-rp0+fscale(rp1,1)+rp1-fscale(rp2,1)-rp2+rp3;
			float	rq1=fscale(rp0,1)-fscale(rp1,2)-rp1+fscale(rp2,2)-rp3;
			float	rq2=-rp0+rp2;

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
				*pDest++ = ((fTime*lq0+lq1)*fTime+lq2)*fTime*0.5f+lp1;
				*pDest++ = ((fTime*rq0+rq1)*fTime+rq2)*fTime*0.5f+rp1;
				m_iFraction+=m_iFreq;
			}

			int	cnt=m_iFraction>>SHIFTFRACTION;

			while( cnt++ )
			{
				lp3=lp2;
				lp2=lp1;
				lp1=lp0;
				rp3=rp2;
				rp2=rp1;
				rp1=rp0;
				n0-=1;
				if( n0<0 )
				{
					if( m_Loop.m_pStart )
					{
						lp0=READLEFTSIGNED16(m_Loop.m_pEnd,n0)*(1.0f/32768.0f);
						rp0=READRIGHTSIGNED16(m_Loop.m_pEnd,n0)*(1.0f/32768.0f);
						iLoopPos+=1;
					}
					else
					{
						lp0=0;
						rp0=0;
					}
				}
				else
				{
					lp0=READLEFTSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
					rp0=READRIGHTSIGNED16(m_Location.m_pStart,n0)*(1.0f/32768.0f);
				}
			}
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}
	}

	return pDest;
}

#ifndef	BUZZ
float	*	CResampler::ResampleSigned8ToStereoFloatBuffer_Spline( float *pDest, int iCount )
{
	return ResampleSigned8ToStereoFloatBuffer_Filter( pDest, iCount );
}
#endif

void	CResampler::ResampleToFloatBuffer_Raw( float *pDest, int iCount )
{
	if( m_Location.m_eFiltering==FILTER_SPLINE )
	{
#ifndef	BUZZ
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleFloatToFloatBuffer_Spline( pDest, iCount );
		else
#endif
		if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleSigned16ToFloatBuffer_Spline( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleStereoSigned16ToFloatBuffer_Spline( pDest, iCount );
#ifndef	BUZZ
		else
			pDest=ResampleSigned8ToFloatBuffer_Spline( pDest, iCount );
#endif
	}
	else if( m_Location.m_eFiltering==FILTER_LINEAR )
	{
#ifndef	BUZZ
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleFloatToFloatBuffer_Filter( pDest, iCount );
		else
#endif
		if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleSigned16ToFloatBuffer_Filter( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleStereoSigned16ToFloatBuffer_Filter( pDest, iCount );
#ifndef	BUZZ
		else
			pDest=ResampleSigned8ToFloatBuffer_Filter( pDest, iCount );
#endif
	}
	else
	{
#ifndef	BUZZ
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleFloatToFloatBuffer_Normal( pDest, iCount );
		else
#endif
		if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleSigned16ToFloatBuffer_Normal( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleStereoSigned16ToFloatBuffer_Normal( pDest, iCount );
#ifndef	BUZZ
		else
			pDest=ResampleSigned8ToFloatBuffer_Normal( pDest, iCount );
#endif
	}
	m_fMaybeLastLeftSample=pDest[-1];
}

void	CResampler::ResampleToStereoFloatBuffer_Raw( float *pDest, int iCount )
{
	if( m_Location.m_eFiltering==FILTER_SPLINE )
	{
#ifndef	BUZZ
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleFloatToStereoFloatBuffer_Spline( pDest, iCount );
		else
#endif
		if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleSigned16ToStereoFloatBuffer_Spline( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleStereoSigned16ToStereoFloatBuffer_Spline( pDest, iCount );
#ifndef	BUZZ
		else
			pDest=ResampleSigned8ToStereoFloatBuffer_Spline( pDest, iCount );
#endif
	}
	else if( m_Location.m_eFiltering==FILTER_LINEAR )
	{
#ifndef	BUZZ
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleFloatToStereoFloatBuffer_Filter( pDest, iCount );
		else
#endif
		if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleSigned16ToStereoFloatBuffer_Filter( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleStereoSigned16ToStereoFloatBuffer_Filter( pDest, iCount );
#ifndef	BUZZ
		else
			pDest=ResampleSigned8ToStereoFloatBuffer_Filter( pDest, iCount );
#endif
	}
	else
	{
#ifndef	BUZZ
		if( m_Location.m_eFormat==SMP_FLOAT )
			pDest=ResampleFloatToStereoFloatBuffer_Normal( pDest, iCount );
		else
#endif
		if( m_Location.m_eFormat==SMP_SIGNED16 )
			pDest=ResampleSigned16ToStereoFloatBuffer_Normal( pDest, iCount );
		else if( m_Location.m_eFormat==SMP_SIGNED16_STEREO )
			pDest=ResampleStereoSigned16ToStereoFloatBuffer_Normal( pDest, iCount );
#ifndef	BUZZ
		else
			pDest=ResampleSigned8ToStereoFloatBuffer_Normal( pDest, iCount );
#endif
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
