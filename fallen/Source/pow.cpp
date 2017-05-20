//
// Yet another explosion system.
//

#include "game.h"
#include "pow.h"
#include "fmatrix.h"
#ifndef PSX
#include "panel.h"
#else
#include "psxeng.h"
#endif


//
// Our data structures.
//

POW_Sprite POW_sprite[POW_MAX_SPRITES];
UBYTE      POW_sprite_free;

POW_Pow POW_pow[POW_MAX_POWS];
UBYTE   POW_pow_free;
UBYTE	POW_pow_used;

UBYTE POW_mapwho[PAP_SIZE_LO];

//
// POW flags. They remember if the've spawned a child yet.
//

#define POW_FLAG_SPAWNED_1	(1 << 0)
#define POW_FLAG_SPAWNED_2	(1 << 1)
#define POW_FLAG_SPAWNED_3	(1 << 2)

//
// The number of ticks per second for counters in the POW module.
//

#define POW_TICKS_PER_SECOND 2000

//
// The shift we use on (dx,dy,dz)s so they can be SWORDS...
// 

#define POW_DELTA_SHIFT 4

//
// The maximum speed a sprite goes through each one of its frames.
//

#define POW_MAX_FRAME_SPEED (POW_TICKS_PER_SECOND / 25)


//
// Info for a POW spawning a child POW.
//

#define POW_SPAWN_FLAG_MIDDLE  (1 << 0)
#define POW_SPAWN_FLAG_AWAY    (1 << 1)	// Makes it move away from the middle of its parent
#define POW_SPAWN_FLAG_UPPER   (1 << 2)	// Only swawn children above yourself
#define POW_SPAWN_FLAG_FAR_OFF (1 << 3)	// Spawn children far away (not for use with POW_SPAWN_FLAG_MIDDLE!)

typedef struct
{
	UWORD when;	// Spawn this pow with the life of the parent gets lower than this value. NULL => NA
	UBYTE type;
	UBYTE flag;

} POW_Spawn;


//
// The characteristics of each explosion type.
//

#define POW_ARRANGE_SPHERE     0
#define POW_ARRANGE_SEMISPHERE 1
#define POW_ARRANGE_CIRCLE     2
#define POW_ARRANGE_NOTHING	   3

#define POW_TYPE_MAX_SPAWN 3

typedef struct
{
	unsigned int arrange    : 2;
	unsigned int speed      : 2;
	unsigned int density    : 2;
	unsigned int framespeed : 2;
	unsigned int damp       : 2;
	unsigned int padding    : 6;

	UWORD     life;
	POW_Spawn spawn[POW_TYPE_MAX_SPAWN];

} POW_Type;


POW_Type POW_type[POW_TYPE_NUMBER] =
{
	// Unused...

	{
		POW_ARRANGE_NOTHING
	},

	// Basic large sphere

	{
		POW_ARRANGE_SPHERE,
		1,					// Speed
		3,					// Density
		0,					// Framespeed
		0,					// Damping
		0,					// Padding
		1,					// Life
	},

	// Basic medium sphere

	{
		POW_ARRANGE_SPHERE,
		2,					// Speed
		1,					// Density
		1,					// Framespeed
		0,					// Damping
		0,					// Padding
		1,					// Life
	},

	// Basic small sphere

	{
		POW_ARRANGE_SPHERE,
		3,					// Speed
		0,					// Density
		0,					// Framespeed
		1,					// Damping
		0,					// Padding
		1,					// Life
	},

	//
	// Large bang with 3 medium bangs.
	//

	{
		POW_ARRANGE_SPHERE,
		1,					// Speed
		3,					// Density
		0,					// Framespeed
		0,					// Damping
		0,					// Padding
		POW_TICKS_PER_SECOND * 4 >> 3,
		{
			{
				POW_TICKS_PER_SECOND * 3 >> 3,
				POW_TYPE_BASIC_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			},

			{
				POW_TICKS_PER_SECOND * 2 >> 3,
				POW_TYPE_BASIC_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			},

			{
				POW_TICKS_PER_SECOND * 1 >> 3,
				POW_TYPE_BASIC_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			}
		}
	},

	//
	// Medium bang with 3 small bangs.
	//

	{
		POW_ARRANGE_SPHERE,
		2,					// Speed
		1,					// Density
		1,					// Framespeed
		0,					// Damping
		0,					// Padding
		POW_TICKS_PER_SECOND * 4 >> 3,
		{
			{
				POW_TICKS_PER_SECOND * 3 >> 3,
				POW_TYPE_BASIC_SPHERE_SMALL,
				POW_SPAWN_FLAG_AWAY
			},

			{
				POW_TICKS_PER_SECOND * 2 >> 3,
				POW_TYPE_BASIC_SPHERE_SMALL,
				POW_SPAWN_FLAG_AWAY
			},

			{
				POW_TICKS_PER_SECOND * 1 >> 3,
				POW_TYPE_BASIC_SPHERE_SMALL,
				POW_SPAWN_FLAG_AWAY
			},
		}
	},

	//
	// Large bang with 3 spawning medium bangs.
	//

	{
		POW_ARRANGE_SPHERE,
		1,					// Speed
		3,					// Density
		0,					// Framespeed
		0,					// Damping
		0,					// Padding
		POW_TICKS_PER_SECOND * 4 >> 3,
		{
			{
				POW_TICKS_PER_SECOND * 3 >> 3,
				POW_TYPE_SPAWN_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			},

			{
				POW_TICKS_PER_SECOND * 2 >> 3,
				POW_TYPE_SPAWN_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			},

			{
				POW_TICKS_PER_SECOND * 1 >> 3,
				POW_TYPE_SPAWN_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			}
		}
	},

	//
	// Large semisphere bang with 3 spawning medium bangs.
	//

	{
		POW_ARRANGE_SEMISPHERE,
		1,					// Speed
		3,					// Density
		0,					// Framespeed
		0,					// Damping
		0,					// Padding
		POW_TICKS_PER_SECOND * 4 >> 3,
		{
			{
				POW_TICKS_PER_SECOND * 3 >> 3,
				POW_TYPE_SPAWN_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			},

			{
				POW_TICKS_PER_SECOND * 2 >> 3,
				POW_TYPE_SPAWN_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			},

			{
				POW_TICKS_PER_SECOND * 1 >> 3,
				POW_TYPE_SPAWN_SPHERE_MEDIUM,
				POW_SPAWN_FLAG_AWAY | POW_SPAWN_FLAG_FAR_OFF
			}
		}
	},
};




//
// Initialises the explosions.
//

void POW_init()
{
	SLONG i;
void	check_pows(void);
		check_pows();

	memset(POW_sprite, 0, sizeof(POW_sprite));
	memset(POW_pow,    0, sizeof(POW_pow   ));
	memset(POW_mapwho, 0, sizeof(POW_mapwho));

	for (i = 1; i < POW_MAX_SPRITES - 1; i++)
	{
		POW_sprite[i].next = i + 1;
	}

	POW_sprite[POW_MAX_SPRITES - 1].next = NULL;

	for (i = 1; i < POW_MAX_POWS - 1; i++)
	{
		POW_pow[i].next = i + 1;
	}

	POW_pow[POW_MAX_POWS - 1].next = NULL;

	POW_sprite_free = 1;
	POW_pow_free    = 1;
}

#ifdef	POO
SLONG	count_occurances(SLONG find)
{
	SLONG	sprite;
	SLONG	count=0;

	sprite=POW_sprite_free;

	while(sprite)
	{

		if(sprite==find)
			count++;


		sprite=POW_sprite[sprite].next;
	}
	return(count);

}

SLONG	count_used(SLONG find)
{
	SLONG	pow,count2,sprite;
	POW_Pow    *pp;
	POW_Sprite *ps;
	POW_Type   *pt;

	SLONG	ret=0;

	for (pow = POW_pow_used; pow&& count2++<50; pow = pp->next)
	{
		ASSERT(WITHIN(pow, 1, POW_MAX_POWS - 1));

		pp = &POW_pow[pow];

		if (pp->sprite)
		{
			SLONG	count=0;
			//
			// Process the pow's sprites.
			//

			for (sprite = pp->sprite; sprite && count++<256; sprite = ps->next)
			{
				if(sprite==find)
					ret++;
				ps = &POW_sprite[sprite];
			}
		}
	}

	return(ret);

}
#endif
void	check_pows(void)
{
	SLONG	sprite;
#ifdef	POO
	sprite=POW_sprite_free;

	while(sprite)
	{
		ASSERT(count_occurances(sprite)==1);
		ASSERT(count_used(sprite)==0);
		sprite=POW_sprite[sprite].next;
	}
#endif

}


//
// Inserts a sprite into the given pow.
//

void POW_insert_sprite(
		POW_Pow *pp,
		SLONG    x,
		SLONG    y,
		SLONG    z,
		SLONG    dx,	// Not shifted by POW_DELTA_SHIFT...
		SLONG    dy,
		SLONG    dz,
		SLONG    frame_speed,
		SLONG    damp)
{
	//
	// Get a sprite from the free list.
	// 

	SLONG       sprite_index;
	POW_Sprite *ps;
	if (POW_sprite_free == NULL)
	{
#ifndef	PSX
//		PANEL_new_text(NULL, 500, "No more sprites");
#endif

		return;
	}

	ASSERT(WITHIN(POW_sprite_free, 1, POW_MAX_SPRITES - 1));
	{
		SLONG	sprite;
		sprite=pp->sprite;
		while(sprite)
		{
			ASSERT(sprite!=POW_sprite_free);
			sprite=POW_sprite[sprite].next;
		}
	}

	ps              = &POW_sprite[POW_sprite_free];
	sprite_index    =  POW_sprite_free;
	POW_sprite_free =  ps->next;

	//
	// Initialise the sprite.
	//

	ps->x             = x;
	ps->y             = y;
	ps->z             = z;
	ps->dx            = dx >> POW_DELTA_SHIFT;
	ps->dy            = dy >> POW_DELTA_SHIFT;
	ps->dz            = dz >> POW_DELTA_SHIFT;
	ps->frame         = 0;
	ps->frame_counter = POW_MAX_FRAME_SPEED;
	ps->frame_speed   = frame_speed;
	ps->damp          = damp;

	//
	// Insert it into the linked list hanging off this pow.
	//

	ps->next   = pp->sprite;
	pp->sprite = sprite_index;
}




//
// Initialises a new pow.
//

void POW_new(SLONG type, SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz)
{
	SLONG i;

	SLONG yaw;
	SLONG pitch;

	SLONG around;
	SLONG upndown;
	SLONG ring;

	SLONG dyaw;
	SLONG dpitch;

	SLONG pow_index;
	SLONG framespeed;

	SLONG vector[3];

	POW_Type *pt;
	POW_Pow  *pp;

	ASSERT(WITHIN(type, 0, POW_TYPE_NUMBER - 1));

	pt = &POW_type[type];

	//
	// Get a pow from the free list.
	//

	if (POW_pow_free == NULL)
	{
#ifndef	PSX
//		PANEL_new_text(NULL, 1000, "No more pows");
#endif

		return;
	}

	ASSERT(WITHIN(POW_pow_free, 1, POW_MAX_POWS - 1));

	pp           = &POW_pow[POW_pow_free];
	pow_index    =  POW_pow_free;
	POW_pow_free =  pp->next;

	//
	// Initialise the pow.
	//

	pp->type      = type;
	pp->timer     = pt->life;
	pp->x         = x;
	pp->y         = y;
	pp->z         = z;
	pp->dx        = dx >> POW_DELTA_SHIFT;
	pp->dy        = dy >> POW_DELTA_SHIFT;
	pp->dz        = dz >> POW_DELTA_SHIFT;
	pp->mapwho    = NULL;
	pp->next      = NULL;
	pp->sprite    = NULL;
	pp->flag      = 0;
	pp->time_warp = Random();

	//
	// Sprite density?
	// 

	around  = 5 + pt->density * 3;
	upndown = 4 + pt->density * 1;

	//
	// Framespeed.
	//

	framespeed = 96 + pt->framespeed * 32;

	//
	// What arrangement of sprites do we want?
	//

	if (pt->arrange == POW_ARRANGE_CIRCLE)
	{
	}
	else
	{
		//
		// A sphere or semi-sphere.
		// 

		if (pt->arrange == POW_ARRANGE_SPHERE)
		{
			dpitch = 1024 / upndown;
			pitch  = -512 + dpitch / 2;
		}
		else
		{
			dpitch = 512 / ((upndown + 1) / 2);
			pitch  = dpitch / 2;
		}

		while(pitch < 512)
		{
			//
			// How many sprites in this particular ring?
			//

			ring   = COS(pitch & 2047);
			ring  *= around;
			ring >>= 16;

			dyaw = 2048 / ring;
			yaw  = 0;
			
			for (i = 0; i < ring; i++)
			{
				FMATRIX_vector(
					vector,
					yaw,
					pitch);

				POW_insert_sprite(
					pp,
					x,y,z,
					(vector[0] + (Random() & 0x3fff) >> (pt->speed + 1)) + dx,
					(vector[1] + (Random() & 0x3fff) >> (pt->speed + 1)) + dy,
					(vector[2] + (Random() & 0x3fff) >> (pt->speed + 1)) + dz,
					framespeed + (Random() & 0x3f),
					pt->damp + 1);

				yaw += dyaw;
			}

			pitch += dpitch;
		}
	}

	/*

	POW_insert_sprite(
		pp,
		pp->x,
		pp->y,
		pp->z,
		0,
		0,
		0,
		128,
		0);

	*/

	//
	// Insert into the used POW list.
	//

	pp->next     = POW_pow_used;
	POW_pow_used = pow_index;
}











void POW_process()
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG pow;
	SLONG yaw;
	SLONG pitch;
	SLONG sprite;
	SLONG vector[3];

	UBYTE  next;
	UBYTE *prev;

	POW_Pow    *pp;
	POW_Sprite *ps;
	POW_Type   *pt;

	SLONG ticks;
	SLONG frame_ticks;
	SLONG frame_counter;
	SLONG pow_index;
	SLONG sprite_index;

	SLONG	count2=0;
	
	ticks = (POW_TICKS_PER_SECOND / 20) * TICK_RATIO >> TICK_SHIFT;

	//
	// Process all the pows.
	//

	for (pow = POW_pow_used; pow&& count2++<50; pow = pp->next)
	{
		ASSERT(WITHIN(pow, 1, POW_MAX_POWS - 1));

		pp = &POW_pow[pow];

		if (pp->sprite)
		{
			SLONG	count=0;
			//
			// Process the pow's sprites.
			//

			for (sprite = pp->sprite; sprite && count++<256; sprite = ps->next)
			{
				

				ASSERT(WITHIN(sprite, 1, POW_MAX_SPRITES - 1));

				ps = &POW_sprite[sprite];

				//				
				// Animation.
				//

				frame_ticks    = ticks * ps->frame_speed >> 8;
				frame_counter  = ps->frame_counter;
				frame_counter -= frame_ticks;

				while(frame_counter <= 0)
				{
					frame_counter += POW_MAX_FRAME_SPEED;
					ps->frame     += 1;
				}

				ps->frame_counter = frame_counter;

				//
				// Movement.
				//

				ps->x += ps->dx << POW_DELTA_SHIFT;
				ps->y += ps->dy << POW_DELTA_SHIFT;
				ps->z += ps->dz << POW_DELTA_SHIFT;

				//
				// Damping.
				//

				ps->dx -= ps->dx >> ps->damp;
				ps->dy -= ps->dy >> ps->damp;
				ps->dz -= ps->dz >> ps->damp;
			}

			//
			// Get rid of all unused sprites.
			//

			next =  pp->sprite;
			prev = &pp->sprite;

			count=0;
			while(count++<256)
			{
				if (next == NULL)
				{
					break;
				}
				
				ASSERT(WITHIN(next, 1, POW_MAX_SPRITES - 1));

				ps = &POW_sprite[next];

				if (ps->frame >= 16)
				{
					//
					// This sprite is dead. Take it out of the pow's list.
					// 

					sprite_index = next;
				   *prev         = ps->next;
					next         = ps->next;

					//
					// Add it to the free sprite list.
					//

					ps->next        = POW_sprite_free;
					POW_sprite_free = sprite_index;
				}
				else
				{
					prev = &ps->next;
					next =  ps->next;
				}
			}
			if(count>=256)
			{
//				ASSERT(0);
				POW_init();
				return;
			}
		}

		if (pp->timer <= ticks)
		{
			pp->timer = 0;
		}
		else
		{
			pp->timer -= ticks;
		}
	}

	if(count2>=50)
	{
//		ASSERT(0);
		POW_init();
		return;
	}
	//
	// Get rid of the unused pows.
	//

	next =  POW_pow_used;
	prev = &POW_pow_used;

	count2=0;
	while(count2++<50)
	{
		if (next == NULL)
		{
			break;
		}

		ASSERT(WITHIN(next, 1, POW_MAX_POWS - 1));

		pp = &POW_pow[next];

		if (pp->sprite == NULL &&
			pp->timer  == NULL)
		{
			//
			// This pow is dead. Take it out of the used list
			// and plonk it in the free list.
			//

			pow_index = next;
		   *prev      = pp->next;
		    next      = pp->next;	

			pp->next     = POW_pow_free;
			POW_pow_free = pow_index;

			pp->type = POW_TYPE_UNUSED;
		}
		else
		{
			prev = &pp->next;
			next =  pp->next;
		}
	}
	if(count2>=50)
	{
//		ASSERT(0);
		POW_init();
		return;
	}


	//
	// Pows spawn off child pows...
	//

	for (i = 1; i < POW_MAX_POWS; i++)
	{
		pp = &POW_pow[i];

		if (pp->type == POW_TYPE_UNUSED)
		{
			continue;
		}

		ASSERT(WITHIN(pp->type, 1, POW_TYPE_NUMBER - 1));

		pt = &POW_type[pp->type];

		for (j = 0; j < 3; j++)
		{
			if (pt->spawn[j].when)
			{
				if (pt->spawn[j].when > pp->timer)
				{
					if (!(pp->flag & (1 << j)))
					{
						//
						// Spawn this pow.
						//

						pp->flag |= 1 << j;

						x = pp->x;
						y = pp->y;
						z = pp->z;

						if (pt->spawn[j].flag & POW_SPAWN_FLAG_MIDDLE)
						{
							dx = 0;
							dy = 0;
							dz = 0;
						}
						else
						{
							yaw    = Random() & 2047;
							pitch  = Random() & 255;
							pitch -= 128;

							if (POW_type[pp->type].arrange == POW_ARRANGE_SEMISPHERE)
							{
								pitch  = abs(pitch);
								pitch += 128;
							}

							pitch &= 2047;

							FMATRIX_vector(
								vector,
								yaw,
								pitch);

							dx = vector[0];
							dy = vector[1];
							dz = vector[2];

							dx >>= 2;
							dy >>= 2;
							dz >>= 2;

							if (pt->spawn[j].flag & POW_SPAWN_FLAG_FAR_OFF)
							{
								dx += dx >> 1;
								dy += dy >> 1;
								dz += dz >> 1;
							}

							x += dx;
							y += dy;
							z += dz;
						}

						if (pt->spawn[j].flag & POW_SPAWN_FLAG_AWAY)
						{
							dx >>= 2;
							dy >>= 2;
							dz >>= 2;
						}
						else
						{
							dx = 0;
							dy = 0;
							dz = 0;
						}

						dx += (Random() & 0xfff) - 0x7ff;
						dy += (Random() & 0xfff) - 0x7ff;
						dz += (Random() & 0xfff) - 0x7ff;

						POW_new(
							pt->spawn[j].type,
							x,
							y,
							z,
							dx,
							dy,
							dz);
					}
				}
			}
		}
	}

/*
	for (i = 1; i < POW_MAX_POWS; i++)
	{
		pp = &POW_pow[i];

		if (pp->type == POW_TYPE_UNUSED)
		{
			continue;
		}

		//
		// Does this pow have a circular sprite queue?
		//

		j = 0;

		for (sprite = pp->sprite; sprite; sprite = POW_sprite[sprite].next)
		{
			ASSERT(WITHIN(sprite, 1, POW_MAX_SPRITES - 1));

			j++;

			ASSERT(j <= POW_MAX_SPRITES);
		}
	}
*/
}

