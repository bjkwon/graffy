#pragma once

#ifndef SIGPROC
#include "sigproc.h"
#endif


//#include <Gdiplusinit.h>
#include "graffy_export.h"

#include "_graffy2.h"

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

GRAPHY_EXPORT HANDLE  FindFigure(const char* caption, int *nFigs=NULL);
GRAPHY_EXPORT vector<HANDLE> graffy_Figures();
GRAPHY_EXPORT HANDLE GCF(CSignals *figID);
GRAPHY_EXPORT HANDLE GetGraffyHandle(int figID);
GRAPHY_EXPORT HANDLE GCA(HANDLE _fig);
GRAPHY_EXPORT int GetFigID(HANDLE h, CSignals& out);

#ifdef _WIN32XX_WINCORE_H_
#define NO_USING_NAMESPACE
GRAPHY_EXPORT HANDLE  OpenFigure(CRect *rt, const CSignals &data, HWND hWndAppl, int devID, double block);
GRAPHY_EXPORT HANDLE  OpenFigure(CRect *rt, const char *caption, const CSignals &data, HWND hWndAppl, int devID, double block);
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
