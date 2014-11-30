#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

double const SilentEnough = log(1.0 / 32768);

// Use functions instead of tables for waves

#define MAXVOICES	3

static int wavetab[2][8192];

CMachineParameter const paraMinDelay = 
{ 
	pt_byte,										// type
	"Min delay",
	"Min delay in MS",						// description
	0,												// MinValue	
	200,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	1,
};

CMachineParameter const paraDelayMod = 
{
	pt_byte,										// type
	"Delay mod",
	"Delay Modulation",						// description
	0,												// MinValue	
	200,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	5,
};

CMachineParameter const paraRate = 
{ 
	pt_byte,										// type
	"Rate",
	"Rate (hz)",										// description
	0,												// MinValue	
	127,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	10,
};

CMachineParameter const paraShape = 
{ 
	pt_byte,										// type
	"LFO Shape",
	"LFO Shape (Sine/Triangle)",							// description
	0,												// MinValue	
	1,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraVoices = 
{ 
	pt_byte,										// type
	"Voices",
	"Number of Voices",							// description
	1,												// MinValue	
	3,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	1,
};


CMachineParameter const paraPhaseDiff = 
{ 
	pt_byte,										// type
	"PhaseDiff",
	"Voice Phase Difference",							// description
	1,												// MinValue	
	23,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	3,
};

CMachineParameter const paraStereoDiff = 
{ 
	pt_byte,										// type
	"StereoPhase",
	"Stereo Phase Difference",							// description
	0,												// MinValue	
	23,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	6,
};

CMachineParameter const paraDelaySpread = 
{ 
	pt_byte,										// type
	"DelaySpread",
	"Voice Delay Difference",							// description
	0,												// MinValue	
	10,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0,
};


CMachineParameter const paraMix = 
{ 
	pt_byte,										// type
	"Mix",
	"Mix (dry/wet)",										// description
	0,												// MinValue	
	127,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	64,
};

CMachineParameter const paraFeedback = 
{ 
	pt_byte,										// type
	"Feedback",
	"Feedback",										// description
	1,												// MinValue	
	199,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	150,
};

#pragma pack(1)	

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraVoices,
	&paraShape,
	&paraMinDelay,
	&paraDelayMod,	
	&paraRate,
	&paraPhaseDiff,
	&paraStereoDiff,
	&paraDelaySpread,
	&paraFeedback,
	&paraMix
};

CMachineAttribute const attrSymmetric = 
{
	"Suck Me Beautiful",
	0,
	1,
	0
};

CMachineAttribute const *pAttributes[] = 
{
	&attrSymmetric
};


#pragma pack(1)	

class gvals
{
public:
	byte voices;
	byte shape;
	byte mindelay;
	byte delaymod;
	byte rate;
	byte phasediff;
	byte stereodiff;
	byte delayspread;
	byte feedback;
	byte mix;
};

class avals
{
public:
	int symmetric;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,						// flags
	0,										// min tracks
	0,										// max tracks
	10,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	1,
	pAttributes,
#ifdef _DEBUG
	"WhiteNoise Chorus",		// name
#else
	"WhiteNoise Chorus",		// name
#endif
	"WhiteChorus",									// short name
	"WhiteNoise",						// author
	NULL
};

class miex : public CMDKMachineInterfaceEx
{

};

class mi : public CMDKMachineInterface
{
public:
	mi();
	virtual ~mi();

	//virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);
	virtual void MDKInit(CMachineDataInput * const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
	
	virtual void MDKSave(CMachineDataOutput * const po);


public:
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
	virtual void OutputModeChanged(bool stereo) {}
	miex ex;

private:

	void MonotoStereoChorus(float *pout, int c);
	void MonotoMonoChorus(float *pout, int c);
	void StereotoStereoChorus(float *pout, int c);
	float *ReadBuffer;
	float *EchoBuffer[3];
			
	int BUFFERSIZE;
	int ReadPos;
	unsigned int LFOPos;
	unsigned int LFOAdd;

	int Voices;
	int MinDelay;		// Minimum delay time in samples
	int DelayMod;		// Modulation time in samples

	int PhaseDiff;		// Phase difference between each voice
	int StereoDiff;		// Phase Difference between left and right
	int DelaySpread;

	float Feedback;

	int Shutofftime;

	int Count;

	int LFOShape;
	const int *pwavetabLFO;	

private:
	float Wet, Dry;
	
	gvals gval;
	avals aval;

};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	AttrVals = (int *)&aval;
}

mi::~mi()
{
	delete ReadBuffer;
	delete EchoBuffer[0];
	delete EchoBuffer[1];
	delete EchoBuffer[2];
}

void mi::MDKInit(CMachineDataInput * const pi)
{
  int i;

	MinDelay = 1;
	DelayMod = 5;
	DelaySpread = 0;
	Wet = 0.5;
	Dry = 0.5;
	Voices = 1;
	PhaseDiff = 1365;
	StereoDiff = 2048;
	ReadPos = 0;
	LFOPos = 0;	
	LFOAdd = ((8192*10)<<11)/pMasterInfo->SamplesPerSec;
	LFOShape = 0;
	Count = 0;

	BUFFERSIZE = (pMasterInfo->SamplesPerSec)/8;

	ReadBuffer = new float[BUFFERSIZE*2];
	EchoBuffer[0] = new float[BUFFERSIZE*2];
	EchoBuffer[1] = new float[BUFFERSIZE*2];
	EchoBuffer[2] = new float[BUFFERSIZE*2];

	//memset(ReadBuffer, 0, BUFFERSIZE*sizeof(float));
	//memset(EchoBuffer[0], 0, BUFFERSIZE*sizeof(float)*2);
	//memset(EchoBuffer[1], 0, BUFFERSIZE*sizeof(float)*2);
	//memset(EchoBuffer[2], 0, BUFFERSIZE*sizeof(float)*2);

	for(i=0; i<BUFFERSIZE; i++)
	{
		ReadBuffer[i] = 0.0f;
		EchoBuffer[0][i<<1] = 0.0f;
		EchoBuffer[1][i<<1] = 0.0f;
		EchoBuffer[2][i<<1] = 0.0f;
		EchoBuffer[0][(i<<1)+1] = 0.0f;
		EchoBuffer[1][(i<<1)+1] = 0.0f;
		EchoBuffer[2][(i<<1)+1] = 0.0f;
	}

	ReadPos = BUFFERSIZE - 1;

	for(i=0; i<8192; i++)
		wavetab[0][i] = (int)((sin((PI*2.0)*(double)i/8192.0))*32768.0) + 32768;
	for(i=0; i<4096; i++)
		wavetab[1][i] = (int)((double)i*65535.0f/4096.0f);
	for(i=0; i<4096; i++)
		wavetab[1][i+4096] = 65536 - (int)((double)i*65535.0f/4096.0f);

	pwavetabLFO = &wavetab[0][0];
}

void mi::MDKSave(CMachineDataOutput * const po)
{
}

void mi::Tick()
{
	if (gval.mindelay != paraMinDelay.NoValue)
		MinDelay = ((pMasterInfo->SamplesPerSec*(int)gval.mindelay)/4000) + (pMasterInfo->SamplesPerSec/10000);
	if (gval.delaymod != paraDelayMod.NoValue)
		DelayMod = (pMasterInfo->SamplesPerSec*(int)gval.delaymod)/4000;
	if (gval.rate != paraRate.NoValue)
	{
		//if(gval.rate < 32)
		//{
			LFOAdd = ((8192*(int)gval.rate)<<16)/pMasterInfo->SamplesPerSec;
		//}
		//else
		//{
			//LFOAdd = ((2048*(int)(gval.rate-31))<<11)/pMasterInfo->SamplesPerSec;
		//}
	}


	if (gval.voices != paraVoices.NoValue)
		Voices = gval.voices;

	if (gval.shape != paraShape.NoValue)
	{
		LFOShape = gval.shape;
		pwavetabLFO = &wavetab[LFOShape][0];
	}

	if (gval.phasediff != paraPhaseDiff.NoValue)
	{
		PhaseDiff = ((int)gval.phasediff*8192)/24;
	}

	if (gval.stereodiff != paraStereoDiff.NoValue)
	{
		StereoDiff = ((int)gval.stereodiff*8192)/24;
	}

	if (gval.delayspread != paraDelaySpread.NoValue)
	{
		DelaySpread = (pMasterInfo->SamplesPerSec*(int)gval.delayspread)/1000;
	}

	if (gval.feedback != paraFeedback.NoValue)
		Feedback = ((float)gval.feedback-100) / 100.0;

	if (gval.mix != paraMix.NoValue)
	{
		Wet = (float)(gval.mix * (1.0 / 0x7f));
		Dry = 1.0 - Wet;
	}

	Shutofftime = MinDelay + DelayMod + DelaySpread*(Voices-1) + 
		(int)(SilentEnough / log(fabs(Feedback)) * (MinDelay + DelayMod));
}

inline int f2i(double d)
{
#ifdef WIN32
  const double magic = 6755399441055744.0; // 2^51 + 2^52
  double tmp = (d-0.5) + magic;
  return *(int*) &tmp;
#else
  // poor man's solution :)
  return (int)rint(d);
#endif
}

inline float LInterpolateF(float x1, float x2, long frac)			// Res: 4096
{
	float F = (float)frac*1.52587890625e-5;

	return x1 + (x2 - x1)*F;
}								

void mi::MonotoStereoChorus(float *pout, int c)
{
	int LookbackL, LookbackR;
	int voicelfoL, voicelfoR;
	int pdif, dmin;
	float outL, outR;
	float *echobuf;
	float delayline, feedline;
	int frac1, frac2;
	int tempind1;
	int readpos_stereoL, readpos_stereoR;
	int LFO_index;

	do
	{
		pdif = 0;
		dmin = MinDelay;
		outL = ReadBuffer[ReadPos]*Dry; // * dry
		outR = outL; // * dry

		readpos_stereoL = ReadPos<<1;
		readpos_stereoR = readpos_stereoL + 1;
		LFO_index = LFOPos>>19;

		for(int i=0; i<Voices; i++)
		{
			voicelfoL = LFO_index + pdif;
			voicelfoL &= 8191;

			voicelfoR = voicelfoL + StereoDiff;
			voicelfoR &= 8191;

			echobuf = EchoBuffer[i];

			pdif += PhaseDiff;
	
			frac1 = (DelayMod * pwavetabLFO[voicelfoL]);
			frac2 = (DelayMod * pwavetabLFO[voicelfoR]);
			LookbackL = ReadPos - dmin - (frac1>>16);		// >>16
			LookbackR = ReadPos - dmin - (frac2>>16);

			frac1 = 65536 - (frac1&65535);
			frac2 = 65536 - (frac2&65535);

			dmin += DelaySpread;

			//assert(Lookback[index1] <= ReadPos);

			if(LookbackL < 0)
				LookbackL += (BUFFERSIZE);

			if(LookbackR < 0)
				LookbackR += (BUFFERSIZE);

			//assert(Lookback[index1] >= 0);
			//assert(Lookback[index1] < BUFFERSIZE);

			tempind1 = (LookbackL+1);

			if(tempind1 >= BUFFERSIZE)
				tempind1 -= BUFFERSIZE;

			delayline = LInterpolateF(ReadBuffer[LookbackL], ReadBuffer[tempind1], frac1);
			feedline = LInterpolateF(echobuf[(LookbackL<<1)], echobuf[(tempind1<<1)], frac1);
			echobuf[readpos_stereoL] = delayline + feedline * Feedback;

			tempind1 = (LookbackR+1);

			if(tempind1 >= BUFFERSIZE)
				tempind1 -= BUFFERSIZE;

			delayline = LInterpolateF(ReadBuffer[LookbackR], ReadBuffer[tempind1], frac2);
			feedline = LInterpolateF(echobuf[(LookbackR<<1) + 1], echobuf[(tempind1<<1) + 1], frac2);
			echobuf[readpos_stereoR] = delayline + feedline * Feedback;

			outL += echobuf[readpos_stereoL]*Wet;
			outR += echobuf[readpos_stereoR]*Wet;
		}

		*pout++ = outL;		// left and right
		*pout++ = outR;

		LFOPos += LFOAdd;
		//LFOPos &= ((8192<<19)-1);

		ReadPos++;

		if(ReadPos >= BUFFERSIZE)
			ReadPos = 0;
	} while(--c);
}

void mi::MonotoMonoChorus(float *pout, int c)
{
	int Lookback[MAXVOICES];
	int voicelfo;
	int pdif, dmin;
	float out;
	float *echobuf;
	float delayline, feedline;
	int frac;
	int tempind;

	do
	{
		pdif = 0;
		dmin = MinDelay;
		out = ReadBuffer[ReadPos]*Dry; // * dry

		for(int i=0; i<Voices; i++)
		{
			voicelfo = (LFOPos>>19) + pdif;
			voicelfo &= 8191;

			echobuf = EchoBuffer[i];

			pdif += PhaseDiff;

			frac = (DelayMod * pwavetabLFO[voicelfo]);
			Lookback[i] = ReadPos - dmin - (frac>>16);		// >>16

			dmin += DelaySpread;

			//assert(Lookback[index1] <= ReadPos);

			if(Lookback[i] < 0)
				Lookback[i] += (BUFFERSIZE);

			tempind = (Lookback[i]+1);

			if(tempind >= BUFFERSIZE)
				tempind -= BUFFERSIZE;

			delayline = LInterpolateF(ReadBuffer[Lookback[i]], ReadBuffer[tempind], 65535 - (frac&65535));
			feedline = LInterpolateF(echobuf[Lookback[i]], echobuf[tempind], 65535 - (frac&65535));
			echobuf[ReadPos] = delayline + feedline * Feedback;

			out += echobuf[ReadPos]*Wet;
		}

		*pout++ = out;		// left and right
		*pout++ = out;

		LFOPos += LFOAdd;

		ReadPos++;

		if(ReadPos >= BUFFERSIZE)
			ReadPos = 0;
	} while(--c);
}

void mi::StereotoStereoChorus(float *pout, int c)
{
	int LookbackL, LookbackR;
	int voicelfoL, voicelfoR;
	int pdif, dmin;
	float outL, outR;
	float *echobuf;
	float delayline, feedline;
	int frac1, frac2;
	int tempind1;
	int readpos_stereoL, readpos_stereoR;
	int LFO_index;

	do
	{
		pdif = 0;
		dmin = MinDelay;
		readpos_stereoL = ReadPos<<1;
		readpos_stereoR = readpos_stereoL + 1;
		outL = ReadBuffer[readpos_stereoL]*Dry; // * dry
		outR = ReadBuffer[readpos_stereoR]*Dry; // * dry

		LFO_index = LFOPos>>19;

		for(int i=0; i<Voices; i++)
		{
			voicelfoL = LFO_index + pdif;
			voicelfoL &= 8191;

			voicelfoR = voicelfoL + StereoDiff;
			voicelfoR &= 8191;

			echobuf = EchoBuffer[i];

			pdif += PhaseDiff;
	
			frac1 = (DelayMod * pwavetabLFO[voicelfoL]);
			frac2 = (DelayMod * pwavetabLFO[voicelfoR]);
			LookbackL = ReadPos - dmin - (frac1>>16);		// >>16
			LookbackR = ReadPos - dmin - (frac2>>16);

			frac1 = 65536 - (frac1&65535);
			frac2 = 65536 - (frac2&65535);

			dmin += DelaySpread;

			//assert(Lookback[index1] <= ReadPos);

			if(LookbackL < 0)
				LookbackL += (BUFFERSIZE);

			if(LookbackR < 0)
				LookbackR += (BUFFERSIZE);

			//assert(Lookback[index1] >= 0);
			//assert(Lookback[index1] < BUFFERSIZE);

			tempind1 = (LookbackL+1);

			if(tempind1 >= BUFFERSIZE)
				tempind1 -= BUFFERSIZE;

			delayline = LInterpolateF(ReadBuffer[(LookbackL<<1)], ReadBuffer[(tempind1<<1)], frac1);
			feedline = LInterpolateF(echobuf[(LookbackL<<1)], echobuf[(tempind1<<1)], frac1);
			echobuf[readpos_stereoL] = delayline + feedline * Feedback;

			tempind1 = (LookbackR+1);

			if(tempind1 >= BUFFERSIZE)
				tempind1 -= BUFFERSIZE;

			// Possible opt's
			delayline = LInterpolateF(ReadBuffer[(LookbackR<<1)+1], ReadBuffer[(tempind1<<1)+1], frac2);
			feedline = LInterpolateF(echobuf[(LookbackR<<1) + 1], echobuf[(tempind1<<1) + 1], frac2);
			echobuf[readpos_stereoR] = delayline + feedline * Feedback;

			outL += echobuf[readpos_stereoL]*Wet;
			outR += echobuf[readpos_stereoR]*Wet;
		}

		*pout++ = outL;		// left and right
		*pout++ = outR;

		LFOPos += LFOAdd;
		//LFOPos &= ((8192<<19)-1);

		ReadPos++;

		if(ReadPos >= BUFFERSIZE)
			ReadPos = 0;
	} while(--c);
}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	if (!(mode & WM_READ) || !(mode & WM_WRITE))
	{
		return false;
	}
	else
		return true;
	
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	int c, i;
	int length;
	float *pin = psamples;

	if (mode == WM_NOIO)
		return false;
	
	if (mode == WM_READ)
		return true;

	if(mode == WM_WRITE)
	{
		if(Count)
		{
			// No input
			// memset readbuffer to zero
			// call main loop function
			length = BUFFERSIZE - ReadPos;
			if(length < numsamples)
			{
				memset(&ReadBuffer[ReadPos], 0, 2*length*sizeof(float));
				memset(ReadBuffer, 0, 2*(numsamples-length)*sizeof(float));
			}
			else
				memset(&ReadBuffer[ReadPos], 0, 2*numsamples*sizeof(float));

			if(Count < numsamples)
			{
				memset(psamples + Count*2, 0, (numsamples-Count)*2);
				c = Count;
			}
			else
				c = numsamples;

			Count -= c;

			//if(StereoDiff)
				StereotoStereoChorus(psamples, c);
			//else
				//MonotoMonoChorus(pout, c);

			return true;
		}
		else
			return false;
	}
	else
	{
		Count = Shutofftime;
		//Count = 0;
		
		// memcopy pin to readbuffer
		// call loop function
		length = BUFFERSIZE - ReadPos;
		if(length < numsamples)
		{
			for(i=0; i<(length<<1); i+=2)
			{
				ReadBuffer[i+(ReadPos<<1)] = *pin++;
				ReadBuffer[1+i+(ReadPos<<1)] = *pin++;
			}
			for(i=0; i<((numsamples-length)<<1); i+=2)
			{
				ReadBuffer[i] = *pin++;
				ReadBuffer[1+i] = *pin++;
			}
		}
		else
			for(i=0; i<(numsamples<<1); i+=2)
			{
				ReadBuffer[i+(ReadPos<<1)] = *pin++;
				ReadBuffer[1+i+(ReadPos<<1)] = *pin++;
			}

		//if(StereoDiff)
			StereotoStereoChorus(psamples, numsamples);
		//else
			//MonotoMonoChorus(pout, numsamples);

		return true;
	}
}

bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
	int c;
	int length;

	if (mode == WM_NOIO)
		return false;
	
	if (mode == WM_READ)
		return true;

	if(mode == WM_WRITE)
	{
		if(Count)
		{
			if(Count < numsamples)
			{
				memset(pout + Count*2, 0, (numsamples-Count)*2);
				c = Count;
			}
			else
				c = numsamples;

			Count -= c;

			// memset readbuffer to zero
			// call main loop function
			length = BUFFERSIZE - ReadPos;
			if(length < numsamples)
			{
				memset(&ReadBuffer[ReadPos], 0, length*sizeof(float));
				memset(ReadBuffer, 0, (numsamples-length)*sizeof(float));
			}
			else
				memset(&ReadBuffer[ReadPos], 0, numsamples*sizeof(float));

			//if(StereoDiff)
				MonotoStereoChorus(pout, c);
			//else
				//MonotoMonoChorus(pout, c);

			return true;
		}
		else
			return false;
	}
	else
	{
		Count = Shutofftime;
		//Count = 0;
		
		// memcopy pin to readbuffer
		// call loop function

		length = BUFFERSIZE - ReadPos;
		if(length < numsamples)
		{
			memcpy(&ReadBuffer[ReadPos], pin, length*sizeof(float));
			memcpy(ReadBuffer, pin+length, (numsamples-length)*sizeof(float));
		}
		else
			memcpy(&ReadBuffer[ReadPos], pin, numsamples*sizeof(float));

		if(StereoDiff)
			MonotoStereoChorus(pout, numsamples);
		else
			MonotoMonoChorus(pout, numsamples);

		return true;
	}
}



