#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"
#include <complex>

using namespace std;

typedef complex<float> ZComplex;

// FFT routine copied from my old OneSignal library, not identical to original Infector's which was some public domain code
// perhaps not as fast either, but it's not exactly time-critical code anyway
template<int O> class ZOSFFT
{
        int m_anScramble[1<<O];
        ZComplex m_acSines[1<<O];
public:
        ZOSFFT()
        {
                int N=1<<O;
                for (int i=0; i<N; i++)
                {
                        int v=0;
                        for (int j=0; j<O; j++)
                                if (i&(1<<j))
                                        v+=(N>>(j+1));
                        m_anScramble[i]=v;
                        m_acSines[i]=ZComplex(cos(2*PI*i/N),sin(2*PI*i/N));
                }
        }
        void FFT(ZComplex *pInput, ZComplex *pOutput, bool bInverse)
        {
                int N=1<<O;
                int N1=N-1;
                int i;
                // Scramble the input data
                if (bInverse)
                {
                        float mf=1.0/N;
                        for (i=0; i<N; i++)
                        {
                                ZComplex &c=pInput[m_anScramble[i]];
                                pOutput[i]=mf*ZComplex(c.imag(),c.real());
                        }
                }
                else
                        for (i=0; i<N; i++)
                                pOutput[i]=pInput[m_anScramble[i]];

                // O butterfiles
                for (i=0; i<O; i++)
                {
                        int PO=1<<i, PNO=1<<(O-i-1);
                        int j,k;
                        for (j=0; j<PNO; j++)
                        {
                                int base=j<<(i+1);
                                for (k=0; k<PO; k++)
                                {
                                        int B1=base+k;
                                        int B2=base+k+(1<<i);
                                        ZComplex r1=pOutput[B1];
                                        ZComplex r2=pOutput[B2];
                                        pOutput[B1]=r1+r2*m_acSines[(B1<<(O-i-1))&N1];
                                        pOutput[B2]=r1+r2*m_acSines[(B2<<(O-i-1))&N1];
                                }
                        }
                }
                if (bInverse)
                {
                        for (i=0; i<N; i++)
                        {
                                ZComplex &c=pOutput[i];
                                pOutput[i]=ZComplex(c.imag(),c.real());
                        }
                }
        }
};

CBandlimitedTable::CBandlimitedTable()
{
  m_nLevels=0;
  m_pBuffer=NULL;
  m_nBufSize=0;
}

CBandlimitedTable::~CBandlimitedTable()
{
  for (int i=0; i<m_nLevels; i++)
    delete []m_levels[i].m_pData;
  m_nLevels=0;
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

static ZOSFFT<11> fft;

void CBandlimitedTable::Make(float fMultiplyFactor, float fMaxScanRate, float fCrispFactor)
{
  if (fCrispFactor==-1)
    fCrispFactor=2.0f/fMultiplyFactor;
  for (int i=0; i<m_nLevels; i++)
    delete []m_levels[i].m_pData;
  m_nLevels=0;

  int nSize=2048;
  m_levels[0].m_nSize=2048;
  m_levels[0].m_nBits=11;
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

  static ZComplex base[2048], spec[2048], speclim[2048], genwave[2048];
  for (int i=0; i<2048; i++)
    base[i]=m_pBuffer[i];

  fft.FFT(base, spec, false);

  m_nLevels=1;
  
  int nLastTable=0;

  int nAllocs=0;

  while(m_levels[m_nLevels-1].m_fMaxScanRate<=fMaxScanRate)
  {
    CAnyWaveLevel &lev=m_levels[m_nLevels];
    lev.m_fMaxScanRate=m_levels[m_nLevels-1].m_fMaxScanRate*fMultiplyFactor;
    int nCount=nSize/2, nLim=nCount/2;
    if (nCount<4) break;
    nAllocs+=nCount;

    lev.m_nSize=nCount;
    lev.m_nBits=(int)(log(nCount)/log(2)+0.5);
    lev.m_fMultiplier=(float)pow(2.0,-31+lev.m_nBits);

    float *pIn=m_levels[nLastTable].m_pData;
    float *pOut=m_levels[m_nLevels].m_pData=new float[nCount+4];
    int nSize1=m_levels[nLastTable].m_nSize-1;
    int scale=(nSize1+1)/nCount;

    int nFirstSample=(int)(fCrispFactor*nSize*(m_levels[nLastTable].m_fMaxScanRate/lev.m_fMaxScanRate));
    for (int i=0; i<nSize; i++)
      if (i>=__min(nFirstSample,nLim))
        speclim[i]=0.0;
      else
        speclim[i]=spec[i];

    fft.FFT(speclim, genwave, true);

    for (int i=0; i<nCount; i++)
      pOut[i]=genwave[i*nSize/nCount].real();

    pOut[nCount]=pOut[0];
    pOut[nCount+1]=pOut[1];
    pOut[nCount+2]=pOut[2];
    pOut[nCount+3]=pOut[3];
    if (nCount<(nSize1>>1))
      nLastTable=m_nLevels;
    m_nLevels++;
  }
} 

