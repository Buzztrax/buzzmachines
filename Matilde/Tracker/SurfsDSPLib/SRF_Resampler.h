#ifndef	CRESAMPLER_H__
#define	CRESAMPLER_H__

namespace SurfDSPLib
{

typedef	void (*DoneCallback)( u_long );

enum	ESampleFormat
{
	SMP_FLAG_STEREO = 8,

	SMP_SIGNED8=0,
	SMP_SIGNED8SWAPPED=1,
	SMP_FLOAT=2,
	SMP_SIGNED16=3,
	SMP_SIGNED24=4,
	SMP_SIGNED32=5,

	SMP_SIGNED8_STEREO=SMP_SIGNED8|SMP_FLAG_STEREO,
	SMP_SIGNED8SWAPPED_STEREO=SMP_SIGNED8SWAPPED|SMP_FLAG_STEREO,
	SMP_FLOAT_STEREO=SMP_FLOAT|SMP_FLAG_STEREO,
	SMP_SIGNED16_STEREO=SMP_SIGNED16|SMP_FLAG_STEREO,
	SMP_SIGNED24_STEREO=SMP_SIGNED24|SMP_FLAG_STEREO,
	SMP_SIGNED32_STEREO=SMP_SIGNED32|SMP_FLAG_STEREO,

};

enum	EFiltering
{
	FILTER_NEAREST,
	FILTER_LINEAR,
	FILTER_SPLINE,
};


#pragma pack(push, 1)
struct S24 {
	union {
		struct {
			char c3[3];
		};
		struct {
			short s;
			char c;
		};
	};
};

struct stereofloat {
	float l, r;

	stereofloat operator-(stereofloat& rhs) {
		stereofloat f(*this);
		f.l -= rhs.l;
		f.r -= rhs.r;
		return f;
	}
	stereofloat operator+(stereofloat& rhs) {
		stereofloat f(*this);
		f.l += rhs.l;
		f.r += rhs.r;
		return f;
	}
	stereofloat operator*(float rhs) {
		stereofloat f(*this);
		f.l *= rhs;
		f.r *= rhs;
		return f;
	}
	stereofloat operator=(float i) {
		l = i;
		r = i;
		return *this;
	}
	stereofloat operator=(stereofloat i) {
		l = i.l;
		r = i.r;
		return *this;
	}
	stereofloat operator-() {
		stereofloat f(*this);
		f.l = -f.l;
		f.r = -f.r;
		return f;
	}
	operator float() {
		return (l+r) / 2;
	}
};
#pragma pack(pop)

inline stereofloat operator * (float f, stereofloat& sf) {
	return sf * f;
}

class	CResampler
{
public:
	CResampler() { Reset(); }

	class CLocation
	{
	public:
		void				AdvanceLocation( int i );
		void				AdvanceEnd( int i );
		long				GetLength();

		void			*	m_pStart;
		void			*	m_pEnd;
		ESampleFormat		m_eFormat;
		EFiltering			m_eFiltering;
	};

	CLocation			m_Location,
						m_Loop;

	long				m_iFreq;

	bool				m_oPingPongLoop;
	bool				m_oForward;

	signed long			m_iPosition;
	signed long			m_iFraction;

	DoneCallback		m_pDoneCallback;
	u_long				m_iCBData;

	int					m_iDelaySamples;
	int					m_iRampTime;

	void				Reset();

	void				ResampleToStereoFloatBuffer( float *pDest, int iCount );
	void				ResampleToFloatBuffer( float *pDest, int iCount );
	void				Skip( int iCount );

	void				SetFrequency( float f );

	bool				Active();
	void				Stop();

private:
	float				m_fLastLeftSample;
	float				m_fLastLeftSampleRamp;
	float				m_fMaybeLastLeftSample;
	float				m_fLastRightSample;
	float				m_fLastRightSampleRamp;
	float				m_fMaybeLastRightSample;

	llong				MultiplyFreq( int i );
	llong				GetSamplesToEnd();

	void				ResampleToFloatBuffer_Raw( float *pDest, int iCount );
	void				ResampleToStereoFloatBuffer_Raw( float *pDest, int iCount );
	void				Skip_Raw( int iCount );
	void				AddFadeOut( float *pDest, int iCount );
	void				AddFadeOutStereo( float *pDest, int iCount );

	inline void ConvertSample(const char &src, float &dst) { dst = (float)src / 127.0f; }
	inline void ConvertSample(const short &src, float &dst) { dst = (float)src / 32767.0f; }
	inline void ConvertSample(const S24 &src, float &dst) { 
		int i = (((int)src.c3[2]) << 16) | (((unsigned int)(unsigned char)src.c3[1]) << 8) | ((unsigned int)(unsigned char)src.c3[0]);
		dst = (float)i / 8388607.0f;
	}
	inline void ConvertSample(const int &src, float &dst) { dst = (float)src / 2147483647.0f; }
	inline void ConvertSample(const float &src, float &dst) { dst = src; }

	template<typename T>
	void ReadT(float& f, void* ptr, int offset) {
		T* samples = ((T*)ptr) + offset;
		ConvertSample(*samples, f);
	}

	template<typename T>
	void ReadT(stereofloat& f, void* ptr, int offset) {
		T* samples = ((T*)ptr) + offset * 2;
		ConvertSample(*samples, f.l);
		samples ++;
		ConvertSample(*samples, f.r);
	}

	// *** Resample normal ***

	template<typename T, typename DT, typename ST>
	DT* ResampleTToFloatBuffer_Normal(DT* pDest, int iCount) {
		while( iCount-- )
		{
			ST	d;
			ReadT<T>(d, m_Location.m_pStart, m_iPosition);
			*pDest = d;
			pDest ++;
			m_iFraction+=m_iFreq;
			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}

		return pDest;
	}

	// *** Resample filter ***

	template<typename T, typename DT, typename ST>
	DT* ResampleTToFloatBuffer_Filter(DT* pDest, int iCount) {

		long	iLast=m_Location.GetLength()-1;

		while( (iCount>0) && (m_iPosition<iLast) && (m_iPosition>=0) )
		{
			ST	s;
			ST	d;
			ReadT<T>(s, m_Location.m_pStart, m_iPosition);
			ReadT<T>(d, m_Location.m_pStart, m_iPosition+1);

			d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				ST	t=d*(float)m_iFraction+s;
				*pDest = t;
				pDest ++;
				m_iFraction+=m_iFreq;
			}

			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}

		if( m_iPosition<0 )
			iCount=iCount;

		if( iCount>0 )
		{
			ST	s;
			ST	d;
			ReadT<T>(s, m_Location.m_pStart,m_iPosition);

			if( m_Loop.m_pStart )
				ReadT<T>(d, m_Loop.m_pStart,0);
			else
				d=0;

			d=(d-s)*(1.0f/(1<<SHIFTFRACTION));

			while( (m_iFraction<=MAXFRACTION) && iCount-- )
			{
				ST	t=d*(float)m_iFraction+s;
				*pDest = t;
				pDest ++;
				m_iFraction+=m_iFreq;
			}

			m_iPosition+=m_iFraction>>SHIFTFRACTION;
			m_iFraction&=MAXFRACTION;
		}

		return pDest;
	}

	// *** Resample spline ***

	template<typename T, typename DT, typename ST>
	DT* ResampleTToFloatBuffer_Spline(DT* pDest, int iCount) {
		long	iLast=m_Location.GetLength()-1;
		long	iLoopPos=0;

		ST	p0,p1,p2,p3;
		int		n0,n1,n2,n3;

		n1=m_iPosition;
		ReadT<T>(p1, m_Location.m_pStart,n1);
		n0=n1-1;
		if( n0<0 )
			n0=0;
		ReadT<T>(p0, m_Location.m_pStart,n0);
		n2=m_iPosition+1;
		if( n2>=iLast )
		{
			if( m_Loop.m_pStart )
			{
				ReadT<T>(p2, m_Loop.m_pStart,iLoopPos);
				iLoopPos+=1;
			}
			else
				p2=0;
		}
		else
			ReadT<T>(p2, m_Location.m_pStart,n2);
		n3=n2+1;
		if( n3>=iLast )
		{
			if( m_Loop.m_pStart )
			{
				ReadT<T>(p3, m_Loop.m_pStart,iLoopPos);
				iLoopPos+=1;
			}
			else
				p3=0;
		}
		else
			ReadT<T>(p3, m_Location.m_pStart,n3);

		if( m_iFreq>0 )
		{
			while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
			{
				ST fp0_1 = fscale(p0,1);
				ST fp1_1 = fscale(p1,1);
				ST fp2_1 = fscale(p2,1);
				ST fp1_2 = fscale(p1,2);
				ST fp2_2 = fscale(p2,2);

				ST	q0=-p0+fp1_1+p1-fp2_1-p2+p3;
				ST	q1=fp0_1-fp1_2-p1+fp2_2-p3;
				ST	q2=-p0+p2;

				while( (m_iFraction<=MAXFRACTION) && iCount-- )
				{
					float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
					ST	d=((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
					*pDest = d;
					pDest ++;
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
							ReadT<T>(p3, m_Loop.m_pStart,iLoopPos);
							iLoopPos+=1;
						}
						else
							p3=0;
					}
					else
						ReadT<T>(p3, m_Location.m_pStart,n3);
				}
				m_iPosition+=m_iFraction>>SHIFTFRACTION;
				m_iFraction&=MAXFRACTION;
			}
		}
		else
		{
			while( (iCount>0) && (m_iPosition<=iLast) && (m_iPosition>=0) )
			{
				ST fp0_1 = fscale(p0,1);
				ST fp1_1 = fscale(p1,1);
				ST fp2_1 = fscale(p2,1);
				ST fp1_2 = fscale(p1,2);
				ST fp2_2 = fscale(p2,2);
				ST	q0=-p0+fp1_1+p1-fp2_1-p2+p3;
				ST	q1=fp0_1-fp1_2-p1+fp2_2-p3;
				ST	q2=-p0+p2;

				while( (m_iFraction<=MAXFRACTION) && iCount-- )
				{
					float	fTime=m_iFraction*(1.0f/(MAXFRACTION+1));
					ST	d=((fTime*q0+q1)*fTime+q2)*fTime*0.5f+p1;
					*pDest = d;
					pDest ++;
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
							ReadT<T>(p0, m_Loop.m_pEnd,n0);
							iLoopPos+=1;
						}
						else
							p0=0;
					}
					else
						ReadT<T>(p0, m_Location.m_pStart,n0);
				}
				m_iPosition+=m_iFraction>>SHIFTFRACTION;
				m_iFraction&=MAXFRACTION;
			}
		}

		return pDest;
	}

};

};

#endif
