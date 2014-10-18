// Stereo limiter

//20:08 < ld0d> no copyright, no responsibility, no restrictions
//20:08 < ld0d> but you can gpl them once you're done with them

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <mdk/mdk.h>


#define WINDOWLEN 64

CMachineParameter const paraBypass =
{
        pt_switch,                                                                                // type
        "Bypass",
        "Bypass",                                                                  // description
        SWITCH_OFF,                                                                                              // Min
        SWITCH_ON,                                                                                    // Max
        SWITCH_NO,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        SWITCH_OFF                                                                                               // default
};


CMachineParameter const paraInputGain =
{
        pt_byte,                                                                                // type
        "Input Gain",
        "Input Gain",                                                                  // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        128                                                                                               // default
};


CMachineParameter const paraTreshold =
{
        pt_byte,                                                                                // type
        "Treshold",
        "Treshold",                                                                  // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        128                                                                                               // default
};

CMachineParameter const paraOutput =
{
        pt_byte,                                                                                // type
        "Output",
        "Output",                                                                  // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        128                                                                                               // default
};

CMachineParameter const paraRelease =
{
        pt_byte,                                                                                // type
        "Release",
        "Release",                                                                  // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        32                                                                                               // default
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraBypass,
	&paraInputGain,
	&paraTreshold,
	&paraOutput,
	&paraRelease
};

#pragma pack(1)		

class gvals
{
public:
	byte bypass;
	byte inputgain;
	byte treshold;
	byte output;
	byte release;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,						// flags
	0,										// min tracks
	0,										// max tracks
	5,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	0,
#ifdef _DEBUG
   "ld slimit (Debug build)",			// name
#else
   "ld slimit",							// name
#endif
	"slimit",									// short name
	"Lauri Koponen",						// author
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

	virtual void MDKInit(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);

	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
	virtual void OutputModeChanged(bool stereo) {}
	virtual void MDKSave(CMachineDataOutput * const po) {}

	char const *DescribeValue(int const param, int const value);

private:
			
	miex ex;


private:

	void reset(void);

	int bypass;
	double input, treshold, output, gain;
	double release;


	double w[WINDOWLEN], rw[WINDOWLEN]; /* attack window, release window */
	int rp; /* release window position */

	double p[WINDOWLEN]; /* compression buffer */
	double d[WINDOWLEN*2]; /* delay buffer */
	int pp; /* compression/delay buffer position */

	int idle;

	
	gvals gval;

};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
}

void mi::reset(void)
{
	memset(p, 0, WINDOWLEN*sizeof(double));
	memset(d, 0, 2*WINDOWLEN*sizeof(double));
	pp = 0;
	rp = 0;
	idle = 0;
}

void mi::MDKInit(CMachineDataInput * const pi)
{
  SetOutputMode( true ); // No mono sounds
	treshold = 1.0;
	output = 1.0;

	for(int i = 0; i < WINDOWLEN; i++) {
		w[i] = cos((double)(i+1) * M_PI / (double)(WINDOWLEN+1)) * 0.5 + 0.5;
	}

	rp = WINDOWLEN - 1;
	pp = 0;
}

float clamp(float x)
{
	if(x < 0) return 0;
	else if(x > 1) return 1;

	return x;
}

void mi::Tick()
{
	if(gval.bypass != 255) {
		bypass = gval.bypass;
	}

	if(gval.inputgain != 255) {
		input = pow(10.0, ((double)(gval.inputgain-128) * 6.0 / 16.0) / 20.0);
	}

	if(gval.treshold != 255) {
		treshold = pow(10.0, ((double)(gval.treshold-128) * 6.0 / 16.0) / 20.0);
		gain = output / treshold;
	}

	if(gval.output != 255) {
		output = pow(10.0, ((double)(gval.output-128) * 6.0 / 16.0) / 20.0);
		gain = output / treshold;
	}

	if(gval.release != 255) {
		double releasems = pow(10.0, (double)(gval.release-48) / 32.0);
		release = exp(-1.0/(releasems*(44100.0/1000.0)));

		for(int i = 0; i < WINDOWLEN; i++) {
			rw[i] = release * pow(1 / release, -cos((double)(i+1) * 0.5 * M_PI / (double)(WINDOWLEN+1)) + 1);
		}
	}
}


bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
  return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	/*if(mode == WM_NOIO) {
		reset();
		return false;
	}

	if(mode == WM_WRITE) {
		idle = WINDOWLEN;
	}*/
	if(mode == WM_WRITE || mode == WM_NOIO)
		return false;

	if(mode == WM_READ || bypass) return true;


	int i, i1;

	i1 = pp;
	i = (i1 + 1) & (WINDOWLEN - 1);

	while(numsamples--) {

		p[i1] = p[(i1-1) & (WINDOWLEN - 1)] * rw[rp];
		if(rp) rp--;

		double a1 = fabs(*psamples);
		double a2 = fabs(*(psamples + 1));
		double peak;
		if(a1 > a2)
			peak = a1 * (1.0 / 32768.0) - treshold;
		else
			peak = a2 * (1.0 / 32768.0) - treshold;

		if(peak > p[i1]) {
			/* attack */

			double diff = (treshold + p[i1]) / (treshold + peak);

			int j = WINDOWLEN - 1;
			int k = i1 + 1;
			double *wp = w;
			while(j--) {
				double g = (*wp) * (1 - diff) + diff;
				d[(k & (WINDOWLEN - 1)) * 2] *= g;
				d[(k & (WINDOWLEN - 1)) * 2 + 1] *= g;
				k++;
				wp++;
			}
			p[i1] = peak;
			rp = WINDOWLEN - 1;
		}

		d[i1 * 2] = (*psamples) * gain * (treshold / (p[i1] + treshold));
		d[i1 * 2 + 1] = (*(psamples+1)) * gain * (treshold / (p[i1] + treshold));

		*psamples = d[i * 2];
		psamples++;
		*psamples = d[i * 2 + 1];
		psamples++;

		i1 = i;
		i = (i + 1) & (WINDOWLEN - 1);
	}

	pp = i1;
	
	return true;
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char buf[256];
	switch(param) {
	case 0:
		if(value == SWITCH_OFF) return "Off";
		if(value == SWITCH_ON) return "On";
	case 1: /* input gain */
	case 2: /* treshold */
	case 3: /* output */
		sprintf(buf, "%1.3f dB", ((double)(value-128) * 6.0 / 16.0));
		return buf;
	case 4: /* release time */
		sprintf(buf, "%1.3f ms", pow(10.0, ((double)(value-48) / 32.0)));
		return buf;
	}

	return NULL;
}
