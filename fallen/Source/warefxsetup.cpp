//	WareFXSetup.cpp
//	Matthew Rosenfeld, 21st September 1998.

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

SLONG	warefx_type;

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

BOOL	CALLBACK	warefx_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;

	
	switch(message)
	{
		case	WM_INITDIALOG:

			INIT_COMBO_BOX(IDC_COMBO1,wwarefx_strings,warefx_type);
			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					warefx_type  = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);

				case	IDCANCEL:
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;

	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_warefx_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	warefx_type 		=	the_ep->Data[0];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_WAREFX_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)warefx_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	warefx_type;
}

//---------------------------------------------------------------

CBYTE	*get_warefx_message(EventPoint *ep, CBYTE *msg) {
	strcpy(msg,wwarefx_strings[ep->Data[0]]);
	return msg;
}


