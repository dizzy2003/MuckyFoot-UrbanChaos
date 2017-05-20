//
// Drawing polygons with D3D
//

#define __MSC__

#include <MFStdLib.h>
#include <glide.h>
#include <math.h>
#include "matrix.h"
#include "glpoly.h"
#include "gltexture.h"
#include "clip.h"

//
// The vertices we pass to Glide.
//

typedef struct
{
	float sx;
	float sy;
	float rhw;	// Used for w-buffering
	FxU32 colour;

	float uow0;	// u * rhw for this TMU
	float vow0;	// v * rhw for this TMU
	float rhw0; //     rhw for this TMU

	float uow1;	// u * rhw for this TMU
	float vow1;	// v * rhw for this TMU
	float rhw1; //     rhw for this TMU

} POLY_GVertex;


//
// The handy buffer
//

POLY_Point POLY_buffer[POLY_BUFFER_SIZE];
SLONG      POLY_buffer_upto;

POLY_Point POLY_shadow[POLY_SHADOW_SIZE];
SLONG      POLY_shadow_upto;

//
// Only points further than 1/256'th of the draw range are drawn.
//

#define POLY_ZCLIP_PLANE (0.00781250F) // i.e. 1/128

//
// The camera and the screen.
//

float POLY_cam_x;
float POLY_cam_y;
float POLY_cam_z;
float POLY_cam_aspect;
float POLY_cam_lens;
float POLY_cam_view_dist;
float POLY_cam_over_view_dist;
float POLY_cam_matrix[9];

#define POLY_SCREEN_WIDTH  (640.0F)
#define POLY_SCREEN_HEIGHT (480.0F)

const float POLY_screen_width  = POLY_SCREEN_WIDTH;
const float POLY_screen_height = POLY_SCREEN_HEIGHT;
const float POLY_screen_mid_x  = POLY_SCREEN_WIDTH  * 0.5F;
const float POLY_screen_mid_y  = POLY_SCREEN_HEIGHT * 0.5F;
const float POLY_screen_mul_x  = POLY_SCREEN_WIDTH  * 0.5F / POLY_ZCLIP_PLANE;
const float POLY_screen_mul_y  = POLY_SCREEN_HEIGHT * 0.5F / POLY_ZCLIP_PLANE;


void POLY_init(void)
{
	//
	// Tell Glide about the GVertex structure.
	//

	grVertexLayout(GR_PARAM_XY,    (long) &((POLY_GVertex *) 0)->sx,     GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_W,     (long) &((POLY_GVertex *) 0)->rhw,    GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_PARGB, (long) &((POLY_GVertex *) 0)->colour, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST0,   (long) &((POLY_GVertex *) 0)->uow0,   GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST1,   (long) &((POLY_GVertex *) 0)->uow1,   GR_PARAM_ENABLE);

	//
	// Setup w-buffering.
	//

    grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER);
    grDepthBufferFunction(GR_CMP_LEQUAL);
    grDepthMask(FXTRUE);	// Triangles write into the wbuffer.

	//
	// Filtering.
	//

	grTexFilterMode(
		GR_TMU1,
		GR_TEXTUREFILTER_BILINEAR,
		GR_TEXTUREFILTER_BILINEAR);

	grTexFilterMode(
		GR_TMU0,
		GR_TEXTUREFILTER_BILINEAR,
		GR_TEXTUREFILTER_BILINEAR);

	//
	// Alpha shading.
	//

	grAlphaCombine(
		GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_LOCAL,
		GR_COMBINE_LOCAL_ITERATED,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);

	//
	// Setup gouraud shading with monochrome specular in alpha.
	//

	grColorCombine(
		GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
		GR_COMBINE_FACTOR_LOCAL,
		GR_COMBINE_LOCAL_ITERATED,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);

	//
	// This always stays the same.
	// 

	grTexCombine(
		GR_TMU1,
		GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
		FXFALSE,
		FXFALSE);
}


ULONG POLY_interpolate_colour(float v, ULONG colour1, ULONG colour2)
{
	SLONG rb1, rb2, drb, rba;
	SLONG ga1, ga2, dga, gaa;

	union
	{
		float vfloat;
		ULONG vfixed8;
	};

	SLONG answer;

	//
	// Early outs.
	//

	if (colour1 == colour2) {return colour1;}
	
	if (v < 0.01F) {return colour1;}
	if (v > 0.99F) {return colour2;}

	//
	// Work out how much to interpolate along in fixed point 8.
	//

	vfloat   = 32768.0F + v;
	vfixed8 &= 0xff;

	//
	// Red and blue.
	//

	rb1 = colour1 & (0x00ff00ff);
	rb2 = colour2 & (0x00ff00ff);

	if (rb1 != rb2)
	{
		//
		// Do interpolation of red and blue simultaneously.
		//

		drb   = rb2 - rb1;
		drb  *= vfixed8;
		drb >>= 8;
		rba   = rb1 + drb;
		rba  &= 0x00ff00ff;
	}
	else
	{
		//
		// No need to interpolated red and blue.
		//

		rba = rb1;
	}

	//
	// Green and alpha.
	//
	
	ga1 = colour1 >> 8;
	ga2 = colour2 >> 8;

	ga1 &= 0x00ff00ff;
	ga2 &= 0x00ff00ff;

	if (ga1 != ga2)
	{
		//
		// Do interpolationg of red and blue simultaneously.
		//

		dga  = ga2 - ga1;
		dga *= vfixed8;
		gaa  = (ga1 << 8) + dga;
		gaa &= 0xff00ff00;
	}
	else
	{
		//
		// No need to interpolate green and alpha.
		//

		gaa = ga1 << 8;
	}

	answer = rba | gaa;

	return answer;
}


//
// Creates an interpolated point for the CLIP module.
// 

void POLY_clip_interpolate_transformed(
		void *void_np,
		void *void_p1,
		void *void_p2,
		float along)
{
	float np_uoz;
	float np_voz;

	float p1_uoz;
	float p1_voz;

	float p2_uoz;
	float p2_voz;

	float z;

	POLY_Point *np;
	POLY_Point *p1;
	POLY_Point *p2;

	np = (POLY_Point *) void_np;
	p1 = (POLY_Point *) void_p1;
	p2 = (POLY_Point *) void_p2;

	np->X = p1->X + along * (p2->X - p1->X);
	np->Y = p1->Y + along * (p2->Y - p1->Y);
	np->Z = p1->Z + along * (p2->Z - p1->Z);

	p1_uoz = p1->u * p1->Z;
	p1_voz = p1->v * p1->Z;
	
	p2_uoz = p2->u * p2->Z;
	p2_voz = p2->v * p2->Z;
	
	np_uoz = p1_uoz + along * (p2_uoz - p1_uoz);
	np_voz = p1_voz + along * (p2_voz - p1_voz);

	z = 1.0F / np->Z;

	np->u = np_uoz * z;
	np->v = np_voz * z;

	np->colour = POLY_interpolate_colour(along, p1->colour, p2->colour);
}

float POLY_signed_dist_left  (void *pp) {return          ((POLY_Point *) pp)->X;}
float POLY_signed_dist_right (void *pp) {return 640.0F - ((POLY_Point *) pp)->X;}
float POLY_signed_dist_top   (void *pp) {return          ((POLY_Point *) pp)->Y;}
float POLY_signed_dist_bottom(void *pp) {return 480.0F - ((POLY_Point *) pp)->Y;}

void POLY_clip_to_screen(
		POLY_Point ***polygon,
		SLONG        *num_points,
		UBYTE         clip_or)
{
	if (clip_or & POLY_CLIP_LEFT)
	{
		CLIP_do(
			(void ***) polygon,
		    num_points,
			sizeof(POLY_Point),
			POLY_clip_interpolate_transformed,
			POLY_signed_dist_left);
	}

	if (clip_or & POLY_CLIP_RIGHT)
	{
		CLIP_do(
			(void ***) polygon,
		    num_points,
			sizeof(POLY_Point),
			POLY_clip_interpolate_transformed,
			POLY_signed_dist_right);
	}

	if (clip_or & POLY_CLIP_TOP)
	{
		CLIP_do(
			(void ***) polygon,
		    num_points,
			sizeof(POLY_Point),
			POLY_clip_interpolate_transformed,
			POLY_signed_dist_top);
	}

	if (clip_or & POLY_CLIP_BOTTOM)
	{
		CLIP_do(
			(void ***) polygon,
		    num_points,
			sizeof(POLY_Point),
			POLY_clip_interpolate_transformed,
			POLY_signed_dist_bottom);
	}
}

void POLY_camera_set(
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float view_dist,
		float lens)
{
	POLY_cam_x = x;
	POLY_cam_y = y;
	POLY_cam_z = z;

	POLY_cam_lens           = lens;
	POLY_cam_view_dist      = view_dist;
	POLY_cam_over_view_dist = 1.0F / view_dist;
	POLY_cam_aspect         = POLY_screen_height / POLY_screen_width;

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


void POLY_transform(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt)
{
	pt->x = world_x - POLY_cam_x;
	pt->y = world_y - POLY_cam_y;
	pt->z = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		pt->x,
		pt->y,
		pt->z);

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

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

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

//
// The combined rotation matrix.
//

float POLY_cam_matrix_comb[9];
float POLY_cam_off_x;
float POLY_cam_off_y;
float POLY_cam_off_z;

void POLY_set_local_rotation(
		float off_x,
		float off_y,
		float off_z,
		float matrix[9])
{
	POLY_cam_off_x = off_x - POLY_cam_x;
	POLY_cam_off_y = off_y - POLY_cam_y;
	POLY_cam_off_z = off_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		POLY_cam_off_x,
		POLY_cam_off_y,
		POLY_cam_off_z);

	MATRIX_3x3mul(
		POLY_cam_matrix_comb,
		POLY_cam_matrix,
		matrix);
}

void POLY_transform_using_local_rotation(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt)
{
	pt->x = local_x;
	pt->y = local_y;
	pt->z = local_z;

	MATRIX_MUL(
		POLY_cam_matrix_comb,
		pt->x,
		pt->y,
		pt->z);

	pt->x += POLY_cam_off_x;
	pt->y += POLY_cam_off_y;
	pt->z += POLY_cam_off_z;

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

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

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

SLONG POLY_sphere_visible(
		float world_x,
		float world_y,
		float world_z,
		float radius)
{
	float view_x;
	float view_y;
	float view_z;

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

	for (i = 0; i < POLY_buffer_upto; i++)
	{
		POLY_fadeout_point(&POLY_buffer[i]);
	}
}

SLONG POLY_valid_triangle(POLY_Point *pp[3])
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

	return TRUE;
}

SLONG POLY_valid_quad(POLY_Point *pp[4])
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
	float vx1;
	float vy1;

	float vx2;
	float vy2;

	float cprod;

	ASSERT((pp1->clip & pp2->clip & pp3->clip) & POLY_CLIP_TRANSFORMED);

	vx1 = pp2->X - pp1->X;
	vy1 = pp2->Y - pp1->Y;

	vx2 = pp3->X - pp1->X;
	vy2 = pp3->Y - pp1->Y;

	cprod = vx1*vy2 - vy1*vx2;

	return (cprod < 0);
}

//
// The z value for when we use detail textures instead of LOD textures.
//

#define POLY_Z_DETAIL (0.25F)

void POLY_add_triangle(POLY_Point *pp[3], SLONG page, SLONG backface_cull)
{
	SLONG i;

	POLY_GVertex gv[8];

	//
	// All the points must be transformed.
	//

	ASSERT((pp[0]->clip & pp[1]->clip & pp[2]->clip) & POLY_CLIP_TRANSFORMED);

	if (backface_cull && POLY_backface_cull(pp[0], pp[1], pp[2]))
	{
		//
		// This triangle is backface culled.
		//

		return;
	}

	#ifndef NDEBUG

	#define MAX_OFFSCREEN 1024.0F
	#define MAX_UV		  1024.0F

	for (i = 0; i < 3; i++)
	{
		if (pp[i]->X < -MAX_OFFSCREEN          ||
			pp[i]->X > +MAX_OFFSCREEN + 480.0F ||
			pp[i]->Y < -MAX_OFFSCREEN          ||
			pp[i]->Y > +MAX_OFFSCREEN + 640.0F)
		{
			return;
		}

		if (pp[i]->u < -MAX_UV ||
			pp[i]->u > +MAX_UV ||
			pp[i]->v < -MAX_UV ||
			pp[i]->v > +MAX_UV)
		{
			TRACE("Out of range UV\n");

			return;
		}

		if (!WITHIN(pp[i]->Z, 1.0F / 65536.0F, 1.0F))
		{
			TRACE("Out of range rhw\n");

			return;
		}
	}

	#endif

	if (pp[0]->z < POLY_Z_DETAIL)
	{
		TEXTURE_set_page(page, TEXTURE_MULTI_DETAIL);
	}
	else
	{
		TEXTURE_set_page(page, TEXTURE_MULTI_TRILINEAR);
	}

	//
	// Clip the triangle?
	//

	POLY_Point **poly      = pp;
	SLONG        poly_size = 3;
	UBYTE        clip_or   = pp[0]->clip | pp[1]->clip | pp[2]->clip;

	if (clip_or)
	{
		POLY_clip_to_screen(
			&poly,
			&poly_size,
			 clip_or);
	}

	if (poly_size == 0)
	{
		return;
	}

	//
	// Copy the points into the Glide Vertices.
	//

	for (i = 0; i < poly_size; i++)
	{
		gv[i].sx     = poly[i]->X;
		gv[i].sy     = poly[i]->Y;
		gv[i].rhw    = poly[i]->Z;
		gv[i].colour = poly[i]->colour;
		gv[i].uow0   = poly[i]->Z * poly[i]->u;
		gv[i].vow0   = poly[i]->Z * poly[i]->v;
		gv[i].rhw0   = poly[i]->Z;
		gv[i].uow1   = gv[i].uow0;
		gv[i].vow1   = gv[i].vow0;
		gv[i].rhw1   = gv[i].rhw0;
	}

	//
	// Draw the triangle.
	//

	grDrawVertexArrayContiguous(
		GR_TRIANGLE_FAN,
		poly_size,
		gv,
		sizeof(POLY_GVertex));
}

void POLY_add_quad(POLY_Point *pp[4], SLONG page, SLONG backface_cull)
{
	SLONG i;

	POLY_GVertex gv[8];

	//
	// All the points must be transformed.
	//

	ASSERT((pp[0]->clip & pp[1]->clip & pp[2]->clip) & POLY_CLIP_TRANSFORMED);

	if (backface_cull)
	{
		SLONG first;
		SLONG second;

		first  = POLY_backface_cull(pp[0], pp[1], pp[2]);
		second = POLY_backface_cull(pp[1], pp[3], pp[2]);

		if (first && second)
		{
			//
			// Both triangles of the quad are backface culled.
			//

			return;
		}

		if (first)
		{
			//
			// Just draw the second triangle.
			//

			POLY_add_triangle(pp + 1, page, FALSE);

			return;
		}

		if (second)
		{
			//
			// Just draw the first triangle.
			//

			POLY_add_triangle(pp + 0, page, FALSE);

			return;
		}
	}

	#ifndef NDEBUG

	for (i = 0; i < 4; i++)
	{
		if (pp[i]->X < -MAX_OFFSCREEN ||
			pp[i]->X > +MAX_OFFSCREEN + 480.0F ||
			pp[i]->Y < -MAX_OFFSCREEN ||
			pp[i]->Y > +MAX_OFFSCREEN + 640.0F)
		{
			return;
		}

		if (pp[i]->u < -MAX_UV ||
			pp[i]->u > +MAX_UV ||
			pp[i]->v < -MAX_UV ||
			pp[i]->v > +MAX_UV)
		{
			TRACE("Out of range UV\n");

			return;
		}

		if (!WITHIN(pp[i]->Z, 1.0F / 65536.0F, 1.0F))
		{
			TRACE("Out of range rhw\n");

			return;
		}
	}

	#endif

	if (pp[0]->z < POLY_Z_DETAIL)
	{
		TEXTURE_set_page(page, TEXTURE_MULTI_DETAIL);
	}
	else
	{
		TEXTURE_set_page(page, TEXTURE_MULTI_TRILINEAR);
	}

	//
	// These points are not in clockwise order!
	//

	{
		POLY_Point *spare;
		
		spare = pp[2];
		pp[2] = pp[3];
		pp[3] = spare;
	}

	//
	// Clip the quad?
	//

	POLY_Point **poly      = pp;
	SLONG        poly_size = 4;
	UBYTE        clip_or   = pp[0]->clip | pp[1]->clip | pp[2]->clip | pp[3]->clip;

	if (clip_or)
	{
		POLY_clip_to_screen(
			&poly,
			&poly_size,
			 clip_or);
	}

	if (poly_size == 0)
	{
		return;
	}

	//
	// Copy the points into our vertex buffer.
	//

	ASSERT(WITHIN(poly_size, 3, 8));

	for (i = 0; i < poly_size; i++)
	{
		gv[i].sx     = poly[i]->X;
		gv[i].sy     = poly[i]->Y;
		gv[i].rhw    = poly[i]->Z;
		gv[i].colour = poly[i]->colour;
		gv[i].uow0   = poly[i]->Z * poly[i]->u;
		gv[i].vow0   = poly[i]->Z * poly[i]->v;
		gv[i].rhw0   = poly[i]->Z;
		gv[i].uow1   = gv[i].uow0;
		gv[i].vow1   = gv[i].vow0;
		gv[i].rhw1   = gv[i].rhw0;
	}

	grDrawVertexArrayContiguous(
		GR_TRIANGLE_FAN,
		poly_size,
		gv,
		sizeof(POLY_GVertex));
}

void POLY_add_line(POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG sort_to_front)
{
	#ifdef WORRY_ABOUT_THIS_LATER

	float dx;
	float dy;

	float dx1;
	float dy1;

	float dx2;
	float dy2;

	float vw1;
	float vw2;

	float sw1;
	float sw2;

	float len;
	float overlen;

	POLY_GVertex gv[4];

	//
	// Both points must be transformed
	//

	ASSERT((p1->clip & p2->clip) & POLY_CLIP_TRANSFORMED);

	pa = &POLY_page[POLY_PAGE_COLOUR];

	if (pa->quad_upto + 4 > pa->size_quad)
	{
		if (!POLY_grow_page_quad(pa))
		{
			return;
		}
	}

	dx = p2->X - p1->X;
	dy = p2->Y - p1->Y;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	len     = (fabs(dx) > fabs(dy)) ? fabs(dx) + 0.414F * fabs(dy) : fabs(dy) + 0.414F * fabs(dx);
	overlen = 1.0F / len;
	
	dx *= overlen;
	dy *= overlen;

	//
	// Convert widths in the world to widths in view space.
	//

	vw1  = width1 * POLY_cam_over_view_dist;
	vw2  = width2 * POLY_cam_over_view_dist;

	//
	// Convert widths in view space to widths on screen.
	//

	sw1 = POLY_screen_mul_x * vw1 * p1->Z;
	sw2 = POLY_screen_mul_x * vw2 * p2->Z;

	dx1 = -dy * sw1;
	dy1 = +dx * sw1;

	dx2 = -dy * sw2;
	dy2 = +dx * sw2;

	if (sort_to_front)
	{
		p1->Z = 1.0F;
		p1->z = 0.0F;

		p2->Z = 1.0F;
		p2->z = 0.0F;
	}

	//
	// Create the four points.
	//

	gv0 = &pa->quad[pa->quad_upto + 0];
	gv1 = &pa->quad[pa->quad_upto + 1];
	gv2 = &pa->quad[pa->quad_upto + 2];
	gv3 = &pa->quad[pa->quad_upto + 3];

	gv0->sx     = p1->X + dx1;
	gv0->sy     = p1->Y + dy1;
	gv0->rhw    = p1->Z;
	gv0->colour = p1->colour;

	gv1->sx     = p1->X - dx1;
	gv1->sy     = p1->Y - dy1;
	gv1->rhw    = p1->Z;
	gv1->colour = p1->colour;

	gv2->sx     = p2->X + dx2;
	gv2->sy     = p2->Y + dy2;
	gv2->rhw    = p2->Z;
	gv2->colour = p2->colour;

	gv3->sx     = p2->X - dx2;
	gv3->sy     = p2->Y - dy2;
	gv3->rhw    = p2->Z;
	gv3->colour = p2->colour;

	pa->quad_upto += 4;

	#endif
}

void  POLY_add_line_2d(float sx1, float sy1, float sx2, float sy2, ULONG colour)
{
	#ifdef WORRY_ABOUT_THIS_LATER

	float dx;
	float dy;

	float len;
	float overlen;

	POLY_Page *pa;

	POLY_GVertex *gv0;
	POLY_GVertex *gv1;
	POLY_GVertex *gv2;
	POLY_GVertex *gv3;

	pa = &POLY_page[POLY_PAGE_COLOUR];

	if (pa->quad_upto + 4 > pa->size_quad)
	{
		if (!POLY_grow_page_quad(pa))
		{
			return;
		}
	}

	dx = sx2 - sx1;
	dy = sy2 - sy1;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	len     = (fabs(dx) > fabs(dy)) ? fabs(dx) + 0.414F * fabs(dy) : fabs(dy) + 0.414F * fabs(dx);
	overlen = 0.5F / len;
	
	dx *= overlen;
	dy *= overlen;

	//
	// Create the four points.
	//

	gv0 = &pa->quad[pa->quad_upto + 0];
	gv1 = &pa->quad[pa->quad_upto + 1];
	gv2 = &pa->quad[pa->quad_upto + 2];
	gv3 = &pa->quad[pa->quad_upto + 3];

	gv0->sx     = sx1 + dy;
	gv0->sy     = sy1 - dx;
	gv0->rhw    = 1.0F;
	gv0->colour = colour;

	gv1->sx     = sx1 - dy;
	gv1->sy     = sy1 + dx;
	gv1->rhw    = 1.0F;
	gv1->colour = colour;

	gv2->sx     = sx2 + dy;
	gv2->sy     = sy2 - dx;
	gv2->rhw    = 1.0F;
	gv2->colour = colour;

	gv3->sx     = sx2 - dy;
	gv3->sy     = sy2 + dx;
	gv3->rhw    = 1.0F;
	gv3->colour = colour;

	pa->quad_upto += 4;

	#endif
}


float POLY_clip_left;
float POLY_clip_right;
float POLY_clip_top;
float POLY_clip_bottom;

void POLY_clip_line_box(float sx1, float sy1, float sx2, float sy2)
{
	POLY_clip_left	 = sx1;
	POLY_clip_right	 = sx2;
	POLY_clip_top	 = sy1;
	POLY_clip_bottom = sy2;
}

void POLY_clip_line_add(float sx1, float sy1, float sx2, float sy2, ULONG colour)
{
	UBYTE clip1 = 0;
	UBYTE clip2 = 0;
	UBYTE clip_and;
	UBYTE clip_or;
	UBYTE clip_xor;

	float along;

	     if (sx1 < POLY_clip_left)   {clip1 |= POLY_CLIP_LEFT;}
	else if (sx1 > POLY_clip_right)  {clip1 |= POLY_CLIP_RIGHT;}

	     if (sx2 < POLY_clip_left)   {clip2 |= POLY_CLIP_LEFT;}
	else if (sx2 > POLY_clip_right)  {clip2 |= POLY_CLIP_RIGHT;}

	     if (sy1 < POLY_clip_top)    {clip1 |= POLY_CLIP_TOP;}
	else if (sy1 > POLY_clip_bottom) {clip1 |= POLY_CLIP_BOTTOM;}

	     if (sy2 < POLY_clip_top)    {clip2 |= POLY_CLIP_TOP;}
	else if (sy2 > POLY_clip_bottom) {clip2 |= POLY_CLIP_BOTTOM;}

	clip_and = clip1 & clip2;
	clip_or  = clip1 | clip2;
	clip_xor = clip1 ^ clip2;

	if (clip_and)
	{
		//
		// Reject the line.
		//

		return;
	}

	#define SWAP_UB(q,w) {UBYTE temp = (q); (q) = (w); (w) = temp;}

	if (clip_or)
	{
		//
		// The line needs clipping.
		//

		if (clip_xor & POLY_CLIP_LEFT)
		{
			if (clip2 & POLY_CLIP_LEFT)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_left - sx1) / (sx2 - sx1);
			sx1    =  POLY_clip_left;
			sy1    =  sy1 + along * (sy2 - sy1);
			clip1 &= ~POLY_CLIP_LEFT;

				 if (sy1 < POLY_clip_top)    {clip1 |= POLY_CLIP_TOP;}
			else if (sy1 > POLY_clip_bottom) {clip1 |= POLY_CLIP_BOTTOM;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}

		if (clip_xor & POLY_CLIP_RIGHT)
		{
			if (clip2 & POLY_CLIP_RIGHT)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_right - sx1) / (sx2 - sx1);
			sx1    =  POLY_clip_right;
			sy1    =  sy1 + along * (sy2 - sy1);
			clip1 &= ~POLY_CLIP_RIGHT;

				 if (sy1 < POLY_clip_top)    {clip1 |= POLY_CLIP_TOP;}
			else if (sy1 > POLY_clip_bottom) {clip1 |= POLY_CLIP_BOTTOM;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}

		if (clip_xor & POLY_CLIP_TOP)
		{
			if (clip2 & POLY_CLIP_TOP)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_top - sy1) / (sy2 - sy1);
			sx1    =  sx1 + along * (sx2 - sx1);
			sy1    =  POLY_clip_top;
			clip1 &= ~POLY_CLIP_TOP;

				 if (sx1 < POLY_clip_left)   {clip1 |= POLY_CLIP_LEFT;}
			else if (sx1 > POLY_clip_right)  {clip1 |= POLY_CLIP_RIGHT;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}

		if (clip_xor & POLY_CLIP_BOTTOM)
		{
			if (clip2 & POLY_CLIP_BOTTOM)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_bottom - sy1) / (sy2 - sy1);
			sx1    =  sx1 + along * (sx2 - sx1);
			sy1    =  POLY_clip_bottom;
			clip1 &= ~POLY_CLIP_BOTTOM;

				 if (sx1 < POLY_clip_left)   {clip1 |= POLY_CLIP_LEFT;}
			else if (sx1 > POLY_clip_right)  {clip1 |= POLY_CLIP_RIGHT;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}
	}

	if (clip1 | clip2)
	{
		return;
	}

	//
	// Phew! Add the clipped line.
	//

	POLY_add_line_2d(sx1, sy1, sx2, sy2, colour);
}











