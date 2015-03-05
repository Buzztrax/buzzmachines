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

#define c_strName			"Compressor"
#define c_strShortName		"Compressor"

CMachineParameter const	mpThreshold	= {
	pt_byte,"Threshold","Threshold (Default=-18dB)",
	0,0x80,0xFF,MPF_STATE,92 };

CMachineParameter const	mpRatio	= {
	pt_byte,"Ratio","Compression ratio (Default=3:1)",
	0,0xF0,0xFF,MPF_STATE,20 };

CMachineParameter const	mpAttack = {
	pt_word,"Attack","Attack Time (Default=10ms)",
	5,8000,0xFFFF,MPF_STATE,10 };

CMachineParameter const	mpRelease = {
	pt_word,"Release","Release Time (Default=2000ms)",
	30,8000,0xFFFF,MPF_STATE,2000 };

CMachineParameter const	mpGain = {
	pt_byte,"Gain","Output Gain (Default=3dB)",
	0,0x80,0xFF,MPF_STATE,6 };

CMachineParameter const *mpArray[] = {
	&mpThreshold,&mpRatio,&mpAttack,&mpRelease,&mpGain };

enum mpValues { mpvThreshold = 0,mpvRatio,mpvAttack,mpvRelease,mpvGain };

CMachineAttribute const maRmsLength = {
	"Level Sensing Buffer",1,500,20 };

CMachineAttribute const maHard = {
	"Hard Limiter",0,1,0 };

CMachineAttribute const *maArray[]	= { &maRmsLength,&maHard };

CMachineInfo const MachineInfo = { 
	MT_EFFECT,MI_VERSION,0,
	0,0,5,0,mpArray,2,maArray,
	"Geonik's Compressor","Compressor","George Nicolaidis aka Geonik","About..." };

#pragma pack(1)		

class Parameters {
public:
	byte	Threshold;
	byte	Ratio;
	word	Attack;
	word	Release;
	byte	Gain; };

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
	double		 DetectPeak(float *,int);

	//CSharedResource		 cSharedResource;

	double		 RmsC1;
	double		 RmsC2;
	double		 RmsQ;

	double		 Threshold;
	double		 InvRatio;
	double		 OutputGain;
	double		 Gain;
	int			 AttackSamples;
	int			 ReleaseSamples;
	double		 AttackSamples_inv;
	double		 ReleaseSamples_inv;

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
/*	AttackSamples_inv	= 0.0004535;
	ReleaseSamples_inv	= 0.00001134;
	Gain				= 1.0;
	Threshold			= 4096;
	InvRatio			= 0.5;
	OutputGain			= 1.414; */
	InitRms(); }


void CMachine::AttributesChanged() {
	InitRms(); }


char const *CMachine::DescribeValue(int const ParamNum, int const Value) {
	static char TxtBuffer[16];

	switch(ParamNum) {
	case mpvThreshold:
		sprintf(TxtBuffer,"%.1f dB", (float)(Value-128)/2.0); break;
	case mpvRatio:
		sprintf(TxtBuffer,"%.1f : 1", 1.0 + (double)Value * 0.1); break;
	case mpvAttack:
		sprintf(TxtBuffer,"%d ms", Value); break;
	case mpvRelease:
		sprintf(TxtBuffer,"%d ms", Value); break;
	case mpvGain:
		sprintf(TxtBuffer,"%.1f dB", ((float)Value/2.0)); break;
	default:
		return NULL; }
	return TxtBuffer; }


void CMachine::Tick() {

	if(Param.Threshold != mpThreshold.NoValue) {
		Threshold = 32768 * pow(2.0,(double)(Param.Threshold-128) / 12.0); }

	if(Param.Ratio != mpRatio.NoValue) {
		InvRatio = 1.0 / (1.0 + 0.1 * (double)Param.Ratio); }

	if(Param.Attack != mpAttack.NoValue) {
		AttackSamples = (int)floor(((double)Param.Attack * (double)pMasterInfo->SamplesPerSec) / 5000.0);
		AttackSamples_inv = 1.0 / (double)AttackSamples; }

	if(Param.Release != mpRelease.NoValue) {
		ReleaseSamples = (int)floor(((double)Param.Release * (double)pMasterInfo->SamplesPerSec) / 5000.0);
		ReleaseSamples_inv = 1.0 / (double)ReleaseSamples; }

	if(Param.Gain != mpThreshold.NoValue) {
		OutputGain = pow(2.0,(double)Param.Gain / 12.0); } }


#pragma optimize ("a", on)


inline double CMachine::DetectLevel(float *inb, int ns) {

	double const c1 = RmsC1;
	double const c2 = RmsC2;

	double		 q  = RmsQ;

	do {
		double v = *inb++;
		q = c1 * v * v + c2 * q; } while(--ns);

	RmsQ = q; return sqrt(q); }


inline double CMachine::DetectPeak(float *inb, int ns) {
	double p = 0;
	do {
		p = __max(fabs(*inb++),p); } while(--ns);
	return p; }


bool CMachine::Work(float *pb, int numSamples, int const Mode) {

	if(Mode != WM_READWRITE) {
		if(!(Mode & WM_READ)) return false;
		double g = Gain;
		double m = Attr.Hard ? DetectPeak(pb,numSamples) : DetectLevel(pb,numSamples);
		double stp,lv;
		if((lv = m-Threshold) > 0)
			stp = ((Threshold + lv*InvRatio)/m - g)*AttackSamples_inv;
		else
			stp = (1.0 - g)*ReleaseSamples_inv;
		Gain += stp * numSamples;
		return false; }

	double		 g	= Gain;

	if(InvRatio==1 && g>0.995) {
		double const o = OutputGain;
		if(o > 1.0) do {
			*pb++ *= (float)o; } while(--numSamples);
		return true; }

	double		 m	= Attr.Hard ? DetectPeak(pb,numSamples) : DetectLevel(pb,numSamples);
	double		 stp,lv;
	double const o	= OutputGain;

	if((lv = m-Threshold) > 0) {
		stp = ((Threshold + lv*InvRatio)/m - g)*AttackSamples_inv;
		if(AttackSamples < numSamples) {
			int ns = AttackSamples;
			do {
				double v = *pb;
				*pb++ = (float)(g*o*v);
				g += stp; } while(--ns);
				numSamples -= AttackSamples; stp = 0; } }
	else {
		stp = (1.0 - g)*ReleaseSamples_inv;
		if(ReleaseSamples < numSamples) {
			int ns = ReleaseSamples;
			do {
				double v = *pb;
				*pb++ = (float)(g*o*v);
				g += stp; } while(--ns);
				numSamples -= ReleaseSamples; stp = 0; } }

	do {
		double v = *pb;
		*pb++ = (float)(g*o*v);
		g += stp; } while(--numSamples);

	Gain = g;

	return true; }
