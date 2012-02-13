#include <cstdio>
#include <cmath>
#include <algorithm>

#include "Distortion.hpp"

using namespace std;

Distortion::Distortion() {
  global_values = &gval;
  attributes = 0;
  track_values = 0;
}

Distortion::~Distortion() {
  delete this;
}

void Distortion::init(zzub::archive* pi) {

}

void Distortion::destroy() {

}

void Distortion::process_events() {
  if (gval.paraPreGain != paraPreGain->value_none) {
    Vals[4] = gval.paraPreGain;
  }
  if (gval.paraThresholdNeg != paraThresholdNeg->value_none) {
    Vals[5] = gval.paraThresholdNeg;
  }
  if (gval.paraThreshold != paraThreshold->value_none) {
    Vals[0] = gval.paraThreshold;
  }
  if (gval.paraGain != paraGain->value_none) {
    Vals[1] = gval.paraGain;
  }
  if (gval.paraInvert != paraInvert->value_none) {
    Vals[2] = gval.paraInvert;
  }
  if (gval.paraMode != paraMode->value_none) {
    Vals[3] = gval.paraMode;
  }
}

inline void Distortion::Clip(float *psamplesleft, float const threshold, 
			     float const negthreshold, float const wet ) {
  float sl = *psamplesleft;
  if (sl > threshold)
    sl = threshold;
  if (sl <= negthreshold)
    sl = negthreshold;
  *psamplesleft = sl * wet;
}

inline void Distortion::Saturate(float * psamplesleft, float const threshold, 
				 float const negthreshold, float const wet) {
  const float s_in = (*psamplesleft);
  float sl = s_in * leftLim;
  if (sl > threshold) {
    leftLim -= (sl - threshold) * 0.1f / s_in;
  } else if (sl <= negthreshold) {
    leftLim -= (sl - negthreshold) * 0.1f / s_in;
  } else if (leftLim > 1.0f) {
    leftLim = 1.0f;
  } else if (leftLim < 1.0f) 
    leftLim = 0.01f + (leftLim * 0.99f);
  *psamplesleft = sl * wet;
}

bool Distortion::process_stereo(float **pin, float **pout, 
				int numsamples, int mode) {
  float *psamplesleft = pin[0];
  float *psamplesright = pin[1];

  float const threshold = (float)(Vals[0]) / float(0x8000);
  float const negthreshold = -(float)(Vals[5]) / float(0x8000);

  float const wet = (float)Vals[1] * 0.00390625f;
  float const negwet = -(float)Vals[1] * 0.00390625f;

  float const pre_gain = (float)Vals[4] * 0.00390625f;
  
  for (int i = 0; i < numsamples; i++) {
    pin[0][i] = pin[0][i] * pre_gain;
    pin[1][i] = pin[1][i] * pre_gain;
  }

  if (Vals[3] == 0) {
    if (Vals[2] == 0) {
      // Clip, No Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Clip(&pin[0][i], threshold, negthreshold, wet);
	Clip(&pin[1][i], threshold, negthreshold, wet);
      }
    } else {
      // Clip, Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Clip(&pin[0][i], threshold, negthreshold, wet);
	Clip(&pin[1][i], threshold, negthreshold, negwet);
      }
    }
  } else {
    if (Vals[2] == 0) {
      // Saturate, No Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Saturate(&pin[0][i], threshold, negthreshold, wet);
	Saturate(&pin[1][i], threshold, negthreshold, wet);
      }
    } else {
      // Saturate, Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Saturate(&pin[0][i], threshold, negthreshold, wet);
	Saturate(&pin[1][i], threshold, negthreshold, negwet);
      }
    } 
  }
  for (int i = 0; i < numsamples; i++) {
    pout[0][i] = psamplesleft[i];
    pout[1][i] = psamplesright[i];
  }
  return true;
}

const char *Distortion::describe_value(int param, int value) {
  static char txt[20];
  switch(param) {
  case 0: {
    if(value > 0)
      sprintf(txt, "%.1f dB", 20.0f * log10(value * 0.00390625f));
    else
      sprintf(txt, "-Inf. dB");				
    return txt;
  }
  case 1: {
    if(value > 0)
      sprintf(txt, "%.1f dB", 20.0f * log10(value * 0.000030517578125f));
    else
      sprintf(txt,"-Inf. dB");				
    return txt;
  }
  case 2: {
    if(value > 0)
      sprintf(txt, "%.1f dB", 20.0f * log10(value * 0.000030517578125f));
    else
      sprintf(txt,"-Inf. dB");				
    return txt;
  }
  case 3: {
    if(value > 0)
      sprintf(txt, "%.1f dB", 20.0f * log10(value * 0.00390625f));
    else
      sprintf(txt, "-Inf. dB");				
    return txt;
  }
  case 4: {
    switch(value) {
      case 0:
	sprintf(txt, "Off");	
	break;
      case 1:
	sprintf(txt, "On");	
	break;
    }
    return txt;
  }
  case 5: {
    switch(value) {
    case 0:
      sprintf(txt, "Clip");	
      break;
    case 1:
      sprintf(txt, "Saturate");
      break;
    }
    return txt;
  }
  default: 
    return 0;
  }
}
