// M3 Buzz plugin by MAKK makk@gmx.de
// released on 04-21-99
// Thanks must go to Robert Bristow-Johnson pbjrbj@viconet.com
// a.k.a. robert@audioheads.com for his excellent
// Cookbook formulas for the filters.
// The code is not really speed optimized
// and compiles with many warnings - i'm to lazy to correct
// them all (mostly typecasts).
// Use the source for your own plugins if you want, but don't
// rip the hole machine please.
// Thanks in advance. MAKK

#include <windef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include <dsplib.h>


#pragma optimize ("a", on)

#define MAX_TRACKS                                                              4

#define EGS_NONE                                0
#define EGS_ATTACK                              1
#define EGS_SUSTAIN                             2
#define EGS_RELEASE                             3


float *freqTab = new float [NOTE_MAX+1];
float *coefsTab = new float [4*128*128*8];
float *LFOOscTab = new float [0x10000];

CMachineParameter const paraNote =
{
        pt_note,                                                                                // type
        "Note",
        "Note",                                                                     // description
        NOTE_MIN,                                                                               // Min
        NOTE_MAX,                                                                               // Max
        0xff,                                                                                   // NoValue
        0,                                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraWave1 =
{
        pt_byte,                                                                                // type
        "Osc1Wav",
        "Oscillator 1 Waveform",                                                // description
        0,                                                                                              // Min
        5,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraPulseWidth1 =
{
        pt_byte,                                                                                // type
        "PulseWidth1",
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
        "Osc2Wav",
        "Oscillator 2 Waveform",                                                // description
        0,                                                                                              // Min
        5,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraPulseWidth2 =
{
        pt_byte,                                                                                // type
        "PulseWidth2",
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
        "Mix",
        "Mix Osc1 <-> Osc2",
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraMixType =
{
        pt_byte,                                                                                // type
        "MixType",
        "MixType",                                                                              // description
        0,                                                                                              // Min
        8,                                                                                              // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};



CMachineParameter const paraSync =
{
        pt_switch,                                                                              // type
        "Sync",
        "Sync: Osc2 synced by Osc1",                                    // description
        SWITCH_OFF,                                                                             // Min
        SWITCH_ON,                                                                              // Max
        SWITCH_NO,                                                                              // NoValue
        MPF_STATE,                                                                              // Flags
        SWITCH_OFF                                                                              // default
};



CMachineParameter const paraDetuneSemi=
{
        pt_byte,                                                                                // type
        "Semi Detune",
        "Semi Detune in Halfnotes",                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                    // default
};

CMachineParameter const paraDetuneFine=
{
        pt_byte,                                                                                // type
        "Fine Detune",
        "Fine Detune",                                                                  // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                            // default
};

CMachineParameter const paraGlide =
{
        pt_byte,                                                                                // type
        "Glide",
        "Glide",                                                                                // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraSubOscWave =
{
        pt_byte,                                                                                // type
        "SubOscWav",
        "Sub Oscillator Waveform",                                              // description
        0,                                                                                              // Min
        4,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraSubOscVol =
{
        pt_byte,                                                                                // type
        "SubOscVol",
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
        "AEGAttack",
        "AEG Attack Time in ms",                                                // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        10                                                                                              // default
};

CMachineParameter const paraAEGSustainTime =
{
        pt_byte,                                                                                // type
        "AEGSustain",
        "AEG Sustain Time in ms",                                               // description
        0,                                                                                              // Min
        127,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        50                                                                                              // default
};

CMachineParameter const paraAEGReleaseTime =
{
        pt_byte,                                                                                // type
        "AEGRelease",
        "AEG Release Time in ms",                                               // description
        0,                                                                                              // Min
        127,                                                                            // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        30                                                                                              // default
};

CMachineParameter const paraFilterType =
{
        pt_byte,                                                                                // type
        "FilterType",
        "Filter Type ... 0=LP 1=HP 2=BP 3=BR",                  // description
        0,                                                                                              // Min
        3,                                                                              // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraCutoff =
{
        pt_byte,                                                                                // type
        "Cutoff",
        "Filter Cutoff Frequency",                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        127                                                                                             // default
};

CMachineParameter const paraResonance =
{
        pt_byte,                                                                                // type
        "Res./Bandw.",
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
        "PEG Attack",
        "PEG Attack Time",                                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};


CMachineParameter const paraPEGDecayTime =
{
        pt_byte,                                                                                // type
        "PEG Release",
        "PEG Release Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};


CMachineParameter const paraPEnvMod =
{
        pt_byte,                                                                                // type
        "PEnvMod",
        "Pitch Envelope Modulation",                                    // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                            // default
};


CMachineParameter const paraFEGAttackTime =
{
        pt_byte,                                                                                // type
        "FEG Attack",
        "FEG Attack Time",                                                              // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraFEGSustainTime =
{
        pt_byte,                                                                                // type
        "FEG Sustain",
        "FEG Sustain Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};


CMachineParameter const paraFEGReleaseTime =
{
        pt_byte,                                                                                // type
        "FEG Release",
        "FEG Release Time",                                                             // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};


CMachineParameter const paraFEnvMod =
{
        pt_byte,                                                                                // type
        "FEnvMod",
        "Filter Envelope Modulation",                                   // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0x40                                                                                            // default
};

// LFOs
CMachineParameter const paraLFO1Dest =
{
        pt_byte,                                                                                // type
        "LFO1Dest",
        "LFO1 Destination",                                                             // description
        0,                                                                                              // Min
        15,                                                                                     // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1Wave =
{
        pt_byte,                                                                                // type
        "LFO1Wav",
        "LFO1 Waveform",                                                                // description
        0,                                                                                              // Min
        4,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1Freq =
{
        pt_byte,                                                                                // type
        "LFO1Freq",
        "LFO1 Frequency",                                                               // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO1Amount =
{
        pt_byte,                                                                                // type
        "LFO1Amount",
        "LFO1 Amount",                                                                  // description
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
        "LFO2Dest",
        "LFO2 Destination",                                                             // description
        0,                                                                                              // Min
        15,                                                                                     // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO2Wave =
{
        pt_byte,                                                                                // type
        "LFO2Wav",
        "LFO2 Waveform",                                                                // description
        0,                                                                                              // Min
        4,                                                                                      // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO2Freq =
{
        pt_byte,                                                                                // type
        "LFO2Freq",
        "LFO2 Frequency",                                                               // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};

CMachineParameter const paraLFO2Amount =
{
        pt_byte,                                                                                // type
        "LFO2Amount",
        "LFO2 Amount",                                                                  // description
        0,                                                                                              // Min
        127,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        0                                                                                               // default
};




CMachineParameter const *pParameters[] = {
        &paraNote,
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

        &paraVolume,
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
        // LFO 2
        &paraLFO2Dest,
        &paraLFO2Wave,
        &paraLFO2Freq,
        &paraLFO2Amount,
};

#pragma pack(1)


class tvals
{
public:
        byte Note;
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

        byte Volume;
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
        byte LFO2Dest;
        byte LFO2Wave;
        byte LFO2Freq;
        byte LFO2Amount;
};


#pragma pack()

CMachineInfo const MacInfo =
{
        MT_GENERATOR,                                                   // type
        MI_VERSION,
        0,                                                                              // flags
        1,                                                                              // min tracks
        MAX_TRACKS,                                                             // max tracks
        0,                                                                              // numGlobalParameters
        35,                                                                             // numTrackParameters
        pParameters,
        0,
        NULL,
#ifdef _DEBUG
        "M3 by Makk (Debug build)",                     // name
#else
        "M3 by Makk",
#endif
        "M3",                                                                   // short name
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
        inline float Osc();
        inline float Filter( float x);
        inline float VCA();
        void NewPhases();
        int MSToSamples(double const ms);

public:

        // ......Osc......
        byte Note;
        const short *pwavetab1, *pwavetab2, *pwavetabsub;
        int SubOscVol;
        bool noise1, noise2;
        int Bal1, Bal2;
        int MixType;
        int Phase1, Phase2, PhaseSub;
        int Ph1, Ph2;
        float center1, center2;
        float Center1, Center2;
        float PhScale1A, PhScale1B;
        float PhScale2A, PhScale2B;
        int PhaseAdd1, PhaseAdd2;
        float Frequency, FrequencyFrom;
        // Detune
        float DetuneSemi, DetuneFine;
        bool Sync;
        // Glide
        bool Glide, GlideActive;
        float GlideMul, GlideFactor;
        int GlideTime, GlideCount;
        // PitchEnvMod
        bool PitchMod, PitchModActive;
        // PEG ... AD-H�llkurve
        int PEGState;
        int PEGAttackTime;
        int PEGDecayTime;
        int PEGCount;
        float PitchMul, PitchFactor;
        int PEnvMod;
        // random generator... rauschen
        short r1, r2, r3, r4;

        float OldOut; // gegen extreme Knackser/Wertespr�nge

        // .........AEG........ ASR-H�llkurve
        float Volume;
        int AEGState;
        int AEGAttackTime;
        int AEGSustainTime;
        int AEGReleaseTime;
        int AEGCount;
        float Amp;
        float AmpAdd;

        // ........Filter..........
        float *coefsTabOffs; // abh�ngig vom FilterTyp
        int Cutoff, Resonance;
        float x1, x2, y1, y2;
        // FEG ... ASR-H�llkurve
        int FEnvMod;
        int FEGState;
        int FEGAttackTime;
        int FEGSustainTime;
        int FEGReleaseTime;
        int FEGCount;
        float Cut;
        float CutAdd;

        // .........LFOs...........
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

        bool LFO1Noise, LFO2Noise; // andere Frequenz
        bool LFO1Synced,LFO2Synced; // zum Songtempo
        const short *pwavetabLFO1, *pwavetabLFO2;
        int LFO1Amount, LFO2Amount;
        int PhaseLFO1, PhaseLFO2, PhaseAddLFO1, PhaseAddLFO2;
        int LFO1Freq, LFO2Freq;



        // RANDOMS
        const short *pnoise;
        int noisePhase;
        bool RandomMixType;
        bool RandomWave1;
        bool RandomWave2;
        bool RandomWaveSub;

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
        inline float LFOFreq( int v);
        inline float EnvTime( int v);
        inline float Cutoff( int v);
        inline float Resonance( float v);
        inline float Bandwidth( int v);

public:
        float TabSizeDivSampleFreq;
        int numTracks;
        CTrack Tracks[MAX_TRACKS];
        tvals tval[MAX_TRACKS];
};


DLL_EXPORTS

// Skalierungsmethoden
inline float mi::Cutoff( int v)
{
        return pow( (v+5)/(127.0+5), 1.7)*13000+30;
}
inline float mi::Resonance( float v)
{
        return pow( v/127.0, 4)*150+0.1;
}
inline float mi::Bandwidth( int v)
{
        return pow( v/127.0, 4)*4+0.1;
}

inline float mi::LFOFreq( int v)
{
        return (pow( (v+8)/(116.0+8), 4)-0.000017324998565270)*40.00072;
}

inline float mi::EnvTime( int v)
{
        return pow( (v+2)/(127.0+2), 3)*10000;
}

//////////////////////////////////////////////////////
// CTRACK METHODEN
//////////////////////////////////////////////////////

inline int CTrack::MSToSamples(double const ms)
{
        return (int)(pmi->pMasterInfo->SamplesPerSec * ms * (1.0 / 1000.0)) + 1; // +1 wg. div durch 0
}


void CTrack::Stop()
{
        AEGState = EGS_NONE;
}

void CTrack::Init()
{
        noise1 = noise2 = Sync = false;
        RandomMixType = false;
        RandomWave1 = false;
        RandomWave2 = false;
        RandomWaveSub = false;
        LFO1Noise = LFO2Noise = false;
        LFO1Synced = LFO2Synced = false;
        AEGState = EGS_NONE;
		FEGState = EGS_NONE;
		PEGState = EGS_NONE;
        r1=26474; r2=13075; r3=18376; r4=31291; // randomGenerator
        noisePhase = Phase1 = Phase2 = PhaseSub = PhaseLFO1 = PhaseLFO2 = 0; // Osc starten neu
        x1 = x2 = y1 = x2 = 0;
        pnoise = pmi->pCB->GetOscillatorTable( OWF_NOISE);
        OldOut = 0;
        pwavetab1=pwavetab2=pwavetabsub=pwavetabLFO1=pwavetabLFO2=
            pmi->pCB->GetOscillatorTable( OWF_SINE);
		
		coefsTabOffs = coefsTab; // lp
        Cutoff = paraCutoff.DefValue;
        Resonance = paraResonance.DefValue;
        FEGAttackTime = MSToSamples( pmi->EnvTime( paraFEGAttackTime.DefValue));
        FEGSustainTime = MSToSamples( pmi->EnvTime( paraFEGSustainTime.DefValue));
        FEGReleaseTime = MSToSamples( pmi->EnvTime( paraFEGReleaseTime.DefValue));
        AEGAttackTime = MSToSamples( pmi->EnvTime( paraAEGAttackTime.DefValue));
        AEGSustainTime = MSToSamples( pmi->EnvTime( paraAEGSustainTime.DefValue));
        AEGReleaseTime = MSToSamples( pmi->EnvTime( paraAEGReleaseTime.DefValue));
		FEnvMod = 0;
        PEGAttackTime = MSToSamples( pmi->EnvTime( paraPEGAttackTime.DefValue));
        PEGDecayTime = MSToSamples( pmi->EnvTime( paraPEGDecayTime.DefValue));
		PEnvMod = 0;
        Bal1 = 127-paraMix.DefValue;
        Bal2 = paraMix.DefValue;
		Glide =  GlideActive = false;
		RandomWave1 = RandomWave2 = RandomWaveSub = false;
		SubOscVol = paraSubOscVol.DefValue;
        Center1 = paraPulseWidth1.DefValue/127.0;
        Center2 = paraPulseWidth2.DefValue/127.0;
		DetuneSemi = DetuneFine = 1;
		Volume = (float)(paraVolume.DefValue/245.0);
		LFO_Osc1 = LFO_PW1 = LFO_Amp = LFO_Cut = false;	
		LFO_Osc2 = LFO_PW2 = LFO_Mix = LFO_Reso = false;
		PhaseAddLFO1 = PhaseAddLFO2 = 0;
		MixType = 0;
}

void CTrack::Tick( tvals const &tv)
{

        // Filter
        if( tv.FilterType != paraFilterType.NoValue)
                coefsTabOffs = coefsTab + (int)tv.FilterType*128*128*8;
        if( tv.Cutoff != paraCutoff.NoValue)
                Cutoff = tv.Cutoff;
        if( tv.Resonance != paraResonance.NoValue)
                Resonance = tv.Resonance;
        // FEG
        if( tv.FEGAttackTime != paraFEGAttackTime.NoValue)
                FEGAttackTime = MSToSamples( pmi->EnvTime( tv.FEGAttackTime));
        if( tv.FEGSustainTime != paraFEGSustainTime.NoValue)
                FEGSustainTime = MSToSamples( pmi->EnvTime( tv.FEGSustainTime));
        if( tv.FEGReleaseTime != paraFEGReleaseTime.NoValue)
                FEGReleaseTime = MSToSamples( pmi->EnvTime( tv.FEGReleaseTime));
        if( tv.FEnvMod != paraFEnvMod.NoValue)
                FEnvMod = (tv.FEnvMod - 0x40)<<1;





        // PEG
        if( tv.PEGAttackTime != paraPEGAttackTime.NoValue)
                PEGAttackTime = MSToSamples( pmi->EnvTime( tv.PEGAttackTime));
        if( tv.PEGDecayTime != paraPEGDecayTime.NoValue)
                PEGDecayTime = MSToSamples( pmi->EnvTime( tv.PEGDecayTime));
        if( tv.PEnvMod != paraPEnvMod.NoValue) {
                if( tv.PEnvMod - 0x40 != 0)
                        PitchMod = true;
                else {
                        PitchMod = false;
                        PitchModActive = false;
                }
                PEnvMod = tv.PEnvMod - 0x40;
        }

        if( tv.Mix != paraMix.NoValue) {
                Bal1 = 127-tv.Mix;
                Bal2 = tv.Mix;
        }

        if( tv.Glide != paraGlide.NoValue) {
                if( tv.Glide == 0) {
                        Glide = false;
                        GlideActive = false;
                }
                else {
                        Glide = true;
                        GlideTime = tv.Glide*10000000/pmi->pMasterInfo->SamplesPerSec;
                }
        }

        // SubOsc
        if( tv.SubOscWave != paraSubOscWave.NoValue) {
                if( tv.SubOscWave == 4) // random
                        RandomWaveSub = true;
                else {
                        pwavetabsub = pmi->pCB->GetOscillatorTable( tv.SubOscWave);
                        RandomWaveSub = false;
                }
        }

        if( tv.SubOscVol != paraSubOscVol.NoValue)
                SubOscVol = tv.SubOscVol;

        // PW
        if( tv.PulseWidth1 != paraPulseWidth1.NoValue) {
                Center1 = tv.PulseWidth1/127.0;
        }
        if( tv.PulseWidth2 != paraPulseWidth2.NoValue) {
                Center2 = tv.PulseWidth2/127.0;
        }

        // Detune
        if( tv.DetuneSemi != paraDetuneSemi.NoValue)
                DetuneSemi = (float)pow( 1.05946309435929526, tv.DetuneSemi-0x40);
        if( tv.DetuneFine != paraDetuneFine.NoValue)
                DetuneFine = (float)pow( 1.00091728179580156, tv.DetuneFine-0x40);
        if( tv.Sync != SWITCH_NO) {
                if( tv.Sync == SWITCH_ON)
                        Sync = true;
                else
                        Sync = false;
        }

        if( tv.MixType != paraMixType.NoValue) {
                if( tv.MixType == 8) // random
                        RandomMixType = true;
                else {
                        MixType = tv.MixType;
                        RandomMixType = false;
                }
        }

        if( tv.Wave1 != paraWave1.NoValue) { // neuer wert
                if( tv.Wave1 == OWF_NOISE)
                        noise1 = true;
                else
                        noise1 = false;
                if( tv.Wave1 == 5) // random
                        RandomWave1 = true;
                else {
                        RandomWave1 = false;
                        pwavetab1 = pmi->pCB->GetOscillatorTable( tv.Wave1);
                }
        }

        if( tv.Wave2 != paraWave2.NoValue) { // neuer wert
                if( tv.Wave2 == OWF_NOISE)
                        noise2 = true;
                else
                        noise2 = false;
                if( tv.Wave2 == 5) // random
                        RandomWave2 = true;
                else {
                        RandomWave2 = false;
                        pwavetab2 = pmi->pCB->GetOscillatorTable( tv.Wave2);
                }
        }

        if( tv.AEGAttackTime != paraAEGAttackTime.NoValue)
                AEGAttackTime = MSToSamples( pmi->EnvTime( tv.AEGAttackTime));
        if( tv.AEGSustainTime != paraAEGSustainTime.NoValue)
                AEGSustainTime = MSToSamples( pmi->EnvTime( tv.AEGSustainTime));
        if( tv.AEGReleaseTime != paraAEGReleaseTime.NoValue)
                AEGReleaseTime = MSToSamples( pmi->EnvTime( tv.AEGReleaseTime));
        if( tv.Volume != paraVolume.NoValue)
                Volume = (float)(tv.Volume/245.0);

        if( tv.Note != paraNote.NoValue) { // neuer wert
                Note = tv.Note;
                if( (Note >= NOTE_MIN) && (Note <= NOTE_MAX)) { // neue note gesetzt
                        FrequencyFrom = Frequency;
                        Frequency = freqTab[Note];


                        // RANDOMS
                        if( RandomMixType) {
                                MixType = (unsigned)pnoise[noisePhase++] % 8;
                                noisePhase &= 0x7ff;
                        }
                        if( RandomWaveSub) {
                                pwavetabsub = pmi->pCB->GetOscillatorTable( (unsigned)pnoise[noisePhase++] % 4);
                                noisePhase &= 0x7ff;
                        }
                        if( RandomWave1) {
                                pwavetab1 = pmi->pCB->GetOscillatorTable( (unsigned)pnoise[noisePhase++] % 4);
                                noisePhase &= 0x7ff;
                        }
                        if( RandomWave2) {
                                pwavetab2 = pmi->pCB->GetOscillatorTable( (unsigned)pnoise[noisePhase++] % 4);
                                noisePhase &= 0x7ff;
                        }

                        if( Glide) {
                                GlideActive = true;
                                if( Frequency > FrequencyFrom)
                                        GlideMul = pow( 2, 1.0/GlideTime);
                                else
                                        GlideMul = pow( 0.5, 1.0/GlideTime);
                                GlideFactor = 1;
                                GlideCount = (int)(log( Frequency/FrequencyFrom)/log(GlideMul));
                        }
                        else
                                GlideActive = false;

                        // trigger envelopes neu an...
                        // Amp
                        AEGState = EGS_ATTACK;
                        AEGCount = AEGAttackTime;
                        AmpAdd = Volume/AEGAttackTime;
                        Amp = 0; //AmpAdd; // fange bei 0 an
                        // Pitch
                        if( PitchMod) {
                                PitchModActive = true;
                                PEGState = EGS_ATTACK;
                                PEGCount = PEGAttackTime;
                                PitchMul = pow( pow( 1.01, PEnvMod), 1.0/PEGAttackTime);
                                PitchFactor = 1.0;
                        }
                        else
                                PitchModActive = false;

                        // Filter
                        FEGState = EGS_ATTACK;
                        FEGCount = FEGAttackTime;
                        CutAdd = ((float)FEnvMod)/FEGAttackTime;
                        Cut = 0.0; // fange bei 0 an


                } else
                        if( tv.Note == NOTE_OFF)
                                AEGState = EGS_NONE; // note aus

        }

        // ..........LFO............

        // LFO1
        if( tv.LFO1Dest != paraLFO1Dest.NoValue)
        {
                LFO_Osc1 = LFO_PW1 = LFO_Amp = LFO_Cut = false;
                switch( tv.LFO1Dest) {
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
        if( tv.LFO1Wave != paraLFO1Wave.NoValue) {
                pwavetabLFO1 = pmi->pCB->GetOscillatorTable( tv.LFO1Wave);
                if( tv.LFO1Wave == OWF_NOISE)
                        LFO1Noise = true;
                else
                        LFO1Noise = false;
        }

        if( tv.LFO1Freq != paraLFO1Freq.NoValue) {
                if( tv.LFO1Freq>116) {
                        LFO1Synced = true;
                        LFO1Freq = tv.LFO1Freq - 117;
                }
                else {
                        LFO1Synced = false;
                        LFO1Freq = tv.LFO1Freq;
                }
        }
        if( tv.LFO1Amount != paraLFO1Amount.NoValue)
                LFO1Amount = tv.LFO1Amount;

        if( LFO1Synced)
                if( LFO1Noise) // sample & hold
                        PhaseAddLFO1 = (int)(0x200000/(pmi->pMasterInfo->SamplesPerTick<<LFO1Freq));
                else
                        PhaseAddLFO1 = (int)((double)0x200000*2048/(pmi->pMasterInfo->SamplesPerTick<<LFO1Freq));
        else
                if( LFO1Noise) // sample & hold
                        PhaseAddLFO1 = (int)(pmi->LFOFreq( LFO1Freq)/pmi->pMasterInfo->SamplesPerSec*0x200000);
                else
                        PhaseAddLFO1 = (int)(pmi->LFOFreq( LFO1Freq)*pmi->TabSizeDivSampleFreq*0x200000);

        // LFO2
        if( tv.LFO2Dest != paraLFO2Dest.NoValue)
        {
                LFO_Osc2 = LFO_PW2 = LFO_Mix = LFO_Reso = false;
                switch( tv.LFO2Dest) {
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
        if( tv.LFO2Wave != paraLFO2Wave.NoValue) {
                pwavetabLFO2 = pmi->pCB->GetOscillatorTable( tv.LFO2Wave);
                if( tv.LFO2Wave == OWF_NOISE)
                        LFO2Noise = true;
                else
                        LFO2Noise = false;
        }

        if( tv.LFO2Freq != paraLFO2Freq.NoValue) {
                if( tv.LFO2Freq>116) {
                        LFO2Synced = true;
                        LFO2Freq = tv.LFO2Freq - 117;
                }
                else {
                        LFO2Synced = false;
                        LFO2Freq = tv.LFO2Freq;
                }
        }

        if( tv.LFO2Amount != paraLFO2Amount.NoValue)
                LFO2Amount = tv.LFO2Amount;

        if( LFO2Synced)
                if( LFO2Noise) // sample & hold
                        PhaseAddLFO2 = (int)(0x200000/(pmi->pMasterInfo->SamplesPerTick<<LFO2Freq));
                else
                        PhaseAddLFO2 = (int)((double)0x200000*2048/(pmi->pMasterInfo->SamplesPerTick<<LFO2Freq));
        else
                if( LFO2Noise) // sample & hold
                        PhaseAddLFO2 = (int)(pmi->LFOFreq( LFO2Freq)/pmi->pMasterInfo->SamplesPerSec*0x200000);
                else
                        PhaseAddLFO2 = (int)(pmi->LFOFreq( LFO2Freq)*pmi->TabSizeDivSampleFreq*0x200000);


        if( GlideActive) {
                PhaseAdd1 = (int)(FrequencyFrom*pmi->TabSizeDivSampleFreq*0x10000);
                PhaseAdd2 = (int)(FrequencyFrom*DetuneSemi*DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
        }
        else {
                PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
                PhaseAdd2 = (int)(Frequency*DetuneSemi*DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
        }

}


inline float CTrack::Osc()
{
        float o, o2;
        int B1, B2;
        if( LFO_Mix) {
                B2 = Bal2 + ((pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*LFO2Amount)>>15);
                if( B2<0)
                        B2 = 0;
                else
                        if( B2>127)
                                B2 = 127;
                B1 = 127-B2;
        }
        else {
                B1 = Bal1;
                B2 = Bal2;
        }

        // osc1
        if( noise1) {
                short t = r1+r2+r3+r4;
                r1=r2; r2=r3; r3=r4; r4=t;
                o = (float)((t*B1)>>7);
        }
        else
                o = (float)((pwavetab1[Ph1>>16]*B1)>>7);

        // osc2
        if( noise2) {
                short u = r1+r2+r3+r4;
                r1=r2; r2=r3; r3=r4; r4=u;
                o2 = (u*B2)>>7;
        }
        else
                o2 = (pwavetab2[Ph2>>16]*B2)>>7;


        switch( MixType)
        {
        case 0: //ADD
                o += o2;
                break;
        case 1: // ABS
                o = fabs(o-o2)*2-0x8000;
                break;
        case 2: // MUL
                o *= o2*(1.0/0x4000);
                break;
        case 3: // highest amp
                if( fabs(o) < fabs(o2))
                        o = o2;
                break;
        case 4: // lowest amp
                if( fabs(o) > fabs(o2))
                        o = o2;
                break;
        case 5: // AND
                o = (int)o & (int)o2;
                break;
        case 6: // OR
                o = (int)o | (int)o2;
                break;
        case 7: // XOR
                o = (int)o ^ (int)o2;
                break;
        }
        return o + ((pwavetabsub[PhaseSub>>16]*SubOscVol)>>7);
}

inline float CTrack::VCA()
{
        // EG...
        if( !AEGCount--)
                switch( ++AEGState)
                {
                case EGS_SUSTAIN:
                        AEGCount = AEGSustainTime;
                        Amp = Volume;
                        AmpAdd = 0.0;
                        break;
                case EGS_RELEASE:
                        AEGCount = AEGReleaseTime;
                        AmpAdd = -Volume/AEGReleaseTime;
                        break;
                case EGS_RELEASE + 1:
                        AEGState = EGS_NONE;
                        AEGCount = -1;
                        Amp = 0.0;
                        break;
                }

        Amp +=AmpAdd;
        if( LFO_Amp) {
                float a =
                  Amp + (pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*LFO1Amount)/(127.0*0x8000);
                if( a<0)
                        a = 0;
                return( a);
        }
        else
                return Amp;
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
                                FEGCount = FEGSustainTime;
                                Cut = (float)FEnvMod;
                                CutAdd = 0.0;
                                break;
                        case EGS_RELEASE:
                                FEGCount = FEGReleaseTime;
                                CutAdd = ((float)-FEnvMod)/FEGReleaseTime;
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
        if( LFO_Cut)
			c = Cutoff + int(Cut) + // Cut = EnvMod
                ((pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*LFO1Amount)>>(7+8));
        else
			c = Cutoff + int(Cut); // Cut = EnvMod
        if( c < 0)
                c = 0;
        else
                if( c > 127)
                        c = 127;
        // Reso
        if( LFO_Reso) {
                r = Resonance +
                ((pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*LFO2Amount)>>(7+8));
        if( r < 0)
                r = 0;
        else
                if( r > 127)
                        r = 127;
        }
        else
                r = Resonance;


        int ofs = ((c<<7)+r)<<3;
        y = coefsTabOffs[ofs]*x +
                coefsTabOffs[ofs+1]*x1 +
                coefsTabOffs[ofs+2]*x2 +
                coefsTabOffs[ofs+3]*y1 +
                coefsTabOffs[ofs+4]*y2;

        y2=y1;
        y1=y;
        x2=x1;
        x1=x;
        return y;
}

inline void CTrack::NewPhases()
{
        if( PitchModActive) {
                if( GlideActive) {
                        if( LFO_Osc1) {
                                float pf = LFOOscTab[(pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*LFO1Amount>>7) + 0x8000];
                                Phase1 += int(PhaseAdd1*GlideFactor*PitchFactor*pf);
                                PhaseSub += int(PhaseAdd1*GlideFactor*PitchFactor*pf/2);
                        }
                        else {
							Phase1 += int(PhaseAdd1*GlideFactor*PitchFactor);
							PhaseSub += int(PhaseAdd1*GlideFactor*PitchFactor/2);
                        }
                        if( LFO_Osc2)
							Phase2 += int(PhaseAdd2*GlideFactor*PitchFactor
										  *LFOOscTab[(pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*LFO2Amount>>7) + 0x8000]);
                        else
							Phase2 += int(PhaseAdd2*GlideFactor*PitchFactor);
                        GlideFactor *= GlideMul;
                        if( !GlideCount--) {
                                GlideActive = false;
                                PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
                                PhaseAdd2 = (int)(Frequency*DetuneSemi*DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
                        }
                }
                else { // kein Glide
                        if( LFO_Osc1) {
                                float pf = LFOOscTab[(pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*LFO1Amount>>7) + 0x8000];
                                Phase1 += int(PhaseAdd1*PitchFactor*pf);
                                PhaseSub += int(PhaseAdd1*PitchFactor*pf/2);
                        }
                        else {
							Phase1 += int(PhaseAdd1*PitchFactor);
							PhaseSub += int(PhaseAdd1*PitchFactor/2);
                        }
                        if( LFO_Osc2)
							Phase2 += int(PhaseAdd2*PitchFactor
										  *LFOOscTab[(pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*LFO2Amount>>7) + 0x8000]);
                        else
							Phase2 += int(PhaseAdd2*PitchFactor);
                }

                PitchFactor *= PitchMul;

                if( !PEGCount--) {
                        if( ++PEGState == 2) {// DECAY-PHASE beginnt
                                PEGCount = PEGDecayTime;
                                PitchMul = pow( pow( 1/1.01, PEnvMod), 1.0/PEGDecayTime);
                        }
                        else  // AD-Kurve ist zu Ende
                                PitchModActive = false;
                }
        }

        else { // kein PitchMod
                if( GlideActive) {
                        if( LFO_Osc1) {
                                float pf = LFOOscTab[(pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*LFO1Amount>>7) + 0x8000];
                                Phase1 += int(PhaseAdd1*GlideFactor*pf);
                                PhaseSub += int(PhaseAdd1*GlideFactor*pf/2);
                        }
                        else {
							Phase1 += int(PhaseAdd1*GlideFactor);
							PhaseSub += int(PhaseAdd1*GlideFactor/2);
                        }
                        if( LFO_Osc2)
							Phase2 += int(PhaseAdd2*GlideFactor
										  *LFOOscTab[(pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*LFO2Amount>>7) + 0x8000]);
                        else
							Phase2 += int(PhaseAdd2*GlideFactor);
                        GlideFactor *= GlideMul;
                        if( !GlideCount--) {
                                GlideActive = false;
                                PhaseAdd1 = (int)(Frequency*pmi->TabSizeDivSampleFreq*0x10000);
                                PhaseAdd2 = (int)(Frequency*DetuneSemi*DetuneFine*pmi->TabSizeDivSampleFreq*0x10000);
                        }
                }
                else {
                        if( LFO_Osc1) {
                                float pf = LFOOscTab[(pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*LFO1Amount>>7) + 0x8000];
                                Phase1 += int(PhaseAdd1*pf);
                                PhaseSub += int(PhaseAdd1*pf/2);
                        }
                        else {
                                Phase1 += PhaseAdd1;
                                PhaseSub += PhaseAdd1/2;
                        }
                        if( LFO_Osc2)
							Phase2 += int(PhaseAdd2
										  *LFOOscTab[(pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*LFO2Amount>>7) + 0x8000]);
                        else
                                Phase2 += PhaseAdd2;
                }
        }


        if( Phase1 & 0xf8000000) { // neuer durchlauf ??
                // PW1
                if( LFO_PW1) { //LFO_PW_Mod
                        center1 = Center1 + (float)pwavetabLFO1[((unsigned)PhaseLFO1)>>21]*
                                                LFO1Amount/(127.0*0x8000);
                        if( center1 < 0)
                                center1 = 0;
                        else
                                if( center1 > 1)
                                        center1 = 1;
                }
                else  // No LFO
                        center1 = Center1;
                PhScale1A = 0.5/center1;
                PhScale1B = 0.5/(1-center1);
                center1 *= 0x8000000;
                // PW2
                if( LFO_PW2) { //LFO_PW_Mod
                        center2 = Center2 + (float)pwavetabLFO2[((unsigned)PhaseLFO2)>>21]*
                                                LFO2Amount/(127.0*0x8000);
                        if( center2 < 0)
                                center2 = 0;
                        else
                                if( center2 > 1)
                                        center2 = 1;
                }
                else  // No LFO
                        center2 = Center2;
                PhScale2A = 0.5/center2;
                PhScale2B = 0.5/(1-center2);
                center2 *= 0x8000000;

                // SYNC
                if( Sync)
                        Phase2 = Phase1; // !!!!!
        }

        Phase1 &= 0x7ffffff;
        Phase2 &= 0x7ffffff;
        PhaseSub &= 0x7ffffff;

        if( Phase1 < center1)
			Ph1 = int(Phase1*PhScale1A);
        else
			Ph1 = int((Phase1 - center1)*PhScale1B + 0x4000000);

        if( Phase2 < center2)
			Ph2 = int(Phase2*PhScale2A);
        else
			Ph2 = int((Phase2 - center2)*PhScale2B + 0x4000000);

		// LFOs
        PhaseLFO1 += PhaseAddLFO1;
        PhaseLFO2 += PhaseAddLFO2;
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
        static char txt[16];

        switch(param){
        case 2: // PW1
        case 4: // PW2
                sprintf(txt, "%u : %u", (int)(value*100.0/127),
                                                                100-(int)(value*100.0/127));
                break;
        case 5: // semi detune
                if( value == 0x40)
                        return "�0 halfnotes";
                else
                        if( value > 0x40)
                                sprintf( txt, "+%i halfnotes", value-0x40);
                        else
                                sprintf( txt, "%i halfnotes", value-0x40);
                break;
        case 6: // fine detune
                if( value == 0x40)
                        return "�0 cents";
                else
                        if( value > 0x40)
                                sprintf( txt, "+%i cents", (int)((value-0x40)*100.0/63));
                        else
                                sprintf( txt, "%i cents", (int)((value-0x40)*100.0/63));
                break;

        case 7: // Sync
                if( value == SWITCH_ON)
                        return( "on");
                else
                        return( "off");
                break;

        case 8: // MixType
                switch( value) {
                case 0: return( "add");
                case 1: return( "difference");
                case 2: return( "mul");
                case 3: return( "highest amp");
                case 4: return( "lowest amp");
                case 5: return( "and");
                case 6: return( "or");
                case 7: return( "xor");
                case 8: return( "random");
                break;
                }
        case 9: // Mix
                switch( value) {
                case 0:return "Osc1";
                case 127:return "Osc2";
                default: sprintf(txt, "%u%% : %u%%", 100-(int)(value*100.0/127),
                                                                (int)(value*100.0/127));
                }
                break;

        case 12: // Pitch Env
        case 13: // Pitch Env
        case 17: // Amp Env
        case 18: // Amp Env
        case 19: // Amp Env
        case 23: // Filter Env
        case 24: // Filter Env
        case 25: // Filter Env
                sprintf( txt, "%.4f sec", EnvTime( value)/1000);
                break;

        case 14: // PitchEnvMod
        case 26: // Filt ENvMod
                sprintf( txt, "%i", value-0x40);
                break;
        case 20:
                switch( value) {
                case 0: return( "lowpass");
                case 1: return( "highpass");
                case 2: return( "bandpass");
                case 3: return( "bandreject");
                }
                break;
        case 27: // LFO1Dest
                switch( value) {
                case 0: return "none";
                case 1: return "osc1";
                case 2: return "p.width1";
                case 3: return "volume";
                case 4: return "cutoff";
                case 5: return "osc1+pw1"; // 12
                case 6: return "osc1+volume"; // 13
                case 7: return "osc1+cutoff"; // 14
                case 8: return "pw1+volume"; // 23
                case 9: return "pw1+cutoff"; // 24
                case 10: return "vol+cutoff"; // 34
                case 11: return "o1+pw1+vol";// 123
                case 12: return "o1+pw1+cut";// 124
                case 13: return "o1+vol+cut";// 134
                case 14: return "pw1+vol+cut";// 234
                case 15: return "all";// 1234
                }
                break;
        case 31: // LFO2Dest
                switch( value) {
                case 0: return "none";
                case 1: return "osc2";
                case 2: return "p.width2";
                case 3: return "mix";
                case 4: return "resonance";

                case 5: return "osc2+pw2"; // 12
                case 6: return "osc2+mix"; // 13
                case 7: return "osc2+res"; // 14
                case 8: return "pw2+mix"; // 23
                case 9: return "pw2+res"; // 24
                case 10: return "mix+res"; // 34

                case 11: return "o2+pw2+mix"; // 123
                case 12: return "o2+pw2+res"; // 124
                case 13: return "o2+mix+res"; // 134
                case 14: return "pw2+mix+res"; // 234
                case 15: return "all"; // 1234
                }
                break;
        case 10: // SubOscWave
                if( value == 4)
                        return( "random");
        case 1: // OSC1Wave
        case 3: // OSC2Wave
        case 28: // LFO1Wave
        case 32: // LFO2Wave
                switch( value) {
                case 0: return "sine";
                case 1: return "saw";
                case 2: return "square";
                case 3: return "triangle";
                case 4: return "noise";
                case 5: return "random";
                }
                break;
        case 29: // LFO1Freq
        case 33: // LFO2Freq
                if( value <= 116)
                        sprintf( txt, "%.4f HZ", LFOFreq( value));
                else
                        sprintf( txt, "%u ticks", 1<<(value-117));
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
        TrackVals = tval;
        GlobalVals = NULL;
        AttrVals = NULL;   // attributes
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

        for( int i=0; i<MAX_TRACKS; i++)
        {
                Tracks[i].pmi = this;
                Tracks[i].Init();
        }

        // generate frequencyTab
        double freq = 16.35; //c0 bis b9
        for( int j=0; j<9; j++)
        {
                for( int i=1; i<=12; i++)
                {
                        freqTab[j*16+i] = (float)freq;
                        freq *= 1.05946309435929526; // *2^(1/12)
                }
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
        for (int i=0; i<numTracks; i++)
                Tracks[i].Tick( tval[i]);
}


bool mi::Work(float *psamples, int numsamples, int const)
{
        bool gotsomething = false;

        for ( int i=0; i<numTracks; i++) {
                if ( Tracks[i].AEGState) {
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
        return gotsomething;
}


void mi::Stop()
{
        for( int i=0; i<numTracks; i++)
                Tracks[i].Stop();
}

void mi::ComputeCoefs( float *coefs, int freq, int r, int t)
{

        float omega = 2*PI*Cutoff(freq)/pMasterInfo->SamplesPerSec;
    float sn = sin( omega);
    float cs = cos( omega);
    float alpha;
        if( t<2)
                alpha = sn / Resonance( r *(freq+70)/(127.0+70));
        else
                alpha = sn * sinh( Bandwidth( r) * omega/sn);

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
