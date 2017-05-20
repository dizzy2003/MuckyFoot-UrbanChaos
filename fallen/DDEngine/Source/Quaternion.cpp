#include	<math.h>

#include	"game.h"
#include	"Quaternion.h"
#include	"c:\fallen\headers\fmatrix.h"

void	QUATERNION_BuildTweenInteger(struct Matrix33 *dest,struct CMatrix33 *cm1,struct CMatrix33 *cm2,SLONG tween);

// JCL 22/12/98
// lots of code shamelessly ripped (and then fixed...) from the Gamasutra Web site.

//***************************************************************************************************

#define DELTA	0.05  //! guess????????????

void	QuatSlerp(CQuaternion *from, CQuaternion *to, float t, CQuaternion *res)
{
    float	to1[4];
    double	omega, cosom, sinom, scale0, scale1;


    // calc cosine
    cosom = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;

    // adjust signs (if necessary)
    if ( cosom <0.0 )
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

	if ( (1.0 - cosom) > DELTA )
	{
		// standard case (slerp)
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0 - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;

    }
	else
	{        
	    // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
        scale0 = 1.0 - t;
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

	fm->M[0][0] = 1.0 - (yy + zz); 		fm->M[1][0] = xy - wz;
	fm->M[2][0] = xz + wy;				

	fm->M[0][1] = xy + wz;				fm->M[1][1] = 1.0 - (xx + zz);
	fm->M[2][1] = yz - wx;				

	fm->M[0][2] = xz - wy;				fm->M[1][2] = yz + wx;
	fm->M[2][2] = 1.0 - (xx + yy);		
}

//***************************************************************************************************
void	MatrixToQuat(FloatMatrix *fm, CQuaternion *quat)
{
	float  tr, s, q[4];
	int    i, j, k;

	int nxt[3] = {1, 2, 0};

	tr = fm->M[0][0] + fm->M[1][1] + fm->M[2][2];

	// check the diagonal
//	if (tr > 1.0) //!???!?!?!?!!?!?!??!?
	if (tr > 0.0)
	{
		s = sqrt (tr + 1.0);
		quat->w = s / 2.0;
		s = 0.5 / s;
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

		s = sqrt ((fm->M[i][i] - (fm->M[j][j] + fm->M[k][k])) + 1.0);

		q[i] = s * 0.5;

		if (s != 0.0) s = 0.5 / s;

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

BOOL	is_unit(float a, float b, float c)
{
	return (fabs(1.0 - (a*a + b*b + c*c)) < 0.01);
}

BOOL	check_isonormal(FloatMatrix &m)
{
	BOOL	r = TRUE;

	if ((m.M[0][0] == 0) && (m.M[0][1] == 0) && (m.M[0][2] == 0))
		return TRUE; // void matrix

#ifdef _DEBUG_POO
	// check units
	r &= is_unit(m.M[0][0], m.M[0][1], m.M[0][2]);
	r &= is_unit(m.M[1][0], m.M[1][1], m.M[1][2]);
	r &= is_unit(m.M[2][0], m.M[2][1], m.M[2][2]);
	r &= is_unit(m.M[0][0], m.M[1][0], m.M[2][0]);
	r &= is_unit(m.M[0][1], m.M[1][1], m.M[2][1]);
	r &= is_unit(m.M[0][2], m.M[1][2], m.M[2][2]);

	ASSERT(r);

	// check orthogonality
	float	a = (m.M[0][0]*m.M[1][0] + m.M[0][1]*m.M[1][1] + m.M[0][2]*m.M[1][2]);
	float	b = (m.M[0][0]*m.M[2][0] + m.M[0][1]*m.M[2][1] + m.M[0][2]*m.M[2][2]);
	float	c = (m.M[2][0]*m.M[1][0] + m.M[2][1]*m.M[1][1] + m.M[2][2]*m.M[1][2]);

	ASSERT(fabs(a) < 0.03);
	ASSERT(fabs(b) < 0.03);
	ASSERT(fabs(c) < 0.03);
#endif

	// check handedness
	float	x = m.M[0][1] * m.M[1][2] - m.M[0][2] * m.M[1][1];
	float	y = m.M[0][2] * m.M[1][0] - m.M[0][0] * m.M[1][2];
	float	z = m.M[0][0] * m.M[1][1] - m.M[0][1] * m.M[1][0];

	if ((fabs(x - m.M[2][0]) > 0.03) ||
	    (fabs(y - m.M[2][1]) > 0.03) ||
	    (fabs(z - m.M[2][2]) > 0.03))
		return FALSE;

	return TRUE;
}

void	build_tween_matrix(struct Matrix33 *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween);

// external stuff

void	CQuaternion::BuildTween(struct Matrix33 *dest,struct CMatrix33 *cm1,struct CMatrix33 *cm2,SLONG tween)
{
	//	* construct the quaternions from the compressed integer matrices
	//  * SLERP the quaternions using the tween value
	//  * construct a non-compressed integer matrix from the resulting quaternion

	//! mmm
//	QUATERNION_BuildTweenInteger(dest, cm1, cm2, tween);
//	return;

#ifndef PSX

	FloatMatrix f1, f2, f3;

	cmat_to_fmat(cm1, &f1);
	cmat_to_fmat(cm2, &f2);

	BOOL	a, b;
	a = check_isonormal(f1);
	b = check_isonormal(f2);

//	if (b)
//	if (1)

	// ok - if handedness is wrong, flip before and afterwards.
	// only if both src and dest handedness is the same..
	if (a == b)
	{
/*
		if (!a)
		{
			// flip
			f1.M[2][0] = -f1.M[2][0];
			f1.M[2][1] = -f1.M[2][1];
			f1.M[2][2] = -f1.M[2][2];
			f2.M[2][0] = -f2.M[2][0];
			f2.M[2][1] = -f2.M[2][1];
			f2.M[2][2] = -f2.M[2][2];
		}
*/
		float t = float(tween) / 256.f;

		CQuaternion q1, q2, q3;

		MatrixToQuat(&f1, &q1);
		MatrixToQuat(&f2, &q2);

		QuatSlerp(&q1, &q2, t, &q3);
		QuatToMatrix(&q3, &f3);
/*
		if (!a)
		{
			// flip
			f3.M[2][0] = -f3.M[2][0];
			f3.M[2][1] = -f3.M[2][1];
			f3.M[2][2] = -f3.M[2][2];
		}
*/
		fmat_to_mat(&f3, dest);

		return;
	}
#endif

	// fallback for dodgy matrices and PSX version
	build_tween_matrix(dest, cm1, cm2, tween);
	normalise_matrix_rows(dest);
}

//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************

// integer version!

//***************************************************************************************************
struct	QuatInt
{
	SLONG	x, y, z, w;
};

//***************************************************************************************************
void	MatrixToQuatInteger(Matrix33 *m, QuatInt *quat)
{
	SLONG	tr, s;
	SLONG   q[4];
	SLONG	i, j, k;

	SLONG	 nxt[3] = {1, 2, 0};

	tr = m->M[0][0] + m->M[1][1] + m->M[2][2];

	// check the diagonal
	if (tr > 0)
	{
		s = Root(tr + (1 << 15)) * 181; /// argh..  ==  << 7.5 ...
		quat->w = s / 2;
		s = ((1 << (14 + 12)) / s) << 3; // hmph - avoid overflow...
		quat->x = ((m->M[1][2] - m->M[2][1]) * s) >> 15;
		quat->y = ((m->M[2][0] - m->M[0][2]) * s) >> 15;
		quat->z = ((m->M[0][1] - m->M[1][0]) * s) >> 15;
	}
	else 
	{		
		// diagonal is negative
		i = 0;
		if (m->M[1][1] > m->M[0][0]) i = 1;
		if (m->M[2][2] > m->M[i][i]) i = 2;
		j = nxt[i];
		k = nxt[j];

		s = Root((m->M[i][i] - (m->M[j][j] + m->M[k][k])) + (1 << 15)) * 181 ;

		q[i] = s / 2;

		if (s != 0)
			s = ((1 << (14 + 12)) / s) << 3; // hmph - avoid overflow...

		q[3] = ((m->M[j][k] - m->M[k][j]) * s) >> 15;
		q[j] = ((m->M[i][j] + m->M[j][i]) * s) >> 15;
		q[k] = ((m->M[i][k] + m->M[k][i]) * s) >> 15;

		quat->x = q[0];
		quat->y = q[1];
		quat->z = q[2];
		quat->w = q[3];
	}
}

//***************************************************************************************************
void	QuatToMatrixInteger(QuatInt *quat, Matrix33 *m)
{
	SLONG wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat->x + quat->x; y2 = quat->y + quat->y; 
	z2 = quat->z + quat->z;
	xx = (quat->x * x2) >> 15;   xy = (quat->x * y2) >> 15;   xz = (quat->x * z2) >> 15;
	yy = (quat->y * y2) >> 15;   yz = (quat->y * z2) >> 15;   zz = (quat->z * z2) >> 15;
	wx = (quat->w * x2) >> 15;   wy = (quat->w * y2) >> 15;   wz = (quat->w * z2) >> 15;

	m->M[0][0] = (1 << 15) - (yy + zz);		m->M[1][0] = xy - wz;
	m->M[2][0] = xz + wy;				

	m->M[0][1] = xy + wz;					m->M[1][1] = (1 << 15) - (xx + zz);
	m->M[2][1] = yz - wx;				

	m->M[0][2] = xz - wy;					m->M[1][2] = yz + wx;
	m->M[2][2] = (1 << 15) - (xx + yy);		
}

//***************************************************************************************************
//! er.. this should be loaded in....

SWORD	acos_table[1025]; // only half of it!
BOOL	acos_table_init = FALSE;

void	BuildACosTable()
{
	SLONG c0;

	for (c0 = 0; c0 < 1025; c0 ++)
	{
		acos_table[c0] = SLONG(acos(float(c0) / 1025.f) / (2 * 3.1415926) * 2047);
	}

	acos_table_init = TRUE;
}

//***************************************************************************************************
#define DELTA_INT 1638

void	QuatSlerpInteger(QuatInt *from, QuatInt *to, SLONG tween, QuatInt *res)
{
    SLONG	to1[4];
//    double	omega, cosom, sinom;
	SLONG	omega, cosom, sinom;
	SLONG	scale0, scale1;

	//! shouldn't be done here...
	if (!acos_table_init)
		BuildACosTable();

    // calc cosine
    cosom = (from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w) >> 15;

    // adjust signs (if necessary)
    if ( cosom < 0 )
	{
		cosom = -cosom;
		
		to1[0] = -to->x;
		to1[1] = -to->y;
		to1[2] = -to->z;
		to1[3] = -to->w;
    }
	else
	{
		to1[0] = to->x;
		to1[1] = to->y;
		to1[2] = to->z;
		to1[3] = to->w;
    }

    // calculate coefficients

	if (((1 << 15) - cosom) > DELTA_INT)
	{
		// standard case (slerp)
//		omega = SLONG((acos(float(cosom) / 32768) / (2 * 3.1415926)) * 2047);

		ASSERT(cosom >= 0);
		ASSERT(cosom <= 32768);
		omega = acos_table[cosom / 32];
		
		sinom = SIN(omega);
		scale0 = (SIN(((256 - tween) * omega) / 256) * 256) / sinom;
		scale1 = (SIN(((tween      ) * omega) / 256) * 256) / sinom;

    }
	else
	{        
	    // "from" and "to" quaternions are very close 
	    //  ... so we can do a linear interpolation
        scale0 = 256 - tween;
        scale1 = tween;
	}

	// calculate final values
	res->x = (scale0 * from->x + scale1 * to1[0]) >> 8;
	res->y = (scale0 * from->y + scale1 * to1[1]) >> 8;
	res->z = (scale0 * from->z + scale1 * to1[2]) >> 8;
	res->w = (scale0 * from->w + scale1 * to1[3]) >> 8;
}

//***************************************************************************************************
void	cmat_to_mat(CMatrix33 *cm, Matrix33 *m)
{
	m->M[0][0] = ((cm->M[0] & CMAT0_MASK) >> 20) << 6;
	m->M[0][1] = ((cm->M[0] & CMAT1_MASK) >> 10) << 6;
	m->M[0][2] = ((cm->M[0] & CMAT2_MASK) >> 00) << 6;

	m->M[1][0] = ((cm->M[1] & CMAT0_MASK) >> 20) << 6;
	m->M[1][1] = ((cm->M[1] & CMAT1_MASK) >> 10) << 6;
	m->M[1][2] = ((cm->M[1] & CMAT2_MASK) >> 00) << 6;

	m->M[2][0] = ((cm->M[2] & CMAT0_MASK) >> 20) << 6;
	m->M[2][1] = ((cm->M[2] & CMAT1_MASK) >> 10) << 6;
	m->M[2][2] = ((cm->M[2] & CMAT2_MASK) >> 00) << 6;

	// this is pretty damn shite.
	if (m->M[0][0] >= 32768) m->M[0][0] -= 65536;
	if (m->M[0][1] >= 32768) m->M[0][1] -= 65536;
	if (m->M[0][2] >= 32768) m->M[0][2] -= 65536;
	if (m->M[1][0] >= 32768) m->M[1][0] -= 65536;
	if (m->M[1][1] >= 32768) m->M[1][1] -= 65536;
	if (m->M[1][2] >= 32768) m->M[1][2] -= 65536;
	if (m->M[2][0] >= 32768) m->M[2][0] -= 65536;
	if (m->M[2][1] >= 32768) m->M[2][1] -= 65536;
	if (m->M[2][2] >= 32768) m->M[2][2] -= 65536;
}

//***************************************************************************************************
BOOL	check_isonormal_integer(Matrix33 &m)
{
	// check handedness
	SLONG	x = (m.M[0][1] * m.M[1][2] - m.M[0][2] * m.M[1][1]) >> 15;
	SLONG	y = (m.M[0][2] * m.M[1][0] - m.M[0][0] * m.M[1][2]) >> 15;
	SLONG	z = (m.M[0][0] * m.M[1][1] - m.M[0][1] * m.M[1][0]) >> 15;

	if ((abs(x - m.M[2][0]) > 1000) ||
	    (abs(y - m.M[2][1]) > 1000) ||
	    (abs(z - m.M[2][2]) > 1000))
		return FALSE;

	return TRUE;
}

//***************************************************************************************************
void	QUATERNION_BuildTweenInteger(struct Matrix33 *dest,struct CMatrix33 *cm1,struct CMatrix33 *cm2,SLONG tween)
{
	//	* construct the quaternions from the compressed integer matrices
	//  * SLERP the quaternions using the tween value
	//  * construct a non-compressed integer matrix from the resulting quaternion

	Matrix33 m1, m2;

	cmat_to_mat(cm1, &m1);
	cmat_to_mat(cm2, &m2);

	BOOL	a, b;
	a = check_isonormal_integer(m1);
	b = check_isonormal_integer(m2);

	// ok - if handedness is wrong, flip before and afterwards.
	// only if both src and dest handedness is the same..
	if (a == b)
	{
		if (!a)
		{
			// flip
			m1.M[2][0] = -m1.M[2][0];
			m1.M[2][1] = -m1.M[2][1];
			m1.M[2][2] = -m1.M[2][2];
			m2.M[2][0] = -m2.M[2][0];
			m2.M[2][1] = -m2.M[2][1];
			m2.M[2][2] = -m2.M[2][2];
		}

		QuatInt q1, q2, q3;

		MatrixToQuatInteger(&m1, &q1);
		MatrixToQuatInteger(&m2, &q2);

		QuatSlerpInteger(&q1, &q2, tween, &q3);
		QuatToMatrixInteger(&q3, dest);

//		QuatToMatrixInteger(&q1, dest);

		if (!a)
		{
			// flip
			dest->M[2][0] = -dest->M[2][0];
			dest->M[2][1] = -dest->M[2][1];
			dest->M[2][2] = -dest->M[2][2];
		}

		return;
	}

	// fallback for dodgy matrices
	build_tween_matrix(dest, cm1, cm2, tween);
	normalise_matrix_rows(dest);

}

//***************************************************************************************************
//***************************************************************************************************
