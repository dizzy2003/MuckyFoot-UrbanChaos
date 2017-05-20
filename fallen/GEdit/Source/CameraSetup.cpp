//	CameraSetup.cpp
//	Matthew Rosenfeld, 30th September 1998.

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

SLONG		camera_type,camera_move,camera_speed,camera_delay,camera_freeze,camera_lock,camera_cant_interrupt;
EventPoint  *use_me_to_debug;

//---------------------------------------------------------------

BOOL	CALLBACK	cams_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	SLONG		c0	=	0;


	switch(message)
	{
		case	WM_INITDIALOG:


			//	Set up the 'waypoint' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(250,1)
						);


			//	Set the current player type.
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			lbitem_str	=	wcammove_strings[0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	wcammove_strings[++c0];
			}
			SendMessage(the_ctrl,CB_SETCURSEL,camera_move-1,0);
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO2);
			lbitem_str	=	wcamtype_strings[c0=0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	wcamtype_strings[++c0];
			}
			SendMessage(the_ctrl,CB_SETCURSEL,camera_type-1,0);
			SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_SETPOS,0,camera_speed);
			SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_SETPOS,0,camera_delay);
			CheckDlgButton(hWnd,IDC_CHECK1,camera_freeze);
			CheckDlgButton(hWnd,IDC_LOCK_DIRECTION,camera_lock);
			CheckDlgButton(hWnd,IDC_CANT_INTERRUPT,camera_cant_interrupt);

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					camera_move= SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0)+1;
					camera_type= SendMessage(GetDlgItem(hWnd,IDC_COMBO2),CB_GETCURSEL,0,0)+1;
					camera_speed=SendMessage(GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
					camera_delay=SendMessage(GetDlgItem(hWnd,IDC_SPIN2),UDM_GETPOS,0,0);
					camera_freeze=IsDlgButtonChecked(hWnd,IDC_CHECK1);
					camera_lock=IsDlgButtonChecked(hWnd,IDC_LOCK_DIRECTION);
					camera_cant_interrupt=IsDlgButtonChecked(hWnd,IDC_CANT_INTERRUPT);
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

void	do_camera_setup(EventPoint *the_ep)
{
	use_me_to_debug=the_ep;
	camera_move           = the_ep->Data[0];
	camera_type           = the_ep->Data[1];
	camera_speed          = the_ep->Data[2];
	camera_delay          = the_ep->Data[3];
	camera_freeze         = the_ep->Data[4];
	camera_lock           = the_ep->Data[5];
	camera_cant_interrupt = the_ep->Data[6];

	if (camera_move==0) camera_move=1;
	if (camera_type==0) camera_type=1;
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_CAMERA_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)cams_proc
				);

	the_ep->Data[0]	=	camera_move;
	the_ep->Data[1]	=	camera_type;
	the_ep->Data[2]	=	camera_speed;
	the_ep->Data[3]	=	camera_delay;
	the_ep->Data[4]	=	camera_freeze;
	the_ep->Data[5] =	camera_lock;
	the_ep->Data[6] =   camera_cant_interrupt;
}

//---------------------------------------------------------------

CBYTE	*get_camera_message(EventPoint *ep, CBYTE *msg) {
  if ((!ep)||(!ep->Data[0])||(!ep->Data[1]))
	  strcpy(msg,"Unknown");
  else
	sprintf(msg,"%s %s",wcammove_strings[ep->Data[0]-1],wcamtype_strings[ep->Data[1]-1]);
  return msg;
}
