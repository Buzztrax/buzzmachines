
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "..\MachineInterface.h"
#include "..\WahMan3\DSPChips_vc.h"

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		8
// 200 ms przy 44100 Hz
#define MAX_DELAY    32768
#define DELAY_MASK   32767

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraDryOut = 
{ 
	pt_byte,										// type
	"Dry out",
	"Dry out [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraMinDelay = 
{ 
	pt_byte,									// type
	"Minimum delay",
	"Minimum delay [ms]", 		// description
	1,												// MinValue	
	100,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	50,                       // default
};

CMachineParameter const paraModDepth = 
{ 
	pt_byte,									// type
	"Slice size",
	"Slice size [ms]",// description
	1,												// MinValue	
	100,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	20,                       // default
};

CMachineParameter const paraTranspose = 
{ 
	pt_byte,										// type
	"Transpose",
	"Transposition (in #s)",    // description
	0,												  // MinValue	
	48,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	36
};

CMachineParameter const paraFinetune = 
{ 
	pt_byte,										// type
	"Finetune",
	"Finetune",		// description
	0,												// MinValue	
	200,											// MaxValue
	255,											// NoValue
	MPF_STATE,										// Flags
	100
};

CMachineParameter const paraWetOut = 
{ 
	pt_byte,										// type
	"Wet out",
	"Wet out [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraOverlap = 
{ 
	pt_byte,										// type
	"Overlap",
	"Overlap",					// description
	0,												  // MinValue	
	100,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	80
};

CMachineParameter const paraVibDepth = 
{ 
	pt_byte,										// type
	"Vib Depth",
	"Vibrato depth",					// description
	0,												  // MinValue	
	200,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraVibRate = 
{ 
	pt_byte,										// type
	"Vib Rate",
	"Vibrato rate",					// description
	0,												  // MinValue	
	254,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	80
};

CMachineParameter const paraReverse = 
{ 
	pt_switch,										// type
	"Direction",
	"Direction",					// description
	0,												  // MinValue	
	1,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraLFOReset = 
{ 
	pt_switch,										// type
	"LFO Reset",
	"LFO Reset",					// description
	0,												  // MinValue	
	1,												  // MaxValue
	255,										// NoValue
	0,										// Flags
	0
};


CMachineParameter const *pParameters[] = 
{ 
	&paraDryOut,
	&paraModDepth,
	&paraTranspose,
	&paraFinetune,
	&paraWetOut,
	&paraOverlap,
  &paraVibDepth,
  &paraVibRate,
  &paraReverse,
  &paraLFOReset,
};

/*
CMachineAttribute const attrMaxDelay = 
{
	"Max Delay (ms)",
	1,
	100000,
	1000	
};

CMachineAttribute const *pAttributes[] = 
{
	&attrMaxDelay
};
*/

#pragma pack(1)

class gvals
{
public:
	byte dryout;
};

class tvals
{
public:
  byte moddepth;
  byte transpose;
  byte finetune;
  byte wetout;
  byte overlap;
  byte vibdepth;
  byte vibrate;
  byte reverse;
  byte lforeset;
};

class avals
{
public:
//	int maxdelay;
};

class CTrack
{
public:
  float MinDelay;
  float ModDepth;
  float DeltaPhase;
  float Feedback;
	float WetOut;
  int Transpose,Finetune;
  int Overlap;
  int VibRate, VibDepth;
  double LFOPhase, LFODeltaPhase;

  float ReadPtr;
  float Reverse;
  
  float Phase;
};
 
#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	1,										// numGlobalParameters
	9,										// numTrackParameters
	pParameters,
	0,                    // 1 (numAttributes)
	NULL,                 // pAttributes
#ifdef _DEBUG
	"FSM TunaMan (Debug build)",			// name
#else
	"FSM TunaMan",
#endif
	"Tuna",								// short name
	"Krzysztof Foltman",						// author
	"A&bout"
};


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
  float *Buffer;
  int Pos;
  float DryOut;
	int numTracks;
	CTrack Tracks[MAX_TAPS];

public:
	avals aval;
	gvals gval;
	tvals tval[MAX_TAPS];
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = (int *)&aval;
  Buffer = new float[MAX_DELAY];
	Pos=0;
}

mi::~mi()
{
  delete Buffer;
}

#define LFOPAR2SAMPLE(value) (pow(2.0,(value-120)/30.0))

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
  case 0:
  case 4:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
	case 1:   // min/delta delay
		sprintf(txt, "%4.1f ms", (double)((30+2.2*value)/2) );
		break;
	case 2:		// transpose
		sprintf(txt, "%d #", value-24);
		break;
  case 5:		// overlap
		sprintf(txt, "%d %%",value);
		break;
	case 3:		// finetune
		sprintf(txt, "%d ct", value-100);
		break;
  case 6:
		sprintf(txt, "%d ct", value);
		break;
  case 7:
    LfoRateDesc(txt,value);
    break;
  case 8:
    if (value)
      strcpy(txt,"back");
    else
      strcpy(txt,"fwd");
    break;
	default:
		return NULL;
	}

	return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	//if (ptval->mindelay != paraMinDelay.NoValue)
	//	pt->MinDelay = (float)(pMasterInfo->SamplesPerSec * ptval->mindelay/1000.0);
	if (ptval->moddepth != paraModDepth.NoValue)
		pt->ModDepth = (float)(int)(pMasterInfo->SamplesPerSec * ((30+2.2*ptval->moddepth)/1000.0));
	if (ptval->transpose != paraTranspose.NoValue)
		pt->Transpose = (int)(ptval->transpose-24);
	if (ptval->finetune != paraFinetune.NoValue)
		pt->Finetune = (int)(ptval->finetune-100);
  if (ptval->wetout != paraWetOut.NoValue)
    pt->WetOut = ptval->wetout?(float)pow(2.0,(ptval->wetout/10.0-24.0)/6.0):(float)0.0;
  if (ptval->overlap!= paraOverlap.NoValue)
    pt->Overlap = ptval->overlap;
  HANDLE_PARAM(vibdepth, VibDepth)
  HANDLE_PARAM(vibrate, VibRate)
  HANDLE_PARAM(reverse, Reverse)
  pt->LFODeltaPhase=(float)LfoRateToDeltaPhase(pt->VibRate,(int)pMasterInfo->TicksPerSec,(float)pMasterInfo->SamplesPerSec);
  if (ptval->lforeset==1)
    pt->LFOPhase=0.0f;
}

void mi::InitTrack(int const i)
{
  Tracks[i].Phase = 0;
	tvals &tv=tval[i];
	tv.finetune=120;
	tv.lforeset=1;
	tv.moddepth=20;
	tv.reverse=0;
	tv.transpose=36;
	tv.wetout=240;
	tv.vibdepth=0;
	tv.vibrate=80;
	tv.overlap=80;
	TickTrack(&Tracks[i],&tv);
}

void mi::Tick()
{
	for (int c = 0; c < numTracks; c++)
		TickTrack(&Tracks[c], &tval[c]);
  if (gval.dryout!=paraDryOut.NoValue)
  {
    if (gval.dryout)
      DryOut=(float)pow(2.0,(gval.dryout/10.0-24.0)/6.0);
    else
      DryOut=0.0f;
  }
}

void mi::Init(CMachineDataInput * const pi)
{
	numTracks = 1;

  InitTrack(0);

	/*
  for (int c = 0; c < MAX_TAPS; c++)
	{
		Tracks[c].DeltaPhase=0.0f;
		Tracks[c].Feedback=0.0f;
		Tracks[c].ModDepth=4.0f;
		Tracks[c].WetOut=0.3f;
    Tracks[c].Phase=0.0f;
	}

	Tracks[0].WetOut = 0.3f;	// enable first track
  */

  for (int c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;
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


void mi::ResetTrack(int const i)
{
}


//#pragma optimize ("a", on) 

// #define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))


inline float Window(float Phase, float SmoothTime)
{
  return (Phase<SmoothTime)?Phase/SmoothTime:1.0f;
}

/*
inline float Window(float Phase, float MaxPhase)
{
  Phase/=MaxPhase;
  float s=sin(PI*Phase);
  return s*s;
}
*/

static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
  // trk->DeltaPhase=(float)pow(2.0,trk->Transpose/12.0+trk->Finetune/1200.0)+1;
  if (trk->Reverse)
    trk->DeltaPhase=(float)(pow(2.0,trk->Transpose/12.0+trk->Finetune/1200.0)+1);
  else
    trk->DeltaPhase=(float)(-pow(2.0,trk->Transpose/12.0+trk->Finetune/1200.0)+1);
  //trk->DeltaPhase=0.0;
  float *pData=pmi->Buffer;
  int nPos=pmi->Pos;
  float odmd=1/trk->ModDepth;
  int disp=(int)(trk->ModDepth/2);
  float SmoothTime=float(trk->Overlap*0.005f);
  float odSmoothTime=1.0f/SmoothTime;
  float Phase=trk->Phase;
  if (trk->VibDepth)
  {
    float VibTerm=(float)pow(2.0,trk->VibDepth/1200.0*sin(trk->LFOPhase))-1;
    trk->LFOPhase+=c*trk->LFODeltaPhase;
    trk->LFOPhase=(float)fmod(trk->LFOPhase,2*PI);
    for (int i=0; i<c; i++)
    {      
      float Ph=Phase*odmd;
      float A;
      if (Ph<SmoothTime)
      {
        float Win=Ph*odSmoothTime;
        float pos=nPos-(10+Phase);
        if (pos<0) pos+=MAX_DELAY;
        int intpos=f2i(pos);
        float fracpos=pos-intpos;
        float d0=pData[(intpos+0)&DELAY_MASK];
        float d1=pData[(intpos+1)&DELAY_MASK];
        A=Win*INTERPOLATE(fracpos,d0,d1);

        intpos-=disp;
        d0=pData[(intpos+0)&DELAY_MASK];
        d1=pData[(intpos+1)&DELAY_MASK];
        A+=(1-Win)*INTERPOLATE(fracpos,d0,d1);
      }
      else
      {
        float pos=nPos-(10+Phase);
        if (pos<0) pos+=MAX_DELAY;
        int intpos=f2i(pos);
        float fracpos=pos-intpos;
        float d0=pData[(intpos+0)&DELAY_MASK];
        float d1=pData[(intpos+1)&DELAY_MASK];
        A=INTERPOLATE(fracpos,d0,d1);
      }

  //    pout[i]+=trk->WetOut*INTERPOLATE(Win,A,B);
      pout[i]+=trk->WetOut*A;

      Phase+=trk->DeltaPhase+VibTerm;
      trk->LFOPhase+=trk->LFODeltaPhase;
      if (Phase>=disp) Phase-=disp;
      if (Phase<0) Phase+=disp;

      nPos=(nPos+1)&DELAY_MASK;
  //    nPos++;
    }
  }
  else
  {
    for (int i=0; i<c; i++)
    {
      float Ph=Phase*odmd;
      float A;
      if (Ph<SmoothTime)
      {
        float Win=Ph*odSmoothTime;
        float pos=nPos-(10+Phase);
        if (pos<0) pos+=MAX_DELAY;
        int intpos=f2i(pos);
        float fracpos=pos-intpos;
        float d0=pData[(intpos+0)&DELAY_MASK];
        float d1=pData[(intpos+1)&DELAY_MASK];
        A=Win*INTERPOLATE(fracpos,d0,d1);

        intpos-=disp;
        d0=pData[(intpos+0)&DELAY_MASK];
        d1=pData[(intpos+1)&DELAY_MASK];
        A+=(1-Win)*INTERPOLATE(fracpos,d0,d1);
      }
      else
      {
        float pos=nPos-(10+Phase);
        if (pos<0) pos+=MAX_DELAY;
        int intpos=f2i(pos);
        float fracpos=pos-intpos;
        float d0=pData[(intpos+0)&DELAY_MASK];
        float d1=pData[(intpos+1)&DELAY_MASK];
        A=INTERPOLATE(fracpos,d0,d1);
      }

  //    pout[i]+=trk->WetOut*INTERPOLATE(Win,A,B);
      pout[i]+=trk->WetOut*A;

      Phase+=trk->DeltaPhase;
      if (Phase>=disp) Phase-=disp;
      if (Phase<0) Phase+=disp;

      nPos=(nPos+1)&DELAY_MASK;
  //    nPos++;
    }
  }
  trk->Phase=Phase;
  //if (last) pmi->Pos=nPos;
}


#pragma optimize ("", on)


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
  DoWork(pin,pout,this,numsamples,pt);
}

int nEmptySamples=0;

bool mi::Work(float *psamples, int numsamples, int const mode)
{
	float *paux = pCB->GetAuxBuffer();

	if (mode & WM_READ)
  {
		// memcpy(paux, psamples, numsamples*4);
    nEmptySamples=0;
  }
  else
  {
    //if (nEmptySamples>2000)
    //  return false;
    for (int i=0; i<numsamples; i++)
      psamples[i]=0.0;
    nEmptySamples+=numsamples;
  }

  int so=0, maxs=64;

  while(so<numsamples)
  {
    int end=__min(so+maxs,numsamples);
    int c;
    for (c=so; c<end; c++)
      Buffer[(Pos+c-so)&DELAY_MASK]=psamples[c],
      paux[c]=DryOut*psamples[c];
    for (c = 0; c < numTracks; c++)
  		WorkTrack(Tracks + c, psamples+so, paux+so, end-so, mode);
    Pos=(Pos+end-so)&DELAY_MASK;
    so=end;
  }
  if (!(mode&WM_WRITE))
    return (mode&WM_READ)!=0;

//	for (int c = 0; c < numTracks; c++)
//		WorkTrack(Tracks + c, psamples, paux, numsamples, mode);

  memcpy(psamples,paux,numsamples*4);
  int *pint=(int *)psamples;
  for (int i=0; i<numsamples; i++)
    if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
      return true;
	return false;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM TunaMan Pitch Shifter version 1.03\nWritten by Krzysztof Foltman (kf@cw.pl)");
}
