#include <MFStdLib.h>
#include "game.h"
#include "spark.h"
#include "glitter.h"
#include "sound.h"
#include "pap.h"
#include "mfx.h"
#include "memory.h"

//
// The points at the start and end of each spark.
//

typedef struct
{
	UBYTE type;
	UBYTE flag;
	UBYTE dist;
	SBYTE dx;
	SBYTE dy;
	SBYTE dz;

	union
	{
		struct
		{
			UWORD x;
			UWORD y;
			UWORD z;
		}Pos;

		struct
		{
			UWORD person;
			UWORD limb;
		}Peep;
	};
	
} SPARK_Point;

typedef struct
{
	UBYTE used;
	UBYTE die;
	UBYTE num_points;
	UBYTE next;
	UBYTE map_z;
	UBYTE map_x;
	UBYTE glitter;
	UBYTE useless_padding;

	SPARK_Point point[4];
	
} SPARK_Spark;

#define SPARK_MAX_SPARKS 32

SPARK_Spark SPARK_spark[SPARK_MAX_SPARKS];
SLONG       SPARK_spark_last;

//
// The spark mapwho.
//

UBYTE SPARK_mapwho[MAP_HEIGHT];


void SPARK_init()
{
	SLONG i;

	//
	// Mark all the sparks as unused.
	//

	for (i = 0; i < SPARK_MAX_SPARKS; i++)
	{
		SPARK_spark[i].used = FALSE;
	}

	SPARK_spark_last = 1;

	//
	// Clear the mapwho.
	//

	for (i = 0; i < MAP_HEIGHT; i++)
	{
		SPARK_mapwho[i] = NULL;
	}
}

//
// Initialises the movement vector of the given point.
//

void SPARK_point_set_velocity(SPARK_Point *sp)
{
	if (sp->type == SPARK_TYPE_LIMB)
	{
		//
		// The values are ignored... so we dont  even have to
		// set them to zero if we don't want to.
		//

		sp->dx = 0;
		sp->dy = 0;
		sp->dz = 0;

		return;
	}
	else
	{
		SLONG dx = (rand() & 0xff) - 0x1f;
		SLONG dy = (rand() & 0xff) - 0x1f;
		SLONG dz = (rand() & 0xff) - 0x1f;

		if (sp->flag & SPARK_FLAG_FAST)
		{
			dx <<= 2;
			dy <<= 2;
			dz <<= 2;
		}
		else
		if (sp->flag & SPARK_FLAG_SLOW)
		{
			dx >>= 2;
			dy >>= 2;
			dz >>= 2;
		}

		SATURATE(dx, -127, +127);
		SATURATE(dy, -127, +127);
		SATURATE(dz, -127, +127);

		if (sp->flag & SPARK_FLAG_CLAMP_X) {dx = 0;}
		if (sp->flag & SPARK_FLAG_CLAMP_Y) {dy = 0;}
		if (sp->flag & SPARK_FLAG_CLAMP_Z) {dz = 0;}

		sp->dx = dx;
		sp->dy = dy;
		sp->dz = dz;
	}
}


//
// Find the (x,y,z) of the given point.
//

void SPARK_point_pos(
		SPARK_Point *sp,
		SLONG       *px,
		SLONG       *py,
		SLONG       *pz)
{
	CollisionVect *p_vect;

	if (sp->type == SPARK_TYPE_LIMB)
	{
		Thing *p_person = TO_THING(sp->Peep.person);

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			sp->Peep.limb,
			px,
			py,
			pz);

		*px += p_person->WorldPos.X >> 8;
		*py += p_person->WorldPos.Y >> 8;
		*pz += p_person->WorldPos.Z >> 8;
	}
	else
	{
		*px = sp->Pos.x;
		*py = sp->Pos.y;
		*pz = sp->Pos.z;

		if (sp->type == SPARK_TYPE_GROUND)
		{
			*py = PAP_calc_map_height_at(sp->Pos.x, sp->Pos.z);
		}
	}
}


//
// Fills in the point structure with a new point somewhere
// near the given sphere.
//

void SPARK_new_point(
		SLONG mid_x,
		SLONG mid_y,
		SLONG mid_z,
		SLONG radius,
		SPARK_Point *sp)
{
	SLONG i;

	//
	// Initialise with a circular spark going into the air somewhere.
	//

	SLONG best_x     = mid_x + (rand() & 0x3f) - 0x1f;
	SLONG best_y     = mid_y + (rand() & 0x3f) - 0x1f;
	SLONG best_z     = mid_z + (rand() & 0x3f) - 0x1f;
	SLONG best_flag  = SPARK_FLAG_FAST | SPARK_FLAG_DART_ABOUT;
	SLONG best_type  = SPARK_TYPE_CIRCULAR;
	SLONG best_dist  = radius;
	SLONG best_score = radius;

	//
	// Look for points on the ground...
	//

	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dist;
	SLONG score;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	for (i = 0; i < 4; i++)
	{
		x = mid_x + (rand() % radius) - (radius >> 1);
		z = mid_z + (rand() % radius) - (radius >> 1);

		y = calc_map_height_at(x,z);

		dx = abs(x - mid_x);
		dy = abs(y - mid_y);
		dz = abs(z - mid_z);

		dist = QDIST3(dx,dy,dz);

		//
		// Is this any good?
		//

		score = dist;

		if (score < best_score)
		{
			best_x     = x;
			best_y     = y;
			best_z     = z;
			best_flag  = SPARK_FLAG_SLOW;
			best_type  = SPARK_TYPE_GROUND;
			best_score = score;
		}
	}

#ifdef	DOG_POO		
	//
	// Look for somewhere on a colvect.
	//

	SLONG mx = mid_x >> 8;
	SLONG mz = mid_z >> 8;

	if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
		WITHIN(mz, 0, MAP_HEIGHT - 1))
	{
		SLONG y_top;
		SLONG y_bot;

		SLONG clamp;

		SLONG v_list;
		SLONG i_vect;

		CollisionVect *p_vect;

		v_list = MAP2(mx,mz).ColVectHead;
		while(v_list)
		{
			i_vect =  col_vects_links[v_list].VectIndex;
			p_vect = &col_vects[i_vect];

			//
			// Info about this colvect.
			//

			clamp = 0;

			if (p_vect->X[0] == p_vect->X[1]) {clamp |= SPARK_FLAG_CLAMP_X;}
			if (p_vect->Z[0] == p_vect->Z[1]) {clamp |= SPARK_FLAG_CLAMP_Z;}

			y_top = p_vect->Y[0] + BLOCK_SIZE * p_vect->PrimExtra;
			y_bot = p_vect->Y[0];

			if (p_vect->PrimType == STOREY_TYPE_NORMAL_FOUNDATION ||
				p_vect->PrimType == STOREY_TYPE_FENCE_FLAT)
			{
				y_bot = -INFINITY;
			}

			for (i = 0; i < 3; i++)
			{
				y = mid_y + (rand() % radius) - (radius >> 1);

				if (WITHIN(y, y_bot, y_top))
				{
					if (clamp & SPARK_FLAG_CLAMP_X) {x = p_vect->X[0];} else {x = mid_x + (rand() % radius) - (radius >> 1);}
					if (clamp & SPARK_FLAG_CLAMP_Z) {z = p_vect->Z[0];} else {z = mid_z + (rand() % radius) - (radius >> 1);}

					dx = abs(x - mid_x);
					dy = abs(y - mid_y);
					dz = abs(z - mid_z);

					dist = QDIST3(dx,dy,dz);

					//
					// Is this any good?
					//

					score = dist;

					if (score < best_score)
					{
						best_x     = x;
						best_y     = y;
						best_z     = z;
						best_flag  = SPARK_FLAG_SLOW | clamp;
						best_type  = SPARK_TYPE_POINT;
						best_score = score;
					}
				}
			}

			v_list = col_vects_links[v_list].Next;
		}		
	}
#endif

	//
	// Create the new point.
	//

	sp->Pos.x    = best_x;
	sp->Pos.y    = best_y;
	sp->Pos.z    = best_z;
	sp->dist = MIN(best_dist,255);
	sp->type = best_type;
	sp->flag = best_flag;

	SPARK_point_set_velocity(sp);
}

//
// Creates new auxillary points (2 and/or 3) for the given spark.
//

void SPARK_create_auxillary(
		SPARK_Spark *ss,
		UBYTE        point_2,
		UBYTE        point_3)
{
	SLONG px1;
	SLONG py1;
	SLONG pz1;
	SLONG px2;
	SLONG py2;
	SLONG pz2;

	SPARK_point_pos(&ss->point[0], &px1, &py1, &pz1);
	SPARK_point_pos(&ss->point[1], &px2, &py2, &pz2);

	//
	// Create a couple of other smaller tendrils.
	//

	SLONG dx = px2 - px1;
	SLONG dy = py2 - py1;
	SLONG dz = pz2 - pz1;

	SLONG dist = abs(dx) + abs(dy) + abs(dz);

	if (dist < 32)
	{
		dist = 32;
	}

	SLONG mid_x;
	SLONG mid_y;
	SLONG mid_z;

	if (point_2)
	{
		mid_x = px1 + (dx >> 1);
		mid_y = py1 + (dy >> 1);
		mid_z = pz1 + (dz >> 1);

		SPARK_new_point(
			mid_x,
			mid_y,
			mid_z,
			dist >> 1,
		   &ss->point[2]);
	}

	if (point_3)
	{
		mid_x = px2;
		mid_y = py2;
		mid_z = pz2;
		
		SPARK_new_point(
			mid_x,
			mid_y,
			mid_z,
			dist >> 1,
		   &ss->point[3]);
	}
}



void SPARK_create(
		SPARK_Pinfo *p1,
		SPARK_Pinfo *p2,
		UBYTE        max_life)
{
	SLONG i;

	SPARK_Spark *ss;

	if (max_life == 0)
	{
		max_life = 1;
	}

	//
	// Look for an unused spark structure...
	//

	for (i = 0; i < SPARK_MAX_SPARKS; i++)
	{
		SPARK_spark_last += 1;

		if (SPARK_spark_last >= SPARK_MAX_SPARKS)
		{
			SPARK_spark_last = 1;
		}

		ss = &SPARK_spark[SPARK_spark_last];

		if (!ss->used)
		{
			goto found_spare_spark;
		}
	}

	//
	// No spare spark structures.
	//

	return;

  found_spare_spark:;

	ss->used    = TRUE;
	ss->die     = rand() % max_life;
	ss->die    += 8;
	ss->glitter = NULL;	// To be created later... maybe!

	//
	// Create the first two points.
	//

	ss->num_points = 4;

	ss->point[0].type = p1->type;
	ss->point[0].flag = p1->flag;
	ss->point[0].dist = p1->dist;
	ss->point[0].Pos.x    = p1->x;
	ss->point[0].Pos.y    = p1->y;
	ss->point[0].Pos.z    = p1->z;

	ss->point[1].type = p2->type;
	ss->point[1].flag = p2->flag;
	ss->point[1].dist = p2->dist;
	ss->point[1].Pos.x    = p2->x;
	ss->point[1].Pos.y    = p2->y;
	ss->point[1].Pos.z    = p2->z;

	if (p1->type == SPARK_TYPE_LIMB)
	{
		ss->point[0].Peep.person = p1->person;
		ss->point[0].Peep.limb   = p1->limb;
	}

	if (p2->type == SPARK_TYPE_LIMB)
	{
		ss->point[1].Peep.person = p2->person;
		ss->point[1].Peep.limb   = p2->limb;
	}

	//
	// Set the velocity of the points.
	//

	SPARK_point_set_velocity(&ss->point[0]);
	SPARK_point_set_velocity(&ss->point[1]);

	//
	// Where shall we locate this spark?
	//

	SLONG px1;
	SLONG py1;
	SLONG pz1;
	SLONG px2;
	SLONG py2;
	SLONG pz2;

	SPARK_point_pos(&ss->point[0], &px1, &py1, &pz1);
	SPARK_point_pos(&ss->point[1], &px2, &py2, &pz2);

	SLONG mx = px1 >> 8;
	SLONG mz = pz1 >> 8;

	if (WITHIN(mz, 0, MAP_HEIGHT - 1))
	{
		ss->map_x        = mx;
		ss->map_z        = mz;
		ss->next         = SPARK_mapwho[mz];
		SPARK_mapwho[mz] = SPARK_spark_last;
#ifndef	PSX
		ss->glitter = GLITTER_create(0, mx, mz, 0x005566ff);
#else
		ss->glitter = 0;//GLITTER_create(0, mx, mz, 0x005566ff);
#endif
	}

	//
	// Create a couple of other smaller tendrils.
	//

	SPARK_create_auxillary(ss, TRUE, TRUE);
}

void SPARK_process()
{
	SLONG i;
	SLONG j;

	UBYTE point_2;
	UBYTE point_3;

	SPARK_Spark *ss;
	SPARK_Point *sp;

	for (i = 0; i < SPARK_MAX_SPARKS; i++)
	{	
		ss = &SPARK_spark[i];

		if (!ss->used)
		{
			continue;
		}

		if (ss->die == 0)
		{
			ss->used = FALSE;

			//
			// Kill off the glitter object.
			//
#ifndef	PSX
			GLITTER_destroy(ss->glitter);
#endif

			//
			// Take this spark out of the linked list.
			//

			UBYTE  next;
			UBYTE *prev;

			prev = &SPARK_mapwho[ss->map_z];
			next =  SPARK_mapwho[ss->map_z];

			while(1)
			{	
				if (next == i)
				{
					//
					// Found our spark.
					// 

				   *prev = ss->next;

					break;
				}
				else
				{
					ASSERT(WITHIN(next, 1, SPARK_MAX_SPARKS - 1));

					prev = &SPARK_spark[next].next;
					next =  SPARK_spark[next].next;
				}
			}
		}
		else
		{
			ss->die -= 1;

			for (j = 0; j < ss->num_points; j++)
			{
				sp = &ss->point[j];

				if (sp->type != SPARK_TYPE_LIMB)
				{
					sp->Pos.x += sp->dx >> 4;
					sp->Pos.y += sp->dy >> 4;
					sp->Pos.z += sp->dz >> 4;

					if (sp->type == SPARK_TYPE_CIRCULAR)
					{
						//
						// MAKE THE POINT sp->dist DISTANCE FROM ss->point[0]...
						//

						sp->dx = 0;
						sp->dy = 0;
						sp->dz = 0;
					}

					if (sp->flag & SPARK_FLAG_DART_ABOUT)
					{
						if ((rand() & 0xf) == (i & 0xf))
						{
							SPARK_point_set_velocity(sp);
						}
					}
				}
			}

			//
			// Create new sparks?
			//

			point_2 = ((rand() & 0xf) == (i & 0xf));
			point_3 = ((rand() & 0xf) == (i & 0xf));

			if (point_2 | point_3)
			{

//				play_quick_wave_xyz(ss->map_x<<16,256<<8,ss->map_z<<16,S_ELEC_START+(i%5),point_3,WAVE_PLAY_INTERUPT);
				MFX_play_xyz(point_3,S_ELEC_START+(i%5),ss->map_x<<16,256<<8,ss->map_z<<16,MFX_REPLACE);

				SPARK_create_auxillary(
					ss,
					point_2,
					point_3);
			}

			//
			// Create a glittery spark at the end of one of the tendrils.
			//

			{
				SLONG px;
				SLONG py;
				SLONG pz;

				j  = rand() % (ss->num_points - 1);
				j += 1;

				SPARK_point_pos(
					&ss->point[j],
					&px,
					&py,
					&pz);
#ifndef	PSX
				GLITTER_add(
					ss->glitter,
					px,
					py,
					pz);
#endif
			}
		}
	}
}

void SPARK_find_midpoint(
		SLONG  x1, SLONG  y1, SLONG z1,
		SLONG  x2, SLONG  y2, SLONG z2,
		SLONG *mx, SLONG *my, SLONG *mz)
{
	SLONG dx = x2 - x1 >> 1;
	SLONG dy = y2 - y1 >> 1;
	SLONG dz = z2 - z1 >> 1;

	SLONG dist = abs(dx) + abs(dy) + abs(dz);

	if (dist < 4) {dist = 4;}

	//
	// Create the mid point.
	//

   *mx = (x1 + dx) + (rand() % (dist >> 1)) - (dist >> 2);
   *my = (y1 + dy) + (rand() % (dist >> 2));
   *mz = (z1 + dz) + (rand() % (dist >> 1)) - (dist >> 2);
}


//
// Fills the (x,y,z)s of the given SPARK_Info structure with a spark between
// the two given points.
//

void SPARK_build_spark(
		SPARK_Info *si,
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2)
{
	si->x[0] = x1;
	si->y[0] = y1;
	si->z[0] = z1;

	si->x[4] = x2;
	si->y[4] = y2;
	si->z[4] = z2;

	//
	// Create the middle points.
	//

	SPARK_find_midpoint(
		 si->x[0],  si->y[0],  si->z[0],
		 si->x[4],  si->y[4],  si->z[4],
		&si->x[2], &si->y[2], &si->z[2]);

	SPARK_find_midpoint(
		 si->x[0],  si->y[0],  si->z[0],
		 si->x[2],  si->y[2],  si->z[2],
		&si->x[1], &si->y[1], &si->z[1]);

	SPARK_find_midpoint(
		 si->x[2],  si->y[2],  si->z[2],
		 si->x[4],  si->y[4],  si->z[4],
		&si->x[3], &si->y[3], &si->z[3]);
}



UBYTE      SPARK_get_z;
UBYTE      SPARK_get_xmin;
UBYTE      SPARK_get_xmax;
UBYTE      SPARK_get_spark;
UBYTE      SPARK_get_point;
SPARK_Info SPARK_get_info;
SLONG      SPARK_get_x1;
SLONG      SPARK_get_y1;
SLONG      SPARK_get_z1;
SLONG      SPARK_get_x2;
SLONG      SPARK_get_y2;
SLONG      SPARK_get_z2;

void SPARK_get_start(UBYTE xmin, UBYTE xmax, UBYTE z)
{
	SPARK_get_xmin       = xmin;
	SPARK_get_xmax       = xmax;
	SPARK_get_z          = z;

	if (WITHIN(SPARK_get_z, 0, MAP_HEIGHT - 1))
	{
		SPARK_get_spark = SPARK_mapwho[z];
		SPARK_get_point = 1;
	}
	else
	{
		SPARK_get_spark = NULL;
	}
}

SPARK_Info *SPARK_get_next()
{
	SPARK_Spark *ss;

  tail_recurse:;

	if (SPARK_get_spark == NULL)
	{
		return NULL;
	}

	ASSERT(WITHIN(SPARK_get_spark, 0, SPARK_MAX_SPARKS - 1));

	ss = &SPARK_spark[SPARK_get_spark];

	//
	// Only used sparks should be in the mapwho...
	//

	ASSERT(ss->used);

	if (!WITHIN(ss->map_x, SPARK_get_xmin, SPARK_get_xmax))
	{
		SPARK_get_spark = ss->next;

		goto tail_recurse;
	}

	if (SPARK_get_point >= ss->num_points)
	{
		SPARK_get_spark = ss->next;
		SPARK_get_point  = 1;

		goto tail_recurse;
	}

	//
	// The two points we build the spark between.
	//

	SPARK_Point *sp1;
	SPARK_Point *sp2;
	SPARK_Point *sp;

	SLONG x1, y1, z1;
	SLONG x2, y2, z2;

	if (SPARK_get_point == 1)
	{
		sp1 = &ss->point[0];
		sp2 = &ss->point[1];

		SPARK_point_pos(sp1, &x1, &y1, &z1);
		SPARK_point_pos(sp2, &x2, &y2, &z2);		

		//
		// Build the spark info...
		//

		SPARK_get_info.num_points = 5;
		SPARK_get_info.colour     = 0x002233ff;
		SPARK_get_info.size       = 40;
		
		SPARK_build_spark(
			&SPARK_get_info,
			 x1, y1, z1,
			 x2, y2, z2);

		SPARK_get_x1 = SPARK_get_info.x[1];
		SPARK_get_y1 = SPARK_get_info.y[1];
		SPARK_get_z1 = SPARK_get_info.z[1];

		SPARK_get_x2 = SPARK_get_info.x[3];
		SPARK_get_y2 = SPARK_get_info.y[3];
		SPARK_get_z2 = SPARK_get_info.z[3];
	}
	else
	{
		if ((rand() & 0xf) > ss->die)
		{
			//
			// Dont draw the extra sparks if it is dying out...
			//

			SPARK_get_point += 1;

			goto tail_recurse;
		}

		sp = &ss->point[SPARK_get_point];

		SPARK_point_pos(sp, &x2, &y2, &z2);

		if (SPARK_get_point == 2)
		{
			x1 = SPARK_get_x1;
			y1 = SPARK_get_y1;
			z1 = SPARK_get_z1;
		}
		else
		{
			x1 = SPARK_get_x2;
			y1 = SPARK_get_y2;
			z1 = SPARK_get_z2;
		}

		//
		// Build the spark info...
		//

		SPARK_get_info.num_points = 5;
		SPARK_get_info.colour     = 0x001825dd;
		SPARK_get_info.size       = 20;
		
		SPARK_build_spark(
			&SPARK_get_info,
			 x1, y1, z1,
			 x2, y2, z2);
	}
	
	SPARK_get_point += 1;

	return &SPARK_get_info;
}



void SPARK_in_sphere(
		SLONG mid_x,
		SLONG mid_y,
		SLONG mid_z,
		SLONG radius,
		UBYTE max_life,
		UBYTE max_create)
{
	SLONG i;
	SLONG j;

	SLONG num;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG mx;
	SLONG mz;

	SLONG dmx;
	SLONG dmz;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;

	SLONG along;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG gx;
	SLONG gy;
	SLONG gz;

	THING_INDEX t_index;

	Thing      *p_thing;
#ifndef TARGET_DC
	MapElement *me;
#endif

	//
	// All the places we could put a spark.
	//

	#define SPARK_MAX_CHOICES 16

	struct
	{
		SPARK_Pinfo p1;
		SPARK_Pinfo p2;
		
	}     choice[SPARK_MAX_CHOICES];
	SLONG choice_upto = 0;

	SPARK_Pinfo *sp1;
	SPARK_Pinfo *sp2;

	//
	// The colvects we have done already.
	//

	#define SPARK_MAX_DONE 64

	UWORD done[SPARK_MAX_DONE];
	SLONG done_upto = 0;

	//
	// The mapsquares to search.
	//

	x1 = mid_x - radius >> 8;
	z1 = mid_z - radius >> 8;
	x2 = mid_x + radius >> 8;
	z2 = mid_z + radius >> 8;

	SATURATE(x1, 0, MAP_WIDTH  - 1);
	SATURATE(z1, 0, MAP_HEIGHT - 1);
	SATURATE(x2, 0, MAP_WIDTH  - 1);
	SATURATE(z2, 0, MAP_HEIGHT - 1);

	//
	// Look for colvects or prims for the sparks to start from...
	//

	for (mx = x1; mx <= x2; mx++)
	for (mz = z1; mz <= z2; mz++)
	{
#ifndef TARGET_DC
		me = &MAP[MAP_INDEX(mx,mz)];
#endif

		/*

		//
		// Any prims on this square?
		//

		t_index = me->MapWho;

		while(t_index)
		{
			p_thing = TO_THING(t_index);

			if (p_thing->Class == CLASS_FURNITURE || p_thing->Class == CLASS_VEHICLE)
			{
				//
				// Pick a random point on the prim.
				//

				get_rotated_point_world_pos(
					-1,
					p_thing->Draw.Mesh->ObjectId,
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					p_thing->Draw.Mesh->Angle,
					p_thing->Draw.Mesh->Tilt,
					p_thing->Draw.Mesh->Roll,
					&px,
					&py,
					&pz);

				dx = abs(px - mid_x);
				dy = abs(py - mid_y);
				dz = abs(pz - mid_z);

				dist = QDIST3(dx,dy,dz);

				if (dist < radius)
				{
					//
					// Pick a point on the ground for the spark to go to.
					//

					dx = px - (p_thing->WorldPos.X >> 8) >> 1;
					dz = pz - (p_thing->WorldPos.Z >> 8) >> 1;

					gx = px + dx;
					gz = pz + dz;

					dy = calc_map_height_at(gx,gz);

					//
					// Make this one of our choices.
					//

					if (choice_upto < SPARK_MAX_CHOICES)
					{
						sp1 = &choice[choice_upto].p1;
						sp2 = &choice[choice_upto].p2;

						choice_upto += 1;
					}
					else
					{
						SLONG overwrite = rand() % SPARK_MAX_CHOICES;

						sp1 = &choice[overwrite].p1;
						sp2 = &choice[overwrite].p2;
					}

					sp1->type = SPARK_TYPE_POINT;
					sp1->flag = SPARK_FLAG_STILL;
					sp1->x    = px;
					sp1->y    = py;
					sp1->z    = pz;

					sp2->type = SPARK_TYPE_GROUND;
					sp2->flag = SPARK_FLAG_SLOW | SPARK_FLAG_DART_ABOUT;
					sp2->x    = gx;
					sp2->y    = gy;
					sp2->z    = gz;
				}
			}

			t_index = p_thing->Child;
		}

		*/

		//
		// Any colvects?
		//

		SLONG v_list;
		SLONG i_vect;

		CollisionVect *p_vect;

		v_list = MAP2(mx,mz).ColVectHead;
#ifdef	DOG_POO		
		while(v_list)
		{
			i_vect =  col_vects_links[v_list].VectIndex;
			p_vect = &col_vects[i_vect];

			if (p_vect->PrimType == STOREY_TYPE_WALL				||
				p_vect->PrimType == STOREY_TYPE_NORMAL				||
				p_vect->PrimType == STOREY_TYPE_NORMAL_FOUNDATION	||
				p_vect->PrimType == STOREY_TYPE_FENCE				||
				p_vect->PrimType == STOREY_TYPE_FENCE_FLAT)
			{
				//
				// Have we done this colvect already?
				//

				for (i = 0; i < done_upto; i++)
				{
					if (done[i] == i_vect)
					{
						goto done_this_already;
					}
				}

				if (done_upto < SPARK_MAX_DONE)
				{
					done[done_upto++] = i_vect;
				}

				//
				// If this is a long colvect have more than one spark along it.
				//

				num   = abs(p_vect->X[1] - p_vect->X[0]);
				num  += abs(p_vect->Z[1] - p_vect->Z[0]);
				num >>= 9;

				SATURATE(num, 1, 4);

				for (j = 0; j < num; j++)
				{
					//
					// Pick somewhere along the colvect.
					//

					along = rand() & 0xff;

					px = p_vect->X[0] + ((p_vect->X[1] - p_vect->X[0]) * along >> 8);
					pz = p_vect->Z[0] + ((p_vect->Z[1] - p_vect->Z[0]) * along >> 8);

					py = p_vect->Y[0] + p_vect->Y[1] + (rand() & 0x1ff) >> 1;
					
					dx = abs(px - mid_x);
					dy = abs(py - mid_y);
					dz = abs(pz - mid_z);

					dist = QDIST3(dx,dy,dz);

					if (dist < radius)
					{
						//
						// Pick somewhere on the ground to the right of the colvect.
						//

						dx = SIGN(p_vect->X[1] - p_vect->X[0]);
						dz = SIGN(p_vect->Z[1] - p_vect->Z[0]);

						gx  = px;
						gz  = pz;

						gx += (+dz) << 7;
						gz += (-dx) << 7;

						gx += (rand() & 0x3f) - 0x1f;
						gz += (rand() & 0x3f) - 0x1f;

						gy = calc_map_height_at(gx,gz);

						if (WITHIN(py - gy, 0, 0x180))
						{
							//
							// Make this one of our choices.
							//

							if (choice_upto < SPARK_MAX_CHOICES)
							{
								sp1 = &choice[choice_upto].p1;
								sp2 = &choice[choice_upto].p2;

								choice_upto += 1;
							}
							else
							{
								SLONG overwrite = rand() % SPARK_MAX_CHOICES;

								sp1 = &choice[overwrite].p1;
								sp2 = &choice[overwrite].p2;
							}

							sp1->type = SPARK_TYPE_POINT;
							sp1->flag = (p_vect->X[0] == p_vect->X[1]) ? SPARK_FLAG_CLAMP_X|SPARK_FLAG_SLOW : SPARK_FLAG_CLAMP_Z|SPARK_FLAG_SLOW;
							sp1->x    = px;
							sp1->y    = py;
							sp1->z    = pz;

							sp2->type = SPARK_TYPE_GROUND;
							sp2->flag = SPARK_FLAG_SLOW | SPARK_FLAG_DART_ABOUT;
							sp2->x    = gx;
							sp2->y    = gy;
							sp2->z    = gz;
						}
					}
				}

			  done_this_already:;

			}		

			v_list = col_vects_links[v_list].Next;
		}
#endif
	}

	//
	// Create the sparks!
	//

	SLONG choose;

	while(max_create && choice_upto)
	{
		choose = rand() % choice_upto;
		
		SPARK_create(
			&choice[choose].p1,
			&choice[choose].p2,
			 max_life);

		choice[choose] = choice[choice_upto - 1];

		max_create  -= 1;
		choice_upto -= 1;
	}
}


#ifndef	PSX
void SPARK_show_electric_fences()
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG bottom;

	SLONG along;

	DFacet *df;

	if (GAME_TURN & 0x1f)
	{
		return;
	}

	for (i = 1; i < next_dfacet; i++)
	{
		df = &dfacets[i];

		if (df->FacetFlags & FACET_FLAG_ELECTRIFIED)
		{
			//
			// Find a random place along the fence.
			//

			dx = df->x[1] - df->x[0] << 8;
			dz = df->z[1] - df->z[0] << 8;

			along = Random() & 0xff;

			{
				SPARK_Pinfo p1;
				SPARK_Pinfo p2;

				extern SLONG get_fence_bottom(SLONG x, SLONG z, SLONG col);

				p1.type   = SPARK_TYPE_POINT;
				p1.flag   = 0;
				p1.x      = (df->x[0] << 8) + (dx * along >> 8);
				p1.z      = (df->z[0] << 8) + (dz * along >> 8);
				p1.y      = (bottom = get_fence_bottom(p1.x, p1.z, i)) + (Random() & 0x7f) + 0x40;

				p2.type   = SPARK_TYPE_GROUND;
				p2.flag   = 0;
				p2.x      = p1.x + (Random() & 0x7f) - 0x40;
				p2.z      = p1.z + (Random() & 0x7f) - 0x40;
				p2.y      = PAP_calc_map_height_at(p2.x, p2.z);

				if (dx == 0)
				{
					p1.flag |= SPARK_FLAG_CLAMP_X;
				}
				else
				{
					p1.flag |= SPARK_FLAG_CLAMP_Z;
				}

				if (abs(bottom - p2.y) < 0x40)
				{
					SPARK_create(&p1, &p2, 50);
				}
			}
		}
	}
}
#endif
