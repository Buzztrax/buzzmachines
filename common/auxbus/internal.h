#ifndef __INTERNAL_H
#define __INTERNAL_H

#include "../machineinterface.h"
#include "../dsplib/dsplib.h"


#define NUM_BUSES	64

typedef void (*AB_DisconnectCallback)(void *user);


class CAuxBus
{
public:
	CAuxBus()
	{
		InputCB = NULL;
		InputUser = NULL;
		InputUserName = NULL;

		OutputCB = NULL;
		OutputUser = NULL;
		OutputUserName = NULL;

		Reset();
	}
		
	void Reset()
	{
		DSP_Zero(Buffer, MAX_BUFFER_LENGTH);
		ReadPos = 0;
		WritePos = 0;
	}

	void SetInput(void *user, AB_DisconnectCallback cb, char const *name);
	void SetOutput(void *user, AB_DisconnectCallback cb, char const *name);



public:
	void *InputUser;
	AB_DisconnectCallback InputCB;
	char const *InputUserName;

	void *OutputUser;
	AB_DisconnectCallback OutputCB;
	char const *OutputUserName;

	int ReadPos;
	int WritePos;
	float Buffer[MAX_BUFFER_LENGTH];

};

#endif