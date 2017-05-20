//
// Primitive shapes...
//

#ifndef _SHAPE_
#define _SHAPE_


#include "ob.h"


//
// Draws a semi-sphere whose edge is black and whose colour in the
// middle is given.  It is drawn with additive-alpha.
//

void SHAPE_semisphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,	// Gives the direction of the semi-sphere (256 long)
		SLONG dy,
		SLONG dz,
		SLONG radius,
		SLONG page,
		UBYTE red,
		UBYTE green,
		UBYTE blue);

void SHAPE_semisphere_textured(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,	// Gives the direction of the semi-sphere (256 long)
		SLONG dy,
		SLONG dz,
		SLONG radius,
		float u_mid,
		float v_mid,
		float uv_radius,
		SLONG page,
		UBYTE red,
		UBYTE green,
		UBYTE blue);

//
// Draws a sphere.
//

void SHAPE_sphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG radius,
		ULONG colour);

//	
//	Draws an alpha sphere.
//

void SHAPE_alpha_sphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG radius,
		ULONG colour,
		ULONG alpha);


//
// Draws a sparky line.
//

#define SHAPE_MAX_SPARKY_POINTS 16

void SHAPE_sparky_line(
		SLONG num_points,
		SLONG px[],
		SLONG py[],
		SLONG pz[],
		ULONG colour,
		float width);


//
// Draws a bit of glitter.
//

void SHAPE_glitter(
		SLONG x1,
		SLONG y1,
		SLONG z1,
		SLONG x2,
		SLONG y2,
		SLONG z2,
		ULONG colour);

//
// Draws a trip-wire.
// 

void SHAPE_tripwire(
		SLONG x1,
		SLONG y1,
		SLONG z1,
		SLONG x2,
		SLONG y2,
		SLONG z2,
		SLONG width,
		ULONG colour,
		UWORD counter,		// A frame animation counter.
		UBYTE along);		// How far along the line the tripwire goes.

//
// Draws a waterfall.
//

void SHAPE_waterfall(
		SLONG map_x,
		SLONG map_z,
		SLONG map_dx,
		SLONG map_dz,
		SLONG top,
		SLONG bot);


//
// Draws a droplet of water.
//

void SHAPE_droplet(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,
		SLONG dy,
		SLONG dz,
		ULONG colour,
		SLONG page);

//
// Draws a shadow for the given prim.
// 

void SHAPE_prim_shadow(OB_Info *oi);


//
// Draws a balloon.
//

void SHAPE_draw_balloon(SLONG balloon);



#endif





