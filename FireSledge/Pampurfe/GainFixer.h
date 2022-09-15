/*****************************************************************************

        GainFixer.h
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



#if ! defined (GainFixer_HEADER_INCLUDED)
#define	GainFixer_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"basic/Array.h"
#include	"dsp/RmsDetector.h"



class GainFixer
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {			MAX_NBR_CHN				= 2		};
	enum {			RMS_ANALYSIS_TIME		= 50		};	// ms

						GainFixer ();
	virtual			~GainFixer () {}

	void				set_sample_freq (float sample_freq);

	float				analyse_block_pre (const float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn);
	float				analyse_block_post (const float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn);
	void				process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn, float gain);
	float				compute_default_gain (float pre_level, float post_level);

	void				clear_buffers ();



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	typedef	basic::Array <dsp::RmsDetector, MAX_NBR_CHN>	RmsList;

	float				analyse_block (RmsList &rms_list, const float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn);

	RmsList			_rms_pre_arr;
	RmsList			_rms_post_arr;
	float				_sample_freq;
	float				_gain_old;
	float				_gain;
	long				_ramp_len;

	static const float
						LIMITER_LEVEL;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						GainFixer (const GainFixer &other);
	GainFixer &		operator = (const GainFixer &other);
	bool				operator == (const GainFixer &other);
	bool				operator != (const GainFixer &other);

};	// class GainFixer



//#include	"GainFixer.hpp"



#endif	// GainFixer_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
