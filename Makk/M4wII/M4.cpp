// MAKE INTERFACE

// Overdrive mode working right??? verify

// wavelevel =  level = (int)ceil(log(step) * (1.0 / log(2)); 

// - Inertia for Phase2	   ?
// Rampsaw lfo pattern
// correct wavelevel for pitch mod effects? Go by phase add?
// LFO freq - NTRIG - (for random wave, picks new val each note, for others, picks random freq+phase) each time a note is hit?

// Interpolated random mode

// attrib: Waveosc path (after filter before amp? after amp?)

// poss. mixmodes w/ waveosc?

// Fix bug in m4w


// M4 Buzz plugin by MAKK makk@gmx.de
// released in July 1999
// formulas for the filters by Robert Bristow-Johnson pbjrbj@viconet.com
// a.k.a. robert@audioheads.com

// M4 Fixes

// [X] proper fadeouts, fadeins
// [X] Release filter too?
// [ ] make Volume settings actually affect volume while it is playing?

// M4w

// [X] Added ?, AM, AM2, Pixelate to mixmodes
// [X] Ability to lock lfo's to oscillators to do FM and other effects
// [X] LFO1 -> Voltage, mod cutoff
// [X] Opt. Built in distortion after filter
// [X] Implement Playmode - Opt. Retrig envelopes, sync lfos 
// [X] Add inertia on the filter. See Inertia in attributes
// [X] Two new LFO waves, stepup and stepdn, good for arpeggiators
// [X] Fixed Sync(!)
// [X] Fixed Bug with LFO2 phase difference being the same as LFO1
// [X] new filters: 24 db bandpass, 24 db peak, 24 db hipass

// M4wII

// [x] New noise mode - noise2 - PW affects color
// [x] Pitch Bend
// [x]   Pitch bend only works when glide is on
// [X] New Interface
// [x] removed Pitch envelope, added ADSR User envelope (bindable to many things including pitch)
// [x] LFO amounts are now track specific, so you can use the UEG to do envelopes on LFO's (for FM and other stuff)
// [x] ADSR Envelopes
// [x] Wave oscillator - fixed or keytracking
// [x] You can not do pitch effects on the waveosc, but it can be affected by amp and filter effects
// [x] Support for buzz bandlimited oscillators (0-5) (These waves won't alias much)
// [x]   These oscillators can use cubic spline interpolation for maximum quality. Default is linear interpolation
// [x] Overdriven mixmode (pre filter) for moog like phatness
// [x] Midi input, w/ velocity, channel control, ect.
// [x] Wavegain parameter (for more/less distortion)
// [x] noinit filter playmode
// [x] optmized some stuff
// [x] Awesome new filter types - Very realistic, but a bit more cpu intensive
// [X] Vibrato mode


// Plans:

// [ ] Interface
// [ ] New LFO patterns and waveforms
// [ ] Mod Wheel functions
// [ ] settable mix path for wave osc (after filter? after amp?)		< should be an attribute..
// [ ] Amp Gain for control over distortion effects.

// Opt:[ ] Add guru Filter or asdev filt?


/// m5w
// [ ] Better modulation of parameters
// [ ] or  Seperate instrument files, evelopes instead of adsr


 //static inline float	fscale( float r, long i )
//{
	//i=(*(long *)&r)+(i<<23);
 	//return *(float *)&i;
//}

typedef struct {
	int size;
	int mask;
	int sh;
	int offset;
	int maskshift;
} bWaveInfo;

#define NOTECONST	1.05946309436

#define NUMWAVES	43
#define NOISE1		1
#define NOISE2		2

#define WAVE_STEPUP		33
#define WAVE_STEPDN		34
#define WAVE_WACKY1		41
#define WAVE_WACKY2		9

#define VC_EXTRALEAN			// Exclude rarely-used stuff from Windows headers

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "filter.h"

#define		AMPCONV		7.45058059692e-9


#include <MachineInterface.h>
#include <dsplib.h>
#ifdef WIN32
#include "cubic.h"
#include "editordlg.h"
#endif

#pragma optimize ("a", on)

#define MAX_TRACKS                              8

#define EGS_NONE                                0
#define EGS_ATTACK                              1
#define EGS_DECAY	                            2
#define EGS_SUSTAIN                             3
#define EGS_RELEASE                             4
#define EGS_DONE								5
#define RESOLUTION								4096

static float coefsTab[4*128*128*8];
static float LFOOscTab[0x10000];
static float LFOVibOscTab[0x10000];

static int nonlinTab[256];

inline int f2i(double d)
{
#ifdef WIN32
  const double magic = 6755399441055744.0; // 2^51 + 2^52
  double tmp = (d-0.5) + magic;
  return *(int*) &tmp;
#else
  // poor man's solution :)
  return (int)rint(d);
#endif
}

inline int LInterpolateI(int x1, int x2, long frac)			// Res: 256
{
	return x1 + (((x2 - x1) * frac)>>8);
}									

inline int LInterpolateF(int x1, int x2, long frac)			// Res: 4096
{
	float F = (float)frac*0.000244140625;

	return x1 + f2i((float)(x2 - x1)*F);
}									


long at[RESOLUTION];
long bt[RESOLUTION];
long ct[RESOLUTION];

void InitSpline()
{
  for (int i=0;i<RESOLUTION;i++)
  {
    float x = (float)i/(float)RESOLUTION;
    at[i] = (0.5*x*x*x*256);
    bt[i] = (x*x*256);
    //ct[i] = -1.5*x*x*x+2.0*x*x+0.5;
    ct[i] = (0.5*x*256);
  }
}

inline void prewarp(
    double *a0, double *a1, double *a2,
    double fc, double fs)
{
    double wp, pi;

    pi = (4.0 * atan(1.0));			// constant
    wp = (2.0 * fs * tan(pi * fc / fs));	// table for tan?

    *a2 = (*a2) / (wp * wp);		// could turn one of these divides into a multiply
    *a1 = (*a1) / wp;
}

inline void bilinear(
    double a0, double a1, double a2,	/* numerator coefficients */
    double b0, double b1, double b2,	/* denominator coefficients */
    double *k,           /* overall gain factor */
    double fs,           /* sampling rate */
    double *coef         /* pointer to 4 iir coefficients */
)
{
    double ad, bd;
    ad = (4. * a2 * fs * fs + 2. * a1 * fs + a0);		// could get rid of a few of these multiplies
    bd = (4. * b2 * fs * fs + 2. * b1* fs + b0);
    *k *= ad/bd;
    *coef++ = ((2. * b0 - 8. * b2 * fs * fs) / bd);			/* beta1 */
    *coef++ = ((4. * b2 * fs * fs - 2. * b1 * fs + b0) / bd); /* beta2 */
    *coef++ = ((2. * a0 - 8. * a2 * fs * fs) / ad);			/* alpha1 */
    *coef = ((4. * a2 * fs * fs - 2. * a1 * fs + a0) / ad);	/* alpha2 */
}

inline void szxform(
    double *a0, double *a1, double *a2,     /* numerator coefficients */
    double *b0, double *b1, double *b2,		/* denominator coefficients */
    double fc,								/* Filter cutoff frequency */
    double fs,								/* sampling rate */
    double *k,								/* overall gain factor */
    double *coef)							/* pointer to 4 iir coefficients */
{
	prewarp(a0, a1, a2, fc, fs);
	prewarp(b0, b1, b2, fc, fs);
	bilinear(*a0, *a1, *a2, *b0, *b1, *b2, k, fs, coef);
}

//inline float SplineInterp(float yo, float y0, float y1, float y2,unsigned __int32 res)
//{
	//return at[res]*yo+bt[res]*y0+ct[res]*y1+dt[res]*y2;	
//}
//float x = (float)r/4096.0;
//float a=(3*(y0-y1)-yo+y2)*0.5;
//float b=2*y1+yo-(5*y0+y2)*0.5;
//float c=(y1-yo)*0.5;
//return a*x*x*x + b*x*x + c*x + y0;

int SplineInterp(int yo, int y0, int y1, int y2, int r)
{
//float x = (float)r/4096.0;
int d=y0-y1;
int a=((int)(d<<1)+d-yo+y2);
int b=((int)(y1<<1) + yo - (int)((5*y0+y2)>>1));
int c=(y1-yo);
return (((a*at[r]) + (b*bt[r]) + (c*ct[r]))>>8) + y0;
}


extern short waves[];

CMachineParameter const paraNote =
{
        pt_note,                                                                                // type
        "Note",
        "Note",                                                                     // description
        NOTE_MIN,                                                                               // Min
        NOTE_MAX,                                                                               // Max
        0,                                                                                   // NoValue
        0,                                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraMode =
{
        pt_byte,                                                                              // type
        "PlayMode",
        "PlayMode",                                    // description
        0,                                                                             // Min
        8,                                                                              // Max
        0xFF,                                                                              // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                              // default
};

CMachineParameter const paraPitchWheel =
{
        pt_byte,                                                                                // type
        "Pitchwheel",
        "PitchWheel",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        64                                                                                            // default
};

CMachineParameter const paraModWheel =
{
        pt_byte,                                                                                // type
        "ModWheel",
        "ModWheel",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                            // default
};
							
CMachineParameter const paraPitchBendAmt =
{
        pt_byte,                                                                                // type
        "Bend",
        "Pitch Bend Amount",                                             // description
        0,                                                                                              // Min
        12,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        2                                                                                            // default
};

CMachineParameter const paraWavetableOsc = 
{ 
	pt_byte,																			// type
	"Wavetable",
	"Wavetable wave",	// description
	0,												// MinValue	
	0xfe,  											// MaxValue
	0xff,    										// NoValue
	MPF_WAVE | MPF_STATE,							// Flags
	0
};

CMachineParameter const paraPhase2 = 
{ 
	pt_byte,																			// type
	"Phase2",
	"Osc2 Phase",	// description
	0,												// MinValue	
	127,  											// MaxValue
	0xff,    										// NoValue
	MPF_STATE,							// Flags
	0
};


CMachineParameter const paraModDest1 = 
{ 
	pt_byte,																			// type
	"ModDest1",
	"Modulation Destination1",	// description
	0,												// MinValue	
	8,  											// MaxValue
	0xff,    										// NoValue
	MPF_STATE,							// Flags
	0
};

CMachineParameter const paraModAmount1 = 
{ 
	pt_byte,																			// type
	"ModAmnt1",
	"Modulation Amount 1",	// description
	0,												// MinValue	
	127,  											// MaxValue
	0xff,    										// NoValue
	MPF_STATE,							// Flags
	0x40
};

CMachineParameter const paraModDest2 = 
{ 
	pt_byte,																			// type
	"ModDest2",
	"Modulation Destination2",	// description
	0,												// MinValue	
	11,  											// MaxValue
	0xff,    										// NoValue
	MPF_STATE,							// Flags
	0
};

CMachineParameter const paraModAmount2 = 
{ 
	pt_byte,																			// type
	"ModAmnt2",
	"Modulation Amount 2",	// description
	0,												// MinValue	
	127,  											// MaxValue
	0xff,    										// NoValue
	MPF_STATE,							// Flags
	0x40
};


CMachineParameter const paraFixedPitch =
{
        pt_switch,                                                                              // type
        "Fixed",
        "Fixed pitch?",                                    // description
        SWITCH_OFF,                                                                             // Min
        SWITCH_ON,                                                                              // Max
        SWITCH_NO,                                                                              // NoValue
        MPF_STATE,                                                                              // Flags
        SWITCH_OFF                                                                              // default
};

CMachineParameter const paraWaveDetuneSemi=
{
        pt_byte,                                                                                // type
        "Wave SemiDet",
        "Wavetable osc Semi Detune in Halfnotes",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraWave1 =
{
        pt_byte,                                                                                // type
        "Osc1 Wave",
        "Oscillator 1 Waveform",                                                // description
        0,                                                                                              // Min
        NUMWAVES+6,                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraPulseWidth1 =
{
        pt_byte,                                                                                // type
        "Osc1 PW",
        "Oscillator 1 Pulse Width",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                            // default
};

CMachineParameter const paraWave2 =
{
        pt_byte,                                                                                // type
        "Osc2 Wave",
        "Oscillator 2 Waveform",                                                // description
        0,                                                                                              // Min
        NUMWAVES+6,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraPulseWidth2 =
{
        pt_byte,                                                                                // type
        "Osc2 PW",
        "Oscillator 2 Pulse Width",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};


CMachineParameter const paraMix =
{
        pt_byte,                                                                                // type
        "Osc Mix",
        "Oscillator Mix (Osc1 <-> Osc2)",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraMixType =
{
        pt_byte,                                                                                // type
        "Osc MixType",
        "Oscillator Mix Type",                                                                              // description
        0,                                                                                              // Min
        13,                                                                                              // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};



CMachineParameter const paraSync =
{
        pt_switch,                                                                              // type
        "Sync",
        "Sync",            // description
        SWITCH_OFF,                                                                             // Min
        SWITCH_ON,                                                                              // Max
        SWITCH_NO,                                                                              // NoValue
        MPF_STATE,                                                                              // Flags
        SWITCH_OFF                                                                              // default
};


CMachineParameter const paraDetuneSemi=
{
        pt_byte,                                                                                // type
        "Osc2 SemiDet",
        "Oscillator 2 Semi Detune in Halfnotes (40h=0)",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraDetuneFine=
{
        pt_byte,                                                                                // type
        "Osc2 FineDet",
        "Oscillator 2 Fine Detune (40h=0)",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x50                                                                                            // default
};

CMachineParameter const paraGlide =
{
        pt_byte,                                                                                // type
        "Pitch Glide",
        "Pitch Glide Amount",                                                                                // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraSubOscWave =
{
        pt_byte,                                                                                // type
        "SubOsc Wave",
        "Sub Oscillator Waveform",                                              // description
        0,                                                                                              // Min
        NUMWAVES+4,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraSubOscVol =
{
        pt_byte,                                                                                // type
        "SubOsc Vol",
        "Sub Oscillator Volume",                                                // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraVolume =
{
        pt_byte,                                                                                // type
        "Volume",
        "Volume (Sustain-Level)",                                               // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraAEGAttackTime =
{
        pt_byte,                                                                                // type
        "Amp Attack",
        "Amplitude Envelope Attack Time",                                                // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        5                                                                                              // default
};

CMachineParameter const paraAEGDecayTime =
{
        pt_byte,                                                                                // type
        "Amp Decay",
        "Amplitude Envelope Decay Time",                                               // description
        0,                                                                                              // Min
        127,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        5                                                                                              // default
};

CMachineParameter const paraAEGSustainTime =
{
        pt_byte,                                                                                // type
        "Amp Sustain",
        "Amplitude Envelope Sustain Time",                                               // description
        0,                                                                                              // Min
        128,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x10                                                                                              // default
};

CMachineParameter const paraAEGSustainLevel =
{
        pt_byte,                                                                                // type
        "Amp Level",
        "Amplitude Envelope Sustain Level",                                               // description
        0,                                                                                              // Min
        127,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        127                                                                                              // default
};

CMachineParameter const paraAEGReleaseTime =
{
        pt_byte,                                                                                // type
        "Amp Release",
        "Amplitude Envelope Release Time",                                               // description
        0,                                                                                              // Min
        127,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x20                                                                                              // default
};

CMachineParameter const paraFilterType =
{
        pt_byte,                                                                                // type
        "Filter Type",
        "Filter Type",                  // description
		//... 0=LowPass24  1=LowPass18  2=LowPass12  3=HighPass  4=BandPass 5=BandReject 6=BP24 7=Peak 8=HP24
        0,                                                                                              // Min
        13,                                                                              // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        2 // LP12                                                                                               // default
};

CMachineParameter const paraDist =
{
        pt_byte,                                                                              // type
        "Dist",
        "Distortion mode (0=off):",                                    // description
        0,                                                                             // Min
        4,                                                                              // Max
        0xff,                                                                              // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                              // default
};

CMachineParameter const paraCutoff =
{
        pt_byte,                                                                                // type
        "Filter Cutoff",
        "Filter Cutoff Frequency",                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        32                                                                                             // default
};

CMachineParameter const paraResonance =
{
        pt_byte,                                                                                // type
        "Filter Q/BW",
        "Filter Resonance/Bandwidth",                                   // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        32                                                                                              // default
};

CMachineParameter const paraUEGAttackTime =
{
        pt_byte,                                                                                // type
        "UEG Attack",
        "User Envelope Attack Time",                                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        7                                                                                               // default
};

CMachineParameter const paraUEGSustainTime =
{
        pt_byte,                                                                                // type
        "UEG Sustain",
        "User Envelope Sustain Time",                                                             // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0b                                                                                               // default
};

CMachineParameter const paraUEGSustainLevel =
{
        pt_byte,                                                                                // type
        "UEG Level",
        "User Envelope Sustain Level",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        127                                                                                               // default
};


CMachineParameter const paraUEGDecayTime =
{
        pt_byte,                                                                                // type
        "UEG Decay",
        "User Envelope Decay Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0b                                                                                               // default
};

CMachineParameter const paraUEGReleaseTime =
{
        pt_byte,                                                                                // type
        "UEG Release",
        "User Envelope Release Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0b                                                                                               // default
};

CMachineParameter const paraUEnvMod =
{							
		pt_byte,                                                                                // type
        "UEnvMod",
        "User Envelope modulation amount",                                    // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                            // default
};

CMachineParameter const paraUEGDest =
{							
		pt_byte,                                                                                // type
        "UEG Dest",
        "User Envelope destination",                                    // description
        0,                                                                                              // Min
        10,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                            // default
};



CMachineParameter const paraFEGAttackTime =
{
        pt_byte,                                                                                // type
        "Filter Attack",
        "Filter Envelope Attack Time",                                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        7                                                                                               // default
};


CMachineParameter const paraFEGDecayTime =
{
        pt_byte,                                                                                // type
        "Filter Decay",
        "Filter Envelope Decay Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0f                                                                                               // default
};

CMachineParameter const paraFEGSustainTime =
{
        pt_byte,                                                                                // type
        "Filter Sustain",
        "Filter Envelope Sustain Time",                                                             // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0e                                                                                               // default
};

CMachineParameter const paraFEGSustainLevel =
{
        pt_byte,                                                                                // type
        "Filter Level",
        "Filter Envelope Sustain Level",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        127                                                                                               // default
};



CMachineParameter const paraFEGReleaseTime =
{
        pt_byte,                                                                                // type
        "Filter Release",
        "Filter Envelope Release Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0f                                                                                               // default
};


CMachineParameter const paraFEnvMod =
{
        pt_byte,                                                                                // type
        "Filter EnvMod",
        "Filter Envelope Modulation ... <40h neg. EnvMod  40h=no EnvMod  >40h pos. EnvMod",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40+32                                                                                            // default
};

// LFOs
CMachineParameter const paraLFO1Dest =
{
        pt_byte,                                                                                // type
        "LFO1 Dest",
        "Low Frequency Oscillator 1 Destination",                                                             // description
        0,                                                                                              // Min
        17,                                                                                     // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1Wave =
{
        pt_byte,                                                                                // type
        "LFO1 Wave",
        "Low Frequency Oscillator 1 Waveform",                                                                // description
        0,                                                                                              // Min
        8,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1Freq =
{
        pt_byte,                                                                                // type
        "LFO1 Freq",
        "Low Frequency Oscillator 1 Frequency",                                                               // description
        0,                                                                                              // Min
        128,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1AttackA =
{
        pt_byte,                                                                                // type
        "Depth Attack",
        "Depth Attack",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1Delay =
{
        pt_byte,                                                                                // type
        "Depth Attack",
        "Depth Attack",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};


CMachineParameter const paraLFO1Amount =
{
        pt_byte,                                                                                // type
        "LFO1 Amount",
        "Low Frequency Oscillator 1 Amount",                                                                  // description
        0,                                                                                              // Min
        255,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

// lfo2
CMachineParameter const paraLFO2Dest =
{
        pt_byte,                                                                                // type
        "LFO2 Dest",
        "Low Frequency Oscillator 2 Destination",                                                             // description
        0,                                                                                              // Min
        16,                                                                                     // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO2Wave =
{
        pt_byte,                                                                                // type
        "LFO2 Wave",
        "Low Frequency Oscillator 2 Waveform",                                                                // description
        0,                                                                                              // Min
        8,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO2Freq =
{
        pt_byte,                                                                                // type
        "LFO2 Freq",
        "Low Frequency Oscillator 2 Frequency",                                                               // description
        0,                                                                                              // Min
        129,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO2Amount =
{
        pt_byte,                                                                                // type
        "LFO2 Amount",
        "Low Frequency Oscillator 2 Amount",                                                                  // description
        0,                                                                                              // Min
        255,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1PhaseDiff =
{
        pt_byte,                                                                                // type
        "LFO1 Ph Diff",
        "Low Frequency Oscillator 1 Phase Difference: 00h=0°  40h=180°  7Fh=357°",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40
};
CMachineParameter const paraLFO2PhaseDiff =
{
        pt_byte,                                                                                // type
        "LFO2 Ph Diff",
        "Low Frequency Oscillator 2 Phase Difference: 00h=0°  40h=180°  7Fh=357°",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40
};

CMachineParameter const paraAmpGain =
{
        pt_byte,                                                                                // type
        "Amp Gain",
        "Amp Gain (for more distortion, or just to normalize the volume)",                                                             // description
        0,                                                                                              // Min
        200,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        32                                                                                               // default
};



// ATTRIBUTES

CMachineAttribute const attrLFO1ScaleOsc1 =
{
        "LFO1 Oscillator1 Scale",
            0,
        127,
        127
};

CMachineAttribute const attrLFO1ScalePW1 =
{
        "LFO1 PulseWidth1 Scale",
            0,
        127,
        127
};

CMachineAttribute const attrLFO1ScaleVolume =
{
        "LFO1 Volume Scale",
            0,
        127,
        127
};

CMachineAttribute const attrLFO1ScaleCutoff =
{
        "LFO1 Cutoff Scale",
            0,
        127,
        127
};

CMachineAttribute const attrLFO2ScaleOsc2 =
{
        "LFO2 Oscillator2 Scale",
            0,
        127,
        127
};

CMachineAttribute const attrLFO2ScalePW2 =
{
        "LFO2 PulseWidth2 Scale",
            0,
        127,
        127
};

CMachineAttribute const attrLFO2ScaleMix =
{
        "LFO2 Mix Scale",
        0,
        127,
        127
};

CMachineAttribute const attrLFO2ScaleReso =
{
        "LFO2 Resonance Scale",
            0,
        127,
        127
};

CMachineAttribute const attrFilterInertia =
{
        "Filter Inertia",
        0,
        256,
        1
};

CMachineAttribute const attrMidiChannel =
{
        "MidiChannel",
        0,
        16,
        0
};

CMachineAttribute const attrMidiTranspose =
{
        "MidiTranspose (-24..24)",
        0,
        48,
        24
};


CMachineAttribute const attrMidiVelocity =
{
        "MidiVelocity (0=Ignore,1=Volume,2=Mod)",
        0,
        2,
        1
};

CMachineAttribute const attrInterpolation =
{
        "Interpolation Mode (0 = None, 1 = Linear, 2 = Spline",
        0,
        2,
        1
};


CMachineAttribute const *pAttributes[] =
{
        &attrLFO1ScaleOsc1,
        &attrLFO1ScalePW1,
        &attrLFO1ScaleVolume,
        &attrLFO1ScaleCutoff,
        &attrLFO2ScaleOsc2,
        &attrLFO2ScalePW2,
        &attrLFO2ScaleMix,
        &attrLFO2ScaleReso,
		&attrFilterInertia,
		&attrMidiChannel,
		&attrMidiTranspose,
		&attrMidiVelocity,
		&attrInterpolation
};



CMachineParameter const *pParameters[] = {
		&paraMode,				// 0

		&paraModWheel,			// 1
		&paraPitchWheel,		// 2
		&paraPitchBendAmt,		// 3

        &paraGlide,				// 4

		&paraWavetableOsc,		// 5
		&paraFixedPitch,		// 6
		&paraWaveDetuneSemi,	// 7

        &paraWave1,				// 8
        &paraPulseWidth1,		// 9

        &paraWave2,				// 10
        &paraPulseWidth2,		// 11
        &paraDetuneSemi,		// 12
        &paraDetuneFine,		// 13

        &paraSync,				// 14
        &paraMixType,			// 15
        &paraMix,				// 16
        &paraSubOscWave,		// 17
        &paraSubOscVol,			// 18

        &paraUEGAttackTime,			// 19
        &paraUEGDecayTime,			// 20
		&paraUEGSustainTime,		// 21
		&paraUEGSustainLevel,		// 22
		&paraUEGReleaseTime,		// 23
        &paraUEnvMod,				// 24

        &paraAEGAttackTime,			// 25
		&paraAEGDecayTime,			// 26
        &paraAEGSustainTime,		// 27
        &paraAEGSustainLevel,		// 28
        &paraAEGReleaseTime,		// 29

        &paraFilterType,		// 30
		&paraDist,				// 31
        &paraCutoff,			// 32
        &paraResonance,			// 33
        &paraFEGAttackTime,		// 34
        &paraFEGDecayTime,		// 35
        &paraFEGSustainTime,	// 36
        &paraFEGSustainLevel,	// 37
        &paraFEGReleaseTime,	// 38
        &paraFEnvMod,			// 39

        // LFO 1
        &paraLFO1Dest,			// 40
        &paraLFO1Wave,			// 41
        &paraLFO1Freq,			// 42
        &paraLFO1Amount,		// 43
        &paraLFO1PhaseDiff,		// 44
        // LFO 2	
        &paraLFO2Dest,			// 45
        &paraLFO2Wave,			// 46
        &paraLFO2Freq,			// 47
        &paraLFO2Amount,		// 48
        &paraLFO2PhaseDiff,		// 49

		&paraUEGDest,			// 50

		&paraPhase2,				// 51

		&paraModDest1,			// 52
		&paraModAmount1,		// 53
		&paraModDest2,			// 54
		&paraModAmount2,		// 55

		&paraAmpGain,			// 56

        &paraNote,				// 
        &paraVolume,
};

#pragma pack(1)


class gvals
{
public:
		byte Mode;

		byte ModWheel;
		byte PitchWheel;
		byte PitchBendAmt;
		byte Glide;
		byte WavetableOsc;
		byte FixedPitch;
		byte WaveDetuneSemi;
        byte Wave1;
        byte PulseWidth1;
        byte Wave2;
        byte PulseWidth2;
        byte DetuneSemi;
        byte DetuneFine;
        byte Sync;
        byte MixType;
        byte Mix;
        byte SubOscWave;
        byte SubOscVol;
        byte UEGAttackTime;
        byte UEGDecayTime;
		byte UEGSustainTime;
		byte UEGSustainLevel;
		byte UEGReleaseTime;
        byte UEnvMod;

        byte AEGAttackTime;
		byte AEGDecayTime;
		byte AEGSustainTime;
		byte AEGSustainLevel;
        byte AEGReleaseTime;

        byte FilterType;
		byte Dist;
        byte Cutoff;
        byte Resonance;
        byte FEGAttackTime;
		byte FEGDecayTime;
        byte FEGSustainTime;		   
		byte FEGSustainLevel;
        byte FEGReleaseTime;
        byte FEnvMod;

        byte LFO1Dest;
        byte LFO1Wave;
        byte LFO1Freq;

        byte LFO1Amount;
        byte LFO1PhaseDiff;

        byte LFO2Dest;
        byte LFO2Wave;
        byte LFO2Freq;

        byte LFO2Amount;
        byte LFO2PhaseDiff;

		byte UEGDest;

		byte Phase2;
		byte ModDest1;
		byte ModAmount1;
		byte ModDest2;
		byte ModAmount2;

		byte AmpGain;
};

class tvals
{
public:
        byte Note;
        byte Volume;
};

class avals
{
public:
        int LFO1ScaleOsc1;
        int LFO1ScalePW1;
        int LFO1ScaleVolume;
        int LFO1ScaleCutoff;
        int LFO2ScaleOsc2;
        int LFO2ScalePW2;
        int LFO2ScaleMix;
        int LFO2ScaleReso;
		int Inertia;
		int MIDIChannel;
		int MIDITranspose;
		int MIDIVelocity;
		int Interpolation;
};



#pragma pack()

CMachineInfo const MacInfo =
{
        MT_GENERATOR,                                                   // type
        MI_VERSION,
        MIF_PLAYS_WAVES,                                                                              // flags
        1,                                                                              // min tracks
        MAX_TRACKS,                                                    // max tracks
        57,                                                            // numGlobalParameters
        2,                                                             // numTrackParameters
        pParameters,
        13,								// num attributes
        pAttributes,
#ifdef _DEBUG
        "M4wII.dll",                     // name
#else
        "M4wII.dll",
#endif
        "M4wII",                                                                   // short name
        "Makk, w/ mods by WhiteNoise",                                                                 // author
        "Edit"
};

class mi;

class CTrack
{
public:
        void Tick(tvals const &tv);
        void Stop();
        void Init();
        void Work(float *psamples, int numsamples);
        inline float Osc();
        inline float VCA();
		inline void UEG();
        inline float Filter( float x);
        void NewPhases();
        int MSToSamples(double const ms);
		void UpdateLFO1Amounts(int amt);
		void UpdateLFO2Amounts(int amt);
		void NoteOn();
		void NoteOff();
		inline float iir_filter(float input, float Cut, float r);

public:
		FILTER	iir;
		int Count;

        // ......Osc......
		int Note;
        long Phase1, Phase2, PhaseSub;
		int PhaseMod;
		CResamplerParams Waveparms;
		CResamplerState Wavestate;
		bWaveInfo bw1, bw2, bw3;
	    CWaveLevel const *pLevel;

        long Ph1, Ph2;
		int LevelShift1, LevelShift2, LevelShift3;		// for bandlimited waves
        float center1, center2;
                int c1, c2;

        float PhScale1A, PhScale1B;
        float PhScale2A, PhScale2B;
        long PhaseAdd1, PhaseAdd2;
        float Frequency, FrequencyFrom;
        // Glide
        bool GlideActive;
        float GlideMul, GlideFactor;
        int GlideCount;
        // PitchEnvMod
        bool PitchModActive;
        // PEG ... AD-Hüllkurve
        int UEGState;
        int UEGCount;

        int UEGAmp;
        int UEGAdd;
		int UEGTarget;


		float UEGPW1Amt;
		float UEGPW2Amt;
		int UEGMixAmt;
		int UEGReso;
		int UEGPhase;

        float PitchFactor1, PitchFactor2;

        // random generator... rauschen
        short r1, r2, r3, r4;
		long r5;

        float OldOut; // gegen extreme Knackser/Wertesprünge
		float OldOldOut;

        // .........AEG........ ASR-Hüllkurve
        int AEGState;
        int AEGCount;
        int Volume;
        int Amp;
        int AmpAdd;
		int AmpTarget;


        // ........Filter..........
        float x1, x2, y1, y2;
        float x241, x242, y241, y242;
        int FEGState;
        int FEGCount;
        float Cut;
        float CutAdd;
		float CutTarget;

        // .........LFOs...........
        int PhLFO1, PhLFO2;

        int LFO1AmountOsc1;
        float LFO1AmountPW1;
        int LFO1AmountVolume;
        int LFO1AmountCutoff;
        int LFO2AmountOsc2;
        float LFO2AmountPW2;
        int LFO2AmountMix;
        int LFO2AmountReso;

        mi *pmi; // ptr to MachineInterface
};


class mi : public CMachineInterface
{
public:
        mi();
        virtual ~mi();

        virtual void Init(CMachineDataInput * const pi);
        virtual void Tick();
        virtual bool Work(float *psamples, int numsamples, int const mode);
        virtual void SetNumTracks(int const n) { numTracks = n; }
        virtual void Stop();
		virtual void Command(int const i);
        virtual char const *DescribeValue(int const param, int const value);
		virtual void MidiNote(int const channel, int const value, int const velocity);
        void ComputeCoefs( float *coefs, int f, int r, int t);
        // skalefuncs
        inline float scalLFOFreq( int v);
        inline float scalEnvTime( int v);
        inline float scalCutoff( int v);
        inline float scalResonance( float v);
        inline float scalBandwidth( int v);
        inline int MSToSamples(double const ms);
		void LaunchEditor();
		const char *ControlWrapper2(int, int);

public:

        // OSC
        char noise1, noise2;
        int SubOscVol;
        float Center1, Center2;
        const short *pwavetab1, *pwavetab2, *pwavetabsub;
		int WaveTableWave;
		int WaveDetuneSemi;
		bool WaveFixedPitch;
		char oscwave1, oscwave2, oscwave3;
		int AmpGain;

	    CWaveInfo const *pWave;			// Wavetable Wave

        // Filter
        float *coefsTabOffs; // abhängig vom FilterTyp
        float Cutoff, CutoffTarget, CutoffAdd, OldCutoff;
		int Resonance;
        bool db24, db18, peak;
		int phat_philter;

		// dist
		byte Dist;
        // UEG
        int UEGAttackTime;
        int UEGDecayTime;
		unsigned int UEGSustainTime;
		int UEGSustainLevel;
		float UEGSustainFrac;
		int UEGReleaseTime;
        int UEnvMod;
		int UEGDest;
        //bool UserMod;
        // AEG
        int AEGAttackTime;
		int AEGDecayTime;
        unsigned int AEGSustainTime;
		int AEGSustainLevel;
		float AEGSustainFrac;
        int AEGReleaseTime;
        // FEG
        int FEGAttackTime;
		int FEGDecayTime;
        unsigned int FEGSustainTime;
		float FEGSustainLevel;
		float FEGSustainFrac;
        int FEGReleaseTime;
        int FEnvMod;
        // Glide
        bool Glide;
        int GlideTime;
        // Detune
        float DetuneSemi, DetuneFine;
        bool Sync;
        // LFOs

        bool LFO1Noise, LFO2Noise; // andere Frequenz
        bool LFO1Synced,LFO2Synced; // zum Songtempo
        const short *pwavetabLFO1, *pwavetabLFO2;
        int PhaseLFO1, PhaseLFO2;
        int PhaseAddLFO1, PhaseAddLFO2;
        int LFO1Freq, LFO2Freq;
        int LFO1PhaseDiff, LFO2PhaseDiff;

        // Amounts
        int LFO1Amount, LFO2Amount;

        float TabSizeDivSampleFreq;
        int numTracks;
        CTrack Tracks[MAX_TRACKS];

		int Playmode;		// playmode
		int PitchBendAmt;
		float PitchMod;

		float BendFactor;
		float BendGlide;
		int BendTime;
		bool PitchBendActive;

        // LFO
        // 1
		bool LFO_VCF;
		bool LFO_Vib;
        bool LFO_Osc1;
        bool LFO_PW1;
        bool LFO_Amp;
        bool LFO_Cut;
		bool LFO_1Lock2;
        // 2
        bool LFO_Osc2;
        bool LFO_PW2;
        bool LFO_Mix;
	    bool LFO_Reso;
		bool LFO_2Lock1;
		bool LFO_2Lock2;
        bool LFO_LFO1;
		bool LFO_Phase2;

		int ModDest1;
		int ModAmount1;
		int ModDest2;
		int ModAmount2;

		int ModWheel;

		int PhaseDiff2;

        // OscMix
        int Bal1, Bal2;
        int MixType;
		CMachine *thismachine;

		gvals ctlval;	// Current values of all the parameters for purposes of keeping the interface up to date

        avals aval; // attributes
        gvals gval; // globals
        tvals tval[MAX_TRACKS]; // track-vals


};


DLL_EXPORTS


void mi::LaunchEditor()
{
#ifdef WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	EditorDlg dlg;
	dlg.p_mi=this;
	dlg.DoModal();
#endif
}

#ifdef WIN32
class CApp : public CWinApp
{
public:
	virtual BOOL InitInstance()
	{
		AfxEnableControlContainer();
		return TRUE;
	}
};

CApp App;
#endif

const char *mi::ControlWrapper2(int parm, int value)
{
	MACHINE_LOCK;
	pCB->ControlChange(thismachine, 1, 0, parm, value);
	
	return DescribeValue(parm, value);
}

#ifdef WIN32
void EditorDlg::SendBuzzParm(int parm, int value)
{				
	static char temp[128];
	const char *desc=NULL;

	desc = p_mi->ControlWrapper2(parm, value);

	if(desc)
		sprintf(temp, "%d: %s:    %s", parm, pParameters[parm]->Description, desc);
	else
		sprintf(temp, "%d: %s:    %d", parm, pParameters[parm]->Description, value);

	m_InfoBar.SetWindowText(temp);
}

#define LFOPICSIZEX	50
#define LFOPICSIZEY	40

void DrawWave(CDC *pDC, const short *wave, int phase)
{
	//pDC->FillSolidRect(rcBounds, RGB(0, 0, 0));
	CBitmap back;
	CPoint p(0,0);
	CSize picsize(LFOPICSIZEX, LFOPICSIZEY);
		
		if(back.LoadBitmap(IDB_BITMAP2))
		{
		// TODO: Replace the following code with your own drawing code.
			//pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
			//pdc->Ellipse(rcBounds);
			pDC->DrawState(p, picsize, back, DSS_NORMAL|DST_BITMAP);

			CPen pen(PS_DOT, 1, RGB(200, 200, 200));
			CPen *pOldPen = pDC->SelectObject(&pen);

			picsize = pDC->GetViewportExt();

			int index=(2048*phase)/127, index_add = 2048/LFOPICSIZEX;
				index &= 2047;

			pDC->MoveTo(0, (LFOPICSIZEY/2) + (wave[index]*(LFOPICSIZEY/2)/32768) );

			for(int x=0; x<=LFOPICSIZEX; x++)
			{

				pDC->LineTo(x, (LFOPICSIZEY/2) + (wave[index]*(LFOPICSIZEY/2)/32768) );
				index += index_add;
				index &= 2047;
			}
			

			pDC->SelectObject(pOldPen);
		}
}

void EditorDlg::UpdateLFOPic(CButton *pic, int wave, int phase)		// phase = 0...127
{
	CSize picsize;
	const short *waveptr;

	if(wave <= 4)
	{
		p_mi->pCB->Lock();
		waveptr = p_mi->pCB->GetOscillatorTable(wave);
		p_mi->pCB->Unlock();
	}
	else if(wave == 5)
		waveptr = waves + (WAVE_STEPUP << 11);
	else if(wave == 6)
		waveptr = waves + (WAVE_STEPDN << 11);
	else if(wave == 7)
		waveptr = waves + (WAVE_WACKY1 << 11);
	else if(wave == 8)
		waveptr = waves + (WAVE_WACKY2 << 11);

	CDC *pDC = pic->GetDC();

	if(waveptr)
	if(pDC)
	{
		DrawWave(pDC, waveptr, phase);
		pic->ReleaseDC(pDC);
	}
}

void EditorDlg::UpdateOscPic(CButton *pic, int wave, int phase)		// phase = 0...127
{
	CSize picsize;	
	const short *wavedata;


    if( wave == NUMWAVES+5 || wave == NUMWAVES+6)
	{
		p_mi->pCB->Lock();
		wavedata = p_mi->pCB->GetOscillatorTable(OWF_NOISE);
		p_mi->pCB->Unlock();
	}
    else {
		if(wave <= 5)
		{
			p_mi->pCB->Lock();
			wavedata = p_mi->pCB->GetOscillatorTable(wave);
			p_mi->pCB->Unlock();
		}
		else
		{
			wavedata = waves + ((wave-6) << 11);
		}
    }


	CDC *pDC = pic->GetDC();

	if(pDC)
	{
		DrawWave(pDC, wavedata, phase);
		pic->ReleaseDC(pDC);
	}
}

void EditorDlg::OnLfo1pic() 
{
	// TODO: Add your control notification handler code here

	LFO1Wave++;

	LFO1Wave %= paraLFO1Wave.MaxValue+1;		// Make sure this is correct if you add more LFO waves

	SendBuzzParm(41, LFO1Wave);

	UpdateLFOPic(&m_LFO1Pic, LFO1Wave, m_LFO1Phase.GetPos());	
}

void EditorDlg::OnLfo2pic() 
{
	// TODO: Add your control notification handler code here
	LFO2Wave++;

	LFO2Wave %= paraLFO2Wave.MaxValue+1;

	SendBuzzParm(46, LFO2Wave);

	UpdateLFOPic(&m_LFO2Pic, LFO2Wave, m_LFO2Phase.GetPos());	
}

void EditorDlg::OnPlaybutton() 
{
	// TODO: Add your control notification handler code here
	int v2 = m_Note.GetPos();

     if (v2 / 12 > 9)
      return;
     byte n = ((v2 / 12) << 4) | ((v2 % 12) + 1);
     for (int c = 0; c < p_mi->numTracks; c++)
     {
             if ( p_mi->Tracks[c].Note == NOTE_NO )		
             {
                      p_mi->Tracks[c].Note = n;
	                  p_mi->Tracks[c].NoteOn();
                     return;
             }
     }

}

void EditorDlg::OnStopbutton() 
{
	// TODO: Add your control notification handler code here
     for (int c = 0; c <  p_mi->numTracks; c++)		// turn off all notes
     {
		 p_mi->Tracks[c].NoteOff();
        return;     
     }	
}

void EditorDlg::InitializeValues()
{
	m_PitchBendAmt.SetRange(paraPitchBendAmt.MinValue, paraPitchBendAmt.MaxValue);
	m_PitchBendAmt.SetPos(p_mi->ctlval.PitchBendAmt);
	m_PitchBendAmt.SetTicFreq(1);
	m_Glide.SetRange(paraGlide.MinValue, paraGlide.MaxValue);
	m_Glide.SetPos(p_mi->ctlval.Glide);
	m_Glide.SetTicFreq(16);
	m_WavetableOsc.SetRange(paraWavetableOsc.MinValue, paraWavetableOsc.MaxValue);
	m_WavetableOsc.SetPos(p_mi->ctlval.WavetableOsc);
	m_WaveDetuneSemi.SetRange(paraWaveDetuneSemi.MinValue, paraWaveDetuneSemi.MaxValue);
	m_WaveDetuneSemi.SetPos(p_mi->ctlval.WaveDetuneSemi);
	m_WaveDetuneSemi.SetTicFreq(16);
	m_Wave1.SetRange(paraWave1.MinValue, paraWave1.MaxValue);
	m_Wave1.SetPos(p_mi->ctlval.Wave1);
	m_PulseWidth1.SetRange(paraPulseWidth1.MinValue, paraPulseWidth1.MaxValue);
	m_PulseWidth1.SetPos(p_mi->ctlval.PulseWidth1);
	m_PulseWidth1.SetTicFreq(16);
	m_Sync.SetCheck(p_mi->ctlval.Sync);
	m_FixedPitch.SetCheck(p_mi->ctlval.FixedPitch);
	m_Wave2.SetRange(paraWave2.MinValue, paraWave2.MaxValue);
	m_Wave2.SetPos(p_mi->ctlval.Wave2);
	m_DetuneSemi.SetRange(paraDetuneSemi.MinValue, paraDetuneSemi.MaxValue);
	m_DetuneSemi.SetPos(p_mi->ctlval.DetuneSemi);
	m_DetuneSemi.SetTicFreq(16);
	m_DetuneFine.SetRange(paraDetuneFine.MinValue, paraDetuneFine.MaxValue);
	m_DetuneFine.SetPos(p_mi->ctlval.DetuneFine);
	m_DetuneFine.SetTicFreq(16);
	m_PulseWidth2.SetRange(paraPulseWidth2.MinValue, paraPulseWidth2.MaxValue);
	m_PulseWidth2.SetPos(p_mi->ctlval.PulseWidth2);
	m_PulseWidth2.SetTicFreq(16);
	m_SubOscWave.SetRange(paraSubOscWave.MinValue, paraSubOscWave.MaxValue);
	m_SubOscWave.SetPos(p_mi->ctlval.SubOscWave);
	m_SubOscVol.SetRange(paraSubOscVol.MinValue, paraSubOscVol.MaxValue);
	m_SubOscVol.SetPos(p_mi->ctlval.SubOscVol);
	m_SubOscVol.SetTicFreq(16);
	m_MixType.SetRange(paraMixType.MinValue, paraMixType.MaxValue);
	m_MixType.SetPos(p_mi->ctlval.MixType);
	m_Mix.SetRange(paraMix.MinValue, paraMix.MaxValue);
	m_Mix.SetPos(p_mi->ctlval.Mix);
	m_Mix.SetTicFreq(16);

	m_UEGAttackTime.SetScrollRange(paraUEGAttackTime.MinValue, paraUEGAttackTime.MaxValue);
	m_UEGAttackTime.SetScrollPos(127-p_mi->ctlval.UEGAttackTime);
	m_AEGAttackTime.SetScrollRange(paraAEGAttackTime.MinValue, paraAEGAttackTime.MaxValue);
	m_AEGAttackTime.SetScrollPos(127-p_mi->ctlval.AEGAttackTime);
	m_FEGAttackTime.SetScrollRange(paraFEGAttackTime.MinValue, paraFEGAttackTime.MaxValue);
	m_FEGAttackTime.SetScrollPos(127-p_mi->ctlval.FEGAttackTime);

	m_UEGDecayTime.SetScrollRange(paraUEGDecayTime.MinValue, paraUEGDecayTime.MaxValue);
	m_UEGDecayTime.SetScrollPos(127-p_mi->ctlval.UEGDecayTime);
	m_AEGDecayTime.SetScrollRange(paraAEGDecayTime.MinValue, paraAEGDecayTime.MaxValue);
	m_AEGDecayTime.SetScrollPos(127-p_mi->ctlval.AEGDecayTime);
	m_FEGDecayTime.SetScrollRange(paraFEGDecayTime.MinValue, paraFEGDecayTime.MaxValue);
	m_FEGDecayTime.SetScrollPos(127-p_mi->ctlval.FEGDecayTime);

	m_UEGReleaseTime.SetScrollRange(paraUEGReleaseTime.MinValue, paraUEGReleaseTime.MaxValue);
	m_UEGReleaseTime.SetScrollPos(127-p_mi->ctlval.UEGReleaseTime);
	m_AEGReleaseTime.SetScrollRange(paraAEGReleaseTime.MinValue, paraAEGReleaseTime.MaxValue);
	m_AEGReleaseTime.SetScrollPos(127-p_mi->ctlval.AEGReleaseTime);
	m_FEGReleaseTime.SetScrollRange(paraFEGReleaseTime.MinValue, paraFEGReleaseTime.MaxValue);
	m_FEGReleaseTime.SetScrollPos(127-p_mi->ctlval.FEGReleaseTime);

	m_UEGSustainTime.SetScrollRange(paraUEGSustainTime.MinValue, paraUEGSustainTime.MaxValue);
	m_UEGSustainTime.SetScrollPos(127-p_mi->ctlval.UEGSustainTime);
	m_AEGSustainTime.SetScrollRange(paraAEGSustainTime.MinValue, paraAEGSustainTime.MaxValue);
	m_AEGSustainTime.SetScrollPos(127-p_mi->ctlval.AEGSustainTime);
	m_FEGSustainTime.SetScrollRange(paraFEGSustainTime.MinValue, paraFEGSustainTime.MaxValue);
	m_FEGSustainTime.SetScrollPos(127-p_mi->ctlval.FEGSustainTime);

	m_UEGSustainLevel.SetRange(paraUEGSustainLevel.MinValue, paraUEGSustainLevel.MaxValue);
	m_UEGSustainLevel.SetPos(127-p_mi->ctlval.UEGSustainLevel);
	m_UEGSustainLevel.SetTicFreq(16);
	m_AEGSustainLevel.SetRange(paraAEGSustainLevel.MinValue, paraAEGSustainLevel.MaxValue);
	m_AEGSustainLevel.SetPos(127-p_mi->ctlval.AEGSustainLevel);
	m_AEGSustainLevel.SetTicFreq(16);
	m_FEGSustainLevel.SetRange(paraFEGSustainLevel.MinValue, paraFEGSustainLevel.MaxValue);
	m_FEGSustainLevel.SetPos(127-p_mi->ctlval.FEGSustainLevel);
	m_FEGSustainLevel.SetTicFreq(16);

	m_FilterType.SetRange(paraFilterType.MinValue, paraFilterType.MaxValue);
	m_FilterType.SetPos(p_mi->ctlval.FilterType);
	m_Cutoff.SetRange(paraCutoff.MinValue, paraCutoff.MaxValue);
	m_Cutoff.SetPos(p_mi->ctlval.Cutoff);
	m_Cutoff.SetTicFreq(16);
	m_Resonance.SetRange(paraResonance.MinValue, paraResonance.MaxValue);
	m_Resonance.SetPos(p_mi->ctlval.Resonance);
	m_Resonance.SetTicFreq(16);
	m_Dist.SetRange(paraDist.MinValue, paraDist.MaxValue);
	m_Dist.SetPos(p_mi->ctlval.Dist);

	m_ModDest1.SetRange(paraModDest1.MinValue, paraModDest1.MaxValue);
	m_ModDest1.SetPos(p_mi->ctlval.ModDest1);
	m_ModDest2.SetRange(paraModDest2.MinValue, paraModDest2.MaxValue);
	m_ModDest2.SetPos(p_mi->ctlval.ModDest2);
	m_ModAmount1.SetRange(paraModAmount1.MinValue, paraModAmount1.MaxValue);
	m_ModAmount1.SetPos(p_mi->ctlval.ModAmount1);
	m_ModAmount1.SetTicFreq(16);
	m_ModAmount2.SetRange(paraModAmount2.MinValue, paraModAmount2.MaxValue);
	m_ModAmount2.SetPos(p_mi->ctlval.ModAmount2);
	m_ModAmount2.SetTicFreq(16);

	UDACCEL spinaccel;

	spinaccel.nSec = 1;
	spinaccel.nInc = 4;

	m_LFO1Dest.SetRange(paraLFO1Dest.MinValue, paraLFO1Dest.MaxValue);
	m_LFO1Dest.SetPos(p_mi->ctlval.LFO1Dest);
	m_LFO1Freq.SetRange(paraLFO1Freq.MinValue, paraLFO1Freq.MaxValue);
	m_LFO1Freq.SetPos(p_mi->ctlval.LFO1Freq);
	m_LFO1Freq.SetTicFreq(16);
	m_LFO1Amount.SetRange(paraLFO1Amount.MinValue, paraLFO1Amount.MaxValue);
	m_LFO1Amount.SetPos(p_mi->ctlval.LFO1Amount);
	m_LFO1Amount.SetTicFreq(16);
	m_LFO1Phase.SetRange(paraLFO1PhaseDiff.MinValue, paraLFO1PhaseDiff.MaxValue);
	m_LFO1Phase.SetPos(p_mi->ctlval.LFO1PhaseDiff);
	m_LFO1Phase.SetAccel(1, &spinaccel );

	m_PhaseDiff2.SetRange(paraPhase2.MinValue, paraPhase2.MaxValue);
	m_PhaseDiff2.SetPos(p_mi->ctlval.Phase2);
	m_PhaseDiff2.SetAccel(1, &spinaccel );

	m_LFO2Dest.SetRange(paraLFO2Dest.MinValue, paraLFO2Dest.MaxValue);
	m_LFO2Dest.SetPos(p_mi->ctlval.LFO2Dest);
	m_LFO2Freq.SetRange(paraLFO2Freq.MinValue, paraLFO2Freq.MaxValue);
	m_LFO2Freq.SetPos(p_mi->ctlval.LFO2Freq);
	m_LFO2Freq.SetTicFreq(16);
	m_LFO2Amount.SetRange(paraLFO2Amount.MinValue, paraLFO2Amount.MaxValue);
	m_LFO2Amount.SetPos(p_mi->ctlval.LFO2Amount);
	m_LFO2Amount.SetTicFreq(16);
	m_LFO2Phase.SetRange(paraLFO2PhaseDiff.MinValue, paraLFO2PhaseDiff.MaxValue);
	m_LFO2Phase.SetPos(p_mi->ctlval.LFO2PhaseDiff);
	m_LFO2Phase.SetAccel(1, &spinaccel );

	m_AmpGain.SetRange(paraAmpGain.MinValue, paraAmpGain.MaxValue);
	m_AmpGain.SetPos(p_mi->ctlval.AmpGain);
	m_AmpGain.SetTicFreq(16);
	m_FEnvMod.SetRange(paraFEnvMod.MinValue, paraFEnvMod.MaxValue);
	m_FEnvMod.SetPos(p_mi->ctlval.FEnvMod);
	m_FEnvMod.SetTicFreq(16);
	m_UEnvMod.SetRange(paraUEnvMod.MinValue, paraUEnvMod.MaxValue);
	m_UEnvMod.SetPos(p_mi->ctlval.UEnvMod);
	m_UEnvMod.SetTicFreq(16);
	m_UEGDest.SetRange(paraUEGDest.MinValue, paraUEGDest.MaxValue);
	m_UEGDest.SetPos(p_mi->ctlval.UEGDest);

	m_Note.SetRange(0, 127);
	m_Note.SetPos(64);
	m_Note.SetTicFreq(16);

	LFO1Wave = p_mi->ctlval.LFO1Wave;
	LFO2Wave = p_mi->ctlval.LFO2Wave;
}
#endif

void mi::Command(int const i)
{
        switch(i)
        {
        case 0:
			LaunchEditor();
			break;
        }
}

// Skalierungsmethoden
inline float mi::scalCutoff( int v)
{
        return (float)(pow( (v+5)/(127.0+5), 1.7)*13000+30);
}
inline float mi::scalResonance( float v)
{
        return (float)(pow( v/127.0, 4)*150+0.1);
}
inline float mi::scalBandwidth( int v)
{
        return (float)(pow( v/127.0, 4)*4+0.1);
}

inline float mi::scalLFOFreq( int v)
{
        return (float)((pow( (v+8)/(116.0+8), 4)-0.000017324998565270)*40.00072);
}

inline float mi::scalEnvTime( int v)
{
        return (float)(pow( (v+2)/(127.0+2), 3)*10000);
}

//////////////////////////////////////////////////////
// CTRACK METHODEN
//////////////////////////////////////////////////////

inline int mi::MSToSamples(double const ms)
{
        return (int)(pMasterInfo->SamplesPerSec * ms * (1.0 / 1000.0)) + 1; // +1 wg. div durch 0
}

void CTrack::Stop()
{
	NoteOff();
}

void CTrack::Init()
{
		LevelShift1 = 0;
		LevelShift2 = 0;
		LevelShift3 = 0;
        AEGState = EGS_NONE;
                FEGState = EGS_NONE;
                UEGState = EGS_NONE;
        r1=26474; r2=13075; r3=18376; r4=31291; // randomGenerator
		r5 = 0;
        Phase1 = Phase2 = Ph1 = Ph2 = PhaseSub = 0; // Osc starten neu
        x1 = x2 = y1 = y2 = 0; //Filter
        x241 = x242 = y241 = y242 = 0; //Filter
        OldOut = 0;
		OldOldOut = 0;
                Amp = 0;
                AEGCount = -1;
                FEGCount = -1;
                UEGCount = -1;
                center1 = pmi->Center1;
                center2 = pmi->Center2;
                PhScale1A = 0.5/center1;
                PhScale1B = 0.5/(1-center1);
                PhScale2A = 0.5/center2;
                PhScale2B = 0.5/(1-center2);	
                c1 = f2i(center1*0x8000000);			// Same
                c2 = f2i(center2*0x8000000);			// FIXME: use f2i
                GlideActive = false;
                PitchModActive = false;
                Volume = paraVolume.DefValue << 20;
				UEGPW1Amt = 0;
				UEGPW2Amt = 0;
				UEGMixAmt = 0;
				UEGReso = 0;
				UEGPhase = 0;
				PitchFactor1 = 1.0;
				PitchFactor2 = 1.0;


		Note = NOTE_NO;

		Wavestate.PosInt = 0;
		Wavestate.PosFrac = 0;
		Wavestate.Amp = 1.0;
		Wavestate.Active = false;

		Waveparms.LoopBegin = -1;			// This stuff can be inited in Init
		Waveparms.Interpolation = RSI_LINEAR;
		Waveparms.AmpMode = RSA_CONSTANT;
		Waveparms.StepMode = RSS_CONSTANT;
		Waveparms.Flags = 0;

		/* Section 1 */		
		ProtoCoef[0].a0 = 1.0;
		ProtoCoef[0].a1 = 0;
		ProtoCoef[0].a2 = 0;
		ProtoCoef[0].b0 = 1.0;
		ProtoCoef[0].b1 = 0.765367;
		ProtoCoef[0].b2 = 1.0;
		/* Section 2 */		
		ProtoCoef[1].a0 = 1.0;
		ProtoCoef[1].a1 = 0;
		ProtoCoef[1].a2 = 0;
		ProtoCoef[1].b0 = 1.0;
		ProtoCoef[1].b1 = 1.847759;
		ProtoCoef[1].b2 = 1.0;

		iir.length = FILTER_SECTIONS;		/* Number of filter sections */
		Count = 1;

		//allocate memory
		iir.coef = (double *) calloc(4 * iir.length + 1, sizeof(double));
		iir.history = (float *) calloc(2*iir.length,sizeof(float));
		iir.last_cutoff = -1.0;
		iir.last_res = -1.0;


}

inline void CTrack::UpdateLFO1Amounts(int amt)
{							
        LFO1AmountOsc1 = (amt*pmi->aval.LFO1ScaleOsc1)>>8;
        LFO1AmountPW1 = (amt*pmi->aval.LFO1ScalePW1/(256.0*127.0*0x8000));		// FIXME: Precalc this
        LFO1AmountVolume = (amt*pmi->aval.LFO1ScaleVolume)>>8;
        LFO1AmountCutoff = (amt*pmi->aval.LFO1ScaleCutoff)>>8;
}

inline void CTrack::UpdateLFO2Amounts(int amt)
{							
        LFO2AmountOsc2 = (amt*pmi->aval.LFO2ScaleOsc2)>>8;
        LFO2AmountPW2 = (amt*pmi->aval.LFO2ScalePW2/(256.0*127.0*0x8000));
        LFO2AmountMix = (amt*pmi->aval.LFO2ScaleMix)>>8;
        LFO2AmountReso = (amt*pmi->aval.LFO2ScaleReso)>>8;
}

void CTrack::NoteOn()
{
		FrequencyFrom = Frequency;
        Frequency = (float)(16.3516*pow(2,((Note>>4)*12+(Note&0x0f)-1)/12.0));

        if( pmi->Glide && AEGState != EGS_NONE) {	// if a note is playing at all, glide.. if not, just play
                GlideActive = true;							
                if( Frequency > FrequencyFrom)
                        GlideMul = (float)pow( 2, 1.0/pmi->GlideTime);
                else
                        GlideMul = (float)pow( 0.5, 1.0/pmi->GlideTime);
                GlideFactor = 1;
                GlideCount = (int)(log( Frequency/FrequencyFrom)/log(GlideMul));
        }
        else
                GlideActive = false;


		// FIXME:
		// If option set, only trigger these if the note is finished or in release?
		
        // trigger envelopes neu an...
        // Amp
		if(!(pmi->Playmode == 8) || (AEGState == EGS_NONE))	// if mono, don't initialize
		{

// Start Amp
			pmi->AEGSustainLevel = Volume*pmi->AEGSustainFrac;
			AEGState = EGS_ATTACK;
			AEGCount = pmi->AEGAttackTime;
			AmpAdd = Volume/pmi->AEGAttackTime;
			if(pmi->Playmode & 1)
				Amp = 0; //AmpAdd; // fange bei 0 an
			AmpTarget = Volume;

// Start LFO's
			if(pmi->Playmode & 2)
			{
				pmi->PhaseLFO1 = 0;
				pmi->PhaseLFO2 = 0;
			}


// Start Filter
			FEGState = EGS_ATTACK;
			FEGCount = pmi->FEGAttackTime;
			CutAdd = ((float)pmi->FEnvMod)/(float)pmi->FEGAttackTime;

			if(pmi->Playmode & 4)
				Cut = 0.0; // fange bei 0 an

			CutTarget = pmi->FEnvMod;

// Start UEG
			if(pmi->UEnvMod)
			{
				UEGState = EGS_ATTACK;
				UEGCount = pmi->UEGAttackTime;
				UEGAdd = (pmi->UEnvMod)/pmi->UEGAttackTime;
				UEGAmp = 0;
				UEGTarget = pmi->UEnvMod;
				PitchFactor1 = 1.0;
				PitchFactor2 = 1.0;
				UEGMixAmt = 0;
				UEGPW1Amt = 0;
				UEGPW2Amt = 0;
				UEGReso = 0;
				UEGPhase = 0;

			}
			else
			{
				UEGState = EGS_NONE;
				UEGAmp = 0;
				UEGTarget = 0;
			}
		}

        // Pitch		
		PitchModActive = false;
		// If pitch bend wheel active or user envelope->pitch, then true

		if(pmi->pWave)
		{
			pLevel = pmi->pCB->GetNearestWaveLevel(pmi->WaveTableWave, Note);

			if(pLevel)
			{
				int N;		
		
				if(pmi->WaveFixedPitch)
					N = pmi->WaveDetuneSemi;
				else
					N = (((Note>>4) - (pLevel->RootNote>>4))*12) + ((Note&0x0f)-(pLevel->RootNote&0x0f)) + pmi->WaveDetuneSemi;
				// FIXME: calc with Frequency?

				if(pmi->pWave->Flags & WF_LOOP)
				{
					Waveparms.LoopBegin = pLevel->LoopStart;
					Waveparms.numSamples = pLevel->LoopEnd;
				}
				else
					Waveparms.LoopBegin = -1;

				Waveparms.SetStep(pow(2.0,(N / 12.0)));

				Wavestate.PosInt = 0;
				Wavestate.PosFrac = 0;
				Wavestate.Amp = pmi->pWave->Volume;	
				
				Wavestate.Active = true;
			}
			else
				Wavestate.Active = false;
		}

		if( GlideActive) {
				PhaseAdd1 = (int)(FrequencyFrom*pmi->TabSizeDivSampleFreq*0x10000);
				PhaseAdd2 = (int)(FrequencyFrom*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
		}
		else {						
				PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
				PhaseAdd2 = (int)(Frequency*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
		}	


		// FIXME: This would be better done in the NewPhases, if there is pitch modification
		if(pmi->oscwave1 != -1)
		{
			LevelShift1 = 0;

			while(Frequency*((float)(2048>>LevelShift1)/(float)pmi->pMasterInfo->SamplesPerSec) > 1.0)
				LevelShift1++;

			if(LevelShift1 > 10)
				LevelShift1 = 10;
		}
		else 
			LevelShift1 = 0;

		if(pmi->oscwave2 != -1)		
		{
			LevelShift2 = 0;

			while(Frequency*pmi->DetuneSemi*pmi->DetuneFine*((float)(2048>>LevelShift2)/(float)pmi->pMasterInfo->SamplesPerSec) > 1.0)
				LevelShift2++;

			if(LevelShift2 > 10)
				LevelShift2 = 10;

		}
		else 
			LevelShift2 = 0;

		if(pmi->oscwave3 != -1)
		{
			LevelShift3 = 0;

			while(Frequency*0.5*((float)(2048>>LevelShift3)/pmi->pMasterInfo->SamplesPerSec) > 1.0)
			{
				LevelShift3++;

				if(LevelShift3 > 10)
					LevelShift3 = 10;

				if(LevelShift3 < 0)
					LevelShift3 = 0;
			}
		}
		else 
			LevelShift3 = 0;

		bw1.offset = GetOscTblOffset(LevelShift1);
		bw1.sh = 16 + LevelShift1;
		bw1.size = (2048>>LevelShift1)-1;
		bw1.mask = ((1<<bw1.sh) - 1);
		bw1.maskshift = (bw1.sh-12);

		bw2.offset = GetOscTblOffset(LevelShift2);
		bw2.sh = 16 + LevelShift2;
		bw2.size = (2048>>LevelShift2)-1;
		bw2.mask = ((1<<bw2.sh) - 1);
		bw2.maskshift = (bw2.sh-12);

		bw3.offset = GetOscTblOffset(LevelShift3);
		bw3.sh = 16 + LevelShift3;
		bw3.size = (2048>>LevelShift3)-1;
		bw3.mask = ((1<<bw3.sh) - 1);
		bw3.maskshift = (bw3.sh-12);

}

void CTrack::NoteOff()
{

	if(!AEGState)		// if note is playing, stop
		return;

//AEGState = EGS_NONE; // note aus
		AEGState = EGS_RELEASE;
		AEGCount = pmi->AEGReleaseTime;
        AmpAdd = -(pmi->AEGSustainFrac*Volume)/pmi->AEGReleaseTime;
		AmpTarget = 0;

		FEGState = EGS_RELEASE;
		CutAdd = -pmi->FEGSustainLevel/pmi->FEGReleaseTime;
		FEGCount = pmi->FEGReleaseTime;
		CutTarget = 0;		

		UEGState = EGS_RELEASE;
		UEGAdd = -pmi->UEGSustainLevel/pmi->UEGReleaseTime;
		UEGCount = pmi->UEGReleaseTime<<1;
		UEGTarget = 0;
/*
		if(pmi->WaveTableWave)
			pLevel = pmi->pCB->GetNearestWaveLevel(pmi->WaveTableWave, Note);
		else
			pLevel = NULL;

		if(pLevel && Wavestate.Active && pmi->pWave)
		{

				//Waveparms.LoopBegin = -1;		// stop any loops
				//Waveparms.numSamples = pLevel->numSamples;	// and allow them to finish to the end
		}
*/
		// Need some other way of saying when a note can be interrupted
		//Note = NOTE_NO;


		// FIXME: ??

}

void CTrack::Tick( tvals const &tv)
{
        if( tv.Volume != paraVolume.NoValue)
		{
                Volume = tv.Volume << 20;
		}

        if( tv.Note != paraNote.NoValue) { // neuer wert
                if( (tv.Note >= NOTE_MIN) && (tv.Note <= NOTE_MAX)) { // neue note gesetzt
						Note = tv.Note;
						NoteOn();

                } else
                        if( tv.Note == NOTE_OFF)
						{
                                NoteOff();
						}
	    }



}


inline float CTrack::Osc()
{
        int o, o2, interp1, interp2, interp3, interp4;
        int B1, B2;
		int index=0, index2, index3, index4;

        o = 0;
        o2 = 0;

        if( pmi->LFO_Mix || UEGMixAmt != 0) { // LFO-MIX
                B2 = pmi->Bal2;
				if(pmi->LFO_Mix) {
					B2 += ((pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*LFO2AmountMix)>>15);

					B2 += UEGMixAmt;

                        if( B2<0)
                                B2 = 0;
                        else
                                if( B2>127)
                                        B2 = 127;
                        B1 = 127-B2;

                        // osc1
                        if( pmi->noise1 == NOISE1) {
                                short t = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=t;

                                o = (t*B1)>>7;
                        }
						else if( pmi->noise1 == NOISE2) 
						{				// FIXME: F2i
								r5 += (((rand()%32768) - 16384)*center2);

								if(r5 > 32768)
									r5 = 32768;
								if(r5 < -32768)
									r5 = -32768;

								o = (r5*B1)>>7;						
						}
                        else
						{
							if(pmi->oscwave1 != -1)
							{
								if(B1 == 0)
								{
									o = 0;
								}
								else
									switch(pmi->aval.Interpolation)
									{
									case 1:
										index =	((unsigned)Ph1>>bw1.sh);
										index2 = (index+1) & bw1.size;
										interp1 = pmi->pwavetab1[index  + bw1.offset];
										interp2 = pmi->pwavetab1[index2 + bw1.offset];
										o = (LInterpolateF(interp1, interp2, (Ph1 & bw1.mask)>>bw1.maskshift)*B1)>>7;
										break;
									case 2:
										index =	((unsigned)Ph1>>bw1.sh);
										index3 = (index-1) & bw1.size;
										index2 = (index+1) & bw1.size;
										index4 = (index+2) & bw1.size;

										interp1 = pmi->pwavetab1[index  + bw1.offset];
										interp2 = pmi->pwavetab1[index2 + bw1.offset];
										interp3 = pmi->pwavetab1[index3 + bw1.offset];
										interp4 = pmi->pwavetab1[index4 + bw1.offset];
										
										o = (int)(SplineInterp(interp3,interp1,interp2, interp4, (Ph1 & bw1.mask)>>bw1.maskshift)*B1)>>7;
										break;
									default:
										o = (pmi->pwavetab1[((unsigned)Ph1>>bw1.sh) + bw1.offset]*B1)>>7;
									}							}
							else
                                o = (pmi->pwavetab1[(unsigned)Ph1>>16]*B1)>>7;
						}

                        // osc2
                        if( pmi->noise2) {
                                short u = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=u;
                                o2 = (u*B2)>>7;
                        }
						else if( pmi->noise2 == NOISE2) 
						{				// FIXME: F2i
								r5 += (((rand()%32768) - 16384)*center2);
								if(r5 > 32768)
									r5 = 32768;
								if(r5 < -32768)
									r5 = -32768;

								o2 = (r5*B2)>>7;						
						}
                        else
						{
							if(pmi->oscwave2 != -1)						
							{
								if(B2 == 0)
								{
									o2 = 0;
								}
								else
									switch(pmi->aval.Interpolation)
									{
									case 1:
										index =	((unsigned)Ph2>>bw2.sh);
										index2 = (index+1) & bw2.size;
										interp1 = pmi->pwavetab2[index  + bw2.offset];
										interp2 = pmi->pwavetab2[index2 + bw2.offset];
										o2 = (LInterpolateF(interp1, interp2, (Ph2 & bw2.mask)>>bw2.maskshift)*B2)>>7;				
										break;
									case 2:
										index =	(Ph2>>bw2.sh);
										index2 = (index+1) & bw2.size;
										index3 = (index-1) & bw2.size;
										index4 = (index+2) & bw2.size;

										interp1 = pmi->pwavetab2[index  + bw2.offset];
										interp2 = pmi->pwavetab2[index2 + bw2.offset];
										interp3 = pmi->pwavetab2[index3 + bw2.offset];
										interp4 = pmi->pwavetab2[index4 + bw2.offset];
										
										o2 = (SplineInterp(interp3,interp1,interp2, interp4, (Ph2 & bw2.mask)>>bw2.maskshift)*B2)>>7;
										break;
									default:
										o2 = (pmi->pwavetab2[((unsigned)Ph2>>bw2.sh) + bw2.offset]*B2)>>7;
									}
							}
							else
                                o2 = (pmi->pwavetab2[(unsigned)Ph2>>16]*B2)>>7;
						}
                }
                else { // kein LFO
                        // osc1
                        if( pmi->noise1 == NOISE1) {
                                short t = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=t;

                                o = (t*pmi->Bal1)>>7;
                        }
						else if( pmi->noise1 == NOISE2) 
						{				// FIXME: F2i
								r5 += (((rand()%32768) - 16384)*center1);
								if(r5 >= 32768)
									r5 = 32768;
								if(r5 <= -32768)
									r5 = -32768;

								o = ((int)r5*pmi->Bal1)>>7;						
						}
                        else
							
							if(pmi->oscwave1 != -1)									
							{
								if(pmi->Bal1 == 0)
								{
									o = 0;
								}
								else
									switch(pmi->aval.Interpolation)
									{
									case 1:
										index =	((unsigned)Ph1>>bw1.sh);
										index2 = (index+1) & bw1.size;
										interp1 = pmi->pwavetab1[index  + bw1.offset];
										interp2 = pmi->pwavetab1[index2 + bw1.offset];
										o = (LInterpolateF(interp1, interp2, (Ph1 & bw1.mask)>>bw1.maskshift)*pmi->Bal1)>>7;				
										break;
									case 2:
										index =	((unsigned)Ph1>>bw1.sh);
										index3 = (index-1) & bw1.size;
										index2 = (index+1) & bw1.size;
										index4 = (index+2) & bw1.size;

										interp1 = pmi->pwavetab1[index  + bw1.offset];
										interp2 = pmi->pwavetab1[index2 + bw1.offset];
										interp3 = pmi->pwavetab1[index3 + bw1.offset];
										interp4 = pmi->pwavetab1[index4 + bw1.offset];
										
										o = (int)(SplineInterp(interp3,interp1,interp2, interp4, (Ph1 & bw1.mask)>>bw1.maskshift)*pmi->Bal1)>>7;
										break;
									default:
										o = (pmi->pwavetab1[((unsigned)Ph1>>bw1.sh) + bw1.offset]*pmi->Bal1)>>7;
									}

							}
							else						
                                o = (pmi->pwavetab1[(unsigned)Ph1>>16]*pmi->Bal1)>>7;

                        // osc2
                        if( pmi->noise2 == NOISE1) {
                                short u = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=u;
                                o2 = (u*pmi->Bal2)>>7;
                        }
						else if( pmi->noise2 == NOISE2) 
						{				// FIXME: F2i
								r5 += (((rand()%32768) - 16384)*center2);
								if(r5 > 32768)
									r5 = 32768;
								if(r5 < -32768)
									r5 = -32768;

								o2 = ((int)r5*pmi->Bal2)>>7;						
						}
                        else
						{
							if(pmi->oscwave2 != -1)
							{
								if(pmi->Bal2 == 0)
								{
									o2 = 0;
								}
								else
									switch(pmi->aval.Interpolation)
									{
									case 1:
										index =	((unsigned)Ph2>>bw2.sh);
										index2 = (index+1) & bw2.size;
										interp1 = pmi->pwavetab2[index  + bw2.offset];
										interp2 = pmi->pwavetab2[index2 + bw2.offset];
										o2 = (LInterpolateF(interp1, interp2, (Ph2 & bw2.mask)>>bw2.maskshift)*pmi->Bal2)>>7;				
										break;
									case 2:
										index =	(Ph2>>bw2.sh);
										index2 = (index+1) & bw2.size;
										index3 = (index-1) & bw2.size;
										index4 = (index+2) & bw2.size;

										interp1 = pmi->pwavetab2[index  + bw2.offset];
										interp2 = pmi->pwavetab2[index2 + bw2.offset];
										interp3 = pmi->pwavetab2[index3 + bw2.offset];
										interp4 = pmi->pwavetab2[index4 + bw2.offset];
										
										o2 = (SplineInterp(interp3,interp1,interp2, interp4, (Ph2 & bw2.mask)>>bw2.maskshift)*pmi->Bal2)>>7;
										break;
									default:
										o2 = (pmi->pwavetab2[((unsigned)Ph2>>bw2.sh) + bw2.offset]*pmi->Bal2)>>7;
									}							
							}
							else
                                o2 = (pmi->pwavetab2[(unsigned)Ph2>>16]*pmi->Bal2)>>7;
						}

                }
        }

        // PhaseDependentMixing

        switch( pmi->MixType)
        {
        case 0: //ADD				
                o += o2;
                break;
        case 1: // ABS
                o = (abs(o-o2)<<1)-0x8000;	// wierdness
                break;
        case 2: // MUL
                o *= o2;
                o >>= 15;
                break;
        case 3: // highest amp
                if( abs(o) < abs(o2))
                        o = o2;
                break;
        case 4: // lowest amp
                if( abs(o) > abs(o2))
                        o = o2;
                break;
        case 5: // AND
                o &= o2;
                break;
        case 6: // OR
                o |= o2;
                break;
        case 7: // XOR
                o ^= o2;
                break;
        case 8: 
             if(o < o2) {
                o ^= ((o2 + o)*o) >> 15;
             }
                break;
        case 9: // AM		// FIXME: Use F2i
                o = (int) ((float)o * (float)o2/16384.0f);
                break;
        case 10: // AM2	
			o = o ^ o2;
			o = (int) ((float)o * (float)o2/16384.0f);
           break;
		case 11:
			o += o2;
			o = (o>>13);
			o = (o<<13);
			break;
        case 12: //ADD				
            o += o2;
            break;
        case 13: //ADD				
            o += o2;
            break;
        }

		if(pmi->oscwave3 != -1)
		{
			switch(pmi->aval.Interpolation)
			{
			case 1:
				index =	((unsigned)PhaseSub>>bw3.sh);
				index2 = (index+1) & bw3.size;
				interp1 = pmi->pwavetabsub[index  + bw3.offset];
				interp2 = pmi->pwavetabsub[index2 + bw3.offset];
				o += (LInterpolateF(interp1, interp2, (PhaseSub & bw3.mask)>>bw3.maskshift)*pmi->SubOscVol)>>7;				
				break;
			case 2:
				index3 = (index-1) & bw3.size;
				index =	((unsigned)PhaseSub>>bw3.sh);
				index2 = (index+1) & bw3.size;
				index4 = (index+2) & bw3.size;

				interp1 = pmi->pwavetabsub[index  + bw3.offset];
				interp2 = pmi->pwavetabsub[index2 + bw3.offset];
				interp3 = pmi->pwavetabsub[index3 + bw3.offset];
				interp4 = pmi->pwavetabsub[index4 + bw3.offset];
									
				o += ((SplineInterp(interp3,interp1,interp2, interp4, (PhaseSub & bw3.mask)>>bw3.maskshift)*pmi->SubOscVol)>>7);
				break;
			default:
				o += ((pmi->pwavetabsub[((unsigned)PhaseSub>>bw3.sh) + bw3.offset]*pmi->SubOscVol)>>7);
			}

		}
		else											
			o += ((pmi->pwavetabsub[PhaseSub>>16]*pmi->SubOscVol)>>7);

		if(pmi->AmpGain <= 32)
			o = (o * pmi->AmpGain)>>5;
		else
			o = (o * (pmi->AmpGain-16))>>4;

		if( pmi->MixType == 12)	// Overdrive baby
		{
			index = (o >> 9) + 128;
			index2 = index + 1;

			if(index < 0)		// clip
				index = 0;
			if(index > 255) 
				index = 255;
			if(index2 < 0)
				index2 = 0;
			if(index2 > 255) index2 = 255;


			o = LInterpolateI(nonlinTab[index], nonlinTab[index2], o & 0xff);

			if(o > 64000)
				o = 64000;
			if(o < -48000)
				o = -48000;

		}

		return o;
}

inline float CTrack::VCA()
{
        // EG...		
        if( !AEGCount-- 
			|| (AEGState == EGS_ATTACK && Amp >= AmpTarget)
			|| (AEGState == EGS_DECAY && Amp <= AmpTarget)
			|| (AEGState == EGS_RELEASE && Amp < AmpTarget)
			)
                switch( ++AEGState)
                {
                case EGS_DECAY:
                        AEGCount = pmi->AEGDecayTime;
                        //Amp = Volume;
						AmpTarget =	pmi->AEGSustainLevel;
                        AmpAdd = (AmpTarget - Volume)/pmi->AEGDecayTime;
						
                        break;
                case EGS_SUSTAIN:
                        AEGCount = pmi->AEGSustainTime;
                        Amp = AmpTarget;
                        AmpAdd = 0;
                        break;
                case EGS_RELEASE:
                        AEGCount = pmi->AEGReleaseTime+100;
                        AmpAdd = -(pmi->AEGSustainFrac*Volume)/pmi->AEGReleaseTime;
						AmpTarget = 0;
						/*
						if(pmi->WaveTableWave)
							pLevel = pmi->pCB->GetNearestWaveLevel(pmi->WaveTableWave, Note);
						else
							pLevel = NULL;

						if(pLevel && Wavestate.Active && pmi->pWave)
						{

								Waveparms.LoopBegin = -1;		// stop any loops
								Waveparms.numSamples = pLevel->numSamples;	// and allow waves to finish to the end

						}
						*/
                        break;
                case EGS_DONE:			// turn off
						//Waveparms.Active = false;
                        AEGState = EGS_NONE;
                        AEGCount = -1;
                        Amp = 0;
						Note = NOTE_NO;
                        break;
                }

        Amp +=AmpAdd;

        if( pmi->LFO_Amp) {
                float a =
                  Amp + ((pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*LFO1AmountVolume)<<5);
                if( a<0)
                        a = 0;
                return( a*AMPCONV);
        }
        else
                return Amp*AMPCONV;
}


inline void CTrack::UEG()
{
        // EG...		

        if( UEGState) {
        if( !UEGCount-- 
				//|| (UEGState == EGS_ATTACK && (UEGTarget > 0) && UEGAmp>= UEGTarget)
				//|| (UEGState == EGS_ATTACK && (UEGTarget <= 0) && UEGAmp <= UEGTarget)
				//|| (UEGState == EGS_DECAY && (UEGTarget > 0) && UEGAmp >= UEGTarget)
				//|| (UEGState == EGS_DECAY && (UEGTarget <= 0) && UEGAmp <= UEGTarget)
				|| ((UEGAdd > 0.0) && UEGAmp > UEGTarget)
				|| ((UEGAdd < 0.0) && UEGAmp < UEGTarget)
			)
                switch( ++UEGState)
                {
                case EGS_DECAY:
                        UEGCount = pmi->UEGDecayTime;
                        //UEGAmp = pmi->UEnvMod;
						UEGTarget =	pmi->UEGSustainLevel;
						UEGAdd = (UEGTarget - pmi->UEnvMod)/pmi->UEGDecayTime;
                        break;
                case EGS_SUSTAIN:
                        UEGCount = pmi->UEGSustainTime;
                        //UEGAmp = UEGTarget;
                        UEGAdd = 0;
                        break;
                case EGS_RELEASE:
                        UEGCount = pmi->UEGReleaseTime;
                        UEGAdd = -pmi->UEGSustainLevel/pmi->UEGReleaseTime;
						UEGTarget = 0;
                        break;
                case EGS_DONE:			// turn off
                        UEGState = EGS_NONE;
                        UEGCount = 0;
                        UEGAmp = 0;
						UEGTarget = 0;
						UEGAdd = 0;
						PitchFactor1 = 1.0;
						PitchFactor2 = 1.0;
						UEGMixAmt = 0;
						UEGPW1Amt = 0;
						UEGPW2Amt = 0;
						UEGReso = 0;
						UEGPhase = 0;

                        break;
                }
			UEGAmp += UEGAdd;
		}
		else
			return;

	PitchModActive = false;

	if(pmi->UEGDest && UEGAmp != 0.0)
	{
		int n;

		switch(pmi->UEGDest)
		{
		case 1:
			PitchFactor1 = LFOOscTab[(UEGAmp>>13) + 0x8000];
			PitchModActive = true;
			break;
		case 2:
			PitchFactor2 = LFOOscTab[(UEGAmp>>13) + 0x8000];
			PitchModActive = true;
			break;
		case 3:
			PitchFactor1 = PitchFactor2 = LFOOscTab[(UEGAmp>>13) + 0x8000];		
			PitchModActive = true;
			break;
		case 4:				// FIXME: use f2i
			UEGPW1Amt = (float)UEGAmp/(0xFFFFF<<7);
			break;
		case 5:				// FIXME: use f2i
			UEGPW2Amt = (float)UEGAmp/(0xFFFFF<<7);
			break;
		case 6:				// FIXME: use f2i
			UEGMixAmt = UEGAmp/(0xFFFFF);
			break;
		case 7:				// FIXME: use f2i
			n = pmi->LFO1Amount + (UEGAmp>>19);
			if(n > 256)
				n = 256;
			if(n < 0)
				n = 0;
			UpdateLFO1Amounts(n);
			break;
		case 8:
			n = pmi->LFO2Amount + (UEGAmp>>19);
			if(n > 255)
				n = 255;
			if(n < 0)
				n = 0;
			UpdateLFO2Amounts(n);
			break;
		case 9:				// FIXME: use f2i
			UEGReso = (int)(UEGAmp>>19);
			break;
		case 10:				// FIXME: use f2i
			UEGPhase = (int)(UEGAmp);
			break;

		}
	}

}


inline float CTrack::Filter( float x)
{
        float y;

// FIXME: We could save about 4 multiplies in here if we save FEGSustainLevel, maybe also 1/Decay, 1/Sustain, ect
        // Envelope
        if( FEGState) {
                if( !FEGCount--
				//|| (FEGState == EGS_ATTACK && (CutAdd > 0) && Cut >= CutTarget)
				//|| (FEGState == EGS_ATTACK && (CutAdd < 0) && Cut <= CutTarget)
				//|| (FEGState == EGS_DECAY && (CutAdd > 0) && Cut >= CutTarget)
				//|| (FEGState == EGS_DECAY && (CutAdd < 0) && Cut <= CutTarget)
				//|| (FEGState == EGS_RELEASE && (CutAdd > 0.0) && Cut >= CutTarget)
				//|| (FEGState == EGS_RELEASE && (CutAdd < 0.0) && Cut <= CutTarget)
					|| ((CutAdd < 0.0) && Cut < CutTarget)
					|| ((CutAdd > 0.0) && Cut > CutTarget)
					)
                        switch( ++FEGState)
                        {
						case EGS_DECAY:
								FEGCount = pmi->FEGDecayTime;
								//Cut = CutTarget;
								CutTarget =	pmi->FEGSustainLevel;
								CutAdd = (CutTarget - Cut)/(float)FEGCount;							
								break;
                        case EGS_SUSTAIN:
                                FEGCount = pmi->FEGSustainTime;
                                //Cut = CutTarget;
                                CutAdd = 0.0;
                                break;
                        case EGS_RELEASE:
                                FEGCount = pmi->FEGReleaseTime;
								//Cut = pmi->FEGSustainLevel;
                                CutAdd = -Cut/pmi->FEGReleaseTime;
								CutTarget = 0.0;
                                break;
                        case EGS_DONE:
                                FEGState = EGS_NONE; // false
                                FEGCount = 0;
                                Cut = 0.0;
                                CutAdd = 0.0;
								CutTarget = 0.0;
                                break;
                        }
                Cut += CutAdd;
        }

        // LFO
        // Cut
        int c, r;
        // Reso
        if( pmi->LFO_Reso || UEGReso) {

			if(pmi->LFO_Reso)
                r = pmi->Resonance + UEGReso +
                ((pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*LFO2AmountReso)>>(7+8));
			else
				r = pmi->Resonance + UEGReso;
	
        }
        else {
	        r = pmi->Resonance;
	    }

		if( r < 0)
				r = 0;
		else if( r > 127)
				r = 127;


		if(pmi->phat_philter)
		{
			if(Count)
				Count--;
			float cut=0, res=0;
			if(Count <= 0)
			{

				if( pmi->LFO_Cut)				// FIXME: use f2i
				{
					if(pmi->LFO_VCF)	  // f2i on Cut here, and OldOut.. 
					{
						OldOldOut += (OldOut - OldOldOut)*0.1;
			            cut = (256.0*(pmi->Cutoff + Cut)) + // Cut = EnvMod
						((f2i(OldOldOut)*LFO1AmountCutoff)>>7);
					}
					else
						cut = (256.0*(pmi->Cutoff + Cut)) + // Cut = EnvMod
							((pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*LFO1AmountCutoff)>>7);
				}
				else		
						cut = (pmi->Cutoff + Cut)*256.0; // Cut = EnvMod

				if(cut > 32768.0)
					cut = 32768.0;
				if(cut < 0.0)
					cut = 0.0;
				res = (float)(r+7)/7.0;

				//if(res < 1.0)
					//res = 1.0;
				//if(res > 20.0)
					//res = 20.0;
			}

			if(pmi->db24)
				iir.length = 2;
			else
				iir.length = 1;
	
			return iir_filter(x, cut, res);
		}
		else
		{
        if( pmi->LFO_Cut)				// FIXME: use f2i
		{
			if(pmi->LFO_VCF)	  // f2i on Cut here, and OldOut.. 
			{
				OldOldOut += (OldOut - OldOldOut)*0.5;
                c = pmi->Cutoff + Cut + // Cut = EnvMod
					((f2i(OldOldOut)*LFO1AmountCutoff)>>(7+8));
				
			}
			else
                c = pmi->Cutoff + Cut + // Cut = EnvMod
					((f2i(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21])*LFO1AmountCutoff)>>(7+8));
			}
			else		
                c = f2i(pmi->Cutoff + Cut); // Cut = EnvMod
		}

        if( c < 1)
                c = 1;
			else
                if( c > 127)
                        c = 127;

        int ofs = ((c<<7)+r)<<3;
        y = pmi->coefsTabOffs[ofs]*x +
                pmi->coefsTabOffs[ofs+1]*x1 +
                pmi->coefsTabOffs[ofs+2]*x2 +
                pmi->coefsTabOffs[ofs+3]*y1 +
                pmi->coefsTabOffs[ofs+4]*y2;

        y2=y1;
        y1=y;
        x2=x1;
        x1=x;

                if( !pmi->db24)
				{
					if(pmi->peak)
						return y + x;
					else
                        return y;
				}
                else { // 24 DB
                        float y24 = pmi->coefsTabOffs[ofs]*y +
                                pmi->coefsTabOffs[ofs+1]*x241 +
                                pmi->coefsTabOffs[ofs+2]*x242 +
                                pmi->coefsTabOffs[ofs+3]*y241 +
                                pmi->coefsTabOffs[ofs+4]*y242;
                        y242=y241;
                        y241=y24;
                        x242=x241;
                        x241=y;
                        if( !pmi->db18)
						{
							if(pmi->peak)
								return y24 + x;
							else
                                return y24;
						}
                        else
						{
							if(pmi->peak)
								return (y + y24 + x)*0.5;
							else
                                return (y+y24)*0.5;
						}
                }
}


inline float CTrack::iir_filter(float input, float Cut, float Resonance)
{
    unsigned int i;
    float *hist1_ptr,*hist2_ptr;						
	double *coef_ptr;
    float output,new_hist,history1,history2;
	double		*coef;
	unsigned	nInd;
	double		a0, a1, a2, b0, b1, b2, k, r;
	double CutFreq[2];

	if(Count <= 0)
	{
		if(iir.last_cutoff != Cut || iir.last_res != Resonance)		// if cutoff or res hasn't changed, save time
		{
			CutFreq[0] = 70 + pow((Cut/32768.0)*pow(22000, (float)1/2), 2);

			if(CutFreq[0] > 22020)
				CutFreq[0] = 22020;
			if(CutFreq[0] < 50)
				CutFreq[0] = 50;
			//if(CutFreq < 375)
				//CutFreq = 375;

			if(pmi->phat_philter == 3)
			{
				CutFreq[1] = CutFreq[0] * 0.8;
				if(CutFreq[1] < 50)
					CutFreq[1] = 50;
			}
			else
				CutFreq[1] = CutFreq[0];

			if(CutFreq[1] < 550.0)
			{
				r = (1.0 / Resonance) * ( ((550.0-CutFreq[1])/550.0)*10.0);
				if(r > 1.0)
					r = 1.0;
			}
			else
			{
				r = (1.0/Resonance);
			}
			if(r > 1.0)
				r = 1.0;
			if(r < 0.005)
				r = 0.005;

			k = 1.0;
			coef = iir.coef + 1;	
			for (nInd = 0; nInd < iir.length; nInd++)
			{
				a0 = ProtoCoef[nInd].a0;
				a1 = ProtoCoef[nInd].a1;
				a2 = ProtoCoef[nInd].a2;
				b0 = ProtoCoef[nInd].b0;
				b1 = ProtoCoef[nInd].b1 * r;
				b2 = ProtoCoef[nInd].b2;				 
				szxform(&a0, &a1, &a2, &b0, &b1, &b2, CutFreq[nInd], pmi->pMasterInfo->SamplesPerSec, &k, coef);
				coef += 4;							
			}
			iir.coef[0] = k;

			iir.last_cutoff = Cut;
			iir.last_res = Resonance;
			Count=24;
		}
	}

    coef_ptr = iir.coef;                /* coefficient pointer */

    hist1_ptr = iir.history;            /* first history */
    hist2_ptr = hist1_ptr + 1;           /* next history */

    output =(float) (input * (*coef_ptr++));

    for (i = 0 ; i < iir.length; i++)
	{
        history1 = *hist1_ptr;							/* history values */
        history2 = *hist2_ptr;

        output = (float) (output - history1 * (*coef_ptr++));
        new_hist = (float) (output - history2 * (*coef_ptr++));    /* poles */

        output = (float) (new_hist + history1 * (*coef_ptr++));
        output = (float) (output + history2 * (*coef_ptr++));      /* zeros */

        *hist2_ptr++ = *hist1_ptr;
        *hist1_ptr++ = new_hist;
        hist1_ptr++;
        hist2_ptr++;
    }

	if(pmi->phat_philter == 2)
		return(input-output);
	else
		return(output);
		
}

inline void CTrack::NewPhases()
{
	float pf, pitchmod1, pitchmod2;
	int phAdd1, phAdd2;

	// phasesub = phase1 * 0.5

	if( GlideActive) {
		pitchmod1 = GlideFactor;

		GlideFactor *= GlideMul;
        if( !GlideCount--) {
              GlideActive = false;
              PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
              PhaseAdd2 = (int)(Frequency*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
        }
	}
	else
	{
		pitchmod1 = 1.0;
	}

	if(pmi->BendFactor != 1.0)
	{
		pitchmod1 *= pmi->BendFactor;
	}

    if( pmi->LFO_Vib && !pmi->LFO_1Lock2) {
		pf = LFOVibOscTab[(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*LFO1AmountOsc1>>8) + 0x8000];

		pitchmod1 *= pf;
	}

	pitchmod2 = pitchmod1;


	if( pmi->LFO_Vib && pmi->LFO_1Lock2)
	{
		pf = LFOVibOscTab[(pmi->pwavetabLFO1[((unsigned)Phase2)>>16]*LFO1AmountOsc1>>8) + 0x8000];

		pitchmod1 *= pf;
	}

    if( pmi->LFO_Osc1) {
		if(pmi->LFO_1Lock2)
			pf = LFOOscTab[(pmi->pwavetabLFO1[((unsigned)Phase2)>>16]*LFO1AmountOsc1>>8) + 0x8000];
		else
			pf = LFOOscTab[(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*LFO1AmountOsc1>>8) + 0x8000];

		pitchmod1 *= pf;
	}

	if( pmi->LFO_Osc2) {
		if(pmi->LFO_2Lock1)
			pf = LFOOscTab[(pmi->pwavetabLFO2[((unsigned)Phase1)>>16]*LFO2AmountOsc2>>8) + 0x8000];
		else if(pmi->LFO_2Lock2)								
			pf = LFOOscTab[(pmi->pwavetabLFO2[((unsigned)Phase2)>>16]*LFO2AmountOsc2>>8) + 0x8000];
		else
			pf = LFOOscTab[(pmi->pwavetabLFO2[((unsigned)PhLFO1)>>21]*LFO2AmountOsc2>>8) + 0x8000];

		pitchmod2 *= pf;
	}

	if(PitchModActive)
	{
		pitchmod1 *= PitchFactor1;
		pitchmod2 *= PitchFactor2;
	}

	if(pitchmod1 != 1.0)
	{
		phAdd1 = f2i(PhaseAdd1*pitchmod1);
		//LevelShift1 =  (int)ceil(log((float)phAdd1/65536) * (1.0 / log(2)));
	}
	else
		phAdd1 = PhaseAdd1;

	
	

	Phase1 += phAdd1;
	PhaseSub += (phAdd1>>1);

	if(pitchmod2 != 1.0)
	{
		phAdd2 = f2i(PhaseAdd2*pitchmod2);
		//LevelShift2 =  (int)ceil(log((float)phAdd2/65536) * (1.0 / log(2)));
		Phase2 += phAdd2;
	}
	else
		Phase2 += PhaseAdd2;


        if( Phase1 & 0xf8000000) { // neuer durchlauf ??
                // PW1

                if( pmi->LFO_PW1 || UEGPW1Amt != 0) { //LFO_PW_Mod
						center1 = pmi->Center1;

						if(pmi->LFO_PW1)
							center1 += (float)pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*
                                                LFO1AmountPW1;
						center1 += UEGPW1Amt;

                        if( center1 < 0)
                                center1 = 0;
                        else
                                if( center1 > 1)
                                        center1 = 1;
                }
                else  // No LFO
	                center1 = pmi->Center1;

                PhScale1A = 0.5/center1;	
                PhScale1B = 0.5/(1-center1);
                c1 = f2i(center1*0x8000000);
                // PW2
                if( pmi->LFO_PW2 || UEGPW2Amt != 0) { //LFO_PW_Mod
                        center2 = pmi->Center2;

						if(pmi->LFO_PW2)
							center2 += (float)pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*
                                                LFO2AmountPW2;

						center2 += UEGPW2Amt;

                        if( center2 < 0)
                                center2 = 0;
                        else
                                if( center2 > 1)
                                        center2 = 1;
                }
                else { // No LFO
					center2 = pmi->Center2;
				}

					PhScale2A = 0.5/center2;
		            PhScale2B = 0.5/(1-center2);
                                c2 = f2i(center2*0x8000000);

				// blargh
        }

                // SYNC
					if(Phase1 >= 0x8000000)
					{
						if( pmi->Sync == 1)	 
							Phase2 = 0; // !!!!!
					}

        Phase1 &= 0x7ffffff;
        Phase2 &= 0x7ffffff;
        PhaseSub &= 0x7ffffff;

        if( Phase1 < c1)
                Ph1 = f2i(Phase1*PhScale1A);
        else
                Ph1 = f2i((Phase1 - c1)*PhScale1B + 0x4000000);

        if( Phase2 < c2)
                Ph2 = f2i(Phase2*PhScale2A);
        else {
                Ph2 = f2i((Phase2 - c2)*PhScale2B + 0x4000000);
        }

		Ph2 = (Ph2 + pmi->PhaseDiff2 + UEGPhase);

		if(pmi->LFO_Phase2)
			Ph2 += ((f2i(pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21])*pmi->LFO2Amount));

		Ph2 &= 0x7ffffff;


                // LFOs
		if(pmi->LFO_1Lock2)
			PhLFO1 = Phase2 << 5;
		else if(pmi->LFO_LFO1)				
			PhLFO1 += (int)((double)0x200000*2048/(pmi->pMasterInfo->SamplesPerTick<<f2i(pmi->LFO1Freq*(pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21])*pmi->LFO2Amount)>>(7+8)));
		else
			PhLFO1 += pmi->PhaseAddLFO1;


		if(pmi->LFO_2Lock1)
			PhLFO2 = Phase1 << 5;
		else if(pmi->LFO_2Lock2)
			PhLFO2 = Phase2 << 5;
		else
			PhLFO2 += pmi->PhaseAddLFO2;

}

void CTrack::Work( float *psamples, int numsamples)
{
	if(pmi->WaveTableWave)
		pLevel = pmi->pCB->GetNearestWaveLevel(pmi->WaveTableWave, Note);
	else
		pLevel = NULL;

	if(pLevel && Wavestate.Active && pmi->pWave)
	{
		Waveparms.Samples = pLevel->pSamples;

		if(pLevel->numSamples < Waveparms.numSamples)		// if wave changed, make sure we don't go past the end
			Waveparms.numSamples = pLevel->numSamples;

		DSP_Resample(psamples, numsamples, Wavestate, Waveparms);

        for( int i=0; i<numsamples; i++) {
				pmi->Cutoff += pmi->CutoffAdd;

				if(pmi->CutoffAdd > 0.0 && pmi->Cutoff > pmi->CutoffTarget)
				{
						pmi->Cutoff = pmi->CutoffTarget;
						//pmi->CutoffAdd = 0;			
				}
				else if(pmi->CutoffAdd < 0.0 && pmi->Cutoff < pmi->CutoffTarget)
				{
						pmi->Cutoff = pmi->CutoffTarget;
						//pmi->CutoffAdd = 0;
				}



                if( AEGState) {
					float s;

					UEG();		// update user envelope

                    float o = (Osc() + (*psamples)) * VCA();
                        s = Filter(o + OldOut); // anti knack
						//s = o;
                        OldOut = o;

						if(pmi->Dist == 1)
						{
							if(s > 16000.0f)
								s = 32768.0f;
							if(s < -16000.0f)
								s = -32768.0f;
						}
						else if(pmi->Dist == 2)
						{
							if(s > 32768.0f)
								s = 8000.0f;
							if(s < -32768.0f)
								s = -8000.0f;
						}
						else if(pmi->Dist == 3)
						{
							if(s > 32768.0f)
								s = 32768.0f;
							if(s < -32768.0f)
								s = -32768.0f;
						}
						else if(pmi->Dist == 4)
						{
							int index = (f2i(s) >> 9) + 128;
							int index2 = index + 1;

							if(index < 0)		// clip
								index = 0;
							if(index > 255) 
								index = 255;
							if(index2 < 0)
								index2 = 0;
							if(index2 > 255) index2 = 255;


							s = LInterpolateI(nonlinTab[index], nonlinTab[index2], f2i(s*256.0) & 0xff);
						}
						*psamples++ = s;
                }
                else
				{
						Filter(0);
                        *psamples++ = 0;
				}
                NewPhases();
        }

	}
	else
        for( int i=0; i<numsamples; i++) {
				pmi->Cutoff += pmi->CutoffAdd;

				if(pmi->CutoffAdd > 0.0 && pmi->Cutoff > pmi->CutoffTarget)
				{
						pmi->Cutoff = pmi->CutoffTarget;
						//pmi->CutoffAdd = 0;			
				}
				else if(pmi->CutoffAdd < 0.0 && pmi->Cutoff < pmi->CutoffTarget)
				{
						pmi->Cutoff = pmi->CutoffTarget;
						//pmi->CutoffAdd = 0;
				}

                if( AEGState) {
					float s;

					UEG();		// update user envelope

                    float o = Osc()*VCA();
                        s = Filter(o + OldOut); // anti knack
						//s = o;
                        OldOut = o;

						if(pmi->Dist == 1)
						{
							if(s > 16000.0f)
								s = 32768.0f;
							if(s < -16000.0f)
								s = -32768.0f;
						}
						else if(pmi->Dist == 2)
						{
							if(s > 32768.0f)
								s = 8000.0f;
							if(s < -32768.0f)
								s = -8000.0f;
						}
						else if(pmi->Dist == 3)
						{
							if(s > 32768.0f)
								s = 32768.0f;
							if(s < -32768.0f)
								s = -32768.0f;
						}
						else if(pmi->Dist == 4)
						{
							int index = (f2i(s) >> 9) + 128;
							int index2 = index + 1;

							if(index < 0)		// clip
								index = 0;
							if(index > 255) 
								index = 255;
							if(index2 < 0)
								index2 = 0;
							if(index2 > 255) index2 = 255;


							s = LInterpolateI(nonlinTab[index], nonlinTab[index2], f2i(s*256.0) & 0xff);
						}
						*psamples++ = s;
                }
                else
				{
						Filter(0);
                        *psamples++ = 0;					
				}

                NewPhases();
        }
}

//.......................................................
//.................... DESCRIPTION ......................
//.......................................................

char const *mi::DescribeValue(int const param, int const value)
{
    static const char *MixTypeTab[14] = {
            "add",
            "Abs",
            "mul",
            "highest amp",
            "lowest amp",
            "and",
            "or",
            "xor",
            "?",
				"AM",
				"AM2",
				"Pixelate",
				"O-drive",
				"sub"
				};

        static const char *UEGDestTab[11] = {
                "none",
                "osc1",
                "osc2",
				"All Osc",
                "p.width1",
                "p.width2",
                "mix",
				"LFO1Depth",
				"LFO2Depth",
				"Resonance",
				"Phase2Diff"

        };

       static const char *LFO1DestTab[18] = {
                "none",
                "osc1",
                "p.width1",
                "volume",
                "cutoff",
                "osc1+pw1", // 12
                "osc1+volume", // 13
                "osc1+cutoff", // 14
                "pw1+volume", // 23
                "pw1+cutoff", // 24
                "vol+cutoff", // 34
                "o1+pw1+vol",// 123
                "o1+pw1+cut",// 124
                "o1+vol+cut",// 134
                "pw1+vol+cut",// 234
                "all",// 1234
				"Vibrato",		// Specials:
				"VCF"
        };					 

        static const char *LFOWaveTab[9] = {
                "sine",
                "saw",
                "square",
                "triangle",
                "random",
				"stepup",
				"stepdn",
				"Wacky1",
				"Wacky2"
        };

        static const char *PlaymodeTab[10] = {
				"Reg",
				"Init Amp",
				"Init LFO",
				"Amp+LFO",
				"Init Flt",
				"Amp+Flt",
				"LFO+Flt",
				"Amp+LFO+Flt",
				"Mono1",
				"Mono2"
		
		};

        static const char *LFO2DestTab[17] = {			
                "none",
                "osc2",
                "p.width2",
                "mix",
                "resonance",
                "osc2+pw2", // 12
                "osc2+mix", // 13
                "osc2+res", // 14
                "pw2+mix", // 23
                "pw2+res", // 24
                "mix+res", // 34
                "o2+pw2+mix", // 123
                "o2+pw2+res", // 124
                "o2+mix+res", // 134
                "pw2+mix+res", // 234
                "all", // 1234
				"Phase2"				

        };

        static const char *FilterTypeTab[14] = {
                "lowpass24",
                "lowpass18",
                "lowpass12",
                "highpass",
                "bandpass",
                "bandreject",
				"peak24",
				"bp24",
				"hp24",
				"phatlp12",
				"phathp12",
				"phatlp24",
				"phathp24",
				"phatlinked"
		};

        static const char *ModDest1Tab[9] = {
				"Off",
                "FEnvMod",
                "Cutoff",
				"Mix",
				"Osc2 Fine",
				"LFO1 Amount",
				"LFO1 Freq",
				"PulseWidth1",
				"Osc2 Semi"
		};

        static const char *ModDest2Tab[12] = {
				"Off",
                "UEnvMod",
                "Resonance",
				"Phase2",
				"AmpGain",
				"LFO2 Amount",
				"LFO2 Freq",
				"PulseWidth2",
				"F.Attack",
				"F.Decay",
				"F.Release",
				"F. Level",
		};


#include "waves/wavename.inc"

		//return NULL;

        static char txt[20];

        switch(param){
		case 0:
				return PlaymodeTab[value];
				break;
		case 3:	// PITCH bend amount
                sprintf( txt, "+/-%i halfnotes", value);
				break;

		case 5:
			sprintf(txt, "%x", value);
			break;
		case 6:		// fixed note
                if( value == SWITCH_ON)
                        return( "yes");
                else
                        return( "no");
                break;
        case 8: // OSC1Wave
        case 10: // OSC2Wave
        case 17:// SubOscWave
                                return( wavenames[value]);
                                break;
        case 9: // PW1
        case 11: // PW2
                sprintf(txt, "%u : %u", (int)(value*100.0/127),
                                                                100-(int)(value*100.0/127));
                break;
		case 7:
        case 12: // semi detune
                if( value == 0x40)
                        return "±0 halfnotes";
                else
                        if( value > 0x40)
                                sprintf( txt, "+%i halfnotes", value-0x40);
                        else
                                sprintf( txt, "%i halfnotes", value-0x40);
                break;
        case 13: // fine detune
                if( value == 0x40)
                        return "±0 cents";
                else
                        if( value > 0x40)
                                sprintf( txt, "+%i cents", (int)((value-0x40)*100.0/63));
                        else
                                sprintf( txt, "%i cents", (int)((value-0x40)*100.0/63));
                break;

        case 31: // distortion
                if( value == 4)
                        return( "Dist4");
                if( value == 3)
                        return( "Dist3");
               else if(value == 2)
						return( "Dist2");
                else if(value == 1)
						return( "Dist1");
				else
                        return( "off");
                break;
		case 14:		// sync
                if( value == 1)
                        return( "Osc2");
				else
                        return( "no");
                break;

        case 15: // MixType
                                return MixTypeTab[value];
                                break;
        case 16: // Mix 
                switch( value) {
					case 0:return "Osc1";
					case 127:return "Osc2";
					default: sprintf(txt, "%u%% : %u%%", 100-(int)(value*100.0/127),
                                                                (int)(value*100.0/127));
                }
                break;

		case 19: // User Env
        case 20: // User Env
		case 21: // User Env
        case 23: // User Env

        case 25: // Amp Env
        case 26: // Amp Env
        case 27: // Amp Env
        case 29: // Amp Env

        case 34: // Filter Env
        case 35: // Filter Env
        case 36: // Filter Env
		case 38: // Filter Env
				if(value == 128)
					return("Infinite");
				else
					sprintf( txt, "%.4f sec", scalEnvTime( value)/1000);
                break;

		case 22: // User level
		case 28: // Amp level
		case 37: // Filt level
                sprintf( txt, "%f%%", (float)value/1.27);
                break;

		case 24: // User EnvMod
		case 39: // Filt ENvMod
		case 53: // mod amount1
		case 55: // mod amount2
                sprintf( txt, "%i", value-0x40);
                break;

		case 30: //FilterType
                                return FilterTypeTab[value];
                                break;
		case 50: //UEGDest
                                return UEGDestTab[value];
                                break;
		case 40: //LFO1Dest
                                return LFO1DestTab[value];
                                break;
        case 45: // LFO2Dest
                                return LFO2DestTab[value];
                                break;
        case 41: // LFO1Wave
        case 46: // LFO2Wave
                                return LFOWaveTab[value];
                                break;
        case 42: // LFO1Freq
        case 47: // LFO2Freq
                if( value <= 116)
                        sprintf( txt, "%.4f HZ", scalLFOFreq( value));
                else if( value <= 127)
                        sprintf( txt, "%u ticks", 1<<(value-117));
				else if(value == 128)
						sprintf( txt, "LFO->O2");
				else if(value == 129)
						sprintf( txt, "LFO->O1");
                break;
        case 44: //LFO1PhaseDiff
        case 49: //LFO2PhaseDiff
                        sprintf( txt, "%i°", value*360/128);
                        break;
		case 52:
				return ModDest1Tab[value];
				break;
		case 54:
				return ModDest2Tab[value];
				break;
		case 56:
			if(value <= 32)
				sprintf( txt, "%.3f%%", ((float)value*100.0/32));
			else if(value < 200)
				sprintf( txt, "%.3f%%", ((float)(value-16)*100.0/16));	
			else
				return("God Help you");
			break;
        default: return NULL;
                }
        return txt;
}



//////////////////////////////////////////////////////
// MACHINE INTERFACE METHODEN
//////////////////////////////////////////////////////

mi::mi()
{
        GlobalVals = &gval;
        TrackVals = tval;
        AttrVals = (int *)&aval;
}

mi::~mi()
{
    for (int c = 0; c < numTracks; c++)
     {
		delete[] Tracks[c].iir.coef;
		delete[] Tracks[c].iir.history;
     }

}

void mi::MidiNote(int const channel, int const value, int const velocity)
{
	int v2;

	if(aval.MIDIChannel != 0) {
		if(channel != aval.MIDIChannel-1)
			return;
	}

		v2 = value + aval.MIDITranspose-24;

         if (v2 / 12 > 9)
          return;
         byte n = ((v2 / 12) << 4) | ((v2 % 12) + 1);
         if (velocity > 0)
         {
				if(Playmode & 8)			// Always use track zero
				{
                         Tracks[0].Note = n;
						 if(aval.MIDIVelocity == 1)
							Tracks[0].Volume = velocity<<20;
						 else if (aval.MIDIVelocity == 2)
						 {
							 gval.ModWheel = velocity;
							 Tick();
						 }

	                     Tracks[0].NoteOn();
                         return;
				}
				else
                 for (int c = 0; c < numTracks; c++)
                 {
                         if (Tracks[c].Note == NOTE_NO || Tracks[c].AEGState > EGS_RELEASE || Tracks[c].Note == n)		
                         {
                                 Tracks[c].Note = n;
								 if(aval.MIDIVelocity == 1)
									Tracks[c].Volume = velocity<<20;
								 else if (aval.MIDIVelocity == 2)
								 {
									 gval.ModWheel = velocity;
									 Tick();
								 }

	                             Tracks[c].NoteOn();
                                 return;
                         }
                 }
         }
         else
         {
                 for (int c = 0; c < numTracks; c++)
                 {
                         if (Tracks[c].Note == n)
                         {
                                 Tracks[c].NoteOff();
                                 return;
                         }
                 }
         }
}

void mi::Init(CMachineDataInput * const pi)
{
		thismachine = pCB->GetThisMachine();
		InitSpline();

        TabSizeDivSampleFreq = (float)(2048.0/(float)pMasterInfo->SamplesPerSec);

        // Filter
        coefsTabOffs = coefsTab; // LowPass
        Cutoff = paraCutoff.DefValue;
        Resonance = paraResonance.DefValue;
        peak = db24 = db18 = false;
		phat_philter = 0;
		// Dist
		Dist = 0;
        //UEG
        UEGAttackTime = MSToSamples( scalEnvTime( paraUEGAttackTime.DefValue));
        UEGDecayTime = MSToSamples( scalEnvTime( paraUEGDecayTime.DefValue));
		UEGSustainTime = MSToSamples( scalEnvTime( paraUEGSustainTime.DefValue));
		UEGReleaseTime = MSToSamples( scalEnvTime( paraUEGReleaseTime.DefValue));
		UEGSustainLevel = 127;
        //UEnvMod = 0;
        UEnvMod = (paraUEnvMod.DefValue-0x40) << 20;
        // FEG
        FEGAttackTime = MSToSamples( scalEnvTime( paraFEGAttackTime.DefValue));
		FEGDecayTime = MSToSamples( scalEnvTime( paraFEGDecayTime.DefValue));
        FEGSustainTime = MSToSamples( scalEnvTime( paraFEGSustainTime.DefValue));
		FEGSustainLevel = 1.0;
        FEGReleaseTime = MSToSamples( scalEnvTime( paraFEGReleaseTime.DefValue));
        FEnvMod = 0;
        // AEG
        AEGAttackTime = MSToSamples( scalEnvTime( paraAEGAttackTime.DefValue));
		AEGDecayTime = MSToSamples( scalEnvTime( paraAEGSustainTime.DefValue));
        AEGSustainTime = MSToSamples( scalEnvTime( paraAEGSustainTime.DefValue));
		AEGSustainLevel = 127;
        AEGReleaseTime = MSToSamples( scalEnvTime( paraAEGReleaseTime.DefValue));

		WaveTableWave = 0;
		WaveDetuneSemi = 0;
		WaveFixedPitch = 0;

        pwavetab1 = pwavetab2 = pwavetabsub = pCB->GetOscillatorTable(0);
		pWave = NULL;

        oscwave1 = oscwave2 = oscwave3 = -1;
		noise1 = noise2 = Sync = false;
        LFO1Noise = LFO2Noise = false;
        LFO1Synced = LFO2Synced = false;

		BendFactor = 1.0;
		BendTime = 0;
		BendGlide = 1.0;
		PitchMod = 0.0;

		UEGDest = 0;

        PhaseLFO1 = PhaseLFO2 = 0;

        pwavetabLFO1 = pwavetabLFO2 = pCB->GetOscillatorTable( OWF_SINE);
        DetuneSemi = DetuneFine = 1.0;

        PhaseAddLFO1 = PhaseAddLFO2 = 0;

        SubOscVol = paraSubOscVol.DefValue;

        // PulseWidth
        Center1 = (float)(paraPulseWidth1.DefValue/127.0);
        Center2 = (float)(paraPulseWidth2.DefValue/127.0);
        LFO1Amount = LFO2Amount = 0;
        LFO1PhaseDiff = paraLFO1PhaseDiff.DefValue<<(9+16);
        LFO2PhaseDiff = paraLFO2PhaseDiff.DefValue<<(9+16);
		PhaseDiff2 = 0;

        // OscMix
        Bal1 = 127-paraMix.DefValue;
        Bal2 = paraMix.DefValue;
        MixType = 0;

		Playmode = 0;

        LFO_1Lock2 = LFO_VCF = LFO_Vib = LFO_Osc1 = LFO_PW1 = LFO_Amp = LFO_Cut = false;
        LFO_2Lock1 = LFO_2Lock2 = LFO_LFO1 = LFO_Osc2 = LFO_PW2 = LFO_Mix = LFO_Phase2 = LFO_Reso = false;

		ModWheel = 0;
		ModDest1 = 0;
		ModDest2 = 0;
		ModAmount1 = 0;
		ModAmount2 = 0;

// init ctl values
		ctlval.Mode = paraMode.DefValue;

		ctlval.ModWheel = paraModWheel.DefValue;
		ctlval.PitchWheel = paraPitchWheel.DefValue;
		ctlval.PitchBendAmt = paraPitchBendAmt.DefValue;
		ctlval.WavetableOsc = paraWavetableOsc.DefValue;
		ctlval.FixedPitch = paraFixedPitch.DefValue;
		ctlval.WaveDetuneSemi = paraWaveDetuneSemi.DefValue;

        ctlval.Wave1 = paraWave1.DefValue;
        ctlval.PulseWidth1 = paraPulseWidth1.DefValue;
        ctlval.Wave2 = paraWave2.DefValue;
        ctlval.PulseWidth2 = paraPulseWidth2.DefValue;
        ctlval.DetuneSemi = paraDetuneSemi.DefValue;
        ctlval.DetuneFine = paraDetuneFine.DefValue;
        ctlval.Sync = paraSync.DefValue;
        ctlval.MixType = paraMixType.DefValue;
        ctlval.Mix = paraMix.DefValue;
        ctlval.SubOscWave = paraSubOscWave.DefValue;
        ctlval.SubOscVol = paraSubOscVol.DefValue;
		ctlval.UEGAttackTime = paraUEGAttackTime.DefValue;
        ctlval.UEGDecayTime = paraUEGDecayTime.DefValue;
		ctlval.UEGSustainTime = paraUEGSustainTime.DefValue;
		ctlval.UEGSustainLevel = paraUEGSustainLevel.DefValue;
		ctlval.UEGReleaseTime = paraUEGReleaseTime.DefValue;
        ctlval.UEnvMod = paraUEnvMod.DefValue;
        ctlval.Glide = paraGlide.DefValue;

		ctlval.AEGAttackTime = paraAEGAttackTime.DefValue;
        ctlval.AEGDecayTime = paraAEGDecayTime.DefValue;
		ctlval.AEGSustainTime = paraAEGSustainTime.DefValue;
		ctlval.AEGSustainLevel = paraAEGSustainLevel.DefValue;
		ctlval.AEGReleaseTime = paraAEGReleaseTime.DefValue;

        ctlval.FilterType = paraFilterType.DefValue;
		ctlval.Dist = paraDist.DefValue;
        ctlval.Cutoff = paraCutoff.DefValue;
        ctlval.Resonance = paraResonance.DefValue;
		ctlval.FEGAttackTime = paraFEGAttackTime.DefValue;
        ctlval.FEGDecayTime = paraFEGDecayTime.DefValue;
		ctlval.FEGSustainTime = paraFEGSustainTime.DefValue;
		ctlval.FEGSustainLevel = paraFEGSustainLevel.DefValue;
		ctlval.FEGReleaseTime = paraFEGReleaseTime.DefValue;
        ctlval.FEnvMod = paraFEnvMod.DefValue;

        ctlval.Phase2 = paraPhase2.DefValue;

        // LFO 1
        ctlval.LFO1Dest = paraLFO1Dest.DefValue;
        ctlval.LFO1Wave = paraLFO1Wave.DefValue;
        ctlval.LFO1Freq = paraLFO1Freq.DefValue;

        ctlval.LFO1Amount = paraLFO1Amount.DefValue;
        ctlval.LFO1PhaseDiff = paraLFO1PhaseDiff.DefValue;
        // LFO 2
        ctlval.LFO2Dest = paraLFO2Dest.DefValue;
        ctlval.LFO2Wave = paraLFO2Wave.DefValue;
        ctlval.LFO2Freq = paraLFO2Freq.DefValue;

        ctlval.LFO2Amount = paraLFO2Amount.DefValue;
        ctlval.LFO2PhaseDiff = paraLFO2PhaseDiff.DefValue;

        ctlval.UEGDest = paraUEGDest.DefValue;
        ctlval.ModDest1 = paraModDest1.DefValue;
		ctlval.ModAmount1 = paraModAmount1.DefValue;
		ctlval.ModDest2 = paraModDest2.DefValue;
		ctlval.ModAmount2 = paraModAmount2.DefValue;

		int i;
    for(i=0; i<MAX_TRACKS; i++)
    {
            Tracks[i].pmi = this;
            Tracks[i].Init();
    }

		// Generate nonlinTab
		for(i=0; i<128; i++)
		{	
			nonlinTab[i] = -(int)(pow( fabs((float)i-127.0)*128.0, 1.5)/150.0);

			if(nonlinTab[i] < -30000)
			{																		
				nonlinTab[i] = -30000 - (int)pow((float)(-nonlinTab[i] + 30000), 0.82);
			}
		}
		for(i=128; i<256; i++)
		{	
			nonlinTab[i] = (int)(pow((float)(i-127)*128.0, 1.5)/80.0);

			if(nonlinTab[i] > 30000)
			{
				nonlinTab[i] = 30000 + (int)pow((float)(nonlinTab[i] - 30000), 0.82);
			}
		}

        // generate coefsTab
        for( int t=0; t<4; t++)
                for( int f=0; f<128; f++)
                        for( int r=0; r<128; r++)
                                ComputeCoefs( coefsTab+(t*128*128+f*128+r)*8, f, r, t);
        int p;
        // generate LFOOscTab
        for(p=0; p<0x10000; p++)
			//LFOOscTab[p] = pow( 1.00004230724139582, p-0x8000);	// old way
            LFOOscTab[p] = pow(NOTECONST, (float)(p-0x8000)/((float)0x8000/64.0));

        for( p=0; p<0x10000; p++)
			//LFOOscTab[p] = pow( 1.00004230724139582, p-0x8000);	// old way
            LFOVibOscTab[p] = pow(NOTECONST, (float)(p-0x8000)/((float)0x8000/2.0));
}


void mi::Tick()
{

	bool mod = false;

		if(gval.ModDest1 != paraModDest1.NoValue)
		{
			ctlval.ModDest1 = gval.ModDest1;
			ModDest1 = gval.ModDest1;
			if(gval.FEnvMod == paraFEnvMod.NoValue)
				gval.FEnvMod = ctlval.FEnvMod;
			if(gval.Cutoff == paraCutoff.NoValue)
				gval.Cutoff = ctlval.Cutoff;
			if(gval.Mix == paraMix.NoValue)
				gval.Mix = ctlval.Mix;
			if(gval.DetuneFine == paraDetuneFine.NoValue)
				gval.DetuneFine = ctlval.DetuneFine;
			if(gval.LFO1Freq == paraLFO1Freq.NoValue)
				gval.LFO1Freq = ctlval.LFO1Freq;
			if(gval.LFO1Amount == paraLFO1Amount.NoValue)
				gval.LFO1Amount = ctlval.LFO1Amount;
			if(gval.PulseWidth1 == paraPulseWidth1.NoValue)
				gval.PulseWidth1 = ctlval.PulseWidth1;
			if(gval.DetuneSemi == paraDetuneSemi.NoValue)
				gval.DetuneSemi = ctlval.DetuneSemi;

		}
		if(gval.ModAmount1 != paraModAmount1.NoValue)
		{
			ctlval.ModAmount1 = gval.ModAmount1;
			ModAmount1 = (gval.ModAmount1-0x40)<<1;
		}
		if(gval.ModDest2 != paraModDest2.NoValue)
		{
			ctlval.ModDest1 = gval.ModDest2;
			ModDest2 = gval.ModDest2;
			if(gval.UEnvMod == paraUEnvMod.NoValue)
				gval.UEnvMod = ctlval.UEnvMod;
			if(gval.Resonance == paraResonance.NoValue)
				gval.Resonance = ctlval.Resonance;
			if(gval.Phase2 == paraPhase2.NoValue)
				gval.Phase2 = ctlval.Phase2;
			if(gval.AmpGain == paraAmpGain.NoValue)
				gval.AmpGain = ctlval.AmpGain;
			if(gval.LFO2Amount == paraLFO2Amount.NoValue)
				gval.LFO2Amount = ctlval.LFO2Amount;
			if(gval.LFO2Freq == paraLFO2Freq.NoValue)
				gval.LFO2Freq = ctlval.LFO2Freq;
			if(gval.PulseWidth2 == paraPulseWidth2.NoValue)
				gval.PulseWidth2 = ctlval.PulseWidth2;
			if(gval.FEGAttackTime == paraFEGAttackTime.NoValue)
				gval.FEGAttackTime = ctlval.FEGAttackTime;
			if(gval.FEGDecayTime == paraFEGDecayTime.NoValue)
				gval.FEGDecayTime = ctlval.FEGDecayTime;
			if(gval.FEGReleaseTime == paraFEGReleaseTime.NoValue)
				gval.FEGReleaseTime = ctlval.FEGReleaseTime;
			if(gval.FEGSustainLevel == paraFEGSustainLevel.NoValue)
				gval.FEGSustainLevel = ctlval.FEGSustainLevel;
		}
		if(gval.ModAmount2 != paraModAmount2.NoValue)
		{
			ctlval.ModAmount2 = gval.ModAmount2;
		 	ModAmount2 = (gval.ModAmount2-0x40)<<1;
		}
		if(gval.ModWheel != paraModWheel.NoValue)
		{
			ctlval.ModWheel = gval.ModWheel;
			ModWheel = gval.ModWheel;
			mod = true;	
			int newval;

			switch(ModDest1)
			{
			case 1:				// FEnvMod
				if(gval.FEnvMod == paraFEnvMod.NoValue)
					newval = (ctlval.FEnvMod + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.FEnvMod = gval.FEnvMod;
					newval = (gval.FEnvMod + ((ModAmount1 * ModWheel)>>7));
				}
				if(newval > paraFEnvMod.MaxValue)
					newval = paraFEnvMod.MaxValue;
				if(newval < paraFEnvMod.MinValue)
					newval = paraFEnvMod.MinValue;

				gval.FEnvMod = newval;
				break;
			case 2:				// Cutoff
				if(gval.Cutoff == paraCutoff.NoValue)
					newval = ((int)ctlval.Cutoff + (((int)ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.Cutoff = gval.Cutoff;
					newval = ((int)gval.Cutoff + (((int)ModAmount1 * ModWheel)>>7));
				}

				if(newval > paraCutoff.MaxValue)
					newval = paraCutoff.MaxValue;
				if(newval < paraCutoff.MinValue)
					newval = paraCutoff.MinValue;

				gval.Cutoff = newval;

				break;
			case 3:				// Mix
				if(gval.Mix == paraMix.NoValue)
					newval = (ctlval.Mix + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.Mix = gval.Mix;
					newval = (gval.Mix + ((ModAmount1 * ModWheel)>>7));
				}

				if(newval > paraMix.MaxValue)
					newval = paraMix.MaxValue;
				if(newval < paraMix.MinValue)
					newval = paraMix.MinValue;

				gval.Mix = newval;
				break;
			case 4:				// Detune
				if(gval.DetuneFine == paraDetuneFine.NoValue)
					newval = (ctlval.DetuneFine + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.DetuneFine = gval.DetuneFine;
					newval = (gval.DetuneFine + ((ModAmount1 * ModWheel)>>7));
				}

				if(newval > paraDetuneFine.MaxValue)
					newval = paraDetuneFine.MaxValue;
				if(newval < paraDetuneFine.MinValue)
					newval = paraDetuneFine.MinValue;

				gval.DetuneFine = newval;
				break;
			case 5:				// LFO1
				if(gval.LFO1Amount == paraLFO1Amount.NoValue)
					newval = (ctlval.LFO1Amount + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.LFO1Amount = gval.LFO1Amount;
					newval = (gval.LFO1Amount + ((ModAmount1 * ModWheel)>>7));
				}
				if(newval > paraLFO1Amount.MaxValue)
					newval = paraLFO1Amount.MaxValue;
				if(newval < paraLFO1Amount.MinValue)
					newval = paraLFO1Amount.MinValue;

				gval.LFO1Amount = newval;
				break;
			case 6:				// LFO1
				if(gval.LFO1Freq == paraLFO1Freq.NoValue)
					newval = (ctlval.LFO1Freq + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.LFO1Freq = gval.LFO1Freq;
					newval = (gval.LFO1Freq + ((ModAmount1 * ModWheel)>>7));
				}
				if(newval > paraLFO1Freq.MaxValue)
					newval = paraLFO1Freq.MaxValue;
				if(newval < paraLFO1Freq.MinValue)
					newval = paraLFO1Freq.MinValue;

				gval.LFO1Freq = newval;
				break;
			case 7:				// PW1
				if(gval.PulseWidth1 == paraPulseWidth1.NoValue)
					newval = (ctlval.PulseWidth1 + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.PulseWidth1 = gval.PulseWidth1;
					newval = (gval.PulseWidth1 + ((ModAmount1 * ModWheel)>>7));
				}
				if(newval > paraPulseWidth1.MaxValue)
					newval = paraPulseWidth1.MaxValue;
				if(newval < paraPulseWidth1.MinValue)
					newval = paraPulseWidth1.MinValue;

				gval.PulseWidth1 = newval;
				break;
			case 8:				// Detune semi
				if(gval.DetuneSemi == paraDetuneSemi.NoValue)
					newval = (ctlval.DetuneSemi + ((ModAmount1 * ModWheel)>>7));
				else
				{
					ctlval.DetuneSemi = gval.DetuneSemi;
					newval = (gval.DetuneSemi + ((ModAmount1 * ModWheel)>>7));
				}

				if(newval > paraDetuneSemi.MaxValue)
					newval = paraDetuneSemi.MaxValue;
				if(newval < paraDetuneSemi.MinValue)
					newval = paraDetuneSemi.MinValue;

				gval.DetuneSemi = newval;
				break;
			}
			switch(ModDest2)
			{
			case 1:			
				if(gval.UEnvMod == paraUEnvMod.NoValue)
					newval = (ctlval.UEnvMod + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.UEnvMod = gval.UEnvMod;
					newval = (gval.UEnvMod + ((ModAmount2 * ModWheel)>>7));			
				}

				if(newval > paraUEnvMod.MaxValue)
					newval = paraUEnvMod.MaxValue;
				if(newval < paraUEnvMod.MinValue)
					newval = paraUEnvMod.MinValue;

				gval.UEnvMod = newval;
				break;
			case 2:				// Resonance
				if(gval.Resonance == paraResonance.NoValue)
					newval = ((int)ctlval.Resonance + (((int)ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.Resonance = gval.Resonance;
					newval = ((int)gval.Resonance + (((int)ModAmount2 * ModWheel)>>7));
				}

				if(newval > paraResonance.MaxValue)
					newval = paraResonance.MaxValue;
				if(newval < paraResonance.MinValue)
					newval = paraResonance.MinValue;

				gval.Resonance = newval;
				break;
			case 3:				// Phase2
				if(gval.Phase2 == paraPhase2.NoValue)
					newval = (ctlval.Phase2 + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.Phase2 = gval.Phase2;
					newval = (gval.Phase2 + ((ModAmount2 * ModWheel)>>7));

				}

				if(newval > paraPhase2.MaxValue)
					newval = paraPhase2.MaxValue;
				if(newval < paraPhase2.MinValue)
					newval = paraPhase2.MinValue;		
				gval.Phase2 = newval;
				break;
			case 4:				// Amp Gain
				if(gval.AmpGain == paraAmpGain.NoValue)
					newval = (ctlval.AmpGain + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.AmpGain = gval.AmpGain;
					newval = (gval.AmpGain + ((ModAmount2 * ModWheel)>>7));
				}

				if(newval > paraAmpGain.MaxValue)
					newval = paraAmpGain.MaxValue;
				if(newval < paraAmpGain.MinValue)
					newval = paraAmpGain.MinValue;

				gval.AmpGain = newval;
				break;
			case 5:				// LFO2amount
				if(gval.LFO2Amount == paraLFO2Amount.NoValue)
					newval = (ctlval.LFO2Amount + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.LFO2Amount = gval.LFO2Amount;
					newval = (gval.LFO2Amount + ((ModAmount2 * ModWheel)>>7));		
				}
				if(newval > paraLFO2Amount.MaxValue)
					newval = paraLFO2Amount.MaxValue;
				if(newval < paraLFO2Amount.MinValue)
					newval = paraLFO2Amount.MinValue;

				gval.LFO2Amount = newval;
				break;
			case 6:				// LFO2freq
				if(gval.LFO2Freq == paraLFO2Freq.NoValue)
					newval = (ctlval.LFO2Freq + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.LFO2Freq = gval.LFO2Freq;
					newval = (gval.LFO2Freq + ((ModAmount2 * ModWheel)>>7));			
				}

				if(newval > paraLFO2Freq.MaxValue)
					newval = paraLFO2Freq.MaxValue;
				if(newval < paraLFO2Freq.MinValue)
					newval = paraLFO2Freq.MinValue;

				gval.LFO2Freq = newval;
				break;
			case 7:				// PW2
				if(gval.PulseWidth2 == paraPulseWidth2.NoValue)
					newval = (ctlval.PulseWidth2 + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.PulseWidth2 = gval.PulseWidth2;
					newval = (gval.PulseWidth2 + ((ModAmount2 * ModWheel)>>7));
				}
				if(newval > paraPulseWidth2.MaxValue)
					newval = paraPulseWidth2.MaxValue;
				if(newval < paraPulseWidth2.MinValue)
					newval = paraPulseWidth2.MinValue;

				gval.PulseWidth2 = newval;
				
				break;
			case 8:				// Filter Attack
				if(gval.FEGAttackTime == paraFEGAttackTime.NoValue)
					newval = (ctlval.FEGAttackTime + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.FEGAttackTime = gval.FEGAttackTime;
					newval = (gval.FEGAttackTime + ((ModAmount2 * ModWheel)>>7));
				}

				if(newval > paraFEGAttackTime.MaxValue)
					newval = paraFEGAttackTime.MaxValue;
				if(newval < paraFEGAttackTime.MinValue)
					newval = paraFEGAttackTime.MinValue;
				gval.FEGAttackTime = newval;
				break;
			case 9:				// Filter Decay
				if(gval.FEGDecayTime == paraFEGDecayTime.NoValue)
					newval = (ctlval.FEGDecayTime + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.FEGDecayTime = gval.FEGDecayTime;
					newval = (gval.FEGDecayTime + ((ModAmount2 * ModWheel)>>7));
				}
				if(newval > paraFEGDecayTime.MaxValue)
					newval = paraFEGDecayTime.MaxValue;
				if(newval < paraFEGDecayTime.MinValue)
					newval = paraFEGDecayTime.MinValue;
				gval.FEGDecayTime = newval;
				break;
			case 10:				// Filter Release
				if(gval.FEGReleaseTime == paraFEGReleaseTime.NoValue)
					newval = (ctlval.FEGReleaseTime + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.FEGReleaseTime = gval.FEGReleaseTime;
					newval = (gval.FEGReleaseTime + ((ModAmount2 * ModWheel)>>7));
				}
				if(newval > paraFEGReleaseTime.MaxValue)
					newval = paraFEGReleaseTime.MaxValue;
				if(newval < paraFEGReleaseTime.MinValue)
					newval = paraFEGReleaseTime.MinValue;
				gval.FEGReleaseTime = newval;

				break;
			case 11:				// Filter Sustain Level
				if(gval.FEGSustainLevel == paraFEGSustainLevel.NoValue)
					newval = (ctlval.FEGSustainLevel + ((ModAmount2 * ModWheel)>>7));
				else
				{
					ctlval.FEGSustainLevel = gval.FEGSustainLevel;
					newval = (gval.FEGSustainLevel + ((ModAmount2 * ModWheel)>>7));
				}	
				if(newval > paraFEGSustainLevel.MaxValue)
					newval = paraFEGSustainLevel.MaxValue;
				if(newval < paraFEGSustainLevel.MinValue)
					newval = paraFEGSustainLevel.MinValue;
				gval.FEGSustainTime = newval;
				break;

			}
		}

        // Filter
        if( gval.FilterType != paraFilterType.NoValue)
		{
			ctlval.FilterType = gval.FilterType;
			phat_philter = 0;

                if( gval.FilterType == 0){ //LP24
                        db18 = false;
                        db24 = true;
                        coefsTabOffs = coefsTab;
                }
                else {
                        if( gval.FilterType == 1){ //LP24
                                db18 = true;
                                db24 = true;
                                coefsTabOffs = coefsTab;
                        }
						else if( gval.FilterType == 6)
						{
							db18 = true;
							db24 = true;
							peak = true;
							coefsTabOffs = coefsTab + (int)2*128*128*8;
						}
						else if( gval.FilterType == 7)
						{
							db18 = true;
							db24 = true;
							peak = false;
							coefsTabOffs = coefsTab + (int)2*128*128*8;
						}
						else if( gval.FilterType == 8)
						{
							db18 = true;
							db24 = true;
							peak = false;
							coefsTabOffs = coefsTab + (int)128*128*8;
						}
						else if( gval.FilterType == 9)
						{
							phat_philter = 1;
							db24 = false;
							
						}
						else if( gval.FilterType == 10)
						{
							phat_philter = 2;
							db24 = false;

							
						}
						else if( gval.FilterType == 11)
						{
							phat_philter = 1;
							db24 = true;


							
						}							 
						else if( gval.FilterType == 12)
						{
							phat_philter = 2;
							db24 = true;					

						}
						else if( gval.FilterType == 13)
						{
							phat_philter = 3;
							db24 = true;
						}					
                        else {						
                                db18 = false;
                                db24 = false;
                                coefsTabOffs = coefsTab + (int)(gval.FilterType-2)*128*128*8;
                        }
                }
		}

		// DIST
		if( gval.Dist != paraDist.NoValue)
		{
			ctlval.Dist = gval.Dist;
			Dist = gval.Dist;
		}

		if( gval.AmpGain != paraAmpGain.NoValue)
		{
			if(!(mod == 1 && ModDest2 == 4))
			ctlval.AmpGain = gval.AmpGain;
			AmpGain = gval.AmpGain;
		}


		// FILTER settings
        if( gval.Cutoff != paraCutoff.NoValue)
		{
			if(!(mod == 1 && ModDest1 == 2))
				ctlval.Cutoff = gval.Cutoff;

			if(aval.Inertia == 0)
			{
				Cutoff = (float)gval.Cutoff;
				CutoffTarget = Cutoff;
				CutoffAdd = 0;


			}
			else
			{
                CutoffTarget = (float)gval.Cutoff;
				CutoffAdd = (CutoffTarget - Cutoff)/(aval.Inertia*pMasterInfo->SamplesPerTick * 0.5);
				if(CutoffAdd > 40.0)
					CutoffAdd = 40.0;
				if(CutoffAdd < -40.0)
					CutoffAdd = -40.0;
			}
		}
        if( gval.Resonance != paraResonance.NoValue)
		{
			if(!(mod == 1 && ModDest2 == 2))		// mod wheel didn't modify resonance
				ctlval.Resonance = gval.Resonance;

            Resonance = gval.Resonance;

		}										   

        // FEG
        if( gval.FEGAttackTime != paraFEGAttackTime.NoValue) {		
			if(!(mod == true && ModDest2 == 8))
				ctlval.FEGAttackTime = gval.FEGAttackTime;
                FEGAttackTime = MSToSamples( scalEnvTime( gval.FEGAttackTime));
		}
        if( gval.FEGDecayTime != paraFEGDecayTime.NoValue) {
			if(!(mod == 1 && ModDest2 == 9))
				ctlval.FEGDecayTime = gval.FEGDecayTime;
                FEGDecayTime = MSToSamples( scalEnvTime( gval.FEGDecayTime));
		}
        if( gval.FEGSustainTime != paraFEGSustainTime.NoValue) {
				ctlval.FEGSustainTime = gval.FEGSustainTime;

				if(gval.FEGSustainTime == 128)		// infinite note (for midi keyboards)
				{
					FEGSustainTime = 0xffffffff;		// maximum unsigned int
				}
				else
					FEGSustainTime = MSToSamples( scalEnvTime( gval.FEGSustainTime));
		}
        if( gval.FEGSustainLevel != paraFEGSustainLevel.NoValue) {
			if(!(mod == 1 && ModDest2 == 11))
				ctlval.FEGSustainLevel = gval.FEGSustainLevel;
                FEGSustainFrac = (float)gval.FEGSustainLevel/127.0;
				FEGSustainLevel = FEGSustainFrac*FEnvMod;
		}
        if( gval.FEGReleaseTime != paraFEGReleaseTime.NoValue) {
			if(!(mod == 1 && ModDest2 == 10))
				ctlval.FEGReleaseTime = gval.FEGReleaseTime;
                FEGReleaseTime = MSToSamples( scalEnvTime( gval.FEGReleaseTime));
		}
        if( gval.FEnvMod != paraFEnvMod.NoValue) {
			if(!(mod == 1 && ModDest1 == 1))
				ctlval.FEnvMod = gval.FEnvMod;

                FEnvMod = (gval.FEnvMod - 0x40)<<1;
				FEGSustainLevel = FEGSustainFrac*FEnvMod;
		}

        if( gval.UEGAttackTime != paraUEGAttackTime.NoValue) {
				ctlval.UEGAttackTime = gval.UEGAttackTime;
                UEGAttackTime = MSToSamples( scalEnvTime( gval.UEGAttackTime));
		}
        if( gval.UEGDecayTime != paraUEGDecayTime.NoValue) {
				ctlval.UEGDecayTime = gval.UEGDecayTime;
                UEGDecayTime = MSToSamples( scalEnvTime( gval.UEGDecayTime));
		}
        if( gval.UEGSustainTime != paraUEGSustainTime.NoValue) {
				ctlval.UEGSustainTime = gval.UEGSustainTime;

				if(gval.UEGSustainTime == 128)		// infinite note (for midi keyboards)
				{
					UEGSustainTime = 0xffffffff;
				}
				else
					UEGSustainTime = MSToSamples( scalEnvTime( gval.UEGSustainTime));
		}
        if( gval.UEGSustainLevel != paraUEGSustainLevel.NoValue) {
				ctlval.UEGSustainLevel = gval.UEGSustainLevel;
                UEGSustainFrac = (float)gval.UEGSustainLevel/127.0;
				UEGSustainLevel = f2i(UEnvMod*UEGSustainFrac);

		}
        if( gval.UEGReleaseTime != paraUEGReleaseTime.NoValue) {
				ctlval.UEGReleaseTime = gval.UEGReleaseTime;
                UEGReleaseTime = MSToSamples( scalEnvTime( gval.UEGReleaseTime));
		}
        if( gval.UEnvMod != paraUEnvMod.NoValue) {
			if(!(mod == 1 && ModDest2 == 1))
				ctlval.UEnvMod = gval.UEnvMod;
                UEnvMod = (gval.UEnvMod - 0x40)<<20;
				UEGSustainLevel = f2i(UEnvMod*UEGSustainFrac);
		}


        if( gval.UEGDest != paraUEGDest.NoValue)
		{
				ctlval.UEGDest = gval.UEGDest;
				UEGDest = gval.UEGDest;
		}

		if( gval.PitchBendAmt != paraPitchBendAmt.NoValue) {
				ctlval.PitchBendAmt = gval.PitchBendAmt;
				PitchBendAmt = gval.PitchBendAmt;
		}

		if( gval.PitchWheel != paraPitchWheel.NoValue) {
				ctlval.PitchWheel = gval.PitchWheel;		
				PitchMod = (float)(gval.PitchWheel - 64)/64.0;

				if(GlideTime)
				{							
					
					BendTime = GlideTime/32;			// FIXME: Ouch does this look fucking slow
					BendGlide = pow( pow( NOTECONST, PitchBendAmt*PitchMod)/BendFactor, 32.0/GlideTime);
						//BendFactor = pow( 1.05946309436, PitchBendAmt*PitchMod);
					PitchBendActive = true;
				}
				else
				{
					BendTime = 0;
					if(PitchMod !=  0.0)
					{
						BendFactor = pow( NOTECONST, PitchBendAmt*PitchMod);
						PitchBendActive = true;
					}
					else
					{
						BendFactor = 1.0;
						PitchBendActive = false;
					}

				}

		}

/*		FIXME: add pitch shift stuff here?
        if( gval.PEnvMod != paraPEnvMod.NoValue) {
                if( gval.PEnvMod - 0x40 != 0)
                        PitchMod = true;
                else {
                        PitchMod = false;
                        for( int i=0; i<numTracks; i++)
                                Tracks[i].PitchModActive = false;
                }
                PEnvMod = gval.PEnvMod - 0x40;
        }
*/

        if( gval.Mix != paraMix.NoValue) {
			if(!(mod == 1 && ModDest1 == 3))
				ctlval.Mix = gval.Mix;
                Bal1 = 127-gval.Mix;
                Bal2 = gval.Mix;
        }

        if( gval.Glide != paraGlide.NoValue)
		{
				ctlval.Glide = gval.Glide;
                if( gval.Glide == 0) {
                        Glide = false;
                        for( int i=0; i<numTracks; i++)
                                Tracks[i].GlideActive = false;
                }
                else {
                        Glide = true;
                        GlideTime = gval.Glide*10000000/pMasterInfo->SamplesPerSec;
                }
		}


        if( gval.WavetableOsc != paraWavetableOsc.NoValue)
		{
				ctlval.WavetableOsc = gval.WavetableOsc;
                WaveTableWave = gval.WavetableOsc;
				pWave = NULL;
		}

        if( gval.WaveDetuneSemi != paraWaveDetuneSemi.NoValue)
		{
				ctlval.WaveDetuneSemi = gval.WaveDetuneSemi;
                WaveDetuneSemi = gval.WaveDetuneSemi-64;
		}			

        if( gval.FixedPitch != paraFixedPitch.NoValue)
		{
				ctlval.FixedPitch = gval.FixedPitch;
                WaveFixedPitch = gval.FixedPitch;
		}

        // SubOsc
        if( gval.SubOscWave != paraSubOscWave.NoValue)
		{
				ctlval.SubOscWave = gval.SubOscWave;

				if(gval.SubOscWave <= 5)
				{
					pwavetabsub = pCB->GetOscillatorTable(gval.SubOscWave);
					if(gval.SubOscWave == 0)
						oscwave3 = -1;
					else
						oscwave3 = gval.SubOscWave;
				}
				else
				{
					oscwave3 = -1;	
					pwavetabsub = waves + ((gval.SubOscWave-6)<<11);
				}
		}

        if( gval.SubOscVol != paraSubOscVol.NoValue)
		{
				ctlval.SubOscVol = gval.SubOscVol;
                SubOscVol = gval.SubOscVol;
		}

        // PW
        if( gval.PulseWidth1 != paraPulseWidth1.NoValue)
		{
			if(!(mod == 1 && ModDest1 == 7))
				ctlval.PulseWidth1 = gval.PulseWidth1;
                Center1 = gval.PulseWidth1/127.0;
		}

        if( gval.PulseWidth2 != paraPulseWidth2.NoValue)
		{
			if(!(mod == 1 && ModDest2 == 7))
				ctlval.PulseWidth2 = gval.PulseWidth2;
                Center2 = gval.PulseWidth2/127.0;
		}

        // Detune
        if( gval.DetuneSemi != paraDetuneSemi.NoValue) {
			if(!(mod == 1 && ModDest1 == 8))
				ctlval.DetuneSemi = gval.DetuneSemi;
                DetuneSemi = (float)pow( 1.05946309435929526, gval.DetuneSemi-0x40);
		}
        if( gval.DetuneFine != paraDetuneFine.NoValue) {
			if(!(mod == 1 && ModDest1 == 4))
				ctlval.DetuneFine = gval.DetuneFine;
		        DetuneFine = (float)pow( 1.00091728179580156, gval.DetuneFine-0x40);
		}
        if( gval.Sync != SWITCH_NO)
		{
				ctlval.Sync = gval.Sync;
				Sync = gval.Sync;
		}

        if( gval.Phase2 != paraPhase2.NoValue)
		{
			if(!(mod == 1 && ModDest2 == 3))
				ctlval.Phase2 = gval.Phase2;
                PhaseDiff2 = (int)gval.Phase2<<20;		// 0...2048
		}

        if( gval.Mode != paraMode.NoValue)
		{
				ctlval.Mode = gval.Mode;
                Playmode = gval.Mode;
		}

        if( gval.MixType != paraMixType.NoValue)
		{
				ctlval.MixType = gval.MixType;
                MixType = gval.MixType;
		}

        if( gval.Wave1 != paraWave1.NoValue) { // neuer wert
				ctlval.Wave1 = gval.Wave1;

                if( gval.Wave1 == NUMWAVES+5)
				{
                        noise1 = NOISE1;
						oscwave1 = -1;
				}
                else if( gval.Wave1 == NUMWAVES+6)
				{
                        noise1 = NOISE2;
						oscwave1 = -1;
				}
                else {

                        noise1 = 0;

						if(gval.Wave1 <= 5)
						{
							pwavetab1 = pCB->GetOscillatorTable(gval.Wave1);
							if(gval.Wave1 == 0)
								oscwave1 = -1;
							else
								oscwave1 = gval.Wave1;
						}
						else
						{
							pwavetab1 = waves + ((gval.Wave1-6) << 11);
							oscwave1 = -1;
						}
                }
        }

        if( gval.Wave2 != paraWave2.NoValue)  { // neuer wert
				ctlval.Wave2 = gval.Wave2;

                if( gval.Wave2 == NUMWAVES+5) {
                        noise2 = NOISE1;
						oscwave2 = -1;
				}
                else if( gval.Wave2 == NUMWAVES+6) {
                        noise2 = NOISE2;
						oscwave2 = -1;
				}
                else 
				{
                        noise2 = false;
						if(gval.Wave2 <= 5)
						{
							pwavetab2 = pCB->GetOscillatorTable(gval.Wave2);
							if(gval.Wave2 == 0)
								oscwave2 = -1;
							else
								oscwave2 = gval.Wave2;
						}
						else
						{
							pwavetab2 = waves + ((gval.Wave2-6) << 11);
							oscwave2 = -1;
						}
                }
		}


        // AEG
        if( gval.AEGAttackTime != paraAEGAttackTime.NoValue) {
				ctlval.AEGAttackTime = gval.AEGAttackTime;
                AEGAttackTime = MSToSamples( scalEnvTime( gval.AEGAttackTime));
		}
        if( gval.AEGDecayTime != paraAEGDecayTime.NoValue) {
				ctlval.AEGDecayTime = gval.AEGDecayTime;
                AEGDecayTime = MSToSamples( scalEnvTime( gval.AEGDecayTime));
		}
        if( gval.AEGSustainTime != paraAEGSustainTime.NoValue) {
				ctlval.AEGSustainTime = gval.AEGSustainTime;

				if(gval.AEGSustainTime == 128)		// infinite note (for midi keyboards)
				{
					AEGSustainTime = 0xffffffff;
				}
				else
					AEGSustainTime = MSToSamples( scalEnvTime( gval.AEGSustainTime));
		}
        if( gval.AEGSustainLevel != paraAEGSustainLevel.NoValue) {
				ctlval.AEGSustainLevel = gval.AEGSustainLevel;
                AEGSustainFrac = (float)gval.AEGSustainLevel/127.0;
		}
        if( gval.AEGReleaseTime != paraAEGReleaseTime.NoValue) {
				ctlval.AEGReleaseTime = gval.AEGReleaseTime;
                AEGReleaseTime = MSToSamples( scalEnvTime( gval.AEGReleaseTime));
		}

        // ..........LFO............

		// FIXME: add more lfo waves
		// FIXME: add other LFO parms

        // LFO1
        if( gval.LFO1Dest != paraLFO1Dest.NoValue) {
				ctlval.LFO1Dest = gval.LFO1Dest;
                   LFO_VCF = LFO_Vib = LFO_Osc1 = LFO_PW1 = LFO_Amp = LFO_Cut = false;
                   switch( gval.LFO1Dest) {
//              case 0: ...none
                case 1:
                        LFO_Osc1 = true;
                        break;
                case 2:
                        LFO_PW1 = true;
                        break;
                case 3:
                        LFO_Amp = true;
                        break;
                case 4:
                        LFO_Cut = true;
                        break;

                case 5: // 12
                        LFO_Osc1 = true;
                        LFO_PW1 = true;
                        break;
                case 6: // 13
                        LFO_Osc1 = true;
                        LFO_Amp = true;
                        break;
                case 7: // 14
                                                LFO_Osc1 = true;
                                                LFO_Cut = true;
                        break;
                case 8: // 23
                        LFO_PW1 = true;
                        LFO_Amp = true;
                        break;
                case 9: // 24
                        LFO_PW1 = true;
                        LFO_Cut = true;
                        break;
                case 10: // 34
                        LFO_Amp = true;
                        LFO_Cut = true;
                        break;

                case 11: // 123
                        LFO_Osc1 = true;
                        LFO_PW1 = true;
                        LFO_Amp = true;
                        break;
                case 12: // 124
                        LFO_Osc1 = true;
                        LFO_PW1 = true;
                        LFO_Cut = true;
                        break;
                case 13: // 134
                        LFO_Osc1 = true;
                        LFO_Amp = true;
                        LFO_Cut = true;
                        break;
                case 14: // 234
                        LFO_PW1 = true;
                        LFO_Amp = true;
                        LFO_Cut = true;
                        break;
                case 15: // 1234
                        LFO_Osc1 = true;
                        LFO_PW1 = true;
                        LFO_Amp = true;
                        LFO_Cut = true;
                        break;
                case 16: // Vibrato
						LFO_Vib = true;
                        break;
                case 17: // Cut->AMP
                        LFO_Cut = true;
						LFO_VCF = true;
                        break;
                        }
                }

		// FIXME: Add 3 step wave here
        if( gval.LFO1Wave != paraLFO1Wave.NoValue) {
				ctlval.LFO1Wave = gval.LFO1Wave;
				if(gval.LFO1Wave <= 4)
					pwavetabLFO1 = pCB->GetOscillatorTable( gval.LFO1Wave);
				else if(gval.LFO1Wave == 5)
					pwavetabLFO1 = waves + (WAVE_STEPUP << 11);
				else if(gval.LFO1Wave == 6)
					pwavetabLFO1 = waves + (WAVE_STEPDN << 11);
				else if(gval.LFO1Wave == 7)
					pwavetabLFO1 = waves + (WAVE_WACKY1  << 11);
				else if(gval.LFO1Wave == 8)
					pwavetabLFO1 = waves + (WAVE_WACKY2  << 11);

                if( gval.LFO1Wave == OWF_NOISE)
				{
                        LFO1Noise = true;
				}
                else
				{
                        LFO1Noise = false;
				}
        }


        if( gval.LFO1Freq != paraLFO1Freq.NoValue)
		{
			if(!(mod == 1 && ModDest1 == 6))
				ctlval.LFO1Freq = gval.LFO1Freq;
                if( gval.LFO1Freq>116 && gval.LFO1Freq<=127) {
                        LFO1Synced = true;
						LFO_1Lock2 = false;
                        LFO1Freq = gval.LFO1Freq - 117;
                }
                else if( gval.LFO1Freq==128) {
                        LFO1Synced = false;
						LFO_1Lock2 = true;
						LFO1Freq = 0;
				}
				else
				{
						LFO_1Lock2 = false;
                        LFO1Synced = false;
                        LFO1Freq = gval.LFO1Freq;
                }
		}

        if( gval.LFO1Amount != paraLFO1Amount.NoValue)
		{
			if(!(mod == 1 && ModDest1 == 5))
				ctlval.LFO1Amount = gval.LFO1Amount;
                LFO1Amount = gval.LFO1Amount;
		}


        if( LFO1Synced)
                if( LFO1Noise) // sample & hold
                        PhaseAddLFO1 = (int)(0x200000/(pMasterInfo->SamplesPerTick<<LFO1Freq));
                else
                        PhaseAddLFO1 = (int)((double)0x200000*2048/(pMasterInfo->SamplesPerTick<<LFO1Freq));
        else
                if( LFO1Noise) // sample & hold
                        PhaseAddLFO1 = (int)(scalLFOFreq( LFO1Freq)/pMasterInfo->SamplesPerSec*0x200000);
                else
                        PhaseAddLFO1 = (int)(scalLFOFreq( LFO1Freq)*TabSizeDivSampleFreq*0x200000);

        // LFO2
                if( gval.LFO2Dest != paraLFO2Dest.NoValue) {
						ctlval.LFO2Dest = gval.LFO2Dest;
                        LFO_2Lock1 = LFO_2Lock2 = LFO_LFO1 = LFO_Osc2 = LFO_PW2 = LFO_Mix = LFO_Phase2 = LFO_Reso = false;

                        switch( gval.LFO2Dest) {
//              case 0: ...none
                case 1:
                        LFO_Osc2 = true;
                        break;
                case 2:
                        LFO_PW2 = true;
                        break;
                case 3:
                        LFO_Mix = true;
                        break;
                case 4:
                        LFO_Reso = true;
                        break;
                case 5: // 12
                        LFO_Osc2 = true;
                        LFO_PW2 = true;
                        break;
                case 6: // 13
                        LFO_Osc2 = true;
                        LFO_Mix = true;
                        break;
                case 7: // 14
                        LFO_Osc2 = true;
                        LFO_Reso = true;
                        break;
                case 8: // 23
                        LFO_PW2 = true;
                        LFO_Mix = true;
                        break;
                case 9: // 24
                        LFO_PW2 = true;
                        LFO_Reso = true;
                        break;
                case 10: // 34
                        LFO_Mix = true;
                        LFO_Reso = true;
                        break;
                case 11: // 123
                        LFO_Osc2 = true;
                        LFO_PW2 = true;
                        LFO_Mix = true;
                        break;
                case 12: // 124
                        LFO_Osc2 = true;
                        LFO_PW2 = true;
                        LFO_Reso = true;
                        break;
                case 13: // 134
                        LFO_Osc2 = true;
                        LFO_Mix = true;
                        LFO_Reso = true;
                        break;
                case 14: // 234
                        LFO_PW2 = true;
                        LFO_Mix = true;
                        LFO_Reso = true;
                        break;
                case 15: // 1234
                        LFO_Osc2 = true;
                        LFO_PW2 = true;
                        LFO_Mix = true;
                        LFO_Reso = true;
                        break;
                case 16:
						LFO_Phase2 = true;
						break;
                case 17:
						LFO_LFO1 = true;
						break;

                }
        }

        if( gval.LFO2Wave != paraLFO2Wave.NoValue) {
				ctlval.LFO2Wave = gval.LFO2Wave;
				if(gval.LFO2Wave <= 4)
	                pwavetabLFO2 = pCB->GetOscillatorTable( gval.LFO2Wave);
				else if(gval.LFO2Wave == 5)
					pwavetabLFO2 = waves + (WAVE_STEPUP << 11);
				else if(gval.LFO2Wave == 6)
					pwavetabLFO2 = waves + (WAVE_STEPDN << 11);
				else if(gval.LFO2Wave == 7)
					pwavetabLFO2 = waves + (WAVE_WACKY1 << 11);
				else if(gval.LFO2Wave == 8)
					pwavetabLFO2 = waves + (WAVE_WACKY2 << 11);
                if( gval.LFO2Wave == OWF_NOISE)
                        LFO2Noise = true;
                else
                        LFO2Noise = false;
        }

        if( gval.LFO2Freq != paraLFO2Freq.NoValue)
		{
			if(!(mod == 1 && ModDest2 == 6))
				ctlval.LFO2Freq = gval.LFO2Freq;
                if( gval.LFO2Freq>116 && gval.LFO2Freq<=127) {
						LFO_2Lock2 = false;
						LFO_2Lock1 = false;
                        LFO2Synced = true;
                        LFO2Freq = gval.LFO2Freq - 117;
                }
                else if( gval.LFO2Freq==128) {
                        LFO2Synced = false;
						LFO_2Lock1 = false;
						LFO_2Lock2 = true;
						LFO2Freq = 0;
				}
                else if( gval.LFO2Freq==129) {

                        LFO2Synced = false;
						LFO_2Lock1 = true;
						LFO_2Lock2 = false;
						LFO2Freq = 0;
				}
                else {
						LFO_2Lock2 = false;
						LFO_2Lock1 = false;
                        LFO2Synced = false;
                        LFO2Freq = gval.LFO2Freq;
                }
		}

        if( gval.LFO2Amount != paraLFO2Amount.NoValue)
		{
			if(!(mod == 1 && ModDest2 == 5))
				ctlval.LFO2Amount = gval.LFO2Amount;
                LFO2Amount = gval.LFO2Amount;
		}

        // LFO-Phasen-Differenzen
        if( gval.LFO1PhaseDiff != paraLFO1PhaseDiff.NoValue)
		{
				ctlval.LFO1PhaseDiff = gval.LFO1PhaseDiff;
                LFO1PhaseDiff = gval.LFO1PhaseDiff << (9+16);
		}
        if( gval.LFO2PhaseDiff != paraLFO2PhaseDiff.NoValue)
		{
				ctlval.LFO2PhaseDiff = gval.LFO2PhaseDiff;
                LFO2PhaseDiff = gval.LFO2PhaseDiff << (9+16);
		}


        if( LFO2Synced)
                if( LFO2Noise) // sample & hold
                        PhaseAddLFO2 = (int)(0x200000/(pMasterInfo->SamplesPerTick<<LFO2Freq));
                else
                        PhaseAddLFO2 = (int)((double)0x200000*2048/(pMasterInfo->SamplesPerTick<<LFO2Freq));
        else
                if( LFO2Noise) // sample & hold
                        PhaseAddLFO2 = (int)(scalLFOFreq( LFO2Freq)/pMasterInfo->SamplesPerSec*0x200000);
                else
                        PhaseAddLFO2 = (int)(scalLFOFreq( LFO2Freq)*TabSizeDivSampleFreq*0x200000);

        // skalierte LFO-Amounts

        // TrackParams durchgehen
        for (int i=0; i<numTracks; i++)
		{
            Tracks[i].Tick( tval[i]);
			Tracks[i].UpdateLFO1Amounts(LFO1Amount);
			Tracks[i].UpdateLFO2Amounts(LFO2Amount);
		}
}


bool mi::Work(float *psamples, int numsamples, int const)
{
        bool gotsomething = false;

		if(WaveTableWave)
			pWave = pCB->GetWave(WaveTableWave);
		else
			pWave = NULL;

		OldCutoff = Cutoff;

        for ( int i=0; i<numTracks; i++) {
                if ( Tracks[i].AEGState) {
                        Tracks[i].PhLFO1 = PhaseLFO1 + i*LFO1PhaseDiff;
                        Tracks[i].PhLFO2 = PhaseLFO2 + i*LFO2PhaseDiff;

						Cutoff = OldCutoff;		// FIXME: This is wasteful

                        if ( !gotsomething) {
                                Tracks[i].Work( psamples, numsamples);
                                gotsomething = true;
                        }
                        else {
                                float *paux = pCB->GetAuxBuffer();
                                Tracks[i].Work( paux, numsamples);
                                DSP_Add(psamples, paux, numsamples);
                        }
                }
        }

		if(BendFactor == 1.0)
			PitchBendActive = false;
		
		if(BendTime)
		{
			BendFactor *= BendGlide;
			BendTime--;
		}

        PhaseLFO1 += PhaseAddLFO1*numsamples;
        PhaseLFO2 += PhaseAddLFO2*numsamples;

		if(!gotsomething)	// no
		{
			Cutoff += CutoffAdd*numsamples;

			if(CutoffAdd > 0.0 && Cutoff > CutoffTarget)
			{
					Cutoff = CutoffTarget;
					//pmi->CutoffAdd = 0;			
			}
			else if(CutoffAdd < 0.0 && Cutoff < CutoffTarget)
			{
					Cutoff = CutoffTarget;
					//pmi->CutoffAdd = 0;
			}
		}

        return gotsomething;
}


void mi::Stop()
{
        for( int i=0; i<numTracks; i++)
                Tracks[i].Stop();
}

void mi::ComputeCoefs( float *coefs, int freq, int r, int t)
{
        float omega = 2*PI*scalCutoff(freq)/pMasterInfo->SamplesPerSec;
    float sn = (float)sin( omega);
    float cs = (float)cos( omega);
    float alpha;
        if( t<2)
                alpha = (float)(sn / scalResonance( r *(freq+70)/(127.0+70)));
        else
                alpha = (float)(sn * sinh( scalBandwidth( r) * omega/sn));

        float a0, a1, a2, b0, b1, b2;

        switch( t) {
        case 0: // LP
                b0 =  (1 - cs)/2;
                b1 =   1 - cs;
                b2 =  (1 - cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 1: // HP
                b0 =  (1 + cs)/2;
                b1 = -(1 + cs);
                b2 =  (1 + cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 2: // BP
                b0 =   alpha;
                b1 =   0;
                b2 =  -alpha;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 3: // BR
                b0 =   1;
                b1 =  -2*cs;
                b2 =   1;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        default:
                return;
        }

        coefs[0] = b0/a0;
        coefs[1] = b1/a0;
        coefs[2] = b2/a0;
        coefs[3] = -a1/a0;
        coefs[4] = -a2/a0;
}
