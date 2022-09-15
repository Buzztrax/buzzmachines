/*****************************************************************************

        ToneFilter.h
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



#if ! defined (ToneFilter_HEADER_INCLUDED)
#define	ToneFilter_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"basic/Array.h"
#include	"dsp/BiquadS.h"



class ToneFilter
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {			MAX_NBR_CHN				= 2		};

						ToneFilter ();
	virtual			~ToneFilter () {}

	void				set_sample_freq (float sample_freq);
	void				set_freq (float freq);
	void				set_tone (float tone);
	void				process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn);
	void				clear_buffers ();



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	typedef	basic::Array <dsp::BiquadS, 4>	FilterGroup;

	void				update_filter ();

	float				_sample_freq;
	float				_freq;				// Hz, > 0
	float				_tone;				// %
	basic::Array <FilterGroup, MAX_NBR_CHN>
						_filter_arr;
	bool				_filter_flag;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						ToneFilter (const ToneFilter &other);
	ToneFilter &	operator = (const ToneFilter &other);
	bool				operator == (const ToneFilter &other);
	bool				operator != (const ToneFilter &other);

};	// class ToneFilter



//#include	"ToneFilter.hpp"



#endif	// ToneFilter_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
