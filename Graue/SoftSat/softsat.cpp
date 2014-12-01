// Public domain, do whatever you want with this.
// Graue <graue@oceanbase.org>, July 16, 2006

#include <stdlib.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

CMachineParameter const paraThreshold =
{
	pt_word,                        // Type
	"Threshold",                    // Name
	"Threshold",                    // Description
	16,                             // Min value
	32768,                          // Max value
	0,                              // No value
	MPF_STATE,                      // Flags (here, display in dbl-click window)
	32768                           // Default value
};

CMachineParameter const paraHardness =
{
	pt_word,                        // Type
	"Hardness",                     // Name
	"Hardness",                     // Description
	1,                              // Min value
	2048,                           // Max value
	0,                              // No value
	MPF_STATE,                      // Flags (here, display in dbl-click window)
	1024                            // Default value
};

CMachineParameter const *pParameters[] =
{
	// global parameters
	&paraThreshold,
	&paraHardness
};

CMachineAttribute const *pAttributes[] =
{
	NULL
};

#pragma pack(1)

class gvals // Global param values; track would be tvals
{
	public:
        word threshold;
        word hardness;
};

#pragma pack()

CMachineInfo const MacInfo =
{
	MT_EFFECT,                      // Type
	MI_VERSION,                     // Machine interface version
	MIF_DOES_INPUT_MIXING,          // Flags
	0,                              // Minimum tracks
	0,                              // Maximum tracks
	2,                              // Num global parameters
	0,                              // Num track parameters
	pParameters,                    // Parameters
	0,                              // Num attributes
	pAttributes,                    // Attributes
	"Graue SoftSat",                // Name
	"SoftSat",                      // Short name
	"Graue",                        // Author
	"About"                         // Commands
};

class miex : public CMDKMachineInterfaceEx { };

class mi: public CMDKMachineInterface
{
	public:
		mi();
		virtual ~mi();
		virtual void Tick();
		virtual void MDKInit(CMachineDataInput * const pi);
		virtual bool MDKWork(float *psamples, int numsamples, int const mode);
		virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
		virtual void Command(int const i);
		virtual void MDKSave(CMachineDataOutput * const po) { }
		virtual char const *DescribeValue(int const param, int const value);
		virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
		virtual void OutputModeChanged(bool stereo) { }

	public:
		miex ex;

	private:
		gvals gval;
		word realthreshold;
		word realhardness;
};

mi::mi() {
	GlobalVals = &gval;
}

mi::~mi() { }

void mi::MDKInit(CMachineDataInput * const pi)
{
	SetOutputMode(true); // no mono sounds
	realthreshold = gval.threshold;
	realhardness = gval.hardness;
}

void mi::Tick()
{
	if (gval.threshold > 0) realthreshold = gval.threshold;
	if (gval.hardness > 0) realhardness = gval.hardness;
}

bool mi::MDKWork(float *psamples, int numsamples, const int mode)
{
	return false; // mono mode not supported
}

// I have no idea why this formula works, but it does.
#define SHAPE							\
	if (smp > hardness)					\
	{							\
		smp = hardness + (smp-hardness)			\
			/ (1+((smp-hardness)/(1-hardness))	\
			   * ((smp-hardness)/(1-hardness)));	\
	}							\
	if (smp > 1.0f)						\
		smp = (hardness + 1.0f) / 2.0f;

bool mi::MDKWorkStereo(float *psamples, int numsamples, const int mode)
{
	const float hardness = (float)realhardness / 2049.0f; // 2049, so it never becomes 1.0
	const float threshold = (float)realthreshold / ((hardness+1)/2);

	if(mode == WM_WRITE || mode == WM_NOIO)
		return false;
	if(mode == WM_READ) // _thru_
		return true;

	numsamples *= 2; // get the real number of samples (as this is stereo)
	for (; numsamples > 0; numsamples--, psamples++)
	{
		float smp = *psamples / threshold;

		if (smp > 0)
		{
			SHAPE
		}
		else
		{
			smp = -smp;
			SHAPE
			smp = -smp;
		}

		*psamples = smp * threshold;
	}

	return true;
}

char const *mi::DescribeValue(const int param, const int value)
{
	static char textstring[20];

	switch(param) {
		case 0: // paraThreshold
			sprintf(textstring, "%d", value);
			break;
		case 1: // paraHardness
			sprintf(textstring, "%.3f", (float)value / 2048.0f);
			break;
		default: // Error
			strcpy(textstring, "Invalid parameter");
			break;
	}

	return textstring;
}

#define ABOUTSTRING \
	"SoftSat 1.0 by Graue. Made July 16, 2006.\n" \
	"email: graue@oceanbase.org\n\n" \
	"This is a saturating limiter that I like to use for mastering purposes.\n" \
	"It adds a relatively soft distortion while preventing the sound from clipping."

void mi::Command(int const i)
{
	switch(i)
	{
		case 0:
			pCB->MessageBox(ABOUTSTRING);
	}
}

#pragma optimize ("", on)

DLL_EXPORTS
