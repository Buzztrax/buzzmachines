#include	"SRF_Amp.h"

#define	ALMOSTZERO	(1.0f/65536.0f)
#define	FIXVOLUME(x)	if((x)<=ALMOSTZERO)(x)=0.0f

using namespace SurfDSPLib;

bool	CAmp::Active()
{
	return m_fLeftDestVolume>ALMOSTZERO || m_fRightDestVolume>ALMOSTZERO || m_fLeftVolume>ALMOSTZERO || m_fRightVolume>ALMOSTZERO || m_fLastLeftSampleRamp || m_fLastRightSampleRamp;
}

void	CAmp::AmpAndAdd( float *pLeft, float *pRight, float *pSrc, int iCount, float fAmp )
{
	float	*pLeftStart=pLeft;
	float	*pRightStart=pRight;
	int		iTotal=iCount;

	if( pRight )
	{
		float	lastl;
		float	lastr;

		if( m_fLeftVolumeRamp!=0.0f || m_fRightVolumeRamp!=0.0f )
		{
			bool	reset=false;
			int		n=0;
			int		n2=0;
			
			if( m_fLeftVolumeRamp!=0.0f ) 
				n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);
			if( m_fRightVolumeRamp!=0.0f )
				n2=int((m_fRightDestVolume-m_fRightVolume)/m_fRightVolumeRamp);

			if( n2>n )
				n=n2;

			if( n>iCount )
				n=iCount;
			else
				reset=true;

			iCount-=n;

			while( n-- )
			{
				float	f=*pSrc++;
				
				*pLeft+=(lastl=f*m_fLeftVolume*fAmp);
				*pRight+=(lastr=f*m_fRightVolume*fAmp);
				pLeft+=1;
				pRight+=1;
				m_fLeftVolume+=m_fLeftVolumeRamp;
				m_fRightVolume+=m_fRightVolumeRamp;
			}

			if( reset )
			{
				m_fLeftVolumeRamp=0.0f;
				m_fLeftVolume=m_fLeftDestVolume;
				m_fRightVolumeRamp=0.0f;
				m_fRightVolume=m_fRightDestVolume;
			}
		}

		if( iCount>0 )
		{
			float	fLeft=m_fLeftVolume*fAmp;
			float	fRight=m_fRightVolume*fAmp;
			while( iCount-- )
			{
				float	f=*pSrc++;
				
				*pLeft+=(lastl=f*fLeft);
				*pRight+=(lastr=f*fRight);
				pLeft+=1;
				pRight+=1;
			}
		}
		m_fMaybeLastLeftSample = lastl;
		m_fMaybeLastRightSample = lastr;
	}
	else
	{
		float	lastf;

		if( m_fLeftVolumeRamp!=0.0f )
		{
			bool	reset=false;
			int		n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);

			if( n>iCount )
				n=iCount;
			else
				reset=true;

			iCount-=n;

			while( n-- )
			{
				float	f=*pSrc++;
				
				*pLeft+=(lastf=f*m_fLeftVolume*fAmp);
				pLeft+=1;
				m_fLeftVolume+=m_fLeftVolumeRamp;
			}

			if( reset )
			{
				m_fLeftVolumeRamp=0.0f;
				m_fLeftVolume=m_fLeftDestVolume;
			}
		}

		if( iCount>0 )
		{
			float	fLeft=m_fLeftVolume*fAmp;

			while( iCount-- )
			{
				float	f=*pSrc++;
				
				*pLeft+=(lastf=f*fLeft);
				pLeft+=1;
			}
		}
		m_fMaybeLastLeftSample = lastf;
	}

	AddFadeOut( pLeftStart, pRightStart, iTotal );
}

void	CAmp::AmpAndAdd_ToStereo( float *pStereo, float *pSrc, int iCount, float fAmp )
{
	float	*pStart=pStereo;
	int		iTotal=iCount;

	float	lastl;
	float	lastr;

	if( m_fLeftVolumeRamp!=0.0f || m_fRightVolumeRamp!=0.0f )
	{
		bool	reset=false;
		int		n=0;
		int		n2=0;
		
		if( m_fLeftVolumeRamp!=0.0f ) 
			n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);
		if( m_fRightVolumeRamp!=0.0f )
			n2=int((m_fRightDestVolume-m_fRightVolume)/m_fRightVolumeRamp);

		if( n2>n )
			n=n2;

		if( n>iCount )
			n=iCount;
		else
			reset=true;

		iCount-=n;

		while( n-- )
		{
			float	f=*pSrc++;
			
			pStereo[0]+=(lastl=f*m_fLeftVolume*fAmp);
			pStereo[1]+=(lastr=f*m_fRightVolume*fAmp);
			pStereo+=2;
			m_fLeftVolume+=m_fLeftVolumeRamp;
			m_fRightVolume+=m_fRightVolumeRamp;
		}

		if( reset )
		{
			m_fLeftVolumeRamp=0.0f;
			m_fLeftVolume=m_fLeftDestVolume;
			m_fRightVolumeRamp=0.0f;
			m_fRightVolume=m_fRightDestVolume;
		}
	}

	if( iCount>0 )
	{
		float	fLeft=m_fLeftVolume*fAmp;
		float	fRight=m_fRightVolume*fAmp;
		while( iCount-- )
		{
			float	f=*pSrc++;
			
			pStereo[0]+=(lastl=f*fLeft);
			pStereo[1]+=(lastr=f*fRight);
			pStereo+=2;
		}
	}
	m_fMaybeLastLeftSample = lastl;
	m_fMaybeLastRightSample = lastr;

	AddFadeOut_Stereo( pStart, iTotal );
}

void	CAmp::AmpAndAdd_StereoToStereo( float *pStereo, float *pSrc, int iCount, float fAmp )
{
	float	*pStart=pStereo;
	int		iTotal=iCount;

	float	lastl;
	float	lastr;

	if( m_fLeftVolumeRamp!=0.0f || m_fRightVolumeRamp!=0.0f )
	{
		bool	reset=false;
		int		n=0;
		int		n2=0;
		
		if( m_fLeftVolumeRamp!=0.0f ) 
			n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);
		if( m_fRightVolumeRamp!=0.0f )
			n2=int((m_fRightDestVolume-m_fRightVolume)/m_fRightVolumeRamp);

		if( n2>n )
			n=n2;

		if( n>iCount )
			n=iCount;
		else
			reset=true;

		iCount-=n;

		while( n-- )
		{
			float	lf=*pSrc++;
			float	rf=*pSrc++;
			
			pStereo[0]+=(lastl=lf*m_fLeftVolume*fAmp);
			pStereo[1]+=(lastr=rf*m_fRightVolume*fAmp);
			pStereo+=2;
			m_fLeftVolume+=m_fLeftVolumeRamp;
			m_fRightVolume+=m_fRightVolumeRamp;
		}

		if( reset )
		{
			m_fLeftVolumeRamp=0.0f;
			m_fLeftVolume=m_fLeftDestVolume;
			m_fRightVolumeRamp=0.0f;
			m_fRightVolume=m_fRightDestVolume;
		}
	}

	if( iCount>0 )
	{
		float	fLeft=m_fLeftVolume*fAmp;
		float	fRight=m_fRightVolume*fAmp;
		while( iCount-- )
		{
			float	lf=*pSrc++;
			float	rf=*pSrc++;
			
			pStereo[0]+=(lastl=lf*fLeft);
			pStereo[1]+=(lastr=rf*fRight);
			pStereo+=2;
		}
	}
	m_fMaybeLastLeftSample = lastl;
	m_fMaybeLastRightSample = lastr;

	AddFadeOut_Stereo( pStart, iTotal );
}

void	CAmp::AmpAndMove( float *pLeft, float *pRight, float *pSrc, int iCount, float fAmp )
{
	float	*pLeftStart=pLeft;
	float	*pRightStart=pRight;
	int		iTotal=iCount;

	if( pRight )
	{
		if( m_fLeftVolumeRamp!=0.0f || m_fRightVolumeRamp!=0.0f )
		{
			bool	reset=false;
			int		n=0;
			int		n2=0;
			
			if( m_fLeftVolumeRamp!=0.0f ) 
				n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);
			if( m_fRightVolumeRamp!=0.0f )
				n2=int((m_fRightDestVolume-m_fRightVolume)/m_fRightVolumeRamp);

			if( n2>n )
				n=n2;

			if( n>iCount )
				n=iCount;
			else
				reset=true;

			iCount-=n;

			while( n-- )
			{
				float	f=*pSrc++;
				
				*pLeft=f*m_fLeftVolume*fAmp;
				*pRight=f*m_fRightVolume*fAmp;
				pLeft+=1;
				pRight+=1;
				m_fLeftVolume+=m_fLeftVolumeRamp;
				m_fRightVolume+=m_fRightVolumeRamp;
			}

			if( reset )
			{
				m_fLeftVolumeRamp=0.0f;
				m_fLeftVolume=m_fLeftDestVolume;
				m_fRightVolumeRamp=0.0f;
				m_fRightVolume=m_fRightDestVolume;
			}
		}

		if( iCount>0 )
		{
			float	fLeft=fAmp*m_fLeftVolume;
			float	fRight=fAmp*m_fRightVolume;

			while( iCount-- )
			{
				float	f=*pSrc++;
				
				*pLeft=f*fLeft;
				*pRight=f*fRight;
				pLeft+=1;
				pRight+=1;
			}
		}
		m_fMaybeLastLeftSample = *--pLeft;
		m_fMaybeLastRightSample = *--pRight;
	}
	else
	{
		if( m_fLeftVolumeRamp!=0.0f )
		{
			bool	reset=false;
			int		n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);

			if( n>iCount )
				n=iCount;
			else
				reset=true;

			iCount-=n;

			while( n-- )
			{
				float	f=*pSrc++;
				
				*pLeft=f*m_fLeftVolume*fAmp;
				pLeft+=1;
				m_fLeftVolume+=m_fLeftVolumeRamp;
			}

			if( reset )
			{
				m_fLeftVolumeRamp=0.0f;
				m_fLeftVolume=m_fLeftDestVolume;
			}
		}

		if( iCount>0 )
		{
			float	fLeft=fAmp*m_fLeftVolume;
			while( iCount-- )
			{
				float	f=*pSrc++;
				
				*pLeft=f*fLeft;
				pLeft+=1;
			}
		}

		m_fMaybeLastLeftSample = *--pLeft;
	}

	AddFadeOut( pLeftStart, pRightStart, iTotal );
}

void	CAmp::AmpAndMove_ToStereo( float *pStereo, float *pSrc, int iCount, float fAmp )
{
	float	*pStart=pStereo;
	int		iTotal=iCount;

	if( m_fLeftVolumeRamp!=0.0f || m_fRightVolumeRamp!=0.0f )
	{
		bool	reset=false;
		int		n=0;
		int		n2=0;
		
		if( m_fLeftVolumeRamp!=0.0f ) 
			n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);
		if( m_fRightVolumeRamp!=0.0f )
			n2=int((m_fRightDestVolume-m_fRightVolume)/m_fRightVolumeRamp);

		if( n2>n )
			n=n2;


		if( n>iCount )
			n=iCount;
		else
			reset=true;

		iCount-=n;

		while( n-- )
		{
			float	f=*pSrc++;
			
			*pStereo++ = f*m_fLeftVolume*fAmp;
			*pStereo++ = f*m_fRightVolume*fAmp;
			m_fLeftVolume+=m_fLeftVolumeRamp;
			m_fRightVolume+=m_fRightVolumeRamp;
		}

		if( reset )
		{
			m_fLeftVolumeRamp=0.0f;
			m_fLeftVolume=m_fLeftDestVolume;
			m_fRightVolumeRamp=0.0f;
			m_fRightVolume=m_fRightDestVolume;
		}
	}

	if( iCount>0 )
	{
		float	fLeft=fAmp*m_fLeftVolume;
		float	fRight=fAmp*m_fRightVolume;

		while( iCount-- )
		{
			float	f=*pSrc++;
			
			*pStereo++ = f*fLeft;
			*pStereo++ = f*fRight;
		}
	}
	m_fMaybeLastRightSample = *--pStereo;
	m_fMaybeLastLeftSample = *--pStereo;

	AddFadeOut_Stereo( pStart, iTotal );
}

void	CAmp::AmpAndMove_StereoToStereo( float *pStereo, float *pSrc, int iCount, float fAmp )
{
	float	*pStart=pStereo;
	int		iTotal=iCount;

	if( m_fLeftVolumeRamp!=0.0f || m_fRightVolumeRamp!=0.0f )
	{
		bool	reset=false;
		int		n=0;
		int		n2=0;
		
		if( m_fLeftVolumeRamp!=0.0f ) 
			n=int((m_fLeftDestVolume-m_fLeftVolume)/m_fLeftVolumeRamp);
		if( m_fRightVolumeRamp!=0.0f )
			n2=int((m_fRightDestVolume-m_fRightVolume)/m_fRightVolumeRamp);

		if( n2>n )
			n=n2;

		if( n>iCount )
			n=iCount;
		else
			reset=true;

		iCount-=n;

		while( n-- )
		{
			float	lf=*pSrc++;
			float	rf=*pSrc++;
			
			*pStereo++ = lf*m_fLeftVolume*fAmp;
			*pStereo++ = rf*m_fRightVolume*fAmp;
			m_fLeftVolume+=m_fLeftVolumeRamp;
			m_fRightVolume+=m_fRightVolumeRamp;
		}

		if( reset )
		{
			m_fLeftVolumeRamp=0.0f;
			m_fLeftVolume=m_fLeftDestVolume;
			m_fRightVolumeRamp=0.0f;
			m_fRightVolume=m_fRightDestVolume;
		}
	}

	if( iCount>0 )
	{
		float	fLeft=fAmp*m_fLeftVolume;
		float	fRight=fAmp*m_fRightVolume;

		while( iCount-- )
		{
			float	lf=*pSrc++;
			float	rf=*pSrc++;
			
			*pStereo++ = lf*fLeft;
			*pStereo++ = rf*fRight;
		}
	}
	m_fMaybeLastRightSample = *--pStereo;
	m_fMaybeLastLeftSample = *--pStereo;

	AddFadeOut_Stereo( pStart, iTotal );
}





void	CAmp::Reset()
{
	m_fLeftVolume=0.0f;
	m_fLeftDestVolume=0.0f;
	m_fLeftVolumeRamp=0.0f;
	m_fRightVolume=0.0f;
	m_fRightDestVolume=0.0f;
	m_fRightVolumeRamp=0.0f;

	m_fMaybeLastLeftSample=0.0f;
	m_fMaybeLastRightSample=0.0f;
	m_fLastLeftSampleRamp=0.0f;
	m_fLastRightSampleRamp=0.0f;

	m_iRampTime=0;
}

void	CAmp::Retrig()
{
	m_fLeftVolume=0.0f;
	m_fRightVolume=0.0f;

	m_fLastLeftSample=m_fMaybeLastLeftSample;
	m_fLastRightSample=m_fMaybeLastRightSample;

	if( m_iRampTime )
	{
		m_fLeftVolumeRamp=(m_fLeftDestVolume-m_fLeftVolume)/m_iRampTime;
		m_fRightVolumeRamp=(m_fRightDestVolume-m_fRightVolume)/m_iRampTime;
		m_fLastLeftSampleRamp=-m_fLastLeftSample/m_iRampTime;
		m_fLastRightSampleRamp=-m_fLastRightSample/m_iRampTime;
	}
	else
	{
		m_fLeftVolumeRamp=0;
		m_fRightVolumeRamp=0;
		m_fLastLeftSampleRamp=0;
		m_fLastRightSampleRamp=0;
	}
	m_fMaybeLastLeftSample=0;
	m_fMaybeLastRightSample=0;
}

void	CAmp::SetVolume( float fLeft, float fRight )
{
	FIXVOLUME(fLeft);	
	FIXVOLUME(fRight);	
	FIXVOLUME(m_fLeftDestVolume);
	FIXVOLUME(m_fRightDestVolume);
	
	m_fLeftDestVolume=fLeft;
	m_fRightDestVolume=fRight;
	if( m_iRampTime )
	{
		m_fLeftVolumeRamp=(m_fLeftDestVolume-m_fLeftVolume)/m_iRampTime;
		m_fRightVolumeRamp=(m_fRightDestVolume-m_fRightVolume)/m_iRampTime;
	}
	else
	{
		m_fLeftVolume=m_fLeftDestVolume;
		m_fRightVolume=m_fRightDestVolume;
		m_fLeftVolumeRamp=0;
		m_fRightVolumeRamp=0;
	}
}

void	CAmp::AddFadeOut( float *pLeft, float *pRight, int iTotal )
{
	if( m_fLastLeftSampleRamp==0.0f )
		return;

	if( pRight )
	{
		int		n=int(-m_fLastLeftSample/m_fLastLeftSampleRamp);
		bool	reset=true;

		if( n>iTotal )
		{
			n=iTotal;
			reset=false;
		}

		while( n-- )
		{
			*pLeft += m_fLastLeftSample;
			*pRight += m_fLastRightSample;
			m_fLastLeftSample+=m_fLastLeftSampleRamp;
			m_fLastRightSample+=m_fLastRightSampleRamp;
			pLeft+=1;
			pRight+=1;
		}

		if( reset )
		{
			m_fLastLeftSample=0.0f;
			m_fLastRightSample=0.0f;
			m_fLastLeftSampleRamp=0.0f;
			m_fLastRightSampleRamp=0.0f;
		}
	}
	else
	{
		int		n=int(-m_fLastLeftSample/m_fLastLeftSampleRamp);
		bool	reset=true;

		if( n>iTotal )
		{
			n=iTotal;
			reset=false;
		}

		while( n-- )
		{
			*pLeft += m_fLastLeftSample;
			m_fLastLeftSample+=m_fLastLeftSampleRamp;
			pLeft+=1;
		}

		if( reset )
		{
			m_fLastLeftSample=0.0f;
			m_fLastLeftSampleRamp=0.0f;
		}
	}
}

void	CAmp::AddFadeOut_Stereo( float *pStart, int iTotal )
{
	if( m_fLastLeftSampleRamp==0.0f && m_fLastRightSampleRamp==0.0f )
		return;

	int		n=int(-m_fLastLeftSample/m_fLastLeftSampleRamp);
	int		n2=int(-m_fLastRightSample/m_fLastRightSampleRamp);
	bool	reset=true;

	if( n2>n )
		n=n2;

	if( n>iTotal )
	{
		n=iTotal;
		reset=false;
	}

	while( n-- )
	{
		pStart[0] += m_fLastLeftSample;
		pStart[1] += m_fLastRightSample;
		m_fLastLeftSample+=m_fLastLeftSampleRamp;
		m_fLastRightSample+=m_fLastRightSampleRamp;
		pStart+=2;
	}

	if( reset )
	{
		m_fLastLeftSample=0.0f;
		m_fLastRightSample=0.0f;
		m_fLastLeftSampleRamp=0.0f;
		m_fLastRightSampleRamp=0.0f;
	}
}
