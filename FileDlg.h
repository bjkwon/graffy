#pragma once
#include "windows.h"
#define ACESET_API __declspec(dllexport)

class CFileDlg
{
public:
	OPENFILENAME ofn;
	ACESET_API void InitFileDlg (HWND hwnd, HINSTANCE hInst, const char *initDir);
	ACESET_API int FileOpenDlg (LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt);
	ACESET_API int FileSaveDlg (LPSTR pstrFileName, LPSTR pstrTitleName, LPSTR szFilter, LPSTR lpstrDefExt);
	ACESET_API CFileDlg(void);
	char LastPath[MAX_PATH];
	char InitDir[MAX_PATH];
};
