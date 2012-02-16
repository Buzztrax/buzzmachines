/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Includes

	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <assert.h>
	#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buzz Includes

	#include <MachineInterface.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines

	#define machine_name	"11-CSI v1.0"
	#define machine_short	"11-CSI "

	#define max_clen		128

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Machine Defines

	#define mpar_clen		"Samples","Samples"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Machine Parameters

	CMachineParameter const para_clen = {pt_byte,mpar_clen,  0,127,0xFF,MPF_STATE,  10};

	CMachineParameter const *pParameters[]	= {&para_clen};

////////////////////////////////////////////////////////////////////////////////

	#pragma pack(1)		

	class gvals
	{ 
		public:
			byte	clen;
	};

	#pragma pack()

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Machine Info

	CMachineInfo const MacInfo = 
	{
		MT_EFFECT,			// type
		MI_VERSION,	
		0,					// flags
		0,					// min tracks
		0,					// max tracks
		1,					// numGlobalParameters
		0,					// numTrackParameters
		pParameters,
		0,
		NULL,
		machine_name,
		machine_short,
		"Holger Zwar",
		"About"
	};

	class mi;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Machine Interface

	class mi : public CMachineInterface
	{
		public:
			mi();
			virtual ~mi();
			virtual void Init(CMachineDataInput * const pi);
			virtual void Tick();
			virtual bool Work(float *psamples, int numsamples, int const mode);
			virtual char const *DescribeValue(int const param, int const value);
			virtual void Command(int const i);

		public:
			gvals	gval;

			int		clen;			// length of cubic-arrays
			int		sc;				// sample-counter
			float	p1,p2,p3,p4;	// the last 4 samples, interpolation between p2 und p3
			float	c1[max_clen],c2[max_clen],c3[max_clen],c4[max_clen];	// the cubic-arrays

	};

	DLL_EXPORTS

	mi::mi()
	{
		GlobalVals	= &gval;
	}

	mi::~mi()
	{
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	mi Init

	void mi::Init(CMachineDataInput * const pi)
	{
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	mi Command

	void mi::Command(int const i)
	{
		pCB->MessageBox("coded in 2000 by z.war\n\nvisit www.zwar.de\n\nthanx to oskari tammelin");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	mi DescribeValue

	char const *mi::DescribeValue(int const param, int const value)
	{
		
		static char txt[16];

		switch(param)
		{
		case 0:
			if (value==0)
				sprintf(txt,"Off");
			else
				sprintf(txt,"%d (%X)",value,value);
			break;
		default:
			return NULL;
		}

		return txt;

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	mi Tick

	void mi::Tick()
	{

		if (gval.clen != para_clen.NoValue)
		{
			clen = gval.clen;
			for (int i=0; i<clen; i++)
			{
				float x = (float)(1.0 / clen * i);
				c1[i]	= (float)(-0.5 * x * x * x + x * x - 0.5 * x);
				c2[i]	= (float)(1.5 * x * x * x - 2.5 * x * x + 1);
				c3[i]	= (float)(-1.5 * x * x * x + 2 * x * x + 0.5 * x);
				c4[i]	= (float)(0.5 * x * x * x - 0.5 * x * x);
			}
		}
		
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	mi Work

bool mi::Work(float *psamples, int numsamples, int const mode)
{

	if (mode == WM_WRITE || mode == WM_NOIO)
		return false;

	if (mode == WM_READ )
		return true;		

	do 

	{

		if (clen>0)
		{
			sc++;			// every clen sample do: p1<p2<p3<p4<*psamples
			if (sc>=clen)
			{
				p1=p2; p2=p3; p3=p4; p4=*psamples;
				sc=0;
			}
				*psamples = c1[sc]*p1 + c2[sc]*p2 + c3[sc]*p3 + c4[sc]*p4;
		}

		psamples++;

	}
	while(--numsamples);


	return true;

}

