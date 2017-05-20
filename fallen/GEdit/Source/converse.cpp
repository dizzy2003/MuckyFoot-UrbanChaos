//	converseSetup.cpp
//	Matthew Rosenfeld, some time in March 1998.

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

CBYTE		*converse_text;
SLONG		 converse_p1, converse_p2;
SLONG        converse_grab_camera;

extern CBYTE *WaypointExtra(EventPoint *ep, CBYTE *msg);

//---------------------------------------------------------------

#define STR_LEN 800

BOOL	CALLBACK	cv_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG len;
	EventPoint	*ep_ptr, *ep_base=current_mission->EventPoints;
	CBYTE		msg[STR_LEN],str[STR_LEN];
	HWND		the_ctrl, the_ctrl2;
	SLONG		ep;
	SLONG		c0	=	0;

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
							0,(LPARAM)converse_text
						);

			// fill the two list boxes
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			the_ctrl2	=	GetDlgItem(hWnd,IDC_COMBO2);
			ep	=	current_mission->UsedEPoints;
			c0  =   0;
			while (ep) {
				ep_ptr	=	TO_EVENTPOINT(ep_base,ep);
				if ((ep_ptr->WaypointType==WPT_CREATE_PLAYER)||(ep_ptr->WaypointType==WPT_CREATE_ENEMIES)) {
					WaypointExtra(ep_ptr,msg);
					sprintf(str,"%d%c: %s",ep,'A' + ep_ptr->Group,msg);
					SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);
					SendMessage(the_ctrl2,CB_ADDSTRING,0,(LPARAM)str);
					if (ep==converse_p1) SendMessage(the_ctrl,CB_SETCURSEL,c0,0);
					if (ep==converse_p2) SendMessage(the_ctrl2,CB_SETCURSEL,c0,0);
					c0++;
				}
				ep =	ep_ptr->Next;
			}

			SetFocus(GetDlgItem(hWnd,IDC_EDIT1));

			CheckDlgButton(hWnd, IDC_GRAB_CAMERA, converse_grab_camera);

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
			if (converse_text) free(converse_text);
			converse_text =	(CBYTE*)malloc(len);
			ZeroMemory(converse_text,len);
			
			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_GETTEXT,
							len,
							(LPARAM)converse_text
						);

			converse_p1 =	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO1),
											CB_GETCURSEL,
											0,0
										);
			converse_p2 =	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO2),
											CB_GETCURSEL,
											0,0
										);
			// now translate phoney people indices to real one
			if (converse_p1==-1) {
				converse_p1=0;
			} else {
				memset(msg,0,STR_LEN);
				SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETLBTEXT,converse_p1,(long)msg);
				sscanf(msg,"%d",&converse_p1);
			}
			if (converse_p2==-1) {
				converse_p2=0;
			} else {
				memset(msg,0,STR_LEN);
				SendMessage(GetDlgItem(hWnd,IDC_COMBO2),CB_GETLBTEXT,converse_p2,(long)msg);
				sscanf(msg,"%d",&converse_p2);
			}

			converse_grab_camera = IsDlgButtonChecked(hWnd, IDC_GRAB_CAMERA);

			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_converse_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	converse_text	=	(CBYTE*)the_ep->Data[0];
	converse_p1  	=	the_ep->Data[1];
	converse_p2  	=	the_ep->Data[2];
	converse_grab_camera = the_ep->Data[3];
	if(!converse_text)
	{
		converse_text	=	(CBYTE*)malloc(STR_LEN);
		ZeroMemory(converse_text,STR_LEN);
		SetEPTextID(the_ep);
	}

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_CONVERSATION_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)cv_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	(SLONG)converse_text;	//	
	the_ep->Data[1]		=	converse_p1;
	the_ep->Data[2]		=	converse_p2;
	the_ep->Data[3]		=	converse_grab_camera;
}

//---------------------------------------------------------------

/*
CBYTE	*get_message_message(EventPoint *ep, CBYTE *msg) {
	msg[0]=0;
	if (ep&&ep->Data[0])
		strcpy(msg,(CBYTE*)ep->Data[0]);
	return msg;
}
*/