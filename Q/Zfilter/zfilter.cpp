
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>

double const Silence = log(1.0 / 32768);


CMachineParameter const parab0 = 
{ 
	pt_word,										// type
	"b0",
	"Order-0 mathematical term (~DryThru)",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0xc000,
};
CMachineParameter const paraa1 = 
{ 
	pt_word,										// type
	"a1 % 1+a2",
	"First order mathematical term (~Cutoff)",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0xfffe,
};
CMachineParameter const parab1 = 
{ 
	pt_word,										// type
	"b1",
	"First order mathematical term (~filter mode)",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x7fff,
};
CMachineParameter const paraa2 = 
{ 
	pt_word,										// type
	"a2",
	"Second order mathematical term",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x7fff,
};
CMachineParameter const parab2 = 
{ 
	pt_word,										// type
	"b2",
	"Second order mathematical term",						// description
	0,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x4000,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&parab0,
	&paraa1,
	&parab1,
	&paraa2,
	&parab2,
	
};

#pragma pack(1)		

class gvals
{
public:
	word b0,a1,b1,a2,b2;
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
	0,									// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Q Zfilter (Debug build)",		// name
#else
	"Q Zfilter",					// name
#endif
	"Z",									// short name
	"Q",						// author
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
	virtual char const *DescribeValue(int const param, int const value);

private:
	float b0,a1,b1,a2,b2;
	
	gvals gval;
	float x1,x2,y1,y2;
	
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
}

void mi::Init(CMachineDataInput * const pi)
{
	x1=x2=y1=y2=0;
	a2=0;
	b2=0;
	b0=1;
	b1=-1;
	a1=(float)0x7FFF/0x8000;
} 

void mi::Tick()
{
	
	if (gval.b0 != parab0.NoValue)
		b0= (float)(gval.b0-0x7FFF)/0x4000;
	if (gval.a1 != paraa1.NoValue)
		a1= (float)(gval.a1-0x7FFF)*(1+a2)/0x8000;
	if (gval.b1 != parab1.NoValue)
		b1= (float)(gval.b1-0x7FFF)/0x4000;
	if (gval.a2 != paraa2.NoValue)
	{
		a1= a1*(float)gval.a2/0x8000/(1+a2);
		a2= (float)(gval.a2-0x7FFF)/0x8000;
	}
		
	if (gval.b2 != parab2.NoValue)
		b2= (float)(gval.b2-0x7FFF)/0x4000;
		
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 0:
		sprintf(txt, "%.4f", b0 );
		break;
	case 1:
		sprintf(txt, "%.4f", a1/(1+a2) );
		break;
	case 2:
		sprintf(txt, "%.4f", b1 );
		break;
	case 3:
		sprintf(txt, "%.4f", a2);
		break;
	case 4:
		sprintf(txt, "%.4f", b2 );
		break;
	default:
		return NULL;
	}

	return txt;
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{

	if (mode == WM_NOIO)
		return false;
	


	do 
	{
		float x0=*psamples;

		if (mode & WM_WRITE)
		{
			*psamples=b0*x0+b1*x1+b2*x2-a1*y1-a2*y2;
			y2=y1;
			y1=*psamples;
		}
		psamples++;
		x2=x1;
		if (mode & WM_READ)
			x1=x0;
		else
			x1=0;	
		
	} while(--numsamples);

	
	if (mode & WM_WRITE)
		return true;
	return false;
}


