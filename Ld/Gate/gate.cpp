//20:08 < ld0d> no copyright, no responsibility, no restrictions
//20:08 < ld0d> but you can gpl them once you're done with them

#include<windef.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../mdk.h"
#include "/buzzdevel/auxbus/auxbus.h"

CMachineParameter const paraBypass =
{
        pt_switch,                                                                                // type
        "Bypass",
        "Bypass",                                                                  // description
        SWITCH_OFF,                                                                                              // Min
        SWITCH_ON,                                                                                    // Max
        SWITCH_NO,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        SWITCH_OFF                                                                                               // default
};

CMachineParameter const paraLookahead =
{
        pt_byte,                                                                                // type
        "Look-ahead",
        "Look-ahead length",                                                                  // description
        0,                                                                                              // Min
        200,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        10                                                                                               // default
};

CMachineParameter const paraHold =
{
        pt_byte,                                                                                // type
        "Hold",
        "Envelope Hold length",                                                                  // description
        1,                                                                                              // Min
        250,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        10                                                                                               // default
};

CMachineParameter const paraAttack =
{
        pt_byte,                                                                                // type
        "Attack",
        "Envelope Attack length",                                                                  // description
        1,                                                                                              // Min
        250,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        2                                                                                               // default
};

CMachineParameter const paraRelease =
{
        pt_byte,                                                                                // type
        "Release",
        "Envelope Release length",                                                                  // description
        1,                                                                                              // Min
        250,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        10                                                                                               // default
};

CMachineParameter const paraThreshold =
{
        pt_byte,                                                                                // type
        "Threshold",
        "Threshold level",                                                                  // description
        0,                                                                                              // Min
        200,                                                                                    // Max
        0xff,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        100                                                                                               // default
};

CMachineParameter const paraInvert =
{
        pt_switch,                                                                                // type
        "Invert",
        "Invert gate",                                                                  // description
        SWITCH_OFF,                                                                                              // Min
        SWITCH_ON,                                                                                    // Max
        SWITCH_NO,                                                                                   // NoValue
        MPF_STATE,                                                                              // Flags
        SWITCH_OFF                                                                                               // default
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraBypass,
	&paraThreshold,
	&paraLookahead,
	&paraAttack,
	&paraHold,
	&paraRelease,
	&paraInvert
};

#pragma pack(1)		

class gvals
{
public:
	byte bypass;
	byte threshold;
	byte lookahead;
	byte attack;
	byte hold;
	byte release;
	byte invert;
};

CMachineAttribute const attrQuietLevel =
{
	"Quiet level (-dB)",
	6,
	200,
	96
};

CMachineAttribute const attrSeparateStereo =
{
	"Separate stereo",
	0,
	1,
	0
};

CMachineAttribute const *pAttributes[] =
{
	&attrQuietLevel,
	&attrSeparateStereo,
};

class avals
{
public:
	int quiet;
	int separatestereo;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,	
	MIF_DOES_INPUT_MIXING,										// flags
	0,										// min tracks
	0,										// max tracks
	7,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	2,
	pAttributes,
#ifdef _DEBUG
   "ld gate (Debug build)",			// name
#else
   "ld gate",						// name
#endif
	"gate",								// short name
	"Lauri Koponen",						// author
	"Set Auxbus Channel...\nDisconnect Auxbus\nAbout"
};

class miex : public CMDKMachineInterfaceEx
{

};

class mi : public CMDKMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void MDKInit(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);

	bool WorkAuxbus(float *psamples, int numsamples, int const m);
	bool WorkStereoAuxbus(float *psamples, int numsamples, int const m);

	virtual char const *mi::DescribeValue(int const param, int const value);

	void DisconnectAux();

	virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
	virtual void OutputModeChanged(bool stereo) {
		memset(delayline, 0, 4096 * 4);
		memset(auxdelayline, 0, 4096 * 4);
		delaypos = 0;
		mode = 3;
		mode2 = 3;
	}

	virtual void MDKSave(CMachineDataOutput * const po) {
        po->Write((byte)1);
        po->Write(Channel);
	}

	virtual void SetNumTracks(int const n) {
	}

	virtual void AttributesChanged() {
		quiet = (float)pow(10.0, (double)-aval.quiet / 20.0);

		attackmul = exp(-log(quiet) / (double)attacktime);
		releasemul = exp(log(quiet) / (double)releasetime);

		stereoseparated = (aval.separatestereo == 1);
	}

	virtual void Command(int const i);

private:

	miex ex;

private:

	int Channel;
	float data[256];

	gvals gval;
	avals aval;

	float level;
	float level2;

	int attacktime;
	int holdtime;
	int releasetime;

	int mode;
	int time;
	int mode2;
	int time2;

	float curattackmul;
	float curattackmul2;
	float attackmul;
	float releasemul;
	float gain;

	float threshold;

	float auxdelayline[4096];
	float delayline[4096];
	int delaypos;
	int delaylen;

	float quiet;

	bool bypass;

	int beenidle;

	bool stereoseparated;

	bool invert;

	CMachine *me;
};

DLL_EXPORTS

void mi::DisconnectAux()
{
	Channel = -1;
	memset(auxdelayline, 0, 4096 * 4);
}

void cb(void *user)
{
	mi *pmi = (mi *)user;
	pmi->DisconnectAux();
}

void mi::Command(int const i) {
	if(i == 0)
		AB_ShowEditor(NULL, &Channel, MacInfo.ShortName, cb, this);
	else if(i == 1) {
		AB_Disconnect(this);
		Channel = -1;
		memset(auxdelayline, 0, 4096 * 4);
	} else
		MessageBox(NULL, "ld gate v0.1\n(C) Lauri Koponen 2002\nld0d@kolumbus.fi\n\nHint: you will probably want to set the\nlook-ahead time to be the same as attack time.\n\n\nSignal is delayed by the look-ahead time.\n12ms look-ahead = 12ms delay", "About ld gate", MB_OK | MB_ICONINFORMATION);
}

mi::mi()
{
	GlobalVals = &gval;
	TrackVals = 0;
	AttrVals = (int *)&aval;

	memset(delayline, 0, 4096 * 4);
	memset(auxdelayline, 0, 4096 * 4);
	delaypos = 0;

	mode = 3;
	mode2 = 3;

	level = 0;
	level2 = 0;
	beenidle = 0xFFFF;
}

mi::~mi()
{
	AB_Disconnect(this);
}

void mi::MDKInit(CMachineDataInput * const pi)
{
	me = pCB->GetThisMachine();

	if (pi != NULL)
	{
			byte ver;
			pi->Read(ver);
			if (ver == 1)
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

void mi::Tick()
{
	int i;

	if(gval.bypass == SWITCH_ON)
		bypass = true;
	else if(gval.bypass == SWITCH_OFF)
		bypass = false;

	if(gval.threshold != 255) {
		threshold = (float)pow(10.0, (double)-(200 - gval.threshold) / 80) * 32768.f;
	}

	if(gval.lookahead != 255)
		delaylen = pMasterInfo->SamplesPerSec * gval.lookahead / 10000;

	if(gval.attack != 255) {
		attacktime = pMasterInfo->SamplesPerSec * (gval.attack) / 2000;
		attackmul = exp(-log(quiet) / (double)attacktime);
	}

	if(gval.hold != 255) {
		holdtime = pMasterInfo->SamplesPerSec * (gval.hold) / 1000;
	}

	if(gval.release != 255) {
		releasetime = pMasterInfo->SamplesPerSec * (gval.release) / 1000;
		releasemul = exp(log(quiet) / (double)releasetime);
	}

	if(gval.invert == SWITCH_OFF)
		invert = false;
	else if(gval.invert == SWITCH_ON)
		invert = true;
}

bool mi::WorkAuxbus(float *psamples, int numsamples, int const m)
{
	bool active = false;

	AB_Receive(Channel, data, numsamples);
	float *auxsamp = (float*)data;

	if(m & WM_WRITE) {
		while(numsamples--) {

			delayline[(delaypos + delaylen) & 2047] = *psamples;
			auxdelayline[(delaypos + delaylen) & 2047] = *auxsamp;

			float in = auxdelayline[delaypos & 2047];

			float out = delayline[delaypos & 2047];

			if(mode == 0) {
				active = true;
				level *= curattackmul;
				time--;
				if(time == 0) {
					mode = 1;
					level = 1;
					time = holdtime;
				}
			} else if(mode == 3) {
				if((fabs(in) > threshold)) {
					level = quiet;
					mode = 0;
					time = attacktime;
					curattackmul = attackmul;
				}
			} else if(mode == 1) {
				active = true;
				time--;
				if((fabs(in) > threshold))
					time = holdtime;

				if(!time) {
					mode = 2;
					time = releasetime;
				}
			} else if(mode == 2) {
				active = true;
				level *= releasemul;
				time--;

				if((fabs(in) > threshold)) {
					mode = 0;
					time = attacktime;
					curattackmul = exp(-log(level) / (double)attacktime);
				}

				if(!time) {
					mode = 3;
					level = 0;
				}
			}

			if(invert) {
				if(*(int*)&level)
					*psamples = out * (quiet / level);
				else {
					*psamples = out;
					active = true;
				}
			} else
				*psamples = out * level;
			delaypos++;
			psamples++;
			auxsamp++;
		}
	} else {
		while(numsamples--) {

			delayline[(delaypos + delaylen) & 2047] = *psamples;
			auxdelayline[(delaypos + delaylen) & 2047] = *auxsamp;

			float in = auxdelayline[delaypos & 2047];

			if(mode == 0) {
				active = true;
				level *= curattackmul;
				time--;
				if(time == 0) {
					mode = 1;
					level = 1;
					time = holdtime;
				}
			} else if(mode == 3) {
				if((fabs(in) > threshold)) {
					level = quiet;
					mode = 0;
					time = attacktime;
					curattackmul = attackmul;
				}
			} else if(mode == 1) {
				active = true;
				time--;
				if((fabs(in) > threshold))
					time = holdtime;

				if(!time) {
					mode = 2;
					time = releasetime;
				}
			} else if(mode == 2) {
				active = true;
				level *= releasemul;
				time--;

				if((fabs(in) > threshold)) {
					mode = 0;
					time = attacktime;
					curattackmul = exp(-log(level) / (double)attacktime);
				}

				if(!time) {
					mode = 3;
					level = 0;
				}
			}

			delaypos++;
			psamples++;
			auxsamp++;
		}
	}

	return active;
}


bool mi::MDKWork(float *psamples, int numsamples, int const m)
{
	bool active = false;

	if(m == WM_NOIO) {
		mode = 3;
		time = 0;
		return false;
	}

	if(bypass == true) {
		if(m & WM_READ) return true;
		else return false;
	}

	if(!(m & WM_READ)) {
		if(beenidle < delaylen) {
			beenidle += numsamples;
			memset(psamples, 0, numsamples * 4);
		} else {
			mode = 3;
			time = 0;
			return false;
		}
	} else
		beenidle = 0;

	if(Channel != -1)
		return WorkAuxbus(psamples, numsamples, m);

	if(m & WM_WRITE) {
		while(numsamples--) {

			float in = *psamples;
			delayline[(delaypos + delaylen) & 2047] = *psamples;

			float out = delayline[delaypos & 2047];

			if(mode == 0) {
				active = true;
				level *= curattackmul;
				time--;
				if(time == 0) {
					mode = 1;
					level = 1;
					time = holdtime;
				}
			} else if(mode == 3) {
				if((fabs(in) > threshold)) {
					level = quiet;
					mode = 0;
					time = attacktime;
					curattackmul = attackmul;
				}
			} else if(mode == 1) {
				active = true;
				time--;
				if((fabs(in) > threshold))
					time = holdtime;

				if(!time) {
					mode = 2;
					time = releasetime;
				}
			} else if(mode == 2) {
				active = true;
				level *= releasemul;
				time--;

				if((fabs(in) > threshold)) {
					mode = 0;
					time = attacktime;
					curattackmul = exp(-log(level) / (double)attacktime);
				}

				if(!time) {
					mode = 3;
					level = 0;
				}
			}

			if(invert) {
				if(*(int*)&level)
					*psamples = out * (quiet / level);
				else {
					*psamples = out;
					active = true;
				}
			} else
				*psamples = out * level;
			delaypos++;
			psamples++;
		}
	} else {
		while(numsamples--) {

			float in = *psamples;
			delayline[(delaypos + delaylen) & 2047] = *psamples;

			if(mode == 0) {
				active = true;
				level *= curattackmul;
				time--;
				if(time == 0) {
					mode = 1;
					level = 1;
					time = holdtime;
				}
			} else if(mode == 3) {
				if((fabs(in) > threshold)) {
					level = quiet;
					mode = 0;
					time = attacktime;
					curattackmul = attackmul;
				}
			} else if(mode == 1) {
				active = true;
				time--;
				if((fabs(in) > threshold))
					time = holdtime;

				if(!time) {
					mode = 2;
					time = releasetime;
				}
			} else if(mode == 2) {
				active = true;
				level *= releasemul;
				time--;

				if((fabs(in) > threshold)) {
					mode = 0;
					time = attacktime;
					curattackmul = exp(-log(level) / (double)attacktime);
				}

				if(!time) {
					mode = 3;
					level = 0;
				}
			}

			delaypos++;
			psamples++;
		}
	}

	return active;
}

bool mi::WorkStereoAuxbus(float *psamples, int numsamples, int const m)
{
	bool active = false;

	AB_Receive(Channel, data, numsamples);
	float *auxsamp = (float*)data;

	if(m & WM_WRITE) {
		while(numsamples--) {

			auxdelayline[(delaypos + (delaylen << 1)) & 4095] = *auxsamp;

			float in = auxdelayline[delaypos & 4095];

			delayline[(delaypos + (delaylen << 1)) & 4095] = *psamples;
			delayline[(delaypos + (delaylen << 1) + 1) & 4095] = *(psamples + 1);

			float outl = delayline[delaypos & 4095];
			float outr = delayline[(delaypos + 1) & 4095];

			if(mode == 0) {
				active = true;
				level *= curattackmul;
				time--;
				if(time == 0) {
					mode = 1;
					level = 1;
					time = holdtime;
				}
			} else if(mode == 3) {
				if((fabs(in) > threshold)) {
					level = quiet;
					mode = 0;
					time = attacktime;
					curattackmul = attackmul;
				}
			} else if(mode == 1) {
				active = true;
				time--;
				if((fabs(in) > threshold))
					time = holdtime;

				if(!time) {
					mode = 2;
					time = releasetime;
				}
			} else if(mode == 2) {
				active = true;
				level *= releasemul;
				time--;

				if((fabs(in) > threshold)) {
					mode = 0;
					time = attacktime;
					curattackmul = exp(-log(level) / (double)attacktime);
				}

				if(!time) {
					mode = 3;
					level = 0;
				}
			}

			if(invert) {
				if(*(int*)&level) {
					float invlevel = (quiet / level);
					*psamples = outl * invlevel;
					*(psamples + 1) = outr * invlevel;
				} else {
					*psamples = outl;
					*(psamples + 1) = outr;
					active = true;
				}
			} else {
				*psamples = outl * level;
				*(psamples + 1) = outr * level;
			}

			delaypos += 2;
			psamples += 2;
			auxsamp++;
		}
	} else {
		while(numsamples--) {

			auxdelayline[(delaypos + (delaylen << 1)) & 4095] = *auxsamp;

			float in = auxdelayline[delaypos & 4095];

			delayline[(delaypos + (delaylen << 1)) & 4095] = *psamples;
			delayline[(delaypos + (delaylen << 1) + 1) & 4095] = *(psamples + 1);

			if(mode == 0) {
				active = true;
				level *= curattackmul;
				time--;
				if(time == 0) {
					mode = 1;
					level = 1;
					time = holdtime;
				}
			} else if(mode == 3) {
				if((fabs(in) > threshold)) {
					level = quiet;
					mode = 0;
					time = attacktime;
					curattackmul = attackmul;
				}
			} else if(mode == 1) {
				active = true;
				time--;
				if((fabs(in) > threshold))
					time = holdtime;

				if(!time) {
					mode = 2;
					time = releasetime;
				}
			} else if(mode == 2) {
				active = true;
				level *= releasemul;
				time--;

				if((fabs(in) > threshold)) {
					mode = 0;
					time = attacktime;
					curattackmul = exp(-log(level) / (double)attacktime);
				}

				if(!time) {
					mode = 3;
					level = 0;
				}
			}

			delaypos += 2;
			psamples += 2;
			auxsamp++;
		}
	}

	return active;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const m)
{
	bool active = false;

	if(m == WM_NOIO) {
		mode = 3;
		mode2 = 3;
		return false;
	}

	if(bypass == true) {
		if(m & WM_READ) return true;
		else return false;
	}

	if(!(m & WM_READ)) {
		if(beenidle < delaylen) {
			beenidle += numsamples;
			memset(psamples, 0, numsamples * 8);
		} else {
			mode = 3;
			mode2 = 3;
			return false;
		}
	} else
		beenidle = 0;

	if(Channel != -1)
		return WorkStereoAuxbus(psamples, numsamples, m);

	if(stereoseparated) {

		if(m & WM_WRITE) {
			while(numsamples--) {

				float in = *psamples;
				float inr = *(psamples + 1);

				delayline[(delaypos + (delaylen << 1)) & 4095] = *psamples;
				delayline[(delaypos + (delaylen << 1) + 1) & 4095] = *(psamples + 1);

				float outl = delayline[delaypos & 4095];
				float outr = delayline[(delaypos + 1) & 4095];

				if(mode == 0) {
					active = true;
					level *= curattackmul;
					time--;
					if(time == 0) {
						mode = 1;
						level = 1;
						time = holdtime;
					}
				} else if(mode == 3) {
					if((fabs(in) > threshold)) {
						level = quiet;
						mode = 0;
						time = attacktime;
						curattackmul = attackmul;
					}
				} else if(mode == 1) {
					active = true;
					time--;
					if((fabs(in) > threshold))
						time = holdtime;

					if(!time) {
						mode = 2;
						time = releasetime;
					}
				} else if(mode == 2) {
					active = true;
					level *= releasemul;
					time--;

					if((fabs(in) > threshold)) {
						mode = 0;
						time = attacktime;
						curattackmul = exp(-log(level) / (double)attacktime);
					}

					if(!time) {
						mode = 3;
						level = 0;
					}
				}

				if(mode2 == 0) {
					active = true;
					level2 *= curattackmul2;
					time2--;
					if(time2 == 0) {
						mode2 = 1;
						level2 = 1;
						time2 = holdtime;
					}
				} else if(mode2 == 3) {
					if((fabs(inr) > threshold)) {
						level2 = quiet;
						mode2 = 0;
						time2 = attacktime;
						curattackmul2 = attackmul;
					}
				} else if(mode2 == 1) {
					active = true;
					time2--;
					if((fabs(inr) > threshold))
						time2 = holdtime;

					if(!time2) {
						mode2 = 2;
						time2 = releasetime;
					}
				} else if(mode2 == 2) {
					active = true;
					level2 *= releasemul;
					time2--;

					if((fabs(inr) > threshold)) {
						mode2 = 0;
						time2 = attacktime;
						curattackmul2 = exp(-log(level2) / (double)attacktime);
					}

					if(!time2) {
						mode2 = 3;
						level2 = 0;
					}
				}

				*psamples = outl * level;
				*(psamples + 1) = outr * level2;

				if(invert) {
					if(*(int*)&level) {
						*psamples = outl * (quiet / level);
					} else {
						*psamples = outl;
						active = true;
					}
					if(*(int*)&level2) {
						*(psamples + 1) = outr * (quiet / level2);
					} else {
						*(psamples + 1) = outr;
						active = true;
					}
				} else {
					*psamples = outl * level;
					*(psamples + 1) = outr * level2;
				}

				delaypos += 2;
				psamples += 2;
			}
		} else {

			while(numsamples--) {
				float in = *psamples;
				float inr = *(psamples + 1);

				delayline[(delaypos + (delaylen << 1)) & 4095] = *psamples;
				delayline[(delaypos + (delaylen << 1) + 1) & 4095] = *(psamples + 1);

				if(mode == 0) {
					active = true;
					level *= curattackmul;
					time--;
					if(time == 0) {
						mode = 1;
						level = 1;
						time = holdtime;
					}
				} else if(mode == 3) {
					if((fabs(in) > threshold)) {
						level = quiet;
						mode = 0;
						time = attacktime;
						curattackmul = attackmul;
					}
				} else if(mode == 1) {
					active = true;
					time--;
					if((fabs(in) > threshold))
						time = holdtime;

					if(!time) {
						mode = 2;
						time = releasetime;
					}
				} else if(mode == 2) {
					active = true;
					level *= releasemul;
					time--;

					if((fabs(in) > threshold)) {
						mode = 0;
						time = attacktime;
						curattackmul = exp(-log(level) / (double)attacktime);
					}

					if(!time) {
						mode = 3;
						level = 0;
					}
				}

				if(mode2 == 0) {
					active = true;
					level2 *= curattackmul2;
					time2--;
					if(time2 == 0) {
						mode2 = 1;
						level2 = 1;
						time2 = holdtime;
					}
				} else if(mode2 == 3) {
					if((fabs(inr) > threshold)) {
						level2 = quiet;
						mode2 = 0;
						time2 = attacktime;
						curattackmul2 = attackmul;
					}
				} else if(mode2 == 1) {
					active = true;
					time2--;
					if((fabs(inr) > threshold))
						time2 = holdtime;

					if(!time2) {
						mode2 = 2;
						time2 = releasetime;
					}
				} else if(mode2 == 2) {
					active = true;
					level2 *= releasemul;
					time2--;

					if((fabs(inr) > threshold)) {
						mode2 = 0;
						time2 = attacktime;
						curattackmul2 = exp(-log(level2) / (double)attacktime);
					}

					if(!time2) {
						mode2 = 3;
						level2 = 0;
					}
				}

				delaypos += 2;
				psamples += 2;
			}
		}

	} else {

		if(m & WM_WRITE) {
			while(numsamples--) {

				float in = *psamples;
				if(in < *(psamples + 1)) in = *(psamples + 1);

				delayline[(delaypos + (delaylen << 1)) & 4095] = *psamples;
				delayline[(delaypos + (delaylen << 1) + 1) & 4095] = *(psamples + 1);

				float outl = delayline[delaypos & 4095];
				float outr = delayline[(delaypos + 1) & 4095];

				if(mode == 0) {
					active = true;
					level *= curattackmul;
					time--;
					if(time == 0) {
						mode = 1;
						level = 1;
						time = holdtime;
					}
				} else if(mode == 3) {
					if((fabs(in) > threshold)) {
						level = quiet;
						mode = 0;
						time = attacktime;
						curattackmul = attackmul;
					}
				} else if(mode == 1) {
					active = true;
					time--;
					if((fabs(in) > threshold))
						time = holdtime;

					if(!time) {
						mode = 2;
						time = releasetime;
					}
				} else if(mode == 2) {
					active = true;
					level *= releasemul;
					time--;

					if((fabs(in) > threshold)) {
						mode = 0;
						time = attacktime;
						curattackmul = exp(-log(level) / (double)attacktime);
					}

					if(!time) {
						mode = 3;
						level = 0;
					}
				}

				if(invert) {
					if(*(int*)&level) {
						float invlevel = (quiet / level);
						*psamples = outl * invlevel;
						*(psamples + 1) = outr * invlevel;
					} else {
						*psamples = outl;
						*(psamples + 1) = outr;
						active = true;
					}
				} else {
					*psamples = outl * level;
					*(psamples + 1) = outr * level;
				}

				delaypos += 2;
				psamples += 2;
			}
		} else {
			while(numsamples--) {

				float in = *psamples;
				if(in < *(psamples + 1)) in = *(psamples + 1);

				delayline[(delaypos + (delaylen << 1)) & 4095] = *psamples;
				delayline[(delaypos + (delaylen << 1) + 1) & 4095] = *(psamples + 1);

				if(mode == 0) {
					active = true;
					level *= curattackmul;
					time--;
					if(time == 0) {
						mode = 1;
						level = 1;
						time = holdtime;
					}
				} else if(mode == 3) {
					if((fabs(in) > threshold)) {
						level = quiet;
						mode = 0;
						time = attacktime;
						curattackmul = attackmul;
					}
				} else if(mode == 1) {
					active = true;
					time--;
					if((fabs(in) > threshold))
						time = holdtime;

					if(!time) {
						mode = 2;
						time = releasetime;
					}
				} else if(mode == 2) {
					active = true;
					level *= releasemul;
					time--;

					if((fabs(in) > threshold)) {
						mode = 0;
						time = attacktime;
						curattackmul = exp(-log(level) / (double)attacktime);
					}

					if(!time) {
						mode = 3;
						level = 0;
					}
				}

				delaypos += 2;
				psamples += 2;
			}
		}
	}

	return active;
}


char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param) {
	case 0:
		if(value == SWITCH_OFF) return "No";
		else return "Yes";

	case 1:
		sprintf(txt, "%1.2f dB", (double)-(200 - value) / 4);
		break;

	case 2:
		sprintf(txt, "%1.1f ms", (double)value / 10);
		break;

	case 3:
		sprintf(txt, "%1.1f ms", (double)value / 2);
		break;

	case 4:
		sprintf(txt, "%1.1f ms", (double)value);
		break;

	case 5:
		sprintf(txt, "%1.1f ms", (double)value);
		break;

	default: return 0;
	}

	return txt;
}
