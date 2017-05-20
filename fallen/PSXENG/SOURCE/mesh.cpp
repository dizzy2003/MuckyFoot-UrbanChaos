//
// Drawing rotating prims.
//

#include "game.h"
#include	"c:\fallen\headers\memory.h"
#include "c:\fallen\psxeng\headers\engine.h"
#include "c:\fallen\headers\FMatrix.h"
//#include "aeng.h"
//#include "matrix.h"
//#include "mesh.h"
//#include "c:\fallen\headers\light.h"
//#include "poly.h"
			     

#define	POLY_FLAG_GOURAD		(1<<0)
#define	POLY_FLAG_TEXTURED		(1<<1)
#define	POLY_FLAG_MASKED		(1<<2)
#define	POLY_FLAG_SEMI_TRANS	(1<<3)
#define	POLY_FLAG_ALPHA			(1<<4)
#define	POLY_FLAG_TILED			(1<<5)
#define	POLY_FLAG_DOUBLESIDED	(1<<6)
#define	POLY_FLAG_CLIPPED		(1<<7)

CVECTOR tint_colour[8]={
	{128,128,128,0},
	{255,32,32,0},
	{32,255,32,0},
	{255,255,32,0},
	{32,32,255,0},
	{255,32,255,0},
	{32,255,255,0},
//	{255,0,0,0}};
	{200,150,100,0}};

//extern	UWORD	debug_count[10];

/*************************************************************
 *
 *   Helicopter
 *
 */

void CHOPPER_draw_chopper(Thing *p_chopper) 
{

	Chopper *chopper = CHOPPER_get_chopper(p_chopper);
	SLONG matrix[9], vector[3];


	vector[0]=(chopper->dx/64)&2047; vector[1]=(-chopper->dz/64)&2047; vector[2]=0;
	p_chopper->Draw.Mesh->Roll=vector[0];
	p_chopper->Draw.Mesh->Tilt=vector[1];


    if (!(chopper->target)) 
	{

		FMATRIX_calc(matrix, p_chopper->Draw.Mesh->Angle, p_chopper->Draw.Mesh->Tilt, p_chopper->Draw.Mesh->Roll);

		vector[0]=vector[2]=0; vector[1]=-1;

		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);

		chopper->spotx=p_chopper->WorldPos.X;
		chopper->spotz=p_chopper->WorldPos.Z;

		chopper->spotdx=chopper->spotdz=0;

	} 
	else
	{

		SLONG target_x;
		SLONG target_y;
		SLONG target_z;

		//
		// Look at Darci's body.
		//

		calc_sub_objects_position(
			chopper->target,
			chopper->target->Draw.Tweened->AnimTween,
			0,
		   &target_x,
		   &target_y,
		   &target_z);

		target_x <<= 8;
		target_y <<= 8;
		target_z <<= 8;

		target_x += chopper->target->WorldPos.X;
		target_y += chopper->target->WorldPos.Y;
		target_z += chopper->target->WorldPos.Z;

		SLONG dx, dz, dist;
		SLONG maxspd = chopper->speed<<6;

		if (chopper->spotx > target_x) chopper->spotdx-=(chopper->since_takeoff >> 1);
		if (chopper->spotz > target_z) chopper->spotdz-=(chopper->since_takeoff >> 1);

		if (chopper->spotx < target_x) chopper->spotdx+=(chopper->since_takeoff >> 1);
		if (chopper->spotz < target_z) chopper->spotdz+=(chopper->since_takeoff >> 1);

		if (chopper->spotdx> maxspd) chopper->spotdx= maxspd;
		if (chopper->spotdx<-maxspd) chopper->spotdx=-maxspd;
		if (chopper->spotdz> maxspd) chopper->spotdz= maxspd;
		if (chopper->spotdz<-maxspd) chopper->spotdz=-maxspd;


		chopper->spotx += chopper->spotdx + chopper->dx;
		chopper->spotz += chopper->spotdz + chopper->dz;

		chopper->spotx = (((chopper->spotx*24)+target_x)*10>>8);	// Was 24 and 0.04
		chopper->spotz = (((chopper->spotz*24)+target_z)*10)>>8;

		vector[0]= chopper->spotx - p_chopper->WorldPos.X;

		// unlikely event you're higher than the chopper, it won't aim up at you
		if (target_y>p_chopper->WorldPos.Y)
			vector[1]=0;
		else
		  vector[1]=target_y - p_chopper->WorldPos.Y;

		vector[2]= chopper->spotz - p_chopper->WorldPos.Z;

	}

	MESH_draw_poly(
		p_chopper->Draw.Mesh->ObjectId,
		p_chopper->WorldPos.X >> 8 ,
		p_chopper->WorldPos.Y >> 8,
		p_chopper->WorldPos.Z >> 8 ,
		p_chopper->Draw.Mesh->Angle,
		p_chopper->Draw.Mesh->Tilt,
		p_chopper->Draw.Mesh->Roll,
		NULL,0);

	MESH_draw_poly(
		chopper->rotorprim,
		p_chopper->WorldPos.X >> 8 ,
		p_chopper->WorldPos.Y >> 8,
		p_chopper->WorldPos.Z >> 8 ,
		(p_chopper->Draw.Mesh->Angle+chopper->rotors),
		(p_chopper->Draw.Mesh->Tilt),
		(p_chopper->Draw.Mesh->Roll),
		NULL,0);

/*
		if (chopper->light) 
		{
			SLONG colour;

			colour=(0x66 * chopper->light)/255;
			colour+=(colour<<8);
			colour<<=8;

			CONE_create(
				p_chopper->WorldPos.X>>8,
				p_chopper->WorldPos.Y>>8,
				p_chopper->WorldPos.Z>>8,
				vector[0],
				vector[1],
				vector[2],
				256.0F * 5.0F,
				256.0F,
				colour,
				0x00000000,
				50);

			CONE_intersect_with_map();

			CONE_draw();

		}
*/

}

extern void	calc_floor_col(SLONG x,SLONG z,SLONG *r,SLONG *g,SLONG *b);

ULONG MESH_colour_and;
extern PSX_POLY_Point *perm_pp_array;
extern UWORD floor_psx_col[128][128];
#if 1
extern char *GDisp_Bucket;
#else
extern char GDisp_Bucket[];
#endif

//
// if any of top 16 bits of prim are set then wont clip/fade out
//

SLONG MESH_draw_poly(
		SLONG         prim,
		MAPCO16	      at_x,
		MAPCO16       at_y,
		MAPCO16	      at_z,
		SLONG         i_yaw,
		SLONG         i_pitch,
		SLONG         i_roll,
		LIGHT_Colour *lpc,
		UBYTE fade)
{
	SLONG i;
	SLONG j;

//	SLONG sp;
//	SLONG ep;

	SLONG	sf,ef;

	SLONG	b0;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	PSX_POLY_Point *pp;

	SLONG r,g,b;
	UBYTE	rt,gt,bt;

//	UWORD	col;

	MATRIX	matrix;
	SLONG	z;
	struct	PrimPoint	*point;
	SLONG	tflag,flag;
	UBYTE	*cp;
//	SLONG	flag_all_and=0xffffffff;
	UBYTE	u,v;

	SWORD	p0,p1,p2,p3;

	SLONG	mid_z,sort_z;

	SLONG	bodge_z_back=0;

	SLONG	ret_z[3];
	SLONG	sub;
/*
	SLONG	sort_offset=0;

	if (prim == 105 ||
	prim == 101 ||
	prim == 110 ||
	prim == 89  ||
	prim == 126 ||
	prim == 95  ||
	prim == 102)
	{
		sort_offset=10;
	}
*/
//#define	sort_offset	0

	p_obj = &prim_objects[prim&0xffff];
//	return(0);
//extern	PrimInfo prim_info[256];//MAX_PRIM_OBJECTS];


	bodge_z_back=0;//prim_info[prim].radius>>5;

	if(at_y>NGAMUT_Ymax)
	{
		return(0);
	}
//	if (the_display.CurrentPrim>&GDisp_Bucket[BUCKET_MEM-5120])
//		return(0);
	
extern	void	build_rot_matrix3(SLONG yaw,SLONG pitch,SLONG roll,MATRIX *m);

	build_rot_matrix3(i_yaw,-i_pitch,-i_roll,&matrix);

//	MATRIX_TRANSPOSE(matrix);

	POLY_set_local_rotation(
		at_x,
		at_y,
		at_z,
		&matrix);


	//
	// Rotate all the points into the POLY_buffer and all the
	// shadow points in the POLY_shadow.
	//



	// Convert to Distance

	pp=perm_pp_array;

	pp[0].World.vx=pp[0].World.vy=pp[0].World.vz=0;

	gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&mid_z)

	b0=getPSXFade(pp->P);

	//
	// This is a well dodgy way to pass info, but it keeps stack down and keeps it PC compatible!!! horrah for not giving a shit
	//
	if((prim&0xffff0000)==0)
	{
		if(mid_z>768)
			goto	exit;

		if(mid_z>600)
			sub=mid_z-600;
		else
			sub=0;
	}
	else
	{
//		ASSERT(0);
		sub=0;
	}


	// Clip out if 


	if (b0<=0||b0>=255||pp->Word.SY<-100 ||pp->Word.SX<-100 ||pp->Word.SX>612 || (pp->Flag&(1<<31)))//||pp->Word.SX<-100 ||pp->Word.SX>612)
	{
		PSX_view_matrix.t[0]=0;
		PSX_view_matrix.t[1]=0;
		PSX_view_matrix.t[2]=0;
		gte_SetRotMatrix(&PSX_view_matrix);
		gte_SetTransMatrix(&PSX_view_matrix);

		return(0);
	}
/*
	{
		CBYTE	str[6];
extern	FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

		sprintf(str,"%d",get_z_sort_near(mid_z));

		FONT2D_DrawString_3d(str,at_x,at_y,at_z,0xffffff,512,0);
//			CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

	}
*/

	cp=the_display.CurrentPrim;

	if (!fade)
	{
		calc_floor_col(at_x,at_z,&r,&g,&b);



//		r = (r*b0)>>8;
//		g = (g*b0)>>8;
//		b = (b*b0)>>8;
		if(sub)
		{

			r=MAX(r-sub,32);
			g=MAX(g-sub,32);
			b=MAX(b-sub,32);

		}
		// 2000 clock cycles to reach here
	} else
	{
		SATURATE(b0,0,64);
		b=r=g=b0;
	}

//	rt=(UBYTE)((tint_colour[MESH_colour_and].r*(r))>>8);
//	gt=(UBYTE)((tint_colour[MESH_colour_and].g*(g))>>8);
//	bt=(UBYTE)((tint_colour[MESH_colour_and].b*(b))>>8);

	rt=(UBYTE)((tint_colour[MESH_colour_and].r*(NIGHT_amb_red))>>8);
	gt=(UBYTE)((tint_colour[MESH_colour_and].g*(NIGHT_amb_green))>>8);
	bt=(UBYTE)((tint_colour[MESH_colour_and].b*(NIGHT_amb_blue))>>8);

	SATURATE(rt,32,255);
	SATURATE(gt,32,255);
	SATURATE(bt,32,255);

	//
	// The quads.
	//

	sf=p_obj->StartFace4;
	ef=p_obj->EndFace4;
	p_f4 = &prim_faces4[sf];

	for (i = sf; i < ef; i++)
	{
		SLONG	clip_or,clip_and,tmp;
		UBYTE	draw_flags;
		SLONG	ret_z[4];
		SLONG	ds=0;

		draw_flags=p_f4->DrawFlags;
		
		p0 = p_f4->Points[0];
		p1 = p_f4->Points[2];
		p2 = p_f4->Points[1];
		p3 = p_f4->Points[3];


		pp[0].World.vx=	prim_points[p0].X;	pp[0].World.vy=	prim_points[p0].Y;	pp[0].World.vz=	prim_points[p0].Z;
		pp[1].World.vx=	prim_points[p1].X;	pp[1].World.vy=	prim_points[p1].Y;	pp[1].World.vz=	prim_points[p1].Z;
		pp[2].World.vx=	prim_points[p2].X;	pp[2].World.vy=	prim_points[p2].Y;	pp[2].World.vz=	prim_points[p2].Z;
		pp[3].World.vx=	prim_points[p3].X;	pp[3].World.vy=	prim_points[p3].Y;	pp[3].World.vz=	prim_points[p3].Z;
		//pp[0].Z=RotAverage4(&pp[0].World,&pp[1].World,&pp[2].World,&pp[3].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[3].SYSX,&pp[0].P,&pp[0].Flag);
//		gte_RotTransPers3(&pp[0].World,&pp[1].World,&pp[2].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		//#define gte_RotTransPers3(r1,r2,r3,r4,r5,r6,r7,r8,r9)		

		{
			gte_ldv3(&pp[0].World,&pp[1].World,&pp[2].World);	
			gte_rtpt();		

			/* CPU 22 ticks available */
			pp[3].World.vx=	prim_points[p3].X;	pp[3].World.vy=	prim_points[p3].Y;	pp[3].World.vz=	prim_points[p3].Z;

			gte_stsxy3(&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX);
			gte_stdp(&pp[0].P);
			gte_stflg(&pp[0].Flag);
			gte_stsz3c(&ret_z[0]);
//			gte_stszotz(&pp[0].Z);	 not needed
		}



		clip_and=pp[0].Flag;

//		clip_or=pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag;
		//clip_and=pp[0].Flag&pp[1].Flag&pp[2].Flag&pp[3].Flag;
	//	clip_and=pp[0].Flag;


//		if (POLY_valid_quadp(quad,!(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED)))
		
		if((clip_and&(1<<31))==0)
		{
			ds=(draw_flags & POLY_FLAG_DOUBLESIDED);

			if (ds||(MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX)<0))
			{
				gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);



				//if (draw_flags & POLY_FLAG_TEXTURED)
				{
					if ((MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>=0)&&
						(MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)&&
						(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>=0)&&
						(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256))
					{
						POLY_FT4	*p;

						SLONG	page,z;

						check_prim_ptr((void**)&cp);
						p=(POLY_FT4 *)cp; //the_display.CurrentPrim;

						setPolyFT4(p);

						page=p_f4->TexturePage;

						setUV4(p,p_f4->UV[0][0],p_f4->UV[0][1],
								 p_f4->UV[2][0],p_f4->UV[2][1],
								 p_f4->UV[1][0],p_f4->UV[1][1],
								 p_f4->UV[3][0],p_f4->UV[3][1]);

	//					if (b0!=128)
	//						setSemiTrans(p,1);

						if (p_f4->FaceFlags &  FACE_FLAG_TINT)
							setRGB0(p,rt,gt,bt);
						else
							setRGB0(p,r,g,b);

						setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
								 pp[1].Word.SX,pp[1].Word.SY,
								 pp[2].Word.SX,pp[2].Word.SY,
								 pp[3].Word.SX,pp[3].Word.SY);

						p->tpage=getPSXTPage(page);
						p->clut=getPSXClut(page);
						pp[3].Z<<=2;
						z=MAX4(ret_z[0],ret_z[1],ret_z[2],pp[3].Z)>>2;

						if (p_f4->FaceFlags &  FACE_FLAG_VERTICAL)
							z+=6;
	//					z=pp[0].Z>>1;

						if (draw_flags & POLY_FLAG_ALPHA)
						{
							setSemiTrans(p,1);
							p->tpage|=(3)<<5;
						}
						if (p_obj->flag & PRIM_FLAG_ENVMAPPED)
						{
							// Add Env Mapping crap here
						}
						if(sub)
						{
							setSemiTrans(p,1);
							p->tpage&=~(3<<5);


						}

							z=get_z_sort(z);//+sort_offset);//_near(z);//+bodge_z_back);
							addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						cp+=sizeof(POLY_FT4);

					}
				}
	#if 0
				else
				{
					{
						POLY_F4	*p;
						r = ENGINE_palette[p_f4->Col].red;
						g = ENGINE_palette[p_f4->Col].green;
						b = ENGINE_palette[p_f4->Col].blue;

	//					if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
	//						ASSERT(0);

						check_prim_ptr((void**)&cp);
						p=(POLY_F4 *)cp; //the_display.CurrentPrim;

						setPolyF4(p);
						setRGB0(p,r,g,b);
						setXY4(p,quad[0]->Word.SX,quad[0]->Word.SY,
								quad[1]->Word.SX,quad[1]->Word.SY,
								quad[2]->Word.SX,quad[2]->Word.SY,
								quad[3]->Word.SX,quad[3]->Word.SY);

						z=MAX4(quad[0]->Z,quad[1]->Z,quad[2]->Z,quad[3]->Z)>>0;
						z=get_z_sort(z);//_near(z);

						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						cp+=sizeof(POLY_F4);

					}
				}
	#endif
			}
		}
next_quad:;
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
		SLONG	clip_or,clip_and;
		UBYTE	draw_flags;

//		printf(".");
		draw_flags=p_f3->DrawFlags;

		p0 = p_f3->Points[0];
		p1 = p_f3->Points[2];
		p2 = p_f3->Points[1];

		pp[0].World.vx=	prim_points[p0].X;	pp[0].World.vy=	prim_points[p0].Y;	pp[0].World.vz=	prim_points[p0].Z;
		pp[1].World.vx=	prim_points[p1].X;	pp[1].World.vy=	prim_points[p1].Y;	pp[1].World.vz=	prim_points[p1].Z;
		pp[2].World.vx=	prim_points[p2].X;	pp[2].World.vy=	prim_points[p2].Y;	pp[2].World.vz=	prim_points[p2].Z;
#ifdef	GOOD_SORT
		gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
		gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
		clip_and=pp[0].Flag&pp[1].Flag&pp[2].Flag;
#else
//		gte_RotAverage3(&pp[0].World,&pp[1].World,&pp[2].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
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


#endif
		if((clip_and&(1<<31))==0)
		if ((draw_flags & POLY_FLAG_DOUBLESIDED) || MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX)<0)
		{
			if (draw_flags & POLY_FLAG_TEXTURED)
			{
				if ((max(pp[0].Word.SX,max(pp[1].Word.SX,pp[2].Word.SX))>=0)&&
				    (min(pp[0].Word.SX,min(pp[1].Word.SX,pp[2].Word.SX))<DISPLAYWIDTH)&&
				    (max(pp[0].Word.SY,max(pp[1].Word.SY,pp[2].Word.SY))>=0)&&
				    (min(pp[0].Word.SY,min(pp[1].Word.SY,pp[2].Word.SY))<256))
				{
					POLY_FT3	*p;

					SLONG	page,z;

					check_prim_ptr((void**)&cp);
					p=(POLY_FT3 *)cp; //the_display.CurrentPrim;

					setPolyFT3(p);

					page=p_f3->TexturePage;

					setUV3(p,p_f3->UV[0][0],p_f3->UV[0][1],
							 p_f3->UV[2][0],p_f3->UV[2][1],
							 p_f3->UV[1][0],p_f3->UV[1][1]);

					setXY3(p,pp[0].Word.SX,pp[0].Word.SY,
							pp[1].Word.SX,pp[1].Word.SY,
							pp[2].Word.SX,pp[2].Word.SY);


	//				if (fade)
	//					setSemiTrans(p,1);

					if (p_f3->FaceFlags &  FACE_FLAG_TINT)
						setRGB0(p,rt,gt,bt);
					else
						setRGB0(p,r,g,b);

//					if (b0!=128)
//						setSemiTrans(p,1);

					//z=pp[0].Z>>0;//max(pp[0].Z,max(pp[1].Z,pp[2].Z))>>1;
					z=MAX3(ret_z[0],ret_z[1],ret_z[2])>>2;
					if (p_f3->FaceFlags &  FACE_FLAG_VERTICAL)
						z+=6;


					p->tpage=getPSXTPage(page);
					p->clut=getPSXClut(page);

					if (draw_flags & POLY_FLAG_ALPHA)
					{
						setSemiTrans(p,1);
						p->tpage|=(3)<<5;
					}
					if (p_obj->flag & PRIM_FLAG_ENVMAPPED)
					{
						// Add Env Mapping crap here
					}

					if(sub)
					{
						setSemiTrans(p,1);
						p->tpage&=~(3<<5);


					}

					z=get_z_sort(z);//+sort_offset);//_near(z);//+bodge_z_back);
					addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
					cp+=sizeof(POLY_FT3);
				}
			}
#if 0
			else
			{
					POLY_F3	*p;
					r = ENGINE_palette[p_f3->Col].red;
					g = ENGINE_palette[p_f3->Col].green;
					b = ENGINE_palette[p_f3->Col].blue;

//					if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
//						ASSERT(0);

					check_prim_ptr((void**)&cp);
					p=(POLY_F3 *)cp; //the_display.CurrentPrim;

					setPolyF3(p);
					setRGB0(p,r,g,b);
					setXY3(p,quad[0]->Word.SX,quad[0]->Word.SY,
						quad[1]->Word.SX,quad[1]->Word.SY,
						quad[2]->Word.SX,quad[2]->Word.SY);		 
 
					z=MAX3(ret_z[0],ret_z[1],ret_z[2])>>2;
					z=get_z_sort(z);//_near(z);

					addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
					cp+=sizeof(POLY_F3);
			}
#endif
		}
		p_f3++;
	}

	check_prim_ptr((void**)&cp);
	the_display.CurrentPrim=cp; //sizeof(POLY_FT3);		 
exit:;
	PSX_view_matrix.t[0]=0;
	PSX_view_matrix.t[1]=0;
	PSX_view_matrix.t[2]=0;
	gte_SetRotMatrix(&PSX_view_matrix);
	gte_SetTransMatrix(&PSX_view_matrix);


	return(1);

}






