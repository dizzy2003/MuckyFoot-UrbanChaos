//	CamTargetSetup.cpp
//	Matthew Rosenfeld, 1st October 1998.

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

SLONG		target_type,target_move,target_speed,target_delay,camera_zoom,camera_rotate;

//---------------------------------------------------------------

BOOL	CALLBACK	camts_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	SLONG		c0	=	0;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set the current player type.
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			lbitem_str	=	wcammove_strings[0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	wcammove_strings[++c0];
			}
			SendMessage(the_ctrl,CB_SETCURSEL,target_move-1,0);
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO2);
			lbitem_str	=	wcamtarg_strings[c0=0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	wcamtarg_strings[++c0];
			}
			SendMessage(the_ctrl,CB_SETCURSEL,target_type-1,0);
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,target_speed);
			SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_SETPOS,0,target_delay);
			SendMessage(GetDlgItem(hWnd,IDC_SPIN3),UDM_SETPOS,0,camera_zoom);
			SendMessage(GetDlgItem(hWnd,IDC_CHECK1),BM_SETCHECK,camera_rotate,0);
			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					target_move  = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0)+1;
					target_type  = SendMessage(GetDlgItem(hWnd,IDC_COMBO2),CB_GETCURSEL,0,0)+1;
					target_speed = SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
					target_delay = SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_GETPOS,0,0);
					camera_zoom  = SendMessage(GetDlgItem(hWnd,IDC_SPIN3),UDM_GETPOS,0,0);
					camera_rotate= SendMessage(GetDlgItem(hWnd,IDC_CHECK1),BM_GETCHECK,0,0);
				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_CLOSE:
//			player_type	=	(SendMessage(GetDlgItem(hWnd,IDC_RADIO1),BM_GETCHECK,0,0) ? PT_DARCI : PT_ROPER);
			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_camtarget_setup(EventPoint *the_ep)
{
	target_move  =	the_ep->Data[0];
	target_type  =	the_ep->Data[1];
	target_speed =	the_ep->Data[2];
	target_delay =	the_ep->Data[3];
	camera_zoom  =	the_ep->Data[4];
	camera_rotate=	the_ep->Data[5];
	if (target_move==0) target_move=1;
	if (target_type==0) target_type=1;
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_CAMERA_TARGET_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)camts_proc
				);

	the_ep->Data[0]	=	target_move;
	the_ep->Data[1]	=	target_type;
	the_ep->Data[2]	=	target_speed;
	the_ep->Data[3]	=	target_delay;
	the_ep->Data[4]	=	camera_zoom;
	the_ep->Data[5]	=	camera_rotate;
}

//---------------------------------------------------------------

CBYTE	*get_camtarget_message(EventPoint *ep, CBYTE *msg) {
  if ((!ep)||(!ep->Data[0])||(!ep->Data[1]))
	  strcpy(msg,"Unknown");
  else
	sprintf(msg,"%s %s",wcammove_strings[ep->Data[0]-1],wcamtarg_strings[ep->Data[1]-1]);
  return msg;
}
