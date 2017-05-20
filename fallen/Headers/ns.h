//
// A new super duper sewer/cavern system.
//

#ifndef _NS_
#define _NS_


#include "pap.h"


// ========================================================
//
// THE SEWER MAP
//
// ========================================================

//
// The high-res map. Set bot, water and packed. 'top' is calculated
// for you.
//

#define NS_HI_PACKED_TYPE (0x07)
#define NS_HI_PACKED_FLAG (0x38)
#define NS_HI_PACKED_DIR  (0xc0)

#define NS_HI_TYPE(nh)			((nh)->packed & NS_HI_PACKED_TYPE)
#define NS_HI_TYPE_SET(nh,t)	{(nh)->packed &= ~NS_HI_PACKED_TYPE; (nh)->packed |= (t);}

#define NS_HI_TYPE_ROCK		0
#define NS_HI_TYPE_SEWER	1
#define NS_HI_TYPE_STONE	2
#define NS_HI_TYPE_NOTHING	3
#define NS_HI_TYPE_CURVE	4	// Private!

#define NS_HI_FLAG_GRATE	(1 << 5)	// A hole through which water can pour.
#define NS_HI_FLAG_LOCKTOP	(1 << 6)	// Private!
#define NS_HI_FLAG_TOPUSED	(1 << 7)	// Private!

typedef struct
{
	//
	// In eighth map-squares: From 32 squares below ground...
	// 'bot' should be on map-square boundaries.
	// 'top' is precalculated by NS_precalculate(void)
	//

	UBYTE bot;		// bot == 0 => A hole into infinity...
	UBYTE top;

	UBYTE water;	// The height of the water or 0 if there is no water.
					// No water allowed on rock.

	UBYTE packed;	// 5:3 FLAGS : TYPE

} NS_Hi;

extern NS_Hi NS_hi[PAP_SIZE_HI][PAP_SIZE_HI];

//
// Each low-res mapsquare has its points and faces pre-calculated
// and cached using the HEAP module. In the sewers, the heap is
// used for the sewer memory not the lighting...
//

typedef struct
{
	UBYTE u[4];
	UBYTE v[4];

}  NS_Texture;

#define NS_MAX_TEXTURES 256

extern NS_Texture NS_texture[NS_MAX_TEXTURES];
extern SLONG      NS_texture_upto;

//
// The page of a face is an index into this array. It leads to different
// things on the PSX to the PC.
//

#ifdef PSX

//
// The offset into the sewer texture page of each type.
//

typedef struct
{
	UBYTE du;
	UBYTE dv;

} NS_Page;

#else

//
// The page number to use: the number of .TGA file.
//

typedef struct
{
	UWORD page;

} NS_Page;

#endif

#define NS_PAGE_ROCK	0
#define NS_PAGE_SEWER	1
#define NS_PAGE_STONE	2
#define NS_PAGE_SWALL	3
#define NS_PAGE_GRATE	4
#define NS_PAGE_NUMBER	5

extern NS_Page NS_page[NS_PAGE_NUMBER];

//
// Waterfalls...
//

typedef struct
{
	UBYTE x;
	UBYTE z;
	SBYTE dx;		// Vector from where the water is coming from.
	SBYTE dz;
	UBYTE top;
	UBYTE bot;
	UBYTE counter;
	UBYTE next;

} NS_Fall;

#define NS_MAX_FALLS 32

extern NS_Fall NS_fall[NS_MAX_FALLS];
extern UBYTE   NS_fall_free;

typedef struct
{
	UBYTE x;		// (x << 3, z << 3) relative to the lo-res mapsquare.
	UBYTE z;
	UBYTE y;		// In eighth map-squares from 32 squares below ground...
	UBYTE bright;	// No coloured lighting...

} NS_Point;

typedef struct
{
	UBYTE p[4];
	UBYTE page;		// Rock/stone/brick...
	UBYTE texture;

} NS_Face;

typedef struct
{
	UBYTE  next;
	UBYTE  used;
	UBYTE  map_x;
	UBYTE  map_z;
	UBYTE *memory;
	UWORD  num_points;
	UWORD  num_faces;	// Face memory starts immediately after point memory.
	UBYTE  fall;		// Any waterfalls that happen to be in this square.
	UBYTE  padding;

} NS_Cache;

#define NS_MAX_CACHES 128

extern NS_Cache NS_cache[NS_MAX_CACHES];
extern UBYTE    NS_cache_free;

//
// The things in the sewers.
//

#define NS_ST_TYPE_UNUSED	0
#define NS_ST_TYPE_PRIM		1
#define NS_ST_TYPE_LADDER	2
#define NS_ST_TYPE_BRIDGE	3
#define NS_ST_TYPE_PLATFORM	4

typedef struct
{
	UBYTE type;
	UBYTE next;

	union
	{
		struct
		{
			UBYTE prim;
			UBYTE yaw;
			UBYTE x;		// (x<<3,z<<3) relative to the lo-res mapsquare it is in
			UBYTE z;
			UBYTE y;

		} prim;

		struct
		{
			UBYTE x1;		// (x,z) are in hi-res mapsquare coordinates.
			UBYTE z1;
			UBYTE x2;
			UBYTE z2;
			UBYTE height;

		} ladder;
	};

} NS_St;

#define NS_MAX_STS 64

extern NS_St NS_st[NS_MAX_STS];
extern UBYTE NS_st_free;

//
// Each lo-res mapsquare.
//

typedef struct
{
	UBYTE cache;
	UBYTE st;			// Linked list of sewer things above this mapsquare.

	//
	// The position of the light for this lo-res mapsquare. If y == 0 then
	// there is no light on this mapsquare.
	//

	UBYTE light_x;		// (x << 3, z << 3) relative to the lo-res mapsquare.
	UBYTE light_z;		// 0 => No light.
	UBYTE light_y;		// In eighth map-squares from 32 squares below ground.

} NS_Lo;

extern NS_Lo NS_lo[PAP_SIZE_LO][PAP_SIZE_LO];




// ========================================================
//
// EDITOR FUNCTIONS
//
// ========================================================

//
// Creates a new sewer map.
//

void NS_init(void);

//
// Calculate the top heights of the rock
// 

void NS_precalculate(void);

//
// Add a ladder to the map.  All arguments are given in hi-res mapsquare
// coordinates. The (x,z)s are given in clockwise order.
//

void NS_add_ladder(SLONG x1, SLONG z1, SLONG x2, SLONG z2, SLONG height);

//
// Adds a prim to the map. It might not go exactly where you want! The
// coordinates are given in 8-bit fixed point.  (x,z) are hi-res fixed 8
// coords and y is in the sewer system y-coordinates, so world_y = (y << 5) + -32 * 0x100
//

void NS_add_prim(
		SLONG prim,
		SLONG yaw,
		SLONG x,
		SLONG y,
		SLONG z);

//
// When the sewer map is complete.
//

void NS_save(CBYTE *fname);
void NS_load(CBYTE *fname);



// ========================================================
//
// SEWER INFO.
//
// ========================================================

//
// Returns the height of the sewer (8-bits per mapsquare)
//
	
SLONG NS_calc_height_at(SLONG x, SLONG z);

//
// Returns either the height of the sewer or the height of the
// water above the sewer (if there is water at (x,z)).
//

SLONG NS_calc_splash_height_at(SLONG x, SLONG z);

//
// Makes the movement vector slide around the sewer system.
// It takes highest-precision movement in 16-bit fixed point!
//

void NS_slide_along(
		SLONG  x1, SLONG  y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  radius);	// radius is only fixed-point 8!


//
// Returns TRUE if the given point is inside.
//

SLONG NS_inside(SLONG x, SLONG y, SLONG z);



extern SLONG NS_los_fail_x;
extern SLONG NS_los_fail_y;
extern SLONG NS_los_fail_z;

SLONG NS_there_is_a_los(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2);



// ========================================================
//
// DRAWING THE SEWER MAP
//
// ========================================================

//
// Initialises the HEAP module and marks all lo-res mapsquares
// as uncached.  Builds the linked list of cache entries.
//

void NS_cache_init(void);

//
// Creates a cache entry for the given mapsquare.
// Destroy the given cache entry.
//

SLONG NS_cache_create (UBYTE mx, UBYTE mz);
void  NS_cache_destroy(UBYTE cache);


//
// Gets rid of all cached data and reinitialises the heap.
//

void NS_cache_fini(void);


#endif
