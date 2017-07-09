//
// Sphere-matter.
//

#include "game.h"
#include <MFStdLib.h>
#include "pap.h"
#include "sm.h"
#include "inline.h"
#ifndef PSX
#include "fallen/ddengine/headers/aeng.h"
#else
#include "fallen/psxeng/headers/psxeng.h"
#endif

//
// The spheres that make up an object.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;	// 8-bit fixed point
	SLONG dy;
	SLONG dz;
	SLONG radius;
	SLONG mass;

} SM_Sphere;

#define SM_MAX_SPHERES 1024

SM_Sphere SM_sphere[SM_MAX_SPHERES];
SLONG     SM_sphere_upto;


//
// The elastic connections between spheres.
//

typedef struct
{
	UWORD sphere1;
	UWORD sphere2;
	SLONG dist;		// Distance squared...

} SM_Link;

#define SM_MAX_LINKS 1024

SM_Link SM_link[SM_MAX_LINKS];
SLONG   SM_link_upto;


//
// An object.
//

typedef struct
{
	UWORD sphere_index;
	UWORD sphere_num;
	UWORD link_index;
	UWORD link_num;
	SLONG jellyness;
	SLONG resolution;
	SLONG density;

} SM_Object;

#define SM_MAX_OBJECTS 16

SM_Object SM_object[SM_MAX_OBJECTS];
SLONG     SM_object_upto;





void SM_init(void)
{
	SM_sphere_upto = 0;
	SM_link_upto   = 0;
	SM_object_upto = 0;
}



//
// The minimum and maximum ranges of the cube variables.
//

#define SM_CUBE_RES_MIN	 2
#define SM_CUBE_RES_MAX	 5

#define SM_CUBE_DENSITY_MIN 0x01000
#define SM_CUBE_DENSITY_MAX 0x10000

#define SM_CUBE_JELLYNESS_MIN 0x10000
#define SM_CUBE_JELLYNESS_MAX 0x00400

void SM_create_cube(
		SLONG cx1, SLONG cy1, SLONG cz1,
		SLONG cx2, SLONG cy2, SLONG cz2,
		SLONG amount_resolution,	// 0 - 256
		SLONG amount_density,		// 0 - 256
		SLONG amount_jellyness)		// 0 - 256
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG i2;
	SLONG j2;
	SLONG k2;

	SLONG di;
	SLONG dj;
	SLONG dk;

	SLONG sx;
	SLONG sy;
	SLONG sz;

	SLONG sx2;
	SLONG sy2;
	SLONG sz2;

	SLONG dsx;
	SLONG dsy;
	SLONG dsz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;

	SLONG index1;
	SLONG index2;

	SLONG radius;
	SLONG volume;
	SLONG mass;

	SM_Object *so;
	SM_Sphere *ss;
	SM_Link   *sl;

	//
	// We work in 16-bit fixed point, not 8-bit.
	//

	cx1 <<= 8;
	cy1 <<= 8;
	cz1 <<= 8;

	cx2 <<= 8;
	cy2 <<= 8;
	cz2 <<= 8;

	//
	// The values we use to create the object.
	//

	SLONG resolution = SM_CUBE_RES_MIN       + (amount_resolution * (SM_CUBE_RES_MAX       - SM_CUBE_RES_MIN      ) >> 8);
	SLONG density    = SM_CUBE_DENSITY_MIN   + (amount_density    * (SM_CUBE_DENSITY_MAX   - SM_CUBE_DENSITY_MIN  ) >> 8);
	SLONG jellyness  = SM_CUBE_JELLYNESS_MIN + (amount_jellyness  * (SM_CUBE_JELLYNESS_MAX - SM_CUBE_JELLYNESS_MIN) >> 8);

	resolution = amount_resolution;
	density    = amount_density;
	jellyness  = amount_jellyness;

	//
	// The mass of the object is divided evenly among all the spheres.
	//

	dx = cx2 - cx1;
	dy = cy2 - cy1;
	dz = cz2 - cz1;

	volume = MUL64(MUL64(dx, dy), dz);
	mass   = MUL64(volume, density);
	mass  /= resolution * resolution * resolution;

	dx = (cx2 - cx1) / resolution;
	dy = (cy2 - cy1) / resolution;
	dz = (cz2 - cz1) / resolution;

	radius = QDIST3(abs(dx >> 1),abs(dy >> 1), abs(dz >> 1));

	//
	// Create the object.
	//

	ASSERT(WITHIN(SM_object_upto, 0, SM_MAX_OBJECTS - 1));

	so = &SM_object[SM_object_upto++];

	so->sphere_index = SM_sphere_upto;
	so->sphere_num   = 0;
	so->link_index   = SM_link_upto;
	so->link_num     = 0;
	so->jellyness    = jellyness;
	so->resolution   = resolution;
	so->density      = density;

	//
	// Add the spheres and links to the object.
	//

	for (i = 0; i < resolution; i++)
	for (j = 0; j < resolution; j++)
	for (k = 0; k < resolution; k++)
	{
		//
		// The position of this sphere.
		//

		sx = cx1 + i * dx + (dx >> 1);
		sy = cy1 + j * dy + (dy >> 1);
		sz = cz1 + k * dx + (dx >> 1);

		//
		// Create a sphere.
		//

		ASSERT(WITHIN(SM_sphere_upto, 0, SM_MAX_SPHERES - 1));

		ss = &SM_sphere[SM_sphere_upto++];

		ss->x      = sx;
		ss->y      = sy;
		ss->z      = sz;
		ss->dx     = 0;
		ss->dy     = 0;
		ss->dz     = 0;
		ss->radius = radius;
		ss->mass   = mass;

		so->sphere_num += 1;

		//
		// Adds links from this sphere to spheres with a higher index.
		//

		index1 = k + (j * resolution) + (i * resolution * resolution);

		for (di = 0; di < 2; di++)
		for (dj = 0; dj < 2; dj++)
		for (dk = 0; dk < 2; dk++)
		{
			i2 = i + di;
			j2 = j + dj;
			k2 = k + dk;

			if (i2 >= resolution ||
				j2 >= resolution ||
				k2 >= resolution)
			{
				continue;
			}

			index2 = k2 + (j2 * resolution) + (i2 * resolution * resolution);

			sx2 = cx1 + i2 * dx + (dx >> 1);
			sy2 = cy1 + j2 * dy + (dy >> 1);
			sz2 = cz1 + k2 * dx + (dx >> 1);

			dsx = sx2 - sx;
			dsy = sy2 - sy;
			dsz = sz2 - sz;

			dist  = MUL64(dsx, dsx);
			dist += MUL64(dsy, dsy);
			dist += MUL64(dsz, dsz);

			//
			// Create the new link.
			//

			ASSERT(WITHIN(SM_link_upto, 0, SM_MAX_LINKS - 1));

			sl = &SM_link[SM_link_upto++];

			sl->sphere1 = index1;
			sl->sphere2 = index2;
			sl->dist    = dist;

			so->link_num += 1;
		}
	}
}


void SM_process()
{
	SLONG i;
	SLONG j;
	SLONG height;

	SLONG ax;
	SLONG ay;
	SLONG az;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG ddist;
	SLONG force;
	SLONG accel;

	SM_Object *so;
	SM_Sphere *ss;
	SM_Sphere *ss1;
	SM_Sphere *ss2;
	SM_Link   *sl;

	//
	// Process all the spheres.
	//

	for (i = 0; i < SM_sphere_upto; i++)
	{
		ss = &SM_sphere[i];

		//
		// Gravity.
		//

		#define SM_GRAVITY (-0x200)

		ss->dy += SM_GRAVITY;

		//
		// Movement.
		//

		ss->x += ss->dx / 256;
		ss->y += ss->dy / 256;
		ss->z += ss->dz / 256;

		//
		// Collision with the ground.
		//

		height = PAP_calc_height_at(ss->x >> 8, ss->z >> 8) << 8;

		if (ss->y - ss->radius < height)
		{
			ss->y   =  ss->radius + height;
			ss->dy  = -abs(ss->dy);
			ss->dy -=  ss->dy >> 9;
			ss->dx /=  2;
			ss->dz /=  2;
		}

		//
		// Friction.
		//

		ss->dx -= ss->dx / 256;
		ss->dy -= ss->dy / 256;
		ss->dz -= ss->dz / 256;

		ss->dx -= ss->dx / 512;
		ss->dy -= ss->dy / 512;
		ss->dz -= ss->dz / 512;

		ss->dx -= SIGN(ss->dx);
		ss->dy -= SIGN(ss->dy);
		ss->dz -= SIGN(ss->dz);
	}

	//
	// Process each object.
	//

	for (i = 0; i < SM_object_upto; i++)
	{
		so = &SM_object[i];

		//
		// Process all the links.
		//

		for (j = 0; j < SM_link_upto; j++)
		{
			sl = &SM_link[j];

			ASSERT(WITHIN(sl->sphere1, 0, SM_sphere_upto - 1));
			ASSERT(WITHIN(sl->sphere2, 0, SM_sphere_upto - 1));

			ss1 = &SM_sphere[sl->sphere1];
			ss2 = &SM_sphere[sl->sphere2];

			dx = ss2->x - ss1->x;
			dy = ss2->y - ss1->y;
			dz = ss2->z - ss1->z;

			dist  = MUL64(dx,dx);
			dist += MUL64(dy,dy);
			dist += MUL64(dz,dz);

			ddist = dist - sl->dist;

			if (ddist)
			{
				force = MUL64(ddist, so->jellyness);

				//
				// Equal and opposite force on both spheres.
				//

				accel = DIV64(force, ss1->mass);

				ss1->dx += MUL64(dx, accel);
				ss1->dy += MUL64(dy, accel);
				ss1->dz += MUL64(dz, accel);

				accel = DIV64(force, ss2->mass);

				ss2->dx -= MUL64(dx, accel);
				ss2->dy -= MUL64(dy, accel);
				ss2->dz -= MUL64(dz, accel);
			}
		}
	}

	if (Keys[KB_P5])
	{
		SM_sphere[8].dy -= SM_GRAVITY * 50;
	}
}


//
// Getting the spheres.
//

SLONG   SM_get_upto;
SM_Info SM_get_info;

void SM_get_start()
{
	SM_get_upto = 0;

	if (ControlFlag)
	{
		SLONG i;

		SM_Link *sl;

		SM_Sphere *ss1;
		SM_Sphere *ss2;

		//
		// Draw all the links.
		//

		for (i = 0; i < SM_link_upto; i++)
		{
			sl = &SM_link[i];

			ASSERT(WITHIN(sl->sphere1, 0, SM_sphere_upto - 1));
			ASSERT(WITHIN(sl->sphere2, 0, SM_sphere_upto - 1));

			ss1 = &SM_sphere[sl->sphere1];
			ss2 = &SM_sphere[sl->sphere2];

			AENG_world_line(
				ss1->x >> 8, ss1->y >> 8, ss1->z >> 8, 16, 0x00ff0000,
				ss2->x >> 8, ss2->y >> 8, ss2->z >> 8, 1,  0x0000ff00,
				TRUE);
		}
	}
}

SM_Info *SM_get_next()
{
	SM_Sphere *ss;

	if (!WITHIN(SM_get_upto, 0, SM_sphere_upto - 1))
	{
		return NULL;
	}

	ss = &SM_sphere[SM_get_upto++];

	SM_get_info.x      = ss->x      >> 8;
	SM_get_info.y      = ss->y      >> 8;
	SM_get_info.z      = ss->z      >> 8;
	SM_get_info.radius = ss->radius >> 8;
	SM_get_info.colour = 123456789 * SM_get_upto + 3141592653;

	return &SM_get_info;
}


