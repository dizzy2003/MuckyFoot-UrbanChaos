// UnInst.c
//
// uninstall thingy

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>

#include "resource.h"

HANDLE	hInstance;
static void DeleteThings();

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	hInstance = hModule;
    return TRUE;
}

LONG APIENTRY UninstInitialize(HWND hwndDlg,HANDLE hTheirInstance,LONG lRes)
{
	char title[MAX_PATH], question[MAX_PATH];
	int value;

	LoadString(hInstance, IDS_REMOVE,   title,    MAX_PATH);
	LoadString(hInstance, IDS_QUESTION, question, MAX_PATH);
	
	value = MessageBox(hwndDlg, question, title, MB_YESNO | MB_ICONQUESTION);

	if (value == IDYES)
	{
		DeleteThings();
	}

	return 0;
}

LONG APIENTRY UninstUnInitialize(HWND hwndDlg,HANDLE hInstance,LONG lRes)
{
	return 0;
}

static void ReportError(const TCHAR* error)	{ MessageBox(NULL, error, "Error", MB_OK | MB_ICONINFORMATION); }

static void ReportLastError()
{
#if 0
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS, 
					NULL, GetLastError(), 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR)&lpMsgBuf, 0, NULL);

	ReportError((LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
#endif
	return;
}

static void DeleteThings()
{
	HKEY				hRegKey;
	TCHAR				szAppPath[MAX_PATH];
	TCHAR				szBuffer[MAX_PATH];
	WIN32_FIND_DATA		found;
	HANDLE				hFile;

	// load registry key
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\fallen.exe", 0, KEY_READ, &hRegKey))
	{
		DWORD	len = MAX_PATH;

		RegQueryValueEx(hRegKey, "Path", NULL, NULL, (unsigned char*)szAppPath, &len);

		RegCloseKey(hRegKey);
	}
	else
	{
		ReportError("Cannot locate registry key");
		return;
	}

	// delete the ini file
	sprintf(szBuffer, "%s\\config.ini", szAppPath);

	if (!DeleteFile(szBuffer))	ReportLastError();

	// delete the save game files
	sprintf(szBuffer, "%s\\saves\\*.*", szAppPath);

	hFile = FindFirstFile(szBuffer, &found);

	do
	{
		if (strcmp(found.cFileName, ".") && strcmp(found.cFileName, ".."))
		{
			sprintf(szBuffer, "%s\\saves\\%s", szAppPath, found.cFileName);
			if (!DeleteFile(szBuffer))	ReportLastError();
		}

	} while (FindNextFile(hFile, &found));

	// remove the saves directory
	sprintf(szBuffer, "%s\\saves", szAppPath);
	RemoveDirectory(szBuffer);
}
