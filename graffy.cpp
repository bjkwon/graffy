// graffy.cpp : Defines the initialization routines for the DLL.
//


#include "PlotDlg.h"
#include "msgCrack.h"
#include <string.h>
#include <algorithm>

map<unsigned int, string> wmstr;
AUDFRET_EXP void makewmstr(map<unsigned int, string> &wmstr);

HINSTANCE hInst;
CPlotDlg* childfig;
GRAPHY_EXPORT HWND hPlotDlgCurrent;

HWND hWndApp(NULL);

class CGraffyDLL : public CWinApp 
{
public:
	CGobj GraffyRoot;
	CPlotDlg **fig;
	int closeFigure(int figId);
	vector<HANDLE> figures();
	vector<CAxis*> m_ax;
	HANDLE  findFigure(const char *caption, int *nFigs=NULL);
	HANDLE  openFigure(CRect *rt, const char* caption, const CSignals &data, HWND hWndAppl, int devID, double blocksize);
	HANDLE  openFigure(CRect *rt, const CSignals &data, HWND hWndAppl, int devID, double blocksize);
	int nFigures;
	CGraffyDLL();
	virtual ~CGraffyDLL();
};	

CGraffyDLL theApp; 

#define THE_CPLOTDLG  static_cast <CPlotDlg*>(theApp.fig[id])

int getID4hDlg(HWND hDlg)
{
	int i;
	if (theApp.fig[theApp.nFigures-1]->hDlg==NULL) return theApp.nFigures-1;
	for (i=0; i<theApp.nFigures; i++)
	{
		if (hDlg==theApp.fig[i]->hDlg) 	return i; 
	}
	//FILE *fp=fopen("getID4hDlg.log","at");
 //   SYSTEMTIME lt;
 //   GetLocalTime(&lt);	
	//char buffer[256];
	//sprintf(buffer, "[%02d/%02d/%4d, %02d:%02d:%02d] getID4hDlg error\n", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
	//fprintf(fp, buffer);
	//fclose(fp);
	return 0;
}

BOOL CALLBACK DlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{

	int id = getID4hDlg(hDlg);
	//FILE *fp=fopen("track.txt","at");
	//if (umsg==WM_ACTIVATE || umsg==WM_NCACTIVATE || umsg==WM_ACTIVATEAPP || umsg==WM_GETICON || umsg==WM_KEYDOWN || umsg==WM_COMMAND )
	//	fprintf(fp, "window %d, %x: msg: 0x%04x %s, wParam=%d\n", id, hDlg, umsg, wmstr[umsg].c_str(), wParam);
	//else if (umsg!=WM_NCHITTEST && umsg!=WM_SETCURSOR && umsg!=WM_MOUSEMOVE && umsg!=WM_NCMOUSEMOVE && umsg!=WM_WINDOWPOSCHANGING && umsg!= WM_WINDOWPOSCHANGED)
	//	fprintf(fp, "window %d, %x: msg: 0x%04x %s\n", id, hDlg, umsg, wmstr[umsg].c_str());
	//fclose(fp);
	switch (umsg)
	{
	chHANDLE_DLGMSG (hDlg, WM_INITDIALOG, THE_CPLOTDLG->OnInitDialog);
	chHANDLE_DLGMSG (hDlg, WM_PAINT, THE_CPLOTDLG->OnPaint);
	chHANDLE_DLGMSG (hDlg, WM_SIZE, THE_CPLOTDLG->OnSize);
	chHANDLE_DLGMSG (hDlg, WM_CLOSE, THE_CPLOTDLG->OnClose);
	chHANDLE_DLGMSG (hDlg, WM_DESTROY, THE_CPLOTDLG->OnDestroy);
	chHANDLE_DLGMSG (hDlg, WM_COMMAND, THE_CPLOTDLG->OnCommand);
	chHANDLE_DLGMSG (hDlg, WM_RBUTTONUP, THE_CPLOTDLG->OnRButtonUp);
	chHANDLE_DLGMSG (hDlg, WM_LBUTTONUP, THE_CPLOTDLG->OnLButtonUp);
	chHANDLE_DLGMSG (hDlg, WM_LBUTTONDOWN, THE_CPLOTDLG->OnLButtonDown);
	chHANDLE_DLGMSG (hDlg, WM_LBUTTONDBLCLK, THE_CPLOTDLG->OnLButtonDblClk);
	chHANDLE_DLGMSG (hDlg, WM_MOUSEMOVE, THE_CPLOTDLG->OnMouseMove);
	chHANDLE_DLGMSG (hDlg, WM_KEYDOWN, THE_CPLOTDLG->OnKeyDown);
	chHANDLE_DLGMSG (hDlg, WM_TIMER, THE_CPLOTDLG->OnTimer);
//	chHANDLE_DLGMSG (hDlg, WM_ACTIVATE, THE_CPLOTDLG->OnActivate);
	chHANDLE_DLGMSG (hDlg, WM__SOUND_EVENT, THE_CPLOTDLG->OnSoundEvent);

	//case WM_ACTIVATE:  // x0006
	//	PostMessage(hDlg, WM_FIGURE_CLICKED, (WPARAM)THE_CPLOTDLG->gcf, 0);
	//	return FALSE;

	case WM_MOUSEACTIVATE: //0x0021
	case WM_NCLBUTTONDOWN: //0x00A1
		THE_CPLOTDLG->SetGCF();
		return FALSE;
		break;

//	case WM_NCACTIVATE: // x0086
//		PostMessage(hDlg, WM_FIGURE_CLICKED, (WPARAM)THE_CPLOTDLG->gcf, 0);
	//	res = THE_CPLOTDLG->OnNCActivate((BOOL)(wParam));
	//	SetWindowLong(hDlg, MSGRESULT, (LPARAM)(LRESULT)(res)); // this is where this message is properly returned
		break;

	//case WM_MOUSELEAVE:
	//case WM_NCMOUSELEAVE:
	//	THE_CPLOTDLG->HandleLostFocus(umsg);
	//	return TRUE;

	//case WM_NCHITTEST:
	//	res = DefWindowProc  (hDlg, umsg, wParam, lParam);
	//	SetWindowLong(hDlg, MSGRESULT, (LPARAM)(LRESULT)(res)); // this is where this message is properly returned
	//	if (res!=HTCLIENT)
	//		THE_CPLOTDLG->HandleLostFocus(umsg, res);
	//	break;

	default:
		return FALSE;
	}
	return TRUE;
}


CGraffyDLL::CGraffyDLL()
{
}

CGraffyDLL::~CGraffyDLL()
{
	for (vector<CAxis*>::iterator it=m_ax.begin(); it!=m_ax.end(); it++)
		delete &it;
	m_ax.clear();
}

vector<HANDLE> CGraffyDLL::figures()
{
	vector<HANDLE> out;
	for (int k=0; k<nFigures; k++)
		out.push_back(fig[k]);
	return out;
}

HANDLE  CGraffyDLL::findFigure(const char *caption, int *nFigs)
{
	char tp[256];
	if (nFigs!=NULL) *nFigs = nFigures;
	for (int i=0; i<nFigures; i++)
	{
		fig[i]->GetWindowText(tp, 256);
		if (!strcmp(tp,caption)) return fig[i]->gcf;
	}
	return NULL;
}

HANDLE CGraffyDLL::openFigure(CRect *rt, const CSignals &data, HWND hWndAppl, int devID, double blocksize)
{
	return openFigure(rt, "", data, hWndAppl, devID, blocksize);
}

HANDLE CGraffyDLL::openFigure(CRect *rt, const char* caption, const CSignals &data, HWND hWndAppl, int devID, double blocksize)
{
	CPlotDlg **tempFigHolder;
	CString s;
	tempFigHolder = new CPlotDlg*[nFigures+1];
	for (int i=0; i<nFigures; i++)
		tempFigHolder[i] = fig[i];
	if (fig) delete[] fig;
	GraffyRoot.hPar=&GraffyRoot; // The parent of root is self.
	fig = tempFigHolder; //why this line should come before the next line---in a multithread situation, while the next line is executed for a new thread, still messages for the old member (thread) could come up... if fig is not updated yet, it would go astray in DlgProc (i.e., theApp.fig[i] would just crash...).... bjk 4/26/2016
	GraffyRoot.m_dlg = static_cast <CWndDlg*>(tempFigHolder[nFigures] = new CPlotDlg(data, hInst, &GraffyRoot));
	// The only purpose that GraffyRoot.m_dlg serves is to hold hWndAppl here...
//	GraffyRoot.m_dlg->hParent = GraffyRoot.m_dlg;
	GraffyRoot.m_dlg->hDlg = hWndAppl;
	nFigures++;
	//second time around... this is where crash occurs....
	tempFigHolder[nFigures-1]->hDlg = CreateDialog(hInst, MAKEINTRESOURCE (IDD_PLOT), hWndAppl, (DLGPROC)DlgProc);

	if (tempFigHolder[nFigures-1]->hDlg==NULL) 
	{	MessageBox(NULL,"Cannot Create graffy dialog box","",MB_OK);	return NULL;	}

	tempFigHolder[nFigures-1]->devID = devID;
	tempFigHolder[nFigures-1]->block = blocksize; // this is ignored. 7/15/2016 bjk
	for (int i=0; i<nFigures-1; i++)
	{
		RECT wndRt;
		GetWindowRect(tempFigHolder[i]->hDlg, &wndRt);
		if (*rt==wndRt) { rt->OffsetRect(20,32); i=0; }
	}
	tempFigHolder[nFigures-1]->MoveWindow(rt);
	if (strlen(caption)==0) // if (caption=="")    NOT THE SAME  in WIN64
	{
		s.Format("Figure %d", nFigures);
		tempFigHolder[nFigures-1]->SetWindowText(s);
	}
	else
		tempFigHolder[nFigures-1]->SetWindowText(caption);
	return tempFigHolder[nFigures-1]->gcf;
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



BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	hInst = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		makewmstr(wmstr);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

GRAPHY_EXPORT HACCEL GetAccel(HANDLE hFig)
{
	for (int i=0; i<theApp.nFigures; i++)
		if (hFig==theApp.fig[i]->gcf)  return theApp.fig[i]->GetAccel();
	return NULL;
}

GRAPHY_EXPORT HWND GetHWND_PlotDlg(HANDLE hFig)
{
	for (int i=0; i<theApp.nFigures; i++)
		if (hFig==theApp.fig[i]->gcf)  return theApp.fig[i]->hDlg;
	return NULL;
}

GRAPHY_EXPORT HWND GetHWND_PlotDlg2(HANDLE hFig)
{
	for (int i=0; i<theApp.nFigures; i++)
		if (hFig==theApp.fig[i])  return theApp.fig[i]->hDlg;
	return NULL;
}


GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, const CSignals &data, HWND hWndAppl, int devID, double block)
{
	return theApp.openFigure(rt, "", data, hWndAppl, devID, block);
}

GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, const char *caption, const CSignals &data, HWND hWndAppl, int devID, double block)
{
	return theApp.openFigure(rt, caption, data, hWndAppl, devID, block);
}

GRAPHY_EXPORT int CloseFigure(int id)
{
	return theApp.closeFigure(id);
}

GRAPHY_EXPORT HANDLE FindFigure(const char* caption, int *nFigs)
{
	return theApp.findFigure(caption, nFigs);
}

GRAPHY_EXPORT vector<HANDLE> graffy_Figures()
{
	return theApp.figures();
}

GRAPHY_EXPORT HANDLE GetGraffyHandle(int figID)
{
	string str;
	for (int k=0; k<theApp.nFigures; k++) 
	{
		CFigure* cfig = theApp.fig[k]->gcf;
		if ((int)cfig->hPar==figID) return cfig->hPar;
		for (vector<CAxis*>::iterator paxit=cfig->ax.begin(); paxit!=cfig->ax.end(); paxit++) 
		{
			if ((int)*paxit==figID) return (HANDLE)*paxit; 
			for (size_t q=0; q < (*paxit)->m_ln.size(); q++)
			{
				CLine *ln = (*paxit)->m_ln[q];
				if ((int)ln==figID) return (HANDLE)ln; 
			}
		}
		for (vector<CText*>::iterator ptxit=cfig->text.begin(); ptxit!=cfig->text.end(); ptxit++) 
			if ((int)*ptxit==figID) return (HANDLE)*ptxit; 
	}
	return NULL;
}

GRAPHY_EXPORT HANDLE GCF(CSignals *figID)
{
	int ext;
	char buf[64];
	string str;
	double val;
	int res(-1);
	if (figID->GetType()==CSIG_SCALAR) val = figID->value();
	for (int k=0; k<theApp.nFigures; k++) 
	{
		theApp.fig[k]->GetWindowText(buf, sizeof(buf));
		str = buf;
		if (figID->GetType()==CSIG_STRING)
		{
			if (str==figID->string()) return theApp.fig[k]->gcf;
		}
		else if (figID->GetType()==CSIG_SCALAR)
		{
			if (str.find("Figure ")==0) 
				str.erase(0, string("Figure ").length());
			if (sscanf(str.c_str(), "%d", &ext)>0)	
			{
				int dummult = 100000;
				if ((int)(val*dummult)==ext*dummult)	
					res=k;
			}
		}
	}
	if (res<0)
		return NULL;
	else
		return theApp.fig[res]->gcf;

}

GRAPHY_EXPORT int GetFigID(HANDLE h, CSignals& out)
{
	char buf[64];
	int ext, res(0);
	string str;
	for (int k=0; k<theApp.nFigures; k++) 
	{
		if (theApp.fig[k]->gcf==h)
		{
			res=1;
			theApp.fig[k]->GetWindowText(buf, sizeof(buf));
			str = buf;
			if (str.find("Figure ")==0) 
			{
				str.erase(0, string("Figure ").length());
				if (sscanf(str.c_str(), "%d", &ext)>0)	
					out.SetValue((double)ext);
			}
			else
			{
				out.SetString(str.c_str());
			}
			k=theApp.nFigures+1;
		}
	}
	return res;
}

GRAPHY_EXPORT HANDLE GCA(HANDLE _fig)
{
	CFigure *fig = static_cast<CFigure *>(_fig);
	for (int k=0; k<theApp.nFigures; k++) 
	{
		if (theApp.fig[k]->gcf==_fig)
			return theApp.fig[k]->gca;
	}
	return NULL;
}

GRAPHY_EXPORT HANDLE  AddAxis(HANDLE _fig, double x0, double y0, double wid, double hei)
{
	CFigure *fig = static_cast<CFigure *>(_fig);
	CPosition pos(x0, y0, wid, hei);
	CAxis * ax = fig->axes(pos);
	theApp.fig[theApp.nFigures-1]->gca = ax; 
	return ax;
}

GRAPHY_EXPORT HANDLE  AddText (HANDLE _fig, const char* text, double x0, double y0, double wid, double hei)
{
	CFigure *fig = static_cast<CFigure *>(_fig);
	CPosition pos(x0, y0, wid, hei);
	return fig->AddText(text, pos);
}

GRAPHY_EXPORT void SetRange(HANDLE _ax, const char xy, double x1, double x2)
{
	CAxis *ax = static_cast<CAxis *>(_ax);
	if (xy=='x')
		ax->xlim[0]=x1, ax->xlim[0]=x2;
	else
		ax->ylim[0]=x1, ax->ylim[0]=x2;
}

GRAPHY_EXPORT HANDLE PlotCSignals(HANDLE _ax, CSignal &data, COLORREF col, char cymbol, LineStyle ls)
{
	if (data.GetType()!=CSIG_AUDIO && data.GetType()!=CSIG_VECTOR) return NULL;
	CAxis *ax = static_cast<CAxis *>(_ax);
	if (data.GetType()==CSIG_VECTOR) strcpy(ax->xtick.format,"%.0f"); // for non-audio, plot(x) call, don't bother to show any decimal point on x-axis.
	return ax->plot(NULL, data, col, cymbol, ls);
}

GRAPHY_EXPORT HANDLE PlotCSignals(HANDLE ax, CSignals &data, COLORREF col, char cymbol, LineStyle ls)
{ return PlotCSignals(ax, (CSignal)data, col, cymbol, ls); }

GRAPHY_EXPORT HANDLE PlotCSignals(HANDLE _ax, CSignals &dataX, CSignals &dataY, COLORREF col, char cymbol, LineStyle ls)
{
	if (dataX.GetType()!=CSIG_AUDIO && dataX.GetType()!=CSIG_VECTOR) return NULL;
	if (dataY.GetType()!=CSIG_AUDIO && dataY.GetType()!=CSIG_VECTOR) return NULL;
	CAxis *ax = static_cast<CAxis *>(_ax);
	return ax->plot(dataX.buf, dataY, col, cymbol, ls);
}

GRAPHY_EXPORT HANDLE  PlotDouble(HANDLE _ax, int len, double *x, double *y, COLORREF col, char cymbol, LineStyle ls)
{	
	CAxis *ax = static_cast<CAxis *>(_ax);
	if (y!=NULL)		
		return ax->plot(x, CSignals(y, len), col, cymbol, ls);
	else
		return ax->plot(NULL, CSignals(x, len), col, cymbol, ls);
}

void _deleteObj (CFigure *hFig)
{
	int i, j, ind2Del;
	bool loop(true);
	for (i=0; i<theApp.nFigures && loop; i++)
		if (hFig==theApp.fig[i]->gcf) 
			{ind2Del=i; loop=false;}
	if (loop) return ; // If hFig is a ghost, do nothing.
	CPlotDlg **pFigs;
	pFigs = new CPlotDlg*[theApp.nFigures-1];
	//storing objects to keep
	for (i=0, j=0; i<theApp.nFigures; i++)
		if (i!=ind2Del) 
			pFigs[j++] = theApp.fig[i];
		else
		{
			theApp.fig[i]->DestroyWindow();
			delete theApp.fig[i];
		}
	delete hFig;
		
	delete[] theApp.fig;
	theApp.fig = pFigs;
	theApp.nFigures--;
}

GRAPHY_EXPORT void deleteObj (HANDLE h)
{
	CFigure *hpar;
	CAxis *hpax;
	CGobj* aa = static_cast<CGobj*>(h);
	switch(aa->type)
	{
	case 'f':
		_deleteObj((CFigure *)h);
		break;
	case 't':
	case 'a':
		hpar = (CFigure *)(static_cast<CGobj*>(h)->hPar);
		for (vector<CText*>::iterator it=hpar->text.begin(); it!=hpar->text.end(); it++) 
		{ if (h==*it) hpar->text.erase(it); return ;}
		for (vector<CAxis*>::iterator it=hpar->ax.begin(); it!=hpar->ax.end(); it++) 
			if (h==*it) 
			{
				delete *it;
				hpar->ax.erase(it); 
				return ;
			}
		break;
	case 'l':
		hpax = (CAxis *)(static_cast<CGobj*>(h)->hPar);
		for (vector<CLine*>::iterator it2=hpax->m_ln.begin(); it2!=hpax->m_ln.end(); it2++) 
 			if (h==*it2) 
			{
				delete *it2;
				hpax->m_ln.erase(it2); 
				return ;
			}
		break;
	}
}

GRAPHY_EXPORT void SetHWND_GRAFFY(HWND h)
{
  // Register it, so that we can communicate between graffy and the application about the status of gcf.
  // To notify the application of gcf status change, call PostMessage/SendMessage
	hWndApp = h;
}

GRAPHY_EXPORT HWND GetHWND_GRAFFY ()
{
	return hWndApp;
}