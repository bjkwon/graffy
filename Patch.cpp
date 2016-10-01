#include "graffy.h"

CPatch::CPatch(CWndDlg * base, CGobj * pParent, int length, double *xdata, double *ydata, COLORREF color)
{
	m_dlg = base;
	hPar=pParent;
	color = RGB(0, 0, 0);
	len = length;
	xdata = new double[len];
	ydata = new double[len];
	type = GRAFFY_patch;
}

CPatch::~CPatch()
{
	delete[] xdata;
	delete[] ydata;
}
