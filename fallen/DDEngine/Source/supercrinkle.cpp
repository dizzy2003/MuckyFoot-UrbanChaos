//
// SUPERCRINKLES!
//

#include "game.h"
#include "ddlib.h"
#include "poly.h"
#include "polypoint.h"
#include "polypage.h"
#include "supercrinkle.h"
#include <math.h>




#if SUPERCRINKLES_ENABLED


//
// The D3DLVERTEX array.
//

#define SUPERCRINKLE_MAX_LVERTS 16384

//D3DLVERTEX  SUPERCRINKLE_lvert_buffer[SUPERCRINKLE_MAX_LVERTS + 1];
D3DLVERTEX *SUPERCRINKLE_lvert_buffer = NULL;
D3DLVERTEX *SUPERCRINKLE_lvert;
SLONG       SUPERCRINKLE_lvert_upto;

//
// Colour interpolations for each lvert.
//

typedef struct
{
	UBYTE i[4];

} SUPERCRINKLE_Colour;

#define SUPERCRINKLE_MAX_COLOURS SUPERCRINKLE_MAX_LVERTS

SUPERCRINKLE_Colour SUPERCRINKLE_colour[SUPERCRINKLE_MAX_COLOURS];
SLONG               SUPERCRINKLE_colour_upto;


//
// The indices.
//

#define SUPERCRINKLE_MAX_INDICES (SUPERCRINKLE_MAX_LVERTS * 2)

UWORD  SUPERCRINKLE_index_buffer[SUPERCRINKLE_MAX_INDICES + 16];
UWORD *SUPERCRINKLE_index;
SLONG  SUPERCRINKLE_index_upto;



//
// Whether each page is crinkled or not.
//

UBYTE SUPERCRINKLE_is_crinkled[512];



//
// The size of the lighting texure for a crinkle.
//

#define SUPERCRINKLE_TEXTURE_SIZE 128



//
// A supercrinkle!
//

typedef struct
{
	UWORD lvert;			// Index into the D3DLVERTEX array.
	UWORD num_lverts;		// Number of lverts used by this crinkle.
	
	UWORD index;			// DrawIndexedPrimitive indices into the vert array
	UWORD num_indices;		// for this crinkle.

	ULONG hash;				// The colours in this crinkle.

} SUPERCRINKLE_Crinkle;

#define SUPERCRINKLE_MAX_CRINKLES 512

SUPERCRINKLE_Crinkle SUPERCRINKLE_crinkle[SUPERCRINKLE_MAX_CRINKLES];



//
// A SUPERCRINKLE_Colour
//

typedef struct
{
	ULONG colour;
	ULONG specular;

} SUPERCRINKLE_Precalc;




//
// Cached lighting.
//

typedef struct supercrinkle_cache
{
	ULONG hash;

	struct supercrinkle_cache *next;

	SUPERCRINKLE_Precalc precalc[2];	// Not really 2!

} SUPERCRINKLE_Cache;



//
// Three arrays, with different numbers of points... This is about 256k
//

#define SUPERCRINKLE_MAX_CACHE64  128
#define SUPERCRINKLE_MAX_CACHE128 96
#define SUPERCRINKLE_MAX_CACHE384 32

union
{
	UBYTE              padding[sizeof(SUPERCRINKLE_Cache) + sizeof(SUPERCRINKLE_Precalc) * 64];
	SUPERCRINKLE_Cache cache;

} SUPERCRINKLE_cache64[SUPERCRINKLE_MAX_CACHE64];

union
{
	UBYTE              padding[sizeof(SUPERCRINKLE_Cache) + sizeof(SUPERCRINKLE_Precalc) * 128];
	SUPERCRINKLE_Cache cache;

} SUPERCRINKLE_cache128[SUPERCRINKLE_MAX_CACHE128];

union
{
	UBYTE              padding[sizeof(SUPERCRINKLE_Cache) + sizeof(SUPERCRINKLE_Precalc) * 384];
	SUPERCRINKLE_Cache cache;

} SUPERCRINKLE_cache384[SUPERCRINKLE_MAX_CACHE384];

//
// A free list for each size.
//

SUPERCRINKLE_Cache *SUPERCRINKLE_free64;
SUPERCRINKLE_Cache *SUPERCRINKLE_free128;
SUPERCRINKLE_Cache *SUPERCRINKLE_free384;


//
// A hash table for each size.
//

#define SUPERCRINKLE_HASH_SIZE 256	// Power of 2 please.

SUPERCRINKLE_Cache *SUPERCRINKLE_hash_table64 [SUPERCRINKLE_HASH_SIZE];
SUPERCRINKLE_Cache *SUPERCRINKLE_hash_table128[SUPERCRINKLE_HASH_SIZE];
SUPERCRINKLE_Cache *SUPERCRINKLE_hash_table384[SUPERCRINKLE_HASH_SIZE];




//
// The hash function.
//

ULONG SUPERCRINKLE_hash_function(SLONG crinkle, ULONG colour[4], ULONG specular[4])
{
	ULONG ans;
	
	ans  = crinkle;

	ans ^= _rotr(colour[0],  2);
	ans ^= _rotr(colour[1],  7);
	ans ^= _rotr(colour[2], 12);
	ans ^= _rotr(colour[3], 17);

	ans ^= _rotl(specular[0],  3);
	ans ^= _rotl(specular[1],  8);
	ans ^= _rotl(specular[2], 13);
	ans ^= _rotl(specular[3], 18);

	return ans;
}





//
// Loads the given SUPERCRINKLE from the .SEX file.
//

void SUPERCRINKLE_load(SLONG crinkle, CBYTE *fname)
{
	//
	// Temporary buffer for holding points and faces.
	//

	typedef struct
	{
		float x;
		float y;
		float z;
		
		float nx;
		float ny;
		float nz;

		UBYTE i[4];		// The colour interpolations for this point.

		UBYTE light;	// How much darker/brigher this point should be than normal... 128 => Same as before.
		UBYTE padding;
		UWORD duplicate;

	} SUPERCRINKLE_Point;

	#define SUPERCRINKLE_MAX_POINTS 1536

	SUPERCRINKLE_Point point[SUPERCRINKLE_MAX_POINTS];
	SLONG              point_upto;

	typedef struct
	{
		UWORD p[3];

		float nx;
		float ny;
		float nz;
		
	} SUPERCRINKLE_Face;

	#define SUPERCRINKLE_MAX_FACES 512

	SUPERCRINKLE_Face face[SUPERCRINKLE_MAX_FACES];
	SLONG             face_upto;


	SLONG i;
	SLONG j;

	SLONG o;
	SLONG f;

	float x;
	float y;
	float z;

	float ax;
	float ay;
	float az;

	float bx;
	float by;
	float bz;

	float dx;
	float dy;
	float dz;

	float nx;
	float ny;
	float nz;

	float dprod;
	float length;
	float overlength;

	SLONG a;
	SLONG b;
	SLONG c;

	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG match;
	SLONG index;

	CBYTE line[512];

	SUPERCRINKLE_Crinkle *sc;
	SUPERCRINKLE_Face    *sf;
	SUPERCRINKLE_Point   *sp;
	D3DLVERTEX           *tl;
	PolyPage             *pp;

	FILE *handle;

	//
	// Clear the temporary buffers.
	//

	memset(point, 0, sizeof(point));
	memset(face,  0, sizeof(face ));

	point_upto = 0;
	face_upto  = 0;

	//
	// Open the file.
	//

	handle = MF_Fopen(fname, "rb");

	if (!handle)
	{
		TRACE("Could not open crinkle file \"%s\"\n", fname);

		return;
	}

	ASSERT(WITHIN(crinkle, 0, SUPERCRINKLE_MAX_CRINKLES - 1));

	//
	// The new crinkle.
	//

	sc = &SUPERCRINKLE_crinkle[crinkle];

	sc->num_lverts  = 0;
	sc->num_indices = 0;
	sc->lvert       = SUPERCRINKLE_lvert_upto;
	sc->index       = SUPERCRINKLE_index_upto;

	//
	// Assume the crinkle index is the same as the POLY_Page index.
	//

	pp = &POLY_Page[crinkle];

	//
	// Load the asc. Put the points into the buffer and the faces
	// into the CRINKLE_face array.
	//

	while(fgets(line, 512, handle))
	{
		match = sscanf(line, "Vertex: (%f, %f, %f)", &x, &y, &z);

		if (match == 3)
		{
			ASSERT(WITHIN(point_upto, 0, SUPERCRINKLE_MAX_POINTS - 1));

			//
			// Found a point. Add it to the buffer.
			//

			SWAP_FL(y, z);
			x = -x;

			point[point_upto].x = -x;
			point[point_upto].y = -y;
			point[point_upto].z =  z;

			point_upto += 1;

			continue;
		}

		match = sscanf(line, "Face: Material %d xyz (%d, %d, %d)", &f, &a, &b, &c);

		if (match == 4)
		{
			ASSERT(WITHIN(face_upto, 0, SUPERCRINKLE_MAX_FACES - 1));

			face[face_upto].p[0] = a;
			face[face_upto].p[1] = b;
			face[face_upto].p[2] = c;

			face_upto += 1;

			continue;
		}
	}

	//
	// Finished with the file.
	//

	MF_Fclose(handle);

	//
	// What is the bounding rectangle?
	//

	float minx = +float(INFINITY);
	float miny = +float(INFINITY);
	float minz = +float(INFINITY);

	float maxx = -float(INFINITY);
	float maxy = -float(INFINITY);
	float maxz = -float(INFINITY);

	for (i = 0; i < point_upto; i++)
	{
		sp = &point[i];

		if (sp->x < minx) {minx = sp->x;}
		if (sp->y < miny) {miny = sp->y;}
		if (sp->z < minz) {minz = sp->z;}

		if (sp->x > maxx) {maxx = sp->x;}
		if (sp->y > maxy) {maxy = sp->y;}
		if (sp->z > maxz) {maxz = sp->z;}
	}

	float sizex = maxx - minx;
	float sizey = maxy - miny;
	float sizez = maxz - minz;

	if (sizey < sizex && sizey < sizez)
	{
		//
		// This is a ground crinkle. ABORT!
		//

		SUPERCRINKLE_lvert_upto = sc->lvert;
		SUPERCRINKLE_index_upto = sc->index;

		memset(sc, 0, sizeof(SUPERCRINKLE_Crinkle));

		return;
	}

	//
	// This is a wall crinkle.
	//

	for (i = 0; i < point_upto; i++)
	{
		sp = &point[i];

		x = sp->x;
		y = sp->y;
		z = sp->z;

		x -= minx;
		y -= miny;

		x *= 1.0F / 100.0F;
		y *= 1.0F / 100.0F;
		z *= 1.0F / 100.0F;

		SATURATE(x, 0.0F, 1.0F);
		SATURATE(y, 0.0F, 1.0F);

		sp->x = x;
		sp->y = y;
		sp->z = z;	// z is the extrusion coordinate...
	}

	//
	// Work out the normal of each face.
	//

	for (i = 0; i < face_upto; i++)
	{
		sf = &face[i];

		ASSERT(WITHIN(sf->p[0], 0, point_upto - 1));
		ASSERT(WITHIN(sf->p[1], 0, point_upto - 1));
		ASSERT(WITHIN(sf->p[2], 0, point_upto - 1));

		//
		// The two vectors of the triangle.
		//

		ax = point[sf->p[1]].x - point[sf->p[0]].x;
		ay = point[sf->p[1]].y - point[sf->p[0]].y;
		az = point[sf->p[1]].z - point[sf->p[0]].z;

		bx = point[sf->p[2]].x - point[sf->p[0]].x;
		by = point[sf->p[2]].y - point[sf->p[0]].y;
		bz = point[sf->p[2]].z - point[sf->p[0]].z;

		//
		// Normal of the triangle.
		//

		nx = ay*bz - az*by;
		ny = az*bx - ax*bz;
		nz = ax*by - ay*bx;

		//
		// Normalise the normal.
		//

		length     = sqrtf(nx*nx + ny*ny + nz*nz);
		overlength = 1.0F / length;

		sf->nx = nx * overlength;
		sf->ny = ny * overlength;
		sf->nz = nz * overlength;
	}

	//
	// Set the colour interpolations.
	//

	float v[4];

	for (i = 0; i < point_upto; i++)
	{
		sp = &point[i];

		v[0] = (1.0F - sp->x) * (1.0F - sp->y);
		v[1] = (       sp->x) * (1.0F - sp->y);
		v[2] = (1.0F - sp->x) * (       sp->y);
		v[3] = (       sp->x) * (       sp->y);

		ASSERT(WITHIN(v[0] + v[1] + v[2] + v[3], 0.9F, 1.1F));

		sp->i[0] = (UBYTE)(v[0] * 128.0F);
		sp->i[1] = (UBYTE)(v[1] * 128.0F);
		sp->i[2] = (UBYTE)(v[2] * 128.0F);
		sp->i[3] = (UBYTE)(v[3] * 128.0F);
	}
	
	//
	// Create duplicate points so that only points with the
	// same normals are shared.
	//

	for (i = 0; i < face_upto; i++)
	{
		sf = &face[i];

		for (j = 0; j < 3; j++)
		{
			sp = &point[sf->p[j]];

			if (sp->nx == 0.0F &&
				sp->ny == 0.0F &&
				sp->nz == 0.0F)
			{
				//
				// The normal of this point hasn't been set yet.
				// Make is equal to this face normal.
				//

				sp->nx = sf->nx;
				sp->ny = sf->ny;
				sp->nz = sf->nz;
			}
			else
			{
				//
				// Is the normal of this point similar enough to our face?
				//

				dprod =
					sp->nx * sf->nx +
					sp->ny * sf->ny +
					sp->nz * sf->nz;

				if (dprod > 0.94F)
				{
					//
					// These normals are similar enough. About 20degrees different.
					//
				}
				else
				{
					//
					// We must create a new point.
					//

					ASSERT(WITHIN(point_upto, 0, SUPERCRINKLE_MAX_POINTS - 1));

					point[point_upto].x = sp->x;
					point[point_upto].y = sp->y;
					point[point_upto].z = sp->z;

					point[point_upto].nx = sf->nx;
					point[point_upto].ny = sf->ny;
					point[point_upto].nz = sf->nz;

					point[point_upto].i[0] = sp->i[0];
					point[point_upto].i[1] = sp->i[1];
					point[point_upto].i[2] = sp->i[2];
					point[point_upto].i[3] = sp->i[3];

					point[point_upto].duplicate = sf->p[j];

					//
					// An index into this new point from the face.
					//

					sf->p[j] = point_upto++;
				}
			}
		}
	}

	//
	// Work out how each point should be lit.
	//

	for (i = 0; i < point_upto; i++)
	{
		sp = &point[i];

		//
		// Kludged so that faces that point to the front are the same brightness
		// as normal and other faces aren't!
		//

		sp->light  = 128;
		sp->light += SLONG(sp->nx * 64.0F);
		sp->light += SLONG(sp->ny * 64.0F);
	}

	//
	// Create the DrawIndexedPrim lvert data and the colour interpolation data.
	//

	for (i = 0; i < point_upto; i++)
	{
		sp = &point[i];

		ASSERT(WITHIN(SUPERCRINKLE_lvert_upto, 0, SUPERCRINKLE_MAX_LVERTS - 1));

		tl = &SUPERCRINKLE_lvert[SUPERCRINKLE_lvert_upto];

		tl->x = 256.0F - sp->x * 256.0F;
		tl->y = 256.0F - sp->y * 256.0F;
		tl->z = sp->z * 256.0F;
		
		//
		// Initialise lighting to reasonable values.
		//

		tl->color    = 0xff000000 | (sp->light << 16) | (sp->light << 8) | (sp->light << 0);
		tl->specular = 0xff000000;

		tl->tu = sp->x;
		tl->tv = sp->y;

		#ifdef TEX_EMBED

		//
		// UV's depend on the paging...
		//

		tl->tu = pp->m_UOffset + tl->tu * pp->m_UScale;
		tl->tv = pp->m_VOffset + tl->tv * pp->m_VScale;

		#endif

		ASSERT(SUPERCRINKLE_colour_upto == SUPERCRINKLE_lvert_upto);

		SUPERCRINKLE_colour[SUPERCRINKLE_colour_upto].i[0] = sp->i[0] * sp->light >> 7;
		SUPERCRINKLE_colour[SUPERCRINKLE_colour_upto].i[1] = sp->i[1] * sp->light >> 7;
		SUPERCRINKLE_colour[SUPERCRINKLE_colour_upto].i[2] = sp->i[2] * sp->light >> 7;
		SUPERCRINKLE_colour[SUPERCRINKLE_colour_upto].i[3] = sp->i[3] * sp->light >> 7;

		SUPERCRINKLE_lvert_upto  += 1;
		SUPERCRINKLE_colour_upto += 1;

	}

	sc->num_lverts = point_upto;

	//
	// Create the DrawIndexedPrim face data.
	//

	for (i = 0; i < face_upto; i++)
	{
		sf = &face[i];

		ASSERT(WITHIN(SUPERCRINKLE_index_upto + 4, 0, SUPERCRINKLE_MAX_INDICES - 1));

		SUPERCRINKLE_index[SUPERCRINKLE_index_upto + 0] = sf->p[0];
		SUPERCRINKLE_index[SUPERCRINKLE_index_upto + 1] = sf->p[1];
		SUPERCRINKLE_index[SUPERCRINKLE_index_upto + 2] = sf->p[2];
		SUPERCRINKLE_index[SUPERCRINKLE_index_upto + 3] = 0xffff;

		SUPERCRINKLE_index_upto += 4;
	}

	sc->num_indices = face_upto * 4;	// Four indices per face.

	//
	// Mark this page as crinkled.
	//

	SUPERCRINKLE_is_crinkled[crinkle] = TRUE;
}


void SUPERCRINKLE_init()
{
	SLONG i;
	CBYTE fname[512];

	PolyPage *pp;

	//
	// Initalise all the data.
	//


	if ( SUPERCRINKLE_lvert_buffer == NULL )
	{
		SUPERCRINKLE_lvert_buffer = (D3DLVERTEX*)MemAlloc ( sizeof(D3DLVERTEX) * ( SUPERCRINKLE_MAX_LVERTS + 1 ) );
	}

	memset(SUPERCRINKLE_lvert_buffer,  0, sizeof(SUPERCRINKLE_lvert_buffer ));
	memset(SUPERCRINKLE_index_buffer,  0, sizeof(SUPERCRINKLE_index_buffer ));
	memset(SUPERCRINKLE_crinkle,       0, sizeof(SUPERCRINKLE_crinkle      ));
	memset(SUPERCRINKLE_colour,        0, sizeof(SUPERCRINKLE_colour       ));
	memset(SUPERCRINKLE_is_crinkled,   0, sizeof(SUPERCRINKLE_is_crinkled  ));
	memset(SUPERCRINKLE_cache64,       0, sizeof(SUPERCRINKLE_cache64      ));
	memset(SUPERCRINKLE_cache128,      0, sizeof(SUPERCRINKLE_cache128     ));
	memset(SUPERCRINKLE_cache384,      0, sizeof(SUPERCRINKLE_cache384     ));
	memset(SUPERCRINKLE_hash_table64,  0, sizeof(SUPERCRINKLE_hash_table64 ));
	memset(SUPERCRINKLE_hash_table128, 0, sizeof(SUPERCRINKLE_hash_table128));
	memset(SUPERCRINKLE_hash_table384, 0, sizeof(SUPERCRINKLE_hash_table384));

	SUPERCRINKLE_lvert = (D3DLVERTEX *) ((SLONG(SUPERCRINKLE_lvert_buffer) + 31) & ~0x1f);
	SUPERCRINKLE_index = (UWORD      *) ((SLONG(SUPERCRINKLE_index_buffer) + 31) & ~0x1f);

	SUPERCRINKLE_lvert_upto  = 0;
	SUPERCRINKLE_index_upto  = 0;
	SUPERCRINKLE_colour_upto = 0;

	//
	// Setup the freelists of cache entries.
	//
	
	for (i = 0; i < SUPERCRINKLE_MAX_CACHE64  - 1; i++) {SUPERCRINKLE_cache64 [i].cache.next = &SUPERCRINKLE_cache64 [i + 1].cache;}
	for (i = 0; i < SUPERCRINKLE_MAX_CACHE128 - 1; i++) {SUPERCRINKLE_cache128[i].cache.next = &SUPERCRINKLE_cache128[i + 1].cache;}
	for (i = 0; i < SUPERCRINKLE_MAX_CACHE384 - 1; i++) {SUPERCRINKLE_cache384[i].cache.next = &SUPERCRINKLE_cache384[i + 1].cache;}

	SUPERCRINKLE_free64  = &SUPERCRINKLE_cache64 [0].cache;
	SUPERCRINKLE_free128 = &SUPERCRINKLE_cache128[0].cache;
	SUPERCRINKLE_free384 = &SUPERCRINKLE_cache384[0].cache;

	//
	// Load all the crinkles we need.
	//

	for (i = 0; i < 512; i++)
	{
		pp = &POLY_Page[i];

		if (pp->RS.GetTexture())
		{
			//
			// This poly page is used.
			//

			extern CBYTE TEXTURE_world_dir[];
			extern CBYTE TEXTURE_shared_dir[];

			if (i < 256)
			{
				sprintf(fname, "%ssex%03dhi.sex", TEXTURE_world_dir, i);
			}
			else
			{
				sprintf(fname, "%ssex%03dhi.sex", TEXTURE_shared_dir, i);
			}

			SUPERCRINKLE_load(i,fname);
		}
	}
}




//
// Relights the given crinkle.
//

void SUPERCRINKLE_relight(SLONG crinkle, ULONG colour[4], ULONG specular[4])
{
	SLONG i;
	SLONG j;

	ULONG index;
	ULONG hash;

	SLONG cr;
	SLONG cg;
	SLONG cb;

	SLONG sr;
	SLONG sg;
	SLONG sb;

	SUPERCRINKLE_Crinkle *sc;
	SUPERCRINKLE_Cache   *sh;
	SUPERCRINKLE_Colour  *sl;
	SUPERCRINKLE_Precalc *sp;
	D3DLVERTEX           *lv;

	ASSERT(WITHIN(crinkle, 0, SUPERCRINKLE_MAX_CRINKLES - 1));

	sc = &SUPERCRINKLE_crinkle[crinkle];

	//
	// The hash value of this lighting.
	//

	hash = SUPERCRINKLE_hash_function(crinkle, colour, specular);

	if (sc->hash == hash)
	{
		//
		// This crinkle already has this lighting! (probably...)
		//

		return;
	}

	//
	// Where is our freelist and hashtable?
	//

	SUPERCRINKLE_Cache **hash_table;
	SUPERCRINKLE_Cache **free_list;
	SLONG                cache_table_size;

	     if (sc->num_lverts <= 64 ) {hash_table = SUPERCRINKLE_hash_table64 ; free_list = &SUPERCRINKLE_free64;  cache_table_size = SUPERCRINKLE_MAX_CACHE64; }
	else if (sc->num_lverts <= 128) {hash_table = SUPERCRINKLE_hash_table128; free_list = &SUPERCRINKLE_free128; cache_table_size = SUPERCRINKLE_MAX_CACHE128;}
	else if (sc->num_lverts <= 384) {hash_table = SUPERCRINKLE_hash_table384; free_list = &SUPERCRINKLE_free384; cache_table_size = SUPERCRINKLE_MAX_CACHE384;}
	else
	{
		//
		// More than 384 points in a single crinkle! Yikes!
		//

#ifdef TARGET_DC
		// Don't want to know during level builds.
		ASSERT(0);
#endif

		return;
	}

	//
	// Do we have the lighting handy?
	//

	index = hash & (SUPERCRINKLE_HASH_SIZE - 1);

	for (sh = hash_table[index]; sh; sh = sh->next)
	{
		if (sh->hash == hash)
		{
			//
			// Don't bother actually checking the color[] and specular[] value!
			// Assume this is correct!
			//

			goto found_cached_info;
		}
	}

	//
	// No cached info found. Create it!
	//

	if (*free_list == NULL)
	{
		//
		// We must free up a structure. Pick a random one.
		//

		ULONG free = rand() % (cache_table_size - 1);

			 if (sc->num_lverts <= 64 ) {sh = &SUPERCRINKLE_cache64 [free].cache;}
		else if (sc->num_lverts <= 128) {sh = &SUPERCRINKLE_cache128[free].cache;}
		else if (sc->num_lverts <= 384) {sh = &SUPERCRINKLE_cache384[free].cache;}

		//
		// Remove this from the hash table.
		//

		SUPERCRINKLE_Cache **prev;
		SUPERCRINKLE_Cache  *next;

		prev = &hash_table[sh->hash & (SUPERCRINKLE_HASH_SIZE - 1)];
		next =  hash_table[sh->hash & (SUPERCRINKLE_HASH_SIZE - 1)];

		while(1)
		{
			if (next == NULL)
			{
				//	 
				// Reached the end of the list without finding our cache element!
				//

				ASSERT(0);
			}

			if (next == sh)
			{
				//
				// This is the one to delete.
				//

			   *prev = next->next;

				break;
			}

			prev = &next->next;
			next =  next->next;
		}
	}
	else
	{
		//
		// Take this element out of the free list.
		//

		sh        = *free_list;
	   *free_list =  sh->next;
	}
	
	//
	// Build the lighting info and put it into *sh.
	//

	sh->hash = hash;
	sh->next = NULL;
	
	for (i = 0; i < sc->num_lverts; i++)
	{
		sl = &SUPERCRINKLE_colour[sc->lvert + i];
		sp = &sh->precalc[i];

		cr = 0;
		cg = 0;
		cb = 0;

		sr = 0;
		sg = 0;
		sb = 0;

		for (j = 0; j < 4; j++)
		{
			cr += ((colour[j] >> 16) & 0xff) * sl->i[j];
			cg += ((colour[j] >>  8) & 0xff) * sl->i[j];
			cb += ((colour[j] >>  0) & 0xff) * sl->i[j];

			sr += ((specular[j] >> 16) & 0xff) * sl->i[j];
			sg += ((specular[j] >>  8) & 0xff) * sl->i[j];
			sb += ((specular[j] >>  0) & 0xff) * sl->i[j];
		}

		cr >>= 7;
		cg >>= 7;
		cb >>= 7;

		sr >>= 7;
		sg >>= 7;
		sb >>= 7;

		if (cr > 255) {cr = 255;}
		if (cg > 255) {cg = 255;}
		if (cb > 255) {cb = 255;}

		if (sr > 255) {sr = 255;}
		if (sg > 255) {sg = 255;}
		if (sb > 255) {sb = 255;}

		sp->colour   = (cr << 16) | (cg << 8) | (cb << 0);
		sp->specular = (sr << 16) | (sg << 8) | (sb << 0);

		//
		// Assume no fogging and that the alpha is the same across the poly.
		//

		sp->colour   |= colour[0] & 0xff000000;
		sp->specular |= 0xff000000;
	}

	//
	// Add it to the correct hash table.
	//

	sh->next          = hash_table[index];
	hash_table[index] = sh;

  found_cached_info:;

	//
	// Put the lighting into our crinkle.
	//

	for (i = 0; i < sc->num_lverts; i++)
	{
		sp = &sh->precalc[i];
		lv = &SUPERCRINKLE_lvert[sc->lvert + i];

		lv->color    = sp->colour;
		lv->specular = sp->specular;
	}

	//
	// Remember which sort of lighting the crinkle has.
	//

	sc->hash = hash;
}






//
// A 32-byte aligned matrix.
//

UBYTE      SUPERCRINKLE_matrix_buffer[sizeof(D3DMATRIX) + 32];
D3DMATRIX *SUPERCRINKLE_matrix;


SLONG SUPERCRINKLE_draw(SLONG page, ULONG colour[4], ULONG specular[4])
{
	PolyPage             *pp;
	SUPERCRINKLE_Crinkle *sc;
	D3DMULTIMATRIX        d3dmm;

	return(0);

	ASSERT(WITHIN(page, 0, SUPERCRINKLE_MAX_CRINKLES - 1));
	ASSERT(SUPERCRINKLE_is_crinkled[page]);

	//
	// Light it.
	//

	SUPERCRINKLE_relight(page, colour, specular);
	
	//
	// Setup the matrix.
	//

	SUPERCRINKLE_matrix = (D3DMATRIX *) ((SLONG(SUPERCRINKLE_matrix_buffer) + 31) & ~0x1f);

	pp = &POLY_Page[page];
	sc = &SUPERCRINKLE_crinkle[page];

	//
	// Set the renderstate.
	//

	pp->RS.SetChanged();

	//
	// Setup the multi-matrix stuff...
	// 

	d3dmm.lpvVertices   = SUPERCRINKLE_lvert + sc->lvert;
	d3dmm.lpd3dMatrices = SUPERCRINKLE_matrix;
	d3dmm.lpvLightDirs  = NULL;
	d3dmm.lpLightTable  = NULL;

	GenerateMMMatrixFromStandardD3DOnes(
		SUPERCRINKLE_matrix,
	    &g_matProjection,
		&g_matWorld,
	    &g_viewData);
	
	//
	// Do the call.
	//

	DrawIndPrimMM(
		the_display.lp_D3D_Device,
		D3DFVF_LVERTEX,
	   &d3dmm,
		sc->num_lverts,
		SUPERCRINKLE_index + sc->index,
		sc->num_indices);

	return TRUE;
}



#endif

