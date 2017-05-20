//
// Cached lighting.
//

#ifndef _NIGHT_
#define _NIGHT_


#include "pap.h"

#define NIGHT_MAX_SLIGHTS 256


#ifdef TARGET_DC

// My DC monitor is seriously dark, so to be able to see anything (i.e. does the texturing work?)
// I turn the brightness of the game up. But on a sensible monitor/TV it looks awful.
#ifdef DEBUG
#define NIGHT_LIGHT_MULTIPLIER 2.0f
#else
#define NIGHT_LIGHT_MULTIPLIER 1.0f
#endif

#else //#ifdef TARGET_DC

#define NIGHT_LIGHT_MULTIPLIER 1.0f

#endif //#else //#ifdef TARGET_DC


//
// The static lights.
// 

typedef struct
{
	SWORD y;
	UBYTE x; // I'm grabbing myself the top bit
	UBYTE z; //  of these for flags of some sort (inside or something)
	SBYTE red;
	SBYTE green;
	SBYTE blue;
	UBYTE radius;

} NIGHT_Slight;

extern	NIGHT_Slight *NIGHT_slight;//[NIGHT_MAX_SLIGHTS];
extern	SLONG        NIGHT_slight_upto;

typedef struct
{
	UBYTE index;
	UBYTE number;

} NIGHT_Smap;

typedef	NIGHT_Smap	NIGHT_Smap_2d[PAP_SIZE_LO];

extern	NIGHT_Smap_2d *NIGHT_smap; //[PAP_SIZE_LO][PAP_SIZE_LO];

//
// The dynamic lights.
//

#define NIGHT_DLIGHT_FLAG_USED   (1 << 0)
#define NIGHT_DLIGHT_FLAG_REMOVE (1 << 1)	// Will be removed next gameturn.

typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;
	UBYTE red;
	UBYTE green;
	UBYTE blue;
	UBYTE radius;
	UBYTE next;
	UBYTE flag;

} NIGHT_Dlight;

#define NIGHT_MAX_DLIGHTS 64

extern	NIGHT_Dlight *NIGHT_dlight; //[NIGHT_MAX_DLIGHTS];



//
// Coloured lighting.
//

typedef struct
{
#ifdef	PSX_COMPRESS_LIGHT
	UWORD	Col;
#else
	UBYTE red;
	UBYTE green;
	UBYTE blue;
#endif

} NIGHT_Colour;



#define	get_red(col)	(((col)>>10)&0x3f)
#define	get_green(col)	(((col)>>5)&0x1f)
#define	get_blue(col)	(((col))&0x1f)

#define	set_red(col,red)	col=(red)<<10
#define	set_green(col,red)	col=(red)<<5
#define	set_blue(col,red)	col=(red)

#define	or_red(col,red)		col|=(red)<<10
#define	or_green(col,red)	col|=(red)<<5
#define	or_blue(col,red)	col|=(red)

#define	nor_red(col,red)	col=(col&(0x3f<<10))|((red)<<10)
#define	nor_green(col,red)	col=(col&(0x1f<<5))|((red)<<5)
#define	nor_blue(col,red)	col=(col&(0x1f<<0))|((red)<<0)

//
// The cached lighting lights all the points and all the prims
// in each lo-res mapsquare.  The first 16 colour entries are
// the colours of the 4x4 points in this mapsquare.  After this
// there are the colours of all the points of the prims as returned
// by the OB module.
//

#define NIGHT_SQUARE_FLAG_USED		(1 << 0)
#define NIGHT_SQUARE_FLAG_WARE		(1 << 1)
#define NIGHT_SQUARE_FLAG_DELETEME	(1 << 2)

typedef struct
{
	UBYTE         next;
	UBYTE         flag;
	UBYTE         lo_map_x;
	UBYTE         lo_map_z;
	NIGHT_Colour *colour;
	ULONG		  sizeof_colour;	// In bytes.

} NIGHT_Square;

#define NIGHT_MAX_SQUARES 256

extern NIGHT_Square NIGHT_square[NIGHT_MAX_SQUARES];
extern UBYTE        NIGHT_square_free;

//
// The cached lighting mapwho. Contains indices into the
// NIGHT_square array, or NULL if there is no cached lighting
// for that square.
//

extern UBYTE NIGHT_cache[PAP_SIZE_LO][PAP_SIZE_LO];

#ifdef	PSX
extern	UWORD	floor_psx_col[PAP_SIZE_HI][PAP_SIZE_HI];
#endif

//
// The cached lighting for dfacets.
//

typedef struct
{
	UBYTE         next;
	UBYTE         counter;
	UWORD         dfacet;
	NIGHT_Colour *colour;
	UWORD         sizeof_colour;

} NIGHT_Dfcache;

#define NIGHT_MAX_DFCACHES 256

extern NIGHT_Dfcache NIGHT_dfcache[NIGHT_MAX_DFCACHES];
extern UBYTE         NIGHT_dfcache_free;
extern UBYTE		 NIGHT_dfcache_used;

// ========================================================
//
// NEW LIGHTING FUNCTIONS
//
// ========================================================

//
// Initialises all the lighting: Removes all static and dynamic lights,
// and clears all cached info.
//

void NIGHT_init(void);

//
// Converts a colour to its D3D equivalents.
//

#define NIGHT_MAX_BRIGHT 64

//
// Make bright light create specular.
//

#ifdef TARGET_DC
static const SLONG NIGHT_specular_enable = 0;
#else
extern SLONG NIGHT_specular_enable;
#endif

inline void NIGHT_get_d3d_colour(NIGHT_Colour col, ULONG *colour, ULONG *specular)
{
	SLONG red   = col.red;
	SLONG green = col.green;
	SLONG blue  = col.blue;

	red   *= (256 / NIGHT_MAX_BRIGHT);
	green *= (256 / NIGHT_MAX_BRIGHT);
	blue  *= (256 / NIGHT_MAX_BRIGHT);

	if (NIGHT_specular_enable)
	{
		SLONG wred   = 0;
		SLONG wgreen = 0;
		SLONG wblue  = 0;

		if (red   > 255) {wred   = (red   - 255) >> 1; red   = 255; if (wred   > 255) {wred   = 255;}}
		if (green > 255) {wgreen = (green - 255) >> 1; green = 255; if (wgreen > 255) {wgreen = 255;}}
		if (blue  > 255) {wblue  = (blue  - 255) >> 1; blue  = 255; if (wblue  > 255) {wblue  = 255;}}

	   *colour    = (red  << 16) | (green  << 8) | (blue  << 0);
	   *specular  = (wred << 16) | (wgreen << 8) | (wblue << 0);
	   *specular |= 0xff000000;		// No fog by default...
	}
	else
	{
		if (red   > 255) {red   = 255;}
		if (green > 255) {green = 255;}
		if (blue  > 255) {blue  = 255;}

	   *colour   = (red << 16) | (green << 8) | (blue << 0);
	   *specular = 0xff000000;		// No fog by default...
	}
	*colour |= 0xff000000;		// No fog by default...
}

//
// z in range 0->1.0
//

#ifndef	POLY_FADEOUT_START
//
// sneak it in here
//
#define POLY_FADEOUT_START	(0.60F)
#define POLY_FADEOUT_END	(0.95F)
#endif

inline void NIGHT_get_d3d_colour_and_fade(NIGHT_Colour col, ULONG *colour, ULONG *specular,float z)
{
	SLONG red   = col.red;
	SLONG green = col.green;
	SLONG blue  = col.blue;
	SLONG multi = 255 - (SLONG)((z - POLY_FADEOUT_START) * (256.0F / (POLY_FADEOUT_END - POLY_FADEOUT_START)));

//red   *= (256 / NIGHT_MAX_BRIGHT);
//green *= (256 / NIGHT_MAX_BRIGHT);
//blue  *= (256 / NIGHT_MAX_BRIGHT);

	if(multi>255)
	{
		multi=255;
	}

	if (multi < 0)
	{
		multi = 0;
	}
	multi*=4; //replaces above

	red		= red	*multi>>8;  // fade out on distance
	green	= green	*multi>>8; 
	blue	= blue	*multi>>8;


/* //dead I believe
	if (NIGHT_specular_enable)
	{
		SLONG wred   = 0;
		SLONG wgreen = 0;
		SLONG wblue  = 0;

		if (red   > 255) {wred   = (red   - 255) >> 1; red   = 255; if (wred   > 255) {wred   = 255;}}
		if (green > 255) {wgreen = (green - 255) >> 1; green = 255; if (wgreen > 255) {wgreen = 255;}}
		if (blue  > 255) {wblue  = (blue  - 255) >> 1; blue  = 255; if (wblue  > 255) {wblue  = 255;}}

	   *colour    = (red  << 16) | (green  << 8) | (blue  << 0);
	   *specular  = (wred << 16) | (wgreen << 8) | (wblue << 0);
	   *specular |= 0xff000000;		// No fog by default...
	}
	else
*/
	{
		if (red   > 255) {red   = 255;}
		if (green > 255) {green = 255;}
		if (blue  > 255) {blue  = 255;}

	   *colour   = (red << 16) | (green << 8) | (blue << 0);
	   *specular = 0xff000000;		// No fog by default...
	}
	*colour |= 0xff000000;		// No fog by default...
}

inline void NIGHT_get_d3d_colour_dim(NIGHT_Colour col, ULONG *colour, ULONG *specular)
{
	SLONG red   = col.red;
	SLONG green = col.green;
	SLONG blue  = col.blue;

	red   *= (256 / NIGHT_MAX_BRIGHT);
	green *= (256 / NIGHT_MAX_BRIGHT);
	blue  *= (256 / NIGHT_MAX_BRIGHT);

	red>>=1;
	green>>=1;
	blue>>=1;

	if (NIGHT_specular_enable)
	{
		SLONG wred   = 0;
		SLONG wgreen = 0;
		SLONG wblue  = 0;

		if (red   > 255) {wred   = (red   - 255) >> 1; red   = 255; if (wred   > 255) {wred   = 255;}}
		if (green > 255) {wgreen = (green - 255) >> 1; green = 255; if (wgreen > 255) {wgreen = 255;}}
		if (blue  > 255) {wblue  = (blue  - 255) >> 1; blue  = 255; if (wblue  > 255) {wblue  = 255;}}

	   *colour    = (red  << 16) | (green  << 8) | (blue  << 0);
	   *specular  = (wred << 16) | (wgreen << 8) | (wblue << 0);
	   *specular |= 0xff000000;		// No fog by default...
	}
	else
	{
		if (red   > 255) {red   = 255;}
		if (green > 255) {green = 255;}
		if (blue  > 255) {blue  = 255;}

	   *colour   = (red << 16) | (green << 8) | (blue << 0);
	   *specular = 0xff000000;		// No fog by default...
	}
	*colour |= 0xff000000;		// No fog by default...
}

//
// Returns the amount of light at the given spot.
//

NIGHT_Colour NIGHT_get_light_at(
				SLONG x,
				SLONG y,
				SLONG z);


#ifndef PSX

//
// Fills the array with all the lights that effect the given point.
// (not including the ambient light)
//

typedef struct
{
	SLONG r;	// (r,g,b) falling on the point.
	SLONG g;
	SLONG b;
	SLONG dx;	// Normalised vector from the point to the light (256 long)
	SLONG dy;
	SLONG dz;

} NIGHT_Found;

#define NIGHT_MAX_FOUND 4

extern NIGHT_Found NIGHT_found[NIGHT_MAX_FOUND];
extern SLONG       NIGHT_found_upto;

void NIGHT_find(SLONG x, SLONG y, SLONG z);

#endif


//
// Initialises the heap and gets rid of all cached lighting.
//

void NIGHT_destroy_all_cached_info(void);

//
// Lighting flags and variables.
//

#define NIGHT_FLAG_LIGHTS_UNDER_LAMPOSTS	(1 << 0)
#define NIGHT_FLAG_DARKEN_BUILDING_POINTS	(1 << 1)
#define NIGHT_FLAG_DAYTIME					(1 << 2)

extern ULONG        NIGHT_flag;
extern NIGHT_Colour NIGHT_sky_colour;
extern UBYTE        NIGHT_lampost_radius;
extern SBYTE        NIGHT_lampost_red;
extern SBYTE        NIGHT_lampost_green;
extern SBYTE        NIGHT_lampost_blue;

//
// Loads a lighting file saved from the ED module.  Return TRUE
// on success.
//

extern SLONG NIGHT_load_ed_file(CBYTE *name);


// ========================================================
//
// AMBIENT LIGHT
//
// ========================================================

extern ULONG NIGHT_amb_d3d_colour;
extern ULONG NIGHT_amb_d3d_specular;
extern SLONG NIGHT_amb_red;
extern SLONG NIGHT_amb_green;
extern SLONG NIGHT_amb_blue;
extern SLONG NIGHT_amb_norm_x;
extern SLONG NIGHT_amb_norm_y;
extern SLONG NIGHT_amb_norm_z;

//
// The normal should be normalised to 256.
//

void NIGHT_ambient(
		UBYTE red,
		UBYTE green,
		UBYTE blue,
		SLONG norm_x,
		SLONG norm_y,
		SLONG norm_z);

//
// Returns the amount of light captured by a point with the
// given normal. (256 long)
// 

NIGHT_Colour NIGHT_ambient_at_point(
				SLONG norm_x,
				SLONG norm_y,
				SLONG norm_z);

// ========================================================
//
// STATIC LIGHTS
//
// ========================================================

SLONG NIGHT_slight_create(		// Returns FALSE on failure.
		SLONG x,
		SLONG y,
		SLONG z,
		UBYTE radius,
		SBYTE red,
		SBYTE green,
		SBYTE blue);

void  NIGHT_slight_delete(
		SLONG x,
		SLONG y,
		SLONG z,
		UBYTE radius,
		SBYTE red,
		SBYTE green,
		SBYTE blue);

void  NIGHT_slight_delete_all(void);


// ========================================================
// 
// DYNAMIC LIGHTS
//
// ========================================================

UBYTE NIGHT_dlight_create(
		SLONG x,
		SLONG y,
		SLONG z,
		UBYTE radius,
		UBYTE red,
		UBYTE green,
		UBYTE blue);

void NIGHT_dlight_destroy(UBYTE dlight_index);
void NIGHT_dlight_move   (UBYTE dlight_index, SLONG x, SLONG y, SLONG z);
void NIGHT_dlight_colour (UBYTE dlight_index, UBYTE red, UBYTE green, UBYTE blue);

//
// Lights all the used cached lighting info from the dynamic lights.
// Undoes all the dynamic lighting.
//

void NIGHT_dlight_squares_up  (void);
void NIGHT_dlight_squares_down(void);


// ========================================================
//
// CACHED LIGHTING OF THE MAP AND PRIMS
//
// ========================================================

//
// Recalculates all the cache entries.
// Creates a mapwho entry in the given lo-res mapwho square.
// Frees up all the memory.
//

void NIGHT_cache_recalc (void);
void NIGHT_cache_create (UBYTE lo_map_x, UBYTE lo_map_z, UBYTE inside_warehouse = FALSE);
void NIGHT_cache_create_inside(UBYTE lo_map_x, UBYTE lo_map_z,SLONG floor_y);
void NIGHT_cache_destroy(UBYTE square_index);


// ========================================================
//
// CACHED LIGHTING OF DFACETS ON THE SUPERMAP
//
// ========================================================

void  NIGHT_dfcache_recalc (void);
UBYTE NIGHT_dfcache_create (UWORD dfacet_index);
void  NIGHT_dfcache_destroy(UBYTE dfcache_index);


// ========================================================
//
// STATIC LIGHTING OF THE WALKABLE FACES' POINTS
//
// ========================================================

//
// The first few thousand prim points dont belong to the walkable faces.
// This is the index of the first walkable prim point.  
// Walkable primpoint x is index as NIGHT_walkable[x - NIGHT_first_walkable_point]...
//

#define NIGHT_MAX_WALKABLE 15000

extern SLONG NIGHT_first_walkable_prim_point;
extern NIGHT_Colour NIGHT_walkable[NIGHT_MAX_WALKABLE];
extern NIGHT_Colour NIGHT_roof_walkable[];

#define NIGHT_ROOF_WALKABLE_POINT(f,p) (NIGHT_roof_walkable[f*4+p])

#ifndef NDEBUG
SLONG NIGHT_check_index(SLONG walkable_prim_point_index);
#define NIGHT_WALKABLE_POINT(p) (NIGHT_check_index(p), NIGHT_walkable[p - NIGHT_first_walkable_prim_point])
#else

//
// Returns the colour of the given walkable prim_point.
//

#define NIGHT_WALKABLE_POINT(p) (NIGHT_walkable[(p) - NIGHT_first_walkable_prim_point])

#endif

//
// Generates the walkable info for the current map.
//

void NIGHT_generate_walkable_lighting(void);


#endif







