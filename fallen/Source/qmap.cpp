//
// A whole new world...
//

#include "game.h"
#include <MFStdLib.h>
#include "qmap.h"
#include "fallen/ddengine/headers/qeng.h"

//
// The actual data declared in the header file...
//

UWORD      QMAP_texture[QMAP_MAX_TEXTURES];
SLONG      QMAP_texture_upto;

QMAP_Style QMAP_style[QMAP_MAX_STYLES];
SLONG      QMAP_style_upto;

QMAP_Road  QMAP_road[QMAP_MAX_ROADS];
SLONG      QMAP_road_upto;

QMAP_Cube  QMAP_cube[QMAP_MAX_CUBES];
SLONG      QMAP_cube_upto;

QMAP_Gtex  QMAP_gtex[QMAP_MAX_GTEXES];
SLONG      QMAP_gtex_upto;

QMAP_Cable QMAP_cable[QMAP_MAX_CABLES];
SLONG      QMAP_cable_upto;

SBYTE      QMAP_height[QMAP_MAX_HEIGHTS];
SLONG      QMAP_height_upto;

QMAP_Hmap  QMAP_hmap[QMAP_MAX_HMAPS];
SLONG      QMAP_hmap_upto;

QMAP_Fence QMAP_fence[QMAP_MAX_FENCES];
SLONG      QMAP_fence_upto;

QMAP_Light QMAP_light[QMAP_MAX_LIGHTS];
SLONG      QMAP_light_upto;

QMAP_Prim  QMAP_prim[QMAP_MAX_PRIMS];
SLONG      QMAP_prim_upto;

UWORD      QMAP_all[QMAP_MAX_ALL];
SLONG      QMAP_all_upto;

QMAP_Map   QMAP_map[QMAP_MAPSIZE][QMAP_MAPSIZE];

QMAP_Point QMAP_point[QMAP_MAX_POINTS];
UWORD      QMAP_point_free;

QMAP_Face  QMAP_face[QMAP_MAX_FACES];
UWORD      QMAP_face_free;



void QMAP_init()
{
	SLONG x;
	SLONG z;

	QMAP_Map *qm;

	//
	// Initialise all data structures.
	//

	QMAP_style_upto  = 0;
	QMAP_road_upto   = 0;
	QMAP_cube_upto   = 0;
	QMAP_gtex_upto   = 0;
	QMAP_cable_upto  = 0;
	QMAP_height_upto = 0;
	QMAP_hmap_upto   = 0;
	QMAP_fence_upto  = 0;
	QMAP_light_upto  = 0;
	QMAP_prim_upto   = 0;
	QMAP_all_upto    = 0;

	//
	// Clear the map.
	//

	memset(QMAP_map, 0, sizeof(QMAP_map));

	for (x = 0; x < QMAP_MAPSIZE; x++)
	for (z = 0; z < QMAP_MAPSIZE; z++)
	{
		qm = &QMAP_map[x][z];

		//
		// Nothing to do in here!
		//
	}

	//
	// Setup the linked lists of points and faces.
	//

	QMAP_draw_init();
}

void QMAP_compress_all(void)
{
	SLONG x;
	SLONG z;

	SLONG total_all;

	UWORD *comp;
	SLONG  comp_upto;

	QMAP_Map *qm;

	comp      = (UWORD *) MemAlloc(sizeof(UWORD) * QMAP_MAX_ALL);
	comp_upto = 0;

	if (comp)
	{
		for (x = 0; x < QMAP_MAPSIZE; x++)
		for (z = 0; z < QMAP_MAPSIZE; z++)
		{
			qm = &QMAP_map[x][z];

			total_all = QMAP_TOTAL_ALL(qm);

			ASSERT(comp_upto + total_all <= QMAP_MAX_ALL);

			memcpy(&comp[comp_upto], &QMAP_all[qm->index_all], sizeof(UWORD) * total_all);

			qm->index_all = comp_upto;
			comp_upto    += total_all;
		}

		//
		// Copy the compressed array over the real array.
		//

		memcpy(QMAP_all, comp, sizeof(UWORD) * QMAP_MAX_ALL);
		QMAP_all_upto = comp_upto;

		MemFree(comp);
	}
}



//
// Makes sure there is then given amount of room at the end of the all array.
// Returns TRUE on success.
//

SLONG QMAP_make_room_at_the_end_of_the_all_array(SLONG elements)
{
	//
	// Enough room already?
	//

	if (QMAP_all_upto + elements <= QMAP_MAX_ALL)
	{
		return TRUE;
	}

	//
	// Compress the array.
	//

	QMAP_compress_all();

	if (QMAP_all_upto + elements <= QMAP_MAX_ALL)
	{
		return TRUE;
	}

	//
	// No room, even after compression :o(
	//

	return FALSE;
}




void QMAP_add_road(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG size_x;
	SLONG size_z;

	SLONG total_all;

	SLONG before;
	SLONG after;

	QMAP_Map *qm;

	if (QMAP_road_upto >= QMAP_MAX_ROADS)
	{
		//
		// Not enough memory to add the road.
		//

		MSG_add("Not enough road memory to add the road.");

		return;
	}

	SATURATE(x1, 0, QMAP_SIZE);
	SATURATE(z1, 0, QMAP_SIZE);
	SATURATE(x2, 0, QMAP_SIZE);
	SATURATE(z2, 0, QMAP_SIZE);

	if (x1 > x2) {SWAP(x1,x2);}
	if (z1 > z2) {SWAP(z1,z2);}

	size_x = x2 - x1;
	size_z = z2 - z1;

	SATURATE(size_x, 1, 255);
	SATURATE(size_z, 1, 255);

	QMAP_road[QMAP_road_upto].x      = x1;
	QMAP_road[QMAP_road_upto].z      = z1;
	QMAP_road[QMAP_road_upto].size_x = size_x;
	QMAP_road[QMAP_road_upto].size_z = size_z;

	//
	// Add this road to all the squares it overlaps.
	//

	for (x = x1 & ~0x1f; x < x2; x += 32)
	for (z = z1 & ~0x1f; z < z2; z += 32)
	{
		mx = x >> 5;
		mz = z >> 5;

		qm = &QMAP_map[mx][mz];

		//
		// Make sure there is enough room at the end of the array.
		//

		total_all = QMAP_TOTAL_ALL(qm);

		if (!QMAP_make_room_at_the_end_of_the_all_array(total_all + 1))
		{
			MSG_add("Not enough all memory to add the road.");

			return;
		}

		//
		// Copy this squares all info to the end of the array and
		// insert an extra road at the same time.
		//

		before = QMAP_ALL_INDEX_ROADS(qm) - qm->index_all;
		after  = total_all - before;

		//
		// Copy all the stuff before the roads.
		// Insert the new road.
		// Copy all the stuff after the roads.
		//

		memcpy(&QMAP_all[QMAP_all_upto], &QMAP_all[qm->index_all], sizeof(UWORD) * before);
		QMAP_all[QMAP_all_upto + before] = QMAP_road_upto;
		memcpy(&QMAP_all[QMAP_all_upto + before + 1], &QMAP_all[qm->index_all + before], sizeof(UWORD) * after);

		//
		// Update the numbers...
		//

		qm->num_roads += 1;
		qm->index_all  = QMAP_all_upto;
		QMAP_all_upto += 1;
		QMAP_all_upto += total_all;
	}

	QMAP_road_upto += 1;
}

void QMAP_add_cube(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2,
		SLONG height)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG size_x;
	SLONG size_y;
	SLONG size_z;

	SLONG total_all;

	SLONG before;
	SLONG after;

	QMAP_Map *qm;

	if (QMAP_cube_upto >= QMAP_MAX_CUBES)
	{
		//
		// Not enough memory to add the cube.
		//

		MSG_add("Not enough cube memory to add the cube.");

		return;
	}

	SATURATE(x1, 0, QMAP_SIZE);
	SATURATE(z1, 0, QMAP_SIZE);
	SATURATE(x2, 0, QMAP_SIZE);
	SATURATE(z2, 0, QMAP_SIZE);

	if (x1 > x2) {SWAP(x1,x2);}
	if (z1 > z2) {SWAP(z1,z2);}

	size_x = x2 - x1;
	size_y = height;
	size_z = z2 - z1;

	SATURATE(size_x, 1, 32);
	SATURATE(size_y, 1, 32);
	SATURATE(size_z, 1, 32);

	QMAP_cube[QMAP_cube_upto].x        = x1;
	QMAP_cube[QMAP_cube_upto].z        = z1;
	QMAP_cube[QMAP_cube_upto].size_x   = size_x;
	QMAP_cube[QMAP_cube_upto].size_y   = size_y;
	QMAP_cube[QMAP_cube_upto].size_z   = size_z;
	QMAP_cube[QMAP_cube_upto].flag     = 0;
	QMAP_cube[QMAP_cube_upto].style[0] = 0;
	QMAP_cube[QMAP_cube_upto].style[1] = 0;
	QMAP_cube[QMAP_cube_upto].style[2] = 0;
	QMAP_cube[QMAP_cube_upto].style[3] = 0;
	QMAP_cube[QMAP_cube_upto].style[4] = 0;

	//
	// Add this cube to all the squares it overlaps.
	//

	for (x = x1 & ~0x1f; x < x2; x += 32)
	for (z = z1 & ~0x1f; z < z2; z += 32)
	{
		mx = x >> 5;
		mz = z >> 5;

		qm = &QMAP_map[mx][mz];

		//
		// Make sure there is enough room at the end of the array.
		//

		total_all = QMAP_TOTAL_ALL(qm);

		if (!QMAP_make_room_at_the_end_of_the_all_array(total_all + 1))
		{
			MSG_add("Not enough all memory to add the cube.");

			return;
		}

		//
		// Copy this squares all info to the end of the array and
		// insert an extra cube at the same time.
		//

		before = QMAP_ALL_INDEX_CUBES(qm) - qm->index_all;
		after  = total_all - before;

		//
		// Copy all the stuff before the cubes.
		// Insert the new cube.
		// Copy all the stuff after the cubes.
		//

		memcpy(&QMAP_all[QMAP_all_upto], &QMAP_all[qm->index_all], sizeof(UWORD) * before);
		QMAP_all[QMAP_all_upto + before] = QMAP_cube_upto;
		memcpy(&QMAP_all[QMAP_all_upto + before + 1], &QMAP_all[qm->index_all + before], sizeof(UWORD) * after);

		//
		// Update the numbers...
		//

		qm->num_cubes += 1;
		qm->index_all  = QMAP_all_upto;
		QMAP_all_upto += 1;
		QMAP_all_upto += total_all;
	}

	QMAP_cube_upto += 1;
}



//
// Compresses the prim array.
//

void QMAP_compress_prim_array(void)
{
	SLONG x;
	SLONG z;

	QMAP_Map *qm;

	QMAP_Prim *comp;
	SLONG      comp_upto;

	comp      = (QMAP_Prim *) MemAlloc(sizeof(QMAP_Prim) * QMAP_MAX_PRIMS);
	comp_upto = 0;

	if (comp)
	{
		for (x = 0; x < QMAP_MAPSIZE; x++)
		for (z = 0; z < QMAP_MAPSIZE; z++)
		{
			qm = &QMAP_map[x][z];

			ASSERT(comp_upto + qm->num_prims <= QMAP_MAX_PRIMS);

			memcpy(&comp[comp_upto], &QMAP_prim[qm->index_prim], sizeof(QMAP_Prim) * qm->num_prims);

			qm->index_prim = comp_upto;
			comp_upto     += qm->num_prims;
		}

		//
		// Copy the compressed array over the real array.
		//

		memcpy(QMAP_all, comp, sizeof(QMAP_Prim) * QMAP_MAX_PRIMS);
		QMAP_prim_upto = comp_upto;
	}
}


void QMAP_add_prim(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG prim,
		SLONG yaw)
{
	SLONG prim_x;
	SLONG prim_y;
	SLONG prim_z;
	SLONG prim_yaw;

	SLONG mx = x >> 21;
	SLONG mz = z >> 21;

	QMAP_Map *qm;

	if (!WITHIN(mx, 0, QMAP_MAPSIZE - 1) ||
		!WITHIN(mz, 0, QMAP_MAPSIZE - 1))
	{
		//
		// Off the map.
		//

		return;
	}

	qm = &QMAP_map[mx][mz];

	//
	// Is there enough room at the end of the prim array for
	// this squares' prims plus one more?
	//

	if (QMAP_prim_upto + qm->num_prims + 1 > QMAP_MAX_PRIMS)
	{
		//
		// Make room by compressing the prim array.
		//

		QMAP_compress_prim_array();

		if (QMAP_prim_upto + qm->num_prims + 1 > QMAP_MAX_PRIMS)
		{
			//
			// Still not enough room. Even after compression :o(
			//

			MSG_add("Not enough prim memory to add another prim.");

			return;
		}
	}

	//
	// Copy the prims to the end of the array.
	//

	memcpy(&QMAP_prim[QMAP_prim_upto], &QMAP_prim[qm->index_prim], sizeof(QMAP_Prim) * qm->num_prims);

	qm->index_prim  = QMAP_prim_upto;
	QMAP_prim_upto += qm->num_prims;

	//
	// The coordinates of the new prim. (x,z) are relative to the mapsquare
	// the prim is in and y is relative to the floor at (x,z). They all
	// are in 3-bits per block fixed point.
	//

	prim_x = (x >> 13) & 0xff;
	prim_z = (z >> 13) & 0xff;
	prim_y = (y >> 13);

	SATURATE(prim_y, 0, 255);

	prim_yaw = yaw >> 3;

	//
	// Tag a new prim on the end.
	//

	ASSERT(WITHIN(QMAP_prim_upto, 0, QMAP_MAX_PRIMS - 1));

	QMAP_prim[QMAP_prim_upto].x    = prim_x;
	QMAP_prim[QMAP_prim_upto].y    = prim_y;
	QMAP_prim[QMAP_prim_upto].z    = prim_z;
	QMAP_prim[QMAP_prim_upto].prim = prim;
	QMAP_prim[QMAP_prim_upto].yaw  = prim_yaw;

	qm->num_prims  += 1;
	QMAP_prim_upto += 1;
}



SLONG QMAP_calc_height_at(SLONG x, SLONG z)
{
	//
	// Easy for now!
	//

	return 0;
}

void QMAP_get_cube_coords(
		UWORD cube,
		SLONG *x1, SLONG *y1, SLONG *z1,
		SLONG *x2, SLONG *y2, SLONG *z2)
{
	SLONG mid_x;
	SLONG mid_z;

	QMAP_Cube *qc;

	ASSERT(WITHIN(cube, 0, QMAP_cube_upto - 1));

	qc = &QMAP_cube[cube];

	//
	// The cube we are creating.
	//

	ASSERT(WITHIN(qc->size_x, 1, 32));
	ASSERT(WITHIN(qc->size_y, 1, 32));
	ASSERT(WITHIN(qc->size_z, 1, 32));

	//
	// If you change this code, change the code in QMAP_create_cube() too!
	//

	*x1 = qc->x;
	*z1 = qc->z;

	*x2 = *x1 + qc->size_x;
	*z2 = *z1 + qc->size_z;

	mid_x  = *x1 + *x2 << 7;
	mid_z  = *z1 + *z2 << 7;

	mid_x &= 0xffff0000;
	mid_z &= 0xffff0000;

	*y1   = QMAP_calc_height_at(mid_x, mid_z);
	*y1 >>= 8;
	*y2   = *y1 + qc->size_y;
}





void QMAP_draw_init()
{
	SLONG i;

	QMAP_point_free = 1;

	for (i = 1; i < QMAP_MAX_POINTS - 1; i++)
	{
		QMAP_point[i].next = i + 1;
	}

	QMAP_point[QMAP_MAX_POINTS - 1].next = 0;

	QMAP_face_free = 1;

	for (i = 1; i < QMAP_MAX_FACES - 1; i++)
	{
		QMAP_face[i].next = i + 1;
	}

	QMAP_face[QMAP_MAX_FACES - 1].next = 0;
}

//
// Returns the texture to use on square coord (x,y) on a
// rectange of (size_x, size_y) using the given style.
//

UWORD QMAP_get_style_texture(
		SLONG x,
		SLONG y,
		SLONG size_x,
		SLONG size_y,
		UWORD style)
{
	SLONG cx;
	SLONG cy;

	SLONG mid;
	SLONG index;
	UWORD texture;

	QMAP_Style *qs;

	ASSERT(WITHIN(style, 0, QMAP_style_upto - 1));

	qs = &QMAP_style[style];

	if (qs->flag & QMAP_STYLE_WRAP_X)
	{
		switch(qs->size_x)
		{
			case 1: cx = 0;     break;
			case 2: cx = x & 1; break;
			case 4: cx = x & 3; break;
			default:
				cx = x % qs->size_x;
				break;
		}
	}
	else
	{
		if (x == 0)
		{
			cx = 0;
		}
		else
		if (x >= qs->size_x - 1)
		{
			cx = qs->size_x - 1;
		}
		else
		{
			mid = qs->size_x - 2;

			switch(mid)
			{
				case 0: cx = 0;
				case 1:	cx = 1;
				default:
					cx  = rand() % mid;
					cx += 1;
					break;
			}
		}
	}

	if (qs->flag & QMAP_STYLE_WRAP_Y)
	{
		switch(qs->size_y)
		{
			case 1: cy = 0;     break;
			case 2: cy = y & 1; break;
			case 4: cy = y & 3; break;
			default:
				cy = y % qs->size_y;
				break;
		}
	}
	else
	{
		if (y == 0)
		{
			cy = 0;
		}
		else
		if (y >= qs->size_y - 1)
		{
			cy = qs->size_y - 1;
		}
		else
		{
			mid = qs->size_y - 2;

			switch(mid)
			{
				case -1:
				case  0: cy = 0; break;
				case  1: cy = 1; break;
				default:
					cy  = rand() % mid;
					cy += 1;
					break;
			}
		}
	}

	ASSERT(WITHIN(cx, 0, qs->size_x - 1));
	ASSERT(WITHIN(cy, 0, qs->size_y - 1));

	index  = cy * qs->size_x + cx;
	index += qs->index;

	ASSERT(WITHIN(index, 0, QMAP_texture_upto - 1));

	texture = QMAP_texture[index];

	return texture;
}


//
// Create the faces for given cube.
//

void QMAP_create_cube(QMAP_Draw *qd, SLONG map_x, SLONG map_z, SLONG cube)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	SLONG lx1;
	SLONG lz1;
	SLONG lx2;
	SLONG lz2;

	SLONG look_x;
	SLONG look_z;

	SLONG look_x1;
	SLONG look_y1;
	SLONG look_z1;

	SLONG look_x2;
	SLONG look_y2;
	SLONG look_z2;

	SLONG k1;
	SLONG k2;

	SLONG base;
	SLONG index;
	SLONG cube_look;

	SLONG width;
	ULONG mask;
	ULONG clip;

	UWORD p_index;
	UWORD f_index;

	ULONG face_on[32];	// 128 bytes of stack!

	QMAP_Cube  *qc;
	QMAP_Cube  *qc_look;
	QMAP_Map   *qm_look;
	QMAP_Point *qp;
	QMAP_Face  *qf;

	ASSERT(WITHIN(cube, 0, QMAP_cube_upto - 1));

	qc = &QMAP_cube[cube];

	//
	// The cube we are creating.
	//

	QMAP_get_cube_coords(
		 cube,
		&x1, &y1, &z1,
		&x2, &y2, &z2);

	//
	// The mapsquare we are creating the cube in.
	//

	mx1 = map_x + 0 << 5;
	mz1 = map_z + 0 << 5;

	mx2 = map_x + 1 << 5;
	mz2 = map_z + 1 << 5;

	//
	// Clip the cube to the mapsquare.
	//

	lx1 = MAX(x1, mx1);
	lz1 = MAX(z1, mz1);

	lx2 = MIN(x2, mx2);
	lz2 = MIN(z2, mz2);

	//
	// For each edge of the cube: XS
	//

	if (x1 < mx1)
	{
		//
		// This edge of the cube is not on the mapsquare.
		//
	}
	else
	{
		ASSERT(x1 < mx2);

		//
		// Make sure we only create the faces of the cube
		// that are above this mapsquare.
		//

		clip = 0xffffffff;

		if (z1 > mz1)
		{
			clip >>= z1 - mz1;
		}
		else
		{
			clip <<= mz1 - z1;
		}

		for (i = 0; i < qc->size_y; i++)
		{
			face_on[i] = clip;
		}

		//
		// Which mapsquare do we search for cube to invalidate the faces?
		//

		look_x = map_x;
		look_z = map_z;

		if (x1 == mx1) {look_x -= 1;}

		if (look_x < 0)
		{
			//
			// All these faces are facing onto the edge of the world...
			//
		}
		else
		{
			ASSERT(WITHIN(look_x, 0, QMAP_MAPSIZE - 1));
			ASSERT(WITHIN(look_z, 0, QMAP_MAPSIZE - 1));

			qm_look = &QMAP_map[look_x][look_z];

			//
			// Go through all the cubes on this square.
			//

			base = QMAP_ALL_INDEX_CUBES(qm_look);

			for (i = 0; i < qm_look->num_cubes; i++)
			{
				ASSERT(WITHIN(base + i, 0, QMAP_all_upto - 1));

				index     = base + i;
				cube_look = QMAP_all[index];

				if (cube_look == cube)
				{
					//
					// A cube can't cancel out its own faces!
					//
				}
				else
				{
					ASSERT(WITHIN(cube_look, 0, QMAP_cube_upto - 1));

					qc_look = &QMAP_cube[cube_look];

					//
					// The dimensions of the look cube.
					//

					look_x1 = qc_look->x;
					look_z1 = qc_look->z;

					look_x2 = look_x1 + qc_look->size_x;
					look_z2 = look_z1 + qc_look->size_z;

					//
					// Is this cube going to invalidate our faces?
					//

					if ((look_x1 == x1 && cube_look > cube) ||
						(look_x1  < x1 && look_x2 >= x1))
					{
						//
						// Do they overlap in z?
						//

						if (look_z1 >= z2 ||
							look_z2 <= z1)
						{
							//
							// They don't overlap.
							//
						}
						else
						{
							//
							// Find the mask for the overlap in z.
							//

							width = look_z2 - look_z1;

							ASSERT(width <= 32);

							if (width == 32)
							{
								//
								// 1 << 32 might not be 0...
								//

								mask = 0xffffffff;
							}
							else
							{
								mask  = 1 << width;
								mask -= 1;
							}

							if (look_z1 > z1)
							{
								mask <<= look_z1 - z1;
							}
							else
							if (look_z1 < z1)
							{
								mask >>= z1 - look_z1;
							}

							//
							// The overlap in y.
							//

							look_y1   = QMAP_calc_height_at((look_x1 + look_x2 << 15) & 0xffff0000, (look_z1 + look_z2 << 15) & 0xffff0000);
							look_y1 >>= 16;
							look_y2   = look_y1 + qc_look->size_y;

							for (y = y1, j = 0; y < y2; y++, j++)
							{
								if (y >= look_y1)
								{
									if (y >= look_y2)
									{
										break;
									}
									else
									{
										face_on[j] &= ~mask;
									}
								}
							}
						}
					}
				}
			}
		}

		#ifndef NDEBUG
		memset(qd->hf, 255, sizeof(qd->hf));
		#endif

		//
		// Create the points using the hf buffer in the QMAP_Draw structure.
		//

		for (z = lz1; z <= lz2; z++)
		{
			i = z - z1;

			ASSERT(WITHIN(i, 0, 32));

			if (i == 0)
			{
				mask = 0x1;
			}
			else
			{
				mask = 0x3 << (i - 1);

			}

			for (y = y1, j = 0; y <= y2; y++, j++)
			{
				ASSERT(WITHIN(j, 0, 32));

				k1 = j;
				k2 = j - 1;

				if (k1 > 31) {k1 = 31;}
				if (k2 <  0) {k2 =  0;}

				if ((face_on[k1] | face_on[k2]) & mask)
				{
					//
					// We need point (i,j)
					//

					if (QMAP_point_free == NULL)
					{
						MSG_add("No more points to create the cube.");

						return;
					}
					else
					{
						ASSERT(WITHIN(QMAP_point_free, 1, QMAP_MAX_POINTS - 1));

						qp              = &QMAP_point[QMAP_point_free];
						p_index         = QMAP_point_free;
						QMAP_point_free = qp->next;

						qp->x      = x1 - mx1 << 8;
						qp->y      = y        << 8;
						qp->z      = z  - mz1 << 8;
						qp->red    = 32;
						qp->green  = 32;
						qp->blue   = 32;
						qp->normal = QMAP_POINT_NORMAL_XNEG;
						qp->trans  = 0;

						//
						// Put in the linked list of the QMAP_Draw structure.
						//

						qp->next	   = qd->next_point;
						qd->next_point = p_index;

						//
						// Remember the index of this point.
						//

						qd->hf[i][j].texture = p_index;
					}
				}
			}
		}

		//
		// Create the faces.
		//

		for (z = lz1; z < lz2; z++)
		{
			i = z - z1;

			ASSERT(WITHIN(i, 0, 31));

			mask = 1 << i;

			for (y = y1, j = 0; y < y2; y++, j++)
			{
				ASSERT(WITHIN(j, 0, 31));

				if (face_on[j] & mask)

				//
				// Generate this face.
				//

				if (QMAP_face_free == NULL)
				{
					MSG_add("No more faces to create the cube.");

					return;
				}
				else
				{
					ASSERT(WITHIN(QMAP_face_free, 1, QMAP_MAX_FACES - 1));

					qf             = &QMAP_face[QMAP_face_free];
					f_index        =  QMAP_face_free;
					QMAP_face_free =  qf->next;

					qf->point[0] = qd->hf[i + 0][j + 0].texture;
					qf->point[1] = qd->hf[i + 1][j + 0].texture;
					qf->point[2] = qd->hf[i + 0][j + 1].texture;
					qf->point[3] = qd->hf[i + 1][j + 1].texture;

					ASSERT(WITHIN(qf->point[0], 1, QMAP_MAX_POINTS - 1));
					ASSERT(WITHIN(qf->point[1], 1, QMAP_MAX_POINTS - 1));
					ASSERT(WITHIN(qf->point[2], 1, QMAP_MAX_POINTS - 1));
					ASSERT(WITHIN(qf->point[3], 1, QMAP_MAX_POINTS - 1));

					qf->texture = cube + 5;
					qf->flag    = 0;
					#ifdef EDITOR
					qf->cube    = cube;
					qf->edge    = 0;
					#endif

					//
					// Put in the linked list of the QMAP_Draw structure.
					//

					qf->next      = qd->next_face;
					qd->next_face = f_index;
				}
			}
		}
	}
}






void QMAP_create(QMAP_Draw *qd, SLONG map_x, SLONG map_z)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG base;
	SLONG index;
	SLONG cube;

	QMAP_Map *qm;

	ASSERT(WITHIN(map_x, 0, QMAP_MAPSIZE - 1));
	ASSERT(WITHIN(map_z, 0, QMAP_MAPSIZE - 1));

	qm = &QMAP_map[map_x][map_z];

	//
	// Initialise.
	//

	for (x = 0; x < 32; x++)
	for (z = 0; z < 32; z++)
	{
		qd->hf[x][z].texture = 2;
	}

	qd->map_x = map_x;
	qd->map_z = map_z;

	qd->next_point = 0;
	qd->next_face  = 0;
	qd->trans      = 0;

	//
	// Create the cube faces.
	//

	base = QMAP_ALL_INDEX_CUBES(qm);

	for (i = 0; i < qm->num_cubes; i++)
	{
		index = base + i;
		cube  = QMAP_all[index];

		QMAP_create_cube(qd, map_x, map_z, cube);
	}
}


void QMAP_free(QMAP_Draw *qd)
{
	UWORD point;
	UWORD face;
	UWORD next;

	QMAP_Point *qp;
	QMAP_Face  *qf;

	//
	// Clear all the points and faces.
	//

	for (point = qd->next_point; point; point = next)
	{
		ASSERT(WITHIN(point, 1, QMAP_MAX_POINTS - 1));

		qp = &QMAP_point[point];

		next            = qp->next;
		qp->next        = QMAP_point_free;
		QMAP_point_free = point;
	}

	for (face = qd->next_face; face; face = next)
	{
		ASSERT(WITHIN(face, 1, QMAP_MAX_FACES - 1));

		qf = &QMAP_face[face];

		next           = qf->next;
		qf->next       = QMAP_face_free;
		QMAP_face_free = face;
	}

	qd->next_point = 0;
	qd->next_face  = 0;
}



