#include	<string.h>
#include	<stdlib.h>
#include	"Envelope.h"
#include <MachineInterface.h>

CEnvelope::CEnvelope()
{
	m_iAllocatedEnvelopeSize=0;
	m_iEnvelopeSize=0;
	m_pEnvelope=NULL;
}

CEnvelope::~CEnvelope()
{
	if( m_pEnvelope )
		free( m_pEnvelope );
}

void	CEnvelope::Reset()
{
	m_iEnvelopeSize=0;
	m_fEnvelopeTime=0;
	m_fEnvelopeSpeed=0;
}

bool	CEnvelope::IsValid()
{
	return m_iEnvelopeSize>=2;
}

void	CEnvelope::Release()
{
	m_oEnvelopeStuck=false;
}

void	CEnvelope::	ReadEnvelope( CMICallbacks *pCB, int iIns, int iEnvIndex )
{
	m_iEnvelopeSize=pCB->GetEnvSize( iIns, iEnvIndex );
	if( m_iEnvelopeSize>m_iAllocatedEnvelopeSize )
	{
		m_iAllocatedEnvelopeSize=m_iEnvelopeSize;
		m_pEnvelope=(SEnvelopePoint *)realloc( m_pEnvelope, sizeof(SEnvelopePoint)*m_iAllocatedEnvelopeSize );
	}
	int	i;
	for( i=0; i<m_iEnvelopeSize; i+=1 )
	{
		word	x,y;
		int		flags;
		pCB->GetEnvPoint( iIns, iEnvIndex, i, x, y, flags );
		m_pEnvelope[i].m_fPos=x/65535.0f;
		m_pEnvelope[i].m_fValue=y/65535.0f;
		m_pEnvelope[i].m_oSustain=(flags&EIF_SUSTAIN)?true:false;
	}
}

void	CEnvelope::Restart( float fFreq )
{
	m_fEnvelopeTime=0;
	m_fEnvelopeSpeed=fFreq;
	m_iEnvelopeIndex=0;
	m_oEnvelopeStuck=true;
}

float	CEnvelope::GetCurrentLevel( int iCount )
{
	if( m_iEnvelopeSize<2 )
		return 1.0f;

	while( (m_fEnvelopeTime>m_pEnvelope[m_iEnvelopeIndex+1].m_fPos) && (m_iEnvelopeIndex<m_iEnvelopeSize) )
	{
		if( m_pEnvelope[m_iEnvelopeIndex].m_oSustain && m_oEnvelopeStuck )
		{
			return m_pEnvelope[m_iEnvelopeIndex].m_fValue;
		}

		m_iEnvelopeIndex+=1;
	}

	if( m_pEnvelope[m_iEnvelopeIndex].m_oSustain && m_oEnvelopeStuck )
	{
		return m_pEnvelope[m_iEnvelopeIndex].m_fValue;
	}

	if( m_iEnvelopeIndex>=m_iEnvelopeSize-1 )
		return m_pEnvelope[m_iEnvelopeSize-1].m_fValue;

	float t=(m_fEnvelopeTime-m_pEnvelope[m_iEnvelopeIndex].m_fPos)/(m_pEnvelope[m_iEnvelopeIndex+1].m_fPos-m_pEnvelope[m_iEnvelopeIndex].m_fPos);

	m_fEnvelopeTime+=iCount*m_fEnvelopeSpeed;
	if( m_fEnvelopeTime>1.0f )
		m_fEnvelopeTime=1.0f;

	return (m_pEnvelope[m_iEnvelopeIndex+1].m_fValue-m_pEnvelope[m_iEnvelopeIndex].m_fValue)*t+m_pEnvelope[m_iEnvelopeIndex].m_fValue;
}

int	CEnvelope::GetPlayPos()
{
	if( IsValid() )
		return int(m_fEnvelopeTime*65535.0f);
	else
		return -1;
}
