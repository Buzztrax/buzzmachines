#include <string.h>
#include <float.h>
#include <math.h>
#include <windows.h>
#include <commctrl.h>
#include "buzz/mdk.h"
#include "resource.h"

#define MAX_TRACKS 16
#define SEMI 1.0594630943592952645618252949461
#define EDUARD 1.0f/32768
#define EDU 0.000030517578125f

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// This basically tells us a sample isn't 16-bit.. and can be 24-bit or 32-bit
#define WF_NOT16 4

#define WFE_16BIT 0
#define WFE_32BITF 1
#define WFE_32BITI 2
#define WFE_24BIT 3

float sint[32768];
float sawt[32768];
float trit[32768];
float squt[32768];
bool tab=false;

int tclabelid[16];
int tcmuteboxid[16];
int tcsoloboxid[16];
int tcoutputid[16];

int vbasex,vbasey;

double powd(double a, double b)
{
	return pow(a,b);
}

enum
{
	WSTATE_OFF,
		WSTATE_ON,
		WSTATE_UP,
		WSTATE_DOWN,
		WSTATE_DIE
};

enum
{
	PSTATE_NO,
		PSTATE_LEFT,
		PSTATE_RIGHT
};

CMachineParameter const paraSt = {
	pt_word,
		"Stretch",
		"Stretch to x ticks",
		0x0000,
		0x8000,
		0xFFFF,
		MPF_STATE,
		0x0000
};

CMachineParameter const paraTs = {
	pt_byte,
		"Subdivide",
		"Tick subdivide",
		0x01,
		0x80,
		0xFF,
		MPF_STATE,
		0x06
};

CMachineParameter const paraDl = {
	pt_byte,
		"Delay",
		"Note Delay",
		0x01,
		0x80,
		0xFF,
		0,
		0xFF
};

CMachineParameter const paraRt = {
	pt_byte,
		"Retrigger",
		"Note Retrigger",
		0x00,
		0x80,
		0xFF,
		0,
		0xFF
};

CMachineParameter const paraNo = {
	pt_note,
		"Note",
		"Note to play",
		NOTE_MIN,
		NOTE_MAX,
		NOTE_NO,
		MPF_TICK_ON_EDIT,
		NOTE_OFF
};

CMachineParameter const paraVl = {
	pt_byte,
		"Velocity",
		"Velocity",
		0x00,
		0x80,
		0xFF,
		0,
		0x40
};

CMachineParameter const paraPa = {
	pt_byte,
		"Pan",
		"Pan",
		0x00,
		0x80,
		0xFF,
		MPF_STATE,
		0x40
};

CMachineParameter const paraSm = {
	pt_byte,
		"Sample",
		"Sample",
		0x01,
		0xE8,
		0x00,
		MPF_WAVE+MPF_STATE,
		0x01
};

CMachineParameter const paraOf = {
	pt_word,
		"Offset",
		"Offset",
		0x0000,
		0xFFFE,
		0xFFFF,
		MPF_STATE,
		0x0000
};

/*
effect parameters: word, xxyy

EFFECT COMMANDS (this list is incomplete)

	00:	Stop retriggering after xxyy times
	01:	Retrigger feedback, xxyy; C000 = 100%
	02:	Retrigger: multiply frequency by xxyy, 8000 = 100%, each time
	03:	Retrigger: Transpose by xx+yy/256 semitones, 8000 = 0, each time
	04:	Retrigger: multiply retrigger time by xxyy, 8000 = 100%, each time
	05:	Reverse direction now
	06:	Pitch down: xx semitones in yy subticks
	07:	Pitch up: xx semitones in yy subticks
	08:	Reverse, offset to be reached after xxyy subticks
	09:	Probability.
	0A:	Retrigger: add to offset on each retrigger
	0B:	Note Cut: Cut note after xxy.y subticks.
	0C:	Note End: Release note after xxy.y subticks.
	0D:	Ret NCut: Change note cut/release time on each retrigger
	0E:	Reverse, preserve reverse through retriggers
	0F:	Portamento to note, xxy.y subticks.
	11:	Set lower limit to retrigger time, xx.yy subticks
	12:	Set upper limit to retrigger time, xxyy subticks
	13:	Change retrigger time to n^(xxyy) times the original time after the nth retrigger, where 0x8000 is zero and each 0x1000 is one.
	20:	Humanize velocity.
	21:	Humanize pitch, range xx.yy semitones
	22:	Humanize offset.
	23:	Humanize pan.
	24:	Humanize cut, range xx.yy semitones
	25:	Humanize timing, range xx.yy subticks
	26:	Humanize timing, range xxyy subticks, snap.
	30:	Set filter type. LP/HP/BP/BP2/BR
	31:	Set filter cutoff.
	32:	Set filter resonance.
	33:	Cut down: xx semitones in yy subticks
	34:	Cut up: xx semitones in yy subticks
	35:	multiply filter cutoff by xxyy, 8000 = 1, on each retrigger
	36:	Change filter cutoff by xx.yy semitones, 8000 = 0, on each retrigger
	40:	Vibrato speed: xxy.y subticks
	41:	Vibrato depth
	42:	Vibrato wave. Sin/Tri/Squ/Saw
	43:	Filter LFO speed: xxy.y subticks
	44:	Filter LFO depth
	45:	Filter LFO wave. Sin/Tri/Squ/Saw
	50:	Sync Vibrato now
	51:	Sync Vibrato on notes, 0=no, 1=yes
	52:	Sync cutoff LFO now
	53:	Sync cutoff LFO on notes, 0=no, 1=yes
	60:	Jump xxyy subticks ahead, using the sample's native sample rate
	61:	Jump xxyy subticks back, using the sample's native sample rate
	62:	Jump xxyy subticks ahead, using the playback rate and Buzz' samplerate
	63:	Jump xxyy subticks back, using the playback rate and Buzz' samplerate
	70:	Save xx to yy
	71:	Load xx from yy
	78:	Set track output
	80:	Shuffle.
	81:	Shuffle.
	A0:	Reset everything.
	A1:	Reset retrigger parameters.
	A2:	Reset humanize parameters.
	A4:	Reset filters.
	A8:	Reset LFOs.
*/

CMachineParameter const paraCom1 = {
	pt_byte,
		"Command",
		"Effect Command",
		0x00,
		0xFE,
		0xFF,
		MPF_STATE,
		0x00
};

CMachineParameter const paraPar1 = {
	pt_word,
		"Parameter",
		"Effect Parameter",
		0x0000,
		0xFFFE,
		0xFFFF,
		MPF_STATE,
		0x0000
};

CMachineParameter const paraCom2 = {
	pt_byte,
		"Command",
		"Effect Command",
		0x00,
		0xFE,
		0xFF,
		MPF_STATE,
		0x00
};

CMachineParameter const paraPar2 = {
	pt_word,
		"Parameter",
		"Effect Parameter",
		0x0000,
		0xFFFE,
		0xFFFF,
		MPF_STATE,
		0x0000
};

CMachineParameter const *pParameters[]=
{
	&paraTs,
		&paraDl,
		&paraRt,
		&paraOf,
		&paraSt,
		&paraNo,
		&paraSm,
		&paraVl,
		&paraPa,
		&paraCom1,
		&paraPar1,
		&paraCom2,
		&paraPar2,
};

CMachineAttribute const attrRamp=
{
	"Amplitude Ramping (ms/10)",
		0,
		10000,
		50
};

CMachineAttribute const attrRpan=
{
	"Pan Ramping (ms/10)",
		0,
		10000,
		50
};

CMachineAttribute const attrRclk=
{
	"Declicker (ms/10)",
		0,
		10000,
		15
};

CMachineAttribute const attrInter=
{
	"Interpolation",
		0,
		2,
		2
};

CMachineAttribute const attrMCC=
{
	"MIDI command channel (0: disabled)",
		0,
		16,
		0
};

CMachineAttribute const attrMute=
{
	"MIDI mute switch start, i.e. mute switch of the first track (0: disabled)",
		0,
		96,
		0
};

CMachineAttribute const attrSolo=
{
	"MIDI solo switch start, i.e. solo switch of the first track (0: disabled)",
		0,
		96,
		0
};

CMachineAttribute const attrReset=
{
	"MIDI reset (remove all mutes/solos)",
		0,
		96,
		0
};

CMachineAttribute const attrDecLoop=
{
	"Declick at loop points? (0=off)",
		0,
		1,
		1
};

CMachineAttribute const attrMIC=
{
	"MIDI input channel (0: disabled)",
		0,
		16,
		0
};

CMachineAttribute const attrBendRange=
{
	"Pitch bend range in cents",
		0,
		4800,
		1200
};

CMachineAttribute const attrBendSpeed=
{
	"Pitch bend inertia in ms/octave",
		0,
		4000,
		20
};

CMachineAttribute const attrMT=
{
	"MIDI transpose",
		0,
		96,
		48
};

CMachineAttribute const attrPT=
{
	"Pattern transpose",
		0,
		96,
		48
};

CMachineAttribute const *pAttributes[]=
{
	&attrRamp,
		&attrRpan,
		&attrRclk,
		&attrInter,
		&attrMCC,
		&attrMute,
		&attrSolo,
		&attrReset,
		&attrDecLoop,
		&attrMIC,
		&attrBendRange,
		&attrBendSpeed,
		&attrMT,
		&attrPT
};

#pragma pack(1)

class tvals
{
public:
	byte sub;
	byte del;
	byte ret;
	word ofs;
	word str;
	byte nte;
	byte smp;
	byte vel;
	byte pan;
	byte ec1;
	word ep1;
	byte ec2;
	word ep2;
};

class avals
{
public:
	int ramp;
	int rpan;
	int rclk;
	int inter;
	int midi_control_channel;
	int midi_mute_switch_start;
	int midi_solo_switch_start;
	int midi_reset_switch;
	int decloop;
	int midi_input_channel;
	int pitch_bend_range;
	int pitch_bend_speed;
	int midi_transpose;
	int pattern_transpose;
};

#pragma pack()

/*class CEnvelopeInfo
{
public:
	char const *Name;
	int Flags;
};*/

CEnvelopeInfo const envVol=
{
	"Volume",
	0
};

CEnvelopeInfo const envPit=
{
	"Pitch",
	0
};

CEnvelopeInfo const envCut=
{
	"Cutoff",
	0
};

CEnvelopeInfo const envRes=
{
	"Resonance",
	0
};

CEnvelopeInfo const envPan=
{
	"Panning",
	0
};

CEnvelopeInfo const *pEnvs[]=
{
	&envVol,
		&envPit,
		&envCut,
		&envRes,
		&envPan,
		NULL
};

CMachineInfo const MacInfo =
{		
	MT_GENERATOR,
		MI_VERSION,
		MIF_PLAYS_WAVES+MIF_MONO_TO_STEREO,
		1,
		MAX_TRACKS,
		0,
		13,
		pParameters,
		14,
		pAttributes,
		"Fuzzpilz UnwieldyTracker",
		"UTrk",
		"Jakob Katz",
		//"&About\nSample &Pools\n&Fix\n&Test"
		"&About\nSample &Pools\n&Track Control\n&Fix"
};

inline byte buzz2midi(byte bnote);
inline byte midi2buzz(byte mnote);
int buzznotediff(int n1,int n2);
inline float abbs(float in);
void midi2text(int in,char* t);
void midi2text2(int in,char* t);
int text2midi(char* in);

inline float get24(byte *psam,int pos);
float nearst16(short *psam, int len, float pos, bool stereo, float *right);
float linear16(short *psam, int len, float pos, bool stereo, float *right);
float spline16(short *psam, int len, float pos, bool stereo, float *right);
float nearst24(short *psam, int len, float pos, bool stereo, float *right);
float linear24(short *psam, int len, float pos, bool stereo, float *right);
float spline24(short *psam, int len, float pos, bool stereo, float *right);
float nearst32i(short *psam, int len, float pos, bool stereo, float *right);
float linear32i(short *psam, int len, float pos, bool stereo, float *right);
float spline32i(short *psam, int len, float pos, bool stereo, float *right);
float nearst32f(short *psam, int len, float pos, bool stereo, float *right);
float linear32f(short *psam, int len, float pos, bool stereo, float *right);
float spline32f(short *psam, int len, float pos, bool stereo, float *right);
float (*interpolate)(short*,int,float,bool,float*);

inline int mini(int a,int b)
{
	if(a<b)return a;
	return b;
}

double fround(double f) // non-negative only
{
	if(ceil(f)==floor(f))return f;
	double g=f-(double)floor(f);
	if(g>=0.5)return (double)ceil(f);
	return (double)floor(f);
}

class EnvPoint
{
public:
	word x;
	word y;
	int flags;
};

class mi;

class miex:public CMachineInterfaceEx
{
public:
	virtual void MidiControlChange(int const ctrl,int const channel,int const value);
	mi *pmi;
};

class CEvent
{
public:
	CEvent()
	{
		cnt=0;
		pp=NULL;
		pn=NULL;
	}
	CEvent(tvals const &itv,int cn,CEvent *pprev)
	{
		cnt=cn;
		tv.del=0xFF;
		tv.ec1=itv.ec1;
		tv.ec2=itv.ec2;
		tv.ep1=itv.ep1;
		tv.ep2=itv.ep2;
		tv.nte=itv.nte;
		tv.ofs=itv.ofs;
		tv.pan=itv.pan;
		tv.ret=itv.ret;
		tv.smp=itv.smp;
		tv.str=itv.str;
		tv.sub=itv.sub;
		tv.vel=itv.vel;
		pp=pprev;
		if(pp)pp->pn=this;
		pn=NULL;
		/*char txt[2048];
		sprintf(txt,"Event added, delay: %i samples.",cnt);
		MessageBox(GetDesktopWindow(),txt,"event",0);*/
	}
	~CEvent()
	{
		if(pn)pn->pp=pp;
		if(pp)pp->pn=pn;
	}
	void step(int smp)
	{
		cnt-=smp;
		/*char txt[2048];
		sprintf(txt,"Event counter decreased by %i samples to %i.",smp,cnt);
		MessageBox(GetDesktopWindow(),txt,"event",0);*/
		if(pn)pn->step(smp);
	}
	void kill()
	{
		if(pn)pn->kill();
		delete this;
	}
	void addev(int cn,tvals const &tv)
	{
		if(pn)pn->addev(cn,tv);
		else
		{
			pn=new CEvent(tv,cn,this);
		}
	}
public:
	tvals tv;
	int cnt;
	CEvent *pp,*pn;
};

class CEventQueue
{
public:
	void step(int smp)
	{
		if(event.pn)event.pn->step(smp);
	}
	int getev(tvals &tv)
	{
		if(!event.pn)return 0;
		CEvent *pe=event.pn;
		do
		{
			if(pe->cnt<0)
			{
				tv.del=pe->tv.del;
				tv.ec1=pe->tv.ec1;
				tv.ec2=pe->tv.ec2;
				tv.ep1=pe->tv.ep1;
				tv.ep2=pe->tv.ep2;
				tv.nte=pe->tv.nte;
				tv.ofs=pe->tv.ofs;
				tv.pan=pe->tv.pan;
				tv.ret=pe->tv.ret;
				tv.smp=pe->tv.smp;
				tv.str=pe->tv.str;
				tv.sub=pe->tv.sub;
				tv.vel=pe->tv.vel;
				delete pe;
				//MessageBox(GetDesktopWindow(),"Event found.","event",0);
				return 1;
			}
			pe=pe->pn;
		}
		while(pe);
		return 0;
	}
	void clear()
	{
		if(event.pn)event.pn->kill();
	}
	void addev(int cn,tvals const &tv)
	{
		event.addev(cn,tv);
	}
public:
	CEvent event;
};

class CTrack
{
public:
	void ResRet();
	void ResHum();
	void ResFil();
	void ResLFO();
	void SaveRet(int i);
	void SaveHum(int i);
	void SaveFil(int i);
	void SaveLFO(int i);
	void LoadRet(int i);
	void LoadHum(int i);
	void LoadFil(int i);
	void LoadLFO(int i);
	void CalcCoef();
	void ClearHis();
	void VEnvPoints();
	void PEnvPoints();
	void CEnvPoints();
	void REnvPoints();
	void NEnvPoints();
	void Tick(tvals const &tv);
	void DoTick(tvals const &tv,int f);
	int Generate(float *psamples,int numsamples);
	void PlayNote(float freq,float velo,float pann);
	void PlayMIDINote(int note,int velocity);
	void StopMIDINote(int note);
	void Stop();
	void Stop2();
	float GetMul(int i);
public:
	int stopsl;
	int trn,chn;
	float tamp,oegn;
	int ncmode,ncc,ncs;
	float pncs,mncs;
	float smncs[16];
	float fstep,dir,hpan;
	float fcn;
	double afcn;
	float mafcn;
	double safcn[16];
	float smafcn[16];
	int keep_str;
	int skeep_str[16];
	float amp,pan,tpan,vel;
	float astep,pstep;
	byte smp,subdiv,wstate,pstate;
	bool stereo,offs;
	int sps,spt,sr;
	int del;
	float pfreq;
	double pdel;
	byte pnote;
	int ret;
	int stretch;
	double pfcn;
	float mdelt,mfreq,mamp,gn,pgn;
	float mcut;
	float smcut[16];
	float smdelt[16];
	float smfreq[16];
	float smamp[16];
	int imaxdel,imindel;
	int simaxdel[16];
	int simindel[16];
	float maxdel,mindel;
	float smaxdel[16];
	float smindel[16];
	float pwr;
	float spwr[16];
	int countr;
	float rfreq,ramp,rfcn,rpan,rcut;
	float srfreq[16];
	float sramp[16];
	float srfcn[16];
	float srpan[16];
	float srcut[16];
	float freqm;
	float pcut,cut,res,pres,cutm,hcutm;
	float spcut[16];
	float scut[16];
	float sres[16];
	float shcutm[16];
	int filt,cutc;
	int sfilt[16];
	float coef[5];
	float lhis[4];
	float rhis[4];
	int porca,loop;
	int vib,vspd,vwav;
	int svib[16];
	int svspd[16];
	int svwav[16];
	unsigned int vcnt;
	unsigned int svcnt[16];
	float vdep;
	float svdep[16];
	int lfo,lfospd,lfowav;
	int slfo[16];
	int slfospd[16];
	int slfowav[16];
	unsigned int lfocnt;
	unsigned int slfocnt[16];
	float lfodep;
	float slfodep[16];
	int syncv,synclfo;
	int ssyncv[16];
	int ssynclfo[16];
	float amul,adelt;
	int amulcn,adelcn;
	EnvPoint vepoint[64];
	int vesize,vecn;
	EnvPoint pepoint[64];
	int pesize,pecn;
	EnvPoint cepoint[64];
	int cesize,cecn;
	EnvPoint repoint[64];
	int resize,recn;
	EnvPoint nepoint[64];
	int nesize,necn;
	int erel;
	float fnstep,fnfcn;
	mi *pmi;
	int trmute,trsolo;
	int maxhums,maxhumsu;
	int smaxhums[16];
	int smaxhumsu[16];
	int poolused;
	float lofs,rofs,lofs2,rofs2,ofd,lpr,rpr,lprd,rprd;
	int dof,ofl;

	int shufflen; // length in ticks
	float shuffstr; // strength 0..1 in ticks
	int longsh,shortsh; // length of long and short resulting tickoids
	int shtickin; // the next tick to receive data
	int shtickout; // the next tick to occur in the shuffle
	int shcount; // counter in samples until the next tick occurs
	tvals *shticks; // shuffle tick buffer

	int wavetype; // 16 bit? 24 bit? 32 bit? 32 bit float? robots?

	CEventQueue queue;

	int acc_midi,midi_age,midi_note_playing;
	float bend,tbend;
};

// POLAC:

class OUTINFO
{
public:	
	char unused;
	float **out;
	char machineName[80];
	int numOutputs;
	char outputNames[32][80];
	OUTINFO()
	{	
		unused=0;
		out=0;
		ZeroMemory(&machineName,80);
		numOutputs=0;
		ZeroMemory(&outputNames,(32*80));		
	};	
};

// :POLAC

class mi:public CMachineInterface
{
public:
	mi();
	~mi();
	
	void Tick();
	bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);
	void Init(CMachineDataInput * const pi);
	void Save(CMachineDataOutput * const po);
	char const *mi::DescribeValue(int const param, int const value);
	void DoTrackM();
	void UpdateSList(HWND shandle,int *sampnum,int *sampnum2);
	void RCSum();
	void CheckWaves();
	void SetNumTracks(int const n)
	{
		numTracks=n;
		for(int i=0;i<32;i++)ZeroMemory(outbuf[i],MAX_BUFFER_LENGTH*sizeof(float));
		if(!lock2)if(!trackmlock)DoTrackM();
	}
	void Command(int const i);
	void Stop();
	inline float note2freq(int note,int srate);
	int GetRndSamp(int pool);
	CEnvelopeInfo const **GetEnvelopeInfos()
	{
		return pEnvs;
	}
	virtual bool PlayWave(int const wave, int const note, float const volume)
	{
		tval[0].smp=wave;
		tval[0].nte=note;
		Tracks[0].Tick(tval[0]);
		return true;
	}
	virtual void StopWave()
	{
		tval[0].nte=NOTE_OFF;
		Tracks[0].Tick(tval[0]);
	}
	int GetWaveEnvPlayPos(int const env)
	{
		switch(env)
		{
		case 0:return mini(Tracks[0].vecn,65535);break;
		case 1:return mini(Tracks[0].pecn,65535);break;
		case 2:return mini(Tracks[0].cecn,65535);break;
		case 3:return mini(Tracks[0].recn,65535);break;
		case 4:return mini(Tracks[0].necn,65535);break;
		default:return 0;break;
		}
	}
	void CreateOutBufs();
	void DeleteOutBufs();
	void MidiNote(int const channel,int const value,int const velocity);
	void AttributesChanged();
public:
	CMachine *ThisMachine;
	int numTracks;
	CTrack Tracks[MAX_TRACKS];
	tvals tval[MAX_TRACKS];
	avals aval;
	float astep,pstep,ofd;
	float *outbuf[32];
	OUTINFO outInfo;
	int trmchan[MAX_TRACKS];
	byte trmmute[MAX_TRACKS];
	HWND dhandle; // about dialogue
	HWND thandle; // track control dialogue
	int lastpoolnum; // number of sample pool to edit
	int smpool[32][200];
	int smpsum[32];
	char poolname[32][256];
	bool tchk;
	int ofl;
	bool muted;
	bool trackmlock,lock2;

	float bendm,dbend;
	int pbs;

	miex ex;
};

//////////////////////////////////////////////////////////////////////////////////
// POLAC /////////////////////////////////////////////////////////////////////////

#define POLAC 0

#define MACH_WANT_MIDI 0
#define MACH_MULTI_OUT 1
#define MACH_WINDOW 2
#define MACH_PATTERN_CHANGE 3

typedef int(*MACHINECALLBACK)(CMachineInterface*,int,int,int,float,void*);

void M_LoadMachines(void);
void M_FreeMachines(void);
bool M_IsActive(void);
void M_Load(CMachineDataInput *pi,GUID *machID);
void M_Save(CMachineDataOutput *po,GUID *machID);

// for synchronisation purpose
void M_Lock();
void M_Unlock();

int MachineCallback(CMachineInterface* pMI,int opCode,int iVal,int iVal2,float fVal,void *pV);
void M_Offer(int opcode,CMachineInterface* pMI,FARPROC callback,CMachine* pM,CMachineInfo *pInfo,void* pOpt);
int M_Deoffer(int opcode,CMachineInterface* pMI);
int M_getListIndex(int n,int opcode,CMachineInterface** ppMI,FARPROC* pCallback,CMachine** ppM,CMachineInfo **ppInfo,void** ppOpt);
int M_findListElement(int opcode,CMachineInterface* pMI,FARPROC* pCallback,CMachine** ppM,CMachineInfo **ppInfo,void** ppOpt);

int							M_loaded=0;
HMODULE						hm_dock=0;
int							mcount=0;

extern int midi_in_mode;


void (__cdecl *Offer)(int opcode, CMachineInterface* pMI, FARPROC callback, CMachine* pM, CMachineInfo *pInfo, void* pOpt);
int (__cdecl *Deoffer)(int opcode, CMachineInterface* pMI);
int (__cdecl *getListIndex)(int n, int opcode, CMachineInterface** ppMI, FARPROC* pCallback, CMachine** ppM, CMachineInfo **ppInfo, void** ppOpt);
int (__cdecl *findListElement)(int opcode, CMachineInterface* pMI, FARPROC* pCallback, CMachine** ppM, CMachineInfo **ppInfo, void** ppOpt);
void (__cdecl *Lock)(void);
void (__cdecl *Unlock)(void);


int MachineCallback(CMachineInterface* pMI,int opcode,int iVal,int iVal2,float fVal,void *pV)
{
	if(HIWORD(opcode)==POLAC)
	{
		switch(LOWORD(opcode))
		{
		case MACH_MULTI_OUT:
			mi* p=(mi*)pMI;
			if (iVal)
			{
				void** ppV=(void**)iVal;
				*ppV=(void**)&p->outInfo;
				return 1;
			}
			break;
		}
	}	
	return -1;
}

void M_Lock()
{
	if(M_loaded)Lock();
}

void M_Unlock()
{
	if(M_loaded)Unlock();
}

int M_getListIndex(int n,int opcode,CMachineInterface** ppMI,FARPROC* pCallback,CMachine** ppM,CMachineInfo **ppInfo,void** ppOpt)
{
	if(M_loaded)return getListIndex(n,opcode,ppMI,pCallback,ppM,ppInfo,ppOpt);
	return -1;
}

int M_findListElement(int opcode,CMachineInterface* pMI,FARPROC* pCallback,CMachine** ppM,CMachineInfo **ppInfo,void** ppOpt)
{
	if(M_loaded)return findListElement(opcode,pMI,pCallback,ppM,ppInfo,ppOpt);
	return -1;
}

void M_Offer(int opcode,CMachineInterface* pMI,FARPROC callback,CMachine* pM,CMachineInfo *pInfo,void* pOpt)
{
	if(M_loaded)Offer(opcode,pMI,callback,pM,pInfo,pOpt);
}

int M_Deoffer(int opcode,CMachineInterface* pMI)
{
	if(M_loaded)return Deoffer(opcode,pMI);
	return -1;
}

bool M_IsActive()
{
	if(M_loaded)return true;
	return false;
}

void M_Save(CMachineDataOutput *po,GUID *machID)
{
	po->Write((void *)machID,sizeof(GUID));
}

void M_Load(CMachineDataInput *pi,GUID *machID)
{
	pi->Read((void *)machID,sizeof(GUID));
}

void M_LoadMachines()
{
	mcount++;
	if(mcount==1)
	{
		char buzz_dock_dll[MAX_PATH];
		buzz_dock_dll[0]=0;
		HKEY hk;
		if(RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Jeskola\\Buzz\\Settings",0, KEY_QUERY_VALUE,&hk)==ERROR_SUCCESS)
		{
			DWORD dwType;
			DWORD dwSize=MAX_PATH;
			if(RegQueryValueEx(hk,"BuzzPath",NULL,&dwType,(BYTE *)buzz_dock_dll,&dwSize)==ERROR_SUCCESS)
			{
				int len=strlen(buzz_dock_dll);
				if(buzz_dock_dll[len-1]!='\\')
				{
					buzz_dock_dll[len]='\\';
					buzz_dock_dll[len+1]=0;
				}
				strcat(buzz_dock_dll,"Gear\\Machines.dll");
			}
			RegCloseKey(hk);
		}
		hm_dock=LoadLibrary(buzz_dock_dll);
		if(hm_dock)
		{
			Offer=(void(__cdecl *)(int opcode,CMachineInterface* pMI,FARPROC callback,CMachine* pM,CMachineInfo *pInfo,void* pOpt))GetProcAddress(hm_dock,"Offer");
			Deoffer=(int(__cdecl *)(int opcode,CMachineInterface* pMI))GetProcAddress(hm_dock,"Deoffer");
			Lock=(void(__cdecl *)(void))GetProcAddress(hm_dock,"Lock");
			Unlock=(void(__cdecl *)(void))GetProcAddress(hm_dock,"Unlock");
			getListIndex=(int(__cdecl *)(int n,int opcode,CMachineInterface** ppMI,FARPROC* pCallback,CMachine** ppM,CMachineInfo **ppInfo,void** ppOpt))GetProcAddress(hm_dock,"getListIndex");
			findListElement=(int(__cdecl *)(int opcode,CMachineInterface* pMI,FARPROC* pCallback,CMachine** ppM,CMachineInfo **ppInfo,void** ppOpt))GetProcAddress(hm_dock,"findListElement");
			if(Offer&&Deoffer&&Lock&&Unlock&&getListIndex&&findListElement)
			{
				M_loaded=1;
				return;
			}
		}
		M_loaded=0;
	}
}



void M_FreeMachines()
{
	mcount--;
	if(!mcount)
	{
		if(hm_dock)
		{
			FreeLibrary(hm_dock);
			hm_dock=0;
		}
	}
}

// /POLAC /////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

DLL_EXPORTS

// ==========================================================================================

// ==========================================================================================

class milist
{
	public:
		mi *thismi;
		HWND dhandle;
		HWND thandle;
};

static milist *tehmilist=NULL;
int micount=0;

milist *fimili(void *ptr)
{
	int i=0; 
	while(i<micount)
	{
		if(tehmilist[i].thismi==ptr||tehmilist[i].dhandle==ptr||tehmilist[i].thandle==ptr)return &tehmilist[i];
		i++;
	}
	return NULL;
}

HINSTANCE dllInstance;

BOOL WINAPI DllMain(HANDLE hModule,DWORD fwdreason,LPVOID lpReserved)
{
	InitCommonControls();

	tclabelid[0]=IDC_L1;
	tclabelid[1]=IDC_L2;
	tclabelid[2]=IDC_L3;
	tclabelid[3]=IDC_L4;
	tclabelid[4]=IDC_L5;
	tclabelid[5]=IDC_L6;
	tclabelid[6]=IDC_L7;
	tclabelid[7]=IDC_L8;
	tclabelid[8]=IDC_L9;
	tclabelid[9]=IDC_L10;
	tclabelid[10]=IDC_L11;
	tclabelid[11]=IDC_L12;
	tclabelid[12]=IDC_L13;
	tclabelid[13]=IDC_L14;
	tclabelid[14]=IDC_L15;
	tclabelid[15]=IDC_L16;
	tcmuteboxid[0]=IDC_MUTE1;
	tcmuteboxid[1]=IDC_MUTE2;
	tcmuteboxid[2]=IDC_MUTE3;
	tcmuteboxid[3]=IDC_MUTE4;
	tcmuteboxid[4]=IDC_MUTE5;
	tcmuteboxid[5]=IDC_MUTE6;
	tcmuteboxid[6]=IDC_MUTE7;
	tcmuteboxid[7]=IDC_MUTE8;
	tcmuteboxid[8]=IDC_MUTE9;
	tcmuteboxid[9]=IDC_MUTE10;
	tcmuteboxid[10]=IDC_MUTE11;
	tcmuteboxid[11]=IDC_MUTE12;
	tcmuteboxid[12]=IDC_MUTE13;
	tcmuteboxid[13]=IDC_MUTE14;
	tcmuteboxid[14]=IDC_MUTE15;
	tcmuteboxid[15]=IDC_MUTE16;
	tcsoloboxid[0]=IDC_SOLO1;
	tcsoloboxid[1]=IDC_SOLO2;
	tcsoloboxid[2]=IDC_SOLO3;
	tcsoloboxid[3]=IDC_SOLO4;
	tcsoloboxid[4]=IDC_SOLO5;
	tcsoloboxid[5]=IDC_SOLO6;
	tcsoloboxid[6]=IDC_SOLO7;
	tcsoloboxid[7]=IDC_SOLO8;
	tcsoloboxid[8]=IDC_SOLO9;
	tcsoloboxid[9]=IDC_SOLO10;
	tcsoloboxid[10]=IDC_SOLO11;
	tcsoloboxid[11]=IDC_SOLO12;
	tcsoloboxid[12]=IDC_SOLO13;
	tcsoloboxid[13]=IDC_SOLO14;
	tcsoloboxid[14]=IDC_SOLO15;
	tcsoloboxid[15]=IDC_SOLO16;
	tcoutputid[0]=IDC_OUTPUT1;
	tcoutputid[1]=IDC_OUTPUT2;
	tcoutputid[2]=IDC_OUTPUT3;
	tcoutputid[3]=IDC_OUTPUT4;
	tcoutputid[4]=IDC_OUTPUT5;
	tcoutputid[5]=IDC_OUTPUT6;
	tcoutputid[6]=IDC_OUTPUT7;
	tcoutputid[7]=IDC_OUTPUT8;
	tcoutputid[8]=IDC_OUTPUT9;
	tcoutputid[9]=IDC_OUTPUT10;
	tcoutputid[10]=IDC_OUTPUT11;
	tcoutputid[11]=IDC_OUTPUT12;
	tcoutputid[12]=IDC_OUTPUT13;
	tcoutputid[13]=IDC_OUTPUT14;
	tcoutputid[14]=IDC_OUTPUT15;
	tcoutputid[15]=IDC_OUTPUT16;

	switch(fwdreason)
	{
	case DLL_PROCESS_ATTACH:
		dllInstance=(HINSTANCE)hModule;
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	case DLL_PROCESS_DETACH:break;
	}
	return TRUE;
}

mi *poolmi=NULL;
int sampnum[200];
int sampnum2[4096];
bool focs=false;
bool focp=false;
int snum=-1;

BOOL APIENTRY PoolDialog(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	mi *pmi=(mi *)lParam;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		if(pmi)
		{
			pmi->UpdateSList(hDlg,sampnum,sampnum2);
			poolmi=pmi;
			SetDlgItemInt(hDlg,IDC_POOLNUM,pmi->lastpoolnum,1);
			HWND sld=GetDlgItem(hDlg,IDC_SPIN);
			SendMessage(sld,TBM_SETRANGE,0,MAKELONG(0,31));
			SendMessage(sld,TBM_SETPOS,0,pmi->lastpoolnum);
			SetDlgItemText(hDlg,IDC_POOLNAME,pmi->poolname[pmi->lastpoolnum]);
			snum=-1;
			SetDlgItemText(hDlg,IDC_SNAME,"");
			SetDlgItemText(hDlg,IDC_SAMPLERATE,"");
			SetDlgItemText(hDlg,IDC_BITS,"");
			SetDlgItemText(hDlg,IDC_MS,"");
			SetDlgItemText(hDlg,IDC_LENGTH,"");
		}
		return 1;
	case WM_CLOSE:
		poolmi=NULL;
		EndDialog(hDlg,0);
		return 0;
	case WM_NOTIFY:
		if((wParam==IDC_SPIN)&&(((NMUPDOWN *)lParam)->hdr.code==UDN_DELTAPOS))
		{
			//int a=((NMUPDOWN *)lParam)->iPos-((NMUPDOWN *)lParam)->iDelta;
			int a=poolmi->lastpoolnum-((NMUPDOWN *)lParam)->iDelta;
			if(poolmi)
			{
				poolmi->lastpoolnum=a;
				SetDlgItemInt(hDlg,IDC_POOLNUM,a,1);
				poolmi->UpdateSList(hDlg,sampnum,sampnum2);
				SetDlgItemText(hDlg,IDC_POOLNAME,poolmi->poolname[poolmi->lastpoolnum]);
				return 1;
			}
			return 0;
		}
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(HIWORD(wParam)==BN_CLICKED)
			{
				poolmi=NULL;
				EndDialog(hDlg,0);
			}
			return 1;
		case IDC_POOLNUM:
			if(HIWORD(wParam)==EN_CHANGE)
			{
				snum=-1;
				SetDlgItemText(hDlg,IDC_SNAME,"");
				SetDlgItemText(hDlg,IDC_SAMPLERATE,"");
				SetDlgItemText(hDlg,IDC_BITS,"");
				SetDlgItemText(hDlg,IDC_MS,"");
				SetDlgItemText(hDlg,IDC_LENGTH,"");
				if(poolmi)
				{
					int a=GetDlgItemInt(hDlg,IDC_POOLNUM,NULL,1);
					if(a<0)
					{
						SetDlgItemInt(hDlg,IDC_POOLNUM,0,1);
						a=0;
					}
					if(a>31)
					{
						SetDlgItemInt(hDlg,IDC_POOLNUM,31,1);
						a=31;
					}
					poolmi->lastpoolnum=a;
					poolmi->UpdateSList(hDlg,sampnum,sampnum2);
				}
			}
			return 1;
		case IDC_POOLNAME:
			if(HIWORD(wParam)==EN_CHANGE)
			{
				if(poolmi)
				{
					char newtext[256];
					GetDlgItemText(hDlg,IDC_POOLNAME,newtext,255);
					memcpy(poolmi->poolname[poolmi->lastpoolnum],newtext,256);
				}
			}
			return 1;
		case IDC_ADD:
			if(poolmi&&(HIWORD(wParam)==BN_CLICKED))
			{
				int a=poolmi->lastpoolnum;

				HWND hSamps=GetDlgItem(hDlg,IDC_SAMPLES);

				if(hSamps)
				{
					int *seld=new int[200];
					int seln=SendMessage(hSamps,LB_GETSELITEMS,200,(long)seld);
					
					for(int c=0;c<seln;c++)
					{
						poolmi->smpool[a][sampnum[seld[c]]]++;
					}
					delete seld;
				}
				
				poolmi->UpdateSList(hDlg,sampnum,sampnum2);
				break;
			}
			return 1;
		case IDC_REMOVE:
			if(poolmi&&(HIWORD(wParam)==BN_CLICKED))
			{
				int a=poolmi->lastpoolnum;
				
				HWND hPool=GetDlgItem(hDlg,IDC_INPOOL);
				if(hPool)
				{
					int *seld=new int[200];
					int seln=SendMessage(hPool,LB_GETSELITEMS,200,(long)seld);
					for(int b=0;b<seln;b++)
					{
						//SendMessage(hPool,LB_DELETESTRING,seld[b],0);
						for(int c=b+1;c<seln;c++)if(seld[c]>seld[b])seld[c]--;
						poolmi->smpool[a][sampnum2[seld[b]]]--;
						for(c=seld[b];c<4095;c++)sampnum2[c]=sampnum2[c+1];sampnum2[4095]=-1;
					}
					delete seld;
				}
				poolmi->UpdateSList(hDlg,sampnum,sampnum2);
			}
			return 1;
		case IDC_CLEAR:
			if(poolmi&&(HIWORD(wParam)==BN_CLICKED))
			{
				int a=poolmi->lastpoolnum;
				
				HWND hPool=GetDlgItem(hDlg,IDC_INPOOL);
				if(hPool)
				{
					int seln=SendMessage(hPool,LB_RESETCONTENT,0,0);
					if(focp)
					{
						snum=-1;
						SetDlgItemText(hDlg,IDC_SNAME,"");
						SetDlgItemText(hDlg,IDC_SAMPLERATE,"");
						SetDlgItemText(hDlg,IDC_BITS,"");
						SetDlgItemText(hDlg,IDC_MS,"");
						SetDlgItemText(hDlg,IDC_LENGTH,"");
					}
					for(int i=0;i<200;i++)
					{
						poolmi->smpool[poolmi->lastpoolnum][i]=0;
					}
				}
				poolmi->RCSum();
			}
			return 1;
		case IDC_SAMPLES:
			if(HIWORD(wParam)==LBN_SETFOCUS)
			{
				focs=true;
			}
			else if((HIWORD(wParam)==LBN_SELCHANGE)&&focs)
			{
				snum=-1;
				SetDlgItemText(hDlg,IDC_SNAME,"");
				SetDlgItemText(hDlg,IDC_SAMPLERATE,"");
				SetDlgItemText(hDlg,IDC_BITS,"");
				SetDlgItemText(hDlg,IDC_MS,"");
				SetDlgItemText(hDlg,IDC_LENGTH,"");
				if(poolmi)
				{
					HWND lbhandle=(HWND)lParam;
					int snum2=SendMessage(lbhandle,LB_GETCARETINDEX,0,0)+1;
					for(int i=0;i<200;i++)
					{
						CWaveLevel const *pwave=poolmi->pCB->GetWaveLevel(i+1,0);
						if(pwave)
						{
							snum=i;
							snum2--;
							if(snum2<=0)break;
						}
					}
					CWaveLevel const *pwave=poolmi->pCB->GetWaveLevel(snum+1,0);
					if(pwave)
					{
						CWaveInfo const *pinfo=poolmi->pCB->GetWave(snum+1);
						int flags=pinfo->Flags;
						static char text[64];
						sprintf(text,"%i. %s",snum+1,poolmi->pCB->GetWaveName(snum+1));
						SetDlgItemText(hDlg,IDC_SNAME,text);
						sprintf(text,"%i Hz",pwave->SamplesPerSec);
						SetDlgItemText(hDlg,IDC_SAMPLERATE,text);
						int samp=pwave->numSamples;
						if(flags&WF_NOT16)
						{
							samp-=4;
							switch(pwave->pSamples[0])
							{
							case WFE_24BIT:SetDlgItemText(hDlg,IDC_BITS,"24 bit");samp=samp*2/3;break;
							case WFE_32BITI:SetDlgItemText(hDlg,IDC_BITS,"32 bit");samp=samp>>1;break;
							case WFE_32BITF:SetDlgItemText(hDlg,IDC_BITS,"32 bit float");samp=samp>>1;break;
							default:SetDlgItemText(hDlg,IDC_BITS,"Unknown");break;
							}
						}
						else SetDlgItemText(hDlg,IDC_BITS,"16 bit");
						SetDlgItemText(hDlg,IDC_MS,(flags&WF_STEREO)?"Stereo":"Mono");
						sprintf(text,"%.2f sec",(float)samp/(pwave->SamplesPerSec));
						SetDlgItemText(hDlg,IDC_LENGTH,text);
					}
				}
			}
			else if(HIWORD(wParam)==LBN_KILLFOCUS)
			{
				focs=false;
			}
			else if(HIWORD(wParam)==LBN_DBLCLK)
			{
				if(poolmi&&(snum>=0))
				{
					poolmi->PlayWave(snum+1,0x41,1);
				}
			}
			return 1;
		case IDC_INPOOL:
			if(HIWORD(wParam)==LBN_SETFOCUS)
			{
				focp=true;
			}
			else if((HIWORD(wParam)==LBN_SELCHANGE)&&focp)
			{
				snum=-1;
				SetDlgItemText(hDlg,IDC_SNAME,"");
				SetDlgItemText(hDlg,IDC_SAMPLERATE,"");
				SetDlgItemText(hDlg,IDC_BITS,"");
				SetDlgItemText(hDlg,IDC_MS,"");
				SetDlgItemText(hDlg,IDC_LENGTH,"");
				HWND lbhandle=(HWND)lParam;
				if((poolmi)&&(SendMessage(lbhandle,LB_GETCOUNT,0,0)))
				{
					snum=sampnum2[SendMessage(lbhandle,LB_GETCARETINDEX,0,0)&0x0FFF];
					CWaveLevel const *pwave=poolmi->pCB->GetWaveLevel(snum+1,0);
					if(pwave)
					{
						CWaveInfo const *pinfo=poolmi->pCB->GetWave(snum+1);
						int flags=pinfo->Flags;
						static char text[64];
						sprintf(text,"%i. %s",snum+1,poolmi->pCB->GetWaveName(snum+1));
						SetDlgItemText(hDlg,IDC_SNAME,text);
						sprintf(text,"%i Hz",pwave->SamplesPerSec);
						SetDlgItemText(hDlg,IDC_SAMPLERATE,text);
						int samp=pwave->numSamples;
						if(flags&WF_NOT16)
						{
							samp-=4;
							switch(pwave->pSamples[0])
							{
							case WFE_24BIT:SetDlgItemText(hDlg,IDC_BITS,"24 bit");samp=samp*2/3;break;
							case WFE_32BITI:SetDlgItemText(hDlg,IDC_BITS,"32 bit");samp=samp>>1;break;
							case WFE_32BITF:SetDlgItemText(hDlg,IDC_BITS,"32 bit float");samp=samp>>1;break;
							default:SetDlgItemText(hDlg,IDC_BITS,"Unknown");break;
							}
						}
						else SetDlgItemText(hDlg,IDC_BITS,"16 bit");
						SetDlgItemText(hDlg,IDC_MS,(flags&WF_STEREO)?"Stereo":"Mono");
						sprintf(text,"%.2f sec",(float)samp/(pwave->SamplesPerSec));
						SetDlgItemText(hDlg,IDC_LENGTH,text);
					}
				}
			}
			else if(HIWORD(wParam)==LBN_KILLFOCUS)
			{
				focp=false;
			}
			else if(HIWORD(wParam)==LBN_DBLCLK)
			{
				if(poolmi&&(snum>=0))
				{
					poolmi->PlayWave(snum+1,0x41,1);
				}
			}
		case IDC_PREVIEWPLAY:
			if(poolmi&&(HIWORD(wParam)==BN_CLICKED))
			{
				if(snum>=0)
				{
					poolmi->PlayWave(snum+1,0x41,1);
				}
			}
			return 1;
		}
		return 1;
	}
	
	return 0;
}

BOOL APIENTRY TrackControlDialog(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	mi *pmi=NULL;
	if(tehmilist!=NULL)
	{
		milist *tmp=fimili(hDlg);
		if(tmp)pmi=tmp->thismi;
	}

	//if(pmi)if(pmi->trackmlock)return 0;

	switch(uMsg)
	{
	case WM_CLOSE:
		ShowWindow(hDlg,SW_HIDE);
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			ShowWindow(hDlg,SW_HIDE);
			return 1;
		}
		if(pmi!=NULL)
		{
			for(int i=0;i<pmi->numTracks;i++)
			{
				if(LOWORD(wParam)==tcmuteboxid[i])
				{
					if(HIWORD(wParam)==BN_CLICKED)
					{
						if(pmi->Tracks[i].trmute==0)
						{
							pmi->Tracks[i].trmute=1;
						}
						else
						{
							pmi->Tracks[i].trmute=0;
						}
						pmi->DoTrackM();
					}
				}
				else if(LOWORD(wParam)==tcsoloboxid[i])
				{
					if(HIWORD(wParam)==BN_CLICKED)
					{
						pmi->Tracks[i].trmute=0;
						if(pmi->Tracks[i].trsolo==0)
						{
							pmi->Tracks[i].trsolo=1;
							for(int j=0;j<pmi->numTracks;j++)
							{
								if(j!=i)
								{
									if(pmi->Tracks[j].trsolo)pmi->Tracks[j].trsolo=0;
									if(pmi->Tracks[j].trmute==0)pmi->Tracks[j].trmute=2;
								}
							}
						}
						else
						{
							pmi->Tracks[i].trsolo=0;
							for(int j=0;j<pmi->numTracks;j++)
							{
								pmi->Tracks[j].trsolo=0;
								if(pmi->Tracks[j].trmute==2)pmi->Tracks[j].trmute=0;
							}
						}
						pmi->DoTrackM();
					}
				}
				else if(LOWORD(wParam)==tcoutputid[i])
				{
					if(HIWORD(wParam)==EN_CHANGE)
					{
						int chn=GetDlgItemInt(hDlg,tcoutputid[i],NULL,0);
						if(chn>15)chn=15;
						else if(chn<0)chn=0;
						if(chn!=pmi->Tracks[i].chn)
						{
							pmi->Tracks[i].chn=chn;
							SetDlgItemInt(hDlg,tcoutputid[i],chn,0);
						}
					}
				}
			}
		}
		break;
	}

	return 0;
}

BOOL APIENTRY AboutDialog(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	mi *pmi=NULL;
	if(tehmilist!=NULL)
	{
		milist *tmp=fimili(hDlg);
		if(tmp)pmi=tmp->thismi;
	}

	//if(pmi)if(pmi->trackmlock)return 0;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		char infotext[10000];
		sprintf(infotext,"UnwieldyTracker is a sampler for Buzz.\r\n\r\n");
		sprintf(infotext,"%sParameters, in order:\r\n",infotext);
		sprintf(infotext,"%s- Tick subdivide\r\n",infotext);
		sprintf(infotext,"%s- Note delay\r\n",infotext);
		sprintf(infotext,"%s- Note retrigger (value 00 resets the note to its normal position and pitch)\r\n",infotext);
		sprintf(infotext,"%s- Offset\r\n",infotext);
		sprintf(infotext,"%s- Stretch (leave this alone or set it to 0 if you want to use notes instead of stretching)\r\n",infotext);
		sprintf(infotext,"%s- Note\r\n",infotext);
		sprintf(infotext,"%s- Sample\r\n",infotext);
		sprintf(infotext,"%s- Velocity (0x40 means 100%)\r\n",infotext);
		sprintf(infotext,"%s- Pan (0x40 means center)\r\n",infotext);
		sprintf(infotext,"%s- Effect Command 1\r\n",infotext);
		sprintf(infotext,"%s- Effect Data 1\r\n",infotext);
		sprintf(infotext,"%s- Effect Command 2\r\n",infotext);
		sprintf(infotext,"%s- Effect Data 2\r\n\r\n",infotext);
		sprintf(infotext,"%sAttributes:\r\n",infotext);
		sprintf(infotext,"%s- Length of amplitude/pan ramping and the strength of the declicker\r\n",infotext);
		sprintf(infotext,"%s- Sample interpolation, where 0=none, 1=linear, 2=5-point spline\r\n",infotext);
		sprintf(infotext,"%s- MIDI control channel\r\n",infotext);
		sprintf(infotext,"%s- MIDI mute switch note for first track (the other tracks' start from there)\r\n",infotext);
		sprintf(infotext,"%s- MIDI solo switch note for first track (the other tracks' start from there)\r\n",infotext);
		sprintf(infotext,"%s- MIDI mute/solo reset switch note.\r\n",infotext);
		sprintf(infotext,"%s- Declick at loop points? Leaving this on can cause problems with small loops.\r\n\r\n",infotext);
		sprintf(infotext,"%sUTrk supports 16 bit, 24 bit, 32 bit and 32 bit float samples. ",infotext);
		sprintf(infotext,"%s24 bit playback is significantly slower than the others! I recommend going on up to 32 bit instead.\r\n\r\n",infotext);
		sprintf(infotext,"%sUTrk can use wavetable envelopes for: velocity, pitch, pan, cutoff, resonance.\r\n\r\n",infotext);
		sprintf(infotext,"%sUTrk can also use Polac's multi-output system because he's nice and gave me some code for that. Refer to his VST loader's documentation for information on how to use it. Tracks use the channel corresponding to their number initially, but you can change that using effect command 78.\r\n\r\n",infotext);
		sprintf(infotext,"%sTracks can be muted/solo'd per MIDI (check the attributes) or by pressing the numbers/underscores you see down there in this dialogue (you can only mute this way).\r\n\r\n",infotext);
		sprintf(infotext,"%sUTrk can take MIDI input. It doesn't currently have virtual channels, so right now you have to have enough tracks and the right samples selected on these tracks. You can tell tracks to not accept MIDI notes by using command 79. It's probably a good idea to use command 72 to make sure all the tracks you want to use for MIDI input have the same settings.\r\n\r\n",infotext);
		sprintf(infotext,"%sUTrk can randomly select samples. Right click on the machine, click on \"Sample Pools\". The pools you arrange there can be accessed as samples #C9-E8.\r\n",infotext);
		sprintf(infotext,"%sThe names of empty pools will not be saved.\r\n\r\n",infotext);
		sprintf(infotext,"%sEffect command reference: (data given as \"xxyy\")\r\n",infotext);
		sprintf(infotext,"%s00	Stop retriggering after xxyy times. Reset on an explicit trigger.\r\n",infotext);
		sprintf(infotext,"%s01	Multiply the velocity by xxyy on each retrigger, where 0xC000 is one (0 dB).\r\n",infotext);
		sprintf(infotext,"%s02	Multiply the frequency by xxyy on each retrigger, where 0x8000 is one.\r\n",infotext);
		sprintf(infotext,"%s03	Transpose by xx.yy semitones on each retrigger, where 0x8000 is zero.\r\n",infotext);
		sprintf(infotext,"%s04	Multiply the time before the next retrigger by this each time. 0x8000 is one.\r\n",infotext);
		sprintf(infotext,"%s05	Reverse sample direction now.\r\n",infotext);
		sprintf(infotext,"%s06	Portamento down, xx semitones in yy subticks.\r\n",infotext);
		sprintf(infotext,"%s07	Portamento up, xx semitones in yy subticks.\r\n",infotext);
		sprintf(infotext,"%s08	Play the sample in reverse so that the specified offset is reached after xxyy subticks.\r\n",infotext);
		sprintf(infotext,"%s09	Probability.\r\n",infotext);
		sprintf(infotext,"%s0A	Change the offset on each retrigger, where 0x8000 is 0.\r\n",infotext);
		sprintf(infotext,"%s0B	Note cut after xxy.y subticks.\r\n",infotext);
		sprintf(infotext,"%s0C	Note release after xxy.y subticks. This has no effect if you don't use envelopes.\r\n",infotext);
		sprintf(infotext,"%s0D	Multiply note cut/release time by xxyy after each retrigger, where 0x8000 is one.\r\n",infotext);
		sprintf(infotext,"%s0E	Reverse and keep the direction in retriggers.\r\n",infotext);
		sprintf(infotext,"%s0F	Portamento to note, xxy.y subticks.\r\n",infotext);
		sprintf(infotext,"%s10	Like 0A, but so that the sample is stretched to xxyy-8000 subticks, negative <-> reverse.\r\n",infotext);
		sprintf(infotext,"%s11	Set lower limit of retrigger time to xx.yy subticks.\r\n",infotext);
		sprintf(infotext,"%s12	Set upper limit of retrigger time to xxyy subticks.\r\n",infotext);
		sprintf(infotext,"%s13	Multiply nth retrig time by n^(xxyy), unit 0x4000, centered about 0x8000.\r\n",infotext);
		sprintf(infotext,"%s14	Like 10, but using ticks instead of subticks.\r\n",infotext);
		sprintf(infotext,"%s15	Like 14, but the length is kept if you change the retrigger time.\r\n",infotext);
		sprintf(infotext,"%s16	Fine portamento down, .xx semitones in yy subticks.\r\n",infotext);
		sprintf(infotext,"%s17	Fine portamento up, .xx semitones in yy subticks.\r\n",infotext);
		sprintf(infotext,"%s18	Linear volume slide to yy in xx subticks.\r\n",infotext);
		sprintf(infotext,"%s19	Logarithmic volume slide to yy in xx subticks.\r\n",infotext);
		sprintf(infotext,"%s1A	Logarithmic volume slide down by yy dB in xx subticks.\r\n",infotext);
		sprintf(infotext,"%s1B	Logarithmic volume slide up by yy dB in xx subticks.\r\n",infotext);
		sprintf(infotext,"%s1C	Fine logarithmic volume slide down by .yy dB in xx subticks.\r\n",infotext);
		sprintf(infotext,"%s1D	Fine logarithmic volume slide up by .yy dB in xx subticks.\r\n",infotext);
		sprintf(infotext,"%s1F	Multiply the retrigger offset change by xxyy on each retrigger, 8000 being one.\r\n",infotext);
		sprintf(infotext,"%s20	Humanize velocity.\r\n",infotext);
		sprintf(infotext,"%s21	Humanize pitch with a range of +- xx.yy semitones.\r\n",infotext);
		sprintf(infotext,"%s22	Humanize offset.\r\n",infotext);
		sprintf(infotext,"%s23	Humanize pan.\r\n",infotext);
		sprintf(infotext,"%s24	Humanize filter cutoff with a range of +-xx.yy semitones.\r\n",infotext);
		sprintf(infotext,"%s25	Humanize timing with a range of +xx.yy subticks.\r\n",infotext);
		sprintf(infotext,"%s26	Humanize timing with a range of +xxyy subticks, and snap to subticks. Can be combined with 25.\r\n",infotext);
		sprintf(infotext,"%s30	Set filter type:\r\n",infotext);
		sprintf(infotext,"%s		0	off\r\n",infotext);
		sprintf(infotext,"%s		1	LP\r\n",infotext);
		sprintf(infotext,"%s		2	HP\r\n",infotext);
		sprintf(infotext,"%s		3	BP (constant skirt gain)\r\n",infotext);
		sprintf(infotext,"%s		4	BP (constant peak gain)\r\n",infotext);
		sprintf(infotext,"%s		5	Notch\r\n",infotext);
		sprintf(infotext,"%s31	Set filter cutoff.\r\n",infotext);
		sprintf(infotext,"%s32	Set filter resonance.\r\n",infotext);
		sprintf(infotext,"%s33	Cutoff slide, see 06.\r\n",infotext);
		sprintf(infotext,"%s34	Cutoff slide, see 07.\r\n",infotext);
		sprintf(infotext,"%s35	Change cutoff on retrigger, see 02.\r\n",infotext);
		sprintf(infotext,"%s36	Change cutoff on retrigger, see 03.\r\n",infotext);
		sprintf(infotext,"%s40	Vibrato speed, the period being xxy.y subticks.\r\n",infotext);
		sprintf(infotext,"%s41	Vibrato depth.\r\n",infotext);
		sprintf(infotext,"%s42	Vibrato waveform:\r\n",infotext);
		sprintf(infotext,"%s		0	Sine\r\n",infotext);
		sprintf(infotext,"%s		1	Triangle\r\n",infotext);
		sprintf(infotext,"%s		2	Square\r\n",infotext);
		sprintf(infotext,"%s		3	Saw\r\n",infotext);
		sprintf(infotext,"%s43	Cutoff LFO speed.\r\n",infotext);
		sprintf(infotext,"%s44	Cutoff LFO depth.\r\n",infotext);
		sprintf(infotext,"%s45	Cutoff LFO waveform.\r\n",infotext);
		sprintf(infotext,"%s50	Sync vibrato to phase xxyy now.\r\n",infotext);
		sprintf(infotext,"%s51	Sync vibrato to phase xxyy on each note. Given no data, sync is turned off.\r\n",infotext);
		sprintf(infotext,"%s52	Sync cutoff LFO now.\r\n",infotext);
		sprintf(infotext,"%s53	Sync cutoff LFO on notes.\r\n",infotext);
		sprintf(infotext,"%s60	Jump xxyy subticks ahead in the wave, disregarding playback speed.\r\n",infotext);
		sprintf(infotext,"%s61	Jump xxyy subticks back in the wave, disregarding playback speed.\r\n",infotext);
		sprintf(infotext,"%s62	Jump xxyy subticks ahead in the wave relative to playback speed.\r\n",infotext);
		sprintf(infotext,"%s63	Jump xxyy subticks back in the wave relative to playback speed.\r\n",infotext);
		sprintf(infotext,"%s64	Jump xxyy ticks ahead in the wave, disregarding playback speed.\r\n",infotext);
		sprintf(infotext,"%s65	Jump xxyy ticks back in the wave, disregarding playback speed.\r\n",infotext);
		sprintf(infotext,"%s66	Jump xxyy ticks ahead in the wave relative to playback speed.\r\n",infotext);
		sprintf(infotext,"%s67	Jump xxyy ticks back in the wave relative to playback speed.\r\n",infotext);
		sprintf(infotext,"%s70	Save settings xx (additively, see below) to slot yy (of 16).\r\n",infotext);
		sprintf(infotext,"%s71	Load settings xx (additively, see below) from slot yy (of 16).\r\n",infotext);
		sprintf(infotext,"%s72	Copy settings xx (as below, also 0x10 for sample number) from track yy.\r\n",infotext);
		sprintf(infotext,"%s78	Set output channel used by this track. No value resets.\r\n",infotext);
		sprintf(infotext,"%s79	Accept MIDI input on this track? 0 = off, any other or no value = on.\r\n",infotext);
		sprintf(infotext,"%s80	Shuffle. xx - length in ticks, yy - strength. (1+0.yy):(1-0.yy).\r\n",infotext); // bing! first xx ticks .yy ticks longer, second xx ticks .yy ticks shorter => at yy==0x00 nothing happens, at yy==0x50 TEH 2:1 SCHUFEL, at yy==0xFF x-treem powar stupid shuffle
		sprintf(infotext,"%s81	Shuffle. xx - length in ticks, yy - strength. (y.y+1):1.\r\n",infotext);
		sprintf(infotext,"%s82	Shuffle. xx - length in ticks, yy - strength. (yy+1):1.\r\n",infotext);
		sprintf(infotext,"%s83	Shuffle. xx - length in ticks, yy - strength. (1+yy/subdiv)/(1-yy/subdiv). yy must be smaller than the subdivide. If you change the subdivide, you'll have to set the shuffle again if you want it to change as well.\r\n",infotext);
		sprintf(infotext,"%sAx	Reset. Works additively:\r\n",infotext);
		sprintf(infotext,"%s		1	On-retrigger effects\r\n",infotext);
		sprintf(infotext,"%s		2	Humanization\r\n",infotext);
		sprintf(infotext,"%s		4	Filters\r\n",infotext);
		sprintf(infotext,"%s		8	LFO and vibrato.\r\n",infotext);
		sprintf(infotext,"%sA0	Does the same as AF, i.e. resets all of the above.\r\n\r\n\r\n",infotext);
		SetDlgItemText(hDlg,IDC_ABTEXT,infotext);
		return 1;
	case WM_SHOWWINDOW:
		//ShowWindow(hDlg,SW_SHOWNORMAL);
		return 1;
	case WM_CLOSE:
		ShowWindow(hDlg,SW_HIDE);
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			ShowWindow(hDlg,SW_HIDE);
			return 1;
		}
		if(pmi!=NULL)switch(LOWORD(wParam))
		{
		case IDC_T1:
			if(pmi->numTracks>=1)
			{
				if(pmi->Tracks[0].trmute==0)
				{
					pmi->Tracks[0].trmute=1;
					SetDlgItemText(hDlg,IDC_T1,"1");
				}
				else
				{
					pmi->Tracks[0].trmute=0;
					SetDlgItemText(hDlg,IDC_T1,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T2:
			if(pmi->numTracks>=2)
			{
				if(pmi->Tracks[1].trmute==0)
				{
					pmi->Tracks[1].trmute=1;
					SetDlgItemText(hDlg,IDC_T2,"2");
				}
				else
				{
					pmi->Tracks[1].trmute=0;
					SetDlgItemText(hDlg,IDC_T2,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T3:
			if(pmi->numTracks>=3)
			{
				if(pmi->Tracks[2].trmute==0)
				{
					pmi->Tracks[2].trmute=1;
					SetDlgItemText(hDlg,IDC_T3,"3");
				}
				else
				{
					pmi->Tracks[2].trmute=0;
					SetDlgItemText(hDlg,IDC_T3,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T4:
			if(pmi->numTracks>=4)
			{
				if(pmi->Tracks[3].trmute==0)
				{
					pmi->Tracks[3].trmute=1;
					SetDlgItemText(hDlg,IDC_T4,"4");
				}
				else
				{
					pmi->Tracks[3].trmute=0;
					SetDlgItemText(hDlg,IDC_T4,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T5:
			if(pmi->numTracks>=5)
			{
				if(pmi->Tracks[4].trmute==0)
				{
					pmi->Tracks[4].trmute=1;
					SetDlgItemText(hDlg,IDC_T5,"5");
				}
				else
				{
					pmi->Tracks[4].trmute=0;
					SetDlgItemText(hDlg,IDC_T5,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T6:
			if(pmi->numTracks>=6)
			{
				if(pmi->Tracks[5].trmute==0)
				{
					pmi->Tracks[5].trmute=1;
					SetDlgItemText(hDlg,IDC_T6,"6");
				}
				else
				{
					pmi->Tracks[5].trmute=0;
					SetDlgItemText(hDlg,IDC_T6,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T7:
			if(pmi->numTracks>=7)
			{
				if(pmi->Tracks[6].trmute==0)
				{
					pmi->Tracks[6].trmute=1;
					SetDlgItemText(hDlg,IDC_T7,"7");
				}
				else
				{
					pmi->Tracks[6].trmute=0;
					SetDlgItemText(hDlg,IDC_T7,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T8:
			if(pmi->numTracks>=8)
			{
				if(pmi->Tracks[7].trmute==0)
				{
					pmi->Tracks[7].trmute=1;
					SetDlgItemText(hDlg,IDC_T8,"8");
				}
				else
				{
					pmi->Tracks[7].trmute=0;
					SetDlgItemText(hDlg,IDC_T8,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T9:
			if(pmi->numTracks>=9)
			{
				if(pmi->Tracks[8].trmute==0)
				{
					pmi->Tracks[8].trmute=1;
					SetDlgItemText(hDlg,IDC_T9,"9");
				}
				else
				{
					pmi->Tracks[8].trmute=0;
					SetDlgItemText(hDlg,IDC_T9,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_TA:
			if(pmi->numTracks>=10)
			{
				if(pmi->Tracks[9].trmute==0)
				{
					pmi->Tracks[9].trmute=1;
					SetDlgItemText(hDlg,IDC_TA,"A");
				}
				else
				{
					pmi->Tracks[9].trmute=0;
					SetDlgItemText(hDlg,IDC_TA,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_TB:
			if(pmi->numTracks>=11)
			{
				if(pmi->Tracks[10].trmute==0)
				{
					pmi->Tracks[10].trmute=1;
					SetDlgItemText(hDlg,IDC_TB,"B");
				}
				else
				{
					pmi->Tracks[10].trmute=0;
					SetDlgItemText(hDlg,IDC_TB,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_TC:
			if(pmi->numTracks>=12)
			{
				if(pmi->Tracks[11].trmute==0)
				{
					pmi->Tracks[11].trmute=1;
					SetDlgItemText(hDlg,IDC_TC,"C");
				}
				else
				{
					pmi->Tracks[11].trmute=0;
					SetDlgItemText(hDlg,IDC_TC,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_TD:
			if(pmi->numTracks>=13)
			{
				if(pmi->Tracks[12].trmute==0)
				{
					pmi->Tracks[12].trmute=1;
					SetDlgItemText(hDlg,IDC_TD,"D");
				}
				else
				{
					pmi->Tracks[12].trmute=0;
					SetDlgItemText(hDlg,IDC_TD,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_TE:
			if(pmi->numTracks>=14)
			{
				if(pmi->Tracks[13].trmute==0)
				{
					pmi->Tracks[13].trmute=1;
					SetDlgItemText(hDlg,IDC_TE,"E");
				}
				else
				{
					pmi->Tracks[13].trmute=0;
					SetDlgItemText(hDlg,IDC_TE,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_TF:
			if(pmi->numTracks>=15)
			{
				if(pmi->Tracks[14].trmute==0)
				{
					pmi->Tracks[14].trmute=1;
					SetDlgItemText(hDlg,IDC_TF,"F");
				}
				else
				{
					pmi->Tracks[14].trmute=0;
					SetDlgItemText(hDlg,IDC_TF,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		case IDC_T0:
			if(pmi->numTracks>=16)
			{
				if(pmi->Tracks[15].trmute==0)
				{
					pmi->Tracks[15].trmute=1;
					SetDlgItemText(hDlg,IDC_T0,"0");
				}
				else
				{
					pmi->Tracks[15].trmute=0;
					SetDlgItemText(hDlg,IDC_T0,"_");
				}
				pmi->DoTrackM();
			}
			return 1;
		default:
			return 0;
		}
		break;
	}
	return 0;
}

int mi::GetRndSamp(int pool)
{
	if(smpsum[pool]>0)
	{
		int a=rand()%smpsum[pool];
		/*do
		{
			a=rand();
		}
		while(a>=smpsum[pool]);*/
		
		for(int b=0;b<200;b++)
		{
			a-=smpool[pool][b];
			if(a<0)return b+1;
		}
	}
	return 0;
}

mi::mi()
{
	trackmlock=false;
	lock2=true;

	int i;
	long units=GetDialogBaseUnits();
	vbasex=LOWORD(units);
	vbasey=HIWORD(units);

	numTracks=1;

	TrackVals=tval;
	AttrVals=(int*)&aval;
	ex.pmi=this;

	dhandle=CreateDialog(dllInstance,MAKEINTRESOURCE(IDD_MABOUT),GetDesktopWindow(),(DLGPROC)&AboutDialog);
	if(dhandle)
	{
		SetDlgItemText(dhandle,IDC_T1,"");
		SetDlgItemText(dhandle,IDC_T2,"");
		SetDlgItemText(dhandle,IDC_T3,"");
		SetDlgItemText(dhandle,IDC_T4,"");
		SetDlgItemText(dhandle,IDC_T5,"");
		SetDlgItemText(dhandle,IDC_T6,"");
		SetDlgItemText(dhandle,IDC_T7,"");
		SetDlgItemText(dhandle,IDC_T8,"");
		SetDlgItemText(dhandle,IDC_T9,"");
		SetDlgItemText(dhandle,IDC_TA,"");
		SetDlgItemText(dhandle,IDC_TB,"");
		SetDlgItemText(dhandle,IDC_TC,"");
		SetDlgItemText(dhandle,IDC_TD,"");
		SetDlgItemText(dhandle,IDC_TE,"");
		SetDlgItemText(dhandle,IDC_TF,"");
		SetDlgItemText(dhandle,IDC_T0,"");
	}
	else MessageBox(GetDesktopWindow(),"Couldn't create about dialogue.","holy cats!",0);

	thandle=CreateDialog(dllInstance,MAKEINTRESOURCE(IDD_TRACKCONTROL),GetDesktopWindow(),(DLGPROC)&TrackControlDialog);
	if(thandle)for(i=0;i<16;i++)
	{
		/*SendMessage(GetDlgItem(thandle,tcmuteboxid[i]),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
		SendMessage(GetDlgItem(thandle,tcsoloboxid[i]),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);*/
		CheckDlgButton(thandle,tcmuteboxid[i],BST_UNCHECKED);
		CheckDlgButton(thandle,tcsoloboxid[i],BST_UNCHECKED);
	}
	else MessageBox(GetDesktopWindow(),"Couldn't create track control dialogue.","holy cats!",0);

	/*micount++;
	tehmilist=(milist *)realloc(tehmilist,micount*sizeof(milist));
	milist *tmp=&tehmilist[micount-1];
	tmp->thismi=this;
	tmp->dhandle=dhandle;*/

	milist *newml=new milist[micount+1];
	for(i=0;i<micount;i++)
	{
		newml[i].thismi=tehmilist[i].thismi;
		newml[i].dhandle=tehmilist[i].dhandle;
		newml[i].thandle=tehmilist[i].thandle;
	}
	newml[micount].thismi=this;
	newml[micount].dhandle=dhandle;
	newml[micount].thandle=thandle;
	//tehmilist=(milist *)realloc(tehmilist,(micount+1)*sizeof(milist));
	free(tehmilist);
	tehmilist=newml;
	micount++;
	/*for(i=0;i<micount;i++)
	{
		tehmilist[i].thismi=newml[i].thismi;
		tehmilist[i].dhandle=newml[i].dhandle;
	}
	free(newml);*/
	M_LoadMachines();

	//MessageBox(GetDesktopWindow(),"test","holy shit!",0);
}

// ==========================================================================================

// ==========================================================================================

mi::~mi()
{
	M_Lock();
	//M_Deoffer(MAKELONG(MACH_WANT_MIDI,POLAC),this);
	M_Deoffer(MAKELONG(MACH_MULTI_OUT,POLAC),this);
	M_Unlock();
	M_FreeMachines();
	DeleteOutBufs();
	DestroyWindow(dhandle);
	DestroyWindow(thandle);

	/*int i=0;
	while(tehmilist[i].thismi!=this)i++;
	milist *tmp=&tehmilist[i];
	
	tmp->thismi=tehmilist[--micount].thismi;
	tmp->dhandle=tehmilist[micount].dhandle;*/
	micount--;
	if(micount>0)
	{
		milist *newml=new milist[micount];
		int j=0;
		for(int i=0;i<=micount;i++)
		{
			if(tehmilist[i].thismi!=this)
			{
				newml[j].thismi=tehmilist[i].thismi;
				newml[j].dhandle=tehmilist[i].dhandle;
				newml[j].thandle=tehmilist[i].thandle;
				j++;
			}
		}
		free(tehmilist);
		tehmilist=newml;
		/*tehmilist=new milist[micount];
		for(i=0;i<micount;i++)
		{
			tehmilist[i].thismi=newml[i].thismi;
			tehmilist[i].dhandle=newml[i].dhandle;
		}
		free(newml);*/
	}
	else
	{
		micount=0;
		free(tehmilist);
		tehmilist=NULL;
	}
}

// ==========================================================================================

// ==========================================================================================

void mi::CreateOutBufs()
{
	outInfo.numOutputs=32;
	strcpy(outInfo.machineName,"UnwieldyTracker");
	for(int i=0;i<16;i++)
	{
		outbuf[i*2]=new float[MAX_BUFFER_LENGTH];
		ZeroMemory(outbuf[i*2],MAX_BUFFER_LENGTH*sizeof(float));
		outbuf[(i*2)+1]=new float[MAX_BUFFER_LENGTH];
		ZeroMemory(outbuf[(i*2)+1],MAX_BUFFER_LENGTH*sizeof(float));
		char buf[80];
		sprintf(buf,"Output %d L",i);
		strcpy(outInfo.outputNames[i*2],buf);
		sprintf(buf,"Output %d R",i);
		strcpy(outInfo.outputNames[(i*2)+1],buf);
	}
	outInfo.out=outbuf;
}

// ==========================================================================================

// ==========================================================================================

void mi::DeleteOutBufs()
{
	for(int i=0;i<32;i++)if(outbuf[i])delete outbuf[i];
}


// ==========================================================================================

// ==========================================================================================

void CTrack::VEnvPoints()
{
	vesize=pmi->pCB->GetEnvSize(smp,0);
	if(vesize>64)vesize=64;
	for(int i=0;i<vesize;i++)
	{
		pmi->pCB->GetEnvPoint(smp,0,i,vepoint[i].x,vepoint[i].y,vepoint[i].flags);
	}
}

// ==========================================================================================

// ==========================================================================================

void CTrack::PEnvPoints()
{
	pesize=pmi->pCB->GetEnvSize(smp,1);
	if(pesize>64)pesize=64;
	for(int i=0;i<pesize;i++)
	{
		pmi->pCB->GetEnvPoint(smp,1,i,pepoint[i].x,pepoint[i].y,pepoint[i].flags);
	}
}

// ==========================================================================================

// ==========================================================================================

void CTrack::CEnvPoints()
{
	cesize=pmi->pCB->GetEnvSize(smp,2);
	if(cesize>64)cesize=64;
	for(int i=0;i<cesize;i++)
	{
		pmi->pCB->GetEnvPoint(smp,2,i,cepoint[i].x,cepoint[i].y,cepoint[i].flags);
	}
}

// ==========================================================================================

// ==========================================================================================

void CTrack::REnvPoints()
{
	resize=pmi->pCB->GetEnvSize(smp,3);
	if(resize>64)resize=64;
	for(int i=0;i<resize;i++)
	{
		pmi->pCB->GetEnvPoint(smp,3,i,repoint[i].x,repoint[i].y,repoint[i].flags);
	}
}

// ==========================================================================================

// ==========================================================================================

void CTrack::NEnvPoints()
{
	nesize=pmi->pCB->GetEnvSize(smp,4);
	if(nesize>64)nesize=64;
	for(int i=0;i<nesize;i++)
	{
		pmi->pCB->GetEnvPoint(smp,4,i,nepoint[i].x,nepoint[i].y,nepoint[i].flags);
	}
}

// ==========================================================================================

// ==========================================================================================

#define SPL5(x,p0,p1,p2,p3,p4,p5) ((p2)+0.04166666666f*x*(((p3)-(p1))*16.0f+((p0)-(p4))*2.0f+x*(((p3)+(p1))*16.0f-(p0)-(p2)*30.0f-(p4)+x*((p3)*66.0f-(p2)*70.0f-(p4)*33.0f+(p1)*39.0f+(p5)*7.0f-(p0)*9.0f+x*((p2)*126.0f-(p3)*124.0f+(p4)*61.0f-(p1)*64.0f-(p5)*12.0f+(p0)*13.0f+x*(((p3)-(p2))*50.0f+((p1)-(p4))*25.0f+((p5)-(p0))*5.0f))))))

float spline16(short *psam, int len,float pos,bool stereo,float *right)
{
	// 6-point spline

	if(!stereo)
	{
		float p0,p1,p2,p3,p4,p5;
		int nrst = (int) pos;
		float x = pos - (float) nrst;
		//int p=nrst-2;
		psam=psam+nrst-2;
		
		if(nrst>1)
		{
			p0=*psam++;
			p1=*psam++;
		}
		else
		{
			p0=0;
			++psam;
			if(nrst>0)p1=*psam++;else{p1=0;++psam;}
		}
		p2=*psam++;
		if(nrst<(len-3))
		{
			p3=*psam++;
			p4=*psam++;
			p5=*psam++;
		}
		else
		{
			p5=0;
			if(nrst<(len-2))
			{
				p3=*psam++;
				p4=*psam++;
			}
			else
			{
				p4=0;
				if(nrst<(len-1))p3=*psam++;else p3=0;
			}
		}
		return SPL5(x,p0,p1,p2,p3,p4,p5);
	}
	else
	{
		float p0l,p1l,p2l,p3l,p4l,p5l;
		float p0r,p1r,p2r,p3r,p4r,p5r;
		int nrst = (int) pos;
		float x = pos - (float) nrst;
		int p=2*nrst-4;

		if(nrst>1)
		{
			p0l=psam[p++];
			p0r=psam[p++];
			p1l=psam[p++];
			p1r=psam[p++];
		}
		else
		{
			p0l=0;
			p0r=0;
			p+=2;
			if(nrst>0)
			{
				p1l=psam[p++];
				p1r=psam[p++];
			}
			else
			{
				p1l=0;
				p1r=0;
				p+=2;
			}
		}
		p2l=psam[p++];
		p2r=psam[p++];
		if(nrst<(len-3))
		{
			p3l=psam[p++];
			p3r=psam[p++];
			p4l=psam[p++];
			p4r=psam[p++];
			p5l=psam[p++];
			p5r=psam[p++];
		}
		else
		{
			p5l=0;
			p5r=0;
			if(nrst<(len-2))
			{
				p3l=psam[p++];
				p3r=psam[p++];
				p4l=psam[p++];
				p4r=psam[p++];
			}
			else
			{
				p4l=0;
				p4r=0;
				if(nrst<(len-1))
				{
					p3l=psam[p++];
					p3r=psam[p++];
				}
				else
				{
					p3l=0;
					p3r=0;
				}
			}
		}
		*right=SPL5(x,p0r,p1r,p2r,p3r,p4r,p5r);
		return SPL5(x,p0l,p1l,p2l,p3l,p4l,p5l);
	}
};

// ==========================================================================================

// ==========================================================================================

float linear16(short *psam, int len, float pos, bool stereo, float *right)
{
	int nearest=(int)pos;	
	float x=pos-(float)nearest;

	if(!stereo)return x*psam[(nearest+1)%len]+(1-x)*psam[nearest];

	int next=2*((nearest+1)%len);
	nearest*=2;
	*right=x*psam[next+1]+(1-x)*psam[nearest+1];
	return x*psam[next]+(1-x)*psam[nearest];
};

// ==========================================================================================

// ==========================================================================================

float nearst16(short *psam, int len, float pos, bool stereo, float *right)
{
	if(!stereo)return psam[(int)pos];
	*right=psam[(((int)pos)<<1)+1];
	return psam[(((int)pos)<<1)];
};

// ==========================================================================================

// ==========================================================================================

float spline24(short *psam, int len, float pos, bool stereo,float *right)
{
	// 6-point spline

	int nrst = (int) pos;
	float x = pos - (float) nrst;
	nrst*=3;
	len*=3;
	
	if(!stereo)
	{
		float p0,p1,p2,p3,p4,p5;
		if(nrst>5)p0=get24((byte *)psam,nrst-6);else p0=0;
		if(nrst>2)p1=get24((byte *)psam,nrst-3);else p1=0;
		p2=get24((byte *)psam,nrst);
		if(nrst<(len-3))p3=get24((byte *)psam,nrst+3);else p3=0;
		if(nrst<(len-6))p4=get24((byte *)psam,nrst+6);else p4=0;
		if(nrst<(len-9))p5=get24((byte *)psam,nrst+9);else p5=0;
		return SPL5(x,p0,p1,p2,p3,p4,p5);
	}
	else
	{
		float p0l,p1l,p2l,p3l,p4l,p5l;
		float p0r,p1r,p2r,p3r,p4r,p5r;
		
		if(nrst>5)
		{
			p0l=get24((byte *)psam,(nrst-6)<<1);
			p0r=get24((byte *)psam,3+((nrst-6)<<1));
			p1l=get24((byte *)psam,(nrst-3)<<1);
			p1r=get24((byte *)psam,3+((nrst-3)<<1));
		}
		else
		{
			p0l=0;
			p0r=0;
			if(nrst>2)
			{
				p1l=get24((byte *)psam,(nrst-3)<<1);
				p1r=get24((byte *)psam,3+((nrst-3)<<1));
			}
			else
			{
				p1l=0;
				p1r=0;
			}
		}
		p2l=get24((byte *)psam,nrst<<1);
		p2r=get24((byte *)psam,3+(nrst<<1));
		if(nrst<(len-3))
		{
			p3l=get24((byte *)psam,(nrst+3)<<1);
			p3r=get24((byte *)psam,3+((nrst+3)<<1));
			p4l=get24((byte *)psam,(nrst+6)<<1);
			p4r=get24((byte *)psam,3+((nrst+6)<<1));
			p5l=get24((byte *)psam,(nrst+9)<<1);
			p5r=get24((byte *)psam,3+((nrst+9)<<1));
		}
		else
		{
			p5l=0;
			p5r=0;
			if(nrst<(len-6))
			{
				p4l=get24((byte *)psam,(nrst+6)<<1);
				p4r=get24((byte *)psam,3+((nrst+6)<<1));
				p5l=get24((byte *)psam,(nrst+9)<<1);
				p5r=get24((byte *)psam,3+((nrst+9)<<1));
			}
			else
			{
				p4l=0;
				p4r=0;
				if(nrst<(len-9))
				{
					p5l=get24((byte *)psam,(nrst+9)<<1);
					p5r=get24((byte *)psam,3+((nrst+9)<<1));
				}
				else
				{
					p5l=0;
					p5r=0;
				}
			}
		}
		*right=SPL5(x,p0r,p1r,p2r,p3r,p4r,p5r);
		return SPL5(x,p0l,p1l,p2l,p3l,p4l,p5l);
	}
};

// ==========================================================================================

// ==========================================================================================

float linear24(short *psam, int len, float pos, bool stereo, float *right)
{
	int nearest=(int)pos;
	float x=pos-(float)nearest;
	nearest*=3;
	len*=3;

	if(!stereo)return x*get24((byte *)psam,(nearest+3)%len)+(1-x)*get24((byte *)psam,nearest);

	int next=2*((nearest+3)%len);
	nearest*=2;
	*right=x*get24((byte *)psam,next+3)+(1-x)*get24((byte *)psam,nearest+3);
	return x*get24((byte *)psam,next)+(1-x)*get24((byte *)psam,nearest);
};

// ==========================================================================================

// ==========================================================================================

float nearst24(short *psam, int len, float pos, bool stereo, float *right)
{
	if(!stereo)return get24((byte *)psam,((int)pos)*3);

	int n=((int)pos)*6;
	*right=get24((byte *)psam,n+3);
	return get24((byte *)psam,n);
};

// ==========================================================================================

// ==========================================================================================

inline float get24(byte *psam,int p)
{
	int i=*(int *)(psam+p-2);
	return (float)((i&0x00FFFFFF)<<8);
}

// ==========================================================================================

// ==========================================================================================

float spline32i(short *psamp, int len, float pos, bool stereo, float *right)
{
	/*if(stereo)*right=0;
	return 0;*/
	
	/*// 6-point spline

	long *psam=(long *)psamp;

	float p0,p1,p2,p3,p4,p5;
	int nrst = (int) pos;
	float x = pos - (float) nrst;
	float out;
	
	if(!stereo)
	{		
		if(nrst>1)p0=(float)psam[nrst-2];else p0=0;
		if(nrst>0)p1=(float)psam[nrst-1];else p1=0;
		p2=(float)psam[nrst];
		if(nrst<(len-1))p3=(float)psam[nrst+1];else p3=0;
		if(nrst<(len-2))p4=(float)psam[nrst+2];else p4=0;
		if(nrst<(len-3))p5=(float)psam[nrst+3];else p5=0;
	}
	else
	{
		if(chn)//right
		{
			if(nrst>1)p0=(float)psam[1+2*(nrst-2)];else p0=0;
			if(nrst>0)p1=(float)psam[1+2*(nrst-1)];else p1=0;
			p2=(float)psam[1+(nrst<<1)];
			if(nrst<(len-1))p3=(float)psam[1+2*(nrst+1)];else p3=0;
			if(nrst<(len-2))p4=(float)psam[1+2*(nrst+2)];else p4=0;
			if(nrst<(len-3))p5=(float)psam[1+2*(nrst+3)];else p5=0;
		}
		else//left
		{
			if(nrst>1)p0=(float)psam[2*(nrst-2)];else p0=0;
			if(nrst>0)p1=(float)psam[2*(nrst-1)];else p1=0;
			p2=(float)psam[(nrst<<1)];
			if(nrst<(len-1))p3=(float)psam[2*(nrst+1)];else p3=0;
			if(nrst<(len-2))p4=(float)psam[2*(nrst+2)];else p4=0;
			if(nrst<(len-3))p5=(float)psam[2*(nrst+3)];else p5=0;
		}
	}

	out=p2+0.04166666666f*x*((p3-p1)*16.0f+(p0-p4)*2.0f+x*((p3+p1)*16.0f-p0-p2*30.0f-p4+x*(p3*66.0f-p2*70.0f-p4*33.0f+p1*39.0f+p5*7.0f-p0*9.0f+x*(p2*126.0f-p3*124.0f+p4*61.0f-p1*64.0f-p5*12.0f+p0*13.0f+x*((p3-p2)*50.0f+(p1-p4)*25.0f+(p5-p0)*5.0f)))));
	return (float)out;*/

	// 6-point spline

	long *psam=(long *)psamp;

	if(!stereo)
	{
		long p0,p1,p2,p3,p4,p5;
		int nrst = (int) pos;
		float x = pos - (float) nrst;
		//int p=nrst-2;
		psam=psam+nrst-2;
		
		if(nrst>1)
		{
			p0=*psam++;
			p1=*psam++;
		}
		else
		{
			p0=0;
			++psam;
			if(nrst>0)p1=*psam++;else{p1=0;++psam;}
		}
		p2=*psam++;
		if(nrst<(len-3))
		{
			p3=*psam++;
			p4=*psam++;
			p5=*psam++;
		}
		else
		{
			p5=0;
			if(nrst<(len-2))
			{
				p3=*psam++;
				p4=*psam++;
			}
			else
			{
				p4=0;
				if(nrst<(len-1))p3=*psam++;else p3=0;
			}
		}
		return SPL5(x,(float)p0,(float)p1,(float)p2,(float)p3,(float)p4,(float)p5);
	}
	else
	{
		long p0l,p1l,p2l,p3l,p4l,p5l;
		long p0r,p1r,p2r,p3r,p4r,p5r;
		int nrst = (int) pos;
		float x = pos - (float) nrst;
		int p=2*nrst-4;

		if(nrst>1)
		{
			p0l=psam[p++];
			p0r=psam[p++];
			p1l=psam[p++];
			p1r=psam[p++];
		}
		else
		{
			p0l=0;
			p0r=0;
			p+=2;
			if(nrst>0)
			{
				p1l=psam[p++];
				p1r=psam[p++];
			}
			else
			{
				p1l=0;
				p1r=0;
				p+=2;
			}
		}
		p2l=psam[p++];
		p2r=psam[p++];
		if(nrst<(len-3))
		{
			p3l=psam[p++];
			p3r=psam[p++];
			p4l=psam[p++];
			p4r=psam[p++];
			p5l=psam[p++];
			p5r=psam[p++];
		}
		else
		{
			p5l=0;
			p5r=0;
			if(nrst<(len-2))
			{
				p3l=psam[p++];
				p3r=psam[p++];
				p4l=psam[p++];
				p4r=psam[p++];
			}
			else
			{
				p4l=0;
				p4r=0;
				if(nrst<(len-1))
				{
					p3l=psam[p++];
					p3r=psam[p++];
				}
				else
				{
					p3l=0;
					p3r=0;
				}
			}
		}
		*right=SPL5(x,(float)p0r,(float)p1r,(float)p2r,(float)p3r,(float)p4r,(float)p5r);
		return SPL5(x,(float)p0l,(float)p1l,(float)p2l,(float)p3l,(float)p4l,(float)p5l);
	}
};

// ==========================================================================================

// ==========================================================================================

float linear32i(short *psamp, int len, float pos, bool stereo, float *right)
{
	long *psam=(long *)psamp;

	int nearest=(int)pos;	
	float x=pos-(float)nearest;

	if(!stereo)return x*psam[(nearest+1)%len]+(1-x)*psam[nearest];

	int next=2*((nearest+1)%len);
	nearest*=2;
	*right=x*psam[next+1]+(1-x)*psam[nearest+1];
	return x*psam[next]+(1-x)*psam[nearest];
};

// ==========================================================================================

// ==========================================================================================

float nearst32i(short *psamp, int len, float pos, bool stereo, float *right)
{
	long *psam=(long *)psamp;

	if(!stereo)return (float)psam[(int)pos];
	*right=(float)psam[(((int)pos)<<1)+1];
	return (float)psam[(((int)pos)<<1)];
};

// ==========================================================================================

// ==========================================================================================

float spline32f(short *psamp, int len, float pos, bool stereo, float *right)
{
	// 6-point spline

	float *psam=(float *)psamp;

	if(!stereo)
	{
		float p0,p1,p2,p3,p4,p5;
		int nrst = (int) pos;
		float x = pos - (float) nrst;
		//int p=nrst-2;
		psam=psam+nrst-2;
		
		if(nrst>1)
		{
			p0=*psam++;
			p1=*psam++;
		}
		else
		{
			p0=0;
			++psam;
			if(nrst>0)p1=*psam++;else{p1=0;++psam;}
		}
		p2=*psam++;
		if(nrst<(len-3))
		{
			p3=*psam++;
			p4=*psam++;
			p5=*psam++;
		}
		else
		{
			p5=0;
			if(nrst<(len-2))
			{
				p3=*psam++;
				p4=*psam++;
			}
			else
			{
				p4=0;
				if(nrst<(len-1))p3=*psam++;else p3=0;
			}
		}
		return SPL5(x,p0,p1,p2,p3,p4,p5);
	}
	else
	{
		float p0l,p1l,p2l,p3l,p4l,p5l;
		float p0r,p1r,p2r,p3r,p4r,p5r;
		int nrst = (int) pos;
		float x = pos - (float) nrst;
		int p=2*nrst-4;

		if(nrst>1)
		{
			p0l=psam[p++];
			p0r=psam[p++];
			p1l=psam[p++];
			p1r=psam[p++];
		}
		else
		{
			p0l=0;
			p0r=0;
			p+=2;
			if(nrst>0)
			{
				p1l=psam[p++];
				p1r=psam[p++];
			}
			else
			{
				p1l=0;
				p1r=0;
				p+=2;
			}
		}
		p2l=psam[p++];
		p2r=psam[p++];
		if(nrst<(len-3))
		{
			p3l=psam[p++];
			p3r=psam[p++];
			p4l=psam[p++];
			p4r=psam[p++];
			p5l=psam[p++];
			p5r=psam[p++];
		}
		else
		{
			p5l=0;
			p5r=0;
			if(nrst<(len-2))
			{
				p3l=psam[p++];
				p3r=psam[p++];
				p4l=psam[p++];
				p4r=psam[p++];
			}
			else
			{
				p4l=0;
				p4r=0;
				if(nrst<(len-1))
				{
					p3l=psam[p++];
					p3r=psam[p++];
				}
				else
				{
					p3l=0;
					p3r=0;
				}
			}
		}
		*right=SPL5(x,p0r,p1r,p2r,p3r,p4r,p5r);
		return SPL5(x,p0l,p1l,p2l,p3l,p4l,p5l);
	}
};

// ==========================================================================================

// ==========================================================================================

float linear32f(short *psamp, int len, float pos, bool stereo, float *right)
{
	float *psam=(float *)psamp;

	int nearest=(int)pos;	
	float x=pos-(float)nearest;

	if(!stereo)return x*psam[(nearest+1)%len]+(1-x)*psam[nearest];

	int next=2*((nearest+1)%len);
	nearest*=2;
	*right=x*psam[next+1]+(1-x)*psam[nearest+1];
	return x*psam[next]+(1-x)*psam[nearest];
};

// ==========================================================================================

// ==========================================================================================

float nearst32f(short *psamp, int len, float pos, bool stereo, float *right)
{
	float *psam=(float *)psamp;

	if(!stereo)return psam[(int)pos];
	*right=psam[(((int)pos)<<1)+1];
	return psam[(((int)pos)<<1)];
};

// ==========================================================================================

// ==========================================================================================

void CTrack::Stop()
{
	pdel=0;
	del=0;
	ret=0;
	tamp=0;
	
	if(trmute==0)
	{
		if(pesize||cesize||resize||nesize)
		{
			erel=1;
		}
		
		if(vesize)
		{
			dir=1;
			erel=1;
		}
		else
		{
			wstate=WSTATE_DIE;
		}
	}
	else
	{
		//trmute=0;
		wstate=WSTATE_OFF;
		fcn=0;
	}

	midi_note_playing=-1;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::Stop2()
{
	if(pesize|cesize|resize|nesize)
	{
		erel=1;
	}

	if(vesize)
	{
		dir=1;
		erel=1;
	}
	else
	{
		wstate=WSTATE_DIE;
	}

	midi_note_playing=-1;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::StopMIDINote(int note)
{
	if(midi_note_playing==note)Stop();
}

// ==========================================================================================

// ==========================================================================================

void CTrack::ResRet()
{
	mcut=mdelt=mamp=mfreq=mncs=1;
	imaxdel=imindel=0;
	maxdel=mindel=0;
	countr=0;
	pwr=0;
	afcn=0;
	mafcn=1;
	keep_str=0;
	del=ret=0;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::ResHum()
{
	rfcn=ramp=rfreq=rpan=rcut=0;
	maxhums=maxhumsu=0;
	hcutm=1;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::ResFil()
{
	pcut=cut=22000;
	hcutm=1;
	res=1;
	filt=0;
	CalcCoef();
	ClearHis();
}

// ==========================================================================================

// ==========================================================================================

void CTrack::ResLFO()
{
	vib=vcnt=vspd=vwav=0;
	vdep=0;
	lfo=lfocnt=lfospd=lfowav=0;
	lfodep=0;
	syncv=synclfo=-1;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::SaveRet(int i)
{
	smdelt[i]=mdelt;
	smamp[i]=mamp;
	smfreq[i]=mfreq;
	smncs[i]=mncs;
	smcut[i]=mcut;
	simaxdel[i]=imaxdel;
	simindel[i]=imindel;
	smaxdel[i]=maxdel;
	smindel[i]=mindel;
	spwr[i]=pwr;
	safcn[i]=afcn;
	smafcn[i]=mafcn;
	skeep_str[i]=keep_str;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::SaveHum(int i)
{
	srfcn[i]=rfcn;
	sramp[i]=ramp;
	srfreq[i]=rfreq;
	srpan[i]=rpan;
	srcut[i]=rcut;
	smaxhums[i]=maxhums;
	smaxhumsu[i]=maxhumsu;
	shcutm[i]=hcutm;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::SaveFil(int i)
{
	spcut[i]=pcut;
	scut[i]=cut;
	shcutm[i]=hcutm;
	sres[i]=res;
	sfilt[i]=filt;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::SaveLFO(int i)
{
	svib[i]=vib;
	svcnt[i]=vcnt;
	svspd[i]=vspd;
	svwav[i]=vwav;
	svdep[i]=vdep;
	slfo[i]=lfo;
	slfocnt[i]=lfocnt;
	slfospd[i]=lfospd;
	slfowav[i]=lfowav;
	slfodep[i]=lfodep;
	ssyncv[i]=syncv;
	ssynclfo[i]=synclfo;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::LoadRet(int i)
{
	mdelt=smdelt[i];
	mamp=smamp[i];
	mfreq=smfreq[i];
	mncs=smncs[i];
	mcut=smcut[i];
	imaxdel=simaxdel[i];
	imindel=simindel[i];
	maxdel=smaxdel[i];
	mindel=smindel[i];
	pwr=spwr[i];
	afcn=safcn[i];
	mafcn=smafcn[i];
	keep_str=skeep_str[i];
}

// ==========================================================================================

// ==========================================================================================

void CTrack::LoadHum(int i)
{
	rfcn=srfcn[i];
	ramp=sramp[i];
	rfreq=srfreq[i];
	rpan=srpan[i];
	rcut=srcut[i];
	maxhums=smaxhums[i];
	maxhumsu=smaxhumsu[i];
	hcutm=shcutm[i];
}

// ==========================================================================================

// ==========================================================================================

void CTrack::LoadFil(int i)
{
	pcut=spcut[i];
	cut=scut[i];
	hcutm=shcutm[i];
	res=sres[i];
	filt=sfilt[i];
	CalcCoef();
	ClearHis();
}

// ==========================================================================================

// ==========================================================================================

void CTrack::LoadLFO(int i)
{
	vib=svib[i];
	vcnt=svcnt[i];
	vspd=svspd[i];
	vwav=svwav[i];
	vdep=svdep[i];
	lfo=slfo[i];
	lfocnt=slfocnt[i];
	lfospd=slfospd[i];
	lfowav=slfowav[i];
	lfodep=slfodep[i];
	syncv=ssyncv[i];
	synclfo=ssynclfo[i];
}

// ==========================================================================================

// ==========================================================================================

void CTrack::ClearHis()
{
	lhis[0]=lhis[1]=lhis[2]=lhis[3]=0;
	rhis[0]=rhis[1]=rhis[2]=rhis[3]=0;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::CalcCoef()
{
	if(filt)
	{
		float alpha,omega,sn,cs;
		float a0;

		if(cut>((pmi->pMasterInfo->SamplesPerSec>>1)-50))cut=(float)(pmi->pMasterInfo->SamplesPerSec>>1)-50;
		if(cut<10)cut=10;
		if(res>20)res=20;
		if(res<0.01f)res=0.01f;

		/*cut=1000;
		res=1;*/

		omega=(float)(2*PI*cut/pmi->pMasterInfo->SamplesPerSec);
		sn=(float)sin(omega);
		cs=(float)cos(omega);
		alpha=0.5f*sn/res;

		a0=1.0f/(1+alpha);
		coef[3]=2*cs*a0;
		coef[4]=(alpha-1)*a0;
		switch(filt)
		{
		case 1: //LP
			coef[0]=coef[2]=(1-cs)*0.5f*a0;
			coef[1]=2*coef[0];
			break;
		case 2: //HP
			coef[0]=coef[2]=(1+cs)*0.5f*a0;
			coef[1]=-2*coef[0];
			break;
		case 3: //BP
			coef[0]=sn*0.5f*a0;
			coef[2]=-coef[0];
			coef[1]=0;
			break;
		case 4: //BP2
			coef[0]=alpha*a0;
			coef[2]=-coef[0];
			coef[1]=0;
			break;
		case 5: //notch
			coef[0]=a0;
			coef[2]=a0;
			coef[1]=-2*a0*cs;
			break;
		}
	}
	else
	{
		coef[0]=1;
		coef[1]=coef[2]=coef[3]=coef[4]=0;
	}
}

// ==========================================================================================

// ==========================================================================================

void ClearTick(tvals *tv)
{
	tv->del=0xFF;
	tv->ec1=0xFF;
	tv->ec2=0xFF;
	tv->ep1=0xFFFF;
	tv->ep2=0xFFFF;
	tv->nte=NOTE_NO;
	tv->ofs=0xFFFF;
	tv->pan=0xFF;
	tv->ret=0xFF;
	tv->smp=0x00;
	tv->str=0xFFFF;
	tv->sub=0xFF;
	tv->vel=0xFF;
}

// ==========================================================================================

// ==========================================================================================

void TransTick(tvals *tv,tvals const &tvi)
{
	tv->del=tvi.del;
	tv->ec1=tvi.ec1;
	tv->ec2=tvi.ec2;
	tv->ep1=tvi.ep1;
	tv->ep2=tvi.ep2;
	tv->nte=tvi.nte;
	tv->ofs=tvi.ofs;
	tv->pan=tvi.pan;
	tv->ret=tvi.ret;
	tv->smp=tvi.smp;
	tv->str=tvi.str;
	tv->sub=tvi.sub;
	tv->vel=tvi.vel;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::Tick(tvals const &tv)
{
	if(tv.sub!=0xFF)
	{
		subdiv=tv.sub;
		mindel=(float)imindel*pmi->pMasterInfo->SamplesPerTick/256/subdiv;
		maxdel=(float)imaxdel*pmi->pMasterInfo->SamplesPerTick/subdiv;
	}

	if(tv.ec1==0x79)acc_midi=(tv.ep1==0)?0:1;
	if(tv.ec2==0x79)acc_midi=(tv.ep2==0)?0:1;

	if(tv.ec1==0x80)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep1&0xFF00)&&(tv.ep1&0x00FF)&&(tv.ep1!=0xFFFF))
		{
			shufflen=(tv.ep1&0xFF00)>>8;
			shuffstr=(float)(tv.ep1&0x00FF)/256;
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec2==0x80)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep2&0xFF00)&&(tv.ep2&0x00FF)&&(tv.ep2!=0xFFFF))
		{
			shufflen=(tv.ep2&0xFF00)>>8;
			shuffstr=(float)(tv.ep2&0x00FF)/256;
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec1==0x81)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep1&0xFF00)&&(tv.ep1&0x00FF)&&(tv.ep1!=0xFFFF))
		{
			shufflen=(tv.ep1&0xFF00)>>8;
			shuffstr=(float)(tv.ep1&0x00FF)/16+1;
			shuffstr=(shuffstr-1)/(shuffstr+1);
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec2==0x81)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep2&0xFF00)&&(tv.ep2&0x00FF)&&(tv.ep2!=0xFFFF))
		{
			shufflen=(tv.ep2&0xFF00)>>8;
			shuffstr=(float)(tv.ep2&0x00FF)/16+1;
			shuffstr=(shuffstr-1)/(shuffstr+1);
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec1==0x82)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep1&0xFF00)&&(tv.ep1&0x00FF)&&(tv.ep1!=0xFFFF))
		{
			shufflen=(tv.ep1&0xFF00)>>8;
			shuffstr=(float)(tv.ep1&0x00FF)+1;
			shuffstr=(shuffstr-1)/(shuffstr+1);
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec2==0x82)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep2&0xFF00)&&(tv.ep2&0x00FF)&&(tv.ep2!=0xFFFF))
		{
			shufflen=(tv.ep2&0xFF00)>>8;
			shuffstr=(float)(tv.ep2&0x00FF)+1;
			shuffstr=(shuffstr-1)/(shuffstr+1);
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec1==0x83)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep1&0xFF00)&&(tv.ep1&0x00FF)&&(tv.ep1!=0xFFFF))
		{
			shufflen=(tv.ep1&0xFF00)>>8;
			int st=tv.ep1&0x00FF;
			if(st>=subdiv)st=subdiv-1;
			shuffstr=(float)st/subdiv;
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	if(tv.ec2==0x83)
	{
		shufflen=0;
		shcount=0;
		shuffstr=0;
		shtickin=shtickout=0;
		if(shticks)delete shticks;shticks=NULL;
		if((tv.ep2&0xFF00)&&(tv.ep2&0x00FF)&&(tv.ep2!=0xFFFF))
		{
			shufflen=(tv.ep2&0xFF00)>>8;
			int st=tv.ep2&0x00FF;
			if(st>=subdiv)st=subdiv-1;
			shuffstr=(float)st/subdiv;
			shticks=new tvals[shufflen*2];
			for(int i=0;i<shufflen*2;i++)ClearTick(&shticks[i]);
		}
	}

	longsh=(int)((float)pmi->pMasterInfo->SamplesPerTick*(1+shuffstr));
	shortsh=2*pmi->pMasterInfo->SamplesPerTick-longsh;

	if(shufflen)
	{
		if(shtickin==0)shcount=0;
		TransTick(&shticks[shtickin],tv);
		shtickin=(shtickin+1)%(shufflen*2);
	}
	else DoTick(tv,1);
}

// ==========================================================================================

// ==========================================================================================

float CTrack::GetMul(int i)
{
	if(!pmi)return 0;
	CWaveInfo const *pinfo=pmi->pCB->GetWave(i);
	if(pinfo)
	{
		if(pinfo->Flags&WF_NOT16)
		{
			CWaveLevel const *pwave=pmi->pCB->GetWaveLevel(i,0);
			if(pwave)
			{
				switch(pwave->pSamples[0])
				{
				case WFE_32BITI:					
				case WFE_32BITF:return 0.5f;break;
				case WFE_24BIT:return 0.66666666666666666666667f;break;
				}
			}
		}
	}
	return 1;
}

// ==========================================================================================

// ==========================================================================================

void CTrack::DoTick(tvals const &tv,int f)
{
	if(tv.ec1==0x25)if(tv.ep1!=0xFFFF)maxhums=tv.ep1*spt/subdiv/256;
	if(tv.ec2==0x25)if(tv.ep2!=0xFFFF)maxhums=tv.ep2*spt/subdiv/256;
	if(tv.ec1==0x26)if(tv.ep1!=0xFFFF)maxhumsu=tv.ep1;
	if(tv.ec2==0x26)if(tv.ep2!=0xFFFF)maxhumsu=tv.ep2;

	if((tv.del!=0xFF)||(f&&((maxhums>0)||(maxhumsu>0))))
	{
		int tdel;
		if(tv.del!=0xFF)
		{
			if(shufflen)
			{
				if(shtickout>=shufflen)tdel=(int)((double)tv.del*shortsh/subdiv);
				else tdel=(int)((double)tv.del*longsh/subdiv);
			}
			else tdel=(int)((double)tv.del*spt/subdiv);
		}
		else tdel=0;
		tdel+=(int)((double)rand()*EDU*maxhums);
		tdel+=(int)((double)spt/subdiv*fround((double)rand()*EDU*maxhumsu));
		queue.addev(tdel,tv);
		return;
	}

	bool vset=false;
	bool dnote=true;
	bool dofl=false;

	astep=pmi->astep;
	pstep=pmi->pstep;
	ofd=pmi->ofd;

	if(tv.str!=0xFFFF)
	{
		CWaveLevel const *pwave;
		stretch=tv.str;

		if(stretch)
		{
			pwave=pmi->pCB->GetWaveLevel(smp,0);
			spt=pmi->pMasterInfo->SamplesPerTick;
			if(pwave)
			{
				fstep=pfreq=(float)pwave->numSamples/stretch/spt;
				//if(wstate==WSTATE_OFF)fstep=pfreq;
			}
		}
	}
	
	if(tv.pan!=0xFF)
	{
		tpan=2-(float)tv.pan/64;
		if(tpan<pan)pstate=PSTATE_RIGHT;
		else pstate=PSTATE_LEFT;
		if(wstate==WSTATE_OFF)
		{
			pan=tpan;
			pstate=PSTATE_NO;
		}
	}
	
	if(tv.vel!=0xFF)
	{
		vel=(float)tv.vel/64;
		tamp=vel;
		if(wstate!=WSTATE_OFF)
		{
			if(vel>amp)wstate=WSTATE_UP;
			else wstate=WSTATE_DOWN;
		}
		vset=true;
	}

	if(tv.ec1&0xA0)
	{
		switch(tv.ec1)
		{
		case 0x20:if(tv.ep1!=0xFFFF)ramp=tv.ep1*0.5f*EDU;break;
		case 0x21:if(tv.ep1!=0xFFFF)rfreq=(float)tv.ep1/256;break;
		case 0x22:if(tv.ep1!=0xFFFF)rfcn=tv.ep1*0.5f*EDU;break;
		case 0x23:if(tv.ep1!=0xFFFF)rpan=tv.ep1*0.5f*EDU;break;
		case 0x24:if(tv.ep1!=0xFFFF)rcut=(float)tv.ep1/256;break;
		case 0xA0:
			ResRet();
			ResHum();
			ResFil();
			ResLFO();
			break;
		}
		if((tv.ec1&0xF0)==0xA0)
		{
			if(tv.ec1&0x01)ResRet();
			if(tv.ec1&0x02)ResHum();
			if(tv.ec1&0x04)ResFil();
			if(tv.ec1&0x08)ResLFO();
		}
	}

	if(tv.ec2&0xA0)
	{
		switch(tv.ec2)
		{
		case 0x20:if(tv.ep2!=0xFFFF)ramp=tv.ep2*0.5f*EDU;break;
		case 0x21:if(tv.ep2!=0xFFFF)rfreq=(float)tv.ep2/256;break;
		case 0x22:if(tv.ep2!=0xFFFF)rfcn=tv.ep2*0.5f*EDU;break;
		case 0x23:if(tv.ep2!=0xFFFF)rpan=tv.ep2*0.5f*EDU;break;
		case 0x24:if(tv.ep2!=0xFFFF)rcut=(float)tv.ep2/256;break;
		case 0xA0:
			ResRet();
			ResHum();
			ResFil();
			ResLFO();
			break;
		}
		if((tv.ec2&0xF0)==0xA0)
		{
			if(tv.ec2&0x01)ResRet();
			if(tv.ec2&0x02)ResHum();
			if(tv.ec2&0x04)ResFil();
			if(tv.ec2&0x08)ResLFO();
		}
	}

	if(tv.ec1==0x09)if(rand()>(tv.ep1>>1))dnote=false;
	if(tv.ec2==0x09)if(rand()>(tv.ep2>>1))dnote=false;
	if(dnote)
	{
		if(tv.smp!=0x00)
		{
			if(wstate!=WSTATE_OFF)
			{
				byte nsmp;
				CWaveLevel const *pwave;
				
				nsmp=tv.smp;

				if(nsmp>200)
				{
					poolused=nsmp-201;
					nsmp=pmi->GetRndSamp(poolused);
				}
				else poolused=-1;

				pwave=pmi->pCB->GetWaveLevel(nsmp,0);
				
				if(pwave)
				{
					CWaveLevel const *pwav;
					pwav=pmi->pCB->GetWaveLevel(smp,0);
					if(stretch==0)if(pwav)fstep*=(float)pwav->SamplesPerSec/pwave->SamplesPerSec;
					smp=nsmp;
					sr=pwave->SamplesPerSec;
				}
				else
				{
					sr=44100;
					pdel=0;
					del=0;
					ret=0;
					smp=nsmp;
					amp=0;
					pan=tpan;
					wstate=WSTATE_OFF;
					fcn=0;
				}
			}
			else
			{
				if(tv.smp>200)
				{
					poolused=tv.smp-201;
					smp=pmi->GetRndSamp(poolused);
				}
				else
				{
					poolused=-1;
					smp=tv.smp;
				}
			}
		}
		bool cnote=true;
		if((tv.ec1==0x0F)&&(tv.nte!=NOTE_NO)&&(tv.nte!=NOTE_OFF))
		{
			pnote=buzz2midi(tv.nte);
			float freqq=pmi->note2freq(pnote,sr)*(float)powd(2,(double)(pmi->aval.pattern_transpose-48)/12);

			if((tv.ep1!=0xFFFF)&&(tv.ep1!=0))
			{
				porca=tv.ep1*spt/subdiv/16;
				freqm=(float)powd(freqq/fstep,1.0/porca); // fstep*(freqm^porca)=freqq ... freqm = (freqq/fstep)^(1/porca)
			}
			else
			{
				porca=0;
				freqm=1;
				pfreq=fstep=freqq;
			}
			cnote=false;
		}
		if((tv.ec2==0x0F)&&(tv.nte!=NOTE_NO)&&(tv.nte!=NOTE_OFF))
		{
			pnote=buzz2midi(tv.nte);
			float freqq=pmi->note2freq(pnote,sr)*(float)powd(2,(double)(pmi->aval.pattern_transpose-48)/12);

			if((tv.ep2!=0xFFFF)&&(tv.ep2!=0))
			{
				porca=tv.ep2*spt/subdiv/16;
				freqm=(float)powd(freqq/fstep,1.0/porca); // fstep*(freqm^porca)=freqq ... freqm = (freqq/fstep)^(1/porca)
			}
			else
			{
				porca=0;
				freqm=1;
				pfreq=fstep=freqq;
			}
			cnote=false;
		}
		if(cnote)if(tv.nte!=NOTE_NO)
		{
			bool feg=false;
			ncmode=0;
			if(tv.nte==NOTE_OFF)Stop();
			else
			{
				midi_note_playing=-1;

				stopsl=1;
				pfcn=0;
				
				del=0;
				pdel=0;
				ret=0;

				fnfcn=0;
				
				sps=pmi->pMasterInfo->SamplesPerSec;
				spt=pmi->pMasterInfo->SamplesPerTick;
				
				CWaveLevel const *pwave;
				pwave=pmi->pCB->GetWaveLevel(smp,0);
				
				if(pwave)sr=pwave->SamplesPerSec;
				
				pnote=buzz2midi(tv.nte);
				if(stretch)
				{
					if(pwave)
					{
						fnstep=pfreq=(float)pwave->numSamples/stretch/spt;
					}
				}
				else fnstep=pfreq=pmi->note2freq(pnote,sr)*(float)powd(2,(double)(pmi->aval.pattern_transpose-48)/12);
				
				/*if((maxhums>0)||(maxhumsu>0))
				{
					pdel+=(double)rand()*EDU*maxhums;
					pdel+=(double)spt/subdiv*fround((double)rand()*EDU*maxhumsu);
					fnstep-=(float)pdel;
					del=(int)pdel;
					ret=0;
					if(!vset)vel=1;
					pgn=1;
					if(del==0)
					{
						pdel=0;
						if(vset)PlayNote(pfreq,vel,tpan);
						else PlayNote(pfreq,1,tpan);
						gn=1;
						pgn=mamp;
					}
					else feg=true;
				}
				else*/
				{
					ret=del=0;
					pdel=0;
					if(vset)PlayNote(pfreq,vel,tpan);
					else PlayNote(pfreq,1,tpan);
					gn=1;
					pgn=mamp;
				}
				if(tv.ret!=0xFF)
				{
					if(tv.ret>0)
					{
						if(shufflen)
						{
							if(shtickout>=shufflen)pdel=(double)tv.ret*shortsh/subdiv;
							else pdel=(double)tv.ret*longsh/subdiv;
						}
						else pdel=(double)tv.ret*spt/subdiv;
						ret=-1;
						if(!feg)del=(int)pdel;
						if(!vset)vel=1;
						pgn=mamp;
						countr=0;
					}
				}
				if(feg)
				{
					pfreq/=mfreq;
					pcut/=mcut;
				}
			}
		}
		else if((tv.ret!=0xFF)&&(wstate!=WSTATE_OFF))
		{
			if(tv.ret>0)
			{
				if(shufflen)
				{
					if(shtickout>shufflen)pdel=(double)tv.ret*0.5*shortsh/subdiv;
					else pdel=(double)tv.ret*0.5*longsh/subdiv;
				}
				else pdel=(double)tv.ret*spt/subdiv;
				ret=-1;
				del=(int)pdel;
				pfcn=fcn;
				countr=0;
			}
			else
			{
				countr=0;
				pdel=0;
				del=0;
				ret=0;
				if(fnfcn>=0)
				{
					pfcn=fcn=fnfcn;
					fstep=pfreq=fnstep;
				}
				else
				{
					pfreq=fnstep;
					del=-(int)fnfcn;
					pfcn=0;
				}
			}
		}
	}
	
	if(tv.ofs!=0xFFFF)
	{
		CWaveLevel const *pwave;
		pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
		if(pwave)
		{
			CWaveInfo const *pinfo;
			pinfo=pmi->pCB->GetWave(smp);
			int nums=pwave->numSamples;
			if(pinfo->Flags&WF_NOT16)
			{
				switch (pwave->pSamples[0])
				{
				case WFE_32BITI:
				case WFE_32BITF:nums=(pwave->numSamples-4)>>1;break;
				case WFE_24BIT:nums=(pwave->numSamples-4)*2/3;break;
				}
			}
			pfcn=(double)nums*tv.ofs/65536;
			fcn=(float)pfcn;
			if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
		}
	}

	if(tv.ec1!=0xFF)
	{
		switch(tv.ec1)
		{
		case 0x00:if(tv.ep1!=0xFFFF)ret=tv.ep1;break;
		case 0x01:if(tv.ep1!=0xFFFF)mamp=(float)tv.ep1/49152;break;
		case 0x02:
			pfreq/=mfreq;
			if(tv.ep1!=0xFFFF)mfreq=(float)tv.ep1/32768;
			pfreq*=mfreq;
			break;
		case 0x03:
			pfreq/=mfreq;
			if(tv.ep1!=0xFFFF)mfreq=(float)powd(SEMI,(float)(tv.ep1-32768)/256);break;
			pfreq*=mfreq;
			break;
		case 0x04:if(tv.ep1!=0xFFFF)mdelt=(float)tv.ep1/32768;break;
		case 0x05:fstep*=-1;break;
		case 0x06:
			if(tv.ep1!=0xFFFF)
			{
				porca=(tv.ep1&0x00FF)*spt/subdiv;
				int semt=(tv.ep1&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,-semt),1.0/porca); // freqm^porca=SEMI^semt
				else pfreq=fstep*=(float)powd(SEMI,-semt);
			}
			break;
		case 0x07:
			if(tv.ep1!=0xFFFF)
			{
				porca=(tv.ep1&0x00FF)*spt/subdiv;
				int semt=(tv.ep1&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,semt),1.0/porca);
				else pfreq=fstep*=(float)powd(SEMI,semt);
			}
			break;
		case 0x08:
			if(tv.ep1!=0xFFFF)
			{
				CWaveLevel const *pwave;
				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					int sdel;
					int ln=pwave->numSamples;
					sdel=(int)((float)spt*tv.ep1/subdiv);
					pfcn=fcn=(float)sdel*abbs(fstep)+(float)pfcn;
					pfreq=-abbs(pfreq);
					fstep=-abbs(fstep);
					if(pfcn>=ln)
					{
						del=(int)((pfcn-ln)/abbs(fstep));
						pfcn=(float)(ln-1);
						wstate=WSTATE_DIE;
						midi_note_playing=-1;
					}
				}
			}
			break;
		case 0x0A:
			if(tv.ep1!=0xFFFF)afcn=(float)tv.ep1*EDU-1;
			break;
		case 0x0B:
			if(tv.ep1!=0xFFFF)
			{
				ncmode=1;
				pncs=(float)tv.ep1*pmi->pMasterInfo->SamplesPerTick/subdiv/16;
				ncc=ncs=(int)pncs;
			}
			break;
		case 0x0C:
			if(tv.ep1!=0xFFFF)
			{
				ncmode=2;
				pncs=(float)tv.ep1*pmi->pMasterInfo->SamplesPerTick/subdiv/16;
				ncc=ncs=(int)pncs;
			}
			break;
		case 0x0D:if(tv.ep1!=0xFFFF)mncs=(float)tv.ep1/32768;break;
		case 0x0E:fstep*=-1;pfreq*=-1;break;
		case 0x10:
			if(tv.ep1!=0xFFFF)
			{
				if(pdel)
				{
					double lenstr=(double)(tv.ep1-32768)/subdiv*pmi->pMasterInfo->SamplesPerTick;
					if(lenstr!=0)afcn=floor(pdel)/lenstr;else afcn=0;
				}
			}
			break;
		case 0x11:
			if(tv.ep1!=0xFFFF)
			{
				imindel=tv.ep1;
				mindel=(float)imindel*pmi->pMasterInfo->SamplesPerTick/subdiv/256;
			}
			else
			{
				imindel=0;
				mindel=0;
			}
			break;
		case 0x12:
			if(tv.ep1!=0xFFFF)
			{
				imaxdel=tv.ep1;
				maxdel=(float)imaxdel*pmi->pMasterInfo->SamplesPerTick/subdiv;
			}
			else
			{
				imaxdel=0;
				maxdel=0;
			}
			break;
		case 0x13:
			if(tv.ep1!=0xFFFF)
			{
				pwr=(float)(tv.ep1-32768)*EDU*2;
			}
			break;
		case 0x14:
			if(tv.ep1!=0xFFFF)
			{
				if(pdel)
				{
					double lenstr=(double)(tv.ep1-32768)*pmi->pMasterInfo->SamplesPerTick;
					if(lenstr!=0)afcn=floor(pdel)/lenstr;else afcn=0;
				}
			}
			break;
		case 0x15:
			if(tv.ep1!=0xFFFF)
			{
				if(pdel)
				{
					keep_str=(tv.ep1-32768);
					double lenstr=(double)keep_str*pmi->pMasterInfo->SamplesPerTick;
					if(lenstr!=0)afcn=floor(pdel)/lenstr;else afcn=0;
				}
				else keep_str=0;
			}
			break;
		case 0x16:
			if(tv.ep1!=0xFFFF)
			{
				porca=(tv.ep1&0x00FF)*spt/subdiv;
				int semt=(tv.ep1&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,-(double)semt/256),1.0/porca); // freqm^porca=SEMI^semt
				else pfreq=fstep*=(float)powd(SEMI,-(double)semt/256);
			}
			break;
		case 0x17:
			if(tv.ep1!=0xFFFF)
			{
				porca=(tv.ep1&0x00FF)*spt/subdiv;
				int semt=(tv.ep1&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,(double)semt/256),1.0/porca);
				else pfreq=fstep*=(float)powd(SEMI,(double)semt/256);
			}
			break;
		case 0x18:
			if(tv.ep1!=0xFFFF)
			{
				float tgn=(float)(tv.ep1&0xFF)/64;
				adelcn=((tv.ep1&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(adelcn)adelt=(tgn-gn)/adelcn;
				else
				{
					gn=tgn;
				}
			}
			break;
		case 0x19:
			if(tv.ep1!=0xFFFF)
			{
				float tgn=(float)(tv.ep1&0xFF)/64;
				amulcn=((tv.ep1&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)
				{
					tgn=MAX(tgn,0.000001f);
					gn=MAX(gn,0.000001f);
					amul=(float)powd(tgn/gn,1.0/amulcn); // amul^amulcn = tgn/gn -> amul = (tgn/gn)^(1/amulcn)
				}
				else
				{
					gn=tgn;
				}
			}
			break;
		case 0x1A:
			if(tv.ep1!=0xFFFF)
			{
				float tgn=(float)powd(10,-(double)(tv.ep1&0xFF)*0.1);
				amulcn=((tv.ep1&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1B:
			if(tv.ep1!=0xFFFF)
			{
				float tgn=(float)powd(10,(double)(tv.ep1&0xFF)*0.1);
				amulcn=((tv.ep1&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1C:
			if(tv.ep1!=0xFFFF)
			{
				float tgn=(float)powd(10,-(double)(tv.ep1&0xFF)*0.000390625);
				amulcn=((tv.ep1&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1D:
			if(tv.ep1!=0xFFFF)
			{
				float tgn=(float)powd(10,(double)(tv.ep1&0xFF)*0.000390625);
				amulcn=((tv.ep1&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1F:
			if(tv.ep1!=0xFFFF)
			{
				mafcn=(float)powd(2.0,(double)(tv.ep1-32768)/16384);
			}
			break;
		case 0x30:
			if(tv.ep1!=0xFFFF)
			{
				filt=tv.ep1%6;
				dofl=true;
			}
			break;
		case 0x31:
			if(tv.ep1!=0xFFFF)
			{
				pcut=(float)((pmi->pMasterInfo->SamplesPerSec-100)*powd(2,(float)tv.ep1/8192-9));
				cut=pcut*hcutm;
				dofl=true;
			}
			break;
		case 0x32:
			if(tv.ep1!=0xFFFF)
			{
				res=pres=(float)tv.ep1/4096;
				dofl=true;
			}
			break;
		case 0x33:
			if(tv.ep1!=0xFFFF)
			{
				cutc=((tv.ep1&0x00FF)*spt/subdiv)>>8;
				if(cutc<1)cutc=1;
				int semt=(tv.ep1&0xFF00)>>8;
				cutm=(float)powd(powd(SEMI,-semt),1.0/cutc);
			}
			break;
		case 0x34:
			if(tv.ep1!=0xFFFF)
			{
				cutc=((tv.ep1&0x00FF)*spt/subdiv)>>8;
				if(cutc<1)cutc=1;
				int semt=(tv.ep1&0xFF00)>>8;
				cutm=(float)powd(powd(SEMI,+semt),1.0/cutc);
			}
			break;
		case 0x35:
			pcut/=mcut;
			if(tv.ep1!=0xFFFF)mcut=(float)tv.ep1/32768;
			pcut*=mcut;
			break;
		case 0x36:
			pcut/=mcut;
			if(tv.ep1!=0xFFFF)mcut=(float)powd(SEMI,(float)(tv.ep1-32768)/256);break;
			pcut*=mcut;
			break;
		case 0x40:
			if(tv.ep1!=0xFFFF)
			{
				float period=((float)(tv.ep1>>1)+(float)(tv.ep1&0x000F)/16)/subdiv*pmi->pMasterInfo->SamplesPerTick;
				vspd=(int)(powd(2,39)/period);
			}
			break;
		case 0x41:
			if(tv.ep1!=0xFFFF)
			{
				vdep=(float)tv.ep1*EDU*0.5f;
				if(vdep==0)vib=0;else vib=1;
			}
			break;
		case 0x42:
			if(tv.ep1!=0xFFFF)vwav=tv.ep1&3;
			break;
		case 0x43:
			if(tv.ep1!=0xFFFF)
			{
				float period=((float)(tv.ep1>>1)+(float)(tv.ep1&0x000F)/16)/subdiv*pmi->pMasterInfo->SamplesPerTick;
				lfospd=(int)(powd(2,39)/period);
			}
			break;
		case 0x44:
			if(tv.ep1!=0xFFFF)
			{
				lfodep=(float)tv.ep1*EDU*0.5f;
				if(lfodep==0)lfo=0;else lfo=1;
			}
			break;
		case 0x45:
			if(tv.ep1!=0xFFFF)lfowav=tv.ep1&3;
			break;
		case 0x50:
			vcnt=tv.ep1<<16;
			break;
		case 0x51:
			if(tv.ep1!=0xFFFF)syncv=tv.ep1;else syncv=-1;
			break;
		case 0x52:
			lfocnt=tv.ep1<<16;
			break;
		case 0x53:
			if(tv.ep1!=0xFFFF)synclfo=tv.ep1;else synclfo=-1;
			break;
		case 0x60:
			if(tv.ep1!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1/subdiv*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x61:
			if(tv.ep1!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1/subdiv*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x62:
			if(tv.ep1!=0xFFFF)
			{
				fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1*fstep/subdiv*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x63:
			if(tv.ep1!=0xFFFF)
			{
				fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1*fstep/subdiv*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x64:
			if(tv.ep1!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x65:
			if(tv.ep1!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x66:
			if(tv.ep1!=0xFFFF)
			{
				fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1*fstep*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x67:
			if(tv.ep1!=0xFFFF)
			{
				fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep1*fstep*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x70:
			if(tv.ep1!=0xFFFF)
			{
				int a=tv.ep1>>8;
				int b=tv.ep1&0x0F;
				if(a&0x01)SaveRet(b);
				if(a&0x02)SaveHum(b);
				if(a&0x04)SaveFil(b);
				if(a&0x08)SaveLFO(b);
			}
			break;
		case 0x71:
			if(tv.ep1!=0xFFFF)
			{
				int a=tv.ep1>>8;
				int b=tv.ep1&0x0F;
				if(a&0x01)LoadRet(b);
				if(a&0x02)LoadHum(b);
				if(a&0x04)LoadFil(b);
				if(a&0x08)LoadLFO(b);
			}
			break;
		case 0x72:
			if(tv.ep1!=0xFFFF)
			{
				int a=tv.ep1>>8;
				int b=tv.ep1&0x0F;
				if((b<pmi->numTracks)&&(b!=trn))
				{
					if(a&0x10)
					{
						int nsmp=pmi->Tracks[b].smp;
						poolused=pmi->Tracks[b].poolused;
						stretch=pmi->Tracks[b].stretch;
						if(poolused>0)nsmp=pmi->GetRndSamp(poolused);
						CWaveLevel const *pwave=pmi->pCB->GetWaveLevel(nsmp,0);
						if(pwave)
						{
							CWaveLevel const *pwav;
							pwav=pmi->pCB->GetWaveLevel(smp,0);
							if(stretch==0)if(pwav)fstep*=(float)pwav->SamplesPerSec/pwave->SamplesPerSec;
							smp=nsmp;
							sr=pwave->SamplesPerSec;
						}
						else
						{
							sr=44100;
							pdel=0;
							del=0;
							ret=0;
							smp=nsmp;
							amp=0;
							pan=tpan;
							wstate=WSTATE_OFF;
							midi_note_playing=-1;
							fcn=0;
						}
					}
					if(a&0x08)
					{
						vib=pmi->Tracks[b].vib;
						vcnt=pmi->Tracks[b].vcnt;
						vspd=pmi->Tracks[b].vspd;
						vwav=pmi->Tracks[b].vwav;
						vdep=pmi->Tracks[b].vdep;
						lfo=pmi->Tracks[b].lfo;
						lfocnt=pmi->Tracks[b].lfocnt;
						lfospd=pmi->Tracks[b].lfospd;
						lfowav=pmi->Tracks[b].lfowav;
						lfodep=pmi->Tracks[b].lfodep;
						syncv=pmi->Tracks[b].syncv;
						synclfo=pmi->Tracks[b].synclfo;
					}
					if(a&0x04)
					{
						pcut=pmi->Tracks[b].pcut;
						cut=pmi->Tracks[b].cut;
						hcutm=pmi->Tracks[b].hcutm;
						res=pmi->Tracks[b].res;
						filt=pmi->Tracks[b].filt;
						CalcCoef();
						ClearHis();
					}
					if(a&0x02)
					{
						rfcn=pmi->Tracks[b].rfcn;
						ramp=pmi->Tracks[b].ramp;
						rfreq=pmi->Tracks[b].rfreq;
						rpan=pmi->Tracks[b].rpan;
						rcut=pmi->Tracks[b].rcut;
						maxhums=pmi->Tracks[b].maxhums;
						maxhumsu=pmi->Tracks[b].maxhumsu;
						hcutm=pmi->Tracks[b].hcutm;
					}
					if(a&0x01)
					{
						mdelt=pmi->Tracks[b].mdelt;
						mamp=pmi->Tracks[b].mamp;
						mfreq=pmi->Tracks[b].mfreq;
						mncs=pmi->Tracks[b].mncs;
						mcut=pmi->Tracks[b].mcut;
						imaxdel=pmi->Tracks[b].imaxdel;
						imindel=pmi->Tracks[b].imindel;
						maxdel=pmi->Tracks[b].maxdel;
						mindel=pmi->Tracks[b].mindel;
						pwr=pmi->Tracks[b].pwr;
						afcn=pmi->Tracks[b].afcn;
					}
				}
			}
			break;
		case 0x78:
			if(tv.ep1<0x0010)chn=tv.ep1;
			if(tv.ep1==0xFFFF)chn=trn;
			SetDlgItemInt(pmi->thandle,tcoutputid[trn],chn,0);
			break;
		}
	}
	if(tv.ec2!=0xFF)
	{
		switch(tv.ec2)
		{
		case 0x00:if(tv.ep2!=0xFFFF)ret=tv.ep2;break;
		case 0x01:if(tv.ep2!=0xFFFF)mamp=(float)tv.ep2/49152;break;
		case 0x02:
			pfreq/=mfreq;
			if(tv.ep2!=0xFFFF)mfreq=(float)tv.ep2/32768;
			pfreq*=mfreq;
			break;
		case 0x03:
			pfreq/=mfreq;
			if(tv.ep2!=0xFFFF)mfreq=(float)powd(SEMI,(float)(tv.ep2-32768)/256);break;
			pfreq*=mfreq;
			break;
		case 0x04:if(tv.ep2!=0xFFFF)mdelt=(float)tv.ep2/32768;break;
		case 0x05:fstep*=-1;break;
		case 0x06:
			if(tv.ep2!=0xFFFF)
			{
				porca=(tv.ep2&0x00FF)*spt/subdiv;
				int semt=(tv.ep2&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,-semt),1.0/porca); // freqm^porca=SEMI^semt
				else pfreq=fstep*=(float)powd(SEMI,-semt);
			}
			break;
		case 0x07:
			if(tv.ep2!=0xFFFF)
			{
				porca=(tv.ep2&0x00FF)*spt/subdiv;
				int semt=(tv.ep2&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,semt),1.0/porca);
				else pfreq=fstep*=(float)powd(SEMI,semt);
			}
			break;
		case 0x08:
			if(tv.ep2!=0xFFFF)
			{
				CWaveLevel const *pwave;
				if(stretch)pwave=pmi->pCB->GetNearestWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					int sdel;
					int ln=pwave->numSamples;
					sdel=(int)((float)spt*tv.ep2/subdiv);
					pfcn=fcn=(float)sdel*abbs(fstep)+(float)pfcn;
					pfreq=-abbs(pfreq);
					fstep=-abbs(fstep);
					if(pfcn>=ln)
					{
						del=(int)((pfcn-ln)/abbs(fstep));
						pfcn=(float)(ln-1);
						wstate=WSTATE_DIE;
						midi_note_playing=-1;
					}
				}
			}
			break;
		case 0x0A:
			if(tv.ep2!=0xFFFF)afcn=(float)tv.ep2*EDU-1;
			break;
		case 0x0B:
			if(tv.ep2!=0xFFFF)
			{
				ncmode=1;
				pncs=(float)tv.ep2*pmi->pMasterInfo->SamplesPerTick/subdiv/16;
				ncc=ncs=(int)pncs;
			}
			break;
		case 0x0C:
			if(tv.ep2!=0xFFFF)
			{
				ncmode=2;
				pncs=(float)tv.ep2*pmi->pMasterInfo->SamplesPerTick/subdiv/16;
				ncc=ncs=(int)pncs;
			}
			break;
		case 0x0D:if(tv.ep2!=0xFFFF)mncs=(float)tv.ep2/32768;break;
		case 0x0E:fstep*=-1;pfreq*=-1;break;
		case 0x10:
			if(tv.ep2!=0xFFFF)
			{
				if(pdel)
				{
					double lenstr=(double)(tv.ep2-32768)/subdiv*pmi->pMasterInfo->SamplesPerTick;
					if(lenstr!=0)afcn=floor(pdel)/lenstr;else afcn=0;
				}
			}
			break;
		case 0x11:
			if(tv.ep2!=0xFFFF)
			{
				imindel=tv.ep2;
				mindel=(float)imindel*pmi->pMasterInfo->SamplesPerTick/subdiv/256;
			}
			else
			{
				imindel=0;
				mindel=0;
			}
			break;
		case 0x12:
			if(tv.ep2!=0xFFFF)
			{
				imaxdel=tv.ep2;
				maxdel=(float)imaxdel*pmi->pMasterInfo->SamplesPerTick/subdiv;
			}
			else
			{
				imaxdel=0;
				maxdel=0;
			}
			break;
		case 0x13:
			if(tv.ep2!=0xFFFF)
			{
				pwr=(float)(tv.ep2-32768)*EDU*2;
			}
			break;
		case 0x14:
			if(tv.ep2!=0xFFFF)
			{
				if(pdel)
				{
					double lenstr=(double)(tv.ep2-32768)*pmi->pMasterInfo->SamplesPerTick;
					if(lenstr!=0)afcn=floor(pdel)/lenstr;else afcn=0;
				}
			}
			break;
		case 0x15:
			if(tv.ep2!=0xFFFF)
			{
				if(pdel)
				{
					keep_str=(tv.ep2-32768);
					double lenstr=(double)keep_str*pmi->pMasterInfo->SamplesPerTick;
					if(lenstr!=0)afcn=floor(pdel)/lenstr;else afcn=0;
				}
				else keep_str=0;
			}
			break;
		case 0x16:
			if(tv.ep2!=0xFFFF)
			{
				porca=(tv.ep2&0x00FF)*spt/subdiv;
				int semt=(tv.ep2&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,-(double)semt/256),1.0/porca); // freqm^porca=SEMI^semt
				else pfreq=fstep*=(float)powd(SEMI,-(double)semt/256);
			}
			break;
		case 0x17:
			if(tv.ep2!=0xFFFF)
			{
				porca=(tv.ep2&0x00FF)*spt/subdiv;
				int semt=(tv.ep2&0xFF00)>>8;
				if(porca)freqm=(float)powd(powd(SEMI,(double)semt/256),1.0/porca);
				else pfreq=fstep*=(float)powd(SEMI,(double)semt/256);
			}
			break;
		case 0x18:
			if(tv.ep2!=0xFFFF)
			{
				float tgn=(float)(tv.ep2&0xFF)/64;
				adelcn=((tv.ep2&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(adelcn)adelt=(tgn-gn)/adelcn;
				else
				{
					gn=tgn;
				}
			}
			break;
		case 0x19:
			if(tv.ep2!=0xFFFF)
			{
				float tgn=(float)(tv.ep2&0xFF)/64;
				amulcn=((tv.ep2&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)
				{
					tgn=MAX(tgn,0.000001f);
					gn=MAX(gn,0.000001f);
					amul=(float)powd(tgn/gn,1.0/amulcn); // amul^amulcn = tgn/gn -> amul = (tgn/gn)^(1/amulcn)
				}
				else
				{
					gn=tgn;
				}
			}
			break;
		case 0x1A:
			if(tv.ep2!=0xFFFF)
			{
				float tgn=(float)powd(10,-(double)(tv.ep2&0xFF)*0.1);
				amulcn=((tv.ep2&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1B:
			if(tv.ep2!=0xFFFF)
			{
				float tgn=(float)powd(10,(double)(tv.ep2&0xFF)*0.1);
				amulcn=((tv.ep2&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1C:
			if(tv.ep2!=0xFFFF)
			{
				float tgn=(float)powd(10,-(double)(tv.ep2&0xFF)*0.000390625);
				amulcn=((tv.ep2&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1D:
			if(tv.ep2!=0xFFFF)
			{
				float tgn=(float)powd(10,(double)(tv.ep2&0xFF)*0.000390625);
				amulcn=((tv.ep2&0xFF00)>>8)*pmi->pMasterInfo->SamplesPerTick/subdiv;
				if(amulcn)amul=(float)powd(tgn,1.0/amulcn); // amul^amulcn = tgn -> amul = (tgn)^(1/amulcn)
				else
				{
					gn*=tgn;
				}
			}
			break;
		case 0x1F:
			if(tv.ep2!=0xFFFF)
			{
				mafcn=(float)powd(2.0,(double)(tv.ep2-32768)/16384);
			}
			break;
		case 0x30:
			if(tv.ep2!=0xFFFF)
			{
				filt=tv.ep2%6;
				dofl=true;
			}
			break;
		case 0x31:
			if(tv.ep2!=0xFFFF)
			{
				pcut=(float)((pmi->pMasterInfo->SamplesPerSec-100)*powd(2,(float)tv.ep2/8192-9));
				cut=pcut*hcutm;
				dofl=true;
			}
			break;
		case 0x32:
			if(tv.ep2!=0xFFFF)
			{
				res=pres=(float)tv.ep2/4096;
				dofl=true;
			}
			break;
		case 0x33:
			if(tv.ep2!=0xFFFF)
			{
				cutc=((tv.ep2&0x00FF)*spt/subdiv)>>8;
				if(cutc<1)cutc=1;
				int semt=(tv.ep2&0xFF00)>>8;
				cutm=(float)powd(powd(SEMI,-semt),1.0/cutc);
			}
			break;
		case 0x34:
			if(tv.ep2!=0xFFFF)
			{
				cutc=((tv.ep2&0x00FF)*spt/subdiv)>>8;
				if(cutc<1)cutc=1;
				int semt=(tv.ep2&0xFF00)>>8;
				cutm=(float)powd(powd(SEMI,+semt),1.0/cutc);
			}
			break;
		case 0x35:
			pcut/=mcut;
			if(tv.ep2!=0xFFFF)mcut=(float)tv.ep2/32768;
			pcut*=mcut;
			break;
		case 0x36:
			pcut/=mcut;
			if(tv.ep2!=0xFFFF)mcut=(float)powd(SEMI,(float)(tv.ep2-32768)/256);break;
			pcut*=mcut;
			break;
		case 0x40:
			if(tv.ep2!=0xFFFF)
			{
				float period=((float)(tv.ep2>>1)+(float)(tv.ep2&0x000F)/16)/subdiv*pmi->pMasterInfo->SamplesPerTick;
				vspd=(int)(powd(2,39)/period);
			}
			break;
		case 0x41:
			if(tv.ep2!=0xFFFF)
			{
				vdep=(float)tv.ep2*EDU*0.5f;
				if(vdep==0)vib=0;else vib=1;
			}
			break;
		case 0x42:
			if(tv.ep2!=0xFFFF)vwav=tv.ep2&3;
			break;
		case 0x43:
			if(tv.ep2!=0xFFFF)
			{
				float period=((float)(tv.ep2>>1)+(float)(tv.ep2&0x000F)/16)/subdiv*pmi->pMasterInfo->SamplesPerTick;
				lfospd=(int)(powd(2,39)/period);
			}
			break;
		case 0x44:
			if(tv.ep2!=0xFFFF)
			{
				lfodep=(float)tv.ep2*EDU*0.5f;
				if(lfodep==0)lfo=0;else lfo=1;
			}
			break;
		case 0x45:
			if(tv.ep2!=0xFFFF)lfowav=tv.ep2&3;
			break;
		case 0x50:
			vcnt=tv.ep2<<16;
			break;
		case 0x51:
			if(tv.ep2!=0xFFFF)syncv=tv.ep2;else syncv=-1;
			break;
		case 0x52:
			lfocnt=tv.ep2<<16;
			break;
		case 0x53:
			if(tv.ep2!=0xFFFF)synclfo=tv.ep2;else synclfo=-1;
			break;
		case 0x60:
			if(tv.ep2!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2/subdiv*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x61:
			if(tv.ep2!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2/subdiv*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x62:
			if(tv.ep2!=0xFFFF)
			{
				fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2*fstep/subdiv*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x63:
			if(tv.ep2!=0xFFFF)
			{
				fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2*fstep/subdiv*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x64:
			if(tv.ep2!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x65:
			if(tv.ep2!=0xFFFF)
			{
				const CWaveLevel *pwave;

				if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
				else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
				if(pwave)
				{
					fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2*pwave->SamplesPerSec/pmi->pMasterInfo->SamplesPerSec*GetMul(smp);
					if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
				}
			}
			break;
		case 0x66:
			if(tv.ep2!=0xFFFF)
			{
				fcn+=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2*fstep*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x67:
			if(tv.ep2!=0xFFFF)
			{
				fcn-=(float)pmi->pMasterInfo->SamplesPerTick*tv.ep2*fstep*GetMul(smp);
				if((wstate!=WSTATE_OFF)&&(ofd>0))offs=true;
			}
			break;
		case 0x70:
			if(tv.ep2!=0xFFFF)
			{
				int a=tv.ep2>>8;
				int b=tv.ep2&0x0F;
				if(a&0x01)SaveRet(b);
				if(a&0x02)SaveHum(b);
				if(a&0x04)SaveFil(b);
				if(a&0x08)SaveLFO(b);
			}
			break;
		case 0x71:
			if(tv.ep2!=0xFFFF)
			{
				int a=tv.ep2>>8;
				int b=tv.ep2&0x0F;
				if(a&0x01)LoadRet(b);
				if(a&0x02)LoadHum(b);
				if(a&0x04)LoadFil(b);
				if(a&0x08)LoadLFO(b);
			}
			break;
		case 0x72:
			if(tv.ep2!=0xFFFF)
			{
				int a=tv.ep2>>8;
				int b=tv.ep2&0x0F;
				if((b<pmi->numTracks)&&(b!=trn))
				{
					if(a&0x10)
					{
						int nsmp=pmi->Tracks[b].smp;
						poolused=pmi->Tracks[b].poolused;
						stretch=pmi->Tracks[b].stretch;
						if(poolused>0)nsmp=pmi->GetRndSamp(poolused);
						CWaveLevel const *pwave=pmi->pCB->GetWaveLevel(nsmp,0);
						if(pwave)
						{
							CWaveLevel const *pwav;
							pwav=pmi->pCB->GetWaveLevel(smp,0);
							if(stretch==0)if(pwav)fstep*=(float)pwav->SamplesPerSec/pwave->SamplesPerSec;
							smp=nsmp;
							sr=pwave->SamplesPerSec;
						}
						else
						{
							sr=44100;
							pdel=0;
							del=0;
							ret=0;
							smp=nsmp;
							amp=0;
							pan=tpan;
							wstate=WSTATE_OFF;
							midi_note_playing=-1;
							fcn=0;
						}
					}
					if(a&0x08)
					{
						vib=pmi->Tracks[b].vib;
						vcnt=pmi->Tracks[b].vcnt;
						vspd=pmi->Tracks[b].vspd;
						vwav=pmi->Tracks[b].vwav;
						vdep=pmi->Tracks[b].vdep;
						lfo=pmi->Tracks[b].lfo;
						lfocnt=pmi->Tracks[b].lfocnt;
						lfospd=pmi->Tracks[b].lfospd;
						lfowav=pmi->Tracks[b].lfowav;
						lfodep=pmi->Tracks[b].lfodep;
						syncv=pmi->Tracks[b].syncv;
						synclfo=pmi->Tracks[b].synclfo;
					}
					if(a&0x04)
					{
						pcut=pmi->Tracks[b].pcut;
						cut=pmi->Tracks[b].cut;
						hcutm=pmi->Tracks[b].hcutm;
						res=pmi->Tracks[b].res;
						filt=pmi->Tracks[b].filt;
						CalcCoef();
						ClearHis();
					}
					if(a&0x02)
					{
						rfcn=pmi->Tracks[b].rfcn;
						ramp=pmi->Tracks[b].ramp;
						rfreq=pmi->Tracks[b].rfreq;
						rpan=pmi->Tracks[b].rpan;
						rcut=pmi->Tracks[b].rcut;
						maxhums=pmi->Tracks[b].maxhums;
						maxhumsu=pmi->Tracks[b].maxhumsu;
						hcutm=pmi->Tracks[b].hcutm;
					}
					if(a&0x01)
					{
						mdelt=pmi->Tracks[b].mdelt;
						mamp=pmi->Tracks[b].mamp;
						mfreq=pmi->Tracks[b].mfreq;
						mncs=pmi->Tracks[b].mncs;
						mcut=pmi->Tracks[b].mcut;
						imaxdel=pmi->Tracks[b].imaxdel;
						imindel=pmi->Tracks[b].imindel;
						maxdel=pmi->Tracks[b].maxdel;
						mindel=pmi->Tracks[b].mindel;
						pwr=pmi->Tracks[b].pwr;
						afcn=pmi->Tracks[b].afcn;
					}
				}
			}
			break;
		case 0x78:
			if(tv.ep2<0x0010)chn=tv.ep2;
			if(tv.ep2==0xFFFF)chn=trn;
			SetDlgItemInt(pmi->thandle,tcoutputid[trn],chn,0);
			break;
		}
	}

	if(dofl)CalcCoef();

	VEnvPoints();
	PEnvPoints();
	CEnvPoints();
	REnvPoints();
	NEnvPoints();
}

void CTrack::PlayMIDINote(int note,int velocity)
{
	PlayNote(pmi->note2freq(note,sr)*(float)powd(2,(double)(pmi->aval.midi_transpose-48)/12),(float)velocity/127,tpan);
	fnstep=fstep;
	midi_age=0;
	midi_note_playing=note;
	for(int i=0;i<pmi->numTracks;i++)
	{
		if(i!=trn)
		{
			if(pmi->Tracks[i].midi_age<0x7FFFFFFF)pmi->Tracks[i].midi_age++;
		}
	}
}

// ==========================================================================================

// GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE

int CTrack::Generate(float *psamples, int numsamples)
{
	queue.step(numsamples);
	tvals ttv;
	while(queue.getev(ttv))DoTick(ttv,0);

	if((wstate!=WSTATE_OFF)||(del>0)||(shufflen>0))
	{
		if(midi_note_playing<0)bend=1;

		if(shufflen)
		{
			shcount-=numsamples;
			if(shcount<=0)
			{
				DoTick(shticks[shtickout],1);
				if(shtickout>=shufflen)
				{
					shcount=shortsh;
					if(shcount<0)shcount=0;
				}
				else
				{
					shcount=longsh;
					if(shcount<0)shcount=0;
				}
				shtickout=(shtickout+1)%(shufflen*2);
			}
		}

		CWaveLevel const *pwave;
		
		if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
		else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
		
		if(pwave)
		{
			int i,j;
			short *psample;
			const CWaveInfo *pinfo;
			int nsamp,flags,lst,lnd,lln;
			float outL,outR,ustep;
			float lfom=1;
			float apan=0;
			bool cc=false;
			
			pinfo=pmi->pCB->GetWave(smp);
			flags=pinfo->Flags;
			stereo=flags&WF_STEREO?true:false;
			loop=flags&WF_LOOP?1:0;
			loop=flags&WF_BIDIR_LOOP?2:loop;
			
			if(loop)
			{
				lst=pwave->LoopStart;
				lnd=pwave->LoopEnd;
				lln=lnd-lst;
			}

			psample=pwave->pSamples;
			nsamp=pwave->numSamples;

			if(spt!=pmi->pMasterInfo->SamplesPerTick)
			{
				spt=pmi->pMasterInfo->SamplesPerTick;
				if(stretch>0)fnstep=pfreq=fstep=(float)pwave->numSamples/spt/stretch;
			}
			if(sr!=pwave->SamplesPerSec)
			{
				fstep*=(float)sr/pwave->SamplesPerSec;
				sr=pwave->SamplesPerSec;
			}

			if(lfo)
			{
				lfocnt+=lfospd;
				switch(lfowav)
				{
				case 0:lfom=1+lfodep*sint[lfocnt>>17];break;
				case 1:lfom=1+lfodep*trit[lfocnt>>17];break;
				case 2:lfom=1+lfodep*squt[lfocnt>>17];break;
				case 3:lfom=1+lfodep*sawt[lfocnt>>17];break;
				}
			}

			if(cutc>0)
			{
				cutc--;
				pcut*=cutm;
				cut=pcut*hcutm*lfom;
				cc=true;
			}
			else if(lfo)
			{
				cut=pcut*lfom;
				cc=true;
			}

			if(vib)
			{
				vcnt+=vspd;
				switch(vwav)
				{
				case 0:ustep=fstep*(1+vdep*sint[vcnt>>17]);break;
				case 1:ustep=fstep*(1+vdep*trit[vcnt>>17]);break;
				case 2:ustep=fstep*(1+vdep*squt[vcnt>>17]);break;
				case 3:ustep=fstep*(1+vdep*sawt[vcnt>>17]);break;
				}
			}
			else ustep=fstep;

			float egn=pinfo->Volume;
			
			if(vesize)
			{
				int lasx,nexx;
				int lasy,nexy;
				int lasi,nexi;
				lasi=-1;
				nexi=-1;
				
				for(int j=0;j<vesize;j++)
				{
					if(vecn>=vepoint[j].x)lasi=j;
				}

				if((!erel)&&(vepoint[lasi].flags&EIF_SUSTAIN))
				{
					vecn=vepoint[lasi].x;
					egn*=(float)vepoint[lasi].y*0.5f*EDU;
				}
				else
				{
					lasx=vepoint[lasi].x;
					lasy=vepoint[lasi].y;
					nexi=lasi+1;
					if(nexi>=vesize)nexi=lasi;
					nexx=vepoint[nexi].x;
					nexy=vepoint[nexi].y;
					if(nexx!=lasx)egn*=((float)(vecn-lasx)/(nexx-lasx)*(nexy-lasy)+lasy)*0.5f*EDU;
					else egn*=(float)nexy*0.5f*EDU;
					vecn+=(int)((float)fnstep/nsamp*65536*128);
				}
			}

			if(pesize)
			{
				int lasx,nexx;
				int lasy,nexy;
				int lasi,nexi;
				lasi=-1;
				nexi=-1;
				
				for(int j=0;j<pesize;j++)
				{
					if(pecn>=pepoint[j].x)lasi=j;
				}

				if((!erel)&&(pepoint[lasi].flags&EIF_SUSTAIN))
				{
					pecn=pepoint[lasi].x;
					ustep*=(float)pepoint[lasi].y*0.5f*EDU;
				}
				else
				{
					lasx=pepoint[lasi].x;
					lasy=pepoint[lasi].y;
					nexi=lasi+1;
					if(nexi>=pesize)nexi=lasi;
					nexx=pepoint[nexi].x;
					nexy=pepoint[nexi].y;
					if(nexx!=lasx)ustep*=((float)(pecn-lasx)/(nexx-lasx)*(nexy-lasy)+lasy)*0.5f*EDU;
					else ustep*=(float)nexy*0.5f*EDU;
					pecn+=(int)((float)fnstep/nsamp*65536*128);
				}

			}

			if(nesize)
			{
				int lasx,nexx;
				int lasy,nexy;
				int lasi,nexi;
				lasi=-1;
				nexi=-1;
				
				for(int j=0;j<nesize;j++)
				{
					if(necn>=nepoint[j].x)lasi=j;
				}

				if((!erel)&&(nepoint[lasi].flags&EIF_SUSTAIN))
				{
					necn=nepoint[lasi].x;
					apan=(float)nepoint[lasi].y*EDU-1;
				}
				else
				{
					lasx=nepoint[lasi].x;
					lasy=nepoint[lasi].y;
					nexi=lasi+1;
					if(nexi>=nesize)nexi=lasi;
					nexx=nepoint[nexi].x;
					nexy=nepoint[nexi].y;
					if(nexx!=lasx)apan=((float)(necn-lasx)/(nexx-lasx)*(nexy-lasy)+lasy)*EDU-1;
					else apan=(float)nexy*EDU-1;
					necn+=(int)((float)fnstep/nsamp*65536*128);
				}

			}

			if(filt)
			{
				if(cesize)
				{
					int lasx,nexx;
					int lasy,nexy;
					int lasi,nexi;
					float cm=1;
					lasi=-1;
					nexi=-1;
					
					for(int j=0;j<cesize;j++)
					{
						if(cecn>=cepoint[j].x)lasi=j;
					}
					
					if((!erel)&&(cepoint[lasi].flags&EIF_SUSTAIN))
					{
						cecn=cepoint[lasi].x;
						cm=(float)cepoint[lasi].y*0.5f*EDU;
					}
					else
					{
						lasx=cepoint[lasi].x;
						lasy=cepoint[lasi].y;
						nexi=lasi+1;
						if(nexi>=cesize)nexi=lasi;
						nexx=cepoint[nexi].x;
						nexy=cepoint[nexi].y;
						if(nexx!=lasx)cm=((float)(cecn-lasx)/(nexx-lasx)*(nexy-lasy)+lasy)*0.5f*EDU;
						else cm=(float)nexy*0.5f*EDU;
						cecn+=(int)((float)fnstep/nsamp*65536*128);
					}
					if(cc)cut*=cm;
					else cut=pcut*cm;
					cc=true;
				}
				
				if(resize)
				{
					int lasx,nexx;
					int lasy,nexy;
					int lasi,nexi;
					float rm=1;
					lasi=-1;
					nexi=-1;
					
					for(int j=0;j<resize;j++)
					{
						if(recn>=repoint[j].x)lasi=j;
					}
					
					if((!erel)&&(repoint[lasi].flags&EIF_SUSTAIN))
					{
						recn=repoint[lasi].x;
						rm=(float)repoint[lasi].y*0.5f*EDU;
					}
					else
					{
						lasx=repoint[lasi].x;
						lasy=repoint[lasi].y;
						nexi=lasi+1;
						if(nexi>=resize)nexi=lasi;
						nexx=repoint[nexi].x;
						nexy=repoint[nexi].y;
						if(nexx!=lasx)rm=((float)(recn-lasx)/(nexx-lasx)*(nexy-lasy)+lasy)*0.5f*EDU;
						else rm=(float)nexy*0.5f*EDU;
						recn+=(int)((float)fnstep/nsamp*65536*128);
					}
					res=pres*rm;
					cc=true;
				}
			}

			if(cc)CalcCoef();

			// ---------------------------------
			// SAMPLE THINGIES WHAT ARE NOT TEH 16 BIT!!1:

			if(flags&WF_NOT16)
			{
				switch (psample[0])
				{
				case WFE_32BITF:
					psample=pwave->pSamples+8;
					if(pmi->aval.inter==0)interpolate=nearst32f;
					else if(pmi->aval.inter==1)interpolate=linear32f;
					else interpolate=spline32f;
					lst=((lst-4)>>1);
					lnd=((lnd-4)>>1);
					lln=lnd-lst;
					nsamp=((nsamp-4)>>1);
					if(stretch)ustep*=0.5f;
					wavetype=WFE_32BITF;
					egn*=32768;
					break;
				case WFE_32BITI:
					psample=pwave->pSamples+8;
					if(pmi->aval.inter==0)interpolate=nearst32i;
					else if(pmi->aval.inter==1)interpolate=linear32i;
					else interpolate=spline32i;
					lst=((lst-4)>>1);
					lnd=((lnd-4)>>1);
					lln=lnd-lst;
					nsamp=((nsamp-4)>>1);
					if(stretch)ustep*=0.5f;
					wavetype=WFE_32BITI;
					egn*=EDU*0.5f;
					break;
				case WFE_24BIT:
					psample=pwave->pSamples+8;
					if(pmi->aval.inter==0)interpolate=nearst24;
					else if(pmi->aval.inter==1)interpolate=linear24;
					else interpolate=spline24;
					/*lst=(int)((float)(lst-4)*0.66666666667f);
					lnd=(int)((float)(lnd-4)*0.66666666667f);*/
					lst=(lst-4)*2/3;
					lnd=(lnd-4)*2/3;
					lln=lnd-lst;
					//nsamp=(int)((float)(nsamp-4)*0.66666666667f);
					nsamp=(nsamp-4)*2/3;
					if(stretch)ustep*=0.66666666667f;
					wavetype=WFE_24BIT;
					egn*=EDU*0.5f;
					break;
				default:
					if(pmi->aval.inter==0)interpolate=nearst16;
					else if(pmi->aval.inter==1)interpolate=linear16;
					else interpolate=spline16;
					wavetype=WFE_16BIT;
					break;
				}
			}
			else
			{
				if(pmi->aval.inter==0)interpolate=nearst16;
				else if(pmi->aval.inter==1)interpolate=linear16;
				else interpolate=spline16;

				wavetype=WFE_16BIT;
			}

			float uegn=oegn;
			float egnstep=(egn-oegn)/numsamples;

			if(pmi->pbs)
			{
				if(bend<tbend)
				{
					bend*=pmi->bendm;
					if(bend>=tbend)
					{
						bend=tbend;
					}
				}
				else if(bend>tbend)
				{
					bend*=pmi->dbend;
					if(bend<=tbend)
					{
						bend=tbend;
					}
				}
			}

			float rnc=1;

			if(!stretch)
			{
				int ndif=buzznotediff(0x41,pwave->RootNote);
				rnc=(float)powd(SEMI,ndif);
			}

			ustep*=bend;
			ustep*=rnc;

			// ----------------------------------------------------------------------------------------------
			// LOOP -----------------------------------------------------------------------------------------
			
			for(i=0;i<numsamples;i++)
			{
				fnfcn+=fnstep;

				if(loop&&(!erel))
				{
					if(loop==1)
					{
						if(fcn>=lnd)
						{
							fcn-=lln;
							if(pmi->aval.decloop)if(ofd>0)offs=true;
						}
					}
					else
					{
						if((dir==1)&&(fcn>=lnd))
						{
							dir=-1;
							if(fcn>=lnd)fcn=(float)lnd-1;
							if(pmi->aval.decloop)if(ofd>0)offs=true;
						}
						else if((dir==-1)&&(fcn<=lst))
						{
							dir=1;
							if(fcn<lst)fcn=(float)lst;
							if(pmi->aval.decloop)if(ofd>0)offs=true;
						}
					}
				}
				else
				{
					if(fcn>=nsamp)
					{
						amp=0;
						pan=tpan;
						wstate=WSTATE_OFF;
						midi_note_playing=-1;
						fcn=0;
						if(!del)return 1;
					}
					else if(fcn<0)
					{
						amp=0;
						pan=tpan;
						wstate=WSTATE_OFF;
						midi_note_playing=-1;
						fcn=0;
						if(!del)return 1;
					}
				}
				
				if(del)
				{
					del--;
					if(del<=0)
					{
						if(ret>0)ret--;
						pdel*=mdelt;
						if(pdel<mindel)pdel=mindel;
						if(imaxdel)if(pdel>maxdel)pdel=maxdel;
						if(pdel<=1)
						{
							wstate=WSTATE_DIE;
							midi_note_playing=-1;
							ret=0;
							del=0;
						}
						if(ret!=0)del=(int)pdel;
						else del=0;
						countr++;
						if((del>0)&&(pwr!=0))
						{
							del=(int)((double)del*powd(countr,pwr));
						}
						if(del<mindel)del=(int)mindel;
						if(imaxdel)if(del>maxdel)del=(int)maxdel;
						if(keep_str)
						{
							if(pdel)
							{
								double lenstr=(double)keep_str*pmi->pMasterInfo->SamplesPerTick;
								if(lenstr!=0)afcn=(double)del/lenstr;else afcn=0;
							}
						}
						else afcn*=mafcn;
						pfcn+=(double)nsamp*afcn;
						if(loop)
						{
							while(pfcn>=lnd)pfcn-=lln;							
						}
						else
						{
							while(pfcn>=nsamp)pfcn-=nsamp;
						}
						while(pfcn<0)pfcn+=nsamp;
						pfreq*=mfreq;
						pcut*=mcut;
						if(filt)CalcCoef();
						PlayNote(pfreq,vel,tpan);
						if(ncmode!=0)
						{
							pncs*=mncs;
							if(pncs<=1)
							{
								wstate=WSTATE_DIE;
								midi_note_playing=-1;
								ret=0;
								del=0;
							}
							if(ret!=0)
							{
								ncc=ncs=(int)pncs;
							}
						}
						gn=pgn;
						pgn*=mamp;
						if(del!=0)
						{
							del+=(int)((float)rand()*EDU*maxhums);
							del+=(int)fround((float)rand()*EDU*maxhumsu*spt/subdiv);
						}
					}
				}

				// GEN.........................................\/

				if(wstate!=WSTATE_OFF)
				{
					j=i<<1;

					float upan=pan+hpan+apan;
					if(upan>2)upan=2;
					if(upan<0)upan=0;

					if(fcn>=nsamp)fcn=(float)nsamp-1;
					if(fcn<0)fcn=0;

					if(adelcn)
					{
						adelcn--;
						if(gn>=0.001f)gn+=adelt;else gn=0.001f;
					}

					if(amulcn)
					{
						amulcn--;
						gn*=amul;
					}

					float mmm=gn*uegn*amp;

					if(stereo)
					{
						outL=mmm*upan*interpolate(psample,nsamp,fcn,true,&outR);
						outR*=mmm*(2-upan);
					}
					else
					{
						float a=mmm*interpolate(psample,nsamp,fcn,false,0);
						outL=upan*a;
						outR=(2-upan)*a;
					}

					if(filt)
					{
						float temp;
						temp=coef[0]*outL+coef[1]*lhis[0]+coef[2]*lhis[1]+coef[3]*lhis[2]+coef[4]*lhis[3];
						lhis[1]=lhis[0];lhis[0]=outL;lhis[3]=lhis[2];lhis[2]=outL=temp;
						temp=coef[0]*outR+coef[1]*rhis[0]+coef[2]*rhis[1]+coef[3]*rhis[2]+coef[4]*rhis[3];
						rhis[1]=rhis[0];rhis[0]=outR;rhis[3]=rhis[2];rhis[2]=outR=temp;
					}

					lpr=outL-lprd;
					rpr=outR-rprd;

					if(offs)
					{
						lofs=lpr;
						rofs=rpr;
						lofs2=lprd;
						rofs2=rprd;
						dof=ofl=pmi->ofl;
						offs=false;
					}
					
					if(dof)
					{
						static float a,b;
						dof--;
						a=(float)dof/ofl;
						a*=a;a*=a;
						b=1.0f-a;
						outL=b*outL+lofs2*a;
						outR=b*outR+rofs2*a;
						lofs2+=(lpr/*+a*lofs*/)*b;
						rofs2+=(rpr/*+a*rofs*/)*b;
						if(dof<=0)
						{
							dof=0;
							lofs=rofs=0;
						}
					}

					lprd=outL;
					rprd=outR;

					if(trmute==0)
					{
						psamples[j]+=outL;
						psamples[j+1]+=outR;
						pmi->outbuf[chn*2][i]+=outL*EDU;
						pmi->outbuf[chn*2+1][i]+=outR*EDU;
					}
					
					fcn+=ustep*dir;

					if(porca>0)
					{
						porca--;
						pfreq=fstep*=freqm;
					}
					
					if(wstate==WSTATE_UP)
					{
						amp+=astep;
						if(amp>=tamp)
						{
							amp=tamp;
							wstate=WSTATE_ON;
						}
					}
					else if(wstate==WSTATE_DOWN)
					{
						amp-=astep;
						if(amp<=tamp)
						{
							amp=tamp;
							wstate=WSTATE_ON;
						}
					}
					else if(wstate==WSTATE_DIE)
					{
						amp-=astep;
						if(amp<=tamp)
						{
							amp=tamp;
							if(tamp==0)
							{
								pan=tpan;
								wstate=WSTATE_OFF;
								midi_note_playing=-1;
								fcn=0;
								if(!del)return 1;
							}
						}
					}
					
					if(pstate==PSTATE_RIGHT)
					{
						pan-=pstep;
						if(pan<=tpan)
						{
							pan=tpan;
							pstate=PSTATE_NO;
						}
					}
					else if(pstate==PSTATE_LEFT)
					{
						pan+=pstep;
						if(pan>=tpan)
						{
							pan=tpan;
							pstate=PSTATE_NO;
						}
					}

					if(ncmode==1)
					{
						ncc--;
						if(ncc<0)
						{
							tamp=0;
							wstate=WSTATE_DIE;
							midi_note_playing=-1;
							//fcn=0;
						}
					}
					else if(ncmode==2)
					{
						ncc--;
						if(ncc<0)Stop2();
					}

					uegn+=egnstep;
				}
				// LOOP -----------------------------------------------------------------------------------------
				// ----------------------------------------------------------------------------------------------

				oegn=egn;
			}
			return 1;
		}
	}

	midi_note_playing=-1;
	
	return 0;
}

// GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE GENERATE

// ==========================================================================================

void mi::Init(CMachineDataInput * const pi)
{
	ThisMachine=pCB->GetThisMachine();
	pCB->SetnumOutputChannels(ThisMachine,2);
	CMachineInfo *cminfo=(CMachineInfo *)pCB->GetMachineInfo(ThisMachine);

	pCB->SetMachineInterfaceEx(&ex);

	float smark=10000.0f/pMasterInfo->SamplesPerSec;

	CreateOutBufs();
	M_Lock();
	M_Offer(MAKELONG(MACH_MULTI_OUT,POLAC),this,(FARPROC)MachineCallback,ThisMachine,cminfo,0);
	M_Unlock();

	//MessageBox(GetDesktopWindow(),"test","holy shit!",0);
	//HWND test=GetDesktopWindow();

	if(!tab)
	{
		for(int i=0;i<32768;i++)sint[i]=(float)sin(EDU*2*PI*i);
		for(i=0;i<32768;i++)sawt[i]=(float)i*2*EDU-1;
		for(i=0;i<16384;i++)squt[i]=-1;
		for(i=16384;i<32768;i++)squt[i]=1;
		for(i=0;i<16384;i++)trit[i]=(float)i*4*EDU-1;
		for(i=16384;i<32768;i++)trit[i]=trit[32767-i];
		tab=true;
	}

	bendm=dbend=1;
	pbs=0;

	muted=false;

	for(int i=0;i<MAX_TRACKS;i++)
	{
		Tracks[i].pmi=this;
		Tracks[i].stopsl=0;
		Tracks[i].trmute=0;
		Tracks[i].trsolo=0;
		Tracks[i].trn=i;
		Tracks[i].oegn=0;
		Tracks[i].chn=i;
		Tracks[i].amp=0;
		Tracks[i].vel=1;
		Tracks[i].tamp=0;
		Tracks[i].astep=smark/aval.ramp;
		Tracks[i].pstep=smark/aval.rpan;
		Tracks[i].fcn=0;
		Tracks[i].acc_midi=1;
		Tracks[i].midi_age=0;
		Tracks[i].midi_note_playing=-1;
		Tracks[i].bend=1;
		Tracks[i].tbend=1;
		Tracks[i].afcn=0;
		Tracks[i].mafcn=1;
		Tracks[i].keep_str=0;
		Tracks[i].pfcn=0;
		Tracks[i].fstep=0;
		Tracks[i].stretch=0;
		Tracks[i].pan=1;
		Tracks[i].smp=1;
		Tracks[i].subdiv=6;
		Tracks[i].tpan=1;
		Tracks[i].sr=44100;
		Tracks[i].sps=pMasterInfo->SamplesPerSec;
		Tracks[i].spt=pMasterInfo->SamplesPerTick;
		Tracks[i].wstate=WSTATE_OFF;
		Tracks[i].pstate=PSTATE_NO;
		Tracks[i].del=0;
		Tracks[i].pdel=0;
		Tracks[i].ret=0;
		Tracks[i].stereo=false;
		Tracks[i].offs=false;
		Tracks[i].lofs=0;
		Tracks[i].rofs=0;
		Tracks[i].lofs2=0;
		Tracks[i].rofs2=0;
		Tracks[i].dof=0;
		Tracks[i].lpr=0;
		Tracks[i].rpr=0;
		Tracks[i].lprd=0;
		Tracks[i].rprd=0;
		Tracks[i].ofd=(float)powd(0.000001,smark/aval.rclk);
		Tracks[i].ofl=(int)((float)aval.rclk/smark);
		Tracks[i].pnote=0;
		Tracks[i].pfreq=0;
		Tracks[i].gn=1;
		Tracks[i].pgn=1;
		Tracks[i].mamp=1;
		Tracks[i].mdelt=1;
		Tracks[i].mfreq=1;
		Tracks[i].maxhums=0;
		Tracks[i].maxhumsu=0;
		Tracks[i].ramp=0;
		Tracks[i].rfcn=0;
		Tracks[i].rfreq=0;
		Tracks[i].rpan=0;
		Tracks[i].rcut=0;
		Tracks[i].freqm=1;
		Tracks[i].porca=0;
		Tracks[i].filt=0;
		Tracks[i].pcut=22000;
		Tracks[i].cut=22000;
		Tracks[i].hcutm=1;
		Tracks[i].pres=1;
		Tracks[i].res=1;
		Tracks[i].loop=0;
		Tracks[i].dir=1;
		Tracks[i].CalcCoef();
		Tracks[i].ClearHis();
		Tracks[i].vcnt=0;
		Tracks[i].vib=0;
		Tracks[i].vdep=0;
		Tracks[i].vspd=0;
		Tracks[i].vwav=0;
		Tracks[i].lfocnt=0;
		Tracks[i].lfo=0;
		Tracks[i].lfodep=0;
		Tracks[i].lfospd=0;
		Tracks[i].lfowav=0;
		Tracks[i].synclfo=-1;
		Tracks[i].syncv=-1;
		Tracks[i].erel=0;
		Tracks[i].ncmode=0;
		Tracks[i].ncc=0;
		Tracks[i].pncs=0;
		Tracks[i].ncs=0;
		Tracks[i].mncs=1;
		Tracks[i].mcut=1;
		Tracks[i].maxdel=0;
		Tracks[i].mindel=0;
		Tracks[i].imaxdel=0;
		Tracks[i].imindel=0;
		Tracks[i].countr=0;
		Tracks[i].pwr=0;
		Tracks[i].poolused=-1;
		Tracks[i].amul=1;
		Tracks[i].adelt=0;
		Tracks[i].adelcn=0;
		Tracks[i].amulcn=0;
		Tracks[i].shufflen=0;
		Tracks[i].shuffstr=0;
		Tracks[i].shtickin=0;
		Tracks[i].shtickout=0;
		Tracks[i].shcount=0;
		Tracks[i].shticks=NULL;
		Tracks[i].wavetype=WFE_16BIT;
		for(int j=0;j<16;j++)
		{
			Tracks[i].SaveFil(j);
			Tracks[i].SaveHum(j);
			Tracks[i].SaveLFO(j);
			Tracks[i].SaveRet(j);
		}
	}

	if(dhandle)
	{
		char fnord[16];
		midi2text2(aval.midi_mute_switch_start,fnord);
		SetDlgItemText(dhandle,IDC_MUNO,fnord);
		if(aval.midi_control_channel)sprintf(fnord,"%i",aval.midi_control_channel);else sprintf(fnord,"OFF");
		SetDlgItemText(dhandle,IDC_COMCHAN,fnord);
		midi2text2(aval.midi_solo_switch_start,fnord);
		SetDlgItemText(dhandle,IDC_SONO,fnord);
		midi2text2(aval.midi_reset_switch,fnord);
		SetDlgItemText(dhandle,IDC_RENO,fnord);
	}

	DoTrackM();

	lastpoolnum=0;

	for(i=0;i<32;i++)
	{
		smpsum[i]=0;
		for(int j=0;j<200;j++)smpool[i][j]=0;
		sprintf(poolname[i],"Pool %i",i);
	}

	if(pi)
	{
		/*

		int version

		then data:
			version 1:
				16 times, on each i:
						int: total number of entries in pool i
						One int for each of the 200 samples - how often it occurs in pool i
			version 2:
				16 times, on each i:
						int: total number of entries in pool i
						One int for each of the 200 samples - how often it occurs in pool i
						256 character string: name of the pool
			version 3:
				16 times, on each i:
						int: total number of entries in pool i
						if this is nonzero:	One int for each of the 200 samples - how often it occurs in pool i
											256 character string: name of the pool
			version 4:
				32 times, on each i:
						int: total number of entries in pool i
						if this is nonzero:	One int for each of the 200 samples - how often it occurs in pool i
											256 character string: name of the pool
			version 5:
				32 times, on each i:
						int: total number of entries in pool i
						if this is nonzero:	Byte, sample number, if 0xFF - stop
											Word, how often the sample occurs in pool i
											<=256 character ASCIIZ string: name of the pool
			version 6:
				[sample pools]
				32 times, on each i:
						int: total number of entries in pool i
						if this is nonzero:	Byte, sample number, if 0xFF - stop
											Word, how often the sample occurs in pool i
											Byte, use velocity layering?
												if nonzero: byte velomin, byte velomax
											Byte, use note layering?
												if nonzero: byte notemin, byte notemax [MIDI notes]
											Byte, base velocity
											Byte, base note [MIDI note]
											<=256 character ASCIIZ string: name of the pool

		*/

		int version;
		pi->Read(version);
		if(version==1)
		{
			for(i=0;i<16;i++)
			{
				pi->Read(smpsum[i]);
				for(int j=0;j<200;j++)pi->Read(smpool[i][j]);
			}
		}
		else if(version==2)
		{
			for(i=0;i<16;i++)
			{
				pi->Read(smpsum[i]);
				for(int j=0;j<200;j++)pi->Read(smpool[i][j]);
				pi->Read(poolname[i],256);
			}
		}
		else if(version==3)
		{
			for(i=0;i<16;i++)
			{
				pi->Read(smpsum[i]);
				if(smpsum[i])
				{
					for(int j=0;j<200;j++)pi->Read(smpool[i][j]);
					pi->Read(poolname[i],256);
				}
			}
		}
		else if(version==4)
		{
			for(i=0;i<32;i++)
			{
				pi->Read(smpsum[i]);
				if(smpsum[i])
				{
					for(int j=0;j<200;j++)pi->Read(smpool[i][j]);
					pi->Read(poolname[i],256);
				}
			}
		}
		else if(version==5)
		{
			for(i=0;i<32;i++)
			{
				pi->Read(smpsum[i]);
				if(smpsum[i])
				{
					byte temp=0xFF;

					do
					{
						word wtemp=0;
						pi->Read(temp);
						if(temp<200)
						{
							pi->Read(wtemp);
							smpool[i][temp]=wtemp;
						}
					}
					while(temp!=0xFF);
					
					char tempc=0;
					int ncnt=0;

					do
					{
						pi->Read(tempc);
						poolname[i][ncnt]=tempc;
						ncnt++;
					}
					while(tempc&&(ncnt<256));
				}
			}
		}
		else MessageBox(GetForegroundWindow(),"NOTE: This file has apparently been\nsaved with a newer version of UTrk!\n\nIt might not work as intended.","UTrk",0);
	}
	tchk=false;
	trackmlock=false;

	//MessageBox(GetDesktopWindow(),"test","holy shit!",0);
}

// ==========================================================================================

// ==========================================================================================

void mi::Save(CMachineDataOutput * const po)
{
	po->Write(5);
	for(int i=0;i<32;i++)
	{
		po->Write(smpsum[i]);
		if(smpsum[i])
		{
			for(int j=0;j<200;j++)
			{
				if(smpool[i][j])
				{
					po->Write((byte)j);
					po->Write((word)smpool[i][j]);
				}
			}
			po->Write((byte)0xFF);
			int ncnt=0;
			char c=0;
			do
			{
				c=poolname[i][ncnt];
				po->Write(c);
				ncnt++;
			}
			while(c&&(ncnt<256));
		}
	}
}

// ==========================================================================================

// ==========================================================================================

void mi::AttributesChanged()
{
	char fnord[16];
	midi2text2(aval.midi_mute_switch_start,fnord);
	SetDlgItemText(dhandle,IDC_MUNO,fnord);
	if(aval.midi_control_channel)sprintf(fnord,"%i",aval.midi_control_channel);else sprintf(fnord,"OFF");
	SetDlgItemText(dhandle,IDC_COMCHAN,fnord);
	midi2text2(aval.midi_solo_switch_start,fnord);
	SetDlgItemText(dhandle,IDC_SONO,fnord);
	midi2text2(aval.midi_reset_switch,fnord);
	SetDlgItemText(dhandle,IDC_RENO,fnord);
}

// ==========================================================================================

// ==========================================================================================

void mi::Tick()
{
	float smark;

	if(lock2)
	{
		lock2=false;
		DoTrackM();
	}

	if(tchk)CheckWaves();else tchk=true;

	smark=10000.0f/pMasterInfo->SamplesPerSec;
	astep=smark/aval.ramp;
	pstep=smark/aval.rpan;
	
	if(aval.rclk)
	{
		ofd=(float)powd(0.001,smark/aval.rclk);
		ofl=(int)((float)aval.rclk/smark);		
	}
	else
	{
		ofl=0;
		ofd=0;
	}

	if(aval.pitch_bend_speed!=pbs)
	{
		pbs=aval.pitch_bend_speed;
		if(pbs)
		{
			bendm=(float)powd(2,256000.0/(pMasterInfo->SamplesPerSec*pbs)); // bendm^(pbs*srate/256000)=2 -> bendm=2(256/(pbs*srate))
			dbend=1.0f/bendm;
		}
	}

	for(int i=0;i<numTracks;i++) Tracks[i].Tick(tval[i]);
}

// ==========================================================================================

// ==========================================================================================

bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
	if(mode==WM_READ)return false;
	if(mode==WM_NOIO)return false;

	int elephantmadeofscorpions=0;

	ZeroMemory(pout,2*MAX_BUFFER_LENGTH*sizeof(float));
	for(int i=0;i<32;i++)ZeroMemory(outbuf[i],MAX_BUFFER_LENGTH*sizeof(float));
	
	if(mode&WM_WRITE)
	{		
		for(i=0;i<numTracks;i++)elephantmadeofscorpions=elephantmadeofscorpions|Tracks[i].Generate(pout,numsamples);
	}
	
	if(elephantmadeofscorpions)for(i=0;i<2*numsamples;i++)if(abbs(pout[i])>1.e-12)return true;
	return false;
}

// ==========================================================================================

// ==========================================================================================

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];
	
	switch(param)
	{
	case 3:
		sprintf(txt,"%.1f%%",(float)value/655.36);
		return txt;
		break;
	case 11:
	case 9:
		switch(value)
		{
		case 0x00:return "Ret Break";break;
		case 0x01:return "Ret Feedback";break;
		case 0x02:return "Ret Freq Mul";break;
		case 0x03:return "Ret Transpose";break;
		case 0x04:return "Ret Time Exp";break;
		case 0x05:return "Rev Direct";break;
		case 0x06:return "Pitch Down";break;
		case 0x07:return "Pitch Up";break;
		case 0x08:return "Rev Target";break;
		case 0x09:return "Probability";break;
		case 0x0A:return "Ret Ofs";break;
		case 0x0B:return "Note Cut";break;
		case 0x0C:return "Note End";break;
		case 0x0D:return "Ret NCut";break;
		case 0x0E:return "Rev Dir +Ret";break;
		case 0x0F:return "To Note";break;
		case 0x10:return "Ret Stretch";break;
		case 0x11:return "Min Ret Time";break;
		case 0x12:return "Max Ret Time";break;
		case 0x13:return "Ret Time";break;
		case 0x14:return "Ret Stretch Tk";break;
		case 0x15:return "Keep Stretch Tk";break;
		case 0x16:return "Fine Down";break;
		case 0x17:return "Fine Up";break;
		case 0x18:return "Vol Slide";break;
		case 0x19:return "Log Vol Slide";break;
		case 0x1A:return "Log Vol down";break;
		case 0x1B:return "Log Vol up";break;
		case 0x1C:return "Fine Vol down";break;
		case 0x1D:return "Fine Vol up";break;
		case 0x1F:return "Ret Ofs Change";break;
		case 0x20:return "Hum Vel";break;
		case 0x21:return "Hum Pitch";break;
		case 0x22:return "Hum Ofs";break;
		case 0x23:return "Hum Pan";break;
		case 0x24:return "Hum Cut";break;
		case 0x25:return "Hum Del";break;
		case 0x26:return "Hum Del Qu";break;
		case 0x30:return "Filter";break;
		case 0x31:return "Cutoff";break;
		case 0x32:return "Resonance";break;
		case 0x33:return "Cut down";break;
		case 0x34:return "Cut up";break;
		case 0x35:return "Ret Cut Mul";break;
		case 0x36:return "Ret Cut Transpose";break;
		case 0x40:return "Vib Spd";break;
		case 0x41:return "Vib Dep";break;
		case 0x42:return "Vib Wav";break;
		case 0x43:return "Cut LFO Spd";break;
		case 0x44:return "Cut LFO Dep";break;
		case 0x45:return "Cut LFO Wav";break;
		case 0x50:return "Sync Vib";break;
		case 0x51:return "Sync Vib Note";break;
		case 0x52:return "Sync LFO";break;
		case 0x53:return "Sync LFO Note";break;
		case 0x60:return "Jmp-> (smp)";break;
		case 0x61:return "Jmp<- (smp)";break;
		case 0x62:return "Jmp-> (pl)";break;
		case 0x63:return "Jmp<- (pl)";break;
		case 0x64:return "Jmp-> (smp)";break;
		case 0x65:return "Jmp<- (smp,tk)";break;
		case 0x66:return "Jmp-> (pl,tk)";break;
		case 0x67:return "Jmp<- (pl,tk)";break;
		case 0x70:return "Save";break;
		case 0x71:return "Load";break;
		case 0x72:return "Copy";break;
		case 0x78:return "Channel";break;
		case 0x79:return "Accept MIDI";break;
		case 0x80:return "Shuffle";break;
		case 0x81:return "Shuffle 2";break;
		case 0x82:return "Shuffle 3";break;
		case 0x83:return "Shuffle 4";break;
		case 0xA0:return "Reset";break;
		case 0xA1:return "Reset Re";break;
		case 0xA2:return "Reset Hu";break;
		case 0xA3:return "Reset Re+Hu";break;
		case 0xA4:return "Reset Fl";break;
		case 0xA5:return "Reset Re+Fl";break;
		case 0xA6:return "Reset Hu+Fl";break;
		case 0xA7:return "Reset Re+Hu+Fl";break;
		case 0xA8:return "Reset LFOs";break;
		case 0xA9:return "Reset Re+LFOs";break;
		case 0xAA:return "Reset Hu+LFOs";break;
		case 0xAB:return "Reset Re+Hu+LFOs";break;
		case 0xAC:return "Reset Fl+LFOs";break;
		case 0xAD:return "Reset Re+Fl+LFOs";break;
		case 0xAE:return "Reset Hu+Fl+LFOs";break;
		case 0xAF:return "Reset";break;
		}
		return "what";
		break;
	case 6:
		if(value<=200)
		{
			return pCB->GetWaveName(value);
		}
		return poolname[value-201];
		break;
	case 7:
		sprintf(txt,"%.1f%%",(float)value/0.64);
		return txt;
		break;
	case 8:
		if(value==128)return "Right";
		if(value==0)return "Left";
		sprintf(txt,"%.1f%%",(float)value/1.28);
		return txt;
		break;
	default:
		return NULL;
		break;
	};	
}

// ==========================================================================================

// ==========================================================================================

void midi2text(int in,char* t)
{
	if(in==0)
	{
		sprintf(t,"%sOFF",t);
		return;
	}
	
	switch(in%12)
	{
	case 0:sprintf(t,"%sC-",t);;break;
	case 1:sprintf(t,"%sC#",t);;break;
	case 2:sprintf(t,"%sD-",t);;break;
	case 3:sprintf(t,"%sD#",t);;break;
	case 4:sprintf(t,"%sE-",t);;break;
	case 5:sprintf(t,"%sF-",t);;break;
	case 6:sprintf(t,"%sF#",t);;break;
	case 7:sprintf(t,"%sG-",t);;break;
	case 8:sprintf(t,"%sG#",t);;break;
	case 9:sprintf(t,"%sA-",t);;break;
	case 10:sprintf(t,"%sA#",t);;break;
	case 11:sprintf(t,"%sB-",t);;break;	
	}

	sprintf(t,"%s%i",t,in/12);
}

// ==========================================================================================

// ==========================================================================================

void midi2text2(int in,char* t)
{
	if(in==0)
	{
		sprintf(t,"OFF");
		return;
	}
	
	switch(in%12)
	{
	case 0:sprintf(t,"C-");;break;
	case 1:sprintf(t,"C#");;break;
	case 2:sprintf(t,"D-");;break;
	case 3:sprintf(t,"D#");;break;
	case 4:sprintf(t,"E-");;break;
	case 5:sprintf(t,"F-");;break;
	case 6:sprintf(t,"F#");;break;
	case 7:sprintf(t,"G-");;break;
	case 8:sprintf(t,"G#");;break;
	case 9:sprintf(t,"A-");;break;
	case 10:sprintf(t,"A#");;break;
	case 11:sprintf(t,"B-");;break;	
	}

	sprintf(t,"%s%i",t,in/12);
}

// ==========================================================================================

// ==========================================================================================

int text2midi(char* in)
{
	if(strlen(in)<3)return -1;

	int oct=in[2]-48;
	if(oct<0)return -1;
	if(oct>9)return -1;

	char tmp[2]={toupper(in[0]),in[1]};
	int nt;

	if(!strcmp(tmp,"C-"))nt=0;
	else if(!strcmp(tmp,"C#"))nt=1;
	else if(!strcmp(tmp,"D-"))nt=2;
	else if(!strcmp(tmp,"D#"))nt=3;
	else if(!strcmp(tmp,"E-"))nt=4;
	else if(!strcmp(tmp,"F-"))nt=5;
	else if(!strcmp(tmp,"F#"))nt=6;
	else if(!strcmp(tmp,"G-"))nt=7;
	else if(!strcmp(tmp,"G#"))nt=8;
	else if(!strcmp(tmp,"A-"))nt=9;
	else if(!strcmp(tmp,"A#"))nt=10;
	else if(!strcmp(tmp,"B-"))nt=11;
	else return -1;

	return nt+12*oct;
}

// ==========================================================================================

// ==========================================================================================

void mi::Command(int const i)
{
	static char text[128];
	int j;
	
	switch (i)
	{
	case 0:
		ShowWindow(dhandle,SW_SHOWNORMAL);
		break;
	case 1:
		DialogBoxParam(dllInstance,MAKEINTRESOURCE(IDD_SAMPOOL),GetForegroundWindow(),&PoolDialog,(LPARAM)this);
		break;
	case 2:
		ShowWindow(thandle,SW_SHOWNORMAL);
		break;
	case 3:
		for(j=0;j<MAX_TRACKS;j++)
		{
			Tracks[j].trsolo=0;
			Tracks[j].trmute=0;
		}
		DoTrackM();
		break;
	case 4:
		sprintf(text,"Track mute status:\n\n");
		for(j=0;j<numTracks;j++)
		{
			sprintf(text,"%s%i: ",text,j+1);
			if(Tracks[j].trmute)sprintf(text,"%sYes\n",text);
			else sprintf(text,"%sNo\n",text);
		}
		sprintf(text,"%s\nMute start key: ",text);
		midi2text(aval.midi_mute_switch_start,text);
		MessageBox(NULL,text,"Test",MB_OK|MB_SYSTEMMODAL);
		break;
	}
}

// ==========================================================================================

// ==========================================================================================

void mi::Stop()
{
	for(int i=0;i<MAX_TRACKS;i++)Tracks[i].Stop();
}

// ==========================================================================================

// ==========================================================================================

void CTrack::PlayNote(float freq,float velo,float pann)
{
	const CWaveLevel *pwave;

	midi_note_playing=-1;

	if(poolused>=0)
	{
		if(wstate!=WSTATE_OFF)
		{
			byte nsmp;
			CWaveLevel const *pwave;
						
			nsmp=pmi->GetRndSamp(poolused);
			
			if(stretch)pwave=pmi->pCB->GetWaveLevel(nsmp,0);
			else pwave=pmi->pCB->GetNearestWaveLevel(nsmp,midi2buzz(pnote));
			
			if(pwave)
			{
				CWaveLevel const *pwav;
				pwav=pmi->pCB->GetWaveLevel(smp,0);
				if(stretch==0)if(pwav)fstep*=(float)pwav->SamplesPerSec/pwave->SamplesPerSec;
				smp=nsmp;
				sr=pwave->SamplesPerSec;
			}
			else
			{
				sr=44100;
				pdel=0;
				del=0;
				ret=0;
				smp=nsmp;
				amp=0;
				pan=tpan;
				wstate=WSTATE_OFF;
				midi_note_playing=-1;
				fcn=0;
				return;
			}
		}
		else smp=pmi->GetRndSamp(poolused);
	}

	if((wstate==WSTATE_OFF)||(wstate==WSTATE_DIE))
	{
		lofs=rofs=0;dof=0;
		amp=0;
		wstate=WSTATE_UP;
	}
	else
	{
		if(amp<velo)wstate=WSTATE_UP;
		else if(amp>velo)wstate=WSTATE_DOWN;
		else wstate=WSTATE_ON;
	}

	if(ofd>0)offs=true;

	vecn=0;
	pecn=0;
	cecn=0;
	recn=0;
	necn=0;
	erel=0;

	/*if(velo>amp)wstate=WSTATE_UP;
	else if(velo<amp)wstate=WSTATE_DOWN;
	else wstate=WSTATE_ON;*/
	if(pann>pan)pstate=PSTATE_RIGHT;
	else if(pann<pan)pstate=PSTATE_LEFT;
	else pstate=PSTATE_NO;
	vel=velo;
	if(ramp>0)vel*=(float)powd(10,2*EDU*(rand()-16384)*ramp);
	tamp=vel;
	tpan=pann;
	hpan=0;
	if(rpan>0)hpan=rpan*2*EDU*(rand()-16384);
	fcn=(float)pfcn;
	if(stopsl)
	{
		porca=0;
		cutc=0;
		stopsl=0;
	}
	dir=1;
	if(syncv>=0)vcnt=(unsigned int)(syncv)<<16;
	if(synclfo>=0)lfocnt=(unsigned int)(synclfo)<<16;

	if(stretch)pwave=pmi->pCB->GetWaveLevel(smp,0);
	else pwave=pmi->pCB->GetNearestWaveLevel(smp,midi2buzz(pnote));
	if(pwave)
	{
		fstep=freq;
		if(rfreq>0)fstep*=(float)powd(SEMI,2*EDU*(rand()-16384)*rfreq);
		if(rfcn>0)fcn+=rfcn*EDU*rand()*pwave->numSamples;
		if(rcut>0)hcutm=(float)powd(SEMI,2*EDU*(rand()-16384)*rcut);
		else hcutm=1;
		cut=pcut*hcutm;CalcCoef();
	}
	else
	{
		wstate=WSTATE_OFF;
		midi_note_playing=-1;
		fcn=0;
	}
}

// ==========================================================================================

// ==========================================================================================

byte buzz2midi(byte bnote)
{
	return (bnote>>4)*12+(bnote&0x0F)-1;
}

// ==========================================================================================

// ==========================================================================================

byte midi2buzz(byte mnote)
{
	return ((mnote/12)<<4)+(mnote%12)+1;
}

// ==========================================================================================

// ==========================================================================================

int buzznotediff(int n1,int n2)
{
	return ((n1>>4)-(n2>>4))*12+((n1&0x0F)-(n2&0x0F));
}

// ==========================================================================================

// ==========================================================================================

float mi::note2freq(int note,int srate)
{
	return (float)powd(SEMI,note-48)*srate/pMasterInfo->SamplesPerSec;
}

// ==========================================================================================

// ==========================================================================================

inline float abbs(float in)
{
	int temp=(*(int *)(&in)&0x7FFFFFFF);
	return *(float *)&temp;
}

// ==========================================================================================

// ==========================================================================================

void mi::MidiNote(int const channel,int const value,int const velocity)
{
	if(((channel+1)==aval.midi_control_channel)&&(aval.midi_control_channel!=0))
	{
		if(value>=aval.midi_mute_switch_start)
			if(value<aval.midi_mute_switch_start+MAX_TRACKS)
				if(aval.midi_mute_switch_start!=0)
					if(velocity!=0)
					{
						int i=value-aval.midi_mute_switch_start;
						if(Tracks[i].trmute==0)Tracks[i].trmute=1;
						else Tracks[i].trmute=0;
					}
		if(value>=aval.midi_solo_switch_start)
			if(value<aval.midi_solo_switch_start+MAX_TRACKS)
				if(aval.midi_solo_switch_start!=0)
					if(velocity!=0)
					{
						int i=value-aval.midi_solo_switch_start;
						Tracks[i].trmute=0;
						if(Tracks[i].trsolo==0)
						{
							Tracks[i].trsolo=1;
							for(int j=0;j<numTracks;j++)
							{
								if(j!=i)
								{
									if(Tracks[j].trsolo)Tracks[j].trsolo=0;
									if(Tracks[j].trmute==0)Tracks[j].trmute=2;
								}
							}
						}
						else
						{
							Tracks[i].trsolo=0;
							for(int j=0;j<numTracks;j++)
							{
								Tracks[j].trsolo=0;
								if(Tracks[j].trmute==2)Tracks[j].trmute=0;
							}
						}
					}
		if(value==aval.midi_reset_switch)			
			if(aval.midi_reset_switch!=0)
				if(velocity!=0)
				{
					for(int i=0;i<MAX_TRACKS;i++)
					{
						Tracks[i].trsolo=0;
						Tracks[i].trmute=0;
					}
				}
	}

	DoTrackM();

	if(((channel+1)==(aval.midi_input_channel))&&(aval.midi_input_channel!=0))
	{
		if(velocity)
		{
			int tracknum=-1;
			int age=-1;
			for(int i=0;i<numTracks;i++)
			{
				if(Tracks[i].acc_midi)
				{
					if(Tracks[i].wstate==WSTATE_OFF)
					{
						age=0x7FFFFFFF;
						tracknum=i;
					}
					else if(Tracks[i].midi_age>age)
					{
						age=Tracks[i].midi_age;
						tracknum=i;
					}
				}
			}
			if(tracknum>=0)Tracks[tracknum].PlayMIDINote(value,velocity);
		}
		else
		{
			for(int i=0;i<numTracks;i++)Tracks[i].StopMIDINote(value);
		}
	}
}

// ==========================================================================================

// ==========================================================================================

void mi::UpdateSList(HWND shandle,int *sampnum,int *sampnum2)	// updates sample list for sample pools
{
	HWND lhandle=NULL;
	HWND phandle=NULL;
	if(shandle)lhandle=GetDlgItem(shandle,IDC_SAMPLES);
	if(shandle)phandle=GetDlgItem(shandle,IDC_INPOOL);

	if(lhandle)
	{
		SendMessage(lhandle,LB_RESETCONTENT,0,0);
	}

	if(phandle)
	{
		SendMessage(phandle,LB_RESETCONTENT,0,0);
	}

	int j=0;
	int k=0;

	CheckWaves();

	for(int i=1;i<=200;i++)
	{
		const CWaveLevel *pwave=pCB->GetWaveLevel(i,0);
		if(pwave)
		{
			sampnum[j]=i-1;j++;
			if(lhandle)
			{
				char *blonk=new char[256];
				strcpy(blonk,pCB->GetWaveName(i));
				SendMessage(lhandle,LB_ADDSTRING,0,(LPARAM)blonk);
				if(phandle)
				{
					int l=smpool[lastpoolnum][i-1];
					while(l>0)
					{
						SendMessage(phandle,LB_ADDSTRING,0,(LPARAM)blonk);
						sampnum2[k]=i-1;k++;
						l--;
					}
				}
				delete blonk;
			}
		}
	}
	
	RCSum();
}

// ==========================================================================================

// ==========================================================================================

void mi::RCSum()
{
	for(int j=0;j<32;j++)
	{
		smpsum[j]=0;
		for(int k=0;k<199;k++)smpsum[j]+=smpool[j][k];
	}
}

// ==========================================================================================

// ==========================================================================================

void mi::CheckWaves()
{
	for(int i=1;i<201;i++)
	{
		CWaveLevel const *pwave=pCB->GetWaveLevel(i,0);
		if(pwave==NULL)for(int j=0;j<32;j++)smpool[j][i-1]=0;
	}
	RCSum();
}

// ==========================================================================================

// ==========================================================================================

void mi::DoTrackM()
{
	//if(trackmlock)return;

	//MessageBox(GetDesktopWindow(),"test","holy shit!",0);
	
	trackmlock=true;

	if(!thandle)
	{
		trackmlock=false;
		return;
	}
	if(!dhandle)
	{
		trackmlock=false;
		return;
	}
	for(int i=0;i<numTracks;i++)
	{
		HWND box=GetDlgItem(thandle,tcmuteboxid[i]);
		if(!box)
		{
			trackmlock=false;
			return;
		}
		//SendMessage(box,BM_SETCHECK,(WPARAM)(Tracks[i].trmute?BST_CHECKED:BST_UNCHECKED),0);
		CheckDlgButton(thandle,tcmuteboxid[i],Tracks[i].trmute?BST_CHECKED:BST_UNCHECKED);
		ShowWindow(box,SW_SHOWNORMAL);
		box=GetDlgItem(thandle,tcsoloboxid[i]);
		if(!box)
		{
			trackmlock=false;
			return;
		}
		//SendMessage(box,BM_SETCHECK,(WPARAM)(Tracks[i].trsolo?BST_CHECKED:BST_UNCHECKED),0);
		CheckDlgButton(thandle,tcsoloboxid[i],Tracks[i].trsolo?BST_CHECKED:BST_UNCHECKED);
		ShowWindow(box,SW_SHOWNORMAL);
		ShowWindow(GetDlgItem(thandle,tclabelid[i]),SW_SHOWNORMAL);
		ShowWindow(GetDlgItem(thandle,tcoutputid[i]),SW_SHOWNORMAL);
		SetDlgItemInt(thandle,tcoutputid[i],Tracks[i].chn,0);
	}
	for(i=numTracks;i<16;i++)
	{
		HWND box=GetDlgItem(thandle,tcmuteboxid[i]);
		if(!box)
		{
			trackmlock=false;
			return;
		}
		ShowWindow(box,SW_HIDE);
		box=GetDlgItem(thandle,tcsoloboxid[i]);
		if(!box)
		{
			trackmlock=false;
			return;
		}
		ShowWindow(box,SW_HIDE);
		ShowWindow(GetDlgItem(thandle,tclabelid[i]),SW_HIDE);
		ShowWindow(GetDlgItem(thandle,tcoutputid[i]),SW_HIDE);
	}

	SetWindowPos(thandle,NULL,0,0,(int)((56+(numTracks-1)*14.5)*vbasex/4),57*vbasey/8,SWP_NOMOVE+SWP_NOZORDER);
	//SetWindowPos(thandle,NULL,0,0,((75+(numTracks-1)*19)*vbasex)/4,(57*vbasey)/8,SWP_NOMOVE+SWP_NOZORDER);

	SetDlgItemText(dhandle,IDC_T1,"");
	SetDlgItemText(dhandle,IDC_T2,"");
	SetDlgItemText(dhandle,IDC_T3,"");
	SetDlgItemText(dhandle,IDC_T4,"");
	SetDlgItemText(dhandle,IDC_T5,"");
	SetDlgItemText(dhandle,IDC_T6,"");
	SetDlgItemText(dhandle,IDC_T7,"");
	SetDlgItemText(dhandle,IDC_T8,"");
	SetDlgItemText(dhandle,IDC_T9,"");
	SetDlgItemText(dhandle,IDC_TA,"");
	SetDlgItemText(dhandle,IDC_TB,"");
	SetDlgItemText(dhandle,IDC_TC,"");
	SetDlgItemText(dhandle,IDC_TD,"");
	SetDlgItemText(dhandle,IDC_TE,"");
	SetDlgItemText(dhandle,IDC_TF,"");
	SetDlgItemText(dhandle,IDC_T0,"");

	if(Tracks[0].trmute==0)SetDlgItemText(dhandle,IDC_T1,"_");
	else SetDlgItemText(dhandle,IDC_T1,"0");
	if(numTracks<=1)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[1].trmute==0)SetDlgItemText(dhandle,IDC_T2,"_");
	else SetDlgItemText(dhandle,IDC_T2,"1");
	if(numTracks<=2)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[2].trmute==0)SetDlgItemText(dhandle,IDC_T3,"_");
	else SetDlgItemText(dhandle,IDC_T3,"2");
	if(numTracks<=3)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[3].trmute==0)SetDlgItemText(dhandle,IDC_T4,"_");
	else SetDlgItemText(dhandle,IDC_T4,"3");
	if(numTracks<=4)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[4].trmute==0)SetDlgItemText(dhandle,IDC_T5,"_");
	else SetDlgItemText(dhandle,IDC_T5,"4");
	if(numTracks<=5)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[5].trmute==0)SetDlgItemText(dhandle,IDC_T6,"_");
	else SetDlgItemText(dhandle,IDC_T6,"5");
	if(numTracks<=6)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[6].trmute==0)SetDlgItemText(dhandle,IDC_T7,"_");
	else SetDlgItemText(dhandle,IDC_T7,"6");
	if(numTracks<=7)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[7].trmute==0)SetDlgItemText(dhandle,IDC_T8,"_");
	else SetDlgItemText(dhandle,IDC_T8,"7");
	if(numTracks<=8)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[8].trmute==0)SetDlgItemText(dhandle,IDC_T9,"_");
	else SetDlgItemText(dhandle,IDC_T9,"8");
	if(numTracks<=9)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[9].trmute==0)SetDlgItemText(dhandle,IDC_TA,"_");
	else SetDlgItemText(dhandle,IDC_TA,"9");
	if(numTracks<=10)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[10].trmute==0)SetDlgItemText(dhandle,IDC_TB,"_");
	else SetDlgItemText(dhandle,IDC_TB,"A");
	if(numTracks<=11)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[11].trmute==0)SetDlgItemText(dhandle,IDC_TC,"_");
	else SetDlgItemText(dhandle,IDC_TC,"B");
	if(numTracks<=12)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[12].trmute==0)SetDlgItemText(dhandle,IDC_TD,"_");
	else SetDlgItemText(dhandle,IDC_TD,"C");
	if(numTracks<=13)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[13].trmute==0)SetDlgItemText(dhandle,IDC_TE,"_");
	else SetDlgItemText(dhandle,IDC_TE,"D");
	if(numTracks<=14)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[14].trmute==0)SetDlgItemText(dhandle,IDC_TF,"_");
	else SetDlgItemText(dhandle,IDC_TF,"E");
	if(numTracks<=15)
	{
		trackmlock=false;
		return;
	}

	if(Tracks[15].trmute==0)SetDlgItemText(dhandle,IDC_T0,"_");
	else SetDlgItemText(dhandle,IDC_T0,"F");

	trackmlock=false;
}

void miex::MidiControlChange(int const ctrl,int const channel,int const value)
{
	if(ctrl==255) // pitch bend
	{
		if((channel+1)==pmi->aval.midi_input_channel)
		{
			float bend0r=(float)powd(SEMI,((double)(value-8192))*EDU*pmi->aval.pitch_bend_range*0.04);
			for(int i=0;i<pmi->numTracks;i++)
			{
				if(pmi->Tracks[i].midi_note_playing>=0)
				{
					pmi->Tracks[i].tbend=bend0r;
					if(pmi->pbs==0)pmi->Tracks[i].bend=bend0r;
				}
			}
		}
	}
}