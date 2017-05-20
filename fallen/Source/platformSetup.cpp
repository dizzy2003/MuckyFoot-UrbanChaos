//	PlatformSetup.cpp
//	Matthew Rosenfeld, 10th November 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
//#include	"fmatrix.h"
//#include	"inline.h"
//#include	"gi.h"
#include	"EdStrings.h"
#include	"ticklist.h"

#include	"GEdit.h"


//---------------------------------------------------------------

SLONG		platform_speed,platform_flags;

//---------------------------------------------------------------

BOOL	CALLBACK	plats_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;
//	LPTSTR		lbitem_str;
//	SLONG		c0	=	0;


	switch(message)
	{
		case	WM_INITDIALOG:
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,platform_speed);
			ticklist_init(hWnd, IDC_LIST1, wplatform_flag_strings,platform_flags);
			return	TRUE;

		case WM_MEASUREITEM:
			return ticklist_measure(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			return ticklist_draw(hWnd, wParam, lParam);

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					platform_speed=SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
					platform_flags=ticklist_bitmask(hWnd,IDC_LIST1);

				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;
		case	WM_CLOSE:
			ticklist_close(hWnd, IDC_LIST1);
			EndDialog(hWnd,0);
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_platform_setup(EventPoint *the_ep)
{
	platform_speed =	the_ep->Data[0];
	platform_flags =	the_ep->Data[1];
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_PLATFORM_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)plats_proc
				);

	the_ep->Data[0]	=	platform_speed;
	the_ep->Data[1]	=	platform_flags;
}

//---------------------------------------------------------------
/*
CBYTE	*get_camera_message(EventPoint *ep, CBYTE *msg) {
  if ((!ep)||(!ep->Data[0])||(!ep->Data[1]))
	  strcpy(msg,"Unknown");
  else
	sprintf(msg,"%s %s",wcammove_strings[ep->Data[0]-1],wcamtype_strings[ep->Data[1]-1]);
  return msg;
}
*/