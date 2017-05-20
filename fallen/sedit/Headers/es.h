//
// The sewer editor.
//

#ifndef _ES_
#define _ES_


#include "pap.h"


//
// The map we edit, load and save.
//

#define ES_TYPE_ROCK	0
#define ES_TYPE_SEWER	1
#define ES_TYPE_GROUND	2
#define ES_TYPE_HOLE	3

#define ES_FLAG_GRATING		(1 << 0)
#define ES_FLAG_ENTRANCE	(1 << 1)
#define ES_FLAG_NOCURBS		(1 << 2)

typedef struct
{
	UBYTE type;
	UBYTE height;
	UBYTE flag;
	UBYTE water;	// 0 => no water.

} ES_Hi;

extern ES_Hi ES_hi[PAP_SIZE_HI][PAP_SIZE_HI];

//
// Water in the city.
//

extern UBYTE ES_city_water_on   [PAP_SIZE_HI][PAP_SIZE_HI];
extern SBYTE ES_city_water_level[PAP_SIZE_LO][PAP_SIZE_LO];


//
// The things.
//

#define ES_THING_TYPE_UNUSED 0
#define ES_THING_TYPE_LADDER 1
#define ES_THING_TYPE_PRIM	 2

typedef struct
{
	UBYTE type;
	UBYTE padding;
	UBYTE prim;
	UBYTE height;
	UBYTE x1;
	UBYTE z1;
	UBYTE x2;
	UBYTE z2;
	UWORD yaw;
	SWORD x;
	SWORD y;
	SWORD z;

} ES_Thing;

#define ES_MAX_THINGS 512

extern ES_Thing ES_thing[ES_MAX_THINGS];

//
// The lo-res mapwho.
//

typedef struct
{
	//
	// Inside the square.
	//

	UBYTE light_on;	// TRUE or FALSE
	UBYTE light_x;
	UBYTE light_y;
	UBYTE light_z;

} ES_Lo;

extern ES_Lo ES_lo[PAP_SIZE_LO][PAP_SIZE_LO];


//
// Initialises the ES module.
//

void ES_init(void);


//
// Draws the game editor.
//

void ES_draw_editor(
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
// Raises the height of the given square- all adjacent sewer squares
// are set to the same height.
//

void ES_change_height(
		SLONG map_x,
		SLONG map_z,
		SLONG dheight);


//
// Build the game NS sewers from the editor description.
//

void ES_build_sewers(void);


// ========================================================
//
// LADDER FUNCTIONS
//
// ========================================================

//
// Creates a ladder made by dragging a point from a to b.
//

void ES_ladder_create(
		SLONG ax,
		SLONG az,
		SLONG bx,
		SLONG bz);

//
// Changes the height of a ladder near the given world position.
//

void ES_ladder_dheight(
		SLONG x,
		SLONG z,
		SLONG dheight);

//
// Deletes a ladder near the given world position.
//

void ES_ladder_delete(
		SLONG x,
		SLONG z);


// ========================================================
//
// LIGHT FUNCTIONS
//
// ========================================================

//
// Validates the light in the implied lo-res mapsquare and
// moves it to the given world (x,z) location.
// 

void ES_light_move(SLONG x, SLONG z);

//
// Changes the height of the light inside the lo-res mapsquare
// given by the world (x,z) position.
//

void ES_light_dheight(SLONG x, SLONG z, SLONG dheight);

//
// Deletes the light inside the lo-res mapsquare
// given by the world (x,z) position.
//

void ES_light_delete(SLONG x, SLONG z);

// ========================================================
//
// WATER FUNCTIONS
//
// ========================================================

//
// Changes the height of the sewer water at world coordinate (x,z)
//

void ES_sewer_water_dheight(SLONG x, SLONG z, SLONG dheight);

//
// Set/get the city water 'on' status of the city water at (x,z)
//

SLONG ES_city_water_get(SLONG x, SLONG z);
void  ES_city_water_set(SLONG x, SLONG z, SLONG on_or_not);

//
// Changes the level of the water at the city world coordinate (x,z).
//

void ES_city_water_dlevel(SLONG x, SLONG z, SLONG dlevel);


// ========================================================
//
// PRIM FUNCTIONS
//
// ========================================================

//
// Creates a new prim.
//

void ES_prim_create(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw);

//
// Deletes a prim near the given coordinate.
//

void ES_prim_delete(
		SLONG x,
		SLONG y,
		SLONG z);

//
// Changes the height of the prim near (x,y,z)
//

void ES_prim_dheight(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dheight);

// ========================================================
//
// UNDO / REDO FUNCTIONS.
//
// ========================================================

void ES_undo_store(void);

void ES_undo_undo(void);
void ES_undo_redo(void);

SLONG ES_undo_undo_valid(void);
SLONG ES_undo_redo_valid(void);


//
// Loading / saving the sewer editor. Returns TRUE on success.
//

SLONG ES_load(CBYTE *filename);
SLONG ES_save(CBYTE *filename);



#endif
