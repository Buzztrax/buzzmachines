// FSM ScrapMan

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "..\MachineInterface_Old.h"
#include "..\WahMan3\DSPChips.h"

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		1
// 200 ms przy 44100 Hz
#define MAX_DELAY    65536
#define DELAY_MASK   65535
#define GRANULE_SIZE 4096
#define MAX_GRANULES 24

#define DELAY_MAX 20000
#define OFFSET_MAX 10000

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

CMachineParameter const paraFeedback = 
{ 
	pt_byte,									// type
	"Feedback",
	"Feedback",// description
	0,												// MinValue	
	99,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	0,                       // default
};

CMachineParameter const paraOctaviation = 
{ 
	pt_byte,									// type
	"Fullness",
	"Fullness",// description
	0,												// MinValue	
	100,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	0,                       // default
};

CMachineParameter const paraRichness = 
{ 
	pt_byte,									// type
	"Richness",
	"Richness", 		// description
	4,												// MinValue	
	MAX_GRANULES,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	12,                       // default
};

CMachineParameter const paraDensity = 
{ 
	pt_byte,									// type
	"Density",
	"Density",// description
	0,												// MinValue	
	64,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	48,                       // default
};

CMachineParameter const paraSpaceyness = 
{ 
	pt_byte,									// type
	"Scattering",
	"Scattering",// description
	0,												// MinValue	
	64,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	48,                       // default
};

CMachineParameter const paraAttack = 
{ 
	pt_byte,									// type
	"Attack",
	"Attack",// description
	0,												// MinValue	
	240,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	120,                       // default
};

CMachineParameter const paraSustain = 
{ 
	pt_byte,									// type
	"Sustain",
	"Sustain",// description
	0,												// MinValue	
	240,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	32,                       // default
};

CMachineParameter const paraRelease = 
{ 
	pt_byte,									// type
	"Release",
	"Release",// description
	0,												// MinValue	
	240,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	64,                       // default
};

CMachineParameter const paraFatness = 
{ 
	pt_byte,									// type
	"Fatness",
	"Fatness",// description
	0,												// MinValue	
	64,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	16,                       // default
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

CMachineParameter const paraDummy = 
{ 
	pt_byte,										// type
	"Dummy",
	"Dummy",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	0,										// Flags
	240
};

CMachineParameter const *pParameters[] = 
{ 
	&paraDryOut,
	&paraFeedback,
	&paraOctaviation,
	&paraRichness,
	&paraDensity,
	&paraSpaceyness,
	&paraFatness,
	&paraAttack,
	&paraSustain,
	&paraRelease,
	&paraWetOut,
	&paraDummy,
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
	byte feedback;
	byte octaviation;
  byte richness;
  byte density;
  byte spaceyness;
  byte fatness;
  byte attack;
  byte sustain;
  byte release;
  byte wetout;
};

class tvals
{
public:
	byte dummy;
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
	11,										// numGlobalParameters
	1,										// numTrackParameters
	pParameters,
	0,                    // 1 (numAttributes)
	NULL,                 // pAttributes
#ifdef _DEBUG
	"FSM ScrapMan (Debug build)",			// name
#else
	"FSM ScrapMan",
#endif
	"ScrapMan",								// short name
	"Krzysztof Foltman",						// author
	"A&bout"
};

class CGranule
{
public:
  int Offset;
  int EnvPoint;
  int Delay;
  int Phase;
  float RunningDetune;
  float DetuneFactor;
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

	virtual void SetNumTracks(int const n);

	virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);
  virtual void Command(int const i);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void WorkTrack(float *pin, float *pout, int numsamples, int const mode);

public:
  float *Buffer;
  int Pos;
  float DryOut;
	int numTracks;
  float Rise[2*GRANULE_SIZE];
  float Fall[2*GRANULE_SIZE];
	float Feedback;
	float Octaviation;
	float Limiter;
	CBiquad Biquad;

  int Richness, Density, Spaceyness, Fatness;
  int Attack, Sustain, Release;
  float WetOut;
  int Transpose,Finetune;
  int Overlap;

  CGranule Granules[MAX_GRANULES];
  
  float Phase;

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
  for (int i=0; i<GRANULE_SIZE; i++)
    Rise[i]=(float)(sin(i*PI/(2*GRANULE_SIZE))),
    Fall[i]=(float)(cos(i*PI/(2*GRANULE_SIZE)));
  for (i=0; i<GRANULE_SIZE; i++)
    Rise[GRANULE_SIZE+i]=1.0f,
    Fall[GRANULE_SIZE+i]=0.0f;
	Limiter=1.0f;
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
  case 10:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
  case 1:
		sprintf(txt, "%5.2f %%", 100*pow(value/100.0,0.5));
		break;
	case 2:
		sprintf(txt, "%d %%", value);
		break;
    /*
	case 1:   // min/delta delay
		sprintf(txt, "%4.1f ms", (double)((100+2*value)) );
		break;
	case 2:		// transpose
		sprintf(txt, "%d #", value-24);
		break;
	case 3:		// finetune
		sprintf(txt, "%d ct", value-100);
		break;
    */
	default:
		return NULL;
	}

	return txt;
}

#undef HANDLE_PARAM
#define HANDLE_PARAM(ptvalName, paraName) if (gval.ptvalName != para##paraName.NoValue) paraName = gval.ptvalName;

void mi::Init(CMachineDataInput * const pi)
{
	numTracks = 1;

  for (int c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;

	Phase = 0;
  for (int j=0; j<MAX_GRANULES; j++)
  {
    Granules[j].EnvPoint=rand()%GRANULE_SIZE;
    Granules[j].Delay=rand()%DELAY_MAX;
    Granules[j].Offset=rand()%OFFSET_MAX;
    Granules[j].Phase=3;
  }
}

void mi::AttributesChanged()
{
	InitTrack(0);
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
  InitTrack(i);
}


void mi::Tick()
{
	if (gval.feedback!=paraFeedback.NoValue)
	{
		if (!gval.feedback)
			Feedback=0.0;
		else
			Feedback=(float)pow(gval.feedback/100.0,0.5);
	}
  if (gval.dryout!=paraDryOut.NoValue)
  {
    if (gval.dryout)
      DryOut=(float)pow(2.0,(gval.dryout/10.0-24.0)/6.0);
    else
      DryOut=0.0f;
  }
	HANDLE_PARAM(richness,Richness)
  HANDLE_PARAM(octaviation,Octaviation)
  HANDLE_PARAM(density,Density)
  HANDLE_PARAM(spaceyness,Spaceyness)
  HANDLE_PARAM(fatness,Fatness)
  HANDLE_PARAM(attack,Attack)
  HANDLE_PARAM(sustain,Sustain)
  HANDLE_PARAM(release,Release)
  if (gval.wetout != paraWetOut.NoValue)
    WetOut = gval.wetout?(float)pow(2.0,(gval.wetout/10.0-24.0)/6.0):(float)0.0;
}

#pragma optimize ("a", on) 

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

inline int f2i(double d)
{
	const double magic = 6755399441055744.0; // 2^51 + 2^52
	double tmp = (d-0.5) + magic;
	return *(int*) &tmp;
}


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

static void DoWork(float *pin, float *pout, mi *pmi, int c)
{
  float *pData=pmi->Buffer;
  for (int g=0; g<pmi->Richness; g++)
  {
    CGranule *pGran=pmi->Granules+g;
    int c1=0;
    while(c1<c)
    {
      int DelayMax=int(4+pow(DELAY_MAX,0.3+0.7*(64-pmi->Density)/64.0));
      int depAttack=1024*(260-pmi->Attack)/240;
      int depSustain=1024*(260-pmi->Sustain)/240;
      int depRelease=1024*(260-pmi->Release)/240;

      if (pGran->EnvPoint>=256*GRANULE_SIZE/* && (rand()&7)==0*/)
      {
        pGran->EnvPoint=0;
        pGran->Phase++;
        if (pGran->Phase>=3)
        {
	        int EnvLength=GRANULE_SIZE*256/(depAttack+depSustain+depRelease)+DELAY_MAX;
          pGran->Offset=80+rand()%int(256+pow(OFFSET_MAX,0.5+0.5*pmi->Spaceyness/64.0));
          pGran->Delay=rand()%DelayMax;
          pGran->Phase=0;
          pGran->DetuneFactor=float(((rand()&1)?-1:+1)*(rand()%(1+10*pmi->Fatness))*0.00003);
          if (pGran->DetuneFactor<0) pGran->Offset-=(int)(15000*pGran->DetuneFactor);
					if (pmi->Octaviation<50)
					{
						if ((rand()&100)<pmi->Octaviation)
							pGran->DetuneFactor+=0.5;
					}
					else
					{
						int nRand=rand()&100;
						if (nRand<pmi->Octaviation/3)
							pGran->DetuneFactor+=0.5;
						else
						if (nRand<pmi->Octaviation*2/3)
							pGran->DetuneFactor+=0.75;
					}
						//pGran->Offset+=EnvLength;
          pGran->RunningDetune=0;
          //pGran->DetuneFactor=0.001;
        }
        //pGran->Offset=200;
      }
      int nPos=pmi->Pos;
      int dep=256;
      float *pEnv=NULL;

      if (pGran->Phase==0) dep=depAttack, pEnv=pmi->Rise; // attack
      if (pGran->Phase==1) dep=depSustain; // sustain
      if (pGran->Phase==2) dep=depRelease, pEnv=pmi->Fall; // release

      int c2=__min(c,c1+((__max(256*GRANULE_SIZE-pGran->EnvPoint,0)+dep-1)/dep)+DelayMax);
      if (pGran->Delay)
      {
        int nSkipped=__min(c-c1,pGran->Delay);
        c1+=nSkipped;
        pGran->Delay-=nSkipped;
      }
      if (c1<c2)
      {
        int ep=pGran->EnvPoint;
        int EnvLength=GRANULE_SIZE*256/(depAttack+depSustain+depRelease)+DELAY_MAX;
        float Amp=float((pmi->Feedback?1.0:pmi->WetOut)*EnvLength/((EnvLength+DelayMax/2)*sqrt(pmi->Richness)));
        if (pGran->Phase==3) // no-op
          ep+=dep*(c2-c1);
        else if (pGran->Phase==1) // sustain
        {
          for (int i=c1; i<c2; i++)
          {
            int intDet=f2i(pGran->RunningDetune);
            float fltDet=pGran->RunningDetune-intDet;
            int nPos=(pmi->Pos-pGran->Offset+i-intDet);
            pout[i]+=Amp*INTERPOLATE(1-fltDet,pData[(nPos-1)&DELAY_MASK],pData[nPos&DELAY_MASK]);
            pGran->RunningDetune+=pGran->DetuneFactor;
          }
          ep+=dep*__max(c2-c1,0);
        }
        else
        {
          for (int i=c1; i<c2; i++)
          {
            int intDet=f2i(pGran->RunningDetune);
            float fltDet=pGran->RunningDetune-intDet;
            int nPos=(pmi->Pos-pGran->Offset+i-intDet);

            pout[i]+=Amp*INTERPOLATE(1-fltDet,pData[(nPos-1)&DELAY_MASK],pData[nPos&DELAY_MASK])*pEnv[ep>>8];
//            pout[i]+=Amp*pData[nPos&DELAY_MASK]*pEnv[ep>>8];
            ep+=dep;
            pGran->RunningDetune+=pGran->DetuneFactor;
						if (ep>GRANULE_SIZE*256)
							ep=GRANULE_SIZE*256;
          }
        }
        pGran->EnvPoint=ep;
      }
      c1=c2;
    }
  }
}


#pragma optimize ("", on)


void mi::WorkTrack(float *pin, float *pout, int numsamples, int const mode)
{
  DoWork(pin,pout,this,numsamples);
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
	//Biquad.rbjBPF(1000,0.05,44100);
	Biquad.SetLowShelf(100,1,0.1,44100);

  int so=0, maxs=64;

  while(so<numsamples)
  {
    int end=__min(so+maxs,numsamples);
    int c;

		if (!Feedback)
			for (c=so; c<end; c++)
				Buffer[(Pos+c-so)&DELAY_MASK]=psamples[c],
				paux[c]=DryOut*psamples[c];
		else
			for (c=so; c<end; c++)
				Buffer[(Pos+c-so)&DELAY_MASK]=psamples[c],
				paux[c]=0.0;

    
  	WorkTrack(psamples+so, paux+so, end-so, mode);
		
		if (Feedback)
		{
	    for (c=so; c<end; c++)
			{
				float Smp=Biquad.ProcessSampleSafe(paux[c]*Feedback*Limiter);
				if (Smp>32000 || Smp<-32000) Limiter*=0.9f;
				if (Smp>-1000 && Smp<1000 && Limiter<1.0f)
				{
					Limiter*=1.01f;
					if (Limiter>1.0f)
						Limiter=1.0f;
				}
				Buffer[(Pos+c-so)&DELAY_MASK]+=Smp;
				paux[c]=paux[c]*WetOut+DryOut*psamples[c];
			}
		}
    
		Pos=(Pos+end-so)&DELAY_MASK;
    so=end;
  }

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
  pCB->MessageBox("FSM ScrapMan version 0.26a !\nWritten by Krzysztof Foltman (kf@cw.pl).\n"
		"Lot of thanks to canc3r for betatesting & ideas\n\n"
    "Visit my homepage at www.mp3.com/FSMachine\nand hear my songs (buzz-generated goa trance) ! :-)");
}

void mi::Stop()
{
  InitTrack(0);
}
