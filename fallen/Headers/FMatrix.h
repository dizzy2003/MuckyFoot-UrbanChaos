//
// Matrix stuff...
//

#ifndef _FMATRIX_
#define _FMATRIX_

//
// All angles go from 0 - 2047.
//

//
// Fills in the matrix appropriately given the three angles (yaw, pitch, roll)
// for the eye along the z-axis.
//

void FMATRIX_calc  (SLONG matrix[9], SLONG yaw, SLONG pitch, SLONG roll);
void FMATRIX_vector(SLONG matrix[3], SLONG yaw, SLONG pitch);
void FMATRIX_find_angles(SLONG matrix[9],SLONG *yaw,SLONG *pitch, SLONG *roll);

void build_tween_matrix(struct Matrix33 *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween);
void init_matrix33(struct Matrix33 *mat);
void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);
void matrix_transform_small(struct Matrix31* result, struct Matrix33* trans,struct  SMatrix31* mat2);
void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
void normalise_matrix(struct Matrix33 *mat);
void normalise_matrix_rows(struct Matrix33 *mat);

#ifdef	PSX
//void build_tween_matrix_psx(MATRIX *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween);
#endif

// Multiplies points x,y,z by matrix m.
//

#define FMATRIX_MUL(m,x,y,z) 	   		\
{										\
	SLONG xnew, ynew, znew;	   			\
							   			\
	xnew  = MUL64((x), (m)[0]);	   		\
	ynew  = MUL64((x), (m)[3]);	   		\
	znew  = MUL64((x), (m)[6]);	   		\
								   		\
	xnew += MUL64((y), (m)[1]);	   		\
	ynew += MUL64((y), (m)[4]);	   		\
	znew += MUL64((y), (m)[7]);	   		\
								   		\
	xnew += MUL64((z), (m)[2]);	   		\
	ynew += MUL64((z), (m)[5]);	   		\
	znew += MUL64((z), (m)[8]);	   		\
							   			\
	(x) = xnew; (y) = ynew; (z) = znew;	\
}

//
// Multiplies points x,y,z by the transpose of matrix m.
//

#define FMATRIX_MUL_BY_TRANSPOSE(m,x,y,z)	\
{											\
	SLONG xnew, ynew, znew;	   				\
							   				\
	xnew  = MUL64((x), (m)[0]);   			\
	ynew  = MUL64((x), (m)[1]);	   			\
	znew  = MUL64((x), (m)[2]);			   	\
							 		   		\
	xnew += MUL64((y), (m)[3]);			   	\
	ynew += MUL64((y), (m)[4]);			   	\
	znew += MUL64((y), (m)[5]);			   	\
							 		   		\
	xnew += MUL64((z), (m)[6]);			   	\
	ynew += MUL64((z), (m)[7]);			   	\
	znew += MUL64((z), (m)[8]);			   	\
							   				\
	(x) = xnew; (y) = ynew; (z) = znew;		\
}



//
// Transposes the matrix m.
//

#define FMATRIX_TRANSPOSE(m) {SWAP(m[1], m[3]); SWAP(m[2], m[6]); SWAP(m[5], m[7]);}


void FMATRIX_vector(SLONG vector[3], SLONG yaw, SLONG pitch);


#endif
