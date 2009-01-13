
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"

#define MAX_TAPS    1

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraCutoff = 
{ 
  pt_byte,                    // type
  "Cutoff",
  "Cutoff frequency",          // description
  0,                           // MinValue  
  240,                         // MaxValue
  255,                         // NoValue
  MPF_STATE,                   // Flags
  120
};

CMachineParameter const paraResonance = 
{ 
  pt_byte,                    // type
  "Resonance",
  "Resonance",                // description
  0,                          // MinValue  
  240,                        // MaxValue
  255,                        // NoValue
  MPF_STATE,                  // Flags
  120
};

CMachineParameter const paraModDepth = 
{ 
  pt_byte,                  // type
  "Modulation",
  "Modulation depth",// description
  0,                        // MinValue  
  240,                      // MaxValue
  255,                      // NoValue
  MPF_STATE,                // Flags
  60,                       // default
};

CMachineParameter const paraLFORate = 
{ 
  pt_byte,                    // type
  "LFO rate",
  "LFO rate [Hz]",            // description
  0,                          // MinValue  
  240,                        // MaxValue
  255,                        // NoValue
  MPF_STATE,                  // Flags
  30
};

CMachineParameter const paraLFOPhase = 
{ 
  pt_byte,                    // type
  "Set phase",
  "Set LFO phase",            // description
  0,                          // MinValue  
  127,                        // MaxValue
  255,                        // NoValue
  MPF_STATE,                  // Flags
  0
};

CMachineParameter const paraInertia = 
{ 
  pt_byte,                    // type
  "Inertia",
  "Cutoff frequency inertia", // description
  0,                          // MinValue  
  240,                        // MaxValue
  255,                        // NoValue
  MPF_STATE,                  // Flags
  40
};


CMachineParameter const *pParameters[] = 
{ 
  &paraCutoff,
  &paraResonance,
  &paraLFORate,
  &paraModDepth,
  &paraLFOPhase,
  &paraInertia,
};

CMachineAttribute const attrMaxDelay = 
{
  "Theviderness factor",
  0,
  20,
  0
};

CMachineAttribute const *pAttributes[] = 
{
  &attrMaxDelay
};

#pragma pack(1)

class gvals
{
public:
  byte dryout;
};

class tvals
{
public:
  byte cutoff;
  byte resonance;
  byte lforate;
  byte lfodepth;
  byte lfophase;
  byte inertia;
};

class avals
{
public:
  int thevfactor;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
  MT_EFFECT,                // type
  MI_VERSION,
  0,                    // flags
  1,                    // min tracks
  MAX_TAPS,                // max tracks
  0,                    // numGlobalParameters
  6,                    // numTrackParameters
  pParameters,
  1,                    // 1 (numAttributes)
  pAttributes,                 // pAttributes
#ifdef _DEBUG
  "FSM WahMan (Debug build)",      // name
#else
  "FSM WahMan",
#endif
  "FSMWahMan",                // short name
  "Krzysztof Foltman",            // author
  "A&bout"
};

class CTrack
{
public:
  float Cutoff;
  float Resonance;
  float LFORate;
  float LFODepth;
  float Inertia;
  double LFOPhase;
  double DeltaPhase;

  float CurCutoff;

  float a0,a1,a2;
  float x1,y1,x2,y2;

  int thevfactor;

  void CalcCoeffs();
};
 

void CTrack::CalcCoeffs()
{
  int sr=44100;

  float CutoffFreq=(float)(264*pow(32,CurCutoff/240.0));
  float cf=(float)CutoffFreq;
  if (cf>=20000) cf=20000; // clip frequency to sane range
  if (cf<33) cf=(float)(33.0);
  float tf=(float)(thevfactor/20.0);
  float ScaleResonance=(float)pow(cf/20000.0,tf);

  float fQ=(float)(1.01+5*Resonance*ScaleResonance/240.0);

  float fB=(float)sqrt(fQ*fQ-1)/fQ;
  float fA=(float)(2*fB*(1-fB));

  float A,B;

  float ncf=(float)(1.0/tan(3.1415926*cf/(double)sr));
  A=fA*ncf;      // denormalization and taking into account the sampling frequency (prewarping for bilinear transform)
  B=fB*ncf*ncf;
  a0=1/(1+A+B);// calculation of digital filter coefficients (bilinear transform)
  a1=a0*(2-B-B);
  a2=a0*(1-A+B);
}

class mi : public CMachineInterface
{
public:
  mi();
  virtual ~mi();

  virtual void Init(CMachineDataInput * const pi);
  virtual void Tick();
  virtual bool Work(float *psamples, int numsamples, int const mode);

  virtual void SetNumTracks(int const n);

  virtual void AttributesChanged();

  virtual char const *DescribeValue(int const param, int const value);

  virtual void Command(int const i);

private:
  void InitTrack(int const i);
  void ResetTrack(int const i);

  void TickTrack(CTrack *pt, tvals *ptval);
  void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  int Pos;
  float DryOut;

private:
  int numTracks;
  CTrack Tracks[MAX_TAPS];

  avals aval;
  gvals gval;
  tvals tval[MAX_TAPS];

  int zeroSamples;
};

DLL_EXPORTS

mi::mi()
{
  GlobalVals = &gval;
  TrackVals = tval;
  AttrVals = (int *)&aval;
  zeroSamples = 1024;
}

mi::~mi()
{
}

#define LFOPAR2TIME(value) (0.05*pow(800.0,value/255.0))

char const *mi::DescribeValue(int const param, int const value)
{
  static char txt[36];

  switch(param)
  {
  case 2:    // LFO rate
    sprintf(txt, "%5.3f Hz", (double)LFOPAR2TIME(value));
    break;
  default:
    return NULL;
  }

  return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
  pt->thevfactor=aval.thevfactor;
  if (ptval->lforate != paraLFORate.NoValue)
    pt->DeltaPhase = (float)(2*3.1415926*LFOPAR2TIME(ptval->lforate)/pMasterInfo->SamplesPerSec);
  if (ptval->lfophase != paraLFOPhase.NoValue)
    pt->LFOPhase = (float)(2*3.1415926*ptval->lfophase/128.0);
  if (ptval->lfodepth!= paraModDepth.NoValue)
    pt->LFODepth = (float)(ptval->lfodepth);
  if (ptval->inertia!= paraInertia.NoValue)
    pt->Inertia = (float)(ptval->inertia/240.0);
  if (ptval->cutoff!= paraCutoff.NoValue)
    pt->Cutoff = ptval->cutoff;
  if (ptval->resonance!= paraResonance.NoValue)
    pt->Resonance = (float)(ptval->resonance);
 }



void mi::Init(CMachineDataInput * const pi)
{
  numTracks = 1;

  for (int c = 0; c < MAX_TAPS; c++)
  {
    tvals vals;
    vals.cutoff=paraCutoff.DefValue;
    vals.resonance=paraResonance.DefValue;
    vals.lforate=paraLFORate.DefValue;
    vals.lfodepth=paraModDepth.DefValue;
    vals.lfophase=paraLFOPhase.DefValue;
    vals.inertia=paraInertia.DefValue;
    TickTrack(&Tracks[c], &vals);
    Tracks[c].x1=Tracks[c].y1=Tracks[c].x2=Tracks[c].y2=0.0f;
  }
}

void mi::AttributesChanged()
{
/*
  MaxDelay = (int)(pMasterInfo->SamplesPerSec * (aval.maxdelay / 1000.0));
  for (int c = 0; c < numTracks; c++)
    InitTrack(c);
    */
}


void mi::SetNumTracks(int const n)
{
  if (numTracks < n)
  {
    for (int c = numTracks; c < n; c++)
      InitTrack(c);
  }
  else if (n < numTracks)
  {
    for (int c = n; c < numTracks; c++)
      ResetTrack(c);
  
  }
  numTracks = n;

}


void mi::InitTrack(int const i)
{
  Tracks[i].LFOPhase = 0;
  Tracks[i].x1 = Tracks[i].x2 = Tracks[i].y1 = Tracks[i].y2 = 0;
}

void mi::ResetTrack(int const i)
{
}


void mi::Tick()
{
  for (int c = 0; c < numTracks; c++)
    TickTrack(&Tracks[c], &tval[c]);
}

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
  float &x1=trk->x1, &x2=trk->x2, &y1=trk->y1, &y2=trk->y2;
  float a0=trk->a0, a1=trk->a1, a2=trk->a2;
  float ai=10*(1-trk->Inertia);
  for (int i=0; i<c; i+=64) // pow(2,CurCutoff/48.0)
  {
    float DestCutoff=(float)(trk->Cutoff+trk->LFODepth*sin(trk->LFOPhase)/2); // pow(2.0,trk->LFODepth*sin(trk->LFOPhase)/100.0);
    if (fabs(trk->CurCutoff-DestCutoff)<ai)
      trk->CurCutoff=DestCutoff;
    else
      trk->CurCutoff+=(float)_copysign(ai,DestCutoff-trk->CurCutoff);
    trk->CalcCoeffs();

    int jmax=__min(i+64,c);
    for (int j=i; j<jmax; j++)
    {
      float in=pin[j];
      float res=(float)(trk->a0*(in+x1+x1+x2)-a1*y1-a2*y2);
      if (res>-0.1 && res<0.1) res=0.0;
      if (res>320000) res=320000;
      if (res<-320000) res=-320000;
      x2=x1;y2=y1;
      x1=in;y1=res;
      pout[j]=res;
    }
    trk->LFOPhase+=(jmax-i)*trk->DeltaPhase;
  }
}

void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
  DoWork(pin,pout,this,numsamples,pt);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
  float *paux = pCB->GetAuxBuffer();

  if (mode & WM_READ)
  {
    memcpy(paux, psamples, numsamples*4);
    zeroSamples=0;
  }
  else
  {
    CTrack *trk=Tracks;
    if (zeroSamples>500 && fabs(trk->y1)<1 && fabs(trk->y2)<1)
    {
      trk->LFOPhase+=numsamples*trk->DeltaPhase;
      return false;
    }
    zeroSamples+=numsamples;
    for (int i=0; i<numsamples; i++)
      paux[i]=0.0;
  }

  for (int c = 0; c < numTracks; c++)
    WorkTrack(Tracks + c, paux, psamples, numsamples, mode);

  return true;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM WahMan version 1.1 !\nWritten by Krzysztof Foltman (kf@cw.pl), Gfx by Oom\n\n"
    "For new songs, please don't use WahMan - FSM WahManPro2 is MUCH better !\n\n");
}

