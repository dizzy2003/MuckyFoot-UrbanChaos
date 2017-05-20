//
// Draw a person.
// 

#include "game.h"
#include	"c:\fallen\headers\memory.h"
#include "c:\fallen\psxeng\headers\engine.h"
//#include "c:\fallen\editor\headers\poly.h"
//#include "light.h"
#include "figure.h"
#include	"c:\fallen\headers\statedef.h"
#include	"c:\fallen\headers\mav.h"

#include "c:\fallen\headers\fmatrix.h"
#include "c:\fallen\headers\interact.h"
#include "c:\fallen\headers\animate.h"
#include "libpad.h"

// For Filtering we need the pad input crap here.

#include	"ctrller.h"

//SLONG	highest=0;
extern	ULONG	available_bucket_ram(void);

extern ControllerPacket PAD_Input1,PAD_Input2;

//void FIGURE_draw(Thing *p_thing);

extern UWORD floor_psx_col[128][128];
extern PSX_POLY_Point *perm_pp_array;

int ware_flag;
extern	void	check_prim_ptr_ni(void **x);

//extern	UWORD	debug_count[10];

#define	RED(col)	(((col) >> 8) & 0xfc)
#define	GREEN(col)	(((col) >> 2) & 0xf8)
#define	BLUE(col)	(((col) << 3) & 0xf8)

#define	MAX_STEAM	100
						   
extern SLONG	steam_seed;
extern	void	fuck_z(PSX_POLY_Point *pp);

extern SLONG	get_steam_rand(void**);
#define	MAT_SHIFT	(6)
#define	MAT_SHIFTD	(8-MAT_SHIFT)

void	build_peep_rot_matrix(SLONG yaw,SLONG roll,MATRIX *m)
{
	SVECTOR r;

	r.vz=((roll+2048)&2047)<<1;
	r.vy=((yaw+2048)&2047)<<1;
	r.vx=0;
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

void	draw_steam(SLONG x,SLONG y,SLONG z,SLONG lod)
{
#ifndef PSX
	SLONG	c0;
	SLONG	u,v;
	SLONG	trans;
	//
	// waft gently up from x,y,z
	//

	steam_seed=54321678;
	if(lod>32)
		lod=32;

	for(c0=0;c0<lod;c0++)
	{
		SLONG	dx,dy,dz;

		/*
		if(c0&1)
			u=0;
		else
			u=16;
		if(c0&2)
			v=0;
		else
			v=16;
		*/
		dy=get_steam_rand()&0x1ff;
		dy+=(GAME_TURN*((c0&3)+2));
		dy%=500;
		dx=(((get_steam_rand()&0xff)-128)*((dy>>2)+80))>>9;
		dz=(((get_steam_rand()&0xff)-128)*((dy>>2)+80))>>9;


		if(c0&4)
		{
			dx+=(COS(dy*4))>>11;
			dz+=(SIN(dy*4))>>11;

		}


		if(dy>500-(13*16))
		{
			trans=(500-dy)>>4;
		}
		if(dy<13*4)
		{
			trans=dy>>2;
		}
		else
		{
			trans=13;
		}


		dx+=x;
		dy+=y;
		dz+=z;

		if(trans>=1)
		{
			trans |= trans << 8;
			trans |= trans << 8;

			SPRITE_draw_tex(
				dx,
				dy,
				dz,
				((dy-y)>>2)+256,
				trans,
				0xff000000,
				POLY_PAGE_STEAM,
				0,0,31,31,
				1);
		}
	}
#endif
}

void	calc_floor_col(SLONG x,SLONG z,SLONG *r,SLONG *g,SLONG *b)
{
	SLONG	in;
	SLONG	in1_r,in2_r,in3_r,in0_r;
	SLONG	in1_g,in2_g,in3_g,in0_g;
	SLONG	in1_b,in2_b,in3_b,in0_b;
	SLONG	dx,dz,mx,mz,lum;


	mx=(x>>8)&0x7f;
	mz=(z>>8)&0x7f;

	lum=LUMI(mx,mz);
	in=floor_psx_col[mx][mz];
	in0_r=MAKELUMI(RED(in),lum);
	in0_g=MAKELUMI(GREEN(in),lum);
	in0_b=MAKELUMI(BLUE(in),lum);

	in=floor_psx_col[(mx+1)&0x7f][mz];
	lum=LUMI(mx+1,mz);
	in1_r=MAKELUMI(RED(in),lum);
	in1_g=MAKELUMI(GREEN(in),lum);
	in1_b=MAKELUMI(BLUE(in),lum);

	in=floor_psx_col[(mx)&0x7f][(mz+1)&0x7f];
	lum=LUMI(mx,mz+1);
	in2_r=MAKELUMI(RED(in),lum);
	in2_g=MAKELUMI(GREEN(in),lum);
	in2_b=MAKELUMI(BLUE(in),lum);

	in=floor_psx_col[(mx+1)&0x7f][(mz+1)&0x7f];
	lum=LUMI(mx+1,mz+1);
	in3_r=MAKELUMI(RED(in),lum);
	in3_g=MAKELUMI(GREEN(in),lum);
	in3_b=MAKELUMI(BLUE(in),lum);


	//   in0    in1
	//
	//
	//   in2    in3

	dx=x&0xff;
	dz=z&0xff;

	if(dx+dz<256)
	{
		*r=in0_r+(((in1_r-in0_r)*dx)>>8)+(((in2_r-in0_r)*dz)>>8);
		*g=in0_g+(((in1_g-in0_g)*dx)>>8)+(((in2_g-in0_g)*dz)>>8);
		*b=in0_b+(((in1_b-in0_b)*dx)>>8)+(((in2_b-in0_b)*dz)>>8);
	}
	else
	{
		*r=in3_r +(((in2_r-in3_r)*(256-dx))>>8)+(((in1_r-in3_r)*(256-dz))>>8);
		*g=in3_g +(((in2_g-in3_g)*(256-dx))>>8)+(((in1_g-in3_g)*(256-dz))>>8);
		*b=in3_b +(((in2_b-in3_b)*(256-dx))>>8)+(((in1_b-in3_b)*(256-dz))>>8);

	}
	if(*r<0)
		*r=0;
	if(*g<0)
		*g=0;
	if(*b<0)
		*b=0;
}

void	build_tween_matrix_psx(MATRIX *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween,SLONG shift)
{
	SLONG	v,w;

	v=((cmat1->M[0]&CMAT0_MASK)<<2)>>22;
	w=((cmat2->M[0]&CMAT0_MASK)<<2)>>22;

	mat->m[0][0]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

	v=((cmat1->M[0]&CMAT1_MASK)<<12)>>22;
	w=((cmat2->M[0]&CMAT1_MASK)<<12)>>22;


	mat->m[0][1]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

	v=((cmat1->M[0]&CMAT2_MASK)<<22)>>22;
	w=((cmat2->M[0]&CMAT2_MASK)<<22)>>22;

	mat->m[0][2]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);




	v=((cmat1->M[1]&CMAT0_MASK)<<2)>>22;
	w=((cmat2->M[1]&CMAT0_MASK)<<2)>>22;

	mat->m[1][0]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

	v=((cmat1->M[1]&CMAT1_MASK)<<12)>>22;
	w=((cmat2->M[1]&CMAT1_MASK)<<12)>>22;

	mat->m[1][1]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

	v=((cmat1->M[1]&CMAT2_MASK)<<22)>>22;
	w=((cmat2->M[1]&CMAT2_MASK)<<22)>>22;

	mat->m[1][2]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);




	v=((cmat1->M[2]&CMAT0_MASK)<<2)>>22;
	w=((cmat2->M[2]&CMAT0_MASK)<<2)>>22;

	mat->m[2][0]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

	v=((cmat1->M[2]&CMAT1_MASK)<<12)>>22;
	w=((cmat2->M[2]&CMAT1_MASK)<<12)>>22;

	mat->m[2][1]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

	v=((cmat1->M[2]&CMAT2_MASK)<<22)>>22;
	w=((cmat2->M[2]&CMAT2_MASK)<<22)>>22;

	mat->m[2][2]=(v<<MAT_SHIFT)+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>(3-shift);

}

//#define	BACK_CULL_MAGIC	3
SLONG FIGURE_draw_prim_tween(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG tween,
		struct GameKeyFrameElement *anim_info,
		struct GameKeyFrameElement *anim_info_next,
		MATRIX *rot_mat,
		SLONG off_dx,			   
		SLONG off_dy,
		SLONG off_dz,
		SLONG backwards,SLONG wx,SLONG wy,SLONG wz,
		SLONG	red,SLONG green,SLONG blue,
		SLONG sort_offset,SLONG scale

		)	// TRUE => The faces are drawn in the wrong order.
{
	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;
	SLONG sf,ef;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	ULONG col;
//	ULONG red;
//	ULONG green;
//	ULONG blue;
	
	UWORD r;
	UWORD g;
	UWORD b;
	ULONG face_colour;
	ULONG	flags;  
	SLONG	b0;

	Matrix31  offset;
//	Matrix33  mat2;
//	MATRIX  mat_final;

	MATRIX	 tween_matrix;
	MATRIX	 comb_matrix;

	SVECTOR temp;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	PSX_POLY_Point *pp;
	PSX_POLY_Point *ps;

//	PSX_POLY_Point *tri [3];
	PSX_POLY_Point *quad[4];
	struct	PrimPoint	*point;

	SLONG	flag;
	SVECTOR	input;
	VECTOR	output;

	SLONG	tflag;
	UBYTE	*cp;
	UBYTE	u,v;
	SLONG	clip;
	SLONG	ret_z[3];
	POLY_FT3	*p;

	pp=perm_pp_array;

	cp=the_display.CurrentPrim;
	check_prim_ptr((void**)&cp);

/*
//
// red,green,blue passed in and interpolated 
//
	pp->World.vx=x-POLY_cam_x;
	pp->World.vy=y-POLY_cam_y;
	pp->World.vz=z-POLY_cam_z;

	gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

	b0=128-(pp->Z>>5);

	if (b0<=0) return;

	if (!ware_flag)
	{
		col=floor_psx_col[x>>8][z>>8];
		red   = (((col >> 8) & 0xfc)*b0)>>8;
		green = (((col >> 2) & 0xf8)*b0)>>8;
		blue  = (((col << 3) & 0xf8)*b0)>>8;
	} else
		red=blue=green=b0;
*/

	//
	// Matrix functions we use.
	// 

//	tween=0;

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
//	mat		 = &anim_info     ->Matrix;
//	mat_next = &anim_info_next->Matrix;

	input.vx = anim_info->OffsetX + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8)+wx;
	input.vy = anim_info->OffsetY + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8)+wy;
	input.vz = anim_info->OffsetZ + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8)+wz;

	if(scale)
	{
		input.vx<<=scale;
		input.vy<<=scale;
		input.vz<<=scale;
	}

	gte_SetRotMatrix(rot_mat);
	gte_RotTrans(&input,&output,&flag);

	x+=output.vx; //offset.M[0];
	y+=output.vy; //offset.M[1];
	z+=output.vz; //offset.M[2];

	//
	// Create a temporary "tween" matrix between current and next
	//

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	build_tween_matrix_psx(&tween_matrix,&m1,&m2,tween,scale);

	gte_MulMatrix0(rot_mat,&tween_matrix,&comb_matrix); //tween_matrix=tween_matrix*rot_mat


	gte_SetRotMatrix(&PSX_view_matrix);

	POLY_set_local_rotation(
		x,
		y,
		z,
		&comb_matrix);

	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	pp = perm_pp_array;

	point=&prim_points[sp];

	sf=p_obj->StartFace4;
	ef=p_obj->EndFace4;
	p_f4 = &prim_faces4[sf];
	for (i = sf; i < ef; i++)
	{
		SLONG	clip_or,clip_and;
		UBYTE	draw_flags;
		draw_flags=p_f4->DrawFlags;

		p0 = p_f4->Points[0];
		p1 = p_f4->Points[2];
		p2 = p_f4->Points[1];

		pp[0].World.vx=prim_points[p0].X; pp[0].World.vy=prim_points[p0].Y; pp[0].World.vz=prim_points[p0].Z;
		pp[1].World.vx=prim_points[p1].X; pp[1].World.vy=prim_points[p1].Y; pp[1].World.vz=prim_points[p1].Z;
		pp[2].World.vx=prim_points[p2].X; pp[2].World.vy=prim_points[p2].Y; pp[2].World.vz=prim_points[p2].Z;
//		pp[3].World.vx=prim_points[p3].X; pp[3].World.vy=prim_points[p3].Y; pp[3].World.vz=prim_points[p3].Z;
		//
		// do it quick,  is a 3 & a 1 inline macro faster than a function?
		// really need to make our own macro as advised in gtemacro.h
		//

		//
		// faces are so small that this should sort ok 
		//
//		pp[0].Z=RotAverage4(&pp[0].World,&pp[1].World,&pp[2].World,&pp[3].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[3].SYSX,&pp[0].P,&pp[0].Flag);

		{
			gte_ldv3(&pp[0].World,&pp[1].World,&pp[2].World);	
			gte_rtpt();		

			/* CPU 22 ticks available */
			p3 = p_f4->Points[3];
			pp[3].World.vx=prim_points[p3].X; pp[3].World.vy=prim_points[p3].Y; pp[3].World.vz=prim_points[p3].Z;

			gte_stsxy3(&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX);	
//			gte_stdp(&pp[0].P);		
			gte_stflg(&pp[0].Flag);		
			gte_stsz3c(&ret_z[0]);
//			gte_stszotz(&pp[0].Z);	 not needed	 		 
		}
		

		clip_and=pp[0].Flag;
		gte_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX,&b0);

		if((clip_and&POLY_CLIP_NEAR)==0)

		if ((draw_flags & POLY_FLAG_DOUBLESIDED) || b0<0)

		{
			//
			// only do 4th point if not backface culled
			//
			gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

			clip=pp[0].SYSX&pp[1].SYSX&pp[2].SYSX&pp[3].SYSX;
			if((clip&(0x82008200))==0)
			{

				if((pp[3].Flag&(1<<31))==0)
//				if (draw_flags & POLY_FLAG_TEXTURED)
				{
					{

						POLY_FT4	*p;

						SLONG	page;

									   
						check_prim_ptr((void**)&cp);
						p=(POLY_FT4 *)cp; //the_display.CurrentPrim;

	//					page = p_f4->TexturePage|((p_f4->UV[0][0]&0xc0)<<2);
						page = p_f4->TexturePage;

	//					u = getPSXU(page);
	//					v = getPSXV(page);

						setPolyFT4(p);
						setUV4(p,p_f4->UV[0][0],p_f4->UV[0][1],
								p_f4->UV[2][0],p_f4->UV[2][1],
								p_f4->UV[1][0],p_f4->UV[1][1],
								p_f4->UV[3][0],p_f4->UV[3][1]);

						setRGB0(p,red,green,blue);
						setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
								pp[1].Word.SX,pp[1].Word.SY,
								pp[2].Word.SX,pp[2].Word.SY,
								pp[3].Word.SX,pp[3].Word.SY);


						p->tpage=getPSXTPage(page);
						if (p_f4->AltPal)
							p->clut=getPSXClut(956+p_f4->AltPal);
						else
							p->clut=getPSXClut(page);
//						pp[3].Z<<=2;
						z=sort_offset+(MAX4(ret_z[0],ret_z[1],ret_z[2],pp[3].Z)>>2);
						z=get_z_sort(z);

						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						cp+=sizeof(POLY_FT4);

					}

				}
				else
				{
	/*
					{
						POLY_F4	*p;
						r = ENGINE_palette[p_f4->Col].red;
						g = ENGINE_palette[p_f4->Col].green;
						b = ENGINE_palette[p_f4->Col].blue;

	//					if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
	//						return;

						p=(POLY_F4 *)cp; //the_display.CurrentPrim;

						setPolyF4(p);
						setRGB0(p,r,g,b);

						setXY4(p,quad[0]->Word.SX,quad[0]->Word.SY,
								quad[1]->Word.SX,quad[1]->Word.SY,
								quad[2]->Word.SX,quad[2]->Word.SY,
								quad[3]->Word.SX,quad[3]->Word.SY);


						z=quad[0]->Z>>1;
						z=get_z_sort(z);

						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						cp+=sizeof(POLY_F4);

					}
					//
					// The colour of the face.
					// 
	*/
				}

			}
		}
		p_f4++;
	}

	//
	// The triangles.
	//

	sf=p_obj->StartFace3;
	ef=p_obj->EndFace3;
	p_f3 = &prim_faces3[sf];
	p=(POLY_FT3 *)cp;
//	if(0)
	for (i = sf; i < ef; i++)
	{
 		SLONG	clip_or,clip_and;
		ULONG	flag,poo;
//		UBYTE	draw_flags;

#ifdef	BACK_CULL_MAGIC
		if(!(p_f3->DrawFlags&POLY_FLAG_TILED) | (i&3)==(GAME_TURN&3))  //back face cull magic
#endif
		{


			p0 = p_f3->Points[0];
			p1 = p_f3->Points[2];
			p2 = p_f3->Points[1];
			
			pp[0].World.vx=prim_points[p0].X; pp[0].World.vy=prim_points[p0].Y; pp[0].World.vz=prim_points[p0].Z;
			pp[1].World.vx=prim_points[p1].X; pp[1].World.vy=prim_points[p1].Y; pp[1].World.vz=prim_points[p1].Z;
			pp[2].World.vx=prim_points[p2].X; pp[2].World.vy=prim_points[p2].Y; pp[2].World.vz=prim_points[p2].Z;

	//		gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	//		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
	//		gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
//			gte_RotTransPers3(&pp[0].World,&pp[1].World,&pp[2].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&poo,&flag,&z);
		{
			gte_ldv3(&pp[0].World,&pp[1].World,&pp[2].World);	
			gte_rtpt();		

			/* CPU 22 ticks available */

			gte_stsxy3(&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX);	
//			gte_stdp(&pp[0].P);		
			gte_stflg(&pp[0].Flag);		
			gte_stsz3c(&ret_z[0]);
//			gte_stszotz(&pp[0].Z);	 not needed
		}



	//		clip_or=pp[0].Flag|pp[1].Flag|pp[2].Flag;
	//		clip_and=pp[0].Flag&pp[1].Flag&pp[2].Flag;
	//		if((clip_or&(1<<31))==0)

			if(!(pp[0].Flag&(1<<31)))
	//		if ((draw_flags & POLY_FLAG_DOUBLESIDED) || MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX)<0)
			if ((MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX))<0)
			{

				clip=pp[0].SYSX&pp[1].SYSX&pp[2].SYSX;
				if((clip&(0x82008200))==0)
				{
					{

						SLONG	page,tpage,clut;
						ULONG	uv;

						check_prim_ptr((void**)&p);

						page = p_f3->TexturePage;
						tpage=getPSXTPage(page);
						if (p_f3->AltPal)
							clut=getPSXClut(959+p_f3->AltPal);
						else
							clut=getPSXClut(page);

						//
						// texture UV's
						//
						uv=*((ULONG*)&p_f3->UV[1][0]);
						((ULONG *)p)[3]=((*((UWORD*)&p_f3->UV[0][0]))<<0)|(clut<<16);
						((ULONG *)p)[5]=((uv&0xffff0000)>>16)|((tpage<<16));
						((ULONG *)p)[7]=((uv&0xffff))>>0;

						//
						// screen x,y's
						//
						((ULONG *)p)[2]=pp[0].SYSX;
						((ULONG *)p)[4]=pp[1].SYSX;
						((ULONG *)p)[6]=pp[2].SYSX;

						//
						// RGB0
						//

						((ULONG *)p)[1]=(0x24<<24)|(blue<<16)|(green<<8)|(red);
						setRGB0(p,red,green,blue);


						setPolyFT3(p);
/*
						setUV3(p,p_f3->UV[0][0],p_f3->UV[0][1],
								p_f3->UV[2][0],p_f3->UV[2][1],
								p_f3->UV[1][0],p_f3->UV[1][1]);

						setRGB0(p,red,green,blue);
						setXY3(p,pp[0].Word.SX,pp[0].Word.SY,
								pp[1].Word.SX,pp[1].Word.SY,
								pp[2].Word.SX,pp[2].Word.SY);
*/

//						z>>=1;
						z=sort_offset+(MAX3(ret_z[0],ret_z[1],ret_z[2])>>2);
						z=get_z_sort(z);
						//z=sort_offset;


						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						p++;//=sizeof(POLY_FT3);

					}
				}
			}
		}
		p_f3++;
	}
//	cp=(UBYTE*)p;

	check_prim_ptr((void**)&p);
	the_display.CurrentPrim=(UBYTE*)p;
//	ASSERT(the_display.CurrentPrim< &the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM]);

	gte_SetRotMatrix(&PSX_view_matrix);
	PSX_view_matrix.t[0]=0;
	PSX_view_matrix.t[1]=0;
	PSX_view_matrix.t[2]=0;
	gte_SetTransMatrix(&PSX_view_matrix);

	return(z);
	
}
/*
MATRIX	light_mat={{{2364,2364,2364},
					{0,0,0},{0,0,0} },
//					{2364,2364,2364},
//					{2364,2364,2364} },
					{0,0,0}};
*/
//MATRIX	col_mat={{{2364,2364,2364},{2364,2364,2364},{2364,2364,2364}},{0,0,0}};
//MATRIX	 comb_matrix;

#define	MAT_SHIFT	(3)
void	build_matrix_psx(MATRIX *mat,struct CMatrix33 *cmat1)
{
	SLONG	v,w;

	v=((cmat1->M[0]&CMAT0_MASK)<<2)>>22;

	mat->m[0][0]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

	v=((cmat1->M[0]&CMAT1_MASK)<<12)>>22;


	mat->m[0][1]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

	v=((cmat1->M[0]&CMAT2_MASK)<<22)>>22;

	mat->m[0][2]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;




	v=((cmat1->M[1]&CMAT0_MASK)<<2)>>22;

	mat->m[1][0]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

	v=((cmat1->M[1]&CMAT1_MASK)<<12)>>22;

	mat->m[1][1]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

	v=((cmat1->M[1]&CMAT2_MASK)<<22)>>22;

	mat->m[1][2]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;




	v=((cmat1->M[2]&CMAT0_MASK)<<2)>>22;

	mat->m[2][0]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

	v=((cmat1->M[2]&CMAT1_MASK)<<12)>>22;

	mat->m[2][1]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

	v=((cmat1->M[2]&CMAT2_MASK)<<22)>>22;

	mat->m[2][2]=(v<<MAT_SHIFT);//+ ( ( (w-v)*tween)>>MAT_SHIFTD)>>3;

}

void POLY_build_tween_matrix(GameKeyFrameElement *anim_info,GameKeyFrameElement *anim_info_next,SLONG tween,MATRIX *rot_mat,MATRIX *output)
{
	MATRIX	 tween_matrix;

	static CMatrix33	m1, m2;

	GetCMatrix(anim_info, &m1);
//	build_matrix_psx(&tween_matrix,&m1);

	GetCMatrix(anim_info_next, &m2);

	build_tween_matrix_psx(&tween_matrix,&m1,&m2,tween,0);
	gte_MulMatrix0(rot_mat,&tween_matrix,output); //tween_matrix=tween_matrix*rot_mat

//	gte_SetRotMatrix(&PSX_view_matrix);
}

void POLY_build_no_tween_matrix(GameKeyFrameElement *anim_info,GameKeyFrameElement *anim_info_next,SLONG tween,MATRIX *rot_mat,MATRIX *output)
{
	MATRIX	 tween_matrix;

	static CMatrix33	m1;

	GetCMatrix(anim_info, &m1);
	build_matrix_psx(&tween_matrix,&m1);

	gte_MulMatrix0(rot_mat,&tween_matrix,output); //tween_matrix=tween_matrix*rot_mat

}

void	set_light_matrix( MATRIX *comb_matrix_local)
{
	MATRIX	light_dir;

	light_dir.m[0][0]=4095;light_dir.m[0][1]=-4095;light_dir.m[0][2]=-4095;
	light_dir.m[1][0]=0;light_dir.m[1][1]=4095;light_dir.m[1][2]=0;
	light_dir.m[2][0]=0;light_dir.m[2][1]=0;light_dir.m[2][2]=0;

	MulMatrix(&light_dir,comb_matrix_local); //light_dir = light_dir * comb_matrix

	SetLightMatrix(&light_dir);
}





extern	UWORD	darci_normal_count;
extern	UWORD	*darci_normal;

void	 FIGURE_draw_prim_tween_lit(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG tween,
		struct GameKeyFrameElement *anim_info,
		struct GameKeyFrameElement *anim_info_next,
		MATRIX *rot_mat,
		SLONG off_dx,			   
		SLONG off_dy,
		SLONG off_dz,
		SLONG lit,SLONG wx,SLONG wy,SLONG wz,
		SLONG	red,SLONG green,SLONG blue,
		SLONG sort_offset,
		SLONG	skill
//		MATRIX	*comb_matrix_local

		)	// TRUE => The faces are drawn in the wrong order.
{
	SLONG i;

	SLONG sf,ef;
	SLONG b0;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	ULONG	uv;

	CVECTOR cv;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	PSX_POLY_Point *pp;


	SVECTOR	input;
	VECTOR	output;

	UBYTE	*cp;
	SLONG	clip;
	
	MATRIX	comb_matrix_local;

	pp=perm_pp_array;
	cp=the_display.CurrentPrim;

	SVECTOR sv0,sv1,sv2,sv3;
	CVECTOR co0,co1,co2,co3;
	UBYTE	do_tween=1;
	SLONG	ret_z[3];
	SLONG	page;

	ASSERT(skill<3);


	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	input.vx = anim_info->OffsetX +wx;//+ ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8)+wx;
	input.vy = anim_info->OffsetY +wy;//+ ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8)+wy;
	input.vz = anim_info->OffsetZ +wz;//+ ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8)+wz;

	if(anim_info!=anim_info_next)
	{
		input.vx += ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8);
		input.vy += ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8);
		input.vz += ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8);
	}
	else
	{
		do_tween=0;
	}

	if(!lit)
	{
		red=MAX(red-32,0);
		green=MAX(green-32,0);
		blue=MAX(blue-32,0);
	}
	cv.r=red;//192;
	cv.g=green;//192;
	cv.b=blue;//192;

	//
	// rotate tweened sub_object offset into direction set by rot_matrix which is figure yaw
	//
	PushMatrix();
	gte_SetRotMatrix(rot_mat);
	gte_RotTrans(&input,&output,&clip);

	x+=output.vx; 
	y+=output.vy; 
	z+=output.vz; 

//	sort_offset=get_z_sort(sort_offset);
	//
	// Create a temporary "tween" matrix between current and next in global comb_matrix
	//
	if(do_tween)
	{
		POLY_build_tween_matrix(anim_info,anim_info_next,tween,rot_mat,&comb_matrix_local);
	}
	else
	{
		POLY_build_no_tween_matrix(anim_info,anim_info_next,tween,rot_mat,&comb_matrix_local);
	}
	if(lit)
		set_light_matrix(&comb_matrix_local);


	//
	// comb_matrix is world to object space
	//

	{
		ULONG	flag;
		SVECTOR	input;
		VECTOR	output;
	//	MATRIX	view_trans;

		PopMatrix();
		PushMatrix();
		//gte_SetRotMatrix(&PSX_view_matrix);


		input.vx = x - POLY_cam_x;
		input.vy = y - POLY_cam_y;
		input.vz = z - POLY_cam_z;


		gte_RotTrans(&input,&output,&flag);
/*
		POLY_cam_off_x = output.vx;
		POLY_cam_off_y = output.vy;
		POLY_cam_off_z = output.vz;

		PSX_view_matrix.t[0]=POLY_cam_off_x;
		PSX_view_matrix.t[1]=POLY_cam_off_y;
		PSX_view_matrix.t[2]=POLY_cam_off_z;
*/
		comb_matrix_local.t[0]=output.vx;
		comb_matrix_local.t[1]=output.vy;
		comb_matrix_local.t[2]=output.vz;


		SetMulMatrix(&PSX_view_matrix,&comb_matrix_local);
		gte_SetTransMatrix(&comb_matrix_local);//&PSX_view_matrix);

	}
/*
	POLY_set_local_rotation(
		x,
		y,
		z,
		&comb_matrix_local);
*/


	//
	// Rotate all the points into the POLY_buffer.
	//

	// Create a lighting matrix
	SetBackColor(red,green,blue);

	p_obj = &prim_objects[prim];

	pp = perm_pp_array;

//	point=&prim_points[sp];

	sf=p_obj->StartFace4;
	ef=p_obj->EndFace4;
	p_f4 = &prim_faces4[sf];

	// draw normals removed 3rd September, check sourcesafe to put back in

	
	for (i = sf; i < ef; i++)
	{
		UBYTE	draw_flags;

		draw_flags=p_f4->DrawFlags;

		p0 = p_f4->Points[0];
		p1 = p_f4->Points[2];
		p2 = p_f4->Points[1];

		pp[0].World.vx=prim_points[p0].X; pp[0].World.vy=prim_points[p0].Y; pp[0].World.vz=prim_points[p0].Z;
		pp[1].World.vx=prim_points[p1].X; pp[1].World.vy=prim_points[p1].Y; pp[1].World.vz=prim_points[p1].Z;
		pp[2].World.vx=prim_points[p2].X; pp[2].World.vy=prim_points[p2].Y; pp[2].World.vz=prim_points[p2].Z;

//		gte_RotTransPers3(&pp[0].World,&pp[1].World,&pp[2].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		//#define gte_RotTransPers3(r1,r2,r3,r4,r5,r6,r7,r8,r9)		

		{
			gte_ldv3(&pp[0].World,&pp[1].World,&pp[2].World);	
			gte_rtpt();		

			/* CPU 22 ticks available */
			p3 = p_f4->Points[3];
			pp[3].World.vx=prim_points[p3].X; pp[3].World.vy=prim_points[p3].Y; pp[3].World.vz=prim_points[p3].Z;

			gte_stsxy3(&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX);	
//			gte_stdp(&pp[0].P);		
			gte_stflg(&pp[0].Flag);		
			gte_stsz3c(&ret_z[0]);
//			gte_stszotz(&pp[0].Z);	 not needed
		}

		if(skill<0)
		{
			if(pp[0].Flag&(3<<17))
			{
				fuck_z(&pp[0]);
				fuck_z(&pp[1]);
				fuck_z(&pp[2]);
				pp[0].Flag|=pp[1].Flag|pp[2].Flag;
			}
		}


		gte_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX,&b0);

//		if((pp[0].Flag&pp[1].Flag&pp[2].Flag&pp[3].Flag&pp[4].Flag&(1<<31))==0)
		if((pp[0].Flag&(1<<31))==0)

		if ((draw_flags & POLY_FLAG_DOUBLESIDED) || b0<0)

		{
			ULONG	clip;

			//
			// only do 4th point if not backface culled
			//
			gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
			if(skill<0)
			{
				if(pp[3].Flag&(3<<17))
				{
					fuck_z(&pp[3]);
				}
			}


			clip=pp[0].SYSX&pp[1].SYSX&pp[2].SYSX&pp[3].SYSX;

			if((clip&(0x82008200))==0)
			if((pp[3].Flag&(1<<31))==0)
//			if (draw_flags & POLY_FLAG_TEXTURED)
			{
				if(lit)
				{
					UWORD	norm;
					norm=darci_normal[p0];

//					ASSERT(p0<darci_normal_count);
//					ASSERT(p1<darci_normal_count);
//					ASSERT(p2<darci_normal_count);
//					ASSERT(p3<darci_normal_count);

					sv0.vx=(((norm>>10)&31)-16)<<8;
					sv0.vy=(((norm>>5)&31)-16)<<8;
					sv0.vz=(((norm>>0)&31)-16)<<8;
					norm=darci_normal[p1];
					sv1.vx=(((norm>>10)&31)-16)<<8;
					sv1.vy=(((norm>>5)&31)-16)<<8;
					sv1.vz=(((norm>>0)&31)-16)<<8;
					norm=darci_normal[p2];
					sv2.vx=(((norm>>10)&31)-16)<<8;
					sv2.vy=(((norm>>5)&31)-16)<<8;
					sv2.vz=(((norm>>0)&31)-16)<<8;
//					gte_NormalColorCol3(&sv0,&sv1,&sv2,&cv,&co0,&co1,&co2);
					//#define gte_NormalColorCol3(r1,r2,r3,r4,r5,r6,r7)		
					{
						gte_ldv3(&sv0,&sv1,&sv2);	
						gte_ldrgb(&cv);		
						gte_ncct();		

						/* CPU 39 ticks */
						norm=darci_normal[p3];  
						sv3.vx=(((norm>>10)&31)-16)<<8;
						sv3.vy=(((norm>>5)&31)-16)<<8;
						sv3.vz=(((norm>>0)&31)-16)<<8;
						ret_z[0]=sort_offset+(MAX4(ret_z[0],ret_z[1],ret_z[2],pp[3].Z)>>2);

						gte_strgb3(&co0,&co1,&co2);	
					}

					gte_NormalColorCol(&sv3,&cv,&co3);
				}
				else
				{
					co0.r=red;
					co0.g=green;
					co0.b=blue;
					ret_z[0]>>=2;
					if(ret_z[0]>550)
					{
						SLONG	sub;
						sub=ret_z[0]-550;
//						sub<<=2;
						co0.r=MAX(red-sub,32);
						co0.g=MAX(green-sub,32);
						co0.b=MAX(blue-sub,32);
					}
					*(ULONG*)&co1=*(ULONG*)&co0;
					*(ULONG*)&co2=*(ULONG*)&co0;
					*(ULONG*)&co3=*(ULONG*)&co0;
					
				}




				{

					POLY_GT4	*p;


					p=(POLY_GT4 *)cp;
					check_prim_ptr((void**)&cp);

					page = p_f4->TexturePage;

					if(skill>0 && (p_f4->FaceFlags&FACE_FLAG_THUG_JACKET))
					{
						SLONG	dx,dy=0;
//					page=199;
//						page+=skill;



					
						page+=skill;

						dx=(skill<<5);

						if( (p_f4->UV[0][0]+dx)>255)
							dy=32;

					setUV4(p,(p_f4->UV[0][0]+dx)&255,(p_f4->UV[0][1]+dy)&255,
							(p_f4->UV[2][0]+dx)&255,(p_f4->UV[2][1]+dy)&255,
							(p_f4->UV[1][0]+dx)&255,(p_f4->UV[1][1]+dy)&255,
							(p_f4->UV[3][0]+dx)&255,(p_f4->UV[3][1]+dy)&255);

						p->tpage=getPSXTPage(page);
						p->clut=getPSXClut(page);

						setUV4(p,p_f4->UV[0][0],p_f4->UV[0][1],
								p_f4->UV[2][0],p_f4->UV[2][1],
								p_f4->UV[1][0],p_f4->UV[1][1],
								p_f4->UV[3][0],p_f4->UV[3][1]);

					}
					else
					{
						setUV4(p,p_f4->UV[0][0],p_f4->UV[0][1],
								p_f4->UV[2][0],p_f4->UV[2][1],
								p_f4->UV[1][0],p_f4->UV[1][1],
								p_f4->UV[3][0],p_f4->UV[3][1]);
					}


					((ULONG *)p)[1]=*((ULONG*)&co0);
					((ULONG *)p)[4]=*((ULONG*)&co1);
					((ULONG *)p)[7]=*((ULONG*)&co2);
					((ULONG *)p)[10]=*((ULONG*)&co3);

					((ULONG *)p)[2]=pp[0].SYSX;
					((ULONG *)p)[5]=pp[1].SYSX;
					((ULONG *)p)[8]=pp[2].SYSX;
					((ULONG *)p)[11]=pp[3].SYSX;



					setPolyGT4(p);

					p->tpage=getPSXTPage(page);
					p->clut=getPSXClut(page);


//					uv=*((ULONG*)&p_f4->UV[0][0]);

//					((ULONG *)p)[3]=((uv&0xffff)>>0)|((getPSXClut(page)<<16));
//					((ULONG *)p)[6]=((uv&0xffff0000)>>16)|((getPSXTPage(page)<<16));

//					uv=*((ULONG*)&p_f4->UV[2][0]);

//					((ULONG *)p)[12]=((uv&0xffff))>>0;
//					((ULONG *)p)[9]=((uv&0xffff0000))>>16;

/*
					setUV4(p,p_f4->UV[0][0],p_f4->UV[0][1],
							p_f4->UV[2][0],p_f4->UV[2][1],
							p_f4->UV[1][0],p_f4->UV[1][1],
							p_f4->UV[3][0],p_f4->UV[3][1]);
					setRGB0(p,co0.r,co0.g,co0.b);
					setRGB1(p,co1.r,co1.g,co1.b);
					setRGB2(p,co2.r,co2.g,co2.b);
					setRGB3(p,co3.r,co3.g,co3.b);
					setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
							pp[1].Word.SX,pp[1].Word.SY,
							pp[2].Word.SX,pp[2].Word.SY,
							pp[3].Word.SX,pp[3].Word.SY);
*/




//					p->tpage=getPSXTPage(page);
//					p->clut=getPSXClut(page);
/*
					pp[0].Z=max(pp[0].Z,pp[1].Z)>>1;
					pp[0].Z+=sort_offset;
					pp[0].Z=get_z_sort(pp[0].Z);
*/

					//pp[0].Z=sort_offset;
//					return_value=ret_z[0];


					if(ret_z[0]>500 ||skill<0)
					{
						setSemiTrans(p,1);
						p->tpage&=~(3<<5);


					}
					ret_z[0]=get_z_sort(ret_z[0]);

					DOPRIM(ret_z[0],p);
					cp+=sizeof(POLY_GT4);

				}
			}
		}

		p_f4++;
	}

	//
	// The triangles.
	//

	sf=p_obj->StartFace3;
	ef=p_obj->EndFace3;
	p_f3 = &prim_faces3[sf];
	for (i = sf; i < ef; i++)
	{
		ULONG	poo;
		ULONG	clip;

		p0 = p_f3->Points[0];
		p1 = p_f3->Points[2];
		p2 = p_f3->Points[1];
		
		pp[0].World.vx=prim_points[p0].X; pp[0].World.vy=prim_points[p0].Y; pp[0].World.vz=prim_points[p0].Z;
		pp[1].World.vx=prim_points[p1].X; pp[1].World.vy=prim_points[p1].Y; pp[1].World.vz=prim_points[p1].Z;
		pp[2].World.vx=prim_points[p2].X; pp[2].World.vy=prim_points[p2].Y; pp[2].World.vz=prim_points[p2].Z;
/*
		v0.vx=pp[1].World.vx-pp[0].World.vx;
		v0.vy=pp[1].World.vy-pp[0].World.vy;
		v0.vz=pp[1].World.vz-pp[0].World.vz;
		v1.vx=pp[2].World.vx-pp[0].World.vx;
		v1.vy=pp[2].World.vy-pp[0].World.vy;
		v1.vz=pp[2].World.vz-pp[0].World.vz;

		PushMatrix();
		gte_OuterProduct0(&v0,&v1,&v2);
		PopMatrix();
		VectorNormalS(&v2,&sv);
*/

//		gte_RotTransPers3(&pp[0].World,&pp[1].World,&pp[2].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&poo,&pp[0].Flag,&pp[0].Z);
		//gte_RotColorDpq3(&pp[0].World,&pp[1].World,&pp[2].World,&sv0,&sv1,&sv2,&cv,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&co0,&co1,&co2,&pp[0].Flag,&pp[0].Z);
		{
			gte_ldv3(&pp[0].World,&pp[1].World,&pp[2].World);	
			gte_rtpt();		

			/* CPU 22 ticks available */

			gte_stsxy3(&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX);	
//			gte_stdp(&pp[0].P);		
			gte_stflg(&pp[0].Flag);		
			gte_stsz3c(&ret_z[0]);
//			gte_stszotz(&pp[0].Z);	 not needed
		}

		if(skill<0)
		{
			if(pp[0].Flag&(3<<17))
			{
				fuck_z(&pp[0]);
				fuck_z(&pp[1]);
				fuck_z(&pp[2]);
				pp[0].Flag|=pp[1].Flag|pp[2].Flag;
			}
		}
		
		clip=pp[0].SYSX&pp[1].SYSX&pp[2].SYSX;

		if((clip&(0x82008200))==0)
		if((pp[0].Flag&(1<<31))==0)
		{
			UBYTE	trans=0;
			if ((MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX))<0)
			{
				POLY_GT3 *p;
				check_prim_ptr((void**)&cp);
				p=(POLY_GT3 *)cp;

//				SVECTOR sv0,sv1,sv2,sv3;
//				CVECTOR co0,co1,co2,co3;

				if(lit)
				{
					UWORD	norm;
					ASSERT(p0<darci_normal_count);
					ASSERT(p1<darci_normal_count);
					ASSERT(p2<darci_normal_count);
					norm=darci_normal[p0];
					sv0.vx=(((norm>>10)&31)-16)<<8;
					sv0.vy=(((norm>>5)&31)-16)<<8;
					sv0.vz=(((norm>>0)&31)-16)<<8;
					norm=darci_normal[p1];
					sv1.vx=(((norm>>10)&31)-16)<<8;
					sv1.vy=(((norm>>5)&31)-16)<<8;
					sv1.vz=(((norm>>0)&31)-16)<<8;
					norm=darci_normal[p2];
					sv2.vx=(((norm>>10)&31)-16)<<8;
					sv2.vy=(((norm>>5)&31)-16)<<8;
					sv2.vz=(((norm>>0)&31)-16)<<8;
					{
						gte_ldv3(&sv0,&sv1,&sv2);	
						gte_ldrgb(&cv);		
						gte_ncct();		

						/* CPU 39 ticks */
						ret_z[0]=sort_offset+(MAX3(ret_z[0],ret_z[1],ret_z[2])>>2);
						((ULONG *)p)[2]=pp[0].SYSX;
						((ULONG *)p)[5]=pp[1].SYSX;
						((ULONG *)p)[8]=pp[2].SYSX;
						uv=*((ULONG*)&p_f3->UV[1][0]);
//						return_value=ret_z[0];
						ret_z[0]=get_z_sort(ret_z[0]);


						gte_strgb3(&co0,&co1,&co2);	
					}
//					gte_NormalColorCol3(&sv0,&sv1,&sv2,&cv,&co0,&co1,&co2);
				}
				else
				{
					co0.r=red;
					co0.g=green;
					co0.b=blue;
					if((ret_z[0]>>2)>550)
					{
						SLONG	sub;
						sub=(ret_z[0]>>2)-550;
//						sub<<=2;
						co0.r=MAX(red-sub,32);
						co0.g=MAX(green-sub,32);
						co0.b=MAX(blue-sub,32);
						trans=1;
					}
					

					*(ULONG*)&co1=*(ULONG*)&co0;
					*(ULONG*)&co2=*(ULONG*)&co0;

					((ULONG *)p)[2]=pp[0].SYSX;
					((ULONG *)p)[5]=pp[1].SYSX;
					((ULONG *)p)[8]=pp[2].SYSX;
					uv=*((ULONG*)&p_f3->UV[1][0]);
//					return_value=ret_z[0]>>2;
					ret_z[0]=get_z_sort(ret_z[0]>>2);
					
				}

/*
				co0.r=128;
				co0.g=128;
				co0.b=128;
				co1.r=128;
				co1.g=128;
				co1.b=128;
				co2.r=128;
				co2.g=128;
				co2.b=128;
*/

				//
				// build the POLY_GT3 using long memory reads/writes
				//

				((ULONG *)p)[1]=*((ULONG*)&co0);
				((ULONG *)p)[4]=*((ULONG*)&co1);
				((ULONG *)p)[7]=*((ULONG*)&co2);

				setPolyGT3(p);

				page=p_f3->TexturePage;
				if(skill>0  && (p_f3->FaceFlags&FACE_FLAG_THUG_JACKET))
				{
					SLONG	dx,dy=0;
					
//					page=199;
					page+=skill;
//						ASSERT(0);

					dx=(skill<<5);

					if( (p_f3->UV[0][0]+dx)>255)
						dy=32;

				setUV3(p,(p_f3->UV[0][0]+dx)&255,(p_f3->UV[0][1]+dy)&255,
						(p_f3->UV[2][0]+dx)&255,(p_f3->UV[2][1]+dy)&255,
						(p_f3->UV[1][0]+dx)&255,(p_f3->UV[1][1]+dy)&255);

					p->tpage=getPSXTPage(page);
					p->clut=getPSXClut(page);


				}
				else
				{
					((ULONG *)p)[3]=((*((UWORD*)&p_f3->UV[0][0]))<<0)|(getPSXClut(page)<<16);
					((ULONG *)p)[6]=((uv&0xffff0000)>>16)|((getPSXTPage(page)<<16));
					((ULONG *)p)[9]=((uv&0xffff))>>0;
				}


//				pp[0].Z=(pp[0].Z>>1)+sort_offset;
//				pp[0].Z=sort_offset;//get_z_sort(sort_offset);//pp[0].Z);

/*
//
// old method, reading and writing each data item at a time
//
				setUV3(p,p_f3->UV[0][0],p_f3->UV[0][1],
						p_f3->UV[2][0],p_f3->UV[2][1],
						p_f3->UV[1][0],p_f3->UV[1][1]);

				setRGB0(p,co0.r,co0.g,co0.b);
				setRGB1(p,co1.r,co1.g,co1.b);
				setRGB2(p,co2.r,co2.g,co2.b);

				setXY3(p,pp[0].Word.SX,pp[0].Word.SY,
						pp[1].Word.SX,pp[1].Word.SY,
						pp[2].Word.SX,pp[2].Word.SY);

				p->tpage=getPSXTPage(p_f3->TexturePage);
				p->clut=getPSXClut(p_f3->TexturePage);
*/
				if(trans||skill<0)
				{
					setSemiTrans(p,1);
					p->tpage&=~(3<<5);
				}


				DOPRIM(ret_z[0],p);

				cp+=sizeof(POLY_GT3);
			}
		}
		p_f3++;
	}

	check_prim_ptr((void**)&cp);
	the_display.CurrentPrim=(UBYTE*)cp;
	PopMatrix();

//	gte_SetRotMatrix(&PSX_view_matrix);
/*
	PSX_view_matrix.t[0]=0;
	PSX_view_matrix.t[1]=0;
	PSX_view_matrix.t[2]=0;
*/
	gte_SetTransMatrix(&PSX_view_matrix);
//	return(return_value);
}

SWORD	mid_peep_z;
UBYTE	draw_order[20];

SWORD	store_z[20];
SBYTE	part_offset[20]=
{
	1,//"pelvis",
	0,//"lfemur",
	0,//"ltibia",
	0,//"lfoot",
	-1,//"torso",
	-1,//"rhumorus",
	-1,//"rradius",
	-1,//"rhand",
	-1,//"lhumorus",
	-1,//"lradius",
	-1,//"lhand",
	-2,//"skull",
	0,//"rfemur",
	0,//"rtibia",
	0,//"rfoot",

	0,0,0,0,0
	
};


SLONG	pers_off=0;
/*
void	precalc_z(Thing *p_thing,SLONG wx,SLONG wy,SLONG wz,GameKeyFrameElement *ae1,GameKeyFrameElement *ae2,SLONG tween,MATRIX *r_matrix,SLONG z_bodge,SLONG ele_count)
{
	struct GameKeyFrameElement *anim_info;
	struct GameKeyFrameElement *anim_info_next;
	SVECTOR	input;
	VECTOR	output;
	SLONG	mx,my,mz,i,j;
	ULONG	flag,p;
//	PSX_POLY_Point point,point2;
	SLONG	mid_z,first=1;

	SBYTE	next[16];
	SBYTE	val[16];
	UBYTE	head;
	UWORD	index;

	if (PadKeyIsPressed(&PAD_Input2,PAD_LU))
	{
		pers_off+=2;
	}
	if (PadKeyIsPressed(&PAD_Input2,PAD_LD))
	{
		pers_off-=2;
	}
	next[1]=0;
	val[1]=0;

	mx=(p_thing->WorldPos.X >> 8);
	my=(p_thing->WorldPos.Y >> 8);
	mz=(p_thing->WorldPos.Z >> 8);


	mx-=POLY_cam_x;
	my-=POLY_cam_y;
	mz-=POLY_cam_z;

	input.vx = mx;
	input.vy = my;
	input.vz = mz;
	gte_RotTrans(&input,&output,&flag);
	mx=output.vx;
	my=output.vy;
	mz=output.vz;



	SetMulMatrix(&PSX_view_matrix,r_matrix);
	head=0;

	for (i = 0; i < ele_count; i++)
	{
		SWORD	my_val,prev,temp;
		anim_info=&ae1[i];
		anim_info_next=&ae2[i];


		input.vx = anim_info->OffsetX + ((anim_info_next->OffsetX  - anim_info->OffsetX) * tween >> 8)+wx;
		input.vy = anim_info->OffsetY + ((anim_info_next->OffsetY  - anim_info->OffsetY) * tween >> 8)+wy;
		input.vz = anim_info->OffsetZ + ((anim_info_next->OffsetZ  - anim_info->OffsetZ) * tween >> 8)+wz;


		gte_RotTrans(&input,&output,&flag);


//		store_z[i]=((output.vz+mz)>>0);//+part_offset[i];
		if(i==0)
		{
			// pelvis

			//
			// first one is zero
			//
			mid_z=output.vz+mz;
			head=1;
//			my_val-=10;
			val[1]=0;
			continue;
		}


		my_val=output.vz+mz-mid_z;
		if(i==4)
		{
			//bodge torso a bit nearer
			my_val-=10;
		}
		index=head;
		prev=0;
		while(val[index]>my_val && index)
		{
			prev=index;
			index=next[index];
		}
		if(prev)
		{
			//
			// insert in middle or end
			//
			temp=next[prev];
			next[prev]=i+1;
			next[i+1]=temp;
		}
		else
		{
			//
			// insert at start
			//
			next[i+1]=head;
			head=i+1;

		}
		val[i+1]=my_val;
		

		
#ifdef	DEBUG_INFO
		if(!((point.Flag|point2.Flag)&(1<<31)))
		{
			LINE_G2	*p;
			SLONG	page,z;

			p=(LINE_G2 *)the_display.CurrentPrim;


			setLineG2(p);

			setRGB0(p,255,18,18);
			setRGB1(p,128,128,128);
			setXY2(p,point.Word.SX,point.Word.SY,
					point2.Word.SX,point2.Word.SY);

			point.Z>>=1;
			z=get_z_sort(point.Z);

			DOPRIM(z,p);
			the_display.CurrentPrim+=sizeof(LINE_G2);
		}
#endif

		//
		// should give us a z for zorting
		//

	}

	index=head;
	mid_z>>=2;
	mid_z-=10;
	mid_z+=z_bodge+pers_off;
	if(mid_z>=768)
		mid_z=767;
	mid_z=get_z_sort_near(mid_z);
//	if(mid_z<20)
//		mid_z=20;

	mid_peep_z=mid_z;

	
	{
		UWORD	count=0;
		while(index)
		{
			store_z[index-1]=mid_peep_z;
	//		mid_z++;
			draw_order[count++]=index-1;
			index=next[index];
		}
	}
//	store_z[1]=store_z[3]; //femur equals foot
//	store_z[12]=store_z[14]; //femur equals foot
	gte_SetRotMatrix(&PSX_view_matrix);
}
*/


void ANIM_obj_draw(Thing *p_thing,DrawTween *dt)
{

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG	wx=0,wy=0,wz=0;
	SLONG	red,green,blue;

	MATRIX r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

/*
	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		return;
	}
*/
	//
	// Find the lighting context for this thing.  The FIGURE_draw_prim_tween() function
	// calls LIGHT_get_point(). LIGHT_get_point returns the light on the given
	// point in this context.
	//
#ifndef	PSX
	LIGHT_get_context(THING_NUMBER(p_thing));
#endif

	//
	// The offset to keep the locked limb in the same place.
	//

	dx = 0;
	dy = 0;
	dz = 0;

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

#ifndef FS_ISO9600
	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}
#endif

	{
		build_rot_matrix(dt->Angle,0,&r_matrix);
	}

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
//	SLONG object_offset;

	if ((p_thing->Class == CLASS_PERSON)&&(p_thing->Genus.Person->Ware))
		ware_flag=1;
	else
		ware_flag=0;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	//
	// calc colour for whole obj
	//
	{
		SLONG	dx,dy,dz;
		SLONG	px,py,pz;
		px=(p_thing->WorldPos.X>>8)+wx;
		py=(p_thing->WorldPos.Y>>8)+wy;
		pz=(p_thing->WorldPos.Z>>8)+wz;

		dx=abs(px-POLY_cam_x);
		dy=abs(py-POLY_cam_y);
		dz=abs(pz-POLY_cam_z);

		dx=QDIST3(dx,dy,dz)>>3;


		dx=256-dx+100;
//		if(dx<0)
//			return;
//		if(dx>255)
//			dx=255;
		dx=0;
		dz=0;


		if (!ware_flag)
		{
			calc_floor_col(px,pz,&red,&green,&blue);
			red=(red*dx)>>8;
			green=(green*dx)>>8;
			blue=(blue*dx)>>8;
		} else
			red=blue=green=128;
	}

//	precalc_z(p_thing,wx,wy,wz,ae1,ae2,dt->AnimTween,&r_matrix,0,ele_count);

	for (i = 0; i < ele_count; i++)
	{
//		SLONG	personid;
		SLONG	index;
//		personid=dt->PersonID;
		index=i;//draw_order[ele_count-1-i];

//		object_offset = dt->TheChunk->PeopleTypes[personid&0x1f].BodyPart[index];
//		object_offset = i; //dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];

		FIGURE_draw_prim_tween(
			start_object + index, //object_offset,
			p_thing->WorldPos.X >> 8,
			(p_thing->WorldPos.Y >> 8),//+5,
			p_thing->WorldPos.Z >> 8,
			dt->AnimTween,
		   &ae1[index],
		   &ae2[index],
		   &r_matrix,
			dx,dy,dz,
			FALSE,wx,wy,wz,128,128,128,0,2);

		//
		// restore normal camera matrix at end of each draw prim tween
		//
		gte_SetRotMatrix(&PSX_view_matrix);
		gte_SetTransMatrix(&PSX_view_matrix);
	}


}
#define	BODGE_SIZE		(20)
//
// If you are the other side of a fence to the camera, then sort yourself away from the fence
//
extern	SLONG AENG_cam_yaw;
extern	UBYTE WARE_get_caps(UBYTE ware,UBYTE x,UBYTE z,UBYTE dir);


inline	UBYTE peep_get_caps(UWORD ware,UBYTE x,UBYTE z,UBYTE dir)
{
/*
	if(ware)
	{
		return(WARE_get_caps(ware,x,z,dir));
	}
	else
*/
	{
		return(MAV_get_caps(x,z,dir));
	}
}

SLONG get_sort_z_bodge(SLONG px,SLONG pz,Thing *p_thing)
{
	SLONG	dx,dz;
	UBYTE	cap;
	UWORD	ware;

	dx=px-POLY_cam_x;
	dz=pz-POLY_cam_z;
	ware=p_thing->Genus.Person->Ware;


//	if (p_thing->Draw.Tweened->CurrentAnim == ANIM_SIT_DOWN   ||
//		p_thing->Draw.Tweened->CurrentAnim == ANIM_SIT_IDLE)
//		return(0);

//	if (p_thing->Genus.Person->Action==47)//ACTION_SIT_BENCH)
//		return(-150);

	if ((p_thing->SubState==SUB_STATE_DANGLING)||
		(p_thing->SubState==SUB_STATE_PULL_UP)||
		(p_thing->SubState==SUB_STATE_GRAB_TO_DANGLE)||
		(p_thing->SubState==SUB_STATE_TRAVERSE_LEFT)||
		(p_thing->SubState==SUB_STATE_TRAVERSE_RIGHT)||
		(p_thing->State==STATE_CLIMB_LADDER))
	{
		SLONG	dangle;
		dangle = abs(((-AENG_cam_yaw)&2047)-p_thing->Draw.Tweened->Angle);

		if(dangle>512 && dangle<1024+512)
		{
			return(20);
		}
		else
		{
			return(-20);
		}





	}
	if(ware)
		return(0);
//			sort_z_offset-=30;

	//if(abs(dz)<abs(dx))
	if(abs(dx)>100)
	{
		if(dx<0)
		{
			if((px&0xff)>128)
			{
				cap=peep_get_caps(ware,px>>8,pz>>8,MAV_DIR_XL);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(BODGE_SIZE);
				}
			}

		}
		else
		{
			if((px&0xff)<128)
			{
				cap=peep_get_caps(ware,px>>8,pz>>8,MAV_DIR_XS);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(BODGE_SIZE);
				}
			}

		}
		
	}
//	else
	if(abs(dz)>100)
	{
		if(dz<0)
		{
			if((pz&0xff)>128)
			{
				cap=peep_get_caps(ware,px>>8,pz>>8,MAV_DIR_ZL);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(BODGE_SIZE);
				}
			}

		}
		else
		{
			if((pz&0xff)<128)
			{
				cap=peep_get_caps(ware,px>>8,pz>>8,MAV_DIR_ZS);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(BODGE_SIZE);
				}
			}

		}
		
	}
	return(0);

}


#if 1
extern char *GDisp_Bucket;
#else
extern char GDisp_Bucket[];
#endif


CBYTE	str2[8];


inline	void FIGURE_draw(Thing *p_thing)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG	wx,wy,wz;

	MATRIX r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;
	SLONG	red,green,blue;
	SLONG	sort_z_offset;
	SLONG	directional_light;

/*	
	if (p_thing->Draw.Tweened->CurrentAnim == ANIM_SIT_DOWN   ||
		p_thing->Draw.Tweened->CurrentAnim == ANIM_SIT_IDLE)
		return;
*/


	if((p_thing->WorldPos.Y)>>8 > NGAMUT_Ymax)
	{
//		debug_count[4]++;
		return;
	}

//	return;

#ifndef FS_ISO9660
	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		return;
	}
//	if (the_display.CurrentPrim>&GDisp_Bucket[BUCKET_MEM-5120])
//		return;

#endif
	//
	// Find the lighting context for this thing.  The FIGURE_draw_prim_tween() function
	// calls LIGHT_get_point(). LIGHT_get_point returns the light on the given
	// point in this context.
	//

	if ((p_thing->Class == CLASS_PERSON)&&(p_thing->Genus.Person->Ware))
		ware_flag=1;
	else
		ware_flag=0;

	//
	// The offset to keep the locked limb in the same place.
	//

	if (dt->Locked)
	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		//
		// Taken from temp.cpp
		//


		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   0, dt->Locked, &x1, &y1, &z1);
		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, 256, dt->Locked, &x2, &y2, &z2);

		dx = x1 - x2;
		dy = y1 - y2;
		dz = z1 - z2;
	}
	else
	{
		dx = 0;
		dy = 0;
		dz = 0;
	}
	{
		SLONG	index1,index2;
		//
		// stuff added for more compression of anims
		//
extern	struct	PrimPoint	*anim_mids; //[256];

		index1=dt->CurrentFrame->XYZIndex;
		index2=dt->NextFrame->XYZIndex;

		if(index1!=index2)
		{
			wx=anim_mids[index1].X+(((anim_mids[index2].X-anim_mids[index1].X)*dt->AnimTween)>>8);
			wy=anim_mids[index1].Y+(((anim_mids[index2].Y-anim_mids[index1].Y)*dt->AnimTween)>>8);
			wz=anim_mids[index1].Z+(((anim_mids[index2].Z-anim_mids[index1].Z)*dt->AnimTween)>>8);

		}
		else
		{
			wx=anim_mids[index1].X;
			wy=anim_mids[index1].Y;
			wz=anim_mids[index1].Z;
		}




	}

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

#ifndef FS_ISO9660
	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}
#endif

	//
	// The rotation matrix of the whole object.
	//
/*
	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		dt->Roll,
	   &r_matrix);
*/
	{
		build_peep_rot_matrix(dt->Angle,-dt->Roll,&r_matrix);
	}


	//
	// calc colour for whole obj
	//
	{
		PSX_POLY_Point *pp=perm_pp_array;
		SLONG	px,py,pz;
		px=(p_thing->WorldPos.X>>8)+wx;
		py=(p_thing->WorldPos.Y>>8)+wy;
		pz=(p_thing->WorldPos.Z>>8)+wz;

//		SetLumi(px>>8,pz>>8);

		sort_z_offset=get_sort_z_bodge(px,pz,p_thing);

//		if ((p_thing->State==STATE_DANGLING)||(p_thing->State==STATE_CLIMB_LADDER))
//			sort_z_offset-=30;

//		if (p_thing==NET_PERSON(0))
		if(1)
		{
			MATRIX temp_matrix;
			{
				SLONG	dx,dy,dz;
				SLONG	px,py,pz;
				px=(p_thing->WorldPos.X>>8)+wx;
				py=(p_thing->WorldPos.Y>>8)+wy;
				pz=(p_thing->WorldPos.Z>>8)+wz;

				dx=abs(px-POLY_cam_x);
				dy=abs(py-POLY_cam_y);
				dz=abs(pz-POLY_cam_z);

				dx=QDIST3(dx,dy,dz);

				if(dx<(7<<8))
				{

					temp_matrix.m[0][0]=2095;temp_matrix.m[0][1]=3000;temp_matrix.m[0][2]=0;
					temp_matrix.m[1][0]=3095;temp_matrix.m[1][1]=0;temp_matrix.m[1][2]=0;
					temp_matrix.m[2][0]=1000;temp_matrix.m[2][1]=0;temp_matrix.m[2][2]=0;
					SetColorMatrix(&temp_matrix);
					directional_light=1;
				}
				else
					directional_light=0;
			}


		}
		else
			directional_light=0;


		pp->World.vx=px-POLY_cam_x;
		pp->World.vy=py-POLY_cam_y;
		pp->World.vz=pz-POLY_cam_z;

		gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

		if(pp->Word.SX<-100 || pp->Word.SX>612 || pp->Word.SY>640 ||pp->Word.SY<-100 ||(pp->Flag&(1<<31)))
		{
//			debug_count[5]++;
			gte_SetRotMatrix(&PSX_view_matrix);
			gte_SetTransMatrix(&PSX_view_matrix);
			return;
		}

		py=getPSXFade(pp->Z);

		if (!ware_flag)
		{
			calc_floor_col(px,pz,&red,&green,&blue);
			red=(red*py)>>7;
			green=(green*py)>>7;
			blue=(blue*py)>>7;
		} else
			red=blue=green=py;
	}

//	if(directional_light)
	{
			red  =MAX(red,96);
			green=MAX(green,96);
			blue =MAX(blue,96);
	}

	//
	// Draw each body part.
	//
	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[dt->MeshID]].StartObject;
//#ifdef	MIKE
//	if(directional_light)
//		if(GAME_TURN&16)
//		directional_light=0;


//	if(directional_light)
//		precalc_z(p_thing,wx,wy,wz,ae1,ae2,dt->AnimTween,&r_matrix,sort_z_offset,ele_count);
//#endif
/*
	{
		CBYTE	str2[6];
extern	FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

		sprintf(str2,"%d",sort_z_offset);

		FONT2D_DrawString_3d(str2,(p_thing->WorldPos.X >> 8),(p_thing->WorldPos.Y >> 8)+128,(p_thing->WorldPos.Z >> 8),0xffffff,512,0);
//			CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

	}
*/


	{
		SLONG	skill=0;
		skill=(GET_SKILL(p_thing));

		if(skill>5)
		{
			if(skill>10)
				skill=2;
			else
				skill=1;
		}
		else
			skill=0;

		ASSERT(skill>=0 && skill<3);



			
		if(p_thing->Genus.Person->PlayerID && (p_thing->Genus.Person->Flags2&FLAG2_PERSON_LOOK))
			skill=-1;

		for (i = 0; i < ele_count; i++)
		{
			SLONG	personid;
			SLONG	index;
			SLONG	id;
			personid=dt->PersonID;
	//		if(directional_light)
				index=i;//draw_order[14-i];
	//		else
	//			index=i;

			if(available_bucket_ram()<2600)
				return;

			

			object_offset = dt->TheChunk->PeopleTypes[personid&0x1f].BodyPart[index];
			id=(personid>>5);
			if(id)
			{
				//
				// draw a gun in the hand
				//

				if((index==7 && id!=2) || (index==10 && id==2))
				{
					FIGURE_draw_prim_tween_lit(
						255 + id,
						(p_thing->WorldPos.X >> 8),
						((p_thing->WorldPos.Y >> 8)), //+5 removed
						(p_thing->WorldPos.Z >> 8),
						dt->AnimTween,
						&ae1[index],
						&ae2[index],
						&r_matrix,
						dx,dy,dz,
						FALSE,wx,wy,wz,red,green,blue,sort_z_offset,0);

				}

			}

	//		if (directional_light)
			{
	//			MATRIX	cml;
	//			POLY_build_tween_matrix(&ae1[index],&ae2[index],dt->AnimTween,&r_matrix,&cml);
				FIGURE_draw_prim_tween_lit(
					start_object + object_offset,
					(p_thing->WorldPos.X >> 8),
					((p_thing->WorldPos.Y >> 8)), //+5 removed
					(p_thing->WorldPos.Z >> 8),
					dt->AnimTween,
					&ae1[index],
					&ae2[index],
					&r_matrix,
					dx,dy,dz,
					directional_light,wx,wy,wz,red,green,blue,sort_z_offset,skill);//,&cml);//store_z[i]);//sort_z_offset);
			}
	/*
			else
				store_z[i]=FIGURE_draw_prim_tween(
					start_object + object_offset,
					(p_thing->WorldPos.X >> 8),
					((p_thing->WorldPos.Y >> 8)), //+5 removed
					(p_thing->WorldPos.Z >> 8),
					dt->AnimTween,
					&ae1[index],
					&ae2[index],
					&r_matrix,
					dx,dy,dz,
					FALSE,wx,wy,wz,red,green,blue,mid_peep_z);//sort_z_offset);
	*/



			//
			// restore normal camera matrix at end of each draw prim tween
			//
			gte_SetRotMatrix(&PSX_view_matrix);
			gte_SetTransMatrix(&PSX_view_matrix);
		}
	}
}

#define	MAX_FIG	16

UWORD	fig_count=0;
ULONG	fig_dist[MAX_FIG];
Thing	*fig_thing[MAX_FIG];	

void sort_figure_queue(void)
{
	SLONG	flip=1;
	SLONG	c0;
	ULONG	*pdist,temp1;
	Thing	**pfthing,*temp2;
	while(flip)
	{
		flip=0;
		pdist=fig_dist;
		pfthing=fig_thing;
		for(c0=0;c0<fig_count-1;c0++)
		{
			if(*pdist>*(pdist+1))
			{
				//swap
				flip=1;

				temp1=*pdist;
				*pdist=*(pdist+1);
				*(pdist+1)=temp1;

				temp2=*pfthing;
				*pfthing=*(pfthing+1);
				*(pfthing+1)=temp2;
			}
			pdist++;
			pfthing++;
		}
	}
}

extern	DrawTween	dead_tween;

void FIGURE_draw_queued(Thing *p_thing,SLONG dist)
{
	ASSERT(p_thing->Draw.Tweened!=&dead_tween);

	if((p_thing->WorldPos.Y)>>8 > NGAMUT_Ymax)
	{
		return;
	}
	if(fig_count>=MAX_FIG)
		return;

	if(p_thing->Genus.Person->PlayerID)
		dist=0;   //players get made nice and close

	fig_dist[fig_count]=dist;
	fig_thing[fig_count]=p_thing;
	fig_count++;
}
extern	UBYTE	remove_dead_people;

extern	SLONG	my_draw_dist;

void	DoFigureDraw(void)
{
	PSX_POLY_Point	holdpp[4];
	perm_pp_array=holdpp;

	SLONG	c0;
//	SLONG	b1,b2;

	sort_figure_queue();

	for(c0=0;c0<fig_count;c0++)
	{
	
//		b1=available_bucket_ram();
//		if(available_bucket_ram()>5000)
			FIGURE_draw(fig_thing[c0]);
//		else
//			remove_dead_people=1;

//		ASSERT(available_bucket_ram()>500);
//		b2=available_bucket_ram();
/*
		if((b1-b2)>highest)
		{
			ASSERT(0);
			highest=(b1-b2);
		}
*/
	}
	fig_count=0;
	if(available_bucket_ram()<3000)
	{
		my_draw_dist-=256;
		SATURATE(my_draw_dist,(10<<8)+128,(20<<8)+128);
	}
}


#ifndef PSX
void FIGURE_draw_reflection(Thing *p_thing, SLONG height)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	MATRIX r_matrix;


	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;

	return;

#ifndef FS_ISO9660
	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR FIGURE_draw_reflection");
		return;
	}
#endif

	//
	// Find the lighting context for this thing.  The FIGURE_draw_prim_tween() function
	// calls LIGHT_get_point(). LIGHT_get_point returns the light on the given
	// point in this context.
	//
#ifndef	PSX
	LIGHT_get_context(THING_NUMBER(p_thing));
#endif

	//
	// The offset to keep the locked limb in the same place.
	//

	if (dt->Locked)
	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		//
		// Taken from temp.cpp
		//


		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   0, dt->Locked, &x1, &y1, &z1);
		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, 256, dt->Locked, &x2, &y2, &z2);

		dx = x1 - x2;
		dy = y1 - y2;
		dz = z1 - z2;
	}
	else
	{
		dx = 0;
		dy = 0;
		dz = 0;
	}

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

#ifndef FS_ISO9660
	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}
#endif

	//
	// The rotation matrix of the whole object.
	//
/*
	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		dt->Roll,
	   &r_matrix);
	   */
	{
		build_rot_matrix(dt->Angle,dt->Tilt,&r_matrix);
	}

	SLONG posx = p_thing->WorldPos.X >> 8;
	SLONG posy = p_thing->WorldPos.Y >> 8;
	SLONG posz = p_thing->WorldPos.Z >> 8;

	//
	// Reflect about y = height.
	//
/*
	posy =  height - (posy - height);
	dy   = -dy;

	r_matrix.M[0][1] = -r_matrix.M[0][1];
	r_matrix.M[1][1] = -r_matrix.M[1][1];
	r_matrix.M[2][1] = -r_matrix.M[2][1];
*/

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[dt->MeshID]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];

		FIGURE_draw_prim_tween(
			start_object + object_offset,
			posx,
			posy,
			posz,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			TRUE);
	}
}
#endif


























