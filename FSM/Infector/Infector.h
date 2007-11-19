#define MAX_CHANNELS		24
#define MAX_TRACKS		12

class mi;

#pragma pack(1)

class gvals
{
public:
  byte vWaveformA;
	byte vPWMRateA;
	byte vPWMRangeA;
	byte vPWOffsetA;
  byte vWaveformB;
	byte vPWMRateB;
	byte vPWMRangeB;
	byte vPWOffsetB;
  byte vTranspose;
  byte vDetune;
	byte vOscMix;     // 10
	byte vSubOscWave;
	byte vSubOscVol;
  byte vGlide;

  byte vFilterType; 
  byte vFilterCutoff;
  byte vFilterResonance;
  byte vFilterModulation;
  byte vFilterAttack;

  byte vFilterDecay; 
	byte vFilterSustain; // 20
	byte vFilterRelease;
  byte vFilterShape;
  byte vFilterInertia;
	byte vFilterTrack;

  byte vLFORate;
  byte vLFOAmount1; 
  byte vLFOAmount2; 
  byte vLFOShape;

  byte vLFO2Rate;
  byte vLFO2Amount1; 
  byte vLFO2Amount2; 
  byte vLFO2Shape;

  byte vAmpAttack;
  byte vAmpDecay;
	byte vAmpSustain;
	byte vAmpRelease;

	byte vLFOMode;
};

class tvals
{
public:
  byte vNote;
  byte vVelocity;
  byte vLength;
	byte vCommand1;
	word vParam1;
	byte vCommand2;
	word vParam2;
};

class avals
{
public:
	int channel;
  int usevelocity;
  int hq;
  int crispness;
  int theviderness;
  int tuning;
	int cliptable;
};

#pragma pack()

class CTrack;

class CChannel
{
public:
  float Frequency;
  float PhaseOSC1;
  float PhaseOSC2;
  float PhaseOSC3;
	float Velocity;
  C6thOrderFilter Filter;
  CADSREnvelope FilterEnv;
  CADSREnvelope AmpEnv;
  int Phase1, Phase2;
  float Detune;
	
  CInertia inrKeyTrack;
  CInertia inrCutoff2;

  CTrack *pTrack;
  
  CChannel();
  void Init();
	void ClearFX();
  bool IsFree() { return AmpEnv.m_nState==4; }
  void Reset();
  void NoteReset();
};
 
class CTrack
{
public:
  mi *pmi;
  int channel;
  byte note,accent,length;
	byte lastnote, lastaccent, lastlength;
  float DestFrequency, BaseFrequency, NotePortFrequency;
  float Detune;
	
  char Arps[3];
	int ArpPoint, ArpCount;
  int MidiNote;
	int RetrigCount, RetrigPoint, RetrigMode;
	float Vib1Phase,Vib2Phase;
	int ShuffleCounter, ShuffleMax, ShuffleAmount;
	int ShuffleData[16];
	float Vib1Rate,Vib1Depth,Vib2Rate,Vib2Depth;
  int NoTrig;
	float SlideCounter, SlideEnd, SlideRange;

  float LFOPhase, LFO2Phase;
  float CurLFO, CurLFO2;

  CInertia inrLFO1;
  CInertia inrLFO2;

  CTrack();
  byte AllocChannel();
  CChannel *Chn();
  void PlayNote(byte note, byte accent, byte length, CMasterInfo *pMasterInfo);
	void CommandA(byte cmd, word param);
	void CommandB(byte cmd, word param);
	void ClearFX();
	int GetWakeupTime(int maxtime);
	void UseWakeupTime(int maxtime);
	void DoWakeup(mi *pmi);
  void DoLFO(mi *pmi, int c);
  void Init();
  void Reset();
};
 
struct CWaveSource {
  int nSampleNo;
  int nPosition;
  int nStretch;
  int nSmoothing;
  int nClip;
  int nBend;
  int nGain;
  int nDummy1;
  int nDummy2;

  CWaveSource(): nSampleNo(0),nPosition(0),nStretch(72*16),nSmoothing(0),nClip(0),nBend(0),nGain(75),nDummy1(0),nDummy2(0) {}
};

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
	virtual void Stop();
  virtual void MidiNote(int const channel, int const value, int const velocity);

	virtual char const *DescribeValue(int const param, int const value);

	short const *GetOscillatorTab(int const waveform);
  void DoPlay();
	virtual void Command(int const i);
	virtual void Save(CMachineDataOutput * const po);

	void GenerateUserWaves(int Slot);
  void Reset();
  void ClearFX();

private:
	void InitTrack(int const i);
	void ResetTrack(int const i);

	void TickTrack(CTrack *pt, tvals *ptval);
	bool WorkTrack(CTrack *pt, float *pin, float *pout, int numsamples, int const mode);


public:
  CInertia inrCutoff;
  CInertia inrResonance;
  CInertia inrModulation;
  CInertia inrModShape;
  CInertia inrLFO1Dest1;
  CInertia inrLFO1Dest2;
  CInertia inrLFO2Dest1;
  CInertia inrLFO2Dest2;

	gvals gvalAct;
	CChannel Channels[MAX_CHANNELS];
	int numTracks;
	CTrack Tracks[MAX_TRACKS+1];
  float CurCutoff, CurRes;
  avals aval;

	CBandlimitedTable usertables[8];
  float userwaves[8][2048];
  CWaveSource usersources[8];
	int PWMBuffer1[256];
	int PWMBuffer2[256];
  CPWMLFO Osc1PWM, Osc2PWM;

private:
  int nCurChannel;

	gvals gval;
	tvals tval[MAX_TRACKS];

  CMachine *ThisMachine;
};

#define LFOPAR2TIME(value) (0.03*pow(600.0,value/240.0))
#define GETENVTIME(value) (0.08*pow(150.0,value/240.0))
#define GETENVTIME2(value) (0.005*pow(2400.0,value/240.0))

