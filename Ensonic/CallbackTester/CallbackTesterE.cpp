#ifdef WIN32
#include <windows.h>
#else
#include <windef.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include <dsplib.h>

#pragma optimize ("a", on)

#define EGS_ATTACK        0
#define EGS_SUSTAIN        1 
#define EGS_RELEASE        2
#define EGS_NONE        3

#define MIN_AMP          (0.0001 * (32768.0 / 0x7fffffff))

#ifdef _DEBUG
#include <windows.h>
static char DebugStr[1024];
#endif

double const oolog2 = 1.0 / log(2.0);

// global values

CMachineParameter const paraTest = 
{ 
  pt_word,                    // type
  "Test",
  "Test value",                      // description
  1,                        // MinValue  
  0xffff,                      // MaxValue
  0,                        // NoValue
  MPF_STATE,                    // Flags
  16                                              // DefValue
};

// track values

CMachineParameter const paraAttack = 
{ 
  pt_word,                    // type
  "Attack",
  "Attack time in ms",              // description
  1,                        // MinValue  
  0xffff,                      // MaxValue
  0,                        // NoValue
  MPF_STATE,                    // Flags
  16                                              // DefValue
};

CMachineParameter const paraSustain = 
{  
  pt_word,                    // type
  "Sustain",
  "Sustain time in ms",              // description
  1,                        // MinValue  
  0xffff,                      // MaxValue
  0,                        // NoValue
  MPF_STATE,                    // Flags
  16                                              // DefValue
};

CMachineParameter const paraRelease = 
{ 
  pt_word,                    // type
  "Release",
  "Release time in ms",              // description
  1,                        // MinValue  
  0xffff,                      // MaxValue
  0,                        // NoValue
  MPF_STATE,                    // Flags
  512                                             // DefValue
};

CMachineParameter const paraColor = 
{ 
  pt_word,                    // type
  "Color",
  "Noise color (0=black, 1000=white)",      // description
  0,                        // MinValue  
  0x1000,                      // MaxValue
  0xffff,                      // NoValue
  MPF_STATE,                    // Flags
  0x1000                                          // DefValue
};

CMachineParameter const paraVolume = 
{ 
  pt_byte,                    // type
  "Volume",
  "Volume [sustain level] (0=0%, 80=100%, FE=~200%)",  // description
  0,                        // MinValue  
  0xfe,                        // MaxValue
  0xff,                        // NoValue
  MPF_STATE,                    // Flags
  0x80                                            // DefValue
};

CMachineParameter const paraTrigger = 
{ 
  pt_switch,                    // type
  "Trigger",
  "Trigger (1=on, 0=off)",            // description
  -1,                       // MinValue  
  -1,                          // MaxValue
  SWITCH_NO,                      // NoValue
  0,                        // Flags
  0                                               // DefValue
};

CMachineParameter const *pParameters[] = {
    // global
    &paraTest,
  &paraAttack,
  &paraSustain,
  &paraRelease,
  &paraColor,
  &paraVolume,
  &paraTrigger
  // track
};

#pragma pack(1)

class gvals
{
public:
  word test;
  word attack;
  word sustain;
  word release;
  word color;
  byte volume;
  byte trigger;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
  MT_EFFECT,                // type
  MI_VERSION,
  0,                    // flags
  0,                    // min tracks
  0,                    // max tracks
  7,                    // numGlobalParameters
  0,                    // numTrackParameters
  pParameters,
  0, 
  NULL,
#ifdef _DEBUG
  "Buzz Callback Tester E(Debug build)",  // name
#else
  "Buzz Callback Tester E",
#endif
  "BCT E",                  // short name
  "Stefan Sauer",               // author
  NULL
};

class mi : public CMachineInterface
{
public:
  mi();
  virtual ~mi();

  virtual void Init(CMachineDataInput * const pi);
  virtual void Tick();
  virtual bool Work(float *psamples, int numsamples, int const mode);
  virtual void Stop();
  virtual void AttributesChanged();
  virtual void Command(int const i);

  virtual void SetNumTracks(int const n);

public:
  int numTracks;
    gvals gval;

};

DLL_EXPORTS

mi::mi()
{
  GlobalVals = &gval;
}

mi::~mi()
{

}

void mi::Init(CMachineDataInput * const pi)
{
#ifdef _DEBUG
    sprintf(DebugStr,"  mi::Init(%p)",pi);
  OutputDebugString(DebugStr);
#endif
#ifndef _MSC_VER
    DSP_Init(pMasterInfo->SamplesPerSec);
#endif

#if 0
  int i=-1,note=-1;
  const CWaveLevel *lv=pCB->GetNearestWaveLevel(i,note);
  if(lv!=NULL) {
    sprintf(DebugStr,"  CWaveLevel.numSamples: %d",lv->numSamples);
    OutputDebugString(DebugStr);
    sprintf(DebugStr,"  CWaveLevel.pSamples: 0x%x",lv->pSamples);
    OutputDebugString(DebugStr);
    sprintf(DebugStr,"  CWaveLevel.RootNote: %d",lv->RootNote);
    OutputDebugString(DebugStr);
    sprintf(DebugStr,"  CWaveLevel.SamplesPerSec: %d",lv->SamplesPerSec);
    OutputDebugString(DebugStr);
  }
  else {
    OutputDebugString("  CWaveLevel is NULL");
  }
#endif

#ifdef _DEBUG
    sprintf(DebugStr,"    gval.test=%d",gval.test);
  OutputDebugString(DebugStr);
    OutputDebugString("  mi:Init() done");
#endif
}
 
void mi::Tick()
{
#ifdef _DEBUG
    sprintf(DebugStr,"    gval.test=%d",gval.test);
  OutputDebugString(DebugStr);
    // shouldn't this be initialy 0? (it is not)
    sprintf(DebugStr,"  mi::Tick(%d)",pMasterInfo->PosInTick);
  OutputDebugString(DebugStr);

  // the the host callbacks
    const CWaveInfo *wi=pCB->GetWave(0);
    if(wi) {
      sprintf(DebugStr,"CWaveInfo: %d : %f\n",wi->Flags, wi->Volume);
      OutputDebugString(DebugStr);
    }
    else {
      sprintf(DebugStr,"CWaveInfo: NULL\n");
      OutputDebugString(DebugStr);
    }
#endif
}

bool mi::Work(float *psamples, int numsamples, int const)
{
  bool gotsomething = false;
  int i;

  for (i=0; i<numsamples; i++) {
    psamples[i] = 0.0;
  }
  return gotsomething;
}

void mi::Stop()
{
#ifdef _DEBUG
  OutputDebugString("  mi::Stop()");
#endif
}
 
void mi::AttributesChanged() {
#ifdef _DEBUG
    sprintf(DebugStr,"    gval.test=%d",gval.test);
  OutputDebugString(DebugStr);
  OutputDebugString("  mi::AttributesChanged");
#endif
}

void mi::Command(int const i) {
#ifdef _DEBUG
  sprintf(DebugStr,"  mi::Command: %d",i);
  OutputDebugString(DebugStr);
#endif
}

void mi::SetNumTracks(int const n) {
#ifdef _DEBUG
    sprintf(DebugStr,"    gval.test=%d",gval.test);
  OutputDebugString(DebugStr);
  sprintf(DebugStr,"  mi::SetNumTracks: %d->%d",numTracks,n);
  OutputDebugString(DebugStr);
#endif
  numTracks = n;
}
