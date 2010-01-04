/*
 *		PrimiFun plug-in for Buzz
 *
 *			Written by George Nicolaidis aka Geonik
 */

#define c_strName		"PrimiFun"
#define c_strShortName	"PrimiFun"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

#include <windef.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include <MachineInterface.h>

#include "../DspClasses/DspClasses.h"
#include "../DspClasses/Adsr.h"
#include "../DspClasses/Filter.h"
#include "../DspClasses/Saturator.h"
//#include "../DspClasses/Osc.h"
#include "../DspClasses/BuzzOsc.h"

//#include "../Common/Shared.h"

typedef unsigned long  ulong;
typedef unsigned short uword;
typedef unsigned char  ubyte;

#define MaxTracks		16
#define MaxDynTracks	64
#define MinVolume		16
#define MaxAmp			32768


/*
 *		Declarations and globals
 */

class				 mi;
struct				 CTrack;

int					 g_iSampleRate;
CMachineInterface	*dspcMachine;

float				*dspcAuxBuffer;

static double const NoteFreqs[12] = { 130.8127, 138.5913, 146.8324, 155.5635, 164.8138, 174.6141, 184.9972, 195.9977, 207.6523, 220, 233.0819, 246.9417 };
static double const OctaveMul[10] = { 1.0 / 16, 1.0 / 8, 1.0 / 4, 1.0 / 2, 1.0, 1.0 * 2, 1.0 * 4, 1.0 * 8, 1.0 * 16, 1.0 * 32 };

/*
 *		Parameters
 */

CMachineParameter const  mpNote		= { pt_note,"Note","Note",NOTE_MIN,NOTE_MAX,NOTE_NO,MPF_TICK_ON_EDIT,0 };
CMachineParameter const  mpVolume	= { pt_byte,"Volume","Volume (0=0%, 80=100%)",0,0x80,0xFF,0,80 };
CMachineParameter const	 mpAttack	= { pt_byte,"Attack","Attack Time",0,120,0xFF,MPF_STATE,0x14 };
CMachineParameter const	 mpDecay	= { pt_byte,"Decay","Decay Time",0,120,0xFF,MPF_STATE,62 };
CMachineParameter const	 mpSustain	= { pt_byte,"Sustain","Sustain Level",0,0x80,0xFF,MPF_STATE,0 };
CMachineParameter const	 mpRelease	= { pt_byte,"Release","Release Time",0,120,0xFF,MPF_STATE,62 };
CMachineParameter const	 mpPWidth	= { pt_byte,"Pulse","Pulse Width",0,0x80,0xFF,MPF_STATE,0x60 };
CMachineParameter const	 mpFEnv		= { pt_byte,"Filt Env","Filter Envelope",0,0x80,0xFF,MPF_STATE,0x40 };
CMachineParameter const	 mpFRes		= { pt_byte,"Resonance","Resonance",0,0x80,0xFF,MPF_STATE,0x74 };
CMachineParameter const *mpArray[]	= { &mpNote,&mpVolume,&mpAttack,&mpDecay,&mpSustain,&mpRelease,&mpPWidth,&mpFEnv,&mpFRes };
enum					 mpValues	  { mpvNote,mpvVolume,mpvAttack,mpvDecay,mpvSustain,mpvRelease,mpvPWidth,mpvFEnv,mpvFRes };

CMachineAttribute const  maMaxDyn	= { "Dynamic Channels",0,MaxDynTracks,4 };
CMachineAttribute const  maDefVol	= { "Default Volume",0,128,128 };
CMachineAttribute const  maDynRange	= { "Track available at (dB)",-120,-30,-40 };
CMachineAttribute const *maArray[]	= { &maMaxDyn,&maDefVol,&maDynRange };

CMachineInfo const		 MacInfo = { MT_GENERATOR,MI_VERSION,0,1,MaxTracks,0,9,mpArray,3,maArray,"Geonik's " c_strName,c_strShortName,"George Nicolaidis aka Geonik","About..." };

#pragma pack(1)		

struct GlobalParameters {
	byte	Dummy; };

struct TrackParameters {
	byte	Note;
	byte	Volume;
	byte	Attack;
	byte	Decay;
	byte	Sustain;
	byte	Release;
	byte	PWidth; 
	byte	FEnv;
	byte	FRes; };

struct Attributes {
	int	 MaxDyn;
	int	 DefVol;
	int	 DynRange; };

#pragma pack()


/*
 *		Custom DSP classes
 */

/*
 *		Track class
 */

class CTrack {
public:
				 CTrack();
				~CTrack();
	void		 Tick(int);
	void		 Allocate();
	void		 Free();
	void		 Work(float *,int);
	void		 Stop();
	void		 NoteOn(int const, int const, CTrack &);
	void		 NoteOff();
	void		 Init();

	mi	*pMachine;
	CTrack		*pPlayingTrack;
	int			 iMidiNote;

	double		 rtAttack;		// Only real tracks
	double		 rtDecay;
	double		 rtRelease;
	double		 rtSustain;
	double		 rtAmplitude;

	double		 fEnvMod;
	CPwPulse	 cPulse;
	FilterQ	 cFilter;
	CSaturator	 cSaturator;
	CAdsrEnv	 cAdsr; };


class mi : public CMachineInterface {
public:
						 mi();
	virtual				~mi();

	void				 Command(int const);
	virtual void		 Init(CMachineDataInput * const pi);
	virtual void		 SetNumTracks(int const n);
	void				 MidiNote(int const channel, int const value, int const velocity);
	virtual void		 AttributesChanged();
	virtual void		 Tick();
	virtual bool		 Work(float *psamples, int numsamples, int const mode);
	virtual char const	*DescribeValue(int const param, int const value);
	virtual void		 Stop();
			void		 SetBuffer(int ms);
	CTrack				*RequestTrack(int const);
	void				 About();

	CTrack				 aTracks[MaxDynTracks];
	int					 iRealTracks;
	int					 iDynTracks;

	double				 fSilentEnough;

	//CSharedResource		 cSharedResource;
	GlobalParameters	 cGlobalParams;
	TrackParameters		 aTrackParams[MaxDynTracks];
	Attributes			 cAttributes; };

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
	case 0:
		//About();
		break; } }


void mi::Init(CMachineDataInput * const pi) {
	g_iSampleRate	= pMasterInfo->SamplesPerSec;		// Dsp Classes
	dspcMachine		= this;
	dspcAuxBuffer	= pCB->GetAuxBuffer();
	iRealTracks		= 0;								// Values
	iDynTracks		= 0;								// Waves
	for (int c = 0; c < MaxDynTracks; c++)	{			// Tracks
		aTracks[c].pMachine = this;
		aTracks[c].iMidiNote = 0;
		aTracks[c].Init(); } }

void mi::SetNumTracks(int const n) {
	if(iDynTracks < n) {
		for(int c = iDynTracks; c < n; c++)
			aTracks[c].Allocate(); }
	iRealTracks = n; iDynTracks = __max(iRealTracks,iDynTracks); }

void mi::Tick() {
    int c;
	for(c = 0; c < iDynTracks; c++)
		if(aTracks[c].cAdsr.IsPlaying() && aTracks[c].cAdsr.GetAvailability() < fSilentEnough) aTracks[c].cAdsr.Stop();
	for(c = 0; c < iRealTracks; c++)
		aTracks[c].Tick(c); }

void mi::AttributesChanged() {
	if(iDynTracks > cAttributes.MaxDyn) {
		for(int i=cAttributes.MaxDyn; i<iDynTracks; i++) {
			aTracks[i].Stop(); }
		iDynTracks = __max(iRealTracks,cAttributes.MaxDyn); } 
	fSilentEnough = pow(2.0,(double)cAttributes.DynRange/6.0); }

char const *mi::DescribeValue(int const ParamNum, int const Value) {
	static char TxtBuffer[16];
	switch(ParamNum) {
	case mpvAttack:
	case mpvDecay:
	case mpvRelease: {
		double v = pow(10.0,(double)Value*(1.0/20));
		if(v < 1000)
			sprintf(TxtBuffer,"%.1f ms", (float)v);
		else
			sprintf(TxtBuffer,"%.1f sec", (float)(v*(1.0/1000))); }
		break;
	case mpvVolume:
	case mpvSustain:
		sprintf(TxtBuffer,"%.1f%%", (float)Value * (100.0 / 128.0));
		break;
	case mpvPWidth: {
		double v = (double)(Value-64) * (100.0 / 128.0);
		if(v)
			sprintf(TxtBuffer,"%.1f : %.1f", (float)(50+v), (float)(50-v));
		else
			return "Square";
		break; }
	case mpvFEnv:
		sprintf(TxtBuffer,"%.0f", (float)Value * (1000.0 / 128.0));
		break;
	case mpvFRes:
		sprintf(TxtBuffer,"%.1f%%", (float)Value * (100.0 / 128.0));
		break;
	default:
		return NULL; }
	return TxtBuffer; }

bool mi::Work(float *pout, int ns, int const mode) {
	bool GotSomething = false;
	for (int c = 0; c < iDynTracks; c++) {
		CTrack *t = aTracks + c;
		if(t->cAdsr.IsPlaying()) {
			if(!GotSomething) {
				memset(pout,0,ns*sizeof(float));
				GotSomething = true; }
			t->Work(dspcAuxBuffer,ns);
			t->cSaturator.WorkSamplesAddDest(dspcAuxBuffer,pout,ns); } }
	return GotSomething; }

void mi::Stop() { 
	for (int c = 0; c < iDynTracks; c++) aTracks[c].Stop(); }

void mi::MidiNote(int const channel, int const value, int const velocity) {
	if(velocity == 0) {
		for(int i=0; i<iDynTracks; i++) {
			if(aTracks[i].iMidiNote != value) continue;
			aTracks[i].iMidiNote = 0;
			aTracks[i].NoteOff();
			return; }
		return; }
	aTracks[0].pPlayingTrack = RequestTrack(0);
	int oct = value / 12 - 1;
	if(oct < 0) oct = 0;
	if(oct > 9) oct = 9;
	int note = value % 12 + 1;
	aTracks[0].pPlayingTrack->iMidiNote = value;
	aTracks[0].pPlayingTrack->NoteOn(((oct << 4) + note),velocity+1,aTracks[0]); }

CTrack *mi::RequestTrack(int pt) {
	double	m = 1000.0,n;
	int		t = pt;
	for(int c=0; c < __max(iRealTracks,cAttributes.MaxDyn); c++) {
		if(c <  iRealTracks && c != pt) continue;
		if(c >= iDynTracks) { aTracks[c].Allocate(); iDynTracks++; }
		if((n=aTracks[c].cAdsr.GetAvailability()) < m) { m = n; t = c; } }
	return (t != -1 ? &aTracks[t] : aTracks); }

/*
 *		Track members
 */

CTrack::CTrack() {								// Construction
	rtAttack		= 50;
	rtDecay			= 500;
	rtRelease		= 2000;
	rtSustain		= 0.1;
	rtAmplitude		= 1.0;
	pPlayingTrack	= this; }

CTrack::~CTrack() { }							// Destruction

void CTrack::Init() {							// One time initialization
	cPulse.Init(pMachine);
	cAdsr.Init();
	cAdsr.SetAdsr(50,1000,0.8,3000); }

void CTrack::Allocate() {						// One time allocation
	}

void CTrack::Free() { 
	cAdsr.Stop(); }


void CTrack::NoteOn(int const notenum, int const vel, CTrack &trk) {
	cAdsr.SetAdsr(trk.rtAttack,trk.rtDecay,trk.rtSustain,trk.rtRelease);
	cAdsr.NoteOn();
	int		note	= (notenum & 15) - 1;
	int		oct		= notenum >> 4;
	double	freq	= NoteFreqs[note] * OctaveMul[oct];
	rtAmplitude	= (double)vel * (1.0 / 128.0);
	rtAmplitude *= rtAmplitude;
	cPulse.SetFrequency(freq);
	cPulse.SetPWidth(trk.cPulse.fPWidth);
	cPulse.Update();
	cFilter.Clear();
	cFilter.fResonance	= trk.cFilter.fResonance; 
	fEnvMod = trk.fEnvMod; }

void CTrack::NoteOff() {
	cAdsr.NoteOff(); }

void CTrack::Tick(int ThisTrack) {

	CTrack			&trk  = pMachine->aTracks[ThisTrack];
	TrackParameters &trkp = pMachine->aTrackParams[ThisTrack];

	if(trkp.Attack != mpAttack.NoValue) {
		trk.rtAttack = pow(10.0,(double)trkp.Attack*(1.0/20)); }

	if(trkp.Decay != mpDecay.NoValue) {
		trk.rtDecay = pow(10.0,(double)trkp.Decay*(1.0/20)); }

	if(trkp.Release != mpRelease.NoValue) {
		trk.rtRelease = pow(10.0,(double)trkp.Release*(1.0/20)); }

	if(trkp.Sustain != mpSustain.NoValue) {
		trk.rtSustain = (1.0 / 128.0) * (double)trkp.Sustain; }
	
	if(trkp.Note == NOTE_OFF) {
		pPlayingTrack->NoteOff(); }

	else if(trkp.Note != NOTE_NO) {
		pPlayingTrack->NoteOff();
		pPlayingTrack = pMachine->RequestTrack(ThisTrack);
		pPlayingTrack->NoteOn(trkp.Note,trkp.Volume != mpVolume.NoValue ? trkp.Volume : pMachine->cAttributes.DefVol,trk); }

	if(trkp.Volume != mpVolume.NoValue)
		pPlayingTrack->rtAmplitude = (double)(trkp.Volume * (1.0 / 128));

	if(trkp.PWidth != mpPWidth.NoValue) {
		pPlayingTrack->cPulse.fPWidth = trk.cPulse.fPWidth = (double)trkp.PWidth * (1.0 / 128);
		pPlayingTrack->cPulse.Update(); }

	if(trkp.FEnv != mpFEnv.NoValue)
		pPlayingTrack->fEnvMod = trk.fEnvMod = (double)trkp.FEnv * (1.0 / 128);

	if(trkp.FRes != mpFRes.NoValue)
		pPlayingTrack->cFilter.fResonance = trk.cFilter.fResonance = 1.0 - (double)trkp.FRes * (1.0 / 129);
}


void CTrack::Stop() {
	cAdsr.Stop(); }


#pragma optimize ("a", on)

void CTrack::Work(float *pb, int numSamples) {
	DspFPUConfigRoundDown();
	double const a	= rtAmplitude;
	double		 L  = cFilter.fLowpass;
	double		 B  = cFilter.fBandpass;
	double		 H  = cFilter.fHighpass;
	double const q  = cFilter.fResonance;
	double const fe = fEnvMod;

	short const	*ptbl	= cPulse.pSubTable;
	int const	 omask	= cPulse.iOscMask;
	int const	 dist	= cPulse.iDistance;
	double const df		= cPulse.fDistance;
	double		 pos	= cPulse.fPos;
	double		 step	= cPulse.fRate;

	while(numSamples) {

		double	lns;
		double	lgs = cAdsr.GetStep(lns);
		double	vol = cAdsr.fVolume;
		int		cnt = __min(numSamples,cAdsr.iCount);

		numSamples -= cnt;
		cAdsr.iCount -= cnt;
		do {
			int			 ipos = DspFastD2I(pos);
			double const frac = pos - ipos;
			pos += step;
			double const s1	  = ptbl[(ipos)&omask];
			double const s2   = ptbl[(ipos+1)&omask];
			double const s3	  = ptbl[(ipos+dist)&omask];
			double const s4   = ptbl[(ipos+dist+1)&omask];
			double t = vol * fe;
			L += t * B;
			H = (s1 - s3 + (s2-s1)*frac - (s4-s3)*(frac+df)) - L - q * B;
			B += t * H;
			*pb++ = (float)(L*a*vol);
			vol *= lgs;
			vol	+= lns;
		} while(--cnt);
		cAdsr.fVolume	= vol; }

	cFilter.fHighpass	= H;
	cFilter.fBandpass	= B;
	cFilter.fLowpass	= L;
	cPulse.fPos			= pos;
	DspFPUConfigReset(); }
