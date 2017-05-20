//
// A new sewer system.
// 

#ifndef TARGET_DC


#include "game.h"
#include "ns.h"
#include "heap.h"
#include "pap.h"


//
// Around the edges of the sewers there are curvey bits.
// This have type NS_HI_TYPE_CURVE.  The type of curve
// is put into the water field of the square.
// 

#define NS_HI_CURVE_XS	0		// Normal curves along corridors...
#define NS_HI_CURVE_XL	1
#define NS_HI_CURVE_ZS	2
#define NS_HI_CURVE_ZL	3

#define NS_HI_CURVE_ASS	4		// Acute angles on the inside of corners.
#define NS_HI_CURVE_ALS	5
#define NS_HI_CURVE_ASL	6
#define NS_HI_CURVE_ALL	7

#define NS_HI_CURVE_OSS	8		// Obtuse angles on the outside of corners.
#define NS_HI_CURVE_OLS	9
#define NS_HI_CURVE_OSL	10
#define NS_HI_CURVE_OLL	11



//
// The textures. The most common textures are already declared
// to speed things up.
// 

#define NS_TEXTURE_FULL		0

#define NS_TEXTURE_HSTRIP1	1
#define NS_TEXTURE_HSTRIP2	2
#define NS_TEXTURE_HSTRIP3	3
#define NS_TEXTURE_HSTRIP4	4
#define NS_TEXTURE_HSTRIP5	5
#define NS_TEXTURE_HSTRIP6	6
#define NS_TEXTURE_HSTRIP7	7
#define NS_TEXTURE_HSTRIP8	8
#define NS_TEXTURE_HSTRIP9	9
#define NS_TEXTURE_HSTRIP10	10

#define NS_TEXTURE_CT1		11
#define NS_TEXTURE_CT2		12
#define NS_TEXTURE_CT3		13
#define NS_TEXTURE_CT4		14

#define NS_TEXTURE_CT5		15
#define NS_TEXTURE_CT6		16
#define NS_TEXTURE_CT7		17
#define NS_TEXTURE_CT8		18

#define NS_TEXTURE_CT9		19
#define NS_TEXTURE_CT10		20
#define NS_TEXTURE_CT11		21
#define NS_TEXTURE_CT12		22
#define NS_TEXTURE_CT13		23
#define NS_TEXTURE_CT14		24
#define NS_TEXTURE_CT15		25
#define NS_TEXTURE_CT16		26

#define NS_TEXTURE_EL1		27
#define NS_TEXTURE_EL2		28

#define NS_TEXTURE_ER1		29
#define NS_TEXTURE_ER2		30

#define NS_TEXTURE_NUMBER	31

NS_Texture NS_texture[NS_MAX_TEXTURES] =
{
	{{0, 32,  0, 32}, { 0,  0, 32, 32}},

	{{ 0, 32,  0, 32}, { 0,  0,  8,  8}},
	{{ 0, 32,  0, 32}, { 0,  0, 12, 12}},
	{{ 0, 32,  0, 32}, {12, 12, 32, 32}},
	{{ 0, 32,  0, 24}, { 0,  0, 12, 12}},
	{{ 0, 32,  8, 32}, { 0,  0, 12, 12}},
	{{ 0, 24,  0, 12}, {12, 12, 32, 32}},
	{{ 8, 32, 20, 32}, {12, 12, 32, 32}},
	{{ 0,  0, 12,  0}, {12,  0, 12, 12}},
	{{ 0,  8,  0, 32}, {12, 12, 32, 32}},
	{{ 8,  0, 32,  0}, {12, 12, 32, 32}},

	{{ 0, 16,  0, 16}, { 0,  0, 32, 32}},
	{{16, 32, 16, 32}, { 0,  0, 32, 32}},
	{{ 0, 32,  0, 32}, { 0,  0, 16, 16}},
	{{ 0, 32,  0, 32}, {16, 16, 32, 32}},

	{{ 0, 16,  0, 12}, { 0,  0, 16, 12}},
	{{16, 32, 20, 32}, { 0,  0, 12, 16}},
	{{ 0, 12,  0, 16}, {16, 20, 32, 32}},
	{{20, 32, 16, 32}, {20, 16, 32, 32}},

	{{ 0, 20,  0, 16}, { 0, 20, 32, 32}},
	{{ 0, 32, 20, 32}, { 0,  0, 20, 16}},
	{{ 0, 32,  0, 12}, { 0,  0, 16, 20}},
	{{12, 32, 16, 32}, {20,  0, 32, 32}},
	{{ 0, 16,  0, 20}, { 0,  0, 32, 12}},
	{{20, 32,  0, 32}, {12, 16, 32, 32}},
	{{ 0, 12,  0, 32}, {16, 12, 32, 32}},
	{{16, 32, 12, 32}, { 0,  0, 12, 32}},

	{{16,  0, 24,  0}, {32, 32, 12,  0}},
	{{32, 24, 32,  0}, { 8, 12,  0,  0}},

	{{32, 16, 32,  8}, {32, 32,  0, 12}},
	{{ 8,  0, 32,  0}, {12,  8,  0,  0}},
};

SLONG NS_texture_upto = NS_TEXTURE_NUMBER;

//
// The pages.
// 

NS_Page NS_page[NS_PAGE_NUMBER] =

#ifdef PSX

{
	1,2,3,4,5
};

//
// What are the offset into the texture page of these textures?
//

#else

{
	149,	// Rock
	187,	// Sewer
	189,	// Stone
	187,	// Sewer walls
	37		// Grating
};

#endif

//
// The maps.
//

NS_Lo NS_lo[PAP_SIZE_LO][PAP_SIZE_LO];
NS_Hi NS_hi[PAP_SIZE_HI][PAP_SIZE_HI];


//
// The cache elemnts.
//

NS_Cache NS_cache[NS_MAX_CACHES];
UBYTE    NS_cache_free;

//
// The water fall.
//

NS_Fall NS_fall[NS_MAX_FALLS];
UBYTE   NS_fall_free;

//
// The things.
//

NS_St NS_st[NS_MAX_STS];
UBYTE NS_st_free;


//
// The normals.
//

#define NS_NORM_XL	   0
#define NS_NORM_XS	   1
#define NS_NORM_ZL	   2
#define NS_NORM_ZS	   3
#define NS_NORM_YL	   4
#define NS_NORM_DUNNO  5
#define NS_NORM_BLACK  6
#define NS_NORM_GREY   7


void NS_init()
{
	SLONG i;

	SLONG x;
	SLONG z;

	NS_Hi *nh;

	memset((UBYTE*)NS_hi, 0, sizeof(NS_hi));
	memset((UBYTE*)NS_lo, 0, sizeof(NS_lo));

	//
	// Rock everywhere.
	//

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{	
		nh = &NS_hi[x][z];

		nh->packed = NS_HI_TYPE_ROCK;
		nh->bot    = 0x100 - 8 * 2;
	}

	//
	// Initialise the things.
	//

	NS_st_free = 1;

	for (i = 1; i < NS_MAX_STS - 1; i++)
	{
		NS_st[i].type = NS_ST_TYPE_UNUSED;
		NS_st[i].next = i + 1;
	}

	NS_st[NS_MAX_STS - 1].next = NULL;
}



//
// Calculate the top heights of the rock
// 

void NS_precalculate()
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG top;
	SLONG num;
	SLONG used;
	SLONG above;

	SLONG nx;
	SLONG nz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG px;
	SLONG pz;

	SLONG sx;
	SLONG sz;

	SLONG px1;
	SLONG pz1;
	SLONG px2;
	SLONG pz2;

	ULONG flag;
	SLONG curve;

	NS_Hi *nh;
	NS_Hi *nh2;

	//
	// Mark all top bits as unused.
	//

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{	
		nh = &NS_hi[x][z];

		nh->packed &= ~NS_HI_FLAG_TOPUSED;
		nh->packed &= ~NS_HI_FLAG_LOCKTOP;

		//
		// Make sure 'bot' lies on a mapsquare boundary.
		//

		nh->bot &= 0xf8;

		//
		// No rock squares can have water.
		//

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_ROCK)
		{
			nh->water = 0;
		}

		//
		// Grates have water beneath them.
		//

		if (nh->packed & NS_HI_FLAG_GRATE)
		{
			ASSERT(nh->bot >= 16);

			nh->water = nh->bot - 8;
		}
	}

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{	
		nh = &NS_hi[x][z];

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_ROCK)
		{
			//
			// Go through all the points used with this square and
			// set the TOPUSED bit.
			//

			for (i = 0; i < 4; i++)
			{
				px = x + (i &  1);
				pz = z + (i >> 1);

				ASSERT(WITHIN(px, 0, PAP_SIZE_HI - 1));
				ASSERT(WITHIN(pz, 0, PAP_SIZE_HI - 1));

				nh2 = &NS_hi[px][pz];

				nh2->packed |= NS_HI_FLAG_TOPUSED;
			}
		}
	}

	//
	// Make sure no walls are too high or too low.
	// 

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{	
		nh = &NS_hi[x][z];

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_STONE)
		{
			for (dx = 0; dx <= 1; dx++)
			for (dz = 0; dz <= 1; dz++)
			{
				nx = x + dx;
				nz = z + dz;

				nh2 = &NS_hi[nx][nz];

				if (nh2->packed & NS_HI_FLAG_TOPUSED)
				{
					dy  = nh2->top;
					dy -= nh->bot;

					if (dy > 32)
					{
						dy = 30 + (rand() & 0x3);

						nh2->packed |= NS_HI_FLAG_LOCKTOP;
					}
					
					if (dy < 8)
					{
						dy = 8;

						nh2->packed |= NS_HI_FLAG_LOCKTOP;
					}

					top  = nh->bot;
					top += dy;

					if (top > 255)
					{
						top = 255;
					}

					nh2->top = top;
				}
			}
		}
	}

	//
	// Find all points that lie on the edges of sewers and
	// set their top height to be at the edge of the sewer.
	// Set their locked bit to make sure they don't move down.
	//

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{	
		nh = &NS_hi[x][z];

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_SEWER)
		{
			top = nh->bot + 8;

			if (top > 255) {top = 255;}

			//
			// Look for neigbouring squares that aren't sewers.
			//

			const struct
			{
				SBYTE dsx;
				SBYTE dsz;
				SBYTE dpx1;
				SBYTE dpz1;
				SBYTE dpx2;
				SBYTE dpz2;

			} neighbour[4] =
			{
				{-1,0, 0,0, 0,1},
				{+1,0, 1,0, 1,1},

				{0,-1, 0,0, 1,0},
				{0,+1, 0,1, 1,1}
			};

			for (i = 0; i < 4; i++)
			{
				sx = x + neighbour[i].dsx;
				sz = z + neighbour[i].dsz;

				ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
				ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

				nh2 = &NS_hi[sx][sz];

				if (NS_HI_TYPE(nh2) != NS_HI_TYPE_SEWER)
				{
					//
					// Found a neighbouring square that ain't a sewer.
					// Set the points shared with the square.
					//

					px1 = x + neighbour[i].dpx1;
					pz1 = z + neighbour[i].dpz1;

					px2 = x + neighbour[i].dpx2;
					pz2 = z + neighbour[i].dpz2;

					ASSERT(WITHIN(px1, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(pz1, 0, PAP_SIZE_HI - 1));

					ASSERT(WITHIN(px2, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(pz2, 0, PAP_SIZE_HI - 1));

					NS_hi[px1][pz1].packed |= NS_HI_FLAG_LOCKTOP;
					NS_hi[px2][pz2].packed |= NS_HI_FLAG_LOCKTOP;
							      
					NS_hi[px1][pz1].top = top;
					NS_hi[px2][pz2].top = top;
				}
			}
		}
	}

	//
	// Even out the top map.
	//

	#define NS_EVEN_OUT_INNER 3
	#define NS_EVEN_OUT_OUTER 5

	const static SLONG jitter[8] =
	{
		-2,
		-1,
		-1,
		 0,
		 0,
		+1,
		+1,
		+2
	};

	for (i = 0; i < NS_EVEN_OUT_OUTER; i++)
	{
		for (x = 1; x < PAP_SIZE_HI - 1; x++)
		for (z = 1; z < PAP_SIZE_HI - 1; z++)
		{
			nh = &NS_hi[x][z];

			if ( (nh->packed & NS_HI_FLAG_TOPUSED) &&
				!(nh->packed & NS_HI_FLAG_LOCKTOP))
			{
				//
				// Move this point to the average of the surrounding points
				// and yourself.
				//

				top = nh->top;
				num = 1;

				for (j = 0; j < NS_EVEN_OUT_INNER; j++)
				{
					dx = jitter[rand() & 0x7];
					dz = jitter[rand() & 0x7];

					px = x + dx;
					pz = z + dz;

					if (WITHIN(px, 1, PAP_SIZE_HI - 1) &&
						WITHIN(pz, 1, PAP_SIZE_HI - 1))
					{
						nh2 = &NS_hi[px][pz];

						if (nh2->packed & NS_HI_FLAG_TOPUSED)
						{
							top += nh2->top;
							num += 1;
						}
					}
				}

				nh->top = top / num;
			}
		}
	}

	/*

	//
	// Randomise everything a bit.
	//

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		nh = &NS_hi[x][z];

		if (!(nh->packed & NS_HI_FLAG_LOCKTOP) &&
			 (nh->packed & NS_HI_FLAG_TOPUSED))
		{
			top = nh->top;

			top += rand() & 0x3;
			top -= 2;

			if (top > 255)
			{
				top = 255;
			}

			nh->top = top;
		}
	}

	*/

	//
	// Make all rock on the edge of sewers a curvey bit.
	//

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		nh = &NS_hi[x][z];

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_ROCK)
		{
			#define FLAG_XS		(1 << 0)
			#define FLAG_XL		(1 << 1)
			#define FLAG_ZS		(1 << 2)
			#define FLAG_ZL		(1 << 3)

			flag = 0;

			if (NS_HI_TYPE(&NS_hi[x + 1][z]) == NS_HI_TYPE_SEWER) {flag |= FLAG_XS;}
			if (NS_HI_TYPE(&NS_hi[x - 1][z]) == NS_HI_TYPE_SEWER) {flag |= FLAG_XL;}
			if (NS_HI_TYPE(&NS_hi[x][z + 1]) == NS_HI_TYPE_SEWER) {flag |= FLAG_ZS;}
			if (NS_HI_TYPE(&NS_hi[x][z - 1]) == NS_HI_TYPE_SEWER) {flag |= FLAG_ZL;}

			curve = -1;

			switch(flag)
			{
				case FLAG_XS: curve = NS_HI_CURVE_XS; break;
				case FLAG_XL: curve = NS_HI_CURVE_XL; break;
				case FLAG_ZS: curve = NS_HI_CURVE_ZS; break;
				case FLAG_ZL: curve = NS_HI_CURVE_ZL; break;

				case FLAG_XS | FLAG_ZS: curve = NS_HI_CURVE_ASS; break;
				case FLAG_XL | FLAG_ZS: curve = NS_HI_CURVE_ALS; break;
				case FLAG_XS | FLAG_ZL: curve = NS_HI_CURVE_ASL; break;
				case FLAG_XL | FLAG_ZL: curve = NS_HI_CURVE_ALL; break;
			}

			if (curve == -1)
			{
				//
				// Do an extra check for obtuse-angle curves.
				//

				if (NS_HI_TYPE(&NS_hi[x + 1][z + 1]) == NS_HI_TYPE_SEWER) {curve = NS_HI_CURVE_OSS;}
				if (NS_HI_TYPE(&NS_hi[x - 1][z + 1]) == NS_HI_TYPE_SEWER) {curve = NS_HI_CURVE_OLS;}
				if (NS_HI_TYPE(&NS_hi[x + 1][z - 1]) == NS_HI_TYPE_SEWER) {curve = NS_HI_CURVE_OSL;}
				if (NS_HI_TYPE(&NS_hi[x - 1][z - 1]) == NS_HI_TYPE_SEWER) {curve = NS_HI_CURVE_OLL;}
			}

			if (curve != -1)
			{
				//
				// Mark this square as being curvey.
				//

				NS_HI_TYPE_SET(nh, NS_HI_TYPE_CURVE);
				nh->water = curve;

				//
				// The bot height of this square should be set to the
				// bot height of the sewer it is adjacent to.
				//

				for (dx = -1; dx <= 1; dx++)
				for (dz = -1; dz <= 1; dz++)
				{
					nh2 = &NS_hi[x + dx][z + dz];

					if (NS_HI_TYPE(nh2) == NS_HI_TYPE_SEWER)
					{
						nh->bot = nh2->bot;

						goto found_adjacent_sewer_square;
					}
				}

				//
				// Argh! The universe has gone wrong.
				//

				ASSERT(0);

			  found_adjacent_sewer_square:;
			}
		}
	}
}





void NS_cache_init()
{
	SLONG i;

	SLONG x;
	SLONG z;

	//
	// We keep cached points and faces on the heap.
	//

	HEAP_init();

	//
	// Build the free list of cache entries.
	//

	NS_cache_free = 1;

	for (i = 1; i < NS_MAX_CACHES - 1; i++)
	{
		NS_cache[i].used = FALSE;
		NS_cache[i].next = i + 1;
	}

	NS_cache[NS_MAX_CACHES - 1].next = NULL;

	//
	// Clear the cache field in the map.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		NS_lo[x][z].cache = NULL;
	}

	//
	// The waterfalls.
	//

	NS_fall_free = 1;

	for (i = 1; i < NS_MAX_FALLS - 1; i++)
	{
		NS_fall[i].next = i + 1;
	}

	NS_fall[NS_MAX_FALLS - 1].next = NULL;
}




//
// Adding points and faces to the scratch buffer.
//

#define NS_MAX_SCRATCH_POINTS	((HEAP_PAD_SIZE / 2) / sizeof(NS_Point))
#define NS_MAX_SCRATCH_FACES	((HEAP_PAD_SIZE / 2) / sizeof(NS_Face))

NS_Point *NS_scratch_point = (NS_Point *) (&HEAP_pad[0]);
NS_Face  *NS_scratch_face  = (NS_Face  *) (&HEAP_pad[HEAP_PAD_SIZE / 2]);

SLONG NS_scratch_point_upto = 0;
SLONG NS_scratch_face_upto  = 0;

//
// The coordinates of the bottom-left corner of the lo-res mapsquare
// whose points we are adding to the scratch buffers.
//

SLONG NS_scratch_origin_x;
SLONG NS_scratch_origin_z;

//
// When we add a point it is lit from this light.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;

} NS_Slight;

#define NS_MAX_SLIGHTS 9

NS_Slight NS_slight[NS_MAX_SLIGHTS];
SLONG     NS_slight_upto;


void NS_add_point(
		SLONG x,
		SLONG y,
		SLONG z,	// y is in the NS coordinate system. Eighth map-square from 32 blocks underground.
		SLONG norm)
{
	SLONG i;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG bright;
	SLONG light;
	SLONG dprod;

	NS_Slight *nss;

	ASSERT(WITHIN(NS_scratch_point_upto, 0, NS_MAX_SCRATCH_POINTS - 1));

	px = x - NS_scratch_origin_x >> 3;
	pz = z - NS_scratch_origin_z >> 3;

	py = y;

	ASSERT(WITHIN(px, 0, 128));
	ASSERT(WITHIN(pz, 0, 128));
	ASSERT(WITHIN(py, 0, 255));

	     if (norm == NS_NORM_BLACK) {bright = 0;}
	else if (norm == NS_NORM_GREY)  {bright = 32;}
	else
	{
		bright = 192 - (norm << 3);

		for (i = 0; i < NS_slight_upto; i++)
		{
			nss = &NS_slight[i];

			dx = nss->x - px;
			dy = nss->y - py << 2;
			dz = nss->z - pz;

			dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

			switch(norm)
			{
				case NS_NORM_XL: if (dx <= 0) {continue;} else {dprod =  dx * 256 / dist;} break;
				case NS_NORM_XS: if (dx >= 0) {continue;} else {dprod = -dx * 256 / dist;} break;
				case NS_NORM_ZL: if (dz <= 0) {continue;} else {dprod =  dz * 256 / dist;} break;
				case NS_NORM_ZS: if (dz >= 0) {continue;} else {dprod = -dz * 256 / dist;} break;
				case NS_NORM_YL: if (dy <= 0) {continue;} else {dprod =  dy * 256 / dist;} break;
				case NS_NORM_DUNNO: dprod = 200; break;
				default:
					ASSERT(0);
					break;
			}

			ASSERT(dprod >= 0);

			light = 256 - (dist << 1);

			if (light > 0)
			{
				light = light * dprod >> 8;
				bright += light;
			}
		}

		SATURATE(bright, 50, 255);
	}

	NS_scratch_point[NS_scratch_point_upto].x      = px;
	NS_scratch_point[NS_scratch_point_upto].y      = py;
	NS_scratch_point[NS_scratch_point_upto].z      = pz;
	NS_scratch_point[NS_scratch_point_upto].bright = bright;

	NS_scratch_point_upto += 1;
}



void NS_add_face(
		SLONG p[4],
		UBYTE page,
		UBYTE texture)
{
	ASSERT(WITHIN(NS_scratch_face_upto, 0, NS_MAX_SCRATCH_FACES - 1));

	ASSERT(WITHIN(p[0], 0, NS_scratch_point_upto - 1));
	ASSERT(WITHIN(p[1], 0, NS_scratch_point_upto - 1));
	ASSERT(WITHIN(p[2], 0, NS_scratch_point_upto - 1));
	ASSERT(WITHIN(p[3], 0, NS_scratch_point_upto - 1));

	ASSERT(WITHIN(page,    0, NS_PAGE_NUMBER  - 1));
	ASSERT(WITHIN(texture, 0, NS_MAX_TEXTURES - 1));

	NS_scratch_face[NS_scratch_face_upto].p[0]    = p[0];
	NS_scratch_face[NS_scratch_face_upto].p[1]    = p[1];
	NS_scratch_face[NS_scratch_face_upto].p[2]    = p[2];
	NS_scratch_face[NS_scratch_face_upto].p[3]    = p[3];
	NS_scratch_face[NS_scratch_face_upto].page    = page;
	NS_scratch_face[NS_scratch_face_upto].texture = texture;

	NS_scratch_face_upto += 1;
}


//
// Creates the top and bottom squares into the scratch pad.
//

void NS_cache_create_floors(UBYTE mx, UBYTE mz)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG bx;
	SLONG bz;

	SLONG sx;
	SLONG sz;

	SLONG page;

	SLONG p[4];

	NS_Hi *ns;

	//
	// The top points.
	//

	UBYTE pindex[5][5];

	#ifndef NDEBUG
	memset((UBYTE*)pindex, 255, sizeof(pindex));
	#endif

	for (x = 0; x <= 4; x++)
	for (z = 0; z <= 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		ns = &NS_hi[sx][sz];

		if (ns->packed & NS_HI_FLAG_TOPUSED)
		{
			px = sx << 8;
			pz = sz << 8;

			py = ns->top;

			pindex[x][z] = NS_scratch_point_upto;

			NS_add_point(
				px,
				py,
				pz,
				NS_NORM_YL);
		}
	}

	//
	// The top squares.
	//

	for (x = 0; x < 4; x++)
	for (z = 0; z < 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		ns = &NS_hi[sx][sz];

		if (NS_HI_TYPE(ns) == NS_HI_TYPE_ROCK)
		{
			p[0] = pindex[x + 0][z + 0];
			p[1] = pindex[x + 1][z + 0];
			p[2] = pindex[x + 0][z + 1];
			p[3] = pindex[x + 1][z + 1];

			ASSERT(p[0] < 25);
			ASSERT(p[1] < 25);
			ASSERT(p[2] < 25);
			ASSERT(p[3] < 25);

			NS_add_face(
				p,
				NS_PAGE_ROCK,
				NS_TEXTURE_FULL);
		}
	}

	//
	// Sort the bottom squares in order of height.
	//

	typedef struct
	{
		UBYTE x;
		UBYTE z;

		UBYTE bot;	// The bot height of square (x,z) (optimisation only)

	} Bsquare;

	#define MAX_BSQUARES 16

	Bsquare bsquare[MAX_BSQUARES];
	SLONG   bsquare_upto = 0;
	SLONG   insert;

	for (x = 0; x < 4; x++)
	for (z = 0; z < 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;
		
		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		ns = &NS_hi[sx][sz];

		if (NS_HI_TYPE(ns) != NS_HI_TYPE_ROCK    &&
			NS_HI_TYPE(ns) != NS_HI_TYPE_NOTHING &&
			NS_HI_TYPE(ns) != NS_HI_TYPE_CURVE)
		{
			if ((ns->water && !(ns->packed & NS_HI_FLAG_GRATE)) || ns->bot == 0)
			{
				//
				// Water in a sewer is not transparent, so there
				// is no need to draw the ground underneath it- except under a grating.
				//
				// if (bot == 0) then the ground is at -INFINITY... 
				// so we can't see it!
				//
			}
			else
			{
				//
				// Add this square to the array sorted in order of depth.
				//

				insert = bsquare_upto;

				while(1)
				{
					if (insert == 0)
					{
						//
						// No square left to compare against.
						//

						bsquare[insert].x   = sx;
						bsquare[insert].z   = sz;
						bsquare[insert].bot = ns->bot;

						break;
					}

					ASSERT(WITHIN(insert - 1, 0, MAX_BSQUARES - 1));
					ASSERT(WITHIN(insert - 0, 0, MAX_BSQUARES - 1));

					if (ns->bot >= bsquare[insert - 1].bot)
					{
						//
						// This is where to insert the square.
						//

						bsquare[insert].x   = sx;
						bsquare[insert].z   = sz;
						bsquare[insert].bot = ns->bot;

						break;
					}
					else
					{
						bsquare[insert] = bsquare[insert - 1];
					}

					insert -= 1;
				}

				bsquare_upto += 1;
			}
		}
	}

	//
	// Mark all points as undefined.
	//

	memset((UBYTE*)pindex, 255, sizeof(pindex));

	//
	// Go through each of the squares from the bottom to the top.
	//
	
	for (i = 0; i < bsquare_upto; i++)
	{
		sx = bsquare[i].x;
		sz = bsquare[i].z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		ns = &NS_hi[sx][sz];

		bx = sx - (mx << 2);
		bz = sz - (mz << 2);

		ASSERT(WITHIN(bx, 0, 3));
		ASSERT(WITHIN(bz, 0, 3));

		//
		// Create the points for this square.
		//

		for (j = 0; j < 4; j++)
		{
			x = bx + (j &  1);
			z = bz + (j >> 1);

			if (pindex[x][z] != 255)
			{
				//
				// We already have a point at this position. It is at the
				// correct height?
				//

				ASSERT(WITHIN(pindex[x][z], 0, NS_scratch_point_upto - 1));

				if (NS_scratch_point[pindex[x][z]].y == ns->bot)
				{
					//
					// Use this point.
					//

					p[j] = pindex[x][z];

					goto reusing_an_old_point;
				}
			}

			//
			// We have to create a new point here.
			//

			pindex[x][z] = NS_scratch_point_upto;
			p[j]         = NS_scratch_point_upto;

			px = (x << 8) + (mx << PAP_SHIFT_LO);
			pz = (z << 8) + (mz << PAP_SHIFT_LO);

			py = ns->bot;

			NS_add_point(px, py, pz, NS_NORM_YL);

		  reusing_an_old_point:;
		}

		//
		// Create the face.
		//

		if (ns->packed & NS_HI_FLAG_GRATE)
		{
			page = NS_PAGE_GRATE;
		}
		else
		{
			page = NS_PAGE_STONE;
		}

		NS_add_face(p, page, NS_TEXTURE_FULL);
	}
}


//
// Used by create_wallstrip() to add points. If it finds the point in the
// search region, then it returns the index of that point, otherwise it
// creates it.
//

SLONG NS_search_start;
SLONG NS_search_end;		// Exclusive

SLONG NS_create_wallstrip_point(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG norm)
{
	SLONG i;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG ans;

	px = (x - NS_scratch_origin_x) >> 3;
	pz = (z - NS_scratch_origin_z) >> 3;

	py = y;

	ASSERT(WITHIN(px, 0, 128));
	ASSERT(WITHIN(pz, 0, 128));

	ASSERT(WITHIN(py, 0, 255));
	
	for (i = NS_search_start; i < NS_search_end; i++)
	{
		ASSERT(WITHIN(i, 0, NS_scratch_point_upto - 1));

		if (NS_scratch_point[i].x == px &&
			NS_scratch_point[i].y == py &&
			NS_scratch_point[i].z == pz)
		{
			return i;
		}
	}

	//
	// We must create the point.
	//

	ans = NS_scratch_point_upto;

	NS_add_point(x, y, z, norm);

	return ans;
}

SLONG NS_create_wallstrip_point(
		SLONG x,
		SLONG y,
		SLONG z)
{
	return NS_create_wallstrip_point(x, y, z, NS_NORM_DUNNO);
}


//
// Creates a strip of wall which must start at a mapsquare boundary.
// 'shared' is a hint.  The function looks from shared to the end of
// the scratch_point array for points that match ones it wants to create.
//

void NS_cache_create_wallstrip(
		SLONG px1, SLONG pz1,
		SLONG px2, SLONG pz2,
		SLONG bot,	// On a mapsquare boundary.
		SLONG ty1, SLONG ty2,
		SLONG shared,
		SLONG norm)
{
	SLONG last1;
	SLONG last2;

	SLONG now1;
	SLONG now2;

	SLONG py1;
	SLONG py2;

	SLONG p[4];

	SLONG darken_bottom;
	SLONG usenorm;

	//
	// Make sure bot is on a mapsquare boundary.
	//

	ASSERT((bot & 0x7) == 0);

	if (bot == 0)
	{
		bot  =  MIN(ty1,ty2);
		bot -=  24;
		bot &= ~7;

		darken_bottom = TRUE;
	}
	else
	{
		darken_bottom = FALSE;
	}

	//
	// The inclusive region searched to avoid creating identical points.
	//

	NS_search_start = shared;
	NS_search_end   = NS_scratch_point_upto;

	//
	// Create the two bottom points.
	//

	last1 = NS_create_wallstrip_point(px1, bot, pz1, (darken_bottom) ? NS_NORM_BLACK : norm);
	last2 = NS_create_wallstrip_point(px2, bot, pz2, (darken_bottom) ? NS_NORM_BLACK : norm);

	py1 = bot + 8;
	py2 = bot + 8;

	while(1)
	{	
		if (py1 >= ty1 ||
			py2 >= ty2)
		{
			py1 = ty1;
			py2 = ty2;
		}

		//
		// Create two new points.
		//

		if (darken_bottom)
		{
			usenorm       = NS_NORM_GREY;
			darken_bottom = FALSE;
		}
		else
		{
			usenorm = norm;
		}

		now1 = NS_create_wallstrip_point(px1, py1, pz1, usenorm);
		now2 = NS_create_wallstrip_point(px2, py2, pz2, usenorm);

		//
		// Create the face quad.
		//

		p[0] = last1;
		p[1] = last2;
		p[2] = now1;
		p[3] = now2;

		NS_add_face(p, NS_PAGE_STONE, NS_TEXTURE_FULL);

		//
		// Reached the end of the strip.
		//

		if (py1 >= ty1 ||
			py2 >= ty2)
		{
			return;
		}

		py1 += 8;
		py2 += 8;

		if (py1 > 255) {py1 = 255;}
		if (py2 > 255) {py2 = 255;}

		last1 = now1;
		last2 = now2;
	}
}

//
// The extra bits of wall at the top of a sewer.
//

void NS_cache_create_extra_bit_left(
		SLONG px1, SLONG pz1,
		SLONG px2, SLONG pz2,
		SLONG bottom,
		SLONG ty1, SLONG ty2,
		SLONG norm)
{
	SLONG dx = px2 - px1 >> 8;
	SLONG dz = pz2 - pz1 >> 8;
	
	SLONG pindex[6];
	SLONG p[4];

	//
	// Create all the points.
	// 

	pindex[0] = NS_create_wallstrip_point(px1,            bottom,     pz1,            norm);
	pindex[1] = NS_create_wallstrip_point(px1,            ty1,        pz1,            norm);
	pindex[2] = NS_create_wallstrip_point(px1 + dx * 128, bottom + 8, pz1 + dz * 128, norm);
	pindex[3] = NS_create_wallstrip_point(px1 + dx * 192, bottom + 3, pz1 + dz * 192, norm);
	pindex[4] = NS_create_wallstrip_point(px1 + dx * 256, bottom + 2, pz1 + dz * 256, norm);
	pindex[5] = NS_create_wallstrip_point(px1 + dx * 256, bottom,     pz1 + dz * 256, norm);

	//
	// Create the two quads.
	//

	p[0] = pindex[2];
	p[1] = pindex[1];
	p[2] = pindex[3];
	p[3] = pindex[0];

	NS_add_face(p, NS_PAGE_STONE, NS_TEXTURE_EL1);

	p[0] = pindex[4];
	p[1] = pindex[3];
	p[2] = pindex[5];
	p[3] = pindex[0];

	NS_add_face(p, NS_PAGE_STONE, NS_TEXTURE_EL2);
}

void NS_cache_create_extra_bit_right(
		SLONG px1, SLONG pz1,
		SLONG px2, SLONG pz2,
		SLONG bottom,
		SLONG ty1, SLONG ty2,
		SLONG norm)
{
	SLONG dx = px2 - px1 >> 8;
	SLONG dz = pz2 - pz1 >> 8;
	
	SLONG pindex[6];
	SLONG p[4];

	//
	// Create all the points.
	// 

	pindex[0] = NS_create_wallstrip_point(px2,            bottom,     pz2,            norm);
	pindex[1] = NS_create_wallstrip_point(px2,            ty2,        pz2,            norm);
	pindex[2] = NS_create_wallstrip_point(px2 - dx * 128, bottom + 8, pz2 - dz * 128, norm);
	pindex[3] = NS_create_wallstrip_point(px2 - dx * 192, bottom + 3, pz2 - dz * 192, norm);
	pindex[4] = NS_create_wallstrip_point(px2 - dx * 256, bottom + 2, pz2 - dz * 256, norm);
	pindex[5] = NS_create_wallstrip_point(px2 - dx * 256, bottom,     pz2 - dz * 256, norm);

	//
	// Create the two quads.
	//

	p[0] = pindex[1];
	p[1] = pindex[2];
	p[2] = pindex[0];
	p[3] = pindex[3];

	NS_add_face(p, NS_PAGE_STONE, NS_TEXTURE_ER1);

	p[0] = pindex[3];
	p[1] = pindex[4];
	p[2] = pindex[0];
	p[3] = pindex[5];

	NS_add_face(p, NS_PAGE_STONE, NS_TEXTURE_ER2);
}


//
// Creates all the vertical walls.
//

void NS_cache_create_walls(UBYTE mx, UBYTE mz)
{
	SLONG x;
	SLONG z;

	SLONG sx;
	SLONG sz;

	SLONG px1, pz1;
	SLONG px2, pz2;
	SLONG ty1;
	SLONG ty2;
	SLONG bot;

	SLONG shared_last;
	SLONG shared_now;

	NS_Hi *nh;
	NS_Hi *nh2;

	//
	// Walls parallel to the z-axis first.
	//

	for (x = 0; x < 4; x++)
	{
		//
		// The strip from x to (x - 1)
		//

		shared_last = NS_scratch_point_upto - 1;

		if (shared_last < 0)
		{
			shared_last = 0;
		}

		for (z = 0; z < 4; z++)
		{
			shared_now = NS_scratch_point_upto - 1;

			if (shared_now < 0)
			{
				shared_now = 0;
			}

			sx = (mx << 2) + x;
			sz = (mz << 2) + z;

			ASSERT(WITHIN(sx, 1, PAP_SIZE_HI - 1));
			ASSERT(WITHIN(sz, 1, PAP_SIZE_HI - 1));

			nh  = &NS_hi[sx    ][sz];
			nh2 = &NS_hi[sx - 1][sz];

			if (NS_HI_TYPE(nh) != NS_HI_TYPE_ROCK &&
				NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE)
			{
				px1 = sx << 8;
				pz1 = sz << 8;

				px2 = sx     << 8;
				pz2 = sz + 1 << 8;

				bot = nh->bot;

				if (NS_HI_TYPE(nh2) == NS_HI_TYPE_ROCK)
				{
					ty1 = NS_hi[sx][sz + 0].top;
					ty2 = NS_hi[sx][sz + 1].top;

					NS_cache_create_wallstrip(
						px1, pz1,
						px2, pz2,
						bot,
						ty1, ty2,
						shared_last,
						NS_NORM_XL);
				}
				else
				{
					if (nh2->bot > nh->bot)
					{
						ty1 = nh2->bot;
						ty2 = nh2->bot;

						NS_cache_create_wallstrip(
							px1, pz1,
							px2, pz2,
							bot,
							ty1, ty2,
							shared_last,
							NS_NORM_XL);

						if (NS_HI_TYPE(nh2) == NS_HI_TYPE_CURVE)
						{
							//
							// Create the extra bits at the side of the sewer.
							//

							ty1 = NS_hi[sx][sz + 0].top;
							ty2 = NS_hi[sx][sz + 1].top;

							if (NS_HI_TYPE(&NS_hi[sx - 1][sz - 1]) == NS_HI_TYPE_ROCK)
							{
								NS_cache_create_extra_bit_left(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_XL);
							}
							else
							{
								NS_cache_create_extra_bit_right(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_XL);
							}
						}
					}
				}
			}

			shared_last = shared_now;
		}

		//
		// The strip from x to (x + 1)
		//

		shared_last = NS_scratch_point_upto - 1;

		if (shared_last < 0)
		{
			shared_last = 0;
		}

		for (z = 0; z < 4; z++)
		{
			shared_now = NS_scratch_point_upto - 1;

			if (shared_now < 0)
			{
				shared_now = 0;
			}

			sx = (mx << 2) + x;
			sz = (mz << 2) + z;

			ASSERT(WITHIN(sx, 1, PAP_SIZE_HI - 1));
			ASSERT(WITHIN(sz, 1, PAP_SIZE_HI - 1));

			nh  = &NS_hi[sx    ][sz];
			nh2 = &NS_hi[sx + 1][sz];

			if (NS_HI_TYPE(nh) != NS_HI_TYPE_ROCK &&
				NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE)
			{
				px1 = sx + 1 << 8;
				pz1 = sz + 1 << 8;

				px2 = sx + 1 << 8;
				pz2 = sz     << 8;

				bot = nh->bot;

				if (NS_HI_TYPE(nh2) == NS_HI_TYPE_ROCK)
				{
					ty1 = NS_hi[sx + 1][sz + 1].top;
					ty2 = NS_hi[sx + 1][sz + 0].top;

					NS_cache_create_wallstrip(
						px1, pz1,
						px2, pz2,
						bot,
						ty1, ty2,
						shared_last,
						NS_NORM_XS);
				}
				else
				{
					if (nh2->bot > nh->bot)
					{
						ty1 = nh2->bot;
						ty2 = nh2->bot;

						NS_cache_create_wallstrip(
							px1, pz1,
							px2, pz2,
							bot,
							ty1, ty2,
							shared_last,
							NS_NORM_XS);

						if (NS_HI_TYPE(nh2) == NS_HI_TYPE_CURVE)
						{
							//
							// Create the extra bits at the side of the sewer.
							//

							ty1 = NS_hi[sx + 1][sz + 1].top;
							ty2 = NS_hi[sx + 1][sz + 0].top;

							if (NS_HI_TYPE(&NS_hi[sx + 1][sz + 1]) == NS_HI_TYPE_ROCK)
							{
								NS_cache_create_extra_bit_left(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_XS);
							}
							else
							{
								NS_cache_create_extra_bit_right(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_XS);
							}
						}
					}
				}
			}

			shared_last = shared_now;
		}
	}

	//
	// Walls parallel to the x-axis now.
	//

	for (z = 0; z < 4; z++)
	{
		//
		// The strip from z to (z - 1)
		//

		shared_last = NS_scratch_point_upto - 1;

		if (shared_last < 0)
		{
			shared_last = 0;
		}

		for (x = 0; x < 4; x++)
		{
			shared_now = NS_scratch_point_upto - 1;

			if (shared_now < 0)
			{
				shared_now = 0;
			}

			sx = (mx << 2) + x;
			sz = (mz << 2) + z;

			ASSERT(WITHIN(sx, 1, PAP_SIZE_HI - 1));
			ASSERT(WITHIN(sz, 1, PAP_SIZE_HI - 1));

			nh  = &NS_hi[sx][sz    ];
			nh2 = &NS_hi[sx][sz - 1];

			if (NS_HI_TYPE(nh) != NS_HI_TYPE_ROCK &&
				NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE)
			{
				px1 = sx + 1 << 8;
				pz1 = sz     << 8;

				px2 = sx << 8;
				pz2 = sz << 8;

				bot = nh->bot;

				if (NS_HI_TYPE(nh2) == NS_HI_TYPE_ROCK)
				{
					ty1 = NS_hi[sx + 1][sz].top;
					ty2 = NS_hi[sx + 0][sz].top;

					NS_cache_create_wallstrip(
						px1, pz1,
						px2, pz2,
						bot,
						ty1, ty2,
						shared_last,
						NS_NORM_ZL);
				}
				else
				{
					if (nh2->bot > nh->bot)
					{
						ty1 = nh2->bot;
						ty2 = nh2->bot;

						NS_cache_create_wallstrip(
							px1, pz1,
							px2, pz2,
							bot,
							ty1, ty2,
							shared_last,
							NS_NORM_ZL);

						if (NS_HI_TYPE(nh2) == NS_HI_TYPE_CURVE)
						{
							//
							// Create the extra bits at the side of the sewer.
							//

							ty1 = NS_hi[sx + 1][sz].top;
							ty2 = NS_hi[sx + 0][sz].top;

							if (NS_HI_TYPE(&NS_hi[sx + 1][sz - 1]) == NS_HI_TYPE_ROCK)
							{
								NS_cache_create_extra_bit_left(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_ZL);
							}
							else
							{
								NS_cache_create_extra_bit_right(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_ZL);
							}
						}
					}
				}
			}

			shared_last = shared_now;
		}

		//
		// The strip from z to (z + 1)
		//

		shared_last = NS_scratch_point_upto - 1;

		if (shared_last < 0)
		{
			shared_last = 0;
		}

		for (x = 0; x < 4; x++)
		{
			shared_now = NS_scratch_point_upto - 1;

			if (shared_now < 0)
			{
				shared_now = 0;
			}

			sx = (mx << 2) + x;
			sz = (mz << 2) + z;

			ASSERT(WITHIN(sx, 1, PAP_SIZE_HI - 1));
			ASSERT(WITHIN(sz, 1, PAP_SIZE_HI - 1));

			nh  = &NS_hi[sx][sz    ];
			nh2 = &NS_hi[sx][sz + 1];

			if (NS_HI_TYPE(nh) != NS_HI_TYPE_ROCK &&
				NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE)
			{
				px1 = sx     << 8;
				pz1 = sz + 1 << 8;

				px2 = sx + 1 << 8;
				pz2 = sz + 1 << 8;

				bot = nh->bot;

				if (NS_HI_TYPE(nh2) == NS_HI_TYPE_ROCK)
				{
					ty1 = NS_hi[sx + 0][sz + 1].top;
					ty2 = NS_hi[sx + 1][sz + 1].top;

					NS_cache_create_wallstrip(
						px1, pz1,
						px2, pz2,
						bot,
						ty1, ty2,
						shared_last,
						NS_NORM_ZS);
				}
				else
				{
					if (nh2->bot > nh->bot)
					{
						ty1 = nh2->bot;
						ty2 = nh2->bot;

						NS_cache_create_wallstrip(
							px1, pz1,
							px2, pz2,
							bot,
							ty1, ty2,
							shared_last,
							NS_NORM_ZS);

						if (NS_HI_TYPE(nh2) == NS_HI_TYPE_CURVE)
						{
							//
							// Create the extra bits at the side of the sewer.
							//

							ty1 = NS_hi[sx + 0][sz + 1].top;
							ty2 = NS_hi[sx + 1][sz + 1].top;

							if (NS_HI_TYPE(&NS_hi[sx - 1][sz + 1]) == NS_HI_TYPE_ROCK)
							{
								NS_cache_create_extra_bit_left(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_ZS);
							}
							else
							{
								NS_cache_create_extra_bit_right(
									px1, pz1,
									px2, pz2,
									nh2->bot,
									ty1, ty2,
									NS_NORM_ZS);
							}
						}
					}
				}
			}

			shared_last = shared_now;
		}
	}
}


//
// Creates a segment of curved sewer wall.
//

void NS_cache_create_curve_sewer(
		SLONG sx,
		SLONG sz)
{
	SLONG px1;
	SLONG pz1;

	SLONG px2;
	SLONG pz2;

	SLONG dx;
	SLONG dz;

	SLONG dx1;
	SLONG dz1;

	SLONG dx2;
	SLONG dz2;

	SLONG px;
	SLONG py;
	SLONG pz;
	
	SLONG p[4];

	SLONG curve;

	NS_Hi *nh;

	UBYTE pindex[16];

	ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

	nh = &NS_hi[sx][sz];

	//
	// What type of sewer wall curve?
	//

	curve = nh->water;

	if (curve < 4)
	{
		//
		// Normal flat types. Set the paramters we use to create
		// the flat curve at different places and rotations.
		//

		switch(curve)
		{
			case NS_HI_CURVE_XS:

				px1 = sx + 1 << 8;
				pz1 = sz + 0 << 8;

				px2 = sx + 1 << 8;
				pz2 = sz + 1 << 8;

				dx = -256;
				dz =  0;

				break;

			case NS_HI_CURVE_XL:

				px1 = sx + 0 << 8;
				pz1 = sz + 1 << 8;

				px2 = sx + 0 << 8;
				pz2 = sz + 0 << 8;

				dx = +256;
				dz =  0;

				break;

			case NS_HI_CURVE_ZS:

				px1 = sx + 1 << 8;
				pz1 = sz + 1 << 8;

				px2 = sx + 0 << 8;
				pz2 = sz + 1 << 8;

				dx =  0;
				dz = -256;

				break;

			case NS_HI_CURVE_ZL:

				px1 = sx + 0 << 8;
				pz1 = sz + 0 << 8;

				px2 = sx + 1 << 8;
				pz2 = sz + 0 << 8;

				dx =  0;
				dz = +256;

				break;

			default:
				ASSERT(0);
				break;
		}

		//
		// Create the points and faces.
		//

		pindex[0] = NS_create_wallstrip_point(px1, nh->bot + 0, pz1);
		pindex[1] = NS_create_wallstrip_point(px2, nh->bot + 0, pz2);

		pindex[2] = NS_create_wallstrip_point(px1, nh->bot + 2, pz1);
		pindex[3] = NS_create_wallstrip_point(px2, nh->bot + 2, pz2);

		px1 += dx >> 2;
		pz1 += dz >> 2;

		px2 += dx >> 2;
		pz2 += dz >> 2;

		pindex[4] = NS_create_wallstrip_point(px1, nh->bot + 3, pz1);
		pindex[5] = NS_create_wallstrip_point(px2, nh->bot + 3, pz2);

		px1 += dx >> 2;
		pz1 += dz >> 2;

		px2 += dx >> 2;
		pz2 += dz >> 2;

		pindex[6] = NS_create_wallstrip_point(px1, nh->bot + 8, pz1);
		pindex[7] = NS_create_wallstrip_point(px2, nh->bot + 8, pz2);

		p[0] = pindex[0];
		p[1] = pindex[1];
		p[2] = pindex[2];
		p[3] = pindex[3];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP1);

		p[0] = pindex[2];
		p[1] = pindex[3];
		p[2] = pindex[4];
		p[3] = pindex[5];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP2);

		p[0] = pindex[4];
		p[1] = pindex[5];
		p[2] = pindex[6];
		p[3] = pindex[7];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP3);
	}
	else
	if (curve < 8)
	{	
		//
		// Inside curvey bits.
		//

		switch(curve)
		{
			case NS_HI_CURVE_ASS:
				
				px = sx + 1 << 8;
				pz = sz + 1 << 8;

				dx1 =  0;
				dz1 = -1;

				dx2 = -1;
				dz2 =  0;

				break;

			case NS_HI_CURVE_ALS:

				px = sx + 0 << 8;
				pz = sz + 1 << 8;

				dx1 = +1;
				dz1 =  0;

				dx2 =  0;
				dz2 = -1;

				break;

			case NS_HI_CURVE_ASL:

				px = sx + 1 << 8;
				pz = sz + 0 << 8;

				dx1 = -1;
				dz1 =  0;

				dx2 =  0;
				dz2 = +1;

				break;

			case NS_HI_CURVE_ALL:

				px = sx + 0 << 8;
				pz = sz + 0 << 8;

				dx1 =  0;
				dz1 = +1;

				dx2 = +1;
				dz2 =  0;

				break;
				
			default:
				ASSERT(0);
				break;

		}

		//
		// Create the points and faces.
		//

		pindex[ 0] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 *   0, nh->bot + 0, pz + dz1 *   0 + dz2 *   0);
		pindex[ 1] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 *   0, nh->bot + 2, pz + dz1 *   0 + dz2 *   0);
		pindex[ 2] = NS_create_wallstrip_point(px + dx1 *  64 + dx2 *  64, nh->bot + 3, pz + dz1 *  64 + dz2 *  64);
		pindex[ 3] = NS_create_wallstrip_point(px + dx1 * 160 + dx2 * 160, nh->bot + 8, pz + dz1 * 160 + dz2 * 160);

		pindex[ 4] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 * 256, nh->bot + 0, pz + dz1 *   0 + dz2 * 256);
		pindex[ 5] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 * 256, nh->bot + 2, pz + dz1 *   0 + dz2 * 256);
		pindex[ 6] = NS_create_wallstrip_point(px + dx1 *  64 + dx2 * 256, nh->bot + 3, pz + dz1 *  64 + dz2 * 256);
		pindex[ 7] = NS_create_wallstrip_point(px + dx1 * 128 + dx2 * 256, nh->bot + 8, pz + dz1 * 128 + dz2 * 256);

		pindex[ 8] = NS_create_wallstrip_point(px + dx1 * 256 + dx2 *   0, nh->bot + 0, pz + dz1 * 256 + dz2 *   0);
		pindex[ 9] = NS_create_wallstrip_point(px + dx1 * 256 + dx2 *   0, nh->bot + 2, pz + dz1 * 256 + dz2 *   0);
		pindex[10] = NS_create_wallstrip_point(px + dx1 * 256 + dx2 *  64, nh->bot + 3, pz + dz1 * 256 + dz2 *  64);
		pindex[11] = NS_create_wallstrip_point(px + dx1 * 256 + dx2 * 128, nh->bot + 8, pz + dz1 * 256 + dz2 * 128);

		p[0] = pindex[8];
		p[1] = pindex[0];
		p[2] = pindex[9];
		p[3] = pindex[1];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP1);

		p[0] = pindex[0];
		p[1] = pindex[4];
		p[2] = pindex[1];
		p[3] = pindex[5];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP1);

		p[0] = pindex[9];
		p[1] = pindex[1];
		p[2] = pindex[10];
		p[3] = pindex[2];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP4);

		p[0] = pindex[1];
		p[1] = pindex[5];
		p[2] = pindex[2];
		p[3] = pindex[6];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP5);

		p[0] = pindex[10];
		p[1] = pindex[2];
		p[2] = pindex[11];
		p[3] = pindex[3];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP6);

		p[0] = pindex[2];
		p[1] = pindex[6];
		p[2] = pindex[3];
		p[3] = pindex[7];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP7);
	}
	else
	{
		//
		// Outside curvey bits.
		//

		switch(curve)
		{
			case NS_HI_CURVE_OSS:
				
				px = sx + 1 << 8;
				pz = sz + 1 << 8;

				dx1 =  0;
				dz1 = -1;

				dx2 = -1;
				dz2 =  0;

				break;

			case NS_HI_CURVE_OLS:

				px = sx + 0 << 8;
				pz = sz + 1 << 8;

				dx1 = +1;
				dz1 =  0;

				dx2 =  0;
				dz2 = -1;

				break;

			case NS_HI_CURVE_OSL:

				px = sx + 1 << 8;
				pz = sz + 0 << 8;

				dx1 = -1;
				dz1 =  0;

				dx2 =  0;
				dz2 = +1;

				break;

			case NS_HI_CURVE_OLL:

				px = sx + 0 << 8;
				pz = sz + 0 << 8;

				dx1 =  0;
				dz1 = +1;

				dx2 = +1;
				dz2 =  0;

				break;
				
			default:
				ASSERT(0);
				break;

		}

		pindex[0] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 *   0, nh->bot + 2, pz + dz1 *   0 + dz2 *   0);
		pindex[1] = NS_create_wallstrip_point(px + dx1 *  64 + dx2 *  64, nh->bot + 3, pz + dz1 *  64 + dz2 *  64);
		pindex[2] = NS_create_wallstrip_point(px + dx1 *  96 + dx2 *  96, nh->bot + 8, pz + dz1 *  96 + dz2 *  96);
		pindex[3] = NS_create_wallstrip_point(px + dx1 *  64 + dx2 *   0, nh->bot + 3, pz + dz1 *  64 + dz2 *   0);
		pindex[4] = NS_create_wallstrip_point(px + dx1 * 128 + dx2 *   0, nh->bot + 8, pz + dz1 * 128 + dz2 *   0);
		pindex[5] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 *  64, nh->bot + 3, pz + dz1 *   0 + dz2 *  64);
		pindex[6] = NS_create_wallstrip_point(px + dx1 *   0 + dx2 * 128, nh->bot + 8, pz + dz1 *   0 + dz2 * 128);

		p[0] = pindex[3];
		p[1] = pindex[0];
		p[2] = pindex[1];
		p[3] = pindex[5];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP8);

		p[0] = pindex[3];
		p[1] = pindex[1];
		p[2] = pindex[4];
		p[3] = pindex[2];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP9);

		p[0] = pindex[1];
		p[1] = pindex[5];
		p[2] = pindex[2];
		p[3] = pindex[6];

		NS_add_face(p, NS_PAGE_SWALL, NS_TEXTURE_HSTRIP10);
	}
}


void NS_cache_create_curve_top(SLONG sx, SLONG sz)
{
	UBYTE pindex[16];
	UBYTE curve;

	SLONG ox = sx << 8;
	SLONG oz = sz << 8;

	SLONG p[4];

	SLONG px;
	SLONG py;
	SLONG pz;

	NS_Hi *nh;

	ASSERT(WITHIN(sx, 1, PAP_SIZE_HI - 2));
	ASSERT(WITHIN(sz, 1, PAP_SIZE_HI - 2));

	nh = &NS_hi[sx][sz];

	ASSERT(NS_HI_TYPE(nh) == NS_HI_TYPE_CURVE);

	curve = nh->water;	// We use the water field to identify the type of curve.

	switch(curve)
	{
		case NS_HI_CURVE_XS:
			
			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT1);

			break;

		case NS_HI_CURVE_XL:
			
			px = ox + 128;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT2);

			break;

		case NS_HI_CURVE_ZS:
			
			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 128;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 128;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT3);

			break;

		case NS_HI_CURVE_ZL:
			
			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz + 128;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz + 128;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT4);

			break;

		case NS_HI_CURVE_ASS:
			
			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 128;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 96;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 96;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT5);

			break;

		case NS_HI_CURVE_ALS:
			
			px = ox + 128;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 160;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 96;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 128;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT6);

			break;

		case NS_HI_CURVE_ASL:
			
			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz + 128;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 96;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz + 160;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT7);

			break;

		case NS_HI_CURVE_ALL:
			
			px = ox + 160;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz + 160;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz + 128;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT8);

			break;

		case NS_HI_CURVE_OSS:

			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 160;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 160;

			pindex[4] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 128;

			pindex[5] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[4];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT9);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[4];
			p[3] = pindex[5];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT10);

			break;

		case NS_HI_CURVE_OLS:

			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 128;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 96;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 160;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[4] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[5] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[2];
			p[3] = pindex[3];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT11);

			p[0] = pindex[3];
			p[1] = pindex[1];
			p[2] = pindex[4];
			p[3] = pindex[5];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT12);

			break;
			
		case NS_HI_CURVE_OSL:

			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 160;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz + 96;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz + 128;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[4] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[5] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[4];
			p[3] = pindex[2];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT13);

			p[0] = pindex[2];
			p[1] = pindex[3];
			p[2] = pindex[4];
			p[3] = pindex[5];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT14);

			break;
			
		case NS_HI_CURVE_OLL:

			px = ox;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz + 128;

			pindex[0] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 96;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz + 96;

			pindex[1] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 128;
			py = NS_hi[sx + 0][sz + 0].top;
			pz = oz + 0;

			pindex[2] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 0].top;
			pz = oz + 0;

			pindex[3] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox;
			py = NS_hi[sx + 0][sz + 1].top;
			pz = oz + 256;

			pindex[4] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			px = ox + 256;
			py = NS_hi[sx + 1][sz + 1].top;
			pz = oz + 256;

			pindex[5] = NS_create_wallstrip_point(px, py, pz, NS_NORM_YL);

			p[0] = pindex[0];
			p[1] = pindex[1];
			p[2] = pindex[4];
			p[3] = pindex[5];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT15);

			p[0] = pindex[2];
			p[1] = pindex[3];
			p[2] = pindex[1];
			p[3] = pindex[5];

			NS_add_face(p, NS_PAGE_ROCK, NS_TEXTURE_CT16);

			break;
			
	}
}



//
// Creates the curvey bits in a lo-res mapsquare
//

void NS_cache_create_curves(UBYTE mx, UBYTE mz)
{
	SLONG x;
	SLONG z;

	SLONG sx;
	SLONG sz;

	NS_Hi *nh;

	ASSERT(WITHIN(mx, 1, PAP_SIZE_LO - 2));
	ASSERT(WITHIN(mz, 1, PAP_SIZE_LO - 2));

	//
	// The sewer points and faces of the curve only.
	//

	NS_search_start = NS_scratch_point_upto;
	NS_search_end   = NS_scratch_point_upto;

	for (x = 0; x < 4; x++)
	for (z = 0; z < 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		nh = &NS_hi[sx][sz];

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_CURVE)
		{
			NS_cache_create_curve_sewer(sx, sz);

			//
			// So we can use the points we've just created again.
			//

			NS_search_end = NS_scratch_point_upto;
		}
	}

	//
	// The top of the walls of the sewer system.
	//

	NS_search_start = NS_scratch_point_upto;
	NS_search_end   = NS_scratch_point_upto;

	for (x = 0; x < 4; x++)
	for (z = 0; z < 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		nh = &NS_hi[sx][sz];

		if (NS_HI_TYPE(nh) == NS_HI_TYPE_CURVE)
		{
			NS_cache_create_curve_top(sx, sz);

			//
			// So we can use the points we've just created again.
			//

			NS_search_end = NS_scratch_point_upto;
		}
	}
}

void NS_cache_create_falls(UBYTE mx, UBYTE mz, NS_Cache *nc)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG sx;
	SLONG sz;

	SLONG dx;
	SLONG dz;
	
	SLONG nx;
	SLONG nz;

	SLONG fall;

	NS_Hi   *nh;
	NS_Hi   *nh2;
	NS_Fall *nf;

	ASSERT(WITHIN(mx, 1, PAP_SIZE_LO - 2));
	ASSERT(WITHIN(mz, 1, PAP_SIZE_LO - 2));

	for (x = 0; x < 4; x++)
	for (z = 0; z < 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		nh = &NS_hi[sx][sz];

		if (NS_HI_TYPE(nh) != NS_HI_TYPE_CURVE && NS_HI_TYPE(nh) != NS_HI_TYPE_ROCK)
		{
			//
			// Look for water on a higher level.
			//

			const struct
			{
				SBYTE dx;
				SBYTE dz;

			} dir[4] =
			{
				{+1, 0},
				{-1, 0},
				{0, +1},
				{0, -1}
			};

			for (i = 0; i < 4; i++)
			{
				dx = dir[i].dx;
				dz = dir[i].dz;

				nx = sx + dx;
				nz = sz + dz;

				ASSERT(WITHIN(nx, 0, PAP_SIZE_HI - 1));
				ASSERT(WITHIN(nz, 0, PAP_SIZE_HI - 1));

				nh2 = &NS_hi[nx][nz];

				if (nh2->water && (NS_HI_TYPE(nh2) != NS_HI_TYPE_CURVE) && (NS_HI_TYPE(nh2) != NS_HI_TYPE_ROCK))
				{
					//
					// Found a neighbouring square with water.
					//

					if ((nh->water && nh2->water > nh->water) || (!nh->water && nh2->water > nh->bot))
					{
						//
						// Found a waterfall!
						//

						if (NS_fall_free)
						{
							//
							// Take a waterfall out of the linked list.
							// 
							
							ASSERT(WITHIN(NS_fall_free, 1, NS_MAX_FALLS - 1));

							fall         =  NS_fall_free;
							nf           = &NS_fall[fall];
							NS_fall_free =  nf->next;

							//
							// Create the waterfall structure.
							//

							nf->x       = sx;
							nf->z       = sz;
							nf->dx      = dx;
							nf->dz      = dz;
							nf->top     = nh2->water;
							nf->bot     = (nh->water) ? nh->water : nh->bot;
							nf->counter = 0;

							//
							// Add it to the linked list for this cache element.
							// 

							nf->next = nc->fall;
							nc->fall = fall;
						}
					}
				}
			}
		}
	}
}

//
// Creates the walls around the grates.
//

void NS_cache_create_grates(UBYTE mx, UBYTE mz)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG sx;
	SLONG sz;

	SLONG fall;

	NS_Hi *nh;

	SLONG p[4]; 

	static struct
	{
		SBYTE dx;
		SBYTE dz;
		UWORD norm;

	} order[4] =
	{
		{+1, 0, NS_NORM_XS},
		{-1, 0, NS_NORM_XL},
		{0, +1, NS_NORM_ZS},
		{0, -1, NS_NORM_ZL}
	};

	ASSERT(WITHIN(mx, 1, PAP_SIZE_LO - 2));
	ASSERT(WITHIN(mz, 1, PAP_SIZE_LO - 2));

	for (x = 0; x < 4; x++)
	for (z = 0; z < 4; z++)
	{
		sx = (mx << 2) + x;
		sz = (mz << 2) + z;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		nh = &NS_hi[sx][sz];

		if (nh->packed & NS_HI_FLAG_GRATE)
		{
			//
			// Create each side of the wall under the grate.
			//

			y1 = nh->bot;
			y2 = nh->bot - 8;

			for (i = 0; i < 4; i++)
			{
				x1 = (sx << 8) + 0x80 + (order[i].dx + (-order[i].dz) << 7);
				z1 = (sz << 8) + 0x80 + (order[i].dz + (+order[i].dx) << 7);

				x2 = (sx << 8) + 0x80 + (order[i].dx - (-order[i].dz) << 7);
				z2 = (sz << 8) + 0x80 + (order[i].dz - (+order[i].dx) << 7);

				p[0] = NS_scratch_point_upto + 0;
				p[1] = NS_scratch_point_upto + 1;
				p[2] = NS_scratch_point_upto + 2;
				p[3] = NS_scratch_point_upto + 3;

				NS_add_point(x1, y2, z1, order[i].norm);
				NS_add_point(x2, y2, z2, order[i].norm);
				NS_add_point(x1, y1, z1, NS_NORM_BLACK);
				NS_add_point(x2, y1, z2, NS_NORM_BLACK);

				NS_add_face(p, NS_PAGE_STONE, NS_TEXTURE_FULL);
			}
		}
	}
}


#if WE_CALCULATE_OUR_OWN_LIGHT_POSITIONS

//
// Returns where the light for a lo-res mapsquare should go. Returns FALSE
// if there is no light in this mapsquare.
//

SLONG NS_cache_find_light(
		UBYTE mx,
		UBYTE mz,
		UBYTE *light_x,
		UBYTE *light_y,
		UBYTE *light_z)
{
	SLONG i;

	SLONG sx;
	SLONG sz;

	SLONG dmx;
	SLONG dmz;

	SLONG map_x;
	SLONG map_z;

	SLONG dlx;
	SLONG dlz;
	SLONG dist;

	SLONG best_x;
	SLONG best_y;
	SLONG best_z;
	SLONG best_score = -INFINITY;

	SLONG lx;
	SLONG ly;
	SLONG lz;
	SLONG score;

	NS_Lo    *nl;
	NS_Lo    *nl2;
	NS_Hi    *nh;
	NS_Cache *nc;

	//
	// The coordinates of this mapsquare.
	//

	map_x = mx << PAP_SHIFT_LO;
	map_z = mz << PAP_SHIFT_LO;

	//
	// Look for lights in the surrounding mapsquares.
	//

	struct
	{
		SLONG x;
		SLONG z;

	}     other_light[8];
	SLONG other_light_upto = 0;

	ASSERT(WITHIN(mx, 1, PAP_SIZE_LO - 2));
	ASSERT(WITHIN(mz, 1, PAP_SIZE_LO - 2));

	nl = &NS_lo[mx][mz];

	for (dmx = -1; dmx <= +1; dmx += 1)
	for (dmz = -1; dmz <= +1; dmz += 1)
	{
		if (dmx == 0 &&
			dmz == 0)
		{
			//
			// Ignore our mapsquare!
			//

			continue;
		}

		ASSERT(WITHIN(mx + dmx, 0, PAP_SIZE_LO - 1));
		ASSERT(WITHIN(mz + dmz, 0, PAP_SIZE_LO - 1));
		
		nl2 = &NS_lo[mx + dmx][mz + dmz];

		if (nl2->cache)
		{
			ASSERT(WITHIN(nl2->cache, 1, NS_MAX_CACHES - 1));

			nc = &NS_cache[nl2->cache];

			//
			// Found a neighbouring light.
			//

			other_light[other_light_upto].x = nc->light_x + (dmx * 128);
			other_light[other_light_upto].z = nc->light_z + (dmz * 128);

			other_light_upto += 1;
		}
	}

	for (sx = 1; sx < 3; sx++)
	for (sz = 1; sz < 3; sz++)
	{
		ASSERT(WITHIN((mx << 2) + sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN((mz << 2) + sz, 0, PAP_SIZE_HI - 1));

		nh = &NS_hi[(mx << 2) + sx][(mz << 2) + sz];

		switch(NS_HI_TYPE(nh))
		{
			case NS_HI_TYPE_SEWER:
			case NS_HI_TYPE_STONE:

				lx  = sx << 5;
				lz  = sz << 5;
				lx += 16;
				lz += 16;
				ly  = nh->bot + 8;
				
				//
				// Score this selection.
				//

				score = 1;

				//
				// Prefer squares in the middle.
				//

				if (sx == 1 || sx == 2) {score += 100;}
				if (sz == 1 || sz == 2) {score += 100;}

				//
				// Check the distances from other lights.
				//
				
				for (i = 0; i < other_light_upto; i++)
				{
					dlx = abs(lx - other_light[i].x);
					dlz = abs(lz - other_light[i].z);

					dist = dlx + dlz;

						 if (dist == 128) {score +=   100;}
					else if (dist  > 128) {score +=    10;}
					else if (dist  <  96) {score -= 10000;}
				}

				if (score > best_score)
				{
					best_score = score;
					best_x     = lx;
					best_y     = ly;
					best_z     = lz;
				}

				break;

			default:

				//
				// Can't have light on any other square type.
				//

				break;
		}
	}

	if (best_score <= 0)
	{
		//
		// No decent place to put a light here.
		//

		return FALSE;
	}

   *light_x = best_x;
   *light_y = best_y;
   *light_z = best_z;

	return TRUE;
}

#endif


SLONG NS_cache_create(UBYTE mx, UBYTE mz)
{
	SLONG dmx;
	SLONG dmz;

	NS_Cache *nc;
	NS_Lo    *nl;

	SLONG c_index;
	SLONG memory_in_bytes;

	SLONG memory_points;
	SLONG memory_faces;

	//
	// Make sure this square is uncached and there is a
	// free cache entry for it.
	//

	ASSERT(WITHIN(mx, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(mz, 0, PAP_SIZE_LO - 1));

	ASSERT(NS_lo[mx][mz].cache == NULL);
	ASSERT(NS_cache_free       != NULL);

	if (mx == PAP_SIZE_LO - 1 ||
		mz == PAP_SIZE_LO - 1 ||
		mx == 0               ||
		mz == 0)
	{
		//
		// We can't generate the edge of the map.
		//

		return FALSE;
	}

	//
	// We create points and faces in the scratch buffer first
	// and then (once we know how much memory we need) we allocated
	// the memory off the heap and copy the data across.
	//

	NS_scratch_point_upto = 0;
	NS_scratch_face_upto  = 0;

	//
	// The origin of this square.
	//

	NS_scratch_origin_x = mx << PAP_SHIFT_LO;
	NS_scratch_origin_z = mz << PAP_SHIFT_LO;

	//
	// Get our cache element from the free list.
	//

	c_index = NS_cache_free;

	ASSERT(WITHIN(c_index, 1, NS_MAX_CACHES - 1));

	nc = &NS_cache[c_index];

	NS_cache_free = nc->next;

	//
	// All the lights that could effect this square.
	//

	NS_slight_upto = 0;

	for (dmx = -1; dmx <= +1; dmx++)
	for (dmz = -1; dmz <= +1; dmz++)
	{
		ASSERT(WITHIN(mx + dmx, 0, PAP_SIZE_LO - 1));
		ASSERT(WITHIN(mz + dmz, 0, PAP_SIZE_LO - 1));

		nl = &NS_lo[mx + dmx][mz + dmz];

		if (nl->light_y)
		{
			ASSERT(WITHIN(NS_slight_upto, 0, NS_MAX_SLIGHTS - 1));

			NS_slight[NS_slight_upto].x = nl->light_x + dmx * 128;
			NS_slight[NS_slight_upto].z = nl->light_z + dmz * 128;
			NS_slight[NS_slight_upto].y = nl->light_y;

			NS_slight_upto += 1;
		}
	}

	//
	// Create the horizontal squares into the scratch buffers.
	//

	NS_cache_create_floors(mx, mz);
	NS_cache_create_walls (mx, mz);
	NS_cache_create_curves(mx, mz);
	NS_cache_create_grates(mx, mz);

	//
	// Setup the cache element.
	//

	nc->next       = NULL;
	nc->used       = TRUE;
	nc->map_x      = mx;
	nc->map_z      = mz;
	nc->num_points = NS_scratch_point_upto;
	nc->num_faces  = NS_scratch_face_upto;
	nc->fall       = NULL;

	memory_points = nc->num_points * sizeof(NS_Point);
	memory_faces  = nc->num_faces  * sizeof(NS_Face);

	memory_in_bytes = memory_points + memory_faces;

	//
	// Allocate memory and copy data over from the scratch buffers.
	//

	nc->memory = (UBYTE *) HEAP_get(memory_in_bytes);

	ASSERT(nc->memory != NULL);

	memcpy(nc->memory,                 NS_scratch_point, memory_points);
	memcpy(nc->memory + memory_points, NS_scratch_face,  memory_faces);

	//
	// Add the waterfalls.
	// 

	NS_cache_create_falls(mx, mz, nc);

	//
	// Link the mapsquare to this cache element 
	//

	NS_lo[mx][mz].cache = c_index;

	return TRUE;
}


void NS_cache_destroy(UBYTE cache)
{
	SLONG memory_in_bytes;
	SLONG fall;
	SLONG next;

	NS_Fall  *nf;
	NS_Cache *nc;

	ASSERT(WITHIN(cache, 1, NS_MAX_CACHES - 1));

	nc = &NS_cache[cache];

	ASSERT(nc->used);
	ASSERT(WITHIN(nc->map_x, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(nc->map_z, 0, PAP_SIZE_LO - 1));

	//
	// Returns the memory to the heap.
	//

	memory_in_bytes  = nc->num_points * sizeof(NS_Point);
	memory_in_bytes += nc->num_faces  * sizeof(NS_Face);

	HEAP_give(nc->memory, memory_in_bytes);

	//
	// Put the waterfalls back into the free list.
	//

	fall = nc->fall;

	while(fall)
	{
		ASSERT(WITHIN(fall, 1, NS_MAX_FALLS - 1));

		nf = &NS_fall[fall];

		next = nf->next;

		//
		// Add to the free list.
		//

		nf->next     = NS_fall_free;
		NS_fall_free = fall;

		fall = next;
	}

	//
	// Mark as unused and return to the free list.
	//

	nc->used      = FALSE;
	nc->next      = NS_cache_free;
	NS_cache_free = cache;

	//
	// Mark the mapsquare as uncached.
	//

	NS_lo[nc->map_x][nc->map_z].cache = NULL;
}

void NS_cache_fini()
{
	//
	// It does slightly more than necessary!
	//

	NS_cache_init();
}



SLONG NS_calc_height_at(SLONG x, SLONG z)
{
	NS_Hi *nh;

	SLONG ans;

	SLONG mx = x >> PAP_SHIFT_HI;
	SLONG mz = z >> PAP_SHIFT_HI;
	
	if (!WITHIN(mx, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(mz, 0, PAP_SIZE_HI - 1))
	{
		return 0;
	}

	nh = &NS_hi[mx][mz];

	if (NS_HI_TYPE(nh) == NS_HI_TYPE_ROCK)
	{
		return -0x200;
	}
	else
	{
		ans = (nh->bot << 5) + (-32 * 0x100);
	}

	return ans;
}

SLONG NS_calc_splash_height_at(SLONG x, SLONG z)
{
	NS_Hi *nh;

	SLONG ans;

	SLONG mx = x >> PAP_SHIFT_HI;
	SLONG mz = z >> PAP_SHIFT_HI;
	
	if (!WITHIN(mx, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(mz, 0, PAP_SIZE_HI - 1))
	{
		return 0;
	}

	nh = &NS_hi[mx][mz];

	if (NS_HI_TYPE(nh) == NS_HI_TYPE_ROCK)
	{
		return -0x200;
	}
	else
	{
		if (nh->water)
		{
			ans = (nh->water << 5) + (-32 * 0x100);
		}
		else
		{
			ans = (nh->bot   << 5) + (-32 * 0x100);
		}
	}

	return ans;
}

void NS_slide_along(
		SLONG  x1, SLONG  y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  radius)
{
	SLONG i;
	SLONG height;
	SLONG collided = FALSE;

	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dz;
	
	SLONG vx1;
	SLONG vz1;
	SLONG vx2;
	SLONG vz2;

	SLONG sx1;
	SLONG sz1;
	SLONG sx2;
	SLONG sz2;
	SLONG sradius;

	NS_Hi *nh;

	const struct {SBYTE dx; SBYTE dz;} order[4] =
	{
		{+1, 0},
		{-1, 0},
		{0, +1},
		{0, -1}
	};

	SLONG collide;

	//
	// Keep on the map!
	//

	#define NS_SLIDE_MIN ((1 << PAP_SHIFT_LO) << 8)
	#define NS_SLIDE_MAX (((PAP_SIZE_LO - 1) << PAP_SHIFT_LO) << 8)

	if (*x2 < NS_SLIDE_MIN) {*x2 = NS_SLIDE_MIN;}
	if (*x2 > NS_SLIDE_MAX) {*x2 = NS_SLIDE_MAX;}
	if (*z2 < NS_SLIDE_MIN) {*z2 = NS_SLIDE_MIN;}
	if (*z2 > NS_SLIDE_MAX) {*z2 = NS_SLIDE_MAX;}

	//
	// Put the radius in the same coordinate system as (x,y,z)
	//

	radius <<= 8;

	//
	// Collide with the map.
	//

	for (i = 0; i < 4; i++)
	{	
		mx = (x1 >> 16) + order[i].dx;
		mz = (z1 >> 16) + order[i].dz;
		
		ASSERT(WITHIN(mx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(mz, 0, PAP_SIZE_HI - 1));

		nh = &NS_hi[mx][mz];

		switch(NS_HI_TYPE(nh))
		{
			case NS_HI_TYPE_ROCK:
			case NS_HI_TYPE_CURVE:
			case NS_HI_TYPE_NOTHING:
				collide = TRUE;
				break;

			default:

				//
				// What is the height of the top of this square?
				//

				height = (nh->bot << (5 + 8)) + (-32 * 0x100 * 0x100);

				//
				// Can step up a quarter of a block.
				//

				if (*y2 + 0x4000 < height)
				{
					collide = TRUE;
				}
				else
				{
					collide = FALSE;
				}

				break;
		}

		if (collide)
		{
			//
			// Collide with the edge of this square.
			//

			dx = order[i].dx << 7;
			dz = order[i].dz << 7;

			mx = x1 >> 16;
			mz = z1 >> 16;

			sx1 = (mx << 8) + 0x80 + dx - dz;
			sz1 = (mz << 8) + 0x80 + dz + dx;

			sx2 = (mx << 8) + 0x80 + dx + dz;
			sz2 = (mz << 8) + 0x80 + dz - dx;

			sradius = radius >> 8;

			vx1 =  x1 >> 8;
			vz1 =  z1 >> 8;
			vx2 = *x2 >> 8;
			vz2 = *z2 >> 8;

			if (slide_around_sausage(
					sx1, sz1,
					sx2, sz2,
					sradius,
					vx1,  vz1,
				   &vx2, &vz2))
			{
				*x2 = vx2 << 8;
				*z2 = vz2 << 8;
			}
		}
	}
}



SLONG NS_inside(SLONG x, SLONG y, SLONG z)
{
	return y < NS_calc_height_at(x,z);
}



SLONG NS_los_fail_x;
SLONG NS_los_fail_y;
SLONG NS_los_fail_z;

SLONG NS_there_is_a_los(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2)
{
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG dx = x2 - x1;
	SLONG dy = y2 - y1;
	SLONG dz = z2 - z1;

	SLONG len   = QDIST3(abs(dx),abs(dy),abs(dz));
	SLONG steps = len >> 5;

	if(len==0)
	{
		//
		// there is a los for this small distance
		//
		return(TRUE); 
	}
	dx = (dx << 5) / len;
	dy = (dy << 5) / len;
	dz = (dz << 5) / len;
	
	x = x1 + dx;
	y = y1 + dy;
	z = z1 + dz;

	for (i = 1; i < steps; i++)
	{
		if (NS_inside(x,y,z))
		{
			NS_los_fail_x = x - dx;
			NS_los_fail_y = y - dy;
			NS_los_fail_z = z - dz;

			return FALSE;
		}
	}

	return TRUE;
}	



//
// Returns the index of an unused sewer thing or NULL if there
// are no spare sewer things.
// 

SLONG NS_get_unused_st(void)
{
	SLONG i;
	SLONG pick;
	
	NS_St *nst;

	//
	// Look for an unused object.
	//

	pick  = rand() % (NS_MAX_STS - 1);
	pick += 1;

	for (i = 0; i < NS_MAX_STS; i++)
	{
		ASSERT(WITHIN(pick, 1, NS_MAX_STS - 1));
		
		nst = &NS_st[pick];

		if (nst->type == NS_ST_TYPE_UNUSED)
		{
			return pick;
		}

		pick += 1;

		if (pick >= NS_MAX_STS)
		{
			pick = 1;
		}
	}

	//
	// No more sewer things left.
	// 

	return NULL;
}



void NS_add_ladder(SLONG x1, SLONG z1, SLONG x2, SLONG z2, SLONG height)
{
	SLONG mx = (x1 + x2 << 7) + ((z2 - z1) << 2) >> PAP_SHIFT_LO;
	SLONG mz = (z1 + z2 << 7) - ((x2 - x1) << 2) >> PAP_SHIFT_LO;

	if (!WITHIN(mx, 1, PAP_SIZE_LO - 2) ||
		!WITHIN(mz, 1, PAP_SIZE_LO - 2))
	{
		//
		// The ladder is off the map!
		//

		return;
	}

	NS_Lo *nl = &NS_lo[mx][mz];

	SLONG index = NS_get_unused_st();

	if (index == NULL)
	{
		//
		// No spare sewer things.
		//

		return;
	}

	//
	// Create the sewer thing.
	// 

	ASSERT(WITHIN(index, 1, NS_MAX_STS - 1));

	NS_St *nst = &NS_st[index];

	nst->type          = NS_ST_TYPE_LADDER;
	nst->ladder.x1     = x1;
	nst->ladder.z1     = z1;
	nst->ladder.x2     = x2;
	nst->ladder.z2     = z2;
	nst->ladder.height = height;

	//
	// Add to the mapwho.
	// 

	nst->next = nl->st;
	nl->st    = index;
}

void NS_add_prim(
		SLONG prim,
		SLONG yaw,
		SLONG x,
		SLONG y,
		SLONG z)
{
	SLONG mx = x >> PAP_SHIFT_LO;
	SLONG mz = z >> PAP_SHIFT_LO;

	if (!WITHIN(mx, 1, PAP_SIZE_LO - 2) ||
		!WITHIN(mz, 1, PAP_SIZE_LO - 2))
	{
		//
		// The prim is off the map!
		//

		return;
	}

	NS_Lo *nl = &NS_lo[mx][mz];

	SLONG index = NS_get_unused_st();

	if (index == NULL)
	{
		//
		// No spare sewer things.
		//

		return;
	}

	ASSERT(WITHIN(index, 1, NS_MAX_STS - 1));

	NS_St *nst = &NS_st[index];

	//
	// Room on this square for another 
	// 

	nst->type      = NS_ST_TYPE_PRIM;
	nst->prim.prim = prim;
	nst->prim.yaw  = yaw >> 3;
	nst->prim.x    = (x - (mx << PAP_SHIFT_LO)) >> 3;
	nst->prim.z    = (z - (mz << PAP_SHIFT_LO)) >> 3;
	nst->prim.y    = y;

	//
	// Add to the mapwho.
	// 

	nst->next = nl->st;
	nl->st    = index;
}



#endif //#ifndef TARGET_DC


