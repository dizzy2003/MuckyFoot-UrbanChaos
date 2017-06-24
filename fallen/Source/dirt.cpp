//
// Bits of dirt that get blown around. Dirt only exists
// around one focal point (the camera). If a bit of dirt gets
// too far away, then it teleports to somewhere on the
// edge on the focus point.
//

#include "game.h"
#include "dirt.h"
#include "water.h"
#include "sound.h"
#include "morph.h"
#include "ns.h"
#include "pap.h"
#include "ob.h"
#include "drip.h"
#include "pcom.h"
#include "animate.h"
#include "fmatrix.h"
#include "fallen/headers/night.h"
#include "pyro.h"
#include "poly.h"
#include "psystem.h"
#include "memory.h"
#include "mfx.h"
#include "pow.h"
#include "mav.h"

#ifndef PSX
#include    "fallen/DDLibrary/headers/D3DTexture.h"
#include    "fallen/DDLibrary/headers/GDisplay.h"
#endif

#define TICK_SHIFT_LOWRES	(TICK_SHIFT-2)

//
// A good version of arctan.
//

SLONG calc_angle(SLONG dx, SLONG dz);

//
// Where all the trees are.
//

typedef struct
{
	UWORD x;
	UWORD z;
	UBYTE inrange;
	UBYTE padding;

} DIRT_Tree;

#define DIRT_MAX_TREES 64

DIRT_Tree DIRT_tree[DIRT_MAX_TREES];
SLONG     DIRT_tree_upto;

//
// Check 1/8th of the dirt each frame.
//

static UWORD DIRT_check=0;


//
// The number of leaves per tree.
//

#ifdef	PSX
#define DIRT_LEAVES_PER_TREE 16
#else
#define DIRT_LEAVES_PER_TREE 200
#endif


//
// The probabilites of each type of dirt cropping up.
// They are in 8-bit fixed point. i.e. they add up to 256.
//

SLONG DIRT_prob_leaf;
SLONG DIRT_prob_can;
SLONG DIRT_prob_pigeon;

//
// The bounding box in which only pigeons are created.
//

SLONG DIRT_pigeon_map_x1;
SLONG DIRT_pigeon_map_z1;
SLONG DIRT_pigeon_map_x2;
SLONG DIRT_pigeon_map_z2;


//
// The bits of dirt.
//

DIRT_Dirt DIRT_dirt[DIRT_MAX_DIRT];


//
// Pigeon states.
//

#define DIRT_PIGEON_WAIT		0
#define DIRT_PIGEON_PECK		1
#define DIRT_PIGEON_WALK		2
#define DIRT_PIGEON_HOP			3
#define DIRT_PIGEON_COURT		4
#define DIRT_PIGEON_RUN			5
#define DIRT_PIGEON_TAKEOFF		6
#define DIRT_PIGEON_FLY			7
#define DIRT_PIGEON_LAND		8
#define DIRT_PIGEON_PERCHED		9
#define DIRT_PIGEON_SIDLE		10
#define DIRT_PIGEON_NUM_STATES  11


//
// The focus.
//

SLONG DIRT_focus_x;
SLONG DIRT_focus_z;
SLONG DIRT_focus_radius;
SLONG DIRT_focus_first;


//
// Uses the probabilites and the pigeon rectangle to decide
// the type of the new bit of dirt at (x,z).
//

SLONG DIRT_get_new_type(SLONG x, SLONG z)
{
	SLONG choice;

	SLONG mx = x >> PAP_SHIFT_HI;
	SLONG mz = z >> PAP_SHIFT_HI;

	if (WITHIN(mx, DIRT_pigeon_map_x1, DIRT_pigeon_map_x2) &&
		WITHIN(mz, DIRT_pigeon_map_z1, DIRT_pigeon_map_z2))
	{
		ASSERT(0);
		return DIRT_TYPE_PIGEON;
	}
	else
	{
//		if ((world_type==WORLD_TYPE_SNOW)||(world_type==WORLD_TYPE_FOREST)) return DIRT_TYPE_LEAF;
		if (world_type==WORLD_TYPE_FOREST) return DIRT_TYPE_LEAF;
		if (world_type==WORLD_TYPE_SNOW) return DIRT_TYPE_SNOW;

		choice = Random() & 0xff;

		choice -= DIRT_prob_leaf;   if (choice < 0) {return DIRT_TYPE_LEAF;}
		choice -= DIRT_prob_can;    if (choice < 0) {return DIRT_TYPE_CAN;}
		choice -= DIRT_prob_pigeon; if (choice < 0) {return DIRT_TYPE_PIGEON;}
		return DIRT_TYPE_UNUSED;
	}
}


void DIRT_init(
		SLONG prob_leaf,
		SLONG prob_can,
		SLONG prob_pigeon,
		SLONG pigeon_map_x1,
		SLONG pigeon_map_z1,
		SLONG pigeon_map_x2,
		SLONG pigeon_map_z2)
{
	SLONG i;
	SLONG prob_sum;

	DIRT_Dirt *dd;

	THING_INDEX index;
	Thing      *p_thing;

	DIRT_focus_x      = 0;
	DIRT_focus_z      = 0;
	DIRT_focus_radius = 0x100;
	DIRT_focus_first  = 4;
	DIRT_check		  = 0;

	//
	// no bug ridden pigeons for us!
	//

	prob_pigeon = 0;

	//
	// Put all the dirt at a stupid place, so that
	// the next time the focus is set, all the dirt will
	// be recalculated.
	//

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		memset((UBYTE*)&DIRT_dirt[i],0,sizeof(DIRT_Dirt));
		DIRT_dirt[i].type = DIRT_TYPE_UNUSED;
	}

	//
	// Normalise the probabilites.
	//

	prob_sum  = prob_leaf;
	prob_sum += prob_can;
	prob_sum += prob_pigeon;

	if (prob_sum == 0)
	{
		DIRT_prob_leaf   = 0;
		DIRT_prob_can    = 0;
		DIRT_prob_pigeon = 0;
	}
	else
	{
		DIRT_prob_leaf   = (prob_leaf   * 256) / prob_sum;
		DIRT_prob_can    = (prob_can    * 256) / prob_sum;
		DIRT_prob_pigeon = (prob_pigeon * 256) / prob_sum;
	}

	//
	// Store the pigeon bounding box.
	//

	DIRT_pigeon_map_x1 = pigeon_map_x1;
	DIRT_pigeon_map_z1 = pigeon_map_z1;
	DIRT_pigeon_map_x2 = pigeon_map_x2;
	DIRT_pigeon_map_z2 = pigeon_map_z2;

	//
	// Look for all the trees.
	//

	DIRT_tree_upto = 0;

	{
		SLONG mx;
		SLONG mz;

		OB_Info *oi;

		for (mx = 0; mx < PAP_SIZE_LO; mx++)
		for (mz = 0; mz < PAP_SIZE_LO; mz++)
		{
			for (oi = OB_find(mx,mz); oi->prim; oi++)
			{
				if (prim_objects[oi->prim].flag & PRIM_FLAG_TREE)
				{
					//
					// We have found a tree!
					//

					if (WITHIN(DIRT_tree_upto, 0, DIRT_MAX_TREES - 1))
					{
						DIRT_tree[DIRT_tree_upto].x       = oi->x;
						DIRT_tree[DIRT_tree_upto].z       = oi->z;
						DIRT_tree[DIRT_tree_upto].inrange = FALSE;

						DIRT_tree_upto += 1;
					}
				}
			}
		}
	}
}


//
// Returns a piece of dirt that isn't very important (offscreen or far away).
//

DIRT_Dirt *DIRT_find_useless(void)
{
	SLONG i;

	DIRT_Dirt *dd;

	//
	// Find offscreen or unused dirt.
	//

	for (i = 0; i < 8; i++)
	{
		dd = &DIRT_dirt[Random() & (DIRT_MAX_DIRT - 1)];

		if ( dd->type == DIRT_TYPE_UNUSED ||
			(dd->flag &  DIRT_FLAG_DELETE_OK))
		{
			return dd;
		}
	}

	//
	// Find a leaf.
	//

	for (i = 0; i < 8; i++)
	{
		dd = &DIRT_dirt[Random() & (DIRT_MAX_DIRT - 1)];

		if ( dd->type == DIRT_TYPE_LEAF)
		{
			return dd;
		}
	}

	//
	// Any dirt will do!
	//

	dd = &DIRT_dirt[Random() & (DIRT_MAX_DIRT - 1)];

	return dd;
}



void DIRT_set_focus(
		SLONG x,
		SLONG z,
		SLONG radius)
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG lx;
	SLONG lz;
	SLONG cx;
	SLONG cz;
	SLONG nx;
	SLONG nz;
	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG angle;
	SLONG type;
	SLONG done;

	DIRT_Dirt  *dd;
	PAP_Hi     *ph;
	PAP_Hi     *ph2;

	struct
	{
		SBYTE dx;
		SBYTE dz;

	} order[8] =
	{
		{-1,-1},
		{ 0,-1},
		{+1,-1},
		{-1, 0},
		{+1, 0},
		{-1,+1},
		{ 0,+1},
		{+1,+1}
	};

	DIRT_focus_x      = x;
	DIRT_focus_z      = z;
	DIRT_focus_radius = radius;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	//
	// Check all the trees.
	//

	for (i = 0; i < DIRT_tree_upto; i++)
	{
		dx = abs(DIRT_tree[i].x - DIRT_focus_x);
		dz = abs(DIRT_tree[i].z - DIRT_focus_z);

		dist = QDIST2(dx,dz);

		if (dist > DIRT_focus_radius)
		{
			if (DIRT_tree[i].inrange)
			{
				//
				// Get rid of this tree's leaves.
				//

				for (j = 0; j < DIRT_MAX_DIRT; j++)
				{
					dd = &DIRT_dirt[j];

					if (dd->type == DIRT_TYPE_LEAF && dd->owner == i)
					{
						dd->type = DIRT_TYPE_UNUSED;
					}
				}

				DIRT_tree[i].inrange = FALSE;
			}
		}
		else
		{
			if (!DIRT_tree[i].inrange)
			{
				//
				// Create leaves for this tree.
				//

				done = 0;

				dd = &DIRT_dirt[Random() % DIRT_MAX_DIRT];

				for (j = 0; j < DIRT_MAX_DIRT; j++)
				{
					dd += 1;

					if (dd >= &DIRT_dirt[DIRT_MAX_DIRT])
					{
						dd = &DIRT_dirt[0];
					}

					if ( dd->type == DIRT_TYPE_UNUSED ||
						(dd->flag  & DIRT_FLAG_DELETE_OK))
					{
						done += 1;

						//
						// How far from the tree?
						//

						dist  = Random() & 0x7f;
						dist += 150;
						dist  = dist * dist >> 8;
						angle = Random() & 2047;

						dx = SIN(angle) * dist >> 16;
						dz = COS(angle) * dist >> 16;

						lx = DIRT_tree[i].x + dx;
						lz = DIRT_tree[i].z + dz;

						//
						// Is this a good place to put some dirt?
						//

						mx = lx >> 8;
						mz = lz >> 8;

						if (PAP_on_map_hi(mx, mz))
						{
							if (!(PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN))
							{
								dd->type   = DIRT_TYPE_LEAF;
								dd->owner  = i;
								dd->flag   = DIRT_FLAG_STILL;
								dd->x      = lx;
								dd->z      = lz;
								dd->y      = PAP_calc_height_at(lx, lz);
								dd->dx     = 0;
								dd->dy     = 0;
								dd->dz     = 0;
								dd->yaw    = 0;
								dd->pitch  = 0;
								dd->roll   = 0;
								dd->dyaw   = 0;
								dd->dpitch = 0;
								dd->droll  = 0;
							}
						}

						if (done >= DIRT_LEAVES_PER_TREE)
						{
							break;
						}
					}
				}

				DIRT_tree[i].inrange = TRUE;
			}
		}
	}


	//
	// Check the dirt for going out of range.
	//

	SLONG number_to_check;

	if (DIRT_focus_first)
	{
		number_to_check = DIRT_MAX_DIRT;
	}
	else
	{
		number_to_check = DIRT_MAX_DIRT / 8;
	}

	for (i = 0; i < number_to_check; i++)
	{
		DIRT_check += 1;

		if (DIRT_check >= DIRT_MAX_DIRT)
		{
			DIRT_check = 0;
		}

		dd = &DIRT_dirt[DIRT_check];

		if (dd->type == DIRT_TYPE_LEAF && dd->owner != 255)
		{
			//
			// This leaf belongs to a tree so let the tree
			// handle the leaf.
			//

			continue;
		}

		if (dd->type == DIRT_TYPE_MINE)
		{
			//
			// Dont get rid of mines.
			//

			continue;
		}

		dx = abs(dd->x - DIRT_focus_x);
		dz = abs(dd->z - DIRT_focus_z);

		dist = QDIST2(dx,dz);
//		DebugText("i %d dc %d dfx dfz (%d,%d) ddx %d ddz %d dist %d type %d\n",i,DIRT_check,DIRT_focus_x,DIRT_focus_z,dd->x,dd->z,dist,dd->type);

		if (dist > DIRT_focus_radius || dd->type == DIRT_TYPE_UNUSED)
		{
			//
			// Create a new bit of dirt.  Where shall we put it?
			//

			angle = Random() & 2047;

			if (DIRT_focus_first)
			{
				dist  = Random() & 0xff;
				dist += Random() & 0xff;
				dist += Random() & 0xff;
				dist += Random() & 0xff;

				dist  = (0x3af - dist) * DIRT_focus_radius >> 10;
			}
			else
			{
				dist  = DIRT_focus_radius;
				dist -= Random() & 0xff;
				dist -= 0x80;
				if (world_type==WORLD_TYPE_SNOW) dist -= 0xff+(Random() & 0x1ff);
				if (dist<10) dist=10;
			}

			cx = DIRT_focus_x + MUL64(SIN(angle), dist);
			cz = DIRT_focus_z + MUL64(COS(angle), dist);

			//
			// Valid position?
			//

			mx = cx >> PAP_SHIFT_HI;
			mz = cz >> PAP_SHIFT_HI;
#ifdef	PSX
			if (PAP_on_map_hi(mx,mz) && !(PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN) && !(PAP_2HI(mx,mz).Texture&(1<<14)))
#else



			if (PAP_on_map_hi(mx,mz) && ( (world_type==WORLD_TYPE_SNOW) || !(PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN) ) && !(MAV_SPARE(mx,mz) & MAV_SPARE_FLAG_WATER)&& !(MAV_SPARE(mx+1,mz) & MAV_SPARE_FLAG_WATER)&& !(MAV_SPARE(mx-1,mz) & MAV_SPARE_FLAG_WATER)&& !(MAV_SPARE(mx,mz+1) & MAV_SPARE_FLAG_WATER)&& !(MAV_SPARE(mx,mz-1) & MAV_SPARE_FLAG_WATER))

#if 0
			if (PAP_on_map_hi(mx,mz))
			if ( (world_type==WORLD_TYPE_SNOW) || !((PAP_hi[mx][mz]).Flags & PAP_FLAG_HIDDEN) )
			if ( !((MAV_nav[((mx) * MAV_nav_pitch) + (mz)] >> 14) & MAV_SPARE_FLAG_WATER) )
			if ( !((MAV_nav[((mx) * MAV_nav_pitch) + (mz-1)] >> 14) & MAV_SPARE_FLAG_WATER) )
			if ( !((MAV_nav[((mx) * MAV_nav_pitch) + (mz+1)] >> 14) & MAV_SPARE_FLAG_WATER) )
			if ( !((MAV_nav[((mx-1) * MAV_nav_pitch) + (mz)] >> 14) & MAV_SPARE_FLAG_WATER) )
			if ( !((MAV_nav[((mx+1) * MAV_nav_pitch) + (mz)] >> 14) & MAV_SPARE_FLAG_WATER))
#endif
#endif
			{
//				DebugText(" dirt valid cx %d cz %d \n",cx,cz);
				//
				// This is an okay place for a new bit of dirt.  What type shall
				// we make it?
				//

				type = DIRT_get_new_type(cx, cz);

				switch(type)
				{
					case DIRT_TYPE_UNUSED:
						dd->type = DIRT_TYPE_UNUSED;
						break;

					case DIRT_TYPE_SNOW:
					case DIRT_TYPE_LEAF:
#ifdef	PSX
						dd->UU.Leaf.col=floor_psx_col[mx][mz];
#endif
						dd->type   = type;
						dd->owner  = 255;
						dd->flag   = DIRT_FLAG_STILL;
						dd->x      = cx;
						dd->z      = cz;
						dd->y	   = PAP_calc_height_at(cx, cz);
						if (type==DIRT_TYPE_SNOW)
						{
							if ((GAME_TURN>100)&&(Random()&1))
							{
								dd->y=NET_PERSON(0)->WorldPos.Y>>8;
								dd->y += 700+(Random()&0x1ff);
								dd->flag   = 0;
								dd->UU.Leaf.fade = 0xff;
							} else dd->UU.Leaf.fade = Random()&0xff;

						}
						dd->dx     = 0;
						dd->dz     = 0;
						dd->dy     = 0;
						dd->yaw    = 0;
						dd->pitch  = 0;
						dd->roll   = 0;
						dd->dyaw   = 0;
						dd->dpitch = 0;
						dd->droll  = 0;
						break;

					case DIRT_TYPE_CAN:
						dd->type   = DIRT_TYPE_CAN;
						dd->flag   = DIRT_FLAG_STILL;
						dd->x      = cx;
						dd->y      = PAP_calc_height_at(cx,cz) + 4;
						dd->z      = cz;
						dd->dx     = 0;
						dd->dy     = 0;
						dd->dz     = 0;
						dd->yaw    = Random() & 2047;
						dd->pitch  = 0;
						dd->roll   = 0;
						dd->dyaw   = 0;
						dd->dpitch = 0;
						dd->droll  = 0;
						break;

					case DIRT_TYPE_PIGEON:
#ifndef PSX
						dd->type    = DIRT_TYPE_PIGEON;
						dd->flag    = 0;
						dd->x       = cx;
						dd->y       = PAP_calc_height_at(cx, cz);
						dd->z       = cz;
						dd->UU.Pidgeon.state   = DIRT_PIGEON_WAIT;
						dd->counter = 16;
						dd->UU.Pidgeon.morph1  = MORPH_PIGEON_STAND;
						dd->UU.Pidgeon.morph2  = MORPH_PIGEON_STAND;
						dd->UU.Pidgeon.tween   = 0;
						dd->dx      = 0;
						dd->dy      = 0;
						dd->dz      = 0;
						dd->yaw     = Random() & 2047;
						dd->pitch   = 0;
						dd->roll    = 0;
						dd->dyaw    = 0;
						dd->dpitch  = 0;
						dd->droll   = 0;
#endif
						break;

					default:
						ASSERT(0);
						break;
				}
			}
			else
			{
				//
				// Couldn't find a place for this piece of dirt.
				//

				dd->type = DIRT_TYPE_UNUSED;
			}
		}
	}

	if (DIRT_focus_first)
	{
		DIRT_focus_first -= 1;
	}

}

#ifndef PSX
//
// Pigeon state initialisation functions.
//

void DIRT_pigeon_init_wait(DIRT_Dirt *dd)
{
	dd->UU.Pidgeon.state    = DIRT_PIGEON_WAIT;
	dd->counter  = Random() & 0x1f;
	dd->counter += 16;
	dd->UU.Pidgeon.morph1   = MORPH_PIGEON_STAND;	// Standing morph?
	dd->UU.Pidgeon.morph2   = MORPH_PIGEON_STAND;
	dd->UU.Pidgeon.tween    = Random() & 0xf;
	dd->UU.Pidgeon.tween   += 5;
}

void DIRT_pigeon_init_peck(DIRT_Dirt *dd)
{
	dd->UU.Pidgeon.state    = DIRT_PIGEON_PECK;
	dd->counter  = Random() & 0x3;
	dd->counter += 9;
	dd->UU.Pidgeon.morph1   = MORPH_PIGEON_STAND;	// Pecking morph?
	dd->UU.Pidgeon.morph2   = MORPH_PIGEON_PECK;
	dd->UU.Pidgeon.tween    = 0;
}

//
// Either starts walking or starts running.
//

void DIRT_pigeon_init_walk(DIRT_Dirt *dd)
{
	SLONG mx;
	SLONG mz;

	SLONG dest_x;
	SLONG dest_z;

	SLONG half_x;
	SLONG half_z;

	SLONG dx;
	SLONG dz;

	SLONG len;
	SLONG count = 0;

	while(1)
	{
		count += 1;

		if (count > 5)
		{
			//
			// Give up walking!
			//

			DIRT_pigeon_init_wait(dd);

			return;
		}

		//
		// Pick somewhere to walk to.
		//

		if (count == 1 && dd->UU.Pidgeon.state == DIRT_PIGEON_RUN)
		{
			//
			// Try walking in the direction you were running.
			//

			dest_x = dd->x + (dd->dx << 4);
			dest_z = dd->z + (dd->dz << 4);
		}
		else
		{
			dest_x = dd->x;
			dest_z = dd->z;

			dest_x += Random() & 0x3ff;
			dest_z += Random() & 0x3ff;

			dest_x -= 0x1ff;
			dest_z -= 0x1ff;
		}

		//
		// Good place to walk to?
		//

		mx = dest_x >> 8;
		mz = dest_z >> 8;

		if (PAP_on_map_hi(mx,mz))
		{
			if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
			{
				//
				// Inside a building!
				//
			}
			else
			{
				half_x = dd->x + dest_x >> 9;
				half_z = dd->z + dest_z >> 9;

				if (PAP_2HI(half_x,half_z).Flags & PAP_FLAG_HIDDEN)
				{
					//
					// Inside a building!
					//
				}
				else
				{
					//
					// Check for crossing colvects?
					//

					break;
				}
			}
		}
	}

	dx = dest_x - dd->x;
	dz = dest_z - dd->z;

	len = QDIST2(abs(dx),abs(dz)) + 1;

	dx  = (dx << 2) / len;
	dz  = (dz << 2) / len;

	dd->UU.Pidgeon.state = DIRT_PIGEON_WALK;
	dd->counter = len >> 2;
	dd->UU.Pidgeon.morph1  = MORPH_PIGEON_WALK1;
	dd->UU.Pidgeon.morph2  = MORPH_PIGEON_WALK2;
	dd->UU.Pidgeon.tween   = 0;
	dd->dx      = dx;
	dd->dz      = dz;
	dd->yaw     = calc_angle(-dx,-dz);
}

//
// You can only start hopping if you are walking.
//

#define DIRT_PIGEON_HOPUP	1
#define DIRT_PIGEON_HOPDOWN	2

void DIRT_pigeon_init_hop(DIRT_Dirt *dd, UBYTE upordown)
{
	ASSERT(dd->UU.Pidgeon.state == DIRT_PIGEON_WALK || dd->UU.Pidgeon.state == DIRT_PIGEON_RUN);

	dd->UU.Pidgeon.state    = DIRT_PIGEON_HOP;
	dd->counter += 4;					// Move forward slower while hopping.
	dd->UU.Pidgeon.morph1   = MORPH_PIGEON_WALK1;	// Standing morph?
	dd->UU.Pidgeon.morph2   = MORPH_PIGEON_WALK1;
	dd->UU.Pidgeon.tween    = 10;

	switch(upordown)
	{
		case DIRT_PIGEON_HOPUP:	  dd->dy = 20 << TICK_SHIFT; break;
		case DIRT_PIGEON_HOPDOWN: dd->dy =  8 << TICK_SHIFT; break;
		default:
			ASSERT(0);
	}
}


void DIRT_pigeon_init_flee(DIRT_Dirt *dd, SLONG scare_x, SLONG scare_z)
{
	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG dest_x;
	SLONG dest_z;

	SLONG half_x;
	SLONG half_z;

	SLONG len;
	SLONG overlen;

	//
	// Start running away. Try running in away from what's scary.
	//

	dx = dd->x - scare_x;
	dz = dd->z - scare_z;

	len     = QDIST2(abs(dx), abs(dz)) + 1;
	overlen = 65536 / len;

	dx = dx * overlen >> 8;
	dz = dz * overlen >> 8;

	dest_x = dd->x + dx;
	dest_z = dd->z + dz;

	//
	// Good place to walk to?
	//

	mx = dest_x >> 8;
	mz = dest_z >> 8;

	if (PAP_on_map_hi(mx,mz))
	{
		if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
		{
			//
			// Inside a building!
			//
		}
		else
		{
			half_x = dd->x + dest_x >> 9;
			half_z = dd->z + dest_z >> 9;

			if (PAP_2HI(half_x,half_z).Flags & PAP_FLAG_HIDDEN)
			{
				//
				// Inside a building!
				//
			}
			else
			{
				//
				// Check for crossing colvects?
				//

				dd->UU.Pidgeon.state   = DIRT_PIGEON_RUN;
				dd->counter = len >> 4;
				dd->UU.Pidgeon.morph1  = MORPH_PIGEON_WALK1;
				dd->UU.Pidgeon.morph2  = MORPH_PIGEON_WALK2;
				dd->UU.Pidgeon.tween   = 0;
				dd->dx      = dx >> 4;
				dd->dz      = dz >> 4;
				dd->yaw     = calc_angle(-dx,-dz);

				return;
			}
		}
	}

	//
	// Start flying?
	//

	return; //DIRT_pigeon_init_fly(dd);
}



//
// Pigeon state processing functions.
//

void DIRT_pigeon_process_wait(DIRT_Dirt *dd)
{
	if (dd->UU.Pidgeon.tween == 0)
	{
		if (dd->UU.Pidgeon.morph1 == MORPH_PIGEON_STAND)
		{
			dd->UU.Pidgeon.morph1   = MORPH_PIGEON_HEADCOCK;
			dd->UU.Pidgeon.morph2   = MORPH_PIGEON_HEADCOCK;

			dd->UU.Pidgeon.tween  = Random() & 0x7;
			dd->UU.Pidgeon.tween += 3;
		}
		else
		{
			dd->UU.Pidgeon.morph1 = MORPH_PIGEON_STAND;
			dd->UU.Pidgeon.morph2 = MORPH_PIGEON_STAND;
			dd->UU.Pidgeon.tween  = Random() & 0xf;
			dd->UU.Pidgeon.tween += 5;
		}
	}
	else
	{
		dd->UU.Pidgeon.tween -= 1;
	}
}

void DIRT_pigeon_process_peck(DIRT_Dirt *dd)
{
	if (dd->UU.Pidgeon.tween < 255) {dd->UU.Pidgeon.tween += 1;}
}

void DIRT_pigeon_process_walkrun(DIRT_Dirt *dd)
{
	dd->UU.Pidgeon.tween += abs(dd->dx) + abs(dd->dz) << 3;

	dd->x += dd->dx;
	dd->z += dd->dz;
	dd->y  = PAP_calc_height_at(dd->x, dd->z);

 	//
	// Are we about to hop up onto a curb?
	//

	SLONG mx1 = dd->x                >> 8;
	SLONG mz1 = dd->z                >> 8;
	SLONG mx2 = dd->x + (dd->dx * 4) >> 8;
	SLONG mz2 = dd->z + (dd->dz * 4) >> 8;

	ASSERT(WITHIN(mx1, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(mz1, 0, MAP_HEIGHT - 1));

	ASSERT(WITHIN(mx2, 0, MAP_WIDTH  - 1));
	ASSERT(WITHIN(mz2, 0, MAP_HEIGHT - 1));

	if (mx1 != mx2 || mz1 != mz2)
	{
		if ((PAP_2HI(mx1,mz1).Flags ^ PAP_2HI(mx2,mz2).Flags) & PAP_FLAG_SINK_SQUARE)
		{
			//
			// About to go up or down the curb.
			//

			DIRT_pigeon_init_hop(dd, (PAP_2HI(mx1,mz1).Flags & PAP_FLAG_SINK_SQUARE) ? DIRT_PIGEON_HOPUP : DIRT_PIGEON_HOPDOWN);
		}
	}
}

void DIRT_pigeon_process_hop(DIRT_Dirt *dd)
{
	if (dd->UU.Pidgeon.tween > 0)
	{
		//
		// Wait a while before hopping...
		//

		dd->UU.Pidgeon.tween   -= 1;
		dd->counter += 1;

		return;
	}

	dd->x += dd->dx >> 1;
	dd->y += dd->dy >> TICK_SHIFT;
	dd->z += dd->dz >> 1;

	dd->dy -= 3 * TICK_RATIO;

	//
	// Finished hopping?
	//

	SLONG height = PAP_calc_height_at(dd->x, dd->z);

	if (dd->y <= height)
	{
		//
		// Go back to walking.
		//

		dd->UU.Pidgeon.state  = DIRT_PIGEON_WALK;
		dd->UU.Pidgeon.morph1 = MORPH_PIGEON_WALK1;
		dd->UU.Pidgeon.morph2 = MORPH_PIGEON_WALK2;
		dd->UU.Pidgeon.tween  = 0;
	}
}

//
// Chooses an initialises a new state for the given pigeon.
// The new state chosen depends on the current state and
// the pigeon flags.
//

void DIRT_pigeon_start_doing_something_new(DIRT_Dirt *dd)
{
	SLONG i;
	SLONG num;
	SLONG total;
	SLONG state;

	ASSERT(dd->type == DIRT_TYPE_PIGEON);

	//
	// The chance array is the relative probability of going
	// into each state...
	//

	UBYTE chance[DIRT_PIGEON_NUM_STATES];

	memset(chance, 0, sizeof(chance));

	switch(dd->UU.Pidgeon.state)
	{
		case DIRT_PIGEON_WAIT:
			chance[DIRT_PIGEON_WAIT] = 2;
			chance[DIRT_PIGEON_PECK] = 2;
			chance[DIRT_PIGEON_WALK] = 2;
			break;

		case DIRT_PIGEON_PECK:
			chance[DIRT_PIGEON_WAIT] = 1;
			chance[DIRT_PIGEON_PECK] = 3;
			break;

		case DIRT_PIGEON_WALK:
			chance[DIRT_PIGEON_WAIT] = 1;
			chance[DIRT_PIGEON_PECK] = 1;
			break;

		case DIRT_PIGEON_HOP:

			//
			// Oh dear!
			//

			return;

		case DIRT_PIGEON_RUN:
			chance[DIRT_PIGEON_WALK] = 1;
			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Use the chance array to figure out which state to go into.
	//

	total = 0;

	for (i = 0; i < DIRT_PIGEON_NUM_STATES; i++)
	{
		total += chance[i];
	}

	num = Random() % total;

	for (i = 0; i < DIRT_PIGEON_NUM_STATES; i++)
	{
		if (num < chance[i])
		{
			state = i;
			break;
		}
		else
		{
			num -= chance[i];
		}
	}

	//
	// Initialise the chosen state.
	//

	switch(state)
	{
		case DIRT_PIGEON_WAIT: DIRT_pigeon_init_wait(dd); break;
		case DIRT_PIGEON_PECK: DIRT_pigeon_init_peck(dd); break;
		case DIRT_PIGEON_WALK: DIRT_pigeon_init_walk(dd); break;
		default:
			ASSERT(0);
			break;
	}
}



//
// Process a pigeon.
//

void DIRT_pigeon_process(DIRT_Dirt *dd)
{
	ASSERT(dd->type == DIRT_TYPE_PIGEON);

	if (dd->counter == 0)
	{
		//
		// Time to do something new.
		//

		DIRT_pigeon_start_doing_something_new(dd);
	}
	else
	{
		dd->counter -= 1;
	}

	switch(dd->UU.Pidgeon.state)
	{
		case DIRT_PIGEON_WAIT:
			DIRT_pigeon_process_wait(dd);
			break;

		case DIRT_PIGEON_PECK:
			DIRT_pigeon_process_peck(dd);
			break;

		case DIRT_PIGEON_WALK:
		case DIRT_PIGEON_RUN:
			DIRT_pigeon_process_walkrun(dd);
			break;

		case DIRT_PIGEON_HOP:
			DIRT_pigeon_process_hop(dd);
			break;

		default:
			ASSERT(0);
			break;
	}
}
#endif

void DIRT_new_water(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,
		SLONG dy,
		SLONG dz,
		SLONG dirt_type)
{
	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	//
	// Find a piece of dirt to replace this water.
	//

	dd = DIRT_find_useless();

	//
	// Randomise the direction a bit.
	//

	dx <<= 4;
	dy <<= 4;
	dz <<= 4;

	dx += (Random() & 0x1f);
	dy += (Random() & 0x1f);
	dz += (Random() & 0x1f);

	dx -= 0xf;
	dy -= 0xf;
	dz -= 0xf;

/*	if (urine_coloured)
	{
		dd->type = DIRT_TYPE_URINE;
	}
	else
	{
		dd->type = DIRT_TYPE_WATER;
	}*/
	dd->type=dirt_type;

	dd->flag = 0;
	dd->x    = x;
	dd->y    = y;
	dd->z    = z;
	dd->dx   = dx;
	dd->dy   = dy << TICK_SHIFT_LOWRES;
	dd->dz   = dz;
	dd->dyaw = 0;

	if (dd->type==DIRT_TYPE_SPARKS)
		dd->UU.ThingWithTime.timer=20+(Random()&7);
	else
		if (dd->type==DIRT_TYPE_BLOOD)
			dd->flag=DIRT_FLAG_HIT_FLOOR;

	return;
}


void DIRT_new_sparks(SLONG px, SLONG py, SLONG pz, UBYTE dir)
{
	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	SLONG dx,dy,dz,i,boost,nodrip;
	if (dir&32)
	{
		nodrip=1;
		dir&=~32;
	}
	if (dir&(128|64))
	{
		boost=dir>>6;
		dir&=63;
	} else boost=0;
	for (i=0;i<4;i++)
	{
		switch(dir)
		{
		case 0: // X+
			dx=5+(Random()&3);
			dy=(Random()&7)-3;
			dz=(Random()&7)-3;
			break;
		case 1: // X-
			dx=-(5+(Random()&3));
			dy=(Random()&7)-3;
			dz=(Random()&7)-3;
			break;
		case 2: // Y+
			dy=5+(Random()&3);
			dx=(Random()&7)-3;
			dz=(Random()&7)-3;
			break;
		case 3: // Y-
			dy=-(5+(Random()&3));
			dx=(Random()&7)-3;
			dz=(Random()&7)-3;
			break;
		case 4: // Z+
			dz=5+(Random()&3);
			dx=(Random()&7)-3;
			dy=(Random()&7)-3;
			break;
		case 5: // Z-
			dz=-(5+(Random()&3));
			dx=(Random()&7)-2;
			dy=(Random()&7)-2;
			break;
		}
		if (boost)
		{
			dx<<=boost; dy<<=boost; dz<<=boost;
		}
		if (!nodrip) DIRT_new_water(px , py, pz , dx, dy, dz,DIRT_TYPE_SPARKS);
		PARTICLE_Add(px<<8,py<<8,pz<<8,dx<<9,dy<<9,dz<<9,POLY_PAGE_EXPLODE1_ADDITIVE,2+((Random()&3)<<2),0x7Fffffff,PFLAG_FADE|PFLAG_GRAVITY|PFLAG_BOUNCE,20,10,1,2+(Random()&7),0);
	}
	if (Random()&1)
		PARTICLE_Add(px<<8,py<<8,pz<<8,0,0,0,POLY_PAGE_EXPLODE1_ADDITIVE,2+((Random()&3)<<2),0xFFffffff,PFLAG_FADE,2,15+(Random()&0x3f),1,0x7f,0);
	else
		PARTICLE_Add(px<<8,py<<8,pz<<8,0,0,0,POLY_PAGE_EXPLODE2_ADDITIVE,2+((Random()&1)<<2),0xFFffffff,PFLAG_FADE,2,15+(Random()&0x3f),1,0x7f,0);
}


void DIRT_spark_shower(DIRT_Dirt *dd)
{
	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	UBYTE i;
	for (i=0;i<5;i++)
	{
		if (Random()&1)
			PARTICLE_Add(dd->x<<8,(dd->y+10)<<8,dd->z<<8,((Random()&0x3f)-0x1f)<<4,((Random()&0x3f)+0x1f)<<4,((Random()&0x3f)-0x2f)<<4,POLY_PAGE_EXPLODE1_ADDITIVE,2|((Random()&3)<<2),0x7Fffffff,PFLAG_FADE|PFLAG_GRAVITY|PFLAG_BOUNCE,20,10,1,2+(Random()&7),0);
		else
			PARTICLE_Add(dd->x<<8,(dd->y+10)<<8,dd->z<<8,((Random()&0x3f)-0x1f)<<4,dd->dy>>4,((Random()&0x3f)-0x1f)<<4,POLY_PAGE_EXPLODE1_ADDITIVE,2|((Random()&3)<<2),0x7Fffffff,PFLAG_FADE|PFLAG_GRAVITY|PFLAG_BOUNCE,20,10,1,2+(Random()&7),0);
	}
}

void DIRT_process(void)
{
	SLONG i;
	SLONG dy;
	SLONG newy;
	SLONG floor;
	SLONG under;
	SLONG waftz;
	SLONG wafty;
	SLONG waftx;
	SLONG mx;
	SLONG mz;
	SLONG speed;
	SLONG oldx;
	SLONG oldy;
	SLONG oldz;
	SLONG collided;

	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_UNUSED)
		{
			continue;
		}

		if (dd->flag & DIRT_FLAG_STILL)
		{
			//
			// It is just lying on the ground.
			//

			if (dd->type==DIRT_TYPE_SNOW) // it fades over time
			{
				if (dd->UU.Leaf.fade>4)
					dd->UU.Leaf.fade-=4;
				else
					dd->type=DIRT_TYPE_UNUSED;
			}

			continue;
		}

		switch(dd->type)
		{
			case DIRT_TYPE_LEAF:
			case DIRT_TYPE_SNOW:

				oldx = dd->x;
				oldy = dd->y;
				oldz = dd->z;

				dd->x += (TICK_RATIO * dd->dx) >> TICK_SHIFT;
				dd->y += (TICK_RATIO * (dd->dy >> TICK_SHIFT)) >> TICK_SHIFT;
				dd->z += (TICK_RATIO * dd->dz) >> TICK_SHIFT;

				dd->yaw   += (TICK_RATIO * dd->dyaw)   >> TICK_SHIFT;
				dd->pitch += (TICK_RATIO * dd->dpitch) >> TICK_SHIFT;
				dd->roll  += (TICK_RATIO * dd->droll)  >> TICK_SHIFT;

				mx = dd->x >> 8;
				mz = dd->z >> 8;

				if (((oldx >> 8) != (dd->x >> 8)) || (oldz >> 8) != (dd->z >> 8) )
				{
					//
					// we have changed hires map cells
					//
					if (PAP_on_map_hi(mx,mz) && (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN))
					{
						//
						// The leaf has hit a wall... not touched the ground.
						//

						dd->x = oldx;
						dd->dx = -dd->dx;
						dd->z = oldz;
						dd->dz = -dd->dz;
					}
					else
					{
#ifdef	PSX
						dd->UU.Leaf.col=floor_psx_col[dd->x>>8][dd->z>>8];
#endif
					}

				}
				//
				// Don't go underground.
				//

				floor = PAP_calc_height_at(dd->x, dd->z);

				if (dd->y <= floor)
				{
					dd->y      = floor;
					dd->dy     = 0;
					dd->yaw    = 0;
					dd->roll   = 0;
					dd->pitch  = 0;
					dd->dyaw   = 0;
					dd->dpitch = 0;
					dd->droll  = 0;

					if (abs(dd->dx) <= 3) {dd->dx = 0;}
					if (abs(dd->dz) <= 3) {dd->dz = 0;}

					if (dd->dx == 0 && dd->dz == 0)
					{
						dd->flag |= DIRT_FLAG_STILL;
					}
				}

				//
				// A leaf has lots of resistance to moving and rotating.
				//

				dd->dx -= dd->dx / 4;
				dd->dy -= dd->dy / 2;
				dd->dz -= dd->dz / 4;

				dd->dpitch -= dd->dpitch / 32;
				dd->droll  -= dd->droll  / 32;

				//
				// Make it float downwards in a leaf-like fashion.
				//

				waftz = dd->pitch / 32;
				waftx = dd->roll  / 32;

				SATURATE(waftz, -0x5, +0x5);
				SATURATE(waftx, -0x5, +0x5);

				dd->dz -= waftz;
				dd->dx += waftx;

				dd->dpitch -= 0xa * SIGN(dd->pitch);
				dd->droll  -= 0xa * SIGN(dd->roll);

				dd->dy -= 4 << TICK_SHIFT;
				dd->dy += MIN(abs(dd->pitch), 180) << (TICK_SHIFT - 5);
				dd->dy += MIN(abs(dd->roll),  180) << (TICK_SHIFT - 5);

				break;


			case DIRT_TYPE_CAN:
			case DIRT_TYPE_HEAD:
			case DIRT_TYPE_BRASS:

				dd->yaw   += dd->dyaw;
				dd->pitch += dd->dpitch;

				dd->yaw   &= 2047;
				dd->pitch &= 2047;

				oldx = dd->x;
				oldz = dd->z;

				dd->x += dd->dx >> 8;
				dd->z += dd->dz >> 8;

				mx = dd->x >> 8;
				mz = dd->z >> 8;

				newy = PAP_calc_map_height_at(dd->x,dd->z) + 6;

				if (dd->type == DIRT_TYPE_BRASS)
				{
					newy-=3;
				}
				else
					if (dd->type == DIRT_TYPE_HEAD)
					{
						newy += 5;
					}

				dy = newy - dd->y;

				if (PAP_on_map_hi(mx,mz))
				{
					if ((dy > 8) && (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN))
					{
						//
						// Make it bounce and start spinning.
						//

						if ((dd->x - (dd->dx >> 8) >> 8) != mx) {dd->dx = -dd->dx;}
						if ((dd->z - (dd->dz >> 8) >> 8) != mz) {dd->dz = -dd->dz;}

						dd->x += dd->dx >> 7;
						dd->z += dd->dz >> 7;

						dd->dyaw  += 200;
						dd->dpitch = 0;

						//newy=dd->y; // um.
						break;
					}
				}

				if (dd->y>newy) {
					dd->dy-=4 << TICK_SHIFT;
					dd->y+=dd->dy >> TICK_SHIFT;
				} // no else in case it drops thru floor this frame
				if (dd->y<newy) {
				  dd->y   = newy;
				  if (dd->type != DIRT_TYPE_BRASS)
					if (dd->dy<-8 << TICK_SHIFT)
					{
#ifdef	MIKE
						ASSERT(0);
#endif
						MFX_play_xyz(i,S_KICK_CAN,MFX_REPLACE,dd->x<<8,dd->y<<8,dd->z<<8);
					}

				  dd->dy=0;
				}

				dd->dx -= dd->dx / 16;
				dd->dz -= dd->dz / 16;

				dd->dx -= SIGN(dd->dx);
				dd->dz -= SIGN(dd->dz);

				dd->dyaw   -= dd->dyaw   / 32;
				dd->dpitch -= dd->dpitch / 32;

				dd->dyaw   -= SIGN(dd->dyaw);
				dd->dpitch -= SIGN(dd->dpitch);

				if (dd->dx     == 0 &&
					dd->dz     == 0 &&
					dd->dyaw   == 0 &&
					dd->dpitch == 0)
				{
					dd->flag |= DIRT_FLAG_STILL;
				}

				break;

			case DIRT_TYPE_PIGEON:
#if !defined(PSX) && !defined(TARGET_DC)
				DIRT_pigeon_process(dd);
#endif
				break;

			case DIRT_TYPE_SPARKS:

				if (--dd->UU.ThingWithTime.timer<1)
				{
					DIRT_spark_shower(dd);
					dd->type=DIRT_TYPE_UNUSED;
					break;
				}

				// FALL THRU!
			case DIRT_TYPE_WATER:
			case DIRT_TYPE_URINE:
			case DIRT_TYPE_BLOOD:

				//
				// Gravity.
				//

				dy = dd->dy;
				dy -= 15 * TICK_RATIO >> 2;

				if ( (dy >> TICK_SHIFT_LOWRES) < -511 )
					dy = -511 << TICK_SHIFT_LOWRES;

				dd->dy = dy;

				oldx = dd->x;
				oldy = dd->y;
				oldz = dd->z;

				dd->x += (TICK_RATIO * (dd->dx + ((Random() & 0x1f) - 0xf))) / (1 << (TICK_SHIFT + 4));
				dd->y += (TICK_RATIO * ((dd->dy >> TICK_SHIFT_LOWRES) + ((Random() & 0x1f) - 0xf))) / (1 << (TICK_SHIFT + 4));
				dd->z += (TICK_RATIO * (dd->dz + ((Random() & 0x1f) - 0xf))) / (1 << (TICK_SHIFT + 4));

				mx = dd->x >> 8;
				mz = dd->z >> 8;

				collided = FALSE;

				if (PAP_on_map_hi(mx,mz))
				{
					/*
#if !defined(PSX) && !defined(TARGET_DC)
					if (GAME_FLAGS & GF_SEWERS)
					{
						collided = (dd->y < NS_calc_height_at(dd->x, dd->z) - 0x40);
					}
					else
#endif
					*/

					{
						collided = (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN);
					}
				}
				else
				{
					//
					// Stop the water going off the map.
					//

					collided = TRUE;
				}

				if (collided)
				{
					//
					// The water has hit a wall... not touched the ground.
					//

					if ((oldx >> 8) != (dd->x >> 8))
					{
						//
						// Travel in the dx direction must be stopped
						//

						dd->x  =  oldx;
						dd->dx = -dd->dx >> 2;
						//dd->dy =  SIN((i*97)&2047)>>(10-TICK_SHIFT_LOWRES);
						dd->dz =  COS((i*97)&2047)>>10;
					}

					if ((oldz >> 8) != (dd->z >> 8))
					{
						//
						// Travel in the dz direction must be stopped
						//

						dd->z  =  oldz;
						dd->dz = -dd->dz >> 2;
						//dd->dy =  SIN((i*97)&2047)>>(10-TICK_SHIFT_LOWRES);
						dd->dx =  COS((i*97)&2047)>>10;
					}
				}

#if !defined(PSX) && !defined(TARGET_DC)
				if (GAME_FLAGS & GF_SEWERS)
				{
					floor = NS_calc_splash_height_at(dd->x, dd->z);
				}
				else
#endif
				{
					floor = PAP_calc_map_height_at(dd->x, dd->z);
				}

				if (dd->y <= floor)
				{
					static SLONG tick = 0;

					dd->y = floor + 1;

					if(dd->flag & DIRT_FLAG_HIT_FLOOR)
					{
						//
						// Create a drip.
						//
#ifndef PSX
						if (dd->type!=DIRT_TYPE_BLOOD)
						{
							if (dd->type!=DIRT_TYPE_SPARKS)
							{
							  if (tick++ & 1) DRIP_create(dd->x, dd->y, dd->z, 0);
							}
							else
							  DIRT_spark_shower(dd);
						}
#endif
						//
						// Kill it. This is the second time it's hit
						// the floor.
						//

						dd->type = DIRT_TYPE_UNUSED;

					}
					else
					{
						dd->dy = abs(dd->dy) >> 1;
						dd->dx = SIN((i*97)&2047)>>10;
						dd->dz = COS((i*97)&2047)>>10;

						dd->flag |= DIRT_FLAG_HIT_FLOOR;

						//
						// Create a drip.
						//
#ifndef PSX
						if (dd->type!=DIRT_TYPE_BLOOD)
						{
							if (dd->type!=DIRT_TYPE_SPARKS)
							{
								if (tick++ & 1) DRIP_create(dd->x, dd->y, dd->z, 0);
							}
							else
							  DIRT_spark_shower(dd);
						}

#endif
					}
				}

				break;

			case DIRT_TYPE_HELDCAN:
			case DIRT_TYPE_HELDHEAD:

				//
				// Take values from the position of owner's right hand.
				//

				{
					SLONG px;
					SLONG py;
					SLONG pz;

					Thing *p_person = TO_THING(dd->droll);	// droll => owner

					calc_sub_objects_position(
						p_person,
						p_person->Draw.Tweened->AnimTween,
						SUB_OBJECT_PREFERRED_HAND,
					   &px,
					   &py,
					   &pz);

					px += p_person->WorldPos.X >> 8;
					py += p_person->WorldPos.Y >> 8;
					pz += p_person->WorldPos.Z >> 8;

					dd->x = px;
					dd->y = py;
					dd->z = pz;

					dd->yaw   = p_person->Draw.Tweened->Angle;
					dd->pitch = 0;
					dd->roll  = 0;
				}

				break;

			case DIRT_TYPE_THROWCAN:
			case DIRT_TYPE_THROWHEAD:
			case DIRT_TYPE_MINE:

				{
					dd->dy -= TICK_RATIO;

					if (dd->dy < -0x20 << TICK_SHIFT) {dd->dy = -0x20 << TICK_SHIFT;}

					oldx = dd->x;
					oldy = dd->y;
					oldz = dd->z;

					dd->x += dd->dx * TICK_RATIO >> TICK_SHIFT;
					dd->y += (dd->dy >> TICK_SHIFT) * TICK_RATIO >> TICK_SHIFT;
					dd->z += dd->dz * TICK_RATIO >> TICK_SHIFT;

					dd->yaw   += dd->dyaw;
					dd->pitch += dd->dpitch;

					floor = PAP_calc_map_height_at(dd->x, dd->z) + 6;

					if (dd->type != DIRT_TYPE_THROWCAN)
					{
						//
						// The head needs to be further off the ground.
						//

						floor += 5;
					}

					if (dd->y < floor)
					{
						under = floor - dd->y;

						if (abs(dd->dy) > 8 << TICK_SHIFT)
						{
							//
							// Make a can sound.
							//

							MFX_play_xyz(i,S_KICK_CAN,MFX_REPLACE,dd->x<<8,dd->y<<8,dd->z<<8);

							//
							// Alert guards.
							//

							PCOM_oscillate_tympanum(
								PCOM_SOUND_UNUSUAL,
								TO_THING(dd->droll),	// droll => who threw the coke can.
								oldx,
								oldy,
								oldz);
						}

						if ((under > 0x40)||(dd->dy>0))
						{
							//
							// Hit a building- but from which direction?
							//

							if ((oldx >> 8) != (dd->x >> 8))
							{
								//
								// Travel in the dx direction must be stopped
								//

								dd->x  =  oldx;
								dd->dx = -dd->dx >> 1;
							}

							if ((oldz >> 8) != (dd->z >> 8))
							{
								//
								// Travel in the dz direction must be stopped
								//

								dd->z  =  oldz;
								dd->dz = -dd->dz >> 1;
							}
						}
						else
						{
							//
							// Hit the floor.
							//

							dd->dy = abs(dd->dy) >> 1;
							dd->y  = 2 * floor - dd->y;

							if (abs(dd->dy) < 10 << TICK_SHIFT)
							{
								if (dd->type == DIRT_TYPE_THROWCAN)
								{
									//
									// Become a normal coke-can.
									//

									dd->dy   = 0;
									dd->type = DIRT_TYPE_CAN;

									dd->dx += (Random() & 0xf) - 0x7;
									dd->dz += (Random() & 0xf) - 0x7;

									dd->dx <<= 8;
									dd->dz <<= 8;

									dd->dyaw   = (Random() & 0x7f) + 0x7f;
									dd->dpitch = (Random() & 0x3f) + 0x3f;
								}
								else
								if (dd->type == DIRT_TYPE_THROWHEAD)
								{
									//
									// Become a head resting on the ground.
									//

									dd->type = DIRT_TYPE_HEAD;

									dd->dx = 0;
									dd->dy = 0;
									dd->dz = 0;

									dd->dyaw   = 0;
									dd->dpitch = 0;
								}
							}
							else
							{
								dd->dx += (Random() & 0xf) - 0x7;
								dd->dz += (Random() & 0xf) - 0x7;

								dd->dyaw   += (Random() & 0x7f) - 0x3f;
								dd->dpitch += (Random() & 0x3f);
							}
						}
					}

					/*

					if ((GAME_TURN & 0x7) == 0)
					{
						if (abs(dd->dx) < 8) {if (dd->dx) dd->dx--;} else {dd->dx -= dd->dx / 8;}
						if (abs(dd->dy) < 8) {if (dd->dy) dd->dy--;} else {dd->dy -= dd->dy / 8;}
						if (abs(dd->dz) < 8) {if (dd->dz) dd->dz--;} else {dd->dz -= dd->dz / 8;}
					}

					*/
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}
}


void DIRT_gust(
		Thing *p_thing,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG push;
	SLONG pushx;
	SLONG pushy;
	SLONG pushz;

	SLONG dpitch;
	SLONG droll;

	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	//
	// Near enough to be of interest?
	//

	dx = DIRT_focus_x - x1;
	dz = DIRT_focus_z - z1;

	dist = abs(dx) + abs(dz);

//	if (dist > (DIRT_focus_radius >> 1))
	if (dist > (DIRT_focus_radius))
	{
		return;
	}

	SLONG dgx = x2 - x1;
	SLONG dgz = z2 - z1;

	dgx -= dgx / 4;
	dgz -= dgz / 4;

	SLONG strength;

	//
	// Strength is radius around point1 in which dirt is effected.
	//

	strength  = QDIST2(abs(dgx),abs(dgz));
	strength += 1;
	strength *= 8;

	#if BIKE

	// blatant bike hack
	if (p_thing&&p_thing->Class==CLASS_BIKE) {
		dgx*=2;
		dgz*=2;
	}

	#endif

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_UNUSED)
		{
			continue;
		}

		dx = dd->x - x1;
		dz = dd->z - z1;

		dist = QDIST2(abs(dx), abs(dz)) + 1;

		if (strength > dist)
		{
			//
			// This bit of dirt is effected by the gust.
			//

			switch(dd->type)
			{
				case DIRT_TYPE_SNOW:
//	  				break; // fall thru -- (un)comment this to toggle snow being affected by gusts

				case DIRT_TYPE_LEAF:

					//
					// Move the leaf away from the gust and
					// in the direction of the gust.
					//

					pushx  = dx * 32 / dist;
					pushz  = dz * 32 / dist;

					pushx += dgx;
					pushz += dgz;

					push = 256 - dist *  256 / strength;

					if (push)
					{
						//
						// Push the leaf.
						//

						pushx = pushx * push >> 12;
						pushz = pushz * push >> 12;

						pushy   = abs(pushx) + abs(pushz);
						pushy <<= 1;

						//
						// Set the leaf spinning.
						//

						dpitch = (Random() & 0x3f) - 0x1f;
						droll  = (Random() & 0x3f) - 0x1f;

						//
						// Depending on the frame speed...
						//

						pushx = pushx * TICK_INV_RATIO >> TICK_SHIFT;
						pushy = pushy * TICK_INV_RATIO >> TICK_SHIFT;
						pushz = pushz * TICK_INV_RATIO >> TICK_SHIFT;

						dpitch = dpitch * TICK_INV_RATIO >> TICK_SHIFT;
						droll  = droll  * TICK_INV_RATIO >> TICK_SHIFT;

						dd->dx += pushx;
						dd->dy += pushy << TICK_SHIFT;
						dd->dz += pushz;

						dd->dpitch += dpitch;
						dd->droll  += droll;

						dd->flag &= ~DIRT_FLAG_STILL;
					}

					break;

				case DIRT_TYPE_CAN:
				case DIRT_TYPE_HEAD:
				case DIRT_TYPE_BRASS:

					if (dist < 32)
					{
						//
						// Kick the can...
						//

						dd->dx      =  (dx << 13) / dist;
						dd->dz      =  (dz << 13) / dist;
						dd->dyaw    =  Random() & 0xff;
						dd->dyaw   +=  0x7f;
						dd->dpitch  = -400;
						dd->flag    =  0;

						if (dd->type != DIRT_TYPE_BRASS)
						{
							MFX_play_xyz(i,S_KICK_CAN,MFX_REPLACE,dd->x<<8,dd->y<<8,dd->z<<8);
//							MFX_play_xyz(i,S_DARCI_ARREST,MFX_REPLACE,dd->x<<8,dd->y<<8,dd->z<<8);

							//
							// Alert guards.
							//

							PCOM_oscillate_tympanum(
								PCOM_SOUND_UNUSUAL,
								p_thing,
								x1,
								0,	// The y-altitude is ignored anyway!
								z1);
						}
					}

					break;

				case DIRT_TYPE_PIGEON:

					//
					// Scare the pigeon?
					//
#ifndef PSX
					DIRT_pigeon_init_flee(dd, x1, z1);
#endif
					break;

				case DIRT_TYPE_WATER:
					break;

				case DIRT_TYPE_HELDCAN:
					break;

				case DIRT_TYPE_THROWCAN:
					break;

				case DIRT_TYPE_HELDHEAD:
					break;

				case DIRT_TYPE_THROWHEAD:
					break;

				case DIRT_TYPE_MINE:
					break;

				case DIRT_TYPE_URINE:
					break;

				case DIRT_TYPE_SPARKS:
					break;

				case DIRT_TYPE_BLOOD:
					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}
}



SLONG DIRT_get_nearest_can_or_head_dist(SLONG x, SLONG y, SLONG z)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;
	SLONG best_dist = INFINITY;

	DIRT_Dirt *dd;

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_CAN)
		{
			dx = abs(dd->x - x);
			dy = abs(dd->y - y);
			dz = abs(dd->z - z);

			dist = QDIST3(dx,dy,dz);

			if (dist < best_dist)
			{
				best_dist = dist;
			}
		}
	}

	return best_dist;
}


void DIRT_pick_up_can_or_head(Thing *p_person)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;

	SLONG best_can  = NULL;
	SLONG best_dist = INFINITY;

	DIRT_Dirt *dd;

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_CAN || dd->type == DIRT_TYPE_HEAD)
		{
			dx = abs(dd->x - (p_person->WorldPos.X >> 8));
			dy = abs(dd->y - (p_person->WorldPos.Y >> 8));
			dz = abs(dd->z - (p_person->WorldPos.Z >> 8));

			dist = QDIST3(dx,dy,dz);

			if (dist < best_dist)
			{
				best_can  = i;
				best_dist = dist;
			}
		}
	}

	if (best_can && best_dist < 0x80)
	{
		dd = &DIRT_dirt[best_can];

		ASSERT(dd->type == DIRT_TYPE_CAN);

		dd->type   =  DIRT_TYPE_HELDCAN;
		dd->droll  =  THING_NUMBER(p_person);		// droll => owner
		dd->flag  &= ~DIRT_FLAG_STILL;

		p_person->Genus.Person->Flags |= FLAG_PERSON_CANNING;
		p_person->Genus.Person->Hold   = best_can;
	}

	return;
}

void DIRT_release_can_or_head(Thing *p_person, SLONG power)	// 0 <= power <= 256
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG vector[3];

	DIRT_Dirt *dd;

	//
	// The speed of the coke can.
	//

	FMATRIX_vector(
		vector,
		p_person->Draw.Tweened->Angle,
		0);

	dx = -vector[0] * power >> 18;
	dz = -vector[2] * power >> 18;

	dy = power >> 2;

	//
	// Set the coke can to become a projectile.
	//

	ASSERT(WITHIN(p_person->Genus.Person->Hold, 0, DIRT_MAX_DIRT - 1));

	dd = &DIRT_dirt[p_person->Genus.Person->Hold];

	ASSERT(dd->type == DIRT_TYPE_HELDCAN);

	dd->type   =  DIRT_TYPE_THROWCAN;
	dd->dx     =  dx;
	dd->dy     =  dy << TICK_SHIFT;
	dd->dz     =  dz;
	dd->dyaw   =  (Random() & 0x3f) - 0x1f;
	dd->dpitch =  50;

	//
	// dd->droll contains the index of the person who held the can.
	//

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_CANNING;
}



SLONG DIRT_get_info(SLONG which,DIRT_Info *ans)
{
	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return FALSE;
	}

	DIRT_Dirt *dd;

	ASSERT(WITHIN(which, 0, DIRT_MAX_DIRT - 1));

	dd = &DIRT_dirt[which];

	dd->flag &= ~DIRT_FLAG_DELETE_OK;

	switch(dd->type)
	{
		case DIRT_TYPE_UNUSED:
			return(0);
			ans->type  = DIRT_INFO_TYPE_UNUSED;
			break;

		case DIRT_TYPE_WATER:
			ans->type  = DIRT_INFO_TYPE_WATER;
			// FALL THRU
		case DIRT_TYPE_SPARKS:
			if (dd->type == DIRT_TYPE_SPARKS)
			{
				ans->type = DIRT_INFO_TYPE_SPARKS;
			}
			// FALL THRU
		case DIRT_TYPE_BLOOD:
			if (dd->type == DIRT_TYPE_BLOOD)
			{
				ans->type = DIRT_INFO_TYPE_BLOOD;
			}
			// FALL THRU
		case DIRT_TYPE_URINE:
			ans->x     = dd->x;
			ans->y     = dd->y;
			ans->z     = dd->z;
			ans->dx    = dd->dx >> 4;
			ans->dy    = dd->dy >> (TICK_SHIFT_LOWRES + 4);
			ans->dz    = dd->dz >> 4;

			if (dd->type == DIRT_TYPE_URINE)
			{
				ans->type  = DIRT_INFO_TYPE_URINE;
			}

			break;

		case DIRT_TYPE_LEAF:
		case DIRT_TYPE_SNOW:
			ans->tween = dd->UU.Leaf.col;
			if (dd->type==DIRT_TYPE_LEAF)
				ans->type  = DIRT_INFO_TYPE_LEAF;
			else
				ans->type  = DIRT_INFO_TYPE_SNOW;
			ans->yaw   = dd->yaw;
			ans->pitch = dd->pitch;
			ans->roll  = dd->roll;
			ans->x     = dd->x;
			ans->y     = dd->y;
			ans->z     = dd->z;
			ans->morph1= dd->UU.Leaf.fade;
			break;

		case DIRT_TYPE_CAN:
		case DIRT_TYPE_THROWCAN:
			ans->type  = DIRT_INFO_TYPE_PRIM;
			ans->prim  = PRIM_OBJ_CAN;
			ans->held  = FALSE;
			ans->yaw   = dd->yaw;
			ans->pitch = dd->pitch;
			ans->roll  = dd->roll;
			ans->x     = dd->x;
			ans->y     = dd->y;
			ans->z     = dd->z;
			break;

		case DIRT_TYPE_HELDCAN:
			{
				Thing *p_person = TO_THING(dd->droll);	// droll => owner

				if(p_person->Genus.Person->InCar)
					return(0);


				ans->type  = DIRT_INFO_TYPE_PRIM;
				ans->prim  = PRIM_OBJ_CAN;
				ans->held  = TRUE;
				ans->yaw   = dd->yaw;
				ans->pitch = dd->pitch;
				ans->roll  = dd->roll;
				ans->x     = dd->x;
				ans->y     = dd->y;
				ans->z     = dd->z;
				break;
			}

		case DIRT_TYPE_PIGEON:
#ifndef PSX
			ans->type   = DIRT_INFO_TYPE_MORPH;
			ans->prim   = PRIM_OBJ_ITEM_KEY;
			ans->morph1 = dd->UU.Pidgeon.morph1;
			ans->morph2 = dd->UU.Pidgeon.morph2;
			ans->tween  = dd->UU.Pidgeon.tween;
			ans->yaw    = dd->yaw;
			ans->pitch  = dd->pitch;
			ans->roll   = dd->roll;
			ans->x      = dd->x;
			ans->y      = dd->y;
			ans->z      = dd->z;

			if (ans->tween == 255)
			{
				//
				// Tween is only a UBYTE where it should really be a UWORD.
				//

				ans->tween = 256;
			}
#endif
			break;

		case DIRT_TYPE_HEAD:
		case DIRT_TYPE_THROWHEAD:
		case DIRT_TYPE_HELDHEAD:
		case DIRT_TYPE_BRASS:
			ans->type  = DIRT_INFO_TYPE_PRIM;
			ans->prim  = dd->UU.Head.prim;
			ans->held  = FALSE;
			ans->yaw   = dd->yaw;
			ans->pitch = dd->pitch;
			ans->roll  = dd->roll;
			ans->x     = dd->x;
			ans->y     = dd->y;
			ans->z     = dd->z;
			break;

		case DIRT_TYPE_MINE:
			ans->type  = DIRT_INFO_TYPE_UNUSED;
			ans->yaw   = dd->yaw;
			ans->pitch = dd->pitch;
			ans->roll  = dd->roll;
			ans->x     = dd->x;
			ans->y     = dd->y;
			ans->z     = dd->z;
			return (0);

		default:
			ASSERT(0);
			break;
	}

	return(1);
}

/*
void DIRT_gale_height(SLONG dx,SLONG dy,SLONG dz)
{
	SLONG i;

	SLONG pushx;
	SLONG pushy;
	SLONG pushz;

	SLONG dyaw;
	SLONG dpitch;
	SLONG droll;

	DIRT_Dirt *dd;

	if (dx == 0 &&
		dz == 0)
	{
		//
		// Early out..
		//

		return;
	}

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_UNUSED)
		{
			continue;
		}

		switch(dd->type)
		{
			case DIRT_TYPE_LEAF:
			case DIRT_TYPE_SNOW:

				pushx   = dx >> 1;
				pushz   = dz >> 1;
				pushy   = (Random() & 0xff);
				pushy  *= pushy * abs(dx);
				pushy >>= 16;


				if (dy!=0xFFFFFF)
				{
					SLONG ydiff=abs(dy-dd->y);
					SATURATE(ydiff,0,256);
					ydiff=256-ydiff;
					pushx=(pushx*ydiff)>>8;
					pushz=(pushz*ydiff)>>8;
					ydiff*=2;
					SATURATE(ydiff,0,256);
					pushy=(pushy*ydiff)>>8;
					if ((dd->flag&DIRT_FLAG_STILL)&&(pushy<10)) break;
				}

				dpitch = (Random() & 0x7) - 0x3;
				droll  = (Random() & 0x7) - 0x3;

				dd->dx += pushx;
				dd->dy += pushy << TICK_SHIFT;
				dd->dz += pushz;

				dd->dpitch += dpitch;
				dd->droll  += droll;

				dd->flag &= ~DIRT_FLAG_STILL;

				break;

			case DIRT_TYPE_CAN:

				pushx  = (Random() & 0xff) * dx >> 3;
				pushz  = (Random() & 0xff) * dz >> 3;

				dd->dx += pushx;
				dd->dz += pushz;

				if (abs(dd->dx) + abs(dd->dz) > 100)
				{
					dpitch = Random() & 0x3;

					dd->dpitch += dpitch;

					//
					// Occasionally do a spinny thing like cans do.
					//

					if ((Random() & 0x7f) == (i & 0x7f))
					{
						dd->dx >>= 3;
						dd->dy >>= 3;

						dd->dyaw  += 1;
						dd->dyaw <<= 3;
					}
				}

				dd->flag &= ~DIRT_FLAG_STILL;

				break;

			case DIRT_TYPE_PIGEON:
				break;

			case DIRT_TYPE_WATER:
				break;

			case DIRT_TYPE_URINE:
			case DIRT_TYPE_BLOOD:
				break;

			case DIRT_TYPE_SPARKS:
				break;

			default:
				ASSERT(0);
				break;
		}
	}
}


void DIRT_gale(SLONG dx,SLONG dz)
{
	DIRT_gale_height(dx,0xFFFFFF,dz);
}
*/

void DIRT_mark_as_offscreen(SLONG which)
{
	DIRT_Dirt *dd;

	ASSERT(WITHIN(which, 0, DIRT_MAX_DIRT - 1));

	dd = &DIRT_dirt[which];

	if (dd->type == DIRT_TYPE_LEAF && dd->owner != 255)
	{
		//
		// This leaf is owned by a tree, so we shouldn't delete it.
		// We should let the tree delete it.
		//
	}
	else
	{
		DIRT_dirt[which].flag |= DIRT_FLAG_DELETE_OK;
	}
}


SLONG DIRT_shoot(Thing *p_person)
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG angle;
	SLONG dangle;

	SLONG score;
	SLONG best_dirt;
	SLONG best_score = -INFINITY;

	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return FALSE;
	}

	//
	// Look for nearby thrown coke cans.
	//

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_THROWCAN ||
			dd->type == DIRT_TYPE_THROWHEAD)
		{
			dx = dd->x - (p_person->WorldPos.X >> 8);
			dz = dd->z - (p_person->WorldPos.Z >> 8);

			angle  = calc_angle(dx,dz);
			angle += 1024;
			angle &= 2047;

			dist   = abs(dx) + abs(dz);
			dangle = angle - p_person->Draw.Tweened->Angle;

			if (dangle < -1024) {dangle += 2048;}
			if (dangle > +1024) {dangle -= 2048;}

			if (WITHIN(dangle, -200, +200) && dist < 256 * 5)
			{
				score = INFINITY - (dangle << 8) - (dist << 4);

				if (score > best_score)
				{
					best_score = score;
					best_dirt  = i;
				}
			}
		}
	}

	if (best_score > -INFINITY)
	{
		ASSERT(WITHIN(best_dirt, 0, DIRT_MAX_DIRT - 1));

		//
		// Found a piece of dirt to shoot.
		//

		dd = &DIRT_dirt[best_dirt];

		switch(dd->type)
		{
			case DIRT_TYPE_THROWCAN:
			case DIRT_TYPE_THROWHEAD:

				dd->dx = (Random() & 0x1f) - 0xf;
				dd->dz = (Random() & 0x1f) - 0xf;

				dd->dy += (Random() & 0xf) << TICK_SHIFT;
				dd->dy += 0xf << TICK_SHIFT;

				dd->dyaw  = Random() & 0x3f;
				dd->dyaw += 0x7f;

				if (Random() & 0x2)
				{
					dd->yaw = -dd->yaw;
				}
#ifndef PSX
				// apply a hitspang to the can
				PYRO_hitspang(p_person,dd->x<<8,dd->y<<8,dd->z<<8);
#endif
				MFX_play_xyz(best_dirt,S_KICK_CAN,MFX_REPLACE,dd->x<<8,dd->y<<8,dd->z<<8);
/*				play_quick_wave_xyz(
					dd->x << 8,
					dd->y << 8,
					dd->z << 8,
					S_KICK_CAN,
					best_dirt,
					WAVE_PLAY_INTERUPT);*/

				break;

			default:
				break;
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



void DIRT_behead_person(Thing *p_person, Thing *p_attacker)
{
	#ifdef BEHEAD

	SLONG i;
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;
	SLONG dz;
	SLONG score;
	SLONG dirt;

	SLONG best_dirt  = 0;
	SLONG best_score = 0;

	DIRT_Dirt *dd;

	//
	// Pick a random bit of dirt.
	//

	for (i = 0; i < 5; i++)
	{
		dirt = Random() % DIRT_MAX_DIRT;
		dd   = &DIRT_dirt[dirt];

		dx = dd->x - (p_person->WorldPos.X >> 8);
		dz = dd->z - (p_person->WorldPos.Z >> 8);

		score = abs(dx) + abs(dz);

		if (best_score < score)
		{
			best_score = score;
			best_dirt  = dirt;
		}
	}

	ASSERT(WITHIN(best_dirt, 0, DIRT_MAX_DIRT - 1));

	dd = &DIRT_dirt[best_dirt];

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_HEAD,
	   &x,
	   &y,
	   &z);

	dd->type         =  DIRT_TYPE_THROWHEAD;
	dd->UU.Head.prim =  prim_multi_objects[p_person->Draw.Tweened->TheChunk->MultiObject[p_person->Draw.Tweened->MeshID]].StartObject + p_person->Draw.Tweened->TheChunk->PeopleTypes[p_person->Draw.Tweened->PersonID].BodyPart[SUB_OBJECT_HEAD];
	dd->flag        &= ~DIRT_FLAG_STILL;

	dd->x = x + (p_person->WorldPos.X >> 8);
	dd->y = y + (p_person->WorldPos.Y >> 8);
	dd->z = z + (p_person->WorldPos.Z >> 8);

	dd->yaw   = 0;
	dd->pitch = 0;
	dd->roll  = 0;

	dd->dyaw   = 0;//rand() & 0xf;
	dd->dpitch = 0;//rand() & 0xf;
	dd->droll  = 0;

	if (p_attacker == NULL)
	{
		dd->dx = (rand() & 0xf) - 0x7;
		dd->dz = (rand() & 0xf) - 0x7;

		dd->dy = 10 << TICK_SHIFT;
	}
	else
	{
		dd->dx = 0;
		dd->dy = 80 << TICK_SHIFT;
		dd->dz = 0;
	}

	//
	// This person doesn't have a head anymore.
	//

	p_person->Genus.Person->Flags |= FLAG_PERSON_BEHEADED;

	#endif
}

#ifndef PSX
UWORD DIRT_create_mine(Thing *p_person)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG power = 128;

	SLONG vector[3];

	DIRT_Dirt *dd = DIRT_find_useless();

	//
	// The speed of the mine.
	//

	FMATRIX_vector(
		vector,
		p_person->Draw.Tweened->Angle,
		0);

	dx = -vector[0] * power >> 18;
	dz = -vector[2] * power >> 18;

	dy = power >> 2;

	//
	// Set the dirt to become a mine.
	//

	dd->type   = DIRT_TYPE_MINE;
	dd->flag   = 0;
	dd->dx     = dx;
	dd->dy     = dy << TICK_SHIFT;
	dd->dz     = dz;
	dd->dyaw   = (Random() & 0x3f) - 0x1f;
	dd->dpitch = 50;

	SLONG px;
	SLONG py;
	SLONG pz;

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_HAND,
	   &px,
	   &py,
	   &pz);

	px += p_person->WorldPos.X >> 8;
	py += p_person->WorldPos.Y >> 8;
	pz += p_person->WorldPos.Z >> 8;

	dd->x = px;
	dd->y = py;
	dd->z = pz;

	//
	// dd->droll contains the index of the person who threw the mine... why?
	//

	dd->droll = THING_NUMBER(p_person);

	return dd - DIRT_dirt;
}

void DIRT_destroy_mine(UWORD dirt_mine)
{
	ASSERT(WITHIN(dirt_mine, 0, DIRT_MAX_DIRT - 1));

	DIRT_dirt[dirt_mine].type = DIRT_TYPE_UNUSED;
}
#endif

void DIRT_create_papers(
		SLONG x,
		SLONG y,
		SLONG z)
{
	SLONG i;
	SLONG created;

	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	//
	// Look for unused bits of newspaper and crisp packets.
	//

	created = 0;

	for (i = 0; i < DIRT_MAX_DIRT; i += 16)
	{
		dd = &DIRT_dirt[i];

		if (dd->type == DIRT_TYPE_UNUSED || (dd->flag &  DIRT_FLAG_DELETE_OK))
		{
			dd->type   = DIRT_TYPE_LEAF;
			dd->x      = x + (Random() & 0x3f) - 0x1f;
			dd->z      = z + (Random() & 0x3f) - 0x1f;
			dd->y      = y + (Random() & 0x1f);
			dd->dy     = Random() & 0x7;
			dd->droll  = (Random() & 0x7f) - 0x3f;
			dd->dpitch = (Random() & 0x7f) - 0x3f;
			dd->owner  = 255;
			dd->dx     = 0;
			dd->dy     = 0;
			dd->dz     = 0;
			dd->flag   = 0;

			created += 1;

			if (created >= 4)
			{
				return;
			}
		}
	}
}

void DIRT_create_cans(
		SLONG x,
		SLONG z,
		SLONG angle)
{
	SLONG i;
	SLONG useangle;

	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	for (i = 0; i < 5; i++)
	{
		dd = DIRT_find_useless();

		if (dd == NULL)
		{
			return;
		}

		useangle  = angle;
		useangle += Random() & 0x7f;
		useangle -= 0x3f;
		useangle &= 2047;

		dd->type   =  DIRT_TYPE_CAN;
		dd->flag   =  0;
		dd->x      =  x + (Random() & 0x3f) - 0x1f;
		dd->y      =  PAP_calc_height_at(x,z) + 4;
		dd->z      =  z + (Random() & 0x3f) - 0x1f;
		dd->dx     =  SIN(useangle) * (Random() & 0xff) >> 13;
		dd->dy     =  0;
		dd->dz     =  COS(useangle) * (Random() & 0xff) >> 13;
		dd->yaw    =  Random() & 2047;
		dd->pitch  = -400;
		dd->roll   =  0;
		dd->dyaw   =  (Random() & 0xff) - 0x7f;
		dd->dpitch =  0;
		dd->droll  =  0;
	}
}

#ifndef PSX
void DIRT_create_brass(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG angle)
{
	SLONG i;
	SLONG useangle;

	DIRT_Dirt *dd;

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		//
		// No dirt if there is no floor!
		//

		return;
	}

	{
		dd = DIRT_find_useless();

		if (dd == NULL)
		{
			return;
		}

		useangle  = angle;
		useangle += Random() & 0x7f;
		useangle -= 0x3f;
		useangle &= 2047;

		i=0x7f-(Random()&0x3f);

		dd->type   =  DIRT_TYPE_BRASS;
		dd->UU.Head.prim = 253;
		dd->flag   =  0;
		dd->x      =  x;
		dd->y      =  y;
		dd->z      =  z;
		dd->dx     =  (SIN(useangle) * i) >> 11;
		dd->dy     =  24 << TICK_SHIFT;
		dd->dz     =  (COS(useangle) * i) >> 11;
		dd->yaw    =  (useangle+512)&2047;
		dd->pitch  =  0;
		dd->roll   =  0;
//		dd->dyaw   =  (Random() & 0xff) - 0x7f;
		dd->dyaw   =  0;
		dd->dpitch =  0;
		dd->droll  =  0;
	}
}

#endif
