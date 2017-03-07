#include "graffy.h"

CTick::CTick(CWndDlg * base)
:automatic(true), mult(1.), offset(0.), labelPos(7), size(4)
{
	m_dlg = base;
	//font.CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
	//	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	type = GRAFFY_tick;
	gap4next.x = 5;
	gap4next.y = 2;
	format[0]='\0';
}

CTick::~CTick()
{
//	font.DeleteObject(); 
}


void CTick::set(vector<int> val, vector<double> xydata, int len)
{
	int k(0);
	tics1.clear();
	tics1.push_back(xydata[k]);
	for (vector<int>::iterator it=val.begin(); it!=val.end(); it++)
	{
		k += *it;
		if (k>=len)
		{
			vector<double> backuptics1(tics1);
			double diff = tics1.back();
			tics1.pop_back();
			diff -= tics1.back();
			tics1 = backuptics1;
			tics1.push_back(tics1.back()+diff);
		}
		else if (*it!=0)
			tics1.push_back(xydata[k]);
	}
}

void CTick::extend(bool direction, double lim)
{
	double step;
	CAxis *ax = (CAxis *)hPar;
	if (tics1.size()<3) return;
	else if (tics1.size()==3)
		step = (tics1[2]-tics1[0])/2;
	else
		step = tics1[2]-tics1[1];
	if (direction) // right or up
	{
		tics1.pop_back();
		do {
			if (tics1.back()<lim)
				tics1.push_back(tics1.back()+step);
		} while (tics1.back()<lim);
	}
	else
	{
		tics1.erase(tics1.begin());
		do {
			if (tics1.front()>lim)
				tics1.insert(tics1.begin(), tics1.front()-step);
		} while (tics1.front()>lim);
		tics1.front() = max(tics1.front(), lim);
	}
}
