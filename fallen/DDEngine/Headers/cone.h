//
// Cones clipped by planar polygons.
//

#ifndef _CONE_
#define _CONE_


//
// Creates a new cone. (dx,dy,dz) need not be normalised.
//

void CONE_create(
		float x,
		float y,
		float z,
		float dx,
		float dy,
		float dz,
		float length,
		float radius,
		ULONG colour_start,
		ULONG colour_end,
		SLONG detail);		//  A value between 0 and 256


//
// Clips the last created cone with the given planar polygon.
//

typedef struct
{
	float x;
	float y;
	float z;

} CONE_Poly;

void CONE_clip(
		CONE_Poly p[],
		SLONG     num_points);


//
// Goes through the map looking for walls to intersect the cone with.
//

void CONE_intersect_with_map(void);



//
// Annoyingly, you can optimise the above call a lot if the poly is paralell to
// the x, y or z axis.  Nearly all the polygons will be.
//

//
// Draws the current cone.
//

void CONE_draw(void);



#endif
