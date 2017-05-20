//	BurnSetup.cpp
//	Matthew Rosenfeld 23th November 1998.

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

SLONG	burn_type;

//---------------------------------------------------------------
/*
#define	INIT_COMBO_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,CB_SETCURSEL,d,0);
*/
//---------------------------------------------------------------


BOOL	CALLBACK	burn_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
//	HWND		the_ctrl;
//	LPTSTR		lbitem_str;

	
	switch(message)
	{
		case	WM_INITDIALOG:
			ticklist_init(hWnd, IDC_LIST1, wfire_strings,burn_type);
			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					burn_type = ticklist_bitmask(hWnd,IDC_LIST1);

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

void	do_burn_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	burn_type		=	the_ep->Data[0];

	//	Do the dialog.

	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_BURN_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)burn_proc
				);


	//	Get the data.
	the_ep->Data[0]		=	burn_type;
}

//---------------------------------------------------------------
