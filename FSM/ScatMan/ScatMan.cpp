// Zrobiæ obwiedniê do dŸwiêku dry przy prze³¹czaniu trybu hold !

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"

#define MAX_TAPS		1
// 200 ms przy 44100 Hz
#define MAX_DELAY    65536
#define DELAY_MASK   65535

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraSampleLen = 
{ 
	pt_byte,										// type
	"Loop time",
	"Loop time",					// description
	1,												  // MinValue	
	140,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraDirection = 
{ 
	pt_byte,									// type
	"Direction",
	"Direction (0-forward, 1-backward, 2-pingpong 3-pongping)", 		// description
	0,												// MinValue	
	3,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	0,                       // default
};

CMachineParameter const paraHold = 
{ 
	pt_switch,									// type
	"Hold",
	"Hold", 		// description
	0,												// MinValue	
	1,										  // MaxValue
	255,										  // NoValue
	0,								// Flags
	0,                       // default
};

CMachineParameter const paraAttack = 
{ 
	pt_byte,									// type
	"H Attack",
	"Hold phase Attack time", 		// description
	0,												// MinValue	
	180,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	0,                       // default
};

CMachineParameter const paraSustain = 
{ 
	pt_byte,									// type
	"H Sustain",
	"Hold phase Sustain time", 		// description
	0,												// MinValue	
	181,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	181,                       // default
};

CMachineParameter const paraRelease = 
{ 
	pt_byte,									// type
	"H Release",
	"Hold phase Release time", 		// description
	0,												// MinValue	
	180,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	0,                       // default
};

CMachineParameter const paraSmoothing = 
{ 
	pt_byte,									// type
	"Smoothing",
	"Smoothing", 		// description
	0,												// MinValue	
	100,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	20,                       // default
};


CMachineParameter const *pParameters[] = 
{ 
	&paraSampleLen,
  &paraDirection,
	&paraAttack,
	&paraSustain,
	&paraRelease,
	&paraSmoothing,
	&paraHold,
};

#pragma pack(1)

class gvals
{
public:
};

class tvals
{
public:
	byte looptime;
  byte direction;
	byte attack;
	byte sustain;
	byte release;
	byte smoothing;
  byte hold;
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
	0,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	0,										// numGlobalParameters
	7,										// numTrackParameters
	pParameters,
	0,                    // 1 (numAttributes)
	NULL,                 // pAttributes
#ifdef _DEBUG
	"FSM ScatMan (Debug build)",			// name
#else
	"FSM ScatMan",
#endif
	"ScatMan",								// short name
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

//	virtual void SetNumTracks(int const n);

	virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);
  virtual void Command(int const i);
	inline float GetWindow();

  virtual void Stop() { Mode=0; }

private:
//	void InitTrack(int const i);
//	void ResetTrack(int const i);

//	void TickTrack(CTrack *pt, tvals *ptval);
//	void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  float *Buffer;
  int Pos,PlayPos,Mode,LoopTime,LoopTimeParam,Direction,Delta;
	int AttackPar, SustainPar, ReleasePar, SmoothingPar, OverlapTime, OverlapPos;
	float SmoothPoint;

	CADSREnvelope ASR;

	inline int ParToTime(int Par);

public:
	avals aval;
	gvals gval;
	tvals tval;
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = &tval;
	AttrVals = (int *)&aval;
  Buffer = new float[MAX_DELAY];
  Mode = 0;
}

mi::~mi()
{
  delete Buffer;
}

#define LFOPAR2SAMPLE(value) (pow(2.0,(value-120)/30.0))

char const *mi::DescribeValue(int const param, int const value)
{
  static char mode[4][16]={"fwd","rev","pingpong","pongping"};
  static char txt[16];

	switch(param)
	{
  case 0:
    if (value<=100)
      sprintf(txt,"%0.2f ms",pow(value/100.0,1.4)*1000);
    else
      sprintf(txt,"%0.2f tick",(value-100)/10.0);
    break;
  case 1:
    strcpy(txt,mode[value]);
    break;
  case 2:
  case 3:
  case 4:
    if (value<=100)
      sprintf(txt,"%0.2f ms",pow(value/100.0,1.4)*1000);
    else
    if (value<=180)
      sprintf(txt,"%0.2f tick",(value-100)/5.0);
    else
			strcpy(txt,"+inf");
    break;
  case 5:
    sprintf(txt,"%d%%",value);
		break;
	default:
		return NULL;
	}

	return txt;
}

int mi::ParToTime(int Par)
{
	if (Par==181)
		return 99999999;
	if (Par<=100)
		return (int)(pow(Par/100.0,1.4)*44100);
	else
    return (Par-100)*pMasterInfo->SamplesPerTick/5;
}

void mi::Tick()
{
  if (tval.looptime!=255) LoopTimeParam=tval.looptime;
  if (tval.direction!=255) Direction=tval.direction;
  if (tval.attack!=255) AttackPar=tval.attack;
  if (tval.sustain!=255) SustainPar=tval.sustain;
  if (tval.release!=255) ReleasePar=tval.release;
  if (tval.smoothing!=255) SmoothingPar=tval.smoothing;

  if (tval.hold==0)
  {
    Mode=0;
    OverlapTime=10+SmoothingPar/4;
    OverlapPos=0;
  }
	SmoothPoint=(float)(LoopTime*0.5*SmoothingPar/100);

	ASR.m_nAttackTime=ParToTime(AttackPar);
	ASR.m_nDecayTime=10;
	ASR.m_fSustLevel=0.99999999f;
	ASR.m_nSustainTime=ParToTime(SustainPar);
	ASR.m_nReleaseTime=ParToTime(ReleasePar);

  if (LoopTimeParam<=100)
    LoopTime=(int)(pow(LoopTimeParam/100.0,1.4)*44100);
  else
    LoopTime=__min(65534,(LoopTimeParam-100)*pMasterInfo->SamplesPerTick/10);

  if (tval.hold==1)
  {
    Mode=1;
    PlayPos=0;
    Delta=1;
		ASR.NoteOn();
    if (Direction==1 || Direction==3)
      PlayPos=LoopTime-1,
      Delta=-1;
    OverlapTime=10+SmoothingPar/4;
    OverlapPos=0;
  }
}

void mi::Init(CMachineDataInput * const pi)
{
  for (int c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;
  Mode=0;
}

void mi::AttributesChanged()
{
/*
	MaxDelay = (int)(pMasterInfo->SamplesPerSec * (aval.maxdelay / 1000.0));
	for (int c = 0; c < numTracks; c++)
		InitTrack(c);
    */
}

int nEmptySamples=0;

inline float mi::GetWindow()
{
	if (PlayPos<SmoothPoint)
		return float(PlayPos/SmoothPoint);
	if ((LoopTime-PlayPos)<SmoothPoint)
		return float((LoopTime-PlayPos)/SmoothPoint);
	return 1.0f;
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
	//float *paux = pCB->GetAuxBuffer();

	if (mode & WM_READ)
  {
		// memcpy(paux, psamples, numsamples*4);
    nEmptySamples=0;
  }
  else
  {
    if (!Mode)
    {
      for (int i=0; i<numsamples; i++)
      {
        Buffer[Pos]=0.0f;
        Pos=(Pos+1)&DELAY_MASK;
      }
      return false;
    }
  }

	int c0=0;
	while(c0<numsamples)
	{
		int end=numsamples;

		if (Mode)
		{
			double I0=ASR.ProcessSample(0);
			double DI=ASR.m_fSeries;

/*
			if (ASR.GetTimeLeft()<=0)
			{
				char buf[128];
				sprintf(buf,"Phase=%d Time=%d StageTime=%d DecayTime=%d",ASR.m_nState,ASR.m_nTime,ASR.m_nStageTime);
				pCB->MessageBox(buf);
				Brk();
			}
			*/
			end=__min(end,c0+ASR.GetTimeLeft());
			
			if (Delta>0)
				end=__min(end,c0+LoopTime-PlayPos);
			if (Delta<0)
			{
				end=__min(end,c0+PlayPos+1);
			}
      if (OverlapPos<OverlapTime)
      {
        float OverlapDelta=float(1.0f/OverlapTime);
        float OverlapVol=float(1.0-float(OverlapPos)/OverlapTime);
			  for (int i=c0; i<end; i++)
			  {
//				  psamples[i]=float(psamples[i]*OverlapVol)+(float)I0*Buffer[(Pos-LoopTime+PlayPos)&DELAY_MASK]*GetWindow();
				  psamples[i]=float(psamples[i]*OverlapVol)+(float)I0*Buffer[(Pos-LoopTime+PlayPos)&DELAY_MASK]*(1-OverlapVol)*GetWindow();
				  I0*=DI;
				  PlayPos+=Delta;
          OverlapVol-=OverlapDelta;
          if (OverlapVol<0) OverlapVol=0;
			  }
        OverlapPos+=end-c0;
      }
      else
      {
			  for (int i=c0; i<end; i++)
			  {
				  psamples[i]=(float)I0*Buffer[(Pos-LoopTime+PlayPos)&DELAY_MASK]*GetWindow();
				  I0*=DI;
				  PlayPos+=Delta;
			  }
        OverlapPos+=end-c0;
      }
			if (PlayPos>=LoopTime)
			{
				if (Direction&2)
					Delta=-1, PlayPos=LoopTime-2;
				else
					PlayPos=0;
			}
			if (PlayPos<0) {
				if (Direction&2)
					Delta=+1, PlayPos=1;
				else
					PlayPos=LoopTime-1;
			}

			if (ASR.m_nState==3)
				Mode=0;

			ASR.ProcessSample(end-c0);
		}
		else
		{
      if (OverlapPos<OverlapTime)
      {
//        end=__min(end,OverlapTime-OverlapPos);
        float OverlapDelta=float(1.0f/OverlapTime);
        float OverlapVol=float(float(OverlapPos)/OverlapTime);
        for (int i=c0; i<end; i++)
			  {
				  Buffer[Pos]=psamples[i];
          psamples[i]*=OverlapVol;
				  Pos=(Pos+1)&DELAY_MASK;
          OverlapVol+=OverlapDelta;
          if (OverlapVol>1) OverlapVol=1;
			  }
        OverlapPos+=end-c0;
      }
      else
      {
        for (int i=c0; i<end; i++)
			  {
				  Buffer[Pos]=psamples[i];
				  Pos=(Pos+1)&DELAY_MASK;
			  }
        OverlapPos+=end-c0;
      }
		}

		c0=end;
	}

  int *pint=(int *)psamples;
  for (int i=0; i<numsamples; i++)
    if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
      return true;
	return false;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM ScatMan version 1.00, by Krzysztof Foltman (kf@onet.pl).\nSpecial thanx to canc3r for a lot of ideas.\n\n"
    "Visit my homepage at www.mp3.com/psytrance\nand hear my songs (buzz-generated goa trance) ! :-)");
}
