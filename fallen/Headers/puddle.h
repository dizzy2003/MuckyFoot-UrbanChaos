//
// Puddles!
//

#ifndef _PUDDLE_
#define _PUDDLE_


#ifndef TARGET_DC

//
// Clears out all puddle info.
//

void PUDDLE_init(void);

//
// Creates a puddle centered at (x,y,z) of the given radius and angle.
//

void PUDDLE_create(
		UWORD x,
		SWORD y,
		UWORD z);

//
// Puts down puddle on the edges of buildings and curbs.
//

void PUDDLE_precalculate(void);


//
// Looks for a puddle under (x,y,z) and splashes it if it find one.
// (x>>8,z>>8) are mapsquare (UBYTE) coordinates.
//

void PUDDLE_splash(
		SLONG x,
		SLONG y,
		SLONG z);

//
// Returns TRUE if the given point is in a puddle.
//

SLONG PUDDLE_in(
		SLONG x,
		SLONG z);

//
// Calms down splashes.
//

void PUDDLE_process(void);

//
// Returns all the puddles on the given z mapsquare line with an 'x'
// in the given range.
//

typedef struct
{
	SLONG x1;
	SLONG z1;
	SLONG u1;		// 0 to 256
	SLONG v1;		// 0 to 256

	SLONG x2;
	SLONG z2;
	SLONG u2;
	SLONG v2;

	SLONG y;

	UBYTE puddle_y1;
	UBYTE puddle_y2;
	UBYTE puddle_g1;
	UBYTE puddle_g2;
	UBYTE puddle_s1;
	UBYTE puddle_s2;
	UWORD rotate_uvs;	// Rotate the uvs relative to the xzs
	
} PUDDLE_Info;

void         PUDDLE_get_start(UBYTE z_map, UBYTE x_map_min, UBYTE x_map_max);
PUDDLE_Info *PUDDLE_get_next(void);


#endif //#ifndef TARGET_DC


#endif
