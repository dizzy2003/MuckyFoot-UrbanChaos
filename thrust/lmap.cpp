//
// Creates light maps.
//

#include "always.h"
#include "lmap.h"
#include "matrix.h"
#include "os.h"
#include "slap.h"



//
// The lightmap textures.
//

typedef struct
{
	SLONG  res;
	UBYTE *bitmap;

} LMAP_Tex;

#define LMAP_MAX_TEXES 5

LMAP_Tex LMAP_tex[LMAP_MAX_TEXES] =
{
	{16,  NULL},
	{32,  NULL},
	{64,  NULL},
	{128, NULL},
	{256, NULL},
};



//
// Our lightmap structure.
//

typedef struct lmap_lmap
{
	SLONG  res;
	UBYTE *bitmap;

} LMAP_Lmap;




LMAP_Lmap *LMAP_create(SLONG resolution)
{
	SLONG i;
	SLONG x;
	SLONG y;

	float dx;
	float dy;
	float dist;
	float frac;

	SLONG value;

	UBYTE *pixel;

	//
	// Have we created the texture for this resolution?
	//

	for (i = 0; i < LMAP_MAX_TEXES; i++)
	{
		if (LMAP_tex[i].res == resolution)
		{
			if (LMAP_tex[i].bitmap != NULL)
			{
				//
				// We've already generated this texture.
				//

				goto found_texture;
			}
			else
			{
				//
				// We must generate this texture.
				//

				LMAP_tex[i].bitmap = (UBYTE *) malloc(sizeof(UBYTE) * resolution * resolution);	// It'll always work!

				pixel = LMAP_tex[i].bitmap;

				for (y = 0; y < resolution; y++)
				for (x = 0; x < resolution; x++)
				{
					if (x == 0 || x == resolution - 1 ||
						y == 0 || y == resolution - 1)
					{
						value = 0;
					}
					else
					{
						dx = fabs(float((resolution >> 1) - x));
						dy = fabs(float((resolution >> 1) - y));

						dist = sqrt(dx*dx + dy*dy);

						frac = dist / float(resolution >> 1);
						frac = frac * frac * frac;
						
						value   = 255 - ftol(256.0F * frac);

						SATURATE(value, 0, 255);

					//	value = value * 64 >> 8;
					}

				   *pixel++ = value;
				}

				goto found_texture;
			}
		}
	}

	//
	// This is a funny resolution!
	//

	ASSERT(0);

	return NULL;

  found_texture:;

	//
	// Create a new lightmap.
	//

	LMAP_Lmap *ans = (LMAP_Lmap *) malloc(sizeof(LMAP_Lmap));

	ans->res    = resolution;
	ans->bitmap = (UBYTE *) malloc(sizeof(UBYTE) * resolution * resolution);

	return ans;
}



void LMAP_init(LMAP_Lmap *lmap)
{
	if (lmap == NULL)
	{
		return;
	}

	//
	// Zero out the bitmap.
	// 

	memset(lmap->bitmap, 0, sizeof(UBYTE) * lmap->res * lmap->res);

	//
	// Initialise the shadowmapper.
	//

	SLAP_init(
		lmap->bitmap,
		lmap->res);
}


void LMAP_add_shadow(
		LMAP_Lmap *lmap,
		IMP_Mesh  *im,
		float      light_x,
		float      light_y,
		float      light_z,
		float      light_matrix[9],
		float      light_lens)
{
	SLONG i;

	float x;
	float y;
	float z;
	float X;
	float Y;
	float Z;

	float au;
	float av;
	float bu;
	float bv;
	float cprod;

	UBYTE f1;
	UBYTE f2;

	IMP_Vert *iv;
	IMP_Face *ic;
	IMP_Edge *ie;

	IMP_Vert *iv1;
	IMP_Vert *iv2;
	IMP_Vert *iv3;

	//
	// Rotate all the points of the mesh into light-source space.
	//

	for (i = 0; i < im->num_verts; i++)
	{
		iv = &im->vert[i];

		//
		// Rotate the point into light-source space.
		//

		x = iv->x - light_x;
		y = iv->y - light_y;
		z = iv->z - light_z;

		MATRIX_MUL(
			light_matrix,
			x,
			y,
			z);

		if (z > 0.0F)	// No zclip plane!
		{
			//
			// Perspective transform.
			//

			Z = light_lens / z;

			X = 0.5F + x * Z;
			Y = 0.5F + y * Z;

			iv->lu = X;
			iv->lv = Y;
		}
		else
		{
			//
			// This point is behind the light!
			//

			ASSERT(0);

			iv->lu = 0.0F;
			iv->lv = 0.0F;
		}
	}

	//
	// Work out which faces are visible from the lightsource (i.e. not
	// backface culled).
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];

		ASSERT(WITHIN(ic->v[0], 0, im->num_verts - 1));
		ASSERT(WITHIN(ic->v[1], 0, im->num_verts - 1));
		ASSERT(WITHIN(ic->v[2], 0, im->num_verts - 1));

		iv1 = &im->vert[ic->v[0]];
		iv2 = &im->vert[ic->v[1]];
		iv3 = &im->vert[ic->v[2]];

		au = iv2->lu - iv1->lu;
		av = iv2->lv - iv1->lv;

		bu = iv3->lu - iv1->lu;
		bv = iv3->lv - iv1->lv;

		cprod = au*bv - av*bu;

		if (cprod <= 0.0F)
		{
			ic->flag |=  IMP_FACE_FLAG_BACKFACE;
		}
		else
		{
			ic->flag &= ~IMP_FACE_FLAG_BACKFACE;
		}
	}

	//
	// Work out the sillouhette edges and add them to the shadow mapper.
	//

	for (i = 0; i < im->num_edges; i++)
	{
		ie = &im->edge[i];

		ASSERT(WITHIN(ie->f1, 0, im->num_faces - 1));
		ASSERT(WITHIN(ie->f2, 0, im->num_faces - 1) || ie->f2 == 0xffff);

		f1 = im->face[ie->f1].flag;

		if (ie->f2 == 0xffff)
		{
			f2 = 0;
		}
		else
		{
			f2 = im->face[ie->f2].flag;
		}

		if ((f1 ^ f2) & IMP_FACE_FLAG_BACKFACE)
		{
			//
			// This is a silhoutte edge.
			//

			ASSERT(WITHIN(ie->v1, 0, im->num_verts - 1));
			ASSERT(WITHIN(ie->v2, 0, im->num_verts - 1));

			iv1 = &im->vert[ie->v1];
			iv2 = &im->vert[ie->v2];

			if (im->face[ie->f1].flag & IMP_FACE_FLAG_BACKFACE)
			{
				SLAP_add_edge(
					ftol(iv1->lu * float(lmap->res << 8)),
					ftol(iv1->lv * float(lmap->res << 8)),
					ftol(iv2->lu * float(lmap->res << 8)),
					ftol(iv2->lv * float(lmap->res << 8)));
			}
			else
			{
				SLAP_add_edge(
					ftol(iv2->lu * float(lmap->res << 8)),
					ftol(iv2->lv * float(lmap->res << 8)),
					ftol(iv1->lu * float(lmap->res << 8)),
					ftol(iv1->lv * float(lmap->res << 8)));
			}
		}
	}
}


void LMAP_render(LMAP_Lmap *lmap, OS_Texture *ot)
{
	UBYTE *shadow;
	UBYTE *lmaptex;

	//
	// Render the shadow map.
	//

	SLAP_render();

	//
	// Find the lightmap texture.
	//

	SLONG i;

	for (i = 0; i < LMAP_MAX_TEXES; i++)
	{
		if (LMAP_tex[i].res == lmap->res)
		{
			if (LMAP_tex[i].bitmap != NULL)
			{
				//
				// We've generated this texture.
				//

				lmaptex = LMAP_tex[i].bitmap;

				goto found_texture;
			}
		}
	}

	ASSERT(0);

	return;

  found_texture:;

	//
	// Create the texture.
	//

	OS_texture_lock(ot);

	SLONG x;
	SLONG y;
	
	SLONG pixel;

	shadow = lmap->bitmap;

	for (y = 0; y < lmap->res; y++)
	for (x = 0; x < lmap->res; x++)
	{
		pixel = (256 - *shadow) * *lmaptex >> 8;

		if (OS_bitmap_ubyte_screen)
		{
			OS_BITMAP_UBYTE_PLOT(x,y, pixel);
		}
		else
		{
			OS_BITMAP_UWORD_PLOT(x,y, pixel,pixel,pixel);
		}

		shadow  += 1;
		lmaptex += 1;
	}

	OS_texture_unlock(ot);
}



