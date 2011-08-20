
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>
#include <dsplib.h>
#include "../dspchips/DSPChips.h"

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		8
// 200 ms przy 44100 Hz
#define MAX_DELAY   (44100*4+10)

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraDryAmp = 
{ 
	pt_byte,										// type
	"Dry amp",
	"Dry amp [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraDryPan = 
{ 
	pt_byte,										// type
	"Dry pan",
	"Dry pan",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraDelayUnit = 
{ 
	pt_byte,										// type
	"Delay unit",
	"Delay unit",					// description
	0,												  // MinValue	
	3,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraFilterType = 
{ 
	pt_byte,										// type
	"Filter type",
	"Filter type",					// description
	0,												  // MinValue	
	5,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraCutoff = 
{ 
	pt_byte,										// type
	"Cutoff",
	"Cutoff frequency",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraResonance = 
{ 
	pt_byte,										// type
	"Resonance",
	"Resonance",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraDelayLen = 
{ 
	pt_word,										// type
	"Tap Delay",
	"Tap Delay Length",					// description
	1,												  // MinValue	
	20000,										  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	3
};

CMachineParameter const paraWetAmp = 
{ 
	pt_byte,										// type
	"Tap Amp",
	"Tap Amp Position",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	180
};

CMachineParameter const paraWetPan = 
{ 
	pt_byte,										// type
	"Tap Pan",
	"Tap Pan Position",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraPhase = 
{ 
	pt_byte,										// type
	"Tap Phase",
	"Tap Phase",					// description
	0,												  // MinValue	
	2,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraFeedback = 
{ 
	pt_byte,										// type
	"Feedback",
	"Feedback",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	150
};

CMachineParameter const *pParameters[] = 
{ 
	&paraDryAmp,
	&paraDryPan,
	&paraDelayUnit,
	&paraFilterType,
	&paraCutoff,
	&paraResonance,

  &paraDelayLen,
  &paraWetAmp,
  &paraWetPan,
  &paraPhase,
  &paraFeedback,
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
	byte dryamp;
  byte drypan;
  byte delayunit;
  byte filtertype;
  byte cutoff;
  byte resonance;
};

class tvals
{
public:
  word delaylen;
  byte wetamp;
  byte panpos;
  byte phase;
  byte feedback;
};

class avals
{
public:
//	int maxdelay;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	MIF_MONO_TO_STEREO,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	6,										// numGlobalParameters
	5,										// numTrackParameters
	pParameters,
	0,                    // 1 (numAttributes)
	NULL,                 // pAttributes
#ifdef _DEBUG
	"FSM TapMan (Debug build)",			// name
#else
	"FSM TapMan",
#endif
	"FSMTapMan",								// short name
	"Krzysztof Foltman",						// author
	"A&bout"
};

class CTrack
{
public:
  word delaylen;
  byte panpos;
  byte wetamp;
  byte phase;
  byte feedback;

  int DelaySamples;
  float WetAmp;
/*
  int Length;
	int Pos;
	int Unit;
  */
};
 

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
  virtual bool Work(float *psamples, int numsamples, int const mode) { return false; }
  virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);

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
  float DryPan;
  float Limiter,RunningSum;
  int DelayUnit;
  int FilterType;
  int Cutoff, Resonance;

private:
	int numTracks;
	CTrack Tracks[MAX_TAPS];

	avals aval;
	gvals gval;
	tvals tval[MAX_TAPS];

  CBiquad m_filter;
  double Gain,GainLimiter;

  int m_nAge;
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = (int *)&aval;
  Buffer = new float[MAX_DELAY];
  m_nAge=0;
  Limiter=1.0f;
  RunningSum=0.0f;
}

mi::~mi()
{
  delete Buffer;
}

#define LFOPAR2TIME(value) (0.05*pow(800.0,value/255.0))

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
  case 0:
  case 7:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
  case 1:
  case 8:
    if (value<120) sprintf(txt,"%d%% L",(120-value)*100/120);
    else if (value==120) strcpy(txt,"Mid");
    else sprintf(txt,"%d%% R",(value-120)*100/120);
    break;
  case 2:
    if (value==0) strcpy(txt,"Ticks");
    if (value==1) strcpy(txt,"Ticks/256");
    if (value==2) strcpy(txt,"Samples");
    if (value==3) strcpy(txt,"milisecs");
    break;
  case 3:
    if (value==0) strcpy(txt,"None");
    if (value==1) strcpy(txt,"Lowpass");
    if (value==2) strcpy(txt,"Bandpass");
    if (value==3) strcpy(txt,"Kewl LP");
    if (value==4) strcpy(txt,"Kewl HP");
    if (value==5) strcpy(txt,"Kewl BP");
    break;
  case 9:
    if (value==0) strcpy(txt,"Normal");
    if (value==1) strcpy(txt,"Inv Left");
    if (value==2) strcpy(txt,"Inv Both");
    break;
  case 10:
    if (value!=120) sprintf(txt,"%d%%",(value-120)*100/120);
    else
      strcpy(txt,"No Feedback");
    break;
/*
	case 1:   // min/delta delay
  case 2:
		sprintf(txt, "%4.1f ms", (double)(value/10.0) );
		break;
	case 3:		// LFO rate
		sprintf(txt, "%5.3f Hz", (double)LFOPAR2TIME(value));
		break;
    */
	default:
		return NULL;
	}

	return txt;
}

void mi::Init(CMachineDataInput * const pi)
{
  int c;
	numTracks = 1;

	for (c = 0; c < MAX_TAPS; c++)
	{
    CTrack *pt=Tracks+c;
    pt->delaylen=paraDelayLen.DefValue;
    pt->panpos=paraWetPan.DefValue;
    pt->wetamp=paraWetAmp.DefValue;
    pt->phase=paraPhase.DefValue;
    pt->feedback=paraFeedback.DefValue;
	}

  for (c=0; c<MAX_DELAY; c++)
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


void mi::InitTrack(int const i)
{
}

void mi::ResetTrack(int const i)
{
}


void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->delaylen != paraDelayLen.NoValue)
    pt->delaylen=ptval->delaylen;
	if (ptval->panpos != paraWetPan.NoValue)
    pt->panpos=ptval->panpos;
	if (ptval->wetamp != paraWetAmp.NoValue)
    pt->wetamp=ptval->wetamp;
	if (ptval->phase!= paraPhase.NoValue)
    pt->phase=ptval->phase;
	if (ptval->feedback!= paraPhase.NoValue)
    pt->feedback=ptval->feedback;

}

void mi::Tick()
{
	for (int c = 0; c < numTracks; c++)
		TickTrack(&Tracks[c], &tval[c]);
  if (gval.dryamp!=paraDryAmp.NoValue)
  {
    if (gval.dryamp)
      DryOut=(float)pow(2.0,(gval.dryamp/10.0-24.0)/6.0);
    else
      DryOut=0.0f;
  }
  if (gval.drypan!=paraDryPan.NoValue)
    DryPan=gval.drypan;
  if (gval.delayunit!=paraDelayUnit.NoValue)
    DelayUnit=gval.delayunit;
  if (gval.filtertype!=paraFilterType.NoValue)
    FilterType=gval.filtertype;
  if (gval.cutoff!=paraCutoff.NoValue)
    Cutoff=gval.cutoff;
  if (gval.resonance!=paraResonance.NoValue)
    Resonance=gval.resonance;
}

#pragma optimize ("a", on) 

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
  float *pData=Buffer;

  int nDelaySamples=0;
  if (DelayUnit==0) nDelaySamples=(int)(pt->delaylen*pMasterInfo->SamplesPerTick);
  if (DelayUnit==1) nDelaySamples=(int)(pt->delaylen*pMasterInfo->SamplesPerTick/256);
  if (DelayUnit==2) nDelaySamples=(int)(pt->delaylen);
  if (DelayUnit==3) nDelaySamples=(int)(pt->delaylen*pMasterInfo->SamplesPerSec/1000);
  if (nDelaySamples<256)
    nDelaySamples=256;

  if (nDelaySamples>MAX_DELAY-1024)
    nDelaySamples=MAX_DELAY-1024;
  pt->DelaySamples=nDelaySamples;

  int nPos=Pos-nDelaySamples;
  if (nPos<0) nPos+=MAX_DELAY;

  float WetOut=0.0f;
  if (pt->wetamp)
    WetOut=(float)pow(2.0,(pt->wetamp/10.0-24.0)/6.0);
  float LeftAmp=(float)(WetOut*sqrt((240-pt->panpos)/240.0));
  float RightAmp=(float)(WetOut*sqrt(1-(240-pt->panpos)/240.0));
  if (pt->phase!=0) LeftAmp*=-1.f;
  if (pt->phase==2) RightAmp*=-1.f;
  
  if (pt->feedback==120)
  {
    for (int i=0; i<numsamples; i++)
    {
      float sig=pData[nPos++];
      nPos=nPos>=MAX_DELAY?nPos-MAX_DELAY:nPos;
      pout[i*2]+=LeftAmp*sig;
      pout[i*2+1]+=RightAmp*sig;
    }
  }
  else
  {
    float Feedback=float((pt->feedback-120)/120.0*GainLimiter);
    int nPos2=Pos;
    for (int i=0; i<numsamples; i++)
    {
      float sig=pData[nPos++];
      nPos=nPos>=MAX_DELAY?nPos-MAX_DELAY:nPos;
      nPos2=nPos2>=MAX_DELAY?nPos2-MAX_DELAY:nPos2;
      pout[i*2]+=LeftAmp*sig;
      pout[i*2+1]+=RightAmp*sig;
      Buffer[nPos2++]+=Feedback*sig;
    }
  }
}

#pragma optimize ("", on)

bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
  float *pData=Buffer;
  int c,i;

  if (Pos>=MAX_DELAY || Pos<0)
    Pos=0;

  Gain=0.0;
  for (c = 0; c < numTracks; c++)
		Gain+=pow(fabs((Tracks[c].feedback-120)/120.0),1.0/Tracks[c].DelaySamples);

  if (Gain<1.0)
    GainLimiter=1.0;
  else
    GainLimiter=1.0/Gain;

  if (!(mode&WM_READ))
  {
    DSP_Zero(pin,numsamples);
    m_nAge+=numsamples;
    /*
    char buf[256];
    sprintf(buf,"%g",gain);
    pCB->MessageBox(buf);
    */
    if (pow(Gain,m_nAge)<0.01)
      return false;
  }
  else
    m_nAge=0;
  if (DryOut!=0.0)
  {
    float LeftAmp=(float)(DryOut*sqrt(1-DryPan/240.0));
    float RightAmp=(float)(DryOut*sqrt(DryPan/240.0));
    for (int i=0; i<numsamples; i++)
      pout[2*i]=pin[i]*LeftAmp,
      pout[2*i+1]=pin[i]*RightAmp;
  }
  else
  {
    DSP_Zero(pout,2*numsamples);
  }

  {
    int nPos=Pos;
    for (i=0; i<numsamples; i++)
    {
      if (nPos>=MAX_DELAY) nPos-=MAX_DELAY;
      pData[nPos++]=pin[i];
    }
  }

  for (c = 0; c < numTracks; c++)
		WorkTrack(Tracks + c, pin, pout, numsamples, mode);

  int nPos=Pos;
  if (FilterType && Gain!=0.0f)
  {
    float Gain=1.0f;
    if (FilterType==1) m_filter.SetResonantLP((float)(264*pow(15000.0/264,Cutoff/240.0)),float(1.01+2*Resonance/240.0),44100),
      Gain=float(1.0f/(1.5+6*Resonance/240.0));
    if (FilterType==2)
      m_filter.SetBandpass(float(264*pow(15000.0/264,Cutoff/240.0)),float(1280/pow(8,Resonance/240.0)*pow(15000.0/264,Cutoff/240.0)),44100);
    if (FilterType==3) m_filter.rbjLPF((float)(264*pow(15000.0/264,Cutoff/240.0)),float(0.6+2*Resonance/240.0),44100);
    if (FilterType==4) m_filter.rbjHPF((float)(264*pow(15000.0/264,Cutoff/240.0)),float(0.6+2*Resonance/240.0),44100);
    if (FilterType==5) m_filter.rbjBPF((float)(264*pow(15000.0/264,Cutoff/240.0)),float(0.1+1.4*Resonance/240.0),44100);
    for (int i=0; i<numsamples; i++)
    {
      if (nPos>=MAX_DELAY) nPos-=MAX_DELAY;
      float Smp=Limiter*Gain*m_filter.ProcessSampleSafe(pData[nPos]);
      RunningSum=RunningSum*0.99+Smp*Smp*0.01;
      if (Smp>32000 || Smp<-32000) Limiter*=0.99f;
      else
      if (RunningSum<16000*16000 && Limiter<1.0f)
      {
        Limiter*=1.001f;
        Limiter=__min(Limiter,1.0f);
      }
      pData[nPos++]=Smp;
    }
    if (nPos>=MAX_DELAY) nPos-=MAX_DELAY;
  }
  else
  {
    nPos+=numsamples;
    if (nPos>=MAX_DELAY) nPos-=MAX_DELAY;
  }
  Pos=nPos;

  int *pint=(int *)pout;
  for (i=0; i<2*numsamples; i++)
    if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
      return true;
	return false;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM TapMan version 1.2\nWritten by Krzysztof Foltman (kf@onet.pl), Gfx by Oom\nKewl filters by Robert Bristow-Johnston (thx a lot!!!)\nSpecial thx to: canc3r & Oom\n\n"
    "Warning ! Because TapMan 1.2 introduced new (Kewl) filter types,\n"
    "if you publish your BMX/BMWs using that filters, place a warning\n"
    "in Notes section saying, that your song requires TapMan version 1.2\n"
    "\n"
    "Warning 2 ! Kewl filters could cause TapMan to go self-oscillate wildly.\n"
    "I've implemented a very primitive limiter so that you won't burn your\n"
    "speakers (at least you shouldn't be able to do that)\n\n"
    "Visit my homepage at www.mp3.com/FSMachine\n"
    "(buzz-generated goa trance) and grab my songs ! :-)");
}
