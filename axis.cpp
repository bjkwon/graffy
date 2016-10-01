#include <math.h>
#include "graffy.h"	
#include "supportFcn.h"
#include "audfret.h"

using namespace Win32xx;
#include <iterator>

static map<int, int> rpix, rpix2; // first number: pixel count, second number: number of divisions

void makeRPIX()
{
	rpix.clear();
	rpix2.clear();

	rpix[100] = 2;
	rpix[140] = 3;
	rpix[190] = 4;
	rpix[250] = 5;
	rpix[310] = 6;
	rpix[380] = 7;
	rpix[460] = 8;
	rpix[540] = 9;
	rpix[640] = 10;

	rpix2[40] = 2;
	rpix2[90] = 3;
	rpix2[145] = 4;
	rpix2[200] = 5;
	rpix2[260] = 6;
	rpix2[325] = 7;
	rpix2[375] = 8;
	rpix2[425] = 15;
	rpix2[475] = 19;
	rpix2[525] = 20;
}

CAxis::CAxis()
:colorAxis(0)
{
	type = GRAFFY_axis;
	color = RGB(200, 210, 200); //default
	pos.x0=0.;
	pos.y0=0.;
	pos.width=0.;
	pos.height=0.;
	visible = true;
	xlim[0]=0.;
	ylim[0]=0.;
	xlim[1] = xlim[0]-1.; //reverse the large-small, so indicate uninitialized for lim
	ylim[1] = ylim[0]-1.; //reverse the large-small, so indicate uninitialized for lim
	m_ln.clear();
}

CAxis::CAxis(CWndDlg * base, CGobj* pParent /*=NULL*/)
:colorAxis(0)
{
	m_dlg = base;
	type = GRAFFY_axis;
	color = RGB(200, 210, 200); //default
	pos.x0=0.;
	pos.y0=0.;
	pos.width=0.;
	pos.height=0.;
	hPar=pParent;
	visible = true;
	xlim[0]=0.;
	ylim[0]=0.;
	xlim[1] = xlim[0]-1.; //reverse the large-small, so indicate uninitialized for lim
	ylim[1] = ylim[0]-1.; //reverse the large-small, so indicate uninitialized for lim
	xtick.hPar = ytick.hPar = this;
	xtick.m_dlg = base;
	ytick.m_dlg = base;
	m_ln.clear();
	hPar->child.push_back(this);
}

CAxis::~CAxis()
{
	hPar->child.pop_back();
	while (!m_ln.empty()) {
		delete m_ln.back();
		m_ln.pop_back();
	}
}

GRAPHY_EXPORT CAxis& CAxis::operator=(const CAxis& rhs)
{
	if (this != &rhs) 
	{
		CGobj::operator=(rhs);

		colorAxis = rhs.colorAxis;
		memcpy(xlim, rhs.xlim, sizeof(xlim));
		memcpy(ylim, rhs.ylim, sizeof(ylim));
		memcpy(xlimFull, rhs.xlimFull, sizeof(xlimFull));
		memcpy(ylimFull, rhs.ylimFull, sizeof(ylimFull));
		xtick = rhs.xtick;
		ytick = rhs.ytick;
		m_ln = rhs.m_ln;
	}

	return *this;
}

void CAxis::GetCoordinate(POINT* pt, double& x, double& y)
{
	int ix = pt->x - rcAx.left;
	int iy = rcAx.bottom - pt->y;
	double width = xlim[1]-xlim[0];
	double height = ylim[1]-ylim[0];
	x = ix*width/rcAx.Width() + xlim[0];
	y = iy*height/rcAx.Height() + ylim[0];
}

double qut(double lim[2], double d)
{
	double range = lim[1]-lim[0];
	return (d-lim[0])/range;
}

double CAxis::GetRangePixel(int x)
{
	double relativeVal = (double)(x - rcAx.left)/(double)rcAx.Width();
	double val = xlim[0] + relativeVal * (xlim[1]-xlim[0]);
	return val;
}

int CAxis::double2pixel(double a, char xy)
{
	double relativeVal;
	int extent; // in pixel
	// rcAx field, xlim, ylim must have been prepared prior to this call.
	switch(xy)
	{
	case 'x':
		relativeVal = qut(xlim, a); 
		extent = rcAx.Width();
		return rcAx.left + (int)((double)extent*relativeVal+.5);
	case 'y':
		relativeVal = qut(ylim, a); 
		extent = rcAx.Height();
		return rcAx.bottom - (int)((double)extent*relativeVal+.5);
	default:
		return -9999;
	}
}

POINT CAxis::double2pixelpt(double x, double y, double *newxlim)
{
	// Returns pixel POINT from (x, y) coordinate in double. 
	// calculates how many pixels the point advances rcAx.left and rcAx.top based on xlim, ylim
	// newxlim is used for audio signal chains (the one with null-signal(s) in the middle)
	POINT pt;
	if (newxlim==NULL) pt.x = double2pixel(x,'x');
	else {
		double ratio = (newxlim[1]-newxlim[0])/(xlim[1]-xlim[0]);
		pt.x = rcAx.left; // + GetOffsetPixel(x-xlim[0],'x', ratio);  //now,  how to proceed with ratio??
	}
	pt.y = double2pixel(y,'y');
	return pt;
}

GRAPHY_EXPORT CRect CAxis::GetWholeRect()
{ // Output: CRect of the whole axis area including xtick, ytick
	// Add xtick2 and ytick2 on the right and top sides when needed.
	CRect out(axRect), outxy;
	outxy.UnionRect(xtick.rt,ytick.rt);
	out.UnionRect(out, outxy);
	return out;
}


GRAPHY_EXPORT void CAxis::DeleteLine(int index)
{
	if (index==-1)
		while(m_ln.size()>0)
			m_ln.pop_back();
	else
		m_ln.erase(m_ln.begin()+index);
	if (m_ln.size()==0)
	{
		xlim[1] = xlim[0]-1.;
		ylim[1] = ylim[0]-1.;
	}
}
//
//GRAPHY_EXPORT void CAxis::DeletePatch(int index)
//{
//	if (index==-1)
//		while(m_pat.size()>0)
//			m_pat.pop_back();
//	else
//		m_pat.erase(m_pat.begin()+index);
//	if (m_pat.size()==0)
//	{
//		xlim[1] = xlim[0]-1.;
//		ylim[1] = ylim[0]-1.;
//	}
//}

int CAxis::GetDivCount(char xy, int dimens)
{
	RECT rt;
	GetClientRect(m_dlg->hDlg, &rt);
	int width(rt.right-rt.left);
	int height(rt.bottom-rt.top);
	if (rpix.empty())	makeRPIX();
	int nTicks;
	map<int,int>::key_compare comp;
	map<int,int>::iterator it;
	switch(xy)
	{
	case 'x':
		if (dimens<0) dimens = (int) (width*pos.width+.5);
		it = rpix.begin();	it++;
		if (dimens > rpix.rbegin()->first) 
			nTicks = rpix.rbegin()->second + (dimens-rpix.rbegin()->first)/110;
		else if (dimens <= (*it).first)
			nTicks = (*it).second;
		else
		{
			comp = rpix.key_comp();
			for (it = rpix.begin(); comp((*it).first, dimens); it++)
				;
			it--, nTicks = (*it).second;
		}
		break;
	case 'y':
		if (dimens<0) dimens = (int) (height*pos.height+.5); // axRect is not updated yet (it is updated inside OnPaint)
		it = rpix2.begin();	it++;
		if (dimens > rpix2.rbegin()->first) 
			nTicks = rpix2.rbegin()->second + (dimens-rpix2.rbegin()->first)/70;
		else if (dimens <= (*it).first)
			nTicks = (*it).second;
		else
		{
			comp = rpix2.key_comp();
			for (it = rpix2.begin(); comp((*it).first, dimens); it++)
				;
			it--, nTicks = (*it).second;
		}	
	}
	return nTicks;
}

void CAxis::setxticks()
{
	int nTicks = GetDivCount('x', -1);
	int beginID, nSamples;
	assert(m_ln.size()>0);
	int fs = m_ln.front()->sig.GetFs();
	vector<int> out;
	vector<double> vxdata;
	double percentShown;
	if (!xtick.automatic) return;
	//nTicks is the number of divided parts (i.e., 2 means split by half)
	percentShown = 1. - ( (xlim[0]-xlimFull[0]) + (xlimFull[1]-xlim[1]) ) / (xlimFull[1]-xlimFull[0]);
	nSamples = (int)(percentShown * m_ln.front()->sig.nSamples+.5);
	splitevenindices(out, nSamples, nTicks);
	vxdata.reserve(nSamples);
	if (m_ln.front()->xdata==NULL)
	{
		beginID = (int)ceil(xlim[0]*fs);
		for (int k=beginID; k<beginID+nSamples; k++) vxdata.push_back((double)(k+1)/fs); // no need to check whether it is an audio signal 
	}
	else
	{
		for (beginID=0; beginID<m_ln.front()->sig.nSamples; beginID++)
			if (m_ln.front()->xdata[beginID]>=xlim[0]) 
				break;
		for (int k=beginID; k<beginID+nSamples; k++) 
			vxdata.push_back(m_ln.front()->xdata[k]);
	}
	xtick.set(out, vxdata, nSamples);
}


GRAPHY_EXPORT CLine * CAxis::plot(int length, double *y, COLORREF col, char cymbol, LineStyle ls)
{
	int i;
	CLine * out;
	double *x = new double[length];
	for (i=0; i<length; i++)
		x[i] = (double)i;
	out = plot(length, x, y, col, cymbol, ls);
	delete[] x;
	return out;
}



GRAPHY_EXPORT CLine * CAxis::plot(double *xdata, CSignal &ydata, COLORREF col, char cymbol, LineStyle ls)
{
	double maax(-1.e100), miin(1.e100);
	CLine *in = new CLine(m_dlg, this, 0);
	in->sig = ydata;
	double dfs = (double)ydata.GetFs();
	xlim[0] = 0; xlim[1]=-1;
	if (xdata!=NULL)
	{
		delete[] in->xdata;
		in->xdata = new double[ydata.nSamples];
		memcpy(in->xdata, xdata, sizeof(double)*ydata.nSamples);
	}
	else
		in->xdata = NULL;
	for (CSignal *p = &ydata; p; p=p->chain)
	{
		if (xdata==NULL)	{
			if (ydata.GetType()==CSIG_AUDIO)	
				xlim[1] = max( xlim[1], (p->dur()+p->tmark)/1000 ); 
			else 
				xlim[0] = 1, xlim[1] = max( xlim[1], (double)p->nSamples ) ; 
		}
		else
		{
			xlim[0] = getMin(ydata.nSamples, xdata);
			xlim[1] = getMax(ydata.nSamples, xdata);	
			if (xlim[0]==xlim[1]) {xlim[0] -= .005;	xlim[1] += .005;}
		}
	}
	in->symbol = cymbol;
	in->color = col;
	in->lineStyle = ls;
	m_ln.push_back(in);

	xlimFull[0] = xlim[0]; xlimFull[1] = xlim[1]; 
	RECT rt;
	GetClientRect(m_dlg->hDlg, &rt);
	int width(rt.right-rt.left);
	int axWidth((int)((double)width*pos.width+.5));
	int height(rt.bottom-rt.top);
	int axHeight((int)((double)height*pos.height+.5));
	if (ydata.GetType()==CSIG_AUDIO) 
		ylim[0]=-1, ylim[1]=1;
	else	
	{
		ylim[0] = getMin(ydata.nSamples, ydata.buf);
		ylim[1] = getMax(ydata.nSamples, ydata.buf);	
		//double diff = ylim[1]-ylim[0];
		//ylim[0] -= diff/10; 
		//ylim[1] += diff/10;
	}
	ylimFull[0] = ylim[0]; ylimFull[1] = ylim[1]; 
	// parent is Figure object, whose parent is root object whose member is m_dlg
	m_dlg->InvalidateRect(NULL);
	return in; //This is the line object that was just created.
}


GRAPHY_EXPORT CLine * CAxis::plot(int length, double *x, double *y, COLORREF col, char cymbol, LineStyle ls)
{
	double maax(-1.e100), miin(1.e100);
	CLine *in = new CLine(m_dlg, this, 0);
	memcpy((void*)in->xdata,(void*)x, sizeof(double)*length);
	memcpy((void*)in->ydata,(void*)y, sizeof(double)*length);
	in->id0 = 0;
	in->id1 = length-1;
	in->symbol = cymbol;
	in->color = col;
	in->lineStyle = ls;
	// parent is Figure object, whose parent is root object whose member is m_dlg
	m_dlg->InvalidateRect(NULL);
	m_ln.push_back(in);
	for (vector<CLine*>::iterator it=m_ln.begin(); it!=m_ln.end(); it++) 
		miin = min(miin, getMin((*it)->orglength(), (*it)->xdata));
	for (vector<CLine*>::iterator it=m_ln.begin(); it!=m_ln.end(); it++) 
		maax = max(maax, getMax((*it)->orglength(), (*it)->xdata));
	xlimFull[0] = miin; xlimFull[1] = maax; 
	for (vector<CLine*>::iterator it=m_ln.begin(); it!=m_ln.end(); it++) 
		miin = min(miin, getMin((*it)->orglength(), (*it)->xdata));
	for (vector<CLine*>::iterator it=m_ln.begin(); it!=m_ln.end(); it++) 
		maax = max(maax, getMax((*it)->orglength(), (*it)->xdata));
	ylimFull[0] = miin; ylimFull[1] = maax; 
	return in;//This is the line object that was just created.
}
//
//GRAPHY_EXPORT CPatch * CAxis::plot(int length, double *x, double *y, COLORREF col)
//{
//	double maax(-1.e100), miin(1.e100);
//	CPatch in(m_dlg, this, length);
//	memcpy((void*)in.xdata,(void*)x, sizeof(double)*length);
//	memcpy((void*)in.ydata,(void*)y, sizeof(double)*length);
//	in.color = col;
//	in.len = length;
//	// parent is Figure object, whose parent is root object whose member is m_dlg
//	m_dlg->InvalidateRect(NULL);
//	m_pat.push_back(in);
//	return &in;//This is the line object that was just created.
//}

void CAxis::GetRange(double *x, double *y)
{
	memcpy((void*)x,(void*)xlim, 2*sizeof(double));
	memcpy((void*)y,(void*)ylim, 2*sizeof(double));
}


int CAxis::timepoint2pix(double timepoint)
{
	double proportion = (timepoint-xlim[0]) / (xlim[1]-xlim[0]);// time proportion re the displayed duration
	int pix_proportion = (int)((double)axRect.Width() * proportion);
	return axRect.left + pix_proportion;
}

double CAxis::pix2timepoint(int pix)
{
	double proportion; // time proportion re the displayed duration
	int ss=axRect.left;
	int ss2=axRect.Width();		
	proportion = (double) (pix-axRect.left) / (double)axRect.Width();
	return xlim[0] + (xlim[1]-xlim[0])*proportion;
}