//#pragma once
#include "FileDlg.h"

CFileDlg::CFileDlg()
{
     ofn.lStructSize       = sizeof (OPENFILENAME) ;
     ofn.hInstance        = NULL ;
	 ofn.lpfnHook = NULL ;
	ofn.lpTemplateName =  ofn.lpstrCustomFilter = NULL ;
     ofn.nMaxCustFilter    = ofn.nFilterIndex
		 = ofn.Flags 
		 = ofn.nFileOffset
		 = ofn.nFileExtension = 0;
	 ofn.lCustData =  0L ;
     ofn.nMaxFile          = _MAX_PATH ;
     ofn.nMaxFileTitle     = _MAX_FNAME + _MAX_EXT ;
	 ofn.lpstrInitialDir   = InitDir ;
	 ofn.hwndOwner = NULL;
	 LastPath[0]='\0';
}

void CFileDlg::InitFileDlg (HWND hwnd, HINSTANCE hInst, const char *initDir)
{
	if (ofn.hwndOwner==NULL)
	{
		ofn.hwndOwner         = hwnd ;
		if (initDir!=NULL)     strcpy(InitDir, initDir);
	}
	ofn.lpstrTitle = NULL;
	ofn.hInstance = hInst;
}
// pstrFileName should be static variable.
int CFileDlg::FileOpenDlg (LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt)
{
	char drive[64], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
	ofn.lpstrFile         = pstrFileName ; // must be NULLi or containt file name to initialize the File Name edit control.
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrDefExt = lpstrDefExt;
	ofn.Flags             = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	BOOL res = GetOpenFileName (&ofn) ;
	if (res)
	{
		_splitpath(pstrFileName, drive, dir, fname, ext);
		_makepath(LastPath, drive, dir, "", "");
	}
	return res;
}
// pstrFileName should be static variable.
int CFileDlg::FileSaveDlg (LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt)
{
	char drive[64], dir[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];
	ofn.lpstrFile         = pstrFileName ; // must be NULLi or containt file name to initialize the File Name edit control.
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrDefExt = lpstrDefExt;

	BOOL res = GetSaveFileName (&ofn) ;
	if (res)
	{
		_splitpath(pstrFileName, drive, dir, fname, ext);
		_makepath(LastPath, drive, dir, "", "");
	}
	return res;
}