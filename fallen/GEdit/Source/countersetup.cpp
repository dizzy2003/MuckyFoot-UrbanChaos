//	CounterSetup.cpp
//	Matthew Rosenfeld, 25th March 1999.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"EdStrings.h"

#include	"GEdit.h"


//---------------------------------------------------------------

SLONG		counter_value,counter_index;

//---------------------------------------------------------------

BOOL	CALLBACK	counter_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;


	switch(message)
	{
		case	WM_INITDIALOG:
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETRANGE,0,MAKELONG(0,256));
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,MAKELONG(counter_value,0));
			SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_SETRANGE,0,MAKELONG(1,10));
			SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_SETPOS,0,MAKELONG(counter_index,0));
			return	TRUE;
		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					counter_value=SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
					counter_index=SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_GETPOS,0,0);
				case	IDCANCEL:
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_counter_setup(EventPoint *the_ep)
{
	counter_value =	the_ep->Data[0];
	counter_index =	the_ep->Data[1];
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_COUNTER_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)counter_proc
				);

	the_ep->Data[0]	=	counter_value;
	the_ep->Data[1]	=	counter_index;
}

//---------------------------------------------------------------

CBYTE	*get_counter_message(EventPoint *ep, CBYTE *msg) {
	if ((!ep)||!ep->Data[1])
		msg[0]=0;
	else {
		sprintf(msg,"counter %d by %d",ep->Data[1],ep->Data[0]);
	}
  return msg;
}
