// lines marked as //o means - optimised for speed
// lines marked as //u means - original equations

// 1. no longer tied to 44.1k but...
// 2. cuttof and some other sliders may be still fixed to 44.1k
// 3. no reinitialisation when samplerate is changed during playing (checked once after loading)
// 4. as far as i remember envelopes are not samplerate friendly, getting shorter when increasing the samplerate
//
// Have fun :)
// Oomek


#include <windef.h>
#include <math.h>
#include <float.h>
#include <stdio.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

#include "fft.h"


#define PITCHRESOLUTION 32 //70 
#define LEVELSPEROCTAVE 4

#pragma optimize ("awy", on)

// global params
CMachineParameter const paraOscType = 
{ 
	pt_switch,										// type
	"Osc Type",
	"Oscillator type (0 = Saw, 1 = Square)",		// description
	-1,												// MinValue	
	-1,												// MaxValue
	SWITCH_NO,										// NoValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraCutoff=
{
	pt_byte,
	"Cutoff",
	"Filter cutoff",
	0x0,
	0xF0,
	0xFF,
	MPF_STATE,
	0x78
};

CMachineParameter const paraResonance=
{
	pt_byte,
	"Res",
	"Filter resonance",
	0,
	0x80,
	0xFF,
	MPF_STATE,
	0x40
};

CMachineParameter const paraEnvmod=
{
	pt_byte,
	"Env.Mod",
	"Envelope modulation",
	0,
	0x80,
	0xFF,
	MPF_STATE,
	0x40
};

CMachineParameter const paraDecay=
{
	pt_byte,
	"Decay",
	"Envelope decay time",
	0,
	0x80,
	0xFF,
	MPF_STATE,
	0x40
};

CMachineParameter const paraAcclevel=
{
	pt_byte,
	"Accent Level",
	"Accent level",
	0,
	0x80,
	0xFF,
	MPF_STATE,
	0x40
};

CMachineParameter const paraFinetune=
{
	pt_byte,
	"Finetune",
	"Finetune",
	0,
	0xC8,
	0xFF,
	MPF_STATE,
	0x64
};

CMachineParameter const paraVolume=
{
	pt_byte,
	"Volume",
	"Volume",
	0,
	0xC8,
	0xFF,
	MPF_STATE,
	0x64
};

// track params
CMachineParameter const paraNote =
{
  pt_note,                                               // type
  "Note",
  "Note",                                                // description
  NOTE_MIN,                                              // Min
  NOTE_MAX,                                              // Max
  0,                                                     // NoValue
  0,                                                                                              // Flags
  0                                                                                               // default
};

CMachineParameter const paraSlide = 
{ 
	pt_switch,										// type
	"Slide",
	"Slide pitch to next note",						// description
	-1,												// MinValue	
	-1,												// MaxValue
	SWITCH_NO,										// NoValue
	0,										// Flags
	0,
};

CMachineParameter const paraAccent = 
{ 
	pt_switch,										// type
	"Accent",
	"Adds accent to volume and cutoff",				// description
	-1,												// MinValue	
	-1,												// MaxValue
	SWITCH_NO,										// NoValue
	0,										// Flags
	0
};

CMachineParameter const *pParameters[]=
{
  // global
	&paraOscType,
	&paraCutoff,
	&paraResonance,
	&paraEnvmod,
	&paraDecay,
	&paraAcclevel,
	&paraFinetune,
	&paraVolume,
  // track
	&paraNote,
	&paraSlide,
	&paraAccent
};

#pragma pack(1)

class gvals
{
public:
	byte osctype;
	byte cutoff;
	byte resonance;
	byte envmod;
	byte decay;
	byte acclevel;
	byte finetune;
	byte volume;
};



class tvals
{
public:
	byte note;
	byte slide;
	byte accent;
};



#pragma pack()

CMachineInfo const MacInfo=
{
	MT_GENERATOR,							// type
	MI_VERSION,
	0,				//MIF_DOES_INPUT_MIXING
	1,										// min tracks
	1,								// max tracks
	8,										// numGlobalParameters
	3,										// numTrackParameters
	pParameters,
	0,
	NULL,
	"Oomek Aggressor 3o3",
	"3o3",
	"Radoslaw Dutkiewicz",
//	"&About\n&Panel"
	"&About"
};


class miex : public CMDKMachineInterfaceEx
{

};


class mi : public CMDKMachineInterface
{

public:
	mi();
	virtual ~mi();
	virtual void Tick();
	virtual void MDKInit(CMachineDataInput *const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
	virtual void Command(int const i);
	virtual void MDKSave(CMachineDataOutput *const po);
	virtual char const *DescribeValue(int const param, int const value);
	virtual CMDKMachineInterfaceEx *GetEx() {return &ex;}
	virtual void OutputModeChanged(bool stereo){};
	virtual inline int ilog2(float x);
	virtual inline float fscale(float x);
	virtual inline int f2i(double d);

public:
	miex ex;

public:
	int tunelv;
	float amlv;
	float out;
	float oscsaw[2048*10*LEVELSPEROCTAVE];
	float oscsqa[2048*10*LEVELSPEROCTAVE];
	float oscpitch[12*100*10];
	bool osctype;
	bool slidestate;
	bool slidestateold;
	float vca[100+1000+500+1];
	float acc[601];
	float accl[12201];
	int Accphase1, Accphase2, Accphase3;
	int Accphasel1, Accphasel2, Accphasel3;
	float Acclevel;
	int Accphaseid;


	float oscphase;
	float oscphaseinc;
	float oldnote;
	float newnote;
	float slidenote;
	int oscphaseint0, oscphaseint1;
	int pitchcounter;
	
	
	int level;
	int osclevel;
	int vcaphase;

// 3p HP filter variables

	float hXa, hXb, hXc;
	float hYa, hYb, hYc;
	float hXaz, hXbz, hXcz;
	float hYaz, hYbz, hYcz;
	float hFh, hFhh;

// 3p LP filter variables

	float Xa, Xb, Xc;
	float Ya, Yb, Yc;
	float Xaz, Xbz, Xcz;
	float Yaz, Ybz, Ycz;
	float Flpfold;
	float Flpfnew, Qlpfnew;
	float Flpf, Qlpf;
	float Flpfh, Qlpfh;
	float Flpfsl, Qlpfsl;
	float Cutfreq, Oscfreq;
	float Qdown;
	float cf;
	float fftable[2048+2048+2];
	bool DoNothing;

	float Envmod, Envmodphase, Envmodinc, Envmodsl, Envmodnew, EnvmodphaseY, EnvmodphaseZ;
	float Decay;
	bool Accstate;

	float temp;

	float *p_Accphasel1;
	float *p_Accphasel2;
	float *p_Accphasel3;

	gvals gval;
	tvals tval;

	
};

mi::mi(){GlobalVals=&gval;TrackVals = &tval;}
mi::~mi(){}



#ifdef WIN32
//////////////dialog////////////////

HINSTANCE dllInstance;
mi *g_mi;

BOOL WINAPI DllMain (HANDLE hModule, DWORD fwdreason, LPVOID lpReserved)
{
	switch (fwdreason)
	{
		case DLL_PROCESS_ATTACH:
			dllInstance = (HINSTANCE) hModule;
			break;

		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
		case DLL_PROCESS_DETACH: break;
	}
	return TRUE;
}

BOOL APIENTRY AboutDialog (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG: return 1;
		case WM_SHOWWINDOW: return 1;
		case WM_CLOSE: EndDialog (hDlg, TRUE); return 0;
		case WM_COMMAND:
		switch (LOWORD (wParam))
		{
			case IDOK:
				EndDialog(hDlg, TRUE);
				return 1;
			default:
				return 0;
		}
		break;
	}
	return 0;
}
#endif

//////////////////////////////////////////

void mi::MDKInit(CMachineDataInput *const pi)
{
	int i;
	int j;
//	SetOutputMode(true);
	tunelv = 0;
	amlv = 0.0f;
	oscphase = 0.0;
	oscphaseinc = 0.0;
	osctype = false;
	slidestate = false;
	slidestateold = false;
	pitchcounter = PITCHRESOLUTION-1;
	Envmodinc = 0.0;
	Accphase1 = 600;
	Accphase2 = 600;
	Accphase3 = 600;
	Accphasel1 = 12200;
	Accphasel2 = 12200;
	Accphasel3 = 12200;
	p_Accphasel1 = accl + 12200 - 1;
	p_Accphasel2 = accl + 12200 - 1;
	p_Accphasel3 = accl + 12200 - 1;
	vcaphase = 1600;

	Flpfold = (float)pow((gval.cutoff / 240.0f) , 2.0f) * 0.8775f + 0.1225f;
	Qlpf = (float)pow((gval.resonance / 128.0f),0.5f);
	Envmod = (float)gval.envmod / 128.0f;

/// vca table filling///

	for (i = 0; i < 200; i++ ) vca[i] = 1.0f - (float)pow(((200 - i) / 200.0f),2.0f);
	for (i = 200; i < 1200; i++ ) vca[i] = (float)pow(((1200 - i) / 1000.0f),2.0f) * 0.25f + 0.75f;
	for (i = 1200; i < 1600; i++) vca[i] = (float)pow(((1600 - i) / 400.0f),1.25f) * 0.75f;
	vca[1600] = 0.0f;

/// vca acccent table filling///

	for (i = 0; i < 200; i++ ) accl[i] = (1.0f - (float)pow(((200 - i) / 200.0f),2.0f)) * 1.5f; //def 2, 1.5
	for (i = 200; i < 12200; i++ ) accl[i] = (float)pow(((12200 - i) / 12000.0f),1.0f) * 1.5f; //def 4, 1.5
	accl[12200] = 0.0f;

/// accent table filling///

	for (i = 0; i < 60; i++ ) acc[i] = 1.0f - (float)pow(((60 - i) / 60.0f),2.0f); // def 50, 2.0f
	for (i = 60; i < 600; i++) acc[i] = (float)pow(((600 - i) / 540.0f),4.0f); // def 50-600, 4.0f
	acc[600] = 0.0f;


/// oscillator pitch table ///
	i=0;
	do
	{
		oscpitch[i] = 2048.0f*(440.0f / pMasterInfo->SamplesPerSec) * (float)(pow(2.0,((((i+1)/100.0)-69)/12.0)));	
		i++;
	} while (i!=120*100);


	
	
/// Saw oscillator table ///

	for (j=0; j<40; j++)
	{
		for (i=0; i< 4096; i++)
			fftable[i]=0;
		for (i=1; i< 900/(float)pow(2.0,((j/4.0))); i+=1)
		{
			fftable[i*2]=0;
			fftable[i*2+1]=1.0f/float(i);

		}
		IFFT(fftable, 2048, 1);	
		for (i=0; i< 2048; i++)
		{
			oscsaw[i+2048*j] = fftable[i*2];
		}

	}


/// Square oscillator table ///

	for (j=0; j<40; j++)
	{
		for (i=0; i< 4096; i++)
			fftable[i]=0;
		for (i=1; i< 900/(float)pow(2.0,((j/4.0))); i+=2)
		{
			fftable[i*2]=0;
			fftable[i*2+1]=1.0f/float(i);	//float(pow(i,1.5));    // float(i)
		}
		IFFT(fftable, 2048, 1);
		for (i=0; i< 2048; i++)
		{
			oscsqa[i+2048*j] = 0.5f * oscsaw[i+2048*j] + fftable[i*2];
		}

	}

//	g_mi=this;
//	CreateDialog (dllInstance, MAKEINTRESOURCE(IDD_MACABOUT), GetForegroundWindow(), (DLGPROC) &AboutDialog);

}




inline int mi::f2i(double d)
{
	const double magic = 6755399441055744.0;
  union { unsigned long ui; double d; } double_as_bits;
  double_as_bits.d = (d-0.5) + magic;
  return int(double_as_bits.ui);
}

inline int mi::ilog2(float x)
{
  union { unsigned int ui; float d; } float_as_bits;
  float_as_bits.d = x;
	unsigned int exp = (float_as_bits.ui >> 23) & 0xFF;
	return int(exp) - 127;
}

inline float mi::fscale(float x)
{
	x = x / (pMasterInfo->SamplesPerSec / 44100.0f);
	float wynik = (((((-2.7528f * x) + 3.0429f) * x) + 1.718f) * x) - 0.9984f; 
//	float wynik = ((((((-2.7528f* 0.5f) * x) + (3.0429f * 0.5f)) * x) + (1.718f * 0.5f)) * x) - (0.9984f * 0.5f); 
	return wynik;
}



void mi::MDKSave(CMachineDataOutput *const po){}





//////////// TICK ////////////

void mi::Tick()
{
	if (gval.osctype != SWITCH_NO)
	{
		if( gval.osctype == SWITCH_ON)
			osctype = true;
		else
			osctype = false;
  }

	if (gval.cutoff != 0xFF)
	{
//		Flpfnew = (float)pow((gval.cutoff / 240.0f) , 2.0f) * 0.8775f + 0.1225f;
		Flpfnew = (float)gval.cutoff * 0.004166666666f; //o
		Flpfnew	= Flpfnew * Flpfnew * 0.8775f + 0.1225f;	//o
//u		Flpfsl = (Flpfnew - Flpfold) / pMasterInfo->SamplesPerTick / 0.5f * PITCHRESOLUTION;
		Flpfsl = (Flpfnew - Flpfold) / pMasterInfo->SamplesPerTick * 1.0f * PITCHRESOLUTION; //o
	}
	
	if (gval.resonance != 0xFF)
	{
//u		Qlpfnew = (float)pow((gval.resonance / 128.0f),0.5f);
		Qlpfnew = (float)pow((gval.resonance * 0.0078125f) , 0.5f);	//o
//u		Qlpfsl = (Qlpfnew - Qlpf) / pMasterInfo->SamplesPerTick / 0.5f * PITCHRESOLUTION;
		Qlpfsl = (Qlpfnew - Qlpf) / pMasterInfo->SamplesPerTick * 1.0f * PITCHRESOLUTION;	//o
	}

	if (gval.envmod != 0xFF) 
	{
//u		Envmodnew = (float)gval.envmod / 128.0f;
		Envmodnew = (float)gval.envmod * 0.0078125f;	//o
//u		Envmodsl = (Envmodnew - Envmod) / pMasterInfo->SamplesPerTick / 0.5f * PITCHRESOLUTION;
		Envmodsl = (Envmodnew - Envmod) / pMasterInfo->SamplesPerTick * 1.0f * PITCHRESOLUTION;	//o
	}


//u	if (gval.decay != 0xFF) Decay = (float)pow((gval.decay / 128.0f) , 0.1f) * 0.992f;
	if (gval.decay != 0xFF) Decay = (float)pow((gval.decay * 0.0078125f) , 0.1f) * 0.992f;	//o

//u	if (gval.acclevel != 0xFF) Acclevel = ((float)gval.acclevel / 128.0f);
	if (gval.acclevel != 0xFF) Acclevel = ((float)gval.acclevel / 64.0f);
//u	if (gval.acclevel != 0xFF) Acclevel = (float)gval.acclevel * 0.0078125f ;	//o

	if (gval.finetune != 0xFF) tunelv = gval.finetune - 100;

//u	if (gval.volume != 0xFF) amlv = ((float)gval.volume / 100.0f) * 8192.0f;
	if (gval.volume != 0xFF) amlv = (float)gval.volume * 81.92f;

	slidestateold = slidestate;			
	if (tval.slide != SWITCH_ON)
		slidestate = false;
	else
		slidestate = true;	
	
	if ((tval.note != NOTE_NO ) && (tval.note != NOTE_OFF))
	{
		if( (tval.note >= NOTE_MIN + 1 ) && (tval.note <= NOTE_MAX - 1))
		{
			newnote = (float)(((tval.note>>4)*12+(tval.note&0x0f)-1)*100.0) + tunelv;
			if (slidestateold == true)
			{
//u				slidenote = (newnote - oldnote) / pMasterInfo->SamplesPerTick / 0.375f * PITCHRESOLUTION;
				slidenote = (newnote - oldnote) / pMasterInfo->SamplesPerTick * 3.0f * PITCHRESOLUTION;	//o
			}
			else
			{
				slidenote = newnote - oldnote;
				vcaphase = 0;
				DoNothing = false;
				Envmodinc = 0.0f;
				Accstate = false;
				if (tval.accent == SWITCH_ON)
				{
					Accstate = true;
					if (Accphaseid > 2) Accphaseid = 0;
					Accphaseid ++;
					switch (Accphaseid)
					{
					case 1: Accphase1 = Accphasel1 = 0; p_Accphasel1 = accl; break;
					case 2: Accphase2 = Accphasel2 = 0; p_Accphasel2 = accl; break;
					case 3: Accphase3 = Accphasel3 = 0; p_Accphasel3 = accl; break;
					}
				}
			}
		}
	    else;
//			tval.note = NOTE_NO;
  }
}


//////////// WORK ////////////



bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{

	if (DoNothing == false)
	{

		DoNothing = true;
		do
		{
			
			if (pitchcounter == PITCHRESOLUTION-1)
			{


	/// Note slide computation ///

				oldnote += slidenote;
				if (slidenote > 0 && oldnote > newnote)
				{
					oldnote = newnote; slidenote = 0;
				}
				else if (slidenote < 0 && oldnote < newnote)
				{
					oldnote = newnote; slidenote = 0;
				}
		
				oscphaseinc = oscpitch[f2i(oldnote)];
		

	/// Table Level computation ///

				osclevel = 0;
				osclevel = ilog2(oscphaseinc*oscphaseinc*oscphaseinc*oscphaseinc) ;
				if (osclevel < 0) osclevel = 0;

				
	/// Cutoff slide computation ///

				Flpfold += Flpfsl;
				if (Flpfsl > 0 && Flpfold > Flpfnew)
				{
					Flpfold = Flpfnew; Flpfsl = 0;
				}
				else if (Flpfsl < 0 && Flpfold < Flpfnew)
				{
					Flpfold = Flpfnew; Flpfsl = 0;
				}

	/// Q slide computation ///

				Qlpf += Qlpfsl;
				if (Qlpfsl > 0 && Qlpf > Qlpfnew)
				{
					Qlpf = Qlpfnew; Qlpfsl = 0;
				}
				else if (Qlpfsl < 0 && Qlpf < Qlpfnew)
				{
					Qlpf = Qlpfnew; Qlpfsl = 0;
				}

	/// Envmod slide computation ///

				Envmod += Envmodsl;
				if (Envmodsl > 0 && Envmod > Envmodnew)
				{
					Envmod = Envmodnew; Envmodsl = 0;
				}
				else if (Envmodsl < 0 && Envmod < Envmodnew)
				{
					Envmod = Envmodnew; Envmodsl = 0;
				}


	/// Cutoff scale computation ///

				if (Accstate == true) 
					Envmodinc += (0.125f);
				else
					Envmodinc += (0.125f * (1 - Decay));

				Envmodphase = (1.0f / (1 + Envmodinc));
				Envmodphase =  (Envmodphase * 0.965f + 0.035f) * Envmod + (Envmodphase * 0.05f + 0.1f) * (1.0f - Envmod);

				EnvmodphaseY = ((Envmodphase - EnvmodphaseZ) * 0.2f) + EnvmodphaseZ;	// Envmod
				EnvmodphaseZ = EnvmodphaseY;											// lowpass


				Cutfreq = EnvmodphaseY * (((acc[Accphase1] + acc[Accphase2] + acc[Accphase3]) * Acclevel) + 1.0f);


				Cutfreq = Cutfreq * Flpfold;
				if (Cutfreq > 0.87f) Cutfreq = 0.87f;
				Cutfreq = Cutfreq * (pMasterInfo->SamplesPerSec * 0.5f); //22050.0f;

				if (Accphase1 < 600) Accphase1 ++;
				if (Accphase2 < 600) Accphase2 ++;
				if (Accphase3 < 600) Accphase3 ++;

        //Oscfreq = oscphaseinc * 21.533203125f;	//o
				Oscfreq = oscphaseinc * pMasterInfo->SamplesPerSec / 2048.0f;	//u

				if (Cutfreq < Oscfreq) Cutfreq = Oscfreq;
				
				Flpf = Cutfreq / (pMasterInfo->SamplesPerSec * 0.5f); //22050.0f;

				Flpf = fscale(Flpf);
				if (Flpf > 1) Flpf = 1.0f;

				Qdown =  1.0f - (float)pow(0.75f, Cutfreq / Oscfreq); // 0,8f def 
        //Qdown = 1.0f;


				
	/// Q scale computation ///

        //cf = (Flpf * 1.012f) + 1;
				cf = (Flpf * 1.00f) + 1;
        //cf *= 0.5f;
	      //Qlpfh = Qlpf * (float)( -0.9308 * pow(cf , 6) + 5.398 * pow(cf , 5) - 11.753 * pow(cf , 4) + 10.655 * pow (cf , 3) - 0.416 * pow (cf , 2) - 7.0114 * cf + 5.9039);
				Qlpfh = 5.9039f - 7.0114f * cf;
				cf *= (Flpf + 1);
				Qlpfh = Qlpfh - 0.416f * cf;
				cf *= (Flpf + 1);
				Qlpfh = Qlpfh + 10.655f * cf;
				cf *= (Flpf + 1);
				Qlpfh = Qlpfh	- 11.753f * cf;
				cf *= (Flpf + 1);
				Qlpfh = Qlpfh + 5.398f * cf;
				cf *= (Flpf + 1);
				Qlpfh = Qlpfh - 0.9308f * cf;
				Qlpfh = Qlpfh * Qlpf;
				Qlpfh *= Qdown;

				///////////////////////////

				pitchcounter = 0;
			}
			else { pitchcounter ++; }


	/// Waveform generation ///

			oscphase += oscphaseinc;
			if (oscphase >= 2048.0f) oscphase -= 2048.0f;
			oscphaseint0 = f2i((float)oscphase);
			oscphaseint1 = oscphaseint0 + 1;
			if (oscphaseint1 >= 2048) oscphaseint1 = 0;
			

			
			if (osctype == false)
			{
			out = oscsaw[oscphaseint0+2048*osclevel] * (1-(oscphase-oscphaseint0))	//linear interpol.
				+ oscsaw[oscphaseint1+2048*osclevel] * (oscphase-oscphaseint0);	//linear interpol.


			}
			else
			{
			out = oscsqa[oscphaseint0+2048*osclevel] * (1-(oscphase-oscphaseint0))	//linear interpol.
				+ oscsqa[oscphaseint1+2048*osclevel] * (oscphase-oscphaseint0);	//linear interpol.

			}

			if (vcaphase < 1200) vcaphase++;
			if ((pMasterInfo->PosInTick > pMasterInfo->SamplesPerTick / 2) && (vcaphase < 1600) && (slidestate == false)) vcaphase++;


	/// Adding VCA ///
			
	/// 3p hipass filter ///

//u			out = out * (vca[vcaphase] + accl[Accphasel1] + accl[Accphasel2] + accl[Accphasel3]);
			temp = vca[vcaphase];
			temp += *p_Accphasel1;
			temp += *p_Accphasel2;
			temp += *p_Accphasel3;
			out = out * temp;


			if (Accphasel1 < 12200)
			{
				Accphasel1 ++;
				p_Accphasel1++;
			}
			
			if (Accphasel2 < 12200)
			{
				Accphasel2 ++;
				p_Accphasel2++;
			}
			
			if (Accphasel3 < 12200)
			{
				Accphasel3 ++;
				p_Accphasel3++;
			}






	// 3p Lowpass Resonant VCF ///

			out = out * (Qlpfh + 1);
			out = out * amlv;
			out = out - Yc * Qlpfh;

			Flpfh = (Flpf + 1) * 0.5f;
			Xaz = Xa;
			Xa = out;
			Yaz = Ya;

			Ya = ((Xa + Xaz) * Flpfh) - (Flpf * Yaz);
			out = Ya;

			
			Xbz = Xb;
			Xb = out;
			Ybz = Yb;

			Yb = ((Xb + Xbz) * Flpfh) - (Flpf * Ybz);
			out = Yb;

			Xcz = Xc;
			Xc = out;
			Ycz = Yc;

			Yc = ((Xc + Xcz) * Flpfh) - (Flpf * Ycz);

			out = Yc;
	///////////////////





	// Allpass shifter ///
			hFh = -0.998f; // -0.994f, 996

			hXa = out;
			hYa = hXa * hFh + hYaz;
			hYaz = hXa - hFh * hYa;
			out = hYa;

	// Clipper def -2, 2 ///
			if (out < -14.0f * 8192.0f) out = -14.0f * 8192.0f;
			if (out > 14.0f * 8192.0f) out = 14.0f * 8192.0f;
	///////////////////



			hXb = out;
			hYb = hXb * hFh + hYbz;
			hYbz = hXb - hFh * hYb;
			out = hYb;
	///////////////////



///////////////////


			*psamples=out;
			if (!(*psamples < 1 && *psamples > -1 && vcaphase == 1600)) DoNothing = false;
			psamples ++;
		}
		while (--numsamples);
		return true;
	}
	else
	{
		do
		{
/// Cutoff slide computation ///

				Flpfold += Flpfsl;
				if (Flpfsl > 0 && Flpfold > Flpfnew)
				{
					Flpfold = Flpfnew; Flpfsl = 0;
				}
				else if (Flpfsl < 0 && Flpfold < Flpfnew)
				{
					Flpfold = Flpfnew; Flpfsl = 0;
				}

/// Q slide computation ///

				Qlpf += Qlpfsl;
				if (Qlpfsl > 0 && Qlpf > Qlpfnew)
				{
					Qlpf = Qlpfnew; Qlpfsl = 0;
				}
				else if (Qlpfsl < 0 && Qlpf < Qlpfnew)
				{
					Qlpf = Qlpfnew; Qlpfsl = 0;
				}

/// Envmod slide computation ///

				Envmod += Envmodsl;
				if (Envmodsl > 0 && Envmod > Envmodnew)
				{
					Envmod = Envmodnew; Envmodsl = 0;
				}
				else if (Envmodsl < 0 && Envmod < Envmodnew)
				{
					Envmod = Envmodnew; Envmodsl = 0;
				}
		} while (--numsamples);
		return false;    //def false
	}
}




bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	return false;
}




void mi::Command(int const i)
{
	switch(i)
	{
	case 0:
#ifdef WIN32
		MessageBox (NULL, "Oomek Aggressor 3o3\nVersion: 1.2\n(samplerate, accent fix)\n\nby\nRadoslaw Dutkiewicz\n","About Aggressor 3o3",MB_OK|MB_SYSTEMMODAL);
#endif
		break;
//	case 1:
//		g_mi=this;
//		CreateDialog (dllInstance, MAKEINTRESOURCE(IDD_MACABOUT), GetForegroundWindow(), (DLGPROC) &AboutDialog);
//		break;
	default:
		break;
	}
}




char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];
	switch(param)
	{
	case 0:
		switch(value)
			{
			case 0: return "Saw";
			case 1: return "Square";
			}

	case 1: // cut off
	case 2: // res
	case 3: // env-mod
	case 4: // decay
	case 5: // accent level
	  return NULL;
	case 6: // finetune
		sprintf(txt,"%i ct",value - 100);
		return txt;
		break;
	case 7: // volume
		sprintf(txt,"%i%%",value);
		return txt;
		break;

	
	case 8: // note
	case 9: // slide
	case 10: // accent
	default:
		return NULL;
	}
}



#pragma optimize("",on)

DLL_EXPORTS



