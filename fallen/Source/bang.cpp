//
// Explosions!
//

#include "game.h"
#include <MFStdLib.h>
#include "bang.h"

//
// Parameters for each phwoar type.
//

#define BANG_CHILD_WHERE_INSIDE 0
#define BANG_CHILD_WHERE_EDGE	1
#define BANG_CHILD_WHERE_MIDDLE	2

#define BANG_MAX_CHILDREN 6

typedef struct
{
	UBYTE initial_radius;
	UBYTE initial_grow;
	UBYTE initial_r;   
	UBYTE initial_g;
	UBYTE initial_b;
	SBYTE dr;	// 4-bit fixed point...
	SBYTE dg;
	SBYTE db;
	UBYTE dgrow;
	UBYTE die;

	struct
	{
		UBYTE counter;
		UBYTE type;
		UBYTE where;

	} child[BANG_MAX_CHILDREN];
	
} BANG_Type;

#define BANG_TYPE_BIG		0
#define BANG_TYPE_MIDDLE	1
#define BANG_TYPE_NEARLY	2
#define BANG_TYPE_END		3
#define BANG_TYPE_NUMBER	4

BANG_Type BANG_type[BANG_TYPE_NUMBER] =
{
	{	// Big ... invisible!
		16,
		0,
		0, 0, 0,
	    0, 0, 0,
		0,
		18,
		{
			{ 1,BANG_TYPE_MIDDLE,BANG_CHILD_WHERE_MIDDLE},
			{ 1,BANG_TYPE_MIDDLE,BANG_CHILD_WHERE_MIDDLE},
			{ 2,BANG_TYPE_MIDDLE,BANG_CHILD_WHERE_INSIDE},
			{ 2,BANG_TYPE_MIDDLE,BANG_CHILD_WHERE_INSIDE},
			{ 7,BANG_TYPE_MIDDLE,BANG_CHILD_WHERE_MIDDLE},
			{12,BANG_TYPE_MIDDLE,BANG_CHILD_WHERE_MIDDLE}
		}
	},

	{	// Middle
		0,
		120,
		255, 255, 255,
		-8, -10, -12,
		1,
		16,
		{
			{1,BANG_TYPE_NEARLY,BANG_CHILD_WHERE_EDGE},
			{2,BANG_TYPE_NEARLY,BANG_CHILD_WHERE_EDGE},
			{1,BANG_TYPE_NEARLY,BANG_CHILD_WHERE_EDGE},
			{2,BANG_TYPE_NEARLY,BANG_CHILD_WHERE_EDGE},
			{0,0,0},
			{0,0,0}
		}
	},

	{	// Nearly
		0,
		90,
		255, 255, 255,
		-8, -10, -12,
		1,
		18,
		{
			{0,0,0},
			{0,0,0},

			{1,BANG_TYPE_END,BANG_CHILD_WHERE_EDGE},
			{2,BANG_TYPE_END,BANG_CHILD_WHERE_EDGE},
			{3,BANG_TYPE_END,BANG_CHILD_WHERE_EDGE},
			{4,BANG_TYPE_END,BANG_CHILD_WHERE_EDGE},
		}
	},

	{	// End
		0,
		40,
		255, 255, 255,
		-8, -10, -12,
		2,
		20,
		{
			{0,0,0},
			{0,0,0},
			{0,0,0},
			{0,0,0},
			{0,0,0},
			{0,0,0}
		}
	}
};

//
// Each bit of an explosion.
//

typedef struct
{
	UBYTE type;
	SBYTE x;	// Relative to the bang whose linked-list the phwoar is in.
	SBYTE y;
	SBYTE z;
	SBYTE dx;	// Normalised to 64
	SBYTE dy;
	SBYTE dz;
	UBYTE radius;
	UBYTE grow;	// 2-bit fixed point.
	UBYTE counter;
	UWORD next;	// The next phwoar.

} BANG_Phwoar;

#define BANG_MAX_PHWOARS 4096

BANG_Phwoar BANG_phwoar[BANG_MAX_PHWOARS];
UBYTE       BANG_phwoar_free;

//
// An explosion.
//

typedef struct
{
	UWORD index;	// Linked list into the phwoar array- NULL => bang is unused.
	UWORD next;		// Linked list on a mapwho square NULL terminated.
	UWORD x;
	SWORD y;
	UWORD z;
	
} BANG_Bang;

#define BANG_MAX_BANGS 64

BANG_Bang BANG_bang[BANG_MAX_BANGS];
SLONG     BANG_last;

//
// The BANG mapwho.
//

#define BANG_MAPWHO 128

UWORD BANG_mapwho[BANG_MAPWHO];


void BANG_init()
{
	SLONG i;

	//
	// Initialise the bangs.
	//

	for (i = 0; i < BANG_MAX_BANGS; i++)
	{
		BANG_bang[i].index = NULL;
		BANG_bang[i].next  = NULL;
	}

	//
	// Initialise the phwoars.
	//

	BANG_phwoar_free = 1;

	for (i = 1; i < BANG_MAX_PHWOARS; i++)
	{
		BANG_phwoar[i].next = i + 1;
	}

	BANG_phwoar[BANG_MAX_PHWOARS - 1].next = NULL;

	//
	// Initialise the mapwho.
	//

	for (i = 0; i < BANG_MAPWHO; i++)
	{
		BANG_mapwho[i] = NULL;
	}
}

//
// Creates a new phwoar structure. Returns NULL if there
// are no free phwoars.  A NULL 'parent' means that this is
// the first phwaor in a new bang. In this case 'where' is
// ignored and the phwoar is placed at (0,0,0)
//

UWORD BANG_new_phwoar(
		UBYTE parent,
		UBYTE type,
		UBYTE where)
{
	UBYTE ph;
	SLONG dist;

	BANG_Phwoar *bp;
	BANG_Phwoar *pp;

	if (BANG_phwoar_free == NULL)
	{
		//
		// No spare phwoars.
		//

		return NULL;
	}

	ph               = BANG_phwoar_free;
	BANG_phwoar_free = BANG_phwoar[ph].next;

	bp = &BANG_phwoar[ph];

	//
	// What are the coordinates of this phwoar?
	//

	if (parent == NULL)
	{
		//
		// This is the first phwoar of a new bang.
		//

		bp->x  = 0;
		bp->y  = 0;
		bp->z  = 0;
		bp->dx = 0;
		bp->dy = 64;	// Unit vector pointing straight up.
		bp->dz = 0;
	}
	else
	{
		ASSERT(WITHIN(parent, 1, BANG_MAX_PHWOARS - 1));

		pp = &BANG_phwoar[parent];

		//
		// A random vector.
		//

		SLONG dx = (rand() & 0x1ff) - 256;
		SLONG dy = (rand() & 0x1ff) - 256;
		SLONG dz = (rand() & 0x1ff) - 256;

		//
		// Make sure this vector is not in the wrong half of the semisphere.
		//

		SLONG dprod = dx*pp->dx + dy*pp->dy + dz*pp->dz;

		if (dprod < 0)
		{
			//
			// The wrong side... reflect the vector about the plane through
			// the bottom of the semi-sphere.
			//

			dprod   = -dprod;
			dprod >>=  5;

			dx += dprod * pp->dx >> 6;
			dy += dprod * pp->dy >> 6;
			dz += dprod * pp->dz >> 6;
		}

		//
		// How far from the parent?
		//

		SLONG len;

		if (where == BANG_CHILD_WHERE_INSIDE)
		{
			len = pp->radius * (rand() & 0xff) >> 8;
		}
		else
		if (where == BANG_CHILD_WHERE_MIDDLE)
		{
			len = pp->radius * (rand() & 0x3f) >> 8;
		}
		else
		{
			ASSERT(where == BANG_CHILD_WHERE_EDGE);

			len = pp->radius;
		}

		//
		// Normalise (dx,dy,dz) to length 64.
		//

		dist = QDIST3(abs(dx),abs(dy),abs(dz));

		dx = (dx << 6) / dist;
		dy = (dy << 6) / dist;
		dz = (dz << 6) / dist;

		//
		// This is the vector the (semi)sphere points in.
		//

		bp->dx = dx;
		bp->dy = dy;
		bp->dz = dz;

		//
		// Make (dx,dy,dz) the 'len' length long to find out the phwoars position.
		//

		dx = (dx * len) >> 6;
		dy = (dy * len) >> 6;
		dz = (dz * len) >> 6;

		SLONG px = pp->x + dx;
		SLONG py = pp->y + dy;
		SLONG pz = pp->z + dz;

		SATURATE(px, -127, +127);
		SATURATE(py, -127, +127);
		SATURATE(pz, -127, +127);
		
		bp->x = px;
		bp->y = py;
		bp->z = pz;
	}

	ASSERT(WITHIN(type, 0, BANG_TYPE_NUMBER - 1));

	bp->type    = type;
	bp->radius  = BANG_type[type].initial_radius;
	bp->grow    = BANG_type[type].initial_grow;
	bp->counter = 0;
	bp->next    = NULL;

	return ph;
}

void BANG_process()
{
	SLONG  i;
	SLONG  j;
	UWORD  ph;
	UWORD  kid;
	UWORD  next;
	UWORD *prev;

	BANG_Bang   *bb;
	BANG_Phwoar *bp;
	BANG_Type   *bt;

	//
	// Go through all the bangs to find the active phwoars.
	//

	for (i = 1; i < BANG_MAX_BANGS; i++)
	{
		bb = &BANG_bang[i];

		if (bb->index == NULL)
		{
			continue;
		}

		for (ph = bb->index, prev = &bb->index; ph; ph = bp->next)
		{
			ASSERT(WITHIN(ph,                   1, BANG_MAX_PHWOARS - 1));
			ASSERT(WITHIN(BANG_phwoar[ph].type, 0, BANG_TYPE_NUMBER - 1));

			bp = &BANG_phwoar[ph];
			bt = &BANG_type  [bp->type];

			//
			// Next turn...
			//

			bp->counter += 1;

			//
			// Change the radius 
			//

			SLONG radius;
			SLONG grow;
			
			radius    = bp->radius;
			grow      = bp->grow;

			radius   += bp->grow >> 2;
			radius   += 1;
			grow     -= bp->grow >> bt->dgrow;
			grow     -= 1;

			if (radius > 255) {radius = 255;}
			if (grow   <   0) {grow   =   0;}

			bp->radius = radius;
			bp->grow   = grow;

			//
			// Spawn off children.
			//

			for (j = 0; j < BANG_MAX_CHILDREN; j++)
			{
				if (bt->child[j].counter == bp->counter)
				{
					//
					// Create a new phwoar.
					//

					kid = BANG_new_phwoar(
							ph,
							bt->child[j].type,
							bt->child[j].where);

					if (kid)
					{
						ASSERT(WITHIN(kid, 1, BANG_MAX_PHWOARS - 1));

						//
						// Insert this phwoar into the linked list for this bang.
						//

						BANG_phwoar[kid].next = bb->index;
						bb->index             = kid;
					}
				}
			}
		}

		//
		// Make another pass through deleting dead phwoars.
		//

		prev = &bb->index;
		next =  bb->index;

		while(next)
		{
			ASSERT(WITHIN(next,                   1, BANG_MAX_PHWOARS - 1));
			ASSERT(WITHIN(BANG_phwoar[next].type, 0, BANG_TYPE_NUMBER - 1));

			bp = &BANG_phwoar[next];
			bt = &BANG_type  [bp->type];

			if (bp->counter >= bt->die)
			{
				UBYTE this_phwoar = next;

				//
				// Kill this phwoar.
				//

				*prev = bp->next;
				 next = bp->next;

				 //
				 // Put it into the free list.
				 //

				 bp->next         = BANG_phwoar_free;
				 BANG_phwoar_free = this_phwoar;
			}
			else
			{
				//
				// The phwoar lives.
				//

				prev = &bp->next;
				next =  bp->next;
			}
		}

		//
		// Is this bang dead?	  Hello Mark! love Jan
		//

		if (bb->index == NULL)
		{
			//
			// Remove the bang from the mapwho.
			//

			SLONG mapz = bb->z >> 8;

			if (WITHIN(mapz, 0, BANG_MAPWHO - 1))
			{
				prev = &BANG_mapwho[mapz];
				next =  BANG_mapwho[mapz];

				while(1)
				{
					if (next == i)
					{
						*prev = bb->next;

						break;
					}
					else
					{
						ASSERT(WITHIN(next, 1, BANG_MAX_BANGS - 1));

						prev = &BANG_bang[next].next;
						next =  BANG_bang[next].next;
					}
				}
			}
		}
	}
}

void BANG_create(
	SLONG type,
	SLONG x,
	SLONG y,
	SLONG z)
{
	SLONG i;
	SLONG use;
	SLONG mapz;

	//
	// Look for an unused bang structure to use.
	//

	for (i = 1; i < BANG_MAX_BANGS; i++)
	{
		BANG_last += 1;
		BANG_last &= BANG_MAX_BANGS - 1;

		if (BANG_last == 0)
		{
			BANG_last = 1;
		}

		if (BANG_bang[BANG_last].index == NULL)
		{
			goto found_unused_bang;
		}
	}

	//
	// No free bang structures.
	//

	return;

  found_unused_bang:;

	//
	// Use BANG_last.
	//

	BANG_bang[BANG_last].x     = x;
	BANG_bang[BANG_last].y     = y;
	BANG_bang[BANG_last].z     = z;
	BANG_bang[BANG_last].index = BANG_new_phwoar(NULL, type, 0);

	//
	// Insert it into the mapwho.
	//

	mapz = z >> 8;

	if (WITHIN(mapz, 0, BANG_MAPWHO - 1))
	{
		BANG_bang[BANG_last].next = BANG_mapwho[mapz];
		BANG_mapwho[mapz]         = BANG_last;
	}
	else
	{
		//
		// Oh well. Nobody is ever going to see it!
		//

		BANG_bang[BANG_last].next = NULL;
	}
}



UBYTE     BANG_get_xmin;
UBYTE     BANG_get_xmax;
UBYTE     BANG_get_z;
UWORD     BANG_get_bang;
UWORD     BANG_get_phwoar;
BANG_Info BANG_get_info;

void BANG_get_start(UBYTE xmin, UBYTE xmax, UBYTE z)
{
	if (WITHIN(z, 0, BANG_MAPWHO - 1))
	{
		BANG_get_bang   = BANG_mapwho[z];
		BANG_get_phwoar = NULL;
		BANG_get_xmin   = xmin;
		BANG_get_xmax   = xmax;
	}
	else
	{
		BANG_get_bang   = NULL;
		BANG_get_phwoar = NULL;
	}
}

BANG_Info *BANG_get_next()
{
	BANG_Bang   *bb;
	BANG_Phwoar *bp;
	BANG_Type   *bt;

	if (BANG_get_phwoar == NULL)
	{
		//
		// Look for a bang within the desired x-range.
		//

		while(1)
		{
			if (BANG_get_bang == NULL)
			{
				return NULL;
			}

			ASSERT(WITHIN(BANG_get_bang, 1, BANG_MAX_BANGS - 1));

			bb = &BANG_bang[BANG_get_bang];

			if (WITHIN(bb->x >> 8, BANG_get_xmin, BANG_get_xmax))
			{
				BANG_get_phwoar = bb->index;

				if (BANG_get_phwoar == NULL)
				{
					//
					// This is bad!
					//

					return NULL;
				}

				break;
			}
			else
			{
				BANG_get_bang = bb->next;
			}
		}
	}

	ASSERT(WITHIN(BANG_get_bang  , 1, BANG_MAX_BANGS   - 1));
	ASSERT(WITHIN(BANG_get_phwoar, 1, BANG_MAX_PHWOARS - 1));

	bb = &BANG_bang  [BANG_get_bang];
	bp = &BANG_phwoar[BANG_get_phwoar];

	ASSERT(WITHIN(bp->type, 0, BANG_TYPE_NUMBER - 1));

	bt = &BANG_type[bp->type];

	#define BANG_SIZE_SHIFT 0

	BANG_get_info.x      = bb->x + (bp->x << BANG_SIZE_SHIFT);
	BANG_get_info.y      = bb->y + (bp->y << BANG_SIZE_SHIFT);
	BANG_get_info.z      = bb->z + (bp->z << BANG_SIZE_SHIFT);
	BANG_get_info.dx     = bp->dx << 2;
	BANG_get_info.dy     = bp->dy << 2;
	BANG_get_info.dz     = bp->dz << 2;
	BANG_get_info.radius = bp->radius << BANG_SIZE_SHIFT;
	BANG_get_info.frame  = bp->counter;

	SLONG red   = bt->initial_r + bt->dr * bp->counter;
	SLONG green = bt->initial_g + bt->dg * bp->counter;
	SLONG blue  = bt->initial_b + bt->db * bp->counter;

	if (red   < 0) {red   = 0;}
	if (green < 0) {green = 0;}
	if (blue  < 0) {blue  = 0;}

	BANG_get_info.red   = red;
	BANG_get_info.green = green;
	BANG_get_info.blue  = blue;

	BANG_get_phwoar = bp->next;

	if (BANG_get_phwoar == NULL)
	{
		BANG_get_bang = bb->next;
	}

	BANG_get_info.y += bp->counter << 1;

	return &BANG_get_info;
}

