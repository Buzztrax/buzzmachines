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

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		8
// 200 ms przy 44100 Hz
#define MAX_DELAY    8192
#define DELAY_MASK   8191

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraDryOut = 
{ 
	pt_byte,										// type
	"Dry out",
	"Dry out [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraMinDelay = 
{ 
	pt_byte,									// type
	"Minimum delay",
	"Minimum delay [ms]", 		// description
	1,												// MinValue	
	100,										  // MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	20,                       // default
};

CMachineParameter const paraModDepth = 
{ 
	pt_byte,									// type
	"Modulation",
	"Modulation depth [ms]",// description
	1,												// MinValue	
	100,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	30,                       // default
};

CMachineParameter const paraLFORate = 
{ 
	pt_byte,										// type
	"LFO rate",
	"LFO rate [Hz]",				    // description
	0,												  // MinValue	
	240,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	150
};

CMachineParameter const paraWetOut = 
{ 
	pt_byte,										// type
	"Wet out",
	"Wet out [dB]",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	240
};

CMachineParameter const paraFeedback = 
{ 
	pt_byte,										// type
	"Feedback",
	"Feedback (00 = -100%, 40=0%, 80 = 100%)",		// description
	0,												// MinValue	
	128,											// MaxValue
	255,											// NoValue
	MPF_STATE,										// Flags
	0x40
};


CMachineParameter const *pParameters[] = 
{ 
	&paraDryOut,
	&paraMinDelay,
	&paraModDepth,
	&paraLFORate,
	&paraWetOut,
	&paraFeedback,
};

/*
CMachineAttribute const attrMaxDelay = 
{
	"Max Delay (ms)",
	1,
	100000,
	1000	
};

CMachineAttribute const *pAttributes[] = 
{
	&attrMaxDelay
};
*/

#pragma pack(1)

class gvals
{
public:
	byte dryout;
};

class tvals
{
public:
  byte mindelay;
  byte moddepth;
  byte lforate;
  byte wetout;
  byte feedback;
};

class avals
{
public:
  int dummy_attr;
//	int maxdelay;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	1,										// numGlobalParameters
	5,										// numTrackParameters
	pParameters,
	0,                    // 1 (numAttributes)
	NULL,                 // pAttributes
#ifdef _DEBUG
	"FSM Chorus (Debug build)",			// name
#else
	"FSM Chorus",
#endif
	"FSMChorus",								// short name
	"Krzysztof Foltman",						// author
	"A&bout"
};

class CTrack
{
public:
/*
  int Length;
	int Pos;
	int Unit;
  */
  float MinDelay;
  float ModDepth;
  float DeltaPhase;
  float Feedback;
	float WetOut;
  
  double Phase;
};
 

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);

	virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);
	virtual void Command(int const i);

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
  float *Buffer;
  int Pos;
  float DryOut;
	int numTracks;
	CTrack Tracks[MAX_TAPS];

private:

	avals aval;
	gvals gval;
	tvals tval[MAX_TAPS];
	int nEmptySamples;
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = (int *)&aval;
  Buffer = new float[MAX_DELAY];
	nEmptySamples=0;
}

mi::~mi()
{
  delete Buffer;
}

#define LFOPAR2TIME(value) (0.05*pow(800.0,value/255.0))

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
  case 0:
  case 4:
    if (value)
      sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
    else
      sprintf(txt, "-inf dB");
		break;
  case 5:
    sprintf(txt, "%4.1f %%", (double)(value*100.0/64.0-100.0) );
    break;
	case 1:   // min/delta delay
  case 2:
		sprintf(txt, "%4.1f ms", (double)(value/10.0) );
		break;
	case 3:		// LFO rate
		sprintf(txt, "%5.3f Hz", (double)LFOPAR2TIME(value));
		break;
	default:
		return NULL;
	}

	return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->mindelay != paraMinDelay.NoValue)
		pt->MinDelay = (float)(pMasterInfo->SamplesPerSec * ptval->mindelay/10000.0);
	if (ptval->moddepth != paraModDepth.NoValue)
		pt->ModDepth = (float)(pMasterInfo->SamplesPerSec * ptval->moddepth/10000.0);
	if (ptval->lforate != paraLFORate.NoValue)
		pt->DeltaPhase = (float)(2*3.1415926*LFOPAR2TIME(ptval->lforate)/pMasterInfo->SamplesPerSec);
  if (ptval->wetout != paraWetOut.NoValue)
    pt->WetOut = ptval->wetout?(float)pow(2.0,(ptval->wetout/10.0-24.0)/6.0):(float)0.0;
	if (ptval->feedback != paraFeedback.NoValue)
		pt->Feedback = (float)((ptval->feedback - 64) * (1.0 / 64.0)); 
}



void mi::Init(CMachineDataInput * const pi)
{
  numTracks = 1;

  for (int c = 0; c < MAX_TAPS; c++)
  {
    Tracks[c].DeltaPhase=0.0f;
    Tracks[c].Feedback=0.0f;
    Tracks[c].MinDelay=10.0f;
    Tracks[c].ModDepth=4.0f;
    Tracks[c].WetOut=0.3f;
    Tracks[c].Phase=0.0f;
  }

  Tracks[0].WetOut = 0.3f;	// enable first track

  for (int c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;
}

void mi::AttributesChanged()
{
/*
	MaxDelay = (int)(pMasterInfo->SamplesPerSec * (aval.maxdelay / 1000.0));
	for (int c = 0; c < numTracks; c++)
		InitTrack(c);
    */
}


void mi::SetNumTracks(int const n)
{
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
	Tracks[i].Phase = 0;
}

void mi::ResetTrack(int const i)
{
}


void mi::Tick()
{
	for (int c = 0; c < numTracks; c++)
		TickTrack(&Tracks[c], &tval[c]);
  if (gval.dryout!=paraDryOut.NoValue)
  {
    if (gval.dryout)
      DryOut=(float)pow(2.0,(gval.dryout/10.0-24.0)/6.0);
    else
      DryOut=0.0f;
  }
}

#pragma optimize ("a", on) 

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

inline int f2i(double d)
{
	const double magic = 6755399441055744.0; // 2^51 + 2^52
	double tmp = (d-0.5) + magic;
	// return *(int*) &tmp;
    return (int)tmp;
}

static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
  float *pData=pmi->Buffer;
  float pos0=(float)(trk->MinDelay+trk->ModDepth*0.5);
  float dpos=(float)(trk->ModDepth*0.5);
  int nPos=pmi->Pos;
  float vsin=sin(trk->Phase), vcos=cos(trk->Phase);
  float dsin=sin(trk->DeltaPhase), dcos=cos(trk->DeltaPhase);
  bool first=(trk==pmi->Tracks);
  //bool last=(trk==pmi->Tracks+pmi->numTracks-1);
  float fbval=(float)(trk->Feedback/pmi->numTracks);
  for (int i=0; i<c; i++)
  {
    float pos=(float)(pos0+dpos*vsin);
    float vsin1=vsin*dcos+vcos*dsin;
    float vcos1=vcos*dcos-vsin*dsin;
    vsin=vsin1;vcos=vcos1;
    float floatPos=nPos-pos;
    //floatPos+=(floatPos<0)?MAX_DELAY:0;
//    int intPos=(int)floatPos;
    int intPos=f2i(floatPos);
    float delayed=INTERPOLATE(floatPos-intPos,pData[intPos&DELAY_MASK],pData[(intPos+1)&DELAY_MASK]);
    if (first)
    {
      pData[nPos]=pin[i]+float(delayed*fbval);
      pout[i]=pin[i]+float(delayed*trk->WetOut);
    }
    else
    {
      pData[nPos]+=float(delayed*fbval);
      pout[i]+=float(delayed*trk->WetOut);
    }
//    pout[i]=pin[i];
    // trk->Phase+=trk->DeltaPhase;
    nPos=(nPos+1)&DELAY_MASK;
  }
  trk->Phase=fmod(trk->Phase+c*trk->DeltaPhase,2*3.141592665);
  //if (last) pmi->Pos=nPos;
}

/*
static void DoWorkNoInput(float *pout, CMachineInterface *mi, int c, CTrack *trk)
{
	do
	{
		double delay = *pbuf;
		*pbuf++ = (float)(feedback * delay);
		*pout++ += (float)(delay * wetout);

	} while(--c);
}

static void DoWorkNoInputNoOutput(CMachineInterface *mi, int c, CTrack *trk)
{
	do
	{
		*pbuf = (float)(*pbuf * feedback);
		pbuf++;
	} while(--c);
}

static void DoWorkNoOutput(float *pin, CMachineInterface *mi, int c, CTrack *trk)
{
	do
	{

		double delay = *pbuf;
		double dry = *pin++;
		*pbuf++ = (float)(feedback * delay + dry);

	} while(--c);
}
*/

#pragma optimize ("", on)


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
  DoWork(pin,pout,this,numsamples,pt);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
	float *paux = pCB->GetAuxBuffer();

	if (mode & WM_READ)
  {
		// memcpy(paux, psamples, numsamples*4);
    nEmptySamples=0;
  }
  else
  {
    if (nEmptySamples>256)
      return false;
    for (int i=0; i<numsamples; i++)
      psamples[i]=0.0;
    nEmptySamples+=numsamples;
  }

  int so=0, maxs=128;

  if (numTracks>1)
  {
    for (int i=0; i<numTracks-1; i++)
    {
      if (f2i(Tracks[i].MinDelay)<maxs)
        maxs=f2i(Tracks[i].MinDelay)-1;
    }
  }

  while(so<numsamples)
  {
    int end=__min(so+maxs,numsamples);
    for (int c = 0; c < numTracks; c++)
  		WorkTrack(Tracks + c, psamples+so, paux+so, end-so, mode);
    Pos=(Pos+end-so)&DELAY_MASK;
    so=end;
  }

//	for (int c = 0; c < numTracks; c++)
//		WorkTrack(Tracks + c, psamples, paux, numsamples, mode);

  memcpy(psamples,paux,numsamples*4);
	return true;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM Chorus version 1.2 !\nWritten by Krzysztof Foltman (kf@cw.pl), Gfx by Oom\nSpecial thx to: oskari, canc3r, Oom, ard0x\n(he noticed the small bug in 1.0/1.1) and all #buzz crew\n\n\n"
    "Visit my homepage at www.mp3.com/psytrance\n(buzz-generated goa/psychedelic trance) and grab my songs ! :-)");
}

