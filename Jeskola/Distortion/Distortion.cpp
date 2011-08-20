
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>

CMachineParameter const paraThreshold = 
{ 
	pt_word,										// type
	"+threshold",
	"Positive threshold level",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x200,
};

CMachineParameter const paraClamp = 
{ 
	pt_word,										// type
	"+clamp",
	"Positive clamp level",							// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x1000
};

CMachineParameter const paraNegThreshold = 
{ 
	pt_word,										// type
	"-threshold",
	"Negative threshold level",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x200
};

CMachineParameter const paraNegClamp = 
{ 
	pt_word,										// type
	"-clamp",
	"Negative clamp level",							// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x1000
};

CMachineParameter const paraAmount = 
{ 
	pt_byte,										// type
	"Amount",
	"Amount",										// description
	0,												// MinValue	
	0x7f,  											// MaxValue
	0xff,   										// NoValue
	MPF_STATE,										// Flags
	0x7f
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraThreshold,
	&paraClamp,
	&paraNegThreshold,
	&paraNegClamp,
	&paraAmount
	
};

CMachineAttribute const attrSymmetric = 
{
	"Symmetric",
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
	word threshold;
	word clamp;
	word negthreshold;
	word negclamp;
	byte amount;
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
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	5,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	1,
	pAttributes,
#ifdef _DEBUG
	"Jeskola Distortion (Debug build)",		// name
#else
	"Jeskola Distortion",					// name
#endif
	"Dist",									// short name
	"Oskari Tammelin",						// author
	NULL
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

private:
			
	

private:
	float Threshold;
	float Clamp;
	float NegThreshold;
	float NegClamp;
	float Amount;
	
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
}

void mi::Init(CMachineDataInput * const pi)
{
	Threshold = 65536 * 16.0;
	Clamp = 65536 * 16.0;
	NegThreshold = -65536 * 16.0;
	NegClamp = -65536 * 16.0;
	Amount = 1.0;
}

void mi::Tick()
{
	if (gval.threshold != paraThreshold.NoValue)
		Threshold = (float)(gval.threshold * 16.0);

	if (gval.clamp != paraClamp.NoValue)
		Clamp = (float)(gval.clamp * 16.0);

	if (gval.negthreshold != paraNegThreshold.NoValue)
		NegThreshold = (float)(gval.negthreshold * -16.0);

	if (gval.negclamp != paraNegClamp.NoValue)
		NegClamp = (float)(gval.negclamp * -16.0);

	if (gval.amount != paraAmount.NoValue)
		Amount = (float)(gval.amount * (1.0 / 0x7f));
}


bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (mode == WM_WRITE || mode == WM_NOIO)
		return false;
	
	if (mode == WM_READ || Amount == 0)
		return true;
	
	double const drymix = 1.0 - Amount;
	float const clamp = (float)(Amount * Clamp);
	double const threshold = Threshold;
	float negclamp;
	double negthreshold; 

	if (aval.symmetric)
	{
		negthreshold = -threshold;
		negclamp = -clamp;
	}
	else
	{
		negthreshold = NegThreshold;
		negclamp = (float)(Amount * NegClamp);
	}


	if (drymix < 0.001)
	{
		do 
		{
			double const s = *psamples;

			if (s >= threshold)
				*psamples = clamp;
			else if (s <= negthreshold)
				*psamples = negclamp;

			psamples++;
			
		} while(--numsamples);
	}
	else
	{
		do 
		{
			double const s = *psamples;

			if (s >= threshold)
				*psamples = (float)(s * drymix + clamp);
			else if (s <= negthreshold)
				*psamples = (float)(s * drymix + negclamp);

			psamples++;
			
		} while(--numsamples);
	}

	return true;
}


