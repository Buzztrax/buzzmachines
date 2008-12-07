#ifndef	SRF_TYPES_H__
#define	SRF_TYPES_H__

namespace SurfDSPLib
{

#ifdef WIN32
typedef	unsigned char		u_char;
typedef	unsigned short		u_short;
typedef	unsigned long		u_long;
#endif

#ifdef WIN32
typedef	unsigned __int64		u_llong;
typedef	signed __int64		llong;
#else
typedef unsigned long long 	u_llong;
typedef signed long long 	llong;
#endif
};

#endif
