//
// Converts facets to draw indexed primitive calls...
//

#ifndef _SUPERFACET_
#define _SUPERFACET_



//
// Call at the start of the game_loop()- after everything has
// been loaded. Sets up memory.
//

void SUPERFACET_init(void);


//
// Sets up the frame for drawing SUPERFACET
//

void SUPERFACET_start_frame(void);



//
// Draws a super-fast facet if it can, otherwise returns FALSE!
//

SLONG SUPERFACET_draw(SLONG facet);



//
// Call at the end of the game_loop()- frees up memory.
//

void SUPERFACET_fini(void);




#endif
