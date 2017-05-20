//
// Handles the allocation of people textures on n:\
//

#ifndef _PERSTEX_
#define _PERSTEX_



// ========================================================

//
// Returns the page number of the given texture file. The texture file should
// be found in n:\urbanchaos\textures\shared\PersTex.  If the texture file has not yet
// been allocated a page number, it gives that file the next spare page number and
// puts a copy of the texture in n:\urbanchaos\textures\shared\people
//

//
// Returned if it couldn't find the .tga in the n:\urbanchaos\textures\prims\MaxTex
// directory.
//

#define PERSTEX_PAGENUMBER_QMARK 0

SLONG PERSTEX_get_number(CBYTE *fname);

// ========================================================






#endif
