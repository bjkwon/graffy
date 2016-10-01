#include <process.h>

#include "PlotDlg.h"
#include "_graffy.h"

extern HINSTANCE hInst;
extern CPlotDlg* childfig;

BOOL CALLBACK DlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProc1 (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChildDlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam);

CGraffyDLL::CGraffyDLL()
{
//	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

CGraffyDLL::~CGraffyDLL()
{
//	GdiplusShutdown(gdiplusToken);
}

vector<HANDLE> CGraffyDLL::figures()
{
	vector<HANDLE> out;
	for (int k=0; k<nFigures; k++)
		out.push_back(fig[k]);
	return out;
}

HANDLE  CGraffyDLL::findFigure(const CStdString& caption, int *nFigs)
{
	CStdString tp;
	if (nFigs!=NULL) *nFigs = nFigures;
	for (int i=0; i<nFigures; i++)
	{
		fig[i]->GetWindowText(tp);
		if (tp==caption) return fig[i]->gcf;
	}
	return NULL;
}

HANDLE CGraffyDLL::openFigure(CRect *rt, const CSignals &data, int devID, double blocksize)
{
	CStdString s="";
	return openFigure(rt, s, data, devID, blocksize);
}

HANDLE CGraffyDLL::openFigure(CRect *rt, const char* caption, const CSignals &data, int nDev, double blocksize)
{
	CStdString s(caption);
	return openFigure(rt, s, data, nDev, blocksize);
}

HANDLE CGraffyDLL::openFigure(CRect *rt, const CStdString& caption, const CSignals &data, int devID, double blocksize)
{
	CPlotDlg **tempFigHolder;
	CStdString s;
	tempFigHolder = new CPlotDlg*[nFigures+1];
	for (int i=0; i<nFigures; i++)
		tempFigHolder[i] = fig[i];
	if (fig) delete[] fig;
	fig = tempFigHolder; //why this line should come before the next line---in a multithread situation, while the next line is executed for a new thread, still messages for the old member (thread) could come up... if fig is not updated yet, it would go astray in DlgProc (i.e., theApp.fig[i] would just crash...).... bjk 4/26/2016
	GraffyRoot.m_dlg = static_cast <CWndDlg*>(tempFigHolder[nFigures] = new CPlotDlg(data, hInst, &GraffyRoot));
	nFigures++;
	//second time around... this is where crash occurs....
	tempFigHolder[nFigures-1]->hDlg = CreateDialog(hInst, MAKEINTRESOURCE (IDD_PLOT), NULL, (DLGPROC)DlgProc);

	if (tempFigHolder[nFigures-1]->hDlg==NULL) 
	{	MessageBox(NULL,"Cannot Create graffy dialog box","",MB_OK);	return NULL;	}

	tempFigHolder[nFigures-1]->devID = devID;
	tempFigHolder[nFigures-1]->block = blocksize;
	for (int i=0; i<nFigures-1; i++)
	{
		RECT wndRt;
		GetWindowRect(tempFigHolder[i]->hDlg, &wndRt);
		if (*rt==wndRt) { rt->OffsetRect(20,32); i=0; }
	}
	tempFigHolder[nFigures-1]->MoveWindow(rt);
	if (caption=="")
	{
		s.Format("Figure %d", nFigures);
		tempFigHolder[nFigures-1]->SetWindowText(s);
	}
	else
		tempFigHolder[nFigures-1]->SetWindowText(caption);
	return tempFigHolder[nFigures-1]->gcf;
}


HANDLE CGraffyDLL::openChildFigure(CRect *rt, const CSignals &data, HWND hParent)
{ 
	int i;
	CPlotDlg **pdlg;
	CStdString s;
	pdlg = new CPlotDlg*[nFigures+1];
	for (i=0; i<nFigures; i++)
		pdlg[i] = fig[i];
	if (fig) delete[] fig;
	childfig = pdlg[nFigures] = new CPlotDlg(data, hInst);
	fig = pdlg;
	if (hParent==NULL)
		pdlg[nFigures]->hDlg = CreateDialog(hInst, MAKEINTRESOURCE (IDD_PLOT_CHILD), NULL, (DLGPROC)ChildDlgProc);
	else
		pdlg[nFigures]->hDlg = CreateDialog(hInst, MAKEINTRESOURCE (IDD_PLOT_CHILD), hParent, (DLGPROC)ChildDlgProc);
	if (pdlg[nFigures]->hDlg==NULL) 
	{	MessageBox(NULL,"Cannot Create graffy dialog box","",MB_OK);	return NULL;	}
	else
	{
		pdlg[nFigures]->MoveWindow(rt);
		nFigures++;
		return pdlg[nFigures-1]->gcf;
	}
}


int CGraffyDLL::closeFigure(int figId)
{
	// figId begins with 1.
	int i;
	if (figId > nFigures || figId < 0)
		return 0;

	if (figId==0) // delete all
	{
		for (i=0; i<nFigures; i++)
			fig[i]->DestroyWindow();
		delete[] fig;
	}
	else
	{
		CPlotDlg **pdlg;
		pdlg = new CPlotDlg*[nFigures-1];
		for (i=0; i<figId-2; i++)
			pdlg[i] = fig[i];
		for (i=figId; i<nFigures; i++)
			pdlg[i] = fig[i];
		fig[figId-1]->DestroyWindow();
		delete[] fig;
	}
	return 1;
}

