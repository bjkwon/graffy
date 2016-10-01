#include "graffy.h"

CText::CText(CWndDlg * base, CGobj* pParent, const char *strInit, CPosition posInit)
:fontsize(12), italic(false), bold(false), underline(false), strikeout(false), alignmode(TA_LEFT|TA_BASELINE)
{
	m_dlg = base;
	type = GRAFFY_text;
	strcpy(fontname,"Arial");
	str.clear();
	str = strInit;
	pos = posInit;
	hPar = pParent;
	color = RGB(0, 0, 0);
    HDC hdc = GetDC(m_dlg->hDlg);
	int nHeight = MulDiv(fontsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(m_dlg->hDlg, hdc);
	int weight = bold ? FW_BOLD : FW_DONTCARE;
	DWORD dwItalic = italic ? 1 : 0;
	DWORD dwUnderline = underline ? 1 : 0;
	DWORD dwStrikeOut = strikeout ? 1 : 0;
	font.CreateFont(nHeight, 0, 0, 0, weight, dwItalic, dwUnderline, dwStrikeOut, ANSI_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, fontname);
	hPar->child.push_back(this);
}

CText::~CText() 
{
	hPar->child.pop_back();
	font.DeleteObject();
}

GRAPHY_EXPORT HFONT CText::ChangeFont(LPCTSTR fontName, int fontSize, DWORD style)
{
	font.DeleteObject();
	char buf[64];
	if (strlen(fontName)==0)	strcpy(buf, fontname);
	else						strcpy(buf, fontName);
    HDC hdc = GetDC(m_dlg->hDlg);
	int nHeight = MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(m_dlg->hDlg, hdc);
	int weight = (style & FONT_STYLE_BOLD) ? FW_BOLD : FW_DONTCARE;
	DWORD dwItalic = (style & FONT_STYLE_ITALIC) ? 1 : 0;
	DWORD dwUnderline = (style & FONT_STYLE_UNDERLINE) ? 1 : 0;
	DWORD dwStrikeOut = (style & FONT_STYLE_STRIKEOUT) ? 1 : 0;
	HFONT res = font.CreateFont(nHeight, 0, 0, 0, weight, dwItalic, dwUnderline, dwStrikeOut, ANSI_CHARSET, 
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, buf);
	if (res!=NULL)
	{
		strcpy(fontname, buf);
		fontsize = fontSize;
	}
	return res;
}

GRAPHY_EXPORT int CText::GetAlignment(string &horizontal, string &vertical)
{
	horizontal.clear();
	vertical.clear();
	unsigned _int16 horr = alignmode & 6;
	unsigned _int16 vert = alignmode & (6<<2);
	switch(horr)
	{
	case TA_LEFT:
		horizontal = "left";
		break;
	case TA_CENTER:
		horizontal = "center";
		break;
	case TA_RIGHT:
		horizontal = "right";
		break;
	}
	switch(vert)
	{
	case TA_BASELINE:
		vertical = "baseline";
		break;
	case TA_BOTTOM:
		vertical = "bottom";
		break;
	case TA_TOP:
		vertical = "top";
		break;
	}
	return (horizontal.size()>0) + (vertical.size()>0) ;
}

GRAPHY_EXPORT int CText::SetAlignment(const char *alignmodestr)
{
	unsigned _int16 horr = alignmode & 6;
	unsigned _int16 vert = alignmode & (6<<2);
	bool done(false);
	if (!stricmp(alignmodestr, "left")) done=true, horr = TA_LEFT;
	else if (!stricmp(alignmodestr, "center")) done=true, horr = TA_CENTER;
	else if (!stricmp(alignmodestr, "right")) done=true, horr = TA_RIGHT;
	else if (!stricmp(alignmodestr, "baseline")) done=true, vert = TA_BASELINE;
	else if (!stricmp(alignmodestr, "bottom")) done=true, vert = TA_BOTTOM;
	else if (!stricmp(alignmodestr, "top")) done=true, vert = TA_TOP;
	return done ? (alignmode = horr + vert) : 0;
}