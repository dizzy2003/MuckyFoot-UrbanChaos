//
// Drawing polygons with D3D
//

#ifndef _POLY_
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
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float view_dist,	// The maximum distance a point should be from the camera.
		float lens);		// Normally around 1.5F... the higher it is the more zoom you get.

//
// Given three points in world space, this function fills in
// the 3d view space coordinates of the point.  If the point
// is not 'behind' the camera, then it is transformed.  If it
// is transformed then the 2d points and clipping flags are
// calculated.
//

#define POLY_CLIP_LEFT			(1 << 0)
#define POLY_CLIP_RIGHT			(1 << 1)
#define POLY_CLIP_TOP			(1 << 2)
#define POLY_CLIP_BOTTOM		(1 << 3)
#define POLY_CLIP_TRANSFORMED	(1 << 4)
#define POLY_CLIP_FAR			(1 << 5)	// View space Z too far away
#define POLY_CLIP_NEAR			(1 << 6)	// View space Z too near

typedef struct
{
	float x;	//              
	float y;	// 3D points... 
	float z;	//              

	float X;	//             
	float Y;	// 2D points...
	float Z;	//             

	ULONG clip;

	float u;
	float v;
	ULONG colour;	// SSRRGGBB SS is specular lighting.
	
} POLY_Point;

void POLY_transform(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt);

//
// Sets an additional matrix to be applied to the world point before
// it is transformed.
//

void POLY_set_local_rotation(
		float off_x,
		float off_y,
		float off_z,
		float matrix[9]);

void POLY_transform_using_local_rotation(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt);

//
// Returns true if the given sphere in world space is visible
// on screen. 'radius' should be given as a fraction of the viewdist passed
// to POLY_camera_set.  radius = world_radius / view_dist.
//

SLONG POLY_sphere_visible(
		float world_x,
		float world_y,
		float world_z,
		float radius);

//
// Handy buffers for rotating objects
//

#define POLY_BUFFER_SIZE 8192
#define POLY_SHADOW_SIZE 8192

extern POLY_Point POLY_buffer[POLY_BUFFER_SIZE];
extern SLONG      POLY_buffer_upto;

extern POLY_Point POLY_shadow[POLY_BUFFER_SIZE];
extern SLONG      POLY_shadow_upto;

//
// The fade-out range of the points.
//

#define POLY_FADEOUT_START	(0.50F)
#define POLY_FADEOUT_END	(1.00F)

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
		SLONG alpha;

		multi = 256 - SLONG((pp->z - POLY_FADEOUT_START) * (256.0F / (POLY_FADEOUT_END - POLY_FADEOUT_START)));

		if (multi < 0)
		{
			pp->colour  = 0;
			pp->clip   |= POLY_CLIP_FAR;
		}
		else
		{
			alpha  = ((pp->colour >> 24) & 0xff) * multi >> 8;
			red    = ((pp->colour >> 16) & 0xff) * multi >> 8;
			green  = ((pp->colour >>  8) & 0xff) * multi >> 8;
			blue   = ((pp->colour >>  0) & 0xff) * multi >> 8;

			pp->colour &= 0x00000000;
			pp->colour |= alpha << 24;
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
// The page argument is one of the TEXTURE module's pages.
// 

SLONG POLY_valid_triangle(POLY_Point *p[3]);
SLONG POLY_valid_quad    (POLY_Point *p[4]);
SLONG POLY_valid_line    (POLY_Point *p1, POLY_Point *p2);
void  POLY_add_triangle  (POLY_Point *p[3], SLONG page, SLONG shall_i_backface_cull);
void  POLY_add_quad      (POLY_Point *p[4], SLONG page, SLONG shall_i_backface_cull);
void  POLY_add_line      (POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG sort_to_front);
void  POLY_add_line_2d   (float sx1, float sy1, float sx2, float sy2, ULONG colour);
void  POLY_clip_line_box (float sx1, float sy1, float sx2, float sy2);
void  POLY_clip_line_add (float sx1, float sy1, float sx2, float sy2, ULONG colour);

#endif
