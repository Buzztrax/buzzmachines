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
#include <malloc.h>
#include <math.h>
#include <float.h>
#include <windef.h>

#include <algorithm>

#ifdef __MSVC__
#define copysign(x,y) _copysign(x,y);
#endif


typedef float SIG;
#define TWOPI_F (2.0f*3.141592665f)

class CDistortion
{
public:
	float m_dPreGain, m_dPostGain;
	inline SIG ProcessSample(SIG dSmp) { 
		return (float)(atan(m_dPreGain*dSmp)*(1.0/1.6*m_dPostGain));
	}
};

class CInertia
{
protected:
  float m_fAI;
public:
  float m_fAccum;
public:
  CInertia() : m_fAccum(0.0), m_fAI(1.0) {}
  void SetInertia(double dInertia)
  {
    m_fAI=(float)(exp(-(dInertia+128)*4.5/240.0));
  }
  inline float Process(float value, int c)
  {
//    float diff=float(m_fAI*c*pow(fabs(value-m_fAccum),0.05));
    if (m_fAccum==value)
      return m_fAccum;
    float diff=float(m_fAI*c);
    if (fabs(m_fAccum-value)<diff)
      m_fAccum=value;
    else
      m_fAccum+=(float)copysign(diff,value-m_fAccum);
    return m_fAccum;
  }
};

class CBiquad
{
public:
	float m_a1, m_a2, m_b0, m_b1, m_b2;
	float m_Oa1, m_Oa2, m_Ob0, m_Ob1, m_Ob2;
	float m_x1, m_x2, m_y1, m_y2;
	CBiquad() { m_x1=0.0f; m_y1=0.0f; m_x2=0.0f; m_y2=0.0f; }
	inline SIG ProcessSample(SIG dSmp) { 
		SIG dOut=m_b0*dSmp+m_b1*m_x1+m_b2*m_x2-m_a1*m_y1-m_a2*m_y2;
//    if (dOut>=-0.00001 && dOut<=0.00001) dOut=0.0;
//    if (dOut>900000) dOut=900000.0;
//		if (dOut<-900000) dOut=-900000.0;
		m_y2=m_y1;
		m_y1=dOut;
		m_x2=m_x1;
		m_x1=dSmp;
		return dOut;
	}
	inline SIG ProcessSampleSafe(SIG dSmp) { 
		SIG dOut=m_b0*dSmp+m_b1*m_x1+m_b2*m_x2-m_a1*m_y1-m_a2*m_y2;
    if (dOut>=-0.00001 && dOut<=0.00001) dOut=0.0;
//    if (dOut>900000) dOut=900000.0;
//		if (dOut<-900000) dOut=-900000.0;
		m_x2=m_x1;
		m_x1=dSmp;
		m_y2=m_y1;
		m_y1=dOut;
		return dOut;
	}
  inline bool IsSilent()
  {
    return fabs(m_x1)<1 && fabs(m_x2)<1 && fabs(m_y1)<1 && fabs(m_y2)<1;
  }
  inline void AvoidExceptions()
  {
    if (IsSilent())
      m_x1=0.0f, m_y1=0.0f, m_x2=0.0f, m_y2=0.0f;
  }
	void PreNewFilter()
	{
		//m_Oa1=m_a1;
		//m_Oa2=m_a2;
		//m_Ob0=m_b0;
		//m_Ob1=m_b1;
		//m_Ob2=m_b2;
	}
	void PostNewFilter()
	{
		//m_x1=m_x1*m_Ob1/m_b1;
		//m_x2=m_x2*m_Ob2/m_b2;
		//m_y1=m_y1*m_Oa1/m_a1;
		//m_y2=m_y2*m_Oa2/m_a2;
	}

	float PreWarp(float dCutoff, float dSampleRate)
	{
		if (dCutoff>dSampleRate*0.4) dCutoff=(float)(dSampleRate*0.4);
		//return (float)(/*1.0/*/tan(3.1415926/2.0-3.1415926*dCutoff/dSampleRate));
		return (float)(tan(3.1415926*dCutoff/dSampleRate));
	}
	float PreWarp2(float dCutoff, float dSampleRate)
	{
		if (dCutoff>dSampleRate*0.4) dCutoff=(float)(dSampleRate*0.4);
		//return (float)(/*1.0/*/tan(3.1415926/2.0-3.1415926*dCutoff/dSampleRate));
		return (float)(tan(3.1415926/2.0-3.1415926*dCutoff/dSampleRate));
	}
	void SetBilinear(float B0, float B1, float B2, float A0, float A1, float A2)
	{
		float q=(float)(1.0/(A0+A1+A2));
		m_b0=(B0+B1+B2)*q;
		m_b1=2*(B0-B2)*q;
		m_b2=(B0-B1+B2)*q;
		m_a1=2*(A0-A2)*q;
		m_a2=(A0-A1+A2)*q;
	}
  // Robert Bristow-Johnson, robert@audioheads.com
  void rbjLPF(double fc, double Q, double esr, double gain=1.0)
  {
		PreNewFilter();
    float omega=(float)(2*PI*fc/esr);
    float sn=(float)sin(omega);
    float cs=(float)cos(omega);
    float alpha=(float)(sn/(2*Q));

    float inv=(float)(1.0/(1.0+alpha));

    m_b0 =  (float)(gain*inv*(1 - cs)/2);
    m_b1 =  (float)(gain*inv*(1 - cs));
    m_b2 =  (float)(gain*inv*(1 - cs)/2);
    m_a1 =  (float)(-2*cs*inv);
    m_a2 =  (float)((1 - alpha)*inv);
		PostNewFilter();
  }
  void rbjHPF(double fc, double Q, double esr, double gain=1.0)
  {
    float omega=(float)(2*PI*fc/esr);
    float sn=(float)sin(omega);
    float cs=(float)cos(omega);
    float alpha=(float)(sn/(2*Q));

    float inv=(float)(1.0/(1.0+alpha));

    m_b0 =  (float)(gain*inv*(1 + cs)/2);
    m_b1 =  (float)(gain*-inv*(1 + cs));
    m_b2 =  (float)(gain*inv*(1 + cs)/2);
    m_a1 =  (float)(-2*cs*inv);
    m_a2 =  (float)((1 - alpha)*inv);
  }
  void rbjBPF(double fc, double Q, double esr, double gain=1.0)
  {
    float omega=(float)(2*PI*fc/esr);
    float sn=(float)sin(omega);
    float cs=(float)cos(omega);
    float alpha=(float)(sn/(2*Q));

    float inv=(float)(1.0/(1.0+alpha));

    m_b0 =  (float)(gain*inv*alpha);
    m_b1 =  (float)(0);
    m_b2 =  (float)(-gain*inv*alpha);
    m_a1 =  (float)(-2*cs*inv);
    m_a2 =  (float)((1 - alpha)*inv);
  }
  void rbjBRF(double fc, double Q, double esr, double gain=1.0)
  {
    float omega=(float)(2*PI*fc/esr);
    float sn=(float)sin(omega);
    float cs=(float)cos(omega);
    float alpha=(float)(sn/(2*Q));

    float inv=(float)(1.0/(1.0+alpha));

    m_b0 =  (float)(gain*inv);
    m_b1 =  (float)(-gain*inv*2*cs);
    m_b2 =  (float)(gain*inv);
    m_a1 =  (float)(-2*cs*inv);
    m_a2 =  (float)((1 - alpha)*inv);
  }
  void rbjBPF2(float fc, float bw, float esr)
  {
    float omega=(float)(2*PI*fc/esr);
    float sn=(float)sin(omega);
    float cs=(float)cos(omega);
    float alpha=(float)(sn/sinh(log(2)/2*bw*omega/sn));

    float inv=(float)(1.0/(1.0+alpha));

    m_b0 =  (float)(inv*alpha);
    m_b1 =  (float)0;
    m_b2 =  (float)(-inv*alpha);
    m_a1 =  (float)(-2*cs*inv);
    m_a2 =  (float)((1 - alpha)*inv);
  }
  void rbjBRF2(float fc, float bw, float esr)
  {
    float omega=(float)(2*PI*fc/esr);
    float sn=(float)sin(omega);
    float cs=(float)cos(omega);
    float alpha=(float)(sn/sinh(log(2)/2*bw*omega/sn));

    float inv=(float)(1.0/(1.0+alpha));

    m_b0 =  (float)(inv);
    m_b1 =  (float)(-inv*2*cs);
    m_b2 =  (float)(inv);
    m_a1 =  (float)(-2*cs*inv);
    m_a2 =  (float)((1 - alpha)*inv);
  }

  // Zoelzer's Parmentric Equalizer Filters - rodem z Csound'a
  void SetLowShelf(float fc, float q, float v, float esr, float gain=1.0f)
  {
    float sq = (float)sqrt(2.0*(double)v);
    float omega = TWOPI_F*fc/esr;
    float k = (float) tan((double)omega*0.5);
    float kk = k*k;
    float vkk = v*kk;
    float oda0 =  1.0f/(1.0f + k/q +kk);
    m_b0 =  oda0*(1.0f + sq*k + vkk);
    m_b1 =  oda0*(2.0f*(vkk - 1.0f));
    m_b2 =  oda0*(1.0f - sq*k + vkk);
    m_a1 =  oda0*(2.0f*(kk - 1.0f));
    m_a2 =  oda0*(1.0f - k/q + kk);
  }
  void SetHighShelf(float fc, float q, float v, float esr, float gain=1.0f)
  {
    float sq = (float)sqrt(2.0*(double)v);
    float omega = TWOPI_F*fc/esr;
    float k = (float) tan((PI - (double)omega)*0.5);
    float kk = k*k;
    float vkk = v*kk;
    float oda0 = 1.0f/( 1.0f + k/q +kk);
    m_b0 = oda0*( 1.0f + sq*k + vkk)*gain;
    m_b1 = oda0*(-2.0f*(vkk - 1.0f))*gain;
    m_b2 = oda0*( 1.0f - sq*k + vkk)*gain;
    m_a1 = oda0*(-2.0f*(kk - 1.0f));
    m_a2 = oda0*( 1.0f - k/q + kk);
  }
  void SetParametricEQ(double fc, double q, double v, double esr, float gain=1.0f)
  {
		PreNewFilter();
    float sq = (float)sqrt(2.0*(double)v);
    float omega = float(TWOPI_F*fc/esr);
    float k = (float) tan((double)omega*0.5);
    float kk = k*k;
    float vk = float(v*k);
    float vkdq = float(vk/q);
    float oda0 =  float(1.0f/(1.0f + k/q +kk));
    m_b0 =  float(gain*oda0*(1.0f + vkdq + kk));
    m_b1 =  float(gain*oda0*(2.0f*(kk - 1.0f)));
    m_b2 =  float(gain*oda0*(1.0f - vkdq + kk));
    m_a1 =  float(oda0*(2.0f*(kk - 1.0f)));
    m_a2 =  float(oda0*(1.0f - k/q + kk));
		PostNewFilter();
  }
	void SetLowpass1(float dCutoff, float dSampleRate)
	{
		float a=PreWarp(dCutoff, dSampleRate);
		SetBilinear(a, 0, 0, a, 1, 0);
	}
	void SetHighpass1(float dCutoff, float dSampleRate)
	{
		float a=PreWarp(dCutoff, dSampleRate);
		SetBilinear(0, 1, 0, a, 1, 0);
	}
	void SetIntegHighpass1(float dCutoff, float dSampleRate)
	{
		float a=PreWarp(dCutoff, dSampleRate);
		SetBilinear(0, 1, 1, 0, 2*a, 2);
	}
	void SetAllpass1(float dCutoff, float dSampleRate)
	{
		float a=PreWarp(dCutoff, dSampleRate);
		SetBilinear(1, -a, 0, 1, a, 0);
	}
	void SetBandpass(float dCutoff, float dBandwith, float dSampleRate)
	{
		float b=(float)(2.0*PI*dBandwith/dSampleRate);
		float a=(float)(PreWarp2(dCutoff, dSampleRate));
		SetBilinear(0, b*a, 0, 1, b*a, a*a);
	}
	void SetBandreject(float dCutoff, float dSampleRate)
	{
		float a=(float)PreWarp(dCutoff, dSampleRate);
		SetBilinear(1, 0, 1, 1, a, 1);
	}
  void SetNBBandpass(float dCutoff, float dBW, float dSampleRate)
  {
    dCutoff/=dSampleRate, dBW/=dSampleRate;
    float R=(float)(1-3*dBW);
    float K=(float)((1-2*R*cos(2*PI*dCutoff)+R*R)/(2-2*cos(2*PI*dCutoff)));
    m_b0=(1-K);
    m_b1=(float)(2*(K-R)*cos(2*PI*dCutoff));
    m_b2=R*R-K;
    m_a1=(float)(2*R*cos(2*PI*dCutoff));
    m_a2=-R*R;
  }
  void SetNBBandreject(float dCutoff, float dBW, float dSampleRate)
  {
    dCutoff/=dSampleRate, dBW/=dSampleRate;
    float R=(1-3*dBW);
    float K=(float)((1-2*R*cos(2*PI*dCutoff)+R*R)/(2-2*cos(2*PI*dCutoff)));
    m_b0=K;
    m_b1=(float)(-2*K*cos(2*PI*dCutoff));
    m_b2=K;
    m_a1=(float)(2*R*cos(2*PI*dCutoff));
    m_a2=-R*R;
  }
	void SetIntegrator()
	{
		m_b0=1.0;
		m_a1=-1.0;
		m_a2=0.0;
		m_b1=0.0;
		m_b2=0.0;
	}
	void SetNothing()
	{
		m_b0=1.0;
		m_a1=0.0;
		m_a2=0.0;
		m_b1=0.0;
		m_b2=0.0;
	}
	void SetResonantLP(float dCutoff, float Q, float dSampleRate)
	{
		float a=(float)PreWarp2(dCutoff, dSampleRate);
		// wsp�czynniki filtru analogowego
		float B=(float)(sqrt(Q*Q-1)/Q);
		float A=(float)(2*B*(1-B));
		SetBilinear(1, 0, 0, 1, A*a, B*a*a);
	}
	void SetResonantHP(float dCutoff, float Q, float dSampleRate) // DOESN'T WORK !!!!!!!!
	{
		float a=(float)PreWarp2((dSampleRate/2)/dCutoff, dSampleRate);
		// wsp�czynniki filtru analogowego
		float B=(float)(sqrt(Q*Q-1)/Q);
		float A=(float)(2*B*(1-B));
		SetBilinear(0, 0, 1, B*a*a, A*a, 1);
	}
	void SetAllpass2(float dCutoff, float fPoleR, float dSampleRate)
	{
		float a=PreWarp(dCutoff, dSampleRate);
		float q=fPoleR;
		SetBilinear((1+q*q)*a*a, -2.0f*q*a, 1, (1+q*q)*a*a, 2.0f*q*a, 1);
	}
  void Reset()
  {
    m_x1=m_y1=m_x2=m_y2=0.0f;
  }
  void Copy(const CBiquad &src)
  {
  	m_a1=src.m_a1;
  	m_a2=src.m_a2;
  	m_b0=src.m_b0;
  	m_b1=src.m_b1;
  	m_b2=src.m_b2;
  }
};

#define EVOLVE_SINcos(outsin,outcos,insin,incos,modsin,modcos) outsin=insin*modcos+incos*modsin; outcos=incos*modcos-insin*modsin; 

class CPulseGenerator
{
public:
	double m_dPhase;
	float m_fFreq;
	float m_fHarms;
	double m_fsd2fi, m_fcd2fi, m_fsd2nfi, m_fcd2nfi;
	CPulseGenerator() { m_dPhase=0.0; m_fFreq=0.0; }
	void SetFrequency(float dFreq, float dSampleRate)
	{
		// static const double PI=4*atan(1.0);
		m_fFreq=(float)(2.0*PI*dFreq/dSampleRate);
		m_fHarms=(float)floor(0.5*3.1415/m_fFreq);
		m_fsd2fi=sin(m_fFreq);m_fcd2fi=cos(m_fFreq);
		m_fsd2nfi=sin(m_fFreq*m_fHarms);m_fcd2nfi=cos(m_fFreq*m_fHarms);
	}
	void GenerateSamples(SIG *pData, int nLen) { 
		// static const double PI=4*atan(1.0);
		double sinFi, cosFi, sinFi2, cosFi2;
		double sinNFi, cosNFi, sinNFi2, cosNFi2;
		double fi=m_dPhase, nfi=m_fHarms*fi;
		sinFi=sin(fi);cosFi=cos(fi);
		sinNFi=sin(m_fHarms*fi);cosNFi=cos(m_fHarms*fi);
		double INVn=2.0/m_fHarms, val;
		m_dPhase+=m_fFreq*nLen;
		if (m_dPhase>=2*PI) m_dPhase=fmod(m_dPhase,2*PI);
		for (int i=0; i<nLen; i++)
		{
			double omcosFi=1.0-cosFi;
			if (omcosFi==0.0)
				val=m_fHarms;
			else
				val=((1-cosNFi)*omcosFi+sinNFi*sinFi)/(omcosFi+omcosFi);
			val-=1.0;
			val*=INVn;

			pData[i]=SIG(val);
			EVOLVE_SINcos(sinFi2,cosFi2,sinFi,cosFi,m_fsd2fi,m_fcd2fi);sinFi=sinFi2;cosFi=cosFi2;
			EVOLVE_SINcos(sinNFi2,cosNFi2,sinNFi,cosNFi,m_fsd2nfi,m_fcd2nfi);sinNFi=sinNFi2;cosNFi=cosNFi2;
		}
	}
};

class CDecayEnvelope {
public:
	double m_fCurState, m_fDecay;

	CDecayEnvelope()
	{
		m_fCurState=0.0;
		m_fDecay=0.0;
	}

	void SetDecayTime(double fTime, double dSampleRate)
	{
		m_fDecay=exp(-2.71/(dSampleRate*fTime));
	}

	SIG ProcessSample(double dTrigger)
	{
		if (dTrigger)
			m_fCurState=dTrigger;
		else
			m_fCurState*=m_fDecay;
		return (float)m_fCurState;
	}
};

class CASREnvelope {
public:
	int m_nState, m_nTime;
  int m_nAttackTime, m_nSustainTime, m_nDecayTime, m_nStageTime;
  double m_fStart, m_fLast, m_fSeries;

	CASREnvelope()
	{
		m_fStart=0.0;
    m_nState=3;
    m_fLast=0.0;
    m_nTime=0;
	}

	void SetEnvelope(double fAttack, double fSustain, double fDecay, double dSampleRate)
	{
    m_nAttackTime=int(fAttack*dSampleRate);
    m_nSustainTime=int(fSustain*dSampleRate);
    m_nDecayTime=int(fDecay*dSampleRate);
	}

  void NoteOn(double dOn)
  {
		if (dOn>0.0)
    {
      m_nStageTime=m_nAttackTime;
      m_fStart=1/4096.0;
      m_fSeries=pow(dOn/m_fStart,1.0/m_nAttackTime);
//      m_fStart=1.0/4096.0;
//      m_fSeries=m_fAttack;
      m_nState=0;
      m_nTime=0;
    }
  }

  void NoteOnGlide(double dOn)
  {
		if (dOn>0.0)
    {
      ProcessSample(0);
      m_nStageTime=m_nAttackTime;
      m_fStart=1/4096.0;
      m_fSeries=pow(dOn/m_fStart,1.0/m_nAttackTime);
      m_nState=0;
      m_nTime=(int)(log(m_fLast/m_fStart)/log(m_fSeries));
      if (m_nTime<0) m_nTime=0;
      if (m_nTime>m_nAttackTime) m_nTime=m_nAttackTime-1;

      /*
      //m_fStart=1/4096.0;
      m_fStart=m_fLast;
      m_fSeries=pow(dOn/m_fStart,1.0/m_nAttackTime);
//      m_fStart=1.0/4096.0;
//      m_fSeries=m_fAttack;
      m_nState=0;
      m_nTime=0;
      */
    }
  }

  void NoteOff()
  {
    m_nTime=0;
    m_nStageTime=m_nDecayTime;
		m_fStart=m_fLast;
    m_fSeries=pow((1.0/4096.0)/m_fLast,1.0/m_nDecayTime);
    if (m_fSeries>0.9999) m_fSeries=0.9999;
    m_nState=2;
  }
  
  int GetTimeLeft()
  {
    if (m_nState<3) return m_nStageTime-m_nTime;
    return 10000;
  }

  SIG ProcessSample(int nDeltaTime)
	{
    float fValue=(float)(m_fStart*pow(m_fSeries,m_nTime));
		m_fLast=fValue;
		m_nTime+=nDeltaTime;
		if (m_nState==0 && m_nTime>=m_nStageTime)
    {
      m_fStart=1.0;
      m_fSeries=1.0;
      m_nState=1;
      m_nTime-=m_nStageTime;
      m_nStageTime=m_nSustainTime;
    }
    if (m_nState==1 && m_nTime>=m_nStageTime)
    {
			m_nTime-=m_nStageTime;
			m_nStageTime=m_nDecayTime;
			m_fStart=1.0;
			m_fSeries=pow((1.0/4096.0)/1.0,1.0/m_nDecayTime);
			if (m_fSeries>0.9999) m_fSeries=0.9999;
			m_nState=2;
    }
    if (m_nState==2 && m_nTime>=m_nStageTime)
    {
      m_fStart=0.0;
      m_nState=3;
      m_nTime=0;
    }
		return fValue;
	}
};

class CADSREnvelope {
public:
	int m_nState, m_nTime;
  int m_nAttackTime, m_nSustainTime, m_nDecayTime, m_nReleaseTime, m_nStageTime;
  double m_fStart, m_fLast, m_fSeries, m_fSustLevel, m_fSilence;

	CADSREnvelope()
	{
		m_fSilence=m_fStart=1/4096.0;
		m_fSeries=1;
    m_nState=4;
    m_fLast=m_fStart;
    m_nTime=0;
    m_fSustLevel=1.0;
	}

	void SetEnvelope(double fAttack, double fDecay, double fSustLevel, double fSustain, double fRelease, double dSampleRate)
	{
    m_nAttackTime=std::max(1,int(fAttack*dSampleRate));
    m_nSustainTime=std::max(1,int(fSustain*dSampleRate));
    m_nDecayTime=std::max(1,int(fDecay*dSampleRate));
    m_nReleaseTime=std::max(1,int(fRelease*dSampleRate));
		m_fSustLevel=fSustLevel;
	}

  void RealNoteOn()
  {
		m_nStageTime=m_nAttackTime;
		m_fLast=m_fStart=0;
	//	m_fSeries=pow(1/m_fSilence,1.0/m_nAttackTime);
		m_fSeries=(1-m_fSilence)/m_nStageTime;
		m_nState=0;
		m_nTime=0;
  }

  void NoteOn()
  {
/*
		m_nStageTime=64;
		m_fStart=__max(m_fSilence,m_fLast);
		m_fSeries=pow(m_fSilence/m_fStart,1.0/m_nStageTime);
		m_nState=-1;
		m_nTime=0;
    */
    RealNoteOn();
  }

  void NoteOnGlide()
  {
    ProcessSample(0);
    m_nStageTime=m_nAttackTime;
    m_fStart=m_fSilence;
    m_fSeries=pow(1.0/m_fStart,1.0/m_nAttackTime);
    m_nState=0;
		m_nTime=(int)(log(m_fLast/m_fStart)/log(m_fSeries));
    if (m_nTime<0) m_nTime=0;
    if (m_nTime>m_nAttackTime) m_nTime=m_nAttackTime-1;
      /*
      //m_fStart=1/4096.0;
      m_fStart=m_fLast;
      m_fSeries=pow(dOn/m_fStart,1.0/m_nAttackTime);
//      m_fStart=1.0/4096.0;
//      m_fSeries=m_fAttack;
      m_nState=0;
      m_nTime=0;
      */
  }

  void NoteOffFast()
  {
		if (m_nState==4 || m_nState==-1)
			return;
		m_nReleaseTime=1024;
		m_nStageTime=m_nReleaseTime;
		m_nTime=0;

		m_nState=-1;
		m_fStart=m_fSustLevel=m_fLast;
		m_fSeries=-m_fStart/m_nReleaseTime;
		//m_fSeries=pow(m_fSilence/m_fSustLevel,1.0/m_nReleaseTime);
  }

  void NoteOff()
  {
		if (m_nState>=3 || m_nState==-1)
			return;
		if (m_nState<2)
		{
			if (m_fLast>=m_fSustLevel)
			{
				// (re)start very short decay from this point, then proceed to release
				m_nSustainTime=0;
				m_nTime=0;
				m_nStageTime=256;
				m_fStart=m_fLast;
				m_fSeries=pow(m_fSustLevel/m_fLast,1.0/m_nStageTime); // go from current point to sustain
				// if (m_fSeries>0.9999) m_fSeries=0.9999;
				m_nState=1;
				return;
			}
			else
				m_fSustLevel=m_fLast; // start release from here
		}
		m_nTime=0;
		m_nStageTime=m_nReleaseTime;
		m_fStart=m_fLast;
		m_fSeries=pow(m_fSilence/m_fSustLevel,1.0/m_nReleaseTime);
		// if (m_fSeries>0.9999) m_fSeries=0.9999;
		m_nState=3;
  }

	inline float Next()
	{
		if (m_nTime<m_nStageTime)
		{
//			float fValue=(float)(m_fStart*pow(m_fSeries,m_nTime));
			float fValue=(float)(m_fLast);
			if (m_nState<1)
				m_fLast+=m_fSeries;
			else
				m_fLast*=m_fSeries;
			m_nTime++;
			return fValue;
		}
		else
			return ProcessSample(1);
	}
  
  int GetTimeLeft()
  {
    if (m_nState<4) return m_nStageTime-m_nTime;
    return 10000;
  }

  SIG ProcessSample(int nDeltaTime)
	{
    float fValue=(float)((m_nState<=0)?(m_fStart+m_nTime*m_fSeries):(m_fStart*pow(m_fSeries,m_nTime)));
		m_nTime+=nDeltaTime;
		if (m_nState==-1 && m_nTime>=m_nStageTime) // sko�czy� si� release
		{
			m_nState=4;
			m_fStart=0;
			m_fSeries=1;
			m_nTime=0;
			m_nStageTime=1000000;
		}
		if (m_nState==0 && m_nTime>=m_nStageTime) // sko�czy� si� attack
    {
			m_nTime-=m_nStageTime;
			m_nState=1;
			m_nStageTime=m_nDecayTime;
			m_fStart=1.0;
			m_fSeries=pow((m_fSustLevel)/1.0,1.0/m_nDecayTime);
			// if (m_fSeries>0.9999) m_fSeries=0.9999;
    }
    if (m_nState==1 && m_nTime>=m_nStageTime) // sko�czy� si� decay zaczyna si� sustain
    {
      m_nTime-=m_nStageTime;
      m_nState=2;
      m_fStart=m_fSustLevel;
      m_fSeries=1.0;
      m_nStageTime=m_nSustainTime;
    }
    if (m_nState==2 && m_nTime>=m_nStageTime) // sko�czy� si� sustain zaczyna si� release
    {
			m_nTime-=m_nStageTime;
			m_nState=3;
			m_nStageTime=m_nReleaseTime;
			m_fStart=m_fSustLevel;
			m_fSeries=pow(m_fSilence/m_fSustLevel,1.0/m_nReleaseTime);
			// if (m_fSeries>0.9999) m_fSeries=0.9999;
    }
    if (m_nState==3 && m_nTime>=m_nStageTime)
    {
      m_fStart=m_fSilence;
      m_nState=4;
      m_nStageTime=10000;
      m_nTime=0;
    }
		if (m_nState<1)
			m_fLast=(float)(m_fStart+m_fSeries*m_nTime);
		else
			m_fLast=(float)(m_fStart*pow(m_fSeries,m_nTime));
		return fValue;
	}
};

inline float CSI(float d0, float d1, float d2, float d3, float fractpart)
{
      return float(((( (((( 3* ( d1 - d2)) - d0 )  + d3 ) * 0.5 * fractpart)+
				    ((2* d2) + d0) - (((5* d1)+ d3)*0.5)          )  * fractpart)
	  			    + ((d2-d0)*0.5) )*fractpart  +  d1);

}

inline float INTERPOLATE(float pos,float start,float end)
{
  return ((start)+(pos)*((end)-(start)));
}

#define LFOPAR2TIME_CHIPS(value) (0.05*pow(800.0,value/255.0))

inline int GetNoOfTicks(int no)
{
  static int times[]={
    1,2,3,4,
    6,8,12,16,
    24,28,32,48,
    64,96,128
  };

  return times[no];
}

inline void LfoRateDesc(char *buf, byte lforate)
{
	if (lforate<240)
    sprintf(buf, "%5.3f Hz", (double)LFOPAR2TIME_CHIPS(lforate));
  else
    sprintf(buf, "%d ticks", GetNoOfTicks(lforate-240));
}

inline void LfoRateDescBFX(char *buf, byte lforate)
{
	if (lforate<240)
    sprintf(buf, "%5.3f Hz", (double)LFOPAR2TIME_CHIPS(lforate));
  else
	{
		int nTicks=GetNoOfTicks(lforate-240);
    if (nTicks&1)
			sprintf(buf, "%d/4 beats", nTicks);
		else
    if (nTicks&2)
			sprintf(buf, "%d/2 beats", nTicks/2);
		else
			sprintf(buf, "%d beats", nTicks/4);
	}
}

inline float LfoRateToDeltaPhase(byte lforate, int TicksPerSec, float SamplesPerSec)
{
  if (lforate<240)
    return (float)(2*3.1415926*LFOPAR2TIME_CHIPS(lforate)/SamplesPerSec);
  else
    return (float)(2*3.1415926*(float(TicksPerSec))/(GetNoOfTicks(lforate-240)*SamplesPerSec));
}

#define HANDLE_PARAM(ptvalName, paraName) if (ptval->ptvalName != para##paraName.NoValue) pt->paraName = ptval->ptvalName;
#define HANDLE_PARAM2(ptvalName) if (ptval->ptvalName != 255) pt->ptvalName = ptval->ptvalName;

inline void Brk()
{
  __asm (" int 0x03; ");
}

inline int DelayLenToSamples(int DelayUnit, int DelayLen, int SamplesPerTick, int SamplesPerSec)
{
  if (DelayUnit==0) return (int)(DelayLen*SamplesPerTick);
  if (DelayUnit==1) return (int)(DelayLen*SamplesPerTick/256);
  if (DelayUnit==2) return (int)(DelayLen);
  if (DelayUnit==3) return (int)(DelayLen*SamplesPerSec/1000);
  return 44100;
}

inline int DelayLenToSamplesBuzzFX(int DelayUnit, int DelayLen, int SamplesPerTick, int SamplesPerSec)
{
  if (DelayUnit==0) return (int)(DelayLen*SamplesPerTick*4/16);
  if (DelayUnit==1) return (int)(DelayLen*SamplesPerTick*4/256);
  if (DelayUnit==2) return (int)(DelayLen);
  if (DelayUnit==3) return (int)(DelayLen*SamplesPerSec/1000);
  return 44100;
}

class CUglyLimiter
{
public:
  float m_fAccum;
  float m_fFactor;
  float m_fAttackCoeff, m_fReleaseCoeff;

  CUglyLimiter(): m_fFactor(1.0f), m_fAccum(0.0f), m_fAttackCoeff(0.99f), m_fReleaseCoeff(1.01f) {}

  inline void AvoidExceptions()
  {
		if (m_fAccum<0.0001f)
			m_fAccum=0.0f;
		if (m_fFactor<0.00001f)
			m_fFactor=0.00001f;
  }
  inline float ProcessSample(float fSmp)
  {
    m_fAccum=0.99f*m_fAccum+0.01f*fSmp*fSmp;
    if (m_fAccum*m_fFactor*m_fFactor>32767*32767)
      m_fFactor*=m_fAttackCoeff;
    else
      if (m_fFactor<1.0f)
        m_fFactor=std::min(m_fFactor*m_fReleaseCoeff,1.0f);
      
    return fSmp*m_fFactor;
  }
};

class C6thOrderFilter
{
public:
  CBiquad m_filter, m_filter2, m_filter3;

  float CurCutoff;
  float Resonance;
  float ThevFactor;

  inline float ProcessSample(float fSmp) { return m_filter3.ProcessSample(m_filter2.ProcessSample(m_filter.ProcessSample(fSmp))); }
  
  static inline int GetFilterCount() { return 15; }
  static void GetFilterDesc(char *txt, int value)
  {
    if (value==0) strcpy(txt,"6L Multipeak");
    if (value==1) strcpy(txt,"6L Separated");
    if (value==2) strcpy(txt,"6L HiSquelch");
    if (value==3) strcpy(txt,"4L Skull D");
    if (value==4) strcpy(txt,"4L TwinPeaks");
    if (value==5) strcpy(txt,"4L Killah");
    if (value==6) strcpy(txt,"4L Phlatt");
    if (value==7) strcpy(txt,"2L Phlatt");
    if (value==8) strcpy(txt,"2L FrontFlt");
    if (value==9) strcpy(txt,"2L LaserOne");
    if (value==10) strcpy(txt,"2L FMish");
    if (value==11) strcpy(txt,"Notchez");
    if (value==12) strcpy(txt,"6H Relaxed");
    if (value==13) strcpy(txt,"6B Plain");
    if (value==14) strcpy(txt,"6X BatGuy");
    if (value==15) strcpy(txt,"6X Vocal1");
    if (value==16) strcpy(txt,"6X Vocal2");
    if (value==17) strcpy(txt,"No Filter");
  }

  void ResetFilter()
  {
    m_filter.Reset();
    m_filter2.Reset();
    m_filter3.Reset();
  }
  bool IsSilent()
  {
    return m_filter.IsSilent() && m_filter2.IsSilent() && m_filter3.IsSilent();
  }
  void AvoidExceptions()
  {
    m_filter.AvoidExceptions();
    m_filter2.AvoidExceptions();
    m_filter3.AvoidExceptions();
  }

  void CalcCoeffs(int nType, float _CurCutoff, float _Resonance, float _ThevFactor=0.0f);

  void CalcCoeffs1();
  void CalcCoeffs2();
  void CalcCoeffs3();
  void CalcCoeffs4();
  void CalcCoeffs5();
  void CalcCoeffs6();
  void CalcCoeffs7();
  void CalcCoeffs8();
  void CalcCoeffs9();
  void CalcCoeffs10();
  void CalcCoeffs11();
  void CalcCoeffs12();
  void CalcCoeffs13();
  void CalcCoeffs14();
  void CalcCoeffs15();
  void CalcCoeffs16();
  void CalcCoeffs17();
  void CalcCoeffs18();
};

template <class T> void LameBandlimiter(T *table, int init_size, int levels)
{
  for (int i=0; i<levels; i++)
  {
    for (int j=0; j<(init_size>>1); j++)
      table[init_size+j]=table[j<<1];
    table+=init_size;
    init_size>>=1;
  }
}

template <class T> void LeetBandlimiter(T *table, int init_size, int levels)
{
	float filter[33];
	for (int i=-16; i<=16; i++)
	{
		if (!i)
			filter[16]=0.5f;
		else
			filter[i+16]=(float)sin(PI*i/2)/(PI*i);
	}
  for (int i=0; i<levels; i++)
  {
    for (int j=0; j<(init_size>>1); j++)
		{
			float accum=0.0;
			for (int k=-16; k<=16; k++)
				accum+=filter[k+16]*table[(j+j+k)&(init_size-1)];
			table[init_size+j]=accum;
		}
    table+=init_size;
    init_size>>=1;
  }
}

inline void MakeSincLowpass(float *vector, int samples, double cutoff) // USELESS CRAP
{
	for (int i=0; i<samples; i++)
	{
		float pt=float(i-(samples-1)/2.0);
		if (!pt)
			vector[i]=float(cutoff);
		else
			vector[i]=(float)(sin(PI*cutoff*pt)/(PI*pt));
	}
}

inline void HammingWindow(float *vector, int samples) // USELESS CRAP
{
	for (int i=0; i<samples; i++)
	{
		float pt=float(i-(samples-1)/2.0);
		vector[i]*=float(0.54+0.46*cos(pt*2*PI/(samples-1)));
	}
}

#define I0_EPS 0.000000000001
inline double I0(double x)
{
  int n=1;
  double S=1, D=1, T;
  while(D>I0_EPS*S)
  {
    T=x/(2*n++);
    D*=T*T;
    S+=D;
  }
	return S;
}

inline void KaiserWindow(float *vector, int samples, float alpha) // USELESS CRAP - except maybe for windowing FFT stuff
{
  float M=float((samples-1)/2.0);
  double den=I0(alpha);
	for (int n=0; n<samples; n++)
    vector[n]*=float(I0(alpha*sqrt(n*(samples-1-n))/M)/den);
}


#define WAVE_POSITION_BITS 31

inline int f2i(double d)
{
#ifdef WIN32
	const double magic = 6755399441055744.0; // 2^51 + 2^52
	double tmp = d + magic;
	return *(int*) &tmp;
#else
    // poor man's solution :)
    return rint(d);
#endif
}

inline double myfmod(double n, double d)
{
  return n-d*f2i(n*(1.0/d));
}

class CAnyWaveLevel
{
public:
  float *m_pData;
  int m_nSize;
	int m_nBits;
  float m_fMaxScanRate;
	float m_fMultiplier;

/*
#define NO_CROSSINGS 16

	float m_fTable[4096*NO_CROSSINGS];

	CAnyWaveLevel()
	{
		m_fTable[0]=1.0f;
		for (int i=1; i<4096*NO_CROSSINGS; i++)
			m_fTable[i]=(float)(sin(PI*i/4096)/(PI*i/4096))*(0.54+0.46*cos(PI*i/(4096*NO_CROSSINGS)));
	}
	*/

	static inline float WavePositionMultiplier()
	{
		return (float)pow(2.0f,31.0f);
	}
	
	inline float GetWaveAt_Cubic(int nPos) // nPos=0..(1<<WAVE_POSITION_BITS)-1
	{
		nPos&=0x7FFFFFFF;
		int nWavePos=nPos>>(31-m_nBits);
		int nRemainder=nPos-(nWavePos<<(31-m_nBits));
		float d0=m_pData[nWavePos+0];
		float d1=m_pData[nWavePos+1];
		float d2=m_pData[nWavePos+2];
		float d3=m_pData[nWavePos+3];
		return CSI(d0,d1,d2,d3,float(nRemainder*m_fMultiplier));
	}
	/*
  // USELESS CRAP
	inline float GetWaveAt_Sinc(int nPos) // nPos=0..(1<<WAVE_POSITION_BITS)-1
	{
		nPos&=0x7FFFFFFF;
		int nWavePos=nPos>>(31-m_nBits);
		int nRemainder=nPos-(nWavePos<<(31-m_nBits));

		float accum=0.0;
		float frac=nRemainder*m_fMultiplier;
		for (int i=-NO_CROSSINGS+1; i<=NO_CROSSINGS; i++)
		{
			float phase=(frac-i);
			float data=m_pData[(nWavePos+i)&(m_nSize-1)];
			if (phase>-NO_CROSSINGS && phase<NO_CROSSINGS)
			{
				int nPlace=abs(f2i(phase*4096));
				accum+=data*m_fTable[nPlace];
			}
//				accum+=data*sin(phase)/(phase)*(0.5+0.5*cos(phase/6));
		}
		return accum;
	}
	*/
	inline float GetWaveAt_Linear(int nPos) // nPos=0..(1<<WAVE_POSITION_BITS)-1
	{
		nPos&=0x7FFFFFFF;
		int nWavePos=nPos>>(31-m_nBits);
		int nRemainder=nPos-(nWavePos<<(31-m_nBits));
		float d0=m_pData[(nWavePos+0)&(m_nSize-1)];
		float d1=m_pData[(nWavePos+1)&(m_nSize-1)];
		return INTERPOLATE(float(nRemainder*m_fMultiplier),d0,d1);
	}
};

class CBandlimitedTable
{
public:
  float *m_pBuffer;
  int m_nBufSize;

  CBandlimitedTable();
  ~CBandlimitedTable();
  CAnyWaveLevel *GetTable(float fScanRate);// fScanRate = 1.0f dla cz�stotliwo�ci jeden okres/pr�bk�
  void Make(float fMultiplyFactor, float fMaxScanRate, float fCrispFactor=-1);

protected:
  CAnyWaveLevel m_levels[128];
  int m_nLevels;
};

inline float CalcLFO(int nShape, float LFOPhase)
{
  float Phs=(float)myfmod(LFOPhase,2*PI);
  float LFO=0.0f;
  switch(nShape)
  {
    case 0: LFO=(float)sin(Phs); break;
    case 1: LFO=(float)(((Phs-PI)/PI-0.5f)*2.0f); break;
    case 2: LFO=(float)(((Phs-PI)/PI-0.5f)*-2.0f); break;
    case 3: LFO=(Phs<PI)?1.0f:-1.0f; break;
    case 4: LFO=(float)(((Phs<PI?(Phs/PI):(2.0-Phs/PI))-0.5)*2); break;
    case 5: LFO=(float)sin(Phs*8*sin(Phs)); break;
    case 6: LFO=(float)sin(Phs*2*pow(2,cos(Phs))); break;
    case 7: LFO=(float)sin(Phs*pow(4,sin(Phs))); break;
    case 8: LFO=(float)(0.5*sin(2*Phs)+0.5*cos(3*Phs)); break;
    case 9: LFO=(float)(0.25*f2i(((Phs-PI)/PI-0.5f)*2.0f*4.0)); break;
    case 10: LFO=(float)(-0.25*f2i(((Phs-PI)/PI-0.5f)*2.0f*4.0)); break;
    case 11: LFO=(float)(0.125*f2i(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(Phs,PI/4)/(PI/4)); break;
    case 12: LFO=(float)(-0.125*f2i(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(Phs,PI/4)/(PI/4)); break;
    case 13: LFO=(float)(0.125*f2i(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(2*PI-Phs,PI/4)/(PI/4)); break;
    case 14: LFO=(float)(-0.125*f2i(((Phs-PI)/PI-0.5f)*2.0f*4.0)+0.5*fmod(2*PI-Phs,PI/4)/(PI/4)); break;
    case 15: LFO=(float)(0.5*(sin(19123*f2i(LFOPhase*8/PI)+40*sin(12*f2i(LFOPhase*8/PI))))); break; // 8 zmian/takt
    case 16: LFO=(float)(0.5*(sin(1239543*f2i(LFOPhase*4/PI)+40*sin(15*f2i(LFOPhase*16/PI))))); break; // 8 zmian/takt
  }
  return LFO;
}

extern int intsinetable[2048];

class CPWMLFO
{
public:
  int m_nPhase;
  int m_nDeltaPhase;
  int m_nDepth;

  inline void Set(int nFrequency, int nDepth) { m_nDeltaPhase=nFrequency; m_nDepth=nDepth; }
  inline int GetSample()
  {
    m_nPhase+=m_nDeltaPhase;
    int nTabPhase=(m_nPhase>>17);
    float pt=INTERPOLATE(float((m_nPhase&65535)*(1.0f/65536.0f)),(float)intsinetable[nTabPhase&2047],(float)intsinetable[(nTabPhase+1)&2047]);
    return f2i(m_nDepth*pt/32768.0);
  }
};

inline int BuzzToSeq(int value)
{
  if (value==NOTE_NO || value==NOTE_OFF) return value;
  return ((value-1)>>4)*12+(value&15);
}

inline int SeqToBuzz(int value)
{
  if (value==NOTE_NO || value==NOTE_OFF) return value;
  return (((value-1)/12)<<4)+((value-1)%12)+1;
}

inline float PitchQuantize(float fFrequency)
{
	float fPitch=float(log(fFrequency/220.0)/log(pow(2.0,1.0/12.0)));
	
	float fInt=float(floor(fPitch+0.5));
	float fFrac=fPitch-fInt;

	fFrac=0.0f;
	
	return float(220*pow(2.0,(fInt+fFrac)/12.0));
}

inline float Glide(float Frequency, float DestFrequency, float GlideCoeff, int c)
{
	if (!GlideCoeff)
		return DestFrequency;
	else
	{
    if (Frequency==DestFrequency)
      return Frequency;

		double glide=pow(1.01,c*(1-pow(GlideCoeff/241.0,0.03)));
		if (Frequency<DestFrequency)
		{
			Frequency*=(float)glide;
			if (Frequency>DestFrequency)
				Frequency=DestFrequency;
		}
		else
		{
			Frequency/=(float)glide;
			if (Frequency<DestFrequency)
				Frequency=DestFrequency;
		}
    return Frequency;
	}
}

inline float Distort(float fIn, float *pTable, int nSize)
{
	fIn=nSize/2+fIn*nSize/65536;
	if (fIn>=nSize) fIn=float(nSize-1);
	if (fIn<0) fIn=0.f;
	int ipart=f2i(fIn);
	float fpart=fIn-ipart;
	return 32767*(pTable[ipart]+fpart*(pTable[ipart+1]-pTable[ipart]));
}
