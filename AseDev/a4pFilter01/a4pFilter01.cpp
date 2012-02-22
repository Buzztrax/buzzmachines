#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <MachineInterface.h>

#define MAX_TAPS		1
#define	FILTER_SECTIONS	2					/* 2 filter sections for 24 db/oct filter */

typedef struct 
{
    unsigned int length;					/* size of filter */
    float *history;							/* pointer to history in filter */
    double *coef;							/* pointer to coefficients of filter */
} FILTER;

typedef struct 
{
	double a0, a1, a2;						/* numerator coefficients */
	double b0, b1, b2;						/* denominator coefficients */
} BIQUAD;

BIQUAD ProtoCoef[FILTER_SECTIONS];			/* Filter prototype coefficients,
											1 for each filter section */

void prewarp(
    double *a0, double *a1, double *a2,
    double fc, double fs)
{
    double wp, pi;

    pi = (4.0 * atan(1.0));
    wp = (2.0 * fs * tan(pi * fc / fs));

    *a2 = (*a2) / (wp * wp);
    *a1 = (*a1) / wp;
}

void bilinear(
    double a0, double a1, double a2,	/* numerator coefficients */
    double b0, double b1, double b2,	/* denominator coefficients */
    double *k,           /* overall gain factor */
    double fs,           /* sampling rate */
    double *coef         /* pointer to 4 iir coefficients */
)
{
    double ad, bd;
    ad = (4. * a2 * fs * fs + 2. * a1 * fs + a0);
    bd = (4. * b2 * fs * fs + 2. * b1* fs + b0);
    *k *= ad/bd;
    *coef++ = ((2. * b0 - 8. * b2 * fs * fs) / bd);			/* beta1 */
    *coef++ = ((4. * b2 * fs * fs - 2. * b1 * fs + b0) / bd); /* beta2 */
    *coef++ = ((2. * a0 - 8. * a2 * fs * fs) / ad);			/* alpha1 */
    *coef = ((4. * a2 * fs * fs - 2. * a1 * fs + a0) / ad);	/* alpha2 */
}

void szxform(
    double *a0, double *a1, double *a2,     /* numerator coefficients */
    double *b0, double *b1, double *b2,		/* denominator coefficients */
    double fc,								/* Filter cutoff frequency */
    double fs,								/* sampling rate */
    double *k,								/* overall gain factor */
    double *coef)							/* pointer to 4 iir coefficients */
{
	prewarp(a0, a1, a2, fc, fs);
	prewarp(b0, b1, b2, fc, fs);
	bilinear(*a0, *a1, *a2, *b0, *b1, *b2, k, fs, coef);
}

CMachineParameter const paraCutoff = 
{ 
	pt_word,								// type
	"Cutoff",
	"Cutoff in Hz",							// description
	1,										// MinValue	
	22050,									// MaxValue
	65535,									// NoValue
	MPF_STATE,								// Flags
	5000
};

CMachineParameter const paraResonance = 
{ 
	pt_word,								// type
	"Resonance",
	"Resonance in units",					// description
	10,										// MinValue	
	10000,									// MaxValue
	65535,									// NoValue
	MPF_STATE,								// Flags
	10
};

CMachineParameter const *pParameters[] = 
{ 
	&paraCutoff,
	&paraResonance,
};

#pragma pack(1)

class gvals
{
public:
	word cutoff;
	word resonance;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_EFFECT,								// type
	MI_VERSION,
	0,										// flags
	0,										// min tracks
	0,										// max tracks
	2,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0,
	NULL,
#ifdef _DEBUG
	"asedev a4pFilter01 (Debug build)",			// name
#else
	"asedev a4pFilter01",
#endif
	"a4pFilter",								// short name
	"ase development",						// author
	NULL
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);

	virtual char const *DescribeValue(int const param, int const value);


private:

	float iir_filter(float input);

private:
	FILTER	iir;
	float	Cutoff;			
	float	Resonance;	

private:

	gvals gval;

};

DLL_EXPORTS

mi::mi()
{
	AttrVals = NULL;
	GlobalVals = &gval;
	TrackVals = NULL;
}

mi::~mi()
{
	delete[] iir.coef;
	delete[] iir.history;
}

char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 0:	
		sprintf(txt, "%.0fHz", (float)(value));
		break;
	case 1:
		sprintf(txt, "%.1f", (float)(value / 10.0));
		break;
	default:
		return NULL;
	}

	return txt;
}

void mi::Init(CMachineDataInput * const pi)
{
	/* Section 1 */		
	ProtoCoef[0].a0 = 1.0;
	ProtoCoef[0].a1 = 0;
	ProtoCoef[0].a2 = 0;
	ProtoCoef[0].b0 = 1.0;
	ProtoCoef[0].b1 = 0.765367;
	ProtoCoef[0].b2 = 1.0;
	/* Section 2 */		
	ProtoCoef[1].a0 = 1.0;
	ProtoCoef[1].a1 = 0;
	ProtoCoef[1].a2 = 0;
	ProtoCoef[1].b0 = 1.0;
	ProtoCoef[1].b1 = 1.847759;
	ProtoCoef[1].b2 = 1.0;

	iir.length = FILTER_SECTIONS;		/* Number of filter sections */

	//allocate memory
	iir.coef = (double *) calloc(4 * iir.length + 1, sizeof(double));
	iir.history = (float *) calloc(2*iir.length,sizeof(float));

	Resonance = 1.0f;						/* preset Resonance */
	Cutoff = 5000;						/* preset Filter cutoff (Hz) */

}

void mi::Tick()
{
	double		*coef;
	unsigned	nInd;
	double		a0, a1, a2, b0, b1, b2, k;
	bool		doit;

	doit = false;

	if (gval.cutoff != paraCutoff.NoValue)
	{
		Cutoff = (float)(gval.cutoff);
		doit = true;
	}
	if (gval.resonance != paraResonance.NoValue)
	{
		Resonance = (float)((gval.resonance) / 10.0);
		doit = true;
	}

	if (doit==true)
	{
		k = 1.0;
		coef = iir.coef + 1;	
		for (nInd = 0; nInd < iir.length; nInd++)
		{
			a0 = ProtoCoef[nInd].a0;
			a1 = ProtoCoef[nInd].a1;
			a2 = ProtoCoef[nInd].a2;
			b0 = ProtoCoef[nInd].b0;
			b1 = ProtoCoef[nInd].b1 / Resonance;	
			b2 = ProtoCoef[nInd].b2;
			szxform(&a0, &a1, &a2, &b0, &b1, &b2, Cutoff, pMasterInfo->SamplesPerSec, &k, coef);
			coef += 4;							
		}
		iir.coef[0] = k;
	}
}

/*
 * --------------------------------------------------------------------
 * 
 * iir_filter - Perform IIR filtering sample by sample on floats
 * 
 * Implements cascaded direct form II second order sections.
 * Requires FILTER structure for history and coefficients.
 * The length in the filter structure specifies the number of sections.
 * The size of the history array is 2*iir->length.
 * The size of the coefficient array is 4*iir->length + 1 because
 * the first coefficient is the overall scale factor for the filter.
 * Returns one output sample for each input sample.  Allocates history
 * array if not previously allocated.
 * 
 * float iir_filter(float input,FILTER *iir)
 * 
 *     float input        new float input sample
 *     FILTER *iir        pointer to FILTER structure
 * 
 * Returns float value giving the current output.
 * 
 * Allocation errors cause an error message and a call to exit.
 * --------------------------------------------------------------------
 */
//float iir_filter(float input,FILTER *iir)
float mi::iir_filter(float input)
{
    unsigned int i;
    float *hist1_ptr,*hist2_ptr;
	double *coef_ptr;
    float output,new_hist,history1,history2;

    coef_ptr = iir.coef;                /* coefficient pointer */

    hist1_ptr = iir.history;            /* first history */
    hist2_ptr = hist1_ptr + 1;           /* next history */

    output =(float) (input * (*coef_ptr++));

    for (i = 0 ; i < iir.length; i++)
	{
        history1 = *hist1_ptr;							/* history values */
        history2 = *hist2_ptr;

        output = (float) (output - history1 * (*coef_ptr++));
        new_hist = (float) (output - history2 * (*coef_ptr++));    /* poles */

        output = (float) (new_hist + history1 * (*coef_ptr++));
        output = (float) (output + history2 * (*coef_ptr++));      /* zeros */

        *hist2_ptr++ = *hist1_ptr;
        *hist1_ptr++ = new_hist;
        hist1_ptr++;
        hist2_ptr++;
    }

    return(output);
}

bool mi::Work(float *psamples, int numsamples, int const mode)
{
	if (mode == WM_WRITE || mode == WM_NOIO)
		return false;
	
	if (mode == WM_READ)
		return true;

	do 
	{
		*psamples = iir_filter(*psamples);
		psamples++;
	} while(--numsamples);

	return true;
}


