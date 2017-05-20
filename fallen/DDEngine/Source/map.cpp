//
// The new map screen
//



#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "game.h"
#include "poly.h"
#include "text.h"
#include "texture.h"
#include "mav.h"
#include "menufont.h"
#include "matrix.h"
#include "memory.h"
#include "fc.h"
#include "font2d.h"


#ifdef TARGET_DC
#include "target.h"
#endif

extern	CBYTE	*EWAY_get_mess(SLONG index);




#ifndef TARGET_DC


//
// The size of the physical screen.
//

float MAP_screen_size_x;
float MAP_screen_size_y;


//
// The current mapping from the world to the screen.
//

float MAP_screen_x = 0.60F;
float MAP_screen_y = 0.55F;
float MAP_world_x;
float MAP_world_z;
float MAP_scale_x = 0.03F;			// Virtual p0ixels per mapsquare 
float MAP_scale_y = 0.03F * 1.33F;



//
// Returns the screen coord of the given (floating point!) world position.
//

#define MAP_SCREEN_X(world_x) (MAP_screen_x + ((world_x) - MAP_world_x) * MAP_scale_x)
#define MAP_SCREEN_Y(world_z) (MAP_screen_y + ((world_z) - MAP_world_z) * MAP_scale_y)

#define MAP_SCREEN_IX(world_x) (MAP_screen_x + (((float)world_x)*(1.0f/256.0f) - MAP_world_x) * MAP_scale_x)
#define MAP_SCREEN_IY(world_z) (MAP_screen_y + (((float)world_z)*(1.0f/256.0f) - MAP_world_z) * MAP_scale_y)

//
// Returns the world position of the given point on the screen.
//

#define MAP_WORLD_X(screen_x) (((screen_x) - MAP_screen_x) / MAP_scale_x + MAP_world_x)
#define MAP_WORLD_Z(screen_y) (((screen_y) - MAP_screen_y) / MAP_scale_y + MAP_world_z)




//
// Returns a shade of grey depending on its virtual screen position.
//

ULONG MAP_fadeout_colour(
		float sx,
		float sy)
{
	float dx = (sx - MAP_screen_x) * (1.0F / 0.38F);
	float dy = (sy - MAP_screen_y) * (1.0F / 0.48F);
	float dist;
	float fmul;
	SLONG imul;
	ULONG colour;

	dist  = dx*dx + dy*dy;
	dist *= dist;
	fmul  = 1.0F - dist;

	imul = SLONG(fmul * 255.0F);

	if (imul < 0)
	{
		imul = 0;
	}

	colour  = imul | (imul << 8);
	colour |= colour << 16;

	return colour;
}


//
// Draws a prim.
//

void MAP_draw_prim(
		SLONG prim,
		float cx,
		float cy,
		float yaw,
		float scale)
{
	SLONG i;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	SLONG page;

	float x;
	float y;
	float z;

	float X;
	float Y;
	float Z;

	float matrix[9];

	PrimObject *po;
	POLY_Point *pp;

	PrimFace4 *p_f4;
	PrimFace3 *p_f3;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	MATRIX_calc(
		matrix,
		yaw + prim,
	   -0.5F,
		0.0F);

	//
	// Assume maximum dimensions of 1 mapsquare...
	// 

	matrix[0] *= 1.0F / 256.0F;
	matrix[1] *= 1.0F / 256.0F;
	matrix[2] *= 1.0F / 256.0F;
	matrix[3] *= 1.0F / 256.0F;
	matrix[4] *= 1.0F / 256.0F;
	matrix[5] *= 1.0F / 256.0F;
	matrix[6] *= 1.0F / 256.0F;
	matrix[7] *= 1.0F / 256.0F;
	matrix[8] *= 1.0F / 256.0F;

	//
	// Aspect ratio...
	// 
	
	matrix[3] *= 1.0F / 1.33F;
	matrix[4] *= 1.0F / 1.33F;
	matrix[5] *= 1.0F / 1.33F;

	//
	// Rotate the points.
	// 

	po = &prim_objects[prim];
	pp = &POLY_buffer [0];

	for (i = po->StartPoint; i < po->EndPoint; i++)
	{
		x = float(prim_points[i].X);
		y = float(prim_points[i].Y);
		z = float(prim_points[i].Z);

		MATRIX_MUL(
			matrix,
			x,
			y,
			z);

		z += 1.0F;

		ASSERT(z > 0.001F);

		Z  = scale / z;
		X  = x * Z;
		Y  = y * Z;
		X  = cx - X;
		Y  = cy - Y;

		X *= MAP_screen_size_x;
		Y *= MAP_screen_size_y;

		pp->x        = x;
		pp->y        = y;
		pp->X        = X;
		pp->Y        = Y;
		pp->Z        = Z;
		pp->z        = z;
#ifdef TARGET_DC
		pp->colour   = 0xffffffff;
#else
		pp->colour   = 0x00ffffff;
#endif
		pp->specular = 0xff000000;

		POLY_setclip(pp);

		pp += 1;
	}

	POLY_buffer_upto = po->EndPoint - po->StartPoint;

	//
	// The quads.
	//

	for (i = po->StartFace4; i < po->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - po->StartPoint;
		p1 = p_f4->Points[1] - po->StartPoint;
		p2 = p_f4->Points[2] - po->StartPoint;
		p3 = p_f4->Points[3] - po->StartPoint;
		
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

			POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
		}
	}

	//
	// The triangles.
	//

	for (i = po->StartFace3; i < po->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - po->StartPoint;
		p1 = p_f3->Points[1] - po->StartPoint;
		p2 = p_f3->Points[2] - po->StartPoint;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

		tri[0] = &POLY_buffer[p0];
		tri[1] = &POLY_buffer[p1];
		tri[2] = &POLY_buffer[p2];

		if (POLY_valid_triangle(tri))
		{
			tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
			tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												   
			tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
			tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												   
			tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
			tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

			page   = p_f3->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= p_f3->TexturePage;

			POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
		}
	}
}



//
// Draws a sprite
//

void MAP_sprite(
		SLONG page,
		float x,
		float y,
		float size_x,
		float size_y,
		float u0,
		float v0,
		float u1,
		float v1,
		float u2,
		float v2,
		float u3,
		float v3,
		UBYTE shadow)
{
	float x1 = x;
	float y1 = y;
	float x2 = x + size_x;
	float y2 = y + size_y;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	ULONG colour0 = MAP_fadeout_colour(x1,y1);
	ULONG colour1 = MAP_fadeout_colour(x2,y1);
	ULONG colour2 = MAP_fadeout_colour(x1,y2);
	ULONG colour3 = MAP_fadeout_colour(x2,y2);
	
	//
	// Convert from virtual to real screen coords.
	// 

	x1 *= MAP_screen_size_x;
	y1 *= MAP_screen_size_y;
	x2 *= MAP_screen_size_x;
	y2 *= MAP_screen_size_y;

	pp[0].X        = x1;
	pp[0].Y        = y1;
	pp[0].z        = 0.9F;
	pp[0].Z        = 0.1F;
	pp[0].u        = u0;
	pp[0].v        = v0;
	pp[0].colour   = colour0;
	pp[0].specular = 0xff000000;

	pp[1].X        = x2;
	pp[1].Y        = y1;
	pp[1].z        = 0.9F;
	pp[1].Z        = 0.1F;
	pp[1].u        = u1;
	pp[1].v        = v1;
	pp[1].colour   = colour1;
	pp[1].specular = 0xff000000;

	pp[2].X        = x1;
	pp[2].Y        = y2;
	pp[2].z        = 0.9F;
	pp[2].Z        = 0.1F;
	pp[2].u        = u2;
	pp[2].v        = v2;
	pp[2].colour   = colour2;
	pp[2].specular = 0xff000000;

	pp[3].X        = x2;
	pp[3].Y        = y2;
	pp[3].z        = 0.9F;
	pp[3].Z        = 0.1F;
	pp[3].u        = u3;
	pp[3].v        = v3;
	pp[3].colour   = colour3;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	if (shadow)
	{
		SLONG i; 

		POLY_Point  ps [4];
		POLY_Point *tri[3];

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
			ps[i].colour >>= 1;
			ps[i].colour  &= 0x7f7f7f7f;
		}

		switch(shadow)
		{
			case 0:
				ASSERT(0);	// We shouldn't be doing any of this in this case.
				break;

			case 1:

				tri[0] = &ps [0];
				tri[1] = quad[1];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = quad[3];
				tri[2] = quad[2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;
				
			case 2:

				tri[0] = &ps [0];
				tri[1] = quad[1];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = quad[3];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;
				
			case 3:

				//ps[2].colour += 0x00101010;

				tri[0] = quad[0];
				tri[1] = quad[1];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = quad[3];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;
				
			case 4:

				tri[0] = quad[0];
				tri[1] = quad[1];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = &ps [3];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;
				
			case 5:

				tri[0] = &ps [0];
				tri[1] = quad[1];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = &ps [3];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;
				
			case 6:

				tri[0] = &ps [0];
				tri[1] = quad[1];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = quad[3];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;
				
			case 7:

				tri[0] = quad[0];
				tri[1] = quad[1];
				tri[2] = quad[2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				tri[0] = quad[1];
				tri[1] = &ps [3];
				tri[2] = &ps [2];

				POLY_add_triangle(tri, page, FALSE, TRUE);

				break;

			default:
				ASSERT(0);
				break;
		}
	}
	else
	{
		POLY_add_quad(quad, page, FALSE, TRUE);
	}
}


//
// Draws a line.
//

void MAP_draw_line(
		float x1,
		float y1,
		float x2,
		float y2,
		ULONG colour)
{	
	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	float dx = fabs(x2 - x1);
	float dy = fabs(y2 - y1);

	SLONG colour0 = MAP_fadeout_colour(x1,y1) & colour;
	SLONG colour1 = MAP_fadeout_colour(x2,y2) & colour;
	
	if (!(colour0 | colour1))
	{
		return;
	}

	//
	// Convert to screen coords.
	//

	x1 *= MAP_screen_size_x;
	y1 *= MAP_screen_size_y;
	x2 *= MAP_screen_size_x;
	y2 *= MAP_screen_size_y;

	//
	// Setup points
	//

	pp[0].z        = 0.9F;
	pp[0].Z        = 0.1F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = colour0;
	pp[0].specular = 0xff000000;

	pp[1].z        = 0.9F;
	pp[1].Z        = 0.1F;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = colour0;
	pp[1].specular = 0xff000000;

	pp[2].z        = 0.9F;
	pp[2].Z        = 0.1F;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = colour1;
	pp[2].specular = 0xff000000;

	pp[3].z        = 0.9F;
	pp[3].Z        = 0.1F;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[3].colour   = colour1;
	pp[3].specular = 0xff000000;

	if (dx > dy)
	{
		pp[0].X = x1;
		pp[0].Y = y1 - 1.0F;

		pp[1].X = x1;
		pp[1].Y = y1 + 1.0F;

		pp[2].X = x2;
		pp[2].Y = y2 - 1.0F;

		pp[3].X = x2;
		pp[3].Y = y2 + 1.0F;
	}
	else
	{
		pp[0].X = x1 - 1.0F;
		pp[0].Y = y1;

		pp[1].X = x1 + 1.0F;
		pp[1].Y = y1;

		pp[2].X = x2 - 1.0F;
		pp[2].Y = y2;

		pp[3].X = x2 + 1.0F;
		pp[3].Y = y2;
	}

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE, TRUE);
}


//
// Draws an orientated alpha dot with the given colour.
//

void MAP_draw_dot(
		float x,
		float y,
		float size,
		float angle,
		ULONG colour)
{
	SLONG mul;

	float dx = size * -(float)sin(angle);
	float dy = size * -(float)cos(angle) * 1.33F;

	POLY_Point  pp [3];
	POLY_Point *tri[3];

	mul = MAP_fadeout_colour(x,y) & 0xff;

	{
		SLONG a;
		SLONG r;
		SLONG g;
		SLONG b;

		a = (colour >> 24) & 0xff;
		r = (colour >> 16) & 0xff;
		g = (colour >> 8 ) & 0xff;
		b = (colour >> 0 ) & 0xff;

		a = a * mul >> 8;
		r = r * mul >> 8;
		g = g * mul >> 8;
		b = b * mul >> 8;

		colour  = a << 24;
		colour |= r << 16;
		colour |= g <<  8;
		colour |= b <<  0;
	}

	//
	// Convert from virtual to real screen coords.
	// 

	x  *= MAP_screen_size_x;
	y  *= MAP_screen_size_y;
	dx *= MAP_screen_size_x;
	dy *= MAP_screen_size_y;

	pp[0].X        = x - dx + dy + dy;
	pp[0].Y        = y - dy - dx - dx;
	pp[0].z        = 0.9F;
	pp[0].Z        = 0.1F;
	pp[0].u        = 141.0F / 256.0F;
	pp[0].v        = 167.0F / 256.0F;
	pp[0].colour   = colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = x - dx - dy - dy;
	pp[1].Y        = y - dy + dx + dx;
	pp[1].z        = 0.9F;
	pp[1].Z        = 0.1F;
	pp[1].u        = 141.0F / 256.0F;
	pp[1].v        = 187.0F / 256.0F;
	pp[1].colour   = colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = x + dx + dx;
	pp[2].Y        = y + dy + dy;
	pp[2].z        = 0.9F;
	pp[2].Z        = 0.1F;
	pp[2].u        = 153.0F / 256.0F;
	pp[2].v        = 177.0F / 256.0F;
	pp[2].colour   = colour;
	pp[2].specular = 0xff000000;

	tri[0] = &pp[0];
	tri[1] = &pp[1];
	tri[2] = &pp[2];

	POLY_add_triangle(tri, POLY_PAGE_IC2_ALPHA, FALSE, TRUE);
}

#endif //#ifndef TARGET_DC




//
// The pulses...
//

typedef struct
{
	SLONG life;		// 0 => unused
	ULONG colour;
	float wx;
	float wz;
	float radius;

} MAP_Pulse;

#define MAP_MAX_PULSES 16

MAP_Pulse MAP_pulse[MAP_MAX_PULSES];



//
// Initialises the pulses
//

void MAP_pulse_init()
{
	memset(MAP_pulse, 0, sizeof(MAP_pulse));
}


//
// Creates a new pulse.
//

void MAP_pulse_create(float wx, float wz, ULONG colour)
{
	SLONG i;
	SLONG best_life  = INFINITY;
	SLONG best_pulse = INFINITY;

	for (i = 0; i < MAP_MAX_PULSES; i++)
	{
		if (best_life > MAP_pulse[i].life)
		{
			best_life  = MAP_pulse[i].life;
			best_pulse = i;
		}
	}

	MAP_Pulse *mp;

	ASSERT(WITHIN(best_pulse, 0, MAP_MAX_PULSES - 1));

	mp = &MAP_pulse[best_pulse];

	mp->life   = 256;
	mp->colour = colour;
	mp->wx     = wx;
	mp->wz     = wz;
	mp->radius = 0.05F;
}


#ifndef TARGET_DC


//
// Draws an individual pulse
//

void MAP_pulse_draw(float wx, float wz, float radius, ULONG colour, UBYTE fade)
{
	float x;
	float y;

	float x1;
	float y1;
	float x2;
	float y2;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	float dx = MAP_scale_x * radius;
	float dy = MAP_scale_y * radius;

	x = MAP_SCREEN_X(wx);
	y = MAP_SCREEN_Y(wz);

	x1 = x - dx;
	y1 = y - dy;
	x2 = x + dx;
	y2 = y + dy;

	colour &= MAP_fadeout_colour(x,y);

	if (colour == 0)
	{
		//
		// Don't draw a black pulse...
		//

		return;
	}

	colour &= 0xffffff;
	colour |= fade << 24;

	//
	// Convert to screen coords.
	//

	x1 *= MAP_screen_size_x;
	y1 *= MAP_screen_size_y;
	x2 *= MAP_screen_size_x;
	y2 *= MAP_screen_size_y;

	//
	// Setup points
	//

	pp[0].X        = x1;
	pp[0].Y        = y1;
	pp[0].z        = 0.9F;
	pp[0].Z        = 0.1F;
	pp[0].u        = 160.0F / 256.0F;
	pp[0].v        = 169.0F / 256.0F;
	pp[0].colour   = colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = x2;
	pp[1].Y        = y1;
	pp[1].z        = 0.9F;
	pp[1].Z        = 0.1F;
	pp[1].u        = 207.0F / 256.0F;
	pp[1].v        = 169.0F / 256.0F;
	pp[1].colour   = colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = x1;
	pp[2].Y        = y2;
	pp[2].z        = 0.9F;
	pp[2].Z        = 0.1F;
	pp[2].u        = 160.0F / 256.0F;
	pp[2].v        = 216.0F / 256.0F;
	pp[2].colour   = colour;
	pp[2].specular = 0xff000000;

	pp[3].X        = x2;
	pp[3].Y        = y2;
	pp[3].z        = 0.9F;
	pp[3].Z        = 0.1F;
	pp[3].u        = 207.0F / 256.0F;
	pp[3].v        = 216.0F / 256.0F;
	pp[3].colour   = colour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_IC2_ALPHA_END, FALSE, TRUE);
}

//
// Draws all the pulses.
//

void MAP_pulse_draw_all()
{
	SLONG i;

	MAP_Pulse *mp;

	for (i = 0; i < MAP_MAX_PULSES; i++)
	{
		mp = &MAP_pulse[i];

		if (!mp->life)
		{
			continue;
		}

		MAP_pulse_draw(
			mp->wx,
			mp->wz,
			mp->radius,
			mp->colour,
			mp->life);
	}
}

#endif //#ifndef TARGET_DC


//
// Processes the pulses.
//

void MAP_process_pulses()
{
	SLONG i;

	MAP_Pulse *mp;

	static SLONG now = 0;
	static SLONG last = 0;

	now = GetTickCount();

	if (last < now - (1000 / 10))
	{
		last = now - (1000 / 10);
	}

	while(last < now)
	{
		//
		// Process at 20 fps...
		//

		last += (1000 / 20);

		for (i = 0; i < MAP_MAX_PULSES; i++)
		{
			mp = &MAP_pulse[i];

			if (!mp->life)
			{
				continue;
			}

			mp->life   -= 4;
			mp->radius += 0.04F;

			if (mp->life < 0)
			{
				mp->life = 0;
			}
		}
	}
}



#ifndef TARGET_DC

//
// Draws an arrow in the given direction...
//

void MAP_draw_arrow(float angle, ULONG colour)
{
	float x;
	float y;

	float dx = sin(angle);
	float dy = cos(angle);

	float dist = 0.33F + (float)sin(float(GetTickCount()) * 0.005F) * 0.02F;

	x = MAP_screen_x + dx * dist;
	y = MAP_screen_y + dy * dist * 1.33F;

	x  *= MAP_screen_size_x;
	y  *= MAP_screen_size_y;
	dx *= MAP_screen_size_x * 0.03F;
	dy *= MAP_screen_size_y * 0.03F;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	pp[0].X        = x + dx - dy;
	pp[0].Y        = y + dy + dx;
	pp[0].z        = 0.9F;
	pp[0].Z        = 0.1F;
	pp[0].u        = 151.0F / 256.0F;
	pp[0].v        = 184.0F / 256.0F;
	pp[0].colour   = colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = x + dx + dy;
	pp[1].Y        = y + dy - dx;
	pp[1].z        = 0.9F;
	pp[1].Z        = 0.1F;
	pp[1].u        = 151.0F / 256.0F;
	pp[1].v        = 197.0F / 256.0F;
	pp[1].colour   = colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = x - dx - dy;
	pp[2].Y        = y - dy + dx;
	pp[2].z        = 0.9F;
	pp[2].Z        = 0.1F;
	pp[2].u        = 141.0F / 256.0F;
	pp[2].v        = 184.0F / 256.0F;
	pp[2].colour   = colour;
	pp[2].specular = 0xff000000;

	pp[3].X        = x - dx + dy;
	pp[3].Y        = y - dy - dx;
	pp[3].z        = 0.9F;
	pp[3].Z        = 0.1F;
	pp[3].u        = 141.0F / 256.0F;
	pp[3].v        = 197.0F / 256.0F;
	pp[3].colour   = colour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_IC2_ALPHA_END, FALSE, TRUE);
}

void MAP_draw_3d_arrow(
		float angle,
		ULONG colour)
{
	SLONG i;

	float vx;
	float vy;
	float vz;

	float pangle;
	float pdist;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	colour &= 0x00ffffff;
	colour |= 0x88000000;

	pp[0].u        = 151.0F / 256.0F;
	pp[0].v        = 184.0F / 256.0F;
	pp[0].colour   = colour;
	pp[0].specular = 0xff000000;

	pp[1].u        = 151.0F / 256.0F;
	pp[1].v        = 197.0F / 256.0F;
	pp[1].colour   = colour;
	pp[1].specular = 0xff000000;

	pp[2].u        = 141.0F / 256.0F;
	pp[2].v        = 184.0F / 256.0F;
	pp[2].colour   = colour;
	pp[2].specular = 0xff000000;

	pp[3].u        = 141.0F / 256.0F;
	pp[3].v        = 197.0F / 256.0F;
	pp[3].colour   = colour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	static float dangle = 0.25F;
	static float ddist  = 0.25F;
	static float width  = 800.0F;
	static float below  = 0.5F;

	//
	// We have our own mini transformation function!
	//

	for (i = 0; i < 4; i++)
	{
		pangle = angle;
		pdist  = 1.0F;

		#define MAP_3DARROW_DANGLE (dangle)
		#define MAP_3DARROW_DDIST  (ddist)

		if (i & 1) {pangle -= MAP_3DARROW_DANGLE;} else {pangle += MAP_3DARROW_DANGLE;}
		if (i & 2) {pdist  += MAP_3DARROW_DDIST ;}

		vx = sin(pangle) * pdist;
		vy = below;
		vz = cos(pangle) * pdist;

		vz += 2.0F;

		pp[i].Z = 0.5F / vz;
		pp[i].z = 0.0F;
		pp[i].X = 320.0F + (vx * pp[i].Z * width);
		pp[i].Y = 240.0F - (vy * pp[i].Z * width);
	}

	POLY_add_quad(quad, POLY_PAGE_IC2_ALPHA_END, FALSE, TRUE);
}

#endif //#ifndef TARGET_DC





//
// The map beacon colours.
//

#define MAP_MAX_BEACON_COLOURS 6

ULONG MAP_beacon_colour[MAP_MAX_BEACON_COLOURS] =
{
	0xffff00,
	0xff0000,
	0x00ff00,
	0x0000ff,
	0xff00ff,
	0x00ffff
};


//
// Initialise the beacons.
//

void MAP_beacon_init()
{
	memset(MAP_beacon, 0, sizeof(MAP_Beacon)*MAP_MAX_BEACONS);
}

//
// Creates a beacon
//

UBYTE MAP_beacon_create(SLONG x, SLONG z, SLONG index, UWORD track_thing)
{
	SLONG i;

	MAP_Beacon *mb;

	extern	SLONG	EWAY_mess_upto;
	ASSERT(index>=0 && index<EWAY_mess_upto);

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			mb->used         = TRUE;
			mb->counter      = 0;
			mb->track_thing  = track_thing;
			mb->index          = index;
			mb->wx           = x>>0;//float(x) * (1.0F / 256.0F);
			mb->wz           = z>>0;//float(z) * (1.0F / 256.0F);
			mb->ticks        = GetTickCount();

			return i;
		}
	}

	return 0;
}

//
// Processes the beacons.
//

void MAP_process_beacons()
{
	SLONG i;

	MAP_Beacon *mb;

	static SLONG now = 0;
	static SLONG last = 0;

	now = GetTickCount();

	if (last < now - (1000 / 10))
	{
		last = now - (1000 / 10);
	}

	while(last < now)
	{
		//
		// Process at 20 fps...
		//

		last += (1000 / 20);

		for (i = 1; i < MAP_MAX_BEACONS; i++)
		{
			mb = &MAP_beacon[i];

			if (!mb->used)
			{
				continue;
			}

			if (mb->track_thing)
			{
				Thing *p_track = TO_THING(mb->track_thing);

				mb->wx = p_track->WorldPos.X>>8;
				mb->wz = p_track->WorldPos.Z>>8;
			}

			mb->counter += 1;

			if (mb->counter >= 20)
			{
				MAP_pulse_create(
				((float)	mb->wx)*(1.0f/256.0f),
					((float)mb->wz)*(1.0f/256.0f),
					MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS]);

				mb->counter = 0;
			}
		}
	}
}


#ifndef TARGET_DC


void MAP_beacon_draw_all()
{
	SLONG i;

	float x;
	float y;

	float list = 0.05F;

	float dx;
	float dy;
	float dist;
	float angle;

	SLONG colour;

	MAP_Beacon *mb;

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			continue;
		}

		x = MAP_SCREEN_IX(mb->wx);
		y = MAP_SCREEN_IY(mb->wz);

		colour = MAP_fadeout_colour(x,y);

		if ((colour & 0xff) < 64)
		{
			dist  = 

			angle = atan2(x - MAP_screen_x, y - MAP_screen_y);

			colour &= 0xff;
			colour *= 4;
			colour  = 255 - colour;

			colour = MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS] | (colour << 24);

			MAP_draw_arrow(
				angle,
				colour);
		}

		{
			float x1;
			float y1;
			float x2;
			float y2;

			float radius = 0.03F;// + sin(GetTickCount() * 0.005F) * 0.01F;

			SLONG colour;

			POLY_Point  pp  [4];
			POLY_Point *quad[4];

			x1 = 0.27F * MAP_screen_size_x;
			y1 = list  * MAP_screen_size_y;

			x2 = x1 + radius * MAP_screen_size_x;
			y2 = y1 + radius * MAP_screen_size_y;

			x1 = x1 - radius * MAP_screen_size_x;
			y1 = y1 - radius * MAP_screen_size_y;

			colour = MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS] | 0xff000000;

			//
			// Setup points
			//

			pp[0].X        = x1;
			pp[0].Y        = y1;
			pp[0].z        = 0.9F;
			pp[0].Z        = 0.1F;
			pp[0].u        = 140.0F / 256.0F;
			pp[0].v        = 197.0F / 256.0F;
			pp[0].colour   = colour;
			pp[0].specular = 0xff000000;

			pp[1].X        = x2;
			pp[1].Y        = y1;
			pp[1].z        = 0.9F;
			pp[1].Z        = 0.1F;
			pp[1].u        = 154.0F / 256.0F;
			pp[1].v        = 197.0F / 256.0F;
			pp[1].colour   = colour;
			pp[1].specular = 0xff000000;

			pp[2].X        = x1;
			pp[2].Y        = y2;
			pp[2].z        = 0.9F;
			pp[2].Z        = 0.1F;
			pp[2].u        = 140.0F / 256.0F;
			pp[2].v        = 212.0F / 256.0F;
			pp[2].colour   = colour;
			pp[2].specular = 0xff000000;

			pp[3].X        = x2;
			pp[3].Y        = y2;
			pp[3].z        = 0.9F;
			pp[3].Z        = 0.1F;
			pp[3].u        = 154.0F / 256.0F;
			pp[3].v        = 212.0F / 256.0F;
			pp[3].colour   = colour;
			pp[3].specular = 0xff000000;

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			POLY_add_quad(quad, POLY_PAGE_IC2_ALPHA_END, FALSE, TRUE);
		
			//
			// Draw the text
			// 

			MENUFONT_Draw(
				SLONG(0.3F * MAP_screen_size_x),
				SLONG(list * MAP_screen_size_y),
				128,
				EWAY_get_mess(mb->index),
				MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS] | 0xff000000,
				0);

		}

		list += 0.03F;
	}
}

#endif //#ifndef TARGET_DC


void MAP_beacon_remove(UBYTE beacon)
{
	ASSERT(WITHIN(beacon, 0, MAP_MAX_BEACONS - 1));

	MAP_beacon[beacon].used = FALSE;
}


#ifndef TARGET_DC


void MAP_draw_weapons(Thing *p_person)
{
	float x;
	float y;

	float yaw = GetTickCount() * 0.004F;

	SLONG index;

	Thing *p_special;

	x = 0.1F;
	y = 0.25F;

	if (p_person->Flags & FLAGS_HAS_GUN)
	{
		MAP_draw_prim(
			PRIM_OBJ_ITEM_GUN,
			x,
			y,
			yaw,
			0.5F);

		y += 0.1F;
	}

	for (index = p_person->Genus.Person->SpecialList; index; index = p_special->Genus.Special->NextSpecial)
	{
		yaw += 1.0F;

		p_special = TO_THING(index);

		ASSERT(p_special->Class == CLASS_SPECIAL);
		ASSERT(WITHIN(p_special->Genus.Special->SpecialType, 1, SPECIAL_NUM_TYPES - 1));

		MAP_draw_prim(
			SPECIAL_info[p_special->Genus.Special->SpecialType].prim,
			x,
			y,
			yaw,
			0.5F);

		y += 0.1F;
	}
}



void MAP_draw()
{
	float x;
	float z;

	float u0;
	float v0;
	float u1;
	float v1;
	float u2;
	float v2;
	float u3;
	float v3;
	
	float x1;
	float y1;
	float x2;
	float y2;

	SLONG mx;
	SLONG mz;
	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	SLONG i;
	SLONG page;
	ULONG colour;
	SLONG blue;
	SLONG red;
	UBYTE opt;

	float scale;
	float fx1;
	float fz1;
	float fx2;
	float fz2;
	CBYTE str[10];

	SLONG index;

	Thing *darci = NET_PERSON(0);
	Thing *p_thing;

	//
	// Clear the screen.
	//

	AENG_clear_screen();

	//
	// Clear the buffers.
	//

#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif

	//
	// The real screen size.
	//

	MAP_screen_size_x = float(DisplayWidth);
	MAP_screen_size_y = float(DisplayHeight);

	//
	// Centre the map on darci.
	//

	MAP_world_x = float(darci->WorldPos.X) * (1.0F / 65536.0F);
	MAP_world_z = float(darci->WorldPos.Z) * (1.0F / 65536.0F);

	//
	// Find the four points at the edge of the screen.
	//

	fx1 = MAP_WORLD_X(0.2F);
	fz1 = MAP_WORLD_Z(0.0F);
	fx2 = MAP_WORLD_X(1.0F);
	fz2 = MAP_WORLD_Z(1.0F);

	mx1 = SLONG(fx1);
	mz1 = SLONG(fz1);
	mx2 = SLONG(fx2);
	mz2 = SLONG(fz2);

	SATURATE(mx1, 1, PAP_SIZE_HI - 2);
	SATURATE(mz1, 1, PAP_SIZE_HI - 2);
	SATURATE(mx2, 1, PAP_SIZE_HI - 2);
	SATURATE(mz2, 1, PAP_SIZE_HI - 2);

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		TEXTURE_get_minitexturebits_uvs(
			PAP_2HI(mx,mz).Texture,
		   &page,
		   &u0,
		   &v0,
		   &u1,
		   &v1,
		   &u2,
		   &v2,
		   &u3,
		   &v3);

		MAP_sprite(
			page,
			MAP_SCREEN_X(float(mx)),
			MAP_SCREEN_Y(float(mz)),
			MAP_scale_x,
			MAP_scale_y,
			u0,
			v0,
			u1,
			v1,
			u2,
			v2,
			u3,
			v3,
			PAP_2HI(mx,mz).Flags & (PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3));

		for (i = 0; i < 4; i++)
		{
			opt = MAV_get_caps(mx,mz,i);

			colour = 0;

			if (!(opt & MAV_CAPS_GOTO      )) {colour = 0xffff0000;}
			if ( (opt & MAV_CAPS_CLIMB_OVER)) {colour = 0xffffff00;}
			if ( (opt & MAV_CAPS_LADDER_UP )) {colour = 0xff00ffff;}

			if (colour)
			{
				switch(i)
				{
					case MAV_DIR_XS:
						x1 = MAP_SCREEN_X(mx  );
						y1 = MAP_SCREEN_Y(mz  );
						x2 = MAP_SCREEN_X(mx  );
						y2 = MAP_SCREEN_Y(mz+1);
						break;

					case MAV_DIR_XL:
						x1 = MAP_SCREEN_X(mx+1);
						y1 = MAP_SCREEN_Y(mz  );
						x2 = MAP_SCREEN_X(mx+1);
						y2 = MAP_SCREEN_Y(mz+1);
						break;

					case MAV_DIR_ZS:
						x1 = MAP_SCREEN_X(mx  );
						y1 = MAP_SCREEN_Y(mz  );
						x2 = MAP_SCREEN_X(mx+1);
						y2 = MAP_SCREEN_Y(mz  );
						break;

					case MAV_DIR_ZL:
						x1 = MAP_SCREEN_X(mx  );
						y1 = MAP_SCREEN_Y(mz+1);
						x2 = MAP_SCREEN_X(mx+1);
						y2 = MAP_SCREEN_Y(mz+1);
						break;
				}

				MAP_draw_line(
					x1, y1,
					x2, y2,
					colour);
			}
		}
	}

	//
	// Draw the important things...
	//

	mx1 >>= 2;
	mz1 >>= 2;
	mx2 >>= 2;
	mz2 >>= 2;

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		index = PAP_2LO(mx,mz).MapWho;

		while(index)
		{
			p_thing = TO_THING(index);

			switch(p_thing->Class)
			{
				case CLASS_PERSON:
					
					switch(p_thing->Genus.Person->PersonType)
					{
						case PERSON_DARCI:
						case PERSON_ROPER:
							scale  = float(GetTickCount());
							scale *= 0.02F;
							scale  = sin(scale);
							scale *= 0.2F;
							scale += 1.0F;
							scale *= MAP_scale_x;
							colour = 0xffffffff;
							break;

						case PERSON_COP:

							red  = ((GetTickCount() >> 3)       ) & 0x1ff;
							blue = ((GetTickCount() >> 3) + 0xff) & 0x1ff;

							if (red  > 0xff) {red  = 0x1ff - red; }
							if (blue > 0xff) {blue = 0x1ff - blue;}

							colour  = 0xff000000;
							colour |= red << 16;
							colour |= blue;
							scale   = MAP_scale_x * 0.5F;
							break;

						case PERSON_THUG_RASTA:
						case PERSON_THUG_GREY:
						case PERSON_THUG_RED:
							scale  = MAP_scale_x * 0.5F;
							colour = 0xffff0000;
							break;

						case PERSON_SLAG_TART:
						case PERSON_SLAG_FATUGLY:
							scale  = MAP_scale_x * 0.5F;
							colour = 0xffffff00;
							break;

						case PERSON_CIV:
						case PERSON_MECHANIC:
							scale  = MAP_scale_x * 0.5F;
							colour = 0xff00ff00;
							break;

						case PERSON_HOSTAGE:
							scale  = MAP_scale_x * 0.5F;
							colour = 0xffff00ff;
							break;

						default:
							scale  = MAP_scale_x * 0.5F;
							colour = 0xff888888;
							break;
					}

					MAP_draw_dot(
						MAP_SCREEN_X(float(p_thing->WorldPos.X) * (1.0F / 65536.0F)),
						MAP_SCREEN_Y(float(p_thing->WorldPos.Z) * (1.0F / 65536.0F)),
						scale,
						float(p_thing->Draw.Tweened->Angle) * (2.0F * PI / 2048.0F),
						colour);

					break;

				case CLASS_VEHICLE:

					MAP_draw_dot(
						MAP_SCREEN_X(float(p_thing->WorldPos.X) * (1.0F / 65536.0F)),
						MAP_SCREEN_Y(float(p_thing->WorldPos.Z) * (1.0F / 65536.0F)),
						MAP_scale_x * 2.0F,
						float(p_thing->Genus.Vehicle->Angle) * (2.0F * PI / 2048.0F),
						0xff0000ff);

					break;

				default:
					break;
			}

			index = p_thing->Child;
		}
	}

	//
	// Use the startscreen code to plonk down a logo...
	// 

	void STARTSCR_plonk_logo(void);

	// this logo sucks
	// STARTSCR_plonk_logo();

	//
	// Draw the stats
	//

	FONT2D_DrawString("Strength:",10,10,0xffffff,192,POLY_PAGE_FONT2D,0);
	FONT2D_DrawString("Stamina:",10,30,0xffffff,192,POLY_PAGE_FONT2D,0);
	FONT2D_DrawString("Skill:",10,50,0xffffff,192,POLY_PAGE_FONT2D,0);
	FONT2D_DrawString("Constitution:",10,70,0xffffff,192,POLY_PAGE_FONT2D,0);
	itoa(NET_PLAYER(0)->Genus.Player->Strength,str,10);
	FONT2D_DrawString(str,100,10,0xffffff,192,POLY_PAGE_FONT2D,0);
	itoa(NET_PLAYER(0)->Genus.Player->Stamina,str,10);
	FONT2D_DrawString(str,100,30,0xffffff,192,POLY_PAGE_FONT2D,0);
	itoa(NET_PLAYER(0)->Genus.Player->Skill,str,10);
	FONT2D_DrawString(str,100,50,0xffffff,192,POLY_PAGE_FONT2D,0);
	itoa(NET_PLAYER(0)->Genus.Player->Constitution,str,10);
	FONT2D_DrawString(str,100,70,0xffffff,192,POLY_PAGE_FONT2D,0);

	//
	// Draw the pulses.
	// 

	MAP_pulse_draw_all();
	MAP_beacon_draw_all();

	//
	// Draw the weapons carried by Darci...
	//

	MAP_draw_weapons(darci);

	//
	// Draw the frame.
	//

#ifndef TARGET_DC
	POLY_frame_draw(FALSE,FALSE);
#endif
}

#endif //#ifndef TARGET_DC


void MAP_process()
{
	MAP_process_beacons();
	MAP_process_pulses ();
}


#ifndef TARGET_DC

void MAP_draw_onscreen_beacons(void)
{
	SLONG i;

	float x;
	float y;

	float list = 0.05F;

	float dx;
	float dz;
	float dist;
	float angle;

	SLONG colour;

	MAP_Beacon *mb;

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			continue;
		}

		dx = float(mb->wx - (FC_cam[0].x >> 8));
		dz = float(mb->wz - (FC_cam[0].z >> 8));

		angle = -atan2(dx,dz) - float(FC_cam[0].yaw) * (2.0F * PI / (2048.0F * 256.0F));
		dist  =  fabs(dx) + fabs(dz);

		colour = MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS];

		MAP_draw_3d_arrow(
			angle,
			colour);
	}
}


#endif //#ifndef TARGET_DC
