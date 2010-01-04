/*
 *		DspAdd
 */

#pragma optimize ("a", on)

#define UNROLL		4

void DspAdd(float *pout, float const *pin, dword const n) {
	if (n >= UNROLL) {
		int c = n / UNROLL;
		do {
			pout[0] += pin[0];
			pout[1] += pin[1];
			pout[2] += pin[2];
			pout[3] += pin[3];
			pin += UNROLL;
			pout += UNROLL; } while(--c); }
	int c = n & (UNROLL-1);
	while(c--)
		*pout++ += *pin++; }

#pragma optimize ("a", off)

