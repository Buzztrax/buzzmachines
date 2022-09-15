/*****************************************************************************

        SoundEngine.cpp
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

#include	"SoundEngine.h"

#include	<cassert>



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



SoundEngine::SoundEngine (float sample_scale)
:	_sample_scale (sample_scale)
,	_sample_freq (-1)
,	_compensation (0)
{
	assert (sample_scale > 0);

	set_sample_freq (44100);
}



void	SoundEngine::set_sample_freq (float sample_freq)
{
	assert (sample_freq > 0);

	_sample_freq = sample_freq;
	_distorter.set_sample_freq (_sample_freq);
	_gain_fixer.set_sample_freq (_sample_freq);
	_tone_filter.set_sample_freq (_sample_freq);
}



void	SoundEngine::set_gain (float gain)
{
	_distorter.set_gain (gain / _sample_scale);
}



void	SoundEngine::set_shape (Distorter::Shape shape)
{
	_distorter.set_shape (shape);
}



void	SoundEngine::set_compensation (float compensation)
{
	_compensation = compensation;
}



void	SoundEngine::set_tone (float tone)
{
	_tone_filter.set_tone (tone);
}



void	SoundEngine::set_freq (float freq)
{
	_tone_filter.set_freq (freq);
}



void	SoundEngine::process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn)
{
	const float **	data_arr_const = const_cast <const float **> (data_arr);

	float				level_pre = 1;
	if (_compensation > 0)
	{
		level_pre = _gain_fixer.analyse_block_pre (
			data_arr_const,
			nbr_spl,
			nbr_chn
		);
		level_pre /= _sample_scale;
	}

	_distorter.process_block (data_arr, nbr_spl, nbr_chn);

	float				comp_gain = 1;
	if (_compensation > 0)
	{
		const float		level_post = _gain_fixer.analyse_block_post (
			data_arr_const,
			nbr_spl,
			nbr_chn
		);

		float			fake_level_post = 0;
		if (! basic::is_null (level_post))
		{
			const float	pre_log = float (log (level_pre));
			const float	post_log = float (log (level_post));
			const float	fake_post_log =
				pre_log + (post_log - pre_log) * _compensation;
			fake_level_post = float (exp (fake_post_log));
		}

		comp_gain = _gain_fixer.compute_default_gain (level_pre, fake_level_post);
	}

	const float		final_gain = comp_gain * _sample_scale;

	_gain_fixer.process_block (data_arr, nbr_spl, nbr_chn, final_gain);

	_tone_filter.process_block (data_arr, nbr_spl, nbr_chn);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
