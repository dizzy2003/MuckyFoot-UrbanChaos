//
// The orbs
//

#include "always.h"
#include "land.h"
#include "orb.h"
#include "os.h"


//
// The orbs.
//

ORB_Orb ORB_orb[ORB_MAX_ORBS];


//
// The texture we draw the orbs with.
//

OS_Texture *ORB_ot;


void ORB_init()
{
	memset(ORB_orb, 0, sizeof(ORB_orb));

	ORB_ot = OS_texture_create("orb.tga");
}


ORB_Orb *ORB_create(float x, float y, float radius)
{
	SLONG i;

	ORB_Orb *oo;

	for (i = 0; i < ORB_MAX_ORBS; i++)
	{
		oo = &ORB_orb[i];

		if (oo->flag & ORB_FLAG_USED)
		{
			//
			// Already in use.
			//
		}
		else
		{
			#define ORB_DENSITY (1.0F)

			oo->flag   = ORB_FLAG_USED;
			oo->x      = x;
			oo->y      = y;
			oo->dx     = 0;
			oo->dy     = 0;
			oo->radius = radius;
			oo->mass   = radius * radius * radius * 4.0F * PI / 3.0F * ORB_DENSITY;

			return oo;
		}
	}

	return NULL;
}



void ORB_process_one(ORB_Orb *oo)
{
	oo->dy -= 0.00007F;

	oo->dx *= 0.999F;
	oo->dy *= 0.999F;

	if (fabs(oo->dx) < 0.00001F) {oo->dx = 0.0F;}
	if (fabs(oo->dy) < 0.00001F) {oo->dy = 0.0F;}

	oo->x += oo->dx;
	oo->y += oo->dy;

	if (oo->x < -ORB_ARENA) {oo->dx =  fabs(oo->dx);}
	if (oo->y < -ORB_ARENA) {oo->dy =  fabs(oo->dy);}
	if (oo->x >  ORB_ARENA) {oo->dx = -fabs(oo->dx);}
	if (oo->y >  ORB_ARENA) {oo->dy = -fabs(oo->dy);}

	//
	// Collide with the landscape.
	//

	float nx;
	float ny;

	oo->flag &= ~ORB_FLAG_COLLIDED;

	if (LAND_collide_sphere(oo->x, oo->y, oo->radius, &nx, &ny))
	{
		//
		// Bounce the ship against the normal.
		//

		float dprod;
		
		dprod = oo->dx*nx + oo->dy*ny;

		oo->dx -= nx * dprod * 2.0F;
		oo->dy -= ny * dprod * 2.0F;

		//
		// Make sure the ship is moving on a positive direction
		// along the normal of the collision point.
		//

		dprod = oo->dx*nx + oo->dy*ny;

		if (dprod < 0.0F)
		{
			oo->dx = -oo->dx;
			oo->dy = -oo->dy;
		}

		oo->x += oo->dx;
		oo->y += oo->dy;

		oo->flag |= ORB_FLAG_COLLIDED;
	}
}



void ORB_process_all()
{
	SLONG i;

	ORB_Orb *oo;

	for (i = 0; i < ORB_MAX_ORBS; i++)
	{
		oo = &ORB_orb[i];

		if (oo->flag & ORB_FLAG_USED)
		{
			ORB_process_one(oo);
		}
	}
}


void ORB_draw_all(float mid_x, float mid_y, float zoom)
{
	SLONG i;

	ORB_Orb   *oo;
	OS_Buffer *ob;

	ob = OS_buffer_new();

	for (i = 0; i < ORB_MAX_ORBS; i++)
	{
		oo = &ORB_orb[i];

		if (oo->flag & ORB_FLAG_USED)
		{
			OS_buffer_add_sprite_rot(
				ob,
				0.5F + (oo->x - mid_x) * zoom,
				0.5F - (oo->y - mid_y) * zoom * 1.33F,
				oo->radius * zoom,
				0.0F);
		}
	}

	OS_buffer_draw(ob, ORB_ot, NULL, OS_DRAW_ADD);
}
