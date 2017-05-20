//
// Drawing rotating prims.
//

#ifndef MESH_H
#define MESH_H


#include "c:\fallen\headers\light.h"


//
// Draws a mesh using the POLY module.
//
extern	void CHOPPER_draw_chopper(Thing *p_chopper);


SLONG MESH_draw_poly(
		SLONG         prim,
		MAPCO16	      at_x,
		MAPCO16       at_y,
		MAPCO16	      at_z,
		SLONG         i_yaw,
		SLONG         i_pitch,
		SLONG         i_roll,
		LIGHT_Colour *lpc,
		UBYTE fade);
/*
void MESH_draw_poly(
		SLONG         prim,
		MAPCO16	      at_x,
		MAPCO16       at_y,
		MAPCO16	      at_z,
		SLONG         i_yaw,
		SLONG         i_pitch,
		SLONG         i_roll
		);
*/


#endif


