//
// Matrix stuff...
//

#ifndef _MATRIX_
#define _MATRIX_

//
// ALL ANGLES ARE IN RADIANS.
//

//
// Fills in the matrix appropriately given the three angles (yaw, pitch, roll)
// for the eye along the z-axis.
//

void MATRIX_calc(float matrix[9], float yaw, float pitch, float roll);

//
// Makes a rotation matrix that rotates 'angle' radians about thie given unit vector
//

void MATRIX_calc_arb(
		float matrix[9],
		float ux,
		float uy,
		float uz,
		float angle);

//
// Calculates a vector from yaw and pitch...
//

void MATRIX_vector(float vector[3], float yaw, float pitch);

//
// Skews the view matrix so that it is correct for 3d clipping.
//
// The skew value should be one over the horizontal view-window resolution
// divided by the vertical view-window resolution.
//
// The x and y vectors are multiplied by the zoom.
//
// The whole matrix is scaled by scale.
//

void MATRIX_skew(float matrix[9], float skew, float zoom, float scale);

//
// Multiplies points x,y,z by matrix m.
//

#define MATRIX_MUL(m,x,y,z) 	   		\
{										\
	float xnew, ynew, znew;	   			\
							   			\
	xnew  = (x) * (m)[0];		   		\
	ynew  = (x) * (m)[3];		   		\
	znew  = (x) * (m)[6];		   		\
								   		\
	xnew += (y) * (m)[1];		   		\
	ynew += (y) * (m)[4];		   		\
	znew += (y) * (m)[7];		   		\
								   		\
	xnew += (z) * (m)[2];		   		\
	ynew += (z) * (m)[5];		   		\
	znew += (z) * (m)[8];		   		\
							   			\
	(x) = xnew; (y) = ynew; (z) = znew;	\
}


//
// Multiplies points x,y,z by the transpose of matrix m.
//

#define MATRIX_MUL_BY_TRANSPOSE(m,x,y,z) \
{										 \
	float xnew, ynew, znew;	   			 \
							   			 \
	xnew  = (x) * (m)[0];		   		 \
	ynew  = (x) * (m)[1];		   		 \
	znew  = (x) * (m)[2];		   		 \
								   		 \
	xnew += (y) * (m)[3];		   		 \
	ynew += (y) * (m)[4];		   		 \
	znew += (y) * (m)[5];		   		 \
								   		 \
	xnew += (z) * (m)[6];		   		 \
	ynew += (z) * (m)[7];		   		 \
	znew += (z) * (m)[8];		   		 \
							   			 \
	(x) = xnew; (y) = ynew; (z) = znew;	 \
}

//
// Multiplies two 3x3 together.  A = MN.
//

void MATRIX_3x3mul(float a[9], float m[9], float n[9]);

//
// Transposes the matrix m.
//

#define MATRIX_TRANSPOSE(m) {SWAP_FL(m[1], m[3]); SWAP_FL(m[2], m[6]); SWAP_FL(m[5], m[7]);}

//
// Rotates a matrix about its x,y or z vector.
//

void MATRIX_rotate_about_its_x(float *matrix, float angle);
void MATRIX_rotate_about_its_y(float *matrix, float angle);
void MATRIX_rotate_about_its_z(float *matrix, float angle);

//
// Convert a matrix into its equivalent yaw, pitch and roll.
//

Direction MATRIX_find_angles(float matrix[9]);


//
// Scales the given vector.
//

void MATRIX_scale(float matrix[9], float mulby);


//
// Constructs an orthonormal matrix from a vector whose [6][7][8] is the normalised
// vector (dx,dy,dz).  It return a matrix without any 'roll'.  (dx,dy,dz) does not
// have to be normalised.
//

void MATRIX_construct(float matrix[9], float dx, float dy, float dz);


#endif







