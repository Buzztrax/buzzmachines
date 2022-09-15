/*****************************************************************************

        Pampurfe.cpp
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
#include	"dsp/BasicMixing.h"
#include	"Distorter.h"
#include	"Pampurfe.h"

#include	<cassert>
#include	<cmath>
#include	<cstdio>



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



Pampurfe::Pampurfe ()
:	_sound_engine (BUZZ_SAMPLE_SCALE)
,	_nbr_tracks (0)
,	_sample_freq (-1)
{
	assert (NBR_GLOB_PARAM >= 0);
	assert (NBR_TRACK_PARAM >= 0);

	assert (GAIN_PARAM_MAX < 0xFF);
	assert (COMP_PARAM_MAX < 0xFF);
	assert (TONE_PARAM_MAX < 0xFF);
	assert (FREQ_PARAM_MAX < 0xFFFF);

	GlobalVals = &_public_param_pack._gval;
	TrackVals = &_public_param_pack._tval;
}



::CMDKMachineInterfaceEx *	Pampurfe::GetEx ()
{
	return (&_dummy_miex);
}



void	Pampurfe::OutputModeChanged (bool stereo)
{
	// Nothing
}



bool	Pampurfe::MDKWork (float *psamples, int numsamples, int const mode)
{
	if (! (mode & WM_READ) || ! (mode & WM_WRITE))
	{
		return (false);
	}

	float *		buf_ptr_arr [2] =
	{
		psamples,
		psamples
	};

	_sound_engine.process_block (buf_ptr_arr, numsamples, 1);

	return (true);
}



bool	Pampurfe::MDKWorkStereo (float *psamples, int numsamples, int const mode)
{
	if (! (mode & WM_READ) || ! (mode & WM_WRITE))
	{
		return (false);
	}

	float *		buf_ptr_arr [2] =
	{
		&_buffer_arr [0] [0],
		&_buffer_arr [1] [0]
	};

	dsp::BasicMixing::copy_2i_2 (
		buf_ptr_arr [0],
		buf_ptr_arr [1],
		psamples,
		numsamples
	);

	_sound_engine.process_block (buf_ptr_arr, numsamples, 2);
	
	dsp::BasicMixing::copy_2_2i (
		psamples,
		buf_ptr_arr [0],
		buf_ptr_arr [1],
		numsamples
	);

	return (true);
}



void	Pampurfe::MDKInit (::CMachineDataInput * const pi)
{
	// Misc DSP init
	{
		_sample_freq = float (pMasterInfo->SamplesPerSec);
		assert (_sample_freq > 0);

		_sound_engine.set_sample_freq (_sample_freq);
		for (int track = 0; track < MAX_NBR_TRACKS; ++track)
		{
			// To do
		}
	}

	// Set default settings
	{
		set_default_global_settings ();
		_nbr_tracks = MIN_NBR_TRACKS;
		for (int track = 0; track < MAX_NBR_TRACKS; ++track)
		{
			set_default_track_settings (track);
		}
	}

	// Overwrite default settings with preset data if any
	if (pi != 0)
	{
		byte			version;
		byte			nbr_tracks;

		pi->Read (version);

		pi->Read (nbr_tracks);
		_nbr_tracks = basic::limit (
			int (nbr_tracks), int (MIN_NBR_TRACKS), int (MAX_NBR_TRACKS)
		);

		pi->Read (&_public_param_pack, sizeof (_public_param_pack));
	}

	// Apply settings
	{
		apply_global_settings ();
		for (int track = 0; track < MAX_NBR_TRACKS; ++track)
		{
			apply_track_settings (track);
		}
	}
}



void	Pampurfe::MDKSave (::CMachineDataOutput * const po)
{
	assert (po != 0);

	po->Write (static_cast <byte> (MCH_VERSION));

	po->Write (static_cast <byte> (_nbr_tracks));
	po->Write (&_savable_param_pack, sizeof (_savable_param_pack));
}



void	Pampurfe::Tick ()
{
	apply_global_settings ();

	for (int track = 0; track < _nbr_tracks; ++track)
	{
		apply_track_settings (track);
	}
}



void	Pampurfe::SetNumTracks (int const n)
{
	for (int track = _nbr_tracks; track < n; ++track)
	{
		set_default_track_settings (track);
		apply_track_settings (track);
		// To do
	}

	_nbr_tracks = n;
}



const char *	Pampurfe::DescribeValue (int const param, int const value)
{
	const long		max_len = 63;
	static char		txt_0 [max_len + 1];

	switch (param)
	{
	case	ParamIndex_GAIN:
		sprintf (txt_0, "%+.1f dB", log (buzz_to_gain (value)) * (20 / basic::LN10));
		break;

	case	ParamIndex_SHAPE:
		sprintf (txt_0, "%s", Distorter::get_shape_name (buzz_to_shape (value)));
		break;

	case	ParamIndex_COMPENSATION:
		sprintf (txt_0, "%.1f %%", double (buzz_to_compensation (value) * 100));
		break;

	case	ParamIndex_TONE:
		sprintf (txt_0, "%.1f %%", double (buzz_to_tone (value) * 100));
		break;

	case	ParamIndex_FREQ:
		sprintf (txt_0, "%d Hz", basic::round_int (buzz_to_freq (value)));
		break;

	default:
		txt_0 [0] = '\0';
		break;
	}

	return (txt_0);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



float	Pampurfe::buzz_to_gain (int param) const
{
	const float	val = GAIN_MIN_DB + param * (1.0f / GAIN_PARAM_RESOL);

	return (float (exp (val * (basic::LN10 / 20))));
}



Distorter::Shape	Pampurfe::buzz_to_shape (int param) const
{
	return (Distorter::Shape (param));
}



float	Pampurfe::buzz_to_compensation (int param) const
{
	return (param * ((COMP_MIN + 1.0f / COMP_PARAM_RESOL) * 0.01f));
}



float	Pampurfe::buzz_to_tone (int param) const
{
	return (param * ((TONE_MIN + 1.0f / TONE_PARAM_RESOL) * 0.01f));
}



float	Pampurfe::buzz_to_freq (int param) const
{
	const float	val = FREQ_MIN_L2 + param * (1.0f / FREQ_PARAM_RESOL);

	return (float (FREQ_BASE_HZ * exp (val * basic::LN2)));
}



void	Pampurfe::set_gain (float gain)
{
	_sound_engine.set_gain (gain);
}



void	Pampurfe::set_shape (Distorter::Shape shape)
{
	_sound_engine.set_shape (shape);
}



void	Pampurfe::set_compensation (float compensation)
{
	_sound_engine.set_compensation (compensation);
}



void	Pampurfe::set_tone (float tone)
{
	_sound_engine.set_tone (tone);
}



void	Pampurfe::set_freq (float freq)
{
	_sound_engine.set_freq (freq);
}



void	Pampurfe::set_default_global_settings ()
{
	GVals &			gval = _public_param_pack._gval;

	gval._gain         = _param_gain.DefValue;
	gval._shape        = _param_shape.DefValue;
	gval._compensation = _param_compensation.DefValue;
	gval._tone         = _param_tone.DefValue;
	gval._freq         = _param_freq.DefValue;
}



void	Pampurfe::set_default_track_settings (int track)
{
	assert (track >= 0);
	assert (track < MAX_NBR_TRACKS);

	TVals &			tval = _public_param_pack._tval [track];

	// To do
}



void	Pampurfe::apply_global_settings ()
{
	const GVals &	gval = _public_param_pack._gval;
	GVals &			sgval = _savable_param_pack._gval;

	if (gval._gain != _param_gain.NoValue)
	{
		set_gain (buzz_to_gain (gval._gain));
		sgval._gain = gval._gain;
	}

	if (gval._shape != _param_shape.NoValue)
	{
		set_shape (buzz_to_shape (gval._shape));
		sgval._shape = gval._shape;
	}

	if (gval._compensation != _param_compensation.NoValue)
	{
		set_compensation (buzz_to_compensation (gval._compensation));
		sgval._compensation = gval._compensation;
	}

	if (gval._tone != _param_tone.NoValue)
	{
		set_tone (buzz_to_tone (gval._tone));
		sgval._tone = gval._tone;
	}

	if (gval._freq != _param_freq.NoValue)
	{
		set_freq (buzz_to_freq (gval._freq));
		sgval._freq = gval._freq;
	}
}



void	Pampurfe::apply_track_settings (int track)
{
	assert (track >= 0);
	assert (track < MAX_NBR_TRACKS);

	const TVals &	tval = _public_param_pack._tval [track];
	TVals &			stval = _savable_param_pack._tval [track];

	// To do
}



const ::CMachineInfo	Pampurfe::_mac_info =
{
	MT_EFFECT,								// type, MT_GENERATOR or MT_EFFECT
	MI_VERSION,								// version, always set this to MI_VERSION
	MIF_DOES_INPUT_MIXING,				// Flags ????
	MIN_NBR_TRACKS,						// minimum number of tracks
	MAX_NBR_TRACKS,						// maximum number of tracks
	NBR_GLOB_PARAM,						// number of global parameters
	NBR_TRACK_PARAM,						// number of track parameters
	_param_list,							// pointer to list of parameters
	0,											// number of attributes
	0,											// pointer to list of attributes
#if ! defined (NDEBUG)
	"FireSledge Pampurfe (debug build)",	// name for debug build
#else
	"FireSledge Pampurfe",				// name for release build
#endif
	"Pampurfe",								// short name
	"FireSledge",							// your name
	0											// list of commands
};



const ::CMachineParameter *	Pampurfe::_param_list [NBR_GLOB_PARAM + NBR_TRACK_PARAM] =
{
	// Global param
	&_param_gain,
	&_param_shape,
	&_param_compensation,
	&_param_tone,
	&_param_freq

	// Track param
};



const ::CMachineParameter	Pampurfe::_param_gain =
{
	pt_byte,								// type
	"Gain",								// short (one or two words) name 
	"Gain (dB)",						// longer description 
	0x00,									// minumum value
	GAIN_PARAM_MAX,					// maximum value
	0xFF,									// some value outside [min..max] range 
	MPF_STATE,							// flags
	-GAIN_MIN_DB * GAIN_PARAM_RESOL	// Default value
};

const ::CMachineParameter	Pampurfe::_param_shape =
{
	pt_byte,								// type
	"Shape",								// short (one or two words) name 
	"Shape ()",							// longer description 
	0x00,									// minumum value
	Distorter::Shape_NBR_ELT - 1,	// maximum value
	0xFF,									// some value outside [min..max] range 
	MPF_STATE,							// flags
	0x00									// Default value
};

const ::CMachineParameter	Pampurfe::_param_compensation =
{
	pt_byte,								// type
	"Compensation",					// short (one or two words) name 
	"Volume auto-compensation",	// longer description 
	0x00,									// minumum value
	COMP_PARAM_MAX,					// maximum value
	0xFF,									// some value outside [min..max] range 
	MPF_STATE,							// flags
	0										// Default value
};

const ::CMachineParameter	Pampurfe::_param_tone =
{
	pt_byte,								// type
	"Tone",								// short (one or two words) name 
	"Tone",								// longer description 
	0x00,									// minumum value
	TONE_PARAM_MAX,					// maximum value
	0xFF,									// some value outside [min..max] range 
	MPF_STATE,							// flags
	0										// Default value
};

const ::CMachineParameter	Pampurfe::_param_freq =
{
	pt_word,								// type
	"Frequency",						// short (one or two words) name 
	"Frequency (log scale)",		// longer description 
	0x0000,								// minumum value
	FREQ_PARAM_MAX,					// maximum value
	0xFFFF,								// some value outside [min..max] range 
	MPF_STATE,							// flags
	-FREQ_MIN_L2 * FREQ_PARAM_RESOL	// Default value
};

const char	Pampurfe::COPYRIGHT_0 [] =
	"@(#) Pampurfe / Build " __DATE__ " / Copyright (c) FireSledge / Freeware.\n"
	"Laurent de Soras\n"
	"4 avenue Alsace-Lorraine\n"
	"92500 Rueil-Malmaison\n"
	"France\n"
	"http://ldesoras.free.fr";



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
