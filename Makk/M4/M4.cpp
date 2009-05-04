// M4 Buzz plugin by MAKK makk@gmx.de
// released in July 1999
// formulas for the filters by Robert Bristow-Johnson pbjrbj@viconet.com
// a.k.a. robert@audioheads.com

#define NUMWAVES 126

#include <windef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include <dsplib.h>


#pragma optimize ("a", on)

#define MAX_TRACKS                              8

#define EGS_NONE                                0
#define EGS_ATTACK                              1
#define EGS_SUSTAIN                             2
#define EGS_RELEASE                             3

float *coefsTab = new float [4*128*128*8];
float *LFOOscTab = new float [0x10000];


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

CMachineParameter const paraWave1 =
{
        pt_byte,                                                                                // type
        "Osc1 Wave",
        "Oscillator 1 Waveform",                                                // description
        0,                                                                                              // Min
        NUMWAVES,                                                                      // Max
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
        NUMWAVES,                                                                                      // Max
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
        "Oscillator Mix (Osc1 <-> Osc2) ... 00h=Osc1  7Fh=Osc2",
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
        7,                                                                                              // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};



CMachineParameter const paraSync =
{
        pt_switch,                                                                              // type
        "Osc2 Sync",
        "Oscillator 2 Sync: Oscillator 2 synced by Oscillator 1 ... 0=off  1=on",                                    // description
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
        "Oscillator 2 Semi Detune in Halfnotes ... 40h=no detune",                                             // description
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
        "Oscillator 2 Fine Detune ... 40h=no detune",
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
        "Pitch Glide ... 00h=no Glide  7Fh=maximum Glide",                                                                                // description
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
        NUMWAVES-1,                                                                                      // Max
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

CMachineParameter const paraAEGSustainTime =
{
        pt_byte,                                                                                // type
        "Amp Sustain",
        "Amplitude Envelope Sustain Time",                                               // description
        0,                                                                                              // Min
        127,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x10                                                                                              // default
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
        "Filter Type ... 0=LowPass24  1=LowPass18  2=LowPass12  3=HighPass  4=BandPass  5=BandReject",                  // description
        0,                                                                                              // Min
        5,                                                                              // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        2 // LP12                                                                                               // default
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

CMachineParameter const paraPEGAttackTime =
{
        pt_byte,                                                                                // type
        "Pitch Attack",
        "Pitch Envelope Attack Time",                                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        7                                                                                               // default
};


CMachineParameter const paraPEGDecayTime =
{
        pt_byte,                                                                                // type
        "Pitch Decay",
        "Pitch Envelope Decay Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0b                                                                                               // default
};


CMachineParameter const paraPEnvMod =
{
        pt_byte,                                                                                // type
        "Pitch EnvMod",
        "Pitch Envelope Modulation",                                    // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40+32                                                                                            // default
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

CMachineParameter const paraFEGSustainTime =
{
        pt_byte,                                                                                // type
        "Filter Sustain",
        "Filter Envelope Sustain Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x0e                                                                                               // default
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
        15,                                                                                     // Max
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
        4,                                                                                      // Max
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
        127,                                                                                    // Max
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
        15,                                                                                     // Max
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
        4,                                                                                      // Max
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
        127,                                                                                    // Max
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
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1PhaseDiff =
{
        pt_byte,                                                                                // type
        "LFO1 Ph Diff",
        "Low Frequency Oscillator 1 Phase Difference: 00h=0�  40h=180�  7Fh=357�",
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
        "Low Frequency Oscillator 2 Phase Difference: 00h=0�  40h=180�  7Fh=357�",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40
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


CMachineAttribute const *pAttributes[] =
{
        &attrLFO1ScaleOsc1,
        &attrLFO1ScalePW1,
        &attrLFO1ScaleVolume,
        &attrLFO1ScaleCutoff,
        &attrLFO2ScaleOsc2,
        &attrLFO2ScalePW2,
        &attrLFO2ScaleMix,
        &attrLFO2ScaleReso
};



CMachineParameter const *pParameters[] = {
                &paraWave1,
        &paraPulseWidth1,
        &paraWave2,
        &paraPulseWidth2,
        &paraDetuneSemi,
        &paraDetuneFine,
        &paraSync,
        &paraMixType,
        &paraMix,
        &paraSubOscWave,
        &paraSubOscVol,

        &paraPEGAttackTime,
        &paraPEGDecayTime,
        &paraPEnvMod,
        &paraGlide,

        &paraAEGAttackTime,
        &paraAEGSustainTime,
        &paraAEGReleaseTime,

        &paraFilterType,
        &paraCutoff,
        &paraResonance,
        &paraFEGAttackTime,
        &paraFEGSustainTime,
        &paraFEGReleaseTime,
        &paraFEnvMod,

        // LFO 1
        &paraLFO1Dest,
        &paraLFO1Wave,
        &paraLFO1Freq,
        &paraLFO1Amount,
                &paraLFO1PhaseDiff,
        // LFO 2
        &paraLFO2Dest,
        &paraLFO2Wave,
        &paraLFO2Freq,
        &paraLFO2Amount,
                &paraLFO2PhaseDiff,

        &paraNote,
        &paraVolume,
};

#pragma pack(1)


class gvals
{
public:
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
        byte PEGAttackTime;
        byte PEGDecayTime;
        byte PEnvMod;
        byte Glide;

        byte AEGAttackTime;
        byte AEGSustainTime;
        byte AEGReleaseTime;

        byte FilterType;
        byte Cutoff;
        byte Resonance;
        byte FEGAttackTime;
        byte FEGSustainTime;
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
};



#pragma pack()

CMachineInfo const MacInfo =
{
        MT_GENERATOR,                                                   // type
        MI_VERSION,
        0,                                                                              // flags
        1,                                                                              // min tracks
        MAX_TRACKS,                                                    // max tracks
        35,                                                            // numGlobalParameters
        2,                                                             // numTrackParameters
        pParameters,
        8,
        pAttributes,
#ifdef _DEBUG
        "M4 by Makk (Debug build)",                     // name
#else
        "M4 by Makk",
#endif
        "M4",                                                                   // short name
        "Makk",                                                                 // author
        NULL
};


class mi;

class CTrack
{
public:
        void Tick(tvals const &tv);
        void Stop();
        void Init();
        void Work(float *psamples, int numsamples);
        inline int Osc();
        inline float VCA();
        inline float Filter( float x);
        void NewPhases();
        int MSToSamples(double const ms);

public:

        // ......Osc......
        int Phase1, Phase2, PhaseSub;
        int Ph1, Ph2;
        float center1, center2;
                int c1, c2;
        float PhScale1A, PhScale1B;
        float PhScale2A, PhScale2B;
        int PhaseAdd1, PhaseAdd2;
        float Frequency, FrequencyFrom;
                // Glide
        bool GlideActive;
        float GlideMul, GlideFactor;
        int GlideCount;
        // PitchEnvMod
        bool PitchModActive;
        // PEG ... AD-H�llkurve
        int PEGState;
        int PEGCount;
        float PitchMul, PitchFactor;
        // random generator... rauschen
        short r1, r2, r3, r4;

        float OldOut; // gegen extreme Knackser/Wertespr�nge

        // .........AEG........ ASR-H�llkurve
        int AEGState;
        int AEGCount;
        int Volume;
        int Amp;
        int AmpAdd;

        // ........Filter..........
        float x1, x2, y1, y2;
        float x241, x242, y241, y242;
        int FEGState;
        int FEGCount;
        float Cut;
        float CutAdd;

        // .........LFOs...........
        int PhLFO1, PhLFO2;

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
        virtual char const *DescribeValue(int const param, int const value);
        void ComputeCoefs( float *coefs, int f, int r, int t);
        // skalefuncs
        inline float scalLFOFreq( int v);
        inline float scalEnvTime( int v);
        inline float scalCutoff( int v);
        inline float scalResonance( float v);
        inline float scalBandwidth( int v);
        inline int MSToSamples(double const ms);
public:

        // OSC
        bool noise1, noise2;
        int SubOscVol;
        float Center1, Center2;
        const short *pwavetab1, *pwavetab2, *pwavetabsub;

        // Filter
        float *coefsTabOffs; // abh�ngig vom FilterTyp
        int Cutoff, Resonance;
        bool db24, db18;
        // PEG
        int PEGAttackTime;
        int PEGDecayTime;
        int PEnvMod;
        bool PitchMod;
        // AEG
        int AEGAttackTime;
        int AEGSustainTime;
        int AEGReleaseTime;
        // FEG
        int FEGAttackTime;
        int FEGSustainTime;
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
        int LFO1AmountOsc1;
        float LFO1AmountPW1;
        int LFO1AmountVolume;
        int LFO1AmountCutoff;
        int LFO2AmountOsc2;
        float LFO2AmountPW2;
        int LFO2AmountMix;
        int LFO2AmountReso;

        float TabSizeDivSampleFreq;
        int numTracks;
        CTrack Tracks[MAX_TRACKS];

        // LFO
        // 1
        bool LFO_Osc1;
        bool LFO_PW1;
        bool LFO_Amp;
        bool LFO_Cut;
        // 2
        bool LFO_Osc2;
        bool LFO_PW2;
        bool LFO_Mix;
    bool LFO_Reso;
        // OscMix
        int Bal1, Bal2;
        int MixType;

        avals aval; // attributes
        gvals gval; // globals
        tvals tval[MAX_TRACKS]; // track-vals


};


DLL_EXPORTS

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
        AEGState = EGS_NONE;
}

void CTrack::Init()
{
        AEGState = EGS_NONE;
                FEGState = EGS_NONE;
                PEGState = EGS_NONE;
        r1=26474; r2=13075; r3=18376; r4=31291; // randomGenerator
        Phase1 = Phase2 = Ph1 = Ph2 = PhaseSub = 0; // Osc starten neu
        x1 = x2 = y1 = y2 = 0; //Filter
        x241 = x242 = y241 = y242 = 0; //Filter
        OldOut = 0;
                Amp = 0;
                AEGCount = -1;
                FEGCount = -1;
                PEGCount = -1;
                center1 = pmi->Center1;
                center2 = pmi->Center2;
                PhScale1A = 0.5/center1;
                PhScale1B = 0.5/(1-center1);
                PhScale2A = 0.5/center2;
                PhScale2B = 0.5/(1-center2);
                c1 = center1*0x8000000;
                c2 = center2*0x8000000;
                GlideActive = false;
                PitchModActive = false;
                Volume = paraVolume.DefValue << 20;

}

void CTrack::Tick( tvals const &tv)
{
        if( tv.Volume != paraVolume.NoValue)
                Volume = tv.Volume << 20;

        if( tv.Note != paraNote.NoValue) { // neuer wert
                if( (tv.Note >= NOTE_MIN) && (tv.Note <= NOTE_MAX)) { // neue note gesetzt
                        FrequencyFrom = Frequency;
                        Frequency = (float)(16.3516*pow(2,((tv.Note>>4)*12+(tv.Note&0x0f)-1)/12.0));

                        if( pmi->Glide) {
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

                        // trigger envelopes neu an...
                        // Amp
                        AEGState = EGS_ATTACK;
                        AEGCount = pmi->AEGAttackTime;
                        AmpAdd = Volume/pmi->AEGAttackTime;
                        Amp = 0; //AmpAdd; // fange bei 0 an
                        // Pitch
                        if( pmi->PitchMod) {
                                PitchModActive = true;
                                PEGState = EGS_ATTACK;
                                PEGCount = pmi->PEGAttackTime;
                                PitchMul = (float)pow( pow( 1.01, pmi->PEnvMod), 1.0/pmi->PEGAttackTime);
                                PitchFactor = 1.0;
                        }
                        else
                                PitchModActive = false;

                        // Filter
                        FEGState = EGS_ATTACK;
                        FEGCount = pmi->FEGAttackTime;
                        CutAdd = ((float)pmi->FEnvMod)/pmi->FEGAttackTime;
                        Cut = 0.0; // fange bei 0 an


                } else
                        if( tv.Note == NOTE_OFF)
                                AEGState = EGS_NONE; // note aus

        }

        if( GlideActive) {
                PhaseAdd1 = (int)(FrequencyFrom*pmi->TabSizeDivSampleFreq*0x10000);
                PhaseAdd2 = (int)(FrequencyFrom*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
        }
        else {
                PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
                PhaseAdd2 = (int)(Frequency*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
        }

}


inline int CTrack::Osc()
{
        int o, o2;
        int B1, B2;

        if( pmi->LFO_Mix) { // LFO-MIX
                B2 = pmi->Bal2 + ((pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*pmi->LFO2AmountMix)>>15);
                        if( B2<0)
                                B2 = 0;
                        else
                                if( B2>127)
                                        B2 = 127;
                        B1 = 127-B2;

                        // osc1
                        if( pmi->noise1) {
                                short t = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=t;
                                o = (t*B1)>>7;
                        }
                        else
                                o = (pmi->pwavetab1[(unsigned)Ph1>>16]*B1)>>7;

                        // osc2
                        if( pmi->noise2) {
                                short u = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=u;
                                o2 = (u*B2)>>7;
                        }
                        else
                                o2 = (pmi->pwavetab2[(unsigned)Ph2>>16]*B2)>>7;
                }
                else { // kein LFO
                        // osc1
                        if( pmi->noise1) {
                                short t = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=t;
                                o = (t*pmi->Bal1)>>7;
                        }
                        else
                                o = (pmi->pwavetab1[(unsigned)Ph1>>16]*pmi->Bal1)>>7;

                        // osc2
                        if( pmi->noise2) {
                                short u = r1+r2+r3+r4;
                                r1=r2; r2=r3; r3=r4; r4=u;
                                o2 = (u*pmi->Bal2)>>7;
                        }
                        else
                                o2 = (pmi->pwavetab2[(unsigned)Ph2>>16]*pmi->Bal2)>>7;

                }

        // PhaseDependentMixing

                switch( pmi->MixType)
        {
        case 0: //ADD
                o += o2;
                break;
        case 1: // ABS
                o = (abs(o-o2)<<1)-0x8000;
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
        }
        return o + ((pmi->pwavetabsub[PhaseSub>>16]*pmi->SubOscVol)>>7);
}

inline float CTrack::VCA()
{
        // EG...
        if( !AEGCount--)
                switch( ++AEGState)
                {
                case EGS_SUSTAIN:
                        AEGCount = pmi->AEGSustainTime;
                        Amp = Volume;
                        AmpAdd = 0;
                        break;
                case EGS_RELEASE:
                        AEGCount = pmi->AEGReleaseTime;
                        AmpAdd = -Volume/pmi->AEGReleaseTime;
                        break;
                case EGS_RELEASE + 1:
                        AEGState = EGS_NONE;
                        AEGCount = -1;
                        Amp = 0;
                        break;
                }

        Amp +=AmpAdd;

        if( pmi->LFO_Amp) {
                float a =
                  Amp + ((pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*pmi->LFO1AmountVolume)<<5);
                if( a<0)
                        a = 0;
                return( a*(1.0/0x8000000));
        }
        else
                return Amp*(1.0/0x8000000);
}


inline float CTrack::Filter( float x)
{
        float y;

        // Envelope
        if( FEGState) {
                if( !FEGCount--)
                        switch( ++FEGState)
                        {
                        case EGS_SUSTAIN:
                                FEGCount = pmi->FEGSustainTime;
                                Cut = (float)pmi->FEnvMod;
                                CutAdd = 0.0;
                                break;
                        case EGS_RELEASE:
                                FEGCount = pmi->FEGReleaseTime;
                                CutAdd = ((float)-pmi->FEnvMod)/pmi->FEGReleaseTime;
                                break;
                        case EGS_RELEASE + 1:
                                FEGState = EGS_NONE; // false
                                FEGCount = -1;
                                Cut = 0.0;
                                CutAdd = 0.0;
                                break;
                        }
                Cut += CutAdd;
        }

        // LFO
        // Cut
        int c, r;
        if( pmi->LFO_Cut)
                c = pmi->Cutoff + Cut + // Cut = EnvMod
                ((pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*pmi->LFO1AmountCutoff)>>(7+8));
        else
                c = pmi->Cutoff + Cut; // Cut = EnvMod
        if( c < 0)
                c = 0;
        else
                if( c > 127)
                        c = 127;
        // Reso
        if( pmi->LFO_Reso) {
                r = pmi->Resonance +
                ((pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*pmi->LFO2AmountReso)>>(7+8));
        if( r < 0)
                r = 0;
        else
                if( r > 127)
                        r = 127;
        }
        else
                r = pmi->Resonance;


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
                        return y;
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
                                return y24;
                        else
                                return (y+y24)*0.5;
                }
}

inline void CTrack::NewPhases()
{
        if( PitchModActive) {
                if( GlideActive) {
                        if( pmi->LFO_Osc1) {
                                float pf = LFOOscTab[(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*pmi->LFO1AmountOsc1>>7) + 0x8000];
                                Phase1 += PhaseAdd1*GlideFactor*PitchFactor*pf;
                                PhaseSub += (PhaseAdd1>>1)*GlideFactor*PitchFactor*pf;
                        }
                        else {
                                Phase1 += PhaseAdd1*GlideFactor*PitchFactor;
                                PhaseSub += (PhaseAdd1>>1)*GlideFactor*PitchFactor;
                        }
                        if( pmi->LFO_Osc2)
                                Phase2 += PhaseAdd2*GlideFactor*PitchFactor
                                  *LFOOscTab[(pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*pmi->LFO2AmountOsc2>>7) + 0x8000];
                        else
                                Phase2 += PhaseAdd2*GlideFactor*PitchFactor;
                        GlideFactor *= GlideMul;
                        if( !GlideCount--) {
                                GlideActive = false;
                                PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
                                PhaseAdd2 = (int)(Frequency*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
                        }
                }
                else { // kein Glide
                        if( pmi->LFO_Osc1) {
                                float pf = LFOOscTab[(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*pmi->LFO1AmountOsc1>>7) + 0x8000];
                                Phase1 += PhaseAdd1*PitchFactor*pf;
                                PhaseSub += (PhaseAdd1>>1)*PitchFactor*pf;
                        }
                        else {
                                Phase1 += PhaseAdd1*PitchFactor;
                                PhaseSub += (PhaseAdd1>>1)*PitchFactor;
                        }
                        if( pmi->LFO_Osc2)
                                Phase2 += PhaseAdd2*PitchFactor
                                  *LFOOscTab[(pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*pmi->LFO2AmountOsc2>>7) + 0x8000];
                        else
                                Phase2 += PhaseAdd2*PitchFactor;
                }

                PitchFactor *= PitchMul;

                if( !PEGCount--) {
                        if( ++PEGState == 2) {// DECAY-PHASE beginnt
                                PEGCount = pmi->PEGDecayTime;
                                PitchMul = pow( pow( 1/1.01, pmi->PEnvMod), 1.0/pmi->PEGDecayTime);
                        }
                        else  // AD-Kurve ist zu Ende
                                PitchModActive = false;
                }
        }

        else { // kein PitchMod
                if( GlideActive) {
                        if( pmi->LFO_Osc1) {
                                float pf = LFOOscTab[(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*pmi->LFO1AmountOsc1>>7) + 0x8000];
                                Phase1 += PhaseAdd1*GlideFactor*pf;
                                PhaseSub += (PhaseAdd1>>1)*GlideFactor*pf;
                        }
                        else {
                                Phase1 += PhaseAdd1*GlideFactor;
                                PhaseSub += (PhaseAdd1>>1)*GlideFactor;
                        }
                        if( pmi->LFO_Osc2)
                                Phase2 += PhaseAdd2*GlideFactor
                                  *LFOOscTab[(pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*pmi->LFO2AmountOsc2>>7) + 0x8000];
                        else
                                Phase2 += PhaseAdd2*GlideFactor;
                        GlideFactor *= GlideMul;
                        if( !GlideCount--) {
                                GlideActive = false;
                                PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
                                PhaseAdd2 = (int)(Frequency*pmi->DetuneSemi*pmi->DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
                        }
                }
                else {
                        if( pmi->LFO_Osc1) {
                                float pf = LFOOscTab[(pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*pmi->LFO1AmountOsc1>>7) + 0x8000];
                                Phase1 += PhaseAdd1*pf;
                                PhaseSub += (PhaseAdd1>>1)*pf;
                        }
                        else {
                                Phase1 += PhaseAdd1;
                                PhaseSub += (PhaseAdd1>>1);
                        }
                        if( pmi->LFO_Osc2)
                                Phase2 += PhaseAdd2
                                  *LFOOscTab[(pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*pmi->LFO2AmountOsc2>>7) + 0x8000];
                        else
                                Phase2 += PhaseAdd2;
                }
        }


        if( Phase1 & 0xf8000000) { // neuer durchlauf ??
                // PW1
                if( pmi->LFO_PW1) { //LFO_PW_Mod
                        center1 = pmi->Center1 + (float)pmi->pwavetabLFO1[((unsigned)PhLFO1)>>21]*
                                                pmi->LFO1AmountPW1;
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
                                c1 = center1*0x8000000;
                // PW2
                if( pmi->LFO_PW2) { //LFO_PW_Mod
                        center2 = pmi->Center2 + (float)pmi->pwavetabLFO2[((unsigned)PhLFO2)>>21]*
                                                pmi->LFO2AmountPW2;
                        if( center2 < 0)
                                center2 = 0;
                        else
                                if( center2 > 1)
                                        center2 = 1;
                }
                else  // No LFO
                        center2 = pmi->Center2;
                PhScale2A = 0.5/center2;
                PhScale2B = 0.5/(1-center2);
                                c2 = center2*0x8000000;

                // SYNC
                if( pmi->Sync)
                        Phase2 = Phase1; // !!!!!
        }

        Phase1 &= 0x7ffffff;
        Phase2 &= 0x7ffffff;
        PhaseSub &= 0x7ffffff;

        if( Phase1 < c1)
                Ph1 = Phase1*PhScale1A;
        else
                Ph1 = (Phase1 - c1)*PhScale1B + 0x4000000;

        if( Phase2 < c2)
                Ph2 = Phase2*PhScale2A;
        else
                Ph2 = (Phase2 - c2)*PhScale2B + 0x4000000;

                // LFOs
        PhLFO1 += pmi->PhaseAddLFO1;
        PhLFO2 += pmi->PhaseAddLFO2;
}

void CTrack::Work( float *psamples, int numsamples)
{
        for( int i=0; i<numsamples; i++) {
                if( AEGState) {

                        float o = Osc()*VCA();
                        *psamples++ = Filter( OldOut + o); // anti knack
                        OldOut = o;
                }
                else
                        *psamples++ = 0;
                NewPhases();
        }
}

//.......................................................
//.................... DESCRIPTION ......................
//.......................................................

char const *mi::DescribeValue(int const param, int const value)
{
        static const char *MixTypeTab[9] = {
                "add",
                "difference",
                "mul",
                "highest amp",
                "lowest amp",
                "and",
                "or",
                "xor",
                "random" };

        static const char *LFO1DestTab[16] = {
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
                "all"// 1234
        };

        static const char *LFOWaveTab[5] = {
                "sine",
                "saw",
                "square",
                "triangle",
                "random",
        };

        static const char *LFO2DestTab[16] = {
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
                "all" // 1234
        };

        static const char *FilterTypeTab[6] = {
                "lowpass24",
                "lowpass18",
                "lowpass12",
                "highpass",
                "bandpass",
                "bandreject" };

#include "waves/wavename.inc"


        static char txt[16];

        switch(param){
        case 0: // OSC1Wave
        case 2: // OSC2Wave
        case 9: // SubOscWave
                                return( wavenames[value]);
                                break;
                case 1: // PW1
        case 3: // PW2
                sprintf(txt, "%u : %u", (int)(value*100.0/127),
                                                                100-(int)(value*100.0/127));
                break;
        case 4: // semi detune
                if( value == 0x40)
                        return "�0 halfnotes";
                else
                        if( value > 0x40)
                                sprintf( txt, "+%i halfnotes", value-0x40);
                        else
                                sprintf( txt, "%i halfnotes", value-0x40);
                break;
        case 5: // fine detune
                if( value == 0x40)
                        return "�0 cents";
                else
                        if( value > 0x40)
                                sprintf( txt, "+%i cents", (int)((value-0x40)*100.0/63));
                        else
                                sprintf( txt, "%i cents", (int)((value-0x40)*100.0/63));
                break;

        case 6: // Sync
                if( value == SWITCH_ON)
                        return( "on");
                else
                        return( "off");
                break;

        case 7: // MixType
                                return MixTypeTab[value];
                                break;
        case 8: // Mix
                switch( value) {
                case 0:return "Osc1";
                case 127:return "Osc2";
                default: sprintf(txt, "%u%% : %u%%", 100-(int)(value*100.0/127),
                                                                (int)(value*100.0/127));
                }
                break;

        case 11: // Pitch Env
        case 12: // Pitch Env
        case 15: // Amp Env
        case 16: // Amp Env
        case 17: // Amp Env
        case 21: // Filter Env
        case 22: // Filter Env
        case 23: // Filter Env
                sprintf( txt, "%.4f sec", scalEnvTime( value)/1000);
                break;

        case 13: // PitchEnvMod
        case 24: // Filt ENvMod
                sprintf( txt, "%i", value-0x40);
                break;
        case 18: //FilterType
                                return FilterTypeTab[value];
                                break;
        case 25: // LFO1Dest
                                return LFO1DestTab[value];
                                break;
        case 30: // LFO2Dest
                                return LFO2DestTab[value];
                                break;
        case 26: // LFO1Wave
        case 31: // LFO2Wave
                                return LFOWaveTab[value];
                                break;
        case 27: // LFO1Freq
        case 32: // LFO2Freq
                if( value <= 116)
                        sprintf( txt, "%.4f HZ", scalLFOFreq( value));
                else
                        sprintf( txt, "%u ticks", 1<<(value-117));
                break;
                case 29: //LFO1PhaseDiff
                case 34: //LFO2PhaseDiff
                        sprintf( txt, "%i�", value*360/128);
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
}


void mi::Init(CMachineDataInput * const pi)
{
#ifndef _MSC_VER
    DSP_Init(pMasterInfo->SamplesPerSec);
#endif
        TabSizeDivSampleFreq = (float)(2048.0/pMasterInfo->SamplesPerSec);

        // Filter
        coefsTabOffs = coefsTab; // LowPass
        Cutoff = paraCutoff.DefValue;
        Resonance = paraResonance.DefValue;
        db24 = db18 = false;
        //PEG
        PEGAttackTime = MSToSamples( scalEnvTime( paraPEGAttackTime.DefValue));
        PEGDecayTime = MSToSamples( scalEnvTime( paraPEGDecayTime.DefValue));
        PEnvMod = 0;
        // FEG
        FEGAttackTime = MSToSamples( scalEnvTime( paraFEGAttackTime.DefValue));
        FEGSustainTime = MSToSamples( scalEnvTime( paraFEGSustainTime.DefValue));
        FEGReleaseTime = MSToSamples( scalEnvTime( paraFEGReleaseTime.DefValue));
        FEnvMod = 0;
        // AEG
        AEGAttackTime = MSToSamples( scalEnvTime( paraAEGAttackTime.DefValue));
        AEGSustainTime = MSToSamples( scalEnvTime( paraAEGSustainTime.DefValue));
        AEGReleaseTime = MSToSamples( scalEnvTime( paraAEGReleaseTime.DefValue));


        pwavetab1 = pwavetab2 = pwavetabsub = waves;


        noise1 = noise2 = Sync = false;
        LFO1Noise = LFO2Noise = false;
        LFO1Synced = LFO2Synced = false;

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
        LFO2PhaseDiff = paraLFO1PhaseDiff.DefValue<<(9+16);

        // OscMix
        Bal1 = 127-paraMix.DefValue;
        Bal2 = paraMix.DefValue;
        MixType = 0;

        LFO_Osc1 = LFO_PW1 = LFO_Amp = LFO_Cut = false;
        LFO_Osc2 = LFO_PW2 = LFO_Mix = LFO_Reso = false;

        for( int i=0; i<MAX_TRACKS; i++)
        {
                Tracks[i].pmi = this;
                Tracks[i].Init();
        }

        // generate coefsTab
        for( int t=0; t<4; t++)
                for( int f=0; f<128; f++)
                        for( int r=0; r<128; r++)
                                ComputeCoefs( coefsTab+(t*128*128+f*128+r)*8, f, r, t);
        // generate LFOOscTab
        for( int p=0; p<0x10000; p++)
                LFOOscTab[p] = pow( 1.00004230724139582, p-0x8000);

}


void mi::Tick()
{
        // Filter
        if( gval.FilterType != paraFilterType.NoValue) {
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
                        else {
                                db18 = false;
                                db24 = false;
                                coefsTabOffs = coefsTab + (int)(gval.FilterType-2)*128*128*8;
                        }
                }
        }

        if( gval.Cutoff != paraCutoff.NoValue)
                Cutoff = gval.Cutoff;
        if( gval.Resonance != paraResonance.NoValue)
                Resonance = gval.Resonance;

        // FEG
        if( gval.FEGAttackTime != paraFEGAttackTime.NoValue)
                FEGAttackTime = MSToSamples( scalEnvTime( gval.FEGAttackTime));
        if( gval.FEGSustainTime != paraFEGSustainTime.NoValue)
                FEGSustainTime = MSToSamples( scalEnvTime( gval.FEGSustainTime));
        if( gval.FEGReleaseTime != paraFEGReleaseTime.NoValue)
                FEGReleaseTime = MSToSamples( scalEnvTime( gval.FEGReleaseTime));
        if( gval.FEnvMod != paraFEnvMod.NoValue)
                FEnvMod = (gval.FEnvMod - 0x40)<<1;

        // PEG
        if( gval.PEGAttackTime != paraPEGAttackTime.NoValue)
                PEGAttackTime = MSToSamples( scalEnvTime( gval.PEGAttackTime));
        if( gval.PEGDecayTime != paraPEGDecayTime.NoValue)
                PEGDecayTime = MSToSamples( scalEnvTime( gval.PEGDecayTime));

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


        if( gval.Mix != paraMix.NoValue) {
                Bal1 = 127-gval.Mix;
                Bal2 = gval.Mix;
        }

        if( gval.Glide != paraGlide.NoValue) {
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

        // SubOsc
        if( gval.SubOscWave != paraSubOscWave.NoValue)
                pwavetabsub = waves + (gval.SubOscWave<<11);


        if( gval.SubOscVol != paraSubOscVol.NoValue)
                SubOscVol = gval.SubOscVol;

        // PW
        if( gval.PulseWidth1 != paraPulseWidth1.NoValue)
                Center1 = gval.PulseWidth1/127.0;

        if( gval.PulseWidth2 != paraPulseWidth2.NoValue)
                Center2 = gval.PulseWidth2/127.0;

        // Detune
        if( gval.DetuneSemi != paraDetuneSemi.NoValue)
                DetuneSemi = (float)pow( 1.05946309435929526, gval.DetuneSemi-0x40);
        if( gval.DetuneFine != paraDetuneFine.NoValue)
                DetuneFine = (float)pow( 1.00091728179580156, gval.DetuneFine-0x40);
        if( gval.Sync != SWITCH_NO) {
                if( gval.Sync == SWITCH_ON)
                        Sync = true;
                else
                        Sync = false;
        }

        if( gval.MixType != paraMixType.NoValue)
                MixType = gval.MixType;

        if( gval.Wave1 != paraWave1.NoValue) { // neuer wert
                if( gval.Wave1 == NUMWAVES)
                        noise1 = true;
                else {
                        noise1 = false;
                        pwavetab1 = waves + (gval.Wave1 << 11);
                }
        }

        if( gval.Wave2 != paraWave2.NoValue) { // neuer wert
                if( gval.Wave2 == NUMWAVES)
                        noise2 = true;
                else {
                        noise2 = false;
                        pwavetab2 = waves + (gval.Wave2 << 11);
                }
        }

        // AEG
        if( gval.AEGAttackTime != paraAEGAttackTime.NoValue)
                AEGAttackTime = MSToSamples( scalEnvTime( gval.AEGAttackTime));
        if( gval.AEGSustainTime != paraAEGSustainTime.NoValue)
                AEGSustainTime = MSToSamples( scalEnvTime( gval.AEGSustainTime));
        if( gval.AEGReleaseTime != paraAEGReleaseTime.NoValue)
                AEGReleaseTime = MSToSamples( scalEnvTime( gval.AEGReleaseTime));

        // ..........LFO............

        // LFO1
        if( gval.LFO1Dest != paraLFO1Dest.NoValue) {
                   LFO_Osc1 = LFO_PW1 = LFO_Amp = LFO_Cut = false;
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
                        }
                }

        if( gval.LFO1Wave != paraLFO1Wave.NoValue) {
                pwavetabLFO1 = pCB->GetOscillatorTable( gval.LFO1Wave);
                if( gval.LFO1Wave == OWF_NOISE)
                        LFO1Noise = true;
                else
                        LFO1Noise = false;
        }


        if( gval.LFO1Freq != paraLFO1Freq.NoValue) {
                if( gval.LFO1Freq>116) {
                        LFO1Synced = true;
                        LFO1Freq = gval.LFO1Freq - 117;
                }
                else {
                        LFO1Synced = false;
                        LFO1Freq = gval.LFO1Freq;
                }
        }

        if( gval.LFO1Amount != paraLFO1Amount.NoValue)
                LFO1Amount = gval.LFO1Amount;


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
                        LFO_Osc2 = LFO_PW2 = LFO_Mix = LFO_Reso = false;
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
                }
        }

        if( gval.LFO2Wave != paraLFO2Wave.NoValue) {
                pwavetabLFO2 = pCB->GetOscillatorTable( gval.LFO2Wave);
                if( gval.LFO2Wave == OWF_NOISE)
                        LFO2Noise = true;
                else
                        LFO2Noise = false;
        }

        if( gval.LFO2Freq != paraLFO2Freq.NoValue) {
                if( gval.LFO2Freq>116) {
                        LFO2Synced = true;
                        LFO2Freq = gval.LFO2Freq - 117;
                }
                else {
                        LFO2Synced = false;
                        LFO2Freq = gval.LFO2Freq;
                }
        }

        if( gval.LFO2Amount != paraLFO2Amount.NoValue)
                LFO2Amount = gval.LFO2Amount;

        // LFO-Phasen-Differenzen
        if( gval.LFO1PhaseDiff != paraLFO1PhaseDiff.NoValue)
                LFO1PhaseDiff = gval.LFO1PhaseDiff << (9+16);
        if( gval.LFO2PhaseDiff != paraLFO2PhaseDiff.NoValue)
                LFO2PhaseDiff = gval.LFO2PhaseDiff << (9+16);


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
        LFO1AmountOsc1 = (LFO1Amount*aval.LFO1ScaleOsc1)>>7;
        LFO1AmountPW1 = (LFO1Amount*aval.LFO1ScalePW1/(128.0*127.0*0x8000));
        LFO1AmountVolume = (LFO1Amount*aval.LFO1ScaleVolume)>>7;
        LFO1AmountCutoff = (LFO1Amount*aval.LFO1ScaleCutoff)>>7;
        LFO2AmountOsc2 = (LFO2Amount*aval.LFO2ScaleOsc2)>>7;
        LFO2AmountPW2 = (LFO2Amount*aval.LFO2ScalePW2/(128.0*127.0*0x8000));
        LFO2AmountMix = (LFO2Amount*aval.LFO2ScaleMix)>>7;
        LFO2AmountReso = (LFO2Amount*aval.LFO2ScaleReso)>>7;


        // TrackParams durchgehen
        for (int i=0; i<numTracks; i++)
                Tracks[i].Tick( tval[i]);
}


bool mi::Work(float *psamples, int numsamples, int const)
{
        bool gotsomething = false;

        for ( int i=0; i<numTracks; i++) {
                if ( Tracks[i].AEGState) {
                        Tracks[i].PhLFO1 = PhaseLFO1 + i*LFO1PhaseDiff;
                        Tracks[i].PhLFO2 = PhaseLFO2 + i*LFO2PhaseDiff;
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
        PhaseLFO1 += PhaseAddLFO1*numsamples;
        PhaseLFO2 += PhaseAddLFO2*numsamples;
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
                a0 = 1;
                a1 = a2 = b0 = b1 = b2 = 0;
                break;
        }

        coefs[0] = b0/a0;
        coefs[1] = b1/a0;
        coefs[2] = b2/a0;
        coefs[3] = -a1/a0;
        coefs[4] = -a2/a0;
}
