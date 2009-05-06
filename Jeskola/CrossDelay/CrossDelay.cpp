/* this machine has been rewritten as the original source is not available
 */
#include <windef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		8

CMachineParameter const paraLeftLength = 
{ 
	pt_word,										// type
	"L. Length",
	"Left length in length units",					// description
	1,												// MinValue	
	32768,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	3
};

CMachineParameter const paraRightLength = 
{ 
	pt_word,										// type
	"R. Length",
	"Right length in length units",					// description
	1,												// MinValue	
	32768,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	3
};

#define UNIT_TICK		0
#define UNIT_MS			1
#define UNIT_SAMPLE		2
#define UNIT_256		3

CMachineParameter const paraDryThru = 
{ 
	pt_switch,										// type
	"Dry thru",
	"Dry thru (1 = yes, 0 = no)",					// description
	-1,												// MinValue	
	-1,												// MaxValue
	SWITCH_NO,										// NoValue
	MPF_STATE,										// Flags
	SWITCH_ON
};

CMachineParameter const paraUnit =
{ 
	pt_byte, 										// type
	"Length unit",
	"Length unit (0 = tick (default), 1 = ms, 2 = sample, 3 = 256th of tick)",		
	0,												// MinValue	
	3,												// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraFeedback = 
{ 
	pt_byte,										// type
	"Feedback",
	"Feedback (00 = -100%, 40=0%, 80 = 100%)",		// description
	0,												// MinValue	
	128,											// MaxValue
	255,											// NoValue
	MPF_STATE,										// Flags
	0x60
};

CMachineParameter const paraWetOut = 
{ 
	pt_byte,										// type
	"Wet out",
	"Wet out (00 = 0%, 80 = 100%)",					// description
	0,												// MinValue	
	128,											// MaxValue
	255,											// NoValue
	MPF_STATE,										// Flags
	0x30
};


CMachineParameter const *pParameters[] = 
{ 
	&paraDryThru,
	&paraLeftLength,
	&paraRightLength,
	&paraUnit,
	&paraFeedback,
	&paraWetOut,
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
	byte drythru;
};

class tvals
{
public:
	word left_length;
	word right_length;
	byte unit;
	byte feedback;
	byte wetout;
};

class avals
{
public:
	int maxdelay;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	MIF_MONO_TO_STEREO,						// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	1,										// numGlobalParameters
	5,										// numTrackParameters
	pParameters,
	1,
	pAttributes,
#ifdef _DEBUG
	"Jeskola Cross Delay (Debug build)",			// name
#else
	"Jeskola Cross Delay",
#endif
	"X-Delay",								// short name
	"Oskari Tammelin, Stefan Kost",						// author
	NULL
};

class CTrack
{
public:
	float *LeftBuffer;
    float *RightBuffer;
	int LeftLength;
    int RightLength;
	int LeftPos;
    int RightPos;
	float Feedback;
	float WetOut;
	int Unit;
};
 

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);

	virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

private:
	int MaxDelay;	// in samples
	int IdleCount;	// in samples
	int DelayTime;
	bool IdleMode;
	bool DryThru;

private:
	int numTracks;
	CTrack Tracks[MAX_TAPS];

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
}

mi::~mi()
{
	for (int c = 0; c < MAX_TAPS; c++)
	{
		delete[] Tracks[c].LeftBuffer;
        delete[] Tracks[c].RightBuffer;
	}
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 1:		// left length
		return NULL;
		break;
	case 2:		// right length
		return NULL;
		break;
	case 3:		// unit
		switch(value)
		{
		case 0: return "tick";
		case 1: return "ms";
		case 2: return "sample";
		case 3: return "tick/256";
		}
		break;
	case 4:		// feedback
		sprintf(txt, "%.1f%%", (double)(value-64) * (100.0 / 64.0));
		break;
	case 5:		// wetout
		sprintf(txt, "%.1f%%", (double)value * (100.0 / 128.0));
		break;
	default:
		return NULL;
	}

	return txt;
}



void mi::Init(CMachineDataInput * const pi)
{
	numTracks = 1;
	DryThru = true;

	for (int c = 0; c < MAX_TAPS; c++)
	{
		Tracks[c].LeftBuffer = Tracks[c].RightBuffer = NULL;
		Tracks[c].Unit = UNIT_TICK;
		Tracks[c].LeftLength = Tracks[c].RightLength = pMasterInfo->SamplesPerTick * 3;
		Tracks[c].LeftPos = Tracks[c].RightPos = 0;
		Tracks[c].Feedback = 0.3f;
		Tracks[c].WetOut = 0;
	}

	Tracks[0].WetOut = 0.3f;	// enable first track

	IdleMode = true;
	IdleCount = 0;

}

void mi::AttributesChanged()
{
	MaxDelay = (int)(pMasterInfo->SamplesPerSec * (aval.maxdelay / 1000.0));
	for (int c = 0; c < numTracks; c++)
		InitTrack(c);
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
	delete[] Tracks[i].LeftBuffer;
	Tracks[i].LeftBuffer = new float [MaxDelay];
	memset(Tracks[i].LeftBuffer, 0, MaxDelay*4);
	delete[] Tracks[i].RightBuffer;
	Tracks[i].RightBuffer = new float [MaxDelay];
	memset(Tracks[i].RightBuffer, 0, MaxDelay*4);

	Tracks[i].LeftPos = Tracks[i].RightPos = 0;

	if (Tracks[i].LeftLength > MaxDelay)
		Tracks[i].LeftLength = MaxDelay;
	if (Tracks[i].RightLength > MaxDelay)
		Tracks[i].RightLength = MaxDelay;
}

void mi::ResetTrack(int const i)
{
	delete[] Tracks[i].LeftBuffer;
	Tracks[i].LeftBuffer = NULL;
	delete[] Tracks[i].RightBuffer;
	Tracks[i].RightBuffer = NULL;
}


void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->unit != paraUnit.NoValue)
		pt->Unit = ptval->unit;

	if (ptval->left_length != paraLeftLength.NoValue)
	{
		switch(pt->Unit)
		{
		case UNIT_MS:
			pt->LeftLength = (int)(pMasterInfo->SamplesPerSec * (ptval->left_length / 1000.0));
			if (pt->LeftLength < 1)
				pt->LeftLength = 1;
			break;
		case UNIT_SAMPLE:
			pt->LeftLength = ptval->left_length;
			break;
		case UNIT_TICK:
			pt->LeftLength = pMasterInfo->SamplesPerTick * ptval->left_length;
			break;
		case UNIT_256:
			pt->LeftLength = (pMasterInfo->SamplesPerTick * ptval->left_length) >> 8;
			if (pt->LeftLength < 1)
				pt->LeftLength = 1;
			break;
		}
        if (pt->LeftLength > MaxDelay)
        {
            pt->LeftLength = MaxDelay;
        }
	}
	if (pt->LeftPos >= pt->LeftLength)
		pt->LeftPos = 0;

	if (ptval->right_length != paraRightLength.NoValue)
	{
		switch(pt->Unit)
		{
		case UNIT_MS:
			pt->RightLength = (int)(pMasterInfo->SamplesPerSec * (ptval->right_length / 1000.0));
			if (pt->RightLength < 1)
				pt->RightLength = 1;
			break;
		case UNIT_SAMPLE:
			pt->RightLength = ptval->right_length;
			break;
		case UNIT_TICK:
			pt->RightLength = pMasterInfo->SamplesPerTick * ptval->right_length;
			break;
		case UNIT_256:
			pt->RightLength = (pMasterInfo->SamplesPerTick * ptval->right_length) >> 8;
			if (pt->RightLength < 1)
				pt->RightLength = 1;
			break;
		}
        if (pt->RightLength > MaxDelay)
        {
            pt->RightLength = MaxDelay;
        }
	}
	if (pt->RightPos >= pt->RightLength)
		pt->RightPos = 0;
      
      
	if (ptval->feedback != paraFeedback.NoValue)
	{
		pt->Feedback = (float)((ptval->feedback - 64) * (1.0 / 64.0)); 
	}

	if (ptval->wetout != paraWetOut.NoValue)
		pt->WetOut = (float)(ptval->wetout * (1.0 / 128.0));
	
}

void mi::Tick()
{
	int c;

	for (c = 0; c < numTracks; c++)
		TickTrack(Tracks + c, tval+c);


	// find max delay time slow we know when to stop wasting CPU time
	int maxdt = 0;
	for (c = 0; c < numTracks; c++)
	{
		int dt;

        dt = Tracks[c].LeftLength + (int)(SilentEnough / log(fabs(Tracks[c].Feedback)) * Tracks[c].LeftLength); 
		if (dt > maxdt)
			maxdt = dt;
        dt = Tracks[c].RightLength + (int)(SilentEnough / log(fabs(Tracks[c].Feedback)) * Tracks[c].RightLength); 
		if (dt > maxdt)
			maxdt = dt;
	}
	
	DelayTime = maxdt;


	
	if (gval.drythru != SWITCH_NO)
		DryThru = gval.drythru != 0;
}

#pragma optimize ("a", on) 

static void DoWork(float *pin, float *pout,  float *plbuf, float *prbuf, int c, double const wetout, double const feedback)
{
	do
	{
		double ldelay = *plbuf;
        double rdelay = *prbuf;
		*plbuf++ = (float)(feedback * rdelay + *pin);
		*prbuf++ = (float)(feedback * ldelay + *pin++);
		*pout++ += (float)(ldelay * wetout);
        *pout++ += (float)(rdelay * wetout);

	} while(--c);
}

static void DoWorkNoFB(float *pin, float *pout,  float *plbuf, float *prbuf, int c, double const wetout)
{
	do
	{
		double ldelay = *plbuf;
        double rdelay = *prbuf;
		*plbuf++ = (float)(*pin);
        *prbuf++ = (float)(*pin++);
		*pout++ += (float)(ldelay * wetout);
        *pout++ += (float)(rdelay * wetout);

	} while(--c);
}

static void DoWorkNoInput(float *pout, float *plbuf, float *prbuf, int c, double const wetout, double const feedback)
{
	do
	{
		double ldelay = *plbuf;
        double rdelay = *prbuf;
		*plbuf++ = (float)(feedback * rdelay);
        *prbuf++ = (float)(feedback * ldelay);
		*pout++ += (float)(ldelay * wetout);
        *pout++ += (float)(rdelay * wetout);

	} while(--c);
}

static void DoWorkNoInputNoFB(float *pout,  float *plbuf, float *prbuf, int c, double const wetout)
{
	do
	{
		double ldelay = *plbuf;
        double rdelay = *prbuf;
		*plbuf++ = 0;
        *prbuf++ = 0;
		*pout++ += (float)(ldelay * wetout);
        *pout++ += (float)(rdelay * wetout);
	} while(--c);
}

static void DoWorkNoInputNoOutput( float *plbuf, float *prbuf, int c, double const feedback)
{
	do
	{
		*plbuf = (float)(*plbuf * feedback);
		plbuf++;
		*prbuf = (float)(*prbuf * feedback);
		prbuf++;
	} while(--c);
}

static void DoWorkNoOutput(float *pin, float *plbuf, float *prbuf, int c, double const feedback)
{
	do
	{
		double ldelay = *plbuf;
        double rdelay = *prbuf;
		double dry = *pin++;
		*plbuf++ = (float)(feedback * rdelay + dry);
        *prbuf++ = (float)(feedback * ldelay + dry);
	} while(--c);
}

static void DoWorkNoOutputNoFB(float *pin, float *plbuf, float *prbuf, int c)
{
    do
	{
      *plbuf++ = *pin;
      *prbuf++ = *pin++;
	} while(--c);
}


#pragma optimize ("", on)


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
	do
	{
		int count = __min(numsamples, pt->LeftLength - pt->LeftPos);
        count = __min(count, pt->RightLength - pt->RightPos);

		if (count > 0)
		{
			if (mode == WM_NOIO)
			{
				if (pt->Feedback != 0)
					DoWorkNoInputNoOutput(pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count, pt->Feedback);
			}
			else if (mode == WM_WRITE)
			{
				if (pt->Feedback != 0)
					DoWorkNoInput(pout, pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count, pt->WetOut, pt->Feedback);
				else
					DoWorkNoInputNoFB(pout, pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count, pt->WetOut);
			}
			else if (mode == WM_READ)
			{
				if (pt->Feedback != 0)
					DoWorkNoOutput(pin, pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count, pt->Feedback);
				else
					DoWorkNoOutputNoFB(pin, pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count);

			}
			else
			{
				if (pt->Feedback != 0)
					DoWork(pin, pout, pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count, pt->WetOut, pt->Feedback);
				else
					DoWorkNoFB(pin, pout, pt->LeftBuffer + pt->LeftPos, pt->RightBuffer + pt->RightPos, count, pt->WetOut);
			}
			
			pin += count;
			pout += count*2;
			numsamples -= count;
			pt->LeftPos += count;
            pt->RightPos += count;
		} 

		if (pt->LeftPos == pt->LeftLength)
			pt->LeftPos = 0;
		if (pt->RightPos == pt->RightLength)
			pt->RightPos = 0;

	} while(numsamples > 0);

}

bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
	if (mode & WM_READ)
	{
		IdleMode = false;
		IdleCount = 0;
	}
	else
	{
		if (IdleMode)
		{
			return false;
		}
		else
		{
			IdleCount += numsamples;
			if (IdleCount >= DelayTime + MAX_BUFFER_LENGTH)
			{
				// clear all buffers
				for (int c = 0; c < numTracks; c++) {
					memset(Tracks[c].LeftBuffer, 0, Tracks[c].LeftLength*4);
					memset(Tracks[c].RightBuffer, 0, Tracks[c].RightLength*4);
                }
				IdleMode = true;
			}
		}
	}

    float *paux = pCB->GetAuxBuffer();

	if (mode & WM_READ)
        memcpy(paux, pin, numsamples*4);

	if (!DryThru || !(mode & WM_READ))
		memset(pout, 0, numsamples*4*2);

	for (int c = 0; c < numTracks; c++)
		WorkTrack(Tracks + c, paux, pout, numsamples, mode);

	return true;
}


