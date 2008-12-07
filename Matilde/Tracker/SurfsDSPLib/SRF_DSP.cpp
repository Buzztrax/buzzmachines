#include	"SRF_DSP.h"

using namespace SurfDSPLib;

void	SurfDSPLib::ZeroFloat( float *p, int iCount )
{
	while( iCount-- )
	{
		*p++ = 0.0f;
	}
}

