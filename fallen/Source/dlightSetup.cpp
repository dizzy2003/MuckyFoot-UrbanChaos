//	TrapSetup.cpp
//	Matthew Rosenfeld, 15th October 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"inline.h"
#include	"gi.h"
#include	"ticklist.h"
#include	"EdStrings.h"
#include	"GEdit.h"


//---------------------------------------------------------------

SLONG	lite_type, lite_speed, lite_steps, lite_mask, lite_rgbA, lite_rgbB;

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


static void InitSteps(HWND hWnd,CBYTE steps,SLONG mask) {
	CBYTE i,j;
	SLONG		c0			=	1;
	HWND		the_ctrl	=	GetDlgItem(hWnd,IDC_LIST1);
	CBYTE		lbitem_str[300];
	SendMessage(the_ctrl,LB_RESETCONTENT,0,0);
	for (i=0;i<steps;i++) {
		j=i+1;
		sprintf(lbitem_str,"Step %d",j);
		SendMessage(the_ctrl,LB_ADDSTRING,0,(LPARAM)lbitem_str);	
		SendMessage(the_ctrl,LB_SETITEMDATA,i,(mask & (1<<(i))) ? 1 : 0);  
	}
}

static CBYTE *blank_string[] = { "!" };

BOOL	CALLBACK	lite_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	NM_UPDOWN	*lp_ntfy;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the combo box.
			INIT_COMBO_BOX(IDC_COMBO1,wlitetype_strings,lite_type);
//			INIT_COMBO_BOX(IDC_COMBO2,wtrapaxis_strings,trap_axis);

			ticklist_init(hWnd, IDC_LIST1,blank_string,0);

			InitSteps(hWnd,lite_steps,lite_mask);

			//	Set up the 'speed' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(32,1)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0,
							MAKELONG(lite_speed,0)
						);

			//	Set up the 'steps' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETRANGE,
							0,
							MAKELONG(32,1)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETPOS,
							0,
							MAKELONG(lite_steps,0)
						);

/*			//	Set up the 'range' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN3),
							UDM_SETRANGE,
							0,
							MAKELONG(32,1)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN3),
							UDM_SETPOS,
							0,
							MAKELONG(trap_range,0)
						);*/

			return	TRUE;

		case WM_MEASUREITEM:
			return ticklist_measure(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			switch(LOWORD(wParam)) {
			case IDC_BUTTON1:
			case IDC_BUTTON2:
			{
				HWND ctl = GetDlgItem(hWnd,wParam);
				LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT) lParam;
//				CBYTE pc[255];
				HBRUSH brs;
				HPEN pen;
				HPEN open;
				SLONG rgb;
				BOOL pushed;
				RECT rc;

				rc=item->rcItem; rc.left+=2; rc.top+=2; rc.bottom-=2; rc.right-=2;

				rgb=(item->itemState & ODS_DEFAULT) ? COLOR_BTNTEXT : COLOR_BTNFACE;
				brs=CreateSolidBrush(GetSysColor(rgb));
				FrameRect(item->hDC,&item->rcItem,brs);
				DeleteObject(brs);

				rgb=(LOWORD(wParam)==IDC_BUTTON1) ? lite_rgbA : lite_rgbB;

				brs=CreateSolidBrush(rgb);
				FillRect(item->hDC, &rc, brs);
				DeleteObject(brs);

				pushed=(SendMessage(ctl,BM_GETSTATE,0,0)&BST_PUSHED);
				
				rgb = pushed ? COLOR_BTNSHADOW : COLOR_BTNHIGHLIGHT;
				pen=CreatePen(PS_SOLID,0,GetSysColor(rgb));
				open=(HPEN)SelectObject(item->hDC,pen);
				MoveToEx(item->hDC,item->rcItem.left+1,item->rcItem.bottom-3,NULL);
				LineTo(item->hDC,item->rcItem.left+1,item->rcItem.top+1);
				LineTo(item->hDC,item->rcItem.right-3,item->rcItem.top+1);
				SelectObject(item->hDC,open);
				DeleteObject(pen);

				rgb = pushed ? COLOR_BTNHIGHLIGHT : COLOR_BTNSHADOW;
				pen=CreatePen(PS_SOLID,0,GetSysColor(rgb));
				open=(HPEN)SelectObject(item->hDC,pen);
				MoveToEx(item->hDC,item->rcItem.right-2,item->rcItem.top+2,NULL);
				LineTo(item->hDC,item->rcItem.right-2,item->rcItem.bottom-2);
				LineTo(item->hDC,item->rcItem.left+2,item->rcItem.bottom-2);
				SelectObject(item->hDC,open);
				DeleteObject(pen);
			
				if (item->itemState & ODS_FOCUS) 
					DrawFocusRect(item->hDC,&item->rcItem);

				return TRUE;
			}
			default:
				return ticklist_draw(hWnd, wParam, lParam);
			}

		case	WM_NOTIFY:
			lp_ntfy	=	(NM_UPDOWN*)lParam;

			if(lp_ntfy->hdr.idFrom==IDC_SPIN2 && lp_ntfy->hdr.code==UDN_DELTAPOS) {
				InitSteps(hWnd,lp_ntfy->iPos+lp_ntfy->iDelta,ticklist_bitmask(hWnd,IDC_LIST1));
			}
			break;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDC_COMBO1:
				if(HIWORD(wParam)==CBN_SELCHANGE)
				{
//					EnableWindow(GetDlgItem(hWnd,IDC_SPIN
				}
				break;
				case	IDC_BUTTON1:
				case	IDC_BUTTON2:
				{
					CHOOSECOLOR choosecol;
					SLONG		*rgb;
					rgb=(LOWORD(wParam)==IDC_BUTTON1) ? &lite_rgbA : &lite_rgbB;
					choosecol.lStructSize=sizeof(choosecol);
					choosecol.hwndOwner=hWnd;
					choosecol.hInstance=NULL;
					choosecol.rgbResult=*rgb;
					choosecol.Flags=CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&choosecol))
						*rgb=choosecol.rgbResult;
				}
				break;
				case	IDOK:
					lite_type =	SendMessage	(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
//					lite_axis =	SendMessage	(GetDlgItem(hWnd,IDC_COMBO2),CB_GETCURSEL,0,0);
					lite_speed= SendMessage (GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);
					lite_steps= SendMessage (GetDlgItem(hWnd,IDC_SPIN2),UDM_GETPOS,0,0);
					lite_mask = ticklist_bitmask(hWnd,IDC_LIST1);
//					lite_range= SendMessage (GetDlgItem(hWnd,IDC_SPIN3),UDM_GETPOS,0,0);
				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;

			}
			break;

		case	WM_CLOSE:
			ticklist_close(hWnd, IDC_LIST1);
			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_lite_setup(EventPoint *the_ep)
{

	if (!the_ep) return;
	//	Set the dialog.
	lite_type			=	the_ep->Data[0];
	lite_speed			=	the_ep->Data[1];
	lite_steps			=	the_ep->Data[2];
	lite_mask			=	the_ep->Data[3];
	lite_rgbA			=	the_ep->Data[4];
	lite_rgbB 			=	the_ep->Data[5];
	if (!lite_steps) lite_steps=1;

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_DLIGHT_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)lite_proc
				);

	//	Set the data.
	the_ep->Data[0]	=	lite_type;
	the_ep->Data[1]	=	lite_speed;
	the_ep->Data[2]	=	lite_steps;
	the_ep->Data[3]	=	lite_mask;
	the_ep->Data[4]	=	lite_rgbA;
	the_ep->Data[5]	=	lite_rgbB;
}

//---------------------------------------------------------------

CBYTE	*get_lite_message(EventPoint *ep, CBYTE *msg) {
	if (!ep)
		strcpy(msg,"Unknown");
	else
		strcpy(msg,wlitetype_strings[ep->Data[0]]);
	return msg;
}


