/* 
 * Copyright (C) 2001 Radoslaw Dutkiewicz <radicdotkey@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses>.
 */

#include <math.h>
#include <float.h>
#include <assert.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

#define ANTIDENORMAL 1.0E-12f

#pragma optimize ("awy", on)



CMachineParameter const paraDrive=
{
	pt_byte,
	"Drive",
	"Drive Level",
	1,
	0xF0,
	0xFF,
	MPF_STATE,
	60
};

CMachineParameter const paraPreFilter=
{
	pt_byte,
	"Pre Cut",
	"Pre Filter Cutoff",
	1,
	0xEF,
	0xFF,
	MPF_STATE,
	0x80
};

CMachineParameter const paraPostFilter=
{
	pt_byte,
	"Post Cut",
	"Post Filter Cutoff",
	1,
	0xEF,
	0xFF,
	MPF_STATE,
	0x3C
};

CMachineParameter const paraAmount=
{
	pt_byte,
	"Amount",
	"Amount of Excitement",
	0,
	0xF0,
	0xFF,
	MPF_STATE,
	0x3C
};


CMachineParameter const *pParameters[]=
{
	&paraDrive,
	&paraPreFilter,
	&paraPostFilter,
	&paraAmount
};

CMachineAttribute const *pAttributes[]={NULL};


#pragma pack(1)

class gvals
{
public:
	byte drive;
	byte prefilter;
	byte postfilter;
	byte amount;
};
class avals {};

#pragma pack()

CMachineInfo const MacInfo=
{
	MT_EFFECT,
	MI_VERSION,
	MIF_DOES_INPUT_MIXING,
	0,
	0,
	4,
	0,
	pParameters,
	0,
	pAttributes,
	"Oomek Exciter",
	"Exciter",
	"Oomek",
	"&About..."
};

class miex : public CMDKMachineInterfaceEx{};

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
	inline float clip (float x, float a, float b);

	
public:
	miex ex;

	float precut, postcut;
	float drlv;
	float amlv;
	float zL1,zR1,zL2,zR2;
	float inL, inR;
	gvals gval;
	avals aval;

	int ModeRelease;
	float AntiDenormal;

};



mi::mi()
{
	GlobalVals=&gval;
	AttrVals = (int *)&aval;
}
mi::~mi(){}

inline float mi::clip (float x, float a, float b) 
{ 
   
   float x1 = (float)fabs (x-a); 
   float x2 = (float)fabs (x-b); 
   x = x1 + (a+b); 
   x -= x2; 
   x *= 0.5; 
   return (x); 
}

void mi::MDKInit(CMachineDataInput *const pi)
{
	SetOutputMode(true);
	zL1=0.0f;
	zR1=0.0f;
	zL2=0.0f;
	zR2=0.0f;

	ModeRelease = 512;
	AntiDenormal = ANTIDENORMAL * 2.0f;
}




void mi::MDKSave(CMachineDataOutput *const po){}

void mi::Tick()
{
	if (gval.amount != 0xFF) amlv = ((float)gval.amount * 0.0041666666667f * 2); // (1 / 240)
	if (gval.prefilter != 0xFF) precut = (float)gval.prefilter * 0.0041666666667f;
	if (gval.postfilter != 0xFF) postcut = (float)gval.postfilter * 0.0041666666667f;
	if (gval.drive != 0xFF) drlv = (float)gval.drive * 0.0083333333333f;// (1 / 120)
}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
	if (mode==WM_WRITE) return false;
	if (mode==WM_NOIO) return false;
	if (mode==WM_READ) return true;

	do
	{
		*psamples = 0;
		psamples ++;
	} while (--numsamples);

	return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
	if (mode==WM_WRITE && ModeRelease <= 0) return false;
	if (mode==WM_NOIO) return false;
	if (mode==WM_READ) return true;
	if (mode==WM_READWRITE) ModeRelease = 512;

	do
	{

////////////// RIGHT //////////////////

		if (mode==WM_WRITE) *psamples = 0.0f;
		--ModeRelease;


//		*psamples = clip (*psamples, -32767.0f * 16.0f, 32767.0f * 16.0f);

		if (*psamples > (32768.0f * 32.0f)) *psamples = (32768.0f * 32.0f);
		if (*psamples < (-32768.0f * 32.0f)) *psamples = (-32768.0f * 32.0f);

		AntiDenormal *= -1.0f;
		*psamples += AntiDenormal;
		*psamples += ANTIDENORMAL;

		inR = *psamples;

		zR1 = ((*psamples - zR1) * precut) + zR1;			//HP pre
		*psamples = *psamples - zR1;						//HP pre

		*psamples *= 0.00048828125; // /= 2048.0f;
		*psamples *= drlv;
		*psamples = (2.0f * *psamples) / (float)(1.0 + 2.0 * fabs(*psamples)); // SHAPER
		*psamples *= 32768.0f;

		zR2 = ((*psamples - zR2) * postcut) + zR2;			//HP post
		*psamples = *psamples - zR2;						//HP post

		*psamples = *psamples * amlv + inR;

		psamples ++;



////////////// LEFT //////////////////

		if (mode==WM_WRITE) *psamples = 0.0f;


//		*psamples = clip (*psamples, -32767.0f * 16.0f, 32767.0f * 16.0f);

		if (*psamples > (32768.0f * 32.0f)) *psamples = (32768.0f * 32.0f);
		if (*psamples < (-32768.0f * 32.0f)) *psamples = (-32768.0f * 32.0f);

		*psamples += AntiDenormal;
		*psamples += ANTIDENORMAL;

		inL = *psamples;

		zL1 = ((*psamples - zL1) * precut) + zL1;			//HP pre
		*psamples = *psamples - zL1;						//HP pre
		
		*psamples *= 0.00048828125; // /= 2048.0f;
		*psamples *= drlv;
		*psamples = (2.0f * *psamples) / (float)(1.0 + 2.0 * fabs(*psamples)); // SHAPER
		*psamples *= 32768.0f;

		zL2 = ((*psamples - zL2) * postcut) + zL2;			//HP post
		*psamples = *psamples - zL2;						//HP post

		*psamples = *psamples * amlv + inL;

		psamples ++;

	} while (--numsamples);




	return true;
}

void mi::Command(int const i)
{
	switch(i)
	{
	case 0:
		pCB->MessageBox("\n\nOomek's Exciter \n v1.1 (optimized & bugfixed)\n\nRadoslaw Dutkiewicz\nmailto:oomek@go2.pl\n\n");
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
		sprintf(txt,"%.0f", (float)value);
		return txt;
		break;

	case 1:
	case 2:
		sprintf(txt,"%.0f Hz", ((float)value  * 0.0041666666667f) / (6.28319 / pMasterInfo->SamplesPerSec)+1);
		return txt;
		break;

	case 3:
		if (value == 0) sprintf(txt,"-inf dB");
		else sprintf(txt,"%.1f dB", 20*log10(((float)value * 0.0041666666667f * 2.0f)));
		return txt;
		break;
	default:
		return NULL;
	}
}

#pragma optimize("",on)

DLL_EXPORTS



