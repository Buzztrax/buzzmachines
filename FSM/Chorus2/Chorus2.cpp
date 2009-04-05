/* $Id$
 *
 * buzzmachines
 * Copyright (C) 2007 Krzysztof Foltman  <kfoltman@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <windef.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>
#include "../dspchips/DSPChips.h"


double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		1
#define MAX_DELAY   32768
#define DELAY_MASK  32767

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraDryAmp = 
{ 
	pt_byte,										// type
	"Dry amp",
	"Dry amp [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraMinDelay = 
{ 
	pt_byte,										// type
	"Min Delay",
	"Minimum Delay",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	80
};

CMachineParameter const paraModDepth = 
{ 
	pt_byte,										// type
	"Mod Depth",
	"Modulation Depth",					// description
	0,												  // MinValue	
	240,										  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	90
};

CMachineParameter const paraModRate = 
{ 
	pt_byte,										// type
	"Mod Rate",
	"Modulation Rate",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	60
};

CMachineParameter const paraFatness = 
{ 
	pt_byte,										// type
	"Fatness",
	"Fatness",					// description
	1,												  // MinValue	
	6,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	3
};

CMachineParameter const paraPhasing = 
{ 
	pt_byte,										// type
	"Stereo",
	"Stereo phasing",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraWetAmp = 
{ 
	pt_byte,										// type
	"Wet amp",
	"Wet amp [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};


CMachineParameter const *pParameters[] = 
{ 
//	&paraDryAmp,
//  &paraWetAmp,
  &paraMinDelay,
  &paraModDepth,
  &paraModRate,
	&paraFatness,
  &paraPhasing,
//  &paraFeedbackLeftPan,
//	&paraFeedbackRightPan,
};

/*
CMachineAttribute const *pAttributes[] = 
{
};
*/

#pragma pack(1)

class avals
{
};

class gvals
{
public:
//	byte dryamp;
};

class tvals
{
public:
//  byte wetamp;
	byte mindelay;
	byte moddepth;
	byte modrate;
  byte fatness;
  byte phasing;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	MIF_DOES_INPUT_MIXING,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	0,										// numGlobalParameters
	5,										// numTrackParameters
	pParameters,
	0, // sizeof(pAttributes)/4,                    // 1 (numAttributes)
	NULL, //pAttributes,                 // pAttributes
#ifdef _DEBUG
	"BuzzFX Chorus2 (Debug build)",			// name
#else
	"BuzzFX Chorus2",
#endif
	"Chorus2",								// short name
	"Krzysztof Foltman",						// author
	NULL
};

class CTrack
{
public:
/*
  int Length;
	int Pos;
	int Unit;
  */
	byte mindelay;
	byte moddepth;
	byte modrate;
  byte fatness;
  byte phasing;
  float WetAmp;
};
 
class miex : public CMDKMachineInterfaceEx
{
};

class mi : public CMDKMachineInterface
{
public:
  miex ex;
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
  virtual void OutputModeChanged(bool stereo) {  }

	mi();
	virtual ~mi();

	virtual void MDKInit(CMachineDataInput * const pi);
  virtual void MDKSave(CMachineDataOutput * const po) {}
	virtual void Tick();
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);

	//virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);
  void PrepareTrack(int tno);
	void WorkTrackStereo(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  float *Buffer;
	int Phase,DeltaPhase;
  int Pos;
  int DelayUnit;
  float DryOut;
  
	int nEmptySamples;
	int numTracks;
	CTrack Tracks[MAX_TAPS];

	float dsv,dcv,psv,pcv;

private:

	avals aval;
	gvals gval;
	tvals tval[MAX_TAPS];
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = (int *)&aval;
#ifdef _DUMMY
  return;
#endif
  Buffer = new float[MAX_DELAY];
	Phase = 0;
	Pos = 0;
}

mi::~mi()
{
#ifdef _DUMMY
  return;
#endif
  delete []Buffer;
  numTracks=-1;
}

char const *mi::DescribeValue(int const param, int const value)
{
#ifdef _DUMMY
  return NULL;
#endif
	static char txt[16];

	switch(param)
	{
		/*
  case 0:
  case 1:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
		*/
	case 0:
		{
			float v=float((2+1102.5*value/240)/pMasterInfo->SamplesPerSec);
			sprintf(txt,"%0.2f ms",1000*v);
		}
		break;
	case 1:
		{
			float v=float(110.25*value/240/pMasterInfo->SamplesPerSec);
			sprintf(txt,"%0.2f ms",1000*v);
		}
		break;
	case 2:
		sprintf(txt,"%0.2f Hz",6.6*value/240.0);
		break;
	//case 3:
 //		sprintf(txt,"%0.1f %%",value*100/240.0);
	//	break;
	case 4:
		sprintf(txt,"%0.2f deg",value*180/240.0);
		break;

	default:
		return NULL;
	}

	return txt;
}

#define HANDLE_TPARAM(name) 	if (ptval->name!=255) pt->name=ptval->name;

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
/*  if (ptval->wetamp!=paraWetAmp.NoValue)
  {
    if (ptval->wetamp)
      pt->WetAmp=(float)pow(2.0,(ptval->wetamp/10.0-24.0)/6.0);
    else
      pt->WetAmp=0.0f;
  }
	*/
	HANDLE_TPARAM(mindelay)
	HANDLE_TPARAM(moddepth)
	HANDLE_TPARAM(modrate)
	HANDLE_TPARAM(fatness)
	HANDLE_TPARAM(phasing)
	//pt->Feedback=(float)pow(2.0,(pt->feedback/240.0-1)*2);
}

void mi::Tick()
{
#ifdef _DUMMY
  return;
#endif
	for (int c = 0; c < numTracks; c++)
		TickTrack(&Tracks[c], &tval[c]);
	/*
  if (gval.dryamp!=paraDryAmp.NoValue)
  {
    if (gval.dryamp)
      DryOut=(float)pow(2.0,(gval.dryamp/10.0-24.0)/6.0);
    else
      DryOut=0.0f;
  }
	*/
	int numTaps=Tracks[0].fatness;
	if (numTaps<1 || numTaps>6) numTaps=1;
	int StereoPhasing=(65536*Tracks[0].phasing/(240*numTaps))<<15;
	float HzFreq=float(6.6*Tracks[0].modrate/240.0);
	DeltaPhase=f2i(HzFreq*65536.0*65536.0/pMasterInfo->SamplesPerSec);
	dsv=(float)sin(DeltaPhase*PI/(1<<31));
	dcv=(float)cos(DeltaPhase*PI/(1<<31));
	psv=(float)sin(StereoPhasing*PI/(1<<31));
	pcv=(float)cos(StereoPhasing*PI/(1<<31));
}



void mi::MDKInit(CMachineDataInput * const pi)
{
#ifdef _DUMMY
  return;
#endif
	numTracks = 1;

  for (int c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;

  SetOutputMode(true);
}

/*
void mi::AttributesChanged()
{
	MaxDelay = (int)(pMasterInfo->SamplesPerSec * (aval.maxdelay / 1000.0));
	for (int c = 0; c < numTracks; c++)
		InitTrack(c);
}
    */


void mi::SetNumTracks(int const n)
{
#ifdef _DUMMY
  return;
#endif
  if (n>MAX_TAPS)
    pCB->MessageBox("Oskari, don't set numTracks above the upper limit");
  if (numTracks < n)
	{
		for (int c = numTracks; c < n; c++)
			InitTrack(c);
	}
	else if (n < numTracks)
	{
		for (int c = n; c < numTracks; c++)
			ResetTrack(c);
	
	}
	numTracks = n;
}


void mi::InitTrack(int const i)
{
#ifdef _DUMMY
  return;
#endif
}

void mi::ResetTrack(int const i)
{
}

void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
}

void mi::PrepareTrack(int tno)
{
}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
#ifdef _DUMMY
  return false;
#endif
	float MinDelay=float(2+1120.5*Tracks[0].mindelay/240);
	float ModDepth=float(110.25*Tracks[0].moddepth/240);
	float AvgDelay=MinDelay+ModDepth;
	if (mode & WM_READ)
  {
    nEmptySamples=0;
  }
  else
  {
		if (nEmptySamples>(AvgDelay+ModDepth))
			return false;

    if (mode&WM_WRITE)
			for (int i=0; i<2*numsamples; i++)
				psamples[i]=0.0;
    nEmptySamples+=numsamples;
  }
	float *pbuf=(mode&WM_WRITE)?psamples:pCB->GetAuxBuffer();

	float tsv=(float)sin(2*PI/6);
	float tcv=(float)cos(2*PI/6);
	float sv=(float)sin(Phase*PI/(1<<31));
	float cv=(float)cos(Phase*PI/(1<<31));
	int numTaps=Tracks[0].fatness;
	float amp2=float(numTaps/2.5);
	float amp=float(1.0/(amp2+numTaps));
	for (int i=0; i<numsamples; i++)
	{
		float DataL=0.f, DataR=0.f;
		float lsv=sv, lcv=cv;

		for (int j=0; j<numTaps; j++)
		{
			{
				float sv1=lsv;
				float delay=AvgDelay+ModDepth*sv1;
				int intpos=f2i(delay);
				float fracpos=delay-intpos;			
				int bufpos=Pos-intpos*2;
				DataL+=Buffer[(bufpos+2)&DELAY_MASK]+fracpos*(Buffer[(bufpos)&DELAY_MASK]-Buffer[(bufpos+2)&DELAY_MASK]);
			}
			{
				float sv2=lsv*pcv+lcv*psv;
				float delay=AvgDelay+ModDepth*sv2;
				int intpos=f2i(delay);
				float fracpos=delay-intpos;			
				int bufpos=Pos-intpos*2+1;
				DataR+=Buffer[(bufpos+2)&DELAY_MASK]+fracpos*(Buffer[(bufpos)&DELAY_MASK]-Buffer[(bufpos+2)&DELAY_MASK]);
			}
			float nsv=lsv*tcv+lcv*tsv, ncv=tcv*lcv-lsv*tsv;
			lsv=nsv, lcv=ncv;
		}
		//DataL/=j;
		//DataR/=j;

		Buffer[Pos]=psamples[i*2];
		Buffer[Pos+1]=psamples[i*2+1];
		pbuf[i*2]=(amp2*psamples[i*2]+DataL)*amp;
		pbuf[i*2+1]=(amp2*psamples[i*2+1]+DataR)*amp;
		Pos=(Pos+2)&DELAY_MASK;
		float nsv=sv*dcv+cv*dsv;
		float ncv=cv*dcv-sv*dsv;

		sv=nsv, cv=ncv;
	}
	Phase+=numsamples*DeltaPhase;

  if (!(mode&WM_WRITE))
    return false;
  int *pint=(int *)psamples;
  for (int i=0; i<2*numsamples; i++)
    if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
      return true;
	return false;
}

