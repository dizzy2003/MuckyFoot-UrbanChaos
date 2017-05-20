//
// Draped cloth and flags.
//

#ifndef _CLOTH_
#define _CLOTH_


//
// Gets rid of all the cloths.
//

void CLOTH_init(void);


//
// Creates a new piece of cloth.  Returns NULL on failure.
//

#define CLOTH_TYPE_UNUSED 0
#define CLOTH_TYPE_FLAG	  1
#define CLOTH_TYPE_CLOAK  2

UBYTE CLOTH_create(
		UBYTE type,
		SLONG ox,
		SLONG oy,
		SLONG oz,
		SLONG wdx, SLONG wdy, SLONG wdz,
		SLONG hdx, SLONG hdy, SLONG hdz,
		SLONG dist,
		ULONG colour);

//
// All cloth is made up of CLOTH_WIDTH x CLOTH_HEIGHT points.
//

#define CLOTH_WIDTH	 6
#define CLOTH_HEIGHT 5


//
// Locks the given point. These are the points of the cloth that are
// attached to somewhere.
//

void CLOTH_point_lock(UBYTE cloth, UBYTE w, UBYTE h);

//
// Moves the given cloth point. Moving locked points is probably
// the best idea.
//

void CLOTH_point_move(UBYTE cloth, UBYTE w, UBYTE h, SLONG x, SLONG y, SLONG z);

//
// Processes all the cloth.
//

void CLOTH_process(void);


// ========================================================
//
// DRAWING THE CLOTH
//
// ========================================================

//
// Accessing the cloth on the mapwho.
//

UBYTE CLOTH_get_first(UBYTE lo_map_x, UBYTE lo_map_z);

//
// The index of cloth point (w,h)
//

#define CLOTH_INDEX(w,h) ((w) + (h) * CLOTH_WIDTH)

//
// Returns an array of CLOTH_WIDTH x CLOTH_HEIGHT points
// and normals.
//

typedef struct
{
	float x;
	float y;
	float z;
	float nx;	// (nx,ny,nz) is normalised
	float ny;
	float nz;
	 
} CLOTH_Drawp;

typedef struct
{
	UBYTE type;
	UBYTE next;		// The next cloth in the linked list hanging of the mapsquare.
	UWORD padding;
	ULONG colour;

	CLOTH_Drawp p[CLOTH_WIDTH * CLOTH_HEIGHT];

} CLOTH_Info;

CLOTH_Info *CLOTH_get_info(UBYTE cloth);


#endif
