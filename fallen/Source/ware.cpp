//
// Warehouses
//
// 
// Far better it is to dare mighty things, to win glorious
// triumphs, even though checkered by failure, than to take
// rank with those poor spirits who neither enjoy nor suffer
// much, because they live in the gray twilight that knows
// neither victory nor defeat.
// 
// THEODORE ROOSEVELT
// 

#include "game.h"
#include "mav.h"
#include "memory.h"
#include "supermap.h"
#include "ware.h"
#include "night.h"
#include "ob.h"
#include "elev.h"


WARE_Ware *WARE_ware;//[WARE_MAX_WARES];
UWORD      WARE_ware_upto;

UWORD *WARE_nav;//[WARE_MAX_NAVS];
UWORD  WARE_nav_upto;

SBYTE *WARE_height;//[WARE_MAX_HEIGHTS];
UWORD  WARE_height_upto;

UWORD *WARE_rooftex;//[WARE_MAX_ROOFTEXES];
UWORD  WARE_rooftex_upto;

UBYTE WARE_in;


//
// The height array for a warehouse.
//

SLONG WARE_calc_height_at(UBYTE ware, SLONG x, SLONG z)
{
	SLONG ans;
	SLONG index;

	WARE_Ware *ww;

	ASSERT(WITHIN(ware, 1, WARE_ware_upto - 1));

	ww = &WARE_ware[ware];

	x >>= 8;
	z >>= 8;

	if (WITHIN(x, ww->minx, ww->maxx) &&
		WITHIN(z, ww->minz, ww->maxz))
	{	
		//
		// Into warehouse space.
		//

		x -= ww->minx;
		z -= ww->minz;

		index = x * ww->nav_pitch + z;

		ASSERT(WITHIN(ww->height + index, 0, WARE_height_upto - 1));

		ans = WARE_height[ww->height + index] << 6;

		return ans;
	}

	return 0;	// Maybe this should assert...
}


#ifndef PSX

//
// Returns the bounding box of the given building. The bounding box is exclusive.
//

void WARE_bounding_box(SLONG dbuilding, 
		SLONG *bx1, SLONG *bz1,
		SLONG *bx2, SLONG *bz2)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	DBuilding *db;
	DFacet    *df;

	SLONG height;

	//
	// Initialise the bounding box.
	//

	x1 = +INFINITY;
	z1 = +INFINITY;
	x2 = -INFINITY;
	z2 = -INFINITY;

	//
	// Find the building.
	//

	ASSERT(WITHIN(dbuilding, 1, next_dbuilding - 1));

	db = &dbuildings[dbuilding];

	//
	// Go through the dfacets for this building.
	//

	for (i = db->StartFacet; i < db->EndFacet; i++)
	{
		ASSERT(WITHIN(i, 1, next_dfacet - 1));

		df = &dfacets[i];

		if (df->FacetType == STOREY_TYPE_NORMAL)
		{
			x = df->x[0];
			z = df->z[0];

			if (x < x1) {x1 = x;}
			if (x > x2) {x2 = x;}
			if (z < z1) {z1 = z;}
			if (z > z2) {z2 = z;}

			x = df->x[1];
			z = df->z[1];

			if (x < x1) {x1 = x;}
			if (x > x2) {x2 = x;}
			if (z < z1) {z1 = z;}
			if (z > z2) {z2 = z;}
		}
	}

	//
	// Return the bounding box.
	//

   *bx1 = x1;
   *bz1 = z1;
   *bx2 = x2;
   *bz2 = z2;
}



//
// The rooftop layer of textures.
//

UWORD WARE_roof_tex[PAP_SIZE_HI][PAP_SIZE_HI];

void WARE_init()
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;
	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;
	SLONG mx;
	SLONG mz;
	SLONG door_x;
	SLONG door_y;
	SLONG door_z;
	SLONG door_angle;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG rx;
	SLONG rz;
	SLONG sx;
	SLONG sz;
	SLONG bx1;
	SLONG bz1;
	SLONG bx2;
	SLONG bz2;
	SLONG nav_memory;
	SLONG facet;
	SLONG index;
	SLONG walkable;

	DBuilding *db;
	DFacet    *df;
	WARE_Ware *ww;
	OB_Info   *oi;
	PrimInfo  *pi;
	RoofFace4 *p_f4;
	DWalkable *p_walk;

	//
	// Load the rooftop textures from the mapfile.
	//
extern	SLONG	save_psx;

	if(!save_psx) // psx has them allready loaded in load_game_map()
	{
		CBYTE *ch;

		for (ch = ELEV_last_map_loaded; *ch; ch++);

		ASSERT(ch[-3] == 'i' || ch[-3] == 'I');
		ASSERT(ch[-2] == 'a' || ch[-2] == 'A');
		ASSERT(ch[-1] == 'm' || ch[-1] == 'M');

		ch[-3] = 'm';
		ch[-2] = 'a';
		ch[-1] = 'p';

		FILE *handle = MF_Fopen(ELEV_last_map_loaded, "rb");

		if (!handle)
		{
			memset(WARE_roof_tex, 0, sizeof(UWORD) * PAP_SIZE_HI * PAP_SIZE_HI);
		}
		else
		{
			#define WARE_OFFSET_TO_ROOFTEXTURES (4 + 128*128*12)

			fseek(handle, WARE_OFFSET_TO_ROOFTEXTURES, SEEK_SET);


			fread(WARE_roof_tex, sizeof(UWORD), PAP_SIZE_HI * PAP_SIZE_HI, handle);

			MF_Fclose(handle);
		}
	}

	//
	// We don't start in a warehouse.
	//

	WARE_in = NULL;

	//
	// The height of the map not including the roofs of warehouses.
	//

	MAV_calc_height_array(TRUE);

	//
	// Initialise the warehouse structures.
	//
	
	memset(WARE_ware, 0, sizeof(WARE_Ware) * WARE_MAX_WARES);

	WARE_ware_upto = 1;

	//
	// Initialise warehouse navigation memory.
	//

	memset(WARE_nav, 0, sizeof(UWORD) * WARE_MAX_NAVS);

	WARE_nav_upto = 0;

	//
	// Initialise warehouse height memory.
	//

	memset(WARE_height, 0, sizeof(UBYTE) * WARE_MAX_HEIGHTS);

	WARE_height_upto = 0;

	//
	// Initialise warehouse rooftex memory.
	//

	memset(WARE_rooftex, 0, sizeof(UWORD) * WARE_MAX_ROOFTEXES);

	WARE_rooftex_upto = 0;

	//
	// Clear all the PAP_LO_FLAG_WAREHOUSE flags.
	//

	for (mx = 0; mx < PAP_SIZE_LO; mx++)
	for (mz = 0; mz < PAP_SIZE_LO; mz++)
	{
		PAP_2LO(mx,mz).Flag &= ~PAP_LO_FLAG_WAREHOUSE;
	}

	//
	// Look for warehouses.
	//

	for (i = 1; i < next_dbuilding; i++)
	{
		db = &dbuildings[i];

		if (db->Type != BUILDING_TYPE_WAREHOUSE)
		{
			continue;
		}

		if (!WITHIN(WARE_ware_upto, 1, WARE_MAX_WARES - 1))
		{
			return;
		}

		ww = &WARE_ware[WARE_ware_upto];

		//
		// Link the warehouse and the building to eachother.
		//

		ww->building = i;
		db->Ware     = WARE_ware_upto;

		//
		// The bounding box the warehouse.
		//

		WARE_bounding_box(i, &bx1, &bz1, &bx2, &bz2);

		ww->minx = bx1;
		ww->minz = bz1;
		ww->maxx = bx2 - 1;	// The bounding box returned by WARE_bounding_box is exclusive, 
		ww->maxz = bz2 - 1;

		//
		// Set the PAP_LO_FLAG_WAREHOUSE flags.
		//

		for (mx = bx1; mx < bx2; mx++)
		for (mz = bz1; mz < bz2; mz++)
		{
			 if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
			 {
				//
				// Mark the lo-res mapsquare as containing a square inside a warehouse.
				//

				PAP_2LO(mx>>2,mz>>2).Flag |= PAP_LO_FLAG_WAREHOUSE;
			 }
		}

		//
		// Find the doors of the warehouse.
		//

		ww->door_upto = 0;

		//
		// Go through all the facets of this building.
		//

		for (facet = db->StartFacet; facet < db->EndFacet; facet++)
		{
			df = &dfacets[facet];

			if (df->FacetType == STOREY_TYPE_DOOR)
			{
				//
				// This is a door. Where should we mavigate to, to be
				// outside the door?
				//

				x1 = df->x[0] << 8;
				z1 = df->z[0] << 8;

				x2 = df->x[1] << 8;
				z2 = df->z[1] << 8;

				dx = x2 - x1 >> 1;
				dz = z2 - z1 >> 1;

				if (!WITHIN(ww->door_upto, 0, WARE_MAX_DOORS - 1))
				{
					//
					// Already found too many doors!
					//

					break;
				}

				//
				// The mapsquares the outside and inside of this door are on.
				//

				ww->door[ww->door_upto].out_x = x1 + dx + dz >> 8;
				ww->door[ww->door_upto].out_z = z1 + dz - dx >> 8;

				ww->door[ww->door_upto].in_x = x1 + dx - dz >> 8;
				ww->door[ww->door_upto].in_z = z1 + dz + dx >> 8;

				ww->door_upto += 1;
			}
		}

		//
		// Allocate navigation memory.
		//

		ww->nav_pitch = bz2 - bz1;
		ww->nav       = WARE_nav_upto;

		//
		// How much memory do we need?
		//

		nav_memory = (bx2 - bx1) * (bz2 - bz1);

		ASSERT(WARE_nav_upto + nav_memory <= WARE_MAX_NAVS);

		WARE_nav_upto += nav_memory;

		//
		// Calculate navigation inside this warehouse.
		//

		MAV_precalculate_warehouse_nav(WARE_ware_upto);

		//
		// Remember the height array for this warehouse. It has the same number
		// of elements as the nav array.
		//

		ASSERT(WARE_height_upto + nav_memory <= WARE_MAX_HEIGHTS);

		ww->height        = WARE_height_upto;
		WARE_height_upto += nav_memory;

		for (x = ww->minx; x <= ww->maxx; x++)
		for (z = ww->minz; z <= ww->maxz; z++)
		{
			index = (x - ww->minx) * ww->nav_pitch + (z - ww->minz);

			ASSERT(WITHIN(ww->height + index, 0, WARE_height_upto - 1));

			WARE_height[ww->height + index] = MAVHEIGHT(x,z);
		}

		//
		// The rooftop textures of this warehouse.
		//

		ww->rooftex = WARE_rooftex_upto;

		for (walkable = db->Walkable; walkable; walkable = p_walk->Next)
		{
			p_walk = &dwalkables[walkable];

			for (j = p_walk->StartFace4; j < p_walk->EndFace4; j++)
			{
				p_f4 = &roof_faces4[j];

				ASSERT(WITHIN(WARE_rooftex_upto, 0, WARE_MAX_ROOFTEXES - 1));

				ASSERT(WITHIN(p_f4->RX&127, 0, PAP_SIZE_HI - 1));
				ASSERT(WITHIN(p_f4->RZ&127, 0, PAP_SIZE_HI - 1));

				WARE_rooftex[WARE_rooftex_upto++] = WARE_roof_tex[p_f4->RX&127][p_f4->RZ&127];
			}
		}

		//
		// Finished with this warehouse.
		//

		WARE_ware_upto++;
	}

	//
	// Work out which OBs are in warehouses and which aren't.
	//

	for (i = 1; i < OB_ob_upto; i++)
	{
		OB_ob[i].flags &= ~OB_FLAG_WAREHOUSE;
	}

	MAV_calc_height_array(FALSE);

	for (i = 1; i < WARE_ware_upto; i++)
	{
		ww = &WARE_ware[i];

		x1 = ww->minx >> 2;
		z1 = ww->minz >> 2;

		x2 = ww->maxx >> 2;
		z2 = ww->maxz >> 2;

		x1 -= 1;
		z1 -= 1;

		x2 += 1;
		z2 += 1;

		SATURATE(x1, 0, PAP_SIZE_LO - 1);
		SATURATE(z1, 0, PAP_SIZE_LO - 1);

		SATURATE(x2, 0, PAP_SIZE_LO - 1);
		SATURATE(z2, 0, PAP_SIZE_LO - 1);

		for (mx = x1; mx <= x2; mx++)
		for (mz = z1; mz <= z2; mz++)
		{
			for (oi = OB_find(mx,mz); oi->prim; oi++)
			{
				PrimInfo *pi = get_prim_info(oi->prim);

				if (oi->prim == 23)
				{
					//
					// This prim is never inside. Stuart knows why.
					//
				}
				else
				{
					//
					// Find which mapsquare the middle of this prim is over.
					//

					if (oi->prim == PRIM_OBJ_SWITCH_OFF)
					{
						sx = oi->x + (SIN(oi->yaw) >> 10) >> 8;
						sz = oi->z + (COS(oi->yaw) >> 10) >> 8;
					}
					else
					{
						dx = pi->minx + pi->maxx >> 1;
						dz = pi->minz + pi->maxz >> 1;

						SLONG matrix[4];
						SLONG useangle;

						SLONG sin_yaw;
						SLONG cos_yaw;

						useangle  = -oi->yaw;
						useangle &=  2047;

						sin_yaw = SIN(useangle);
						cos_yaw = COS(useangle);

						matrix[0] =  cos_yaw;
						matrix[1] = -sin_yaw;
						matrix[2] =  sin_yaw;
						matrix[3] =  cos_yaw;

						rx = MUL64(dx, matrix[0]) + MUL64(dz, matrix[1]);
						rz = MUL64(dx, matrix[2]) + MUL64(dz, matrix[3]);

						sx = oi->x + rx;
						sz = oi->z + rz;

						sx >>= 8;
						sz >>= 8;
					}

					ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

					if (oi->y + pi->maxy < (MAVHEIGHT(sx,sz) << 6) - 0x20)
					{
						OB_ob[oi->index].flags |= OB_FLAG_WAREHOUSE;
					}
				}
			}
		}
	} 

	#if 0

	//
	// Put in the warehouse-door prims.
	//

	for (i = 1; i < WARE_ware_upto; i++)
	{
		ww = &WARE_ware[i];

		for (j = 0; j < ww->door_upto; j++)
		{
			//
			// Where should this door go?
			//

			x1 = (ww->door[j].in_x << 8) + 0x80;
			z1 = (ww->door[j].in_z << 8) + 0x80;

			x2 = (ww->door[j].out_x << 8) + 0x80;
			z2 = (ww->door[j].out_z << 8) + 0x80;

			door_x = x1 + x2 >> 1;
			door_z = z1 + z2 >> 1;

			//
			// Is there already a door here?
			//

			x1 = door_x - 0x100 >> PAP_SHIFT_LO;
			z1 = door_z - 0x100 >> PAP_SHIFT_LO;

			x2 = door_x + 0x100 >> PAP_SHIFT_LO;
			z2 = door_z + 0x100 >> PAP_SHIFT_LO;

			for (mx = x1; mx <= x2; mx++)
			for (mz = z1; mz <= z2; mz++)
			{
				for (oi = OB_find(mx,mz); oi->prim; oi++)
				{
					if (oi->prim == 117)
					{
						//
						// This is the door prim. Is it in the right place?
						//

						if (oi->x == door_x &&
							oi->z == door_z)
						{
							goto already_a_doorframe_here;
						}
					}
				}
			}

			//
			// What's the angle of the door?
			// 

			dx = ww->door[j].out_x - ww->door[j].in_x;
			dz = ww->door[j].out_z - ww->door[j].in_z;

			door_angle  = -Arctan(dx,dz);
			door_angle &=  2047;

			//
			// We need to put in two doors. What is the y-coordinate just
			// outside the door?
			//

			door_y = PAP_calc_map_height_at(
					door_x + (dx << 2),
					door_z + (dz << 2)) + 0x80;

			//
			// Create the two doors.
			//

			OB_create(
				door_x,
				door_y,
				door_z,
				door_angle,
				0,
				0,
				117,
				0,0,0);

			OB_create(
				door_x,
				door_y,
				door_z,
				(door_angle + 1024) & 2047,
				0,
				0,
				117,
				OB_FLAG_WAREHOUSE,0,0);

		  already_a_doorframe_here:;
		}
	}	

	#endif
}

#endif

/*

SLONG WARE_old_amb_red;
SLONG WARE_old_amb_green;
SLONG WARE_old_amb_blue;
SLONG WARE_old_amb_norm_x;
SLONG WARE_old_amb_norm_y;
SLONG WARE_old_amb_norm_z;

void WARE_enter(SLONG building)
{
	WARE_Ware *ww;
	DBuilding *db;

	ASSERT(WITHIN(building, 1, next_dbuilding - 1));

	db = &dbuildings[building];

	//
	// Make sure this building is a warehouse.
	//

	ASSERT(db->Type == BUILDING_TYPE_WAREHOUSE);
	ASSERT(WITHIN(db->Ware, 1, WARE_ware_upto - 1));

	ww = &WARE_ware[db->Ware];

	//
	// Remember which warehouse we are in.
	//

	WARE_in = db->Ware;

	//
	// Invalidate all the cached lighting so when it is recreated it will be
	// correct for the warehouse.
	//

	NIGHT_destroy_all_cached_info();

	//
	// Change the ambient light.
	//

	WARE_old_amb_red   = NIGHT_amb_red;
	WARE_old_amb_green = NIGHT_amb_green;
	WARE_old_amb_blue  = NIGHT_amb_blue;

 	WARE_old_amb_norm_x = NIGHT_amb_norm_x;
	WARE_old_amb_norm_y = NIGHT_amb_norm_y;
	WARE_old_amb_norm_z = NIGHT_amb_norm_z;

	NIGHT_amb_red   = 45;
	NIGHT_amb_green = 45;
	NIGHT_amb_blue  = 45;

	NIGHT_amb_norm_x =  0;
	NIGHT_amb_norm_y = -255;
	NIGHT_amb_norm_z =  0;
}

void WARE_exit()
{
	//
	// We aren't in a warehouse anymore.
	//

	WARE_in = NULL;

	//
	// Invalidate all the cached lighting so when it is recreated it will be
	// correct for the outside.
	//

	NIGHT_destroy_all_cached_info();

	//
	// Restore the ambient light for the outside.
	//

	NIGHT_amb_red   = WARE_old_amb_red;
	NIGHT_amb_green = WARE_old_amb_green;
	NIGHT_amb_blue  = WARE_old_amb_blue;

 	NIGHT_amb_norm_x = WARE_old_amb_norm_x;
	NIGHT_amb_norm_y = WARE_old_amb_norm_y;
	NIGHT_amb_norm_z = WARE_old_amb_norm_z;
}

*/

MAV_Action WARE_mav_enter(Thing *p_person, UBYTE ware, UBYTE caps)
{
	UBYTE i;
	UBYTE best_door;
	SLONG best_dist = 0xffff;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	MAV_Action ans;

	WARE_Ware *ww;

	ASSERT(WITHIN(ware, 1, WARE_ware_upto - 1));

	ww = &WARE_ware[ware];

	//
	// Which warehouse entrance shall we mav to?
	//

	for (i = 0; i < ww->door_upto; i++)
	{
		dx = ww->door[i].out_x - (p_person->WorldPos.X >> 16);
		dz = ww->door[i].out_z - (p_person->WorldPos.Z >> 16);

		dist = abs(dx) + abs(dz);

		if (best_dist > dist)
		{
			best_dist = dist;
			best_door = i;
		}
	}

	if (best_dist == 0)
	{
		//
		// The person is already at the entrance square. Make him walk
		// into the warehouse.
		// 

		ans.dest_x = ww->door[best_door].in_x;
		ans.dest_z = ww->door[best_door].in_z;
		ans.action = MAV_ACTION_GOTO;
		ans.dir    = 0;
	}
	else
	{
		//
		// Do the mav.
		//

		ans = MAV_do(
				p_person->WorldPos.X >> 16,
				p_person->WorldPos.Z >> 16,
				ww->door[best_door].out_x,
				ww->door[best_door].out_z,
				caps);
#ifndef PSX
#ifndef TARGET_DC
#ifdef DEBUG
		AENG_world_line(
			p_person->WorldPos.X >> 8,
			p_person->WorldPos.Y >> 8,
			p_person->WorldPos.Z >> 8,
			16,
			0xffffff,
			(ww->door[best_door].out_x << 8) + 0x80,
			0,
			(ww->door[best_door].out_z << 8) + 0x80,
			0,
			0x00ff00,
			TRUE);
#endif
#endif
#endif
	}

	return ans;
}

MAV_Action WARE_mav_inside(Thing *p_person, UBYTE dest_x, UBYTE dest_z, UBYTE caps)
{
	UBYTE start_x;
	UBYTE start_z;

	UWORD     *old_mav;
	UWORD      old_mav_pitch;

	MAV_Action ma;
	WARE_Ware *ww;

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_WAREHOUSE);
	ASSERT(WITHIN(p_person->Genus.Person->Ware, 1, WARE_ware_upto - 1));

	ww = &WARE_ware[p_person->Genus.Person->Ware];

	//
	// Make sure the destination is inside the warehouse.
	//

	ASSERT(WARE_in_floorplan(p_person->Genus.Person->Ware, dest_x, dest_z));

	//
	// SATURATE TO BE IN THE WAREHOUSE MARK!
	//


	//
	// Set the MAV module to use this warehouses' MAV array.
	//

	old_mav       = MAV_nav;
	old_mav_pitch = MAV_nav_pitch;

	MAV_nav       = &WARE_nav[ww->nav];
	MAV_nav_pitch =  ww->nav_pitch;

	//
	// Put the start and destination into warehouse space.
	//

	start_x = (p_person->WorldPos.X >> 16);
	start_z = (p_person->WorldPos.Z >> 16);

	ASSERT(WARE_in_floorplan(p_person->Genus.Person->Ware, start_x, start_z));

	start_x -= ww->minx;
	start_z -= ww->minz;

	dest_x -= ww->minx;
	dest_z -= ww->minz;

	//
	// Do the mav.
	//

	ma = MAV_do(
			start_x,
			start_z,
			dest_x,
			dest_z,
			caps);

	//
	// Put the answer into worldspace
	//
	
	ma.dest_x += ww->minx;
	ma.dest_z += ww->minz;

	//
	// Restore the old MAV array.
	//

	MAV_nav       = old_mav;
	MAV_nav_pitch = old_mav_pitch;

	return ma;
}

MAV_Action WARE_mav_exit(Thing *p_person, UBYTE caps)
{
	UBYTE i;
	UBYTE best_door = 0;
	UBYTE start_x;
	UBYTE start_z;
	UBYTE dest_x;
	UBYTE dest_z;
	SLONG best_dist = 0xffff;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	UWORD     *old_mav;
	UWORD      old_mav_pitch;

	MAV_Action ans;
	WARE_Ware *ww;

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_WAREHOUSE);
	ASSERT(WITHIN(p_person->Genus.Person->Ware, 1, WARE_ware_upto - 1));

	ww = &WARE_ware[p_person->Genus.Person->Ware];

	//
	// Set the MAV module to use this warehouses' MAV array.
	//

	old_mav       = MAV_nav;
	old_mav_pitch = MAV_nav_pitch;

	MAV_nav       = &WARE_nav[ww->nav];
	MAV_nav_pitch =  ww->nav_pitch;

	//
	// Which warehouse entrance shall we mav to?
	//

	for (i = 0; i < ww->door_upto; i++)
	{
		dx = ww->door[i].in_x - (p_person->WorldPos.X >> 16);
		dz = ww->door[i].in_z - (p_person->WorldPos.Z >> 16);

		dist = abs(dx) + abs(dz);

		SLONG height_in = WARE_calc_height_at(
							p_person->Genus.Person->Ware,
							(ww->door[i].in_x << 8) + 0x80,
							(ww->door[i].in_z << 8) + 0x80);

		SLONG height_out = PAP_calc_map_height_at(
							(ww->door[i].out_x << 8) + 0x80,
							(ww->door[i].out_z << 8) + 0x80);

		if (abs(height_out - height_in) < 0x80)
		{
			if (best_dist > dist)
			{
				best_dist = dist;
				best_door = i;
			}
		}
	}

	if (best_dist == 0)
	{
		//
		// Already at the exit square. Walk out of the warehouse.
		//

		ans.dest_x = ww->door[best_door].out_x;
		ans.dest_z = ww->door[best_door].out_z;
		ans.action = MAV_ACTION_GOTO;
		ans.dir    = 0;
	}
	else
	{
		//
		// Put the start and destination into warehouse space.
		//

		start_x = (p_person->WorldPos.X >> 16);
		start_z = (p_person->WorldPos.Z >> 16);

		ASSERT(WARE_in_floorplan(p_person->Genus.Person->Ware, start_x, start_z));

		start_x -= ww->minx;
		start_z -= ww->minz;

		dest_x = ww->door[best_door].in_x - ww->minx;
		dest_z = ww->door[best_door].in_z - ww->minz;

		//
		// Do the mav.
		//

		ans = MAV_do(
				start_x,
				start_z,
				dest_x,
				dest_z,
				caps);

		ans.dest_x += ww->minx;
		ans.dest_z += ww->minz;
	}
	
	//
	// Restore the old MAV array.
	//

	MAV_nav       = old_mav;
	MAV_nav_pitch = old_mav_pitch;

	return ans;
}


SLONG WARE_in_floorplan(UBYTE ware, UBYTE x, UBYTE z)
{
	ASSERT(WITHIN(ware, 1, WARE_ware_upto - 1));

	WARE_Ware *ww = &WARE_ware[ware];

	if (WITHIN(x, ww->minx, ww->maxx) &&
		WITHIN(z, ww->minz, ww->maxz))
	{
		if (PAP_2HI(x,z).Flags & PAP_FLAG_HIDDEN)
		{
			//
			// Inside the bounding box of the warehouse and in a building too...
			//

			return TRUE;
		}
	}

	return FALSE;
}

#ifndef PSX
#ifndef TARGET_DC
void WARE_debug(void)
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG x1, y1, z1;
	SLONG x2, y2, z2;

	WARE_Ware *ww;

	if (!Keys[KB_T])
	{
		return;
	}

	for (i = 1; i < WARE_ware_upto; i++)
	{
		ww = &WARE_ware[i];

		//
		// Draw the doors.
		//

		for (j = 0; j < ww->door_upto; j++)
		{
			x1 = ww->door[j].in_x << 8;
			z1 = ww->door[j].in_z << 8;

			x2 = ww->door[j].out_x << 8;
			z2 = ww->door[j].out_z << 8;

			x1 += 0x80;
			z1 += 0x80;

			x2 += 0x80;
			z2 += 0x80;

			y1 = PAP_calc_height_at(x1, z1);
			y2 = PAP_calc_height_at(x2, z2);

			AENG_world_line(
				x1, y1, z1,
				00,
				0xffffff,
				x2, y2, z2,
				16,
				0x00ff00,
				TRUE);
		}

		//
		// Draw the MAV info.
		//

		{
			SLONG x;
			SLONG z;

			SLONG x1;
			SLONG y1;
			SLONG z1;
			SLONG x2;
			SLONG y2;
			SLONG z2;

			SLONG mx;
			SLONG mz;

			SLONG dx;
			SLONG dz;

			SLONG lx;
			SLONG lz;

			SLONG index;

			MAV_Opt *mo;

			struct
			{
				SLONG dx;
				SLONG dz;

			} order[4] =
			{
				{-1, 0},
				{+1, 0},
				{0, -1},
				{0, +1}
			};

			ULONG colour[7] =
			{
				0x00ff0000,
				0x0000ff00,
				0x000000ff,
				0x00ffff00,
				0x00ff00ff,
				0x0000ffff,
				0x00ffaa88
			};

			for (x = ww->minx; x <= ww->maxx; x++)
			for (z = ww->minz; z <= ww->maxz; z++)
			{
				//
				// Draw a blue cross at the height we think the square is at.
				// 

				x1 = x + 0 << 8;
				z1 = z + 0 << 8;
				x2 = x + 1 << 8;
				z2 = z + 1 << 8;

				y1 = 
				y2 = WARE_calc_height_at(i, (x << 8) + 0x80, (z << 8) + 0x80);

				AENG_world_line(
					x1, y1, z1, 4, 0x00000077,
					x2, y2, z2, 4, 0x00000077,
					TRUE);

				AENG_world_line(
					x2, y1, z1, 4, 0x00000077,
					x1, y2, z2, 4, 0x00000077,
					TRUE);

				//
				// Draw the options for leaving this square.
				//

				index = (x - ww->minx) * ww->nav_pitch + (z - ww->minz);

				ASSERT(WITHIN(ww->nav + index, 0, WARE_nav_upto - 1));

				mo = &MAV_opt[WARE_nav[ww->nav + index]];

				mx = x1 + x2 >> 1;
				mz = z1 + z2 >> 1;

				for (j = 0; j < 4; j++)
				{
					dx = order[j].dx;
					dz = order[j].dz;

					lx  = mx + dx * 96;
					lz  = mz + dz * 96;

					lx +=  dz * (16 * 3);
					lz += -dx * (16 * 3);

					for (k = 0; k < 8; k++)
					{
						if (mo->opt[j] & (1 << k))
						{
							AENG_world_line(
								mx, y1, mz, 0, 0,
								lx, y2, lz, 9, colour[k],
								TRUE);
						}

						lx += -dz * 16;
						lz += +dx * 16;
					}
				}
			}
		}
	}
}
#endif
#endif

SLONG WARE_inside(UBYTE ware, SLONG x, SLONG y, SLONG z)
{
	SLONG index;
	SLONG height;

	WARE_Ware *ww;

	ASSERT(WITHIN(ware, 1, WARE_ware_upto -1 ));

	ww = &WARE_ware[ware];

	//
	// Inside our floorplan?
	//

	x >>= 8;
	z >>= 8;

	if (!WITHIN(x, ww->minx, ww->maxx) ||
		!WITHIN(z, ww->minz, ww->maxz) ||
		!(PAP_2HI(x,z).Flags & PAP_FLAG_HIDDEN))
	{
		//
		// Outside the floorplan of the building.
		//

		return TRUE;
	}

	//
	// Go into warehouse space.
	//

	x -= ww->minx;
	z -= ww->minz;

	index = x * ww->nav_pitch + z;

	ASSERT(WITHIN(ww->height + index, 0, WARE_height_upto - 1));

	height = WARE_height[ww->height + index] << 6;

	return y < height;
}



SLONG WARE_which_contains(UBYTE x, UBYTE z)
{
	SLONG i;

	for (i = 1; i < WARE_ware_upto; i++)
	{
		if (WARE_in_floorplan(i,x,z))
		{
			return i;
		}
	}

	return NULL;
}


UBYTE WARE_get_caps(
		UBYTE ware,
		UBYTE x,
		UBYTE z,
		UBYTE dir)
{
	SLONG index;
	SLONG mo_index;

	WARE_Ware *ww;

	ASSERT(WITHIN(ware, 1, WARE_ware_upto -1 ));

	ww = &WARE_ware[ware];

	if (!WITHIN(x, ww->minx, ww->maxx) ||
		!WITHIN(z, ww->minz, ww->maxz))
	{
		//
		// Outside the floorplan of the building.
		//

		return 0;
	}

	//
	// Go into warehouse space.
	//

	x -= ww->minx;
	z -= ww->minz;

	index = x * ww->nav_pitch + z;

	ASSERT(WITHIN(ww->nav + index, 0, WARE_nav_upto - 1));

	mo_index = WARE_nav[ww->nav + index];

	ASSERT(WITHIN(mo_index, 0, MAV_opt_upto - 1));

	return MAV_opt[mo_index].opt[dir];
}
