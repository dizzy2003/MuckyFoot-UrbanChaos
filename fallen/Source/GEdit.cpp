//	GEdit.cpp
//	Guy Simmons, 27th July 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	<zmouse.h>		// Mouse wheel support
#include	"resource.h"

#include	"pap.h"
#include	"EdStrings.h"
#include	"EngWind.h"
#include	"GEdit.h"
#include	"MapView.h"
#include	"SubClass.h"
#include	"WayWind.h"
#include	"WSpace.h"
#include	"tabctl.h"
#include	"ticklist.h"
#include	"renderstate.h"

#include	"ob.h"
#include	"game.h"
#include	"inside2.h"
#include	"c:\fallen\headers\memory.h"

//---------------------------------------------------------------
// from supermap.cpp
extern UWORD	calc_inside_for_xyz(SLONG x,SLONG y,SLONG z,UWORD *room);

// from MapView
extern SLONG GetEventY(EventPoint *ep, BOOL base=0);

// from aeng.cpp
extern UBYTE AENG_transparent_warehouses;

//---------------------------------------------------------------

#define	IDM_FIRSTCHILD	100

//---------------------------------------------------------------

//	Strings.
CBYTE		*GEDIT_editor_name	=	"Editor Class",
			*GEDIT_engine_name	=	"Engine View",
			*GEDIT_frame_name	=	"Urban Chaos mission editor",
			*GEDIT_map_name		=	"Map View";

SLONG		df,dl,dy,dp,dd;

//	Application instance.
HINSTANCE	GEDIT_hinstance;

//	Icons, Cursors & Menus.
HCURSOR		GEDIT_arrow,
			GEDIT_busy;
HICON		GEDIT_app_icon;
HMENU		GEDIT_main_menu,
			main_menu;

//	Window handles.
HWND		GEDIT_client_wnd,
			GEDIT_edit_wnd,
			GEDIT_engine_wnd,
			GEDIT_frame_wnd,
			GEDIT_view_wnd,
			GEDIT_way_wnd,
			GEDIT_workspace_wnd;

//	Window classes.
WNDCLASSEX	GEDIT_class_editor,
			GEDIT_class_engine,
			GEDIT_class_frame;


// Accel table
HACCEL GEDIT_accel;

// Mask for waypoints-to-display
SLONG display_mask = 0xFFFFFFFF;

// Editing mode (0=waypoints, 1=zones, 2=prims)
UBYTE edit_mode = 0;

// Jumping on waypoint-tree select
UBYTE leaping_disabled=0;

// storage for when a wpt is not selected
EventPoint dummy_ep;

//
// prim edit stuff
//
SLONG	prim_num	=	125; // helicopter; hey, it's _visible_...
SLONG	prim_height	=	0;
SLONG	prim_index	=	-1;
SLONG	prim_drag	=	0;
SLONG	prim_dir	=	0;
SLONG   prim_x		=	0,
		prim_z		=	0;
BOOL	prim_ware	=	0;
BOOL	prim_psxmode=	0;

//
// The mousewheel message.
//

UINT GEDIT_wm_mousewheel;

// Result passed back to main game (eg to bail early)
INT EditorResult=0;


extern	int		waypoint_colour,
				waypoint_group;
extern	UBYTE	button_colours[][3];
//extern	TCHAR	button_classes[][_MAX_PATH];

BOOL				init_mission_editor(void);
void				fini_mission_editor(void);
BOOL CALLBACK		mission_editor_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

BOOL is_in_mission_editor = 0;

//---------------------------------------------------------------
//	WindProc for editor class.

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
extern void ClearLatchedKeys();

extern HINSTANCE		hGlobalThisInst;


CBYTE old_path[_MAX_PATH];


int	gedit(void)
{
	int			result;
	MSG			msg;
	UBYTE		block_keyboard_messages;

	is_in_mission_editor = 1;

	//	Get the working directory.
	GetCurrentDirectory(_MAX_PATH,old_path);

	GEDIT_hinstance	=	hGlobalThisInst;

	//	Load cursors.
	GEDIT_arrow		=	LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW));
	GEDIT_busy		=	LoadCursor(NULL,MAKEINTRESOURCE(IDC_WAIT));

	//	Load icons.
	GEDIT_app_icon	=	LoadIcon(GEDIT_hinstance,MAKEINTRESOURCE(IDI_MFLOGO));

	//	Load menus.
	GEDIT_main_menu	=	LoadMenu(GEDIT_hinstance,MAKEINTRESOURCE(IDR_GEDIT_MENU));

	GEDIT_accel = LoadAccelerators(GEDIT_hinstance, MAKEINTRESOURCE(IDR_GEDIT_ACCELERATOR));

	GEDIT_wm_mousewheel = RegisterWindowMessage(MSH_MOUSEWHEEL);

/*
	// Initialise windows common controls.
	InitCommonControls();

	//	Open the display.
	if(OpenDisplay(640,480,16,FLAGS_USE_3D) != 0)
		return;		//	Couldn't open the display.
*/
	init_mission_editor();
	GEDIT_edit_wnd	=	CreateDialog(
										GEDIT_hinstance,
										MAKEINTRESOURCE(IDD_MISSION_EDITOR2),
										0,
										(DLGPROC)mission_editor_proc
									);
	ShowWindow(GEDIT_edit_wnd, SW_SHOW);

	//
	// Try to load the last workspace that was loaded.
	//

	load_workspace(TRUE);

	//	The message loop.
	while(1)
	{
		if(!PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// No messages pending- send a user message so we can
			// do our processing and display the engine.

			PostMessage(GEDIT_edit_wnd, WM_USER, 0, 0);

			// clear latched keys in keyboard driver

			ClearLatchedKeys();
		}

		result	=	GetMessage(&msg, NULL, 0, 0);
		if(result==0 || result==-1)
			break;

		// damn you, evil mousewheel handler!
		if (msg.message==WM_MOUSEWHEEL) {
			msg.wParam=(short)HIWORD(msg.wParam);
		}
		if (msg.message==GEDIT_wm_mousewheel) {
			msg.message=WM_MOUSEWHEEL;
		}


		// damn you, evil keyboard handler!
		block_keyboard_messages=0;
		switch(msg.message)
		{
			case	WM_KEYDOWN:
			case	WM_KEYUP:
				KeyboardProc(msg.message,msg.wParam,msg.lParam);
				if(ED_KEYS) block_keyboard_messages=1;
				break;
			default:
				TranslateMessage(&msg);
		}


		if ((!block_keyboard_messages)&&!TranslateAccelerator(GEDIT_edit_wnd, GEDIT_accel, &msg))
			if(GEDIT_edit_wnd==0 || !IsDialogMessage(GEDIT_edit_wnd,&msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
	}

	fini_mission_editor();

	//	Reset the working directory.
	SetCurrentDirectory(old_path);

	is_in_mission_editor = 0;

	return EditorResult;
}

//---------------------------------------------------------------

BOOL	init_mission_editor(void)
{
	//	Set up the mission data.
	MISSION_init();

	init_wwind();
	init_map_view();

	return	TRUE;
}

//---------------------------------------------------------------

void	fini_mission_editor(void)
{
//	fini_map_view();
	fini_wwind();
}

//---------------------------------------------------------------

#define	INIT_COMBO_BOX(i,s)			the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,CB_SETCURSEL,0,0);


/******************************************************************8
 *		Waypoint Info	--	tab dialog procedure
 */

BOOL CALLBACK		waypoint_tab_proc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	HWND  the_ctrl, hwndParent = GetParent(hWnd); 
/*
	if (message==WM_MOUSEWHEEL) {
		wParam=HIWORD(wParam); // don't ask...
	} else {
		if (message==GEDIT_wm_mousewheel) message=WM_MOUSEWHEEL;
	}
*/
	switch (message) {
	case WM_INITDIALOG:
		{
			SLONG i;
			CBYTE str[2];

			SetWindowPos(hWnd, HWND_TOP, 2, 26, 0, 0, SWP_NOSIZE); // relative to parent (tab)

			// colours:
			the_ctrl=GetDlgItem(hWnd,IDC_COMBO1);
			for (i=0;i<WAY_COLOURS;i++) {
				SendMessage(the_ctrl,CB_ADDSTRING,0,i);
			}
			SendMessage(the_ctrl,CB_SETCURSEL,0,0);

			// groups:
			the_ctrl=GetDlgItem(hWnd,IDC_COMBO2);
			str[1]=0;
			for (i=0;i<26;i++) {
				str[0]=i+65;
				SendMessage(the_ctrl,CB_ADDSTRING,0,(LONG)str);
			}
			SendMessage(the_ctrl,CB_SETCURSEL,0,0);

		//	Init the height
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(0,8192)
						);

			ep_to_controls2(selected_ep,0,hWnd);
		}
		break;

	case WM_CLOSE:
		controls_to_ep2(selected_ep,0,hWnd);
		return TRUE;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT item = (LPMEASUREITEMSTRUCT) lParam;
			RECT rc;
			GetWindowRect(GetDlgItem(hWnd,item->CtlID),&rc);
			item->itemWidth=rc.right-rc.left;
			item->itemHeight=20;
			return TRUE;
		}

	case	WM_MOUSEWHEEL:
		if (selected_ep)
		{
			SLONG	the_value;
			SWORD	the_wheel;
			SLONG	diff;
				
			the_wheel = wParam;    // wheel rotation
			the_ctrl = GetDlgItem(hWnd,IDC_SPIN1);
			the_value = SendMessage(the_ctrl, UDM_GETPOS, 0, 0);

			if (selected_ep->Flags&WPT_FLAGS_INSIDE) {
//				diff=(the_wheel<0) ? diff=-128 : diff=260;
//				selected_ep->Y=INDOORS_INDEX=calc_inside_for_xyz(selected_ep->X, GetEventY(selected_ep)+diff, selected_ep->Z,&INDOORS_ROOM);

extern SLONG GetNextFloor(EventPoint *ep, SBYTE dir, UWORD *room);				
				selected_ep->Y=INDOORS_INDEX=GetNextFloor(selected_ep,the_wheel,&INDOORS_ROOM);
				if (INDOORS_INDEX) 
					INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
				else INDOORS_DBUILDING=0;

			} else {

				if (GetAsyncKeyState(VK_SHIFT)&(1<<15)) {
					diff=(the_wheel<0) ? -256 : 256;
				} else {
					diff=(the_wheel<0) ? -32 : 32;
				}
				the_value+=diff;
				if (the_value<0) {
					diff-=the_value;
				} else {
					if (the_value>8192) {
						diff-=(the_value-8192);
					}
				}

				selected_ep->Y+=diff;
			}

			SendMessage(the_ctrl, UDM_SETPOS, 0,MAKELONG(the_value,0));
				controls_to_ep2(selected_ep,0);
		}
	return	TRUE;

	case	WM_NOTIFY:
		switch(LOWORD(wParam)) {
			case IDC_SPIN1:
			{
				LPNMUPDOWN ud = (LPNMUPDOWN)lParam;
				SLONG newpos = (ud->iPos+ud->iDelta);
				if ((newpos<0)||(newpos>1024)) { // that's bad
					return TRUE;
				} else {
					if (selected_ep)
					{
						selected_ep->Y+=ud->iDelta;
					}
					return FALSE;
				}
			}
			break;
		}
		break;

	case	WM_COMMAND:
		switch(LOWORD(wParam)) {

			case	IDC_COMBO1:
				waypoint_colour = SendMessage(GetDlgItem(TABCTL_gethwnd(GEDIT_edit_wnd,IDC_TAB1),IDC_COMBO1),CB_GETCURSEL,0,0);
				// intentional fallthru
			case	IDC_COMBO2:
				controls_to_ep2(selected_ep,0);
				break;
		}
		break;

	case WM_DRAWITEM:
		{
			HWND ctl = GetDlgItem(hWnd,wParam);
			LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT) lParam;
			HBRUSH hBr;
			RECT rc;

			FillRect(item->hDC, &item->rcItem, (HBRUSH) GetStockObject(WHITE_BRUSH));

			if (item->itemData != -1) {
				hBr = CreateSolidBrush(RGB(
					button_colours[item->itemData][0],
					button_colours[item->itemData][1],
					button_colours[item->itemData][2]));
				rc=item->rcItem;
				rc.top+=2; rc.bottom-=2; rc.right-=2; rc.left=rc.right-32;
				FillRect(item->hDC, &rc, hBr);
				DeleteObject(hBr);
				TextOut(item->hDC,item->rcItem.left+4,item->rcItem.top+4,colour_strings[item->itemData],strlen(colour_strings[item->itemData]));
			}

			if (item->itemState & ODS_FOCUS) 
				DrawFocusRect(item->hDC,&item->rcItem);

			return TRUE;
		}
	}
	return FALSE;
}

/******************************************************************8
 *		Condition Tab	--	tab dialog procedure
 */

void condition_tab_depend1(HWND hWnd, UBYTE enable) {
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_DEPEND1),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT1),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN1),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_BUTTON1),enable);
//	EnableWindow(GetDlgItem(hWnd,IDC_CHECK1),enable);
}
void condition_tab_depend2(HWND hWnd, UBYTE enable) {
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_DEPEND2),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT2),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN2),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_BUTTON2),enable);
}
void condition_tab_timer(HWND hWnd, UBYTE enable) {
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_TIME),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT3),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN3),enable);
	if (enable) SetWindowText(GetDlgItem(hWnd,IDC_STATIC_TIME),"Time:");
}
void condition_tab_radius(HWND hWnd, UBYTE enable) {
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_RADIUS),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT4),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN4),enable);
}
void condition_tab_shout(HWND hWnd, UBYTE enable) {
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_LISTEN),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT5),enable);
}
void condition_tab_counter(HWND hWnd, UBYTE enable) {
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_TIME),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT3),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN3),enable);
	if (enable) SetWindowText(GetDlgItem(hWnd,IDC_STATIC_TIME),"Count:");
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_DEPEND1),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT1),enable);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN1),enable);

}

void condition_tab_update(HWND hWnd) {
	SLONG ndx = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);

	condition_tab_depend1(hWnd,WaypointUses[ndx] & WPU_DEPEND);
	condition_tab_depend2(hWnd,WaypointUses[ndx] & WPU_BOOLEAN);
	condition_tab_timer(hWnd,WaypointUses[ndx] & WPU_TIME);
	condition_tab_radius(hWnd,WaypointUses[ndx] & WPU_RADIUS);
	condition_tab_shout(hWnd,WaypointUses[ndx] & WPU_RADTEXT);
	if (!(WaypointUses[ndx] & (WPU_TIME|WPU_DEPEND))) condition_tab_counter(hWnd,WaypointUses[ndx] & WPU_COUNTER);

}


BOOL CALLBACK		condition_tab_proc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	HWND  the_ctrl, hwndParent = GetParent(hWnd); 
	SLONG c0;
	LPTSTR lbitem_str;
/*
	if (message==WM_MOUSEWHEEL) {
		wParam=HIWORD(wParam); // don't ask...
	} else {
		if (message==GEDIT_wm_mousewheel) message=WM_MOUSEWHEEL;
	}
*/
	switch (message) {
	case WM_INITDIALOG:
		{
		SetWindowPos(hWnd, HWND_TOP, 2, 26, 0, 0, SWP_NOSIZE); // relative to parent (tab)

		INIT_COMBO_BOX(IDC_COMBO1,wtrigger_strings);

		//	Init the dependency
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(0,511)
						);
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0,MAKELONG(0,0)
						);
		//	Init the boolean to be the same
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETRANGE,
							0,
							MAKELONG(0,511)
						);
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN2),
							UDM_SETPOS,
							0,MAKELONG(0,0)
						);
		//	Init the timer
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN3),
							UDM_SETRANGE,
							0,
							MAKELONG(0,3600)
						);
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN3),
							UDM_SETPOS,
							0,MAKELONG(0,0)
						);
		//  Init the radius
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN4),
							UDM_SETRANGE,
							0,
							MAKELONG(0,2560)
						);
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN4),
							UDM_SETPOS,
							0,MAKELONG(0,0)
						);

			UDACCEL ud_accel = {0,32};

			SendMessage (	GetDlgItem(hWnd,IDC_SPIN4),
							UDM_SETACCEL,
							1,(long)&ud_accel);

		ep_to_controls2(selected_ep,1,hWnd);
		}

	case	WM_DESTROY:
	case	WM_CLOSE:
		controls_to_ep2(selected_ep,1,hWnd);
		return TRUE;

	case	WM_MOUSEWHEEL:
		{
		SLONG	ndx = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
		SLONG	the_value;
		SWORD	the_wheel;
		SWORD	ctlidx, scale1, scale2;

			switch(ndx) {
			case TT_RADIUS:
			case TT_ENEMYRADIUS:
			case TT_PLAYER_USES_RADIUS:
			case TT_THING_RADIUS_DIR:
			case TT_MOVE_RADIUS_DIR:
				ctlidx = IDC_SPIN4;
				scale1 = 32;
				scale2 = 128;
				break;
			case TT_TIMER:
			case TT_COUNTDOWN:
			case TT_VISIBLECOUNTDOWN:
			case TT_COUNTER:
				ctlidx = IDC_SPIN3;
				scale1 = 1;
				scale2 = 10;
				break;
			default:
				return TRUE;
			}
			
			the_wheel = wParam;    // wheel rotation
			the_ctrl = GetDlgItem(hWnd,ctlidx);
			the_value = SendMessage(the_ctrl, UDM_GETPOS, 0, 0);
			if (GetAsyncKeyState(VK_SHIFT)&(1<<15))
				the_value +=(the_wheel<0) ? -scale2 : scale2;
			else
				the_value +=(the_wheel<0) ? -scale1 : scale1;
			SendMessage(the_ctrl, UDM_SETPOS, 0,MAKELONG(the_value,0));
			controls_to_ep2(selected_ep,1);
		}
		return	TRUE;

	case	WM_VSCROLL:
		controls_to_ep2(selected_ep,1);
		break;

/*	case	WM_NOTIFY: // UDN's _have_ to be awkward...
		{
			NMHDR			*p_nmhdr;
			p_nmhdr	=	(NMHDR*)lParam;
			switch(p_nmhdr->code)
			{
				case	UDN_DELTAPOS:
					switch (p_nmhdr->idFrom) {
					case IDC_SPIN4:
						NM_UPDOWN* p_updn;
						//	Make the 'radius' spin go up/down in steps of 32.
						p_updn	=	(NM_UPDOWN*)p_nmhdr;
						SendMessage	(
										p_nmhdr->hwndFrom,
										UDM_SETPOS,
										0,
										MAKELONG(p_updn->iPos+(p_updn->iDelta*31),0)
									);
						break;
					}
					break;
			}
			controls_to_ep2(selected_ep,1);
		}
		break;
*/
	case	WM_COMMAND:
		switch(LOWORD(wParam)) {
			case	IDC_SPIN4:
				controls_to_ep2(selected_ep,1);
				break;

			case	IDC_BUTTON1:
//				link_start_ep=selected_ep;
				link_mode=1;
				return TRUE;

			case	IDC_BUTTON2:
//				link_start_ep=selected_ep;
				link_mode=2;
				return TRUE;

			case	IDC_EDIT5:
				if (selected_ep&&(HIWORD(wParam)==EN_CHANGE))
					controls_to_ep2(selected_ep,1);
				return TRUE;

			case	IDC_EDIT3:
//				controls_to_ep2(selected_ep,1);
/*				{
				static BOOL blah=0;
				if (HIWORD(wParam)==EN_SETFOCUS) blah=1;
				if (HIWORD(wParam)==EN_KILLFOCUS) blah=0;
				if (blah&&selected_ep&&(HIWORD(wParam)==EN_CHANGE))
					selected_ep->Radius	= (SendMessage(GetDlgItem(hWnd,IDC_SPIN3),UDM_GETPOS,0,0)&0xffff);
					if (WaypointUses[SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0)] & WPU_TIME) selected_ep->Radius*=100;

					CBYTE msg[20];
					sprintf(msg,"rad: %d\n",selected_ep->Radius);
				    CONSOLE_text(msg,5000);
				}*/
				return TRUE;

			case	IDC_CHECK1:
			case	IDC_CHECK2:
				controls_to_ep2(selected_ep,1);
				return TRUE;

			case	IDC_COMBO1:
				if(HIWORD(wParam)==CBN_SELCHANGE)
				{
//					condition_tab_update(hWnd);
					controls_to_ep2(selected_ep,1);
					ep_to_controls2(selected_ep,1); // to reset unused stuff

				}
				return	TRUE;
		}

		break;
	}
	return FALSE;
}


/******************************************************************8
 *		Action Tab		--	tab dialog	procedure
 */

void action_tab_update(HWND hWnd) {
	SLONG ndx = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
	EnableWindow(GetDlgItem(hWnd,IDC_BUTTON1),(selected_ep&&TypeHasProperties(ndx+1)));
	ndx = SendMessage(GetDlgItem(hWnd,IDC_COMBO2),CB_GETCURSEL,0,0);
	CBYTE enabled = (ndx==2);
	EnableWindow(GetDlgItem(hWnd,IDC_STATIC_DELAY),enabled);
	EnableWindow(GetDlgItem(hWnd,IDC_EDIT1),enabled);
	EnableWindow(GetDlgItem(hWnd,IDC_SPIN1),enabled);
}

BOOL CALLBACK		action_tab_proc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	HWND  the_ctrl, hwndParent = GetParent(hWnd); 
	SLONG c0;
	LPTSTR lbitem_str;
/*
	if (message==WM_MOUSEWHEEL) {
		wParam=HIWORD(wParam); // don't ask...
	} else {
		if (message==GEDIT_wm_mousewheel) message=WM_MOUSEWHEEL;
	}
*/
	switch (message) {
	case WM_INITDIALOG:
		SetWindowPos(hWnd, HWND_TOP, 2, 26, 0, 0, SWP_NOSIZE); // relative to parent (tab)
		INIT_COMBO_BOX(IDC_COMBO1,wtype_strings);
		INIT_COMBO_BOX(IDC_COMBO2,on_trigger_strings);
		//	Init the timer
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETRANGE,
							0,
							MAKELONG(0,3600)
						);
			SendMessage	(	GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0,MAKELONG(0,0)
						);

		ep_to_controls2(selected_ep,2,hWnd);
		break;

	case WM_DESTROY:
		controls_to_ep2(selected_ep,2,hWnd);
		return TRUE;

	case	WM_VSCROLL:
		controls_to_ep2(selected_ep,2);
		break;

	case	WM_MOUSEWHEEL:
		{
		SLONG	the_value;
		SWORD	the_wheel;

			the_wheel = wParam;    // wheel rotation
			the_ctrl = GetDlgItem(hWnd,IDC_SPIN1);

			if (IsWindowEnabled(the_ctrl)) {
				the_value = SendMessage(the_ctrl, UDM_GETPOS, 0, 0);
				if (GetAsyncKeyState(VK_SHIFT)&(1<<15))
					the_value +=(the_wheel<0) ? -10 : 10;
				else
					the_value +=(the_wheel<0) ? -1 : 1;
				SendMessage(the_ctrl, UDM_SETPOS, 0,MAKELONG(the_value,0));
				controls_to_ep2(selected_ep,2);
			}
		}
		return	TRUE;

	case	WM_COMMAND:
		switch(LOWORD(wParam)) {
			case	IDC_BUTTON1:
				OpenProperties(selected_ep);
				ep_to_controls2(selected_ep,2); // this updates the caption. i hope.
				break;
			case	IDC_COMBO1:
			case	IDC_COMBO2:
				if(HIWORD(wParam)==CBN_SELCHANGE) {
					action_tab_update(hWnd);
					controls_to_ep2(selected_ep,2);
				}
				break;
			case	IDC_SPIN1:
				if (HIWORD(wParam)==UDN_DELTAPOS)
					controls_to_ep2(selected_ep,2);
				break;
			case	IDC_EDIT1:
/*				if (HIWORD(wParam)==EN_CHANGE)
					controls_to_ep2(selected_ep,2);*/
				break;
		}
		break;
	}
	return FALSE;
}

/******************************************************************8
 *		Mission Editor	--	main dialog	procedure
 */

HRESULT combo_draw(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	HWND ctl = GetDlgItem(hWnd,wParam);
	LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT) lParam;
	HBRUSH hBr;
	RECT rc;
	SLONG bk,tx,col;

	FillRect(item->hDC, &item->rcItem, (HBRUSH) GetStockObject(WHITE_BRUSH));

	bk=SetBkColor(item->hDC,0xFFFFFF);
	tx=SetTextColor(item->hDC,0x000000);
	if (item->itemData != -1) {
		col=zone_colours[item->itemData];
		col=((col&0xff)<<16)+((col>>16)&0xff)+(col&0xff00);
		hBr = CreateSolidBrush(col);
		rc=item->rcItem;
		rc.top+=2; rc.bottom-=2; rc.right-=2; rc.left=rc.right-24;
		FillRect(item->hDC, &rc, hBr);
		DeleteObject(hBr);
		TextOut(item->hDC,item->rcItem.left+4,item->rcItem.top+4,zonetype_strings[item->itemData],strlen(zonetype_strings[item->itemData]));
	}
	SetBkColor(item->hDC,bk);
	SetTextColor(item->hDC,tx);

	if (item->itemState & ODS_FOCUS) 
		DrawFocusRect(item->hDC,&item->rcItem);
	return TRUE;
}



#ifdef EDITOR

BOOL CALLBACK		mission_editor_proc	(
											HWND hWnd,
											UINT message,
											WPARAM wParam,
											LPARAM lParam
										)
{
	BOOL			update	=	FALSE;
	int				c0;
	HWND			the_ctrl;
	LPTSTR			lbitem_str;
	NMHDR			*p_nmhdr;
	NM_TREEVIEW		*p_tview;
	POINT			bottom_right,
					click_point,
					top_left;
	HMENU			ws_menu;
	HTREEITEM			item_handle;
	TV_HITTESTINFO		hit_test;
	TV_ITEM				the_item;
	
/*
	// heh heh heh
	if (message==WM_MOUSEWHEEL) {
		wParam=HIWORD(wParam); // don't ask...
		CONSOLE_text("WIN98 MOUSEWHEEL");
	} else {
		if (message==GEDIT_wm_mousewheel) {
			message=WM_MOUSEWHEEL;
			CONSOLE_text("WIN95 WHEEL CONVERT");
		}
	}
*/	
	switch(message)
	{
		case	WM_INITDIALOG:

			TBBUTTON btns[3];
			HWND res;

			btns[0].iBitmap=0;
			btns[0].idCommand=ID_TOOLBAR_WAYPOINT;
			btns[0].fsState=TBSTATE_ENABLED|TBSTATE_CHECKED;
			btns[0].fsStyle=TBSTYLE_CHECK|TBSTYLE_GROUP;
			btns[0].iString=0;
			btns[1].iBitmap=1;
			btns[1].idCommand=ID_TOOLBAR_ZONE;
			btns[1].fsState=TBSTATE_ENABLED;
			btns[1].fsStyle=TBSTYLE_CHECK|TBSTYLE_GROUP;
			btns[1].iString=1;
			btns[2].iBitmap=2;
			btns[2].idCommand=ID_TOOLBAR_PRIM;
			btns[2].fsState=TBSTATE_ENABLED;
			btns[2].fsStyle=TBSTYLE_CHECK|TBSTYLE_GROUP;
			btns[2].iString=2;


			// Set up the toolbar. eek.
			res=CreateToolbarEx(GetDlgItem(hWnd,IDC_STATIC_TRAY), WS_CHILD|WS_VISIBLE|TBSTYLE_TOOLTIPS, IDC_TOOLBAR1, 3, GEDIT_hinstance, IDB_TOOLBAR, btns, 3, 16, 16, 16, 15, sizeof(TBBUTTON));
//			res=CreateToolbarEx(hWnd, WS_CHILD|WS_VISIBLE, IDC_TOOLBAR1, 2, GEDIT_hinstance, IDB_TOOLBAR, btns, 2, 24, 24, 16, 16, sizeof(TBBUTTON));
			if (!res) {
				DWORD err=GetLastError();
				CBYTE msg[800];
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, msg, 256, NULL);
				TRACE("bugger: ");
				TRACE(msg);

			} else {
//				CBYTE msg[]="Edit Waypoints\0Edit Zones\0\0";
//				SendMessage(res,TB_ADDSTRING,0,(long)msg);
				SendMessage(res,TB_SETPARENT,(WPARAM)hWnd,0);
			}

			// Set up the tab controls
			TABCTL_add(GEDIT_hinstance,hWnd,IDC_TAB1,IDD_EVENTTAB_INFO,"Waypoint Info",waypoint_tab_proc,IDD_EVENTTAB_CONDITION,"When this happens...",condition_tab_proc,IDD_EVENTTAB_ACTION,"Do this...",action_tab_proc,0);

			// Display options
			INIT_COMBO_BOX(IDC_COMBO4,showlines_strings)
			SendMessage(the_ctrl,CB_SETCURSEL,0,0);
/*
			//	Init the 'Type' check boxes.
			for(c0=IDC_CHECK_1;c0<=IDC_CHECK_8;c0++)
			{
				the_ctrl	=	GetDlgItem(hWnd,c0);
				SendMessage(the_ctrl,BM_SETCHECK,1,0);
			}
*/
			// zone colours:
			the_ctrl=GetDlgItem(hWnd,IDC_COMBO1);
			for (c0=0;c0<ZF_NUM;c0++) {
				SendMessage(the_ctrl,CB_ADDSTRING,0,c0);
			}
			SendMessage(the_ctrl,CB_SETCURSEL,0,0);

			
//			EnableWindow(GetDlgItem(hWnd,IDC_EDIT2),FALSE);

			ShowWindow(GetDlgItem(hWnd,IDC_CHECK1),SW_HIDE);
			ShowWindow(GetDlgItem(hWnd,IDC_NOPSX_BUTTON),SW_HIDE);

			//	Set up control handles.
			GEDIT_view_wnd	=	GetDlgItem(hWnd,IDC_MAP_VIEW);
			GEDIT_main_menu	=	GetMenu(hWnd);

			//	Init 'Workspace' tree control.
			init_workspace(hWnd);
			//  and list of waypoints
			init_wptlist(hWnd);

			ticklist_init(hWnd,IDC_LIST1,wtype_strings,display_mask);

			return	TRUE;

		case	WM_MEASUREITEM:
			if (wParam==IDC_COMBO1) {
				LPMEASUREITEMSTRUCT item = (LPMEASUREITEMSTRUCT) lParam;
				RECT rc;
				GetWindowRect(GetDlgItem(hWnd,item->CtlID),&rc);
				item->itemWidth=rc.right-rc.left;
				item->itemHeight=20;
				return TRUE;
			} else {
				return	ticklist_measure(hWnd, wParam, lParam);
			}
			break;

		case	WM_DRAWITEM:
			if (wParam==IDC_COMBO1) {
				return	combo_draw(hWnd, wParam, lParam); // just too untidy otherwise
			} else {
				return	ticklist_draw(hWnd, wParam, lParam);
			}
			break;

		case	WM_MOUSEWHEEL:

			switch(edit_mode) {
			case 0:
				SendMessage(TABCTL_gethwnd(hWnd,IDC_TAB1),WM_MOUSEWHEEL,wParam,lParam);
				break;
			case 1:
				break;
			case 2:
				{
					SWORD the_wheel = wParam;    // wheel rotation
					if (GetAsyncKeyState(VK_SHIFT)&(1<<15)) {
						prim_height+=(the_wheel<0) ? -256 : 256;
					} else {
						prim_height+=(the_wheel<0) ? -32 : 32;
					}
					if (GetAsyncKeyState(VK_CONTROL)&(1<<15)) {
						if (prim_height<0) prim_height=0;
					}
					if (prim_index!=-1) {
						UBYTE flag;
//						OB_ob[prim_index].flags=(!prim_height) ? OB_FLAG_ON_FLOOR : 0;
						OB_ob[prim_index].y=prim_height+PAP_calc_map_height_at(prim_x,prim_z);

						INDOORS_INDEX=calc_inside_for_xyz(prim_x, OB_ob[prim_index].y, prim_z,&INDOORS_ROOM);

						if (GetAsyncKeyState(VK_SHIFT)&(1<<15)) {
							OB_ob[prim_index].y=get_inside_alt(INDOORS_INDEX);
						}

						if (INDOORS_INDEX) {
							INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
//							OB_ob[prim_index].flags|=OB_FLAG_INSIDE;
						} else {
							INDOORS_DBUILDING=0;
//							OB_ob[prim_index].flags&=~OB_FLAG_INSIDE;
						}
						OB_ob[prim_index].InsideIndex=INDOORS_INDEX;
					}
				}
				break;
			}

			return	TRUE;


		case	WM_CLOSE:
			
			// restore normality to list box
			ticklist_close(hWnd, IDC_LIST1);

			// free up tab info structures
			TABCTL_del(GEDIT_edit_wnd,IDC_TAB1);

			//	Close the current workspace.
			if(close_workspace())
			{
				EndDialog(hWnd,TRUE);
				PostQuitMessage(0);
			}
			return	TRUE;

//		case	WM_DRAWITEM:
		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	ID_FILE_NEW_WS:
					create_workspace();
					return	TRUE;

				case	ID_FILE_OPEN_WS:
					load_workspace(FALSE);
					return	TRUE;

				case	ID_FILE_CLOSE_WS:
					close_workspace();
					return	TRUE;

				case	ID_FILE_SAVE_WS:
					save_workspace();
					return	TRUE;

				case	ID_FILE_SAVE_WS_AS:
					return	TRUE;

				case	ID_FILE_EXITALL:
					EditorResult=-1; // fallthru 
				case	ID_FILE_EXIT:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;

				case	ID_GEDIT_DEL_PRIMS:
					if (edit_mode==2) {
						if (prim_index!=-1) {
							if (MessageBox(hWnd,"Are you sure you want to trash ALL prims on this level of the same type as the one you have selected now?",
								"Delete Prims", MB_ICONEXCLAMATION|MB_OKCANCEL)==IDOK) {
								SLONG x,z,ct=0;
								CBYTE str[255];

								for (x=0;x<OB_SIZE;x++)
									for(z=0;z<OB_SIZE;z++) {
										OB_Info *oi=OB_find(x,z);
										UBYTE oict=0;
										while((oict<31)&&oi->prim)
										{
											if (oi->prim==prim_num) {
												OB_remove(oi);
												oi=OB_find(x,z);
												ct++;
											} else oi++;
											oict++;
										}

									}
								sprintf(str,"Removed %d prims.",ct);
								MessageBox(hWnd,str,"Delete Prims",MB_OK|MB_ICONINFORMATION);
								prim_index=-1;
/*
								num=0;
								while (num<OB_ob_upto) {
								}
								for (num=0;num<OB_ob_upto;num++) {
									OB_Ob     *oo;
									oo = &OB_ob[index];
									if (oo->prim==prim_index) {
										OB_Info oi;

										oi.x=; oi.z=prim_z;
										oi.index=prim_index;
										OB_remove(&oi);
									}
								}*/

							};
						}

					}
					return	TRUE;
				
				case	ID_TOOLBAR_WAYPOINT:
					edit_mode=0;
					ShowWindow(GetDlgItem(hWnd,IDC_COMBO1),SW_HIDE);
					ShowWindow(GetDlgItem(hWnd,IDC_CHECK1),SW_HIDE);
					ShowWindow(GetDlgItem(hWnd,IDC_NOPSX_BUTTON),SW_HIDE);
					return TRUE;

				case	ID_TOOLBAR_ZONE:
					edit_mode=1;
					ShowWindow(GetDlgItem(hWnd,IDC_COMBO1),SW_SHOW);
					ShowWindow(GetDlgItem(hWnd,IDC_CHECK1),SW_HIDE);
					ShowWindow(GetDlgItem(hWnd,IDC_NOPSX_BUTTON),SW_HIDE);
					return TRUE;

				case	ID_TOOLBAR_PRIM:
					edit_mode=2;
					ShowWindow(GetDlgItem(hWnd,IDC_COMBO1),SW_HIDE);
					ShowWindow(GetDlgItem(hWnd,IDC_CHECK1),SW_SHOW);
					ShowWindow(GetDlgItem(hWnd,IDC_NOPSX_BUTTON),SW_SHOW);
					return TRUE;

				case	IDC_CHECK1:
extern void MAV_calc_height_array(SLONG ignore_warehouses);
					AENG_transparent_warehouses = SendMessage(GetDlgItem(hWnd,IDC_CHECK1),BM_GETSTATE,0,0) & BST_CHECKED;
					MAV_calc_height_array(AENG_transparent_warehouses);
					return TRUE;

				case	IDC_NOPSX_BUTTON:
#ifdef EDITOR
					RenderState::AllowFadeOut=prim_psxmode = (IsDlgButtonChecked(hWnd,IDC_NOPSX_BUTTON)==BST_CHECKED);
#endif
					return TRUE;

/*
				case	IDC_CUSTOM_1:
				case	IDC_CUSTOM_2:
				case	IDC_CUSTOM_3:
				case	IDC_CUSTOM_4:
				case	IDC_CUSTOM_5:
				case	IDC_CUSTOM_6:
				case	IDC_CUSTOM_7:
				case	IDC_CUSTOM_8:

					hdc				=	GetDC(hWnd);

					//	Erase the old selected colour box.
					the_ctrl		=	GetDlgItem(hWnd,waypoint_colour+IDC_CUSTOM_1);
					GetWindowRect(the_ctrl,&button_rect);
					top_left.x		=	button_rect.left;
					top_left.y		=	button_rect.top;
					bottom_right.x	=	button_rect.right;
					bottom_right.y	=	button_rect.bottom;
					ScreenToClient(hWnd,&top_left);
					ScreenToClient(hWnd,&bottom_right);
					SetRect(&button_rect,top_left.x,top_left.y,bottom_right.x,bottom_right.y);
					InflateRect(&button_rect,3,3);
					FrameRect(hdc,&button_rect,(HBRUSH)GetStockObject(LTGRAY_BRUSH));

					//	Set the new one & draw it.
					waypoint_colour	=	LOWORD(wParam)-IDC_CUSTOM_1;
					the_ctrl		=	GetDlgItem(hWnd,LOWORD(wParam));
					GetWindowRect(the_ctrl,&button_rect);
					top_left.x		=	button_rect.left;
					top_left.y		=	button_rect.top;
					bottom_right.x	=	button_rect.right;
					bottom_right.y	=	button_rect.bottom;
					ScreenToClient(hWnd,&top_left);
					ScreenToClient(hWnd,&bottom_right);
					SetRect(&button_rect,top_left.x,top_left.y,bottom_right.x,bottom_right.y);
					InflateRect(&button_rect,3,3);
					DrawFocusRect(hdc,&button_rect);

					ReleaseDC(hWnd,hdc);

					//	Update the event point.
					if (HIWORD(wParam)==BN_CLICKED) // heh...
					{
						if(selected_ep)
							controls_to_ep(selected_ep,UD_COLOUR);
					}

					return	TRUE;

				case	IDC_COMBO1:
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						//	Update the event point.
						if(selected_ep)
							controls_to_ep(selected_ep,UD_TCOMBO);
					}
					return	TRUE;

				case	IDC_COMBO2:
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						if(SendMessage(GetDlgItem(hWnd,IDC_COMBO2),CB_GETCURSEL,0,0)==(OT_ACTIVE_TIME-1))
							EnableWindow(GetDlgItem(hWnd,IDC_EDIT4),TRUE);
						else
							EnableWindow(GetDlgItem(hWnd,IDC_EDIT4),FALSE);

						//	Update the event point.
						if(selected_ep)
							controls_to_ep(selected_ep,UD_OTCOMBO);
					}
					return	TRUE;

				case	IDC_EDIT1:
					if(HIWORD(wParam)==EN_UPDATE)
					{
						//	Get the new text.
						the_ctrl	=	(HWND)lParam;
						SendMessage(the_ctrl,WM_GETTEXT,2,(LPARAM)&edit_text);

						//	Make sure it's an alpha char.
						if(!isalpha(edit_text[0]))
						{
							//	Update the position.
							SendMessage	(
											GetDlgItem(hWnd,IDC_SPIN1),
											UDM_SETPOS,
											0,MAKELONG(waypoint_group,0)
										);

							//	Set the string.
							edit_text[0]	=	waypoint_group+'A';
							edit_text[1]	=	0;

							update	=	TRUE;
						}

						//	Set the text if necessary.
						if(update)
						{
							SendMessage(the_ctrl,WM_SETTEXT,0,(LPARAM)edit_text);
						}

						//	Select it regardless.
//						SendMessage(the_ctrl,EM_SETSEL,0,-1);

						//	Set the new group.
						waypoint_group	=	edit_text[0]-'A';

						//	Update the event point.
//						if(selected_ep)
//							controls_to_ep(selected_ep);
						return	TRUE;
					}
					break;

				case	IDC_EDIT2:
					if(selected_ep)
						controls_to_ep(selected_ep,UD_TRIGGEREDBY);
					return	TRUE;

				case	IDC_EDIT3:
					if(selected_ep)
						controls_to_ep(selected_ep,UD_DEPENDENCY);
					return	TRUE;

				case	IDC_EDIT4:
					if(HIWORD(wParam)==EN_CHANGE)
					{
						if(selected_ep)
						{
							controls_to_ep(selected_ep,UD_OTCOMBO);
						}
						return	TRUE;
					}
					break;

				case	IDC_CHECK1:
					if(SendMessage(GetDlgItem(hWnd,IDC_CHECK1),BM_GETCHECK,0,0)==BST_CHECKED)
						EnableWindow(GetDlgItem(hWnd,IDC_EDIT2),TRUE);
					else
						EnableWindow(GetDlgItem(hWnd,IDC_EDIT2),FALSE);

					if(selected_ep)
						controls_to_ep(selected_ep,UD_TRIGGEREDBY);
					return	TRUE;
*/
			}
			break;

		case	WM_NOTIFY:
			p_nmhdr	=	(NMHDR*)lParam;
			switch (p_nmhdr->idFrom) {
			case IDC_TAB1:
				if (p_nmhdr->code==TCN_SELCHANGE) TABCTL_sel(hWnd,IDC_TAB1);
				break;
			case IDC_WORKSPACE_TREE:
				if (p_nmhdr->code==TVN_SELCHANGED) {
				//	Deal with a workspace selected item.
					p_tview	=	(NM_TREEVIEW*)p_nmhdr;
					handle_ws_select((WSElement*)p_tview->itemNew.lParam);
					return	TRUE;
				}
				break;

			case IDC_WORKSPACE_TREE2:
				switch (p_nmhdr->code) {
				case NM_RCLICK:
					GetCursorPos(&click_point);
					hit_test.pt		=	click_point;
					ScreenToClient(wpt_tree,&hit_test.pt);
					item_handle		=	TreeView_HitTest(wpt_tree,&hit_test);
					if(item_handle && hit_test.flags&TVHT_ONITEM)
					{
						//	Yes, so get a pointer to its WSElement structure.
						the_item.hItem	=	item_handle;
						the_item.mask	=	TVIF_PARAM;
						if(TreeView_GetItem(ws_tree,&the_item))
						{
							if (the_item.lParam<100) break;
							EventPoint *was_ep = hilited_ep, *ep = (EventPoint*)the_item.lParam;
							if (ep!=selected_ep) {
								ws_sel_waypoint(ep);
								process_view_wind();
							}
							hilited_ep=selected_ep;
							SendMessage(GEDIT_view_wnd,WM_CONTEXTMENU,(WPARAM)hWnd,MAKELONG(click_point.x,click_point.y));
//							hilited_ep=was_ep;
/*							ws_menu	=	GetSubMenu(LoadMenu(GEDIT_hinstance,MAKEINTRESOURCE(IDR_GEDIT_POPUPS)),3);
							TrackPopupMenu	(
												ws_menu,
												TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
												click_point.x,click_point.y,
												0,GEDIT_view_wnd,NULL
											);
							DestroyMenu(ws_menu);						*/
						}
					}
					break;
				case TVN_SELCHANGED:
					p_tview	=	(NMTREEVIEW*)p_nmhdr;
					if (p_tview->itemNew.lParam>100) {
						selected_ep=(EventPoint*)p_tview->itemNew.lParam;
						ep_to_controls2(selected_ep);
						if (!leaping_disabled) {
							cam_focus_x=selected_ep->X;
							cam_focus_z=selected_ep->Z;
						}
					}
//					handle_ws_select((WSElement*)p_tview->itemNew.lParam);
					break;
				}
				break;

			case IDC_TOOLBAR1:
				if (p_nmhdr->code==TBN_GETINFOTIP)
				{
					LPNMTBGETINFOTIP tip = (LPNMTBGETINFOTIP) lParam;
					switch(tip->iItem) {
					case ID_TOOLBAR_WAYPOINT:
						strncpy(tip->pszText,"Edit Waypoints",tip->cchTextMax);
						break;
					case ID_TOOLBAR_ZONE:
						strncpy(tip->pszText,"Edit Zones",tip->cchTextMax);
						break;
					case ID_TOOLBAR_PRIM:
						strncpy(tip->pszText,"Edit Prims",tip->cchTextMax);
						break;
					}
				}
				break;
			}
			break;

		case	WM_CONTEXTMENU:
			click_point.x	=	LOWORD(lParam);
			click_point.y	=	HIWORD(lParam);
			if(wParam==(WPARAM)ws_tree)
			{
				// Pass the context click info onto the Workspace.
				handle_ws_context(&click_point);
				return	TRUE;
			}
			break;

		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			if(ED_KEYS)
				return	TRUE;
			break;

		case	WM_USER:
			process_view_wind();
			return	TRUE;
	}

	return	FALSE;
}


#endif //#ifdef EDITOR


//---------------------------------------------------------------
/*
void	controls_to_ep(EventPoint *ep,ULONG flags)
{
	SBYTE			wp_type;

	return; // move along, there's nothing to see here

	//	Get type.
	if(flags&UD_TCOMBO)
		ep->WaypointType	=	SendMessage	(
												GetDlgItem(GEDIT_edit_wnd,IDC_COMBO1),
												CB_GETCURSEL,
												0,0
											) + 1;

	//	Get dependency.
	if(flags&UD_DEPENDENCY)
	{
		ep->EPRef	=	SendMessage	(
										GetDlgItem(GEDIT_edit_wnd,IDC_SPIN3),
										UDM_GETPOS,
										0,0
									);
	}

	//	Get ID.
	if(flags&UD_COLOUR)
		ep->Colour		=	waypoint_colour;
	if(flags&UD_GROUP)
		ep->Group		=	waypoint_group;

	//	Get triggered by.
	if(flags&UD_TRIGGEREDBY)
	{
		if(SendMessage(GetDlgItem(GEDIT_edit_wnd,IDC_CHECK1),BM_GETCHECK,0,0)==BST_CHECKED)
		{
			ep->TriggeredBy	=	TB_PROXIMITY;
			ep->Radius		=	SendMessage	(
												GetDlgItem(GEDIT_edit_wnd,IDC_SPIN2),
												UDM_GETPOS,
												0,0
											);
		}
		else
			ep->TriggeredBy	=	TB_NONE;
	}

	//	Get on trigger.
	if(flags&UD_OTCOMBO)
	{
		ep->OnTrigger	=	SendMessage	(
											GetDlgItem(GEDIT_edit_wnd,IDC_COMBO2),
											CB_GETCURSEL,
											0,0
										) + 1;
		ep->Data[9]		=	SendMessage	(
											GetDlgItem(GEDIT_edit_wnd,IDC_SPIN4),
											UDM_GETPOS,
											0,0
										);
	}

	WaypointCaption(ep);

}
*/
//---------------------------------------------------------------
/*
void	ep_to_controls(EventPoint *ep,ULONG flags)
{
	CBYTE			edit_text[2];
	WPARAM			w_param;
	CBYTE			msg[255];

	return; // move along, there's nothing to see here

	WaypointCaption(ep);

	//	Set 'Type' combo.
	if(flags&UD_TCOMBO)
		SendMessage	(
						GetDlgItem(GEDIT_edit_wnd,IDC_COMBO1),
						CB_SETCURSEL,
						ep->WaypointType-1,
						0
					);

	//	Set dependency.
	if(flags&UD_DEPENDENCY)
	{
		SendMessage	(
						GetDlgItem(GEDIT_edit_wnd,IDC_SPIN3),
						UDM_SETPOS,
						0,
						MAKELONG(ep->EPRef,0)
					);
		if(!ep->EPRef)
		{
			SendMessage	(
							GetDlgItem(GEDIT_edit_wnd,IDC_EDIT3),
							WM_SETTEXT,
							0,(LPARAM)"Startup"
						);
		}
	}

	//	Set the colour.
	if(flags&UD_COLOUR)
		SendMessage	(
						GEDIT_edit_wnd,
						WM_COMMAND,
						IDC_CUSTOM_1+ep->Colour,
						(LPARAM)GetDlgItem(GEDIT_edit_wnd,IDC_CUSTOM_1+ep->Colour)
					);

	//	Set the group.
	if(flags&UD_GROUP)
	{
		waypoint_group	=	ep->Group;
		edit_text[0]	=	waypoint_group+'A';
		edit_text[1]	=	0;
		SendMessage	(
						GetDlgItem(GEDIT_edit_wnd,IDC_EDIT1),
						WM_SETTEXT,
						0,
						(LPARAM)edit_text
					);

		SendMessage	(	
						//	This will submit an invalid char to the edit control & force
						//	the correct group update.
						GetDlgItem(GEDIT_edit_wnd,IDC_EDIT1),
						WM_SETTEXT,
						0,
						(LPARAM)" "
					);
	}

	//	Set triggered by.
	if(flags&UD_TRIGGEREDBY)
	{
		switch(ep->TriggeredBy)
		{
			case	TB_PROXIMITY:
				SendMessage(GetDlgItem(GEDIT_edit_wnd,IDC_CHECK1),BM_SETCHECK,TRUE,0);
				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT2),TRUE);
//				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_COMBO2),TRUE);
//				if(ep->OnTrigger==OT_ACTIVE_TIME)
//					flags	|=	UD_OTCOMBO;
				break;
			default:
				SendMessage(GetDlgItem(GEDIT_edit_wnd,IDC_CHECK1),BM_SETCHECK,FALSE,0);
				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT2),FALSE);
//				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT4),FALSE);
//				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_COMBO2),FALSE);
		}
		SendMessage	(
						GetDlgItem(GEDIT_edit_wnd,IDC_SPIN2),
						UDM_SETPOS,
						0,
						MAKELONG(ep->Radius,0)
					);
	}

	//	Set on trigger.
	if(flags&UD_OTCOMBO)
	{
		switch(ep->OnTrigger)
		{
			case	OT_NONE:
				//	Default to OT_ACTIVE.
				ep->OnTrigger	=	OT_ACTIVE;
				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT4),FALSE);
				break;
			case	OT_ACTIVE:
				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT4),FALSE);
				break;
			case	OT_ACTIVE_WHILE:
				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT4),FALSE);
				break;
			case	OT_ACTIVE_TIME:
				EnableWindow(GetDlgItem(GEDIT_edit_wnd,IDC_EDIT4),TRUE);
				break;
		}
		SendMessage	(
						GetDlgItem(GEDIT_edit_wnd,IDC_COMBO2),
						CB_SETCURSEL,
						ep->OnTrigger-1,
						0
					);
		SendMessage	(
						GetDlgItem(GEDIT_edit_wnd,IDC_SPIN4),
						UDM_SETPOS,
						0,
						MAKELONG(ep->Data[9],0)
					);
	}
}
*/
//---------------------------------------------------------------

void	menu_no_workspace(void)
{
	EnableMenuItem(GEDIT_main_menu,ID_FILE_NEW_WS,MF_ENABLED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_OPEN_WS,MF_ENABLED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_CLOSE_WS,MF_GRAYED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_SAVE_WS,MF_GRAYED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_SAVE_WS_AS,MF_GRAYED);
}

//---------------------------------------------------------------

void	menu_has_workspace(void)
{
	EnableMenuItem(GEDIT_main_menu,ID_FILE_NEW_WS,MF_ENABLED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_OPEN_WS,MF_ENABLED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_CLOSE_WS,MF_ENABLED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_SAVE_WS,MF_ENABLED);
	EnableMenuItem(GEDIT_main_menu,ID_FILE_SAVE_WS_AS,MF_ENABLED);
}

//---------------------------------------------------------------

void	menu_workspace_changed(void)
{
//	EnableMenuItem(GEDIT_main_menu,ID_FILE_SAVE_WS,MF_ENABLED);
}

//---------------------------------------------------------------

void	menu_workspace_saved(void)
{
//	EnableMenuItem(GEDIT_main_menu,ID_FILE_SAVE_WS,MF_GRAYED);
}

//---------------------------------------------------------------


/****************************************************************
 *		Version 2.0 control<-->ep routines
 */

void ep_to_controls2(EventPoint *ep, SWORD tabpage, HWND tab) {
	HWND ctl;
	CBYTE msg[800];

	if (!tab) tab=TABCTL_gethwnd(GEDIT_edit_wnd,IDC_TAB1);
//	if ((!ep)||(!tab)) return;
	if (!tab) return;
	if (!ep) ep=&dummy_ep;

	if (tabpage==-1) tabpage=TABCTL_getsel(GEDIT_edit_wnd, IDC_TAB1);

	switch (tabpage) {
	case 0: // waypoint info
		SendMessage(GetDlgItem(tab,IDC_COMBO1),CB_SETCURSEL,ep->Colour,0);
		SendMessage(GetDlgItem(tab,IDC_COMBO2),CB_SETCURSEL,ep->Group,0);
		waypoint_colour = ep->Colour;
		SendMessage(GetDlgItem(tab,IDC_SPIN1),UDM_SETPOS,0,MAKELONG(ep->Y-PAP_calc_map_height_at(ep->X, ep->Z),0));
		break;
	case 1:
		SendMessage(GetDlgItem(tab,IDC_COMBO1),CB_SETCURSEL,ep->TriggeredBy,0);
		condition_tab_update(tab);
		SendMessage(GetDlgItem(tab,IDC_SPIN1),UDM_SETPOS,0,MAKELONG((short) ep->EPRef,0));
		SendMessage(GetDlgItem(tab,IDC_SPIN2),UDM_SETPOS,0,MAKELONG((short) ep->EPRefBool,0));
//		SendMessage(GetDlgItem(tab,IDC_SPIN4),UDM_SETPOS,0,MAKELONG((short) ep->Radius,0));
//		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_SPIN3)))

		SendMessage(GetDlgItem(tab,IDC_CHECK1),BM_SETCHECK,(ep->Flags&WPT_FLAGS_INVERSE)>>1,0);
		SendMessage(GetDlgItem(tab,IDC_CHECK2),BM_SETCHECK,((ep->Flags&WPT_FLAGS_OPTIONAL)?1:0),0);

		if (WaypointUses[ep->TriggeredBy] & WPU_TIME)
			SendMessage(GetDlgItem(tab,IDC_SPIN3),UDM_SETPOS,0,MAKELONG((short) (ep->Radius/100),0));	// timer and 

		if (WaypointUses[ep->TriggeredBy] & WPU_COUNTER)
			SendMessage(GetDlgItem(tab,IDC_SPIN3),UDM_SETPOS,0,MAKELONG((short) ep->Radius,0));		// counter and

		if (WaypointUses[ep->TriggeredBy] & WPU_RADIUS)
			SendMessage(GetDlgItem(tab,IDC_SPIN4),UDM_SETPOS,0,MAKELONG((short) ep->Radius,0));		// radius and

		msg[0]=0;
		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_EDIT5)))
			SendMessage(ctl,WM_SETTEXT,0,(long)ep->Radius);		// shout are mutually exclusive...
		else
			SendMessage(ctl,WM_SETTEXT,0,(long)msg);

		break;
	case 2:
		SendMessage(GetDlgItem(tab,IDC_COMBO1),CB_SETCURSEL,ep->WaypointType-1,0);
		SendMessage(GetDlgItem(tab,IDC_COMBO2),CB_SETCURSEL,ep->OnTrigger-1,0);
		WaypointExtra(ep,msg);
		SendMessage(GetDlgItem(tab,IDC_STATIC_LABEL),WM_SETTEXT,0,(long)msg);
		action_tab_update(tab);
		SendMessage(GetDlgItem(tab,IDC_SPIN1),UDM_SETPOS,0,MAKELONG((short) ep->AfterTimer,0));
		break;
	}
}

BOOL UseCheck(SLONG newtype, SLONG oldtype, SLONG mask) {
  return ((WaypointUses[newtype] & mask)&&!(WaypointUses[oldtype] & mask));
}

void controls_to_ep2(EventPoint *ep, SWORD tabpage, HWND tab) {
//	HWND ctl;
	SLONG ndx;

	if (!tab) tab=TABCTL_gethwnd(GEDIT_edit_wnd,IDC_TAB1);
//	if ((!ep)||(!tab)) return;
	if (!tab) return;
	if (!ep) ep=&dummy_ep;

	if (tabpage==-1) tabpage=TABCTL_getsel(GEDIT_edit_wnd, IDC_TAB1);

	workspace_changed=TRUE;

	switch (tabpage) {
	case 0: // waypoint info
		ep->Colour		= SendMessage(GetDlgItem(tab,IDC_COMBO1),CB_GETCURSEL,0,0);
		ep->Group		= SendMessage(GetDlgItem(tab,IDC_COMBO2),CB_GETCURSEL,0,0);
		waypoint_colour = ep->Colour;
		break;
	case 1:
		ndx=SendMessage(GetDlgItem(tab,IDC_COMBO1),CB_GETCURSEL,0,0);
/*		if ( ((ndx==TT_SHOUT_ALL)||(ndx==TT_SHOUT_ANY))
			&& ((ep->TriggeredBy!=TT_SHOUT_ANY)&&(ep->TriggeredBy!=TT_SHOUT_ANY)) )
			ep->Radius = (SLONG) malloc(_MAX_PATH);
		if ( ((ndx!=TT_SHOUT_ALL)&&(ndx!=TT_SHOUT_ANY))
			&& ((ep->TriggeredBy==TT_SHOUT_ALL)||(ep->TriggeredBy==TT_SHOUT_ANY)) )
			free((void*)ep->Radius);*/

		if (UseCheck(ndx,ep->TriggeredBy,WPU_RADTEXT))
			ep->Radius = (SLONG) malloc(_MAX_PATH);

		if (UseCheck(ep->TriggeredBy,ndx,WPU_RADTEXT))
			free((void*)ep->Radius);

/*		if (UseCheck(ndx,ep->TriggeredBy,WPU_RADBOX))
			ep->Radius=0;*/

		ep->EPRef=ep->EPRefBool=0;

		ep->TriggeredBy=ndx;

//		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_SPIN1)))
		if (WaypointUses[ndx] & (WPU_DEPEND|WPU_COUNTER))
			ep->EPRef		= LOWORD(SendMessage(GetDlgItem(tab,IDC_SPIN1),UDM_GETPOS,0,0));	


		ep->Flags&=~(WPT_FLAGS_INVERSE|WPT_FLAGS_OPTIONAL);
		ep->Flags|=(SendMessage(GetDlgItem(tab,IDC_CHECK1),BM_GETCHECK,0,0) ? WPT_FLAGS_INVERSE : 0);
		ep->Flags|=(SendMessage(GetDlgItem(tab,IDC_CHECK2),BM_GETCHECK,0,0) ? WPT_FLAGS_OPTIONAL : 0);


//		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_SPIN2)))
		if (WaypointUses[ndx] & WPU_BOOLEAN)
			ep->EPRefBool	= LOWORD(SendMessage(GetDlgItem(tab,IDC_SPIN2),UDM_GETPOS,0,0));	

//		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_SPIN3)))
		if (WaypointUses[ndx] & WPU_TIME)
//			ep->Radius	= LOWORD(SendMessage(GetDlgItem(tab,IDC_SPIN3),UDM_GETPOS,0,0))*100;	// timer and 
			ep->Radius	= (SendMessage(GetDlgItem(tab,IDC_SPIN3),UDM_GETPOS,0,0)&0xffff)*100;	// timer and 

//		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_SPIN4)))
		if (WaypointUses[ndx] & WPU_COUNTER)
			ep->Radius	= LOWORD(SendMessage(GetDlgItem(tab,IDC_SPIN3),UDM_GETPOS,0,0));		// radius are

		if (WaypointUses[ndx] & WPU_RADIUS)
			ep->Radius	= LOWORD(SendMessage(GetDlgItem(tab,IDC_SPIN4),UDM_GETPOS,0,0));		// radius are

//		if (IsWindowEnabled(ctl=GetDlgItem(tab,IDC_EDIT5)))
		if (WaypointUses[ndx] & WPU_RADTEXT)
			SendMessage(GetDlgItem(tab,IDC_EDIT5),WM_GETTEXT,_MAX_PATH,ep->Radius);		// shout are mutually exclusive

		break;
	case 2:
		ndx=SendMessage(GetDlgItem(tab,IDC_COMBO1),CB_GETCURSEL,0,0)+1;
		ep->OnTrigger	= SendMessage(GetDlgItem(tab,IDC_COMBO2),CB_GETCURSEL,0,0)+1;
		ep->AfterTimer	= LOWORD(SendMessage(GetDlgItem(tab,IDC_SPIN1),UDM_GETPOS,0,0));
		{
			CBYTE msg[20];
			sprintf(msg,"timer: %d",ep->AfterTimer);
//			CONSOLE_text(msg,1000);
		}
		if (ndx!=ep->WaypointType) {
			if ((ep->WaypointType==WPT_CREATE_ENEMIES)&&(ndx==WPT_ADJUST_ENEMY)) {
			} else {
				CleanProperties(ep);
			}
			leaping_disabled=1;
			ws_set_waypoint(ep,ndx);
			ws_sel_waypoint(ep);
			leaping_disabled=0;
		}
//		ep->WaypointType= ndx;
		break;
	}
}

