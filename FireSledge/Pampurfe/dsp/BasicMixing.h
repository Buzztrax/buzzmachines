/*****************************************************************************

        BasicMixing.h
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



#if ! defined (dsp_BasicMixing_HEADER_INCLUDED)
#define	dsp_BasicMixing_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace dsp
{



class BasicMixing
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	static void		copy_1_1_v (float out_ptr [], const float in_ptr [], long nbr_spl, float vol);
	static void		copy_1_1_vlr (float out_ptr [], const float in_ptr [], long nbr_spl, float s_vol, float e_vol);
	static void		copy_2_2i (float out_ptr [], const float in_1_ptr [], const float in_2_ptr [], long nbr_spl);
	static void		copy_2i_2 (float out_1_ptr [], float out_2_ptr [], const float in_ptr [], long nbr_spl);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						BasicMixing ();
						BasicMixing (const BasicMixing &other);
	BasicMixing &	operator = (const BasicMixing &other);
	bool				operator == (const BasicMixing &other);
	bool				operator != (const BasicMixing &other);

};	// class BasicMixing



}	// namespace dsp



//#include	"dsp/BasicMixing.hpp"



#endif	// dsp_BasicMixing_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
