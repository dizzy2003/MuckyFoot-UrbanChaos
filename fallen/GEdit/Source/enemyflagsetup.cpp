//	EnemySetup.cpp
//	Guy Simmons, 23rd August 1998.

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

SLONG			enemyf_flags,
				enemyf_to_change;

extern CBYTE *WaypointExtra(EventPoint *ep, CBYTE *msg);
#define STR_LEN 800

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



BOOL	CALLBACK	efs_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		ep;
	SLONG		c0	=	0;
	HWND		the_ctrl;
	EventPoint	*ep_ptr, *ep_base=current_mission->EventPoints;
	CBYTE		msg[STR_LEN],str[STR_LEN];


	switch(message)
	{
		case	WM_INITDIALOG:
		{

			// fill the list box
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			ep	=	current_mission->UsedEPoints;
			c0  =   0;
			while (ep) {
				ep_ptr	=	TO_EVENTPOINT(ep_base,ep);
				if (ep_ptr->WaypointType==WPT_CREATE_ENEMIES) {
					WaypointExtra(ep_ptr,msg);
					sprintf(str,"%d%c: %s",ep,'A' + ep_ptr->Group,msg);
					SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);
					if (ep==enemyf_to_change) SendMessage(the_ctrl,CB_SETCURSEL,c0,0);
					c0++;
				}
				ep =	ep_ptr->Next;
			}

			//  Subclass and init the listbox
			ticklist_init(hWnd, IDC_LIST1, wenemy_flag_strings,enemyf_flags);
	
			return	TRUE;
		}

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case WM_MEASUREITEM:
			return ticklist_measure(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			return ticklist_draw(hWnd, wParam, lParam);


		case	WM_CLOSE:
			enemyf_flags = ticklist_bitmask(hWnd,IDC_LIST1);

			enemyf_to_change	=	SendMessage (	GetDlgItem(hWnd,IDC_COMBO1),
											CB_GETCURSEL,
											0,0
										);
			// now translate phoney people indices to real one
			if (enemyf_to_change==-1) {
				enemyf_to_change=0;
			} else {
				memset(msg,0,STR_LEN);
				SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETLBTEXT,enemyf_to_change,(long)msg);
				sscanf(msg,"%d",&enemyf_to_change);
			}

			ticklist_close(hWnd, IDC_LIST1);

			EndDialog(hWnd,0);

			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_enemy_flags_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	enemyf_to_change	=	the_ep->Data[0];
	enemyf_flags		=	the_ep->Data[1];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_ENEMYFLAGS_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)efs_proc
				);

	//	Set the data.
	the_ep->Data[0]	=	enemyf_to_change;
	the_ep->Data[1]	=	enemyf_flags;
}

//---------------------------------------------------------------

