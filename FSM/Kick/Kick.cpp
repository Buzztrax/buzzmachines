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
#include <float.h>
#include <windef.h>
#include <MachineInterface.h>

#define MAX_TAPS        8

///////////////////////////////////////////////////////////////////////////////////

#pragma pack(push,4)

CMachineParameter const paraTrigger = 
{ 
    pt_byte,                        // type
    "Trigger",
    "Trigger/volume",                    // description
    0,                            // MinValue    
    240,                            // MaxValue
    255,                            // NoValue
    0,                            // Flags// MPF_STATE
    0
};

CMachineParameter const paraStartFrq = 
{ 
    pt_byte,                        // type
    "Start",
    "Start frequency",                    // description
    1,                            // MinValue    
    240,                            // MaxValue
    255,                            // NoValue
    MPF_STATE,                        // Flags
    198
};

CMachineParameter const paraEndFrq = 
{ 
    pt_byte,                        // type
    "End",
    "End frequency",                    // description
    1,                            // MinValue    
    240,                            // MaxValue
    255,                            // NoValue
    MPF_STATE,                        // Flags
    64
};

CMachineParameter const paraToneDecay = 
{ 
    pt_byte,                        // type
    "T DecTime",
    "Tone decay time",// description
    1,                            // MinValue    
    240,                            // MaxValue
    255,                            // NoValue
    MPF_STATE,                        // Flags
    46,                                                     // default
};

CMachineParameter const paraToneShape = 
{ 
    pt_byte,                        // type
    "T DecShape",
    "Tone decay shape",                        // description
    1,                            // MinValue    
    240,                            // MaxValue
    255,                            // NoValue
    MPF_STATE,                        // Flags
    27
};

CMachineParameter const paraAmpDecay = 
{ 
    pt_byte,                        // type
    "A DecTime",
    "Amplitude decay time",                        // description
    1,                            // MinValue    
    240,                            // MaxValue
    255,                            // NoValue
    MPF_STATE,                        // Flags
    55
};

CMachineParameter const *pParameters[] = 
{ 
  &paraTrigger,
    &paraStartFrq,
    &paraEndFrq,
    &paraToneDecay,
    &paraToneShape,
    &paraAmpDecay,
};

CMachineAttribute const attrFloor = 
{
    "Floor force",
    0,
    100,
    0
};

CMachineAttribute const *pAttributes[] = 
{
    &attrFloor
};

#pragma pack(1)

class gvals
{
public:
};

class tvals
{
public:
  byte volume;
  byte startfrq;
  byte endfrq;
  byte tdecay;
  byte tshape;
  byte adecay;
};

class avals
{
public:
    int Floor;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
    MT_GENERATOR,                                // type
    MI_VERSION,
    0,                                        // flags
    1,                                        // min tracks
    MAX_TAPS,                                // max tracks
    0,                                        // numGlobalParameters
    6,                                        // numTrackParameters
    pParameters,
    1,                    // 1 (numAttributes)
    pAttributes,                 // pAttributes
#ifdef _DEBUG
    "FSM Kick (Debug build)",            // name
#else
    "FSM Kick",
#endif
    "FSMKick",                                // short name
    "Krzysztof Foltman",                        // author
    NULL
};

class CTrack
{
public:
    float StartFrq;
    float EndFrq;
    float TDecay;
    float TShape;
    float ADecay;
    float CurVolume;
    float LastValue;
    float AntiClick;
    float Amp;
    float MulAmp;
    float Frequency;

    double xSin, xCos, dxSin, dxCos;

    int EnvPhase;
    int LeftOver;
    double OscPhase;
};
 
class mi : public CMachineInterface
{
public:
    mi();
    ~mi() {}
    virtual void Destructor();

    virtual void Init(CMachineDataInput * const pi);
    virtual void Tick();
    virtual bool Work(float *psamples, int numsamples, int const mode);

    virtual void SetNumTracks(int const n);

    virtual void AttributesChanged();

    virtual char const *DescribeValue(int const param, int const value);
    virtual void Stop() {  }

private:
    void InitTrack(int const i);
    void ResetTrack(int const i);

    void TickTrack(CTrack *pt, tvals *ptval);
    bool WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
    float *Buffer;
    int Pos;
    float DryOut;
    avals aval;

public:
    short *thump1;
    int thump1len;

private:
    int numTracks;
    CTrack Tracks[MAX_TAPS];

    gvals gval;
    tvals tval[MAX_TAPS];
};

DLL_EXPORTS

mi::mi()
{
    GlobalVals = &gval;
    TrackVals = tval;
    AttrVals = (int *)&aval;
}

void mi::Destructor()
{
    this->~mi();
}

// unused
//#define LFOPAR2TIME(value) (0.05*pow(800.0,value/255.0))

char const *mi::DescribeValue(int const param, int const value)
{
    return NULL;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
    if (ptval->volume != paraTrigger.NoValue)
    {
        pt->AntiClick = pt->LastValue;
        pt->CurVolume = (float)(ptval->volume*(32000.0/128.0));
        pt->EnvPhase = 0;
        pt->OscPhase = 0.0;
        pt->LeftOver = 0;
    }
    if (ptval->startfrq != paraStartFrq.NoValue)
        pt->StartFrq = (float)(33.0*pow(128,ptval->startfrq/240.0));
    if (ptval->endfrq != paraEndFrq.NoValue)
        pt->EndFrq = (float)(33.0*pow(16,ptval->endfrq/240.0));
    if (ptval->tdecay!= paraToneDecay.NoValue)
        pt->TDecay = (float)(ptval->tdecay/240.0);
    if (ptval->tshape!= paraToneShape.NoValue)
        pt->TShape = (float)(ptval->tshape/240.0);
    if (ptval->adecay!= paraAmpDecay.NoValue)
        pt->ADecay = (float)(ptval->adecay/240.0);
 }



void mi::Init(CMachineDataInput * const pi)
{
    numTracks = 1;
        InitTrack(0);

    for (int c = 0; c < MAX_TAPS; c++)
    {
        tvals vals;
        vals.volume=paraTrigger.DefValue;
        vals.startfrq=paraStartFrq.DefValue;
        vals.endfrq=paraEndFrq.DefValue;
        vals.tdecay=paraToneDecay.DefValue;
        vals.tshape=paraToneShape.DefValue;
        vals.adecay=paraAmpDecay.DefValue;
        TickTrack(&Tracks[c], &vals);
    }
}

void mi::AttributesChanged()
{
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
    Tracks[i].LeftOver = 0;
    Tracks[i].EnvPhase = 0;
    Tracks[i].OscPhase = 0;
    Tracks[i].CurVolume = 0;
    Tracks[i].LastValue = 0;
    Tracks[i].AntiClick = 0;
    Tracks[i].Amp = 0;
    Tracks[i].MulAmp = 0;
    Tracks[i].Frequency = 0;
    Tracks[i].xSin = 0;
    Tracks[i].xCos = 0;
    Tracks[i].dxSin = 0;
    Tracks[i].dxCos = 0;
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

// unused
//#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

static bool DoWork(float *pout, mi *pmi, int c, CTrack *trk)
{
    float Ratio=trk->EndFrq/trk->StartFrq;
    int i=0;
    double xSin=trk->xSin, xCos=trk->xCos;
    double dxSin=trk->dxSin, dxCos=trk->dxCos;
    float LVal=0.0f;
    float AClick=trk->AntiClick;
    if (AClick<-32000.0) AClick=-32000.0;
    else if (AClick>=32000.0) AClick=32000.0;
    float Amp=trk->Amp;
    float MulAmp=trk->MulAmp;
    bool amphigh=Amp>=16;

    while(i<c)
    {
        if (trk->LeftOver<=0)
        {
            trk->LeftOver=32;
            double EnvPoint=trk->EnvPhase*trk->TDecay/400.0;
            double ShapedPoint=pow(EnvPoint,trk->TShape*2);
            trk->Frequency=(float)(trk->StartFrq*pow(Ratio,__max(pmi->aval.Floor/100.0,ShapedPoint)));
            if (trk->Frequency>10000 || trk->CurVolume<1) trk->CurVolume=0.0;
            Amp=(float)(trk->CurVolume*pow(1.0/256.0,trk->ADecay*trk->EnvPhase/5000.0));
            
            if ((Amp<16) && (fabs(AClick)<256)) {
                Amp = 0.0;
                break;
            }

            trk->OscPhase=fmod(trk->OscPhase,1.0);
            trk->MulAmp=MulAmp=(float)pow(1.0/256.0,trk->ADecay/5000.0);
            xSin=(float)sin(2.0*M_PI*trk->OscPhase);
            xCos=(float)cos(2.0*M_PI*trk->OscPhase);
            dxSin=(float)sin(2.0*M_PI*trk->Frequency/44100.0);
            dxCos=(float)cos(2.0*M_PI*trk->Frequency/44100.0);
            LVal=0.0f;
            trk->dxSin=dxSin, trk->dxCos=dxCos;
        }
        int end=i+trk->LeftOver;
        end=__min(end,c);
        if (Amp)
        {
            for (int j=i; j<end; j++)
            {
                pout[j]+=LVal=float(AClick+Amp*xSin);
                double xSin2=double(xSin*dxCos+xCos*dxSin);
                double xCos2=double(xCos*dxCos-xSin*dxSin);
                xSin=xSin2;xCos=xCos2;
                Amp*=MulAmp;
                AClick*=0.98f;
            }
        }
        trk->OscPhase+=(end-i)*trk->Frequency/44100.0;
        trk->EnvPhase+=end-i;
        trk->LeftOver-=end-i;
        i=end;
    }
    trk->xSin=xSin, trk->xCos=xCos;
    trk->LastValue=LVal;
    trk->AntiClick=AClick;
    trk->Amp=Amp;
    return (Amp>=16) || amphigh;
}


#pragma optimize ("", on)


bool mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
    return DoWork(pout,this,numsamples,pt);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
    for (int i=0; i<numsamples; i++)
        psamples[i]=0.0;

    bool donesth=false;
    for (int c = 0; c < numTracks; c++)
        donesth |= WorkTrack(Tracks + c, NULL, psamples, numsamples, mode);

    return donesth;
}


