// ISample.h: interface for the ISample class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISAMPLE_H__9038B66B_59BD_4125_A8E5_D78554C8E461__INCLUDED_)
#define AFX_ISAMPLE_H__9038B66B_59BD_4125_A8E5_D78554C8E461__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class	ISample  
{
public:
						ISample();
	virtual				~ISample()=0;

	virtual	float		GetVolume()=0;
	virtual	int			GetRootNote()=0;
	virtual	int			GetRootFrequency()=0;

	virtual	bool		IsValid()=0;
	virtual	bool		IsStereo()=0;
	virtual	bool		IsExtended()=0;
	virtual	bool		IsLoop()=0;
	virtual	bool		IsPingPongLoop()=0;

	virtual	void	*	GetSampleStart()=0;
	virtual	long		GetSampleLength()=0;
	virtual	long		GetLoopStart()=0;
	virtual	long		GetLoopEnd()=0;
	virtual int			GetSampleFormat()=0;
	virtual int			GetBitsPerSample()=0;

	virtual	bool		IsStillValid()=0;

	virtual	void		Free()=0;
};

#endif // !defined(AFX_ISAMPLE_H__9038B66B_59BD_4125_A8E5_D78554C8E461__INCLUDED_)
