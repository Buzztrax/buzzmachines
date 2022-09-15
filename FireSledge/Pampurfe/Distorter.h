/*****************************************************************************

        Distorter.h
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



#if ! defined (Distorter_HEADER_INCLUDED)
#define	Distorter_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	"basic/Array.h"
#include	"basic/basic_def.h"
#include	"basic/basic_fnc.h"
#include	"dsp/InterpFncFinite.h"
#include	"dsp/Shelf.h"

#include	<cmath>



class Distorter
{

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	enum {			MAX_NBR_CHN				= 2		};

	enum	Shape
	{
		Shape_TUBE_ATAN = 0,
		Shape_TUBE_LOG,
		Shape_BRAMPOS,
		Shape_TUBE_ASYM_1,
		Shape_TUBE_ASYM_3,
		Shape_HARDCLIP,
		Shape_BRAMNEG,
		Shape_SIN,
		Shape_RAPH_1,
		Shape_SLEW_RATE_SOFT,

		Shape_NBR_ELT
	};

						Distorter ();
	virtual			~Distorter () {}

	void				set_sample_freq (float sample_freq);
	void				set_gain (float gain);
	float				get_gain () const;
	void				set_shape (Shape type);
	Shape				get_shape () const;
	void				process_block (float *data_arr [MAX_NBR_CHN], long nbr_spl, int nbr_chn);
	void				clear_buffers ();

	static const char *
						get_shape_name (Shape type);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	struct FncTubeAtan
	{
		double			operator () (double x) const
		{
			using namespace std;

			return (atan (x * (basic::PI / 2)) * (2 / basic::PI));
		}
	};

	struct FncTubeLog
	{
		double			operator () (double x) const
		{
			using namespace std;

			if (x < 0)
			{
				return (-log (1-x));
			}

			return (log (1+x));
		}
	};

	struct FncTubeAsym1	// [-7 ; 7]
	{
		double			operator () (double x) const
		{
			using namespace std;

			const double	a = 1 + exp (sqrt (fabs (x)) * -0.75);

			return ((3.0 / 4) * (exp (x) - exp (-x * a)) / (exp (x) + exp (-x)));
		}
	};

	struct FncTubeAsym3	// [-8 ; 8]
	{
		double			operator () (double x) const
		{
			using namespace std;

			const double	a = exp (x - 1);
			const double	b = exp (-x);
			const double	num = a - b - (1 / basic::EXP1) + 1;
			const double	denom = a + b;

			return (num / denom);
		}
	};

	struct FncPunch
	{
		double			operator () (double x) const
		{
			using namespace std;

			return (sin (x));
		}
	};

	struct FncRaph1	// [-32 ; 32]
	{
		double			operator () (double x) const
		{
			using namespace std;

			if (x < 0)
			{
				return (exp(x)-1 - basic::sinc (3 + x));
			}

			return (1-exp(-x) + basic::sinc (x - 3));
		}
	};

	struct DistSlewRate
	{
		enum {			UNITS_PER_SEC = 1000	};

		inline float	process_sample (float spl);

		float				_rate;		// Unit per sample
		float				_pos;
	};

	void				distort_block_one_voice (int chn, float data [], long nbr_spl);

	float				_sample_freq;
	float				_gain;
	Shape				_shape;
	basic::Array <dsp::Shelf, MAX_NBR_CHN>
						_dc_remover_arr;

	dsp::InterpFncFinite <32, FncTubeAtan>
						_fnc_tube_atan;
	dsp::InterpFncFinite <128, FncTubeLog>
						_fnc_tube_log;
	dsp::InterpFncFinite <7, FncTubeAsym1>
						_fnc_tube_asym_1;
	dsp::InterpFncFinite <8, FncTubeAsym3>
						_fnc_tube_asym_3;
	dsp::InterpFncFinite <4, FncPunch>
						_fnc_sin;
	dsp::InterpFncFinite <32, FncRaph1>
						_fnc_raph_1;
	basic::Array <DistSlewRate, MAX_NBR_CHN>
						_slew_rate_arr;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

						Distorter (const Distorter &other);
	Distorter &		operator = (const Distorter &other);
	bool				operator == (const Distorter &other);
	bool				operator != (const Distorter &other);

};	// class Distorter



//#include	"Distorter.hpp"



#endif	// Distorter_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
