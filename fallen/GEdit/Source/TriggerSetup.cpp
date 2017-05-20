//	TriggerSetup.cpp
//	Guy Simmons, 27th August 1998.

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

SLONG		action_off,
			action_on,
			triggered_by,
			trigger_radius,
			trigger_depend,
			trigger_type;			
EventPoint	*current_ep;

//---------------------------------------------------------------

void	update_action_combos(HWND parent)
{
	SLONG		c0;

/*obsolete
	if(trigger_type==TT_NONE || trigger_type==TT_NORMAL)
	{
		EnableWindow(GetDlgItem(parent,IDC_COMBO3),FALSE);
		EnableWindow(GetDlgItem(parent,IDC_COMBO4),FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(parent,IDC_COMBO3),TRUE);
		EnableWindow(GetDlgItem(parent,IDC_COMBO4),TRUE);
	}
	*/
}

//---------------------------------------------------------------

void	update_trigger_radios(HWND parent)
{
/*
	if(SendMessage(GetDlgItem(parent,IDC_RADIO1),BM_GETCHECK,0,0)==BST_CHECKED)
	{
		triggered_by	=	TB_PROXIMITY;
		EnableWindow(GetDlgItem(parent,IDC_EDIT2),TRUE);
		EnableWindow(GetDlgItem(parent,IDC_EDIT3),FALSE);
		SendMessage	(
						GetDlgItem(parent,IDC_SPIN2),
						UDM_SETPOS,
						0,
						MAKELONG(trigger_radius,0)
					);
	}
	else
	{
		triggered_by	=	TB_DEPENDENCY;
		EnableWindow(GetDlgItem(parent,IDC_EDIT2),FALSE);
		EnableWindow(GetDlgItem(parent,IDC_EDIT3),TRUE);
		SendMessage	(
						GetDlgItem(parent,IDC_SPIN3),
						UDM_SETPOS,
						0,
						MAKELONG(trigger_depend,0)
					);
		if(!trigger_depend)
		{
			SendMessage	(
							GetDlgItem(parent,IDC_EDIT3),
							WM_SETTEXT,
							0,(LPARAM)"None"
						);
		}
	}
*/
}

//---------------------------------------------------------------

void	process_view_wind(void);

BOOL	CALLBACK	ts_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	NM_UPDOWN	*lp_ntfy;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the 'Trigger Type' combo.
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			lbitem_str	=	wtrigger_strings[0];
			while(*lbitem_str!='!')
			{
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);
				lbitem_str	=	wtrigger_strings[++c0];
			}
			//	Set its default item.
			if (trigger_type<1) trigger_type=1;
			SendMessage(the_ctrl,CB_SETCURSEL,trigger_type-1,0);

			//	Set up the radius & dependency spins.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETRANGE,
							0,
							MAKELONG(2560,0)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN3),
							UDM_SETRANGE,
							0,
							MAKELONG(511,0)
						);
/*
			//	Set up the radio buttons & the trigger data.
			switch(triggered_by)
			{
				case	TB_NONE:
					//	Default to proximity.
					triggered_by			=	TB_PROXIMITY;
					current_ep->TriggeredBy	=	triggered_by;
					SendMessage(GetDlgItem(hWnd,IDC_RADIO1),BM_SETCHECK,TRUE,0);
					trigger_radius	=	0;
					break;
				case	TB_PROXIMITY:
					SendMessage(GetDlgItem(hWnd,IDC_RADIO1),BM_SETCHECK,TRUE,0);
					SendMessage	(
									GetDlgItem(hWnd,IDC_SPIN2),
									UDM_SETPOS,
									0,
									MAKELONG(trigger_radius,0)
								);
					SendMessage	(
									GetDlgItem(hWnd,IDC_SPIN3),
									UDM_SETPOS,
									0,
									MAKELONG(0,0)
								);
					break;
				case	TB_DEPENDENCY:
					SendMessage(GetDlgItem(hWnd,IDC_RADIO2),BM_SETCHECK,TRUE,0);
					SendMessage	(
									GetDlgItem(hWnd,IDC_SPIN2),
									UDM_SETPOS,
									0,
									MAKELONG(0,0)
								);
					SendMessage	(
									GetDlgItem(hWnd,IDC_SPIN3),
									UDM_SETPOS,
									0,
									MAKELONG(trigger_depend,0)
								);
					break;
			}
*/
			//	Set up the time spin.


			update_action_combos(hWnd);
			update_trigger_radios(hWnd);

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
//					SendMessage(hWnd,WM_CLOSE,0,0);
					EndDialog(hWnd,IDOK);
					return	TRUE;

				case	IDC_COMBO1:
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						//	Get the 'Trigger Type'.
						trigger_type	=	SendMessage	(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0)	+	1;
						update_action_combos(hWnd);
						return	TRUE;
					}
					break;

				case	IDC_RADIO1:
				case	IDC_RADIO2:
					update_trigger_radios(hWnd);
					return	TRUE;
			}
			break;

		case	WM_NOTIFY:
			lp_ntfy	=	(NM_UPDOWN*)lParam;

			//	Make the 'radius' spin go up/down in steps of 32.
			if(lp_ntfy->hdr.idFrom==IDC_SPIN2 && lp_ntfy->hdr.code==UDN_DELTAPOS)
			{
				SendMessage	(
								lp_ntfy->hdr.hwndFrom,
								UDM_SETPOS,
								0,
								MAKELONG(lp_ntfy->iPos+(lp_ntfy->iDelta*31),0)
							);
				return	TRUE;
			}
			break;
/*
		case	WM_VSCROLL:
			//	Set up the 'radius', 'dependency' or 'time'.
			if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN1 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				trigger_data1	=	HIWORD(wParam);
				return	TRUE;
			}
			else if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN2 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				trigger_radius		=	HIWORD(wParam);
				current_ep->Radius	=	trigger_radius;
				process_view_wind();
				SendMessage(GEDIT_edit_wnd,WM_PAINT,0,0);
				return	TRUE;
			}
			else if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN3 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				trigger_depend		=	HIWORD(wParam);
				current_ep->EPRef	=	trigger_depend;
				if(!trigger_depend)
					SendMessage	(
									GetDlgItem(hWnd,IDC_EDIT3),
									WM_SETTEXT,
									0,(LPARAM)"None"
								);
//				process_view_wind();
//				SendMessage(GEDIT_edit_wnd,WM_PAINT,0,0);
				return	TRUE;
			}
			break;
*/
		case	WM_CLOSE:
			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_trigger_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	triggered_by	=	the_ep->TriggeredBy;
	trigger_radius	=	the_ep->Radius;
	trigger_depend	=	the_ep->EPRef;
	trigger_type	=	the_ep->Data[0];

	current_ep		=	the_ep;

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_TRIGGER_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)ts_proc
				);

	//	Get the data.
	the_ep->TriggeredBy	=	triggered_by;
	the_ep->EPRef		=	trigger_depend;	//	Dependency.
	the_ep->Radius		=	trigger_radius;	//	Radius.
	the_ep->Data[0]		=	trigger_type;
}

//---------------------------------------------------------------

CBYTE	*get_trigger_message(EventPoint *ep, CBYTE *msg) {
	if ((!ep)||(!ep->Data[0])) 
		strcpy(msg,"Unknown");
	else
		strcpy(msg,wtrigger_strings[ep->Data[0]-1]);
	return msg;
}

