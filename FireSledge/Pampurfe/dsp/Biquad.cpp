/*****************************************************************************

        Biquad.cpp
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

#include	"dsp/Biquad.h"

#include	<cassert>



namespace dsp
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: ctor
Description:
	Builds the filter, ready to work (transfer function is 1)
Throws: Nothing
==============================================================================
*/

Biquad::Biquad ()
{
	_z_eq_b [0] = 1;
	_z_eq_b [1] = 0;
	_z_eq_b [2] = 0;
	_z_eq_a [0] = 1;
	_z_eq_a [1] = 0;
	_z_eq_a [2] = 0;

	clear_buffers ();
}



/*
==============================================================================
Name: process_block
Description:
	Filters a block of samples.
Input/Output parameters:
	- spl_ptr: Sample block to be filtered
	- nbr_spl: Number of samples to process. >= 0
Throws: Nothing
==============================================================================
*/

void	Biquad::process_block (float *spl_ptr, long nbr_spl)
{
	long				pos = 0;
	do
	{
		spl_ptr [pos] = process_sample (spl_ptr [pos]);
		++ pos;
	}
	while (pos < nbr_spl);
}



/*
==============================================================================
Name: clear_buffers
Description:
	Clears all filter buffers.
Throws: Nothing
==============================================================================
*/

void	Biquad::clear_buffers ()
{
	_mem_x [0] = 0;
	_mem_x [1] = 0;
	_mem_y [0] = 0;
	_mem_y [1] = 0;
	_mem_pos = 0;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace dsp



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
