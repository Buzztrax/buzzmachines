#ifndef	TRACK_H__
#define	TRACK_H__

#include <MachineInterface.h>
#include	"SurfsDSPLib/SRF_DSP.h"
#include	"Envelope.h"

#define	TOTALEFFECTS			2

#pragma pack(1)		// new by JM
class	CGlobalVals
{
public:
	byte	ampdecay;
	byte	percoffset;
	byte	percquantize;
	byte	tuning;
};
#pragma pack()

#pragma pack(1)
class	CTrackVals
{
public:
	byte	note;
	byte	instrument;
	byte	volume;
	struct
	{
		byte	command, argument;
	} effects[TOTALEFFECTS];
};
#pragma pack()

class	CMachine;
class	CChannel;
class	ISample;

class	CTrack
{
public:
							CTrack();
							~CTrack();

	void					Tick( CTrackVals &tv, CGlobalVals &gv);
	void					Process( int iStep );
	void					Stop();
	void					Reset();
	/*
#ifdef	MONO
	bool					GenerateMono_Move(float *psamples, int numsamples);
	bool					GenerateMono_Add(float *psamples, int numsamples);
#else
	bool					GenerateStereo_Move(float *psamples, int numsamples);
	bool					GenerateStereo_Add(float *psamples, int numsamples);
#endif
	*/
	void					SetMute( bool oMuted ) { m_oMuted=oMuted; }
	bool					IsMute() { return m_oMuted; }

	int						NewNote( bool oRetrig=false );
	int						DoVibrato();
	int						DoAutopan();
	int						DoTremolo();
	int						DoToneport();
	int						DoCutoffLFO();
	int						DoResonanceLFO();
	int						DoVolslide( int argument );
	int						DoPanslide( int argument );

	void					ProcessRetrig( int retrig );

	int						GetWaveEnvPlayPos( const int env );

	void					Release();

public:
	CChannel			*	m_pChannel;
	CMachine			*	m_pMachine;

	ISample				*	m_pSample;

	bool					m_oAvailableForMIDI;

	int						m_iLastSample;
	int						m_iLastTick;
	int						m_iSubDivide;
	int						m_iInstrument;
	int						m_iLastCommand;
	int						m_iBaseNote;
	int						m_iLoopStretch;
	bool					m_oLoopStretchTrack;

	int						m_iAutoShuffleAmount;
	int						m_iAutoShuffleSteps;
	int						m_iAutoShuffleCount;
	bool					m_oAutoShuffled;

	int						m_iVibratoType;
	float					m_fVibratoPos;
	float					m_fVibratoSpeed;
	float					m_fVibratoDepth;

	int						m_iPanType;
	float					m_fPanPos;
	float					m_fPanSpeed;
	float					m_fPanDepth;

	int						m_iTremoloType;
	float					m_fTremoloPos;
	float					m_fTremoloSpeed;
	float					m_fTremoloDepth;

	float					m_fToneportSpeed;
	float					m_fBaseFreq;
	float					m_fWantedFreq;
	float					m_fFreq;
	float					m_fVolume;
	float					m_fBaseVolume;
	float					m_fSampleOffset;
	bool					m_oMuted;
	float					m_fBasePan;
	float					m_fPan;
//	float					m_fFinetune;
	int						m_iProbability;

	float					m_fCutoffFreq;
	float					m_fBaseCutoffFreq;
	int						m_iCutoffLFOType;
	float					m_fCutoffLFOPos;
	float					m_fCutoffLFOSpeed;
	float					m_fCutoffLFODepth;

	float					m_fResonance;
	float					m_fBaseResonance;
	int						m_iResonanceLFOType;
	float					m_fResonanceLFOPos;
	float					m_fResonanceLFOSpeed;
	float					m_fResonanceLFODepth;

	int						m_iFilterType;

	bool					m_oReverse;

	CGlobalVals				m_GVals;		// new by JM
	CTrackVals				m_Vals;

	int						m_iRandomDelay;
	bool					m_oSustained;

	// new by JM

	float					m_fFinetune;
	int						m_iDecaySpeed;
	int						m_iPercOffset;
	int						m_iPercQuantize;
};

#endif
