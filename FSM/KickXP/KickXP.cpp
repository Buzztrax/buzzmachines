
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <windef.h>
#include <MachineInterface.h>

#define MAX_TAPS		8
// 200 ms przy 44100 Hz

static float thumpdata1[1024];

///////////////////////////////////////////////////////////////////////////////////

#pragma pack(push,4)

CMachineParameter const paraPitchLimit = 
{ 
	pt_note,										// type
	"Pitch limit",
	"Lower pitch limit",					            // description
	NOTE_MIN,												  // MinValue	
	NOTE_MAX,												  // MaxValue
	NOTE_NO,										// NoValue
	MPF_TICK_ON_EDIT,										// Flags // MPF_STATE
	NOTE_NO
};

CMachineParameter const paraTrigger = 
{ 
	pt_byte,										// type
	"Trigger",
	"Trigger/volume",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	0,										// Flags // MPF_STATE
	0
};

CMachineParameter const paraStartFrq = 
{ 
	pt_byte,										// type
	"Start",
	"Start frequency",					// description
	1,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	145
};

CMachineParameter const paraEndFrq = 
{ 
	pt_byte,										// type
	"End",
	"End frequency",					// description
	1,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	50
};

CMachineParameter const paraBuzzAmt = 
{ 
	pt_byte,										// type
	"Buzz",
	"Amount of Buzz",				    // description
	0,												  // MinValue	
	100,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	55
};

CMachineParameter const paraClickAmt = 
{ 
	pt_byte,										// type
	"Click",
	"Amount of Click",				    // description
	0,												  // MinValue	
	100,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	28
};

CMachineParameter const paraPunchAmt = 
{ 
	pt_byte,										// type
	"Punch",
	"Amount of Punch",				    // description
	0,												  // MinValue	
	100,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	47
};

CMachineParameter const paraToneDecay = 
{ 
	pt_byte,									// type
	"T DecRate",
	"Tone decay rate",// description
	1,												// MinValue	
	240,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	30,                       // default
};

CMachineParameter const paraToneShape = 
{ 
	pt_byte,										// type
	"T DecShape",
	"Tone decay shape",				    // description
	1,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	27
};

CMachineParameter const paraBDecay = 
{ 
	pt_byte,										// type
	"B DecRate",
	"Buzz decay rate",				    // description
	1,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	55
};

CMachineParameter const paraCDecay = 
{ 
	pt_byte,										// type
	"C+P DecRate",
	"Click+Punch decay rate",				    // description
	1,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	55
};

CMachineParameter const paraDecSlope = 
{ 
	pt_byte,										// type
	"A DecSlope",
	"Amplitude decay slope",				    // description
	1,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	1
};

CMachineParameter const paraDecTime = 
{ 
	pt_byte,										// type
	"A DecTime",
	"Amplitude decay time",				    // description
	1,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	32
};

CMachineParameter const paraRelSlope = 
{ 
	pt_byte,										// type
	"A RelSlope",
	"Amplitude release slope",				    // description
	1,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	105
};

CMachineParameter const paraNoteDelay = 
{ 
	pt_byte,										// type
	"NoteDelay",
	"0..5:Note Delay (N/6) | 6..A = retrig at (N-5)/6 | B/C = retrig each 1/6, 1/3",				    // description
	0,												  // MinValue	
	12,												// MaxValue
	255,									// NoValue
	0,									// Flags
	0
};

CMachineParameter const *pParameters[] = 
{ 
  &paraPitchLimit,
  &paraTrigger,
	&paraStartFrq,
	&paraEndFrq,
  &paraBuzzAmt,

  &paraClickAmt,
  &paraPunchAmt,
	&paraToneDecay,
	&paraToneShape,
	&paraBDecay,

	&paraCDecay,
	&paraDecSlope,
	&paraDecTime,
	&paraRelSlope,
	&paraNoteDelay,
};

#pragma pack(1)

class gvals
{
public:
};

class tvals
{
public:
  byte pitchlimit;
  byte volume;
  byte startfrq;
  byte endfrq;
  byte buzz;
  byte click;
  byte punch;
  byte tdecay;
  byte tshape;
  byte bdecay;
  byte cdecay;
  byte dslope;
  byte dtime;
  byte rslope;
  byte ndelay;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,								// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	0,										// numGlobalParameters
	15,										// numTrackParameters
	pParameters,
	0,                    // 1 (numAttributes)
	NULL,                 // pAttributes
#ifdef _DEBUG
	"FSM Kick XP (Debug build)",			// name
#else
	"FSM Kick XP",
#endif
	"KickXP",								// short name
	"Krzysztof Foltman",						// author
	"About"
};

class CTrack
{
public:
/*
  int Length;
	int Pos;
	int Unit;
  */
  float PitchLimit;
  float ThisPitchLimit;
  float StartFrq;
  float ThisStartFrq;
  float EndFrq;
  float ThisEndFrq;
  float TDecay;
  float ThisTDecay;
  float TShape;
  float ThisTShape;
  float DSlope;
  float ThisDSlope;
  float DTime;
  float ThisDTime;
  float RSlope;
  float ThisRSlope;
  float BDecay;
  float ThisBDecay;
  float CDecay;
  float ThisCDecay;
  float CurVolume;
  float ThisCurVolume;
  float LastValue;
  float AntiClick;
  float ClickAmt;
  float PunchAmt;
  float BuzzAmt;
  float Amp;
  float DecAmp;
  float BAmp;
  float MulBAmp;
  float CAmp;
  float MulCAmp;
  float Frequency;
  int SamplesToGo;
  int Retrig;
  int RetrigCount;

  double xSin, xCos, dxSin, dxCos;

  int EnvPhase;
  int LeftOver;
  int Age;
  double OscPhase;
};
 
class mi : public CMachineInterface
{
public:
	mi();
	~mi() {}
	virtual void Destructor();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);

	virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);
	virtual void Stop() {  }
	virtual void Command(int const i);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	bool WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  float *Buffer;
  int Pos;
  float DryOut;
	void Trigger(CTrack *pt);

public:
  short *thump1;
  int thump1len;

private:
	int numTracks;
	CTrack Tracks[MAX_TAPS];

	gvals gval;
	tvals tval[MAX_TAPS];
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = NULL;
}

void mi::Destructor()
{
	this->~mi();
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM Kick XP 1.0SP1\r\n\r\nIf you like this machine, consider supporting some people\r\n"
    "that are doing something more useful than coding Buzz machines\r\n(like FSF, Debian, Xiph, EFF, ACLU, Amnesty Intl etc)");
}

#define LFOPAR2TIME(value) (0.05*pow(800.0,value/255.0))

char const *mi::DescribeValue(int const param, int const value)
{
  static char txt[36];

  switch(param)
	{
	case 12:		// delay time
		sprintf(txt, "%0.2f s", (double)(value/240.0));
		break;
	default:
		return NULL;
	}

	return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
  bool bTrig=false;
	if (ptval->volume != paraTrigger.NoValue)
  {
    pt->CurVolume = (float)(ptval->volume*(32000.0/128.0));
    bTrig=true;
  }
	if (ptval->startfrq != paraStartFrq.NoValue)
		pt->StartFrq = (float)(33.0*pow(128,ptval->startfrq/240.0));
	if (ptval->endfrq != paraEndFrq.NoValue)
		pt->EndFrq = (float)(33.0*pow(16,ptval->endfrq/240.0));
	if (ptval->tdecay!= paraToneDecay.NoValue)
		pt->TDecay = (float)(ptval->tdecay/240.0)*((1.0/400.0)*(44100.0/pMasterInfo->SamplesPerSec));
	if (ptval->tshape!= paraToneShape.NoValue)
		pt->TShape = (float)(ptval->tshape/240.0);
	if (ptval->dslope!= paraDecSlope.NoValue)
		pt->DSlope = (float)pow(20,ptval->dslope/240.0-1)*25/pMasterInfo->SamplesPerSec;
	if (ptval->dtime!= paraDecTime.NoValue)
		pt->DTime = (float)ptval->dtime*pMasterInfo->SamplesPerSec/240.0;
	if (ptval->rslope!= paraRelSlope.NoValue)
		pt->RSlope = (float)pow(20,ptval->rslope/240.0-1)*25/pMasterInfo->SamplesPerSec;
	if (ptval->bdecay!= paraBDecay.NoValue)
		pt->BDecay = (float)(ptval->bdecay/240.0);
	if (ptval->cdecay!= paraCDecay.NoValue)
		pt->CDecay = (float)(ptval->cdecay/240.0);
	if (ptval->click!= paraClickAmt.NoValue)
		pt->ClickAmt = (float)(ptval->click/100.0);
	if (ptval->buzz!= paraBuzzAmt.NoValue)
		pt->BuzzAmt = 3*(float)(ptval->buzz/100.0);
	if (ptval->punch!= paraPunchAmt.NoValue)
		pt->PunchAmt = (float)(ptval->punch/100.0);
	if (ptval->pitchlimit != paraPitchLimit.NoValue && ptval->pitchlimit != NOTE_OFF)
  {
    int v=ptval->pitchlimit;
    v=(v&15)-1+12*(v>>4);
		pt->PitchLimit = (float)(440.0*pow(2,(v-69)/12.0));
    bTrig=true;
  }
  if (ptval->pitchlimit == NOTE_OFF && pt->EnvPhase<pt->ThisDTime)
  {
    if (ptval->ndelay && ptval->ndelay<6)
      pt->ThisDTime=pt->EnvPhase+ptval->ndelay*pMasterInfo->SamplesPerTick/6;
    else
      pt->ThisDTime=pt->EnvPhase;
  }
  if (bTrig)
  {
    if (ptval->ndelay && ptval->ndelay!=255)
    {
      pt->Retrig = 0;
      if (ptval->ndelay<6)
        pt->SamplesToGo = ptval->ndelay*pMasterInfo->SamplesPerTick/6;
      else
      if (ptval->ndelay<11)
      {
        Trigger(pt);
        pt->SamplesToGo = (ptval->ndelay-5)*pMasterInfo->SamplesPerTick/6;
      }
      else
      {
        Trigger(pt);
        pt->Retrig = pt->SamplesToGo = (ptval->ndelay-10)*pMasterInfo->SamplesPerTick/6;
        pt->RetrigCount = 6/(ptval->ndelay-10)-2;
      }
    }
    else
    {
      pt->Retrig = 0;
      Trigger(pt);
    }
  }
}

void mi::Trigger(CTrack *pt)
{
  if (pt->Retrig && pt->RetrigCount>0)
  {
    pt->SamplesToGo = pt->Retrig;
    pt->RetrigCount--;
  } 
  else
    pt->SamplesToGo = 0;
  pt->AntiClick = pt->LastValue;
  pt->EnvPhase = 0;
  pt->OscPhase = pt->ClickAmt;
  pt->LeftOver = 0;
  pt->Age = 0;
  pt->Amp = 32;
  pt->ThisPitchLimit = pt->PitchLimit;
  pt->ThisDTime = pt->DTime;
  pt->ThisDSlope = pt->DSlope;
  pt->ThisRSlope = pt->RSlope;
  pt->ThisBDecay = pt->BDecay;
  pt->ThisCDecay = pt->CDecay;
  pt->ThisTDecay = pt->TDecay;
  pt->ThisTShape = pt->TShape;
  pt->ThisStartFrq = pt->StartFrq;
  pt->ThisEndFrq = pt->EndFrq;
  pt->ThisCurVolume = pt->CurVolume;
}


void mi::Init(CMachineDataInput * const pi)
{
	numTracks = 1;
  InitTrack(0);

  for (int i=0; i<1024; i++)
    thumpdata1[i]=float(sin(1.37*i+0.1337*(1024-i)*sin(1.1*i))*pow(1.0/256.0,i/1024.0));
  
	for (int c = 0; c < MAX_TAPS; c++)
	{
    tvals vals;
    
    vals.pitchlimit=NOTE_NO;
    vals.volume=paraTrigger.DefValue;

    vals.startfrq=paraStartFrq.DefValue;
    vals.endfrq=paraEndFrq.DefValue;
    vals.buzz=paraBuzzAmt.DefValue;
    vals.click=paraClickAmt.DefValue;
    vals.punch=paraPunchAmt.DefValue;

    vals.tdecay=paraToneDecay.DefValue;
    vals.tshape=paraToneShape.DefValue;

    vals.bdecay=paraBDecay.DefValue;
    vals.cdecay=paraCDecay.DefValue;
    vals.dslope=paraDecSlope.DefValue;
    vals.dtime=paraDecTime.DefValue;
    vals.rslope=paraRelSlope.DefValue;
    vals.ndelay=0;

    TickTrack(&Tracks[c], &vals);
    Tracks[c].CurVolume = 32000.0;
    Tracks[c].Age=0;
    Tracks[c].Amp=0;
    Tracks[c].LeftOver=32000;
    Tracks[c].EnvPhase=6553600;
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
	Tracks[i].Retrig = 0;
	Tracks[i].SamplesToGo = 0;
	Tracks[i].EnvPhase = 0;
	Tracks[i].OscPhase = 0;
	Tracks[i].CurVolume = 32000;
	Tracks[i].LastValue = 0;
	Tracks[i].AntiClick = 0;
	Tracks[i].LeftOver = 0;
	Tracks[i].Age = 0;
	Tracks[i].Amp = 0;
	Tracks[i].DecAmp = 0;
	Tracks[i].BAmp = 0;
	Tracks[i].MulBAmp = 0;
	Tracks[i].CAmp = 0;
	Tracks[i].MulCAmp = 0;
	Tracks[i].Frequency = 0;
	Tracks[i].xSin = 0;
	Tracks[i].xCos = 0;
	Tracks[i].dxSin = 0;
	Tracks[i].dxCos = 0;
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

static bool DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
  trk->OscPhase=fmod(trk->OscPhase,1.0);
  float Ratio=trk->ThisEndFrq/trk->ThisStartFrq;
  if (trk->AntiClick<-64000) trk->AntiClick=-64000;
  if (trk->AntiClick>=64000) trk->AntiClick=64000;
  int i=0;
  double xSin=trk->xSin, xCos=trk->xCos;
  double dxSin=trk->dxSin, dxCos=trk->dxCos;
  float LVal=0;
  float AClick=trk->AntiClick;
  float Amp=trk->Amp;
  float DecAmp=trk->DecAmp;
  float BAmp=trk->BAmp;
  float MulBAmp=trk->MulBAmp;
  float CAmp=trk->CAmp;
  float MulCAmp=trk->MulCAmp;
  float Vol=0.5*trk->ThisCurVolume;
  bool amphigh=Amp>=16;
  int Age=trk->Age;
  float sr=pmi->pMasterInfo->SamplesPerSec;
  float odsr=1.0f/sr;
  while(i<c)
  {
    if (trk->SamplesToGo==1)
    {
      pmi->Trigger(trk);
      AClick=trk->AntiClick;
      Age=trk->Age;
      Amp=trk->Amp;
    }
    if (trk->LeftOver<=0)
    {
      trk->LeftOver=32;
      double EnvPoint=trk->EnvPhase*trk->ThisTDecay;
      double ShapedPoint=pow(EnvPoint,trk->ThisTShape*2.0);
      trk->Frequency=(float)(trk->ThisStartFrq*pow((double)Ratio,ShapedPoint));
      if (trk->Frequency>10000.f) trk->EnvPhase=6553600;
      if (trk->EnvPhase<trk->ThisDTime)
      {
        trk->DecAmp=DecAmp=trk->ThisDSlope;
        trk->Amp=Amp=(float)(1-DecAmp*trk->EnvPhase);
      }
      else
      {
        DecAmp=trk->ThisDSlope;
        Amp=(float)(1-DecAmp*trk->ThisDTime);
        if (Amp>0)
        {
          trk->DecAmp=DecAmp=trk->ThisRSlope;
          trk->Amp=Amp=Amp-DecAmp*(trk->EnvPhase-trk->ThisDTime);
        }
      }
      if (trk->Amp<=0)
      {
        trk->Amp=0;
        trk->DecAmp=0;
        if (fabs(AClick)<0.00012f && !trk->SamplesToGo)
          return amphigh;
      }

      trk->BAmp=BAmp=trk->BuzzAmt*(float)(pow(1.0f/256.0f,trk->ThisBDecay*trk->EnvPhase*(odsr*10)));
      float CVal=(float)(pow(1.0f/256.0f,trk->ThisCDecay*trk->EnvPhase*(odsr*20)));
      trk->CAmp=CAmp=trk->ClickAmt*CVal;
      trk->Frequency*=(1+2*trk->PunchAmt*CVal*CVal*CVal);
      if (trk->Frequency>10000) trk->Frequency=10000;
      if (trk->Frequency<trk->ThisPitchLimit) trk->Frequency=trk->ThisPitchLimit;

      trk->MulBAmp=MulBAmp=(float)pow(1.0f/256.0f,trk->ThisBDecay*(10*odsr));
      trk->MulCAmp=MulCAmp=(float)pow(1.0f/256.0f,trk->ThisCDecay*(10*odsr));
      xSin=(float)sin(2.0*3.141592665*trk->OscPhase);
      xCos=(float)cos(2.0*3.141592665*trk->OscPhase);
      dxSin=(float)sin(2.0*3.141592665*trk->Frequency/sr);
      dxCos=(float)cos(2.0*3.141592665*trk->Frequency/sr);
      LVal=0.0;
      trk->dxSin=dxSin, trk->dxCos=dxCos;
    }
    int max=__min(i+trk->LeftOver,c);
    if (trk->SamplesToGo>0)
      max=__min(max,i+trk->SamplesToGo-1);
    if (Amp>0.00001f && Vol>0)
    {
      amphigh = true;
      float OldAmp=Amp;
      if (BAmp>0.01f)
      {
        for (int j=i; j<max; j++)
        {
          pout[j]+=float(LVal=float(AClick+Amp*Vol*xSin));
          if (xSin>0)
          {
            float D=(float)(Amp*Vol*BAmp*xSin*xCos);
            pout[j]-=D;
            LVal-=D;
          }
          double xSin2=double(xSin*dxCos+xCos*dxSin);
          double xCos2=double(xCos*dxCos-xSin*dxSin);
          xSin=xSin2;xCos=xCos2;
          Amp-=DecAmp;
          BAmp*=MulBAmp;
          AClick*=0.98f;
        }
      }
      else
      for (int j=i; j<max; j++)
      {
        pout[j]+=float(LVal=float(AClick+Amp*Vol*xSin));
        double xSin2=double(xSin*dxCos+xCos*dxSin);
        double xCos2=double(xCos*dxCos-xSin*dxSin);
        xSin=xSin2;xCos=xCos2;
        Amp-=DecAmp;
        AClick*=0.98f;
      }
      if (fabs(AClick)<0.0001f) AClick=0.0001f;
      if (OldAmp>0.1f && CAmp>0.001f)
      {
        int max2=i+__min(max-i, 1024-Age);
        float LVal2=0.f;
        for (int j=i; j<max2; j++)
        {
          pout[j]+=(LVal2=OldAmp*Vol*CAmp*thumpdata1[Age]);
          OldAmp-=DecAmp;
          CAmp*=MulCAmp;
          Age++;
        }
        LVal+=LVal2;
      }
    }
    if (Amp)
    {
      trk->OscPhase+=(max-i)*trk->Frequency/sr;
      trk->EnvPhase+=max-i;
      trk->LeftOver-=max-i;
    }
    else
      trk->LeftOver=32000;
    if (trk->SamplesToGo>0) trk->SamplesToGo-=max-i;
    i=max;
  }
  trk->xSin=xSin, trk->xCos=xCos;
  trk->LastValue=LVal;
  trk->AntiClick=AClick;
  trk->Amp=Amp;
  trk->BAmp=BAmp;
  trk->CAmp=CAmp;
  trk->Age=Age;
  return amphigh;
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


bool mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
  return DoWork(pin,pout,this,numsamples,pt);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
  for (int i=0; i<numsamples; i++)
    psamples[i]=0.0;

  bool donesth=false;
  for (int c = 0; c < numTracks; c++)
		donesth |= WorkTrack(Tracks + c, NULL, psamples, numsamples, mode);

	return donesth;
}


