#include <assert.h>
#include <math.h>
#include <float.h>
#include "../../common/MachineInterface.h"
#include "../dspchips/DSPChips.h"

#include "Infector.h"

CChannel::CChannel()
{
  Frequency=0.01f;
  FilterEnv.m_nState=4;
  pTrack=NULL;
}

void CChannel::Init()
{
}

void CChannel::ClearFX()
{
}

void CChannel::Reset()
{
	AmpEnv.NoteOff();
	FilterEnv.NoteOff();
	AmpEnv.m_fSilence=1.0/128.0;
  Frequency=0.01f;
	pTrack=NULL;

}

void CChannel::NoteReset()
{
  PhaseOSC1=0.0f;
  PhaseOSC2=0.0f;
  Filter.ResetFilter();
  Phase1=0;
  Phase2=0;
}
