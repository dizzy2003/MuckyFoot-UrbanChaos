//
// Functions for helping with the outline of shapes on grids.
//

#ifndef _OUTLINE_
#define _OUTLINE_


typedef struct outline_outline OUTLINE_Outline;


//
// Creates a new outline.
//

OUTLINE_Outline *OUTLINE_create(SLONG num_z_squares);

//
// Adds the given line to the outline.  Must be orthogonal. The shape
// can be concave or convex.  It must link up completely before it is used
// by OUTLINE_overlap().
//

void OUTLINE_add_line(
		OUTLINE_Outline *oo,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);

//
// Frees up all the memory used in the given outline.
//

void OUTLINE_free(OUTLINE_Outline *oo);


//
// Returns TRUE if the two outlines overlap.
//

SLONG OUTLINE_overlap(
		OUTLINE_Outline *oo1,
		OUTLINE_Outline *oo2);


//
// Returns TRUE if the given line goes through an outline.
//

SLONG OUTLINE_intersects(
		OUTLINE_Outline *oo,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);




#endif
