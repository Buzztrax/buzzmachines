#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "MachineInterface.h"

CMachineParameter const paraLDelay = 
{ 
	pt_word,										// Type
	"Left Delay",									// Short Description
	"Left Delay (Ticks/16)",						// Long Description
	0x0001,											// MinValue	
	0x0400,											// MaxValue
	0x0401,											// NoValue
	MPF_STATE,										// Flags
	96,												// Default Value
};

CMachineParameter const paraLFB = 
{
	pt_word,
	"L Feedback",
	"Left Feedback",
	0x0000,
	0x0190,
	0x0191,
	MPF_STATE,
	0x00fa,
};

CMachineParameter const paraRDelay = 
{ 
	pt_word,										// Type
	"Right Delay",									// Short Description
	"Right Delay (Ticks/16)",						// Long Description
	0x0001,											// MinValue	
	0x0400,											// MaxValue
	0x0401,											// NoValue
	MPF_STATE,										// Flags
	96,												// Default Value
};

CMachineParameter const paraRFB = 
{
	pt_word,
	"R Feedback",
	"Right Feedback",
	0x0000,
	0x0190,
	0x0191,
	MPF_STATE,
	0x00fa,
};

CMachineParameter const paraDryAmt = 
{
	pt_byte,
	"Dry Thru",
	"Dry Through Amount",
	0,
	200,
	201,
	MPF_STATE,
	100,
};

CMachineParameter const paraXOver = 
{
	pt_word,
	"X-Delay Amt",
	"Cross Over Amount",
	0,
	400,
	401,
	MPF_STATE,
	250,
};

CMachineParameter const paraLIn = 
{
	pt_word,
	"Left In",
	"Left Input Amount",
	0,
	400,
	401,
	MPF_STATE,
	250,
};

CMachineParameter const paraRIn = 
{
	pt_word,
	"Right In",
	"Right Input Amount",
	0,
	400,
	401,
	MPF_STATE,
	250,
};

CMachineParameter const *pParameters[] =			// Array of Parameters
{ 
	&paraLDelay,
	&paraLFB,
	&paraRDelay,
	&paraRFB,
	&paraDryAmt,
	&paraXOver,
	&paraLIn,
	&paraRIn,
};

#pragma pack(1)										// Place to retrieve parameters	

class gvals
{
public:
	word ldelay;
	word lfb;
	word rdelay;
	word rfb;
	byte dry;
	word xover;
	word lin;
	word rin;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,								// always
	MIF_MONO_TO_STEREO,						// flags (MIF_MONO_TO_STEREO, MIF_PLAYS_WAVES)
	0,										// min tracks
	0,										// max tracks
	8,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,							// pointer to parameters
	0,										// numAttributes			
	NULL,									// pointer to attributes
	"FUK O-Delay",							// name
	"O-Delay",								// short name
	"Frequency UnKnown",					// author
	NULL
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);
	char const* DescribeValue(int const param, int const value); 



private:
	float dummy;
	int count, rate;
	float *lringbuff, *rringbuff;
	int lrbpos, rrbpos;
	int lrblen, rrblen;
	float leftFB, rightFB;
	float dryAmt;
	float XOverAmt;
	float leftIn, rightIn;

	gvals gval;

};


DLL_EXPORTS


mi::mi()
{
	GlobalVals = &gval;
	AttrVals = NULL;
	TrackVals = NULL;
}


mi::~mi()
{
	// Deallocate stuff
}


void mi::Init(CMachineDataInput * const pi)
{
	rate = pMasterInfo->SamplesPerSec;
	lringbuff = new float[44100];
	lrbpos = 0;
	lrblen = 44100;
	rringbuff = new float[44100];
	rrbpos = 0;
	rrblen = 44100;
}


void mi::Tick()
{
	if (gval.ldelay != paraLDelay.NoValue)
	{
		lrblen = gval.ldelay / 16.0 * pMasterInfo->SamplesPerTick;
		lringbuff = new float[lrblen];
		lrbpos = 0;
	}

	if (gval.lfb != paraLFB.NoValue)
		leftFB = (gval.lfb - 200) / 100.0;

	if (gval.rdelay != paraRDelay.NoValue)
	{
		rrblen = gval.rdelay / 16.0 * pMasterInfo->SamplesPerTick;
		rringbuff = new float[rrblen];
		rrbpos = 0;
	}

	if (gval.rfb != paraRFB.NoValue)
		rightFB = (gval.rfb - 200) / 100.0;

	if (gval.dry != paraDryAmt.NoValue)
		dryAmt = gval.dry / 100.0;

	if (gval.xover != paraXOver.NoValue)
		XOverAmt = (gval.xover - 200) / 100.0;

	if (gval.lin != paraLIn.NoValue)
	leftIn = (gval.lin - 200) / 100.0;

	if (gval.rin != paraRIn.NoValue)
	rightIn = (gval.rin - 200) / 100.0;

}


bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
	if (mode == WM_NOIO)
	{
		return false;
	}

	if (mode == WM_READ || mode == WM_WRITE)
	{	
		do
		{
			lringbuff[(lrbpos + lrblen - 1) % lrblen] = rringbuff[rrbpos] * XOverAmt + lringbuff[lrbpos] * leftFB;
			*pout = lringbuff[lrbpos];
			pout++;
			rringbuff[(rrbpos + rrblen - 1) % rrblen] = lringbuff[lrbpos] * XOverAmt + rringbuff[rrbpos] * rightFB;
			*pout = rringbuff[rrbpos];
			pout++;
			pin++;
			lrbpos = (lrbpos + 1) % lrblen;
			rrbpos = (rrbpos + 1) % rrblen;
		}
		while (--numsamples);
	}

	if (mode == WM_READWRITE)
	{
		do
		{
			lringbuff[(lrbpos + lrblen - 1) % lrblen] = rringbuff[rrbpos] * XOverAmt + lringbuff[lrbpos] * leftFB + *pin * leftIn;
			*pout = lringbuff[lrbpos] + dryAmt * *pin;
			pout++;
			rringbuff[(rrbpos + rrblen - 1) % rrblen] = lringbuff[lrbpos] * XOverAmt + rringbuff[rrbpos] * rightFB + *pin * rightIn;
			*pout = rringbuff[rrbpos] + dryAmt * *pin;
			pout++;
			pin++;
			lrbpos = (lrbpos + 1) % lrblen;
			rrbpos = (rrbpos + 1) % rrblen;
		}
		while (--numsamples);
	}

	return true;
}

char const* mi::DescribeValue(int const param, int const value)
{
	static char str[12];
	int temp1, temp2;

	if (param == 0 || param == 2)
	{
		temp1 = value/16;
		temp2 = value%16;
		sprintf (str,"%d %d/16 ticks", temp1, temp2);
	}

	if (param == 1 || param == 3 || param == 5 || param == 6 ||
		param == 7)
	{
		sprintf (str, "%d%%", value - 200);
	}

	if (param == 4)
	{
		sprintf (str, "%d%%", value);
	}

	return str;
}

