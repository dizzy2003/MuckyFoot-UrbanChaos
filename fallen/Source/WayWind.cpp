//	WayWind.cpp
//	Guy Simmons, 31st July 1998.


#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"EdStrings.h"
#include	"waywind.h"


//---------------------------------------------------------------


int						waypoint_colour,
						waypoint_group;
UBYTE					button_colours[WAY_COLOURS][3]	=	{
																{	0,		0,		0	},
																{	255,	255,	255	},
																{	255,	0,		0	},
																{	255,	255,	0	},
																{	0,		255,	0	},
																{	0,		255,	255	},
																{	0,		0,		255	},
																{	255,	0,		255	},
																{	238,	176,	176 },
																{	139,	112,	85	},
																{	127,	76,		180 },
																{	76,		196,	174 },
																{	195,	52,		3	},
																{	171,	249,	167	},
																{	168,	178,	54	}
															};
TCHAR					button_classes[WAY_COLOURS][_MAX_PATH];


extern HCURSOR			GEDIT_arrow;
extern HINSTANCE		GEDIT_hinstance;
extern HWND				GEDIT_client_wnd,
						GEDIT_engine_wnd,
						GEDIT_frame_wnd,
						GEDIT_way_wnd;

//---------------------------------------------------------------

void	update_combos(HWND parent)
{
/*
	int			c0,
				selected;
	HWND		the_combo;


	//	Get a handle to the main combo & the currently selected item.
	the_combo	=	GetDlgItem(parent,IDC_COMBO1);
	selected	=	SendMessage(the_combo,CB_GETCURSEL,0,0);

	//	Now set the state of all the sub combos.
	for(c0=IDC_COMBO2;c0<=IDC_COMBO8;c0++)
	{
		the_combo	=	GetDlgItem(parent,c0);
		if(((c0-IDC_COMBO2)+1)==selected)
			EnableWindow(the_combo,TRUE);
		else
			EnableWindow(the_combo,FALSE);
	}

*/
}

//---------------------------------------------------------------

#define	INIT_COMBO_BOX(i,s)			the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}							

BOOL CALLBACK		waypoint_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
/*
	BOOL				update	=	FALSE;
	int					c0,
						ctrl_id,
						sel_id;
	HBRUSH				outline_brush;
	HDC					hdc;
	HWND				the_ctrl;
	LPDRAWITEMSTRUCT	draw_item;
	PAINTSTRUCT			ps;
	POINT				bottom_right,
						top_left;
	RECT				button_rect,
						caption_rect;
	LPTSTR				lbitem_str;
	TCHAR				edit_text[MAX_PATH];


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Init the 'Action' combos.
			INIT_COMBO_BOX(IDC_COMBO1,wtype_strings)
			INIT_COMBO_BOX(IDC_COMBO2,wplayer_strings)
			INIT_COMBO_BOX(IDC_COMBO3,wenemy_strings)
			INIT_COMBO_BOX(IDC_COMBO4,witem_strings)
			INIT_COMBO_BOX(IDC_COMBO5,wsfx_strings)
			INIT_COMBO_BOX(IDC_COMBO6,wvfx_strings)
			INIT_COMBO_BOX(IDC_COMBO7,wcscenes_strings)
			INIT_COMBO_BOX(IDC_COMBO8,wmessage_strings)
			update_combos(hWnd);

			//	Init the edit thingy.
			the_ctrl	=	GetDlgItem(hWnd,IDC_EDIT1);
			SendMessage(the_ctrl,EM_SETLIMITTEXT,1,0);
			SendMessage(the_ctrl,WM_SETTEXT,0,(LPARAM)"A");

			return	TRUE;

		case	WM_VSCROLL:
			if(GetWindowLong((HWND)lParam,GWL_ID)==IDC_SCROLLBAR1)
			{
				//	Make the 'character' go up or down.
				the_ctrl	=	GetDlgItem(hWnd,IDC_EDIT1);
				SendMessage(the_ctrl,WM_GETTEXT,2,(LPARAM)&edit_text);
				switch(LOWORD(wParam))
				{
					case	SB_LINEDOWN:
						edit_text[0]--;
						if(edit_text[0]<'A')
							edit_text[0]	=	'Z';
						SendMessage(the_ctrl,WM_SETTEXT,0,(LPARAM)edit_text);
						SendMessage(the_ctrl,EM_SETSEL,0,-1);
						return	TRUE;
					case	SB_LINEUP:
						edit_text[0]++;
						if(edit_text[0]>'Z')
							edit_text[0]	=	'A';
						SendMessage(the_ctrl,WM_SETTEXT,0,(LPARAM)edit_text);
						SendMessage(the_ctrl,EM_SETSEL,0,-1);
						return	TRUE;
				}
			}
			break;

		case	WM_DRAWITEM:
		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDC_CUSTOM1:
				case	IDC_CUSTOM2:
				case	IDC_CUSTOM3:
				case	IDC_CUSTOM4:
				case	IDC_CUSTOM5:
				case	IDC_CUSTOM6:
				case	IDC_CUSTOM7:
				case	IDC_CUSTOM8:

					hdc				=	GetDC(hWnd);

					//	Erase the old selected colour box.
					the_ctrl		=	GetDlgItem(hWnd,waypoint_colour+IDC_CUSTOM1);
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
					waypoint_colour	=	LOWORD(wParam)-IDC_CUSTOM1;
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

					return	TRUE;

				case	IDC_COMBO1:
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						update_combos(hWnd);
					}
					return	TRUE;

				case	IDC_EDIT1:
					if(HIWORD(wParam)==EN_UPDATE)
					{
						//	Get the new text.
						the_ctrl	=	(HWND)lParam;
						SendMessage(the_ctrl,WM_GETTEXT,2,(LPARAM)&edit_text);
						
						//	Make sure the text is in range.
						if(edit_text[0]<'A' || edit_text[0]>'z' || (edit_text[0]>'Z' && edit_text[0]<'a'))
						{
							edit_text[0]	=	waypoint_group+'A';
							update	=	TRUE;
						}

						//	Now make sure it's upper case.
						if(islower(edit_text[0]))
						{
							edit_text[0]	=	toupper(edit_text[0]);
							update	=	TRUE;
						}

						//	Set the text if necessary.
						if(update)
							SendMessage(the_ctrl,WM_SETTEXT,0,(LPARAM)edit_text);

						//	Select it regardless.
						SendMessage(the_ctrl,EM_SETSEL,0,-1);

						//	Set the new group.
						waypoint_group	=	edit_text[0]-'A';

						return	TRUE;
					}
					break;
			}
			break;
	}
*/
	return	FALSE;
}

//---------------------------------------------------------------

LRESULT	CALLBACK	button_proc	(
									HWND hWnd,
									UINT message,
									WPARAM wParam,
									LPARAM lParam
								)
{
	DRAWITEMSTRUCT		draw_item;


	switch(message)
	{
		case	WM_LBUTTONDOWN:
			//	Notify the parent that this colour box has been clicked.
			SendMessage	(
							GetParent(hWnd),
							WM_COMMAND,
							(BN_CLICKED<<16)|GetWindowLong(hWnd,GWL_ID),
							(LPARAM)hWnd
						);
			return	0;

		case	WM_PAINT:
			//	Draw the colour box & if it's the selected box, force an update.
			DefWindowProc(hWnd,message,wParam,lParam);
			if(GetDlgCtrlID(hWnd)==waypoint_colour+IDC_CUSTOM_1)
			{
				draw_item.CtlID			=	waypoint_colour+IDC_CUSTOM_1;
				draw_item.itemAction	=	ODA_DRAWENTIRE;
				SendMessage	(
								GetParent(hWnd),
								WM_DRAWITEM,
								waypoint_colour+IDC_CUSTOM_1,
								(LPARAM)&draw_item
							);
			}
			return	0;

	}
	return	DefWindowProc(hWnd,message,wParam,lParam);
}

//---------------------------------------------------------------

BOOL	init_wwind(void)
{
	int				c0;
	WNDCLASSEX		new_class;


	//	Initialise globals.
	waypoint_colour	=	0;

	//	Create the button classes.
	for(c0=0;c0<WAY_COLOURS;c0++)
	{
		//	Create the class name.
		sprintf(button_classes[c0],"BUTTON_CLASS%d",c0);

		//	create the button classes.
		new_class.cbSize		=	sizeof(WNDCLASSEX);
		new_class.style			=	CS_PARENTDC|CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
		new_class.lpfnWndProc	=	button_proc;
		new_class.cbClsExtra	=	0;
		new_class.cbWndExtra	=	0;
		new_class.hInstance		=	GEDIT_hinstance;
		new_class.hIcon			=	NULL;
		new_class.hCursor		=	GEDIT_arrow;
		new_class.hbrBackground	=	CreateSolidBrush(
														RGB	(
																button_colours[c0][0],
																button_colours[c0][1],
																button_colours[c0][2]
															)
													);
		new_class.lpszMenuName	=	NULL;
		new_class.lpszClassName	=	button_classes[c0];
		new_class.hIconSm		=	NULL;
		if(!RegisterClassEx(&new_class))
			return	FALSE;		//	Couldn't register the class.
	}

	return	TRUE;
}

//---------------------------------------------------------------

void	fini_wwind(void)
{
	int			c0;


	for(c0=0;c0<WAY_COLOURS;c0++)
	{
		UnregisterClass(button_classes[c0],GEDIT_hinstance);
	}
}

//---------------------------------------------------------------

