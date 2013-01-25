/* $Id$
 *
 * buzzmachines
 * Copyright (C) 2001-2007 Krzysztof Foltman  <kfoltman@users.sourceforge.net>
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
#include <float.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"

#define MAX_TAPS		1

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraCutoff = 
{ 
	pt_byte,										// type
	"Cutoff",
	"Cutoff frequency",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraResonance = 
{ 
	pt_byte,										// type
	"Resonance",
	"Resonance",					// description
	0,												  // MinValue	
	240,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraFilterType = 
{ 
	pt_byte,										// type
	"Filter",
	"Filter type",	// description
	0,												  // MinValue	
	7,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	6
};

CMachineParameter const paraLFODepth = 
{ 
	pt_byte,									// type
	"LFO mod",
	"LFO Modulation depth",// description
	0,												// MinValue	
	240,											// MaxValue
	255,										  // NoValue
	MPF_STATE,								// Flags
	60,                       // default
};

CMachineParameter const paraLFORate = 
{ 
	pt_byte,										// type
	"LFO rate",
	"LFO rate [Hz]",				    // description
	0,												  // MinValue	
	254,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	30
};

CMachineParameter const paraLFOShape = 
{ 
	pt_byte,										// type
	"LFO Shape",
	"LFO Shape",	// description
	0,												  // MinValue	
	16,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraInertia = 
{ 
	pt_byte,										// type
	"Inertia",
	"Cutoff frequency inertia",	// description
	0,												  // MinValue	
	60,												  // MaxValue
	255,										// NoValue
	MPF_STATE,										// Flags
	10
};

CMachineParameter const paraLFOPhase = 
{ 
	pt_byte,										// type
	"Set phase",
	"Set LFO phase",				    // description
	0,												  // MinValue	
	127,												// MaxValue
	255,									// NoValue
	MPF_STATE,									// Flags
	0
};



CMachineParameter const *pParameters[] = 
{ 
	&paraCutoff,
	&paraResonance,
  &paraFilterType,
	&paraLFORate,
	&paraLFODepth,
  &paraLFOShape,
	&paraInertia,
	&paraLFOPhase,
};

CMachineAttribute const attrMaxDelay = 
{
	"Theviderness factor",
	0,
	20,
	0
};

CMachineAttribute const *pAttributes[] = 
{
	&attrMaxDelay
};


#pragma pack(1)

class gvals
{
public:
	byte dryout;
};

class tvals
{
public:
  byte cutoff;
  byte resonance;
  byte filtype;
  byte lforate;
  byte lfodepth;
  byte lfoshape;
  byte inertia;
  byte lfophase;
};

class avals
{
public:
	int thevfactor;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	MAX_TAPS,								// max tracks
	0,										// numGlobalParameters
	8,										// numTrackParameters
	pParameters,
	1,                    // 1 (numAttributes)
	pAttributes,                 // pAttributes
#ifdef _DEBUG
	"FSM Philta (Debug build)",			// name
#else
	"FSM Philta",
#endif
	"Philta",								// short name
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
  float Cutoff;
  float Resonance;
  float LFORate;
  float LFODepth;
  float Inertia;
  double LFOPhase;
  double DeltaPhase;

  float CurCutoff;

  C6thOrderFilter m_filter;
  int FilterType;
  int LFOShape;
  float ThevFactor;

  void ResetFilter() { m_filter.ResetFilter(); }
  void CalcCoeffs() { m_filter.CalcCoeffs(FilterType,CurCutoff,Resonance,ThevFactor); }
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
  int Pos;
  float DryOut;
	avals aval;

private:
	int numTracks;
	CTrack Tracks[MAX_TAPS];

	gvals gval;
	tvals tval[MAX_TAPS];
  int Counter;
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals = (int *)&aval;
}

mi::~mi()
{
}

#define LFOPAR2TIME(value) (0.05*pow(800.0,value/255.0))

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[36];

	switch(param)
	{
/*  case 0:
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
    */
  case 2:
    Tracks[0].m_filter.GetFilterDesc(txt,value);
    break;
  case 3:		// LFO rate
    LfoRateDesc(txt,value);
		break;
  case 5: // LFO shape
    if (value==0) strcpy(txt,"sine");
    if (value==1) strcpy(txt,"saw up");
    if (value==2) strcpy(txt,"saw down");
    if (value==3) strcpy(txt,"square");
    if (value==4) strcpy(txt,"triangle");
    if (value==5) strcpy(txt,"weird 1");
    if (value==6) strcpy(txt,"weird 2");
    if (value==7) strcpy(txt,"weird 3");
    if (value==8) strcpy(txt,"weird 4");
    if (value==9) strcpy(txt,"steps up");
    if (value==10) strcpy(txt,"steps down");
    if (value==11) strcpy(txt,"upsaws up");
    if (value==12) strcpy(txt,"upsaws down");
    if (value==13) strcpy(txt,"dnsaws up");
    if (value==14) strcpy(txt,"dnsaws down");
    if (value==15) strcpy(txt,"S'n'H 1");
    if (value==16) strcpy(txt,"S'n'H 2");
    break;
	default:
		return NULL;
	}

	return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->lforate != paraLFORate.NoValue)
    pt->DeltaPhase=LfoRateToDeltaPhase(ptval->lfodepth,pMasterInfo->TicksPerSec,pMasterInfo->SamplesPerSec);
	if (ptval->lfophase != paraLFOPhase.NoValue)
		pt->LFOPhase = (float)(2*3.1415926*ptval->lfophase/128.0);
	if (ptval->lfodepth!= paraLFODepth.NoValue)
		pt->LFODepth = (float)(ptval->lfodepth);
	if (ptval->inertia!= paraInertia.NoValue)
		pt->Inertia = (float)(ptval->inertia/240.0);
	if (ptval->cutoff!= paraCutoff.NoValue)
		pt->Cutoff = ptval->cutoff;
	if (ptval->resonance!= paraResonance.NoValue)
		pt->Resonance = (float)(ptval->resonance);
  if (ptval->lfoshape!=paraLFOShape.NoValue)
    pt->LFOShape = ptval->lfoshape;
  if (ptval->filtype!=paraFilterType.NoValue)
  {
    if (pt->FilterType != ptval->filtype)
      pt->ResetFilter();
    pt->FilterType = ptval->filtype;
  }
 }



void mi::Init(CMachineDataInput * const pi)
{
	numTracks = 1;
  Counter=0;

	for (int c = 0; c < MAX_TAPS; c++)
	{
    tvals vals;
    vals.cutoff=paraCutoff.DefValue;
    vals.resonance=paraResonance.DefValue;
    vals.lforate=paraLFORate.DefValue;
    vals.lfodepth=paraLFODepth.DefValue;
    vals.lfophase=paraLFOPhase.DefValue;
    vals.lfoshape=paraLFOShape.DefValue;
    vals.inertia=paraInertia.DefValue;
    vals.filtype=paraFilterType.DefValue;
    TickTrack(&Tracks[c], &vals);
    Tracks[c].ResetFilter();
	}
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
	Tracks[i].LFOPhase = 0;
	Tracks[i].ResetFilter();
}

void mi::ResetTrack(int const i)
{
}


void mi::Tick()
{
	for (int c = 0; c < numTracks; c++)
		TickTrack(&Tracks[c], &tval[c]);
}

#pragma optimize ("a", on) 

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
  trk->ThevFactor=float(pmi->aval.thevfactor/20.0);
  float ai=(float)(10*exp(-trk->Inertia*9.0));
  for (int i=0; i<c; i+=64) // pow(2,CurCutoff/48.0)
  {
    float LFO=0.0;
    float Phs=(float)fmod(trk->LFOPhase,(float)(2*PI));
    switch(trk->LFOShape)
    {
      case 0: LFO=(float)sin(Phs); break;
      case 1: LFO=(float)(((Phs-PI)/PI-0.5f)*2.0f); break;
      case 2: LFO=(float)(((Phs-PI)/PI-0.5f)*-2.0f); break;
      case 3: LFO=(Phs<PI)?1.0f:-1.0f; break;
      case 4: LFO=(float)(((Phs<PI?(Phs/PI):(2.0-Phs/PI))-0.5)*2); break;
      case 5: LFO=(float)sin(trk->LFOPhase+PI/4*sin(trk->LFOPhase)); break;
      case 6: LFO=(float)sin(trk->LFOPhase+PI/6*sin(2*trk->LFOPhase)); break;
      case 7: LFO=(float)sin(2*trk->LFOPhase+PI*cos(3*trk->LFOPhase)); break;
      case 8: LFO=(float)(0.5*sin(2*trk->LFOPhase)+0.5*cos(3*trk->LFOPhase)); break;
      case 9: LFO=(float)(0.25*floor(((Phs-PI)/PI-0.5f)*2.0f*4.0)); break;
      case 10: LFO=(float)(-0.25*floor(((Phs-PI)/PI-0.5f)*2.0f*4.0)); break;
      case 11: LFO=(float)(0.125*floor(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(Phs,PI/4)/(PI/4)); break;
      case 12: LFO=(float)(-0.125*floor(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(Phs,PI/4)/(PI/4)); break;
      case 13: LFO=(float)(0.125*floor(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(2*PI-Phs,PI/4)/(PI/4)); break;
      case 14: LFO=(float)(-0.125*floor(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(2*PI-Phs,PI/4)/(PI/4)); break;
      case 15: LFO=(float)(0.5*(sin(19123*floor(trk->LFOPhase*8/PI)+40*sin(12*floor(trk->LFOPhase*8/PI))))); break; // 8 zmian/takt
      case 16: LFO=(float)(0.5*(sin(1239543*floor(trk->LFOPhase*4/PI)+40*sin(15*floor(trk->LFOPhase*16/PI))))); break; // 8 zmian/takt
    }
    float DestCutoff=(float)(trk->Cutoff+trk->LFODepth*LFO/2); // pow(2.0,trk->LFODepth*sin(trk->LFOPhase)/100.0);
    if (fabs(trk->CurCutoff-DestCutoff)<ai)
      trk->CurCutoff=DestCutoff;
    else
      trk->CurCutoff+=(float)_copysign(ai,DestCutoff-trk->CurCutoff);
    trk->CalcCoeffs();

    int jmax=__min(i+64,c);
    for (int j=i; j<jmax; j++)
    {
      pout[j]=2.0f*trk->m_filter.ProcessSample(pin[j]);
    }
    trk->LFOPhase+=(jmax-i)*trk->DeltaPhase;
    if (trk->LFOPhase>1024*PI)
      trk->LFOPhase-=1024*PI;
  }
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
  pt->m_filter.AvoidExceptions();
  DoWork(pin,pout,this,numsamples,pt);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
	float *paux = pCB->GetAuxBuffer();

	if (mode & WM_READ)
  {
    Counter=0;
		memcpy(paux, psamples, numsamples*4);
  }
  else
  {
    CTrack *trk=Tracks;
    if (Counter>1000 && trk->m_filter.IsSilent())
    {
      trk->LFOPhase+=numsamples*trk->DeltaPhase;
      return false;
    }
    Counter+=numsamples;
    for (int i=0; i<numsamples; i++)
      paux[i]=0.0;
  }

  for (int c = 0; c < numTracks; c++)
		WorkTrack(Tracks + c, paux, psamples, numsamples, mode);

  return true;
}

void mi::Command(int const i)
{
  pCB->MessageBox("FSM Philta version 0.1a !\nWritten by Krzysztof Foltman (kf@cw.pl)\n\n"
    "Visit my homepage at www.mp3.com/FSMachine\n(buzz-generated goa trance) and hear my songs ! :-)");
}

/*
void __declspec(dllexport) foo()
{
  mi *pmi=NULL;
  delete pmi;
}
*/
