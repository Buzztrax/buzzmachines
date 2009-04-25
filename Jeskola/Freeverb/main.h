
const int BYTE_NO=0xff;

class miex : public CMDKMachineInterfaceEx {
	/*virtual void Input(float* psamples, int numsamples, float amp) {
		char pc[1024];
		sprintf(pc, "amp: %f", amp);
		//MessageBox(0, pc, "Input", MB_OK);
		CMDKMachineInterfaceEx::Input(psamples, numsamples, amp);
	}*/
	virtual void AddInput(char const *macname, bool stereo) {
		CMDKMachineInterfaceEx::AddInput(macname, stereo);
	}	// called when input is added to a machine
	virtual void DeleteInput(char const *macename) {
		CMDKMachineInterfaceEx::DeleteInput(macename);
	}			
	virtual void RenameInput(char const *macoldname, char const *macnewname) {
		CMDKMachineInterfaceEx::RenameInput(macoldname, macnewname);
	}

};


class mi : public CMDKMachineInterface {

	revmodel reverb;

	int faderCounter;

#pragma pack(1)	
	class gvals {
	public:
		byte revTime;
		byte hiDamp;
		byte preDelay;
		byte loCut;
		byte hiCut;
		byte revOut;
		byte dryOut;
	};
#pragma pack()

public:
	mi();
	virtual ~mi();

	virtual void Tick();

	virtual void MDKInit(CMachineDataInput * const pi);
	virtual bool MDKWork(float *psamples, int numsamples, int const mode);
	virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
	virtual char const *DescribeValue(int const param, int const value);
	virtual void Command(int const i);

	virtual void MDKSave(CMachineDataOutput * const po);

	virtual void AttributesChanged() {
		//MessageBox(0, "AttributesChanged", "mi", MB_OK);
	}

	virtual void SetNumTracks(int const n) {
		//MessageBox(0, "SetNumTracks", "mi", MB_OK);
	}


public:
	virtual CMDKMachineInterfaceEx *GetEx() { 
		return &ex; 
	}
	virtual void OutputModeChanged(bool stereo) { 
		//MessageBox(0, "OutputModeChanged", "mi", MB_OK);
		//CMDKMachineInterface::OutputModeChanged(stereo);
	}

public:
	miex ex;

public:
	gvals gval;
};
