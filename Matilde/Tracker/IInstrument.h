// IInstrument.h: interface for the IInstrument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IINSTRUMENT_H__CAE365A6_76F3_477B_B389_DE228046E9C2__INCLUDED_)
#define AFX_IINSTRUMENT_H__CAE365A6_76F3_477B_B389_DE228046E9C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class	ISample;

class	IInstrument  
{
public:
							IInstrument();
	virtual					~IInstrument();

	virtual	ISample		*	GetSample( int iNote )=0;
};

#endif // !defined(AFX_IINSTRUMENT_H__CAE365A6_76F3_477B_B389_DE228046E9C2__INCLUDED_)
