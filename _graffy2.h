#include "WndDlg0.h"
#include "graffy_export.h"
#ifndef SIGPROC
#include "sigproc.h"
#endif
#include "wincore.h"
#include "winutils.h"


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
//	int GetOffsetPixel(double a, char xy, double offset);
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
	CFigure(CWndDlg * base, CGobj* pParent = NULL);   // standard constructor
	~CFigure();
};

