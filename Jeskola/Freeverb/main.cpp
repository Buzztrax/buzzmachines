#include <windef.h>
#include <list>
#include <MachineInterface.h>
#include <mdk/mdk.h>
#include <dsplib.h>
#include "revmodel.h"
#include "main.h"

#pragma comment (lib, "../buzzlib/buzz/mdk.lib")
#pragma comment (lib, "../buzzlib/buzz/dsplib.lib")

/*


s� jeg tror du m� sette opp en delay(preDelay)->lowPass(loCut)->hiPass(hiCut) f�r du forer boksen


original freeverb params:
	void	setroomsize(float value);
	void	setdamp(float value);
	void	setwet(float value);
	void	setdry(float value);
	void	setwidth(float value);
	void	setmode(float value);


jeskola params (and mappings):

byte revTime;	// setroomsize(float value)
byte hiDamp;	// setdamp(float value)
byte preDelay;	// 
byte loCut;		// 
byte hiCut;		// 
byte revOut;	// setwet(float value)
byte dryOut;	// setdry(float value)
*/

CMachineParameter const paraRevTime = { 
	pt_byte, 
	"RevTime", 
	"Reverberation time", 
	0x00, 
	0x7f, 
	BYTE_NO, 
	MPF_STATE,
	40
};

CMachineParameter const paraHiDamp = { 
	pt_byte, 
	"HiDamp", 
	"High Frequency Damping", 
	0x00, 
	0x7f, 
	BYTE_NO, 
	MPF_STATE, 
	50
};
	
CMachineParameter const paraPreDelay = { 
	pt_byte, 
	"PreDelay", 
	"Pre delay", 
	0x00, 
	0x7F, 
	BYTE_NO, 
	MPF_STATE, 
	0
};

CMachineParameter const paraLoCut = { 
	pt_byte, 
	"LowCut", 
	"Low Cut", 
	0x00, 
	0x7F, 
	BYTE_NO, 
	MPF_STATE, 
	0
};

CMachineParameter const paraHiCut = { 
	pt_byte, 
	"HiCut", 
	"Hi Cut", 
	0x00, 
	0x7F, 
	BYTE_NO, 
	MPF_STATE, 
	0x7F
};


CMachineParameter const paraRevOut = { 
	pt_byte, 
	"RevOut", 
	"Reverb output level", 
	0x00, 
	0x7F, 
	BYTE_NO, 
	MPF_STATE, 
	0x50
};


CMachineParameter const paraDryOut = { 
	pt_byte, 
	"DryOut", 
	"Dry Out", 
	0x00, 
	0x7F, 
	BYTE_NO, 
	MPF_STATE, 
	0x7F
};


CMachineParameter const *pParameters[] = { 
	// global
	&paraRevTime, 
	&paraHiDamp,
	&paraPreDelay,
	&paraLoCut,
	&paraHiCut,
	&paraRevOut,
	&paraDryOut
};


CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,					// flags
	0,										// min tracks
	0,										// max tracks
	7,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,		// pAttributes
	"Jeskola Freeverb",					// name
	"Freeverb",									// short name
	"andyw/batongen/Jezar",						// author
	NULL
};

DLL_EXPORTS

const int FADE_LEN=0x800;

mi::mi() {
	faderCounter=0;
	GlobalVals = &gval;
	AttrVals = NULL;
}

mi::~mi() {
}

void mi::MDKInit(CMachineDataInput * const pi) {
#ifndef WIN32
    DSP_Init(pMasterInfo->SamplesPerSec);
#endif
	//reverb.mute();
}

void mi::MDKSave(CMachineDataOutput * const po) {
}

#include <complex>

float linear_to_dB(float val) { 
	return(10.0 * log10(val)); 
}

static float dB_to_linear(float val) {
	if (val == 0.0) return(1.0);
	return(pow(10.0, val / 20.0));
}

void mi::Tick() {
	if (gval.loCut!=BYTE_NO) 
		reverb.setlocut((float)gval.loCut*4);

	if (gval.hiCut!=BYTE_NO) 
		reverb.sethicut(100+(float)gval.hiCut*128);

	if (gval.preDelay!=BYTE_NO) {
		reverb.setpredelay(gval.preDelay);
	}

	if (gval.revTime!=BYTE_NO)
		reverb.setroomsize((float)gval.revTime / 0x80);

	if (gval.dryOut!=BYTE_NO)
		reverb.setdry((float)gval.dryOut/0xFF);

	if (gval.hiDamp!=BYTE_NO)
		reverb.setdamp((float)gval.hiDamp / 0x80);

	if (gval.revOut!=BYTE_NO) {
		float dB=dB_to_linear(((float)(0x80-gval.revOut) / (float)0x80)*-60.0f);
		reverb.setwet(dB);
	}

}


bool mi::MDKWork(float *psamples, int numsamples, int const mode) {
	float fadeLen=floor(FADE_LEN*(reverb.getroomsize()))*MAX_BUFFER_LENGTH + reverb.delaySamples + pMasterInfo->SamplesPerSec; // 1 second more for salely
	//NOTE: fade length should based on numsamples everytime becoz nobody know
    // how many samples will take audio drive. It may be 256 or lower
    // (for example ASIO driver).

	if (mode == WM_NOIO) {
		return false;
	} else
	if (mode==WM_WRITE) {
		// roomsize tells how fast we're going to fade down
		if (faderCounter>=fadeLen) return false;
		faderCounter+=numsamples;
	} else	
		faderCounter=0;

	// så jeg tror du må sette opp en delay(preDelay)->lowPass(loCut)->hiPass(hiCut) før du forer boksen
	// freeverb skrur seg aldri av = alle eq’er o.l i chain med freeverb blir fucked. vi får altså en bug på freeverb->eq7 som må fikses
    // so I think you need to set up a delay (preDelay) -> lowPass (loCut) -> hiPass (hiCut) before you list box
    // free verb turns never of = all eq'er of the chain with free verbs are fucked. Thus we get a bug for free verb-> eq7 that must be fixed

	if (faderCounter>fadeLen)
		faderCounter=int(fadeLen+1);

    //float amp=logf(FADE_LEN-faderCounter) / logf(FADE_LEN);
	float amp=dB_to_linear(((float)(faderCounter/MAX_BUFFER_LENGTH) / (float)fadeLen)*-60.0f);
	bool processresult=reverb.processreplace(psamples, psamples, psamples, psamples, numsamples, 1, amp);
	if ((faderCounter>=fadeLen || !processresult) && mode == WM_WRITE) {
        /*
        reverb.mute();
		if (faderCounter<fadeLen)
			faderCounter=fadeLen+1;
        */
		processresult=false;
	}
	return processresult;

}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode) {
	//int i=pCB->GetWritePos();
	float fadeLen=floor(FADE_LEN*(reverb.getroomsize()))*MAX_BUFFER_LENGTH + reverb.delaySamples + pMasterInfo->SamplesPerSec;// read note @ MDKWork

	if (mode == WM_NOIO) {
		return false;
	} else
	if (mode==WM_WRITE) {
		if (faderCounter>=fadeLen) return false;
		faderCounter+=numsamples;
	} else	
		faderCounter=0;

	if (faderCounter>fadeLen)
		faderCounter=int(fadeLen+1);

    float amp=dB_to_linear(((float)(faderCounter/MAX_BUFFER_LENGTH) / (float)fadeLen)*-60.0f);
	bool processresult=reverb.processreplace(psamples, psamples+1, psamples, psamples+1, numsamples, 2, amp);

	if ((faderCounter>=fadeLen || !processresult) && mode == WM_WRITE) {
        /*		
        reverb.mute();
		if (faderCounter<fadeLen)
			faderCounter=fadeLen+1;*/
		processresult=false;
	}
	return processresult;
}

void mi::Command(int const i) {
}

char const *mi::DescribeValue(int const param, int const value) {
	return 0;
}
