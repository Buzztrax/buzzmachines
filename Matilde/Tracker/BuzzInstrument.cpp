// BuzzInstrument.cpp: implementation of the CBuzzInstrument class.
//
//////////////////////////////////////////////////////////////////////

#include	"BuzzInstrument.h"
#include	"BuzzSample.h"
#include	"Tracker.h"
#include	<stdlib.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CBuzzSample		CBuzzInstrument::s_Samples[MAX_BUZZSAMPLES];
//int				CBuzzInstrument::s_iNextFreeSample=0;


CBuzzInstrument::CBuzzInstrument()
{

}

CBuzzInstrument::~CBuzzInstrument()
{

}

ISample		*	CBuzzInstrument::GetSample( int iNote )
{
	CBuzzSample	*pSmp=m_pTracker->m_Wavetable.AllocBuzzSample();

	pSmp->m_iNote=iNote;
	pSmp->m_pInstrument=this;
	pSmp->m_pWaveLevel=m_pTracker->pCB->GetNearestWaveLevel( m_iInsNum, iNote );

	if( pSmp->m_pWaveLevel )
	{
		pSmp->m_iSavedNumSamples=pSmp->m_pWaveLevel->numSamples;
		pSmp->m_pSavedSamples=pSmp->m_pWaveLevel->pSamples;
		pSmp->m_oUsed=true;
		return pSmp;
	}
	else
	{
		return NULL;
	}
}

bool	CBuzzInstrument::IsSampleStillValid( CBuzzSample *pSmp )
{
	const CWaveLevel	*wl=m_pTracker->pCB->GetNearestWaveLevel( m_iInsNum, pSmp->m_iNote );

	if( wl )
		return m_pWaveInfo==m_pTracker->pCB->GetWave(m_iInsNum) && pSmp->m_pWaveLevel==wl && wl->numSamples==pSmp->m_iSavedNumSamples && wl->pSamples==pSmp->m_pSavedSamples;
	else
		return false;
}

 