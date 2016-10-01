#include "graffy.h"

CLine::CLine(CWndDlg * base, CGobj * pParent, int length)
: symbol('\0'), lineWidth(1), id0(0), markersize(4), markerColor(-1)
{
	m_dlg = base;
	hPar=pParent;
	color = RGB(0, 0, 200);
	id1 = max(length-1,0);
	len = length;
	xdata = new double[len];
	ydata = new double[len];
	type = GRAFFY_line;
	hPar->child.push_back(this);
}

CLine::~CLine() 
{	
	hPar->child.pop_back();
	delete[] xdata;  delete[] ydata; 
}
