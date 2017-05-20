//
// The interface between the game and the light editor.
//

#ifndef _GI_
#define _GI_

//
// Call once at the start of the light editor.
//

void GI_init(void);


//
// Releases any old stuff and loads the given map file.
// Enters city view automatically.
// Returns TRUE on success.
//

SLONG GI_load_map(CBYTE *name);


//
// Changes the view. The view default to CITY view.
//

#define GI_VIEW_CITY	1
#define GI_VIEW_SEWERS	2

void GI_set_view(SLONG view);

//
// Renders the view into the the_display.lp_DD_BackSurface.
//

void GI_render_view_into_backbuffer(
		SLONG cam_x,
		SLONG cam_y,
		SLONG cam_z,
		SLONG cam_yaw,
		SLONG cam_pitch,
		SLONG cam_roll);

//
// Gives the position of the given screen pixel in the world.
// Returns FALSE if there was no intersection.
//

SLONG GI_get_pixel_world_pos(
		SLONG  sx,
		SLONG  sy,
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG inside=0);


//
// Draws a light into the backbuffer and returns which bits of the
// light the mouse is over.
//

#define GI_MOUSE_OVER_LIGHT_BOT (1 << 0)
#define GI_MOUSE_OVER_LIGHT_TOP (1 << 1)

ULONG GI_light_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight);	// Not a bool!


//
// Call once at the end of the light editor.
//

void GI_fini(void);

//---------------------------------------------------------------
//	GUY.
//---------------------------------------------------------------

ULONG GI_waypoint_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight);


ULONG GI_rad_trigger_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG rad,
		ULONG colour,
		UBYTE highlight);

//---------------------------------------------------------------
//  CANIS
//---------------------------------------------------------------

void GI_groundsquare_draw(
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE polyinit);

//---------------------------------------------------------------

//---------------------------------------------------------------

#endif
