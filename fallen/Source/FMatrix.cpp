#include "game.h"
#include "FMatrix.h"

#define ULTRA_DEBUG		0

void FMATRIX_calc(SLONG matrix[9], SLONG yaw, SLONG pitch, SLONG roll)
{
	SLONG cy, cp, cr;
	SLONG sy, sp, sr;

	if (pitch && roll)
	{
		cy = COS(yaw   & 2047);
		cp = COS(pitch & 2047);
		cr = COS(roll  & 2047);
		
		sy = SIN(yaw   & 2047);
		sp = SIN(pitch & 2047);
		sr = SIN(roll  & 2047);

		//
		// Jan I trust you... but only becuase I've already seen it working!
		//

		matrix[0] = MUL64( cy, cr) + MUL64(MUL64(sy, sp), sr);
		matrix[3] = MUL64( cy, sr) - MUL64(MUL64(sy, sp), cr);
		matrix[6] = MUL64( sy, cp);
		matrix[1] = MUL64(-cp, sr);
		matrix[4] = MUL64( cp, cr);
		matrix[7] = sp;
		matrix[2] = MUL64(-sy, cr) + MUL64(MUL64(cy, sp), sr);
		matrix[5] = MUL64(-sy, sr) - MUL64(MUL64(cy, sp), cr);
		matrix[8] = MUL64( cy, cp);
	}
	else if (pitch)
	{
		cy = COS(yaw   & 2047);
		cp = COS(pitch & 2047);
		cr = 65536;
		
		sy = SIN(yaw   & 2047);
		sp = SIN(pitch & 2047);
		sr = 0;

		matrix[0] = cy;
		matrix[3] = -MUL64(sy, sp);
		matrix[6] = MUL64(sy, cp);
		matrix[1] = 0;
		matrix[4] = cp;
		matrix[7] = sp;
		matrix[2] = -sy;
		matrix[5] = -MUL64(cy, sp);
		matrix[8] = MUL64(cy, cp);

#if ULTRA_DEBUG
		ASSERT(matrix[0] == MUL64( cy, cr) + MUL64(MUL64(sy, sp), sr));
		ASSERT(matrix[3] == MUL64( cy, sr) - MUL64(MUL64(sy, sp), cr));
		ASSERT(matrix[6] == MUL64( sy, cp));
		ASSERT(matrix[1] == MUL64(-cp, sr));
		ASSERT(matrix[4] == MUL64( cp, cr));
		ASSERT(matrix[7] == sp);
		ASSERT(matrix[2] == MUL64(-sy, cr) + MUL64(MUL64(cy, sp), sr));
		ASSERT(matrix[5] == MUL64(-sy, sr) - MUL64(MUL64(cy, sp), cr));
		ASSERT(matrix[8] == MUL64( cy, cp));
#endif
	}
	else if (roll)
	{
		cy = COS(yaw   & 2047);
		cp = 65536;
		cr = COS(roll  & 2047);
		
		sy = SIN(yaw   & 2047);
		sp = 0;
		sr = SIN(roll  & 2047);

		matrix[0] = MUL64( cy, cr);
		matrix[3] = MUL64( cy, sr);
		matrix[6] = sy;
		matrix[1] = -sr;
		matrix[4] = cr;
		matrix[7] = sp;
		matrix[2] = MUL64(-sy, cr);
		matrix[5] = MUL64(-sy, sr);
		matrix[8] = cy;

#if ULTRA_DEBUG
		ASSERT(matrix[0] == MUL64( cy, cr) + MUL64(MUL64(sy, sp), sr));
		ASSERT(matrix[3] == MUL64( cy, sr) - MUL64(MUL64(sy, sp), cr));
		ASSERT(matrix[6] == MUL64( sy, cp));
		ASSERT(matrix[1] == MUL64(-cp, sr));
		ASSERT(matrix[4] == MUL64( cp, cr));
		ASSERT(matrix[7] == sp);
		ASSERT(matrix[2] == MUL64(-sy, cr) + MUL64(MUL64(cy, sp), sr));
		ASSERT(matrix[5] == MUL64(-sy, sr) - MUL64(MUL64(cy, sp), cr));
		ASSERT(matrix[8] == MUL64( cy, cp));
#endif
	}
	else
	{
		cy = COS(yaw   & 2047);
		cp = 65536;
		cr = 65536;
		
		sy = SIN(yaw   & 2047);
		sp = 0;
		sr = 0;

		matrix[0] = cy;
		matrix[3] = 0;
		matrix[6] = sy;
		matrix[1] = 0;
		matrix[4] = 65536;
		matrix[7] = 0;
		matrix[2] = -sy;
		matrix[5] = 0;
		matrix[8] = cy;

#if ULTRA_DEBUG
		ASSERT(matrix[0] == MUL64( cy, cr) + MUL64(MUL64(sy, sp), sr));
		ASSERT(matrix[3] == MUL64( cy, sr) - MUL64(MUL64(sy, sp), cr));
		ASSERT(matrix[6] == MUL64( sy, cp));
		ASSERT(matrix[1] == MUL64(-cp, sr));
		ASSERT(matrix[4] == MUL64( cp, cr));
		ASSERT(matrix[7] == sp);
		ASSERT(matrix[2] == MUL64(-sy, cr) + MUL64(MUL64(cy, sp), sr));
		ASSERT(matrix[5] == MUL64(-sy, sr) - MUL64(MUL64(cy, sp), cr));
		ASSERT(matrix[8] == MUL64( cy, cp));
#endif
	}
}


void FMATRIX_vector(SLONG vector[3], SLONG yaw, SLONG pitch)
{
	SLONG cy, cp;
	SLONG sy, sp;

	sy = SIN(yaw   & 2047);
	sp = SIN(pitch & 2047);

	cy = COS(yaw   & 2047);
	cp = COS(pitch & 2047);

	vector[0] =  MUL64(sy, cp);
	vector[1] =  sp;
	vector[2] =  MUL64(cy, cp);
}

void	init_matrix33(struct Matrix33 *mat)
{
	mat->M[0][0] = (1<<15);
	mat->M[0][1] = 0;
	mat->M[0][2] = 0;
	mat->M[1][0] = 0;
	mat->M[1][1] = (1<<15);
	mat->M[1][2] = 0;
	mat->M[2][0] = 0;
	mat->M[2][1] = 0;
	mat->M[2][2] = (1<<15);
}
void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2)
{

//	ASSERT(SIGN(mat2->M[0] * trans->M[0][0])==SIGN(mat2->M[0])*SIGN(trans->M[0][0]));
//	ASSERT(SIGN(mat2->M[1] * trans->M[1][1])==SIGN(mat2->M[1])*SIGN(trans->M[1][1]));
//	ASSERT(SIGN(mat2->M[2] * trans->M[2][2])==SIGN(mat2->M[2])*SIGN(trans->M[2][2]));

	//LogText(" draw len before %d \n",SDIST3(mat2->M[0],mat2->M[1],mat2->M[2]));
	result->M[0] =  (mat2->M[0] * trans->M[0][0])+(mat2->M[1] * trans->M[0][1])+(mat2->M[2] * trans->M[0][2])>>15; 
	result->M[1] =  (mat2->M[0] * trans->M[1][0])+(mat2->M[1] * trans->M[1][1])+(mat2->M[2] * trans->M[1][2])>>15; 
	result->M[2] =  (mat2->M[0] * trans->M[2][0])+(mat2->M[1] * trans->M[2][1])+(mat2->M[2] * trans->M[2][2])>>15;
	//LogText(" draw len after %d \n",SDIST3(result->M[0],result->M[1],result->M[2]));
}

#ifndef	PSX
void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2)
{
	//LogText(" draw-a len before %d \n",SDIST3(mat2->M[0],mat2->M[1],mat2->M[2]));
	ASSERT(SIGN(mat2->M[0] * trans->M[0][0])==SIGN(mat2->M[0])*SIGN(trans->M[0][0]));
	ASSERT(SIGN(mat2->M[1] * trans->M[1][1])==SIGN(mat2->M[1])*SIGN(trans->M[1][1]));
	ASSERT(SIGN(mat2->M[2] * trans->M[2][2])==SIGN(mat2->M[2])*SIGN(trans->M[2][2]));
	result->M[0] = (mat2->M[0] * trans->M[0][0])+(mat2->M[1] * trans->M[0][1])+(mat2->M[2] * trans->M[0][2])>>15; 
	result->M[1] = (mat2->M[0] * trans->M[1][0])+(mat2->M[1] * trans->M[1][1])+(mat2->M[2] * trans->M[1][2])>>15; 
	result->M[2] = (mat2->M[0] * trans->M[2][0])+(mat2->M[1] * trans->M[2][1])+(mat2->M[2] * trans->M[2][2])>>15;
	//LogText(" draw-a len after %d \n",SDIST3(result->M[0],result->M[1],result->M[2]));
}

void matrix_transform_small(struct Matrix31* result, struct Matrix33* trans,struct  SMatrix31* mat2)
{
	//LogText(" draw-a len before %d \n",SDIST3(mat2->M[0],mat2->M[1],mat2->M[2]));
	ASSERT(SIGN(mat2->M[0] * trans->M[0][0])==SIGN(mat2->M[0])*SIGN(trans->M[0][0]));
	ASSERT(SIGN(mat2->M[1] * trans->M[1][1])==SIGN(mat2->M[1])*SIGN(trans->M[1][1]));
	ASSERT(SIGN(mat2->M[2] * trans->M[2][2])==SIGN(mat2->M[2])*SIGN(trans->M[2][2]));
	result->M[0] = (mat2->M[0] * trans->M[0][0])+(mat2->M[1] * trans->M[0][1])+(mat2->M[2] * trans->M[0][2])>>15; 
	result->M[1] = (mat2->M[0] * trans->M[1][0])+(mat2->M[1] * trans->M[1][1])+(mat2->M[2] * trans->M[1][2])>>15; 
	result->M[2] = (mat2->M[0] * trans->M[2][0])+(mat2->M[1] * trans->M[2][1])+(mat2->M[2] * trans->M[2][2])>>15;
	//LogText(" draw-a len after %d \n",SDIST3(result->M[0],result->M[1],result->M[2]));
}



void	normalise_matrix(struct Matrix33 *mat)
{
	SLONG c0;

	for(c0=0;c0<3;c0++)
	{
		SLONG size;
		size =(mat->M[0][c0]*mat->M[0][c0]);
		size+=(mat->M[1][c0]*mat->M[1][c0]);
		size+=(mat->M[2][c0]*mat->M[2][c0]);
		size=Root(size);
		if(size==0)
			size=1;
		mat->M[0][c0]=(mat->M[0][c0]<<15)/size;
		mat->M[1][c0]=(mat->M[1][c0]<<15)/size;
		mat->M[2][c0]=(mat->M[2][c0]<<15)/size;
	}
}

// JCL - use *this* one to normalise a linearly interpolated rotation matrix...
void	normalise_matrix_rows(struct Matrix33 *mat)
{
	SLONG c0;

	for(c0=0;c0<3;c0++)
	{
		SLONG size;
		size =(mat->M[c0][0]*mat->M[c0][0]);
		size+=(mat->M[c0][1]*mat->M[c0][1]);
		size+=(mat->M[c0][2]*mat->M[c0][2]);
		size=Root(size);
		if(size==0)
			size=1;
		mat->M[c0][0]=(mat->M[c0][0]<<15)/size;
		mat->M[c0][1]=(mat->M[c0][1]<<15)/size;
		mat->M[c0][2]=(mat->M[c0][2]<<15)/size;
	}
}
#endif

#define	MAT_SHIFT	(6)
#define	MAT_SHIFTD	(8-MAT_SHIFT)




void	build_tween_matrix(struct Matrix33 *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween)
{
	SLONG	v,w;

	v=((cmat1->M[0]&CMAT0_MASK)<<2)>>22;
	w=((cmat2->M[0]&CMAT0_MASK)<<2)>>22;

	mat->M[0][0]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

	v=((cmat1->M[0]&CMAT1_MASK)<<12)>>22;
	w=((cmat2->M[0]&CMAT1_MASK)<<12)>>22;


	mat->M[0][1]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

	v=((cmat1->M[0]&CMAT2_MASK)<<22)>>22;
	w=((cmat2->M[0]&CMAT2_MASK)<<22)>>22;

	mat->M[0][2]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);




	v=((cmat1->M[1]&CMAT0_MASK)<<2)>>22;
	w=((cmat2->M[1]&CMAT0_MASK)<<2)>>22;

	mat->M[1][0]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

	v=((cmat1->M[1]&CMAT1_MASK)<<12)>>22;
	w=((cmat2->M[1]&CMAT1_MASK)<<12)>>22;

	mat->M[1][1]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

	v=((cmat1->M[1]&CMAT2_MASK)<<22)>>22;
	w=((cmat2->M[1]&CMAT2_MASK)<<22)>>22;

	mat->M[1][2]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);




	v=((cmat1->M[2]&CMAT0_MASK)<<2)>>22;
	w=((cmat2->M[2]&CMAT0_MASK)<<2)>>22;

	mat->M[2][0]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

	v=((cmat1->M[2]&CMAT1_MASK)<<12)>>22;
	w=((cmat2->M[2]&CMAT1_MASK)<<12)>>22;

	mat->M[2][1]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

	v=((cmat1->M[2]&CMAT2_MASK)<<22)>>22;
	w=((cmat2->M[2]&CMAT2_MASK)<<22)>>22;

	mat->M[2][2]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD);

}
#ifdef	PSX
#endif

void	FMATRIX_find_angles(SLONG *matrix,SLONG *yaw,SLONG *pitch, SLONG *roll)
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG xz;


	//
	// Look from above at the z-vector to work out the yaw.
	//

	x = matrix[6];
	y = matrix[7];
	z = matrix[8];

	if (abs(x) + abs(z) ==0)
	{
		*yaw = 0;
	}
	else
	{
	   *yaw = 1024-Arctan(x, z);
	}

	//
	// Look down the x vector to at the z-vector to work out the pitch.
	//

	xz = Root( (((x>>1)*(x>>1))>>14) + (((z>>1)*(z>>1))>>14))<<8;

	if (abs(xz) + abs(y) == 0)  //MATRIX_FA_VECTOR_TOO_SMALL)
	{
		if (y < 0) {*pitch = -1024;} else {*pitch = +1024;}
	}
	else
	{
		*pitch = 1024-Arctan(y, xz);
	}

	//
	// Now... matrix[4] =  cos(pitch) * cos(roll)
	//		  matrix[1] = -cos(pitch) * sin(roll)
	//
	// so...  cos(roll) = matrix[4] /  cos(pitch)
	//        sin(roll) = matrix[1] / -cos(pitch)
	//
	
	SLONG cos_roll;
	SLONG sin_roll;
	SLONG cos_pitch;

	*pitch=(*pitch+2048)&2047;
	cos_pitch = COS(*pitch);

	if (abs(cos_pitch) ==0)// MATRIX_FA_ANGLE_TOO_SMALL)
	{
		*roll = 0; //0.0F;
	}
	else
	{
		SLONG	temp_pitch;
		if(cos_pitch<0)
		{
			temp_pitch=-((-cos_pitch)>>2);
		}
		else
		{
			temp_pitch=-(cos_pitch>>2);
		}

		


		cos_roll = (matrix[4]<<14) /  cos_pitch;
		sin_roll = (matrix[1]<<14) / -cos_pitch;

		cos_roll<<=2;
		sin_roll<<=2;

		*roll = 1024-Arctan(sin_roll, cos_roll);
	}

	*roll=(*roll+2048)&2047;
	*pitch=(*pitch+2048)&2047;

}
