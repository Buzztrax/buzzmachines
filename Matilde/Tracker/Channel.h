// Channel.h: interface for the CChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNEL_H__2EE63E18_CAF9_44DA_9C74_8891690EEB93__INCLUDED_)
#define AFX_CHANNEL_H__2EE63E18_CAF9_44DA_9C74_8891690EEB93__INCLUDED_

#include	"ISample.h"
#include	"IInstrument.h"

class	CTrack;

class	CChannel  
{
public:
							CChannel();
	virtual					~CChannel();

	void					Free();
	void					Reset();
	void					SetRampTime( int iRamp );
	void					SetVolume( float fVol ) { m_fVolume=fVol; UpdateAmp(); }
	void					SetPan( float fPan ) { m_fPan=fPan; UpdateAmp(); }
	void					SetVolumeAndPan( float fVolume, float fPan ) { m_fVolume=fVolume; m_fPan=fPan; UpdateAmp(); }
	bool					Generate_Move( float *psamples, int numsamples );
	bool					Generate_Add( float *psamples, int numsamples );
	int						GetWaveEnvPlayPos( const int env );
	bool					Release();

	SurfDSPLib::CResampler	m_Resampler;
	SurfDSPLib::CAmp		m_Amp;
	SurfDSPLib::C2PFilter	m_Filter;
	CEnvelope				m_VolumeEnvelope;
	CEnvelope				m_PanningEnvelope;
	CEnvelope				m_PitchEnvelope;

	CTrack				*	m_pOwner;

	CMachine			*	m_pMachine;

	ISample				*	m_pSample;
	IInstrument			*	m_pInstrument;
	/*
	const CWaveLevel	*	m_pWaveLevel;
	const CWaveInfo		*	m_pWaveInfo;
	*/

	bool					m_oFree;
	float					m_fPitchEnvFreq;
protected:
	float					m_fVolume;
	float					m_fPan;

	void					UpdateAmp()
	{
#ifdef	MONO
		if( m_pSample )
			m_Amp.SetVolume( m_fVolume*m_pSample->GetVolume(), 0 );
		else
			m_Amp.SetVolume( m_fVolume, 0 );
#else
		if( m_pSample )
			m_Amp.SetVolume( m_fVolume*m_pSample->GetVolume()*(1.0f-m_fPan), m_fVolume*m_pSample->GetVolume()*(1.0f+m_fPan) );
		else
			m_Amp.SetVolume( m_fVolume*(1.0f-m_fPan), m_fVolume*(1.0f+m_fPan) );
		/*
		if( m_pWaveInfo )
			m_Amp.SetVolume( m_fVolume*m_pWaveInfo->Volume*(1.0f-m_fPan), m_fVolume*m_pWaveInfo->Volume*(1.0f+m_fPan) );
		else
			m_Amp.SetVolume( m_fVolume*(1.0f-m_fPan), m_fVolume*(1.0f+m_fPan) );
			*/
#endif
	}
};

#endif // !defined(AFX_CHANNEL_H__2EE63E18_CAF9_44DA_9C74_8891690EEB93__INCLUDED_)
