#include <cstdio>
#include <cmath>
#include <algorithm>

#include "Compressor.hpp"

using namespace std;

Compressor::Compressor() {
  global_values = &gval;
  attributes = 0;
  track_values = 0;
  currentGain = 1.0f;
  currentSR = 0;
}

Compressor::~Compressor() {
  delete this;
}

void Compressor::init(zzub::archive* pi) {
  currentSR = _master_info->samples_per_second;
}

void Compressor::destroy() {

}

void Compressor::process_events() {
  if (gval.paraGain != paraGain->value_none) {
    Vals[paramGain] = gval.paraGain;
  }
  if (gval.paraThreshold != paraThreshold->value_none) {
    Vals[paramThreshold] = gval.paraThreshold;
  }
  if (gval.paraRatio != paraRatio->value_none) {
    Vals[paramRatio] = gval.paraRatio;
  }
  if (gval.paraAttack != paraAttack->value_none) {
    Vals[paramAttack] = gval.paraAttack;
  }
  if (gval.paraRelease != paraRelease->value_none) {
    Vals[paramRelease] = gval.paraRelease;
  }
  if (gval.paraClip != paraClip->value_none) {
    Vals[paramClip] = gval.paraClip;
  }
}

bool Compressor::process_stereo(float **pin, float **pout, 
				int numsamples, int mode) {
  //float const corrected_gain = (Vals[paramGain] * 0.015625000f + 1.0f);
  float *psamplesleft = pin[0];
  float *psamplesright = pin[1];
  float *pleft = psamplesleft;
  float *pright = psamplesright;
  float gain = float(Vals[paramGain]) / float(paraGain->value_max) * 4.8;
  gain = deci_bell_to_linear(gain);
  for (int i = 0; i < numsamples; i++) {
    pleft[i] = pleft[i] * gain;
    pright[i] = pright[i] * gain;
  }
  if (Vals[paramRatio] != 0) {
    float const correctedthreshold = Vals[paramThreshold] * 0.0078125000f;
    double const corrected_ratio = (Vals[paramRatio] < 16) ? 
      1.0f / (1.0f + Vals[paramRatio]) : 0.0f;
    double const attackconst = 
      1.0 / ((1.0 + Vals[paramAttack]) * currentSR * 0.001);
    double const releaseconst = 
      1.0 / ((1.0 + Vals[paramRelease]) * currentSR * 0.001);
    float* pleft = psamplesleft;
    float* pright = psamplesright;
    for (int cont = numsamples; cont > 0; cont--) {
      double targetGain;
      double const analyzedValue = std::max(fabs(*pleft), fabs(*pright));
      if (analyzedValue <= correctedthreshold) {
	targetGain = 1.0f;
      }
      else {
	targetGain = ((analyzedValue - correctedthreshold) * 
		      corrected_ratio + correctedthreshold) / analyzedValue;
      }
      double newgain = (targetGain - currentGain);
      if (targetGain < currentGain) {
	newgain *= attackconst;
      }
      else {
	newgain *= releaseconst;
      }
      currentGain += newgain;
      *(pleft++) *= currentGain;
      *(pright++) *= currentGain;
    }
  }
  if (Vals[paramClip] != 0) {
    float *pleft = psamplesleft;
    float *pright = psamplesright;
    for (int cont = numsamples; cont > 0; cont--) {
      *pleft = tanh(*pleft);
      *pright = tanh(*pright);
      pleft++;
      pright++;
    }
  }
  for (int i = 0; i < numsamples; i++) {
    pout[0][i] = pleft[i];
    pout[1][i] = pright[i];
  }
  return true;
}

const char *Compressor::describe_value(int param, int value) {
  static char txt[20];
  switch(param) {
  case paramGain:
    std::sprintf(txt,"+%.1f dB", linear_to_deci_bell(1 + value * 0.015625f));
    return txt;
  case paramThreshold:
    if(value == 0)
      std::sprintf(txt,"-inf. dB");
    else
      std::sprintf(txt,"%.1f dB", linear_to_deci_bell(value * 0.0078125f));
    return txt;
  case paramRatio:
    if(value >= 16)
      std::sprintf(txt,"1:inf (Limiter)");
    else if(value == 0)
      std::sprintf(txt,"1:1 (Bypass)");
    else
      std::sprintf(txt,"1:%d", value + 1);
    return txt;
  case paramAttack: //fallthrough
  case paramRelease:
    std::sprintf(txt,"%d ms", value);
    return txt;
  case paramClip:
    std::sprintf(txt, value == 0 ? "Off" : "On");
    return txt;
  default: 
    return 0;
  }
}
