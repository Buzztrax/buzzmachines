
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <windef.h>
#include <MachineInterface.h>
#include <mdk/mdk.h>
#include "../dspchips/DSPChips.h"

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		4
// 200 ms at 44100 Hz
#define MAX_DELAY   131072
#define DELAY_MASK  131070

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

CMachineParameter const paraDryLeftPan = 
{ 
	pt_byte,										// type
	"Dry L Pan",
	"Dry left channel pan",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraDryRightPan = 
{ 
	pt_byte,										// type
	"Dry R Pan",
	"Dry right channel pan",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
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
	3,												  // MaxValue
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

CMachineParameter const paraLeftDelayLen = 
{ 
	pt_word,										// type
	"L Delay Len",
	"Length of the left delay line",					// description
	1,												  // MinValue	
	20000,										  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	3
};

CMachineParameter const paraRightDelayLen = 
{ 
	pt_word,										// type
	"R Delay Len",
	"Length of the right delay line",					// description
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
	240
};

CMachineParameter const paraWetLeftPan = 
{ 
	pt_byte,										// type
	"Wet L Pan",
	"Tap left channel pan position",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraWetRightPan = 
{ 
	pt_byte,										// type
	"Wet R Pan",
	"Tap right channel pan position",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraFeedback = 
{ 
	pt_byte,										// type
	"FB Amp",
	"FB Amplitude",					// description
	0,												  // MinValue	
	239,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	150
};

CMachineParameter const paraFeedbackLeftPan = 
{ 
	pt_byte,										// type
	"FB L Pan",
	"FB left channel pan position",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraFeedbackRightPan = 
{ 
	pt_byte,										// type
	"FB R Pan",
	"FB right channel pan position",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraPingPong = 
{ 
	pt_byte,										// type
	"FB PingPong",
	"Reverse channels in feedback",					// description
	0,												  // MinValue	
	1,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraSpread = 
{ 
	pt_byte,										// type
	"FB Spread",
	"Stereo spread in feedback",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};



CMachineParameter const *pParameters[] = 
{ 
	&paraDryAmp,
	&paraDryLeftPan,
	&paraDryRightPan,
	&paraDelayUnit,
	&paraFilterType,
	&paraCutoff,
	&paraResonance,

  &paraLeftDelayLen,
  &paraRightDelayLen,
  &paraWetAmp,
  &paraWetLeftPan,
	&paraWetRightPan,
	&paraFeedback,
  &paraPingPong,
  &paraSpread,
//  &paraFeedbackLeftPan,
//	&paraFeedbackRightPan,
};

/*
CMachineAttribute const *pAttributes[] = 
{
};
*/

#pragma pack(1)

class gvals
{
public:
	byte dryamp;
  byte dryleftpan;
  byte dryrightpan;
  byte delayunit;
  byte filtertype;
  byte cutoff;
  byte resonance;
};

class tvals
{
public:
  word leftdelaylen;
  word rightdelaylen;
  byte wetamp;
  byte leftpanpos;
  byte rightpanpos;
  byte feedbackamp;
  byte pingpong;
  byte spread;
  // byte fblpanpos;
  // byte fbrpanpos;
};


class avals
{
public:
	int lfoshape;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	MIF_DOES_INPUT_MIXING,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	7,										// numGlobalParameters
	8,										// numTrackParameters
	pParameters,
	0, // sizeof(pAttributes)/4,                    // 1 (numAttributes)
	NULL, //pAttributes,                 // pAttributes
#ifdef _DEBUG
	"FSM PanzerDelay (Debug build)",			// name
#else
	"FSM PanzerDelay",
#endif
	"PanzDelay",								// short name
	"Krzysztof Foltman",						// author
	"A&bout"
};

class CTrack
{
public:
/*
  int Length;
	int Pos;
	int Unit;
  */
  word leftdelaylen;
  word rightdelaylen;
  byte wetamp;
  byte leftpanpos;
  byte rightpanpos;
  byte fbimage;
  byte feedbackamp;
  byte pingpong;
  byte spread;

  int LeftDelaySamples;
  int RightDelaySamples;
  float WetAmp;
  float Feedback;
};
 
class miex : public CMDKMachineInterfaceEx
{
};

class mi : public CMDKMachineInterface
{
public:
  miex ex;
  bool isstereo;
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
  virtual void OutputModeChanged(bool stereo) {  }

	mi();
	virtual ~mi();

	virtual void MDKInit(CMachineDataInput * const pi);
  virtual void MDKSave(CMachineDataOutput * const po) {}
	virtual void Tick();
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);

	//virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);
	virtual void Command(int const i);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);
  void PrepareTrack(int tno);
	void WorkTrackStereo(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  float *Buffer;
  int Pos;
  int DelayUnit;
  float DryOut,DryLeftPan,DryRightPan;
  float FeedbackLimiter;
	int numTracks;
	CTrack Tracks[MAX_TAPS];
  int FilterType,Cutoff,Resonance;
  CBiquad m_filterL, m_filterR;
  CUglyLimiter m_limL, m_limR;

private:

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

  float slope=12.0f;
  
}

mi::~mi()
{
  delete []Buffer;
  numTracks=-1;
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
  case 0:
  case 9:
  case 12:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
  case 3:
    if (value==0) strcpy(txt,"Ticks");
    if (value==1) strcpy(txt,"Ticks/256");
    if (value==2) strcpy(txt,"Samples");
    if (value==3) strcpy(txt,"milisecs");
    break;
  case 4:
    if (value==0) strcpy(txt,"None");
    if (value==1) strcpy(txt,"LP");
    if (value==2) strcpy(txt,"HP");
    if (value==3) strcpy(txt,"BP");
    break;
  case 1:
  case 2:
  case 10:
  case 11:
    if (value<120) sprintf(txt,"%d%% L",(120-value)*100/120);
    else if (value==120) strcpy(txt,"Mid");
    else sprintf(txt,"%d%% R",(value-120)*100/120);
    break;
  case 13:
    if (value==0) strcpy(txt,"Off");
    if (value==1) strcpy(txt,"On");
    break;
  case 14:
    if (value<120) sprintf(txt,"%d%% inside",(120-value)*100/120);
    else if (value==120) strcpy(txt,"No change");
    else sprintf(txt,"%d%% outside",(value-120)*100/120);
    break;
/*
	case 2:   // min/delta delay
  case 3:
		sprintf(txt, "%4.1f ms", (double)(value/10.0) );
		break;
	case 4:		// LFO rate
    LfoRateDesc(txt,value);
		break;
  case 7:
    sprintf(txt, "%4.1f deg", (double)((value-64)*180/64.0) );
    break;
    */
	default:
		return NULL;
	}

	return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->leftdelaylen != paraLeftDelayLen.NoValue)
    pt->leftdelaylen=ptval->leftdelaylen;
	if (ptval->rightdelaylen != paraRightDelayLen.NoValue)
    pt->rightdelaylen=ptval->rightdelaylen;
	if (ptval->leftpanpos != paraWetLeftPan.NoValue)
    pt->leftpanpos=ptval->leftpanpos;
	if (ptval->rightpanpos!= paraWetRightPan.NoValue)
    pt->rightpanpos=ptval->rightpanpos;
	if (ptval->spread != paraSpread.NoValue)
    pt->spread=ptval->spread;
	if (ptval->wetamp != paraWetAmp.NoValue)
  {
    pt->wetamp=ptval->wetamp;
    if (pt->wetamp)
      pt->WetAmp=(float)pow(2.0,(pt->wetamp/10.0-24.0)/6.0);
    else
      pt->WetAmp=0.0f;
  }
	if (ptval->feedbackamp != paraFeedback.NoValue)
  {
    pt->feedbackamp=ptval->feedbackamp;
    if (pt->feedbackamp)
      pt->Feedback=(float)pow(2.0,(pt->feedbackamp/10.0-24.0)/6.0);
    else
      pt->Feedback=0.0f;
  }
	if (ptval->pingpong!= paraPingPong.NoValue)
    pt->pingpong=ptval->pingpong;
/*
	if (ptval->fblpanpos!= paraFeedbackLeftPan.NoValue)
    pt->fblpanpos=ptval->fblpanpos;
	if (ptval->fbrpanpos!= paraFeedbackRightPan.NoValue)
    pt->fbrpanpos=ptval->fbrpanpos;
    */
  
  pt->LeftDelaySamples=2*DelayLenToSamples(DelayUnit,pt->leftdelaylen,pMasterInfo->SamplesPerTick,pMasterInfo->SamplesPerSec);
  if (pt->LeftDelaySamples<100) pt->LeftDelaySamples=100;
  if (pt->LeftDelaySamples>DELAY_MASK-400) pt->LeftDelaySamples=DELAY_MASK-400;
  
  pt->RightDelaySamples=2*DelayLenToSamples(DelayUnit,pt->rightdelaylen,pMasterInfo->SamplesPerTick,pMasterInfo->SamplesPerSec);
  if (pt->RightDelaySamples<100) pt->RightDelaySamples=100;
  if (pt->RightDelaySamples>DELAY_MASK-400) pt->RightDelaySamples=DELAY_MASK-400;
}

void mi::Tick()
{
  if (gval.dryamp!=paraDryAmp.NoValue)
  {
    if (gval.dryamp)
      DryOut=(float)pow(2.0,(gval.dryamp/10.0-24.0)/6.0);
    else
      DryOut=0.0f;
  }
  if (gval.delayunit!=paraDelayUnit.NoValue)
    DelayUnit=gval.delayunit;
  if (gval.dryleftpan!=paraDryLeftPan.NoValue)
    DryLeftPan=gval.dryleftpan;
  if (gval.dryrightpan!=paraDryRightPan.NoValue)
    DryRightPan=gval.dryrightpan;
  if (gval.filtertype!=paraFilterType.NoValue)
    FilterType=gval.filtertype;
  if (gval.cutoff!=paraCutoff.NoValue)
    Cutoff=gval.cutoff;
  if (gval.resonance!=paraResonance.NoValue)
    Resonance=gval.resonance;
  for (int c = 0; c < numTracks; c++)
    TickTrack(&Tracks[c], &tval[c]);
}



void mi::MDKInit(CMachineDataInput * const pi)
{
	numTracks = 1;

	for (int c = 0; c < MAX_TAPS; c++)
	{
	}

  for (int c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;

  SetOutputMode(true);
}

/*
void mi::AttributesChanged()
{
	MaxDelay = (int)(pMasterInfo->SamplesPerSec * (aval.maxdelay / 1000.0));
	for (int c = 0; c < numTracks; c++)
		InitTrack(c);
}
    */


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


static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
}


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
}

void mi::PrepareTrack(int tno)
{
}

int nEmptySamples=0;

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	return false;
}

void mi::WorkTrackStereo(CTrack *trk, float *pin, float *pout, int numsamples, int const mode)
{
  float *pData=Buffer;
  int nPos=Pos&DELAY_MASK;

  float DryLeftLeftAmp=(float)(DryOut*sqrt(1-DryLeftPan/240.0));
  float DryLeftRightAmp=(float)(DryOut*sqrt(DryLeftPan/240.0));
  float DryRightLeftAmp=(float)(DryOut*sqrt(1-DryRightPan/240.0));
  float DryRightRightAmp=(float)(DryOut*sqrt(DryRightPan/240.0));

  float WetLeftLeftAmp=(float)(trk->WetAmp*sqrt(1-trk->leftpanpos/240.0));
  float WetLeftRightAmp=(float)(trk->WetAmp*sqrt(trk->leftpanpos/240.0));
  float WetRightLeftAmp=(float)(trk->WetAmp*sqrt(1-trk->rightpanpos/240.0));
  float WetRightRightAmp=(float)(trk->WetAmp*sqrt(trk->rightpanpos/240.0));

/*
  float FeedbackLeftLeftAmp=(float)(trk->Feedback*sqrt(1-trk->fblpanpos/240.0));
  float FeedbackLeftRightAmp=(float)(trk->Feedback*sqrt(trk->fblpanpos/240.0));
  float FeedbackRightLeftAmp=(float)(trk->Feedback*sqrt(1-trk->fbrpanpos/240.0));
  float FeedbackRightRightAmp=(float)(trk->Feedback*sqrt(trk->fbrpanpos/240.0));
  */
  float FeedbackAmp=(float)trk->Feedback*FeedbackLimiter;
  
  bool first=trk==Tracks;
  
  float FBThis=float(FeedbackAmp*(1-fabs((120-trk->spread)/240.0)));
  float FBReverse=float(FeedbackAmp*(120-trk->spread)/240.0);
  
  for (int i=0; i<2*numsamples; i+=2)
  {
    float WetLeftValue=Buffer[(nPos-trk->LeftDelaySamples)&DELAY_MASK];
    float WetRightValue=Buffer[1+((nPos-trk->RightDelaySamples)&DELAY_MASK)];
		if (fabs(WetLeftValue)<1) WetLeftValue=0.0f;
		if (fabs(WetRightValue)<1) WetRightValue=0.0f;

    switch(trk->pingpong+2*!first)
    {
    case 3:
      pout[i]+=WetLeftValue*WetLeftLeftAmp+WetRightValue*WetRightLeftAmp,
      pout[i+1]+=WetLeftValue*WetLeftRightAmp+WetRightValue*WetRightRightAmp,
      Buffer[nPos]+=FBThis*WetRightValue+FBReverse*WetLeftValue,
      Buffer[nPos+1]+=FBThis*WetLeftValue+FBReverse*WetRightValue;
      break;
    case 2:
      pout[i]+=pin[i]*DryLeftLeftAmp+pin[i+1]*DryRightLeftAmp+WetLeftValue*WetLeftLeftAmp+WetRightValue*WetRightLeftAmp,
      pout[i+1]+=pin[i]*DryLeftRightAmp+pin[i+1]*DryRightRightAmp+WetLeftValue*WetLeftRightAmp+WetRightValue*WetRightRightAmp,
      Buffer[nPos]+=FBReverse*WetRightValue+FBThis*WetLeftValue,
      Buffer[nPos+1]+=FBReverse*WetLeftValue+FBThis*WetRightValue;
      break;
    case 1:
      pout[i]=pin[i]*DryLeftLeftAmp+pin[i+1]*DryRightLeftAmp+WetLeftValue*WetLeftLeftAmp+WetRightValue*WetRightLeftAmp,
      pout[i+1]=pin[i]*DryLeftRightAmp+pin[i+1]*DryRightRightAmp+WetLeftValue*WetLeftRightAmp+WetRightValue*WetRightRightAmp,
      Buffer[nPos]=pin[i]+FBThis*WetRightValue+FBReverse*WetLeftValue,
      Buffer[nPos+1]=pin[i+1]+FBThis*WetLeftValue+FBReverse*WetRightValue;
      break;
    case 0:
      pout[i]=pin[i]*DryLeftLeftAmp+pin[i+1]*DryRightLeftAmp+WetLeftValue*WetLeftLeftAmp+WetRightValue*WetRightLeftAmp,
      pout[i+1]=pin[i]*DryLeftRightAmp+pin[i+1]*DryRightRightAmp+WetLeftValue*WetLeftRightAmp+WetRightValue*WetRightRightAmp,
      Buffer[nPos]=pin[i]+FBReverse*WetRightValue+FBThis*WetLeftValue,
      Buffer[nPos+1]=pin[i+1]+FBReverse*WetLeftValue+FBThis*WetRightValue;
      break;
    }
    /*
    if (first)
      pout[i]=pin[i]*DryLeftLeftAmp+pin[i+1]*DryRightLeftAmp+WetLeftValue,
      pout[i+1]=pin[i]*DryLeftRightAmp+pin[i+1]*DryRightRightAmp+WetRightValue,
      Buffer[nPos]=pin[i]*WetLeftLeftAmp+pin[i+1]*WetRightLeftAmp+FeedbackLeftLeftAmp*WetLeftValue+FeedbackRightLeftAmp*WetRightValue,
      Buffer[nPos+1]=pin[i]*WetLeftRightAmp+pin[i+1]*WetRightRightAmp+FeedbackLeftRightAmp*WetLeftValue+FeedbackRightRightAmp*WetRightValue;
    else
      pout[i]+=WetLeftValue*WetLeftLeftAmp+WetRightValue*WetRightLeftAmp,
      pout[i+1]+=WetLeftValue*WetLeftRightAmp+WetRightValue*WetRightRightAmp,
      Buffer[nPos]+=FeedbackLeftLeftAmp*WetLeftValue+FeedbackRightLeftAmp*WetLeftValue,
      Buffer[nPos+1]+=FeedbackLeftRightAmp*WetLeftValue+FeedbackRightRightAmp*WetRightValue;
      */

    nPos=(nPos+2)&DELAY_MASK;
  }
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
  float Fb=0;
  int nMaxDelay=1;
  for (int i=0; i<numTracks; i++)
  {
    Fb+=(float)fabs(Tracks[i].Feedback);
    nMaxDelay=__max(Tracks[i].LeftDelaySamples,nMaxDelay);
    nMaxDelay=__max(Tracks[i].RightDelaySamples,nMaxDelay);
  }
  FeedbackLimiter=(float)((Fb>0.9999)?(0.9999/Fb):0.9999);
  //FeedbackLimiter=1.0;
	if (mode & WM_READ)
  {
		// memcpy(paux, psamples, numsamples*4);
    nEmptySamples=0;
  }
  else
  {
    //if (Fb>0.98f) Fb=0.98f;
    if (nEmptySamples>512 && pow(Fb*FeedbackLimiter,nEmptySamples/float(nMaxDelay))<(4.0/32767))
      return false;
    for (int i=0; i<2*numsamples; i++)
      psamples[i]=0.0;
    nEmptySamples+=numsamples;
  }

	float *paux = pCB->GetAuxBuffer();

/*
  if (!isstereo)
  {
    for (int i=numsamples-1; i>=0; i--)
      psamples[2*i]=psamples[i],
      psamples[2*i+1]=psamples[i];
  }
  */

  int so=0, maxs=96;

  if (numTracks>1)
  {
    for (int i=0; i<numTracks; i++)
    {
      if (f2i(Tracks[i].LeftDelaySamples)<maxs)
        maxs=f2i(Tracks[i].LeftDelaySamples);
      if (f2i(Tracks[i].RightDelaySamples)<maxs)
        maxs=f2i(Tracks[i].RightDelaySamples);
    }
  }

  if (!aval.lfoshape)
    for (int i=0; i<numTracks; i++)
      PrepareTrack(i);

  Pos&=DELAY_MASK;
  while(so<numsamples)
  {
    int end=__min(so+maxs,numsamples);
    for (int c = 0; c < numTracks; c++)
  		WorkTrackStereo(Tracks + c, psamples+so*2, paux+so*2, end-so, mode);
    if (FilterType)
    {
			m_limL.AvoidExceptions();
			m_filterL.AvoidExceptions();
			m_filterR.AvoidExceptions();
      float CF=440*pow(8000.0/440.0,Cutoff/240.0);
      float Q=0.01+0.99*Resonance/240.0;
      switch(FilterType)
      {
        case 1:
          m_filterL.rbjLPF(CF,Q,44100);
          m_filterR.rbjLPF(CF,Q,44100);
          break;
        case 2:
          m_filterL.rbjHPF(CF,Q,44100);
          m_filterR.rbjHPF(CF,Q,44100);
          break;
        case 3:
          m_filterL.rbjBPF(CF,Q,44100);
          m_filterR.rbjBPF(CF,Q,44100);
          break;
      }
      for (int i=2*so; i<2*end; i+=2)
      {
        float &ValL=Buffer[(Pos+i-2*so)&DELAY_MASK];
        float &ValR=Buffer[1+((Pos+i-2*so)&DELAY_MASK)];
        ValL=m_filterL.ProcessSample(ValL);
        ValR=m_filterL.ProcessSample(ValR);
        m_limL.ProcessSample(0.5f*(ValL+ValR));
        ValL*=m_limL.m_fFactor;
        ValR*=m_limL.m_fFactor;
      }
    }

    Pos=(Pos+2*(end-so))&DELAY_MASK;
    so=end;
  }
  if (!(mode&WM_WRITE))
    return false;
  memcpy(psamples,paux,numsamples*8);
  int *pint=(int *)paux;
  for (int i=0; i<2*numsamples; i++)
    if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
      return true;
	return false;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM PanzerDelay version 0.6 buzztard edition\nWritten by Krzysztof Foltman (kf@cw.pl), Gfx by Oom\nSpecial thx to: oskari [esp. for FP tricks!], canc3r, Oom, Zephod, Thev, HymaX, cmicali and all #buzz crew\n\n\n");
}

