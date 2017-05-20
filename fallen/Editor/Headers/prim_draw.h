#ifndef	PRIM_DRAW_H
#define	PRIM_DRAW_H	1

//**********
//*  DATA  *
//**********



//***************
//*  FUNCTIONS  *
//***************

extern	void	draw_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,UBYTE shade);
extern	void	draw_a_multi_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z);
extern	void	draw_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,UBYTE shade);
extern	void	draw_a_rot_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct	PrimMultiAnim *anim_info,struct Matrix33 *rot_mat);

extern	void	init_matrix33(struct Matrix33 *mat);
extern	void 	matrix_mult33(struct Matrix33* result,struct Matrix33* mat1,struct  Matrix33* mat2);
extern	void 	rotate_obj(SWORD xangle,SWORD yangle,SWORD zangle, Matrix33 *r3);


#endif
