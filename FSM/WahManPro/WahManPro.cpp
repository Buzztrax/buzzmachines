
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"

#define MAX_TAPS		8
// 200 ms przy 44100 Hz

///////////////////////////////////////////////////////////////////////////////////

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

CMachineParameter const paraFilterType = 
{ 
	pt_byte,										// type
	"Filter",
	"Filter type",	// description
	0,												  // MinValue	
	4,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	1
};

CMachineParameter const paraLFODepth = 
{ 
	pt_byte,									// type
	"LFO mod",
	"LFO Modulation depth",// description
	0,												// MinValue	
	240,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	60,                       // default
};

CMachineParameter const paraLFORate = 
{ 
	pt_byte,										// type
	"LFO rate",
	"LFO rate [Hz]",				    // description
	0,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	30
};

CMachineParameter const paraLFOShape = 
{ 
	pt_byte,										// type
	"LFO Shape",
	"LFO Shape",	// description
	0,												  // MinValue	
	8,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	1
};

CMachineParameter const paraInertia = 
{ 
	pt_byte,										// type
	"Inertia",
	"Cutoff frequency inertia",	// description
	0,												  // MinValue	
	60,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	10
};

CMachineParameter const paraLFOPhase = 
{ 
	pt_byte,										// type
	"Set phase",
	"Set LFO phase",				    // description
	0,												  // MinValue	
	127,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	255
};



CMachineParameter const *pParameters[] = 
{ 
	&paraCutoff,
	&paraResonance,
  &paraFilterType,
	&paraLFORate,
	&paraLFODepth,
  &paraLFOShape,
	&paraInertia,
	&paraLFOPhase,
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
  byte filtype;
  byte lforate;
  byte lfodepth;
  byte lfoshape;
  byte inertia;
  byte lfophase;
};

class avals
{
public:
	int thevfactor;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	1,    								// max tracks
	0,										// numGlobalParameters
	8,										// numTrackParameters
	pParameters,
	1,                    // 1 (numAttributes)
	pAttributes,                 // pAttributes
#ifdef _DEBUG
	"FSM WahMan Pro (Debug build)",			// name
#else
	"FSM WahMan Pro",
#endif
	"WahPro",								// short name
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
  float Cutoff;
  float Resonance;
  float LFORate;
  float LFODepth;
  float Inertia;
  double LFOPhase;
  double DeltaPhase;

  float CurCutoff;

  CBiquad m_filter, m_filter2;
  int FilterType;
  int LFOShape;
  float ThevFactor;

  void CalcCoeffs1();
  void CalcCoeffs2();
  void CalcCoeffs3();
  void CalcCoeffs4();
  void CalcCoeffs5();
  void ResetFilter() { m_filter.Reset(); m_filter2.Reset(); }
};
 

void CTrack::CalcCoeffs1()
{
	int sr=44100;

  float CutoffFreq=(float)(264*pow(32,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=(float)pow(cf/20000.0,0.5);
  // float ScaleResonance=1.0;
  float fQ=(float)(1.01+5*Resonance*ScaleResonance/240.0);

  float fB=(float)sqrt(fQ*fQ-1)/fQ;
	float fA=(float)(2*fB*(1-fB));

  float A,B;

	float ncf=(float)(1.0/tan(M_PI*cf/(double)sr));
	A=fA*ncf;      // denormalizacja i uwzgl�dnienie cz�stotliwo�ci pr�bkowania
	B=fB*ncf*ncf;
  float a0=float(1/(1+A+B));
	m_filter.m_b1=2*(m_filter.m_b2=m_filter.m_b0=a0);// obliczenie wsp�czynnik�w filtru cyfrowego (przekszta�cenie dwuliniowe)
	m_filter.m_a1=a0*(2-B-B);
	m_filter.m_a2=a0*(1-A+B);
}

void CTrack::CalcCoeffs2()
{
  float CutoffFreq=(float)(264*pow(32,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=8000) cf=8000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,0.5);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  float fQ=(float)(1.01+30*Resonance*ScaleResonance/240.0);

  m_filter.SetParametricEQ(CutoffFreq,10,fQ,44100);
}

void CTrack::CalcCoeffs3()
{
  float CutoffFreq=(float)(264*pow(32,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=8000) cf=8000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,0.5);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  //float fQ=(float)(1.01+30*Resonance*ScaleResonance/240.0);

  m_filter.SetParametricEQ(CutoffFreq,float(1.0+Resonance*ScaleResonance/6.0),32,44100);
}

void CTrack::CalcCoeffs4()
{
	int sr=44100;

  float CutoffFreq=(float)(264*pow(32,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,0.5);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  float fQ=(float)sqrt(1.01+5*Resonance*ScaleResonance/240.0);

  float fB=(float)sqrt(fQ*fQ-1)/fQ;
	float fA=(float)(2*fB*(1-fB));

  float A,B;

	float ncf=(float)(1.0/tan(M_PI*cf/(double)sr));
	A=fA*ncf;      // denormalizacja i uwzgl�dnienie cz�stotliwo�ci pr�bkowania
	B=fB*ncf*ncf;
  float a0=float(1/(1+A+B));
	m_filter.m_b1=2*(m_filter.m_b2=m_filter.m_b0=a0);// obliczenie wsp�czynnik�w filtru cyfrowego (przekszta�cenie dwuliniowe)
	m_filter.m_a1=a0*(2-B-B);
	m_filter.m_a2=a0*(1-A+B);

  cf=(float)(cf*(1.0-0.3*ThevFactor));
  ncf=(float)(1.0/tan(M_PI*(cf*0.7)/(double)sr));
	A=fA*ncf;      // denormalizacja i uwzgl�dnienie cz�stotliwo�ci pr�bkowania
	B=fB*ncf*ncf;
  a0=float(1/(1+A+B));
	m_filter2.m_b1=2*(m_filter2.m_b2=m_filter2.m_b0=a0);// obliczenie wsp�czynnik�w filtru cyfrowego (przekszta�cenie dwuliniowe)
	m_filter2.m_a1=a0*(2-B-B);
	m_filter2.m_a2=a0*(1-A+B);
}

void CTrack::CalcCoeffs5()
{
  float CutoffFreq=(float)(264*pow(32,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=8000) cf=8000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,0.5);
  //float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  //float fQ=(float)(1.01+30*Resonance*ScaleResonance/240.0);

  m_filter.SetParametricEQ(CutoffFreq,(float)(1.0+Resonance/12.0),float(6+Resonance/30.0),44100);
  m_filter2.SetParametricEQ(float(CutoffFreq/(1+Resonance/240.0)),float(1.0+Resonance/12.0),float(6+Resonance/30.0),44100);
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
  virtual void Command(int const i);

	virtual char const *DescribeValue(int const param, int const value);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  int Pos;
  float DryOut;
	avals aval;

private:
	int numTracks;
	CTrack Tracks[MAX_TAPS];

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
/*  case 0:
  case 4:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
  case 5:
    sprintf(txt, "%4.1f %%", (double)(value*100.0/64.0-100.0) );
    break;
	case 1:   // min/delta delay
  case 2:
		sprintf(txt, "%4.1f ms", (double)(value/10.0) );
		break;
    */
  case 2:
    if (value==0) strcpy(txt,"2pole LP");
    if (value==1) strcpy(txt,"2pole Peak I");
    if (value==2) strcpy(txt,"2pole Peak II");
    if (value==3) strcpy(txt,"4pole LP");
    if (value==4) strcpy(txt,"4pole Peak");
    break;
  case 3:		// LFO rate
		sprintf(txt, "%5.3f Hz", (double)LFOPAR2TIME(value));
		break;
  case 5: // LFO shape
    if (value==0) strcpy(txt,"sine");
    if (value==1) strcpy(txt,"saw up");
    if (value==2) strcpy(txt,"saw down");
    if (value==3) strcpy(txt,"square");
    if (value==4) strcpy(txt,"triangle");
    if (value==5) strcpy(txt,"weird 1");
    if (value==6) strcpy(txt,"weird 2");
    if (value==7) strcpy(txt,"weird 3");
    if (value==8) strcpy(txt,"weird 4");
    break;
	default:
		return NULL;
	}

	return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->lforate != paraLFORate.NoValue)
		pt->DeltaPhase = (float)(2*M_PI*LFOPAR2TIME(ptval->lforate)/pMasterInfo->SamplesPerSec);
	if (ptval->lfophase != paraLFOPhase.NoValue)
		pt->LFOPhase = (float)(2*M_PI*ptval->lfophase/128.0);
	if (ptval->lfodepth!= paraLFODepth.NoValue)
		pt->LFODepth = (float)(ptval->lfodepth);
	if (ptval->inertia!= paraInertia.NoValue)
		pt->Inertia = (float)(ptval->inertia/240.0);
	if (ptval->cutoff!= paraCutoff.NoValue)
		pt->Cutoff = ptval->cutoff;
	if (ptval->resonance!= paraResonance.NoValue)
		pt->Resonance = (float)(ptval->resonance);
  if (ptval->lfoshape!=paraLFOShape.NoValue)
    pt->LFOShape = ptval->lfoshape;
  if (ptval->filtype!=paraFilterType.NoValue)
  {
    if (pt->FilterType != ptval->filtype)
      pt->ResetFilter();
    pt->FilterType = ptval->filtype;
  }
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
    vals.lfodepth=paraLFODepth.DefValue;
    vals.lfophase=paraLFOPhase.DefValue;
    vals.lfoshape=paraLFOShape.DefValue;
    vals.inertia=paraInertia.DefValue;
    vals.filtype=paraFilterType.DefValue;
    TickTrack(&Tracks[c], &vals);
    Tracks[c].ResetFilter();
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
	Tracks[i].ResetFilter();
}

void mi::ResetTrack(int const i)
{
}


void mi::Tick()
{
	for (int c = 0; c < numTracks; c++)
		TickTrack(&Tracks[c], &tval[c]);
}

#pragma optimize ("a", on) 

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
  float ai=(float)(10*exp(-trk->Inertia*9.0));
  for (int i=0; i<c; i+=64) // pow(2,CurCutoff/48.0)
  {
    float LFO=0.0;
    float Phs=(float)fmod(trk->LFOPhase,(float)(2*PI));
    trk->ThevFactor=(float)(pmi->aval.thevfactor/20.0);
    switch(trk->LFOShape)
    {
      case 0: LFO=(float)sin(Phs); break;
      case 1: LFO=(float)(((Phs-PI)/PI-0.5f)*2.0f); break;
      case 2: LFO=(float)(((Phs-PI)/PI-0.5f)*-2.0f); break;
      case 3: LFO=(Phs<PI)?1.0f:-1.0f; break;
      case 4: LFO=(float)(((Phs<PI?(Phs/PI):(2.0-Phs/PI))-0.5)*2); break;
      case 5: LFO=(float)sin(trk->LFOPhase+PI/4*sin(trk->LFOPhase)); break;
      case 6: LFO=(float)sin(trk->LFOPhase+PI/6*sin(2*trk->LFOPhase)); break;
      case 7: LFO=(float)sin(2*trk->LFOPhase+PI*cos(3*trk->LFOPhase)); break;
      case 8: LFO=(float)(0.5*sin(2*trk->LFOPhase)+0.5*cos(3*trk->LFOPhase)); break;
    }
    float DestCutoff=(float)(trk->Cutoff+trk->LFODepth*LFO/2); // pow(2.0,trk->LFODepth*sin(trk->LFOPhase)/100.0);
    if (fabs(trk->CurCutoff-DestCutoff)<ai)
      trk->CurCutoff=DestCutoff;
    else
      trk->CurCutoff+=(float)_copysign(ai,DestCutoff-trk->CurCutoff);
    if (trk->FilterType==0) trk->CalcCoeffs1();
    if (trk->FilterType==1) trk->CalcCoeffs2();
    if (trk->FilterType==2) trk->CalcCoeffs3();
    if (trk->FilterType==3) trk->CalcCoeffs4();
    if (trk->FilterType==4) trk->CalcCoeffs5();

    int jmax=__min(i+64,c);
    if (trk->FilterType<3)
    {
      for (int j=i; j<jmax; j++)
      {
        pout[j]=trk->m_filter.ProcessSample(pin[j]);
      }
    }
    else
    {
      for (int j=i; j<jmax; j++)
      {
        pout[j]=trk->m_filter2.ProcessSample(trk->m_filter.ProcessSample(pin[j]));
      }
    }
    trk->LFOPhase+=(jmax-i)*trk->DeltaPhase;
  }
}

/*
static void DoWorkNoInput(float *pout, CMachineInterface *mi, int c, CTrack *trk)
{
	do
	{
		double delay = *pbuf;
		*pbuf++ = (float)(feedback * delay);
		*pout++ += (float)(delay * wetout);

	} while(--c);
}

static void DoWorkNoInputNoOutput(CMachineInterface *mi, int c, CTrack *trk)
{
	do
	{
		*pbuf = (float)(*pbuf * feedback);
		pbuf++;
	} while(--c);
}

static void DoWorkNoOutput(float *pin, CMachineInterface *mi, int c, CTrack *trk)
{
	do
	{

		double delay = *pbuf;
		double dry = *pin++;
		*pbuf++ = (float)(feedback * delay + dry);

	} while(--c);
}
*/

#pragma optimize ("", on)


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
  DoWork(pin,pout,this,numsamples,pt);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
	float *paux = pCB->GetAuxBuffer();

	if (mode & WM_READ)
  {
    zeroSamples=0;
		memcpy(paux, psamples, numsamples*4);
  }
  else
  {
    CTrack *trk=Tracks;
    if (zeroSamples>1000 && (trk->FilterType<2 || (fabs(trk->m_filter2.m_y1)<1 && fabs(trk->m_filter2.m_y2)<1)) && fabs(trk->m_filter.m_y2)<1 && fabs(trk->m_filter.m_y1)<1)
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
  pCB->MessageBox("FSM WahManPro version 1.1 !\nWritten by Krzysztof Foltman (kf@cw.pl), Gfx by Oom\n\n"
    "For new songs, please don't use WahMan - FSM WahManPro2 is MUCH better !\n\n"
    "Visit my homepage at www.mp3.com/FSMachine\n(buzz-generated goa trance) and hear my songs ! :-)");
}

