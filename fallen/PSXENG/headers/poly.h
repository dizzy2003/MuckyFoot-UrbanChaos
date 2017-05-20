//
// Drawing polygons with D3D
//

#ifndef _POLY_H
#define _POLY_H

#define _POLY_

//
// Call once at the start of the whole program.
//

void POLY_init(void);


// ========================================================
//
// TRANSFORMING POINTS
//
// ========================================================

//
// Sets the position of the camera.
//

void POLY_camera_set(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,
		SLONG view_dist,	// The maximum distance a point should be from the camera.
		SLONG lens);		// Normally around 1.5F... the higher it is the more zoom you get.

//
// Given three points in world space, this function fills in
// the 3d view space coordinates of the point.  If the point
// is not 'behind' the camera, then it is transformed.  If it
// is transformed then the 2d points and clipping flags are
// calculated.
//

extern	MATRIX	PSX_view_matrix;

#define POLY_CLIP_LEFT			(1 << 0)
#define POLY_CLIP_RIGHT			(1 << 1)
#define POLY_CLIP_TOP			(1 << 2)
#define POLY_CLIP_BOTTOM		(1 << 3)
#define POLY_CLIP_TRANSFORMED	(1 << 4)
#define POLY_CLIP_FAR			(1 << 5)	// View space Z too far away
#define POLY_CLIP_NEAR			(1 << 6)	// View space Z too near
#define POLY_CLIP_ONSCREEN      (POLY_CLIP_LEFT | POLY_CLIP_RIGHT | POLY_CLIP_TOP | POLY_CLIP_BOTTOM | POLY_CLIP_FAR | POLY_CLIP_NEAR)

typedef struct
{
	SLONG x;	//              
	SLONG y;	// 3D points... 
	SLONG z;	//              

	SLONG X;	//             
	SLONG Y;	// 2D points...
	SLONG Z;	//             

	ULONG clip;

	SLONG u;
	SLONG v;
	ULONG colour;		// xxRRGGBB
	ULONG specular;		// xxRRGGBB
	
} POLY_Point;

typedef struct 
{
	SVECTOR World;
	union 
	{
		struct 
		{
			SWORD SX,SY;
		}Word;
		SLONG	SYSX;
	};
	SLONG	Z;
	SLONG   P;
	SLONG	Flag;
} PSX_POLY_Point;

typedef union 
{
	struct 
	{
		SWORD SX,SY;
	}Word;
	SLONG	SYSX;
}PSX_Screen_XY;

void	build_rot_matrix(SLONG yaw,SLONG pitch,MATRIX *m);

void POLY_transform(
		SLONG       world_x,
		SLONG       world_y,
		SLONG       world_z,
		POLY_Point *pt);

//
// Sets an additional matrix to be applied to the world point before
// it is transformed.
//

void POLY_set_local_rotation(
		SLONG off_x,
		SLONG off_y,
		SLONG off_z,
		MATRIX *matrix);

void POLY_transform_using_local_rotation(
		SLONG       local_x,
		SLONG       local_y,
		SLONG       local_z,
		POLY_Point *pt);

//
// Returns true if the given sphere in world space is visible
// on screen. 'radius' should be given as a fraction of the viewdist passed
// to POLY_camera_set.  radius = world_radius / view_dist.
//

SLONG POLY_sphere_visible(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG radius);

//
// Handy buffers for rotating objects
//
/*
*/
#define POLY_BUFFER_SIZE 768
#define POLY_SHADOW_SIZE 10

//extern POLY_Point POLY_buffer[POLY_BUFFER_SIZE];
extern PSX_POLY_Point PSX_POLY_buffer[POLY_BUFFER_SIZE];
extern SLONG      POLY_buffer_upto;

//extern POLY_Point POLY_shadow[POLY_BUFFER_SIZE];
extern SLONG      POLY_shadow_upto;
//
// The fade-out range of the points.
//

#define POLY_FADEOUT_START	((SLONG)(0.75F*65536))
#define POLY_FADEOUT_END	(65536)

//
// Applies fade out to the given point.
// Applies fade out to all the points in the POLY_buffer array.
//

static void inline POLY_fadeout_point(POLY_Point *pp)
{
	if (pp->z > POLY_FADEOUT_START)
	{
		SLONG multi;
		SLONG red;
		SLONG green;
		SLONG blue;

		multi = (256<<16) - SLONG((pp->z - POLY_FADEOUT_START) * ((256<<16) / (POLY_FADEOUT_END - POLY_FADEOUT_START)));
		multi>>=8;

		if (multi < 0)
		{
			pp->colour   = 0;
			pp->specular = 0;
			pp->clip    |= POLY_CLIP_FAR;
		}
		else
		{
			red   = ((pp->colour >> 16) & 0xff) * multi >> 8;
			green = ((pp->colour >>  8) & 0xff) * multi >> 8;
			blue  = ((pp->colour >>  0) & 0xff) * multi >> 8;

			pp->colour &= 0xff000000;
			pp->colour |= red   << 16;
			pp->colour |= green << 8;
			pp->colour |= blue  << 0;

		}
	}
}

void POLY_fadeout_buffer(void);

// ========================================================
//
// ADDING TRIANGLES / QUADS	/ LINES
//
// ========================================================

//
// NB There is no vertex sharing except in a quad.
// NO SIMPLE REJECTION ON THE CLIP FLAGS OR BACKFACE CULLING IS PERFORMED
// BY THE add_triangle() or add_quad() functions!
//

//
// Clears all buffers, ready for a new frame.
// Checks the clipflags and backface culling of the triangle and returns TRUE if it should be drawn.
// Adds a triangle and a quad.
// Adds a line. The widths are given in world-space sizes. if (sort_to_front) then lines will be drawn last with no z-buffer.
// Sets the box against which clip-lines are clipped.
// Adds a line clipped against the clip box.
// Draws all the triangles and quads.
//

//
// The 'page' argument should either be one of the TEXTURE module's
// texture pages or one of these defines.
//


#define EXTRA(x,y) (((y)*8)+(x))

#define POLY_PAGE_METEOR		(EXTRA(2,5))
#define POLY_PAGE_FINALGLOW		(EXTRA(0,1))
#define POLY_PAGE_BOLT			(EXTRA(2,0))
#define POLY_PAGE_LADDER		(EXTRA(1,4))
//#define POLY_PAGE_LEDNUMBER		(EXTRA(2,0))
#define POLY_PAGE_SMOKECLOUD	(EXTRA(0,7))
#define POLY_PAGE_EXPLODE1		(EXTRA(4,0))
#define POLY_PAGE_EXPLODE1_ADDITIVE (EXTRA(4,0))
#define POLY_PAGE_EXPLODE2		(EXTRA(4,0))
#define POLY_PAGE_EXPLODE2_ADDITIVE (EXTRA(4,0))
//#define POLY_PAGE_CIRCLE		(EXTRA(1,6))
//#define POLY_PAGE_GUNSIGHT		(EXTRA(1,5))
#define POLY_PAGE_SMOKER		(EXTRA(3,2))
//#define POLY_PAGE_DEVIL			(EXTRA(1,5))
//#define POLY_PAGE_ANGEL			(EXTRA(1,5))
#define POLY_PAGE_ADDITIVEALPHA	(EXTRA(7,7))
#define POLY_PAGE_TYRETRACK		(EXTRA(2,4))
#define POLY_PAGE_LEAF			(EXTRA(0,6))
//#define POLY_PAGE_WINMAP		(EXTRA(7,7))
//#define POLY_PAGE_ENVMAP		(EXTRA(7,7))
//#define POLY_PAGE_LENSFLARE		(EXTRA(2,7))
#define POLY_PAGE_HITSPANG		(EXTRA(3,6))
#define POLY_PAGE_BLOOM2		(EXTRA(0,7))
#define POLY_PAGE_BLOOM1		(EXTRA(1,6))
#define POLY_PAGE_BLOODSPLAT	(EXTRA(3,0))
#define POLY_PAGE_FLAMES3		(EXTRA(3,4))
#define POLY_PAGE_DUSTWAVE		(EXTRA(0,4))
//#define POLY_PAGE_BIGBANG		(EXTRA(7,7))
#define POLY_PAGE_FONT2D		(EXTRA(0,0))
#define POLY_PAGE_FOOTPRINT		(EXTRA(3,5))
#define POLY_PAGE_BARBWIRE		(EXTRA(0,5))
//#define POLY_PAGE_MENUPASS		(EXTRA(7,7))
//#define POLY_PAGE_MENUTEXT		(EXTRA(7,7))
#define POLY_PAGE_FLAMES2		(EXTRA(4,4))
#define POLY_PAGE_SMOKE			(EXTRA(0,7))
#define POLY_PAGE_FLAMES		(EXTRA(3,3))
//#define POLY_PAGE_SEWATER		(EXTRA(7,7))
//#define POLY_PAGE_SKY			(EXTRA(7,7))
#define POLY_PAGE_SHADOW		(EXTRA(3,7))
//#define POLY_PAGE_PUDDLE		(EXTRA(7,7))	
//#define POLY_PAGE_MOON			(EXTRA(0,0))
//#define POLY_PAGE_MANONMOON		(EXTRA(0,0))
//#define POLY_PAGE_CLOUDS		(EXTRA(7,7))
//#define POLY_PAGE_ALPHA			(EXTRA(7,7))
//#define POLY_PAGE_ADDITIVE		(EXTRA(7,7))
//#define POLY_PAGE_MASHED		(EXTRA(7,7))
//#define POLY_PAGE_COLOUR		(EXTRA(7,7))
//#define POLY_PAGE_WATER			(EXTRA(7,7))
#define POLY_PAGE_DRIP			(EXTRA(3,1))
//#define POLY_PAGE_FOG			(EXTRA(7,7))
//#define POLY_PAGE_BANG			(EXTRA(7,7))
#define POLY_PAGE_TEXT			(EXTRA(0,0))
//#define POLY_PAGE_LOGO			(EXTRA(7,7))
//#define POLY_PAGE_DROPLET		(EXTRA(7,7))
//#define POLY_PAGE_RAINDROP		(EXTRA(7,7))
//#define POLY_PAGE_SPARKLE		(EXTRA(7,7))
#define POLY_PAGE_STEAM			(EXTRA(3,2))
#define POLY_PAGE_SMOKECLOUD2	(EXTRA(1,0))
#define POLY_PAGE_TYRESKID		(EXTRA(2,4))

#define POLY_PAGE_ARROW			(EXTRA(0,0))
//#define POLY_PAGE_PADBITS		(EXTRA(2,6))

void  POLY_frame_init    (SLONG keep_shadow_page);	// TRUE => doesn't delete the shadow polygons.
SLONG POLY_valid_triangle(POLY_Point *p);

SLONG POLY_valid_quad    (POLY_Point *p);
SLONG POLY_valid_line    (POLY_Point *p1, POLY_Point *p2);
void  POLY_add_triangle  (POLY_Point *p, SLONG page, SLONG shall_i_backface_cull);
void  POLY_add_quad      (POLY_Point *p, SLONG page, SLONG shall_i_backface_cull);
void  POLY_add_line      (POLY_Point *p1, POLY_Point *p2, SLONG width1, SLONG width2, SLONG sort_to_front);
void  POLY_add_line_tex(PSX_POLY_Point *p1, PSX_POLY_Point *p2, SLONG width1, SLONG width2,SLONG page, ULONG colour, SLONG sort_to_front);
void  POLY_add_line_tex_uv(PSX_POLY_Point *p1, PSX_POLY_Point *p2, SLONG width1, SLONG width2,SLONG page, ULONG colour, SLONG sort_to_front);
void  POLY_add_line_2d   (SLONG sx1, SLONG sy1, SLONG sx2, SLONG sy2, ULONG colour);
void  POLY_clip_line_box (SLONG sx1, SLONG sy1, SLONG sx2, SLONG sy2);
void  POLY_clip_line_add (SLONG sx1, SLONG sy1, SLONG sx2, SLONG sy2, ULONG colour);
void  POLY_frame_draw    (SLONG draw_shadow_page);	// FALSE => Doens't draw the shadow polygons.
extern	SLONG POLY_valid_quadp(POLY_Point *pp[4],UWORD back_cull);
extern	SLONG POLY_valid_trianglep(POLY_Point *pp[3],UWORD back_cull);


extern	SLONG POLY_cam_x;
extern	SLONG POLY_cam_y;
extern	SLONG POLY_cam_z;

extern	SLONG POLY_cam_off_x;
extern	SLONG POLY_cam_off_y;
extern	SLONG POLY_cam_off_z;

#endif
