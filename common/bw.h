#ifndef __BUZZ_DSPLIB_BW_H
#define __BUZZ_DSPLIB_BW_H

class CBWState
{
public:
	float a[5];		// coefficients
	float i[2];		// past inputs
	float o[2];		// past outputs
	float ri[2];	// past right inputs (for stereo mode)
	float ro[2];	// past right outputs 
	int IdleCount;
};

// work modes
#define WM_NOIO                                 0
#define WM_READ                                 1
#define WM_WRITE                                2
#define WM_READWRITE							3

#define BW_SETTLE_TIME		256		

#endif
