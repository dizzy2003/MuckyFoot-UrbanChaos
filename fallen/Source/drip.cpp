#include "game.h"
#include "drip.h"
#include "pap.h"
#include "psystem.h"
#include "poly.h"
#include "puddle.h"
#include "mav.h"

#include "memory.h"

typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;
	UBYTE size;
	UBYTE fade;	// 0 => No drip.
	UBYTE flags;
	
} DRIP_Drip;

#define DRIP_MAX_DRIPS 1024

DRIP_Drip DRIP_drip[DRIP_MAX_DRIPS];
SLONG     DRIP_last;

void DRIP_init()
{
	SLONG i;

	for (i = 0; i < DRIP_MAX_DRIPS; i++)
	{
		DRIP_drip[i].fade = 0;
	}
}

#define DRIP_SFADE (255)
#define DRIP_SSIZE (rand() & 0x7)

#define DRIP_DFADE 16
#define DRIP_DSIZE 4

void DRIP_create(
		UWORD x,
		SWORD y,
		UWORD z,
		UBYTE flags)
{
	DRIP_last += 1;
	DRIP_last &= DRIP_MAX_DRIPS - 1;

	DRIP_drip[DRIP_last].x    = x;
	DRIP_drip[DRIP_last].y    = y;
	DRIP_drip[DRIP_last].z    = z;
	DRIP_drip[DRIP_last].size = DRIP_SSIZE;
	DRIP_drip[DRIP_last].fade = DRIP_SFADE;
	DRIP_drip[DRIP_last].flags= flags;
}

#ifndef TARGET_DC
void DRIP_create_if_in_puddle(
		UWORD x,
		SWORD y,
		UWORD z)
{
	SLONG mx = x >> 8;
	SLONG mz = z >> 8;
	SLONG my;

	if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
		WITHIN(mz, 0, MAP_HEIGHT - 1))
	{
		if (((PAP_2HI(mx,mz).Flags & PAP_FLAG_REFLECTIVE)
		  &&!(PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN ))||
		  (MAV_SPARE(mx,mz) & MAV_SPARE_FLAG_WATER))
		{
		if (PUDDLE_in(x,z))
		{
			DRIP_last += 1;
			DRIP_last &= DRIP_MAX_DRIPS - 1;

			DRIP_drip[DRIP_last].x    = x;
			DRIP_drip[DRIP_last].y    = y;
			DRIP_drip[DRIP_last].z    = z;
			DRIP_drip[DRIP_last].size = DRIP_SSIZE;
			DRIP_drip[DRIP_last].fade = DRIP_SFADE;
			DRIP_drip[DRIP_last].flags= DRIP_FLAG_PUDDLES_ONLY;


			my=PAP_calc_height_at(x,z);
//			if (y<my) y=my+10;

			PARTICLE_Add(
				x<<8,my<<8,z<<8,
				0,0,0,
				POLY_PAGE_SPLASH, 2, 0x7Fffffff,PFLAG_SPRITEANI,
				10,40,1,0,0
				);
		}
		}
	}
}
#endif

void DRIP_process()
{
	SLONG i;
	SLONG fade;
	SLONG size;

	DRIP_Drip *dd;

	for (i = 0; i < DRIP_MAX_DRIPS; i++)
	{
		dd = &DRIP_drip[i];

		if (dd->fade)
		{
			fade = dd->fade;
			size = dd->size;

			fade -= DRIP_DFADE;
			size += DRIP_DSIZE;

			if (fade < 0)
			{
				fade = 0;
			}

			dd->fade = fade;
			dd->size = size;
		}
	}
}

SLONG DRIP_get_upto;

void DRIP_get_start()
{
	DRIP_get_upto = 0;
}

DRIP_Info *DRIP_get_next()
{
	DRIP_Info *di;
	DRIP_Drip *dd;

	while(DRIP_get_upto < DRIP_MAX_DRIPS)
	{
		dd = &DRIP_drip[DRIP_get_upto++];

		if (dd->fade)
		{
			di = (DRIP_Info *) dd;

			return di;
		}
	}

	return NULL;
}







