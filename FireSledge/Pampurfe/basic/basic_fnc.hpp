/*****************************************************************************

        basic_fnc.hpp
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



#if ! defined (basic_basic_fnc_CODEHEADER_INCLUDED)
#define	basic_basic_fnc_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"basic/basic_def.h"

#include	<cassert>
#include	<climits>
#include	<cmath>



namespace std {};

namespace basic
{



template <class T>
T	max (T a, T b)
{
	return (a >= b ? a : b);
}



template <class T>
T	min (T a, T b)
{
	return (a <= b ? a : b);
}



template <class T>
T	limit (T a, T b, T c)
{
	assert (b <= c);

	return (max (min (a, c), b));
}



bool	is_null (double x)
{
	using namespace std;

	return (fabs (x) < EPSILON);
}



int	round_int (double x)
{
	assert (x <= double (INT_MAX));
	assert (x >= double (INT_MIN));

	return (int (floor (x + 0.5)));
}



double	sinc (double x)
{
	if (x == 0)
	{
		return (1);
	}

	using namespace std;

	const double	pi_x = x * basic::PI;

	return (sin (pi_x) / pi_x);
}



}	// namespace basic



#endif	// basic_basic_fnc_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
