// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

#ifndef _denormals_
#define _denormals_
#include "math.h"

#ifdef FP_SUBNORMAL
#define undenormalise(sample) if(!isnormal(sample)) sample=0.0f
#else
#define undenormalise(sample) if(((*(unsigned int*)((void*)&sample))&0x7f800000)==0) sample=0.0f
#endif

#endif//_denormals_

//ends
