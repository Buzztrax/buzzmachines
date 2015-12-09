#define M_PI 3.14159265358979323846
static void IFFT(float *fftBuffer, long fftFrameSize, long sign)

/* 
FFT routine, (C)1996 S.M.Sprenger. Sign = -1 is FFT, 1 is iFFT (inverse)
Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes and returns the cosine and sine parts in an interleaved manner, ie. fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize must be a power of 2. It expects a complex input signal (see footnote 2), ie. when working with 'common' audio signals our input signal has to be passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	long i, bitm, j, le, le2, k;
 
	for (i = 2; i < 2*fftFrameSize-2; i += 2) {
		for (bitm = 2, j = 0; bitm < 2*fftFrameSize; bitm <<= 1) {
			if (i & bitm) j++;
			j <<= 1;
		}
		if (i < j)
		{
			p1 = fftBuffer+i; p2 = fftBuffer+j;
			temp = *p1; *(p1++) = *p2;
			*(p2++) = temp; temp = *p1;
			*p1 = *p2; *p2 = temp;
		}
	}
	for (k = 0, le = 2; k < log((float)fftFrameSize)/log(2.0f); k++) {
		le <<= 1;
		le2 = le>>1;
		ur = 1.0;
		ui = 0.0;
		arg = (float)M_PI / (le2>>1);
		wr = (float)cos(arg);
		wi = sign*(float)sin(arg);
		for (j = 0; j < le2; j += 2) {
			p1r = fftBuffer+j; p1i = p1r+1;
			p2r = p1r+le2; p2i = p2r+1;
			for (i = j; i < 2*fftFrameSize; i += le) {
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr; *p2i = *p1i - ti;
				*p1r += tr; *p1i += ti;
				p1r += le; p1i += le;
				p2r += le; p2i += le;
			}
			tr = ur*wr - ui*wi;
			ui = ur*wi + ui*wr;
			ur = tr;
		}
	}
}