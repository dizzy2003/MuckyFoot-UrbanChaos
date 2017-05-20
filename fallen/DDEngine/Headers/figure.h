//
// Draws a person.
//

#ifndef _FIGURE_
#define _FIGURE_


//
// This function uses the POLY module, and assumes
// that all the camera stuff has already been set up.
//

void FIGURE_draw(Thing *person);

extern SLONG	FIGURE_alpha;


//
// Draws a reflection of the person about the plane, y = height.  It fills in
// the bounding box of the object on screen.  The bounding box isn't necessarily
// on screen!
//

extern SLONG FIGURE_reflect_x1;
extern SLONG FIGURE_reflect_y1;
extern SLONG FIGURE_reflect_x2;
extern SLONG FIGURE_reflect_y2;

void FIGURE_draw_reflection(Thing *person, SLONG height);


//
// What's this doing in figure.h?
// Don't ask me. I was just told to dump it here...
//

void    init_flames();


//
// Draws an animating prim.
//

void ANIM_obj_draw(Thing *p_thing, DrawTween *dt);




#endif