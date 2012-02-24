#include "../mdk/mdk.h"
#include <windows.h>
#include <math.h>
#include <float.h>
#include <assert.h>


#pragma optimize ("awy", on)



CMachineParameter const paraDrive=
{
	pt_byte,
	"Drive",
	"Drive Level",
	1,
	0xF0,
	0xFF,
	MPF_STATE,
	60
};

CMachineParameter const paraPreFilter=
{
	pt_byte,
	"Pre Cut",
	"Pre Filter Cutoff",
	1,
	0xF0,
	0xFF,
	MPF_STATE,
	0x80
};

CMachineParameter const paraPostFilter=
{
	pt_byte,
	"Post Cut",
	"Post Filter Cutoff",
	1,
	0xF0,
	0xFF,
	MPF_STATE,
	0x3C
};

CMachineParameter const paraAmount=
{
	pt_byte,
	"Amount",
	"Amount of Excitement",
	0,
	0xF0,
	0xFF,
	MPF_STATE,
	0x3C
};


CMachineParameter const *pParameters[]=
{
	&paraDrive,
	&paraPreFilter,
	&paraPostFilter,
	&paraAmount
};

CMachineAttribute const *pAttributes[]={NULL};


#pragma pack(1)

class gvals
{
public:
	byte drive;
	byte prefilter;
	byte postfilter;
	byte amount;
	byte quality;
};
class avals {};

#pragma pack()

CMachineInfo const MacInfo=
{
	MT_EFFECT,
	MI_VERSION,
	MIF_DOES_INPUT_MIXING,
	0,
	0,
	4,
	0,
	pParameters,
	0,
	pAttributes,
	"Oomek Exciter",
	"Exciter",
	"Oomek",
	"&About..."
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

	inline int f2i(double d);
	inline float mi::clip (float x, float a, float b);

public:
	miex ex;

	float precut, postcut;
	float drlv;
	float amlv;
	float tangensc[32768*32];
	float zL1,zR1,zL2,zR2;
	int poz;
	float inL, inR;
	int counter;
	gvals gval;
	avals aval;

	double x1, x2; //clipper variables
	float denormal;	//denormal treshold
	float denormalBuf[64];
	int denormalPos;



};



mi::mi()
{
	GlobalVals=&gval;
	AttrVals = (int *)&aval;
}
mi::~mi(){}



inline int mi::f2i(double d)
{
	const double magic = 6755399441055744.0;
	double tmp = (d-0.5) + magic;
	return *(int*) &tmp;
}


inline float mi::clip (float x, float a, float b) 
{ 
   x1 = fabs (x-a); 
   x2 = fabs (x-b); 
   x = (float)x1 + (a+b); 
   x -= (float)x2; 
   x *= 0.5f; 
   return (x); 
} 


void mi::MDKInit(CMachineDataInput *const pi)
{
	SetOutputMode(true);
	zL1=0.0f;
	zR1=0.0f;
	zL2=0.0f;
	zR2=0.0f;
	for (int k=0; k<32768*32; k++) tangensc[k]=(float)(atan((k-32768*16)/32768.0f*32)*32768.0f/(PI*0.5f));
	counter = 0;
	for (int i = 0; i < 64; i++)
	{
		denormalBuf[i] = (float)rand() / RAND_MAX * 1.0E-20f;
	}
	denormalPos = 0;





}

void mi::MDKSave(CMachineDataOutput *const po){}

void mi::Tick()
{
	if (gval.amount != 0xFF) amlv = ((float)gval.amount * 0.0041666666667f * 2); // (1 / 240)
	if (gval.prefilter != 0xFF) precut = (float)gval.prefilter * 0.0041666666667f;
	if (gval.postfilter != 0xFF) postcut = (float)gval.postfilter * 0.0041666666667f;
	if (gval.drive != 0xFF) drlv = (float)gval.drive * 0.0083333333333f;// (1 / 120)
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

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if (mode==WM_WRITE) return false;
	if (mode==WM_NOIO) return false;
	if (mode==WM_READ) return true;



	do
	{





////////////// KANAL PRAWY //////////////////

//		if (inR < 0.01f && inR > -0.01f){
//			DescribeValue(1, counter++);
//			pCB->ControlChange(pCB->GetThisMachine(), 1, 1, 1, counter);
//		}


		inR = *psamples;
		*psamples += denormalBuf[denormalPos];

		zR1 = ((*psamples - zR1) * precut) + zR1;			//HP pre
		*psamples = *psamples - zR1;						//HP pre
		*psamples *= drlv;

		poz = f2i(*psamples);
		if (poz > 524287 ) poz = 524287;
		if (poz < -524287 ) poz = -524287;

		*psamples = tangensc[poz + 524288]; // (32768 * 16)
		*psamples += denormalBuf[denormalPos];

		zR2 = ((*psamples - zR2) * postcut) + zR2;			//HP post
		*psamples = *psamples - zR2;						//HP post
		*psamples = *psamples * amlv + inR;

		psamples ++;



////////////// KANAL LEWY //////////////////


		inL = *psamples;
		*psamples += denormalBuf[denormalPos];

		zL1 = ((*psamples - zL1) * precut) + zL1;			//HP pre
		*psamples = *psamples - zL1;						//HP pre
		*psamples *= drlv;

		poz = f2i(*psamples);
		if (poz > 524287 ) poz = 524287;
		if (poz < -524287 ) poz = -524287;


		*psamples = tangensc[poz + 524288]; // (32768 * 16)
		*psamples += denormalBuf[denormalPos];

		zL2 = ((*psamples - zL2) * postcut) + zL2;			//HP post
		*psamples = *psamples - zL2;						//HP post
		*psamples = *psamples * amlv + inL ;

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
		MessageBox(NULL, "\n\nOomek's Exciter \n v1.1 (bugfixed)\n\nRadoslaw Dutkiewicz\nmailto:oomek@go2.pl\n\n","About Exciter",MB_OK|MB_SYSTEMMODAL);
		break;
	default:
		break;
	}
}
char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];
	switch(param)
	{
	case 0:
		sprintf(txt,"%.0f", (float)value);
		return txt;
		break;

	case 1:

	case 2:
		sprintf(txt,"%.0f Hz", ((float)value  * 0.0041666666667f) / (6.28319 / pMasterInfo->SamplesPerSec) + 1);
		return txt;
		break;

	case 3:
		if (value == 0) sprintf(txt,"-inf dB");
		else sprintf(txt,"%.1f dB", 20*log10(((float)value * 0.0041666666667f * 2.0f)));
		return txt;
		break;
	default:
		return NULL;
	}
}

#pragma optimize("",on)

DLL_EXPORTS



