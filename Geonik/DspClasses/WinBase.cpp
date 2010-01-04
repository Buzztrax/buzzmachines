/*
 *		Dsp Classes : Windows Base helpers
 *
 *			Written by George Nicolaidis aka Geonik
 */

#include "../DspClasses/DspClasses.h"

#include <windows.h>
#include <winbase.h>


//	Return the Pentium clock counter

__int64 DspGetCpuTickCount() {

	LARGE_INTEGER d; QueryPerformanceCounter(&d); return d.QuadPart; }
