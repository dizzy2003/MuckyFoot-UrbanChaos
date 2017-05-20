//	WptSetup.cpp
//	Matthew Rosenfeld 04th March 1998.

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

static SLONG sign;
static SLONG flip;

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


BOOL	CALLBACK	sign_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;

	
	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the 'speed' spin.

			switch(sign)
			{
				case 0:
					SendMessage(
						GetDlgItem(hWnd, IDC_UTURN),
						BM_SETCHECK,
						(WPARAM) BST_CHECKED,
						0);
					break;

				case 1:
					SendMessage(
						GetDlgItem(hWnd, IDC_RIGHT_TURN),
						BM_SETCHECK,
						(WPARAM) BST_CHECKED,
						0);
					break;

				case 2:
					SendMessage(
						GetDlgItem(hWnd, IDC_AHEAD),
						BM_SETCHECK,
						(WPARAM) BST_CHECKED,
						0);
					break;

				case 3:
					SendMessage(
						GetDlgItem(hWnd, IDC_STOP),
						BM_SETCHECK,
						(WPARAM) BST_CHECKED,
						0);
					break;
			}

			CheckDlgButton(hWnd, IDC_FLIP_LEFT_RIGHT, (flip & 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_FLIP_TOP_BOTTOM, (flip & 2) ? BST_CHECKED : BST_UNCHECKED);

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:

					sign = 0;

					if (SendMessage(GetDlgItem(hWnd, IDC_UTURN),     BM_GETCHECK,0,0) == BST_CHECKED) {sign = 0;}
					if (SendMessage(GetDlgItem(hWnd, IDC_RIGHT_TURN),BM_GETCHECK,0,0) == BST_CHECKED) {sign = 1;}
					if (SendMessage(GetDlgItem(hWnd, IDC_AHEAD),     BM_GETCHECK,0,0) == BST_CHECKED) {sign = 2;}
					if (SendMessage(GetDlgItem(hWnd, IDC_STOP),      BM_GETCHECK,0,0) == BST_CHECKED) {sign = 3;}

					flip = 0;

					if (IsDlgButtonChecked(hWnd, IDC_FLIP_LEFT_RIGHT) == BST_CHECKED) {flip |= 1;}
					if (IsDlgButtonChecked(hWnd, IDC_FLIP_TOP_BOTTOM) == BST_CHECKED) {flip |= 2;}

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

void	do_sign_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	sign		=	the_ep->Data[0];
	flip		=	the_ep->Data[1];

	//	Do the dialog.

	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_SIGN_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)sign_proc
				);


	//	Get the data.
	the_ep->Data[0]		=	sign;
	the_ep->Data[1]     =	flip;
}

//---------------------------------------------------------------
