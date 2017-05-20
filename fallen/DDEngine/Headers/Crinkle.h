//
// Crinkles!
//

#ifndef CRINKLE_H
#define CRINKLE_H


#include "aeng.h"
#include "poly.h"
#include "fileclump.h"


typedef UWORD CRINKLE_Handle;


//
// Clears out all the crinkles and invalidates all the handles.
//

void CRINKLE_init(void);


//
// Loads the given crinkle in and returns its handle.  Returns
// CRINKLE_NULL if it could not load the crinkle.
//

CRINKLE_Handle CRINKLE_load(CBYTE *sex_filename);
CRINKLE_Handle CRINKLE_read_bin(FileClump* tclump, int id);

void CRINKLE_write_bin(FileClump* tclump, CRINKLE_Handle hnd, int id);

//
// Sets the view-space light vector.
//

void CRINKLE_light(float dx, float dy, float dz);


//
// Tells the crinkle module how view-space is skewed.
//

void CRINKLE_skew(float aspect, float lens);



//
// Draws a crinkle extruded by 'amount' from 0.0 to 1.0 between
// the four points.
//

void CRINKLE_do(
		CRINKLE_Handle crinkle,
		SLONG          page,
		float          amount,
		POLY_Point    *pp[4],
		SLONG          flip);


//
// Projects an SMAP shadow over the crinkle.
//

void CRINKLE_project(
		CRINKLE_Handle crinkle,
		float          amount,
		SVector_F      poly[4],
		SLONG          flip);




#endif
