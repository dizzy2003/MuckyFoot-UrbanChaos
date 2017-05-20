#include "game.h"
#include <MFStdLib.h>
#include "glitter.h"

#ifndef	PSX

//
// The sparks.
//

#define GLITTER_SPARK_LIFE    (32)
#define GLITTER_SPARK_GRAVITY (-2)

typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;
	SBYTE dx;
	SBYTE dy;
	SBYTE dz;
	UBYTE next;		// The next spark in the linked list.
	UBYTE die;
	UBYTE useless_padding;

} GLITTER_Spark;

#define GLITTER_MAX_SPARKS 128

GLITTER_Spark GLITTER_spark[GLITTER_MAX_SPARKS];
UBYTE         GLITTER_spark_free;

//
// The collections of sparks.
//

typedef struct
{
	UBYTE flag;
	UBYTE spark;	// The linked list of sparks.
	UBYTE next;		// On this mapwho.
	UBYTE map_x;
	UBYTE map_z;
	UBYTE red;
	UBYTE green;
	UBYTE blue;

} GLITTER_Glitter;

#define GLITTER_MAX_GLITTER 32

GLITTER_Glitter GLITTER_glitter[GLITTER_MAX_GLITTER];
SLONG           GLITTER_glitter_last;

//
// The glitter mapwho.
//

#define GLITTER_MAPWHO 128

UBYTE GLITTER_mapwho[GLITTER_MAPWHO];


void GLITTER_init(void)
{
	SLONG i;

	//
	// Clear the mapwho.
	//

	for (i = 0; i < GLITTER_MAPWHO; i++)
	{
		GLITTER_mapwho[i] = NULL;
	}

	//
	// Mark all the glitter as unused.
	//

	for (i = 0; i < GLITTER_MAX_GLITTER; i++)
	{
		GLITTER_glitter[i].flag &= ~GLITTER_FLAG_USED;
	}

	GLITTER_glitter_last = 1;

	//
	// Build the free list of sparks.
	//

	GLITTER_spark_free = 1;

	for (i = 1; i < GLITTER_MAX_SPARKS - 1; i++)
	{
		GLITTER_spark[i].next = i + 1;
	}

	GLITTER_spark[GLITTER_MAX_SPARKS - 1].next = 0;
}



UBYTE GLITTER_create(
		UBYTE flag,
		UBYTE map_x,
		UBYTE map_z,
		ULONG colour)
{
	SLONG i;

	GLITTER_Glitter *gg;

	if (!WITHIN(map_z, 0, GLITTER_MAPWHO - 1))
	{
		//
		// Off the map.
		//

		return NULL;
	}

	//
	// Look for a spare glitter structure.
	//

	for (i = 0; i < GLITTER_MAX_GLITTER; i++)
	{
		GLITTER_glitter_last += 1;

		if (GLITTER_glitter_last >= GLITTER_MAX_GLITTER)
		{
			GLITTER_glitter_last = 1;
		}

		gg = &GLITTER_glitter[GLITTER_glitter_last];

		if (!(gg->flag & GLITTER_FLAG_USED))
		{
			goto found_unused_glitter;
		}
	}

	//
	// No spare glitter.
	//

	return NULL;

  found_unused_glitter:;

	//
	// Initialise the new glitter.
	//

	gg->flag  =  flag;
	gg->flag |=  GLITTER_FLAG_USED;
	gg->flag &= ~GLITTER_FLAG_DESTROY;
	gg->map_x =  map_x;
	gg->map_z =  map_z;
	gg->red   = (colour >> 16) & 0xff;
	gg->green = (colour >>  8) & 0xff;
	gg->blue  = (colour >>  0) & 0xff;
	gg->spark =  NULL;

	//
	// Add te glitter to the mapwho.
	//

	gg->next              = GLITTER_mapwho[map_z];
	GLITTER_mapwho[map_z] = GLITTER_glitter_last;

	return GLITTER_glitter_last;
}

void GLITTER_add(
		UBYTE glitter,
		SLONG x,
		SLONG y,
		SLONG z)
{
	UBYTE spark;

	if (glitter == NULL)
	{
		return;
	}

	//
	// Are there any sparks in the free list?
	//

	if (GLITTER_spark_free == NULL)
	{
		return;
	}

	//
	// The glitter we are adding the spark to.
	// 

	GLITTER_Glitter *gg;

	ASSERT(WITHIN(glitter, 1, GLITTER_MAX_GLITTER - 1));

	gg = &GLITTER_glitter[glitter];

	//
	// The new spark.
	//

	GLITTER_Spark *gs;

	ASSERT(WITHIN(GLITTER_spark_free, 1, GLITTER_MAX_SPARKS - 1));

	spark              =  GLITTER_spark_free;
	gs                 = &GLITTER_spark[spark];
	GLITTER_spark_free =  gs->next;

	gs->x = x;
	gs->y = y;
	gs->z = z;

	SLONG dx = (rand() & 0x3f) - 0x1f;
	SLONG dy = (rand() & 0x3f) - 0x1f;
	SLONG dz = (rand() & 0x3f) - 0x1f;

	if (gg->flag & GLITTER_FLAG_DXPOS) {dx = +abs(dx);}
	if (gg->flag & GLITTER_FLAG_DXNEG) {dx = -abs(dx);}
	if (gg->flag & GLITTER_FLAG_DYPOS) {dy = +abs(dy);}
	if (gg->flag & GLITTER_FLAG_DYNEG) {dy = -abs(dy);}
	if (gg->flag & GLITTER_FLAG_DZPOS) {dz = +abs(dz);}
	if (gg->flag & GLITTER_FLAG_DZNEG) {dz = -abs(dz);}

	gs->dx = dx;
	gs->dy = dy;
	gs->dz = dz;

	gs->die = GLITTER_SPARK_LIFE;

	//
	// Put the spark in the glitter's linked list.
	//

	gs->next  = gg->spark;
	gg->spark = spark;

	return;
}

void GLITTER_destroy(UBYTE glitter)
{
	if (glitter == NULL)
	{
		return;
	}

	ASSERT(WITHIN(glitter, 1, GLITTER_MAX_GLITTER));

	GLITTER_glitter[glitter].flag |= GLITTER_FLAG_DESTROY;
}


void GLITTER_process()
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	UBYTE  spark;
	UBYTE  next;
	UBYTE *prev;

	GLITTER_Glitter *gg;
	GLITTER_Spark   *gs;

	for (i = 1; i < GLITTER_MAX_GLITTER; i++)
	{
		gg = &GLITTER_glitter[i];

		if (!(gg->flag & GLITTER_FLAG_USED))
		{
			continue;
		}

		//
		// Remove all dead sparks from the glitter.
		//

		next =  gg->spark;
		prev = &gg->spark;
		
		while(1)
		{
			if (next == 0)
			{
				break;
			}

			ASSERT(WITHIN(next, 1, GLITTER_MAX_SPARKS - 1));

			gs = &GLITTER_spark[next];

			if (gs->die == 0)
			{
				spark = next;

				//
				// Take this spark out of the linked list.
				//

			   *prev = gs->next;
				next = gs->next;

				//
				// Put this spark in the free list.
				//

				gs->next           = GLITTER_spark_free;
				GLITTER_spark_free = spark;
			}
			else
			{
				//
				// Go onto the next spark.
				//

				prev = &gs->next;
				next =  gs->next;
			}
		}

		if (gg->flag & GLITTER_FLAG_DESTROY)
		{
			if (gg->spark == NULL)
			{
				//
				// Kill this glitter.
				//

				gg->flag &= ~GLITTER_FLAG_USED;

				//
				// Take it out of the mapwho.
				//

				ASSERT(WITHIN(gg->map_z, 0, GLITTER_MAPWHO - 1));

				prev = &GLITTER_mapwho[gg->map_z];
				next =  GLITTER_mapwho[gg->map_z];

				while(1)
				{
					if (next == i)
					{
						//
						// Found the glitter.
						//

					   *prev = gg->next;

						break;
					}
					else
					{
						ASSERT(WITHIN(next, 1, GLITTER_MAX_GLITTER - 1));

						prev = &GLITTER_glitter[next].next;
						next =  GLITTER_glitter[next].next;
					}
				}

				continue;
			}
		}

		//
		// Process all the sparks in this glitter's linked list.
		//

		for (spark = gg->spark; spark; spark = gs->next)
		{
			ASSERT(WITHIN(spark, 1, GLITTER_MAX_SPARKS - 1));

			gs = &GLITTER_spark[spark];

			//
			// Gravity...
			//

			dy  = gs->dy;
			dy += GLITTER_SPARK_GRAVITY;

			if (dy < -127) {dy = -127;}

			gs->dy = dy;

			gs->x += gs->dx >> 3;
			gs->y += gs->dy >> 3;
			gs->z += gs->dz >> 3;

			gs->die -= 1;
		}
	}
}


UBYTE        GLITTER_get_z;
UBYTE        GLITTER_get_xmin;
UBYTE        GLITTER_get_xmax;
UBYTE        GLITTER_get_glitter;
UBYTE        GLITTER_get_spark;
GLITTER_Info GLITTER_get_info;

void GLITTER_get_start(UBYTE xmin, UBYTE xmax, UBYTE z)
{
	GLITTER_get_z    = z;
	GLITTER_get_xmin = xmin;
	GLITTER_get_xmax = xmax;

	if (WITHIN(GLITTER_get_z, 0, GLITTER_MAPWHO - 1))
	{
		GLITTER_get_glitter = GLITTER_mapwho[GLITTER_get_z];

		if (GLITTER_get_glitter != NULL)
		{
			ASSERT(WITHIN(GLITTER_get_glitter, 1, GLITTER_MAX_GLITTER - 1));

			GLITTER_get_spark = GLITTER_glitter[GLITTER_get_glitter].spark;
		}
	}
	else
	{
		GLITTER_get_glitter = NULL;
		GLITTER_get_spark   = NULL;
	}
}

GLITTER_Info *GLITTER_get_next()
{
	GLITTER_Glitter *gg;
	GLITTER_Spark   *gs;

  tail_recurse:;

	if (GLITTER_get_glitter == NULL)
	{
		return NULL;
	}

	ASSERT(WITHIN(GLITTER_get_glitter, 1, GLITTER_MAX_GLITTER - 1));

	gg = &GLITTER_glitter[GLITTER_get_glitter];

	if (GLITTER_get_spark == NULL)
	{
		ASSERT(WITHIN(gg->next, 0, GLITTER_MAX_GLITTER - 1));

		GLITTER_get_glitter = gg->next;
		GLITTER_get_spark   = GLITTER_glitter[gg->next].spark;

		goto tail_recurse;
	}

	ASSERT(WITHIN(GLITTER_get_spark, 1, GLITTER_MAX_SPARKS - 1));

	gs = &GLITTER_spark[GLITTER_get_spark];

	GLITTER_get_info.x1 = gs->x;
	GLITTER_get_info.y1 = gs->y;
	GLITTER_get_info.z1 = gs->z;
	GLITTER_get_info.x2 = gs->x + (gs->dx >> 2);
	GLITTER_get_info.y2 = gs->y + (gs->dy >> 2);
	GLITTER_get_info.z2 = gs->z + (gs->dz >> 2);

	SLONG red   = (gg->red   * gs->die) / GLITTER_SPARK_LIFE;
	SLONG green = (gg->green * gs->die) / GLITTER_SPARK_LIFE;
	SLONG blue  = (gg->blue  * gs->die) / GLITTER_SPARK_LIFE;

	GLITTER_get_info.colour = (red << 16) | (green << 8) | (blue << 0);

	GLITTER_get_spark = gs->next;

	return &GLITTER_get_info;
}


#endif