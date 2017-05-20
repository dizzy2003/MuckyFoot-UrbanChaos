//
// Movie compression routines.
//

#ifndef _COMP_
#define _COMP_


#include "tga.h"


//
// The dimensions of the compressed frames.
// 

#define COMP_SIZE  64
#define COMP_SNUM  8
#define COMP_SSIZE (COMP_SIZE / COMP_SNUM)


//
// Our types.
//

typedef struct
{
	TGA_Pixel p[COMP_SIZE][COMP_SIZE];	// Access (y,x)!

} COMP_Frame;

typedef struct
{
	SLONG size;
	UBYTE data[];
 
} COMP_Delta;


//
// Loads a frame in from a .TGA file. Returns TRUE on success.
//

#define COMP_TGA_MAX_WIDTH  640
#define COMP_TGA_MAX_HEIGHT	480

SLONG COMP_load(CBYTE *filename, COMP_Frame *cf);

//
// Calculates a structure that maps one frame onto the next.
// It returns the COMP_Delta in the same block of memory each time!
// i.e. subsequent calls overwrite previous answers.  Returns the
// compressed frame.
// 

COMP_Delta *COMP_calc(COMP_Frame *f1, COMP_Frame *f2, COMP_Frame *ans);

//
// Calculates one frame from the next, using a delta calculated with
// COMP_calc().
//

void COMP_decomp(
		COMP_Frame *base,
		COMP_Delta *delta,
		COMP_Frame *result);


#endif
