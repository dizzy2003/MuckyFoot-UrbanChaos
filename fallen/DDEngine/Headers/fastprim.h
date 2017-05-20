//
// Draws prims super-fast!
//

#ifndef _FASTPRIM_
#define _FASTPRIM_

#include "night.h"


//
// Initialises memory.
//

void FASTPRIM_init(void);


//
// Draws a prim fastly. Return TRUE if it did it okay.
//

SLONG FASTPRIM_draw(
		SLONG prim,
		float x,
		float y,
		float z,
		float matrix[9],
		NIGHT_Colour *lpc);


//
// Frees up memory.
//

void FASTPRIM_fini(void);



#endif









