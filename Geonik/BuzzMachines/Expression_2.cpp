/*
 *		Expression 2 plug-in for Buzz
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

#include <MachineInterface.h>

#include "../DspClasses/DspClasses.h"
#include "../DspClasses/Saturator.h"
#include "../DspClasses/Filter.h"
#include "../DspClasses/Volume.h"

//#include "../Common/Shared.h"

typedef unsigned long  ulong;
typedef unsigned short uword;
typedef unsigned char  ubyte;

#define c_strName			"Expression 2"
#define c_strShortName		"Expression 2"

#define c_maxAmp			32768.0
#define c_numControls		6

#define c_maxLvl			1.0
#define c_minLvl			0.0005


/*
 *		Declarations and globals
 */

struct				 CMachine;

int					 g_iSampleRate;
CMachineInterface	*dspcMachine;

/*
 *		Parameters
 */

CMachineParameter const mpDrive		= { pt_byte,"Drive","Drive (0=-32dB, 40=0, 80=32dB, Default = 0dB)",0x00,0x80,0xFF,MPF_STATE,0x40-8 };
CMachineParameter const	mpFEnv		= {	pt_byte,"Filter Env","Filter envelope",0,0x80,0xFF,MPF_STATE,0x40 };
CMachineParameter const	mpBpRes		= {	pt_byte,"Hi Resonance","Stopband resonance",0,0x80,0xFF,MPF_STATE,0x40 };
CMachineParameter const	mpLpRes		= {	pt_byte,"Lo Resonance","Passband resonance",0,0x80,0xFF,MPF_STATE,0x08 };
CMachineParameter const	mpAttack	= { pt_byte,"Attack","Attack time",0,106,0xFF,MPF_STATE,0x14 };
CMachineParameter const	mpRelease	= { pt_byte,"Release","Release time",0,106,0xFF,MPF_STATE,0x30 };

CMachineParameter const *mpArray[]	= { &mpDrive,&mpFEnv,&mpBpRes,&mpLpRes,&mpAttack,&mpRelease };
enum					 mpValues	  { mpvDrive,mpvFEnv,mpvBpRes,mpvLpRes,mpvAttack,mpvRelease };
int const				 mpvC0		=   mpvDrive;

#pragma pack(1)		
struct CGlobalParameters			  { byte C[c_numControls]; };
struct CParameters					  { };
#pragma pack()


/*
 *		Attributes
 */

CMachineAttribute const  maDynRange	= { "Track available at (dB)",-120,-30,-50 };
CMachineAttribute const *maArray[]	= { &maDynRange };

#pragma pack(1)		
struct CAttributes					  { int DynRange; };
#pragma pack()


/*
 *		CMachineInfo
 */

CMachineInfo const miMachineInfo = { 
	MT_GENERATOR,MI_VERSION,0,
	0,0,c_numControls,0,mpArray,0,maArray,
	"Geonik's " c_strName,c_strShortName,"George Nicolaidis aka Geonik","About..." };

enum miCommands { micAbout };


/*
 *		Custom DSP classes
 */


/*
 *		General purpose functions
 */

double ControlByteToDouble(int const,byte const);	// Convert control byte


/*
 *		Machine class
 */

struct CMachine : public CMachineInterface {
				 CMachine();
				~CMachine();
	void		 Init(CMachineDataInput * const pi);
	void		 SetNumTracks(int const n);
	void		 AttributesChanged();
	char const	*DescribeValue(int const param, int const value);
	void		 Tick();
	bool		 Work(float *psamples, int numsamples, int const Mode);
	void		 Stop();
	void		 Command(int const);
	void		 About();
	void		 ControlChange(int const,double const);
	void		 Expression(float *,int const,double const);

	//CSharedResource		 cSharedResource;
	CGlobalParameters	 cGlobalParameters;
	CAttributes			 cAttributes;

	byte				 bControl[c_numControls];
	double				 fControl[c_numControls];

	CRms				 cRms;
	CSaturator			 cSaturator;
	//C2p2qFilter			 cFilter;
	Filter2p2q			 cFilter;

	double				 fLastLevel; };


/*
 *		Dll Exports
 */

extern "C" {
__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() {
	return &miMachineInfo; }
__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() {
	return new CMachine; } }


/*
 *		Machine members
 */

CMachine::CMachine() {
	GlobalVals	= &cGlobalParameters;
	AttrVals	= (int *)&cAttributes;
}

CMachine::~CMachine() { }

void CMachine::Init(CMachineDataInput * const pi) {
	g_iSampleRate	= pMasterInfo->SamplesPerSec;		// Dsp Classes
	dspcMachine		= this;
	for(int i=0; i<c_numControls; i++) {				// Controls
		CMachineParameter const *p = mpArray[i + mpvC0];
		bControl[i] = p->DefValue;
		fControl[i] = ControlByteToDouble(i,bControl[i]); }
	cRms.Configure(5);
 }

void CMachine::Command(int const i) {
	switch(i) {
	case micAbout:
		//About();
		break; }
 }


//#include "../Common/About.h"


void CMachine::SetNumTracks(int const n) {
 }

char const *CMachine::DescribeValue(int const pc, int const iv) {
	static char b[16];
	switch(pc) {
	case mpvDrive:
		sprintf(b,"%.1f dB", (float)(iv-64) * (1.0 / 2.0));
		break;
	case mpvFEnv:
		sprintf(b,"%.0f", (float)iv * (1000.0 / 128.0));
		break;
	case mpvLpRes:
	case mpvBpRes:
		sprintf(b,"%.1f%%", (float)iv * (100.0 / 128.0));
		break;
	case mpvAttack:
	case mpvRelease: {
		double v = pow(10.0,(double)(iv+14)*(1.0/20));
		if(v < 1000)
			sprintf(b,"%.1f ms", (float)v);
		else
			sprintf(b,"%.1f sec", (float)(v*(1.0/1000))); }
		break;
	default:
		sprintf(b,"%.3f", (float)(iv)); }
	return b;
 }

void CMachine::Tick() {
	for(int i=0; i<c_numControls; i++) {
		if(cGlobalParameters.C[i] != 0xff) {				// Waring: Hardcoded novalue
			bControl[i] = cGlobalParameters.C[i];
			fControl[i] = ControlByteToDouble(i,bControl[i]);
			ControlChange(i,fControl[i]); } }
 }

void CMachine::AttributesChanged() {
 }

void CMachine::ControlChange(int const cc, double const v) {
	switch(cc) {
	case mpvBpRes:
		cFilter.fBpRes = v; break;
	case mpvLpRes:
		cFilter.fLpRes = v; break; }
 }

void CMachine::Stop() {
 }


/*
 *		General purpose functions
 */

double ControlByteToDouble(int const cc, byte const b) {
	switch(cc) {
	case mpvDrive:	return pow(2.0,(double)(b-64)/12.0);
	case mpvFEnv:	return (double)b * (1.0 / (128*c_maxAmp));
	case mpvBpRes:
	case mpvLpRes:  return 1.0 - ((double)b)*(0.995/128);
	case mpvAttack:
	case mpvRelease: {
		double t = (double)g_iSampleRate * pow(10.0,(double)(b+14)*(1.0/20)) * (1.0 / 5000.0);
		if(t < 256) t = 256;
		return 1.0/t; }
	default: return ((double)b); }
 }


/*
 *		Worker functions
 */

#pragma optimize ("a", on)

inline void CMachine::Expression(float *pb, int const ns, double const step) {
	double const a  = fControl[mpvDrive];
	double		 L1 = cFilter.fLopass[0];
	double		 B1 = cFilter.fBdpass[0];
	double		 H1 = cFilter.fHipass[0];
	double		 L2 = cFilter.fLopass[1];
	double		 B2 = cFilter.fBdpass[1];
	double		 H2 = cFilter.fHipass[1];
	double const bq = cFilter.fBpRes;
	double const lq = cFilter.fLpRes;
	double		 l  = fLastLevel;
	int c = ns;
	do {
		L1 += l * B1;
		H1 = *pb - L1 - bq * B1;		// first pole input / BP resonance
		B1 += l * H1;
		l += step;
		L2 += l * B2;
		H2 = L1 - lq * L2 - B2;			// second pole input / LP resonance
		B2 += l * H2;
		*pb++ = (float)(L2*a);			// output
	} while(--c);
	cFilter.fLopass[0] = L1;
	cFilter.fBdpass[0] = B1;
	cFilter.fHipass[0] = H1;
	cFilter.fLopass[1] = L2;
	cFilter.fBdpass[1] = B2;
	cFilter.fHipass[1] = H2;
	fLastLevel = l;
}
	
bool CMachine::Work(float *pout, int ns, int const mode) {
	if(mode == WM_READWRITE) {
		double lvl = fControl[mpvFEnv] * sqrt(cRms.WorkSamples(pout,ns));
		if(lvl > c_maxLvl) lvl=c_maxLvl;
		if(lvl < c_minLvl) lvl=c_minLvl;
		if(lvl >= fLastLevel) Expression(pout,ns,(lvl-fLastLevel)*fControl[mpvAttack]);
		else				  Expression(pout,ns,(lvl-fLastLevel)*fControl[mpvRelease]);
		cSaturator.WorkSamples(pout,ns);
		return true;
  }
	else if(mode == WM_READ) {
		double lvl = fControl[mpvFEnv] * sqrt(cRms.WorkSamples(pout,ns));
		if(lvl > c_maxLvl) lvl=c_maxLvl;
		if(lvl < c_minLvl) lvl=c_minLvl;
		if(lvl >= fLastLevel)	fLastLevel += ns * (lvl-fLastLevel)*fControl[mpvAttack];
		else				fLastLevel += ns * (lvl-fLastLevel)*fControl[mpvRelease];
	}
	else {
		fLastLevel += ns * (c_minLvl-fLastLevel)*fControl[mpvRelease];
  }
	return false;
}

