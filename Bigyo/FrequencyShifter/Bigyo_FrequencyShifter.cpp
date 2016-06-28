#include <math.h>
#include <float.h>

#include <MachineInterface.h>
#include <mdk/mdk.h>

#include "ComplexFloat.h"
#include "FastCosSin.h"
#include "HilbertPair.h"
#include "Allpass2.h"
#include "LinLog.h"

#pragma optimize ("awy", on) 

CMachineParameter const paraDirectionL = { pt_byte, "Left Direction", "Left Direction", 0x00, 0x02, 0xFF, MPF_STATE, 0x00 };
CMachineParameter const paraDirectionR = { pt_byte, "Right Direction", "Right Direction", 0x00, 0x02, 0xFF, MPF_STATE, 0x00 };
CMachineParameter const paraRate = { pt_word, "Frequency", "Frequency", 0, 0xFFFE, 0xFFFF, MPF_STATE, 0x0000 };
CMachineParameter const paraDry = { pt_word, "Dry", "Dry", 0, 0xFFFE, 0xFFFF, MPF_STATE, 0x7fff };
CMachineParameter const paraWet = { pt_word, "Wet", "Wet", 0, 0xFFFE, 0xFFFF, MPF_STATE, 0x7fff };

CMachineParameter const *pParameters[] =
{ 
	&paraRate, &paraDirectionL, &paraDirectionR 
};

CMachineAttribute const attrNonlinearity = { "Frequency non-linearity", 0, 10, 5 };
CMachineAttribute const attrMaxfreq = { "Max. frequency (Hz)", 20, 20000, 5000 };

CMachineAttribute const *pAttributes[] = 
{
	&attrNonlinearity, &attrMaxfreq 
};

#pragma pack(1)                        

class gvals
{
	public:
			word Rate;
			byte DirectionL;
			byte DirectionR;
};

class avals
{
	public:
		int nonlinearity;
		int maxfreq;
};

#pragma pack()
/////////////////////////////////////////////////////////////////////////////////////

#define miMACHINE_NAME "Bigyo FrequencyShifter"
#define miSHORT_NAME "FreqShift"
#define miMACHINE_AUTHOR "Marcin Dabrowski"
#define miVERSION "1.12"
#define miABOUTTXT1 "Marcin Dabrowski\n"  
#define miABOUTTXT2	"bigyo@wp.pl\n" 
#define miABOUTTXT miMACHINE_NAME " v" miVERSION "\n\nbuild: " __DATE__ "\n\n" miABOUTTXT1 "" miABOUTTXT2

CMachineInfo const MacInfo = 
{
	MT_EFFECT,				// Machine type
	MI_VERSION,				// Machine interface version
	MIF_DOES_INPUT_MIXING,	// Machine flags
	0,						// min tracks
	0,						// max tracks
	3,						// numGlobalParameters
	0,						// numTrackParameters
	pParameters,			// pointer to parameter stuff
	2,						// numAttributes
	pAttributes,			// pointer to attribute stuff
	miMACHINE_NAME,		// Full Machine Name
	miSHORT_NAME,		// Short name
	miMACHINE_AUTHOR,				// Author name
	"&About..."				// Right click menu commands
};

/////////////////////////////////////////////////////////////////////////////////////

class miex : public CMDKMachineInterfaceEx { };
class mi : public CMDKMachineInterface
{
public:
            mi();
            virtual ~mi();
            virtual void Tick();
            virtual void MDKInit(CMachineDataInput * const pi);
            virtual bool MDKWork(float *psamples, int numsamples, int const mode);
            virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
            virtual void Command(int const i);
            virtual void MDKSave(CMachineDataOutput * const po);
            virtual char const *DescribeValue(int const param, int const value);
            virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
            virtual void OutputModeChanged(bool stereo) {}
			virtual void AttributesChanged();

            miex ex;
			avals aval;
			gvals gval;

			inline float dB2lin(float dB)  {return powf(10.0f, dB/20.0f);}
			inline float lin2dB(float lin) {return 20.0f*log10f(lin);}
			inline float freq2omega(float freq) {return (float) (2.0f * PI * freq/pMasterInfo->SamplesPerSec);}
			inline float freq2rate(float freq) {return  (2.0f * (float) freq/pMasterInfo->SamplesPerSec);}
			inline float msec2samples(float msec) {return  (((float) pMasterInfo->SamplesPerSec) * msec * 0.001f) ;}
			inline float lin2log(float value,float minlin,float maxlin,float minlog,float maxlog) { return minlog * (float) pow (maxlog/minlog, (value-minlin) / (maxlin-minlin));}
			
			HilbertPair hL, hR;
			FastCosSin carrier;
			
			int dirL, dirR;

			float slope;
			float rate;
			float MaxRate;
};
/////////////////////////////////////////////////////////////////////////////////////
mi::mi()
{	
			GlobalVals = &gval;
			AttrVals = (int *)&aval;
}

mi::~mi() { }

void mi::MDKInit(CMachineDataInput * const pi)
{
	SetOutputMode( true ); // No mono sounds

}

void mi::AttributesChanged()
{
	MaxRate = (float) aval.maxfreq ;
	slope = powf( 0.5f, (float)aval.nonlinearity + 1.0f );
	float freq = (  rate / (float) paraRate.MaxValue ) * MaxRate ; 
	float omega = freq2omega( (float) linlog(freq , 0.0 , MaxRate , slope) ) ;
	carrier.setOmega( omega ); 
}

void mi::MDKSave(CMachineDataOutput * const po) { }

void mi::Tick()
{
	if (gval.Rate != paraRate.NoValue)
	{
		rate = gval.Rate;
		float freq = ( rate / (float) paraRate.MaxValue ) * MaxRate ; 
		float omega = freq2omega( (float) linlog(freq , 0.0 , MaxRate , slope) ) ;
		carrier.setOmega( omega ); 
	};

	if (gval.DirectionL != paraDirectionL.NoValue)
	{
		dirL =  gval.DirectionL  ;
	};

	if (gval.DirectionR != paraDirectionR.NoValue)
	{
		dirR =  gval.DirectionR  ;
	};
}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
            return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
            if (mode==WM_WRITE)
                        return false;
            if (mode==WM_NOIO)
                        return false;
            if (mode==WM_READ)                        // <thru>
                        return true;
			
            do
			{
				complex<float> c = carrier.process();

				if (dirL)
				{
					complex<float> l = hL.process(*psamples);
					*psamples++ = (dirL==1) ? c.re * l.re - c.im * l.im : c.re * l.re + c.im * l.im ;
				}
				else
					psamples++ ;
				if (dirR)
				{
					complex<float> r = hR.process(*psamples);
					*psamples++ = (dirR==1) ? c.re * r.re - c.im * r.im : c.re * r.re + c.im * r.im  ;
				}
				else
					psamples++ ;
            } while (--numsamples);

			return true;
}

void mi::Command(int const i)
{
            switch (i)
            {
	            case 0:
                        pCB->MessageBox(miABOUTTXT);
                        break;
	            default:
                        break;
            }
}

char const *mi::DescribeValue(int const param, int const value)
{
	int n;
	float v,v1;
            static char txt[16];
            switch(param)
            {
				case 0: //  Rate
					v = (float) linlog(  MaxRate * value / paraRate.MaxValue , 0.0f , MaxRate , slope ) ;
					v1 = (float) linlog(  MaxRate * (value+1) / paraRate.MaxValue   , 0.0f , MaxRate , slope ) ;
					n=(int) (1.0f-log10f( v1 - v )) ;
					if (n<0) n=0;
					sprintf	(txt,"%.*f Hz", n, 	v	);
					break;		

				case 1: // Dir L
				case 2: // Dir R
					switch(value)
					{	case 0: return("Off");
						case 1: return("Down");
						case 2: return("Up");
					}
				
				default:
					sprintf(txt,"%.2f %%",	(float) value / 65534.0f * 100.0f  );
            }
			return txt;
}

#pragma optimize ("", on) 

DLL_EXPORTS

