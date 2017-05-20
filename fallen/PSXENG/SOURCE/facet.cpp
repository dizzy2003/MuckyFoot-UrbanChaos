/*

  // that's c++ that is

class	ChooseZero
{
	public:
		SLONG	Query()	{	return	0;	}
};

class	ChooseOne
{
	public:
		SLONG	Query()	{	return	1;	}
};

DrawT( blah, blah, ChooseOne() );

template <class hilite> DrawT( M31 *draw_pos, Mesh *the_mesh, hilite )
{
	if( hilite::Query() )
	{
		DrawHilite();
	}
	else
	{
		Draw();
	}
}	 




*/

#include	"game.h"
#include	"c:\fallen\headers\memory.h"

#include	"psxeng.h"		   
#include	"poly.h"
#include	"c:\fallen\headers\supermap.h"
#include	"c:\fallen\headers\inside2.h"
#include	"night.h"
#include	"c:\fallen\headers\pap.h"
//#include	"math.h"
#include	"libgpu.h"
#include	"c:\fallen\headers\fc.h"
#include "c:\fallen\psxeng\headers\engine.h"
#include	"c:\fallen\headers\ware.h"

// For Filtering we need the pad input crap here.

#include	"ctrller.h"
extern ControllerPacket PAD_Input1,PAD_Input2;

// **********************************************

#define	KERB_HEIGHT	32

extern PSX_POLY_Point *perm_pp_array;
extern	UWORD	*psx_remap;
SLONG dfacets_drawn_this_gameturn;
extern	void	cable_draw(struct DFacet *p_facet);

static	ULONG	facet_seed=0x12345678;

void	DRAW_ladder(struct DFacet *p_facet);
extern	UWORD	debug_count[10];
void	fuck_z(PSX_POLY_Point *pp);
void build_split_face_2quad(PSX_POLY_Point *pp,SLONG page,POLY_GT4 *q,SLONG z);
SLONG build_split_face_4quad(PSX_POLY_Point *pp,SLONG page,POLY_GT4 *q,SLONG z);

inline	SLONG	grid_height_at(SLONG mx,SLONG mz)
{
	SLONG	dy;
	PAP_Hi	*pap;
	pap=&PAP_hi[mx][mz];
	dy=(SLONG)(pap->Alt<<3);
/*
	if (pap->Flags & PAP_FLAG_SINK_SQUARE)
	{
		dy -= KERB_HEIGHT;
	}
*/

	return(dy);
}

inline	SLONG	grid_height_at_world(SLONG x,SLONG z)
{
	return(grid_height_at( ((SLONG)x)>>8,((SLONG)z)>>8) );
}

inline	ULONG	facet_rand(void)
{
	facet_seed=(facet_seed*12345678)+12345678;
//	LogText(" build_seed %x \n",build_seed);
//	facet_seed = facet_seed * 328573 + 123456789;

	return(facet_seed>>16);
}

void	set_facet_seed(SLONG seed)
{
	facet_seed=seed;
}

/*
SLONG	texture_quad(POLY_Point *quad[4],SLONG texture_style,SLONG pos,SLONG count)
{
	SLONG	tx,ty;
	SLONG	page;
	SLONG	texture_piece;
	SLONG   rand;
	SLONG	flip=0;

	rand = facet_rand() & 0x3;
	if(pos==0)
		texture_piece=TEXTURE_PIECE_RIGHT;
	else
	if(pos==count-2)
		texture_piece=TEXTURE_PIECE_LEFT;
	else
	{
		static const UBYTE choice[4] =
		{
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE1,
			TEXTURE_PIECE_MIDDLE2
		};

		texture_piece = choice[rand];
	}
	if(texture_style<0)
	{
		SLONG	index;
		struct	DStorey *p_storey;

		p_storey=&dstoreys[-texture_style];

		index=p_storey->Index;
		ASSERT(p_storey->Count);
		
		if(pos<=p_storey->Count)
		{
			page=paint_mem[index+pos];
			if(page==0)
			{
				texture_style = p_storey->Style;
			}
			else
			{
				ty=(page&0x38)<<2;
				tx=(page&7)<<5;
				page=(page&0xfc0)>>6;
				goto	got_texture; //goto's are great!!
			}
		}
		else
		{
			texture_style = p_storey->Style;
		}
		flip=0;
	}

	if(texture_style>=0)
	{
		page=textures_xy[texture_style][texture_piece].Page;
		tx=textures_xy[texture_style][texture_piece].Tx<<5;
		ty=textures_xy[texture_style][texture_piece].Ty<<5;
		flip=textures_xy[texture_style][texture_piece].Flip;
	}


	ASSERT(page<10);

got_texture:;
//	p4->DrawFlags=textures_flags[texture_style][texture_piece];

	switch(flip) //textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			quad[1]->u = tx+31;
			quad[1]->v = ty+0;
			quad[0]->u = tx+0;
			quad[0]->v = ty+0;
			quad[3]->u = tx+31;
			quad[3]->v = ty+31;
			quad[2]->u = tx+0;
			quad[2]->v = ty+31;

			break;
		case	1: //flip x
			quad[1]->u = tx+0;
			quad[1]->v = ty+0;
			quad[0]->u = tx+31;
			quad[0]->v = ty+0;
			quad[3]->u = tx+0;
			quad[3]->v = ty+31;
			quad[2]->u = tx+31;
			quad[2]->v = ty+31;

			break;
		case	2: //flip y
			quad[1]->u = tx+31;
			quad[1]->v = ty+31;
			quad[0]->u = tx+0;
			quad[0]->v = ty+31;
			quad[3]->u = tx+31;
			quad[3]->v = ty+0;
			quad[2]->u = tx+0;
			quad[2]->v = ty+0;

			break;
		case	3: //flip x+y
			quad[1]->u = tx+0;
			quad[1]->v = ty+31;
			quad[0]->u = tx+31;
			quad[0]->v = ty+31;
			quad[3]->u = tx+0;
			quad[3]->v = ty+0;
			quad[2]->u = tx+31;
			quad[2]->v = ty+0;

			break;
	}


	return(page);

}

SLONG	texture_quad2(POLY_Point *quad[4],SLONG texture_style,SLONG texture_piece)
{
	SLONG	tx,ty;
	SLONG	page;
//	SLONG   rand;

	page=textures_xy[texture_style][texture_piece].Page;
	tx=textures_xy[texture_style][texture_piece].Tx<<5;
	ty=textures_xy[texture_style][texture_piece].Ty<<5;

//	p4->DrawFlags=textures_flags[texture_style][texture_piece];

	switch(textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			quad[1]->u = tx+31;
			quad[1]->v = ty+0;
			quad[0]->u = tx+0;
			quad[0]->v = ty+0;
			quad[3]->u = tx+31;
			quad[3]->v = ty+31;
			quad[2]->u = tx+0;
			quad[2]->v = ty+31;

			break;
		case	1: //flip x
			quad[1]->u = tx+0;
			quad[1]->v = ty+0;
			quad[0]->u = tx+31;
			quad[0]->v = ty+0;
			quad[3]->u = tx+0;
			quad[3]->v = ty+31;
			quad[2]->u = tx+31;
			quad[2]->v = ty+31;

			break;
		case	2: //flip y
			quad[1]->u = tx+31;
			quad[1]->v = ty+31;
			quad[0]->u = tx+0;
			quad[0]->v = ty+31;
			quad[3]->u = tx+31;
			quad[3]->v = ty+0;
			quad[2]->u = tx+0;
			quad[2]->v = ty+0;

			break;
		case	3: //flip x+y
			quad[1]->u = tx+0;
			quad[1]->v = ty+31;
			quad[0]->u = tx+31;
			quad[0]->v = ty+31;
			quad[3]->u = tx+0;
			quad[3]->v = ty+0;
			quad[2]->u = tx+31;
			quad[2]->v = ty+0;

			break;
	}

		  

//	LogText("quad text page %d \n",page);

	return(page);

}
SLONG	texture_tri2(POLY_Point *quad[3],SLONG texture_style,SLONG texture_piece)
{
	SLONG	tx,ty;
	SLONG	page;
//	SLONG   rand;

	page=textures_xy[texture_style][texture_piece].Page;
	switch(textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			quad[1]->u = 32;
			quad[1]->v = 0;
			quad[0]->u = 0;
			quad[0]->v = 0;
			quad[2]->u = 0;
			quad[2]->v = 32;

			break;
		case	1: //flip x
			quad[1]->u = 0;
			quad[1]->v = 0;
			quad[0]->u = 32;
			quad[0]->v = 0;
			quad[2]->u = 32;
			quad[2]->v = 32;

			break;
		case	2: //flip y
			quad[1]->u = 32;
			quad[1]->v = 32;
			quad[0]->u = 0;
			quad[0]->v = 32;
			quad[2]->u = 0;
			quad[2]->v = 0;

			break;
		case	3: //flip x+y
			quad[1]->u = 0;
			quad[1]->v = 32;
			quad[0]->u = 32;
			quad[0]->v = 32;
			quad[2]->u = 32;
			quad[2]->v = 0;

			break;
	}

		  

//	LogText("quad text page %d \n",page);

	return(page);

}
*/
//
// should work for any angle
//
void	build_fence_poles(SLONG sx,SLONG sy,SLONG sz,SLONG fdx,SLONG fdz,SLONG count,SLONG *rdx,SLONG *rdz,SLONG style)
{
#ifdef GOTTA_DO_A_BETTA_JOB
	SLONG	x[13],y[13],z[13];
	SLONG	dx,dz,nx,nz,dist;
	SLONG	gx,gy,gz;
	POLY_Point   *quad[4];
	POLY_Point   *tri[3];
  	POLY_Point   *pp;

	SLONG	dy;

	NIGHT_Colour col;


	col.red=64;
	col.green=64;
	col.blue=64;


	x[0]=0;
	y[0]=0;
	z[0]=0;

	dist=Root(fdx*fdx+fdz*fdz);
	if(dist==0)
		return;

	dx=(fdx)/dist;
	dz=(fdz)/dist;

	*rdx=dx;
	*rdz=dz;

	nx=(dz*10);// /1024;
	nz=-(dx*10); ///1204;

	x[1]=dx*20;
	y[1]=0;
	z[1]=dz*20;

	x[2]=dx*10+nx;
	y[2]=0;
	z[2]=dz*10+nz;


	gx=sx;
	gy=sy;
	gz=sz;
	return;
	while(count-->0)
	{
		SLONG	c0;
		
//		POLY_buffer_upto = 0;
		pp = &POLY_buffer[0];

		dy=	grid_height_at_world(gx,gz);

		//
		// 3 points at base of pilla
		//
		for(c0=0;c0<3;c0++)
		{
			POLY_transform(gx+x[c0],gy+y[c0]+dy,gz+z[c0],pp);
			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
/*
				NIGHT_get_d3d_colour(
								col,
								&pp->colour,
								&pp->specular);

				POLY_fadeout_point(pp);
				*/
			}
			pp++;
		}

		y[2]=-10;
		// 3 points at middle
		for(c0=0;c0<3;c0++)
		{
			POLY_transform(gx+x[c0],gy+y[c0]+dy+200,gz+z[c0],pp);
			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
				/*
				NIGHT_get_d3d_colour(
								col,
								&pp->colour,
								&pp->specular);
				POLY_fadeout_point(pp);
				  */

			}
			pp++;
		}
		y[2]=0;

		// point at top
		POLY_transform(gx+x[2]+nx*5,gy+y[2]+dy+250,gz+z[2]+nz*5,pp);
		if (pp->clip & POLY_CLIP_TRANSFORMED)
		{
		   /*
			NIGHT_get_d3d_colour(
							col,
							&pp->colour,
							&pp->specular);

			POLY_fadeout_point(pp);
			*/

		}
		pp++;



		//
		// 3 quads and 3 tris
		//

		//   0	   3
		//	   2	 5     6
		//	 1	   4
		{
			SLONG	q,t;
			SLONG	q_lookup[]={1,2,0};

			for(q=0;q<3;q++)
			{
				
				quad[0] = &POLY_buffer[3+q];
				quad[1] = &POLY_buffer[3+q_lookup[q]];
				quad[2] = &POLY_buffer[0+q];
				quad[3] = &POLY_buffer[0+q_lookup[q]];

				if (POLY_valid_quadp(quad,1))
				{
					SLONG	page;
					
					//
					// Texture the quad.
					// 

					page=texture_quad2(quad,dstyles[style],TEXTURE_PIECE_TOP_LEFT);

//					POLY_add_quad(quad, page, 1); // 1 means perform a backface cull
				}

			}



			tri[2] = &POLY_buffer[6];
			for(t=0;t<3;t++)
			{
				
				tri[1] = &POLY_buffer[3+t];
				tri[0] = &POLY_buffer[3+q_lookup[t]];

				if (POLY_valid_trianglep(tri,1))
				{
					SLONG	page=0;
					
					//
					// Texture the quad.
					// 

					page=texture_tri2(tri,dstyles[style],TEXTURE_PIECE_TOP_LEFT);

//					poly_add_trianglep(tri, page, 1); // 1 means perform a backface cull
				}

			}
			

		}

		gx+=fdx;
		gz+=fdz;

		// end while
	}

#endif

}
/*
void	draw_wall_thickness(struct DFacet *p_facet,ULONG fade_alpha)
{
  	PSX_POLY_Point   *pp=perm_pp_array;
	SLONG	x1,y1,z1,x2,z2;
	SLONG	dx,dz,dist,sdx,sdz;
	POLY_F4 *p;

	x1=(p_facet->x[0]<<8)-POLY_cam_x;
	z1=(p_facet->z[0]<<8)-POLY_cam_z;
	y1=(p_facet->Y[0]+256)-POLY_cam_y;

	x2=(p_facet->x[1]<<8)-POLY_cam_x;
	z2=(p_facet->z[1]<<8)-POLY_cam_z;

	dz= (x2-x1);
	dx= (z2-z1);

	dist=abs(dx)+abs(dz);

	sdx=(dz<<8)/dist;
	sdz=(dx<<8)/dist;

	dx=(dx*10)/dist;
	dz=(dz*10)/dist;

	pp[0].World.vx=x1+dx;
	pp[0].World.vy=y1;
	pp[0].World.vz=z1+dz;

	pp[1].World.vx=x1-dx;
	pp[1].World.vy=y1;
	pp[1].World.vz=z1-dz;

	p=(POLY_F4*)the_display.CurrentPrim;

	while(dist>0)
	{
		pp[2].World.vx=pp[0].World.vx+sdx;
		pp[2].World.vy=y1;
		pp[2].World.vz=pp[0].World.vz+sdz;
	
		pp[3].World.vx=pp[1].World.vx+sdx;
		pp[3].World.vy=y1;
		pp[3].World.vz=pp[1].World.vz+sdz;


		gte_RotTransPers4(&pp[0].World,&pp[1].World,&pp[2].World,&pp[3].World,
						 (SLONG*)&p->x0,(SLONG*)&p->x1,(SLONG*)&p->x2,(SLONG*)&p->x3,&pp[0].P,&pp->Flag,&pp->Z);

//		 if ((MAX4(p->x0,p->x1,p->x2,p->x3)<0)||
//		 	(MIN4(p->x0,p->x1,p->x3,p->x3)>511)||
//	 	 	(MAX4(p->y0,p->y1,p->y2,p->y3)<0)||
//		 	(MIN4(p->y0,p->y1,p->y2,p->y3)>255))
		{
			 setPolyF4(p);
			 if (fade_alpha)
			 {
				 setRGB0(p,128-(fade_alpha>>1),128-(fade_alpha>>1),128-(fade_alpha>>1));
				 setSemiTrans(p,1);
			 } else
				 setRGB0(p,255,255,255);
			 pp[0].Z=get_z_sort(pp[0].Z>>0);
			 DOPRIM(pp[0].Z,p);
			 p++;
		 }
		 pp[0].World.vx=pp[2].World.vx;
		 pp[0].World.vz=pp[2].World.vz;
		 pp[1].World.vx=pp[3].World.vx;
		 pp[1].World.vz=pp[3].World.vz;
		 the_display.CurrentPrim=(UBYTE*)p;
		 dist-=abs(sdx)+abs(sdz);
	}
}
*/

/*
void	DRAW_stairs(SLONG stair,SLONG storey,UBYTE fade)
{
	SLONG	x,y,z;
	SLONG	prim=0;
	SLONG	dir,angle;

	x=inside_stairs[stair].X<<8;
	z=inside_stairs[stair].Z<<8;

	y=inside_storeys[storey].StoreyY;

	if(inside_stairs[stair].UpInside)
		prim|=1;

	if(inside_stairs[stair].DownInside)
		prim|=2;

	switch(prim)
	{
		case	1:
			prim=27;
			break;
		case	2:
			prim=29;
			break;
		case	3:
			prim=28;
			break;
	}

	dir=GET_STAIR_DIR(inside_stairs[stair].Flags);
	switch(dir)
	{
		case	0:
			//n
			angle=0;
			break;
		case	1:
			//e
			angle=2048-512;
			break;
		case	2:
			//s
			angle=1024;
			break;
		case	3:
			//w
			angle=512;
			break;


	}


	if(prim)
	{
		MESH_draw_poly(prim,x,y,z,angle,0,0,0,fade);
	}
}
*/
void FACET_draw(SLONG facet,UBYTE alpha);
/*
void	draw_insides(SLONG indoor_index,SLONG room,UBYTE fade)
{
	struct	InsideStorey	*p_inside;
	SLONG	c0;
	static	recursive=0;
	SLONG	stair;
	ASSERT(recursive==0)

	recursive++;
	p_inside=&inside_storeys[indoor_index];
//	MSG_add(" fade %d \n",INDOORS_INDEX_FADE);
	for(c0=p_inside->FacetStart;c0<p_inside->FacetEnd;c0++)
	{
		FACET_draw(c0,fade);
	}
	stair=inside_storeys[indoor_index].StairCaseHead;
	while(stair)
	{
		DRAW_stairs(stair,indoor_index,fade);
		stair=inside_stairs[stair].NextStairs;
	}
	recursive--;
}
*/

#if 0
void AENG_add_fade(PSX_POLY_Point *pp,SLONG z)
{
	POLY_F4 *p;
	ALLOCPRIM(p,POLY_F4);
	setPolyF4(p);
	setSemiTrans(p,1);
	setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
			 pp[1].Word.SX,pp[1].Word.SY,
			 pp[2].Word.SX,pp[2].Word.SY,
			 pp[3].Word.SX,pp[3].Word.SY);
	setRGB0(p,NIGHT_sky_colour.red,NIGHT_sky_colour.green,NIGHT_sky_colour.blue);
	DOPRIM(z,p);
}

void AENG_add_semi_fade(PSX_POLY_Point *pp,UWORD b0,UWORD b1,UWORD b2,UWORD b3,SLONG z)
{
	POLY_G4 *p;
	UBYTE r,g,b;

	r=NIGHT_sky_colour.red;
	g=NIGHT_sky_colour.green;
	b=NIGHT_sky_colour.blue;

	ALLOCPRIM(p,POLY_G4);
	setPolyG4(p);
	setSemiTrans(p,1);
	setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
			 pp[1].Word.SX,pp[1].Word.SY,
			 pp[2].Word.SX,pp[2].Word.SY,
			 pp[3].Word.SX,pp[3].Word.SY);
	if (b0!=0)
		setRGB0(p,r,g,b);//NIGHT_amb_red,NIGHT_amb_green,NIGHT_amb_blue);
	else
		setRGB0(p,0,0,0);

	if (b1!=0)
		setRGB1(p,r,g,b);//NIGHT_amb_red,NIGHT_amb_green,NIGHT_amb_blue);
	else
		setRGB1(p,0,0,0);

	if (b2!=0)
		setRGB2(p,r,g,b);//NIGHT_amb_red,NIGHT_amb_green,NIGHT_amb_blue);
	else	  
		setRGB2(p,0,0,0);

	if (b3!=0)
		setRGB3(p,r,g,b);//NIGHT_amb_red,NIGHT_amb_green,NIGHT_amb_blue);
	else
		setRGB3(p,0,0,0);
	DOPRIM(z,p);
}
#endif

//   0     1
//
//   2
//         3
SLONG	texture_psx_quad_gt4(POLY_GT4 *prim,SLONG texture_style,SLONG pos,SLONG count,SLONG t_width,SLONG t_height,SLONG t_x_offset);

void	draw_facet_foundation(SLONG sx,SLONG sy,SLONG sz,SLONG fdx,SLONG fdz,SLONG count,SLONG bheight,NIGHT_Colour *col,SLONG style_index,UBYTE *shadow_byte,SLONG shadow_shift,SLONG *drawn,SLONG black)
{

  	SLONG			hf;
  	PSX_POLY_Point  *pp = perm_pp_array;
//	PSX_POLY_Point  *quad[4];
//	SLONG	x,y;
	SLONG	z,px,pz;
	SLONG	c0,b0,lum;
	SLONG	dy1,dy2;

	hf=0;
//	sy-=fheight<<6;
//	height=(SLONG)(fheight<<6);

	px=(sx+POLY_cam_x)>>8;
	pz=(sz+POLY_cam_z)>>8;
	lum=LUMI(px,pz)<<8;

//	while(hf<1)
	{
		pp[0].World.vx=	sx;
		pp[0].World.vy=	sy+bheight;
		pp[0].World.vz=	sz;

		pp[1].World.vx=	sx+fdx;
		pp[1].World.vy=	sy+bheight;
	 	pp[1].World.vz=	sz+fdz;

		pp[2].World.vx=	sx;
		pp[2].World.vy=	(PAP_2HI((sx+POLY_cam_x)>>8,(sz+POLY_cam_z)>>8).Alt<<3)-POLY_cam_y;
		pp[2].World.vz=	sz;


		pp[3].World.vx=	sx+fdx;
		pp[3].World.vy=	(PAP_2HI(((sx+POLY_cam_x)+fdx)>>8,(sz+fdz+POLY_cam_z)>>8).Alt<<3)-POLY_cam_y;
		pp[3].World.vz=	sz+fdz;
		dy1=(sy+bheight)-(pp[2].World.vy);
		dy2=(sy+bheight)-(pp[3].World.vy);
		if(dy1>255)
			dy1=255;
		if(dy2>255)
			dy2=255;
/*
		pp[0].World.vx=sx;
		pp[0].World.vy=sy+bheight;
		pp[0].World.vz=sz;
		pp[1].World.vx=sx+fdx;
		pp[1].World.vy=sy; //PAP_2HI(sx>>8,sz>>8).Alt<<3;
		pp[1].World.vz=sz+fdz;
*/

		gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
		c0=count-1;
		while(c0-->0)
		{
			px+=(fdx>>8);
			pz+=(fdz>>8);
			lum=(lum>>8)+(LUMI(px,pz)<<8);

			gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
			gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

			b0=getPSXFade(pp[0].P);

			if ((b0>0)&&!((pp[0].Flag&pp[1].Flag&pp[2].Flag&pp[3].Flag)&(1<<31)))
			{
				if ((MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)&&
					(MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>=0)&&
					(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256)&&
					(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>=0))
				{
					POLY_GT4 *p;
					SLONG	page;

					(*drawn)++;
					ALLOCPRIM(p,POLY_GT4);
					setPolyGT4(p);


					SWORD	b1,b2,b3;
					b1=getPSXFade(pp[1].P);
					b2=getPSXFade(pp[2].P);
					b3=getPSXFade(pp[3].P);

					z=MAX4(pp[0].Z,pp[1].Z,pp[2].Z,pp[3].Z)>>0;
					z=get_z_sort(z);

#if 0
					if ((b0!=128)&&(b1!=128)&&(b2!=128)&&(b3!=128))
					{
						setSemiTrans(p,1);
//							AENG_add_fade(pp,z);
					}
					else if ((b0!=128)||(b1!=128)|(b2!=128)|(b3!=128))
					{
						AENG_add_semi_fade(pp,128-b0,128-b1,128-b2,128-b3,z);
					} 
#endif
					if(black)
					{
						setRGB0(p,0,0,0);
						setRGB1(p,0,0,0);
						setRGB2(p,0,0,0);
						setRGB3(p,0,0,0);

					}
					else
					{
						setRGB0(p,MAKELUMI(((col+count)->red)*b0>>8,lum&0xff),
								  MAKELUMI((col+count)->green*b0>>8,lum&0xff),
								  MAKELUMI((col+count)->blue*b0>>8,lum&0xff));
						setRGB2(p,MAKELUMI((col+0)->red*b2>>8,lum&0xff),
								  MAKELUMI((col+0)->green*b2>>8,lum&0xff),
								  MAKELUMI((col+0)->blue*b2>>8,lum&0xff));

						setRGB1(p,MAKELUMI((col+count+1)->red*b1>>8,lum>>8),
								  MAKELUMI((col+count+1)->green*b1>>8,lum>>8),
								  MAKELUMI((col+count+1)->blue*b1>>8,lum>>8));
						setRGB3(p,MAKELUMI((col+1)->red*b3>>8,lum>>8),
								  MAKELUMI((col+1)->green*b3>>8,lum>>8),
								  MAKELUMI((col+1)->blue*b3>>8,lum>>8));
					}

					page=texture_psx_quad_gt4(p,style_index,c0,count,63,63,0);
					// Bodge job to shift coords to correct position

					p->v2=p->v0+(dy1>>2);
					p->v3=p->v1+(dy2>>2);
					if(z>700)
					{
						build_split_face_4quad(pp,page,p,z);
					}
					else
					if(z>600)
					{
						build_split_face_2quad(pp,page,p,z);
					}
					else
					{
						setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
							pp[1].Word.SX,pp[1].Word.SY,
							pp[2].Word.SX,pp[2].Word.SY,
							pp[3].Word.SX,pp[3].Word.SY);
						DOPRIM(z,p);

					}

				} else 
					facet_rand();
			} else
				facet_rand();
				
			col += 1;

			pp[0].SYSX=pp[1].SYSX;
			pp[0].Z=pp[1].Z;
			pp[0].P=pp[1].P;
			pp[0].Flag=pp[1].Flag;
			pp[0].World.vx=pp[1].World.vx;
			pp[0].World.vy=pp[1].World.vy; //just incase there's a split
			pp[0].World.vz=pp[1].World.vz;

			pp[2].SYSX=pp[3].SYSX;
			pp[2].Z=pp[3].Z;
			pp[2].P=pp[3].P;
			pp[2].Flag=pp[3].Flag;
			pp[2].World.vx=pp[3].World.vx;
			pp[2].World.vy=pp[3].World.vy;
			pp[2].World.vz=pp[3].World.vz;

			pp[1].World.vx+=fdx;
			pp[1].World.vz+=fdz;

			pp[3].World.vx+=fdx;
			pp[3].World.vz+=fdz;
			dy1=dy2;

			pp[3].World.vy=	(PAP_2HI((pp[3].World.vx+POLY_cam_x)>>8,(pp[3].World.vz+POLY_cam_z)>>8).Alt<<3)-POLY_cam_y;
			dy2=(sy+bheight)-(pp[3].World.vy);
			if(dy2>255)
				dy2=255;


//			memcpy(&pp[0],&pp[2],2*sizeof(PSX_POLY_Point));
		}

//		facet_rand();
//		sy+=bheight;
//		hf+=1;
//		style_index++;

	}

}
#if	0
SLONG	texture_psx_quad(POLY_FT4 *prim,SLONG texture_style,SLONG pos,SLONG count,SLONG size)
{
	SLONG	tx,ty;
	SLONG	page;
	SLONG	texture_piece;
	SLONG   rand,flip=0;

//	size=31;
	rand = facet_rand() & 0x3;
	if(pos==0)
		texture_piece=TEXTURE_PIECE_RIGHT;
	else
	if(pos==count-2)
		texture_piece=TEXTURE_PIECE_LEFT;
	else
	{
		static const UBYTE choice[4] =
		{
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE1,
			TEXTURE_PIECE_MIDDLE2
		};

		texture_piece = choice[rand];
	}

	if(texture_style<0)
	{
		SLONG	index;
		struct	DStorey *p_storey;

		p_storey=&dstoreys[-texture_style];

		index=p_storey->Index;
		ASSERT(p_storey->Count);
		flip=0;
		
		if(pos<p_storey->Count)
		{
			page=paint_mem[index+pos];
			if((page&0x7f)==0)
			{
				texture_style = p_storey->Style;
			}
			else
			{
				if(page&0x80)
				{
					flip=1;
					page&=0x7f;
				}
				page=psx_remap[page];
				flip^=(page>>14)&1;
				page&=0xfff;
				page--;
 				if(page<0)
					page=99;


				prim->clut=getPSXClut2(page);
				tx=getPSXU2(page);
				ty=getPSXV2(page);
				page=(page&0xfc0)>>6;
				goto	got_texture; //goto's are great!!
			}
		}
		else
		{
			texture_style = p_storey->Style;
		}
	}

	if(texture_style>=0)
	{
		UWORD	cpage;
		SLONG	clutx,cluty;

		if(texture_style==0)
			texture_style=1;

		page=psx_textures_xy[texture_style][texture_piece]&0x3fff;

		prim->clut=getPSXClut2(page);

		tx=getPSXU2(page);
		ty=getPSXV2(page);
		page=page>>6;

		flip=psx_textures_xy[texture_style][texture_piece]>>14;
	}

got_texture:;
	
	ASSERT(page<10);
	prim->tpage=psx_tpages[page];	   
//	p4->DrawFlags=textures_flags[texture_style][texture_piece];

	switch(flip) //textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			setUV4(prim,tx+size,ty,tx,ty,tx+size,ty+size,tx,ty+size);

			break;
		case	1: //flip x
			setUV4(prim,tx,ty,tx+size,ty,tx,ty+size,tx+size,ty+size);

			break;
		case	2: //flip y
			setUV4(prim,tx+size,ty+size,tx,ty+size,tx+size,ty,tx,ty);

			break;
		case	3: //flip x+y
			setUV4(prim,tx,ty+size,tx+size,ty+size,tx,ty,tx+size,ty);

			break;
	}


	return(page);

}
#endif
SLONG	texture_psx_quad(POLY_FT4 *prim,SLONG texture_style,SLONG pos,SLONG count,SLONG t_width,SLONG t_height,SLONG t_x_offset)
{
	SLONG	tx,ty;
	SLONG	page;
	SLONG	texture_piece;
	SLONG   rand,flip=0;

//	size=31;
	rand = facet_rand() & 0x3;
	if(pos==0)
		texture_piece=TEXTURE_PIECE_RIGHT;
	else
	if(pos==count-2)
		texture_piece=TEXTURE_PIECE_LEFT;
	else
	{
		static const UBYTE choice[4] =
		{
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE1,
			TEXTURE_PIECE_MIDDLE2
		};

		texture_piece = choice[rand];
	}

	if(texture_style<0)
	{
		SLONG	index;
		struct	DStorey *p_storey;

		p_storey=&dstoreys[-texture_style];

		index=p_storey->Index;
		ASSERT(p_storey->Count);
		flip=0;
		
		if(pos<p_storey->Count)
		{
			page=paint_mem[index+pos];
			if((page&0x7f)==0)
			{
				texture_style = p_storey->Style;
			}
			else
			{
				if(page&0x80)
				{
					flip=1;
					page&=0x7f;
				}
				page=psx_remap[page];
				flip^=(page>>14)&1;
				page&=0xfff;
				page--;
 				if(page<0)
					page=99;


				prim->clut=getPSXClut2(page);
				tx=getPSXU2(page);
				ty=getPSXV2(page);
				page=(page&0xfc0)>>6;
				goto	got_texture; //goto's are great!!
			}
		}
		else
		{
			texture_style = p_storey->Style;
		}
	}

	if(texture_style>=0)
	{
//		UWORD	cpage;
		SLONG	clutx,cluty;

		if(texture_style==0)
			texture_style=1;

		page=psx_textures_xy[texture_style][texture_piece]&0x3fff;

		prim->clut=getPSXClut2(page);

		tx=getPSXU2(page);
		ty=getPSXV2(page);
		page=page>>6;

		flip=psx_textures_xy[texture_style][texture_piece]>>14;
	}


got_texture:;
	
	ASSERT(page<10);
	prim->tpage=psx_tpages[page];	   
//	p4->DrawFlags=textures_flags[texture_style][texture_piece];
	tx+=t_x_offset;

	switch(flip) //textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			setUV4(prim,tx+t_width,ty,tx,ty,tx+t_width,ty+t_height,tx,ty+t_height);

			break;
		case	1: //flip x
			setUV4(prim,tx,ty,tx+t_width,ty,tx,ty+t_height,tx+t_width,ty+t_height);

			break;
		case	2: //flip y
			setUV4(prim,tx+t_width,ty+t_height,tx,ty+t_height,tx+t_width,ty,tx,ty);

			break;
		case	3: //flip x+y
			setUV4(prim,tx,ty+t_height,tx+t_height,ty+t_height,tx,ty,tx+t_width,ty);

			break;
	}


	return(page);

}

SLONG	texture_psx_quad_gt4(POLY_GT4 *prim,SLONG texture_style,SLONG pos,SLONG count,SLONG t_width,SLONG t_height,SLONG t_x_offset)
{
	SLONG	tx,ty;
	SLONG	page;
	SLONG	texture_piece;
	SLONG   rand,flip=0;

//	size=31;
	rand = facet_rand() & 0x3;
	if(pos==0)
		texture_piece=TEXTURE_PIECE_RIGHT;
	else
	if(pos==count-2)
		texture_piece=TEXTURE_PIECE_LEFT;
	else
	{
		static const UBYTE choice[4] =
		{
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE,
			TEXTURE_PIECE_MIDDLE1,
			TEXTURE_PIECE_MIDDLE2
		};

		texture_piece = choice[rand];
	}

	if(texture_style<0)
	{
		SLONG	index;
		struct	DStorey *p_storey;

		p_storey=&dstoreys[-texture_style];

		index=p_storey->Index;
		ASSERT(p_storey->Count);
		flip=0;
		
		if(pos<p_storey->Count)
		{
			page=paint_mem[index+pos];
			if((page&0x7f)==0)
			{
				texture_style = p_storey->Style;
			}
			else
			{
				if(page&0x80)
				{
					flip=1;
					page&=0x7f;
				}
				page=psx_remap[page];
				flip^=(page>>14)&1;
				page&=0xfff;
				page--;
 				if(page<0)
					page=99;


				prim->clut=getPSXClut2(page);
				tx=getPSXU2(page);
				ty=getPSXV2(page);
				page=(page&0xfc0)>>6;
				goto	got_texture; //goto's are great!!
			}
		}
		else
		{
			texture_style = p_storey->Style;
		}
	}

	if(texture_style>=0)
	{
//		UWORD	cpage;
		SLONG	clutx,cluty;

		if(texture_style==0)
			texture_style=1;

		page=psx_textures_xy[texture_style][texture_piece]&0x3fff;

		prim->clut=getPSXClut2(page);

		tx=getPSXU2(page);
		ty=getPSXV2(page);
		page=page>>6;

		flip=psx_textures_xy[texture_style][texture_piece]>>14;
	}


got_texture:;
	
	ASSERT(page<10);
	prim->tpage=psx_tpages[page];	   
//	p4->DrawFlags=textures_flags[texture_style][texture_piece];
	tx+=t_x_offset;

	switch(flip) //textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			setUV4(prim,tx+t_width,ty,tx,ty,tx+t_width,ty+t_height,tx,ty+t_height);

			break;
		case	1: //flip x
			setUV4(prim,tx,ty,tx+t_width,ty,tx,ty+t_height,tx+t_width,ty+t_height);

			break;
		case	2: //flip y
			setUV4(prim,tx+t_width,ty+t_height,tx,ty+t_height,tx+t_width,ty,tx,ty);

			break;
		case	3: //flip x+y
			setUV4(prim,tx,ty+t_height,tx+t_height,ty+t_height,tx,ty,tx+t_width,ty);

			break;
	}


	return(page);

}

//
// chop a quad into smaller bits to reduce texture warp
//



//    0                1
//
//
//             4
//
//
//    2                 3

//
// tri's    014  134 324 204
//

#ifdef	UNUSED
UBYTE *build_split_face_4tri(PSX_POLY_Point *pp,SLONG page,POLY_GT4 *q,SLONG z)
{
	POLY_GT3	*f3;
	SLONG	r,g,b,u,v;
	PSX_POLY_Point p4;

	p4.World.vx=(pp[0].World.vx+pp[1].World.vx)>>1;
	p4.World.vy=(pp[0].World.vy+pp[2].World.vy)>>1;
	p4.World.vz=(pp[0].World.vz+pp[1].World.vz)>>1;


	gte_RotTransPers(&p4.World,&p4.SYSX,&p4.P,&p4.Flag,&p4.Z);

	u=(q->u0+q->u1)>>1;
	v=(q->v0+q->v2)>>1;

	r=(q->r0+q->r1+q->r2+q->r3)>>2;
	g=(q->g0+q->g1+q->g2+q->g3)>>2;
	b=(q->b0+q->b1+q->b2+q->b3)>>2;

	q++;
	f3=(POLY_GT3*)q;
	q--;

	setPolyGT3(f3);
	setXY3(f3,pp[0].Word.SX,pp[0].Word.SY,
			 pp[1].Word.SX,pp[1].Word.SY,
			 p4.Word.SX,p4.Word.SY);
	setRGB0(f3,q->r0,q->g0,q->b0);
	setRGB1(f3,q->r1,q->g1,q->b1);
	setRGB2(f3,r,g,b);
	setUV3(f3,q->u0,q->v0,q->u1,q->v1,u,v);
	f3->tpage=q->tpage;
	f3->clut=q->clut;
	DOPRIM(z,f3);
	f3++;

	setPolyGT3(f3);
	setXY3(f3,pp[1].Word.SX,pp[1].Word.SY,
			 pp[3].Word.SX,pp[3].Word.SY,
			 p4.Word.SX,p4.Word.SY);
	setRGB0(f3,q->r1,q->g1,q->b1);
	setRGB1(f3,q->r3,q->g3,q->b3);
	setRGB2(f3,r,g,b);
	setUV3(f3,q->u1,q->v1,q->u3,q->v3,u,v);
	f3->tpage=q->tpage;
	f3->clut=q->clut;
	DOPRIM(z,f3);
	f3++;

	setPolyGT3(f3);
	setXY3(f3,pp[3].Word.SX,pp[3].Word.SY,
			 pp[2].Word.SX,pp[2].Word.SY,
			 p4.Word.SX,p4.Word.SY);
	setRGB0(f3,q->r3,q->g3,q->b3);
	setRGB1(f3,q->r2,q->g2,q->b2);
	setRGB2(f3,r,g,b);
	setUV3(f3,q->u3,q->v3,q->u2,q->v2,u,v);
	f3->tpage=q->tpage;
	f3->clut=q->clut;
	DOPRIM(z,f3);
	f3++;

	setPolyGT3(f3);
	setXY3(f3,pp[2].Word.SX,pp[2].Word.SY,
			 pp[0].Word.SX,pp[0].Word.SY,
			 p4.Word.SX,p4.Word.SY);
	setRGB0(f3,q->r2,q->g2,q->b2);
	setRGB1(f3,q->r0,q->g0,q->b0);
	setRGB2(f3,r,g,b);
	setUV3(f3,q->u2,q->v2,q->u0,q->v0,u,v);
	f3->tpage=q->tpage;
	f3->clut=q->clut;
	DOPRIM(z,f3);
	f3++;
	return((UBYTE*)f3);
}
#endif
#if 0
void	do_overlay(POLY_GT4	*q,SLONG z)
{
	POLY_GT4	*overlay,*overlay2;
	ULONG	*from,*to,*to2,c0,temp;

	ALLOCPRIM(overlay,POLY_GT4);
	ALLOCPRIM(overlay2,POLY_GT4);

	to=(ULONG*)overlay;
	to2=(ULONG*)overlay2;

	from=(ULONG*)q;
	for(c0=0;c0<13;c0++)
	{
		// 
		temp=*from++;
		*to++=temp;
		*to2++=temp;
	}
	setSemiTrans(overlay,1);
	setSemiTrans(overlay2,1);

	overlay->x0-=2;
	overlay->x1+=2;
	overlay->x2-=2;
	overlay->x3+=2;

	overlay2->y0-=2;
	overlay2->y1-=2;
	overlay2->y2+=2;
	overlay2->y3+=2;

	overlay->tpage&=~((0x7)<<5);
	overlay2->tpage&=~((0x7)<<5);

//	overlay->r0=255;
//	overlay2->g1=255;
	


	DOPRIM(z,overlay);
	DOPRIM(z,overlay2);
}
#endif

//		p1
//
//  p4  p0  p3
//
//		p2

#define	QSET_TX_TY(p,x1,y1,x2,y2,x3,y3,x4,y4,c,pal)	((ULONG*)p)[3]=((x1)<<0)|((y1)<<8)|(c<<16);((ULONG*)p)[6]=((x2)<<0)|((y2)<<8)|(pal<<16);((ULONG*)p)[9]=((x3)<<0)|((y3)<<8);((ULONG*)p)[12]=((x4)<<0)|((y4)<<8);
#ifdef	FOUR
SLONG build_split_face_4quad_mid(PSX_POLY_Point *quad,SLONG page,POLY_GT4 *p,SLONG z)
{
	POLY_GT4	*pa;//,*overlay,*overlay2;
	SLONG	r1,g1,b1,u1,v1;
	PSX_POLY_Point pp[5];
//	ULONG	*from,*to,*to2,c0,temp;

	pp[1].World.vx=(quad[0].World.vx+quad[1].World.vx)>>1;
	pp[1].World.vy=(quad[0].World.vy+quad[1].World.vy)>>1;
	pp[1].World.vz=(quad[0].World.vz+quad[1].World.vz)>>1;

	pp[2].World.vx=(quad[2].World.vx+quad[3].World.vx)>>1;
	pp[2].World.vy=(quad[2].World.vy+quad[3].World.vy)>>1;
	pp[2].World.vz=(quad[2].World.vz+quad[3].World.vz)>>1;


	pp[4].World.vx=(quad[0].World.vx+quad[2].World.vx)>>1;
	pp[4].World.vy=(quad[0].World.vy+quad[2].World.vy)>>1;
	pp[4].World.vz=(quad[0].World.vz+quad[2].World.vz)>>1;

	pp[3].World.vx=(quad[1].World.vx+quad[3].World.vx)>>1;
	pp[3].World.vy=(quad[1].World.vy+quad[3].World.vy)>>1;
	pp[3].World.vz=(quad[1].World.vz+quad[3].World.vz)>>1;


	pp[0].World.vx=pp[1].World.vx;
	pp[0].World.vy=pp[3].World.vy;
	pp[0].World.vz=pp[1].World.vz;



	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
	gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
	gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
	gte_RotTransPers(&pp[4].World,&pp[4].SYSX,&pp[4].P,&pp[4].Flag,&pp[4].Z);

	r1=(p->r0+p->r1+p->r2+p->r3)>>2;
	g1=(p->g0+p->g1+p->g2+p->g3)>>2;
	b1=(p->b0+p->b1+p->b2+p->b3)>>2;

	u1=(p->u0+p->u1+p->u2+p->u3)>>2;
	v1=(p->v0+p->v1+p->v2+p->v3)>>2;

	r1=0;
	g1=0;
	b1=0;

//  0    n1     1
//	   X
//	n4	 n0		n3
//
//	2    n2     3
	if(((quad[0].Flag|pp[0].Flag|pp[1].Flag|pp[4].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);



		setXY4(pa,quad[0].Word.SX,quad[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,pp[4].Word.SX,pp[4].Word.SY,pp[0].Word.SX,pp[0].Word.SY);

		setRGB0(pa,p->r0,p->g0,p->b0);
		setRGB1(pa,(p->r0+p->r1)>>1,(p->g0+p->g1)>>1,(p->b0+p->b1)>>1);
		setRGB2(pa,(p->r0+p->r2)>>1,(p->g0+p->g2)>>1,(p->b0+p->b2)>>1);
		setRGB3(pa,r1,g1,b1);

		QSET_TX_TY(pa,p->u0,p->v0,
				(p->u0+p->u1)>>1,(p->v0+p->v1)>>1,
				(p->u0+p->u2)>>1,(p->v0+p->v2)>>1,
				u1,v1,p->clut,p->tpage);


//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}

//  0    n1     1
//			 X
//	n4	 n0		n3
//
//	2    n2     3

	if(((quad[0].Flag|pp[0].Flag|pp[1].Flag|pp[4].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);



		setXY4(pa,pp[1].Word.SX,pp[1].Word.SY,quad[1].Word.SX,quad[1].Word.SY,pp[0].Word.SX,pp[0].Word.SY,pp[3].Word.SX,pp[3].Word.SY);

		setRGB0(pa,(p->r0+p->r1)>>1,(p->g0+p->g1)>>1,(p->b0+p->b1)>>1);
		setRGB1(pa,p->r1,p->g1,p->b1);
		setRGB2(pa,r1,g1,b1);
		setRGB3(pa,(p->r3+p->r1)>>1,(p->g3+p->g1)>>1,(p->b3+p->b1)>>1);

		QSET_TX_TY(pa,(p->u0+p->u1)>>1,(p->v0+p->v1)>>1,
				p->u1,p->v1,
				u1,v1,
				(p->u1+p->u3)>>1,(p->v1+p->v3)>>1,p->clut,p->tpage);


//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}


//  0    n1     1
//
//	n4	 n0		n3
//	  X
//	2    n2     3

	if(((quad[2].Flag|pp[0].Flag|pp[2].Flag|pp[4].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);



		setXY4(pa,pp[4].Word.SX,pp[4].Word.SY,pp[0].Word.SX,pp[0].Word.SY,quad[2].Word.SX,quad[2].Word.SY,pp[2].Word.SX,pp[2].Word.SY);

		setRGB0(pa,(p->r0+p->r2)>>1,(p->g0+p->g2)>>1,(p->b0+p->b2)>>1);
		setRGB1(pa,r1,g1,b1);
		setRGB2(pa,p->r2,p->g2,p->b2);
		setRGB3(pa,(p->r2+p->r3)>>1,(p->g2+p->g3)>>1,(p->b2+p->b3)>>1);

		QSET_TX_TY(pa,
				(p->u0+p->u2)>>1,(p->v0+p->v2)>>1,
				u1,v1,
				p->u2,p->v2,
				(p->u3+p->u2)>>1,(p->v3+p->v2)>>1,p->clut,p->tpage);


//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}
//  0    n1     1
//
//	n4	 n0		n3
//			 X
//	2    n2     3

	if(((quad[3].Flag|pp[0].Flag|pp[2].Flag|pp[3].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);

		setXY4(pa,pp[0].Word.SX,pp[0].Word.SY,pp[3].Word.SX,pp[3].Word.SY,pp[2].Word.SX,pp[2].Word.SY,quad[3].Word.SX,quad[3].Word.SY);

		setRGB0(pa,r1,g1,b1);
		setRGB1(pa,(p->r3+p->r1)>>1,(p->g3+p->g1)>>1,(p->b3+p->b1)>>1);
		setRGB2(pa,(p->r3+p->r2)>>1,(p->g3+p->g2)>>1,(p->b3+p->b2)>>1);
		setRGB3(pa,p->r3,p->g3,p->b3);
//		setRGB3(pa,255,0,0);

		QSET_TX_TY(pa,u1,v1,
				(p->u3+p->u1)>>1,(p->v3+p->v1)>>1,
				(p->u3+p->u2)>>1,(p->v3+p->v2)>>1,
				p->u3,p->v3,p->clut,p->tpage);

//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}
}
#endif

//0    1

//p4  p5

//2    3
void build_split_face_2quad(PSX_POLY_Point *pp,SLONG page,POLY_GT4 *q,SLONG z)
{
	POLY_GT4	*f4,*overlay,*overlay2;
	SLONG	r0,g0,b0,r1,g1,b1,u,v1,v2;
	PSX_POLY_Point p4,p5;
	ULONG	*from,*to,*to2,c0,temp;

	p4.World.vx=(pp[0].World.vx+pp[2].World.vx)>>1;
	p4.World.vy=(pp[0].World.vy+pp[2].World.vy)>>1;
	p4.World.vz=(pp[0].World.vz+pp[2].World.vz)>>1;

	p5.World.vx=(pp[1].World.vx+pp[3].World.vx)>>1;
	p5.World.vy=(pp[1].World.vy+pp[3].World.vy)>>1;
	p5.World.vz=(pp[1].World.vz+pp[3].World.vz)>>1;

	gte_RotTransPers(&p4.World,&p4.SYSX,&p4.P,&p4.Flag,&p4.Z);
	if((p4.Flag&(3<<17)))
	{
		fuck_z(&p4);
	}
	if(p4.Word.SY>=350)
		p4.Word.SY=350;
	if(p4.Word.SX>=390)
		p4.Word.SX=390;

	gte_RotTransPers(&p5.World,&p5.SYSX,&p5.P,&p5.Flag,&p5.Z);
	if((p5.Flag&(3<<17)))
	{
		fuck_z(&p5);
	}
	if(p5.Word.SY>=350)
		p5.Word.SY=350;
	if(p5.Word.SX>=390)
		p5.Word.SX=390;

	v1=((q->v0+q->v2)>>1)+1;
	v2=((q->v1+q->v3)>>1)+1;
//	v=((q->v0+q->v2)>>1)+1;

	r0=(q->r0+q->r2)>>1;
	g0=(q->g0+q->g2)>>1;
	b0=(q->b0+q->b2)>>1;

	r1=(q->r1+q->r3)>>1;
	g1=(q->g1+q->g3)>>1;
	b1=(q->b1+q->b3)>>1;

	ALLOCPRIM(f4,POLY_GT4);

	setPolyGT4(f4);
	setXY4(q,pp[0].Word.SX,pp[0].Word.SY,
			 pp[1].Word.SX,pp[1].Word.SY,
			 p4.Word.SX,p4.Word.SY,
			 p5.Word.SX,p5.Word.SY);
	setXY4(f4,p4.Word.SX,p4.Word.SY,
			  p5.Word.SX,p5.Word.SY,
			  pp[2].Word.SX,pp[2].Word.SY,
			  pp[3].Word.SX,pp[3].Word.SY);

	setRGB2(f4,q->r2,q->g2,q->b2);
	setRGB3(f4,q->r3,q->g3,q->b3);

	setRGB0(f4,r0,g0,b0);
	setRGB1(f4,r1,g1,b1);
	setRGB2(q,r0,g0,b0);
	setRGB3(q,r1,g1,b1);

//	setRGB3(q,255,255,255); //remove me
//	setRGB3(f4,255,255,255);

	setUV4(f4,q->u0,v1,q->u1,v2,q->u2,q->v2,q->u3,q->v3);
	q->v2=v1;
	q->v3=v2;
	f4->tpage=q->tpage;
	f4->clut=q->clut;

//	do_overlay(q,z); //a couple of semi_trans poly's over the top


	DOPRIM(z,q);
	DOPRIM(z,f4);
}

//  0    1
//
//	p0--p1
//
//	p2--p3    // 6 points
//
//	p4--p5
//
//  2    3

SLONG build_split_face_4quad(PSX_POLY_Point *pp,SLONG page,POLY_GT4 *q,SLONG z)
{
	POLY_GT4	*f4;
//	SLONG	dr1,dg1,db1,dr2,dg2,db2;
	PSX_POLY_Point p[6];
	ULONG	c0;
	SLONG	dy1,dy2;//,dv1,dv2,du1,du2;

	//
	// stack space overflow bodger
	//
#define	dr1	(p[0].World.vx)
#define	dg1	(p[0].World.vy)
#define	db1	(p[0].World.vz)

#define	dr2	(p[1].World.vx)
#define	dg2	(p[1].World.vy)
#define	db2	(p[1].World.vz)

#define	du1	(p[2].World.vx)
#define	dv1	(p[2].World.vy)

#define	du2	(p[3].World.vx)
#define	dv2	(p[3].World.vy)

	dy1=(-pp[0].World.vy+pp[2].World.vy)>>2;
	dy2=(-pp[1].World.vy+pp[3].World.vy)>>2;

	p[0].World=pp[0].World;
	p[0].World.vy+=dy1;

	p[2].World=pp[0].World;
	p[2].World.vy+=dy1<<1;

	p[4].World=pp[2].World;
	p[4].World.vy-=dy1;


	p[1].World=pp[1].World;
	p[1].World.vy+=dy2;

	p[3].World=pp[1].World;
	p[3].World.vy+=dy2<<1;

	p[5].World=pp[3].World;
	p[5].World.vy-=dy2;

	gte_RotTransPers3(&p[0].World,&p[1].World,&p[2].World,&p[0].SYSX,&p[1].SYSX,&p[2].SYSX,&p[0].P,&p[0].Flag,&p[0].Z);
	gte_RotTransPers3(&p[3].World,&p[4].World,&p[5].World,&p[3].SYSX,&p[4].SYSX,&p[5].SYSX,&p[3].P,&p[3].Flag,&p[3].Z);
	if((p[0].Flag|p[3].Flag)&(1<<31))
		return(0);


	du1=((-q->u0+q->u2+2)>>2);
	dv1=((-q->v0+q->v2+2)>>2);

	du2=((-q->u1+q->u3+2)>>2);
	dv2=((-q->v1+q->v3+2)>>2);



	dr1=(q->r2-q->r0)>>2;
	dg1=(q->g2-q->g0)>>2;
	db1=(q->b2-q->b0)>>2;

	dr2=(q->r3-q->r1)>>2;
	dg2=(q->g3-q->g1)>>2;
	db2=(q->b3-q->b1)>>2;

//	dr1=(-q->r0+q->r2)>>2;
//	dg1=(-q->g0+q->g2)>>2;
//	db1=(-q->b0+q->b2)>>2;

//	dr2=(-q->r1+q->r3)>>2;
//	dg2=(-q->g1+q->g3)>>2;
//	db2=(-q->b1+q->b3)>>2;


	for(c0=0;c0<4;c0++)
	{
		SLONG	index;
		ALLOCPRIM(f4,POLY_GT4);
		setPolyGT4(f4);
		switch(c0)
		{
			case	0:
				setXY4(f4,pp[0].Word.SX,pp[0].Word.SY,
						 pp[1].Word.SX,pp[1].Word.SY,
						 p[0].Word.SX,p[0].Word.SY,
						 p[1].Word.SX,p[1].Word.SY);
				setUV4(f4,q->u0,q->v0
						,q->u1,q->v1
						,q->u0+du1,q->v0+dv1
						,q->u1+du1,q->v1+dv1);

				setRGB0(f4,q->r0,q->g0,q->b0);
				setRGB1(f4,q->r1,q->g1,q->b1);
				setRGB2(f4,q->r0+dr1,q->g0+dg1,q->b0+db1);
				setRGB3(f4,q->r1+dr2,q->g1+dg2,q->b1+db2);

				break;
			case	3:
				setXY4(f4,p[4].Word.SX,p[4].Word.SY,
						 p[5].Word.SX,p[5].Word.SY,
						 pp[2].Word.SX,pp[2].Word.SY,
						 pp[3].Word.SX,pp[3].Word.SY);
				setUV4(f4,q->u2-du1,q->v2-dv1
						,q->u3-du2,q->v3-dv2
						,q->u2,q->v2
						,q->u3,q->v3);

				setRGB0(f4,q->r2-dr1,q->g2-dg1,q->b2-db1);
				setRGB1(f4,q->r3-dr2,q->g3-dg2,q->b3-db2);
				setRGB2(f4,q->r2,q->g2,q->b2);
				setRGB3(f4,q->r3,q->g3,q->b3);
				break;
			case	1:
				setXY4(f4,p[0].Word.SX,p[0].Word.SY,p[1].Word.SX,p[1].Word.SY,
						  p[2].Word.SX,p[2].Word.SY,p[3].Word.SX,p[3].Word.SY);
				setUV4(f4,q->u0+du1,q->v0+dv1,q->u1+du2,q->v1+dv2,
						  q->u0+(du1<<1),q->v0+(dv1<<1),q->u1+(du2<<1),q->v1+(dv2<<1));
				setRGB0(f4,q->r0+dr1,q->g0+dg1,q->b0+db1);
				setRGB1(f4,q->r1+dr2,q->g1+dg2,q->b1+db2);
				setRGB2(f4,q->r0+(dr1<<1),q->g0+(dg1<<1),q->b0+(db1<<1));
				setRGB3(f4,q->r1+(dr2<<1),q->g1+(dg2<<1),q->b1+(db2<<1));
				break;

			case	2:
				setXY4(f4,p[2].Word.SX,p[2].Word.SY,p[3].Word.SX,p[3].Word.SY,
						  p[4].Word.SX,p[4].Word.SY,p[5].Word.SX,p[5].Word.SY);
				setUV4(f4,q->u0+du1+du1,q->v0+dv1+dv1,q->u1+du2+du2,q->v1+dv2+dv2,
						  q->u2-du1,q->v2-dv1,q->u3-du2,q->v3-dv2);
				setRGB0(f4,q->r0+dr1+dr1,q->g0+dg1+dg1,q->b0+db1+db1);
				setRGB1(f4,q->r1+dr2+dr2,q->g1+dg2+dg2,q->b1+db2+db2);
				setRGB2(f4,q->r2-dr1,q->g2-dg1,q->b3-db1);
				setRGB3(f4,q->r3-dr2,q->g3-dg2,q->b2-db2);
				break;
/*
			default:
				index=(c0-1)<<1;
				setXY4(f4,p[index].Word.SX,p[index].Word.SY,
						 p[index+1].Word.SX,p[index+1].Word.SY,
						 p[index+2].Word.SX,p[index+2].Word.SY,
						 p[index+3].Word.SX,p[index+3].Word.SY);

				setUV4(f4,q->u0+du1*(c0),q->v0+dv1*(c0)
						,q->u1+du2*(c0),q->v1+dv2*(c0)
						,q->u0+du1*(c0+1),q->v0+dv1*(c0+1)
						,q->u1+du2*(c0+1),q->v1+dv2*(c0+1));

				setRGB0(f4,q->r0+dr1*(c0),q->g0+dg1*(c0),q->b0+db1*(c0));
				setRGB1(f4,q->r1+dr2*(c0),q->g1+dg2*(c0),q->b1+db2*(c0));
				setRGB2(f4,q->r0+dr1*(c0+1),q->g0+dg1*(c0+1),q->b0+db1*(c0));
				setRGB3(f4,q->r1+dr2*(c0+1),q->g1+dg2*(c0+1),q->b1+db2*(c0));

				break;
*/

		}
		f4->tpage=q->tpage;
		f4->clut=q->clut;
		DOPRIM(z,f4);


	}
	return(1);
}


#ifndef	PSX
void FACET_draw_quick(SLONG facet)
{
	PSX_POLY_Point	*pp=perm_pp_array;
	SLONG	fx1,fx2,fz1,fz2,fy1,fy2,height;
	struct		  DFacet	*p_facet;

	p_facet=&dfacets[facet];
//	return;

	
	
	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_OUTSIDE_DOOR:
			return;
		case	STOREY_TYPE_FENCE:
		case	STOREY_TYPE_FENCE_FLAT:
		case	STOREY_TYPE_NORMAL:

			fx1=p_facet->x[0]<<8;
			fx2=p_facet->x[1]<<8;
			fz1=p_facet->z[0]<<8;
			fz2=p_facet->z[1]<<8;

			fy1=p_facet->Y[0];
			fy2=fy1+p_facet->Height*(p_facet->BlockHeight<<2);

			pp[0].World.vx=	fx1-POLY_cam_x;
			pp[0].World.vy=	fy2-POLY_cam_y;
			pp[0].World.vz=	fz1-POLY_cam_z;

			pp[1].World.vx=	fx2-POLY_cam_x;
			pp[1].World.vy=	fy2-POLY_cam_y;
			pp[1].World.vz=	fz2-POLY_cam_z;

			pp[2].World.vx=	fx1-POLY_cam_x;
			pp[2].World.vy=	fy1-POLY_cam_y;
			pp[2].World.vz=	fz1-POLY_cam_z;

			pp[3].World.vx=	fx2-POLY_cam_x;
			pp[3].World.vy=	fy1-POLY_cam_y;
			pp[3].World.vz=	fz2-POLY_cam_z;

			gte_RotTransPers4(&pp[0].World,&pp[1].World,&pp[2].World,&pp[3].World,
							  &pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[3].SYSX,
							  &pp[0].P,&pp[0].Flag,&pp[0].Z);

//			if(pp[0].Flag&(1<<31)==0)
			if ((MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>0)||
				(MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)||
				(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>0)||
				(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256))
			{
				//
				// Draw two overlapping lines.
				//
				POLY_F4	*p;


				ALLOCPRIM(p,POLY_F4);

				setPolyF4(p);
				setXY4(p,pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,
						 pp[2].Word.SX,pp[2].Word.SY,pp[3].Word.SX,pp[3].Word.SY);

				setRGB0(p,0,0,0);

				DOPRIM(1,p);
			}
			break;
	}


}
#endif

void	fuck_z(PSX_POLY_Point *pp)
{

	SLONG	dx,dy,dz;
	SLONG	t;
	VECTOR	out1;
	ULONG	flag;

	extern	SLONG	POLY_cam_yaw;
extern	SLONG AENG_cam_yaw;
/*
	if (PadKeyIsPressed(&PAD_Input1,PAD_RL))
		return;
*/

	pp->Flag|=(1<<17);
	gte_RotTrans(&pp->World,&out1,&flag);
	if(out1.vz<211 && out1.vz>0)
	{
		t=214-out1.vz;

		dx=(PSX_view_matrix.m[2][0]*t)>>12;
		dy=(PSX_view_matrix.m[2][1]*t)>>12;
		dz=(PSX_view_matrix.m[2][2]*t)>>12;

//		if(dx<296 && dz<296)
		{

			pp->World.vx+=dx;
			pp->World.vy+=dy;
			pp->World.vz+=dz;
/*
			RotTrans(&pp->World,&out1,&flag);
			if(out1.vz<210)
			{
				ASSERT(0);
				if(t>1000000)
					if(abs(dx)>1000)
					if(abs(dz)>1000)
						ASSERT(0);
			}
*/


			gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

			pp->World.vx-=dx;
			pp->World.vy-=dy;
			pp->World.vz-=dz;
		}
	}
}

#ifdef	OLD_FACET_CLIP
{
	{
		VECTOR	out1,out2;
		ULONG	flag;

		gte_RotTrans(&pp[0].World,&out1,&flag);

		pp[1].World.vx=	sx+dx*count;
		pp[1].World.vy=	y2+height;
		pp[1].World.vz=	sz+dz*count;
		gte_RotTrans(&pp[1].World,&out2,&flag);

		if(out1.vz<210)
		{

			
			//
			// starts off behind camera
			//
			if(out2.vz<210)
			{
				//
				// ends behind camera so dont draw, facets above may go infront of camera though
				//
				return(0);
			}
			else
			if(out2.vz>210 && out1.vz<-200)  // first point should be at least a block away to bother with this
			{
				//
				// starts off behind and goes infront of camera
				//
				SLONG	step_along;

				SLONG	vx,vz,wx,wz;

				SLONG	cosangle;


//				step_along=-out1.vz>>8;

				//(fixed point 12)
				wx=PSX_view_matrix.m[2][0];
				wz=PSX_view_matrix.m[2][2];

				vx=dx; // -256,0 or 256
				vz=dz; // -256,0 or 256
				vx<<=4;
				vz<<=4;

				//
				// both vectors are now normalised, cross product to get cos of angle between them , the simple Pythag for length of hypot
				//
				if(vx)
					cosangle=(vx*wx)>>12;
				else
					cosangle=(vz*wz)>>12;

				step_along=(-out1.vz<<12)/cosangle;  // h=cos theta/adjacent
				step_along>>=8;
				step_along=abs(step_along);

//				if(step_along>0)
				{
					px+=(dx>>8)*step_along;
					pz+=(dz>>8)*step_along;

					sx+=dx*step_along;
					sz+=dz*step_along;
					pos=step_along;
					pp[0].World.vx=	sx;
					pp[0].World.vz=	sz;

					col+=step_along;

					while(step_along--)
					{
						facet_rand();
					}

				}
			}

		}
		else
		{
			//
			// starts infront of camera
			//

			if(out1.vz>256*18)
			{
				// starts far far in the distance

				if(out2.vz>256*18)
				{
					//
					// both ends are far away dont bother with the rest of this facet
					//
					return(1);
				}

				//
				// ends nearer the camera
				//
				SLONG	step_along;

				SLONG	vx,vz,wx,wz;

				SLONG	cosangle;


//				step_along=-out1.vz>>8;

				//(fixed point 12)
				wx=PSX_view_matrix.m[2][0];
				wz=PSX_view_matrix.m[2][2];

				vx=dx; // -256,0 or 256
				vz=dz; // -256,0 or 256
				vx<<=4;
				vz<<=4;

				//
				// both vectors are now normalised, cross product to get cos of angle between them , the simple Pythag for length of hypot
				//
				if(vx)
					cosangle=(vx*wx)>>12;
				else
					cosangle=(vz*wz)>>12;

				step_along=((out1.vz-256*18)<<12)/cosangle;  // h=cos theta/adjacent
				step_along>>=8;

				step_along=abs(step_along);
				if(step_along>0)
				{
					px+=(dx>>8)*step_along;
					pz+=(dz>>8)*step_along;

					sx+=dx*step_along;
					sz+=dz*step_along;
					pos=step_along;
					pp[0].World.vx=	sx;
					pp[0].World.vz=	sz;

					col+=step_along;

					while(step_along--)
					{
						facet_rand();
					}

				}




			}
			if(out2.vz<210)
			{
				clip_end=1;
				//
				// ends behind camera
				//
			}
			else
			{
				//
				// starts and ends infront of camera
				//
			}
		}
	}
}
#endif

SLONG	draw_facet_strip(SLONG sx,SLONG sy,SLONG sz,SLONG dx,SLONG dz,SLONG style,SLONG sort_z,SLONG count,SLONG height,SLONG wrap,NIGHT_Colour *col,SLONG sort_offset,SLONG *drawn,struct DFacet *p_facet,SLONG black,SLONG lower)
{
	PSX_POLY_Point	*pp=perm_pp_array;
//	SLONG	p;
	SLONG	flag,y,y2,px,pz;
//	SLONG	z;
	SLONG	pos=0;
//	SLONG	c0;
	SLONG	b0,lum;
	SLONG	two_pass=0,nosplit=0;
	SLONG	quick_exit=POLY_CLIP_TOP|POLY_CLIP_LEFT|POLY_CLIP_RIGHT;//|POLY_CLIP_BOTTOM;
	SLONG	use_exit=0;
	ULONG	flaga,flagb;
	SLONG	clip_end=0;

	SLONG	tex_height=63;
	SLONG	fuckeda=0,fuckedb=0;


//	if(lower)
//		lower=50;

	if(col==0)
		black=1;

	if(height<256&&sort_z==99)
	{
		if(height>=192)
		{
			tex_height=48;//(75*64)/100;
		}
		else
		if(height>=128)
		{
			tex_height=32;
		}
		else
		if(height>=64)
		{
			tex_height=16;
		}
		else
			ASSERT(0);
	}


#ifndef FS_ISO9660
//	if(PadKeyIsPressed(&PAD_Input1,PAD_FLT))
//		two_pass=1;
#endif
//	if(PadKeyIsPressed(&PAD_Input1,PAD_FLB))
//		nosplit=1;

	if (sort_z==0)
	{
		y=grid_height_at_world(sx+POLY_cam_x,sz+POLY_cam_z)-POLY_cam_y;
		y2=grid_height_at_world(sx+dx+POLY_cam_x,sz+dz+POLY_cam_z)-POLY_cam_y;
	}
	else
	{
		y=y2=sy;
	} 

	if ((y+POLY_cam_y)<256)
	{
		wrap=0;
	} else
		wrap=1;

	px=(sx+POLY_cam_x)>>8;
	pz=(sz+POLY_cam_z)>>8;


//	if (!PadKeyIsPressed(&PAD_Input1,PAD_RU))
	{
		SLONG	clip=0;

		if(dx)
		{
			if(pz<NGAMUT_zmin || pz>NGAMUT_zmax)
				return(1); //dont draw any above this

			if(dx>0)
			{
				//
				// left to right
				//
				clip=-((px) -NGAMUT_point_gamut[pz].xmin);
				clip_end=p_facet->x[1]-NGAMUT_point_gamut[pz].xmax;
			}
			else
			{
				// right to left
				clip=(px) -NGAMUT_point_gamut[pz].xmax;
				clip_end=-(p_facet->x[1]-NGAMUT_point_gamut[pz].xmin); 
			}
		}
		else
		{
			if(px<NGAMUT_xmin || px>NGAMUT_xmax)
				return(1); //dont draw any above this

			// north to south

			if(dz>0)
			{
				clip=-((pz) -NGAMUT_gamut2[px].zmin);
				clip_end=(p_facet->z[1]-NGAMUT_gamut2[px].zmax); 

			}
			else
			{
				clip=(pz) -NGAMUT_gamut2[px].zmax;
				clip_end=-(p_facet->z[1]-NGAMUT_gamut2[px].zmin); 

			}
		}
/*
		{
			CBYTE	str[20];
			sprintf(str,"c %d e %d",clip,clip_end);
			FONT2D_DrawString(str,20,112,0xffffff,256);
		}
*/


		if(clip>1)
		{
			clip-=1;
			//
			// start to left of gamut so advance
			//
			if(dx)
			{
				sx+=dx*clip;
				px+=(dx>>8)*clip;
			}
			else
			{
				sz+=dz*clip;
				pz+=(dz>>8)*clip;
			}
			pos=clip;
			if(col)
				col+=clip;
			while(clip--)
			{
				facet_rand();
			}
		}
		if(clip_end>1)
			clip_end-=1;
		else
			clip_end=0;

		if (sort_z==0)
		{
			y=grid_height_at_world(sx+POLY_cam_x,sz+POLY_cam_z)-POLY_cam_y;
			y2=grid_height_at_world(sx+dx+POLY_cam_x,sz+dz+POLY_cam_z)-POLY_cam_y;
		}
	}



	//  ...................1   0
	//
	//  ...................3   2

	pp[0].World.vx=	sx;
	pp[0].World.vy=	y+height;
	pp[0].World.vz=	sz;

	//
	// OLD_FACET_CLIP was here
	//

	pp[1].World.vx=	sx+dx;
	pp[1].World.vy=	y2+height;
	pp[1].World.vz=	sz+dz;

	pp[2].World.vx=	sx;
	pp[2].World.vy=	y;
	pp[2].World.vz=	sz;

	pp[3].World.vx=	sx+dx;
	pp[3].World.vy=	y2;
	pp[3].World.vz=	sz+dz;


	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	if((pp[0].Flag&(3<<17)))// ||pp[0].Z<800)
	{
		fuckeda|=1;
		fuck_z(&pp[0]);
	}
	gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
	if((pp[2].Flag&(3<<17)))// ||pp[2].Z<800)
	{
		fuckeda|=2;
		fuck_z(&pp[2]);
	}
	pp[2].Word.SY+=lower;

	{
		SLONG	tflag,c0;
		flag=0xffffffff;
		for(c0=0;c0<3;c0+=2)
		{							   
			tflag=0;
			if(pp[c0].Word.SX<0)
				tflag|=POLY_CLIP_LEFT;
			if(pp[c0].Word.SX>=320)
			{
				tflag|=POLY_CLIP_RIGHT;
				if(pp[c0].Word.SX>=550)
					pp[c0].Word.SX=550;
			}
			if(pp[c0].Word.SY<0)
			{
				tflag|=POLY_CLIP_TOP;
			}
			if(pp[c0].Word.SY>=SCREEN_HEIGHT)
			{
				if(pp[c0].Word.SY>=350)
					pp[c0].Word.SY=350;
				tflag|=POLY_CLIP_BOTTOM;
			}

			flag&=tflag;
		}
	}

//	dx+=dx>>5; //+=8
//	dz+=dz>>5; //+=8

	flaga=flag;

//	flaga=pp[0].Flag|pp[2].Flag;

	lum=(LUMI(px,pz)<<8);
	for(;pos<count-1-clip_end;pos++)
	{

		//half_clip=0;
		px+=(dx>>8);
		pz+=(dz>>8);
		lum=(lum>>8)+(LUMI(px,pz)<<8);

		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
		if((pp[1].Flag&(3<<17)))// ||pp[1].Z<800)
		{
			fuckedb|=1;
			fuck_z(&pp[1]);
		}
		gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
		if((pp[3].Flag&(3<<17)))// ||pp[3].Z<800)
		{
			fuckedb|=2;
			fuck_z(&pp[3]);
		}
		pp[3].Word.SY+=lower;

//		flagb=pp[1].Flag|pp[3].Flag;


		//
		// if getting further away and near point is fully faded out
		//

//
// z might not be valid!
//



//		flag=flaga|flagb;//pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag;
/*
					if(0)
					{
						SLONG	r=0,g=0,b=0;
						LINE_F2	*line;

						ALLOCPRIM(line,LINE_F2);

						setLineF2(line);

						setXY2(line,(POLY_cam_x+pp[0].World.vx)>>6,(POLY_cam_z+pp[0].World.vz)>>7,
							(POLY_cam_x+pp[1].World.vx)>>6,(POLY_cam_z+pp[1].World.vz)>>7);
//						if(fuckeda)
							r=255;
						if(fuckedb)
							b=255;
						setRGB0(line,r,g,b);
						DOPRIM(PANEL_OTZ,line);
					
					}
*/




		
//		if( (flag&(1<<31))==0)
		//
		// if all points valid && not both ends fucked (both ends fucked should get rid of those polys round the corner
		//
		if(((pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag)&(1<<31))==0 && ((fuckeda&fuckedb)!=3))
		{
			SLONG	r=0,g=0,b=0;
/*
			if(pp[1].Z>pp[0].Z && pp[0].P>=(128<<5) )
			{
				//
				// exit asap
				//
	//			printf(" %d faces avoided in dist\n",count-pos-1);
				goto	early_out;
			}

			//				    					    
			// if each point is further left, and prev point is off left then exit
			//
			if(!(fuckeda|fuckedb))
			{
				if(pp[0].Word.SX<0 && pp[1].Word.SX<pp[0].Word.SX)
				{
	//				printf(" %d faces avoided LEFT\n",count-pos-1);
					goto	early_out;
				}


				//
				// if each point is further right, and prev point is off right then exit
				//
				if(pp[0].Word.SX>=SCREEN_WIDTH && pp[1].Word.SX>pp[0].Word.SX)
				{
	//				printf(" %d faces avoided RIGHT\n",count-pos-1);
					goto	early_out;
				}
			}
*/





			{
				SLONG	tflag,c0;
				flag=0xffffffff;
				for(c0=1;c0<4;c0+=2)
				{							   
					tflag=0;
					if(pp[c0].Word.SX<0)
						tflag|=POLY_CLIP_LEFT;
					if(pp[c0].Word.SX>=320)
					{
						tflag|=POLY_CLIP_RIGHT;
						if(pp[c0].Word.SX>=550)
							pp[c0].Word.SX=550;
					}
					if(pp[c0].Word.SY<0)
					{
						tflag|=POLY_CLIP_TOP;
					}
					if(pp[c0].Word.SY>=SCREEN_HEIGHT)
					{
						if(pp[c0].Word.SY>=350)
							pp[c0].Word.SY=350;
						tflag|=POLY_CLIP_BOTTOM;
					}
/*
					else
						if(c0<2)
							quick_exit&=~POLY_CLIP_TOP;
*/

					flag&=tflag;
				}
				flagb=flag;
				flag&=flaga;

				//
				// If every face is off left or off right or all top points are off top then don't draw more strips above
				//
				quick_exit&=(flag);//|POLY_CLIP_TOP;

				b0=getPSXFade(pp[0].P);
				if((flag==0))//&&(b0>0))
				{
					POLY_GT4	*p;
					SLONG	z;
					SLONG	page;
					use_exit=1;





					(*drawn)++;
					z=pp[0].Z;
					if(z<pp[1].Z)
						z=pp[1].Z;
					if(z<pp[2].Z)
						z=pp[2].Z;		   
					if(z<pp[3].Z)
						z=pp[3].Z;



					z>>=0;
					z+=sort_offset;
//					z-=4; //fix fences sorting with floor on both sides
					z=get_z_sort(z);

					SWORD	b1,b2,b3;
					b1=getPSXFade(pp[1].P);
					b2=getPSXFade(pp[2].P);
					b3=getPSXFade(pp[3].P);

					ALLOCPRIM(p,POLY_GT4); //the_display.CurrentPrim;

					setPolyGT4(p);
					page=texture_psx_quad_gt4(p,style,pos,count,63,tex_height,0);
					
					if(black)
					{
						setRGB0(p,0,0,0);
						setRGB1(p,0,0,0);
						setRGB2(p,0,0,0);
						setRGB3(p,0,0,0);

					}
					else
					{

						if ((wrap==0)&&((y+height+POLY_cam_y)<256))
							setRGB0(p,MAKELUMI(((col+count)->red)*b0>>8,lum&0xff),
									  MAKELUMI((col+count)->green*b0>>8,lum&0xff),
									  MAKELUMI((col+count)->blue*b0>>8,lum&0xff));
						else
							setRGB0(p,((col+count)->red)*b0>>8,(col+count)->green*b0>>8,(col+count)->blue*b0>>8);

						if ((wrap==0)&&((y2+height+POLY_cam_y)<256))
							setRGB1(p,MAKELUMI((col+count+1)->red*b1>>8,lum>>8),
									  MAKELUMI((col+count+1)->green*b1>>8,lum>>8),
									  MAKELUMI((col+count+1)->blue*b1>>8,lum>>8));
						else
							setRGB1(p,(col+count+1)->red*b1>>8,(col+count+1)->green*b1>>8,(col+count+1)->blue*b1>>8);

						if ((wrap==0))
							setRGB2(p,MAKELUMI((col+0)->red*b2>>8,lum&0xff),
									  MAKELUMI((col+0)->green*b2>>8,lum&0xff),
									  MAKELUMI((col+0)->blue*b2>>8,lum&0xff));
						else
							setRGB2(p,(col+0)->red*b2>>8,(col+0)->green*b2>>8,(col+0)->blue*b2>>8);

						if ((wrap==0))
							setRGB3(p,MAKELUMI((col+1)->red*b3>>8,lum>>8),
									  MAKELUMI((col+1)->green*b3>>8,lum>>8),
									  MAKELUMI((col+1)->blue*b3>>8,lum>>8));
						else
							setRGB3(p,(col+1)->red*b3>>8,(col+1)->green*b3>>8,(col+1)->blue*b3>>8);
					}


					// Increment the map coord pointers.
					//
					//split quad into 4 triangles if perspective error is too great
					//
					debug_count[1]++;
//					if(half_clip==0 && nosplit==0 && abs(abs(pp[0].Word.SY-pp[2].Word.SY)-abs(pp[1].Word.SY-pp[3].Word.SY))>10)
//					if(height>128 && 
//					if(nosplit==0 && half_clip==0 && (z>700 || (abs(abs(pp[0].Word.SY-pp[2].Word.SY)-abs(pp[1].Word.SY-pp[3].Word.SY))>10)) )

/*
					if(half_clip|| (nosplit==0 && (z>700 || (abs(abs(pp[0].Word.SY-pp[2].Word.SY)-abs(pp[1].Word.SY-pp[3].Word.SY))>10)) ))
					{
						build_split_face_2quad(pp,page,p,z);
					}
					else

*/

//					if(nosplit==0 && z>700)
					if(fuckeda||fuckedb)
					{
						build_split_face_2quad(pp,page,p,z);
					}
					else
					if(z>650)
					{
						if(tex_height<=48 || z<750)
						{
							build_split_face_2quad(pp,page,p,z);
						}
						else
						{
							build_split_face_4quad(pp,page,p,z);
						}
					}						  
					else
					{

						setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
							pp[1].Word.SX,pp[1].Word.SY,
							pp[2].Word.SX,pp[2].Word.SY,
							pp[3].Word.SX,pp[3].Word.SY);


						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
					}

					//
					// dan   dandan den derrrrrrr (Dan Dan Dan Dare? What are you on Mike?)
					//


				}
				else
				{
/*
//					if(0)
					if(b0>0)
					{
						LINE_F2	*line;

						ALLOCPRIM(line,LINE_F2);
						setLineF2(line);
						setXY2(line,80+(pp[0].Word.SX>>2),60+(pp[0].Word.SY>>2),80+(pp[1].Word.SX>>2),60+(pp[1].Word.SY>>2));
						setRGB0(line,128,128,128);
						DOPRIM(PANEL_OTZ,line);
					
						ALLOCPRIM(line,LINE_F2);
						setLineF2(line);
						setXY2(line,80+(pp[1].Word.SX>>2),60+(pp[1].Word.SY>>2),80+(pp[3].Word.SX>>2),60+(pp[3].Word.SY>>2));
						if(fuckedb)
							setRGB0(line,128,0,0);
						else
							setRGB0(line,128,128,128);
						DOPRIM(PANEL_OTZ,line);

						ALLOCPRIM(line,LINE_F2);
						setLineF2(line);
						setXY2(line,80+(pp[3].Word.SX>>2),60+(pp[3].Word.SY>>2),80+(pp[2].Word.SX>>2),60+(pp[2].Word.SY>>2));
						setRGB0(line,128,128,128);
						DOPRIM(PANEL_OTZ,line);

						ALLOCPRIM(line,LINE_F2);
						setLineF2(line);
						setXY2(line,80+(pp[2].Word.SX>>2),60+(pp[2].Word.SY>>2),80+(pp[0].Word.SX>>2),60+(pp[0].Word.SY>>2));
						if(fuckeda)
							setRGB0(line,0,0,128);
						else
							setRGB0(line,128,128,128);
						DOPRIM(PANEL_OTZ,line);



					}
*/

//					facet_rand();
					goto not_drawn;
				}


			}


		}

		else
		{
not_drawn:;
			facet_rand();
			debug_count[4]++;

		}



		pp[0].SYSX=pp[1].SYSX;
		pp[0].Z=pp[1].Z;
		pp[0].P=pp[1].P;
		pp[0].Flag=pp[1].Flag;
		pp[0].World.vx=pp[1].World.vx;
		pp[0].World.vy=pp[1].World.vy; //just incase there's a split
		pp[0].World.vz=pp[1].World.vz;

		pp[2].SYSX=pp[3].SYSX;
		pp[2].Z=pp[3].Z;
		pp[2].P=pp[3].P;
		pp[2].Flag=pp[3].Flag;
		pp[2].World.vx=pp[3].World.vx;
		pp[2].World.vy=pp[3].World.vy;
		pp[2].World.vz=pp[3].World.vz;

		pp[1].World.vx+=dx;
		pp[1].World.vz+=dz;

		pp[3].World.vx+=dx;
		pp[3].World.vz+=dz;

		flaga=flagb;
		fuckeda=fuckedb;

		fuckedb=0;

		if (sort_z==0)
		{
			pp[3].World.vy=grid_height_at_world(pp[3].World.vx+POLY_cam_x,pp[3].World.vz+POLY_cam_z)-POLY_cam_y;
//			pp[3].World.vy=sy+(PAP_2HI((pp[3].World.vx+POLY_cam_x)>>8,(pp[3].World.vz+POLY_cam_z)>>8).Alt<<3);
			pp[1].World.vy=pp[3].World.vy+height;
		}

		if(col)
			col++;

	}
early_out:;
	for(;pos<count-1;pos++)
	{
		debug_count[0]++;
		facet_rand();
	}

//	return(quick_exit&POLY_CLIP_TOP);

	if(use_exit)
	{
		//
		// at least some faces have to transform to make the decision that the rest is unusable
		//
		return(quick_exit);
//		return(0);//quick_exit);
	}
	else
		return(0);

//	ASSERT(the_display.CurrentPrim < &the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM]);
}

#ifdef	NOT_USED
void	draw_inside_strip(SLONG sx,SLONG sy,SLONG sz,SLONG dx,SLONG dz,SLONG style,SLONG sort_z,SLONG count,SLONG height,NIGHT_Colour *col)
{
	PSX_POLY_Point	*pp=perm_pp_array;
//	SLONG	p;
	SLONG	flag;
//	SLONG	z;
	SLONG	pos;
//	SLONG	c0;
	UBYTE	*cp;
	SLONG	b0;
	SLONG	two_pass=0;

//	return;

#ifndef FS_ISO9660
//	if(PadKeyIsPressed(&PAD_Input1,PAD_FLT))
//		two_pass=1;
#endif


	cp=the_display.CurrentPrim;

	pp[0].World.vx=	sx;
	pp[0].World.vy=	sy+height;
	pp[0].World.vz=	sz;

	pp[1].World.vx=	sx+dx;
	pp[1].World.vy=	sy+height;
	pp[1].World.vz=	sz+dz;

	pp[2].World.vx=	sx;
	pp[2].World.vy=	sy;
	pp[2].World.vz=	sz;

	pp[3].World.vx=	sx+dx;
	pp[3].World.vy=	sy;
	pp[3].World.vz=	sz+dz;


	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
	for(pos=0;pos<count-1;pos++)
	{

		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
		gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

		flag=pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag;

//		pp[0].World.vx+=dx;
//		pp[0].World.vz+=dz;

		pp[1].World.vx+=dx;
		pp[1].World.vz+=dz;

//		pp[2].World.vx+=dx;
//		pp[2].World.vz+=dz;

		pp[3].World.vx+=dx;
		pp[3].World.vz+=dz;

		if( (flag&(1<<31))==0)
		{
			{
				SLONG	tflag,c0;
				flag=0xffffffff;
				for(c0=0;c0<4;c0++)
				{
					tflag=0;
					if(pp[c0].Word.SX<0)
						tflag|=POLY_CLIP_LEFT;
					if(pp[c0].Word.SX>=320)
						tflag|=POLY_CLIP_RIGHT;
					if(pp[c0].Word.SY<0)
						tflag|=POLY_CLIP_TOP;
					if(pp[c0].Word.SY>=SCREEN_HEIGHT)
						tflag|=POLY_CLIP_BOTTOM;
					flag&=tflag;
				}
				b0=getPSXFade(pp[0].P);
				if(flag==0&&b0)
				{
					POLY_FT4	*p;
					SLONG	z;
					SLONG	page;

//					if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
//						return;

					z=pp[0].Z;
					if(z<pp[1].Z)
						z=pp[1].Z;
					if(z<pp[2].Z)
						z=pp[2].Z;
					if(z<pp[3].Z)
						z=pp[3].Z;

					check_prim_ptr((void**)&cp);
					p=(POLY_FT4 *)cp; //the_display.CurrentPrim;

					setPolyFT4(p);
					page=texture_psx_quad(p,style,pos,count,32);

					z>>=0;
					z=get_z_sort(z);
#if 0
					if (b0!=128)
					{
//						AENG_add_fade(pp,z);
						setSemiTrans(p,1);
//						b0>>=1;
//						if (b0<64)
//							p->tpage&=~(3<<5);
					}
#endif

					setRGB0(p,b0,b0,b0);

					setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
						pp[1].Word.SX,pp[1].Word.SY,
						pp[2].Word.SX,pp[2].Word.SY,
						pp[3].Word.SX,pp[3].Word.SY);

					cp+=sizeof(POLY_FT4);
					addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);

					//
					// dan   dandan den derrrrrrr
					//


				}
				else
					goto not_drawn;


			}


		}
		else
		{
not_drawn:;
		facet_rand();

		}

		pp[0].SYSX=pp[1].SYSX;
		pp[0].Z=pp[1].Z;
		pp[0].P=pp[1].P;
		pp[0].Flag=pp[1].Flag;

		pp[2].SYSX=pp[3].SYSX;
		pp[2].Z=pp[3].Z;
		pp[2].P=pp[3].P;
		pp[2].Flag=pp[3].Flag;
		if(col)
			col++;

	}
	check_prim_ptr((void**)&cp);
	the_display.CurrentPrim=cp;
//	ASSERT(the_display.CurrentPrim < &the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM]);
}
#endif
void FENCE_draw_barbs(struct DFacet *p_facet)
{
	SLONG dx=(p_facet->x[1]-p_facet->x[0])<<8;
	SLONG dz=(p_facet->z[1]-p_facet->z[0])<<8;
	SLONG mag=SquareRoot0((dx*dx)+(dz*dz));
	SLONG sx=((dx<<5)/mag);
	SLONG sz=((dz<<5)/mag);
	SLONG cx=p_facet->x[0]<<8;
	SLONG cz=p_facet->z[0]<<8;
	SLONG seed=54321678;
	SLONG base=0;
	SLONG contour = 0;

//	return;

	while (base<mag) 
	{

		//...
		//sprite it as a test (do it proper later)
		//

		seed*=31415965;
		seed+=123456789;

		if (!(p_facet->FacetFlags & FACET_FLAG_ONBUILDING))
		{
			contour = PAP_calc_map_height_at(cx,cz);
		}
		else
		{
			contour = p_facet->Y[0];
		}

		SPRITE_draw(
			cx,
			(64*p_facet->Height)-64+((seed>>8)&0xf) + contour,
			cz,
			50,
			0x3f3f3f,
			0,
			POLY_PAGE_BARBWIRE,
			1);

			base+=32;
			cx+=sx;
			cz+=sz;
	}
}

void FENCE_draw_slope(SLONG x,SLONG y,SLONG z,SLONG dx,SLONG dz,SLONG count,SLONG style,NIGHT_Colour *colour,SLONG flag)
{
	PSX_POLY_Point *pp=perm_pp_array;
	POLY_FT4	*p;
	SLONG	zz;
	SLONG	page,c0;

	SLONG	nx,nz,sx,sy,sz;
	UBYTE	u,v;
	SWORD	b0;

	//		build_fence_poles(sx,sy,sz,fdx,fdz,count,&dx,&dz,style_index);

	nx=dz>>8;
	nz=-dx>>8;

	sx=x;
	sz=z+nz;
	sy=y;

	pp[0].World.vx=sx;

	if (flag & FACET_FLAG_ONBUILDING)
		pp[0].World.vy=sy;
	else
		pp[0].World.vy=grid_height_at_world(sx+POLY_cam_x,sz+POLY_cam_z)+192-POLY_cam_y;

	pp[0].World.vz=sz;
	pp[1].World.vx=sx+40*nx;
	pp[1].World.vy=pp[0].World.vy+40;
	pp[1].World.vz=sz+40*nz;
	memcpy(&pp[2],&pp[0],2*sizeof(PSX_POLY_Point));

	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);

	p=(POLY_FT4 *)the_display.CurrentPrim;

	c0=0;
	while(c0<count-1)
	{
		pp[2].World.vx+=dx;
		pp[2].World.vz+=dz;
		pp[3].World.vx+=dx;
		pp[3].World.vz+=dz;

		if (flag & FACET_FLAG_ONBUILDING)
			pp[2].World.vy=sy;
		else
			pp[2].World.vy=grid_height_at_world(pp[2].World.vx+POLY_cam_x,pp[2].World.vz+POLY_cam_z)+192-POLY_cam_y;

		pp[3].World.vy=pp[2].World.vy+40;

		gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
		gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

		//
		// create the quads and submit them for drawing
		//

				
		//
		// Texture the quad.
		// 


		if (!(pp[0].Flag&pp[1].Flag&pp[2].Flag&pp[3].Flag&1<<31))
			if ((MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>=0)&&
				(MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)&&
				(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>=0)&&
				(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256))
			{
				check_prim_ptr((void**)&p);
				setPolyFT4(p);

				page=texture_psx_quad(p,style,c0,count,63,20,0);
//				page=texture_psx_quad(p,dstyles[style],c0,count,31);
//				setUV4(p,u,v,u+31,v,u,v+31,u+31,v+31);

				b0=getPSXFade(pp[0].P);
				
				SATURATE(b0,0,128);

				if (b0)
				{
					setRGB0(p,b0,b0,b0);
					setXY4(p,pp[1].Word.SX,pp[1].Word.SY,pp[3].Word.SX,pp[3].Word.SY,
							 pp[0].Word.SX,pp[0].Word.SY,pp[2].Word.SX,pp[2].Word.SY);

					zz=pp[0].Z;
					if(zz<pp[1].Z)
						zz=pp[1].Z;

					if(zz<pp[2].Z)
						zz=pp[2].Z;

					if(zz<pp[3].Z)
						zz=pp[3].Z;

					zz>>=0;
					zz=get_z_sort(zz);

#if 0					
					if (b0!=128)
					{
	//					AENG_add_fade(pp,z);
						setSemiTrans(p,1);
	//						b0>>=1;
	//					if (b0<64)
	//						p->tpage&=~(3<<5);
					}
#endif

					DOPRIM(zz,p);
					p++;
				}
			}
			else
			{
				//
				// Push on the random number generator.
				//

				facet_rand();
			}

		memcpy(&pp[0],&pp[2],2*sizeof(PSX_POLY_Point));
		c0++;
	}
	check_prim_ptr((void**)&p);
	the_display.CurrentPrim=(UBYTE*)p;
}


//		1				0
//		x    0xxxx1     x   1xxx0
//		x				x
//		0				1

#ifdef	ONE_DAY
void	pre_process_facets(void)
{
	SLONG	c0;
	struct		  DFacet	*p_facet;
	SLONG	dx,dz;

	for(c0=1;c0<next_dfacet;c0++)
	{
//		SLONG	type;
		p_facet=&dfacets[c0];
/*
		dx=p_facet->x[1]-p_facet->x[0];
		dz=p_facet->z[1]-p_facet->z[0];

		if(dz<0)
			type=1;


		if(dx>0)
			type=2;

		if(dz>0)
			type=3;

		if(dx<0)
			type=4;


		p_facet->FacetFlags&=~(3<<1);
		p_facet->FacetFlags|=(type-1)<<1;
*/

		if ((p_facet->FacetType == STOREY_TYPE_FENCE ||
			p_facet->FacetType == STOREY_TYPE_FENCE_BRICK ||
			p_facet->FacetType == STOREY_TYPE_INSIDE ||
			p_facet->FacetType == STOREY_TYPE_INSIDE_DOOR ||
			p_facet->FacetType == STOREY_TYPE_LADDER ||
			p_facet->FacetType == STOREY_TYPE_FENCE_FLAT)
			&& !(p_facet->FacetFlags&FACET_FLAG_2SIDED))
		{
			p_facet->FacetFlags|=FACET_FLAG_DONT_CULL;

		}
		else
		{
			p_facet->FacetFlags&=~FACET_FLAG_DONT_CULL;
		}


	}
}
#endif


#if 1
extern char *GDisp_Bucket;
#else
extern char GDisp_Bucket[];
#endif

void FACET_draw(SLONG facet,UBYTE fade_alpha)
{
	struct		  DFacet	*p_facet;
	SWORD		  rows[2];
	SLONG		  c0,count;
	SLONG		  dx,dz;
	SLONG		  x,y,z,sx,sy,sz,fdx,fdz;
	SLONG		  height;
  	POLY_Point   *pp;
	SLONG         hf;
	POLY_Point   *quad[4];
	SLONG		  style_index;
	NIGHT_Colour *col;
	SLONG         max_height;
	SLONG		  inside_clip=0;
	SLONG		  block_height=256;
	SLONG		facet_backwards=0;
	SLONG		warehouse;
	SLONG	drawn=0;
	SLONG	black=0;

//	ASSERT(facet>0&&facet<next_dfacet);

//	if (the_display.CurrentPrim>&GDisp_Bucket[BUCKET_MEM-5120])
//		return(0);

	p_facet=&dfacets[facet];

	if (INDOORS_DBUILDING==p_facet->Building && INDOORS_INDEX)
		inside_clip=1;

//	if(facet!=2)// && facet!=2)
//		return;
	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_CABLE:
			cable_draw(p_facet);
			return;
			break;
	}


	if(dbuildings[p_facet->Building].Type == BUILDING_TYPE_WAREHOUSE|| dbuildings[p_facet->Building].Type == BUILDING_TYPE_CRATE_IN)
		warehouse=1;
	else
		warehouse=0;

#ifdef DOGPOO
	{
		LINE_F2 *p;
		ALLOCPRIM(p,LINE_F2)
		setLineF2(p);
		setSemiTrans(p,1);
		setXY2(p,p_facet->x[0]<<1,p_facet->z[0]<<1,p_facet->x[1]<<1,p_facet->z[1]<<1);
		setRGB0(p,64,64,64);
		DOPRIM(PANEL_OTZ,p);
	}
#endif
	//
	// Should we bother drawing this facet?
	//

	if ((p_facet->FacetType == STOREY_TYPE_FENCE ||
		p_facet->FacetType == STOREY_TYPE_FENCE_BRICK ||
		p_facet->FacetType == STOREY_TYPE_INSIDE ||
//		p_facet->FacetType == STOREY_TYPE_INSIDE_DOOR ||
		p_facet->FacetType == STOREY_TYPE_LADDER ||
		p_facet->FacetType == STOREY_TYPE_OUTSIDE_DOOR ||
		p_facet->FacetType == STOREY_TYPE_DOOR ||
		p_facet->FacetType == STOREY_TYPE_FENCE_FLAT)
		&& !(p_facet->FacetFlags&FACET_FLAG_2SIDED))
	{
		//
		// These facets are double-sided so they can't be backface culled.
		//
	}
	else
	{
		//
		// Backface cull the entire facet?
		//

		SLONG x1, z1;
		SLONG x2, z2;

		SLONG vec1x;
		SLONG vec1z;

		SLONG vec2x;
		SLONG vec2z;

		SLONG cprod;

		x1 = p_facet->x[0]<<8;
		z1 = p_facet->z[0]<<8;

		x2 = p_facet->x[1]<<8;
		z2 = p_facet->z[1]<<8;

		vec1x = x2 - x1;
		vec1z = z2 - z1;

		vec2x = POLY_cam_x - x1;
		vec2z = POLY_cam_z - z1;

		cprod = vec1x*vec2z - vec1z*vec2x;

		if (cprod >= 0)
		{
			//
			// We've got rid of a whole facet :o)
			// 
			if ((p_facet->FacetFlags&FACET_FLAG_2SIDED) || p_facet->FacetType==STOREY_TYPE_OINSIDE)
				facet_backwards=1;
			else
			{
				debug_count[5]++;
				p_facet->FacetFlags|=FACET_FLAG_INVISIBLE;
				return;
			}
		}
	}

	//
	// Transform the bounding box of the facet to quickly try and reject the
	// entire facet.
	//
/*
	if (abs(p_facet->x[1] - p_facet->x[0]) +
		abs(p_facet->z[1] - p_facet->z[0]) <= 2)
	{
		//
		// Too small the bother with the rejection test?
		// Nah! just do the rejection test anyway...
		//
	}
*/
	//
	// Draw the facet.
	// 

	dfacets_drawn_this_gameturn += 1;

	if (p_facet->FacetType == STOREY_TYPE_LADDER)
	{
		//
		// This is a ladder, and it is drawn with its
		// own special routine.
		//
//		if (inside_clip)
//			DRAW_ladder(p_facet,(inside_storeys[INDOORS_INDEX].StoreyY+256)-POLY_cam_y);
//		else
			DRAW_ladder(p_facet);

		return;
	}

//	POLY_buffer_upto = 0;

	style_index=p_facet->StyleIndex;

	//
	// Should this be passed an x,y,z to be relative to? Nah!
	//

	set_facet_seed((p_facet->x[0]<<8) * (p_facet->z[0]<<8) + (p_facet->Y[0]));

	//
	// If we are drawing the building we are in, dont draw
	// the building above the ceiling.
	//

	if (GAME_FLAGS & GF_INDOORS)
	{
		max_height = INDOORS_HEIGHT_CEILING;
	}
	else
	{
		max_height = INFINITY;
	}

	//
	// If there is no cached lighting for this facet, then we
	// must make some.
	//

	if (p_facet->Dfcache == 0)
	{
		// I've just seen how this work's and I'm scared 
		// looks like NIGHT_dfcache_create has to completely understand how to build any facet
		// like the fence struts I've just crafted...

		p_facet->Dfcache = NIGHT_dfcache_create(facet);
		if (p_facet->Dfcache == 0)
		{
			col=0;
		}
		else
		{
			col = NIGHT_dfcache[p_facet->Dfcache].colour;
		}

//		ASSERT(p_facet->Dfcache != NULL);
	}
	else
	{
		col = NIGHT_dfcache[p_facet->Dfcache].colour;
	}




	//
	// Get the shadow bits from the colour pointer.
	//
/*
	shadow_byte  = (UBYTE *) col;
	shadow_byte += NIGHT_dfcache[p_facet->Dfcache].sizeof_colour;
	shadow_byte -= NIGHT_dfcache[p_facet->Dfcache].shadow_bytes;
	shadow_shift = 0;
  */
	sx=(p_facet->x[0]<<8)-POLY_cam_x;
	sy=p_facet->Y[0]-POLY_cam_y;
	sz=(p_facet->z[0]<<8)-POLY_cam_z;

	height=p_facet->Height;

	dx=(p_facet->x[1]-p_facet->x[0])<<8;
	dz=(p_facet->z[1]-p_facet->z[0])<<8;

	if(dx&&dz)
	{
		LogText(" diagonal wall \n");
//		ASSERT(0);
//		return;
	}

	if(dx)
	{
		count=abs(dx)>>8;
		if(dx>0)
			dx=256;
		else
			dx=-256;
	}
	else
	{
		count=abs(dz)>>8;
		if(dz>0)
			dz=256;
		else
			dz=-256;
	}

	if (p_facet->Open)
	{
		SLONG temp=dx;
		SLONG open=p_facet->Open<<2;
		dx=(dx*COS(open)+dz*SIN(open))>>16;
		dz=(-temp*SIN(open)+dz*COS(open))>>16;
	}

	count++;

	fdx=dx;
	fdz=dz;

	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_CABLE:
			cable_draw(p_facet);
			break;
#ifdef INSIDES_EXIST
		case	STOREY_TYPE_INSIDE_DOOR:
			if(facet_backwards)
				style_index++;

			// Hacky cludge to get some form of door in right now.

//			draw_inside_strip(sx,sy,sz,dx,dz,dstyles[style_index],99,count,256,col,fade_alpha); //99 could be z_sort
//			DRAW_door(sx,sy,sz,256,0,fdz,block_height,count,0,dstyle[style_index]);
			break;
		case	STOREY_TYPE_INSIDE:
		case	STOREY_TYPE_OINSIDE:
/*
				draw_wall_thickness(p_facet,fade_alpha);
				if(facet_backwards)
						style_index++;

				hf=0;
   				while(height>0) //was >=
				{
					draw_inside_strip(sx,sy,sz,dx,dz,dstyles[style_index],99,count,256,col,fade_alpha); //99 could be z_sort
					sy+=256;
					if (sy > max_height)
					{
						break;
					}
					height-=4;
					hf+=1;
					style_index++;
					col+=count;
				}
				break;
*/
#endif
		case	STOREY_TYPE_DOOR:
			black=1;
//			break;
//		case	STOREY_TYPE_TRENCH:
//			LogText(" alt %d \n",p_facet->Y[0]);
		case	STOREY_TYPE_NORMAL:

				if(facet_backwards)
					style_index++;

				block_height=p_facet->BlockHeight<<4;

				if(inside_clip) //INDOORS_INDEX)
				{
					SLONG	top;
//					top=block_height;
					top=(block_height*height)>>2;
					top+=sy+POLY_cam_y;
					if( top>=(inside_storeys[INDOORS_INDEX].StoreyY+256))
					{
						//
						// clip the top of the building, but first check fade status
						//

						{
							height-=((top+4)-(inside_storeys[INDOORS_INDEX].StoreyY+256))/(p_facet->BlockHeight<<2);
						}
					}


				}

				if(p_facet->FHeight)
				{
					draw_facet_foundation(sx,sy,sz,fdx,fdz,count,p_facet->BlockHeight<<4,col,dstyles[style_index],0,0,&drawn,black);//style_index,shadow_byte,shadow_shift);
					sy+=p_facet->BlockHeight<<4;
					height-=4;
					style_index++;
					if(col)
						col+=count;
				}
				hf=0;
				{
					SLONG	style_index_offset,style_step=1,reverse;
					SLONG	sort_z;

  /*
					if(p_facet->FacetFlags&FACET_FLAG_INSIDE)
					{
						style_index-=2;
						reverse=1;
						style_step=2;
					}
					else
					if(p_facet->FacetFlags&FACET_FLAG_2TEXTURED)
					{
						style_index-=2;
						style_step=2;
					}
*/
					if(p_facet->FacetFlags&(FACET_FLAG_INSIDE))
					{
		//				reverse_textures=1;
						style_index--;
					}
					else
					if(p_facet->FacetFlags&(FACET_FLAG_2TEXTURED))
					{
						style_index--;
					}

					if(!(p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR) && (p_facet->FacetFlags&(FACET_FLAG_2TEXTURED|FACET_FLAG_2SIDED)))
					{
						style_index_offset=-1;
						style_step=2;
					}
					else
					{
						style_index_offset=0;
						style_step=1;

					}

					//if(warehouse)
					
extern	UBYTE	in_ware;

					if(in_ware)
					{
						sort_z=20;
					}
					else
					{
						sort_z=15;
					}

					while(height>0) //was >=
					{
						SLONG	quick_exit;
/*
						if (warehouse&&facet_backwards)
						{
							draw_inside_strip(sx,sy,sz,dx,dz,dstyles[style_index],99,count,p_facet->BlockHeight<<4,col); //99 could be z_sort
						}
						else
*/

						{
							SLONG	add;
							if(height<4)
								add=1;
							else
								add=0;
/*							
							if(in_ware&&height==4)
							{
								sort_z=0;
							}
*/
//							if(height==8)
								quick_exit=draw_facet_strip(sx,sy,sz,dx,dz,dstyles[style_index-style_index_offset],99,count,(p_facet->BlockHeight<<4),hf,col,sort_z,&drawn,p_facet,black,add); //99 could be z_sort
						}

						if(quick_exit)
						{
							if((FC_cam[0].pitch>>8)<-1024)
							{
								//
								// camera is looking up
								//
								if(quick_exit&POLY_CLIP_TOP)
								{
//									return;
								}

							}
							else
							{

		//						printf(" height %d * faces %d avoided\n",height>>2,count-1);
								debug_count[0]+=(height>>2)*(count-1);
								return;
							}
						}
						sy+=p_facet->BlockHeight<<4;
						height-=4;
						hf+=1;
						style_index+=style_step;//++;
						if(col)
							col+=count;
					}

					if(drawn==0)
					{
						//
						// don't bother with it next gameturn
						//
						p_facet->FacetFlags|=FACET_FLAG_INVISIBLE;
					}

				}

				break;

		case	STOREY_TYPE_FENCE_BRICK:
			// Insert code to do barb-wire on the top of fence

			// hardwire the height
			p_facet->Height=6;
			// bollocks to it all i'm doing my own so nnyeerrrr
			FENCE_draw_barbs(p_facet);
			p_facet->Height--;

		case	STOREY_TYPE_OUTSIDE_DOOR:
		case	STOREY_TYPE_FENCE:

			//
			// build the slope at the top of the fence
			//

			if (p_facet->FacetType==STOREY_TYPE_FENCE)
			{
				p_facet->Height=3;
				FENCE_draw_slope(sx,sy,sz,dx,dz,count,dstyles[style_index],col,p_facet->FacetFlags);
			}

			height=p_facet->Height;

		case	STOREY_TYPE_FENCE_FLAT:
			{
				SLONG	drawn=0;
				draw_facet_strip(sx,sy,sz,dx,dz,dstyles[style_index],(p_facet->FacetFlags & FACET_FLAG_ONBUILDING)?100:0,count,height<<6,0,col,0,&drawn,p_facet,0,0); //99 could be z_sort
				if(drawn==0)
				{
					//
					// don't bother with it next gameturn
					//
					p_facet->FacetFlags|=FACET_FLAG_INVISIBLE;
				}
			}
			break;
	}
}

/*
void	psx_add_quad(POLY_Point *quad[4],SLONG page)
{
	POLY_FT4	*p;
	SLONG	z;

	if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
		return;

	p=(POLY_FT4 *)the_display.CurrentPrim;

	setPolyFT4(p);
	setUV4(p,quad[0]->u,quad[0]->v
			,quad[1]->u,quad[1]->v
			,quad[2]->u,quad[2]->v
			,quad[3]->u,quad[3]->v);

	setRGB0(p,128,128,128);
	setXY4(p,quad[0]->X,quad[0]->Y,quad[1]->X,quad[1]->Y,quad[2]->X,quad[2]->Y,quad[3]->X,quad[3]->Y);

	z=quad[0]->Z;
	if(z<quad[1]->Z)
		z=quad[1]->Z;

	if(z<quad[2]->Z)
		z=quad[2]->Z;

	if(z<quad[3]->Z)
		z=quad[3]->Z;


	z>>=0;
	z=get_z_sort(z);

//						z=max_z; //POLY_buffer[p0].Z;
	

	p->tpage=psx_tpages[page];	   

	p->clut =getClut(960+((page&3)<<4),256+(page>>2));

	addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
	the_display.CurrentPrim+=sizeof(POLY_FT4);

}
*/


void	DRAW_ladder_rungs(SLONG x1,SLONG z1,SLONG x2,SLONG z2,struct DFacet	*p_facet,SLONG dx,SLONG dz,int backwards)
{
	SLONG	count;
	SLONG	y;
	SVECTOR quad[4];
	POLY_FT4 *pp;
	SLONG	page,p,flag,z;
	SLONG	clip_flag=0;
	SWORD	b0;
	UBYTE	u,v;

	//
	//  do a back face cull to reject whole facet
	//

	//
	// do a height test so just process rungs in height range we can see
	//


//	dx=x2-x1;
//	dz=z2-z1;

	x1+=(dx*3)>>2;//(dx*(3.0/4.0)); //>>2;
	z1+=(dz*3)>>2;//(dz*(3.0/4.0)); //>>2;

	x2-=(dx*3)>>2;//(dx*(3.0/4.0)); //>>2;
	z2-=(dz*3)>>2;//(dz*(3.0/4.0)); //>>2;

	u=getPSXU(POLY_PAGE_LADDER);
	v=getPSXV(POLY_PAGE_LADDER);

	y=(SLONG)p_facet->Y[0]-POLY_cam_y;
	count=(p_facet->Height);

	quad[0].vx=x1;
	quad[0].vz=z1;
	quad[1].vx=x2;
	quad[1].vz=z2;
	quad[2].vx=x1;
	quad[2].vz=z1;
	quad[3].vx=x2;
	quad[3].vz=z2;

//	pp=(POLY_FT4*)the_display.CurrentPrim;


	while(count--&&(clip_flag<2))
	{

		y+=BLOCK_SIZE;
		quad[0].vy=quad[1].vy=y;
		quad[2].vy=quad[3].vy=y-8;
		
		
		ALLOCPRIM(pp,POLY_FT4);
		gte_RotTransPers4(&quad[0],&quad[1],&quad[2],&quad[3],(SLONG*)&pp->x0,(SLONG*)&pp->x1,(SLONG*)&pp->x2,(SLONG*)&pp->x3,&p,&flag,&z);
		z=get_z_sort(z>>0);

		if (backwards)
			z-=15;
		else
			z+=5;

		b0=getPSXFade(p);

		if (dz) 
			b0=(b0*3)>>2;

		SATURATE(b0,0,128);

		if ((b0==0)||(flag&(1<<31))||
			(MAX4(pp->x0,pp->x1,pp->x2,pp->x3)<0)||
			(MIN4(pp->x0,pp->x1,pp->x3,pp->x3)>511)||
			(MAX4(pp->y0,pp->y1,pp->y2,pp->y3)<0)||
			(MIN4(pp->y0,pp->y1,pp->y2,pp->y3)>255))
		{
			// Experimental	clip flag routine, should be faster than
			clip_flag|=clip_flag<<1;
//			if (clip_flag&1)
//				clip_flag!=2;
		} else
			clip_flag|=1;


		if (clip_flag&1)
		{	

			setPolyFT4(pp);
			
			//
			// Texture the quad.
			// 
			setUVWH(pp,u,v+2,31,7);
			pp->tpage=getPSXTPageE(POLY_PAGE_LADDER);
			pp->clut=getPSXClutE(POLY_PAGE_LADDER);

//			texture_psx_quad(pp,dstyles[p_facet->StyleIndex],3,5,31);

//			if (b0!=128)
//				setSemiTrans(pp,1);
		
			setRGB0(pp,b0,b0,b0);

			DOPRIM(z,pp);
//			ALLOCPRIM(pp,POLY_FT4);
//			addPrim(&the_display.CurrentDisplayBuffer->ot[z],pp);
		}
	}
//	DEALLOCPRIM(POLY_FT4);
}

#define	LADDER_SPINE_WIDTH		12

PSX_POLY_Point ladder_pp_array[8];

void	DRAW_ladder_sides(SLONG x1,SLONG z1, struct DFacet	*p_facet,SLONG dx,SLONG dz,int backwards)
{
	SLONG	count;
	SLONG	y;
	PSX_POLY_Point *quad=ladder_pp_array;
	POLY_FT4 *pp;
//	POLY_FT4 *pp;
	SLONG	height;
	SLONG	x1mdz;
	SLONG	z1pdx;
	SLONG	c0=0;//,pc=0;
	SLONG	face_flag=0;
	SLONG	bottom_off=FALSE;
	SLONG	top_off=FALSE;
	SWORD	b0,z;
	UBYTE	u,v;


	//
	//  do a back face cull to reject whole facet
	//

	//
	// do a height test so just process rungs in height range we can see
	//

	u=getPSXU(POLY_PAGE_LADDER);
	v=getPSXV(POLY_PAGE_LADDER);

	y=(SLONG)p_facet->Y[0]-POLY_cam_y;
	count=(p_facet->Height)>>2;

	if ((p_facet->Height&3)==0)
		height=256;
	else
		height=(SLONG)((p_facet->Height*BLOCK_SIZE)/count);

//	height=256;

	
		x1mdz=x1-dz-dx;


		z1pdx=z1+dz+dx;


		quad[0].World.vx=quad[4].World.vx=x1mdz;
		quad[0].World.vy=quad[4].World.vy=y;
		quad[0].World.vz=quad[4].World.vz=z1pdx;
		                 
		quad[1].World.vx=quad[5].World.vx=x1mdz;
		quad[1].World.vy=quad[5].World.vy=y;
		quad[1].World.vz=quad[5].World.vz=z1;
		                 
		quad[2].World.vx=quad[6].World.vx=x1;
		quad[2].World.vy=quad[6].World.vy=y;
		quad[2].World.vz=quad[6].World.vz=z1;
		                 
		quad[3].World.vx=quad[7].World.vx=x1;
		quad[3].World.vy=quad[7].World.vy=y;
		quad[3].World.vz=quad[7].World.vz=z1pdx;

		gte_RotTransPers4(&quad[0].World,&quad[1].World,&quad[2].World,&quad[3].World,&quad[0].SYSX,&quad[1].SYSX,&quad[2].SYSX,&quad[3].SYSX,&quad[0].P,&quad[0].Flag,&quad[0].Z);

		if ((quad[4].Flag&(1<<31))
			||(MAX4(quad[0].Word.SX,quad[1].Word.SX,quad[2].Word.SX,quad[3].Word.SX)<0)
			||(MIN4(quad[0].Word.SX,quad[1].Word.SX,quad[2].Word.SX,quad[3].Word.SX)>511)
			||(MAX4(quad[0].Word.SY,quad[1].Word.SY,quad[2].Word.SY,quad[3].Word.SY)<0)
			||(MIN4(quad[0].Word.SY,quad[1].Word.SY,quad[2].Word.SY,quad[3].Word.SY)>255))
				bottom_off=TRUE;

		quad[0].Z=get_z_sort(quad[0].Z>>0);

		while((c0<count)&&!top_off)
		{
			quad[4].World.vy+=height;
			quad[5].World.vy+=height;
			quad[6].World.vy+=height;
			quad[7].World.vy+=height;

			gte_RotTransPers4(&quad[4].World,&quad[5].World,&quad[6].World,&quad[7].World,&quad[4].SYSX,&quad[5].SYSX,&quad[6].SYSX,&quad[7].SYSX,&quad[4].P,&quad[4].Flag,&quad[4].Z);
			quad[4].Z=get_z_sort(quad[4].Z>>0);

			b0=getPSXFade(quad[0].P);
			SATURATE(b0,0,128);

 			if ((b0==0)||(quad[4].Flag&(1<<31))
				||(MAX4(quad[4].Word.SX,quad[5].Word.SX,quad[6].Word.SX,quad[7].Word.SX)<0)
				||(MIN4(quad[4].Word.SX,quad[5].Word.SX,quad[6].Word.SX,quad[7].Word.SX)>511)
				||(MAX4(quad[4].Word.SY,quad[5].Word.SY,quad[6].Word.SY,quad[7].Word.SY)<0)
				||(MIN4(quad[4].Word.SY,quad[5].Word.SY,quad[6].Word.SY,quad[7].Word.SY)>255))
			{
				if (!bottom_off)
					 top_off=TRUE;
			} else
				bottom_off=FALSE;

			if (!bottom_off)
			{
				if (face_flag==0)
				{
					face_flag|=(MF_NormalClip(quad[0].SYSX,quad[1].SYSX,quad[4].SYSX)>0)?1:0;
					face_flag|=(MF_NormalClip(quad[1].SYSX,quad[2].SYSX,quad[5].SYSX)>0)?2:0;
					face_flag|=(MF_NormalClip(quad[2].SYSX,quad[3].SYSX,quad[6].SYSX)>0)?4:0;
					face_flag|=(MF_NormalClip(quad[3].SYSX,quad[0].SYSX,quad[7].SYSX)>0)?8:0;
				}

				z=min(quad[0].Z,quad[4].Z);
				if (backwards)
					z-=15;
				else
					z+=5;

				if ( face_flag&1 )
				{
//					face_flag|=1;
	
					ALLOCPRIM(pp,POLY_FT4);
					setPolyFT4(pp);
	
					setUVWH(pp,u,v,31,31);
					pp->tpage=getPSXTPageE(POLY_PAGE_LADDER);
					pp->clut=getPSXClutE(POLY_PAGE_LADDER);

//					texture_psx_quad(pp,dstyles[p_facet->StyleIndex],1,5,31);
//		 			if (b0!=128)
//						setSemiTrans(pp,1);

					setRGB0(pp,b0>>1,b0>>1,b0>>1);
					setXY4(pp,	quad[0].Word.SX,quad[0].Word.SY,
								quad[1].Word.SX,quad[1].Word.SY,
								quad[4].Word.SX,quad[4].Word.SY,
								quad[5].Word.SX,quad[5].Word.SY);

					addPrim(&the_display.CurrentDisplayBuffer->ot[z],pp);
				}

				if ( face_flag&2 )
				{
//					face_flag|=2;

					ALLOCPRIM(pp,POLY_FT4);
					setPolyFT4(pp);
	
					setUVWH(pp,u,v,31,31);
					pp->tpage=getPSXTPageE(POLY_PAGE_LADDER);
					pp->clut=getPSXClutE(POLY_PAGE_LADDER);

//					texture_psx_quad(pp,dstyles[p_facet->StyleIndex],1,5,31);

//					if (b0!=128)
//						setSemiTrans(pp,1);
	
					setRGB0(pp,b0,b0,b0);
					setXY4(pp,	quad[1].Word.SX,quad[1].Word.SY,
								quad[2].Word.SX,quad[2].Word.SY,
								quad[5].Word.SX,quad[5].Word.SY,
								quad[6].Word.SX,quad[6].Word.SY);

					addPrim(&the_display.CurrentDisplayBuffer->ot[z],pp);
				}

				if ( face_flag&4 )
				{
//					face_flag|=4;
	
					ALLOCPRIM(pp,POLY_FT4);
					setPolyFT4(pp);
	
					setUVWH(pp,u,v,31,31);
					pp->tpage=getPSXTPageE(POLY_PAGE_LADDER);
					pp->clut=getPSXClutE(POLY_PAGE_LADDER);

//					texture_psx_quad(pp,dstyles[p_facet->StyleIndex],1,5,31);
	
//					if (b0!=128)
//						setSemiTrans(pp,1);

					setRGB0(pp,(b0*3)>>2,(b0*3)>>2,(b0*3)>>2);
					setXY4(pp,	quad[2].Word.SX,quad[2].Word.SY,
								quad[3].Word.SX,quad[3].Word.SY,
								quad[6].Word.SX,quad[6].Word.SY,
								quad[7].Word.SX,quad[7].Word.SY);

					addPrim(&the_display.CurrentDisplayBuffer->ot[z],pp);
				}

				if ( face_flag&8 )
				{
//					face_flag|=8;
	
					ALLOCPRIM(pp,POLY_FT4);
					setPolyFT4(pp);
	
					setUVWH(pp,u,v,31,31);
					pp->tpage=getPSXTPageE(POLY_PAGE_LADDER);
					pp->clut=getPSXClutE(POLY_PAGE_LADDER);

//					texture_psx_quad(pp,dstyles[p_facet->StyleIndex],1,5,31);
	
					setRGB0(pp,b0,b0,b0);
					setXY4(pp,	quad[3].Word.SX,quad[3].Word.SY,
								quad[0].Word.SX,quad[0].Word.SY,
								quad[7].Word.SX,quad[7].Word.SY,
								quad[4].Word.SX,quad[4].Word.SY);

					addPrim(&the_display.CurrentDisplayBuffer->ot[quad[0].Z],pp);
				}
				face_flag|=16;
			}

			memcpy(&quad[0],&quad[4],sizeof(PSX_POLY_Point)*4);

			c0++;
		}
		//printf("%d\n",pc);
		if (!top_off&&(MF_NormalClip(quad[4].SYSX,quad[5].SYSX,quad[6].SYSX)>0))
		{
			ALLOCPRIM(pp,POLY_FT4);
			setPolyFT4(pp);
	
			setUVWH(pp,u,v,31,31);
			pp->tpage=getPSXTPageE(POLY_PAGE_LADDER);
			pp->clut=getPSXClutE(POLY_PAGE_LADDER);

//			texture_psx_quad(pp,dstyles[p_facet->StyleIndex],1,5,31);

			setRGB0(pp,b0,b0,b0);
			setXY4(pp,	quad[4].Word.SX,quad[4].Word.SY,
						quad[5].Word.SX,quad[5].Word.SY,
						quad[7].Word.SX,quad[7].Word.SY,
						quad[6].Word.SX,quad[6].Word.SY);

			addPrim(&the_display.CurrentDisplayBuffer->ot[quad[4].Z],pp);
		}
}
   
void	DRAW_ladder(struct DFacet *p_facet)
{
	SLONG	dx,dz;
	SLONG	dx3,dz3;
	SLONG	x1,z1,x2,z2;
	
	//
	// Backface cull the entire facet?
	//

	SLONG vec2x;
	SLONG vec2z;

	SLONG cprod,reverse;

//	return;

	x1=(p_facet->x[0]<<8)-POLY_cam_x;
	x2=(p_facet->x[1]<<8)-POLY_cam_x;
	z1=(p_facet->z[0]<<8)-POLY_cam_z;
	z2=(p_facet->z[1]<<8)-POLY_cam_z;

	dx = x2 - x1;
	dz = z2 - z1;

	vec2x = - x1;
	vec2z = - z1;

	cprod = dx*vec2z - dz*vec2x;
	
	if (cprod >= 0)
		reverse=1;
	else
		reverse=0;

	//
	// Don't bother drawing the sewer ladders when we're not in the sewers.
	//
/*
	if (p_facet->FacetFlags & FACET_FLAG_LADDER_LINK)
	{
		//
		// We always have to draw these facets.
		//
	}
	else
	{
		if (p_facet->FacetFlags & FACET_FLAG_IN_SEWERS)
		{
			if (!(GAME_FLAGS & GF_SEWERS))
			{
				return;
			}
		}
		else
		{
			if (GAME_FLAGS & GF_SEWERS)
			{
				return;
			}
		}
	}
*/
/*
*/
//	dx=x2-x1;
//	dz=z2-z1;

	dx3=(dx*21845)>>16; //   divide by 3
	dz3=(dz*21845)>>16; //   divide by 3

	x1+=dx3;
	z1+=dz3;

	x2-=dx3;
	z2-=dz3;

	dx>>=3;
	dz>>=3;

	x1+=dz;
	x2+=dz;

	z1-=dx;
	z2-=dx;

	dx=x2-x1;
	dz=z2-z1;

	if(dx>0)
		dx=LADDER_SPINE_WIDTH;
	else
		if(dx<0)
			dx=-LADDER_SPINE_WIDTH;

	if(dz>0)
		dz=LADDER_SPINE_WIDTH;
	else
		if(dz<0)
			dz=-LADDER_SPINE_WIDTH;

	DRAW_ladder_rungs((SLONG)x1,(SLONG)z1,(SLONG)x2,(SLONG)z2,p_facet,(SLONG)dx,(SLONG)dz,reverse);
	DRAW_ladder_sides((SLONG)x1+dx,(SLONG)z1,p_facet,(SLONG)dx,(SLONG)dz,reverse);
	DRAW_ladder_sides((SLONG)x2,(SLONG)z2-dz,p_facet,(SLONG)dx,(SLONG)dz,reverse);
}

#ifdef SEWERS_EXIST
void FACET_draw_ns_ladder(
		SLONG x1,
		SLONG z1,
		SLONG x2,
		SLONG z2,
		SLONG height)
{
	DFacet df;

	SLONG y;

	SLONG mx = (x1 + x2 << 7) + ((z2 - z1) << 2) >> 8;
	SLONG mz = (z1 + z2 << 7) - ((x2 - x1) << 2) >> 8;

	ASSERT(WITHIN(mx, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(mz, 0, PAP_SIZE_HI - 1));

	y  = 0;//NS_hi[mx][mz].bot << 5;
	y += -32 * 0x100; 

	//
	// Create a pretend facet for the ladder facet function to use.
	//

	df.FacetType = STOREY_TYPE_LADDER;
	df.x[0]      = x1;
	df.z[0]      = z1;
	df.x[1]      = x2;
	df.z[1]      = z2;
	df.Y[0]      = y;
	df.Y[1]      = y;
	df.Height    = height * 4;

	//
	// Draw a ladder facet.
	//

	DRAW_ladder(&df,INFINITY);
}
#endif

#define	SET_TX_TY(p,x1,y1,x2,y2,x3,y3,x4,y4)	setUV4(p,x1,y1,x2,y2,x3,y3,x4,y4)

void set_roof_texture(POLY_GT4 *p,UWORD texture,int flip)
{
	SLONG tpage;
	SLONG trot;
	SLONG tflip;

	SLONG num;

	SLONG u;
	SLONG v;
	SLONG s;

	if(texture)
	{
		UWORD	cpage;
		SLONG	clutx,cluty;
		u    = (texture >> 0x0) & 0x7;
		v    = (texture >> 0x3) & 0x7;
		tpage = (texture >> 0x6) & 0xf;
		trot  = (texture >> 0xa) & 0x3 | (flip>>5);
		tflip = (texture >> 0xc) & 0x3;
		s = 31; //(texture >> 0xe) & 0x3;



//		cpage=tpage*64+u+(v<<3);
		cpage=texture&0x3ff;

		cluty=256+(cpage>>2);
		clutx=960+((cpage&3)<<4);
		p->clut=getClut(clutx,cluty);

		u=u<<5;
		v=v<<5;

	}
	else
	{
		u    = 0;
		v    = 0;
		tpage = 1;
		trot  = flip>>5;
		tflip = 0;
		s = 31; //(texture >> 0xe) & 0x3;
		p->clut  = getClut(960,256);
	}


	p->tpage = psx_tpages[tpage];
//	setUV4(p,u,v,u+s,v,u,v+s,u+s,v+s);

	//
	// The texture coordinates depend of the rotation.
	//


	switch(trot)
	{

		case	0:		
			SET_TX_TY(p,u,v,u+s,v,u,v+s,u+s,v+s);
			break;
		case	1:		
			SET_TX_TY(p,u+s,v,u+s,v+s,u,v,u,v+s);
			break;
		case	2:	
			SET_TX_TY(p,u+s,v+s,u,v+s,u+s,v,u,v);
			break;
		case	3:	
			SET_TX_TY(p,u,v+s,u,v,u+s,v+s,u+s,v);
			break;
		case	4:
			SET_TX_TY(p,u+s,v,u,v,u+s,v+s,u,v+s);
			break;
		case	5:
			SET_TX_TY(p,u+s,v+s,u+s,v,u,v+s,u,v);
			break;
		case	6:	
			SET_TX_TY(p,u,v+s,u+s,v+s,u,v,u+s,v);
			break;
		case	7:	
			SET_TX_TY(p,u,v,u,v+s,u+s,v,u+s,v+s);
			break;
	}

}

extern	UBYTE	in_ware;

void FACET_draw_walkable(SLONG build)
{
	SLONG		i,count;

	SLONG		sp;
	SLONG		ep;

	RoofFace4	*p_f4;

	struct		DWalkable *p_walk;
	struct		DBuilding *p_dbuilding=&dbuildings[build];

	PSX_POLY_Point *pp=perm_pp_array;

	POLY_GT4	*p;
//	POLY_FT4	*p;
	PAP_Hi		*ph;

	SLONG		walkable;

	SLONG		z;
	SLONG		flag;

	SLONG		max_y;
	SLONG		col,lum;

	UBYTE		col_red,col_green,col_blue;
	UBYTE		col_red1,col_green1,col_blue1;
	UBYTE		col_red2,col_green2,col_blue2;
	UBYTE		col_red3,col_green3,col_blue3;
	UBYTE		warehouse,NormalRot;

	UWORD		*rooftex=NULL;


	ASSERT(build>=1 && build<=next_dbuilding-1);
	col_red=NIGHT_amb_red<<1;
	col_green=NIGHT_amb_green<<1;
	col_blue=NIGHT_amb_blue<<1;

	//
	// Draw each facet.
	//

	p=(POLY_GT4*)the_display.CurrentPrim;
//	p=(POLY_FT4*)the_display.CurrentPrim;

	warehouse=(p_dbuilding->Type == BUILDING_TYPE_WAREHOUSE);
	max_y=(warehouse?INFINITY:NGAMUT_Ymax+260);

	if (warehouse)
	{
		rooftex = &WARE_rooftex[WARE_ware[p_dbuilding->Ware].rooftex];
	}

	count=0;
	for (walkable = p_dbuilding->Walkable; walkable; walkable = p_walk->Next)
	{
		SLONG world_x=POLY_cam_x;
		SLONG world_y=POLY_cam_y;
		SLONG world_z=POLY_cam_z;
//		SLONG	max_z=-999999;
		ULONG	all_flags=0xffffffff;

		p_walk = &dwalkables[walkable];


		count++;
		ASSERT(count<100);

#ifdef INSIDES_EXIST
		//
		// REJECTION OF WHOLE WALKABLE SET IF ITS ABOVE THE CAMERA?
		//
		if ((build==INDOORS_DBUILDING) && (p_walk->StoreyY*256 >= inside_storeys[INDOORS_INDEX].StoreyY))
			break;
#endif
		sp = p_walk->StartFace4;
		ep = p_walk->EndFace4;

//		ASSERT((ep-sp)!=0);
		if(ep==sp)
			continue;

//			POLY_buffer_upto = 0;
//			POLY_shadow_upto = 0;

			p_f4=&roof_faces4[sp];

			for (i = sp; i < ep; i++,rooftex++)
			{
				SLONG	tflag,rx,rz;
				PSX_Screen_XY sysx;

//				ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

				col=floor_psx_col[rx=p_f4->RX&127][rz=p_f4->RZ&127];
				col_red=(col>>10)<<2;
				col_green=((col>>5)&0x1f)<<3;
				col_blue=((col)&0x1f)<<3;

				col=floor_psx_col[rx+1][rz];
				col_red1=(col>>10)<<2;
				col_green1=((col>>5)&0x1f)<<3;
				col_blue1=((col)&0x1f)<<3;

				col=floor_psx_col[rx][rz+1];
				col_red2=(col>>10)<<2;
				col_green2=((col>>5)&0x1f)<<3;
				col_blue2=((col)&0x1f)<<3;

				col=floor_psx_col[rx+1][rz+1];
				col_red3=(col>>10)<<2;
				col_green3=((col>>5)&0x1f)<<3;
				col_blue3=((col)&0x1f)<<3;

				if(!(p_f4->DrawFlags&RFACE_FLAG_NODRAW))
				if(p_f4->Y <=max_y)
				{
					if (p_f4->RX&128)
					{
						pp[1].World.vx=	(rx<<8)-world_x;
						pp[1].World.vy=	(p_f4->Y)-world_y;
						pp[1].World.vz=	(rz<<8)-world_z;

						pp[0].World.vx=pp[1].World.vx+256;
						pp[0].World.vy=pp[1].World.vy+(p_f4->DY[0]<<ROOF_SHIFT);
						pp[0].World.vz=pp[1].World.vz;
						
						pp[3].World.vx=pp[1].World.vx;
						pp[3].World.vy=pp[1].World.vy+(p_f4->DY[2]<<ROOF_SHIFT);
						pp[3].World.vz=pp[1].World.vz+256;

						pp[2].World.vx=pp[0].World.vx;
						pp[2].World.vy=pp[1].World.vy+(p_f4->DY[1]<<ROOF_SHIFT);
						pp[2].World.vz=pp[3].World.vz;
					}
					else
					{
						pp[0].World.vx=	(rx<<8)-world_x;
						pp[0].World.vy=	(p_f4->Y)-world_y;
						pp[0].World.vz=	(rz<<8)-world_z;

						pp[1].World.vx=pp[0].World.vx+256;
						pp[1].World.vy=pp[0].World.vy+(p_f4->DY[0]<<ROOF_SHIFT);
						pp[1].World.vz=pp[0].World.vz;
						
						pp[2].World.vx=pp[0].World.vx;
						pp[2].World.vy=pp[0].World.vy+(p_f4->DY[2]<<ROOF_SHIFT);
						pp[2].World.vz=pp[0].World.vz+256;

						pp[3].World.vx=pp[1].World.vx;
						pp[3].World.vy=pp[0].World.vy+(p_f4->DY[1]<<ROOF_SHIFT);
						pp[3].World.vz=pp[2].World.vz;
					}
					gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
					gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
					gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
					gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

					flag=pp[0].Flag&pp[1].Flag&pp[2].Flag&pp[3].Flag;

					if (!(flag&(1<<31))&&
						(MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>=0)&&
						(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>=0)&&
						(MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)&&
						(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256))
					{
						
						if (warehouse||(p_f4->RX&128)||(p_f4->RZ&128)||(MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX)>0))
						{
							UWORD b0;

							b0=getPSXFade(pp[0].P);

//							if (b0!=128)
//								setSemiTrans(p,1);
	
							if (b0>0)
							{
								check_prim_ptr((void**)&p);
								setPolyGT4(p);
//								setPolyFT4(p);
								setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
									     pp[1].Word.SX,pp[1].Word.SY,
									     pp[2].Word.SX,pp[2].Word.SY,
									     pp[3].Word.SX,pp[3].Word.SY);
								if(in_ware && warehouse)
								{
									setRGB0(p,0,0,0);
									setRGB1(p,0,0,0);
									setRGB2(p,0,0,0);
									setRGB3(p,0,0,0);
								}
								else
								if (p_f4->RX&128)
								{
									lum=LUMI(rx,rz);
									setRGB1(p,MAKELUMI((col_red*b0)>>7,lum),MAKELUMI((col_green*b0)>>7,lum),MAKELUMI((col_blue*b0)>>7,lum));
									lum=LUMI(rx+1,rz);
									setRGB0(p,MAKELUMI((col_red1*b0)>>7,lum),MAKELUMI((col_green1*b0)>>7,lum),MAKELUMI((col_blue1*b0)>>7,lum));
									lum=LUMI(rx,rz+1);											
									setRGB3(p,MAKELUMI((col_red2*b0)>>7,lum),MAKELUMI((col_green2*b0)>>7,lum),MAKELUMI((col_blue2*b0)>>7,lum));
									lum=LUMI(rx+1,rz+1);
									setRGB2(p,MAKELUMI((col_red3*b0)>>7,lum),MAKELUMI((col_green3*b0)>>7,lum),MAKELUMI((col_blue3*b0)>>7,lum));
								}
								else
								{
									lum=LUMI(rx,rz);
									setRGB0(p,MAKELUMI((col_red*b0)>>7,lum),MAKELUMI((col_green*b0)>>7,lum),MAKELUMI((col_blue*b0)>>7,lum));
									lum=LUMI(rx+1,rz);
									setRGB1(p,MAKELUMI((col_red1*b0)>>7,lum),MAKELUMI((col_green1*b0)>>7,lum),MAKELUMI((col_blue1*b0)>>7,lum));
									lum=LUMI(rx,rz+1);
									setRGB2(p,MAKELUMI((col_red2*b0)>>7,lum),MAKELUMI((col_green2*b0)>>7,lum),MAKELUMI((col_blue2*b0)>>7,lum));
									lum=LUMI(rx+1,rz+1);
									setRGB3(p,MAKELUMI((col_red3*b0)>>7,lum),MAKELUMI((col_green3*b0)>>7,lum),MAKELUMI((col_blue3*b0)>>7,lum));
								}
								ph=&PAP_2HI(rx,rz);
								if (warehouse)
									set_roof_texture(p,*rooftex,p_f4->RX&128);
								else
									set_roof_texture(p,ph->Texture,p_f4->RX&128);
								z=MAX4(pp[0].Z,pp[1].Z,pp[2].Z,pp[3].Z)>>0;
								//z-=10;
								z=get_z_sort(z);
								DOPRIM(z,p);
								p++;
							}
						}
					}
				}
			p_f4++;
		}   
	
	}		 
	check_prim_ptr((void**)&p);
	the_display.CurrentPrim=(UBYTE*)p;
}

void	cable_draw(struct DFacet *p_facet)
{
	SLONG	p1;
	SLONG	len,dx,dy,dz,count;
	SLONG	px,py,pz;
	SLONG	c0,b0;
	SLONG	light_x,light_y,light_z;
	SLONG	step_angle1,step_angle2,angle;

	SLONG	x1,y1,z1,x2,y2,z2;
	SLONG	ex,ey,ez;
	SLONG	step_x,step_y,step_z,sag;
	ULONG	flag;
	UBYTE	*cp;
	SLONG	red,green,blue;


	cp=the_display.CurrentPrim;

	PSX_POLY_Point *pp=perm_pp_array;


	x1=(p_facet->x[0]<<8)-POLY_cam_x;
	y1=p_facet->Y[0]-POLY_cam_y;
	z1=(p_facet->z[0]<<8)-POLY_cam_z;

	x2=(p_facet->x[1]<<8)-POLY_cam_x;
	y2=p_facet->Y[1]-POLY_cam_y;
	z2=(p_facet->z[1]<<8)-POLY_cam_z;



	count=p_facet->Height;
	dx=(x2-x1);
	dy=(y2-y1);
	dz=(z2-z1);


	px=ex=x1;
	py=ey=y1;
	pz=ez=z1;

	ex<<=8;
	ey<<=8;
	ez<<=8;

	step_x=(dx<<8)/count;
	step_y=(dy<<8)/count;
	step_z=(dz<<8)/count;

	

	pp[0].World.vx=px;
	pp[0].World.vy=py;
	pp[0].World.vz=pz;

	pp[1].World.vx=px;
	pp[1].World.vy=py+8;
	pp[1].World.vz=pz;

	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);

	step_angle1=(SWORD)p_facet->StyleIndex;
	step_angle2=(SWORD)p_facet->Building;

	sag=p_facet->FHeight * 64;

	red=NIGHT_amb_red;
	green=NIGHT_amb_green;
	blue=NIGHT_amb_blue;

	angle=-512;
	for(c0=1;c0<=count;c0++)
	{
		SLONG	oy;

		angle+=step_angle1;
		if(angle>=-30)
		{
			step_angle1=step_angle2;
		}


		ex+=step_x;
		ey+=step_y;
		ez+=step_z;
		oy=(COS((angle+2048)&2047)*sag)>>16;


		pp[2].World.vx=ex>>8;
		pp[2].World.vy=(ey>>8)-oy;
		pp[2].World.vz=(ez>>8);

		pp[3].World.vx=ex>>8;
		pp[3].World.vy=(ey>>8)-oy+8;
		pp[3].World.vz=(ez>>8);
		gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
		gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

		flag=pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag;
		if( (flag&(1<<31))==0)
		{
			{
				SLONG	tflag,c0;
				flag=0xffffffff;
				for(c0=0;c0<4;c0++)
				{
					tflag=0;
					if(pp[c0].Word.SX<0)
						tflag|=POLY_CLIP_LEFT;
					if(pp[c0].Word.SX>=320)
						tflag|=POLY_CLIP_RIGHT;
					if(pp[c0].Word.SY<0)
						tflag|=POLY_CLIP_TOP;
					if(pp[c0].Word.SY>=SCREEN_HEIGHT)
						tflag|=POLY_CLIP_BOTTOM;
					flag&=tflag;
				}

				b0=getPSXFade(pp[0].P);
//				if(0)
				if(flag==0)
				{
					POLY_F4	*p;
					SLONG	z;
					SLONG	page;

					z=MAX4(pp[0].Z,pp[1].Z,pp[2].Z,pp[3].Z);
					if(z<(20*256>>2) )
					{

						check_prim_ptr((void**)&cp);
						p=(POLY_F4 *)cp; //the_display.CurrentPrim;

						setPolyF4(p);

	//					if (b0<128)
	//						setSemiTrans(p,1);

						setRGB0(p,(red*b0)>>8,(green*b0)>>8,(blue*b0)>>8);
							//b0>>2,b0>>2,b0>>1);
			//			setRGB0(p,255,0,0);//b0>>3,b0>>3,b0>>3);

						setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
							pp[1].Word.SX,pp[1].Word.SY,
							pp[2].Word.SX,pp[2].Word.SY,
							pp[3].Word.SX,pp[3].Word.SY);

						//z>>=0;
						z=get_z_sort(z);

						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						cp+=sizeof(POLY_F4);
					}
					else
					{
						//
						// far away, and getting further
						//

						if(pp[3].Z>pp[0].Z)
						{
							check_prim_ptr((void**)&cp);
							the_display.CurrentPrim=cp;
							return;
						}

					}
				}
			}
		}
		pp[0].SYSX=pp[2].SYSX;
		pp[0].Z=pp[2].Z;
		pp[0].P=pp[2].P;
		pp[0].Flag=pp[2].Flag;

		pp[1].SYSX=pp[3].SYSX;
		pp[1].Z=pp[3].Z;
		pp[1].P=pp[3].P;
		pp[1].Flag=pp[3].Flag;
	}
	check_prim_ptr((void**)&cp);
	the_display.CurrentPrim=cp;


}

