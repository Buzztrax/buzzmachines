/*****************************************************************************

        Distorter.cpp
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

#include	"dsp/BasicMixing.h"
#include	"Distorter.h"

#include	<cassert>



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Distorter::Distorter ()
:	_gain (1)
,	_shape (Shape_TUBE_ATAN)
{
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		_dc_remover_arr [chn].set_freq (10);
		_dc_remover_arr [chn].set_level (0);
	}

	set_sample_freq (44100);
	clear_buffers ();
}



void	Distorter::set_sample_freq (float sample_freq)
{
	assert (sample_freq > 0);

	_sample_freq = sample_freq;

	const float	rate = DistSlewRate::UNITS_PER_SEC / sample_freq;
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		_slew_rate_arr [chn]._rate = rate;
		_dc_remover_arr [chn].set_sample_freq (_sample_freq);
		_dc_remover_arr [chn].make_low_shelf ();
	}
}



void	Distorter::set_gain (float gain)
{
	assert (gain >= 0);

	_gain = gain;
}



float	Distorter::get_gain () const
{
	return (_gain);
}



void	Distorter::set_shape (Shape shape)
{
	assert (shape >= 0);
	assert (shape < Shape_NBR_ELT);

	_shape = shape;
}



Distorter::Shape	Distorter::get_shape () const
{
	return (_shape);
}



void	Distorter::process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn)
{
	assert (data_arr != 0);
	assert (nbr_spl > 0);
	assert (nbr_chn >= 0);
	assert (nbr_chn <= MAX_NBR_CHN);

	for (int chn = 0; chn < nbr_chn; ++chn)
	{
		dsp::BasicMixing::copy_1_1_v (
			data_arr [chn],
			data_arr [chn],
			nbr_spl,
			_gain
		);

		distort_block_one_voice (chn, data_arr [chn], nbr_spl);

		_dc_remover_arr [chn].process_block (data_arr [chn], nbr_spl);
	}
}



void	Distorter::clear_buffers ()
{
	for (int chn = 0; chn < MAX_NBR_CHN; ++chn)
	{
		_slew_rate_arr [chn]._pos = 0;
		_dc_remover_arr [chn].clear_buffers ();
	}
}



const char *	Distorter::get_shape_name (Distorter::Shape type)
{
	assert (type >= 0);
	assert (type < Shape_NBR_ELT);

	static const char *	name_arr_0 [Shape_NBR_ELT] =
	{
		"Overdrive",
		"Boost",
		"Tube Sym",
		"TubeAsym1",
		"TubeAsym2",
		"Hardclip",
		"Defect",
		"Punch",
		"Rich",
		"Slew Rate"
	};

	return (name_arr_0 [type]);
}



float	Distorter::DistSlewRate::process_sample (float spl)
{
	using namespace std;
	
	if (fabs (spl - _pos) < _rate)
	{
		_pos = spl;
	}
	else
	{
		if (spl > _pos)
		{
			_pos += _rate;
		}
		else
		{
			_pos -= _rate;
		}
	}

	return (_pos);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	Distorter::distort_block_one_voice (int chn, float data [], long nbr_spl)
{
	assert (data != 0);
	assert (nbr_spl > 0);

	using namespace std;

	long				pos = 0;
	
	switch (_shape)
	{
	case	Shape_TUBE_ATAN:
		do
		{
			data [pos] = _fnc_tube_atan (data [pos]);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_TUBE_LOG:
		do
		{
			data [pos] = _fnc_tube_log (data [pos]);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_BRAMPOS:
		do
		{
			const float		spl = data [pos];
			const float		abs_spl = float (fabs (spl));
			const float		alpha = 0.5;
			assert (alpha > -1);
			data [pos] *=   (abs_spl + alpha)
			              / (spl*spl + (alpha-1) * abs_spl + 1);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_TUBE_ASYM_1:
		do
		{
			data [pos] = _fnc_tube_asym_1 (data [pos]);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_TUBE_ASYM_3:
		do
		{
			data [pos] = _fnc_tube_asym_3 (data [pos]);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_HARDCLIP:
		do
		{
			data [pos] = basic::limit (data [pos], -1.0f, 1.0f);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_BRAMNEG:
		do
		{
			const float		spl = data [pos];
			const float		abs_spl = float (fabs (spl));
			const float		alpha = -0.5;
			assert (alpha > -1);
			data [pos] *=   (abs_spl + alpha)
			              / (spl*spl + (alpha-1) * abs_spl + 1);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_SIN:
		do
		{
			data [pos] = _fnc_sin (data [pos]);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_RAPH_1:
		do
		{
			data [pos] = _fnc_raph_1 (data [pos]);
			++pos;
		}
		while (pos < nbr_spl);
		break;

	case	Shape_SLEW_RATE_SOFT:
		{
			DistSlewRate &		sr = _slew_rate_arr [chn];
			do
			{
				const float		spl = _fnc_tube_atan (data [pos]);
				data [pos] = sr.process_sample (spl);
				++pos;
			}
			while (pos < nbr_spl);
		}
		break;

	default:
		assert (false);
		break;
	}
}



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
