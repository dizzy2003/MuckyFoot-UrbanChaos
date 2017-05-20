#include <MFStdLib.h>
#include "matrix.h"

//#include <math.h>



//
// OPTIMISE THIS AT SOME POINT MARK! 'roll' AND 'pitch' ARE NEARLY ALWAYS 0!
//
void MATRIX_calc(SLONG matrix[9], SLONG yaw, SLONG pitch, SLONG roll)
{
	SLONG cy, cp, cr;
	SLONG sy, sp, sr;

	yaw=(yaw+2048)&2047;
	pitch=(pitch+2048)&2047;
	roll=(roll+2048)&2047;

	sy = SIN(yaw)>>1;
	sp = SIN(pitch)>>1;
	sr = SIN(roll)>>1;

	cy = COS(yaw)>>1;
	cp = COS(pitch)>>1;
	cr = COS(roll)>>1;

	
	//
	// Jan I trust you... but only becuase I've already seen it working!
	//

	matrix[0] =  (((cy * cr)>>15) + ((((sy * sp)>>15) * sr)>>15))<<1;
	matrix[3] =  (((cy * sr)>>15) - ((((sy * sp)>>15) * cr)>>15))<<1;
	matrix[6] =  ((sy * cp)>>15)<<1;
	matrix[1] = -(((cp * sr)>>15))<<1;
	matrix[4] =  ((cp * cr)>>15)<<1;
	matrix[7] =  (sp)<<1;
	matrix[2] = -(((sy * cr)>>15) + ((((cy * sp)>>15) * sr)>>15))<<1;
	matrix[5] = -(((sy * sr)>>15) - ((((cy * sp)>>15) * cr)>>15))<<1;
	matrix[8] =  ((cy * cp)>>15)<<1;
}
/*
void MATRIX_calc(SLONG matrix[9], SLONG yaw, SLONG pitch, SLONG roll)
{
	SLONG cy, cp, cr;
	SLONG sy, sp, sr;

	sy = sin(yaw);
	sp = sin(pitch);
	sr = sin(roll);

	cy = cos(yaw);
	cp = cos(pitch);
	cr = cos(roll);
	
	//
	// Jan I trust you... but only becuase I've already seen it working!
	//

	matrix[0] =  cy * cr + sy * sp * sr;
	matrix[3] =  cy * sr - sy * sp * cr;
	matrix[6] =  sy * cp;
	matrix[1] = -cp * sr;
	matrix[4] =  cp * cr;
	matrix[7] =  sp;
	matrix[2] = -sy * cr + cy * sp * sr;
	matrix[5] = -sy * sr - cy * sp * cr;
	matrix[8] =  cy * cp;
}
*/



#ifndef	PSX
void MATRIX_vector(SLONG vector[3], SLONG yaw, SLONG pitch)
{
	SLONG cy, cp;
	SLONG sy, sp;

	yaw=(yaw+2048)&2047;
	pitch=(pitch+2048)&2047;

	sy = SIN(yaw)>>1;
	sp = SIN(pitch)>>1;

	cy = COS(yaw)>>1;
	cp = COS(pitch)>>1;

	vector[0] =  (sy * cp)>>14;
	vector[1] =  sp<<1;
	vector[2] =  (cy * cp)>>14;
}

//                                     .075     3.3         1/(22*256)                               
#define	pMUL64(a,b)	(((a>>2)*(b>>2))>>12)
void MATRIX_skew(SLONG matrix[9], FIX_16 skew, FIX_16 zoom, FIX_16 scale)
{
	//
	// Squish up the matrix according to the aspect ratio of the screen.
	//

	matrix[0] = pMUL64(matrix[0] , skew);
	matrix[1] = pMUL64(matrix[1] , skew);
	matrix[2] = pMUL64(matrix[2] , skew);

	//
	// Create a lens by multiplying the x and y rows by the zoom factor...
	//

	matrix[0] = pMUL64(zoom , matrix[0]);
	matrix[1] = pMUL64(zoom , matrix[1]);
	matrix[2] = pMUL64(zoom , matrix[2]);

	matrix[3] = pMUL64(zoom , matrix[3]);
	matrix[4] = pMUL64(zoom , matrix[4]);
	matrix[5] = pMUL64(zoom , matrix[5]);

	//
	// Scale the whole matrix.
	//

	//
	// scale is quite small so we will loose a lot of precision here
	//

	matrix[0] = (matrix[0]*scale)>>8;
	matrix[1] = (matrix[1]*scale)>>8;
	matrix[2] = (matrix[2]*scale)>>8;
	matrix[3] = (matrix[3]*scale)>>8;
	matrix[4] = (matrix[4]*scale)>>8;
	matrix[5] = (matrix[5]*scale)>>8;
	matrix[6] = (matrix[6]*scale)>>8;
	matrix[7] = (matrix[7]*scale)>>8;
	matrix[8] = (matrix[8]*scale)>>8;

	// To avoid loss of precision matrix[8] has been made FIX_24
	//
	//

}
#endif
#ifndef	PSX
void MATRIX_3x3mul(SLONG a[9], SLONG m[9], SLONG n[9])
{
	a[0] = ((m[0] * n[0])>>16) + ((m[1] * n[3])>>16) + ((m[2] * n[6])>>16);
	a[1] = ((m[0] * n[1])>>16) + ((m[1] * n[4])>>16) + ((m[2] * n[7])>>16);
	a[2] = ((m[0] * n[2])>>16) + ((m[1] * n[5])>>16) + ((m[2] * n[8])>>16);
						           			           
	a[3] = ((m[3] * n[0])>>16) + ((m[4] * n[3])>>16) + ((m[5] * n[6])>>16);
	a[4] = ((m[3] * n[1])>>16) + ((m[4] * n[4])>>16) + ((m[5] * n[7])>>16);
	a[5] = ((m[3] * n[2])>>16) + ((m[4] * n[5])>>16) + ((m[5] * n[8])>>16);
						           			           
	a[6] = ((m[6] * n[0])>>16) + ((m[7] * n[3])>>16) + ((m[8] * n[6])>>16);
	a[7] = ((m[6] * n[1])>>16) + ((m[7] * n[4])>>16) + ((m[8] * n[7])>>16);
	a[8] = ((m[6] * n[2])>>16) + ((m[7] * n[5])>>16) + ((m[8] * n[8])>>16);
}
#endif










