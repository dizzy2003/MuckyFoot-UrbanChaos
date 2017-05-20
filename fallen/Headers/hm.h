//
// Hypermatter!
//

#ifndef HM_H
#define HM_H



//
// Initialises all the hypermatter objects.
//

void HM_init(void);



//
// Loads in the hm prim data from the editor. Expects a file
// that start with a header followed by an array
//

typedef struct
{
	SLONG version;			// Must be set to 1
	SLONG num_primgrids;	// The number of prim grids.
 
} HM_Header;

#define HM_MAX_RES 8

typedef struct
{
	UBYTE prim;
	UBYTE x_res;
	UBYTE y_res;
	UBYTE z_res;

	SLONG x_point[HM_MAX_RES];	// No point of the prim should lie outside the hypermatter
	SLONG y_point[HM_MAX_RES];
	SLONG z_point[HM_MAX_RES];

	float x_dgrav;
	float y_dgrav;
	float z_dgrav;

} HM_Primgrid;

void HM_load(CBYTE *fname);

//
// Returns an HM_Primgrid for the given prim. If one has not
// been defined, then it returns a 1x1x1 box around the
// entire prim.  Ignore the 'prim' field of what you get back.
//

HM_Primgrid *HM_get_primgrid(SLONG prim);


//
// Creates a new hypermatter object around the given mesh or returns
// HM_NO_MORE_OBJECTS
//

#define HM_NO_MORE_OBJECTS 255

UBYTE HM_create(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,

		SLONG dx,
		SLONG dy,
		SLONG dz,

		// Between 2 and HM_MAX_RES

		SLONG x_res,		// The number of points along the x-axis
		SLONG y_res,		// The number of points along the y-axis
		SLONG z_res,		// The number of points along the z-axis

		SLONG x_point[],	// The position of each point along the x-axis, 0 => The bounding box min, 0x10000 => the bb max.
		SLONG y_point[],
		SLONG z_point[],

		//
		// How the mass varies across the HM_object. If either of them
		// has an absolute value of > 1.0F then part of the object will
		// have negative weight!
		//

		float x_dgrav,
		float y_dgrav,
		float z_dgrav,

		//
		// These go from 0 to 1.
		// 

		float elasticity,
		float bounciness,
		float friction,
		float damping);

//
// Destroys the given HM_object.
//

void HM_destroy(UBYTE hm_index);


//
// Sets off a shockwave that effect the given hm object.
//

void HM_shockwave(
		UBYTE hm_index,
		float x,
		float y,
		float z,
		float range,
		float force);

//
// Returns where the hm_object's centre of gravity is.
//

void HM_find_cog(
		UBYTE  hm_index,
		float *x,
		float *y,
		float *z);

//
// Collides hypermatter object1 against hypermatter object2
//

void HM_collide_all(void);
void HM_collide(UBYTE hm_index1, UBYTE hm_index2);

//
// Collision of hypermatter objects with the colvects. Colvects
// are assumed to be infinitely tall.
//

void HM_colvect_clear(UBYTE hm_index);
void HM_colvect_add(
		UBYTE hm_index,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);

//
// Processes all the hypermatter objects.
//

void HM_process(void);

//
// Returns the given point of the mesh the hypermatter object
// was derived from.
// 

void HM_find_mesh_point(
		UBYTE  hm_index,
		SLONG  point,
		float *x,
		float *y,
		float *z);

//
// Returns the position and angle of the given hypermatter object.
//

void HM_find_mesh_pos(
		UBYTE  hm_index,
		SLONG *x,
		SLONG *y,
		SLONG *z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll);

//
// Returns TRUE if the given hypermatter object is stationary.
//

SLONG HM_stationary(UBYTE hm_index);

//
// Draws the hypermatter objects.
//

void HM_draw(void);







#endif
