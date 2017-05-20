//
// Stairs.
//

#include "game.h"
#include "id.h"
#include "stair.h"



//
// The bounding box of the building.
//

UBYTE STAIR_x1;
UBYTE STAIR_z1;
UBYTE STAIR_x2;
UBYTE STAIR_z2;

//
// The height of the roof of the building, 1 + the height of
// the highest storey.
// 

UBYTE STAIR_roof_height;

//
// The maximum dimensions of the bounding box.
// This must be a multiple of 8.
//

#define STAIR_MAX_SIZE 32

//
// The array where we store the positions of the stairs.
//

//
// NB. Both flags can be set at once!
//

// in building.h
//#define STAIR_FLAG_UP	(1 << 0)	// The staircase goes up
//#define STAIR_FLAG_DOWN (1 << 1)	// The staircase goes down.

typedef struct
{
	UBYTE flag;
	UBYTE up;	// The storey you go up to   (if (flag & STAIR_FLAG_UP))
	UBYTE down;	// The storey you go down to (if (flag & STAIR_FLAG_DOWN))
	UBYTE x1;
	UBYTE z1;
	UBYTE x2;
	UBYTE z2;

} STAIR_Stair;

#define STAIR_MAX_STAIRS 32

STAIR_Stair STAIR_stair[STAIR_MAX_STAIRS];
SLONG       STAIR_stair_upto;

//
// The info for each storey.
//

#define STAIR_MAX_PER_STOREY 3

typedef struct
{
	SLONG handle;	// How the storey is identified.
	UBYTE opp_x1;	// A wall is it good to have a staircase opposite on.
	UBYTE opp_z1;
	UBYTE opp_x2;
	UBYTE opp_z2;
	UBYTE height;
	UBYTE stair[STAIR_MAX_PER_STOREY];	// Indices into the STAIR_stair array. 0 => NULL index.
	UBYTE square[STAIR_MAX_SIZE][STAIR_MAX_SIZE / 8];

} STAIR_Storey;

#define STAIR_MAX_STOREYS 32

STAIR_Storey STAIR_storey[STAIR_MAX_STOREYS];
SLONG        STAIR_storey_upto;

//
// Sets the given square on the given storey.
//

void STAIR_set_bit(SLONG storey, SLONG x, SLONG z)
{
	SLONG off_x;
	SLONG off_z;

	SLONG byte;
	SLONG bit;

	ASSERT(WITHIN(storey, 0, STAIR_storey_upto - 1));
	ASSERT(WITHIN(x, STAIR_x1, STAIR_x2 - 1));
	ASSERT(WITHIN(z, STAIR_z1, STAIR_z2 - 1));

	off_x = x - STAIR_x1;
	off_z = z - STAIR_z1;

	ASSERT(WITHIN(off_x, 0, STAIR_MAX_SIZE - 1));
	ASSERT(WITHIN(off_z, 0, STAIR_MAX_SIZE - 1));

	byte = off_x >> 3;
	bit  = off_x  & 7;

	STAIR_storey[storey].square[off_z][byte] |= (1 << bit);
}

UBYTE STAIR_get_bit(SLONG storey, SLONG x, SLONG z)
{
	SLONG off_x;
	SLONG off_z;		

	SLONG byte;
	SLONG bit;

	ASSERT(WITHIN(storey, 0, STAIR_storey_upto - 1));
	ASSERT(WITHIN(x, STAIR_x1, STAIR_x2 - 1));
	ASSERT(WITHIN(z, STAIR_z1, STAIR_z2 - 1));

	off_x = x - STAIR_x1;
	off_z = z - STAIR_z1;

	ASSERT(WITHIN(off_x, 0, STAIR_MAX_SIZE - 1));
	ASSERT(WITHIN(off_z, 0, STAIR_MAX_SIZE - 1));

	byte = off_x >> 3;
	bit  = off_x  & 7;

	return STAIR_storey[storey].square[off_z][byte] & (1 << bit);
}

UBYTE STAIR_get_bit_from_square(UBYTE square[STAIR_MAX_SIZE][STAIR_MAX_SIZE / 8], SLONG x, SLONG z)
{
	SLONG off_x;
	SLONG off_z;

	SLONG byte;
	SLONG bit;

	ASSERT(WITHIN(x, STAIR_x1, STAIR_x2 - 1));
	ASSERT(WITHIN(z, STAIR_z1, STAIR_z2 - 1));

	off_x = x - STAIR_x1;
	off_z = z - STAIR_z1;

	ASSERT(WITHIN(off_x, 0, STAIR_MAX_SIZE - 1));
	ASSERT(WITHIN(off_z, 0, STAIR_MAX_SIZE - 1));

	byte = off_x >> 3;
	bit  = off_x  & 7;

	return square[off_z][byte] & (1 << bit);
}



//
// For working out the inside squares on each floor of the building.
//

//
// For ENTER link types, the square field is the first x-line that
// the wall encloses a complete square. For a LEAVE link, square is
// the last x-line that the wall encloses a complete square.
//

#define STAIR_LINK_T_ENTER 1
#define STAIR_LINK_T_LEAVE 2

typedef struct
{
	UBYTE next;
	UBYTE square;
	UWORD pos;		// 8-bit fixed point.

} STAIR_Link;

#define STAIR_MAX_LINKS 128

STAIR_Link STAIR_link[STAIR_MAX_LINKS];
SLONG      STAIR_link_upto;

//
// One linked list per z-row of floor square.
// 0 is the NULL index
//

UBYTE STAIR_edge[STAIR_MAX_SIZE];

//
// How to access the EDGE list. It is relative to
// the bounding box, so it takes up less memory.
//

#ifndef NDEBUG
#define STAIR_EDGE(z) (STAIR_check_edge((z)), STAIR_edge[(z) - STAIR_z1])
#else
#define STAIR_EDGE(z) (STAIR_edge[(z) - STAIR_z1])
#endif

void STAIR_check_edge(SLONG z)
{
	SLONG sz;

	ASSERT(WITHIN(z, STAIR_z1, STAIR_z2 - 1));

	sz = z - STAIR_z1;

	ASSERT(WITHIN(sz, 0, STAIR_MAX_SIZE - 1));
}


//
// Random numbers for the STAIR module.
//

ULONG STAIR_rand_seed;

inline void STAIR_srand(ULONG seed)
{
	STAIR_rand_seed = seed;
}

inline ULONG STAIR_grand(void)
{
	return STAIR_rand_seed;
}

inline UWORD STAIR_rand(void)
{
	UWORD ans;

	STAIR_rand_seed *= 328573;
	STAIR_rand_seed += 123456789;

	ans = STAIR_rand_seed >> 7;

	return ans;
}



//
// Adds the stair to the given storey.
//

void STAIR_add_to_storey(SLONG storey, SLONG stair)
{
	SLONG i;

	ASSERT(WITHIN(storey, 0, STAIR_storey_upto - 1));
	ASSERT(WITHIN(stair,  1, STAIR_stair_upto  - 1));

	for (i = 0; i < STAIR_MAX_PER_STOREY; i++)
	{
		if (STAIR_storey[storey].stair[i] == NULL)
		{
			STAIR_storey[storey].stair[i] = stair;

			return;
		}
	}

	//
	// No more room!
	//

	ASSERT(0);
}


void STAIR_init()
{
	//
	// Clear the bounding box.
	//

	STAIR_x1 = 255;
	STAIR_z1 = 255;
	STAIR_x2 = 0;
	STAIR_z2 = 0;

	//
	// Clear the stair and storey array.
	//

	STAIR_stair_upto  = 1;	// 0 is the NULL stair...
	STAIR_storey_upto = 0;

	//
	// Reset the roof height.
	//

	STAIR_roof_height = 0;
}

void STAIR_set_bounding_box(UBYTE x1, UBYTE z1, UBYTE x2, UBYTE z2)
{
	ASSERT(WITHIN(x1, 0, 255));
	ASSERT(WITHIN(z1, 0, 255));
	ASSERT(WITHIN(x2, 0, 255));
	ASSERT(WITHIN(z2, 0, 255));

	STAIR_x1 = x1;
	STAIR_z1 = z1;
	STAIR_x2 = x2;
	STAIR_z2 = z2;

	ASSERT(WITHIN(STAIR_x2 - STAIR_x1, 0, STAIR_MAX_SIZE));
	ASSERT(WITHIN(STAIR_z2 - STAIR_z1, 0, STAIR_MAX_SIZE));
}

void STAIR_storey_new(SLONG handle, UBYTE height)
{
	SLONG i;

	STAIR_Storey *st;

	ASSERT(WITHIN(STAIR_storey_upto, 0, STAIR_MAX_STOREYS - 1));

	st = &STAIR_storey[STAIR_storey_upto];

	//
	// Work out the new roof height.
	//

	if (STAIR_roof_height < height + 1)
	{
		STAIR_roof_height = height + 1;
	}

	//
	// Initialise the new storey.
	//

	memset ((UBYTE*)st, 0, sizeof(STAIR_Storey));

	st->handle = handle;
	st->height = height;
	st->opp_x1 = 0;
	st->opp_z1 = 0;
	st->opp_x2 = 0;
	st->opp_z2 = 0;

	//
	// Initialise the link lists with which we calculate
	// the inside squares of each storey.
	//

	for (i = 0; i < STAIR_MAX_SIZE; i++)
	{
		STAIR_edge[i] = 0;
	}

	//
	// We don't use link zero because that is the NULL link index.
	//

	STAIR_link_upto = 1;

	//
	// Next storey...
	//

	STAIR_storey_upto += 1;
}

void STAIR_storey_wall(UBYTE x1, UBYTE z1, UBYTE x2, UBYTE z2, SLONG opposite)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG dxdz;
	SLONG pos;
	SLONG square;

	UBYTE  link_t;
	UBYTE  next;
	UBYTE *prev;

	//
	// Make sure this wall is inside the bounding rectangle of
	// the building.
	//

	ASSERT(WITHIN(x1, STAIR_x1, STAIR_x2));
	ASSERT(WITHIN(z1, STAIR_z1, STAIR_z2));

	if (opposite)
	{
		STAIR_Storey *st;

		ASSERT(WITHIN(STAIR_storey_upto - 1, 0, STAIR_MAX_STOREYS - 1));

		st = &STAIR_storey[STAIR_storey_upto - 1];
		
		st->opp_x1 = x1;
		st->opp_z1 = z1;
		st->opp_x2 = x2;
		st->opp_z2 = z2;

		if (x1 > x2) {SWAP(st->opp_x1, st->opp_x2);}
		if (z1 > z2) {SWAP(st->opp_z1, st->opp_z2);}
	}

	if (z1 == z2)
	{
		//
		// Ignore this line because it does not cross any
		// z-lines.
		//

		return;
	}

	if (z1 > z2)
	{
		link_t = STAIR_LINK_T_ENTER;

		//
		// Always go from top to bottom.
		//

		SWAP(x1, x2);
		SWAP(z1, z2);
	}
	else
	{
		link_t = STAIR_LINK_T_LEAVE;
	}

	dx = x2 - x1 << 16;
	dz = z2 - z1 << 16;

	dxdz = DIV64(dx, dz);

	x = x1 << 16;
	z = z1;

	while(z < z2)
	{
		switch(link_t)
		{
			case STAIR_LINK_T_ENTER:
				pos      = x >> 8;	// 8-bit fixed point.
				square   = MAX(x, x + dxdz);
				square  += 0xffff;
				square >>= 16;
				break;
			case STAIR_LINK_T_LEAVE:
				pos      = x >> 8;	// 8-bit fixed point.
				square   = MIN(x, x + dxdz);
				square >>= 16;
				break;

			default:
				ASSERT(0);
				break;
		}

		//
		// Create a new link.
		//

		ASSERT(WITHIN(STAIR_link_upto, 1, STAIR_MAX_LINKS - 1));

		STAIR_link[STAIR_link_upto].square = square;
		STAIR_link[STAIR_link_upto].pos    = pos;
		STAIR_link[STAIR_link_upto].next   = 0;

		//
		// Insert it in the correct place in the linked list.
		//

		prev = &STAIR_EDGE(z);
		next =  STAIR_EDGE(z);

		while(1)
		{
			if (next == NULL)
			{
				//
				// We have reached end of the linked list.
				//

				break;
			}

			ASSERT(WITHIN(next, 1, STAIR_link_upto - 1));

			if (STAIR_link[next].pos >= STAIR_link[STAIR_link_upto].pos)
			{
				if (STAIR_link[next].pos == STAIR_LINK_T_ENTER)
				{
					//
					// Make the ENTER link appear first in the linked list.
					//
				}
				else
				{
					//
					// This is the right place to insert it.
					//

					break;
				}
			}

			//
			// Go onto the next link.
			//

			prev = &STAIR_link[next].next;
			next =  STAIR_link[next].next;
		}

		ASSERT(*prev == next);

		//
		// Insert it.
		//

		STAIR_link[STAIR_link_upto].next = next;
	   *prev                             = STAIR_link_upto;
		STAIR_link_upto					+= 1;

		//
		// Do the next line.
		//

		x += dxdz;
		z += 1;
	}
}

SLONG STAIR_storey_finish()
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG x2;

	UBYTE next;
	UBYTE next1;
	UBYTE next2;

	STAIR_Storey *st;

	ASSERT(WITHIN(STAIR_storey_upto - 1, 0, STAIR_MAX_STOREYS - 1));

	st = &STAIR_storey[STAIR_storey_upto - 1];

	//
	// Go through the linked lists and mark each square as
	// being inside or outside.
	//

	for (z = STAIR_z1; z < STAIR_z2; z++)
	{
		next = STAIR_EDGE(z);

		if (next == NULL)
		{
			//
			// This whole z-row is outside.
			//

			continue;
		}

		//
		// These should come in pairs.
		//

		next1 = next;
		next2 = STAIR_link[next].next;

		//
		// The pairs should be start and end.
		//

		if (next1 == NULL ||
			next2 == NULL)
		{
			//
			// An invalid storey.
			//

			return FALSE;
		}

		ASSERT(WITHIN(next1, 1, STAIR_link_upto - 1));
		ASSERT(WITHIN(next2, 1, STAIR_link_upto - 1));

		x1 = STAIR_link[next1].square;
		x2 = STAIR_link[next2].square;

		//
		// We only mark squares that are completely inside.
		//

		for (x = x1; x < x2; x++)
		{
			//
			// Mark these squares as inside.
			//

			STAIR_set_bit(STAIR_storey_upto - 1, x, z);
		}
	}

	return TRUE;
}

//
// Returns whether there is an outside door opening
// onto the given square on the given storey.
// 

SLONG STAIR_is_door(SLONG storey, SLONG x, SLONG z)
{
	return 0;
}


void STAIR_calculate(UWORD seed)
{
	SLONG i;
	SLONG j;
	SLONG k;
	SLONG l;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG x1;
	SLONG x2;
	SLONG z1;
	SLONG z2;
	SLONG score;

	SLONG ox;
	SLONG oz;
	SLONG outside;

	SLONG best_x1;
	SLONG best_z1;
	SLONG best_x2;
	SLONG best_z2;
	SLONG best_score;

	STAIR_Storey *st;
	STAIR_Storey *st_n;

	STAIR_Stair *ss;
	STAIR_Stair *ss_new;

	SLONG dox;
	SLONG doz;

	SLONG height;
	SLONG height_n;

	UBYTE square[STAIR_MAX_SIZE][STAIR_MAX_SIZE / 8];
	UBYTE go_up;
	UBYTE go_down;

	//
	// Randomise the order we do the storeys in.
	//

	UBYTE storey_order1[STAIR_MAX_STOREYS];

	STAIR_srand(seed);

	for (i = 0; i < STAIR_MAX_STOREYS; i++)
	{
		storey_order1[i] = i;
	}

	for (i = 0; i < STAIR_MAX_STOREYS; i++)
	{
		j = STAIR_rand() % STAIR_MAX_STOREYS;
		k = STAIR_rand() % STAIR_MAX_STOREYS;

		SWAP(storey_order1[j], storey_order1[k]);
	}

	//
	// Workout where the stairs should be for each storey.
	//

	for (i = 0; i < STAIR_storey_upto; i++)
	{
		st = &STAIR_storey[i];

		//
		// The height of this storey.
		//

		height = st->height;

		for (j = 0; j < STAIR_storey_upto; j++)
		{
			//
			// Look for neighbouring storeys.
			//

			st_n = &STAIR_storey[j];

			//
			// The height of this storey.
			//

			height_n = st_n->height;

			if (abs(height - height_n) == 1)
			{
				//
				// This is a neighbouring storey. AND in this storeys
				// squares.
				//

				memcpy(square, st->square, sizeof(square));

				for (z = 0; z < STAIR_MAX_SIZE;     z++)
				for (x = 0; x < STAIR_MAX_SIZE / 8; x++)
				{
					square[z][x] &= st_n->square[z][x];
				}

				//
				// Is this storey higher or lower?
				//

				go_up   = 0;
				go_down = 0;

				     if (height_n == height + 1) {go_up   = 1;}
				else if (height_n == height - 1) {go_down = 1;}
				else {ASSERT(0);}

				//
				// Is there a staircase on this storey we use?
				//

				for (k = 0; k < STAIR_MAX_PER_STOREY; k++)
				{
					if (st->stair[k] != NULL)
					{
						ASSERT(WITHIN(st->stair[k], 1, STAIR_stair_upto - 1));

						ss = &STAIR_stair[st->stair[k]];

						//
						// There is already a staircase on this storey. It is
						// in an okay place?
						//

						if (STAIR_get_bit_from_square(square, ss->x1, ss->z1) &&
							STAIR_get_bit_from_square(square, ss->x2, ss->z2))
						{
							//
							// We can just use this staircase.
							//

							if (go_up)
							{
								if (ss->flag & STAIR_FLAG_UP)
								{
									//
									// There is already a staircase up to this floor!
									//

									goto placed_stairs;
								}
								else
								{
									//
									// Make this staircase go upto the next floor and
									// create a down staircase on the next floor.
									//

									ss->flag |= STAIR_FLAG_UP;
									ss->up    = j;

									//
									// New stair...
									//

									ss_new = &STAIR_stair[STAIR_stair_upto++];

									ss_new->flag = STAIR_FLAG_DOWN;
									ss_new->down = i;
									ss_new->x1   = ss->x1;
									ss_new->z1   = ss->z1;
									ss_new->x2   = ss->x2;
									ss_new->z2   = ss->z2;

									STAIR_add_to_storey(j, STAIR_stair_upto - 1);

									goto placed_stairs;
								}
							}

							if (go_down)
							{
								if (ss->flag & STAIR_FLAG_DOWN)
								{
									//
									// There is already a staircase down to this floor!
									//

									goto placed_stairs;
								}
								else
								{
									//
									// Make this staircase go down to the next floor and
									// create a down staircase on the next floor.
									//

									ss->flag |= STAIR_FLAG_DOWN;
									ss->down  = j;

									//
									// New stair...
									//

									ss_new = &STAIR_stair[STAIR_stair_upto++];

									ss_new->flag = STAIR_FLAG_UP;
									ss_new->up   = i;
									ss_new->x1   = ss->x1;
									ss_new->z1   = ss->z1;
									ss_new->x2   = ss->x2;
									ss_new->z2   = ss->z2;

									STAIR_add_to_storey(j, STAIR_stair_upto - 1);

									goto placed_stairs;
								}
							}
						}
					}
				}

				//
				// Does the neighbour have a suitable staircase already?
				//

				for (k = 0; k < STAIR_MAX_PER_STOREY; k++)
				{
					if (st_n->stair[k] != NULL)
					{
						ASSERT(WITHIN(st_n->stair[k], 1, STAIR_stair_upto - 1));

						ss = &STAIR_stair[st_n->stair[k]];

						//
						// There is a staircase on this storey.  Is is in an
						// okay place?
						//

						if (STAIR_get_bit_from_square(square, ss->x1, ss->z1) &&
							STAIR_get_bit_from_square(square, ss->x2, ss->z2))
						{
							//
							// We could just use this staircase.
							//

							if (go_up)
							{
								if (ss->flag & STAIR_FLAG_DOWN)
								{
									//
									// There is staircase going down from the floor above, but
									// we didn't find a staircase going up!
									// 

									ASSERT(0);
								}

								//
								// Use this staircase.
								//

								ss->flag |= STAIR_FLAG_DOWN;
								ss->down  = i;

								//
								// Create a new staircase on this floor.
								//

								ss_new = &STAIR_stair[STAIR_stair_upto++];

								ss_new->flag = STAIR_FLAG_UP;
								ss_new->up   = j;
								ss_new->x1   = ss->x1;
								ss_new->z1   = ss->z1;
								ss_new->x2   = ss->x2;
								ss_new->z2   = ss->z2;

								STAIR_add_to_storey(i, STAIR_stair_upto - 1);

								goto placed_stairs;
							}

							if (go_down)
							{
								if (ss->flag & STAIR_FLAG_UP)
								{
									//
									// There is a staircase going up to this floor, but
									// we didn't find a staircase going down to it!
									//

									ASSERT(0);
								}

								//
								// Use this staircase.
								//

								ss->flag |= STAIR_FLAG_UP;
								ss->up    = i;

								//
								// Create a new staircase on this floor.
								//

								ss_new = &STAIR_stair[STAIR_stair_upto++];

								ss_new->flag = STAIR_FLAG_DOWN;
								ss_new->down = j;
								ss_new->x1   = ss->x1;
								ss_new->z1   = ss->z1;
								ss_new->x2   = ss->x2;
								ss_new->z2   = ss->z2;

								STAIR_add_to_storey(i, STAIR_stair_upto - 1);

								goto placed_stairs;
							}
						}
					}
				}

				//
				// Look for the best place for a new staircase in the bounding
				// box of the building.
				//

				best_score = -INFINITY;

				for (x = STAIR_x1; x < STAIR_x2; x++)
				for (z = STAIR_z1; z < STAIR_z2; z++)
				{
					x1 = x;
					z1 = z;

					for (k = 0; k < 4; k++)
					{
						x2 = x1;
						z2 = z1;

						switch(k)
						{
							case 0: x2 += 1; break;
							case 1: x2 -= 1; break;
							case 2: z2 += 1; break;
							case 3: z2 -= 1; break;
							default:
								ASSERT(0);
								break;
						}

						if (!WITHIN(x2, STAIR_x1, STAIR_x2 - 1) ||
							!WITHIN(z2, STAIR_z1, STAIR_z2 - 1))
						{
							//
							// Outside the building!
							//

							continue;
						}

						//
						// Make sure these two squares exist on both storeys.
						//

						if (STAIR_get_bit_from_square(square, x1, z1) &&
							STAIR_get_bit_from_square(square, x2, z2))
						{
							//
							// Score this choice of staircase.
							//

							score   = STAIR_rand() & 0xffff;
							outside = 0;

							dx = x2 - x1;
							dz = z2 - z1;

							//
							// If both squares of the staircase are against an outside wall
							// then that is good. If only one is than that is bad, but not
							// disasterous. If the staircase is in the way of an outside door
							// than that is disasterous.
							//

							for (l = 0; l < 2; l++)
							{
								switch(l)
								{	
									case 0: ox = x1 + (+dz); oz = z1 + (-dx); break;
									case 1: ox = x2 + (+dz); oz = z2 + (-dx); break;
									default:
										ASSERT(0);
										break;
								}

								if (!WITHIN(ox, STAIR_x1, STAIR_x2 - 1) ||
									!WITHIN(oz, STAIR_z1, STAIR_z2 - 1) ||
									!STAIR_get_bit(i, ox, oz))
								{
									//
									// (ox,oz) is outside, so this square of the staircase lies
									// outside of the storey.
									//

									outside += 1;

									if (STAIR_is_door(i, ox, oz))
									{
										//
										// Arghh! A staircase here would block a door from the
										// outside.
										//

										score = -INFINITY;
									}
								}
							}

							//
							// Favour stairs on outside walls.
							//

							switch(outside)
							{
								case 0: score  = 0x0;	  break;
								case 1: score -= 0x10000; break;
								case 2: score += 0x10000; break;
								default:
									ASSERT(0);
									break;
							}

							//
							// We like stairs in corners.
							//

							if (x1 == STAIR_x1) {score += 0x5000;}
							if (x1 == STAIR_x2) {score += 0x5000;}
							if (x2 == STAIR_x1) {score += 0x5000;}
							if (x2 == STAIR_x2) {score += 0x5000;}

							if (z1 == STAIR_z1) {score += 0x5000;}
							if (z1 == STAIR_z2) {score += 0x5000;}
							if (z2 == STAIR_z1) {score += 0x5000;}
							if (z2 == STAIR_z2) {score += 0x5000;}

							//
							// We like stairs opposite the 'opposite' wall.
							//

							if (z1 == z2 && st->opp_z1 == st->opp_z2)
							{
								doz = st->opp_z1 - z1;

								if (abs(doz) > 2)
								{
									if (WITHIN(x1, st->opp_x1, st->opp_x2)) {score += 0xbabe;}
									if (WITHIN(x2, st->opp_x1, st->opp_x2)) {score += 0xbabe;}
									
								}
							}

							if (x1 == x2 && st->opp_x1 == st->opp_x2)
							{
								dox = st->opp_x1 - x1;

								if (abs(dox) > 2)
								{
									if (WITHIN(z1, st->opp_z1, st->opp_z2)) {score += 0xbabe;}
									if (WITHIN(z2, st->opp_z1, st->opp_z2)) {score += 0xbabe;}
									
								}
							}

							if (score > best_score)
							{
								best_x1    = x1;
								best_z1    = z1;
								best_x2    = x2;
								best_z2    = z2;
								best_score = score;
							}
						}
					}
				}

				if (best_score >= 0)
				{
					ASSERT(WITHIN(STAIR_stair_upto, 1, STAIR_MAX_STAIRS - 2));

					//
					// Put a staircase here and on the other floor.
					//

					ss = &STAIR_stair[STAIR_stair_upto++];

					ss->flag = 0;
					ss->x1   = best_x1;
					ss->z1   = best_z1;
					ss->x2   = best_x2;
					ss->z2   = best_z2;

					if (go_up)   {ss->flag |= STAIR_FLAG_UP;   ss->up   = j;}
					if (go_down) {ss->flag |= STAIR_FLAG_DOWN; ss->down = j;}

					STAIR_add_to_storey(i, STAIR_stair_upto - 1);

					//
					// The other floor.
					//

					ss = &STAIR_stair[STAIR_stair_upto++];

					ss->flag = 0;
					ss->x1   = best_x1;
					ss->z1   = best_z1;
					ss->x2   = best_x2;
					ss->z2   = best_z2;

					//
					// These are swapped around for the other floor.
					//

					if (go_up)   {ss->flag |= STAIR_FLAG_DOWN; ss->down = i;}
					if (go_down) {ss->flag |= STAIR_FLAG_UP;   ss->up   = j;}

					STAIR_add_to_storey(j, STAIR_stair_upto - 1);
				}
				else
				{
					//
					// We need some stairs, but there is nowhere to put
					// them!
					//

					TRACE("Oh no! Can't place stairs.\n");
				}

              placed_stairs:;
			}
			else
			{
				//
				// Storeys i and j are more than one floor apart.
				//
			}
		}
	}
}


ID_Stair STAIR_id_stair[STAIR_MAX_PER_STOREY];

SLONG STAIR_get(SLONG handle, ID_Stair **stair, SLONG *num_stairs)
{
	SLONG i;
	SLONG storey;

	STAIR_Storey *st;
	STAIR_Stair  *ss;
	ID_Stair     *is;

	for (i = 0; i < STAIR_storey_upto; i++)
	{
		st = &STAIR_storey[i];

		if (st->handle == handle)
		{
			//
			// Found the storey.
			//

			goto found_storey_handle;
		}
		
	}

	//
	// Could not find a storey with the given handle.
	//

	return FALSE;

  found_storey_handle:

   *num_stairs = 0;	

	for (i = 0; i < STAIR_MAX_PER_STOREY; i++)
	{
		if (st->stair[i] != NULL)
		{
			ASSERT(WITHIN(*num_stairs,  0, STAIR_MAX_PER_STOREY - 1));
			ASSERT(WITHIN(st->stair[i], 1, STAIR_stair_upto - 1));

			ss = &STAIR_stair   [st->stair[i]];
			is = &STAIR_id_stair[*num_stairs];

			//
			// Add this stair to the array of ID_stairs...
			//

			is->x1 = ss->x1;
			is->z1 = ss->z1;
			is->x2 = ss->x2;
			is->z2 = ss->z2;

			if (ss->flag == STAIR_FLAG_UP)
			{
				ASSERT(WITHIN(ss->up, 0, STAIR_storey_upto - 1));

				is->type      = ID_STAIR_TYPE_BOTTOM;
				is->handle_up = STAIR_storey[ss->up].handle;
			}
			else
			if (ss->flag == STAIR_FLAG_DOWN)
			{
				ASSERT(WITHIN(ss->down, 0, STAIR_storey_upto - 1));

				is->type        = ID_STAIR_TYPE_TOP;
				is->handle_down = STAIR_storey[ss->down].handle;
			}
			else
			{
				ASSERT(ss->flag == (STAIR_FLAG_UP | STAIR_FLAG_DOWN));

				ASSERT(WITHIN(ss->up,   0, STAIR_storey_upto - 1));
				ASSERT(WITHIN(ss->down, 0, STAIR_storey_upto - 1));

				is->type = ID_STAIR_TYPE_MIDDLE;

				is->handle_up   = STAIR_storey[ss->up  ].handle;
				is->handle_down = STAIR_storey[ss->down].handle;
			}

			//
			// Do not define the id...
			//

			is->id = 0;

		   *num_stairs += 1;
		}
	}

	//
	// Return the array...
	//

   *stair = STAIR_id_stair;

    return TRUE;
}









