/*
#DEDACODE STEREOGAIN
#Code by Lustri Daniele
#
#Versione 1.0 9-14-2003: creata machine mono-input, stereo-output utilizzando machineInterface.h
#
#Versione 1.1 e v1.2: modifiche (ora non presenti!) per la regolazione del volume con più precisione 
#                     
#Versione 1.3 : su suggerimento di LadProject ho portato la regolazione del volume fino ad un
#               guadagno del 600%(prima arrivava al 200%)
#
#Versione 2.0 : STEREOMACHINE Sfruttando mdk.h : Stereo-input, Stereo-Output
#
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "C:/audio/Buzz/Jeskola Buzz/Dev/mdk/mdk.h"
#include <windows.h>
#pragma optimize ("awy", on) 

CMachineParameter const paraVolume = 
{
	pt_word,             //tipo 
	"Volume",            //nome
	"Volume",            //descrizione
	0,                   //Valore Minimo
	600,                 //Valore Massimo 
	0xffff,              //NoValue
	MPF_STATE,           //Flags 
	100                  //Valore Iniziale
};
CMachineParameter const paraRight = 
{
	pt_byte,
	"Right",
	"Right",
	0,
	100,
	0xff,
	MPF_STATE,
	100
};
CMachineParameter const paraLeft = 
{
	pt_byte,
	"Left",
	"Left",
	0,
	100,
	0xff,
	MPF_STATE,
	100
};
CMachineParameter const paraMute =
{
	pt_switch,
	"ByPass",
	"ByPass ",
	-1,
	-1,
	SWITCH_NO,
	MPF_STATE,
	SWITCH_OFF
};
CMachineParameter const *pParameters[]=
{
	&paraVolume,
    &paraRight,
	&paraLeft,
	&paraMute,

};

#pragma pack(1)

class gvals
{
public:
	word volume;
	byte right;
	byte left;
	byte mute;
};
class avals
{
public:
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,	                //Tipo Machine							
	MI_VERSION,	                
    MIF_DOES_INPUT_MIXING,	    //Flags						
	0,							//numero minimo di Traccie		
	0,						    //numero massimo di Traccie				
	4,	                        //numero Globale dei Parametri usati									
	0,							//numero di parametri per Traccia			
	pParameters,                //puntatore ai parametri
	0,                          //numero Attributi
	NULL,		                //puntatore agli attributi
	"DedaCode StereoGain",		//name		
	"Gain",								
	"Daniele Lustri",			//Autore				
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
		float Vol;
		float Right;
		float Left;
		bool Mute;

        gvals gval;
};

mi::mi() {  GlobalVals = &gval; }
mi::~mi() { }





char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];
	switch(param)
	{
	case 0:
		sprintf(txt,"%d %%", value);
		return txt;
		break;

	case 1:
		sprintf(txt,"%.1f%%", value*1.0f );
		return txt;
		break;

    case 2 :
	    sprintf(txt,"%.1f%%", value*1.0f );
		return txt;
		break;
	case 3:
		if(value==0)
			return "no";
		else
			return "yes";
		break;
	default:
		return NULL;
	}
}
void mi::MDKSave(CMachineDataOutput * const po) { }

void mi::MDKInit(CMachineDataInput * const pi)
{
	SetOutputMode( true );
	Vol=100;
	Left=100;
	Right=100;
	Mute=false;
	
}

void mi::Tick()
{
	if(gval.volume!=paraVolume.NoValue)
	   Vol=(float)(gval.volume*1.000f/100);
	
	if(gval.right!=paraRight.NoValue)
		Right=(float)(gval.right*1.000/100);
	
	if(gval.left!=paraLeft.NoValue)
		Left=(float)(gval.left*1.000/100);
	
	if(gval.mute!=paraMute.NoValue)
		Mute=(bool)gval.mute;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
		 if (mode==WM_WRITE)
			 return false;
         if (mode==WM_NOIO)
			 return false;
         if (mode==WM_READ)                        
			 return true;

	do{
		/* 
		X Amplificare: 
		moltiplicare input per costante superiore a 1.Per Esempio: moliplicare l'output per 2 
		vuol dire raddoppiare l'ampiezza del segnale.
		Nota: L'orecchio però percepisce la variazione di ampiezza in maniera LINEARE solo se
		ho un'incremento ESPONENZIALE del volume.
		
		Se mute=true bypassa il segnale: segale input=segnale output
		
		La plug-in è Stereo quindi i campioni che vengono passati alla funzione MDKWorkStereo
		comprendono quelli del canale destro e quelli del canale sinistro, E sono alternati..
		il primo campione corrisponde al canale sinistro all'istante t(0), il secondo campione
		canale destro t(0), terzo campione sinistro t(1), quarto campione destro t(1) e così via*/

		if(Mute!=true)
		{	
			*psamples++ =(float)((Vol* *psamples)*Left);
			*psamples++ =(float)((Vol* *psamples)*Right);
			
		}
		  

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
		MessageBox(NULL,"DedaCode StereoGain v.2 \n 9-14-2003 \n Code by: Lustri Daniele (DedaCode) \n nice_code@virgilio.it \n\n Beta Testing, Skin and Helpfile by: Rubèn Aiello (ladproject) \n http://ladproject.cjb.net/","About DedaCode StereoGain ",MB_OK|MB_SYSTEMMODAL|MB_ICONINFORMATION);
		break;
	default:
		break;
	}
}

#pragma optimize ("", on) 

DLL_EXPORTS
