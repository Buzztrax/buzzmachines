
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>

double const Silence = log(1.0 / 32768);


CMachineParameter const paraDeltaT = 
{ 
	pt_word,										// type
	"Delta T",
	"Temps du premier rebond",						// description
	1,												// MinValue	
	0xfffe,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x1000,
};

CMachineParameter const paraAttenuation = 
{ 
	pt_byte,										// type
	"Attenuation",
	"Attenuation a chaque rebond",							// description
	0,												// MinValue	
	0xc0,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0x80
};


CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraDeltaT,
	&paraAttenuation,
	
};

#pragma pack(1)		

class gvals
{
public:
	word deltaT;
	byte attenuation;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	2,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Q-Rebond (Debug build)",		// name
#else
	"Q-Rebond",					// name
#endif
	"Rebond",									// short name
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
	int Attenuation;
	int DeltaT;
	
	gvals gval;
	float *buffer;
	int buffcount;
	
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
delete (buffer);
}

void mi::Init(CMachineDataInput * const pi)
{
	DeltaT = (int) (0x1000 * 44100 *2.0 /0xFFFE);
	Attenuation = 0x80;
	buffcount=0;
	buffer = new float[ 8*44100];
	memset (buffer,0,8*44100);
} 

void mi::Tick()
{
	
	if (gval.attenuation != paraAttenuation.NoValue)
		Attenuation = gval.attenuation;
		

	if (gval.deltaT != paraDeltaT.NoValue)
		DeltaT = (int) (gval.deltaT * 2.0 / 0xFFFE *44100);
		
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 0:
		sprintf(txt, "%.1f ticks", (float)DeltaT / pMasterInfo->SamplesPerTick);
		break;
	case 1:		
		sprintf(txt, "%.1f%%", Attenuation * 100.0 / 256);
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
		int j;
		float s=0;

		if (mode & WM_READ)
			s=buffer[buffcount]=*psamples;
		else
			buffer[buffcount]=0;
		if (--buffcount<0) buffcount = 8*44100-1;
		if (mode & WM_WRITE)
		{
			float p=1;

			j=0;
			do
			{
				p=p*(float)(Attenuation / 256.0);
				j = DeltaT+Attenuation*j/256;
				s+=buffer[(buffcount+j+1) % (8*44100)]* p;
			} while (p*DeltaT > 1 && p>Silence);
			*psamples=s;
		}
		psamples++;

	} while(--numsamples);

	
	if (mode & WM_WRITE)
		return true;
	else	return false;
}


