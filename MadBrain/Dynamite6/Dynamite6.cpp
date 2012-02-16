// Vesion 1.2

/**********************************************************
*           Include, define, etc...                       *
**********************************************************/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include <dsplib.h>
#pragma optimize ("a", on)

#define MAX_CHANNELS 32

#define ENV_DECAY 0
#define ENV_ATTACK 1
#define ENV_SUSTAIN 2
#define ENV_ATT_SUSTAIN 3
#define ENV_LFO 4
#define ENV_ATT_LFO 5

/**********************************************************
*                       Coarse tune                       *
***********************************************************
*          Range : 1 to ff (byte) Default :80h            *
**********************************************************/
CMachineParameter const paraCoarseTune = 
{ 
	pt_byte,				// Type
	"Coarse tune",				// Name
	"Coarse tune",	// description
	1,						// MinValue	
	0xff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x80  					// Default
};

/**********************************************************
*                       Fine tune                         *
***********************************************************
*          Range : 1 to ff (byte) Default :80h            *
**********************************************************/
CMachineParameter const paraFineTune = 
{ 
	pt_byte,				// Type
	"Fine tune",				// Name
	"Fine tune",	// description
	1,						// MinValue	
	0xff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x80  					// Default
};

/**********************************************************
*                     Amplification                       *
***********************************************************
*          Range : 1 to ff (byte) Default :80h            *
**********************************************************/
CMachineParameter const paraAmplification = 
{ 
	pt_byte,				// Type
	"Amplification",				// Name
	"Amplification",	// description
	1,						// MinValue	
	0xff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x20  					// Default
};

/**********************************************************
*                      Env Attack                         *
***********************************************************
*          Range : 1 to 255 (byte) Default :80h           *
**********************************************************/
CMachineParameter const paraEnvAttack = 
{ 
	pt_byte,				// Type
	"Env Attack",				// Name
	"Env Attack",	// description
	0,						// MinValue	
	0xfe,					// MaxValue
	0xff,						// NoValue
	MPF_STATE,				// Flags
	0x4  					// Default
};

/**********************************************************
*                      Env Decay                          *
***********************************************************
*          Range : 1 to 255 (byte) Default :80h           *
**********************************************************/
CMachineParameter const paraEnvDecay = 
{ 
	pt_byte,				// Type
	"Env Decay",				// Name
	"Env Decay",	// description
	1,						// MinValue	
	0xff,					// MaxValue
	0,						// NoValue
	MPF_STATE,				// Flags
	0xff  					// Default
};


/**********************************************************
*                        Routing                          *
***********************************************************
*          Range : 1 to 255 (byte) Default :80h           *
     -1-2-3-4-5-6-

          .-3-.
     -1-2-|-4-|-
          |-5-|
          '-6-'

   _,-1-¬,-3-4-5-6-
    `-2-´

          .-3-.
   _,-1-¬_|-4-|_
    `-2-´ |-5-|
          '-6-'

   _,---1-2---¬_
    `-3-4-5-6-´

     .-1-2-.
     |--3--|
    -|--4--|-
     |--5--|
     '--6--'

    .----1----.
  --|----2----|--
    '-3-4-5-6-'


    .-1-.
    |-2-|
   _|-3-|_
    |-4-|
    |-5-|
    '-6-'

**********************************************************/
CMachineParameter const paraRouting = 
{ 
	pt_byte,				// Type
	"Routing",				// Name
	"Routing",	// description
	0,						// MinValue	
	0xa,					// MaxValue
	0xff,						// NoValue
	MPF_STATE,				// Flags
	0  					// Default
};

/**********************************************************
*                       Release                           *
***********************************************************
*          Range : 1 to 255 (byte) Default :80h           *
**********************************************************/
CMachineParameter const paraRelease = 
{ 
	pt_word,				// Type
	"Release_____",				// Name
	"Release",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe1 Length                         *
***********************************************************
*          Range : 1 to 3ff (byte) Default :100h          *
**********************************************************/
CMachineParameter const paraPipe1Length = 
{ 
	pt_word,				// Type
	"Pipe1 Length",				// Name
	"Pipe1 Length",	// description
	1,						// MinValue	
	0x3ff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xfe  					// Default
};

/**********************************************************
*                    Pipe1 Feedback                       *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe1Feedback = 
{ 
	pt_word,				// Type
	"          FBack",				// Name
	"Pipe1 Feedback",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe1 Filter                         *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe1Filter = 
{ 
	pt_word,				// Type
	"_____Filter___",				// Name
	"Pipe1 Filter",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x4000  					// Default
};

/**********************************************************
*                    Pipe2 Length                         *
***********************************************************
*          Range : 1 to 3ff (byte) Default :100h          *
**********************************************************/
CMachineParameter const paraPipe2Length = 
{ 
	pt_word,				// Type
	"Pipe2 Length",				// Name
	"Pipe2 Length",	// description
	1,						// MinValue	
	0x3ff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xff  					// Default
};

/**********************************************************
*                    Pipe2 Feedback                       *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe2Feedback = 
{ 
	pt_word,				// Type
	"          FBack",				// Name
	"Pipe2 Feedback",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe2 Filter                         *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe2Filter = 
{ 
	pt_word,				// Type
	"_____Filter___",				// Name
	"Pipe2 Filter",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x4000  					// Default
};


/**********************************************************
*                    Pipe3 Length                         *
***********************************************************
*          Range : 1 to 3ff (byte) Default :100h          *
**********************************************************/
CMachineParameter const paraPipe3Length = 
{ 
	pt_word,				// Type
	"Pipe3 Length",				// Name
	"Pipe3 Length",	// description
	1,						// MinValue	
	0x3ff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x100  					// Default
};

/**********************************************************
*                    Pipe3 Feedback                       *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe3Feedback = 
{ 
	pt_word,				// Type
	"          FBack",				// Name
	"Pipe3 Feedback",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe3 Filter                         *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe3Filter = 
{ 
	pt_word,				// Type
	"_____Filter___",				// Name
	"Pipe3 Filter",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x4000  					// Default
};

/**********************************************************
*                    Pipe4 Length                         *
***********************************************************
*          Range : 1 to 3ff (byte) Default :100h          *
**********************************************************/
CMachineParameter const paraPipe4Length = 
{ 
	pt_word,				// Type
	"Pipe4 Length",				// Name
	"Pipe4 Length",	// description
	1,						// MinValue	
	0x3ff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x101  					// Default
};

/**********************************************************
*                    Pipe4 Feedback                       *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe4Feedback = 
{ 
	pt_word,				// Type
	"          FBack",				// Name
	"Pipe4 Feedback",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe4 Filter                         *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe4Filter = 
{ 
	pt_word,				// Type
	"_____Filter___",				// Name
	"Pipe4 Filter",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x4000  					// Default
};



/**********************************************************
*                    Pipe5 Length                         *
***********************************************************
*          Range : 1 to 3ff (byte) Default :100h          *
**********************************************************/
CMachineParameter const paraPipe5Length = 
{ 
	pt_word,				// Type
	"Pipe5 Length",				// Name
	"Pipe5 Length",	// description
	1,						// MinValue	
	0x3ff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x102  					// Default
};

/**********************************************************
*                    Pipe5 Feedback                       *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe5Feedback = 
{ 
	pt_word,				// Type
	"          FBack",				// Name
	"Pipe5 Feedback",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe5 Filter                         *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe5Filter = 
{ 
	pt_word,				// Type
	"_____Filter___",				// Name
	"Pipe5 Filter",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x4000  					// Default
};



/**********************************************************
*                    Pipe6 Length                         *
***********************************************************
*          Range : 1 to 3ff (byte) Default :100h          *
**********************************************************/
CMachineParameter const paraPipe6Length = 
{ 
	pt_word,				// Type
	"Pipe6 Length",				// Name
	"Pipe6 Length",	// description
	1,						// MinValue	
	0x3ff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x100  					// Default
};

/**********************************************************
*                    Pipe6 Feedback                       *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe6Feedback = 
{ 
	pt_word,				// Type
	"          FBack",				// Name
	"Pipe6 Feedback",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0xf000  					// Default
};

/**********************************************************
*                    Pipe6 Filter                         *
***********************************************************
*          Range : 1 to ffff (byte) Default :f000h        *
**********************************************************/
CMachineParameter const paraPipe6Filter = 
{ 
	pt_word,				// Type
	"_____Filter___",				// Name
	"Pipe6 Filter",	// description
	1,						// MinValue	
	0xffff,					// MaxValue
	0x0,						// NoValue
	MPF_STATE,				// Flags
	0x4000  					// Default
};



/**********************************************************
*                        Note                             *
***********************************************************
*                      Standard                           *
*   1/2 tones from A-4: (x/16)*12+(x%16)-70+detune-128    *
*             pitch in hz: 440*2^(fromA4/12)              *
*             Waveguide length: sr/hz                     *
**********************************************************/
CMachineParameter const paraNote = 
{ 
	pt_note,				// Type
	"Note",				// Name
	"Note",	// description
	NOTE_MIN,						// MinValue	
	NOTE_MAX,					// MaxValue
	NOTE_NO,						// NoValue
	0,				// Flags
	0x80  					// Default
};

/**********************************************************
*                     Volume                              *
***********************************************************
* 0 = 0
* 80h = 100%
* FEh = ~200%
**********************************************************/
CMachineParameter const paraVolume = 
{ 
	pt_byte,				// Type
	"Volume",				// Name
	"Volume, 80h = 100%, FEh = ~200%",		// description
	0,						// MinValue	
	0xfe,					// MaxValue
	0xff,						// NoValue
	MPF_STATE,				// Flags
	0x80  					// Default
};

// Parameter Declaration
CMachineParameter const *pParameters[] = { 
	// global
	&paraCoarseTune,
	&paraFineTune,
	&paraAmplification,

	&paraEnvAttack,
	&paraEnvDecay,
	&paraRouting,
	&paraRelease,

	&paraPipe1Length,
	&paraPipe1Feedback,
	&paraPipe1Filter,

	&paraPipe2Length,
	&paraPipe2Feedback,
	&paraPipe2Filter,

	&paraPipe3Length,
	&paraPipe3Feedback,
	&paraPipe3Filter,

	&paraPipe4Length,
	&paraPipe4Feedback,
	&paraPipe4Filter,

	&paraPipe5Length,
	&paraPipe5Feedback,
	&paraPipe5Filter,

	&paraPipe6Length,
	&paraPipe6Feedback,
	&paraPipe6Filter,

	// Track
	&paraNote,
	&paraVolume

};

// Parameter structures
#pragma pack(1)

class gvals
{
public:
	unsigned char coarse_tune;
	unsigned char fine_tune;
	unsigned char amplification;

	unsigned char env_attack;
	unsigned char env_decay;
	unsigned char routing;
	unsigned short release;

	unsigned short pipe1_length;
	unsigned short pipe1_feedback;
	unsigned short pipe1_filter;

	unsigned short pipe2_length;
	unsigned short pipe2_feedback;
	unsigned short pipe2_filter;

	unsigned short pipe3_length;
	unsigned short pipe3_feedback;
	unsigned short pipe3_filter;

	unsigned short pipe4_length;
	unsigned short pipe4_feedback;
	unsigned short pipe4_filter;

	unsigned short pipe5_length;
	unsigned short pipe5_feedback;
	unsigned short pipe5_filter;

	unsigned short pipe6_length;
	unsigned short pipe6_feedback;
	unsigned short pipe6_filter;
};

class tvals
{
public:
	unsigned char note;
	unsigned char volume;
};

class pvals
{
public:
	unsigned short length;
	unsigned short feedback;
	unsigned short filter;
};

#pragma pack()

/**********************************************************
*           Machine Info                                  *
**********************************************************/

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,			// type
	MI_VERSION,			// buzz version
	0,					// flags
	1,					// min tracks
	MAX_CHANNELS,					// max tracks
	25,					// numGlobalParameters
	2,					// numTrackParameters
	pParameters,		// Parameters
	0, 					// numAttributes?
	NULL,				// pAttributes?
#ifdef BETA
	"MadBrain's Dynamite 6 (Beta) version 1.2",// name
#else
	"MadBrain's Dynamite 6 1.2",
#endif
	"Dynamite6",		// short name
	"Hubert Lamontagne (aka MadBrain)",// author
	NULL				// menu commands?
};


CMICallbacks *mrbox;
char mrtext[100];



class env
{

public:
	inline void work();
	void on();
	void off();
	void init();

	// State
public:

	struct
	{
		int attack;
		int decay;
	}p;
	struct
	{
		int time;
		int val;
		int dir; // -1 = down, 1 = up, 0 = stuck
		int key;
	}s;
};

class pipe
{

public:
	void tick();
	inline float generate(float input);
	inline float generate_rotational(float input,pipe *p2);
	void init();
	void stop();
	// State
public:

	pvals pv;

	int pos;
	int length;

	short osc;

	float lowpass_pos;
	float lowpass_coef;
	float lowpass_anti_coef;

	float feedback;
	float feedback_cache;

	float sin_cache;
	float cos_cache;

	float data[1024];

};


class channel
{

public:
	void tick(int sample_rate);
	inline float generate();
	inline float advance();
	void init();
	void stop();

	// State
public:

	gvals gv;
	tvals tv;

	env input_env;
	int noise_state;
	float noise_amp;

	float freq_sub;
	float pos_sub;
	unsigned char freq_maj;
	float interpol, interpol2;
	
	float coarse_tune;
	float fine_tune;
	float note_tune;
	
	int routing;
	pipe pipes[6];
	float release;

	float final_amp;

	float volume_detector;
};


class mi : public CMachineInterface
{
public:
	mi();

	virtual void Init();
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

	virtual void SetNumTracks(int const n);
	virtual void Stop();
	virtual char const *DescribeValue(int const param, int const value);

public:
	gvals gval;	// Store your global parameters here
	tvals tval[MAX_CHANNELS];
	channel channels[MAX_CHANNELS];
	int active_channels;

};




 DLL_EXPORTS;	// Needed for DLLs


void env::init()
{
	s.time=0;
	s.val=0;
	s.dir=0;
	s.key=0;
}

void env::on()
{
	s.key=1;
	s.time=0;
	s.dir = 1;
	s.val = 0;

	if(p.attack == 0)
	{
		s.val=255;
	}
}


void env::off()
{
	s.key=0;
	if(s.val > 0)
		s.dir = -1;
	else
		s.dir = 0;
}

inline void env::work()
{

	if(!s.time)
	{
		if(s.dir==1)
		{
			if(p.attack)
				s.time=p.attack;
			else
				s.time=1;
		}
		else if(s.dir==-1)
			s.time=p.decay;
		else
			s.time=255;

		if(s.dir)
		{
			if(s.dir==1)
			{
				if (s.val>=255)
				{
					if(p.decay!=255)
						s.dir= -1;
					else
						s.dir= 0;
				}
			}
					
			else if (s.val<=0)
			{
				s.val=0;
				s.dir=0;
			}
		}	
		
		s.val+=s.dir;
	}
	s.time--;
}

void pipe::init()
{
	int i;

	pos=0;
	length=256;
	for (i=0;i<1024;i++) data[i]=0;

	feedback = 0.75;
	feedback_cache = 0.75;

	lowpass_pos=0;
	lowpass_anti_coef=0.1;
	lowpass_coef=0.9;
}

void channel::init()
{
	int i;

	input_env.init();

	freq_sub=0;
	freq_maj=0;
	pos_sub=0;
	interpol = interpol2 = 0;
	note_tune = 1;

	noise_state=666 + rand();
	for (i=0;i<6;i++)
		pipes[i].init();

	volume_detector = 0;
}

void pipe::stop()
{
	int i;
	for(i=0;i<1024;i++) data[i]=0;
	
}

void channel::stop()
{
	int i;
	input_env.init();
	input_env.s.val=0;
	input_env.s.dir=0;
	input_env.s.time=0;
	input_env.s.key=0;
	for(i=0;i<6;i++) pipes[i].stop();

	volume_detector = 0;
}


inline float pipe::generate_rotational(float input, pipe *p2)
{
	float r, i, t;

	r = input + data[pos] * feedback_cache;
	r = lowpass_pos = r*lowpass_anti_coef + lowpass_pos*lowpass_coef;
	i = p2->data[p2->pos] * feedback_cache;
	i = p2->lowpass_pos = i*p2->lowpass_anti_coef + p2->lowpass_pos*p2->lowpass_coef;

	t = p2->cos_cache*r - p2->sin_cache*i;
	i = p2->cos_cache*i + p2->sin_cache*r;
	r = t;

	data[pos++] = r;
	if(pos>=length)
		pos=0;

	p2->data[(p2->pos)++] = i;
	if((p2->pos) >= (p2->length))
		p2->pos=0;

	return (r);
}

inline float pipe::generate(float input)
{
	float soap = input;

	soap += data[pos] * feedback_cache;
	soap = lowpass_pos = soap*lowpass_anti_coef + lowpass_pos*lowpass_coef;
	data[pos++] = soap;

	if(pos>=length)
		pos=0;


	return (soap);
}

inline float channel::advance()
{
	float soap,soap2,soap3;

	soap =  noise_state = ((noise_state * 1103515245 + 12345) & 0x7fffffff) - 0x40000000;
	soap *= noise_amp;
	soap *= input_env.s.val;
	soap += 0.0000000000000001;

	if (routing == 10)
	{
		soap = pipes[0].generate_rotational(soap,&pipes[1]);
		soap = pipes[2].generate_rotational(soap,&pipes[3]);
		soap = pipes[4].generate_rotational(soap,&pipes[5]);

		return(soap);
	}

	if (routing & 8)
	{
		soap2 = pipes[0].generate_rotational(soap,&pipes[1]);
	}
	else
	{
		if (routing & 2)
			soap2 = pipes[0].generate(soap) + pipes[1].generate(soap);
		else
		soap2 = pipes[1].generate(pipes[0].generate(soap));
	}

	if (!(routing & 4))
		soap = soap2;

	if (routing & 1)
		soap3 = pipes[2].generate(soap) + pipes[3].generate(soap)
		     + pipes[4].generate(soap) + pipes[5].generate(soap);
	else
		soap3 = pipes[2].generate ( pipes[3].generate
                     ( pipes[4].generate ( pipes[5].generate(soap))));

	if (routing & 4)
		soap3 += soap2;
		
	return(soap3);
}

inline float channel::generate()
{
	input_env.work();

	for(int i=0;i<freq_maj;i++)
	{
		interpol2 = interpol;
		interpol = advance();
	}
	pos_sub+=freq_sub;

	if(pos_sub>=1)
	{
		pos_sub-=1;
		interpol2 = interpol;
		interpol = advance();
	}

	volume_detector += fabsf(interpol);
	volume_detector *= 0.999;

	return ((interpol-interpol2)*pos_sub + interpol2);
}

void pipe::tick()
{
	int i,old;
	char txt[128];

	if (pv.length != 0)
	{
		old = length;
		length = pv.length;
		if (old<length)
			for(i=old;i<length;i++)
				data[i] = data[old-1];
	}

	if (pv.feedback != 0)
	{
		if (feedback == feedback_cache)
			feedback = feedback_cache = (pv.feedback/32768.0)-1.0;
		else
			feedback = (pv.feedback/32768.0)-1.0;

		sin_cache = sin(3.14159268*(pv.feedback-32768)/32768.0);
		cos_cache = cos(3.14159268*(pv.feedback-32768)/32768.0);
	}

	if (pv.filter != 0)
	{
		lowpass_coef = pv.filter/65536.0;
		lowpass_anti_coef = 1 - lowpass_coef;
	}
}


void channel::tick(int sample_rate)
{
	int i,j;
	char *hack = (char *)(&gv.pipe1_length);
	char *hack2;

	if (gv.coarse_tune != 0)
		coarse_tune = pow(2,(gv.coarse_tune-128.0)/12.0);
	if (gv.fine_tune != 0)
		fine_tune = pow(2,((gv.fine_tune/128.0)-1.0)/12.0);
	if (gv.amplification != 0)
		final_amp = pow(2,(gv.amplification-128.0)/8.0);

	if (gv.env_attack != paraEnvAttack.NoValue)
		input_env.p.attack = gv.env_attack;
	if (gv.env_decay != paraEnvDecay.NoValue)
	{
		if(input_env.p.decay == 255 && gv.env_decay != 255)
			for(i=0;i<6;i++)
				if(input_env.s.dir == 0)
					input_env.s.dir = -1;
		input_env.p.decay = gv.env_decay;
	}
	if (gv.routing != paraRouting.NoValue)
		routing = gv.routing;
	if (gv.release != 0)
		release = (gv.release/32768.0)-1.0;

	for(i=0;i<6;i++)
	{
		hack2 = (char *)(&pipes[i].pv);
		for(j=0;j<6;j++)			
			hack2[j] = hack[j + i*6];
		pipes[i].tick();
	}

	if (tv.note != NOTE_NO)
	if (tv.note != NOTE_OFF)
	{
		input_env.on();
		note_tune = pow(2,(tv.note>>4)-5+((tv.note%16)-10.0)/12.0 )*256.0*440.0/sample_rate;
		for(i=0;i<6;i++)
			pipes[i].feedback_cache = pipes[i].feedback;
	}

	freq_sub = fine_tune * coarse_tune * note_tune;

	if (freq_sub >= 40)
		freq_sub = 40;

	freq_maj = freq_sub;
	freq_sub -= freq_maj;
	
	if (tv.note == NOTE_OFF)
	{
		input_env.off();
		for(i=0;i<6;i++)
			pipes[i].feedback_cache *= release;
	}

	if (tv.volume != 0xff)
		noise_amp = tv.volume/65536.0/256.0/128.0;

}

/**********************************************************
*           mi:mi - constructor                           *
**********************************************************/

mi::mi()
{
	int i;
	GlobalVals = &gval;
	TrackVals = tval;
	AttrVals   = NULL;
	for (i=0;i<MAX_CHANNELS;i++)
	{
		channels[i].init();
	}
	active_channels = 1;
	mrbox= pCB;

}

void mi::Init()
{
	int i;
	for (i=0;i<active_channels;i++)
		channels[i].init();
}

void mi::Tick()
{
	int i;

	for (i=0;i<active_channels;i++)
	{
		channels[i].gv                    = gval;
		channels[i].tv                    = tval[i];
		channels[i].tick(pMasterInfo->SamplesPerSec);
	}
}




bool mi::Work(float *psamples, int numsamples, int const)
{
	int i,j,k;
	float sum;
	int flag=0;
	int active[MAX_CHANNELS];

	for (i=0;i<active_channels;i++)
		if(channels[i].input_env.s.val||channels[i].input_env.s.dir)
		{
			flag=1;
			active[i] = 1;
		}
		else
		{
			if(channels[i].volume_detector * channels[i].final_amp > 0.5)
			{
				flag=1;
				active[i] = 1;
			}
			else
				active[i] = 0;
		}

	if(!flag)	return false;

	for(j=0;j<numsamples;j++)
		psamples[j] = 0;



	for (i=0;i<active_channels;i++)
	{
		if(active[i])
		{
			for(j=0;j<numsamples;j++)
				psamples[j] += channels[i].generate();			
		}
	}


	for(j=0;j<numsamples;j++)
		psamples[j] *= channels[0].final_amp;
	

	return true;
}




char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[32];
	switch(param){	

	case 0:		// Coarse tune

		if(value >= 128)
			sprintf(txt,"+%d",value-128); 
		else
			sprintf(txt," %d",value-128); 
		return txt;

	case 1:		// Fine tune
		if(value >= 128)
			sprintf(txt,"+%2.3f",(double)(value-128)/128.0); 
		else
			sprintf(txt," %2.3f",(double)(value-128)/128.0); 
		return txt;



	case 2:		// Amplification
		if(value >= 128)
			sprintf(txt,"+%2.2f db",(double)(value-128)/8.0*6.0); 
		else
			sprintf(txt," %2.2f db",(double)(value-128)/8.0*6.0); 
		return txt;

	case 4:		// Decay
		if(value==255)
			sprintf(txt,"Sustain"); 
		else
			sprintf(txt,"%d",value); 
		return txt;
	
	case 3:		// Attack
	case 7:		// P1 Length
	case 10:	// P2 Length
	case 13:	// P3 Length
	case 16:	// P4 Length
	case 19:	// P5 Length
	case 22:	// P6 Length
		sprintf(txt,"%d",value); 
		return txt;

	case 5:		// Routing
		switch(value)
		{
		case 0:
			sprintf(txt,"123456"); 
			break;
		case 1:
			sprintf(txt,"12(3+4+5+6)"); 
			break;
		case 2:
			sprintf(txt,"(1+2)3456"); 
			break;
		case 3:
			sprintf(txt,"1+2->3+4+5+6"); 
			break;
		case 4:
			sprintf(txt,"12+3456"); 
			break;
		case 5:
			sprintf(txt,"12+3+4+5+6"); 
			break;
		case 6:
			sprintf(txt,"1+2+3456"); 
			break;
		case 7:
			sprintf(txt,"1+2+3+4+5+6"); 
			break;
		case 8:
			sprintf(txt,"(1*2)3456"); 
			break;
		case 9:
			sprintf(txt,"1*2->3+4+5+6"); 
			break;
		case 10:
			sprintf(txt,"1*2->3*4->5*6"); 
			break;
/*		case 8:
			sprintf(txt,"%d",channels[0].pipes[0].length); 
			break;
		case 9:
			sprintf(txt,"%f",(double)channels[0].pipes[0].feedback_cache); 
			break;
		case 10:
			sprintf(txt,"%f",(double)channels[0].pipes[0].lowpass_coef); 
			break; */
		default:
			sprintf(txt,"Bug!"); 
			break;
		}
		return txt;

	case 6:		// Release
	case 8:		// P1 Feedback
	case 11:	// P2 Feedback
	case 14:	// P3 Feedback
	case 17:	// P4 Feedback
	case 20:	// P5 Feedback
	case 23:	// P6 Feedback
		if(value >= 32768)
			sprintf(txt,"+%2.3f%%",(double)(value-32768)/32768.0*100.0); 
		else
			sprintf(txt," %2.3f%%",(double)(value-32768)/32768.0*100.0); 
		return txt;

	case 9:		// P1 Filter
	case 12:	// P2 Filter
	case 15:	// P3 Filter
	case 18:	// P4 Filter
	case 21:	// P5 Filter
	case 24:	// P6 Filter
		sprintf(txt,"%2.3f%%",(double)(value)/65536.0*100.0); 
		return txt;

	case 25: // Note
		sprintf(txt,"Note"); 
		return txt;

	case 26: // Volume
		sprintf(txt,"%d",value); 
		return txt;

	default:
		sprintf(txt,"Bug!"); 
		break;
	}
}

void mi::Stop()
{
	int i;
	for (i=0;i<MAX_CHANNELS;i++)
		channels[i].stop();
}


void mi::SetNumTracks(int const n)
{
	int i;

	if(n<active_channels)
		for(i=n;i<active_channels;i++)
			channels[i].stop();

	else
		for(i=active_channels;i<n;i++)
		{
			channels[i].init();
			channels[i]=channels[0];
			channels[i].stop();
		}
	active_channels = n;
}
