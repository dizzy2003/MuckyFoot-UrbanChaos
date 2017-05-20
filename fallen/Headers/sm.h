//
// SphereMatter - Hypermatter version 2!
//

#ifndef _SM_
#define _SM_




//
// Initialises all the sphere matter
//

void SM_init(void);


//
// Create a sphere-matter cube.
//

#define SM_CUBE_TYPE_ROCK		0
#define SM_CUBE_TYPE_JELLY		1
#define SM_CUBE_TYPE_CARDBOARD	2
#define SM_CUBE_TYPE_NUMBER		3

void SM_create_cube(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2,
		SLONG amount_resolution,	// 0 - 256
		SLONG amount_density,		// 0 - 256
		SLONG amount_jellyness);	// 0 - 256


//
// Processes all the sphere-matter
//

void SM_process(void);


//
// Drawing the sphere matter just involves drawing lots of spheres.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG radius;
	ULONG colour;
	
} SM_Info;

void     SM_get_start(void);
SM_Info *SM_get_next (void);



#endif
