
#ifndef __AUX_BUS_H
#define __AUX_BUS_H

#define DI __declspec(dllimport)

typedef void (*AB_DisconnectCallback)(void *user);

DI void AB_ShowEditor(int *in, int *out, char const *name, AB_DisconnectCallback cb, void *user);

DI void AB_Send(int in, float *psamples, int numsamples);
DI void AB_Receive(int out, float *psamples, int numsamples);

DI void AB_ConnectInput(int in, char const *name, AB_DisconnectCallback cb, void *user);
DI void AB_ConnectOutput(int out, char const *name, AB_DisconnectCallback cb, void *user);

DI void AB_Disconnect(void *user);

#endif