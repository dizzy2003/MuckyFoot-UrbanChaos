//
// Faster far-facets...
//

#include "game.h"
#include "ddlib.h"
#include "pap.h"
#include "poly.h"
#include "polypoint.h"
#include "polypage.h"
#include "night.h"
#include "supermap.h"
#include "memory.h"
#include "matrix.h"
#include <math.h>


//
// How many lo-res mapsquares per FARFACET_square.
//

#define FARFACET_RATIO (4)			// 4x4 lo-res mapsqures in a FARFACET_square
#define FARFACET_SIZE  (PAP_SIZE_LO / FARFACET_RATIO)



// Set to 1 to use DrawIndPrim, rather than the MM stuff.
#define FARFACET_USE_INDEXED_LISTS 1				   

//
// Where we store the lverts.
//

D3DLVERTEX *FARFACET_lvert_buffer;	// Unaligned buffer FARFACET_lvert_max elements + 31bytes long.
D3DLVERTEX *FARFACET_lvert;			// Aligned to 32 bytes
SLONG       FARFACET_lvert_max;
SLONG       FARFACET_lvert_upto;


//
// Index storage.
//

UWORD *FARFACET_index;
SLONG  FARFACET_index_max;		// The number of elements in the FARFACET_index[] array.
SLONG  FARFACET_index_upto;


//
// 2D map.
//

typedef struct
{
	UWORD lvert;
	UWORD lvertcount;
	UWORD index;
	UWORD indexcount;

} FARFACET_Square;

FARFACET_Square FARFACET_square[FARFACET_SIZE][FARFACET_SIZE];


//
// During initialisation we use this array to store the rectangular
// outline of each facet.
//

typedef struct
{
	UBYTE x1;
	SBYTE y1;
	UBYTE z1;

	UBYTE x2;
	SBYTE y2;
	UBYTE z2;

} FARFACET_Outline;

#define FARFACET_MAX_OUTLINES (8192 / ((32 / FARFACET_RATIO) * (32 / FARFACET_RATIO)) + 256)

FARFACET_Outline *FARFACET_outline;
SLONG             FARFACET_outline_upto;


//
// The renderstate of the farfacets... no texture, no fogging, backface culled.
//

RenderState FARFACET_renderstate;
RenderState FARFACET_default_renderstate;


//
// A 32-byte aligned matrix.
//

UBYTE      FARFACET_matrix_buffer[sizeof(D3DMATRIX) + 32];
D3DMATRIX *FARFACET_matrix;



//
// Looks for a vertex at (x,y,z) for the given square. If found
// it returns that vertex index, otherwise it creates one.
//

UWORD FARFACET_find_vertex(FARFACET_Square *fs, UBYTE map_x, SBYTE map_y, UBYTE map_z)
{
	SLONG i;

	float x = float(map_x << 8);
	float y = float(map_y << 6);
	float z = float(map_z << 8);

	D3DLVERTEX *lv;

	for (i = 0; i < fs->lvertcount; i++)
	{
		ASSERT(WITHIN(fs->lvert + i, 0, FARFACET_lvert_upto - 1));

		lv = &FARFACET_lvert[fs->lvert + i];

		if (lv->x == x &&
			lv->y == y &&
			lv->z == z)
		{
			return i;
		}
	}

	//
	// We need to create another point.
	//

	if (FARFACET_lvert_upto >= FARFACET_lvert_max)
	{
		SLONG old_offset;
		SLONG new_offset;

		//
		// How much is FARFACET_lvert offset into the FARFACET_lvert_buffer?
		//

		old_offset = ((UBYTE *) FARFACET_lvert) - ((UBYTE *) FARFACET_lvert_buffer);

		//
		// Double the length of the array.
		//

		FARFACET_lvert_max   *= 2;
		FARFACET_lvert_buffer = (D3DLVERTEX *) realloc(FARFACET_lvert_buffer, sizeof(D3DLVERTEX) * FARFACET_lvert_max + 31);
		ASSERT ( FARFACET_lvert_buffer != NULL );
		FARFACET_lvert        = (D3DLVERTEX *) ((SLONG(FARFACET_lvert_buffer) + 31) & ~0x1f);

		ASSERT ( FARFACET_lvert_upto < FARFACET_lvert_max );

		//
		// What is the new offset into the FARFACET_lvert_buffer?
		//

		new_offset = ((UBYTE *) FARFACET_lvert) - ((UBYTE *) FARFACET_lvert_buffer);

		//
		// If the offsets are different then we have to move the data around.
		//

		if (new_offset != old_offset)
		{
			memmove(((UBYTE *) FARFACET_lvert_buffer) + new_offset, ((UBYTE *) FARFACET_lvert_buffer) + old_offset, sizeof(D3DLVERTEX) * FARFACET_lvert_upto);
		}
	}

	ASSERT(fs->lvert + fs->lvertcount == FARFACET_lvert_upto);

	lv = &FARFACET_lvert[FARFACET_lvert_upto++];

	lv->x          = x;
	lv->y          = y;
	lv->z          = z;
	lv->tu         = 0.0F;
	lv->tv         = 0.0F;
#ifdef DEBUG
	//lv->color      = 0x00ff0000;
	lv->color      = 0x0000ff00;
#else
	lv->color      = 0x00000000;
#endif
	lv->specular   = 0x00000000;
	lv->dwReserved = 0;

	return fs->lvertcount++;
}


//
// Adds an index to the given square.
//

void FARFACET_add_index(FARFACET_Square *fs, UWORD index)
{
	if (FARFACET_index_upto >= FARFACET_index_max)
	{
		FARFACET_index_max *= 2;
		FARFACET_index      = (UWORD *) realloc(FARFACET_index, sizeof(UWORD) * FARFACET_index_max);
	}

	ASSERT(FARFACET_index_upto == fs->index + fs->indexcount);

	FARFACET_index[FARFACET_index_upto] = index;

	FARFACET_index_upto += 1;
	fs->indexcount      += 1;
}








//
// Builds the drawprim call for the given square.
//

void FARFACET_create_square(SLONG square_x, SLONG square_z)
{
	SLONG i;
	SLONG j;
	SLONG f_list;
	SLONG facet;
	SLONG build;
	SLONG exit;
	SLONG lvert_memory;
	SLONG old_outline_upto;

	SLONG v1;
	SLONG v2;

	SLONG dx1;
	SLONG dz1;

	SLONG dx2;
	SLONG dz2;

	FARFACET_Outline *fo;
	FARFACET_Outline *fo1;
	FARFACET_Outline *fo2;
	DFacet           *df;
	FARFACET_Square  *fs;

	ASSERT(WITHIN(square_x, 0, FARFACET_SIZE - 1));
	ASSERT(WITHIN(square_z, 0, FARFACET_SIZE - 1));
	
	fs = &FARFACET_square[square_x][square_z];

	//
	// Initialise the outline array.
	//

	memset(FARFACET_outline, 0, sizeof(FARFACET_outline));

	FARFACET_outline_upto = 0;

	//
	// Mark all facets as not added.
	//

	for (i = 1; i < next_dfacet; i++)
	{
		df = &dfacets[i];

		df->Counter[0] = FALSE;
	}

	//
	// The bounding box of lo-res mapsquares covered by this
	// farfacet square.
	//

	SLONG lo_min_x;
	SLONG lo_min_z;

	SLONG lo_max_x;
	SLONG lo_max_z;

	SLONG lo_x;
	SLONG lo_z;

	lo_min_x = (square_x + 0) * FARFACET_RATIO;
	lo_min_z = (square_z + 0) * FARFACET_RATIO;

	lo_max_x = (square_x + 1) * FARFACET_RATIO;
	lo_max_z = (square_z + 1) * FARFACET_RATIO;

	//
	// The box we clip facet outlines against.
	//

	SLONG hi_min_x = lo_min_x * 4;
	SLONG hi_min_z = lo_min_z * 4;

	SLONG hi_max_x = lo_max_x * 4;
	SLONG hi_max_z = lo_max_z * 4;

	//
	// Add this squares facets to the outline array.
	//

	for (lo_x = lo_min_x; lo_x < lo_max_x; lo_x++)
	for (lo_z = lo_min_z; lo_z < lo_max_z; lo_z++)
	{
		f_list = PAP_2LO(lo_x,lo_z).ColVectHead;

		if (f_list)
		{
			exit = FALSE;

			while(!exit)
			{
				facet = facet_links[f_list];

				if (facet < 0)
				{
					//
					// The last facet in the list for each square is negative.
					//

					facet = -facet;
					exit  =  TRUE;
				}

				ASSERT(WITHIN(facet, 1, next_dfacet - 1));

				df = &dfacets[facet];

				//
				// Have we done this facet before?
				//

				if (df->Counter[0])
				{
					//
					// Don't do this facet twice.
					//

					goto abort_facet;
				}

				//
				// Mark this facet as done.
				//

				df->Counter[0] = TRUE;

				//
				// Only draw certain types of facet.
				//

				if (df->FacetType != STOREY_TYPE_NORMAL &&
					df->FacetType != STOREY_TYPE_DOOR)
				{
					goto abort_facet;
				}
				
				if (dbuildings[df->Building].Type == BUILDING_TYPE_CRATE_IN)
				{
					goto abort_facet;
				}

				if (df->FacetFlags & FACET_FLAG_INSIDE)
				{
					goto abort_facet;
				}

				//
				// Add this facet's outline to the array.
				//

				ASSERT(WITHIN(FARFACET_outline_upto, 0, FARFACET_MAX_OUTLINES - 1));

				fo = &FARFACET_outline[FARFACET_outline_upto++];

				fo->x1 = df->x[0];
				fo->y1 = df->Y[0] >> 6;
				fo->z1 = df->z[0];

				fo->x2 = df->x[1];
				fo->y2 = df->Y[0] + df->Height * df->BlockHeight * 4 >> 6;
				fo->z2 = df->z[1];

				SATURATE(fo->x1, hi_min_x, hi_max_x);
				SATURATE(fo->z1, hi_min_z, hi_max_z);

				SATURATE(fo->x2, hi_min_x, hi_max_x);
				SATURATE(fo->z2, hi_min_z, hi_max_z);

			  abort_facet:;
			  
				f_list++;	
			}
		}
	}

	//
	// Go through the outline array and merge rectangles that connect to eachother.
	//

	while(1)
	{
		old_outline_upto = FARFACET_outline_upto;

		for (i = 0; i < FARFACET_outline_upto; i++)
		{
			fo1 = &FARFACET_outline[i];

			dx1 = SIGN(fo1->x2 - fo1->x1);
			dz1 = SIGN(fo1->z2 - fo1->z1);

			for (j = i + 1; j < FARFACET_outline_upto; j++)
			{
				fo2 = &FARFACET_outline[j];

				dx2 = SIGN(fo2->x2 - fo2->x1);
				dz2 = SIGN(fo2->z2 - fo2->z1);

				//
				// Do facet1 and facet2 connect?
				//

				if (dx1 == dx2 && dz1 == dz2)
				{
					//
					// They are pointing in the same direction- that's a good start!
					//

					if (fo1->x1 == fo2->x1 &&
						fo1->z1 == fo2->z1 && 
						fo1->x2 == fo2->x2 &&
						fo1->z2 == fo2->z2)
					{
						//
						// The facets are on top of oneanother.
						//

						if (fo1->y2 == fo2->y1)
						{
							//
							// Facet2 is directly on top of our facet.
							//

							fo1->y2 = fo2->y2;

							//
							// Replace this facet with the one at the end of the array
							// and shorten the array.
							//

							FARFACET_outline[j] = FARFACET_outline[--FARFACET_outline_upto];

							//
							// Do the facet again.
							//
							
							j--;
						}
						else
						if (fo1->y1 == fo2->y2)
						{
							//
							// Facet2 is directly below our facet.
							//

							fo1->y1 = fo2->y1;

							//
							// Replace this facet with the one at the end of the array
							// and shorten the array.
							//

							FARFACET_outline[j] = FARFACET_outline[--FARFACET_outline_upto];

							//
							// Do the facet again.
							//
							
							j--;
						}
					}
					else
					if (fo1->y1 == fo2->y1 &&
						fo1->y2 == fo2->y2)
					{
						//
						// The facets are at the same height.
						//

						if (fo1->x1 == fo2->x2 &&
							fo1->z1 == fo2->z2)
						{
							//
							// Facet2 is directly to the left of our facet.
							//

							fo1->x1 = fo2->x1;
							fo1->z1 = fo2->z1;

							//
							// Replace this facet with the one at the end of the array
							// and shorten the array.
							//

							FARFACET_outline[j] = FARFACET_outline[--FARFACET_outline_upto];

							//
							// Do the facet again.
							//
							
							j--;
						}
						else
						if (fo1->x2 == fo2->x1 &&
							fo1->z2 == fo2->z1)
						{
							//
							// Facet2 is directly to the right of our facet.
							//

							fo1->x2 = fo2->x2;
							fo1->z2 = fo2->z2;

							//
							// Replace this facet with the one at the end of the array
							// and shorten the array.
							//

							FARFACET_outline[j] = FARFACET_outline[--FARFACET_outline_upto];

							//
							// Do the facet again.
							//
							
							j--;
						}
					}
				}
			}
		}

		if (FARFACET_outline_upto == old_outline_upto)
		{
			//
			// Looped through without making any changes.
			//

			break;
		}
	}

	//
	// Now convert the outlines to strips and build the
	// DrawPrimitive data.
	//

	fs->lvert      = FARFACET_lvert_upto;
	fs->lvertcount = 0;
	fs->index      = FARFACET_index_upto;
	fs->indexcount = 0;
	
	#define FARFACET_MAX_STRIP_LENGTH 256

	UWORD strip[FARFACET_MAX_STRIP_LENGTH];
	SLONG strip_upto;

	while(1)
	{
		if (FARFACET_outline_upto == 0)
		{
			break;
		}

		//
		// Create a simple strip from the last outline
		// and shorten the array so we don't look for
		// that outline again.
		//

		strip[0]               = FARFACET_outline_upto - 1;
		strip_upto             = 1;
		FARFACET_outline_upto -= 1;

		//
		// Try and lengthen the strip by adding onto the beginning.
		//

		fo1 = &FARFACET_outline[strip[0]];

	  add_onto_beginning:;

		for (i = 0; i < FARFACET_outline_upto; i++)
		{
			fo2 = &FARFACET_outline[i];

			if (fo2->x2 == fo1->x1 &&
				fo2->y2 == fo1->y1 &&
				fo2->z2 == fo1->z1)
			{
				//
				// Swap this facet with the one at the end of the
				// outline array and shorten the array so we dont
				// consider it again.
				//

				FARFACET_outline_upto -= 1;

				SWAP(fo2->x1, FARFACET_outline[FARFACET_outline_upto].x1);
				SWAP(fo2->y1, FARFACET_outline[FARFACET_outline_upto].y1);
				SWAP(fo2->z1, FARFACET_outline[FARFACET_outline_upto].z1);

				SWAP(fo2->x2, FARFACET_outline[FARFACET_outline_upto].x2);
				SWAP(fo2->y2, FARFACET_outline[FARFACET_outline_upto].y2);
				SWAP(fo2->z2, FARFACET_outline[FARFACET_outline_upto].z2);

				//
				// We can add this outline onto the beginning
				// of the strip.
				//

				ASSERT(WITHIN(strip_upto, 1, FARFACET_MAX_STRIP_LENGTH - 1));

				memmove(strip + 1, strip + 0, sizeof(UWORD) * strip_upto);
				strip_upto += 1;

				strip[0] = FARFACET_outline_upto;

				//
				// Start again.
				//

				goto add_onto_beginning;
			}
		}

		//
		// Now lengthen the strip by adding onto the end.
		//

		ASSERT(WITHIN(strip_upto, 1, FARFACET_MAX_STRIP_LENGTH - 1));

		fo1 = &FARFACET_outline[strip[strip_upto - 1]];

	   add_onto_end:;

		for (i = 0; i < FARFACET_outline_upto; i++)
		{
			fo2 = &FARFACET_outline[i];

			if (fo2->x1 == fo1->x2 &&
				fo2->y1 == fo1->y2 &&
				fo2->z1 == fo1->z2)
			{
				//
				// Swap this facet with the one at the end of the
				// outline array and shorten the array so we dont
				// consider it again.
				//

				FARFACET_outline_upto -= 1;

				SWAP(fo2->x1, FARFACET_outline[FARFACET_outline_upto].x1);
				SWAP(fo2->y1, FARFACET_outline[FARFACET_outline_upto].y1);
				SWAP(fo2->z1, FARFACET_outline[FARFACET_outline_upto].z1);

				SWAP(fo2->x2, FARFACET_outline[FARFACET_outline_upto].x2);
				SWAP(fo2->y2, FARFACET_outline[FARFACET_outline_upto].y2);
				SWAP(fo2->z2, FARFACET_outline[FARFACET_outline_upto].z2);

				//
				// We can add this outline onto the end of the strip.
				//

				ASSERT(WITHIN(strip_upto, 1, FARFACET_MAX_STRIP_LENGTH - 1));

				strip[strip_upto++] = FARFACET_outline_upto;

				//
				// Start again.
				//

				goto add_onto_end;
			}
		}

		//
		// Add this strip to the square. The initial points of the
		// first outline now.
		//

		fo = &FARFACET_outline[strip[0]];

		v1 = FARFACET_find_vertex(fs, fo->x1,fo->y1,fo->z1);
		v2 = FARFACET_find_vertex(fs, fo->x1,fo->y2,fo->z1);

#if FARFACET_USE_INDEXED_LISTS
		// Do nothing.
		SLONG v1prev = v1;
		SLONG v2prev = v2;
#else
		FARFACET_add_index(fs, v1);
		FARFACET_add_index(fs, v2);
#endif

		for (i = 0; i < strip_upto; i++)
		{
			fo = &FARFACET_outline[strip[i]];

			v1 = FARFACET_find_vertex(fs, fo->x2,fo->y1,fo->z2);
			v2 = FARFACET_find_vertex(fs, fo->x2,fo->y2,fo->z2);

#if FARFACET_USE_INDEXED_LISTS
			FARFACET_add_index(fs, v1prev);
			FARFACET_add_index(fs, v2prev);
			FARFACET_add_index(fs, v1);
			FARFACET_add_index(fs, v1);
			FARFACET_add_index(fs, v2prev);
			FARFACET_add_index(fs, v2);
			v1prev = v1;
			v2prev = v2;
#else
			FARFACET_add_index(fs, v1);
			FARFACET_add_index(fs, v2);
#endif
		}

#if !FARFACET_USE_INDEXED_LISTS
		FARFACET_add_index(fs, -1);	// End of a strip...
#endif
	}
}




void FARFACET_init()
{
	//
	// Initialise memory.
	//

	FARFACET_lvert_max    = 1024;
	FARFACET_lvert_upto   = 0;
	FARFACET_lvert_buffer = (D3DLVERTEX *) malloc(sizeof(D3DLVERTEX) * FARFACET_lvert_max + 31);
	FARFACET_lvert        = (D3DLVERTEX *) ((SLONG(FARFACET_lvert_buffer) + 31) & ~0x1f);

#if FARFACET_USE_INDEXED_LISTS
	FARFACET_index_max  = FARFACET_lvert_max * 5 / 4;
#else
	FARFACET_index_max  = FARFACET_lvert_max * 6 / 4;
#endif
	FARFACET_index_upto = 0;
	FARFACET_index      = (UWORD *) malloc(sizeof(UWORD) * FARFACET_index_max);

	FARFACET_outline      = (FARFACET_Outline *) malloc(sizeof(FARFACET_Outline) * FARFACET_MAX_OUTLINES);
	FARFACET_outline_upto = 0;

	memset(FARFACET_square,  0, sizeof(FARFACET_square   )                        );
	memset(FARFACET_lvert,   0, sizeof(D3DLVERTEX        ) * FARFACET_lvert_max   );
	memset(FARFACET_index,   0, sizeof(UWORD             ) * FARFACET_index_max   );
	memset(FARFACET_outline, 0, sizeof(FARFACET_Outline  ) * FARFACET_MAX_OUTLINES);

	FARFACET_matrix = (D3DMATRIX *) ((SLONG(FARFACET_matrix_buffer) + 31) & ~0x1f);

	//
	// Calculate the FARFACET squares.
	//

	SLONG x;
	SLONG z;

	for (x = 0; x < FARFACET_SIZE; x++)
	for (z = 0; z < FARFACET_SIZE; z++)
	{
		FARFACET_create_square(x,z);
	}

	//
	// We don't need the outline array any more.
	//

	free(FARFACET_outline);

	//
	// Setup our renderstate.
	//


	FARFACET_renderstate.SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);

#if 0
// These are all completely ignored by the DC SetChanged thing - don't bother.
	FARFACET_renderstate.SetRenderState(D3DRENDERSTATE_ZENABLE,   FALSE);
	FARFACET_renderstate.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,   FALSE);
#endif

	FARFACET_renderstate.SetTexture(NULL);

}


SLONG FARFACET_num_squares_drawn;

void FARFACET_draw(
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float draw_dist,
		float lens)
{
	#ifndef TARGET_DC

	if (!Keys[KB_R])
	{
		return;
	}

	#endif

	//
	// The five point of the view pyramid.
	//

	float width;
	float height;
	float depth;
	float aspect;
	float matrix[9];

	struct
	{
		float x;
		float z;

	} cone[5];

	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	FARFACET_num_squares_drawn = 0;

	//
	// The dimensions of the view pyramid.
	//

	width  = draw_dist;
	height = draw_dist;
	depth  = draw_dist;

	width *= POLY_screen_width;
	width /= POLY_screen_height;

	width  /= lens;
	height /= lens;

	//
	// Finds the points of the cone in world space
	//

	cone[3].x = cone[4].x = x / 256.0f;
	cone[3].z = cone[4].z = z / 256.0f;

	cone[3].x += depth * matrix[6];
	cone[3].z += depth * matrix[8];

	//
	// cone[0] is the top left corner...
	//

	cone[0].x = cone[3].x + height * matrix[3];
	cone[0].z = cone[3].z + height * matrix[5];

	cone[0].x = cone[0].x - width *  matrix[0];
	cone[0].z = cone[0].z - width *  matrix[2];

	//
	// cone[1] is the top right corner...
	//

	cone[1].x = cone[3].x + height * matrix[3];
	cone[1].z = cone[3].z + height * matrix[5];

	cone[1].x = cone[1].x + width *  matrix[0];
	cone[1].z = cone[1].z + width *  matrix[2];

	//
	// cone[2] is the bottom right corner...
	//

	cone[2].x = cone[3].x - height * matrix[3];
	cone[2].z = cone[3].z - height * matrix[5];

	cone[2].x = cone[2].x + width *  matrix[0];
	cone[2].z = cone[2].z + width *  matrix[2];

	//
	// cone[3] is the bottom left corner...
	//

	cone[3].x = cone[3].x - height * matrix[3];
	cone[3].z = cone[3].z - height * matrix[5];

	cone[3].x = cone[3].x - width *  matrix[0];
	cone[3].z = cone[3].z - width *  matrix[2];

	//
	// Find the bounding box of squares we should draw.
	//

	SLONG square_min_x = +INFINITY;
	SLONG square_min_z = +INFINITY;

	SLONG square_max_x = -INFINITY;
	SLONG square_max_z = -INFINITY;

	SLONG i;

	for (i = 0; i < 5; i++)
	{
		if (ftol(cone[i].x) < square_min_x) {square_min_x = ftol(cone[i].x);}
		if (ftol(cone[i].z) < square_min_z) {square_min_z = ftol(cone[i].z);}

		if (ftol(cone[i].x) > square_max_x) {square_max_x = ftol(cone[i].x);}
		if (ftol(cone[i].z) > square_max_z) {square_max_z = ftol(cone[i].z);}
	}

	//
	// This is in high-res map squares, now convert to our
	// size of square.
	//

	square_min_x /= 4 * FARFACET_RATIO;
	square_min_z /= 4 * FARFACET_RATIO;

	square_max_x /= 4 * FARFACET_RATIO;
	square_max_z /= 4 * FARFACET_RATIO;

	SATURATE(square_min_x, 0, FARFACET_SIZE - 1);
	SATURATE(square_min_z, 0, FARFACET_SIZE - 1);

	SATURATE(square_max_x, 0, FARFACET_SIZE - 1);
	SATURATE(square_max_z, 0, FARFACET_SIZE - 1);

	#if 0

	//
	// To draw every far-facet...
	//

	square_min_x = 0;
	square_min_z = 0;

	square_max_x = FARFACET_SIZE - 1;
	square_max_z = FARFACET_SIZE - 1;

	#endif

	//
	// Setup renderstate.
	//

	FARFACET_renderstate.SetChanged();


	//
	// Initialise Tom's doberries...
	//


	// Adjust the projection to put the RHW values a little further back than normal.
	// This is sort of like a Z-bias. It's there to avoid Z-fighting with the real scene polygons.
	// Note that this uniform scale _does_ put the RHWs back because we are using the multimatrix stuff,
	// which has several cheats. Fortunately.
#define MY_PROJ_MATRIX_SCALE 1.01f
	D3DMATRIX matMyProj = g_matProjection;
	matMyProj._11 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._12 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._13 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._14 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._21 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._22 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._23 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._24 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._31 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._32 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._33 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._34 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._41 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._42 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._43 *= MY_PROJ_MATRIX_SCALE;
	matMyProj._44 *= MY_PROJ_MATRIX_SCALE;


#if FARFACET_USE_INDEXED_LISTS
	(the_display.lp_D3D_Device)->SetTransform ( D3DTRANSFORMSTATE_PROJECTION, &matMyProj );
#else
	GenerateMMMatrixFromStandardD3DOnes(
		FARFACET_matrix,
	    &matMyProj,
	    NULL,
	    &g_viewData);
#endif


#ifdef DEBUG
	the_display.SetRenderState ( D3DRENDERSTATE_FOGENABLE, FALSE );
#endif

	//
	// Draw all the squares.
	//

	SLONG square_x;
	SLONG square_z;

	float mid_x;
	float mid_y;
	float mid_z;

	float dx;
	float dy;
	float dz;

	float dist;
	float dprod;

	FARFACET_Square *fs;

	for (square_x = square_min_x; square_x <= square_max_x; square_x++)
	for (square_z = square_min_z; square_z <= square_max_z; square_z++)
	{
		fs = &FARFACET_square[square_x][square_z];


#ifdef DEBUG
		// Colourise the different squares
		DWORD dwColour = (DWORD)fs;
		dwColour += 0x32928157;
		dwColour *= 0x90781;
		dwColour ^= (dwColour << 3) ^ (dwColour << 5) ^(dwColour << 9) ^(dwColour << 18) ^(dwColour << 23) ^(dwColour << 28);
		dwColour *= 0x9574;
		dwColour += 0x90846347;
		dwColour &= 0x7f7f7f7f;
		for ( int i = 0; i < fs->lvertcount; i++ )
		{
			FARFACET_lvert[fs->lvert + i].color		= dwColour;
			FARFACET_lvert[fs->lvert + i].specular	= dwColour;
		}
#endif

		if (fs->indexcount)
		{
			//
			// Don't draw squares to close to the camera- or behind it!
			//

			mid_y = 0.0F;

			mid_x = (float(square_x) + 0.5F) * (4.0F * FARFACET_RATIO * 256.0F);
			mid_z = (float(square_z) + 0.5F) * (4.0F * FARFACET_RATIO * 256.0F);

			dx = mid_x - x;
			dy = mid_y - y;
			dz = mid_z - z;

			dprod = dx*matrix[6] + dy*matrix[7] + dz*matrix[8];

#if 0
			if (dprod < 12.0F * 256.0F)
#else
			// React to the fog distance change.
extern SLONG CurDrawDistance;
			if (dprod * 2.5f < (float)CurDrawDistance)
#endif
			{
				continue;
			}

			FARFACET_num_squares_drawn += 1;

#if FARFACET_USE_INDEXED_LISTS
			the_display.DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				D3DFVF_LVERTEX,
				FARFACET_lvert + fs->lvert,
				fs->lvertcount,
				FARFACET_index + fs->index,
				fs->indexcount,
				D3DDP_DONOTUPDATEEXTENTS);
#else
			D3DMULTIMATRIX d3dmm =
			{
				FARFACET_lvert + fs->lvert,
				FARFACET_matrix,
				NULL,
				NULL
			};

			//TRACE ( "S2" );
			DrawIndPrimMM(
				the_display.lp_D3D_Device,
				D3DFVF_LVERTEX,
			   &d3dmm,
				fs->lvertcount,
				FARFACET_index + fs->index,
				fs->indexcount);
			//TRACE ( "F2" );
#endif

		}
	}

	//
	// Revert to default renderstate.
	//

#ifdef DEBUG
	the_display.SetRenderState ( D3DRENDERSTATE_FOGENABLE, TRUE );
#endif

	FARFACET_default_renderstate.SetChanged();

#if FARFACET_USE_INDEXED_LISTS
	// Restore the projection matrix.
	(the_display.lp_D3D_Device)->SetTransform ( D3DTRANSFORMSTATE_PROJECTION, &g_matProjection );
#endif



}




void FARFACET_fini()
{
	//
	// Free memory.
	//

	free(FARFACET_lvert_buffer);
	free(FARFACET_index       );
}





