//
// Converts facets to draw indexed primitive calls...
//

#include "game.h"
#include "ddlib.h"
#include "poly.h"
#include "polypoint.h"
#include "polypage.h"
#include "night.h"
#include "supermap.h"
#include "memory.h"
#include "interfac.h"
#include "supercrinkle.h"
#include "matrix.h"




#ifndef TARGET_DC
// Only do this on the PC!
#define SUPERFACET_PERFORMANCE
#define	POLY_set_local_rotation_none() {}
#else
extern void POLY_set_local_rotation_none(void);

#endif

#ifdef SUPERFACET_PERFORMANCE
FILE *SUPERFACET_handle;
SLONG SUPERFACET_total_freed;
SLONG SUPERFACET_total_converted;
SLONG SUPERFACET_total_already_cached;
SLONG SUPERFACET_total_drawn;
SLONG SUPERFACET_num_gameturns;
#endif



// A hack to test the speed of theoretical crinkles.
#undef TESSELATE_DEGREE
//#define TESSELATE_DEGREE 1


#ifdef DEBUG
// Some interesting colouring.
#define SHOW_ME_SUPERFACET_DEBUGGING_PLEASE_BOB defined
#endif

//
// The verts....
//

SLONG SUPERFACET_max_lverts;

#define SUPERFACET_MAX_LVERTS (SUPERFACET_max_lverts)

UBYTE      *SUPERFACET_lvert_buffer;
D3DLVERTEX *SUPERFACET_lvert;
SLONG       SUPERFACET_lvert_upto;

//
// The indices....
//

SLONG SUPERFACET_max_indices;

#define SUPERFACET_MAX_INDICES (SUPERFACET_max_indices)

UWORD *SUPERFACET_index;
SLONG  SUPERFACET_index_upto;


//
// The lvert buffer has an unused range. The beginning
// of the range is where we allocate new data.  When
// we free data the end of the range is moved on forwards.
//

SLONG SUPERFACET_free_range_start;
SLONG SUPERFACET_free_range_end;



#define SUPERFACET_CALL_FLAG_USED     (1 << 0)
#define SUPERFACET_CALL_FLAG_2PASS    (1 << 1)
#define SUPERFACET_CALL_FLAG_CRINKLED (1 << 2)

#ifdef SHOW_ME_SUPERFACET_DEBUGGING_PLEASE_BOB
bool m_bShowDebuggingInfo = FALSE;
#endif



typedef struct
{
	UBYTE flag;		// TRUE => this call is in use.
	UBYTE dir;		// The direction of the facet...
	UWORD quads;	// The number of quads this call draws.
	UWORD lvert;
	UWORD lvertcount;
	UWORD index;
	UWORD indexcount;
	UWORD index2;						// For the 2-pass textures...
	UWORD crinkle;						// The PolyPage or SUPERCRINKLE for this call...

	LPDIRECT3DTEXTURE2 texture;
	LPDIRECT3DTEXTURE2 texture_2pass;	// For the 2-pass textures...

} SUPERFACET_Call;

#define SUPERFACET_MAX_CALLS 2048

SUPERFACET_Call SUPERFACET_call[SUPERFACET_MAX_CALLS];
SLONG           SUPERFACET_call_upto;


//
// Incides into the calllist array.
//

SLONG SUPERFACET_max_facets;

#define SUPERFACET_MAX_FACETS SUPERFACET_max_facets

typedef struct
{
	UWORD call;	// Index into the SUPERFACET_call[] array
	UWORD num;

} SUPERFACET_Facet;

SUPERFACET_Facet *SUPERFACET_facet;


//
// A circular queue of facets. It remembers the order the
// facets were cached so they can be uncached in the same
// order.
//

#define SUPERFACET_QUEUE_SIZE 512	// Power of 2 please...

UWORD SUPERFACET_queue[SUPERFACET_QUEUE_SIZE];
SLONG SUPERFACET_queue_start;
SLONG SUPERFACET_queue_end;




//
// A 32-byte aligned matrix.
//

UBYTE      SUPERFACET_matrix_buffer[sizeof(D3DMATRIX) + 32];
D3DMATRIX *SUPERFACET_matrix;




//
// The direction matrices for each direction of facet.
//

float SUPERFACET_direction_matrix[4][9];




//
// From facet.cpp
//

extern ULONG facet_rand (void);
extern void  set_facet_seed(SLONG seed);
extern SLONG texture_quad(POLY_Point *quad[4],SLONG texture_style,SLONG pos,SLONG count,SLONG flipx=0);






//
// Frees up the memory for the facet at the end of a queue.
//

void SUPERFACET_free_end_of_queue(void)
{
	SLONG i;
	SLONG facet;
	
	SUPERFACET_Facet *sf;
	SUPERFACET_Call  *sc;

	#ifdef SUPERFACET_PERFORMANCE
	SUPERFACET_total_freed += 1;
	#endif

	ASSERT(SUPERFACET_queue_end < SUPERFACET_queue_start);

	facet = SUPERFACET_queue[SUPERFACET_queue_end & (SUPERFACET_QUEUE_SIZE - 1)];

	ASSERT(WITHIN(facet, 0, SUPERFACET_MAX_FACETS - 1));
	
	sf = &SUPERFACET_facet[facet];

	//
	// Free up all the call structures.
	//

	for (i = 0; i < sf->num; i++)
	{
		ASSERT(WITHIN(sf->call + i, 0, SUPERFACET_MAX_CALLS - 1));

		sc = &SUPERFACET_call[sf->call + i];
		
		ASSERT(sc->flag & SUPERFACET_CALL_FLAG_USED);

		sc->flag = 0;
	}
	
	//
	// Upto the end of the last call structure is now free.
	//

	SUPERFACET_free_range_end = sc->lvert + sc->lvertcount;

	ASSERT(WITHIN(SUPERFACET_free_range_end, 0, SUPERFACET_MAX_LVERTS));

	//
	// If the next call structure is used and is at the beginning of the
	// array then all the memory to the end of the lvert array is free.
	//

	ASSERT(WITHIN(sf->call + sf->num, 0, SUPERFACET_MAX_CALLS - 1));

	sc = &SUPERFACET_call[sf->call + sf->num];

	if (sc->flag & SUPERFACET_CALL_FLAG_USED)
	{
		if (sc->lvert == 0)
		{
			SUPERFACET_free_range_end = SUPERFACET_MAX_LVERTS;
		}
	}


	//
	// Pop the facet off the end of the queue.
	//

	SUPERFACET_queue_end += 1;

	#ifdef SUPERFACET_PERFORMANCE
	fprintf(SUPERFACET_handle, "Game turn %5d  freed up facet %4d\n", GAME_TURN, facet);
	fflush(SUPERFACET_handle);
	#endif									  

	//
	// Mark as unused.
	//

	sf->call = 0;
	sf->num  = 0;
}








//
// Returns the actual page used, and converts the texture
// coordinates in the quad.
//

LPDIRECT3DTEXTURE2 SUPERFACET_convert_texture(SLONG page, POLY_Point *quad[4])
{
	SLONG i;

	ASSERT(WITHIN(page, 0, 511));	// Special subset of pages just for facets...

	PolyPage *pp = &POLY_Page[page];

	#ifdef TEX_EMBED

	//
	// Convert texture coordinates.
	//

	for (i = 0; i < 4; i++)
	{
		quad[i]->u = quad[i]->u * pp->m_UScale + pp->m_UOffset;
		quad[i]->v = quad[i]->v * pp->m_VScale + pp->m_VOffset;
	}

	#endif

	return pp->RS.GetTexture();
}





//
// The start of the NIGHT_Colour[] array for this facet.
// So we can work out the index into the NIGHT_Colour[] array
// for each point.
//

NIGHT_Colour *SUPERFACET_colour_base;

//
// Stolen from facet.cpp. Fills in POLY_buffer[] with the points
// of the facet.
//

extern SWORD FacetRows [100];
extern float FacetDiffY[128];

void SUPERFACET_make_facet_points(
		float sx,
		float sy,
		float sz,
		float dx,
		float dz,
		float block_height, 
		SLONG height,
		NIGHT_Colour *col,
		SLONG foundation,
		SLONG count,
		SLONG hug)
{	
	SLONG hf = 0;

	POLY_buffer_upto = 0;	// or else FacetDiffY is accessed wrongly

	ASSERT(WITHIN(block_height, 32.0F, 256.0F));

	while (height >= 0)
	{
		float x = sx;
		float y = sy;
		float z = sz;

		FacetRows[hf] = POLY_buffer_upto;

		for (SLONG c0 = count; c0 > 0; c0--)
		{
			float ty;

			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			POLY_Point *pp = &POLY_buffer[POLY_buffer_upto++];

			if (hug)
			{
				ty  = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);
				ty += y;
			}
			else
			if (foundation != 2)
			{
				ty = y;
			}
			else
			{
				ty = float(PAP_2HI(SLONG(x) >> 8, SLONG(z) >> 8).Alt << 3);

				FacetDiffY[POLY_buffer_upto - 1] = ( ( y - ty ) * ( 1.0f / 256.0f ) ) + 1.0f;
			}

			pp->x = x;
			pp->y = ty;
			pp->z = z;

			//
			// The index into the dfcache array for this facet.
			//

			pp->user = col - SUPERFACET_colour_base;

			NIGHT_get_d3d_colour(*col, &pp->colour, &pp->specular);

			x   += dx;
			z   += dz;
			col += 1;
		}
		sy         += block_height;
		height     -= 4;
		hf         += 1;
		foundation -= 1;
	}

	FacetRows[hf] = POLY_buffer_upto;
}


//
// Generates the points for for the facet.
//

int m_iFacetDirection;

void SUPERFACET_create_points(SLONG facet)
{
	SLONG i;
	SLONG j;
	SLONG dx;
	SLONG dz;
	SLONG hf;
	SLONG height;
	SLONG count;
	SLONG foundation;

	float sx;
	float sy;
	float sz;
	float block_height;

	LPDIRECT3DTEXTURE2 texture;
	DFacet            *df;
	SUPERFACET_Facet  *sf;
	SUPERFACET_Call   *sc;
	POLY_Point        *quad[4];
	NIGHT_Colour      *col;

	ASSERT(WITHIN(facet, 0, next_dfacet           - 1));
	ASSERT(WITHIN(facet, 0, SUPERFACET_MAX_FACETS - 1));

	df = &dfacets         [facet];
	sf = &SUPERFACET_facet[facet];

	//
	// Set the seed for the random facet texture system.
	//

	set_facet_seed(df->x[0] * df->z[0] + df->Y[0]);

	//
	// How long is the wall? No diagonal walls allowed.
	//

	ASSERT(WITHIN(df->Dfcache, 1, NIGHT_MAX_DFCACHES - 1));

	SUPERFACET_colour_base = col = NIGHT_dfcache[df->Dfcache].colour;

	dx = df->x[1] - df->x[0] << 8;
	dz = df->z[1] - df->z[0] << 8;

	sx = float(df->x[0] << 8);
	sy = float(df->Y[0]     );
	sz = float(df->z[0] << 8);

	ASSERT(!(dx && dz));

	if (dx) {count = abs(dx) >> 8;}
	else    {count = abs(dz) >> 8;}

#ifdef TESSELATE_DEGREE
	if ( dx == 0 )
	{
		if ( dz > 0 )
		{
			m_iFacetDirection = 3;
		}
		else
		{
			m_iFacetDirection = 2;
		}
	}
	else if ( dx > 0 )
	{
		m_iFacetDirection = 0;
	}
	else
	{
		m_iFacetDirection = 1;
	}
#endif

	count++;	// Count is the number of points, not the number of texture squares...

	if (df->FHeight)
	{
		foundation = 2;
	}
	else
	{	
		foundation = 0;
	}

	block_height = float(df->BlockHeight << 4);
	height       = df->Height;

	SUPERFACET_make_facet_points(
		sx,sy,sz,
		SIGN(dx) * 256.0F,
		SIGN(dz) * 256.0F,
		block_height,
		height,
		col,
		foundation,
		count,
		df->FacetFlags & FACET_FLAG_HUG_FLOOR);
}






void SUPERFACET_fill_facet_points(
		SLONG count,
		ULONG base_row,
		SLONG foundation,
		SLONG style_index,
		float block_height,
		SUPERFACET_Call *sc)
{
	SLONG i;
	SLONG j;
	SLONG page;
	float vheight = float(block_height) * ( 1.0f / 256.0f );

	POLY_Point        *quad[4];
	LPDIRECT3DTEXTURE2 texture;
	D3DLVERTEX        *lv;

	SLONG	row1 = FacetRows[base_row];
	SLONG	row2 = FacetRows[base_row+1];

	ASSERT(row2 - row1 == count);

	for (i = 0; i < row2 - row1 - 1; i++)
	{
		//
		// The four points of this quad.
		//

		quad[0] = &POLY_buffer[row2 + i + 1];
		quad[1] = &POLY_buffer[row2 + i    ];
		quad[2] = &POLY_buffer[row1 + i + 1];
		quad[3] = &POLY_buffer[row1 + i    ];

		page = texture_quad(quad, dstyles[style_index], i, count);

		//
		// Scale for block height.
		//

		// MASSIVE FUDGE.
		if ( quad[0]->v >= 0.9999f )
		{
			quad[0]->v = 0.0f;
		}
		if ( quad[1]->v >= 0.9999f )
		{
			quad[1]->v = 0.0f;
		}


		quad[2]->v = vheight;
		quad[3]->v = vheight;

		if (foundation == 2)
		{
			quad[3]->v = FacetDiffY[i    ];
			quad[2]->v = FacetDiffY[i + 1];

			//
			// OH MY GOD! FACET FOUNDATION TEXTURES WRAP!!!
			//

			// As a panic measure which might sort-of work, clamp the coords to 1.
			if ( quad[3]->v > 1.0f )
			{
				quad[3]->v = 1.0f;
			}
			if ( quad[2]->v > 1.0f )
			{
				quad[2]->v = 1.0f;
			}
		}

		//
		// Is this texture used in our call?
		//

		texture = SUPERFACET_convert_texture(page, quad);

		if (texture == sc->texture)
		{
			//
			// Add this quad to the call structure. First four points.
			//


			#ifdef TESSELATE_DEGREE
			//Make some more.

			float fXNorm, fZNorm;
			switch ( m_iFacetDirection )
			{
			case 0:
				fZNorm = 1.0f;
				fXNorm = 0.0f;
				break;
			case 1:
				fZNorm = -1.0f;
				fXNorm = 0.0f;
				break;
			case 2:
				fZNorm = 0.0f;
				fXNorm = 1.0f;
				break;
			case 3:
				fZNorm = 0.0f;
				fXNorm = -1.0f;
				break;
			}
			// Random scale.
			fXNorm *= 64.0f;
			fZNorm *= 64.0f;


			j = 0;
			int iX, iY;
			for ( iY = 0; iY <= TESSELATE_DEGREE; iY++ )
			{
				for ( iX = 0; iX <= TESSELATE_DEGREE; iX++ )
				{
					float fXLerp = (float)iX / (float)TESSELATE_DEGREE;
					float fYLerp = (float)iY / (float)TESSELATE_DEGREE;
					int iPerturb = ( iX << 6 ) ^ ( iX << 3 ) ^ ( iY << 5 ) ^ ( iY << 1 ) ^ ( page << 1 ) ^ ( page << 5 );
					float fPerturb = (float)( iPerturb & 0xff ) * ( 1.0f / 256.0f ) - 0.5f;
					// Stop boring flat perturbs.
					if ( ( fPerturb > -0.2f ) || ( fPerturb < 0.2f ) )
					{
						fPerturb *= 5.0f;
					}
					if ( ( iX == 0 ) || ( iX == TESSELATE_DEGREE ) || ( iY == 0 ) || ( iY == TESSELATE_DEGREE ) )
					{
						// Edges smooth.
						fPerturb = 0.0f;
					}

					float fX1 = ( quad[0]->x * fXLerp ) + ( quad[1]->x * ( 1.0f - fXLerp ) );
					float fX2 = ( quad[2]->x * fXLerp ) + ( quad[3]->x * ( 1.0f - fXLerp ) );
					float fY1 = ( quad[0]->y * fXLerp ) + ( quad[1]->y * ( 1.0f - fXLerp ) );
					float fY2 = ( quad[2]->y * fXLerp ) + ( quad[3]->y * ( 1.0f - fXLerp ) );
					float fZ1 = ( quad[0]->z * fXLerp ) + ( quad[1]->z * ( 1.0f - fXLerp ) );
					float fZ2 = ( quad[2]->z * fXLerp ) + ( quad[3]->z * ( 1.0f - fXLerp ) );
					float fU1 = ( quad[0]->u * fXLerp ) + ( quad[1]->u * ( 1.0f - fXLerp ) );
					float fU2 = ( quad[2]->u * fXLerp ) + ( quad[3]->u * ( 1.0f - fXLerp ) );
					float fV1 = ( quad[0]->v * fXLerp ) + ( quad[1]->v * ( 1.0f - fXLerp ) );
					float fV2 = ( quad[2]->v * fXLerp ) + ( quad[3]->v * ( 1.0f - fXLerp ) );
					float fX = ( fX1 * fYLerp ) + ( fX2 * ( 1.0f - fYLerp ) );
					float fY = ( fY1 * fYLerp ) + ( fY2 * ( 1.0f - fYLerp ) );
					float fZ = ( fZ1 * fYLerp ) + ( fZ2 * ( 1.0f - fYLerp ) );
					float fU = ( fU1 * fYLerp ) + ( fU2 * ( 1.0f - fYLerp ) );
					float fV = ( fV1 * fYLerp ) + ( fV2 * ( 1.0f - fYLerp ) );

					ASSERT(WITHIN(SUPERFACET_lvert_upto+j, 0, SUPERFACET_MAX_LVERTS - 1));

					lv = &SUPERFACET_lvert[SUPERFACET_lvert_upto + j];

					lv->x        = fX + fPerturb * fXNorm;
					lv->y        = fY;
					lv->z        = fZ + fPerturb * fZNorm;
					#ifdef SHOW_ME_SUPERFACET_DEBUGGING_PLEASE_BOB
					if ( m_bShowDebuggingInfo )
					{
						// Make a pretty colour from the call pointer,
						// so we can see the efficiency.
						DWORD dwColour = (DWORD)sc;
						dwColour = ( dwColour >> 2 ) ^ ( dwColour >> 6 ) ^ ( dwColour ) ^ ( dwColour << 3 );
						dwColour = ( dwColour << 9 ) ^ ( dwColour << 19 ) ^ ( dwColour ) ^ ( dwColour << 29 ) ^ ( dwColour >> 3 );
						dwColour &= 0x7f7f7f7f;
						lv->color    = dwColour;
						lv->specular = dwColour;
					}
					else
					#endif
					{
						lv->specular = quad[0]->specular;
						lv->color    = quad[0]->colour;
					}
					lv->tu       = fU;
					lv->tv       = fV;

					j++;
				}
			}

			//
			// Now strip indices.
			//

			ASSERT(WITHIN(SUPERFACET_index_upto + j, 0, SUPERFACET_MAX_INDICES - 1));

			int iCurStart = 0;
			for ( iY = 0; iY < TESSELATE_DEGREE; iY++ )
			{
				// Do the first two indices.
				SUPERFACET_index[SUPERFACET_index_upto + 0] = SUPERFACET_lvert_upto - sc->lvert + iCurStart + 0;
				SUPERFACET_index[SUPERFACET_index_upto + 1] = SUPERFACET_lvert_upto - sc->lvert + iCurStart + (TESSELATE_DEGREE+1);
				SUPERFACET_index_upto += 2;
				for ( iX = 0; iX < TESSELATE_DEGREE; iX++ )
				{
					SUPERFACET_index[SUPERFACET_index_upto + 0] = SUPERFACET_lvert_upto - sc->lvert + iCurStart + iX + 1;
					SUPERFACET_index[SUPERFACET_index_upto + 1] = SUPERFACET_lvert_upto - sc->lvert + iCurStart + iX + 1 + (TESSELATE_DEGREE+1);
					SUPERFACET_index_upto += 2;
				}
				// Terminate the strip.
				SUPERFACET_index[SUPERFACET_index_upto] = -1;
				SUPERFACET_index_upto++;
				iCurStart += (TESSELATE_DEGREE+1);
			}

			sc->lvertcount += (TESSELATE_DEGREE+1)*(TESSELATE_DEGREE+1);
			sc->indexcount += (TESSELATE_DEGREE)*( (TESSELATE_DEGREE+1) * 2 + 1 );

			SUPERFACET_lvert_upto += (TESSELATE_DEGREE+1)*(TESSELATE_DEGREE+1);

			ASSERT(sc->lvert + sc->lvertcount == SUPERFACET_lvert_upto);
			ASSERT(sc->index + sc->indexcount == SUPERFACET_index_upto);
			#else

			//ASSERT(abs(quad[0]->y - quad[2]->y) <= 257);

			for (j = 0; j < 4; j++)
			{
				ASSERT(WITHIN(SUPERFACET_lvert_upto, 0, SUPERFACET_MAX_LVERTS - 1));

				lv = &SUPERFACET_lvert[SUPERFACET_lvert_upto + j];

				lv->x        = quad[j]->x;
				lv->y        = quad[j]->y;
				lv->z        = quad[j]->z;

				#ifdef SHOW_ME_SUPERFACET_DEBUGGING_PLEASE_BOB
				if ( m_bShowDebuggingInfo || ( abs(quad[0]->y - quad[2]->y) > 257 ) )
				{
					// Make a pretty colour from the call pointer,
					// so we can see the efficiency.

					// It also shows up things that won't repeat properly. There aren't many, and we can ignore them.
					// (e.g. wall of the 
					DWORD dwColour = (DWORD)sc;
					dwColour = ( dwColour >> 2 ) ^ ( dwColour >> 6 ) ^ ( dwColour ) ^ ( dwColour << 3 );
					dwColour = ( dwColour << 9 ) ^ ( dwColour << 19 ) ^ ( dwColour ) ^ ( dwColour << 29 ) ^ ( dwColour >> 3 );
					dwColour &= 0x7f7f7f7f;
					lv->color    = dwColour;
					lv->specular = dwColour;
				}
					else
				#endif
				{
					lv->color    = quad[j]->colour;
					lv->specular = quad[j]->specular;
				}
				lv->tu         = quad[j]->u;
				lv->tv         = quad[j]->v;
				lv->dwReserved = quad[j]->user << 16;
			}

			//
			// Now six indices.
			//

			ASSERT(WITHIN(SUPERFACET_index_upto + 4, 0, SUPERFACET_MAX_INDICES - 1));

#if 0
			// Striplists
			SUPERFACET_index[SUPERFACET_index_upto + 0] = SUPERFACET_lvert_upto - sc->lvert + 0;
			SUPERFACET_index[SUPERFACET_index_upto + 1] = SUPERFACET_lvert_upto - sc->lvert + 2;
			SUPERFACET_index[SUPERFACET_index_upto + 2] = SUPERFACET_lvert_upto - sc->lvert + 1;
			SUPERFACET_index[SUPERFACET_index_upto + 3] = SUPERFACET_lvert_upto - sc->lvert + 3;
			SUPERFACET_index[SUPERFACET_index_upto + 4] = 0xffff;

			sc->lvertcount += 4;
			sc->indexcount += 5;

			SUPERFACET_lvert_upto += 4;
			SUPERFACET_index_upto += 5;
#else
			// Lists
			SUPERFACET_index[SUPERFACET_index_upto + 0] = SUPERFACET_lvert_upto - sc->lvert + 0;
			SUPERFACET_index[SUPERFACET_index_upto + 1] = SUPERFACET_lvert_upto - sc->lvert + 2;
			SUPERFACET_index[SUPERFACET_index_upto + 2] = SUPERFACET_lvert_upto - sc->lvert + 1;
			SUPERFACET_index[SUPERFACET_index_upto + 3] = SUPERFACET_lvert_upto - sc->lvert + 1;
			SUPERFACET_index[SUPERFACET_index_upto + 4] = SUPERFACET_lvert_upto - sc->lvert + 2;
			SUPERFACET_index[SUPERFACET_index_upto + 5] = SUPERFACET_lvert_upto - sc->lvert + 3;

			sc->lvertcount += 4;
			sc->indexcount += 6;

			SUPERFACET_lvert_upto += 4;
			SUPERFACET_index_upto += 6;
#endif

			ASSERT(sc->lvert + sc->lvertcount == SUPERFACET_lvert_upto);
			ASSERT(sc->index + sc->indexcount == SUPERFACET_index_upto);

			#endif
		}
	}

//	for (;i < count; i++)
	{
		facet_rand();
	}
}










//
// Builds the 'call'th call structure for the given facet.
//

void SUPERFACET_build_call(SLONG facet, SLONG call)
{
	SLONG i;
	SLONG j;
	SLONG dx;
	SLONG dz;
	SLONG hf;
	SLONG height;
	SLONG count;
	SLONG foundation;
	SLONG style_index;
	SLONG style_index_step;

	float block_height;

	LPDIRECT3DTEXTURE2 texture;
	DFacet            *df;
	SUPERFACET_Facet  *sf;
	SUPERFACET_Call   *sc;
	POLY_Point        *quad[4];

	ASSERT(WITHIN(facet, 0, next_dfacet                 - 1));
	ASSERT(WITHIN(facet, 0, SUPERFACET_MAX_FACETS       - 1));

	df = &dfacets         [facet];
	sf = &SUPERFACET_facet[facet];

	ASSERT(WITHIN(call,            0, sf->num              - 1));
	ASSERT(WITHIN(sf->call + call, 0, SUPERFACET_MAX_CALLS - 1));

	sc = &SUPERFACET_call[sf->call + call];

	//
	// Initialise this call. This texture should already be setup.
	//

	sc->index      = SUPERFACET_index_upto;
	sc->lvert      = SUPERFACET_lvert_upto;
	sc->indexcount = 0;
	sc->lvertcount = 0;

	//
	// Set the seed for the random facet texture system.
	//

	set_facet_seed(df->x[0] * df->z[0] + df->Y[0]);

	//
	// How long is the wall? No diagonal walls allowed.
	//

	dx = (df->x[1] - df->x[0]) << 8;
	dz = (df->z[1] - df->z[0]) << 8;

	ASSERT(!(dx && dz));

	if (dx) {count = abs(dx) >> 8;}
	else    {count = abs(dz) >> 8;}

	count++;	// Count is the number of points, not the number of texture squares...

	//
	// Set up the styles...
	//

	style_index = df->StyleIndex;

	if (df->FacetFlags & FACET_FLAG_2TEXTURED)
	{
		style_index -= 1;
	}

	if (!(df->FacetFlags & FACET_FLAG_HUG_FLOOR) && (df->FacetFlags & (FACET_FLAG_2TEXTURED | FACET_FLAG_2SIDED)))
	{
		style_index_step = 2;
	}
	else
	{
		style_index_step = 1;
	}

	//
	// Do we have a foundation?
	//

	if (df->FHeight)
	{
		foundation = 2;
	}

	block_height = float(df->BlockHeight << 4);
	height       = df->Height;

	//
	// Go through the facet and find all the quads that use
	// this call's texture.
	//

	hf = 0;

	while (height >= 0)
	{
		if (hf)
		{
			SUPERFACET_fill_facet_points(
				count,
				hf - 1,
				foundation + 1,
				style_index - 1,
				block_height,
				sc);
		}

		height      -= 4;
		hf          += 1;
		foundation  -= 1;
		style_index += style_index_step;
	}

#ifdef TESSELATE_DEGREE
	ASSERT(sc->lvertcount == sc->quads * (TESSELATE_DEGREE+1)*(TESSELATE_DEGREE+1));
#else
	ASSERT(sc->lvertcount == sc->quads * 4);
#endif

	/*

	#ifndef TARGET_DC

	ASSERT(sc->indexcount);
	ASSERT(sc->lvertcount);

	#endif

	*/

	SUPERFACET_free_range_start = sc->lvert + sc->lvertcount;

	return;
}



//
// Create a call for each new texture page.
//

void SUPERFACET_create_calls(SLONG facet, SLONG direction)
{
	SLONG i;
	SLONG j;
	SLONG dx;
	SLONG dz;
	SLONG hf;
	SLONG page;
	SLONG count;
	SLONG height;
	SLONG style_index;
	SLONG style_index_step;

	LPDIRECT3DTEXTURE2 texture;
	DFacet            *df;
	SUPERFACET_Facet  *sf;
	SUPERFACET_Call   *sc;
	POLY_Point        *quad[4];

	ASSERT(WITHIN(facet, 0, next_dfacet           - 1));
	ASSERT(WITHIN(facet, 0, SUPERFACET_MAX_FACETS - 1));

	df = &dfacets         [facet];
	sf = &SUPERFACET_facet[facet];

	//
	// Set the seed for the random facet texture system.
	//

	set_facet_seed(df->x[0] * df->z[0] + df->Y[0]);

	//
	// How long is the wall? No diagonal walls allowed.
	//

	dx = (df->x[1] - df->x[0]) << 8;
	dz = (df->z[1] - df->z[0]) << 8;

	ASSERT(!(dx && dz));

	if (dx) {count = abs(dx) >> 8;}
	else    {count = abs(dz) >> 8;}

	count++;	// Count is the number of points, not the number of texture squares...

	//
	// Set up the styles...
	//

	style_index = df->StyleIndex;

	if (df->FacetFlags & FACET_FLAG_2TEXTURED)
	{
		style_index -= 1;
	}

	if (!(df->FacetFlags & FACET_FLAG_HUG_FLOOR) && (df->FacetFlags & (FACET_FLAG_2TEXTURED | FACET_FLAG_2SIDED)))
	{
		style_index_step = 2;
	}
	else
	{
		style_index_step = 1;
	}

	//
	// Make 'quad' point to a junk bit of memory.
	//

	POLY_Point pp[4];

	memset(pp, 0, sizeof(pp));

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// Go through the facet and find all the textures used by the
	// facet.
	//

	hf     = 0;
	height = df->Height;

	while (height >= 0)
	{
		if (hf)
		{
			for (i = 0; i < count - 1; i++)
			{
				page = texture_quad(quad, dstyles[style_index - 1], i, count);

				//
				// Find the actual d3d texture used.
				//

				texture = SUPERFACET_convert_texture(page, quad);

				// Some textures are indeed NULL, if they are missing.
				//ASSERT ( texture != NULL );

				//
				// Do we already have this texture?
				//

				for (j = 0; j < sf->num; j++)
				{
					ASSERT(WITHIN(sf->call + j, 0, SUPERFACET_MAX_CALLS - 1));

					sc = &SUPERFACET_call[sf->call + j];

					if (sc->texture == texture)
					{
						//
						// Already have a SUPERFACET_Call for this texture.
						//

						if (SUPERCRINKLE_IS_CRINKLED(page))
						{
							//
							// We need the exact page.
							//

							if ((sc->flag & SUPERFACET_CALL_FLAG_CRINKLED) && sc->crinkle == page)
							{
								sc->quads += 1;

								goto already_got_a_call;
							}
						}
						else
						{
							sc->quads += 1;

							goto already_got_a_call;
						}
					}
				}

				//
				// Create a new call structure for this facet.
				//

				ASSERT(WITHIN(SUPERFACET_call_upto, 0, SUPERFACET_MAX_CALLS - 1));
				ASSERT(SUPERFACET_call_upto == sf->call + sf->num);

				sc = &SUPERFACET_call[SUPERFACET_call_upto++];

				ASSERT(!(sc->flag & SUPERFACET_CALL_FLAG_USED));

				sc->flag    = SUPERFACET_CALL_FLAG_USED;
				sc->texture = texture;
				sc->quads   = 1;	// Just to be sure...
				sc->dir     = direction;

				if (POLY_page_flag[page] & POLY_PAGE_FLAG_2PASS)
				{
					sc->flag          |= SUPERFACET_CALL_FLAG_2PASS;
					sc->texture_2pass  = SUPERFACET_convert_texture(page + 1, quad);
				}

				if (page < 512 && SUPERCRINKLE_IS_CRINKLED(page))
				{
					sc->flag   |= SUPERFACET_CALL_FLAG_CRINKLED;
					sc->crinkle = page;
				}

				sf->num    += 1;

			  already_got_a_call:;
			}

			facet_rand();
		}

		height      -= 4;
		hf          += 1;
		style_index += style_index_step;
	}

	return;
}








void SUPERFACET_convert_facet(SLONG facet)
{
	SLONG i;
	SLONG room;
	SLONG memory;
	SLONG direction;

	DFacet           *df;
	SUPERFACET_Facet *sf;
	SUPERFACET_Call  *sc;

	#ifdef SUPERFACET_PERFORMANCE
	SUPERFACET_total_converted += 1;
	#endif

	ASSERT(WITHIN(facet, 1, next_dfacet - 1));

	df = &dfacets[facet];

	if (df->Dfcache == NULL)
	{
		//
		// Create cached lighting for the facet.
		//

		df->Dfcache = NIGHT_dfcache_create(facet);

		ASSERT(df->Dfcache);
	}

	//
	// What direction does this facet point in?
	//

	if (df->z[0] == df->z[1])
	{
		if (df->x[0] < df->x[1])
		{
			direction = 0;
		}
		else
		{
			direction = 2;
		}
	}
	else
	{
		if (df->z[0] > df->z[1])
		{
			direction = 3;
		}
		else
		{
			direction = 1;
		}
	}

	//
	// Make sure there'll be enough call structures... On RTA
	// there was a max of 12 call structures per facet- use
	// 32 just in case.
	//

	if (SUPERFACET_call_upto > SUPERFACET_MAX_CALLS - 32)
	{
		SUPERFACET_call_upto = 0;
	}

	//
	// Find all the texture pages used by the facet and
	// create a SUPERFACET_call for each one of them.
	//

	ASSERT(WITHIN(facet, 0, SUPERFACET_MAX_FACETS - 1));

	sf = &SUPERFACET_facet[facet];

	sf->call = SUPERFACET_call_upto;
	sf->num  = 0;

	SUPERFACET_create_calls(facet, direction);

	//
	// How much lvert/index memory do we need?
	//
	
	memory = 0;

	for (i = 0; i < sf->num; i++)
	{
		ASSERT(WITHIN(sf->call + i, 0, SUPERFACET_MAX_CALLS - 1));
		
		sc = &SUPERFACET_call[sf->call + i];

		memory += sc->quads * 4;	// 4 verts per quad.
	}

//	ASSERT(memory < 200 * 4);

	//
	// Free up old facets until we have enough room.
	//

	while(1)
	{
		if (SUPERFACET_free_range_start <= SUPERFACET_free_range_end)
		{
			room = SUPERFACET_free_range_end - SUPERFACET_free_range_start;
		}
		else
		{
			room = SUPERFACET_MAX_LVERTS - SUPERFACET_free_range_start;
		}

		if (room >= memory)
		{
			break;
		}

		if (SUPERFACET_free_range_start > SUPERFACET_free_range_end)
		{
			//
			// We've looped around...
			//

			SUPERFACET_free_range_start = 0;
			SUPERFACET_lvert_upto       = 0;
			SUPERFACET_index_upto       = 0;
		}

		SUPERFACET_free_end_of_queue();
	}

	//
	// Build the points.
	//

	SUPERFACET_create_points(facet);

	//
	// Create each call.
	//

	for (i = 0; i < sf->num; i++)
	{
		SUPERFACET_build_call(facet, i);
	}

	//
	// Get rid of the cached lighting.
	//

	NIGHT_dfcache_destroy(df->Dfcache);

	df->Dfcache = NULL;

	//
	// Add to the queue of facets.
	//

	ASSERT(SUPERFACET_queue_start < SUPERFACET_queue_end + SUPERFACET_QUEUE_SIZE);

	SUPERFACET_queue[SUPERFACET_queue_start & (SUPERFACET_QUEUE_SIZE - 1)] = facet;

	SUPERFACET_queue_start++;

	#ifdef SUPERFACET_PERFORMANCE
	fprintf(SUPERFACET_handle, "Game turn %5d converted facet %4d\n", GAME_TURN, facet);
	fflush(SUPERFACET_handle);
	#endif
}
 

void SUPERFACET_init()
{
	#ifdef SUPERFACET_PERFORMANCE

	if (SUPERFACET_handle == NULL)
	{
		SUPERFACET_handle = fopen("facetcache.txt", "wb");
	}

	SUPERFACET_total_freed          = 0;
	SUPERFACET_total_converted      = 0;
	SUPERFACET_total_already_cached = 0;
	SUPERFACET_num_gameturns        = 0;

	#endif

	//
	// How big should our arrays be?
	//

	SUPERFACET_max_facets  = next_dfacet;
#ifdef TESSELATE_DEGREE
	// Mad amount of memory.
	SUPERFACET_max_lverts  = 40000;
#else
	SUPERFACET_max_lverts  = 8192;
#endif

#if 0
	// For striplists
	SUPERFACET_max_indices = SUPERFACET_max_lverts * 5 / 4;
#else
	// For lists
	SUPERFACET_max_indices = SUPERFACET_max_lverts * 6 / 4;
#endif

	//
	// Allocate memory.
	//

	SUPERFACET_lvert_buffer = (UBYTE            *) MemAlloc(sizeof(D3DLVERTEX)       * SUPERFACET_MAX_LVERTS + 32);
	ASSERT ( SUPERFACET_lvert_buffer != NULL );
	SUPERFACET_index        = (UWORD            *) MemAlloc(sizeof(UWORD)            * SUPERFACET_MAX_INDICES    );
	ASSERT ( SUPERFACET_index != NULL );
	SUPERFACET_facet        = (SUPERFACET_Facet *) MemAlloc(sizeof(SUPERFACET_Facet) * SUPERFACET_MAX_FACETS     );
	ASSERT ( SUPERFACET_facet != NULL );

	//
	// Initialise data.
	//

	memset(SUPERFACET_lvert_buffer, 0, sizeof(D3DLVERTEX) * SUPERFACET_MAX_LVERTS + 32);
	memset(SUPERFACET_index,        0, sizeof(UWORD)      * SUPERFACET_MAX_INDICES    );
	memset(SUPERFACET_call,         0, sizeof(SUPERFACET_call));
	memset(SUPERFACET_facet,        0, sizeof(SUPERFACET_Facet) * SUPERFACET_MAX_FACETS);
	memset(SUPERFACET_queue,        0, sizeof(SUPERFACET_queue));

	SUPERFACET_queue_start = 0;
	SUPERFACET_queue_end   = 0;

	SUPERFACET_free_range_start = 0;
	SUPERFACET_free_range_end   = SUPERFACET_MAX_LVERTS;

	SUPERFACET_lvert = (D3DLVERTEX *) ((SLONG(SUPERFACET_lvert_buffer) + 31) & ~0x1f);

	SUPERFACET_lvert_upto  = 0;
	SUPERFACET_index_upto  = 0;
	SUPERFACET_call_upto   = 0;
	SUPERFACET_queue_start = 0;
	SUPERFACET_queue_end   = 0;

	SUPERFACET_matrix = (D3DMATRIX *) ((SLONG(SUPERFACET_matrix_buffer) + 31) & ~0x1f);

	//
	// Build the direction matrices.
	//

	SLONG i;

	for (i = 0; i < 4; i++)
	{
		MATRIX_calc(
			SUPERFACET_direction_matrix[i],
			float(i) * (PI / 2),
			0.0F,
			0.0F);
	}

	//
	// This bit of code pre-converts all the suitable facets...
	//

	#if 0

	//
	// Now go through and find 'common' facets.
	//

	SLONG i;

	DFacet *df;

	for (i = 1; i < next_dfacet; i++)
	{
		df = &dfacets[i];

		if (df->FacetFlags & FACET_FLAG_INVISIBLE)
		{
			continue;
		}

		if (df->FacetType != STOREY_TYPE_NORMAL)
		{
			continue;
		}

		if (df->FacetFlags & (FACET_FLAG_BARB_TOP | FACET_FLAG_2SIDED | FACET_FLAG_INSIDE))
		{
			continue;
		}

		if (df->Open)
		{
			continue;
		}

		//
		// Now convert each of these 'common' facets.
		//

		SUPERFACET_convert_facet(i);
	}

	#endif
}



void SUPERFACET_redo_lighting(SLONG facet)
{
	SLONG i;
	SLONG j;

	DFacet           *df;
	SUPERFACET_Call  *sc;
	SUPERFACET_Facet *sf;
	D3DLVERTEX       *lv;

	df = &dfacets         [facet];
	sf = &SUPERFACET_facet[facet];

					ASSERT(0);
	if (df->Dfcache == NULL)
	{
		//
		// Create cached lighting for the facet.
		//

		df->Dfcache = NIGHT_dfcache_create(facet);

		ASSERT(df->Dfcache);
	}

	//
	// Go through each call structure
	//

	NIGHT_Colour *col = NIGHT_dfcache[df->Dfcache].colour;

	for (i = 0; i < sf->num; i++)
	{
		ASSERT(WITHIN(sf->call + i, 0, SUPERFACET_MAX_CALLS - 1));

		sc = &SUPERFACET_call[sf->call + i];

		ASSERT(sc->flag & SUPERFACET_CALL_FLAG_USED);

		//
		// Go through all this SUPERFACET's points and change their lighting.
		//

		for (j = 0; j < sc->lvertcount; j++)
		{
			lv = &SUPERFACET_lvert[sc->lvert + j];

			ASSERT(WITHIN(lv->dwReserved >> 16, 0, 2048));	// Sensible number of vertices???

			NIGHT_get_d3d_colour(col[lv->dwReserved >> 16], &lv->color, &lv->specular);
		}
	}
}





void SUPERFACET_start_frame()
{
	//
	// Initialise Tom's doberries...
	//



#ifdef SHOW_ME_SUPERFACET_DEBUGGING_PLEASE_BOB
#ifdef DEBUG
#ifdef TARGET_DC
	#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)
extern DIJOYSTATE the_state;
	bool bShowDebug = FALSE;
	if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) && BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) )
	{
		bShowDebug = TRUE;
	}

	if ( m_bShowDebuggingInfo != bShowDebug )
	{
		m_bShowDebuggingInfo = bShowDebug;
		// And clear all the facets, so they can be remade in the right colours.

		memset(SUPERFACET_lvert_buffer, 0, sizeof(D3DLVERTEX) * SUPERFACET_MAX_LVERTS + 32);
		memset(SUPERFACET_index,        0, sizeof(UWORD)      * SUPERFACET_MAX_INDICES    );
		memset(SUPERFACET_call,         0, sizeof(SUPERFACET_call));
		memset(SUPERFACET_facet,        0, sizeof(SUPERFACET_Facet) * SUPERFACET_MAX_FACETS);
		memset(SUPERFACET_queue,        0, sizeof(SUPERFACET_queue));

		SUPERFACET_queue_start = 0;
		SUPERFACET_queue_end   = 0;

		SUPERFACET_free_range_start = 0;
		SUPERFACET_free_range_end   = SUPERFACET_MAX_LVERTS;

		SUPERFACET_lvert = (D3DLVERTEX *) ((SLONG(SUPERFACET_lvert_buffer) + 31) & ~0x1f);

		SUPERFACET_lvert_upto  = 0;
		SUPERFACET_index_upto  = 0;
		SUPERFACET_call_upto   = 0;
		SUPERFACET_queue_start = 0;
		SUPERFACET_queue_end   = 0;

		SUPERFACET_matrix = (D3DMATRIX *) ((SLONG(SUPERFACET_matrix_buffer) + 31) & ~0x1f);

	}
#endif
#endif
#endif




//#ifdef TARGET_DC
//extern void POLY_set_local_rotation_none ( void );
//	POLY_set_local_rotation_none();
//	//POLY_flush_local_rot();
//#endif
#if 0
	GenerateMMMatrixFromStandardD3DOnes(
		SUPERFACET_matrix,
	    &g_matProjection,
	    //g_matWorld,
		NULL,
	    &g_viewData);
#endif
	#ifdef SUPERFACET_PERFORMANCE
	SUPERFACET_num_gameturns += 1;
	#endif

	//
	// We are using DrawIndexedPrimitive so must call this...
	//

	POLY_set_local_rotation_none();

	POLY_flush_local_rot();
}


#if 0
//
// The INDEX buffer we use for the 2-pass calls.
//

#define SUPERFACET_MAX_2PASS_INDICES (6 * 256)	// 256 lit windows per facet maximum...

UWORD SUPERFACET_2pass_index[SUPERFACET_MAX_2PASS_INDICES];
SLONG SUPERFACET_2pass_index_upto;
#endif


SLONG SUPERFACET_draw(SLONG facet)
{
	SLONG i;
	SLONG j;

	SUPERFACET_Facet *sf;
	SUPERFACET_Call  *sc;

	D3DMULTIMATRIX d3dmm;

	#ifdef SUPERFACET_PERFORMANCE
	SUPERFACET_total_drawn += 1;
	#endif

	#ifndef TARGET_DC

	//
	// Not on the PC!
	//

	if (Keys[KB_R])
	{
		return FALSE;
	}

	#endif

	//
	// Do we have any pre-calculated info for this facet?
	//

	ASSERT(WITHIN(facet, 0, SUPERFACET_MAX_FACETS - 1));

	sf = &SUPERFACET_facet[facet];

	if (sf->num == 0)
	{
		//
		// No cached info, create it!
		//

		SUPERFACET_convert_facet(facet);
	}
	#ifdef SUPERFACET_PERFORMANCE
	else
	{
		SUPERFACET_total_already_cached += 1;
	}
	#endif

	if (dfacets[facet].FacetFlags & FACET_FLAG_DLIT)
	{
		//
		// We must re-create the lighting for this facet. :(
		//

		SUPERFACET_redo_lighting(facet);
	}


	POLY_set_local_rotation_none();
	POLY_flush_local_rot();

	//
	// Do a drawindexprim call per 'call'
	//


	// Some default settings, for later.
#if 0
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA );
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
#else
	// Should do this properly.
	the_display.SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
	the_display.SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA );
	the_display.SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	the_display.SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
#endif



	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_TEXCOORDINDEX, 0 );

	the_display.lp_D3D_Device->SetTextureStageState ( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	the_display.lp_D3D_Device->SetTextureStageState ( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );



#if 1
	// Using DrawIndPrim all the time, not MM.
	// Means we don't have to clip.


	for (i = 0; i < sf->num; i++)
	{
		ASSERT(WITHIN(sf->call + i, 0, SUPERFACET_MAX_CALLS - 1));

		sc = &SUPERFACET_call[sf->call + i];

		ASSERT(sc->flag & SUPERFACET_CALL_FLAG_USED);

		if (!sc->indexcount)
		{
			//
			// Oh dear...
			//

			continue;
		}

		//
		// Setup renderstates...?
		//

		the_display.lp_D3D_Device->SetTexture(0, sc->texture);

		//ASSERT ( sc->texture != NULL );

		if (sc->flag & SUPERFACET_CALL_FLAG_CRINKLED)
		{
			//
			// Draw each quad separately...
			//

			// Doesn't work yet.
			ASSERT ( FALSE );

			ULONG colour  [4];
			ULONG specular[4];

			SLONG index[4];

			for (j = 0; j < sc->indexcount; j += 6)
			{
				index[0] = sc->lvert + SUPERFACET_index[sc->index + j + 0];
				index[1] = sc->lvert + SUPERFACET_index[sc->index + j + 1];
				index[2] = sc->lvert + SUPERFACET_index[sc->index + j + 2];
				index[3] = sc->lvert + SUPERFACET_index[sc->index + j + 5];

				//ASSERT(SUPERFACET_index[sc->index + j + 4] == 0xffff);

				ASSERT(WITHIN(index[0], 0, SUPERFACET_MAX_LVERTS - 1));
				ASSERT(WITHIN(index[1], 0, SUPERFACET_MAX_LVERTS - 1));
				ASSERT(WITHIN(index[2], 0, SUPERFACET_MAX_LVERTS - 1));
				ASSERT(WITHIN(index[3], 0, SUPERFACET_MAX_LVERTS - 1));

				float world_x;
				float world_y;
				float world_z;

				if (SUPERFACET_lvert[index[0]].tu > SUPERFACET_lvert[index[2]].tu)
				{
					//
					// We must x-flip the crinkle...
					//

					ASSERT(WITHIN(sc->dir, 0, 3));

					world_x = SUPERFACET_lvert[index[1]].x;
					world_y = SUPERFACET_lvert[index[1]].y;
					world_z = SUPERFACET_lvert[index[1]].z;

					SUPERFACET_direction_matrix[sc->dir][0] = -SUPERFACET_direction_matrix[sc->dir][0];
					SUPERFACET_direction_matrix[sc->dir][1] = -SUPERFACET_direction_matrix[sc->dir][1];
					SUPERFACET_direction_matrix[sc->dir][2] = -SUPERFACET_direction_matrix[sc->dir][2];

					POLY_set_local_rotation(
						world_x,
						world_y,
						world_z,
						SUPERFACET_direction_matrix[sc->dir]);

					SUPERFACET_direction_matrix[sc->dir][0] = -SUPERFACET_direction_matrix[sc->dir][0];
					SUPERFACET_direction_matrix[sc->dir][1] = -SUPERFACET_direction_matrix[sc->dir][1];
					SUPERFACET_direction_matrix[sc->dir][2] = -SUPERFACET_direction_matrix[sc->dir][2];


					static UBYTE order[4] = {2,0,3,1};


					/*
					if ((GAME_TURN & 0x1f) == 0)
					{
						SLONG a;
						SLONG b;

						SLONG twizzle;

						for (twizzle = 0; twizzle < 5; twizzle++)
						{
							a = rand() & 3;
							b = rand() & 3;
							
							SWAP(order[a],order[b]);
						}
					}
					*/

					colour[0] = SUPERFACET_lvert[index[order[0]]].color;
					colour[1] = SUPERFACET_lvert[index[order[1]]].color;
					colour[2] = SUPERFACET_lvert[index[order[2]]].color;
					colour[3] = SUPERFACET_lvert[index[order[3]]].color;

					specular[0] = SUPERFACET_lvert[index[order[0]]].specular;
					specular[1] = SUPERFACET_lvert[index[order[1]]].specular;
					specular[2] = SUPERFACET_lvert[index[order[2]]].specular;
					specular[3] = SUPERFACET_lvert[index[order[3]]].specular;

				}
				else
				{
					world_x = SUPERFACET_lvert[index[3]].x;
					world_y = SUPERFACET_lvert[index[3]].y;
					world_z = SUPERFACET_lvert[index[3]].z;

					ASSERT(WITHIN(sc->dir, 0, 3));			

					POLY_set_local_rotation(
						world_x,
						world_y,
						world_z,
						SUPERFACET_direction_matrix[sc->dir]);

					static UBYTE order[4] = {0,2,1,3};
					
					/*

					if ((GAME_TURN & 0x1f) == 0)
					{
						SLONG a;
						SLONG b;

						SLONG twizzle;

						for (twizzle = 0; twizzle < 5; twizzle++)
						{
							a = rand() & 3;
							b = rand() & 3;
							
							SWAP(order[a],order[b]);
						}
					}

					*/

					colour[0] = SUPERFACET_lvert[index[order[0]]].color;
					colour[1] = SUPERFACET_lvert[index[order[1]]].color;
					colour[2] = SUPERFACET_lvert[index[order[2]]].color;
					colour[3] = SUPERFACET_lvert[index[order[3]]].color;

					specular[0] = SUPERFACET_lvert[index[order[0]]].specular;
					specular[1] = SUPERFACET_lvert[index[order[1]]].specular;
					specular[2] = SUPERFACET_lvert[index[order[2]]].specular;
					specular[3] = SUPERFACET_lvert[index[order[3]]].specular;


				}

				SUPERCRINKLE_draw(
					sc->crinkle,
					colour,
					specular);


				POLY_set_local_rotation_none();
				POLY_flush_local_rot();

			}
		}
		else
		{

#if 0
			//
			// Setup transform states...?
			//

			D3DMULTIMATRIX d3dmm =
			{
				SUPERFACET_lvert + sc->lvert,
				SUPERFACET_matrix,
				NULL,
				NULL
			};
#endif



#if 1
			the_display.lp_D3D_Device->DrawIndexedPrimitive(
											D3DPT_TRIANGLELIST,
											D3DFVF_LVERTEX,
											SUPERFACET_lvert + sc->lvert,
											sc->lvertcount,
											SUPERFACET_index + sc->index,
											sc->indexcount,
											0);

#else
			DrawIndPrimMM(
				the_display.lp_D3D_Device,
				D3DFVF_LVERTEX,
			   &d3dmm,
				sc->lvertcount,
				SUPERFACET_index + sc->index,
				sc->indexcount);
#endif
		}

		//
		// What about those 2-pass texture pages???
		//

		if (sc->flag & SUPERFACET_CALL_FLAG_2PASS)
		{
			//
			// Create a 2pass index list from this index list.
			//

#if 0
			SUPERFACET_2pass_index_upto = 0;
			for (j = 0; j < sc->indexcount; j += 5)
			{
				ASSERT(WITHIN(SUPERFACET_2pass_index_upto + 5, 0, SUPERFACET_MAX_2PASS_INDICES - 1));

				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 0] = SUPERFACET_index[sc->index + j + 0];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 1] = SUPERFACET_index[sc->index + j + 1];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 2] = SUPERFACET_index[sc->index + j + 2];

				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 3] = SUPERFACET_index[sc->index + j + 3];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 4] = SUPERFACET_index[sc->index + j + 2];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 5] = SUPERFACET_index[sc->index + j + 1];

				SUPERFACET_2pass_index_upto += 6;
			}
#endif

			// Switch the modes for the windows.
			the_display.lp_D3D_Device->SetTexture(0, sc->texture_2pass);
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
			the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

#if 1
			the_display.lp_D3D_Device->DrawIndexedPrimitive(
											D3DPT_TRIANGLELIST,
											D3DFVF_LVERTEX,
											SUPERFACET_lvert + sc->lvert,
											sc->lvertcount,
											SUPERFACET_index + sc->index,
											sc->indexcount,
											0);
#else
			the_display.lp_D3D_Device->DrawIndexedPrimitive(
											D3DPT_TRIANGLELIST,
											D3DFVF_LVERTEX,
											SUPERFACET_lvert + sc->lvert,
											sc->lvertcount,
											SUPERFACET_2pass_index,
											SUPERFACET_2pass_index_upto,
											//SUPERFACET_index + sc->index,
											//sc->indexcount,
											0);
#endif

			// And go back to sanity.
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);
			the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );


		}
	}



#else
	// Using DrawIndPrimMM - a pain in the arse.

#define USING_MULTITEX_FOR_WINDOWS 0


	for (i = 0; i < sf->num; i++)
	{
		ASSERT(WITHIN(sf->call + i, 0, SUPERFACET_MAX_CALLS - 1));

		sc = &SUPERFACET_call[sf->call + i];

		ASSERT(sc->flag & SUPERFACET_CALL_FLAG_USED);

		if (!sc->indexcount)
		{
			//
			// Oh dear...
			//

			continue;
		}

		//
		// Setup renderstates...?
		//

		the_display.lp_D3D_Device->SetTexture(0, sc->texture);

		//ASSERT ( sc->texture != NULL );

		if (sc->flag & SUPERFACET_CALL_FLAG_CRINKLED)
		{
			//
			// Draw each quad separately...
			//

			ULONG colour  [4];
			ULONG specular[4];

			SLONG index[4];

			for (j = 0; j < sc->indexcount; j += 5)
			{
				index[0] = sc->lvert + SUPERFACET_index[sc->index + j + 0];
				index[1] = sc->lvert + SUPERFACET_index[sc->index + j + 1];
				index[2] = sc->lvert + SUPERFACET_index[sc->index + j + 2];
				index[3] = sc->lvert + SUPERFACET_index[sc->index + j + 3];

				ASSERT(SUPERFACET_index[sc->index + j + 4] == 0xffff);

				ASSERT(WITHIN(index[0], 0, SUPERFACET_MAX_LVERTS - 1));
				ASSERT(WITHIN(index[1], 0, SUPERFACET_MAX_LVERTS - 1));
				ASSERT(WITHIN(index[2], 0, SUPERFACET_MAX_LVERTS - 1));
				ASSERT(WITHIN(index[3], 0, SUPERFACET_MAX_LVERTS - 1));

				float world_x;
				float world_y;
				float world_z;

				if (SUPERFACET_lvert[index[0]].tu > SUPERFACET_lvert[index[2]].tu)
				{
					//
					// We must x-flip the crinkle...
					//

					ASSERT(WITHIN(sc->dir, 0, 3));

					world_x = SUPERFACET_lvert[index[1]].x;
					world_y = SUPERFACET_lvert[index[1]].y;
					world_z = SUPERFACET_lvert[index[1]].z;

					SUPERFACET_direction_matrix[sc->dir][0] = -SUPERFACET_direction_matrix[sc->dir][0];
					SUPERFACET_direction_matrix[sc->dir][1] = -SUPERFACET_direction_matrix[sc->dir][1];
					SUPERFACET_direction_matrix[sc->dir][2] = -SUPERFACET_direction_matrix[sc->dir][2];

					POLY_set_local_rotation(
						world_x,
						world_y,
						world_z,
						SUPERFACET_direction_matrix[sc->dir]);

					SUPERFACET_direction_matrix[sc->dir][0] = -SUPERFACET_direction_matrix[sc->dir][0];
					SUPERFACET_direction_matrix[sc->dir][1] = -SUPERFACET_direction_matrix[sc->dir][1];
					SUPERFACET_direction_matrix[sc->dir][2] = -SUPERFACET_direction_matrix[sc->dir][2];


					static UBYTE order[4] = {2,0,3,1};


					/*
					if ((GAME_TURN & 0x1f) == 0)
					{
						SLONG a;
						SLONG b;

						SLONG twizzle;

						for (twizzle = 0; twizzle < 5; twizzle++)
						{
							a = rand() & 3;
							b = rand() & 3;
							
							SWAP(order[a],order[b]);
						}
					}
					*/

					colour[0] = SUPERFACET_lvert[index[order[0]]].color;
					colour[1] = SUPERFACET_lvert[index[order[1]]].color;
					colour[2] = SUPERFACET_lvert[index[order[2]]].color;
					colour[3] = SUPERFACET_lvert[index[order[3]]].color;

					specular[0] = SUPERFACET_lvert[index[order[0]]].specular;
					specular[1] = SUPERFACET_lvert[index[order[1]]].specular;
					specular[2] = SUPERFACET_lvert[index[order[2]]].specular;
					specular[3] = SUPERFACET_lvert[index[order[3]]].specular;

				}
				else
				{
					world_x = SUPERFACET_lvert[index[3]].x;
					world_y = SUPERFACET_lvert[index[3]].y;
					world_z = SUPERFACET_lvert[index[3]].z;

					ASSERT(WITHIN(sc->dir, 0, 3));			

					POLY_set_local_rotation(
						world_x,
						world_y,
						world_z,
						SUPERFACET_direction_matrix[sc->dir]);

					static UBYTE order[4] = {0,2,1,3};
					
					/*

					if ((GAME_TURN & 0x1f) == 0)
					{
						SLONG a;
						SLONG b;

						SLONG twizzle;

						for (twizzle = 0; twizzle < 5; twizzle++)
						{
							a = rand() & 3;
							b = rand() & 3;
							
							SWAP(order[a],order[b]);
						}
					}

					*/

					colour[0] = SUPERFACET_lvert[index[order[0]]].color;
					colour[1] = SUPERFACET_lvert[index[order[1]]].color;
					colour[2] = SUPERFACET_lvert[index[order[2]]].color;
					colour[3] = SUPERFACET_lvert[index[order[3]]].color;

					specular[0] = SUPERFACET_lvert[index[order[0]]].specular;
					specular[1] = SUPERFACET_lvert[index[order[1]]].specular;
					specular[2] = SUPERFACET_lvert[index[order[2]]].specular;
					specular[3] = SUPERFACET_lvert[index[order[3]]].specular;


				}

				SUPERCRINKLE_draw(
					sc->crinkle,
					colour,
					specular);

				extern void POLY_set_local_rotation_none(void);

				POLY_set_local_rotation_none();
				POLY_flush_local_rot();

			}
		}
		else
		{
			//
			// Setup transform states...?
			//

			#if 0
			// I keep getting "datatype misalignment" errors on this on the DC. Let's do something more conventional.
			D3DMULTIMATRIX d3dmm =
			{
				SUPERFACET_lvert + sc->lvert,
				SUPERFACET_matrix,
				NULL,
				NULL
			};
			#else
			d3dmm.lpvVertices   = SUPERFACET_lvert + sc->lvert;
			d3dmm.lpd3dMatrices = SUPERFACET_matrix;
			d3dmm.lpvLightDirs  = NULL;
			d3dmm.lpLightTable  = NULL;
			#endif

			/*


			the_display.lp_D3D_Device->DrawIndexedPrimitive(
											D3DPT_TRIANGLELIST,
											D3DFVF_LVERTEX,
											SUPERFACET_lvert + sc->lvert,
											sc->lvertcount,
											SUPERFACET_index + sc->index,
											sc->indexcount,
											0);
			*/

			//TRACE ( "S5" );
			DrawIndPrimMM(
				the_display.lp_D3D_Device,
				D3DFVF_LVERTEX,
			   &d3dmm,
				sc->lvertcount,
				SUPERFACET_index + sc->index,
				sc->indexcount);
			//TRACE ( "F5" );
		}

		//
		// What about those 2-pass texture pages???
		//

		if (sc->flag & SUPERFACET_CALL_FLAG_2PASS)
		{
			//
			// Create a 2pass index list from this index list.
			//

			SUPERFACET_2pass_index_upto = 0;

			for (j = 0; j < sc->indexcount; j += 5)
			{
				ASSERT(WITHIN(SUPERFACET_2pass_index_upto + 5, 0, SUPERFACET_MAX_2PASS_INDICES - 1));

				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 0] = SUPERFACET_index[sc->index + j + 0];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 1] = SUPERFACET_index[sc->index + j + 1];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 2] = SUPERFACET_index[sc->index + j + 2];

				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 3] = SUPERFACET_index[sc->index + j + 3];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 4] = SUPERFACET_index[sc->index + j + 2];
				SUPERFACET_2pass_index[SUPERFACET_2pass_index_upto + 5] = SUPERFACET_index[sc->index + j + 1];

				SUPERFACET_2pass_index_upto += 6;
			}

			the_display.lp_D3D_Device->SetTexture(0, sc->texture_2pass);

			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC,       D3DCMP_NOTEQUAL);
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHAREF,        0              );
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE           );

			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_DECAL);

			/*

			DrawIndPrimMM(
				the_display.lp_D3D_Device,
				D3DFVF_LVERTEX,
			   &d3dmm,
				sc->lvertcount,
				SUPERFACET_index + sc->index,
				sc->indexcount);
			
			*/

			the_display.lp_D3D_Device->DrawIndexedPrimitive(
											D3DPT_TRIANGLELIST,
											D3DFVF_LVERTEX,
											SUPERFACET_lvert + sc->lvert,
											sc->lvertcount,
											SUPERFACET_2pass_index,
											SUPERFACET_2pass_index_upto,
											//SUPERFACET_index + sc->index,
											//sc->indexcount,
											0);

			// And go back to sanity.
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE );
			the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		}
	}


#endif

	return TRUE;
}


void SUPERFACET_fini()
{
	//
	// Free memory.
	//

	MemFree(SUPERFACET_lvert_buffer);
	MemFree(SUPERFACET_index       );
	MemFree(SUPERFACET_facet       );

	#ifdef SUPERFACET_PERFORMANCE

	fprintf(SUPERFACET_handle, "Number of gameturns    = %5d\n", SUPERFACET_num_gameturns       );
	fprintf(SUPERFACET_handle, "Total facets freed     = %5d\n", SUPERFACET_total_freed         );
	fprintf(SUPERFACET_handle, "Total facets converted = %5d\n", SUPERFACET_total_converted     );
	fprintf(SUPERFACET_handle, "Total facets drawn     = %5d\n", SUPERFACET_total_drawn         );
	fprintf(SUPERFACET_handle, "Total facets precached = %5d\n", SUPERFACET_total_already_cached);
	fprintf(SUPERFACET_handle, "Percentage cached      = %5f%%\n", float(SUPERFACET_total_already_cached) * 100.0F / float(SUPERFACET_total_drawn));

	fflush(SUPERFACET_handle);
	
	#endif
}

