#include "resource.h"		// main symbols
#include "sigproc.h"	
#include "supportFcn.h"
#include "graffy.h"



class CGraffyDLL : public CWinApp 
{
public:
	CGobj GraffyRoot;
	CPlotDlg **fig;
	int closeFigure(int figId);
	vector<HANDLE> figures();
	HANDLE  openChildFigure(CRect *rt, const CSignals &data, HWND hParent=NULL);
	HANDLE  findFigure(const CStdString& caption, int *nFigs=NULL);
	HANDLE  openFigure(CRect *rt, const CStdString& caption, const CSignals &data, int devID, double blocksize);
	HANDLE  openFigure(CRect *rt, const char* caption, const CSignals &data, int devID, double blocksize);
	HANDLE  openFigure(CRect *rt, const CSignals &data, int devID, double blocksize);
	int nFigures;
	CGraffyDLL();
	virtual ~CGraffyDLL();
};	

