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
#include <windef.h>
#include <MachineInterface.h>
#include <mdk/mdk.h>
#include "../dspchips/DSPChips.h"

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS        8
// 200 ms przy 44100 Hz
#define MAX_DELAY   8192
#define DELAY_MASK  8191

#define TABLE_BITS 15
#define COUNTER_BITS 28
#define TABLE_SIZE (1<<TABLE_BITS)
#define FRACT_RANGE (1<<(COUNTER_BITS-TABLE_BITS))

///////////////////////////////////////////////////////////////////////////////////

CMachineParameter const paraDryOut = 
{ 
    pt_byte,                                        // type
    "Dry out",
    "Dry out [dB]",                    // description
    0,                                                  // MinValue    
    240,                                                  // MaxValue
    255,                                        // NoValue
    MPF_STATE,                                        // Flags
    240
};

CMachineParameter const paraResetPhase = 
{ 
    pt_byte,                                        // type
    "Reset Phase",
    "Reset Phase",                    // description
    0,                                                  // MinValue    
    240,                                                  // MaxValue
    255,                                        // NoValue
    0,                                        // Flags
    0
};

CMachineParameter const paraMinDelay = 
{ 
    pt_byte,                                    // type
    "Minimum delay",
    "Minimum delay [ms]",         // description
    1,                                                // MinValue    
    100,                                          // MaxValue
    255,                                          // NoValue
    MPF_STATE,                                // Flags
    20,                       // default
};

CMachineParameter const paraModDepth = 
{ 
    pt_byte,                                    // type
    "Modulation",
    "Modulation depth [ms]",// description
    1,                                                // MinValue    
    100,                                            // MaxValue
    255,                                          // NoValue
    MPF_STATE,                                // Flags
    10,                       // default
};

CMachineParameter const paraLFORate = 
{ 
    pt_byte,                                        // type
    "LFO rate",
    "LFO rate [Hz]",                    // description
    0,                                                  // MinValue    
    254,                                                // MaxValue
    255,                                    // NoValue
    MPF_STATE,                                    // Flags
    10
};

CMachineParameter const paraWetOut = 
{ 
    pt_byte,                                        // type
    "Wet out",
    "Wet out [dB]",                    // description
    0,                                                  // MinValue    
    240,                                                  // MaxValue
    255,                                        // NoValue
    MPF_STATE,                                        // Flags
    240
};

CMachineParameter const paraFeedback = 
{ 
    pt_byte,                                        // type
    "Feedback",
    "Feedback (00 = -100%, 40=0%, 80 = 100%)",        // description
    0,                                                // MinValue    
    128,                                            // MaxValue
    255,                                            // NoValue
    MPF_STATE,                                        // Flags
    0x40
};

CMachineParameter const paraPhasing = 
{ 
    pt_byte,                                        // type
    "Phasing",
    "Stereo Phasing",        // description
    0,                                                // MinValue    
    128,                                            // MaxValue
    255,                                            // NoValue
    MPF_STATE,                                        // Flags
    0x20
};


CMachineParameter const *pParameters[] = 
{ 
    &paraDryOut,
    &paraResetPhase,
    &paraMinDelay,
    &paraModDepth,
    &paraLFORate,
    &paraWetOut,
    &paraFeedback,
    &paraPhasing,
};

CMachineAttribute const attrMaxDelay = 
{
    "LFO Shape (0=sine, 1=triangle, 2,3=saw, 4=phat, 5-7=weirdo)",
    0,
    7,
    4
};

CMachineAttribute const *pAttributes[] = 
{
    &attrMaxDelay
};

class gvals
{
public:
    byte dryout;
    byte resetphase;
};

class tvals
{
public:
    byte mindelay;
    byte moddepth;
    byte lforate;
    byte wetout;
    byte feedback;
    byte stereophasing;
};

class avals
{
public:
    int lfoshape;
};

CMachineInfo const MacInfo = 
{
    MT_EFFECT,                                // type
    MI_VERSION,
    MIF_DOES_INPUT_MIXING,                                        // flags
    1,                                        // min tracks
    MAX_TAPS,                                // max tracks
    2,                                        // numGlobalParameters
    6,                                        // numTrackParameters
    pParameters,
    1,                    // 1 (numAttributes)
    pAttributes,                 // pAttributes
#ifdef _DEBUG
    "FSM PhatMan (Debug build)",            // name
#else
    "FSM PhatMan",
#endif
    "PhatMan",                                // short name
    "Krzysztof Foltman",                        // author
    "A&bout"
};

class CTrack
{
public:
    float MinDelay;
    float ModDepth;
    float DeltaPhase;
    float Feedback;
    float WetOut;
    float StereoPhasing;
    float LastPos, LastPos2;

    float vsin,vcos,dsin,dcos,psin,pcos;
  
    byte LFORate;

    double Phase;
};
 
class miex : public CMDKMachineInterfaceEx
{
};

class mi : public CMDKMachineInterface
{
public:
    miex ex;
    bool isstereo;
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
    virtual void Command(int const i);

private:
    void InitTrack(int const i);
    void ResetTrack(int const i);

    void TickTrack(CTrack *pt, tvals *ptval);
    void WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);
    void PrepareTrack(int tno);
    void WorkTrackStereo(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);

public:
    float *Buffer;
    int Pos;
    float DryOut;
    float FeedbackLimiter;
    int numTracks;
    CTrack Tracks[MAX_TAPS];

    float FuncTable[8*TABLE_SIZE];

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
    int i;
    Buffer = new float[MAX_DELAY];

    float slope=12.0f;
  
    float accum=0.0;
    for (i=0; i<TABLE_SIZE; i++)
    {
        accum=accum*0.8+0.2*((rand()&255)/256.0)*(i<TABLE_SIZE/2?1:(1-(i-TABLE_SIZE/2)/(TABLE_SIZE/2.0)));
        FuncTable[i+0*TABLE_SIZE]=(i<TABLE_SIZE/2)?(i*4.0/TABLE_SIZE-1):(3-i*4.0/TABLE_SIZE);
        FuncTable[i+1*TABLE_SIZE]=(i<TABLE_SIZE/slope)?-cos(i*PI/(TABLE_SIZE/slope)):INTERPOLATE((TABLE_SIZE-i)/(TABLE_SIZE-TABLE_SIZE/slope),-1,+1);
        FuncTable[i+2*TABLE_SIZE]=-((i<TABLE_SIZE/slope)?-cos(i*PI/(TABLE_SIZE/slope)):INTERPOLATE((TABLE_SIZE-i)/(TABLE_SIZE-TABLE_SIZE/slope),-1,+1));
        FuncTable[i+4*TABLE_SIZE]=sin(i*2*PI/8192+0.8*cos(i*6*PI/8192)+0.7*sin(i*10*PI/8192));
        FuncTable[i+5*TABLE_SIZE]=sin(PI*cos(i*2*PI/8192)+0.1*accum);
        FuncTable[i+6*TABLE_SIZE]=sin(i*2*PI/8192+0.3*cos(i*10*PI/8192)+0.4*cos(i*12*PI/8192));
    }

    for (i=0; i<TABLE_SIZE/2; i++)
    {
        double phs=double(i-TABLE_SIZE/4)/(TABLE_SIZE/4);
        FuncTable[i+3*TABLE_SIZE]=((phs*phs-1));
        FuncTable[i+int(TABLE_SIZE*3.5)]=-((phs*phs-1));
    }
}

mi::~mi()
{
    delete []Buffer;
    numTracks=-1;
}

char const *mi::DescribeValue(int const param, int const value)
{
    static char txt[16];

    switch(param)
    {
        case 0:
        case 5:
            if (value)
                sprintf(txt, "%4.1f dB", (double)(value/10.0-24.0) );
            else
                sprintf(txt, "-inf dB");
            break;
        case 6:
            sprintf(txt, "%4.1f %%", (double)(value*100.0/64.0-100.0) );
            break;
        case 2:   // min/delta delay
        case 3:
            sprintf(txt, "%4.1f ms", (double)(value/10.0) );
            break;
        case 4:        // LFO rate
            LfoRateDesc(txt,value);
            break;
        case 7:
            sprintf(txt, "%4.1f deg", (double)((value-64)*180/64.0) );
            break;
        default:
            return NULL;
    }

    return txt;
}

void mi::TickTrack(CTrack *pt, tvals *ptval)
{
    if (gval.resetphase!=255)
        pt->Phase=(float)(2*PI*gval.resetphase/240);
    if (ptval->mindelay != paraMinDelay.NoValue)
        pt->MinDelay = (float)(pMasterInfo->SamplesPerSec * ptval->mindelay/10000.0);
    if (ptval->moddepth != paraModDepth.NoValue)
        pt->ModDepth = (float)(pMasterInfo->SamplesPerSec * ptval->moddepth/10000.0);
    if (ptval->lforate != paraLFORate.NoValue)
        pt->LFORate=ptval->lforate;
//        pt->DeltaPhase = (float)(2*M_PI*LFOPAR2TIME(ptval->lforate)/pMasterInfo->SamplesPerSec);
    pt->DeltaPhase=(float)LfoRateToDeltaPhase(pt->LFORate,(int)pMasterInfo->TicksPerSec,(float)pMasterInfo->SamplesPerSec);
    if (ptval->wetout != paraWetOut.NoValue)
        pt->WetOut = ptval->wetout?(float)pow(2.0,(ptval->wetout/10.0-24.0)/6.0):(float)0.0;
    if (ptval->feedback != paraFeedback.NoValue)
        pt->Feedback = (float)((ptval->feedback - 64) * (1.0 / 64.0)); 
    if (ptval->stereophasing != paraPhasing.NoValue)
        pt->StereoPhasing = (float)((ptval->stereophasing - 64) * PI * (1.0 / 64.0)); 
}



void mi::MDKInit(CMachineDataInput * const pi)
{
    int c;
    numTracks = 1;

    for (c = 0; c < MAX_TAPS; c++)
    {
        Tracks[c].DeltaPhase=0.0f;
        Tracks[c].Feedback=0.0f;
        Tracks[c].MinDelay=10.0f;
        Tracks[c].ModDepth=4.0f;
        Tracks[c].WetOut=0.3f;
        Tracks[c].Phase=0.0f;
        Tracks[c].StereoPhasing=0.0f;
    }

    Tracks[0].WetOut = 0.3f;    // enable first track

  for (c=0; c<MAX_DELAY; c++)
    Buffer[c]=0.0f;

  Pos=0;

  SetOutputMode(true);

  //printf("%s:%s()\n",__FILE__,__FUNCTION__);
  //fflush(stdout);
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
    Tracks[i].LastPos = 0;
    Tracks[i].LastPos2 = 0;
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

static void DoWork(float *pin, float *pout, mi *pmi, int c, CTrack *trk)
{
    float *pData=pmi->Buffer;
    float pos0=(float)(trk->MinDelay+trk->ModDepth*0.5);
    float dpos=(float)(trk->ModDepth*0.5);
    int nPos=pmi->Pos;
    float vsin=(float)sin(trk->Phase), vcos=(float)cos(trk->Phase);
    float dsin=(float)sin(trk->DeltaPhase), dcos=(float)cos(trk->DeltaPhase);
    float FB=float(trk->Feedback*pmi->FeedbackLimiter);
    bool first=(trk==pmi->Tracks);
    float temp[1024];
    if (!pout) pout=temp;
    for (int i=0; i<c; i++)
    {
        float pos=(float)(pos0+dpos*vsin);
        float vsin1=vsin*dcos+vcos*dsin;
        float vcos1=vcos*dcos-vsin*dsin;
        vsin=vsin1;vcos=vcos1;
        float floatPos=nPos-pos;
        int intPos=f2i(floatPos);
        int intPos2=(intPos<MAX_DELAY-1)?(intPos+1):0;
        
        //printf("%d, %f -> %lf\n",nPos,pos,floatPos);
        //printf("%d, %d, %d\n",intPos,intPos2,MAX_DELAY);
        
        float delayed=INTERPOLATE(floatPos-intPos,pData[intPos],pData[intPos2]);
        if (first)
        {
            pData[nPos]=pin[i]+float(delayed*FB);
            pout[i]=pmi->DryOut*pin[i]+float(delayed*trk->WetOut);
        }
        else
        {
            pData[nPos]+=float(delayed*FB);
            pout[i]+=float(delayed*trk->WetOut);
        }
        nPos=(nPos>=MAX_DELAY-1)?0:nPos+1;
    }
    trk->Phase=fmod(trk->Phase+c*trk->DeltaPhase,2*M_PI);
}


void mi::WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode)
{
    DoWork(pin,(mode&WM_WRITE)?pout:NULL,this,numsamples,pt);
}

void mi::PrepareTrack(int tno)
{
    CTrack *trk=Tracks+tno;
    trk->vsin=(float)sin(trk->Phase), trk->vcos=(float)cos(trk->Phase);
    trk->dsin=(float)sin(trk->DeltaPhase), trk->dcos=(float)cos(trk->DeltaPhase);
    trk->psin=(float)sin(trk->StereoPhasing), trk->pcos=(float)cos(trk->StereoPhasing);
}

void mi::WorkTrackStereo(CTrack *trk, float *pin, float *pout, int numsamples, int const mode)
{
    float *pData=Buffer;
    int nPos=Pos&DELAY_MASK;

    float pos0=(float)(trk->MinDelay+trk->ModDepth*0.5);
    float dpos=(float)(trk->ModDepth*0.5);
    bool first=(trk==Tracks);
    float FB=float(trk->Feedback*FeedbackLimiter);
    float WO=trk->WetOut;
    float pos, pos2;

    //acpos=0;acpos2=0;
    if (aval.lfoshape)
    {
        int nPhase=int((1<<COUNTER_BITS)*trk->Phase/(2*PI));
        int nDPhase=int((1<<COUNTER_BITS)*trk->DeltaPhase/(2*PI));
        int nPhaseShift=int(TABLE_SIZE*trk->StereoPhasing/(2*PI));
        int nShift=TABLE_SIZE*(aval.lfoshape-1);

        int nPhasePos=nPhase>>(COUNTER_BITS-TABLE_BITS);
        float fracPhase=float((nPhase&(FRACT_RANGE-1))*(1.0/FRACT_RANGE));
        float d0=FuncTable[nShift+(nPhasePos&(TABLE_SIZE-1))];
        float d1=FuncTable[nShift+((nPhasePos+1)&(TABLE_SIZE-1))];
        pos=pos0+dpos*(d0+(d1-d0)*fracPhase);

        nPhasePos+=nPhaseShift; // przesuni�cie fazy
        d0=FuncTable[nShift+((nPhasePos)&(TABLE_SIZE-1))];
        d1=FuncTable[nShift+((nPhasePos+1)&(TABLE_SIZE-1))];
        pos2=pos0+dpos*(d0+(d1-d0)*fracPhase);

        float acpos=trk->LastPos-pos;
        float acpos2=trk->LastPos2-pos2;
        pos+=acpos;pos2+=acpos2;

        for (int i=0; i<2*numsamples; i+=2)
        {
            //pos=(float)(pos0+dpos*vsin+acpos);
            //pos2=(float)(pos0+dpos*(psin*vcos+pcos*vsin)+acpos2);
            int nPhasePos=nPhase>>(COUNTER_BITS-TABLE_BITS);
            nPhase+=nDPhase;
            float fracPhase=float((nPhase&(FRACT_RANGE-1))*(1.0/FRACT_RANGE));
            d0=FuncTable[nShift+(nPhasePos&(TABLE_SIZE-1))];
            d1=FuncTable[nShift+((nPhasePos+1)&(TABLE_SIZE-1))];
            pos=pos0+dpos*(d0+(d1-d0)*fracPhase)+acpos;

            nPhasePos+=nPhaseShift; // przesuni�cie fazy
            d0=FuncTable[nShift+((nPhasePos)&(TABLE_SIZE-1))];
            d1=FuncTable[nShift+((nPhasePos+1)&(TABLE_SIZE-1))];
            pos2=pos0+dpos*(d0+(d1-d0)*fracPhase)+acpos2;

            acpos*=0.9995f;acpos2*=0.9995f;
            float floatPos=nPos-pos;
            int intPos=(int)(floatPos);
            float delayed=INTERPOLATE(floatPos-intPos,pData[intPos&DELAY_MASK],pData[(intPos+1)&DELAY_MASK]);

            floatPos=nPos-pos2;
            intPos=(int)(floatPos);
            // printf("floatPos = %f, intPos = %d\n", floatPos, intPos);
            float delayed2=INTERPOLATE(floatPos-intPos,pData[intPos&DELAY_MASK],pData[(intPos+1)&DELAY_MASK]);
            if (first)
            {
                pData[nPos]=float(0.5*(pin[i]+pin[i+1]+float((delayed+delayed2)*FB)));
                pout[i]=pin[i]*DryOut+float(delayed*WO);
                pout[i+1]=pin[i+1]*DryOut+float(delayed2*WO);
            }
            else
            {
                pData[nPos]+=float((delayed+delayed2)*0.5*FB);
                pout[i]+=float(delayed*WO);
                pout[i+1]+=float(delayed2*WO);
            }
            nPos=(nPos+1)&DELAY_MASK;
        }
    }
    else
    {
        float vsin=trk->vsin, vcos=trk->vcos, dsin=trk->dsin, dcos=trk->dcos, psin=trk->psin, pcos=trk->pcos;
        pos=(float)(pos0+dpos*vsin);
        pos2=(float)(pos0+dpos*(psin*vcos+pcos*vsin));
        float acpos=trk->LastPos-pos;
        float acpos2=trk->LastPos2-pos2;
        pos+=acpos;pos2+=acpos2;
        for (int i=0; i<2*numsamples; i+=2)
        {
            pos=(float)(pos0+dpos*vsin+acpos);
            pos2=(float)(pos0+dpos*(psin*vcos+pcos*vsin)+acpos2);
            acpos*=0.9995f;acpos2*=0.9995f;
            float vsin1=vsin*dcos+vcos*dsin;
            float vcos1=vcos*dcos-vsin*dsin;
            vsin=vsin1;vcos=vcos1;
            float floatPos=nPos-pos;
            int intPos=f2i(floatPos);
            float delayed=INTERPOLATE(floatPos-intPos,pData[intPos&DELAY_MASK],pData[(intPos+1)&DELAY_MASK]);
            //    float delayed=CSI(pData[(intPos+0)&DELAY_MASK],pData[(intPos+1)&DELAY_MASK],pData[(intPos+2)&DELAY_MASK],pData[(intPos+3)&DELAY_MASK],floatPos-intPos);

    
            floatPos=nPos-pos2;
            intPos=f2i(floatPos);
            //    float delayed2=CSI(pData[(intPos+0)&DELAY_MASK],pData[(intPos+1)&DELAY_MASK],pData[(intPos+2)&DELAY_MASK],pData[(intPos+3)&DELAY_MASK],floatPos-intPos);
            float delayed2=INTERPOLATE(floatPos-intPos,pData[intPos&DELAY_MASK],pData[(intPos+1)&DELAY_MASK]);
            //float delayed2=INTERPOLATE(floatPos2-intPos,pData[intPos],pData[intPos2]);
            if (first)
            {
                pData[nPos]=float(0.5*(pin[i]+pin[i+1]+float((delayed+delayed2)*FB)));
                pout[i]=pin[i]*DryOut+float(delayed*WO);
                pout[i+1]=pin[i+1]*DryOut+float(delayed2*WO);
            }
            else
            {
                pData[nPos]+=float((delayed+delayed2)*0.5*FB);
                //      pData[nPos]+=float((pin[i]+pin[i+1])*0.5*trk->Feedback);
                pout[i]+=float(delayed*WO);
                pout[i+1]+=float(delayed2*WO);
            }
            //    pout[i]=pin[i];
            // trk->Phase+=trk->DeltaPhase;
            nPos=(nPos+1)&DELAY_MASK;
        }
        trk->vsin=vsin, trk->vcos=vcos;
        trk->dsin=dsin, trk->dcos=dcos;
        trk->psin=psin, trk->pcos=pcos;
    }
    trk->Phase=fmod(trk->Phase+numsamples*trk->DeltaPhase,2*M_PI);
    trk->LastPos=pos;
    trk->LastPos2=pos2;
}

int nEmptySamples=0;

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
    float *paux = pCB->GetAuxBuffer();

    // this officially doesn't work, and it's never used either
    
    if (mode & WM_READ)
    {
        //memcpy(paux, psamples, numsamples*4);
        nEmptySamples=0;
    }
    else
    {
        if (nEmptySamples>256)
            return false;
        for (int i=0; i<numsamples*2; i++)
            psamples[i]=0.0;
        nEmptySamples+=numsamples;
    }

    for (int c = 0; c < numTracks; c++)
        WorkTrack(Tracks + c, psamples, paux, numsamples, mode);

    if (mode & WM_WRITE)
    {
        // now copy back to output
        memcpy(psamples, paux, numsamples*4);
    }

    int *pint=(int *)paux;
    for (int i=0; i<numsamples; i++)
        if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
            return true;
    return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
    float *paux = pCB->GetAuxBuffer();

    float Fb=0;
    int i;
    for (i=0; i<numTracks; i++)
        Fb+=(float)fabs(Tracks[i].Feedback*0.995);
    FeedbackLimiter=(float)((Fb>0.995)?(0.995/Fb):0.995);
    //printf("MDKWorkStereo numsamples = %d, mode = %d\n", numsamples, mode);
    //FeedbackLimiter=1.0;
    if (mode & WM_READ)
    {
        // memcpy(paux, psamples, numsamples*4);
        nEmptySamples=0;
    }
    else
    {
        //if (Fb>0.98) Fb=0.98;
        if (nEmptySamples>512 && pow(Fb*FeedbackLimiter,nEmptySamples/256.0)<(16.0/32767))
            return false;
        for (i=0; i<2*numsamples; i++)
            psamples[i]=0.0;
        nEmptySamples+=numsamples;
    }

    int so=0, maxs=128;

    if (numTracks>1)
    {
        for (int i=0; i<numTracks; i++)
        {
            if (f2i(Tracks[i].MinDelay)<maxs)
                maxs=f2i(Tracks[i].MinDelay)-1;
        }
    }

    if (!aval.lfoshape)
        for (i=0; i<numTracks; i++)
            PrepareTrack(i);

    Pos&=DELAY_MASK;
    while(so<numsamples)
    {
        int end=__min(so+maxs,numsamples);
        for (int c = 0; c < numTracks; c++)
            WorkTrackStereo(Tracks + c, psamples+so*2, paux+so*2, end-so, mode);
        Pos=(Pos+end-so)&DELAY_MASK;
        so=end;
    }
    if (!(mode&WM_WRITE))
      return mode; // FIXME: it should return bool

    // now copy back to output
    memcpy(psamples,paux,numsamples*8);

    // check for silence?
    int *pint=(int *)paux;
    for (i=0; i<2*numsamples; i++)
        if ((pint[i]&0x7FFFFFFF)>=0x3F800000)
            return true;
    return false;
}

void mi::Command(int const i)
{
    pCB->MessageBox("FSM PhatMan version 0.95b (XionD pack edition)!\nWritten by Krzysztof Foltman (kf@cw.pl), Gfx by Oom\n");  
}

