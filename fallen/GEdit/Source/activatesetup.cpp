//	ActivateSetup.cpp
//	Matthew Rosenfeld, 13rd October 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"inline.h"
#include	"gi.h"

#include	"EdStrings.h"
#include	"GEdit.h"


//---------------------------------------------------------------

SLONG			prim_type, prim_anim;

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


BOOL	CALLBACK	acts_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the combo box.
			INIT_COMBO_BOX(IDC_COMBO1,wactivate_strings,prim_type);
			PostMessage(hWnd,WM_COMMAND,MAKELONG(IDC_COMBO1,CBN_SELCHANGE),(LPARAM)the_ctrl);
			//	Set up the 'count' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(99,1)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0,
							MAKELONG(prim_anim,0)
						);


			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDC_COMBO1:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						SLONG show=(SendMessage((HWND)lParam,CB_GETCURSEL,0,0)==3)?SW_SHOW:SW_HIDE;
						ShowWindow(GetDlgItem(hWnd,IDC_STATIC_ANIM),show);
						ShowWindow(GetDlgItem(hWnd,IDC_SPIN1),show);
						ShowWindow(GetDlgItem(hWnd,IDC_EDIT1),show);
					}
					break;
				case	IDOK:
					prim_type =	SendMessage	(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
					prim_anim = SendMessage	(GetDlgItem(hWnd,IDC_SPIN1),
											UDM_GETPOS,
											0,0
										);
				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;

			}
			break;

		case	WM_CLOSE:
			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_activate_setup(EventPoint *the_ep)
{

	if (!the_ep) return;
	//	Set the dialog.
	prim_type			=	the_ep->Data[0];
	prim_anim			=	the_ep->Data[1];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_ACTIVATE_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)acts_proc
				);

	//	Set the data.
	the_ep->Data[0]	=	prim_type;
	the_ep->Data[1]	=	prim_anim;
}

//---------------------------------------------------------------

CBYTE	*get_activate_message(EventPoint *ep, CBYTE *msg) {
	if (!ep)
		strcpy(msg,"Unknown");
	else
		strcpy(msg,wactivate_strings[ep->Data[0]]);
	return msg;
}


