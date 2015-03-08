/*
 *		Compressor plug-in for Buzz
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

//#include "../Common/Shared.h"

typedef unsigned long  ulong;
typedef unsigned short uword;
typedef unsigned char  ubyte;

#define c_strName			"Gate"
#define c_strShortName		"Gate"

#define c_maxAmp			32767.0
#define c_imaxAmp			(1.0 / 32767.0)

CMachineParameter const	mpThreshold	= {
	pt_byte,"Threshold","Threshold (Default=-18dB)",
	0,0x80,0xFF,MPF_STATE,92 };

CMachineParameter const	mpRatio	= {
	pt_byte,"Ratio","Expansion ratio (Default=3:1)",
	0,0xF0,0xFF,MPF_STATE,20 };

CMachineParameter const	mpAttack = {
	pt_word,"Attack","Attack Time (Default=10ms)",
	5,8000,0xFFFF,MPF_STATE,10 };

CMachineParameter const	mpRelease = {
	pt_word,"Release","Release Time (Default=2000ms)",
	30,8000,0xFFFF,MPF_STATE,2000 };

CMachineParameter const *mpArray[] = {
	&mpThreshold,&mpRatio,&mpAttack,&mpRelease };

enum mpValues { mpvThreshold = 0,mpvRatio,mpvAttack,mpvRelease };

CMachineAttribute const maRmsLength = {
	"Level Sensing Buffer",1,500,20 };

CMachineAttribute const maHard = {
	"Hard Limiter",0,1,0 };

CMachineAttribute const *maArray[]	= { &maRmsLength,&maHard };

CMachineInfo const MachineInfo = { 
	MT_EFFECT,MI_VERSION,0,
	0,0,4,0,mpArray,0,maArray,
	"Geonik's Gate","Gate","George Nicolaidis aka Geonik","About..." };

#pragma pack(1)		

class Parameters {
public:
	byte	Threshold;
	byte	Ratio;
	word	Attack;
	word	Release; };

class Attributes {
public:
	int		RmsLength;
	int		Hard; };

#pragma pack()

class CMachine : public CMachineInterface {
public:
				 CMachine();
				~CMachine();

	void		 Command(int const);
	void		 About();
	void		 Init(CMachineDataInput * const pi);
	void		 Tick();
	bool		 Work(float *psamples, int numsamples, int const mode);
	char const	*DescribeValue(int const param, int const value);
	void		 AttributesChanged();
	void		 InitRms();
	double		 DetectLevel(float *,int);
	void		 UpdateConsts();

	//CSharedResource		 cSharedResource;

	double		 RmsC1;
	double		 RmsC2;
	double		 RmsQ;

	double		 fiAttack;
	double		 fiRelease;

	double		 fLastGain;
	double		 fRatio;
	double		 fThreshold;
	double		 fBeta;

	Parameters	 Param;
	Attributes	 Attr; };

extern "C" {
__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() {
	return &MachineInfo; }
__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() {
	return new CMachine; } }

	
CMachine::CMachine() {
	GlobalVals	= &Param;
	AttrVals	= (int *)&Attr;  }


CMachine::~CMachine() { }


void CMachine::Command(int const i) {
	switch(i) {
	case 0:
		//About();
		break; }
 }


//#include "../Common/About.h"


void CMachine::InitRms() {
	double b	= 2.0 - cos(10 * 2 * PI / (double)pMasterInfo->SamplesPerSec);
	RmsC2		= b - sqrt(b * b - 1.0);
	RmsC1		= 1.0 - RmsC2;
	RmsQ		= 0; }


void CMachine::Init(CMachineDataInput * const pi) {
	fLastGain = 0;
	InitRms(); }


void CMachine::AttributesChanged() {
 }


char const *CMachine::DescribeValue(int const ParamNum, int const Value) {
	static char TxtBuffer[16];

	switch(ParamNum) {
	case mpvThreshold:
		sprintf(TxtBuffer,"%.1f dB", (float)(Value-128)/2.0); break;
	case mpvRatio:
		if(Value == 0xF0) return "Noise Gate";
		sprintf(TxtBuffer,"%.1f : 1", 1.0 + (double)Value * 0.1); break;
	case mpvAttack:
		sprintf(TxtBuffer,"%d ms", Value); break;
	case mpvRelease:
		sprintf(TxtBuffer,"%d ms", Value); break;
	default:
		return NULL; }
	return TxtBuffer; }


void CMachine::Tick() {

	if(Param.Threshold != mpThreshold.NoValue) {
		fThreshold = pow(2.0,(double)(Param.Threshold-128) / 12.0);
		UpdateConsts(); }

	if(Param.Ratio != mpRatio.NoValue) {
		fRatio = 1.0 + 0.1 * (double)Param.Ratio;
		UpdateConsts(); }

	if(Param.Attack != mpAttack.NoValue) {
		double f = (int)floor(((double)Param.Attack * (double)pMasterInfo->SamplesPerSec) / 5000.0);
		if(f < MAX_BUFFER_LENGTH) f = MAX_BUFFER_LENGTH;
		fiAttack = 1.0 / (double)f; }

	if(Param.Release != mpRelease.NoValue) {
		double f = (int)floor(((double)Param.Release * (double)pMasterInfo->SamplesPerSec) / 5000.0);
		if(f < MAX_BUFFER_LENGTH) f = MAX_BUFFER_LENGTH;
		fiRelease = 1.0 / (double)f; }
 }


void CMachine::UpdateConsts() {
	fBeta = - fThreshold * (fRatio - 1.0); }


#pragma optimize ("a", on)


inline double CMachine::DetectLevel(float *pb, int ns) {

	double const c1 = RmsC1;
	double const c2 = RmsC2;

	double		 q  = RmsQ;

	if (ns >= 4) {
		int cnt = ns >> 2;
		do {
			q = c1 * pb[0] * pb[0] + c2 * q;
			q = c1 * pb[1] * pb[1] + c2 * q;
			q = c1 * pb[2] * pb[2] + c2 * q;
			q = c1 * pb[3] * pb[3] + c2 * q;
			pb += 4; } while(--cnt); }
	int cnt = ns & 3;
	while(cnt--) {
		double const v = *pb++;
		q = c1 * v * v + c2 * q; }

	RmsQ = q; return sqrt(q); }


bool CMachine::Work(float *pb, int ns, int const Mode) {

	if(!(Mode & WM_READ)) return false;

	double	g = fLastGain;
	double	x = c_imaxAmp * DetectLevel(pb,ns);
	double  y = fRatio * x + fBeta;

	if(y > fThreshold)	y = 1.0;
	else if(y < 0.0)	y = 0.0;
	else if(x) y /= x;
	else y = 0;

	double stp = y > g ? (y-g)*fiAttack : (y-g)*fiRelease;

	if(!(Mode & WM_WRITE)) {
		fLastGain += stp * ns;
		return true; }

	if (ns >= 4) {
		int cnt = ns >> 2;
		do {
			pb[0] *= (float)g;
			g += stp;
			pb[1] *= (float)g;
			g += stp;
			pb[2] *= (float)g;
			g += stp;
			pb[3] *= (float)g;
			g += stp;
			pb += 4; } while(--cnt); }
	int cnt = ns & 3;
	while(cnt--) {
		*pb++ *= (float)(g);
		g += stp; }

	fLastGain = g;
	return true; }
