/*****************************************************************************

        GainFixer.cpp
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

#include	"basic/basic_fnc.h"
#include	"dsp/BasicMixing.h"
#include	"GainFixer.h"

#include	<cassert>



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



GainFixer::GainFixer ()
:	_gain_old (1)
,	_gain (1)
,	_ramp_len (0)
{
	set_sample_freq (44100);

	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		_rms_pre_arr [chn].set_analysis_time (RMS_ANALYSIS_TIME * 0.001f);
	}
}



void	GainFixer::set_sample_freq (float sample_freq)
{
	assert (sample_freq > 0);

	_sample_freq = sample_freq;
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		_rms_pre_arr [chn].set_sample_freq (sample_freq);
		_rms_post_arr [chn].set_sample_freq (sample_freq);
	}
}



float	GainFixer::analyse_block_pre (const float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn)
{
	assert (data_arr != 0);
	assert (nbr_spl > 0);
	assert (nbr_chn > 0);
	assert (nbr_chn <= MAX_NBR_CHN);

	return (analyse_block (_rms_pre_arr, data_arr, nbr_spl, nbr_chn));
}



float	GainFixer::analyse_block_post (const float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn)
{
	assert (data_arr != 0);
	assert (nbr_spl > 0);
	assert (nbr_chn > 0);
	assert (nbr_chn <= MAX_NBR_CHN);

	return (analyse_block (_rms_post_arr, data_arr, nbr_spl, nbr_chn));
}



void	GainFixer::process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn, float gain)
{
	assert (data_arr != 0);
	assert (nbr_spl > 0);
	assert (nbr_chn > 0);
	assert (nbr_chn <= MAX_NBR_CHN);

	if (gain != _gain)
	{
		_gain = gain;
		_ramp_len = 256;
	}

	long				pos = 0;
	if (_ramp_len > 0)
	{
		const long		work_len = basic::min (_ramp_len, nbr_spl);
		const float		ratio = float (work_len) / _ramp_len;
		const float		end_gain = _gain_old + (_gain - _gain_old) * ratio;

		for (int chn = 0; chn < nbr_chn; ++chn)
		{
			dsp::BasicMixing::copy_1_1_vlr (
				data_arr [chn],
				data_arr [chn],
				work_len,
				_gain_old,
				end_gain
			);
		}

		_gain_old = end_gain;
		pos += work_len;
		_ramp_len -= work_len;
	}

	if (pos < nbr_spl)
	{
		assert (_gain_old == _gain);
		const long		work_len = nbr_spl - pos;

		for (int chn = 0; chn < nbr_chn; ++chn)
		{
			dsp::BasicMixing::copy_1_1_v (
				data_arr [chn] + pos,
				data_arr [chn] + pos,
				work_len,
				_gain
			);
		}
	}
}



float	GainFixer::compute_default_gain (float pre_level, float post_level)
{
	float				gain = 0;

	if (! basic::is_null (post_level))
	{
		gain = basic::min (pre_level, LIMITER_LEVEL) / post_level;
	}

	return (gain);
}



void	GainFixer::clear_buffers ()
{
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		_rms_pre_arr [chn].clear_buffers ();
		_rms_post_arr [chn].clear_buffers ();
	}

	_gain_old = _gain;
	_ramp_len = 0;
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



float	GainFixer::analyse_block (RmsList &rms_list, const float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn)
{
	assert (data_arr != 0);
	assert (nbr_spl > 0);
	assert (nbr_chn > 0);
	assert (nbr_chn <= MAX_NBR_CHN);

	float				max_level = 0;
	for (int chn = 0; chn < nbr_chn; ++chn)
	{
		const float		level = rms_list [chn].analyse_block (data_arr [chn], nbr_spl);
		max_level = basic::max (level, max_level);
	}

	return (max_level);
}



const float	GainFixer::LIMITER_LEVEL = 0.75f;



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
