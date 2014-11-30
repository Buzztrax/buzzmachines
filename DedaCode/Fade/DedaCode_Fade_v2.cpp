/*
#DEDACODE SLICER
#Code by Lustri Daniele
#
#Versione 1.0: 9-15-2003 MONOMACHINE: mono-input, mono-output
#
#Versione 1.1: aggiunto parametro Type. L'effetto può essere ora applicato in maniera lineare o 
#              o seguendo una funzione parabolica
#
#Versione 2.0 10-31-2003 STEREOMACHINE: stereo-input, stereo-output
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

CMachineParameter const paraLength = 
{
	pt_byte,
	"Length",
	"Length",
	2,
	128,
	1,
	MPF_STATE,
	8
};
CMachineParameter const paraFade = 
{
	pt_switch,
	"Fade",
	"Fade(0=in,1=out)",
	-1,
	-1,
	SWITCH_NO,
	MPF_STATE,
	SWITCH_ON
};
CMachineParameter const paraType = 
{
	pt_switch,
	"Type",
	"Type(0= linear,1= parabolic)",
	-1,
	-1,
	SWITCH_NO,
	MPF_STATE,
	0
};
CMachineParameter const paraUnit = 
{
	pt_byte,
	"Unit",
	"Unit(0= in tick,1=in sec )",
	0,
	1,
	2,
	MPF_STATE,
	0
};

CMachineParameter const paraTrigger = 
{
	pt_switch,
	"Trigger",
	"Trigger(1=on,0=off)",
	-1,
	-1,
	SWITCH_NO,
	0,
	0
};


CMachineParameter const *pParameters[]=
{
	&paraLength,
    &paraFade,
	&paraType,
	&paraUnit,
	&paraTrigger,

};

#pragma pack(1)

class gvals
{
public:
	byte length;
	byte fade;
	byte type;
	byte unit;
	byte trigger;

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
	"DedaCode Fade",				
	"Fade",								
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
	int Length;
	int Fade;
	int Type;
	int count;
	int Unit;
	bool Trigger;
	
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
	switch(param)
	{
	case 0:
		sprintf(txt,"%d ", value );
		return txt;
		break;
	
	case 1:
		if(value==0)
			return "in";
		else
			return "out";
		break;
    case 2:
		if(value==0)
			return "Linear";
		else
			return "Parabolic ";
		break;
	case 3:
		if(value==0)
			return "in tick";
		else
			return "in sec";
	    break;
	default:
		return NULL;
	}
}
void mi::MDKSave(CMachineDataOutput * const po) { }
void mi::MDKInit(CMachineDataInput * const pi)
{
	SetOutputMode( true );
	Fade=0;
	Length=8;
	Type=0;
	count=0;
	Unit=0;
	Trigger=false;
	

}

void mi::Tick()
{

	//La lettura dei parametri avviene solo quando trigger è attivo
	if(gval.trigger==true)
	{
        Trigger=true;
		count=1;	
		if(gval.length!=paraLength.NoValue)
			Length=(int)(gval.length);
		if(gval.fade!=paraFade.NoValue)
			Fade=(int)gval.fade;
		if(gval.type!=paraType.NoValue)
			Type=(int)gval.type;
		if(gval.unit!=paraUnit.NoValue)
			Unit=(int)gval.unit;

	}
	if(gval.trigger==false)
	{
		Trigger=false;
		count=0;
	}


}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if ((mode & WM_READ) == 0)  {  return false;  }
	
	if (mode == WM_READ)
		return true;

	double sq;
	double xq;
	double difq;
	float sample = 0;
	do{
		if(Unit==0)
			sample=pMasterInfo->SamplesPerTick*Length;
		else if(Unit==1)
			sample=pMasterInfo->SamplesPerSec*Length;
		
		if(Trigger==true){
		  float p1;
			if(Type==0){
				if(Fade==0){
					p1=(count*1.00f)/sample;
				}
				else{
					p1=(sample-count*1.00f)/sample;
				}
			}
			else{
				sq=(sample*sample);
				xq=(sample-count)*(sample-count);
				difq=((sample-count)*(sample-count));
				
				p1=1.00/sq;
				if(Fade==0){
					p1=p1*xq;
				}
				else{
					p1=p1*difq;
				}
			}
      *psamples++ *=p1;
      *psamples++ *=p1;
			count++;
		}
		
		if(count==sample)
			Trigger=false;
	 
  
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
		pCB->MessageBox("DedaCode Fade v.2 \n 9-15-2003 \n Code by: Lustri Daniele(DedaCode)\n nice_code@virgilio.it \n\n Beta Testing, Skin and Helpfile by: Rubèn Aiello (ladproject) \n http://ladproject.cjb.net/");
		break;
	default:
		break;
	}
}
#pragma optimize ("", on) 

DLL_EXPORTS

