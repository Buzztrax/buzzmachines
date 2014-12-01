// Marcin Dabrowski
// bigyo@wp.pl
// www.patafonia.co.nr

#ifndef __FASTCOSSIN_H
#define __FASTCOSSIN_H
///////////////////////////////////////////////////////////////////
#include "ComplexFloat.h"
///////////////////////////////////////////////////////////////////
class FastCosSin            
{
public:
	FastCosSin();
	virtual ~FastCosSin();
	void setOmega(float omega);	
	void setPhase(float phase);	
	inline complex<float> process();

private:
		complex<double> z, c;
};
///////////////////////////////////////////////////////////////////
FastCosSin::FastCosSin()
{
	z = complex<double>(1.0, 0.0);
}

FastCosSin::~FastCosSin()
{}

void FastCosSin::setOmega(float omega) 
{
	c = complex<double>( cos(omega), sin(omega)  ); 
}

void FastCosSin::setPhase(float phase) 
{
	z = complex<double>( cos(phase), sin(phase) );
}

inline complex<float> FastCosSin::process() 
{
	double z1re = z.re;
	double z1im = z.im;
	z.re = c.re * z1re - c.im * z1im ;
	z.im = c.im * z1re + c.re * z1im ;
	return complex<float>(z1re, z1im);
}
///////////////////////////////////////////////////////////////////
#endif
