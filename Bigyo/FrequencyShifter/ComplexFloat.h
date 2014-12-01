#ifndef __COMPLEXFLOAT_H
#define __COMPLEXFLOAT_H


template <class T>
class complex
{
	public:
			T re, im;
			complex() {}
			complex(T x, T y): re(x), im(y) {}

};

//template <class T>
//			__forceinline complex<T> operator* (complex<T> a , complex<T> b)
//			{
//				 return   complex<T>( a.re * b.re - a.im * b.im , a.im * b.re + a.re * b.im ) ;
//			}

#endif