//	PlayerSetup.cpp
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
#include	"edstrings.h"
#include	"GEdit.h"


//---------------------------------------------------------------

SLONG		player_type, player_equip;

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

BOOL	CALLBACK	ps_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;
	SLONG		c0	=	0;
	LPTSTR		lbitem_str;
	NM_UPDOWN	*lp_ntfy;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set the current player type.
			INIT_COMBO_BOX(IDC_COMBO1,wplayer_strings,player_type-PT_DARCI);
//			the_ctrl	=	GetDlgItem(hWnd,IDC_RADIO1+(player_type-PT_DARCI));
//			SendMessage(the_ctrl,BM_SETCHECK,1,0);
			SendMessage(GetDlgItem(hWnd,IDC_CHECK1),BM_SETCHECK,player_equip,0);
			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_CLOSE:
//			player_type	=	(SendMessage(GetDlgItem(hWnd,IDC_RADIO1),BM_GETCHECK,0,0) ? PT_DARCI : PT_ROPER);
			player_type	=	SendMessage	(	GetDlgItem(hWnd,IDC_COMBO1),
											CB_GETCURSEL,
											0,0
										)	+	PT_DARCI;
			player_equip=	SendMessage(GetDlgItem(hWnd,IDC_CHECK1),BM_GETCHECK,0,0);
			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_player_setup(EventPoint *the_ep)
{
	player_type	=	the_ep->Data[0];
	player_equip=	the_ep->Data[1];
	if(player_type==PT_NONE)
		player_type	=	PT_DARCI;	//	Default to Darci.

	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_PLAYER_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)ps_proc
				);

	the_ep->Data[0]	=	player_type;
	the_ep->Data[1]	=	player_equip;
}

//---------------------------------------------------------------

CBYTE	*get_player_message(EventPoint *ep, CBYTE *msg) {
	strcpy(msg,"Unknown");
	if (ep) {
		strcpy(msg,wplayer_strings[ep->Data[0]-PT_DARCI]);
/*  		switch(ep->Data[0]) {
			case PT_ROPER:
				strcpy(msg,"Roper");
				break;
			case PT_DARCI:
				strcpy(msg,"Darci");
				break;

		}*/
	}
    return msg;
}