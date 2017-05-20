//	ItemSetup.cpp
//	Guy Simmons, 24th August 1998.

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
#include	"ticklist.h"


//---------------------------------------------------------------

SLONG	item_count,item_type,item_flags;//,item_container;

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


BOOL	CALLBACK	is_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;


	switch(message)
	{
		case	WM_INITDIALOG:
/*			//	Set up the 'item' combo.
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			lbitem_str	=	witem_strings[0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	witem_strings[++c0];
			}
			//	Set its default item.
			SendMessage(the_ctrl,CB_SETCURSEL,item_type-1,0);*/
			INIT_COMBO_BOX(IDC_COMBO1,witem_strings,item_type-1);
//			INIT_COMBO_BOX(IDC_COMBO2,witemcontainer_strings,item_container);
			ticklist_init(hWnd, IDC_LIST1, witem_flag_strings,item_flags);

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
							MAKELONG(item_count,0)
						);
			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
				//	Get the 'type'.
				item_type	=	SendMessage	(	GetDlgItem(hWnd,IDC_COMBO1),
												CB_GETCURSEL,
												0,0
											) + 1;
				item_count	=	SendMessage (	GetDlgItem(hWnd,IDC_SPIN1),
												UDM_GETPOS,
												0,0
											);
/*				item_container= SendMessage	(	GetDlgItem(hWnd,IDC_COMBO2),
												CB_GETCURSEL,
												0,0
											);*/
				item_flags = ticklist_bitmask(hWnd,IDC_LIST1);

				case	IDCANCEL:
					ticklist_close(hWnd, IDC_LIST1);
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;

		case WM_MEASUREITEM:
			return ticklist_measure(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			return ticklist_draw(hWnd, wParam, lParam);

		case	WM_VSCROLL:
			if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN1 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				item_count			=	HIWORD(wParam);
				return	TRUE;
			}
			break;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_item_setup(EventPoint *the_ep)
{
	item_type	=	the_ep->Data[0];
	item_count	=	the_ep->Data[1];
	item_flags  = the_ep->Data[2];
	if(item_count==0)
		item_count	=	1;

	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_ITEM_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)is_proc
				);

	the_ep->Data[0]	=	item_type;
	the_ep->Data[1]	=	item_count;
	the_ep->Data[2]	=	item_flags;
}

//---------------------------------------------------------------

CBYTE	*get_item_message(EventPoint *ep, CBYTE *msg) {
	if ((!ep)||!ep->Data[0]) 
		strcpy(msg,"Unknown");
	else {
/*		if (ep->Data[2])
			sprintf(msg,"%s (in a %s)",witem_strings[ep->Data[0]-1],witemcontainer_strings[ep->Data[2]]);
		else*/
			strcpy(msg,witem_strings[ep->Data[0]-1]);
	}
	return msg;
}

