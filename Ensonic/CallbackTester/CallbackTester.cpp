#include <windef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include "MachineInterface.h"
#include "dsplib.h"

#pragma optimize ("a", on)

#define MAX_TRACKS				8

#define EGS_ATTACK				0
#define EGS_SUSTAIN				1 
#define EGS_RELEASE				2
#define EGS_NONE				3

#define MIN_AMP					(0.0001 * (32768.0 / 0x7fffffff))

#ifdef _DEBUG
#include <windows.h>
static char DebugStr[1024];
#endif

double const oolog2 = 1.0 / log(2.0);

// global values

CMachineParameter const paraTest = 
{ 
	pt_word,										// type
	"Test",
	"Test value",							        // description
	1,												// MinValue	
	0xffff,											// MaxValue
	0,												// NoValue
	MPF_STATE,										// Flags
	16                                              // DefValue
};

// track values

CMachineParameter const paraAttack = 
{ 
	pt_word,										// type
	"Attack",
	"Attack time in ms",							// description
	1,												// MinValue	
	0xffff,											// MaxValue
	0,												// NoValue
	MPF_STATE,										// Flags
	16                                              // DefValue
};

CMachineParameter const paraSustain = 
{  
	pt_word,										// type
	"Sustain",
	"Sustain time in ms",							// description
	1,												// MinValue	
	0xffff,											// MaxValue
	0,												// NoValue
	MPF_STATE,										// Flags
	16                                              // DefValue
};

CMachineParameter const paraRelease = 
{ 
	pt_word,										// type
	"Release",
	"Release time in ms",							// description
	1,												// MinValue	
	0xffff,											// MaxValue
	0,												// NoValue
	MPF_STATE,										// Flags
	512                                             // DefValue
};

CMachineParameter const paraColor = 
{ 
	pt_word,										// type
	"Color",
	"Noise color (0=black, 1000=white)",			// description
	0,												// MinValue	
	0x1000,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x1000                                          // DefValue
};

CMachineParameter const paraVolume = 
{ 
	pt_byte,										// type
	"Volume",
	"Volume [sustain level] (0=0%, 80=100%, FE=~200%)",	// description
	0,												// MinValue	
	0xfe,  											// MaxValue
	0xff,    										// NoValue
	MPF_STATE,										// Flags
	0x80                                            // DefValue
};

CMachineParameter const paraTrigger = 
{ 
	pt_switch,										// type
	"Trigger",
	"Trigger (1=on, 0=off)",						// description
	-1, 											// MinValue	
	-1,			  									// MaxValue
	SWITCH_NO,    									// NoValue
	0,												// Flags
	0                                               // DefValue
};


CMachineParameter const *pParameters[] = {
    // global
    &paraTest,
	// track
	&paraAttack,
	&paraSustain,
	&paraRelease,
	&paraColor,
	&paraVolume,
	&paraTrigger
};

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

#pragma pack(1)

class gvals
{
public:
	word test;
};

class tvals
{
public:
	word attack;
	word sustain;
	word release;
	word color;
	byte volume;
	byte trigger;

};

class avals
{
public:
	int maxdelay;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,							// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	MAX_TRACKS,								// max tracks
	1,										// numGlobalParameters
	6,										// numTrackParameters
	pParameters,
	1, 
	pAttributes,
#ifdef _DEBUG
	"Buzztard Callback Tester (Debug build)",	// name
#else
	"Buzztard Callback Tester",
#endif
	"BCT",									// short name
	"Stefan Kost", 							// author
	NULL
};

class mi;

class CTrack
{
public:
	void Tick(tvals const &tv);
	void Stop();
	void Reset();
	void Generate(float *psamples, int numsamples);
	void Noise(float *psamples, int numsamples);

	int MSToSamples(double const ms);

public:
	double Amp;
	double AmpStep;
	double S1;
	double S2;

	float Volume;
	int Pos;
	int Step;
	int RandStat;
	
	int EGStage;
	int EGCount;
	int Attack;
	int Sustain;
	int Release;

	mi *pmi;
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
	CTrack Tracks[MAX_TRACKS];

    avals aval;
    gvals gval;
	tvals tval[MAX_TRACKS];

};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = (int *)&aval;
}

mi::~mi()
{

}

inline int CTrack::MSToSamples(double const ms)
{
	return (int)(pmi->pMasterInfo->SamplesPerSec * ms * (1.0 / 1000.0));
}

inline double CalcStep(double from, double to, int time)
{
	assert(from > 0);
	assert(to > 0);
	assert(time > 0);
	return pow(to / from, 1.0 / time);
}

void CTrack::Reset()
{
	EGStage = EGS_NONE;

	Attack = MSToSamples(16);
	Sustain = MSToSamples(16);
	Release = MSToSamples(512);

	Pos = 0;
	Step = 65536;
	Volume = 1.0;
	S1 = S2 = 0;
	RandStat = 0x16BA2118;
}

void mi::Init(CMachineDataInput * const pi)
{
#ifdef _DEBUG
    sprintf(DebugStr,"  mi::Init(%p)",pi);
	OutputDebugString(DebugStr);
#endif
	for (int c = 0; c < MAX_TRACKS; c++)
	{
		Tracks[c].pmi = this;
		Tracks[c].Reset();
	}

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
    sprintf(DebugStr,"    aval.maxdelay=%d",aval.maxdelay);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    gval.test=%d",gval.test);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    tval[0].attack=%d",tval[0].attack);
	OutputDebugString(DebugStr);
    OutputDebugString("  mi:Init() done");
#endif
}

void CTrack::Tick(tvals const &tv)
{
	if (tv.attack != paraAttack.NoValue)
		Attack = MSToSamples(tv.attack);

	if (tv.sustain != paraSustain.NoValue)
		Sustain = MSToSamples(tv.sustain);

	if (tv.release != paraRelease.NoValue)
		Release = MSToSamples(tv.release);

	if (tv.color != paraColor.NoValue)
		Step = tv.color * 16;	// 0..4096 -> 0..65536
	
	if (tv.volume != paraVolume.NoValue)
		Volume = (float)(tv.volume * (1.0 / 0x80));

	if (tv.trigger != SWITCH_NO)
	{
		if (Volume > 0)
		{
			EGStage = EGS_ATTACK;
			EGCount = Attack;
			Amp = MIN_AMP;
			AmpStep = CalcStep(MIN_AMP, Volume * (32768.0 / 0x7fffffff), Attack);
		}
	}
}

void CTrack::Noise(float *psamples, int numsamples)
{
	double amp = Amp;
	double const ampstep = AmpStep;		
	double s1 = S1;
	double s2 = S2;
	int const step = Step;
	int stat = RandStat;
	int pos = Pos;

	int c = numsamples;
	do
	{
		*psamples++ = (float)(s1 + (s2 - s1) * (pos * 1.0 / 65536.0));
		amp *= ampstep;

		pos += step;
		if (pos & 65536)
		{
			s1 = s2;
			stat = ((stat * 1103515245 + 12345) & 0x7fffffff) - 0x40000000;
			s2 = stat * amp;

			pos -= 65536;
		}

	} while(--c);

	Pos = pos;
	S2 = s2;
	S1 = s1;
	RandStat = stat;
	Amp = amp;
}

void CTrack::Generate(float *psamples, int numsamples) {
	do
	{
		int const c = __min(EGCount, numsamples);
		assert(c > 0);

		if (EGStage != EGS_NONE)
			Noise(psamples, c);
		else
			memset(psamples, 0, c * sizeof(float));
		
		numsamples -= c;
		psamples += c;
		EGCount -= c;

		if (!EGCount)
		{
			switch(++EGStage)
			{
			case EGS_SUSTAIN:
				EGCount = Sustain;
				AmpStep = 1.0;
				break;
			case EGS_RELEASE:
				EGCount = Release;
				AmpStep = CalcStep(Amp, MIN_AMP, Release);
				break;
			case EGS_NONE:
				EGCount = 0x7fffffff;
				break;
			}
		}

		

	} while(numsamples > 0);
}
 
void mi::Tick()
{
#ifdef _DEBUG
    sprintf(DebugStr,"    aval.maxdelay=%d",aval.maxdelay);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    gval.test=%d",gval.test);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    tval[0].attack=%d",tval[0].attack);
	OutputDebugString(DebugStr);
    // shouldn't this be initialy 0? (it is not)
    sprintf(DebugStr,"  mi::Tick(%d)",pMasterInfo->PosInTick);
	OutputDebugString(DebugStr);
#endif
	for (int c = 0; c < numTracks; c++)
		Tracks[c].Tick(tval[c]);

}

bool mi::Work(float *psamples, int numsamples, int const)
{
	bool gotsomething = false;

	for (int c = 0; c < numTracks; c++)
	{
		if (Tracks[c].EGStage != EGS_NONE)
		{
			if (!gotsomething)
			{
				Tracks[c].Generate(psamples, numsamples);
				gotsomething = true;
			}
			else
			{
				float *paux = pCB->GetAuxBuffer();
				Tracks[c].Generate(paux, numsamples);

				DSP_Add(psamples, paux, numsamples);

			}
		}
	}

	return gotsomething;
}

void CTrack::Stop()
{
	EGStage = EGS_NONE;
}

void mi::Stop()
{
#ifdef _DEBUG
	OutputDebugString("  mi::Stop()");
#endif
	for (int c = 0; c < numTracks; c++)
		Tracks[c].Stop();
}
 
void mi::AttributesChanged() {
#ifdef _DEBUG
    sprintf(DebugStr,"    aval.maxdelay=%d",aval.maxdelay);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    gval.test=%d",gval.test);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    tval[0].attack=%d",tval[0].attack);
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
    sprintf(DebugStr,"    aval.maxdelay=%d",aval.maxdelay);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    gval.test=%d",gval.test);
	OutputDebugString(DebugStr);
    sprintf(DebugStr,"    tval[0].attack=%d",tval[0].attack);
	OutputDebugString(DebugStr);
	sprintf(DebugStr,"  mi::SetNumTracks: %d->%d",numTracks,n);
	OutputDebugString(DebugStr);
#endif
	numTracks = n;
}
