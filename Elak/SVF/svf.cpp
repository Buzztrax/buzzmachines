/* $Id$
 *
 * buzzmachines
 * Copyright (C) 2007 Johan Larsby <larsby@algonet.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
* Elak SVF
* http://larsby.com/johan/
**/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <MachineInterface.h>


CMachineParameter const paraFC =
{
	pt_word,										// type
	"cf",
	"Cut-off frequency",							// description
	0,												// MinValue
	1000,											// MaxValue
	2000,											// NoValue
	MPF_STATE,										// Flags
	0x200,
};

CMachineParameter const paraResonance =
{
	pt_word,										// type
	"Resonance",
	"Resonance",									// description
	0,												// MinValue
	0xFFFE,											// MaxValue
	0xFFFF,											// NoValue
	MPF_STATE,										// Flags
	0x200,
};


CMachineParameter const *pParameters[] =
{
	// global
	&paraFC,
	&paraResonance
};

CMachineAttribute const attrSymmetric =
{
	"Symmetric",
	0,
	1,
	0
};


#pragma pack(1)

class gvals
{
public:
	word fc;
	word resonance;
};

#pragma pack()

CMachineInfo const MacInfo =
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	2,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	0,
#ifdef _DEBUG
	"SVF Filter (Debug build)",				// name
#else
	"SVF Filter",							// name
#endif
	"SVF",									// short name
	"Elak",									// author
	NULL
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

private:



private:
	float fc;
	float resonance;
	float fs;
	float hi;
	float lo;
	float bp;
	float f;

	gvals gval;
};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
}

void mi::Init(CMachineDataInput * const pi)
{
	fc = 500.0f;

	fs = 22500.0f;


	lo = 0.0f;
	bp = 0.0f;
	f  = 0.0f;
	hi = 0.0f;

    lo = bp = 0.0f;

}

void mi::Tick()
{

	if (gval.fc != paraFC.NoValue)
		fc = (float)(gval.fc);

	if( gval.resonance != paraResonance.NoValue)
		resonance = (float)(gval.resonance)/65535;

	//these should really be initialized after a note has is finished playing, or is it when f = 0;
    f = 6*fc/fs;
}



bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (mode == WM_WRITE || mode == WM_NOIO)
		return false;

	if (mode == WM_READ)
		return true;
	do
	{
		float in = *psamples/65534.0f; //calculations on float, more fun

		hi  = in - lo - (1-resonance)*bp;
		bp += f*hi;
		lo += f*bp;

		if (lo >  1.0f) lo =  1.0f;
		if (lo < -1.0f) lo = -1.0f;

		*psamples = (lo*65534.0f);	//boost the volume ?!?
		f *= 0.99998f;          // exponential - case

		psamples++;

	} while(--numsamples);

	return true;
}


