
/*	CNoise
 *
 *		Fast noise generator
 */

struct CNoise {

	double	fY1;
	long	iStat;


	CNoise() {

		iStat	 = 0x16BA2118;
		fY1		 = 0.0; }


	void Clear() {

		fY1 = 0.0; }


	double GetWhiteSample() {

		iStat = iStat * 1103515245 + 12345;
		return (double)iStat * (1.0 / 0x80000000); }


	double GetBlackSample(double a) {
		return (fY1 = fY1 * a + GetWhiteSample() * (1.0-a)); } 


	void WorkSamples(float *pout, int const ns, double const amp=1.0) {

		long		 s = iStat;
		double const a = amp * (1.0 / 0x80000000);

		if (ns >= 4) {
			int cnt = ns >> 2;
			do {
				s = s * 1103515245 + 12345;
				*pout++ = (float)((double)s * a);
				s = s * 1103515245 + 12345;
				*pout++ = (float)((double)s * a);
				s = s * 1103515245 + 12345;
				*pout++ = (float)((double)s * a);
				s = s * 1103515245 + 12345;
				*pout++ = (float)((double)s * a);
			} while(--cnt); }
		int cnt = ns & 3;
		while(cnt--) {
			s = s * 1103515245 + 12345;
			*pout++ = (float)((double)iStat * a); }

		iStat = s; }
 };

