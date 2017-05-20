//
// Another navigation system.
//

#include "game.h"
#include "mav.h"
#ifndef	PSX
#include "c:\fallen\ddengine\headers\aeng.h"
#endif
#include "pap.h"
#include "supermap.h"
#include "walkable.h"
#include "memory.h"
#include "ware.h"
#include "ob.h"

//
// A prototype here never hurt anyone
//

//
// The height in stories of each square.
//

//MAV_height_workaround *MAV_height;


//
// The navigation info. Each value in the nav array is an index
// into this array.
//

MAV_Opt *MAV_opt;
SLONG    MAV_opt_upto;


//
// How you can move out of each square.
//

UWORD *MAV_nav;
SLONG  MAV_nav_pitch = 128;




void MAV_init()
{
	SLONG i;

	MAV_nav_pitch = 128;
	MAV_opt_upto  = 0;

	//
	// Create common MAV_opts first. We need these for the inside2 nav
	// so macke sure they are all here (so we won't have to dynamically
	// allocate them).
	//

	for (i = 0; i < 16; i++)
	{
		//
		// If you change this bit of code, make sure you change the corresponding
		// bit of code in INSIDE2_mav_nav_calc().
		//

		MAV_opt[i].opt[0] = (i & 1) ? 0 : MAV_CAPS_GOTO;
		MAV_opt[i].opt[1] = (i & 2) ? 0 : MAV_CAPS_GOTO;
		MAV_opt[i].opt[2] = (i & 4) ? 0 : MAV_CAPS_GOTO;
		MAV_opt[i].opt[3] = (i & 8) ? 0 : MAV_CAPS_GOTO;
	}

	MAV_opt_upto = 16;
}

static void StoreMavOpts(SLONG x, SLONG z, UBYTE* opt)
{
	for (SLONG ii = 0; ii < MAV_opt_upto; ii++)
	{
		if ((MAV_opt[ii].opt[0] == opt[0]) &&
			(MAV_opt[ii].opt[1] == opt[1]) &&
			(MAV_opt[ii].opt[2] == opt[2]) &&
			(MAV_opt[ii].opt[3] == opt[3]))
		{
			SET_MAV_NAV(x, z, ii);
			return;
		}
	}

	if (MAV_opt_upto == MAV_MAX_OPTS)
	{
		TRACE("Run out of MAV opts!\n");
		ASSERT(0);
	}
	else
	{
		MAV_opt[MAV_opt_upto].opt[0] = opt[0];
		MAV_opt[MAV_opt_upto].opt[1] = opt[1];
		MAV_opt[MAV_opt_upto].opt[2] = opt[2];
		MAV_opt[MAV_opt_upto].opt[3] = opt[3];

		SET_MAV_NAV(x, z, MAV_opt_upto);

		MAV_opt_upto++;
	}
}

//
// Calculates the MAV_height array.
//
#ifndef	PSX
void MAV_calc_height_array(SLONG ignore_warehouses)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG index;
	SLONG face;
	SLONG faceheight;

	SLONG height;
	SLONG walk;

	DBuilding *db;
	PrimFace4 *p_f4;
	RoofFace4 *rf;
	DWalkable *dw;
	PAP_Hi    *ph;

	//
	// Work out the MAV_height array.
	//

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		//
		// The middle of this mapsquare.
		//

		mx = x << 8;
		mz = z << 8;

		mx += 0x80;
		mz += 0x80;

		if ((PAP_2HI(x,z).Flags & PAP_FLAG_ROOF_EXISTS))
		{
		}
		else
		if ((PAP_2HI(x,z).Flags & PAP_FLAG_HIDDEN) && !ignore_warehouses)
		{
			MAVHEIGHT(x,z) = -127;
		}
		else
		{
			height = PAP_calc_height_at(mx, mz);

			//
			// Convert to the SBYTE coordinate system in the MAV_height array.
			//

			height /= 0x40;

			SATURATE(height, -127, +127);

			MAVHEIGHT(x,z) = height;
		}
	}

	//
	// Use the walkable faces to set the height of the hidden squares.
	//

	for (i = 1; i < next_dbuilding; i++)
	{
		db = &dbuildings[i];

		if (db->Type == BUILDING_TYPE_WAREHOUSE)
		{
			if (ignore_warehouses)
			{
				//
				// Ignore these roof faces.
				//

				continue;
			}
		}

		for (walk = db->Walkable; walk; walk = dw->Next)
		{
			dw = &dwalkables[walk];

			for (j = dw->StartFace4; j < dw->EndFace4; j++)
			{
				rf = &roof_faces4[j];

				rf->DrawFlags &= ~RFACE_FLAG_NODRAW;	// This flag is set later... but shouldn't be set here.

				if (db->Type == BUILDING_TYPE_WAREHOUSE)
				{
					//
					// Mark this mapsquare as being inside a warehouse (top bit!)
					//

					ph = &PAP_2HI(rf->RX&127,rf->RZ&127);

					ph->Texture |= 1 << 15;
				}

				height  = rf->Y;
				height /= 0x40;

				SATURATE(height, -127, +127);

				if (MAVHEIGHT(rf->RX&127,rf->RZ&127) < height)
				{
					MAVHEIGHT(rf->RX&127,rf->RZ&127) = height;
				}
			}
		}
	}

	//
	// Nowadays there are buildings without roofs.
	// 

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		if (MAVHEIGHT(x,z) <= -127)
		{
			if (PAP_2HI(x,z).Flags & PAP_FLAG_HIDDEN)
			{
				MAVHEIGHT(x,z) = 127;

				PAP_2HI(x,z).Flags |= PAP_FLAG_NOGO;
			}
		}
	}

	/*

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		face = PAP_2LO(x,z).Walkable;

		while(face)
		{
			if (face > 0)
			{
				p_f4 = &prim_faces4[face];

				//
				// ASSUME FACES DONT SPAN MORE THAN ONE MAPSQUARE!
				//

				mx = prim_points[p_f4->Points[0]].X + prim_points[p_f4->Points[1]].X + prim_points[p_f4->Points[2]].X + prim_points[p_f4->Points[3]].X >> 10;
				mz = prim_points[p_f4->Points[0]].Z + prim_points[p_f4->Points[1]].Z + prim_points[p_f4->Points[2]].Z + prim_points[p_f4->Points[3]].Z >> 10;

				if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
				{
					if(calc_height_on_face((mx << 8) + 0x80, (mz << 8) + 0x80, face,&height))
					{

						//
						// Convert to the SBYTE coordinate system in the MAV_height array.
						//

						height /= 0x40;

						SATURATE(height, -127, +127);

						if (MAV_height[mx][mz] < height)
						{
							MAV_height[mx][mz] = height;
						}
					}

				}

				face = p_f4->WALKABLE;
			}
			else
			{
				rf = &roof_faces4[-face];

				//
				// This is a cunning roof face.
				//

				height = rf->Y / 0x40;

				SATURATE(height, -127, +127);

				if (MAV_height[rf->X][rf->Z] < height)
				{
					MAV_height[rf->X][rf->Z] = height;
				}

				face = rf->Next;
			}
		}
	}

	*/
}
#endif

//
// Makes sure that nobody mavigates into the given square by walking
// into it.
//
#ifndef	PSX
void MAV_turn_off_square(
		SLONG x,
		SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

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

	UBYTE opt[4];

	SLONG i;
	SLONG j;

	SLONG mx;
	SLONG mz;

	for (i = 0; i < 4; i++)
	{
		mx = x - order[i].dx;
		mz = z - order[i].dz;

		if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
			WITHIN(mz, 0, MAP_HEIGHT - 1))
		{
			ASSERT(WITHIN(MAV_NAV(mx,mz), 0, MAV_opt_upto - 1));

			opt[0] = MAV_opt[MAV_NAV(mx,mz)].opt[0];
			opt[1] = MAV_opt[MAV_NAV(mx,mz)].opt[1];
			opt[2] = MAV_opt[MAV_NAV(mx,mz)].opt[2];
			opt[3] = MAV_opt[MAV_NAV(mx,mz)].opt[3];

			opt[i] &= ~MAV_CAPS_GOTO;

			StoreMavOpts(mx, mz, opt);
		}
	}
}
#endif
#ifndef PSX

//
// Makes sure nobody can go into this square in any way.
// 

void MAV_turn_off_whole_square(
		SLONG x,
		SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

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

	UBYTE opt[4];

	SLONG i;
	SLONG j;

	SLONG mx;
	SLONG mz;

	for (i = 0; i < 4; i++)
	{
		mx = x - order[i].dx;
		mz = z - order[i].dz;

		if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
			WITHIN(mz, 0, MAP_HEIGHT - 1))
		{
			ASSERT(WITHIN(MAV_NAV(mx,mz), 0, MAV_opt_upto - 1));

			opt[0] = MAV_opt[MAV_NAV(mx,mz)].opt[0];
			opt[1] = MAV_opt[MAV_NAV(mx,mz)].opt[1];
			opt[2] = MAV_opt[MAV_NAV(mx,mz)].opt[2];
			opt[3] = MAV_opt[MAV_NAV(mx,mz)].opt[3];

			opt[i] = 0;

			StoreMavOpts(mx, mz, opt);
		}
	}
}

//
// Makes sure nobody can go into this square in any way.
// 

void MAV_turn_off_whole_square_car(
		SLONG x,
		SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

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

	UBYTE opt[4];

	SLONG i;
	SLONG j;

	SLONG mx;
	SLONG mz;

	for (i = 0; i < 4; i++)
	{
		mx = x - order[i].dx;
		mz = z - order[i].dz;

		if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
			WITHIN(mz, 0, MAP_HEIGHT - 1))
		{
			UBYTE	caropt = MAV_CAR(mx, mz);

			caropt &= ~(1 << i);

			SET_MAV_CAR(mx, mz, caropt);
		}
	}
}

//
// cut off a facet from the car mav
//

void MAV_remove_facet_car(SLONG x1, SLONG z1, SLONG x2, SLONG z2)
{
	if (x1 == x2)
	{
		if (z1 > z2)	SWAP(z1,z2);

		for (SLONG z = z1; z < z2; z++)
		{
			UBYTE	caropt;

			if (x1 > 0)
			{
				caropt = MAV_CAR(x1 - 1, z);
				caropt &= ~(1 << MAV_DIR_XL);
				SET_MAV_CAR(x1 - 1, z, caropt);
			}
			if (x1 < MAP_WIDTH)
			{
				caropt = MAV_CAR(x1, z);
				caropt &= ~(1 << MAV_DIR_XS);
				SET_MAV_CAR(x1, z, caropt);
			}
		}
	}
	else
	{
		ASSERT(z1 == z2);
		if (x1 > x2)	SWAP(x1,x2);

		for (SLONG x = x1; x < x2; x++)
		{
			UBYTE	caropt;

			if (z1 > 0)
			{
				caropt = MAV_CAR(x, z1 - 1);
				caropt &= ~(1 << MAV_DIR_ZL);
				SET_MAV_CAR(x, z1 - 1, caropt);
			}
			if (z1 < MAP_HEIGHT)
			{
				caropt = MAV_CAR(x, z1);
				caropt &= ~(1 << MAV_DIR_ZS);
				SET_MAV_CAR(x, z1, caropt);
			}
		}
	}
}

#endif

//
// Turns off movement in the given direction from the square.
//

void MAV_turn_movement_off(UBYTE mx, UBYTE mz, UBYTE dir)
{
	SLONG   j;
	SLONG   mo_index;
	MAV_Opt mo;

	mo_index = MAV_NAV(mx,mz);

	ASSERT(WITHIN(mo_index, 0, MAV_opt_upto - 1));

	mo = MAV_opt[mo_index];

	mo.opt[dir] &= ~MAV_CAPS_GOTO;

	StoreMavOpts(mx, mz, mo.opt);
}


//
// Turns on movement in the given direction from the square.
//

void MAV_turn_movement_on(UBYTE mx, UBYTE mz, UBYTE dir)
{
	SLONG   j;
	SLONG   mo_index;
	MAV_Opt mo;

	mo_index = MAV_NAV(mx,mz);

	ASSERT(WITHIN(mo_index, 0, MAV_opt_upto - 1));

	mo = MAV_opt[mo_index];

	mo.opt[dir] = MAV_CAPS_GOTO;

	StoreMavOpts(mx, mz, mo.opt);
}



#ifndef PSX
#ifndef TARGET_DC
void MAV_precalculate()
{
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;
	
	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG dx;
	SLONG dz;

	SLONG tx;
	SLONG tz;

	SLONG rx;
	SLONG rz;

	SLONG dh;

	SLONG useangle;
	SLONG matrix[4];
	SLONG ladder;

	SLONG sin_yaw;
	SLONG cos_yaw;

	SLONG both_ground;

	OB_Info *oi;

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

	UBYTE opt[4];

	//
	// Calculates the MAV_height array including warehouses.
	//

	MAV_calc_height_array(FALSE);

	//
	// Make the staircase prims change the MAV_height array 
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		for (oi = OB_find(x,z); oi->prim; oi++)
		{
			if (oi->prim == 41)
			{
				//
				// The step prim!
				//

				if (!(oi->flags & OB_FLAG_WAREHOUSE))
				{
					//
					// Find which mapsquare the middle of this prim is over.
					//

					PrimInfo *pi = get_prim_info(oi->prim);

					SLONG mx = pi->minx + pi->maxx >> 1;
					SLONG mz = pi->minz + pi->maxz >> 1;

					SLONG matrix[4];
					SLONG useangle;

					SLONG sin_yaw;
					SLONG cos_yaw;

					SLONG rx;
					SLONG rz;

					SLONG sx;
					SLONG sz;

					useangle  = -oi->yaw;
					useangle &=  2047;

					sin_yaw = SIN(useangle);
					cos_yaw = COS(useangle);

					matrix[0] =  cos_yaw;
					matrix[1] = -sin_yaw;
					matrix[2] =  sin_yaw;
					matrix[3] =  cos_yaw;

					rx = MUL64(mx, matrix[0]) + MUL64(mz, matrix[1]);
					rz = MUL64(mx, matrix[2]) + MUL64(mz, matrix[3]);

					rx += oi->x;
					rz += oi->z;

					rx >>= 8;
					rz >>= 8;

					MAVHEIGHT(rx,rz) = oi->y + 0x40 >> 6;
				}
			}
		}
	}


	//
	// Work out the nav for each square.
	//

	for (x = 0; x < PAP_SIZE_HI;  x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		//
		// Look for a nearby ladder.
		//

		ladder = find_nearby_ladder_colvect_radius(
					(x << 8) + 0x80,
					(z << 8) + 0x80,
					0x100);

		UBYTE	caropts = 0;	// car opts

		for (i = 0; i < 4; i++)
		{
			opt[i] = 0;

			dx = order[i].dx;
			dz = order[i].dz;

			tx = x + dx;
			tz = z + dz;

			if (WITHIN(tx, 0, PAP_SIZE_HI - 1) &&
				WITHIN(tz, 0, PAP_SIZE_HI - 1))
			{
				//
				// Is one of the squares on a building?
				//

				if (!(PAP_2HI( x, z).Flags & PAP_FLAG_HIDDEN) &&
					!(PAP_2HI(tx,tz).Flags & PAP_FLAG_HIDDEN))
				{
					both_ground = TRUE;
				}
				else
				{
					both_ground = FALSE;
				}

				//
				// Can we walk from (x,z) to (tx,tz)?
				//

				dh = MAVHEIGHT(tx,tz) - MAVHEIGHT(x,z);

				if ((!both_ground && abs(dh) > 1) || (abs(dh) > 2))
				{
					//
					// There are at different heights, so you cant
					// just walk between the two squares.
					//

					if (dh < 0)
					{
						//
						// There might be a wall or fence in the way.
						//

						x1 = ( x << 8) + 0x80;
						z1 = ( z << 8) + 0x80;
						x2 = (tx << 8) + 0x80;
						z2 = (tz << 8) + 0x80;

						y1 = PAP_calc_map_height_at(x1,z1) + 0x50;
						y2 = PAP_calc_map_height_at(x2,z2) + 0x50;

						y = MAX(y1,y2);

						if (there_is_a_los(
								x1, y, z1,
								x2, y, z2,
								LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
						{
							//
							// We can always fall down becuase there is nothing in the way.
							// 

							opt[i] |= MAV_CAPS_FALL_OFF;
						}
						else
						{
							//
							// If there is a fence in the way, then we can scale the fence.
							//

							DFacet *df = &dfacets[los_failure_dfacet];

							if (df->FacetType == STOREY_TYPE_FENCE_FLAT)
							{
								if (df->FacetFlags & FACET_FLAG_UNCLIMBABLE)
								{
									//
									// Unclimbable fence.
									//
								}
								else
								{
									//
									// We can scale this fence.
									//

									opt[i] |= MAV_CAPS_CLIMB_OVER;
								}
							}
						}

						if (there_is_a_los(x1,y,z1, x2,y,z2, 
							LOS_FLAG_IGNORE_PRIMS | LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG | LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
						{
							caropts |= 1 << i;
						}
					}
					else
					{
						if (WITHIN(dh, 2, 4))
						{
							//
							// We can pull ourselves up as long as there isn't a fence in the way.
							//

							SLONG fx1;
							SLONG fz1;
							SLONG fx2;
							SLONG fz2;

							fx1 = (x << 8) + 0x80 + (dx << 7) - (dz << 7);
							fz1 = (z << 8) + 0x80 + (dz << 7) + (dx << 7);

							fx2 = (x << 8) + 0x80 + (dx << 7) + (dz << 7);
							fz2 = (z << 8) + 0x80 + (dz << 7) - (dx << 7);

							if (does_fence_lie_along_line(
									fx1, fz1,
									fx2, fz2))
							{
								//
								// You can't pull yourself up if there is a fence in the way.
								//
							}
							else
							{
								opt[i] |= MAV_CAPS_PULLUP;
							}
						}

						if (ladder)
						{
							DFacet *df_ladder;

							ASSERT(WITHIN(ladder, 1, next_dfacet - 1));

							df_ladder = &dfacets[ladder];

							ASSERT(df_ladder->FacetType == STOREY_TYPE_LADDER);

							//
							// There is a ladder- can we climb up it in this direction?
							//

							x1 = ( x << 8) + 0x80;
							z1 = ( z << 8) + 0x80;
							x2 = (tx << 8) + 0x80;
							z2 = (tz << 8) + 0x80;

							if (two4_line_intersection(
									x1, z1,
									x2, z2,
									df_ladder->x[0] << 8, df_ladder->z[0] << 8,
									df_ladder->x[1] << 8, df_ladder->z[1] << 8))
							{
								//
								// Make sure the ladder reaches to the bottom.
								//

								if (abs(df_ladder->Y[0] - PAP_calc_map_height_at(x1,z1)) <= 0x50)
								{
									opt[i] |= MAV_CAPS_LADDER_UP;
								}
							}
						}
					}
				}
				else
				{
					//
					// There might be a wall or fence in the way.
					//

					x1 = ( x << 8) + 0x80;
					z1 = ( z << 8) + 0x80;
					x2 = (tx << 8) + 0x80;
					z2 = (tz << 8) + 0x80;

					y1 = PAP_calc_map_height_at(x1, z1) + 0x50;
					y2 = PAP_calc_map_height_at(x2, z2) + 0x50;

					y = MAX(y1,y2);

					if (there_is_a_los(
							x1, y, z1,
							x2, y, z2,
							LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
					{
						//
						// Nothing in the way...
						//

						opt[i] |= MAV_CAPS_GOTO;
					}
					else
					{
						//
						// If there is a fence in the way, then we can scale the fence.
						//

						DFacet *df = &dfacets[los_failure_dfacet];

						if (df->FacetType == STOREY_TYPE_FENCE_FLAT)
						{
							if (df->FacetFlags & FACET_FLAG_UNCLIMBABLE)
							{
								//
								// Unclimbable fence.
								//
							}
							else
							{
								//
								// We can scale this fence.
								//

								opt[i] |= MAV_CAPS_CLIMB_OVER;
							}
						}
					}

					if (there_is_a_los(x1,y,z1, x2,y,z2, 
						LOS_FLAG_IGNORE_PRIMS | LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG | LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
					{
						caropts |= 1 << i;
					}
				}

				if (!(opt[i] & MAV_CAPS_GOTO) &&
					!(opt[i] & MAV_CAPS_CLIMB_OVER))
				{
					//
					// Now what about jumping one block?
					//

					tx += dx;
					tz += dz;

					if (WITHIN(tx, 0, MAP_WIDTH  - 1) &&
						WITHIN(tz, 0, MAP_HEIGHT - 1))
					{
						dh = MAVHEIGHT(tx,tz) - MAVHEIGHT(x,z);

						//
						// Can we jump there?
						//

						x1 = ( x << 8) + 0x80;
						z1 = ( z << 8) + 0x80;
						x2 = (tx << 8) + 0x80;
						z2 = (tz << 8) + 0x80;

						y1 = (MAVHEIGHT( x, z) << 6) + 0xa0;
						y2 = (MAVHEIGHT(tx,tz) << 6) + 0xa0;

						if (there_is_a_los(
								x1, y1, z1,
								x2, y2, z2,
								LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
						{
							if (dh < 2)
							{
								opt[i] |= MAV_CAPS_JUMP;
							}
							else
							{
								if (WITHIN(dh, 2, 5))
								{
									opt[i] |= MAV_CAPS_JUMPPULL;
								}
							}
						}

						//
						// What about jumping two blocks?
						//

						tx += dx;
						tz += dz;

						if (WITHIN(tx, 0, MAP_WIDTH  - 1) &&
							WITHIN(tz, 0, MAP_HEIGHT - 1))
						{
							dh = MAVHEIGHT(tx,tz) - MAVHEIGHT(x,z);

							if (dh > 4)
							{
								//
								// Can't make this jump
								//
							}
							else
							{
								if (dh > -6)
								{
									//
									// Can we jump there?
									//

									x1 = ( x << 8) + 0x80;
									z1 = ( z << 8) + 0x80;
									x2 = (tx << 8) + 0x80;
									z2 = (tz << 8) + 0x80;

									y1 = (MAVHEIGHT( x, z) << 6) + 0xa0;
									y2 = (MAVHEIGHT(tx,tz) << 6) + 0xa0;

									if (there_is_a_los(
											x1, y1, z1,
											x2, y2, z2,
											LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
									{
										opt[i] |= MAV_CAPS_JUMPPULL2;
									}
								}
							}
						}
					}
				}
			}
		}

		StoreMavOpts(x, z, opt);
		SET_MAV_CAR(x, z, caropts);
		SET_MAV_SPARE(x, z, 3);
	}

	//
	// A hack for the staircase prim and the skylights.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		for (oi = OB_find(x,z); oi->prim; oi++)
		{
			if (oi->prim == 41)
			{
				//
				// The step prim!
				//

				if (!(oi->flags & OB_FLAG_WAREHOUSE))
				{
					//
					// Find which mapsquare the middle of this prim is over.
					//

					PrimInfo *pi = get_prim_info(oi->prim);

					SLONG mx = pi->minx + pi->maxx >> 1;
					SLONG mz = pi->minz + pi->maxz >> 1;

					SLONG matrix[4];
					SLONG useangle;

					SLONG sin_yaw;
					SLONG cos_yaw;

					SLONG rx;
					SLONG rz;

					SLONG sx;
					SLONG sz;

					useangle  = -oi->yaw;
					useangle &=  2047;

					sin_yaw = SIN(useangle);
					cos_yaw = COS(useangle);

					matrix[0] =  cos_yaw;
					matrix[1] = -sin_yaw;
					matrix[2] =  sin_yaw;
					matrix[3] =  cos_yaw;

					rx = MUL64(mx, matrix[0]) + MUL64(mz, matrix[1]);
					rz = MUL64(mx, matrix[2]) + MUL64(mz, matrix[3]);

					rx += oi->x;
					rz += oi->z;

					rx >>= 8;
					rz >>= 8;

					if (oi->yaw < 256 || oi->yaw > 1792 || WITHIN(oi->yaw, 768, 1280))
					{
						if (MAVHEIGHT(rx,rz) == MAVHEIGHT(rx-1,rz))
						{
							//
							// Walking left-right on a wide staircase.
							//
						}
						else
						{
							MAV_turn_movement_off(rx,     rz, MAV_DIR_XS);
							MAV_turn_movement_off(rx - 1, rz, MAV_DIR_XL);
						}

						if (MAVHEIGHT(rx,rz) == MAVHEIGHT(rx+1,rz))
						{
							//
							// Walking left-right on a wide staircase.
							//
						}
						else
						{
							MAV_turn_movement_off(rx,     rz, MAV_DIR_XL);
							MAV_turn_movement_off(rx + 1, rz, MAV_DIR_XS);
						}

						if (!WITHIN(oi->yaw, 768, 1280))
						{
							if (MAVHEIGHT(rx,rz+1) <= MAVHEIGHT(rx,rz) + 3)
							{
								MAV_turn_movement_on(rx,rz,   MAV_DIR_ZL);
								MAV_turn_movement_on(rx,rz+1, MAV_DIR_ZS);
							}
						}
						else
						{
							if (MAVHEIGHT(rx,rz-1) <= MAVHEIGHT(rx,rz) + 3)
							{
								MAV_turn_movement_on(rx,rz,   MAV_DIR_ZS);
								MAV_turn_movement_on(rx,rz-1, MAV_DIR_ZL);
							}
						}
					}					     
					else				     
					{
						if (MAVHEIGHT(rx, rz) == MAVHEIGHT(rx, rz-1))
						{
							//
							// Walking across a wide staircase.
							//
						}
						else
						{
							MAV_turn_movement_off(rx, rz,     MAV_DIR_ZS);
							MAV_turn_movement_off(rx, rz - 1, MAV_DIR_ZL);
						}

						if (MAVHEIGHT(rx, rz) == MAVHEIGHT(rx, rz+1))
						{
							//
							// Walking across a wide staircase.
							//
						}
						else
						{
							MAV_turn_movement_off(rx, rz,     MAV_DIR_ZL);
							MAV_turn_movement_off(rx, rz + 1, MAV_DIR_ZS);
						}

						if (!WITHIN(oi->yaw, 256, 768))
						{
							if (MAVHEIGHT(rx-1,rz) <= MAVHEIGHT(rx,rz) + 3)
							{
								MAV_turn_movement_on(rx,  rz, MAV_DIR_XS);
								MAV_turn_movement_on(rx-1,rz, MAV_DIR_XL);
							}
						}
						else
						{
							if (MAVHEIGHT(rx+1,rz) <= MAVHEIGHT(rx,rz) + 3)
							{
								MAV_turn_movement_on(rx,   rz, MAV_DIR_XL);
								MAV_turn_movement_on(rx+1, rz, MAV_DIR_XS);
							}
						}
					}

					WALKABLE_remove_rface(rx,rz);
				}
			}
			else
			if (oi->prim == 226)
			{
				//
				// This is a skylight. Remove the two roof faces that
				// are covered by the skylight.
				//

				SLONG useangle;

				useangle  = oi->yaw + 1024;
				useangle &= 2047;

				SLONG mx = oi->x >> 8;
				SLONG mz = oi->z >> 8;

				SLONG rx = mx + SIGN(SIN(useangle) >> 14);
				SLONG rz = mz + SIGN(COS(useangle) >> 14);

				WALKABLE_remove_rface(mx,mz);
				WALKABLE_remove_rface(rx,rz);
			}
			else
			if (oi->prim == 227)
			{
				//
				// This is the large skylight. Remove the roof faces that
				// are covered by it.
				//

				SLONG i;
				SLONG j;
				SLONG useangle;

				useangle  = oi->yaw + 1024;
				useangle &= 2047;

				SLONG mx = oi->x;
				SLONG mz = oi->z;

				SLONG rx = SIN(useangle) >> 8;
				SLONG rz = COS(useangle) >> 8;

				SLONG sx;
				SLONG sz;

				for (i = -1; i <= 1; i += 1)
				for (j = -1; j <= 1; j += 2)
				{
					sx = mx + rx * i - (rz * j >> 1);
					sz = mz + rz * i + (rx * j >> 1);

					WALKABLE_remove_rface(
						sx >> 8,
						sz >> 8);
				}
			}
			else
			if (oi->prim == 133)
			{
				//
				// This is the rotating UCPD sign.
				//

				MAV_turn_off_whole_square(oi->x >> 8, oi->z >> 8);
			}
		}
	}

	{
		extern SLONG PAP_on_slope(SLONG x,SLONG z,SLONG *angle);

		//
		// Take all slippy squares out of the mav.
		//

		SLONG mx;
		SLONG mz;

		SLONG angle;

		for (mx = 0; mx < PAP_SIZE_HI; mx++)
		for (mz = 0; mz < PAP_SIZE_HI; mz++)
		{
			if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
			{
				//
				// These squares aren't slippy.
				//

				continue;
			}

			if (PAP_on_slope((mx << 8) + 0x40, (mz << 8) + 0x40, &angle) > 100 ||
//				PAP_on_slope((mx << 8) + 0xc0, (mz << 8) + 0x40, &angle) > 50 ||
//				PAP_on_slope((mx << 8) + 0x40, (mz << 8) + 0xc0, &angle) > 50 ||
				PAP_on_slope((mx << 8) + 0xc0, (mz << 8) + 0xc0, &angle) > 100)
			{
				MAV_turn_off_whole_square(mx,mz);
			}
		}
	}

	// remove NOGO sqaures from the car map
	{
		SLONG	mx;
		SLONG	mz;

		SLONG	angle;

		for (mx = 0; mx < PAP_SIZE_HI; mx++)
		for (mz = 0; mz < PAP_SIZE_HI; mz++)
		{
			if (PAP_2HI(mx,mz).Flags & PAP_FLAG_NOGO)
			{
				MAV_turn_off_whole_square_car(mx,mz);
			}
		}
	}

	/*

	//
	// Look for prims that block off certain squares
	//

	THING_INDEX t_index;

	Thing *p_thing;
	t_index = PRIMARY_USED;

	while(t_index)
	{
		p_thing = TO_THING(t_index);

		if (p_thing->Class == CLASS_FURNITURE)
		{
			if (p_thing->Draw.Mesh->ObjectId  < 5               ||
				p_thing->Draw.Mesh->ObjectId == 55              ||
				p_thing->Draw.Mesh->ObjectId == 40              ||
				p_thing->Draw.Mesh->ObjectId == 41              ||
				p_thing->Draw.Mesh->ObjectId == PRIM_OBJ_CANOPY ||
				p_thing->Draw.Mesh->ObjectId == PRIM_OBJ_SIGN)
			{
				//
				// Ignore these objects.
				//
			}
			else
			{
				//
				// The rotation matrix of the prim.
				//

				useangle  = -p_thing->Draw.Mesh->Angle;
				useangle &=  2047;

				sin_yaw = SIN(useangle);
				cos_yaw = COS(useangle);

				matrix[0] =  cos_yaw;
				matrix[1] =  sin_yaw;
				matrix[2] = -sin_yaw;
				matrix[3] =  cos_yaw;

				//
				// Take out all squares whose centre is in the
				// bounding box of the prim.
				//

				PrimInfo *pi = get_prim_info(p_thing->Draw.Mesh->ObjectId);

				x1 = (p_thing->WorldPos.X >> 8) - pi->radius >> 8;
				z1 = (p_thing->WorldPos.Z >> 8) - pi->radius >> 8;
				x2 = (p_thing->WorldPos.X >> 8) + pi->radius >> 8;
				z2 = (p_thing->WorldPos.Z >> 8) + pi->radius >> 8;

				SATURATE(x1, 0, MAP_WIDTH  - 1);
				SATURATE(z1, 0, MAP_HEIGHT - 1);
				SATURATE(x2, 0, MAP_WIDTH  - 1);
				SATURATE(z2, 0, MAP_HEIGHT - 1);

				for (x = x1; x <= x2; x++)
				for (z = z1; z <= z2; z++)
				{
					//
					// Rotate the centre of the square into the space of the prim.
					//

					tx  = ((x << 8) + 0x80) - (p_thing->WorldPos.X >> 8);
					tz  = ((z << 8) + 0x80) - (p_thing->WorldPos.Z >> 8);

					rx = MUL64(tx, matrix[0]) + MUL64(tz, matrix[1]);
					rz = MUL64(tx, matrix[2]) + MUL64(tz, matrix[3]);

					#define MAV_PUSH_OUT (0x10)

					if (WITHIN(rx, pi->minx - MAV_PUSH_OUT, pi->maxx + MAV_PUSH_OUT) &&
						WITHIN(rz, pi->minz - MAV_PUSH_OUT, pi->maxz + MAV_PUSH_OUT))
					{
						//
						// Turn off this square.
						//

						MAV_turn_off_square(x, z);
					}
				}
			}
		}

		t_index = p_thing->LinkChild;
	}

	*/

	//
	// Set a bit where there is a water texture on the ground.
	//

	{
		SLONG mx;
		SLONG mz;

		SLONG page;

		for (mx = 0; mx < PAP_SIZE_HI; mx++)
		for (mz = 0; mz < PAP_SIZE_HI; mz++)
		{
			page = PAP_2HI(mx,mz).Texture & 0x3ff;

			if (page == 454 ||
				page == 99456 ||
				page == 99457)
			{
				//
				// This is water texture.
				//

				SET_MAV_SPARE(mx,mz,MAV_SPARE_FLAG_WATER);
			}
			else
			{
				SET_MAV_SPARE(mx,mz,0);
			}
		}
	}
}

void MAV_draw(
		SLONG sx1, SLONG sz1,
		SLONG sx2, SLONG sz2)
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

	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dz;

	SLONG lx;
	SLONG lz;

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

	ULONG colour[8] =
	{
		0x00ff0000,
		0x0000ff00,
		0x000000ff,
		0x00ffff00,
		0x00ff00ff,
		0x0000ffff,
		0x00ffaa88,
		0x00ffffff
	};

	SATURATE(sx1, 0, MAP_WIDTH  - 1);
	SATURATE(sz1, 0, MAP_HEIGHT - 1);

	for (x = sx1; x < sx2; x++)
	for (z = sz1; z < sz2; z++)
	{
		//
		// Draw a blue cross at the height we think the square is at.
		// 

		x1 = x + 0 << 8;
		z1 = z + 0 << 8;
		x2 = x + 1 << 8;
		z2 = z + 1 << 8;

		y1 = MAVHEIGHT(x,z) << 6;
		y2 = MAVHEIGHT(x,z) << 6;

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

		if (!ControlFlag)
		{
			ASSERT(WITHIN(MAV_NAV(x,z), 0, MAV_opt_upto - 1));

			mo = &MAV_opt[MAV_NAV(x,z)];

			mx = x1 + x2 >> 1;
			mz = z1 + z2 >> 1;

			for (i = 0; i < 4; i++)
			{
				dx = order[i].dx;
				dz = order[i].dz;

				lx  = mx + dx * 96;
				lz  = mz + dz * 96;

				lx +=  dz * (16 * 3);
				lz += -dx * (16 * 3);

				for (j = 0; j < 8; j++)
				{
					if (mo->opt[i] & (1 << j))
					{
						AENG_world_line(
							mx, y1, mz, 0, 0,
							lx, y2, lz, 9, colour[j],
							TRUE);
					}

					lx += -dz * 16;
					lz += +dx * 16;
				}
			}
		}
		else
		{
			mx = x1 + x2 >> 1;
			mz = z1 + z2 >> 1;

			for (i = 0; i < 4; i++)
			{
				dx = order[i].dx;
				dz = order[i].dz;

				lx  = mx + dx * 96;
				lz  = mz + dz * 96;

				if (MAV_CAR_GOTO(x, z, i))
				{
					AENG_world_line(
						mx, y1, mz, 0, 0,
						lx, y2, lz, 9, colour[0],
						TRUE);
				}
			}
		}
	}

	TRACE("MAV_opts_upto = %d\n", MAV_opt_upto);
}
#endif
#endif

//
// Returns TRUE if you can walk from a to b. If it returns FALSE, then
// (MAV_last_x, MAV_last_z) is the last square it reached, and (MAV_dmx, MAV_dmz)
// is the direction it tried to leave the square in.
//

SLONG MAV_last_mx;
SLONG MAV_last_mz;
SLONG MAV_dmx;
SLONG MAV_dmz;

SLONG MAV_can_i_walk(
		UBYTE ax, UBYTE az,
		UBYTE bx, UBYTE bz)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG dist;
	SLONG overdist;

	SLONG mx;
	SLONG mz;

	MAV_Opt *mo;

	ASSERT(WITHIN(ax, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(az, 0, MAP_HEIGHT - 1));

	ASSERT(WITHIN(bx, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(bz, 0, MAP_HEIGHT - 1));

	dx = bx - ax << 4;
	dz = bz - az << 4;

	dist = QDIST2(abs(dx),abs(dz));

	if (dist == 0)
	{
		return TRUE;
	}
	else
	{
		//
		// Normalise (dx,dz) to length 0x40 ish
		//

		overdist = 0x4000 / dist;

		dx = dx * overdist >> 8;
		dz = dz * overdist >> 8;
	}

	MAV_last_mx = ax;
	MAV_last_mz = az;

	x = (ax << 8) + 0x80;
	z = (az << 8) + 0x80;

	while(1)
	{
		x += dx;
		z += dz;

		mx = x >> 8;
		mz = z >> 8;

		MAV_dmx = mx - MAV_last_mx;
		MAV_dmz = mz - MAV_last_mz;

		if (MAV_dmx | MAV_dmz)
		{
			ASSERT(WITHIN(MAV_last_mx, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(MAV_last_mz, 0, MAP_HEIGHT - 1));

			ASSERT(WITHIN(MAV_NAV(MAV_last_mx,MAV_last_mz), 0, MAV_opt_upto - 1));

			mo = &MAV_opt[MAV_NAV(MAV_last_mx,MAV_last_mz)];

			//
			// Is there a wall in the way?
			//

			if (MAV_dmx == -1 && !(mo->opt[MAV_DIR_XS] & MAV_CAPS_GOTO)) {return FALSE;}
			if (MAV_dmx == +1 && !(mo->opt[MAV_DIR_XL] & MAV_CAPS_GOTO)) {return FALSE;}

			if (MAV_dmz == -1 && !(mo->opt[MAV_DIR_ZS] & MAV_CAPS_GOTO)) {return FALSE;}
			if (MAV_dmz == +1 && !(mo->opt[MAV_DIR_ZL] & MAV_CAPS_GOTO)) {return FALSE;}

			if (MAV_dmx && MAV_dmz)
			{
				//
				// We have to try the corner pieces as well because we are moving diagonally.
				//

				mo = &MAV_opt[MAV_NAV(mx,MAV_last_mz)];

				if (MAV_dmz == -1 && !(mo->opt[MAV_DIR_ZS] & MAV_CAPS_GOTO)) {return FALSE;}
				if (MAV_dmz == +1 && !(mo->opt[MAV_DIR_ZL] & MAV_CAPS_GOTO)) {return FALSE;}

				mo = &MAV_opt[MAV_NAV(MAV_last_mx,mz)];

				if (MAV_dmx == -1 && !(mo->opt[MAV_DIR_XS] & MAV_CAPS_GOTO)) {return FALSE;}
				if (MAV_dmx == +1 && !(mo->opt[MAV_DIR_XL] & MAV_CAPS_GOTO)) {return FALSE;}
			}

			if (mx == bx &&
				mz == bz)
			{
				return TRUE;
			}

			MAV_last_mx = mx;
			MAV_last_mz = mz;
		}
	}
}



UBYTE MAV_start_x;
UBYTE MAV_start_z;

UBYTE MAV_dest_x;
UBYTE MAV_dest_z;

//
// How much look-ahead we use in the navigation.  MAV_node[0] is the end of
// the looked-ahead route and (MAV_node_upto - 1) is the start(x,z)
//

#define MAV_LOOKAHEAD 32

MAV_Action MAV_node[MAV_LOOKAHEAD];
SLONG      MAV_node_upto;

//
// Each UBYTE is four two squares. There is three bits for action, and one bit
// to say whether we have been here or not.
//

UBYTE MAV_flag[MAP_HEIGHT][MAP_WIDTH / 2];

//
// Each UBYTE is four squares-worth of the direction
// we have come from.
//

UBYTE MAV_dir[MAP_HEIGHT][MAP_WIDTH / 4];



//
// Functions to set bits...
//

inline void MAV_visited_set(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

	SLONG byte = x >> 1;
	SLONG bit  = 8 << ((x & 0x1) << 2);

	MAV_flag[z][byte] |= bit;
}

inline SLONG MAV_visited_get(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

	SLONG byte = x >> 1;
	SLONG bit  = 8 << ((x & 0x1) << 2);

	return (MAV_flag[z][byte] & bit);
}

//
// This function also sets the visited flag.
//

inline void MAV_action_set(SLONG x, SLONG z, SLONG dir)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

	SLONG byte  = x >> 1;
	SLONG shift = (x & 0x1) << 2;

	dir |= 0x08;	// Set the visited flag too.

	MAV_flag[z][byte] &= ~(0x7 << shift);
	MAV_flag[z][byte] |=  (dir << shift);
}

inline SLONG MAV_action_get(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

	SLONG byte  = x >> 1;
	SLONG shift = (x & 0x1) << 2;

	return ((MAV_flag[z][byte] >> shift) & 0x7);
}

inline void MAV_dir_set(SLONG x, SLONG z, SLONG dir)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

	SLONG byte  = x >> 2;
	SLONG shift = (x & 0x3) << 1;

	MAV_dir[z][byte] &= ~(0x3 << shift);
	MAV_dir[z][byte] |=  (dir << shift);
}

inline SLONG MAV_dir_get(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

	SLONG byte  = x >> 2;
	SLONG shift = (x & 0x3) << 1;

	return ((MAV_dir[z][byte] >> shift) & 0x3);
}

//
// Clears the visited flags in given box. The
// other flags are undefined after this call.
//

void MAV_clear_bbox(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG  z;
	SLONG  len;
	SLONG  count;
	SLONG *zero;

	//
	// Round x1 down and x2 up to the nearest 4 byte boundary.
	//

	x2 +=  0x7;
	x1 &= ~0x7;
	x2 &= ~0x7;

	SATURATE(x1, 0, MAP_WIDTH);
	SATURATE(x2, 0, MAP_WIDTH);
	SATURATE(z1, 0, MAP_HEIGHT);
	SATURATE(z2, 0, MAP_HEIGHT);

	//
	// Clear a SLONG at a time.
	//

	len = x2 - x1 >> 3;

	for (z = z1; z < z2; z++)
	{
		zero  = (SLONG *) (&MAV_flag[z][x1 >> 1]);
		count = len;

		while(count--) {*zero++ = 0;}
	}
}




//
// Returns the first thing that should be done according to the current nodelist.
//

MAV_Action MAV_get_first_action_from_nodelist()
{
	SLONG i;

	UBYTE ax;
	UBYTE az;

	UBYTE bx;
	UBYTE bz;

	MAV_Action ans;

	//
	// Remember the last place we can walk to.
	//

	ax = MAV_start_x;
	az = MAV_start_z;

	ans.action = MAV_ACTION_GOTO;
	ans.dest_x = ax;
	ans.dest_z = az;

	for (i = MAV_node_upto - 1; i >= 0; i--)
	{
 		if (MAV_node[i].action != MAV_ACTION_GOTO)
		{
			ans.action = MAV_node[i].action;
			ans.dir    = MAV_node[i].dir;

			return ans;
		}

		bx = MAV_node[i].dest_x;
		bz = MAV_node[i].dest_z;

		if (MAV_can_i_walk(
				ax, az,
				bx, bz))
		{
			ans.dest_x = bx;
			ans.dest_z = bz;
		}
		else
		{
			return ans;
		}
	}

	return ans;
}

//
// Uses the MAV_flag and MAV_dir arrays to constuct a nodelist from
// from the given position back to (start_x,start_z).
//

void MAV_create_nodelist_from_pos(UBYTE end_x, UBYTE end_z)
{
	UBYTE x;
	UBYTE z;

	UBYTE action;
	UBYTE dir;

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

	x = end_x;
	z = end_z;

	MAV_node_upto = 0;

	while(
		x != MAV_start_x ||
		z != MAV_start_z)
	{
		ASSERT(WITHIN(MAV_node_upto, 0, MAV_LOOKAHEAD - 1));

		dir    = MAV_dir_get(x,z);
		action = MAV_action_get(x,z);

		MAV_node[MAV_node_upto].dest_x = x;
		MAV_node[MAV_node_upto].dest_z = z;
		MAV_node[MAV_node_upto].dir    = dir;
		MAV_node[MAV_node_upto].action = action;

		MAV_node_upto += 1;

		ASSERT(WITHIN(dir, 0, 3));

		x -= order[dir].dx;
		z -= order[dir].dz;

		if (action == MAV_ACTION_JUMP ||
			action == MAV_ACTION_JUMPPULL)
		{
			//
			// Moves you two squares.
			//
			
			x -= order[dir].dx;
			z -= order[dir].dz;
		}

		if (action == MAV_ACTION_JUMPPULL2)
		{
			//
			// Moves you three squares.
			//

			x -= order[dir].dx;
			z -= order[dir].dz;

			x -= order[dir].dx;
			z -= order[dir].dz;
		}
	}
}

//
// The priority queue needs these definitions...
//

typedef struct
{
	UBYTE x;
	UBYTE z;
	UBYTE score;	// The lower the score the better...
	UBYTE length;

} PQ_Type;

#define PQ_HEAP_MAX_SIZE 256

SLONG PQ_better(PQ_Type *a, PQ_Type *b)
{
	return a->score < b->score;
}

#include "pq.h"
#include "pq.cpp"


//
// Returns the score associated with the given position.
//

UBYTE MAV_score_pos(UBYTE x, UBYTE z)
{
	SLONG dx;
	SLONG dz;

	SLONG dist;

	//
	// Just return the distance to the destination.
	//

	dx = abs((MAV_dest_x - x) << 1);
	dz = abs((MAV_dest_z - z) << 1);

	dist = QDIST2(dx,dz);

	if (dist > 255) {dist = 255;}

	return dist;
}


UBYTE MAV_do_found_dest;

MAV_Action MAV_do(
			SLONG me_x,
			SLONG me_z,
			SLONG dest_x,
			SLONG dest_z,
			UBYTE caps)
{
	SLONG i;
	SLONG j;

	UBYTE opt;
	UBYTE move_one;
	UBYTE move_two;
	UBYTE move_three;
	UBYTE action;

	SLONG overflows;
	SLONG best_score;
	MAV_Action ans;

	MAV_Opt *mo;

	PQ_Type start;
	PQ_Type best;
	PQ_Type next;

	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dz;

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

	//
	// Remember the destination.
	//

	MAV_start_x = me_x;
	MAV_start_z = me_z;

	MAV_dest_x = dest_x;
	MAV_dest_z = dest_z;

	//
	// Clear the flag by default.
	//

	MAV_do_found_dest = FALSE;

	//
	// Clear the flags.
	//
	
	MAV_clear_bbox(
		me_x - MAV_LOOKAHEAD,
		me_z - MAV_LOOKAHEAD,
		me_x + MAV_LOOKAHEAD,
		me_z + MAV_LOOKAHEAD);

	//memset(MAV_flag, 0, sizeof(MAV_flag));

	//
	// Initialise the heap with our start square.
	//

	PQ_init();

	start.x      = me_x;
	start.z      = me_z;
	start.score  = MAV_score_pos(me_x, me_z);
	start.length = 0;

	PQ_add(start);

	MAV_visited_set(me_x,me_z);

	//
	// Initialise the score and answer.
	//

	#define MAV_MAX_OVERFLOWS 8

	overflows  = 0;
	best_score = INFINITY;
	ans.action = MAV_ACTION_GOTO;
	ans.dir    = 0;
	ans.dest_x = me_x;
	ans.dest_z = me_z;

	while(1)
	{
		if (PQ_empty())
		{
			break;
		}

		//
		// Get the best square so far and move it on a bit.
		//

		best = PQ_best();
		       PQ_remove();

		if (best.length >= MAV_LOOKAHEAD)
		{
			if (best.score < best_score)
			{
				best_score = best.score;

				//
				// Work out the first action and put it in answer.
				//

				MAV_create_nodelist_from_pos(best.x, best.z);
				ans = MAV_get_first_action_from_nodelist();
			}

			overflows += 1;

			if (overflows >= MAV_MAX_OVERFLOWS)
			{
				//
				// Dont do any more calculation.
				//

				return ans;
			}

			continue;
		}

		if (best.x == MAV_dest_x &&
			best.z == MAV_dest_z)
		{
			//
			// Found the destination.
			//

			MAV_do_found_dest = TRUE;

			//
			// Work out the first action and return it as the answer.
			//

			MAV_create_nodelist_from_pos(best.x, best.z);
			ans = MAV_get_first_action_from_nodelist();

			return ans;
		}

		ASSERT(WITHIN(best.x, 0, MAP_WIDTH  - 1));
		ASSERT(WITHIN(best.z, 0, MAP_HEIGHT - 1));

		ASSERT(WITHIN(MAV_NAV(best.x,best.z), 0, MAV_opt_upto - 1));

		mo = &MAV_opt[MAV_NAV(best.x,best.z)];

		//
		// Add neighbouring squares to the priority queue.
		//

		for (i = 0; i < 4; i++)
		{
			opt = mo->opt[i] & caps;

			//
			// Moving one square.
			//

			move_one   = opt & (MAV_CAPS_GOTO | MAV_CAPS_PULLUP | MAV_CAPS_CLIMB_OVER | MAV_CAPS_FALL_OFF | MAV_CAPS_LADDER_UP);
			move_two   = opt & (MAV_CAPS_JUMP | MAV_CAPS_JUMPPULL);
			move_three = opt & (MAV_CAPS_JUMPPULL2);

			dx = order[i].dx;
			dz = order[i].dz;

			if (move_one)
			{
				mx = best.x + dx;
				mz = best.z + dz;

				if (!MAV_visited_get(mx,mz))
				{
					next.x      = mx;
					next.z      = mz;
					next.length = best.length + 1;
					next.score  = MAV_score_pos(mx,mz);

					//
					// What action did we use to get here?
					//

					     if (opt & MAV_CAPS_GOTO)		{action = MAV_ACTION_GOTO;}
					else if (opt & MAV_CAPS_PULLUP)		{action = MAV_ACTION_PULLUP;}
					else if (opt & MAV_CAPS_CLIMB_OVER)	{action = MAV_ACTION_CLIMB_OVER;}
					else if (opt & MAV_CAPS_FALL_OFF)	{action = MAV_ACTION_FALL_OFF;}
					else if (opt & MAV_CAPS_LADDER_UP)	{action = MAV_ACTION_LADDER_UP;}
					else ASSERT(0);

					//
					// How we reached this square.
					//

					MAV_action_set(	// Sets the visited flag aswell.
						mx,
						mz,
						action);

					MAV_dir_set(
						mx,
						mz,
						i);

					if (action != MAV_ACTION_GOTO)
					{
						//
						// Bias against using strange methods of travel!
						//

						next.score += Random() & 0x3;

						if (action == MAV_ACTION_FALL_OFF)
						{
							//
							// Falling off isn't so bad...
							//
						}
						else
						{
							next.score += 3;
						}
					}

					//
					// Add this square to the priority queue.
					//

					PQ_add(next);
				}
			}

			if (move_two)
			{
				mx = best.x + dx + dx;
				mz = best.z + dz + dz;

				if (!MAV_visited_get(mx,mz))
				{
					next.x      = mx;
					next.z      = mz;
					next.length = best.length + 1;
					next.score  = MAV_score_pos(mx,mz);

					//
					// What action did we use to get here?
					//

					     if (opt & MAV_CAPS_JUMP)	  {action = MAV_ACTION_JUMP;}
					else if (opt & MAV_CAPS_JUMPPULL) {action = MAV_ACTION_JUMPPULL;}
					else ASSERT(0);

					//
					// How we reached this square.
					//

					MAV_action_set(	// Sets the visited flag aswell.
						mx,
						mz,
						action);

					MAV_dir_set(
						mx,
						mz,
						i);

					//
					// Add this square to the priority queue.
					//

					PQ_add(next);
				}
			}

			if (move_three)
			{
				mx = best.x + dx + dx + dx;
				mz = best.z + dz + dz + dz;

				if (!MAV_visited_get(mx,mz))
				{
					next.x      = mx;
					next.z      = mz;
					next.length = best.length + 1;
					next.score  = MAV_score_pos(mx,mz);

					//
					// What action did we use to get here?
					//

					action = MAV_ACTION_JUMPPULL2;

					//
					// How we reached this square.
					//

					MAV_action_set(	// Sets the visited flag aswell.
						mx,
						mz,
						action);

					MAV_dir_set(
						mx,
						mz,
						i);

					//
					// Add this square to the priority queue.
					//

					PQ_add(next);
				}
			}
		}
	}

	return ans;
}



SLONG MAV_inside(
		SLONG x,
		SLONG y,
		SLONG z)
{
	x >>= 8;
	y >>= 6;
	z >>= 8;

	if (WITHIN(x, 0, MAP_WIDTH  - 1) &&
		WITHIN(z, 0, MAP_HEIGHT - 1))
	{
		if (y < -127) {return TRUE;}
		if (y > +127) {return FALSE;}

		if (y < MAVHEIGHT(x,z))
		{
			return TRUE;
		}
	}

	return FALSE;
}


SLONG MAV_height_los_fail_x;
SLONG MAV_height_los_fail_y;
SLONG MAV_height_los_fail_z;

SLONG MAV_height_los_fast(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2)
{
	SLONG dx = x2 - x1;
	SLONG dy = y2 - y1;
	SLONG dz = z2 - z1;

	SLONG dist  = QDIST2(abs(dx),abs(dz));
	SLONG steps = (dist >> 8) + 1;

	SLONG x = x1;
	SLONG y = y1;
	SLONG z = z1;

	dx /= steps;
	dy /= steps;
	dz /= steps;

	while(steps-- >= 0)
	{
		if (MAV_inside(x,y,z))
		{
			MAV_height_los_fail_x = x - (dx >> 0);
			MAV_height_los_fail_y = y - (dy >> 0);
			MAV_height_los_fail_z = z - (dz >> 0);

			return FALSE;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	return TRUE;
}

SLONG MAV_height_los_slow(
		SLONG ware,
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2)
{
	SLONG dx = x2 - x1;
	SLONG dy = y2 - y1;
	SLONG dz = z2 - z1;

	SLONG dist  = QDIST2(abs(dx),abs(dz));
	SLONG steps = (dist >> 8) + 1;

	dx /= steps;
	dy /= steps;
	dz /= steps;

	SLONG x = x1 + dx;
	SLONG y = y1 + dy;
	SLONG z = z1 + dz;

	while(steps-- > 0)
	{
		SLONG inside;

		if (ware)
		{
			inside = WARE_inside(ware, x, y, z);
		}
		else
		{
			inside = MAV_inside(x,y,z);
		}

		if (inside)
		{
			MAV_height_los_fail_x = x;
			MAV_height_los_fail_y = y;
			MAV_height_los_fail_z = z;

			return FALSE;
		}

		x += dx;
		y += dy;
		z += dz;
	}

	return TRUE;
}




//
// Finds the nearest building entrance to the given place.  Returns
// FALSE if the building doesn't have an entrance.
//

#ifdef UNUSED
SLONG MAV_find_building_entrance(
		SLONG  building,
		SLONG  near_to_x,
		SLONG  near_to_y,
		SLONG  near_to_z,
		SLONG *door_x,
		SLONG *door_z)
{
	SLONG facet;
	SLONG score;
	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;
	SLONG dx;
	SLONG dz;
	SLONG goto_x;
	SLONG goto_z;
	SLONG best_x;
	SLONG best_z;
	SLONG best_score;

	DBuilding *db = &dbuildings[building];
	DFacet    *df;

	best_x     = 0;
	best_z     = 0;
	best_score = INFINITY;

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

			goto_x = x1 + dx - dz;
			goto_z = z1 + dz + dx;

			//
			// How far is this door?
			//

			dx = near_to_x - goto_x;
			dz = near_to_z - goto_z;

			score = abs(dx) + abs(dz);

			//
			// Nearest door so far?
			//

			if (best_score > score)
			{
				best_score = score;
				best_x     = goto_x;
				best_z     = goto_z;
			}
		}
	}

   *door_x = best_x >> 8;
   *door_z = best_z >> 8;

	return best_score != INFINITY;
}
#endif


#ifndef	PSX
void MAV_precalculate_warehouse_nav(UBYTE ware)
{
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG dx;
	SLONG dz;
	
	SLONG mx;
	SLONG mz;

	SLONG tx;
	SLONG tz;

	SLONG rx;
	SLONG rz;

	SLONG dh;

	SLONG useangle;
	SLONG matrix[4];
	SLONG ladder;

	SLONG sin_yaw;
	SLONG cos_yaw;

	SLONG both_ground;

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

	UBYTE opt[4];

	WARE_Ware *ww = &WARE_ware[ware];

	//
	// Remember the old MAV_nav array.
	//

	UWORD *old_mav_nav       = MAV_nav;
	SLONG  old_mav_nav_pitch = MAV_nav_pitch;

	//
	// Set the MAV_nav array to point to the warehouse's private array.
	//

	MAV_nav       = &WARE_nav[ww->nav];
	MAV_nav_pitch =  ww->nav_pitch;

	//
	// Make the staircase prims change the MAV_height array 
	//

	OB_Info *oi;

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		for (oi = OB_find(x,z); oi->prim; oi++)
		{
			if (oi->prim == 41)
			{
				//
				// The step prim!
				//

				//
				// Find which mapsquare the middle of this prim is over.
				//

				PrimInfo *pi = get_prim_info(oi->prim);

				SLONG mx = pi->minx + pi->maxx >> 1;
				SLONG mz = pi->minz + pi->maxz >> 1;

				SLONG matrix[4];
				SLONG useangle;

				SLONG sin_yaw;
				SLONG cos_yaw;

				SLONG rx;
				SLONG rz;

				SLONG sx;
				SLONG sz;

				useangle  = -oi->yaw;
				useangle &=  2047;

				sin_yaw = SIN(useangle);
				cos_yaw = COS(useangle);

				matrix[0] =  cos_yaw;
				matrix[1] = -sin_yaw;
				matrix[2] =  sin_yaw;
				matrix[3] =  cos_yaw;

				rx = MUL64(mx, matrix[0]) + MUL64(mz, matrix[1]);
				rz = MUL64(mx, matrix[2]) + MUL64(mz, matrix[3]);

				rx += oi->x;
				rz += oi->z;

				rx >>= 8;
				rz >>= 8;
	
				if (WITHIN(rx, ww->minx, ww->maxx) &&
					WITHIN(rz, ww->minz, ww->maxz))
				{
					MAVHEIGHT(rx,rz) = oi->y + 0x40 >> 6;
				}
			}
		}
	}

	//
	// Work out the mav for each square in the bounding box of the warehouse.
	//

	for (x = ww->minx; x <= ww->maxx; x++)
	for (z = ww->minz; z <= ww->maxz; z++)
	{
		mx = x - ww->minx;
		mz = z - ww->minz;

		//
		// Look for a nearby ladder.
		//

		ladder = find_nearby_ladder_colvect_radius(
					(x << 8) + 0x80,
					(z << 8) + 0x80,
					0x100);

		for (i = 0; i < 4; i++)
		{
			opt[i] = 0;

			dx = order[i].dx;
			dz = order[i].dz;

			tx = x + dx;
			tz = z + dz;

			if (!(PAP_2HI(x,z).Flags & PAP_FLAG_HIDDEN))
			{	
				//
				// This square is outside the warehouse.
				//

				continue;
			}

			if (!WITHIN(tx, ww->minx, ww->maxx) ||
				!WITHIN(tz, ww->minz, ww->maxz))
			{
				//
				// Cannot navigate in this direction because it is
				// outside the warehouse.
				//

				continue;
			}

			if (!(PAP_2HI(tx,tz).Flags & PAP_FLAG_HIDDEN))
			{	
				//
				// This square is outside the warehouse.
				//

				continue;
			}

			//
			// Can we walk from (x,z) to (tx,tz)?
			//

			dh = MAVHEIGHT(tx,tz) - MAVHEIGHT(x,z);

			if (abs(dh) > 1)
			{
				//
				// There are at different heights, so you cant
				// just walk between the two squares.
				//

				if (dh < 0)
				{
					//
					// There might be a wall or fence in the way.
					//

					x1 = ( x << 8) + 0x80;
					z1 = ( z << 8) + 0x80;
					x2 = (tx << 8) + 0x80;
					z2 = (tz << 8) + 0x80;

					y1 = (MAVHEIGHT( x, z) << 6) + 0x50;
					y2 = (MAVHEIGHT(tx,tz) << 6) + 0x50;

					y = MAX(y1,y2);

					if (there_is_a_los(
							x1, y, z1,
							x2, y, z2,
							LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
					{
						//
						// We can always fall down becuase there is nothing in the way.
						// 

						opt[i] |= MAV_CAPS_FALL_OFF;
					}
					else
					{
						//
						// If there is a fence in the way, then we can scale the fence.
						//

						DFacet *df = &dfacets[los_failure_dfacet];

						if (df->FacetType == STOREY_TYPE_FENCE_FLAT)
						{
							if (df->FacetFlags & FACET_FLAG_UNCLIMBABLE)
							{
								//
								// Unclimbable fence.
								//
							}
							else
							{
								//
								// We can scale this fence.
								//

								opt[i] |= MAV_CAPS_CLIMB_OVER;
							}
						}
					}
				}
				else
				{
					if (WITHIN(dh, 3, 5))
					{
						//
						// We can pull ourselves up.
						//

						opt[i] |= MAV_CAPS_PULLUP;
					}

					if (ladder)
					{
						DFacet *df_ladder;

						ASSERT(WITHIN(ladder, 1, next_dfacet - 1));

						df_ladder = &dfacets[ladder];

						ASSERT(df_ladder->FacetType == STOREY_TYPE_LADDER);

						//
						// There is a ladder- can we climb up it in this direction?
						//

						x1 = ( x << 8) + 0x80;
						z1 = ( z << 8) + 0x80;
						x2 = (tx << 8) + 0x80;
						z2 = (tz << 8) + 0x80;

						if (two4_line_intersection(
								x1, z1,
								x2, z2,
								df_ladder->x[0] << 8, df_ladder->z[0] << 8,
								df_ladder->x[1] << 8, df_ladder->z[1] << 8))
						{
							opt[i] |= MAV_CAPS_LADDER_UP;
						}
					}
				}
			}
			else
			{
				//
				// There might be a wall or fence in the way.
				//

				x1 = ( x << 8) + 0x80;
				z1 = ( z << 8) + 0x80;
				x2 = (tx << 8) + 0x80;
				z2 = (tz << 8) + 0x80;

				y1 = (MAVHEIGHT( x, z) << 6) + 0x50;
				y2 = (MAVHEIGHT(tx,tz) << 6) + 0x50;

				y = MAX(y1,y2);

				if (there_is_a_los(
						x1, y, z1,
						x2, y, z2,
						LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
				{
					//
					// Nothing in the way...
					//

					opt[i] |= MAV_CAPS_GOTO;
				}
				else
				{
					//
					// If there is a fence in the way, then we can scale the fence.
					//

					DFacet *df = &dfacets[los_failure_dfacet];

					if (df->FacetType == STOREY_TYPE_FENCE_FLAT)
					{
						if (df->FacetFlags & FACET_FLAG_UNCLIMBABLE)
						{
							//
							// Unclimbable fence.
							//
						}
						else
						{
							//
							// We can scale this fence.
							//

							opt[i] |= MAV_CAPS_CLIMB_OVER;
						}
					}
				}
			}

			if (!(opt[i] & MAV_CAPS_GOTO) &&
				!(opt[i] & MAV_CAPS_CLIMB_OVER))
			{
				//
				// Now what about jumping one block?
				//

				tx += dx;
				tz += dz;

				if (WITHIN(tx, ww->minx, ww->maxx) &&
					WITHIN(tz, ww->minz, ww->maxz) &&
					(PAP_2HI(tx,tz).Flags & PAP_FLAG_HIDDEN))
				{
					dh = MAVHEIGHT(tx,tz) - MAVHEIGHT(x,z);

					//
					// Can we jump there?
					//

					x1 = ( x << 8) + 0x80;
					z1 = ( z << 8) + 0x80;
					x2 = (tx << 8) + 0x80;
					z2 = (tz << 8) + 0x80;

					y1 = (MAVHEIGHT( x, z) << 6) + 0xa0;
					y2 = (MAVHEIGHT(tx,tz) << 6) + 0xa0;

					if (there_is_a_los(
							x1, y1, z1,
							x2, y2, z2,
							LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
					{
						if (dh < 2)
						{
							opt[i] |= MAV_CAPS_JUMP;
						}
						else
						{
							if (WITHIN(dh, 2, 5))
							{
								opt[i] |= MAV_CAPS_JUMPPULL;
							}
						}
					}

					//
					// What about jumping two blocks?
					//

					tx += dx;
					tz += dz;

					if (WITHIN(tx, 0, MAP_WIDTH  - 1) &&
						WITHIN(tz, 0, MAP_HEIGHT - 1))
					{
						dh = MAVHEIGHT(tx,tz) - MAVHEIGHT(x,z);

						if (dh > 4)
						{
							//
							// Can't make this jump
							//
						}
						else
						{
							if (dh > -6)
							{
								//
								// Can we jump there?
								//

								x1 = ( x << 8) + 0x80;
								z1 = ( z << 8) + 0x80;
								x2 = (tx << 8) + 0x80;
								z2 = (tz << 8) + 0x80;

								y1 = (MAVHEIGHT( x, z) << 6) + 0xa0;
								y2 = (MAVHEIGHT(tx,tz) << 6) + 0xa0;

								if (there_is_a_los(
										x1, y1, z1,
										x2, y2, z2,
										LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
								{
									opt[i] |= MAV_CAPS_JUMPPULL2;
								}
							}
						}
					}
				}
			}
		}

		StoreMavOpts(mx, mz, opt);
	}

	//
	// A hack for the staircase prim.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		for (oi = OB_find(x,z); oi->prim; oi++)
		{
			if (oi->prim == 41)
			{
				//
				// The step prim!
				//

				{
					//
					// Find which mapsquare the middle of this prim is over.
					//

					PrimInfo *pi = get_prim_info(oi->prim);

					SLONG mx = pi->minx + pi->maxx >> 1;
					SLONG mz = pi->minz + pi->maxz >> 1;

					SLONG matrix[4];
					SLONG useangle;

					SLONG sin_yaw;
					SLONG cos_yaw;

					SLONG rx;
					SLONG rz;

					SLONG sx;
					SLONG sz;

					useangle  = -oi->yaw;
					useangle &=  2047;

					sin_yaw = SIN(useangle);
					cos_yaw = COS(useangle);

					matrix[0] =  cos_yaw;
					matrix[1] = -sin_yaw;
					matrix[2] =  sin_yaw;
					matrix[3] =  cos_yaw;

					rx = MUL64(mx, matrix[0]) + MUL64(mz, matrix[1]);
					rz = MUL64(mx, matrix[2]) + MUL64(mz, matrix[3]);

					rx += oi->x;
					rz += oi->z;

					rx >>= 8;
					rz >>= 8;

					if (WITHIN(rx, ww->minx, ww->maxx) &&
						WITHIN(rz, ww->minz, ww->maxz))
					{
						mx = rx - ww->minx;
						mz = rz - ww->minz;

						if (oi->yaw < 256 || oi->yaw > 1792 || WITHIN(oi->yaw, 768, 1280))
						{
							if (MAVHEIGHT(rx,rz) == MAVHEIGHT(rx-1,rz))
							{
								//
								// Walking left-right on a wide staircase.
								//
							}
							else
							{
								MAV_turn_movement_off(mx,     mz, MAV_DIR_XS);
								MAV_turn_movement_off(mx - 1, mz, MAV_DIR_XL);
							}

							if (MAVHEIGHT(rx,rz) == MAVHEIGHT(rx+1,rz))
							{
								//
								// Walking left-right on a wide staircase.
								//
							}
							else
							{
								MAV_turn_movement_off(mx,     mz, MAV_DIR_XL);
								MAV_turn_movement_off(mx + 1, mz, MAV_DIR_XS);
							}

							if (!WITHIN(oi->yaw, 768, 1280))
							{
								if (MAVHEIGHT(rx,rz+1) <= MAVHEIGHT(rx,rz) + 3)
								{
									MAV_turn_movement_on(mx,mz,   MAV_DIR_ZL);
									MAV_turn_movement_on(mx,mz+1, MAV_DIR_ZS);
								}
							}
							else
							{
								if (MAVHEIGHT(rx,rz-1) <= MAVHEIGHT(rx,rz) + 3)
								{
									MAV_turn_movement_on(mx,mz,   MAV_DIR_ZS);
									MAV_turn_movement_on(mx,mz-1, MAV_DIR_ZL);
								}
							}
						}					     
						else				     
						{
							if (MAVHEIGHT(rx, rz) == MAVHEIGHT(rx, rz-1))
							{
								//
								// Walking across a wide staircase.
								//
							}
							else
							{
								MAV_turn_movement_off(mx, mz,     MAV_DIR_ZS);
								MAV_turn_movement_off(mx, mz - 1, MAV_DIR_ZL);
							}

							if (MAVHEIGHT(rx, rz) == MAVHEIGHT(rx, rz+1))
							{
								//
								// Walking across a wide staircase.
								//
							}
							else
							{
								MAV_turn_movement_off(mx, mz,     MAV_DIR_ZL);
								MAV_turn_movement_off(mx, mz + 1, MAV_DIR_ZS);
							}

							if (!WITHIN(oi->yaw, 256, 768))
							{
								if (MAVHEIGHT(rx-1,rz) <= MAVHEIGHT(rx,rz) + 3)
								{
									MAV_turn_movement_on(mx,  mz, MAV_DIR_XS);
									MAV_turn_movement_on(mx-1,mz, MAV_DIR_XL);
								}
							}
							else
							{
								if (MAVHEIGHT(rx+1,rz) <= MAVHEIGHT(rx,rz) + 3)
								{
									MAV_turn_movement_on(mx,   mz, MAV_DIR_XL);
									MAV_turn_movement_on(mx+1, mz, MAV_DIR_XS);
								}
							}
						}
					}
				}
			}
		}
	}

	//
	// Restore the old MAV_nav array.
	//

	MAV_nav       = old_mav_nav;
	MAV_nav_pitch = old_mav_nav_pitch;
}
#endif


UBYTE MAV_get_caps(
		UBYTE x,
		UBYTE z,
		UBYTE dir)
{
	UBYTE ans;

	MAV_Opt *mo;

	if (WITHIN(x, 0, MAV_nav_pitch - 1) &&
		WITHIN(z, 0, MAV_nav_pitch - 1))
	{
		mo = &MAV_opt[MAV_NAV(x,z)];

		ASSERT(WITHIN(dir, 0, 3));

		ans = mo->opt[dir];

		return ans;
	}

	return 0;
}



void MAV_turn_car_movement_on(UBYTE mx, UBYTE mz, UBYTE dir)
{
	UBYTE mav;

	ASSERT(WITHIN(mx, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(mz, 0, PAP_SIZE_HI - 1));

	mav  = MAV_CAR(mx,mz);
	mav |= 1 << dir;

	SET_MAV_CAR(mx,mz,mav);
}

void MAV_turn_car_movement_off(UBYTE mx, UBYTE mz, UBYTE dir)
{
	UBYTE mav;

	ASSERT(WITHIN(mx, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(mz, 0, PAP_SIZE_HI - 1));

	mav  =  MAV_CAR(mx,mz);
	mav &= ~(1 << dir);

	SET_MAV_CAR(mx,mz,mav);
}

