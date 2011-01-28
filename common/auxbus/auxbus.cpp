// auxbus.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>
#include "editordlg.h"
#include "internal.h"

/*
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
*/
class CApp : public CWinApp
{
public:
        virtual BOOL InitInstance()
        {
                AfxEnableControlContainer();
                return TRUE;
        }
};


CApp App;

#define DE __declspec(dllexport)


CAuxBus AuxBuses[NUM_BUSES];



DE void AB_ShowEditor(int *in, int *out, char const *name, AB_DisconnectCallback cb, void *user)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	

	CEditorDlg dlg;
	dlg.AuxIn = in;
	dlg.AuxOut = out;
	dlg.AuxName = name;
	dlg.AuxCB = cb;
	dlg.AuxUser = user;

	dlg.DoModal();


}


DE void AB_Send(int in, float *psamples, int numsamples)
{
	CAuxBus *paux = AuxBuses + in - 1;

	while(numsamples > 0)
	{
		int n = __min(numsamples, MAX_BUFFER_LENGTH - paux->WritePos);

		if (psamples == NULL)
			DSP_Zero(paux->Buffer + paux->WritePos, n);
		else
			DSP_Copy(paux->Buffer + paux->WritePos, psamples, n);

		if (psamples != NULL)
			psamples += n;

		numsamples -= n;
		paux->WritePos = (paux->WritePos + n) & (MAX_BUFFER_LENGTH-1);

	}


}

DE void AB_Receive(int out, float *psamples, int numsamples)
{
	CAuxBus *paux = AuxBuses + out - 1;
	paux->ReadPos = (paux->WritePos - numsamples) & (MAX_BUFFER_LENGTH-1);	// fix: always read last written samples

	while(numsamples > 0)
	{
		int n = __min(numsamples, MAX_BUFFER_LENGTH - paux->ReadPos);

		DSP_Copy(psamples, paux->Buffer + paux->ReadPos, n);

		psamples += n;
		numsamples -= n;
		paux->ReadPos = (paux->ReadPos + n) & (MAX_BUFFER_LENGTH-1);

	}

}




DE void AB_ConnectInput(int in, char const *name, AB_DisconnectCallback cb, void *user)
{
	AuxBuses[in-1].SetInput(user, cb, name);
}

DE void AB_ConnectOutput(int out, char const *name, AB_DisconnectCallback cb, void *user)
{
	AuxBuses[out-1].SetOutput(user, cb, name);
}

DE void AB_Disconnect(void *user)
{
	for (int c = 0; c < NUM_BUSES; c++)
	{
		if (AuxBuses[c].InputUser == user)
		{
			AuxBuses[c].InputCB(AuxBuses[c].InputUser);
			AuxBuses[c].InputUser = NULL;
			AuxBuses[c].InputCB = NULL;
			AuxBuses[c].InputUserName = NULL;
		}
	}

	for (c = 0; c < NUM_BUSES; c++)
	{
		if (AuxBuses[c].OutputUser == user)
		{
			AuxBuses[c].OutputCB(AuxBuses[c].OutputUser);
			AuxBuses[c].OutputUser = NULL;
			AuxBuses[c].OutputCB = NULL;
			AuxBuses[c].OutputUserName = NULL;
		}
	}

}


void CAuxBus::SetInput(void *user, AB_DisconnectCallback cb, char const *name)
{
	if (InputUser != NULL)
		InputCB(InputUser);

	for (int c = 0; c < NUM_BUSES; c++)
	{
		if (AuxBuses[c].InputUser == user)
		{
			AuxBuses[c].InputCB(AuxBuses[c].InputUser);
			AuxBuses[c].InputUser = NULL;
			AuxBuses[c].InputCB = NULL;
			AuxBuses[c].InputUserName = NULL;
		}
	}

	InputUser = user;
	InputCB = cb;
	InputUserName = name;

}


void CAuxBus::SetOutput(void *user, AB_DisconnectCallback cb, char const *name)
{
	if (OutputUser != NULL)
		OutputCB(OutputUser);

	for (int c = 0; c < NUM_BUSES; c++)
	{
		if (AuxBuses[c].OutputUser == user)
		{
			AuxBuses[c].OutputCB(AuxBuses[c].OutputUser);
			AuxBuses[c].OutputUser = NULL;
			AuxBuses[c].OutputCB = NULL;
			AuxBuses[c].OutputUserName = NULL;
		}
	}

	OutputUser = user;
	OutputCB = cb;
	OutputUserName = name;

}