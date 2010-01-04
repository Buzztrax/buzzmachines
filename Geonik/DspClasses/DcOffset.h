/*
 *		Dsp Classes : Dc Offset removal
 *
 *			Written by George Nicolaidis aka Geonik
 */

#ifndef inc_dspcDcOffset
#define inc_dspcDcOffset


//	CDcBlock
//
//		DC Removal class
//

struct CDcBlock {

	double fX1,fY1;

	CDcBlock() {

		fX1	= 0;
		fY1	= 0; }


	void Clear() {

		fX1	= 0;
		fY1	= 0; }


	double Work(double const fIn) {

		double fOut = fIn - fX1 + (0.99 * fY1);
		fX1 = fIn; fY1 = fOut;
		return fOut; }


	void WorkSamples(float *pb, int const ns) {

		double x1 = fX1;
		double y1 = fY1;
		int c = ns;
		do {
			double x = *pb;
			double y = (float)(x - x1 + (0.99*y1));
			*pb++ = (float)y;
			y1 = y;
			x1 = x; } while(--c);
		fX1 = x1;
		fY1 = y1; } 
};


#endif
