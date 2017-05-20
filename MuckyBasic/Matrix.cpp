#include "always.h"
#include "matrix.h"



//
// OPTIMISE THIS AT SOME POINT MARK! 'roll' AND 'pitch' ARE NEARLY ALWAYS 0!
//

void MATRIX_calc(float matrix[9], float yaw, float pitch, float roll)
{
	float cy, cp, cr;
	float sy, sp, sr;

	sy = sinf(yaw);
	sp = sinf(pitch);
	sr = sinf(roll);

	cy = cosf(yaw);
	cp = cosf(pitch);
	cr = cosf(roll);
	
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

void MATRIX_calc_arb(
		float matrix[9],
		float ux,
		float uy,
		float uz,
		float angle)
{
	float sa = sinf(angle);
	float ca = cosf(angle);

	float ux2 = ux*ux;
	float uy2 = uy*uy;
	float uz2 = uz*uz;

	float uxuy = ux*uy;
	float uxuz = ux*uz;
	float uyuz = uy*uz;

	matrix[0] = ux2 + ca*(1 - ux2);

	matrix[1] = uxuy*(1 - ca) - uz*sa;

	matrix[2] = uxuz*(1 - ca) + uy*sa;

	matrix[3] = uxuy*(1 - ca) + uz*sa;

	matrix[4] = uy2 + ca*(1 - uy2);

	matrix[5] = uyuz*(1 - ca) - ux*sa;

	matrix[6] = uxuz*(1 - ca) - uy*sa;

	matrix[7] = uy*uz*(1 - ca) + ux*sa;

	matrix[8] = uz2 + ca*(1 - uz2);

}


void MATRIX_vector(float vector[3], float yaw, float pitch)
{
	float cy, cp;
	float sy, sp;

	sy = sin(yaw);
	sp = sin(pitch);

	cy = cos(yaw);
	cp = cos(pitch);

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
	float result[9];

	result[0] = m[0] * n[0] + m[1] * n[3] + m[2] * n[6];
	result[1] = m[0] * n[1] + m[1] * n[4] + m[2] * n[7];
	result[2] = m[0] * n[2] + m[1] * n[5] + m[2] * n[8];

	result[3] = m[3] * n[0] + m[4] * n[3] + m[5] * n[6];
	result[4] = m[3] * n[1] + m[4] * n[4] + m[5] * n[7];
	result[5] = m[3] * n[2] + m[4] * n[5] + m[5] * n[8];

	result[6] = m[6] * n[0] + m[7] * n[3] + m[8] * n[6];
	result[7] = m[6] * n[1] + m[7] * n[4] + m[8] * n[7];
	result[8] = m[6] * n[2] + m[7] * n[5] + m[8] * n[8];

	a[0] = result[0];
	a[1] = result[1];
	a[2] = result[2];

	a[3] = result[3];
	a[4] = result[4];
	a[5] = result[5];

	a[6] = result[6];
	a[7] = result[7];
	a[8] = result[8];
}


//
// Rotates the matrix by angle. You don't have to pass the 3rd row of the
// matrix, because it will be unchanged.
//

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

		ans.yaw = atan2f(-z1, x1);

		//ans.yaw = 0.0F;
	}
	else
	{
	   ans.yaw = atan2f(x, z);
	}

	//
	// Look down the x vector to at the z-vector to work out the pitch.
	//

	xz = sqrtf(x*x + z*z);

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

	cos_pitch = cosf(ans.pitch);

	if (fabsf(cos_pitch) < MATRIX_FA_ANGLE_TOO_SMALL)
	{
		ans.roll = 0.0F;
	}
	else
	{
		cos_roll = matrix[4] /  cos_pitch;
		sin_roll = matrix[1] / -cos_pitch;

		ans.roll = atan2f(sin_roll, cos_roll);
	}

	return ans;
}



Direction MATRIX_find_angles(float matrix[9])
{
	Direction ans;

	//
	// matrix[7] is the sin(pitch)
	//

	ans.pitch = asinf(matrix[7]);

	//
	// Work out yaw differently depending on how much we are looking
	// down or up.
	//

	if (fabsf(ans.pitch) > (PI / 4.0F))
	{
		if (fabsf(matrix[0]) + fabsf(matrix[2]) < 0.1F)
		{
		}

		ans.yaw = atan2f(matrix[0], matrix[2]) - (PI / 2.0F);
	}
	else
	{
		ans.yaw = atan2f(matrix[6], matrix[8]);
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

	cos_pitch = cosf(ans.pitch);

	if (fabsf(cos_pitch) < 0.0001F)
	{
		ans.roll = 0.0F;
	}
	else
	{
		cos_roll = matrix[4] /  cos_pitch;
		sin_roll = matrix[1] / -cos_pitch;

		ans.roll = atan2f(sin_roll, cos_roll);
	}

	return ans;
}


void MATRIX_scale(float matrix[9], float mulby)
{
	matrix[0] *= mulby;
	matrix[1] *= mulby;
	matrix[2] *= mulby;
	matrix[3] *= mulby;
	matrix[4] *= mulby;
	matrix[5] *= mulby;
	matrix[6] *= mulby;
	matrix[7] *= mulby;
	matrix[8] *= mulby;
}


void MATRIX_construct(float matrix[9], float dx, float dy, float dz)
{
	float cx;
	float cy;
	float cz;
	float len;
	float overlen;
	
	len     = sqrtf(dx*dx + dy*dy + dz*dz);
	overlen = 1.0F / len;

	dx *= overlen;
	dy *= overlen;
	dz *= overlen;

	//
	// We've got the z-row of the matrix now.
	//

	matrix[6] = dx;
	matrix[7] = dy;
	matrix[8] = dz;

	cx = 0.0F;
	cy = 1.0F;
	cz = 0.0F;

	matrix[0] = cy*dz - cz*dy;
	matrix[1] = cz*dx - cx*dz;
	matrix[2] = cx*dy - cy*dx;

	len =
		matrix[0]*matrix[0] + 
		matrix[1]*matrix[1] + 
		matrix[2]*matrix[2];

	if (len < 0.001F)
	{
		//
		// Oh dear... too small!
		//

		MATRIX_calc(
			matrix,
			0.0F,
		   -PI * 0.5F,
			0.0F);

		return;
	}

	MATRIX_NORMALISE(
		matrix[0],
		matrix[1],
		matrix[2]);

	matrix[3] = -matrix[1]*matrix[8] + matrix[2]*matrix[7];
	matrix[4] = -matrix[2]*matrix[6] + matrix[0]*matrix[8];
	matrix[5] = -matrix[0]*matrix[7] + matrix[1]*matrix[6];

	MATRIX_NORMALISE(
		matrix[3],
		matrix[4],
		matrix[5]);

	
}

