/*****************************************************************************

        ToneFilter.cpp
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
#include	"ToneFilter.h"

#include	<cassert>
#include	<cmath>



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



ToneFilter::ToneFilter ()
:	_sample_freq (44100)
,	_freq (4000)
,	_tone (0)
,	_filter_flag (true)
{
	set_sample_freq (44100);
	update_filter ();
	clear_buffers ();
}



void	ToneFilter::set_sample_freq (float sample_freq)
{
	assert (sample_freq > 0);

	_sample_freq = sample_freq;
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		for (int biq = 0; biq < _filter_arr [chn].size (); ++biq)
		{
			_filter_arr [chn] [biq].set_sample_freq (_sample_freq);
		}
	}
	update_filter ();
}



void	ToneFilter::set_freq (float freq)
{
	assert (freq > 0);

	_freq = freq;
	update_filter ();
}



void	ToneFilter::set_tone (float tone)
{
	assert (tone >= 0);
	assert (tone <= 1);

	_tone = tone;
	update_filter ();
}



void	ToneFilter::process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn)
{
	if (_filter_flag)
	{
		for (int chn = 0; chn < nbr_chn; ++chn)
		{
			for (int biq = 0; biq < _filter_arr [chn].size (); ++biq)
			{
				_filter_arr [chn] [biq].process_block (&data_arr [chn] [0], nbr_spl);
			}
		}
	}
}



void	ToneFilter::clear_buffers ()
{
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		for (int biq = 0; biq < _filter_arr [chn].size (); ++biq)
		{
			_filter_arr [chn] [biq].clear_buffers ();
		}
	}
}



void	ToneFilter::update_filter ()
{
	// Filter is deactivated if its cutoff frequency is (almost) above the
	// Nyquist frequency, or if the filter is flat.
	const float		max_filter_freq = _sample_freq * (19.0f/40);
	const bool		old_filter_flag = _filter_flag;
	_filter_flag =   (_freq < max_filter_freq)
	               & ! basic::is_null (_tone);

	if (_filter_flag)
	{
		const int		nbr_biq = _filter_arr [0].size ();

		double			lvl_pole;
		double			q;
		float				a1_ratio;	// Fades from '2' to the regular Butterworth coefficients
		float				freq_ratio;	// Frequency ratio between two consecutive biquads
		float				freq_inc;
		if (_tone > 0.5)
		{
			lvl_pole = 0;
			q = exp ((_tone - 0.5) * (basic::LN2 * 2));
			freq_ratio = 1;
			freq_inc = _freq * (_tone * 2 - 1);
			a1_ratio = 1;
		}
		else
		{
			const double	lvl = exp (_tone * (basic::LN10 * -4));
			lvl_pole = exp (log (lvl) / (nbr_biq * 2));
			q = 1;
			freq_ratio = 1 + 3 * (1 - _tone * 2);	// x4 -> x1 (constant freq)
			freq_inc = 0;
			a1_ratio = _tone * 2;
		}
		const double	q_biq = exp (-log (q) / nbr_biq);

		double			b [3];
		double			a [3];

		b [0] = 1;
		b [2] = lvl_pole * lvl_pole;

		a [0] = 1;
		a [2] = 1;

		float				biq_freq = _freq;
		const double	step = a1_ratio * (basic::PI / (nbr_biq * 4));
		for (int biq = 0; biq < nbr_biq; ++biq)
		{
			const double	a1 = 2 * cos ((2 * biq + 1) * step) * q_biq;
			b [1] = a1 * lvl_pole;
			a [1] = a1;

			_filter_arr [0] [biq].set_sample_freq (_sample_freq);
			_filter_arr [0] [biq].set_freq (_freq);
			_filter_arr [0] [biq].set_s_eq (b, a);
			_filter_arr [0] [biq].transform_s_to_z ();

			for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
			{
				_filter_arr [chn] [biq].copy_filter (_filter_arr [0] [biq]);
			}

			biq_freq *= freq_ratio;
			biq_freq += freq_inc;
			biq_freq = basic::min (biq_freq, max_filter_freq);
		}

		// If reactivated, we clean the buffers up.
		if (! old_filter_flag)
		{
			clear_buffers ();
		}
	}
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
