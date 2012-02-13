/*

  The Elenzil Frequency Bomb 1.0

  20010529 Santa Cruz California


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

#define UNIT_MHZ	0
#define UNIT_MS		1
#define UNIT_TICK	2
#define UNIT_256	3
#define UNIT_SECOND	4

#define OWF_SINE		0
#define OWF_SAWTOOTH	1
#define OWF_PULSE		2		// square 
#define OWF_TRIANGLE	3
#define OWF_NOISE		4



CMachineParameter const paraFreq = 
{ 
	pt_word,										// type
	"Freq x 100",
	"Target Frequency (Hz x 100)",					// description
	1,												// MinValue	
	65534,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	5000											// Default
};

CMachineParameter const paraLFOPeriod = 
{ 
	pt_word,										// type
	"LFO s/100",
	"LFO Period (Seconds / 100)",					// description
	1,												// MinValue	
	65534,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	1000											// Default
};

CMachineParameter const paraLFOAmt = 
{ 
	pt_word,										// type
	"LFO Amtx1000",
	"LFO Amount (Hz x 1000)",						// description
	1,												// MinValue	
	65534,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	0												// Default
};

CMachineParameter const paraFreqAttack = 
{ 
	pt_word,										// type
	"Freq Attack",
	"Frequency Attack",								// description
	0,												// MinValue	
	65534,											// MaxValue
	65535,											// NoValue
	MPF_STATE,										// Flags
	10												// Default
};

CMachineParameter const paraFreqAttackUnit =
{ 
	pt_byte, 										// type
	"Freq Attack Unit",
	"Freq Attack Unit (1 = ms, 2 = tick, 3 = 256th of tick, 4 = Seconds)",		
	1,												// MinValue	
	4,												// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	UNIT_SECOND										// Default
};

CMachineParameter const paraVolume =
{ 
	pt_byte, 										// type
	"Volume",
	"Gain x00 - xfe",
	0x00,											// MinValue	
	0xf0,											// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	0x80,											// Default
};

CMachineParameter const paraWave =
{ 
	pt_byte, 										// type
	"Wave",
	"Wave (0 = sine, 1 = saw, 2 = square 3 = triangle, 4 = noise)",
	0,												// MinValue	
	4,												// MaxValue
	0xff,											// NoValue
	MPF_STATE,										// Flags
	OWF_SINE,										// Default
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

CMachineParameter const *pParameters[] = 
{ 
	&paraFreq,
	&paraLFOPeriod,
	&paraLFOAmt,
//	&paraFreqAttack,
//	&paraFreqAttackUnit,
//	&paraVolume,
	&paraWave,
//	&paraWavePow,
//	&paraSlur,
};


#pragma pack(1)

class tvals
{
public:
	word	m_Freq;
	word	m_LFOPeriod;
	word	m_LFOAmt;
	byte	m_Wave;
	word	m_FreqAttack;
	byte	m_FreqAttackUnit;
	byte	m_Volume;
	byte	m_WavePow;
	byte	m_Slur;
};


#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,							// type
	MI_VERSION,
	0,										// flags
	1,										// min tracks
	1,										// max tracks
	0,										// numGlobalParameters
	4,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"Elenzil Frequency Bomb (Debug)",	// name
#else
	"Elenzil Frequency Bomb",
#endif
	"FreqBomb",								// short name
	"www.elenzil.com",						// author
	NULL
};

class CTrack
{
	public:

	double	m_Freq;
	double	m_LFOPeriod;
	double	m_LFOAmt;
	double	m_FreqAttack;
	int		m_FreqAttackUnit;
	int		m_Volume;
	byte	m_Wave;
	int		m_WavePow;
	double	m_Slur;
		
	double	GetFreqAttack(CMasterInfo* pMasterInfo);
};


// Return attack in seconds.
inline	double	CTrack::GetFreqAttack(CMasterInfo* pMasterInfo)
	{
	double	f;

	f	=	m_FreqAttack;

	switch (m_FreqAttackUnit)
		{
		default:
			return	f;
			break;

		case	UNIT_MS:
			f	*=	0.001;
		case	UNIT_SECOND:
			return	f;
			break;

		case	UNIT_256:
			f	/=	256.0;
		case	UNIT_TICK:
			if (pMasterInfo -> TicksPerSec != 0.0)
				return f / pMasterInfo -> TicksPerSec;
			else
				return f;
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
	virtual bool Work(float *psamples, int numsamples, int const mode);

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
	double	m_Clock1;
	double	m_Clock2;

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
	case 0:		// m_Freq
		CommaPrint(txt, val);
		sprintf(tmp, " (%.4x)", val);
		strcat(txt, tmp);
		break;
	case 1:		// m_LFOPeriod
		CommaPrint(txt, val);
		sprintf(tmp, " (%.4x)", val);
		strcat(txt, tmp);
		break;
	case 2:		// m_LFOAmt
		CommaPrint(txt, val);
		sprintf(tmp, " (%.4x)", val);
		strcat(txt, tmp);
		break;
	case 3:		// m_Wave
		char*	s;
		switch(val)
			{
			case OWF_SINE		: s = "sine"		; break;
			case OWF_PULSE		: s = "square"		; break;
			case OWF_TRIANGLE	: s = "triangle"	; break;
			case OWF_SAWTOOTH	: s = "saw"			; break;
			case OWF_NOISE		: s = "noise"		; break;
			}
			sprintf(txt, "%s (%.2x)", s, val);
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
		Tracks[c].m_Freq			= (float)paraFreq		.DefValue;
		Tracks[c].m_LFOPeriod		= (float)paraLFOPeriod	.DefValue;
		Tracks[c].m_LFOAmt			= (float)paraLFOAmt		.DefValue;
		Tracks[c].m_FreqAttack		= 10.0;
		Tracks[c].m_FreqAttackUnit	= UNIT_SECOND;
		Tracks[c].m_Volume			= 0x80;
		Tracks[c].m_Wave			= paraWave				.DefValue;
		Tracks[c].m_WavePow			= 1;
		Tracks[c].m_Slur			= 0.94;
		}

	IdleMode	=	true;
	IdleCount	=	0;
	pos			=	0;
	pos2		=	0;
	
	m_Tick		=	0;
	m_Second	=	0.0;
	m_Clock1	=	0.0;
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
	if (ptval	-> m_Freq			!=	paraFreq.NoValue)
		pt		-> m_Freq			=	(double)ptval -> m_Freq;

	if (ptval	-> m_LFOPeriod		!=	paraLFOPeriod.NoValue)
		pt		-> m_LFOPeriod		=	(double)ptval -> m_LFOPeriod;

	if (ptval	-> m_LFOAmt			!=	paraLFOAmt.NoValue)
		pt		-> m_LFOAmt			=	(double)ptval -> m_LFOAmt;

	if (ptval	-> m_Wave			!=	paraWave.NoValue)
		pt		-> m_Wave			=	ptval -> m_Wave;

	if (ptval	-> m_FreqAttack		!=	paraFreqAttack.NoValue)
		pt		-> m_FreqAttack		=	(double)ptval -> m_FreqAttack;

	if (ptval	-> m_FreqAttackUnit	!=	paraFreqAttackUnit.NoValue)
		pt		-> m_FreqAttackUnit	=	ptval -> m_FreqAttackUnit;

	if (ptval	-> m_Volume			!=	paraVolume.NoValue)
		pt		-> m_Volume			=	ptval -> m_Volume;

	if (ptval	-> m_WavePow		!=	paraWavePow.NoValue)
		pt		-> m_WavePow		=	ptval -> m_WavePow;

	if (ptval	-> m_Slur			!=	paraSlur.NoValue)
		pt		-> m_Slur			=	(double)ptval -> m_Slur;
}

void mi::Tick()
{
	m_Tick++;

	for (int c = 0; c < numTracks; c++)
		TickTrack(Tracks + c, tval+c);
}



bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (mode != WM_WRITE)
		return false;

	int				i, inum;
	double			f1;
	double			SecondsPerSample;
	double			basefreq;
	double			lfofreq;
	double			lfoval;
	double			lfoamt;
	double			dc1;
	const short*	tableBase;
	const short*	tableLFO;

	tableBase	=	pCB -> GetOscillatorTable((int)(Tracks[0].m_Wave));
	
	tableBase	+=	GetOscTblOffset(0);

	tableLFO	=	pCB -> GetOscillatorTable(OWF_SINE);
	tableLFO	+=	GetOscTblOffset(0);

	SecondsPerSample	=	1.0 / (double)pMasterInfo -> SamplesPerSec;

	inum		=	numsamples;

	lfofreq		=	100.0 / (Tracks[0].m_LFOPeriod);
	basefreq	=	Tracks[0].m_Freq	* .01;
	lfoamt		=	Tracks[0].m_LFOAmt	* .001  / 32768.0f;

	// seems good on my 'puter,
	// but if lfofreq & numsamples are too high, this will break down.
	lfoval		=	(double)(tableLFO[(int)(m_Clock2 * 2048.0) % 2048]) * lfoamt;
	m_Clock2	+=	SecondsPerSample * (double)numsamples * lfofreq;

	dc1			=	SecondsPerSample * (basefreq + lfoval);

	for (i = 0; i < inum; i++)
		{
		f1			=	(double)(tableBase[(int)(m_Clock1 * 2048.0) % 2048]);
		psamples[i]	=	(float)f1;
		m_Clock1	+=	dc1;
		}

	// take care of roll-over
	while (m_Clock1 > 10000.0f)
		m_Clock1 -=	  10000.0f;
	while (m_Clock2 > 10000.0f)
		m_Clock2 -=	  10000.0f;


	return true;
}
