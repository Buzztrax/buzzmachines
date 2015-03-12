/*
#DEDACODE Moving_Average
#Code by Lustri Daniele
#
#Versione 1.0: Il filtro esegue la media matematica di N campioni. N è l'unico parametro del 
#filtro. 
#               output(t)=1/N[input(t)+input(t-1)+input(t-2)+...+input(t-N)]
#
#Per rendere più efficiente il filtro ho dovuto apportare una piccola modifica all'algoritmo 
#in fase di realizzazione.
#Ho fatto in modo che ogni volta non venissero ricalcolate somme inutili già calcolate 
#nella precedente media. 
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "C:/audio/Buzz/Jeskola Buzz/Dev/mdk/mdk.h"
#include <windows.h>
#pragma optimize ("awy", on) 

CMachineParameter const paraLength = 
{
	pt_byte,
	"Sample ",
	"Sample ",
	1,
	120,
	0,
	MPF_STATE,
	8
};

CMachineParameter const *pParameters[]=
{
	&paraLength,
};

#pragma pack(1)

class gvals
{
public:
	byte length;
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
	1,										
	0,										
	pParameters,
	0,
	NULL,		
	"DedaCode Moving_Average",				
	"Moving_A",								
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
	float MatriceL[120];
	float MatriceR[120];
	int count;
	float sommaL;
	float sommaR;

	
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
		sprintf(txt,"%d samples", value );
		return txt;
		break;
	
	default:
		return NULL;
	}
}
void mi::MDKSave(CMachineDataOutput * const po) { }
void mi::MDKInit(CMachineDataInput * const pi)
{
	SetOutputMode( true );
	memset(MatriceL,0, 4*120);
	memset(MatriceR,0, 4*120);


}

void mi::Tick()
{ 	
	if(gval.length!=paraLength.NoValue)
		Length=(int)(gval.length);

}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if ((mode & WM_READ) == 0)  {  return false;  }
	
	if (mode == WM_READ)
		return true;
	float prosL=0;
	float prosR=0;


	do{
		if(count>=Length)
			count=0;
		MatriceL[count]=*psamples;
		MatriceR[count]=*(psamples+1);
		if(prosL==0)
		{	for(int i=0; i<Length; i++){
				sommaL=sommaL+MatriceL[i];
				sommaR=sommaR+MatriceR[i];
			}
		}
		else
		{
			sommaL=prosL+MatriceL[count];
			sommaR=prosR+MatriceR[count];
		}
		if(count>=Length-1){
			prosL=sommaL-MatriceL[0];
			prosR=sommaR-MatriceR[0];
		}
		else
		{
			prosL=sommaL-MatriceL[count+1];
			prosR=sommaR-MatriceR[count+1];
		}

		*psamples=sommaL/Length;
		psamples++;
		*psamples=sommaR/Length;
		psamples++;
		sommaL=0;
		sommaR=0;
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
	
	static char txt[16];
	switch (i)
	{
	case 0:
		MessageBox(NULL,"DedaCode Moving_Average v.1.0 \n 12-15-2003 \n Code by: Lustri Daniele(DedaCode)\n nice_code@virgilio.it \n http://dedacode.too.it \n\n\n Betatesting, skin, demotune and documentation by Rubèn Aiello (ladproject) \n http://ladproject.cjb.net","About DedaCode Moving_Average",MB_OK|MB_SYSTEMMODAL|MB_ICONINFORMATION);
		break;
	default:
		break;
	}
}
#pragma optimize ("", on) 

DLL_EXPORTS

