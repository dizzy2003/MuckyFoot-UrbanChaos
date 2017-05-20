#include <MFStdLib.h>
#include <windows.h>
#include <windowsx.h>
#include <ddlib.h>
#include <commctrl.h>
#include <zmouse.h>		// Mouse wheel support
#include "resource.h"
#include "gi.h"
#include "fmatrix.h"
#include "pap.h"
#include "inline.h"
#include "ed.h"
#include "game.h"
#include "inside2.h"
#include "memory.h"

extern UWORD	calc_inside_for_xyz(SLONG x,SLONG y,SLONG z,UWORD *room);

HINSTANCE LEDIT_hinstance;

WNDCLASSEX LEDIT_class_frame;
WNDCLASSEX LEDIT_class_engine;
WNDCLASSEX LEDIT_class_light;
WNDCLASSEX LEDIT_class_colour;

HWND LEDIT_handle_frame;
HWND LEDIT_handle_engine;
HWND LEDIT_handle_light;
HWND LEDIT_handle_red;
HWND LEDIT_handle_green;
HWND LEDIT_handle_blue;
HWND LEDIT_handle_range;
HWND LEDIT_handle_anti;
HWND LEDIT_handle_colour;
HWND LEDIT_handle_bright;
HWND LEDIT_handle_bwhite;
HWND LEDIT_handle_blgrey;
HWND LEDIT_handle_bdgrey;
HWND LEDIT_handle_bpyellow;
HWND LEDIT_handle_bpblue;
HWND LEDIT_handle_bpred;

CBYTE *LEDIT_name_frame  = "Urban Chaos lighting editor";
CBYTE *LEDIT_name_engine = "Engine view";
CBYTE *LEDIT_name_light  = "Light info";
CBYTE *LEDIT_name_colour = "Colour box";

HMENU  LEDIT_main_menu;
HACCEL LEDIT_accel;

//
// Our cursors and icons.
//

HCURSOR LEDIT_arrow;
HCURSOR LEDIT_busy_bee;
HCURSOR LEDIT_all_dirs;
HCURSOR LEDIT_upndown;
HICON   LEDIT_icon;

//
// The colour the brightness trackbar is offset from.
// 

SLONG LEDIT_bright_base_red;
SLONG LEDIT_bright_base_green;
SLONG LEDIT_bright_base_blue;

//
// The current map.
// 

CBYTE LEDIT_map_name[_MAX_PATH];
SLONG LEDIT_map_valid;		// TRUE => A map is loaded.

//
// The program default directory.
//

CBYTE LEDIT_default_dir[_MAX_PATH];

//
// The mousewheel message.
//

UINT LEDIT_wm_mousewheel;

//
// Whats going on at the moment?
//

SLONG LEDIT_mode;

#define LEDIT_MODE_NOTHING		0
#define LEDIT_MODE_PLACE_LIGHT	1
#define LEDIT_MODE_EDIT_LIGHT	2
#define LEDIT_MODE_SET_AMBIENT	3
#define LEDIT_MODE_SET_LAMPOST	4
#define LEDIT_MODE_SET_SKY		5

UBYTE LEDIT_insides;

//
// The light we are editing.
// The last light we placed down.
//

SLONG LEDIT_edit_light;
SLONG LEDIT_edit_dragging;	// Dragging the edit light.
SLONG LEDIT_edit_dragged;
SLONG LEDIT_edit_drag_dx;
SLONG LEDIT_edit_drag_dy;
SLONG LEDIT_edit_drag_dz;

//
// The last light we placed down.
// 

SLONG LEDIT_last_placed;

//
// The mouse in the world.
//

SLONG LEDIT_mouse_valid;
SLONG LEDIT_mouse_over;
SLONG LEDIT_mouse_world_x;
SLONG LEDIT_mouse_world_y;
SLONG LEDIT_mouse_world_z;
SLONG LEDIT_mouse_light;

//
// Our file access structures.
//

OPENFILENAME LEDIT_ofn_map;
OPENFILENAME LEDIT_ofn_light;

CBYTE LEDIT_ofn_default_dir_map  [_MAX_PATH];
CBYTE LEDIT_ofn_default_dir_light[_MAX_PATH];

CBYTE LEDIT_ofn_file_map  [_MAX_PATH];
CBYTE LEDIT_ofn_file_light[_MAX_PATH];


//
// The camera.
//

SLONG LEDIT_cam_x;
SLONG LEDIT_cam_y;
SLONG LEDIT_cam_z;
SLONG LEDIT_cam_yaw;
SLONG LEDIT_cam_pitch;
SLONG LEDIT_cam_focus_x;
SLONG LEDIT_cam_focus_z;
SLONG LEDIT_cam_focus_dist;

SLONG LEDIT_cam_matrix [9];
SLONG LEDIT_cam_forward[3];	// The movement vector forward
SLONG LEDIT_cam_left   [3]; // The movement vector left

//
// The ID of the child controls.
//

#define LEDIT_CHILD_ENGINE		1
#define LEDIT_CHILD_LIGHT		2
#define LEDIT_CHILD_RED			3
#define LEDIT_CHILD_GREEN		4
#define LEDIT_CHILD_BLUE		5
#define LEDIT_CHILD_RANGE		6
#define LEDIT_CHILD_COLOUR		7
#define LEDIT_CHILD_ANTI		8
#define LEDIT_CHILD_BWHITE		9
#define LEDIT_CHILD_BLGREY		10
#define LEDIT_CHILD_BDGREY		11
#define LEDIT_CHILD_BPYELLOW	12
#define LEDIT_CHILD_BPBLUE		13
#define LEDIT_CHILD_BPRED		14
#define LEDIT_CHILD_BRIGHT		15


//
// Changes the colour we are editing.
// 

void LEDIT_change_colour(COLORREF cr);



//
// Sets everything up to look correct for the current state.
//

void LEDIT_set_state_look(void)
{
	SLONG enable_flag;

	//
	// Uncheck all menu items.
	//

	CheckMenuItem(LEDIT_main_menu, ID_MAP_NIGHTSKY,                 MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_MAP_DAYTIME,                  MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_MAP_LIGHTSUNDERLAMPOSTS,      MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_MAP_DARKENBOTTOMSOFBUILDINGS, MF_UNCHECKED);

	CheckMenuItem(LEDIT_main_menu, ID_EDIT_PLACELIGHT,       MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_EDIT_EDITLIGHTS,       MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_EDIT_SETAMBIENT,       MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_EDIT_SETLAMPOSTCOLOUR, MF_UNCHECKED);
	CheckMenuItem(LEDIT_main_menu, ID_EDIT_SETSKYCOLOUR,     MF_UNCHECKED);

	//
	// Name of the map in the engine window.
	//

	SetWindowText(LEDIT_handle_engine, LEDIT_map_name);

	// Validate some menuitems

	enable_flag = MF_BYCOMMAND | (LEDIT_map_valid ? MF_ENABLED : MF_DISABLED);
	EnableMenuItem(LEDIT_main_menu, ID_EDIT_INSIDES, enable_flag);

	// question for mark... why are all these done individually instead of setting a flag?

	if (LEDIT_map_valid)
	{
		//
		// Validate the menuitems.
		//

		EnableMenuItem(LEDIT_main_menu, ID_FILE_LOAD_ARSE, MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_FILE_SAVE_ARSE, MF_ENABLED);

		EnableMenuItem(LEDIT_main_menu, ID_MAP_NIGHTSKY,                 MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_MAP_DAYTIME,                  MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_MAP_LIGHTSUNDERLAMPOSTS,      MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_MAP_DARKENBOTTOMSOFBUILDINGS, MF_ENABLED);

		EnableMenuItem(LEDIT_main_menu, ID_EDIT_CLEARALL,         MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_DELETE_ARSE,      MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_PLACELIGHT,       MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_EDITLIGHTS,       MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_SETAMBIENT,       MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_SETLAMPOSTCOLOUR, MF_ENABLED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_SETSKYCOLOUR,     MF_ENABLED);

		//
		// Show the windows.
		//

		ShowWindow(LEDIT_handle_engine, SW_SHOW);
		ShowWindow(LEDIT_handle_light,  SW_SHOW);
	}
	else
	{
		//
		// Invalidate most of the menuitems.
		//

		EnableMenuItem(LEDIT_main_menu, ID_FILE_LOAD_ARSE, MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_FILE_SAVE_ARSE, MF_GRAYED);

		EnableMenuItem(LEDIT_main_menu, ID_MAP_NIGHTSKY,                 MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_MAP_DAYTIME,                  MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_MAP_LIGHTSUNDERLAMPOSTS,      MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_MAP_DARKENBOTTOMSOFBUILDINGS, MF_GRAYED);

		EnableMenuItem(LEDIT_main_menu, ID_EDIT_CLEARALL,         MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_DELETE_ARSE,      MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_PLACELIGHT,       MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_EDITLIGHTS,       MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_SETAMBIENT,       MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_SETLAMPOSTCOLOUR, MF_GRAYED);
		EnableMenuItem(LEDIT_main_menu, ID_EDIT_SETSKYCOLOUR,     MF_GRAYED);

		//
		// Hide the windows.
		//

		ShowWindow(LEDIT_handle_engine, SW_HIDE);
		ShowWindow(LEDIT_handle_light,  SW_HIDE);

		return;
	}

	//
	// Undo/redo?
	//

	EnableMenuItem(LEDIT_main_menu, ID_EDIT_UNDO_ARSE, (ED_undo_undo_valid()) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(LEDIT_main_menu, ID_EDIT_REDO_ARSE, (ED_undo_redo_valid()) ? MF_ENABLED : MF_GRAYED);

	//
	// Things that depend on what we are doing.
	//

	EnableWindow(LEDIT_handle_bwhite,   TRUE);
	EnableWindow(LEDIT_handle_blgrey,   TRUE);
	EnableWindow(LEDIT_handle_bdgrey,   TRUE);
	EnableWindow(LEDIT_handle_bpyellow, TRUE);
	EnableWindow(LEDIT_handle_bpblue,   TRUE);
	EnableWindow(LEDIT_handle_bpred,    TRUE);
	
	EnableMenuItem(LEDIT_main_menu, ID_EDIT_DELETE_ARSE, MF_GRAYED);

	switch(LEDIT_mode)
	{
		case LEDIT_MODE_NOTHING:
			break;

		case LEDIT_MODE_PLACE_LIGHT:
			
			SetWindowText(LEDIT_handle_light, "Values to place a light with");

			EnableWindow(LEDIT_handle_red,    TRUE);
			EnableWindow(LEDIT_handle_green,  TRUE);
			EnableWindow(LEDIT_handle_blue,   TRUE);
			EnableWindow(LEDIT_handle_bright, TRUE);
			EnableWindow(LEDIT_handle_range,  TRUE);
			EnableWindow(LEDIT_handle_anti,   TRUE);

			CheckMenuItem(LEDIT_main_menu, ID_EDIT_PLACELIGHT, MF_CHECKED);

			break;

		case LEDIT_MODE_EDIT_LIGHT:
			
			if (LEDIT_edit_light == NULL)
			{
				//
				// No light is being edited at the moment.
				//

				SetWindowText(LEDIT_handle_light, "Select a light to edit...");

				EnableWindow(LEDIT_handle_red,    FALSE);
				EnableWindow(LEDIT_handle_green,  FALSE);
				EnableWindow(LEDIT_handle_blue,   FALSE);
				EnableWindow(LEDIT_handle_bright, FALSE);
				EnableWindow(LEDIT_handle_range,  FALSE);
				EnableWindow(LEDIT_handle_anti,   FALSE);

				EnableWindow(LEDIT_handle_bwhite,   FALSE);
				EnableWindow(LEDIT_handle_blgrey,   FALSE);
				EnableWindow(LEDIT_handle_bdgrey,   FALSE);
				EnableWindow(LEDIT_handle_bpyellow, FALSE);
				EnableWindow(LEDIT_handle_bpblue,   FALSE);
				EnableWindow(LEDIT_handle_bpred,    FALSE);
			}
			else
			{
				SetWindowText(LEDIT_handle_light, "Editing a light");

				EnableWindow(LEDIT_handle_red,    TRUE);
				EnableWindow(LEDIT_handle_green,  TRUE);
				EnableWindow(LEDIT_handle_blue,   TRUE);
				EnableWindow(LEDIT_handle_bright, TRUE);
				EnableWindow(LEDIT_handle_range,  TRUE);
				EnableWindow(LEDIT_handle_anti,   TRUE);

				EnableMenuItem(LEDIT_main_menu, ID_EDIT_DELETE_ARSE, MF_ENABLED);
			}

			CheckMenuItem(LEDIT_main_menu, ID_EDIT_EDITLIGHTS, MF_CHECKED);

			break;

		case LEDIT_MODE_SET_AMBIENT:
			
			SetWindowText(LEDIT_handle_light, "Setting ambient light");

			EnableWindow(LEDIT_handle_red,    TRUE);
			EnableWindow(LEDIT_handle_green,  TRUE);
			EnableWindow(LEDIT_handle_blue,   TRUE);
			EnableWindow(LEDIT_handle_bright, TRUE);
			EnableWindow(LEDIT_handle_range,  FALSE);
			EnableWindow(LEDIT_handle_anti,   FALSE);

			SendMessage(LEDIT_handle_anti, BM_SETCHECK, BST_UNCHECKED, 0);

			CheckMenuItem(LEDIT_main_menu, ID_EDIT_SETAMBIENT, MF_CHECKED);

			break;

		case LEDIT_MODE_SET_LAMPOST:
			
			SetWindowText(LEDIT_handle_light, "Changing the lights under lamposts");

			EnableWindow(LEDIT_handle_red,    TRUE);
			EnableWindow(LEDIT_handle_green,  TRUE);
			EnableWindow(LEDIT_handle_blue,   TRUE);
			EnableWindow(LEDIT_handle_bright, TRUE);
			EnableWindow(LEDIT_handle_range,  TRUE);
			EnableWindow(LEDIT_handle_anti,   TRUE);

			CheckMenuItem(LEDIT_main_menu, ID_EDIT_SETLAMPOSTCOLOUR, MF_CHECKED);

			break;

		case LEDIT_MODE_SET_SKY:
			
			SetWindowText(LEDIT_handle_light, "Set the sky colour");

			EnableWindow(LEDIT_handle_red,    TRUE);
			EnableWindow(LEDIT_handle_green,  TRUE);
			EnableWindow(LEDIT_handle_blue,   TRUE);
			EnableWindow(LEDIT_handle_bright, TRUE);
			EnableWindow(LEDIT_handle_range,  FALSE);
			EnableWindow(LEDIT_handle_anti,   FALSE);

			SendMessage(LEDIT_handle_anti, BM_SETCHECK, BST_UNCHECKED, 0);

			CheckMenuItem(LEDIT_main_menu, ID_EDIT_SETSKYCOLOUR, MF_CHECKED);

			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Check other menu items.
	//

	if (ED_lampost_on_get())        {CheckMenuItem(LEDIT_main_menu, ID_MAP_LIGHTSUNDERLAMPOSTS,      MF_CHECKED);}
	if (ED_darken_bottoms_on_get()) {CheckMenuItem(LEDIT_main_menu, ID_MAP_DARKENBOTTOMSOFBUILDINGS, MF_CHECKED);}

	if (ED_night_get())
	{
		CheckMenuItem(LEDIT_main_menu, ID_MAP_NIGHTSKY, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(LEDIT_main_menu, ID_MAP_DAYTIME,  MF_CHECKED);
	}

	//
	// Update the menu.
	//

	DrawMenuBar((struct HWND__*)LEDIT_main_menu);
}


//
// Saves the lighting.
//

void LEDIT_lighting_save(void)
{
	LEDIT_ofn_light.lpstrTitle = "Save a lighting file.";

	if (!GetSaveFileName(&LEDIT_ofn_light))
	{
		return;
	}

	SetCursor(LEDIT_busy_bee);
	ED_save(LEDIT_ofn_file_light);
	LEDIT_set_state_look();
	SetCursor(LEDIT_arrow);
}

void LEDIT_lighting_load(void)
{
	LEDIT_ofn_light.lpstrTitle = "Load a lighting file.";

	if (ED_undo_undo_valid())
	{
		switch(MessageBox(
					LEDIT_handle_frame,
					"Save changes to current map?",
					"Load lighting",
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL))
		{
			case IDYES:
				LEDIT_lighting_save();
				break;

			case IDNO:
				break;

			case IDCANCEL:
				return;

			default:
				ASSERT(0);
				break;
		}
	}

 	if (!GetOpenFileName(&LEDIT_ofn_light))
	{
		return;
	}

	SetCursor(LEDIT_busy_bee);
	ED_load(LEDIT_ofn_file_light);
	SetCursor(LEDIT_arrow);

	LEDIT_mode = LEDIT_MODE_PLACE_LIGHT;

	LEDIT_set_state_look();
}




//
// Synchronises the colour stuff to the currently edited thing.
//

void LEDIT_sync_colours()
{
	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG signed_red;
	SLONG signed_green;
	SLONG signed_blue;
	SLONG range;

	switch(LEDIT_mode)
	{
		case LEDIT_MODE_NOTHING:
			break;

		case LEDIT_MODE_PLACE_LIGHT:
			break;

		case LEDIT_MODE_EDIT_LIGHT:

			if (LEDIT_edit_light)
			{
				ASSERT(WITHIN(LEDIT_edit_light, 1, ED_MAX_LIGHTS - 1));

				ED_Light *el = &ED_light[LEDIT_edit_light];

				red   = abs(el->red)   << 1;
				green = abs(el->green) << 1;
				blue  = abs(el->blue)  << 1;

				if (el->red < 0 || el->green < 0 || el->blue < 0)
				{
					//
					// This is an anti-light.
					//

					SendMessage(LEDIT_handle_anti, BM_SETCHECK, BST_CHECKED, 0);
				}
				else
				{
					SendMessage(LEDIT_handle_anti, BM_SETCHECK, BST_UNCHECKED, 0);
				}

				LEDIT_change_colour(RGB(red, green, blue));

				SendMessage(LEDIT_handle_range, TBM_SETPOS, TRUE, 255 - el->range);
			}

			break;

		case LEDIT_MODE_SET_AMBIENT:

			ED_amb_get(
				&red,
				&green,
				&blue);

			LEDIT_change_colour(RGB(red, green, blue));

			break;

		case LEDIT_MODE_SET_LAMPOST:

			ED_lampost_get(
			    &range,
				&signed_red,
				&signed_green,
				&signed_blue);

			if (signed_red   < 0 ||
				signed_green < 0 ||
				signed_blue  < 0)
			{
				//
				// This is an antilight.
				// 

				SendMessage(LEDIT_handle_anti, BM_SETCHECK, BST_CHECKED, 0);
			}
			else
			{
				SendMessage(LEDIT_handle_anti, BM_SETCHECK, BST_UNCHECKED, 0);
			}

			signed_red   = abs(signed_red)   << 1;
			signed_green = abs(signed_green) << 1;
			signed_blue  = abs(signed_blue)  << 1;

			LEDIT_change_colour(RGB(signed_red, signed_green, signed_blue));

			SendMessage(LEDIT_handle_range, TBM_SETPOS, TRUE, 255 - range);

			break;

		case LEDIT_MODE_SET_SKY:

			ED_sky_get(
				&red,
				&green,
				&blue);

			LEDIT_change_colour(RGB(red, green, blue));

			break;

		default:
			break;
	}
}


//
// Captures the lighting state.
//

void LEDIT_make_undoable(void)
{
	ED_undo_store();

	EnableMenuItem(LEDIT_main_menu, ID_EDIT_UNDO_ARSE, (ED_undo_undo_valid()) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(LEDIT_main_menu, ID_EDIT_REDO_ARSE, (ED_undo_redo_valid()) ? MF_ENABLED : MF_GRAYED);

	DrawMenuBar((struct HWND__*)LEDIT_main_menu);
}

void LEDIT_undo()
{
	ED_undo_undo();

	//
	// Make sure that our last_placed light is always valid.
	//

	if (LEDIT_last_placed != NULL)
	{
		ASSERT(WITHIN(LEDIT_last_placed, 1, ED_MAX_LIGHTS - 1));

		if (!ED_light[LEDIT_last_placed].used)
		{
			LEDIT_last_placed = NULL;
		}
	}

	LEDIT_sync_colours();
	LEDIT_set_state_look();
}

void LEDIT_redo()
{
	ED_undo_redo();

	LEDIT_sync_colours();
	LEDIT_set_state_look();
}






//
// Collides the given rectangle with all the windows
// except the given one. Makes sure it doesn't overlap them.
//

void LEDIT_collide_window_rect(
		RECT *rect,
		HWND  ignore_window)
{
	SLONG i;

	HWND hwnd;

	SLONG dist;

	SLONG best_dx;
	SLONG best_dy;
	SLONG best_dist;

	#define MAX_COLRECTS 8

	RECT  colrect[MAX_COLRECTS];
	SLONG colrect_upto = 0;

	for (i = 0; i < 2; i++)
	{
		switch(i)
		{
			case 0: hwnd = LEDIT_handle_engine; break;
			case 1: hwnd = LEDIT_handle_light;  break;
		}

		if (hwnd != ignore_window)
		{
			ASSERT(WITHIN(colrect_upto, 0, MAX_COLRECTS - 1));

			//
			// Find this windows bounding rectangle.
			//

			GetWindowRect(hwnd, &colrect[colrect_upto]);

			colrect_upto += 1;
		}
	}

	//
	// Collide our rectangle against all the colrect structures.
	//

	for (i = 0; i < colrect_upto; i++)
	{
		//
		// Do they overlap?
		//

		if (rect->top    >= colrect[i].bottom ||
			rect->bottom <= colrect[i].top    ||
			rect->left   >= colrect[i].right  ||
			rect->right  <= colrect[i].left)
		{
			//
			// No overlap.
			//
		}
		else
		{
			//
			// The best way to stop them overlapping.
			//

			best_dx   = 0;
			best_dy   = 0;
			best_dist = INFINITY;

			//
			// Bottom edge.
			// 

			dist = colrect[i].bottom - rect->top;
		
			if (dist > 0)
			{
				if (dist < best_dist)
				{
					best_dx   = 0;
					best_dy   = dist;
					best_dist = dist;
				}
			}	

			//
			// Top edge.
			// 

			dist = rect->bottom - colrect[i].top;
		
			if (dist > 0)
			{
				if (dist < best_dist)
				{
					best_dx   =  0;
					best_dy   = -dist;
					best_dist =  dist;
				}
			}

			//
			// Right edge.
			//

			dist = colrect[i].right - rect->left;
			
			if (dist > 0)
			{
				if (dist < best_dist)
				{
					best_dx   = dist;
					best_dy   = 0;
					best_dist = dist;
				}
			}

			//
			// Left edge.
			//

			dist = rect->right - colrect[i].left;
			
			if (dist > 0)
			{
				if (dist < best_dist)
				{
					best_dx   = -dist;
					best_dy   =  0;
					best_dist =  dist;
				}
			}

			//
			// Move the collided rectangle.
			//

			rect->top    += best_dy;
			rect->bottom += best_dy;

			rect->left  += best_dx;
			rect->right += best_dx;
		}
	}
}

//
// Returns the current light.
//

void LEDIT_get_light_signed(
		SLONG *red,
		SLONG *green,
		SLONG *blue,
		SLONG *range)
{
	SLONG pos_red;
	SLONG pos_green;
	SLONG pos_blue;
	SLONG pos_range;

	SLONG signed_red;
	SLONG signed_green;
	SLONG signed_blue;
	SLONG signed_range;

	pos_red   = SendMessage(LEDIT_handle_red,   TBM_GETPOS, 0, 0);
	pos_green = SendMessage(LEDIT_handle_green, TBM_GETPOS, 0, 0);
	pos_blue  = SendMessage(LEDIT_handle_blue,  TBM_GETPOS, 0, 0);
	pos_range = SendMessage(LEDIT_handle_range, TBM_GETPOS, 0, 0);

	signed_range = 255 - pos_range;

	signed_red   = pos_red   >> 1;
	signed_green = pos_green >> 1;
	signed_blue  = pos_blue  >> 1;

	if (SendMessage(LEDIT_handle_anti, BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		//
		// This is an anti-light.
		//

		signed_red   = -signed_red;
		signed_green = -signed_green;
		signed_blue  = -signed_blue;
	}

   *red   = signed_red;
   *green = signed_green;
   *blue  = signed_blue;
   *range = signed_range;
}

void LEDIT_get_light_unsigned(
		SLONG *red,
		SLONG *green,
		SLONG *blue,
		SLONG *range)
{
	SLONG pos_red;
	SLONG pos_green;
	SLONG pos_blue;
	SLONG pos_range;

	SLONG unsigned_red;
	SLONG unsigned_green;
	SLONG unsigned_blue;
	SLONG unsigned_range;

	pos_red   = SendMessage(LEDIT_handle_red,   TBM_GETPOS, 0, 0);
	pos_green = SendMessage(LEDIT_handle_green, TBM_GETPOS, 0, 0);
	pos_blue  = SendMessage(LEDIT_handle_blue,  TBM_GETPOS, 0, 0);
	pos_range = SendMessage(LEDIT_handle_range, TBM_GETPOS, 0, 0);

	unsigned_range = 255 - pos_range;

	unsigned_red   = pos_red;
	unsigned_green = pos_green;
	unsigned_blue  = pos_blue;

   *red   = unsigned_red;
   *green = unsigned_green;
   *blue  = unsigned_blue;
   *range = unsigned_range;
}




//
// If the light window changes, call this funciton to put the
// new data into the right thing.
//

void LEDIT_edited_light(SLONG make_undoable)
{
	SLONG signed_red;
	SLONG signed_green;
	SLONG signed_blue;
	SLONG signed_range;

	SLONG unsigned_red;
	SLONG unsigned_green;
	SLONG unsigned_blue;
	SLONG unsigned_range;

	LEDIT_get_light_signed(
		&signed_red,
		&signed_green,
		&signed_blue,
		&signed_range);

	LEDIT_get_light_unsigned(
		&unsigned_red,
		&unsigned_green,
		&unsigned_blue,
		&unsigned_range);

	switch(LEDIT_mode)
	{
		case LEDIT_MODE_NOTHING:
			break;

		case LEDIT_MODE_PLACE_LIGHT:
			break;

		case LEDIT_MODE_EDIT_LIGHT:

			if (LEDIT_edit_light)
			{
				//
				// Change the attribute of our light.
				//

				ED_light_change(
					LEDIT_edit_light,
					signed_range,
					signed_red,
					signed_green,
					signed_blue);

				if (make_undoable)
				{
					LEDIT_make_undoable();
				}
			}

			break;

		case LEDIT_MODE_SET_AMBIENT:

			ED_amb_set(
				unsigned_red,
				unsigned_green,
				unsigned_blue);

			if (make_undoable)
			{
				LEDIT_make_undoable();
			}

			break;

		case LEDIT_MODE_SET_LAMPOST:

			ED_lampost_set(
				signed_range,
				signed_red,
				signed_green,
				signed_blue);

			if (make_undoable)
			{
				LEDIT_make_undoable();
			}

			break;

		case LEDIT_MODE_SET_SKY:

			ED_sky_set(
				unsigned_red,
				unsigned_green,
				unsigned_blue);

			if (make_undoable)
			{
				LEDIT_make_undoable();
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Changes the colour of the colour box in the light window.
//

SLONG LEDIT_dont_normalise_brightness_bar;	// Set to TRUE while dragging the brightness trackbar

void LEDIT_change_colour(COLORREF cr)
{
	//
	// Get our last brush.
	//

	HBRUSH brush = (HBRUSH) GetClassLong(LEDIT_handle_colour, GCL_HBRBACKGROUND);

	if (brush)
	{
		DeleteObject(brush);
	}

	//
	// Make the chosen colour our background colour.
	//

	brush = CreateSolidBrush(cr);

	SetClassLong(LEDIT_handle_colour, GCL_HBRBACKGROUND, (LONG) brush);

	//
	// Make sure the window is redrawn.
	//
	
	InvalidateRect(LEDIT_handle_colour, NULL, TRUE);

	//
	// Set the correct positions of the colour bars.
	//

	SendMessage(LEDIT_handle_red,   TBM_SETPOS, TRUE, (cr >>  0) & 0xff);
	SendMessage(LEDIT_handle_green, TBM_SETPOS, TRUE, (cr >>  8) & 0xff);
	SendMessage(LEDIT_handle_blue,  TBM_SETPOS, TRUE, (cr >> 16) & 0xff);

	if (!LEDIT_dont_normalise_brightness_bar)
	{
		//
		// Normalise the brightness bar.
		//

		LEDIT_bright_base_red   = (cr >>  0) & 0xff;
		LEDIT_bright_base_green = (cr >>  8) & 0xff;
		LEDIT_bright_base_blue  = (cr >> 16) & 0xff;

		SendMessage(LEDIT_handle_bright, TBM_SETPOS, TRUE, 256);
	}
}


//
// Calculates the camera position and matrix from the cam_focus
// and yaw,pitch variables.
//

void LEDIT_calc_camera_pos()
{
	FMATRIX_calc(
		LEDIT_cam_matrix,
		LEDIT_cam_yaw,
		LEDIT_cam_pitch,
		0);

	LEDIT_cam_x = LEDIT_cam_focus_x;
	LEDIT_cam_y = 0x100; // PAP_calc_height_at(LEDIT_cam_focus_x, LEDIT_cam_focus_z) + 0x100;
	LEDIT_cam_z = LEDIT_cam_focus_z;

	LEDIT_cam_x -= MUL64(LEDIT_cam_matrix[6], LEDIT_cam_focus_dist);
	LEDIT_cam_y -= MUL64(LEDIT_cam_matrix[7], LEDIT_cam_focus_dist);
	LEDIT_cam_z -= MUL64(LEDIT_cam_matrix[8], LEDIT_cam_focus_dist);

	FMATRIX_vector(
		LEDIT_cam_forward,
		LEDIT_cam_yaw,
		0);

	FMATRIX_vector(
		LEDIT_cam_left,
		(LEDIT_cam_yaw + 512) & 2047,
		0);
}


//
// Exits the lighting editor.
// 

void LEDIT_request_exit()
{
	if (ED_undo_undo_valid())
	{
		switch (MessageBox(
					LEDIT_handle_frame,
					"Save changes to current map?",
					"Exit lighting editor",
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL))
		{
			case IDYES:
				LEDIT_lighting_save();
				break;

			case IDNO:
				break;

			case IDCANCEL:
				return;

			default:
				ASSERT(0);
				break;
		}
	}

	PostQuitMessage(0);
}

//
// Loads a new map.
//

void LEDIT_load_map(CBYTE *name)
{
	if (ED_undo_undo_valid())
	{
		switch(MessageBox(
					LEDIT_handle_frame,
					"Save changes to current map?",
					"Load a new map",
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL))
		{
			case IDYES:
				
				//
				// Save changes.
				//

				break;

			case IDNO:
				break;

			case IDCANCEL:
				return;

			default:
				ASSERT(0);
				break;
		}
	}

	//
	// Change the cursor to a busy bee.
	//

	SetCursor(LEDIT_busy_bee);

	//
	// Change to the default directory.
	//

	SetCurrentDirectory(LEDIT_default_dir);

	//
	// Load in the new map.
	//

	LEDIT_map_valid   = GI_load_map(name);
	LEDIT_mode        = LEDIT_MODE_PLACE_LIGHT;
	LEDIT_last_placed = NULL;

	if (LEDIT_map_valid)
	{
		strcpy(LEDIT_map_name, name);
	}
	else
	{
		strcpy(LEDIT_map_name, "No map loaded");
	}
	
	//
	// Initialise the editor.
	//

	ED_init();

	//
	// Setup everything to look correct.
	//

	LEDIT_set_state_look();

	//
	// Start looking at the middle of it.
	//

	LEDIT_cam_focus_x    = 64 << 8;
	LEDIT_cam_focus_z    = 64 << 8;
	LEDIT_cam_focus_dist = 10 << 8;
	LEDIT_cam_pitch      = 1600;
	LEDIT_cam_yaw        = 0;

	LEDIT_calc_camera_pos();

	//
	// Draw the engine.
	//

	GI_render_view_into_backbuffer(
		LEDIT_cam_x,
		LEDIT_cam_y,
		LEDIT_cam_z,
		LEDIT_cam_yaw,
		LEDIT_cam_pitch,
		0);
	
	//
	// Make sure the engine draws something.
	//

	InvalidateRect(LEDIT_handle_engine, NULL, FALSE);

	//
	// Change the cursor back to normal.
	//

	SetCursor(LEDIT_arrow);
}


//
// Editor processing when there are no pending messages.
//

void LEDIT_process()
{
	SLONG i;
	
	SLONG df;
	SLONG dl;
	SLONG dy;
	SLONG dp;
	SLONG dd;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	static turn = 0;

	turn += 1;

	UBYTE highlight;

	ED_Light *el;

	if (!LEDIT_map_valid)
	{
		//
		// No map loaded.
		//

		return;
	}

	//
	// Do engine movement.
	//

	LEDIT_calc_camera_pos();

	df = 0;
	dl = 0;
	dd = 0;
	dy = 0;
	dp = 0;

	if (Keys[KB_LEFT ]) {dl = +1;}
	if (Keys[KB_RIGHT]) {dl = -1;}
	if (Keys[KB_UP   ]) {df = +1;}
	if (Keys[KB_DOWN ]) {df = -1;}

	if (Keys[KB_HOME ]) {dd = -1;}
	if (Keys[KB_END  ]) {dd = +1;}

	if (Keys[KB_DEL  ]) {dy = -1;}
	if (Keys[KB_PGDN ]) {dy = +1;}
	if (Keys[KB_INS  ]) {dp = -1;}
	if (Keys[KB_PGUP ]) {dp = +1;}

	//
	// Are we moving faster?
	//

	if (ShiftFlag)
	{
		dl <<= 2;
		df <<= 2;
		dd <<= 2;
		dy <<= 2;
		dp <<= 2;
	}

	if (dl || df || dd || dy || dp)
	{
		//
		// If the engine has moved, pretend to the engine
		// that the mouse has moved.  It has in a way...
		//

		SendMessage(LEDIT_handle_engine, WM_MOUSEMOVE, 0, 0); // Doens't care where the mouse is...
	}

	//
	// Update position.
	//

	LEDIT_cam_focus_x += df * LEDIT_cam_forward[0] >> 11;
	LEDIT_cam_focus_z += df * LEDIT_cam_forward[2] >> 11;

	LEDIT_cam_focus_x += dl * LEDIT_cam_left[0] >> 11;
	LEDIT_cam_focus_z += dl * LEDIT_cam_left[2] >> 11;

	LEDIT_cam_focus_dist += dd * 16;
	LEDIT_cam_yaw        += dy * 16;
	LEDIT_cam_pitch      += dp * 16;

	LEDIT_cam_yaw   &= 2047;
	LEDIT_cam_pitch &= 2047;

	LEDIT_calc_camera_pos();

	//
	// Draw the engine.
	//

	GI_render_view_into_backbuffer(
		LEDIT_cam_x,
		LEDIT_cam_y,
		LEDIT_cam_z,
		LEDIT_cam_yaw,
		LEDIT_cam_pitch,
		0);

	//
	// Is the mouse over the engine window?
	//
	
	POINT mouse;
	RECT  clientrect;

	GetCursorPos(&mouse);
	ScreenToClient(LEDIT_handle_engine, &mouse);
	GetClientRect(LEDIT_handle_engine, &clientrect);

	if (WITHIN(mouse.x, clientrect.left, clientrect.right) &&
		WITHIN(mouse.y, clientrect.top,  clientrect.bottom))
	{
/*		if (!LEDIT_insides)*/ INDOORS_INDEX=0;
		LEDIT_mouse_over  = 0;
		LEDIT_mouse_valid = GI_get_pixel_world_pos(
								mouse.x,
								mouse.y,
							   &LEDIT_mouse_world_x,
							   &LEDIT_mouse_world_y,
							   &LEDIT_mouse_world_z,INDOORS_INDEX);

/*		INDOORS_INDEX = calc_inside_for_xyz(LEDIT_mouse_world_x,LEDIT_mouse_world_y,LEDIT_mouse_world_z,&INDOORS_ROOM);
		if (LEDIT_insides&&!INDOORS_INDEX) {
			INDOORS_INDEX = calc_inside_for_xyz(LEDIT_mouse_world_x,LEDIT_mouse_world_y-128,LEDIT_mouse_world_z,&INDOORS_ROOM);
		}*/
		if (LEDIT_insides)
		{
			ED_Light *el=0;
			SLONG ei;
			ei= (LEDIT_mode == LEDIT_MODE_PLACE_LIGHT) ? LEDIT_last_placed : LEDIT_edit_light;
			if (ei) el = &ED_light[ei]; //else TRACE("no ei\n");
			if (el) INDOORS_INDEX = calc_inside_for_xyz(el->x,el->y,el->z,&INDOORS_ROOM); //else TRACE("no el\n");
		}
		if (INDOORS_INDEX) {
			INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
//			TRACE("thinks it's inside...\n");
		}
		else {
//			TRACE("thinks it's outside...\n");
			INDOORS_DBUILDING=0;
		}

		//
		// Draw all the lights within a certain distance of the camera.
		//

		for (i = 1; i < ED_MAX_LIGHTS; i++)
		{
			el = &ED_light[i];

			if (el->used)
			{
				if (LEDIT_mode       == LEDIT_MODE_EDIT_LIGHT &&
					LEDIT_edit_light == i)
				{
					if (el->red   < 0 ||
						el->green < 0 ||
						el->blue  < 0)
					{
						highlight = 256 - (turn << 5);
					}
					else
					{
						highlight = turn << 5;
					}
				}
				else
				{
					highlight = 0;
				}

				dx = el->x - LEDIT_cam_x;
				dz = el->z - LEDIT_cam_z;

				dist = QDIST2(abs(dx),abs(dz));

				if (dist < (20 << 8))
				{
					//
					// Draw the bugger!
					//

					ULONG over = GI_light_draw(
									mouse.x,
									mouse.y,
									el->x,
									el->y,
									el->z,
									(el->red << 16) | (el->green << 8) | (el->blue << 0),
									highlight);

					if (over)
					{
						LEDIT_mouse_over  = over;
						LEDIT_mouse_light = i;
					}
				}
			}
		}
	}
	else
	{
		LEDIT_mouse_valid = FALSE;
	}

	if (LEDIT_mouse_valid && LEDIT_mode == LEDIT_MODE_PLACE_LIGHT)
	{
		SLONG red;
		SLONG green;
		SLONG blue;
		SLONG range;
		ULONG colour;

		LEDIT_get_light_signed(
			&red,
			&green,
			&blue,
			&range);

		colour = (red << 16) | (green << 8) | (blue << 0);

		GI_light_draw(
			mouse.x,
			mouse.y,
			LEDIT_mouse_world_x,
			LEDIT_mouse_world_y + 0x100,
			LEDIT_mouse_world_z,
			colour,
			0);
	}

	//
	// Make sure the engine draws something.
	//

	InvalidateRect(LEDIT_handle_engine, NULL, FALSE);

	//
	// Make sure the cursor looks correct.
	//

	if (LEDIT_mouse_valid                         &&
		LEDIT_mode       == LEDIT_MODE_EDIT_LIGHT &&
		LEDIT_mouse_over == GI_MOUSE_OVER_LIGHT_BOT)
	{
		SetCursor(LEDIT_all_dirs);

		//
		// Make it the default for the engine window.
		//

		SetClassLong(LEDIT_handle_engine, GCL_HCURSOR, (long) LEDIT_all_dirs);
	}
	else
	{
		SetCursor(LEDIT_arrow);

		//
		// Make it the default for the engine window.
		//

		SetClassLong(LEDIT_handle_engine, GCL_HCURSOR, (long) LEDIT_arrow);
	}
}


//
// Callback function for the frame window
//

LRESULT CALLBACK LEDIT_callback_frame(
					HWND   window_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG range;
	SLONG scancode;

	if (message_type == LEDIT_wm_mousewheel || message_type == WM_MOUSEWHEEL)
	{
		//
		// Turning the mousewheel raises and lowers the current
		// light we are editing.
		//

		if (LEDIT_mode       == LEDIT_MODE_EDIT_LIGHT &&
			LEDIT_edit_light != NULL)
		{	
			ASSERT(WITHIN(LEDIT_edit_light, 1, ED_MAX_LIGHTS - 1));

			//
			// Move the edit light to the new place.
			//

			ED_Light *el = &ED_light[LEDIT_edit_light];

			ED_light_move(
				LEDIT_edit_light,
				el->x,
				el->y + (((short) LOWORD(param_w)) * 64 / 120) + (((short) HIWORD(param_w)) * 64 / 120),
				el->z);

			LEDIT_make_undoable();
		}
		else
		if (LEDIT_mode        == LEDIT_MODE_PLACE_LIGHT &&
			LEDIT_last_placed != NULL)
		{
			//
			// Move the light we last placed to the new position.
			//

			ED_Light *el = &ED_light[LEDIT_last_placed];

			ED_light_move(
				LEDIT_last_placed,
				el->x,
				el->y + (((short) LOWORD(param_w)) * 64 / 120),
				el->z);

			LEDIT_make_undoable();
		}

		return 0;
	}

	switch(message_type)
	{
        case WM_CLOSE:
			LEDIT_request_exit();
            return 0;

        case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(param_w))
			{
				case ID_FILE_OPEN_ARSE:

					if (GetOpenFileName(&LEDIT_ofn_map))
					{
						LEDIT_load_map(LEDIT_ofn_file_map);
					}

					break;

				case ID_FILE_LOAD_ARSE:
					LEDIT_lighting_load();
					break;

				case ID_FILE_SAVE_ARSE:
					LEDIT_lighting_save();
					break;

				case ID_FILE_EXIT:
					LEDIT_request_exit();
					break;

				case ID_EDIT_UNDO_ARSE:
					LEDIT_undo();
					break;

				case ID_EDIT_REDO_ARSE:
					LEDIT_redo();
					break;

				case ID_EDIT_CLEARALL:

					//
					// Delete all the lights.
					//

					ED_delete_all();
					LEDIT_make_undoable();

					break;

				case ID_EDIT_DELETE_ARSE:

					if (LEDIT_mode == LEDIT_MODE_EDIT_LIGHT &&
						LEDIT_edit_light != NULL)
					{
						//
						// Delete the light we are editing.
						// 

						ED_delete(LEDIT_edit_light);
						LEDIT_edit_light = NULL;
						LEDIT_make_undoable();
						LEDIT_set_state_look();
					}

					break;

				case ID_EDIT_INSIDES:
					LEDIT_insides^=1;
					CheckMenuItem(LEDIT_main_menu, ID_EDIT_INSIDES, MF_BYCOMMAND | (LEDIT_insides ? MF_CHECKED : MF_UNCHECKED) );
					DrawMenuBar((struct HWND__*)LEDIT_main_menu);
					break;

				case ID_EDIT_PLACELIGHT:
					LEDIT_mode        = LEDIT_MODE_PLACE_LIGHT;
					LEDIT_last_placed = NULL;
					LEDIT_set_state_look();
					break;

				case ID_EDIT_EDITLIGHTS:
					LEDIT_mode          = LEDIT_MODE_EDIT_LIGHT;
					LEDIT_edit_light    = NULL;
					LEDIT_edit_dragging = FALSE;
					LEDIT_edit_dragged  = FALSE;
					LEDIT_set_state_look();
					break;

				case ID_EDIT_SETAMBIENT:

					LEDIT_mode = LEDIT_MODE_SET_AMBIENT;
					LEDIT_sync_colours();
					LEDIT_set_state_look();

					break;

				case ID_EDIT_SETLAMPOSTCOLOUR:

					if (!ED_lampost_on_get())
					{
						//
						// Editing lampost light but lamposts aren't
						// enabled.
						//

						switch(MessageBox(
								LEDIT_handle_frame,
								"Lights under lamposts is not enabled. Do you want to turn it on?",
								"Edit lights under lamposts",
								MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL))
						{
							case IDYES:
								LEDIT_mode = LEDIT_MODE_SET_LAMPOST;
								ED_lampost_on_set(TRUE);
								LEDIT_set_state_look();
								break;

							case IDNO:
								LEDIT_mode = LEDIT_MODE_SET_LAMPOST;
								LEDIT_set_state_look();
								break;

							case IDCANCEL:
								break;
						}
					}
					else
					{
						LEDIT_mode = LEDIT_MODE_SET_LAMPOST;
						LEDIT_set_state_look();
					}

					if (LEDIT_mode == LEDIT_MODE_SET_LAMPOST)
					{
						LEDIT_sync_colours();
					}

					break;

				case ID_EDIT_SETSKYCOLOUR:
					LEDIT_mode = LEDIT_MODE_SET_SKY;
					LEDIT_sync_colours();
					LEDIT_set_state_look();
					break;

				case ID_MAP_NIGHTSKY:
					ED_night_set(TRUE);
					LEDIT_sync_colours();
					LEDIT_set_state_look();
					break;

				case ID_MAP_DAYTIME:
					ED_night_set(FALSE);
					LEDIT_set_state_look();
					break;

				case ID_MAP_LIGHTSUNDERLAMPOSTS:
					ED_lampost_on_set(!ED_lampost_on_get());
					LEDIT_set_state_look();
					break;

				case ID_MAP_DARKENBOTTOMSOFBUILDINGS:
					ED_darken_bottoms_on_set(!ED_darken_bottoms_on_get());
					LEDIT_set_state_look();
					break;

				case ID_HELP_ABOUT:
					
					MessageBox(
						LEDIT_handle_frame,
						"Mark is the main guilty party here",
						"Urban Chaos lighting editor",
						MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);

					break;
			}

			return 0;

		case WM_KEYDOWN:

			scancode = (param_l >> 16) & 0x7f;

			if (param_l & 0x01000000)
			{
				//
				// Exteneded key.
				//

				scancode += 0x80;
			}

			Keys[scancode] = TRUE;

			AltFlag     = (Keys[KB_LALT]     || Keys[KB_RALT]);
			ControlFlag = (Keys[KB_LCONTROL] || Keys[KB_RCONTROL]);
			ShiftFlag   = (Keys[KB_LSHIFT]   || Keys[KB_RSHIFT]);

			return 0;

		case WM_KEYUP:

			scancode = (param_l >> 16) & 0x7f;

			if (param_l & 0x01000000)
			{
				//
				// Exteneded key.
				//

				scancode += 0x80;
			}

			Keys[scancode] = FALSE;

			AltFlag     = (Keys[KB_LALT]     || Keys[KB_RALT]);
			ControlFlag = (Keys[KB_LCONTROL] || Keys[KB_RCONTROL]);
			ShiftFlag   = (Keys[KB_LSHIFT]   || Keys[KB_RSHIFT]);

			return 0;

		case WM_KILLFOCUS:

			//
			// Stop the focus going to the trackbars.
			//

			if ((HWND) param_w == LEDIT_handle_red   ||
				(HWND) param_w == LEDIT_handle_green ||
				(HWND) param_w == LEDIT_handle_blue  ||
				(HWND) param_w == LEDIT_handle_range ||
				(HWND) param_w == LEDIT_handle_bright)
			{
				SetFocus(LEDIT_handle_frame);
			}

			return 0;

		case WM_USER:

			//
			// The message we send to ourselves when nothing else is happening.
			// 

			LEDIT_process();

			return 0;

		default:
			break;
	}

	return DefWindowProc(
				window_handle,
				message_type,
				param_w,
				param_l);
}



//
// Callback function for the engine window
//

LRESULT CALLBACK LEDIT_callback_engine(
					HWND   window_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	HDC         hdc;
	PAINTSTRUCT ps;
	POINT       clientpos;
	RECT        dest;
	RECT		src;

	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG range;

	SLONG world_x;
	SLONG world_y;
	SLONG world_z;

	ED_Light *el;

	switch(message_type)
	{
		case WM_PAINT:

			//
			// Blit across the invalid region from the back-buffer.
			//

			hdc = BeginPaint(window_handle, &ps);

			clientpos.x = 0;
			clientpos.y = 0;

			ClientToScreen(
				 window_handle,
				&clientpos);

			dest.top    = ps.rcPaint.top    + clientpos.y;
			dest.left   = ps.rcPaint.left   + clientpos.x;
			dest.right  = ps.rcPaint.right  + clientpos.x;
			dest.bottom = ps.rcPaint.bottom + clientpos.y;

			src         = ps.rcPaint;

			the_display.lp_DD_FrontSurface->Blt(&dest,the_display.lp_DD_BackSurface,&src,DDBLT_WAIT,0);

			EndPaint(window_handle, &ps);

			return 0;

		case WM_CONTEXTMENU:
			return 0;

		case WM_LBUTTONDOWN:

			if (LEDIT_map_valid && LEDIT_mouse_valid)
			{
				switch(LEDIT_mode)
				{
					case LEDIT_MODE_PLACE_LIGHT:

						//
						// Place a light.
						//

						LEDIT_get_light_signed(
							&red,
							&green,
							&blue,
							&range);

						LEDIT_last_placed = ED_create(
												LEDIT_mouse_world_x,
												LEDIT_mouse_world_y + 0x100,
												LEDIT_mouse_world_z,
												range,
												red,
												green,
												blue);

						LEDIT_make_undoable();

						break;

					case LEDIT_MODE_EDIT_LIGHT:

						if (LEDIT_mouse_over == GI_MOUSE_OVER_LIGHT_BOT)
						{
							ASSERT(WITHIN(LEDIT_mouse_light, 1, ED_MAX_LIGHTS - 1));

							//
							// Select the light.
							//

							LEDIT_edit_light = LEDIT_mouse_light;

							//
							// Start dragging it.
							//

							el = &ED_light[LEDIT_edit_light];

							LEDIT_edit_dragging = TRUE;
							LEDIT_edit_dragged  = FALSE;
							LEDIT_edit_drag_dx  = LEDIT_mouse_world_x - el->x;
							LEDIT_edit_drag_dy  = LEDIT_mouse_world_y - el->y;
							LEDIT_edit_drag_dz  = LEDIT_mouse_world_z - el->z;

							//
							// Grab control of the mouse.
							//

							SetCapture(LEDIT_handle_engine);

							//
							// Update the light window.
							//

							LEDIT_set_state_look();

							//
							// Sync the colours to the new light we are editing.
							//

							LEDIT_sync_colours();
						}
						else
						{
							LEDIT_edit_dragging = FALSE;
						}

						break;

					case LEDIT_MODE_SET_AMBIENT:
						break;

					case LEDIT_MODE_SET_LAMPOST:
						break;
					
					case LEDIT_MODE_SET_SKY:
						break;

					default:
						ASSERT(0);
						break;
				}
			}

			return 0;

		case WM_MOUSEMOVE:

			if (LEDIT_mode       == LEDIT_MODE_EDIT_LIGHT &&
				LEDIT_edit_light != NULL                  &&
				LEDIT_edit_dragging)
			{
				ASSERT(WITHIN(LEDIT_edit_light, 1, ED_MAX_LIGHTS - 1));

				el = &ED_light[LEDIT_edit_light];

				//
				// Move the edit light to the new place.
				//

				ED_light_move(
					LEDIT_edit_light,
					LEDIT_mouse_world_x - LEDIT_edit_drag_dx,
					LEDIT_mouse_world_y - LEDIT_edit_drag_dy,
					LEDIT_mouse_world_z - LEDIT_edit_drag_dz);
			}

			return 0;

		case WM_LBUTTONUP:

			if (LEDIT_mode       == LEDIT_MODE_EDIT_LIGHT &&
				LEDIT_edit_light != NULL                  &&
				LEDIT_edit_dragging)
			{
				//
				// Stop dragging the light.
				//

				LEDIT_edit_dragging = FALSE;

				//
				// Release the capture on the mouse.
				//

				ReleaseCapture();

				if (LEDIT_edit_dragged)
				{
					//
					// We did move the light, so make sure we can undo it.
					//

					LEDIT_make_undoable();
				}
			}			

			return 0;

		case WM_MOVING:

			LEDIT_collide_window_rect(
				(RECT *) param_l,
				window_handle);

			break;

		default:
			break;
	}

	return DefWindowProc(
				window_handle,
				message_type,
				param_w,
				param_l);
}


//
// Callback function for the light window.
// 

LRESULT CALLBACK LEDIT_callback_light(
					HWND   window_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	COLORREF cr;
	HBRUSH   brush;
	SLONG    red;
	SLONG    green;
	SLONG    blue;
	SLONG    delta;

	switch(message_type)
	{
		case WM_MOVING:

			LEDIT_collide_window_rect(
				(RECT *) param_l,
				window_handle);

			break;

		case WM_HSCROLL:

			if (param_l == (int) LEDIT_handle_bright)
			{
				delta = SendMessage(LEDIT_handle_bright, TBM_GETPOS, 0, 0) - 256;

				red   = LEDIT_bright_base_red   + delta;
				green = LEDIT_bright_base_green + delta;
				blue  = LEDIT_bright_base_blue  + delta;

				SATURATE(red,   0, 255);
				SATURATE(green, 0, 255);
				SATURATE(blue,  0, 255);

				LEDIT_dont_normalise_brightness_bar = TRUE;

				//
				// Set the colour in the colour box.
				// 

				cr = RGB(red,green,blue);

				LEDIT_change_colour(cr);

				//
				// Change what we are editing.  Only create an undo
				// packet at the end of the dragging of the trackbar- i.e.
				// a TB_THUMPOSITION message.
				//

				LEDIT_edited_light(LOWORD(param_w) == TB_THUMBPOSITION);

				LEDIT_dont_normalise_brightness_bar = FALSE;
			}
			else
			{
				//
				// Get the current colours.
				//

				red   = SendMessage(LEDIT_handle_red,   TBM_GETPOS, 0, 0);
				green = SendMessage(LEDIT_handle_green, TBM_GETPOS, 0, 0);
				blue  = SendMessage(LEDIT_handle_blue,  TBM_GETPOS, 0, 0);

				//
				// Set the colour in the colour box.
				// 

				cr = RGB(red,green,blue);

				LEDIT_change_colour(cr);

				//
				// Change what we are editing.  Only create an undo
				// packet at the end of the dragging of the trackbar- i.e.
				// a TB_THUMPOSITION message.
				//

				LEDIT_edited_light(LOWORD(param_w) == TB_THUMBPOSITION);
			}

			return 0;

		case WM_VSCROLL:

			if (LEDIT_mode == LEDIT_MODE_EDIT_LIGHT ||
				LEDIT_mode == LEDIT_MODE_SET_LAMPOST)
			{
				//
				// Change what we are editing.  Only create an undo
				// packet at the end of the dragging of the trackbar- i.e.
				// a TB_THUMPOSITION message.
				//

				LEDIT_edited_light(LOWORD(param_w) == TB_THUMBPOSITION);
			}

			return 0;

		case WM_COMMAND:

			//
			// A button has been pressed. Make sure that the frame
			// window has the focus.
			//

			SetFocus(LEDIT_handle_frame);

			if (LOWORD(param_w) == LEDIT_CHILD_ANTI)
			{
				//
				// Clicked on the anti-light button.
				//
			}
			else
			{
				switch(LOWORD(param_w))
				{
					case LEDIT_CHILD_BWHITE:
						red   = 255;
						green = 255;
						blue  = 255;
						break;

					case LEDIT_CHILD_BLGREY:
						red   = 160;
						green = 160;
						blue  = 160;
						break;

					case LEDIT_CHILD_BDGREY:
						red   = 64;
						green = 64;
						blue  = 64;
						break;

					case LEDIT_CHILD_BPYELLOW:
						red   = 255;
						green = 255;
						blue  = 196;
						break;

					case LEDIT_CHILD_BPBLUE:
						red   = 200;
						green = 200;
						blue  = 255;
						break;

					case LEDIT_CHILD_BPRED:
						red   = 255;
						green = 200;
						blue  = 200;
						break;

					default:
						ASSERT(0);
				}

				LEDIT_change_colour(RGB(red,green,blue));
			}

			LEDIT_edited_light(TRUE);

			return 0;

		default:
			break;
	}

	return DefWindowProc(
				window_handle,
				message_type,
				param_w,
				param_l);
}

//
// Callback function for the colour window.
// 

LRESULT CALLBACK LEDIT_callback_colour(
					HWND   window_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG range;

	static CHOOSECOLOR cc;
	static COLORREF    cr[16] =
	{
		0x00ffffff,
		0x00cccccc,
		0x00888888,
		0x00ff0000,
		0x00cc0000,
		0x00880000,
		0x0000ff00,
		0x00008800,
		0x000000ff,
		0x00000088,
		0x00ffff00,
		0x00888800,
		0x00ff00ff,
		0x00880088,
		0x0000ffff,
		0x00008888
	};

	switch(message_type)
	{
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:

			LEDIT_get_light_unsigned(
				&red,
				&green,
				&blue,
				&range);

			cc.lStructSize    = sizeof(cc);
			cc.hwndOwner      = window_handle;
			cc.hInstance      = (struct HWND__*)LEDIT_hinstance;
			cc.lpCustColors   = cr;
			cc.Flags          = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;
			cc.lCustData      = 0;
			cc.lpfnHook       = NULL;
			cc.lpTemplateName = NULL;
			cc.rgbResult      = RGB(red,green,blue);

			if (ChooseColor(&cc))
			{
				//
				// Set the colour in the colour box.
				//

				LEDIT_change_colour(cc.rgbResult);

				//
				// Change what we are editing.
				//

				LEDIT_edited_light(TRUE);
			}
			
			return 0;

		default:
			return DefWindowProc(
						window_handle,
						message_type,
						param_w,
						param_l);
	}
}


void LEDIT_do(void)
{
	RECT rect;

	//
	// Stuff we need to know from the direct draw library.
	//

	extern HINSTANCE hGlobalThisInst;

	LEDIT_hinstance = hGlobalThisInst;

	//
	// Load our cursors.
	//

	LEDIT_arrow    = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	LEDIT_busy_bee = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
	LEDIT_all_dirs = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL));
	LEDIT_upndown  = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));

	int x1 = SM_CXSMICON;
	int y1 = SM_CYSMICON;

	int x2 = SM_CXICON;
	int y2 = SM_CYICON;

	//
	// Our icons.
	//

	LEDIT_icon = LoadIcon(LEDIT_hinstance, MAKEINTRESOURCE(IDI_MFLOGO));

	LEDIT_class_frame.hInstance		= LEDIT_hinstance;
	LEDIT_class_frame.lpszClassName	= LEDIT_name_frame;
	LEDIT_class_frame.lpfnWndProc	= LEDIT_callback_frame;
	LEDIT_class_frame.style			= 0;
	LEDIT_class_frame.cbSize		= sizeof(WNDCLASSEX);
	LEDIT_class_frame.cbClsExtra	= 0;
	LEDIT_class_frame.cbWndExtra	= 0;
	LEDIT_class_frame.lpszMenuName	= NULL;
	LEDIT_class_frame.hIcon			= LEDIT_icon;
	LEDIT_class_frame.hIconSm		= LEDIT_icon;
	LEDIT_class_frame.hCursor		= LoadCursor(NULL, IDC_ARROW);
	LEDIT_class_frame.hbrBackground	= (struct HBRUSH__*)GetStockObject(WHITE_BRUSH);
	
	if (!RegisterClassEx(&LEDIT_class_frame))
	{
		//
		// Could not register the class.
		//
		
		return;
	}

	//
	// Create the frame window.
	//
	
	LEDIT_handle_frame = CreateWindow(
							LEDIT_name_frame,
							LEDIT_name_frame,
							WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							NULL,
							NULL,
							LEDIT_hinstance,
							NULL);

	if (LEDIT_handle_frame == NULL)
	{
		//
		// Could not create our main window. 
		//

		UnregisterClass(LEDIT_name_frame, LEDIT_hinstance);

		return;
	}

	//
	// Our main menu.
	// 

	LEDIT_main_menu = LoadMenu(LEDIT_hinstance, MAKEINTRESOURCE(IDR_LEDIT_MENU));

	SetMenu(LEDIT_handle_frame, LEDIT_main_menu);

	//
	// SetupHost does this- so so must we.
	//

	SetupMemory();

	//
	// Sneakily pretend that this was the window created by SetupHost!
	//

	extern volatile BOOL ShellActive;

	hDDLibWindow = LEDIT_handle_frame;
	ShellActive  = TRUE;

	//
	// Open this display using our engine window.
	//

	if(OpenDisplay(640,480,16,FLAGS_USE_3D|FLAGS_USE_WORKSCREEN) != 0)
	{
		//
		// Could not open display.
		//

		return;
	}

	//
	// Maximize our window.
	//

	ShowWindow(LEDIT_handle_frame, SW_MAXIMIZE);

	//
	// Make the back buffer a nice colour!
	//

	the_display.SetUserColour(0x7c, 0x11, 0x1d);
	the_display.SetUserBackground();
	the_display.ClearViewport();

	//
	// The engine window class.
	//

	LEDIT_class_engine.hInstance		= LEDIT_hinstance;
	LEDIT_class_engine.lpszClassName	= LEDIT_name_engine;
	LEDIT_class_engine.lpfnWndProc		= LEDIT_callback_engine;
	LEDIT_class_engine.style			= 0;
	LEDIT_class_engine.cbSize			= sizeof(WNDCLASSEX);
	LEDIT_class_engine.cbClsExtra		= 0;
	LEDIT_class_engine.cbWndExtra		= 0;
	LEDIT_class_engine.lpszMenuName		= NULL;
	LEDIT_class_engine.hIcon			= NULL;
	LEDIT_class_engine.hIconSm			= NULL;
	LEDIT_class_engine.hCursor			= LoadCursor(NULL, IDC_ARROW);
	LEDIT_class_engine.hbrBackground	= (struct HBRUSH__*)GetStockObject(GRAY_BRUSH);

	if (RegisterClassEx(&LEDIT_class_engine) == 0)
	{
		//
		// Could not register the window class!
		//

		return;
	}
	
	rect.left   = 0;
	rect.right  = 640;
	rect.top    = 0;
	rect.bottom = 480;

	AdjustWindowRectEx(
		&rect,
		 WS_CAPTION | WS_VISIBLE | WS_CHILD,
		 FALSE,
		 0);

	LEDIT_handle_engine = CreateWindow(
							LEDIT_name_engine,
							"No map loaded",
							WS_CAPTION | WS_CHILD,
							350,
							50,
							rect.right  - rect.left,
							rect.bottom - rect.top,
							LEDIT_handle_frame,
							NULL,
							LEDIT_hinstance,
							NULL);

	//
	// The light window class.
	//

	LEDIT_class_light.hInstance		= LEDIT_hinstance;
	LEDIT_class_light.lpszClassName	= LEDIT_name_light;
	LEDIT_class_light.lpfnWndProc	= LEDIT_callback_light;
	LEDIT_class_light.style			= 0;
	LEDIT_class_light.cbSize		= sizeof(WNDCLASSEX);
	LEDIT_class_light.cbClsExtra	= 0;
	LEDIT_class_light.cbWndExtra	= 0;
	LEDIT_class_light.lpszMenuName	= NULL;
	LEDIT_class_light.hIcon			= NULL;
	LEDIT_class_light.hIconSm		= NULL;
	LEDIT_class_light.hCursor		= LoadCursor(NULL, IDC_ARROW);
	LEDIT_class_light.hbrBackground	= (struct HBRUSH__*)GetStockObject(LTGRAY_BRUSH);

	if (RegisterClassEx(&LEDIT_class_light) == 0)
	{
		//
		// Could not register the window class!
		//

		return;
	}
	
	rect.left   = 0;
	rect.right  = 300;
	rect.top    = 0;
	rect.bottom = 200;

	AdjustWindowRectEx(
		&rect,
		 WS_CAPTION | WS_VISIBLE | WS_CHILD,
		 FALSE,
		 0);

	LEDIT_handle_light = CreateWindow(
							LEDIT_name_light,
							LEDIT_name_light,
							WS_CAPTION | WS_CHILD,
							32,
							22,
							rect.right  - rect.left,
							rect.bottom - rect.top,
							LEDIT_handle_frame,
							NULL,
							LEDIT_hinstance,
							NULL);

	//
	// So we can use the common controls like trackbars.
	//

	InitCommonControls();

	//
	// Put controls onto the light window.
	//

	LEDIT_handle_red = CreateWindow(
							TRACKBAR_CLASS,
							"Red",
							WS_CHILD | WS_VISIBLE | TBS_HORZ,
							10, 10,
							165, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_RED,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_green = CreateWindow(
							TRACKBAR_CLASS,
							"Green",
							WS_CHILD | WS_VISIBLE | TBS_HORZ,
							10, 30,
							165, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_GREEN,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_blue = CreateWindow(
							TRACKBAR_CLASS,
							"Blue",
							WS_CHILD | WS_VISIBLE | TBS_HORZ,
							10, 50,
							165, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_BLUE,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_bright = CreateWindow(
							TRACKBAR_CLASS,
							"Brightness",
							WS_CHILD | WS_VISIBLE | TBS_HORZ,
							10, 75,
							165, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_BRIGHT,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_range = CreateWindow(
							TRACKBAR_CLASS,
							"Range",
							WS_CHILD | WS_VISIBLE | TBS_VERT,
							270, 10,
							20, 85,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_RANGE,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_anti = CreateWindow(
							"Button",
							"Antilight",
							BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD,
							180, 75,
							85, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_ANTI,
							LEDIT_hinstance,
							NULL);

	//
	// Predefined colour buttons.
	//
	
	LEDIT_handle_bwhite = CreateWindow(
							"Button",
							"White",
							BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
							10, 110,
							135, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_BWHITE,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_blgrey = CreateWindow(
							"Button",
							"Light grey",
							BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
							10, 135,
							135, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_BLGREY,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_bdgrey = CreateWindow(
							"Button",
							"Dark grey",
							BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
							10, 160,
							135, 20,
							LEDIT_handle_light,
							(HMENU) LEDIT_CHILD_BDGREY,
							LEDIT_hinstance,
							NULL);

	LEDIT_handle_bpyellow = CreateWindow(
								"Button",
								"Pale yellow",
								BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
								155, 110,
								135, 20,
								LEDIT_handle_light,
								(HMENU) LEDIT_CHILD_BPYELLOW,
								LEDIT_hinstance,
								NULL);

	LEDIT_handle_bpred = CreateWindow(
								"Button",
								"Pale red",
								BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
								155, 135,
								135, 20,
								LEDIT_handle_light,
								(HMENU) LEDIT_CHILD_BPRED,
								LEDIT_hinstance,
								NULL);

	LEDIT_handle_bpblue = CreateWindow(
								"Button",
								"Pale blue",
								BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,
								155, 160,
								135, 20,
								LEDIT_handle_light,
								(HMENU) LEDIT_CHILD_BPBLUE,
								LEDIT_hinstance,
								NULL);

	//
	// Set the range of the trackbars.
	// 

	SendMessage(LEDIT_handle_red,    TBM_SETRANGE, TRUE, MAKELONG(0, 255));
	SendMessage(LEDIT_handle_green,  TBM_SETRANGE, TRUE, MAKELONG(0, 255));
	SendMessage(LEDIT_handle_blue,   TBM_SETRANGE, TRUE, MAKELONG(0, 255));
	SendMessage(LEDIT_handle_range,  TBM_SETRANGE, TRUE, MAKELONG(0 ,220));
	SendMessage(LEDIT_handle_bright, TBM_SETRANGE, TRUE, MAKELONG(0 ,512));

	//
	// The colour window is a window we create ourselves.
	//

	LEDIT_class_colour.hInstance		= LEDIT_hinstance;
	LEDIT_class_colour.lpszClassName	= LEDIT_name_colour;
	LEDIT_class_colour.lpfnWndProc		= LEDIT_callback_colour;
	LEDIT_class_colour.style			= 0;
	LEDIT_class_colour.cbSize			= sizeof(WNDCLASSEX);
	LEDIT_class_colour.cbClsExtra		= 0;
	LEDIT_class_colour.cbWndExtra		= 0;
	LEDIT_class_colour.lpszMenuName		= NULL;
	LEDIT_class_colour.hIcon			= NULL;
	LEDIT_class_colour.hIconSm			= NULL;
	LEDIT_class_colour.hCursor			= LoadCursor(NULL, IDC_ARROW);
	LEDIT_class_colour.hbrBackground	= (struct HBRUSH__*)GetStockObject(BLACK_BRUSH);

	if (RegisterClassEx(&LEDIT_class_colour) == 0)
	{
		//
		// Could not register the window class!
		//

		return;
	}

	LEDIT_handle_colour = CreateWindow(
							LEDIT_name_colour,
							LEDIT_name_colour,
							WS_DLGFRAME | WS_VISIBLE | WS_CHILD,
							180, 10,
							85,  60,
							LEDIT_handle_light,
							NULL,
							LEDIT_hinstance,
							NULL);

	//
	// Start off with white not black.
	// 

	LEDIT_change_colour(RGB(255,255,255));

	//
	// Unaltered brightness.
	//

	LEDIT_bright_base_red   = 255;
	LEDIT_bright_base_green = 255;
	LEDIT_bright_base_blue  = 255;

	SendMessage(LEDIT_handle_bright, TBM_SETPOS, TRUE, 256);

	//
	// Put a tick in the middle of the brightness trackbar.
	//

	SendMessage(LEDIT_handle_bright, TBM_SETTIC, 0, 256);

	//
	// Set-up everything to look correct.
	//

	LEDIT_set_state_look();

	//
	// Load the accelerator table.
	//

	LEDIT_accel = LoadAccelerators(LEDIT_hinstance, MAKEINTRESOURCE(IDR_LEDIT_ACCELERATOR));

	//
	// The message we get when a mouse wheel event occurs.
	//

	LEDIT_wm_mousewheel = RegisterWindowMessage(MSH_MOUSEWHEEL);

	//
	// Our current directory.
	// 

	GetCurrentDirectory(_MAX_PATH, LEDIT_default_dir);

	//
	// The default directories of the map and lighting files.
	// 

	sprintf(LEDIT_ofn_default_dir_map,   "%s\\data",           LEDIT_default_dir);
	sprintf(LEDIT_ofn_default_dir_light, "%s\\data\\lighting", LEDIT_default_dir);

	//
	// Initialise the file structures.
	//

	LEDIT_ofn_map.lStructSize       = sizeof(OPENFILENAME);
	LEDIT_ofn_map.hwndOwner         = LEDIT_handle_frame;
	LEDIT_ofn_map.hInstance         = NULL;
	LEDIT_ofn_map.lpstrFilter       = "Game map files\0*.iam\0\0";
	LEDIT_ofn_map.lpstrCustomFilter = NULL;
	LEDIT_ofn_map.nMaxCustFilter    = 0;
	LEDIT_ofn_map.nFilterIndex      = 0;
	LEDIT_ofn_map.lpstrFile         = LEDIT_ofn_file_map;
	LEDIT_ofn_map.nMaxFile          = _MAX_PATH;
	LEDIT_ofn_map.lpstrFileTitle    = NULL;
	LEDIT_ofn_map.nMaxFileTitle     = 0;
	LEDIT_ofn_map.lpstrInitialDir   = LEDIT_ofn_default_dir_map;
	LEDIT_ofn_map.lpstrTitle        = "Load a game map";
	LEDIT_ofn_map.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	LEDIT_ofn_map.nFileOffset       = 0;
	LEDIT_ofn_map.nFileExtension    = 0;
	LEDIT_ofn_map.lpstrDefExt       = "iam";
	LEDIT_ofn_map.lCustData         = NULL;
	LEDIT_ofn_map.lpfnHook          = NULL;
	LEDIT_ofn_map.lpTemplateName    = NULL;

	LEDIT_ofn_light.lStructSize       = sizeof(OPENFILENAME);
	LEDIT_ofn_light.hwndOwner         = LEDIT_handle_frame;
	LEDIT_ofn_light.hInstance         = NULL;
	LEDIT_ofn_light.lpstrFilter       = "Lighting files\0*.lgt\0\0";
	LEDIT_ofn_light.lpstrCustomFilter = NULL;
	LEDIT_ofn_light.nMaxCustFilter    = 0;
	LEDIT_ofn_light.nFilterIndex      = 0;
	LEDIT_ofn_light.lpstrFile         = LEDIT_ofn_file_light;
	LEDIT_ofn_light.nMaxFile          = _MAX_PATH;
	LEDIT_ofn_light.lpstrFileTitle    = NULL;
	LEDIT_ofn_light.nMaxFileTitle     = 0;
	LEDIT_ofn_light.lpstrInitialDir   = LEDIT_ofn_default_dir_light;
	LEDIT_ofn_light.lpstrTitle        = "Load a lighting file";
	LEDIT_ofn_light.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	LEDIT_ofn_light.nFileOffset       = 0;
	LEDIT_ofn_light.nFileExtension    = 0;
	LEDIT_ofn_light.lpstrDefExt       = "lgt";
	LEDIT_ofn_light.lCustData         = NULL;
	LEDIT_ofn_light.lpfnHook          = NULL;
	LEDIT_ofn_light.lpTemplateName    = NULL;

	//
	// The program proper...
	//

	GI_init();
	ED_init();

	{
		MSG msg;
		int ret;

		while(1)
		{
			if (!PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
			{
				//
				// No messages pending- send a user message so we can
				// do our processing and display the engine.
				//

				PostMessage(LEDIT_handle_frame, WM_USER, 0, 0);
			}

			ret = GetMessage(&msg, NULL, 0, 0);

			if (ret == 0 || ret == -1)
			{
				break;
			}

			if (!TranslateAccelerator(LEDIT_handle_frame, LEDIT_accel, &msg))
			{
				//
				// Pass the message to the message handler.
				//

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	
	//
	// Clean up.
	//

	GI_fini();

	DestroyWindow(LEDIT_handle_frame);
	UnregisterClass(LEDIT_name_frame, LEDIT_hinstance);
}

