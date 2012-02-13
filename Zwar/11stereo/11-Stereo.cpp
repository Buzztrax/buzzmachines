////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "d:\buzz\dev\MachineInterface.h"

////////////////////////////////////////////////////////////////////////////////

#define	hz				"Holger Zwar"

////////////////////////////////////////////////////////////////////////////////

CMachineParameter const para_mix = 
{ 
	pt_byte,									// type
	"Left : Right",
	"Left : Right",								// description
	0,											// MinValue	
	0x7E, 										// MaxValue
	0xFF,  										// NoValue
	MPF_STATE,									// Flags
	0x40
};

CMachineParameter const para_tic = 
{ 
	pt_byte,									// type
	"Inertia",
	"Inertia",									// description
	1,											// MinValue	
	0x80, 										// MaxValue
	0xFF,  										// NoValue
	MPF_STATE,									// Flags
	0x10
};

////////////////////////////////////////////////////////////////////////////////

CMachineParameter const *pParameters[] = 
{ 
	&para_mix,
	&para_tic,
};

////////////////////////////////////////////////////////////////////////////////

#pragma pack(1)		

class gvals
{
public:
	byte mix;
	byte tic;
};

#pragma pack()

////////////////////////////////////////////////////////////////////////////////

CMachineInfo const MacInfo = 
{
	MT_EFFECT,											// type
	MI_VERSION,	
	MIF_MONO_TO_STEREO,									// flags
	0,													// min tracks
	0,													// max tracks
	2,													// numGlobalParameters
	0,													// numTrackParameters
	pParameters,
	0,
	NULL,

#ifdef _DEBUG
	"11-Stereo v1.0 (Debug build)",
#else
	"11-Stereo v1.0",
#endif

	"11-Stereo ",									// short name
	hz,												// author
	"About"
};

////////////////////////////////////////////////////////////////////////////////

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);
	virtual char const *DescribeValue(int const param, int const value);
	virtual void Command(int const i);

private:

	float	mix;	// destination
	float	cur;	// current pos
	float	inc;	// increment
	byte	tic;

	gvals	gval;

};

////////////////////////////////////////////////////////////////////////////////

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
}

////////////////////////////////////////////////////////////////////////////////

void mi::Init(CMachineDataInput * const pi)
{
	cur = 0.5;
}

////////////////////////////////////////////////////////////////////////////////

void mi::Command(int const i)
{

	pCB->MessageBox("coded in 2000 by z.war\n\nvisit www.zwar.de\n\nthanx to oskari tammelin");

}

////////////////////////////////////////////////////////////////////////////////

char const *mi::DescribeValue(int const param, int const value)
{
	
	static char txt[16];

	switch(param)
	{
	case 0:		// Mix
		sprintf(txt, "%.1f : %.1f",(float)((126-value)*(100.0/126)),(float)((value)*(100.0/126)));
		break;
	case 1:		// Inertia
		sprintf(txt, "%d Ticks",(int)value);
		break;
	default:
		return NULL;
	}

	return txt;

}

////////////////////////////////////////////////////////////////////////////////

void mi::Tick()
{

	if (gval.tic != para_tic.NoValue)
		tic = gval.tic;
	
	if (gval.mix != para_mix.NoValue)
	{
		mix = (float)(gval.mix*(1.0/126));
		inc = (float)((mix-cur)/(tic*pMasterInfo->SamplesPerTick));
	}
	
}

////////////////////////////////////////////////////////////////////////////////

bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{

	if (mode == WM_WRITE || mode == WM_NOIO)
		return false;

	if (mode == WM_READ )
		return true;		

	do 

	{

		cur+=inc;
		if (cur<=0 || cur>=1)
			cur=mix;

		float in = *pin++;

		float ol = in * (1-cur);
		float or = in * cur;

		*pout++ = ol;
		*pout++ = or;

	}
	while(--numsamples);

	return true;

}

////////////////////////////////////////////////////////////////////////////////