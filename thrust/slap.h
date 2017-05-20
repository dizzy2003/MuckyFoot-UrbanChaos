//
// Creates a shadowed bitmap for a light map, given the silhoutte.
// Credit to Eddie Edwards for the outline rendering idea.
//

#ifndef _SLAP_
#define _SLAP_




//
// Initialises the SLAPPER with a new bitmap. It does not clear the bitmap!
//

#define SLAP_MAX_BITMAP_SIZE 256

void SLAP_init(
		UBYTE *bitmap,
		SLONG  bitmap_size);	// Power of 2 max of SLAP_MAX_BITMAP_SIZE


//
// Adds an outline edge given in 8-bit fixed point.
//

void SLAP_add_edge(
		SLONG x1, SLONG y1,
		SLONG x2, SLONG y2);


//
// Renders the bitmap. The image is clipped to the bitmap.
//

void SLAP_render(void);




#endif
