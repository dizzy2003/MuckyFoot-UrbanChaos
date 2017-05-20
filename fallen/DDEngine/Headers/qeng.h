//
// An engine for the qmap.
//

#ifndef _QENG_
#define _QENG_


#include "c:\fallen\headers\qmap.h"


//
// Once at the start of the whole program.
//

void QENG_init(void);


//
// Debug messages drawn to the screen.
//

void MSG_add(CBYTE *message, ...);


//
// Where everything is drawn from.
//

void QENG_set_camera(
		float world_x,
		float world_y,
		float world_z,
		float yaw,
		float pitch,
		float roll);

//
// Clears the screen.
//

void QENG_clear_screen(void);


//
// Draws a line in the world. Sets QENG_mouse_over and QENG_mouse_pos if the
// mouse is over the line.
//

extern SLONG QENG_mouse_over;
extern float QENG_mouse_pos_x;	// Position in the world
extern float QENG_mouse_pos_y;
extern float QENG_mouse_pos_z;

void QENG_world_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		bool sort_to_front);

//
// Draws a QMAP_Draw structure.
//

void QENG_draw(QMAP_Draw *qd);

//
// Draws everything.
//

void QENG_render(void);


//
// Flips in the backbuffer.
//

void QENG_flip(void);


//
// Once at the end of the whole program.
//

void QENG_fini(void);


#endif
