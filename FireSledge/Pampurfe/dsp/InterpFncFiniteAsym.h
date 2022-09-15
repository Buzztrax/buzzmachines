/*****************************************************************************

        InterpFncFiniteAsym.h
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



#if ! defined (dsp_InterpFncFiniteAsym_HEADER_INCLUDED)
#define	dsp_InterpFncFiniteAsym_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace dsp
{



template <int LBOUND, int UBOUND, class GenFtor>
class InterpFncFiniteAsym
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {			LENGTH			= UBOUND - LBOUND	};

						InterpFncFiniteAsym ();

	static void		init_coef ();

	float inline	operator () (float x) const;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	static float	coef [LENGTH] [4];
	static float	val_minus_inf;
	static float	val_plus_inf;
	static bool		coef_init_flag;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						InterpFncFiniteAsym (const InterpFncFiniteAsym &other);
	InterpFncFiniteAsym &		operator = (const InterpFncFiniteAsym &other);
	bool				operator == (const InterpFncFiniteAsym &other);
	bool				operator != (const InterpFncFiniteAsym &other);

};	// class InterpFncFiniteAsym



}	// namespace dsp



#include	"dsp/InterpFncFiniteAsym.hpp"



#endif	// dsp_InterpFncFiniteAsym_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
