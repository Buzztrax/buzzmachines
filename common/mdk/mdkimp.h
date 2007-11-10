#ifndef __MDK_IMP_H
#define __MDK_IMP_H

#include <list>
#include <string>

#include <MachineInterface.h>

#define MDK_VERSION		2

class CInput
{
public:
	CInput(char const *n, bool st) : Name(n), Stereo(st) {}

public:
	std::string Name;
	bool Stereo;

};

typedef std::list<CInput> InputList;

class CMDKImplementation
{
	friend class CMDKMachineInterface;
	friend class CMDKMachineInterfaceEx;
public:
	virtual ~CMDKImplementation();

	virtual void AddInput(char const *macname, bool stereo);
	virtual void DeleteInput(char const *macname);
	virtual void RenameInput(char const *macoldname, char const *macnewname);
	virtual void SetInputChannels(char const *macname, bool stereo);
	virtual void Input(float *psamples, int numsamples, float amp);

	virtual bool Work(float *psamples, int numsamples, int const mode);
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode);
	virtual void Init(CMachineDataInput * const pi);
	virtual void Save(CMachineDataOutput * const po);
	
	virtual void SetOutputMode(bool stereo);
	
protected:	
	void SetMode();


public:
	CMDKMachineInterface *pmi;

	InputList Inputs;
	InputList::iterator InputIterator;

	int HaveInput;
	int numChannels;
	int MachineWantsChannels;

	CMachine *ThisMachine;
	
	float Buffer[2*MAX_BUFFER_LENGTH];
};

#endif
