// graffy.cpp : Defines the initialization routines for the DLL.
//


#include "PlotDlg.h"
#include "msgCrack.h"
#include <string.h>
#include <algorithm>

map<unsigned int, string> wmstr;
AUDFRET_EXP void makewmstr(map<unsigned int, string> &wmstr);
AUDFRET_EXP int spyWM(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, map<unsigned int, string> wmstr, vector<UINT> msg2excl, char* const tagstr);
AUDFRET_EXP int spyWM2(HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam, char* const fname, map<unsigned int, string> wmstr, vector<UINT> msg2excl, char* const tagstr);
AUDFRET_EXP int SpyGetMessage(MSG msg, char* const fname, map<unsigned int, string> wmstr, vector<UINT> msg2excl, char* const tagstr);
AUDFRET_EXP int SpyGetMessage2(MSG msg, char* const fname, map<unsigned int, string> wmstr, vector<UINT> msg2excl, char* const tagstr);
vector<UINT> exc;

HINSTANCE hInst;
CPlotDlg* childfig;
GRAPHY_EXPORT HWND hPlotDlgCurrent;

#define WM_PLOT_DONE	WM_APP+328


HANDLE mutexPlot;
HANDLE hEvent;
HWND hWndApp(NULL);

class CGraffyDLL : public CWinApp 
{
public:
	CGobj GraffyRoot;
	vector<CPlotDlg *> fig;
	vector<HWND> hDlg_fig;
	vector<HANDLE> figures();
	vector<CAxis*> m_ax;
	HANDLE  findFigure(const char *caption, int *nFigs=NULL);
	HANDLE  openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize);
	HANDLE  openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize);
	int closeFigure(HANDLE h);
	CGraffyDLL();
	virtual ~CGraffyDLL();
};	

CGraffyDLL theApp; 

#define THE_CPLOTDLG  static_cast <CPlotDlg*>(theApp.fig[id])

int getID4hDlg(HWND hDlg)
{
/*	FILE *fp=fopen("getID4hDlg.log","at");
    SYSTEMTIME lt;
    GetLocalTime(&lt);	
	char buffer[256];
	sprintf(buffer, "[%02d/%02d/%4d, %02d:%02d:%02d] getID4hDlg error\n", lt.wMonth, lt.wDay, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
	fprintf(fp, buffer);
	fclose(fp);*/
	size_t k(0);
	for (vector<HWND>::iterator it=theApp.hDlg_fig.begin(); it!=theApp.hDlg_fig.end(); it++, k++) 
	{ if (hDlg==*it)   return (int)k; }
	return -1;
}

INT_PTR CALLBACK DlgProc (HWND hDlg, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	if (exc.size()==0)
	{
		exc.push_back(WM_NCHITTEST);
		exc.push_back(WM_SETCURSOR);
		exc.push_back(WM_MOUSEMOVE);
		exc.push_back(WM_NCMOUSEMOVE);
		exc.push_back(WM_WINDOWPOSCHANGING);
		exc.push_back(WM_WINDOWPOSCHANGED);
		exc.push_back(WM_CTLCOLORDLG);
		exc.push_back(WM_NCPAINT);
		exc.push_back(WM_GETMINMAXINFO);
		exc.push_back(WM_MOVE);
		exc.push_back(WM_MOVING);
		exc.push_back(WM_NCMOUSEMOVE);
		exc.push_back(WM_ERASEBKGND);
		//exc.push_back(WM_ACTIVATE);
		//exc.push_back(WM_ACTIVATEAPP);
		//exc.push_back(WM_SETFOCUS);
		//exc.push_back(WM_KILLFOCUS);
		//exc.push_back(WM_NCACTIVATE);
		//exc.push_back(WM_SHOWWINDOW);
	}
	int id = getID4hDlg(hDlg);
	if (id<0) // This means theApp.hDlg_fig has not gotten hDlg for the created window, i.e., processing early messages prior to WM_INITDIALOG
	{ 
		//then add hDlg to the vector
		id = (int)theApp.hDlg_fig.size();
		theApp.hDlg_fig.push_back(hDlg);
	}
//	char buf[32];
//	sprintf(buf, "%d: ", id);
//	spyWM(hDlg, umsg, wParam, lParam, "track.txt", wmstr, exc, buf);
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

//	case WM_MOUSEACTIVATE: //0x0021
//	case WM_NCLBUTTONDOWN: //0x00A1
//		THE_CPLOTDLG->SetGCF();
//		return FALSE;
//		break;

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
//	if (!mutexPlot) mutexPlot = CreateMutex(0, 0, 0);
	hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("AUXCONScriptEvent")); 

}

CGraffyDLL::~CGraffyDLL()
{
	for (vector<CPlotDlg*>::iterator it=fig.begin(); it!=fig.end(); it++)
		delete *it;
	fig.clear();
	CloseHandle(hEvent);
}

vector<HANDLE> CGraffyDLL::figures()
{
	vector<HANDLE> out;
	for (vector<CPlotDlg*>::iterator it=fig.begin(); it!=fig.end(); it++) 
		out.push_back(*it);
	return out;
}

HANDLE  CGraffyDLL::findFigure(const char *caption, int *nFigs)
{
	char tp[256];
	if (nFigs!=NULL) *nFigs = (int)fig.size();
	for (vector<CPlotDlg*>::iterator it=fig.begin(); it!=fig.end(); it++) 
	{
		(*it)->GetWindowText(tp, 256);
		if (!strcmp(tp,caption)) return &(*it)->gcf;
	}
	return NULL;
}

HANDLE CGraffyDLL::openFigure(CRect *rt, HWND hWndAppl, int devID, double blocksize)
{
	return openFigure(rt, "", hWndAppl, devID, blocksize);
}

HANDLE CGraffyDLL::openFigure(CRect *rt, const char* caption, HWND hWndAppl, int devID, double blocksize)
{
	CString s;
	CPlotDlg *newFig;
	fig.push_back(newFig = new CPlotDlg(hInst, &GraffyRoot)); // this needs before CreateDialogParam

	if ((newFig->hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE (IDD_PLOT), hWndAppl, (DLGPROC)DlgProc, NULL))==NULL)
	{	MessageBox(NULL,"Cannot Create graffy dialog box","",MB_OK);	fig.pop_back(); delete newFig; return NULL;	}

	newFig->devID = devID;
	newFig->block = blocksize; // this is ignored. 7/15/2016 bjk
	for (size_t k=0; k<hDlg_fig.size(); k++)
	{
		RECT wndRt;
		GetWindowRect(hDlg_fig[k], &wndRt);
		if (*rt==wndRt) { rt->OffsetRect(20,32); k=0; }
	}
	newFig->MoveWindow(rt);
	if (strlen(caption)==0) // if (caption=="")    NOT THE SAME  in WIN64
	{
		s.Format("Figure %d", hDlg_fig.size());
		newFig->SetWindowText(s);
	}
	else
		newFig->SetWindowText(caption);
	return &newFig->gcf;
}


int CGraffyDLL::closeFigure(HANDLE h)
{
	//Returns the number of fig dlg windows remaining.
	if (h==NULL) // delete all
	{
		for (vector<CPlotDlg*>::iterator it=fig.begin(); it!=fig.end(); it++)
			(*it)->OnClose(); // inside OnClose(), delete *it is called.
		fig.clear();
		hDlg_fig.clear();
	}
	else
	{
		CFigure *cfig = (CFigure*)h;
		for (vector<HWND>::iterator it=hDlg_fig.begin(); it!=hDlg_fig.end(); it++) 
		{ if (cfig->m_dlg->hDlg==*it)  {hDlg_fig.erase(it); break;} }
		for (vector<CPlotDlg*>::iterator it=fig.begin(); it!=fig.end(); it++) 
		{ if (cfig->m_dlg==*it)  {/*(*it)->OnClose(); */fig.erase(it); break;} }
		delete cfig->m_dlg;
	}
	return (int)fig.size();
}



BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	hInst = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
//		makewmstr(wmstr);
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
	for (vector<CPlotDlg*>::iterator it=theApp.fig.begin(); it!=theApp.fig.end(); it++) 
		if (hFig==&(*it)->gcf)  return (*it)->GetAccel();
	return NULL;
}

GRAPHY_EXPORT HWND GetHWND_PlotDlg(HANDLE hFig)
{
	for (vector<CPlotDlg*>::iterator it=theApp.fig.begin(); it!=theApp.fig.end(); it++) 
		if (hFig==&(*it)->gcf)  return (*it)->hDlg;
	return NULL;
}

GRAPHY_EXPORT HWND GetHWND_PlotDlg2(HANDLE hFig)
{
	for (size_t i=0; i<theApp.fig.size(); i++)
		if (hFig==theApp.fig[i])  return theApp.fig[i]->hDlg;
	return NULL;
}

FILE* fpp;

void thread4Plot (PVOID var)
{
	MSG         msg ;
	CSignals gcf;
	GRAFWNDDLGSTRUCT *in = (GRAFWNDDLGSTRUCT *)var;
	if ((in->fig = OpenFigure(&in->rt, in->caption.c_str(), in->hWndAppl, in->devID, in->block))==NULL) 
	{
		PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
		return;
	}
	in->cfig = static_cast<CFigure *>(in->fig);
	in->hAccel = GetAccel(in->fig);
	in->threadPlot = GetCurrentThreadId(); 
	PostThreadMessage(in->threadCaller, WM_PLOT_DONE, 0, 0);
	while (GetMessage (&msg, NULL, 0, 0))
	{
		if (msg.message==WM_DESTROY || !in->cfig->m_dlg)			break;
 		if (!TranslateAccelerator(in->cfig->m_dlg->hDlg, in->hAccel, &msg))
		{
			if (msg.message==WM_KEYDOWN)
			if (msg.message==WM_KEYDOWN && msg.wParam==17 && GetParent(msg.hwnd)==in->cfig->m_dlg->hDlg) // Left control key for window size adjustment
				msg.hwnd = in->cfig->m_dlg->hDlg;
			if (!IsDialogMessage(msg.hwnd, &msg))
			{
//				SpyGetMessage(msg, "track.txt", wmstr, dum, "Dispatching ");
				TranslateMessage (&msg) ;
				DispatchMessage (&msg) ;
			}
//			else
//				SpyGetMessage2(msg, "track.txt", wmstr, dum, "Dialog ");
		}
	}
	CloseFigure(in->fig);
	delete in;
}

GRAPHY_EXPORT HANDLE OpenGraffy(CRect rt, const char *caption, DWORD threadID, HWND hApplDlg, GRAFWNDDLGSTRUCT &in)
{ // in in in out
	GRAFWNDDLGSTRUCT* pin = new GRAFWNDDLGSTRUCT;
	pin->fig=NULL;
	pin->threadCaller = threadID;
	pin->caption = caption;
	pin->rt = rt;
	pin->hWndAppl = hApplDlg;
	pin->block = 0.;
	pin->devID = 0;
	_beginthread (thread4Plot, 0, (void*)pin);
//	DWORD dw = WaitForSingleObject(hEvent, INFINITE);

	MSG         msg ;
	while (GetMessage (&msg, NULL, 0, 0))
	{
		if (msg.message==WM_PLOT_DONE)
		{
			in = *pin;
			break;
		}
	}
	return pin->fig;
}


GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, HWND hWndAppl, int devID, double block)
{
	return theApp.openFigure(rt, "", hWndAppl, devID, block);
}

GRAPHY_EXPORT HANDLE OpenFigure(CRect *rt, const char *caption, HWND hWndAppl, int devID, double block)
{
	return theApp.openFigure(rt, caption, hWndAppl, devID, block);
}

GRAPHY_EXPORT int CloseFigure(HANDLE h)
{
//	delete h;
	return theApp.closeFigure(h);
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
	for (size_t k=0; k<theApp.fig.size(); k++) 
	{
		CFigure* cfig = &theApp.fig[k]->gcf;
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
{//set gcf with figID
	int ext;
	char buf[64];
	string str;
	double val;
	int res(-1);
	if (figID->GetType()==CSIG_SCALAR) val = figID->value();
	for (size_t k=0; k<theApp.fig.size(); k++) 
	{
		theApp.fig[k]->GetWindowText(buf, sizeof(buf));
		str = buf;
		if (figID->GetType()==CSIG_STRING)
		{
			if (str==figID->string()) return &theApp.fig[k]->gcf;
		}
		else if (figID->GetType()==CSIG_SCALAR)
		{
			if (str.find("Figure ")==0) 
				str.erase(0, string("Figure ").length());
			if (sscanf(str.c_str(), "%d", &ext)>0)	
			{
				int dummult = 100000;
				if ((int)(val*dummult)==ext*dummult)	
					res=(int)k;
			}
		}
	}
	if (res<0)
		return NULL;
	else
		return &theApp.fig[res]->gcf;

}

GRAPHY_EXPORT HANDLE GetGraffyHandle(HWND h)
{
	for (size_t k=0; k<theApp.fig.size(); k++) 
		if (theApp.fig[k]->hDlg == h)
			return &theApp.fig[k]->gcf;
	return NULL;
}

GRAPHY_EXPORT int GetFigID(HWND h, CSignals& out)
{
	char buf[64];
	int ext, res(0);
	string str;
	for (size_t k=0; k<theApp.fig.size(); k++) 
	{
		if (theApp.fig[k]->hDlg == h)
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
			break;
		}
	}
	return res;
}

GRAPHY_EXPORT int GetFigID(HANDLE h, CSignals& out)
{
	char buf[64];
	int ext, res(0);
	string str;
	for (size_t k=0; k<theApp.fig.size(); k++) 
	{
		if (&theApp.fig[k]->gcf==h)
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
			break;
		}
	}
	return res;
}

GRAPHY_EXPORT HANDLE GCA(HANDLE _fig)
{
	CFigure *fig = static_cast<CFigure *>(_fig);
	for (size_t k=0; k<theApp.fig.size(); k++) 
	{
		if (&theApp.fig[k]->gcf==_fig)
			return theApp.fig[k]->gca;
	}
	return NULL;
}

GRAPHY_EXPORT HANDLE  AddAxis(HANDLE _fig, double x0, double y0, double wid, double hei)
{
	CFigure *fig = static_cast<CFigure *>(_fig);
	CPosition pos(x0, y0, wid, hei);
	CAxis * ax = fig->axes(pos);
	theApp.fig.back()->gca = ax; 
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
		ax->xlim[0]=x1, ax->xlim[1]=x2;
	else
		ax->ylim[0]=x1, ax->ylim[1]=x2;
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
	for (vector<CPlotDlg*>::iterator it=theApp.fig.begin(); it!=theApp.fig.end(); it++) 
		if (hFig==&(*it)->gcf) 
		{delete *it; /*theApp.fig.erase(it); */break;}
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