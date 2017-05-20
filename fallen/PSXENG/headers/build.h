//
// Draws buildings.
//

#ifndef _BUILD_
#define _BUILD_


//
// This function uses the POLY module, and assumes
// that all the camera stuff has already been set up.
//

void BUILD_draw(Thing *building);

//
// bodge for now
//
void BUILD_draw_facet(Thing *p_thing,UWORD facet);

//
// Draws a building you are inside.
//

void BUILD_draw_inside(void);



#endif