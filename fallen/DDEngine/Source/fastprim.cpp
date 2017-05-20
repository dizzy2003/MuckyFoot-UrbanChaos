//
// Draws prims super-fast!
//

#include "game.h"
#include "ddlib.h"
#include "poly.h"
#include "polypoint.h"
#include "polypage.h"
#include "fastprim.h"
#include "memory.h"
#include "texture.h"
#include "matrix.h"


//#define FASTPRIM_PERFORMANCE

#ifdef  FASTPRIM_PERFORMANCE
FILE *FASTPRIM_handle;
SLONG FASTPRIM_num_freed;
SLONG FASTPRIM_num_cached;
SLONG FASTPRIM_num_already_cached;
SLONG FASTPRIM_num_drawn;
#endif


//
// The D3D vertices and indices passed to DrawIndexedPrimitive
// 

D3DLVERTEX *FASTPRIM_lvert_buffer;
D3DLVERTEX *FASTPRIM_lvert;
SLONG       FASTPRIM_lvert_max;
SLONG       FASTPRIM_lvert_upto;
SLONG       FASTPRIM_lvert_free_end;	// The end of the free range of lverts.
SLONG       FASTPRIM_lvert_free_unused;	// From this lvert to the end of the array is unused.

UWORD *FASTPRIM_index;
SLONG  FASTPRIM_index_max;
SLONG  FASTPRIM_index_upto;
SLONG  FASTPRIM_index_free_end;		// The end of the free range of indices
SLONG  FASTPRIM_index_free_unused;	// From this index to the end of the array is unused.

//
// A 32-byte aligned matrix.
//

UBYTE      FASTPRIM_matrix_buffer[sizeof(D3DMATRIX) + 32];
D3DMATRIX *FASTPRIM_matrix;

//
// Each DrawIndexedPrim call has one of these structures.
//

#define FASTPRIM_CALL_TYPE_NORMAL    0  // Uses DrawIndMM - no alpha, no MESH_colour_and, no envmapping...
#define FASTPRIM_CALL_TYPE_COLOURAND 1	// Uses DrawIndMM. The colours for this call should be AND'ed with MESH_colour_and
#define FASTPRIM_CALL_TYPE_INDEXED   2  // Uses DrawIndexedPrimitive and the texture is an alpha texture
#define FASTPRIM_CALL_TYPE_ENVMAP    3  // This is an an environment mapping call - uses DrawIndexedPrimitive and the (u,v) must be set each frame.

#define FASTPRIM_CALL_FLAG_SELF_ILLUM (1 << 0)

typedef struct
{
	UWORD flag;
	UWORD type;

	UWORD lvert;
	UWORD lvertcount;
	UWORD index;
	UWORD indexcount;

	LPDIRECT3DTEXTURE2 texture;
 
} FASTPRIM_Call;

#define FASTPRIM_MAX_CALLS 512

FASTPRIM_Call FASTPRIM_call[FASTPRIM_MAX_CALLS];
SLONG         FASTPRIM_call_upto;


//
// Each prim...
//

#define FASTPRIM_PRIM_FLAG_CACHED  (1 << 0)
#define FASTPRIM_PRIM_FLAG_INVALID (1 << 1)	// This prim cannot be converted for some reason.

typedef struct
{
	UWORD flag;
	UWORD call_index;
	UWORD call_count;

} FASTPRIM_Prim;

#define FASTPRIM_MAX_PRIMS 256

FASTPRIM_Prim FASTPRIM_prim[FASTPRIM_MAX_PRIMS];


//
// Circular queue of prims we have cached.
//

#define FASTPRIM_MAX_QUEUE 128

UBYTE FASTPRIM_queue[FASTPRIM_MAX_QUEUE];
SLONG FASTPRIM_queue_start;
SLONG FASTPRIM_queue_end;




//
// Returns the actual page used.
//

LPDIRECT3DTEXTURE2 FASTPRIM_find_texture_from_page(SLONG page)
{
	SLONG i;

	PolyPage *pp = &POLY_Page[page];

	return pp->RS.GetTexture();
}


#ifdef DEBUG
void *pvJustChecking1 = NULL;
void *pvJustChecking2 = NULL;
#endif


void FASTPRIM_init()
{
	//
	// Allocate memory.
	//

	FASTPRIM_lvert_max         = 4096;
	//FASTPRIM_lvert_max         = 256;


#ifdef DEBUG
	FASTPRIM_lvert_buffer      = (D3DLVERTEX *) MemAlloc(sizeof(D3DLVERTEX) * FASTPRIM_lvert_max + 31 + 32);
	FASTPRIM_lvert             = (D3DLVERTEX *) ((((SLONG) FASTPRIM_lvert_buffer) + 31) & ~0x1f);
	// And write magic numbers just afterwards to check for scribbles.
	char *pcTemp = (char *)( (SLONG)FASTPRIM_lvert + sizeof(D3DLVERTEX) * FASTPRIM_lvert_max );
	strcpy ( pcTemp, "ThisIsAMagicString901234567890" );
#else
	FASTPRIM_lvert_buffer      = (D3DLVERTEX *) MemAlloc(sizeof(D3DLVERTEX) * FASTPRIM_lvert_max + 31);
	FASTPRIM_lvert             = (D3DLVERTEX *) ((((SLONG) FASTPRIM_lvert_buffer) + 31) & ~0x1f);
#endif
	FASTPRIM_lvert_upto        = 0;
	FASTPRIM_lvert_free_end    = FASTPRIM_lvert_max;
	FASTPRIM_lvert_free_unused = FASTPRIM_lvert_max;

	FASTPRIM_index_max         = FASTPRIM_lvert_max * 3;	// Not * 5 / 4 because we may have some extra sharing...
	FASTPRIM_index             = (UWORD *) MemAlloc(sizeof(UWORD) * FASTPRIM_index_max);
	FASTPRIM_index_upto        = 0;
	FASTPRIM_index_free_end    = FASTPRIM_index_max;
	FASTPRIM_index_free_unused = FASTPRIM_index_max;

#ifdef DEBUG
	pvJustChecking1 = (void *)FASTPRIM_lvert_buffer;
	pvJustChecking2 = (void *)FASTPRIM_index;
#endif

	FASTPRIM_call_upto = 0;

	FASTPRIM_queue_start = 0;
	FASTPRIM_queue_end   = 0;

	memset(FASTPRIM_lvert, 0, sizeof(D3DLVERTEX) * FASTPRIM_lvert_max);
	memset(FASTPRIM_index, 0, sizeof(UWORD     ) * FASTPRIM_index_max);
	memset(FASTPRIM_call,  0, sizeof(FASTPRIM_call));
	memset(FASTPRIM_prim,  0, sizeof(FASTPRIM_prim));
	memset(FASTPRIM_queue, 0, sizeof(FASTPRIM_queue));

	FASTPRIM_matrix = (D3DMATRIX *) ((SLONG(FASTPRIM_matrix_buffer) + 31) & ~0x1f);

	//
	// Don't convert car wheels because their texture coordinates rotate!
	//

	FASTPRIM_prim[PRIM_OBJ_CAR_WHEEL].flag = FASTPRIM_PRIM_FLAG_INVALID;

	#ifdef FASTPRIM_PERFORMANCE
	if (!FASTPRIM_handle){FASTPRIM_handle = fopen("c:\\fastprim.txt", "wb");}
	FASTPRIM_num_freed          = 0;
	FASTPRIM_num_cached         = 0;
	FASTPRIM_num_already_cached = 0;
	FASTPRIM_num_drawn          = 0;
	#endif
}


//
// Frees up the given cached prim.
//

void FASTPRIM_free_cached_prim(SLONG prim)
{
	SLONG i;

	FASTPRIM_Call *fc;
	FASTPRIM_Prim *fp;

	ASSERT(WITHIN(prim, 0, FASTPRIM_MAX_PRIMS - 1));

	fp = &FASTPRIM_prim[prim];

	for (i = 0; i < fp->call_count; i++)
	{
		ASSERT(WITHIN(fp->call_index, 0, FASTPRIM_MAX_CALLS - 1));

		fc = &FASTPRIM_call[fp->call_index + i];

		//
		// Free up the verts and indices used by this call.
		//

		FASTPRIM_lvert_free_end = fc->lvert + fc->lvertcount;
		FASTPRIM_index_free_end = fc->index + fc->indexcount;

		if (FASTPRIM_lvert_free_end >= FASTPRIM_lvert_free_unused ||
			FASTPRIM_index_free_end >= FASTPRIM_index_free_unused)
		{
			FASTPRIM_lvert_free_end = FASTPRIM_lvert_max;
			FASTPRIM_index_free_end = FASTPRIM_index_max;

			FASTPRIM_lvert_free_unused = FASTPRIM_lvert_max;
			FASTPRIM_index_free_unused = FASTPRIM_index_max;

			break;
		}
	}

	fp->flag      &= ~FASTPRIM_PRIM_FLAG_CACHED;
	fp->call_count =  0;
	fp->call_index =  0;

	#ifdef FASTPRIM_PERFORMANCE
	fprintf(FASTPRIM_handle, "Gameturn %5d free  prim %3d\n", GAME_TURN, prim);
	fflush(FASTPRIM_handle);
	FASTPRIM_num_freed += 1;
	#endif
}



//
// Frees up the oldest cached prim to make room for the given call
// to grow. It may move all the data in the call!
//

void FASTPRIM_free_queue_for_call(FASTPRIM_Call *fc)
{
	SLONG duplicates = 0;

	SLONG old_lvert_free_end;
	SLONG old_index_free_end;

	SLONG copy_to_beginning = FALSE;

	while(1)
	{
		old_lvert_free_end = FASTPRIM_lvert_free_end;
		old_index_free_end = FASTPRIM_index_free_end;

		//
		// Free up the last cached prim.
		//

		ASSERT(FASTPRIM_queue_end < FASTPRIM_queue_start);

		FASTPRIM_free_cached_prim(FASTPRIM_queue[FASTPRIM_queue_end++ & (FASTPRIM_MAX_QUEUE - 1)]);

		if (FASTPRIM_lvert_free_end < old_lvert_free_end ||
			FASTPRIM_index_free_end < old_index_free_end)
		{
			//
			// The lverts have wrapped around. We must copy all the lverts
			// in the call to the beginning of the array (when we have enough room).
			//

			FASTPRIM_lvert_upto = fc->lvertcount;
			FASTPRIM_index_upto = fc->indexcount;

			copy_to_beginning = TRUE;

			#ifdef FASTPRIM_PERFORMANCE
			fprintf(FASTPRIM_handle, "Wrap...\n");
			fflush(FASTPRIM_handle);
			#endif
		}

		if (FASTPRIM_index_upto + 16 < FASTPRIM_index_free_end &&
			FASTPRIM_lvert_upto + 16 < FASTPRIM_lvert_free_end)
		{
			//
			// Enough room! Do we have to copy the lverts or indices to the
			// beginning of the array?
			//

			if (copy_to_beginning)
			{
				memcpy(FASTPRIM_lvert, FASTPRIM_lvert + fc->lvert, sizeof(D3DLVERTEX) * fc->lvertcount);
				memcpy(FASTPRIM_index, FASTPRIM_index + fc->index, sizeof(UWORD     ) * fc->indexcount);

				FASTPRIM_lvert_free_unused = fc->lvert;
				FASTPRIM_index_free_unused = fc->index;

				fc->lvert = 0;
				fc->index = 0;
			}

			return;
		}
		else
		{
   			duplicates += 1;
		}
	}
}






//
// Makes sure the given FASTPRIM_Prim has a call structure for
// the given texture.
//

void FASTPRIM_create_call(FASTPRIM_Prim *fp, LPDIRECT3DTEXTURE2 texture, UWORD type)
{
	SLONG i;

	FASTPRIM_Call *fc;

	//
	// Do we already have a call structure for this texture?
	//

	for (i = 0; i < fp->call_count; i++)
	{
		ASSERT(WITHIN(fp->call_index + i, 0, FASTPRIM_call_upto - 1));

		fc = &FASTPRIM_call[fp->call_index + i];

		if (fc->texture == texture && fc->type == type)
		{
			return;
		}
	}

	//
	// Create a new call structure.
	//

	ASSERT(WITHIN(FASTPRIM_call_upto, 0, FASTPRIM_MAX_CALLS - 1));
	ASSERT(fp->call_index + fp->call_count == FASTPRIM_call_upto);

	fc = &FASTPRIM_call[fp->call_index + fp->call_count];

	fc->type       = type;
	fc->lvert      = 0;
	fc->lvertcount = 0;
	fc->index      = 0;
	fc->indexcount = 0;
	fc->texture    = texture;

	fp->call_count     += 1;
	FASTPRIM_call_upto += 1;

	return;
}

//
// Adds a point to the given call structure. If the point already
// exists then it returns the old point.
//

UWORD FASTPRIM_add_point_to_call(
		FASTPRIM_Call *fc,
		float x,
		float y,
		float z,
		float u,
		float v,
		ULONG colour,
		ULONG specular)
{
	SLONG i;

	D3DLVERTEX *lv;

	//
	// Does this point already exist?
	//

	for (i = fc->lvertcount - 1; i >= 0; i--)
	{
		ASSERT(WITHIN(fc->lvert + i, 0, FASTPRIM_lvert_max - 1));

		lv = &FASTPRIM_lvert[fc->lvert + i];

		if (lv->x        == x      &&
			lv->y        == y      &&
			lv->z        == z      &&
			lv->tu       == u      &&
			lv->tv       == v      &&
			lv->color    == colour &&
			lv->specular == specular)
		{
			return i;
		}
	}

	//
	// Create a new point.
	//

	if (FASTPRIM_lvert_upto >= FASTPRIM_lvert_free_end)
	{
		//
		// Need more room!
		//

		#ifdef FASTPRIM_PERFORMANCE
		fprintf(FASTPRIM_handle, "No lverts...\n");
		fflush(FASTPRIM_handle);
		#endif

		FASTPRIM_free_queue_for_call(fc);
	}

	ASSERT(WITHIN(FASTPRIM_lvert_upto, 0, FASTPRIM_lvert_max - 1));
	ASSERT(fc->lvert + fc->lvertcount == FASTPRIM_lvert_upto);

	lv = &FASTPRIM_lvert[fc->lvert + fc->lvertcount];

	lv->x        = x;
	lv->y        = y;
	lv->z        = z;
	lv->tu       = u;
	lv->tv       = v;
	lv->color    = colour;
	lv->specular = specular;

	FASTPRIM_lvert_upto += 1;

	return fc->lvertcount++;
}


void FASTPRIM_ensure_room_for_indices(FASTPRIM_Call *fc)
{
	if (FASTPRIM_index_upto + 8 >= FASTPRIM_index_free_end)
	{
		#ifdef FASTPRIM_PERFORMANCE
		fprintf(FASTPRIM_handle, "No indices...\n");
		fflush(FASTPRIM_handle);
		#endif

		FASTPRIM_free_queue_for_call(fc);
	}
}



SLONG FASTPRIM_draw(
		SLONG prim,
		float x,
		float y,
		float z,
		float matrix[9],
		NIGHT_Colour *lpc)
{
	#ifndef TARGET_DC

	if (!Keys[KB_R])
	{
		return FALSE;
	}

	#endif

	SLONG i;
	SLONG j;
	SLONG k;
	SLONG page;
	SLONG type;
	SLONG index[4];

	float px;
	float py;
	float pz;
	float pu;
	float pv;

	ULONG pcolour;
	ULONG pspecular;

	FASTPRIM_Prim *fp;
	PrimObject    *po;
	PrimFace4     *f4;
	PrimFace3     *f3;
	FASTPRIM_Call *fc;
	PolyPage      *pp;

	LPDIRECT3DTEXTURE2 texture;

	ASSERT(WITHIN(prim, 0, FASTPRIM_MAX_PRIMS - 1));	

	//
	// Is this prim too close to the camera in (x,z)? Ignore y
	// because lamposts and trees have a huge y!
	//

	{
		PrimInfo *pi = get_prim_info(prim);

		extern float AENG_cam_x;
		extern float AENG_cam_z;

		float dx = x - AENG_cam_x;
		float dz = z - AENG_cam_z;

		float dist   = dx*dx + dz*dz;
		float radius = float(MAX(pi->maxx-pi->minx,pi->maxz-pi->minz)) + 16.0f;

		if (dist < radius*radius)
		{
			//
			// Less than 1 mapsquare distant... drawn the old way!
			//

			return FALSE;
		}
	}


	fp = &FASTPRIM_prim[prim];

	#ifndef TARGET_DC
	if (!Keys[KB_R])
	{
		return FALSE;
	}
	#endif

	#ifdef FASTPRIM_PERFORMANCE
	FASTPRIM_num_drawn += 1;
	#endif

	if (fp->flag & FASTPRIM_PRIM_FLAG_CACHED)
	{
		#ifdef FASTPRIM_PERFORMANCE
		FASTPRIM_num_already_cached	+= 1;
		#endif
	}
	else
	{
		//
		// We've come across a new prim.
		//
		
		if (FASTPRIM_call_upto + 32 >= FASTPRIM_MAX_CALLS)
		{
			//
			// Assume max of 32 calls per prim? And pray to god
			// that there is enough room!
			// 

			FASTPRIM_call_upto = 0;
		}

		fp->call_count = 0;
		fp->call_index = FASTPRIM_call_upto;

		//
		// Go through all the faces and create a new call
		// structure for each new texture/type we come across.
		//

		po = &prim_objects[prim];

		for (i = po->StartFace3; i < po->EndFace3; i++)
		{
			ASSERT(WITHIN(i, 0, next_prim_face3 - 1));

			f3 = &prim_faces3[i];

			page   = f3->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= f3->TexturePage;
			page  += FACE_PAGE_OFFSET;

			texture = FASTPRIM_find_texture_from_page(page);

			if (POLY_Page[page].RS.IsAlphaBlendEnabled())
			{
				type = FASTPRIM_CALL_TYPE_INDEXED;
			}
			else
			if (f3->FaceFlags & FACE_FLAG_TINT)
			{
				type = FASTPRIM_CALL_TYPE_COLOURAND;
			}
			else
			{
				type = FASTPRIM_CALL_TYPE_NORMAL;
			}

			FASTPRIM_create_call(fp, texture, type);
		}

		for (i = po->StartFace4; i < po->EndFace4; i++)
		{
			ASSERT(WITHIN(i, 0, next_prim_face4 - 1));

			f4 = &prim_faces4[i];

			page   = f4->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= f4->TexturePage;
			page  += FACE_PAGE_OFFSET;

			texture = FASTPRIM_find_texture_from_page(page);

			if (POLY_Page[page].RS.IsAlphaBlendEnabled())
			{
				type = FASTPRIM_CALL_TYPE_INDEXED;
			}
			else
			if (f4->FaceFlags & FACE_FLAG_TINT)
			{
				type = FASTPRIM_CALL_TYPE_COLOURAND;
			}
			else
			{
				type = FASTPRIM_CALL_TYPE_NORMAL;
			}

			FASTPRIM_create_call(fp, texture, type);
		}

		//
		// Now go through each texture and add the faces
		// that use that texture.
		//

		for (i = 0; i < fp->call_count; i++)
		{
			ASSERT(WITHIN(fp->call_index + i, 0, FASTPRIM_call_upto - 1));

			fc = &FASTPRIM_call[fp->call_index + i];

			fc->flag = 0;

			fc->lvert = FASTPRIM_lvert_upto;
			fc->index = FASTPRIM_index_upto;
			
			fc->lvertcount = 0;
			fc->indexcount = 0;

			for (j = po->StartFace3; j < po->EndFace3; j++)
			{
				ASSERT(WITHIN(j, 0, next_prim_face3 - 1));

				f3 = &prim_faces3[j];

				page   = f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= f3->TexturePage;
				page  += FACE_PAGE_OFFSET;

				texture = FASTPRIM_find_texture_from_page(page);

				if (POLY_Page[page].RS.IsAlphaBlendEnabled())
				{
					type = FASTPRIM_CALL_TYPE_INDEXED;
				}
				else
				if (f3->FaceFlags & FACE_FLAG_TINT)
				{
					type = FASTPRIM_CALL_TYPE_COLOURAND;
				}
				else
				{
					type = FASTPRIM_CALL_TYPE_NORMAL;
				}

				if (texture == fc->texture && type == fc->type)
				{
					//
					// Add the three points of this face.
					//

					pp = &POLY_Page[page];

					for (k = 0; k < 3; k++)
					{
						ASSERT(WITHIN(f3->Points[k], 0, next_prim_point - 1));

						px = AENG_dx_prim_points[f3->Points[k]].X;
						py = AENG_dx_prim_points[f3->Points[k]].Y;
						pz = AENG_dx_prim_points[f3->Points[k]].Z;

						pu = (f3->UV[k][0] & 0x3f) * (1.0F / 32.0F);
						pv = (f3->UV[k][1] 		 ) * (1.0F / 32.0F);

						#ifdef TEX_EMBED

						pu = pu * pp->m_UScale + pp->m_UOffset;
						pv = pv * pp->m_VScale + pp->m_VOffset;

						#endif

						if (lpc)
						{
							NIGHT_get_d3d_colour(
								lpc[f3->Points[k] - po->StartPoint],
							   &pcolour,
							   &pspecular);
						}
						else
						{
							//
							// Light this point badly...
							//

							pcolour   = NIGHT_amb_d3d_colour;
							pspecular = NIGHT_amb_d3d_specular;
						}

						if (POLY_page_flag[page] & POLY_PAGE_FLAG_SELF_ILLUM)
						{
							pcolour   = 0xffffff;
							pspecular = 0xff000000;

							fc->flag |= FASTPRIM_CALL_FLAG_SELF_ILLUM;
						}

						index[k] = FASTPRIM_add_point_to_call(fc, px,py,pz, pu,pv, pcolour, pspecular);

						if (type == FASTPRIM_CALL_TYPE_COLOURAND)
						{
							//
							// Put the 'y' component of the normal into the
							// top byte of dwReserved- we cheekily use just this
							// value to do the lighting...
							//

							ASSERT(WITHIN(fc->lvert + index[k], 0, FASTPRIM_lvert_max - 1));

							FASTPRIM_lvert[fc->lvert + index[k]].dwReserved = prim_normal[f3->Points[k]].Y << 24;
						}
						else
						{
							//
							// Put the point index into the top UWORD of the dwReserved field.
							//

							ASSERT(WITHIN(fc->lvert + index[k], 0, FASTPRIM_lvert_max - 1));

							FASTPRIM_lvert[fc->lvert + index[k]].dwReserved = (f3->Points[k] - po->StartPoint) << 16;
						}
					}

					//
					// Now add the face.
					//

					FASTPRIM_ensure_room_for_indices(fc);

					ASSERT(WITHIN(FASTPRIM_index_upto + 3, 0, FASTPRIM_index_max));
					ASSERT(fc->index + fc->indexcount == FASTPRIM_index_upto);

					if (type != FASTPRIM_CALL_TYPE_INDEXED)
					{
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[0];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[1];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[2];
						FASTPRIM_index[FASTPRIM_index_upto++] = -1;

						fc->indexcount += 4;
					}
					else
					{
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[0];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[1];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[2];

						fc->indexcount += 3;
					}
				}
			}

			for (j = po->StartFace4; j < po->EndFace4; j++)
			{
				ASSERT(WITHIN(j, 0, next_prim_face4 - 1));

				f4 = &prim_faces4[j];

				page   = f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= f4->TexturePage;
				page  += FACE_PAGE_OFFSET;

				texture = FASTPRIM_find_texture_from_page(page);

				if (POLY_Page[page].RS.IsAlphaBlendEnabled())
				{
					type = FASTPRIM_CALL_TYPE_INDEXED;
				}
				else
				if (f4->FaceFlags & FACE_FLAG_TINT)
				{
					type = FASTPRIM_CALL_TYPE_COLOURAND;
				}
				else
				{
					type = FASTPRIM_CALL_TYPE_NORMAL;
				}

				if (texture == fc->texture && type == fc->type)
				{
					//
					// Add the four points of this face.
					//

					pp = &POLY_Page[page];

					for (k = 0; k < 4; k++)
					{
						ASSERT(WITHIN(f4->Points[k], 0, next_prim_point - 1));

						px = AENG_dx_prim_points[f4->Points[k]].X;
						py = AENG_dx_prim_points[f4->Points[k]].Y;
						pz = AENG_dx_prim_points[f4->Points[k]].Z;

						pu = (f4->UV[k][0] & 0x3f) * (1.0F / 32.0F);
						pv = (f4->UV[k][1] 		 ) * (1.0F / 32.0F);

						#ifdef TEX_EMBED

						pu = pu * pp->m_UScale + pp->m_UOffset;
						pv = pv * pp->m_VScale + pp->m_VOffset;

						#endif

						if (lpc)
						{
							NIGHT_get_d3d_colour(
								lpc[f4->Points[k] - po->StartPoint],
							   &pcolour,
							   &pspecular);
						}
						else
						{
							//
							// Light this point badly...
							//

							pcolour   = NIGHT_amb_d3d_colour;
							pspecular = NIGHT_amb_d3d_specular;
						}

						if (POLY_page_flag[page] & POLY_PAGE_FLAG_SELF_ILLUM)
						{
							pcolour   = 0xffffff;
							pspecular = 0xff000000;
						}

						index[k] = FASTPRIM_add_point_to_call(fc, px,py,pz, pu,pv, pcolour, pspecular);

						if (type == FASTPRIM_CALL_TYPE_COLOURAND)
						{
							//
							// Put the 'y' component of the normal into the
							// top byte of dwReserved- we cheekily use just this
							// value to do the lighting...
							//

							ASSERT(WITHIN(fc->lvert + index[k], 0, FASTPRIM_lvert_max - 1));

							FASTPRIM_lvert[fc->lvert + index[k]].dwReserved = prim_normal[f4->Points[k]].Y << 24;
						}
						else
						{
							//
							// Put the point index into the top UWORD of the dwReserved field.
							//

							ASSERT(WITHIN(fc->lvert + index[k], 0, FASTPRIM_lvert_max - 1));

							FASTPRIM_lvert[fc->lvert + index[k]].dwReserved = (f4->Points[k] - po->StartPoint) << 16;
						}
					}

					//
					// Now add the face.
					//

					FASTPRIM_ensure_room_for_indices(fc);

					ASSERT(WITHIN(FASTPRIM_index_upto + 6, 0, FASTPRIM_index_max));
					ASSERT(fc->index + fc->indexcount == FASTPRIM_index_upto);

					if (type != FASTPRIM_CALL_TYPE_INDEXED)
					{
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[0];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[1];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[2];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[3];
						FASTPRIM_index[FASTPRIM_index_upto++] = -1;

						fc->indexcount += 5;
					}
					else
					{
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[0];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[1];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[2];

						FASTPRIM_index[FASTPRIM_index_upto++] =  index[3];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[2];
						FASTPRIM_index[FASTPRIM_index_upto++] =  index[1];

						fc->indexcount += 6;
					}
				}
			}
		}

		if (po->flag & PRIM_FLAG_ENVMAPPED)
		{
			//
			// Create another call for the environment mapped faces.
			//

			LPDIRECT3DTEXTURE2 envtexture = FASTPRIM_find_texture_from_page(POLY_PAGE_ENVMAP);

			FASTPRIM_create_call(fp, envtexture, FASTPRIM_CALL_TYPE_ENVMAP);

			//
			// Find the newly-created call structure- should be the last one in the array.
			//

			ASSERT(WITHIN(fp->call_index + fp->call_count - 1, 0, FASTPRIM_call_upto - 1));

			fc = &FASTPRIM_call[fp->call_index + fp->call_count - 1];

			ASSERT(fc->texture == envtexture && fc->type == FASTPRIM_CALL_TYPE_ENVMAP);

			fc->flag = 0;

			fc->lvert = FASTPRIM_lvert_upto;
			fc->index = FASTPRIM_index_upto;
			
			fc->lvertcount = 0;
			fc->indexcount = 0;

			//
			// Add all the environment mapped faces.
			//

			for (i = po->StartFace3; i < po->EndFace3; i++)
			{
				ASSERT(WITHIN(i, 0, next_prim_face3 - 1));

				f3 = &prim_faces3[i];

				if (f3->FaceFlags & FACE_FLAG_ENVMAP)
				{
					for (j = 0; j < 3; j++)
					{
						ASSERT(WITHIN(f3->Points[j], 0, next_prim_point - 1));

						px = AENG_dx_prim_points[f3->Points[j]].X;
						py = AENG_dx_prim_points[f3->Points[j]].Y;
						pz = AENG_dx_prim_points[f3->Points[j]].Z;

						pu = 0.0F;
						pv = 0.0F;

						pcolour   = 0xff888888;
						pspecular = 0x00000000;

						index[j] = FASTPRIM_add_point_to_call(fc, px,py,pz, pu,pv, pcolour, pspecular);

						//
						// Put the point index into the top UWORD of 'dwReserved' so we know
						// which normal to use.
						//

						ASSERT(WITHIN(fc->lvert + index[j], 0, FASTPRIM_lvert_max - 1));

						FASTPRIM_lvert[fc->lvert + index[j]].dwReserved = f3->Points[j] << 16;
					}

					//
					// Now add the face.
					//

					FASTPRIM_ensure_room_for_indices(fc);

					ASSERT(WITHIN(FASTPRIM_index_upto + 4, 0, FASTPRIM_index_max));
					ASSERT(fc->index + fc->indexcount == FASTPRIM_index_upto);

					FASTPRIM_index[FASTPRIM_index_upto++] = index[0];
					FASTPRIM_index[FASTPRIM_index_upto++] = index[1];
					FASTPRIM_index[FASTPRIM_index_upto++] = index[2];

					fc->indexcount += 3;
					
				}
			}

			for (i = po->StartFace4; i < po->EndFace4; i++)
			{
				ASSERT(WITHIN(i, 0, next_prim_face4 - 1));

				f4 = &prim_faces4[i];

				if (f4->FaceFlags & FACE_FLAG_ENVMAP)
				{
					for (j = 0; j < 4; j++)
					{
						ASSERT(WITHIN(f4->Points[j], 0, next_prim_point - 1));

						px = AENG_dx_prim_points[f4->Points[j]].X;
						py = AENG_dx_prim_points[f4->Points[j]].Y;
						pz = AENG_dx_prim_points[f4->Points[j]].Z;

						pu = 0.0F;
						pv = 0.0F;

						pcolour   = 0xff888888;
						pspecular = 0x00000000;

						index[j] = FASTPRIM_add_point_to_call(fc, px,py,pz, pu,pv, pcolour, pspecular);

						//
						// Put the point index into the top UWORD of 'dwReserved' so we know
						// which normal to use.
						//

						ASSERT(WITHIN(fc->lvert + index[j], 0, FASTPRIM_lvert_max - 1));

						FASTPRIM_lvert[fc->lvert + index[j]].dwReserved = f4->Points[j] << 16;
					}

					//
					// Now add the face.
					//

					FASTPRIM_ensure_room_for_indices(fc);

					ASSERT(WITHIN(FASTPRIM_index_upto + 5, 0, FASTPRIM_index_max));
					ASSERT(fc->index + fc->indexcount == FASTPRIM_index_upto);

					FASTPRIM_index[FASTPRIM_index_upto++] = index[0];
					FASTPRIM_index[FASTPRIM_index_upto++] = index[1];
					FASTPRIM_index[FASTPRIM_index_upto++] = index[2];

					FASTPRIM_index[FASTPRIM_index_upto++] = index[3];
					FASTPRIM_index[FASTPRIM_index_upto++] = index[2];
					FASTPRIM_index[FASTPRIM_index_upto++] = index[1];

					fc->indexcount += 6;
				}
			}
		}

		//
		// We've cached it now!
		//

		fp->flag |= FASTPRIM_PRIM_FLAG_CACHED;

		FASTPRIM_queue[FASTPRIM_queue_start++ & (FASTPRIM_MAX_QUEUE - 1)] = prim;

		#ifdef FASTPRIM_PERFORMANCE
		fprintf(FASTPRIM_handle, "Gameturn %5d cache prim %3d\n", GAME_TURN, prim);
		fflush(FASTPRIM_handle);
		FASTPRIM_num_cached += 1;
		#endif
	}

	//
	// Are we strinking this prim?
	//

	extern UBYTE kludge_shrink;

	if (kludge_shrink)
	{
		float fstretch;

		fstretch = (prim == PRIM_OBJ_ITEM_AMMO_SHOTGUN) ? 0.15F : 0.7F;

		matrix[0] *= fstretch;
		matrix[1] *= fstretch;
		matrix[2] *= fstretch;

		matrix[3] *= fstretch;
		matrix[4] *= fstretch;
		matrix[5] *= fstretch;

		matrix[6] *= fstretch;
		matrix[7] *= fstretch;
		matrix[8] *= fstretch;
	}

	//
	// Are we strinking this prim?
	//

	extern UBYTE kludge_shrink;

	if (kludge_shrink)
	{
		float fstretch;

		fstretch = (prim == PRIM_OBJ_ITEM_AMMO_SHOTGUN) ? 0.15F : 0.7F;

		matrix[0] *= fstretch;
		matrix[1] *= fstretch;
		matrix[2] *= fstretch;

		matrix[3] *= fstretch;
		matrix[4] *= fstretch;
		matrix[5] *= fstretch;

		matrix[6] *= fstretch;
		matrix[7] *= fstretch;
		matrix[8] *= fstretch;
	}

	//
	// Draw the cached version. Set up the matrix for this object's rotation.
	//

	POLY_set_local_rotation(
		x,y,z,
		matrix);

	GenerateMMMatrixFromStandardD3DOnes(
		FASTPRIM_matrix,
	    &g_matProjection,
		&g_matWorld,
	    &g_viewData);
	
	for (i = 0; i < fp->call_count; i++)
	{
		ASSERT(WITHIN(fp->call_index + i, 0, FASTPRIM_MAX_CALLS - 1));

		fc = &FASTPRIM_call[fp->call_index + i];

		if (fc->type == FASTPRIM_CALL_TYPE_ENVMAP)
		{
			//
			// Work out the environment mapping. Calculate the rotation
			// matrix combined with the camera.
			//

			float nx;
			float ny;
			float nz;

			float comb[9];
			float cam_matrix[9];

			extern float AENG_cam_yaw;
			extern float AENG_cam_pitch;
			extern float AENG_cam_roll;

			D3DLVERTEX *lv;

			MATRIX_calc(cam_matrix, AENG_cam_yaw, AENG_cam_pitch, AENG_cam_roll);
			MATRIX_3x3mul(comb, cam_matrix, matrix);

			for (j = 0; j < fc->lvertcount; j++)
			{
				ASSERT(WITHIN(fc->lvert + j, 0, FASTPRIM_lvert_max - 1));

				lv = &FASTPRIM_lvert[fc->lvert + j];

				ASSERT(WITHIN(lv->dwReserved >> 16, 0, (unsigned)( next_prim_point - 1 )));

				nx = prim_normal[lv->dwReserved >> 16].X * (2.0F / 256.0F);
				ny = prim_normal[lv->dwReserved >> 16].Y * (2.0F / 256.0F);
				nz = prim_normal[lv->dwReserved >> 16].Z * (2.0F / 256.0F);

				MATRIX_MUL(
					comb,
					nx,
					ny,
					nz);

				lv->tu = (nx * 0.5F) + 0.5F;
				lv->tv = (ny * 0.5F) + 0.5F;
			}
		}
		else
		if (fc->type == FASTPRIM_CALL_TYPE_COLOURAND)
		{
			ULONG default_colour;
			ULONG default_specular;

			NIGHT_get_d3d_colour(
				NIGHT_get_light_at(x,y,z),
			   &default_colour,
			   &default_specular);

			extern ULONG MESH_colour_and;

			default_colour &= MESH_colour_and;

			//
			// Relight then AND out the colours...
			//

			D3DLVERTEX *lv;

			for (j = 0; j < fc->lvertcount; j++)
			{
				ASSERT(WITHIN(fc->lvert + j, 0, FASTPRIM_lvert_max - 1));

				lv = &FASTPRIM_lvert[fc->lvert + j];

				lv->color    = default_colour;
				lv->specular = default_specular;
			}
		}
		else
		if (fc->type == FASTPRIM_CALL_TYPE_NORMAL)
		{
			if (lpc)
			{
				//
				// Relight from the cached lighting.
				//
				
				D3DLVERTEX *lv;

				for (j = 0; j < fc->lvertcount; j++)
				{
					ASSERT(WITHIN(fc->lvert + j, 0, FASTPRIM_lvert_max - 1));

					lv = &FASTPRIM_lvert[fc->lvert + j];

					NIGHT_get_d3d_colour(
						lpc[lv->dwReserved >> 16],
					   &lv->color,
					   &lv->specular);
				}
			}
			else
			{
				//
				// Relight using local light...
				//

				ULONG default_colour;
				ULONG default_specular;

				NIGHT_get_d3d_colour(
					NIGHT_get_light_at(x,y,z),
				   &default_colour,
				   &default_specular);

				
				D3DLVERTEX *lv;

				for (j = 0; j < fc->lvertcount; j++)
				{
					ASSERT(WITHIN(fc->lvert + j, 0, FASTPRIM_lvert_max - 1));

					lv = &FASTPRIM_lvert[fc->lvert + j];

					lv->color    = default_colour;
					lv->specular = default_specular;
				}
			}
		}

		if (fc->type == FASTPRIM_CALL_TYPE_INDEXED ||
			fc->type == FASTPRIM_CALL_TYPE_ENVMAP)
		{
			//
			// Standard D3D DrawIndexedPrimitive() call...
			//

			if (fc->type == FASTPRIM_CALL_TYPE_INDEXED)
			{
				//
				// Setup alphablend renderstates...
				//

				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_SRCALPHA   );
				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_INVSRCALPHA);
				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE                );
			}
			else
			{
				//
				// Setup additive renderstates...
				//

				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_ONE);
				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_ONE); 
				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE        );
			}

			the_display.lp_D3D_Device->SetTexture(0, fc->texture);

			the_display.lp_D3D_Device->DrawIndexedPrimitive(
											D3DPT_TRIANGLELIST,
											D3DFVF_LVERTEX,
											FASTPRIM_lvert + fc->lvert,
											fc->lvertcount,
											FASTPRIM_index + fc->index,
											fc->indexcount,
											0);

			the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		}
		else
		{
			//
			// Tom's DrawIndMM() call...
			//

			D3DMULTIMATRIX d3dmm =
			{
				FASTPRIM_lvert + fc->lvert,
				FASTPRIM_matrix,
				NULL,
				NULL
			};

			the_display.lp_D3D_Device->SetTexture(0, fc->texture);

			if (fc->flag & FASTPRIM_CALL_FLAG_SELF_ILLUM)
			{
				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_DECAL);
			}

			//TRACE ( "S3" );
			DrawIndPrimMM(
				the_display.lp_D3D_Device,
				D3DFVF_LVERTEX,
			   &d3dmm,
				fc->lvertcount,
				FASTPRIM_index + fc->index,
				fc->indexcount);
			//TRACE ( "F3" );

			if (fc->flag & FASTPRIM_CALL_FLAG_SELF_ILLUM)
			{
				the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
			}
		}
	}

	return TRUE;
}


void FASTPRIM_fini()
{

#ifdef DEBUG
	ASSERT ( FASTPRIM_lvert_buffer != NULL );
	ASSERT ( FASTPRIM_index != NULL );
	ASSERT ( FASTPRIM_lvert_buffer == (void *)pvJustChecking1 );
	ASSERT ( FASTPRIM_index == (void *)pvJustChecking2 );


	char *pcTemp = (char *)( (SLONG)FASTPRIM_lvert + sizeof(D3DLVERTEX) * FASTPRIM_lvert_max );
	ASSERT ( 0 == strcmp ( pcTemp, "ThisIsAMagicString901234567890" ) );

#endif

	TRACE ( "FASTPRIM_fini " );
	MemFree(FASTPRIM_lvert_buffer);
	TRACE ( "1 " );
	MemFree(FASTPRIM_index);
	TRACE ( "2\n" );

	FASTPRIM_lvert_buffer = NULL;
	FASTPRIM_index = NULL;

	#ifdef FASTPRIM_PERFORMANCE

	float av_freed_per_turn    = float(FASTPRIM_num_freed )         / float(GAME_TURN);
	float av_cached_per_turn   = float(FASTPRIM_num_cached)         / float(GAME_TURN);
	float av_in_cache_per_turn = float(FASTPRIM_num_already_cached) / float(GAME_TURN);
	float av_drawn_per_turn    = float(FASTPRIM_num_drawn)          / float(GAME_TURN);

	fprintf(FASTPRIM_handle, "Average freed    per gameturn = %f\n", av_freed_per_turn   );
	fprintf(FASTPRIM_handle, "Average cached   per gameturn = %f\n", av_cached_per_turn  );
	fprintf(FASTPRIM_handle, "Average in cache per gameturn = %f\n", av_in_cache_per_turn);
	fprintf(FASTPRIM_handle, "Average drawn    per gameturn = %f\n", av_drawn_per_turn   );

	fflush(FASTPRIM_handle);

	#endif
}
