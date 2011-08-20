// Chimp's PitchShifter v1.1a
// Simple buffer-based pitch shifting
// buffer size is adjustable (it's one of the machine's attributes)

// TODO: 
//	*	implement some kind of filtering or overlap to remove
//		unwanted discontinuities (clicks) in the output signal
//		caused by the writehead overtaking the readhead
//			(when shifting pitch down)
//		or by the readhead overtaking the writehead
//			(when shifting pitch up)
//
//		(i have already created an attribute to deal with
//		click removal, it just hasn't been used yet)
//
//	*	some bugs need fixing (namely those concerned with the
//		buffer sometimes containing 'old' data and emitting
//		unwanted sounds)
//
//	*	minor bug: when changing the bufferlength attribute
//		whilst the machine is playing generates a noticeable
//		click. i have left this since it is fair to assume you
//		aren't going to change the attributes very often when
//		a buzzsong is playing!
//
//	*	related bug: this machine only senses a change in
//		the master sample rate when the machine is stopped.
//		the reasoning behind this is that it puts a non-trivial
//		load on the CPU to keep checking that the sample rate
//		hasn't changed, and also that it's fair to assume you
//		aren't going to change the samplerate very often when
//		a buzzsong is playing. just stop the buzzsong, restart it,
//		and the pitchshifter will work properly again.
//
//	The REAL Todo list:
//
//	*	Make the size of the buffer dependent upon the fundamental
//		frequency of the input signal (ie, when the frequency of
//		the input signal changes, the buffer size changes accordingly
//		to generate a more authentic pitch scaling effect)
//
//	*	Ramping for smoother changes of pitch
//
//	*	Consistency (for example, a pitch shift of zero should
//		NEVER introduce a delay... at the moment, i have gotten
//		away with this as a special case but it should really be
//		built into the algorithm)
//
//	*	Better pitch shifting algorithm? (eg, using ffts / formants)
//
// (c) 1998 dave hooper @ spc

#ifndef _DEBUG
#pragma optimize ("awy",on)
#endif

#include <math.h>
#include <windef.h>

#include <MachineInterface.h>

CMachineParameter const paraShift = 
{ 
	pt_word,										// type
	"Shift",
	"PitchShift in cents (0 = no shift, 2400 = 2 octaves (24 semitones) shift)",
	0,												// MinValue	
	2400,											// MaxValue
	2500,											// NoValue
	MPF_STATE,										// Flags
	1200											// DefValue
};

CMachineParameter const paraDirection =
{ 
	pt_byte, 										// type
	"Direction",
	"Shift Direction (0 = down, 1 = up)",			// description
	0,												// MinValue	
	1,												// MaxValue
	2,												// NoValue
	MPF_STATE,										// Flags
	0												// DefValue
};

CMachineParameter const paraDry = 
{ 
	pt_byte,										// type
	"Dry out",
	"Dry out (00 = -100%, 40 = 0%, 80 = 100%)",		// description
	0x00,											// MinValue	
	0x80,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0x80											// DefValue
};

CMachineParameter const paraWet = 
{ 
	pt_byte,										// type
	"Wet out",
	"Wet out (00 = -100%, 40 = 0%, 80 = 100%)",		// description
	0x00,											// MinValue	
	0x80,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0x80											// DefValue
};


CMachineAttribute const attrBufferlength = 
{
	"Buffer Size (ms)",								// description
	2,												// MinValue
	1000,											// MaxValue
	30												// DefValue
};

CMachineAttribute const attrOverlap =
{
	"Overlap Amount on Heads Passed (%)",			// description
	0,												// MinValue
	100,											// MaxValue
	0												// DefValue
};


CMachineParameter const *pParameters[] = 
{ 
	&paraShift,
	&paraDirection,
	&paraDry,
	&paraWet
};

CMachineAttribute const *pAttributes[] =
{
	&attrBufferlength,
	&attrOverlap,
};



#pragma pack(1)

class gvals
{
public:
	word shift;
	byte direction;
	byte dry;
	byte wet;
};

class avals
{
public:
	int bufferlength;
	int overlap;
};

#pragma pack()



CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,								// always MI_VERSION
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	4,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	2,										// numAttributes
	pAttributes,
#ifdef _DEBUG
	"Chimp's PitchShifter v1.1 (Debug)",	// name
#else
	"Chimp's PitchShifter v1.1",
#endif
	"PitchShifter",							// short name
	"Dave Hooper",							// author
	"&About..."								// commands
};



// derive a new class for this machine
class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void AttributesChanged(void);
	virtual void Command(int const command);
	virtual char const *DescribeValue(int const param, int const value);
	virtual void Init(CMachineDataInput * const pi);
	virtual void Stop(void);
	virtual void Tick(void);
	virtual bool Work(float *psamples, int numsamples, int const mode);

private:
	gvals gval;
	avals aval;

	// variables concerned with the buffer
	float * buffer; // the buffer itself
	bool buffer_zeroed; // flag to say whether the buffer is empty - not fully implemented
	unsigned short bufferlength, writeposition; // allows for a buffer up to 64k long
	unsigned long readposition, pitchadjustment, bufferlength_times_64k; // 32 bits allows for a buffer up to 64k long, with 16 bits of fractional accuracy for fractional positions of the read head

	// variables concerned with keeping track of internal data
	int SamplesPerSec;
	unsigned short Shift;
	short Direction;
	float Wet,Dry;

};



DLL_EXPORTS


mi::mi()
{
	GlobalVals = &gval;
	TrackVals = NULL;
	AttrVals = (int *)&aval;
}


mi::~mi()
{
	// free up any allocated memory in the destructor
	// in this case it's just the buffer
	delete[] buffer;
}


void mi::AttributesChanged(void)
{
	// recalculate the bufferlength and associated variables,
	// reallocate the buffer and set the read and write heads
	// to valid positions within the buffer
	// note:	the way i'm resetting the read and write heads
	//			WILL cause clicking when you change the attributes
	bufferlength = (unsigned short)(SamplesPerSec * (aval.bufferlength/1000.0f));
	delete[] buffer;
	buffer = new float[bufferlength];

	bufferlength_times_64k = bufferlength << 16;
	writeposition%=bufferlength;
	readposition%=bufferlength_times_64k;
}


void mi::Command(int const command)
{
	// called when the machine's context menu generates a
	// command request. in this case, the only command
	// available is 'About...' so popup an About box
	
	switch(command)
	{
	case 0:
		pCB->MessageBox(
		    "Chimp's PitchShifter v1.1\n\n"
				"Copyright (c) 1998 no-brain@mindless.com\n\n"
				"If you like and use Chimp's plugins, you can\n"
				"support the author by sending any amount of cash\n"
				"(in any currency) to the following address\n\n"
				"\tdave hooper\n\t2a corringway fleet\n\thants gu13 0an\n\tUK");
		break;
	default:
		break;
	}
}

						 
char const *mi::DescribeValue(int const param, int const value)
{
	// display information in the machine's parameters dialog.
	// returning NULL means buzz interprets the value data
	// in its default manner - currently, buzz's default
	// representation is the number itself
	
	static char txt[16];

	switch(param)
	{
	case 0:		// shift
		sprintf(txt,"%.2f", (double)(value)/100.0);
		break;
	case 1:		// direction
		switch(value)
		{
		case 0: return "down";
		case 1: return "up";
		default: return NULL;
		}
	case 2:		// dry
	case 3:		// wet
		sprintf(txt, "%.1f%%", 100.0*(((double)(value)/(float)0x40)-1));
		break;
	default:	// ...else...
		return NULL;
	}

	return txt;
}


void mi::Init(CMachineDataInput * const pi)
{

	// called when the machine is LOADED into buzz at STARTUP

	// initialise machine defaults that require no thought at all
	readposition=writeposition=0;
	Shift=gval.shift;
	SamplesPerSec = pMasterInfo->SamplesPerSec;
	
	// deal with the possibilty that this machine has replaced
	// a previous version that didn't use attributes
	if (aval.bufferlength < 2) aval.bufferlength=2;

	// initialise other machine defaults
	bufferlength = (unsigned short)(SamplesPerSec * (aval.bufferlength/1000.0f));
	Direction=(gval.direction)*2-1;
	Dry=((gval.dry)/(float)0x40)-1;
	Wet=((gval.wet)/(float)0x40)-1;
	pitchadjustment = (unsigned long)(65536.0*pow(2.0,Direction*(Shift/1200.0)));

	// create some 'derived' constants
	bufferlength_times_64k = bufferlength << 16;
	
	//  allocate the buffer and empty it
	buffer = new float[bufferlength];
	memset(buffer,0,bufferlength*sizeof(float));
	buffer_zeroed = true;

	// the memory just allocated is deallocated in ~mi
	// and also in AttributesChanged (where it is reallocated
	// with a different size)
}


void mi::Stop(void)
{
	// if the samplerate has changed, update the internal record
	// of the samplerate and call AttributesChanged to force the
	// bufferlength to be recalculated, since it depends on the
	// current sample rate

	if (pMasterInfo->SamplesPerSec != SamplesPerSec)
	{
		SamplesPerSec=pMasterInfo->SamplesPerSec;
		AttributesChanged();
	}
}


void mi::Tick(void)
{
	// called every 'tick' generated by the buzz engine
	// all i do here is update the internal data since there might
	// be a pattern in the current buzzsong which controls
	// the PitchShifter

	if (gval.shift != paraShift.NoValue)
	{
		// recalculate pitchadjustment, based on the new shift
		Shift = gval.shift;

		// also have to check direction hasn't changed
		if (gval.direction != paraDirection.NoValue)
			Direction = (gval.direction)*2-1;

		pitchadjustment = (unsigned long)(65536.0*pow(2.0,Direction*(Shift/1200.0)));
	}
	else if (gval.direction != paraDirection.NoValue)
	{
		// recalculate pitchadjustment, based on the new direction
		Direction = (gval.direction)*2-1;
		pitchadjustment = (unsigned long)(65536.0*pow(2.0,Direction*(Shift/1200.0)));
	}

	// recalculate Dry and Wet
	if (gval.dry != paraDry.NoValue)
		Dry = ((gval.dry)/(float)0x40)-1;

	if (gval.wet != paraWet.NoValue)
		Wet = ((gval.wet)/(float)0x40)-1;

}


bool mi::Work(float *psamples, int numsamples, int const mode)
{

	// called repeatedly when a buzzsong is playing

	// this machine is not a generator, so it only
	// does work if it has an input AND an output.

	if (!(mode & WM_READ))
	{
		// if it has no output, then we're ok to ignore the
		// input too
		if (!buffer_zeroed)
		{
			buffer_zeroed=true;
			memset(buffer,0,bufferlength*sizeof(float));
		}

		// tell buzz we've effectively done nothing
		return false;
	}

	if (mode == WM_READ)
	{
		// if we ONLY have output (no input) then there IS
		// no input so empty the buffer
		if (!buffer_zeroed)
		{
			buffer_zeroed=true;
			memset(buffer,0,bufferlength*sizeof(float));
		}
		
		// and tell buzz we've effectively done nothing
		return false;
	}

	
	// else, perform the pitch shift

	// create local copies of certain constants, because they
	// can be accessed quicker this way

	unsigned short const BUFFERLENGTH=bufferlength;
	unsigned long const BUFFERLENGTH_TIMES_64K=bufferlength_times_64k;

	if (Shift!=0 && Wet!=0)
	{
		// in the case where the parameters of the machine are
		// such that there IS a pitch shift (ie, Shift is not
		// set to zero) and there IS some wet output, then 
		// pitch shifting takes place here

		// signify that the buffer is not empty and that it's
		// contents are important
		buffer_zeroed = false;

		// generate the output based on the input
		// on input psamples points to the input data, which
		// is ALSO the output data. in other words, to 'effect' the
		// input data, we just change the contens of the buffer
		// pointed to by psamples

		do
		{
			// write to buffer position currently under write head
			buffer[writeposition] = *psamples;
			// advance write head by one position
			// write head wraps around from the end of the buffer to the beginning
			writeposition=(writeposition+1)%BUFFERLENGTH;

			// generate the output as the sum of the unaffected input
			*psamples = (float)((Dry* (*psamples))+
			// and the data currently under the read head
						(Wet * buffer[readposition >> 16]));

			// note: the top 16 bits of readposition index into the 
			// buffer; the bottom 16 bits of readposition are for increased
			// accuracy when moving the read head

			// advance the input pointer to the next sample
			psamples++;

			// advance the read head by the amount determined by
			// the current contents of the pitchadjustment register
			// read head wraps around from the end of the buffer to the beginning
			readposition=(readposition+pitchadjustment) % (BUFFERLENGTH_TIMES_64K);

		} while (--numsamples);
	}
	else 
	{
		// no effective shift so simply adjust amplitude of output
		// by the factor Wet+Dry
		// also fill buffer, so make a note that the buffer contents
		// are still important to us

		buffer_zeroed = false;

		float const WD = Wet + Dry;

		do
		{
			// write to buffer position currently under write head
			buffer[writeposition] = *psamples;
			// advance write head by one position
			// write head wraps around from the end of the buffer to the beginning
			writeposition=(writeposition+1)%BUFFERLENGTH;

			// adjust the amplitude of the output signal (no pitch
			// shifting involved, so we don't even need to worry
			// about the read head)
			*psamples = (float)(WD* (*psamples));

			// advance the input pointer to the next sample
			psamples++;
		
		} while (--numsamples);

	}

	return true;
}


#pragma optimize ("", on)