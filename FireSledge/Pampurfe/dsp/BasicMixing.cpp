/*****************************************************************************

        BasicMixing.cpp
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

#include	<cassert>



namespace dsp
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



void	BasicMixing::copy_1_1_v (float out_ptr [], const float in_ptr [], long nbr_spl, float vol)
{
	assert (out_ptr != 0);
	assert (in_ptr != 0);
	assert (nbr_spl > 0);

	long				pos = 0;
	do
	{
		out_ptr [pos] = in_ptr [pos] * vol;
		++ pos;
	}
	while (pos < nbr_spl);
}



void	BasicMixing::copy_1_1_vlr (float out_ptr [], const float in_ptr [], long nbr_spl, float s_vol, float e_vol)
{
	assert (out_ptr != 0);
	assert (in_ptr != 0);
	assert (nbr_spl > 0);

	const float		step = (e_vol - s_vol) / nbr_spl;
	long				pos = 0;
	do
	{
		out_ptr [pos] = in_ptr [pos] * s_vol;
		++ pos;
		s_vol += step;
	}
	while (pos < nbr_spl);
}



void	BasicMixing::copy_2_2i (float out_ptr [], const float in_1_ptr [], const float in_2_ptr [], long nbr_spl)
{
	assert (out_ptr != 0);
	assert (in_1_ptr != 0);
	assert (in_2_ptr != 0);
	assert (nbr_spl > 0);

#if defined (WIN32) && defined (_MSC_VER)

	__asm
	{
		mov				esi, in_1_ptr
		mov				ecx, in_2_ptr
		mov				edi, out_ptr
		mov				eax, nbr_spl
		mov				ebx, eax
		and				ebx, 3
		sub				eax, ebx
		shl				eax, 2			; eax is now a byte counter
		lea				esi, [esi + eax]
		lea				ecx, [ecx + eax]
		lea				edi, [edi + eax*2]
		neg				eax
		jz					short endif_1

	boucle_1:

		movq				mm0, [esi + eax]
		movq				mm4, [esi + eax + 8]
		movq				mm1, [ecx + eax]
		movq				mm5, [ecx + eax + 8]

		movq				mm2, mm0
		punpckldq		mm0, mm1
		punpckhdq		mm2, mm1

		add				eax, 16			; 4 frames at each iteration

		movq				mm6, mm4
		punpckldq		mm4, mm5
		punpckhdq		mm6, mm5

		movq				[edi + eax*2      - 32], mm0
		movq				[edi + eax*2 +  8 - 32], mm2
		movq				[edi + eax*2 + 16 - 32], mm4
		movq				[edi + eax*2 + 24 - 32], mm6

		jl					short boucle_1

	endif_1:

		and				ebx, ebx
		jz					short endif_2

	boucle_2:

		add				eax, 4
		dec				ebx
		movd				mm0, [esi + eax - 4]
		movd				mm1, [ecx + eax - 4]
		punpckldq		mm0, mm1
		movq				[edi + eax*2 - 8], mm0

		jg					short boucle_2

	endif_2:

		emms
	}

#else

	long				pos = 0;
	do
	{
		out_ptr [pos * 2    ] = in_1_ptr [pos];
		out_ptr [pos * 2 + 1] = in_2_ptr [pos];
		++ pos;
	}
	while (pos < nbr_spl);

#endif
}



void	BasicMixing::copy_2i_2 (float out_1_ptr [], float out_2_ptr [], const float in_ptr [], long nbr_spl)
{
	assert (out_1_ptr != 0);
	assert (out_2_ptr != 0);
	assert (in_ptr != 0);
	assert (nbr_spl > 0);

#if defined (WIN32) && defined (_MSC_VER)

	__asm
	{
		mov				esi, in_ptr
		mov				edi, out_1_ptr
		mov				edx, out_2_ptr
		mov				eax, nbr_spl
		mov				ebx, eax
		and				ebx, 3
		sub				eax, ebx
		shl				eax, 2			; eax is now a byte counter
		lea				esi, [esi + eax*2]
		lea				edi, [edi + eax]
		lea				edx, [edx + eax]
		neg				eax
		jz					short endif_1

	boucle_1:

		movq				mm0, [esi + eax*2     ]
		movq				mm1, [esi + eax*2 +  8]
		movq				mm4, [esi + eax*2 + 16]
		movq				mm5, [esi + eax*2 + 24]

		movq				mm2, mm0
		punpckldq		mm0, mm1
		punpckhdq		mm2, mm1

		add				eax, 16			; 4 frames at each iteration

		movq				mm6, mm4
		punpckldq		mm4, mm5
		punpckhdq		mm6, mm5

		movq				[edi + eax      - 16], mm0
		movq				[edi + eax +  8 - 16], mm4
		movq				[edx + eax      - 16], mm2
		movq				[edx + eax +  8 - 16], mm6

		jl					short boucle_1

	endif_1:

		and				ebx, ebx
		jz					short endif_2

	boucle_2:

		add				eax, 4
		dec				ebx
		movq				mm0, [esi + eax*2 - 8]
		movd				[edi + eax - 4], mm0
		pshufw			mm0, mm0, 0EEh	; Copy the 32 higher bits into 32 lower
		movd				[edx + eax - 4], mm0

		jg					short boucle_2

	endif_2:

		emms
	}

#else

	long				pos = 0;
	do
	{
		out_1_ptr [pos] = in_ptr [pos * 2    ];
		out_2_ptr [pos] = in_ptr [pos * 2 + 1];
		++ pos;
	}
	while (pos < nbr_spl);

#endif
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace dsp



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
