// BuzzSample.cpp: implementation of the CBuzzSample class.
//
//////////////////////////////////////////////////////////////////////

#include	"BuzzSample.h"
#include	"BuzzInstrument.h"
#include <MachineInterface.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuzzSample::CBuzzSample() : m_oUsed(false)
{

}

CBuzzSample::~CBuzzSample()
{

}

float		CBuzzSample::GetVolume()
{
	return m_pInstrument->m_pWaveInfo->Volume;
}

int			CBuzzSample::GetRootNote()
{
	return m_pWaveLevel->RootNote;
}

int			CBuzzSample::GetRootFrequency()
{
	return m_pWaveLevel->SamplesPerSec;
}

bool		CBuzzSample::IsValid()
{
	return m_iSavedNumSamples && m_pSavedSamples;
}

bool		CBuzzSample::IsStereo()
{
	return (m_pInstrument->m_pWaveInfo->Flags&WF_STEREO)?true:false;
}

bool		CBuzzSample::IsPingPongLoop()
{
	return ((m_pInstrument->m_pWaveInfo->Flags&WF_BIDIR_LOOP)?true:false) && m_pWaveLevel->LoopEnd>m_pWaveLevel->LoopStart;
}

bool		CBuzzSample::IsLoop()
{
	return ((m_pInstrument->m_pWaveInfo->Flags&WF_LOOP)?true:false) && m_pWaveLevel->LoopEnd>m_pWaveLevel->LoopStart;
}

void	*	CBuzzSample::GetSampleStart()
{
	return m_pWaveLevel->pSamples;
}

long		CBuzzSample::GetSampleLength()
{
	return m_pWaveLevel->numSamples;
}

long		CBuzzSample::GetLoopStart()
{
	return m_pWaveLevel->LoopStart;
}

long		CBuzzSample::GetLoopEnd()
{
	return m_pWaveLevel->LoopEnd;
}

bool		CBuzzSample::IsStillValid()
{
	return m_pInstrument->IsSampleStillValid( this );
}

void		CBuzzSample::Free()
{
	m_iSavedNumSamples=0;
	m_pSavedSamples=NULL;
	m_oUsed=false;
}
