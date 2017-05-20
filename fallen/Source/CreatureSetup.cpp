//	CreatureSetup.cpp
//	Guy Simmons, 5th September 1998.


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

SLONG			creature_count,
				creature_type;

//---------------------------------------------------------------

BOOL	CALLBACK	cs_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the combo box.
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			lbitem_str	=	wcreature_strings[0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	wcreature_strings[++c0];
			}
			//	Set its default item.
			SendMessage(the_ctrl,CB_SETCURSEL,creature_type-1,0);

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
							MAKELONG(creature_count,0)
						);

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_VSCROLL:
			//	Set up the 'count' or 'constitution'.
			if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN1 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				creature_count			=	HIWORD(wParam);
				return	TRUE;
			}
			break;

		case	WM_CLOSE:
			//	Set the 'type'.
			creature_type	=	SendMessage	(
												GetDlgItem(hWnd,IDC_COMBO1),
												CB_GETCURSEL,
												0,0
											)	+	1;

			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_creature_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	creature_type			=	the_ep->Data[0];
	creature_count			=	the_ep->Data[1];
	if(creature_count==0)
		creature_count	=	1;		//	Default to 1.

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_CREATURE_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)cs_proc
				);

	//	Set the data.
	the_ep->Data[0]	=	creature_type;
	the_ep->Data[1]	=	creature_count;
}

//---------------------------------------------------------------
