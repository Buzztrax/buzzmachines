#define	FILTER_SECTIONS	2					/* 2 filter sections for 24 db/oct filter */

typedef struct 
{
    unsigned int length;					/* size of filter */
    float *history;							/* pointer to history in filter */
    double *coef;							/* pointer to coefficients of filter */
	float last_cutoff;
	float last_res;
} FILTER;

typedef struct 
{
	double a0, a1, a2;						/* numerator coefficients */
	double b0, b1, b2;						/* denominator coefficients */
} BIQUAD;

BIQUAD ProtoCoef[FILTER_SECTIONS];			/* Filter prototype coefficients,
											1 for each filter section */
inline void prewarp(
    double *a0, double *a1, double *a2,
    double fc, double fs);

inline void bilinear(
    double a0, double a1, double a2,	/* numerator coefficients */
    double b0, double b1, double b2,	/* denominator coefficients */
    double *k,           /* overall gain factor */
    double fs,           /* sampling rate */
    double *coef         /* pointer to 4 iir coefficients */
);

inline void szxform(
    double *a0, double *a1, double *a2,     /* numerator coefficients */
    double *b0, double *b1, double *b2,		/* denominator coefficients */
    double fc,								/* Filter cutoff frequency */
    double fs,								/* sampling rate */
    double *k,								/* overall gain factor */
    double *coef);