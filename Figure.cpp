#include "graffy.h"	
#include "supportFcn.h"

CGobj::CGobj()
:m_dlg(NULL), hPar(NULL), visible(true)
{
	type = GRAFFY_root;
}

GRAPHY_EXPORT void CGobj::setPos(double x0, double y0, double width, double height)
{
	pos.x0 = x0;
	pos.y0 = y0;
	pos.width = width;
	pos.height  = height;
	
}

void CGobj::setPos(CPosition &posIn)
{
	pos = posIn;
}

GRAPHY_EXPORT CGobj& CGobj::operator=(const CGobj& rhs)
{
	if (this != &rhs) 
	{
		color = rhs.color;
		pos = rhs.pos;
		hPar = rhs.hPar;
		m_dlg = rhs.m_dlg;
		visible = rhs.visible;
		type = rhs.type;
	}
	return *this;
}

CPosition::CPosition()
{

}

CPosition::CPosition(double x, double y, double w, double h)
:x0(x), y0(y), width(w), height(h)
{
}

void CPosition::AdjustPos2FixLocation(CRect oldrt, CRect newrt)
{

}

CRect CPosition::GetRect(CRect windRect)
{ // from pos to RECT
	POINT org;
	int windWidth, windHeight, axHeight, axWidth;
	windWidth = windRect.right-windRect.left;
	windHeight= windRect.bottom-windRect.top;
	org.x = (int)((double)windRect.left + (double)windRect.Width()*x0+.5);
	org.y = (int)((double)windRect.bottom - (double)windRect.Height()*y0+.5);
	axWidth = (int)((double)windRect.Width()*width+.5);
	axHeight = (int)((double)windRect.Height()*height+.5);
	CRect rctAx(org, CPoint(org.x+axWidth, org.y-axHeight));
	return rctAx;
}

void CPosition::Set(CRect windRect, CRect axRect)
{ // from RECT to pos
	width = (double)axRect.Width()/windRect.Width();
	height = (double)axRect.Height()/windRect.Height();
	x0 = (double)(axRect.left-windRect.left) / windRect.Width();
	y0 = (double)(windRect.bottom-axRect.bottom) / windRect.Height();
}

CFigure::CFigure(CWndDlg * base, CGobj* pParent)
{
	m_dlg = base;
	hPar = pParent;
	color = RGB(230, 230, 210);
	type = GRAFFY_figure;
	hPar->child.push_back(this);
}

CFigure::~CFigure()
{
	hPar->child.pop_back();
	while (!ax.empty()) {
		delete ax.back();
		ax.pop_back();
	}
	while (!text.empty()) {
		delete text.back();
		text.pop_back();
	}
	m_dlg=NULL;
}

CAxis *CFigure::axes(CPosition pos)
{
	CAxis *in = new CAxis(m_dlg, this);
	in->setPos(pos);
	ax.push_back(in);
	return in;
} 

CAxis *CFigure::axes(double x0, double y0, double width, double height)
{
	CAxis *in = new CAxis(m_dlg, this);
	in->setPos(x0, y0, width, height);
	ax.push_back(in);
	return in;
}

void CFigure::DeleteAxis(int index)
{
	ax.pop_back();
}

CText *CFigure::AddText(const char* strIn, CPosition pos)
{
	CText *in = new CText(m_dlg, this, strIn, pos);
	text.push_back(in);
	return in;
}

RECT CFigure::GetRect(CPosition pos)
{
	RECT rt, tp;
	HWND h = GetHWND_PlotDlg((HANDLE)this);
	GetClientRect(h,&rt);
	int width = (rt.right-rt.left);
	int height = (rt.bottom-rt.top);
	tp.left = rt.left + (int)(width * pos.x0+.5);
	tp.top = rt.top + (int)(height * pos.y0+.5);
	tp.right = tp.left + (int)(width * pos.width+.5);
	tp.bottom = tp.top+ (int)(height* pos.height+.5);

	return tp;
}