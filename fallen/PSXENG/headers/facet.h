//
// Facet drawing functions.
//

#ifndef _FACET_
#define _FACET_



//
// Draws the given facet.
//

void FACET_draw(SLONG facet,UBYTE fade_alpha);


//
// Draws all the walkable faces for the given building.
//

void FACET_draw_walkable(SLONG building);
void FACET_draw_ware_walkable(SLONG build);


//
// NOT USED ANY MORE!
//

//
// Draws a sewer ladder. All coordinates and the height are
// in hi-res mapsquares
//

void FACET_draw_ns_ladder(
		SLONG x1,
		SLONG z1,
		SLONG x2,
		SLONG z2,
		SLONG height);



#endif
