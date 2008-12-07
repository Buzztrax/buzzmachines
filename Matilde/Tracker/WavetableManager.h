// WavetableManager.h: interface for the CWavetableManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVETABLEMANAGER_H__6DB4E742_F3EB_4F41_B083_CEF88343DD24__INCLUDED_)
#define AFX_WAVETABLEMANAGER_H__6DB4E742_F3EB_4F41_B083_CEF88343DD24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class	IInstrument;
class	CMachine;

#include	"BuzzInstrument.h"
#include <MachineInterface.h>

class	CWavetableManager  
{
	friend class CBuzzInstrument;
public:
						CWavetableManager();
						~CWavetableManager();

	IInstrument		*	GetInstrument( int iNum );
	void				SetTracker( CMachine *pTrk );
	void				Stop();

	int					GetUsedSamples();

protected:
	CBuzzSample		*	AllocBuzzSample();
	
	CMachine		*	m_pTracker;

	CBuzzInstrument		m_BuzzInstruments[WAVE_MAX];
	CBuzzSample			m_BuzzSamples[MAX_BUZZSAMPLES];
	int					m_iNextFreeBuzzSample;
};

#endif // !defined(AFX_WAVETABLEMANAGER_H__6DB4E742_F3EB_4F41_B083_CEF88343DD24__INCLUDED_)
