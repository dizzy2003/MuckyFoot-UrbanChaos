//
// Handles the allocation of prims and prim textures on n:\
//

#ifndef _PRIMTEX_
#define _PRIMTEX_


// ========================================================

//
// Returns the page number of the given texture file. The texture file should
// be found in n:\urbanchaos\textures\shared\MaxTex.  If the texture file has not yet
// been allocated a page number, it gives that file the next spare page number and
// puts a copy of the texture in n:\urbanchaos\textures\shared\prims
//

//
// Returned if it couldn't find the .tga in the n:\urbanchaos\textures\prims\MaxTex
// directory.
//

#define PRIMTEX_PAGENUMBER_QMARK 0

SLONG PRIMTEX_get_number(CBYTE *fname);

// ========================================================



#endif
