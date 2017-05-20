#include <MFStdLib.h>
#include <DDLib.h>
#include "game.h"
#include "ray.h"
#include "poly.h"
#include "inline.h"
#include <math.h>

#if 0	// Intel C compiler barfs

D3DTexture RAY_screen1;
D3DTexture RAY_screen2;


//
// The current locked RAY_screen.
//

#define RAY_SCREEN_SIZE 256

SLONG RAY_which_screen;

UWORD *RAY_screen_bitmap;
SLONG  RAY_screen_pitch;		// In bytes!

SLONG RAY_screen_mask_red;
SLONG RAY_screen_mask_green;
SLONG RAY_screen_mask_blue;
SLONG RAY_screen_mask_alpha;

SLONG RAY_screen_shift_red;
SLONG RAY_screen_shift_green;
SLONG RAY_screen_shift_blue;
SLONG RAY_screen_shift_alpha;


#define RAY_LIGHT_X (+0x77B0)
#define RAY_LIGHT_Y (+0x77B0)
#define RAY_LIGHT_Z (-0xC000)



#define LOOKUP_MAX	  65536
#define LOOKUP_SHIFT  8	

SLONG RAY_sqrt[LOOKUP_MAX];





typedef struct
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG sky_r;
	SLONG sky_b;
	SLONG ground_ix;
	SLONG ground_iz;
	SLONG ground_distr;
 
} RAY_Vector;

RAY_Vector RAY_pixel[RAY_SCREEN_SIZE][RAY_SCREEN_SIZE];


void RAY_pixel_calc(void)
{
	SLONG x;
	SLONG y;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG sky_r;
	SLONG sky_b;

	SLONG ground_ix;
	SLONG ground_iz;
	SLONG ground_distr;

	SLONG length;
	float flen;
	SLONG overlength;

	for (y = 0; y < RAY_SCREEN_SIZE; y++)
	for (x = 0; x < RAY_SCREEN_SIZE; x++)
	{
		dx = (x - (RAY_SCREEN_SIZE / 2)) *  ((0x15555 * 20 / 20) / (RAY_SCREEN_SIZE / 2));
		dy = (y - (RAY_SCREEN_SIZE / 2)) * -((0x10000 *  5 / 4) / (RAY_SCREEN_SIZE / 2));
		dz = 0x10000;

		length = MUL64(dx,dx) + MUL64(dy,dy) + MUL64(dz,dz);

		flen  = float(length);
		flen *= 1.0F / 65536.0F;
		flen  = 1.0F / sqrt(flen);
		flen *= 65536.0F;

		overlength = SLONG(flen);

		dx = MUL64(dx, overlength);
		dy = MUL64(dy, overlength);
		dz = MUL64(dz, overlength);

		if (dy > -0x1400)
		{
			sky_r = MAX(0, ((dy + 0x1400 + 0x200) >> 8)  - 1);
			sky_b = MAX(0, ((dy + 0x1400 + 0x588) >> 10) - 1);
		}
		else
		{
			//
			// Intersect the ground plane at (dx/dy) (dz/dy)
			//

			ground_ix    = DIV64(dx,-dy) << 1;
			ground_iz    = DIV64(dz,-dy) << 1;
			ground_distr = 255 - ((MUL64(dx,dx) + MUL64(dz,dz) >> 7) & 0xff);

			if (ground_distr < 0)
			{
				ground_distr = 0;
			}
		}

		RAY_pixel[y][x].dx = dx;
		RAY_pixel[y][x].dy = dy;
		RAY_pixel[y][x].dz = dz;

		RAY_pixel[y][x].sky_r = sky_r;
		RAY_pixel[y][x].sky_b = sky_b;


		RAY_pixel[y][x].ground_ix    = ground_ix;
		RAY_pixel[y][x].ground_iz    = ground_iz;
		RAY_pixel[y][x].ground_distr = ground_distr;
	}
}


//
// Returns the 16:16 square root.
//

SLONG SQRT64_slow(SLONG x)
{
	float f = float(x);
	
	f *= (1.0F / 65536.0F);
	f  = sqrt(f);
	f *= 65536.0F;

	SLONG ans;

	ans = SLONG(f);

	return ans;
}

SLONG SQRT64(SLONG x)
{
	SLONG i = x >> LOOKUP_SHIFT;

	ASSERT(WITHIN(i, 0, LOOKUP_MAX - 1));

	return RAY_sqrt[i];
}


//
// D3D Poly stuff...
//

#define RAY_D3D_SIZE 32

D3DTLVERTEX RAY_d3d_vertex[RAY_D3D_SIZE];
SLONG       RAY_d3d_vertex_upto;
UWORD       RAY_d3d_index [RAY_D3D_SIZE];
SLONG       RAY_d3d_index_upto;


//
// The spheres...
//

#define RAY_SPHERE_SPECULAR		(1 << 0)
#define RAY_SPHERE_REFLECTIVE	(1 << 1)

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG radius;	// Always 1?
	SLONG dist2;	// Distance from origin squared.
	SLONG minx;
	SLONG maxx;
	SLONG miny;
	SLONG maxy;
	SLONG minz;
	SLONG maxz;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	UBYTE r;
	UBYTE g;
	UBYTE b;
	UBYTE flag;

	SLONG sx;
	SLONG sy;

	SLONG sxmin;
	SLONG sxmax;

	SLONG symin;
	SLONG symax;
	
} RAY_Sphere;

#define RAY_MAX_SPHERES 4

RAY_Sphere RAY_sphere[RAY_MAX_SPHERES];
SLONG      RAY_sphere_upto = 0;



//
// Initialises everything.
//

void RAY_init()
{
	//
	// Sets up the screen.
	//

	RAY_screen1.CreateUserPage(RAY_SCREEN_SIZE, FALSE);
	RAY_screen2.CreateUserPage(RAY_SCREEN_SIZE, FALSE);

	//
	// Precalc.
	//

	RAY_pixel_calc();


	//
	// Sqrt precalc.
	//

	SLONG i;

	for (i = 0; i < LOOKUP_MAX; i++)
	{
		RAY_sqrt[i] = SQRT64_slow(i << LOOKUP_SHIFT);
	}


	//
	// Setup world.
	//

	RAY_sphere[0].x      =-0x18000;
	RAY_sphere[0].y      = 0x25000;
	RAY_sphere[0].z      = 0x38000;
	RAY_sphere[0].radius = 0x10000;	// HARDCODED!
	RAY_sphere[0].r      = 30;
	RAY_sphere[0].g      = 150;
	RAY_sphere[0].b      = 21;
	RAY_sphere[0].flag   = RAY_SPHERE_SPECULAR;
//	RAY_sphere[0].flag   = RAY_SPHERE_REFLECTIVE|RAY_SPHERE_SPECULAR;

	RAY_sphere[1].x      = 0x10000;
	RAY_sphere[1].y      = 0x18000;
	RAY_sphere[1].z      = 0x39000;
	RAY_sphere[1].radius = 0x10000;	// HARDCODED!
	RAY_sphere[1].r      = 0;
	RAY_sphere[1].g      = 225;
	RAY_sphere[1].b      = 200;
	RAY_sphere[1].flag   = 0;	 
//	RAY_sphere[1].flag   = RAY_SPHERE_REFLECTIVE|RAY_SPHERE_SPECULAR;


	RAY_sphere[2].x      =-0x10000;
	RAY_sphere[2].y      = 0x50000;
	RAY_sphere[2].z      = 0x58000;
	RAY_sphere[2].radius = 0x10000;	// HARDCODED!
	RAY_sphere[2].dx     = 0x2000;
	RAY_sphere[2].r      = 225;
	RAY_sphere[2].g      = 225;
	RAY_sphere[2].b      = 225;
	RAY_sphere[2].flag   = RAY_SPHERE_REFLECTIVE|RAY_SPHERE_SPECULAR;

	RAY_sphere_upto = 3;
}


void RAY_screen_lock(void)
{
	HRESULT res;

	D3DTexture *rs;

	RAY_which_screen ^= 1;

	rs = (RAY_which_screen) ? &RAY_screen1 : &RAY_screen2;

	res = rs->LockUser(
			&RAY_screen_bitmap,
			&RAY_screen_pitch);

	ASSERT(SUCCEEDED(res));

	RAY_screen_mask_red	   = rs->mask_red;
	RAY_screen_mask_green  = rs->mask_green;
	RAY_screen_mask_blue   = rs->mask_blue;
	RAY_screen_mask_alpha  = rs->mask_alpha;
	RAY_screen_shift_red   = rs->shift_red;
	RAY_screen_shift_green = rs->shift_green;
	RAY_screen_shift_blue  = rs->shift_blue;
	RAY_screen_shift_alpha = rs->shift_alpha;
}

void RAY_screen_unlock(void)
{
	D3DTexture *rs;

	rs = (RAY_which_screen) ? &RAY_screen1 : &RAY_screen2;

	rs->UnlockUser();
}

void RAY_screen_update(void)
{
}




struct
{
	SLONG x1;
	SLONG x2;
	SLONG y1;
	SLONG y2;

} offset[3] =
{
	{-22,+55,-37,+45},
	{-40,+31,-36,+38},
	{-46,+46,-30,+45}
};



void RAY_animate(void)
{
	SLONG i;

	static animate = 1;

	if (Keys[KB_A])
	{
		Keys[KB_A] = 0;

		animate ^= 1;
	}

	RAY_Sphere *rs;

	{
		rs = &RAY_sphere[0];

		if (Keys[KB_LEFT ]) {rs->x -= 0x1000;}
		if (Keys[KB_RIGHT]) {rs->x += 0x1000;}

		if (Keys[KB_UP  ]) {rs->z += 0x1000;}
		if (Keys[KB_DOWN]) {rs->z -= 0x1000;}
	}

	for (i = 0; i < RAY_sphere_upto; i++)
	{
		rs = &RAY_sphere[i];

		if (animate)
		{
			rs->dy -= 0x300;

			rs->x += rs->dx;
			rs->y += rs->dy;
			rs->z += rs->dz;

			if (rs->y < -0x10000)
			{
				rs->dy = abs(rs->dy);// - (abs(rs->dy) >> 14);
				rs->y  = -0x10000;
			}
		}

		if (rs->x >  0x48000) {rs->dx = -abs(rs->dx);}
		if (rs->x < -0x48000) {rs->dx = +abs(rs->dx);}

		rs->minx = rs->x - rs->radius;
		rs->maxx = rs->x + rs->radius;
		rs->miny = rs->y - rs->radius;
		rs->maxy = rs->y + rs->radius;
		rs->minz = rs->z - rs->radius;
		rs->maxz = rs->z + rs->radius;

		rs->dist2 = MUL64(rs->x,rs->x) + MUL64(rs->y,rs->y) + MUL64(rs->z, rs->z);

		rs->sx = 128 + (DIV64(rs->x, rs->z) * 128 >> 16);
		rs->sy = 128 - (DIV64(rs->y, rs->z) * 128 >> 16);
	}

	SLONG a = 0;

	if (ShiftFlag) a = 1;
	if (ControlFlag) a= 2;

	if (Keys[KB_P7]) {offset[a].y1 -= 1;}
	if (Keys[KB_P8]) {offset[a].y1 += 1;}

	if (Keys[KB_P9]) {offset[a].x2 -= 1;}
	if (Keys[KB_P6]) {offset[a].x2 += 1;}

	if (Keys[KB_P4]) {offset[a].x1 -= 1;}
	if (Keys[KB_P1]) {offset[a].x1 += 1;}

	if (Keys[KB_P2]) {offset[a].y2 -= 1;}
	if (Keys[KB_P3]) {offset[a].y2 += 1;}

	for (i = 0; i < 3; i++)
	{
		RAY_sphere[i].sxmin = RAY_sphere[i].sx + offset[i].x1;
		RAY_sphere[i].sxmax = RAY_sphere[i].sx + offset[i].x2;
											
		RAY_sphere[i].symin = RAY_sphere[i].sy + offset[i].y1;
		RAY_sphere[i].symax = RAY_sphere[i].sy + offset[i].y2;

		if (Keys[KB_D])
		{
			TRACE("Vals(%d) = %d,%d,%d,%d\n", i, 
				offset[i].x1,
				offset[i].x2,
				offset[i].y1,
				offset[i].y2);
		}
	}

	if (Keys[KB_B])
	{
		Keys[KB_B] = 0;

		for (i = 0; i < RAY_sphere_upto; i++)
		{
			rs = &RAY_sphere[i];

			rs->dy += rs->dy >> 4;
		}
	}
}


//
// Secondary intersection...
//

void RAY_intersect2(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG dx, SLONG dy, SLONG dz,
		ULONG use,
		UBYTE *r,
		UBYTE *g,
		UBYTE *b)
{
	SLONG i;

	SLONG v;
	SLONG disc;
	SLONG dist;

	SLONG dsx;
	SLONG dsy;
	SLONG dsz;

	SLONG ground_ix;
	SLONG ground_iz;
	SLONG ground_distr;

	RAY_Sphere *rs;

	for (i = RAY_sphere_upto - 1; i >= 0; i--)
	{
		rs = &RAY_sphere[i];

		if (!(use & (1 << i)))
		{
			continue;
		}

		//
		// Simple reject?
		//

		if (dx < 0)
		{
			if (rs->minx > x1) {continue;}
		}
		else
		{
			if (rs->maxx < x1) {continue;}
		}

		if (dy < 0)
		{
			if (rs->miny > y1) {continue;}
		}
		else
		{
			if (rs->maxy < y1) {continue;}
		}

		if (dz < 0)
		{
			if (rs->minz > z1) {continue;}
		}
		else
		{
			if (rs->maxz < z1) {continue;}
		}

		dsx = rs->x - x1;
		dsy = rs->y - y1;
		dsz = rs->z - z1;

		v    = MUL64(dsx, dx)  + MUL64(dsy, dy)  + MUL64(dsz, dz);
		dist = MUL64(dsx, dsx) + MUL64(dsy, dsy) + MUL64(dsz, dsz);

		disc = 0x10000 - (dist - MUL64(v,v));

		if (disc <= 0)
		{
			//
			// No intersection.
			//
		}
		else
		{
			*r = rs->r >> 1;
			*g = rs->g >> 1;
			*b = rs->b >> 1;

			return;
		}
	}

	if (dy > -0x1400)
	{
		SLONG sr = ((dy + 0x1400 + 0x200) >>  8) - 1; 
		SLONG sb = ((dy + 0x1400 + 0x588) >> 10) - 1;

		SATURATE(sr, 0, 255);
		SATURATE(sb, 0, 255);

		*r = sr;
		*g = 0;
		*b = sb;
	}
	else
	{
		//
		// Intersect the ground plane at (dx/dy) (dz/dy)
		//

		SLONG overdy = DIV64(0x10000, -dy);

		ground_ix    = MUL64(dx,overdy) << 1;
		ground_iz    = MUL64(dz,overdy) << 1;
		ground_distr = 255 - ((MUL64(dx,dx) + MUL64(dz,dz) >> 8) & 0xff);

		if ((ground_ix ^ ground_iz)	& 0x10000)
		{
			*r = ground_distr;
			*g = ground_distr;
			*b = 0;
		}
		else
		{
			*r = 0;
			*g = 0;
			*b = ground_distr;
		}
	}
}





//
// Returns TRUE if the given point is in shadow.
//

SLONG RAY_in_shadow(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG start_sphere)
{
	SLONG i;

	SLONG dsx;
	SLONG dsy;
	SLONG dsz;

	SLONG v;
	SLONG disc;
	SLONG dist2;

	RAY_Sphere *rs;

	for (i = start_sphere; i >= 0; i--)
	{
		rs = &RAY_sphere[i];

		//
		// Early outs?
		// 

		dsx = rs->x - x;
		dsy = rs->y - y;
		dsz = rs->z - z;

		v     = MUL64(dsx, -RAY_LIGHT_X)  + MUL64(dsy, -RAY_LIGHT_Y) + MUL64(dsz, -RAY_LIGHT_Z);
		dist2 = MUL64(dsx, dsx) + MUL64(dsy,dsy) + MUL64(dsz,dsz);

		disc = 0x10000 - (dist2 - MUL64(v,v));

		if (disc <= 0)
		{
			//
			// No intersection.
			//
		}
		else
		{
			//
			// In shadow.
			//

			return TRUE;
		}
	}

	return FALSE;
}


//
// Gives the colour of the given NORMALISED! ray. It has a bitfield for which
// sphere to ignore and a max_recursion level too.  Returns FALSE if no
// intersection occured.
//

SLONG RAY_get_colour_from_origin(
		SLONG dx, SLONG dy, SLONG dz,
		ULONG use,
		UBYTE *ray_r,
		UBYTE *ray_g,
		UBYTE *ray_b)
{
	SLONG i;

	SLONG d;
	SLONG v;
	SLONG dist;
	SLONG disc;
	SLONG len;

	RAY_Sphere *rs;

	SLONG ix;
	SLONG iy;
	SLONG iz;

	SLONG r;
	SLONG g;
	SLONG b;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG dsx;
	SLONG dsy;
	SLONG dsz;

	//
	// Check the spheres.
	//

	for (i = 0; i < RAY_sphere_upto; i++)
	{
		rs = &RAY_sphere[i];

		if (!(use & (1 << i)))
		{
			continue;
		}

		//
		// Simple reject?
		//

		if (dx < 0)
		{
			if (rs->minx > 0) {continue;}
		}
		else
		{
			if (rs->maxx < 0) {continue;}
		}

		if (dy < 0)
		{
			if (rs->miny > 0) {continue;}
		}
		else
		{
			if (rs->maxy < 0) {continue;}
		}

		if (dz < 0)
		{
			if (rs->minz > 0) {continue;}
		}
		else
		{
			if (rs->maxz < 0) {continue;}
		}

		dsx = rs->x;
		dsy = rs->y;
		dsz = rs->z;

		v    = MUL64(dsx, dx)  + MUL64(dsy, dy)  + MUL64(dsz, dz);

		disc = 0x10000 - (rs->dist2 - MUL64(v,v));

		if (disc <= 0)
		{
			//
			// No intersection.
			//
		}
		else
		{
			//
			// Intersection! But where?
			//

			SLONG along;
			SLONG bright;

			d = SQRT64(disc);
			
			along = v - d;

			ix = MUL64(along, dx);
			iy = MUL64(along, dy);
			iz = MUL64(along, dz);

			nx = ix - rs->x;
			ny = iy - rs->y;
			nz = iz - rs->z;
			
			#define RT 0x93cd

			bright = MUL64(nx,RAY_LIGHT_X) + MUL64(ny,RAY_LIGHT_Y) + MUL64(nz,RAY_LIGHT_Z);
			
			if (bright < 0 || RAY_in_shadow(ix, iy, iz, i - 1))
			{
				*ray_r = rs->r >> 1;
				*ray_g = rs->g >> 1;
				*ray_b = rs->b >> 1;
			}
			else
			{
				bright = 0x8000 + (bright >> 1);

				if (rs->flag & RAY_SPHERE_SPECULAR)
				{
					r = MUL64(bright, rs->r);
					g = MUL64(bright, rs->g);
					b = MUL64(bright, rs->b);

					bright = MUL64(bright, bright);
					bright = MUL64(bright, bright);
					bright = MUL64(bright, bright);
					bright = MUL64(bright, bright);

					r += bright >> 8;
					g += bright >> 8;
					b += bright >> 8;

					if (r > 255) {r = 255;}
					if (g > 255) {g = 255;}
					if (b > 255) {b = 255;}

					*ray_r = r;
					*ray_g = g;
					*ray_b = b;
				}
				else
				{
					*ray_r = MUL64(bright, rs->r);
					*ray_g = MUL64(bright, rs->g);
					*ray_b = MUL64(bright, rs->b);
				}
			}

			if (rs->flag & RAY_SPHERE_REFLECTIVE)
			{
				UBYTE rr;
				UBYTE rg;
				UBYTE rb;

				SLONG rx;
				SLONG ry;
				SLONG rz;

				SLONG dprod = MUL64(nx,dx) + MUL64(ny, dy) + MUL64(nz, dz);

				dprod <<= 1;

				rx = dx - MUL64(nx, dprod);
				ry = dy - MUL64(ny, dprod);
				rz = dz - MUL64(nz, dprod);

				RAY_intersect2(
					ix, iy, iz,
					rx, ry, rz,
					0x3,
					&rr,
					&rg,
					&rb);

				*ray_r = (*ray_r * rr >> 8);
				*ray_g = (*ray_g * rg >> 8);
				*ray_b = (*ray_b * rb >> 8);
			}

			return TRUE;
		}
	}

	return FALSE;
}


typedef struct span
{
	ULONG in;
	ULONG out;
	SLONG x;

	struct span *next;

} Span;

#define MAX_SPANS 8

Span  span[MAX_SPANS];
SLONG span_upto;

Span *span_head;



void RAY_render_scene(void)
{
	SLONG i;

	SLONG x;
	SLONG y;

	SLONG span_minx;
	SLONG span_maxx;
	SLONG span_width;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG index;
	UWORD *pixel;

	Span *s1;
	Span *s2;

	UBYTE r;
	UBYTE g;
	UBYTE b;

	ULONG in;

	RAY_Sphere *rs;
	RAY_Vector *rv;

	Span **prev;
	Span  *next;

	#define BB_SIZE_X 50
	#define BB_SIZE_Y 60

	for (y = 0; y < 205; y++)
	{
		//
		// Create the spans...
		//

		span_head = &span[0];

		span[0].in   = 0;
		span[0].out  = 0;
		span[0].x    = 0;
		span[0].next = &span[1];

		span[1].in   = 0;
		span[1].out  = 0;
		span[1].x    = RAY_SCREEN_SIZE;
		span[1].next = NULL;

		span_upto = 2;

		//
		// Go through the bounding boxes of the spheres.
		// 

		for (i = 0; i < RAY_sphere_upto; i++)
		{
			rs = &RAY_sphere[i];

			if (WITHIN(y, rs->symin, rs->symax))
			{
				span_minx = rs->sxmin;
				span_maxx = rs->sxmax;

				//
				// Insert MIN span
				//

				span[span_upto].in  = 1 << i;
				span[span_upto].out = 0;
				span[span_upto].x   = span_minx;

				prev = &span_head;
				next =  span_head;

				while(1)
				{
					if (next != NULL && span_minx > next->x)
					{
						//
						// Don't insert it here.
						//

						prev = &next->next;
						next =  next->next;
					}
					else
					{
						//
						// Insert here.
						//

						span[span_upto].next = next;
					   *prev = &span[span_upto];

					    break;
					}
				}

				span_upto += 1;

				//
				// Insert MAX span.
				//

				span[span_upto].in  = 0;
				span[span_upto].out = 1 << i;
				span[span_upto].x   = span_maxx;

				prev = &span_head;
				next =  span_head;

				while(1)
				{
					if (next != NULL && span_maxx > next->x)
					{
						//
						// Don't insert it here.
						//

						prev = &next->next;
						next =  next->next;
					}
					else
					{
						//
						// Insert here.
						//

						span[span_upto].next = next;
					   *prev = &span[span_upto];

					    break;
					}
				}

				span_upto += 1;


			}
		}

		//
		// Go through all the spans on this line.
		//

		s1 = span_head;
		s2 = span_head->next;

		in = 0;

		while(s2)
		{
			in |=  s1->in;
			in &= ~s1->out;

			span_minx = s1->x;
			span_maxx = s2->x;

			SATURATE(span_minx, 0, RAY_SCREEN_SIZE);
			SATURATE(span_maxx, 0, RAY_SCREEN_SIZE);

			span_width = span_maxx - span_minx;

			index = span_minx + (y * RAY_screen_pitch >> 1);
			pixel = &RAY_screen_bitmap[index];
			rv = &RAY_pixel[y][span_minx];

			if (in == 0)
			{
				//
				// Easy!
				//

				if (y < 150)
				{
					while(span_width--)
					{
						*pixel  = (rv->sky_r >> RAY_screen_mask_red) << RAY_screen_shift_red;
						*pixel |= (rv->sky_b >> RAY_screen_mask_blue) << RAY_screen_shift_blue;
						
						rv++;
						pixel++;
					}
				}
				else
				{
					while(span_width--)
					{
						SLONG distr = rv->ground_distr;

						if (distr == 0)
						{
							*pixel = 0;
						}
						else
						{
							if (y <= 188 && span_minx < 180 && RAY_in_shadow(rv->ground_ix, -0x20000, rv->ground_iz, RAY_sphere_upto - 2))
							{
								distr >>= 1;
							}

							if ((rv->ground_ix ^ rv->ground_iz) & 0x10000)
							{
								r = distr;
								g = distr;
								b = 0;
							}
							else
							{
								r = 0;
								g = 0;
								b = distr;
							}


							*pixel  = (r >> RAY_screen_mask_red)   << RAY_screen_shift_red;
							*pixel |= (g >> RAY_screen_mask_green) << RAY_screen_shift_green;
							*pixel |= (b >> RAY_screen_mask_blue)  << RAY_screen_shift_blue;
						}
						
						span_minx++;

						rv++;
						pixel++;
					}
				}
			}
			else
			{
				//
				// We actually have to RayTrace :-(
				//

				while(span_width--)
				{
					if (!RAY_get_colour_from_origin(
							rv->dx,
							rv->dy,
							rv->dz,
							in,
							&r,
							&g,
							&b))
					{
						//if (rv->dy > -0x1400)

						if (y < 150)
						{
							r = rv->sky_r;
							g = 0;
							b = rv->sky_b;
						}
						else
						{
							SLONG distr = rv->ground_distr;

							{
								if (distr && RAY_in_shadow(rv->ground_ix, -0x20000, rv->ground_iz, RAY_sphere_upto - 2))
								{
									distr >>= 1;
								}

								if ((rv->ground_ix ^ rv->ground_iz) & 0x10000)
								{
									r = distr;
									g = distr;
									b = 0;
								}
								else
								{
									r = 0;
									g = 0;
									b = distr;
								}
							}
						}
					}

					/*
					static lookat = 1;

					if(in & lookat)
					{
						r += 16;
						g += 16;
						b += 16;
					}
					*/

					*pixel  = (r >> RAY_screen_mask_red)   << RAY_screen_shift_red;
					*pixel |= (g >> RAY_screen_mask_green) << RAY_screen_shift_green;
					*pixel |= (b >> RAY_screen_mask_blue)  << RAY_screen_shift_blue;
					
					rv++;
					pixel++;
				}
			}

			s1 = s2;
			s2 = s2->next;
		}
	}

	/*








	rv = &RAY_pixel[0][0];

	for (y = 0; y < RAY_SCREEN_SIZE; y++)
	{
		for (x = 0; x < RAY_SCREEN_SIZE; x++, rv++)
		{
			if (!RAY_get_colour_from_origin(
					rv->dx,
					rv->dy,
					rv->dz,
					0xffffffff,
					&r,
					&g,
					&b))
			{
				if (rv->dy > -0x1400)
				{
					r = rv->sky_r;
					g = 0;
					b = rv->sky_b;
				}
				else
				{
					SLONG distr = rv->ground_distr;

					if (RAY_in_shadow(rv->ground_ix, -0x20000, rv->ground_iz, RAY_sphere_upto - 1))
					{
						distr >>= 1;
					}

					if ((rv->ground_ix ^ rv->ground_iz) & 0x10000)
					{
						r = distr;
						g = distr;
						b = 0;
					}
					else
					{
						r = 0;
						g = 0;
						b = distr;
					}
				}
			}


			if (x == RAY_sphere[0].sx - BB_SIZE_X ||
				x == RAY_sphere[0].sx + BB_SIZE_X ||
				y == RAY_sphere[0].sy - BB_SIZE_Y ||
				y == RAY_sphere[0].sy + BB_SIZE_Y)
			{
				r = 255;
				g = 255;
				b = 255;
			}

			if (y == 150)
			{
				r = 255;
				g = 255;
				b = (x & 1) << 7;
			}

			if (y == 140)
			{
				r = 255;
				g = 255;
				b = (x & 2) << 7;
			}

			if (y == 160)
			{
				r = 255;
				g = 255;
				b = (x & 4) << 7;
			}



			index = x + (y * RAY_screen_pitch >> 1);

			RAY_screen_bitmap[index]  = (r >> RAY_screen_mask_red)   << RAY_screen_shift_red;
			RAY_screen_bitmap[index] |= (g >> RAY_screen_mask_green) << RAY_screen_shift_green;
			RAY_screen_bitmap[index] |= (b >> RAY_screen_mask_blue)  << RAY_screen_shift_blue;
		}
	}

	*/
}


void RAY_render_polys(void)
{
	D3DTexture *rs;

	rs = (RAY_which_screen) ? &RAY_screen1 : &RAY_screen2;

	//
	// Setup vertices and indices...
	//

	RAY_d3d_vertex[0].sx       = 0.0F;
	RAY_d3d_vertex[0].sy       = 0.0F;
	RAY_d3d_vertex[0].sz       = 0.5F;
	RAY_d3d_vertex[0].rhw      = 0.5F;
	RAY_d3d_vertex[0].tu       = 0.0F;
	RAY_d3d_vertex[0].tv       = 0.0F;
	RAY_d3d_vertex[0].color    = 0x00ffffff;
	RAY_d3d_vertex[0].specular = 0x00000000;

	RAY_d3d_vertex[1].sx       = 640.0F;
	RAY_d3d_vertex[1].sy       = 0.0F;
	RAY_d3d_vertex[1].sz       = 0.5F;
	RAY_d3d_vertex[1].rhw      = 0.5F;
	RAY_d3d_vertex[1].tu       = 1.0F;
	RAY_d3d_vertex[1].tv       = 0.0F;
	RAY_d3d_vertex[1].color    = 0x00ffffff;
	RAY_d3d_vertex[1].specular = 0x00000000;

	RAY_d3d_vertex[2].sx       = 640.0F;
	RAY_d3d_vertex[2].sy       = 480.0F;
	RAY_d3d_vertex[2].sz       = 0.5F;
	RAY_d3d_vertex[2].rhw      = 0.5F;
	RAY_d3d_vertex[2].tu       = 1.0F;
	RAY_d3d_vertex[2].tv       = 0.8F;
	RAY_d3d_vertex[2].color    = 0x00ffffff;
	RAY_d3d_vertex[2].specular = 0x00000000;

	RAY_d3d_vertex[3].sx       = 0.0F;
	RAY_d3d_vertex[3].sy       = 480.0F;
	RAY_d3d_vertex[3].sz       = 0.5F;
	RAY_d3d_vertex[3].rhw      = 0.5F;
	RAY_d3d_vertex[3].tu       = 0.0F;
	RAY_d3d_vertex[3].tv       = 0.8F;
	RAY_d3d_vertex[3].color    = 0x00ffffff;
	RAY_d3d_vertex[3].specular = 0x00000000;

	RAY_d3d_index[0] = 0;
	RAY_d3d_index[1] = 1;
	RAY_d3d_index[2] = 2;

	RAY_d3d_index[3] = 0;
	RAY_d3d_index[4] = 2;
	RAY_d3d_index[5] = 3;

	//
	// Setup renderstates.
	//

	SET_RENDER_STATE(D3DRENDERSTATE_SHADEMODE,D3DSHADE_GOURAUD);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_LINEAR);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMIN,D3DFILTER_LINEAR);
	SET_RENDER_STATE(D3DRENDERSTATE_DITHERENABLE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_SUBPIXEL,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL);
	SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
	SET_RENDER_STATE(D3DRENDERSTATE_FOGCOLOR,  0x008890ee);
	SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FALSE);
//	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREHANDLE, rs->GetTextureHandle());
	SET_TEXTURE(rs->GetD3DTexture());
	SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_WRAP);

	//
	// Do the draw primitive call!
	//

	DRAW_INDEXED_PRIMITIVE(
		D3DPT_TRIANGLELIST,
		D3DFVF_TLVERTEX,
		RAY_d3d_vertex,
		4,
		RAY_d3d_index,
		6,
		D3DDP_DONOTUPDATEEXTENTS);
}


void RAY_do()
{
	SLONG i;

	RAY_init();

	while(1)
	{
		SHELL_ACTIVE;

		if (Keys[KB_Q]) {return;}

		for (i = 0; i < 256; i++)
		{
			Keys[i] = 0;
		}


		/*
		the_display.SetUserColour(0, 0, 0);
		the_display.SetUserBackground();
		the_display.ClearViewport();
		*/

		RAY_animate();
		RAY_screen_lock();
		RAY_render_scene();
		RAY_screen_unlock();
		RAY_screen_update();

		BEGIN_SCENE;
		RAY_render_polys();
		END_SCENE;

		AENG_flip();
	}
}


#endif