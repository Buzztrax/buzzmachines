#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"

#include "Infector.h"

static const int times[]={
  1,2,3,4,6,8,12,16,24,28,32,48,64,96,128
};

CTrack::CTrack()
{
  channel=-1;
  ClearFX();
  Init();
}

void CTrack::ClearFX()
{
  Vib1Rate=0.0f;
  Vib2Rate=0.0f;
  Vib1Depth=0.0f;
  Vib2Depth=0.0f;
  RetrigPoint=0;
  Detune=0;
  ShuffleAmount=0;ShuffleCounter=0;ShuffleMax=0;
  for (int i=0; i<16; i++)
    ShuffleData[i]=(i&1)?100:0;
  Chn()->ClearFX();
  SlideCounter=SlideEnd=SlideRange=0.0f;
  CurLFO=0.0;
  CurLFO2=0.0;
}

void CTrack::Reset()
{
  ClearFX();
  if (channel!=-1)
    Chn()->Reset();
  channel=-1;
}

int CTrack::GetWakeupTime(int maxtime)
{
  if (RetrigCount)
    maxtime=__min(RetrigCount-RetrigPoint,maxtime);
  
///////////////

  return maxtime;
}

void CTrack::UseWakeupTime(int maxtime)
{
  if (RetrigCount)
    RetrigPoint+=maxtime;
}

static float SustLevel(byte value)
{
  return (float)pow(1.0/128,1-value/240.0);
}

byte CTrack::AllocChannel()
{
  int i;

  float MinAmp=0.1f;
  int last=-1;
  for (i=0; i<MAX_CHANNELS; i++)
  {
    CADSREnvelope *pADSR=&pmi->Channels[i].AmpEnv;
    if (pADSR->m_nState>0 && pADSR->m_nState!=4 && pmi->Channels[i].Velocity*pADSR->m_fLast<MinAmp)
      MinAmp=float(pmi->Channels[i].Velocity*pADSR->m_fLast),
      last=i;
  }
  if (last!=-1)
    return last;

  for (i=0; i<MAX_CHANNELS; i++)
    if (pmi->Channels[i].IsFree())
      return i;

  MinAmp=9e6f;
  for (i=0; i<MAX_CHANNELS; i++)
  {
    CADSREnvelope *pADSR=&pmi->Channels[i].AmpEnv;
    if (pADSR->m_nState>0 && pmi->Channels[i].Velocity*pADSR->m_fLast<MinAmp)
      MinAmp=float(pmi->Channels[i].Velocity*pADSR->m_fLast),
      last=i;
  }
  if (last!=-1)
    return last;


  // zaimplementowaæ jakiœ inny sprytny algorytm channel stealingu

  //__asm { 
  //  int 0x03;
  //}
  return rand()%MAX_CHANNELS;
}

void CTrack::PlayNote(byte note, byte _accent, byte _length, CMasterInfo *pMasterInfo)
{
  if (note==NOTE_OFF)
  {
    CChannel *chn=Chn();
    if (!chn)
      return;
    chn->FilterEnv.NoteOff();
    chn->AmpEnv.NoteOff();
    MidiNote=-1;
    return;
  }

/*
// wersja 1

  CChannel *chn=Chn();
  channel=AllocChannel();
  if (chn)
  {
    //chn->FilterEnv.NoteOffFast();
    chn->AmpEnv.NoteOffFast();
    chn->pTrack=NULL;
  }
  chn=Chn();
  if (chn->pTrack && chn->pTrack!=this) // w³aœnie ukradliœmy komuœ kana³
    chn->pTrack->channel=-1;
  */

  CChannel *chn=Chn();
  channel=this-pmi->Tracks;
  if (chn)
  {
    int nNew=AllocChannel();
/*
    char ss[128];
    sprintf(ss,"Track %d moved to channel %d\n",channel,nNew);
    OutputDebugString(ss);
    */
    chn->AmpEnv.NoteOffFast();
    chn->pTrack=NULL;
    memcpy(&pmi->Channels[nNew],chn,sizeof(CChannel));
  }
  chn=Chn();

/*
  char ss[128];
  sprintf(ss,"Allocated channel %d\n",channel);
  OutputDebugString(ss);
  */
  chn->pTrack=this;
  chn->Phase1=0;
  chn->Phase2=0;
  chn->Filter.m_filter.Reset();
  chn->Filter.m_filter2.Reset();
  chn->Filter.m_filter3.Reset();
  lastnote=note;
  lastaccent=_accent;
  lastlength=_length;
  MidiNote=note;
  chn->Velocity=float(_accent/240.0);
  double length=(_length==240)?32768:double(_length)*pMasterInfo->SamplesPerTick/(16*pMasterInfo->SamplesPerSec);
  RetrigPoint=0;

  gvals &gvalAct=pmi->gvalAct;
  chn->FilterEnv.SetEnvelope(GETENVTIME2(gvalAct.vFilterAttack),GETENVTIME(gvalAct.vFilterDecay),SustLevel(gvalAct.vFilterSustain),length,GETENVTIME(gvalAct.vFilterRelease),pMasterInfo->SamplesPerSec);
  chn->AmpEnv.SetEnvelope(GETENVTIME2(gvalAct.vAmpAttack),GETENVTIME(gvalAct.vAmpDecay),SustLevel(gvalAct.vAmpSustain),length,GETENVTIME(gvalAct.vAmpRelease),pMasterInfo->SamplesPerSec);
   if (!(gvalAct.vLFOMode&4))
  {
//    if (chn->FilterEnv.m_nAttackTime<200)
//      chn->FilterEnv.RealNoteOn(float(1-(200-chn->FilterEnv.m_nAttackTime)/200.0*4095/4096));
//    else
      chn->FilterEnv.RealNoteOn();
  }
  if (!(gvalAct.vLFOMode&8))
    chn->AmpEnv.NoteOn();
  SlideEnd=0.0;
  chn->Detune=Detune;
  BaseFrequency=DestFrequency=float((220*pow(2.0,((note-1)>>4)+((note&15)-22-36)/12.0))/pMasterInfo->SamplesPerSec);
  if (!gvalAct.vGlide)
    chn->Frequency=BaseFrequency;
  chn->NoteReset();
  if (gvalAct.vLFOMode&1)
    LFOPhase=0.0f;
  if (gvalAct.vLFOMode&2)
    LFO2Phase=0.0f;
}


CChannel *CTrack::Chn()
{
  if (channel==-1)
    return NULL;
  return &pmi->Channels[channel];
}

void CTrack::DoLFO(mi *pmi, int c)
{
  CurLFO=(float)(inrLFO1.Process(float(CalcLFO(pmi->gvalAct.vLFOShape,LFOPhase)*30.0),c)/30.0);

  float DeltaPhase;
  if (pmi->gvalAct.vLFORate<240)
    DeltaPhase = (float)(2*3.1415926*LFOPAR2TIME(pmi->gvalAct.vLFORate)/pmi->pMasterInfo->SamplesPerSec);
  else
    DeltaPhase = (float)(2*3.1415926*(float(pmi->pMasterInfo->TicksPerSec))/(times[pmi->gvalAct.vLFORate-240]*pmi->pMasterInfo->SamplesPerSec));
  LFOPhase+=c*DeltaPhase;
  if (LFOPhase>1024*PI)
    LFOPhase-=float(1024*PI);
  

  CurLFO2=(float)(inrLFO2.Process(float(CalcLFO(pmi->gvalAct.vLFO2Shape,LFO2Phase)*30.0),c)/30.0);

  if (pmi->gvalAct.vLFO2Rate<240)
    DeltaPhase = (float)(2*3.1415926*LFOPAR2TIME(pmi->gvalAct.vLFO2Rate)/pmi->pMasterInfo->SamplesPerSec);
  else
    DeltaPhase = (float)(2*3.1415926*(float(pmi->pMasterInfo->TicksPerSec))/(times[pmi->gvalAct.vLFO2Rate-240]*pmi->pMasterInfo->SamplesPerSec));
  LFO2Phase+=c*DeltaPhase;
  if (LFO2Phase>1024*PI)
    LFO2Phase-=float(1024*PI);
}

void CTrack::Init()
{
  DestFrequency=0.01f;
  Vib1Phase=0.0f;
  Vib2Phase=0.0f;
  RetrigCount=0;
  Arps[0]=0;
  ArpPoint=0; ArpCount=1;
  LFOPhase=0.0f;
  LFO2Phase=0.0f;
}

void CTrack::DoWakeup(mi *pmi)
{
  if (RetrigCount && RetrigPoint>=RetrigCount)
  {
    RetrigPoint=0;
    if (Arps[ArpPoint%ArpCount]!=char(0x80))
      PlayNote(SeqToBuzz(BuzzToSeq(lastnote)+Arps[ArpPoint%ArpCount]),lastaccent,lastlength,pmi->pMasterInfo);
    lastnote-=Arps[ArpPoint%ArpCount];
    ArpPoint=(ArpPoint+1)%ArpCount;
  }
}

void CTrack::CommandA(byte cmd, word param)
{
  if (cmd==0xFE)
    pmi->ClearFX();
  if (cmd==0xFD)
    ClearFX();
  if (cmd==0xED)
  {
    RetrigCount=pmi->pMasterInfo->SamplesPerTick;
    RetrigPoint=RetrigCount-(param&0x0F)*pmi->pMasterInfo->SamplesPerTick/12;
    NoTrig=1;
    RetrigMode=0;
    Arps[0]=0;
    ArpPoint=0; ArpCount=1;
  }
  if (cmd==0x13)
  {
    ShuffleAmount=param&0x0F;
    ShuffleMax=2;
    ShuffleCounter=0;
  }
  if (cmd==0x05 || cmd==0x06)
    NoTrig=-1;
  if (cmd==0x03)
    NoTrig=2;
}

void CTrack::CommandB(byte cmd, word param)
{
  if (cmd==0x01)
  {
    if (SlideEnd) BaseFrequency=Chn()->Frequency;
    SlideCounter=0;
    SlideRange=byte(param);
    SlideEnd=float(param>>8)/4.0f;
  }
  if (cmd==0x02)
  {
    if (SlideEnd) BaseFrequency=Chn()->Frequency;
    SlideCounter=0;
    SlideRange=float(-(param&0xFF));
    SlideEnd=float((param>>8)/4.0f);
  }
  if (cmd==0x03)
  {
    if (Chn())
    {
      if (SlideEnd) BaseFrequency=Chn()->Frequency;
      SlideCounter=0;
      SlideRange=float(12*log(NotePortFrequency/Chn()->Frequency)/log(2));
      SlideEnd=float((param&0xFF)/4.0f);
      NoTrig=2;
    }
  }
  if (cmd==0x0C)
  {
    if (byte(param)!=0xFF) LFOPhase=float(param*2*PI/255);
    if (byte(param>>8)!=0xFF) LFO2Phase=float(param*2*PI/255);
  }
  if (cmd==0xE5)
  {
    Detune=(float)(param-0x8000);
    CChannel *chn=Chn();
    if (chn)
      chn->Detune=Detune;
  }
  if (cmd==0x04)
  {
    Vib2Phase=Vib1Phase;
    Vib1Rate= float((param>>12)&0x0F)/2.0f;
    Vib2Rate= float((param>>4)&0x0F)/2.0f;
    Vib1Depth=float((param&0x00F0?((param>>8)&0x0F)/384.0:0.0));
    Vib2Depth=float((param&0x0F00?((param>>0)&0x0F)/384.0:0.0));
  }
  if (cmd==0x05)
  {
    RetrigCount=pmi->pMasterInfo->SamplesPerTick/3+1;
    RetrigPoint=0;
    RetrigMode=0;
    Arps[0]=0;Arps[1]=char(param>>8);Arps[2]=char(param&0xFF);
    ArpPoint=1; ArpCount=3;
  }
  if (cmd==0x06)
  {
    RetrigCount=pmi->pMasterInfo->SamplesPerTick+1;
    RetrigPoint=(12-(param>>8))*pmi->pMasterInfo->SamplesPerTick/12;
    RetrigMode=0;
    Arps[0]=0;Arps[1]=char(param&0xFF);
    ArpPoint=1; ArpCount=2;
  }
  if (cmd==0xE9)
  {
    RetrigCount=int((param&0x0F)*(pmi->pMasterInfo->SamplesPerTick+1)/12.0+0.99);
    RetrigPoint=((param&0xF0)>>4)*pmi->pMasterInfo->SamplesPerTick/12;
    RetrigMode=0;
    Arps[0]=0;
    ArpPoint=0; ArpCount=1;
  }
}

