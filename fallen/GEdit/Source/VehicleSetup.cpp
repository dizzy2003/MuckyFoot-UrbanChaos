//	VehicleSetup.cpp
//	Guy Simmons, 3rd September 1998.

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

SLONG	veh_type, veh_move, veh_targ, veh_key;

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

extern CBYTE *WaypointTitle(EventPoint *ep, CBYTE *msg);
CBYTE *WaypointExtra(EventPoint *ep, CBYTE *msg);


BOOL	CALLBACK	vs_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	BOOL		en;
	SLONG		ep;
	EventPoint	*ep_ptr, *ep_base=current_mission->EventPoints;
	CBYTE		msg[_MAX_PATH],str[_MAX_PATH];

	
	switch(message)
	{
		case	WM_INITDIALOG:
			INIT_COMBO_BOX(IDC_COMBO1,wvehicle_strings,veh_type-1);
			INIT_COMBO_BOX(IDC_COMBO2,wvehicle_behaviour_strings,veh_move);
			INIT_COMBO_BOX(IDC_COMBO4,wvehicle_key_strings,veh_key);
			PostMessage(hWnd,WM_COMMAND,MAKELONG(IDC_COMBO2,CBN_SELCHANGE),(LPARAM)the_ctrl);

			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO3);
			ep	=	current_mission->UsedEPoints;
			c0  =   0;
			while (ep) {
				ep_ptr	=	TO_EVENTPOINT(ep_base,ep);
				if ((ep_ptr->WaypointType==WPT_CREATE_PLAYER)||(ep_ptr->WaypointType==WPT_CREATE_ENEMIES)) {
					WaypointExtra(ep_ptr,msg);
					sprintf(str,"%d%c: %s",ep,'A' + ep_ptr->Group,msg);
					SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);
					if (ep==veh_targ) SendMessage(the_ctrl,CB_SETCURSEL,c0,0);
					c0++;
				}
				ep =	ep_ptr->Next;
			}

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
				case	IDC_COMBO2:
					c0=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
					en=(c0==3);
					EnableWindow(GetDlgItem(hWnd,IDC_COMBO3),en);
					EnableWindow(GetDlgItem(hWnd,IDC_LABEL1),en);
					return TRUE;
			}
			break;

		case	WM_CLOSE:
			//	Get the 'type'.
			veh_type	=	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO1),
											CB_GETCURSEL,
											0,0
										)	+	1;
			veh_move	=	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO2),
											CB_GETCURSEL,
											0,0
										);
			veh_targ	=	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO3),
											CB_GETCURSEL,
											0,0
										);
			veh_key 	=	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO4),
											CB_GETCURSEL,
											0,0
										);
			// now translate phoney veh_targ to real one
			if (veh_targ==-1) {
				veh_targ=0;
			} else {
				memset(msg,0,_MAX_PATH);
				SendMessage(GetDlgItem(hWnd,IDC_COMBO3),CB_GETLBTEXT,veh_targ,(long)msg);
				sscanf(msg,"%d",&veh_targ);
			}

			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_vehicle_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	veh_type		=	the_ep->Data[0];
	veh_move		=	the_ep->Data[1];
	veh_targ		=	the_ep->Data[2];
	veh_key 		=	the_ep->Data[3];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_VEHICLE_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)vs_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	veh_type;
	the_ep->Data[1]		=	veh_move;
	the_ep->Data[2]		=	veh_targ;
	the_ep->Data[3]		=	veh_key;
}

//---------------------------------------------------------------

//---------------------------------------------------------------

CBYTE	*get_vehicle_message(EventPoint *ep, CBYTE *msg) {
	if ((!ep)||(!ep->Data[0])) 
		strcpy(msg,"Unknown");
	else
		strcpy(msg,wvehicle_strings[ep->Data[0]-1]);
	return msg;
}


