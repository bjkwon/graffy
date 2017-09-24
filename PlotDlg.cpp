// PlotDlg.cpp : implementation file
//

#include "PlotDlg.h"
#include "FileDlg.h"
#include <limits>

int sprintfFloat(double f, int max_fp, char *strOut, size_t sizeStrOut);
void splitevenindices(int ** out, int nTotals, int nGroups);

#define IDM_SPECTRUM_INTERNAL	2222
#define IDC_LEVEL			7000
#define IDC_STATIC_LEVEL	7001
#define MOVE_SPECAX			8560
#define ID_STATUSBAR  1030
#define NO_SELECTION  RANGE_PX(-1,-1)

#define PBPROG_ADVANCE_PIXELS	2

#define MAKE_dB(x) (20*log10((x)) + 3.0103)
#define ROUND(x) (int)((x)+.5)
#define FIX(x) (int)((x))

extern HWND hPlotDlgCurrent;



FILE *fp;

#define RMSDB(BUF,FORMAT1,FORMAT2,X) { double rms;	if ((rms=X)==-1.*std::numeric_limits<double>::infinity()) strcpy(BUF, FORMAT1); else sprintf(BUF, FORMAT2, rms); }

unsigned _int8 GetMousePosAxis(CPoint pt, CAxis* pax)
{
	if (pax->axRect.PtInRect(pt)) return AXCORE;
	unsigned _int8 res(0);
	if (pax->xtick.rt.PtInRect(pt)) res = GRAF_REG1 << 2;
	if (pax->ytick.rt.PtInRect(pt)) res += GRAF_REG1;
	return res;
}

CPlotDlg::CPlotDlg()
:axis_expanding(false), levelView(false), playing(false), paused(false), ClickOn(0), MoveOn(0), devID(0), playLoc(-1), zoom(0), spgram(false), selColor(RGB(150, 180, 155)), hStatusbar(NULL)
{ // do not use this.
}

CPlotDlg::CPlotDlg(HINSTANCE hInstance, CGobj *hPar)
:axis_expanding(false), levelView(false), playing(false), paused(false), ClickOn(0), MoveOn(0), devID(0), playLoc(-1), zoom(0), spgram(false), selColor(RGB(150, 180, 155)), hStatusbar(NULL)
{
	opacity = 0xff;

	gcf.m_dlg = this;
	gcf.hPar = hPar;
	gcf.hPar->child.push_back(&gcf);

	menu.LoadMenu(IDR_POPMENU);
	subMenu = menu.GetSubMenu(0);
	gcmp=CPoint(-1,-1);
	z0pt=CPoint(-1,-1);
	hInst = hInstance;
	hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));
	RMSselected[0]=0;
	for (int k(0); k<10; k++) hTTtimeax[k]=NULL;
	ttstat.push_back("begin(screen)");
	ttstat.push_back("end(screen)");
	ttstat.push_back("Cursor position");
	ttstat.push_back("");
	ttstat.push_back("begin(selection)");
	ttstat.push_back("end(selection)");
	ttstat.push_back("Frequency");
	sbinfo.initialshow = false;
	sbinfo.xSelBegin = sbinfo.xSelEnd = 0.;
}

CPlotDlg::~CPlotDlg()
{
}

void CPlotDlg::OnDestroy()
{
	//Don't "delete this" here
	DestroyWindow();
}

void CPlotDlg::OnClose()
{
	// Instead of calling DestroyWindow(), post the message to message loop (either in Auxtra.cpp or plotThread.cpp) and properly delete the figure (and reduce theApp.nFigures by 1)
//	PostMessage(WM_DESTROY);
	PostMessage(WM_QUIT);
	for (size_t k=0; k<gcf.ax.size(); k++)
		deleteObj(gcf.ax[k]); 
	for (size_t k=0; k<gcf.text.size(); k++)
		deleteObj(gcf.text[k]);
}

HACCEL CPlotDlg::GetAccel()
{
	return hAccel;
}

HWND CPlotDlg::CreateTT(HWND hPar, TOOLINFO *tinfo)
{
    HWND TT = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,        
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hPar, NULL, hInst, NULL);

    ::SetWindowPos(hTTscript, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    tinfo->cbSize = sizeof(TOOLINFO);
    tinfo->uFlags = TTF_SUBCLASS;
    tinfo->hwnd = hPar;
    tinfo->hinst = hInst;
    tinfo->lpszText = "";
	tinfo->rect.left=tinfo->rect.right=0;
    
    /* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
    ::SendMessage(TT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) tinfo);	
	::SendMessage(TT, TTM_SETMAXTIPWIDTH, 0, 400);

	return TT;
}

BOOL CPlotDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA); // how can I transport transparency from application? Let's think about it tomorrow 1/6/2017 12:19am
	hTTscript = CreateTT(hwnd, &ti_script);
    GetClientRect (hwnd, &ti_script.rect);
	ti_script.rect.bottom = ti_script.rect.top + 30;
	::SendMessage(hTTscript, TTM_ACTIVATE, TRUE, 0);	
	::SendMessage(hTTscript, TTM_SETMAXTIPWIDTH, 0, 400);
	//These two functions are typically called in pair--so sigproc and graffy can communicate with each other for GUI updates, etc.
	return TRUE;
}

void CPlotDlg::OnCommand(int idc, HWND hwndCtl, UINT event)
{
	int res(0);
	switch (idc)
	{
	case IDM_CHANNEL_TOGGLE:
	case IDM_FULLVIEW:
	case IDM_ZOOM_IN:
	case IDM_ZOOM_OUT:
	case IDM_LEFT_STEP:
	case IDM_RIGHT_STEP:
	case IDM_TRANSPARENCY:
#ifndef NO_PLAYSND
	case IDM_PLAY:
	case IDM_STOP:
#endif
#ifndef NO_FFTW
	case IDM_SPECTRUM:
#endif
#ifndef NO_SF
	case IDM_WAVWRITE:
#endif
	case IDM_SPECTROGRAM:
	case IDM_ZOOMSELECT:
		OnMenu(idc);
		break;
	}
}

void CPlotDlg::GetSignalIndicesofInterest(int code, int & ind1, int &ind2)
{// This returns the indices of the "main" signal first axis, m_ln[0].sig currently selected, based on xlim, and curRange.
 // if not selected, ind1 and ind2 are the indices of the screen viewing range
 // Assumption: the signal is audio without chain
	
	CAxis *pax(gcf.ax.front());
	int fs = pax->m_ln[0]->sig.GetFs();
	if (code) 
	{
		ind1 = (int)(pax->pix2timepoint(curRange.px1)*fs+.5);
		ind2 = (int)(pax->pix2timepoint(curRange.px2)*fs);
	}
	else
	{
		ind1 = (int)(pax->xlim[0]*fs+.5);
		ind2 = (int)(pax->xlim[1]*fs);
	}
}

POINT CPlotDlg::GetIndDisplayed(CAxis *pax)
{ // This returns the indices (x for begin and y for end) of the "main" signal first axis, m_ln[0].sig	currently displayed on the screen (based on current xlim)
	POINT out = {0,0};
	if (gcf.ax.size()==0) return out;
	if (gcf.ax.front()->m_ln.size()==0) return out;
	if (gcf.ax.front()->xlim[0]>=pax->xlim[1]) return out;
	CSignal *p = &pax->m_ln.front()->sig;
	int fs = p->GetFs();
	if (p->GetType()==CSIG_AUDIO)
	{ // Just assume one big chunk (no chain)....
		out.x = (int)(pax->xlim[0]*fs+.5);
		out.y = (int)(pax->xlim[1]*fs+.5);
	}
	else
	{
		if (pax->m_ln.front()->xdata==NULL) // xdata is just the sample index.
		{
			out.x = (int)ceil(pax->xlim[0])-1; // zero-based index
			out.y = (int)pax->xlim[1];
		}
		else // xdata is specified by the user.
		{ // Let's assume that xdata is monotonously increasing.
			float fxlim0((float)pax->xlim[0]);
			float fxlim1((float)pax->xlim[1]); // why float? the last point was missing (id was less than one than should have been) because xlim[1] was 3.9999999999999991 and xdata was 4
			for (int id=0; id<p->nSamples; id++)
				if (pax->m_ln.front()->xdata[id]>=fxlim0) { out.x = id; break;}
			for (int id=p->nSamples-1; id>=0; id--)
				if (pax->m_ln.front()->xdata[id]<=fxlim1) { out.y = id; break;}
		}
	}
	return out;
}

vector<POINT>  CPlotDlg::makeDrawVector(CSignal *p, CAxis *pax)
{
	int id(0), beginID, ind1, ind2 ;
	int fs = pax->m_ln.front()->sig.GetFs();
	vector<POINT> draw;
	if (pax->xlim[0]>=pax->xlim[1]) return draw;
	CPoint pt;
	double xPerPixel = (pax->xlim[1]-pax->xlim[0]) / (double)pax->rcAx.Width(); // How much advance in x-axis per one pixel--time for audio-sig, sample points for nonaudio plot(y), whatever x-axis means for nonaudio play(x,y)
	double nSamplesPerPixel; // how many sample counts of the sig are covered per one pixel. Calculated as a non-integer, rounded-down value is used and every once in a while the remainder is added
	POINT pp = GetIndDisplayed(pax);//The indices of xlim[0] and xlim[1], if p were to be a single chain.
	ind1 = pp.x;
	ind2 = pp.y; //this is just the right end value, not the y-axis value
	int inttmark, endttmark;
	if (p->GetType()==CSIG_AUDIO) 
	{ // if p is one of multiple chains, ind1 and ind2 need to be adjusted to reflect indices of the current p.
		nSamplesPerPixel = xPerPixel * fs; 
		if (p->tmark>=pax->xlim[1]*1000. || p->endt()<=pax->xlim[0]*1000.) return draw;//return empty
		inttmark = (int)(p->tmark*fs/1000.+.5);
		ind1 = max(ind1, inttmark);
		endttmark = (int)((p->endt()/1000)*fs+.5);
		ind2 = min(ind2, endttmark);
	}
	else
	{
		nSamplesPerPixel = xPerPixel * (pax->xlim[1]-pax->xlim[0]) / (ind2-ind1) ; 
		inttmark = ind1;
		endttmark = ind2;
	}

	double multiplier = 2.;
	double maax, miin;
	int nPixels = pax->rcAx.right-pax->rcAx.left+1; // pixel count to cover the plot

	(p->tmark<=pax->xlim[0]*1000.) ?  beginID = (int)((pax->xlim[0]-p->tmark/1000.)*(double)fs +.5) : beginID = 0;
	int estimatedNSamples = (int)((pax->xlim[1]-pax->xlim[0])*(double)fs);
	if (!pax->hChild && pax->m_ln.front()->sig.GetType()==CSIG_VECTOR)  // For FFT, go to full drawing preemptively...
		estimatedNSamples=0; 
	double remnant(0);
	int adder;
	if (estimatedNSamples>multiplier*nPixels) // Quick drawing // condition: the whole nSamples points are drawn by nPixels points in pax->rcAx
	{
		do {
		remnant += nSamplesPerPixel - (int)(nSamplesPerPixel);
		if (remnant>1) {adder = 1; remnant -= 1.;}
		else adder = 0;
		id = min(beginID+(int)(nSamplesPerPixel)+adder, p->nSamples);
		if (!p->IsLogical())
		{
			body temp(p->buf+beginID, id-beginID);
			miin = temp.Min();
			maax = temp.Max();
		}
		else
		{
			body temp(p->logbuf+beginID, id-beginID);
			miin = temp.Min();
			maax = temp.Max();
		}
		if (miin!=1.e100) 
		{
			if (pax->m_ln.front()->xdata!=NULL) // for non-audio, plot(x,y) 
			{
				pt = pax->double2pixelpt(pax->m_ln.front()->xdata[beginID], miin, NULL);
				pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				draw.push_back(pt);
				pt = pax->double2pixelpt(pax->m_ln.front()->xdata[beginID], maax, NULL);
				pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
			}
			else
			{
				if (p->GetType()==CSIG_AUDIO) // for audio, plot(x)
				{
					pt = pax->double2pixelpt((double)beginID/(double)fs+p->tmark/1000., miin, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
					draw.push_back(pt);
					pt = pax->double2pixelpt((double)beginID/(double)fs+p->tmark/1000., maax, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				}
				else // for non-audio, plot(x)
				{
					double xval = pax->xlim[0] + (double) (beginID-ind1) / (ind2-ind1) * (pax->xlim[1]-pax->xlim[0]);
					pt = pax->double2pixelpt((double)xval, miin, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
					draw.push_back(pt);
					pt = pax->double2pixelpt((double)xval, maax, NULL);
					pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
					if (id>ind2-ind1-100)
						miin=11111;
				}
			}
			if (p->IsLogical() && maax>0.) 
				pt.y--; // to show inside the axis box
			draw.push_back(pt);
		}
		beginID  = id; 
	} while (id<min(p->nSamples, ind2));
	}
	else // Full drawing 
	{
		if (p->GetType()==CSIG_AUDIO)
			if (p->IsLogical())
				for (int k=ind1; k<ind2; k++)
				{
					pt = pax->double2pixelpt((double)k/fs, (double)p->logbuf[k-inttmark], NULL);
					draw.push_back(pt);
				}
			else
				for (int k=ind1; k<ind2; k++)
				{
					pt = pax->double2pixelpt((double)k/fs, p->buf[k-inttmark], NULL);
					draw.push_back(pt);
				}
		else
		{
			bool wasbull(0);
			if (pax->m_ln.front()->xdata==NULL) // plot(y) 
			{
				wasbull=true;
				pax->m_ln.front()->xdata = new double[p->nSamples];
				for (int k=0; k<p->nSamples; k++) pax->m_ln.front()->xdata[k] = (double)(k+1);
			}
			else
				ind2++; // The meaning of ind2 is different whether xdata is NULL or not, this line is necessary otherwise the last point is missing
			for (id=ind1; id<ind2; id++)
			{
				if (p->bufBlockSize==1)
					pt = pax->double2pixelpt(pax->m_ln.front()->xdata[id], p->logbuf[id], NULL);
				else
					pt = pax->double2pixelpt(pax->m_ln.front()->xdata[id], p->buf[id], NULL);
				pt.y = min(max(pax->rcAx.top, pt.y), pax->rcAx.bottom);
				draw.push_back(pt);
			}
			if (wasbull)
			{
				delete[] pax->m_ln.front()->xdata;
				pax->m_ln.front()->xdata=NULL;
			}
		}
	}
	return draw;
}

int CPlotDlg::GetCSignalsInRange(int code, CAxis *pax, CSignals &_sig, bool makechainless)
{
	// return 0: no CSignals available 
	//
	if (pax->m_ln.size()==0) return 0;
	int fs(pax->m_ln.front()->sig.GetFs());
	CSignal out(fs);
	CSignal chainless(fs);
	out = pax->m_ln.front()->sig;
	code &= (curRange == NO_SELECTION) ? 0 : 1;
	if (!GetSignalofInterest(code, out, makechainless)) return 0;
	_sig = out;
	//This only processes the first line or the first two lines.
	if (pax->m_ln.size()>1)
	{
		CSignals _sig2 = pax->m_ln[1]->sig;
		if (!GetSignalofInterest(code, _sig2, makechainless)) return 0;
		_sig.SetNextChan(&_sig2);
		if (makechainless) _sig.MakeChainless(); // need another MakeChainless() to even out a possible nSamples difference.
	}
	return 1;
}

void OnPaint_createpen_with_linestyle(CLine *pln, CDC& dc)
{
	switch(pln->lineStyle)
	{
	case LineStyle_solid:
		dc.CreatePen(PS_SOLID, pln->lineWidth, pln->color);
		break;
	case LineStyle_dash:
		dc.CreatePen(PS_DASH, pln->lineWidth, pln->color);
		break;
	case LineStyle_dot:
		dc.CreatePen(PS_DOT, pln->lineWidth, pln->color);
		break;
	case LineStyle_dashdot:
		dc.CreatePen(PS_DASHDOT, pln->lineWidth, pln->color);
		break;
	default:
		dc.CreatePen(PS_NULL, 0, 0);
		break;
	}
}


void CPlotDlg::OnPaint() 
{
	PAINTSTRUCT  ps;
	if (hDlg==NULL) return;
	HDC hdc = BeginPaint(hDlg, &ps);
	if (hdc==NULL) { EndPaint(hDlg, &ps); return; }
	CDC dc(hdc, hDlg);
	CClientDC dc2(hDlg);
	CPoint pt;
	vector<POINT> draw;
	char buf[512];
	CRect clientRt;
	CAxis* pax;
	double xlims[2];
	double maax(-1.e100), miin(1.e100);
	GetClientRect(hDlg, &clientRt);
	if (clientRt.Height()<15) { EndPaint(hDlg, &ps); return; }
	dc.SolidFill(gcf.color, clientRt);
	static int count(0);
	CRect rt;
	GetClientRect(hDlg, &rt);
	if (gcf.ax.size()>0)
	{
		CAxis *pax0 = gcf.ax.front();
		int nax(1);
		// drawing lines
		bool paxready(false);
		for (size_t k=0; k<gcf.ax.size();)
		{
			if (find(sbinfo.vax.begin(), sbinfo.vax.end(), gcf.ax[k]) == sbinfo.vax.end())
				sbinfo.vax.push_back(gcf.ax[k]);
		//the reason I changed from iterator to indexing---
		// now, all axes are not necessarily in the ax vector. some (e.g., FFT axis) are linked from another axis and it's incredibly difficult to make it with an iterator 
		// 8/12/2017 bjk
			if (!paxready) 
				pax = gcf.ax[k];
			else
				paxready=false;
			if (!pax->visible) continue;
			if (!axis_expanding)
				CPosition lastpos(pax->pos);
			else
				pax->pos.Set(clientRt, pax->axRect); // do this again
			pax->rcAx=DrawAxis(&dc, &ps, pax);
			//when mouse is moving while clicked
			if (curRange != NO_SELECTION && pax->hPar->type==GRAFFY_figure ) // this applies only to waveform axis (not FFT axis)
			{
				rt = pax->axRect;
				rt.left = curRange.px1+1; 
				rt.right = curRange.px2-1; 
				rt.top--;
				rt.bottom++;
				dc.SolidFill(selColor, rt);
				if (ClickOn)
				{
					dc.CreatePen(PS_DOT, 1, RGB(255, 100, 0));
					dc.MoveTo(curRange.px1, rt.bottom-2);
					dc.LineTo(curRange.px1, rt.top+1); 
					dc.MoveTo(curRange.px2, rt.bottom-2);
					dc.LineTo(curRange.px2, rt.top+1); 
				}
			}
			size_t nLines = pax->m_ln.size(); // just FYI
			for (vector<CLine*>::iterator liit=pax->m_ln.begin(); liit!=pax->m_ln.end(); liit++)  
			{
				OnPaint_createpen_with_linestyle(*liit, dc);
				xlims[0] = miin;		xlims[1] = maax;
				buf[0] = (*liit)->symbol, buf[1] = 0;
				if ((*liit)->lineWidth>0)
				{
					for (CSignal *p = &((*liit)->sig); p ; p = p->chain)
					{
						draw = makeDrawVector(p, pax);
						if (p->GetType()==CSIG_AUDIO)	{
							if (pt.y < pax->axRect.top)  pt.y = pax->axRect.top;
							if (pt.y > pax->axRect.bottom) pt.y = pax->axRect.bottom;
						}
						if (draw.size()>0)
						{
							if ((*liit)->lineStyle!=LineStyle_noline)
								if (draw[draw.size()-1].x <= pax->axRect.right)
									dc.Polyline(draw.data(), (int)draw.size());
								else
								{
									int shortlen(-1);
									int nDrawPt = (int)draw.size();
									for (int p = nDrawPt-1; p>0; p--)
										if (draw[p].x<=pax->axRect.right)
											shortlen = p, p=-1;
									if (shortlen>0)
										dc.Polyline(draw.data(), shortlen);
								}
							if (p->GetType()!=CSIG_AUDIO && buf[0]!=0)	//this is where markers are drawn
								DrawMarker(dc, *liit, draw);
						}
					}
				}
				dc.SetBkColor(pax->color);
			} 
			// add ticks and ticklabels
			dc.SetBkColor(gcf.color);
			// For the very first call to onPaint, axRect is not known so settics is skipped, and need to set it here
			// also when InvalidateRect(NULL) is called, always remake ticks
			if (pax->m_ln.size() > 0)
			{
				if ( pax->xtick.tics1.size()==0 || CRect(ps.rcPaint) == clientRt )
					if (pax->m_ln.front()->sig.GetType()==CSIG_AUDIO)
						pax->xtick.tics1 = makefixtick(pax->xlim[0], pax->xlim[1], pax->GetDivCount('x', -1));
					else
						pax->setxticks();
				if (pax->ytick.tics1.size()==0 || pax->ylim[1]-pax->ylim[0] >  (pax->ytick.tics1.back()-pax->ytick.tics1.front())*2 )
				{
					if (pax->m_ln.front()->sig.bufBlockSize==1)
					{
						vector<double> tics;
						tics.push_back(0); tics.push_back(1);
						pax->ytick.tics1 = tics;
					}
					else
						pax->ytick.tics1 = makefixtick(pax->ylim[0], pax->ylim[1], pax->GetDivCount('y', -1));
				}
				DrawTicks(&dc, pax, 0);

				//x & y labels
				dc.SetTextAlign(TA_RIGHT|TA_BOTTOM);
				if (!gcf.ax.empty() && !gcf.ax.front()->m_ln.empty())
				{
					if (IsSpectrumAxis(pax))
					{
						dc.TextOut(pax->axRect.right-3, pax->axRect.bottom, "Hz");
						dc.TextOut(pax->axRect.left-3, pax->axRect.top-1, "dB");
					}
					else if (pax->m_ln.front()->sig.GetType()==CSIG_AUDIO)
						dc.SetBkMode(TRANSPARENT), dc.TextOut(pax->axRect.right-3, pax->axRect.bottom, "sec");
				}
			}
			if (pax->hChild) 	
				paxready=true, pax = (CAxis*)pax->hChild;
			else
				k++;
		}
		//CAxis *paxx(gcf.ax.front()); 
		//if (LRrange(&paxx->rcAx, playLoc, 'x')==0) 
		//{
		//	dc.CreatePen(PS_SOLID, 1, RGB(204,77,0));
		//	dc.MoveTo(playLoc, gcf.ax.front()->axRect.bottom);
		//	dc.LineTo(playLoc, gcf.ax.front()->axRect.top); 
		//	setpbprogln(playLoc);
		//}

		if (pax0->m_ln.size()>0 || (gcf.ax.size()>1 && gcf.ax[1]->m_ln.size()>0) )
		{
			sbinfo.xBegin = pax0->xlim[0];
			sbinfo.xEnd = pax0->xlim[1];
		}
	} // k<gcf.ax.size()
	//Drawing texts
	for (vector<CText*>::iterator txit=gcf.text.begin(); txit!=gcf.text.end(); txit++) { 
		if ((*txit)->pos.x0>=0 && (*txit)->pos.y0>=0)
		{
			dc.SelectObject(&(*txit)->font);
			pt.x = (int)((double)clientRt.right * (*txit)->pos.x0+.5);
			pt.y = (int)((double)clientRt.bottom * (1-(*txit)->pos.y0)+.5);
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor((*txit)->color);
//			dc.DrawText((*txit)->str.c_str(),-1, &(*txit)->textRect, (*txit)->alignmode | DT_NOCLIP);
			DWORD dw = (*txit)->alignmode;
			DWORD dw2 = TA_RIGHT|TA_TOP;
			DWORD dw3 = TA_CENTER|TA_BASELINE;
			dc.SetTextAlign(dw);
			dc.TextOut(pt.x, pt.y, (*txit)->str.c_str());
		}
		else
		{
			::SendMessage(hTTscript, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti_script);
			::SendMessage(hTTscript, TTM_UPDATETIPTEXT, 0,  (LPARAM) (LPTOOLINFO) &ti_script);
		}
	}
	if (count++==50)
	{
		pt.x++;
		pt.x--;
	}
	EndPaint(hDlg, &ps);
	if (!sbinfo.initialshow) {sbinfo.initialshow=true; ShowStatusBar();}
	// vector gcf.ax is pushed in AddAxis() called in showvar.cpp 
	// find out why sometimes gcf.ax[k] is pointing something already deleted and causing a crash here. 8/216/2017 bjk
}

void CPlotDlg::DrawTicks(CDC *pDC, CAxis *pax, char xy)
{
	LOGFONT fnt;
	int loc;
	double value;
	int lastpt2Draw(-32767), cum(0);
	double nextpt;
	CSize sz;
	char label[256], tempformat[8];
	HDC hdc = HDC(*pDC);
	vector<double>::iterator it;
	CRect out;
	double step, scale, scalemant;
	strcpy(tempformat,"%d");
	switch(xy)
	{
	case 'x':
		pDC->SetTextAlign (TA_CENTER);
		//find the first point in tics1 in the range and update loc
		for (it=pax->xtick.tics1.begin(); it !=pax->xtick.tics1.end(); it++)
		{
			if (pax->m_ln[0]->sig.GetType()==CSIG_VECTOR && pax->m_ln[0]->xdata==NULL) 
				loc = pax->double2pixel((int)(*it+.5), xy);
			else																	
				loc = pax->double2pixel(*it, xy);
			if (LRrange(&pax->rcAx, loc, xy)==0) break;
		}
		//iterate within the range
		for (; it !=pax->xtick.tics1.end() && LRrange(&pax->rcAx, loc, xy)==0 ; it++)
		{
			nextpt = *it;
			if (pax->m_ln[0]->sig.GetType()==CSIG_VECTOR && pax->m_ln[0]->xdata==NULL)	
				loc = pax->double2pixel((int)(nextpt+.5), xy); 
			else											
				loc = pax->double2pixel(nextpt, xy); 
			if (LRrange(&pax->rcAx, loc, xy)>0) // if the current point is right side of axis
			{
				loc = pax->rcAx.right;
				value = max(0, pax->xtick.mult*pax->GetRangePixel(loc)+pax->xtick.offset);
			}
			else if (LRrange(&pax->rcAx, loc, xy)<0)  // if the current point is left side of axis, make it forcefully inside.
			{
				loc = pax->rcAx.left+1;
				value = max(0, pax->xtick.mult*pax->GetRangePixel(loc)+pax->xtick.offset);
				//further adjust tics1 according to the adjusted loc
			}
			else
				value = max(0, pax->xtick.mult*nextpt+pax->xtick.offset);
			loc = min(pax->rcAx.right-1, loc);  //loc should not be the same as pax->rcAx.right (then the ticks would protrude right from the axis rectangle)
			pDC->MoveTo(loc, pax->rcAx.bottom-1);
			pDC->LineTo(loc, pax->rcAx.bottom-1 - pax->xtick.size); 
			if (pax->m_ln[0]->sig.GetType()==CSIG_AUDIO && !pax->xtick.format) 
				strcpy(pax->xtick.format, "%4.2f"); // This is where two digits under decimal are drawn on the time axis.
			if (pax->xtick.format[0]!=0)
			{
				if (value>=1. || pax->xtick.mult==1.)	sprintf(label, pax->xtick.format, value);
				else			sprintf(label, tempformat, (int)(value/pax->xtick.mult+.5)); // needed for Spectrum axis, when freq is below 1k.
			}
			else
				sprintfFloat(value, 3, label, 256);
			GetTextExtentPoint32(hdc, label, (int)strlen(label), &sz);
			if (iabs(loc-lastpt2Draw)> sz.cx + pax->xtick.gap4next.x) // only if there's enough gap, Textout
			{
				pDC->TextOut(loc, pax->rcAx.bottom + pax->xtick.labelPos, label);
				lastpt2Draw = loc;
			}
			cum++;
		}
		break;
	case 'y':
		pDC->SetTextAlign (TA_RIGHT);
		fnt = pDC->GetLogFont(); 
		step = pax->ytick.tics1[1]-pax->ytick.tics1.front();
		scale = pow(10., ceil(log10(step)));
		scalemant = log10(scale);
		//find the first point in tics1 in the range and update loc
		for (it=pax->ytick.tics1.begin(); it !=pax->ytick.tics1.end(); it++)
		{
			loc = pax->double2pixel(*it, xy);
			if (LRrange(&pax->rcAx, loc, xy)==0) break;
		}
		//iterate within the range
		for (; it !=pax->ytick.tics1.end() && LRrange(&pax->rcAx, loc, xy)==0 ; it++)
		{
			nextpt = *it;
			loc = min(pax->double2pixel(nextpt, xy), pax->rcAx.bottom-1); //loc should not be the same as pax->rcAx.bottom (then the ticks would protrude downward from the axis rectangle)
			if (LRrange(&pax->rcAx, loc, xy)>0) // if the current point is above of axis
				loc = pax->rcAx.top;
			else if (LRrange(&pax->rcAx, loc, xy)<0)  // if the current point is left side of axis, make it forcefully inside.
				loc = pax->rcAx.bottom-1;
			value = nextpt;
			pDC->MoveTo(pax->rcAx.left, loc);
			pDC->LineTo(pax->rcAx.left + pax->ytick.size, loc);
			if (pax->ytick.format[0]!=0)
				sprintf(label, pax->ytick.format, value);
			else
				sprintfFloat(value, max(0,min(3,1-(int)scalemant)), label, 256);
			GetTextExtentPoint32(hdc, label, (int)strlen(label), &sz);
			if (iabs(loc-lastpt2Draw)> sz.cy + pax->xtick.gap4next.y) // only if there's enough gap, Textout
			{
				pDC->TextOut(pax->rcAx.left - pax->ytick.labelPos, loc-fnt.lfHeight/2, label);
				lastpt2Draw = loc;
			}
			cum++;
		}		
		break;
	default:
		if (pax->m_ln.size()==0) break; // so that it skips when FFT rountine doesn't produce output
		pDC->CreatePen(PS_SOLID, 1, pax->colorAxis);
		DrawTicks(pDC, pax, 'x');
		DrawTicks(pDC, pax, 'y');
		break;
	}
}

CRect CPlotDlg::DrawAxis(CDC *pDC, PAINTSTRUCT *ps, CAxis *pax)
{
	CRect rt;
	GetClientRect(hDlg, &rt);
	unsigned int rr = GetDoubleClickTime();
	char buf1[64];
	if (axis_expanding)
		pDC->CreatePen(PS_DOT, 1, pax->colorAxis), pDC->CreateHatchBrush(HS_BDIAGONAL, RGB(160, 170, 200));
	else
		pDC->CreatePen(PS_SOLID, 1, pax->colorAxis), pDC->CreateSolidBrush(pax->color);
	CRect rt3(pax->pos.GetRect(rt));
	LONG temp = rt3.bottom;
	rt3.bottom = rt3.top;
	rt3.top = temp;
	pDC->Rectangle(rt3);
	pax->axRect = rt3;
	SIZE sz (pDC->GetTextExtentPoint32 ("X", 5));
	pax->ytick.rt = CRect( CPoint(rt3.left-sz.cx, rt3.top), CPoint(rt3.left,rt3.bottom));
	pax->xtick.rt = CRect( CPoint(rt3.left,rt3.bottom), CSize(rt3.right-rt3.left,pax->xtick.labelPos+sz.cy));
	if (axis_expanding)
	{
		strcpy(buf1,"Adjust Window Size.");
		pDC->TextOut(50, 5, buf1);
		strcpy(buf1,"Press Ctrl key again to revert to the normal mode.");
		pDC->TextOut(80, 20, buf1);
	}

	pDC->CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	return rt3;
}


void CPlotDlg::OnSize(UINT nType, int cx, int cy) 
{
	int new1, new2;
	int sigtype(-1);
	if (gcf.ax.size()>0 && gcf.ax.front()->m_ln.size()>0)
		sigtype = gcf.ax.front()->m_ln.front()->sig.GetType();

	if (hStatusbar==NULL)
	{
		hStatusbar = CreateWindow (STATUSCLASSNAME, "", WS_CHILD|WS_VISIBLE|WS_BORDER|SBS_SIZEGRIP,
			0, 0, 0, 0, hDlg, (HMENU)0, hInst, NULL);
		int width[] = {40, 110, 2, 40, 40, 160,};
		int sbarWidthArray[8];
		sbarWidthArray[0] = 40;
		for (int k=1;k<8;k++) 
			sbarWidthArray[k] = sbarWidthArray[k-1] + width[k-1];
		sbarWidthArray[7]=-1;
		::SendMessage (hStatusbar, SB_SETPARTS, 12, (LPARAM)sbarWidthArray);
		SetHWND_GRAFFY(hDlg);
#ifndef NO_PLAYSND
		SetHWND_SIGPROC(GetParent(hDlg));
#endif // NO_PLAYSND
	}
	else
		::MoveWindow(hStatusbar, 0, 0, cx, 0, 1); // no need to worry about y pos and height
	
	for (vector<CAxis*>::iterator it=gcf.ax.begin(); it!=gcf.ax.end(); it++) 
	{
		(*it)->ytick.tics1 = makefixtick((*it)->ylim[0], (*it)->ylim[1], (*it)->GetDivCount('y', -1));
		if ( (*it)->m_ln.front()->sig.GetType()==CSIG_AUDIO)
			(*it)->xtick.tics1 = makefixtick((*it)->xlim[0], (*it)->xlim[1], (*it)->GetDivCount('x', -1));
		else
			(*it)->setxticks();
	}
	if (curRange != NO_SELECTION && gcf.ax.size()>0)
	{
		//need to adjust curRange according to the change of size 
		//new pixel pt after size change
		new1 = gcf.ax.front()->timepoint2pix(selRange.tp1);
		new2 = gcf.ax.front()->timepoint2pix(selRange.tp2);
		curRange.px1 = new1;
		curRange.px2 = new2;
	}
	if (hTTtimeax[0]==NULL)
	{
		for (int k=0; k<7; k++)
		{
			hTTtimeax[k] = CreateTT(hStatusbar, &ti_taxis);
			::SendMessage (hStatusbar, SB_GETRECT, k, (LPARAM)&ti_taxis.rect);
			ti_taxis.lpszText=(LPSTR)ttstat[k].c_str();
			::SendMessage(hTTtimeax[k], TTM_ACTIVATE, TRUE, 0);	
			::SendMessage(hTTtimeax[k], TTM_SETMAXTIPWIDTH, 0, 400);
			::SendMessage(hTTtimeax[k], TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti_taxis);
			::SendMessage(hTTtimeax[k], TTM_UPDATETIPTEXT, 0,  (LPARAM) (LPTOOLINFO) &ti_taxis);
			//if (k==2) k++;
			//if (k==4) k++;
		}
	}
	::SendMessage(hTTscript, TTM_ACTIVATE, TRUE, 0);
	InvalidateRect	(NULL);
}
//Convention: if a figure handle has two axes, the first axis is the waveform viewer, the second one is for spectrum viewing.
void CPlotDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (gcf.ax.empty()) return;
	CAxis *pax = CurrentPoint2CurrentAxis(&point);
	if (pax!=NULL)
	{
		int iSel(-1);
		for (size_t i=0; i<gcf.ax.size(); i++) if (pax==gcf.ax[i]) iSel=(int)i;
		//Following the convention
		subMenu.EnableMenuItem(IDM_PLAY, iSel==0? MF_ENABLED : MF_GRAYED);
		subMenu.EnableMenuItem(IDM_WAVWRITE, iSel==0? MF_ENABLED : MF_GRAYED);
		subMenu.EnableMenuItem(IDM_SPECTRUM, iSel==0? MF_ENABLED : MF_GRAYED);
		ClientToScreen(hDlg, &point);    // To convert point to re: the whole screen 
		TrackPopupMenu(subMenu.GetHandle(), TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, 0);
	}
}

void CPlotDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
}

void CPlotDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetGCF();
	edge.px1 = edge.px2 = -1;
	gcmp=point;
	CAxis *cax = CurrentPoint2CurrentAxis(&point);
//	UpdateRects(cax);
	if (axis_expanding) {ClickOn=0; return;}
	ClickOn = GetMousePos(point);
	clickedPt = point;
	switch(ClickOn = GetMousePos(point)) // ClickOn indicates where mouse was click.
	{
	case RC_WAVEFORM:  //0x000f
		if (curRange != NO_SELECTION) // if there's previous selection
		{
			ShowStatusBar();
//			ShowStatusS electionOfRange(cax,"off");
			for (size_t k=0; k<gcf.ax.size(); k++)
			{
				CRect rt(curRange.px1, gcf.ax[k]->axRect.top, curRange.px2+1, gcf.ax[k]->axRect.bottom+1);
				InvalidateRect(&rt);
			}
		}
		lbuttondownpoint.x = gcmp.x;
		curRange.reset();
		{
			CClientDC dc(hDlg);
			dc.CreatePen(PS_DOT, 1, RGB(255, 100, 0));
			dc.MoveTo(gcmp.x, cax->axRect.bottom-1);
			dc.LineTo(gcmp.x, cax->axRect.top+1); 
		}
		break;
	default:
		gcmp=CPoint(-1,-1);
		break;
	}
}

void CPlotDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rt;
	if (axis_expanding | gcf.ax.empty()) {ClickOn=0; return;}
	CAxis *cax = CurrentPoint2CurrentAxis(&point);
	CAxis *pax = gcf.ax.front();
	if (curRange.px2<0) // if button up without mouse moving, reset
		curRange.reset();
	CSignals _sig;
	clickedPt.x=-999; clickedPt.y=-999;
	lastPtDrawn.x=-1; lastPtDrawn.y=-1;
	switch(ClickOn)
	{
	case RC_WAVEFORM:  //0x000f
		if (point.x > pax->axRect.right || point.x < pax->axRect.left) // mouse was up outside axis
			cax = pax;
		rt.top = cax->axRect.top;
		rt.bottom = cax->axRect.bottom+1;
		rt.left = point.x-1; //default
		rt.right = point.x+1; //default
		if (point.x > cax->axRect.right) 
		{
			rt.left = curRange.px2-1;
			rt.right = curRange.px2 = cax->axRect.right;
		}
		else if (point.x < cax->axRect.left) 
		{
			rt.right = curRange.px1+1;
			rt.left = curRange.px1 = cax->axRect.left+1;
		}
		else
		{
			if (point.x > lbuttondownpoint.x) // moving right
			{
				curRange.px2 = point.x;
				if (edge.px1>curRange.px2) { //moved right and left
					rt.left = curRange.px2;
					rt.right = edge.px1+2; }
			}
			else
			{
				curRange.px1 = point.x;
				if (edge.px1>0 && edge.px1<curRange.px1) { //moved left and right
					rt.left = edge.px1-2;
					rt.right = curRange.px1;
				}
			}
		}
		if (curRange.px2-curRange.px1<=3) curRange.reset();
		InvalidateRect(&rt);
		selRange.tp1 = cax->pix2timepoint(curRange.px1); 
		selRange.tp2 = cax->pix2timepoint(curRange.px2); 
		if (ClickOn && gcf.ax.front()->hChild)
			OnMenu(IDM_SPECTRUM_INTERNAL);
		break;
	default:
		unsigned short mask = ClickOn & 0xff00;
		if ( mask==RC_SPECTRUM_XTICK || mask==RC_SPECTRUM_YTICK )
		{
			if ( (MoveOn & RC_SPECTRUM_XTICK) | (MoveOn & RC_SPECTRUM_YTICK) )	MoveOn = 0;
		}
		else if (mask==RC_SPECTRUM)
			if (1)// (MoveOn & RC_SPECTRUM)
			{
				ChangeColorSpecAx(CRect(((CAxis*)gcf.ax.front()->hChild)->axRect), MoveOn = (bool)0);
				InvalidateRect(NULL);
			}
			else
			{
//				UpdateRects(cax);
				for (int k=0; k<5; k++) InvalidateRect(&roct[k]);
			}
		break;
	}
	ClickOn = 0;
	gcmp=CPoint(-1,-1);

	for (vector<CAxis*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
	{
		CAxis* paxFFT;
		if (paxFFT = (CAxis*)(*axt)->hChild) ShowSpectrum(paxFFT, *axt);
	}
}

void CPlotDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	static int count(0);
	double x, y;
	if (axis_expanding) return;
	CAxis *cax = CurrentPoint2CurrentAxis(&point);
	unsigned short  mousePt = GetMousePos(point);
	CRect rt, rect2Invalidate;
	char buf[64], buf2[64], buf0[64];
	CPoint shift;
	CAxis *paxFFT = (CAxis*)gcf.ax.front()->hChild;
	switch(mousePt & 0xff00) // spectrum axis has the priority over waveform axis 
	{
	case RC_SPECTRUM:
		sprintf(buf,"%.1fHz",paxFFT->pix2timepoint(point.x));
		paxFFT->GetCoordinate(point, x, y);
		if (gcf.ax.size()>1)
		{
			CAxis *paxFFT2 = (CAxis*)gcf.ax.back()->hChild;
			if (GetMousePosAxis(point, paxFFT2))
			{
				paxFFT2->GetCoordinate(point, x, y);
				paxFFT = paxFFT2;
			}
		}
		sprintf(buf2,"%.2f",y);
		sprintf(buf0, "(%s,%s)", buf, buf2);
		ShowStatusBar(CURSOR_LOCATION_SPECTRUM, buf0);
		KillTimer(CUR_MOUSE_POS);
		SetTimer(CUR_MOUSE_POS, 2000, NULL);
		if (!(ClickOn & RC_SPECTRUM)) break;
		if (lastPtDrawn.x>0 && lastPtDrawn.y>0) // moving while button down 
		{
			shift = point - lastPtDrawn;
			CRect axRectOld(paxFFT->axRect);
			CRect clientrt;
			GetClientRect(hDlg, &clientrt);
			paxFFT->axRect.MoveToXY(paxFFT->axRect.TopLeft()+shift); // new top left point is shifted
			paxFFT->pos.Set(clientrt, paxFFT->axRect);
		}
		rect2Invalidate = paxFFT->GetWholeRect();
		rect2Invalidate.top -= 15;
		rect2Invalidate.right += 20;
		if (shift.x!=0 || shift.y!=0)
			ChangeColorSpecAx(CRect(paxFFT->axRect), true);
		InvalidateRect(rect2Invalidate); 
		lastPtDrawn = point;
		break;
	case RC_SPECTRUM_YTICK:
	// If the y-axis of spectrum needs to be adjusted with mouse-dragging
	// add the code here.
		break;
	case RC_SPECTRUM_XTICK:
		if (!(ClickOn & RC_SPECTRUM_XTICK) & !(ClickOn & RC_SPECTRUM_YTICK)) break; 
		if (lastPtDrawn.x>0 && lastPtDrawn.y>0) // moving while button down 
		{
			shift = point - lastPtDrawn;
			if (shift.x!=0)
			{
				int k(iabs(shift.x));
				if (k>6 && k<15) k=7;
				else if (k>13) k=10;
				if (shift.x>0)	
				{
					for (; k>0; k--)	paxFFT->xlim[1] *= .95 ;
					paxFFT->xlim[1] = max(100, paxFFT->xlim[1]);
				}
				else  /* shift.x<0 */
				{
					for (; k>0; k--)	paxFFT->xlim[1] /= .95 ;
					paxFFT->xlim[1] = min(paxFFT->xlim[1], paxFFT->xlimFull[1]);
				}
				rect2Invalidate = paxFFT->GetWholeRect();
				rect2Invalidate.right += 5 + (int)(rect2Invalidate.Width()/10);
				InvalidateRect(rect2Invalidate); 
				paxFFT->setxticks();
			}
		}
		lastPtDrawn = point;
		break;
	}
	if (mousePt == 0x000f) // RC_WAVEFORM
	{
		for (size_t k=0; k<gcf.ax.size(); k++)
		{
			cax = gcf.ax[k];
			if (ClickOn & (unsigned short)128) lbuttondownpoint.x = cax->axRect.left;
			else if (ClickOn & (unsigned short)32) lbuttondownpoint.x = cax->axRect.right;
			if (ClickOn)
			{ // lbuttondownpoint.x is the x point when mouse was clicked
				rt.top = cax->axRect.top+1;
				rt.bottom = cax->axRect.bottom-1;
				if (point.x > lbuttondownpoint.x) // current position right side of the beginning point
				{
					if (edge.px1==-1) edge.px1 = lbuttondownpoint.x;
					curRange.px1 = lbuttondownpoint.x;
					curRange.px2 = point.x;
					edge.px2 = max(lastpoint.x, max(edge.px2, point.x));
					rt.right = edge.px2;

					// If move left-right and passed the beginning point,i.w., lbuttondownpoint, keep the edge.px1 in rt, so it can be properly redrawn
					if ( (point.x-lbuttondownpoint.x)*(point.x-edge.px1)>0) 
						rt.left = edge.px1-1;
					else
						rt.left = curRange.px1;	
					edge.px1 =  max( edge.px1, curRange.px2);
				}
				else if (point.x < lbuttondownpoint.x) // moving left
				{
					if (edge.px1==-1) edge.px1 = lbuttondownpoint.x;
					curRange.px2 = lbuttondownpoint.x;
					curRange.px1 = point.x;
					if (point.x>lastpoint.x) // moving left but just turned right 
						rt.left = lastpoint.x;
					else
						rt.left = curRange.px1;
					// If move right-left and passed the beginning point, i.e., lbuttondownpoint), keep the edge.px1 in rt, so it can be properly redrawn
					if ( (point.x-lbuttondownpoint.x)*(point.x-edge.px1)>0) 
						rt.right = edge.px1+1;
					else
						rt.right = curRange.px2;
					edge.px1 =  min(edge.px1, curRange.px1);
				}
				else 
					curRange.reset();
				if (curRange != NO_SELECTION)
					InvalidateRect(&rt);
				lastpoint = point;

			}
			else edge.px1 = -1;
			ShowStatusBar(FULL);
		}
		cax = CurrentPoint2CurrentAxis(&point);
		cax->GetCoordinate(point, sbinfo.xCur, sbinfo.yCur);
		sbinfo.xSelBegin = gcf.ax.back()->pix2timepoint(curRange.px1);
		sbinfo.xSelEnd = gcf.ax.back()->pix2timepoint(curRange.px2);
		ShowStatusBar(CURSOR_LOCATION);
//		KillTimer(CUR_MOUSE_POS);
//		SetTimer(CUR_MOUSE_POS, 2000, NULL);
		selRange.tp1 = cax->pix2timepoint(curRange.px1); 
		selRange.tp2 = cax->pix2timepoint(curRange.px2); 
	}
	if (mousePt==0) // outside ANY axis
	{ // clean the status of current location 
		::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)"");
	}
}

void CPlotDlg::SetGCF()
{
	if (hDlg!=GetForegroundWindow())	SetForegroundWindow(hDlg);
	::PostMessage(GetHWND_GRAFFY(), WM_GCF_UPDATED, (WPARAM)&gcf, (LPARAM)hDlg);
}

#define LOG(X) fprintf(fp,(X));

void CPlotDlg::setpbprogln(int curPtx)
{
	for (vector<CAxis*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
	{
		CAxis *pax = *axt;
		playLoc = curPtx;
		InvalidateRect(CRect(playLoc-1, pax->axRect.top, playLoc+1, pax->axRect.bottom),0);
	}
}

void CPlotDlg::OnSoundEvent(int index, __int64 bufferlocation)
{//index is the number sent by CWavePlay as each playback block of data is played successfully
 //bufferlocation is current point location of the data block (not really useful on the application side)
 // Both arguments 0 means the playback device is opened and ready to go.
 // index=-1 means playback ended and device is being closed.

 // if the overall duration is too short, the playback progress line may appear to move too sparsely
 // yet you can't do anything about it... Tried to use WM_TIMER (hoping that in between these OnSoundEvent callbacks 
 // kind of "fake" progress line would show up to make it move smoothly), but it wasn't useful--
 // For the most part, WM_TIMER wasn't being processed as frequently as needed---remember it has a very
 // low priority and it was hardly processed if all while this callback was processed.

	//FILE *pot = fopen("playback_track.txt","at");
	//fprintf(pot, "index=%d, bufferlocation=%d, playLoc=%d --> ", index, bufferlocation, playLoc);
	CAxis *pax=gcf.ax.front();
	if (index==0 && bufferlocation==0)
	{
		int leftEdge = (curRange == NO_SELECTION)? pax->axRect.left: curRange.px1;
		setpbprogln(leftEdge);
	}
	else if (index==-1)
	{
		KillTimer(PLAYBACKPROGRESS);
		playLoc=-1;
//		InvalidateRect(NULL);
	}
	else
	{
		if (curRange == NO_SELECTION)
			playLoc = pax->timepoint2pix(pax->xlim[0]+block*++index/1000);
		else
			playLoc = pax->timepoint2pix(selRange.tp1+block*++index/1000);
		setpbprogln(playLoc);
	}
	//fprintf(pot, "%d\n", playLoc);
	//fclose(pot);
}

void CPlotDlg::ChangeColorSpecAx(CRect rt, bool onoff)
{// on: ready to move, off: movind done
	CAxis* paxFFT = (CAxis*)gcf.ax.front()->hChild;
	static COLORREF col1, col2;
	if (!onoff)
	{
		paxFFT->color = col2;
		col1 = col2;
	}
	else if (col1 == col2)
	{
		BYTE r = GetRValue(paxFFT->color);
		BYTE g = GetGValue(paxFFT->color);
		BYTE b = GetBValue(paxFFT->color);
		col2 = paxFFT->color;
		col1 = paxFFT->color = RGB(r*7/8,g*7/8,b*7/8);
	}
	InvalidateRect(&rt);
}

void CPlotDlg::OnTimer(UINT id)
{
	switch (id)
	{
	case PLAYBACKPROGRESS:
		playLoc += 2;
		setpbprogln(playLoc);
		break;

	case CUR_MOUSE_POS:
		::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
		KillTimer(id);
		break;
	case MOVE_SPECAX:
		if (ClickOn & RC_SPECTRUM)
		{
			CAxis *ax= (CAxis*)gcf.ax.front()->hChild;
			BYTE r = GetRValue(ax->color);
			BYTE g = GetGValue(ax->color);
			BYTE b = GetBValue(ax->color);
			ax->color = RGB(g,b,r);
			CRect rt(ax->axRect.left, ax->axRect.top, ax->axRect.right, ax->axRect.bottom);
			InvalidateRect(&rt);
		}
		KillTimer(id);
		break;
	}
}

int CPlotDlg::IsSpectrumAxis(CAxis* pax)
{ // return true when xtick.tics1 is empty or its full x axis right edge.px1 is half the sample rate of the front signal
	if (pax == gcf.ax.front())	return 0;
	int fs = gcf.ax.front()->m_ln.front()->sig.GetFs();
	if ( abs((int)(pax->xlimFull[1] - fs/2)) <= 2 ) return 1;
	else return 0;
}

void CPlotDlg::OnMenu(UINT nID)
{
	if (axis_expanding) return;
	char fullfname[MAX_PATH];
	char fname[MAX_PATH];
	CFileDlg fileDlg;
	CAxis *cax(gcf.ax.front()); // Following the convention
	CAxis *paxFFT;
	CRect rt;
	CSize sz;
	CFont editFont;
	CPosition pos;
	int  len, iMul(1);
	char errstr[256];
	CSignals _sig, _sig2play, chainlessed;
	CSignal dummy;
	double dfs;
	char buf[64];
	int deltapixel;
	vector<CLine*> swappy;
	double shift, newlimit1,  newlimit2, deltaxlim, dval;
	INT_PTR res;
	errstr[0]=0;
	CPosition SpecAxPos(.75, .6, .22, .35);
	switch (nID)
	{
	case IDM_CHANNEL_TOGGLE:
		if (gca->m_ln.size()<2) break;
		swappy.push_back(gca->m_ln[1]);
		swappy.push_back(gca->m_ln[0]);
		gca->m_ln = swappy;
		InvalidateRect(NULL);
		break;

	case IDM_TRANSPARENCY:
		dval = (double)opacity/255.*100.;
		sprintf(buf,"%d",(int)(dval+.5));
		res = InputBox("Transparency", "transparent(0)--opaque(100)", buf, sizeof(buf));
		if (res==1 && strlen(buf)>0)
		{
			if (res=sscanf(buf,"%lf", &dval))
			{
				if (dval<=100. && dval>=0.)
				{
					opacity = (unsigned char)(dval/100.*255-.5);
					SetLayeredWindowAttributes(hDlg, 0, opacity, LWA_ALPHA); // how can I transport transparency from application? Let's think about it tomorrow 1/6/2017 12:19am
				}
			}
		}
		break;

	case IDM_FULLVIEW:
		zoom=0;
		for (vector<CAxis*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
		{
			cax = *axt;
			memcpy(cax->xlim, cax->xlimFull, sizeof(double)*2);
			if ( cax->m_ln.front()->sig.GetType()==CSIG_AUDIO)
				cax->xtick.tics1 = makefixtick(cax->xlim[0], cax->xlim[1], cax->GetDivCount('x', -1));
			else
			{// it redraws the xticks only when it is the first axis (when it is nonaudio), 
				if (axt==gcf.ax.begin() || cax->xtick.tics1.empty())
					cax->setxticks();
			}
			cax->ytick.tics1 = makefixtick(cax->ylim[0], cax->ylim[1], cax->GetDivCount('y', -1));
			if (paxFFT = (CAxis*)cax->hChild) ShowSpectrum(paxFFT, cax);
		}
		ShowStatusBar();
		return;

	case IDM_ZOOM_IN:
		if (cax->xlim[1] - cax->xlim[0]<0.009) 
			return; // no zoom for 5ms of less
	case IDM_ZOOM_OUT:
		for (vector<CAxis*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
		{
			cax = *axt;
			if (nID == IDM_ZOOM_OUT && fabs(cax->xlim[1]-cax->xlimFull[1])<1.e-5 && fabs(cax->xlim[0]-cax->xlimFull[0])<1.e-5) 
				return; // no unzoom if full length
			if (nID == IDM_ZOOM_IN && cax->m_ln.front()->sig.GetType()==CSIG_VECTOR)
			{
				double percentShown = 1. - ( (cax->xlim[0]-cax->xlimFull[0]) + (cax->xlimFull[1]-cax->xlim[1]) ) / (cax->xlimFull[1]-cax->xlimFull[0]);
				if ((len = (int)(percentShown * cax->m_ln.front()->sig.nSamples+.5))<=3)
					return; // no unzoom if full length
			}
			deltaxlim = cax->xlim[1]-cax->xlim[0];
			if (nID == IDM_ZOOM_IN)
				cax->xlim[0] += deltaxlim/4, 	cax->xlim[1] -= deltaxlim/4;
			else
			{
				cax->xlim[0] -= deltaxlim/2;
				cax->xlim[0] = MAX(cax->xlimFull[0], cax->xlim[0]);
				cax->xlim[1] += deltaxlim/2;
				cax->xlim[1] = MIN(cax->xlim[1], cax->xlimFull[1]);
			}
			if ( cax->m_ln.front()->sig.GetType()==CSIG_AUDIO)
			{
				if ((cax->xlim[1]-cax->xlim[0])<=0.019)		strcpy(cax->xtick.format, "%.4f"); 
				else										strcpy(cax->xtick.format, ""); // Clear to use sprintfFloat with 3 for default in DrawTicks
				cax->xtick.tics1 = makefixtick(cax->xlim[0], cax->xlim[1], cax->GetDivCount('x', -1));
			}
			else
				cax->setxticks();
			cax->ytick.tics1 = makefixtick(cax->ylim[0], cax->ylim[1], cax->GetDivCount('y', -1));
			if (paxFFT = (CAxis*)cax->hChild) ShowSpectrum(paxFFT, cax);

		}
		ShowStatusBar();
		InvalidateRect(NULL);								
		return;
	case IDM_LEFT_STEP:
		iMul *= -1;
	case IDM_RIGHT_STEP:
		for (vector<CAxis*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
		{
			cax = *axt;
			shift = (cax->xlim[1]-cax->xlim[0]) / 4;
			newlimit1 = cax->xlim[0] + shift*iMul; // only effective for IDM_LEFT_STEP
			if (newlimit1<cax->xlimFull[0]) 
				shift = cax->xlim[0] - cax->xlimFull[0];
			newlimit2 = cax->xlim[1] + shift*iMul; // only effective for IDM_RIGHT_STEP
			if (newlimit2>cax->xlimFull[1]) 
				shift = cax->xlim[1] - cax->xlimFull[1];
			if (shift<0.001) return;
			cax->xlim[0] += shift*iMul;
			cax->xlim[1] += shift*iMul;
			// further adjusting lim[0] to xlimFull[0] (i.e., 0 for audio signals) is necessary to avoid in makefixtick when re-generating xticks
	//		if ((cax->xlim[0]-cax->xlimFull[0])<1.e-6) 	cax->xlim[0] = cax->xlimFull[0];
	//		if ((cax->xlimFull[1]-cax->xlim[1])<1.e-6) 	cax->xlim[1] = cax->xlimFull[1]; // this one may not be necessary.
			if (nID == IDM_RIGHT_STEP)		cax->xtick.extend(true, cax->xlim[1]);
			else							cax->xtick.extend(false, cax->xlim[0]);
			//Assuming that the range is determined by the first line
			rt = cax->axRect;
			rt.InflateRect(-10,0,10,30);

			sbinfo.xBegin = cax->xlim[0];
			sbinfo.xEnd = cax->xlim[1];
			ShowStatusBar();
			InvalidateRect(&rt, FALSE);
			if (paxFFT = (CAxis*)cax->hChild) ShowSpectrum(paxFFT, cax);
		}
		InvalidateRect(NULL);								
		return;

	case IDM_ZOOMSELECT:
		if (curRange != NO_SELECTION)
		{ // need jst, because the moment you update cax->xlim[], you can't call pix2timepoint it any more.
			double jst = cax->pix2timepoint(curRange.px1);
			cax->xlim[1] = cax->pix2timepoint(curRange.px2);
			cax->xlim[0] = jst;
			if ( cax->m_ln.front()->sig.GetType()==CSIG_AUDIO)
			{
				if ((cax->xlim[1]-cax->xlim[0])<=0.019)		strcpy(cax->xtick.format, "%.4f"); 
				else										strcpy(cax->xtick.format, "%.3f"); 
				cax->xtick.tics1 = makefixtick(cax->xlim[0], cax->xlim[1], cax->GetDivCount('x', -1));
			}
			else
				cax->setxticks();
			cax->ytick.tics1 = makefixtick(cax->ylim[0], cax->ylim[1], cax->GetDivCount('y', -1));
			InvalidateRect(NULL);
			if (gcf.ax.front()->hChild) OnMenu(IDM_SPECTRUM_INTERNAL); // do this later.
		}
		curRange.reset();
		::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
		return;

	case IDM_PLAY:
#ifndef NO_PLAYSND
		if (curRange == NO_SELECTION)
		{
			deltapixel = cax->axRect.right - cax->axRect.left;
			deltaxlim = cax->xlim[1]-cax->xlim[0];
		}
		else
		{
			deltapixel = curRange.px2 - curRange.px1;
			deltaxlim = selRange.tp2 - selRange.tp1;
		}
		block = deltaxlim * PBPROG_ADVANCE_PIXELS / deltapixel * 1000.;
		// Below, if this put too low maximum, the progress line may move smoothly, but the playback sound will be choppy.
		block = max(50, block);
		_sig = cax->m_ln.front()->sig;
		if (_sig.GetType()!=CSIG_AUDIO) return;
		for (vector<CAxis*>::iterator axt=gcf.ax.begin(); axt!=gcf.ax.end(); axt++) 
		{
			cax = *axt;
			if (GetCSignalsInRange(1, cax, _sig, 1))
			{
				if (_sig2play.IsEmpty()) _sig2play = _sig;
				else
					_sig2play.SetNextChan(&_sig);
			}
		}
		_sig2play.tmark=0; // why is this necessary?
		_sig2play.PlayArray(devID, WM__SOUND_EVENT, hDlg, &block, errstr);
		playing = true;
		return;
	case IDM_STOP:
		playing = false;
		TerminatePlay();
		playLoc = -1;
		OnSoundEvent(-1,0);
#endif
		return;
	case IDM_SPECTRUM:
		if (cax->m_ln.front()->sig.GetType()!=CSIG_AUDIO) break;
		// at this point, sbinfo.vax can be used
		if (gcf.ax.size()>1) SpecAxPos.height -= .02;
		for (size_t k=0; k<gcf.ax.size(); k++)
		{
			cax = gcf.ax[k];
			if (!cax->hChild)
			{
				
				if (k>0) //if second channel (stereo)
				{ SpecAxPos.y0 = .1; }
				cax->hChild = paxFFT = cax->create_linked_axis(SpecAxPos);
				paxFFT->color = RGB(220, 220, 150);
				dfs = (double)cax->m_ln.front()->sig.GetFs();
				paxFFT->xlim[0]=0;  paxFFT->xlim[1]=dfs/2;
				paxFFT->visible = true;
				ShowSpectrum(paxFFT,cax);
			}
			else
			{
				CRect rt, rt2;
				GetClientRect(hDlg, &rt);
				paxFFT = (CAxis*)cax->hChild;
				rt2=paxFFT->pos.GetRect(rt);
				rt2 =paxFFT->axRect;
				rt2.InflateRect(40,30);
//				rt2.bottom += 30; rt2.left -= 40;
				InvalidateRect(rt2);
				delete paxFFT; // do not call deleteObj(paxFFT); --> that is to clean up vectors and assumes CAxis object is a child (vector) of CFigure
				cax->hChild=NULL;
			}
		}
		return;
	case IDM_SPECTRUM_INTERNAL:
		return;
	case IDM_WAVWRITE:
#ifndef NO_PLAYSND
		fullfname[0]=0;
		fileDlg.InitFileDlg(hDlg, hInst, "");
		_sig = cax->m_ln.front()->sig;
		if (_sig.GetType()!=CSIG_AUDIO) return;
		if (!GetCSignalsInRange(1, cax, _sig, 1)) break;
		if (fileDlg.FileSaveDlg(fullfname, fname, "Wav file (*.WAV)\0*.wav\0", "wav"))
		{
			if (!_sig.Wavwrite(fullfname, errstr))	MessageBox (errstr);
		}
		else
		{
			if (GetLastError()!=0) GetLastErrorStr(errstr), MessageBox (errstr, "Filesave dialog box error");
		}
#endif
		return;
	}
	InvalidateRect(NULL);
}

void CPlotDlg::ShowSpectrum(CAxis *pax, CAxis *paxBase)
{
	bool stereo;
	int len;
	fftw_plan plan;
	double *freq, *fft, *mag, maxmag, dfs;
	CSignals _sig;
	double lastxlim[2];
	CTick lastxtick;

#ifndef NO_FFTW
	stereo = pax->m_ln.size()>1 ? true : false;
	if ( gcf.ax.size()==0) return;

	// It gets Chainless inside GetSignalofInterest in this call
	dfs = (double)paxBase->m_ln.front()->sig.GetFs();
	_sig = paxBase->m_ln.front()->sig;
	if (!GetCSignalsInRange(1, paxBase, _sig, 1)) return;
	if (pax->m_ln.empty()) lastxlim[0]=1.,lastxlim[1]=-1.;
	for (; pax->m_ln.size()>0;)	
	{
		lastxtick = pax->xtick;
		memcpy((void*)lastxlim, (void*)pax->xlim, sizeof(pax->xlim));
		deleteObj(pax->m_ln[0]);
	}
	if ((len=_sig.nSamples)<20) 
	{
		CClientDC dc(hDlg);
		CPen *dotted = new CPen;
		COLORREF dd = RGB(255, 0, 0);
		dotted->CreatePen(PS_DOT, 1, dd);
		dc.SelectObject(dotted);
		dc.TextOut(pax->axRect.left, (pax->axRect.bottom-pax->axRect.top)/2, "Selection too short");
		delete[] dotted;
		return;
	}
	freq = new double[len];
	fft = new double[len];
	mag = new double[len/2+1];
	int lenFFT = (len+1)/2;
	for (int k=0; k<len; k++)		freq[k]=(double)k/(double)len*dfs;
	plan = fftw_plan_r2r_1d(len, _sig.buf, fft, FFTW_R2HC, FFTW_ESTIMATE);
	if (fabs(_sig.Max()[0]-_sig.Min()[0])>1.e-5) 
	{
		fftw_execute (plan);
		mag[0] = 20.*log10(fft[0]*fft[0]);
		
		for (int k = 1; k < (len+1)/2; ++k)  /* (k < N/2 rounded up) */
			mag[k] = 20.*log10(fft[k]*fft[k] + fft[len-k]*fft[len-k]);
		if (len % 2 == 0) /* N is even */
			mag[len/2] = 20.*log10(fft[len/2]*fft[len/2]);  /* Nyquist freq. */
		maxmag = getMax(lenFFT,mag);
		for (int j=0; j<lenFFT; j++)	mag[j] -= maxmag;
	}
	else
		for (int k = 0; k < lenFFT; ++k) mag[k] = 0.;
	fftw_destroy_plan(plan);
	PlotDouble(pax, lenFFT, freq, mag);
	//FILE *fpfp=fopen("fftresult.txt","wt");
	//fprintf(fpfp, "+sig begins with %d\n",curRange.px1);
	//for (int k=0; k<10; k++)
	//	fprintf(fpfp, "%g\t",_sig.buf[k]);
	//fprintf(fpfp,"\n");
	//for (int k=0; k<lenFFT; k++)
	//	fprintf(fpfp, "%d\t%5.5f\n",k,mag[k]);
	//fclose(fpfp);
	strcpy(pax->xtick.format,"%.2gk"); // pax->xtick.format is called in anticipation of drawticks. i.e., format is used in drawticks
	if (lastxlim[0]>lastxlim[1])
	{
		pax->xtick.tics1 = makefixtick(pax->xlim[0], pax->xlim[1], pax->GetDivCount('x', -1));
		pax->xtick.mult = 0.001;		
	}
	else
	{
		pax->xtick = lastxtick;
		memcpy((void*)pax->xlim, (void*)lastxlim, sizeof(pax->xlim));
	}
	pax->ylim[0]=-110; pax->ylim[1] = 0;
	pax->ytick.tics1 = makefixtick(pax->ylim[0], pax->ylim[1], pax->GetDivCount('y', -1));
	pax->m_ln.front()->color = gcf.ax.front()->m_ln.front()->color;
	delete[] freq;
	delete[] fft;
	delete[] mag;
	CRect rt;
	GetClientRect(hDlg, &rt);
	pax->axRect = pax->pos.GetRect(rt);
	InvalidateRect(pax->axRect);
#endif

}

CAxis * CPlotDlg::CurrentPoint2CurrentAxis(CPoint *point)
{
	for (int k((int)gcf.ax.size()-1); k>=0; k--) //reason for decreasing: when the spectrum axis is clicked, that should be gca even if that overlaps with signal axis
	{
		if (IsInsideRect(gcf.ax[k]->axRect, point))
		{
			gca = gcf.ax[k];
			return gcf.ax[k];
		}
	}
	return NULL;
}

int CPlotDlg::GetSignalofInterest(int code, CSignal &out, bool chainless)
{ //code=0 means Get the signal in the viewing area
  //code=1 means Get the signal in the selected area
	// out is an CSignal object (not CSignals) to work with (for stereo, put left and right in two separate calls)
	//return 0 if the range is zero (if ind1==ind2 or xlim is not initialized yet)
	int ind1, ind2;
	int fs = out.GetFs();
	CAxis *pax(gcf.ax.front());  // Following the convention
	if (pax->xlim[0]>=pax->xlim[1]) return 0;
 	GetSignalIndicesofInterest(code, ind1, ind2);
	if (ind1==ind2) return 0;
	if (!out.chain) //if there's no chain, skip all the dirty routines involving checking and handling null durations
		out.Truncate(ind1, ind2, code);
	else
	{
		vector<double> tmarks;
		CSignal old(out), dummy;
		CSignal *extracted;
		bool loop(true);
		while( (extracted = old.ExtractDeepestChain(&dummy))!=&old || loop )
		{
			tmarks.push_back(extracted->tmark + extracted->dur());
			tmarks.push_back(extracted->tmark);
			if (extracted == &old) loop = false;
		}
		out.MakeChainless();
		if (chainless)
		{
			out.Truncate(ind1, ind2, code);
			out.tmark = (double)ind1/out.GetFs()*1000.;
		}
		else
		{
			//MC is MakeChains version 2
			dummy.Reset();
			out.MC(dummy, tmarks, ind1, ind2);
			out = dummy;
		}
	}
	return 1;
}

void CPlotDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	char buf1[64], buf2[64], buf3[64];
	switch(nChar)
	{
	case VK_CONTROL:
		if (ClickOn) ClickOn=0, OnLButtonUp(0, CPoint(-1,-1));
		if (playing) OnMenu(IDM_STOP);
		if (axis_expanding)
			strcpy(buf1,""), strcpy(buf2,""), strcpy(buf3,""); 
		else
			strcpy(buf1,"Adjust"), strcpy(buf2,"Window Size with"), strcpy(buf3,"Mouse"); 
		axis_expanding = !axis_expanding;
		::SendMessage (hStatusbar, SB_SETTEXT, 7, (LPARAM)buf1);
		::SendMessage (hStatusbar, SB_SETTEXT, 8, (LPARAM)buf2);
		::SendMessage (hStatusbar, SB_SETTEXT, 10, (LPARAM)buf3);
		InvalidateRect(NULL);
		break;
	}
}

void CPlotDlg::UpdateRects(CAxis *ax)
{
	if (ax==NULL) return;
	CRect rt0(ax->axRect);

	roct[0] = CRect(rt0.left, gcmp.y-2, gcmp.x+2,gcmp.y+2);
	roct[1] = CRect(gcmp.x-2, gcmp.y-2, gcmp.x+2, rt0.bottom);
	roct[2] = CRect(CPoint(gcmp.x-40,rt0.bottom+6), CSize(80,18));
	roct[3] = CRect(CPoint(rt0.left,gcmp.y-20), CSize(40,18));
	roct[4] = CRect(CPoint(gcmp.x-50,gcmp.y-40), CSize(100,40));
}



unsigned short CPlotDlg::GetMousePos(CPoint pt)
{ // revise to accommodate stereo
	unsigned short res(0);
	static int count(0);
	if (gcf.ax.empty()) return 0;
	//process only the first two axes
	CAxis* paxFFT2, *paxFFT = (CAxis*)gcf.ax.front()->hChild;
	switch(gcf.ax.size())
	{
	case 1: // mono
		if (paxFFT)
			return MAKEWORD(GetMousePosAxis(pt, gcf.ax.front()),  GetMousePosAxis(pt, paxFFT)); 
		else
			return MAKEWORD(GetMousePosAxis(pt, gcf.ax.front()),  0); 
	case 2: //stereo
		paxFFT2 = (CAxis*)gcf.ax.back()->hChild;
		if (paxFFT2)
		{
			unsigned char a = GetMousePosAxis(pt, gcf.ax.front());
			unsigned char b = GetMousePosAxis(pt, gcf.ax.back());
			unsigned char c = GetMousePosAxis(pt, paxFFT);
			unsigned char d = GetMousePosAxis(pt, paxFFT2);
			return MAKEWORD(a|b,c|d); 
		}
		else
		{
			unsigned char a = GetMousePosAxis(pt, gcf.ax.front());
			unsigned char b = GetMousePosAxis(pt, gcf.ax.back());
			return MAKEWORD(a|b,0); 
		}
	}
	return 0;
}

void CPlotDlg::OnActivate(UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	hPlotDlgCurrent = hDlg;
	//hQuickSolidBrush = new CBrush[gca->nLines];
	//for (int i=0; i<gca->nLines;i++)
	//	hQuickSolidBrush[i].CreateSolidBrush (gca->m_ln[i]->color);

	HandleLostFocus(WM_ACTIVATE);
}

BOOL CPlotDlg::OnNCActivate(UINT state)
{
	HandleLostFocus(WM_NCACTIVATE);

	if (!state) return TRUE;
	else		return FALSE;

}

void CPlotDlg::MouseLeave(UINT umsg)
{

}

void CPlotDlg::HandleLostFocus(UINT umsg, LPARAM lParam)
{
	CRect rt0, rt;
	CAxis *pax;
	char buf[128];
	GetClientRect(hDlg, &rt0);
	//FILE *fp = fopen("WMlog.txt","at");
 //   SYSTEMTIME lt;
 //   GetLocalTime(&lt);	
	if (ClickOn)
	{
		pax = gcf.ax.front();
		rt.top = pax->axRect.top;
		rt.bottom = pax->axRect.bottom+1;
		if (pax->axRect.right - curRange.px2 < curRange.px1 - pax->axRect.left) // toward right
		{
			rt.left = curRange.px2-1;
			rt.right = curRange.px2 = pax->axRect.right;
			sprintf(buf,"%.3f",pax->pix2timepoint(curRange.px2));
			::SendMessage (hStatusbar, SB_SETTEXT, 7, (LPARAM)buf);
		}
		else
		{
			rt.right = curRange.px1+1;
			rt.left = curRange.px1 = pax->axRect.left;
			sprintf(buf,"%.3f",pax->pix2timepoint(curRange.px1));
			::SendMessage (hStatusbar, SB_SETTEXT, 6, (LPARAM)buf);
		}
		InvalidateRect(&rt);
		//fprintf(fp,"[%02d:%02d:%02d:%02d] msg=%x, ClickOn=%d\n", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, umsg, ClickOn);
	}
	ClickOn=0;
    //GetLocalTime(&lt);	
	//if (lParam!=0) fprintf(fp,"NCHITTEST res = %d  ", (LRESULT)lParam);
	//fprintf(fp,"[%02d:%02d:%02d:%02d] msg=%x, ClickOn=%d\n", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds, umsg, ClickOn);
	//fclose(fp);
}

void CPlotDlg::ShowStatusBar(SHOWSTATUS status, const char* msg)
{
	char buf[64]={}, buf2[64];

	if (status==CURSOR_LOCATION)
	{
		if (gcf.ax.front()->m_ln[0]->sig.GetType()==CSIG_AUDIO)
			sprintf(buf, "(%.3fs,%.3f)", sbinfo.xCur, sbinfo.yCur);
		else
			sprintf(buf, "(%.3f,%.3f)", sbinfo.xCur, sbinfo.yCur);
		::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)buf);
		return;
	}
	else if (status==CURSOR_LOCATION_SPECTRUM)
	{
		::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)msg);
		return;
	}

	char rmstext[64]={};
	CSignals _sig;
	bool b;
	size_t nAxes = sbinfo.vax.size();
	for (size_t k=0;k<nAxes; k++)
	{
		if (buf[0]) sprintf(rmstext, "(L)%s", buf);
		b = curRange!=NO_SELECTION;
		if (GetCSignalsInRange(b, sbinfo.vax[k], _sig, 0)) // if no selection, get sig in the screen viewing range; otherwise, _sig is in the selected range
		{
			if (_sig.IsLogical())	strcpy(buf, "logical");
			else if (_sig.GetType()==CSIG_AUDIO)
			{	RMSDB(buf,"-Inf dB","%.1f dB",_sig.RMS())	}
			if (rmstext[0]) {	sprintf(buf2, "(R)%s", buf);	strcat(rmstext, buf2); }
		}
	}
	if (!rmstext[0])	strcpy(rmstext, buf);
	strcat(rmstext, " RMS");
	::SendMessage (hStatusbar, SB_SETTEXT, 6, (LPARAM)rmstext);

	//From OnPaint()---maybe use predefined format for non-audio???
//	char *format = pax0->xtick.format[0]? pax0->xtick.format : "%.2f";
//	sprintf(buf, format, pax0->xlim[0]);
//	sprintf(buf2, format, pax0->xlim[1]);

	if (sbinfo.vax.size()==0)
		{MessageBox("sbinfo.vax.size()==0"); return;}
	
	sbinfo.xBegin = sbinfo.vax[0]->xlim[0];
	sbinfo.xEnd = sbinfo.vax[0]->xlim[1];
	sprintf(buf, "%.3f", sbinfo.xBegin); 
	sprintf(buf2, "%.3f", sbinfo.xEnd);
//	sprintf(buf3, "%.3f", sbinfo.xCur);
	if (_sig.GetType()==CSIG_AUDIO) // for now, assume that two axes have both same sig type (check only the last one)
	{	strcat(buf, "s"); strcat(buf2, "s");   }
	::SendMessage (hStatusbar, SB_SETTEXT, 0, (LPARAM)buf);
	::SendMessage (hStatusbar, SB_SETTEXT, 1, (LPARAM)buf2);
//	sprintf(buf, "(%s,%.3f)", buf3, sbinfo.yCur);
//	::SendMessage (hStatusbar, SB_SETTEXT, 2, (LPARAM)buf);
	if (curRange==NO_SELECTION)
	{buf[0]=0; buf2[0]=0;}
	else
	{ // for now, use the last one---assume that two axes have both same sig type 
		sprintf(buf, "%.3f", sbinfo.xSelBegin); 
		sprintf(buf2, "%.3f", sbinfo.xSelEnd);
		if (_sig.GetType()==CSIG_AUDIO) // for now, assume that two axes have both same sig type (check only the last one)
		{	strcat(buf, "s"); strcat(buf2, "s");  }
	}
	::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
	::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)buf2);
}

void CPlotDlg::ShowStatusSelectionOfRange(CAxis *pax, const char *swich)
{
	if (swich!=NULL && !strcmp(swich,"off")) 
	{
		::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)"");
		::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)"");
	}
	else
	{
		char buf[64], buf2[64];
		//if (curRange != NO_SELECTION)
		//{
		//	CSignals _sig = pax->m_ln[0]->sig;
		//	if (_sig.GetType()==CSIG_AUDIO)
		//	{
		//		if (_sig.IsLogical())	strcpy(buf, "logical");
		//		else if (GetCSignalsInRange(1, pax, _sig, 0))
		//		{
		//			RMSDB(buf,"-Inf dB","%.1f dB",_sig.RMS())
		//			sprintf(RMSselected,"%s RMS", buf);
		//		}
		//		sprintf(buf,"%.3fs",pax->pix2timepoint(curRange.px1));
		//		sprintf(buf2,"%.3fs",pax->pix2timepoint(curRange.px2));
		//	}
		//	else
		//	{
		//		sprintf(buf,"%.2f",pax->pix2timepoint(curRange.px1));
		//		sprintf(buf2,"%.2f",pax->pix2timepoint(curRange.px2));
		//	}
		//	::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
		//	::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)buf2);
		//}


		if (curRange != NO_SELECTION)
		{
			CSignals _sig = pax->m_ln[0]->sig;
//			if (_sig.GetType()==CSIG_AUDIO)
//			{
				if (_sig.IsLogical())	strcpy(buf, "logical");
				else if (GetCSignalsInRange(1, pax, _sig, 0))
				{
					if (gcf.ax.size()==1)
					{
						RMSDB(RMSselected,"-Inf dB","%.1f dB monoRMS",_sig.RMS())
					}
					else // just assume that only stereo (2 channels)
					{
						char LR[3];
						strcpy(LR,"LR");
						RMSselected[0]=0;
						for (size_t k=0; k<gcf.ax.size(); k++)
						{
							GetCSignalsInRange(1, gcf.ax[k], _sig, 0);
							if (_sig.nSamples==0) strcpy(buf,"???");
							else RMSDB(buf,"-Inf dB","%.1f dB RMS",_sig.RMS())
							sprintf(buf2,"(%c)%s", LR[k], buf);
							strcat(RMSselected,buf2);
						}
					}
//					::SendMessage (hStatusbar, SB_SETTEXT, 6, (LPARAM)buf2);

				}
				sprintf(buf,"%.3fs",pax->pix2timepoint(curRange.px1));
				sprintf(buf2,"%.3fs",pax->pix2timepoint(curRange.px2));
//			}
//			else
//			{
//				sprintf(buf,"%.2f",pax->pix2timepoint(curRange.px1));
//				sprintf(buf2,"%.2f",pax->pix2timepoint(curRange.px2));
//			}
			::SendMessage (hStatusbar, SB_SETTEXT, 4, (LPARAM)buf);
			::SendMessage (hStatusbar, SB_SETTEXT, 5, (LPARAM)buf2);
		}

	}
}