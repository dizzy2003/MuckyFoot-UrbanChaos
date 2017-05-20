#ifndef	QUATERNION_H
#define QUATERNION_H

class	FloatMatrix
{
public:
	float M[3][3];
};

class	CQuaternion
{
public:

	float	w;
	float	x, y, z;
	
	static void	BuildTween(struct Matrix33 *dest,struct CMatrix33 *cm1,struct CMatrix33 *cm2,SLONG tween);
};

#endif