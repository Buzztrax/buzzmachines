
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../../MachineInterface.h"

#define MAX_TRACKS	4

#define WAVEFORM_SAWTOOTH		1
#define WAVEFORM_PULSE			2
#define WAVEFORM_TRIANGLE		3

#define EGS_NONE				0
#define EGS_ATTACK				1
#define EGS_DECAY				2
#define EGS_SUSTAIN				3
#define EGS_RELEASE				4

// times are in milliseconds  
#define ATTACK_TIME				2
#define SUSTAIN_TIME			10
//#define RELEASE_TIME			150

//#define TOTAL_TIME				(ATTACK_TIME + SUSTAIN_TIME + RELEASE_TIME)

CMachineParameter const paraBDTrigger = 
{ 
	pt_switch,										// type
	"BD Trigger",
	"Bassdrum trigger",								// description
	-1,												// MinValue	
	-1,												// MaxValue
	SWITCH_NO,										// NoValue
	0,												// Flags
	0
};

CMachineParameter const paraBDTone = 
{ 
	pt_byte,										// type
	"BD Tone",
	"Bassdrum tone (0-7F)",							// description
	0,												// MinValue	
	127,											// MaxValue
	255,											// NoValue
	MPF_STATE,										// Flags
	64
};

CMachineParameter const paraBDDecay = 
{ 
	pt_byte,										// type
	"BD Decay",
	"Bassdrum decay (0-7F)",						// description
	0,												// MinValue	
	127,											// MaxValue
	255,											// NoValue
	MPF_STATE,										// Flags
	64
};


CMachineParameter const paraBDVolume = 
{ 
	pt_byte,										// type
	"BD Volume",
	"Bassdrum Volume (0=0%, 80=100%, FE=~198%)",	// description
	0,												// MinValue	
	254,											// MaxValue
	255,											// NoValue
	0,												// Flags
	0
};

CMachineParameter const *pParameters[] = { 
	// global
	&paraBDTrigger,
	&paraBDTone,
	&paraBDDecay,
	&paraBDVolume,
};

#pragma pack(1)

class gvals
{
public:
	byte bd_trigger;
	byte bd_tone;
	byte bd_decay;
	byte bd_volume;
};

class tvals
{
public:
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,							// type
	MI_VERSION,
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	4,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Jeskola Trilok (Debug build)",			// name
#else
	"Jeskola Trilok",
#endif
	"Trilok",								// short name
	"Oskari Tammelin", 						// author
	NULL
};

class CTrackState
{
public:

};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);
	virtual void SetNumTracks(int const n) { numTracks = n; }

private:

	void TickBassdrum();
	void GenerateBassdrum(float *psamples, int numsamples);

	void Filter(float *psamples, int numsamples);

private:
	double RELEASE_TIME;
	double TOTAL_TIME;

	double Cutoff;
	double Resonance;
	double Lowpass;
	double Bandpass;
	double Highpass;

	double BDPos;
	double BDStep;
	double BDStepStep;
	double BDEGValue;
	double BDEGStep;
	int BDEGStage;
	int BDEGCounter;
	int BDVolume;
	
	bool BDPlaying;
	

	int numTracks;
	CTrackState TrackStates[MAX_TRACKS];

	gvals gval;
	tvals tval[MAX_TRACKS];


};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;

}

mi::~mi()
{

}

void mi::Init(CMachineDataInput * const pi)
{
	Cutoff = 0.06;
	RELEASE_TIME = 150;
	TOTAL_TIME = (ATTACK_TIME + SUSTAIN_TIME + RELEASE_TIME);
	Resonance = 1.0;
	Lowpass = Bandpass = Highpass = 0;

	BDPlaying = false;
	BDPos = -32768;
	BDStep = 0;
	BDStepStep = 0;
	BDEGStage = EGS_NONE;
	BDEGCounter = 0;

}


void mi::TickBassdrum()
{
	if (gval.bd_tone != paraBDTone.NoValue)
	{
		Cutoff = 0.06 + (gval.bd_tone-64) * 0.05 / 127;
		if (Cutoff > 0.06)
		{
			Cutoff += (Cutoff - 0.06) * 0.3;
			Resonance = 1 - (Cutoff-0.06)*30;
		}
		else
		{
			Resonance = 1.0;
		}
	} 

	if (gval.bd_decay != paraBDDecay.NoValue)
	{
		RELEASE_TIME = pow(10, 2.17609 + (gval.bd_decay-64) * 1.9 / 127);

		TOTAL_TIME = (ATTACK_TIME + SUSTAIN_TIME + RELEASE_TIME);
	}
 

	if (gval.bd_trigger != SWITCH_NO)
	{
		BDPos = 0;
		BDStep = (65536.0) / (pMasterInfo->SamplesPerSec / 250.0);
		
		double endstep = (65536.0) / (pMasterInfo->SamplesPerSec / 1.0);
		double totallen = (TOTAL_TIME * (1.0 / 1000.0)) * pMasterInfo->SamplesPerSec;

		BDStepStep = 1 * (endstep - BDStep) / totallen;

		BDEGValue = 0;
		double len = (ATTACK_TIME * (1.0 / 1000.0)) * pMasterInfo->SamplesPerSec;
		BDEGCounter = (int)len;
		BDEGStep = 1.0 / len;
		BDEGStage = EGS_ATTACK;
		BDPlaying = true;

		
	}

}

void mi::Tick()
{
/*
	if (gval.cutoff != paraCutoff.NoValue)
	{
		Cutoff = gval.cutoff * (1.0 / 128.0);
	}
	
	if (gval.resonance != paraResonance.NoValue)
	{
		Resonance = (128 - gval.resonance) * (1.0 / 128.0);
	}
*/	
	TickBassdrum();

}



void mi::GenerateBassdrum(float *psamples, int numsamples)
{
	double pos = BDPos;
	double step = BDStep;
	double amp = BDEGValue;
	double ampstep = BDEGStep;
	double stepstep = BDStepStep;
	int egcounter = BDEGCounter;

	
	int c = numsamples;
	do
	{
		*psamples++ += (float)(pos * amp);
		pos += step;
		if (pos >= 32768.0)
		{
			pos -= step * 2;
			step = -step;
			stepstep = -stepstep;
		}
		else if (pos < -32768.0)
		{
			pos -= step * 2;
			step = -step;
			stepstep = -stepstep;
		}
		step += stepstep;

		amp += ampstep;

		if (--egcounter == 0)
		{
			if (BDEGStage == EGS_ATTACK)
			{
				double len = (SUSTAIN_TIME * (1.0 / 1000.0)) * pMasterInfo->SamplesPerSec;
				egcounter = (int)len;
				ampstep = 0;
				BDEGStage = EGS_SUSTAIN;
			}
			else if (BDEGStage == EGS_SUSTAIN)
			{
				double len = (RELEASE_TIME * (1.0 / 1000.0)) * pMasterInfo->SamplesPerSec;
				egcounter = (int)len;
				ampstep = -1.0 / len;
				BDEGStage = EGS_RELEASE;
			}
			else
			{
				ampstep = 0;
				amp = 0;
				BDPlaying = false;
			}
			
		}
	
	} while(--c);

	BDEGCounter = egcounter;
	BDEGStep = ampstep;
	BDEGValue = amp;
	BDStepStep = stepstep;
	BDStep = step;
	BDPos = pos;
}

void mi::Filter(float *psamples, int numsamples)
{
	double L = Lowpass;
	double B = Bandpass;
	double H = Highpass;

	double f1 = Cutoff, f2 = Cutoff, q = Resonance;

	int c = numsamples;
	do
	{
		L += f1 * B;
		H = *psamples - L - q * B;
		B += f2 * H;
		*psamples++ = (float)L;
	} while(--c);

	Highpass = H;
	Bandpass = B;
	Lowpass = L;
}


bool mi::Work(float *psamples, int numsamples, int const)
{
	memset(psamples, 0, numsamples * 4);

	bool gotstuff = false;
	
	if (BDPlaying)
	{
		GenerateBassdrum(psamples, numsamples);
		Filter(psamples, numsamples);
		gotstuff = true;
	}
	
	if (!gotstuff)
		return false;
	
	return true;
}
