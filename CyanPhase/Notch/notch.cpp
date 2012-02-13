#include "../mdk/mdk.h"
#include <float.h>
#include <math.h>
#include "resource.h"
#include <windows.h>

#pragma optimize ("awy", on) 

float const pi2 = 2 * PI;

CMachineParameter const paraFilter1On = { pt_switch, "Fixed1 Filter", "Fixed 1", -1, -1, SWITCH_NO, MPF_STATE, 1 };
CMachineParameter const paraFilter2On = { pt_switch, "Fixed2 Filter", "Fixed 2", -1, -1, SWITCH_NO, MPF_STATE, 0 };
CMachineParameter const paraFilter3On = { pt_switch, "LFO Filter", "LFO", -1, -1, SWITCH_NO, MPF_STATE, 0 };

CMachineParameter const paraFreq1 =
{
	pt_word,
	"  Fixed1:Freq",
	"Fixed Filter 1 Frequency (in Hz)",
	5,
	21500,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraNote1 = {
	pt_note,
	"  Fixed1:Note",
	"Fixed Filter 1 Frequency by Note",
	NOTE_MIN,
	NOTE_OFF,
	NOTE_NO,
	0,
	0
};

CMachineParameter const paraBW1 =
{
	pt_word,
	"  Fixed1:BW",
	"Fixed Filter 1 Bandwidth",
	2,
	2500,
	0xFFFF,
	MPF_STATE,
	100
};


CMachineParameter const paraFreq2 =
{
	pt_word,
	"  Fixed2:Freq",
	"Fixed Filter 2 Frequency (in Hz)",
	5,
	21500,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraNote2 = {
	pt_note,
	"  Fixed2:Note",
	"Fixed Filter 2 Frequency by Note",
	NOTE_MIN,
	NOTE_OFF,
	NOTE_NO,
	0,
	0
};

CMachineParameter const paraBW2 =
{
	pt_word,
	"  Fixed2:BW",
	"Fixed Filter 2 Bandwidth",
	2,
	2500,
	0xFFFF,
	MPF_STATE,
	100
};


CMachineParameter const paraLFOFreq1 =
{
	pt_word,
	"  LFO:Freq1",
	"LFO Frequency 1 (in Hz)",
	5,
	21500,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraLFOFreq2 =
{
	pt_word,
	"  LFO:Freq2",
	"LFO Frequency 2 (in Hz)",
	5,
	21500,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraLFOFreq3 =
{
	pt_word,
	"  LFO:FreqLFO",
	"LFO Frequency Speed (in secs)",
	1,
	0xFFFE,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraLFONote1 = {
	pt_note,
	"  LFO:Note1",
	"LFO Frequency 1 by Note",
	NOTE_MIN,
	NOTE_OFF,
	NOTE_NO,
	0,
	0
};

CMachineParameter const paraLFONote2 = {
	pt_note,
	"  LFO:Note2",
	"LFO Frequency 2 by Note",
	NOTE_MIN,
	NOTE_OFF,
	NOTE_NO,
	0,
	0
};

CMachineParameter const paraLFOBW1 =
{
	pt_word,
	"  LFO:BW1",
	"LFO Bandwidth 1",
	2,
	2500,
	0xFFFF,
	MPF_STATE,
	100
};

CMachineParameter const paraLFOBW2 =
{
	pt_word,
	"  LFO:BW2",
	"LFO Bandwidth 2",
	2,
	2500,
	0xFFFF,
	MPF_STATE,
	100
};

CMachineParameter const paraLFOBW3 =
{
	pt_word,
	"  LFO:BW-LFO",
	"LFO Bandwidth Speed (in secs)",
	1,
	0xFFFE,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraInertia1 =
{
	pt_word,
	"  Fixed1:Inertia",
	"Fixed Filter 1 Inertia (in units per sec)",
	1,
	0xFFFE,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const paraInertia2 =
{
	pt_word,
	"  Fixed2:Inertia",
	"Fixed Filter 2 Inertia (in units per sec)",
	1,
	0xFFFE,
	0xFFFF,
	MPF_STATE,
	2000
};

CMachineParameter const *pParameters[] = {
	&paraFilter1On,		// 0
	&paraFreq1,			// 1
	&paraNote1,			// 2
	&paraBW1,			// 3
	&paraInertia1,		// 4

	&paraFilter2On,		// 5
	&paraFreq2,			// 6
	&paraNote2,			// 7
	&paraBW2,			// 8
	&paraInertia2,		// 9

	&paraFilter3On,		// 10
	&paraLFOFreq1,		// 11
	&paraLFOFreq2,		// 12
	&paraLFOFreq3,		// 13
	&paraLFONote1,		// 14
	&paraLFONote2,		// 15
	&paraLFOBW1,		// 16
	&paraLFOBW2,		// 17
	&paraLFOBW3,		// 18
};


CMachineAttribute const attrInertiaType = {
	"Inertia Type (0=u/s,1=ticks)",
	0,
	5,
	0
};

CMachineAttribute const *pAttributes[] = {
	&attrInertiaType
};

#pragma pack(1)		

class gvals
{
public:
	byte filter1on;
	word freq1;
	byte note1;
	word bw1;
	word inertia1;

	byte filter2on;
	word freq2;
	byte note2;
	word bw2;
	word inertia2;

	byte filter3on;
	word lfofreq1;
	word lfofreq2;
	word freqspeed;
	byte lfonote1;

	byte lfonote2;
	word lfobw1;
	word lfobw2;
	word bwspeed;
};

class avals
{
public:
	int inertiatype;
};

#pragma pack()


CMachineInfo const MacInfo = 
{
	MT_EFFECT,
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,
	0,										// min tracks
	0,										// max tracks
	19,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	1,
	pAttributes,
	"CyanPhase Notch",								// name
	"Notch",								// short name
	"Edward L. Blake",						// author
	"&Reset\n&About..."
};


class miex : public CMDKMachineInterfaceEx { };

class mi : public CMDKMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Tick();

	virtual void MDKInit(CMachineDataInput * const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
	virtual void Command(int const i);
	virtual void FixedFilterMe();

	virtual void MDKSave(CMachineDataOutput * const po);

	virtual char const *DescribeValue(int const param, int const value);

public:
	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
	virtual void OutputModeChanged(bool stereo) { }


public:
	miex ex;

public:

	float coefsTab1[5];
	float coefsTab2[5];
	float coefsTab3[5];

	float lax1,lay1,lax2,lay2;
	float lbx1,lby1,lbx2,lby2;
	float lcx1,lcy1,lcx2,lcy2;
	float rax1,ray1,rax2,ray2;
	float rbx1,rby1,rbx2,rby2;
	float rcx1,rcy1,rcx2,rcy2;
	
	int filt1on, filt2on, filt3on;
	int icnt;
	
	float fixedfreq1, fixedfreq2;
	float tfixedfreq1, tfixedfreq2;
	float fixedbw1, fixedbw2;
	float tfixedbw1, tfixedbw2;
	float inertiavel1, inertiavel2;

	float lfospeed1, lfospeed2;
	float phase1,phase2;
	float lfofreq1, lfofreq2;
	float lfobw1, lfobw2;

	float lfoamp1, lfoamp2;
	float lfomid1, lfomid2;

	float inertiatick1, inertiatick2;

	avals aval;
	gvals gval;

};


mi::mi() {
	GlobalVals = &gval;
	AttrVals = (int *)&aval;
}
mi::~mi() { }

static double const NoteFreqs[12] = { 130.8127, 138.5913, 146.8324, 155.5635, 164.8138, 174.6141, 184.9972, 195.9977, 207.6523, 220, 233.0819, 246.9417 };
static float const OctaveMul[10] = { 1.0 / 16, 1.0 / 8, 1.0 / 4, 1.0 / 2, 1.0, 1.0 * 2, 1.0 * 4, 1.0 * 8, 1.0 * 16, 1.0 * 32 };

void mi::FixedFilterMe ()
{
    float omega, cs, sn, alpha;
	float b0, b1, b2, a0, a1, a2;
	float curlfofreq, curlfobw;

	// Fixed Filter 1
	if (filt1on == 1) {
		if ((tfixedfreq1 != fixedfreq1) || (tfixedbw1 != fixedbw1)) {
			if (fixedfreq1 > tfixedfreq1) {
				fixedfreq1 -= inertiavel1;
				if (fixedfreq1 <= tfixedfreq1) fixedfreq1 = tfixedfreq1;
			} else if (fixedfreq1 < tfixedfreq1) {
				fixedfreq1 += inertiavel1;
				if (fixedfreq1 >= tfixedfreq1) fixedfreq1 = tfixedfreq1;
			};
			if (fixedbw1 > tfixedbw1) {
				fixedbw1 -= inertiavel1;
				if (fixedbw1 <= tfixedbw1) fixedbw1 = tfixedbw1;
			} else if (fixedbw1 < tfixedbw1) {
				fixedbw1 += inertiavel1;
				if (fixedbw1 >= tfixedbw1) fixedbw1 = tfixedbw1;
			};

			omega = 2.0f * 3.141592654f * fixedfreq1 / pMasterInfo->SamplesPerSec;
			sn = (float)sin(omega); cs = (float)cos(omega);
			alpha = sn * sinh( pow( fixedbw1 / 2000.0, 4.0) * 4.0 + 0.1 * omega/sn);
			b0 =   1.0f;
			b1 =  -2.0f * cs;
			b2 =   1.0f;
			a0 =   1.0f + alpha;
			a1 =  -2.0f * cs;
			a2 =   1.0f - alpha;

			coefsTab1[0] = b0/a0;
			coefsTab1[1] = b1/a0;
			coefsTab1[2] = b2/a0;
			coefsTab1[3] = -a1/a0;
			coefsTab1[4] = -a2/a0;
		};
	};

	// Fixed Filter 2
	if (filt2on == 1) {
		if ((tfixedfreq2 != fixedfreq2) || (tfixedbw2 != fixedbw2)) {
			if (fixedfreq2 > tfixedfreq2) {
				fixedfreq2 -= inertiavel2;
				if (fixedfreq2 <= tfixedfreq2) fixedfreq2 = tfixedfreq2;
			} else if (fixedfreq2 < tfixedfreq2) {
				fixedfreq2 += inertiavel2;
				if (fixedfreq2 >= tfixedfreq2) fixedfreq2 = tfixedfreq2;
			};
			if (fixedbw2 > tfixedbw2) {
				fixedbw2 -= inertiavel2;
				if (fixedbw2 <= tfixedbw2) fixedbw2 = tfixedbw2;
			} else if (fixedbw2 < tfixedbw2) {
				fixedbw2 += inertiavel2;
				if (fixedbw2 >= tfixedbw2) fixedbw2 = tfixedbw2;
			};

			omega = 2.0f * 3.141592654f * fixedfreq2 / pMasterInfo->SamplesPerSec;
			sn = (float)sin(omega); cs = (float)cos(omega);
			alpha = sn * sinh( pow( fixedbw2 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
			b0 =   1.0f;
			b1 =  -2.0f * cs;
			b2 =   1.0f;
			a0 =   1.0f + alpha;
			a1 =  -2.0f * cs;
			a2 =   1.0f - alpha;

			coefsTab2[0] = b0/a0;
			coefsTab2[1] = b1/a0;
			coefsTab2[2] = b2/a0;
			coefsTab2[3] = -a1/a0;
			coefsTab2[4] = -a2/a0;	
		};
	};
	
	// LFO Filter
	if (filt3on == 1) {

		phase1 += lfospeed1;
		phase2 += lfospeed2;
		if (phase1 >= pi2) phase1 -= pi2;
		if (phase2 >= pi2) phase2 -= pi2;

		curlfofreq = sin(phase1) * lfoamp1 + lfomid1;
		curlfobw = sin(phase2) * lfoamp2 + lfomid2;

		omega = 2.0f * 3.141592654f * curlfofreq / pMasterInfo->SamplesPerSec;
		sn = (float)sin(omega); cs = (float)cos(omega);
		alpha = sn * sinh( pow( curlfobw / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
		b0 =   1.0f;
		b1 =  -2.0f * cs;
		b2 =   1.0f;
		a0 =   1.0f + alpha;
		a1 =  -2.0f * cs;
		a2 =   1.0f - alpha;

		coefsTab3[0] = b0/a0;
		coefsTab3[1] = b1/a0;
		coefsTab3[2] = b2/a0;
		coefsTab3[3] = -a1/a0;
		coefsTab3[4] = -a2/a0;

	};
}

void mi::MDKInit(CMachineDataInput * const pi)
{
    float omega, cs, sn, alpha, b0, b1, b2, a0, a1, a2;
	SetOutputMode( true );	//	If true, the MDKWork will never be called, meaning that Buzz will convert a mono signal to
							//	stereo itself and call MDKWorkStereo insted.
							//	If false, MDKWork will be called in mono cases, and the output should be mono
	icnt = 100;
	filt1on = 1; filt2on = 1; filt3on = 1;
	phase1 = 0.0f;
	phase2 = 0.0f;
	lfospeed1 = 0.0f;
	lfospeed2 = 0.0f;

	fixedfreq1 = 2000.0f;
	fixedfreq2 = 2000.0f;
	fixedbw1 = 100.0f;
	fixedbw2 = 100.0f;
	tfixedfreq1 = 2000.0f;
	tfixedfreq2 = 2000.0f;
	tfixedbw1 = 100.0f;
	tfixedbw2 = 100.0f;

	inertiavel1 = (2000.0f / pMasterInfo->SamplesPerSec) * 100.0f;
	inertiavel2 = (2000.0f / pMasterInfo->SamplesPerSec) * 100.0f;

	if (gval.filter1on != SWITCH_NO) filt1on = gval.filter1on ;
	if (gval.filter2on != SWITCH_NO) filt2on = gval.filter2on ;
	if (gval.filter3on != SWITCH_NO) filt3on = gval.filter3on ;

	if (aval.inertiatype == 0) {
		if (gval.freq1 != 0xFFFF) tfixedfreq1 = (float)(gval.freq1);
		if (gval.freq2 != 0xFFFF) tfixedfreq2 = (float)(gval.freq2);
		if (gval.bw1 != 0xFFFF) tfixedbw1 = (float)(gval.bw1);
		if (gval.bw2 != 0xFFFF) tfixedbw2 = (float)(gval.bw2);
		if (gval.inertia1 != 0xFFFF) inertiavel1 = (((float)gval.inertia1) / pMasterInfo->SamplesPerSec) * 100.0f;
		if (gval.inertia2 != 0xFFFF) inertiavel2 = (((float)gval.inertia2) / pMasterInfo->SamplesPerSec) * 100.0f;
	} else {
		if (gval.inertia1 != 0xFFFF) {
			inertiavel1 = (float)((tfixedfreq1 - fixedfreq1)/(((float)gval.inertia1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
			inertiatick1 = gval.inertia1;
		};
		if (gval.inertia2 != 0xFFFF) {
			inertiavel2 = (float)((tfixedfreq2 - fixedfreq2)/(((float)gval.inertia2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
			inertiatick2 = gval.inertia2;
		};
		if (gval.freq1 != 0xFFFF) {
			tfixedfreq1 = (float)(gval.freq1);
			inertiavel1 = (float)((tfixedfreq1 - fixedfreq1)/((inertiatick1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
		};
		if (gval.freq2 != 0xFFFF) {
			tfixedfreq2 = (float)(gval.freq2);
			inertiavel2 = (float)((tfixedfreq2 - fixedfreq2)/((inertiatick2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
		};
		if (gval.bw1 != 0xFFFF) {
			tfixedbw1 = (float)(gval.bw1);
			inertiavel1 = (float)((tfixedbw1 - fixedbw1)/((inertiatick1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
		};
		if (gval.bw2 != 0xFFFF) {
			tfixedbw2 = (float)(gval.bw2);
			inertiavel2 = (float)((tfixedbw2 - fixedbw2)/((inertiatick2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
		};
	};
	if (gval.freqspeed != 0xFFFF) lfospeed1 = (((2.0f * PI) / pMasterInfo->SamplesPerSec) / ((float)gval.freqspeed / 100.0f))* 100.0f;
	if (gval.bwspeed != 0xFFFF) lfospeed2 = (((2.0f * PI) / pMasterInfo->SamplesPerSec) / ((float)gval.bwspeed / 100.0f)) * 100.0f;

	if (gval.lfobw1 != 0xFFFF) {
		lfobw1 = (float)gval.lfobw1;
		if (lfobw2 > lfobw1)
		{
			lfoamp2 = ((lfobw2 - lfobw1) * 0.5f);
			lfomid2 = ((lfobw2 - lfobw1) * 0.5f) + lfobw1;
		} else {
			lfoamp2 = ((lfobw1 - lfobw2) * 0.5f);
			lfomid2 = ((lfobw1 - lfobw2) * 0.5f) + lfobw2;
		};
	};
	if (gval.lfobw2 != 0xFFFF) {
		lfobw2 = (float)gval.lfobw2;
		if (lfobw2 > lfobw1)
		{
			lfoamp2 = ((lfobw2 - lfobw1) * 0.5f);
			lfomid2 = ((lfobw2 - lfobw1) * 0.5f) + lfobw1;
		} else {
			lfoamp2 = ((lfobw1 - lfobw2) * 0.5f);
			lfomid2 = ((lfobw1 - lfobw2) * 0.5f) + lfobw2;
		};
	};
	if (gval.lfofreq1 != 0xFFFF) {
		lfofreq1 = (float)gval.lfofreq1;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
	if (gval.lfofreq2 != 0xFFFF) {
		lfofreq2 = (float)gval.lfofreq2;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
	
	lax1=0.0f; lay1=0.0f; lax2=0.0f; lay2=0.0f;
	lbx1=0.0f; lby1=0.0f; lbx2=0.0f; lby2=0.0f;
	lcx1=0.0f; lcy1=0.0f; lcx2=0.0f; lcy2=0.0f;
	rax1=0.0f; ray1=0.0f; rax2=0.0f; ray2=0.0f;
	rbx1=0.0f; rby1=0.0f; rbx2=0.0f; rby2=0.0f;
	rcx1=0.0f; rcy1=0.0f; rcx2=0.0f; rcy2=0.0f;

	omega = 2.0f * 3.141592654f * fixedfreq1 / pMasterInfo->SamplesPerSec;
	sn = (float)sin(omega); cs = (float)cos(omega);
	alpha = sn * sinh( pow( fixedbw1 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
	b0 =   1.0f;
	b1 =  -2.0f * cs;
	b2 =   1.0f;
	a0 =   1.0f + alpha;
	a1 =  -2.0f * cs;
	a2 =   1.0f - alpha;

	coefsTab1[0] = b0/a0;
	coefsTab1[1] = b1/a0;
	coefsTab1[2] = b2/a0;
	coefsTab1[3] = -a1/a0;
	coefsTab1[4] = -a2/a0;

	omega = 2.0f * 3.141592654f * fixedfreq2 / pMasterInfo->SamplesPerSec;
	sn = (float)sin(omega); cs = (float)cos(omega);
	alpha = sn * sinh( pow( fixedbw2 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
	b0 =   1.0f;
	b1 =  -2.0f * cs;
	b2 =   1.0f;
	a0 =   1.0f + alpha;
	a1 =  -2.0f * cs;
	a2 =   1.0f - alpha;

	coefsTab2[0] = b0/a0;
	coefsTab2[1] = b1/a0;
	coefsTab2[2] = b2/a0;
	coefsTab2[3] = -a1/a0;
	coefsTab2[4] = -a2/a0;
	
	omega = 2.0f * 3.141592654f * fixedfreq1 / pMasterInfo->SamplesPerSec;
	sn = (float)sin(omega); cs = (float)cos(omega);
	alpha = sn * sinh( pow( fixedbw1 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
	b0 =   1.0f;
	b1 =  -2.0f * cs;
	b2 =   1.0f;
	a0 =   1.0f + alpha;
	a1 =  -2.0f * cs;
	a2 =   1.0f - alpha;

	coefsTab3[0] = b0/a0;
	coefsTab3[1] = b1/a0;
	coefsTab3[2] = b2/a0;
	coefsTab3[3] = -a1/a0;
	coefsTab3[4] = -a2/a0;

	FixedFilterMe();
}

void mi::MDKSave(CMachineDataOutput * const po) { }

void mi::Tick() {
	int note, octv;

	if (gval.filter1on != SWITCH_NO) filt1on = gval.filter1on ;
	if (gval.filter2on != SWITCH_NO) filt2on = gval.filter2on ;
	if (gval.filter3on != SWITCH_NO) filt3on = gval.filter3on ;
//	if (gval.freq1 != 0xFFFF) tfixedfreq1 = (float)(gval.freq1);
//	if (gval.freq2 != 0xFFFF) tfixedfreq2 = (float)(gval.freq2);
//	if (gval.bw1 != 0xFFFF) tfixedbw1 = (float)(gval.bw1);
//	if (gval.bw2 != 0xFFFF) tfixedbw2 = (float)(gval.bw2);

	//if (gval.inertia1 != 0xFFFF) inertiavel1 = (((float)gval.inertia1) / pMasterInfo->SamplesPerSec) * 100.0f;
	//if (gval.inertia2 != 0xFFFF) inertiavel2 = (((float)gval.inertia2) / pMasterInfo->SamplesPerSec) * 100.0f;
	if (aval.inertiatype == 0) {
		if (gval.freq1 != 0xFFFF) tfixedfreq1 = (float)(gval.freq1);
		if (gval.freq2 != 0xFFFF) tfixedfreq2 = (float)(gval.freq2);
		if (gval.bw1 != 0xFFFF) tfixedbw1 = (float)(gval.bw1);
		if (gval.bw2 != 0xFFFF) tfixedbw2 = (float)(gval.bw2);
		if (gval.inertia1 != 0xFFFF) inertiavel1 = (((float)gval.inertia1) / pMasterInfo->SamplesPerSec) * 100.0f;
		if (gval.inertia2 != 0xFFFF) inertiavel2 = (((float)gval.inertia2) / pMasterInfo->SamplesPerSec) * 100.0f;
	} else {
		if (gval.inertia1 != 0xFFFF) {
			inertiavel1 = (float)((tfixedfreq1 - fixedfreq1)/(((float)gval.inertia1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
			inertiatick1 = gval.inertia1;
		};
		if (gval.inertia2 != 0xFFFF) {
			inertiavel2 = (float)((tfixedfreq2 - fixedfreq2)/(((float)gval.inertia2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
			inertiatick2 = gval.inertia2;
		};
		if (gval.freq1 != 0xFFFF) {
			tfixedfreq1 = (float)(gval.freq1);
			inertiavel1 = (float)((tfixedfreq1 - fixedfreq1)/((inertiatick1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
		};
		if (gval.freq2 != 0xFFFF) {
			tfixedfreq2 = (float)(gval.freq2);
			inertiavel2 = (float)((tfixedfreq2 - fixedfreq2)/((inertiatick2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
		};
		if (gval.bw1 != 0xFFFF) {
			tfixedbw1 = (float)(gval.bw1);
			inertiavel1 = (float)((tfixedbw1 - fixedbw1)/((inertiatick1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
		};
		if (gval.bw2 != 0xFFFF) {
			tfixedbw2 = (float)(gval.bw2);
			inertiavel2 = (float)((tfixedbw2 - fixedbw2)/((inertiatick2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
		};
	};

	if (gval.freqspeed != 0xFFFF) lfospeed1 = (((2.0f * PI) / pMasterInfo->SamplesPerSec) / ((float)gval.freqspeed / 100.0f))* 100.0f;
	if (gval.bwspeed != 0xFFFF) lfospeed2 = (((2.0f * PI) / pMasterInfo->SamplesPerSec) / ((float)gval.bwspeed / 100.0f)) * 100.0f;

	if (gval.lfobw1 != 0xFFFF) {
		lfobw1 = (float)gval.lfobw1;
		if (lfobw2 > lfobw1)
		{
			lfoamp2 = ((lfobw2 - lfobw1) * 0.5f);
			lfomid2 = ((lfobw2 - lfobw1) * 0.5f) + lfobw1;
		} else {
			lfoamp2 = ((lfobw1 - lfobw2) * 0.5f);
			lfomid2 = ((lfobw1 - lfobw2) * 0.5f) + lfobw2;
		};
	};
	if (gval.lfobw2 != 0xFFFF) {
		lfobw2 = (float)gval.lfobw2;
		if (lfobw2 > lfobw1)
		{
			lfoamp2 = ((lfobw2 - lfobw1) * 0.5f);
			lfomid2 = ((lfobw2 - lfobw1) * 0.5f) + lfobw1;
		} else {
			lfoamp2 = ((lfobw1 - lfobw2) * 0.5f);
			lfomid2 = ((lfobw1 - lfobw2) * 0.5f) + lfobw2;
		};
	};
	if (gval.lfofreq1 != 0xFFFF) {
		lfofreq1 = (float)gval.lfofreq1;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
	if (gval.lfofreq2 != 0xFFFF) {
		lfofreq2 = (float)gval.lfofreq2;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};

	if (gval.note1 != NOTE_NO) {
		if (gval.note1 != NOTE_OFF) {
			int note = (gval.note1 & 15) - 1;
			int octv = gval.note1 >> 4;
			tfixedfreq1 = (float)(NoteFreqs[note] * OctaveMul[octv]);
		};
		if (tfixedfreq1 > 21500.0f) tfixedfreq1 = 21500.0f;
		if (tfixedfreq1 < 5.0f) tfixedfreq1 = 5.0f;
	};
	if (gval.note2 != NOTE_NO) {
		if (gval.note2 != NOTE_OFF) {
			int note = (gval.note2 & 15) - 1;
			int octv = gval.note2 >> 4;
			tfixedfreq2 = (float)(NoteFreqs[note] * OctaveMul[octv]);
		};
		if (tfixedfreq2 > 21500.0f) tfixedfreq2 = 21500.0f;
		if (tfixedfreq2 < 5.0f) tfixedfreq2 = 5.0f;
	};
	if (gval.lfonote1 != NOTE_NO) {
		if (gval.lfonote1 != NOTE_OFF) {
			int note = (gval.lfonote1 & 15) - 1;
			int octv = gval.lfonote1 >> 4;
			lfofreq1 = (float)(NoteFreqs[note] * OctaveMul[octv]);
		};
		if (lfofreq1 > 21500.0f) lfofreq1 = 21500.0f;
		if (lfofreq1 < 5.0f) lfofreq1 = 5.0f;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
	if (gval.lfonote2 != NOTE_NO) {
		if (gval.lfonote2 != NOTE_OFF) {
			note = (gval.lfonote2 & 15) - 1;
			int octv = gval.lfonote2 >> 4;
			lfofreq2 = (float)(NoteFreqs[note] * OctaveMul[octv]);
		};
		if (lfofreq2 > 21500.0f) lfofreq2 = 21500.0f;
		if (lfofreq2 < 5.0f) lfofreq2 = 5.0f;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if (mode==WM_WRITE)
		return false;
	if (mode==WM_NOIO)
		return false;
	if (mode==WM_READ)		// <thru>
		return true;
	int i;
	float in, y;

	for( i=0; i<numsamples*2; i++ ) {
		icnt++;
		if (icnt >= 100) {
			icnt = 0;
			FixedFilterMe();
		};

		in = psamples[i];

		if (filt1on == 1) {
			y = coefsTab1[0] * in +
				coefsTab1[1] * lax1 +
				coefsTab1[2] * lax2 +
				coefsTab1[3] * lay1 +
				coefsTab1[4] * lay2;
			lay2 = lay1; lay1 = y; lax2 = lax1; lax1 = in; in = y;
		};

		if (filt2on == 1) {
			y = coefsTab2[0] * in +
				coefsTab2[1] * lbx1 +
				coefsTab2[2] * lbx2 +
				coefsTab2[3] * lby1 +
				coefsTab2[4] * lby2;
			lby2 = lby1; lby1 = y; lbx2 = lbx1; lbx1 = in; in = y;
		};

		if (filt3on == 1) {
			y = coefsTab3[0] * in +
				coefsTab3[1] * lcx1 +
				coefsTab3[2] * lcx2 +
				coefsTab3[3] * lcy1 +
				coefsTab3[4] * lcy2;
			lcy2 = lcy1; lcy1 = y; lcx2 = lcx1; lcx1 = in; in = y;
			if (lcy1 > 80000.0f) lcy1 = 80000.0f;
			if (lcx1 > 80000.0f) lcx1 = 80000.0f;
			if (lcy1 < -80000.0f) lcy1 = -80000.0f;
			if (lcx1 < -80000.0f) lcx1 = -80000.0f;
		};


		psamples[i] = in;
		i++;
		in = psamples[i];

		if (filt1on == 1) {
			y = coefsTab1[0] * in +
				coefsTab1[1] * rax1 +
				coefsTab1[2] * rax2 +
				coefsTab1[3] * ray1 +
				coefsTab1[4] * ray2;
			ray2 = ray1; ray1 = y; rax2 = rax1; rax1 = in; in = y;
		};

		if (filt2on == 1) {
			y = coefsTab2[0] * in +
				coefsTab2[1] * rbx1 +
				coefsTab2[2] * rbx2 +
				coefsTab2[3] * rby1 +
				coefsTab2[4] * rby2;
			rby2 = rby1; rby1 = y; rbx2 = rbx1; rbx1 = in; in = y;
		};

		if (filt3on == 1) {
			y = coefsTab3[0] * in +
				coefsTab3[1] * rcx1 +
				coefsTab3[2] * rcx2 +
				coefsTab3[3] * rcy1 +
				coefsTab3[4] * rcy2;
			rcy2 = rcy1; rcy1 = y; rcx2 = rcx1; rcx1 = in; in = y;
			if (rcy1 > 80000.0f) rcy1 = 80000.0f;
			if (rcx1 > 80000.0f) rcx1 = 80000.0f;
			if (rcy1 < -80000.0f) rcy1 = -80000.0f;
			if (rcx1 < -80000.0f) rcx1 = -80000.0f;
		};

		psamples[i] = in;
	};

	return true;
}



HINSTANCE dllInstance;
mi *g_mi;

BOOL WINAPI DllMain ( HANDLE hModule, DWORD fwdreason, LPVOID lpReserved )
{
	switch (fwdreason) {
	case DLL_PROCESS_ATTACH:
		dllInstance = (HINSTANCE) hModule;
		break;

	case DLL_THREAD_ATTACH:

		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		
		break;
	}
	return TRUE;
}

BOOL APIENTRY AboutDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_INITDIALOG:

		return 1;

	case WM_SHOWWINDOW:
		return 1;
	case WM_CLOSE:
		EndDialog (hDlg, TRUE);

	case WM_COMMAND:
		switch ( LOWORD (wParam))
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

void mi::Command(int const i)
{
    float omega, cs, sn, alpha, b0, b1, b2, a0, a1, a2;
	switch (i)
	{
	case 0:
		fixedfreq1 = 2000.0f;
		fixedfreq2 = 2000.0f;
		fixedbw1 = 100.0f;
		fixedbw2 = 100.0f;
		tfixedfreq1 = 2000.0f;
		tfixedfreq2 = 2000.0f;
		tfixedbw1 = 100.0f;
		tfixedbw2 = 100.0f;

		inertiavel1 = (2000.0f / pMasterInfo->SamplesPerSec) * 100.0f;
		inertiavel2 = (2000.0f / pMasterInfo->SamplesPerSec) * 100.0f;

	if (gval.filter1on != SWITCH_NO) filt1on = gval.filter1on ;
	if (gval.filter2on != SWITCH_NO) filt2on = gval.filter2on ;
	if (gval.filter3on != SWITCH_NO) filt3on = gval.filter3on ;
//	if (gval.freq1 != 0xFFFF) tfixedfreq1 = (float)(gval.freq1);
//	if (gval.freq2 != 0xFFFF) tfixedfreq2 = (float)(gval.freq2);
//	if (gval.bw1 != 0xFFFF) tfixedbw1 = (float)(gval.bw1);
//	if (gval.bw2 != 0xFFFF) tfixedbw2 = (float)(gval.bw2);
	if (aval.inertiatype == 0) {
		if (gval.freq1 != 0xFFFF) tfixedfreq1 = (float)(gval.freq1);
		if (gval.freq2 != 0xFFFF) tfixedfreq2 = (float)(gval.freq2);
		if (gval.bw1 != 0xFFFF) tfixedbw1 = (float)(gval.bw1);
		if (gval.bw2 != 0xFFFF) tfixedbw2 = (float)(gval.bw2);
		if (gval.inertia1 != 0xFFFF) inertiavel1 = (((float)gval.inertia1) / pMasterInfo->SamplesPerSec) * 100.0f;
		if (gval.inertia2 != 0xFFFF) inertiavel2 = (((float)gval.inertia2) / pMasterInfo->SamplesPerSec) * 100.0f;
	} else {
		if (gval.inertia1 != 0xFFFF) {
			inertiavel1 = (float)((tfixedfreq1 - fixedfreq1)/(((float)gval.inertia1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
			inertiatick1 = gval.inertia1;
		};
		if (gval.inertia2 != 0xFFFF) {
			inertiavel2 = (float)((tfixedfreq2 - fixedfreq2)/(((float)gval.inertia2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
			inertiatick2 = gval.inertia2;
		};
		if (gval.freq1 != 0xFFFF) {
			tfixedfreq1 = (float)(gval.freq1);
			inertiavel1 = (float)((tfixedfreq1 - fixedfreq1)/((inertiatick1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
		};
		if (gval.freq2 != 0xFFFF) {
			tfixedfreq2 = (float)(gval.freq2);
			inertiavel2 = (float)((tfixedfreq2 - fixedfreq2)/((inertiatick2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
		};
		if (gval.bw1 != 0xFFFF) {
			tfixedbw1 = (float)(gval.bw1);
			inertiavel1 = (float)((tfixedbw1 - fixedbw1)/((inertiatick1/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel1 < 0.0f) { inertiavel1 = -inertiavel1; };
		};
		if (gval.bw2 != 0xFFFF) {
			tfixedbw2 = (float)(gval.bw2);
			inertiavel2 = (float)((tfixedbw2 - fixedbw2)/((inertiatick2/500.0f) * pMasterInfo->SamplesPerTick)) * 100.0f;
			if (inertiavel2 < 0.0f) { inertiavel2 = -inertiavel2; };
		};
	};

	if (gval.freqspeed != 0xFFFF) lfospeed1 = (((2.0f * PI) / pMasterInfo->SamplesPerSec) / ((float)gval.freqspeed / 100.0f))* 100.0f;
	if (gval.bwspeed != 0xFFFF) lfospeed2 = (((2.0f * PI) / pMasterInfo->SamplesPerSec) / ((float)gval.bwspeed / 100.0f)) * 100.0f;

	if (gval.lfobw1 != 0xFFFF) {
		lfobw1 = (float)gval.lfobw1;
		if (lfobw2 > lfobw1)
		{
			lfoamp2 = ((lfobw2 - lfobw1) * 0.5f);
			lfomid2 = ((lfobw2 - lfobw1) * 0.5f) + lfobw1;
		} else {
			lfoamp2 = ((lfobw1 - lfobw2) * 0.5f);
			lfomid2 = ((lfobw1 - lfobw2) * 0.5f) + lfobw2;
		};
	};
	if (gval.lfobw2 != 0xFFFF) {
		lfobw2 = (float)gval.lfobw2;
		if (lfobw2 > lfobw1)
		{
			lfoamp2 = ((lfobw2 - lfobw1) * 0.5f);
			lfomid2 = ((lfobw2 - lfobw1) * 0.5f) + lfobw1;
		} else {
			lfoamp2 = ((lfobw1 - lfobw2) * 0.5f);
			lfomid2 = ((lfobw1 - lfobw2) * 0.5f) + lfobw2;
		};
	};
	if (gval.lfofreq1 != 0xFFFF) {
		lfofreq1 = (float)gval.lfofreq1;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
	if (gval.lfofreq2 != 0xFFFF) {
		lfofreq2 = (float)gval.lfofreq2;
		if (lfofreq2 > lfofreq1)
		{
			lfoamp1 = ((lfofreq2 - lfofreq1) * 0.5f);
			lfomid1 = ((lfofreq2 - lfofreq1) * 0.5f) + lfofreq1;
		} else {
			lfoamp1 = ((lfofreq1 - lfofreq2) * 0.5f);
			lfomid1 = ((lfofreq1 - lfofreq2) * 0.5f) + lfofreq2;
		};
	};
		
		lax1=0.0f; lay1=0.0f; lax2=0.0f; lay2=0.0f;
		lbx1=0.0f; lby1=0.0f; lbx2=0.0f; lby2=0.0f;
		lcx1=0.0f; lcy1=0.0f; lcx2=0.0f; lcy2=0.0f;
		rax1=0.0f; ray1=0.0f; rax2=0.0f; ray2=0.0f;
		rbx1=0.0f; rby1=0.0f; rbx2=0.0f; rby2=0.0f;
		rcx1=0.0f; rcy1=0.0f; rcx2=0.0f; rcy2=0.0f;

		omega = 2.0f * 3.141592654f * fixedfreq1 / pMasterInfo->SamplesPerSec;
		sn = (float)sin(omega); cs = (float)cos(omega);
		alpha = sn * sinh( pow( fixedbw1 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
		b0 =   1.0f;
		b1 =  -2.0f * cs;
		b2 =   1.0f;
		a0 =   1.0f + alpha;
		a1 =  -2.0f * cs;
		a2 =   1.0f - alpha;

		coefsTab1[0] = b0/a0;
		coefsTab1[1] = b1/a0;
		coefsTab1[2] = b2/a0;
		coefsTab1[3] = -a1/a0;
		coefsTab1[4] = -a2/a0;

		omega = 2.0f * 3.141592654f * fixedfreq2 / pMasterInfo->SamplesPerSec;
		sn = (float)sin(omega); cs = (float)cos(omega);
		alpha = sn * sinh( pow( fixedbw2 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
		b0 =   1.0f;
		b1 =  -2.0f * cs;
		b2 =   1.0f;
		a0 =   1.0f + alpha;
		a1 =  -2.0f * cs;
		a2 =   1.0f - alpha;

		coefsTab2[0] = b0/a0;
		coefsTab2[1] = b1/a0;
		coefsTab2[2] = b2/a0;
		coefsTab2[3] = -a1/a0;
		coefsTab2[4] = -a2/a0;

		omega = 2.0f * 3.141592654f * fixedfreq1 / pMasterInfo->SamplesPerSec;
		sn = (float)sin(omega); cs = (float)cos(omega);
		alpha = sn * sinh( pow( fixedbw1 / 2000.0f, 4.0f)*4.0 + 0.1 * omega/sn);
		b0 =   1.0f;
		b1 =  -2.0f * cs;
		b2 =   1.0f;
		a0 =   1.0f + alpha;
		a1 =  -2.0f * cs;
		a2 =   1.0f - alpha;

		coefsTab3[0] = b0/a0;
		coefsTab3[1] = b1/a0;
		coefsTab3[2] = b2/a0;
		coefsTab3[3] = -a1/a0;
		coefsTab3[4] = -a2/a0;
		
		FixedFilterMe();

		break;
	case 1:
		g_mi=this;
		DialogBox(dllInstance, MAKEINTRESOURCE (IDD_CYANABOUT), GetForegroundWindow(), (DLGPROC) &AboutDialog);

		break;
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
	case 5:
	case 10:
		switch (value) {
		case 0: return "off"; break;
		case 1: return "on"; break;
		default: return NULL; break;
		}
		break;
	case 1:
	case 6:
	case 11:
	case 12:
		sprintf(txt,"%.1f Hz", (float)value );
		return txt;
		break;

	case 3:
	case 8:
	case 16:
	case 17:
		sprintf(txt,"%i", value);
		return txt;
		break;

	case 13:
	case 18:
		sprintf(txt,"%.2f secs", ((float)value / 100.0f));
		return txt;
		break;

	case 4:
	case 9:
		switch (aval.inertiatype)
		{
		case 0: sprintf(txt,"%i u/s", (int)(value) ); break;
		case 1: sprintf(txt,"%.2f ticks", ((float)value/500.0f) ); break;
		default: sprintf(txt,"%i N\\A", (int)(value) ); break;
		};
		return txt;
		break;


	default:
		return NULL;
	}
}

#pragma optimize ("", on) 

DLL_EXPORTS

