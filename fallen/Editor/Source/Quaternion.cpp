#include	<math.h>

#include	"Editor.hpp"
#include	"Quaternion.h"


// JCL 22/12/98
// lots of code shamelessly ripped from the Gamasutra Web site.

//***************************************************************************************************

#define DELTA	0.05f  //! guess????????????

void	QuatSlerp(CQuaternion *from, CQuaternion *to, float t, CQuaternion *res)
{
    float	to1[4];
#ifdef TARGET_DC
	// Why the PC guys need doubles, I'll never know...
    float	omega, cosom, sinom, scale0, scale1;
#else
    double	omega, cosom, sinom, scale0, scale1;
#endif

    // calc cosine
    cosom = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;

    // adjust signs (if necessary)
    if ( cosom <0.0f )
	{
		cosom = -cosom;
		
		to1[0] = - to->x;
		to1[1] = - to->y;
		to1[2] = - to->z;
		to1[3] = - to->w;
    }
	else
	{
		to1[0] = to->x;
		to1[1] = to->y;
		to1[2] = to->z;
		to1[3] = to->w;
    }

    // calculate coefficients

	if ( (1.0f - cosom) > DELTA )
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0f - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;

    }
	else
	{        
	    // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
        scale0 = 1.0f - t;
        scale1 = t;
	}

	// calculate final values
	res->x = scale0 * from->x + scale1 * to1[0];
	res->y = scale0 * from->y + scale1 * to1[1];
	res->z = scale0 * from->z + scale1 * to1[2];
	res->w = scale0 * from->w + scale1 * to1[3];
}

//***************************************************************************************************
void	QuatMul(CQuaternion *q1, CQuaternion *q2, CQuaternion *res)
{
	float A, B, C, D, E, F, G, H;

	A = (q1->w + q1->x) * (q2->w + q2->x);
	B = (q1->z - q1->y) * (q2->y - q2->z);
	C = (q1->x - q1->w) * (q2->y - q2->z);
	D = (q1->y + q1->z) * (q2->x - q2->w);
	E = (q1->x + q1->z) * (q2->x + q2->y);
	F = (q1->x - q1->z) * (q2->x - q2->y);
	G = (q1->w + q1->y) * (q2->w - q2->z);
	H = (q1->w - q1->y) * (q2->w + q2->z);

	res->w =  B + (-E - F + G + H) / 2;
	res->x =  A - ( E + F + G + H) / 2; 
	res->y = -C + ( E - F + G - H) / 2;
	res->z = -D + ( E - F - G + H) / 2;
}

//***************************************************************************************************
void	EulerToQuat(float roll, float pitch, float yaw, CQuaternion * quat)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	// calculate trig identities
	cr = cos(roll/2);
	cp = cos(pitch/2);
	cy = cos(yaw/2);

	sr = sin(roll/2);
	sp = sin(pitch/2);
	sy = sin(yaw/2);
	
	cpcy = cp * cy;
	spsy = sp * sy;

	quat->w = cr * cpcy + sr * spsy;
	quat->x = sr * cpcy - cr * spsy;
	quat->y = cr * sp * cy + sr * cp * sy;
	quat->z = cr * cp * sy - sr * sp * cy;
}

//***************************************************************************************************
void	QuatToMatrix(CQuaternion *quat, FloatMatrix *fm)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat->x + quat->x; y2 = quat->y + quat->y; 
	z2 = quat->z + quat->z;
	xx = quat->x * x2;   xy = quat->x * y2;   xz = quat->x * z2;
	yy = quat->y * y2;   yz = quat->y * z2;   zz = quat->z * z2;
	wx = quat->w * x2;   wy = quat->w * y2;   wz = quat->w * z2;

	fm->M[0][0] = 1.0f - (yy + zz); 		fm->M[1][0] = xy - wz;
	fm->M[2][0] = xz + wy;				

	fm->M[0][1] = xy + wz;				fm->M[1][1] = 1.0f - (xx + zz);
	fm->M[2][1] = yz - wx;				

	fm->M[0][2] = xz - wy;				fm->M[1][2] = yz + wx;
	fm->M[2][2] = 1.0f - (xx + yy);		
}

//***************************************************************************************************
void	MatrixToQuat(FloatMatrix *fm, CQuaternion *quat)
{
	float  tr, s, q[4];
	int    i, j, k;

	int nxt[3] = {1, 2, 0};

	tr = fm->M[0][0] + fm->M[1][1] + fm->M[2][2];

	// check the diagonal
	if (tr > 0.0f)
	{
		s = sqrt (tr + 1.0f);
		quat->w = s / 2.0f;
		s = 0.5f / s;
		quat->x = (fm->M[1][2] - fm->M[2][1]) * s;
		quat->y = (fm->M[2][0] - fm->M[0][2]) * s;
		quat->z = (fm->M[0][1] - fm->M[1][0]) * s;
	}
	else 
	{		
		// diagonal is negative
		i = 0;
		if (fm->M[1][1] > fm->M[0][0]) i = 1;
		if (fm->M[2][2] > fm->M[i][i]) i = 2;
		j = nxt[i];
		k = nxt[j];

		s = sqrt ((fm->M[i][i] - (fm->M[j][j] + fm->M[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0f) s = 0.5f / s;

		q[3] = (fm->M[j][k] - fm->M[k][j]) * s;
		q[j] = (fm->M[i][j] + fm->M[j][i]) * s;
		q[k] = (fm->M[i][k] + fm->M[k][i]) * s;

		quat->x = q[0];
		quat->y = q[1];
		quat->z = q[2];
		quat->w = q[3];
	}
}

//***************************************************************************************************
//***************************************************************************************************

// temporary matrix conversion stuff

void	cmat_to_fmat(CMatrix33 *cm, FloatMatrix *fm)
{
	fm->M[0][0] = float(((cm->M[0] & CMAT0_MASK) >> 20)) / 512.f; 
	fm->M[0][1] = float(((cm->M[0] & CMAT1_MASK) >> 10)) / 512.f; 
	fm->M[0][2] = float(((cm->M[0] & CMAT2_MASK) >> 00)) / 512.f; 

	fm->M[1][0] = float(((cm->M[1] & CMAT0_MASK) >> 20)) / 512.f; 
	fm->M[1][1] = float(((cm->M[1] & CMAT1_MASK) >> 10)) / 512.f; 
	fm->M[1][2] = float(((cm->M[1] & CMAT2_MASK) >> 00)) / 512.f; 

	fm->M[2][0] = float(((cm->M[2] & CMAT0_MASK) >> 20)) / 512.f; 
	fm->M[2][1] = float(((cm->M[2] & CMAT1_MASK) >> 10)) / 512.f; 
	fm->M[2][2] = float(((cm->M[2] & CMAT2_MASK) >> 00)) / 512.f; 

	// this is pretty damn shite.
	if (fm->M[0][0] >= 1.f) fm->M[0][0] -= 2.f;
	if (fm->M[0][1] >= 1.f) fm->M[0][1] -= 2.f;
	if (fm->M[0][2] >= 1.f) fm->M[0][2] -= 2.f;
	if (fm->M[1][0] >= 1.f) fm->M[1][0] -= 2.f;
	if (fm->M[1][1] >= 1.f) fm->M[1][1] -= 2.f;
	if (fm->M[1][2] >= 1.f) fm->M[1][2] -= 2.f;
	if (fm->M[2][0] >= 1.f) fm->M[2][0] -= 2.f;
	if (fm->M[2][1] >= 1.f) fm->M[2][1] -= 2.f;
	if (fm->M[2][2] >= 1.f) fm->M[2][2] -= 2.f;
}

//***************************************************************************************************
void	fmat_to_mat(FloatMatrix *fm, Matrix33 *m)
{
	m->M[0][0] = SLONG(fm->M[0][0] * 32768.f);
	m->M[0][1] = SLONG(fm->M[0][1] * 32768.f);
	m->M[0][2] = SLONG(fm->M[0][2] * 32768.f);

	m->M[1][0] = SLONG(fm->M[1][0] * 32768.f);
	m->M[1][1] = SLONG(fm->M[1][1] * 32768.f);
	m->M[1][2] = SLONG(fm->M[1][2] * 32768.f);

	m->M[2][0] = SLONG(fm->M[2][0] * 32768.f);
	m->M[2][1] = SLONG(fm->M[2][1] * 32768.f);
	m->M[2][2] = SLONG(fm->M[2][2] * 32768.f);
}


//***************************************************************************************************
//***************************************************************************************************

void	build_tween_matrix(struct Matrix33 *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween);

// external stuff

void	CQuaternion::BuildTween(struct Matrix33 *dest,struct CMatrix33 *m1,struct CMatrix33 *m2,SLONG tween)
{
	// 1st attempt - (slow)
	//	* construct the quaternions from the compressed integer matrices
	//  * SLERP the quaternions using the tween value
	//  * construct a non-compressed integer matrix from the resulting quaternion

	FloatMatrix f1, f2, f3;

	cmat_to_fmat(m1, &f1);
	cmat_to_fmat(m2, &f2);
	
	float t = float(tween) / 256.f;
	float u = 1.f - t;

	CQuaternion q1, q2, q3;

	MatrixToQuat(&f1, &q1);
	MatrixToQuat(&f2, &q2);
	QuatSlerp(&q1, &q2, t, &q3);
	QuatToMatrix(&q3, &f3);

	fmat_to_mat(&f3, dest);
}
