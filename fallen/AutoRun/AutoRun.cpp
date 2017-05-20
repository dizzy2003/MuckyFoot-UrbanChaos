// AutoRun.cpp
//
// Defines the entry point for the application

#include "AutoRun.h"

// defines

#define	WINCLASS	"AUTORUN"

// globals

HINSTANCE	hInst;					// current instance
TCHAR		szLanguage[MAX_PATH];	// the user's language
TCHAR		szAutorunDir[MAX_PATH];	// the autorun directory
Director*	pDirector;				// the director structure
HCURSOR		hNormalCursor;			// the pointer
HCURSOR		hSelectCursor;			// the hand cursor

Menu*		pMenu;					// current menu
Menu*		pLastMenu;				// last menu
MenuItem*	pMenuItem;				// current item
HBITMAP		hBackground;			// background bitmap
BITMAP		bmBackground;			// bitmap info
HFONT		hMenuFont;				// menu font

// statics

static ATOM				MyRegisterClass(HINSTANCE hInstance);
static void				GetLanguageData(void);
static Director*		LoadMenuData(void);
static BOOL				InitInstance(HINSTANCE, int);
static void				OpenMenu(HWND hWnd, Menu* menu);
static void				CloseMenu();
static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
static void				OnPaint(HWND hWnd);
static void				OnMouseMove(HWND hWnd, int x, int y);
static void				OnLButtonDown(HWND hWnd, int x, int y);
static bool				MacroReplace(TCHAR* cmd, TCHAR* buffer, UINT blen);

// WinMain
//
// entry point

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// try and create an event
	// this always succeeds
	// if event existed before (still succeeds) and ERROR_ALREADY_EXISTS is returned, so die
	// note the event is automatically deleted by the system when the app exits (even if it crashes)
	HANDLE	hEvent = CreateEventA(NULL, FALSE, FALSE, "AutoRunExclusionZone");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return FALSE;
	}

	// get the autorun directory
	if (!GetCurrentDirectory(MAX_PATH, szAutorunDir))
	{
		return FALSE;
	}

	// remove trailing \ if present
	int	len = strlen(szAutorunDir);
	if (szAutorunDir[len - 1] == '\\')	szAutorunDir[len - 1] = '\0';

	// go into AUTORUN directory
	if (!SetCurrentDirectory("AUTORUN"))
	{
		return FALSE;
	}

	// register window class
	MyRegisterClass(hInstance);

	// get user's language info
	GetLanguageData();

	// load menu data
	pDirector = LoadMenuData();

	if (!pDirector)
	{
		return FALSE;
	}

	// go back up
	SetCurrentDirectory("..");

	// init application instance
	if (!InitInstance(hInstance, nCmdShow))
	{
		delete pDirector;
		return FALSE;
	}

	// load accelerators
	HACCEL	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_AUTORUN);

	// Main message loop:
	MSG	msg;

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete pDirector;

	return msg.wParam;
}

// MyRegisterClass
//
// register the window class

static ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_CLASSDC;										// keep a class DC for speed
	wcex.lpfnWndProc	= (WNDPROC)WndProc;									// window proc
	wcex.cbClsExtra		= 0;												// no class extra data
	wcex.cbWndExtra		= 0;												// no window extra data
	wcex.hInstance		= hInstance;										// us
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_AUTORUN);		// icon
	wcex.hCursor		= NULL;												// cursor
	wcex.hbrBackground	= NULL;												// don't redraw background
	wcex.lpszMenuName	= NULL;												// no menu
	wcex.lpszClassName	= WINCLASS;											// class name from resource file
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_AUTORUN);	// small icon

	return RegisterClassEx(&wcex);
}

// GetLanguageData
//
// obtain user's language

static void GetLanguageData(void)
{
	// get locale ID
	LCID	lcid = GetUserDefaultLCID();
	
	// read language name (in English)
	if (!GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, szLanguage, MAX_PATH))
	{
		// copy default language name (English)
		strcpy(szLanguage, "English");
	}
}

// LoadMenuData
//
// load menu data

static Director* LoadMenuData(void)
{
	FILE*	fd;
	TCHAR	filename[MAX_PATH];

	// first try to find <language>.txt
	sprintf(filename, "%s.txt", szLanguage);
	fd = fopen(filename, "r");

	if (!fd)
	{
		// try English.txt
		strcpy(filename, "English.txt");
		fd = fopen(filename, "r");
	}

	if (!fd)	return NULL;

	Director*	obj = new Director(fd);

	fclose(fd);

	return obj;
}

// InitInstance
//
// get system information
// create main window

static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	// store instance handle
	hInst = hInstance;

	// load cursors
	hNormalCursor = LoadCursor(NULL, (LPCTSTR)IDC_ARROW);
	hSelectCursor = LoadCursor(hInstance, (LPCTSTR)IDC_SELECTCURSOR);

	// create window
	hWnd = CreateWindow(WINCLASS,
						pDirector->WindowTitle,
						WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
						NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// modify window menu
	HMENU	hMenu = GetSystemMenu(hWnd, FALSE);

	RemoveMenu(hMenu, SC_RESTORE, MF_BYCOMMAND);
	RemoveMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
	RemoveMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);

	// open the initial menu
	OpenMenu(hWnd, pDirector->Active);

	// show the window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return TRUE;
}

// OpenMenu
//
// open the given menu

static void OpenMenu(HWND hWnd, Menu* menu)
{
	if (pMenu == menu)	return;	// no change

	if (pMenu)
	{
		CloseMenu();
	}

	// save menu pointer
	pMenu = menu;

	// load bitmap
	SetCurrentDirectory("AUTORUN");

	hBackground = (HBITMAP)LoadImage(NULL, pMenu->Bitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	SetCurrentDirectory("..");

	// get bitmap info
	GetObject(hBackground, sizeof(BITMAP), &bmBackground);

	// get window rects, resize and centre
	RECT	winsize;
	RECT	clientsize;
	RECT	scrsize;
	HWND	hScreen;

	hScreen = GetDesktopWindow();

	GetWindowRect(hWnd, &winsize);
	GetClientRect(hWnd, &clientsize);
	GetClientRect(hScreen, &scrsize);

	int		width = (winsize.right - winsize.left) - (clientsize.right - clientsize.left) + bmBackground.bmWidth;
	int		height = (winsize.bottom - winsize.top) - (clientsize.bottom - clientsize.top) + bmBackground.bmHeight;
	int		xoff = ((scrsize.right - scrsize.left) - width) / 2;
	int		yoff = ((scrsize.bottom - scrsize.top) - height) / 2;

	SetWindowPos(hWnd, NULL, xoff, yoff, width, height, SWP_NOZORDER);

	// create font
	hMenuFont = CreateFont(pMenu->FontSize, 0, 0, 0, pMenu->FontWeight,
							FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, 
							pMenu->FontName);
	
	// measure the menu strings
	HDC			hdc = GetDC(hWnd);
	HFONT		hOldFont = (HFONT)SelectObject(hdc, hMenuFont);
	MenuItem*	item = pMenu->Item;
	ULONG		y = pMenu->TopBorder;

	while (item)
	{
		SIZE	sz;

		GetTextExtentPoint(hdc, item->Text, strlen(item->Text), &sz);

		item->Area.left = pMenu->LeftBorder;
		item->Area.right = pMenu->LeftBorder + sz.cx;
		item->Area.top = y;
		item->Area.bottom = y + sz.cy;

		y += pMenu->LineSpacing * (1 + item->Spacing);

		item = item->Next;
	}

	SelectObject(hdc, hOldFont);
	ReleaseDC(hWnd, hdc);

	// refresh
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);

	// set normal cursor
	SetCursor(hNormalCursor);
}

// CloseMenu
//
// close the menu

static void CloseMenu()
{
	pMenu = NULL;
	pMenuItem = NULL;

	if (hBackground)	DeleteObject(hBackground);
	hBackground = NULL;

	if (hMenuFont)		DeleteObject(hMenuFont);
	hMenuFont = NULL;
}

// WndProc
//
// the window procedure

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_PAINT:
		OnPaint(hWnd);
		break;

	case WM_MOUSEMOVE:
		OnMouseMove(hWnd, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_LBUTTONDOWN:
		OnLButtonDown(hWnd, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// OnPaint
//
// WM_PAINT handler

static void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hWnd, &ps);

	// draw background bitmap
	HDC	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBackground);
	
	BitBlt(hdc, 0, 0, bmBackground.bmWidth, bmBackground.bmHeight, hdcMem, 0, 0, SRCCOPY);

	DeleteDC(hdcMem);

	// draw menu items
	HFONT hFontOld = (HFONT)SelectObject(hdc, hMenuFont);
	MenuItem*	item = pMenu->Item;
	SetBkMode(hdc, TRANSPARENT);

	while (item)
	{
		SetTextColor(hdc, pMenuItem == item ? pMenu->ColourSelected : pMenu->ColourNormal);
		DrawTextEx(hdc, item->Text, -1, &item->Area, DT_SINGLELINE | DT_LEFT | DT_TOP, NULL);
		item = item->Next;
	}

	SelectObject(hdc, hFontOld);

	EndPaint(hWnd, &ps);
}

// OnMouseMove
//
// WM_MOUSEMOVE handler

static void OnMouseMove(HWND hWnd, int x, int y)
{
	MenuItem*	over = NULL;

	MenuItem*	item = pMenu->Item;
	while (item)
	{
		if ((x >= item->Area.left) && (x <= item->Area.right) &&
			(y >= item->Area.top) && (y <= item->Area.bottom))
		{
			over = item;
			break;
		}
		item = item->Next;
	}

	if (over == pMenuItem)	return;	// nothing to do

	HDC		hdc = GetDC(hWnd);
	HFONT	hFontOld = (HFONT)SelectObject(hdc, hMenuFont);
	SetBkMode(hdc, TRANSPARENT);

	// draw old menu item unselected
	if (pMenuItem)
	{
		SetTextColor(hdc, pMenu->ColourNormal);
		DrawTextEx(hdc, pMenuItem->Text, -1, &pMenuItem->Area, DT_SINGLELINE | DT_LEFT | DT_TOP, NULL);
	}

	// draw new item selected
	if (over)
	{
		SetTextColor(hdc, pMenu->ColourSelected);
		DrawTextEx(hdc, over->Text, -1, &over->Area, DT_SINGLELINE | DT_LEFT | DT_TOP, NULL);
	}

	SelectObject(hdc, hFontOld);
	ReleaseDC(hWnd, hdc);

	pMenuItem = over;
	SetCursor(pMenuItem ? hSelectCursor : hNormalCursor);
}

// OnLButtonDown
//
// WM_LBUTTONDOWN handler

static void OnLButtonDown(HWND hWnd, int x, int y)
{
	OnMouseMove(hWnd, x, y);

	if (!pMenuItem)	return;

	// decode command
	TCHAR*	verb = pMenuItem->Verb;
	TCHAR*	doc = pMenuItem->Document;
	TCHAR*	dir = pMenuItem->Directory;

	// set defaults
	if (!dir[0])	dir = "$SRC$";

	bool	bExit = false;

	if (verb[0] == '@')
	{
		bExit = true;
		verb++;
	}

	// run special verbs
	if (!stricmp(verb, "uninstall"))
	{
		// uninstall - run the uninstall string
		verb = "run";
		doc = pDirector->UninstallString;
		bExit = true;
		if (!doc[0])
		{
			ReportError("Product is not installed");
			return;
		}
	}
	else if (!stricmp(verb, "back"))
	{
		// back - go to the last menu
		if (pLastMenu)
		{
			OpenMenu(hWnd, pLastMenu);
			pLastMenu = NULL;
		}
		else
		{
			ReportError("'back' command in top-level menu");
		}
		return;
	}
	else if (!stricmp(verb, "menu"))
	{
		// menu - go to a different menu
		Menu*	menu = pDirector->FindMenu(doc);

		if (menu)
		{
			pLastMenu = pMenu;
			OpenMenu(hWnd, menu);
		}
		else
		{
			ReportError("No such submenu");
		}
		return;
	}

	// macro-replace the document and directory fields
	TCHAR	document[MAX_PATH];
	TCHAR	directory[MAX_PATH];

	if (!MacroReplace(doc, document, MAX_PATH))		return;
	if (!MacroReplace(dir, directory, MAX_PATH))	return;

	// now run the verb
	if (!stricmp(verb, "run"))
	{
		// run - use CreateProcess
		STARTUPINFO		sinfo;

		memset(&sinfo, 0, sizeof(sinfo));
		sinfo.cb = sizeof(sinfo);

		PROCESS_INFORMATION	pinfo;

		if (!CreateProcess(NULL, document, NULL, NULL, FALSE, 0, NULL, directory, &sinfo, &pinfo))
		{
			LPVOID lpMsgBuf;

			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
							FORMAT_MESSAGE_FROM_SYSTEM | 
							FORMAT_MESSAGE_IGNORE_INSERTS, 
							NULL, GetLastError(), 
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							(LPTSTR)&lpMsgBuf, 0, NULL);

			ReportError((LPCTSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);

			return;
		}
	}
	else if (verb[0])
	{
		// other verb - use ShellExecute
		ShellExecute(hWnd, verb, document, NULL, directory, SW_SHOWNORMAL);
	}

	if (bExit)
	{
		PostQuitMessage(0);
	}
}

// MacroReplace
//
// copy cmd to buffer and perform macro replacement

static bool MacroReplace(TCHAR* cmd, TCHAR* buffer, UINT blen)
{
	while (*cmd)
	{
		if (*cmd == '$')
		{
			cmd++;

			TCHAR*	cptr = cmd;

			while (*cptr && (*cptr != '$'))	cptr++;

			if (!*cptr)
			{
				ReportError("Illegal macro in command");
				return false;
			}

			TCHAR*	replace = NULL;

			if (!strnicmp(cmd, "src", cptr - cmd))		replace = szAutorunDir;
			else if (!strnicmp(cmd, "dst", cptr - cmd))	replace = pDirector->AppPath;

			if (!replace || !replace[0])
			{
				ReportError("No such macro");
				return false;
			}

			if (blen < strlen(replace) + 1)
			{
				ReportError("Command is too long");
				return false;
			}
	
			strcpy(buffer, replace);
			buffer += strlen(replace);
			blen -= strlen(replace);
			cmd = cptr + 1;
		}
		else
		{
			if (blen < 2)
			{
				ReportError("Command is too long");
				return false;
			}
			else
			{
				*buffer++ = *cmd++;
				blen--;
			}
		}
	}

	*buffer++ = '\0';
	return true;
}
