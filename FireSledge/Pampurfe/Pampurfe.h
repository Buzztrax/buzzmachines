/*****************************************************************************

        Pampurfe.h
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



#if ! defined (Pampurfe_HEADER_INCLUDED)
#define	Pampurfe_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"Distorter.h"
#include	"SoundEngine.h"

#include	"mdk.h"



class Pampurfe
:	public ::CMDKMachineInterface
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {			MCH_VERSION			= 1		};
	enum {			MIN_NBR_TRACKS		= 0		};
	enum {			MAX_NBR_TRACKS		= 0		};

						Pampurfe ();
	virtual			~Pampurfe () {}

	// CMDKMachineInterface
	virtual ::CMDKMachineInterfaceEx *
						GetEx ();
	virtual void	OutputModeChanged (bool stereo);
	virtual bool	MDKWork (float *psamples, int numsamples, int const mode);
	virtual bool	MDKWorkStereo (float *psamples, int numsamples, int const mode);
	virtual void	MDKInit (::CMachineDataInput * const pi);
	virtual void	MDKSave (::CMachineDataOutput * const po);

	// CMachineInterface
	virtual void	Tick ();
	virtual void	SetNumTracks (int const n);
	virtual const char *
						DescribeValue (int const param, int const value);

	// Buzz stuff
	static const ::CMachineInfo
						_mac_info;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	enum {			BUZZ_SAMPLE_SCALE	= 0x8000	};

#pragma pack (push, 1)	// <- don't forget that

	class GVals
	{
	public:
		byte				_gain;
		byte				_shape;
		byte				_compensation;
		byte				_tone;
		word				_freq;
	};

	class TVals
	{
	public:
	};

	class ParamPack
	{
	public:
		GVals				_gval;
		TVals				_tval [(MAX_NBR_TRACKS > 0) ? MAX_NBR_TRACKS : 1];
	};

#pragma pack (pop)

	enum {			GAIN_MIN_DB			= -20		};
	enum {			GAIN_MAX_DB			= 40		};
	enum {			GAIN_RANGE_DB		= GAIN_MAX_DB - GAIN_MIN_DB	};
	enum {			GAIN_PARAM_RESOL	= 0xFF / (GAIN_RANGE_DB + 1)	};
	enum {			GAIN_PARAM_MAX		= GAIN_RANGE_DB * GAIN_PARAM_RESOL	};
	enum {			COMP_MIN				= 0		}; // %
	enum {			COMP_MAX				= 200		};	// %
	enum {			COMP_RANGE			= COMP_MAX - COMP_MIN	};
	enum {			COMP_PARAM_RESOL	= 0xFF / (COMP_RANGE + 1)	};
	enum {			COMP_PARAM_MAX		= COMP_RANGE * COMP_PARAM_RESOL	};
	enum {			TONE_MIN				= 0		}; // %
	enum {			TONE_MAX				= 100		};	// %
	enum {			TONE_RANGE			= TONE_MAX - TONE_MIN	};
	enum {			TONE_PARAM_RESOL	= 0xFF / (TONE_RANGE + 1)	};
	enum {			TONE_PARAM_MAX		= TONE_RANGE * TONE_PARAM_RESOL	};
	enum {			FREQ_MIN_L2			= -3		};
	enum {			FREQ_MAX_L2			= 3		};
	enum {			FREQ_RANGE_L2		= FREQ_MAX_L2 - FREQ_MIN_L2	};
	enum {			FREQ_PARAM_RESOL	= 0xFFFF / (FREQ_RANGE_L2 + 1)	};
	enum {			FREQ_PARAM_MAX		= FREQ_RANGE_L2 * FREQ_PARAM_RESOL	};
	enum {			FREQ_BASE_HZ		= 1000	};

	enum ParamIndex
	{
		ParamIndex_GAIN = 0,
		ParamIndex_SHAPE,
		ParamIndex_COMPENSATION,
		ParamIndex_TONE,
		ParamIndex_FREQ,

		ParamIndex_TRACK_PARAM_ZONE,

		ParamIndex_NBR_PARAM = ParamIndex_TRACK_PARAM_ZONE
	};

	enum {			NBR_GLOB_PARAM		= ParamIndex_TRACK_PARAM_ZONE	};
	enum {			NBR_TRACK_PARAM	= ParamIndex_NBR_PARAM - ParamIndex_TRACK_PARAM_ZONE	};

	class DummyMiEx
	:	public ::CMDKMachineInterfaceEx
	{
	};

	float				buzz_to_gain (int param) const;
	Distorter::Shape
						buzz_to_shape (int param) const;
	float				buzz_to_compensation (int param) const;
	float				buzz_to_tone (int param) const;
	float				buzz_to_freq (int param) const;

	void				set_gain (float gain);
	void				set_shape (Distorter::Shape shape);
	void				set_compensation (float compensation);
	void				set_tone (float tone);
	void				set_freq (float freq);

	void				set_default_global_settings ();
	void				set_default_track_settings (int track);
	void				apply_global_settings ();
	void				apply_track_settings (int track);

	SoundEngine		_sound_engine;

	float				_sample_freq;		// Sample frequency, Hz, > 0
	int				_nbr_tracks;
	float				_buffer_arr [2] [MAX_BUFFER_LENGTH];	// 2 = stereo

	// Buzz stuff
	ParamPack		_public_param_pack;
	ParamPack		_savable_param_pack;
	DummyMiEx		_dummy_miex;

	static const ::CMachineParameter *
						_param_list [NBR_GLOB_PARAM + NBR_TRACK_PARAM];

	static const ::CMachineParameter
						_param_gain;
	static const ::CMachineParameter
						_param_shape;
	static const ::CMachineParameter
						_param_compensation;
	static const ::CMachineParameter
						_param_tone;
	static const ::CMachineParameter
						_param_freq;

	static const char
						COPYRIGHT_0 [];



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						Pampurfe (const Pampurfe &other);
	Pampurfe &		operator = (const Pampurfe &other);
	bool				operator == (const Pampurfe &other);
	bool				operator != (const Pampurfe &other);

};	// class Pampurfe



//#include	"Pampurfe.hpp"



#endif	// Pampurfe_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
