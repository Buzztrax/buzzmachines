/*****************************************************************************

        Array.hpp
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



#if ! defined (basic_Array_CODEHEADER_INCLUDED)
#define	basic_Array_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include	<cassert>



namespace basic
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <class T, long LENGTH>
Array <T, LENGTH>::Array (const Array <T, LENGTH> &other)
{
	for (long pos = 0; pos < LENGTH; ++pos)
	{
		_data [pos] = other._data [pos];
	}
}



template <class T, long LENGTH>
Array <T, LENGTH> &	Array <T, LENGTH>::operator = (const Array <T, LENGTH> &other)
{
	for (long pos = 0; pos < LENGTH; ++pos)
	{
		_data [pos] = other._data [pos];
	}

	return (*this);
}



template <class T, long LENGTH>
const typename Array <T, LENGTH>::Element &	Array <T, LENGTH>::operator [] (long pos) const
{
	assert (pos >= 0);
	assert (pos < LENGTH);

	return (_data [pos]);
}



template <class T, long LENGTH>
typename Array <T, LENGTH>::Element &	Array <T, LENGTH>::operator [] (long pos)
{
	assert (pos >= 0);
	assert (pos < LENGTH);

	return (_data [pos]);
}



template <class T, long LENGTH>
long	Array <T, LENGTH>::size () const
{
	return (LENGTH);
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}	// namespace basic



#endif	// basic_Array_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
