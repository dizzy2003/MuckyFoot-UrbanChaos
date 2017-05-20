#include <MFStdLib.h>
#include "c:\fallen\ddengine\headers\matrix.h"

#include <math.h>

#ifdef TARGET_DC
#include "target.h"
#endif

#ifdef TARGET_DC
#include "shsgintr.h"
#endif


//
// OPTIMISE THIS AT SOME POINT MARK! 'roll' AND 'pitch' ARE NEARLY ALWAYS 0!
//

void MATRIX_calc(float matrix[9], float yaw, float pitch, float roll)
{

	float cy, cp, cr;
	float sy, sp, sr;

#ifdef TARGET_DC

	// Use the fast intrinsics.
	// Error is 2e-21 at most.
	_SinCosA ( &sy, &cy, yaw );
	_SinCosA ( &sr, &cr, roll );
	_SinCosA ( &sp, &cp, pitch );

#else //#ifdef TARGET_DC

	sy = sin(yaw);
	sp = sin(pitch);
	sr = sin(roll);

	cy = cos(yaw);
	cp = cos(pitch);
	cr = cos(roll);

#endif //#else //#ifdef TARGET_DC

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




#ifndef TARGET_DC
//
// i'll do this later / miked
//
void MATRIX_calc_int(SLONG matrix[9], SLONG yaw, SLONG pitch, SLONG roll)
{
	float cy, cp, cr;
	float sy, sp, sr;

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
#endif //#ifndef TARGET_DC


void MATRIX_vector(float vector[3], float yaw, float pitch)
{
	float cy, cp;
	float sy, sp;

#ifdef TARGET_DC

	// Use the fast intrinsics.
	// Error is 2e-21 at most.
	_SinCosA ( &sy, &cy, yaw );
	_SinCosA ( &sp, &cp, pitch );

#else
	sy = sin(yaw);
	sp = sin(pitch);

	cy = cos(yaw);
	cp = cos(pitch);
#endif

	vector[0] =  sy * cp;
	vector[1] =  sp;
	vector[2] =  cy * cp;
}

void MATRIX_skew(float matrix[9], float skew, float zoom, float scale)
{
	//
	// Squish up the matrix according to the aspect ratio of the screen.
	//

	matrix[0] = matrix[0] * skew;
	matrix[1] = matrix[1] * skew;
	matrix[2] = matrix[2] * skew;

	//
	// Create a lens by multiplying the x and y rows by the zoom factor...
	//

	matrix[0] = zoom * matrix[0];
	matrix[1] = zoom * matrix[1];
	matrix[2] = zoom * matrix[2];

	matrix[3] = zoom * matrix[3];
	matrix[4] = zoom * matrix[4];
	matrix[5] = zoom * matrix[5];

	//
	// Scale the whole matrix.
	//

	matrix[0] *= scale;
	matrix[1] *= scale;
	matrix[2] *= scale;
	matrix[3] *= scale;
	matrix[4] *= scale;
	matrix[5] *= scale;
	matrix[6] *= scale;
	matrix[7] *= scale;
	matrix[8] *= scale;
}


void MATRIX_3x3mul(float a[9], float m[9], float n[9])
{
	a[0] = m[0] * n[0] + m[1] * n[3] + m[2] * n[6];
	a[1] = m[0] * n[1] + m[1] * n[4] + m[2] * n[7];
	a[2] = m[0] * n[2] + m[1] * n[5] + m[2] * n[8];

	a[3] = m[3] * n[0] + m[4] * n[3] + m[5] * n[6];
	a[4] = m[3] * n[1] + m[4] * n[4] + m[5] * n[7];
	a[5] = m[3] * n[2] + m[4] * n[5] + m[5] * n[8];

	a[6] = m[6] * n[0] + m[7] * n[3] + m[8] * n[6];
	a[7] = m[6] * n[1] + m[7] * n[4] + m[8] * n[7];
	a[8] = m[6] * n[2] + m[7] * n[5] + m[8] * n[8];
}


//
// Rotates the matrix by angle. You don't have to pass the 3rd row of the
// matrix, because it will be unchanged.
//

#ifdef TARGET_DC

// Use the fast intrinsics.
// Error is 2e-21 at most.

#define	MATRIX_ROTATE_ABOUT_Z(ax, ay, az, bx, by, bz, cx, cy, cz, angle)	\
{																			\
	float c;																\
	float s;																\
																			\
	_SinCosA ( &s, &c, angle );												\
																			\
	float px;																\
	float py;																\
	float pz;																\
																			\
	float qx;																\
	float qy;																\
	float qz;																\
																			\
	if (angle)																\
	{																		\
		px =  c * (ax) + s * (bx);											\
		py =  c * (ay) + s * (by);											\
		pz =  c * (az) + s * (bz);											\
			 																\
		qx = -s * (ax) + c * (bx);											\
		qy = -s * (ay) + c * (by);											\
		qz = -s * (az) + c * (bz);											\
																			\
		(ax) = px; (ay) = py; (az) = pz;									\
		(bx) = qx; (by) = qy; (bz) = qz;									\
	}																		\
}

#else

#define	MATRIX_ROTATE_ABOUT_Z(ax, ay, az, bx, by, bz, cx, cy, cz, angle)	\
{																			\
	float c = cos(angle);													\
	float s = sin(angle);													\
																			\
	float px;																\
	float py;																\
	float pz;																\
																			\
	float qx;																\
	float qy;																\
	float qz;																\
																			\
	if (angle)																\
	{																		\
		px =  c * (ax) + s * (bx);											\
		py =  c * (ay) + s * (by);											\
		pz =  c * (az) + s * (bz);											\
			 																\
		qx = -s * (ax) + c * (bx);											\
		qy = -s * (ay) + c * (by);											\
		qz = -s * (az) + c * (bz);											\
																			\
		(ax) = px; (ay) = py; (az) = pz;									\
		(bx) = qx; (by) = qy; (bz) = qz;									\
	}																		\
}

#endif


void MATRIX_rotate_about_its_x(float *matrix, float angle)
{
	MATRIX_ROTATE_ABOUT_Z(
		matrix[1], matrix[4], matrix[7],
		matrix[2], matrix[5], matrix[8],
		matrix[0], matrix[3], matrix[6],
	   -angle);
}


void MATRIX_rotate_about_its_y(float *matrix, float angle)
{
	MATRIX_ROTATE_ABOUT_Z(
		matrix[2], matrix[5], matrix[8],
		matrix[0], matrix[3], matrix[6],
		matrix[1], matrix[4], matrix[7],
	   -angle);
}


void MATRIX_rotate_about_its_z(float *matrix, float angle)
{
	MATRIX_ROTATE_ABOUT_Z(
		matrix[0], matrix[3], matrix[6],
		matrix[1], matrix[4], matrix[7],
		matrix[2], matrix[5], matrix[8],
	   -angle);
}


#define MATRIX_FA_VECTOR_TOO_SMALL (0.001F)
#define MATRIX_FA_ANGLE_TOO_SMALL  (PI * 2.0F / 180.0F)

#ifndef TARGET_DC
Direction MATRIX_find_angles_old(float matrix[9])
{
	float x;
	float y;
	float z;
	float xz;

	Direction ans;

	//
	// Look from above at the z-vector to work out the yaw.
	//

	x = matrix[6];
	y = matrix[7];
	z = matrix[8];

	if (fabsf(x) + fabsf(z) < MATRIX_FA_VECTOR_TOO_SMALL)
	{
		//
		// Try using the x-vector instead.
		//

		float x1 = matrix[0];
		float y1 = matrix[1];
		float z1 = matrix[2];

		ans.yaw = atan2(-z1, x1);

		//ans.yaw = 0.0F;
	}
	else
	{
	   ans.yaw = atan2(x, z);
	}

	//
	// Look down the x vector to at the z-vector to work out the pitch.
	//

	xz = sqrt(x*x + z*z);

	if (fabsf(xz) + fabsf(y) < MATRIX_FA_VECTOR_TOO_SMALL)
	{
		if (y < 0) {ans.pitch = -PI;} else {ans.pitch = +PI;}
	}
	else
	{
		ans.pitch = atan2(y, xz);
	}

	//
	// Now... matrix[4] =  cos(pitch) * cos(roll)
	//		  matrix[1] = -cos(pitch) * sin(roll)
	//
	// so...  cos(roll) = matrix[4] /  cos(pitch)
	//        sin(roll) = matrix[1] / -cos(pitch)
	//
	
	float cos_roll;
	float sin_roll;
	float cos_pitch;

	cos_pitch = cos(ans.pitch);

	if (fabsf(cos_pitch) < MATRIX_FA_ANGLE_TOO_SMALL)
	{
		ans.roll = 0.0F;
	}
	else
	{
		cos_roll = matrix[4] /  cos_pitch;
		sin_roll = matrix[1] / -cos_pitch;

		ans.roll = atan2(sin_roll, cos_roll);
	}

	return ans;
}
#endif


Direction MATRIX_find_angles(float matrix[9])
{
	Direction ans;

	//
	// matrix[7] is the sin(pitch)
	//

	ans.pitch = (float)asin(matrix[7]);

	//
	// Work out yaw differently depending on how much we are looking
	// down or up.
	//

	if (fabsf(ans.pitch) > (PI / 4.0F))
	{
		if (fabsf(matrix[0]) + fabsf(matrix[2]) < 0.1F)
		{
		}

		ans.yaw = (float)atan2(matrix[0], matrix[2]) - (PI / 2.0F);
	}
	else
	{
		ans.yaw = (float)atan2(matrix[6], matrix[8]);
	}

	//
	// Now... matrix[4] =  cos(pitch) * cos(roll)
	//		  matrix[1] = -cos(pitch) * sin(roll)
	//
	// so...  cos(roll) = matrix[4] /  cos(pitch)
	//        sin(roll) = matrix[1] / -cos(pitch)
	//

	float cos_roll;
	float sin_roll;
	float cos_pitch;

	cos_pitch = (float)cos(ans.pitch);

	if (fabsf(cos_pitch) < 0.0001F)
	{
		ans.roll = 0.0F;
	}
	else
	{
		cos_roll = matrix[4] /  cos_pitch;
		sin_roll = matrix[1] / -cos_pitch;

		ans.roll = (float)atan2(sin_roll, cos_roll);
	}

	return ans;
}












