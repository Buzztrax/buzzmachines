// Marcin Dabrowski
// bigyo@wp.pl
// www.patafonia.co.nr

#ifndef __LINLOG_H
#define __LINLOG_H
///////////////////////////////////////////////////////////////////////////

inline double linlog(double value, double min, double max, double slope)
{
	double delta_x, t, A, env;

	if (max>min)
	{
		delta_x = max - min;
		t = (value - min) / delta_x;
		t = 1- t ;
	}
	else
	{
		delta_x = min - max;
		t = (value - max) / delta_x;
	}

	if (slope <= 0.5)
	{
		A = 0.25/(slope*slope);
		env = (1-t) * pow(A,-t);
	}
	else // slope = (0.5 .. 1)
	{
		A = 0.25/((1-slope)*(1-slope));
		env = 1-t*pow(A,(t-1));
	}

	return env * (delta_x) + min;
}
///////////////////////////////////////////////////////////////////////////
#endif