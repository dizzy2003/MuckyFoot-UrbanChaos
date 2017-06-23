// GHost.cpp
// Guy Simmons, 20th November 1997.

#include	"DDLib.h"
#include	"fallen/headers/Sound.h"
#include	"a3dmanager.h"
#include	"snd_type.h"
#include	"mfx.h"
#include	"mfx_miles.h"

#define	PAUSE_TIMEOUT			500
#define	PAUSE					(1<<0)
#define	PAUSE_ACK				(1<<1)

int					iGlobalWinMode;
DWORD				ShellID;
HACCEL				hDDLibAccel;
HANDLE				hDDLibThread;
HINSTANCE			hGlobalPrevInst,
					hGlobalThisInst;
LPSTR				lpszGlobalArgs;
WNDCLASS			DDLibClass;

volatile BOOL		ShellActive;
volatile ULONG		PauseFlags		=	0,
					PauseCount		=	0;


LRESULT	CALLBACK	WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------

DWORD	DDLibThread(LPVOID param)
{
	MSG		msg;

#ifdef TARGET_DC
	// Create a basic window - only used for message passing.
	hDDLibWindow = CreateWindowEx (
											0,
											TEXT("Urban Chaos"),
								            TEXT("Urban Chaos"),
											WS_VISIBLE,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											HWND_DESKTOP,
											NULL,
											hGlobalThisInst,
								            NULL
										);

#else
	hDDLibWindow	=	CreateWindowEx	(
											0,
											"Urban Chaos",
								            "Urban Chaos",
											#ifndef NDEBUG
											WS_OVERLAPPEDWINDOW,
											#else
											WS_POPUP,
											#endif
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											HWND_DESKTOP,
											NULL,
											hGlobalThisInst,
								            NULL
										);
#endif

	ShowWindow(hDDLibWindow,iGlobalWinMode);
	UpdateWindow(hDDLibWindow);

	ShellActive	=	TRUE;
	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ShellActive	=	FALSE;

	return	0;
}

//---------------------------------------------------------------

BOOL	SetupKeyboard(void);
void	ResetKeyboard(void);

BOOL	SetupHost(ULONG flags)
{
	DWORD			id;


	ShellActive	=	FALSE;

#ifndef NDEBUG
	InitDebugLog();
#endif

	if(!SetupMemory())
		return	FALSE;
	if(!SetupKeyboard())
		return	FALSE;

#ifdef TARGET_DC
	if(the_input_manager.Init()!=DI_OK)
	{
		ASSERT ( FALSE );
		return	FALSE;
	}
#endif

	// Create the shell window.
#ifdef TARGET_DC
	// Create & register the window class.
	DDLibClass.hInstance		=	hGlobalThisInst;
	DDLibClass.lpszClassName	=	TEXT("Urban Chaos");
	DDLibClass.lpfnWndProc		=	DDLibShellProc;
	DDLibClass.style			=	0;
	DDLibClass.hIcon			=	NULL;
	DDLibClass.hCursor			=	NULL;
	#ifndef NDEBUG
	DDLibClass.lpszMenuName		=	NULL;
	#else
	DDLibClass.lpszMenuName		=	NULL;
	#endif
	DDLibClass.cbClsExtra		=	0;
	DDLibClass.cbWndExtra		=	0;
	DDLibClass.hbrBackground	=	NULL;

	RegisterClass(&DDLibClass);

	hDDLibWindow = CreateWindowEx (
											0,
											TEXT("Urban Chaos"),
								            TEXT("Urban Chaos"),
											WS_VISIBLE,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											HWND_DESKTOP,
											NULL,
											hGlobalThisInst,
								            NULL
										);
#else
	// Create & register the window class.
	DDLibClass.hInstance		=	hGlobalThisInst;
	DDLibClass.lpszClassName	=	TEXT("Urban Chaos");
	DDLibClass.lpfnWndProc		=	DDLibShellProc;
	DDLibClass.style			=	0;
	DDLibClass.hIcon			=	LoadIcon(hGlobalThisInst, MAKEINTRESOURCE(IDI_ICON2));
	DDLibClass.hCursor			=	LoadCursor(NULL, IDC_ARROW);
	#ifndef NDEBUG
	DDLibClass.lpszMenuName		=	MAKEINTRESOURCE(IDR_MAIN_MENU);
	#else
	DDLibClass.lpszMenuName		=	NULL;
	#endif
	DDLibClass.cbClsExtra		=	0;
	DDLibClass.cbWndExtra		=	0;
	DDLibClass.hbrBackground	=	(HBRUSH)GetStockObject(BLACK_BRUSH);

	RegisterClass(&DDLibClass);

	hDDLibWindow	=	CreateWindowEx	(
											0,
											"Urban Chaos",
								            "Urban Chaos",
											#ifndef NDEBUG
											WS_OVERLAPPEDWINDOW,
											#else
											WS_POPUP,
											#endif
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											HWND_DESKTOP,
											NULL,
											hGlobalThisInst,
								            NULL
										);
#endif

	if(hDDLibWindow)
	{
		// Init the sound manager.  We're not too fussed about the result.

#ifdef A3D_SOUND
		A3D_Check_Init();
#endif

#ifdef Q_SOUND
		the_qs_sound_manager.Init();
#endif

#ifdef	M_SOUND
		MilesInit(hGlobalThisInst, hDDLibWindow);
#endif



		// This does shed-loads of work, so it's been moved to much later in the boot process.
//#ifdef TARGET_DC
//extern void MFX_DC_init ( void );
//		MFX_DC_init();
//#endif



#ifndef TARGET_DC
		// Load the keyboard accelerators.
		hDDLibAccel	=	LoadAccelerators(hGlobalThisInst,MAKEINTRESOURCE(IDR_MAIN_ACCELERATOR));
#endif

		// Display the window.
//		ShowWindow(hDDLibWindow,iGlobalWinMode);
//		UpdateWindow(hDDLibWindow);

		ShellActive	=	TRUE;
	}

	the_game.DarciStrength=0;
	the_game.DarciStamina=0;
	the_game.DarciSkill=0;
	the_game.DarciConstitution=0;

	return	ShellActive;
}

//---------------------------------------------------------------

void	ResetHost(void)
{
#ifdef M_SOUND
	MilesTerm();
#endif

	ResetKeyboard();
	ResetMemory();

#ifndef NDEBUG
	FiniDebugLog();
#endif

    UnregisterClass(TEXT("Urban Chaos"),GetModuleHandle(NULL));
}

//---------------------------------------------------------------

void	ShellPaused(void)
{
	return;

	SLONG		timeout;


	if(PauseFlags&PAUSE)
	{
		PauseFlags	|=	PAUSE_ACK;						// Send acknowledgement,
// do_pause_check:
		timeout	=	GetTickCount();
		while(PauseFlags&PAUSE)							// Wait while paused
		{
			if(Keys[KB_L])
			{
				LibShellMessage("ShellPauseOff: Timeout",__FILE__,__LINE__);
			}
/*
			if((GetTickCount()-timeout)>PAUSE_TIMEOUT)
			{
				g_ShellMessage("ShellPaused: Timeout",__FILE__,__LINE__);
				goto	do_pause_check;
			}
*/
		}
		PauseFlags	|=	PAUSE_ACK;						// Send acknowledgement,
	}
}

//---------------------------------------------------------------

void	ShellPauseOn(void)
{
	the_display.toGDI();
	return;

	SLONG		timeout;


	PauseCount++;
	if(PauseCount==1)
	{
		PauseFlags	|=	PAUSE;							// Set pause flag.
// do_ack_check:
		timeout	=	GetTickCount();
		while(!(PauseFlags&PAUSE_ACK))					// Wait for acknowledgement.
		{
			if(Keys[KB_L])
			{
				LibShellMessage("ShellPauseOff: Timeout",__FILE__,__LINE__);
			}
/*
			if((GetTickCount()-timeout)>PAUSE_TIMEOUT)
			{
				g_ShellMessage("ShellPauseOn: Timeout",__FILE__,__LINE__);
				goto	do_ack_check;
			}
*/
		}
		PauseFlags	&=	~PAUSE_ACK;						// Clear ack flag.

		the_display.toGDI();
	}
}

//---------------------------------------------------------------

void	ShellPauseOff(void)
{
	return;

	SLONG		timeout;


	if(PauseCount==0)
		return;

	if(PauseCount==1)
	{
		the_display.fromGDI();

		PauseFlags	&=	~PAUSE;							// Clear pause flag.
// do_ack_check:
		timeout	=	GetTickCount();
		while(!(PauseFlags&PAUSE_ACK))	// Wait for acknowledgement.
		{
			if(Keys[KB_L])
			{
				LibShellMessage("ShellPauseOff: Timeout",__FILE__,__LINE__);
			}
/*
			if((GetTickCount()-timeout)>PAUSE_TIMEOUT)
			{
				g_ShellMessage("ShellPauseOff: Timeout",__FILE__,__LINE__);
				goto	do_ack_check;
			}
*/
		}
		PauseFlags	&=	~PAUSE_ACK;						// Clear ack flag.
	}
	PauseCount--;
}

//---------------------------------------------------------------
extern void ClearLatchedKeys();

extern SLONG app_inactive;
extern SLONG restore_surfaces;


BOOL	LibShellActive(void)
{
	SLONG		result;
	MSG			msg;

	//
	// release any remembered keys
	//
	ClearLatchedKeys();

	while(1)
	{
		while(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
		{
			result	=	(SLONG)GetMessage(&msg,NULL,0,0);
#ifndef TARGET_DC
			if(result)
			{
				if(!TranslateAccelerator(hDDLibWindow,hDDLibAccel,&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				ShellActive	=	FALSE;
			}
#endif
		}


		if (app_inactive && the_display.lp_DD4 && the_display.IsFullScreen())
		{
			Sleep(100);
		}
		else
		{
			break;
		}
	}

	if (restore_surfaces)
	{
		if (the_display.lp_DD4)
		{
			the_display.lp_DD4->RestoreAllSurfaces();

			extern void FRONTEND_restore_screenfull_surfaces(void);

			FRONTEND_restore_screenfull_surfaces();
		}

		restore_surfaces = FALSE;
	}

	return	ShellActive;
}

//---------------------------------------------------------------

BOOL	LibShellChanged(void)
{
	if(the_display.IsDisplayChanged())
	{
		the_display.DisplayChangedOff();
		return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

BOOL	LibShellMessage(const char *pMessage, const char *pFile, ULONG dwLine)
{
	BOOL		result;
	CBYTE		buff1[512],
				buff2[512];
	ULONG		flag;

	if (pMessage == NULL)
	{
		pMessage = "Looks like a coder has caught a bug.";
	}

	TRACE("LbShellMessage: %s\n", pMessage);

#ifndef TARGET_DC
	wsprintf( buff1, "Uh oh, something bad's happened!");
	wsprintf( buff2, "%s\n\n%s\n\nIn   : %s\nline : %u", buff1, pMessage, pFile, dwLine);
	strcat(buff2, "\n\nAbort=Kill Application, Retry=Debug, Ignore=Continue");
	flag	=	MB_ABORTRETRYIGNORE|MB_ICONSTOP|MB_DEFBUTTON3;

	result	=	FALSE;
	the_display.toGDI();
	switch(MessageBox(hDDLibWindow,buff2,"Mucky Foot Message",flag))
	{
		case	IDABORT:
			exit(1);
			break;
		case	IDCANCEL:
		case	IDIGNORE:
			break;
		case	IDRETRY:
			_asm int 3;
			break;
	}

	the_display.fromGDI();
#else
	result = TRUE;
#endif

	return	result;
}

//---------------------------------------------------------------

void	Time(MFTime *the_time)
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
//
// WinMain - Entry point for windows application.
//
//---------------------------------------------------------------

static UWORD	argc;
static LPTSTR	argv[MAX_PATH];

#ifdef TARGET_DC
// Include this again in just one file - this one.
#include "dtags.h"
#endif

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPTSTR lpszArgs, int iWinMode)
{
	// Store WinMain parameters.
#ifdef TARGET_DC
	// This malloc has to be a malloc, not a MemAlloc - the heap has not yet been set up.
	lpszGlobalArgs = (char *)malloc ( ( _tcslen (lpszArgs) + 1 ) * sizeof(*lpszGlobalArgs) );
	ASSERT ( lpszGlobalArgs != NULL );
	textConvertUniToChar ( lpszGlobalArgs, lpszArgs );
#else
	lpszGlobalArgs	=	lpszArgs;
#endif
	iGlobalWinMode	=	iWinMode;
	hGlobalThisInst	=	hThisInst;
	hGlobalPrevInst	=	hPrevInst;

void	init_best_found(void);
	init_best_found();


#ifdef TARGET_DC
	// Init the DTrace logging.
	LOG_INIT
#endif


	#if 0	// Someone already done it! :)
	#ifdef NDEBUG

	//
	// So you can't have multiple release builds of fallen running at once!
	//

	CreateMutex(NULL, TRUE, "This is your friendly Urban Chaos mutex!");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		//
		// Fallen is already running!
		//

		return 0;
	}

	#endif
	#endif

#ifndef FINAL
//	extern void CONSOLE_TCP();
//	CONSOLE_TCP();
#endif

	// try and create an event
	// this always succeeds
	// if event existed before (still succeeds) and ERROR_ALREADY_EXISTS is returned, so die
	// note the event is automatically deleted by the system when the app exits (even if it crashes)

	#ifdef FINAL
	HANDLE	hEvent = CreateEventA(NULL, FALSE, FALSE, "UrbanChaosExclusionZone");
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	#endif
	{
		return MF_main(argc,argv);
	}

	return ERROR_ALREADY_EXISTS;
}

//---------------------------------------------------------------
