#include <MFStdLib.h>
#include <windows.h>
#include <windowsx.h>
#include <ddlib.h>
#include <zmouse.h>		// Mouse wheel support
#include "inline.h"
#include "resource.h"
#include "gi.h"
#include "fmatrix.h"
#include "es.h"
#include "ns.h"
#include "fallen/editor/headers/prim.h"
#include "fallen/headers/building.h"
#include	"fallen/headers/memory.h"


HINSTANCE SEDIT_hinstance;

WNDCLASSEX SEDIT_class_frame;
WNDCLASSEX SEDIT_class_engine;

HWND SEDIT_handle_frame;
HWND SEDIT_handle_engine;

CBYTE *SEDIT_name_frame  = "Urban Chaos sewer editor";
CBYTE *SEDIT_name_engine = "Engine window";

HMENU  SEDIT_main_menu;
HACCEL SEDIT_accel;

//
// Our cursors and icons.
//

HCURSOR SEDIT_arrow;
HCURSOR SEDIT_busy_bee;
HICON   SEDIT_icon;

//
// The current prim.
//

SLONG SEDIT_prim_object;
SLONG SEDIT_prim_yaw;

//
// The mousewheel message.
//

UINT SEDIT_wm_mousewheel;

//
// The current map.
//

CBYTE SEDIT_map_name[_MAX_PATH];
SLONG SEDIT_map_valid;		// TRUE => A map is loaded.

//
// The program default directory.
//

CBYTE SEDIT_default_dir[_MAX_PATH];

//
// Our file access structures.
//

OPENFILENAME SEDIT_ofn_map;
CBYTE        SEDIT_ofn_default_dir_map[_MAX_PATH];
CBYTE        SEDIT_ofn_file_map       [_MAX_PATH];

OPENFILENAME SEDIT_ofn_sewers;
CBYTE        SEDIT_ofn_default_dir_sewers[_MAX_PATH];
CBYTE        SEDIT_ofn_file_sewers       [_MAX_PATH];

//
// The mouse in the world.
//

SLONG SEDIT_mouse_valid;
SLONG SEDIT_mouse_world_x;
SLONG SEDIT_mouse_world_y;
SLONG SEDIT_mouse_world_z;
SLONG SEDIT_mouse_map_x;
SLONG SEDIT_mouse_map_z;
SLONG SEDIT_mouse_light;

//
// Our current engine view.
//

#define SEDIT_VIEW_TYPE_NONE   0
#define SEDIT_VIEW_TYPE_SEWERS 1
#define SEDIT_VIEW_TYPE_EDITOR 2
#define SEDIT_VIEW_TYPE_ENGINE 3

//
// View flags.
//

#define SEDIT_VIEW_FLAG_SHOW_BUILDINGS	(1 << 0)

SLONG SEDIT_view_flag;
SLONG SEDIT_view_type;

//
// Your current tool.
//

#define SEDIT_TOOL_PLACE_SEWERS		1
#define SEDIT_TOOL_PLACE_GROUND		2
#define SEDIT_TOOL_PLACE_ROCK		3
#define SEDIT_TOOL_PLACE_WATER		4
#define SEDIT_TOOL_PLACE_HOLE		5
#define SEDIT_TOOL_PLACE_ENTRANCE	6
#define SEDIT_TOOL_PLACE_GRATING	7
#define SEDIT_TOOL_PLACE_PRIM		8
#define SEDIT_TOOL_PLACE_LADDER		9
#define SEDIT_TOOL_EDIT_LIGHT		10

SLONG SEDIT_tool;

//
// In the engine window we can place down water!
//

SLONG SEDIT_city_water_place;
SLONG SEDIT_city_water_place_state;


//
// What we are currently doing.
//

#define SEDIT_DOING_NOTHING			   0
#define SEDIT_DOING_PAINT_SEWERS	   1
#define SEDIT_DOING_PAINT_GROUND	   2
#define SEDIT_DOING_PAINT_ROCK		   3
#define SEDIT_DOING_PAINT_HOLE		   4
#define SEDIT_DOING_PAINT_WATER_ON	   5
#define SEDIT_DOING_PAINT_WATER_OFF    6
#define SEDIT_DOING_PAINT_ENTRANCE_ON  7
#define SEDIT_DOING_PAINT_ENTRANCE_OFF 8
#define SEDIT_DOING_PAINT_GRATING_ON   9
#define SEDIT_DOING_PAINT_GRATING_OFF  10
#define SEDIT_DOING_DRAG_LIGHT		   11
#define SEDIT_DOING_PLACE_LADDER	   12

SLONG SEDIT_doing;

//
// For placing ladders.
//

SLONG SEDIT_ladder_mid_x;
SLONG SEDIT_ladder_mid_z;

//
// The camera.
//

SLONG SEDIT_cam_x;
SLONG SEDIT_cam_y;
SLONG SEDIT_cam_z;
SLONG SEDIT_cam_yaw;
SLONG SEDIT_cam_pitch;
SLONG SEDIT_cam_focus_x;
SLONG SEDIT_cam_focus_y;
SLONG SEDIT_cam_focus_z;
SLONG SEDIT_cam_focus_dist;

SLONG SEDIT_cam_matrix [9];
SLONG SEDIT_cam_forward[3];	// The movement vector forward
SLONG SEDIT_cam_left   [3]; // The movement vector left


//
// Changes the windows and menus to reflect the current state.
//

CBYTE SEDIT_engine_window_text[256];

void SEDIT_set_state_look()
{
	CBYTE *tool_name;

	switch(SEDIT_tool)
	{
		case SEDIT_TOOL_PLACE_SEWERS	: tool_name = "Place sewers";   break;
		case SEDIT_TOOL_PLACE_GROUND	: tool_name = "Place ground";   break;
		case SEDIT_TOOL_PLACE_ROCK		: tool_name = "Place rock"; 	break;
		case SEDIT_TOOL_PLACE_WATER		: tool_name = "Place water"; 	break;
		case SEDIT_TOOL_PLACE_HOLE		: tool_name = "Place hole";		break;
		case SEDIT_TOOL_PLACE_ENTRANCE	: tool_name = "Place entrance";	break;
		case SEDIT_TOOL_PLACE_GRATING	: tool_name = "Place grating";	break;
		case SEDIT_TOOL_PLACE_PRIM		: tool_name = "Place prim (Doens't work yet)"; break;
		case SEDIT_TOOL_PLACE_LADDER	: tool_name = "Place ladder";	break;
		case SEDIT_TOOL_EDIT_LIGHT		: tool_name = "Edit light";		break;
		default:
			tool_name = "";
			break;
	}

	sprintf(SEDIT_engine_window_text, "%s : %s", SEDIT_map_name, tool_name);

	//
	// Name of the map:tool in the engine window.
	//

	SetWindowText(SEDIT_handle_engine, SEDIT_engine_window_text);

	if (!SEDIT_map_valid)
	{
		//
		// No map loaded. Hide the engine window and invalidate all the menuitems.
		//

		ShowWindow(SEDIT_handle_engine, SW_HIDE);

		//
		// Untick irrelevant options.
		//

		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_SEWERS,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GROUND,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ROCK,     MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_WATER,    MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_HOLE,     MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ENTRANCE, MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GRATING,  MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_PRIM,     MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_LADDER,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_EDIT_LIGHT,     MF_UNCHECKED);

		CheckMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_ENGINE, MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_EDITOR, MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_VIEW_CITY_ENGINE,  MF_UNCHECKED);

		//
		// Invalidate most of the menu items.
		//

		EnableMenuItem(SEDIT_main_menu, ID_FILE_LOAD_ARSE, MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_FILE_SAVE_ARSE, MF_GRAYED);

		EnableMenuItem(SEDIT_main_menu, ID_EDIT_UNDO_ARSE, MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_REDO_ARSE, MF_GRAYED);

		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_SEWERS,   MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GROUND,   MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ROCK,     MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_WATER,    MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_HOLE,     MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ENTRANCE, MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GRATING,  MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_PRIM,     MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_LADDER,   MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_EDIT_LIGHT,     MF_GRAYED);

		EnableMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_ENGINE,   MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_EDITOR,   MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_VIEW_CITY_ENGINE,    MF_GRAYED);
		EnableMenuItem(SEDIT_main_menu, ID_VIEW_SHOW_BUILDINGS, MF_GRAYED);

		//
		// Hide the engine window.
		//

		ShowWindow(SEDIT_handle_engine, SW_HIDE);
	}
	else
	{
		//
		// Validate the menu items.
		//

		EnableMenuItem(SEDIT_main_menu, ID_FILE_LOAD_SEWERS, MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_FILE_SAVE_SEWERS, MF_ENABLED);

		EnableMenuItem(SEDIT_main_menu, ID_EDIT_UNDO_ARSE, MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_REDO_ARSE, MF_ENABLED);

		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_SEWERS,   MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GROUND,   MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ROCK,     MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_WATER,    MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_HOLE,     MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ENTRANCE, MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GRATING,  MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_PRIM,     MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_LADDER,   MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_EDIT_EDIT_LIGHT,     MF_ENABLED);

		EnableMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_ENGINE,   MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_EDITOR,   MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_VIEW_CITY_ENGINE,    MF_ENABLED);
		EnableMenuItem(SEDIT_main_menu, ID_VIEW_SHOW_BUILDINGS, MF_ENABLED);

		//
		// Make sure the correct menuitems are checked.
		//

		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_SEWERS,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GROUND,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ROCK,     MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_WATER,    MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_HOLE,     MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ENTRANCE, MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GRATING,  MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_PRIM,     MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_LADDER,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_EDIT_EDIT_LIGHT,     MF_UNCHECKED);

		CheckMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_ENGINE,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_EDITOR,   MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_VIEW_CITY_ENGINE,    MF_UNCHECKED);
		CheckMenuItem(SEDIT_main_menu, ID_VIEW_SHOW_BUILDINGS, MF_UNCHECKED);

		switch(SEDIT_tool)
		{
			case SEDIT_TOOL_PLACE_SEWERS:	CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_SEWERS,   MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_GROUND:	CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GROUND,   MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_ROCK:		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ROCK,     MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_WATER:	CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_WATER,    MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_HOLE:		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_HOLE,     MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_ENTRANCE:	CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_ENTRANCE, MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_GRATING:	CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_GRATING,  MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_PRIM:		CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_PRIM,     MF_CHECKED); break;
			case SEDIT_TOOL_PLACE_LADDER:	CheckMenuItem(SEDIT_main_menu, ID_EDIT_PLACE_LADDER,   MF_CHECKED); break;
			case SEDIT_TOOL_EDIT_LIGHT:		CheckMenuItem(SEDIT_main_menu, ID_EDIT_EDIT_LIGHT,     MF_CHECKED); break;

			default:
				ASSERT(0);
				break;
		}

		switch(SEDIT_view_type)
		{
			case SEDIT_VIEW_TYPE_NONE:
				break;

			case SEDIT_VIEW_TYPE_SEWERS: CheckMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_ENGINE, MF_CHECKED); break;
			case SEDIT_VIEW_TYPE_EDITOR: CheckMenuItem(SEDIT_main_menu, ID_VIEW_SEWER_EDITOR, MF_CHECKED); break;
			case SEDIT_VIEW_TYPE_ENGINE: CheckMenuItem(SEDIT_main_menu, ID_VIEW_CITY_ENGINE,  MF_CHECKED); break;

			default:
				ASSERT(0);
				break;
		}

		//
		// Show building flag.
		//

		if (SEDIT_view_flag & SEDIT_VIEW_FLAG_SHOW_BUILDINGS)
		{
			CheckMenuItem(SEDIT_main_menu, ID_VIEW_SHOW_BUILDINGS, MF_CHECKED);
		}

		//
		// Undo/redo support.
		//

		if (!ES_undo_undo_valid()) {EnableMenuItem(SEDIT_main_menu, ID_EDIT_UNDO_ARSE, MF_GRAYED);}
		if (!ES_undo_redo_valid()) {EnableMenuItem(SEDIT_main_menu, ID_EDIT_REDO_ARSE, MF_GRAYED);}

		//
		// Show the engine window.
		//

		ShowWindow(SEDIT_handle_engine, SW_SHOW);
	}

	//
	// Update the menu bar.
	//

	DrawMenuBar((struct HWND__*)SEDIT_main_menu);
}

//
// Undo functions.
//

void SEDIT_make_undoable()
{
	ES_undo_store();
	SEDIT_set_state_look();
}

void SEDIT_undo()
{
	ES_undo_undo();
	SEDIT_set_state_look();
}

void SEDIT_redo()
{
	ES_undo_redo();
	SEDIT_set_state_look();
}



//
// Calculates the camera position and matrix from the cam_focus
// and yaw,pitch variables.
//

void SEDIT_calc_camera_pos()
{
	FMATRIX_calc(
		SEDIT_cam_matrix,
		SEDIT_cam_yaw,
		SEDIT_cam_pitch,
		0);

	SEDIT_cam_x = SEDIT_cam_focus_x;
	SEDIT_cam_y = SEDIT_cam_focus_y;
	SEDIT_cam_z = SEDIT_cam_focus_z;

	SEDIT_cam_x -= MUL64(SEDIT_cam_matrix[6], SEDIT_cam_focus_dist);
	SEDIT_cam_y -= MUL64(SEDIT_cam_matrix[7], SEDIT_cam_focus_dist);
	SEDIT_cam_z -= MUL64(SEDIT_cam_matrix[8], SEDIT_cam_focus_dist);

	FMATRIX_vector(
		SEDIT_cam_forward,
		SEDIT_cam_yaw,
		0);

	FMATRIX_vector(
		SEDIT_cam_left,
		(SEDIT_cam_yaw + 512) & 2047,
		0);
}


//
// Loads in a map.
//

void SEDIT_load_map(CBYTE *name)
{
	//
	// Change the cursor to a busy bee.
	//

	SetCursor(SEDIT_busy_bee);

	//
	// Change to the default directory.
	//

	SetCurrentDirectory(SEDIT_default_dir);

	//
	// Load in the new map.
	//

	SEDIT_map_valid  = GI_load_map(name);

	if (SEDIT_map_valid)
	{
		strcpy(SEDIT_map_name, name);
	}
	else
	{
		strcpy(SEDIT_map_name, "No map loaded");
	}

	//
	// Initial map editing state.
	//

	SEDIT_view_type        = SEDIT_VIEW_TYPE_ENGINE;
	SEDIT_view_flag        = 0;
	SEDIT_tool             = SEDIT_TOOL_PLACE_SEWERS;
	SEDIT_doing            = SEDIT_DOING_NOTHING;
	SEDIT_city_water_place = FALSE;

	//
	// Start looking at the middle of the map.
	//

	SEDIT_cam_focus_x    = 64 << 8;
	SEDIT_cam_focus_y    =  8 << 8;
	SEDIT_cam_focus_z    = 64 << 8;
	SEDIT_cam_focus_dist = 10 << 8;
	SEDIT_cam_pitch      = 1600;
	SEDIT_cam_yaw        = 0;

	SEDIT_calc_camera_pos();

	//
	// Draw the engine.
	//

	GI_render_view_into_backbuffer(
		SEDIT_cam_x,
		SEDIT_cam_y,
		SEDIT_cam_z,
		SEDIT_cam_yaw,
		SEDIT_cam_pitch,
		0);

	//
	// Make sure the engine draws something.
	//

	InvalidateRect(SEDIT_handle_engine, NULL, FALSE);

	//
	// Change the cursor back to normal.
	//

	SetCursor(SEDIT_arrow);

	//
	// Change the look of the game.
	//

	SEDIT_set_state_look();

	//
	// Change the default sewer file name.
	//

	//
	// The default sewer file name.
	//

	{
		CBYTE *ch;
		CBYTE *ci;

		for (ch = name; *ch; ch++);

		while(1)
		{
			if (ch == name)
			{
				break;
			}

			if (*ch == '\\')
			{
				ch++;

				break;
			}

			ch--;
		}

		ci = SEDIT_ofn_file_sewers;

		while(1)
		{
			*ci++ = *ch++;

			if (*ch == '.' || *ch == '\000')
			{
				break;
			}
		}

		*ci++ = '\000';
	}
}

//
// Saves the sewers.
//

void SEDIT_sewers_save(void)
{
	SEDIT_ofn_sewers.lpstrTitle = "Save a sewer file.";

 	if (!GetSaveFileName(&SEDIT_ofn_sewers))
	{
		return;
	}

	//
	// Save the sewer map.
	//

	SetCursor(SEDIT_busy_bee);
	ES_save(SEDIT_ofn_file_sewers);
	SetCursor(SEDIT_arrow);

	//
	// Get everything looking right.
	//

	SEDIT_set_state_look();
}


//
// Loads in the sewers.
//

void SEDIT_sewers_load(void)
{
	SEDIT_ofn_sewers.lpstrTitle = "Load a sewer file.";

	if (ES_undo_undo_valid())
	{
		switch(MessageBox(
					SEDIT_handle_frame,
					"Save changes to current map?",
					"Load sewers",
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL))
		{
			case IDYES:
				SEDIT_sewers_save();
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

 	if (!GetOpenFileName(&SEDIT_ofn_sewers))
	{
		return;
	}

	//
	// Load the sewer map.
	//

	SetCursor(SEDIT_busy_bee);
	ES_load(SEDIT_ofn_file_sewers);
	SetCursor(SEDIT_arrow);

	if (SEDIT_view_type == SEDIT_VIEW_TYPE_SEWERS)
	{
		//
		// Build the new sewer system.
		//

		ES_build_sewers();
	}

	//
	// Get everything looking right.
	//

	SEDIT_set_state_look();
}

//
// Exits the sewer editor.
//

void SEDIT_request_exit()
{
	if (ES_undo_undo_valid())
	{
		switch (MessageBox(
					SEDIT_handle_frame,
					"Save changes to current map?",
					"Exit sewer editor",
					MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL))
		{
			case IDYES:
				SEDIT_sewers_save();
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
// The processing function.
//

void SEDIT_process(void)
{
	POINT mouse;
	RECT  clientrect;

	SLONG df;
	SLONG dl;
	SLONG dy;
	SLONG dp;
	SLONG dd;

	if (!SEDIT_map_valid)
	{
		//
		// No map loaded.
		//

		return;
	}

	if (Keys[KB_BS])
	{
		Keys[KB_BS] = 0;

		//
		// Delete something?
		//

		if (SEDIT_mouse_valid)
		{
			switch(SEDIT_tool)
			{
				case SEDIT_TOOL_EDIT_LIGHT:
					ES_light_delete(
						SEDIT_mouse_world_x,
						SEDIT_mouse_world_z);
					break;

				case SEDIT_TOOL_PLACE_LADDER:
					ES_ladder_delete(
						SEDIT_mouse_world_x,
						SEDIT_mouse_world_z);
					break;

				case SEDIT_TOOL_PLACE_PRIM:
					ES_prim_delete(
						SEDIT_mouse_world_x,
						SEDIT_mouse_world_y,
						SEDIT_mouse_world_z);
					break;

				default:
					break;
			}
		}
	}

	if (Keys[KB_LBRACE]) {Keys[KB_LBRACE] = 0; SEDIT_prim_yaw -= 128;}
	if (Keys[KB_RBRACE]) {Keys[KB_RBRACE] = 0; SEDIT_prim_yaw += 128;}

	SEDIT_prim_yaw &= 2047;

	//
	// Do engine movement.
	//

	SEDIT_calc_camera_pos();

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

		SendMessage(SEDIT_handle_engine, WM_MOUSEMOVE, 0, 0); // Doens't care where the mouse is...
	}

	//
	// Update position.
	//

	SEDIT_cam_focus_x += df * SEDIT_cam_forward[0] >> 11;
	SEDIT_cam_focus_z += df * SEDIT_cam_forward[2] >> 11;

	SEDIT_cam_focus_x += dl * SEDIT_cam_left[0] >> 11;
	SEDIT_cam_focus_z += dl * SEDIT_cam_left[2] >> 11;

	SEDIT_cam_focus_dist += dd * 16;
	SEDIT_cam_yaw        += dy * 16;
	SEDIT_cam_pitch      += dp * 16;

	SEDIT_cam_yaw   &= 2047;
	SEDIT_cam_pitch &= 2047;

	//
	// The y of the focus is more tricky.
	//

	SLONG want_focus_y;

	switch(SEDIT_view_type)
	{
		case SEDIT_VIEW_TYPE_SEWERS:
			want_focus_y = NS_calc_height_at(SEDIT_cam_focus_x, SEDIT_cam_focus_z);
			break;

		case SEDIT_VIEW_TYPE_EDITOR:

			if (WITHIN(SEDIT_cam_focus_x >> 8, 0, PAP_SIZE_HI - 1) &&
				WITHIN(SEDIT_cam_focus_z >> 8, 0, PAP_SIZE_HI - 1))
			{
				ES_Hi *eh = &ES_hi[SEDIT_cam_focus_x >> 8][SEDIT_cam_focus_z >> 8];

				want_focus_y = (eh->height << 5) + (-32 * 0x100);
			}
			else
			{
				want_focus_y = -0x200;
			}

			break;

		case SEDIT_VIEW_TYPE_ENGINE:
			want_focus_y = 0x100;
			break;

		default:
			ASSERT(0);
			break;
	}

	SLONG dfocus_y;

	dfocus_y   = want_focus_y - SEDIT_cam_focus_y;
	dfocus_y >>= 4;

	SATURATE(dfocus_y, -16, +16);

	SEDIT_cam_focus_y += dfocus_y;

	SEDIT_calc_camera_pos();

	switch(SEDIT_view_type)
	{
		case SEDIT_VIEW_TYPE_SEWERS:

			//
			// Draw the sewers.
			//

			GI_set_view(GI_VIEW_SEWERS);

			GI_render_view_into_backbuffer(
				SEDIT_cam_x,
				SEDIT_cam_y,
				SEDIT_cam_z,
				SEDIT_cam_yaw,
				SEDIT_cam_pitch,
				0);

			SEDIT_mouse_valid = FALSE;

			break;

		case SEDIT_VIEW_TYPE_EDITOR:

			GetCursorPos(&mouse);
			ScreenToClient(SEDIT_handle_engine, &mouse);
			GetClientRect(SEDIT_handle_engine, &clientrect);

			ES_draw_editor(
				SEDIT_cam_x,
				SEDIT_cam_y,
				SEDIT_cam_z,
				SEDIT_cam_yaw,
				SEDIT_cam_pitch,
				0,
				mouse.x,
				mouse.y,
			   &SEDIT_mouse_valid,
			   &SEDIT_mouse_world_x,
			   &SEDIT_mouse_world_y,
			   &SEDIT_mouse_world_z,
			    (SEDIT_tool == SEDIT_TOOL_PLACE_PRIM) && SEDIT_prim_object,
				SEDIT_prim_object,
				SEDIT_prim_yaw);

			break;

		case SEDIT_VIEW_TYPE_ENGINE:

			//
			// Draw the engine.
			//

			GI_set_view(GI_VIEW_CITY);

			GI_render_view_into_backbuffer(
				SEDIT_cam_x,
				SEDIT_cam_y,
				SEDIT_cam_z,
				SEDIT_cam_yaw,
				SEDIT_cam_pitch,
				0);

			//
			// Is the mouse over the engine window?
			//

			GetCursorPos(&mouse);
			ScreenToClient(SEDIT_handle_engine, &mouse);
			GetClientRect(SEDIT_handle_engine, &clientrect);

			if (WITHIN(mouse.x, clientrect.left, clientrect.right) &&
				WITHIN(mouse.y, clientrect.top,  clientrect.bottom))
			{
				SEDIT_mouse_valid = GI_get_pixel_world_pos(
										mouse.x,
										mouse.y,
									   &SEDIT_mouse_world_x,
									   &SEDIT_mouse_world_y,
									   &SEDIT_mouse_world_z);

			}
			else
			{
				SEDIT_mouse_valid = FALSE;
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	if (SEDIT_mouse_valid)
	{
		SEDIT_mouse_map_x = SEDIT_mouse_world_x >> PAP_SHIFT_HI;
		SEDIT_mouse_map_z = SEDIT_mouse_world_z >> PAP_SHIFT_HI;

		ASSERT(WITHIN(SEDIT_mouse_map_x, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(SEDIT_mouse_map_z, 0, PAP_SIZE_HI - 1));
	}

	//
	// Make sure the engine draws something.
	//

	InvalidateRect(SEDIT_handle_engine, NULL, FALSE);
}


//
// Callback function for the frame window
//

LRESULT CALLBACK SEDIT_callback_frame(
					HWND   window_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	SLONG dwheel;
	SLONG dheight;
	SLONG scancode;

	if (message_type == SEDIT_wm_mousewheel || message_type == WM_MOUSEWHEEL)
	{
		//
		// Turning the mousewheel.
		//

		dwheel = (((short) LOWORD(param_w)) * 64 / 120) + (((short) HIWORD(param_w)) * 64 / 120);

		if (SEDIT_mouse_valid)
		{
			if (SEDIT_view_type == SEDIT_VIEW_TYPE_ENGINE)
			{
				dheight = (dwheel < 0) ? -16 : +16;

				//
				// Change the height of the city water.
				//

				ES_city_water_dlevel(
					SEDIT_mouse_world_x,
					SEDIT_mouse_world_z,
					dheight);
			}
			else
			{
				switch(SEDIT_tool)
				{
					case SEDIT_TOOL_EDIT_LIGHT:

						dheight = (dwheel < 0) ? -2 : +2;

						//
						// Change the light height.
						//

						ES_light_dheight(
							SEDIT_mouse_world_x,
							SEDIT_mouse_world_z,
							dheight);

						break;

					case SEDIT_TOOL_PLACE_LADDER:

						dheight = (dwheel < 0) ? -4 : +4;

						//
						// Change the ladder height.
						//

						ES_ladder_dheight(
							SEDIT_mouse_world_x,
							SEDIT_mouse_world_z,
							dheight);

						break;

					case SEDIT_TOOL_PLACE_WATER:

						dheight = (dwheel < 0) ? -1 : +1;

						//
						// Change the water height.
						//

						ES_sewer_water_dheight(
							SEDIT_mouse_world_x,
							SEDIT_mouse_world_z,
							dheight);

						break;

					case SEDIT_TOOL_PLACE_PRIM:

						dheight = (dwheel < 0) ? -2 : +2;

						//
						// Change the prim height.
						//

						ES_prim_dheight(
							SEDIT_mouse_world_x,
							SEDIT_mouse_world_y,
							SEDIT_mouse_world_z,
							dheight);

						break;

					default:

						dheight = (dwheel < 0) ? -8 : +8;

						ES_change_height(
							SEDIT_mouse_map_x,
							SEDIT_mouse_map_z,
							dheight);

						break;
				}
			}

			SEDIT_make_undoable();
		}

		return 0;
	}

	switch(message_type)
	{
        case WM_DESTROY:
			SEDIT_request_exit();
			return 0;

		case WM_COMMAND:

			switch(LOWORD(param_w))
			{
				case ID_FILE_EXIT:
					SEDIT_request_exit();
					break;

				case ID_FILE_OPEN_ARSE:

					if (GetOpenFileName(&SEDIT_ofn_map))
					{
						SEDIT_load_map(SEDIT_ofn_file_map);
					}

					break;

				case ID_FILE_LOAD_SEWERS:
					SEDIT_sewers_load();
					break;

				case ID_FILE_SAVE_SEWERS:
					SEDIT_sewers_save();
					break;

				case ID_EDIT_UNDO_ARSE:
					SEDIT_undo();
					break;

				case ID_EDIT_REDO_ARSE:
					SEDIT_redo();
					break;

				case ID_EDIT_PLACE_SEWERS	: SEDIT_tool = SEDIT_TOOL_PLACE_SEWERS;   SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_GROUND	: SEDIT_tool = SEDIT_TOOL_PLACE_GROUND;   SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_ROCK		: SEDIT_tool = SEDIT_TOOL_PLACE_ROCK;     SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_WATER	: SEDIT_tool = SEDIT_TOOL_PLACE_WATER;    SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_HOLE		: SEDIT_tool = SEDIT_TOOL_PLACE_HOLE;     SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_ENTRANCE	: SEDIT_tool = SEDIT_TOOL_PLACE_ENTRANCE; SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_GRATING	: SEDIT_tool = SEDIT_TOOL_PLACE_GRATING;  SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_PRIM		: SEDIT_tool = SEDIT_TOOL_PLACE_PRIM;     SEDIT_set_state_look(); break;
				case ID_EDIT_PLACE_LADDER	: SEDIT_tool = SEDIT_TOOL_PLACE_LADDER;   SEDIT_set_state_look(); break;
				case ID_EDIT_EDIT_LIGHT		: SEDIT_tool = SEDIT_TOOL_EDIT_LIGHT;     SEDIT_set_state_look(); break;

				case ID_VIEW_SEWER_ENGINE:

					//
					// Prepare the game for entering the sewers- transfer the
					// editor map into the game data structures.
					//

					ES_build_sewers();

					SEDIT_view_type = SEDIT_VIEW_TYPE_SEWERS;
					SEDIT_set_state_look();
					break;
				case ID_VIEW_SEWER_EDITOR: SEDIT_view_type = SEDIT_VIEW_TYPE_EDITOR; SEDIT_set_state_look(); break;
				case ID_VIEW_CITY_ENGINE:  SEDIT_view_type = SEDIT_VIEW_TYPE_ENGINE; SEDIT_set_state_look(); break;

				case ID_VIEW_SHOW_BUILDINGS:
					SEDIT_view_flag ^= SEDIT_VIEW_FLAG_SHOW_BUILDINGS;
					SEDIT_set_state_look();
					break;

				case ID_HELP_ABOUT:
					MessageBox(
						SEDIT_handle_frame,
						"Mark is the guilty party!",
						"Urban Chaos sewer editor",
						MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
					break;

				default:
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

		case WM_USER:

			//
			// The message we send to ourselves when nothing else is happening.
			//

			SEDIT_process();

			return 0;

		case WM_KILLFOCUS:

			//
			// Stop the focus going elsewhere!
			//

			SetFocus(SEDIT_handle_frame);

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
// The callback function for the choose-a-prim dialog box.
//

LRESULT CALLBACK SEDIT_callback_choose_prim(
					HWND   dialog_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	SLONG i;
	SLONG item;
	HWND  list_handle;
	RECT  rect;
	POINT mouse;

	static SLONG selection;

	switch(message_type)
	{
		case WM_INITDIALOG:

			if (SEDIT_map_valid)
			{
				list_handle = GetDlgItem(dialog_handle, IDC_PRIM_LIST);

				for (i = 1; i < next_prim_object; i++)
				{
					SendMessage(
						list_handle,
						LB_ADDSTRING,
						0,
						(long) prim_names[i]);
//						(long) prim_objects[i].ObjectName);
				}

				selection = 0;

				//
				// Move the window to the mouse cursor position.
				//

				GetWindowRect(dialog_handle, &rect);
				GetCursorPos (&mouse);
				MoveWindow   (dialog_handle, mouse.x, mouse.y, rect.right - rect.left, rect.bottom - rect.top, FALSE);
			}
			else
			{
				EndDialog(dialog_handle, FALSE);
			}

			return 0;

		case WM_COMMAND:

			switch(LOWORD(param_w))
			{
				case IDC_PRIM_LIST:

					if (HIWORD(param_w) == LBN_SELCHANGE)
					{
						list_handle = GetDlgItem (dialog_handle, IDC_PRIM_LIST);
						item        = SendMessage(list_handle, LB_GETCURSEL, 0, 0);
						selection   = item + 1;

						return TRUE;
					}

					break;

				case IDOK:
					EndDialog(dialog_handle, selection);
					return TRUE;

				case IDCANCEL:
					EndDialog(dialog_handle, 0);
					return TRUE;

				default:
					break;
			}

			break;

		default:
			break;
	}

	return FALSE;
}


//
// Callback function for the engine window
//

LRESULT CALLBACK SEDIT_callback_engine(
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
	ES_Hi      *eh;
	SLONG		ret;

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

		case WM_LBUTTONDOWN:

			if (SEDIT_mouse_valid)
			{
				if (SEDIT_view_type == SEDIT_VIEW_TYPE_ENGINE)
				{
					//
					// Start placing down water.
					//

					SEDIT_city_water_place       =  TRUE;
					SEDIT_city_water_place_state = !ES_city_water_get(SEDIT_mouse_world_x, SEDIT_mouse_world_z);
				}
				else
				{
					ASSERT(WITHIN(SEDIT_mouse_map_x, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(SEDIT_mouse_map_z, 0, PAP_SIZE_HI - 1));

					eh = &ES_hi[SEDIT_mouse_map_x][SEDIT_mouse_map_z];

					switch(SEDIT_doing)
					{
						case SEDIT_DOING_NOTHING:

							switch(SEDIT_tool)
							{
								case SEDIT_TOOL_PLACE_SEWERS:   SEDIT_doing = SEDIT_DOING_PAINT_SEWERS;   break;
								case SEDIT_TOOL_PLACE_GROUND:   SEDIT_doing = SEDIT_DOING_PAINT_GROUND;   break;
								case SEDIT_TOOL_PLACE_ROCK:     SEDIT_doing = SEDIT_DOING_PAINT_ROCK;     break;
								case SEDIT_TOOL_PLACE_HOLE:     SEDIT_doing = SEDIT_DOING_PAINT_HOLE;     break;
								case SEDIT_TOOL_PLACE_WATER:
									SEDIT_doing = (eh->water) ? SEDIT_DOING_PAINT_WATER_OFF : SEDIT_DOING_PAINT_WATER_ON;
									break;
								case SEDIT_TOOL_PLACE_ENTRANCE:
									SEDIT_doing = (eh->flag & ES_FLAG_ENTRANCE) ? SEDIT_DOING_PAINT_ENTRANCE_OFF : SEDIT_DOING_PAINT_ENTRANCE_ON;
									break;
								case SEDIT_TOOL_PLACE_GRATING:
									SEDIT_doing = (eh->flag & ES_FLAG_GRATING) ? SEDIT_DOING_PAINT_GRATING_OFF : SEDIT_DOING_PAINT_GRATING_ON;
									break;
								case SEDIT_TOOL_EDIT_LIGHT:
									SEDIT_doing = SEDIT_DOING_DRAG_LIGHT;
									break;
								case SEDIT_TOOL_PLACE_LADDER:
									SEDIT_doing        = SEDIT_DOING_PLACE_LADDER;
									SEDIT_ladder_mid_x = SEDIT_mouse_world_x;
									SEDIT_ladder_mid_z = SEDIT_mouse_world_z;
									break;

								case SEDIT_TOOL_PLACE_PRIM:

									if (SEDIT_prim_object && SEDIT_mouse_valid)
									{
										//
										// Place the prim.
										//

										ES_prim_create(
											SEDIT_prim_object,
											SEDIT_mouse_world_x,
											SEDIT_mouse_world_y,
											SEDIT_mouse_world_z,
											SEDIT_prim_yaw);
									}

									break;

								default:
									break;
							}

							break;

						default:
							break;
					}
				}

				//
				// Pretend that the mouse has moved- because the action we
				// are doing only occurs when the mouse moves.
				//

				SendMessage(SEDIT_handle_engine, WM_MOUSEMOVE, 0, 0); // Doens't care where the mouse is...
			}

			//
			// Grab control of the mouse.
			//

			SetCapture(SEDIT_handle_engine);

			return 0;

		case WM_LBUTTONUP:

			switch(SEDIT_doing)
			{
				case SEDIT_DOING_PAINT_SEWERS:
				case SEDIT_DOING_PAINT_GROUND:
				case SEDIT_DOING_PAINT_ROCK:
				case SEDIT_DOING_PAINT_HOLE:
				case SEDIT_DOING_PAINT_WATER_ON:
				case SEDIT_DOING_PAINT_WATER_OFF:
				case SEDIT_DOING_PAINT_ENTRANCE_ON:
				case SEDIT_DOING_PAINT_ENTRANCE_OFF:
				case SEDIT_DOING_PAINT_GRATING_ON:
				case SEDIT_DOING_PAINT_GRATING_OFF:
				case SEDIT_DOING_DRAG_LIGHT:
					SEDIT_doing = SEDIT_DOING_NOTHING;
					break;

				case SEDIT_DOING_PLACE_LADDER:

					if (SEDIT_mouse_valid)
					{
						ES_ladder_create(
							SEDIT_ladder_mid_x,
							SEDIT_ladder_mid_z,
							SEDIT_mouse_world_x,
							SEDIT_mouse_world_z);
					}

					SEDIT_doing = SEDIT_DOING_NOTHING;

					break;

				default:
					break;
			}

			SEDIT_city_water_place = FALSE;

			//
			// Make anything the user did undoable.
			//

			SEDIT_make_undoable();

			//
			// Release the capture on the mouse.
			//

			ReleaseCapture();

			return 0;

		case WM_MOUSEMOVE:

			if (SEDIT_mouse_valid)
			{
				if (SEDIT_view_type == SEDIT_VIEW_TYPE_ENGINE)
				{
					if (SEDIT_city_water_place)
					{
						ES_city_water_set(
							SEDIT_mouse_world_x,
							SEDIT_mouse_world_z,
							SEDIT_city_water_place_state);
					}
				}
				else
				{
					ASSERT(WITHIN(SEDIT_mouse_map_x, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(SEDIT_mouse_map_z, 0, PAP_SIZE_HI - 1));

					eh = &ES_hi[SEDIT_mouse_map_x][SEDIT_mouse_map_z];

					switch(SEDIT_doing)
					{
						case SEDIT_DOING_PAINT_SEWERS: eh->type = ES_TYPE_SEWER;  break;
						case SEDIT_DOING_PAINT_GROUND: eh->type = ES_TYPE_GROUND; break;
						case SEDIT_DOING_PAINT_ROCK:   eh->type = ES_TYPE_ROCK;   break;
						case SEDIT_DOING_PAINT_HOLE:   eh->type = ES_TYPE_HOLE;   break;

						case SEDIT_DOING_PAINT_WATER_ON:  eh->water = eh->height + 1; break;
						case SEDIT_DOING_PAINT_WATER_OFF: eh->water = 0;              break;

						case SEDIT_DOING_PAINT_ENTRANCE_ON:
						case SEDIT_DOING_PAINT_ENTRANCE_OFF:
							if (ShiftFlag)
							{
								eh->flag |=  ES_FLAG_NOCURBS;
							}
							else
							{
								eh->flag &= ~ES_FLAG_NOCURBS;
							}
							switch(SEDIT_doing)
							{
								case SEDIT_DOING_PAINT_ENTRANCE_ON:  eh->flag |=  ES_FLAG_ENTRANCE; break;
								case SEDIT_DOING_PAINT_ENTRANCE_OFF: eh->flag &= ~ES_FLAG_ENTRANCE; break;
							}
							break;

						case SEDIT_DOING_PAINT_GRATING_ON: 	eh->flag |=  ES_FLAG_GRATING; break;
						case SEDIT_DOING_PAINT_GRATING_OFF: eh->flag &= ~ES_FLAG_GRATING; break;

						case SEDIT_DOING_DRAG_LIGHT:
							ES_light_move(
								SEDIT_mouse_world_x,
								SEDIT_mouse_world_z);
							break;

						case SEDIT_DOING_PLACE_LADDER:
							break;

						default:
							break;
					}
				}
			}

			return 0;

		case WM_RBUTTONDOWN:

			ret = DialogBox(
					SEDIT_hinstance,
					MAKEINTRESOURCE(IDD_CHOOSE_PRIM),
					SEDIT_handle_engine,
					(DLGPROC) SEDIT_callback_choose_prim);

			if (ret)
			{
				SEDIT_prim_object = ret;
			}

			return 0;

		case WM_COMMAND:
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




void SEDIT_do(void)
{
	RECT rect;

	//
	// Stuff we need to know from the direct draw library.
	//

	extern HINSTANCE hGlobalThisInst;

	SEDIT_hinstance = hGlobalThisInst;

	//
	// Load our cursors.
	//

	SEDIT_arrow    = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	SEDIT_busy_bee = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));

	//
	// Our icons.
	//

	SEDIT_icon = LoadIcon(SEDIT_hinstance, MAKEINTRESOURCE(IDI_MFLOGO));

	//
	// Our frame window class.
	//

	SEDIT_class_frame.hInstance		= SEDIT_hinstance;
	SEDIT_class_frame.lpszClassName	= SEDIT_name_frame;
	SEDIT_class_frame.lpfnWndProc	= SEDIT_callback_frame;
	SEDIT_class_frame.style			= 0;
	SEDIT_class_frame.cbSize		= sizeof(WNDCLASSEX);
	SEDIT_class_frame.cbClsExtra	= 0;
	SEDIT_class_frame.cbWndExtra	= 0;
	SEDIT_class_frame.lpszMenuName	= NULL;
	SEDIT_class_frame.hIcon			= SEDIT_icon;
	SEDIT_class_frame.hIconSm		= SEDIT_icon;
	SEDIT_class_frame.hCursor		= LoadCursor(NULL, IDC_ARROW);
	SEDIT_class_frame.hbrBackground	= (struct HBRUSH__ *)GetStockObject(WHITE_BRUSH);

	if (!RegisterClassEx(&SEDIT_class_frame))
	{
		//
		// Could not register the class.
		//

		return;
	}
	//
	// Create the frame window.
	//

	SEDIT_handle_frame = CreateWindow(
							SEDIT_name_frame,
							SEDIT_name_frame,
							WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							NULL,
							NULL,
							SEDIT_hinstance,
							NULL);

	if (SEDIT_handle_frame == NULL)
	{
		//
		// Could not create our main window.
		//

		UnregisterClass(SEDIT_name_frame, SEDIT_hinstance);

		return;
	}

	//
	// The engine window class.
	//

	SEDIT_class_engine.hInstance		= SEDIT_hinstance;
	SEDIT_class_engine.lpszClassName	= SEDIT_name_engine;
	SEDIT_class_engine.lpfnWndProc		= SEDIT_callback_engine;
	SEDIT_class_engine.style			= 0;
	SEDIT_class_engine.cbSize			= sizeof(WNDCLASSEX);
	SEDIT_class_engine.cbClsExtra		= 0;
	SEDIT_class_engine.cbWndExtra		= 0;
	SEDIT_class_engine.lpszMenuName		= NULL;
	SEDIT_class_engine.hIcon			= NULL;
	SEDIT_class_engine.hIconSm			= NULL;
	SEDIT_class_engine.hCursor			= LoadCursor(NULL, IDC_ARROW);
	SEDIT_class_engine.hbrBackground	= (struct HBRUSH__ *)GetStockObject(GRAY_BRUSH);

	if (RegisterClassEx(&SEDIT_class_engine) == 0)
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
		 WS_CAPTION | WS_CHILD,
		 FALSE,
		 0);

	SEDIT_handle_engine = CreateWindow(
							SEDIT_name_engine,
							"No map loaded",
							WS_CAPTION | WS_CHILD,
							350,
							50,
							rect.right  - rect.left,
							rect.bottom - rect.top,
							SEDIT_handle_frame,
							NULL,
							SEDIT_hinstance,
							NULL);

	//
	// Our main menu.
	//

	SEDIT_main_menu = LoadMenu(SEDIT_hinstance, MAKEINTRESOURCE(IDR_SEDIT_MENU));

	SetMenu(SEDIT_handle_frame, SEDIT_main_menu);

	//
	// SetupHost does this- so so must we.
	//

	SetupMemory();

	//
	// Sneakily pretend that this was the window created by SetupHost!
	//

	extern volatile BOOL ShellActive;

	hDDLibWindow = SEDIT_handle_frame;
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

	ShowWindow(SEDIT_handle_frame, SW_MAXIMIZE);

	//
	// Make the back buffer a nice colour!
	//

	the_display.SetUserColour(0x7c, 0x11, 0x1d);
	the_display.SetUserBackground();
	the_display.ClearViewport();

	//
	// Load the accelerator table.
	//

	SEDIT_accel = LoadAccelerators(SEDIT_hinstance, MAKEINTRESOURCE(IDR_SEDIT_ACCELERATOR));

	//
	// The message we get when a mouse wheel event occurs.
	//

	SEDIT_wm_mousewheel = RegisterWindowMessage(MSH_MOUSEWHEEL);

	//
	// Our current directory.
	//

	GetCurrentDirectory(_MAX_PATH, SEDIT_default_dir);

	//
	// The default directories of the map and lighting files.
	//

	sprintf(SEDIT_ofn_default_dir_map,    "data");
	sprintf(SEDIT_ofn_default_dir_sewers, "data\\sewers", SEDIT_default_dir);

	//
	// Initialise the file structures.
	//

	SEDIT_ofn_map.lStructSize       = sizeof(OPENFILENAME);
	SEDIT_ofn_map.hwndOwner         = SEDIT_handle_frame;
	SEDIT_ofn_map.hInstance         = NULL;
	SEDIT_ofn_map.lpstrFilter       = "Game map files\0*.iam\0\0";
	SEDIT_ofn_map.lpstrCustomFilter = NULL;
	SEDIT_ofn_map.nMaxCustFilter    = 0;
	SEDIT_ofn_map.nFilterIndex      = 0;
	SEDIT_ofn_map.lpstrFile         = SEDIT_ofn_file_map;
	SEDIT_ofn_map.nMaxFile          = _MAX_PATH;
	SEDIT_ofn_map.lpstrFileTitle    = NULL;
	SEDIT_ofn_map.nMaxFileTitle     = 0;
	SEDIT_ofn_map.lpstrInitialDir   = SEDIT_ofn_default_dir_map;
	SEDIT_ofn_map.lpstrTitle        = "Load a game map";
	SEDIT_ofn_map.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	SEDIT_ofn_map.nFileOffset       = 0;
	SEDIT_ofn_map.nFileExtension    = 0;
	SEDIT_ofn_map.lpstrDefExt       = "iam";
	SEDIT_ofn_map.lCustData         = NULL;
	SEDIT_ofn_map.lpfnHook          = NULL;
	SEDIT_ofn_map.lpTemplateName    = NULL;

	SEDIT_ofn_sewers.lStructSize       = sizeof(OPENFILENAME);
	SEDIT_ofn_sewers.hwndOwner         = SEDIT_handle_frame;
	SEDIT_ofn_sewers.hInstance         = NULL;
	SEDIT_ofn_sewers.lpstrFilter       = "Sewer map files\0*.sew\0\0";
	SEDIT_ofn_sewers.lpstrCustomFilter = NULL;
	SEDIT_ofn_sewers.nMaxCustFilter    = 0;
	SEDIT_ofn_sewers.nFilterIndex      = 0;
	SEDIT_ofn_sewers.lpstrFile         = SEDIT_ofn_file_sewers;
	SEDIT_ofn_sewers.nMaxFile          = _MAX_PATH;
	SEDIT_ofn_sewers.lpstrFileTitle    = NULL;
	SEDIT_ofn_sewers.nMaxFileTitle     = 0;
	SEDIT_ofn_sewers.lpstrInitialDir   = SEDIT_ofn_default_dir_sewers;
	SEDIT_ofn_sewers.lpstrTitle        = "Load a sewer map";
	SEDIT_ofn_sewers.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	SEDIT_ofn_sewers.nFileOffset       = 0;
	SEDIT_ofn_sewers.nFileExtension    = 0;
	SEDIT_ofn_sewers.lpstrDefExt       = "sew";
	SEDIT_ofn_sewers.lCustData         = NULL;
	SEDIT_ofn_sewers.lpfnHook          = NULL;
	SEDIT_ofn_sewers.lpTemplateName    = NULL;

	//
	// The program proper...
	//

	GI_init();
	ES_init();

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

				PostMessage(SEDIT_handle_frame, WM_USER, 0, 0);
			}

			ret = GetMessage(&msg, NULL, 0, 0);

			if (ret == 0 || ret == -1)
			{
				break;
			}

			if (!TranslateAccelerator(SEDIT_handle_frame, SEDIT_accel, &msg))
			{
				//
				// Pass the message to the message handler.
				//

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	GI_fini();
}
