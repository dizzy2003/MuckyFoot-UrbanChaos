//
// Another engine.
//

#ifndef _AENG_
#define _AENG_

//
//  defines
//

#define	KERB_HEIGHT	32.0F
#define	KERB_HEIGHTI	((SLONG)(KERB_HEIGHT))




// Set this to 1 to enable TomF's new D3D-friendly engine.
// 0 enables the old engine again.
// NOTE! There are other versions of this define dotted around in other header
// files! Make sure they all agree or you'll have grief.
#ifdef TARGET_DC
#define USE_TOMS_ENGINE_PLEASE_BOB 1
#else
#define USE_TOMS_ENGINE_PLEASE_BOB 1
#endif



//
// Call once at the start of the whole program.
//

void AENG_init(void);



//
// Chooses the texture set to use. If the current texture set is
// different from the new one, then all the textures are released.
//

void TEXTURE_choose_set(SLONG number);


//
// Loads only the textures that are used in the prim faces and the current map.
//

// iStartCompletionBar = where it is now.
// iEndCompletionBar = where it needs to get to.
// iNumberTexturesProbablyLoaded = roughly how many textures will be loaded.
void TEXTURE_load_needed(CBYTE*	fname_level,
						 int iStartCompletionBar = 0,
						 int iEndCompletionBar = 0,
						 int iNumberTexturesProbablyLoaded = 0
						 );

//
// Loads the textures needed for the given prim object.
//

void TEXTURE_load_needed_object(SLONG prim_object);


//
// This function makes a local copy of the prim points for
// the engine's own use.
//

typedef struct
{
	float X;
	float Y;
	float Z;

} SVector_F;
  
extern SVector_F AENG_dx_prim_points[];

void AENG_create_dx_prim_points(void);

//
// After you have loaded all the prims, call this function. It
// fixes the texture coordinates of the prims if the engine has
// fiddled with the texture pages.
// 

void TEXTURE_fix_texture_styles(void);
void TEXTURE_fix_prim_textures (void);

//
// Given a texture square coordinate on a page, it returns the
// page and texture square coordinates of the fiddled position.
//
// 'square_u' and 'square_v' should be between 0 and 7, and the
// fiddled position are returned.
//

SLONG TEXTURE_get_fiddled_position(
		SLONG  square_u,
		SLONG  square_v,
		SLONG  page,
		float *u,
		float *v);

//
// Initialises a new frame.
// Draws the engine.
//

void AENG_set_camera(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll);

void AENG_set_camera_radians(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		float yaw,
		float pitch,
		float roll);

//
// Get/set draw distance
//

void  AENG_set_draw_distance(SLONG dist);
SLONG AENG_get_draw_distance();

//
// Draws world lines.
//

// Just for debugging please!
void AENG_world_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front);

// For lines that aren't debug.
void AENG_world_line_nondebug (
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front);


//
// Draws the line as a series of smaller lines so you can draw lines.
// with endpoints out of the view frustrum.
//

void AENG_world_line_infinite(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front);

void	AENG_draw_col_tri(SLONG x0,SLONG y0,SLONG col0,SLONG x1,SLONG y1,SLONG col1,SLONG x2,SLONG z2,SLONG col2,SLONG layer);


//
// Older engine compatability.
//

void AENG_e_draw_3d_line           (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void AENG_e_draw_3d_line_dir       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void AENG_e_draw_3d_line_col       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void AENG_e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void AENG_e_draw_3d_mapwho         (SLONG x1,SLONG z1);
void AENG_e_draw_3d_mapwho_y       (SLONG x1,SLONG y1,SLONG z1);

void AENG_draw_rect(SLONG x,SLONG y,SLONG w,SLONG h,SLONG col,SLONG layer,SLONG page);

#define e_draw_3d_line            AENG_e_draw_3d_line
#define	e_draw_3d_line_dir        AENG_e_draw_3d_line_dir
#define	e_draw_3d_line_col        AENG_e_draw_3d_line_col
#define	e_draw_3d_line_col_sorted AENG_e_draw_3d_line_col_sorted
#define	e_draw_3d_mapwho          AENG_e_draw_3d_mapwho
#define	e_draw_3d_mapwho_y        AENG_e_draw_3d_mapwho_y

//
// Sets the type of sky to use.
//

void AENG_set_sky_nighttime(void);
void AENG_set_sky_daytime  (ULONG bottom_colour, ULONG top_colour);

//
// The new engines- you don't have to call these- just call
// AENG_draw() and it will take care of everything for you.
// 

void AENG_draw_city  (void);
void AENG_draw_inside(void);
void AENG_draw_sewer (void);
void AENG_draw_ns    (void);

//
// Draw the correct engine given the current gamestate. If
// (draw_3d) then it draws the scene red/blue for use with
// 3d glasses. Make sure that the textures are all greyscale.
// 

void AENG_draw(SLONG draw_3d);

//
// Clear the viewport (moved to just after flip for concurrency)
//

void AENG_clear_viewport();

//
// The scanner.
// 

void AENG_draw_scanner(
		SLONG screen_x1,
		SLONG screen_y1,
		SLONG screen_x2,
		SLONG screen_y2,
		SLONG map_x,
		SLONG map_z,
		SLONG map_zoom,		// The number of pixels per mapsquare in fixed-point 8.
		SLONG map_angle);


void AENG_draw_power(SLONG x,SLONG y,SLONG w,SLONG h,SLONG val,SLONG max);

//
// Draws the messages and the FPS stuff to the screen.
//

void AENG_draw_FPS(void);
void AENG_draw_messages(void);

//
// Fades in from black via the MF logo.
// Fades out to the mucky foot logo.
//

void AENG_fade_in (UBYTE amount);
void AENG_fade_out(UBYTE amount);


//
// Flips/blits the back-buffer.
//

void AENG_flip(void);
void AENG_blit(void);

//
// Adds a message to the message system.
//

void MSG_add(CBYTE *message, ...);
//#define MSG_add

//
// Drawing stuff straight to the screen...
//

void  AENG_clear_screen(void);
SLONG AENG_lock(void);
SLONG FONT_draw(SLONG x, SLONG y, CBYTE *text, ...);
void  AENG_unlock(void);


//
// Call once at the end of the whole program.
// 

void AENG_fini(void);


// ========================================================
//
// EDITOR SUPPORT FUNCTIONS
//
// ========================================================

//
// Returns the position of the ray going through the
// given screen pixel.  Return FALSE if no intersection
// happened!
//

SLONG AENG_raytraced_position(
		SLONG  sx,
		SLONG  sy,
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG  indoors=0);

//
// Returns the position of the point above (wx,wz) in
// the world whose screeny coordinate is the same as
// the given point.
//

void AENG_raytraced_y_position(
		SLONG  sy,
		SLONG  wx,
		SLONG  wz,
		SLONG *world_y);
		

//
// Draws a light. Returns bitfield to say if the given screen coord
// is over any part of the light.
//

#define AENG_MOUSE_OVER_LIGHT_BOT (1 << 0)
#define AENG_MOUSE_OVER_LIGHT_TOP (1 << 1)

ULONG AENG_light_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight);	// How much to expand the light ball by...


//
// Draws the editor mode of the sewer editor.
//

void AENG_draw_sewer_editor(
		SLONG  cam_x,
		SLONG  cam_y,
		SLONG  cam_z,
		SLONG  cam_yaw,
		SLONG  cam_pitch,
		SLONG  cam_roll,
		SLONG  mouse_x,
		SLONG  mouse_y,
		SLONG *mouse_over_valid,
		SLONG *mouse_over_x,
		SLONG *mouse_over_y,
		SLONG *mouse_over_z,
		SLONG  draw_prim_at_mouse,
		SLONG  prim_object,
		SLONG  prim_yaw);


//
// Draws text at the given point by calling FONT_buffer_add().
// So remember to call FONT_buffer_draw() before you flip the screen.
//

void AENG_world_text(
		SLONG  x,
		SLONG  y,
		SLONG  z,
		UBYTE  red,
		UBYTE  blue,
		UBYTE  green,
		UBYTE  shadowed_or_not,
		CBYTE *fmt, ...);



//---------------------------------------------------------------
//	GUY.
//---------------------------------------------------------------

#define AENG_MOUSE_OVER_WAYPOINT	(1 << 0)

ULONG AENG_waypoint_draw(
		SLONG mx,
		SLONG my,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight);

ULONG AENG_rad_trigger_draw(
		SLONG mx,
		SLONG my,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG rad,
		ULONG colour,
		UBYTE highlight);

//---------------------------------------------------------------
//	CANIS.
//---------------------------------------------------------------

void AENG_groundsquare_draw(
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE polyinit);

//---------------------------------------------------------------

//
// Drawing buffered up text anywhere on the screen.
//

#include "font.h"
#include "console.h"


//
// panel stuff
//
extern	void	PANEL_draw_health_bar(SLONG x,SLONG y,SLONG percentage);
extern	void	PANEL_draw_timer(SLONG time_in_hundreths, SLONG x, SLONG y);
extern	void	PANEL_draw_buffered(void);	// Actually draws the timers....
extern	void	PANEL_finish(void);

//
// detail level stuff
//

// read detail levels from disc
void AENG_read_detail_levels();


#ifdef TARGET_DC
// get the prevaling settings
void AENG_get_detail_levels(//int* stars, 
							int* shadows, 
							//int* moon_reflection, 
							//int* people_reflection, 
							int* puddles,
							int* dirt,
							int* mist,
							int* rain,
							int* skyline,
							//int* filter,
							//int* perspective,
							int* crinkles);

// change the prevaling settings
void AENG_set_detail_levels(//int stars,
							int shadows,
							//int moon_reflection,
							//int people_reflection,
							int puddles,
							int dirt,
							int mist,
							int rain,
							int skyline,
							//int filter,
							//int perspective,
							int crinkles);


#else
// get the prevaling settings
void AENG_get_detail_levels(int* stars, 
							int* shadows, 
							int* moon_reflection, 
							int* people_reflection, 
							int* puddles,
							int* dirt,
							int* mist,
							int* rain,
							int* skyline,
							int* filter,
							int* perspective,
							int* crinkles);

// change the prevaling settings
void AENG_set_detail_levels(int stars,
							int shadows,
							int moon_reflection,
							int people_reflection,
							int puddles,
							int dirt,
							int mist,
							int rain,
							int skyline,
							int filter,
							int perspective,
							int crinkles);

#endif

#endif
