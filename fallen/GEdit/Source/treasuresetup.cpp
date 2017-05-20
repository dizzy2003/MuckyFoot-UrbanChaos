//	TreasureSetup.cpp
//	Matthew Rosenfeld, some time in March 1999.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"EdStrings.h"

#include	"GEdit.h"


//---------------------------------------------------------------

SLONG		treasure_value;

//---------------------------------------------------------------

BOOL	CALLBACK	treasure_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;


	switch(message)
	{
		case	WM_INITDIALOG:
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETRANGE,0,MAKELONG(0,10000));
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,treasure_value);
			return	TRUE;
		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					treasure_value=SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
				case	IDCANCEL:
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_treasure_setup(EventPoint *the_ep)
{
	treasure_value =	the_ep->Data[0];
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_TREASURE_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)treasure_proc
				);

	the_ep->Data[0]	=	treasure_value;
}

//---------------------------------------------------------------

CBYTE	*get_treasure_message(EventPoint *ep, CBYTE *msg) {
	if ((!ep)||!ep->Data[0])
		msg[0]=0;
	else {
		sprintf(msg,"%d points",ep->Data[0]);
	}
  return msg;
}
