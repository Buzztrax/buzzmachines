#ifndef	SRF_AMP_H__
#define	SRF_AMP_H__

#include	"SRF_Types.h"

namespace SurfDSPLib
{

class	CAmp
{
public:
				CAmp() { Reset(); }

	int			m_iRampTime;

	void		Reset();

	void		AmpAndAdd( float *pLeft, float *pRight, float *pSrc, int iCount, float fAmp );
	void		AmpAndAdd_ToStereo( float *pStereo, float *pSrc, int iCount, float fAmp );
	void		AmpAndAdd_StereoToStereo( float *pStereo, float *pSrc, int iCount, float fAmp );
	void		AmpAndMove( float *pLeft, float *pRight, float *pSrc, int iCount, float fAmp );
	void		AmpAndMove_ToStereo( float *pStereo, float *pSrc, int iCount, float fAmp );
	void		AmpAndMove_StereoToStereo( float *pStereo, float *pSrc, int iCount, float fAmp );

	void		SetVolume( float fLeft, float fRight );
	void		Retrig();
	bool		Active();

private:
	void		AddFadeOut( float *pLeftStart, float *pRightStart, int iTotal );
	void		AddFadeOut_Stereo( float *pStart, int iTotal );

	float		m_fLeftVolume;
	float		m_fLeftDestVolume;
	float		m_fLeftVolumeRamp;

	float		m_fRightVolume;
	float		m_fRightDestVolume;
	float		m_fRightVolumeRamp;

	float		m_fLastLeftSample;
	float		m_fLastRightSample;
	float		m_fLastLeftSampleRamp;
	float		m_fLastRightSampleRamp;

	float		m_fMaybeLastLeftSample;
	float		m_fMaybeLastRightSample;
};

};

#endif