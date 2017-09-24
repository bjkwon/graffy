#pragma once

#include "wxx_wincore.h" // Win32++ 8.2. This must be placed prior to <windows.h> to avoid including winsock.h

#ifndef SIGPROC
#include "sigproc.h"
#endif


#ifdef GRAFFY_STATIC //====================== STATIC LIBRARY
#define GRAPHY_EXPORT
#else

#ifdef GRAFFY_INTERNAL //====================== GRAFFY_INTERNAL
#define GRAPHY_EXPORT __declspec (dllexport)
#else
#define GRAPHY_EXPORT __declspec (dllimport)
#endif                //====================== GRAFFY_INTERNAL

#endif               //====================== STATIC LIBRARY


#define GRAPHY_REPLACE_SAME_KEY		(DWORD)0x0000
#define GRAPHY_REPLACE_NO_KEY		(DWORD)0x0001
#define GRAPHY_RETURN_ERR_SAME_KEY	(DWORD)0x0002

#define FONT_STYLE_NORMAL	0
#define FONT_STYLE_ITALIC	1
#define FONT_STYLE_BOLD		2
#define FONT_STYLE_UNDERLINE	4	
#define FONT_STYLE_STRIKEOUT	8	


#define WM_GCF_UPDATED		WM_APP+752
#define WM_PARENT_THREAD	WM_APP+753


#ifndef GRAFFY // ======== CLASS DEFINITIONS FOR graffy

#include "WndDlg0.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif
//#include "wincore.h" // OLD WIN32+
//#include "winutils.h"  // OLD WIN32+


// CFigure objects are instantiated ("new'ed") during instantiation of CPlotDlg
// CAxis objects are instantiated by CFigure::axes() 

// CFigure, CAxis objects have the proper m_dlg member variable during their instantiation.

class CPosition
{
public:
	//all the values in this class is scaled by 1 ("normalized" as in Matlab)
	double x0;
	double y0;
	double width;
	double height;
	CPosition(double x, double y, double w, double h);
	CPosition();
	CRect GetRect(CRect windRect);
	void Set(CRect windRect, CRect axRect);
	void AdjustPos2FixLocation(CRect oldrt, CRect newrt);
};

enum graffytype: char
{ 
	GRAFFY_root='r',
	GRAFFY_figure='f',
	GRAFFY_axis='a',
	GRAFFY_text='t',
	GRAFFY_patch='p',
	GRAFFY_line='l',
	GRAFFY_tick='k',
};


enum LineStyle: unsigned _int8
{ 
	LineStyle_noline = 0,
	LineStyle_solid, 
	LineStyle_dash, 
	LineStyle_dot, 
	LineStyle_dashdot, 
};

class CGobj
{
public:
	COLORREF color;
	CPosition pos;
	CGobj *hPar;
	CGobj *hChild; // this is a linked list, different from vector child, used only for special purpose (for example, to keep a direct genealogy line)
	CWndDlg *m_dlg;
	bool visible;
	graffytype type;
	vector<CGobj *> child;
	CGobj();
	virtual ~CGobj() {};
	GRAPHY_EXPORT CGobj& operator=(const CGobj& rhs);
	GRAPHY_EXPORT void setPos(double x0, double y0, double width, double height);
	void setPos(CPosition &posIn);
};

class CTick : public CGobj
{
public:
	bool automatic; 
	POINT gap4next;
	int size; // in pixel
	int labelPos; // in pixel
	char format[16];
	double mult;
	double offset;
	CFont font;
	CRect rt; // the region where ticks are drawn
	vector<double> tics1;
	//set() generates tic vector and fills tics1
	void set(vector<int> val, vector<double> xydata, int len); // val.size() is the number of division (i.e., 2 means split by half)
	void extend(bool direction, double xlim);
	CTick(CWndDlg * base=NULL);
	~CTick();
};

class CLine : public CGobj
{ //The class constitutes the line object with the whole duration. id0 and id1 sets the viewing portion.
public:
//	list<map<double,double>> chain; // this is the same meaning as chain in CSignals
	double *xdata, *ydata;
	CSignal sig;
	int id0, id1;
	char symbol; // marker symbol
	unsigned _int8 lineWidth;
	unsigned _int8 markersize;
	COLORREF markerColor;
	LineStyle lineStyle;

	CLine(CWndDlg * base, CGobj* pParent = NULL, int len=0);   // standard constructor
	~CLine();

	int orglength() {return len;};
private:
	int len;
};

class CText : public CGobj
{
public:
	char fontname[32];
	bool italic;
	bool bold;
	bool underline;
	bool strikeout;
	DWORD alignmode;
	int fontsize; // in pixel
	CRect textRect; // in Client coordiate
	CFont font;
	string str;
	GRAPHY_EXPORT HFONT ChangeFont(LPCTSTR fontName, int fontSize=15, DWORD style=0);
	GRAPHY_EXPORT int GetAlignment(string &horizontal, string &vertical);
	GRAPHY_EXPORT int SetAlignment(const char *alignmodestr);
	CText(CWndDlg * base, CGobj* pParent = NULL, const char* strInit=NULL, CPosition posInit=CPosition(0,0,0,0));   // standard constructor
	~CText();   
};

class CAxis : public CGobj
{
public:
	COLORREF colorAxis;
	double xlim[2], ylim[2];
	double xlimFull[2], ylimFull[2];
	CTick xtick; // How do I pass the argument for instantiation of these objects?
	CTick ytick; // CTick xtick(this) didn't work..... Maybe during instantiation of CAxis set xtick.m_dlg = this...
	vector<CLine*> m_ln;
//	vector<CPatch*> m_pat;

	GRAPHY_EXPORT CAxis& operator=(const CAxis& rhs);
	GRAPHY_EXPORT CAxis *create_linked_axis(CPosition pos);
	void GetCoordinate(POINT* pt, double& x, double& y);
	GRAPHY_EXPORT CLine * plot(int length, double *y, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
	GRAPHY_EXPORT CLine * plot(int length, double *x, double *y, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
	GRAPHY_EXPORT CLine * plot(double *xdata, CSignal &ydata, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
	GRAPHY_EXPORT CLine * plot(double *xdata, CSignals &ydata, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid) { return plot(xdata, (CSignal)ydata,col,cymbol,ls);}
//	GRAPHY_EXPORT CPatch * plot(int length, double *x, double *y, COLORREF col);
	void GetRange(double *x, double *y);
	GRAPHY_EXPORT void DeleteLine(int index);
//	GRAPHY_EXPORT void DeletePatch(int index);
	GRAPHY_EXPORT CRect GetWholeRect(); // Retrieve the whole area including xtick, ytick
	void setxticks();
	void setRangeFromLines(char xy);
	int GetDivCount(char xy, int dimens);
	POINT double2pixelpt(double x, double y, double *newxlim);
	int double2pixel(double a, char xy);
	double GetRangePixel(int x);
	double pix2timepoint(int pix);
	int timepoint2pix(double timepoint);
	GRAPHY_EXPORT void setRange(const char xy, double x1, double x2);
	GRAPHY_EXPORT void setTick(const char xy, double const x1, double const x2, const double step, char const *format=NULL, double const coeff=1., double const offset=0.);
	CAxis(CWndDlg * base, CGobj* pParent = NULL);   // standard constructor
	CAxis();
	~CAxis();
	CRect axRect;
	CRect rcAx;
};

class CFigure : public CGobj
{
public:
	void DeleteAxis(int index);
	vector<CText*> text;
	vector<CAxis*> ax;
	CText *AddText(const char* string, CPosition pos);
	CAxis *axes(double x0, double y0, double width, double height);
	CAxis *axes(CPosition pos);
	RECT GetRect(CPosition pos);
	CFigure();   
	CFigure(CWndDlg * base, CGobj* pParent = NULL);   // standard constructor
	~CFigure();
};



#endif //GRAFFY


#define WM__VAR_CHANGED			WM_APP+811
#define WM__PLOTDLG_CREATED		WM_APP+823
#define WM__PLOTDLG_DESTROYED	WM_APP+824

struct GRAFWNDDLGSTRUCT
{
	//input parameters
	CRect rt;
	string caption;
	string lineSpecifer;
	int devID;
	double block;
	HWND hWndAppl;
	DWORD threadCaller;
	//output parameters
	DWORD threadPlot;
	HANDLE fig;
	HACCEL hAccel;
	CFigure *cfig;
};


GRAPHY_EXPORT HANDLE  FindFigure(const char* caption, int *nFigs=NULL);
GRAPHY_EXPORT vector<HANDLE> graffy_Figures();
GRAPHY_EXPORT HANDLE GCF(CSignals *figID);
GRAPHY_EXPORT HANDLE GetGraffyHandle(int figID);
GRAPHY_EXPORT HANDLE GetGraffyHandle(HWND h);
GRAPHY_EXPORT HANDLE GCA(HANDLE _fig);
GRAPHY_EXPORT int GetFigID(HANDLE h, CSignals& out);
GRAPHY_EXPORT int GetFigID(HWND h, CSignals& out);
GRAPHY_EXPORT int CloseFigure(HANDLE h);


#ifdef _WIN32XX_WINCORE_H_
#define NO_USING_NAMESPACE
GRAPHY_EXPORT HANDLE OpenGraffy(CRect rt, const char *caption, DWORD threadID, HWND hApplDlg, GRAFWNDDLGSTRUCT &in);

GRAPHY_EXPORT HANDLE  OpenFigure(CRect *rt, HWND hWndAppl, int devID, double block);
GRAPHY_EXPORT HANDLE  OpenFigure(CRect *rt, const char *caption, HWND hWndAppl, int devID, double block);
#endif 



GRAPHY_EXPORT HANDLE	AddAxis(HANDLE fig, double x0, double y0, double wid, double hei);
GRAPHY_EXPORT HANDLE	AddText (HANDLE fig, const char* text, double x, double y, double wid, double hei);
GRAPHY_EXPORT HANDLE	PlotDouble(HANDLE ax, int len, double *x, double *y=NULL, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
GRAPHY_EXPORT HANDLE	PlotCSignals(HANDLE ax, CSignal &data, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
GRAPHY_EXPORT HANDLE	PlotCSignals(HANDLE ax, CSignals &data, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
GRAPHY_EXPORT HANDLE	PlotCSignals(HANDLE ax, CSignals &dataX, CSignals &dataY, COLORREF col=0xff0000, char cymbol=0, LineStyle ls=LineStyle_solid);
GRAPHY_EXPORT void		SetRange(HANDLE ax, const char xy, double x1, double x2);

GRAPHY_EXPORT HACCEL GetAccel(HANDLE hFig);
GRAPHY_EXPORT HWND GetHWND_PlotDlg(HANDLE hFig);
GRAPHY_EXPORT HWND GetHWND_PlotDlg2(HANDLE hFig);
GRAPHY_EXPORT void SetHWND_GRAFFY(HWND hAppl);
GRAPHY_EXPORT HWND GetHWND_GRAFFY ();
GRAPHY_EXPORT void deleteObj (HANDLE h);

vector<double> makefixtick(double _x1, double _x2, int count);

