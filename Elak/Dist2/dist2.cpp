/**
* Elak Dist 2
* crappy forms of distortion
*
* But hey, I listen to M�torhead
*
**/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <MachineInterface.h>


CMachineParameter const paraScatterInterval = 
{ 
	pt_byte,										// type
	"ScatterInt",
	"Interval between scatters",						// description
	0,												// MinValue	
	254,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0x200,
};

CMachineParameter const paraScatterAmount = 
{ 
	pt_word,										// type
	"ScatterAmount",
	"The level of the scattering",						// description
	1,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x200,
};

CMachineParameter const paraKindOf = 
{ 
	pt_byte,										// type
	"KindOf",
	"The kind of scattering",						// description
	1,												// MinValue	
	3,												// MaxValue
	0xfe,											// NoValue
	MPF_STATE,										// Flags
	0x200,
};


CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraScatterInterval,
	&paraScatterAmount,
	&paraKindOf
	
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
	byte scatterInterval;
	word scatterAmount;
	byte kindOf;
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
	3,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Elak Dist 2(Debug build)",		// name
#else
	"Elak Dist 2",					// name
#endif
	"ElakDist2",									// short name
	"Johan Larsby",						// author
	NULL
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples,int numsamples, int const mode);

	

private:

	byte ScatterInterval;
	word ScatterAmount;
	byte  KindOf;

	word count;
	word mR;
	word mR2;

	gvals gval;
//	avals aval;

};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	AttrVals = NULL;
}

mi::~mi()
{
}

void mi::Init(CMachineDataInput * const pi)
{
	ScatterInterval = 1;
	ScatterAmount = 1;
	KindOf = 1;
	count = 0;


}

void mi::Tick()
{

		if (gval.scatterInterval != paraScatterInterval.NoValue)
		ScatterInterval = (gval.scatterInterval);

		if (gval.scatterAmount != paraScatterAmount.NoValue)
		ScatterAmount = (gval.scatterAmount);

		if (gval.kindOf!= paraKindOf.NoValue)
		KindOf = (gval.kindOf);
}



bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (mode == WM_WRITE) 
		return false;

	if (mode == WM_NOIO) //
		return false;

	if (mode == WM_READ)
		return true;



	if (KindOf == 1)
	{
		do 
		{
			double const s = *psamples;
	
				if (count > ScatterInterval)
				{
					count =0;
					mR = rand()%(ScatterAmount);
					*psamples = mR ;
				}
	
				count ++;

				if (s >= 65534)
					*psamples = 65534;
				else if (s <= -65534)
					*psamples = -65534;

				psamples++;

			
		} while(--numsamples);
	}


	else if (KindOf == 2)
	{
		do 
		{
			double const s = *psamples;
	
				if (count > ScatterInterval)
				{
					count =0;
					mR = rand()%(ScatterAmount);
					mR2 = rand()%(2);
					if (mR2==2)
					{
						*psamples = (*psamples + mR)/2 ;
					}
					else
					{
						*psamples = (*psamples - mR)/2 ;
					}


				}
	
				count ++;

				if (s >= 65534)
					*psamples = 65534;
				else if (s <= -65534)
					*psamples = -65534;

				psamples++;

			
		} while(--numsamples);

	}

	else if (KindOf == 3)
	{

				{
			double const s = *psamples;
	
				if (count < ScatterInterval/2)
				{
					count =0;
					mR = rand()%(ScatterAmount);
					if (s >= 0)
					{
						*psamples =- *psamples  ;
					}
					else
					{
						*psamples =+ *psamples ;		
					}

				}
	
				count ++;

				if (s >= 65534)
					*psamples = 65534;
				else if (s <= -65534)
					*psamples = -65534;

				psamples++;

			
		} while(--numsamples);

	}

	return true;
}


