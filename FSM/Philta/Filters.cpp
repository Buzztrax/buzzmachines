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

// globalny tuning
// zakres dla suwak�w (cutoff, resonance, modulation)
// lepszy tryb mono
// sustain 0 -> b��d
// startuje -> bzdury
// bug w seq<->buzz

// lokalne/globalne LFO
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#pragma optimize ("a", on)
#include "../dspchips/DSPChips.h"

void C6thOrderFilter::CalcCoeffs(int nType, float _CurCutoff, float _Resonance, float _ThevFactor)
{
  CurCutoff=_CurCutoff;
  Resonance=_Resonance;
  ThevFactor=_ThevFactor;
  switch(nType)
  {
  case 0: CalcCoeffs1(); break;
  case 1: CalcCoeffs2(); break;
  case 2: CalcCoeffs3(); break;
  case 3: CalcCoeffs4(); break;
  case 4: CalcCoeffs5(); break;
  case 5: CalcCoeffs6(); break;
  case 6: CalcCoeffs7(); break;
  case 7: CalcCoeffs8(); break;
  case 8: CalcCoeffs9(); break;
  case 9: CalcCoeffs10(); break;
  case 10: CalcCoeffs11(); break;
  case 11: CalcCoeffs12(); break;
  case 12: CalcCoeffs13(); break;
  case 13: CalcCoeffs14(); break;
  case 14: CalcCoeffs15(); break;
  case 15: CalcCoeffs16(); break;
  case 16: CalcCoeffs17(); break;
  case 17: CalcCoeffs18(); break;
  }
}

void C6thOrderFilter::CalcCoeffs1() // 6L Multipeak
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.707+7*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf/3.0f,fQ,44100,sqrt(0.707f)/sqrt(fQ));
  m_filter2.rbjLPF(2.0f*cf/3.0f,fQ/2,44100);
  m_filter3.rbjLPF(cf,fQ/3,44100);
}

void C6thOrderFilter::CalcCoeffs2() // 6L Separated
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=16000) cf=16000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/22000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(1.50f+10.6f*Resonance/240.0*ScaleResonance);

  float sep=float(0.05+0.6*Resonance/240.0);
  m_filter.rbjLPF(cf,fQ,44100,0.3f/pow(fQ/2.50f,0.05));
  m_filter2.rbjLPF(cf*(1-sep),fQ,44100);
  m_filter3.rbjLPF(__min(cf*(1+sep),21000),fQ,44100);
}

void C6thOrderFilter::CalcCoeffs3() // 6L HiSquelch
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+10*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf,fQ,44100,0.6f/pow(__max(fQ,1.0),1.7));
  m_filter2.rbjLPF(cf,fQ,44100);
  m_filter3.rbjLPF(cf,fQ,44100);
}

void C6thOrderFilter::CalcCoeffs4() // 4L Skull D
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/21000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(1.0+10*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf,0.707,44100,0.50f);
  m_filter2.rbjLPF(cf,0.707,44100);
  m_filter3.SetParametricEQ(cf,fQ*4,fQ*2.0f,44100);
}

void C6thOrderFilter::CalcCoeffs5() // 4L TwinPeaks
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+5*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf,fQ,44100,0.3f/__max(sqrt(fQ)*fQ,1.0));
  m_filter2.rbjLPF(cf,fQ,44100);
  m_filter3.SetParametricEQ(cf/2,3*(fQ-0.7)+1,8*(fQ-0.7)+1,44100);
}

void C6thOrderFilter::CalcCoeffs6() // 4L Killah
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+5*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf/1.41,fQ,44100,0.6f/__max(sqrt(fQ)*fQ,1.0));
  m_filter2.rbjLPF(__min(cf*1.41,22000),fQ,44100);
  m_filter3.SetParametricEQ(cf,16/fQ,fQ*4.0f,44100);
}

void C6thOrderFilter::CalcCoeffs7() // 4L Phlatt
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+5*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf,fQ,44100,0.8f/__max(fQ,1.0));
  m_filter2.rbjLPF(cf,fQ,44100);
  m_filter3.rbjBRF(cf,fQ,44100);
}

void C6thOrderFilter::CalcCoeffs8() // 2L phlatt
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float fQ=(float)(1.0+4*(240-Resonance)/240.0);

  m_filter.rbjLPF(cf,1.007,44100,float(0.8f/__max(sqrt(fQ),1)));
  m_filter2.rbjBRF(cf*0.707,fQ/2,44100);
  m_filter3.rbjBRF(cf,fQ/2,44100);
}

void C6thOrderFilter::CalcCoeffs9() // 2L FrontFlt
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/22000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+6*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf,2*fQ,44100,float(0.3f/__max(sqrt(fQ),1)));
  m_filter2.SetParametricEQ(cf/2,3*(fQ-0.7)+1,3*(fQ-0.7)+1,44100);
  m_filter3.SetParametricEQ(cf/4,3*(fQ-0.7)+1,3*(fQ-0.7)+1,44100);
}

void C6thOrderFilter::CalcCoeffs10() // 2L LaserOne
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+6*Resonance*ScaleResonance/240.0);

  m_filter.rbjLPF(cf,2*fQ,44100,float(0.15f/__max(sqrt(fQ),1)));
  m_filter2.SetParametricEQ(cf*3/4,2*(fQ-0.7)+1,3*(fQ-0.7)+1,44100);
  m_filter3.SetParametricEQ(cf/2,2*(fQ-0.7)+1,3*(fQ-0.7)+1,44100);
}

#define THREESEL(sel,a,b,c) ((sel)<120)?((a)+((b)-(a))*(sel)/120):((b)+((c)-(b))*((sel)-120)/120)

void C6thOrderFilter::CalcCoeffs11() // 2L FMish
{
  float CutoffFreq=(float)(132*pow(64,CurCutoff/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;
  float fQ=(float)(0.71+6*120*ScaleResonance/240.0);

  float sc1=(float)pow(__min(0.89,0.33+0.2*CurCutoff/240.0),1-Resonance/240.0+0.5);
  float sc2=(float)pow(__min(0.9,0.14+0.1*CurCutoff/240.0),1-Resonance/240.0+0.5);
  m_filter.rbjLPF(cf,2*fQ,44100,0.2f/__max(sqrt(fQ),1.0));
  m_filter2.SetParametricEQ(cf*sc1,2*(fQ-0.7)+1,3*(fQ-0.7)+1,44100);
  m_filter3.SetParametricEQ(cf*sc2,2*(fQ-0.7)+1,3*(fQ-0.7)+1,44100);
}

void C6thOrderFilter::CalcCoeffs12()
{
  float CutoffFreq=(float)(132*pow(64,(240-CurCutoff)/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;

  float q=0.1f+Resonance*0.6f/240.0f;
  //float q=3.6f;
  float spacing=(float)pow(1.3f+3*(240-Resonance)/240.0,1-cf/20000.0f);
  m_filter.rbjBRF(cf,q,44100);
  m_filter2.rbjBRF(cf/spacing,q,44100);
  m_filter3.rbjBRF(__min(21000,cf*spacing),q,44100);
}

void C6thOrderFilter::CalcCoeffs13()
{
  float CutoffFreq=(float)(66*pow(64,(CurCutoff)/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;

  float q=0.71f+Resonance*2.6f/240.0f;
  //float q=3.6f;
  float spacing=(float)pow(1.3f+3*(240-Resonance)/240.0,1-cf/20000.0f);
  m_filter.rbjHPF(cf,q,44100,0.71/(pow(q,0.7)));
  m_filter2.rbjHPF(cf/spacing,q,44100);
  m_filter3.rbjHPF(__min(21000,cf*spacing),q,44100);
}

void C6thOrderFilter::CalcCoeffs14()
{
  float CutoffFreq=(float)(66*pow(64,(CurCutoff)/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;

  float q=0.1f+ScaleResonance*Resonance*2.6f/240.0f;
  //float q=3.6f;
  m_filter.rbjBPF(cf,q,44100,pow(q,0.7)/1.7f);
  m_filter2.rbjBPF(cf*0.9,q,44100);
  m_filter3.rbjBPF(__min(21000,cf*1.01),q,44100);
}

void C6thOrderFilter::CalcCoeffs15()
{
  float CutoffFreq=(float)(132*pow(64,(CurCutoff)/240.0));
	float cf=(float)CutoffFreq;
	if (cf>=20000) cf=20000; // pr�ba wprowadzenia nieliniowo�ci przy ko�cu charakterystyki
	if (cf<33) cf=(float)(33.0);
  // float ScaleResonance=(float)pow(cf/20000.0,ThevFactor);
  // float ScaleResonance=1.0;

  float q=2.1f+Resonance*9.6f/240.0f;
  //float q=3.6f;
  m_filter.SetParametricEQ(float(cf/4),1,q,44100,float(0.25/sqrt(q)));
  m_filter2.SetParametricEQ(float(cf/2),2,float(1/q),44100);
  m_filter3.SetParametricEQ(cf,1,q,44100);
}

void C6thOrderFilter::CalcCoeffs16()
{
  float q=2.1f+Resonance*32.6f/240.0f;

	if (CurCutoff<0) CurCutoff=0;
	if (CurCutoff>240) CurCutoff=240;
  float Cutoff1=THREESEL(CurCutoff,270,800,400);
  float Cutoff2=THREESEL(CurCutoff,2140,1150,800);

  m_filter.SetParametricEQ(Cutoff1,2.5,q,44100,float(1.0/q));
  m_filter2.rbjLPF(Cutoff2*1.2,sqrt(q),44100);
  m_filter3.SetParametricEQ(Cutoff2,2.5,sqrt(q),44100);
}


void C6thOrderFilter::CalcCoeffs17()
{
  float q=2.1f+Resonance*32.6f/240.0f;

	if (CurCutoff<0) CurCutoff=0;
	if (CurCutoff>240) CurCutoff=240;
  float Cutoff1=THREESEL(CurCutoff,650,400,270);
  float Cutoff2=THREESEL(CurCutoff,1080,1700,2140);
  m_filter.SetParametricEQ(Cutoff1,2.5,q,44100,float(1.0/q));
  m_filter2.rbjLPF(Cutoff2*1.2,sqrt(q),44100);
  m_filter3.SetParametricEQ(Cutoff2,2.5,sqrt(q),44100);
}

void C6thOrderFilter::CalcCoeffs18()
{
	m_filter.SetNothing();
	m_filter2.SetNothing();
	m_filter3.SetNothing();
}

