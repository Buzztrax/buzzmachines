/*
 *		Dsp Classes
 *
 *			Written by George Nicolaidis aka Geonik
 */

#ifndef inc_dspcBuffer
#define inc_dspcBuffer


/*	CBuffer
 *
 *		A circular buffer with independant read and write positions
 */

struct CBuffer {

	float	*pBuffer;
	int		 iLength;
	int		 iReadPos;
	int		 iWritePos;
	int		 iSyncDistance;

				 CBuffer();
				~CBuffer();
	void		 Init(int const length);
	void		 SetSyncDistance(int const sync) { iSyncDistance = sync; };
	void		 Free();
	void		 Clear();
	inline void	 Clear(int numsamples);
	inline void	 Read(float *source, int numsamples);
	inline void	 Write(float *dest, int numsamples);
	inline void	 Write(CBuffer &dest, int numsamples);
	inline void	 Synchronize();
	inline void	 IterateAll(void *f(const int c, const float s));
 };


CBuffer::CBuffer() {

		// Constructor

	pBuffer = NULL;
	iLength = 0; }


CBuffer::~CBuffer() {

		// Destructor

	Free(); }


void CBuffer::Init(int const len) {
	
		// Allocate the buffer

	assert(!pBuffer);
	pBuffer	= new float[len];
	iLength			= len;
	iReadPos		= 0;
	iSyncDistance	= 0;
	iWritePos		= 0; }


void CBuffer::Free() {

		// Cleanup

	iLength = iReadPos = iWritePos = 0;
	if(pBuffer) delete[] pBuffer;
	pBuffer = NULL; }


void CBuffer::Clear() {

		// Zero buffer contents

	memset(pBuffer,0,iLength*sizeof(float)); }


void CBuffer::Clear(int ns) {
	
		// Put ns zeros

	int	c = min(ns,iLength - iWritePos); ns -= c;
	float *b = pBuffer + iWritePos; iWritePos += c;
	while(c--) *b++ = 0;
	if(ns > 0) {
		iWritePos = ns;
		b = pBuffer;
		while(ns--) *b++ = 0; } }


void CBuffer::Read(float *s, int ns) {
	
		// Read ns samples into the buffer

	int	c = min(ns,iLength - iWritePos); ns -= c;
	float *b = pBuffer + iWritePos; iWritePos += c;
	while(c--) *b++ = *s++;
	if(ns > 0) {
		iWritePos = ns;
		b = pBuffer;
		while(ns--) *b++ = *s++; } }


void CBuffer::Write(float *d, int ns) {
	
		// Write ns samples to the destination
	
	int	c = min(ns,iLength - iReadPos); ns -= c;
	float *b = pBuffer + iReadPos; iReadPos += c;
	while(c--) *d++ = *b++;
	if(ns > 0) {
		iReadPos = ns;
		b = pBuffer;
		while(ns--) *d++ = *b++; } }


inline void CBuffer::Write(CBuffer &pD, int ns) {

		// Write ns samples to the destination
		// ns <= this, this <= pD

	int	a = min(ns,iLength - iReadPos);
	pD.Read(pBuffer + iReadPos, a); ns -=a;
	if(ns > 0) {
		pD.Read(pBuffer, ns);
		iReadPos = ns; }
	else {
		iReadPos += a; } }


void CBuffer::Synchronize() {
	
		// Synchronize read pos to write pos
	
	iReadPos = iWritePos + iSyncDistance;
	if(iReadPos > iLength) iReadPos -= iLength; }


void CBuffer::IterateAll(void *f(const int c, const float s)) {

		// Call f for every sample
	
	float *b = pBuffer + iWritePos;
	for(int c = 0; c < iLength - iWritePos; c++) f(c,*b++);
	for(b = pBuffer; c < iLength; c++) f(c,*b++); }

#endif
