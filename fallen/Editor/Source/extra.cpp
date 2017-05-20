#include	"Editor.hpp"

#include <MFStdLib.h>
#include "extra.h"


//
// The things.
// 

EXTRA_Thing EXTRA_thing[EXTRA_MAX_THINGS];


void EXTRA_create_or_delete(SLONG type, SLONG x, SLONG z)
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	EXTRA_Thing *et;

	//
	// look for a thing to delete.
	//

	for (i = 0; i < EXTRA_MAX_THINGS; i++)
	{
		et = &EXTRA_thing[i];

		if (et->type == type)
		{
			dx = abs(et->x - x);
			dz = abs(et->z - z);

			dist = QDIST2(dx,dz);

			if (dist < EXTRA_SELECT_DIST)
			{
				//
				// Delete this thing.
				//

				et->type = EXTRA_TYPE_NONE;

				return;
			}
		}
	}

	//
	// Look for a spare thing structure.
	//

	for (i = 0; i < EXTRA_MAX_THINGS; i++)
	{
		et = &EXTRA_thing[i];

		if (et->type == EXTRA_TYPE_NONE)
		{
			switch(type)
			{
				case EXTRA_TYPE_PUDDLE:
					et->type   = EXTRA_TYPE_PUDDLE;
					et->x      = x;
					et->z      = z;
					et->radius = 384;
					et->angle  = 0;
					return;

				case EXTRA_TYPE_MIST:
					et->type    = EXTRA_TYPE_MIST;
					et->x       = x;
					et->z       = z;
					et->radius  = 0x600;
					et->height  = rand() & 63;
					et->height += 50;
					et->detail  = 17;
					return;

				default:
					ASSERT(0);
					break;
			}
		}
	}
}


