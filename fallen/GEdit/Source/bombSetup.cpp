//	BombSetup.cpp
//	Guy Simmons, 17th November 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"

#include	"EdStrings.h"
#include	"GEdit.h"
#include	"ticklist.h"


//---------------------------------------------------------------

SLONG	bomb_type,bomb_size,bomb_fx;

//---------------------------------------------------------------

#define	INIT_COMBO_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,CB_SETCURSEL,d,0);

//---------------------------------------------------------------


BOOL	CALLBACK	bomb_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;

	
	switch(message)
	{
		case	WM_INITDIALOG:
			INIT_COMBO_BOX(IDC_COMBO1, wbombtype_strings,bomb_type);
			ticklist_init(hWnd, IDC_LIST1, wvfx_strings,bomb_fx);

			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(1024,0));
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,MAKELONG(bomb_size,0));

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					bomb_type = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
					bomb_size = SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
					bomb_fx   = ticklist_bitmask(hWnd,IDC_LIST1);

				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case WM_CLOSE:
			ticklist_close(hWnd, IDC_LIST1);
			EndDialog(hWnd,0);
			return TRUE;

		case WM_MEASUREITEM:
			return ticklist_measure(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			return ticklist_draw(hWnd, wParam, lParam);

	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_bomb_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	bomb_type		=	the_ep->Data[0];
	bomb_size		=	the_ep->Data[1];
	bomb_fx			=	the_ep->Data[2];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_BOMB_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)bomb_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	bomb_type;
	the_ep->Data[1]		=	bomb_size;
	the_ep->Data[2]		=	bomb_fx;
}

//---------------------------------------------------------------
