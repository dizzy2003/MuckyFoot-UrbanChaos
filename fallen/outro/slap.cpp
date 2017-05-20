//
// Creates a shadowed bitmap for a light map, given the silhoutte.
// Credit to Eddie Edwards for the outline rendering idea.
//

#include "always.h"
#include "slap.h"



//
// The bitmap we are rendering to.
//

UBYTE *SLAP_bitmap;
SLONG  SLAP_bitmap_size;



//
// Each line of the bitmap has a sorted linked list for all the lines that cross it.
//

#define SLAP_TYPE_START 0
#define SLAP_TYPE_END   1

typedef struct
{
	UBYTE type;
	UBYTE padding;
	UWORD next;
	SLONG pos;		// 8-bit fixed point.

} SLAP_Link;

#define SLAP_MAX_LINKS (SLAP_MAX_BITMAP_SIZE * 4 * 8)

SLAP_Link SLAP_link[SLAP_MAX_LINKS];
SLONG     SLAP_link_upto;


//
// 4 line per bitmap pixel. Each line has a linked list of SLAP_Links.
//

UWORD SLAP_line[SLAP_MAX_BITMAP_SIZE * 4];
SLONG SLAP_line_number;	// SLAP_bitmap_size * 4




//
// Adds a value to a pixel.
// 

inline void SLAP_add_pixel(SLONG px, SLONG py, SLONG value)
{
	ASSERT(WITHIN(px, 0, SLAP_bitmap_size - 1));
	ASSERT(WITHIN(py, 0, SLAP_bitmap_size - 1));

	SLONG now = SLAP_bitmap[py * SLAP_bitmap_size + px];

	now += value;

	if (now > 255)
	{
		now = 255;
	}

	SLAP_bitmap[py * SLAP_bitmap_size + px] = now;
}







void SLAP_init(
		UBYTE *bitmap,
		SLONG  bitmap_size)
{
	//
	// Proper size...
	//

	ASSERT(
		bitmap_size ==   4 ||
		bitmap_size ==   8 ||
		bitmap_size ==  16 ||
		bitmap_size ==  32 ||
		bitmap_size ==  64 ||
		bitmap_size == 128 ||
		bitmap_size == 256 ||
		bitmap_size == 512 ||
		bitmap_size == 256);

	ASSERT(bitmap_size <= SLAP_MAX_BITMAP_SIZE);

	SLAP_bitmap      = bitmap;
	SLAP_bitmap_size = bitmap_size;
	SLAP_link_upto   = 1;
	SLAP_line_number = bitmap_size * 4;

	#ifndef NDEBUG

	//
	// Make sure all the links have been cleared up after the last render.
	//

	SLONG i;

	for (i = 0; i < SLAP_line_number; i++)
	{
		ASSERT(SLAP_line[i] == NULL);
	}

	#endif
}



void SLAP_add_edge(
		SLONG x1, SLONG y1,
		SLONG x2, SLONG y2)
{
	SLONG x;
	SLONG y;

	SLONG dx;
	SLONG dy;
	
	SLONG type;

	UWORD  next;
	UWORD *prev;


	//
	// Clip to the top and bottom of the bitmap.
	// 

	if (y1 < 0 && y2 < 0)
	{
		return;
	}

	if ((y1 >= SLAP_bitmap_size << 8) && y2 >= (SLAP_bitmap_size << 8))
	{
		return;
	}

	//
	// What type of edge is this?
	//

	if (y1 < y2)
	{
		type = SLAP_TYPE_START;
	}
	else
	{
		type = SLAP_TYPE_END;

		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	//
	// The gradient down the edge.
	//

	dx = x2 - x1;
	dy = y2 - y1;

	if (dy == 0)
	{
		//
		// We can ignore horizontal edges.
		//

		return;
	}

	dx = ((dx << 6) << 7) / dy;

	//
	// Add a link to each line covered by the edge.
	//

	if (y1 & 0x3f)
	{
		x  = (x1 << 7) + ((0x40 - (y1 & 0x3f)) * dx >> 6);
		y  = y1 >> 6;
		y += 1;
	}
	else
	{
		x = x1 << 7;
		y = y1 >> 6;
	}

	y1 >>= 6;
	y2   = y2 + 0x3f >> 6;

	if (y < 0)
	{
		x += -y * dx;
		y  =  0;
	}

	if (y2 > SLAP_line_number)
	{
		y2 = SLAP_line_number;
	}

	while(y < y2)
	{
		ASSERT(WITHIN(SLAP_link_upto, 1, SLAP_MAX_LINKS - 1));

		SLAP_link[SLAP_link_upto].pos  = x >> 7;
		SLAP_link[SLAP_link_upto].type = type;
		
		//
		// Insert into the linked list for this line.
		//

		ASSERT(WITHIN(y, 0, SLAP_line_number - 1));

		prev = &SLAP_line[y];
		next =  SLAP_line[y];

		while(1)
		{
			ASSERT(WITHIN(next, 0, SLAP_link_upto - 1));

			if (next == NULL || SLAP_link[next].pos >= (x >> 7))
			{
				//
				// This is where to insert...
				//

				SLAP_link[SLAP_link_upto].next = next;
			   *prev                           = SLAP_link_upto;
				SLAP_link_upto                += 1;

				break;
			}

			prev = &SLAP_link[next].next;
			next =  SLAP_link[next].next;
		}

		x += dx;
		y += 1;
	}
}



void SLAP_render()
{
	SLONG x;
	SLONG y;

	SLONG on;

	SLONG add;

	SLAP_Link *sk1;
	SLAP_Link *sk2;

	for (y = 0; y < SLAP_line_number; y++)
	{
		if (SLAP_line[y] == NULL)
		{
			//
			// Nothing to render on this line.
			//

			continue;
		}

		//
		// The first link on this line.
		//

		ASSERT(WITHIN(SLAP_line[y], 1, SLAP_link_upto - 1));

		sk1 = &SLAP_link[SLAP_line[y]];

		//
		// All links should at least come in pairs.
		//

		ASSERT(WITHIN(sk1->next, 1, SLAP_link_upto - 1));

		sk2 = &SLAP_link[sk1->next];

		//
		// Render the span between sk1 and sk2.
		//

		on = 1;

		while(1)
		{
			ASSERT(sk1->pos <= sk2->pos);

			if ((sk1->pos >> 8) >= SLAP_bitmap_size)
			{
				//
				// Finished this line.
				//

				break;
			}

			if (!on || sk2->pos <= 0)
			{
				//
				// Easy peasy! This is some blank space.
				// 
			}
			else
			{

				if ((sk1->pos >> 8) == (sk2->pos >> 8))
				{
					//
					// Start and end in the same pixel.
					//

					add = sk2->pos - sk1->pos >> 2;

					SLAP_add_pixel(sk1->pos >> 8, y >> 2, add);
				}
				else
				{
					x = sk1->pos;

					if (x <= 0)
					{
						//
						// Zoom to the beginning of the line.
						//

						x = 0;
					}
					else
					{
						//
						// The first pixel might be fractional.
						//

						SLAP_add_pixel(x >> 8, y >> 2, 256 - (x & 0xff) >> 2);

						x &= ~0xff;
						x +=  256;
					}

					//
					// Don't zoom off the end of the bitmap.
					// 

					if (sk2->pos > (SLAP_bitmap_size << 8))
					{
						sk2->pos = SLAP_bitmap_size << 8;
					}

					ASSERT((x & 0xff) == 0);

					//
					// This'd be the place to optimise!
					//

					while((x >> 8) < (sk2->pos >> 8))
					{
						SLAP_add_pixel(x >> 8, y >> 2, 64);

						x += 256;
					}


					if (x >= (SLAP_bitmap_size << 8))
					{
						//
						// Finished the line.
						//

						break;
					}
					else
					{
						//
						// The final pixel of the line.
						//

						add = sk2->pos - x >> 2;

						if (add)
						{
							SLAP_add_pixel(x >> 8, y >> 2, add);
						}
					}
				}
			}

			switch(sk2->type)
			{
				case SLAP_TYPE_START:
					on += 1;
					break;

				case SLAP_TYPE_END:
					on -= 1;
					break;

				default:
					ASSERT(0);
					break;
			}

			if (sk2->next == NULL)
			{
				break;
			}

			ASSERT(WITHIN(sk2->next, 1, SLAP_link_upto - 1));

			sk1 =  sk2;
			sk2 = &SLAP_link[sk2->next];
		}

		//
		// Finished this line.
		//

		SLAP_line[y] = NULL;
	}

	//
	// Clear the links
	//

	SLAP_link_upto = 1;
}




