//
// Functions for helping with the outline of shapes on grids.
//

#include	"Editor.hpp"
#include <MFStdLib.h>


//
// An outline.
//

#define OUTLINE_LINK_TYPE_START 0
#define OUTLINE_LINK_TYPE_END   1

typedef struct outline_link
{
	SLONG x;
	SLONG type;
	
	struct outline_link *next;

} OUTLINE_Link;

typedef struct outline_outline
{
	SLONG max_z;
	OUTLINE_Link *link[];

} OUTLINE_Outline;


OUTLINE_Outline *OUTLINE_create(SLONG num_z_squares)
{
	SLONG num_bytes;

	OUTLINE_Outline *oo;
	
	//
	// How much memory does this outline take?
	//
	
	num_bytes = sizeof(OUTLINE_Outline) + sizeof(OUTLINE_Link *) * num_z_squares;

	oo = (OUTLINE_Outline *) malloc(num_bytes);

	//
	// Initialise it.
	//

	memset(oo, 0, num_bytes);

	oo->max_z = num_z_squares;

	return oo;
}


//
// Inserts the link into the correct place in the outline.
//

void OUTLINE_insert_link(OUTLINE_Outline *oo, OUTLINE_Link *ol, SLONG link_z)
{
	SLONG here;

	OUTLINE_Link  *next;
	OUTLINE_Link **prev;

	ASSERT(WITHIN(link_z, 0, oo->max_z - 1));

	prev = &oo->link[link_z];
	next =  oo->link[link_z];

	while(1)
	{
		here = FALSE;

		if (next == NULL)
		{
			//
			// We must insert the link here.
			//

			here = TRUE;
		}
		else
		if (next->x > ol->x)
		{
			here = TRUE;
		}
		else
		if (next->x == ol->x)
		{
			//
			// For links at the same x, insert END links before START links.
			//

			if (ol->x == OUTLINE_LINK_TYPE_END)
			{
				here = TRUE;
			}
		}

		if (here)
		{
			//
			// This is where we insert the link.
			//

		    ol->next = next;
		   *prev     = ol;

			return;
		}

		prev = &next->next;
		next =  next->next;
	}
}



void OUTLINE_add_line(
		OUTLINE_Outline *oo,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG z;

	SLONG type;

	ASSERT(x1 == x2 || z1 == z2);

	ASSERT(WITHIN(z1, 0, oo->max_z - 1));
	ASSERT(WITHIN(z2, 0, oo->max_z - 1));

	if (z1 == z2)
	{
		//
		// We can ignore this line.
		//

		return;
	}

	if (z1 > z2)
	{	
		SWAP(z1,z2);

		type = OUTLINE_LINK_TYPE_START;
	}
	else
	{
		type = OUTLINE_LINK_TYPE_END;
	}

	//
	// Insert it in all the zlines.
	//

	for (z = z1; z < z2; z++)
	{
		//
		// Allocate a new link.
		//

		OUTLINE_Link *ol = (OUTLINE_Link *) malloc(sizeof(OUTLINE_Link));

		ol->type = type;
		ol->x    = x1;
		ol->next = NULL;

		//
		// Insert it.
		//

		OUTLINE_insert_link(oo, ol, z);
	}
}


void OUTLINE_free(OUTLINE_Outline *oo)
{
	SLONG z;

	OUTLINE_Link *ol;
	OUTLINE_Link *next;

	//
	// Free all the links first.
	//

	for (z = 0; z < oo->max_z; z++)
	{
		for (ol = oo->link[z]; ol; ol = next)
		{
			next = ol->next;

			free(ol);
		}
	}

	//
	// Now free the actual outline.
	//

	free(oo);
}


//
// Returns TRUE if the two linked lists overlap.
//

SLONG OUTLINE_overlap(
		OUTLINE_Link *ol1,
		OUTLINE_Link *ol2)
{
	SLONG on1 = FALSE;
	SLONG on2 = FALSE;

	while(1)
	{
		if (ol1 == NULL ||
			ol2 == NULL)
		{
			if (ol1 == NULL) {ASSERT(!on1);}
			if (ol2 == NULL) {ASSERT(!on2);}

			return FALSE;
		}

		if ((ol1->x < ol2->x) || (ol1->x == ol2->x && ol1->type == OUTLINE_LINK_TYPE_END))
		{
			//
			// Move on the first line.
			//


			if((!on1 && ol1->type == OUTLINE_LINK_TYPE_START) ||( on1 && ol1->type == OUTLINE_LINK_TYPE_END))
			{

			}
			else
			return(0);
			ASSERT(
				(!on1 && ol1->type == OUTLINE_LINK_TYPE_START) ||
				( on1 && ol1->type == OUTLINE_LINK_TYPE_END));

			ol1  = ol1->next;
			on1 ^= TRUE;
		}
		else
		if (ol2->x < ol1->x || (ol1->x == ol2->x && ol2->type == OUTLINE_LINK_TYPE_END))
		{
			//
			// Move on the second line.
			//

				if((!on2 && ol2->type == OUTLINE_LINK_TYPE_START) ||( on2 && ol2->type == OUTLINE_LINK_TYPE_END))
				{
				}
				else
				{
					return(0);
				}
			ASSERT(
				(!on2 && ol2->type == OUTLINE_LINK_TYPE_START) ||
				( on2 && ol2->type == OUTLINE_LINK_TYPE_END));

			ol2  = ol2->next;
			on2 ^= TRUE;
		}
		else
		if (ol1->x == ol2->x)
		{
			ASSERT(ol1->type == OUTLINE_LINK_TYPE_START);
			ASSERT(ol2->type == OUTLINE_LINK_TYPE_START);

			//
			// Neither line is ending here (OUTLINE_LINK_TYPE_END) so they must be
			// both starting here- so they must overlap.
			//

			return TRUE;
		}

		if (on1 && on2)
		{
			return TRUE;
		}
	}
}


SLONG OUTLINE_overlap(
		OUTLINE_Outline *oo1,
		OUTLINE_Outline *oo2)
{
	SLONG z;
	SLONG minz = MIN(oo1->max_z, oo2->max_z);

	for (z = 0; z < minz; z++)
	{
		if (OUTLINE_overlap(
				oo1->link[z],
				oo2->link[z]))
		{
			return TRUE;
		}
	}

	return FALSE;
}

//
// Returns TRUE if the given square is in the outline.
//

SLONG OUTLINE_inside(
		OUTLINE_Outline *oo,
		SLONG x,
		SLONG z)
{
	OUTLINE_Link *ol;

	if (!WITHIN(z, 0, oo->max_z - 1))
	{
		//
		// Not in z-range.
		//

		return FALSE;
	}

	ol = oo->link[z];

	while(1)
	{
		if (ol == NULL || ol->next == NULL)
		{
			return FALSE;
		}

		if (ol->type       == OUTLINE_LINK_TYPE_START &&
			ol->next->type == OUTLINE_LINK_TYPE_END)
		{
			if (WITHIN(x, ol->x, ol->next->x - 1))
			{
				return TRUE;
			}
		}

		ol = ol->next;
	}
}


SLONG OUTLINE_intersects(
		OUTLINE_Outline *oo,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG i;
	SLONG x;
	SLONG z;
	SLONG dx;
	SLONG dz;
	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;
	SLONG len;

	ASSERT(x1 == x2 || z1 == z2);

	dx = x2 - x1;
	dz = z2 - z1;

	len = MAX(abs(dx),abs(dz));

	x = x1 << 8;
	z = z1 << 8;

	dx = SIGN(dx);
	dz = SIGN(dz);

	x += dx << 7;
	z += dz << 7;

	for (i = 0; i < len; i++)
	{
		mx1 = x - (dz << 7) >> 8;
		mz1 = z + (dx << 7) >> 8;

		mx2 = x + (dz << 7) >> 8;
		mz2 = z - (dx << 7) >> 8;

		if (OUTLINE_inside(oo, mx1, mz1) ||
			OUTLINE_inside(oo, mx2, mz1))
		{
			return TRUE;
		}

		x += dx << 8;
		z += dz << 8;
	}

	return FALSE;
}

