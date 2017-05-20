//	SpotFXSetup.cpp
//	Matthew Rosenfeld, 10th February 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"fmatrix.h"
#include	"inline.h"
#include	"gi.h"

#include	"EdStrings.h"
#include	"GEdit.h"


//---------------------------------------------------------------

SLONG	spotfx_type,spotfx_scale;

//---------------------------------------------------------------

#define	INIT_LIST_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,LB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,LB_SETCURSEL,d,0);

BOOL	CALLBACK	spotfx_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;

	
	switch(message)
	{
		case	WM_INITDIALOG:

			INIT_LIST_BOX(IDC_LIST1,wspotfx_strings,spotfx_type);
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(1024,0));
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,MAKELONG(spotfx_scale,0));

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					spotfx_type  = SendMessage(GetDlgItem(hWnd,IDC_LIST1),LB_GETCURSEL,0,0);
					spotfx_scale = SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);

				case	IDCANCEL:
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;

	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_spotfx_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	spotfx_type 		=	the_ep->Data[0];
	spotfx_scale		=	the_ep->Data[1];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_SPOTFX_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)spotfx_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	spotfx_type;
	the_ep->Data[1]		=	spotfx_scale;
}

//---------------------------------------------------------------

CBYTE	*get_spotfx_message(EventPoint *ep, CBYTE *msg) {
	strcpy(msg,wspotfx_strings[ep->Data[0]]);
	return msg;
}


