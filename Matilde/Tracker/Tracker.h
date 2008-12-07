#ifndef	CTRACKER_H__
#define	CTRACKER_H__

#include	"Track.h"
#include	"Channel.h"
#include	"WavetableManager.h"
#include <windef.h>

#define MAX_TRACKS		16
#define	MAX_CHANNELS	64

class	CAttrVals
{
public:
	int	iVolumeRamp;
	int	iVolumeEnvelopeTicks;
	int	iMIDIChannel;
	int	iMIDIVelocity;
	int	iMIDIWave;
	int	iMIDIUsesFreeTracks;
	int	iFilterMode;
	int	iPitchEnvelopeDepth;
	int	oVirtualChannels;
	int	iLongLoopFit;					// by JM: long loop fit - multiply factor
	int iOffsetGain;					// by JM: gain volume when going towards end of sample
	int iTuningRange;
};

class	CMachine : public CMachineInterface
{
public:
										CMachine();
	virtual								~CMachine();

	virtual void						Init(CMachineDataInput * const pi);
	virtual void						Tick();
#ifdef	MONO
	virtual bool						Work(float *psamples, int numsamples, int const mode);
#endif
	virtual void						SetNumTracks(int const n);
	virtual void						Stop();

#ifndef	MONO
	virtual bool						WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);
#endif
	virtual void						Save(CMachineDataOutput * const po);
	virtual void						AttributesChanged();
	virtual void						Command(int const i);

	virtual void						MuteTrack(int const i);
	virtual bool						IsTrackMuted(int const i) const;

	virtual void						MidiNote(int const channel, int const value, int const velocity);
	virtual void						Event(dword const data);

	virtual char const				*	DescribeValue(int const param, int const value);

	virtual const CEnvelopeInfo		**	GetEnvelopeInfos();

	virtual bool						PlayWave(int const wave, int const note, float const volume);
	virtual void						StopWave();
	virtual int							GetWaveEnvPlayPos(int const env);

public:
	CWavetableManager					m_Wavetable;

	int									numTracks;
	CTrack								m_Tracks[MAX_TRACKS];
	CChannel							m_Channels[MAX_CHANNELS];

	CChannel						*	AllocChannel();

	CGlobalVals							m_GlobalValues[1];		// new by JM
	CTrackVals							m_TrackValues[MAX_TRACKS];
	CAttrVals							m_Attributes;
	int									m_iNextMIDITrack;
	int									m_iWaveTrack;
	int									m_iNextFreeChannel;

	static const CMachineInfo			m_MachineInfo;
	
	static const CEnvelopeInfo			m_PanningEnvelope;
	static const CEnvelopeInfo			m_PitchEnvelope;
	static const CEnvelopeInfo			m_VolumeEnvelope;
	static const CEnvelopeInfo		*	m_Envelopes[4];

	static const CMachineAttribute		m_attrVolumeRamp;
	static const CMachineAttribute		m_attrVolumeEnvelope;
	static const CMachineAttribute		m_attrMIDIChannel;
	static const CMachineAttribute		m_attrMIDIVelocitySensitivity;
	static const CMachineAttribute		m_attrMIDIWave;
	static const CMachineAttribute		m_attrMIDIUsesFreeTracks;
	static const CMachineAttribute		m_attrFilterMode;
	static const CMachineAttribute		m_attrPitchEnvelopeDepth;
	static const CMachineAttribute		m_attrVirtualChannels;
	static const CMachineAttribute		m_attrLongLoopFit;
	static const CMachineAttribute		m_attrOffsetGain;
	static const CMachineAttribute		m_attrTuningRange;
	static const CMachineAttribute	*	m_pAttributes[];

	static const CMachineParameter		m_paraNote;
	static const CMachineParameter		m_paraInstrument;
	static const CMachineParameter		m_paraVolume;
	static const CMachineParameter		m_paraEffect1;
	static const CMachineParameter		m_paraArgument1;
	static const CMachineParameter		m_paraEffect2;
	static const CMachineParameter		m_paraArgument2;
	static const CMachineParameter	*	m_pParameters[];

	// these are global instead of per-track
	static const CMachineParameter		m_paraAmpDecay;
	static const CMachineParameter		m_paraPercOffset;
	static const CMachineParameter		m_paraPercQuantize;
	static const CMachineParameter		m_paraTuning;
	static const CMachineParameter	*	m_pGlobalParameters[];

	bool								m_oSustainAllReleases;
	bool								m_oVirtualChannels;

	HWND								m_hDlg;
	bool								m_oDoTick;
};



#endif	//	CTRACKER_H__
