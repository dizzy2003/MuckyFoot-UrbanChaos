//
// Tractor beams!
// 

#include "always.h"
#include "orb.h"
#include "os.h"
#include "ship.h"
#include "tb.h"


//
// The tractor beams.
//

TB_Tb TB_tb[TB_MAX_TBS];


//
// The texture we use to draw the tractor beams.
//

OS_Texture *TB_ot;


void TB_init()
{
	memset(TB_tb, 0, sizeof(TB_tb));

	TB_ot = OS_texture_create("orb.tga");
}


TB_Tb *TB_create(SHIP_Ship *ss, float length)
{
	SLONG i;

	float dx;
	float dy;
	float dist;

	ORB_Orb *oo;
	TB_Tb   *tt;

	//
	// Look for an orb close to the ship.
	//
	
	for (i = 0; i < ORB_MAX_ORBS; i++)
	{
		oo = &ORB_orb[i];

		if (oo->flag & ORB_FLAG_USED)
		{
			dx = oo->x - ss->x;
			dy = oo->y - ss->y;

			dist = sqrt(dx*dx + dy*dy);

			if (dist < length)
			{
				goto found_an_orb;
			}
		}
	}

	return NULL;

  found_an_orb:;

	//
	// Create a tractor beam between the ship and orb.
	//

	for (i = 0; i < TB_MAX_TBS; i++)
	{
		tt = &TB_tb[i];

		if (tt->flag & TB_FLAG_USED)
		{
			//
			// Already in use.
			//
		}
		else
		{
			//
			// Inialise the new tractor beam.
			//

			tt->flag   = TB_FLAG_USED;
			tt->length = length;
			tt->ship   = ss - SHIP_ship;
			tt->orb    = oo - ORB_orb;

			return tt;
		}
	}

	return NULL;
}


void TB_destroy(TB_Tb *tt)
{
	//
	// Easy!
	// 

	tt->flag = 0;
}


void TB_process_one(TB_Tb *tt)
{
	float dlen;
	float dx;
	float dy;
	float dist;
	float force;

	SHIP_Ship *ss = &SHIP_ship[tt->ship];
	ORB_Orb   *oo = &ORB_orb  [tt->orb ];

	//
	// How far apart are this tractor beams ship and orb.
	//

	dx = ss->x - oo->x;
	dy = ss->y - oo->y;

	dist = sqrt(dx*dx + dy*dy);

	if (!(tt->flag & TB_FLAG_LOCKED))
	{
		//
		// Wait until the ship and the orb are too far apart before applying
		// any force.
		//

		if (dist > tt->length)
		{
			tt->flag |= TB_FLAG_LOCKED;
		}
		else
		{
			return;
		}
	}

	//
	// Too far apart or too close together?
	//

	dlen = dist - tt->length;

	//
	// And what's the force associated with change of length?
	//

	force = dlen * 0.25F;

	dx = dx * force / dist;
	dy = dy * force / dist;

	//
	// Accelerate the ship and orb.
	//

	ss->dx -= dx / ss->mass;
	ss->dy -= dy / ss->mass;

	oo->dx += dx / oo->mass;
	oo->dy += dy / oo->mass;
}


void TB_process_all()
{
	SLONG i;

	TB_Tb *tt;

	//
	// Process all the tractor beams.
	//

	for (i = 0; i < TB_MAX_TBS; i++)
	{
		tt = &TB_tb[i];

		if (tt->flag & TB_FLAG_USED)
		{
			TB_process_one(tt);
		}
	}
}



//
// Draws the tractor beams.
//

void TB_draw_all(float mid_x, float mid_y, float zoom)
{
	SLONG i;
	SLONG j;

	float x;
	float y;

	TB_Tb     *tt;
	SHIP_Ship *ss;
	ORB_Orb   *oo;
	OS_Buffer *ob;

	//
	// Draw all the tractor beams.
	//

	ob = OS_buffer_new();

	for (i = 0; i < TB_MAX_TBS; i++)
	{
		tt = &TB_tb[i];

		if (tt->flag & TB_FLAG_USED)
		{
			ss = &SHIP_ship[tt->ship];
			oo = &ORB_orb  [tt->orb ];

			#ifndef USE_DOTS

			OS_buffer_add_line_2d(
				ob,
				0.5F + (ss->x - mid_x) * zoom,
				0.5F - (ss->y - mid_y) * zoom * 1.33F,
				0.5F + (oo->x - mid_x) * zoom,
				0.5F - (oo->y - mid_y) * zoom * 1.33F,
				0.1F * zoom,
				0.0F, 0.0F,
				1.0F, 1.0F,
				0.0F,
				(tt->flag & TB_FLAG_LOCKED) ? 0x00ff5522 : 0x00888822);

			#else

			#define TB_DOTS_ALONG 32

			for (j = 0; j < TB_DOTS_ALONG; j++)
			{
				x = ss->x + (oo->x - ss->x) * float(j) / float(TB_DOTS_ALONG);
				y = ss->y + (oo->y - ss->y) * float(j) / float(TB_DOTS_ALONG);

				OS_buffer_add_sprite_rot(
					ob,
					0.5F + (x - mid_x) * zoom,
					0.5F - (y - mid_y) * zoom * 1.33F,
					0.1F * zoom,
					0.0F,
					0.0F, 0.0F,
					1.0F, 1.0F,
					0.0F,
					(tt->flag & TB_FLAG_LOCKED) ? 0x00665522 : 0x00888822);
			}

			#endif
		}
	}

	OS_buffer_draw(ob, TB_ot, NULL, OS_DRAW_ADD | OS_DRAW_DOUBLESIDED);
}



