#ifndef	CENVELOPE_H__
#define	CENVELOPE_H__

class	CMICallbacks;

class	CEnvelope
{
public:
							CEnvelope();
							~CEnvelope();

	void					Reset();

	void					ReadEnvelope( CMICallbacks *pCB, int iIns, int iEnvIndex );
	bool					IsValid();

	void					Restart( float fFreq );
	void					Release();
	float					GetCurrentLevel( int iCount );

	int						GetPlayPos();

	bool					HasEnded() { return IsValid() && m_iEnvelopeIndex>=m_iEnvelopeSize-1 || m_fEnvelopeTime>=1.0f; }

protected:
	struct	SEnvelopePoint
	{
		float	m_fPos;
		float	m_fValue;
		bool	m_oSustain;
	};

	int						m_iAllocatedEnvelopeSize;
	int						m_iEnvelopeSize;
	SEnvelopePoint		*	m_pEnvelope;
	float					m_fEnvelopeTime;
	float					m_fEnvelopeSpeed;
	int						m_iEnvelopeIndex;
	bool					m_oEnvelopeStuck;
};

#endif
