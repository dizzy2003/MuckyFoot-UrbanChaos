//
// Another engine.
//

#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "game.h"
#include "aeng.h"
#include "font.h"
#include "ngamut.h"
#include "poly.h"
#include "texture.h"
#include "matrix.h"
#include "message.h"
#include "font2d.h"
#include "figure.h"
#include "build.h"
#include "mesh.h"
#include "dirt.h"
#include "sky.h"
#include "mist.h"
#include "id.h"
#include "shadow.h"
#include "puddle.h"
#include "az.h"
#include "aa.h"
#include "smap.h"
#include "sewer.h"
#include "drip.h"
#include "wibble.h"
#include "shape.h"
#include "bang.h"
#include "mav.h"
#include "fire.h"
#include "animtmap.h"
#include "sprite.h"
#include "spark.h"
#include "glitter.h"
#include	"Text.h"	//	Guy.
#include "cone.h"
#include "ob.h"
#include "morph.h"
#include "trip.h"
#include "text.h"
#include "pap.h"
#include "night.h"
#include "fallen/headers/supermap.h"
#include "hook.h"
#include "sm.h"
#include "ns.h"
#include "cloth.h"
#include "facet.h"
//#include "fallen/sedit/headers/es.h"
#include "ic.h"
#include "comp.h"
#include "cam.h"
#include "fallen/headers/tracks.h"
#include "pcom.h"
#include "drawxtra.h"
#include "balloon.h"
#include "snipe.h"
#include "fallen/headers/inside2.h"
#include "psystem.h"
#include "fc.h"
#include "memory.h"
#include "ware.h"
#include "statedef.h"
#include "pow.h"
#include "FMatrix.h"
#include "eway.h"
#include "env.h"
#include "animate.h"
#include "oval.h"
#include "crinkle.h"
#include "sw.h"
#include "fallen/headers/sound.h"

#include "vertexbuffer.h"

#include "BreakTimer.h"
#include "polypoint.h"

#include "grenade.h"
#include "superfacet.h"
#include "farfacet.h"

#include "interfac.h"


#include "polypage.h"
#include "DCLowLevel.h"

#ifdef TARGET_DC
#include <shsgintr.h>
#else
#define	POLY_set_local_rotation_none() {}

#endif


#define AENG_MAX_BBOXES		8
#define AENG_BBOX_PUSH_IN	16
#define AENG_BBOX_PUSH_OUT	4




//#ifdef DEBUG
#if 0
#define ANNOYINGSCRIBBLECHECK ScribbleCheck()

static void ScribbleCheck ( void )
{
	ASSERT ( prim_faces4[1].Points[0] >= 48 );
	ASSERT ( prim_faces4[1].Points[0] < 62 );
	ASSERT ( prim_faces4[1].Points[1] >= 48 );
	ASSERT ( prim_faces4[1].Points[1] < 62 );
	ASSERT ( prim_faces4[1].Points[2] >= 48 );
	ASSERT ( prim_faces4[1].Points[2] < 62 );
	ASSERT ( prim_faces4[1].Points[3] >= 48 );
	ASSERT ( prim_faces4[1].Points[3] < 62 );
}

#else
#define ANNOYINGSCRIBBLECHECK
#endif



#ifndef TARGET_DC

#define LOG_ENTER(x) {}
#define LOG_EXIT(x)  {}
#define LOG_EVENT(x) {}

#endif



#ifdef TARGET_DC


#define AENG_rdtsc() 0
#else
ULONG AENG_rdtsc()
{
	ULONG hi;
	ULONG lo;

	_asm
	{
		rdtsc
		mov		hi, edx
		mov		lo, eax
	}

	ULONG ans;

	ans  = lo >> 16;
	ans |= hi << 16;
	ans  = lo;

	return ans;
}
#endif





void	AENG_draw_far_facets(void);
void AENG_draw_box_around_recessed_door(DFacet *df, SLONG inside_out);
void AENG_get_rid_of_unused_dfcache_lighting(SLONG splitscreen);
void	AENG_draw_inside_floor(UWORD inside_index,UWORD inside_room,UBYTE fade);

UBYTE	aeng_draw_cloud_flag = 1;
UWORD	light_inside=0;
UWORD	fade_black=1;
#ifdef TARGET_DC
#define sw_hack FALSE
#else
UBYTE   sw_hack;
#endif

#ifndef TARGET_DC
UBYTE	cloud_data[32][32];
SLONG	cloud_x,cloud_z;
#endif

extern SLONG FirstPersonMode;
SLONG	FirstPersonAlpha = 255;
#define	MAX_FPM_ALPHA	160



int AENG_total_polys_drawn;

int	AENG_detail_stars = 1;
int	AENG_detail_shadows = 1;
int AENG_detail_moon_reflection = 1;
int AENG_detail_people_reflection = 1;
int	AENG_detail_puddles = 1;
int	AENG_detail_dirt = 1;
int AENG_detail_mist = 1;
int AENG_detail_rain = 1;
int AENG_detail_skyline = 1;
int AENG_detail_filter = 1;
int AENG_detail_perspective = 1;
int AENG_detail_crinkles = 1;
#ifndef TARGET_DC
int AENG_estimate_detail_levels = 1;
#endif





SLONG AENG_cur_fc_cam;


#ifndef TARGET_DC
// Clouds?!?!?!?!?!?!? Madness.

void	move_clouds(void)
{
	cloud_x+=10;
	cloud_z+=5;
}

SLONG	global_b=0;

//
// calc a cloud shadow value for a world co-ord for reapeated use by prim or person or ...
//
void	calc_global_cloud(SLONG x,SLONG y,SLONG z)
{
	SLONG	in,r,g,b;
	SLONG	in1,in2,in3,in0;
	SLONG	dx,dz,mx,mz;
	if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
		return;
	y>>=1;

	x=(x+(cloud_x)-y);//&0x1f;
	z=(z+(cloud_z)-y);//&0x1f;

	mx=(x>>8)&0x1f;
	mz=(z>>8)&0x1f;

	in0=cloud_data[mx][mz];
	in1=cloud_data[(mx+1)&0x1f][mz];
	in2=cloud_data[(mx)&0x1f][(mz+1)&0x1f];
	in3=cloud_data[(mx+1)&0x1f][(mz+1)&0x1f];


	//   in0    in1
	//
	//
	//   in2    in3

	dx=x&0xff;
	dz=z&0xff;

	if(dx+dz<256)
	{
		in=in0;
		in+=((in1-in0)*dx)>>8;
		in+=((in2-in0)*dz)>>8;
	}
	else
	{
		in=in3;
		in+=((in2-in3)*(256-dx))>>8;
		in+=((in1-in3)*(256-dz))>>8;

	}
	if(in<0)
		in=0;
	global_b=in;

}

//
// use pre-calced shadow value
//
void	use_global_cloud(ULONG *col)
{
	SLONG	r,g,b;
	SLONG	in=global_b;

    if (!aeng_draw_cloud_flag) return;

	if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
		return;
	r=((*col)&0xff0000)>>16;

	in=(in*(r))>>8;


	{
		r-=in;
		if(r<0)
			r=0;
	}

	g=((*col)&0xff00)>>8;
//	if(g-global_bright>Mglobal_bright_CLOUD)
	{
		g-=in;
		if(g<0)
			g=0;
	}

	b=((*col)&0xff);

//	if(b-global_bright>Mglobal_bright_CLOUD)
	{
		b-=in;//(global_bright*220)>>8;
		if(b<0)
			b=0;
	}


	*col=(*col&0xff000000)|(r<<16)|(g<<8)|(b);

}

//
// Take a co-ord in the world, calcultae how much shadow the clouds are producing at that point
// and darken the colour accordingly


#define	MIN_CLOUD	48
inline void	apply_cloud(SLONG x,SLONG y,SLONG z,ULONG *col)
{
	return;

    if (!aeng_draw_cloud_flag) return;

	SLONG	in,r,g,b;
	SLONG	in1,in2,in3,in0;
	SLONG	dx,dz,mx,mz;
	if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
		return;
	y>>=1;

	x=(x+(cloud_x)-y);//&0x1f;
	z=(z+(cloud_z)-y);//&0x1f;

	mx=(x>>8)&0x1f;
	mz=(z>>8)&0x1f;

	in0=cloud_data[mx][mz];
	in1=cloud_data[(mx+1)&0x1f][mz];
	in2=cloud_data[(mx)&0x1f][(mz+1)&0x1f];
	in3=cloud_data[(mx+1)&0x1f][(mz+1)&0x1f];


	//   in0    in1
	//
	//
	//   in2    in3

	dx=x&0xff;
	dz=z&0xff;

	if(dx+dz<256)
	{
		in=in0;
		in+=((in1-in0)*dx)>>8;
		in+=((in2-in0)*dz)>>8;
	}
	else
	{
		in=in3;
		in+=((in2-in3)*(256-dx))>>8;
		in+=((in1-in3)*(256-dz))>>8;

	}
	if(in<0)
		in=0;


	r=((*col)&0xff0000)>>16;

	//
	// use the redness that's there as a general brightness factor for the vertex
	//

	//
	// adjust the shadow factor so dark things only get affected a little, & light things get affected a lot
	//

	in=(in*(r))>>8;

	{
		r-=in;
		if(r<0)
			r=0;
//		if(r>255)
//			r=255;
	}

	g=((*col)&0xff00)>>8;
//	if(g-in>MIN_CLOUD)
	{
		g-=in;
		if(g<0)
			g=0;
//		if(g>255)
//			g=255;
	}

	b=((*col)&0xff);

//	if(b-in>MIN_CLOUD)
	{
		b-=in;//(in*220)>>8;
		if(b<0)
			b=0;
//		if(b>255)
//			b=255;
	}

	*col=(*col&0xff000000)|(r<<16)|(g<<8)|(b);


}

void	init_clouds(void)
{
	MFFileHandle	handle;
	handle=FileOpen("data\\cloud.raw");
	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,cloud_data,1024);
		FileClose(handle);
	}
	else
	{
		memset(cloud_data,0,1024);
	}
}

#endif //#ifndef TARGET_DC

// GetShadowPixelMapping
//
// returns a UBYTE -> UWORD table for mapping
// from the shadow buffer to the shadow texture surface

UWORD*	GetShadowPixelMapping()
{
	static UWORD	mapping[256];
	static int		mapping_state = -1;

	//
	// create mapping, if necessary
	//

	if ((mapping_state == -1) || (mapping_state != (int)the_display.GetDeviceInfo()->DestInvSourceColourSupported()))
	{
		// mapping must change
		mapping_state = the_display.GetDeviceInfo()->DestInvSourceColourSupported();

		if (mapping_state)
		{
			// density colourmap
			for (int ii = 0; ii < 256; ii++)
			{
				int val = ii >> 1;	// not too shadowy

				mapping[ii] = ((val >> TEXTURE_shadow_mask_red) << TEXTURE_shadow_shift_red)
							| ((val >> TEXTURE_shadow_mask_green) << TEXTURE_shadow_shift_green)
							| ((val >> TEXTURE_shadow_mask_blue) << TEXTURE_shadow_shift_blue);
			}
		}
		else
		{
			// density alphamap
			for (int ii = 0; ii < 256; ii++)
			{
				int	val = ii >> 1;	// not too shadowy

				mapping[ii] = (val >> TEXTURE_shadow_mask_alpha) << TEXTURE_shadow_shift_alpha;
			}
		}
	}

	return mapping;
}

extern SLONG dfacets_drawn_this_gameturn;
extern BOOL allow_debug_keys;

//
// The shift of the floor...
//

#define ALT_SHIFT (3)

//
// The maximum draw distance.
//

float AENG_lens = 4.0F;

//static SLONG	NormalDrawDistance = 22<<8;
SLONG	CurDrawDistance = 22<<8;

//#define AENG_DRAW_DIST ((FC_cam[1].focus) ? 16 : 22)
#define AENG_DRAW_DIST			(CurDrawDistance>>8)
#define AENG_DRAW_DIST_PRECISE	(CurDrawDistance)
#define	AENG_DRAW_PEOPLE_DIST	(CurDrawDistance + 128)

#define AENG_LENS		(AENG_lens)

SLONG AENG_get_draw_distance()
{
	return CurDrawDistance >> 8;
}

void AENG_set_draw_distance(SLONG dist)
{
#if 0
	NormalDrawDistance = dist;
	ENV_set_value_number("draw_distance", dist, "Render");
#endif
}

//
// The camera.
//

float AENG_cam_x;
float AENG_cam_y;
float AENG_cam_z;

float AENG_cam_yaw;
float AENG_cam_pitch;
float AENG_cam_roll;

float AENG_cam_matrix[9];

SLONG AENG_cam_vec[3];

//
// The floating point prim points.
//

SVector_F AENG_dx_prim_points[RMAX_PRIM_POINTS];

struct	StoreLine
{
	SLONG	x1,y1,z1,x2,y2,z2;
	ULONG	col;
};

#define	MAX_LINES	100
struct	StoreLine	Lines[MAX_LINES];
SLONG	next_line=0;


void	add_debug_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG colour)
{
	SLONG	line;

	line=next_line%MAX_LINES;

	Lines[line].x1=x1;
	Lines[line].y1=y1;
	Lines[line].z1=z1;

	Lines[line].x2=x2;
	Lines[line].y2=y2;
	Lines[line].z2=z2;
	Lines[line].col=colour;
	next_line++;

}

#ifdef DEBUG
void	draw_debug_lines(void)
{
   SLONG	c0,s,e;
   if((!ControlFlag)||(!allow_debug_keys)) return;

   s=next_line-MAX_LINES;
   if(s<0)
	   s=0;
   e=next_line;


   for(c0=s;c0<e;c0++)
   {
	   SLONG	index;

	   index=c0%MAX_LINES;

		AENG_world_line(Lines[index].x1,Lines[index].y1,Lines[index].z1,6,Lines[index].col,
						Lines[index].x2,Lines[index].y2,Lines[index].z2,1,Lines[index].col,1);
   }
}
#endif
//
// The sewer pages.
//

SLONG AENG_sewer_page[SEWER_PAGE_NUMBER] =
{
	3,
	4,
	5,
	POLY_PAGE_WATER
};

//
// Swaps two frames...
//

#define SWAP_FRAME(a,b) {COMP_Frame *spare; spare = (a); (a) = (b); (b) = spare;}


// ========================================================
//
// MOVIE STUFF
//
// ========================================================

#ifdef TARGET_DC
#define AENG_MAX_MOVIE_DATA (128 * 1024)
#else
#define AENG_MAX_MOVIE_DATA (512 * 1024)
#endif

UBYTE       AENG_movie_data[AENG_MAX_MOVIE_DATA];
UBYTE      *AENG_movie_upto;
COMP_Frame  AENG_frame_one;
COMP_Frame  AENG_frame_two;
COMP_Frame *AENG_frame_last = &AENG_frame_one;
COMP_Frame *AENG_frame_next = &AENG_frame_two;
SLONG       AENG_frame_count;
SLONG       AENG_frame_tick;
SLONG       AENG_frame_number;

//
// Initialises the movie stuff.
//

void AENG_movie_init()
{
	SLONG bytes_read;

	FILE *handle;

	//
	// Load the movie in.
	//

	handle = MF_Fopen("movie\\bond.mmv", "rb");

	if (!handle)
	{
		goto file_error;
	}

	bytes_read = fread(AENG_movie_data, sizeof(UBYTE), AENG_MAX_MOVIE_DATA, handle);
	ASSERT ( bytes_read < AENG_MAX_MOVIE_DATA );

	AENG_frame_last   = &AENG_frame_one;
	AENG_frame_next   = &AENG_frame_two;
	AENG_frame_count  = 0;
	AENG_frame_tick   = 0;
	AENG_frame_number = 200;
	AENG_movie_upto   = AENG_movie_data;

	return;

  file_error:;

	AENG_frame_last   = NULL;
	AENG_frame_next   = NULL;
	AENG_frame_count  = 0;
	AENG_frame_tick   = 0;
	AENG_frame_number = 0;

	return;
}

//
// Updates the movie texture page.
//

void AENG_movie_update()
{
		return;
	if (AENG_frame_number == 0)
	{
		//
		// No movie!
		//

		return;
	}

	COMP_Delta *cd;

	AENG_frame_tick += TICK_RATIO >> 1;

	if (AENG_frame_tick >= (1 << TICK_SHIFT))
	{
		AENG_frame_tick -= (1 << TICK_SHIFT);
		AENG_frame_tick &= (1 << TICK_SHIFT) - 1;

		AENG_frame_count += 1;

		if (AENG_frame_count >= AENG_frame_number)
		{
			if (AENG_frame_count < AENG_frame_number + 32)
			{
				//
				// Wait a while at the end of the movie.
				//

				return;
			}

			AENG_frame_count = 0;
			AENG_movie_upto  = AENG_movie_data;
		}

		//
		// Load in the new frame.
		//

		cd = (COMP_Delta *) AENG_movie_upto;

		COMP_decomp(
			AENG_frame_last,
			cd,
			AENG_frame_next);

		AENG_movie_upto += cd->size + 4;

		//
		// Download the new frame.
		//

		ASSERT(TEXTURE_VIDEO_SIZE == COMP_SIZE);

		if (TEXTURE_86_lock())
		{
			SLONG x;
			SLONG y;

			UWORD *data;
			UWORD  pixel;

			TGA_Pixel *tp;

			for (y = 0; y < TEXTURE_VIDEO_SIZE; y++)
			{
				tp = &AENG_frame_next->p[y][0];

				data  = TEXTURE_shadow_bitmap;
				data += y * (TEXTURE_shadow_pitch >> 1);

				for (x = 0; x < TEXTURE_VIDEO_SIZE; x++)
				{
					pixel  = 0;
					pixel |= (tp->red   >> TEXTURE_shadow_mask_red  ) << TEXTURE_shadow_shift_red;
					pixel |= (tp->green >> TEXTURE_shadow_mask_green) << TEXTURE_shadow_shift_green;
					pixel |= (tp->blue  >> TEXTURE_shadow_mask_blue ) << TEXTURE_shadow_shift_blue;

					*data = pixel;

					tp   += 1;
					data += 1;
				}
			}

			TEXTURE_86_unlock();
			TEXTURE_86_update();
		}

		SWAP_FRAME(AENG_frame_last, AENG_frame_next);
	}
}




/*

COMP_Frame cf1;
COMP_Frame cf2;
COMP_Frame cf3;
COMP_Frame cf4;

#define MAX_MOVIE_DATA (1024 * 512)

UBYTE  movie_data[MAX_MOVIE_DATA];
UBYTE *movie_data_upto;

*/

void AENG_init(void)
{
	extern void this_may_well_be_the_last_ever_function_call_put_into_the_game(void);

	this_may_well_be_the_last_ever_function_call_put_into_the_game();


	MESH_init();
//	FONT2D_init();
#ifndef TARGET_DC
	init_clouds();
#endif
	SKY_init("stars\\poo");
	POLY_init();
	AENG_movie_init();
	TEXTURE_choose_set(1);
	INDOORS_INDEX_FADE=255;
	//POLY_frame_init(FALSE, FALSE);

	#if 0

	//
	// Create the movie.
	//

	{
		SLONG i;

		COMP_Frame *curr = &cf1;
		COMP_Frame *last = &cf2;
		COMP_Frame *next = &cf3;

		COMP_Delta *cd;

		SLONG load_ok;

		CBYTE name[256];

		movie_data_upto = movie_data;

		for (i = 1; i <= 200; i++)
		{
			sprintf(name, "movie\\frames\\cin1%03d.tga", i);

			load_ok = COMP_load(name, curr);

			ASSERT(load_ok);

			cd = COMP_calc(last, curr, next);

			ASSERT(movie_data_upto + (cd->size + 4) <= &movie_data[MAX_MOVIE_DATA]);

			memcpy(movie_data_upto, cd, cd->size + 4);

			movie_data_upto += cd->size + 4;

			sprintf(name, "movie\\comp\\comp%03d.tga", i);

			TGA_save(
				name,
				COMP_SIZE,
				COMP_SIZE,
				(TGA_Pixel *) next->p,
				FALSE);

			SWAP_FRAME(last,next);
		}
	}

	/*

	SLONG cf1_ok = COMP_load("movie\\frames\\cin1040.tga", &cf1);
	SLONG cf2_ok = COMP_load("movie\\frames\\cin1041.tga", &cf2);

	COMP_Delta *cd = COMP_calc(&cf1, &cf2, &cf3);

	COMP_decomp(&cf1, cd, &cf4);

	TGA_save(
		"movie\\test1.tga",
		COMP_SIZE,
		COMP_SIZE,
		(TGA_Pixel *) cf1.p,
		FALSE);

	TGA_save(
		"movie\\test2.tga",
		COMP_SIZE,
		COMP_SIZE,
		(TGA_Pixel *) cf2.p,
		FALSE);

	TGA_save(
		"movie\\test3.tga",
		COMP_SIZE,
		COMP_SIZE,
		(TGA_Pixel *) cf3.p,
		FALSE);

	TGA_save(
		"movie\\test4.tga",
		COMP_SIZE,
		COMP_SIZE,
		(TGA_Pixel *) cf4.p,
		FALSE);

	IC_test();

	*/

	SLONG num_bytes;

	num_bytes = movie_data_upto - movie_data;

	FILE *handle = MF_Fopen("movie\\bond.mmv", "wb");

	if (handle)
	{
		fwrite(movie_data, sizeof(UBYTE), num_bytes, handle);
		MF_Fclose(handle);
	}

	exit(0);

	#endif

	//
	// Load the fade palette for the bonfires
	//

	init_flames();

}

void AENG_fini()
{
	TEXTURE_free();
}

void AENG_create_dx_prim_points()
{
	SLONG i;

	for (i = 0; i < MAX_PRIM_POINTS; i++)
	{
		AENG_dx_prim_points[i].X = float(prim_points[i].X);
		AENG_dx_prim_points[i].Y = float(prim_points[i].Y);
		AENG_dx_prim_points[i].Z = float(prim_points[i].Z);
	}
}

void AENG_world_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1,
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front)
{

#ifdef DEBUG
#ifdef TARGET_DC
	ASSERT ( FALSE );
	return;
#endif
#else

	POLY_Point p1;
	POLY_Point p2;

	POLY_transform(float(x1), float(y1), float(z1), &p1);
	POLY_transform(float(x2), float(y2), float(z2), &p2);

	if (POLY_valid_line(&p1, &p2))
	{
		p1.colour   = colour1;
		p1.specular = 0xff000000;

		p2.colour   = colour2;
		p2.specular = 0xff000000;

		POLY_add_line(&p1, &p2, float(width1), float(width2), POLY_PAGE_COLOUR, sort_to_front);
	}
#endif
}



void AENG_world_line_nondebug (
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1,
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front)
{

	POLY_Point p1;
	POLY_Point p2;

	POLY_transform(float(x1), float(y1), float(z1), &p1);
	POLY_transform(float(x2), float(y2), float(z2), &p2);

	if (POLY_valid_line(&p1, &p2))
	{
		p1.colour   = colour1;
		p1.specular = 0xff000000;

		p2.colour   = colour2;
		p2.specular = 0xff000000;

		POLY_add_line(&p1, &p2, float(width1), float(width2), POLY_PAGE_COLOUR, sort_to_front);
	}
}



void AENG_world_line_infinite(
		SLONG ix1, SLONG iy1, SLONG iz1, SLONG iwidth1, ULONG colour1,
		SLONG ix2, SLONG iy2, SLONG iz2, SLONG iwidth2, ULONG colour2,
		SLONG sort_to_front)
{

#ifdef TARGET_DC
	ASSERT ( FALSE );
	return;

#else


	float x1 = float(ix1);
	float y1 = float(iy1);
	float z1 = float(iz1);
	float w1 = float(iwidth1);
	float r1 = float((colour1 >> 16) & 0xff);
	float g1 = float((colour1 >>  8) & 0xff);
	float b1 = float((colour1 >>  0) & 0xff);

	float x2 = float(ix2);
	float y2 = float(iy2);
	float z2 = float(iz2);
	float w2 = float(iwidth2);
	float r2 = float((colour2 >> 16) & 0xff);
	float g2 = float((colour2 >>  8) & 0xff);
	float b2 = float((colour2 >>  0) & 0xff);

	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;
	float dr = r2 - r1;
	float dg = g2 - g1;
	float db = b2 - b1;
	float dw = w2 - w1;

	float dist      = sqrt(dx*dx + dy*dy + dz*dz);
	float steps     = (float)floor(dist * (1.0F / 1024.0F)) + 1.0F;
	float oversteps = 1.0F / steps;

	float x = x1;
	float y = y1;
	float z = z1;
	float w = w1;
	float r = r1;
	float g = g1;
	float b = b1;

	dx *= oversteps;
	dy *= oversteps;
	dz *= oversteps;
	dw *= oversteps;
	dr *= oversteps;
	dg *= oversteps;
	db *= oversteps;

	float f;

	for (f = 0.0F; f < steps; f += 1.0F)
	{
		colour1 = (SLONG(r     ) << 16) | (SLONG(g     ) << 8) | (SLONG(b     ) << 0);
		colour2 = (SLONG(r + dr) << 16) | (SLONG(g + dg) << 8) | (SLONG(b + db) << 0);

		AENG_world_line(
			SLONG(x),
			SLONG(y),
			SLONG(z),
			SLONG(w),
			colour1,
			SLONG(x + dx),
			SLONG(y + dy),
			SLONG(z + dz),
			SLONG(w + dw),
			colour2,
			sort_to_front);

		x += dx;
		y += dy;
		z += dz;
		w += dw;
		r += dr;
		g += dg;
		b += db;
	}
#endif
}


struct
{
	float x;
	float y;
	float z;

} AENG_cone[5];

void AENG_calc_gamut(
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float draw_dist,
		float lens)
{
	float width;
	float height;
	float depth;
	float aspect;
	float matrix[9];

	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	//
	// The dimensions of the view pyramid.
	//

	width  = draw_dist;
	height = draw_dist;
	depth  = draw_dist;

	width *= POLY_screen_width;
	width /= POLY_screen_height;

	if (FC_cam[1].focus)
	{
		//
		// We are in splitscreen mode.
		//

		width *= 2.0F;
	}

	width  /= lens;
	height /= lens;

	//
	// Finds the points of the cone in world space
	//

	AENG_cone[3].x = AENG_cone[4].x = x / 256.0f;
	AENG_cone[3].y = AENG_cone[4].y = y / 256.0f;
	AENG_cone[3].z = AENG_cone[4].z = z / 256.0f;

	AENG_cone[3].x += depth * matrix[6];
	AENG_cone[3].y += depth * matrix[7];
	AENG_cone[3].z += depth * matrix[8];

	//
	// AENG_cone[0] is the top left corner...
	//

	AENG_cone[0].x = AENG_cone[3].x + height * matrix[3];
	AENG_cone[0].y = AENG_cone[3].y + height * matrix[4];
	AENG_cone[0].z = AENG_cone[3].z + height * matrix[5];

	AENG_cone[0].x = AENG_cone[0].x - width *  matrix[0];
	AENG_cone[0].y = AENG_cone[0].y - width *  matrix[1];
	AENG_cone[0].z = AENG_cone[0].z - width *  matrix[2];

	//
	// AENG_cone[1] is the top right corner...
	//

	AENG_cone[1].x = AENG_cone[3].x + height * matrix[3];
	AENG_cone[1].y = AENG_cone[3].y + height * matrix[4];
	AENG_cone[1].z = AENG_cone[3].z + height * matrix[5];

	AENG_cone[1].x = AENG_cone[1].x + width *  matrix[0];
	AENG_cone[1].y = AENG_cone[1].y + width *  matrix[1];
	AENG_cone[1].z = AENG_cone[1].z + width *  matrix[2];

	//
	// AENG_cone[2] is the bottom right corner...
	//

	AENG_cone[2].x = AENG_cone[3].x - height * matrix[3];
	AENG_cone[2].y = AENG_cone[3].y - height * matrix[4];
	AENG_cone[2].z = AENG_cone[3].z - height * matrix[5];

	AENG_cone[2].x = AENG_cone[2].x + width *  matrix[0];
	AENG_cone[2].y = AENG_cone[2].y + width *  matrix[1];
	AENG_cone[2].z = AENG_cone[2].z + width *  matrix[2];

	//
	// AENG_cone[3] is the bottom left corner...
	//

	AENG_cone[3].x = AENG_cone[3].x - height * matrix[3];
	AENG_cone[3].y = AENG_cone[3].y - height * matrix[4];
	AENG_cone[3].z = AENG_cone[3].z - height * matrix[5];

	AENG_cone[3].x = AENG_cone[3].x - width *  matrix[0];
	AENG_cone[3].y = AENG_cone[3].y - width *  matrix[1];
	AENG_cone[3].z = AENG_cone[3].z - width *  matrix[2];

	//
	// Create the gamut.
	//

	NGAMUT_init();

	NGAMUT_add_line(AENG_cone[4].x, AENG_cone[4].z, AENG_cone[0].x, AENG_cone[0].z);
	NGAMUT_add_line(AENG_cone[4].x, AENG_cone[4].z, AENG_cone[1].x, AENG_cone[1].z);
	NGAMUT_add_line(AENG_cone[4].x, AENG_cone[4].z, AENG_cone[2].x, AENG_cone[2].z);
	NGAMUT_add_line(AENG_cone[4].x, AENG_cone[4].z, AENG_cone[3].x, AENG_cone[3].z);

	NGAMUT_add_line(AENG_cone[0].x, AENG_cone[0].z, AENG_cone[1].x, AENG_cone[1].z);
	NGAMUT_add_line(AENG_cone[1].x, AENG_cone[1].z, AENG_cone[2].x, AENG_cone[2].z);
	NGAMUT_add_line(AENG_cone[2].x, AENG_cone[2].z, AENG_cone[3].x, AENG_cone[3].z);
	NGAMUT_add_line(AENG_cone[3].x, AENG_cone[3].z, AENG_cone[0].x, AENG_cone[0].z);

	NGAMUT_calculate_point_gamut();
	NGAMUT_calculate_out_gamut();
	NGAMUT_calculate_lo_gamut();
}




// The gamut calculation for the skyline - lo-rez map only.
// This doesn't even do the gamut "properly" - there are only 32x32 squares,
// so it's much quicker to just find the bounding box. Much easier too.
// So, only these four are updated:
SLONG AENG_gamut_lo_xmin;
SLONG AENG_gamut_lo_xmax;
SLONG AENG_gamut_lo_zmin;
SLONG AENG_gamut_lo_zmax;

void AENG_calc_gamut_lo_only(
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float draw_dist,
		float lens)
{
	float width;
	float height;
	float depth;
	float aspect;
	float matrix[9];

	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	//
	// The dimensions of the view pyramid.
	//

	width  = draw_dist;
	height = draw_dist;
	depth  = draw_dist;

	width *= POLY_screen_width;
	width /= POLY_screen_height;

	if (FC_cam[1].focus)
	{
		//
		// We are in splitscreen mode.
		//

		width *= 2.0F;
	}

	width  /= lens;
	height /= lens;

	//
	// Finds the points of the cone in world space
	//

	AENG_cone[3].x = AENG_cone[4].x = x / 256.0f;
	//AENG_cone[3].y = AENG_cone[4].y = y / 256.0f;
	AENG_cone[3].z = AENG_cone[4].z = z / 256.0f;

	AENG_cone[3].x += depth * matrix[6];
	//AENG_cone[3].y += depth * matrix[7];
	AENG_cone[3].z += depth * matrix[8];

	//
	// AENG_cone[0] is the top left corner...
	//

	AENG_cone[0].x = ( AENG_cone[3].x + height * matrix[3] );
	//AENG_cone[0].y = ( AENG_cone[3].y + height * matrix[4] );
	AENG_cone[0].z = ( AENG_cone[3].z + height * matrix[5] );

	AENG_cone[0].x = ( AENG_cone[0].x - width *  matrix[0] );
	//AENG_cone[0].y = ( AENG_cone[0].y - width *  matrix[1] );
	AENG_cone[0].z = ( AENG_cone[0].z - width *  matrix[2] );

	//
	// AENG_cone[1] is the top right corner...
	//

	AENG_cone[1].x = ( AENG_cone[3].x + height * matrix[3] );
	//AENG_cone[1].y = ( AENG_cone[3].y + height * matrix[4] );
	AENG_cone[1].z = ( AENG_cone[3].z + height * matrix[5] );

	AENG_cone[1].x = ( AENG_cone[1].x + width *  matrix[0] );
	//AENG_cone[1].y = ( AENG_cone[1].y + width *  matrix[1] );
	AENG_cone[1].z = ( AENG_cone[1].z + width *  matrix[2] );

	//
	// AENG_cone[2] is the bottom right corner...
	//

	AENG_cone[2].x = ( AENG_cone[3].x - height * matrix[3] );
	//AENG_cone[2].y = ( AENG_cone[3].y - height * matrix[4] );
	AENG_cone[2].z = ( AENG_cone[3].z - height * matrix[5] );

	AENG_cone[2].x = ( AENG_cone[2].x + width *  matrix[0] );
	//AENG_cone[2].y = ( AENG_cone[2].y + width *  matrix[1] );
	AENG_cone[2].z = ( AENG_cone[2].z + width *  matrix[2] );

	//
	// AENG_cone[3] is the bottom left corner...
	//

	AENG_cone[3].x = ( AENG_cone[3].x - height * matrix[3] );
	//AENG_cone[3].y = ( AENG_cone[3].y - height * matrix[4] );
	AENG_cone[3].z = ( AENG_cone[3].z - height * matrix[5] );

	AENG_cone[3].x = ( AENG_cone[3].x - width *  matrix[0] );
	//AENG_cone[3].y = ( AENG_cone[3].y - width *  matrix[1] );
	AENG_cone[3].z = ( AENG_cone[3].z - width *  matrix[2] );

	//
	// Create the gamut.
	//


	// Clip the eight lines to the size of the map, then find their bounding box.
	float gamut_lo_xmax = AENG_cone[4].x * 0.25f;
	float gamut_lo_xmin = AENG_cone[4].x * 0.25f;
	float gamut_lo_zmax = AENG_cone[4].z * 0.25f;
	float gamut_lo_zmin = AENG_cone[4].z * 0.25f;
	// Point 4 should always be on the map, AFAICS.
	ASSERT ( ( gamut_lo_xmax < PAP_SIZE_LO ) && ( gamut_lo_xmin > 0 ) );
	ASSERT ( ( gamut_lo_xmax < PAP_SIZE_LO ) && ( gamut_lo_xmin > 0 ) );

	for ( int i = 0; i < 8; i++ )
	{
		int iPt1, iPt2;
		switch ( i )
		{
		case 0: iPt1 = 4; iPt2 = 0; break;
		case 1: iPt1 = 4; iPt2 = 1; break;
		case 2: iPt1 = 4; iPt2 = 2; break;
		case 3: iPt1 = 4; iPt2 = 3; break;
		case 4: iPt1 = 0; iPt2 = 1; break;
		case 5: iPt1 = 1; iPt2 = 2; break;
		case 6: iPt1 = 2; iPt2 = 3; break;
		case 7: iPt1 = 3; iPt2 = 4; break;
		}

		float fX1 = AENG_cone[iPt1].x * 0.25f;
		float fZ1 = AENG_cone[iPt1].z * 0.25f;
		float fX2 = AENG_cone[iPt2].x * 0.25f;
		float fZ2 = AENG_cone[iPt2].z * 0.25f;

		// Clip to +ve X
		if ( fX1 >= PAP_SIZE_LO )
		{
			if ( fX2 >= PAP_SIZE_LO )
			{
				// Quick reject.
				gamut_lo_xmax = PAP_SIZE_LO;
				continue;
			}
			else
			{
				float lambda = ( (float)PAP_SIZE_LO - fX1 ) / ( fX2 - fX1 );
				fZ1 = fZ1 + lambda * ( fZ2 - fZ1 );
				fX1 = PAP_SIZE_LO;
			}
		}
		else
		{
			if ( fX2 >= PAP_SIZE_LO )
			{
				float lambda = ( (float)PAP_SIZE_LO - fX1 ) / ( fX2 - fX1 );
				fZ2 = fZ1 + lambda * ( fZ2 - fZ1 );
				fX2 = PAP_SIZE_LO;
			}
			else
			{
				// Accept.
			}
		}

		// Clip to 0 X
		if ( fX1 <= 0.0f )
		{
			if ( fX2 <= 0.0f )
			{
				// Quick reject.
				gamut_lo_xmin = 0;
				continue;
			}
			else
			{
				float lambda = ( 0.0f - fX1 ) / ( fX2 - fX1 );
				fZ1 = fZ1 + lambda * ( fZ2 - fZ1 );
				fX1 = 0.0f;
			}
		}
		else
		{
			if ( fX2 <= 0.0f )
			{
				float lambda = ( 0.0f - fX1 ) / ( fX2 - fX1 );
				fZ2 = fZ1 + lambda * ( fZ2 - fZ1 );
				fX2 = 0.0f;
			}
			else
			{
				// Accept.
			}
		}

		// Clip to +ve Z
		if ( fZ1 >= PAP_SIZE_LO )
		{
			if ( fZ2 >= PAP_SIZE_LO )
			{
				// Quick reject.
				gamut_lo_zmax = PAP_SIZE_LO;
				continue;
			}
			else
			{
				float lambda = ( (float)PAP_SIZE_LO - fZ1 ) / ( fZ2 - fZ1 );
				fX1 = fX1 + lambda * ( fX2 - fX1 );
				fZ1 = PAP_SIZE_LO;
			}
		}
		else
		{
			if ( fZ2 >= PAP_SIZE_LO )
			{
				float lambda = ( (float)PAP_SIZE_LO - fZ1 ) / ( fZ2 - fZ1 );
				fX2 = fX1 + lambda * ( fX2 - fX1 );
				fZ2 = PAP_SIZE_LO;
			}
			else
			{
				// Accept.
			}
		}

		// Clip to 0 Z
		if ( fZ1 <= 0.0f )
		{
			if ( fZ2 <= 0.0f )
			{
				// Quick reject.
				gamut_lo_zmin = 0;
				continue;
			}
			else
			{
				float lambda = ( 0.0f - fZ1 ) / ( fZ2 - fZ1 );
				fX1 = fX1 + lambda * ( fX2 - fX1 );
				fZ1 = 0.0f;
			}
		}
		else
		{
			if ( fZ2 <= 0.0f )
			{
				float lambda = ( 0.0f - fZ1 ) / ( fZ2 - fZ1 );
				fX2 = fX1 + lambda * ( fX2 - fX1 );
				fZ2 = 0.0f;
			}
			else
			{
				// Accept.
			}
		}


		ASSERT ( ( fX1 <= PAP_SIZE_LO ) && ( fX1 >= 0.0f ) );
		ASSERT ( ( fX2 <= PAP_SIZE_LO ) && ( fX2 >= 0.0f ) );
		ASSERT ( ( fZ1 <= PAP_SIZE_LO ) && ( fZ1 >= 0.0f ) );
		ASSERT ( ( fZ2 <= PAP_SIZE_LO ) && ( fZ2 >= 0.0f ) );


		// Expand BB.
		if ( gamut_lo_xmax < fX1 )
		{
			gamut_lo_xmax = fX1;
		}
		else if ( gamut_lo_xmin > fX1 )
		{
			gamut_lo_xmin = fX1;
		}
		if ( gamut_lo_zmax < fZ1 )
		{
			gamut_lo_zmax = fZ1;
		}
		else if ( gamut_lo_zmin > fZ1 )
		{
			gamut_lo_zmin = fZ1;
		}

		if ( gamut_lo_xmax < fX2 )
		{
			gamut_lo_xmax = fX2;
		}
		else if ( gamut_lo_xmin > fX2 )
		{
			gamut_lo_xmin = fX2;
		}
		if ( gamut_lo_zmax < fZ2 )
		{
			gamut_lo_zmax = fZ2;
		}
		else if ( gamut_lo_zmin > fZ2 )
		{
			gamut_lo_zmin = fZ2;
		}

	}


#if 0

	float gamut_lo_xmax = AENG_cone[0].x;
	float gamut_lo_xmin = AENG_cone[0].x;
	float gamut_lo_zmax = AENG_cone[0].z;
	float gamut_lo_zmin = AENG_cone[0].z;
	for ( int i = 1; i <= 4; i++ )
	{
		if ( gamut_lo_xmax < AENG_cone[i].x )
		{
			gamut_lo_xmax = AENG_cone[i].x;
		}
		else if ( gamut_lo_xmin > AENG_cone[i].x )
		{
			gamut_lo_xmin = AENG_cone[i].x;
		}
		if ( gamut_lo_zmax < AENG_cone[i].z )
		{
			gamut_lo_zmax = AENG_cone[i].z;
		}
		else if ( gamut_lo_zmin > AENG_cone[i].z )
		{
			gamut_lo_zmin = AENG_cone[i].z;
		}
	}

#endif

#if 1
	AENG_gamut_lo_xmin = (SLONG)( gamut_lo_xmin );
	AENG_gamut_lo_xmax = (SLONG)( gamut_lo_xmax );
	AENG_gamut_lo_zmin = (SLONG)( gamut_lo_zmin );
	AENG_gamut_lo_zmax = (SLONG)( gamut_lo_zmax );
#else
	AENG_gamut_lo_xmin = (SLONG)( gamut_lo_xmin * 0.25f );
	AENG_gamut_lo_xmax = (SLONG)( gamut_lo_xmax * 0.25f );
	AENG_gamut_lo_zmin = (SLONG)( gamut_lo_zmin * 0.25f );
	AENG_gamut_lo_zmax = (SLONG)( gamut_lo_zmax * 0.25f );
#endif

	// Just catch the dodgy edge condition.
	if ( AENG_gamut_lo_xmax == PAP_SIZE_LO )
	{
		AENG_gamut_lo_xmax = PAP_SIZE_LO - 1;
	}
	if ( AENG_gamut_lo_xmin == PAP_SIZE_LO )
	{
		AENG_gamut_lo_xmin = PAP_SIZE_LO - 1;
	}
	if ( AENG_gamut_lo_zmax == PAP_SIZE_LO )
	{
		AENG_gamut_lo_zmax = PAP_SIZE_LO - 1;
	}
	if ( AENG_gamut_lo_zmin == PAP_SIZE_LO )
	{
		AENG_gamut_lo_zmin = PAP_SIZE_LO - 1;
	}



#if 0
	if ( AENG_gamut_lo_xmax < 0 )
	{
		AENG_gamut_lo_xmax = 0;
		AENG_gamut_lo_xmin = 0;
	}
	else if ( AENG_gamut_lo_xmax >= PAP_SIZE_LO )
	{
		AENG_gamut_lo_xmax = PAP_SIZE_LO - 1;
	}

	if ( AENG_gamut_lo_xmin >= PAP_SIZE_LO )
	{
		AENG_gamut_lo_xmax = PAP_SIZE_LO - 1;
		AENG_gamut_lo_xmin = PAP_SIZE_LO - 1;
	}
	else if ( AENG_gamut_lo_xmin < 0 )
	{
		AENG_gamut_lo_xmin = 0;
	}

	if ( AENG_gamut_lo_zmax < 0 )
	{
		AENG_gamut_lo_zmax = 0;
		AENG_gamut_lo_zmin = 0;
	}
	else if ( AENG_gamut_lo_zmax >= PAP_SIZE_LO )
	{
		AENG_gamut_lo_zmax = PAP_SIZE_LO - 1;
	}

	if ( AENG_gamut_lo_zmin >= PAP_SIZE_LO )
	{
		AENG_gamut_lo_zmax = PAP_SIZE_LO - 1;
		AENG_gamut_lo_zmin = PAP_SIZE_LO - 1;
	}
	else if ( AENG_gamut_lo_zmin < 0 )
	{
		AENG_gamut_lo_zmin = 0;
	}
#endif

}


void AENG_set_camera_radians(
		SLONG wx,
		SLONG wy,
		SLONG wz,
		float y,
		float p,
		float r)
{
	AENG_cam_x = float(wx);
	AENG_cam_y = float(wy);
	AENG_cam_z = float(wz);

	AENG_cam_yaw   = y;
	AENG_cam_pitch = p;
	AENG_cam_roll  = r;

	MATRIX_calc(
		AENG_cam_matrix,
		y,p,r);

	POLY_camera_set(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		float(AENG_DRAW_DIST) * 256.0F,
		AENG_LENS,
		POLY_SPLITSCREEN_NONE);

	FMATRIX_vector(AENG_cam_vec,(y*2048)/(2*PI),(p*2048)/(2*PI));

	//
	// Create the gamut
	//

	AENG_calc_gamut(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		AENG_DRAW_DIST,
		AENG_LENS);
}


void AENG_set_camera_radians(
		SLONG wx,
		SLONG wy,
		SLONG wz,
		float y,
		float p,
		float r,
		SLONG splitscreen)
{
	AENG_cam_x = float(wx);
	AENG_cam_y = float(wy);
	AENG_cam_z = float(wz);

	AENG_cam_yaw   = y;
	AENG_cam_pitch = p;
	AENG_cam_roll  = r;

	MATRIX_calc(
		AENG_cam_matrix,
		y,p,r);

	POLY_camera_set(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		float(AENG_DRAW_DIST) * 256.0F,
		AENG_LENS,
		splitscreen);

	FMATRIX_vector(AENG_cam_vec,(y*2048)/(2*PI),(p*2048)/(2*PI));


	//
	// Create the gamut
	//

	AENG_calc_gamut(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		AENG_DRAW_DIST,
		AENG_LENS);
}

void AENG_set_camera(
		SLONG wx,
		SLONG wy,
		SLONG wz,
		SLONG y,
		SLONG p,
		SLONG r)
{
	float radians_yaw   = float(y) * (2.0F * PI / 2048.0F);
	float radians_pitch = float(p) * (2.0F * PI / 2048.0F);
	float radians_roll  = float(r) * (2.0F * PI / 2048.0F);

	FC_cam[0].x=wx<<8;
	FC_cam[0].y=wy<<8;
	FC_cam[0].z=wz<<8;

	FC_cam[0].yaw=y<<8;
	FC_cam[0].pitch=p<<8;
	FC_cam[0].roll=r<<8;

	AENG_set_camera_radians(
		wx,
		wy,
		wz,
		radians_yaw,
		radians_pitch,
		radians_roll,
		POLY_SPLITSCREEN_NONE);
}






void AENG_do_cached_lighting_old(void)
{
	SLONG i;
	SLONG x;
	SLONG z;
	SLONG	kept=0,new_squares=0;

	NIGHT_Square *nq;

extern	SLONG	HEAP_max_free(void);

	if(HEAP_max_free()<4000 || Keys[KB_Q])
	{
		CBYTE	str[100];
		NIGHT_destroy_all_cached_info();

//		sprintf(str," RECALC LIGHTING FREE = %d",HEAP_max_free());
//		CONSOLE_text(str,10000);
	}
	else
	{
		//
		// Get rid of any cached squares we dont need anymore.
		//

		for (i = 1; i < NIGHT_MAX_SQUARES; i++)
		{
			nq = &NIGHT_square[i];

			if (nq->flag & NIGHT_SQUARE_FLAG_USED)
			{
				if (WITHIN(nq->lo_map_z, NGAMUT_lo_zmin, NGAMUT_lo_zmax) &&
					WITHIN(
						nq->lo_map_x,
						NGAMUT_lo_gamut[nq->lo_map_z].xmin,
						NGAMUT_lo_gamut[nq->lo_map_z].xmax))
				{
					//
					// We still need this cache info.
					//
					kept++;
				}
				else
				{
					//
					// We dont need this square any more.
					//

					NIGHT_cache_destroy(i);
				}
			}
		}
	}




	if(INDOORS_INDEX)
	{
		if(light_inside!=INDOORS_INDEX)
		{
			//
			// remove floor lighting
			//
			for (i = 1; i < NIGHT_MAX_SQUARES; i++)
			{
				nq = &NIGHT_square[i];
				if (nq->flag & NIGHT_SQUARE_FLAG_USED)
					NIGHT_cache_destroy(i);
			}

		}
		light_inside=INDOORS_INDEX;

		//
		// create the squares we need for the inside bounding rect clipped with gamut
		//

		{
			SLONG	x,z,floor_y;
			struct	InsideStorey	*p_inside;
			SLONG	in_width;
			UBYTE	*in_block;
			SLONG	min_z,max_z;
			SLONG	min_x,max_x;



			p_inside=&inside_storeys[INDOORS_INDEX];

			floor_y=p_inside->StoreyY;

			min_z=MAX(NGAMUT_lo_zmin,p_inside->MinZ>>2);
			max_z=MIN(NGAMUT_lo_zmax,p_inside->MaxZ>>2);

			in_width=p_inside->MaxX-p_inside->MinX;

			for (z = min_z; z <= max_z+1; z++)
			{
				min_x=MAX(NGAMUT_lo_gamut[z].xmin,p_inside->MinX>>2);
				max_x=MIN(NGAMUT_lo_gamut[z].xmax,p_inside->MaxX>>2);

				if(z>min_z)
				{
					min_x=MIN(NGAMUT_lo_gamut[z-1].xmin,min_x);
					max_x=MAX(NGAMUT_lo_gamut[z-1].xmax,max_x);
				}

				for (x = min_x;x<=max_x+1;x++)
				{
					if (NIGHT_cache[x][z] == NULL)
					{
						//
						// Creating cached lighting for this square.
						//

						NIGHT_cache_create_inside(x,z,floor_y);
						new_squares++;
					}


				}
			}
		}

		return;
	}
	else
	if(light_inside)
	{
		light_inside=0;
		for (i = 1; i < NIGHT_MAX_SQUARES; i++)
		{
			nq = &NIGHT_square[i];

			if (nq->flag & NIGHT_SQUARE_FLAG_USED)
			{
				NIGHT_cache_destroy(i);
			}
		}
	}

	//
	// Create any square we need.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

			if (NIGHT_cache[x][z] == NULL)
			{
				//
				// Creating cached lighting for this square.
				//

				NIGHT_cache_create(x,z);
			}
		}
	}
}

//
// Marks all caches squares with 'DELETEME'. When a square is drawn
// its DELETEME flag is cleared.
//

void AENG_mark_night_squares_as_deleteme(void)
{
	SLONG i;

	NIGHT_Square *nq;

	for (i = 1; i < NIGHT_MAX_SQUARES; i++)
	{
		nq = &NIGHT_square[i];

		//
		// Do this to all NIGHT_squares... not just the used ones.
		//

		nq->flag |= NIGHT_SQUARE_FLAG_DELETEME;
	}
}

//
// Makes sure that all lo-res gamut squares have the right kind of
// lighting caching done for them. (In a warehouse or not).
//

void AENG_ensure_appropriate_caching(SLONG ware)
{
	SLONG x;
	SLONG z;
	SLONG ok;

	NIGHT_Square *nq;

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

			if (NIGHT_cache[x][z] == NULL)
			{
				//
				// Creating cached lighting for this square.
				//

				NIGHT_cache_create(x,z,ware);
			}
			else
			{
				//
				// Make sure this square has the correct type.
				//

				nq = &NIGHT_square[NIGHT_cache[x][z]];

				if (nq->flag & NIGHT_SQUARE_FLAG_WARE)
				{
					ok =  ware;
				}
				else
				{
					ok = !ware;
				}

				if (!ok)
				{
					//
					// The caching is the wrong sort!
					//

					NIGHT_cache_destroy(NIGHT_cache[x][z]);
					NIGHT_cache_create(x,z,ware);
				}
			}
		}
	}
}


void AENG_get_rid_of_deleteme_squares()
{
	SLONG i;

	NIGHT_Square *nq;

	for (i = 1; i < NIGHT_MAX_SQUARES; i++)
	{
		nq = &NIGHT_square[i];

		if (nq->flag & NIGHT_SQUARE_FLAG_USED)
		{
			if (nq->flag & NIGHT_SQUARE_FLAG_DELETEME)
			{
				NIGHT_cache_destroy(i);
			}
		}
	}
}

//
// Adds a projected shadow poly to the POLY module.
//

float AENG_project_offset_u;
float AENG_project_offset_v;

float AENG_project_mul_u;
float AENG_project_mul_v;

float AENG_project_lit_light_x;
float AENG_project_lit_light_y;
float AENG_project_lit_light_z;
float AENG_project_lit_light_range;

float AENG_project_fadeout_x;
float AENG_project_fadeout_z;


#define SHADOW_Z_BIAS_BODGE 0.0001f


void AENG_add_projected_shadow_poly(SMAP_Link *sl)
{
	SLONG i;

	POLY_Point *pp;

	//
	// Transform all the points into the poly buffer.
	//

	POLY_buffer_upto = 0;

	while(sl)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform(
			sl->wx,
			sl->wy,
			sl->wz,
			pp);

		if (pp->MaybeValid())
		{
			pp->u = AENG_project_offset_u + sl->u * AENG_project_mul_u;
			pp->v = AENG_project_offset_v + sl->v * AENG_project_mul_v;

			pp->colour   = 0xffffffff;
			pp->specular = 0xff000000;

#ifdef TARGET_DC
			// Stop Z fighting
			pp->Z += SHADOW_Z_BIAS_BODGE;
#endif
		}
		else
		{
			//
			// Abandon the whole shadow polygon.
			//

			return;
		}

		sl = sl->next;
	}

	//
	// Add the triangles.
	//

	POLY_Point *tri[3];

	tri[0] = &POLY_buffer[0];

	for (i = 1; i < POLY_buffer_upto - 1; i++)
	{
		tri[1] = &POLY_buffer[i + 0];
		tri[2] = &POLY_buffer[i + 1];

		if (POLY_valid_triangle(tri))
		{
			POLY_add_triangle(tri, POLY_PAGE_SHADOW, TRUE);
		}
	}
}

void AENG_add_projected_fadeout_shadow_poly(SMAP_Link *sl)
{
	float dx;
	float dz;
	float dist;

	SLONG i;
	SLONG alpha;

	POLY_Point *pp;

	//
	// Transform all the points into the poly buffer.
	//

	POLY_buffer_upto = 0;

	while(sl)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform(
			sl->wx,
			sl->wy,
			sl->wz,
			pp);

		if (pp->MaybeValid())
		{
			dx = sl->wx - AENG_project_fadeout_x;
			dz = sl->wz - AENG_project_fadeout_z;

			dist = fabs(dx) + fabs(dz);

			if (dist < 64.0F)
			{
				alpha = 0xff;
			}
			else
			{
				if (dist > 256.0F)
				{
					alpha = 0;
				}
				else
				{
					alpha = 0xff - SLONG((dist - 64.0F) * (255.0F / 192.0F));
				}
			}

			pp->u = AENG_project_offset_u + sl->u * AENG_project_mul_u;
			pp->v = AENG_project_offset_v + sl->v * AENG_project_mul_v;

			alpha |= alpha << 8;
			alpha |= alpha << 16;

			pp->colour   = alpha;
			pp->specular = 0xff000000;

#ifdef TARGET_DC
			// Stop Z fighting
			pp->Z += SHADOW_Z_BIAS_BODGE;
#endif
		}
		else
		{
			//
			// Abandon the whole shadow polygon.
			//

			return;
		}

		sl = sl->next;
	}

	//
	// Add the triangles.
	//

	POLY_Point *tri[3];

	tri[0] = &POLY_buffer[0];

	for (i = 1; i < POLY_buffer_upto - 1; i++)
	{
		tri[1] = &POLY_buffer[i + 0];
		tri[2] = &POLY_buffer[i + 1];

		if (POLY_valid_triangle(tri))
		{
			POLY_add_triangle(tri, POLY_PAGE_SHADOW, TRUE);
		}
	}
}

void AENG_add_projected_lit_shadow_poly(SMAP_Link *sl)
{
	SLONG i;

	float dx;
	float dy;
	float dz;
	float dist;
	float bright;

	POLY_Point *pp;

	//
	// Transform all the points into the poly buffer.
	//

	POLY_buffer_upto = 0;

	while(sl)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform(
			sl->wx,
			sl->wy,
			sl->wz,
			pp);

		if (pp->MaybeValid())
		{
			pp->u = AENG_project_offset_u + sl->u * AENG_project_mul_u;
			pp->v = AENG_project_offset_v + sl->v * AENG_project_mul_v;

			dx = sl->wx - AENG_project_lit_light_x;
			dy = sl->wy - AENG_project_lit_light_y;
			dz = sl->wz - AENG_project_lit_light_z;

			dist    = fabs(dx) + fabs(dy) + fabs(dz);
			bright  = dist / AENG_project_lit_light_range;
			bright  = 1.0F - bright;
			bright *= 512.0F;

			if (bright > 0.0F)
			{
				SLONG alpha = SLONG(bright);

				if (alpha > 255) {alpha = 255;}

				alpha |= alpha << 8;
				alpha |= alpha << 16;
#ifdef TARGET_DC
				alpha |= 0xff000000;
#endif

				pp->colour   = alpha;
				pp->specular = 0xff000000;
			}
			else
			{
#ifdef TARGET_DC
				pp->colour   = 0xff000000;
#else
				pp->colour   = 0x00000000;
#endif
				pp->specular = 0xff000000;
			}

#ifdef TARGET_DC
			// Stop Z fighting
			pp->Z += SHADOW_Z_BIAS_BODGE;
#endif

		}
		else
		{
			//
			// Abandon the whole shadow polygon.
			//

			return;
		}

		sl = sl->next;
	}

	//
	// Add the triangles.
	//

	POLY_Point *tri[3];

	tri[0] = &POLY_buffer[0];

	for (i = 1; i < POLY_buffer_upto - 1; i++)
	{
		tri[1] = &POLY_buffer[i + 0];
		tri[2] = &POLY_buffer[i + 1];

		if (POLY_valid_triangle(tri))
		{
			POLY_add_triangle(tri, POLY_PAGE_SHADOW, TRUE);
		}
	}
}



//
// Draws the rain.
//

void AENG_draw_rain_old(float angle)
{
	SLONG i;

	float vec1x;
	float vec1y;
	float vec2x;
	float vec2y;

	float z;
	float X;
	float Y;
	float Z;

	POLY_Point  pp [3];
	POLY_Point *tri[3];

	tri[0] = &pp[0];
	tri[1] = &pp[1];
	tri[2] = &pp[2];

	//
	// Common to all poly points.
	//

	pp[0].colour   = 0x00000000;
	pp[1].colour   = 0x88333344;
	pp[2].colour   = 0x88555577;

	pp[0].specular = 0x00000000;
	pp[1].specular = 0x00000000;
	pp[2].specular = 0x00000000;

	pp[0].u = 0.0F;
	pp[0].v = 0.0F;
	pp[1].u = 0.0F;
	pp[1].v = 0.0F;
	pp[2].u = 0.0F;
	pp[2].v = 0.0F;

	//
	// Work out the vectors from the angle.
	//

	#define AENG_RAIN_SIZE (4.0F)

	vec1x =  (float)sin(angle) * (32.0F * AENG_RAIN_SIZE);
	vec1y = -(float)cos(angle) * (32.0F * AENG_RAIN_SIZE);

	vec2x =  (float)cos(angle) * AENG_RAIN_SIZE;
	vec2y =  (float)sin(angle) * AENG_RAIN_SIZE;

	#define AENG_NUM_RAINDROPS 128

	for (i = 0; i < AENG_NUM_RAINDROPS; i++)
	{
		z = float(rand() & 0xff) * (0.5F / 256.0F) + POLY_ZCLIP_PLANE;
		X = float(rand() % DisplayWidth);
		Y = float(rand() % DisplayHeight);
		Z = POLY_ZCLIP_PLANE / z;

		pp[0].X = X;
		pp[0].Y = Y;
		pp[0].Z = Z;
		pp[0].z = z;

		pp[1].X = X + vec1x * Z;
		pp[1].Y = Y + vec1y * Z;
		pp[1].Z = Z;
		pp[1].z = z;

		pp[2].X = X + vec2x * Z;
		pp[2].Y = Y + vec2y * Z;
		pp[2].Z = Z;
		pp[2].z = z;

		POLY_add_triangle(tri, POLY_PAGE_ALPHA, FALSE, TRUE);
	}
}


void AENG_draw_rain()
{
	SLONG i;

	float x1;
	float y1;
	float z1;

	float x2;
	float y2;
	float z2;

	float matrix[9];

	float fade;
	SLONG bright;
	SLONG r;
	SLONG g;
	SLONG b;
	ULONG colour;

	//
	// The cameras view matrix.
	//

	MATRIX_calc(
		matrix,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll);

	//
	// Fiddle the matrix so multipling (a,b,c) by the matrix
	// where a,b are between -1 and 1 and c is between 0 and 1
	// gives a place in the world that the camera can see...
	//

	matrix[0] *= 640.0F / 480.0F;
	matrix[1] *= 640.0F / 480.0F;
	matrix[2] *= 640.0F / 480.0F;

	matrix[0] /= AENG_LENS;
	matrix[1] /= AENG_LENS;
	matrix[2] /= AENG_LENS;

	matrix[3] /= AENG_LENS;
	matrix[4] /= AENG_LENS;
	matrix[5] /= AENG_LENS;

	//
	// Rain lasts 8 squares into the distance.
	//

//#undef AENG_NUM_RAINDROPS
//#define AENG_NUM_RAINDROPS	500

	matrix[0] *= 256.0F * 8.0F;
	matrix[1] *= 256.0F * 8.0F;
	matrix[2] *= 256.0F * 8.0F;

	matrix[3] *= 256.0F * 8.0F;
	matrix[4] *= 256.0F * 8.0F;
	matrix[5] *= 256.0F * 8.0F;

	matrix[6] *= 256.0F * 8.0F;
	matrix[7] *= 256.0F * 8.0F;
	matrix[8] *= 256.0F * 8.0F;

	for (i = 0; i < AENG_NUM_RAINDROPS; i++)
	{
		//
		// Pick a random place in world space in front of the camera.
		//

		x1 = float(rand()) * (1.0F / float(RAND_MAX >> 1)) - 1.0F;
		y1 = float(rand()) * (1.0F / float(RAND_MAX >> 1)) - 0.5F;
		z1 = float(rand()) * (1.0F / float(RAND_MAX     )) + 0.1F;

		fade   = 1.0F - z1 * 0.8F;
		bright = SLONG(fade * 256.0F);

		colour = (bright << 24) | ((69 << 16) | (74 << 8) | (98 << 0));

		MATRIX_MUL_BY_TRANSPOSE(
			matrix,
			x1,
			y1,
			z1);

		x1 += AENG_cam_x;
		y1 += AENG_cam_y;
		z1 += AENG_cam_z;

#if 1	// shade the rain
		SLONG	px = SLONG(x1) >> 10;
		SLONG	pz = SLONG(z1) >> 10;
		SLONG	dx = (SLONG(x1) >> 8) & 3;
		SLONG	dz = (SLONG(z1) >> 8) & 3;

		if ((px < 0) || (px >= PAP_SIZE_LO))	continue;
		if ((pz < 0) || (pz >= PAP_SIZE_LO))	continue;

		SLONG square = NIGHT_cache[px][pz];

		if (!square)	continue;

		ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
		ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);

		NIGHT_Square*	nq = &NIGHT_square[square];
		ULONG			col,spec;

		NIGHT_get_d3d_colour(nq->colour[dx + dz * PAP_BLOCKS], &col, &spec);

		colour = col;
#endif

		SHAPE_droplet(
			SLONG(x1),
			SLONG(y1),
			SLONG(z1),
			 8,
		   -64,
			 8,
			 colour,
			 POLY_PAGE_RAINDROP);
	}
}

void AENG_draw_drips(UBYTE puddles_only)
{
	//
	// Draw the drips.
	//

	SLONG i;

	float midx;
	float midy;
	float midz;

	float px;
	float pz;

	float radius;
	ULONG colour;

	DRIP_Info *di;

	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// The same for all drips.
	//

	pp[0].u = 0.0F;
	pp[0].v = 0.0F;
	pp[1].u = 1.0F;
	pp[1].v = 0.0F;
	pp[2].u = 0.0F;
	pp[2].v = 1.0F;
	pp[3].u = 1.0F;
	pp[3].v = 1.0F;

	pp[0].specular = 0xff000000;
	pp[1].specular = 0xff000000;
	pp[2].specular = 0xff000000;
	pp[3].specular = 0xff000000;

	DRIP_get_start();

	while(di = DRIP_get_next())
	{

		if (puddles_only != (di->flags&DRIP_FLAG_PUDDLES_ONLY))
			continue;  // Abandon this drip.


		midx  = float(di->x);
		midy  = float(di->y);
		midz  = float(di->z);

		midy += 8.0F;

		radius = float(di->size);

		for (i = 0; i < 4; i++)
		{
			px = midx + ((i & 0x1) ? +radius : -radius);
			pz = midz + ((i & 0x2) ? +radius : -radius);

			POLY_transform(px, midy, pz, &pp[i]);

			if (!pp[i].IsValid())
				continue;  // Abandon this drip.
		}

		if (POLY_valid_quad(quad))
		{
			colour = (di->fade << 16) | (di->fade << 8) | (di->fade << 0);

			pp[0].colour = colour;
			pp[1].colour = colour;
			pp[2].colour = colour;
			pp[3].colour = colour;

			POLY_add_quad(quad, POLY_PAGE_DRIP, FALSE);
		}

	}
}


//
// Adds the bangs in the current gamut to the POLY module.
//

void AENG_draw_bangs()
{
	float u_mid;
	float v_mid;
#ifdef	DOG_POO
	SLONG z;

	BANG_Info *bi;

	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		BANG_get_start(
			NGAMUT_point_gamut[z].xmin,
			NGAMUT_point_gamut[z].xmax,
			z);

		while(bi = BANG_get_next())
		{
			u_mid = (1.0F / 8.0F) + (1.0F / 4.0F) * float(bi->frame  & 0x3);
			v_mid = (1.0F / 8.0F) + (1.0F / 4.0F) * float(bi->frame >>   2);

			#ifdef MAKE_THEM_FACE_THE_CAMERA

			//
			// Always make it face the camera.
			//

			float dx = AENG_cam_x - float(bi->x);
			float dy = AENG_cam_y - float(bi->y);
			float dz = AENG_cam_z - float(bi->z);

#ifdef TARGET_DC
			float len = 256.0f * _InvSqrtA(dx*dx + dy*dy + dz*dz);

			dx = dx * len;
			dy = dy * len;
			dz = dz * len;
#else
			float len = sqrt(dx*dx + dy*dy + dz*dz);

			dx = dx * (256.0F / len);
			dy = dy * (256.0F / len);
			dz = dz * (256.0F / len);
#endif

			#endif

			SHAPE_semisphere_textured(
				bi->x,
				bi->y,
				bi->z,
				bi->dx,
				bi->dy,
				bi->dz,
				bi->radius,
				u_mid,
				v_mid,
				1.0F / 8.0F,
				POLY_PAGE_BANG,
				bi->red,
				bi->green,
				bi->blue);
		}
	}
#endif
}

//
// Draws the cloth.
//

void AENG_draw_cloth(void)
{
#ifdef	DOG_POO
	SLONG i;

	SLONG x;
	SLONG z;

	UBYTE cloth;

	CLOTH_Info *ci;

	POLY_Point pp[CLOTH_WIDTH * CLOTH_HEIGHT];

	//
	// Our lighting normal for each point. The normal's length is less
	// than one so the dprod will always be in range. The normal from
	// the CLOTH module is only approximately of length 1.0!
	//

	static float light_x = 0.55F;
	static float light_y = 0.55F;
	static float light_z = 0.55F;

	float dprod;

	SLONG bright;
	SLONG r;
	SLONG g;
	SLONG b;

	SLONG base_r;
	SLONG base_g;
	SLONG base_b;

	SLONG px;
	SLONG py;

	POLY_Point *quad[4];

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			for (cloth = CLOTH_get_first(x,z); cloth; cloth = ci->next)
			{
				ci = CLOTH_get_info(cloth);

				base_r = (ci->colour >> 16) & 0xff;
				base_g = (ci->colour >>  8) & 0xff;
				base_b = (ci->colour >>  0) & 0xff;

				//
				// Transform all the points.
				//

				for (i = 0; i < CLOTH_WIDTH * CLOTH_HEIGHT; i++)
				{
					POLY_transform(
						ci->p[i].x,
						ci->p[i].y,
						ci->p[i].z,
					   &pp[i]);

					if (!pp[i].MaybeValid())
					{
						goto abandon_this_cloth;
					}

					//
					// Light the point.
					//

					dprod =
						light_x * ci->p[i].nx +
					    light_y * ci->p[i].ny +
					    light_z * ci->p[i].nz;

					dprod  = fabs(dprod);
					bright = SLONG(dprod * 255.0F);

					r = bright * base_r >> 8;
					g = bright * base_g >> 8;
					b = bright * base_b >> 8;

					pp[i].colour   = (r << 16) | (g << 8) | (b << 0);
					pp[i].specular = 0xff000000;
					pp[i].u        = 0.0F;
					pp[i].v        = 0.0F;
				}

				//
				// Create all the faces.
				//

				for (px = 0; px < CLOTH_WIDTH  - 1; px++)
				for (py = 0; py < CLOTH_HEIGHT - 1; py++)
				{
					quad[0] = &pp[CLOTH_INDEX(px + 0, py + 0)];
					quad[1] = &pp[CLOTH_INDEX(px + 1, py + 0)];
					quad[2] = &pp[CLOTH_INDEX(px + 0, py + 1)];
					quad[3] = &pp[CLOTH_INDEX(px + 1, py + 1)];

					if (POLY_valid_quad(quad))
					{
						POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE);
					}
				}

			  abandon_this_cloth:;
			}
		}
	}
#endif
}

//
// Adds the fire in the current gamut to the POLY module.
//

void AENG_draw_fire()
{
	SLONG z;

	FIRE_Info  *fi;
	FIRE_Point *fp;

	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		FIRE_get_start(
			NGAMUT_point_gamut[z].xmin,
			NGAMUT_point_gamut[z].xmax,
			z);
	}
}

void AENG_draw_sparks()
{
	SLONG z;

	SPARK_Info   *si;
	GLITTER_Info *gi;

	// Internal gubbins.
	POLY_flush_local_rot();

	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		SPARK_get_start(
			NGAMUT_point_gamut[z].xmin,
			NGAMUT_point_gamut[z].xmax,
			z);

		while(si = SPARK_get_next())
		{
			SHAPE_sparky_line(
				si->num_points,
				si->x,
				si->y,
				si->z,
				si->colour,
				float(si->size));
		}

		GLITTER_get_start(
			NGAMUT_point_gamut[z].xmin,
			NGAMUT_point_gamut[z].xmax,
			z);

		while(gi = GLITTER_get_next())
		{
			SHAPE_glitter(
				gi->x1,
				gi->y1,
				gi->z1,
				gi->x2,
				gi->y2,
				gi->z2,
				gi->colour);
		}
	}
}

//
// Draws the hook.
//

void AENG_draw_hook(void)
{
#ifdef TARGET_DC
	ASSERT ( FALSE );
#else //#ifdef TARGET_DC
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;
	SLONG yaw;
	SLONG pitch;
	SLONG roll;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG red   = 0x80;
	SLONG green = 0x20;
	SLONG blue  = 0x00;

	ULONG colour1;
	ULONG colour2;

	HOOK_pos_grapple(
		&x,
		&y,
		&z,
		&yaw,
		&pitch,
		&roll);

	x >>= 8;
	y >>= 8;
	z >>= 8;

	MESH_draw_poly(
		PRIM_OBJ_HOOK,
		x, y, z,
		yaw,
		pitch,
		roll,
		NULL,0xff,0);

	for (i = HOOK_NUM_POINTS - 1; i >= 1; i--)
	{
		HOOK_pos_point(i + 0, &x1, &y1, &z1);
		HOOK_pos_point(i - 1, &x2, &y2, &z2);

		x1 >>= 8;
		y1 >>= 8;
		z1 >>= 8;

		x2 >>= 8;
		y2 >>= 8;
		z2 >>= 8;

		if (red < 250)
		{
			red += 2;
		}
		else
		{
			if (green < 250)
			{
				green += 2;
			}
			else
			{
				if (blue < 250)
				{
					blue += 3;
				}
			}
		}

		colour1 = (red << 16) | (green << 8) | (blue << 0);
		colour2 = (red << 17) | (green << 8) | (blue >> 1);

		AENG_world_line(
			x1, y1, z1, 0x8, colour1,
			x2, y2, z2, 0x6, colour2,
			FALSE);
	}
#endif //#else //#ifdef TARGET_DC
}

ULONG AENG_colour_mult(ULONG c1, ULONG c2)
{
	SLONG r1 = (c1 >> 16) & 0xff;
	SLONG g1 = (c1 >>  8) & 0xff;
	SLONG b1 = (c1 >>  0) & 0xff;

	SLONG r2 = (c2 >> 16) & 0xff;
	SLONG g2 = (c2 >>  8) & 0xff;
	SLONG b2 = (c2 >>  0) & 0xff;

	SLONG ar = r1 * r2 >> 8;
	SLONG ag = g1 * g2 >> 8;
	SLONG ab = b1 * b2 >> 8;

	ULONG ans = (ar << 16) | (ag << 8) | (ab << 0);

	return ans;
}

//
// Draws the dirt.
//

extern UBYTE estate;

#define AENG_MAX_DIRT_LVERTS  (64 * 3)
#define AENG_MAX_DIRT_INDICES (AENG_MAX_DIRT_LVERTS * 4 / 3)

D3DLVERTEX  AENG_dirt_lvert_buffer[AENG_MAX_DIRT_LVERTS + 1];
D3DLVERTEX *AENG_dirt_lvert;
SLONG       AENG_dirt_lvert_upto;

UWORD AENG_dirt_index[AENG_MAX_DIRT_INDICES];
SLONG AENG_dirt_index_upto;

UBYTE      AENG_dirt_matrix_buffer[sizeof(D3DMATRIX) + 32];
D3DMATRIX *AENG_dirt_matrix;


#define AENG_MAX_DIRT_UVLOOKUP 16

struct
{
	float u;
	float v;

}     AENG_dirt_uvlookup[AENG_MAX_DIRT_UVLOOKUP];
SLONG AENG_dirt_uvlookup_valid;
SWORD AENG_dirt_uvlookup_world_type;


void AENG_draw_dirt()
{
	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	SLONG i;

	#define LEAF_PAGE		(POLY_PAGE_LEAF)
	#define LEAF_CENTRE_U	(0.5F)
	#define LEAF_CENTRE_V	(0.5F)
	#define LEAF_RADIUS		(0.5F)
	#define LEAF_U(a)		(LEAF_CENTRE_U + LEAF_RADIUS * (float)sin(a))
	#define LEAF_V(a)		(LEAF_CENTRE_V + LEAF_RADIUS * (float)cos(a))


	#define SNOW_CENTRE_U	(0.5F)
	#define SNOW_CENTRE_V	(0.5F)
	#define SNOW_RADIUS		(1.0F)

//#ifdef TARGET_DC
	// Slightly more for the DC - not sure why.
	//#define LEAF_UP			12
//#else
	#define LEAF_UP			8
//#endif
	#define LEAF_SIZE       (20.0F+(float)(i&15))

	SLONG j;

	float fyaw;
	float fpitch;
	float froll;
	float ubase;
	float vbase;

	float       matrix[9];
	float       angle;
	SVector_F   temp[4];
	PolyPage   *pp;
	D3DLVERTEX *lv;
	ULONG       rubbish_colour;

	#ifdef TARGET_DC
	ULONG leaf_colour_choice_rgb[4] =
	{
		0xff332d1d,
		0xff243224,
		0xff123320,
		0xff332f07
	};

	ULONG leaf_colour_choice_grey[4] =
	{
		0xff333333,
		0xff444444,
		0xff222222,
		0xff383838
	};
	#else
	ULONG leaf_colour_choice_rgb[4] =
	{
		0x332d1d,
		0x243224,
		0x123320,
		0x332f07
	};

	ULONG leaf_colour_choice_grey[4] =
	{
		0x333333,
		0x444444,
		0x222222,
		0x383838
	};
	#endif

	if (AENG_dirt_uvlookup_valid && AENG_dirt_uvlookup_world_type == world_type)
	{
		//
		// Valid lookup table.
		//
	}
	else
	{
		//
		// Calclate the uvlookup table.
		//

		for (i = 0; i < AENG_MAX_DIRT_UVLOOKUP; i++)
		{
			float angle = float(i) * (2.0F * PI / AENG_MAX_DIRT_UVLOOKUP);

			float cangle;
			float sangle;

			#ifdef TARGET_DC

			_SinCosA(&sangle, &cangle, angle);

			#else

			sangle = sinf(angle);
			cangle = cosf(angle);

			#endif

			//
			// Fix the uv's for texture paging.
			//

			#ifdef TEX_EMBED

			if (world_type == WORLD_TYPE_SNOW)
			{
				pp = &POLY_Page[POLY_PAGE_SNOWFLAKE];
				// And the snowflake texture is bigger and needs a bit of squishing.
				AENG_dirt_uvlookup[i].u = SNOW_CENTRE_U + sangle * SNOW_RADIUS;
				AENG_dirt_uvlookup[i].v = SNOW_CENTRE_V + cangle * SNOW_RADIUS;
			}
			else
			{
				pp = &POLY_Page[POLY_PAGE_LEAF];
				AENG_dirt_uvlookup[i].u = LEAF_CENTRE_U + sangle * LEAF_RADIUS;
				AENG_dirt_uvlookup[i].v = LEAF_CENTRE_V + cangle * LEAF_RADIUS;
			}

			AENG_dirt_uvlookup[i].u = AENG_dirt_uvlookup[i].u * pp->m_UScale + pp->m_UOffset;
			AENG_dirt_uvlookup[i].v = AENG_dirt_uvlookup[i].v * pp->m_VScale + pp->m_VOffset;

			#endif
		}

		AENG_dirt_uvlookup_valid      = TRUE;
		AENG_dirt_uvlookup_world_type = world_type;
	}


	for (i = 0; i < 4; i++)
	{
		leaf_colour_choice_rgb[i] = AENG_colour_mult(leaf_colour_choice_rgb[i], NIGHT_amb_d3d_colour);
	}

	ULONG flag[4];
	ULONG leaf_colour;
	ULONG leaf_specular;

	// Cope with some wacky internals.
	POLY_set_local_rotation_none();
	POLY_flush_local_rot();

	//
	// Initialise the leaf page and MM stuff...
	//

	AENG_dirt_lvert_upto = 0;
	AENG_dirt_index_upto = 0;

	AENG_dirt_lvert  = (D3DLVERTEX *) ((SLONG(AENG_dirt_lvert_buffer) + 31) & ~0x1f);
	AENG_dirt_matrix = (D3DMATRIX  *) ((SLONG(AENG_dirt_matrix_buffer) + 31) & ~0x1f);

	//
	// Draw the dirt.
	//

	DIRT_Dirt *dd;

#ifdef DEBUG
	int iDrawnDirtCount = 0;
#endif
	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_UNUSED)
		{
			continue;
		}

		dd->flag &= ~DIRT_FLAG_DELETE_OK;

		//
		// Is this bit of dirt behind the camera?
		//

		{
			float dx;
			float dy;
			float dz;

			dx = float(dd->x) - AENG_cam_x;
			dy = float(dd->y) - AENG_cam_y;
			dz = float(dd->z) - AENG_cam_z;

			float dprod;

			dprod =
				dx * AENG_cam_matrix[6] +
				dy * AENG_cam_matrix[7] +
				dz * AENG_cam_matrix[8];

			if (dprod < 64.0F)
			{
				//
				// Offscreen...
				//

				DIRT_MARK_AS_OFFSCREEN_QUICK(i);

				goto do_next_dirt;
			}
		}

#ifdef DEBUG
		iDrawnDirtCount++;
#endif

		switch(dd->type)
		{
			case DIRT_TYPE_LEAF:
			case DIRT_TYPE_SNOW:

				{
					//
					// Get four vertices from the leaf page.
					//

					if (AENG_dirt_lvert_upto + 4 > AENG_MAX_DIRT_LVERTS)
					{
						//
						// Draw what we have so far...
						//

						// Cope with some wacky internals.
						POLY_set_local_rotation_none();

						if (world_type == WORLD_TYPE_SNOW)
						{
							POLY_Page[POLY_PAGE_SNOWFLAKE].RS.SetChanged();
						}
						else
						{
							POLY_Page[POLY_PAGE_LEAF].RS.SetChanged();
						}


						the_display.lp_D3D_Device->DrawIndexedPrimitive(
														D3DPT_TRIANGLELIST,
														D3DFVF_LVERTEX,
														AENG_dirt_lvert,
														AENG_dirt_lvert_upto,
														AENG_dirt_index,
														AENG_dirt_index_upto,
														0);

						AENG_dirt_lvert_upto = 0;
						AENG_dirt_index_upto = 0;

						lv = AENG_dirt_lvert;
					}
					else
					{
						lv = &AENG_dirt_lvert[AENG_dirt_lvert_upto];
					}

					if ((i & 0xf) == 0 && !estate && world_type != WORLD_TYPE_SNOW)
					{
						//
						// This is some rubbish...
						//

						fpitch = float(dd->pitch) * (PI / 1024.0F);
						froll  = float(dd->roll)  * (PI / 1024.0F);

						//
						// Copied from MATRIX_calc then fucked with...
						//

						float cy, cp, cr;
						float sy, sp, sr;

						#ifdef TARGET_DC

						// Use the fast intrinsics.
						// Error is 2e-21 at most.

						sy = 0.0F; // sin(0)
						cy = 1.0F; // cos(0)

						_SinCosA ( &sr, &cr, froll );
						_SinCosA ( &sp, &cp, fpitch );

						#else

						sy = 0.0F; // sin(0)
						cy = 1.0F; // cos(0)

						sp = sin(fpitch);
						sr = sin(froll);

						cp = cos(fpitch);
						cr = cos(froll);

						#endif

						//
						// (matrix[3],matrix[4],matrix[5]) remains undefined...
						//

						matrix[0] =  cy * cr + sy * sp * sr;
						matrix[6] =  sy * cp;
						matrix[1] = -cp * sr;
						matrix[7] =  sp;
						matrix[2] = -sy * cr + cy * sp * sr;
						matrix[8] =  cy * cp;

						matrix[0] *= 24.0F;
						matrix[1] *= 24.0F;
						matrix[2] *= 24.0F;

						matrix[6] *= 24.0F;
						matrix[7] *= 24.0F;
						matrix[8] *= 24.0F;

						//
						// Work out the position of the points.
						//

						float base_x = float(dd->x);
						float base_y = float(dd->y + LEAF_UP);
						float base_z = float(dd->z);

						lv[0].x = base_x + matrix[6] + matrix[0];
						lv[0].y = base_y + matrix[7] + matrix[1];
						lv[0].z = base_z + matrix[8] + matrix[2];

						lv[1].x = base_x + matrix[6] - matrix[0];
						lv[1].y = base_y + matrix[7] - matrix[1];
						lv[1].z = base_z + matrix[8] - matrix[2];

						lv[2].x = base_x - matrix[6] + matrix[0];
						lv[2].y = base_y - matrix[7] + matrix[1];
						lv[2].z = base_z - matrix[8] + matrix[2];

						lv[3].x = base_x - matrix[6] - matrix[0];
						lv[3].y = base_y - matrix[7] - matrix[1];
						lv[3].z = base_z - matrix[8] - matrix[2];

						//
						// What are the uv's and colour of this quad?
						//

						rubbish_colour = NIGHT_amb_d3d_colour;

						if (i & 32)
						{
							ubase = 0.0F;
							vbase = 0.0F;
						}
						else
						{
							ubase = 0.5F;
							vbase = 0.0F;
						}

						if (i == 64)
						{
							//
							// Only one bit of money!
							//

							ubase = 0.0F;
							vbase = 0.5F;
						}
						else
						{
							if (!(i & 32))
							{
								if (i & 64)
								{
									rubbish_colour &= 0xffffff00;
								}
							}
						}

						lv[0].tu       = ubase;
						lv[0].tv       = vbase;
						lv[0].color    = rubbish_colour;
						lv[0].specular = 0xff000000;

						lv[1].tu       = ubase + 0.5F;
						lv[1].tv       = vbase;
						lv[1].color    = rubbish_colour;
						lv[1].specular = 0xff000000;

						lv[2].tu       = ubase;
						lv[2].tv       = vbase + 0.5F;
						lv[2].color    = rubbish_colour;
						lv[2].specular = 0xff000000;

						lv[3].tu       = ubase + 0.5F;
						lv[3].tv       = vbase + 0.5F;
						lv[3].color    = rubbish_colour;
						lv[3].specular = 0xff000000;

						#ifdef TEX_EMBED

						pp = &POLY_Page[POLY_PAGE_RUBBISH];

						lv[0].tu = lv[0].tu * pp->m_UScale + pp->m_UOffset;
						lv[0].tv = lv[0].tv * pp->m_VScale + pp->m_VOffset;

						lv[1].tu = lv[1].tu * pp->m_UScale + pp->m_UOffset;
						lv[1].tv = lv[1].tv * pp->m_VScale + pp->m_VOffset;

						lv[2].tu = lv[2].tu * pp->m_UScale + pp->m_UOffset;
						lv[2].tv = lv[2].tv * pp->m_VScale + pp->m_VOffset;

						lv[3].tu = lv[3].tu * pp->m_UScale + pp->m_UOffset;
						lv[3].tv = lv[3].tv * pp->m_VScale + pp->m_VOffset;

						#endif

						//
						// Build the indices.
						//

						ASSERT(AENG_dirt_index_upto + 6 <= AENG_MAX_DIRT_INDICES);

						AENG_dirt_index[AENG_dirt_index_upto + 0] =  AENG_dirt_lvert_upto + 0;
						AENG_dirt_index[AENG_dirt_index_upto + 1] =  AENG_dirt_lvert_upto + 1;
						AENG_dirt_index[AENG_dirt_index_upto + 2] =  AENG_dirt_lvert_upto + 2;

						AENG_dirt_index[AENG_dirt_index_upto + 3] =  AENG_dirt_lvert_upto + 3;
						AENG_dirt_index[AENG_dirt_index_upto + 4] =  AENG_dirt_lvert_upto + 2;
						AENG_dirt_index[AENG_dirt_index_upto + 5] =  AENG_dirt_lvert_upto + 1;

						AENG_dirt_index_upto += 6;
						AENG_dirt_lvert_upto += 4;
					}
					else
					{
						//
						// This is a leaf or snowflake
						//

						float leaf_size = LEAF_SIZE;

						if ((dd->pitch | dd->roll) == 0)
						{
							//
							// This happens often... so we optimise it out.
							//

							lv[0].x = float(dd->x);
							lv[0].y = float(dd->y + LEAF_UP);
							lv[0].z = float(dd->z + leaf_size);

							lv[1].x = float(dd->x + leaf_size);
							lv[1].y = float(dd->y + LEAF_UP);
							lv[1].z = float(dd->z - leaf_size);

							lv[2].x = float(dd->x - leaf_size);
							lv[2].y = float(dd->y + LEAF_UP);
							lv[2].z = float(dd->z - leaf_size);
						}
						else
						{
							//
							// The rotation matrix of this bit of dirt.
							//

							fpitch = float(dd->pitch) * (PI / 1024.0F);
							froll  = float(dd->roll)  * (PI / 1024.0F);

							//
							// Copied from MATRIX_calc then fucked with...
							//

							float cy, cp, cr;
							float sy, sp, sr;

							#ifdef TARGET_DC

							// Use the fast intrinsics.
							// Error is 2e-21 at most.

							sy = 0.0F; // sin(0)
							cy = 1.0F; // cos(0)

							_SinCosA ( &sr, &cr, froll );
							_SinCosA ( &sp, &cp, fpitch );

							#else

							sy = 0.0F; // sin(0)
							cy = 1.0F; // cos(0)

							sp = sin(fpitch);
							sr = sin(froll);

							cp = cos(fpitch);
							cr = cos(froll);

							#endif

							//
							// (matrix[3],matrix[4],matrix[5]) remains undefined...
							//

							matrix[0] =  cy * cr + sy * sp * sr;
							matrix[6] =  sy * cp;
							matrix[1] = -cp * sr;
							matrix[7] =  sp;
							matrix[2] = -sy * cr + cy * sp * sr;
							matrix[8] =  cy * cp;

							matrix[0] *= leaf_size;
							matrix[1] *= leaf_size;
							matrix[2] *= leaf_size;

							matrix[6] *= leaf_size;
							matrix[7] *= leaf_size;
							matrix[8] *= leaf_size;

							//
							// Work out the position of the points.
							//

							lv[0].x  = float(dd->x);
							lv[0].y  = float(dd->y + LEAF_UP);
							lv[0].z  = float(dd->z);

							lv[1].x = lv[0].x - matrix[6] + matrix[0];
							lv[1].y = lv[0].y - matrix[7] + matrix[1];
							lv[1].z = lv[0].z - matrix[8] + matrix[2];

							lv[2].x = lv[0].x - matrix[6] - matrix[0];
							lv[2].y = lv[0].y - matrix[7] - matrix[1];
							lv[2].z = lv[0].z - matrix[8] - matrix[2];

							lv[0].x += matrix[6];
							lv[0].y += matrix[7];
							lv[0].z += matrix[8];
						}

						if (world_type == WORLD_TYPE_SNOW)
						{
							// A snowflake - just subtle shades of grey
							DWORD dwColour = ( ( i & 0x0f ) << 2 ) + 0xc0;
							dwColour *= 0x010101;
							dwColour |= 0xff000000;

							lv[0].color    = dwColour;
							lv[0].specular = 0xff000000;

							lv[1].color    = dwColour;
							lv[1].specular = 0xff000000;

							lv[2].color    = dwColour;
							lv[2].specular = 0xff000000;
						}
						else
						{
							//
							// The colour of this leaf.
							//

							leaf_colour = leaf_colour_choice_rgb[i & 0x3];

							lv[0].color    = (leaf_colour * 3) | 0xff000000;
							lv[0].specular = 0xff000000;

							lv[1].color    = (leaf_colour * 4) | 0xff000000;
							lv[1].specular = 0xff000000;

							lv[2].color    = (leaf_colour * 5) | 0xff000000;
							lv[2].specular = 0xff000000;
						}

						//
						// The rotation angle of the leaf.
						//

						lv[0].tu = AENG_dirt_uvlookup[(i + (AENG_MAX_DIRT_UVLOOKUP * 0 / 3)) & (AENG_MAX_DIRT_UVLOOKUP - 1)].u;
						lv[0].tv = AENG_dirt_uvlookup[(i + (AENG_MAX_DIRT_UVLOOKUP * 0 / 3)) & (AENG_MAX_DIRT_UVLOOKUP - 1)].v;

						lv[1].tu = AENG_dirt_uvlookup[(i + (AENG_MAX_DIRT_UVLOOKUP * 1 / 3)) & (AENG_MAX_DIRT_UVLOOKUP - 1)].u;
						lv[1].tv = AENG_dirt_uvlookup[(i + (AENG_MAX_DIRT_UVLOOKUP * 1 / 3)) & (AENG_MAX_DIRT_UVLOOKUP - 1)].v;

						lv[2].tu = AENG_dirt_uvlookup[(i + (AENG_MAX_DIRT_UVLOOKUP * 2 / 3)) & (AENG_MAX_DIRT_UVLOOKUP - 1)].u;
						lv[2].tv = AENG_dirt_uvlookup[(i + (AENG_MAX_DIRT_UVLOOKUP * 2 / 3)) & (AENG_MAX_DIRT_UVLOOKUP - 1)].v;

						//
						// Build the indices.
						//

						ASSERT(AENG_dirt_index_upto + 3 <= AENG_MAX_DIRT_INDICES);

						AENG_dirt_index[AENG_dirt_index_upto + 0] =  AENG_dirt_lvert_upto + 0;
						AENG_dirt_index[AENG_dirt_index_upto + 1] =  AENG_dirt_lvert_upto + 1;
						AENG_dirt_index[AENG_dirt_index_upto + 2] =  AENG_dirt_lvert_upto + 2;

						AENG_dirt_index_upto += 3;
						AENG_dirt_lvert_upto += 3;
					}
				}

				break;

			case DIRT_TYPE_HELDCAN:

				//
				// Don't draw inside the car?!
				//

				{
					Thing *p_person = TO_THING(dd->droll);	// droll => owner

					if (p_person->Genus.Person->InCar)
					{
						continue;
					}
				}

				//
				// FALLTHROUGH!
				//

			case DIRT_TYPE_CAN:
			case DIRT_TYPE_THROWCAN:

				MESH_draw_poly(
					PRIM_OBJ_CAN,
					dd->x,
					dd->y,
					dd->z,
					dd->yaw,
					dd->pitch,
					dd->roll,
					#ifdef TARGET_DC
					NULL,0xff,0);
					#else
					NULL,0,0);
					#endif

				break;

			case DIRT_TYPE_BRASS:

				extern UBYTE kludge_shrink;

				kludge_shrink = TRUE;

				MESH_draw_poly(
					PRIM_OBJ_ITEM_AMMO_SHOTGUN,
					dd->x,
					dd->y,
					dd->z,
					dd->yaw,
					dd->pitch,
					dd->roll,
					#ifdef TARGET_DC
					NULL,0xff,0);
					#else
					NULL,0,0);
					#endif

				kludge_shrink = FALSE;

				break;

			case DIRT_TYPE_WATER:

				SHAPE_droplet(
					dd->x,
					dd->y,
					dd->z,
					dd->dx >> 2,
					dd->dy >> TICK_SHIFT,
					dd->dz >> 2,
					#ifdef TARGET
					0xff224455,
					#else
					0x00224455,
					#endif
					POLY_PAGE_DROPLET);
				break;

			case DIRT_TYPE_SPARKS:

				SHAPE_droplet(
					dd->x,
					dd->y,
					dd->z,
					dd->dx >> 2,
					dd->dy >> TICK_SHIFT,
					dd->dz >> 2,
					0x7f997744,
					POLY_PAGE_BLOOM1);
				break;

			case DIRT_TYPE_URINE:
				SHAPE_droplet(
					dd->x,
					dd->y,
					dd->z,
					dd->dx >> 2,
					dd->dy >> TICK_SHIFT,
					dd->dz >> 2,
					#ifdef TARGET
					0xff775533,
					#else
					0x00775533,
					#endif
					POLY_PAGE_DROPLET);
				break;

			case DIRT_TYPE_BLOOD:
				SHAPE_droplet(
					dd->x,
					dd->y,
					dd->z,
					dd->dx >> 2,
					dd->dy >> TICK_SHIFT,
					dd->dz >> 2,
					0x9fFFFFFF,
					POLY_PAGE_BLOODSPLAT);
				break;

			default:
				ASSERT(0);
				break;
		}



#if 0
/*
		switch(di.type)
		{
			case DIRT_INFO_TYPE_WATER:

				SHAPE_droplet(
					di.x,
					di.y,
					di.z,
					di.dx * 4,
					di.dy * 4,
					di.dz * 4,
#ifdef TARGET
					0xff224455,
#else
					0x00224455,
#endif
					POLY_PAGE_DROPLET);

				break;

			case DIRT_INFO_TYPE_URINE:

				SHAPE_droplet(
					di.x,
					di.y,
					di.z,
					di.dx * 4,
					di.dy * 4,
					di.dz * 4,
#ifdef TARGET
					0xff775533,
#else
					0x00775533,
#endif
					POLY_PAGE_DROPLET);

				break;

			case DIRT_INFO_TYPE_SPARKS:

				SHAPE_droplet(
					di.x,
					di.y,
					di.z,
					di.dx * 4,
					di.dy * 4,
					di.dz * 4,
					0x7f997744,
					POLY_PAGE_BLOOM1);

				break;

			case DIRT_INFO_TYPE_BLOOD:

				SHAPE_droplet(
					di.x,
					di.y,
					di.z,
					di.dx * 4,
					di.dy * 4,
					di.dz * 4,
					0x9fFFFFFF,
					POLY_PAGE_BLOODSPLAT);

				break;

			case DIRT_INFO_TYPE_SNOW:
				leaf_colour=di.morph1;
				leaf_colour<<=23;
				leaf_colour|=0xffFFff;
				SPRITE_draw_tex(di.x,di.y,di.z,20,leaf_colour,0xFF000000,POLY_PAGE_SNOWFLAKE,0.0,0.0,1.0,1.0,SPRITE_SORT_NORMAL);
				break;

			case DIRT_INFO_TYPE_LEAF:

				//
				// Create the rotation matrix for this bit of dirt...
				//

				if ((di.pitch | di.roll) == 0)
				{

				}

				//
				// There is a chance we are going to draw some rubbish instead of a leaf.
				//

				if ((i & 0xf) == 0 && estate==0)
				{
					//
					// The rotation matrix of this bit of dirt.
					//

					fpitch = float(di.pitch) * (PI / 1024.0F);
					froll  = float(di.roll)  * (PI / 1024.0F);
					fyaw   = float(i);

					MATRIX_calc(matrix, fyaw, fpitch, froll);

					matrix[0] *= 24.0F;
					matrix[1] *= 24.0F;
					matrix[2] *= 24.0F;

					matrix[6] *= 24.0F;
					matrix[7] *= 24.0F;
					matrix[8] *= 24.0F;

					temp[0].X = float(di.x) + matrix[6] + matrix[0];
					temp[0].Y = float(di.y) + matrix[7] + matrix[1];
					temp[0].Z = float(di.z) + matrix[8] + matrix[2];

					temp[1].X = float(di.x) + matrix[6] - matrix[0];
					temp[1].Y = float(di.y) + matrix[7] - matrix[1];
					temp[1].Z = float(di.z) + matrix[8] - matrix[2];

					temp[2].X = float(di.x) - matrix[6] + matrix[0];
					temp[2].Y = float(di.y) - matrix[7] + matrix[1];
					temp[2].Z = float(di.z) - matrix[8] + matrix[2];

					temp[3].X = float(di.x) - matrix[6] - matrix[0];
					temp[3].Y = float(di.y) - matrix[7] - matrix[1];
					temp[3].Z = float(di.z) - matrix[8] - matrix[2];

					//
					// Transform the points.
					//

					for (j = 0; j < 4; j++)
					{
						POLY_transform(
							temp[j].X,
							temp[j].Y + 4.0F,
							temp[j].Z,
						   &pp[j]);

						if (!pp[j].IsValid())
						{
							//
							// Tell the DIRT module that the leaf is off-screen.
							//

							DIRT_mark_as_offscreen(i);

							//
							// Don't bother transforming the other points.
							//

							goto do_next_dirt;
						}
					}

					if (POLY_valid_quad(quad))
					{
						float ubase;
						float vbase;

						SLONG colour_and = 0xffffffff;

						if (i & 32)
						{
							ubase = 0.0F;
							vbase = 0.0F;
						}
						else
						{
							ubase = 0.5F;
							vbase = 0.0F;
						}

						if (i == 64)
						{
							//
							// Only one bit of money!
							//

							ubase = 0.0F;
							vbase = 0.5F;
						}
						else
						{
							if (!(i & 32))
							{
								if (i & 64)
								{
									colour_and = 0xffffff00;
								}
							}
						}

						//
						// Set the uvs.
						//

						for (j = 0; j < 4; j++)
						{
							pp[j].u = ubase;
							pp[j].v = vbase;

							if (j & 1) {pp[j].u += 0.5F;}
							if (j & 2) {pp[j].v += 0.5F;}

#ifdef TARGET_DC
							pp[j].colour   = ( NIGHT_amb_d3d_colour & colour_and ) | 0xff000000;
#else
							pp[j].colour   = NIGHT_amb_d3d_colour & colour_and;
#endif
							pp[j].specular = 0xff000000;
						}

						//
						// Draw the quad.
						//

						POLY_add_quad(quad, POLY_PAGE_RUBBISH, FALSE);
					}
					else
					{
						//
						// Tell the DIRT module that the leaf is off-screen.
						//

						DIRT_mark_as_offscreen(i);
					}
				}
				else
				{
					if ((di.yaw | di.pitch | di.roll) == 0)
					{
						//
						// This happens often... so we optimise it out.
						//

						temp[0].X = float(di.x);
						temp[0].Y = float(di.y + LEAF_UP);
						temp[0].Z = float(di.z + LEAF_SIZE);

						temp[1].X = float(di.x + LEAF_SIZE);
						temp[1].Y = float(di.y + LEAF_UP);
						temp[1].Z = float(di.z - LEAF_SIZE);

						temp[2].X = float(di.x - LEAF_SIZE);
						temp[2].Y = float(di.y + LEAF_UP);
						temp[2].Z = float(di.z - LEAF_SIZE);
					}
					else
					{
						//
						// The rotation matrix of this bit of dirt.
						//

						fyaw   = float(di.yaw)   * (PI / 1024.0F);
						fpitch = float(di.pitch) * (PI / 1024.0F);
						froll  = float(di.roll)  * (PI / 1024.0F);

						MATRIX_calc(matrix, fyaw, fpitch, froll);

						//
						// Work out the position of the points.
						//

						for (j = 0; j < 3; j++)
						{

							temp[j].X  = float(di.x);
							temp[j].Y  = float(di.y);
							temp[j].Z  = float(di.z);

							temp[j].Y += float(LEAF_UP);
						}


						temp[0].X += matrix[6] * LEAF_SIZE;
						temp[0].Y += matrix[7] * LEAF_SIZE;
						temp[0].Z += matrix[8] * LEAF_SIZE;

						temp[1].X -= matrix[6] * LEAF_SIZE;
						temp[1].Y -= matrix[7] * LEAF_SIZE;
						temp[1].Z -= matrix[8] * LEAF_SIZE;

						temp[2].X -= matrix[6] * LEAF_SIZE;
						temp[2].Y -= matrix[7] * LEAF_SIZE;
						temp[2].Z -= matrix[8] * LEAF_SIZE;

						temp[1].X += matrix[0] * LEAF_SIZE;
						temp[1].Y += matrix[1] * LEAF_SIZE;
						temp[1].Z += matrix[2] * LEAF_SIZE;

						temp[2].X -= matrix[0] * LEAF_SIZE;
						temp[2].Y -= matrix[1] * LEAF_SIZE;
						temp[2].Z -= matrix[2] * LEAF_SIZE;

						falling = TRUE;
					}

					//
					// Transform the points.
					//

					for (j = 0; j < 3; j++)
					{
						POLY_transform(
							temp[j].X,
							temp[j].Y,
							temp[j].Z,
						   &pp[j]);

						if (!pp[j].IsValid())
						{
							//
							// Tell the DIRT module that the leaf is off-screen.
							//

							DIRT_mark_as_offscreen(i);

							//
							// Don't bother transforming the other points.
							//

							goto do_next_dirt;
						}
					}

					if (POLY_valid_triangle(tri))
					{
						//
						// The colour and texture of the leaf.
						//

						if (POLY_force_additive_alpha)
						{
							leaf_colour = leaf_colour_choice_grey[i & 0x3];
						}
						else
						{
							leaf_colour = leaf_colour_choice_rgb[i & 0x3];
							leaf_colour = AENG_colour_mult(leaf_colour, NIGHT_amb_d3d_colour);
						}

						angle = float(i);

						for (j = 0; j < 3; j++)
						{
						    pp[j].colour =  leaf_colour * (j + 3);
							pp[j].colour  &= ~POLY_colour_restrict;
#ifdef TARGET_DC
							pp[j].colour |= 0xff000000;
#endif
							pp[j].specular =  0xff000000;
							pp[j].u        =  LEAF_U(angle);
							pp[j].v        =  LEAF_V(angle);

							angle += 2.0F * PI / 3.0F;
						}

						POLY_add_triangle(tri, LEAF_PAGE, FALSE);
					}
					else
					{
						//
						// Tell the DIRT module that the leaf is off-screen.
						//

						DIRT_mark_as_offscreen(i);
					}

				}

				break;

			case DIRT_INFO_TYPE_PRIM:

				extern UBYTE kludge_shrink;

				if (di.held||(di.prim==253))
				{
					kludge_shrink = TRUE;
				}

				MESH_draw_poly(
					di.prim,
					di.x,
					di.y,
					di.z,
					di.yaw,
					di.pitch,
					di.roll,
#ifdef TARGET_DC
					NULL,0xff,0);
#else
					NULL,0,0);
#endif

				kludge_shrink = FALSE;

				break;

			case DIRT_INFO_TYPE_MORPH:

				MESH_draw_morph(
					di.prim,
					di.morph1,
					di.morph2,
					di.tween,
					di.x,
					di.y,
					di.z,
					di.yaw,
					di.pitch,
					di.roll,
					NULL);

				break;

			default:
				ASSERT(0);
				break;
		}
*/
#endif

	  do_next_dirt:;

	}

	//
	// Draw left-over leaves.
	//

	if (AENG_dirt_lvert_upto)
	{
		// Cope with some wacky internals.
		POLY_set_local_rotation_none();

		if (world_type == WORLD_TYPE_SNOW)
		{
			POLY_Page[POLY_PAGE_SNOWFLAKE].RS.SetChanged();
		}
		else
		{
			POLY_Page[POLY_PAGE_LEAF].RS.SetChanged();
		}

		the_display.lp_D3D_Device->DrawIndexedPrimitive(
										D3DPT_TRIANGLELIST,
										D3DFVF_LVERTEX,
										AENG_dirt_lvert,
										AENG_dirt_lvert_upto,
										AENG_dirt_index,
										AENG_dirt_index_upto,
										0);
	}

	//TRACE ( "Drew %i bits of dirt\n", iDrawnDirtCount );
}


//
// A bucket list for the pows!
//

typedef struct aeng_pow
{
	SLONG frame;
	float sx;
	float sy;
	float sz;
	float Z;

	struct aeng_pow *next;

} AENG_Pow;

#define AENG_MAX_POWS 256

AENG_Pow AENG_pow[AENG_MAX_POWS];
SLONG    AENG_pow_upto;

#define AENG_POW_NUM_BUCKETS 1024

AENG_Pow *AENG_pow_bucket[AENG_POW_NUM_BUCKETS];

//
// Draws the POWS
//

void AENG_draw_pows(void)
{
	SLONG pow;
	SLONG sprite;
	SLONG bucket;

	AENG_Pow   *ap;
	POW_Pow    *pp;
	POW_Sprite *ps;

	POLY_Point pt;

	//
	// Clear the buckets and the AENG_pows.
	//

	memset(AENG_pow_bucket, 0, sizeof(AENG_pow_bucket));

	AENG_pow_upto = 0;

	//
	// Draw all used pows. Ignore the NGAMUT for now.
	//

	for (pow = POW_pow_used; pow; pow = pp->next)
	{
		ASSERT(WITHIN(pow, 1, POW_MAX_POWS - 1));

		pp = &POW_pow[pow];

		for (sprite = pp->sprite; sprite; sprite = ps->next)
		{
			ASSERT(WITHIN(sprite, 1, POW_MAX_SPRITES - 1));

			ps = &POW_sprite[sprite];

			POLY_transform(
				float(ps->x) * (1.0F / 256.0F),
				float(ps->y) * (1.0F / 256.0F),
				float(ps->z) * (1.0F / 256.0F),
			   &pt);

			if (pt.clip & POLY_CLIP_TRANSFORMED)
			{
				//
				// Create an AENG_pow.
				//

				ASSERT(WITHIN(AENG_pow_upto, 0, AENG_MAX_POWS - 1));

				ap = &AENG_pow[AENG_pow_upto++];

				ap->frame = ps->frame;
				ap->sx    = pt.X;
				ap->sy    = pt.Y;
				ap->sz    = pt.z;
				ap->Z     = pt.Z;
				ap->next  = NULL;

				//
				// Add to the bucket list.
				//

				bucket = ftol(pt.z * float(AENG_POW_NUM_BUCKETS));

				SATURATE(bucket, 0, AENG_POW_NUM_BUCKETS - 1);

				ap->next                = AENG_pow_bucket[bucket];
				AENG_pow_bucket[bucket]	= ap;
			}
		}
	}

	//
	// Draw the buckets.
	//

	{
		POLY_Point  ppt[4];
		POLY_Point *quad[4];

		float size;
		float u;
		float v;

		ppt[0].colour = 0xffffffff;
		ppt[1].colour = 0xffffffff;
		ppt[2].colour = 0xffffffff;
		ppt[3].colour = 0xffffffff;

		ppt[0].specular = 0xff000000;
		ppt[1].specular = 0xff000000;
		ppt[2].specular = 0xff000000;
		ppt[3].specular = 0xff000000;

		quad[0] = &ppt[0];
		quad[1] = &ppt[1];
		quad[2] = &ppt[2];
		quad[3] = &ppt[3];

		for (bucket = 0; bucket < AENG_POW_NUM_BUCKETS; bucket++)
		{
			for (ap = AENG_pow_bucket[bucket]; ap; ap = ap->next)
			{
				//
				// Push forward in the z-buffer
				//

				ap->sz -= 0.025F;	// Half a mapsquare!

				if (ap->sz < POLY_ZCLIP_PLANE)
				{
					 ap->sz = POLY_ZCLIP_PLANE;
				}

				ap->Z = POLY_ZCLIP_PLANE / ap->sz;

				//
				// The frame.
				//

				u = float(ap->frame & 0x3) * (1.0F / 4.0F);
				v = float(ap->frame >>  2) * (1.0F / 4.0F);

				//
				// Add this sprite to the bucket list.
				//

				size = 650.0F * ap->Z;

				ppt[0].X = ap->sx - size;
				ppt[0].Y = ap->sy - size;
				ppt[1].X = ap->sx + size;
				ppt[1].Y = ap->sy - size;
				ppt[2].X = ap->sx - size;
				ppt[2].Y = ap->sy + size;
				ppt[3].X = ap->sx + size;
				ppt[3].Y = ap->sy + size;

				ppt[0].u = u + (0.0F / 4.0F);
				ppt[0].v = v + (0.0F / 4.0F);
				ppt[1].u = u + (1.0F / 4.0F);
				ppt[1].v = v + (0.0F / 4.0F);
				ppt[2].u = u + (0.0F / 4.0F);
				ppt[2].v = v + (1.0F / 4.0F);
				ppt[3].u = u + (1.0F / 4.0F);
				ppt[3].v = v + (1.0F / 4.0F);

				ppt[0].Z = ap->Z;
				ppt[1].Z = ap->Z;
				ppt[2].Z = ap->Z;
				ppt[3].Z = ap->Z;

				ppt[0].z = ap->sz;
				ppt[1].z = ap->sz;
				ppt[2].z = ap->sz;
				ppt[3].z = ap->sz;

				POLY_add_quad(quad, POLY_PAGE_EXPLODE1, FALSE, TRUE);
			}
		}
	}
}


#ifndef TARGET_DC
void AENG_draw_released_balloons(void)
{
	SLONG i;

	BALLOON_Balloon *bb;

	for (i = 1; i < BALLOON_balloon_upto; i++)
	{
		bb = &BALLOON_balloon[i];

		if (bb->type && !bb->thing)
		{
			//
			// Nobody is holding this balloon so we have to draw it here.
			//

			SHAPE_draw_balloon(i);
		}
	}
}
#endif


//
// AA test.
//

#define AENG_AA_LEFT	 20
#define AENG_AA_TOP		 20
#define AENG_AA_PIX_SIZE  4
#define AENG_AA_BUF_SIZE 32

UBYTE AENG_aa_buffer[AENG_AA_BUF_SIZE][AENG_AA_BUF_SIZE];

#ifdef TARGET_DC
// Try to misalign the map rows to try to stop the cache thrashing.
// DC's cache is 32k and 1-way asssociative!
//#define MAP_SIZE_TWEAK 4
// Actually, it seemed to make very little difference.
#define MAP_SIZE_TWEAK 0
#else
#define MAP_SIZE_TWEAK 0
#endif


#ifdef TARGET_DC
#define	NEW_FLOOR defined
#else
#define	NEW_FLOOR defined
#endif

#ifndef	NEW_FLOOR
POLY_Point AENG_upper[MAP_WIDTH / 2 + MAP_SIZE_TWEAK][MAP_HEIGHT / 2 + MAP_SIZE_TWEAK];
POLY_Point AENG_lower[MAP_WIDTH / 2 + MAP_SIZE_TWEAK*2][MAP_HEIGHT / 2 + MAP_SIZE_TWEAK*2];
#endif


//
// Globals affecting the way the engine works.
//

#define AENG_SKY_TYPE_NIGHT	0
#define AENG_SKY_TYPE_DAY	1

SLONG AENG_torch_on       = FALSE;
SLONG AENG_shadows_on     = TRUE;
SLONG AENG_sky_type       = AENG_SKY_TYPE_DAY;
ULONG AENG_sky_colour_bot = 0x008890ee;
ULONG AENG_sky_colour_top = 0x006670cc;


void AENG_set_sky_nighttime()
{
	AENG_sky_type = AENG_SKY_TYPE_NIGHT;
}

void AENG_set_sky_daytime(ULONG bottom_colour, ULONG top_colour)
{
	AENG_sky_type       = AENG_SKY_TYPE_DAY;
	AENG_sky_colour_bot = bottom_colour;
	AENG_sky_colour_top = top_colour;
}

struct	RRect
{
	SLONG	x;
	SLONG y;
	SLONG w;
	SLONG h;
	SLONG col;
	SLONG layer;
	SLONG page;

};

struct	RRect	rrect[2000];
SLONG	next_rrect=1;



void	AENG_draw_rectr(SLONG x,SLONG y,SLONG w,SLONG h,SLONG col,SLONG layer,SLONG page)
{
	ASSERT(next_rrect<2000);
	rrect[next_rrect].x=x;
	rrect[next_rrect].y=y;
	rrect[next_rrect].w=w;
	rrect[next_rrect].h=h;
	rrect[next_rrect].col=col;
	rrect[next_rrect].layer=layer;
	rrect[next_rrect].page=page;
	next_rrect++;

}

void	AENG_draw_rect(SLONG x,SLONG y,SLONG w,SLONG h,SLONG col,SLONG layer,SLONG page);
void	draw_all_boxes(void)
{
	SLONG x,y,w,h, col,layer,page;
	SLONG	c0;



	for(c0=1;c0<next_rrect;c0++)
	{
		x=rrect[c0].x;
		y=rrect[c0].y;
		w=rrect[c0].w;
		h=rrect[c0].h;
		col=rrect[c0].col;
		layer=rrect[c0].layer;
		page=rrect[c0].page;

		AENG_draw_rect(x,y,w,h,col,layer,page);
	}
	next_rrect=0;
}
void	AENG_draw_rect(SLONG x,SLONG y,SLONG w,SLONG h,SLONG col,SLONG layer,SLONG page)
{
	float	offset=0.0;
	POLY_Point  pp  [4];
	POLY_Point *quad[4];
	float	top,bottom,left,right;

	offset=((float)layer)*0.0001f;

	top=(float)y;
	bottom=(float)(y+h);
	left=(float)x;
	right=(float)(x+w);




	#define AENG_BACKGROUND_COLOUR 0x55888800

	pp[0].X        = left;
	pp[0].Y        = top;
	pp[0].z        = 0.0F+offset;
	pp[0].Z        = 1.0F-offset;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = col;
	pp[0].specular = 0;

	pp[1].X        = right;
	pp[1].Y        = top;
	pp[1].z        = 0.0F+offset;
	pp[1].Z        = 1.0F-offset;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = col;
	pp[1].specular = 0;

	pp[2].X        = left;
	pp[2].Y        = bottom;
	pp[2].z        = 0.0F+offset;
	pp[2].Z        = 1.0F-offset;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = col;
	pp[2].specular = 0;

	pp[3].X        = right;
	pp[3].Y        = bottom;
	pp[3].z        = 0.0F+offset;
	pp[3].Z        = 1.0F-offset;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[3].colour   = col;
	pp[3].specular = 0;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, page, FALSE, TRUE);

}

void	AENG_draw_col_tri(SLONG x0,SLONG y0,SLONG col0,SLONG x1,SLONG y1,SLONG col1,SLONG x2,SLONG y2,SLONG col2,SLONG layer)
{
	float	offset=0.0;
	POLY_Point  pp  [4];
	POLY_Point *tri[4];
	POLY_Point *quad[4];

	float	left,right,top,bottom;

	left=100.0;
	right=200.0;
	top=100.0;
	bottom=200.0;

	offset=((float)layer)*0.0001f;

/*
	#define AENG_BACKGROUND_COLOUR 0x55888800

	pp[0].X        = left;
	pp[0].Y        = top;
	pp[0].z        = 0.0F;
	pp[0].Z        = 1.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = AENG_BACKGROUND_COLOUR;
	pp[0].specular = 0;

	pp[1].X        = right;
	pp[1].Y        = top;
	pp[1].z        = 0.0F;
	pp[1].Z        = 1.0F;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = AENG_BACKGROUND_COLOUR;
	pp[1].specular = 0;

	pp[2].X        = left;
	pp[2].Y        = bottom;
	pp[2].z        = 0.0F;
	pp[2].Z        = 1.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = AENG_BACKGROUND_COLOUR;
	pp[2].specular = 0;

	pp[3].X        = right;
	pp[3].Y        = bottom;
	pp[3].z        = 0.0F;
	pp[3].Z        = 1.0F;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[3].colour   = AENG_BACKGROUND_COLOUR;
	pp[3].specular = 0;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE, TRUE);
*/


	#define AENG_BACKGROUND_COLOUR 0x55888800

	pp[0].X        = (float)x0;
	pp[0].Y        = (float)y0;
//	pp[0].X        = left;
//	pp[0].Y        = top;
	pp[0].z        = 0.0F+offset;
	pp[0].Z        = 1.0F-offset;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = col0; //AENG_BACKGROUND_COLOUR;
	pp[0].specular = 0;

	pp[1].X        = (float)x1;
	pp[1].Y        = (float)y1;
//	pp[1].X        = right;
//	pp[1].Y        = top;
	pp[1].z        = 0.0F+offset;
	pp[1].Z        = 1.0F-offset;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = col1; //AENG_BACKGROUND_COLOUR;
	pp[1].specular = 0;

	pp[2].X        = (float)x2;
	pp[2].Y        = (float)y2;
	pp[2].z        = 0.0F+offset;
	pp[2].Z        = 1.0F-offset;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = col2; //AENG_BACKGROUND_COLOUR;
	pp[2].specular = 0;

	tri[0] = &pp[0];
	tri[1] = &pp[1];
	tri[2] = &pp[2];

	POLY_add_triangle(tri, POLY_PAGE_COLOUR, FALSE, TRUE);
}


void	show_gamut_lo(SLONG	x,SLONG z)
{
	return;

//	AENG_draw_rect(x*8,z*8,8,8,0xff0000,2,POLY_PAGE_COLOUR);
}

void	show_gamut_hi(SLONG	x,SLONG z)
{
	return;

//	AENG_draw_rect(x*2,z*2,1,1,0xffff00,1,POLY_PAGE_COLOUR);
}


#if 1
// Bin this.
#define show_facet(thing) sizeof(thing)

#else

void	show_facet(SLONG facet)
{
	return;

	struct	DFacet	*p_facet;
	SLONG	x1,z1,x2,z2;
	SLONG	colour=0xffffff;


	p_facet=&dfacets[facet];

	x1=p_facet->x[0];
	x2=p_facet->x[1];
	z1=p_facet->z[0];
	z2=p_facet->z[1];

	x1*=2;
	z1*=2;
	x2*=2;
	z2*=2;

	x1+=(p_facet->Y[0]>>8);
	z1-=(p_facet->Y[0]>>8);
	x2+=(p_facet->Y[0]>>8);
	z2-=(p_facet->Y[0]>>8);

	switch(p_facet->Height)
	{
		case	2:
			colour=0xff;
			break;
		case	3:
			colour=0xff00;
			break;
		case	4:
			colour=0x7f7f;
			break;
		case	5:
			colour=0xffff;
			break;
	}

	POLY_add_line_2d( (float)x1,(float)z1,(float)x2,(float)z2,colour);

}

#endif


void AENG_draw_people_messages()
{
	return;

	SLONG x;
	SLONG z;

	SLONG  t_index;
	Thing *p_thing;

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (p_thing->Flags & FLAGS_IN_BUILDING)
				{
					//
					// Dont draw things inside buildings when we are outdoors.
					//
				}
				else
				{
					switch(p_thing->DrawType)
					{
						case DT_ROT_MULTI:

							if (POLY_sphere_visible(
									float(p_thing->WorldPos.X >> 8),
									float(p_thing->WorldPos.Y >> 8) + KERB_HEIGHT,
									float(p_thing->WorldPos.Z >> 8),
									256.0F / (AENG_DRAW_DIST * 256.0F)))
							{
								CBYTE	str[100];
//								FIGURE_draw(p_thing);

								sprintf(str,"%d %d",p_thing->State,p_thing->SubState);
								AENG_world_text(
									(p_thing->WorldPos.X >> 8),
									(p_thing->WorldPos.Y >> 8) + 0x60,
									(p_thing->WorldPos.Z >> 8),
									200,
									180,
									50,
									TRUE,
									str);
//									PCOM_person_state_debug(p_thing));


							}

							break;
					}
				}

				t_index = p_thing->Child;
			}
		}
	}
}




//
// Sets the rotation of the bike wheel prim for the given rotation.
//

void AENG_set_bike_wheel_rotation(UWORD rot, UBYTE prim)
{
	SLONG i;

	PrimObject *po;
	PrimFace4  *f4;

	po = &prim_objects[prim];

	//
	// The texture rotation vector.
	//



// Oh no they're not.
#if 0
	// All the textures are symmetrical, so you can't tell if they rotate.
	// Optimised!
	const SLONG du1 = 0xf;
	const SLONG dv1 = 0;

	const SLONG du2 = 0xf;
	const SLONG dv2 = 0;

	SLONG u;
	SLONG v;

	static SLONG order[4] = {2, 1, 3, 0};

	//
	// The faces we rotate the textures on are faces 6 and 7 for PRIM_OBJ_BIKE_BWHEEL.
	//

	f4 = &prim_faces4[po->StartFace4 + 6];

	for (i = 0; i < 4; i++)
	{
		switch(order[i])
		{
			case 0: u = 16 + du1; v = 16 + dv1; break;
			case 1: u = 16 + dv1; v = 16 - du1; break;
			case 2: u = 16 - du1; v = 16 - dv1; break;
			case 3: u = 16 - dv1; v = 16 + du1; break;
		}

		f4[0].UV[i][0] &= ~0x3f;
		f4[0].UV[i][1] &= ~0x3f;

		f4[0].UV[i][0] |= u;
		f4[0].UV[i][1] |= v;

		switch(order[i])
		{
			case 0: u = 16 + du2; v = 16 + dv2; break;
			case 1: u = 16 + dv2; v = 16 - du2; break;
			case 2: u = 16 - du2; v = 16 - dv2; break;
			case 3: u = 16 - dv2; v = 16 + du2; break;
		}

		f4[1].UV[i][0] &= ~0x3f;
		f4[1].UV[i][1] &= ~0x3f;

		f4[1].UV[i][0] |= u;
		f4[1].UV[i][1] |= v;
	}

#else

	SLONG du1 = SIN(+rot & 2047) * 15 >> 16;
	SLONG dv1 = COS(+rot & 2047) * 15 >> 16;

	SLONG du2 = SIN(-rot & 2047) * 15 >> 16;
	SLONG dv2 = COS(-rot & 2047) * 15 >> 16;

	SLONG u;
	SLONG v;

	static SLONG order[4] = {2, 1, 3, 0};

	//
	// The faces we rotate the textures on are faces 6 and 7 for PRIM_OBJ_BIKE_BWHEEL.
	//

	f4 = &prim_faces4[po->StartFace4 + 6];

	for (i = 0; i < 4; i++)
	{
		switch(order[i])
		{
			case 0: u = 16 + du1; v = 16 + dv1; break;
			case 1: u = 16 + dv1; v = 16 - du1; break;
			case 2: u = 16 - du1; v = 16 - dv1; break;
			case 3: u = 16 - dv1; v = 16 + du1; break;
		}

		f4[0].UV[i][0] &= ~0x3f;
		f4[0].UV[i][1] &= ~0x3f;

		f4[0].UV[i][0] |= u;
		f4[0].UV[i][1] |= v;

		switch(order[i])
		{
			case 0: u = 16 + du2; v = 16 + dv2; break;
			case 1: u = 16 + dv2; v = 16 - du2; break;
			case 2: u = 16 - du2; v = 16 - dv2; break;
			case 3: u = 16 - dv2; v = 16 + du2; break;
		}

		f4[1].UV[i][0] &= ~0x3f;
		f4[1].UV[i][1] &= ~0x3f;

		f4[1].UV[i][0] |= u;
		f4[1].UV[i][1] |= v;
	}
#endif
}



/*

//
// Draws warehouse floor surrounding the given facet.
//

void AENG_draw_warehouse_floor_near_door(DFacet *df)
{
	SLONG dx;
	SLONG dz;

	SLONG sx;
	SLONG sz;

	SLONG lx1;
	SLONG lx2;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG mx;
	SLONG mz;
	SLONG page;

	POLY_Point *quad[4];
	PAP_Hi     *ph;

	//
	// The bounding box of the points.
	//

	x1 = df->x[0];
	z1 = df->z[0];
	x2 = df->x[1];
	z2 = df->z[1];

	if (x1 > x2) {SWAP(x1,x2);}
	if (z1 > z2) {SWAP(z1,z2);}

	x1 -= 1;
	x2 += 1;

	z1 -= 1;
	z2 += 1;

	SATURATE(x1, 1, PAP_SIZE_HI - 2);
	SATURATE(z1, 1, PAP_SIZE_HI - 2);
	SATURATE(x2, 1, PAP_SIZE_HI - 2);
	SATURATE(z2, 1, PAP_SIZE_HI - 2);

	//
	// Set the colour of the points depending on their distance
	// from from the entrance of the warehouse.
	//

	for (mz = z1; mz <= z2; mz++)
	{
		if (!WITHIN(mz, NGAMUT_point_zmin, NGAMUT_point_zmax))
		{
			continue;
		}

		lx1 = x1;
		lx2 = x2;

		SATURATE(lx1, NGAMUT_point_gamut[mz].xmin, NGAMUT_point_gamut[mz].xmax);
		SATURATE(lx2, NGAMUT_point_gamut[mz].xmin, NGAMUT_point_gamut[mz].xmax);

		for (mx = lx1; mx <= lx2; mx++)
		{
			//
			// Remember the upper point colour.
			//

			AENG_lower[mx][mz].colour = AENG_upper[mx][mz].colour;

			//
			// Darken this point if it is not adjacent to the exit.
			//

			for (dx = -1; dx <= 0; dx++);
			for (dz = -1; dz <= 0; dz++);
			{
				sx = mx + dx;
				sz = mz + dz;

				if (!(PAP_2HI(sx,sz).Flags & PAP_FLAG_HIDDEN))
				{
					goto dont_darken;
				}
			}

			//
			// Completely fogged out...
			//

			AENG_upper[mx][mz].colour &= 0x00ffffff;

		  dont_darken:;
		}
	}

	for (mz = z1; mz < z2; mz++)
	{
		if (!WITHIN(mz, NGAMUT_zmin, NGAMUT_zmax))
		{
			continue;
		}

		lx1 = x1;
		lx2 = x2;

		SATURATE(lx1, NGAMUT_gamut[mz].xmin, NGAMUT_gamut[mz].xmax);
		SATURATE(lx2, NGAMUT_gamut[mz].xmin, NGAMUT_gamut[mz].xmax);

		for (mx = lx1; mx < lx2; mx++)
		{
			ph = &PAP_2HI(mx,mz);

			if (!(ph->Flags & PAP_FLAG_HIDDEN))
			{
				continue;
			}

			quad[0] = &AENG_upper[mx + 0][mz + 0];
			quad[1] = &AENG_upper[mx + 1][mz + 0];
			quad[2] = &AENG_upper[mx + 0][mz + 1];
			quad[3] = &AENG_upper[mx + 1][mz + 1];

			if (POLY_valid_quad(quad))
			{
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

				POLY_add_quad(quad, page, TRUE);
			}
		}
	}
}

*/



// return current detail levels

//#pragma optimize( "a", off )
//#pragma optimize( "y", off )

// store new detail levels

#ifdef TARGET_DC
// Fewer things to set.
void AENG_set_detail_levels(//int stars,
							int shadows,
							int puddles,
							int dirt,
							int mist,
							int rain,
							int skyline,
							int crinkles)
#else
void AENG_set_detail_levels(int stars,
							int shadows,
							int moon_reflection,
							int people_reflection,
							int puddles,
							int dirt,
							int mist,
							int rain,
							int skyline,
							int filter,
							int perspective,
							int crinkles)
#endif
{
	//ENV_set_value_number("detail_stars", stars, "Render");
	ENV_set_value_number("detail_shadows", shadows, "Render");
	//ENV_set_value_number("detail_moon_reflection", moon_reflection, "Render");
	//ENV_set_value_number("detail_people_reflection", people_reflection, "Render");
	ENV_set_value_number("detail_puddles", puddles, "Render");
	ENV_set_value_number("detail_dirt", dirt, "Render");
	ENV_set_value_number("detail_mist", mist, "Render");
	ENV_set_value_number("detail_rain", rain, "Render");
	ENV_set_value_number("detail_skyline", skyline, "Render");
	//ENV_set_value_number("detail_filter", filter, "Render");
	//ENV_set_value_number("detail_perspective", perspective, "Render");
	ENV_set_value_number("detail_crinkles", crinkles, "Render");

	// heh heh heh
	AENG_read_detail_levels();
}

#ifndef TARGET_DC
// draw some polys

float AENG_draw_some_polys(bool large, bool blend)
{
	PolyPoint2D		*vert,*vp;
	WORD			*ind,*ip;

	vert = new PolyPoint2D[large ? 300 : 30000];
	vp = vert;

	ind = new WORD[large ? 300 : 30000];
	ip = ind;

	float	u = 0;
	float	v = 0;

	if (large)
	{
		for (int ii = 0; ii < 100; ii++)
		{
			vp->SetSC(0,0);
			vp->SetColour(0x80FFFFFF);
			vp->SetSpecular(0);
			vp->SetUV(u,v);
			vp++;

			vp->SetSC(640,0);
			vp->SetColour(0x80FFFFFF);
			vp->SetSpecular(0);
			vp->SetUV(u,v);
			vp++;

			vp->SetSC(0,480);
			vp->SetColour(0x80FFFFFF);
			vp->SetSpecular(0);
			vp->SetUV(u,v);
			vp++;

			*ip++ = ii*3;
			*ip++ = ii*3+1;
			*ip++ = ii*3+2;
		}
	}
	else
	{
		for (int ii = 0; ii < 10000; ii++)
		{
			int	x = ii % 20;
			int	y = (ii / 20) % 15;

			vp->SetSC(x*32,y*32);
			vp->SetColour(0x80FFFFFF);
			vp->SetSpecular(0);
			vp->SetUV(u,v);
			vp++;

			vp->SetSC(x*32+32,y*32);
			vp->SetColour(0x80FFFFFF);
			vp->SetSpecular(0);
			vp->SetUV(u,v);
			vp++;

			vp->SetSC(x*32,y*32+32);
			vp->SetColour(0x80FFFFFF);
			vp->SetSpecular(0);
			vp->SetUV(u,v);
			vp++;

			*ip++ = ii*3;
			*ip++ = ii*3+1;
			*ip++ = ii*3+2;
		}
	}

	StartStopwatch();

	BEGIN_SCENE;

	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE, FALSE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	if (blend)
	{
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	}
	else
	{
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	}

	if (large)
	{

		HRESULT res = DRAW_INDEXED_PRIMITIVE(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (D3DTLVERTEX*)vert, 300, ind, 300, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
		//HRESULT res = DRAW_INDEXED_PRIMITIVE(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, vert->GetTLVert(), 300, ind, 300, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
		ASSERT(!FAILED(res));
	}
	else
	{
		HRESULT res = DRAW_INDEXED_PRIMITIVE(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (D3DTLVERTEX*)vert, 30000, ind, 30000, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
		//HRESULT res = DRAW_INDEXED_PRIMITIVE(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, vert->GetTLVert(), 30000, ind, 30000, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
		ASSERT(!FAILED(res));
	}

	END_SCENE;

	the_display.screen_lock();
	the_display.screen_unlock();

	float	time = StopStopwatch();

	delete[] vert;
	delete[] ind;

	return time;
}

// guess detail levels
//
// this is only done if AENG_estimate_detail_levels is true

void AENG_guess_detail_levels()
{
	if (!AENG_estimate_detail_levels)	return;	// don't estimate them

	ENV_set_value_number("estimate_detail_levels", 0, "Render");
	AENG_estimate_detail_levels = 0;

	int		generation;

	// 0 = software or 1-2 for hardware
	D3DDeviceInfo*	dev = the_display.GetDeviceInfo();

	if (!dev->IsHardware())
	{
		generation = 0;
	}
	else
	{
		// software (P450)	3500/s
		// Permedia2		60000/s

		float	tso = 10000 / AENG_draw_some_polys(false, false);

		TRACE("card draws 512-pixel opaque polys at %f per second\n", tso);

		if (tso < 10000)
		{
			generation = 0;
		}
		else if (tso < 40000)
		{
			generation = 1;
		}
		else
		{
			generation = 2;
			if (dev->ModulateAlphaSupported() && dev->DestInvSourceColourSupported())
			{
				generation = 3;
			}
		}
	}

	TRACE("Card generation = %d\n", generation);

	// GENVAR(n) = in all generations >= n
#define	GENVAR(G)	((generation >= G) ? 1 : 0)

	int		stars				= GENVAR(0);
	int		shadows				= GENVAR(2);
	int		moon_reflection		= GENVAR(2);
	int		people_reflection	= GENVAR(3);
	int		puddles				= GENVAR(2);
	int		dirt				= GENVAR(1);
	int		mist				= GENVAR(2);
	int		rain				= GENVAR(0);
	int		skyline				= GENVAR(2);
	int		filter				= GENVAR(1);
	int		perspective			= GENVAR(1);
	int	    crinkles			= GENVAR(3);

	AENG_set_detail_levels(stars, shadows, moon_reflection, people_reflection, puddles, dirt, mist, rain, skyline, filter, perspective, crinkles);
}

#endif





#ifdef TARGET_DC
// Fewer things to set.
void AENG_get_detail_levels(//int* stars,
							int* shadows,
							int* puddles,
							int* dirt,
							int* mist,
							int* rain,
							int* skyline,
							int* crinkles)
#else
void AENG_get_detail_levels(int* stars,
							int* shadows,
							int* moon_reflection,
							int* people_reflection,
							int* puddles,
							int* dirt,
							int* mist,
							int* rain,
							int* skyline,
							int* filter,
							int* perspective,
							int* crinkles)
#endif
{
	//*stars = AENG_detail_stars;
	*shadows = AENG_detail_shadows;
	//*moon_reflection = AENG_detail_moon_reflection;
	//*people_reflection = AENG_detail_people_reflection;
	*puddles = AENG_detail_puddles;
	*dirt = AENG_detail_dirt;
	*mist = AENG_detail_mist;
	*rain = AENG_detail_rain;
	*skyline = AENG_detail_skyline;
	//*filter = AENG_detail_filter;
	//*perspective = AENG_detail_perspective;
	*crinkles = AENG_detail_crinkles;
}

//#pragma optimize( "y", on )
//#pragma optimize( "a", on )


#define	MAX_WIDTH_DRAWN	100


#define	MAX_FLOOR_TILES_FOR_STRIPS	(16) // I doubt we will get more than 32 very often (map is 128 wide)
#define	MAX_VERTS_FOR_STRIPS 	  (MAX_FLOOR_TILES_FOR_STRIPS*4) // 4 verts per map square
#define	MAX_INDICES_FOR_STRIPS 	  (MAX_FLOOR_TILES_FOR_STRIPS*5) // 5 indicies per map square



#ifdef	STRIP_STATS
ULONG	strip_stats[MAX_FLOOR_TILES_FOR_STRIPS+10];
#endif






#ifdef	MIKES_UNUSED_AUTOMATIC_FLOOR_TEXTURE_GROUPER

//
// draws the floor quicker by grouping adjacent squares with the same texture into one index prim
//
UWORD	in_group[64*10];

UWORD	groups[256][9];
UWORD	group_count[256];
SLONG	group_stats[256][9]; //how often each member of the group is used
SLONG	group_break[256][64*10]; // for each group how often the strip is broken by each texture
SLONG	group_hits[256][64*10]; // for each group how often the strip is broken by each texture

UWORD	page_next[64*10][64*10];
ULONG	group_upto=1;


#define	MAX_PREV	8
float	how_good(void)
{
	SLONG	x,z;
	SLONG	page,my_group=0,prev_group[10];
	SLONG	bucket_group[10],bucket_length[10],bucket=0;
	SLONG	length=1;
	SLONG	total_length=0,strip_count=0;
	float	average_len,prev_average_len=0.0f;
	SLONG	count;
	SLONG	quit=0;
	SLONG	c0;
	UWORD	strips[128];
	SLONG	match;

	memset(strips,0,128*2);

	PAP_Hi *ph;

	group_upto=1;

	memset(prev_group,0,4*10);

	memset(bucket_group,0,4*10);
	memset(bucket_length,0,4*10);
//	prev_group[0]=0;

	total_length=0;
	strip_count=0;

	for(z=0;z<128;z++)
	{
		for(x=0;x<128;x++)
		{

			ph = &PAP_2HI(x,z);
			ASSERT(bucket_group[0]<128*128);
			ASSERT(bucket_length[0]<128*128);
//			if(x==22 && z==16)
//				ASSERT(0);

			my_group=0;

			page=ph->Texture&0x3ff;

			if(in_group[page])
			{
				my_group=in_group[page];
			}
			else
			{
				ASSERT(0);

			}

			match=0;
			for(c0=0;c0<MAX_PREV;c0++)
			{
				if(my_group==bucket_group[c0])
				{
					match=c0+1;

				}
			}

			if(match)
			{
				if(bucket_length[match-1]>32)
				{
					match=0;
				}
			}

			if(match)//y_group==prev_group)
			{
				//
				// the same group hoorah
				//

				bucket_length[match-1]++;
				ASSERT(bucket_length[bucket]<128);
				//length++;

			}
			else
			{
				//
				// different group
				//

				// store length stat

				bucket++;
				bucket%=MAX_PREV;


				total_length+=bucket_length[bucket];
				ASSERT(bucket_length[bucket]<128);
				strip_count++;
				strips[bucket_length[bucket]]++;

				bucket_length[bucket]=1;
				bucket_group[bucket]=my_group;
			}
			ASSERT(my_group);
			for(c0=MAX_PREV;c0>0;c0--)
			{
				prev_group[c0]=prev_group[c0-1];
			}
			prev_group[0]=my_group;
		}
	}


	DebugText(" FLOOR STRIP \n");
	for(c0=0;c0<128;c0++)
	{
		DebugText(" %d %d \n",c0,strips[c0]);
	}


	ASSERT(strip_count>0);
	average_len=(float)total_length/(float)strip_count;
	DebugText(" FLOOR STRIP average %f\n",average_len);

	return(average_len);


}
UWORD	frequency[64*10];

float	init_groups2(void)
{
	SLONG	x,z;
	SLONG	page,p1,p2;
	SLONG	highest,best;
	SLONG	g_count=0,g_index=0;
	SLONG	c1,c2;
	SLONG	c0;

	PAP_Hi *ph;
/*
	for(c0=0;c0<64*10;c0++)
	{
		in_group[c0]=c0+1;

	}
	how_good();
*/

	memset(page_next,0,64*10*64*10*2);
	memset(frequency,0,64*10*2);
	memset(in_group,0,64*10*2);
	memset(groups,0,256*9*2);

//	for(page=0;page<8*64;page++)
	for(z=0;z<128;z++)
	{
		for(x=0;x<128;x++)
		{
			ph = &PAP_2HI(x,z);
			p1=ph->Texture&0x3ff;
			if(x)
			{
				ph = &PAP_2HI(x-1,z);	   //to the left
				p2=ph->Texture&0x3ff;
				if(p1!=p2)
					page_next[p1][p2]++;
			}
			if(x<127)
			{
				ph = &PAP_2HI(x+1,z);      //to the right
				p2=ph->Texture&0x3ff;
				if(p1!=p2)
					page_next[p1][p2]++;

			}


			frequency[p1]++;
		}

	}
	//
	// for every texture we now know how many times every other texture appears next to it
	//

/*
	highest=0;
	for(c0=0;c0<64*10;c0++)
	{
		if(frequency[c0]>highest)
		{
			highest=frequency[c0];
			best=c0;
		}

	}
*/

	while(1)
	{
		g_count=0;

		highest=0;
		best=-1;
		for(c0=0;c0<64*10;c0++)
		{
			if(!in_group[c0])
			for(c1=0;c1<64*10;c1++)
			{
				if(c0!=c1)
				if(page_next[c0][c1]>highest)
				{
					highest=page_next[c0][c1];
					best=c0;
				}


			}
		}
		if(best==-1)
			break;



		//
		// for the most frequently used texture, group with it the textures that often appear next to it
		//

		g_index++;
		groups[g_index][g_count]=best;
		in_group[best]=g_index;
		for(c0=0;c0<64*10;c0++)
		{
			page_next[c0][best]=0;   //remove me from next to other textures
//			page_next[best][c0]=0;   //remove me from next to other textures
		}
//		frequency[best]=0;

		g_count++;
		{
			SLONG	group_perc[9];
			SLONG	ndhighest=0;
			SLONG	perc;
			SLONG	current_best_perc=0;
			SLONG	c2;

			page=best;

			page=-1;

			while(g_count<9)
			{
				SLONG	n_of;

				//
				// for all members of the group find the highest probabilty neihbour
				//
				ndhighest=0;
				best=-1;
				for(c1=0;c1<g_count;c1++)
				{
					n_of=groups[g_index][c1];
					for(c0=0;c0<64*10;c0++)
					{
						if(frequency[c0])
						{
							perc=0;

							for(c2=0;c2<g_count;c2++)
								perc+=(page_next[groups[g_index][c2]][c0]);// /frequency[c0];

							if(perc>ndhighest)
							{
								SLONG	abort_store=0;
								//
								//check if better paired up elsewhere
								//
								for(c2=0;c2<64*10;c2++)
								{
									if(page_next[c0][c2]>perc)
										abort_store=1;

								}
								if(!abort_store)
								{
									ndhighest=perc;
									best=c0;
								}
							}
						}
					}
				}
				if(best>=0)
				{
					groups[g_index][g_count]=best;
					g_count++;
					in_group[best]=g_index;
//						frequency[best]=0;
					for(c0=0;c0<64*10;c0++)
					{
						page_next[c0][best]=0;   //remove me from next to other textures
//						page_next[best][c0]=0;   //remove me from next to other textures
					}
				}
				else
				{
					//
					// no more for this group so exit the while loop
					//
					break;
				}

			}
			group_count[g_index]=g_count;
		}
	}

	for(c0=0;c0<64*10;c0++)
	{
		if(!in_group[c0] && frequency[c0])
		{
			//
			// not in a group but exists on map
			//
			groups[g_index][g_count]=c0;
			in_group[c0]=g_index;

			g_count++;
			if(g_count>=9)
			{
				g_count=0;
				g_index++;
			}


		}
	}

	//
	// compress groups
	//

	for(c0=1;c0<g_index;c0++)
	{
		if(group_count[c0]<9)
		{
			for(c1=c0+1;c1<g_index;c1++)
			{
				if(group_count[c1])
				if(group_count[c0]+group_count[c1]<=9)
				{
					//
					// group c1 can fit into group c0
					//
					for(c2=0;c2<group_count[c1];c2++)
					{
						SLONG	group,index;
						groups[c0][group_count[c0]+c2]=groups[c1][c2];
						in_group[groups[c1][c2]]=c0;

					}
					group_count[c0]=group_count[c0]+group_count[c1];
					group_count[c1]=0;
					c0--;
					break;
				}

			}

		}
	}

	//
	// move g_index back
	//
	for(;g_index>0;g_index--)
	{
		if(group_count[g_index-1])
			break;
	}


	float	ret;

	ret=how_good();
	DebugText(" FLOOR STRIP pages %d\n",g_index);

	DebugText(" try random \n",g_index);



	return(ret);
}

#endif


#define	IPRIM_COUNT	5

//
// 16k cache // 512x32byte // 1 way associative
//

//
// data bus 64 bit  //100mhz
//

#define	MAX_DRAW_WIDTH	128

struct	FloorStore
{
	ULONG	Colour;
//	ULONG	Specular;        // is this needed?
	float	Alt;
	UWORD	Flags;		    //not really needed
	UWORD	Texture;	   	//not really needed
//	SLONG	x,z;
};

//40*12 =480 bytes per row, need 2 rows

inline	void cache_a_row(SLONG x,SLONG z,struct FloorStore *p2,SLONG endx)
{
	SLONG	px,pz,dx,dz;
	SLONG	square;
	SLONG	mapz;
	NIGHT_Square *nq;
	PAP_Hi *ph;
	ULONG	spec;
	SLONG   y;

	//
	// pre fetch the data, to avoid reading it 4 times
	//

//	TRACE(" cache row %d -> %d \n",x,endx);



	for (ph = &PAP_2HI(x,z); x <= endx; x++, ph += PAP_SIZE_HI)
	{
		float	dist;

//		p2->Alt=ph->Alt; //15 ticks later the cache will have loaded

		y = ph->Alt << ALT_SHIFT;

		p2->Alt=float(y);

		px = x >> 2;
		pz = z >> 2;

		dx = x & 0x3;
		dz = z & 0x3;


		square = NIGHT_cache[px][pz];


		ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
		ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);

		nq = &NIGHT_square[square];

		/*

		{
			SLONG	cdx,cdz;
			cdx=abs(AENG_cam_x-(x<<8));
			cdz=abs(AENG_cam_z-(z<<8));
			mapz = QDIST2(cdx,cdz);
			dist=((float)mapz)/(float)(AENG_DRAW_DIST<<8);
			if(dist>1.0f)
				dist=1.0f;
		}

		NIGHT_get_d3d_colour_and_fade(
			nq->colour[dx + dz * PAP_BLOCKS],
		   &p2->Colour,
		   &spec,dist);

		*/

		{
			NIGHT_Colour *col = &nq->colour[dx + dz * PAP_BLOCKS];

			SLONG r = col->red   << 2;
			SLONG g = col->green << 2;
			SLONG b = col->blue  << 2;

			if (r > 255) {r = 255;}
			if (g > 255) {g = 255;}
			if (b > 255) {b = 255;}

			p2->Colour = (r << 16) | (g << 8) | b;
		}

		p2->Flags=ph->Flags;
		p2->Texture=ph->Texture;

		/*

		dx = abs(quick_floor_cam_x - (x << 8));
		dy = abs(quick_floor_cam_y - y       );
		dz = abs(quick_floor_cam_z - (z << 8));

		if (dx + dy + dz <= 256)
		{
			//
			// Too close to camera- set the alpha of the colour.
			//

			p2->Colour |= 0x01000000;
		}

		*/

//		p2->x=x;
//		p2->z=z;

			//POLY_fadeout_point(pp);

//		ph++;


		p2++;
	}

}


float	kerb_scaleu;
float	kerb_scalev;
float	kerb_du;
float	kerb_dv;

inline	SLONG	add_kerb(float alt1,float alt2,SLONG x,SLONG z,SLONG dx,SLONG dz,D3DLVERTEX	*pv, UWORD *p_indicies,SLONG count,ULONG c1,ULONG c2,SLONG flip)
{
//	pv=&p_verts[current_set][vert_count[current_set]];
//	p_indicies=&indicies[current_set][index_count[current_set]];



	//   0   1	     0    1	       1
	//
	//	 3   2		 3	       3   2

	pv->x = x   * 256.0F;
	pv->z = z   * 256.0F;
	pv->y = alt1-KERB_HEIGHT;

	pv->tu=0.0f;
	pv->tv=1.0f;

#ifdef	TEX_EMBED
	pv->tu=pv->tu*kerb_scaleu+kerb_du;
	pv->tv=pv->tv*kerb_scalev+kerb_dv;

#endif



	// set verts colour
	pv->color=c1;//0xff808080;//202020;
	pv->specular=0xff000000;
	SET_MM_INDEX(*pv,0);
	pv++;


	pv->x = (x+dx)       * 256.0F;
	pv->z = (z+dz)       * 256.0F;
	pv->y = alt2-KERB_HEIGHT;

	pv->tu=1.0f;
	pv->tv=1.0f;

#ifdef	TEX_EMBED
	pv->tu=pv->tu*kerb_scaleu+kerb_du;
	pv->tv=pv->tv*kerb_scalev+kerb_dv;

#endif

	// set verts colour
	pv->color=c2;//0xff808080;//202020;
	pv->specular=0xff000000;
	SET_MM_INDEX(*pv,0);
	pv++;

	pv->x = (x+dx)       * 256.0F;
	pv->z = (z+dz)       * 256.0F;
	pv->y = alt2;

	pv->tu=1.0f;
	pv->tv=0.0f;

#ifdef	TEX_EMBED
	pv->tu=pv->tu*kerb_scaleu+kerb_du;
	pv->tv=pv->tv*kerb_scalev+kerb_dv;

#endif

	// set verts colour
	pv->color=c2;//0xff808080;//202020;
	pv->specular=0xff000000;
	SET_MM_INDEX(*pv,0);
	pv++;

	pv->x = (x)       * 256.0F;
	pv->z = (z)       * 256.0F;
	pv->y = alt1;

	pv->tu=0.0f;
	pv->tv=0.0f;

#ifdef	TEX_EMBED

	pv->tu=pv->tu*kerb_scaleu+kerb_du;
	pv->tv=pv->tv*kerb_scalev+kerb_dv;

#endif

	// set verts colour
	pv->color=c1;//0xff808080;//202020;
	pv->specular=0xff000000;
	SET_MM_INDEX(*pv,0);
	pv++;

	pv-=4;


	//
	// Shall we z-reject
	//

	{
		SLONG i;

		float dx;
		float dy;
		float dz;

		float dprod;

		dx = pv[0].x - AENG_cam_x;
		dy = pv[0].y - AENG_cam_y;
		dz = pv[0].z - AENG_cam_z;

		float dist;

		float adx;
		float ady;
		float adz;

		adx = fabsf(dx);
		ady = fabsf(dy);
		adz = fabsf(dz);

		dist = adx + ady + adz;

		if (dist > 768.0F)
		{
			//
			// No need to zclip!
			//
		}
		else
		{
			for (i = 0; i < 4; i++)
			{
				dx = pv[i].x - AENG_cam_x;
				dy = pv[i].y - AENG_cam_y;
				dz = pv[i].z - AENG_cam_z;

				dprod = dx*AENG_cam_matrix[6] + dy*AENG_cam_matrix[7] + dz*AENG_cam_matrix[8];

				if (dprod < 8.0F)
				{
					//
					// Too close the camera!
					//

					return FALSE;
				}
			}
		}
	}

	//
	// make the indicies as we go along
	//

	if (flip)
	{
		*p_indicies++=count+0;
		*p_indicies++=count+3;
		*p_indicies++=count+1;

		*p_indicies++=count+2;
		*p_indicies++=0xffff;
	}
	else
	{
//		SLONG	count=vert_count[current_set];

		*p_indicies++=count;
		*p_indicies++=count+1;
		*p_indicies++=count+3;

		*p_indicies++=count+2;
		*p_indicies++=0xffff;
	}

	return TRUE;
}

inline void	draw_i_prim ( LPDIRECT3DTEXTURE2 page, D3DLVERTEX *verts, UWORD *indicies, SLONG *vert_count, SLONG *index_count, D3DMULTIMATRIX *mm_draw_floor )
{
	HRESULT res;
#ifdef	STRIP_STATS
	strip_stats[(*vert_count>>2)+1]++;
	strip_stats[0]++;
	strip_stats[1]+=(*vert_count>>2);
#endif

	mm_draw_floor->lpvVertices	=verts;

	indicies[*index_count]=0x1234;

	REALLY_SET_TEXTURE(page);

	//TRACE ( "S1" );
	res=DrawIndPrimMM (the_display.lp_D3D_Device,D3DFVF_LVERTEX ,mm_draw_floor,*vert_count,indicies,*index_count);
	//TRACE ( "F1" );



	ASSERT(res == DD_OK);	 //761 		//0x887602f9


	*index_count = 0;
	*vert_count = 0;

}


#define	HALF_COL(col) (col)=((col)>>2)&0xff3f3f3f

#define	KERB_TILES	16
#define	KERB_VERTS	 (4*KERB_TILES)
#define	KERB_INDICIES (5*KERB_TILES)


// Well, it's now become the final code...
#define TOMS_TEST_FIXUP_CODE yes



#ifdef DEBUG
int m_iDrawThingCount = 0;
#endif



// Look Mike, when I say "don't put stuff on the stack", I mean
// DONT PUT STUFF ON THE STACK. And it's "indices" - only two "i"s.
UBYTE	m_vert_mem_block32[sizeof(D3DLVERTEX)*KERB_VERTS+sizeof(D3DLVERTEX)*MAX_VERTS_FOR_STRIPS*IPRIM_COUNT+32];		  // used to 32 byte align the vertex memory
UWORD	m_indicies[IPRIM_COUNT][MAX_INDICES_FOR_STRIPS+1];    //data for verts, on stack or not?


struct	GroupInfo
{
	LPDIRECT3DTEXTURE2 page; //ptr to actual page to use for drawing
#ifndef TOMS_TEST_FIXUP_CODE
	float	uscale;
	float	vscale;
	float	du,dv;
#endif
#ifdef DEBUG
	int		iDebugCount;
#endif
};


#define	MAX_STEAM	20
inline	void	general_steam(SLONG x,SLONG z,UWORD texture,SLONG mode)
{
	static	SLONG	stx[MAX_STEAM],sty[MAX_STEAM],stz[MAX_STEAM],lod[MAX_STEAM];
	static	SLONG	count_steam=0;

	if(mode==0)
	{
		count_steam=0;
		return;
	}
	else
	if(mode==2)
	{
		for(SLONG c0=0;c0<count_steam;c0++)
		{
extern	void draw_steam(SLONG x,SLONG y,SLONG z,SLONG lod);
			draw_steam(stx[c0],sty[c0],stz[c0],lod[c0]);//10+(15-dist)*3);

		}
		count_steam=0;
		return;
	}
	if(count_steam>=MAX_STEAM)
		return;


#ifdef TARGET_DC
	// Key off the "mist" detail setting instead
	if (AENG_detail_mist)
#else
	if (AENG_detail_shadows)
#endif
//		if(page==4*64+53) function only called if page is correct
		{
			SLONG	dx,dz,dist,sx,sy,sz;

			dx=abs( (((SLONG)AENG_cam_x)>>8)-(x) );
			dz=abs( (((SLONG)AENG_cam_z)>>8)-(z) );

			dist=QDIST2(dx,dz);

			if(dist<15)
			{
				SLONG	sx,sy,sz;
				switch((texture >> 0xa) & 0x3)
				{
					case	0:
						sx=190;
						sz=128;
						break;
					case	1:
						sx=128;
						sz=66;
						break;
					case	2:
						sx=66;
						sz=128;
						break;
					case	3:
						sx=128;
						sz=190;
						break;
					default:
						ASSERT(0);
						break;

				}
				sx+=x<<8;
				sz+=z<<8;
				sy=PAP_calc_height_at(sx,sz);

				stx[count_steam]=sx;
				sty[count_steam]=sy;
				stz[count_steam]=sz;
				lod[count_steam]=10+(15-dist)*3;
				count_steam++;
//				draw_steam(sx,sy,sz,10+(15-dist)*3);

			}
		}

}

void	draw_quick_floor(SLONG warehouse)
{
	PAP_Hi *ph;
//	ULONG	colour,specular;

	SLONG	c0;
	SLONG	x,z;
	float	dy,y;
	UWORD	index;

	SLONG	page,page2,prev_page=-10000,apage=0;

	SLONG	current_set=0;

	struct	GroupInfo group[IPRIM_COUNT];

	PolyPage *pp;
	LPDIRECT3DTEXTURE2 tex_handle;


	UWORD	kerb_indicies[KERB_INDICIES];


	UWORD	*p_indicies;

	D3DMATRIX *m_view;
	D3DLVERTEX	*p_verts[IPRIM_COUNT],*pv,*kerb_verts;

	UBYTE	some_data[sizeof(D3DMATRIX)+32];                                  // used to 32 byte align the funny fanny thing
	UBYTE	*ptr32;

	SLONG	index_count[IPRIM_COUNT],vert_count[IPRIM_COUNT],age[IPRIM_COUNT],kerb_counti=0,kerb_countv=0;


	SLONG	bin_set;

	D3DMULTIMATRIX	mm_draw_floor;

	static	SLONG	init_stats=1;

	static	SLONG	biggest=0;

	struct	FloorStore	row[MAX_DRAW_WIDTH*2+2];
	struct	FloorStore	*p1,*p2;
	SLONG	startx,endx,offsetx;
	SLONG	no_floor=0;
	SLONG	is_shadow;


#ifdef	TEX_EMBED
	pp=&POLY_Page[0];

	kerb_du		=pp->m_UOffset;
	kerb_dv		=pp->m_VOffset;
	kerb_scaleu	=pp->m_UScale;
	kerb_scalev	=pp->m_VScale;
#endif

	if (GAME_FLAGS & GF_NO_FLOOR)
		no_floor=1;

	general_steam(0,0,0,0); //init it


#ifdef	STRIP_STATS
	memset(strip_stats,0,4*MAX_FLOOR_TILES_FOR_STRIPS+4*10);
#endif
	memset(group,0,sizeof(struct GroupInfo)*IPRIM_COUNT);

#ifdef DEBUG
	for ( int i = 0; i < IPRIM_COUNT; i++ )
	{
		group[i].iDebugCount = ( i << 5 ) + 2;
	}
#endif

	if(init_stats)
	{
		init_stats=0;
	}


	ptr32=(UBYTE *)(((ULONG)(some_data+32))&0xffffffe0);
	m_view=(D3DMATRIX *)ptr32;

	mm_draw_floor.lpd3dMatrices	=m_view;
	mm_draw_floor.lpvLightDirs	=NULL;
	mm_draw_floor.lpLightTable	=NULL;

	ptr32=(UBYTE *)(((ULONG)(m_vert_mem_block32+32))&0xffffffe0);

	kerb_verts=(D3DLVERTEX *)ptr32;
	ptr32+=sizeof(D3DLVERTEX)*KERB_VERTS;

	for(c0=0;c0<IPRIM_COUNT;c0++)
	{
		p_verts[c0]=(D3DLVERTEX *)ptr32;
		ptr32+=sizeof(D3DLVERTEX)*MAX_VERTS_FOR_STRIPS;
		index_count[c0]=0;
		vert_count[c0]=0;
		age[c0]=0x7fff;
	}

	BEGIN_SCENE;

	//
	// setup render states for the floor, who knows how many we can assume are ok?
	//


/*
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,TRUE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZBIAS,0);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE,FALSE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
*/

	RenderState default_renderstate;

	default_renderstate.SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	default_renderstate.SetChanged();

	//REALLY_SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE,D3DCULL_CCW);


//	GenerateMMMatrixFromStandardD3DOnes (m_view,g_matProjection,g_matWorld,g_viewData);
	GenerateMMMatrixFromStandardD3DOnes (m_view,&g_matProjection,NULL,&g_viewData);

//	colour   = 0x00888888;
//	specular = 0xff000000;

	z=NGAMUT_zmin;


	startx	=	NGAMUT_point_gamut[z].xmin;
	endx	=	NGAMUT_point_gamut[z].xmax;

	extern SLONG NGAMUT_xmin;

	offsetx=startx-(NGAMUT_xmin);

	ASSERT(offsetx<MAX_DRAW_WIDTH);
	ASSERT(offsetx+endx-startx<MAX_DRAW_WIDTH);
	if(z&1)
	{
		p1=&row[0+offsetx];
	}
	else
	{
		p1=&row[MAX_DRAW_WIDTH+offsetx];
	}
	cache_a_row(startx,z,p1,endx);


	if(!INDOORS_INDEX)
	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{

		startx	=	NGAMUT_point_gamut[z+1].xmin;
		endx	=	NGAMUT_point_gamut[z+1].xmax;
		offsetx=startx-(NGAMUT_xmin);

		ASSERT(offsetx<MAX_DRAW_WIDTH);
		ASSERT(offsetx+endx-startx<MAX_DRAW_WIDTH);

		if(z&1)
		{
			p2=&row[MAX_DRAW_WIDTH+offsetx];
			memset(&row[MAX_DRAW_WIDTH],0,MAX_DRAW_WIDTH*sizeof(struct FloorStore));
		}
		else
		{
			p2=&row[0+offsetx];
			memset(&row[0],0,MAX_DRAW_WIDTH*sizeof(struct FloorStore));
		}

		cache_a_row(startx,z+1,p2,endx);


		offsetx=NGAMUT_gamut[z].xmin-(NGAMUT_xmin);

		ASSERT(offsetx<MAX_DRAW_WIDTH);
		ASSERT(offsetx+endx-startx<MAX_DRAW_WIDTH);

		if(z&1)
		{
			p1=&row[0+offsetx];
			p2=&row[MAX_DRAW_WIDTH+offsetx];
		}
		else
		{
			p1=&row[MAX_DRAW_WIDTH+offsetx];
			p2=&row[0+offsetx];
		}

		//
		//
		//

//		if(NGAMUT_gamut[z].xmax-NGAMUT_gamut[z].xmin>biggest)
//			biggest=NGAMUT_gamut[z].xmax-NGAMUT_gamut[z].xmin;

		for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++,p1++,p2++)
		{
//			ASSERT(p1->x==x);
//			ASSERT(p1->z==z);
//			ASSERT(p2->x==x);
//			ASSERT(p2->z==z+1);
			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

			ASSERT(p1>=&row[0]);
			ASSERT(p2>=&row[0]);
			ASSERT(p1<&row[MAX_DRAW_WIDTH*2+2]);
			ASSERT(p2<&row[MAX_DRAW_WIDTH*2+2]);

			ph = &PAP_2HI(x,z);


//			ASSERT(ph->Texture==p1->Texture);

			if(warehouse)
			{
				if (!(p1->Flags & PAP_FLAG_HIDDEN))
				{
					continue;
				}

			}
			else
			{
				SLONG	s1,s2;

				s1=p1->Flags & PAP_FLAG_SINK_SQUARE;
				s2=(p1+1)->Flags & PAP_FLAG_SINK_SQUARE;

				if(s1!=s2)
				{
					//
					// change of sink status along x
					//

					if(kerb_countv>=KERB_VERTS-4)
					{
						draw_i_prim(POLY_Page[0].RS.GetTexture(),kerb_verts,kerb_indicies,&kerb_countv,&kerb_counti,&mm_draw_floor);
					}

					if (add_kerb((p1+1)->Alt,(p2+1)->Alt,x+1,z,0,1,&kerb_verts[kerb_countv],&kerb_indicies[kerb_counti],kerb_countv,(p1+1)->Colour,(p2+1)->Colour,s1))
					{
						kerb_countv+=4;
						kerb_counti+=5;
					}

				}

				s1=p1->Flags & PAP_FLAG_SINK_SQUARE;
				s2=(p2)->Flags & PAP_FLAG_SINK_SQUARE;

				if(s1!=s2)
				{
					//
					// change of sink status along z
					//

					if(kerb_countv>=KERB_VERTS-4)
					{
						draw_i_prim(POLY_Page[0].RS.GetTexture(),kerb_verts,kerb_indicies,&kerb_countv,&kerb_counti,&mm_draw_floor);
					}

					if (add_kerb((p2)->Alt,(p2+1)->Alt,x,z+1,1,0,&kerb_verts[kerb_countv],&kerb_indicies[kerb_counti],kerb_countv,(p2)->Colour,(p2+1)->Colour,s2))
					{
						kerb_countv+=4;
						kerb_counti+=5;
					}

				}

				//
				// Mark this needs to be in the else! MikeD
				//

				if ((p1->Flags & (PAP_FLAG_HIDDEN|PAP_FLAG_ROOF_EXISTS))== PAP_FLAG_HIDDEN)
				{
					continue;
				}
			}


			if (warehouse==0 && (p1->Flags & (PAP_FLAG_ROOF_EXISTS)) )
			{
				y=MAVHEIGHT(x,z)<<6;
				if(y>AENG_cam_y)
					continue;
			}
			else
			{
				if (no_floor)
				{
					//
					// Don't draw the floor if there isn't any! (like the final level)
					//

					continue;
				}

				//
				// not a roof so might have a kerb
				//


			}

			if(warehouse)
				is_shadow=0;
			else
				is_shadow=p1->Flags & (PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3);


			page=p1->Texture&0x3ff;

			if(page==4*64+53)
				general_steam(x,z,p1->Texture,1); //store it

			pp=&POLY_Page[page];

			tex_handle=pp->RS.GetTexture();

			current_set=-1;

			//
			// age the iprims
			//
			for(c0=0;c0<IPRIM_COUNT;c0++)
				age[c0]++;


			for(c0=0;c0<IPRIM_COUNT;c0++)
			{
				if(group[c0].page==tex_handle)//&&0)
				{
					age[c0]=0;
					current_set=c0;
					break;
				}
			}

			bin_set=-1;
			if(current_set==-1)
			{
				SLONG	oldest=-1;
				//
				// no group currently supports this new page, so find an empty one, or an old one to bin
				//
				for(c0=0;c0<IPRIM_COUNT;c0++)
				{
					if(vert_count[c0]==0)
					{
						bin_set=-1; //no need to bin an empty one
						current_set=c0;

						group[current_set].page=tex_handle;
#ifdef	TEX_EMBED
#ifndef TOMS_TEST_FIXUP_CODE
						group[current_set].du=pp->m_UOffset;
						group[current_set].dv=pp->m_VOffset;
						group[current_set].uscale=pp->m_UScale;
						group[current_set].vscale=pp->m_VScale;
#endif
#endif


						break;
					}

					if(age[c0]>oldest)
					{
						bin_set=c0;
						current_set=c0;
						oldest=age[c0];
					}
				}

			}
			else
			{
				SLONG	cv=4,ci=5;
				if(is_shadow)
				{
					cv=5;	 // shadow squares require more verts and indicies as quads are drawn as two seperate tri's
					ci=8;	 //
				}
				//
				// do we have to bin(draw) a prim because it is too full?
				//
				if(vert_count[current_set]>=MAX_VERTS_FOR_STRIPS-cv || index_count[current_set]>=MAX_INDICES_FOR_STRIPS-ci)
					bin_set=current_set;
			}
			age[current_set]=0;

			ASSERT(current_set>=0&& current_set<IPRIM_COUNT);

			//
			// Draw this prim because its buffer is full, or its buffer is required for new texture page
			//
			if(bin_set>=0)// || vert_count[current_set]>=MAX_VERTS_FOR_STRIPS-4)
			{
				if(vert_count[bin_set])
				{
					draw_i_prim(group[bin_set].page,p_verts[bin_set],&m_indicies[bin_set][0],&vert_count[bin_set],&index_count[bin_set],&mm_draw_floor);
#ifdef DEBUG
					group[bin_set].iDebugCount++;
#endif

					ASSERT(bin_set==current_set);

					group[current_set].page=tex_handle;
#ifdef	TEX_EMBED
#ifndef TOMS_TEST_FIXUP_CODE
					group[current_set].du=pp->m_UOffset;
					group[current_set].dv=pp->m_VOffset;
					group[current_set].uscale=pp->m_UScale;
					group[current_set].vscale=pp->m_VScale;
#endif
#endif

				}
			}
//			group[current_set]=page;

//			current_set=0;

			//
			// build the floor quad
			//
			pv=&p_verts[current_set][vert_count[current_set]];
			p_indicies=&m_indicies[current_set][index_count[current_set]];

			ASSERT(vert_count[current_set]<MAX_VERTS_FOR_STRIPS);
			ASSERT(index_count[current_set]<MAX_INDICES_FOR_STRIPS);

			if((p1->Flags & PAP_FLAG_SINK_SQUARE) && warehouse==0)
			{
				dy = -KERB_HEIGHT;
			}
			else
			{
				dy = 0.0f;
			}


			//   0   1	     0    1	       1
			//
			//	 3   2		 3	       3   2

			pv->x = x       * 256.0F;
			pv->z = z       * 256.0F;
			pv->color=p1->Colour;//0xff808080;//202020;
			pv->specular=0xff000000;
			SET_MM_INDEX(*pv,0);
			pv++;


			pv->x = (x+1)       * 256.0F;
			pv->z = z       * 256.0F;
			pv->color=(p1+1)->Colour;//202020;
			pv->specular=0xff000000;
			SET_MM_INDEX(*pv,0);
			pv++;

			pv->x = (x+1)       * 256.0F;
			pv->z = (z+1)       * 256.0F;
			pv->color=(p2+1)->Colour;//202020;
			pv->specular=0xff000000;
			SET_MM_INDEX(*pv,0);
			pv++;

			pv->x = x       * 256.0F;
			pv->z = (z+1)       * 256.0F;
			pv->color=p2->Colour;//202020;
			pv->specular=0xff000000;
			SET_MM_INDEX(*pv,0);
//			pv++;

			pv-=3;

			TEXTURE_get_minitexturebits_uvs(
					p1->Texture,
				   &page2,
				   &pv[0].tu,
				   &pv[0].tv,
				   &pv[1].tu,
				   &pv[1].tv,
				   &pv[3].tu,
				   &pv[3].tv,
				   &pv[2].tu,
				   &pv[2].tv);

			#ifdef TEX_EMBED

#ifdef TOMS_TEST_FIXUP_CODE
			pv[0].tu=pv[0].tu*pp->m_UScale+pp->m_UOffset;
			pv[1].tu=pv[1].tu*pp->m_UScale+pp->m_UOffset;
			pv[2].tu=pv[2].tu*pp->m_UScale+pp->m_UOffset;
			pv[3].tu=pv[3].tu*pp->m_UScale+pp->m_UOffset;

			pv[0].tv=pv[0].tv*pp->m_VScale+pp->m_VOffset;
			pv[1].tv=pv[1].tv*pp->m_VScale+pp->m_VOffset;
			pv[2].tv=pv[2].tv*pp->m_VScale+pp->m_VOffset;
			pv[3].tv=pv[3].tv*pp->m_VScale+pp->m_VOffset;
#else
			pv[0].tu=pv[0].tu*group[current_set].uscale+group[current_set].du;
			pv[1].tu=pv[1].tu*group[current_set].uscale+group[current_set].du;
			pv[2].tu=pv[2].tu*group[current_set].uscale+group[current_set].du;
			pv[3].tu=pv[3].tu*group[current_set].uscale+group[current_set].du;

			pv[0].tv=pv[0].tv*group[current_set].vscale+group[current_set].dv;
			pv[1].tv=pv[1].tv*group[current_set].vscale+group[current_set].dv;
			pv[2].tv=pv[2].tv*group[current_set].vscale+group[current_set].dv;
			pv[3].tv=pv[3].tv*group[current_set].vscale+group[current_set].dv;
#endif

			#endif


#ifdef DEBUG
#ifdef TARGET_DC
			// Colour the vertices.
#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)
extern DIJOYSTATE the_state;
			if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) && BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) )
			{
				DWORD dwTemp = group[current_set].iDebugCount;
				dwTemp = ( dwTemp ) + ( dwTemp << 8 ) + ( dwTemp << 17 ) + ( dwTemp << 26 );
				dwTemp &= 0x7f7f7f7f;
				pv[0].dcColor = dwTemp;
				pv[1].dcColor = dwTemp;
				pv[2].dcColor = dwTemp;
				pv[3].dcColor = dwTemp;
				pv[0].dcSpecular = dwTemp;
				pv[1].dcSpecular = dwTemp;
				pv[2].dcSpecular = dwTemp;
				pv[3].dcSpecular = dwTemp;
			}
#endif
#endif

			if ((p1->Flags & (PAP_FLAG_ROOF_EXISTS)) && warehouse==0)
			{
				y=MAVHEIGHT(x,z)<<6;

				pv[0].y = y;
				pv[1].y = y;
				pv[2].y = y;
				pv[3].y = y;
			}
			else
			{

				pv[0].y = p1->Alt+dy;// * float(1 << ALT_SHIFT)+dy;
				pv[1].y = (p1+1)->Alt+dy;// * float(1 << ALT_SHIFT)+dy;
				pv[2].y = (p2+1)->Alt+dy;// * float(1 << ALT_SHIFT)+dy;
				pv[3].y = (p2)->Alt+dy;// * float(1 << ALT_SHIFT)+dy;
			}

			{
				SLONG i;

				float dx;
				float dy;
				float dz;

				float dprod;

				dx = (pv[0].x + pv[2].x) * 0.5F - AENG_cam_x;
				dy = (pv[0].y + pv[2].y) * 0.5F - AENG_cam_y;
				dz = (pv[0].z + pv[2].z) * 0.5F - AENG_cam_z;

				float dist;

				float adx;
				float ady;
				float adz;

				adx = fabsf(dx);
				ady = fabsf(dy);
				adz = fabsf(dz);

				dist  = 0;
				dist += adx;
				dist += ady;
				dist += adz;

				if (dist > 512.0F)
				{
					//
					// No need to zclip!
					//
				}
				else
				{
					ULONG zclip = 0;
					float along[4];

					for (i = 0; i < 4; i++)
					{
						dx = pv[i].x - AENG_cam_x;
						dy = pv[i].y - AENG_cam_y;
						dz = pv[i].z - AENG_cam_z;

						dprod = dx*AENG_cam_matrix[6] + dy*AENG_cam_matrix[7] + dz*AENG_cam_matrix[8];

						if (dprod < 8.0F)
						{
							//
							// Too close the camera- zfuck!
							//

							along[i] = 8.0F - dprod;

							zclip |= 1 << i;

						}
					}

					if (zclip)
					{
						if (zclip == 0xf)
						{
							//
							// Reject whole quad!
							//

							goto abandon_quad;
						}

						for (i = 0; i < 4; i++)
						{
							if (zclip & (1 << i))
							{
								pv[i].x += along[i] * AENG_cam_matrix[6];
								pv[i].y += along[i] * AENG_cam_matrix[7];
								pv[i].z += along[i] * AENG_cam_matrix[8];
							}
						}
					}
				}
			}

			pv += 4;

			//
			// shadows
			//
			if(is_shadow)
			{
				//
				// for shadows we need to duplicate the diag verts
				//

				pv-=4;
				pv[4]=pv[3];

				// to be compatible with shadow.h we have to rotate quad by 180 Degrees


				//     3		   2   4
				//
				// 1   0		   1

				switch(is_shadow)//p1->Flags & (PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3))
				{
					case 0:
						ASSERT(0);	// We shouldn't be doing any of this in this case.
						break;

					case 1:
						HALF_COL(pv[0].color);
						HALF_COL(pv[3].color);

						break;

					case 2:
					case 6:
						HALF_COL(pv[4].color);
						HALF_COL(pv[3].color);
						HALF_COL(pv[0].color);
						//HALF_COL(pv[2].color);

						break;

					case 3:
						HALF_COL(pv[4].color);
						HALF_COL(pv[3].color);


						break;

					case 4:
						HALF_COL(pv[2].color);
						HALF_COL(pv[3].color);
						HALF_COL(pv[4].color);

						break;

					case 5:
						HALF_COL(pv[2].color);
						HALF_COL(pv[0].color);
						HALF_COL(pv[4].color);
						HALF_COL(pv[3].color);
						break;

					case 7:
						HALF_COL(pv[2].color);
						HALF_COL(pv[4].color);
						break;

					default:
						ASSERT(0);
						break;
				}
				pv+=5;

				//
				// make the indicies as we go along, for 6 vert shadow split quad
				//


				SLONG	count=vert_count[current_set];

				*p_indicies++=count+3;
				*p_indicies++=count+0;
				*p_indicies++=count+1;
				*p_indicies++=0xffff;

				*p_indicies++=count+2;
				*p_indicies++=count+4;
				*p_indicies++=count+1;
				*p_indicies++=0xffff;

				vert_count[current_set]+=5;
				index_count[current_set]+=8; //8 per quad because its two separte tri's

			}
			else
			{
				//
				// make the indicies as we go along, for 4 vert normal quad
				//

				SLONG	count=vert_count[current_set];

				*p_indicies++=count+0;
				*p_indicies++=count+1;
				*p_indicies++=count+3;
				*p_indicies++=count+2;
				*p_indicies++=0xffff;

				vert_count[current_set]+=4;
				index_count[current_set]+=5; //5 per quad because terminates with -1

			}

		  abandon_quad:;
		}
	}



	//
	// Draw any prims left with data in
	//
	for(c0=0;c0<IPRIM_COUNT;c0++)
	{
		if(vert_count[c0])
		{

			draw_i_prim(group[c0].page,p_verts[c0],&m_indicies[c0][0],&vert_count[c0],&index_count[c0],&mm_draw_floor);
#ifdef DEBUG
			group[c0].iDebugCount++;
#endif

		}
	}

	if(kerb_countv)
	{
		draw_i_prim(POLY_Page[0].RS.GetTexture(),kerb_verts,kerb_indicies,&kerb_countv,&kerb_counti,&mm_draw_floor);
	}
	general_steam(0,0,0,2); //draw it

	// Deal with internals.
	POLY_set_local_rotation_none();
	general_steam(0,0,0,2); //draw it

	END_SCENE;
}




#ifdef TARGET_DC

#ifdef DEBUG
int m_iDCFramerateMin = 15;
int m_iDCFramerateMax = 17;
#else
int m_iDCFramerateMin = 15;
int m_iDCFramerateMax = 18;
#endif

bool m_bTweakFramerates = FALSE;

void	fiddle_draw_distance_DC(void)
{


//#ifdef DEBUG

#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)

extern DIJOYSTATE			the_state;

	if ( m_bTweakFramerates &&
		 ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) ) &&
		 ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) ) )
	{

		static iNotTooFastCounter = 0;

		iNotTooFastCounter++;

		if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_X] ) )
		{
			// Change DrawDistance
			CurDrawDistance += ( the_state.lX - 128 );
			TRACE ( "Draw dist: 0x%x\n", CurDrawDistance );
			SATURATE(CurDrawDistance,(10<<8)+128,(22<<8)+128);
		}
		if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_A] ) )
		{
			// Change min frame rate
			if ( ( iNotTooFastCounter & 0x3 ) == 0 )
			{
				if ( the_state.lX < 64 )
				{
					m_iDCFramerateMin--;
				}
				else if ( the_state.lX > 192 )
				{
					m_iDCFramerateMin++;
				}
			}
			TRACE ( "Min framerate: %i\n", m_iDCFramerateMin );
		}
		if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Y] ) )
		{
			// Change min frame rate
			if ( ( iNotTooFastCounter & 0x3 ) == 0 )
			{
				if ( the_state.lX < 64 )
				{
					m_iDCFramerateMax--;
				}
				else if ( the_state.lX > 192 )
				{
					m_iDCFramerateMax++;
				}
			}
			TRACE ( "Max framerate: %i\n", m_iDCFramerateMax );
		}
		if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_B] ) )
		{
			// Kill the auto throttle system.
			m_iDCFramerateMin = -1;
			m_iDCFramerateMax = 60;
		}
	}
	else
//#endif
	{

	extern	SLONG	tick_tock_unclipped;
		if(tick_tock_unclipped)
		{
			int iFramerate = 1000/tick_tock_unclipped;

			// If in a cutscene, or in "first-person" mode, move the fog plane to the distance,
			// because it looks nicer, and the framerate hit is not nearly as noticeable.
			if ( FirstPersonMode || EWAY_stop_player_moving() )
			{
				CurDrawDistance += 64;
			}
			else if( iFramerate < m_iDCFramerateMin)
			{
				CurDrawDistance -= 64;
				//remove_dead_people=1;
			}
			else if ( iFramerate > m_iDCFramerateMax )
			{
				CurDrawDistance += 64;
			}
			SATURATE(CurDrawDistance,(10<<8)+128,(22<<8)+128);
		}
	}

}
#endif



UBYTE	index_lookup[]={0,1,3,2};

void AENG_draw_city()
{

	//LOG_ENTER ( AENG_Draw_City )


	//TRACE ( "AengIn" );

	ANNOYINGSCRIBBLECHECK;


	DumpTracies();


	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;

	SLONG nx;
	SLONG nz;

	SLONG page;
	SLONG shadow;
	SLONG square;

	float world_x;
	float world_y;
	float world_z;

	POLY_Point *pp;
	POLY_Point *ppl;
#ifndef TARGET_DC
	MapElement *me;
#endif

	PAP_Lo *pl;
	PAP_Hi *ph;

	THING_INDEX t_index;
	Thing      *p_thing;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	NIGHT_Square *nq;

	SLONG red;
	SLONG green;
	SLONG blue;

	SLONG px;
	SLONG pz;

	SLONG sx;
	SLONG sz;

	SLONG px1;
	SLONG pz1;
	SLONG px2;
	SLONG pz2;

	SLONG mx;
	SLONG mz;

	SLONG worked_out_colour;
	ULONG colour;
	ULONG specular;

	OB_Info *oi;

	LIGHT_Colour pcol;
	static	SLONG outside=1;
	static	SLONG sea_offset=0;

	AENG_total_polys_drawn = 0;
void	draw_all_boxes(void);
	draw_all_boxes();


extern	SLONG	tick_tock_unclipped;
	sea_offset+=(tick_tock_unclipped);


#if 0
#ifdef TARGET_DC
	// For some reason the PC version decides not to call this.
	// I think they have some mad scheme of calling it in the previous
	// frame, but it really screws things up. STOP IT!
	POLY_frame_init(TRUE,TRUE);
#endif
#endif





	LOG_ENTER ( AENG_Check_Visible );

	//
	// Work out which things are in view.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (p_thing->Flags & FLAGS_IN_BUILDING)
				{
					//
					// Dont draw things inside buildings when we are outdoors.
					//
				}
				else
				{
					switch(p_thing->Class)
					{
						case CLASS_PERSON:

							//
							// We only have a rejection test for people now.
							//

							if(p_thing->Genus.Person->PlayerID && !p_thing->Genus.Person->Ware)
								p_thing->Flags |= FLAGS_IN_VIEW;
							else
							if (!p_thing->Genus.Person->Ware && FC_can_see_person(AENG_cur_fc_cam, p_thing))
							{
								if (POLY_sphere_visible(
									float(p_thing->WorldPos.X >> 8),
									float(p_thing->WorldPos.Y >> 8) + 0x80,
									float(p_thing->WorldPos.Z >> 8),
									256.0F / (AENG_DRAW_DIST * 256.0F)))
								{
									p_thing->Flags |= FLAGS_IN_VIEW;
								}
							}

							break;

						default:
							p_thing->Flags |= FLAGS_IN_VIEW;
							break;
					}
				}

				t_index = p_thing->Child;
			}

			NIGHT_square[NIGHT_cache[x][z]].flag &= ~NIGHT_SQUARE_FLAG_DELETEME;
		}
	}

	BreakTime("Worked out things in view");

	LOG_EXIT ( AENG_Check_Visible );

	LOG_ENTER ( AENG_Draw_Indoors_Floors );

	ANNOYINGSCRIBBLECHECK;


	//
	// Points out of the ambient light.
	//

	shadow =
		((NIGHT_amb_red   >> 1) << 16) |
		((NIGHT_amb_green >> 1) <<  8) |
		((NIGHT_amb_blue  >> 1) <<  0);

	if(Keys[KB_L]&&ControlFlag)
	{
		outside^=1;
	}

	if(INDOORS_INDEX)
	{
#ifndef TARGET_DC
		POLY_frame_draw(TRUE,TRUE);
		POLY_frame_init(TRUE,TRUE);
#endif
		if(INDOORS_INDEX_NEXT)
			AENG_draw_inside_floor(INDOORS_INDEX_NEXT,INDOORS_ROOM_NEXT,0);

//		POLY_frame_draw(TRUE,TRUE);
		if(INDOORS_INDEX)
			AENG_draw_inside_floor(INDOORS_INDEX,INDOORS_ROOM,INDOORS_INDEX_FADE);
#ifndef TARGET_DC
		POLY_frame_draw(TRUE,TRUE);
		POLY_frame_init(TRUE,TRUE);
#endif
//		return;
	}

	LOG_EXIT ( AENG_Draw_Indoors_Floors );

#ifdef	NEW_FLOOR
	draw_quick_floor(0);
#endif

	//
	// Rotate all the points.   //draw_floor
	//

	LOG_ENTER ( AENG_Rotate_Points );
#ifndef	NEW_FLOOR
	colour   = 0x00888888;
	specular = 0xff000000;

#ifdef TARGET_DC
	// DC internals fixup.
	POLY_flush_local_rot();
#endif

	if(!INDOORS_INDEX||outside)
	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		for (x = NGAMUT_point_gamut[z].xmin; x <= NGAMUT_point_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

extern	UBYTE	player_visited[16][128];
//			player_visited[x>>3][z]|=1<<(x&7);

			ph = &PAP_2HI(x,z);
			//show_gamut_hi(x,z);

			//
			// The upper point.
			//

			world_x = x       * 256.0F;
			world_y = ph->Alt * float(1 << ALT_SHIFT);
			world_z = z       * 256.0F;

			worked_out_colour = FALSE;

			if (!(ph->Flags & PAP_FLAG_NOUPPER))
			{
				pp = &AENG_upper[x & 63][z & 63];

				POLY_transform(world_x, world_y, world_z, pp);

				if (pp->MaybeValid())
				{
					//
					// Work out the colour of this point... what a palaver!
					//


					if(INDOORS_INDEX)
					{
/*
						NIGHT_get_d3d_colour_dim(
							nq->colour[dx + dz * PAP_BLOCKS],
						   &pp->colour,
						   &pp->specular);
*/
#ifdef TARGET_DC
						pp->colour=0xff808080;//202020;
						pp->specular=0xff000000;
#else
						pp->colour=0x80808080;//202020;
						pp->specular=0x80000000;
#endif
					}
					else
					{
						px = x >> 2;
						pz = z >> 2;

						dx = x & 0x3;
						dz = z & 0x3;

						ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
						ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

						square = NIGHT_cache[px][pz];

						ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
						ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);

						nq = &NIGHT_square[square];

						NIGHT_get_d3d_colour(
							nq->colour[dx + dz * PAP_BLOCKS],
						   &pp->colour,
						   &pp->specular);
					}

#ifndef TARGET_DC
					apply_cloud((SLONG)world_x,(SLONG)world_y,(SLONG)world_z,&pp->colour);
#endif


					POLY_fadeout_point(pp);

					colour   = pp->colour;
					specular = pp->specular;

					worked_out_colour = TRUE;
				}
			}

			//
			// The lower point.
			//

			if (ph->Flags & PAP_FLAG_SINK_POINT|| (MAV_SPARE(x,z) & MAV_SPARE_FLAG_WATER))
			{

				if(ph->Flags & PAP_FLAG_SINK_POINT)
				{
					world_y -= KERB_HEIGHT;
				}
				else
				{
					world_y+=(COS(((x<<8)+(sea_offset>>1))&2047)+SIN(((z<<8)+(sea_offset>>1)+700)&2047))>>13;

				}

				ppl = &AENG_lower[x & 63][z & 63];

				POLY_transform(world_x, world_y, world_z, ppl);

				if (ppl->MaybeValid())
				{
					if (worked_out_colour)
					{
						//
						// Use the colour of the upper point...
						//

						ppl->colour   = colour;
						ppl->specular = specular;
#ifdef TARGET_DC
						ppl->colour |= 0xff000000;
						ppl->specular |= 0xff000000;
#endif
					}
					else
					{
						//
						// Work out the colour of this point... what a palaver!
						//


						if(INDOORS_INDEX)
						{
/*
							NIGHT_get_d3d_colour_dim(
								nq->colour[dx + dz * PAP_BLOCKS],
							   &ppl->colour,
							   &ppl->specular);
*/
							ppl->colour=0x202020;
							ppl->specular=0xff000000;
#ifdef TARGET_DC
							ppl->colour |= 0xff000000;
							ppl->specular |= 0xff000000;
#endif

						}
						else
						{
							px = x >> 2;
							pz = z >> 2;

							dx = x & 0x3;
							dz = z & 0x3;

							ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
							ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

							square = NIGHT_cache[px][pz];

							ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
							ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);

							nq = &NIGHT_square[square];

							NIGHT_get_d3d_colour(
								nq->colour[dx + dz * PAP_BLOCKS],
							   &ppl->colour,
							   &ppl->specular);
						}

						POLY_fadeout_point(ppl);
					}
#ifndef TARGET_DC
					apply_cloud((SLONG)world_x,(SLONG)world_y,(SLONG)world_z,&ppl->colour);
#endif
				}
			}
		}
	}
#endif
	BreakTime("Rotated points");

	ANNOYINGSCRIBBLECHECK;

	LOG_EXIT ( AENG_Rotate_Points );

	//
	// No reflection and shadow stuff second time round during 3d mode.
	//


#ifndef TARGET_DC
	LOG_ENTER ( AENG_Draw_Stars );

	if (AENG_detail_stars && !(NIGHT_flag & NIGHT_FLAG_DAYTIME))
	{
		//
		// Draw the stars...
		//
		if (the_display.screen_lock())
		{
			BreakTime("Locked for stars");

			SKY_draw_stars(
				AENG_cam_x,
				AENG_cam_y,
				AENG_cam_z,
				AENG_DRAW_DIST * 256.0F);

			BreakTime("Drawn stars");

			the_display.screen_unlock();
		}
	}

	BreakTime("Done stars");

	LOG_EXIT ( AENG_Draw_Stars );
#endif


	ANNOYINGSCRIBBLECHECK;

	//
	// Shadows.
	//

#ifdef TARGET_DC
	// Tweak my number of shadows PLEASE BOB
	#define AENG_NUM_SHADOWS 4
#else
	#define AENG_NUM_SHADOWS 4
#endif

	LOG_ENTER ( AENG_Draw_Shadows )

	struct
	{
		Thing *p_person;
		SLONG  dist;

	}      shadow_person[AENG_NUM_SHADOWS];
	SLONG  shadow_person_upto = 0;


	if (AENG_detail_shadows)
	{
		Thing *darci = FC_cam[AENG_cur_fc_cam].focus;

		//
		// How many people do we generate shadows for?
		//

		SLONG  shadow_person_worst_dist = -INFINITY;
		SLONG  shadow_person_worst_person;

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				t_index = PAP_2LO(x,z).MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->Class == CLASS_PERSON && (p_thing->Flags & FLAGS_IN_VIEW))
					{
						//
						// Distance from darci.
						//

						dx = p_thing->WorldPos.X - darci->WorldPos.X;
						dz = p_thing->WorldPos.Z - darci->WorldPos.Z;

						dist = abs(dx) + abs(dz);

						if (dist < 0x60000)
						{
							if (shadow_person_upto < AENG_NUM_SHADOWS)
							{
								//
								// Put this person in the shadow array.
								//

								shadow_person[shadow_person_upto].p_person = p_thing;
								shadow_person[shadow_person_upto].dist     = dist;

								//
								// Keep track of the furthest person away.
								//

								if (dist > shadow_person_worst_dist)
								{
									shadow_person_worst_dist   = dist;
									shadow_person_worst_person = shadow_person_upto;
								}

								shadow_person_upto += 1;
							}
							else
							{
								if (dist < shadow_person_worst_dist)
								{
									//
									// Replace the worst person.
									//

									ASSERT(WITHIN(shadow_person_worst_person, 0, AENG_NUM_SHADOWS - 1));

									shadow_person[shadow_person_worst_person].p_person = p_thing;
									shadow_person[shadow_person_worst_person].dist     = dist;

									//
									// Find the worst person
									//

									shadow_person_worst_dist = -INFINITY;

									for (i = 0; i < AENG_NUM_SHADOWS; i++)
									{
										if (shadow_person[i].dist > shadow_person_worst_dist)
										{
											shadow_person_worst_dist   = shadow_person[i].dist;
											shadow_person_worst_person = i;
										}
									}
								}
							}
						}
					}

					t_index = p_thing->Child;
				}
			}
		}

		//
		// Draw the people's shadow maps.
		//

		SLONG offset_x;
		SLONG offset_y;

		POLY_flush_local_rot();

		for (i = 0; i < shadow_person_upto; i++)
		{
			darci = shadow_person[i].p_person;

			memset(AENG_aa_buffer, 0, sizeof(AENG_aa_buffer));

			SMAP_person(
				darci,
				(UBYTE *) AENG_aa_buffer,
				AENG_AA_BUF_SIZE,
				AENG_AA_BUF_SIZE,
				147,
			   -148,
			   -147);

			//
			// Where do we put it in the shadow texture page? Hard code everything!
			//

			ASSERT(AENG_AA_BUF_SIZE    == 32);
			ASSERT(TEXTURE_SHADOW_SIZE == 64);

			offset_x = (i & 1) << 5;
			offset_y = (i & 2) << 4;

			//
			// Plonk it into the shadow texture page.
			//

			if (TEXTURE_shadow_lock())
			{
				SLONG  x;
				SLONG  y;
				UWORD *line;
				UBYTE *buf = (UBYTE *) AENG_aa_buffer;
				UWORD*	mapping = GetShadowPixelMapping();

				for (y = 0; y < AENG_AA_BUF_SIZE; y++)
				{
					line = &TEXTURE_shadow_bitmap[((y + offset_y) * TEXTURE_shadow_pitch >> 1) + offset_x];

					for (x = AENG_AA_BUF_SIZE - 1; x >= 0; x--)
					{
						*line++ = mapping[*buf++];
					}
				}

				TEXTURE_shadow_unlock();
			}

			//
			// How we map floating points coordinates from 0 to 1 onto
			// where we plonked the shadow map in the texture page.
			//

			AENG_project_offset_u = float(offset_x) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_offset_v = float(offset_y) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_mul_u    = float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_mul_v    = float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE);

			//
			// The position from which the shadow fades out.
			//

			AENG_project_fadeout_x = float(darci->WorldPos.X >> 8);
			AENG_project_fadeout_z = float(darci->WorldPos.Z >> 8);

			{
				//
				// Map this poly onto the mapsquares surrounding darci.
				//

				SLONG i;

				SLONG mx;
				SLONG mz;
				SLONG dx;
				SLONG dz;

				SLONG mx1;
				SLONG mz1;
				SLONG mx2;
				SLONG mz2;
				SLONG exit = FALSE;

				SLONG mx_lo;
				SLONG mz_lo;

#ifndef TARGET_DC
				MapElement *me[4];
#endif
				PAP_Hi     *ph[4];

				SVector_F  poly[4];
				SMAP_Link *projected;

				SLONG v_list;
				SLONG i_vect;

				DFacet *df;

				SLONG w_list;
				SLONG w_face;

				PrimFace4 *p_f4;
				PrimPoint *pp;

				SLONG wall;
				SLONG storey;
				SLONG building;
				SLONG thing;
				SLONG face_height;
				UBYTE face_order[4] = {0,1,3,2};

				Thing *p_fthing;

				//
				// Colvects we have already done.
				//

				#define AENG_MAX_DONE 8

				SLONG done[AENG_MAX_DONE];
				SLONG done_upto = 0;

				for (dx = -1; dx <= 1; dx++)
				for (dz = -1; dz <= 1; dz++)
				{
					mx = (darci->WorldPos.X >> 16) + dx;
					mz = (darci->WorldPos.Z >> 16) + dz;

					if (WITHIN(mx, 0, MAP_WIDTH  - 2) &&
						WITHIN(mz, 0, MAP_HEIGHT - 2))
					{
#ifndef TARGET_DC
						me[0] = &MAP[MAP_INDEX(mx + 0, mz + 0)];
						me[1] = &MAP[MAP_INDEX(mx + 1, mz + 0)];
						me[2] = &MAP[MAP_INDEX(mx + 1, mz + 1)];
						me[3] = &MAP[MAP_INDEX(mx + 0, mz + 1)];
#endif

						ph[0] = &PAP_2HI(mx + 0, mz + 0);
						ph[1] = &PAP_2HI(mx + 1, mz + 0);
						ph[2] = &PAP_2HI(mx + 1, mz + 1);
						ph[3] = &PAP_2HI(mx + 0, mz + 1);

						if ( (!(PAP_2HI(mx,mz).Flags & (PAP_FLAG_HIDDEN|PAP_FLAG_WATER))) || (PAP_2HI(mx,mz).Flags & (PAP_FLAG_ROOF_EXISTS)))
						{
							poly[3].X = float(mx << 8);
							poly[3].Z = float(mz << 8);

							poly[0].X = poly[3].X + 256.0F;
							poly[0].Z = poly[3].Z;

							poly[1].X = poly[3].X + 256.0F;
							poly[1].Z = poly[3].Z + 256.0F;

							poly[2].X = poly[3].X;
							poly[2].Z = poly[3].Z + 256.0F;


							if (PAP_2HI(mx,mz).Flags & (PAP_FLAG_ROOF_EXISTS))
							{
								poly[0].Y =poly[1].Y =poly[2].Y =poly[3].Y = MAVHEIGHT(mx,mz)<<6;
							}
							else
							{
								poly[3].Y = float(ph[0]->Alt << ALT_SHIFT);
								poly[2].Y = float(ph[3]->Alt << ALT_SHIFT);
								poly[0].Y = float(ph[1]->Alt << ALT_SHIFT);
								poly[1].Y = float(ph[2]->Alt << ALT_SHIFT);
							}

							if (PAP_2HI(mx,mz).Flags & PAP_FLAG_SINK_SQUARE)
							{
								poly[0].Y -= KERB_HEIGHT;
								poly[1].Y -= KERB_HEIGHT;
								poly[2].Y -= KERB_HEIGHT;
								poly[3].Y -= KERB_HEIGHT;
							}

							if (ph[0]->Alt == ph[1]->Alt &&
								ph[0]->Alt == ph[2]->Alt &&
								ph[0]->Alt == ph[3]->Alt)
							{
								//
								// This quad is coplanar.
								//

								projected = SMAP_project_onto_poly(poly, 4);

								if (projected)
								{
									AENG_add_projected_fadeout_shadow_poly(projected);
								}
							}
							else
							{
								//
								// Do each triangle separatly.
								//

								projected = SMAP_project_onto_poly(poly, 3);

								if (projected)
								{
									AENG_add_projected_fadeout_shadow_poly(projected);
								}

								//
								// The triangles are 0,1,2 and 0,2,3.
								//

								poly[1] = poly[0];

								projected = SMAP_project_onto_poly(poly + 1, 3);

								if (projected)
								{
									AENG_add_projected_fadeout_shadow_poly(projected);
								}
							}
						}
					}
				}

				mx1 = (darci->WorldPos.X >> 8) - 0x100 >> PAP_SHIFT_LO;
				mz1 = (darci->WorldPos.Z >> 8) - 0x100 >> PAP_SHIFT_LO;

				mx2 = (darci->WorldPos.X >> 8) + 0x100 >> PAP_SHIFT_LO;
				mz2 = (darci->WorldPos.Z >> 8) + 0x100 >> PAP_SHIFT_LO;

				SATURATE(mx1, 0, PAP_SIZE_LO - 1);
				SATURATE(mz1, 0, PAP_SIZE_LO - 1);
				SATURATE(mx2, 0, PAP_SIZE_LO - 1);
				SATURATE(mz2, 0, PAP_SIZE_LO - 1);

				for (mx_lo = mx1; mx_lo <= mx2; mx_lo++)
				for (mz_lo = mz1; mz_lo <= mz2; mz_lo++)
				{
					SLONG count = 0;

					//
					// Project onto nearby colvects...
					//

					v_list = PAP_2LO(mx_lo,mz_lo).ColVectHead;

					if (v_list)
					{
						exit = FALSE;

						while(!exit)
						{
							i_vect = facet_links[v_list];

							if(i_vect<0)
							{
								i_vect = -i_vect;

								exit = TRUE;
							}

							df = &dfacets[i_vect];

							if (df->FacetType == STOREY_TYPE_NORMAL)
							{
								for (i = 0; i < done_upto; i++)
								{
									if (done[i] == i_vect)
									{
										//
										// Dont do this facet again
										//

										goto ignore_this_facet;
									}
								}

								if (1 /* Fast facet shadows */)
								{
									float facet_height = float((df->BlockHeight << 4) * (df->Height >> 2));

									poly[0].X = float(df->x[1] << 8);
									poly[0].Y = float(df->Y[1]     );
									poly[0].Z = float(df->z[1] << 8);

									poly[1].X = float(df->x[1] << 8);
									poly[1].Y = float(df->Y[1]     ) + facet_height;
									poly[1].Z = float(df->z[1] << 8);

									poly[2].X = float(df->x[0] << 8);
									poly[2].Y = float(df->Y[0]     ) + facet_height;
									poly[2].Z = float(df->z[0] << 8);

									poly[3].X = float(df->x[0] << 8);
									poly[3].Y = float(df->Y[0]     );
									poly[3].Z = float(df->z[0] << 8);

									if (df->FHeight)
									{
										//
										// Foundations go down deep into the ground...
										//

										poly[0].Y -= 512.0F;
										poly[3].Y -= 512.0F;
									}

									projected = SMAP_project_onto_poly(poly, 4);

									if (projected)
									{
										AENG_add_projected_fadeout_shadow_poly(projected);
									}
								}
								else
								{
									//
									// Slow crinkled-shadows!
									//

									FACET_project_crinkled_shadow(i_vect);
								}

								//
								// Remember We've already done this facet.
								//

								if (done_upto < AENG_MAX_DONE)
								{
									done[done_upto++] = i_vect;
								}
							}

						  ignore_this_facet:;

							v_list++;
						}
					}

					//if (darci->OnFace) Always do this.
					{
						//
						// Cast shadow on walkable faces.
						//

						w_face = PAP_2LO(mx_lo,mz_lo).Walkable;

						while(w_face)
						{
							if (w_face > 0)
							{
								p_f4        = &prim_faces4[w_face];
								face_height =  prim_points[p_f4->Points[0]].Y;

								if (face_height > ((darci->WorldPos.Y + 0x11000) >> 8))
								{
									//
									// This face is above Darci, so don't put her shadow
									// on it.
									//
								}
								else
								{
									for (i = 0; i < 4; i++)
									{
										pp = &prim_points[p_f4->Points[face_order[i]]];

										poly[i].X = float(pp->X);
										poly[i].Y = float(pp->Y);
										poly[i].Z = float(pp->Z);
									}

									projected = SMAP_project_onto_poly(poly, 4);

									if (projected)
									{
										AENG_add_projected_fadeout_shadow_poly(projected);
									}
								}

								w_face = p_f4->Col2;
							}
							else
							{
								struct		RoofFace4	*rf;
								rf        = &roof_faces4[-w_face];
								face_height =  rf->Y;

								if (face_height > ((darci->WorldPos.Y + 0x11000) >> 8))
								{
									//
									// This face is above Darci, so don't put her shadow
									// on it.
									//
								}
								else
								{
									SLONG	mx,mz;
									mx=(rf->RX&127)<<8;
									mz=(rf->RZ&127)<<8;

									poly[0].X=(float)(mx);
									poly[0].Y=(float)(rf->Y);
									poly[0].Z=(float)(mz);

									poly[1].X=(float)(mx+256);
									poly[1].Y=(float)(rf->Y+(rf->DY[0]<<ROOF_SHIFT));
									poly[1].Z=(float)(mz);

									poly[2].X=(float)(mx+256);
									poly[2].Y=(float)(rf->Y+(rf->DY[1]<<ROOF_SHIFT));
									poly[2].Z=(float)(mz+256);

									poly[3].X=(float)(mx);
									poly[3].Y=(float)(rf->Y+(rf->DY[2]<<ROOF_SHIFT));
									poly[3].Z=(float)(mz+256);


									projected = SMAP_project_onto_poly(poly, 4);

									if (projected)
									{
										AENG_add_projected_fadeout_shadow_poly(projected);
									}
								}

								w_face = rf->Next;

							}
						}
					}
				}
			}

			TEXTURE_shadow_update();
		}
	}

	BreakTime("Done shadows");

	LOG_EXIT ( AENG_Draw_Shadows )


#ifndef TARGET_DC
// No reflections on DC.



	ANNOYINGSCRIBBLECHECK;

	//
	// Where we remember the bounding boxes of reflections.
	//

	LOG_ENTER ( AENG_Moon )

	struct
	{
		SLONG x1;
		SLONG y1;
		SLONG x2;
		SLONG y2;
		SLONG water_box;

	}     bbox[AENG_MAX_BBOXES];
	SLONG bbox_upto = 0;

	if (AENG_detail_moon_reflection && !(NIGHT_flag & NIGHT_FLAG_DAYTIME) && !(GAME_FLAGS & GF_NO_FLOOR))
	{
		//
		// The moon upside down...
		//

		float moon_x1;
		float moon_y1;
		float moon_x2;
		float moon_y2;

		if (SKY_draw_moon_reflection(
				AENG_cam_x,
				AENG_cam_y,
				AENG_cam_z,
				AENG_DRAW_DIST * 256.0F,
			   &moon_x1,
			   &moon_y1,
			   &moon_x2,
			   &moon_y2))
		{
			/*

			//
			// The moon is wibbled with polys now.
			//

			bbox[0].x1 = MAX((SLONG)moon_x1 - AENG_BBOX_PUSH_OUT, AENG_BBOX_PUSH_IN);
			bbox[0].y1 = MAX((SLONG)moon_y1, 0);
			bbox[0].x2 = MIN((SLONG)moon_x2 + AENG_BBOX_PUSH_OUT, DisplayWidth  - AENG_BBOX_PUSH_IN);
			bbox[0].y2 = MIN((SLONG)moon_y2, DisplayHeight);

			bbox[0].water_box = FALSE;

			bbox_upto = 1;

			*/
		}
	}

	LOG_EXIT ( AENG_Moon )

	ANNOYINGSCRIBBLECHECK;

	//
	// Draw the reflections of people.
	//

	LOG_ENTER ( AENG_People_Reflection )

	if (AENG_detail_people_reflection)
	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (p_thing->Class == CLASS_PERSON && (p_thing->Flags & FLAGS_IN_VIEW))
				{
					//
					// This is a person... Is she standing near a puddle or some water?
					//

					mx = p_thing->WorldPos.X >> 16;
					mz = p_thing->WorldPos.Z >> 16;

					if (!PAP_on_map_hi(mx,mz))
					{
						//
						// Off the map.
						//
					}
					else
					{
						ph = &PAP_2HI(mx,mz);

						if (ph->Flags & (PAP_FLAG_REFLECTIVE|PAP_FLAG_WATER))
						{
							//
							// Not too far away?
							//

							dx = abs(p_thing->WorldPos.X - FC_cam[AENG_cur_fc_cam].x >> 8);
							dz = abs(p_thing->WorldPos.Z - FC_cam[AENG_cur_fc_cam].z >> 8);

							if (dx + dz < 0x600)
							{
								SLONG reflect_height;

								//
								// Puddles are always on the floor nowadays...
								//

								if (ph->Flags & PAP_FLAG_REFLECTIVE)
								{
									reflect_height = PAP_calc_height_at(p_thing->WorldPos.X >> 8, p_thing->WorldPos.Z >> 8);
								}
								else
								{
									//
									// The height of the water is given by the lo-res mapsquare corresponding
									// to the hi-res mapsquare that Darci is standing on.
									//

									pl = &PAP_2LO(
											p_thing->WorldPos.X >> (8 + PAP_SHIFT_LO),
											p_thing->WorldPos.Z >> (8 + PAP_SHIFT_LO));

									reflect_height = pl->water << ALT_SHIFT;
								}

								//
								// Draw the reflection!
								//

								FIGURE_draw_reflection(p_thing, reflect_height);

								if (WITHIN(bbox_upto, 0, AENG_MAX_BBOXES - 1))
								{
									//
									// Create a new bounding box
									//

									bbox[bbox_upto].x1 = MAX(FIGURE_reflect_x1 - AENG_BBOX_PUSH_OUT, AENG_BBOX_PUSH_IN);
									bbox[bbox_upto].y1 = MAX(FIGURE_reflect_y1, 0);
									bbox[bbox_upto].x2 = MIN(FIGURE_reflect_x2 + AENG_BBOX_PUSH_OUT, DisplayWidth  - AENG_BBOX_PUSH_IN);
									bbox[bbox_upto].y2 = MIN(FIGURE_reflect_y2, DisplayHeight);

									bbox[bbox_upto].water_box = ph->Flags & PAP_FLAG_WATER;

									bbox_upto += 1;
								}
							}
						}
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	/*

	//
	// Draw the reflections of the OBs.
	//

	if(DETAIL_LEVEL&DETAIL_REFLECTIONS)
	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			for (oi = OB_find(x,z); oi->prim; oi += 1)
			{
				//
				// On map?
				//

				mx = oi->x >> 8;
				mz = oi->z >> 8;

				if (WITHIN(mx, 0, PAP_SIZE_HI - 1) &&
					WITHIN(mz, 0, PAP_SIZE_HI - 1))
				{
					//
					// On a reflective square?
					//

					if (PAP_2HI(mx,mz).Flags & (PAP_FLAG_WATER|PAP_FLAG_REFLECTIVE))
					{
						//
						// Not too far away?
						//

						dx = abs(oi->x - (FC_cam[AENG_cur_fc_cam].x >> 8));
						dz = abs(oi->z - (FC_cam[AENG_cur_fc_cam].z >> 8));

						if (dx + dz < 0x00)
						{
							MESH_draw_reflection(
								oi->prim,
								oi->x,
								oi->y,
								oi->z,
								oi->yaw,
								NULL);
						}
					}
				}
			}
		}
	}

	*/

	BreakTime("Drawn reflections");

	LOG_EXIT ( AENG_People_Reflection )

	ANNOYINGSCRIBBLECHECK;


#ifdef TARGET_DC
	if ( AENG_detail_moon_reflection || AENG_detail_people_reflection )
#endif
	{
		//
		// Drips inside puddles only...
		//

		AENG_draw_drips(1);

		//
		// Draw the reflections and drips.  Clear the poly lists.
		//

#ifndef TARGET_DC
		POLY_frame_draw(FALSE,FALSE);
		POLY_frame_init(TRUE,TRUE);
#endif
		BreakTime("Done first poly flush");
	}

	//
	// Draw the puddles.
	//

	LOG_ENTER ( AENG_Puddles )

	if (AENG_detail_puddles && !(GAME_FLAGS & GF_NO_FLOOR))
	{
		SLONG i;

		PUDDLE_Info *pi;

		float px1;
		float pz1;
		float px2;
		float pz2;

		float world_x;
		float world_y;
		float world_z;

		static struct
		{
			float u;
			float v;

		} texture_coords[4] =
		{
			{0.0F,0.0F},
			{1.0F,0.0F},
			{1.0F,1.0F},
			{0.0F,1.0F}
		};

		POLY_Point  pp  [4];
		POLY_Point *quad[4];

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
		{
			PUDDLE_get_start(z, NGAMUT_gamut[z].xmin, NGAMUT_gamut[z].xmax);

			while(pi = PUDDLE_get_next())
			{
				px1 = float(pi->x1);
				pz1 = float(pi->z1);
				px2 = float(pi->x2);
				pz2 = float(pi->z2);

				world_y = float(pi->y);

				for (i = 0; i < 4; i++)
				{
					world_x = (i & 0x1) ? px1 : px2;
					world_z = (i & 0x2) ? pz1 : pz2;

					POLY_transform(
						 world_x,
						 world_y,
						 world_z,
						&pp[i]);

					if (pp[i].MaybeValid())
					{
						pp[i].u        = ((i & 0x1) ? float(pi->u1) : float(pi->u2)) * (1.0F / 256.0F);
						pp[i].v        = ((i & 0x2) ? float(pi->v1) : float(pi->v2)) * (1.0F / 256.0F);
						pp[i].colour   = 0xffffffff;

						if (ControlFlag) {pp[i].colour = 0x44ffffff;}
						if (ShiftFlag)   {pp[i].colour = 0x88ffffff;}

						pp[i].specular = 0xff000000;
					}
					else
					{
						goto ignore_this_puddle;
					}
				}

				if (POLY_valid_quad(quad))
				{
					if ((GAME_FLAGS & GF_RAINING) && (rand() & 0x100))
					{
						float drip_along_x;
						float drip_along_z;

						//
						// Choose somewhere in the puddle to put a drip.
						//

						drip_along_x = float(rand() & 0xff) * (1.0F / 256.0F);
						drip_along_z = float(rand() & 0xff) * (1.0F / 256.0F);

						world_x = px1 + (px2 - px1) * drip_along_x;
						world_z = pz1 + (pz2 - pz1) * drip_along_z;

						DRIP_create(
							UWORD(world_x),
							SWORD(world_y),
							UWORD(world_z),
							1);

						if (rand() & 0x11)
						{
							//
							// Don't splash.
							//
						}
						else
						{
							//
							// Splash the puddle.
							//

							PUDDLE_splash(
								SLONG(world_x),
								SLONG(world_y),
								SLONG(world_z));
						}
					}

					if (pi->rotate_uvs)
					{
						SWAP_FL(pp[0].u, pp[1].u);
						SWAP_FL(pp[1].u, pp[3].u);
						SWAP_FL(pp[3].u, pp[2].u);

						SWAP_FL(pp[0].v, pp[1].v);
						SWAP_FL(pp[1].v, pp[3].v);
						SWAP_FL(pp[3].v, pp[2].v);
					}

					POLY_add_quad(quad, POLY_PAGE_PUDDLE, FALSE);

					if (pi->puddle_s1 | pi->puddle_s2)
					{
						//
						// Find the bounding box of this puddle quad on screen.
						//

						SLONG px;
						SLONG py;
						SLONG px1 = +INFINITY;
						SLONG py1 = +INFINITY;
						SLONG px2 = -INFINITY;
						SLONG py2 = -INFINITY;

						for (i = 0; i < 4; i++)
						{
							px = SLONG(pp[i].X);
							py = SLONG(pp[i].Y);

							if (px < px1) {px1 = px;}
							if (py < py1) {py1 = py;}
							if (px > px2) {px2 = px;}
							if (py > py2) {py2 = py;}
						}

						//
						// Wibble the intersection of this bounding box with the bounding
						// box of each reflection we have drawn.
						//

						SLONG ix1;
						SLONG iy1;
						SLONG ix2;
						SLONG iy2;

						for (i = 0; i < bbox_upto; i++)
						{
							if (bbox[i].water_box)
							{
								//
								// This box always gets wibbled anyway.
								//

								continue;
							}

							ix1 = MAX(px1, bbox[i].x1);
							iy1 = MAX(py1, bbox[i].y1);
							ix2 = MIN(px2, bbox[i].x2);
							iy2 = MIN(py2, bbox[i].y2);

							if (ix1 < ix2 && iy1 < iy2)
							{
								if (the_display.screen_lock())
								{
									//
									// Wibble away!
									//

									WIBBLE_simple(
										ix1, iy1,
										ix2, iy2,
										pi->puddle_y1,
										pi->puddle_y2,
										pi->puddle_g1,
										pi->puddle_g2,
										pi->puddle_s1,
										pi->puddle_s2);

									the_display.screen_unlock();
								}
							}
						}
					}
				}

			  ignore_this_puddle:;
			}
		}
	}

	BreakTime("Drawn puddles");

	LOG_EXIT ( AENG_Puddles )

	ANNOYINGSCRIBBLECHECK;

#ifndef	NEW_FLOOR
	if (AENG_detail_people_reflection)
#ifndef TARGET_DC
	if(!SOFTWARE)
#endif
	{
		SLONG oldcolour  [4];
		SLONG oldspecular[4];

		//
		// Draw all the floor-reflective squares
		//

		for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
		{
			for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
			{
#ifndef TARGET_DC
				me = &MAP[MAP_INDEX(x,z)];
#endif
				ph = &PAP_2HI(x,z);

				if (ph->Flags & PAP_FLAG_HIDDEN)
				{
					continue;
				}

				if (!(ph->Flags & PAP_FLAG_REFLECTIVE))
				{
					continue;
				}

				//
				// The four points of the quad.
				//

				if (ph->Flags & PAP_FLAG_SINK_SQUARE)
				{
					quad[0] = &AENG_lower[(x + 0) & 63][(z + 0) & 63];
					quad[1] = &AENG_lower[(x + 1) & 63][(z + 0) & 63];
					quad[2] = &AENG_lower[(x + 0) & 63][(z + 1) & 63];
					quad[3] = &AENG_lower[(x + 1) & 63][(z + 1) & 63];
				}
				else
				{
					quad[0] = &AENG_upper[(x + 0) & 63][(z + 0) & 63];
					quad[1] = &AENG_upper[(x + 1) & 63][(z + 0) & 63];
					quad[2] = &AENG_upper[(x + 0) & 63][(z + 1) & 63];
					quad[3] = &AENG_upper[(x + 1) & 63][(z + 1) & 63];
				}

				if (POLY_valid_quad(quad))
				{
					//
					// Darken the quad.
					//

					for (i = 0; i < 4; i++)
					{
						oldcolour  [i] = quad[i]->colour;
						oldspecular[i] = quad[i]->specular;

						quad[i]->colour >>= 1;
						quad[i]->colour  &= 0x007f7f7f;
#ifdef TARGET_DC
						quad[i]->colour  |= 0xff000000;
#endif
						quad[i]->specular = 0xff000000;
					}

					//
					// Texture the quad.
					//

					if (ph->Flags & PAP_FLAG_ANIM_TMAP)
					{
						struct	AnimTmap	*p_a;
						SLONG	cur;
						p_a=&anim_tmaps[ph->Texture];
						cur=p_a->Current;

						quad[0]->u = float(p_a->UV[cur][0][0] & 0x3f) * (1.0F / 32.0F);
						quad[0]->v = float(p_a->UV[cur][0][1]       ) * (1.0F / 32.0F);

						quad[1]->u = float(p_a->UV[cur][1][0]       ) * (1.0F / 32.0F);
						quad[1]->v = float(p_a->UV[cur][1][1]       ) * (1.0F / 32.0F);

						quad[2]->u = float(p_a->UV[cur][2][0]       ) * (1.0F / 32.0F);
						quad[2]->v = float(p_a->UV[cur][2][1]       ) * (1.0F / 32.0F);

						quad[3]->u = float(p_a->UV[cur][3][0]       ) * (1.0F / 32.0F);
						quad[3]->v = float(p_a->UV[cur][3][1]       ) * (1.0F / 32.0F);

						page   = p_a->UV[cur][0][0] & 0xc0;
						page <<= 2;
						page  |= p_a->Page[cur];
					}
					else
					{
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

					POLY_add_quad(quad, page, FALSE);

					//
					// Restore old colour info.
					//

					for (i = 0; i < 4; i++)
					{
						quad[i]->colour   = oldcolour  [i];
						quad[i]->specular = oldspecular[i];
					}
				}
			}
		}
	}
#endif
	BreakTime("Drawn reflective squares");

	/*

	//
	// Draw the water in the city.
	//

	{
		SLONG dmx;
		SLONG dmz;

		SLONG dx;
		SLONG dz;

		SLONG px;
		SLONG py;
		SLONG pz;

		SLONG ix;
		SLONG iz;

		POLY_Point  water_pp[5][5];
		POLY_Point *quad[4];
		POLY_Point *pp;

		SLONG user_upto = 1;

		for (i = 0, pp = &water_pp[0][0]; i < 25; i++, pp++)
		{
			pp->user = 0;
		}

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				pl = &PAP_2LO(x,z);

				if (pl->water == PAP_LO_NO_WATER)
				{
					//
					// No water in this square.
					//
				}
				else
				{
					//
					// The height of water in this lo-res mapsquare.
					//

					py = pl->water << ALT_SHIFT;

					//
					// Look for water in the hi-res mapsquare enclosed by this
					// lo-res mapsqure.
					//

					for (dmx = 0; dmx < 4; dmx++)
					for (dmz = 0; dmz < 4; dmz++)
					{
						mx = (x << 2) + dmx;
						mz = (z << 2) + dmz;

						ph = &PAP_2HI(mx,mz);

						if (ph->Flags & PAP_FLAG_WATER)
						{
							//
							// Transform all the points.
							//

							i = 0;

							for (i = 0; i < 4; i++)
							{
								dx = i &  1;
								dz = i >> 1;

								ix = dmx + dx;
								iz = dmz + dz;

								if (water_pp[ix][iz].user == user_upto)
								{
									//
									// Already transformed.
									//
								}
								else
								{
									//
									// Transform this point.
									//

									water_pp[ix][iz].user = user_upto;

									px = mx + dx << 8;
									pz = mz + dz << 8;

									POLY_transform(
										float(px),
										float(py),
										float(pz),
									   &water_pp[ix][iz]);

									water_pp[ix][iz].colour   = 0x44608564;
									water_pp[ix][iz].specular = 0xff000000;

									POLY_fadeout_point(&water_pp[ix][iz]);

									//
									// This point's (u,v) coordinates are a function of its
									// position and the time.
									//

									{
										float angle_u = float(((((mx + dx) * 5 + (mz + dz) * 6) & 0x1f) + 0x1f) * GAME_TURN) * (1.0F / 1754.0F);
										float angle_v = float(((((mx + dx) * 4 + (mz + dz) * 7) & 0x1f) + 0x1f) * GAME_TURN) * (1.0F / 1816.0F);

										water_pp[ix][iz].u = px * (1.0F / 256.0F) + sin(angle_u) * 0.15F;
										water_pp[ix][iz].v = pz * (1.0F / 256.0F) + cos(angle_v) * 0.15F;
									}
								}

								if (!water_pp[ix][iz].MaybeValid())
								{
									goto abandon_poly;
								}

								quad[i] = &water_pp[ix][iz];
							}

							if (POLY_valid_quad(quad))
							{
								POLY_add_quad(quad, POLY_PAGE_SEWATER, FALSE);
							}
						}

					  abandon_poly:;
					}

					user_upto += 1;
				}
			}
		}
	}

	*/

// End of reflection stuff.
#endif //#ifndef TARGET_DC


	//
	// The sky.
	//

	LOG_ENTER ( AENG_Draw_Sky )

extern	void SKY_draw_poly_sky_old(float world_camera_x,float world_camera_y,float world_camera_z,float world_camera_yaw,float max_dist,ULONG bot_colour,ULONG top_colour);

#ifdef TARGET_DC
	// Fade sky textures out a bit.
	if (AENG_detail_skyline)
		SKY_draw_poly_sky_old(AENG_cam_x,AENG_cam_y,AENG_cam_z,AENG_cam_yaw,AENG_DRAW_DIST * 256.0F,0x80808080,0x80808080);
#else
	if (AENG_detail_skyline)
		SKY_draw_poly_sky_old(AENG_cam_x,AENG_cam_y,AENG_cam_z,AENG_cam_yaw,AENG_DRAW_DIST * 256.0F,0xffffff,0xffffff);
#endif
/*
	SKY_draw_poly_clouds(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_DRAW_DIST * 256.0F);
  */
	if(!INDOORS_INDEX||outside)
	if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
	{
		SKY_draw_poly_moon(
			AENG_cam_x,
			AENG_cam_y,
			AENG_cam_z,
			/*AENG_DRAW_DIST * 256.0F*/
			256.0f * 22.0f);
	}

	LOG_EXIT ( AENG_Draw_Sky )



	ANNOYINGSCRIBBLECHECK;









	//
	// Draw the puddles and clear the poly lists.
	//

#ifdef TARGET_DC
	if ( AENG_detail_people_reflection && AENG_detail_puddles )
#endif
	{
#ifndef TARGET_DC
		POLY_frame_draw_odd();
		POLY_frame_draw_puddles();
#endif

#ifdef DEBUG
		POLY_frame_draw_sewater();
#endif

#ifndef TARGET_DC
		POLY_frame_init(TRUE,TRUE);
#endif
	}
	BreakTime("Done second polygon flush");






	//
	// FAR FACETS!!!!!!!!!!!!!
	//

	LOG_ENTER ( AENG_Draw_Skyline )

	FARFACET_draw(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		// Draw dist changes dynamically, so do it to a fixed distance.
		//AENG_DRAW_DIST * 4,
		90.0F*256.0F,

		AENG_LENS);

//#if defined(NDEBUG) || defined(TARGET_DC)
//	if (AENG_detail_skyline)
//		AENG_draw_far_facets();
//#endif

	LOG_EXIT ( AENG_Draw_Skyline )





	ANNOYINGSCRIBBLECHECK;


	//
	// Create all the squares.
	//

	//
	// draw floor draw_floor  //things to search for
	//

#ifndef	NEW_FLOOR
	LOG_ENTER ( AENG_Draw_Floors )

	SLONG num_squares_drawn = 0;

	{
		if(!INDOORS_INDEX||outside)
		for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
		{
			for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
			{
				ASSERT(WITHIN(x, 0, PAP_SIZE_HI - 2));
				ASSERT(WITHIN(z, 0, PAP_SIZE_HI - 2));

				ph = &PAP_2HI(x,z);

				if ((ph->Flags & (PAP_FLAG_HIDDEN|PAP_FLAG_ROOF_EXISTS))== PAP_FLAG_HIDDEN)
				{
					continue;
				}

	#ifdef FAST_EDDIE
				if (Keys[KB_1] && ((x ^ z) & 1))	continue;
	#endif

				/*

				if (ph->Flags & PAP_FLAG_WATER)
				{
					//
					// We don't draw the ground because it is covered by water.
					//
				}
				else

				*/
				{
					POLY_Point fake_roof[4];

					//
					// The four points of the quad.
					//
					if ((ph->Flags & (PAP_FLAG_ROOF_EXISTS)) )
					{
						SLONG	c0,light_lookup;
						float	y;

						y=MAVHEIGHT(x,z)<<6;//(PAP_hi[x][z].Height<<6);
						// 0   1
						//
						// 2   3

						POLY_transform((x)<<8,y, (z)<<8, &fake_roof[0]);
						POLY_transform((x+1)<<8,y, (z)<<8, &fake_roof[1]);
						POLY_transform((x)<<8,y, (z+1)<<8, &fake_roof[3]);
						POLY_transform((x+1)<<8,y, (z+1)<<8, &fake_roof[2]);


	extern	UWORD	hidden_roof_index[128][128];

						light_lookup=hidden_roof_index[x][z];

						ASSERT(light_lookup);

						for(c0=0;c0<4;c0++)
						{
							SLONG	c0_bodge;
							c0_bodge=index_lookup[c0];
							pp=&fake_roof[c0];
	//						pp->colour=0x808080;

							if (pp->MaybeValid())
							{
								NIGHT_get_d3d_colour(NIGHT_ROOF_WALKABLE_POINT(light_lookup,c0),&pp->colour,&pp->specular);

								apply_cloud((x+(c0_bodge&1))<<8,y,(z+((c0_bodge&2)>>1))<<8,&pp->colour);

								POLY_fadeout_point(pp);
							}
	//						pp->specular=0xff000000;
	//						pp->colour=0xff000000;


						}


						quad[0] = &fake_roof[0];
						quad[1] = &fake_roof[1];
						quad[2] = &fake_roof[3];
						quad[3] = &fake_roof[2];

					}
					else
					{
						if (GAME_FLAGS & GF_NO_FLOOR)
						{
							//
							// Don't draw the floor if there isn't any!
							//

							continue;
						}

						if (ph->Flags & PAP_FLAG_SINK_SQUARE)
						{
							quad[0] = &AENG_lower[(x + 0) & 63][(z + 0) & 63];
							quad[1] = &AENG_lower[(x + 1) & 63][(z + 0) & 63];
							quad[2] = &AENG_lower[(x + 0) & 63][(z + 1) & 63];
							quad[3] = &AENG_lower[(x + 1) & 63][(z + 1) & 63];
						}
						else
						{

							if((MAV_SPARE(x,z) & MAV_SPARE_FLAG_WATER))
								quad[0] = &AENG_lower[(x + 0) & 63][(z + 0) & 63];
							else
								quad[0] = &AENG_upper[(x + 0) & 63][(z + 0) & 63];

							if((MAV_SPARE(x+1,z) & MAV_SPARE_FLAG_WATER))
								quad[1] = &AENG_lower[(x + 1) & 63][(z + 0) & 63];
							else
								quad[1] = &AENG_upper[(x + 1) & 63][(z + 0) & 63];

							if((MAV_SPARE(x,z+1) & MAV_SPARE_FLAG_WATER))
								quad[2] = &AENG_lower[(x + 0) & 63][(z + 1) & 63];
							else
								quad[2] = &AENG_upper[(x + 0) & 63][(z + 1) & 63];

							if((MAV_SPARE(x+1,z+1) & MAV_SPARE_FLAG_WATER))
								quad[3] = &AENG_lower[(x + 1) & 63][(z + 1) & 63];
							else
								quad[3] = &AENG_upper[(x + 1) & 63][(z + 1) & 63];

						}
					}

					if (POLY_valid_quad(quad))
					{
						num_squares_drawn += 1;

						{
							//
							// Texture the quad.
							//
	/*
							if (ph->Flags & PAP_FLAG_ANIM_TMAP)
							{
								struct	AnimTmap	*p_a;
								SLONG	cur;
								p_a=&anim_tmaps[ph->Texture];
								cur=p_a->Current;

								quad[0]->u = float(p_a->UV[cur][0][0] & 0x3f) * (1.0F / 32.0F);
								quad[0]->v = float(p_a->UV[cur][0][1]       ) * (1.0F / 32.0F);

								quad[1]->u = float(p_a->UV[cur][1][0]       ) * (1.0F / 32.0F);
								quad[1]->v = float(p_a->UV[cur][1][1]       ) * (1.0F / 32.0F);

								quad[2]->u = float(p_a->UV[cur][2][0]       ) * (1.0F / 32.0F);
								quad[2]->v = float(p_a->UV[cur][2][1]       ) * (1.0F / 32.0F);

								quad[3]->u = float(p_a->UV[cur][3][0]       ) * (1.0F / 32.0F);
								quad[3]->v = float(p_a->UV[cur][3][1]       ) * (1.0F / 32.0F);

								page   = p_a->UV[cur][0][0] & 0xc0;
								page <<= 2;
								page  |= p_a->Page[cur];
							}
							else
	*/
							{
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

								// page = 86;
							}

#ifdef TARGET_DC
							// Key off the "mist" detail setting instead
							if (AENG_detail_mist)
#else
							if (AENG_detail_shadows)
#endif
								if(page==4*64+53)
								{
									SLONG	dx,dz,dist;

									dx=abs( (((SLONG)AENG_cam_x)>>8)-(x) );
									dz=abs( (((SLONG)AENG_cam_z)>>8)-(z) );

									dist=QDIST2(dx,dz);

									if(dist<15)
									{
										SLONG	sx,sy,sz;
	void draw_steam(SLONG x,SLONG y,SLONG z,SLONG lod);
										switch((ph->Texture >> 0xa) & 0x3)
										{
											case	0:
												sx=190;
												sz=128;
												break;
											case	1:
												sx=128;
												sz=66;
												break;
											case	2:
												sx=66;
												sz=128;
												break;
											case	3:
												sx=128;
												sz=190;
												break;
											default:
												ASSERT(0);
												break;

										}
										sx+=x<<8;
										sz+=z<<8;
										sy=PAP_calc_height_at(sx,sz);


										draw_steam(sx,sy,sz,10+(15-dist)*3);

									}
								}


							if (ph->Flags & (PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3))
							{
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

								for (i = 0; i < 4; i++)
								{
									red   = (ps[i].colour >> 16) & 0xff;
									green = (ps[i].colour >>  8) & 0xff;
									blue  = (ps[i].colour >>  0) & 0xff;

									red   -= 120;
									green -= 120;
									blue  -= 120;

									if (red   < 0) {red   = 0;}
									if (green < 0) {green = 0;}
									if (blue  < 0) {blue  = 0;}

									ps[i].colour = (red << 16) | (green << 8) | (blue << 0)|(0xff000000);
								}

								ASSERT(PAP_FLAG_SHADOW_1 == 1);

								switch(ph->Flags & (PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3))
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

										//ps[2].colour += 0x00101010;

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
								#if 0

								//
								// Crinkle? Not on the ground yet... I'll have to code rotation first.
								//

								if (/*crinkles enabled*/ 0 && page < 64 * 8 && TEXTURE_crinkle[page])
								{
									//
									// This quad could be crinkled!
									//

									if (quad[0]->z > 0.5F)
									{
										//
										// Too far away to be crinkled.
										//

										POLY_add_quad(quad, page, FALSE);
									}
									else
									if (quad[0]->z < 0.2F)
									{
										//
										// Maximum crinkleyness!
										//

										CRINKLE_do(
											TEXTURE_crinkle[page],
											page,
											1.0F,
											quad,
											FALSE);
									}
									else
									{
										float extrude;
										float av_z;

										//
										// Intermediate crinkle extrusion.
										//

										av_z  = quad[0]->z + quad[1]->z + quad[2]->z + quad[3]->z;
										av_z *= 0.25F;

										extrude  = av_z - 0.4F;
										extrude *= 1.0F / (0.3F - 0.4F);

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
												FALSE);
										}
										else
										{
											POLY_add_quad(quad, page, FALSE);
										}
									}
								}
								else

								#endif
								{
									POLY_add_quad(quad, page, FALSE);
								}
							}
						}
					}
				}

				if (ph->Flags & (PAP_FLAG_SINK_SQUARE|PAP_FLAG_WATER))
				{
					//
					// Do the curbs now.
					//

					static struct
					{
						SLONG dpx1;
						SLONG dpz1;
						SLONG dpx2;
						SLONG dpz2;

						SLONG dsx;
						SLONG dsz;

					} curb[4] =
					{
						{0,0,0,1,-1,0},
						{0,1,1,1,0,+1},
						{1,1,1,0,+1,0},
						{1,0,0,0,0,-1}
					};

					for (i = 0; i < 4; i++)
					{
						nx = x + curb[i].dsx;
						nz = z + curb[i].dsz;

						if (PAP_on_map_hi(nx,nz))
						{
							if (PAP_2HI(nx,nz).Flags & (PAP_FLAG_SINK_SQUARE|PAP_FLAG_WATER))
							{
								//
								// No need for a curb here.
								//
							}
							else
							{
								quad[0] = &AENG_lower[(x + curb[i].dpx1) & 63][(z + curb[i].dpz1) & 63];
								quad[1] = &AENG_lower[(x + curb[i].dpx2) & 63][(z + curb[i].dpz2) & 63];
								quad[2] = &AENG_upper[(x + curb[i].dpx1) & 63][(z + curb[i].dpz1) & 63];
								quad[3] = &AENG_upper[(x + curb[i].dpx2) & 63][(z + curb[i].dpz2) & 63];

								if (POLY_valid_quad(quad))
								{
									TEXTURE_get_minitexturebits_uvs(
										0,
									   &page,
									   &quad[0]->u,
									   &quad[0]->v,
									   &quad[1]->u,
									   &quad[1]->v,
									   &quad[2]->u,
									   &quad[2]->v,
									   &quad[3]->u,
									   &quad[3]->v);

									//
									// Add the poly.
									//

									POLY_add_quad(quad, page, TRUE);
								}
							}
						}
					}
				}
			}
		}
	}
#endif
	BreakTime("Drawn floors");

	LOG_EXIT ( AENG_Draw_Floors )

	ANNOYINGSCRIBBLECHECK;


//	POLY_frame_draw(FALSE,FALSE);
//	POLY_frame_init(TRUE,TRUE);
//	BreakTime("Done another flush");

	//
	// Draw the objects and the things.
	//

	LOG_ENTER ( AENG_Draw_Things )

	dfacets_drawn_this_gameturn = 0;
	{

		LOG_ENTER ( AENG_Draw_Prims )

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				OB_Info *oi;
				NIGHT_Colour *col;

				//
				// The cached lighting for this low-res mapsquare.
				//

				ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
				ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

				//ASSERT(WITHIN(NIGHT_cache[x][z], 1, NIGHT_MAX_SQUARES - 1));

				col = NIGHT_square[NIGHT_cache[x][z]].colour;

				//
				// Skip over the mapsquares.
				//

				col += PAP_BLOCKS * PAP_BLOCKS;

				//
				// The objects on this mapsquare.
				//

				oi = OB_find(x,z);

				while(oi->prim)
				{
					if (!(oi->flags & OB_FLAG_WAREHOUSE))
					{
						if (oi->prim == 133 || oi->prim == 235)
						{
							//
							// This prim slowly rotates...
							//

							oi->yaw = (GAME_TURN << 1) & 2047;
						}

#ifdef EDITOR
				UBYTE fade;
extern HWND GEDIT_edit_wnd;

				if (GEDIT_edit_wnd)
					fade=(oi->flags & OB_FLAG_NOT_ON_PSX)?0x7f:0xff;
				else
					fade=0xff;
#else
				UBYTE fade=0xff;

#endif

						col = MESH_draw_poly(
								oi->prim,
								oi->x,
								oi->y,
								oi->z,
								oi->yaw,
								oi->pitch,
								oi->roll,
								col,
								fade,
								oi->crumple);
						if(!SOFTWARE)
							SHAPE_prim_shadow(oi);

						if ((prim_objects[oi->prim].flag & PRIM_FLAG_GLARE)&& !SOFTWARE)
						{
							if (oi->prim == 230)
							{
								BLOOM_draw(oi->x,oi->y+48,oi->z, 0,0,0,0x808080,BLOOM_RAISE_LOS);
							}
							else
							{
								BLOOM_draw(oi->x,oi->y,oi->z, 0,0,0,0x808080,0);
							}
						}
						else
						if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME) && !SOFTWARE)
						{

							switch (oi->prim)
							{
							case 2:
								/*
								BLOOM_draw(oi->x+270,oi->y+350,oi->z, 0,-255,0,0x7f6500,BLOOM_BEAM);
								BLOOM_draw(oi->x-270,oi->y+350,oi->z, 0,-255,0,0x7f6500,BLOOM_BEAM);
								BLOOM_draw(oi->x,oi->y+350,oi->z+270, 0,-255,0,0x7f6500,BLOOM_BEAM);
								BLOOM_draw(oi->x,oi->y+350,oi->z-270, 0,-255,0,0x7f6500,BLOOM_BEAM);
								*/
								break;
							case 190:
								BLOOM_draw(oi->x,oi->y,oi->z, 0,0,0,0x808080,0);
								break;
							}
						}

						//
						// As good a place as any to put this!
						//

						if (prim_objects[oi->prim].flag & PRIM_FLAG_ITEM)
						{
							OB_ob[oi->index].yaw += 1;
						}
					}

					oi += 1;
				}
			}
		}

		LOG_EXIT ( AENG_Draw_Prims )

		ANNOYINGSCRIBBLECHECK;


		BreakTime("Drawn prims");
//		POLY_frame_draw(FALSE,FALSE);
//		POLY_frame_init(TRUE,TRUE);
//		BreakTime("Flushed prims");



		LOG_ENTER ( AENG_Draw_Facets )

		POLY_set_local_rotation_none();

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				//
				// The cached lighting for this low-res mapsquare.
				//

				ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
				ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

				//
				// Look at the colvects on this square.
				//

				{
					SLONG f_list;
					SLONG facet;
					SLONG build;
					SLONG exit = FALSE;

					f_list = PAP_2LO(x,z).ColVectHead;

					show_gamut_lo(x,z);

					if (f_list)
					{
						SLONG	count=0;
						while(!exit)
						{
							struct	DFacet	*p_vect;
							facet=facet_links[f_list];

							p_vect=&dfacets[facet];
/*
							AENG_world_text(x<<10,(PAP_2HI(x<<2,z<<2).Alt<<3)+count*50,z<<10,128,128,128,1,"x %d z %d %d type %d",x,z,facet,p_vect->FacetType);
							if(x==13 && z==5)
							AENG_world_line_infinite(p_vect->X[0],p_vect->Y[0]+10+count*50,p_vect->Z[0],2,0xffffff,
							  p_vect->X[1],p_vect->Y[1]+10+count*50,p_vect->Z[1],2,0xffffff,0);
							  */

							count++;

							ASSERT(facet);

							if (facet < 0)
							{
								//
								// The last facet in the list for each square
								// is negative.
								//

								facet = -facet;
								exit  =  TRUE;
							}

							if(dfacets[facet].Counter[AENG_cur_fc_cam] != SUPERMAP_counter[AENG_cur_fc_cam])
							{
//								ASSERT(facet!=676);
								//
								// Mark this facet as drawn this gameturn already.
								//

								dfacets[facet].Counter[AENG_cur_fc_cam] = SUPERMAP_counter[AENG_cur_fc_cam];

								if(dfacets[facet].FacetType==STOREY_TYPE_NORMAL)
									build = dfacets[facet].Building;
								else
									build = 0;

								//
								// Don't draw inside facets
								//
								if (build && dbuildings[build].Type == BUILDING_TYPE_CRATE_IN || (dfacets[facet].FacetFlags&FACET_FLAG_INSIDE))
								{
									//
									// Don't draw inside buildings outside.
									//
								}
								else
								if (dfacets[facet].FacetType == STOREY_TYPE_DOOR)
								{
									//
									// Draw the warehouse ground around this facet but don't draw
									// the facet.
									//

									AENG_draw_box_around_recessed_door(&dfacets[facet], FALSE);
								}
								else
								{
									//
									// Draw the facet.
									//

									show_facet(facet);
									FACET_draw(facet,0);

									//
									// Has this facet's building been processed this
									// gameturn yet?
									//

									switch(dfacets[facet].FacetType)
									{
										case	STOREY_TYPE_NORMAL:

											if (build)
											{
												if (dbuildings[build].Counter[AENG_cur_fc_cam] != SUPERMAP_counter[AENG_cur_fc_cam])
												{
													//
													// Draw all the walkable faces for this building.
													//

													FACET_draw_walkable(build);

													//
													// Mark the buiding as procesed this gameturn.
													//

													dbuildings[build].Counter[AENG_cur_fc_cam] = SUPERMAP_counter[AENG_cur_fc_cam];
												}
											}
											break;

									}
								}
							}

							f_list++;
						}
					}
				}
			}
		}

		BreakFacets(dfacets_drawn_this_gameturn);
		BreakTime("Drawn facets");
//		POLY_frame_draw(FALSE,FALSE);
//		POLY_frame_init(TRUE,TRUE);
//		BreakTime("Flushed facets");

		LOG_EXIT ( AENG_Draw_Facets )

		ANNOYINGSCRIBBLECHECK;


		LOG_ENTER ( AENG_Draw_Other_Things )

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				//
				// The cached lighting for this low-res mapsquare.
				//

				ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
				ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));


				t_index = PAP_2LO(x,z).MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->Flags & FLAGS_IN_VIEW)
					{
//						p_thing->Flags &=~FLAGS_IN_VIEW;
						switch(p_thing->DrawType)
						{
							case DT_NONE:
								break;

							case DT_BUILDING:
								break;

							case DT_PRIM:
								break;
							case DT_ANIM_PRIM:
								extern	void ANIM_obj_draw(Thing *p_thing,DrawTween *dt);
								ANIM_obj_draw(p_thing,p_thing->Draw.Tweened);

								if (p_thing->Class == CLASS_BAT &&
									p_thing->Genus.Bat->type == BAT_TYPE_BANE)
								{
									DRAWXTRA_final_glow(
										p_thing->WorldPos.X          >> 8,
										p_thing->WorldPos.Y + 0x8000 >> 8,
										p_thing->WorldPos.Z          >> 8,
									    p_thing->Genus.Bat->glow >> 8);
								}
								break;

							case DT_ROT_MULTI:
								LOG_ENTER ( AENG_Draw_DT_ROT_MULTI )

								/*
								if (ControlFlag)
								if (p_thing->Genus.Person->PlayerID)
								{
									//
									// Draw some wheels above Darci's head!
									//

									AENG_set_bike_wheel_rotation((GAME_TURN << 3) & 2047, PRIM_OBJ_BIKE_BWHEEL);

									MESH_draw_poly(
											PRIM_OBJ_BIKE_BWHEEL,
											p_thing->WorldPos.X          >> 8,
											p_thing->WorldPos.Y + 0xa000 >> 8,
											p_thing->WorldPos.Z          >> 8,
											p_thing->Draw.Tweened->Angle,
											0,0,
											NULL,0);

									AENG_set_bike_wheel_rotation((GAME_TURN << 3) & 2047, PRIM_OBJ_VAN_WHEEL);

									MESH_draw_poly(
											PRIM_OBJ_VAN_WHEEL,
											p_thing->WorldPos.X           >> 8,
											p_thing->WorldPos.Y + 0x10000 >> 8,
											p_thing->WorldPos.Z           >> 8,
											p_thing->Draw.Tweened->Angle,
											0,0,
											NULL,0);

									AENG_set_bike_wheel_rotation((GAME_TURN << 3) & 2047, PRIM_OBJ_CAR_WHEEL);

									MESH_draw_poly(
											PRIM_OBJ_CAR_WHEEL,
											p_thing->WorldPos.X           >> 8,
											p_thing->WorldPos.Y + 0x16000 >> 8,
											p_thing->WorldPos.Z           >> 8,
											p_thing->Draw.Tweened->Angle,
											0,0,
											NULL,0);
								}
								*/

								{
									ASSERT(p_thing->Class == CLASS_PERSON);

#ifdef BIKE
#error Better not be doing this.
									//
									// If this person is riding the bike...
									//

									if (p_thing->SubState == SUB_STATE_RIDING_BIKE)
									{
										Thing *p_bike = TO_THING(p_thing->Genus.Person->InCar);

										ASSERT(p_thing->Genus.Person->Flags & FLAG_PERSON_BIKING);
										ASSERT(p_thing->Genus.Person->InCar);

										BIKE_Drawinfo bdi = BIKE_get_drawinfo(p_bike);

										//
										// Move to the same position above the bike.
										//

										GameCoord newpos = p_bike->WorldPos;

										p_thing->Draw.Tweened->Angle = bdi.yaw;
										p_thing->Draw.Tweened->Tilt  = bdi.pitch;
										p_thing->Draw.Tweened->Roll  = bdi.roll;

										/*
										{
											SLONG roll = bdi.roll;

											if (roll > 1024)
											{
												roll -= 2048;
											}

											roll /= 2;
											roll &= 2047;

											p_thing->Draw.Tweened->Roll = roll;
										}
										*/

										{
											BIKE_Control bc;
											DrawTween   *dt = p_thing->Draw.Tweened;
											SLONG	steer;

											bc = BIKE_control_get(p_bike);
											steer=bc.steer>>1;

											if(steer>32)
												steer=32;
											else
												if(steer<-32)
													steer=-32;

											if(abs(steer)>21)
											{
												SLONG	tween;
												if(steer<0)
												{
													dt->CurrentFrame    =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_RIGHT];
													dt->NextFrame    =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_RIGHT_FOOT];
													tween    = ((-steer)-21) << 5;


												}
												else
												{
													dt->CurrentFrame    =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_LEFT];
													dt->NextFrame    =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_LEFT_FOOT];
													tween    = ((steer)-21) << 5;
												}
												if(tween<0)
													tween=0;
												if(tween>255)
													tween=255;

												dt->AnimTween=tween;
											}
											else
											if (bc.steer == 0)
											{
												//dt->CurrentFrame = dt->TheChunk->AnimList[248];
												//dt->NextFrame    = dt->TheChunk->AnimList[248];
												dt->CurrentFrame = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
												dt->NextFrame    = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
											}
											else
											if (bc.steer < 0)
											{

												//dt->CurrentFrame =  dt->TheChunk->AnimList[248];
												//dt->NextFrame    =  dt->TheChunk->AnimList[250];
												dt->CurrentFrame =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
												dt->NextFrame    =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_RIGHT];
												dt->AnimTween    = -steer << 3;
											}
											else
											{
												//dt->CurrentFrame = dt->TheChunk->AnimList[248];
												//dt->NextFrame    = dt->TheChunk->AnimList[252];
												dt->CurrentFrame = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
												dt->NextFrame    = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_LEFT];
												dt->AnimTween    = steer << 3;
											}
										}

										{
											GameCoord oldpos = p_thing->WorldPos;


											p_thing->WorldPos = newpos;
											FIGURE_draw(p_thing);

											p_thing->WorldPos = oldpos;
										}

										/*


									//	p_person->Draw.Tweened->Roll = bdi.roll;//BIKE_get_roll(TO_THING(p_person->Genus.Person->InCar));
									//	p_person->Draw.Tweened->Tilt = bdi.pitch;

									//	if (p_person->Draw.Tweened.Roll > 1024)

										*/
									}
									else
#endif
									{
										if (p_thing->Genus.Person->PlayerID)
										{
											if (FirstPersonMode)
											{
												FirstPersonAlpha -= (TICK_RATIO * 16) >> TICK_SHIFT;
												if (FirstPersonAlpha < MAX_FPM_ALPHA)	FirstPersonAlpha = MAX_FPM_ALPHA;
											}
											else
											{
												FirstPersonAlpha += (TICK_RATIO * 16) >> TICK_SHIFT;
												if (FirstPersonAlpha > 255)				FirstPersonAlpha = 255;
											}

											//FIGURE_alpha = FirstPersonAlpha;
											FIGURE_draw(p_thing);
											//FIGURE_alpha = 255;
										}
										else
										{
											SLONG	dx,dy,dz,dist;

											dx=fabs((p_thing->WorldPos.X >> 8)-AENG_cam_x);
											dy=fabs((p_thing->WorldPos.Y >> 8)-AENG_cam_y);
											dz=fabs((p_thing->WorldPos.Z >> 8)-AENG_cam_z);

											dist=QDIST3(dx,dy,dz);

											if(dist<AENG_DRAW_PEOPLE_DIST)
											{

												FIGURE_draw(p_thing);
											}
										}
									}

									p_thing->Draw.Tweened->Drawn=(UBYTE)SUPERMAP_counter;

									if (ControlFlag&&allow_debug_keys)
									{
										AENG_world_text(
											(p_thing->WorldPos.X >> 8),
											(p_thing->WorldPos.Y >> 8) + 0x60,
											(p_thing->WorldPos.Z >> 8),
											200,
											180,
											50,
											TRUE,
											PCOM_person_state_debug(p_thing));
									}

									#if DRAW_THIS_DEBUG_STUFF

									AENG_world_line(
										(p_thing->WorldPos.X >> 8),
										(p_thing->WorldPos.Y >> 8) + 0x60,
										(p_thing->WorldPos.Z >> 8),
										32,
										0x00ffffff,
										(x << PAP_SHIFT_LO) + (1 << (PAP_SHIFT_LO - 1)),
										(p_thing->WorldPos.Y >> 8),
										(z << PAP_SHIFT_LO) + (1 << (PAP_SHIFT_LO - 1)),
										0,
										0x0000ff00,
										FALSE);

									#endif
								}

#ifndef TARGET_DC
								#if NO_MORE_BALLOONS

								if (p_thing->Genus.Person->Balloon)
								{
									SLONG balloon;
									BALLOON_Balloon *bb;

									//
									// Draw this person's balloon.
									//

									for (balloon = p_thing->Genus.Person->Balloon; balloon; balloon = BALLOON_balloon[balloon].next)
									{
										SHAPE_draw_balloon(balloon);
									}
								}

								#endif
#endif

								if (p_thing->State == STATE_DEAD)
								{
									if (p_thing->Genus.Person->Timer1 > 10)
									{
										if (p_thing->Genus.Person->PersonType == PERSON_MIB1 ||
											p_thing->Genus.Person->PersonType == PERSON_MIB2 ||
											p_thing->Genus.Person->PersonType == PERSON_MIB3)
										{
											//
											// Dead MIB self destruct!
											//
											DRAWXTRA_MIB_destruct(p_thing);
/*
											SLONG px;
											SLONG py;
											SLONG pz;

											calc_sub_objects_position(
												p_thing,
												p_thing->Draw.Tweened->AnimTween,
												SUB_OBJECT_PELVIS,
											   &px,
											   &py,
											   &pz);

											px += p_thing->WorldPos.X >> 8;
											py += p_thing->WorldPos.Y >> 8;
											pz += p_thing->WorldPos.Z >> 8;

											//
											// Ripped from the DRAWXTRA_special!
											//

											// (So why didn't you put it there?!)

											{
												SLONG c0;
												SLONG dx;
												SLONG dz;

											  c0=3+(THING_NUMBER(p_thing)&7);
											  c0=(((GAME_TURN*c0)+(THING_NUMBER(p_thing)*9))<<4)&2047;
											  dx=SIN(c0)>>8;
											  dz=COS(c0)>>8;
											  BLOOM_draw(
												px, py+15, pz,
												dx,0,dz,0x9F2040,0);
											}*/
										}
									}
								}

								if (p_thing->Genus.Person->pcom_ai == PCOM_AI_BANE)
								{
									DRAWXTRA_final_glow(
										p_thing->WorldPos.X          >> 8,
										p_thing->WorldPos.Y + 0x8000 >> 8,
										p_thing->WorldPos.Z          >> 8,
									   -p_thing->Draw.Tweened->Tilt);
								}

								LOG_EXIT ( AENG_Draw_DT_ROT_MULTI )

								break;

							case DT_EFFECT:
								break;

							case DT_MESH:

								{
									// Weapons & other powerups.
									if (p_thing->Class == CLASS_SPECIAL) DRAWXTRA_Special(p_thing);

									MESH_draw_poly(
											p_thing->Draw.Mesh->ObjectId,
											p_thing->WorldPos.X >> 8,
											p_thing->WorldPos.Y >> 8,
											p_thing->WorldPos.Z >> 8,
											p_thing->Draw.Mesh->Angle,
											p_thing->Draw.Mesh->Tilt,
											p_thing->Draw.Mesh->Roll,
											NULL,0xff,0);
								}

								break;

							#ifdef BIKE


#error A bike! Are you mad?

							case DT_BIKE:

								ASSERT(p_thing->Class == CLASS_BIKE);
								{
									//
									// Nasty eh! But I can't be arsed to create a new drawtype.
									//

									BIKE_Drawinfo bdi = BIKE_get_drawinfo(p_thing);

									//
									// Draw the frame of the bike.
									//

									ANIM_obj_draw(p_thing, p_thing->Draw.Tweened);

									//
									// If the bike is parked or being mounted then the wheels are
									// included in the animating object.
									//

									if (p_thing->Genus.Bike->mode == BIKE_MODE_DRIVING)
									{
										AENG_set_bike_wheel_rotation(bdi.front_rot, PRIM_OBJ_BIKE_BWHEEL);

										MESH_draw_poly(
												PRIM_OBJ_BIKE_BWHEEL,
												bdi.front_x,
												bdi.front_y,
												bdi.front_z,
												bdi.steer,
												bdi.pitch,
												bdi.roll,
												NULL,0xff,0);

										AENG_set_bike_wheel_rotation(bdi.back_rot, PRIM_OBJ_BIKE_BWHEEL);

										MESH_draw_poly(
												PRIM_OBJ_BIKE_BWHEEL,
												bdi.back_x,
												bdi.back_y,
												bdi.back_z,
												bdi.yaw,
												0,
												bdi.roll,
												NULL,0xff,0);
									}

									// Now some bike fx... first the exhaust
									PARTICLE_Exhaust2(p_thing, 5, 16);

									if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
									{
										SLONG matrix[9], vector[3], dx,dy,dz;
//										FMATRIX_calc(matrix, 1024-bdi.steer, bdi.pitch, bdi.roll);
										FMATRIX_calc(matrix, bdi.steer, bdi.pitch, bdi.roll);
										FMATRIX_TRANSPOSE(matrix);
										vector[2]=-255; vector[1]=0; vector[0]=0;
										FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
										dx=vector[0]; dy=vector[1]; dz=vector[2];
										vector[2]=25; vector[1]=80; vector[0]=0;
										FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
										BLOOM_draw(bdi.front_x+vector[0],bdi.front_y+vector[1],bdi.front_z+vector[2],dx,dy,dz,0x606040,BLOOM_LENSFLARE|BLOOM_BEAM);

										FMATRIX_calc(matrix, bdi.yaw, bdi.pitch, bdi.roll);
										FMATRIX_TRANSPOSE(matrix);
										vector[2]=255; vector[1]=0; vector[0]=0;
										FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
										dx=vector[0]; dy=vector[1]; dz=vector[2];
										vector[2]=70; vector[1]=75; vector[0]=0;
										FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);

										BLOOM_draw(
											(p_thing->WorldPos.X >> 8)+vector[0],
											(p_thing->WorldPos.Y >> 8)+vector[1],
											(p_thing->WorldPos.Z >> 8)+vector[2],
											dx,dy,dz,0x800000,0);

									}
								}

								break;

							#endif

							case DT_VEHICLE:

								if(p_thing->Class==CLASS_VEHICLE)
								{
									if(p_thing->Genus.Vehicle->Driver)
									{
										TO_THING(p_thing->Genus.Vehicle->Driver)->Flags|=FLAGS_IN_VIEW;

									}
								}
								//
								// Set the tinted colour of this van.
								//

								{
#if 1
									ULONG car_colours[6] =
									{
										0xffffff00,
										0xffff00ff,
										0xff00ffff,
										0xffff0000,
										0xff00ff00,
										0xf00000ff
									};

									MESH_colour_and = car_colours[THING_NUMBER(p_thing) % 6];
#else

#define	DEFCOL(R,G,B)	(0xFF000000 | R | (G << 8) | (B << 16))

									static DWORD colours[7] =
									{
										DEFCOL(18,192,120),
										DEFCOL(255,14,90),
										DEFCOL(112,122,216),
										DEFCOL(176,158,54),
										DEFCOL(0,149,186),
										DEFCOL(194,162,34),
										DEFCOL(255,255,255),
									};

									int	col = THING_NUMBER(p_thing) % 7;
									MESH_colour_and = colours[col];
#endif
								}

								extern void draw_car(Thing *p_car);

								draw_car(p_thing);

#ifndef TARGET_DC
								if (ControlFlag && allow_debug_keys)
								{
									//
									// Draw a line towards where you have to be
									// to get into the van.
									//

									SLONG dx = -SIN(p_thing->Genus.Vehicle->Draw.Angle);
									SLONG dz = -COS(p_thing->Genus.Vehicle->Draw.Angle);

									SLONG ix = p_thing->WorldPos.X >> 8;
									SLONG iz = p_thing->WorldPos.Z >> 8;

									ix += dx >> 9;
									iz += dz >> 9;

									ix -= dz >> 9;
									iz += dx >> 9;

									AENG_world_line(
										p_thing->WorldPos.X >> 8, p_thing->WorldPos.Y >> 8, p_thing->WorldPos.Z >> 8, 64, 0x00ffffff,
										ix, p_thing->WorldPos.Y >> 8, iz, 0, 0x0000ff00, TRUE);
								}
#endif

								break;

							case DT_CHOPPER:
								CHOPPER_draw_chopper(p_thing);
								break;

							case DT_PYRO:
								PYRO_draw_pyro(p_thing);
								break;
							case DT_ANIMAL_PRIM:
#if 0
extern	void	ANIMAL_draw(Thing *p_thing);
								ANIMAL_draw(p_thing);
#endif
								break;

							case DT_TRACK:
							if(!INDOORS_INDEX)
								TRACKS_DrawTrack(p_thing);
								break;


							default:
								ASSERT(0);
								break;
						}
					}
					t_index = p_thing->Child;
				}

			}
		}
		LOG_EXIT ( AENG_Draw_Other_Things )
	}

	BreakTime("Drawn things");

	LOG_EXIT ( AENG_Draw_Things )


	ANNOYINGSCRIBBLECHECK;


//	POLY_frame_draw(FALSE,FALSE);
//	POLY_frame_init(TRUE,TRUE);
//	BreakTime("Flushed things");

	//
	// debug info for the car movement
	//
//void	draw_car_tracks(void);
//	if(!INDOORS_INDEX||outside)
//		draw_car_tracks();

	LOG_ENTER ( AENG_Draw_Oval_Shadows )

	if(!SOFTWARE)
	{
		//
		// Do oval shadows.
		//

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				t_index = PAP_2LO(x,z).MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->Flags & FLAGS_IN_VIEW)
					{
						bool	draw = true;

						for (int ii = 0; ii < shadow_person_upto; ii++)
						{
							if (shadow_person[ii].p_person == p_thing)
							{
								draw = false;
								break;
							}
						}

						if (draw)
						{
							switch(p_thing->Class)
							{
								case CLASS_PERSON:

									{

										SLONG px;
										SLONG py;
										SLONG pz;

										calc_sub_objects_position(
											p_thing,
											p_thing->Draw.Tweened->AnimTween,
											SUB_OBJECT_PELVIS,
										   &px,
										   &py,
										   &pz);

										px += p_thing->WorldPos.X >> 8;
										py += p_thing->WorldPos.Y >> 8;
										pz += p_thing->WorldPos.Z >> 8;

										OVAL_add(
											px,
											py,
											pz,
											130);
									}

									break;

								case CLASS_SPECIAL:

									OVAL_add(
										p_thing->WorldPos.X >> 8,
										p_thing->WorldPos.Y >> 8,
										p_thing->WorldPos.Z >> 8,
										100);

									break;

								case CLASS_BARREL:

									OVAL_add(
										p_thing->WorldPos.X >> 8,
										p_thing->WorldPos.Y >> 8,
										p_thing->WorldPos.Z >> 8,
										128);

									break;

								default:
									break;
							}
						}
					}

					t_index = p_thing->Child;
				}
			}
		}
	}

	LOG_EXIT ( AENG_Draw_Oval_Shadows )


	ANNOYINGSCRIBBLECHECK;

	//
	// The dirt.
	//

	LOG_ENTER ( AENG_Draw_Dirt )

	if(!INDOORS_INDEX||outside)
	if(AENG_detail_dirt)
		AENG_draw_dirt();

	LOG_EXIT ( AENG_Draw_Dirt )


	// Cope with some wacky internals.
	POLY_set_local_rotation_none();
	POLY_flush_local_rot();


	ANNOYINGSCRIBBLECHECK;

	// Grenades should be drawn here.
	DrawGrenades();

	ANNOYINGSCRIBBLECHECK;


	LOG_ENTER ( AENG_Draw_Ballons )

	//
	// The balloons that nobody is holding.
	//

#ifndef TARGET_DC
	if(!INDOORS_INDEX||outside)
	AENG_draw_released_balloons();
#endif

	LOG_EXIT ( AENG_Draw_Ballons )

	ANNOYINGSCRIBBLECHECK;

	//
	// The POWs!
	//

	LOG_ENTER ( AENG_Draw_POWs )

	AENG_draw_pows();

	LOG_EXIT ( AENG_Draw_POWs )



	ANNOYINGSCRIBBLECHECK;



	//
	// Draw the mist.
	//

	LOG_ENTER ( AENG_Draw_Mist )

	if(!INDOORS_INDEX||outside)
	if (AENG_detail_mist)
	{
		SLONG i;

		SLONG sx;
		SLONG sz;

		SLONG px;
		SLONG pz;
		SLONG detail;

		SLONG wx;
		SLONG wy;
		SLONG wz;

		// Internal gubbins.
		POLY_flush_local_rot();

		POLY_Point *pp;
		POLY_Point *quad[4];

		#define MAX_MIST_DETAIL 32

		POLY_Point mist_pp[MAX_MIST_DETAIL][MAX_MIST_DETAIL];

		MIST_get_start();

		while(detail = MIST_get_detail())
		{
			//
			// Only draw as much as we can!
			//

			if (detail > MAX_MIST_DETAIL)
			{
				detail = MAX_MIST_DETAIL;
			}

			for (px = 0; px < detail; px++)
			for (pz = 0; pz < detail; pz++)
			{
				pp = &mist_pp[px][pz];

				MIST_get_point(px, pz,
				   &wx,
				   &wy,
				   &wz);

				POLY_transform(
					float(wx),
					float(wy),
					float(wz),
				    pp);

				if (pp->MaybeValid())
				{
					pp->colour   = 0x88666666;
					pp->specular = 0xff000000;

					MIST_get_texture(
						px,
						pz,
					   &pp->u,
					   &pp->v);

					POLY_fadeout_point(pp);
				}
			}

			for (sx = 0; sx < detail - 1; sx++)
			for (sz = 0; sz < detail - 1; sz++)
			{
				quad[0] = &mist_pp[sx + 0][sz + 0];
				quad[1] = &mist_pp[sx + 1][sz + 0];
				quad[2] = &mist_pp[sx + 0][sz + 1];
				quad[3] = &mist_pp[sx + 1][sz + 1];

				if (POLY_valid_quad(quad))
				{
					POLY_add_quad(quad, POLY_PAGE_FOG, FALSE);
				}
			}
		}
	}

	LOG_EXIT ( AENG_Draw_Mist )

	ANNOYINGSCRIBBLECHECK;


	//
	// Rain.
	//

	LOG_ENTER ( AENG_Draw_Rain )


/*
#ifdef _DEBUG	// about time we removed this kind of crap
	if (Keys[KB_R] && !ShiftFlag)
	{
		Keys[KB_R] = 0;

		GAME_FLAGS ^= GF_RAINING;
	}
#endif
*/

	if(!INDOORS_INDEX||outside)
	{
		if (GAME_FLAGS & GF_RAINING)
		{
			if(AENG_detail_rain)
				AENG_draw_rain();
		}
	}

#ifdef _DEBUG
	if(Keys[KB_N])
	{
		Keys[KB_N]=0;

		if ((NIGHT_flag & NIGHT_FLAG_DAYTIME))
		{
			NIGHT_flag &=~NIGHT_FLAG_DAYTIME;
			DETAIL_LEVEL|=DETAIL_RAIN;
		}
		else
		{
			NIGHT_flag |=NIGHT_FLAG_DAYTIME;
			DETAIL_LEVEL&=~DETAIL_RAIN;
		}
	}
#endif

#if defined(_DEBUG) && defined(FAST_EDDIE) && 0
		POLY_Point	tpt[4];
		POLY_Point	*tptp[4];

		tpt[0].X = 0;	tpt[0].Y = 0; 	tpt[0].Z = 1;	tpt[0].u = 0;	tpt[0].v = 0;
		tpt[1].X = 64;	tpt[1].Y = 0; 	tpt[1].Z = 1;	tpt[1].u = 1;	tpt[1].v = 0;
		tpt[2].X = 0;	tpt[2].Y = 64; 	tpt[2].Z = 1;	tpt[2].u = 0;	tpt[2].v = 1;
		tpt[3].X = 64;	tpt[3].Y = 64; 	tpt[3].Z = 1;	tpt[3].u = 1;	tpt[3].v = 1;

		tpt[0].colour = 0xFFFFFFFF;		tpt[0].specular = 0xFF000000;
		tpt[1].colour = 0xFFFFFFFF;		tpt[1].specular = 0xFF000000;
		tpt[2].colour = 0xFFFFFFFF;		tpt[2].specular = 0xFF000000;
		tpt[3].colour = 0xFFFFFFFF;		tpt[3].specular = 0xFF000000;

		tptp[0] = &tpt[0];
		tptp[1] = &tpt[1];
		tptp[2] = &tpt[2];
		tptp[3] = &tpt[3];

		POLY_add_quad(tptp, POLY_PAGE_TEST_SHADOWMAP, FALSE, TRUE);
#endif

	LOG_EXIT ( AENG_Draw_Rain )

	ANNOYINGSCRIBBLECHECK;


	//
	// Drawing the steam.
	//

//	void draw_steam(SLONG x,SLONG y,SLONG z,SLONG lod);
//	void draw_flames(SLONG x,SLONG y,SLONG z,SLONG lod);

	//
	// Pyrotechincs.
	//

	LOG_ENTER ( AENG_Draw_Pyros )

	if(!INDOORS_INDEX||outside)
	AENG_draw_bangs();
	if(!INDOORS_INDEX||outside)
	AENG_draw_fire();
	if(!INDOORS_INDEX||outside)
	AENG_draw_sparks();
//	ANEG_draw_messages();

	LOG_EXIT ( AENG_Draw_Pyros )


	ANNOYINGSCRIBBLECHECK;

  /*

	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{
		PUDDLE_Info *pi;
			SLONG	sx,sy,sz;
			SLONG	dx,dy,dz,dist,lod;
			PUDDLE_get_start(z, NGAMUT_gamut[z].xmin, NGAMUT_gamut[z].xmax);

			while(pi = PUDDLE_get_next())
			{
				sx = (pi->x1+pi->x2)>>1;
				sz = (pi->z1+pi->z2)>>1;
				sy = pi->y;

				dx=abs( ((SLONG)AENG_cam_x>>0)-sx);
//				dy=abs( ((SLONG)AENG_cam_y>>0)-SLONG(world_y))*4;
				dy=abs( ((SLONG)AENG_cam_y>>0)-sy);
				dz=abs( ((SLONG)AENG_cam_z>>0)-sz);

				dist=QDIST3(dx,dy,dz);

				lod = 90-(dist/(32*2));
				if(lod>0)
					draw_steam(sx,sy,sz,lod);
//                    draw_flames(sx,sy,sz,lod);
			}
		}
*/


//	if (Keys[KB_RBRACE] && !ShiftFlag) {Keys[KB_RBRACE] = 0; AENG_torch_on ^= TRUE;}

	//
	// Draw a torch out of darci...
	//

#ifdef TARGET_DC
	// No more torches.
	ASSERT ( !AENG_torch_on );
#else
	if (AENG_torch_on)
	{
		Thing *darci = NET_PERSON(0);

		float angle;

		float dx;
		float dy;
		float dz;

		SLONG x;
		SLONG y;
		SLONG z;

		calc_sub_objects_position(
			darci,
			darci->Draw.Tweened->AnimTween,
			0,		// Torso?
		   &x,
		   &y,
		   &z);

		x += darci->WorldPos.X >> 8;
		y += darci->WorldPos.Y >> 8;
		z += darci->WorldPos.Z >> 8;

		angle  = float(darci->Draw.Tweened->Angle);
		angle *= 2.0F * PI / 2048.0F;

		float dyaw   = ((float)sin(float(GAME_TURN) * 0.025F))        * 0.25F;
		float dpitch = ((float)cos(float(GAME_TURN) * 0.020F) - 0.5F) * 0.25F;

		float matrix[3];

		dyaw += angle;
		dyaw += PI;

		MATRIX_vector(
			matrix,
			dyaw,
			dpitch);

		dx = matrix[0];
		dy = matrix[1];
		dz = matrix[2];

		// experimental light bloom
//		BLOOM_draw(x,y,z,dx*256,dy*256,dz*256,0x00666600);
		BLOOM_draw(x,y,z,dx*256,dy*256,dz*256,0x00ffffa0);

		CONE_create(
			float(x),
			float(y),
			float(z),
			dx,
			dy,
			dz,
			256.0F * 4.0F,
			256.0F,
			0x00666600,
			0x00000000,
			50);

		CONE_intersect_with_map();

//		CONE_draw();
	}
#endif

	//
	// Draw the tripwires.
	//

	LOG_ENTER ( AENG_Draw_Tripwires )

	{
		SLONG map_x1;
		SLONG map_z1;

		SLONG map_x2;
		SLONG map_z2;

		TRIP_Info *ti;

		// Deal with internal bollocks.
		POLY_flush_local_rot();

		TRIP_get_start();

		while(ti = TRIP_get_next())
		{
			//
			// Check whether this tripwire is on the map.
			//

			map_x1 = ti->x1 >> 8;
			map_z1 = ti->z1 >> 8;

			map_x2 = ti->x2 >> 8;
			map_z2 = ti->z2 >> 8;

			if ((WITHIN(map_z1, NGAMUT_zmin, NGAMUT_zmax) && WITHIN(map_x1, NGAMUT_gamut[map_z1].xmin, NGAMUT_gamut[map_z1].xmax)) ||
				(WITHIN(map_z2, NGAMUT_zmin, NGAMUT_zmax) && WITHIN(map_x2, NGAMUT_gamut[map_z2].xmin, NGAMUT_gamut[map_z2].xmax)))
			{
				//
				// Draw the bugger.
				//

				#define AENG_TRIPWIRE_WIDTH 0x3

				SHAPE_tripwire(
					ti->x1,
					ti->y,
					ti->z1,
					ti->x2,
					ti->y,
					ti->z2,
					AENG_TRIPWIRE_WIDTH,
					0x00661122,
					ti->counter,
					ti->along);
			}
		}
	}

	LOG_EXIT ( AENG_Draw_Tripwires )


	ANNOYINGSCRIBBLECHECK;


	//
	// Draw the hook.
	//

#ifndef TARGET_DC
	if(!INDOORS_INDEX||outside)
	AENG_draw_hook();
#endif

	//
	// Draw the sphere-matter.
	//

	LOG_ENTER ( AENG_Draw_Spherematter )

	if(!INDOORS_INDEX||outside)
	{
		SM_Info *si;

		SM_get_start();

		while(si = SM_get_next())
		{
			SHAPE_sphere(
				si->x,
				si->y,
				si->z,
				si->radius,
				si->colour);
		}
	}

	LOG_EXIT ( AENG_Draw_Spherematter )


	ANNOYINGSCRIBBLECHECK;


	//
	// Draw the drips... again!
	//

	LOG_ENTER ( AENG_Draw_Drips2 )

	if(!INDOORS_INDEX||outside)
		AENG_draw_drips(0);

	LOG_EXIT ( AENG_Draw_Drips2 )

	ANNOYINGSCRIBBLECHECK;

	//
	// Draw the particle system
	//

	LOG_ENTER ( AENG_Draw_Particles )

	if(!INDOORS_INDEX||outside)
		PARTICLE_Draw();

	LOG_EXIT ( AENG_Draw_Particles )

	ANNOYINGSCRIBBLECHECK;

	//
	// Draw the ribbon system
	//

	LOG_ENTER ( AENG_Draw_Ribbons )

	if(!INDOORS_INDEX||outside)
		RIBBON_draw();

	LOG_EXIT ( AENG_Draw_Ribbons )


	ANNOYINGSCRIBBLECHECK;

	//
	// Draw the tyre tracks (changed -- now turned into things)
	//
/*
	if (ControlFlag) {
		TRACKS_Draw();
	}*/

	//
	// Draw the cloth.
	//


//	if(!INDOORS_INDEX||outside)
//	AENG_draw_cloth();

	//
	// Draw.
	//
	//
	// Draw the people messages.
	//

	AENG_draw_people_messages();


//	MSG_add(" draw insides %d and %d \n",INDOORS_INDEX,INDOORS_INDEX_NEXT);

	/*

	POLY_frame_draw(TRUE,TRUE);
	if(INDOORS_INDEX_NEXT)
		AENG_draw_inside_floor(INDOORS_INDEX_NEXT,INDOORS_ROOM_NEXT,255); //downtairs non transparent

	POLY_frame_draw(TRUE,TRUE);
	if(INDOORS_INDEX)
	{
		AENG_draw_inside_floor(INDOORS_INDEX,INDOORS_ROOM,INDOORS_INDEX_FADE);

	}

	*/








	BreakTime("Drawn other crap");



	LOG_ENTER ( AENG_Poly_Flush )

#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
#endif

	LOG_EXIT ( AENG_Poly_Flush )

	BreakTime("Done final polygon flush");

	ANNOYINGSCRIBBLECHECK;


	// Tell the pyros we've done a frame.
	Pyros_EndOfFrameMarker();


//	ANEG_draw_messages();

	/*

	//
	// This really _is_ shite!
	//

	if (ShiftFlag)
	{
		static float focus = 0.95F;

		if (Keys[KB_P7]) {focus += 0.001F;}
		if (Keys[KB_P4]) {focus -= 0.001F;}

		POLY_frame_draw_focused(focus);
	}

	*/

//	TRACE("Total polys = %d\n", AENG_total_polys_drawn);

	//LOG_EXIT ( AENG_Draw_City )

	LOG_EVENT ( AENG_Draw_End )


	//TRACE ( "AengOut" );



}









void	AENG_draw_far_facets(void)
{
	SLONG	x,z;

	POLY_camera_set(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		129.0F*256.0F,//float(AENG_DRAW_DIST) * 256.0F*3.0F,
		AENG_LENS,
		POLY_SPLITSCREEN_NONE);

	AENG_calc_gamut_lo_only(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		64.0F*256.0F,//AENG_DRAW_DIST*3,
		AENG_LENS);


#ifdef DEBUG
	SLONG	count = 0;
#endif

		LOG_ENTER ( Skyline_Scan_Map_Square )

		for (z = AENG_gamut_lo_zmin; z <= AENG_gamut_lo_zmax; z++)
		{
			for (x = AENG_gamut_lo_xmin; x <= AENG_gamut_lo_xmax; x++)
			{
				//
				// The cached lighting for this low-res mapsquare.
				//

				ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
				ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

				//
				// Look at the colvects on this square.
				//

				{
					SLONG f_list;
					SLONG facet;
					SLONG build;
					SLONG exit = FALSE;

					f_list = PAP_2LO(x,z).ColVectHead;


					if (f_list)
					{

						LOG_ENTER ( Skyline_Draw_Facet )


						while(!exit)
						{
							struct	DFacet	*p_vect;
							facet=facet_links[f_list];

							p_vect=&dfacets[facet];

							ASSERT(facet);

							if (facet < 0)
							{
								//
								// The last facet in the list for each square
								// is negative.
								//

								facet = -facet;
								exit  =  TRUE;
							}

							if(dfacets[facet].Counter[AENG_cur_fc_cam] != SUPERMAP_counter[AENG_cur_fc_cam])
							{

								dfacets[facet].Counter[AENG_cur_fc_cam] = SUPERMAP_counter[AENG_cur_fc_cam];

								if(dfacets[facet].FacetType==STOREY_TYPE_NORMAL)
									build = dfacets[facet].Building;
								else
									build = 0;

								//
								// Don't draw inside facets
								//
								if (build && dbuildings[build].Type == BUILDING_TYPE_CRATE_IN || (dfacets[facet].FacetFlags&FACET_FLAG_INSIDE))
								{
									//
									// Don't draw inside buildings outside.
									//
								}
								else
								if (dfacets[facet].FacetType == STOREY_TYPE_DOOR)
								{
								}
								else
								{
									//
									// Draw the facet.
									//


									show_facet(facet);
extern	void FACET_draw_quick(SLONG facet,UBYTE alpha);
									FACET_draw_quick(facet,0);
#ifdef DEBUG
									count++;
#endif

								}
							}

							f_list++;
						}

						LOG_EXIT ( Skyline_Draw_Facet )


					}
				}
			}
		}

		//TRACE("Far facets = %d\n", count);

		LOG_EXIT ( Skyline_Scan_Map_Square )


	POLY_camera_set(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		float(AENG_DRAW_DIST) * 256.0F,
		AENG_LENS,
		POLY_SPLITSCREEN_NONE);
}


//
// Draws the warehouse that they player is in.
//

void AENG_draw_warehouse()
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG px;
	SLONG pz;

	SLONG dist;
	SLONG t_index;
	SLONG square;
	SLONG build;
	SLONG page;
	SLONG facet;
	SLONG f_list;
	SLONG exit;
	SLONG balloon;
	SLONG dfcache;
	SLONG next;

	ULONG colour;
	ULONG specular;

	float world_x;
	float world_y;
	float world_z;

	//
	// No cloud inside warehouses!
	//


	SLONG old_aeng_draw_cloud_flag = aeng_draw_cloud_flag; FALSE;

	aeng_draw_cloud_flag = FALSE;

	Thing           *p_thing;
	OB_Info         *oi;
	PAP_Hi          *ph;
	NIGHT_Square    *nq;
	NIGHT_Colour    *col;
	NIGHT_Dfcache   *ndf;
	POLY_Point      *pp;
	DFacet          *df;
#ifndef TARGET_DC
	//BALLOON_Balloon *bb;
#endif
	POLY_Point      *quad[4];

	//POLY_frame_init(FALSE,FALSE);

#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif

	//
	// Work out which things are in view.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				switch(p_thing->Class)
				{
					case CLASS_PERSON:

						//
						// Only draw people who are in warehouses.
						//
						if(p_thing->Genus.Person->PlayerID && (p_thing->Genus.Person->Flags & FLAG_PERSON_WAREHOUSE))
							p_thing->Flags |= FLAGS_IN_VIEW;
						else
						if (p_thing->Genus.Person->Flags & FLAG_PERSON_WAREHOUSE)
						{
							if (POLY_sphere_visible(
								float(p_thing->WorldPos.X >> 8),
								float(p_thing->WorldPos.Y >> 8) + KERB_HEIGHT,
								float(p_thing->WorldPos.Z >> 8),
								256.0F / (AENG_DRAW_DIST * 256.0F)))
							{
								p_thing->Flags |= FLAGS_IN_VIEW;
							}
						}

						break;

					default:

						//
						// Draw everything else that is on a HIDDEN square (i.e. is inside the
						// floorplan of the warehouse)
						//

						if (PAP_2HI(p_thing->WorldPos.X >> 16, p_thing->WorldPos.Z >> 16).Flags & PAP_FLAG_HIDDEN)
						{
							p_thing->Flags |= FLAGS_IN_VIEW;
						}

						break;
				}

				t_index = p_thing->Child;
			}

			NIGHT_square[NIGHT_cache[x][z]].flag &= ~NIGHT_SQUARE_FLAG_DELETEME;
		}
	}

	//
	// Rotate all the points.
	//
#ifndef	NEW_FLOOR
	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		for (x = NGAMUT_point_gamut[z].xmin; x <= NGAMUT_point_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, PAP_SIZE_HI - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_HI - 1));

			ph = &PAP_2HI(x,z);

			//
			// We only have upper points inside a warehouse.
			//

			world_x = x       * 256.0F;
			world_y = ph->Alt * float(1 << ALT_SHIFT);
			world_z = z       * 256.0F;

			pp = &AENG_upper[x & 63][z & 63];

			POLY_transform(
				world_x,
				world_y,
				world_z,
				pp);

			if (pp->MaybeValid())
			{
				//
				// Work out the colour of this point... what a palaver!
				//

				px = x >> 2;
				pz = z >> 2;

				dx = x & 0x3;
				dz = z & 0x3;

				ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
				ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

				square = NIGHT_cache[px][pz];

				ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
				ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);

				nq = &NIGHT_square[square];

				NIGHT_get_d3d_colour(
					nq->colour[dx + dz * PAP_BLOCKS],
				   &pp->colour,
				   &pp->specular);

				apply_cloud((SLONG)world_x,(SLONG)world_y,(SLONG)world_z,&pp->colour);

				POLY_fadeout_point(pp);

				colour   = pp->colour;
				specular = pp->specular;
			}
		}
	}
#endif
	//
	// Who shall we generate shadows for?
	//

	if (AENG_shadows_on)
	{
		Thing *darci = NET_PERSON(0);

		//
		// How many people do we generate shadows for?
		//

		#define AENG_NUM_SHADOWS 4

		struct
		{
			Thing *p_person;
			SLONG  dist;

		}      shadow_person[AENG_NUM_SHADOWS];
		SLONG  shadow_person_upto = 0;
		SLONG  shadow_person_worst_dist = -INFINITY;
		SLONG  shadow_person_worst_person;

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				t_index = PAP_2LO(x,z).MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->Class == CLASS_PERSON && (p_thing->Flags & FLAGS_IN_VIEW))
					{
						//
						// Distance from darci.
						//

						dx = p_thing->WorldPos.X - darci->WorldPos.X;
						dz = p_thing->WorldPos.Z - darci->WorldPos.Z;

						dist = abs(dx) + abs(dz);

						if (dist < 0x60000)
						{
							if (shadow_person_upto < AENG_NUM_SHADOWS)
							{
								//
								// Put this person in the shadow array.
								//

								shadow_person[shadow_person_upto].p_person = p_thing;
								shadow_person[shadow_person_upto].dist     = dist;

								//
								// Keep track of the furthest person away.
								//

								if (dist > shadow_person_worst_dist)
								{
									shadow_person_worst_dist   = dist;
									shadow_person_worst_person = shadow_person_upto;
								}

								shadow_person_upto += 1;
							}
							else
							{
								if (dist < shadow_person_worst_dist)
								{
									//
									// Replace the worst person.
									//

									ASSERT(WITHIN(shadow_person_worst_person, 0, AENG_NUM_SHADOWS - 1));

									shadow_person[shadow_person_worst_person].p_person = p_thing;
									shadow_person[shadow_person_worst_person].dist     = dist;

									//
									// Find the worst person
									//

									shadow_person_worst_dist = -INFINITY;

									for (i = 0; i < AENG_NUM_SHADOWS; i++)
									{
										if (shadow_person[i].dist > shadow_person_worst_dist)
										{
											shadow_person_worst_dist   = shadow_person[i].dist;
											shadow_person_worst_person = i;
										}
									}
								}
							}
						}
					}

					t_index = p_thing->Child;
				}
			}
		}

		//
		// Draw the people's shadow maps.
		//

		SLONG offset_x;
		SLONG offset_y;

		for (i = 0; i < shadow_person_upto; i++)
		{
			darci = shadow_person[i].p_person;

			memset(AENG_aa_buffer, 0, sizeof(AENG_aa_buffer));

			SMAP_person(
				darci,
				(UBYTE *) AENG_aa_buffer,
				AENG_AA_BUF_SIZE,
				AENG_AA_BUF_SIZE,
				0,
			   -255,
			   -0);

			//
			// Where do we put it in the shadow texture page? Hard code everything!
			//

			ASSERT(AENG_AA_BUF_SIZE    == 32);
			ASSERT(TEXTURE_SHADOW_SIZE == 64);

			offset_x = (i & 1) << 5;
			offset_y = (i & 2) << 4;

			//
			// Plonk it into the shadow texture page.
			//

			if (TEXTURE_shadow_lock())
			{
				SLONG  x;
				SLONG  y;
				UWORD *line;
				UBYTE *buf = (UBYTE *) AENG_aa_buffer;
				UWORD*	mapping = GetShadowPixelMapping();

				for (y = 0; y < AENG_AA_BUF_SIZE; y++)
				{
					line = &TEXTURE_shadow_bitmap[((y + offset_y) * TEXTURE_shadow_pitch >> 1) + offset_x];

					for (x = AENG_AA_BUF_SIZE - 1; x >= 0; x--)
					{
						*line++ = mapping[*buf++];
					}
				}

				TEXTURE_shadow_unlock();
			}

			//
			// How we map floating points coordinates from 0 to 1 onto
			// where we plonked the shadow map in the texture page.
			//

			AENG_project_offset_u = float(offset_x) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_offset_v = float(offset_y) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_mul_u    = float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_mul_v    = float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE);

			{
				//
				// Map this poly onto the mapsquares surrounding darci.
				//

				SLONG i;

				SLONG mx;
				SLONG mz;
				SLONG dx;
				SLONG dz;

				SLONG mx1;
				SLONG mz1;
				SLONG mx2;
				SLONG mz2;
				SLONG exit = FALSE;

				SLONG mx_lo;
				SLONG mz_lo;

#ifndef TARGET_DC
				MapElement *me[4];
#endif
				PAP_Hi     *ph[4];

				SVector_F  poly[4];
				SMAP_Link *projected;

				SLONG v_list;
				SLONG i_vect;

				DFacet *df;

				SLONG w_list;
				SLONG w_face;

				PrimFace4 *p_f4;
				PrimPoint *pp;

				SLONG wall;
				SLONG storey;
				SLONG building;
				SLONG thing;
				SLONG face_height;
				UBYTE face_order[4] = {0,1,3,2};

				Thing *p_fthing;

				//
				// Colvects we have already done.
				//

				#define AENG_MAX_DONE 8

				SLONG done[AENG_MAX_DONE];
				SLONG done_upto = 0;

				for (dx = -1; dx <= 1; dx++)
				for (dz = -1; dz <= 1; dz++)
				{
					mx = (darci->WorldPos.X >> 16) + dx;
					mz = (darci->WorldPos.Z >> 16) + dz;

					if (WITHIN(mx, 0, MAP_WIDTH  - 2) &&
						WITHIN(mz, 0, MAP_HEIGHT - 2))
					{
#ifndef TARGET_DC
						me[0] = &MAP[MAP_INDEX(mx + 0, mz + 0)];
						me[1] = &MAP[MAP_INDEX(mx + 1, mz + 0)];
						me[2] = &MAP[MAP_INDEX(mx + 1, mz + 1)];
						me[3] = &MAP[MAP_INDEX(mx + 0, mz + 1)];
#endif

						ph[0] = &PAP_2HI(mx + 0, mz + 0);
						ph[1] = &PAP_2HI(mx + 1, mz + 0);
						ph[2] = &PAP_2HI(mx + 1, mz + 1);
						ph[3] = &PAP_2HI(mx + 0, mz + 1);

						if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
						{
							poly[3].X = float(mx << 8);
							poly[3].Y = float(ph[0]->Alt << ALT_SHIFT);
							poly[3].Z = float(mz << 8);

							poly[0].X = poly[3].X + 256.0F;
							poly[0].Y = float(ph[1]->Alt << ALT_SHIFT);
							poly[0].Z = poly[3].Z;

							poly[1].X = poly[3].X + 256.0F;
							poly[1].Y = float(ph[2]->Alt << ALT_SHIFT);
							poly[1].Z = poly[3].Z + 256.0F;

							poly[2].X = poly[3].X;
							poly[2].Y = float(ph[3]->Alt << ALT_SHIFT);
							poly[2].Z = poly[3].Z + 256.0F;

							if (ph[0]->Alt == ph[1]->Alt &&
								ph[0]->Alt == ph[2]->Alt &&
								ph[0]->Alt == ph[3]->Alt)
							{
								//
								// This quad is coplanar.
								//

								projected = SMAP_project_onto_poly(poly, 4);

								if (projected)
								{
									AENG_add_projected_shadow_poly(projected);
								}
							}
							else
							{
								//
								// Do each triangle separatly.
								//

								projected = SMAP_project_onto_poly(poly, 3);

								if (projected)
								{
									AENG_add_projected_fadeout_shadow_poly(projected);
								}

								//
								// The triangles are 0,1,2 and 0,2,3.
								//

								poly[1] = poly[0];

								projected = SMAP_project_onto_poly(poly + 1, 3);

								if (projected)
								{
									AENG_add_projected_fadeout_shadow_poly(projected);
								}
							}
						}
					}
				}

				mx1 = (darci->WorldPos.X >> 8) - 0x100 >> PAP_SHIFT_LO;
				mz1 = (darci->WorldPos.Z >> 8) - 0x100 >> PAP_SHIFT_LO;

				mx2 = (darci->WorldPos.X >> 8) + 0x100 >> PAP_SHIFT_LO;
				mz2 = (darci->WorldPos.Z >> 8) + 0x100 >> PAP_SHIFT_LO;

				SATURATE(mx1, 0, PAP_SIZE_LO - 1);
				SATURATE(mz1, 0, PAP_SIZE_LO - 1);
				SATURATE(mx2, 0, PAP_SIZE_LO - 1);
				SATURATE(mz2, 0, PAP_SIZE_LO - 1);

				for (mx_lo = mx1; mx_lo <= mx2; mx_lo++)
				for (mz_lo = mz1; mz_lo <= mz2; mz_lo++)
				{
					//
					// Cast shadow on walkable faces.
					//

					w_face = PAP_2LO(mx_lo,mz_lo).Walkable;

					while(w_face)
					{
						if (w_face > 0)
						{
							p_f4        = &prim_faces4[w_face];
							face_height =  prim_points[p_f4->Points[0]].Y;

							if (face_height > ((darci->WorldPos.Y + 0x11000) >> 8))
							{
								//
								// This face is above Darci, so don't put her shadow
								// on it.
								//
							}
							else
							{
								for (i = 0; i < 4; i++)
								{
									pp = &prim_points[p_f4->Points[face_order[i]]];

									poly[i].X = float(pp->X);
									poly[i].Y = float(pp->Y);
									poly[i].Z = float(pp->Z);
								}

								projected = SMAP_project_onto_poly(poly, 4);

								if (projected)
								{
									AENG_add_projected_shadow_poly(projected);
								}
							}

							w_face = p_f4->Col2;
						}
						else
						{
							struct		RoofFace4	*rf;
							rf        = &roof_faces4[-w_face];
							face_height =  rf->Y;

							if (face_height > ((darci->WorldPos.Y + 0x11000) >> 8))
							{
								//
								// This face is above Darci, so don't put her shadow
								// on it.
								//
							}
							else
							{
								SLONG	mx,mz;

								mx=(rf->RX&127)<<8;
								mz=(rf->RZ&127)<<8;



								poly[0].X=(float)(mx);
								poly[0].Y=(float)(rf->Y);
								poly[0].Z=(float)(mz);

								poly[1].X=(float)((mx)+256);
								poly[1].Y=(float)(rf->Y+(rf->DY[0]<<ROOF_SHIFT));
								poly[1].Z=(float)((mz));

								poly[2].X=(float)((mx)+256);
								poly[2].Y=(float)(rf->Y+(rf->DY[1]<<ROOF_SHIFT));
								poly[2].Z=(float)((mz)+256);

								poly[3].X=(float)((mx));
								poly[3].Y=(float)(rf->Y+(rf->DY[2]<<ROOF_SHIFT));
								poly[3].Z=(float)((mz)+256);

								//
								// Assuming its coplanar!
								//

								projected = SMAP_project_onto_poly(poly, 4);

								if (projected)
								{
									AENG_add_projected_shadow_poly(projected);
								}
							}

							w_face = rf->Next;

						}
					}
				}
			}

			TEXTURE_shadow_update();
		}
	}
	else
	{
		//
		// Do oval shadows instead!
		//

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				t_index = PAP_2LO(x,z).MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->Flags & FLAGS_IN_VIEW)
					{
						switch(p_thing->Class)
						{
							case CLASS_PERSON:

								OVAL_add(
									p_thing->WorldPos.X >> 8,
									p_thing->WorldPos.Y >> 8,
									p_thing->WorldPos.Z >> 8,
									32);

								break;

							case CLASS_SPECIAL:

								OVAL_add(
									p_thing->WorldPos.X >> 8,
									p_thing->WorldPos.Y >> 8,
									p_thing->WorldPos.Z >> 8,
									16);

								break;

							case CLASS_BARREL:

								OVAL_add(
									p_thing->WorldPos.X >> 8,
									p_thing->WorldPos.Y >> 8,
									p_thing->WorldPos.Z >> 8,
									32);

								break;

							default:
								break;
						}
					}

					t_index = p_thing->Child;
				}
			}
		}
	}

#ifdef	NEW_FLOOR
	draw_quick_floor(1);
#endif

	//
	// Create all the squares.
	//
#ifndef	NEW_FLOOR
	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{
		for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, PAP_SIZE_HI - 2));
			ASSERT(WITHIN(z, 0, PAP_SIZE_HI - 2));

			ph = &PAP_2HI(x,z);

			if (!(ph->Flags & PAP_FLAG_HIDDEN))
			{
				//
				// We only draw hidden squares!
				//

				continue;
			}

			//
			// The four points of the quad.
			//

			quad[0] = &AENG_upper[(x + 0) & 63][(z + 0) & 63];
			quad[1] = &AENG_upper[(x + 1) & 63][(z + 0) & 63];
			quad[2] = &AENG_upper[(x + 0) & 63][(z + 1) & 63];
			quad[3] = &AENG_upper[(x + 1) & 63][(z + 1) & 63];

			if (POLY_valid_quad(quad))
			{
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

				POLY_add_quad(quad, page, TRUE);
			}
		}
	}
#endif

	//
	// Draw the objects and the things.
	//

	SLONG pos = 0;

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			/*

			{
				CBYTE str[20];

				sprintf(str, "%d,%d", x, z);

				FONT2D_DrawString(str, (pos & 7) * 60 + 20, (pos >> 3) * 20 + 200, 0xff00ff);
			}

			pos += 1;

			*/

			//
			// The cached lighting for this low-res mapsquare.
			//

			ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(NIGHT_cache[x][z], 1, NIGHT_MAX_SQUARES - 1));

			col = NIGHT_square[NIGHT_cache[x][z]].colour;

			//
			// Skip over the mapsquares.
			//

			col += PAP_BLOCKS * PAP_BLOCKS;

			//
			// The objects on this mapsquare.
			//

			oi = OB_find(x,z);

			while(oi->prim)
			{
				//
				// Only draw objects that are in buildings. (Assume that means our warehouse!)
				//

				if (oi->flags & OB_FLAG_WAREHOUSE)
				{
					if (oi->prim == 235)
					{
						oi->yaw = GAME_TURN;
					}

					col = MESH_draw_poly(
							oi->prim,
							oi->x,
							oi->y,
							oi->z,
							oi->yaw,
							oi->pitch,
							oi->roll,
							col,0xff,0);

					//
					// As good a place as any to put this!
					//

					if (prim_objects[oi->prim].flag & PRIM_FLAG_ITEM)
					{
						OB_ob[oi->index].yaw += 1;
					}
				}

				oi += 1;
			}

			//
			// Draw the facets on this square.
			//

			f_list = PAP_2LO(x,z).ColVectHead;

			if (f_list)
			{
				exit = FALSE;

				while(!exit)
				{
					facet = facet_links[f_list];

					ASSERT(facet);

					if (facet < 0)
					{
						//
						// The last facet in the list for each square is negative.
						//

						facet = -facet;
						exit  =  TRUE;
					}

					df = &dfacets[facet];

					if ( ( df->FacetType == STOREY_TYPE_NORMAL ) || ( df->FacetType == STOREY_TYPE_DOOR ) ||( df->FacetType == STOREY_TYPE_FENCE )||( df->FacetType == STOREY_TYPE_FENCE_FLAT ))
					{
						build = df->Building;

						//
						// Has this facet's building been processed this gameturn yet?
						//

						if (df->Counter[AENG_cur_fc_cam] != SUPERMAP_counter[AENG_cur_fc_cam])
						{
							//
							// warehouse walls only drawn if they are inside walls
							//

							if ( (dbuildings[build].Type == BUILDING_TYPE_WAREHOUSE && (df->FacetFlags&FACET_FLAG_INSIDE))||
								  dbuildings[build].Type == BUILDING_TYPE_CRATE_IN||df->FacetType == STOREY_TYPE_DOOR)
							{
								//
								// Draw the facet.
								//

								if (df->FacetType == STOREY_TYPE_DOOR)
								{
									//
									// Draw the outside around this facet but don't draw
									//

									AENG_draw_box_around_recessed_door(&dfacets[facet], TRUE);
								}
								else
								//if (df->FacetType == STOREY_TYPE_NORMAL) Why only draw normal facets?
								{
									FACET_draw(facet,0);

									build = df->Building;

									if (df->FacetType == STOREY_TYPE_NORMAL) //Why only draw normal facets?
									if (build)
									{
										if (dbuildings[build].Counter[AENG_cur_fc_cam] != SUPERMAP_counter[AENG_cur_fc_cam])
										{
											//
											// Draw all the walkable faces for this building.
											//

											FACET_draw_walkable(build);

											//
											// Mark the buiding as procesed this gameturn.
											//

											dbuildings[build].Counter[AENG_cur_fc_cam] = SUPERMAP_counter[AENG_cur_fc_cam];
										}
									}
								}
							}

							//
							// Mark this facet as drawn this gameturn already.
							//

							dfacets[facet].Counter[AENG_cur_fc_cam] = SUPERMAP_counter[AENG_cur_fc_cam];
						}
					}

					f_list++;
				}
			}

			//
			// Draw the things.
			//

			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (p_thing->Flags & FLAGS_IN_VIEW)
				{
					switch(p_thing->DrawType)
					{
						case DT_NONE:
							break;

						case DT_BUILDING:
							break;

						case DT_PRIM:
							break;

						case DT_ANIM_PRIM:
							ANIM_obj_draw(p_thing,p_thing->Draw.Tweened);
							break;

						case DT_ROT_MULTI:

							ASSERT(p_thing->Class == CLASS_PERSON);

							if (p_thing->Genus.Person->PlayerID)
							{
								if (FirstPersonMode)
								{
									FirstPersonAlpha -= (TICK_RATIO * 16) >> TICK_SHIFT;
									if (FirstPersonAlpha < MAX_FPM_ALPHA)	FirstPersonAlpha = MAX_FPM_ALPHA;
								}
								else
								{
									FirstPersonAlpha += (TICK_RATIO * 16) >> TICK_SHIFT;
									if (FirstPersonAlpha > 255)				FirstPersonAlpha = 255;
								}

								//FIGURE_alpha = FirstPersonAlpha;
								FIGURE_draw(p_thing);
								//FIGURE_alpha = 255;
							}
							else
							{
								SLONG	dx,dy,dz,dist;

								dx=fabs((p_thing->WorldPos.X >> 8)-AENG_cam_x);
								dy=fabs((p_thing->WorldPos.Y >> 8)-AENG_cam_y);
								dz=fabs((p_thing->WorldPos.Z >> 8)-AENG_cam_z);

								dist=QDIST3(dx,dy,dz);

								if(dist<AENG_DRAW_PEOPLE_DIST)
								{

									FIGURE_draw(p_thing);
								}
							}

							#if NO_MORE_BALLOONS_NOW

							if (p_thing->Genus.Person->Balloon)
							{
								//
								// Draw this person's balloon.
								//

								for (balloon = p_thing->Genus.Person->Balloon; balloon; balloon = BALLOON_balloon[balloon].next)
								{
									SHAPE_draw_balloon(balloon);
								}
							}

							#endif

							if (ControlFlag&&allow_debug_keys)
							{
								AENG_world_text(
									(p_thing->WorldPos.X >> 8),
									(p_thing->WorldPos.Y >> 8) + 0x60,
									(p_thing->WorldPos.Z >> 8),
									200,
									180,
									50,
									TRUE,
									PCOM_person_state_debug(p_thing));
							}

							if ((p_thing->State == STATE_DEAD)&&(p_thing->Genus.Person->Timer1 > 10))
							{
								if (p_thing->Genus.Person->PersonType == PERSON_MIB1 ||
									p_thing->Genus.Person->PersonType == PERSON_MIB2 ||
									p_thing->Genus.Person->PersonType == PERSON_MIB3)
								{
									//
									// Dead MIB self destruct!
									//
									DRAWXTRA_MIB_destruct(p_thing);
								}
							}

							break;

						case DT_EFFECT:
							break;

						case DT_MESH:

							if (p_thing->Class == CLASS_SPECIAL)
							{
								DRAWXTRA_Special(p_thing);
							}

							if (p_thing->Class == CLASS_BIKE)
							{
								//
								// There shouldn't be any bikes indoors.
								//

								ASSERT(0);
							}
							else
							{
								MESH_draw_poly(
										p_thing->Draw.Mesh->ObjectId,
										p_thing->WorldPos.X >> 8,
										p_thing->WorldPos.Y >> 8,
										p_thing->WorldPos.Z >> 8,
										p_thing->Draw.Mesh->Angle,
										p_thing->Draw.Mesh->Tilt,
										p_thing->Draw.Mesh->Roll,
										NULL,0xff,0);
							}

							break;

						case DT_VEHICLE:
						case DT_CHOPPER:

							//
							// There shouldn't be any vehicles or helicopters indoors.
							//

							break;

						case DT_PYRO:
							PYRO_draw_pyro(p_thing);
							break;

						case DT_ANIMAL_PRIM:
#if 0
							ANIMAL_draw(p_thing);
#endif
							break;

						case DT_TRACK:
							TRACKS_DrawTrack(p_thing);
							break;

						default:
							break;
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	//
	// Now draw the special fx that somebody left out before
	//

	POLY_set_local_rotation_none();
	PARTICLE_Draw();
	RIBBON_draw();
	AENG_draw_sparks();

	//
	// Now actually draw everything!
	//

#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
#endif

	//
	// Restore cloud to default value.
	//

	aeng_draw_cloud_flag = old_aeng_draw_cloud_flag;
}









//
// For drawing water.
//

#ifdef EDITOR
typedef struct
{
	UBYTE      height[4];
	POLY_Point pp[4];

} AENG_Nswater;

AENG_Nswater AENG_nswater[5][5];


void AENG_draw_ns()
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG x;
	SLONG z;

	float base_x;
	float base_z;

	float px;
	float py;
	float pz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG dropx;
	SLONG dropy;
	SLONG dropz;

	SLONG dropdx;
	SLONG dropdy;
	SLONG dropdz;

	SLONG along;

	SLONG bx;
	SLONG bz;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG dsx;
	SLONG dsz;

	SLONG sx;
	SLONG sz;

	SLONG ix;
	SLONG iz;

	SLONG page;

	NS_Cache   *nc;
	NS_Point   *np;
	NS_Face    *nf;
	NS_Texture *nt;
	NS_Lo      *nl;
	NS_St      *nst;

	POLY_Point *pp;
	POLY_Point *quad[4];

	THING_INDEX t_index;
	Thing      *p_thing;

	static bright = FALSE;

	if (Keys[KB_COLON])
	{
		Keys[KB_COLON] = 0;

		bright ^= TRUE;
	}

	//
	// Where we remember the bounding boxes of reflections.
	//

	struct
	{
		SLONG x1;
		SLONG y1;
		SLONG x2;
		SLONG y2;

	}     bbox[AENG_MAX_BBOXES];
	SLONG bbox_upto = 0;

	//
	// Create the gamut
	//

	AENG_calc_gamut(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		AENG_DRAW_DIST,
		AENG_LENS);

	//
	// Start the frame.
	//

#ifndef TARGET_DC
	POLY_frame_init(TRUE, TRUE);
#endif

	//
	// Go through the cache squares and free any we don't need.
	//

	for (i = 1; i < NS_MAX_CACHES; i++)
	{
		nc = &NS_cache[i];

		if (nc->used)
		{
			ASSERT(WITHIN(nc->map_x, 1, PAP_SIZE_LO - 2));
			ASSERT(WITHIN(nc->map_z, 1, PAP_SIZE_LO - 2));

			if (!WITHIN(nc->map_z, NGAMUT_lo_zmin, NGAMUT_lo_zmax) ||
				!WITHIN(nc->map_x, NGAMUT_lo_gamut[nc->map_z].xmin, NGAMUT_lo_gamut[nc->map_z].xmax))
			{
				//
				// We don't need this any more.
				//

				NS_cache_destroy(i);
			}
		}
	}

	//
	// Shadows.
	//

	if (Keys[KB_3]) {Keys[KB_3] = 0; AENG_shadows_on ^= TRUE;}

	if (AENG_shadows_on)
	{
		Thing *darci = NET_PERSON(0);

		//
		// Find the 'AENG_NUM_SHADOWS' nearest people to generate shadows for.
		//

		struct
		{
			Thing *p_person;
			SLONG  dist;

		}      shadow_person[AENG_NUM_SHADOWS];
		SLONG  shadow_person_upto = 0;
		SLONG  shadow_person_worst_dist = -INFINITY;
		SLONG  shadow_person_worst_person;

		for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
		{
			for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
			{
				t_index = PAP_2LO(x,z).MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->Class == CLASS_PERSON && (p_thing->Flags & FLAGS_IN_SEWERS))
					{
						if (POLY_sphere_visible(
								float(p_thing->WorldPos.X >> 8),
								float(p_thing->WorldPos.Y >> 8) + KERB_HEIGHT,
								float(p_thing->WorldPos.Z >> 8),
								256.0F / (AENG_DRAW_DIST * 256.0F)))
						{
							//
							// Distance from darci.
							//

							dx = p_thing->WorldPos.X - darci->WorldPos.X;
							dz = p_thing->WorldPos.Z - darci->WorldPos.Z;

							dist = abs(dx) + abs(dz);

							if (shadow_person_upto < AENG_NUM_SHADOWS)
							{
								//
								// Put this person in the shadow array.
								//

								shadow_person[shadow_person_upto].p_person = p_thing;
								shadow_person[shadow_person_upto].dist     = dist;

								//
								// Keep track of the furthest person away.
								//

								if (dist > shadow_person_worst_dist)
								{
									shadow_person_worst_dist   = dist;
									shadow_person_worst_person = shadow_person_upto;
								}

								shadow_person_upto += 1;
							}
							else
							{
								if (dist < shadow_person_worst_dist)
								{
									//
									// Replace the worst person.
									//

									ASSERT(WITHIN(shadow_person_worst_person, 0, AENG_NUM_SHADOWS - 1));

									shadow_person[shadow_person_worst_person].p_person = p_thing;
									shadow_person[shadow_person_worst_person].dist     = dist;

									//
									// Find the worst person
									//

									shadow_person_worst_dist = -INFINITY;

									for (i = 0; i < AENG_NUM_SHADOWS; i++)
									{
										if (shadow_person[i].dist > shadow_person_worst_dist)
										{
											shadow_person_worst_dist   = shadow_person[i].dist;
											shadow_person_worst_person = i;
										}
									}
								}
							}
						}
					}

					t_index = p_thing->Child;
				}
			}
		}

		//
		// Draw the people's shadow maps.
		//

		SLONG mx;
		SLONG mz;

		SLONG light_x;
		SLONG light_y;
		SLONG light_z;

		SLONG light_dx;
		SLONG light_dy;
		SLONG light_dz;

		SLONG offset_x;
		SLONG offset_y;

		NS_Lo *nl;

		for (i = 0; i < shadow_person_upto; i++)
		{
			darci = shadow_person[i].p_person;

			mx = darci->WorldPos.X >> (8 + PAP_SHIFT_LO);
			mz = darci->WorldPos.Z >> (8 + PAP_SHIFT_LO);

			if (!WITHIN(mx, 0, PAP_SIZE_LO - 1) ||
				!WITHIN(mz, 0, PAP_SIZE_LO - 1))
			{
				continue;
			}

			nl = &NS_lo[mx][mz];

			//
			// If this lo-res sewer square doesn't have a light, then
			// we don't have to do any shadow stuff.
			//

			if (nl->light_y == 0)
			{
				continue;
			}

			//
			// Where is the light in this lo-res mapsquare?
			//

			light_x = (mx << PAP_SHIFT_LO) + (nl->light_x << 3);
			light_z = (mz << PAP_SHIFT_LO) + (nl->light_z << 3);
			light_y = (nl->light_y << 5) + -32 * 0x100;

			//
			// The direction vector of the light creating the shadow-
			// we dont have to bother normalising it.
			//

			light_dx = (darci->WorldPos.X >> 8) - light_x;
			light_dy = (darci->WorldPos.Y >> 8) - light_y;
			light_dz = (darci->WorldPos.Z >> 8) - light_z;

			//
			// Draw the shadow of the person from the light.
			//

			memset(AENG_aa_buffer, 0, sizeof(AENG_aa_buffer));

			SMAP_person(
				darci,
				(UBYTE *) AENG_aa_buffer,
				AENG_AA_BUF_SIZE,
				AENG_AA_BUF_SIZE,
				light_dx,
				light_dy,
				light_dz);

			//
			// Where do we put it in the shadow texture page? Hard code everything!
			//

			ASSERT(AENG_AA_BUF_SIZE    == 32);
			ASSERT(TEXTURE_SHADOW_SIZE == 64);

			offset_x = (i & 1) << 5;
			offset_y = (i & 2) << 4;

			//
			// Plonk it into the shadow texture page.
			//

			if (TEXTURE_shadow_lock())
			{
				SLONG  x;
				SLONG  y;
				UWORD *line;
				UBYTE *buf = (UBYTE *) AENG_aa_buffer;
				UWORD*	mapping = GetShadowPixelMapping();

				for (y = 0; y < AENG_AA_BUF_SIZE; y++)
				{
					line = &TEXTURE_shadow_bitmap[((y + offset_y) * TEXTURE_shadow_pitch >> 1) + offset_x];

					for (x = AENG_AA_BUF_SIZE - 1; x >= 0; x--)
					{
						*line++ = mapping[*buf++];
					}
				}

				TEXTURE_shadow_unlock();
			}

			//
			// How we map floating points coordinates from 0 to 1 onto
			// where we plonked the shadow map in the texture page.
			//

			AENG_project_offset_u         = float(offset_x)         / float(TEXTURE_SHADOW_SIZE);
			AENG_project_offset_v         = float(offset_y)         / float(TEXTURE_SHADOW_SIZE);
			AENG_project_mul_u            = float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_mul_v            = float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE);
			AENG_project_lit_light_x      = float(light_x);
			AENG_project_lit_light_y      = float(light_y);
			AENG_project_lit_light_z      = float(light_z);
			AENG_project_lit_light_range  = 600.0F;

			//
			// Map the shadow polygon onto the sewers around each Darci.
			//

			{
				SLONG mx1;
				SLONG mz1;

				SLONG mx2;
				SLONG mz2;

				SLONG px;
				SLONG py;
				SLONG pz;

				SLONG base_x;
				SLONG base_z;

				NS_Hi    *nh;
				NS_Lo    *nl;
				NS_Cache *nc;
				NS_Point *np_base;
				NS_Face  *nf_base;
				NS_Point *np;
				NS_Face  *nf;

				SLONG face_point;
				UBYTE face_order[4] = {0,1,3,2};

				SVector_F  poly[4];
				SMAP_Link *projected;

				mx1 = (darci->WorldPos.X - 0x10000) >> (8 + PAP_SHIFT_LO);
				mz1 = (darci->WorldPos.Z - 0x10000) >> (8 + PAP_SHIFT_LO);
				mx2 = (darci->WorldPos.X + 0x10000) >> (8 + PAP_SHIFT_LO);
				mz2 = (darci->WorldPos.Z + 0x10000) >> (8 + PAP_SHIFT_LO);

				SATURATE(mx1, 0, PAP_SIZE_LO - 1);
				SATURATE(mz1, 0, PAP_SIZE_LO - 1);
				SATURATE(mx2, 0, PAP_SIZE_LO - 1);
				SATURATE(mz2, 0, PAP_SIZE_LO - 1);

				for (mx = mx1; mx <= mx2; mx++)
				for (mz = mz1; mz <= mz2; mz++)
				{
					nl = &NS_lo[mx][mz];

					if (nl->cache == NULL)
					{
						//
						// We can't see this lo-res mapsquare, so there is no point projecting
						// a shadow onto it!
						//

						continue;
					}
					ASSERT(WITHIN(nl->cache, 1, NS_MAX_CACHES - 1));

					nc = &NS_cache[nl->cache];

					//
					// The origin of this lo-res mapsquare.
					//

					base_x = mx << PAP_SHIFT_LO;
					base_z = mz << PAP_SHIFT_LO;

					//
					// Point and face memory.
					//

					np_base = (NS_Point *)  nc->memory;
					nf_base = (NS_Face  *) &np_base[nc->num_points];

					for (j = 0, nf = nf_base; j < nc->num_faces; j++, nf++)
					{
						for (k = 0; k < 4; k++)
						{
							face_point = face_order[k];

							ASSERT(WITHIN(nf->p[face_point], 0, nc->num_points - 1));

							np = &np_base[nf->p[face_point]];

							px = base_x + (np->x << 3);
							pz = base_z + (np->z << 3);

							py = (np->y << 5) + -32 * 0x100;

							poly[k].X = float(px);
							poly[k].Y = float(py);
							poly[k].Z = float(pz);
						}

						projected = SMAP_project_onto_poly(poly, 4);

						if (projected)
						{
							//
							// We have generated a shadow poly!
							//

							AENG_add_projected_lit_shadow_poly(projected);
						}
					}
				}
			}
		}
	}

	TEXTURE_shadow_update();

	//
	// Draw the reflections of people in the sewers.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (!(p_thing->Flags & FLAGS_IN_SEWERS))
				{
					//
					// Only draw things in the sewers.
					//
				}
				else
				{
					if (p_thing->DrawType == DT_ROT_MULTI)
					{
						if (POLY_sphere_visible(
								float(p_thing->WorldPos.X >> 8),
								float(p_thing->WorldPos.Y >> 8) + KERB_HEIGHT,
								float(p_thing->WorldPos.Z >> 8),
								256.0F / (AENG_DRAW_DIST * 256.0F)))
						{
							SLONG mx = p_thing->WorldPos.X >> 16;
							SLONG mz = p_thing->WorldPos.Z >> 16;

							ASSERT(WITHIN(mx, 0, PAP_SIZE_HI - 1));
							ASSERT(WITHIN(mz, 0, PAP_SIZE_HI - 1));

							NS_Hi *nh = &NS_hi[mx][mz];

							if (NS_HI_TYPE(nh) != NS_HI_TYPE_ROCK  &&
								NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE &&
								nh->water)
							{
								//
								// The height of the reflection.
								//

								SLONG reflect_height = (nh->water << 5) + (-32 * 0x100);

								FIGURE_draw_reflection(p_thing, reflect_height);

								if (WITHIN(bbox_upto, 0, AENG_MAX_BBOXES - 1))
								{
									//
									// Create a new bounding box
									//

									bbox[bbox_upto].x1 = MAX(FIGURE_reflect_x1 - AENG_BBOX_PUSH_OUT, AENG_BBOX_PUSH_IN);
									bbox[bbox_upto].y1 = MAX(FIGURE_reflect_y1, 0);
									bbox[bbox_upto].x2 = MIN(FIGURE_reflect_x2 + AENG_BBOX_PUSH_OUT, DisplayWidth  - AENG_BBOX_PUSH_IN);
									bbox[bbox_upto].y2 = MIN(FIGURE_reflect_y2, DisplayHeight);

									bbox_upto += 1;
								}
							}
						}
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	//
	// Draw the reflections.
	// Clear the poly lists.
	//

#ifndef TARGET_DC
	POLY_frame_draw(FALSE,FALSE);
	POLY_frame_init(TRUE,TRUE);
#endif

	//
	// Draw the water in all the lo-res mapsquares. Create the ones
	// that aren't already cached.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		if (z == 0 || z == PAP_SIZE_LO - 1)
		{
			continue;
		}

		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			if (x == 0 || x == PAP_SIZE_LO - 1)
			{
				continue;
			}

			ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

			nl = &NS_lo[x][z];

			if (nl->cache == NULL)
			{
				//
				// Create the points and faces for this mapsquare.
				//

				if (!NS_cache_create(x,z))
				{
					//
					// Houston, we have a problem [grrrrrrrgh]
					//

					continue;
				}
			}

			ASSERT(WITHIN(nl->cache, 1, NS_MAX_CACHES - 1));

			nc = &NS_cache[nl->cache];

			base_x = float(x << PAP_SHIFT_LO);
			base_z = float(z << PAP_SHIFT_LO);

			//
			// Draw the water.
			//

			{
				float water_height;

				AENG_Nswater *answ;

				NS_Hi *nh;

				//
				// Mark all the water points as untransformed.
				//

				answ = &AENG_nswater[0][0];

				for (i = 0; i < 25; i++)
				{
					answ->height[0] = 0;
					answ->height[1] = 0;
					answ->height[2] = 0;
					answ->height[3] = 0;

					answ += 1;
				}

				//
				// Draw the water squares.
				//

				bx = x << 2;
				bz = z << 2;

				for (dx = 0; dx < 4; dx++)
				for (dz = 0; dz < 4; dz++)
				{
					sx = bx + dx;
					sz = bz + dz;

					ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

					nh = &NS_hi[sx][sz];

					//
					// Curve squares use the water field as the type of curve...
					//

					if (nh->water && NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE)
					{
						//
						// The height of this square.
						//

						water_height = float((nh->water << 5) + (-32 * 0x100));

						for (i = 0; i < 4; i++)
						{
							ix = dx + (i  & 1);
							iz = dz + (i >> 1);

							answ = &AENG_nswater[ix][iz];

							for (j = 0; j < 4; j++)
							{
								if (answ->height[j] == nh->water)
								{
									quad[i] = &answ->pp[j];

									goto found_point;
								}

								if (answ->height[j] == 0)
								{
									//
									// We must create a new point.
									//

									POLY_transform(
										float(bx + ix << 8),
										water_height,
										float(bz + iz << 8),
									   &answ->pp[j]);

									if (answ->pp[j].MaybeValid())
									{
										answ->pp[j].colour   = 0xaa4488aa;
										answ->pp[j].specular = 0xff000000;

										POLY_fadeout_point(&answ->pp[j]);
									}

									quad[i] = &answ->pp[j];

									goto found_point;
								}
							}

						  found_point:;
						}

						if (POLY_valid_quad(quad))
						{
							//
							// No textures?
							//

							quad[0]->u = 0.0F;
							quad[0]->v = 0.0F;
							quad[1]->u = 1.0F;
							quad[1]->v = 0.0F;
							quad[2]->u = 0.0F;
							quad[2]->v = 1.0F;
							quad[3]->u = 1.0F;
							quad[3]->v = 1.0F;

							POLY_add_quad(quad, POLY_PAGE_SEWATER, TRUE);
						}
					}
				}
			}
		}
	}

	if (bbox_upto)
	{
		if (the_display.screen_lock())
		{
			//
			// Wibble the bounding boxes of the reflections.
			//

			for (i = 0; i < bbox_upto; i++)
			{
				WIBBLE_simple(
					bbox[i].x1,
					bbox[i].y1,
					bbox[i].x2,
					bbox[i].y2,
					62, 137, 17, 178, 40, 45);
			}

			the_display.screen_unlock();
		}
	}

	//
	// Draw the sewer water.
	//

/*
#if 0
	POLY_sort_sewater_page();
	POLY_frame_draw_sewater();
#else
	// Shouldn't need to render sewer stuff.
	ASSERT ( !(POLY_Page[POLY_PAGE_SEWATER].NeedsRendering()) );
#endif
*/

#ifndef TARGET_DC
	POLY_frame_init(TRUE,TRUE);
#endif

	//
	// Draw all the low res mapsquares.  None of the mapsquare
	// should be uncached.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		if (z == 0 || z == PAP_SIZE_LO - 1)
		{
			continue;
		}

		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			if (x == 0 || x == PAP_SIZE_LO - 1)
			{
				continue;
			}

			ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

			nl = &NS_lo[x][z];

			ASSERT(WITHIN(nl->cache, 1, NS_MAX_CACHES - 1));

			nc = &NS_cache[nl->cache];

			//
			// Draw the waterfalls...
			//

			{
				SLONG top;
				SLONG bot;
				SLONG fall;

				NS_Fall *nf;

				for (fall = nc->fall; fall; fall = nf->next)
				{
					ASSERT(WITHIN(fall, 1, NS_MAX_FALLS - 1));

					nf = &NS_fall[fall];

					top = (nf->top << 5) + (-32 * 0x100);
					bot = (nf->bot << 5) + (-32 * 0x100);

					SHAPE_waterfall(
						nf->x,
						nf->z,
						nf->dx,
						nf->dz,
						top,
						bot);

					//
					// Create dirt-drops...
					//

					sx = (nf->x << 8) + 0x80;
					sz = (nf->z << 8) + 0x80;

					dsx = nf->dx;
					dsz = nf->dz;

					dsx <<= 7;
					dsz <<= 7;

					x1 = sx + dsx;
					z1 = sz + dsz;

					x2 = sx + dsx;
					z2 = sz + dsz;

					x1 += -dsz;
					z1 += +dsx;

					x2 -= -dsz;
					z2 -= +dsx;

					dropdx = -nf->dx << 3;
					dropdz = -nf->dz << 3;
					dropdy =  0;

					for (i = 0; i < 4; i++)
					{
						along = rand() & 0xff;

						dropx = x1 + ((x2 - x1) * along >> 8);
						dropz = z1 + ((z2 - z1) * along >> 8);

						dropy = top;

						DIRT_new_water(
							dropx,
							dropy,
							dropz,
							dropdx,
							dropdy,
							dropdz);
					}
				}
			}

			base_x = float(x << PAP_SHIFT_LO);
			base_z = float(z << PAP_SHIFT_LO);

			//
			// Rotate all the points into the POLY_buffer.
			//

			POLY_buffer_upto = 0;
			np               = (NS_Point *) nc->memory;
			pp               = &POLY_buffer[0];

			for (i = 0; i < nc->num_points; i++)
			{
				px = float(np->x << 3) + base_x;
				pz = float(np->z << 3) + base_z;

				py = float(np->y << 5) + (32.0F * -256.0F);

				POLY_transform(
					px,
					py,
					pz,
					pp);

				if (pp->MaybeValid())
				{
					pp->colour   = (np->bright) | (np->bright << 8) | (np->bright << 16);
#ifdef TARGET_DC
					pp->colour |= 0xff000000;
#endif
					pp->specular = 0xff000000;

					if (bright)
					{
#ifdef TARGET_DC
						pp->colour = 0xffffffff;
#else
						pp->colour = 0x00ffffff;
#endif
					}

					POLY_fadeout_point(pp);
				}

				np++;
				pp++;
			}

			//
			// Create all the faces.
			//

			nf = (NS_Face *) np;

			for (i = 0; i < nc->num_faces; i++)
			{
				ASSERT(WITHIN(nf->p[0], 0, nc->num_points - 1));
				ASSERT(WITHIN(nf->p[1], 0, nc->num_points - 1));
				ASSERT(WITHIN(nf->p[2], 0, nc->num_points - 1));
				ASSERT(WITHIN(nf->p[3], 0, nc->num_points - 1));

				quad[0] = &POLY_buffer[nf->p[0]];
				quad[1] = &POLY_buffer[nf->p[1]];
				quad[2] = &POLY_buffer[nf->p[2]];
				quad[3] = &POLY_buffer[nf->p[3]];

				if (POLY_valid_quad(quad))
				{
					//
					// Texture the quad.
					//

					ASSERT(WITHIN(nf->texture, 0, NS_texture_upto - 1));

					nt = &NS_texture[nf->texture];

					for (j = 0; j < 4; j++)
					{
						quad[j]->u = float(nt->u[j]) * (1.0F / 32.0F);
						quad[j]->v = float(nt->v[j]) * (1.0F / 32.0F);
					}

					//
					// Find the texture page.
					//

					ASSERT(WITHIN(nf->page, 0, NS_PAGE_NUMBER - 1));

					//
					// And add it.
					//

					page = NS_page[nf->page].page;

					POLY_add_quad(quad, page, TRUE);
				}

				nf += 1;
			}


			//
			// The sewer mapwho.
			//

			for (i = nl->st; i; i = nst->next)
			{
				ASSERT(WITHIN(i, 1, NS_MAX_STS - 1));

				nst = &NS_st[i];

				switch(nst->type)
				{
					case NS_ST_TYPE_PRIM:

						MESH_draw_poly(
							nst->prim.prim,
							(x << PAP_SHIFT_LO) + (nst->prim.x << 3),
							(nst->prim.y << 5) + 0x100 * -32,
							(z << PAP_SHIFT_LO) + (nst->prim.z << 3),
							nst->prim.yaw << 3,
							0, 0,
							NULL,0xff,0);

						break;

					case NS_ST_TYPE_LADDER:

						FACET_draw_ns_ladder(
							nst->ladder.x1,
							nst->ladder.z1,
							nst->ladder.x2,
							nst->ladder.z2,
							nst->ladder.height);

						break;

					default:
						ASSERT(0);
						break;

				}
			}

			//
			// Look at the colvects on this square.
			//

			{
				SLONG f_list;
				SLONG facet;
				SLONG build;
				SLONG exit = FALSE;

				f_list = PAP_2LO(x,z).ColVectHead;

				show_gamut_lo(x,z);

				if (f_list)
				{
					while(!exit)
					{
						facet=facet_links[f_list];

						ASSERT(facet);

						if (facet < 0)
						{
							//
							// The last facet in the list for each square
							// is negative.
							//

							facet = -facet;
							exit  =  TRUE;
						}

						if (dfacets[facet].Counter[AENG_cur_fc_cam] != SUPERMAP_counter[AENG_cur_fc_cam])
						{
							if (dfacets[facet].FacetType == STOREY_TYPE_LADDER)
							{
								//
								// Draw the facet.
								//

								FACET_draw(facet,0);
							}

							//
							// Mark this facet as drawn this gameturn already.
							//

							dfacets[facet].Counter[AENG_cur_fc_cam] = SUPERMAP_counter[AENG_cur_fc_cam];
						}

						f_list++;
					}
				}
			}
		}
	}

	//
	// Draw the objects and the things.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			//
			// The game mapwho.
			//

			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (!(p_thing->Flags & FLAGS_IN_SEWERS))
				{
					//
					// Only draw things in the sewers.
					//
				}
				else
				{
					switch(p_thing->DrawType)
					{
						case DT_NONE:
							break;

						case DT_BUILDING:
							break;

						case DT_PRIM:
							break;

						case DT_ROT_MULTI:

							if (POLY_sphere_visible(
									float(p_thing->WorldPos.X >> 8),
									float(p_thing->WorldPos.Y >> 8) + KERB_HEIGHT,
									float(p_thing->WorldPos.Z >> 8),
									256.0F / (AENG_DRAW_DIST * 256.0F)))
							{
								FIGURE_draw(p_thing);
							}

							break;

						case DT_EFFECT:
							break;

						case DT_MESH:
							break;

						default:
							ASSERT(0);
							break;
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	//
	// The dirt.
	//

	AENG_draw_dirt();

	//
	// Draw the drips.
	//

//	AENG_draw_drips(0);

	//
	// Draw the polys.
	//

	POLY_frame_draw(TRUE,TRUE);

}


#endif


void AENG_draw_scanner(
		SLONG screen_x1,
		SLONG screen_y1,
		SLONG screen_x2,
		SLONG screen_y2,
		SLONG map_x,
		SLONG map_z,
		SLONG map_zoom,
		SLONG map_angle)
{
#ifdef	DOG_POO
	SLONG i;

	AZ_Line *al;

	float tx1;
	float tx2;
	float tz1;
	float tz2;

	float rx1;
	float rx2;
	float rz1;
	float rz2;

	float sx1;
	float sx2;
	float sy1;
	float sy2;

	float left   = float(screen_x1);
	float right  = float(screen_x2);
	float top    = float(screen_y1);
	float bottom = float(screen_y2);

	float screen_mid_x = float(screen_x1 + screen_x2 >> 1);
	float screen_mid_y = float(screen_y1 + screen_y2 >> 1);
	float angle        = float(map_angle) * (-2.0F * PI / 2048.0F);
	float zoom         = float(map_zoom)  * (1.0F / 65536.0F);

	float sin_yaw = sin(angle);
	float cos_yaw = cos(angle);
	float matrix[4];

	UBYTE clip1;
	UBYTE clip2;
	UBYTE clip_and;
	UBYTE clip_xor;

	matrix[0] =  cos_yaw;
	matrix[1] =  sin_yaw;
	matrix[2] = -sin_yaw;
	matrix[3] =  cos_yaw;

	//
	// Set the clipping rectangle.
	//

	POLY_clip_line_box(
		left,
		top,
		right,
		bottom);

	//
	// Initialise the frame.
	//

	POLY_frame_init(FALSE,FALSE);

	//
	// Add each line in turn.
	//

	ULONG type_colour[AZ_LINE_TYPE_NUMBER] =
	{
		0x00d83377,
		0x00eed811,
		0x0033ccff
	};

	for (i = 0; i < AZ_line_upto; i++)
	{
		al = &AZ_line[i];

		tx1 = float((al->x1 << ELE_SHIFT) - map_x);
		tz1 = float((al->z1 << ELE_SHIFT) - map_z);

		tx2 = float((al->x2 << ELE_SHIFT) - map_x);
		tz2 = float((al->z2 << ELE_SHIFT) - map_z);

		//
		// Rotate the points.
		//

		rx1 = tx1 * matrix[0] + tz1 * matrix[1];
		rz1 = tx1 * matrix[2] + tz1 * matrix[3];

		rx2 = tx2 * matrix[0] + tz2 * matrix[1];
		rz2 = tx2 * matrix[2] + tz2 * matrix[3];

		//
		// Their screen positions.
		//

		sx1 = screen_mid_x + zoom * rx1;
		sy1 = screen_mid_y + zoom * rz1;

		sx2 = screen_mid_x + zoom * rx2;
		sy2 = screen_mid_y + zoom * rz2;

		//
		// Add the line.
		//

		ASSERT(WITHIN(al->type, 0, AZ_LINE_TYPE_NUMBER - 1));

		POLY_clip_line_add(sx1, sy1, sx2, sy2, type_colour[al->type]);
	}

	//
	// Draw all the people.
	//

	{
		THING_INDEX t_index;
		Thing      *p_thing;

		SLONG dx;
		SLONG dz;

		for (t_index = the_game.UsedPrimaryThings; t_index; t_index = p_thing->LinkChild)
		{
			p_thing = TO_THING(t_index);

			if (p_thing->Class == CLASS_PERSON)
			{
				dx = -SIN(p_thing->Draw.Tweened->Angle) >> 9;
				dz = -COS(p_thing->Draw.Tweened->Angle) >> 9;

				tx1 = float((p_thing->WorldPos.X >> 8) - map_x);
				tz1 = float((p_thing->WorldPos.Z >> 8) - map_z);

				tx2 = float((p_thing->WorldPos.X >> 8) + dx - map_x);
				tz2 = float((p_thing->WorldPos.Z >> 8) + dz - map_z);

				//
				// Rotate the points.
				//

				rx1 = tx1 * matrix[0] + tz1 * matrix[1];
				rz1 = tx1 * matrix[2] + tz1 * matrix[3];

				rx2 = tx2 * matrix[0] + tz2 * matrix[1];
				rz2 = tx2 * matrix[2] + tz2 * matrix[3];

				//
				// Their screen positions.
				//

				sx1 = screen_mid_x + zoom * rx1;
				sy1 = screen_mid_y + zoom * rz1;

				sx2 = screen_mid_x + zoom * rx2;
				sy2 = screen_mid_y + zoom * rz2;

				//
				// Add the line.
				//

				POLY_clip_line_add(sx1, sy1, sx2, sy2, 0x00ffffff);
			}
		}
	}

	//
	// Draw a cross in the middle of the map.
	//

	#define AENG_CROSS_SIZE		5
	#define AENG_CROSS_COLOUR	0x0033aa33

	POLY_clip_line_add(
		screen_mid_x - AENG_CROSS_SIZE,
		screen_mid_y - AENG_CROSS_SIZE,
		screen_mid_x + AENG_CROSS_SIZE,
		screen_mid_y + AENG_CROSS_SIZE,
		AENG_CROSS_COLOUR);

	POLY_clip_line_add(
		screen_mid_x - AENG_CROSS_SIZE,
		screen_mid_y + AENG_CROSS_SIZE,
		screen_mid_x + AENG_CROSS_SIZE,
		screen_mid_y - AENG_CROSS_SIZE,
		AENG_CROSS_COLOUR);

	//
	// Draw an outline.
	//

	#define AENG_BORDER_COLOUR 0x003355cc

	POLY_add_line_2d(left,  top,    right, top,    AENG_BORDER_COLOUR);
	POLY_add_line_2d(right, top,    right, bottom, AENG_BORDER_COLOUR);
	POLY_add_line_2d(right, bottom, left,  bottom, AENG_BORDER_COLOUR);
	POLY_add_line_2d(left,  bottom, left,  top,    AENG_BORDER_COLOUR);

	//
	// The transparent background background.
	//

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	#define AENG_BACKGROUND_COLOUR 0x55888800

	pp[0].X        = left;
	pp[0].Y        = top;
	pp[0].z        = 0.0F;
	pp[0].Z        = 1.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = AENG_BACKGROUND_COLOUR;
	pp[0].specular = 0;

	pp[1].X        = right;
	pp[1].Y        = top;
	pp[1].z        = 0.0F;
	pp[1].Z        = 1.0F;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = AENG_BACKGROUND_COLOUR;
	pp[1].specular = 0;

	pp[2].X        = left;
	pp[2].Y        = bottom;
	pp[2].z        = 0.0F;
	pp[2].Z        = 1.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = AENG_BACKGROUND_COLOUR;
	pp[2].specular = 0;

	pp[3].X        = right;
	pp[3].Y        = bottom;
	pp[3].z        = 0.0F;
	pp[3].Z        = 1.0F;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[3].colour   = AENG_BACKGROUND_COLOUR;
	pp[3].specular = 0;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE, TRUE);

	#if WE_WANT_TO_DRAW_THE_TEXTURE_SHADOW_PAGE

	{
		left = 80;
		top  = 80;

		right  = 144;
		bottom = 144;

		#define USIZE (float(AENG_AA_BUF_SIZE) / float(TEXTURE_SHADOW_SIZE))

		pp[0].X        = left;
		pp[0].Y        = top;
		pp[0].z        = 0.0F;
		pp[0].Z        = 1.0F;
		pp[0].u        = 0.0F;
		pp[0].v        = 0.0F;
		pp[0].colour   = 0xffffffff;
		pp[0].specular = 0xff000000;

		pp[1].X        = right;
		pp[1].Y        = top;
		pp[1].z        = 0.0F;
		pp[1].Z        = 1.0F;
		pp[1].u        = USIZE;
		pp[1].v        = 0.0F;
		pp[1].colour   = 0xffffffff;
		pp[1].specular = 0xff000000;

		pp[2].X        = left;
		pp[2].Y        = bottom;
		pp[2].z        = 0.0F;
		pp[2].Z        = 1.0F;
		pp[2].u        = 0.0F;
		pp[2].v        = USIZE;
		pp[2].colour   = 0xffffffff;
		pp[2].specular = 0xff000000;

		pp[3].X        = right;
		pp[3].Y        = bottom;
		pp[3].z        = 0.0F;
		pp[3].Z        = 1.0F;
		pp[3].u        = USIZE;
		pp[3].v        = USIZE;
		pp[3].colour   = 0xffffffff;
		pp[3].specular = 0xff000000;

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		POLY_add_quad(quad, POLY_PAGE_SHADOW, FALSE, TRUE);
	}

	#endif

	//
	// Draw the polys.
	//

	POLY_frame_draw(TRUE,TRUE);
#endif
}

void AENG_draw_power(SLONG x,SLONG y,SLONG w,SLONG h,SLONG val,SLONG max)
{
	/*

	SLONG	left,right,top,bottom;
	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	left=x;
	right=left+w;
	top=y;
	bottom=y+h;

	//
	// Set the clipping rectangle.
	//
	POLY_clip_line_box(
		x,
		y,
		x+w,
		y+h);

	POLY_frame_init(FALSE,FALSE);

	//
	// Draw an outline.
	//

	#define AENG_PBORDER_COLOUR 0x003355cc
	#define AENG_PFOREGROUND_COLOUR 0x55888800

	POLY_add_line_2d(left,  top,    right, top,    AENG_PBORDER_COLOUR);
	POLY_add_line_2d(right, top,    right, bottom, AENG_PBORDER_COLOUR);
	POLY_add_line_2d(right, bottom, left,  bottom, AENG_PBORDER_COLOUR);
	POLY_add_line_2d(left,  bottom, left,  top,    AENG_PBORDER_COLOUR);

	right=left+(val*w)/max;

	pp[0].X        = left;
	pp[0].Y        = top;
	pp[0].z        = 0.0F;
	pp[0].Z        = 1.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[1].colour   = AENG_PFOREGROUND_COLOUR;
	pp[0].specular = 0;

	pp[1].X        = right;
	pp[1].Y        = top;
	pp[1].z        = 0.0F;
	pp[1].Z        = 1.0F;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = AENG_PFOREGROUND_COLOUR;
	pp[1].specular = 0;

	pp[2].X        = left;
	pp[2].Y        = bottom;
	pp[2].z        = 0.0F;
	pp[2].Z        = 1.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[1].colour   = AENG_PFOREGROUND_COLOUR;
	pp[2].specular = 0;

	pp[3].X        = right;
	pp[3].Y        = bottom;
	pp[3].z        = 0.0F;
	pp[3].Z        = 1.0F;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[1].colour   = AENG_PFOREGROUND_COLOUR;
	pp[3].specular = 0;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE, TRUE);

	POLY_frame_draw(TRUE,TRUE);

	*/
}

UBYTE	record_video=0;

#if defined(TARGET_DC)

// Chews memory - sod it.
void	AENG_screen_shot(void)
{
}

// time.h doesn't seem to exist in the DC stuff - wierd.
// Bin this function on the DC for now.
void AENG_draw_FPS()
{
}


#else //#if defined(TARGET_DC)

void	AENG_screen_shot(void)
{
	if(allow_debug_keys)
	if (Keys[KB_S] || record_video)
	{
		Keys[KB_S] = 0;
		if(ShiftFlag)
		{
			record_video^=1;
		}

		if (the_display.screen_lock())
		{
extern	void	tga_dump(void);
			tga_dump();

			//DumpBackToRaw();
			the_display.screen_unlock();
		}
	}
}


void AENG_draw_FPS()
{
	static	SLONG	fps = 0;			// current FPS
	static	SLONG	avfps = 0;			// average FPS
	static	SLONG	last_game_turn = 0;	// game turn when FPS was sampled
	static	clock_t	last_time = 0;		// time when FPS was sampled
	static	SLONG	total_frames = 0;
	static	float	total_time = 0;
	static	SLONG	ups = 0;			// us per frame
	static	SLONG	avups = 0;

	clock_t	this_time = clock();

	int	frames_passed = GAME_TURN - last_game_turn;

	if ((frames_passed < 0) || (frames_passed > 200))
	{
		// reset
		fps = 0;
		last_game_turn = GAME_TURN;
		last_time = this_time;
	}
	else
	{
		if (frames_passed >= 20)
		{
			float	seconds = float(this_time - last_time) / float(CLOCKS_PER_SEC);
			float	_fps = float(frames_passed) / seconds;
			fps = floor(_fps + 0.5);
			float _ups = 1000000 / _fps;
			ups = floor(_ups + 0.5);

			if (fps > 10)	// don't include really slow frames in the average
			{
				total_frames += frames_passed;
				total_time += seconds;
				float	_avfps = float(total_frames) / total_time;
				avfps = floor(_avfps + 0.5);
				float	_avups = 1000000 / _avfps;
				avups = floor(_avups + 0.5);
			}

			last_time = this_time;
			last_game_turn = GAME_TURN;

		}
	}
/*
	if (the_display.screen_lock())
	{
		FONT_draw(DisplayWidth >> 1, 10, "FPS: %d = %d us", fps, ups);
		FONT_draw(DisplayWidth >> 1, 30, "Avg: %d = %d us", avfps, avups);
		the_display.screen_unlock();
	}
*/

	if (allow_debug_keys)
	{
		CBYTE	str[100];

		sprintf(str,"FPS: %d = %d us",fps,ups);
		FONT2D_DrawString(str,DisplayWidth >> 1, 10,0xffffff,128);
		sprintf(str,"Avg: %d = %d us",avfps,avups);
		FONT2D_DrawString(str,DisplayWidth >> 1, 30,0xffffff,128);
	}

}

#endif //#else //#if defined(TARGET_DC)

void AENG_draw_messages()
{
	//
	// fps stuff.
	//

	static SLONG   fps = 0;
#if !defined(TARGET_DC)
	static SLONG   last_game_turn = 0;
	static clock_t last_time = 0;
		   clock_t this_time = 0;

	this_time = clock() / CLOCKS_PER_SEC;

	if (this_time != last_time)
	{
		if (this_time - last_time > 1)
		{
			//
			// Only work out the new frames per second if there hasn't been a major
			// delay.
			//
		}
		else
		{
			fps = GAME_TURN - last_game_turn;
		}

		last_time      = this_time;
		last_game_turn = GAME_TURN;
	}
#else
	fps = 0;
#endif

	#if ARGH

	static SLONG px[3] = {4 << 16,  8 << 16, 8 << 16};
	static SLONG py[3] = {4 << 16,  4 << 16, 8 << 16};

	if (LeftButton)
	{
		SLONG mx;
		SLONG my;

		SLONG dx;
		SLONG dy;

		mx = MouseX - AENG_AA_LEFT;
		my = MouseY - AENG_AA_TOP;

		mx <<= 16;
		my <<= 16;

		mx /= AENG_AA_PIX_SIZE;
		my /= AENG_AA_PIX_SIZE;

		SLONG i;

		SLONG  dist;
		SLONG  best_dist = INFINITY;
		SLONG *best_x;
		SLONG *best_y;

		for (i = 0; i < 3; i++)
		{
			dx = abs(px[i] - mx);
			dy = abs(py[i] - my);

			dist = dx + dy;

			if (dist < best_dist)
			{
				best_dist =  dist;
				best_x    = &px[i];
				best_y    = &py[i];
			}
		}

		*best_x = mx;
		*best_y = my;
	}

	{
		//
		// Draw a mad quad!
		//

		memset(AENG_aa_buffer, 0, sizeof(AENG_aa_buffer));

		AA_draw(
			(UBYTE *) AENG_aa_buffer,
			AENG_AA_BUF_SIZE,
			AENG_AA_BUF_SIZE,
			AENG_AA_BUF_SIZE,
			px[0], py[0],
			px[1], py[1],
			px[2], py[2]);
	}

	#endif

	//
	// Draw stuff straight to the screen.
	//

	if (the_display.screen_lock())
	{
/*
		if (Keys[KB_S])
		{
			Keys[KB_S] = 0;

			//
			// Take a screen shot.
			//

			DumpBackToRaw();
		}
*/


		//
		// Draw the fps.
		//
		FONT_draw(DisplayWidth >> 1, 10, "FPS: %d", fps);
/*

		//
		// Number of facets drawn.
		//

		FONT_draw(DisplayWidth >> 1, 20, "Facets: %d", dfacets_drawn_this_gameturn);
extern	SLONG	damp;
		FONT_draw(20,30, "DAMP: %d", damp);
		*/

		//
		// Draw the messages.
		//

		//MSG_draw();
		#if werrr

		SLONG x;
		SLONG y;

		SLONG dx;
		SLONG dy;

		for (x = 0; x < AENG_AA_BUF_SIZE; x++)
		for (y = 0; y < AENG_AA_BUF_SIZE; y++)
		{
			for (dx = 0; dx < AENG_AA_PIX_SIZE; dx++)
			for (dy = 0; dy < AENG_AA_PIX_SIZE; dy++)
			{
				the_display.PlotPixel(
					AENG_AA_LEFT + x * AENG_AA_PIX_SIZE + dx,
					AENG_AA_TOP	 + y * AENG_AA_PIX_SIZE + dy,
					AENG_aa_buffer[y][x],
					AENG_aa_buffer[y][x],
					AENG_aa_buffer[y][x]);
			}
		}

		for (SLONG i = 0; i < 3; i++)
		{
			x = AENG_AA_LEFT + (px[i] * AENG_AA_PIX_SIZE >> 16);
			y = AENG_AA_TOP	 + (py[i] * AENG_AA_PIX_SIZE >> 16);

			the_display.PlotPixel(x + 0, y + 0, 255, 255, 0);
			the_display.PlotPixel(x + 1, y + 0, 255, 100, 0);
			the_display.PlotPixel(x + 0, y + 1, 255, 100, 0);
			the_display.PlotPixel(x - 1, y + 0, 255, 100, 0);
			the_display.PlotPixel(x + 0, y - 1, 255, 100, 0);
		}

		#endif

		the_display.screen_unlock();
	}
}

void AENG_fade_out(UBYTE amount)
{
	SLONG logo_fade_top = amount;
	SLONG back_fade_top = amount;

	ULONG logo_colour_top = (logo_fade_top << 24) | 0x00ffffff;
	ULONG back_colour_top = (back_fade_top << 24) | 0x00ffffff;

	SLONG logo_fade_bot = amount;
	SLONG back_fade_bot = amount;

	ULONG logo_colour_bot = (logo_fade_bot << 24) | 0x00ffffff;
	ULONG back_colour_bot = (back_fade_bot << 24) | 0x00ffffff;

	//
	// Draw the logo.
	//

	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	#define AENG_LOGO_MID_X (320.0F)
	#define AENG_LOGO_MID_Y (240.0F)
	#define AENG_LOGO_SIZE  (128.0F)

#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif

	pp[0].X        = AENG_LOGO_MID_X - AENG_LOGO_SIZE;
	pp[0].Y        = AENG_LOGO_MID_Y - AENG_LOGO_SIZE;
	pp[0].z        = 1.0F / 65536.0F;
	pp[0].Z        = 1.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = logo_colour_top;
	pp[0].specular = 0x00000000;

	pp[1].X        = AENG_LOGO_MID_X + AENG_LOGO_SIZE;
	pp[1].Y        = AENG_LOGO_MID_Y - AENG_LOGO_SIZE;
	pp[1].z        = 1.0F / 65536.0F;
	pp[1].Z        = 1.0F;
	pp[1].u        = 1.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = logo_colour_top;
	pp[1].specular = 0x00000000;

	pp[2].X        = AENG_LOGO_MID_X - AENG_LOGO_SIZE;
	pp[2].Y        = AENG_LOGO_MID_Y + AENG_LOGO_SIZE;
	pp[2].z        = 1.0F / 65536.0F;
	pp[2].Z        = 1.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 1.0F;
	pp[2].colour   = logo_colour_bot;
	pp[2].specular = 0x00000000;

	pp[3].X        = AENG_LOGO_MID_X + AENG_LOGO_SIZE;
	pp[3].Y        = AENG_LOGO_MID_Y + AENG_LOGO_SIZE;
	pp[3].z        = 1.0F / 65536.0F;
	pp[3].Z        = 1.0F;
	pp[3].u        = 1.0F;
	pp[3].v        = 1.0F;
	pp[3].colour   = logo_colour_bot;
	pp[3].specular = 0x00000000;

	POLY_add_quad(quad, POLY_PAGE_LOGO, FALSE, TRUE);

#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
	POLY_frame_init(FALSE,FALSE);
#endif

	pp[0].X        = 0.0F;
	pp[0].Y        = 0.0F;
	pp[0].z        = 2.0F     / 65536.0F;
	pp[0].Z        = 65535.0F / 65536.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = back_colour_top;
	pp[0].specular = 0x00000000;

	pp[1].X        = 640.0F;
	pp[1].Y        = 0.0F;
	pp[1].z        = 2.0F     / 65536.0F;
	pp[1].Z        = 65535.0F / 65536.0F;
	pp[1].u        = 1.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = back_colour_top;
	pp[1].specular = 0x00000000;

	pp[2].X        = 0.0F;
	pp[2].Y        = 480.0F;
	pp[2].z        = 2.0F     / 65536.0F;
	pp[2].Z        = 65535.0F / 65536.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 1.0F;
	pp[2].colour   = back_colour_bot;
	pp[2].specular = 0x00000000;

	pp[3].X        = 640.0F;
	pp[3].Y        = 480.0F;
	pp[3].z        = 2.0F     / 65536.0F;
	pp[3].Z        = 65535.0F / 65536.0F;
	pp[3].u        = 1.0F;
	pp[3].v        = 1.0F;
	pp[3].colour   = back_colour_bot;
	pp[3].specular = 0x00000000;

	POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE, TRUE);

#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
#endif
}


void AENG_fade_in(UBYTE amount)
{
	SLONG logo_fade_top = 255 - amount;
	SLONG back_fade_top = 255 - (amount * amount >> 8);

	ULONG logo_colour_top = (logo_fade_top << 24);
	ULONG back_colour_top = (back_fade_top << 24);

	SLONG logo_fade_bot = 255 - amount;
	SLONG back_fade_bot = 255 - (amount * amount >> 8);

	ULONG logo_colour_bot = (logo_fade_bot << 24);
	ULONG back_colour_bot = (back_fade_bot << 24);

	//
	// Draw the logo.
	//

	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

#ifndef TARGET_DC
	POLY_frame_init(TRUE,TRUE);
#endif

	pp[0].X        = AENG_LOGO_MID_X - AENG_LOGO_SIZE;
	pp[0].Y        = AENG_LOGO_MID_Y - AENG_LOGO_SIZE;
	pp[0].z        = 1.0F / 65536.0F;
	pp[0].Z        = 1.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = logo_colour_top;
	pp[0].specular = 0xff000000;

	pp[1].X        = AENG_LOGO_MID_X + AENG_LOGO_SIZE;
	pp[1].Y        = AENG_LOGO_MID_Y - AENG_LOGO_SIZE;
	pp[1].z        = 1.0F / 65536.0F;
	pp[1].Z        = 1.0F;
	pp[1].u        = 1.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = logo_colour_top;
	pp[1].specular = 0xff000000;

	pp[2].X        = AENG_LOGO_MID_X - AENG_LOGO_SIZE;
	pp[2].Y        = AENG_LOGO_MID_Y + AENG_LOGO_SIZE;
	pp[2].z        = 1.0F / 65536.0F;
	pp[2].Z        = 1.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 1.0F;
	pp[2].colour   = logo_colour_bot;
	pp[2].specular = 0xff000000;

	pp[3].X        = AENG_LOGO_MID_X + AENG_LOGO_SIZE;
	pp[3].Y        = AENG_LOGO_MID_Y + AENG_LOGO_SIZE;
	pp[3].z        = 1.0F / 65536.0F;
	pp[3].Z        = 1.0F;
	pp[3].u        = 1.0F;
	pp[3].v        = 1.0F;
	pp[3].colour   = logo_colour_bot;
	pp[3].specular = 0xff000000;

	POLY_add_quad(quad, POLY_PAGE_LOGO, FALSE, TRUE);

#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
#endif

	pp[0].X        = 0.0F;
	pp[0].Y        = 0.0F;
	pp[0].z        = 2.0F     / 65536.0F;
	pp[0].Z        = 65535.0F / 65536.0F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = back_colour_top;
	pp[0].specular = 0xff000000;

	pp[1].X        = 640.0F;
	pp[1].Y        = 0.0F;
	pp[1].z        = 2.0F     / 65536.0F;
	pp[1].Z        = 65535.0F / 65536.0F;
	pp[1].u        = 1.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = back_colour_top;
	pp[1].specular = 0xff000000;

	pp[2].X        = 0.0F;
	pp[2].Y        = 480.0F;
	pp[2].z        = 2.0F     / 65536.0F;
	pp[2].Z        = 65535.0F / 65536.0F;
	pp[2].u        = 0.0F;
	pp[2].v        = 1.0F;
	pp[2].colour   = back_colour_bot;
	pp[2].specular = 0xff000000;

	pp[3].X        = 640.0F;
	pp[3].Y        = 480.0F;
	pp[3].z        = 2.0F     / 65536.0F;
	pp[3].Z        = 65535.0F / 65536.0F;
	pp[3].u        = 1.0F;
	pp[3].v        = 1.0F;
	pp[3].colour   = back_colour_bot;
	pp[3].specular = 0xff000000;

	POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE, TRUE);

#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
	POLY_frame_init(FALSE,FALSE);
#endif
}


void AENG_clear_screen()
{
#ifdef TARGET_DC
	static int iBlah = 0;
	iBlah++;
	iBlah = 0;
	if ( ( iBlah & 0x3 ) == 0 )
	{
		SET_BLACK_BACKGROUND;
	}
	else if ( ( iBlah & 0x3 ) == 1 )
	{
		SET_WHITE_BACKGROUND;
	}
	else if ( ( iBlah & 0x3 ) == 2 )
	{
		SET_BLACK_BACKGROUND;
	}
	else if ( ( iBlah & 0x3 ) == 3 )
	{
		SET_WHITE_BACKGROUND;
	}
#else
	SET_BLACK_BACKGROUND;
#endif
	CLEAR_VIEWPORT;
	TheVPool->ReclaimBuffers();
}

SLONG AENG_lock()
{
	return SLONG(the_display.screen_lock());
}

void AENG_unlock()
{
	the_display.screen_unlock();
}

void AENG_flip()
{
#ifndef TARGET_DC
	if (sw_hack)
	{
		SW_copy_to_bb();
	}
#endif

	FLIP(NULL, DDFLIP_WAIT);	// PerMedia2 needs this, or else!
}

void AENG_blit()
{
#ifndef TARGET_DC
	if (sw_hack)
	{
		SW_copy_to_bb();
	}
#endif

	the_display.blit_back_buffer();
}


#ifndef TARGET_DC
void AENG_e_draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	AENG_world_line(
		x1,y1,z1,8,0x00ffffff,
		x2,y2,z2,8,0x00ffffff,
		TRUE);
}

void AENG_e_draw_3d_line_dir(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	AENG_world_line(
		x1,y1,z1,32,0x00ffffff,
		x2,y2,z2, 0,0x00553311,
		TRUE);
}

void AENG_e_draw_3d_line_col(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b)
{
	ULONG colour;

	colour  = r << 16;
	colour |= g << 8;
	colour |= b << 0;

	AENG_world_line(
		x1,y1,z1,8,colour,
		x2,y2,z2,8,colour,
		TRUE);
}

void AENG_e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b)
{
	ULONG colour;

	colour  = r << 16;
	colour |= g << 8;
	colour |= b << 0;

	AENG_world_line(
		x1,y1,z1,8,colour,
		x2,y2,z2,8,colour,
		FALSE);
}

void AENG_e_draw_3d_mapwho(SLONG x1, SLONG z1)
{
	x1 <<= ELE_SHIFT;
	z1 <<= ELE_SHIFT;

	e_draw_3d_line(x1,0,z1,x1+256,0,z1);
	e_draw_3d_line(x1+256,0,z1,x1+256,0,z1+256);
	e_draw_3d_line(x1+256,0,z1+256,x1,0,z1+256);
	e_draw_3d_line(x1,0,z1+256,x1,0,z1);
}

void AENG_e_draw_3d_mapwho_y(SLONG x1, SLONG y1, SLONG z1)
{
	x1 <<= ELE_SHIFT;
	z1 <<= ELE_SHIFT;

	e_draw_3d_line(x1,y1,z1,x1+256,y1,z1);
	e_draw_3d_line(x1+256,y1,z1,x1+256,y1,z1+256);
	e_draw_3d_line(x1+256,y1,z1+256,x1,y1,z1+256);
	e_draw_3d_line(x1,y1,z1+256,x1,y1,z1);
}
#endif //#ifndef TARGET_DC


//---------------------------------------------------------------

void	AENG_demo_attract(SLONG x,SLONG y,CBYTE *text)
{
	/*

	static flash = 0;

	POLY_frame_init(FALSE,FALSE);


	text_fudge = TRUE;


	draw_centre_text_at(x,y,text,1,1);

	//if (flash++ & 0x10)	Do it all the time!
	{
		text_fudge  = FALSE;
		text_colour = 0x00eeeeff;

		draw_centre_text_at(
			320, 30,
			"Press any button to play demo\n",
			0,0);
	}

	POLY_frame_draw(FALSE,TRUE);

	*/
}

//---------------------------------------------------------------


#ifndef TARGET_DC
// ========================================================
//
// EDITOR SUPPORT FUNCTIONS.
//
// ========================================================

SLONG AENG_raytraced_position(
		SLONG  sx,
		SLONG  sy,
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG indoors)
{
	SLONG i;

	float ax;
	float ay;

	float ex;
	float ey;
	float ez;

	float dx;
	float dy;
	float dz;
	float len;

	SLONG wx;
	SLONG wy;
	SLONG wz;

	float rx = AENG_cam_x;
	float ry = AENG_cam_y;
	float rz = AENG_cam_z;

	//
	// Use to cone to find out the direction of the vector through (sx,sy).
	//

	ax = float(sx) * (1.0F / 640.0F);
	ay = float(sy) * (1.0F / 480.0F);

	ex  = AENG_cone[1].x;
	ey  = AENG_cone[1].y;
	ez  = AENG_cone[1].z;

	ex += ax * (AENG_cone[0].x - AENG_cone[1].x);
	ey += ax * (AENG_cone[0].y - AENG_cone[1].y);
	ez += ax * (AENG_cone[0].z - AENG_cone[1].z);

	ex += ay * (AENG_cone[2].x - AENG_cone[1].x);
	ey += ay * (AENG_cone[2].y - AENG_cone[1].y);
	ez += ay * (AENG_cone[2].z - AENG_cone[1].z);

	//
	// This is all in mapsquare coordinates, not world coordinates.
	//

	ex *= 256.0F;
	ey *= 256.0F;
	ez *= 256.0F;

	//
	// The direction of our ray.
	//

	dx = ex - rx;
	dy = ey - ry;
	dz = ez - rz;

	len = sqrt(dx*dx + dy*dy + dz*dz);

	#define AENG_RAYTRACE_ACCURACY 16

	dx *= (256.0F / AENG_RAYTRACE_ACCURACY) / len;
	dy *= (256.0F / AENG_RAYTRACE_ACCURACY) / len;
	dz *= (256.0F / AENG_RAYTRACE_ACCURACY) / len;

	if (GAME_FLAGS & GF_SEWERS)
	{
		//
		// Use the sewer LOS function.
		//

		SLONG x1 = SLONG(rx);
		SLONG y1 = SLONG(ry);
		SLONG z1 = SLONG(rz);

		SLONG x2 = SLONG(rx + dx * (AENG_RAYTRACE_ACCURACY * 16));
		SLONG y2 = SLONG(ry + dy * (AENG_RAYTRACE_ACCURACY * 16));
		SLONG z2 = SLONG(rz + dz * (AENG_RAYTRACE_ACCURACY * 16));

		if (NS_there_is_a_los(
				x1, y1, z1,
				x2, y2, z2))
		{
			return FALSE;
		}

	   *world_x = NS_los_fail_x;
	   *world_y = NS_los_fail_y;
	   *world_z = NS_los_fail_z;

		if (!WITHIN(*world_x, 0, (PAP_SIZE_HI << 8) - 1) ||
			!WITHIN(*world_z, 0, (PAP_SIZE_HI << 8) - 1))
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	//
	// Intersect the ray with the world.
	//

	rx += dx * AENG_RAYTRACE_ACCURACY;
	ry += dy * AENG_RAYTRACE_ACCURACY;
	rz += dz * AENG_RAYTRACE_ACCURACY;

	for (i = 0; i < AENG_RAYTRACE_ACCURACY * (AENG_DRAW_DIST - 1); i++)
	{
		wx = (SLONG) rx;
		wy = (SLONG) ry;
		wz = (SLONG) rz;

		if (
			(indoors&&(wy<get_inside_alt(indoors))) ||
			((!indoors)&&(wy < PAP_calc_map_height_at(wx, wz)))
		   )
		{
			if (indoors)
			  wy = get_inside_alt(indoors);
			else
			  wy = PAP_calc_map_height_at(wx, wz);

			*world_x = wx;
			*world_y = wy;
			*world_z = wz;

			if (!WITHIN(*world_x, 0, (PAP_SIZE_HI << 8) - 1) ||
				!WITHIN(*world_z, 0, (PAP_SIZE_HI << 8) - 1))
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}

		rx += dx;
		ry += dy;
		rz += dz;
	}

	return FALSE;
}


ULONG AENG_light_draw(
		SLONG mx,
		SLONG my,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight)
{
	ULONG ans = 0;

	SLONG h1 = PAP_calc_map_height_at(lx, lz);
	SLONG h2 = ly;

#ifndef TARGET_DC
	POLY_frame_init(FALSE, FALSE);
#endif

	//
	// Draw a couple of sphere connected by a line.
	//

	SHAPE_alpha_sphere(
		lx,
		h1,
		lz,
		30,
		0x00ffffff,
		0xff000000);

	SHAPE_alpha_sphere(
		lx,
		h2,
		lz,
		30 + (highlight >> 4),
		colour,0xFF000000);

	AENG_world_line(
		lx, h1, lz, 32, 0xffffff,
		lx, h2, lz,  0, colour,
		FALSE);

#ifndef TARGET_DC
	POLY_frame_draw(FALSE, FALSE);
#endif

	//
	// Was either over the mouse?
	//

	SLONG dx;
	SLONG dy;
	SLONG dist;

	SLONG sx;
	SLONG sy;
	SLONG swidth;

	if (POLY_get_sphere_circle(
			float(lx),
			float(h1),
			float(lz),
			30.0F,
		   &sx,
		   &sy,
		   &swidth))
	{
		dx = abs(sx - mx);
		dy = abs(sy - my);

		dist = QDIST2(dx,dy);

		if (dist < swidth * 3)
		{
			ans |= AENG_MOUSE_OVER_LIGHT_BOT;
		}
	}

	if (POLY_get_sphere_circle(
			float(lx),
			float(h2),
			float(lz),
			30.0F,
		   &sx,
		   &sy,
		   &swidth))
	{
		dx = abs(sx - mx);
		dy = abs(sy - my);

		dist = QDIST2(dx,dy);

		if (dist < swidth * 3)
		{
			ans |= AENG_MOUSE_OVER_LIGHT_TOP;
		}
	}

	return ans;
}


#ifdef SEWERS

void AENG_draw_sewer_editor(
		SLONG  cam_x,
		SLONG  cam_y,
		SLONG  cam_z,
		SLONG  cam_yaw,
		SLONG  cam_pitch,
		SLONG  cam_roll,
		SLONG  mouse_x,
		SLONG  mouse_y,
		SLONG *mouse_over_valid,
		SLONG *mouse_over_x,
		SLONG *mouse_over_y,
		SLONG *mouse_over_z,
		SLONG  draw_prim_at_mouse,
		SLONG  prim_object,
		SLONG  prim_yaw)
{
	SLONG i;

	SLONG x;
	SLONG z;

	float px;
	float py;
	float pz;

	float wy;

	SLONG height;
	SLONG page;

	float along_01;
	float along_02;

	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	pp[0].colour = 0x00ffffff;
	pp[1].colour = 0x00ffffff;
	pp[2].colour = 0x00ffffff;
	pp[3].colour = 0x00ffffff;

	pp[0].specular = 0xff000000;
	pp[1].specular = 0xff000000;
	pp[2].specular = 0xff000000;
	pp[3].specular = 0xff000000;

	pp[0].u = 0.0F;
	pp[0].v = 0.0F;
	pp[1].u = 1.0F;
	pp[1].v = 0.0F;
	pp[2].u = 0.0F;
	pp[2].v = 1.0F;
	pp[3].u = 1.0F;
	pp[3].v = 1.0F;

	ES_Hi *eh;
	ES_Lo *el;

	ES_Thing *et;

	//
	// Clear screen.
	//

	AENG_clear_screen();

	//
	// Clear out stuff.
	//

	POLY_frame_init(FALSE,FALSE);

	//
	// Set the camera.
	//

	POLY_camera_set(
		float(cam_x),
		float(cam_y),
		float(cam_z),
		float(cam_yaw)   * 2.0F * PI / 2048.0F,
		float(cam_pitch) * 2.0F * PI / 2048.0F,
		float(cam_roll)  * 2.0F * PI / 2048.0F,
		float(AENG_DRAW_DIST) * 256.0F,
		AENG_LENS);

	//
	// Calculate the gamut.
	//

	AENG_calc_gamut(
		float(cam_x),
		float(cam_y),
		float(cam_z),
		float(cam_yaw)   * 2.0F * PI / 2048.0F,
		float(cam_pitch) * 2.0F * PI / 2048.0F,
		float(cam_roll)  * 2.0F * PI / 2048.0F,
		float(AENG_DRAW_DIST),
		AENG_LENS);

   *mouse_over_valid = FALSE;

	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{
		for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, PAP_SIZE_HI - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_HI - 1));

			eh = &ES_hi[x][z];

			switch(eh->type)
			{
				case ES_TYPE_ROCK:   page = NS_page[NS_PAGE_ROCK ].page; break;
				case ES_TYPE_SEWER:	 page = NS_page[NS_PAGE_SWALL].page; break;
				case ES_TYPE_GROUND: page = NS_page[NS_PAGE_STONE].page; break;
				case ES_TYPE_HOLE:   page = 0;					    	 break;
				default:
					ASSERT(0);
					break;
			}

			if (eh->flag & ES_FLAG_GRATING)
			{
				page = NS_page[NS_PAGE_GRATE].page;
			}

			//
			// The height.
			//

			py = float((eh->height << 5) + -32 * 0x100);

			//
			// Create the four points.
			//

			for (i = 0; i < 4; i++)
			{
				px = float(x + ((i & 1) ? 1 : 0)) * 256.0F;
				pz = float(z + ((i & 2) ? 1 : 0)) * 256.0F;

				POLY_transform(
					px,
					py,
					pz,
				   &pp[i]);

				if (!pp[i].MaybeValid())
				{
					goto abandon_this_square;
				}
			}

			if (POLY_valid_quad(quad))
			{
				POLY_add_quad(quad, page, FALSE);

				//
				// Is the mouse in this quad?
				//

				if (POLY_inside_quad(
						float(mouse_x),
						float(mouse_y),
						quad,
					   &along_01,
					   &along_02))
				{
					*mouse_over_valid = TRUE;

					*mouse_over_x = x * 256 + 0x80;
					*mouse_over_y = SLONG(py);
					*mouse_over_z = z * 256 + 0x80;
				}
			}

		  abandon_this_square:;

			//
			// Draw water above this square?
			//

			if (eh->water)
			{
				//
				// The height.
				//

				wy = float((eh->water << 5) + -32 * 0x100);

				//
				// Create the four points.
				//

				for (i = 0; i < 4; i++)
				{
					px = float(x + ((i & 1) ? 1 : 0)) * 256.0F;
					pz = float(z + ((i & 2) ? 1 : 0)) * 256.0F;

					POLY_transform(
						px,
						wy,
						pz,
					   &pp[i]);

					if (!pp[i].MaybeValid())
					{
						goto abandon_this_water;
					}
				}

				pp[0].colour = 0x00222266;
				pp[1].colour = 0x00222266;
				pp[2].colour = 0x00222266;
				pp[3].colour = 0x00222266;

				if (POLY_valid_quad(quad))
				{
					POLY_add_quad(quad, POLY_PAGE_ADDITIVE, FALSE);
				}

				pp[0].colour = 0x00ffffff;
				pp[1].colour = 0x00ffffff;
				pp[2].colour = 0x00ffffff;
				pp[3].colour = 0x00ffffff;

			  abandon_this_water:;

			}

			if (eh->flag & ES_FLAG_ENTRANCE)
			{
				SLONG mx = (x << 8) + 0x80;
				SLONG mz = (z << 8) + 0x80;

				SLONG colourbot;
				SLONG colourtop;

				if (eh->flag & ES_FLAG_NOCURBS)
				{
					colourbot = 0x0000ff00;
					colourtop = 0x000000ff;
				}
				else
				{
					colourbot = 0x00ff0000;
					colourtop = 0x00ffff88;
				}

				AENG_world_line(
					mx, SLONG(py) + 0x010, mz, 32, colourbot,
					mx, SLONG(py) + 0x280, mz,  0, colourtop,
					FALSE);
			}
		}
	}

	//
	// Draw the lights.
	//

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(z, 0, PAP_SIZE_LO - 1));

			el = &ES_lo[x][z];

			if (el->light_y)
			{
				SLONG lx = (x << PAP_SHIFT_LO) + (el->light_x << 3);
				SLONG ly = (el->light_y << 5) + -32 * 0x100;
				SLONG lz = (z << PAP_SHIFT_LO) + (el->light_z << 3);

				SHAPE_sphere(
					lx, ly, lz,
					32,
					0x00ddddff);
			}
		}
	}

	//
	// Draw the things.
	//

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		switch(et->type)
		{
			case ES_THING_TYPE_UNUSED:
				break;

			case ES_THING_TYPE_LADDER:

				FACET_draw_ns_ladder(
					et->x1,
					et->z1,
					et->x2,
					et->z2,
					et->height);

				break;

			case ES_THING_TYPE_PRIM:

				MESH_draw_poly(
					et->prim,
					et->x,
					et->y,
					et->z,
					et->yaw, 0, 0,
					NULL,0xff,0);

				break;

			default:
				ASSERT(0);
				break;
		}
	}

	if (*mouse_over_valid)
	{
		//
		// Highlight the square the mouse is over.
		//

		AENG_e_draw_3d_mapwho_y(
			*mouse_over_x >> 8,
			*mouse_over_y,
			*mouse_over_z >> 8);

		if (draw_prim_at_mouse)
		{
			MESH_draw_poly(
				prim_object,
				*mouse_over_x & ~0xff,
				*mouse_over_y,
				*mouse_over_z & ~0xff,
				prim_yaw, 0, 0,
				NULL,0xff,0);
		}
	}

	POLY_frame_draw(TRUE,TRUE);

	return;
}

#endif

#endif //#ifndef TARGET_DC


//
// Draws text at the given point.
//

void AENG_world_text(
		SLONG  x,
		SLONG  y,
		SLONG  z,
		UBYTE  red,
		UBYTE  blue,
		UBYTE  green,
		UBYTE  shadowed_or_not,
		CBYTE *fmt, ...)
{
	POLY_Point pp;

//	return;

	POLY_transform(
		float(x),
		float(y),
		float(z),
	   &pp);

	if (pp.IsValid())
	{
		//
		// Work out the real message.
		//

		CBYTE   message[FONT_MAX_LENGTH];
		va_list	ap;

		va_start(ap, fmt);
		vsprintf(message, fmt, ap);
		va_end  (ap);

		//
		// Add the message.
		//

		FONT_buffer_add(
			pp.X,
			pp.Y,
			red,
			green,
			blue,
			shadowed_or_not,
			message);
	}
}






#ifndef TARGET_DC
//---------------------------------------------------------------
//	GUY.
//---------------------------------------------------------------

ULONG AENG_waypoint_draw(
		SLONG mx,
		SLONG my,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight)
{
	ULONG ans = 0;

//	SLONG h1 = PAP_calc_map_height_at(lx, lz);
	SLONG h2 = ly;

	POLY_frame_init(FALSE, FALSE);

	//
	// Draw a single sphere.
	//

	if (INDOORS_INDEX)
	SHAPE_alpha_sphere(
		lx,
		h2,
		lz,
		30 + (highlight >> 4),
		colour,
		0xff000000);
	else
	SHAPE_sphere(
		lx,
		h2,
		lz,
		30 + (highlight >> 4),
		colour);

	POLY_frame_draw(FALSE, FALSE);

	//
	// Was it over the mouse?
	//

	SLONG dx;
	SLONG dy;
	SLONG dist;

	SLONG sx;
	SLONG sy;
	SLONG swidth;

	if (POLY_get_sphere_circle(
			float(lx),
			float(h2),
			float(lz),
			(float)(30 + (highlight >> 4)),
		   &sx,
		   &sy,
		   &swidth))
	{
		dx = abs(sx - mx);
		dy = abs(sy - my);

		dist = QDIST2(dx,dy);

		if (dist < swidth * 2)
		{
			ans |= AENG_MOUSE_OVER_WAYPOINT;
		}
	}

	return ans;
}

//---------------------------------------------------------------

ULONG AENG_rad_trigger_draw(
		SLONG mx,
		SLONG my,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG rad,
		ULONG colour,
		UBYTE highlight)
{
	ULONG ans = 0;

	SLONG h1 = PAP_calc_map_height_at(lx, lz);
	SLONG h2 = ly;

	POLY_frame_init(FALSE, FALSE);

	//
	// Draw a single sphere.
	//
/*
	SHAPE_sphere(
		lx,
		h1,
		lz,
		30 + (highlight >> 4),
		colour);
*/

	SHAPE_alpha_sphere(
		lx,
		h2,
		lz,
		rad,
		colour,
		0x88000000);

	POLY_frame_draw(FALSE, FALSE);

	//
	// Was it over the mouse?
	//

	SLONG dx;
	SLONG dy;
	SLONG dist;

	SLONG sx;
	SLONG sy;
	SLONG swidth;

	if (POLY_get_sphere_circle(
			float(lx),
			float(h1),
			float(lz),
			(float)(30 + (highlight >> 4)),
		   &sx,
		   &sy,
		   &swidth))
	{
		dx = abs(sx - mx);
		dy = abs(sy - my);

		dist = QDIST2(dx,dy);

		if (dist < swidth * 2)
		{
			ans |= AENG_MOUSE_OVER_WAYPOINT;
		}
	}

	return ans;
}


//---------------------------------------------------------------
//	CANIS.
//---------------------------------------------------------------


void AENG_groundsquare_draw(
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE polyinit)
{
	POLY_Point  pp[4];
	POLY_Point *quad[4];
	SLONG x,y,z,id,diff;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

//	y = PAP_calc_map_height_at(lx>>8,lz>>8)+ly;
	POLY_transform(lx	 ,ly,lz		,&pp[0]);

//	y = PAP_calc_map_height_at((lx>>8)+1,lz>>8)+ly;
	POLY_transform(lx+256,ly,lz		,&pp[1]);

//	y = PAP_calc_map_height_at(lx>>8,(lz>>8)+1)+ly;
	POLY_transform(lx	 ,ly,lz+256	,&pp[2]);

//	y = PAP_calc_map_height_at((lx>>8)+1,(lz>>8)+1)+ly;
	POLY_transform(lx+256,ly,lz+256 ,&pp[3]);

	pp[0].specular=pp[1].specular=pp[2].specular=pp[3].specular=0xFF000000;
	pp[0].colour=pp[1].colour=pp[2].colour=pp[3].colour=colour;

	if (polyinit & 1) POLY_frame_init(FALSE, FALSE);

	if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid() && pp[3].MaybeValid())
	{
		POLY_add_quad(quad,POLY_PAGE_ALPHA,FALSE);
	}

	if (polyinit & 2) POLY_frame_draw(FALSE, FALSE);
}

#endif //#ifndef TARGET_DC


//---------------------------------------------------------------

UBYTE AENG_transparent_warehouses;

void AENG_clear_viewport()
{
	//
	// Clear screen...
	//

	if (INDOORS_INDEX||(GAME_FLAGS & GF_SEWERS) || (GAME_FLAGS & GF_INDOORS))
	{
		SET_BLACK_BACKGROUND;
		CLEAR_VIEWPORT;
	}
	else
	{
		if (draw_3d)
		{
			SLONG white = NIGHT_sky_colour.red + NIGHT_sky_colour.green + NIGHT_sky_colour.blue;

			white /= 3;

			the_display.SetUserColour(
				white,
				white,
				white);
		}
		else
		{
			if(fade_black)
			{
				the_display.SetUserColour(0,0,0);
			}
			else
			{
				the_display.SetUserColour(
				NIGHT_sky_colour.red,
				NIGHT_sky_colour.green,
				NIGHT_sky_colour.blue);

			}
		}

		the_display.SetUserBackground();
		the_display.ClearViewport();

#if 0 && USE_TOMS_ENGINE_PLEASE_BOB
		// Haha! Nasty kludge!
		the_display.lp_D3D_Viewport->Clear2(
								1,
								&(the_display.ViewportRect),
								D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
								0xff806040,
								1.0f,
								0);
#endif
	}

	BreakTime("Cleared Viewport");
}


SLONG AENG_drawing_a_warehouse;



ULONG AENG_draw_time;
ULONG AENG_poly_add_quad_time;



void AENG_draw(SLONG draw_3d)
{
	ULONG start_rdtsc = AENG_rdtsc();

	AENG_poly_add_quad_time = 0;



#ifndef TARGET_DC
	if (SOFTWARE)
	{
		//
		// Leave dirt as it is. Turn everything else off.
		//

		AENG_detail_crinkles          = FALSE;
		AENG_detail_stars             = FALSE;
		AENG_detail_shadows           = FALSE;
		AENG_detail_moon_reflection   = FALSE;
		AENG_detail_people_reflection = FALSE;
		AENG_detail_puddles           = FALSE;
		AENG_detail_mist              = FALSE;
		AENG_detail_rain              = FALSE;
		AENG_detail_skyline           = FALSE;
		AENG_detail_filter            = FALSE;
	}
#endif //#ifndef TARGET_DC

	/*

	if (Keys[KB_PPOINT])
	{
		Keys[KB_PPOINT] = 0;

		sw_hack ^= TRUE;

		if (sw_hack)
		{
			SW_reload_textures();

			NIGHT_amb_red   >>= 1;
			NIGHT_amb_green >>= 1;
			NIGHT_amb_blue  >>= 1;
		}
		else
		{
			NIGHT_amb_red   <<= 1;
			NIGHT_amb_green <<= 1;
			NIGHT_amb_blue  <<= 1;
		}

		NIGHT_Colour amb_colour;

		amb_colour.red   = NIGHT_amb_red;
		amb_colour.green = NIGHT_amb_green;
		amb_colour.blue  = NIGHT_amb_blue;

		NIGHT_get_d3d_colour(
			amb_colour,
		   &NIGHT_amb_d3d_colour,
		   &NIGHT_amb_d3d_specular);

		NIGHT_cache_recalc();
		NIGHT_dfcache_recalc();
		NIGHT_generate_walkable_lighting();
	}

	*/

	SLONG i;
	SLONG warehouse;

	FC_Cam *fc;

#if 0
	// set CurDrawDistance
	CurDrawDistance = FC_cam[1].focus ? 16 : NormalDrawDistance;
#endif

#ifndef TARGET_DC
	if (SOFTWARE)
	{
		CurDrawDistance = 16;
	}
#endif



#ifdef TARGET_DC
	fiddle_draw_distance_DC();
#endif


/*
	if (Keys[KB_RBRACE])
	{
		Keys[KB_RBRACE] = 0;

		AENG_transparent_warehouses ^= 1;
	}
*/

	AENG_drawing_a_warehouse = FALSE;

	//
	// Update stuff.
	//

//d	AENG_movie_update();
#ifndef TARGET_DC
	move_clouds();
#endif
	POLY_set_wibble(62, 137, 17, 178, 20, 25);

#ifndef TARGET_DC
	if (sw_hack)
	{
		SLONG width  = MIN(RealDisplayWidth,  SW_MAX_WIDTH);
		SLONG height = MIN(RealDisplayHeight, SW_MAX_HEIGHT);

		SW_init(width, height);
	}
#endif

	AENG_clear_viewport();

	//
	// reclaim vertex buffers
	//

	TheVPool->ReclaimBuffers();

	//
	// Put in the dynamic lighting.
	//

	NIGHT_dlight_squares_up();

	POLY_colour_restrict      = 0;
	POLY_force_additive_alpha = FALSE;

	//
	// Mark all NIGHT cache squares for deletion.
	//

	AENG_mark_night_squares_as_deleteme();

	if (FC_cam[1].focus)
	{
		//
		// Splitscreen mode!
		//

		SUPERMAP_counter_increase(0);
		SUPERMAP_counter_increase(1);

		for (i = 0; i < FC_MAX_CAMS; i++)
		{
			fc = &FC_cam[i];

			AENG_lens = fc->lens * 1.5F * (1.0F / float(65536.0F));

			AENG_set_camera_radians(
				fc->x >> 8,
				fc->y >> 8,
				fc->z >> 8,
				float(fc->yaw)   * (2.0F * PI / (2048.0F * 256.0F)),
				float(fc->pitch) * (2.0F * PI / (2048.0F * 256.0F)),
				float(fc->roll)  * (2.0F * PI / (2048.0F * 256.0F)),
				(i == 0) ? POLY_SPLITSCREEN_TOP : POLY_SPLITSCREEN_BOTTOM);

			AENG_cur_fc_cam = i;

			if (fc->focus->Class == CLASS_PERSON &&
				fc->focus->Genus.Person->Ware)
			{
				AENG_ensure_appropriate_caching(TRUE);
				AENG_draw_warehouse();
			}
			else
			{
				AENG_ensure_appropriate_caching(FALSE);
				AENG_draw_city();
			}
		}

		AENG_get_rid_of_deleteme_squares();
		AENG_get_rid_of_unused_dfcache_lighting(TRUE);
	}
	else
	{
		fc = &FC_cam[0];

		SUPERMAP_counter_increase(0);

		//
		// Not splitscreen.  We might have to use the cutscene camera.
		//

		SLONG old_cam_x     = fc->x;
		SLONG old_cam_y     = fc->y;
		SLONG old_cam_z     = fc->z;
		SLONG old_cam_yaw   = fc->yaw;
		SLONG old_cam_pitch = fc->pitch;
		SLONG old_cam_roll  = fc->roll;
		SLONG old_cam_lens  = fc->lens;

		//
		// If there is a cut-scene camera...
		//

		if (EWAY_grab_camera(
				&fc->x,
				&fc->y,
				&fc->z,
				&fc->yaw,
				&fc->pitch,
				&fc->roll,
				&fc->lens))
		{
			warehouse = EWAY_camera_warehouse();
		}
		else
		{
			warehouse = (fc->focus->Class == CLASS_PERSON && fc->focus->Genus.Person->Ware);
		}

extern MFFileHandle	playback_file;
extern MFFileHandle	verifier_file;

		if (GAME_STATE & GS_PLAYBACK)
		{
			FileRead(playback_file, &fc->x, sizeof(fc->x));
			FileRead(playback_file, &fc->y, sizeof(fc->y));
			FileRead(playback_file, &fc->z, sizeof(fc->z));
			FileRead(playback_file, &fc->yaw, sizeof(fc->yaw));
			FileRead(playback_file, &fc->pitch, sizeof(fc->pitch));
			FileRead(playback_file, &fc->roll, sizeof(fc->roll));
			FileRead(playback_file, &fc->lens, sizeof(fc->lens));

			if (verifier_file)
			{
extern void check_thing_data();
				check_thing_data();
			}
		}
		else if (GAME_STATE & GS_RECORD)
		{
			FileWrite(playback_file, &fc->x, sizeof(fc->x));
			FileWrite(playback_file, &fc->y, sizeof(fc->y));
			FileWrite(playback_file, &fc->z, sizeof(fc->z));
			FileWrite(playback_file, &fc->yaw, sizeof(fc->yaw));
			FileWrite(playback_file, &fc->pitch, sizeof(fc->pitch));
			FileWrite(playback_file, &fc->roll, sizeof(fc->roll));
			FileWrite(playback_file, &fc->lens, sizeof(fc->lens));

			if (verifier_file)
			{
extern void store_thing_data();
				store_thing_data();
			}
		}

		AENG_lens = fc->lens * (1.0F / float(65536.0F));

		AENG_set_camera_radians(
			fc->x >> 8,
			fc->y >> 8,
			fc->z >> 8,
			float(fc->yaw  ) * (2.0F * PI / (2048.0F * 256.0F)),
			float(fc->pitch) * (2.0F * PI / (2048.0F * 256.0F)),
			float(fc->roll ) * (2.0F * PI / (2048.0F * 256.0F)),
			POLY_SPLITSCREEN_NONE);

		AENG_cur_fc_cam = 0;

		if (warehouse)
		{
			AENG_drawing_a_warehouse = TRUE;

			AENG_ensure_appropriate_caching(TRUE);
			AENG_draw_warehouse();
		}
		else
		{
			AENG_ensure_appropriate_caching(FALSE);
//			if(ShiftFlag)
				AENG_draw_city();

			if (AENG_transparent_warehouses)
			{
				SUPERMAP_counter_increase(0);

				AENG_ensure_appropriate_caching(TRUE);
				AENG_draw_warehouse();
			}
		}

		AENG_get_rid_of_deleteme_squares();
		AENG_get_rid_of_unused_dfcache_lighting(FALSE);

		//
		// Restore the old camera.
		//

		fc->x     = old_cam_x;
		fc->y	  = old_cam_y;
		fc->z	  = old_cam_z;
		fc->yaw	  = old_cam_yaw;
		fc->pitch = old_cam_pitch;
		fc->roll  = old_cam_roll;
		fc->lens  = old_cam_lens;
	}

	/*

	if (draw_3d)
	{
		//
		// How far apart are our eyes!?
		//

		static float eyes_apart = 10.0F;

		if (Keys[KB_6])
		{
			if (ShiftFlag)
			{
				eyes_apart -= 1.0F;
			}
			else
			{
				eyes_apart += 1.0F;
			}
		}

		float cam_x      = AENG_cam_x;
		float cam_y	     = AENG_cam_y;
		float cam_z		 = AENG_cam_z;
		float cam_yaw	 = AENG_cam_yaw;
		float cam_pitch	 = AENG_cam_pitch;
		float cam_roll	 = AENG_cam_roll;
		float cam_matrix[9];

		MATRIX_calc(
			cam_matrix,
			cam_yaw,
			cam_pitch,
			cam_roll);

		//
		// Left eye.
		//

		if (!ControlFlag)
		{
			AENG_set_camera_radians(
				SLONG(cam_x + cam_matrix[0] * eyes_apart),
				SLONG(cam_y + cam_matrix[1] * eyes_apart),
				SLONG(cam_z + cam_matrix[2] * eyes_apart),
				cam_yaw,
				cam_pitch,
				cam_roll);

			POLY_colour_restrict      = 0x0000ffff;		// No green or blue
			POLY_force_additive_alpha = FALSE;

			AENG_draw_city();

			//
			// Clear just the z-buffer.
			//

			HRESULT res = the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);

			ASSERT(res == DD_OK);
		}

		if (!ShiftFlag)
		{
			//
			// Right eye.
			//

			AENG_set_camera_radians(
				SLONG(cam_x - cam_matrix[0] * eyes_apart),
				SLONG(cam_y - cam_matrix[1] * eyes_apart),
				SLONG(cam_z - cam_matrix[2] * eyes_apart),
				cam_yaw,
				cam_pitch,
				cam_roll);

			POLY_colour_restrict      = 0x00ff0000;		// No red
			POLY_force_additive_alpha = TRUE;

			GAME_TURN += 1;		// So the buildings are drawn again...

			AENG_draw_city();
		}
	}
	else
	{
		if (GAME_FLAGS & GF_INDOORS)
		{
//			AENG_draw_inside();
		}
		else
		if (GAME_FLAGS & GF_SEWERS)
		{
			AENG_draw_ns();
		}
		else
		if (WARE_in)
		{
			AENG_draw_warehouse();
		}
		else
		{
			AENG_draw_city();
		}
	}

	*/

	//
	// Take out the dynamic lighting.
	//

	NIGHT_dlight_squares_down();

	POLY_colour_restrict      = 0;
	POLY_force_additive_alpha = FALSE;

	//SPONG
//#if !USE_TOMS_ENGINE_PLEASE_BOB
	//
	// Do it here so that AENG_world_line works during the game.
	//

#ifndef TARGET_DC
	POLY_frame_init(FALSE, FALSE);
#endif
//#endif


	ULONG end_rdtsc = AENG_rdtsc();

	if (end_rdtsc > start_rdtsc)
	{
		//
		// The counter hasn't wrapped...
		//

		AENG_draw_time = end_rdtsc - start_rdtsc;
	}
}








// read detail levels from file

void AENG_read_detail_levels()
{
#ifdef TARGET_DC
	// Most things turned on, apart from the things that won't work.
	//AENG_estimate_detail_levels = ENV_get_value_number("estimate_detail_levels", 0, "Render");

	AENG_detail_stars = ENV_get_value_number("detail_stars", 1, "Render");
	AENG_detail_shadows = ENV_get_value_number("detail_shadows", 0, "Render");
	AENG_detail_moon_reflection = ENV_get_value_number("detail_moon_reflection", 0, "Render");
	AENG_detail_people_reflection = ENV_get_value_number("detail_people_reflection", 0, "Render");
	AENG_detail_puddles = ENV_get_value_number("detail_puddles", 0, "Render");
	AENG_detail_dirt = ENV_get_value_number("detail_dirt", 1, "Render");
	AENG_detail_mist = ENV_get_value_number("detail_mist", 1, "Render");
	AENG_detail_rain = ENV_get_value_number("detail_rain", 1, "Render");
	AENG_detail_skyline = ENV_get_value_number("detail_skyline", 1, "Render");
	AENG_detail_filter = ENV_get_value_number("detail_filter", 1, "Render");
	AENG_detail_perspective = ENV_get_value_number("detail_perspective", 1, "Render");
	AENG_detail_crinkles = ENV_get_value_number("detail_crinkles", 0, "Render");
#ifndef DEBUG
	// Release build - doesn't include the things that don't work yet!
	AENG_detail_stars = ENV_get_value_number("detail_stars", 1, "Render");
	AENG_detail_shadows = ENV_get_value_number("detail_shadows", 0, "Render");
	AENG_detail_moon_reflection = ENV_get_value_number("detail_moon_reflection", 0, "Render");
	AENG_detail_people_reflection = ENV_get_value_number("detail_people_reflection", 0, "Render");
	AENG_detail_puddles = ENV_get_value_number("detail_puddles", 1, "Render");
	AENG_detail_dirt = ENV_get_value_number("detail_dirt", 1, "Render");
	AENG_detail_mist = ENV_get_value_number("detail_mist", 1, "Render");
	AENG_detail_rain = ENV_get_value_number("detail_rain", 1, "Render");
	AENG_detail_skyline = ENV_get_value_number("detail_skyline", 1, "Render");
	AENG_detail_filter = ENV_get_value_number("detail_filter", 1, "Render");
	AENG_detail_perspective = ENV_get_value_number("detail_perspective", 1, "Render");
	AENG_detail_crinkles = ENV_get_value_number("detail_crinkles", 0, "Render");
#endif
#else
	AENG_estimate_detail_levels = ENV_get_value_number("estimate_detail_levels", 1, "Render");

	AENG_detail_stars = ENV_get_value_number("detail_stars", 1, "Render");
	AENG_detail_shadows = ENV_get_value_number("detail_shadows", 1, "Render");
	AENG_detail_moon_reflection = ENV_get_value_number("detail_moon_reflection", 1, "Render");
	AENG_detail_people_reflection = ENV_get_value_number("detail_people_reflection", 1, "Render");
	AENG_detail_puddles = ENV_get_value_number("detail_puddles", 1, "Render");
	AENG_detail_dirt = ENV_get_value_number("detail_dirt", 1, "Render");
	AENG_detail_mist = ENV_get_value_number("detail_mist", 1, "Render");
	AENG_detail_rain = ENV_get_value_number("detail_rain", 1, "Render");
	AENG_detail_skyline = ENV_get_value_number("detail_skyline", 1, "Render");
	AENG_detail_filter = ENV_get_value_number("detail_filter", 1, "Render");
	AENG_detail_perspective = ENV_get_value_number("detail_perspective", 1, "Render");
	AENG_detail_crinkles = ENV_get_value_number("detail_crinkles", 1, "Render");
#endif
}





//
// Draws a small inside of the warehouse.
//

void AENG_draw_box_around_recessed_door(DFacet *df, SLONG inside_out)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG page;
	SLONG upto;

	ULONG sky_colour;
	ULONG sky_specular;

	SLONG col_page;
	SLONG specular;

#ifdef TARGET_DC
	POLY_flush_local_rot();
#endif

	NIGHT_get_d3d_colour(
		NIGHT_sky_colour,
	   &sky_colour,
	   &sky_specular);

	sky_specular |= 0xff000000;

	PAP_Hi *ph;

	float wx;
	float wy;
	float wz;

	#define MAX_DOOR_LENGTH 8

	POLY_Point lower[MAX_DOOR_LENGTH][2];
	POLY_Point upper[MAX_DOOR_LENGTH][2];

	POLY_Point *pp;
	POLY_Point *quad[4];

	if (inside_out)
	{
		SWAP(df->x[0], df->x[1]);
		SWAP(df->z[0], df->z[1]);
	}

	if (sw_hack)
	{
		col_page = POLY_PAGE_COLOUR;
		specular = 0xee000000;
	}
	else
	{
		col_page = POLY_PAGE_COLOUR_WITH_FOG;
		specular = 0xff000000;
	}

	//
	// Create the points.
	//

	x = df->x[0];
	z = df->z[0];

	dx = df->x[1] - df->x[0];
	dz = df->z[1] - df->z[0];

	dx = SIGN(dx);
	dz = SIGN(dz);

	upto = 0;

	while(1)
	{
		ASSERT(WITHIN(upto, 0, MAX_DOOR_LENGTH - 1));

		for (i = 0; i < 4; i++)
		{
			wx = float(x << 8);
			wz = float(z << 8);
			wy = float(df->Y[0]);

			if (i & 1)
			{
				//
				// One block inside the warehouse.
				//

				wx += float(-dz << 8);
				wz += float(+dx << 8);
			}

			if (i & 2)
			{
				//
				// The upper level.
				//

				pp = &upper[upto][i & 1];

				wy += 256.0F;
			}
			else
			{
				//
				// The lower level.
				//

				pp = &lower[upto][i & 1];
			}

			POLY_transform(
				wx,
				wy,
				wz,
			    pp);

			if (i == 0)
			{
				if (inside_out)
				{
					pp->colour   = 0xffaaaaaa;
					pp->specular = 0xff000000;
				}
				else
				{
					pp->colour   = 0xffaaaaaa;
					pp->specular = 0xff000000;
				}
			}
			else
			{
				if (inside_out)
				{
					pp->colour   = 0xffffffff;
					pp->specular = 0x01000000;	// Completely fogged out...
				}
				else
				{
					pp->colour   = 0xff000000;
					pp->specular = 0xff000000;
				}
			}

			pp->u = 0.0F;
			pp->v = 0.0F;
		}

		upto += 1;

		if (x == df->x[1] && z == df->z[1])
		{
			break;
		}

		x += dx;
		z += dz;
	}

	ASSERT(upto >= 2);

	//
	// The faces on the floor and the ceiling.
	//

	x = df->x[0];
	z = df->z[0];

	for (i = 0; i < upto - 1; i++)
	{
		mx = (x << 8) + dx - dz >> 8;
		mz = (z << 8) + dz + dx >> 8;

		//
		// The floor.
		//

		ph = &PAP_2HI(mx,mz);

		quad[0] = &lower[i + 0][0];
		quad[1] = &lower[i + 0][1];
		quad[2] = &lower[i + 1][0];
		quad[3] = &lower[i + 1][1];

		if (POLY_valid_quad(quad))
		{
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

			POLY_add_quad(quad, page, FALSE);
		}

		//
		// The ceiling.
		//

		quad[0] = &upper[i + 0][0];
		quad[1] = &upper[i + 0][1];
		quad[2] = &upper[i + 1][0];
		quad[3] = &upper[i + 1][1];

		if (POLY_valid_quad(quad))
		{
			POLY_add_quad(quad, col_page, FALSE);
		}

		//
		// The back wall.
		//

		quad[0] = &lower[i + 0][1];
		quad[1] = &lower[i + 1][1];
		quad[2] = &upper[i + 0][1];
		quad[3] = &upper[i + 1][1];

		if (POLY_valid_quad(quad))
		{
			POLY_add_quad(quad, col_page, FALSE);
		}

		x += dx;
		z += dz;
	}

	//
	// Create the two faces at each end.
	//

	quad[0] = &upper[0][0];
	quad[2] = &upper[0][1];
	quad[1] = &lower[0][0];
	quad[3] = &lower[0][1];

	if (POLY_valid_quad(quad))
	{
		if (!inside_out)
		{
			quad[0]->colour   = 0xff000000;
			quad[0]->specular = 0xff000000;
			quad[1]->colour   = 0xff000000;
			quad[1]->specular = 0xff000000;
		}

		POLY_add_quad(quad, col_page, TRUE);
	}

	quad[0] = &upper[upto - 1][0];
	quad[1] = &upper[upto - 1][1];
	quad[2] = &lower[upto - 1][0];
	quad[3] = &lower[upto - 1][1];

	if (POLY_valid_quad(quad))
	{
		if (!inside_out)
		{
			quad[0]->colour   = 0xff000000;
			quad[0]->specular = 0xff000000;
			quad[2]->colour   = 0xff000000;
			quad[2]->specular = 0xff000000;
		}

		POLY_add_quad(quad, col_page, TRUE);
	}

	if (inside_out)
	{
		SWAP(df->x[0], df->x[1]);
		SWAP(df->z[0], df->z[1]);
	}
}




//
// Get rid of any unused dfcache lighting.
//

void AENG_get_rid_of_unused_dfcache_lighting(SLONG splitscreen)	// Splitscreen = TRUE or FALSE
{
	SLONG dfcache;
	SLONG next;

	NIGHT_Dfcache *ndf;

	for (dfcache = NIGHT_dfcache_used; dfcache; dfcache = next)
	{
		ASSERT(WITHIN(dfcache, 1, NIGHT_MAX_DFCACHES - 1));

		ndf  = &NIGHT_dfcache[dfcache];
		next = 	ndf->next;

		//
		// Was this facet drawn this gameturn? If it wasn't then
		// free up the cached lighting info for it.
		//

		ASSERT(WITHIN(ndf->dfacet, 1, next_dfacet - 1));
		ASSERT(dfacets[ndf->dfacet].Dfcache == dfcache);

		if (dfacets[ndf->dfacet].Counter[0] != SUPERMAP_counter[0])
		{
			if (splitscreen)
			{
				//
				// Might have been drawn from the second camera.
				//

				if (dfacets[ndf->dfacet].Counter[1] == SUPERMAP_counter[1])
				{
					//
					// It was drawn from the second camera! Don't get rid of it.
					//

					continue;
				}
			}

			//
			// Free up the lighting info.
			//

			dfacets[ndf->dfacet].Dfcache = 0;

			NIGHT_dfcache_destroy(dfcache);
		}
	}
}


void	AENG_draw_inside_floor(UWORD inside_index,UWORD inside_room,UBYTE fade)
{
	SLONG	x,z;
	SLONG page;

	float world_x;
	float world_y,floor_y,roof_y;
	float world_z;

	POLY_Point pp[4];
#ifndef TARGET_DC
	MapElement *me;
#endif

	PAP_Lo *pl;
	PAP_Hi *ph;

	POLY_Point *quad[4];

	struct	InsideStorey	*p_inside;
	SLONG	in_width;
	UBYTE	*in_block;
	SLONG	min_z,max_z;
	SLONG	c0;
	SLONG	floor_type;
	SLONG	do_light;

	if(inside_index==light_inside)
		do_light=1;
	else
		do_light=0;


	//
	// draw the internal walls
	//
extern	void	draw_insides(SLONG indoor_index,SLONG room,UBYTE fade);
	draw_insides(inside_index,inside_room,fade);


	p_inside=&inside_storeys[inside_index];

	floor_type=p_inside->TexType;

//	MSG_add("in room %d\n",INDOORS_ROOM);


	floor_y=(float)p_inside->StoreyY;
	roof_y=floor_y+256.0f;

	min_z=MAX(NGAMUT_point_zmin,p_inside->MinZ);
	max_z=MIN(NGAMUT_point_zmax,p_inside->MaxZ);

	in_width=p_inside->MaxX-p_inside->MinX;

	in_block=&inside_block[p_inside->InsideBlock];

	for(c0=0;c0<4;c0++)
	{
		pp[c0].colour=0xffffff;
		pp[c0].specular=0xff000000;
	}

	quad[0]=&pp[0];
	quad[1]=&pp[1];
	quad[2]=&pp[2];
	quad[3]=&pp[3];

	quad[0]->u=0.0;
	quad[0]->v=0.0;
	quad[1]->u=1.0;
	quad[1]->v=0.0;
	quad[2]->u=0.0;
	quad[2]->v=1.0;
	quad[3]->u=1.0;
	quad[3]->v=1.0;


	for (z = min_z; z < max_z; z++)
	{
		SLONG	min_x,max_x;
		float	face_y;
		SLONG	col;
		min_x=MAX(NGAMUT_point_gamut[z].xmin,p_inside->MinX);
		max_x=MIN(NGAMUT_point_gamut[z].xmax,p_inside->MaxX);

		for (x = min_x;x<max_x;x++)
		{
			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

#ifndef TARGET_DC
			me = &MAP[MAP_INDEX(x, z)];
#endif
			ph = &PAP_2HI(x,z);

			if ((PAP_2HI(x,z).Flags & (PAP_FLAG_HIDDEN)))
			{
				SLONG	room_id;
				SLONG	px,pz,dx,dz,square;
				NIGHT_Square *nq;




				room_id=in_block[(x-p_inside->MinX)+(z-p_inside->MinZ)*in_width]&(0xf|0x80|0x40);
				if(!(room_id&0xc0))
				{
					if(1||(room_id&0xf)==inside_room)
					{
						face_y=floor_y;
						col=0xffffff;
					}
					else
					{
						face_y=roof_y;
						col=0;

					}

					col=col|( (fade&255)<<24);
					if(room_id)
					{

						world_x = x       * 256.0F;
						world_z = z       * 256.0F;
						POLY_transform(world_x, face_y, world_z, &pp[0]);

						if(do_light)
						{
							px = x >> 2;
							pz = z >> 2;


							ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
							ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

							square = NIGHT_cache[px][pz];
							ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
							ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);
							if(!square)
								return;
							nq = &NIGHT_square[square];

							NIGHT_get_d3d_colour(
								nq->colour[(x&3) + (z&3) * PAP_BLOCKS],
							   &pp->colour,
							   &pp->specular);
							pp->colour|=fade<<24;
						}
						else
						{
							pp->colour=0x7f7f7f|(fade<<24);
							pp->specular=0xff000000;
						}


						world_x = (x+1)       * 256.0F;
						world_z = z       * 256.0F;
						POLY_transform(world_x, face_y, world_z, &pp[1]);

						if(do_light)
						{
							if( (x+1)>>2!=px)
							{
								px = (x+1) >> 2;
	//							pz = z >> 2;


								ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
								ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

								square = NIGHT_cache[px][pz];
								ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
								ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);
							if(!square)
								return;
								nq = &NIGHT_square[square];
							}
							NIGHT_get_d3d_colour(
								nq->colour[((x+1)&3) + (z&3) * PAP_BLOCKS],
							   &(pp[1].colour),
							   &(pp[1].specular));

							pp[1].colour|=fade<<24;
						}
						else
						{
							pp[1].colour=0x7f7f7f|(fade<<24);
							pp[1].specular=0xff000000;
						}





						world_x = x       * 256.0F;
						world_z = (z+1)       * 256.0F;
						POLY_transform(world_x, face_y, world_z, &pp[2]);


						if(do_light)
						{
							if( (z+1)>>2!=pz || (x>>2)!=px)
							{
								px = x >> 2;
								pz = (z+1) >> 2;


								ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
								ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

								square = NIGHT_cache[px][pz];
								ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
								ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);
							if(!square)
								return;
								nq = &NIGHT_square[square];
							}

							NIGHT_get_d3d_colour(
								nq->colour[(x&3) + ((z+1)&3) * PAP_BLOCKS],
							   &(pp[2].colour),
							   &(pp[2].specular));
							pp[2].colour|=fade<<24;
						}
						else
						{
							pp[2].colour=0x7f7f7f|(fade<<24);
							pp[2].specular=0xff000000;
						}




						world_x = (x+1)       * 256.0F;
						world_z = (z+1)       * 256.0F;
						POLY_transform(world_x, face_y, world_z, &pp[3]);

						if(do_light)
						{
							if( (x+1)>>2!=px || (z+1)>>2!=pz)
							{
								px = (x+1) >> 2;
								pz = (z+1) >> 2;


								ASSERT(WITHIN(px, 0, PAP_SIZE_LO - 1));
								ASSERT(WITHIN(pz, 0, PAP_SIZE_LO - 1));

								square = NIGHT_cache[px][pz];
							if(!square)
								return;
								ASSERT(WITHIN(square, 1, NIGHT_MAX_SQUARES - 1));
								ASSERT(NIGHT_square[square].flag & NIGHT_SQUARE_FLAG_USED);
								nq = &NIGHT_square[square];
							}

							NIGHT_get_d3d_colour(
								nq->colour[((x+1)&3) + ((z+1)&3) * PAP_BLOCKS],
							   &(pp[3].colour),
							   &(pp[3].specular));
							pp[3].colour|=fade<<24;
						}
						else
						{
							pp[3].colour=0x7f7f7f|(fade<<24);
							pp[3].specular=0xff000000;
						}



						if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid() && pp[3].MaybeValid())
						{
							pp[0].colour=col;
							pp[1].colour=col;
							pp[2].colour=col;
							pp[3].colour=col;
							page=inside_tex[floor_type][room_id-1]+START_PAGE_FOR_FLOOR*64; // temp
							POLY_add_quad(quad, page, FALSE);
						}
					}
				}
			}
		}
	}
}



