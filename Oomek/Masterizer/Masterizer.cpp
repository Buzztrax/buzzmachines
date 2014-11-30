#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdlib.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

#define MAXDELAY	128
#define ANTIDENORMAL 1.0E-10f
#define E 2.718281828459045235360f

#pragma optimize ("awy", on)



CMachineParameter const paraInputGain=
{
	pt_byte,
	"Input gain",
	"Input gain level",
	0,
	0xF0,
	0xFF,
	MPF_STATE,
	0x3C
};


CMachineParameter const paraRatioLo=
{
	pt_byte,
	"Treshold Lo",
	"Low Band Treshold",
	0,
	0xF0,
	0xFF,
	MPF_STATE,
	120
};

CMachineParameter const paraRatioMid=
{
	pt_byte,
	"Treshold Mid",
	"Mid Band Treshold",
	0,
	0xF0,
	0xFF,
	MPF_STATE,
	120
};

CMachineParameter const paraRatioHi=
{
	pt_byte,
	"Treshold Hi",
	"High Band Treshold",
	0,
	0xF0,
	0xFF,
	MPF_STATE,
	120
};


CMachineParameter const paraDecayLo=
{
	pt_byte,
	"Decay Lo",
	"Decay in ms",
	10,
	0xF0,
	0xFF,
	MPF_STATE,
	50
};

CMachineParameter const paraDecayMid=
{
	pt_byte,
	"Decay Mid",
	"Decay in ms",
	10,
	0xF0,
	0xFF,
	MPF_STATE,
	50
};

CMachineParameter const paraDecayHi=
{
	pt_byte,
	"Decay Hi",
	"Decay in ms",
	10,
	0xF0,
	0xFF,
	MPF_STATE,
	50
};

CMachineParameter const paraSplitOne=
{
	pt_byte,
	"Lo | Mid Split",
	"Lo|Mid Split Freq.",
	2,
	112,
	0xFF,
	MPF_STATE,
	17
};

CMachineParameter const paraSplitTwo=
{
	pt_byte,
	"Mid | Hi Split",
	"Mid|Hi Split Freq.",
	0,
	240,
	0xFF,
	MPF_STATE,
	18
};


CMachineParameter const paraKnee=
{
	pt_byte,
	"Knee",
	"Knee soft/hard level",
	0x00,
	0x20,
	0xFF,
	MPF_STATE,
	0x10
};

CMachineParameter const *pParameters[]=
{
	&paraInputGain,
	&paraRatioLo,
	&paraRatioMid,
	&paraRatioHi,
	&paraDecayLo,
	&paraDecayMid,
	&paraDecayHi,
	&paraSplitOne,
	&paraSplitTwo,
	&paraKnee

};


CMachineAttribute const attrStereoLink =
{
	"Stereo Link",
	0,
	1,
	1
};


CMachineAttribute const *pAttributes[]=
{
	&attrStereoLink
};



#pragma pack(1)

class gvals
{
public:
	byte inputgain;
	byte ratiolo;
	byte ratiomid;
	byte ratiohi;
	byte decaylo;
	byte decaymid;
	byte decayhi;
	byte splitone;
	byte splittwo;
	byte knee;
};

class avals {
public:
	int stereolink;
};


#pragma pack()

CMachineInfo const MacInfo=
{
	MT_EFFECT,
	MI_VERSION,
	MIF_DOES_INPUT_MIXING,
	0,
	0,
	10,
	0,
	pParameters,
	1,
	pAttributes,
	"Oomek Masterizer",
	"Masterizer",
	"Oomek",
	"&About..."
};


class cLoPass
{
public:
	float z;

public:
	inline float Filter(float in, float Cutoff)
	{
		z = ((in - z) * Cutoff) + z;
		return z;// + 0.0001f;
	}

};




class cLimiter
{
public:
	float zD;
	float zA;

public:


inline float Decay(float in, float DecayVal)
	{
		in = fabsf(in);
		if (in - zD < 0.0f)
		{
			in = (zD + ((in - zD) * DecayVal));
		}
		zD = in;
		return in;
	}

inline float Attack(float in,float AttackVal)
	{
		if (in - zA > 0.0f)
		{
			in = (zA + ((in - zA) * AttackVal));
		}
		zA = in;
		return in;
	}
};


class cDelay
{
public:
	float DelayTable[MAXDELAY];
	int DelayInPos;
	int DelayPos;

public:
	float Delay(float in, int DelayInSamples)
	{
		DelayInPos&=MAXDELAY-1;
		DelayTable[DelayInPos] = in;
	
		DelayPos = DelayInPos - DelayInSamples;
		DelayPos&=MAXDELAY-1;

		++ DelayInPos;
		in = DelayTable[DelayPos];
		return in;
	}
};





class miex : public CMDKMachineInterfaceEx{};

class mi : public CMDKMachineInterface
{

public:
	mi();
	virtual ~mi();
	virtual void Tick();
	virtual void MDKInit(CMachineDataInput *const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
	virtual void Command(int const i);
	virtual void MDKSave(CMachineDataOutput *const po);
	virtual char const *DescribeValue(int const param, int const value);
	virtual CMDKMachineInterfaceEx *GetEx() {return &ex;}
	virtual void OutputModeChanged(bool stereo){};
	inline float KneeValue(float in);


private:
	miex ex;

	float SplitOne, SplitTwo;
	float DecayLo, DecayMid, DecayHi;
	float RatioLo, RatioMid, RatioHi;
	float InputGain;

	float inL, inLoL, inMidL, inHiL;
	float inR, inLoR, inMidR, inHiR;
	float LoLevelL, MidLevelL, HiLevelL, PostLevelL;
	float LoLevelR, MidLevelR, HiLevelR, PostLevelR;
	
	gvals gval;
	avals aval;

	cLoPass LoBandL;
	cLoPass MidBandL;
	cLoPass LoBandR;
	cLoPass MidBandR;

	cLimiter LoLimiterL;
	cLimiter LoLimiterR;
	cLimiter MidLimiterL;
	cLimiter MidLimiterR;
	cLimiter HiLimiterL;
	cLimiter HiLimiterR;

	cDelay LoDelayL;
	cDelay LoDelayR;
	cDelay MidDelayL;
	cDelay MidDelayR;
	cDelay HiDelayL;
	cDelay HiDelayR;


	cLimiter PostLimiterL;
	cLimiter PostLimiterR;

	cDelay PostDelayL;
	cDelay PostDelayR;

	cLoPass DcFilterL;
	cLoPass DcFilterR;

	float DcFrequency;

	float Knee;
	float KneeCoeff;


	float denormalBuf[64];
	int denormalPos;

	int ModeRelease;
	float AntiDenormal;

};



mi::mi()
{
	GlobalVals=&gval;
	AttrVals = (int *)&aval;
}
mi::~mi(){}





void mi::MDKInit(CMachineDataInput *const pi)
{
	SetOutputMode(true);
	int i;

	LoLevelL = 1.0f;
	MidLevelL = 1.0f;
	HiLevelL = 1.0f;

	LoLevelR = 1.0f;
	MidLevelR = 1.0f;
	HiLevelR = 1.0f;

	for (i=0; i<128; i++)
	{
		LoDelayL.DelayTable[i] = 0.0f;
		LoDelayR.DelayTable[i] = 0.0f;
		MidDelayL.DelayTable[i] = 0.0f;
		MidDelayR.DelayTable[i] = 0.0f;
		HiDelayL.DelayTable[i] = 0.0f;
		HiDelayR.DelayTable[i] = 0.0f;
		PostDelayL.DelayTable[i] = 0.0f;
		PostDelayR.DelayTable[i] = 0.0f;
	}

	LoLimiterL.zA = 0.0f;
	LoLimiterL.zD = 0.0f;
	LoLimiterR.zA = 0.0f;
	LoLimiterR.zD = 0.0f;

	MidLimiterL.zA = 0.0f;
	MidLimiterL.zD = 0.0f;
	MidLimiterR.zA = 0.0f;
	MidLimiterR.zD = 0.0f;

	HiLimiterL.zA = 0.0f;
	HiLimiterL.zD = 0.0f;
	HiLimiterR.zA = 0.0f;
	HiLimiterR.zD = 0.0f;

	PostLimiterL.zA = 0.0f;
	PostLimiterL.zD = 0.0f;
	PostLimiterR.zA = 0.0f;
	PostLimiterR.zD = 0.0f;

	LoBandL.z = 0.0f;
	MidBandL.z = 0.0f;
	LoBandR.z = 0.0f;
	MidBandR.z = 0.0f;

	DcFilterL.z = 0.0f;
	DcFilterR.z = 0.0f;

	DcFrequency = (float)(1.0 * ((PI * 2.0) / pMasterInfo->SamplesPerSec)); // cut at 1 Hz

	for (i = 0; i < 64; i++)
	{
		denormalBuf[i] = ((float)rand() / RAND_MAX * 1.0E-20f );// + (1.0E-20f * 2.0f);
	}
	denormalPos = 0;


	ModeRelease = 512;
	AntiDenormal = ANTIDENORMAL * 2.0f;


}






void mi::MDKSave(CMachineDataOutput *const po){}



void mi::Tick()
{
	if (gval.inputgain != 0xFF && gval.inputgain !=0x00) InputGain = (((float)(gval.inputgain * gval.inputgain) / (float)(240 * 240)) * 16.0f); // (1 / 240)
	if (gval.inputgain == 0) InputGain = 0;

	if (gval.decaylo != 0xFF) DecayLo = 2.0f / (((float)gval.decaylo * 0.001f * pMasterInfo->SamplesPerSec * 4.0f) + 1.0f);
	if (gval.decaymid != 0xFF) DecayMid = 2.0f / (((float)gval.decaymid * 0.001f * pMasterInfo->SamplesPerSec * 4.0f) + 1.0f);
	if (gval.decayhi != 0xFF) DecayHi = 2.0f / (((float)gval.decayhi * 0.001f * pMasterInfo->SamplesPerSec * 4.0f) + 1.0f);
	
	if (gval.ratiolo != 0xFF) RatioLo = (float)gval.ratiolo / 120.0f;
	if (gval.ratiomid != 0xFF) RatioMid = (float)gval.ratiomid / 120.0f;
	if (gval.ratiohi != 0xFF) RatioHi = (float)gval.ratiohi / 120.0f;
	
	if (gval.splitone != 0xFF) SplitOne = (float)gval.splitone / 960.0f;
	if (gval.splittwo != 0xFF) SplitTwo = (float)(gval.splittwo + 35) / 240.0f;//* 0.0041666666667f;
	
	if (gval.knee != 0xFF)
	{
		Knee = (float) (pow((gval.knee / 32.0) , 2.0) * 3.75 + 0.25);
		KneeCoeff = (0.5f / Knee) * (0.5f / Knee);
	}

}




bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	if (mode==WM_WRITE) return false;
	if (mode==WM_NOIO) return false;
	if (mode==WM_READ) return true;

	do
	{
		*psamples = 0;
		psamples ++;
	} while (--numsamples);

	return false;
}



inline float mi::KneeValue(float in)
{

	//	in = (float)( (exp((in * Knee) - Knee - 1.0f)) * KneeCoeff) + 1.0f;

	if (in < (1.0f + KneeCoeff)) {
		if (in < (1.0f - KneeCoeff)) in = 1.0f;
		else in = ((in + KneeCoeff - 1.0f) * Knee) * ((in + KneeCoeff - 1.0f) * Knee) + 1.0f;
//		else in = (float)fpow(((in + KneeCoeff - 1.0f) * Knee), 2) + 1.0f;
	}
	return in;
}


bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if (mode==WM_WRITE && ModeRelease <= 0) return false;
	if (mode==WM_NOIO) return false;
	if (mode==WM_READ) return true;
	if (mode==WM_READWRITE) ModeRelease = 512;

	if (InputGain == 0) return false;
	do
	{

		if (mode==WM_WRITE) *psamples = 0.0f;
		--ModeRelease;

////////////// KANAL Lewy //////////////////


//		if (*psamples > (32768.0f * 8.0f) || *psamples < (-32768.0f * 8.0f))
//		MessageBox(NULL,"Underflow!!!","pimpom",MB_OK);


		if (*psamples > (32768.0f * 32.0f)) *psamples = (32768.0f * 32.0f);
		if (*psamples < (-32768.0f * 32.0f)) *psamples = (-32768.0f * 32.0f);

		*psamples *= 0.000030517578125f; // div 32768.f
		
		inL = *psamples + denormalBuf[denormalPos]; //Denormal eliminator;

//		inL = inL - DcFilterL.Filter(inL, DcFrequency); // DC Filter


		inL *= InputGain + denormalBuf[denormalPos]; //Denormal eliminator;

		inLoL = LoBandL.Filter(inL, SplitOne);
		LoLevelL = inLoL + denormalBuf[denormalPos]; //Denormal eliminator;
		LoLevelL = LoLimiterL.Decay(LoLevelL, DecayLo);
		LoLevelL = LoLimiterL.Attack(LoLevelL, 0.0443459f); //1 ms  przy 44100 - poprawiæ

//		if (LoLevelL < 1.0f) LoLevelL = 1.0f;
		LoLevelL = KneeValue(LoLevelL);
		LoLevelL = (RatioLo) / LoLevelL; // BQP LoLevelL = (RatioLo * 32768.0f) / LoLevelL;



		inL = inL - inLoL + denormalBuf[denormalPos]; //Denormal eliminator;;


//////////

		inMidL = MidBandL.Filter(inL, SplitTwo);
		
		MidLevelL = inMidL + denormalBuf[denormalPos]; //Denormal eliminator;;
		MidLevelL = MidLimiterL.Decay(MidLevelL, DecayMid);
		MidLevelL = MidLimiterL.Attack(MidLevelL, 0.0443459f); //1 ms  przy 44100 - poprawiæ

//		if (MidLevelL < 1.0f) MidLevelL = 1.0f;
		MidLevelL = KneeValue(MidLevelL);
		MidLevelL = (RatioMid) / MidLevelL; // BQP MidLevelL = (RatioMid * 32768.0f) / MidLevelL;


		inHiL = inL - inMidL;

//////////


		HiLevelL = inHiL + denormalBuf[denormalPos]; //Denormal eliminator;;
		HiLevelL = HiLimiterL.Decay(HiLevelL, DecayHi);
		HiLevelL = HiLimiterL.Attack(HiLevelL, 0.0443459f); //1 ms  przy 44100 - poprawiæ

//		if (HiLevelL < 1.0f) HiLevelL = 1.0f;
		HiLevelL = KneeValue(HiLevelL);
		HiLevelL = (RatioHi) / HiLevelL; // BQP HiLevelL = (RatioHi * 32768.0f) / HiLevelL;



//////////



		inLoL = LoDelayL.Delay(inLoL, 88);
		inMidL = MidDelayL.Delay(inMidL, 88);
		inHiL = HiDelayL.Delay(inHiL, 88);

		*psamples = inLoL * LoLevelL;
		*psamples += (inMidL * MidLevelL);
		*psamples += (inHiL * HiLevelL);



//// Post Limit


		PostLevelL = *psamples + denormalBuf[denormalPos]; //Denormal eliminator;
		PostLevelL = PostLimiterL.Decay(PostLevelL, 0.00011f); //400 ms
		PostLevelL = PostLimiterL.Attack(PostLevelL, 0.0443459f); //1 ms  przy 44100 - poprawiæ

		if (PostLevelL < 1.0f) PostLevelL = 1.0f;
		PostLevelL = 1.0f / PostLevelL; // BQP PostLevelL = 29204.0f / PostLevelL;


		*psamples = PostDelayL.Delay(*psamples, 88);
		*psamples *= PostLevelL;
		*psamples *= 29204.0f;

////////////////

		psamples ++;



		if (mode==WM_WRITE) *psamples = 0.0f;

////////////// KANAL Prawy //////////////////
		
		if (*psamples > (32768.0f * 32.0f)) *psamples = (32768.0f * 32.0f);
		if (*psamples < (-32768.0f * 32.0f)) *psamples = (-32768.0f * 32.0f);

		*psamples *= 0.000030517578125f; // div 32768.f

		inR = *psamples + denormalBuf[denormalPos]; //Denormal eliminator;

//		inR = inR - DcFilterR.Filter(inR, DcFrequency); //DC Filter

		inR *= InputGain + denormalBuf[denormalPos]; //Denormal eliminator;

		inLoR = LoBandR.Filter(inR, SplitOne);
		
		LoLevelR = inLoR + denormalBuf[denormalPos]; //Denormal eliminator;
		LoLevelR = LoLimiterR.Decay(LoLevelR, DecayLo);
		LoLevelR = LoLimiterR.Attack(LoLevelR, 0.0443459f); //1 ms  przy 44100 - poprawiæ

		LoLevelR = KneeValue(LoLevelR);
		LoLevelR = (RatioLo) / LoLevelR;
//		if (LoLevelR > 1.0f) LoLevelR = 1.0f;

		inR = inR - inLoR + denormalBuf[denormalPos]; //Denormal eliminator;


//////////

		inMidR = MidBandR.Filter(inR, SplitTwo);
		
		MidLevelR = inMidR + denormalBuf[denormalPos]; //Denormal eliminator;
		MidLevelR = MidLimiterR.Decay(MidLevelR, DecayMid);
		MidLevelR = MidLimiterR.Attack(MidLevelR, 0.0443459f); //1 ms  przy 44100 - poprawiæ

		MidLevelR = KneeValue(MidLevelR);
		MidLevelR = (RatioMid) / MidLevelR;
//		if (MidLevelR > 1.0f) MidLevelR = 1.0f;

		inHiR = inR - inMidR;

//////////


		HiLevelR = inHiR  + denormalBuf[denormalPos]; //Denormal eliminator;
		HiLevelR = HiLimiterR.Decay(HiLevelR, DecayHi);
		HiLevelR = HiLimiterR.Attack(HiLevelR, 0.0443459f); //1 ms  przy 44100 - poprawiæ

		HiLevelR = KneeValue(HiLevelR);
		HiLevelR = (RatioHi) / HiLevelR;
//		if (HiLevelR > 1.0f) HiLevelR = 1.0f;


//////////



		inLoR = LoDelayR.Delay(inLoR, 88);
		inMidR = MidDelayR.Delay(inMidR, 88);
		inHiR = HiDelayR.Delay(inHiR, 88);

		*psamples = inLoR * LoLevelR;
		*psamples += (inMidR * MidLevelR);
		*psamples += (inHiR * HiLevelR);


//// Post Limit


		PostLevelR = *psamples + denormalBuf[denormalPos]; //Denormal eliminator;
		PostLevelR = PostLimiterR.Decay(PostLevelR, 0.00011f); //400 ms
		PostLevelR = PostLimiterR.Attack(PostLevelR, 0.0443459f); //1 ms  przy 44100 - poprawiæ


		if (PostLevelR < 1.0f) PostLevelR = 1.0f;
		PostLevelR = 1.0f / PostLevelR;

		*psamples = PostDelayR.Delay(*psamples, 88);
		*psamples *= PostLevelR;
		*psamples *= 29204.0f;

////////////////

		psamples ++;


	if (++denormalPos > 63) denormalPos = 0;

	} while (--numsamples);




	return true;
}

void mi::Command(int const i)
{
	switch(i)
	{
	case 0:
		pCB->MessageBox("\n\n  .: Oomek's Masterizer :. \n        a 3 band limiter\n              v1.0\n\n            ©2002\n    Radoslaw Dutkiewicz    \n        oomek@go2.pl         \n\n");
		break;
	default:
		break;
	}
}




char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[30];
	switch(param)
	{
	case 0:
		if (value == 0) sprintf(txt,"-inf dB");
		else sprintf(txt,"%.1f dB", 20*log10(((float)(value * value) / (float)(240 * 240)) * 16.0f));
		return txt;
		break;
	case 1:
	case 2:
	case 3:
		if (value == 0) sprintf(txt,"-inf dB");
		else sprintf(txt,"%.1f dB", 20*log10((float)(value) / (float)(120)));
		return txt;
		break;
	case 4:
	case 5:
	case 6:
		sprintf(txt,"%.0f ms", (float)value * 4);
		return txt;
		break;
	case 7:
		sprintf(txt,"%.0f Hz", ((float)value  / 960.0f) / (6.28319 / pMasterInfo->SamplesPerSec) + 1.0f);
		return txt;
		break;
	case 8:
		sprintf(txt,"%.0f Hz", ((float)(value + 35)  / 240.0f) / (6.28319 / pMasterInfo->SamplesPerSec));
		return txt;
		break;
	case 9:
		switch (value)
		{
		case 0: 
		case 1: 
		case 2:
		case 3:
			sprintf(txt,"s o f t -- 0%d -- h a r d", value);
			return txt;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			sprintf(txt,"o f t -- 0%d -- h a r d", value);
			return txt;
			break;
		case 8:
		case 9:
			sprintf(txt,"f t -- 0%d -- h a r d", value);
			return txt;
			break;
		case 10:
		case 11: 
			sprintf(txt,"f t -- %d -- h a r d", value);
			return txt;
			break;
		case 12:
		case 13:
		case 14:
		case 15:
			sprintf(txt,"t -- %d -- h a r d", value);
			return txt;
			break;
		case 16:
		case 17:
		case 18:
		case 19:
			sprintf(txt," -- %d -- h a r d", value);
			return txt;
			break;
		case 20:
		case 21: 
		case 22:
		case 23:
			sprintf(txt,"- %d -- h a r d", value);
			return txt;
			break;
		case 24:
		case 25:
		case 26:
		case 27:
			sprintf(txt," %d -- h a r d", value);
			return txt;
			break;
		case 28:
		case 29:
		case 30:
		case 31: 
		case 32:
			sprintf(txt,"%d -- h a r d", value);
			return txt;
			break;
		default:
			return NULL;
		}
		break;
	default:
		return NULL;
	}
}

#pragma optimize("",on)

DLL_EXPORTS



