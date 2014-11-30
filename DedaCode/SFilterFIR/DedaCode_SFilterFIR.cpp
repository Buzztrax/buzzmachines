/*
#DedaCode SFilterFIR
#Code by Lustri Daniele
#
#Versione 1.0: Implementazione dell'algoritmo di convoluzione.
#
#
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

CMachineParameter const paraCoefa1 = 
{
	pt_byte,
	"Coef a0 ",
	"Coef a0 ",
	0,
	200,
	201,
	MPF_STATE,
	200
};

CMachineParameter const paraCoefa2 = 
{
	pt_byte,
	"Coef a1 ",
	"Coef a1 ",
	0,
	200,
	201,
	MPF_STATE,
	100
};

CMachineParameter const paraCoefa3 = 
{
	pt_byte,
	"Coef a2 ",
	"Coef a2 ",
	0,
	200,
	201,
	MPF_STATE,
	100
};

CMachineParameter const paraCoefa4 = 
{
	pt_byte,
	"Coef a3 ",
	"Coef a3 ",
	0,
	200,
	201,
	MPF_STATE,
	100
};

CMachineParameter const paraCoefa5 = 
{
	pt_byte,
	"Coef a4 ",
	"Coef a4 ",
	0,
	200,
	201,
	MPF_STATE,
	100
};

CMachineParameter const *pParameters[]=
{
	&paraCoefa1,
	&paraCoefa2,
	&paraCoefa3,
	&paraCoefa4,
	&paraCoefa5,

};

#pragma pack(1)

class gvals
{
public:
	byte coefa1;
	byte coefa2;
	byte coefa3;
	byte coefa4;
	byte coefa5;
};
class avals
{
public:
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								
	MI_VERSION,	
    MIF_DOES_INPUT_MIXING,							
	0,									
	0,					
	5,										
	0,										
	pParameters,
	0,
	NULL,		
	"DedaCode SFilterFIR",				
	"FIR",								
	"Daniele Lustri",							
	"About.." 
};
class miex : public CMDKMachineInterfaceEx { };

class mi : public CMDKMachineInterface
{
public:
	mi();
    virtual ~mi();
    virtual void Tick();
    virtual void MDKInit(CMachineDataInput * const pi);
    virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
    virtual void Command(int const i);
    virtual void MDKSave(CMachineDataOutput * const po);
    virtual char const *DescribeValue(int const param, int const value);
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
    virtual void OutputModeChanged(bool stereo) {}


public:
            miex ex;

private:
	float Coefa[5];
	float HistoryL[5];
	float HistoryR[5];
	int count;
	
private:
	
	gvals gval;

};



mi::mi()
{

	GlobalVals = &gval;
}

mi::~mi()
{
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	sprintf(txt,"%.2f ", ((value-100)*1.00f)/100.00f );
	return txt;

}
void mi::MDKSave(CMachineDataOutput * const po) { }
void mi::MDKInit(CMachineDataInput * const pi)
{
	SetOutputMode( true );

}

void mi::Tick()
{ 	
	if(gval.coefa1!=paraCoefa1.NoValue)
		Coefa[0]=(float)((100-gval.coefa1)/100.00f);

	if(gval.coefa2!=paraCoefa2.NoValue)
		Coefa[1]=(float)((100-gval.coefa2)/100.00f);

	if(gval.coefa3!=paraCoefa3.NoValue)
		Coefa[2]=(float)((100-gval.coefa3)/100.00f);
	
	if(gval.coefa4!=paraCoefa4.NoValue)
		Coefa[3]=(float)((100-gval.coefa4)/100.00f);

	if(gval.coefa5!=paraCoefa5.NoValue)
		Coefa[4]=(float)((100-gval.coefa5)/100.00f);


}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if ((mode & WM_READ) == 0)  {  return false;  }
	
	if (mode == WM_READ)
		return true;


	do{
		if(count>4)
			count=0;
		HistoryL[count]=*psamples;
		HistoryR[count]=*(psamples+1);
		*psamples=HistoryL[count%5]*Coefa[0]+HistoryL[(count+1)%5]*Coefa[1]+HistoryL[(count+2)%5]*Coefa[2]+HistoryL[(count+3)%5]*Coefa[3]+HistoryL[(count+4)%5]*Coefa[4];
		psamples++;
		*psamples=HistoryR[count%5]*Coefa[0]+HistoryR[(count+1)%5]*Coefa[1]+HistoryR[(count+2)%5]*Coefa[2]+HistoryR[(count+3)%5]*Coefa[3]+HistoryR[(count+4)%5]*Coefa[4];
		psamples++;
		count++;

	}while(--numsamples);
	return true;


}
bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
            return false;
}

void mi::Command(int const i)
{
	switch (i)
	{
	case 0:
		pCB->MessageBox("DedaCode SFilterFIR v.1.0 \n 12-15-2003 \n Code by: Lustri Daniele(DedaCode)\n nice_code@virgilio.it \n http://dedacode.too.it \n\n\n Betatesting, skin and documentation by Rubèn Aiello (ladproject) \n http://ladproject.cjb.net");
		break;
	default:
		break;
	}
}
#pragma optimize ("", on) 

DLL_EXPORTS

