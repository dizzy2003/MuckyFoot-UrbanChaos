//	BonusSetup.cpp
//	Matthew Rosenfeld, 15th March 1998.

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
#include	"GEdit.h"


//---------------------------------------------------------------

CBYTE		*bonus_text;
SLONG		 bonus_pts;
SLONG		 bonus_type;
SLONG		 bonus_gender; // for translators

//---------------------------------------------------------------

TCHAR	*bstrings[]	=
{
	"Primary Objective",
	"Secondary Objective",
	"Bonus",
	"!"
};


#define	INIT_COMBO_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,CB_SETCURSEL,d,0);


BOOL	CALLBACK	bonus_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG len;
	HWND		the_ctrl;
	SLONG		c0	=	0;
	LPTSTR		lbitem_str;

	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the edit text.
			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							EM_SETLIMITTEXT,
							2550,0
						);

			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_SETTEXT,
							0,(LPARAM)bonus_text
						);

			SendMessage(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0, MAKELONG(0,1000)
						);
			SendMessage(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0, MAKELONG(bonus_pts,0)
						);

			INIT_COMBO_BOX(IDC_COMBO1,bstrings,bonus_type);

			if (bonus_gender) c0=IDC_RADIO2; else c0=IDC_RADIO1;
			SendMessage(GetDlgItem(hWnd,c0),BM_SETCHECK,BST_CHECKED,0);

			SetFocus(GetDlgItem(hWnd,IDC_EDIT1));
			return	FALSE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_CLOSE:
			len=SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_GETTEXTLENGTH,
							0,0
						)+1;
			if (bonus_text) free(bonus_text);
			bonus_text =	(CBYTE*)malloc(len);
			ZeroMemory(bonus_text,len);
			
			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_GETTEXT,
							len,
							(LPARAM)bonus_text
						);
			bonus_pts   = SendMessage( GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
			bonus_type  = SendMessage( GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);

			bonus_gender= SendMessage( GetDlgItem(hWnd,IDC_RADIO2),BM_GETCHECK,0,0)==BST_CHECKED;

			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_bonus_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	bonus_text	=	(CBYTE*)the_ep->Data[0];
	bonus_pts	=	the_ep->Data[1];
	bonus_type 	=	the_ep->Data[2];
	bonus_gender=	the_ep->Data[3];
	if(!bonus_text)
	{
		bonus_text	=	(CBYTE*)malloc(_MAX_PATH);
		ZeroMemory(bonus_text,_MAX_PATH);
		strcpy(bonus_text,"Secondary objective complete. 200 points.");
		bonus_pts=200;
		bonus_type=1;
		SetEPTextID(the_ep);
	}

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_BONUS_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)bonus_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	(SLONG)bonus_text;
	the_ep->Data[1]		=	bonus_pts;
	the_ep->Data[2]		=	bonus_type;
	the_ep->Data[3]		=	bonus_gender;
}

//---------------------------------------------------------------


CBYTE	*get_bonus_message(EventPoint *ep, CBYTE *msg) {
	msg[0]=0;
	if (ep&&ep->Data[0])
		strcpy(msg,(CBYTE*)ep->Data[0]);
	return msg;
}
