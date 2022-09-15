/*****************************************************************************

        BiquadS.cpp
        Copyright (c) 2002-2008 Laurent de Soras

--- Legal stuff ---

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*Tab=3***********************************************************************/



#if defined (_MSC_VER)
	#pragma warning (1 : 4130 4223 4705 4706)
	#pragma warning (4 : 4355 4786 4800)
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"basic/basic_def.h"
#include	"basic/basic_fnc.h"
#include	"dsp/BiquadS.h"

#include	<cmath>



namespace std { }

namespace dsp
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



BiquadS::BiquadS ()
:	_sample_freq (44100)
,	_f0 (1000)
{
	_s_eq_a [0] = 1;
	_s_eq_a [1] = 2;
	_s_eq_a [2] = 1;

	_s_eq_b [0] = _s_eq_a [0];
	_s_eq_b [1] = _s_eq_a [1];
	_s_eq_b [2] = _s_eq_a [2];
}



/*
==============================================================================
Name: transform_s_to_z
Description:
	Transforms the continous s-plane equation in a discrete z-plane equation
	which can be used directly to filter signal, using the bilinear transfrom.

        ---        k                  ---        -k
        \   a[k].s                    \   a'[k].z
        /__                           /__
H(s) = --------------   ==>   H(z) = ---------------
        ---       k                   ---        -k
        \   b[k].s                    \   b'[k].z
        /__                           /__

Throws: Nothing
==============================================================================
*/

void	BiquadS::transform_s_to_z ()
{
	using namespace std;
	using basic::is_null;
	using basic::PI;

	// s to z bilinear transform
	const double	inv_k = tan (_f0 * PI / _sample_freq);
	assert (! is_null (inv_k));
	const double	k = 1 / inv_k;
	const double	kk = k*k;

	const double	b1k = _s_eq_b [1] * k;
	const double	b2kk = _s_eq_b [2] * kk;
	const double	b2kk_plus_b0 = b2kk + _s_eq_b [0];
	const double	b0z = b2kk_plus_b0 + b1k;
	const double	b2z = b2kk_plus_b0 - b1k;
	const double	b1z = 2 * (_s_eq_b [0] - b2kk);

	const double	a1k = _s_eq_a [1] * k;
	const double	a2kk = _s_eq_a [2] * kk;
	const double	a2kk_plus_a0 = a2kk + _s_eq_a [0];
	const double	a0z = a2kk_plus_a0 + a1k;
	const double	a2z = a2kk_plus_a0 - a1k;
	const double	a1z = 2 * (_s_eq_a [0] - a2kk);

	// IIR coefficients
	assert (! is_null (a0z));
	const double	mult = 1 / a0z;

	_z_eq_b [0] = float (b0z * mult);
	_z_eq_b [1] = float (b1z * mult);
	_z_eq_b [2] = float (b2z * mult);

	_z_eq_a [0] = 1;
	_z_eq_a [1] = float (a1z * mult);
	_z_eq_a [2] = float (a2z * mult);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace dsp



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
