//
// Facet drawing functions.
//

#ifndef _FACET_
#define _FACET_



//
// Draws the given facet.
//

void FACET_draw(SLONG facet,UBYTE alpha);


//
// Draws all the walkable faces for the given building.
//

void FACET_draw_walkable(SLONG building);



//
// Projects a shadow onto the crinkled facet. When it has made a
// properly crinkled polygon, it calls AENG_add_projected_shadow_poly().
//
// Only call this function on NORMAL facets!
//

void FACET_project_crinkled_shadow(SLONG facet);




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
