/*	CAdsrEnv
 *
 *		ADSR with linear attack, exp decay and release, click removal
 */

#ifndef inc_dspcAdsr
#define inc_dspcAdsr

#ifndef dspcAdsr_StartVol
#define dspcAdsr_StartVol 0.001
#endif

class CAdsrEnv {
	enum  {	sAttack=0,sDecay,sSustain,sRelease,sStopped };

	double	fMsToSample;
	double	fLogStep;
	double	fLinStep;
	int		iState;
	int		iAttack;
	int		iDecay;
	int		iRelease;
	double	fInvAttack;
	double	fInvDecay;
	double	fInvRelease;
	double	fSustain;
	double	fAmplitude;
public:
	double	fVolume;
	int		iCount;

	void Init() {
		fMsToSample = (double)g_iSampleRate / 1000.0;
		SetAdsr(10,500,0.5,2000);
		Reset(); }

	void Reset() {
		iState		= sStopped;
		fLinStep	= 0.0;
		fLogStep	= 1.0;
		iCount		= 0x7FFFFFFF;
		fVolume		= 0; }

	bool IsPlaying() { return (iState < sStopped); }

	void Stop() { Reset(); }

	void SetAttack(double const a) {
		iAttack = (int)floor(a * fMsToSample);
		fInvAttack = 1.0 / (double)iAttack; }
	
	void SetDecay(double const a) {
		iDecay = (int)floor(a * fMsToSample);
		fInvDecay = 1.0 / (double)iDecay; }
	
	void SetRelease(double const a) {
		iRelease = (int)floor(a * fMsToSample);
		fInvRelease = 1.0 / (double)iRelease; }

	void SetSustain(double const s) {
		fSustain = s; }
	
	void SetAdsr(double const a,double const d,double const s,double const r) {
		SetAttack(a);
		SetDecay(d);
		SetRelease(r);
		SetSustain(s); }

	double GetAvailability() {
		if(iState > sAttack) return fVolume;
		else return fAmplitude; }

	void NoteOn(double const a=1.0) {
		fAmplitude	= a;
		fVolume		= fAmplitude*dspcAdsr_StartVol;
		iCount		= iAttack;
		fLinStep	= DspCalcLinStep(fAmplitude*dspcAdsr_StartVol,fAmplitude,fInvAttack);
		fLogStep	= 1.0;
		iState		= sAttack; }

	void NoteOff() {
		if(iState < sRelease) {
			fLogStep	= DspCalcLogStep(fVolume,dspcAdsr_StartVol,fInvRelease);
			fLinStep	= DspCalcLinStep(fAmplitude*dspcAdsr_StartVol,0,fInvRelease);
			iCount		= iRelease;
			iState		= sRelease; } }

#pragma optimize ("a", on)

	double Work() {
		double	lns;
		double	lgs = GetStep(lns);
		double	vol = fVolume;
		iCount--;
		fVolume *= lgs;
		fVolume += lns;
		return vol; }

	void WorkSamples(float *pb, int numsamples) {
		double vol = fVolume;
		while(numsamples) {
			double		 lns;
			double const lgs = GetStep(lns);
			int			 cnt = __min(numsamples,iCount);
			numsamples		-= cnt;
			iCount			-= cnt;
			do {
				*pb++ = (float)vol;
				vol *= lgs;
				vol	+= lns;
			} while(--cnt); }
		fVolume	= vol; }

	void WorkSamplesScale(float *pb, int numsamples) {
		double vol = fVolume;
		while(numsamples) {
			double		 lns;
			double const lgs = GetStep(lns);
			int			 cnt = __min(numsamples,iCount);
			numsamples		-= cnt;
			iCount			-= cnt;
			do {
				*pb++ *= (float)vol;
				vol *= lgs;
				vol	+= lns;
			} while(--cnt); }
		fVolume	= vol; }

	double GetStep(double &l) {
		if(iCount<=0) {
			switch(++iState) {
			case sDecay:
				if(!fSustain) {
					fLogStep	= DspCalcLogStep(1,dspcAdsr_StartVol,fInvDecay);
					fLinStep	= DspCalcLinStep(fAmplitude*dspcAdsr_StartVol,0,fInvDecay);
					iState		= sRelease; }
				else {
					fLogStep	= DspCalcLogStep(1,fSustain,fInvDecay);
					fLinStep	= 0.0; }
				iCount	= iDecay;
				break;
			case sSustain:
				fLinStep	= 0.0;
				fLogStep	= 1.0; 
				iCount		= 0x7FFFFFFF;
				break;
			case sStopped:
				fLinStep	= 0.0;
				fLogStep	= 1.0;
				iCount		= 0x7FFFFFFF;
				fVolume		= 0;
				break; } }
		l = fLinStep; return fLogStep; }

#pragma optimize ("a", off)

 };


#endif
