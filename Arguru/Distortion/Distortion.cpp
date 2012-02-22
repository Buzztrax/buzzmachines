#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <MachineInterface.h>

CMachineParameter const paraPreGain = 
{
      pt_word,
      "Input Gain",
      "Input Gain",
      0x0001,
      0x0800,
      0xFFFF,
      MPF_STATE,
      0x0100,
};
CMachineParameter const paraThresholdNeg =
{
      pt_word,
      "Threshold (-)",
      "Threshold level (negative)",
      0x0001,
      0x8000,
      0xFFFF,
      MPF_STATE,
      0x200,
};
CMachineParameter const paraThreshold =
{
      pt_word,
      "Threshold (+)",
      "Threshold level (positive)",
      0x0001,
      0x8000,
      0xFFFF,
      MPF_STATE,
      0x200
};
CMachineParameter const paraGain =
{
      pt_word,
      "Output Gain",
      "Output Gain",
      0x0001,
      0x0800,
      0xFFFF,
      MPF_STATE,
      0x0400
};
CMachineParameter const paraInvert =
{
      pt_byte,
      "Phase inversor",
      "Stereo phase inversor",
      0x00,
      0x01,
      0xFF,
      MPF_STATE,
      0x00
};
CMachineParameter const paraMode =
{
      pt_byte,
      "Mode",
      "Operational mode",
      0x00,
      0x01,
      0xFF,
      MPF_STATE,
      0x00
};

CMachineParameter const *pParameters[] =
{
	// global
	&paraPreGain,
	&paraThresholdNeg,
	&paraThreshold,
	&paraGain,
	&paraInvert,
	&paraMode,
};

#pragma pack(1)

class gvals
{
public:
  word paraPreGain;
  word paraThresholdNeg;
  word paraThreshold;
  word paraGain;
  byte paraInvert;
  byte paraMode;
};

#pragma pack()

CMachineInfo const MacInfo =
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	6,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	0,
#ifdef _DEBUG
	"Arguru Distortion (Debug build)",				// name
#else
	"Arguru Distortion",					// name
#endif
	"Distortion",									// short name
	"Arguru",									// author
	NULL
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);
	virtual char const *DescribeValue(int const param, int value);

private:
  gvals gval;

  inline void Clip(float *psamplesleft, float const threshold, 
		   float const negthreshold, float const wet);
  inline void Saturate(float *psamplesleft, float const threshold,
		       float const negthreshold, float const wet);

  float leftLim;
  float rightLim;
  int Vals[6];
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
}

void mi::Init(CMachineDataInput * const pi)
{
}

void mi::Tick()
{
  if (gval.paraPreGain != paraPreGain.NoValue) {
    Vals[4] = gval.paraPreGain;
  }
  if (gval.paraThresholdNeg != paraThresholdNeg.NoValue) {
    Vals[5] = gval.paraThresholdNeg;
  }
  if (gval.paraThreshold != paraThreshold.NoValue) {
    Vals[0] = gval.paraThreshold;
  }
  if (gval.paraGain != paraGain.NoValue) {
    Vals[1] = gval.paraGain;
  }
  if (gval.paraInvert != paraInvert.NoValue) {
    Vals[2] = gval.paraInvert;
  }
  if (gval.paraMode != paraMode.NoValue) {
    Vals[3] = gval.paraMode;
  }
}

inline void mi::Clip(float *psamplesleft, float const threshold, 
			     float const negthreshold, float const wet ) {
  float sl = *psamplesleft;
  if (sl > threshold)
    sl = threshold;
  if (sl <= negthreshold)
    sl = negthreshold;
  *psamplesleft = sl * wet;
}

inline void mi::Saturate(float * psamplesleft, float const threshold, 
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

bool mi::Work(float *psamples, int numsamples, int const mode)
{
  float const threshold = (float)(Vals[0]) / float(0x8000);
  float const negthreshold = -(float)(Vals[5]) / float(0x8000);

  float const wet = (float)Vals[1] * 0.00390625f;

  float const pre_gain = (float)Vals[4] * 0.00390625f;
  
  for (int i = 0; i < numsamples; i++) {
    psamples[i] = psamples[i] * pre_gain;
  }

  if (Vals[3] == 0) {
    if (Vals[2] == 0) {
      // Clip, No Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Clip(&psamples[i], threshold, negthreshold, wet);
      }
    } else {
      // Clip, Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Clip(&psamples[i], threshold, negthreshold, wet);
      }
    }
  } else {
    if (Vals[2] == 0) {
      // Saturate, No Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Saturate(&psamples[i], threshold, negthreshold, wet);
      }
    } else {
      // Saturate, Phase inversion
      for (int i = 0; i < numsamples; i++) {
	Saturate(&psamples[i], threshold, negthreshold, wet);
      }
    } 
  }
  return true;
}
/*
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
*/

char const *mi::DescribeValue(int const param, int value) {
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
