// MFHost.cpp	-	Windows.
// Guy Simmons, 1st February 1997.


#include	<MFHeader.h>

#define	SHELL_NAME		"Mucky Foot Shell"

int						iGlobalCmdShow;
HANDLE					hShellThread	=	NULL;
HINSTANCE				hGlobalInstance,
						hGlobalPrevInstance;
HWND					hShellWindow	=	NULL;
LPSTR					szGlobalCmdLine;
MFFileHandle			log_handle		=	NULL;
WNDCLASS				wnd_class;
RECT					ShellRect;

static UWORD	argc;
static LPTSTR	argv[MAX_PATH];

LRESULT CALLBACK		WndProc(HWND hwnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
ULONG					ShellThread(ULONG arg);
SLONG					CreateShellWindow(void);

extern UBYTE					DisplayState;

//---------------------------------------------------------------

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR szCmdLine, int iCmdShow)
{
	LPSTR		lp_str;


	szGlobalCmdLine		=	szCmdLine;
	iGlobalCmdShow		=	iCmdShow;
	hGlobalInstance		=	hInstance;
	hGlobalPrevInstance	=	hPrevInstance;

	argc	=	1;
	lp_str	=	szCmdLine;							 
	while(*lp_str!=0)
	{
		while(*lp_str=='\t'||*lp_str==' ')
		{
			lp_str++;
		}
		if(*lp_str!=0)
		{
			argv[argc++]	=	lp_str;
			while(*lp_str!='\t'&&*lp_str!=' '&&*lp_str!=0)
			{
				lp_str++;
			}
			if(*lp_str!=0)
			{
				*lp_str	=	0;
				lp_str++;
			}
		}
	}

	return MF_main(argc,argv);
}

//---------------------------------------------------------------

BOOL	SetupHost(ULONG flags)
{
	DWORD			id;


	if(!SetupMemory())
		return	FALSE;
	if(!SetupKeyboard())
		return	FALSE;

	if(flags&H_CREATE_LOG)
	{
		log_handle	=	FileCreate("debug.log",1);
		if(log_handle==FILE_CREATION_ERROR)
			log_handle	=	NULL;			
	}

	// Set up shell window.
	wnd_class.style			=	0; //CS_HREDRAW|CS_VREDRAW;
	wnd_class.lpfnWndProc	=	WndProc;
	wnd_class.cbClsExtra	=	0;
	wnd_class.cbWndExtra	=	0;
	wnd_class.hInstance		=	hGlobalInstance;
	wnd_class.hIcon			=	LoadIcon(NULL, IDI_APPLICATION);
	wnd_class.hCursor		=	LoadCursor(NULL, IDC_ARROW);
	wnd_class.hbrBackground	=	(HBRUSH)GetStockObject(BLACK_BRUSH);
	wnd_class.lpszMenuName	=	NULL;
	wnd_class.lpszClassName	=	SHELL_NAME;

	RegisterClass(&wnd_class);

	hShellThread	=	CreateThread(
										NULL,
										0,
										(LPTHREAD_START_ROUTINE)ShellThread,
										0,
										0,
										&id
									);
	ERROR_MSG(hShellThread,"Can't create the shell thread.")
	if(hShellThread)
	{
		// Wait for shell to activate.
		// This could potentially cause a problem if the shell window couldn't be created.
		while(!DisplayActive)
		{
			SleepEx(100,FALSE);
		}
	}

	return	TRUE;
}

//---------------------------------------------------------------

void	ResetHost(void)
{
	if(log_handle)
		FileClose(log_handle);
	ResetKeyboard();
	ResetMemory();
}

//---------------------------------------------------------------

LRESULT CALLBACK	WndProc(HWND h_window,UINT i_message,WPARAM w_param,LPARAM l_param)
{
	switch (i_message)
	{
	    case WM_SIZE:
	    case WM_MOVE:
//			LogText("WM_SIZE or WM_MOVE\n");
	        if (IsIconic(h_window))
	        {
	            // Shell is minimized, maybe set Pause flag?
	        }
#ifdef	_DEBUG
	            GetClientRect(h_window, &ShellRect);
	            ClientToScreen(h_window, (LPPOINT)&ShellRect);
	            ClientToScreen(h_window, (LPPOINT)&ShellRect+1);

#else
				SetRect(&ShellRect,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
#endif
			if(lp_DD_FrontSurface)
				ClearDisplay();
	        break;
	    case WM_QUERYNEWPALETTE:
//			LogText("WM_QUERYNEWPALETTE\n");
			RestorePalettes();
	        break;
	    case WM_PALETTECHANGED:
//			LogText("WM_PALETTECHANGED\n");
			RestorePalettes();
			break;
		case	WM_MOUSEMOVE:
//			LogText("WM_MOUSEMOVE:\n");
			MouseX			=	LOWORD(l_param);
			MouseY			=	HIWORD(l_param);
			MousePoint.X	=	MouseX;
			MousePoint.Y	=	MouseY;
			MouseMoved		=	1;
			break;
		case	WM_RBUTTONUP:
//			LogText("WM_RBUTTONUP\n");
			RightButton		=	0;
			break;
		case	WM_RBUTTONDOWN:
//			LogText("WM_RBUTTONDOWN\n");
			RightButton		=	1;
			if(!RightMouse.ButtonState)
			{
				RightMouse.ButtonState	=	1;
				RightMouse.MouseX		=	LOWORD(l_param);
				RightMouse.MouseY		=	HIWORD(l_param);
				RightMouse.MousePoint.X	=	LOWORD(l_param);
				RightMouse.MousePoint.Y	=	HIWORD(l_param);
			}
			break;
		case	WM_RBUTTONDBLCLK:
			break;
		case	WM_LBUTTONUP:
			LogText("WM_LBUTTONUP\n");
			LeftButton		=	0;
			break;
		case	WM_LBUTTONDOWN:
//			LogText("WM_LBUTTONDOWN\n");
			LeftButton		=	1;
			if(!LeftMouse.ButtonState)
			{
				LeftMouse.ButtonState	=	1;
				LeftMouse.MouseX		=	LOWORD(l_param);
				LeftMouse.MouseY		=	HIWORD(l_param);
				LeftMouse.MousePoint.X	=	LOWORD(l_param);
				LeftMouse.MousePoint.Y	=	HIWORD(l_param);
			}
			break;
		case	WM_LBUTTONDBLCLK:
			break;
		case	WM_MBUTTONUP:
			MiddleButton	=	0;
			break;
		case	WM_MBUTTONDOWN:
//			LogText("WM_MBUTTONDOWN\n");
			MiddleButton	=	1;
			if(!MiddleMouse.ButtonState)
			{
				MiddleMouse.ButtonState	=	1;
				MiddleMouse.MouseX		=	LOWORD(l_param);
				MiddleMouse.MouseY		=	HIWORD(l_param);
				MiddleMouse.MousePoint.X=	LOWORD(l_param);
				MiddleMouse.MousePoint.Y=	HIWORD(l_param);
			}
			break;
		case	WM_MBUTTONDBLCLK:
			break;

//#ifdef	_DEBUG
extern LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
		case	WM_KEYDOWN:
		case	WM_KEYUP:
//			LogText("WM_KEYDOWN or WM_KEYUP\n");
			KeyboardProc( 0, 0, l_param);
			break;
//#endif

		case	WM_CLOSE:
//			LogText("WM_CLOSE\n");
			DisplayActive	=	0;
			break;
		case	WM_DESTROY:
//			LogText("WM_DESTROY\n");
			PostQuitMessage(0);
			DisplayState	&=	~(DS_SHELL_WINDOW_OPEN);
			break;
//		default:
//			LogText("Unknown event - %ld\n",i_message);
	}
	return DefWindowProc(h_window,i_message,w_param,l_param);
}

//---------------------------------------------------------------

ULONG	ShellThread(ULONG arg)
{
	SLONG		result;
	MSG			msg;


	arg		=	arg;

	DisplayState	|=	DS_THREAD_ACTIVE;
	result			=	CreateShellWindow();

	if(result==NoError)
	{
/*
		while(1)
		{
	        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	        {
//	            if(!GetMessage(&msg,NULL,0,0))
//	                break;
	            TranslateMessage(&msg); 
	            DispatchMessage(&msg);
	        }
	        else
	        {
	            // make sure we go to sleep if we have nothing else to do
	            WaitMessage();
	        }
		}
*/
		while(GetMessage(&msg,NULL,0,0) && (DisplayState&DS_SHELL_WINDOW_OPEN))
		{
	    	TranslateMessage(&msg); 
	    	DispatchMessage(&msg);
		}
	}
	SleepEx(100,FALSE);
	DisplayActive	=	0;
	MFShellActive	=	0;

	DisplayState	&=	~(DS_THREAD_ACTIVE);
	return	EXIT_SUCCESS;
}

//---------------------------------------------------------------

SLONG	CreateShellWindow(void)
{
	hShellWindow	=	CreateWindowEx	(
											0,
											SHELL_NAME,
								            SHELL_NAME,
											WS_POPUP|WS_VISIBLE|WS_SYSMENU,
#ifdef	_DEBUG
											0,0,
											640,
											480,
#else
											0,0,
											GetSystemMetrics(SM_CXSCREEN),
											GetSystemMetrics(SM_CYSCREEN),
#endif
											NULL,
											NULL,
											hGlobalInstance,
								            NULL
										);
	ERROR_MSG(hShellWindow,"Can't create the shell window.")

	if(hShellWindow)
	{
		DisplayState	|=	DS_SHELL_WINDOW_OPEN;
		DisplayActive	=	1;
		MFShellActive	=	1;

//		ShowWindow(hShellWindow,SW_SHOWNORMAL);
//		UpdateWindow(hShellWindow);
		return	NoError;
	}
	else
	{
		return	-1;
	}
}

//---------------------------------------------------------------

void	LogText(CBYTE *error, ...)
{
	CBYTE 			buf[512];
	va_list 		argptr;

	if(log_handle)
	{
		va_start(argptr,error); 
		vsprintf(buf, error,argptr); 
		va_end(argptr);
		
		FileWrite(log_handle,buf,strlen(buf));
	}
}

//---------------------------------------------------------------
extern HWND					hShellWindow;

int MFMessage(const char *pMessage, const char *pFile, ULONG dwLine)
{
	char buff1[512];
	char buff2[512];
	ULONG	flag; 


	LogText("Mucky Foot Message\n    %s\nIn   : %s\nLine : %u",pMessage,pFile,dwLine);

	wsprintf( buff1, "Something strange has happened!");
	wsprintf( buff2, "%s\n\n%s\n\nIn   : %s\nline : %u", buff1, pMessage, pFile, dwLine);
	strcat(buff2, "\n\nAbort=Kill Application, Retry=Debug, Ignore=Continue");
	flag = MB_ABORTRETRYIGNORE|MB_ICONSTOP|MB_DEFBUTTON3;

	switch(MessageBox(hShellWindow, buff2, "Mucky Foot Message", flag))
	{
		case IDABORT:
		case IDCANCEL:
			exit(1);
			break;

		case IDRETRY:
			return TRUE;

	}
	return FALSE;

}

//---------------------------------------------------------------

void	Time(struct MFTime *the_time)
{
	SYSTEMTIME	new_time;


	GetLocalTime(&new_time);
	the_time->Hours		=	new_time.wHour;
	the_time->Minutes	=	new_time.wMinute;
	the_time->Seconds	=	new_time.wSecond;
	the_time->MSeconds	=	new_time.wMilliseconds;
	the_time->DayOfWeek	=	new_time.wDayOfWeek;
	the_time->Day		=	new_time.wDay;
	the_time->Month		=	new_time.wMonth;
	the_time->Year		=	new_time.wYear;
	the_time->Ticks		=	GetTickCount();
}

//---------------------------------------------------------------
