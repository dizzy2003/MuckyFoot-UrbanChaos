//
// Drawing rotating prims.
//

#ifndef MESH_H
#define MESH_H


#include "night.h"


//
// If a prim face has its FACE_FLAG_TINTED flag set, then
// the colours the face is drawn with is ANDED with this
// value
// 

extern ULONG MESH_colour_and;

//
// Call once at the start of the whole game. Calculates the
// crumple look-up tables.
//

void MESH_init(void);


//
// Draws a mesh using the POLY module.  If 'lpc' is NULL then, the ambient
// light colour is used.  Returns the address of after the end of the lpc array.
//

NIGHT_Colour *MESH_draw_poly(
				SLONG         prim,
				SLONG	      at_x,
				SLONG         at_y,
				SLONG	      at_z,
				SLONG         i_yaw,
				SLONG         i_pitch,
				SLONG         i_roll,
				NIGHT_Colour *lpc,
				UBYTE         fade,
				SLONG         crumple = 0);

//
// Sets car crumple parameters before a call to MESH_draw_poly(..., -1)
//

void MESH_set_crumple(UBYTE* assignments, UBYTE* crumples);

//
// Draws an environment map over this given prim.
//

void MESH_draw_envmap(
		SLONG prim,
		SLONG at_x,
		SLONG at_y,
		SLONG at_z,
		SLONG i_yaw,
		SLONG i_pitch,
		SLONG i_roll);


//
// This version uses a flipped matrix compares to MESH_draw_poly
// It's useful for helicopters. Trust me. I'm a programmer.
//

NIGHT_Colour *MESH_draw_poly_inv_matrix(
				SLONG         prim,
				SLONG	      at_x,
				SLONG       at_y,
				SLONG	      at_z,
				SLONG         i_yaw,
				SLONG         i_pitch,
				SLONG         i_roll,
				NIGHT_Colour *lpc);

//
// The relfections are calculated once and then reused. The init() function clears
// all old reflection data.
//

void MESH_init_reflections(void);

//
// Draws the reflection of the given object. The reflection is always
// at the bottom of the prim.
//

void MESH_draw_reflection(
		SLONG         prim,
		SLONG         at_x,
		SLONG         at_y,
		SLONG         at_z,
		SLONG         at_yaw,
		NIGHT_Colour *lpc);


//
// Draws a mesh using the faces and textures from the given prim, but the
// points from the two given morphs.
//

void MESH_draw_morph(
		SLONG         prim,
		UBYTE         morph1,
		UBYTE         morph2,
		UWORD		  tween,		// 0 - 256         
		SLONG	      at_x,
		SLONG       at_y,
		SLONG	      at_z,
		SLONG         i_yaw,
		SLONG         i_pitch,
		SLONG         i_roll,
		NIGHT_Colour *lpc);




#endif


