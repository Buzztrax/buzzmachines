#ifndef	CRESAMPLER_H__
#define	CRESAMPLER_H__

namespace SurfDSPLib
{

typedef	void (*DoneCallback)( u_long );

enum	ESampleFormat
{
	SMP_SIGNED8=0,
	SMP_SIGNED8SWAPPED=1,
	SMP_FLOAT=2,
	SMP_SIGNED16=3,
	SMP_SIGNED8_STEREO=4,
	SMP_SIGNED8SWAPPED_STEREO=5,
	SMP_FLOAT_STEREO=6,
	SMP_SIGNED16_STEREO=7,
};

enum	EFiltering
{
	FILTER_NEAREST,
	FILTER_LINEAR,
	FILTER_SPLINE,
};

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

#ifndef	BUZZ
	float			*	ResampleFloatToFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleFloatToFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleFloatToFloatBuffer_Spline( float *pDest, int iCount );
	float			*	ResampleFloatToStereoFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleFloatToStereoFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleFloatToStereoFloatBuffer_Spline( float *pDest, int iCount );
#endif

	float			*	ResampleSigned16ToFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleStereoSigned16ToFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleSigned16ToStereoFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleStereoSigned16ToStereoFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleSigned16ToFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleSigned16ToStereoFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleStereoSigned16ToFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleStereoSigned16ToStereoFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleSigned16ToFloatBuffer_Spline( float *pDest, int iCount );
	float			*	ResampleSigned16ToStereoFloatBuffer_Spline( float *pDest, int iCount );
	float			*	ResampleStereoSigned16ToFloatBuffer_Spline( float *pDest, int iCount );
	float			*	ResampleStereoSigned16ToStereoFloatBuffer_Spline( float *pDest, int iCount );

#ifndef	BUZZ
	float			*	ResampleSigned8ToFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleSigned8ToFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleSigned8ToFloatBuffer_Spline( float *pDest, int iCount );
	float			*	ResampleSigned8ToStereoFloatBuffer_Normal( float *pDest, int iCount );
	float			*	ResampleSigned8ToStereoFloatBuffer_Filter( float *pDest, int iCount );
	float			*	ResampleSigned8ToStereoFloatBuffer_Spline( float *pDest, int iCount );
#endif
};

};

#endif
