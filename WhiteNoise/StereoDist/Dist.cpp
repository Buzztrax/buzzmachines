//#include "stdafx.h"

#include <math.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

#define MIN_AMP					(0.0001 * (32768.0 / 0x7fffffff))

CMachineParameter const paraDrive = { 
	pt_byte, 
	"Drive", 
	"Pre distortion drive (-32 .. +64 db)", 
	0, 
	96, 
	255, 
	MPF_STATE,
	32
};

CMachineParameter const paraThresholdT = { 
	pt_byte, 
	"Top", 
	"Top Threshold", 
	0, 
	248, 
	255, 
	MPF_STATE, 
	254
};
	
CMachineParameter const paraThresholdB = { 
	pt_byte, 
	"Bottom", 
	"Bottom Threshold", 
	0, 
	248, 
	255, 
	MPF_STATE, 
	254
};

CMachineParameter const paraModeT = { 
	pt_byte, 
	"Top Mode", 
	"Top Mode (Limit, Zero, Rectify, Pixelate, Saturate, Wrap)", 
	0, 
	5, 
	255, 
	MPF_STATE, 
	0
};

CMachineParameter const paraModeB = { 
	pt_byte, 
	"Bottom Mode", 
	"Bottom Mode (Limit, Zero, Rectify, Pixelate, Saturate, Wrap)", 
	0, 
	5, 
	255, 
	MPF_STATE, 
	0
};

CMachineParameter const paraLowCut = { 
	pt_word, 
	"Lowpass", 
	"Lowpass", 
	0, 
	1000, 
	0xffff, 
	MPF_STATE, 
	1000
};

CMachineParameter const paraHiCut = { 
	pt_word, 
	"Hipass", 
	"Hipass", 
	0, 
	1000, 
	0xffff, 
	MPF_STATE, 
	0
};

CMachineParameter const paraDry = { 
	pt_byte, 
	"Dry", 
	"Dry volume", 
	0, 
	96, 
	0xff, 
	MPF_STATE, 
	48
};

CMachineParameter const paraWet = { 
	pt_byte, 
	"Wet", 
	"Wet volume", 
	0, 
	96, 
	0xff, 
	MPF_STATE, 
	48
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraDrive, 
	&paraThresholdT,
	&paraThresholdB,
	&paraModeT,
	&paraModeB,
	&paraLowCut,
	&paraHiCut,
	&paraDry,
	&paraWet
};


#pragma pack(1)	

class gvals
{
public:
	byte drive;
	byte thresholdt;
	byte thresholdb;
	byte modet;
	byte modeb;
	word lowcut;
	word hicut;
	byte dry;
	byte wet;
};

/*		maybe an attribute for saturate
CMachineAttribute const attrMethod =
{
        "Method (Not Used yet)",
        0,
        2,
        1
};


CMachineAttribute const *pAttributes[] =
{
        &attrMethod
};
*/

static double nonlinTab[256];

class avals
{
public:
        int method;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,					// flags
	0,										// min tracks
	0,										// max tracks
	9,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,		// pAttributes
	"WhiteNoise Stereodist",					// name
	"StereoDist",									// short name
	"David Wallin",						// author
	NULL
};

class miex : public CMDKMachineInterfaceEx
{

};

class mi : public CMDKMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Tick();

	virtual void MDKInit(CMachineDataInput * const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
	virtual char const *DescribeValue(int const param, int const value);
	virtual void Command(int const i);

	virtual void MDKSave(CMachineDataOutput * const po);

public:
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
	virtual void OutputModeChanged(bool stereo) {}

	inline int MSToSamples(double const ms);
	inline void LowFilter(double &input);
	inline void HiFilter(double &input);

	inline void LowFilterStereo(double &inputL, double &inputR);
	inline void HiFilterStereo(double &inputL, double &inputR);
public:
	miex ex;


public:
	double threshold_top;
	double threshold_bottom;

	int mode_top;
	int mode_bottom;

	float buf0l, buf1l,	// Lowpass buffers
		  buf0r, buf1r,
		  buf2l, buf3l,	// Hipass buffers
		  buf2r, buf3r;

	float HiCutoff, LowCutoff;

	double drive;
	double wet_vol;
	double dry_vol;


	gvals gval;
//	avals aval;
};

inline double LInterpolateF(double x1, double x2, double F)			// Res: 4096
{
	return x1 + (x2 - x1)*F;
}									


mi::mi()
{
	GlobalVals = &gval;
    //AttrVals = (int *)&aval;
	AttrVals = NULL;
}


mi::~mi()
{

}

void mi::MDKInit(CMachineDataInput * const pi)
{
	int i;

	// Generate nonlinTab
/*
	for(i=0; i<128; i++)
	{	
		nonlinTab[i] = -(pow( fabs((float)i-127.0)*128.0, 1.5)/150.0);

		if(nonlinTab[i] < -30000)
		{																		
			nonlinTab[i] = -30000 - pow((-nonlinTab[i] + 30000.0), 0.82);
		}
	}
*/
	HiCutoff = 0.01f;
	LowCutoff = 0.999f;
	dry_vol = 1.0f;
	wet_vol = 1.0f;

	buf0l = 0.0;
	buf1l = 0.0;
	buf2l = 0.0;
	buf3l = 0.0;

	buf0r = 0.0;
	buf1r = 0.0;
	buf2r = 0.0;
	buf3r = 0.0;

	for(i=0; i<256; i++)
	{	
		nonlinTab[i] = (pow((float)(i*256.0), 1.5)/160.0);

		if(nonlinTab[i] > 30000)
		{
			nonlinTab[i] = 30000 + pow((nonlinTab[i] - 30000.0), 0.82);
		}
	}
}

void mi::MDKSave(CMachineDataOutput * const po)
{

}

void mi::Tick()
{
	if(gval.thresholdt != paraThresholdT.NoValue)
	{
		threshold_top = pow(10,((gval.thresholdt-200)*0.25)/20)*32768;
	}

	if(gval.thresholdb != paraThresholdB.NoValue)
	{
		threshold_bottom = -pow(10,((gval.thresholdb-200)*0.25)/20)*32768;
	}

	if(gval.modet != paraModeT.NoValue)
		mode_top = gval.modet;

	if(gval.modeb != paraModeB.NoValue)
		mode_bottom = gval.modeb;

	if(gval.drive != paraDrive.NoValue)
	{
		drive = pow(10,((double)(gval.drive-32)/20.0) );
	}

	if(gval.hicut != paraHiCut.NoValue)
	{
		//HiCutoff = (float)gval.hicut / 1001.0;
		HiCutoff = pow((gval.hicut/1001.0)*sqrt(0.999), 2.0);
	}

	if(gval.lowcut != paraLowCut.NoValue)
	{
		//LowCutoff = (float)gval.lowcut / 1001.0;
		LowCutoff = pow((gval.lowcut/1001.0)*sqrt(0.999), 2.0);
	}

	if(gval.dry != paraDry.NoValue)
	{
		dry_vol = pow(10, ((double)(gval.dry-64))/20.0);
	}

	if(gval.wet != paraWet.NoValue)
	{
		wet_vol = pow(10, ((double)(gval.wet-64))/20.0);
	}
}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	if (!(mode & WM_READ) || !(mode & WM_WRITE))
	{
		return false;
	}
	
	do
	{
		double f, s;

		f = s = *psamples;

		HiFilter(f);
		LowFilter(f);

		f = f * drive;

		//"Top Mode (Limit, Zero, Rectify, Pixelate, Saturate)", 
		if(f > threshold_top)
			switch(mode_top)
			{
				case 1:
					f = 0;
					break;
				case 2:
					f = threshold_top - (f - threshold_top);
					break;
				case 3:
					f = ((int)f & ~4095);
					break;
				case 4:
					f = threshold_top + (f - threshold_top) / ((f*2.0) / threshold_top);
					break;
				case 5:
					do
					{			
						f = threshold_top - (f - threshold_top);

						if(f < threshold_bottom)
						{
							f = threshold_bottom + (threshold_bottom - f);
						}
						
					} while(f > threshold_top);
					break;

				default:
					f = threshold_top;
					break;
			}

		if(f < threshold_bottom)
			switch(mode_bottom)
			{
				case 1:
					f = 0;
					break;
				case 2:
					f = threshold_bottom + (threshold_bottom - f);
					break;
				case 3:
					f = ((int)f & ~4095);
					break;
				case 4:
					f = threshold_bottom - (threshold_bottom - f) / ((f * 2.0) / threshold_bottom);
					break;
				case 5:
					do
					{
						f = threshold_bottom + (threshold_bottom - f);

						if(f > threshold_top)
						{
							f = threshold_top - (f - threshold_top);
						}
					} while (f < threshold_bottom);
					break;

				default:
					f = threshold_bottom;
					break;
			}

		//f -= DC;
		
		*psamples++ = (float)(s*dry_vol) + (float)(f*wet_vol);

	} while(--numsamples);

	return true;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if (!(mode & WM_READ) || !(mode & WM_WRITE))
	{
		return false;
	}
	
	do
	{
		double l = psamples[0];
		double r = psamples[1];
		double fl, fr;		// filter left and right

		fl = l;
		fr = r;

		HiFilterStereo(fl, fr);
		LowFilterStereo(fl, fr);

		fl = fl * drive;
		fr = fr * drive;

		if(fl > threshold_top)
			switch(mode_top)
			{
				case 1:
					fl = 0;
					break;
				case 2:
					fl = threshold_top - (fl - threshold_top);
					break;
				case 3:
					fl = ((int)fl & ~4095);
					break;
				case 4:
					fl = threshold_top + (fl - threshold_top) / ((fl*2.0) / threshold_top);
					break;
				case 5:
					do
					{			
						fl = threshold_top - (fl - threshold_top);

						if(fl < threshold_bottom)
						{
							fl = threshold_bottom + (threshold_bottom - fl);
						}
						
					} while(fl > threshold_top);
					break;
				default:
					fl = threshold_top;
					break;
			}

		if(fl < threshold_bottom)
			switch(mode_bottom)
			{
				case 1:
					fl = 0;
					break;
				case 2:
					fl = threshold_bottom + (threshold_bottom - fl);
					break;
				case 3:
					fl = ((int)fl & ~4095);
					break;
				case 4:
					fl = threshold_bottom - (threshold_bottom - fl) / ((fl*2.0) / threshold_bottom);
					break;
				case 5:
					do
					{
						fl = threshold_bottom + (threshold_bottom - fl);

						if(fl > threshold_top)
						{
							fl = threshold_top - (fl - threshold_top);
						}
					} while (fl < threshold_bottom);
					break;

				default:
					fl = threshold_bottom;
					break;
			}
		if(fr > threshold_top)
			switch(mode_top)
			{
				case 1:
					fr = 0;
					break;
				case 2:
					fr = threshold_top - (fr - threshold_top);
					break;
				case 3:
					fr = ((int)fr & ~4095);
					break;
				case 4:
					fr = threshold_top + (fr - threshold_top) / ((fr*2.0) / threshold_top);
					break;
				case 5:
					do
					{			
						fr = threshold_top - (fr - threshold_top);

						if(fr < threshold_bottom)
						{
							fr = threshold_bottom + (threshold_bottom - fr);
						}
						
					} while(fr > threshold_top);
					break;
				default:
					fr = threshold_top;
					break;
			}

		if(fr < threshold_bottom)
			switch(mode_bottom)
			{
				case 1:
					fr = 0;
					break;
				case 2:
					fr = threshold_bottom + (threshold_bottom - fr);
					break;
				case 3:
					fr = -((int)fr & ~4095);
					break;
				case 4:
					fr = threshold_bottom - (threshold_bottom - fr) / ((fr*2.0) / threshold_bottom);
					break;
				case 5:
					do
					{
						fr = threshold_bottom + (threshold_bottom - fr);

						if(fr > threshold_top)
						{
							fr = threshold_top - (fr - threshold_top);
						}
					} while (fr < threshold_bottom);
					break;
				default:
					fr = threshold_bottom;
					break;
			}

		// Set buffer to result, mix dry and wet

		//fl -= DC;
		//fr -= DC;

		psamples[0] = (float)(l*dry_vol) + (float)(fl*wet_vol);
		psamples[1] = (float)(r*dry_vol) + (float)(fr*wet_vol);

		psamples += 2;

	} while(--numsamples);

	return true;
}

void mi::Command(int const i)
{
}

DLL_EXPORTS

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{

	case 0:	
		sprintf(txt, "%d db", value-32);
		break;
	case 1:	
		sprintf(txt, "%d db", value-200);
		break;
	case 2:	
		sprintf(txt, "%d db", value-200);
		break;
	case 3:
	case 4:
			//"Top Mode (Limit, Zero, Rectify, Pixelate, Saturate)", 
		switch(value)
		{
		case 0:
			sprintf(txt, "Clip");
			break;
		case 1:
			sprintf(txt, "Zero");
			break;
		case 2:
			sprintf(txt, "Rectify");
			break;
		case 3:
			sprintf(txt, "Pixelate");
			break;
		case 4:
			sprintf(txt, "Saturate");
			break;
		case 5:
			sprintf(txt, "Wrap");
			break;
		}
		break;
	case 7:		
		sprintf(txt, "%d db", value-64);
		break;
	case 8:	
		sprintf(txt, "%d db", value-64);
		break;
	default:
		return NULL;
	}

	return txt;
}

//		- For Reference -
inline void mi::LowFilter(double &input)
{
  double fa = double(1.0 - LowCutoff); 
  double fb = 0.0;						// Optimize
  buf0l = fa * buf0l + LowCutoff * (input + fb * (buf0l - buf1l)); 
  buf1l = fa * buf1l + LowCutoff * buf0l;
  input = buf1l;
}

inline void mi::HiFilter(double &input)
{
  double fa = double(1.0 - HiCutoff); 
  double fb = 0.0;		// float(1.0 + (1.0/fa))
  buf2l = fa * buf2l + HiCutoff * (input + fb * (buf2l - buf3l)); 
  buf3l = fa * buf3l + HiCutoff * buf2l;
  input = input - buf3l;
}

inline void mi::LowFilterStereo(double &inputL, double &inputR)
{
  double fa = double(1.0 - LowCutoff);	// Optimize
  double fb = 0.0;						// Optimize

  buf0l = fa * buf0l + LowCutoff * (inputL + fb * (buf0l - buf1l)); 
  buf1l = fa * buf1l + LowCutoff * buf0l;
  inputL = buf1l;

  buf0r = fa * buf0r + LowCutoff * (inputR + fb * (buf0r - buf1r)); 
  buf1r = fa * buf1r + LowCutoff * buf0r;
  inputR = buf1r;
}

inline void mi::HiFilterStereo(double &inputL, double &inputR)
{
  double fa = double(1.0 - HiCutoff); 
  double fb = 0.0;		// float(1.0 + (1.0/fa))

  buf2l = fa * buf2l + HiCutoff * (inputL + fb * (buf2l - buf3l)); 
  buf3l = fa * buf3l + HiCutoff * buf2l;
  inputL -= buf3l;

  buf2r = fa * buf2r + HiCutoff * (inputR + fb * (buf2r - buf3r)); 
  buf3r = fa * buf3r + HiCutoff * buf2r;
  inputR -= buf3r;
}
