/*

  The Elenzil Amplitude Modulator	1.0

  20010303 Santa Cruz California

  This machine was written primarily because Goenik's Amplitude Modulator
  introduces small but noticable artifacts in the sound.  For instance,
  try tunning the M4 in the included demo song thru a single Goenik's
  and compare to a single Elenzil. You may need to listen with headphones.

  Since i used the AM far more than any other single effect,
  i finally got tired of fixing the artifacts up in cooledit,
  and went to write my own.

  My guess at the cause of goenik's artifacts is that he calculated
  the stereo spread only once per Work() routine, rather than for each Sample.
  My version does it for each sample. This is probably overkill, but it works for me.
  

  !!!
  This machine is Crap for Efficiency.
  I don't really care, as i usually have fast computers.
  --> I am using actual Sin() calls, not lookup tables!
  There is no justification for this except my own laziness.

  Also, i built this off of an example from the standard buzz dist,
  and havn't cleaned anything up.

  If you fix it up, god bless you.

  Okay,
  that's about it.
  Compiled w/ VC6.

  Orion Elenzil
  www.elenzil.com


  */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../MachineInterface.h"
#include "../dsplib/dsplib.h"
#include <time.h>

double const SilentEnough = log(1.0 / 32768);

#define MAX_TAPS		1

CMachineParameter const paraSpeed = 
{ 
	pt_word,										// type
	"Speed",
	"Oscillation Speed",							// description
	0,												// MinValue	
	65534,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	1000											// Default
};

#define UNIT_MHZ	0
#define UNIT_MS		1
#define UNIT_TICK	2
#define UNIT_256	3


CMachineParameter const paraUnit =
{ 
	pt_byte, 										// type
	"Speed Unit",
	"Speed unit (0 = mHz, 1 = ms, 2 = tick, 3 = 256th of tick)",		
	0,												// MinValue	
	3,												// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	UNIT_MHZ										// Default
};

#define	WAVE_SIN		0
#define	WAVE_SQUARE		1
#define	WAVE_TRIANGLE	2
#define	WAVE_SAW		3
#define	WAVE_INVSAW		4
#define	WAVE_CRAZY		5


CMachineParameter const paraWave =
{ 
	pt_byte, 										// type
	"Wave Type",
	"Wave Type (0 = sin, 1 = square, 2 = triangle 3 = saw, 4 = inv. saw)",
	0,												// MinValue	
	4,												// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	WAVE_SIN,										// Default
};

CMachineParameter const paraWavePow =
{ 
	pt_byte, 										// type
	"Wave Power",
	"Wave Power (1 = 1, 2 = ^2, 3 = ^3 etc)",
	1,												// MinValue	
	13,												// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	1,												// Default
};

CMachineParameter const paraFloor =
{ 
	pt_byte, 										// type
	"Floor",
	"Floor 00 - fe",
	0,												// MinValue	
	0xfe,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0												// Default
};

CMachineParameter const paraSpread =
{ 
	pt_word, 										// type
	"Phase",
	"Phase x0000 - x0800",
	0x0000,											// MinValue	
	0x0800,											// MaxValue
	0xffff,											// NoValue
	MPF_STATE,										// Flags
	0x0800,											// Default
};

CMachineParameter const paraSlur =
{ 
	pt_byte, 										// type
	"Slur",
	"Slur x00 - xfe",
	0x00,											// MinValue	
	0xfe,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0xf2,											// Default
};

CMachineParameter const paraGain =
{ 
	pt_byte, 										// type
	"Gain",
	"Gain x00 - xfe",
	0x00,											// MinValue	
	0xf0,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0x12,											// Default
};

CMachineParameter const paraReset =
{ 
	pt_switch, 										// type
	"Reset",
	"Reset 1 = Restart Oscillator",
	0x01,											// MinValue	
	0x01,											// MaxValue
	0xff,											// NoValue
	MPF_TICK_ON_EDIT,								// Flags
	0xff,											// Default
};

CMachineParameter const *pParameters[] = 
{ 
	&paraSpeed,
	&paraUnit,
	&paraWave,
	&paraWavePow,
	&paraFloor,
	&paraSpread,
	&paraSlur,
	&paraGain,
	&paraReset,
};

#pragma pack(1)

class tvals
{
public:
	word	speed;
	byte	unit;
	byte	wave;
	byte	wavepow;
	byte	floor;
	word	spread;
	byte	slur;
	byte	gain;
	byte	reset;
};


#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	MIF_MONO_TO_STEREO,						// flags
	1,										// min tracks
	1,										// max tracks
	0,										// numGlobalParameters
	9,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Elenzil Amplitude Modulator (Debug)",	// name
#else
	"Elenzil Amplitude Modulator",
#endif
	"Modulator",							// short name
	"www.elenzil.com",						// author
	NULL
};

class CTrack
{
	public:
	double	Speed;
	int		Unit;
	int		Wave;
	int		WavePow;
	int		Floor;
	double	Spread;
	double	Slur;
	double	Gain;
	bool	Reset;

	double	GetFrequency(CMasterInfo* pMasterInfo);
};


// Return frequency in seconds.
inline	double	CTrack::GetFrequency(CMasterInfo* pMasterInfo)
	{
	switch (Unit)
		{
		default:
		case	UNIT_MHZ:
			return Speed * 0.001;
			break;
		case	UNIT_MS:
			if (Speed != 0.0)
				return 1000.0 / Speed;
			else
				return 1000.0 / 0.5;
			break;
		case	UNIT_TICK:
			if (Speed != 0.0)
				return pMasterInfo -> TicksPerSec / Speed;
			else
				return pMasterInfo -> TicksPerSec;
			break;
		case	UNIT_256:
			if (Speed != 0.0)
				return 256.0 * pMasterInfo -> TicksPerSec / Speed;
			else
				return 256.0 * pMasterInfo -> TicksPerSec;
			break;
		}
	}

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
//	virtual bool Work(float *psamples, int numsamples, int const mode);
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);

	virtual void AttributesChanged();

	virtual char const *DescribeValue(int const param, int const value);

private:
	void	InitTrack(int const i);
	void	ResetTrack(int const i);

	void	TickTrack(CTrack *pt, tvals *ptval);
	void	WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);
	int		MSToSamples(double const ms);

private:
	int		IdleCount;	// in samples
	bool	IdleMode;
	int		pos;
	int		pos2;

	long	m_Tick;
	double	m_Second;
	double	m_PrevF1;
	double	m_PrevF2;

private:
	int numTracks;
	CTrack Tracks[MAX_TAPS];

	tvals tval[MAX_TAPS];


};

DLL_EXPORTS

mi::mi()
{
	GlobalVals	= NULL;
	TrackVals	= tval;
	AttrVals	= NULL;
}

mi::~mi()
{
}

char*	CommaPrint(char* txt, int const value)
	{
	char	tmp[32];
	int		i, inum;
	int		j, k;
	sprintf(tmp, "%d", value);
	inum	=	strlen(tmp);
	j		=	inum	% 3;
	k		=	0;
	for (i = 0; i < inum; i++)
		{
		txt[k++] = tmp[i];
		j--;
		if (j == 0 && i < inum - 1)
			{
			txt[k++] = ',';
			j = 3;
			}
		}
	txt[k] = 0;

	return txt;
	}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];
	char	tmp[9];
	int		val;

	val	=	value;

	switch(param)
	{
	case 0:		// freq
		switch(Tracks[0].Unit)
			{
			default:
			case	UNIT_MS:
			case	UNIT_MHZ:
				CommaPrint(txt, val);
				sprintf(tmp, " (%.4x)", val);
				strcat(txt, tmp);
				break;
			}
		break;
	case 1:		// unit
		switch(val)
			{
			case 0: return "mHz (00)";
			case 1: return "ms (01)";
			case 2: return "tick (02)";
			case 3: return "tick/256 (03)";
			}
		break;
	case 2:		// wave
		char*	s;
		switch(val)
			{
			case WAVE_SIN		: s = "sin"			; break;
			case WAVE_SQUARE	: s = "square"		; break;
			case WAVE_TRIANGLE	: s = "triangle"	; break;
			case WAVE_SAW		: s = "saw"			; break;
			case WAVE_INVSAW	: s = "inv. saw"	; break;
			case WAVE_CRAZY		: s = "crazy"		; break;
			}
			sprintf(txt, "%s (%.2x)", s, val);
		break;
	case 3:		// wave power
		sprintf(txt, "^%d", val);
		break;
	case 4:		// floor
		sprintf(txt, "%d%c (%.2x)", val * 100 / paraFloor.MaxValue, '%', val);
		break;
	case 5:		// spread
		sprintf(txt, "%.3f (%.4x)", (double)val / (double)paraSpread.MaxValue * 2.0 - 1.0, val);
		break;
	case 6:		// slur
		sprintf(txt, "%.3f (%.2x)", (double)val / (double)paraSlur.MaxValue, val);
		break;
	case 7:		// gain
		sprintf(txt, "%.2f (%.2x)", (double)val / (double)paraGain.DefValue, val);
		break;
	case 8:		// reset
		sprintf(txt, "1 = reset oscillation");
		break;
	default:
		return NULL;
	}

	return txt;
}



void mi::Init(CMachineDataInput * const pi)
{
	numTracks = 1;

	for (int c = 0; c < MAX_TAPS; c++)
	{
		Tracks[c].Speed		= 1000.0;
		Tracks[c].Unit		= UNIT_MHZ;
		Tracks[c].Wave		= WAVE_SIN;
		Tracks[c].WavePow	= 1;
		Tracks[c].Floor		= 0;
		Tracks[c].Spread	= 1.0;
		Tracks[c].Slur		= 0.94;
		Tracks[c].Gain		= 1.0;
		Tracks[c].Reset		= false;
	}

	IdleMode	=	true;
	IdleCount	=	0;
	pos			=	0;
	pos2		=	0;
	
	m_Tick		=	0;
	m_Second	=	0.0;

	m_PrevF1	=	1.0;
	m_PrevF2	=	1.0;
}

void mi::AttributesChanged()
{
	for (int c = 0; c < numTracks; c++)
		InitTrack(c);
}


void mi::SetNumTracks(int const n)
{
	if (numTracks < n)
	{
		for (int c = numTracks; c < n; c++)
			InitTrack(c);
	}
	else if (n < numTracks)
	{
		for (int c = n; c < numTracks; c++)
			ResetTrack(c);
	
	}
	numTracks = n;
}

int mi::MSToSamples(double const ms)
{
	return (int)(pMasterInfo->SamplesPerSec * ms * (1.0 / 1000.0));
}

void mi::InitTrack(int const i)
{
}

void mi::ResetTrack(int const i)
{
}


void mi::TickTrack(CTrack *pt, tvals *ptval)
{
	if (ptval->speed	!=	paraSpeed	.NoValue)
		pt->Speed		=	(double)ptval -> speed;

	if (ptval->unit		!=	paraUnit	.NoValue)
		pt->Unit		=			ptval -> unit;

	if (ptval->wave		!=	paraWave	.NoValue)
		pt->Wave		=			ptval -> wave;

	if (ptval->wavepow	!=	paraWavePow	.NoValue)
		pt->WavePow		=			ptval -> wavepow;

	if (ptval->floor	!=	paraFloor	.NoValue)
		pt->Floor		=			ptval -> floor;

	if (ptval->spread	!=	paraSpread	.NoValue)
		pt->Spread		=	(double)ptval -> spread	/ (double)paraSpread.MaxValue * 2.0 - 1.0;

	if (ptval->slur		!=	paraSlur	.NoValue)
		pt->Slur		=	(double)ptval -> slur	/ (double)paraSlur.MaxValue;

	if (ptval->gain		!=	paraGain	.NoValue)
		pt->Gain		=	(double)ptval -> gain	/ (double)paraGain.DefValue;

	if (ptval->reset	!=	paraReset	.NoValue)
		pt->Reset		=	(bool)	ptval -> reset;
}

void mi::Tick()
{
	m_Tick++;

	for (int c = 0; c < numTracks; c++)
		TickTrack(Tracks + c, tval+c);
}



bool mi::WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode)
{
	if (mode != WM_READWRITE)
		return false;

	int	i, inum;
	int	j;
	int	p, pnum;

	inum	=	numsamples;
	j		=	0;

	int		op;
	op		=	pos;
	pos		+=	pMasterInfo ->	PosInTick - pos2;
	pos2	=	pMasterInfo ->	PosInTick;

	double	f1;
	double	f2;
	double	SecondsPerSample;
	double	floor;
	double	one_minus_floor;
	double	Spread;
	double	Slur, OneMinusSlur;
	double	Gain;

	floor				=	(double)(Tracks[0].Floor) / (double)paraFloor.MaxValue;
	one_minus_floor		=	1.0 - floor;

	SecondsPerSample	=	1.0 / (double)pMasterInfo -> SamplesPerSec;
	SecondsPerSample	*=	Tracks[0].GetFrequency(pMasterInfo);

	Spread				=	Tracks[0].Spread;

	pnum				=	Tracks[0].WavePow;

	Slur				=	Tracks[0].Slur;
	OneMinusSlur		=	1.0 - Slur;

	Gain				=	Tracks[0].Gain;

	if (Tracks[0].Reset)
		{
		m_Second		=	0;
		Tracks[0].Reset	=	false;
		}


	switch (Tracks[0].Wave)
		{
		default:
		case	WAVE_SIN:
			Spread			*=	PI * 0.5;
			for (i = 0; i < inum; i++)
				{
				f1			=	sin(m_Second * PI * 2.0 - Spread) * 0.5 + 0.5;
				f2			=	sin(m_Second * PI * 2.0 + Spread) * 0.5 + 0.5;
				m_Second	+=	SecondsPerSample;
				// This prevents the math from overflowing.
				if (m_Second > 1.0)
					m_Second -= 1.0;

				for (p = 1; p < pnum; p++)
					{
					f1		*=	f1;
					f2		*=	f2;
					}

				f1			=	f1 * one_minus_floor + floor;
				f2			=	f2 * one_minus_floor + floor;

				f1			*=	Gain;
				f2			*=	Gain;

				f1			=	OneMinusSlur * f1 + Slur * m_PrevF1;
				f2			=	OneMinusSlur * f2 + Slur * m_PrevF2;
				m_PrevF1	=	f1;
				m_PrevF2	=	f2;

				pout[j]		=	pin[i] * (float)f1;
				j++;
				pout[j]		=	pin[i] * (float)f2;
				j++;
				}
			break;

		case	WAVE_SQUARE:
			Spread			*=	0.25;
			for (i = 0; i < inum; i++)
				{
				f1			=	m_Second - Spread + 0.25;
				if (f1 < 0.0)
					f1 += 1.0;
				else if (f1 > 1.0)
					f1 -= 1.0;
				f2			=	m_Second + Spread + 0.25;
				if (f2 < 0.0)
					f2 += 1.0;
				else if (f2 > 1.0)
					f2 -= 1.0;
				if (f1 < 0.5)
					f1	=	0.0;
				else
					f1	=	1.0;
				if (f2 < 0.5)
					f2	=	0.0;
				else
					f2	=	1.0;

				m_Second	+=	SecondsPerSample;
				// This prevents the math from overflowing.
				if (m_Second > 1.0)
					m_Second -= 1.0;

				f1			=	f1 * one_minus_floor + floor;
				f2			=	f2 * one_minus_floor + floor;

				f1			*=	Gain;
				f2			*=	Gain;

				f1			=	OneMinusSlur * f1 + Slur * m_PrevF1;
				f2			=	OneMinusSlur * f2 + Slur * m_PrevF2;
				m_PrevF1	=	f1;
				m_PrevF2	=	f2;

				pout[j]		=	pin[i] * (float)f1;
				j++;
				pout[j]		=	pin[i] * (float)f2;
				j++;
				}
			break;

		case	WAVE_TRIANGLE:
			Spread			*=	0.25;
			for (i = 0; i < inum; i++)
				{
				f1			=	m_Second - Spread + 0.25;
				if (f1 < 0.0)
					f1 += 1.0;
				else if (f1 > 1.0)
					f1 -= 1.0;
				f2			=	m_Second + Spread + 0.25;
				if (f2 < 0.0)
					f2 += 1.0;
				else if (f2 > 1.0)
					f2 -= 1.0;

				f1	*=	2.0;
				f2	*=	2.0;

				if (f1 > 1.0)
					f1	=	2.0 - f1;
				if (f2 > 1.0)
					f2	=	2.0 - f2;

				m_Second	+=	SecondsPerSample;
				// This prevents the math from overflowing.
				if (m_Second > 1.0)
					m_Second -= 1.0;

				for (p = 1; p < pnum; p++)
					{
					f1		*=	f1;
					f2		*=	f2;
					}

				f1			=	f1 * one_minus_floor + floor;
				f2			=	f2 * one_minus_floor + floor;

				f1			*=	Gain;
				f2			*=	Gain;

				f1			=	OneMinusSlur * f1 + Slur * m_PrevF1;
				f2			=	OneMinusSlur * f2 + Slur * m_PrevF2;
				m_PrevF1	=	f1;
				m_PrevF2	=	f2;

				pout[j]		=	pin[i] * (float)f1;
				j++;
				pout[j]		=	pin[i] * (float)f2;
				j++;
				}
			break;

		case	WAVE_SAW:
			Spread			*=	0.25;
			for (i = 0; i < inum; i++)
				{
				f1			=	m_Second - Spread + 0.25;
				if (f1 < 0.0)
					f1 += 1.0;
				else if (f1 > 1.0)
					f1 -= 1.0;
				f2			=	m_Second + Spread + 0.25;
				if (f2 < 0.0)
					f2 += 1.0;
				else if (f2 > 1.0)
					f2 -= 1.0;

				m_Second	+=	SecondsPerSample;
				// This prevents the math from overflowing.
				if (m_Second > 1.0)
					m_Second -= 1.0;

				for (p = 1; p < pnum; p++)
					{
					f1		*=	f1;
					f2		*=	f2;
					}

				f1			=	f1 * one_minus_floor + floor;
				f2			=	f2 * one_minus_floor + floor;

				f1			*=	Gain;
				f2			*=	Gain;

				f1			=	OneMinusSlur * f1 + Slur * m_PrevF1;
				f2			=	OneMinusSlur * f2 + Slur * m_PrevF2;
				m_PrevF1	=	f1;
				m_PrevF2	=	f2;

				pout[j]		=	pin[i] * (float)f1;
				j++;
				pout[j]		=	pin[i] * (float)f2;
				j++;
				}
			break;
		case	WAVE_INVSAW:
			Spread			*=	0.25;
			for (i = 0; i < inum; i++)
				{
				f1			=	m_Second - Spread + 0.25;
				if (f1 < 0.0)
					f1 += 1.0;
				else if (f1 > 1.0)
					f1 -= 1.0;
				f2			=	m_Second + Spread + 0.25;
				if (f2 < 0.0)
					f2 += 1.0;
				else if (f2 > 1.0)
					f2 -= 1.0;

				f1	=	1.0 - f1;
				f2	=	1.0 - f2;

				m_Second	+=	SecondsPerSample;
				// This prevents the math from overflowing.
				if (m_Second > 1.0)
					m_Second -= 1.0;

				for (p = 1; p < pnum; p++)
					{
					f1		*=	f1;
					f2		*=	f2;
					}

				f1			=	f1 * one_minus_floor + floor;
				f2			=	f2 * one_minus_floor + floor;

				f1			*=	Gain;
				f2			*=	Gain;

				f1			=	OneMinusSlur * f1 + Slur * m_PrevF1;
				f2			=	OneMinusSlur * f2 + Slur * m_PrevF2;
				m_PrevF1	=	f1;
				m_PrevF2	=	f2;

				pout[j]		=	pin[i] * (float)f1;
				j++;
				pout[j]		=	pin[i] * (float)f2;
				j++;
				}
			break;
		}

	return true;
}
