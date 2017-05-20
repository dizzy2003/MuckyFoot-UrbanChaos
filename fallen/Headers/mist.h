//
// Mist.
//

#ifndef MIST_H
#define MIST_H


//
// Initialises all the mist.
//

void MIST_init(void);


//
// Creates a quad-patch of mist over the ground.
//

void MIST_create(
		SLONG detail,			// The number of quads-per-row on the quad patch.
		SLONG height,			// Above the ground.
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);

//
// A gust of wind that the fog reacts to.
//

void MIST_gust(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);


//
// Processes all the layers of mist.
//

void MIST_process(void);


//
// How to get each point of each layer of mist
//

//
// Start getting each layer of mist.
// Returns the detail of the next layer of mist.
// Gives the position and texture coords of a point in the last
//		layer whose detail was returned by MIST_get_detail().
//

void  MIST_get_start (void);
SLONG MIST_get_detail(void);	// NULL => No more layers of mist.
void  MIST_get_point (SLONG px, SLONG pz,
		SLONG *x,
		SLONG *y,
		SLONG *z);
void  MIST_get_texture(SLONG px, SLONG pz,
		float *u,
		float *v);


#endif
