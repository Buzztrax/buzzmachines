
/*	CJetTable 
 *
 *		Looks up x^3 - x
 */


struct CJetTable {
	double LookupSample(double const s) {
		double t = s * (s * s - 1.0);
		if(t >  1.0) t =  1.0;
		else if(t < -1.0) t = -1.0;
		return t; }
};

struct CBowTable{
	double fSlope;

	CBowTable() {
		fSlope = 0.1; }

	void SetSlope(double s) {
		fSlope = s; }

	double LookupSample(double const s) {
		double t = pow(fabs(s*fSlope) + 0.75,-4.0);
		return (t > 1.0 ? 1.0 : t); }
};