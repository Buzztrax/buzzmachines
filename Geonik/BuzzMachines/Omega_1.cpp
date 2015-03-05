/*
 *		Omega-1 Synth plug-in for Buzz
 *
 *			Written by George Nicolaidis aka Geonik
 */

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windef.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

typedef unsigned long  ulong;
typedef unsigned short uword;
typedef unsigned char  ubyte;

//#include "Resource.h"
#include <MachineInterface.h>

#include "../DspClasses/DspClasses.h"
#include "../DspClasses/Filter.h"
#include "../DspClasses/Delay.h"
#include "../DspClasses/Volume.h"
#include "../DspClasses/Noise.h"
#include "../DspClasses/LookUp.h"
#include "../DspClasses/Wave.h"

//#include "../Common/Shared.h"

#define c_strName			"Omega-1"
#define c_strShortName		"Omega-1"

#define MaxTracks		16
#define MaxDynTracks	64
#define BufferSize		5120
#define MaxAmp			32768


/*
 *		Declarations and globals
 */

class mi;
class CTrack;

int				g_iSampleRate;

static double const NoteFreqs[12] = { 130.8127, 138.5913, 146.8324, 155.5635, 164.8138, 174.6141, 184.9972, 195.9977, 207.6523, 220, 233.0819, 246.9417 };
static double const OctaveMul[10] = { 1.0 / 16, 1.0 / 8, 1.0 / 4, 1.0 / 2, 1.0, 1.0 * 2, 1.0 * 4, 1.0 * 8, 1.0 * 16, 1.0 * 32 };

float			AuxBuffer[MAX_BUFFER_LENGTH];


/*
 *		Parameters
 */

CMachineParameter const mpNote		= { pt_note,"Note","Note",NOTE_MIN,NOTE_MAX,NOTE_NO,MPF_TICK_ON_EDIT,0 };
CMachineParameter const mpInstr		= { pt_byte,"Instrument","Instrument number",1,4,0xFF,MPF_STATE,1 };
CMachineParameter const mpVolume	= { pt_byte,"Volume","Volume (0=0%, 80=100%, FE=198%)",0,0xFE,0xFF,0,80 };
CMachineParameter const mpC1		= { pt_byte,"Control1","Control 1 (0 to 80)",0,0x80,0xFF,MPF_STATE,0x40 };
CMachineParameter const mpC2		= { pt_byte,"Control2","Control 2 (0 to 80)",0,0x80,0xFF,MPF_STATE,0x40 };
CMachineParameter const mpC3		= { pt_byte,"Control3","Control 3 (0 to 80)",0,0x80,0xFF,MPF_STATE,0x40 };
CMachineParameter const mpReserved	= { pt_byte,"Reserved","Reserved (do not use !)",0,0x80,0xFF,0,0x00 };

CMachineParameter const *mpArray[]	= { &mpNote,&mpInstr,&mpVolume,&mpC1,&mpC2,&mpC3,&mpReserved };
enum					 mpValues	  { mpvNote,mpvInstr,mpvVolume };

#pragma pack(1)		
struct TrackParameters				  { byte Note,Instr,Volume,C1,C2,C3,Reserved; };
struct GlobalParameters { };
#pragma pack()


/*
 *		Attributes
 */

CMachineAttribute const  maMaxDyn	= { "Dynamic Channels",0,MaxDynTracks,8 };
CMachineAttribute const  maDefVol	= { "Default Volume",0,128,128 };
CMachineAttribute const  maDynRange	= { "Dynamic Range (dB)",30,120,50 };
CMachineAttribute const  maNOffAmp	= { "Note Off Amplitude (%)",0,100,35 };
CMachineAttribute const *maArray[]	= { &maMaxDyn,&maDefVol,&maDynRange,&maNOffAmp };

#pragma pack(1)		
struct Attributes					  { int MaxDyn,DefVol,DynRange,NOffAmp; };
#pragma pack()


/*
 *		CMachineInfo
 */

CMachineInfo const MacInfo = { 
	MT_GENERATOR,MI_VERSION,0,
	1,MaxTracks,0,7,mpArray,4,maArray,
	"Geonik's Omega-1","Omega-1","George Nicolaidis aka Geonik" };

enum miCommands { micAbout=0 };


/*
 *		Custom DSP classes
 */

class CCusDelay : public CDelay {
public:
	inline void WorkComb(float *,int); };


/*
 *		Track class
 */

class CTrack {
public:
					 CTrack();
					~CTrack();
	void			 Tick(int);
	void			 Allocate();
	void			 CheckIfPlaying();
	void			 Stop();
	void			 NoteOn(int const,CTrack &,TrackParameters &,int);
	void			 NoteOff();
	void			 Init();
	void			 OriginalPS	(float *,int);
	void			 TunedPS	(float *,int);
	void			 Mandolin	(float *,int);
	void			 Empty		(float *,int);

	mi		*pMachine;
	void  (CTrack:: *hookWork)(float *,int);

	CTrack			*pPlayingTrack;
	int				 iInstrument;
	byte			 bControl1;
	byte			 bControl2;
	byte			 bControl3;

	CNoise			 cNoise;
	CRms			 cRms;
	CLiDelay		 cDelay;
	CCusDelay		 cSecDelay;
	CWave			 cWave;

	double			 fVar1;

	double			 fPrevSample;
	double			 fAmplitude; };


/*
 *		Machine class
 */

class mi : public CMachineInterface {
public:
				 mi();
				~mi();
	void		 Init(CMachineDataInput * const pi);
	void		 SetNumTracks(int const n);
	void		 AttributesChanged();
	char const	*DescribeValue(int const param, int const value);
	void		 Tick();
	void		 MidiNote(int const channel, int const value, int const velocity);
	bool		 Work(float *psamples, int numsamples, int const Mode);
	void		 Stop();
	void		 Command(int const);
	void		 About();
	CTrack		*RequestTrack(int const);

	CTrack				 aTracks[MaxDynTracks];
	int					 iRealTracks;
	int					 iDynTracks;

	double				 fSilentEnough;

	CWaveBuffer			 cMandolinWave;

	//CSharedResource		 cSharedResource;
	GlobalParameters	 cGlobalParams;
	TrackParameters		 aTrackParams[MaxDynTracks];
	Attributes			 cAttributes; };


/*
 *		Instruments
 */

enum insNumbers { inOriginalPS, inTunedPS, inGrandPS, inMandolin, inEmpty, inNumOf };

void (CTrack:: *insCbTable[inNumOf])(float *,int)	= {
	&CTrack::OriginalPS,&CTrack::TunedPS,&CTrack::TunedPS,&CTrack::Mandolin,&CTrack::Empty };

const char *insNameTable[inNumOf] = { "1-OriginalPs","2-GuitarString","3-GrandPluck","4-Mandolin","5-???" };

DLL_EXPORTS

/*
 *		Machine members
 */

mi::mi() {
	GlobalVals	= &cGlobalParams;
	TrackVals	= aTrackParams;
	AttrVals	= (int *)&cAttributes; }


mi::~mi() { }


void mi::Command(int const i) {
	switch(i) {
	case micAbout:
		//About();
		break; }
 }


//#include "../Common/About.h"

// ~/bin/bin2h ./Mandpluk.sw >./Mandpluk.h
// defines short *IDR_MAND;
#include "Mandpluk.h"

void mi::Init(CMachineDataInput * const pi) {

	g_iSampleRate	= pMasterInfo->SamplesPerSec;		// Dsp Classes

	iRealTracks		= 0;								// Values
	iDynTracks		= 0;
														// Waves
	cMandolinWave.Init(IDR_MAND,8900,(0.3/MaxAmp));

	for (int c = 0; c < MaxDynTracks; c++)	{			// Tracks
		aTracks[c].pMachine = this;
		aTracks[c].Init(); } }


void mi::SetNumTracks(int const n) {
	if(iDynTracks < n) {
		for(int c = iDynTracks; c < n; c++)
			aTracks[c].Allocate(); }
	iRealTracks = n; iDynTracks = __max(iRealTracks,iDynTracks); }


char const *mi::DescribeValue(int const ParamNum, int const Value) {
	static char TxtBuffer[16];
	switch(ParamNum) {
	case mpvInstr:
		return insNameTable[Value-1];
	case mpvVolume:
		sprintf(TxtBuffer, "%.1f%%", (double)Value * (100.0 / 128.0)); break;
	default:
		return NULL; }
	return TxtBuffer; }


void mi::Tick() {
	int c;
	for(c = 0; c < iDynTracks; c++) {
		aTracks[c].CheckIfPlaying(); }

	for(c = 0; c < iRealTracks; c++)
		aTracks[c].Tick(c); }


void mi::AttributesChanged() {

	if(iDynTracks > cAttributes.MaxDyn) {
		for(int i=cAttributes.MaxDyn; i<iDynTracks; i++) {
			aTracks[i].Stop(); }
		iDynTracks = __max(iRealTracks,cAttributes.MaxDyn); }

	fSilentEnough = pow(2.0,-(double)cAttributes.DynRange/3.0); }


bool mi::Work(float *pout, int ns, int const mode) {
	bool GotSomething = false;

	for (int c = 0; c < iDynTracks; c++) {
		CTrack *t = aTracks + c;
		if(t->hookWork) {
			if(!GotSomething) {
				memset(pout,0,ns*sizeof(float));
				GotSomething = true; }
			(t->*(t->hookWork))(pout,ns); } }
	return GotSomething; }


void mi::Stop() { 
	for (int c = 0; c < iDynTracks; c++) aTracks[c].Stop(); }


void mi::MidiNote(int const channel, int const value, int const velocity) {
	if(velocity == 0) return;
//	if(aTracks[0].pPlayingTrack->bPlaying) aTracks[0].pPlayingTrack->NewNote();
	aTracks[0].pPlayingTrack = RequestTrack(0);
	unsigned int oct = value / 12 - 1;
	unsigned int note= value % 12 + 1;
	aTrackParams[0].Volume = velocity;
	aTracks[0].pPlayingTrack->NoteOn(((oct << 4) + note),aTracks[0],aTrackParams[0],0); }


CTrack *mi::RequestTrack(int pt) {
	double	m = 1000.0;
	int		t = pt;
	for(int c=0; c < __max(iRealTracks,cAttributes.MaxDyn); c++) {
		if(c <  iRealTracks && c != pt) continue;
		if(c >= iDynTracks) { aTracks[c].Allocate(); iDynTracks++; }
		if(aTracks[c].cRms.fQ < m) { m = aTracks[c].cRms.fQ; t = c; }
		if(m < fSilentEnough) break; }
	return (t != -1 ? &aTracks[t] : aTracks); }


/*
 *		Track members
 */


CTrack::CTrack() {								// Construction
	fAmplitude		= MaxAmp;
	fPrevSample		= 0;
	pPlayingTrack	= this; }


CTrack::~CTrack() { }							// Destruction


void CTrack::Init() {							// One time initialization
	 }


void CTrack::Allocate() {						// One time allocation
	cDelay.Alloc(BufferSize);
	cSecDelay.Alloc(BufferSize/2);
	cDelay.Clear();
	cSecDelay.Clear();
	cRms.Clear();
	hookWork		= NULL;
	fAmplitude		= MaxAmp;
	fPrevSample		= 0;
	pPlayingTrack	= this; }


void CTrack::CheckIfPlaying() {					// Called every tick
	if(hookWork)
		if(cRms.WorkSamples(cDelay.pBuffer,cDelay.iLength) < pMachine->fSilentEnough) {
			cRms.Clear();
			hookWork = NULL; } }


void CTrack::NoteOn(int const notenum, CTrack &trk, TrackParameters &trkp, int x) {

	if(trkp.Volume != mpVolume.NoValue)
		fAmplitude = (double)(trkp.Volume * (MaxAmp / 128));
	else
		fAmplitude		= (double)pMachine->cAttributes.DefVol * (MaxAmp / 128.0);

	int		note	= (notenum & 15) - 1;
	int		oct		= notenum >> 4;
	double	freq	= NoteFreqs[note] * OctaveMul[oct];

	double	a		= (double)trk.bControl1 * (1.0/256.0);

	fVar1			= 0.995 - a*a + (freq * 0.000005);
	fVar1			= __min(fVar1,0.99999);

	if(trk.iInstrument == inGrandPS) {
		if(x) freq /= 1.0 - ((double)trk.bControl2 + DspFastRand(4.0))*(0.01/128.0);
		else  freq *= 1.0 - ((double)trk.bControl2 + DspFastRand(4.0))*(0.01/128.0);
	}

	cDelay.SetFrequency(freq);

	int i; switch(trk.iInstrument) {
	case inOriginalPS:
		fVar1 *= 0.5;
		for(i=0; i < cDelay.iLength; i++)
			cDelay.pBuffer[i] = (float)cNoise.GetWhiteSample();
		break;
	case inTunedPS:
		for(i=0; i < cDelay.iLength; i++) {
			cDelay.pBuffer[i] = (float)cNoise.GetBlackSample(0.6); }
		break;
	case inGrandPS:
		for(i=0; i < cDelay.iLength; i++) {
			cDelay.pBuffer[i] = (float)(0.707 * cNoise.GetBlackSample(0.6)); }
		break;
	case inMandolin:
		fVar1 *= 0.96;
		cDelay.ScaleBuffer(0.60);
		cSecDelay.SetFrequency((freq / (0.01 + (double)trk.bControl2*(0.5/128.0))) * (1.0 + DspFastRand(0.05)));
		cWave.SetWave(&(pMachine->cMandolinWave));
		cWave.SetRate(0.1 + trk.bControl3 * (2.0/128.0));
		cWave.Play();
		break;
	case inEmpty:
		break;
	default:
		assert(false); }

	fPrevSample = cDelay.pBuffer[cDelay.iLength-1];
	hookWork = insCbTable[trk.iInstrument];
	cRms.Configure(10,pMachine->pMasterInfo->TicksPerSec * cDelay.iLength);
	cRms.SetRms(1.0);
	
	if(trk.iInstrument == inGrandPS && !x) {
		pMachine->RequestTrack(-1)->NoteOn(notenum,trk,trkp,1); } }


void CTrack::NoteOff() {
	cDelay.ScaleBuffer((double)pMachine->cAttributes.NOffAmp * (1.0/100.0)); }


void CTrack::Tick(int ThisTrack) {

	CTrack			&trk  = pMachine->aTracks[ThisTrack];
	TrackParameters &trkp = pMachine->aTrackParams[ThisTrack];

	if(trkp.Instr != mpInstr.NoValue) {
		trk.iInstrument = trkp.Instr - 1; }

	if(trkp.C1 != mpC1.NoValue) {
		trk.bControl1 = trkp.C1; }

	if(trkp.C2 != mpC2.NoValue) {
		trk.bControl2 = trkp.C2; }

	if(trkp.C3 != mpC3.NoValue) {
		trk.bControl3 = trkp.C3; }

	if(trkp.Note == NOTE_OFF) {
		pPlayingTrack->NoteOff(); }

	else if(trkp.Note != NOTE_NO) {
		pPlayingTrack = pMachine->RequestTrack(ThisTrack);
		pPlayingTrack->NoteOn(trkp.Note,trk,trkp,0); }

	if(trkp.Volume != mpVolume.NoValue) {
		pPlayingTrack->fAmplitude = (double)(trkp.Volume * (MaxAmp / 128)); }	}


void CTrack::Stop() { 
	NoteOff(); }


/*
 *		Worker functions
 */

/*
 *		Original Pluck-String
 */

#pragma optimize ("a", on)

void CTrack::OriginalPS(float *Dest, int numsamples) {
	double const a  = fAmplitude;
	float		*dp = cDelay.pBuffer + cDelay.iPos;
	double		 lv = fPrevSample;
	double const d  = fVar1;

	while(numsamples > 0) {
		int cnt = __min(numsamples,(cDelay.pBuffer + cDelay.iLength) - dp);
		numsamples -= cnt;
		do {
			double v = *dp;
			*dp++ = (float)(d * (v + lv));
			lv = v;
			*Dest++ += (float)(v*a);
		} while(--cnt);
		if(dp == cDelay.pBuffer + cDelay.iLength) dp = cDelay.pBuffer; }

	cDelay.iPos  = dp - cDelay.pBuffer;
	fPrevSample = lv; }

#pragma optimize ("a", off)


/*
 *		Better Pluck-String
 */

#pragma optimize ("a", on)

void CTrack::TunedPS(float *Dest, int numsamples) {
	double const a  = fAmplitude;
	float		*dp = cDelay.pBuffer + cDelay.iPos;
	double		 lv = fPrevSample;
	double const c1	= cDelay.fAlpha_1m;
	double const c2	= cDelay.fAlpha;
	double const d  = fVar1;

	while(numsamples > 0) {
		int cnt = __min(numsamples,(cDelay.pBuffer + cDelay.iLength) - dp);
		numsamples -= cnt;
		do {
			double v = *dp;
			*dp++ = (float)(d * (0.1*v + 0.9*lv));
			*Dest++ += (float)((c1*lv + c2*v)*a);
			lv=v;
		} while(--cnt);
		if(dp == cDelay.pBuffer + cDelay.iLength) dp = cDelay.pBuffer; }

	cDelay.iPos  = dp - cDelay.pBuffer;
	fPrevSample = lv; }


/*
 *		Mandolin
 */

inline void CCusDelay::WorkComb(float *pw, int ns) {
	float *dp = pBuffer + iPos;
	while(ns > 0) {
		int cnt = __min(ns,(pBuffer + iLength) - dp);
		ns -= cnt;
		do {
			double s = *dp;
			*dp++ = *pw;
			*pw++ -= (float)s;
		} while(--cnt);
		if(dp == pBuffer + iLength) dp = pBuffer; }
	iPos = dp - pBuffer; }

void CTrack::Mandolin(float *Dest, int numsamples) {
	double const a  = fAmplitude;
	float		*dp = cDelay.pBuffer + cDelay.iPos;
	double		 lv = fPrevSample;
	double const c1	= cDelay.fAlpha_1m;
	double const c2	= cDelay.fAlpha;
	double const d  = fVar1;

	if(cWave.bPlaying) {
		float *pw = AuxBuffer;
		cWave.WorkSamples(pw,numsamples);
		cSecDelay.WorkComb(pw,numsamples);
		while(numsamples > 0) {
			int cnt = __min(numsamples,(cDelay.pBuffer + cDelay.iLength) - dp);
			numsamples -= cnt;
			do {
				double v = *dp + (*pw++);
				*dp++ = (float)(d * ((0.04/0.96-0.0000000001)*v + lv));
				*Dest++ += (float)((c1*lv + c2*v)*a);
				lv=v;
			} while(--cnt);
			if(dp == cDelay.pBuffer + cDelay.iLength) dp = cDelay.pBuffer; } }
	else {
		while(numsamples > 0) {
			int cnt = __min(numsamples,(cDelay.pBuffer + cDelay.iLength) - dp);
			numsamples -= cnt;
			do {
				double v = *dp;
				*dp++ = (float)(d * ((0.04/0.96-0.0000000001)*v + lv));
				*Dest++ += (float)((c1*lv + c2*v)*a);
				lv=v;
			} while(--cnt);
			if(dp == cDelay.pBuffer + cDelay.iLength) dp = cDelay.pBuffer; } }

	cDelay.iPos  = dp - cDelay.pBuffer;
	fPrevSample = lv; }


/*
 *		Empty for future expansion
 */

void CTrack::Empty(float *pout, int numsamples) {
 }
