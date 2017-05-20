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

void ANIM_obj_draw(Thing *p_thing,DrawTween *dt);


//
// Draws a reflection of the person about the plane, y = height.
//

void FIGURE_draw_reflection(Thing *person, SLONG height);


#endif