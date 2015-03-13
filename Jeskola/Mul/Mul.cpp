#include <windef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>
#include <auxbus/auxbus.h>

CMachineParameter const paraDummy = { pt_byte, "Dummy", "Dummy", 0, 127, 255, MPF_STATE, 0 };

CMachineParameter const *pParameters[] =
{
	// global
	&paraDummy
};


#pragma pack(1)

class gvals
{
public:
	byte dummy;
};


#pragma pack()

CMachineInfo const MacInfo =
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	1,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Jeskola Multiplier (Debug build)",		// name
#else
	"Jeskola Multiplier",					// name
#endif
	"Multiplier",									// short name
	"Oskari Tammelin",						// author
	"Set Channel"
};


class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

	virtual void Command(int const i);
	virtual void Save(CMachineDataOutput * const po);

	void DisconnectAux();

private:
	int Channel;


	gvals gval;

};

DLL_EXPORTS

mi::mi()
{
	GlobalVals = &gval;
}

mi::~mi()
{
	AB_Disconnect(this);
}

void cb(void *user)
{
	mi *pmi = (mi *)user;
	pmi->DisconnectAux();
}


#define VERSION		1

void mi::Init(CMachineDataInput * const pi)
{
	if (pi != NULL)
	{
		byte ver;
		pi->Read(ver);
		if (ver == VERSION)
		{
			pi->Read(Channel);

			if (Channel != -1)
				AB_ConnectOutput(Channel, MacInfo.ShortName, cb, this);
		}

	}
	else
	{
		Channel = -1;
	}
}

void mi::Save(CMachineDataOutput * const po)
{
	po->Write((byte)VERSION);
	po->Write(Channel);
}


void mi::Tick()
{
}


bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (Channel != -1 && mode & WM_READ)
	{
		float *paux = pCB->GetAuxBuffer();		// note: AuxBuffer and AuxBus are not related in any way

		AB_Receive(Channel, paux, numsamples);

		do
		{

			double i = *psamples * (1.0 / 32768.0);
			double a = *paux++ * (1.0 / 32868.0);

			double o = i * a;

			if (o < -1.0)
				o = -1.0;
			else if (o > 1.0)
				o = 1.0;

			*psamples++ = (float)(o * 32768.0);

		} while(--numsamples);

		return true;
	}
	else
	{
		// no aux connected or no input, pass signal thru unmodified
		return mode & WM_READ;
	}

}



void mi::DisconnectAux()
{
	MACHINE_LOCK;

	Channel = -1;
}


void mi::Command(int const i)
{
	if (i == 0)
		AB_ShowEditor(NULL, &Channel, MacInfo.ShortName, cb, this);
}
