//
// Shadow maps.
//

#ifndef _SMAP_
#define _SMAP_



//
// Creates a shadow map of the given person.
//

void SMAP_person(
		Thing     *person, 
		UBYTE     *bitmap,	// 0 => transparent 255 => opaque
		UBYTE      u_res,
		UBYTE      v_res,
		SLONG      light_dx, // This vector need not be normalised
		SLONG      light_dy,
		SLONG      light_dz);

void SMAP_bike(
		Thing     *person, 
		UBYTE     *bitmap,	// 0 => transparent 255 => opaque
		UBYTE      u_res,
		UBYTE      v_res,
		SLONG      light_dx, // This vector need not be normalised
		SLONG      light_dy,
		SLONG      light_dz);


//
// Projects the last shadow map created onto a poly in world-space. The poly
// should be given in clockwise order.  This function returns NULL if the poly
// is facing away from the light, or if the poly is on the 'wrong side' of
// the shadow map.  The polygon linked list is valid until the next call
// to the function.
//
// This is not a circular data structure. The last point of the poly has a NULL
// pointer. There is not a copy of the first point at the end of the linked list.
//

typedef struct smap_link
{
	float wx;
	float wy;
	float wz;
	float u;
	float v;

	struct smap_link *next;

	ULONG clip;	// Private!
	
} SMAP_Link;

SMAP_Link *SMAP_project_onto_poly(SVector_F poly[], SLONG num_points); // poly must be planar.



#endif
