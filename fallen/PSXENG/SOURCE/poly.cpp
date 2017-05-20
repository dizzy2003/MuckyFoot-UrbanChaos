//
// Drawing polygons with D3D
//

#include <MFStdLib.h>
//#include <math.h>
#include "c:\fallen\psxeng\headers\engine.h"
#include "c:\fallen\headers\eway.h"
//#include "c:\fallen\psxeng\headers\texture.h"	 
//#include "message.h"
								 
//
// The handy buffer
//
inline SLONG POLY_backface_cull(POLY_Point *pp1, POLY_Point *pp2, POLY_Point *pp3);
//SLONG      POLY_buffer_upto;
//POLY_Point POLY_buffer[POLY_BUFFER_SIZE];
//PSX_POLY_Point PSX_POLY_buffer[POLY_BUFFER_SIZE];
//POLY_Point POLY_shadow[POLY_SHADOW_SIZE];
//SLONG      POLY_shadow_upto;

/*
POLY_Point POLY_shadow[POLY_SHADOW_SIZE];
SLONG      POLY_shadow_upto;
*/

//
// The vertex buffers for each texture page.
//

typedef struct
{
	SLONG        size;
//	D3DTLVERTEX *vertex;
	SLONG        vertex_upto;
	UWORD       *index;
	SLONG        index_upto;

} POLY_Page;

//POLY_Page POLY_page[16];

SLONG	mylens=420;

//
// Only points further than 1/256'th of the draw range are drawn.
//

#define POLY_ZCLIP_PLANE ((SLONG)(0.00781250F*65536)) // i.e. 1/128
#define POLY_ZCLIP_PLANE_RECIP_FIX0 (128) 
//
// The camera and the screen.
//

SLONG POLY_cam_x;
SLONG POLY_cam_y;
SLONG POLY_cam_z;
SLONG POLY_cam_yaw;

FIX_16 POLY_cam_aspect;
FIX_16 POLY_cam_lens;
SLONG POLY_cam_view_dist;
FIX_16 POLY_cam_over_view_dist;
SLONG POLY_cam_matrix[9];

SLONG POLY_screen_width;
SLONG POLY_screen_height;
SLONG POLY_screen_mid_x;
SLONG POLY_screen_mid_y;
SLONG POLY_screen_mul_x_fix0;
SLONG POLY_screen_mul_y_fix0;

MATRIX	PSX_view_matrix;

void POLY_init(void)
{
}

void	build_rot_matrix(SLONG yaw,SLONG pitch,MATRIX *m)
{
	SVECTOR r;

	r.vx=((pitch+2048)&2047)<<1;
	r.vy=((yaw+2048)&2047)<<1;
	r.vz=0;
/*
	m->m[0][0]=1<<12;
	m->m[0][1]=0;
	m->m[0][2]=0;

	m->m[1][0]=0;
	m->m[1][1]=1<<12;
	m->m[1][2]=0;

	m->m[2][0]=0;
	m->m[2][1]=0;
	m->m[2][2]=1<<12;
*/
	RotMatrixYXZ_gte(&r,m);

//	m->t[0]=0;
//	m->t[1]=0;
//	m->t[2]=0;
}

void	build_rot_matrix3(SLONG yaw,SLONG pitch,SLONG roll,MATRIX *m)
{
	SVECTOR r;

	r.vx=((pitch+2048)&2047)<<1;
	r.vy=((yaw+2048)&2047)<<1;
	r.vz=((roll+2048)&2047)<<1;
	RotMatrixYXZ_gte(&r,m);

}

void POLY_camera_set(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,
		SLONG view_dist,
		SLONG lens)
{
	SVECTOR	angles;
	MATRIX	temp1,temp2;
	POLY_screen_width  = SLONG(DisplayWidth);
	POLY_screen_height = SLONG(DisplayHeight);
	POLY_screen_mid_x  = POLY_screen_width  >>1;
	POLY_screen_mid_y  = POLY_screen_height >>1;

	yaw=(yaw+2048)&2047;
	pitch=(pitch+2048)&2047;

	POLY_cam_yaw=yaw;

//	angles.vy=yaw<<1;
//	angles.vx=-pitch<<1;
//	angles.vz=0;//roll<<1;

//	RotMatrixYXZ_gte(&angles,&PSX_view_matrix);

	PSX_view_matrix.m[0][0]=1<<12;
	PSX_view_matrix.m[0][1]=0;
	PSX_view_matrix.m[0][2]=0;

	PSX_view_matrix.m[1][0]=0;
	PSX_view_matrix.m[1][1]=1<<12;
	PSX_view_matrix.m[1][2]=0;

	PSX_view_matrix.m[2][0]=0;
	PSX_view_matrix.m[2][1]=0;
	PSX_view_matrix.m[2][2]=1<<12;



	RotMatrixY(yaw<<1,&PSX_view_matrix);
	RotMatrixX(pitch<<1,&PSX_view_matrix);
//	MulMatrix0(&temp2,&temp1,&PSX_view_matrix);

//	RotMatrixYXZ(&angles,&PSX_view_matrix);
//	RotMatrixZYX(&angles,&PSX_view_matrix);



	PSX_view_matrix.t[0]=0;
	PSX_view_matrix.t[1]=0;
	PSX_view_matrix.t[2]=0;

#ifdef	VERSION_NTSC
	PSX_view_matrix.m[0][0]=(PSX_view_matrix.m[0][0]*DISPLAYWIDTH)/(400-64); //was320
	PSX_view_matrix.m[0][1]=(PSX_view_matrix.m[0][1]*DISPLAYWIDTH)/(400-64);
	PSX_view_matrix.m[0][2]=(PSX_view_matrix.m[0][2]*DISPLAYWIDTH)/(400-64);
#else

	{


		PSX_view_matrix.m[0][0]=(PSX_view_matrix.m[0][0]*DISPLAYWIDTH)/(400); //was320
		PSX_view_matrix.m[0][1]=(PSX_view_matrix.m[0][1]*DISPLAYWIDTH)/(400);
		PSX_view_matrix.m[0][2]=(PSX_view_matrix.m[0][2]*DISPLAYWIDTH)/(400);
	}
#endif


	gte_SetRotMatrix(&PSX_view_matrix);
	gte_SetTransMatrix(&PSX_view_matrix);

	POLY_cam_x = x;
	POLY_cam_y = y;
	POLY_cam_z = z;

	
}

#ifndef PSX
void POLY_camera_set_old(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,
		SLONG view_dist,
		SLONG lens)
{

//	yaw  =(yaw+2048)&2047;
//	pitch=(pitch+2048)&2047;
//	roll =(roll+2048)&2047;

	POLY_screen_width  = SLONG(DisplayWidth);
	POLY_screen_height = SLONG(DisplayHeight);
	POLY_screen_mid_x  = POLY_screen_width  >>1;
	POLY_screen_mid_y  = POLY_screen_height >>1;

	//
	// POLY_ZCLIP_PLANE <1.0
	//
//	POLY_screen_mul_x  = ((POLY_screen_width  <<20) / (POLY_ZCLIP_PLANE))<<12;
//	POLY_screen_mul_y  = ((POLY_screen_height <<20) / (POLY_ZCLIP_PLANE))<<12;
	POLY_screen_mul_x_fix0  = POLY_screen_width*POLY_ZCLIP_PLANE_RECIP_FIX0;
	POLY_screen_mul_y_fix0  = POLY_screen_height*POLY_ZCLIP_PLANE_RECIP_FIX0;

	POLY_cam_x = x;
	POLY_cam_y = y;
	POLY_cam_z = z;

	POLY_cam_lens           = lens; //3.3*65536
	POLY_cam_view_dist      = view_dist; //22
	POLY_cam_over_view_dist = 65536 / view_dist;
	POLY_cam_aspect         = (POLY_screen_height<<16) / POLY_screen_width;

	MATRIX_calc(
		POLY_cam_matrix,
		yaw,
		pitch,
		roll);

	MATRIX_skew(
		POLY_cam_matrix,
		POLY_cam_aspect,
		POLY_cam_lens,
		POLY_cam_over_view_dist);	// Shrink the matrix down so the furthest point has a view distance z of 1.0F
}
#endif

#if 0
void POLY_transform(
		SLONG       world_x,
		SLONG       world_y,
		SLONG       world_z,
		POLY_Point *pt)
{
	SVECTOR	input;
	VECTOR	output;
	SLONG	sysx,p,otz;
	SLONG	flag;

	pt->x = world_x;// - POLY_cam_x; //just lighting at the mo
	pt->y = world_y;// - POLY_cam_y;
	pt->z = world_z;// - POLY_cam_z;

	input.vx=world_x - POLY_cam_x;
	input.vy=world_y - POLY_cam_y;
	input.vz=world_z - POLY_cam_z;

	gte_RotTransPers(&input,&sysx,&p,&flag,&otz);
	flag&=~(1<<12);
	pt->Y=sysx>>16;
	pt->X=(sysx<<16)>>16;
	pt->Z=otz<<2;

//		RotTrans(&input,&output,&flag);
/*
	pt->X=output.vx;	
	pt->Y=output.vy;	
	pt->Z=output.vz;	
*/

	if(pt->Z<=0||pt->Z>40000||flag)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	{
		//pt->Z = otz; //fix32

//		pt->X = (sxsy&0xffff0000)>>16;
//		pt->Y = (sxsy&0xffff);

		//200  should be 3.3 * 160

//		pt->X=POLY_screen_mid_x+((pt->X*mylens)/pt->Z);
//		pt->Y=POLY_screen_mid_y+((pt->Y*mylens)/pt->Z);

		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < 0) 
		{
			pt->clip |= POLY_CLIP_LEFT;
		}
		else
		if (pt->X > POLY_screen_width) 
		{
			pt->clip |= POLY_CLIP_RIGHT;
		}

		if (pt->Y < 0) 
		{
			pt->clip |= POLY_CLIP_TOP;
		}
		else
		if (pt->Y > POLY_screen_height) 
		{
			pt->clip |= POLY_CLIP_BOTTOM;
		}
	}



	
}
#endif
#ifdef WHAT_THE_FUCK_IS_THIS_DOING_HERE
void POLY_transform_old(
		SLONG       world_x,
		SLONG       world_y,
		SLONG       world_z,
		POLY_Point *pt)
{
	pt->x = world_x - POLY_cam_x;
	pt->y = world_y - POLY_cam_y;
	pt->z = world_z - POLY_cam_z;

/*	MATRIX_MUL(
		POLY_cam_matrix,
		pt->x,
		pt->y,
		pt->z);
*/

	MATRIX24_MUL(
		POLY_cam_matrix,
		pt->x,
		pt->y,
		pt->z);

	if (pt->z < POLY_ZCLIP_PLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	if (pt->z > (1<<16))
	{
		pt->clip = POLY_CLIP_FAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		//
		// GOD DAMN TRICKY,  
		//

		// pt->Z >0 in FIX_16

		pt->Z = (POLY_ZCLIP_PLANE<<16) / pt->z; //fix32

		pt->X = POLY_screen_mid_x - ((((POLY_screen_mul_x_fix0 * POLY_ZCLIP_PLANE)/pt->z)*pt->x)>>16);
		pt->Y = POLY_screen_mid_y - ((((POLY_screen_mul_y_fix0 * POLY_ZCLIP_PLANE)/pt->z)*pt->y)>>16);

//		pt->X = POLY_screen_mid_x - (((((POLY_screen_mul_x_fix0 * pt->x)) * pt->Z)>>16))>>16;
//		pt->Y = POLY_screen_mid_y - (((((POLY_screen_mul_y_fix0 * pt->y)) * pt->Z)>>16))>>16;

		//
		// Set the clipping flags.
		//

		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < 0) {pt->clip |= POLY_CLIP_LEFT;}
		else
		if (pt->X > POLY_screen_width) {pt->clip |= POLY_CLIP_RIGHT;}

		if (pt->Y < 0) {pt->clip |= POLY_CLIP_TOP;}
		else
		if (pt->Y > POLY_screen_height) {pt->clip |= POLY_CLIP_BOTTOM;}
	}
}
#endif
//
// The combined rotation matrix.
//

SLONG POLY_cam_matrix_comb[9];	 
SLONG POLY_cam_off_x;
SLONG POLY_cam_off_y;
SLONG POLY_cam_off_z;

void POLY_set_local_rotation(	 
		SLONG off_x,
		SLONG off_y,
		SLONG off_z,
		MATRIX *matrix)
{
	ULONG	flag;
	SVECTOR	input;
	VECTOR	output;
//	MATRIX	view_trans;

//transposeMatrix(&PSX_view_matrix,&view_trans);
	gte_SetRotMatrix(&PSX_view_matrix);


	input.vx = off_x - POLY_cam_x;
	input.vy = off_y - POLY_cam_y;
	input.vz = off_z - POLY_cam_z;


	gte_RotTrans(&input,&output,&flag);

	POLY_cam_off_x = output.vx;
	POLY_cam_off_y = output.vy;
	POLY_cam_off_z = output.vz;

	PSX_view_matrix.t[0]=POLY_cam_off_x;
	PSX_view_matrix.t[1]=POLY_cam_off_y;
	PSX_view_matrix.t[2]=POLY_cam_off_z;


	SetMulMatrix(&PSX_view_matrix,matrix);
	gte_SetTransMatrix(&PSX_view_matrix);

/*
	MATRIX_3x3mul(
		POLY_cam_matrix_comb,
		POLY_cam_matrix,
		matrix);
*/
}

void POLY_transform_using_local_rotation(
		SLONG       local_x,
		SLONG       local_y,
		SLONG       local_z,
		POLY_Point *pt)
{
	SLONG	flag;
	SVECTOR	input;
	VECTOR	output;
	SLONG	sysx,p,otz;

	input.vx = local_x;
	input.vy = local_y;
	input.vz = local_z;

/*
	MATRIX_MUL(
		POLY_cam_matrix_comb,
		pt->x,
		pt->y,
		pt->z);
		*/


	gte_RotTransPers(&input,&sysx,&p,&flag,&otz);
	pt->Y=sysx>>16;
	pt->X=(sysx<<16)>>16;
	pt->Z=otz<<2;
	

//	RotTrans(&input,&output,&flag);


//	pt->X = output.vx;//+POLY_cam_off_x;
//	pt->Y = output.vy;//+POLY_cam_off_y;
//	pt->Z = output.vz;//+POLY_cam_off_z;


	if(pt->Z<=0||pt->Z>40000||(flag&(1<<31)))
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	{
		//pt->Z = otz; //fix32

//		pt->X = (sxsy&0xffff0000)>>16;
//		pt->Y = (sxsy&0xffff);

		//200  should be 3.3 * 160

//		pt->X=POLY_screen_mid_x+((pt->X*mylens)/pt->Z);
//		pt->Y=POLY_screen_mid_y+((pt->Y*mylens)/pt->Z);

		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < 0) 
		{
			pt->clip |= POLY_CLIP_LEFT;
		}
		else
		if (pt->X > POLY_screen_width) 
		{
			pt->clip |= POLY_CLIP_RIGHT;
		}

		if (pt->Y < 0) 
		{
			pt->clip |= POLY_CLIP_TOP;
		}
		else
		if (pt->Y > POLY_screen_height) 
		{
			pt->clip |= POLY_CLIP_BOTTOM;
		}
	}


/*
	if (pt->z < POLY_ZCLIP_PLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	if (pt->z > 1.0F)
	{
		pt->clip = POLY_CLIP_FAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

		pt->X = POLY_screen_mid_x - (((POLY_screen_mul_x_fix0 * pt->x) * pt->Z)>>16);
		pt->Y = POLY_screen_mid_y - (((POLY_screen_mul_y_fix0 * pt->y) * pt->Z)>>16);

		//
		// Set the clipping flags.
		//

		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < 0) {pt->clip |= POLY_CLIP_LEFT;}
		else
		if (pt->X > POLY_screen_width) {pt->clip |= POLY_CLIP_RIGHT;}

		if (pt->Y < 0) {pt->clip |= POLY_CLIP_TOP;}
		else
		if (pt->Y > POLY_screen_height) {pt->clip |= POLY_CLIP_BOTTOM;}
	}
	*/
}

SLONG POLY_sphere_visible(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG radius)
{
	SLONG view_x;
	SLONG view_y;
	SLONG view_z;

	//
	// Rotate into viewspace.
	//

	view_x = world_x - POLY_cam_x;
	view_y = world_y - POLY_cam_y;
	view_z = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		view_x,
		view_y,
		view_z);

	if (view_z + radius <=  POLY_ZCLIP_PLANE ||
		view_x - radius >= +view_z           ||
		view_x + radius <= -view_z           ||
		view_y - radius >= +view_z           ||
		view_y + radius <= -view_z)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


void POLY_fadeout_buffer()
{
	SLONG i;
/*
	for (i = 0; i < POLY_buffer_upto; i++)
	{
		POLY_fadeout_point(&POLY_buffer[i]);
	}
	*/
}


#if 0
SLONG POLY_valid_trianglep(POLY_Point *pp[3],UWORD back_cull)
{
	ULONG flag_and = pp[0]->clip & pp[1]->clip & pp[2]->clip;

	if (!(flag_and & POLY_CLIP_TRANSFORMED))
	{
		return FALSE;
	}

	if (flag_and & (POLY_CLIP_LEFT|POLY_CLIP_RIGHT|POLY_CLIP_TOP|POLY_CLIP_BOTTOM|POLY_CLIP_NEAR|POLY_CLIP_FAR))
	{
		return FALSE;
	}

	if(back_cull)
	if(!POLY_backface_cull(pp[0],pp[1],pp[2]))
	{
		return FALSE;
	}

	return TRUE;
}

SLONG POLY_valid_quadp(POLY_Point *pp[4],UWORD back_cull)
{
	ULONG flag_and = pp[0]->clip & pp[1]->clip & pp[2]->clip & pp[3]->clip;

	if (!(flag_and & POLY_CLIP_TRANSFORMED))
	{
		return FALSE;
	}

	if (flag_and & (POLY_CLIP_LEFT|POLY_CLIP_RIGHT|POLY_CLIP_TOP|POLY_CLIP_BOTTOM|POLY_CLIP_NEAR|POLY_CLIP_FAR))
	{
		return FALSE;
	}

	if(back_cull)
	if(!POLY_backface_cull(pp[0],pp[1],pp[2]))
	{
		return FALSE;
	}

	return TRUE;
}

SLONG POLY_valid_triangle(POLY_Point *pp)
{
	ULONG flag_and;
	flag_and = pp[0].clip & pp[1].clip & pp[2].clip;

	if (!(flag_and & POLY_CLIP_TRANSFORMED))
	{
		return FALSE;
	}

	if (flag_and & (POLY_CLIP_LEFT|POLY_CLIP_RIGHT|POLY_CLIP_TOP|POLY_CLIP_BOTTOM|POLY_CLIP_NEAR|POLY_CLIP_FAR))
	{
		return FALSE;
	}

	return TRUE;
}

SLONG POLY_valid_quad(POLY_Point *pp)
{
	ULONG flag_and;
	flag_and = pp[0].clip & pp[1].clip & pp[2].clip & pp[3].clip;

	if (!(flag_and & POLY_CLIP_TRANSFORMED))
	{
		return FALSE;
	}

	if (flag_and & (POLY_CLIP_LEFT|POLY_CLIP_RIGHT|POLY_CLIP_TOP|POLY_CLIP_BOTTOM|POLY_CLIP_NEAR|POLY_CLIP_FAR))
	{
		return FALSE;
	}

	return TRUE;
}

SLONG POLY_valid_line(POLY_Point *p1, POLY_Point *p2)
{
	ULONG flag_and = p1->clip & p2->clip;

	if (!(flag_and & POLY_CLIP_TRANSFORMED))
	{
		return FALSE;
	}
	
	//
	// Do no clip rejection, because we don't know the widths of the line.
	//

	return TRUE;
}

inline SLONG POLY_backface_cull(POLY_Point *pp1, POLY_Point *pp2, POLY_Point *pp3)
{
	SLONG vx1;
	SLONG vy1;

	SLONG vx2;
	SLONG vy2;

	SLONG cprod;

	ASSERT((pp1->clip & pp2->clip & pp3->clip) & POLY_CLIP_TRANSFORMED);

	vx1 = pp2->X - pp1->X;
	vy1 = pp2->Y - pp1->Y;

	vx2 = pp3->X - pp1->X;
	vy2 = pp3->Y - pp1->Y;

	cprod = vx1*vy2 - vy1*vx2;

	return (cprod < 0);
}
#endif
void  POLY_add_line_tex(PSX_POLY_Point *p1, PSX_POLY_Point *p2, SLONG width1, SLONG width2,SLONG page, ULONG colour, SLONG sort_to_front)
{
	SLONG dx;
	SLONG dy;

	SLONG dx1;
	SLONG dy1;

	SLONG dx2;
	SLONG dy2;

	SLONG sw1;
	SLONG sw2;

	SLONG len;
//	SLONG overlen;

	UBYTE u,v;

	POLY_FT4	*p;

//	p=(POLY_FT4*)the_display.CurrentPrim;
//	the_display.CurrentPrim+=sizeof(POLY_FT4);

	dx = p2->Word.SX - p1->Word.SX;
	dy = p2->Word.SY - p1->Word.SY;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	len     = (abs(dx) > abs(dy)) ? abs(dx) +  (abs(dy)>>1) : abs(dy) + (abs(dx)>>1);

	if (len==0)
		return;
	//
	// Convert widths in the world to widths in view space.
	//
	
	ALLOCPRIM(p,POLY_FT4);

	//
	// Convert widths in view space to widths on screen.
	//

	sw1 = (width1<<7) / (420+p1->Z);
	sw2 = (width2<<7) / (420+p2->Z);

	dx1 = (-dy * sw1) / len;
	dy1 = (+dx * sw1) / len;

	dx2 = (-dy * sw2) / len;
	dy2 = (+dx * sw2) / len;

	if (sort_to_front)
		p1->Z = PANEL_OTZ;
	else
		p1->Z = get_z_sort(p1->Z>>0);

	
	setPolyFT4(p);

	setXY4(p, p1->Word.SX + dx1, p1->Word.SY + dy1,
			  p1->Word.SX - dx1, p1->Word.SY - dy1,
			  p2->Word.SX + dx2, p2->Word.SY + dy2,
			  p2->Word.SX - dx2, p2->Word.SY - dy2);
	setSemiTrans(p,1);

	u=getPSXU(page);
	v=getPSXV(page);

	setUV4(p,u,v,u+31,v,u,v+31,u+31,v+31);
	setRGB0(p,colour>>16,(colour>>8)&0xff,colour&0xff);
	p->tpage=getPSXTPageE(page);

	if (colour&0xff000000) 
		p->tpage=((p->tpage)&~(3<<5))|(2<<5);

	p->clut=getPSXClutE(page);

	DOPRIM(p1->Z,p);

}

void  POLY_add_line_tex_uv(PSX_POLY_Point *p1, PSX_POLY_Point *p2, SLONG width1, SLONG width2,SLONG page, ULONG colour, SLONG sort_to_front)
{
	SLONG dx;
	SLONG dy;

	SLONG dx1;
	SLONG dy1;

	SLONG dx2;
	SLONG dy2;

	SLONG sw1;
	SLONG sw2;

	SLONG len;
//	SLONG overlen;

	UBYTE u,v;

	POLY_FT4	*p;


	dx = p2->Word.SX - p1->Word.SX;
	dy = p2->Word.SY - p1->Word.SY;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

//	len     = (abs(dx) > abs(dy)) ? abs(dx) +  (abs(dy)>>1) : abs(dy) + (abs(dx)>>1);
	len=Root(dx*dx+dy*dy);

	if (len==0)
		return;
	ALLOCPRIM(p,POLY_FT4);
	//
	// Convert widths in the world to widths in view space.
	//


	//
	// Convert widths in view space to widths on screen.
	//

	sw1 = (width1<<7) / (420+p1->Z);
	sw2 = (width2<<7) / (420+p2->Z);

	dx1 = (-dy * sw1) / len;
	dy1 = (+dx * sw1) / len;

	dx2 = (-dy * sw2) / len;
	dy2 = (+dx * sw2) / len;

	if (sort_to_front)
		p1->Z = PANEL_OTZ;
	else
		p1->Z = get_z_sort(p1->Z>>0);

	setPolyFT4(p);

	setXY4(p, p1->Word.SX + dx1, p1->Word.SY + dy1,
			  p1->Word.SX - dx1, p1->Word.SY - dy1,
			  p2->Word.SX + dx2, p2->Word.SY + dy2,
			  p2->Word.SX - dx2, p2->Word.SY - dy2);

	setSemiTrans(p,1);

	u=getPSXU(page);
	v=getPSXV(page);

	setUV4(p,u+p1->World.vx,v+p1->World.vy,
			 u+p2->World.vx,v+p1->World.vy,
			 u+p1->World.vx,v+p2->World.vy,
			 u+p2->World.vx,v+p2->World.vy);

	setRGB0(p,colour>>16,(colour>>8)&0xff,colour&0xff);
	p->tpage=getPSXTPageE(page);

//	if (colour&0xff000000) 
//		p->tpage=((p->tpage)&~(3<<5))|(2<<5);

	p->clut=getPSXClutE(page);

	DOPRIM(p1->Z,p);

}


void POLY_add_line(POLY_Point *p1, POLY_Point *p2, SLONG width1, SLONG width2, SLONG sort_to_front)
{
}

void  POLY_add_line_2d(SLONG sx1, SLONG sy1, SLONG sx2, SLONG sy2, ULONG colour)
{
}

void POLY_frame_draw(SLONG draw_shadow_page)
{
}
