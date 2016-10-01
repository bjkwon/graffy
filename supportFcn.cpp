#include "audfret.h"
//#include "StdString.h"

int iabs(int x)
{
	if (x<0)		return -x;
	return x;
}

int LRrange(RECT* rect, int var, char xy)
{
	// 1 for R, -1 for L, 0 for between    (x)
	// 1 for above, -1 for below, 0 for bet (y)
	switch(xy)
	{
	case 'x':
		if (var > rect->right)	return 1;
		else if (var < rect->left)	return -1;
		else return 0;
	case 'y':
		if (var > rect->bottom)	return -1;
		else if (var < rect->top)	return 1;
		else return 0;
	default:
		return -9999;
	}
}

int IsInsideRect(RECT* rect, POINT* pt)
{
	return ( pt->x >= rect->left && pt->x <= rect->right && pt->y >= rect->top && pt->y <= rect->bottom );
}

double getMin(int len, double *x)
{
	double miin=1.e100;
	for (int i=0; i<len; i++)
	{
		if (x[i]<miin)
			miin = x[i];
	}
	return miin;
}

double getMax(int len, double *x)
{
	double maax=-1.e100;
	for (int i=0; i<len; i++)
		if (x[i]>maax)
			maax = x[i];
	return maax;
}

double getMean(int len, double *x)
{
	double sum(0.);
	for (int i=0; i<len; i++) sum += x[i];
	return sum/len;
}
