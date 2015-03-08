/*
 *		Expression plug-in for Buzz
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

#define c_strName			"Expression"
#define c_strShortName		"Expression"

#define MaxAmp			32768
#define SatAmp			32768

#pragma warning (disable:4305)

static float Tanh[] = {
	-1.,-0.999608,-0.999199,-0.998772,-0.998327,-0.997862,-0.997378,-0.996873,
	-0.996347,-0.995797,-0.995225,-0.994627,-0.994005,-0.993355,-0.992678,-0.991972,
	-0.991235,-0.990468,-0.989667,-0.988833,-0.987963,-0.987056,-0.98611,-0.985125,
	-0.984097,-0.983026,-0.98191,-0.980747,-0.979534,-0.978271,-0.976954,
	-0.975582,-0.974152,-0.972663,-0.971111,-0.969494,-0.96781,-0.966056,-0.964229,
	-0.962326,-0.960345,-0.958281,-0.956133,-0.953896,-0.951568,-0.949144,
	-0.946622,-0.943996,-0.941265,-0.938422,-0.935465,-0.93239,-0.92919,-0.925864,
	-0.922404,-0.918808,-0.91507,-0.911185,-0.907148,-0.902953,-0.898596,-0.894071,
	-0.889371,-0.884493,-0.879429,-0.874174,-0.868722,-0.863067,-0.857202,
	-0.851122,-0.844819,-0.838288,-0.831522,-0.824515,-0.817261,-0.809752,-0.801982,
	-0.793946,-0.785636,-0.777047,-0.768171,-0.759004,-0.749538,-0.739769,
	-0.729691,-0.719298,-0.708585,-0.697548,-0.686181,-0.674481,-0.662444,-0.650066,
	-0.637344,-0.624276,-0.61086,-0.597094,-0.582977,-0.568509,-0.55369,-0.538522,
	-0.523006,-0.507143,-0.490939,-0.474395,-0.457518,-0.440312,-0.422784,
	-0.404941,-0.386791,-0.368343,-0.349606,-0.330591,-0.311309,-0.291773,-0.271995,
	-0.25199,-0.231771,-0.211353,-0.190753,-0.169986,-0.14907,-0.128023,-0.106862,
	-0.0856048,-0.0642711,-0.0428797,-0.0214495,0,0.0214495,0.0428797,0.0642711,
	0.0856048,0.106862,0.128023,0.14907,0.169986,0.190753,0.211353,0.231771,
	0.25199,0.271995,0.291773,0.311309,0.330591,0.349606,0.368343,0.386791,
	0.404941,0.422784,0.440312,0.457518,0.474395,0.490939,0.507143,0.523006,
	0.538522,0.55369,0.568509,0.582977,0.597094,0.61086,0.624276,0.637344,
	0.650066,0.662444,0.674481,0.686181,0.697548,0.708585,0.719298,0.729691,
	0.739769,0.749538,0.759004,0.768171,0.777047,0.785636,0.793946,0.801982,
	0.809752,0.817261,0.824515,0.831522,0.838288,0.844819,0.851122,0.857202,
	0.863067,0.868722,0.874174,0.879429,0.884493,0.889371,0.894071,0.898596,
	0.902953,0.907148,0.911185,0.91507,0.918808,0.922404,0.925864,0.92919,
	0.93239,0.935465,0.938422,0.941265,0.943996,0.946622,0.949144,0.951568,
	0.953896,0.956133,0.958281,0.960345,0.962326,0.964229,0.966056,0.96781,
	0.969494,0.971111,0.972663,0.974152,0.975582,0.976954,0.978271,0.979534,
	0.980747,0.98191,0.983026,0.984097,0.985125,0.98611,0.987056,0.987963,
	0.988833,0.989667,0.990468,0.991235,0.991972,0.992678,0.993355,0.994005,
	0.994627,0.995225,0.995797,0.996347,0.996873,0.997378,0.997862,0.998327,
	0.998772,0.999199,0.999608,1.,1. };

#pragma warning (default:4305)


CMachineParameter const mpDrive = {
	pt_byte,"Drive","Drive (0=-32dB, 40=0, 80=32dB, Default = 0dB)",
	0x00,0x80,0xFF,MPF_STATE,0x40-6 };

CMachineParameter const	mpFEnv = {
	pt_byte,"Filt Env","Filter Envelope",
	0,0x80,0xFF,MPF_STATE,0x40 };

CMachineParameter const	mpFRes = {
	pt_byte,"Resonance","Resonance",
	0,0x80,0xFF,MPF_STATE,0x74 };

CMachineParameter const	mpInertia = {
	pt_byte,"Inertia","Inertia",
	0,106,0xFF,MPF_STATE,0x14 };

CMachineParameter const *mpArray[] = {
	&mpDrive,&mpFEnv,&mpFRes,&mpInertia };

enum mpValues { mpvDrive = 0,mpvFEnv,mpvFRes,mpvInertia };

CMachineAttribute const maHighFreq	= {
	"High Frequency Lfo",0,1,0 };

CMachineAttribute const *maArray[]	= { &maHighFreq };

CMachineInfo const MachineInfo = { 
	MT_EFFECT,MI_VERSION,0,
	0,0,4,0,mpArray,0,maArray,
	"Geonik's "c_strName, c_strShortName,"George Nicolaidis aka Geonik","About..." };

#pragma pack(1)		

class Parameters {
public:
	byte	Drive;
	byte	FEnv;
	byte	FRes;
	byte	Inertia; };

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
	bool		 Work(float *pin,int numsamples,int const mode);
	char const	*DescribeValue(int const param, int const value);
	double		 DetectLevel(float *inb, int ns);
	void		 Filter(float *pb, int const ns_, double const t);
	void		 Command(int const);
	void		 About();

	//CSharedResource		 cSharedResource;

private:
	double	gDrive;
	double	gInertiaSamples;
	double	gInertiaSamples_inv;
	double	gLastT;
	double	rmsC1;
	double	rmsC2;
	double	rmsQ;
	double	fEnv;
	double	fRes;
	double	fLowpass;
	double	fBandpass;
	double	fHighpass;

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
	double b			= 2.0 - cos(10 * 2 * PI / (double)pMasterInfo->SamplesPerSec);
	rmsC2				= b - sqrt(b * b - 1.0);
	rmsC1				= 1.0 - rmsC2;
	rmsQ				= 0;
	gDrive				= 127.0/(3.0*32768);
	gInertiaSamples		= 256;
	gInertiaSamples_inv	= 1.0 / 256;
	gLastT				= 0;
	fEnv				= 0.0000061;
	fRes				= 0.1;
	fLowpass			= 0;
	fHighpass			= 0;
	fBandpass			= 0; }


char const *CMachine::DescribeValue(int const ParamNum, int const Value) {
	static char TxtBuffer[16];

	switch(ParamNum) {
	case mpvDrive:
		sprintf(TxtBuffer,"%.1f dB", (float)(Value-64) * (1.0 / 2.0));
		break;
	case mpvFEnv:
		sprintf(TxtBuffer,"%.0f", (float)Value * (1000.0 / 128.0));
		break;
	case mpvFRes:
		sprintf(TxtBuffer,"%.1f%%", (float)Value * (100.0 / 128.0));
		break;
	case mpvInertia: {
		double v = pow(10.0,(double)(Value+14)*(1.0/20));
		if(v < 1000)
			sprintf(TxtBuffer,"%.1f ms", (float)v);
		else
			sprintf(TxtBuffer,"%.1f sec", (float)(v*(1.0/1000))); }
		break;
	default:
		return NULL; }
	return TxtBuffer; }


void CMachine::Tick() {

	if(Param.Drive != mpDrive.NoValue)
		gDrive = pow(2.0,(double)(Param.Drive-64)/12.0);

	if(Param.FEnv != mpFEnv.NoValue)
		fEnv = (double)Param.FEnv * (1.0 / (128*MaxAmp));

	if(Param.FRes != mpFRes.NoValue)
		fRes = 1.0 - (double)Param.FRes * (1.0 / 129);

	if(Param.Inertia != mpInertia.NoValue) {
		gInertiaSamples		= (double)pMasterInfo->SamplesPerSec * pow(10.0,(double)(Param.Inertia+14)*(1.0/20)) * (1.0 / 5000.0);
		if(gInertiaSamples < 256) gInertiaSamples = 256;
		gInertiaSamples_inv = 1.0 / gInertiaSamples; } }


#pragma optimize ("a", on)

inline int fint(double n) {
#ifdef WIN32
	double const d2i = (1.5 * (1 << 26) * (1 << 26));
	double n_ = n + d2i;
	return *(int *)&n_; 
#else
  // poor man's solution :)
  return (int)rint(n);
#endif
}


void Saturate(float *pb, int numSamples) {
	int		 i;
	double	 fr,s1,s2;

	//int oldctrlword = _control87(0, 0);
	//_control87(_RC_DOWN, _MCW_RC);			

	do {
		double s = *pb;
		s *= 127.0/(3.0*SatAmp);
		i = fint(s);
		if(i>127) 
			*pb++ = SatAmp;
		else if(i<-127)
			*pb++ = -SatAmp;
		else {
			fr = s - i; i+=127;
			s1 = Tanh[i++];
			s2 = Tanh[i];
			*pb++ = (float)((s1+(s2-s1)*fr)*SatAmp);
		}
	} while(--numSamples);

	//_control87(oldctrlword, _MCW_RC);
}


inline double CMachine::DetectLevel(float *inb, int ns) {
	double const a  = gDrive;
	double const c1 = rmsC1;
	double const c2 = rmsC2;
	double		 q  = rmsQ;

	do {
		double v = *inb++;
		q = c1*v*v*a + c2 * q;
	} while(--ns);

	rmsQ = q;
	return sqrt(q);
}


inline void CMachine::Filter(float *pb, int const ns_, double const step) {
	double const a  = gDrive;
	double		 L  = fLowpass;
	double		 B  = fBandpass;
	double		 H  = fHighpass;
	double const q  = fRes;
	double		 t  = gLastT;

	int ns = ns_;
	do {
		L += t * B;
		H = *pb - L - q * B;
		B += t * H;
		*pb++ = (float)(L*a);
		t += step;
	} while(--ns);

	fHighpass	= H;
	fBandpass	= B;
	fLowpass	= L;
	gLastT		= t;
}
	
bool CMachine::Work(float *pin,int numSamples,int const Mode) {

	if(Mode == WM_READWRITE) {
		double t = fEnv * DetectLevel(pin,numSamples); if(t>1.0) t=1.0;
		Filter(pin,numSamples,(t-gLastT)*gInertiaSamples_inv);
		Saturate(pin,numSamples);
		return true;
	}
	else if(Mode == WM_READ) {
		double t = fEnv * DetectLevel(pin,numSamples); if(t>1.0) t=1.0;
		gLastT += numSamples * (t-gLastT)*gInertiaSamples_inv;
	}
	else {
		gLastT += numSamples * (0.00001-gLastT)*gInertiaSamples_inv;
	}
	return false;
}

