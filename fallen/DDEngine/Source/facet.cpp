#include	"game.h"

#include	"aeng.h"
#include	"mesh.h"
#include	"math.h"
#include	"poly.h"
#include	"c:\fallen\headers\supermap.h"
#include	"c:\fallen\headers\inside2.h"
#include	"night.h"
#include	"c:\fallen\headers\pap.h"
#include	"math.h"
#include	"ns.h"
#include	"c:\fallen\sedit\headers\es.h"
#include	"memory.h"
#include "texture.h"
#include	"polypage.h"
#include	"ware.h"
#include	"fc.h"
#include	"font2d.h"
#include	"smap.h"
#include	"superfacet.h"
#include	"supercrinkle.h"
#include	"matrix.h"

// temptemptempetc.
#include "sprite.h"


#ifdef TARGET_DC
#include "target.h"
#endif

#ifdef TARGET_DC
// intrinsic maths
#include <shsgintr.h>
#endif


#ifndef TARGET_DC

#define LOG_ENTER(x) {}
#define LOG_EXIT(x)  {}
#define LOG_EVENT(x) {}
#define	POLY_set_local_rotation_none() {}

#endif




#define FACETINFO
#ifdef  FACETINFO

//
// This is code to count the number of texture pages used in each
// facet and how many quads use each page...
//

typedef struct
{
	UWORD page;
	UWORD quads;
	
} FACET_Pageinfo;

#define FACET_MAX_PAGEINFO_PER_FACET 32

typedef struct
{
	UBYTE          done;
	UBYTE          num_pages;
	FACET_Pageinfo page[FACET_MAX_PAGEINFO_PER_FACET];

} FACET_Facetinfo;

#define FACET_MAX_FACETINFO 4096

FACET_Facetinfo FACET_facetinfo[FACET_MAX_FACETINFO];

//
// The facet we are currently drawing... (this isn't passed down to FillFacetPointsCommon())!
//

SLONG FACET_facetinfo_current;


#endif
 



float FACET_direction_matrix[9];




void FACET_output(CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[512];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	OutputDebugString(message);
}



void FACET_facetinfo_trace(void)
{
	SLONG i;
	SLONG j;

	SLONG num_facets = 0;
	SLONG num_pages  = 0;
	SLONG num_quads  = 0;

	SLONG max_pages_per_facet = 0;

	#define MAX_QUADS_PER_PAGE 1024

	SLONG quads_per_page[MAX_QUADS_PER_PAGE];

	memset(quads_per_page, 0, sizeof(quads_per_page));

	float ave_pages_per_facet;
	float ave_quads_per_page;

	FACET_Facetinfo *ff;

	for (i = 0; i < FACET_MAX_FACETINFO; i++)
	{
		ff = &FACET_facetinfo[i];

		if (ff->done)
		{
			num_facets += 1;
			num_pages  += ff->num_pages;
			
			for (j = 0; j < ff->num_pages; j++)
			{
				num_quads += ff->page[j].quads;

				ASSERT(WITHIN(ff->page[j].quads, 0, MAX_QUADS_PER_PAGE - 1));

				quads_per_page[ff->page[j].quads] += 1;
			}

			if (ff->num_pages > max_pages_per_facet)
			{
				max_pages_per_facet = ff->num_pages;
			}
		}
	}

	ave_pages_per_facet = float(num_pages) / float(num_facets);
	ave_quads_per_page  = float(num_quads) / float(num_pages );

	FACET_output("*****************************************\n");
	FACET_output("\n");
	FACET_output("Num facets = %d\n", num_facets);
	FACET_output("Average number of pages per facet = %f\n", ave_pages_per_facet);
	FACET_output("Average number of quads per page  = %f\n", ave_quads_per_page );
	FACET_output("Max pages per facet               = %d\n", max_pages_per_facet);
	FACET_output("\n");
	
	for (i = 0; i < MAX_QUADS_PER_PAGE; i++)
	{
		FACET_output("%4d Quads per page : %5d\n", i, quads_per_page[i]);
	}

	FACET_output("\n");
	FACET_output("*****************************************\n");
}









static int iNumFacets = 0;
static int iNumFacetTextures = 0;




//#define	QUICK_FACET	1
//#define	FACET_REMOVAL_TEST		// show removed facets and use 'F' key to swap (must be defined in build2.cpp too)

SLONG	dfacets_drawn_this_gameturn;
extern	SLONG	get_fence_hole(struct DFacet *p_facet);
extern	SLONG	get_fence_hole_next(struct DFacet *p_facet,SLONG along);

static	ULONG	facet_seed=0x12345678;

inline void	apply_cloud(SLONG x,SLONG y,SLONG z,ULONG *col)
{
	return;
}

void	DRAW_ladder(struct DFacet *p_facet);


void FACET_draw(SLONG facet,UBYTE alpha);
void FACET_draw_rare(SLONG facet,UBYTE alpha);


extern BOOL allow_debug_keys;

// inline utility calls

inline float grid_height_at(SLONG mx,SLONG mz)
{
	float	dy;
	PAP_Hi	*pap;
	pap=&PAP_hi[mx][mz];
	dy=(float)(pap->Alt<<3);
//	if (pap->Flags & PAP_FLAG_SINK_SQUARE)
//	{
//		dy -= KERB_HEIGHT;
//	}

	return(dy);
}

inline float grid_height_at_world(float x,float z)
{
	return(grid_height_at( ((SLONG)x)>>8,((SLONG)z)>>8) );
}

ULONG facet_rand(void)
{
	facet_seed=(facet_seed*69069)+1;

	return(facet_seed>>7);
}

void	set_facet_seed(SLONG seed)
{
	facet_seed=seed;
}

//   1        0
//		 b
//	 3  a c   2
#define	GAP_HEIGHT		96.0
#define	GAP_WIDTH_PERC	0.2
#ifdef	UNUSED_WIRE_CUTTERS
void	draw_fence_gap(POLY_Point *quad[4],SLONG page,SLONG along,float sx,float sy,float sz,float dx,float dy,float dz)
{
  	POLY_Point   *pp;
  	POLY_Point   ppb[3];
	float	a_f;
	float	mx,my,mz,du;
	POLY_Point   *g_quad[4];
	POLY_Point   *g_tri[3];

	a_f=(float)(along&0xff);

	a_f*=1.0/256.0;

	if(a_f<GAP_WIDTH_PERC)
		a_f=GAP_WIDTH_PERC;

	if(a_f>1.0-GAP_WIDTH_PERC)
		a_f=1.0-GAP_WIDTH_PERC;

	du=(quad[2]->u-quad[3]->u);  // 1 or -1 ?


	mx=sx+dx*a_f;
	my=sy+dy*a_f;
	mz=sz+dz*a_f;

	dx*=GAP_WIDTH_PERC;
	dy*=GAP_WIDTH_PERC;
	dz*=GAP_WIDTH_PERC;


	pp = &ppb[0];

	POLY_transform(mx-dx,my-dy,mz-dz,pp);
	if (pp->MaybeValid())
	{
		pp->colour=quad[3]->colour;
		pp->specular=quad[3]->specular;
		pp->u=quad[3]->u+du*(a_f-GAP_WIDTH_PERC);
		pp->v=quad[3]->v;
	}
	pp++;

	POLY_transform(mx,my-dy+GAP_HEIGHT,mz,pp);
	if (pp->MaybeValid())
	{
		pp->colour=quad[3]->colour;
		pp->specular=quad[3]->specular;
		pp->u=a_f;
		pp->v=quad[3]->v-(GAP_HEIGHT/512.0);
//		pp->u=mu;
//		pp->v=mv;
	}
	pp++;

	POLY_transform(mx+dx,my-dy,mz+dz,pp);
	if (pp->MaybeValid())
	{
		pp->colour=quad[3]->colour;
		pp->specular=quad[3]->specular;
		pp->u=quad[3]->u+du*(a_f+GAP_WIDTH_PERC);
//		pp->u=a_f+du;//GAP_WIDTH_PERC;
		pp->v=quad[3]->v;
	}
	pp++;

//   1        0
//		 b
//	 3  a c   2

	
	g_quad[0]=quad[0];
	g_quad[1]=&ppb[1];
	g_quad[2]=quad[2];
	g_quad[3]=&ppb[2];

	
	if (POLY_valid_quad(g_quad))
		POLY_add_quad(g_quad, page, 0); // 1 means perform a backface cull

	g_quad[0]=&ppb[1];
	g_quad[1]=quad[1];
	g_quad[2]=&ppb[0];
	g_quad[3]=quad[3];
	if (POLY_valid_quad(g_quad))
		POLY_add_quad(g_quad, page, 0); // 1 means perform a backface cull

	g_tri[0]=quad[0];
	g_tri[1]=quad[1];
	g_tri[2]=&ppb[1];
	if (POLY_valid_triangle(g_tri))
		POLY_add_triangle(g_tri, page, 0); // 1 means perform a backface cull

}
#endif

// texture_quad
//
// set u,v in the poly points and return the page number for this quad

SLONG flip;	// Nasty!

SLONG texture_quad(POLY_Point *quad[4],SLONG texture_style,SLONG pos,SLONG count,SLONG flipx=0)
{
	SLONG	page;
	SLONG	texture_piece;
	SLONG   rand;
	SLONG	random=0;

	rand = facet_rand() & 0x3;

	if(pos==0)
		texture_piece=flipx?TEXTURE_PIECE_LEFT:TEXTURE_PIECE_RIGHT;
	else
	if(pos==count-2)
		texture_piece=flipx?TEXTURE_PIECE_RIGHT:TEXTURE_PIECE_LEFT;
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
		if(rand>1)
			random=1;
	}

	if(texture_style<0)
	{
		SLONG	index;
		struct	DStorey *p_storey;

		flip=0;
		p_storey=&dstoreys[-texture_style];
//		ASSERT(p_storey->Count!=2);

		index=p_storey->Index;
		ASSERT(p_storey->Count);
		
		if(pos<p_storey->Count)
		{
			page=paint_mem[index+pos];
//	ASSERT(page!=156);

//			MSG_add(" pos %d page %x count %d index %d \n",pos,page,p_storey->Count,p_storey->Index); 

			if(page&0x80)
			{
				
				flip=1;
				page&=0x7f;
				if(ControlFlag&&allow_debug_keys)
				{
					quad[0]->colour=0xff0000;
					quad[0]->specular=0xff000000;
					quad[1]->colour=0xff0000;
					quad[1]->specular=0xff000000;
					quad[2]->colour=0xff0000;
					quad[2]->specular=0xff000000;
					quad[3]->colour=0xff0000;
					quad[3]->specular=0xff000000;
					page=POLY_PAGE_COLOUR;
//	ASSERT(page!=156);
				}
			}
			else
			{
				if(ControlFlag&&allow_debug_keys)
				{
					quad[0]->colour=0xff00;
					quad[0]->specular=0xff000000;
					quad[1]->colour=0xff00;
					quad[1]->specular=0xff000000;
					quad[2]->colour=0xff00;
					quad[2]->specular=0xff000000;
					quad[3]->colour=0xff00;
					quad[3]->specular=0xff000000;
					page=POLY_PAGE_COLOUR;
//	ASSERT(page!=156);
				}

			}

			if((page&0x7f)==0)
			{
				texture_style = p_storey->Style;
				if(ControlFlag&&allow_debug_keys)
				{
					quad[0]->colour=0xff;
					quad[0]->specular=0xff000000;
					quad[1]->colour=0xff;
					quad[1]->specular=0xff000000;
					quad[2]->colour=0xff;
					quad[2]->specular=0xff000000;
					quad[3]->colour=0xff;
					quad[3]->specular=0xff000000;
					page=POLY_PAGE_COLOUR;
//	ASSERT(page!=156);
				}

				
			}
		}
		else
		{
			texture_style = p_storey->Style;

		}
	}

	if(texture_style>=0)
	{
		if(texture_style==0)
			texture_style=1;
		page=dx_textures_xy[texture_style][texture_piece].Page;
//	ASSERT(page!=156);
//		ASSERT(page<64*8);
		flip=dx_textures_xy[texture_style][texture_piece].Flip;
		if(ControlFlag&&allow_debug_keys&&random)
		{
			quad[0]->colour=0xffff;
			quad[0]->specular=0xff000000;
			quad[1]->colour=0xffff;
			quad[1]->specular=0xff000000;
			quad[2]->colour=0xffff;
			quad[2]->specular=0xff000000;
			quad[3]->colour=0xffff;
			quad[3]->specular=0xff000000;
			page=POLY_PAGE_COLOUR;
//	ASSERT(page!=156);
		}


	}
/*
	if(flipx)
	{
		flip^=1;
	}
*/
	switch(flip)
	{
		case	0:
			quad[1]->u = 1.0;
			quad[1]->v = 0.0;
			quad[0]->u = 0.0;
			quad[0]->v = 0.0;
			quad[3]->u = 1.0;
			quad[3]->v = 1.0;
			quad[2]->u = 0.0;
			quad[2]->v = 1.0;

			break;
		case	1: //flip x
			quad[1]->u = 0.0;
			quad[1]->v = 0.0;
			quad[0]->u = 1.0;
			quad[0]->v = 0.0;
			quad[3]->u = 0.0;
			quad[3]->v = 1.0;
			quad[2]->u = 1.0;
			quad[2]->v = 1.0;

			break;
		case	2: //flip y
			quad[1]->u = 1.0;
			quad[1]->v = 1.0;
			quad[0]->u = 0.0;
			quad[0]->v = 1.0;
			quad[3]->u = 1.0;
			quad[3]->v = 0.0;
			quad[2]->u = 0.0;
			quad[2]->v = 0.0;

			break;
		case	3: //flip x+y
			quad[1]->u = 0.0;
			quad[1]->v = 1.0;
			quad[0]->u = 1.0;
			quad[0]->v = 1.0;
			quad[3]->u = 0.0;
			quad[3]->v = 0.0;
			quad[2]->u = 1.0;
			quad[2]->v = 0.0;

			break;
	}

		  

//	LogText("quad text page %d \n",page);

//	ASSERT(page!=156);
	return(page);

}


//
// find page and flip
//
SLONG	get_texture_page(SLONG texture_style,SLONG pos,SLONG count,UBYTE *rflip)
{
	SLONG	page;
	SLONG	texture_piece;
	SLONG   rand;
	SLONG	flip;

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

		flip=0;
		p_storey=&dstoreys[-texture_style];
//		ASSERT(p_storey->Count!=2);

		index=p_storey->Index;
		ASSERT(p_storey->Count);
		
		if(pos<p_storey->Count)
		{
			page=paint_mem[index+pos];
//			MSG_add(" pos %d page %x count %d index %d \n",pos,page,p_storey->Count,p_storey->Index); 

			if(page&0x80)
			{
				flip=1;
				page&=0x7f;
			}
			if((page&0x7f)==0)
			{
				texture_style = p_storey->Style;

				
			}
		}
		else
		{
			texture_style = p_storey->Style;

		}
	}

	if(texture_style>=0)
	{
		if(texture_style==0)
			texture_style=1;
		page=dx_textures_xy[texture_style][texture_piece].Page;
		flip=dx_textures_xy[texture_style][texture_piece].Flip;
	}

	*rflip=UBYTE(flip);
	return(page);
}


SLONG	texture_quad2(POLY_Point *quad[4],SLONG texture_style,SLONG texture_piece)
{
	SLONG	page;

	if(texture_style==0)
		texture_style=1;
	page=dx_textures_xy[texture_style][texture_piece].Page;
	switch(dx_textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			quad[1]->u = 1.0;
			quad[1]->v = 0.0;
			quad[0]->u = 0.0;
			quad[0]->v = 0.0;
			quad[3]->u = 1.0;
			quad[3]->v = 1.0;
			quad[2]->u = 0.0;
			quad[2]->v = 1.0;

			break;
		case	1: //flip x
			quad[1]->u = 0.0;
			quad[1]->v = 0.0;
			quad[0]->u = 1.0;
			quad[0]->v = 0.0;
			quad[3]->u = 0.0;
			quad[3]->v = 1.0;
			quad[2]->u = 1.0;
			quad[2]->v = 1.0;

			break;
		case	2: //flip y
			quad[1]->u = 1.0;
			quad[1]->v = 1.0;
			quad[0]->u = 0.0;
			quad[0]->v = 1.0;
			quad[3]->u = 1.0;
			quad[3]->v = 0.0;
			quad[2]->u = 0.0;
			quad[2]->v = 0.0;

			break;
		case	3: //flip x+y
			quad[1]->u = 0.0;
			quad[1]->v = 1.0;
			quad[0]->u = 1.0;
			quad[0]->v = 1.0;
			quad[3]->u = 0.0;
			quad[3]->v = 0.0;
			quad[2]->u = 1.0;
			quad[2]->v = 0.0;

			break;
	}

		  

//	LogText("quad text page %d \n",page);

	return(page);

}
SLONG	texture_tri2(POLY_Point *quad[3],SLONG texture_style,SLONG texture_piece)
{
	SLONG	page;

	if(texture_style==0)
		texture_style=1;
	page=dx_textures_xy[texture_style][texture_piece].Page;
	switch(dx_textures_xy[texture_style][texture_piece].Flip)
	{
		case	0:
			quad[1]->u = 1.0;
			quad[1]->v = 0.0;
			quad[0]->u = 0.0;
			quad[0]->v = 0.0;
			quad[2]->u = 0.0;
			quad[2]->v = 1.0;

			break;
		case	1: //flip x
			quad[1]->u = 0.0;
			quad[1]->v = 0.0;
			quad[0]->u = 1.0;
			quad[0]->v = 0.0;
			quad[2]->u = 1.0;
			quad[2]->v = 1.0;

			break;
		case	2: //flip y
			quad[1]->u = 1.0;
			quad[1]->v = 1.0;
			quad[0]->u = 0.0;
			quad[0]->v = 1.0;
			quad[2]->u = 0.0;
			quad[2]->v = 0.0;

			break;
		case	3: //flip x+y
			quad[1]->u = 0.0;
			quad[1]->v = 1.0;
			quad[0]->u = 1.0;
			quad[0]->v = 1.0;
			quad[2]->u = 1.0;
			quad[2]->v = 0.0;

			break;
	}

		  

//	LogText("quad text page %d \n",page);

	return(page);

}

//
// should work for any angle
//
void	build_fence_poles(float sx,float sy,float sz,float fdx,float fdz,SLONG count,float *rdx,float *rdz,SLONG style)
{
	float	x[13],y[13],z[13];
	float	dx,dz,nx,nz,dist;
	float	gx,gy,gz;
	POLY_Point   *quad[4];
	POLY_Point   *tri[3];
  	POLY_Point   *pp;

	float	dy;

	NIGHT_Colour col;

	col.red=64;
	col.green=64;
	col.blue=64;


	x[0]=0;
	y[0]=0;
	z[0]=0;

#ifdef TARGET_DC
	dist=(fdx*fdx+fdz*fdz);
	if(dist==0.0f)
		return;

	dist = _InvSqrtA(dist);

	dx=(fdx)*dist;
	dz=(fdz)*dist;
#else
	dist=sqrt(fdx*fdx+fdz*fdz);
	if(dist==0.0f)
		return;

	dx=(fdx)/dist;
	dz=(fdz)/dist;
#endif

	*rdx=dx;
	*rdz=dz;

	nx=(dz*10.0f);// /1024;
	nz=-(dx*10.0f); ///1204;

	x[1]=dx*20.0f;
	y[1]=0.0f;
	z[1]=dz*20.0f;

	x[2]=dx*10.0f+nx;
	y[2]=0.0f;
	z[2]=dz*10.0f+nz;


	gx=sx;
	gy=sy;
	gz=sz;

	while(count-->0)
	{
		SLONG	c0;
		
		POLY_buffer_upto = 0;
		pp = &POLY_buffer[0];

		dy=	grid_height_at_world(gx,gz);

		//
		// 3 points at base of pilla
		//
		for(c0=0;c0<3;c0++)
		{
			POLY_transform(gx+x[c0],gy+y[c0]+dy,gz+z[c0],pp);

			if (pp->MaybeValid())
			{
				NIGHT_get_d3d_colour(
								col,
								&pp->colour,
								&pp->specular);

				//POLY_fadeout_point(pp);
			}
			pp++;
		}

		y[2]=-10.0;
		// 3 points at middle
		for(c0=0;c0<3;c0++)
		{
			POLY_transform(gx+x[c0],gy+y[c0]+dy+200.0f,gz+z[c0],pp);

			if (pp->MaybeValid())
			{
				NIGHT_get_d3d_colour(
								col,
								&pp->colour,
								&pp->specular);

				//POLY_fadeout_point(pp);
			}
			pp++;
		}
		y[2]=0.0;

		// point at top
		POLY_transform(gx+x[2]+nx*5.0f,gy+y[2]+dy+250.0f,gz+z[2]+nz*5.0f,pp);
		if (pp->MaybeValid())
		{
			NIGHT_get_d3d_colour(
							col,
							&pp->colour,
							&pp->specular);

			//POLY_fadeout_point(pp);
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

				if (POLY_valid_quad(quad))
				{
					SLONG	page;
					
					//
					// Texture the quad.
					// 

					page=texture_quad2(quad,dstyles[style],TEXTURE_PIECE_MIDDLE2);

					POLY_add_quad(quad, page, 1); // 1 means perform a backface cull
				}

			}



			tri[2] = &POLY_buffer[6];
			for(t=0;t<3;t++)
			{
				
				tri[1] = &POLY_buffer[3+t];
				tri[0] = &POLY_buffer[3+q_lookup[t]];

				if (POLY_valid_triangle(tri))
				{
					SLONG	page=0;
					
					//
					// Texture the quad.
					// 

					page=texture_tri2(tri,dstyles[style],TEXTURE_PIECE_MIDDLE2); //TOP_LEFT);

					POLY_add_triangle(tri, page, 1); // 1 means perform a backface cull
				}

			}
			

		}

		gx+=fdx;
		gz+=fdz;

		// end while
	}
}



void cable_draw(struct DFacet *p_facet)
{
	static const float	width = 4.0F * 65536.0F /3.0F;	// width of cable

	float	x1,y1,z1;			// start of cable
	float	x2,y2,z2;			// end of cable
	float	dx,dy,dz;			// section vector
	SLONG	count;				// number of sections
	float	cx,cy,cz;			// current point on cable

	SLONG	angle;				// current eccentricity angle
	SLONG	dangle1;			// delta angle #1
	SLONG	dangle2;			// delta angle #2
	SLONG	sag;				// sagginess


	POLY_flush_local_rot();

	//
	// setup
	//

	x1 = p_facet->x[0] * 256;
	y1 = p_facet->Y[0];
	z1 = p_facet->z[0] * 256;

	x2 = p_facet->x[1] * 256;
	y2 = p_facet->Y[1];
	z2 = p_facet->z[1] * 256;

	count = p_facet->Height;

	dx = (x2 - x1) / count;
	dy = (y2 - y1) / count;
	dz = (z2 - z1) / count;

	cx = x1;
	cy = y1;
	cz = z1;

	angle = -512;
	dangle1 = (SWORD)p_facet->StyleIndex;
	dangle2 = (SWORD)p_facet->Building;
	sag = p_facet->FHeight * 64;

	//
	// generate points & transform them
	//

	POLY_Point*	pp;				// point ptr
	POLY_Point	qp[4];			// quad points
	POLY_Point*	pqp[4];			// ptrs to quad points

	SLONG	ii;

	POLY_buffer_upto = 0;
	pp = &POLY_buffer[0];

	float	u = 0.0F;
	float	du = 1.0F / count;

	for (ii = 0; ii <= count; ii++)
	{
		float sagy = float((COS((angle+2048)&2047)*sag) >> 16);

		POLY_transform(cx, cy - sagy, cz, pp);
		pp->u = u;
		pp->v = 0.0;
		NIGHT_get_d3d_colour(
			NIGHT_get_light_at(cx, cy - sagy, cz),
			&pp->colour,
			&pp->specular);
		//POLY_fadeout_point(pp);

		pp++;

		cx += dx;
		cy += dy;
		cz += dz;

		angle += dangle1;
		if (angle >= -30)	dangle1 = dangle2;

		u += du;
	}

	//
	// expand points to quads
	//

	pqp[0] = &qp[3];
	pqp[1] = &qp[1];
	pqp[2] = &qp[2];
	pqp[3] = &qp[0];

	pp = &POLY_buffer[0];

	POLY_Point	old[2];
	bool		old_set = false;

	for (ii = 0; ii < count; ii++)
	{
		if (pp[0].IsValid() && pp[1].IsValid())
		{
			POLY_create_cylinder_points(&pp[0], &pp[1], width, &qp[0]);

			// set V coords for envmap
			qp[0].v = 0.0F;
			qp[1].v = 1.0F;
			qp[2].v = 0.0F;
			qp[3].v = 1.0F;

			if (old_set)
			{
				//
				// force coordinates to match up with last quad
				//

				qp[0].X = old[0].X;
				qp[0].Y = old[0].Y;
				qp[1].X = old[1].X;
				qp[1].Y = old[1].Y;
			}

			if (POLY_valid_quad(pqp))
			{
#ifndef TARGET_DC
				extern UBYTE sw_hack;

				if (sw_hack)
				{
					qp[0].colour = pp[0].colour;
					qp[1].colour = pp[0].colour;
					qp[2].colour = pp[1].colour;
					qp[3].colour = pp[1].colour;
				}
				else
#endif
				{
					qp[0].colour = 0;
					qp[1].colour = 0;
					qp[2].colour = 0;
					qp[3].colour = 0;
				}

				POLY_add_quad(pqp, POLY_PAGE_COLOUR, false, true);

				qp[0].colour = pp[0].colour | 0xFF000000;
				qp[1].colour = pp[0].colour | 0xFF000000;
				qp[2].colour = pp[1].colour | 0xFF000000;
				qp[3].colour = pp[1].colour | 0xFF000000;
				POLY_add_quad(pqp, POLY_PAGE_ENVMAP, false, true);
			}

			//
			// save end coordinates for next quad
			//

			old[0].X = qp[2].X;
			old[0].Y = qp[2].Y;
			old[1].X = qp[3].X;
			old[1].Y = qp[3].Y;
			old_set = true;
		}
		else
		{
			old_set = false;
		}
		pp++;
	}
}


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

void	draw_insides(SLONG indoor_index,SLONG room,UBYTE fade)
{
	struct	InsideStorey	*p_inside;
	SLONG	c0;
	static	int recursive=0;
	SLONG	stair;
	ASSERT(recursive==0);

//	fade=128;

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

//
//	   1	   2
//		  6 5
//
//
//     0  7 4  3


UBYTE	door_poly[][4]=
{
	{0,1,7,6},
	{1,2,6,5},
	{2,3,5,4}
};

void	DRAW_door(float sx,float sy,float sz,float fx,float fy,float fz,float block_height,SLONG count,ULONG fade_alpha,SLONG style,SLONG flipx=0)
{

	float	dy,dx,dz;
	SLONG	start_index;
	SLONG	page;
	UBYTE	flip;

  	POLY_Point   *pp,*ps;
	POLY_Point   *quad[4];
	SLONG	c0,face;

	start_index=POLY_buffer_upto;
	dy=block_height*0.7f;

	dx=fx*0.3f;
	dz=fz*0.3f;
	pp = &POLY_buffer[POLY_buffer_upto];
	ps=pp;

	POLY_transform(sx,sy,sz,pp++);
	POLY_transform(sx,sy+block_height,sz,pp++);
	POLY_transform(sx+fx,sy+block_height,sz+fz,pp++);
	POLY_transform(sx+fx,sy,sz+fz,pp++);
	POLY_transform(sx+fx-dx,sy,sz+fz-dz,pp++);
	POLY_transform(sx+fx-dx,sy+dy,sz+fz-dz,pp++);
	POLY_transform(sx+dx,sy+dy,sz+dz,pp++);
	POLY_transform(sx+dx,sy,sz+dz,pp++);

	POLY_buffer_upto+=8;


	for(c0=0;c0<8;c0++)
	{
		ps[c0].colour=0xffffff|fade_alpha;
		ps[c0].specular=0xff000000;

	}

SLONG	get_texture_page(SLONG texture_style,SLONG pos,SLONG count,UBYTE *rflip);
	page=get_texture_page(style,0,1,&flip);

	ps[0].u=0.0f;
	ps[0].v=1.0f-0.0f;

	ps[1].u=0.0f;
	ps[1].v=1.0f-1.0f;

	ps[2].u=1.0f;
	ps[2].v=1.0f-1.0f;

	ps[3].u=1.0f;
	ps[3].v=1.0f-0.0f;

	ps[4].u=0.7f;
	ps[4].v=1.0f-0.0f;

	ps[5].u=0.7f;
	ps[5].v=1.0f-0.7f;

	ps[6].u=0.3f;
	ps[6].v=1.0f-0.7f;

	ps[7].u=0.3f;
	ps[7].v=1.0f-0.0f;


	if(flipx)
	for(c0=0;c0<8;c0++)
	{
		ps[c0].u=1.0f-ps[c0].u;

	}

	for(face=0;face<3;face++)
	{
		for(c0=0;c0<4;c0++)
		{
			quad[c0] = &ps[door_poly[face][c0]];;
		}
		if (POLY_valid_quad(quad))
		{
			POLY_add_quad(quad, page, FALSE); // TRUE means perform a backface cull
		}
	}
}

//
// draw the top of an inside wall
//

void draw_wall_thickness(struct DFacet *p_facet,ULONG fade_alpha)
{
  	POLY_Point*	pp;
	POLY_Point*	quad[4];
	float		x1,z1;
	float		x2,z2;
	float		y;
	float		dx,dz;

	static const ULONG	colour = 0xFF8844;

	// get coordinates & vector along the facet

	y = float(p_facet->Y[0] + 256);

	x1 = float(p_facet->x[0] << 8);
	z1 = float(p_facet->z[0] << 8);

	x2 = float(p_facet->x[1] << 8);
	z2 = float(p_facet->z[1] << 8);

	dx = x2 - x1;
	dz = z2 - z1;

	// generate dx,dy for segment (premultiplied by half the wall thickness)

	if (dz == 0)
	{
		dx = (dx < 0) ? -10 : +10;
	}
	else if (dx == 0)
	{
		dz = (dz < 0) ? -10 : +10;
	}
	else
	{
		float scalar = 10.0f / (float)sqrt(dx*dx+dz*dz);

		dx *= scalar;
		dz *= scalar;
	}

	// now, "along" vector is (dx,dz) and "perp" vector is (dz,-dx)

	pp = &POLY_buffer[POLY_buffer_upto++];

	// pt 1: p1, along -1, perp +1
	POLY_transform(x1-dx+dz, y, z1-dz-dx, pp);
	pp->colour = colour | fade_alpha;
	pp->specular = 0xff000000;

	quad[0] = pp++;

	// pt2: p1, along -1, perp -1
	POLY_transform(x1-dx-dz, y, z1-dz+dx, pp);
	pp->colour = colour | fade_alpha;
	pp->specular = 0xff000000;

	quad[1] = pp++;

	// pt3: p2, along +1, perp +1
	POLY_transform(x2+dx+dz, y, z2+dz-dx, pp);
	pp->colour = colour | fade_alpha;
	pp->specular = 0xff000000;

	quad[2] = pp++;

	// pt4: p2, along +1, perp -1
	POLY_transform(x2+dx-dz, y, z2+dz+dx, pp);
	pp->colour = colour | fade_alpha;
	pp->specular = 0xff000000;

	quad[3] = pp;

	// draw the quad

	if (POLY_valid_quad(quad))
	{
		POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE); // TRUE means perform a backface cull
	}
}

void FACET_barbedwire_top(struct DFacet *p_facet)
{
	float dx=(p_facet->x[1]-p_facet->x[0] << 8);
	float dy=(p_facet->Y[1]-p_facet->Y[0]);
	float dz=(p_facet->z[1]-p_facet->z[0] << 8);
	float mag=sqrt((dx*dx)+(dz*dz));
	float stepx=(dx/mag)*10;
	float stepy=(dy/mag)*10;
	float stepz=(dz/mag)*10;
	SLONG cx=p_facet->x[0] << 8;
	SLONG cy=p_facet->Y[0];
	SLONG cz=p_facet->z[0] << 8;
	SLONG seed=54321678;
	float base=0;
	SLONG contour = 0;
	float block_height=256.0;
	float height;
	ULONG	normal=0;

	POLY_set_local_rotation_none();

	if(p_facet->FacetType==STOREY_TYPE_NORMAL)
	{
		block_height=p_facet->BlockHeight<<4;
		height=(p_facet->Height*block_height)*0.25f;
		normal=1;
		contour=90;
	}
	else
	{
		height=(64*p_facet->Height);
	}


	while (base<mag) {

		//...
		//sprite it as a test (do it proper later)
		//

		seed*=31415965;
		seed+=123456789;


		if(!normal)
		{

			if (!(p_facet->FacetFlags & FACET_FLAG_ONBUILDING))
			{
				//contour = PAP_calc_map_height_at(cx,cz);
				contour = PAP_calc_height_noroads(cx,cz);
			}
			else
			{
				contour = p_facet->Y[0];
			}
		}

		SPRITE_draw_tex(
			cx,
			height-64+((seed>>8)&0xf) + contour,
			cz,
			50,
			0xffffff,
			0,
			POLY_PAGE_BARBWIRE,
			0.0, 0.0, 1.0, 1.0,
			SPRITE_SORT_NORMAL);


			base+=10;
			cx+=stepx;
			cy+=stepy;
			cz+=stepz;
	}

}

extern UBYTE AENG_transparent_warehouses;

SWORD FacetRows[100];
float FacetDiffY[128];

static void MakeFacetPoints(float sx, float sy, float sz, float dx, float dz, float block_height, 
							SLONG height, SLONG max_height, NIGHT_Colour* col, SLONG foundation, SLONG count, SLONG invisible,SLONG hug)
{	
	SLONG	hf = 0;

	ASSERT(POLY_buffer_upto == 0);	// or else FacetDiffY is accessed wrongly

	while (height >= 0)
	{
		float	x = sx;
		float	y = sy;
		float	z = sz;

		FacetRows[hf] = POLY_buffer_upto;

		for (SLONG c0 = count; c0 > 0; c0--)
		{
			float	ty;

			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			POLY_Point*	pp = &POLY_buffer[POLY_buffer_upto++];

			if(hug)
			{
				
				ty = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);
				ty+=y;

			}
			else
			if (foundation != 2)
			{
				ty=y;
			}
			else
			{
				ty = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);

				FacetDiffY[POLY_buffer_upto - 1] = ( ( y - ty ) * ( 1.0f / 256.0f ) ) + 1.0f;
			}

			POLY_transform_c_saturate_z(x,ty,z, pp);

			if (pp->MaybeValid())
			{
				if (INDOORS_INDEX)
				{
					NIGHT_get_d3d_colour_dim(*col, &pp->colour, &pp->specular);
				}
				else
				{
					NIGHT_get_d3d_colour(*col, &pp->colour, &pp->specular);
				}

				//apply_cloud(SLONG(x), SLONG(ty), SLONG(z), &pp->colour);

				//POLY_fadeout_point(pp);

#if defined(FACET_REMOVAL_TEST) || defined(_DEBUG)

				// colour invisible facets red
				if (invisible)
				{
					pp->colour = 0xFF0000;
					pp->specular = 0xFF000000;
				}
#endif
			}

#if 0
			if (((pp->clip & POLY_CLIP_LEFT) && (pp->clip & POLY_CLIP_TRANSFORMED) && (c0 < count) && (pp[-1].clip & POLY_CLIP_TRANSFORMED) && (pp->X < pp[-1].X)) ||	// off left and going left
				((pp->clip & POLY_CLIP_RIGHT) && (pp->clip & POLY_CLIP_TRANSFORMED) && (c0 < count) && (pp[-1].clip & POLY_CLIP_TRANSFORMED) && (pp->X > pp[-1].X)))		// off right and going right
			{
				col += c0;
				break;
			}
#endif

			x += dx;
			z += dz;
			col++;
		}
		sy += block_height;
		height -= 4;
		hf++;
		foundation--;
		if (sy > max_height) break;
	}
	FacetRows[hf] = POLY_buffer_upto;

}


static void MakeFacetPointsFence(float sx, float sy, float sz, float dx, float dz, float block_height, 
							SLONG height, SLONG max_height, NIGHT_Colour* col, SLONG foundation, SLONG count, SLONG invisible,float *diff_y)
{	
	SLONG	hf = 0;
	float	*p_diffy;



	ASSERT(POLY_buffer_upto == 0);	// or else FacetDiffY is accessed wrongly

	while (height >= 0)
	{
		float	x = sx;
		float	y = sy;
		float	z = sz;
		p_diffy=diff_y;
		FacetRows[hf] = POLY_buffer_upto;

		for (SLONG c0 = count; c0 > 0; c0--)
		{
			float	ty;

			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			POLY_Point*	pp = &POLY_buffer[POLY_buffer_upto++];

			{
				ty=y+*p_diffy;
				p_diffy++;
				POLY_transform_c_saturate_z(x,ty,z, pp);
			}

			if (pp->MaybeValid())
			{
				{
					NIGHT_get_d3d_colour(*col, &pp->colour, &pp->specular);
				}

				{
				//	apply_cloud(SLONG(x), SLONG(ty), SLONG(z), &pp->colour);
				}

				//POLY_fadeout_point(pp);

#if defined(FACET_REMOVAL_TEST) || defined(_DEBUG)

				// colour invisible facets red
				if (invisible)
				{
					pp->colour = 0xFF0000;
					pp->specular = 0xFF000000;
				}
#endif
			}


			x += dx;
			z += dz;
			col++;
		}
		sy += block_height;
		height -= 4;
		hf++;
		foundation--;
		if (sy > max_height) break;
	}
	FacetRows[hf] = POLY_buffer_upto;
}

static void FillFacetPoints(SLONG count, ULONG base_row, SLONG foundation, SLONG facet_backwards, SLONG style_index, float block_height,SLONG reverse_texture)
{
	POLY_Point*		quad[4];

	SLONG	row1 = FacetRows[base_row];
	SLONG	row2 = FacetRows[base_row+1];

	for (SLONG c0 = 0; c0 < row2 - row1 - 1; c0++)
	{
		if (facet_backwards)
		{
			quad[0] = &POLY_buffer[row2+c0];
			quad[1] = &POLY_buffer[row2+c0+1];
			quad[2] = &POLY_buffer[row1+c0];
			quad[3] = &POLY_buffer[row1+c0+1];
		}
		else
		{
			quad[0] = &POLY_buffer[row2+c0+1];
			quad[1] = &POLY_buffer[row2+c0];
			quad[2] = &POLY_buffer[row1+c0+1];
			quad[3] = &POLY_buffer[row1+c0];
		}

		if (POLY_valid_quad(quad))
		{
			SLONG page;
			if(reverse_texture)
				page = texture_quad(quad, dstyles[style_index], count-2-c0, count,1);
			else
				page = texture_quad(quad, dstyles[style_index], c0, count);

			if(block_height!=256.0f)
			{
				if(block_height==192.0f)
				{
					quad[2]->v=0.75f;
					quad[3]->v=0.75f;
				}
				else if(block_height==128.0f)
				{
					quad[2]->v=0.5f;
					quad[3]->v=0.5f;
				}
				else if(block_height==64.0f)
				{
					quad[2]->v=0.25f;
					quad[3]->v=0.25f;
				}
			}

			if (foundation == 2)
			{
				quad[3]->v = FacetDiffY[c0];
				quad[2]->v = FacetDiffY[c0+1];
				
				// This doesn't work with texture paging.
				//POLY_Page[page].RS.WrapJustOnce();

				// So we panic and clamp to 1.0. Pants stretchy effect, but better than
				// going on to the next bit of the page.
				if ( quad[3]->v > 1.0f )
				{
					quad[3]->v = 1.0f;
				}
				if ( quad[2]->v > 1.0f )
				{
					quad[2]->v = 1.0f;
				}

			}

			//
			// Add crinkle?
			//
extern int AENG_detail_crinkles;
			if (AENG_detail_crinkles)
			{
				if (page < 64 * 8)
				{
					if (TEXTURE_crinkle[page])
					{
						//
						// This quad could be crinkled!
						//

						if (quad[0]->z > 0.6F)
						{
							//
							// Too far away to be crinkled.
							//
						}
						else
						if (quad[0]->z < 0.3F)
						{
							//
							// Maximum crinkleyness!
							//

							CRINKLE_do(
								TEXTURE_crinkle[page],
								page,
								1.0F,
								quad,
								flip);

							goto added_crinkle;
						}
						else
						{
							float extrude;
							float av_z;

							//
							// Intermediate crinkle extrusion.
							//

							// av_z  = quad[0]->z + quad[1]->z + quad[2]->z + quad[3]->z;
							// av_z *= 0.25F;

							av_z  = quad[0]->z;

							extrude  = av_z - 0.5F;
							extrude *= 1.0F / (0.4F - 0.5F);

							if (extrude > 0.0F)
							{
								if (extrude > 1.0F)
								{
									extrude = 1.0F;
								}

								CRINKLE_do(
									TEXTURE_crinkle[page],
									page,
									extrude,
									quad,
									flip);

								goto added_crinkle;
							}
						}
					}
				}
			}

			POLY_add_quad(quad, page, TRUE); // TRUE means perform a backface cull

		  added_crinkle:;
		}
		else
		{
			//
			// Even though we don't draw the quad, we must
			// push on the random number generator.
			//

			facet_rand();
		}
	}
	for (;c0 < count; c0++)
	{
		facet_rand();
	}
}






// Like MakeFacetPoints, but used in the most common cases.
inline void MakeFacetPointsCommon(float sx, float sy, float sz, float dx, float dz, float block_height, 
							SLONG height, NIGHT_Colour* col, SLONG foundation, SLONG count, SLONG hug)
{	
	SLONG	hf = 0;




	LOG_ENTER ( Facet_MakeFacetPoints )

	ASSERT(POLY_buffer_upto == 0);	// or else FacetDiffY is accessed wrongly

	while (height >= 0)
	{
		float	x = sx;
		float	y = sy;
		float	z = sz;

		FacetRows[hf] = POLY_buffer_upto;

		for (SLONG c0 = count; c0 > 0; c0--)
		{
			float	ty;

			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			POLY_Point*	pp = &POLY_buffer[POLY_buffer_upto++];

			if(hug)
			{
				
				ty = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);
				ty+=y;

			}
			else
			if (foundation != 2)
			{
				ty=y;
			}
			else
			{
				ty = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);

				FacetDiffY[POLY_buffer_upto - 1] = ( ( y - ty ) * ( 1.0f / 256.0f ) ) + 1.0f;
			}

			POLY_transform_c_saturate_z(x,ty,z, pp);

			if (pp->MaybeValid())
			{
				if (INDOORS_INDEX)
				{
					NIGHT_get_d3d_colour_dim(*col, &pp->colour, &pp->specular);
				}
				else
				{
					NIGHT_get_d3d_colour(*col, &pp->colour, &pp->specular);
				}

				//apply_cloud(SLONG(x), SLONG(ty), SLONG(z), &pp->colour);

				//POLY_fadeout_point(pp);

			}

			x += dx;
			z += dz;
			col++;
		}
		sy += block_height;
		height -= 4;
		hf++;
		foundation--;
	}
	FacetRows[hf] = POLY_buffer_upto;

	LOG_EXIT ( Facet_MakeFacetPoints )

}




// Like FillFacetPoints, but used in the most common cases.
inline void FillFacetPointsCommon(SLONG count, ULONG base_row, SLONG foundation, SLONG style_index, float block_height )
{
	POLY_Point*		quad[4];

	LOG_ENTER ( Facet_FillFacetPoints )

	SLONG	row1 = FacetRows[base_row];
	SLONG	row2 = FacetRows[base_row+1];

	#ifdef FACETINFO
	FACET_Facetinfo *ff;
	#endif

	for (SLONG c0 = 0; c0 < row2 - row1 - 1; c0++)
	{
		{
			quad[0] = &POLY_buffer[row2+c0+1];
			quad[1] = &POLY_buffer[row2+c0];
			quad[2] = &POLY_buffer[row1+c0+1];
			quad[3] = &POLY_buffer[row1+c0];
		}

		if (POLY_valid_quad(quad))
		{
			SLONG page;
			page = texture_quad(quad, dstyles[style_index], c0, count);

			#ifdef FACETINFO

			if (FACET_facetinfo[FACET_facetinfo_current].done)
			{
				//
				// Already got facet info for this facet.
				//
			}
			else
			{
				ASSERT(WITHIN(FACET_facetinfo_current, 0, FACET_MAX_FACETINFO - 1));

				ff = &FACET_facetinfo[FACET_facetinfo_current];

				SLONG i;

				for (i = 0; i < ff->num_pages; i++)
				{
					if (ff->page[i].page == page)
					{
						ff->page[i].quads += 1;

						goto found_page1;
					}
				}

				ASSERT(WITHIN(ff->num_pages, 0, FACET_MAX_PAGEINFO_PER_FACET - 1));

				ff->page[ff->num_pages].page  = page;
				ff->page[ff->num_pages].quads = 1;

				ff->num_pages += 1;

			  found_page1:;
			}

			#endif

			if(block_height!=256.0f)
			{
				float fTemp = (float)block_height * ( 1.0f / 256.0f );
				quad[2]->v = fTemp;
				quad[3]->v = fTemp;
				/*
				if(block_height==192.0f)
				{
					quad[2]->v=0.75f;
					quad[3]->v=0.75f;
				}
				else if(block_height==128.0f)
				{
					quad[2]->v=0.5f;
					quad[3]->v=0.5f;
				}
				else if(block_height==64.0f)
				{
					quad[2]->v=0.25f;
					quad[3]->v=0.25f;
				}
				*/
			}

			if (foundation == 2)
			{
				quad[3]->v = FacetDiffY[c0];
				quad[2]->v = FacetDiffY[c0+1];

				POLY_Page[page].RS.WrapJustOnce();
			}

			//
			// Add crinkle?
			//
extern int AENG_detail_crinkles;
			if (AENG_detail_crinkles && (GAME_TURN & 0x20))
			{
				if (page < 64 * 8)
				{
					if (TEXTURE_crinkle[page])
					{
						//
						// This quad could be crinkled!
						//

						if (quad[0]->z > 0.6F)
						{
							//
							// Too far away to be crinkled.
							//
						}
						else
						if (quad[0]->z < 0.3F)
						{
							//
							// Maximum crinkleyness!
							//

							CRINKLE_do(
								TEXTURE_crinkle[page],
								page,
								1.0F,
								quad,
								flip);

							goto added_crinkle_common;
						}
						else
						{
							float extrude;
							float av_z;

							//
							// Intermediate crinkle extrusion.
							//

							// av_z  = quad[0]->z + quad[1]->z + quad[2]->z + quad[3]->z;
							// av_z *= 0.25F;

							av_z  = quad[0]->z;

							extrude  = av_z - 0.5F;
							extrude *= 1.0F / (0.4F - 0.5F);

							if (extrude > 0.0F)
							{
								if (extrude > 1.0F)
								{
									extrude = 1.0F;
								}

								CRINKLE_do(
									TEXTURE_crinkle[page],
									page,
									extrude,
									quad,
									flip);

								goto added_crinkle_common;
							}
						}
					}
				}
			}
			else
			{
				if (page < 512 && SUPERCRINKLE_IS_CRINKLED(page))
				{
					float world_x;
					float world_y;
					float world_z;

					world_x = quad[3]->x;
					world_y = quad[3]->y;
					world_z = quad[3]->z;

					extern float AENG_cam_matrix[9];
					extern float POLY_cam_over_view_dist;
					extern float POLY_cam_aspect;
					extern float POLY_cam_lens;

					MATRIX_MUL_BY_TRANSPOSE(
						AENG_cam_matrix,
						world_x,
						world_y,
						world_z);

					world_x /= POLY_cam_over_view_dist;
					world_y /= POLY_cam_over_view_dist;
					world_z /= POLY_cam_over_view_dist;

					world_x /= POLY_cam_aspect;

					world_x /= POLY_cam_lens;
					world_y /= POLY_cam_lens;

					world_x += POLY_cam_x;
					world_y += POLY_cam_y;
					world_z += POLY_cam_z;

					POLY_set_local_rotation(
						world_x,
						world_y,
						world_z,
						FACET_direction_matrix);

					ULONG colour  [4] = {quad[0]->colour,   quad[1]->colour,   quad[2]->colour,   quad[3]->colour  };
					ULONG specular[4] = {quad[0]->specular, quad[1]->specular, quad[2]->specular, quad[3]->specular};

					if (SUPERCRINKLE_draw(page, colour, specular))
					{
						goto added_crinkle_common;
					}
				}
			}



			// I don't think we need to backface cull - should already have been done for the whole facet.
			//POLY_add_quad(quad, page, TRUE); // TRUE means perform a backface cull
			POLY_add_quad(quad, page, FALSE);

added_crinkle_common:;
		}
		else
		{
			//
			// Even though we don't draw the quad, we must
			// push on the random number generator.
			//

			#ifdef FACETINFO

			SLONG page;
			page = texture_quad(quad, dstyles[style_index], c0, count);

			if (FACET_facetinfo[FACET_facetinfo_current].done)
			{
				//
				// Already got facet info for this facet.
				//
			}
			else
			{
				ASSERT(WITHIN(FACET_facetinfo_current, 0, FACET_MAX_FACETINFO - 1));

				ff = &FACET_facetinfo[FACET_facetinfo_current];

				SLONG i;

				for (i = 0; i < ff->num_pages; i++)
				{
					if (ff->page[i].page == page)
					{
						ff->page[i].quads += 1;

						goto found_page2;
					}
				}

				ASSERT(WITHIN(ff->num_pages, 0, FACET_MAX_PAGEINFO_PER_FACET - 1));

				ff->page[ff->num_pages].page  = page;
				ff->page[ff->num_pages].quads = 1;

				ff->num_pages += 1;

			  found_page2:;
			}

			#else

			facet_rand();

			#endif
		}
	}
	for (;c0 < count; c0++)
	{
		#ifdef FACETINFO

		SLONG page;
		page = texture_quad(quad, dstyles[style_index], c0, count);

		if (FACET_facetinfo[FACET_facetinfo_current].done)
		{
			//
			// Already got facet info for this facet.
			//
		}
		else
		{
			ASSERT(WITHIN(FACET_facetinfo_current, 0, FACET_MAX_FACETINFO - 1));

			ff = &FACET_facetinfo[FACET_facetinfo_current];

			SLONG i;

			for (i = 0; i < ff->num_pages; i++)
			{
				if (ff->page[i].page == page)
				{
					ff->page[i].quads += 1;

					goto found_page3;
				}
			}

			ASSERT(WITHIN(ff->num_pages, 0, FACET_MAX_PAGEINFO_PER_FACET - 1));

			ff->page[ff->num_pages].page  = page;
			ff->page[ff->num_pages].quads = 1;

			ff->num_pages += 1;

		  found_page3:;
		}

		#else

		facet_rand();

		#endif
	}

	LOG_EXIT ( Facet_FillFacetPoints )
}








extern	UWORD	fade_black;

void FACET_draw_quick(SLONG facet,UBYTE alpha)
{
  	POLY_Point   *pp;
	POLY_Point   *quad[4];
	float	fx1,fx2,fz1,fz2,fy1,fy2,height;
	ULONG	col=0xff000000;
	struct		  DFacet	*p_facet;


	p_facet=&dfacets[facet];
	if(allow_debug_keys)
	if (Keys[KB_P1])
	{
		col = 0xFF200000;
	}
	
	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_FENCE:
		case	STOREY_TYPE_FENCE_FLAT:
		case	STOREY_TYPE_NORMAL:

			fx1=p_facet->x[0]<<8;
			fx2=p_facet->x[1]<<8;
			fz1=p_facet->z[0]<<8;
			fz2=p_facet->z[1]<<8;

			fy1=p_facet->Y[0];
			fy2=fy1+p_facet->Height*p_facet->BlockHeight*4;


			POLY_buffer_upto=0;
			pp = &POLY_buffer[POLY_buffer_upto];

			POLY_transform(fx2,fy2,fz2,pp);
			if (pp->clip & POLY_CLIP_NEAR)	return;
			if (pp->MaybeValid())
			{
				pp->colour=col;
				//pp->colour = 0xff000000;
				pp->specular=0;
				pp->z=0.99999f;
				pp->Z=0.00001f;
				pp->u=0.5f;
				pp->v=0.5f;
			}
			pp++;
			POLY_transform(fx1,fy2,fz1,pp);
			if (pp->clip & POLY_CLIP_NEAR)	return;
			if (pp->MaybeValid())
			{
				pp->colour=col;
				//pp->colour = 0x00ff0000;
				pp->specular=0;
				pp->z=0.99999f;
				pp->Z=0.00001f;
				pp->u=0.5f;
				pp->v=0.5f;
			}
			pp++;
			POLY_transform(fx2,fy1,fz2,pp);
			if (pp->clip & POLY_CLIP_NEAR)	return;
			if (pp->MaybeValid())
			{
				pp->colour=col;
				//pp->colour = 0xff00;
				pp->specular=0;
				pp->z=0.99999f;
				pp->Z=0.00001f;
				pp->u=0.5f;
				pp->v=0.5f;
			}
			pp++;
			POLY_transform(fx1,fy1,fz1,pp);
			if (pp->clip & POLY_CLIP_NEAR)	return;
			if (pp->MaybeValid())
			{
				pp->colour=col;
				//pp->colour = 0xff;
				pp->specular=0;
				pp->z=0.99999f;
				pp->Z=0.00001f;
				pp->u=0.5f;
				pp->v=0.5f;
			}
			pp++;

			quad[0]=&POLY_buffer[0];
			quad[1]=&POLY_buffer[1];
			quad[2]=&POLY_buffer[2];
			quad[3]=&POLY_buffer[3];

			// add with no back-facing culling and no clip flag generation
			// clip flags should be correct already; back-face culling ruins
			// the render (since tops of buildings aren't plotted)
			if (POLY_valid_quad(quad))
				POLY_add_quad(quad, POLY_PAGE_COLOUR_WITH_FOG, 0,0);
		break;
	}

}




// Handles all sorts of facets - called by FACET_draw in those cases.
void FACET_draw_rare(SLONG facet,UBYTE alpha)
{
	struct		  DFacet	*p_facet;
	static SWORD  rows[100];
	SLONG		  c0,count;
	SLONG		  dx,dz;
	float		  x,y,z,sx,sy,sz,fdx,fdz;
	SLONG		  height;
  	POLY_Point   *pp;
	SLONG         hf;
	POLY_Point   *quad[4];
	SLONG		  style_index;
	NIGHT_Colour *col;
	SLONG         max_height;
	SLONG		foundation=0;
	static float	diff_y[128];
	float		block_height=256.0;

	SLONG		diag=0;
	SLONG		facet_backwards=0;
	ULONG		fade_alpha=alpha<<24;
	SLONG		inside_clip=0;
	SLONG		reverse_textures=0;
	SLONG		style_index_offset=1;
	SLONG		style_index_step=2;
#ifdef	UNUSED_WIRECUTTERS
	SLONG		fence_gap,fence_gap_compare; //GAME_TURN&0xff;
#endif
	SLONG	flipx=0;


	#define MAX_SHAKE 32


	// Deal with internal gubbins.
	POLY_set_local_rotation_none();
	POLY_flush_local_rot();


	static float shake[MAX_SHAKE] =
	{
		0,0,0,0,
		+1,-2,+1,-1,
		+3,-3,+2,-2,
		+4,-3,+2,-4,
		+5,-5,+5,-4,
		+6,-4,+7,-6,
		+7,-6,+8,-9,
		+9,-7,+8,-9,
	};

	ASSERT(facet>0&&facet<next_dfacet);
	p_facet=&dfacets[facet];

	if(p_facet->FacetFlags&FACET_FLAG_INVISIBLE)
	{
		return;
	}

#ifdef	QUICK_FACET
	draw_quick_facet(p_facet);
	return;
#endif

//	if(facet==114 || facet==115 || facet==2037 ||facet==2036)
//		return;
#ifdef	UNUSED_WIRECUTTERS
	fence_gap=get_fence_hole(p_facet);
#endif




	if(INDOORS_DBUILDING==p_facet->Building && INDOORS_INDEX)
		inside_clip=1;

	if(facet==1)
	{
//		MSG_add("style %d \n",p_facet->StyleIndex);
	}

	//
	//Some types dont need back face culling
	//
	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_CABLE:
			cable_draw(p_facet);
			return;
			break;
	}

#ifndef TARGET_DC
	#ifndef NDEBUG

	//
	// For the editors...
	//

	if (dbuildings[p_facet->Building].Type == BUILDING_TYPE_WAREHOUSE)
	{
		if (AENG_transparent_warehouses)
		{
			return;
		}
	}

	#endif
#endif

	//
	// Should we bother drawing this facet?
	//

#ifdef FACET_REMOVAL_TEST

	// test
	//
	// normally only show invisible facets; press F to
	// remove invisible and show visible ones

	if (Keys[KB_F] && p_facet->Invisible)
	{
		return;
	}
	if (!Keys[KB_F] && !p_facet->Invisible)
	{
		return;
	}

#endif

	if ((p_facet->FacetType == STOREY_TYPE_FENCE        ||
		 p_facet->FacetType == STOREY_TYPE_FENCE_FLAT   ||
		 p_facet->FacetType == STOREY_TYPE_FENCE_BRICK  ||
		 p_facet->FacetType == STOREY_TYPE_INSIDE       ||
//		 p_facet->FacetType == STOREY_TYPE_OINSIDE      ||
		 p_facet->FacetType == STOREY_TYPE_INSIDE_DOOR  ||
		 p_facet->FacetType == STOREY_TYPE_OUTSIDE_DOOR ||
		 p_facet->FacetType == STOREY_TYPE_LADDER)
		&& !(p_facet->FacetFlags&FACET_FLAG_2SIDED)) // actually brick is barbed wire...
	{
		// 2sided flag means textured for both sides, if its textued on both sideds then you need to 
		// do a backface cull to decide which side you are drawing


		//
		// These facets are double-sided so they can't be backface culled.
		//
	}
	else
	if (p_facet->FacetType == STOREY_TYPE_JUST_COLLISION)
	{
		//
		// Don't draw these.
		// 

//#define WE_WANT_TO_DRAW_THESE_FACET_LINES 1
		#if WE_WANT_TO_DRAW_THESE_FACET_LINES

		AENG_world_line_infinite(
			p_facet->X[0],
			p_facet->Y[0],
			p_facet->Z[0],
			128,
			0x00ffff00,
			p_facet->X[1],
			p_facet->Y[1],
			p_facet->Z[1],
			0,
			0x00444488,
			TRUE);

		#endif

		return;
	}
	else
	{
		//
		// Backface cull the entire facet?
		//

		float x1, z1;
		float x2, z2;

		float vec1x;
		float vec1z;

		float vec2x;
		float vec2z;

		float cprod;

		x1 = float(p_facet->x[0] << 8);
		z1 = float(p_facet->z[0] << 8);

		x2 = float(p_facet->x[1] << 8);
		z2 = float(p_facet->z[1] << 8);

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

			if((p_facet->FacetFlags&FACET_FLAG_2SIDED) || p_facet->FacetType == STOREY_TYPE_OINSIDE)
			{
				//except its textured on the other side
				facet_backwards=1;
			}
			else
			{
				//
				// draw the barbedwire on the top of backface culled normal facets
				//
				if(p_facet->FacetFlags&FACET_FLAG_BARB_TOP)
					FACET_barbedwire_top(p_facet);

				return;
			}
		}
		else
		{
			// if OINSIDE, we don't draw it (there's another facet
			// for the outside wall) *but* we still need its thickness
			if (p_facet->FacetType == STOREY_TYPE_OINSIDE)
			{
				draw_wall_thickness(p_facet,fade_alpha);
				return; //only draw the 'other' side of these facets
			}
		}
	}

	//
	// Transform the bounding box of the facet to quickly try and reject the
	// entire facet.
	//

	if (abs(p_facet->x[1] - p_facet->x[0]) +
		abs(p_facet->z[1] - p_facet->z[0]) <= 2)
	{
		//
		// Too small the bother with the rejection test?
		// Nah! just do the rejection test anyway...
		//
	}

	if (p_facet->FacetType == STOREY_TYPE_OUTSIDE_DOOR)
	{
		//
		// We can't do this rejection test because the fence might be
		// open or closed.
		//
#ifdef EDITOR
		extern BOOL is_in_mission_editor;
		
		if (is_in_mission_editor)
		{
			p_facet->Open += 1;

			if (p_facet->FacetFlags & FACET_FLAG_90DEGREE)
			{
				if (p_facet->Open > 120)
				{
					p_facet->Open = 0;
				}
			}
		}
#endif
	}
	else
	{

		SLONG i;

		ULONG clip;
		ULONG clip_and;
		ULONG clip_or;

		POLY_Point bound;

		float x;
		float y;
		float z;

		float x1 = float(p_facet->x[0] << 8);
		float y1 = float(p_facet->Y[0]);
		float z1 = float(p_facet->z[0] << 8);

		float x2 = float(p_facet->x[1] << 8);
		float y2 = float(p_facet->Y[1]) + float(p_facet->Height * 64);
		float z2 = float(p_facet->z[1] << 8);

		clip_or  = 0x00000000;
		clip_and = 0xffffffff;

		for (i = 0; i < 4; i++)
		{
			x = (i & 0x1) ? x1 : x2;
			y = (i & 0x2) ? y1 : y2;
			z = (i & 0x1) ? z1 : z2;

			POLY_transform_c_saturate_z(x, y, z, &bound);

			clip = bound.clip;

			if ((clip & POLY_CLIP_TRANSFORMED) && !(clip & POLY_CLIP_OFFSCREEN))
			{
				//
				// Draw the whole facet because this point is on-screen.
				//

				goto draw_the_facet;
			}

			clip_and &= clip;
			clip_or  |= clip;
		}

		if (clip_and & POLY_CLIP_OFFSCREEN)
		{
			//
			// Reject the whole facet.
			//

			return;
		}

		if (!(clip_or & POLY_CLIP_TRANSFORMED))
		{
			//
			// Reject the whole facet if all points are too far or all points are too near
			//
			if(clip_and & (POLY_CLIP_NEAR|POLY_CLIP_FAR))
			{
				return;
			}
		}

	  draw_the_facet:;
	}

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

		POLY_flush_local_rot();
		DRAW_ladder(p_facet);

		return;
	}

	POLY_buffer_upto = 0;

	style_index=p_facet->StyleIndex;

	//
	// Should this be passed an x,y,z to be relative to? Nah!
	//

	set_facet_seed(p_facet->x[0] * p_facet->z[0] + p_facet->Y[0]);

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

		if (p_facet->Dfcache == NULL)
		{
			return;
		}
	}
	
	ASSERT(WITHIN(p_facet->Dfcache, 1, NIGHT_MAX_DFCACHES - 1));

	col = NIGHT_dfcache[p_facet->Dfcache].colour;

	dx = (p_facet->x[1]-p_facet->x[0]) << 8;
	dz = (p_facet->z[1]-p_facet->z[0]) << 8;

	sx = float(p_facet->x[0] << 8);
	sy = float(p_facet->Y[0]     );
	sz = float(p_facet->z[0] << 8);
/*
	if(dz)
		return;

		if(sy>256.0) // || sy>513.0)
			return;
  */

	height=p_facet->Height;
	//	if(height!=12)
	//		return;

	if(dx&&dz)
	{
		LogText(" diagonal wall \n");
		if(p_facet->FacetType==STOREY_TYPE_NORMAL)
		{
			ASSERT(0);
		}



		{
			SLONG	len;
			SLONG	adx,adz;

			adx=abs(dx);
			adz=abs(dz);
			len=QDIST2(adx,adz);
			count=len>>8;
			if(count==0)
				count=1;

			fdx=(float)dx/(float)count;
			fdz=(float)dz/(float)count;

			diag=0;
			
		}

//		return;
	}
	else
	{
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

		fdx=(float)dx;
		fdz=(float)dz;

		if (p_facet->Open)
		{
			float rdx;
			float rdz;
			float angle = float(p_facet->Open) * (PI / 256.0F);

			//
			// Open the facet!
			//

			rdx =  cos(angle) * fdx + sin(angle) * fdz;
			rdz = -sin(angle) * fdx + cos(angle) * fdz;

			fdx = rdx;
			fdz = rdz;
		}	
	}

	count++;
/*

	{
		CBYTE	str[10];
		sprintf(str,"facet %d h %d bs %d",facet,p_facet->Height,p_facet->BlockHeight);

extern	FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);
		FONT2D_DrawString_3d(str,p_facet->x[0]<<8,p_facet->Y[0],p_facet->z[0]<<8,0xff0000,60,0);
	}
*/




	//
	// Work out the height offset for each point along a fence.
	//

	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_FENCE:
		case	STOREY_TYPE_FENCE_FLAT:
		case	STOREY_TYPE_FENCE_BRICK:
		case	STOREY_TYPE_OUTSIDE_DOOR:

			{
				float	*p_diffy;
				float	dy;

				p_diffy=diff_y;

				x=sx;
				y=sy;
				z=sz;
				c0=count;
				while(c0-->0)
				{
					if (p_facet->FacetFlags & FACET_FLAG_ONBUILDING)
					{
						//
						// No offset for building facets.
						//

						*p_diffy++ = p_facet->Y[0];
					}
					else
					{
						if(diag)
						{
							dy=(float)PAP_calc_height_noroads((SLONG)x,(SLONG)z);
						}
						else
						{
							dy=grid_height_at_world(x,z);
						}
						*p_diffy=dy;

						p_diffy++;

						x   += fdx;
						z   += fdz;
					}
				}
			}
			break;
	}


	//MSG_add(" facet %d draw count %d\n",facet,count);

	switch(p_facet->FacetType)
	{
		case	STOREY_TYPE_CABLE:
			cable_draw(p_facet);
			break;
		case	STOREY_TYPE_INSIDE_DOOR:
			if(facet_backwards && (p_facet->FacetFlags&FACET_FLAG_2SIDED))
				style_index++;
			if(facet_backwards)
			{
				
				DRAW_door(sx,sy,sz,fdx,256.0,fdz,block_height,count,fade_alpha,dstyles[style_index],1);
			}
			else
			{
				DRAW_door(sx,sy,sz,fdx,256.0,fdz,block_height,count,fade_alpha,dstyles[style_index],0);
			}
			break;
		case	STOREY_TYPE_OINSIDE:

			style_index--;
//			ASSERT(0);

		case	STOREY_TYPE_INSIDE:
				draw_wall_thickness(p_facet,fade_alpha);
				if(facet_backwards)
				{
					flipx=1;
					if(!ShiftFlag)
						style_index++;
				}

				hf=0;
   				while(height>=0)
				{
					x=sx;
					y=sy;
					z=sz;
					rows[hf]=POLY_buffer_upto;
					c0=count;
					while(c0-->0)
					{

						ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

						pp = &POLY_buffer[POLY_buffer_upto++];

						POLY_transform(x,y,z,pp);

						if (pp->MaybeValid())
						{

							NIGHT_get_d3d_colour(
								*col,
								&pp->colour,
								&pp->specular);
							//POLY_fadeout_point(pp);

							pp->colour|=fade_alpha;
//							pp->specular=0xff000000;
						}

						x   += fdx;
						z   += fdz;
						col += 1;
					}

					if(hf>0)
					{
						SLONG	row1,row2;

						row1=rows[hf-1];
						row2=rows[hf];

						//
						// create the quads and submit them for drawing
						//

						c0=0; //count-1;
						while(c0<count-1)
						{
							quad[0] = &POLY_buffer[row2+c0+1];
							quad[1] = &POLY_buffer[row2+c0];
							quad[2] = &POLY_buffer[row1+c0+1];
							quad[3] = &POLY_buffer[row1+c0];

							if (POLY_valid_quad(quad))
							{
								SLONG page;
								
								//
								// Texture the quad.
								// 

								page=texture_quad(quad,dstyles[style_index],c0,count,flipx);

								if(foundation==1)
								{
									quad[3]->v=diff_y[c0]/256.0f;
									quad[2]->v=diff_y[c0+1]/256.0f;
								}

								POLY_add_quad(quad, page, FALSE); // TRUE means perform a backface cull
							}
							else
							{
								//
								// Even though we don't draw the quad, we must
								// push on the random number generator.
								//

								facet_rand();
							}

							c0++;

						}
					}
					foundation--;
					sy+=block_height;//+64.0;
					height-=4;
					hf+=1;
/*
					style_index++;
					if(facet_backwards)
					{
						style_index++;
					}
*/

					//
					// For when we are drawing the building we are in.
					//

					if (sy > max_height)
					{
						break;
					}
				}
				break;


		case	STOREY_TYPE_TRENCH:
			LogText(" alt %d \n",p_facet->Y[0]);

		case	STOREY_TYPE_NORMAL:
		case	STOREY_TYPE_DOOR:

				if(p_facet->FacetFlags&FACET_FLAG_BARB_TOP)
					FACET_barbedwire_top(p_facet);

			//
			//	warehouses can be double sided and storey_type_normal
			//
				if(facet_backwards&&!(p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR))
				{
					//
					// I don't understand by Mike!
					//
					style_index++;
				}

				block_height=p_facet->BlockHeight<<4;

				if(inside_clip) //INDOORS_INDEX)
				{
					SLONG	top;
					top=block_height;
					top=(top*height)>>2;
					top+=sy;
					if( top>=(inside_storeys[INDOORS_INDEX].StoreyY+256))
					{
						//
						// clip the top of the building, but first check fade status
						//

/*
						if(INDOORS_INDEX_FADE_EXT_DIR)
						{
							//
							// we are fadeing so draw it
							//
							fade_alpha=INDOORS_INDEX_FADE_EXT<<24;
						}
						else
*/
						{
							height-=((top+4)-(inside_storeys[INDOORS_INDEX].StoreyY+256))/(p_facet->BlockHeight<<2);
						}
					}


				}

			if(p_facet->FHeight)
				foundation=2;

//			MakeFacetPoints(sx, sy, sz, fdx, fdz, block_height, height, max_height, col, foundation, count, p_facet->FacetFlags&(FACET_FLAG_INVISIBLE|FACET_FLAG_IN_SEWERS),p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR);
			MakeFacetPoints(sx, sy, sz, fdx, fdz, block_height, height, max_height, col, foundation, count, p_facet->FacetFlags&(FACET_FLAG_INVISIBLE),p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR);

//			p_facet->FacetFlags&=~(FACET_FLAG_IN_SEWERS);

			if(p_facet->FacetFlags&(FACET_FLAG_INSIDE))
			{
				reverse_textures=1;
				style_index--;
			}
			else
			if(p_facet->FacetFlags&(FACET_FLAG_2TEXTURED))
			{
				style_index--;
			}

			if(!(p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR) && (p_facet->FacetFlags&(FACET_FLAG_2TEXTURED|FACET_FLAG_2SIDED)))
			{
				style_index_offset=1;
				style_index_step=2;
			}
			else
			{
				style_index_offset=1;
				style_index_step=1;

			}


			hf = 0;
			while (height >= 0)
			{
				if (hf)
				{
					FillFacetPoints(count, hf - 1, foundation + 1, facet_backwards, style_index - style_index_offset, block_height,reverse_textures);
				}

				foundation--;
				sy += block_height;
				height -= 4;
				hf++;
				style_index+=style_index_step;
//				if(p_facet->FacetFlags&(FACET_FLAG_2SIDED|FACET_FLAG_2TEXTURED))
//					style_index++;

				if (sy > max_height)
				{
					break;
				}
			}
			break;

		case	STOREY_TYPE_FENCE_BRICK:

			// hardwire the height
			p_facet->Height += 1;

			// bollocks to it all i'm doing my own so nnyeerrrr
			FACET_barbedwire_top(p_facet);
			/*
			{
				float dx=(p_facet->x[1]-p_facet->x[0] << 8);
				float dy=(p_facet->Y[1]-p_facet->Y[0]);
				float dz=(p_facet->z[1]-p_facet->z[0] << 8);
				float mag=sqrt((dx*dx)+(dz*dz));
				float stepx=(dx/mag)*10;
				float stepy=(dy/mag)*10;
				float stepz=(dz/mag)*10;
				SLONG cx=p_facet->x[0] << 8;
				SLONG cy=p_facet->Y[0];
				SLONG cz=p_facet->z[0] << 8;
				SLONG seed=54321678;
				float base=0;
				SLONG contour = 0;


				while (base<mag) {

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

					SPRITE_draw_tex(
						cx,
						(64*p_facet->Height)-64+((seed>>8)&0xf) + contour,
						cz,
						50,
						0xffffff,
						0,
						POLY_PAGE_BARBWIRE,
						0.0, 0.0, 1.0, 1.0,
						SPRITE_SORT_NORMAL);


						base+=10;
						cx+=stepx;
						cy+=stepy;
						cz+=stepz;
				}

			}
			*/
			p_facet->Height -= 1;

			//
			// FALLTHROUGH!
			//

		case	STOREY_TYPE_FENCE:

				POLY_set_local_rotation_none();

				// this check is due to fence_brick dropping thru
				if (p_facet->FacetType==STOREY_TYPE_FENCE)
				{
					//
					// build the slope at the top of the fence
					//
					float	dx,dz,nx,nz;
					float	tsx,tsy,tsz;
					sy=0;
					build_fence_poles(sx,sy,sz,fdx,fdz,count,&dx,&dz,style_index);

					tsx=sx;
					tsy=sy;
					tsz=sz;

					nx=dz;
					nz=-dx;

					sx+=nx*10.0f;
					sz+=nz*10.0f;
					sy+=210.0f;

					hf=0;
					while(hf<=1)
					{
						float	*p_diffy=diff_y;
						x=sx;
						y=sy;
						z=sz;
						rows[hf]=POLY_buffer_upto;
						c0=count;
						while(c0-->0)
						{
							float	dy = 0;

							ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

							pp = &POLY_buffer[POLY_buffer_upto++];

//							dy=0; //(float)(PAP_2HI(((SLONG)x)>>8,((SLONG)z)>>8).Alt<<3);
/*
							if(diag)
							{
								dy=PAP_calc_height_at((SLONG)x,(SLONG)z);
							}
							else
							{
								dy=grid_height_at_world(x,z);
							}
							*/

							dy=*p_diffy++;

							POLY_transform_c_saturate_z(x,y+dy,z,pp);

							if (pp->MaybeValid())
							{
								NIGHT_get_d3d_colour(
									*col,
									&pp->colour,
									&pp->specular);
							//apply_cloud((SLONG)x,(SLONG)(y+dy),(SLONG)z,&pp->colour);

								//POLY_fadeout_point(pp);
							}

							pp->colour|=fade_alpha;
							x   += fdx;
							z   += fdz;
//							col += 1; better not increase this until the cache create knows about fences
						}
						sy+=40.0f;
						sx+=nx*50.0f;
						sz+=nz*50.0f;
						hf++;

					}

					//
					// now draw the quads
					//
					{
						SLONG	row1,row2;

						row1=rows[0];
						row2=rows[1];

						//
						// create the quads and submit them for drawing
						//

						c0=0; //count-1;
						while(c0<count-1)
						{


							quad[0] = &POLY_buffer[row2+c0+1];
							quad[1] = &POLY_buffer[row2+c0];
							quad[2] = &POLY_buffer[row1+c0+1];
							quad[3] = &POLY_buffer[row1+c0];

							if (POLY_valid_quad(quad))
							{
								SLONG	page;
								
								//
								// Texture the quad.
								// 

								page=texture_quad(quad,dstyles[style_index],c0,count);

								POLY_add_quad(quad, page, 0); // 1 means perform a backface cull
							}
							else
							{
								//
								// Push on the random number generator.
								//

								facet_rand();
							}

							c0++;
						}
					}



					//
					//correct the fiddling we did to these values
					//

					sx=tsx;
					sy=tsy;
					sz=tsz;


					height=3; //fence is 3/4 height

					
				}

				//
				//proceed to draw the fence
				//


		case	STOREY_TYPE_OUTSIDE_DOOR:
		case	STOREY_TYPE_FENCE_FLAT:

				hf=0;
/*
				if(height==12)
				{
					//
					// triple high
					// 


					MakeFacetPointsFence(sx, sy, sz, fdx, fdz, block_height, height, max_height, col, foundation, count, p_facet->FacetFlags&(FACET_FLAG_INVISIBLE|FACET_FLAG_IN_SEWERS),diff_y);
					p_facet->FacetFlags&=~(FACET_FLAG_IN_SEWERS);



					hf = 0;
					while (height >= 0)
					{
						if (hf)
						{
							FillFacetPoints(count, hf - 1, foundation + 1, facet_backwards, style_index - 1, block_height,reverse_textures);
						}

						foundation--;
						sy += block_height;
						height -= 4;
						hf++;
						//style_index++;
		//				if(p_facet->FacetFlags&(FACET_FLAG_2SIDED|FACET_FLAG_2TEXTURED))
		//					style_index++;

						if (sy > max_height)
						{
							break;
						}
					}
					break;


				}
*/


				if (p_facet->Shake)
				{
					p_facet->Shake -= p_facet->Shake >> 2;
					p_facet->Shake -= 1;
				}

				//
				// fences DO lock to the floor
				//

				sy=0;

#ifdef	UNUSED_WIRECUTTERS
				fence_gap_compare=fence_gap*(count-1);
#endif
				y=sy;


				while(hf<=1)
				{
					float	*p_diffy=diff_y;
					x=sx;
//					y=sy;
					z=sz;
					rows[hf]=POLY_buffer_upto;
					c0=count;
					while(c0-->0)
					{
						float	dy = 0;

						ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

						pp = &POLY_buffer[POLY_buffer_upto++];
//						dy=(float)(PAP_2HI(((SLONG)x)>>8,((SLONG)z)>>8).Alt<<3);

/*
						if(diag)
						{
							dy=PAP_calc_height_at((SLONG)x,(SLONG)z);
						}
						else
						{
							dy=grid_height_at_world(x,z);
						}
*/
//						ASSERT(dy==0);

						dy=*p_diffy++;

						POLY_transform_c_saturate_z(
							x + shake[(Random() & 3) + ((p_facet->Shake >> 3) & ~0x3)],
							y + dy,
							z + shake[(Random() & 3) + ((p_facet->Shake >> 3) & ~0x3)],
							pp);

						if (pp->MaybeValid())
						{
							NIGHT_get_d3d_colour(
								*col,
								&pp->colour,
								&pp->specular);

							//apply_cloud((SLONG)x,(SLONG)(y+dy),(SLONG)z,&pp->colour);
							//POLY_fadeout_point(pp);
							pp->colour|=fade_alpha;
						}

						x   += fdx;
						z   += fdz;
						col += 1;
					}
					if(height==2)
						y+=102; //256.0/3.0;
					else
						y+=height*BLOCK_SIZE;

					hf++;

				}

				{
					SLONG	row1,row2;

					row1=rows[0];
					row2=rows[1];

					//
					// create the quads and submit them for drawing
					//

					c0=0; //count-1;
					while(c0<count-1)
					{


						quad[0] = &POLY_buffer[row2+c0+1];
						quad[1] = &POLY_buffer[row2+c0];
						quad[2] = &POLY_buffer[row1+c0+1];
						quad[3] = &POLY_buffer[row1+c0];

						/*

						if(fence_gap && c0==(fence_gap_compare>>8))
						{
							SLONG	page;
							float	fdy;

							fdy=diff_y[c0+1]-diff_y[c0];
							page=texture_quad(quad,dstyles[style_index],c0,count);
							draw_fence_gap(quad,page,fence_gap_compare,sx+fdx*c0,sy+fdy*c0,sz+fdz*c0,fdx,fdy,fdz);
								//
								// Push on the random number generator.
								//

							facet_rand();

							fence_gap=get_fence_hole_next(p_facet,fence_gap);
							fence_gap_compare=fence_gap*(count-1);
						}
						else

						*/
						{
							if (POLY_valid_quad(quad))
							{
								SLONG	page;
								
								//
								// Texture the quad.
								// 

								page=texture_quad(quad,dstyles[style_index],c0,count);

// Flashing pink! No thanks.
#if 0
								#ifndef NDEBUG

								if (p_facet->Height == 2 && p_facet->BlockHeight == 16)
								{
									//
									// This is a vaultable fence- so it doesn't matter.
									//
								}
								else
								{
									if (page == 284)
									{
										//
										// This is the mesh texture so the fence should be
										// climable.
										// 

										if (p_facet->FacetFlags & FACET_FLAG_UNCLIMBABLE)
										{
											if (GAME_TURN & 0x8)
											{
												quad[0]->specular |= 0xff00ff;
												quad[1]->specular |= 0xff00ff;
												quad[2]->specular |= 0xff00ff;
												quad[3]->specular |= 0xff00ff;
											}
										}
									}
									else
									{
										//
										// A climbable non-mesh fence.
										//

										if (!(p_facet->FacetFlags & FACET_FLAG_UNCLIMBABLE))
										{
											if (GAME_TURN & 0x8)
											{
												quad[0]->specular |= 0xff00ff;
												quad[1]->specular |= 0xff00ff;
												quad[2]->specular |= 0xff00ff;
												quad[3]->specular |= 0xff00ff;
											}
										}
									}
								}

								#endif
#endif

								POLY_add_quad(quad, page, 0); // 1 means perform a backface cull
							}
							else
							{
								//
								// Push on the random number generator.
								//

								facet_rand();
							}
						}

						c0++;
					}
				}

				break;

			break;
	}
	return;
}




#ifdef DEBUG
static bPleaseDoSuperFacets = TRUE;
#endif



// Like FACET_draw_rare, but for the most common class.
void FACET_draw(SLONG facet,UBYTE alpha)
{
	struct		  DFacet	*p_facet;
	static SWORD  rows[100];
	SLONG		  c0,count;
	SLONG		  dx,dz;
	float		  x,y,z,sx,sy,sz,fdx,fdz;
	SLONG		  height;
  	POLY_Point   *pp;
	SLONG         hf;
	POLY_Point   *quad[4];
	SLONG		  style_index;
	NIGHT_Colour *col;
	SLONG         max_height;
	SLONG		foundation=0;
	static float	diff_y[128];
	float		block_height=256.0;

	SLONG		diag=0;
	SLONG		facet_backwards=0;
	ULONG		fade_alpha=alpha<<24;
	SLONG		inside_clip=0;
	SLONG		reverse_textures=0;
	//SLONG		style_index_offset=1;
	SLONG		style_index_step=2;
	SLONG	flipx=0;


	LOG_ENTER ( Facet_draw_start )


	ASSERT(facet>0&&facet<next_dfacet);
	p_facet=&dfacets[facet];


	if(p_facet->FacetFlags&FACET_FLAG_INVISIBLE)
	{
		LOG_EXIT ( Facet_draw_start )
		return;
	}


	// Alpha seems to be never used, and always set to 0
	ASSERT ( alpha == 0 );

	// Now spot the odder forms of facet, and use the _rare routine to draw them instead.
	if ( ( p_facet->FacetType != STOREY_TYPE_NORMAL ) ||
		 ( INDOORS_INDEX ) ||
		 ( ( p_facet->FacetFlags & ( FACET_FLAG_BARB_TOP | FACET_FLAG_2SIDED | FACET_FLAG_INSIDE ) ) != 0 ) )
	{
		// Use it!
		FACET_draw_rare ( facet, alpha );
		LOG_EXIT ( Facet_draw_start )
		return;
	}


	ASSERT ( ( p_facet->FacetType == STOREY_TYPE_NORMAL ) );
	ASSERT ( ( p_facet->FacetFlags & FACET_FLAG_INVISIBLE ) == 0 );
	ASSERT ( ( p_facet->FacetFlags & FACET_FLAG_BARB_TOP ) == 0 );
	ASSERT ( ( p_facet->FacetFlags & FACET_FLAG_2SIDED ) == 0 );
	ASSERT ( ( p_facet->FacetFlags & FACET_FLAG_INSIDE ) == 0 );
	ASSERT ( !INDOORS_INDEX );

	//
	// Should we bother drawing this facet?
	//

	{
		//
		// Backface cull the entire facet?
		//

		float x1, z1;
		float x2, z2;

		float vec1x;
		float vec1z;

		float vec2x;
		float vec2z;

		float cprod;

		x1 = float(p_facet->x[0] << 8);
		z1 = float(p_facet->z[0] << 8);

		x2 = float(p_facet->x[1] << 8);
		z2 = float(p_facet->z[1] << 8);

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
			LOG_EXIT ( Facet_draw_start )
			return;
		}
	}

	//
	// Transform the bounding box of the facet to quickly try and reject the
	// entire facet.
	//

	ULONG clip_or;		// Used below.
	{

		SLONG i;

		ULONG clip;
		ULONG clip_and;

		POLY_Point bound;

		float x;
		float y;
		float z;

		float x1 = float(p_facet->x[0] << 8);
		float y1 = float(p_facet->Y[0]);
		float z1 = float(p_facet->z[0] << 8);

		float x2 = float(p_facet->x[1] << 8);
		float y2 = float(p_facet->Y[1]) + float(p_facet->Height * 64);
		float z2 = float(p_facet->z[1] << 8);

		clip_or  = 0x00000000;
		clip_and = 0xffffffff;

		for (i = 0; i < 4; i++)
		{
			x = (i & 0x1) ? x1 : x2;
			y = (i & 0x2) ? y1 : y2;
			z = (i & 0x1) ? z1 : z2;

			POLY_transform_c_saturate_z(x, y, z, &bound);

			clip = bound.clip;

			if ((clip & POLY_CLIP_TRANSFORMED) && !(clip & POLY_CLIP_OFFSCREEN))
			{
				//
				// Draw the whole facet because this point is on-screen.
				//

				// But this frags the near-plane clip detection.
				// Don't need it any more. Hooray!
				goto draw_the_facet_common;
			}

			clip_and &= clip;
			clip_or  |= clip;
		}

		if (clip_and & POLY_CLIP_OFFSCREEN)
		{
			//
			// Reject the whole facet.
			//

			LOG_EXIT ( Facet_draw_start )
			return;
		}

		if (!(clip_or & POLY_CLIP_TRANSFORMED))
		{
			//
			// Reject the whole facet if all points are too far or all points are too near
			//
			if(clip_and & (POLY_CLIP_NEAR|POLY_CLIP_FAR))
			{
				LOG_EXIT ( Facet_draw_start )
				return;
			}
		}

draw_the_facet_common:;
	}

	LOG_EXIT ( Facet_draw_start )

	dfacets_drawn_this_gameturn += 1;
	LOG_ENTER ( Facet_draw_mid )

	{
		float yaw;

		if (p_facet->z[0] == p_facet->z[1])
		{
			if (p_facet->x[0] < p_facet->x[1])
			{
				yaw = 0.0F;
			}
			else
			{
				yaw = PI;
			}
		}
		else
		{
			if (p_facet->z[0] > p_facet->z[1])
			{
				yaw = 3 * PI / 2;
			}
			else
			{
				yaw = PI / 2;
			}
		}

		MATRIX_calc(
			FACET_direction_matrix,
			yaw,
			0.0F,
			0.0F);
	}

// Can't do these for release yet - no glowing windows, and the fog doesn't work.
// Fog works, and Mark says he's done the glowing windows. Hooray!
#define DO_SUPERFACETS_PLEASE_BOB defined


#ifdef DO_SUPERFACETS_PLEASE_BOB
	if (p_facet->Open)
	{
		//
		// Don't cache facets that open and close!
		//
	}
	else
	{
		if (SUPERFACET_draw(facet))
		{
			p_facet->FacetFlags &= ~FACET_FLAG_DLIT;

			return;
		}

		p_facet->FacetFlags &= ~FACET_FLAG_DLIT;

		/*



		if ( ( ( clip_or & POLY_CLIP_NEAR ) == 0)
#ifdef DEBUG
			&& bPleaseDoSuperFacets
#endif
			)
		{
			if (SUPERFACET_draw(facet))
			{
				return;
			}
		}

		*/

	}
#endif

	//
	// Draw the facet.
	// 

	p_facet->FacetFlags &= ~FACET_FLAG_DLIT;

	POLY_buffer_upto = 0;

	style_index=p_facet->StyleIndex;

	//
	// Should this be passed an x,y,z to be relative to? Nah!
	//

	set_facet_seed(p_facet->x[0] * p_facet->z[0] + p_facet->Y[0]);

	ASSERT ( ( GAME_FLAGS & GF_INDOORS ) == 0 );

	max_height = INFINITY;

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

		if (p_facet->Dfcache == NULL)
		{
			LOG_EXIT ( Facet_draw_mid )
			return;
		}
	}
	
	ASSERT(WITHIN(p_facet->Dfcache, 1, NIGHT_MAX_DFCACHES - 1));

	col = NIGHT_dfcache[p_facet->Dfcache].colour;

	dx = (p_facet->x[1]-p_facet->x[0]) << 8;
	dz = (p_facet->z[1]-p_facet->z[0]) << 8;

	sx = float(p_facet->x[0] << 8);
	sy = float(p_facet->Y[0]     );
	sz = float(p_facet->z[0] << 8);
/*
	if(dz)
		return;

		if(sy>256.0) // || sy>513.0)
			return;
  */

	height=p_facet->Height;
	//	if(height!=12)
	//		return;

	// No diagonal walls allowed.
	ASSERT ( !(dx && dz) );

	{
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

		fdx=(float)dx;
		fdz=(float)dz;

		if (p_facet->Open)
		{
			float rdx;
			float rdz;
			float angle = float(p_facet->Open) * (PI / 256.0F);

			//
			// Open the facet!
			//

			rdx =  cos(angle) * fdx + sin(angle) * fdz;
			rdz = -sin(angle) * fdx + cos(angle) * fdz;

			fdx = rdx;
			fdz = rdz;
		}	
	}

	count++;

	LOG_EXIT ( Facet_draw_mid )

	LOG_ENTER ( Facet_draw_main )

	//MSG_add(" facet %d draw count %d\n",facet,count);

	ASSERT ( p_facet->FacetType == STOREY_TYPE_NORMAL );

	ASSERT ( (p_facet->FacetFlags&FACET_FLAG_BARB_TOP) == 0 );

	//
	//	warehouses can be double sided and storey_type_normal
	//

	ASSERT ( facet_backwards == 0 );

	block_height=p_facet->BlockHeight<<4;

	ASSERT ( inside_clip == 0 );
	
	if(p_facet->FHeight)
	{
		foundation=2;
	}

	MakeFacetPointsCommon(sx, sy, sz, fdx, fdz, block_height, height, col, foundation, count, p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR );

//	p_facet->FacetFlags&=~(FACET_FLAG_IN_SEWERS);

	ASSERT ( ( p_facet->FacetFlags&(FACET_FLAG_INSIDE) ) == 0 );
	if(p_facet->FacetFlags&(FACET_FLAG_2TEXTURED))
	{
		style_index--;
	}

	if(!(p_facet->FacetFlags&FACET_FLAG_HUG_FLOOR) && (p_facet->FacetFlags&(FACET_FLAG_2TEXTURED|FACET_FLAG_2SIDED)))
	{
		//style_index_offset=1;
		style_index_step=2;
	}
	else
	{
		//style_index_offset=1;
		style_index_step=1;

	}

	#ifdef FACETINFO
	FACET_facetinfo_current = facet;
	#endif

	ASSERT ( reverse_textures == 0 );

	hf = 0;
	while (height >= 0)
	{
		if (hf)
		{
			ASSERT ( facet_backwards == 0 );
			ASSERT ( reverse_textures == 0 );
			//FillFacetPoints(count, hf - 1, foundation + 1, 0, style_index - style_index_offset, block_height, 0);
			FillFacetPointsCommon(count, hf - 1, foundation + 1, style_index - 1, block_height );
		}

		foundation--;
		sy += block_height;
		height -= 4;
		hf++;
		style_index+=style_index_step;
	//				if(p_facet->FacetFlags&(FACET_FLAG_2SIDED|FACET_FLAG_2TEXTURED))
	//					style_index++;

	}

	#ifdef FACETINFO
	ASSERT(WITHIN(FACET_facetinfo_current, 0, FACET_MAX_FACETINFO - 1));
	FACET_facetinfo[FACET_facetinfo_current].done = TRUE;
	#endif

	LOG_EXIT ( Facet_draw_main )
	return;
}



extern SLONG AENG_drawing_a_warehouse;

void FACET_draw_walkable(SLONG build)
{
	SLONG i;
	SLONG j;

	SLONG ep;

	SLONG red;
	SLONG green;
	SLONG blue;

	SLONG page;
	SLONG warehouse; 

	SLONG walkable;

	struct	RoofFace4  *p_f4;
	struct DWalkable *p_walk;
	struct DBuilding *p_dbuilding;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri[3];
	POLY_Point *quad[4];

	ASSERT(WITHIN(build, 1, next_dbuilding - 1));

	p_dbuilding = &dbuildings[build];

	UWORD *rooftex = NULL;

#ifdef TARGET_DC
	// Internal stuff fixup
	POLY_flush_local_rot();
#endif

	//
	// Is this building a warehouse?
	//

	warehouse = (p_dbuilding->Type == BUILDING_TYPE_WAREHOUSE);

	#ifndef NDEBUG

	if (warehouse && AENG_transparent_warehouses)
	{
		return;
	}

	#endif

	if (warehouse)
	{
		ASSERT(WITHIN(p_dbuilding->Ware, 0, WARE_ware_upto - 1));

		rooftex = &WARE_rooftex[WARE_ware[p_dbuilding->Ware].rooftex];

		//FONT2D_DrawString("Warehouse walkables", 50 + (rand() & 0xf), 50, 0xffffff);

	}
	else
	{
		//FONT2D_DrawString("Non-warehouse walkables", 50 + (rand() & 0xf), 70, 0xffffff);
	}

	//
	// Rotate all the points into the POLY_buffer and all the
	// shadow points in the POLY_shadow.
	//

	for (walkable = p_dbuilding->Walkable; walkable; walkable = p_walk->Next)
	{
		p_walk = &dwalkables[walkable];

		//
		// REJECTION OF WHOLE WALKABLE SET IF ITS ABOVE THE CAMERA?
		//

		{
			//
			// The quads.
			//

			POLY_buffer_upto = 0;
			POLY_shadow_upto = 0;

			for (i = p_walk->StartFace4; i < p_walk->EndFace4; i++, rooftex++)
			{
				float	px,py,pz,sy;
				p_f4 = &roof_faces4[i];

				if (p_f4->DrawFlags & RFACE_FLAG_NODRAW)
				{
					continue;
				}

				pp = &POLY_buffer[0];

				px=(float)((p_f4->RX&127)<<8);
				pz=(float)((p_f4->RZ&127)<<8);
				py=(float)(p_f4->Y);
				sy=py;
				POLY_transform(px,py,pz,pp);

				if (pp->MaybeValid())
				{
					NIGHT_get_d3d_colour(
						NIGHT_ROOF_WALKABLE_POINT(i,0),
					   &pp->colour,
					   &pp->specular);

					//apply_cloud((SLONG)px,(SLONG)py,(SLONG) pz,&pp->colour);

					//POLY_fadeout_point(pp);
				}
				pp++;
				px+=256.0f;
				py+=p_f4->DY[0]<<ROOF_SHIFT;
				POLY_transform(px,py,pz,pp);

				if (pp->MaybeValid())
				{
					NIGHT_get_d3d_colour(
						NIGHT_ROOF_WALKABLE_POINT(i,1),
					   &pp->colour,
					   &pp->specular);

					//apply_cloud((SLONG)px,(SLONG)py,(SLONG) pz,&pp->colour);

					//POLY_fadeout_point(pp);
				}
				pp++;
				pz+=256.0f;
				py=sy+(p_f4->DY[1]<<ROOF_SHIFT);
				POLY_transform(px,py,pz,pp);

				if (pp->MaybeValid())
				{
					NIGHT_get_d3d_colour(
						NIGHT_ROOF_WALKABLE_POINT(i,3),
					   &pp->colour,
					   &pp->specular);

					//apply_cloud((SLONG)px,(SLONG)py,(SLONG) pz,&pp->colour);

					//POLY_fadeout_point(pp);
				}
				pp++;
				px-=256.0f;
				py=sy+(p_f4->DY[2]<<ROOF_SHIFT);
				POLY_transform(px,py,pz,pp);

				if (pp->MaybeValid())
				{
					NIGHT_get_d3d_colour(
						NIGHT_ROOF_WALKABLE_POINT(i,2),
					   &pp->colour,
					   &pp->specular);

					//apply_cloud((SLONG)px,(SLONG)py,(SLONG) pz,&pp->colour);

					//POLY_fadeout_point(pp);
				}

				quad[0] = &POLY_buffer[0];
				quad[1] = &POLY_buffer[1];
				quad[2] = &POLY_buffer[3];
				quad[3] = &POLY_buffer[2];

				if (POLY_valid_quad(quad))
				{
//					#if DRAW_THIS_DEBUG_STUFF

#ifndef TARGET_DC
					if(ControlFlag&&allow_debug_keys)
					{
						SLONG	x,z,y;

						x=(p_f4->RX&127)<<8;
						z=(p_f4->RZ&127)<<8;
						y=p_f4->Y;
						//
						// Draw the slide-edges
						//

						
						if (p_f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE_0))
						{
								AENG_world_line(x,y,z,4,0xffffff,x+256,y,z,4,0xffffff,1);

						}
						if (p_f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE_1))
						{
								AENG_world_line(x+256,y,z,4,0xffffff,x+256,y,z+256,4,0xffffff,1);

						}
						if (p_f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE_2))
						{
								AENG_world_line(x+256,y,z+256,4,0xffffff,x,y,z+256,4,0xffffff,1);

						}
						if (p_f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE_3))
						{
								AENG_world_line(x,y,z+256,4,0xffffff,x,y,z,4,0xffffff,1);

						}
					}
#endif

//					#endif

					if (warehouse)
					{
						//
						// If this face is above the camera... it must be the ceiling-
						// so draw it all in black.
						//

						if (p_f4->Y > (FC_cam[0].y >> 8))
						{
							quad[0]->colour   = 0x00000000;
							quad[0]->specular = 0xff000000;

							quad[1]->colour   = 0x00000000;
							quad[1]->specular = 0xff000000;

							quad[2]->colour   = 0x00000000;
							quad[2]->specular = 0xff000000;

							quad[3]->colour   = 0x00000000;
							quad[3]->specular = 0xff000000;

							POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE);

							continue;
						}
						else
						{
							TEXTURE_get_minitexturebits_uvs(
								   *rooftex,
								   &page,
								   &quad[0]->u,
								   &quad[0]->v,
								   &quad[1]->u,
								   &quad[1]->v,
								   &quad[2]->u,
								   &quad[2]->v,
								   &quad[3]->u,
								   &quad[3]->v);
						}
					}
					else
					{
						PAP_Hi	*ph;
						ph = &PAP_2HI(p_f4->RX&127,p_f4->RZ&127);

						TEXTURE_get_minitexturebits_uvs(
								ph->Texture,
							   &page,
							   &quad[0]->u,
							   &quad[0]->v,
							   &quad[1]->u,
							   &quad[1]->v,
							   &quad[2]->u,
							   &quad[2]->v,
							   &quad[3]->u,
							   &quad[3]->v);
					}

					if(page>POLY_NUM_PAGES - 2)
						page=0;

					//
					// Do the shadowing of this quad.
					//

					{
						if (!AENG_drawing_a_warehouse && (p_f4->DrawFlags & (RFACE_FLAG_SHADOW_1 | RFACE_FLAG_SHADOW_2 | RFACE_FLAG_SHADOW_3)))
						{
							//
							// Create the shadow points.
							//

							POLY_Point pshad[4];

							//
							// Create four darkened points.
							// 

							pshad[0] = *(quad[0]);
							pshad[1] = *(quad[1]);
							pshad[2] = *(quad[2]);
							pshad[3] = *(quad[3]);

							//
							// Darken the points.
							//

							for (j = 0; j < 4; j++)
							{
								red   = (pshad[j].colour >> 16) & 0xff;
								green = (pshad[j].colour >>  8) & 0xff;
								blue  = (pshad[j].colour >>  0) & 0xff;

								red   -= 130;
								green -= 130;
								blue  -= 130;

								if (red   < 0) {red   = 0;}
								if (green < 0) {green = 0;}
								if (blue  < 0) {blue  = 0;}

								pshad[j].colour = (red << 16) | (green << 8) | (blue << 0) | 0xff000000;
							}

							ASSERT(FACE_FLAG_SHADOW_1 == 1 << 2);

							switch((p_f4->DrawFlags & (RFACE_FLAG_SHADOW_1|RFACE_FLAG_SHADOW_2|RFACE_FLAG_SHADOW_3)))
							{
								case 0:
									ASSERT(0);	// We shouldn't be doing any of this in this case.
									break;

								case 1:

									tri[0] = &pshad [0];
									tri[1] = quad[1];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = quad[2];

									POLY_add_triangle(tri, page, !warehouse);

									break;
									
								case 2:

									tri[0] = &pshad [0];
									tri[1] = quad[1];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									break;
									
								case 3:

									//pshad[2].colour += 0x00101010;

									tri[0] = quad[0];
									tri[1] = quad[1];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									break;
									
								case 4:

									tri[0] = quad[0];
									tri[1] = quad[1];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = &pshad [3];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									break;
									
								case 5:

									tri[0] = &pshad [0];
									tri[1] = quad[1];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = &pshad [3];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									break;
									
								case 6:

									tri[0] = &pshad [0];
									tri[1] = quad[1];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									break;
									
								case 7:

									tri[0] = quad[0];
									tri[1] = quad[1];
									tri[2] = quad[2];

									POLY_add_triangle(tri, page, !warehouse);

									tri[0] = quad[1];
									tri[1] = &pshad [3];
									tri[2] = &pshad [2];

									POLY_add_triangle(tri, page, !warehouse);

									break;

								default:
									ASSERT(0);
									break;
							}
						}
						else
						{
							if(p_f4->RX&(1<<7))
							{
								tri[0] = quad[0];
								tri[1] = quad[1];
								tri[2] = quad[3];

								POLY_add_triangle(tri, page, !warehouse);

								tri[0] = quad[3];
								tri[1] = quad[2];
								tri[2] = quad[0];

								POLY_add_triangle(tri, page, !warehouse);
							}
							else
							{
								POLY_add_quad(quad, page, !warehouse);
							}
						}
					}
				}
			}
		}
	}
}

//
// when walkable faces where prim_faces
//
void FACET_draw_walkable_old(SLONG build)
{
	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG red;
	SLONG green;
	SLONG blue;

	SLONG page;

	SLONG walkable;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	struct DWalkable *p_walk;
	struct DBuilding *p_dbuilding;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	ASSERT(WITHIN(build, 1, next_dbuilding - 1));

	p_dbuilding = &dbuildings[build];

	//
	// Rotate all the points into the POLY_buffer and all the
	// shadow points in the POLY_shadow.
	//

	for (walkable = p_dbuilding->Walkable; walkable; walkable = p_walk->Next)
	{
		p_walk = &dwalkables[walkable];

		//
		// REJECTION OF WHOLE WALKABLE SET IF ITS ABOVE THE CAMERA?
		//

		if((build!=INDOORS_DBUILDING) || p_walk->StoreyY*256< inside_storeys[INDOORS_INDEX].StoreyY)
		{
			sp = p_walk->StartPoint;
			ep = p_walk->EndPoint;

			POLY_buffer_upto = 0;
			POLY_shadow_upto = 0;

			for (i = sp; i < ep; i++)
			{
				ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

				pp = &POLY_buffer[POLY_buffer_upto++];

				POLY_transform(
					AENG_dx_prim_points[i].X,
					AENG_dx_prim_points[i].Y,
					AENG_dx_prim_points[i].Z,
					pp);

				if (pp->MaybeValid())
				{

#ifndef TARGET_DC
					NIGHT_get_d3d_colour(
						NIGHT_WALKABLE_POINT(i),
					   &pp->colour,
					   &pp->specular);
#endif
					//apply_cloud((SLONG)AENG_dx_prim_points[i].X,(SLONG)AENG_dx_prim_points[i].Y,(SLONG)AENG_dx_prim_points[i].Z,&pp->colour);

					//POLY_fadeout_point(pp);
//					pp->colour|=fade_alpha;
				}
			}

			//
			// The quads.
			//

			for (i = p_walk->StartFace4; i < p_walk->EndFace4; i++)
			{
				p_f4 = &prim_faces4[i];

				p0 = p_f4->Points[0] - sp;
				p1 = p_f4->Points[1] - sp;
				p2 = p_f4->Points[2] - sp;
				p3 = p_f4->Points[3] - sp;
				
				ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

				quad[0] = &POLY_buffer[p0];
				quad[1] = &POLY_buffer[p1];
				quad[2] = &POLY_buffer[p2];
				quad[3] = &POLY_buffer[p3];

				if (POLY_valid_quad(quad))
				{
					#if DRAW_THIS_DEBUG_STUFF

					{
						//
						// Draw the slide-edges
						//

						SLONG ei;
						
						UBYTE point_order[4] = {0, 1, 3, 2};
						
						for (ei = 0; ei < 4; ei++)
						{
							if (p_f4->FaceFlags & (FACE_FLAG_SLIDE_EDGE << ei))
							{
								AENG_world_line(
									prim_points[p_f4->Points[point_order[(ei + 0) & 0x3]]].X,
									prim_points[p_f4->Points[point_order[(ei + 0) & 0x3]]].Y,
									prim_points[p_f4->Points[point_order[(ei + 0) & 0x3]]].Z,
									32,
									0xffffff,										   
									prim_points[p_f4->Points[point_order[(ei + 1) & 0x3]]].X,
									prim_points[p_f4->Points[point_order[(ei + 1) & 0x3]]].Y,
									prim_points[p_f4->Points[point_order[(ei + 1) & 0x3]]].Z,
									32,
									0xffffff,
									TRUE);
							}
						}
					}

					#endif

					if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
					{
						quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
						quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);

						quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
						quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);

						quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
						quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

						quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
						quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

						page   = p_f4->UV[0][0] & 0xc0;
						page <<= 2;
						page  |= p_f4->TexturePage;

						if(page>POLY_NUM_PAGES - 2)
							page=0;

						//
						// Do the shadowing of this quad.
						//

						if (p_f4->FaceFlags & (FACE_FLAG_SHADOW_1 | FACE_FLAG_SHADOW_2 | FACE_FLAG_SHADOW_3))
						{
							//
							// Create the shadow points.
							//

							POLY_Point ps[4];

							//
							// Create four darkened points.
							// 

							ps[0] = *(quad[0]);
							ps[1] = *(quad[1]);
							ps[2] = *(quad[2]);
							ps[3] = *(quad[3]);

							//
							// Darken the points.
							//

							for (j = 0; j < 4; j++)
							{
								red   = (ps[j].colour >> 16) & 0xff;
								green = (ps[j].colour >>  8) & 0xff;
								blue  = (ps[j].colour >>  0) & 0xff;

								red   -= 130;
								green -= 130;
								blue  -= 130;

								if (red   < 0) {red   = 0;}
								if (green < 0) {green = 0;}
								if (blue  < 0) {blue  = 0;}

								ps[j].colour = (red << 16) | (green << 8) | (blue << 0) | 0xff000000;
							}

							ASSERT(FACE_FLAG_SHADOW_1 == 1 << 2);

							switch((p_f4->FaceFlags & (FACE_FLAG_SHADOW_1|FACE_FLAG_SHADOW_2|FACE_FLAG_SHADOW_3)) >> 2)
							{
								case 0:
									ASSERT(0);	// We shouldn't be doing any of this in this case.
									break;

								case 1:

									tri[0] = &ps [0];
									tri[1] = quad[1];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = quad[2];

									POLY_add_triangle(tri, page, TRUE);

									break;
									
								case 2:

									tri[0] = &ps [0];
									tri[1] = quad[1];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									break;
									
								case 3:


									ps[2].colour += 0x00202020;

									tri[0] = quad[0];
									tri[1] = quad[1];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									break;
									
								case 4:

									tri[0] = quad[0];
									tri[1] = quad[1];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = &ps [3];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									break;
									
								case 5:

									tri[0] = &ps [0];
									tri[1] = quad[1];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = &ps [3];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									break;
									
								case 6:

									tri[0] = &ps [0];
									tri[1] = quad[1];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = quad[3];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									break;
									
								case 7:

									tri[0] = quad[0];
									tri[1] = quad[1];
									tri[2] = quad[2];

									POLY_add_triangle(tri, page, TRUE);

									tri[0] = quad[1];
									tri[1] = &ps [3];
									tri[2] = &ps [2];

									POLY_add_triangle(tri, page, TRUE);

									break;

								default:
									ASSERT(0);
									break;
							}
						}
						else
						{
							POLY_add_quad(quad, page, TRUE);
						}
					}
					else
					{
						POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
					}
				}
			}
		}
	}
}




void	DRAW_ladder_rungs(float x1,float z1,float x2,float z2,struct DFacet	*p_facet,float dx,float dz,ULONG colour, ULONG specular)
{
	SLONG	count;
	float	y;
	POLY_Point *quad[4];
	POLY_Point *pp;

	// Do this signed so that the alpha values stay at ff or 00 as appropriate.
	ULONG dcolour = ((signed)colour >> 2) & 0xff3f3f3f;

	//
	// do a height test so just process rungs in height range we can see
	//

	x1 += dx * (3.0f / 4.0f);
	z1 += dz * (3.0f / 4.0f);

	x2 -= dx * (3.0f / 4.0f);
	z2 -= dz * (3.0f / 4.0f);

	y = (float) p_facet->Y[0];

	count = p_facet->Height;

	quad[0] = &POLY_buffer[0];
	quad[1] = &POLY_buffer[1];
	quad[2] = &POLY_buffer[2];
	quad[3] = &POLY_buffer[3];

	while(count--)
	{
		y+=BLOCK_SIZE;

		POLY_buffer_upto=0;
		pp = &POLY_buffer[0];

		POLY_transform(x1,y-8,z1,pp);
		pp->colour   = dcolour;
		pp->specular = specular;
		pp->u        = 0.0F;
		pp->v        = 0.2F;

		//POLY_fadeout_point(pp);

		POLY_transform(x2,y-8,z2,++pp);
		pp->colour   = dcolour;
		pp->specular = specular;
		pp->u        = 2.0F;
		pp->v        = 0.2F;

		//POLY_fadeout_point(pp);

		POLY_transform(x1,y,z1,++pp);
		pp->colour   = colour;
		pp->specular = specular;
		pp->u        = 0.0F;
		pp->v        = 0.8F;

		//POLY_fadeout_point(pp);

		POLY_transform(x2,y,z2,++pp);
		pp->colour   = colour;
		pp->specular = specular;
		pp->u        = 2.0F;
		pp->v        = 0.8F;

		//POLY_fadeout_point(pp);

		if (POLY_valid_quad(quad))
		{
			SLONG	page;
			
			//
			// Texture the quad.
			// 

			page = POLY_PAGE_LADDER;

			POLY_add_quad(quad, page, FALSE); // TRUE means perform a backface cull
		}
	}
}

#define	LADDER_SPINE_WIDTH		12
void	DRAW_ladder_sides(float x1,float z1,float x2,float z2,struct DFacet	*p_facet,float dx,float dz,ULONG colour, ULONG specular)
{
	SLONG	count;
	float	y;
	POLY_Point *quad[4];
	POLY_Point *pp;
	float	height;
	UWORD	sp[64];

	// Do this signed so that the alpha values stay at ff or 00 as appropriate.
	ULONG dcolour = ((signed)colour >> 2) & 0xff3f3f3f;

	float v;

	//
	// do a height test so just process rungs in height range we can see
	//

	y=(float)p_facet->Y[0];
	count=(p_facet->Height*BLOCK_SIZE)/256;
	if(count==0)
		height=256;
	else
		height=(float)((p_facet->Height*BLOCK_SIZE)/count);
	count++;

	{
		float	x1mdz,x1pdx,x2mdx,x2mdz;
		float	z1pdx,z1pdz,z2mdz,z2pdx;
		SLONG	c0=0;

		x1mdz=x1-dz;
		x1pdx=x1+dx;
		x2mdx=x2-dx;
		x2mdz=x2-dz;

		z1pdx=z1+dx;
		z1pdz=z1+dz;
		z2mdz=z2-dz;
		z2pdx=z2+dx;

		POLY_buffer_upto=0;
		pp = &POLY_buffer[0];
		while(c0<count)
		{
			//
			// The 'v' coordinate for this height.
			// 

			v = y * (1.0F / 64.0F);

			sp[c0]=POLY_buffer_upto;

			POLY_transform(x1mdz,y,z1pdx,pp);
			pp->colour   = dcolour;
		    pp->specular = specular;
			pp->u        = 0.0F;
			pp->v        = v;

			//POLY_fadeout_point(pp);

			POLY_transform(x1,y,z1,++pp);
			pp->colour   = colour;
		    pp->specular = specular;
			pp->u        = 0.5F;
			pp->v        = v;

			//POLY_fadeout_point(pp);

			POLY_transform(x1pdx,y,z1pdz,++pp);
			pp->colour   = dcolour;
		    pp->specular = specular;
			pp->u        = 1.0F;
			pp->v        = v;

			//POLY_fadeout_point(pp);

			POLY_transform(x2mdx,y,z2mdz,++pp);
			pp->colour   = dcolour;
		    pp->specular = specular;
			pp->u        = 0.0F;
			pp->v        = v;

			//POLY_fadeout_point(pp);

			POLY_transform(x2,y,z2,++pp);
			pp->colour   = colour;
		    pp->specular = specular;
			pp->u        = 0.5F;
			pp->v        = v;

			//POLY_fadeout_point(pp);

			POLY_transform(x2mdz,y,z2pdx,++pp);
			pp->colour   = dcolour;
		    pp->specular = specular;
			pp->u        = 1.0F;
			pp->v        = v;

			//POLY_fadeout_point(pp);

			pp++;

			POLY_buffer_upto+=6;

			y+=height;

			if(c0>0)
			{
				quad[0] = &POLY_buffer[sp[c0]];
				quad[1] = &POLY_buffer[sp[c0]+1];
				quad[2] = &POLY_buffer[sp[c0-1]];
				quad[3] = &POLY_buffer[sp[c0-1]+1];

				if (POLY_valid_quad(quad))
				{
					SLONG	page;
					
					page = POLY_PAGE_LADDER;

					POLY_add_quad(quad, page, 0); // 1 means perform a backface cull
				}

				quad[0] = &POLY_buffer[sp[c0]+1];
				quad[1] = &POLY_buffer[sp[c0]+2];
				quad[2] = &POLY_buffer[sp[c0-1]+1];
				quad[3] = &POLY_buffer[sp[c0-1]+2];

				if (POLY_valid_quad(quad))
				{
					SLONG	page;

					page = POLY_PAGE_LADDER;

					POLY_add_quad(quad, page, 0); // 1 means perform a backface cull
				}

				quad[0] = &POLY_buffer[sp[c0]+3];
				quad[1] = &POLY_buffer[sp[c0]+4];
				quad[2] = &POLY_buffer[sp[c0-1]+3];
				quad[3] = &POLY_buffer[sp[c0-1]+4];

				if (POLY_valid_quad(quad))
				{
					SLONG	page;
					
					//page = texture_quad(quad,dstyles[0],0,0);
					//page=texture_quad(quad,dstyles[p_facet->StyleIndex],1,5);
					page = POLY_PAGE_LADDER;

					POLY_add_quad(quad, page, 0); // 1 means perform a backface cull
				}

				quad[0] = &POLY_buffer[sp[c0]+4];
				quad[1] = &POLY_buffer[sp[c0]+5];
				quad[2] = &POLY_buffer[sp[c0-1]+4];
				quad[3] = &POLY_buffer[sp[c0-1]+5];

				if (POLY_valid_quad(quad))
				{
					SLONG	page;
					
					//page = texture_quad(quad,dstyles[0],0,0);
					//page=texture_quad(quad,dstyles[p_facet->StyleIndex],1,5);
					page = POLY_PAGE_LADDER;

					POLY_add_quad(quad, page, 0); // 1 means perform a backface cull
				}

			}
			c0++;
		}
	}
}
	
void	DRAW_ladder(struct DFacet *p_facet)
{
	SLONG	dx,dz;
	SLONG	dx3,dz3;
	SLONG	x1,z1,x2,z2;

	ULONG colour;
	ULONG specular;

	//
	// Don't bother drawing the sewer ladders when we're not in the sewers.
	//

	if (p_facet->FacetFlags & FACET_FLAG_LADDER_LINK)
	{
		//
		// We always have to draw these facets.
		//
	}
	else
	{
		if (0 && (p_facet->FacetFlags & FACET_FLAG_IN_SEWERS))
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

	x1=p_facet->x[0] << 8;
	x2=p_facet->x[1] << 8;
	z1=p_facet->z[0] << 8;
	z2=p_facet->z[1] << 8;

	dx=x2-x1;
	dz=z2-z1;

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

	NIGHT_Colour col = NIGHT_get_light_at(
						((x1 + x2) >> 1) + (dz << 3),
						p_facet->Y[0]+ p_facet->Y[1] >> 1,
						((z1 + z2) >> 1) - (dz << 3));

	NIGHT_get_d3d_colour(
		col,
	   &colour,
	   &specular);

	colour |= 0x3f3f3f;	// Always have a bit of colour!

	DRAW_ladder_rungs((float)x1,(float)z1,(float)x2,(float)z2,p_facet,(float)dx,(float)dz,colour,specular);
	DRAW_ladder_sides((float)x1,(float)z1,(float)x2,(float)z2,p_facet,(float)dx,(float)dz,colour,specular);

	//
	// Draw the ladder shadow...
	//

	SLONG bx;
	SLONG bz;

	bx = (x1 + x2 - (dz << 3) >> 9);
	bz = (z1 + z2 + (dx << 3) >> 9);

	if (!WITHIN(bx, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(bz, 0, PAP_SIZE_HI - 1))
	{
		return;
	}

	if (PAP_2HI(bx,bz).Flags & PAP_FLAG_HIDDEN)
	{
		float height = p_facet->Height * BLOCK_SIZE;

		POLY_Point  pp  [4];
		POLY_Point *quad[4];

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		dx >>= 3;
		dz >>= 3;

		POLY_transform(
			(p_facet->x[0] << 8) + dz,
			(p_facet->Y[0]),
			(p_facet->z[0] << 8) - dx,
		   &pp[0]);

		POLY_transform(
			(p_facet->x[1] << 8) + dz,
			(p_facet->Y[0]),	         
			(p_facet->z[1] << 8) - dx,
		   &pp[1]);

		POLY_transform(
			(p_facet->x[0] << 8) + dz,
			(p_facet->Y[0]) + height,
			(p_facet->z[0] << 8) - dx,
		   &pp[2]);

		POLY_transform(
			(p_facet->x[1] << 8) + dz,
			(p_facet->Y[0]) + height,
			(p_facet->z[1] << 8) - dx,
		   &pp[3]);

		if (POLY_valid_quad(quad))
		{
			float top_v = p_facet->Height;

			pp[0].colour   = 0xffffffff;
			pp[0].specular = 0xff000000;
			pp[0].u        = 0.0F;
			pp[0].v        = 0.0F;

			pp[1].colour   = 0xffffffff;
			pp[1].specular = 0xff000000;
			pp[1].u        = 1.0F;
			pp[1].v        = 0.0F;

			pp[2].colour   = 0xffffffff;
			pp[2].specular = 0xff000000;
			pp[2].u        = 0.0F;
			pp[2].v        = top_v;

			pp[3].colour   = 0xffffffff;
			pp[3].specular = 0xff000000;
			pp[3].u        = 1.0F;
			pp[3].v        = top_v;

			POLY_add_quad(quad, POLY_PAGE_LADSHAD, TRUE);
		}
	}
}


#ifdef EDITOR

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

	y  = ES_hi[mx][mz].height << 5;
	y += -32 * 0x100; 

	//
	// Create a pretend facet for the ladder facet function to use.
	//

	df.FacetType  = STOREY_TYPE_LADDER;
	df.FacetFlags = FACET_FLAG_IN_SEWERS;
	df.x[0]       = x1;
	df.z[0]       = z1;
	df.x[1]       = x2;
	df.z[1]       = z2;
	df.Y[0]       = y;
	df.Y[1]       = y;
	df.Height     = height;

	//
	// Draw a ladder facet.
	//

	DRAW_ladder(&df);
}

#endif






void FACET_project_crinkled_shadow(SLONG facet)
{
	SLONG i;
	SLONG page;
	SLONG style_index;

	//
	// Our facet.
	//

	DFacet *p_facet;

	ASSERT(WITHIN(facet, 1, next_dfacet - 1));
	
	p_facet = &dfacets[facet];

	//
	// Ignore double-sided facets.
	//

	if ((p_facet->FacetFlags & FACET_FLAG_2SIDED) || p_facet->FacetType == STOREY_TYPE_OINSIDE)
	{
		return;
	}

	//
	// Everything we need to work out the world positions of all the facet points.
	//

	SLONG dx = p_facet->x[1] - p_facet->x[0];
	SLONG dz = p_facet->z[1] - p_facet->z[0];

	float sx = float(p_facet->x[0] << 8);
	float sy = float(p_facet->Y[0]     );
	float sz = float(p_facet->z[0] << 8);

	float fdx = SIGN(dx) * 256.0F;
	float fdz = SIGN(dz) * 256.0F;

	float block_height = float(p_facet->BlockHeight << 4);
	SLONG height       = p_facet->Height;
	SLONG max_height   = INFINITY;

	NIGHT_Colour *col = NULL;	// No cached lighting needed!
	
	SLONG foundation = (p_facet->FHeight) ? 2 : 0;

	SLONG count = abs(dx) + abs(dz);

	//
	// Work out the texturing.
	//

	style_index = p_facet->StyleIndex;

	set_facet_seed(p_facet->x[0] * p_facet->z[0] + p_facet->Y[0]);

	//
	// Where we put the points of the facet in.
	//

	#define MAX_FACET_POINTS 512
	#define MAX_FACET_ROWS   32

	SVector_F  facet_point[MAX_FACET_POINTS];
	SLONG      facet_point_upto;

	SVector_F *facet_row[MAX_FACET_ROWS];
	SLONG      facet_row_upto;

	SVector_F *sv;

	//
	// This is code taken from MakeFacetPoints and changed slightly...
	//

	facet_row_upto   = 0;
	facet_point_upto = 0;

	float x;
	float y;
	float z;

	y = sy;

	while(height >= 0)
	{
		x = sx;
		z = sz;

		ASSERT(WITHIN(facet_row_upto, 0, MAX_FACET_ROWS - 1));

		facet_row[facet_row_upto] = &facet_point[facet_point_upto];

		for (i = 0; i <= count; i++)
		{
			sv = &facet_point[facet_point_upto++];

			//
			// The y coordinate we actually use...
			//

			float ty;

			if (foundation == 2)
			{
				ty = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);
			}
			else
			{
				ty = y;
			}

			//
			// Store this point.
			//

			sv->X = x;
			sv->Y = ty;
			sv->Z = z;

			x += fdx;
			z += fdz;
		}

		y              += block_height;
		height         -= 4;
		facet_row_upto += 1;
		foundation     -= 1;
	}

	//
	// Now we have all our points, we can go through each face in turn.
	//

	SLONG base_row;
	SVector_F  poly[4];

	height     = p_facet->Height;
	foundation = (p_facet->FHeight) ? 2 : 0;

	//
	// Start from the second row!
	//

	base_row  = 0;
	height   -= 4;

	while(height >= 0)
	{
		{
			UBYTE rflip;

			ASSERT(WITHIN(base_row,     0, facet_row_upto - 1));
			ASSERT(WITHIN(base_row + 1, 0, facet_row_upto - 1));

			SVector_F *row1 = facet_row[base_row    ];
			SVector_F *row2 = facet_row[base_row + 1];

			for (i = 0; i < count; i++)
			{
				poly[0] = row2[1];
				poly[1] = row2[0];
				poly[2] = row1[1];
				poly[3] = row1[0];

				row1 += 1;
				row2 += 1;

				page = get_texture_page(dstyles[style_index], i, count, &rflip);

				//
				// Add crinkle?
				//

				SMAP_Link *sl = SMAP_project_onto_poly(poly, 4);

				if (sl)
				{
					extern int AENG_detail_crinkles;

					if (AENG_detail_crinkles)
					{
						if (page < 64 * 8)
						{
							if (TEXTURE_crinkle[page])
							{
								POLY_Point pp;

								POLY_transform(
									poly[0].X,
									poly[0].Y,
									poly[0].Z,
								   &pp);

								//
								// This quad could be crinkled!
								//

								if (pp.z > 0.6F)
								{
									//
									// Too far away to be crinkled.
									//
								}
								else
								if (pp.z < 0.3F)
								{
									//
									// Maximum crinkleyness!
									//

									CRINKLE_project(
										TEXTURE_crinkle[page],
										1.0F,
										poly,
										rflip);

									goto added_crinkle;
								}
								else
								{
									float extrude;
									float av_z;

									//
									// Intermediate crinkle extrusion.
									//

									av_z = pp.z;

									extrude  = av_z - 0.5F;
									extrude *= 1.0F / (0.4F - 0.5F);

									if (extrude > 0.0F)
									{
										if (extrude > 1.0F)
										{
											extrude = 1.0F;
										}

										CRINKLE_project(
											TEXTURE_crinkle[page],
											extrude,
											poly,
											rflip);

										goto added_crinkle;
									}
								}
							}
						}
					}

				  added_crinkle:;
				}
			}
		}

		foundation  -= 1;
		height      -= 4;
		i           += 1;
		style_index += 1;
		base_row    += 1;
	}
}




