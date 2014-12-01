// Marcin Dabrowski
// bigyo@wp.pl
// www.patafonia.co.nr

#ifndef __ALLPASS2_H
#define __ALLPASS2_H
///////////////////////////////////////////////////////////////////////////

#include "denormals.h"

///////////////////////////////////////////////////////////////////////////

class Allpass2
{
public:
	Allpass2();
	~Allpass2();
	void setFeedback(float g);
	inline float process(float in);
	inline void processSamples(float * puot, int numsamples);

private:
	float g, x1, x2, y1, y2;
};

///////////////////////////////////////////////////////////////////////////

Allpass2 :: Allpass2()
{
	g = x1 = x2 = y1 = y2 = 0.0f ;
}

Allpass2 :: ~Allpass2()
{
}

inline float Allpass2 :: process(float in)
{
	float out = g * (in + y2)  - x2;	

	undenormalise(out);

	x2 = x1 ; 
	x1 = in ;
	y2 = y1 ;
	y1 = out ;

	return out;
}

inline void Allpass2::processSamples(float *pout, int numsamples)
{
		do 
		{
			float const in = *pout;
			float out = g * (in + y2)  - x2;	
			undenormalise(out);
			x2 = x1 ; 
			x1 = in ;
			y2 = y1 ;
			y1 = out ;
			*(pout++) = out;
		} while(--numsamples);
}

void Allpass2::setFeedback(float feedback)
{
	g = feedback; 
}
///////////////////////////////////////////////////////////////////////////
#endif 
 