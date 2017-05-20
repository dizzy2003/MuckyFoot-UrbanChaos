//
// A thrust ship.
//

#include "always.h"
#include "game.h"
#include "land.h"
#include "log.h"
#include "os.h"
#include "ship.h"
#include "tb.h"


//
// The ships.
//

SHIP_Ship SHIP_ship[SHIP_MAX_SHIPS];


//
// Our ship texture.
//

OS_Texture *SHIP_ot;


//
// The radius of our ship.
//

#define SHIP_RADIUS (1.0F)


void SHIP_init()
{
	memset(SHIP_ship, 0, sizeof(SHIP_ship));

	SHIP_ot = OS_texture_create("ship.tga");
}

SHIP_Ship *SHIP_create(float x, float y, float mass, float power)
{
	SLONG i;

	SHIP_Ship *ss;

	for (i = 0; i < SHIP_MAX_SHIPS; i++)
	{
		ss = &SHIP_ship[i];

		if (!(ss->flag & SHIP_FLAG_USED))
		{
			ss->flag  = SHIP_FLAG_USED;
			ss->tb    = 255;
			ss->red   = 255;
			ss->green = 255;
			ss->blue  = 255;
			ss->x     = x;
			ss->y     = y;
			ss->dx    = 0.0F;
			ss->dy    = 0.0F;
			ss->angle = 0.0F;
			ss->mass  = mass;
			ss->power = power;

			return ss;
		}
	}

	return NULL;
}


void SHIP_flag_active(SLONG gameturn)
{
	SLONG i;

	SHIP_Ship *ss;

	for (i = 0; i < SHIP_MAX_SHIPS; i++)
	{
		ss = &SHIP_ship[i];

		if (ss->flag & SHIP_FLAG_USED)
		{
			if (ss->active <= gameturn)
			{
				ss->flag |= SHIP_FLAG_ACTIVE;
			}
		}
	}
}



//
// Processes a particular ship.
//

void SHIP_process_one(SHIP_Ship *ss)
{
	if (!(ss->flag & SHIP_FLAG_ACTIVE))
	{
		return;
	}

	static counter;

	LOG_file("Ship %5d = (%f,%f) %f = %f", counter, ss->x, ss->y, ss->angle, ss->x + ss->y + ss->angle);

	{
		union
		{
			ULONG a;
			float b;
		} u1;

		union
		{
			ULONG a;
			float b;
		} u2;

		union
		{
			ULONG a;
			float b;
		} u3;

		u1.b = ss->x;
		u2.b = ss->y;
		u3.b = ss->x + ss->y;

		LOG_file("%d,%d,%d\n", u1.a, u2.a, u3.a);
	}

	counter += 1;

	if (ss->flag & SHIP_FLAG_KEY_LEFT)  {ss->angle += 0.013F;}
	if (ss->flag & SHIP_FLAG_KEY_RIGHT)	{ss->angle -= 0.013F;}

	if (ss->flag & SHIP_FLAG_KEY_THRUST)
	{
		ss->dx += ss->power * sin(ss->angle) * 0.00025F / ss->mass;
		ss->dy -= ss->power * cos(ss->angle) * 0.00025F / ss->mass;
	}

//	if (WITHIN(counter,635,642))
	{
		union
		{
			float f;
			int   i;

		} v[4];

		v[0].f = ss->x;
		v[1].f = ss->y;
		v[2].f = ss->dx;
		v[3].f = ss->dy;

		LOG_file("1: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
	}

	ss->dy -= 0.00007F;

//	if (WITHIN(counter,635,642))
	{
		union
		{
			float f;
			int   i;

		} v[4];

		v[0].f = ss->x;
		v[1].f = ss->y;
		v[2].f = ss->dx;
		v[3].f = ss->dy;

		LOG_file("2: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
	}

	if (fabs(ss->dx) > 0.00004F) {ss->dx *= 0.999F;} else {ss->dx = 0.0F;}
	if (fabs(ss->dy) > 0.00004F) {ss->dy *= 0.999F;} else {ss->dy = 0.0F;}

//	if (WITHIN(counter,635,642))
	{
		union
		{
			float f;
			int   i;

		} v[4];

		v[0].f = ss->x;
		v[1].f = ss->y;
		v[2].f = ss->dx;
		v[3].f = ss->dy;

		LOG_file("3: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
	}

	ss->x += ss->dx;
	ss->y += ss->dy;

//	if (WITHIN(counter,635,642))
	{
		union
		{
			float f;
			int   i;

		} v[4];

		v[0].f = ss->x;
		v[1].f = ss->y;
		v[2].f = ss->dx;
		v[3].f = ss->dy;

		LOG_file("4: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
	}

	if (ss->x < -SHIP_ARENA) {ss->dx =  fabs(ss->dx);}
	if (ss->y < -SHIP_ARENA) {ss->dy =  fabs(ss->dy);}
	if (ss->x >  SHIP_ARENA) {ss->dx = -fabs(ss->dx);}
	if (ss->y >  SHIP_ARENA) {ss->dy = -fabs(ss->dy);}

	//
	// Collide with the landscape.
	//

	float nx;
	float ny;

	ss->flag &= ~SHIP_FLAG_COLLIDED;

	if (LAND_collide_sphere(ss->x, ss->y, SHIP_RADIUS, &nx, &ny))
	{
		LOG_file("    SHIP COLLISION! Ship pos = (%f,%f) vel = (%f,%f)\n", ss->x, ss->y, ss->dx, ss->dy);


		//
		// Bounce the ship against the normal.
		//

		float dprod;
		
		dprod = ss->dx*nx + ss->dy*ny;

		{
			union
			{
				float f;
				int   i;

			} v[4];

			v[0].f = dprod;

			LOG_file("5: %f %d\n", v[0].f, v[0].i);
		}


		{
			union
			{
				float f;
				int   i;

			} v[4];

			v[0].f = nx;
			v[1].f = ny;
			v[2].f = nx * dprod * 2.0F;
			v[3].f = ny * dprod * 2.0F;

			LOG_file("6: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
		}


		ss->dx -= nx * dprod * 2.0F;
		ss->dy -= ny * dprod * 2.0F;

		{
			union
			{
				float f;
				int   i;

			} v[4];

			v[0].f = ss->x;
			v[1].f = ss->y;
			v[2].f = ss->dx;
			v[3].f = ss->dy;

			LOG_file("7: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
		}

		//
		// Make sure the ship is moving on a positive direction
		// along the normal of the collision point.
		//

		dprod = ss->dx*nx + ss->dy*ny;

		if (dprod < 0.0F)
		{
			ss->dx = -ss->dx;
			ss->dy = -ss->dy;
		}


		{
			union
			{
				float f;
				int   i;

			} v[4];

			v[0].f = dprod;

			LOG_file("8: %f %d\n", v[0].f, v[0].i);
		}

		ss->x += ss->dx;
		ss->y += ss->dy;

		{
			union
			{
				float f;
				int   i;

			} v[4];

			v[0].f = ss->x;
			v[1].f = ss->y;
			v[2].f = ss->dx;
			v[3].f = ss->dy;

			LOG_file("9: (%f,%f) (%f,%f)  (%d,%d) (%d,%d)\n", v[0].f, v[1].f, v[2].f, v[3].f, v[0].i, v[1].i, v[2].i, v[3].i);
		}

		ss->flag |= SHIP_FLAG_COLLIDED;
	}

	//
	// Create/destroy a tractor beam.
	//

	if (ss->flag & SHIP_FLAG_KEY_TRACTOR_BEAM)
	{
		ss->flag &= ~SHIP_FLAG_KEY_TRACTOR_BEAM;

		if (ss->flag & SHIP_FLAG_TRACTORING)
		{
			//
			// Get rid of the current tractor beam.
			//

			TB_destroy(&TB_tb[ss->tb]);

			//
			// No longer got a tractor beam.
			//

			ss->flag &= ~SHIP_FLAG_TRACTORING;
			ss->tb    =  255;
		}
		else
		{
			TB_Tb *tb;

			//
			// Try to create a tractor beam.
			//

			tb = TB_create(ss, 5.0F);
			
			if (tb)
			{
				ss->tb    = tb - TB_tb;
				ss->flag |= SHIP_FLAG_TRACTORING;
			}
		}
	}
}



void SHIP_process_all()
{
	SLONG i;

	SHIP_Ship *ss;

	for (i = 0; i < SHIP_MAX_SHIPS; i++)
	{
		ss = &SHIP_ship[i];

		if (!(ss->flag & SHIP_FLAG_USED))
		{
			continue;
		}

		SHIP_process_one(ss);
	}
}



void SHIP_draw_all(float mid_x, float mid_y, float zoom)
{
	SLONG i;
	ULONG colour;

	SHIP_Ship *ss;
	OS_Buffer *ob;

	ob = OS_buffer_new();

	for (i = 0; i < SHIP_MAX_SHIPS; i++)
	{
		ss = &SHIP_ship[i];

		if (!(ss->flag & SHIP_FLAG_USED))
		{
			continue;
		}

		colour  = ss->red   << 16;
		colour |= ss->green <<  8;
		colour |= ss->blue  <<  0;

		if ((GAME_turn & 0x4) && !(ss->flag & SHIP_FLAG_ACTIVE))
		{
			//
			// Inactive ships flash.
			//

			colour &= 0x7f7f7f7f;
		}

		OS_buffer_add_sprite_rot(
			ob,
			0.5F + (ss->x - mid_x) * zoom,
			0.5F - (ss->y - mid_y) * zoom * 1.33F,
			SHIP_RADIUS * zoom,
			ss->angle,
			0.0F, 0.0F,
			1.0F, 1.0F,
			0.0F,
			0xffffff);
	}

	OS_buffer_draw(ob, SHIP_ot, NULL, OS_DRAW_ADD);
}

