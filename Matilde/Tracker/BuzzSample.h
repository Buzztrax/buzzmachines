// BuzzSample.h: interface for the CBuzzSample class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUZZSAMPLE_H__DBE91D09_2682_4476_B331_700B215C6F7B__INCLUDED_)
#define AFX_BUZZSAMPLE_H__DBE91D09_2682_4476_B331_700B215C6F7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISample.h"

class	CBuzzInstrument;
class	CWaveLevel;
class	CWaveInfo;

class	CBuzzSample : public ISample  
{
	friend class CBuzzInstrument;
	friend class CWavetableManager;

public:
							CBuzzSample();
	virtual					~CBuzzSample();

	virtual	float			GetVolume();
	virtual	int				GetRootNote();
	virtual	int				GetRootFrequency();

	virtual	bool			IsValid();
	virtual	bool			IsStereo();
	virtual	bool			IsPingPongLoop();
	virtual	bool			IsLoop();

	virtual	void		*	GetSampleStart();
	virtual	long			GetSampleLength();
	virtual	long			GetLoopStart();
	virtual	long			GetLoopEnd();

	virtual	bool			IsStillValid();

	virtual	void			Free();

protected:
	bool					m_oUsed;

	CBuzzInstrument		*	m_pInstrument;
	int						m_iNote;
	const CWaveLevel	*	m_pWaveLevel;

	int						m_iSavedNumSamples;
	short				*	m_pSavedSamples;

};

#endif // !defined(AFX_BUZZSAMPLE_H__DBE91D09_2682_4476_B331_700B215C6F7B__INCLUDED_)
