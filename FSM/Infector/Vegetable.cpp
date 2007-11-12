// globalny tuning
// zakres dla suwaków (cutoff, resonance, modulation)
// lepszy tryb mono
// sustain 0 -> b³¹d
// startuje -> bzdury
// bug w seq<->buzz

// lokalne/globalne LFO
#include <math.h>
#include <float.h>
#include "../../common/MachineInterface.h"
#include "../dspchips/DSPChips.h"

CBandlimitedTable::CBandlimitedTable()
{
  m_nLevels=0;
  m_pBuffer=NULL;
  m_nBufSize=0;
}

CAnyWaveLevel *CBandlimitedTable::GetTable(float fScanRate)
{
  for (int i=0; i<m_nLevels; i++)
  {
    if (fScanRate<=m_levels[i].m_fMaxScanRate)
    {
      if (i>0)
        return &m_levels[i-1];
      else
        return m_levels;
    }
  }
  return &m_levels[m_nLevels-1];
}

void CBandlimitedTable::Make(float fMultiplyFactor, float fMaxScanRate, float fCrispFactor)
{
	if (fCrispFactor==-1)
		fCrispFactor=2.0f/fMultiplyFactor;
  for (int i=0; i<m_nLevels; i++)
    delete []m_levels[i].m_pData;
  m_nLevels=0;

  int nSize=1<<(int)(log(m_nBufSize)/log(2)+0.999);
  if (nSize!=m_nBufSize) Brk(); // XXXKF
  m_levels[0].m_nSize=nSize;
  m_levels[0].m_nBits=(int)(log(nSize)/log(2)+0.5);
  float *pOut=m_levels[0].m_pData=new float[nSize+4];
  m_levels[0].m_fMultiplier=(float)pow(2.0,-31+m_levels[0].m_nBits);
  for (int i=0; i<nSize; i++)
    m_levels[0].m_pData[i]=m_pBuffer[i];
	float qf=0.25f;
  m_levels[0].m_fMaxScanRate=qf/nSize;
	
	pOut[nSize]=pOut[0];
	pOut[nSize+1]=pOut[1];
	pOut[nSize+2]=pOut[2];
	pOut[nSize+3]=pOut[3];

	float *pBase=new float[2*nSize];
	float *pTemp=new float[2*nSize];
	float *pTemp2=new float[2*nSize];
	float *pTemp3=new float[2*nSize];
	for (int i=0; i<nSize; i++)
		pBase[i]=m_pBuffer[i],
		pBase[i+nSize]=0.0;

	fft_float(nSize,0,pBase,pBase+nSize,pTemp,pTemp+nSize);

  m_nLevels=1;
	
	int nLastTable=0;

	int nAllocs=0;

  while(m_levels[m_nLevels-1].m_fMaxScanRate<=fMaxScanRate)
  {
		CAnyWaveLevel &lev=m_levels[m_nLevels];
		lev.m_fMaxScanRate=m_levels[m_nLevels-1].m_fMaxScanRate*fMultiplyFactor;
		//int nCount=__min(nSize,2<<int(log(qf/lev.m_fMaxScanRate)/log(2.0)+0.9999)), nLim=nCount/4;
		//int nCount=1<<int(log(qf/lev.m_fMaxScanRate)/log(2.0)+0.9999), nLim=nCount/2;
		int nCount=nSize/2, nLim=nCount/2;
		//int nCount=nSize, nLim=nCount/2;
		if (nCount<4) break;
		nAllocs+=nCount;

    lev.m_nSize=nCount;
		lev.m_nBits=(int)(log(nCount)/log(2)+0.5);
    lev.m_fMultiplier=(float)pow(2.0,-31+lev.m_nBits);

		//MakeSincLowpass(filter,129,m_levels[nLastTable].m_fMaxScanRate/lev.m_fMaxScanRate);
		//HammingWindow(filter,129);
		//MakeSincLowpass(filter,FILT_LEN,2*(qf/m_levels[nLastTable].m_nSize)/lev.m_fMaxScanRate);
		//KaiserWindow(filter,FILT_LEN,4.3);

    float *pIn=m_levels[nLastTable].m_pData;
    float *pOut=m_levels[m_nLevels].m_pData=new float[nCount+4];
		int nSize1=m_levels[nLastTable].m_nSize-1;
		int scale=(nSize1+1)/nCount;

		int nFirstSample=(int)(fCrispFactor*nSize*(m_levels[nLastTable].m_fMaxScanRate/lev.m_fMaxScanRate));
		for (int i=0; i<nSize; i++)
			if (i>=__min(nFirstSample,nLim))
				pTemp2[i]=0.0,
				pTemp2[i+nSize]=0.0;
			else
				pTemp2[i]=pTemp[i],
				pTemp2[i+nSize]=pTemp[i+nSize];

		fft_float(nSize,1,pTemp2,pTemp2+nSize,pTemp3,pTemp3+nSize);
		// rfft(pTemp,nCount);

		for (int i=0; i<nCount; i++)
			pOut[i]=pTemp3[i*nSize/nCount];

		pOut[nCount]=pOut[0];
		pOut[nCount+1]=pOut[1];
		pOut[nCount+2]=pOut[2];
		pOut[nCount+3]=pOut[3];
		if (nCount<(nSize1>>1))
			nLastTable=m_nLevels;
		m_nLevels++;
  }

	delete []pTemp3;
	delete []pTemp2;
	delete []pTemp;
	delete []pBase;
/*
	{
		FILE *f=fopen("c:\\allocs.txt","w");
		fprintf(f,"Allocated memory per table = %d bytes",nAllocs*4);
		fclose(f);
	}
  */
} 

