// BuzzSample.cpp: implementation of the CBuzzSample class.
//
//////////////////////////////////////////////////////////////////////

#include	"BuzzSample.h"
#include	"BuzzInstrument.h"
#include	<MachineInterface.h>

#define WF_EXTENDED		4

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

bool		CBuzzSample::IsExtended()
{
	return (m_pInstrument->m_pWaveInfo->Flags&WF_EXTENDED)?true:false;
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
	if (IsExtended())
		return m_pWaveLevel->pSamples + 4; else
		return m_pWaveLevel->pSamples;
}

long		CBuzzSample::GetSampleLength()
{
	return AdjustSampleCount(m_pWaveLevel->numSamples);
}

long		CBuzzSample::GetLoopStart()
{
	return AdjustSampleCount(m_pWaveLevel->LoopStart);
}

long		CBuzzSample::GetLoopEnd()
{
	return AdjustSampleCount(m_pWaveLevel->LoopEnd);
}

int			CBuzzSample::GetSampleFormat() {
	if (IsExtended()) {
		return m_pWaveLevel->pSamples[0];
	} else
		return 0;
}

int			CBuzzSample::GetBitsPerSample() {
	switch (GetSampleFormat()) {
		case 0:
			return 16;
		case 1:
			return 32;
		case 2:
			return 32;
		case 3:
			return 24;
		default:
			return 16;
	}
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
