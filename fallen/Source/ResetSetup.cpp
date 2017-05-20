//	TransferSetup.cpp
//	Mark Adami.

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

SLONG	reset_counter;

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


BOOL	CALLBACK	reset_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;

	
	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the 'transfer' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(9,0)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0,
							MAKELONG(reset_counter,0)
						);

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					reset_counter = SendMessage (GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);

				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hWnd,0);
			return TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_reset_pick(EventPoint *the_ep)
{
	//	Set the dialog.
	reset_counter		=	the_ep->Data[0];

	//	Do the dialog.

	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_RESET_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)reset_proc
				);


	//	Get the data.
	the_ep->Data[0]		=	reset_counter;
}

//---------------------------------------------------------------
