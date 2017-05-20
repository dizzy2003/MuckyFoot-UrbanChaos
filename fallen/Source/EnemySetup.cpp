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


// THIS IS A NASTY COPY OF THE DEFINES IN pcom.cpp!

#define PCOM_COMBAT_SLIDE          (1 << 0)
#define PCOM_COMBAT_COMBO_PPP      (1 << 1)
#define PCOM_COMBAT_COMBO_KKK      (1 << 2)
#define PCOM_COMBAT_COMBO_ANY      (1 << 3)
#define PCOM_COMBAT_GRAPPLE_ATTACK (1 << 4)
#define PCOM_COMBAT_SIDE_KICK	   (1 << 5)
#define PCOM_COMBAT_BACK_KICK      (1 << 6)


//---------------------------------------------------------------

SLONG			enemy_count,
				enemy_constitution,
				enemy_type,
				enemy_ai,
				enemy_move,
				enemy_flags,
				enemy_to_change,
				enemy_guard,
				enemy_weaps,
				enemy_follow,
				enemy_items,
				enemy_has,
				enemy_combat;

BOOL			adjust;

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



BOOL	CALLBACK	es_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	NM_UPDOWN	*lp_ntfy;

	CBYTE		msg[800];


	switch(message)
	{
		case	WM_INITDIALOG:
		{

			if (adjust) {
				strcpy(msg,"Adjust Enemy");
				SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)msg);
				ShowWindow(GetDlgItem(hWnd,IDC_STATIC_ADJUST),SW_SHOW);
				ShowWindow(GetDlgItem(hWnd,IDC_EDIT3),SW_SHOW);
				ShowWindow(GetDlgItem(hWnd,IDC_SPIN3),SW_SHOW);
				//	Set up the 'target' spin.
				SendMessage	(
								GetDlgItem(hWnd,IDC_SPIN3),
								UDM_SETRANGE,
								0,
								MAKELONG(1000,0)
							);
				SendMessage	(
								GetDlgItem(hWnd,IDC_SPIN3),
								UDM_SETPOS,
								0,
								MAKELONG(enemy_to_change,0)
							);
			}

			//	Set up the combo box.
			INIT_COMBO_BOX(IDC_COMBO1,wenemy_strings,enemy_type-1);
			INIT_COMBO_BOX(IDC_COMBO2,wenemy_move_strings,enemy_move);
			PostMessage(hWnd,WM_COMMAND,MAKELONG(IDC_COMBO2,CBN_SELCHANGE),(LPARAM)the_ctrl);
			INIT_COMBO_BOX(IDC_COMBO4,wenemy_ai_strings,LOWORD(enemy_ai));
			PostMessage(hWnd,WM_COMMAND,MAKELONG(IDC_COMBO4,CBN_SELCHANGE),(LPARAM)the_ctrl);

			INIT_COMBO_BOX(IDC_COMBO3,wenemy_ability_strings,HIWORD(enemy_ai));

			//  Subclass and init the listbox
//			TICKLIST_INIT(IDC_LIST3,wenemy_flag_strings,enemy_flags);
			ticklist_init(hWnd, IDC_LIST3, wenemy_flag_strings,enemy_flags);
			ticklist_init(hWnd, IDC_LIST1, wweaponitem_strings,enemy_weaps);
			ticklist_init(hWnd, IDC_LIST2, wotheritem_strings,enemy_items);
	

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
							MAKELONG(enemy_count,0)
						);

			//	Set up the 'constitution' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETRANGE,
							0,
							MAKELONG(100,0)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETPOS,
							0,
							MAKELONG(enemy_constitution,0)
						);
			//	Set up the 'guard' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN4),
							UDM_SETRANGE,
							0,
							MAKELONG(1000,0)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN4),
							UDM_SETPOS,
							0,
							MAKELONG(enemy_guard,0)
						);
			//	Set up the 'follow' spin.
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN5),
							UDM_SETRANGE,
							0,
							MAKELONG(1000,0)
						);
			SendMessage	(
							GetDlgItem(hWnd,IDC_SPIN5),
							UDM_SETPOS,
							0,
							MAKELONG(enemy_follow,0)
						);

			//
			// Set up the 'has' buttons.
			// 

			CheckDlgButton(hWnd, IDC_HAS_PISTOL,  (enemy_has & HAS_PISTOL)  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_HAS_SHOTGUN, (enemy_has & HAS_SHOTGUN) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_HAS_AK47,    (enemy_has & HAS_AK47)    ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_HAS_GRENADE, (enemy_has & HAS_GRENADE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_HAS_BALLOON, (enemy_has & HAS_BALLOON) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_KNIFE,		  (enemy_has & HAS_KNIFE)   ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_BAT,		  (enemy_has & HAS_BAT)		? BST_CHECKED : BST_UNCHECKED);

			//
			// Set up the combat check boxes.
			//

			CheckDlgButton(hWnd, IDC_WITH_SLIDE         ,  (enemy_combat & PCOM_COMBAT_SLIDE         )  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_WITH_COMBO_PPP     ,  (enemy_combat & PCOM_COMBAT_COMBO_PPP     )  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_WITH_COMBO_KKK     ,  (enemy_combat & PCOM_COMBAT_COMBO_KKK     )  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_WITH_COMBO_ANY     ,  (enemy_combat & PCOM_COMBAT_COMBO_ANY     )  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_WITH_GRAPPLE_ATTACK,  (enemy_combat & PCOM_COMBAT_GRAPPLE_ATTACK)  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_WITH_SIDE_KICK     ,  (enemy_combat & PCOM_COMBAT_SIDE_KICK     )  ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_WITH_BACK_KICK     ,  (enemy_combat & PCOM_COMBAT_BACK_KICK     )  ? BST_CHECKED : BST_UNCHECKED);

			return	TRUE;
		}

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDC_COMBO4:
					if (HIWORD(wParam)==CBN_SELCHANGE) {

						{
							SLONG ai;
							BOOL  enable;

							ai = SendMessage((HWND)lParam,CB_GETCURSEL,0,0);

							switch(ai)
							{
								case 3: //PCOM_AI_ASSASIN:
								case 8: //PCOM_AI_BODYGUARD:
								case 17://PCOM_AI_GENOCIDE:
								case 21://PCOM_AI_SHOOT_DEAD:

									//
									// We need to know who to kill/protect.
									//

									enable = TRUE;
									break;

								default:
									enable = FALSE;
									break;
							}

							EnableWindow(GetDlgItem(hWnd,IDC_STATIC_ADJUST2), enable);
							EnableWindow(GetDlgItem(hWnd,IDC_SPIN4),          enable);
							EnableWindow(GetDlgItem(hWnd,IDC_EDIT4),		  enable);
						}
					}
					break;
				case	IDC_COMBO2:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						BOOL b=(SendMessage((HWND)lParam,CB_GETCURSEL,0,0)==4 || (SendMessage((HWND)lParam,CB_GETCURSEL,0,0)==6));
						EnableWindow(GetDlgItem(hWnd,IDC_STATIC_ADJUST3),b);
						EnableWindow(GetDlgItem(hWnd,IDC_SPIN5),b);
						EnableWindow(GetDlgItem(hWnd,IDC_EDIT5),b);
					}
					break;
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case WM_MEASUREITEM:
			return ticklist_measure(hWnd, wParam, lParam);
		case WM_DRAWITEM:
			return ticklist_draw(hWnd, wParam, lParam);
/*
		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT item = (LPMEASUREITEMSTRUCT) lParam;
			RECT rc;
			GetWindowRect(GetDlgItem(hWnd,item->CtlID),&rc);
			item->itemWidth=rc.right-rc.left;
			item->itemHeight=16;
			return TRUE;
		}

		case WM_DRAWITEM:
		{
			HWND ctl = GetDlgItem(hWnd,wParam);
			HDC  memdc;
			LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT) lParam;
			HBITMAP bmp,obmp;
			CBYTE pc[255];

			FillRect(item->hDC, &item->rcItem, (HBRUSH) GetStockObject(WHITE_BRUSH));
			SendMessage(ctl,LB_GETTEXT,item->itemID,(long)pc);
			TextOut(item->hDC,item->rcItem.left+20,item->rcItem.top+2,pc,strlen(pc));
			

			memdc=CreateCompatibleDC(item->hDC);
			bmp=(HBITMAP)LoadImage(NULL, (LPCTSTR)OBM_CHECKBOXES, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
			obmp=(HBITMAP)SelectObject(memdc,bmp);

			BitBlt(item->hDC,item->rcItem.left+2,item->rcItem.top+2,12,12,memdc,(13*item->itemData),0,SRCCOPY);

			SelectObject(memdc,obmp);
			DeleteDC(memdc);
			DeleteObject(bmp);

			if (item->itemState & ODS_FOCUS) 
				DrawFocusRect(item->hDC,&item->rcItem);

			return TRUE;
		}
*/
		case	WM_NOTIFY:
			lp_ntfy	=	(NM_UPDOWN*)lParam;

			//	Make the 'constitution' spin go up/down in steps of 5.
			if(lp_ntfy->hdr.idFrom==IDC_SPIN2 && lp_ntfy->hdr.code==UDN_DELTAPOS)
			{
				SendMessage	(
								lp_ntfy->hdr.hwndFrom,
								UDM_SETPOS,
								0,
								MAKELONG(lp_ntfy->iPos+(lp_ntfy->iDelta*4),0)
							);
				return	TRUE;
			}
			break;

		case	WM_VSCROLL:
			//	Set up the 'count' or 'constitution'.
			if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN1 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				enemy_count			=	HIWORD(wParam);
				return	TRUE;
			}
			else if(GetDlgCtrlID((HWND)lParam)==IDC_SPIN2 && LOWORD(wParam)==SB_THUMBPOSITION)
			{
				enemy_constitution	=	HIWORD(wParam);
				return	TRUE;
			}
			break;

		case	WM_CLOSE:
			enemy_type	=	SendMessage	(	GetDlgItem(hWnd,IDC_COMBO1),
											CB_GETCURSEL,
											0,0
										)	+	1;
			enemy_count =	SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
											UDM_GETPOS,
											0,0
										);
			enemy_constitution
						=	SendMessage	(	GetDlgItem(hWnd,IDC_SPIN2),
											UDM_GETPOS,
											0,0
										);
			enemy_move	=	SendMessage (	GetDlgItem(hWnd,IDC_COMBO2),
											CB_GETCURSEL,
											0,0
										);
/*			enemy_mood	=	SendMessage (	GetDlgItem(hWnd,IDC_COMBO3),
											CB_GETCURSEL,
											0,0
										);*/
			enemy_flags = ticklist_bitmask(hWnd,IDC_LIST3);

			enemy_ai	=	SendMessage (	GetDlgItem(hWnd,IDC_COMBO4),
											CB_GETCURSEL,
											0,0
										);
			enemy_ai |= SendMessage (	GetDlgItem(hWnd,IDC_COMBO3),
											CB_GETCURSEL,
											0,0
										) << 16;
			enemy_to_change
						=	SendMessage	(	GetDlgItem(hWnd,IDC_SPIN3),
											UDM_GETPOS,
											0,0
										);
			enemy_guard =	SendMessage	(	GetDlgItem(hWnd,IDC_SPIN4),
											UDM_GETPOS,
											0,0
										);
			enemy_follow=	SendMessage	(	GetDlgItem(hWnd,IDC_SPIN5),
											UDM_GETPOS,
											0,0
										);
			enemy_weaps = ticklist_bitmask(hWnd,IDC_LIST1);
			enemy_items = ticklist_bitmask(hWnd,IDC_LIST2);

			//
			// What this enemy has...
			// 

			enemy_has = 0;

			if (IsDlgButtonChecked(hWnd, IDC_HAS_PISTOL)  == BST_CHECKED) {enemy_has |= HAS_PISTOL;}
			if (IsDlgButtonChecked(hWnd, IDC_HAS_SHOTGUN) == BST_CHECKED) {enemy_has |= HAS_SHOTGUN;}
			if (IsDlgButtonChecked(hWnd, IDC_HAS_AK47)    == BST_CHECKED) {enemy_has |= HAS_AK47;}
			if (IsDlgButtonChecked(hWnd, IDC_HAS_GRENADE) == BST_CHECKED) {enemy_has |= HAS_GRENADE;}
			if (IsDlgButtonChecked(hWnd, IDC_HAS_BALLOON) == BST_CHECKED) {enemy_has |= HAS_BALLOON;}
			if (IsDlgButtonChecked(hWnd, IDC_KNIFE)		  == BST_CHECKED) {enemy_has |= HAS_KNIFE;}
			if (IsDlgButtonChecked(hWnd, IDC_BAT)		  == BST_CHECKED) {enemy_has |= HAS_BAT;}

			enemy_combat = 0;

			if (IsDlgButtonChecked(hWnd, IDC_WITH_SLIDE         ) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_SLIDE         ;}
			if (IsDlgButtonChecked(hWnd, IDC_WITH_COMBO_PPP     ) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_COMBO_PPP     ;}
			if (IsDlgButtonChecked(hWnd, IDC_WITH_COMBO_KKK     ) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_COMBO_KKK     ;}
			if (IsDlgButtonChecked(hWnd, IDC_WITH_COMBO_ANY     ) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_COMBO_ANY     ;}
			if (IsDlgButtonChecked(hWnd, IDC_WITH_GRAPPLE_ATTACK) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_GRAPPLE_ATTACK;}
			if (IsDlgButtonChecked(hWnd, IDC_WITH_SIDE_KICK     ) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_SIDE_KICK     ;}
			if (IsDlgButtonChecked(hWnd, IDC_WITH_BACK_KICK     ) == BST_CHECKED) {enemy_combat |= PCOM_COMBAT_BACK_KICK     ;}
			
			ticklist_close(hWnd, IDC_LIST1);
			ticklist_close(hWnd, IDC_LIST2);
			ticklist_close(hWnd, IDC_LIST3);

			EndDialog(hWnd,0);

			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_enemy_setup(EventPoint *the_ep, BOOL do_adjust)
{
	//	Set the dialog.
	adjust=do_adjust;
	enemy_type			=	LOWORD(the_ep->Data[0]);
	enemy_count			=	HIWORD(the_ep->Data[0]);
	enemy_follow		=	the_ep->Data[1];
	if(enemy_count==0)
		enemy_count	=	1;		//	Default to 1.
	enemy_constitution	=	LOWORD(the_ep->Data[2]);
	enemy_has           =   HIWORD(the_ep->Data[2]);
	enemy_move			=	the_ep->Data[3];
	enemy_flags			=	the_ep->Data[4];
	enemy_ai			=	the_ep->Data[5];
	if (do_adjust)
		enemy_to_change	=	the_ep->Data[6];
	enemy_guard			=	the_ep->Data[7] & 0xffff;
	enemy_combat        =   the_ep->Data[7] >> 16;

	enemy_weaps			=	the_ep->Data[8];
	enemy_items			=	the_ep->Data[9];
	if(enemy_constitution==0)
	{
		if(enemy_type==ET_NONE)
			enemy_constitution	=	0;
		else
			enemy_constitution	=	50;		//	Default for now, this should be set according to e type.
	}

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_ENEMY_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)es_proc
				);

	//	Set the data.
	the_ep->Data[0]	 =	MAKELONG(enemy_type,enemy_count);
	the_ep->Data[1]	 =	enemy_follow;
	the_ep->Data[2]	 =	MAKELONG(enemy_constitution,enemy_has);
	the_ep->Data[3]	 =	enemy_move;
	the_ep->Data[4]	 =	enemy_flags;
	the_ep->Data[5]	 =	enemy_ai;
	if (do_adjust)
		the_ep->Data[6] =	enemy_to_change;
	the_ep->Data[7]	 =	enemy_guard;
	the_ep->Data[7] |=  enemy_combat << 16;
	the_ep->Data[8]	 =	enemy_weaps;
	the_ep->Data[9]  =	enemy_items;
}

//---------------------------------------------------------------

CBYTE	*get_enemy_message(EventPoint *ep, CBYTE *msg) {
	if ((!ep)||(!ep->Data[0])) 
		strcpy(msg,"Unknown");
	else
		strcpy(msg,wenemy_strings[LOWORD(ep->Data[0])-1]);
	return msg;
}


