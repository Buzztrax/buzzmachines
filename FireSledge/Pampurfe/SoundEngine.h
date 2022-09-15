/*****************************************************************************

        SoundEngine.h
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



#if ! defined (SoundEngine_HEADER_INCLUDED)
#define	SoundEngine_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"Distorter.h"
#include	"GainFixer.h"
#include	"ToneFilter.h"



class SoundEngine
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {			MAX_NBR_CHN		= 2	};

	explicit			SoundEngine (float sample_scale);
	virtual			~SoundEngine () {}

	void				set_sample_freq (float freq);
	void				set_gain (float gain);
	void				set_shape (Distorter::Shape shape);
	void				set_compensation (float compensation);
	void				set_tone (float tone);
	void				set_freq (float freq);
	void				process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	const float		_sample_scale;
	float				_sample_freq;	// Hz, > 0
	float				_compensation;
	Distorter		_distorter;
	GainFixer		_gain_fixer;
	ToneFilter		_tone_filter;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						SoundEngine ();
						SoundEngine (const SoundEngine &other);
	SoundEngine &		operator = (const SoundEngine &other);
	bool				operator == (const SoundEngine &other);
	bool				operator != (const SoundEngine &other);

};	// class SoundEngine



//#include	"SoundEngine.hpp"



#endif	// SoundEngine_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
