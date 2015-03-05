/*
 *		Overdrive plug-in for Buzz
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

#define c_strName			"Overdrive"
#define c_strShortName		"Overdrive"

#define StdVol		32768

CMachineParameter const mpDrive	= {
	pt_byte,"Drive","Drive amp in dB (30=-16dB, 40=0, 60=32dB, Default = 17dB)",
	48,96,0xFF,MPF_STATE,81 };

CMachineParameter const mpAssym = {
	pt_byte,"Assymetry","Assymetry (0-80, Default = 44)",
	0,0x80,0xFF,MPF_STATE,44 };

CMachineParameter const mpCutOff = {
	pt_byte,"CutOff","CutOff frequency (0-80, Default = 108)",
	0,0x80,0xFF,MPF_STATE,108 };

CMachineParameter const mpEmphasis = {
	pt_byte,"Emphasis","Emphasis (0-80, Default = 95)",
	0,0x80,0xFF,MPF_STATE,95 };

CMachineParameter const *mpArray[] = {
	&mpDrive,&mpAssym,&mpCutOff,&mpEmphasis };

enum mpValues { mpvDrive,mpvAssym,mpvCutOff,mpvEmphasis };

CMachineAttribute const maHighFreq	= {
	"High Frequency Lfo",0,1,0 };

CMachineAttribute const *maArray[]	= { &maHighFreq };

CMachineInfo const MachineInfo = { 
	MT_EFFECT,MI_VERSION,0,
	0,0,4,0,mpArray,0,maArray,
	"Geonik's Overdrive","Overdrive","George Nicolaidis aka Geonik","About..." };

#pragma pack(1)		

class Parameters {
public:
	byte	Drive;
	byte	Assym;
	byte	CutOff;
	byte	Emphasis; };

class Attributes {
public:
	int		HighFreq; };

#pragma pack()

class CMachine : public CMachineInterface {
public:
				 CMachine();
				~CMachine();

	void		 Init(CMachineDataInput * const pi);
	void		 Tick();
	bool		 Work(float *psamples, int numsamples, int const Mode);
	char const	*DescribeValue(int const param, int const value);
	void		 Command(int const);
	void		 About();

	//CSharedResource		 cSharedResource;

private:
	void				 Filter(float *psamples, int numsamples);

private:
	double			DriveAmp;
	double			Assymetry;
	double			Correction;
	double			Volume;

	double			CutOff;
	double			Resonance;
	double			Lowpass;
	double			Bandpass;
	double			Highpass;
	double			DcLastX;
	double			DcLastY;

	Parameters		Param;
	Attributes		Attr; };

extern "C" {
__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() {
	return &MachineInfo; }
__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() {
	return new CMachine; } }

	
CMachine::CMachine() {
	GlobalVals	= &Param;
	AttrVals	= (int *)&Attr; }


CMachine::~CMachine() { }


void CMachine::Command(int const i) {
	switch(i) {
	case 0:
		//About();
		break; }
 }


//#include "../Common/About.h"


void CMachine::Init(CMachineDataInput * const pi) {
	DriveAmp	= 7.0 * (1.0/(StdVol*2));
	Assymetry	= 1.7;
	Correction	= -1.33875;
	CutOff		= 108.0/127.0;
	Resonance	= 0.29;
	Volume		= StdVol/2;
	Lowpass = Bandpass = Highpass = DcLastX = DcLastY = 0; }


char const *CMachine::DescribeValue(int const ParamNum, int const Value) {
	static char TxtBuffer[16];

	switch(ParamNum) {
	case mpvDrive:
		sprintf(TxtBuffer,"%d dB", Value-64); break;
	case mpvAssym:
	case mpvCutOff:
	case mpvEmphasis:
		sprintf(TxtBuffer,"%.1f%%", (float)Value * (100.0 / 128.0)); break;
	default:
		return NULL; }
	return TxtBuffer; }


void CMachine::Tick() {

	if(Param.Drive != mpDrive.NoValue) { 
		DriveAmp = pow(2.0,(double)(Param.Drive-64)/6.0) * (1.0/(StdVol*2)); }

	if(Param.Assym != mpAssym.NoValue) { 
		Assymetry = Param.Assym*(4.0/129.0);
		Correction = -Assymetry*(Assymetry*(-0.125)+1.0); }

	if(Param.Emphasis != mpEmphasis.NoValue) {
		Resonance = 1.0 - (Param.Emphasis * (960.0 / 1000.0) / 127.0); }

	if(Param.CutOff != mpCutOff.NoValue) {
		CutOff = Param.CutOff * 1.0 / 128.0; } }


#pragma optimize ("a", on)

void CMachine::Filter(float *psamples, int numsamples) {
	double L = Lowpass;
	double B = Bandpass;
	double H = Highpass;
	double f = CutOff;
	double const q = Resonance;

	int c = numsamples;
	do {
		L += f * B;
		H = *psamples - L - q * B;
		B += f * H;
		*psamples++ = (float)L;
	} while(--c);

	CutOff = f;
	Highpass = H;
	Bandpass = B;
	Lowpass = L; }


bool CMachine::Work(float *Dest, int Samples, int const Mode) {

	if (Mode == WM_WRITE || Mode == WM_NOIO)
		return false;
	if (Mode == WM_READ)
		return true;

	int ns = Samples;
	double const amp = DriveAmp;
	double const a = Assymetry;
	double const ac = Correction;
	double const vol = Volume;
	double t;
	float *d = Dest;

	do {
		t = ((double)*d * amp + a);
		if(t > 4.0) t = 2.0;
		else if(t < -4.0) t = -2.0;
		t += ac;
		*d++ = (float)(t * vol); } while(--ns);

	Filter(Dest,Samples);

	return true; }
