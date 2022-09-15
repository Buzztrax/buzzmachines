/*****************************************************************************

        InterpFncFiniteAsym.hpp
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



#if ! defined (dsp_InterpFncFiniteAsym_CODEHEADER_INCLUDED)
#define	dsp_InterpFncFiniteAsym_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	<cassert>
#include	<cmath>



namespace dsp
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int LBOUND, int UBOUND, class GenFtor>
InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::InterpFncFiniteAsym ()
{
	assert (LENGTH > 0);

	init_coef ();
}



template <int LBOUND, int UBOUND, class GenFtor>
void	InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::init_coef ()
{
	if (! coef_init_flag)
	{
		double			y [LENGTH + 1];
		double			slope [LENGTH + 1];
		int				pos;
		GenFtor			fnc;

		for (pos = 0; pos <= LENGTH; ++pos)
		{
			const double	x = pos + LBOUND;

			y [pos] = double (fnc (x));
			slope [pos] =   (  double (fnc (x + 1.0/2048))
			                 - double (fnc (x - 1.0/2048)))
			              * 1024;
		}

		val_minus_inf = float (y [0]);
		val_plus_inf = float (y [LENGTH]);

		for (pos = 0; pos < LENGTH; ++pos)
		{
			const int		x = pos + LBOUND;
			const double	p = slope [pos  ];
			const double	q = slope [pos+1];
			const double	k = y [pos+1] - y [pos];

			// Calcul des coefs en [0 ; 1]
			const double	A =  q +   p - 2*k;
			const double	B = -q - 2*p + 3*k;
			const double	C = p;
			const double	D = y [pos];

			// Translation en [x ; x+1]
			const double	a =    A;
			const double	b = -3*A*x + B;
			const double	c = (3*A*x - B*2) * x + C;
			const double	d = ((-A*x + B)   * x - C) * x + D;

			coef [pos] [0] = float (d);
			coef [pos] [1] = float (c);
			coef [pos] [2] = float (b);
			coef [pos] [3] = float (a);
		}

		coef_init_flag = true;
	}
}



template <int LBOUND, int UBOUND, class GenFtor>
float	InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::operator () (float x) const
{

	if (x <= LBOUND)
	{
		return (val_minus_inf);
	}

	else if (x >= UBOUND)
	{
		return (val_plus_inf);
	}

#if defined (WIN32) && defined (_MSC_VER)

	// Important: FPU rounding mode should be set to "to closest integer".

	static const float	round = (-LBOUND) - 0.5f;
	int				pos;

	__asm
	{
		fld			x
		fadd			round
		fistp			pos
	}

#else

	int				pos = int (x - (LBOUND - 1)) - 1;	// Rounding toward -oo

#endif

	return (       coef [pos] [0]
	        + x * (coef [pos] [1]
	        + x * (coef [pos] [2]
	        + x *  coef [pos] [3])));
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int LBOUND, int UBOUND, class GenFtor>
float	InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::coef [LENGTH] [4];

template <int LBOUND, int UBOUND, class GenFtor>
float	InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::val_minus_inf;

template <int LBOUND, int UBOUND, class GenFtor>
float	InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::val_plus_inf;

template <int LBOUND, int UBOUND, class GenFtor>
bool	InterpFncFiniteAsym <LBOUND, UBOUND, GenFtor>::coef_init_flag = false;



}	// namespace dsp



#endif	// dsp_InterpFncFiniteAsym_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
