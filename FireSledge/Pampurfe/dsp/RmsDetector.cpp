/*****************************************************************************

        RmsDetector.cpp
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
#include	"dsp/RmsDetector.h"

#include	<cassert>
#include	<cmath>



namespace dsp
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



RmsDetector::RmsDetector ()
:	_fs (44100)
,	_t (0.030f)
{
	update_param ();
	clear_buffers ();
}



void	RmsDetector::set_sample_freq (float fs)
{
	assert (_fs > 0);

	_fs = fs;
	update_param ();
}



void	RmsDetector::set_analysis_time (float t)
{
	assert (t > 2.0 / _fs);

	_t = t;
	update_param ();
}



float	RmsDetector::analyse_block (const float *spl_ptr, long nbr_spl)
{
	assert (nbr_spl > 0);

	float				mem_x = _mem_x / _coef_x;
	float				y = _mem_y / _coef_x;

	do
	{
		float		x = *spl_ptr;
		x *= x;
		y = x + mem_x - _coef_y * y;
		mem_x = x;

		++ spl_ptr;
		-- nbr_spl;
	}
	while (nbr_spl > 0);

	_mem_x = mem_x * _coef_x;
	_mem_y = y * _coef_x;

	using namespace std;

	return (float (sqrt (fabs (_mem_y))));
}



void	RmsDetector::clear_buffers ()
{
	_mem_x = 0;
	_mem_y = 0;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	RmsDetector::update_param ()
{
	using namespace std;

	assert (_fs * _t > 2);

	// s to z bilinear transform
	const double	k = 1 / tan (basic::PI / (_fs * _t));

	const double	a1z = 1 - k;
	const double	a0z = 1 + k;

	// IIR coefficients
	const double	mult = 1 / a0z;

	_coef_x = float (mult);
	_coef_y = float (a1z * mult);
}



}	// namespace dsp



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
