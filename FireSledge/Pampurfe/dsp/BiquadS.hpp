/*****************************************************************************

        BiquadS.hpp
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



#if ! defined (dsp_BiquadS_CODEHEADER_INCLUDED)
#define	dsp_BiquadS_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	<cassert>



namespace dsp
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	BiquadS::copy_filter (const BiquadS &other)
{
	Inherited::copy_filter (other);

	_sample_freq = other._sample_freq;
	_f0 = other._f0;
	set_s_eq (other._s_eq_b, other._s_eq_a);
}



/*
==============================================================================
Name: set_sample_freq
Description:
	Sets the filter sampling rate.
Input parameters:
	- fs: Sampling frequency in Hz, > 0
Throws: Nothing
==============================================================================
*/

void	BiquadS::set_sample_freq (float fs)
{
	assert (fs > 0);

	_sample_freq = fs;
}



/*
==============================================================================
Name: set_freq
Description:
	Sets the filter cutoff. transform_s_to_z() must be called for the change
	to be effective.
Input parameters:
	- f0: Sampling frequency in Hz, > 0.
Throws: Nothing
==============================================================================
*/

void	BiquadS::set_freq (float f0)
{
	assert (f0 > 0);

	_f0 = f0;
}



void	BiquadS::set_s_eq (const float b [3], const float a [3])
{
	assert (a != 0);
	assert (a [2] != 0);
	assert (b != 0);

	_s_eq_b [0] = b [0];
	_s_eq_b [1] = b [1];
	_s_eq_b [2] = b [2];

	_s_eq_a [0] = a [0];
	_s_eq_a [1] = a [1];
	_s_eq_a [2] = a [2];
}



void	BiquadS::set_s_eq (const double b [3], const double a [3])
{
	assert (a != 0);
	assert (a [2] != 0);
	assert (b != 0);

	_s_eq_b [0] = float (b [0]);
	_s_eq_b [1] = float (b [1]);
	_s_eq_b [2] = float (b [2]);

	_s_eq_a [0] = float (a [0]);
	_s_eq_a [1] = float (a [1]);
	_s_eq_a [2] = float (a [2]);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace dsp



#endif	// dsp_BiquadS_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
