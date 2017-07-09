//
// Crappy barrels with kludged physics
//

#include "game.h"
#include "barrel.h"
#include "fmatrix.h"
#include "mav.h"
#include "pap.h"
#include "statedef.h"
#include "animate.h"
#include "pcom.h"
#include "psystem.h"
#include "poly.h"
#include "eway.h"
#include "sound.h"
#include "pow.h"
#include "dirt.h"
#ifndef PSX
#include "panel.h"
#else
#include "fallen/psxeng/headers/panel.h"
#endif

BARREL_Sphere *BARREL_sphere; //[BARREL_MAX_SPHERES];
SLONG          BARREL_sphere_last;	  // MARK!!! WTF, you usuall call thing BLAH_blah_upto


//
// The states a barrel can be in.
//

#define BARREL_FLAG_STACKED  (1 << 0)
#define BARREL_FLAG_STILL    (1 << 1)
#define BARREL_FLAG_GROUNDED (1 << 2)
#define BARREL_FLAG_HELD     (1 << 3)	// A person is holding this barrel- don't process it.
#define BARREL_FLAG_HIT		 (1 << 4)	// This cone has already been hit- so don't be penalised for hitting it again.
#define BARREL_FLAG_RUBBISH  (1 << 5)	// This bin contains rubbish.
#define BARREL_FLAG_CANS     (1 << 6)	// This bin contains cans.

#ifndef PSX
extern BOOL allow_debug_keys;
#endif

Barrel *BARREL_barrel;//[BARREL_MAX_BARRELS];
SLONG   BARREL_barrel_upto;


//
// The radius and height of a barrel. A barrel has its pivot in the middle of itself,
// not at its base!
//

#define BARREL_DIAMETER      80
#define BARREL_RADIUS        40
#define BARREL_HEIGHT        93
#define BARREL_STACK_RADIUS  45
#define BARREL_SPHERE_RADIUS 85
#define BARREL_SPHERE_DIST   50
#define BARREL_GRAVITY       0x80


SLONG BARREL_fx_rate;

inline void BARREL_hit_noise(Thing *p_barrel) {

	BARREL_fx_rate++;

	//TRACE("barrel: %d\n",BARREL_fx_rate);

	if (BARREL_fx_rate>4) return;

	if (p_barrel->Genus.Barrel->type == BARREL_TYPE_CONE)
		MFX_play_thing(THING_NUMBER(p_barrel),S_TRAFFIC_CONE,0,p_barrel);
	else
		MFX_play_thing(THING_NUMBER(p_barrel),SOUND_Range(S_BARREL_START,S_BARREL_END),0,p_barrel);
}
#ifndef PSX

void BARREL_init()
{
	SLONG i;

	memset(BARREL_sphere, 0, sizeof(BARREL_Sphere)*BARREL_MAX_SPHERES);
	memset(BARREL_barrel, 0, sizeof(Barrel)*BARREL_MAX_BARRELS);

	BARREL_barrel_upto = 0;
	BARREL_sphere_last = 2;

	for (i = 0; i < BARREL_MAX_SPHERES; i++)
	{
		BARREL_sphere[i].x = -INFINITY;	// The marker for being unused.
	}
	BARREL_fx_rate = 0;
}
#endif

//
// Returns an index to two free BARREL_Spheres.
//

SLONG BARREL_spheres_get(void)
{
	SLONG i;

	for (i = 0; i < BARREL_MAX_SPHERES / 2; i++)
	{
		ASSERT(WITHIN(BARREL_sphere_last, 2, BARREL_MAX_SPHERES - 2));

		if (BARREL_sphere[BARREL_sphere_last].x == -INFINITY)
		{
			return BARREL_sphere_last;
		}

		BARREL_sphere_last += 2;

		if (BARREL_sphere_last >= BARREL_MAX_SPHERES - 2)
		{
			BARREL_sphere_last = 2;
		}
	}

	return NULL;
}

//
// Marks the two barrel spheres as unused.
//

void BARREL_spheres_give(SLONG bs)
{
	ASSERT(WITHIN(bs + 0, 2, BARREL_MAX_SPHERES - 2));
	ASSERT(WITHIN(bs + 1, 3, BARREL_MAX_SPHERES - 1));

	BARREL_sphere[bs + 0].x = -INFINITY;
	BARREL_sphere[bs + 1].x = -INFINITY;
}

//
// Returns the position where the given sphere of a stacked/still barrel would be.
//

void BARREL_stacked_sphere(Thing *p_barrel, SLONG which_sphere, SLONG *sx, SLONG *sy, SLONG *sz, SLONG *sradius)
{
	ASSERT(p_barrel->Class == CLASS_BARREL);

	SLONG ans_x = p_barrel->WorldPos.X;
	SLONG ans_y = p_barrel->WorldPos.Y;
	SLONG ans_z = p_barrel->WorldPos.Z;

	SLONG vector[3];

	FMATRIX_vector(
		vector,
		p_barrel->Draw.Mesh->Angle,
		p_barrel->Draw.Mesh->Tilt);

	vector[0] = MUL64(vector[0], (BARREL_SPHERE_DIST << 7));
	vector[1] = MUL64(vector[1], (BARREL_SPHERE_DIST << 7));
	vector[2] = MUL64(vector[2], (BARREL_SPHERE_DIST << 7));

	if (which_sphere)
	{
		ans_x  += vector[0];
		ans_y  += vector[1];
		ans_z  += vector[2];
	   *sradius = BARREL_SPHERE_RADIUS << 8;

		if (p_barrel->Genus.Barrel->type == BARREL_TYPE_CONE)
		{
			*sradius >>= 1;
		}
	}
	else
	{
		ans_x  -= vector[0];
		ans_y  -= vector[1];
		ans_z  -= vector[2];
	   *sradius = BARREL_SPHERE_RADIUS << 8;

		if (p_barrel->Genus.Barrel->type == BARREL_TYPE_CONE)
		{
			*sradius -= *sradius >> 2;
		}
	}

   *sx = ans_x;
   *sy = ans_y;
   *sz = ans_z;
}


//
// Converts a stationary barrel to a moving one.
//

void BARREL_convert_stationary_to_moving(Thing *p_barrel)
{
	SLONG i;

	SLONG sx;
	SLONG sy;
	SLONG sz;
	SLONG sradius;

	ASSERT(p_barrel->Class == CLASS_BARREL);

	Barrel        *bb = p_barrel->Genus.Barrel;
	BARREL_Sphere *bs;

	ASSERT(bb->flag & (BARREL_FLAG_STACKED|BARREL_FLAG_STILL));

	if (p_barrel->Genus.Barrel->type == BARREL_TYPE_CONE)
	{
/*
		if (!(p_barrel->Genus.Barrel->flag & BARREL_FLAG_HIT))
		{
			p_barrel->Genus.Barrel->flag |= BARREL_FLAG_HIT;

			extern UBYTE EWAY_count_up_visible;
			extern SLONG EWAY_count_up;

			if (EWAY_count_up_visible)
			{
				EWAY_count_up += 500;

				void add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text);

				add_damage_text(
					p_barrel->WorldPos.X >> 8,
					p_barrel->WorldPos.Y >> 8,
					p_barrel->WorldPos.Z >> 8,
					"Time penalty");

				PANEL_new_info_message("Time Penalty");
			}

			if (GAME_FLAGS & GF_CONE_PENALTIES)
			{
				EWAY_deduct_time_penalty(50);
			}
		}
*/

		MFX_play_thing(THING_NUMBER(p_barrel),S_TRAFFIC_CONE,0,p_barrel);
	} else MFX_play_thing(THING_NUMBER(p_barrel),S_CAR_BUMP,0,p_barrel);

	if (p_barrel->Genus.Barrel->flag & BARREL_FLAG_RUBBISH)
	{
		//
		// Make some newspapers fly out from the top of the barrel.
		//

		DIRT_create_papers(
			(p_barrel->WorldPos.X >> 8),
			(p_barrel->WorldPos.Y >> 8) + 0xa0,
			(p_barrel->WorldPos.Z >> 8));
	}

	//
	// Find a spare set of spheres.
	//

	bb->bs    =  BARREL_spheres_get();
	bb->flag &= ~(BARREL_FLAG_STACKED|BARREL_FLAG_STILL);
	bb->on    =  NULL;

	for (i = 0; i < 2; i++)
	{
		ASSERT(WITHIN(bb->bs + i, 2, BARREL_MAX_SPHERES - 1));

		bs = &BARREL_sphere[bb->bs + i];

		BARREL_stacked_sphere(p_barrel, i, &sx, &sy, &sz, &sradius);

		bs->x = sx;
		bs->y = sy;
		bs->z = sz;

		bs->dx = 0;
		bs->dy = (Random() & 0xff);
		bs->dz = 0;

		bs->still  = 0;
		bs->radius = sradius;
	}
}

void BARREL_convert_moving_to_stationary(Thing *p_barrel)
{
	ASSERT(p_barrel->Class == CLASS_BARREL);

	Barrel        *bb = p_barrel->Genus.Barrel;
	BARREL_Sphere *bs;

	BARREL_spheres_give(bb->bs);

	bb->bs    = 0;
	bb->on    = 0;
	bb->flag |= BARREL_FLAG_STILL;

	if (BARREL_fx_rate>4)
		BARREL_fx_rate-=3;
	else
		BARREL_fx_rate=0;
}


//
// decrease stack usage
//

#define BARREL_MAX_FIND 16
THING_INDEX found_barrel[BARREL_MAX_FIND];

void BARREL_hit_with_sphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG radius)
{
	SLONG i;
	SLONG j;

	SLONG sx;
	SLONG sy;
	SLONG sz;
	SLONG sradius;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG ddist;

	Thing         *p_found;
	Barrel        *bb;
	BARREL_Sphere *bs;

	SLONG       num;

	num = THING_find_sphere(
			x, y, z,
			radius,
			found_barrel,
			BARREL_MAX_FIND,
			1 << CLASS_BARREL);

	//
	// Convert (x,y,z,radius) to the barrel coordinate system.
	//

	x      <<= 8;
	y      <<= 8;
	z      <<= 8;
	radius <<= 8;

	for (i = 0; i < num; i++)
	{
		p_found = TO_THING(found_barrel[i]);
		bb      = p_found->Genus.Barrel;

	  try_this_again:;

		if (bb->flag & (BARREL_FLAG_STACKED | BARREL_FLAG_STILL))
		{
			//
			// If any of the potential spheres of the stacked barrel are effected by
			// this hit, then we must convert the barrel to a moving one and then apply
			// the hit to each.
			//

			for (j = 0; j < 2; j++)
			{
				BARREL_stacked_sphere(
					p_found,
					j,
				   &sx,
				   &sy,
				   &sz,
				   &sradius);

				dx = abs(sx - x);
				dy = abs(sy - y);
				dz = abs(sz - z);

				dist = QDIST3(dx,dy,dz);

				if (dist < radius + sradius)
				{
					//
					// Convert this barrel to a moving one.
					//

					BARREL_convert_stationary_to_moving(p_found);

					goto try_this_again;
				}
			}
		}
		else
		{
			//
			// Make sure none of the spheres of the barrel are within our collision sphere.
			//

			for (j = 0; j < 2; j++)
			{
				ASSERT(WITHIN(bb->bs + j, 2, BARREL_MAX_SPHERES - 1));

				bs = &BARREL_sphere[bb->bs + j];

				dx = bs->x - x;
				dy = bs->y - y;
				dz = bs->z - z;

				dist  = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;
				ddist = radius + bs->radius - dist;

				if (ddist > 0)
				{
					//
					// Push this barrel sphere away from the collision sphere.
					//

					dx = dx * ddist / dist;
					dy = dy * ddist / dist;
					dz = dz * ddist / dist;

					bs->x += dx;
					bs->y += dy;
					bs->z += dz;

					bs->dx += dx / 8;
					bs->dy += dy / 8;
					bs->dz += dz / 8;

					BARREL_hit_noise(p_found);
				}
			}
		}
	}
}


void BARREL_hit_with_prim(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw)
{
	SLONG i;
	SLONG j;

	Thing         *p_found;
	Barrel        *bb_found;
	BARREL_Sphere *bs;

	SLONG       num;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG sx;
	SLONG sy;
	SLONG sz;
	SLONG sradius;

	SLONG tx;
	SLONG tz;

	SLONG rx;
	SLONG rz;

	SLONG matrix[4];
	SLONG useangle;

	SLONG sin_yaw;
	SLONG cos_yaw;

	SLONG minx;
	SLONG miny;
	SLONG minz;

	SLONG maxx;
	SLONG maxy;
	SLONG maxz;

	SLONG dminx;
	SLONG dminz;
	SLONG dmaxx;
	SLONG dmaxy;
	SLONG dmaxz;

	SLONG best;
	SLONG best_x;
	SLONG best_y;
	SLONG best_z;

	PrimInfo *pi = get_prim_info(prim);

	//
	// Find all barrels withing the bounding sphere of the prim.
	//

	num = THING_find_sphere(
			x, y, z,
			pi->radius,
			found_barrel,
			BARREL_MAX_FIND,
			1 << CLASS_BARREL);

	//
	// The y-range we consider in worldspace.
	//

	miny = y + pi->miny - BARREL_SPHERE_RADIUS;
	maxy = y + pi->maxy + BARREL_SPHERE_RADIUS;

	//
	// The bounding box in object space.
	//

	minx = pi->minx - BARREL_SPHERE_RADIUS;
	minz = pi->minz - BARREL_SPHERE_RADIUS;

	maxx = pi->maxx + BARREL_SPHERE_RADIUS;
	maxz = pi->maxz + BARREL_SPHERE_RADIUS;

	//
	// Find the bounding box of the prim.
	//

	useangle  = -yaw;
	useangle &=  2047;

	sin_yaw = SIN(useangle);
	cos_yaw = COS(useangle);

	matrix[0] =  cos_yaw;
	matrix[1] =  sin_yaw;
	matrix[2] = -sin_yaw;
	matrix[3] =  cos_yaw;

	for (i = 0; i < num; i++)
	{
		ASSERT(TO_THING(found_barrel[i])->Class == CLASS_BARREL);

		p_found  = TO_THING(found_barrel[i]);
		bb_found = p_found->Genus.Barrel;

	  try_this_again:;

		if (bb_found->flag & (BARREL_FLAG_STACKED|BARREL_FLAG_STILL))
		{
			//
			// Check each of the potential spheres.
			//

			for (j = 0; j < 2; j++)
			{
				BARREL_stacked_sphere(
					p_found,
					j,
				   &sx,
				   &sy,
				   &sz,
				   &sradius);

				sx >>= 8;
				sy >>= 8;
				sz >>= 8;

				if (WITHIN(sy, miny, maxy))
				{
					tx = sx - x;
					tz = sz - z;

					rx = MUL64(tx, matrix[0]) + MUL64(tz, matrix[1]);
					rz = MUL64(tx, matrix[2]) + MUL64(tz, matrix[3]);

					if (WITHIN(rx, minx, maxx) &&
						WITHIN(rz, minz, maxz))
					{
						//
						// The barrel would collide.  Convert this barrel to a moving one and
						// do the collision stuff again.
						//

						BARREL_convert_stationary_to_moving(p_found);

						goto try_this_again;
					}
				}
			}
		}
		else
		{
			//
			// Check each sphere.
			//

			for (j = 0; j < 2; j++)
			{
				ASSERT(WITHIN(bb_found->bs + j, 2, BARREL_MAX_SPHERES - 1));

				bs = &BARREL_sphere[bb_found->bs + j];

				sx = bs->x >> 8;
				sy = bs->y >> 8;
				sz = bs->z >> 8;

				if (WITHIN(sy, miny, maxy))
				{
					tx = sx - x;
					tz = sz - z;

					rx = MUL64(tx, matrix[0]) + MUL64(tz, matrix[1]);
					rz = MUL64(tx, matrix[2]) + MUL64(tz, matrix[3]);

					if (WITHIN(rx, minx, maxx) &&
						WITHIN(rz, minz, maxz))
					{
						//
						// The barrel sphere has collided.  Find the nearest point on the
						// edge of the box.
						//

						dminx = rx - minx;
						dminz = rz - minz;

						dmaxx = maxx - rx;
						dmaxy = maxy - sy;
						dmaxz = maxz - rz;

						best   = dminx;
						best_x = rx;
						best_y = sy;
						best_z = rz;

						if (dminz < best)
						{
							best   = dminz;
							best_x = rx;
							best_y = sy;
							best_z = minz;
						}

						if (dmaxx < best)
						{
							best   = dmaxx;
							best_x = maxx;
							best_y = sy;
							best_z = rz;
						}

						if (dmaxy < best)
						{
							best   = dmaxy;
							best_x = rx;
							best_y = maxy;
							best_z = rz;
						}

						if (dmaxz < best)
						{
							best   = dmaxz;
							best_x = rx;
							best_y = sy;
							best_z = maxz;
						}

						//
						// Unrotate (best_x,best_y,best_z) out of barrel space.
						//

						sx = MUL64(best_x, matrix[0]) + MUL64(best_z, matrix[2]);
						sz = MUL64(best_x, matrix[1]) + MUL64(best_z, matrix[3]);

						sx += x;
						sy  = best_y;
						sz += z;

						//
						// Move the sphere to the best position on the edge of the box.
						//

						sx <<= 8;
						sy <<= 8;
						sz <<= 8;

						dx = sx - bs->x;
						dy = sy - bs->y;
						dz = sz - bs->z;

						bs->x = sx;
						bs->y = sy;
						bs->z = sz;

						bs->dx += dx / 8;
						bs->dy += dy / 8;
						bs->dz += dz / 8;

						BARREL_hit_noise(p_found);
					}
				}
			}
		}
	}
}


//
// Processes a barrel sphere.
//

void BARREL_process_sphere(Thing *p_barrel, Barrel *bb, BARREL_Sphere *bs)
{
	SLONG i;
	SLONG j;

	SLONG sx;
	SLONG sy;
	SLONG sz;
	SLONG sradius;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG ddist;

	Thing  *p_found;
	Barrel *bb_found;

	BARREL_Sphere *bso;

	SLONG num;
	SLONG ground;

	//
	// Gravity and damping.
	//

	bs->dx -= bs->dx / 32;
	bs->dy -= bs->dy / 32;
	bs->dz -= bs->dz / 32;

	bs->dy -= BARREL_GRAVITY;

	bs->x += bs->dx;
	bs->y += bs->dy;
	bs->z += bs->dz;

	//
	// Collision with other barrels.
	//

	num = THING_find_sphere(
			bs->x >> 8,
			bs->y >> 8,
			bs->z >> 8,
			BARREL_HEIGHT,
			found_barrel,
			BARREL_MAX_FIND,
			1 << CLASS_BARREL);

	for (i = 0; i < num ; i++)
	{
		ASSERT(TO_THING(found_barrel[i])->Class == CLASS_BARREL);

		p_found  = TO_THING(found_barrel[i]);

		if (p_found == p_barrel)
		{
			//
			// Don't collide with yourself.
			//

			continue;
		}

		bb_found = p_found->Genus.Barrel;

	  try_this_again:;

		if (bb_found->flag & (BARREL_FLAG_STACKED|BARREL_FLAG_STILL))
		{
			//
			// Would we collide with this barrel if it was moving?
			//

			for (j = 0; j < 2; j++)
			{
				BARREL_stacked_sphere(
					p_found,
					j,
				   &sx,
				   &sy,
				   &sz,
				   &sradius);

				dx = sx - bs->x;
				dy = sy - bs->y;
				dz = sz - bs->z;

				dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

				if (dist < sradius)
				{
					//
					// Yes. Convert this barrel to a moving one and try again.
					//

					BARREL_convert_stationary_to_moving(p_found);

					goto try_this_again;
				}
			}
		}
		else
		{
			//
			// Collide with the spheres of this barrel.
			//

			for (j = 0; j < 2; j++)
			{
				ASSERT(WITHIN(bb_found->bs + j, 2, BARREL_MAX_SPHERES - 1));

				bso = &BARREL_sphere[bb_found->bs + j];

				dx = bso->x - bs->x;
				dy = bso->y - bs->y;
				dz = bso->z - bs->z;

				dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

				if (dist < bs->radius)
				{
					//
					// These two spheres have collided. Now push them apart.
					//

					ddist = bs->radius - dist;

					ddist /= 4;

					dx = dx * ddist / dist;
					dy = dy * ddist / dist;
					dz = dz * ddist / dist;

					bs->x -= dx;
					bs->y -= dy;
					bs->z -= dz;

					bso->x += dx;
					bso->y += dy;
					bso->z += dz;

					BARREL_hit_noise(p_barrel);
				}
			}
		}
	}

	//
	// Collision with the ground.
	//

	ground = PAP_calc_map_height_at(
				bs->x >> 8,
				bs->z >> 8) << 8;

	ground += bs->radius >> 2;

	if (bs->y < ground)
	{
		if (bs->dy<-1000) BARREL_hit_noise(p_barrel);
		if ((bs->y < ground - 0x8000) || (bs->dy > 0))
		{
			//
			// Hit a wall- not gone underground.
			//

			bs->x -= bs->dx;
			bs->z -= bs->dz;

			bs->dx = -bs->dx;
			bs->dz = -bs->dz;
		}
		else
		{
			bs->y   = ground;
			bs->dx  = 0;
			bs->dy  = abs(bs->dy) >> 1;
			bs->dz  = 0;

			bb->flag |= BARREL_FLAG_GROUNDED;
		}
	}

	//
	// Collision with the sides of fences and buildings.
	//

	if (WITHIN(bs->x >> 16, 0, PAP_SIZE_HI - 1) &&
		WITHIN(bs->z >> 16, 0, PAP_SIZE_HI - 1))
	{
		MAV_Opt mo = MAV_opt[MAV_NAV(bs->x >> 16,bs->z >> 16)];

		if (!(mo.opt[MAV_DIR_XS] & (MAV_CAPS_GOTO|MAV_CAPS_FALL_OFF)))
		{
			//
			// The sphere shouldn't be near the (x-small) bit of this mapsquare.
			//

			if ((bs->x & 0xffff) < bs->radius)
			{
				bs->x &= ~0xffff;
				bs->x |=  bs->radius;
				bs->dx =  abs(bs->dx) >> 1;
				bs->dy =  0;
				bs->dz =  0;
			}
		}

		if (!(mo.opt[MAV_DIR_XL] & (MAV_CAPS_GOTO|MAV_CAPS_FALL_OFF)))
		{
			//
			// The sphere shouldn't be near the (x-large) bit of this mapsquare.
			//

			if ((bs->x & 0xffff) > 0x10000 - bs->radius)
			{
				bs->x &= ~0xffff;
				bs->x |=  0x10000 - bs->radius;
				bs->dx =  -(abs(bs->dx) >> 1);
				bs->dy =  0;
				bs->dz =  0;
			}
		}

		if (!(mo.opt[MAV_DIR_ZS] & (MAV_CAPS_GOTO|MAV_CAPS_FALL_OFF)))
		{
			//
			// The sphere shouldn't be near the (z-small) bit of this mapsquare.
			//

			if ((bs->z & 0xffff) < bs->radius)
			{
				bs->z &= ~0xffff;
				bs->z |=  bs->radius;
				bs->dx =  0;
				bs->dy =  0;
				bs->dz =  abs(bs->dz) >> 1;
			}
		}

		if (!(mo.opt[MAV_DIR_ZL] & (MAV_CAPS_GOTO|MAV_CAPS_FALL_OFF)))
		{
			//
			// The sphere shouldn't be near the (z-large) bit of this mapsquare.
			//

			if ((bs->z & 0xffff) > 0x10000 - bs->radius)
			{
				bs->z &= ~0xffff;
				bs->z |=  0x10000 - bs->radius;
				bs->dx =  0;
				bs->dy =  0;
				bs->dz =  -(abs(bs->dz) >> 1);
			}
		}
	}

	if (abs(bs->dx) + abs(bs->dy) + abs(bs->dz) < 0x200)
	{
		bs->still += 1;
	}
	else
	{
		bs->still = 0;
	}
}


//
// Pushes the two spheres together/away so they are the right distance apart.
//

void BARREL_push_apart(
		BARREL_Sphere *bs1,
		BARREL_Sphere *bs2,
		SLONG          accelerate_the_spheres_apart_if_they_are_too_close_together)
{
	SLONG dx = bs2->x - bs1->x;
	SLONG dy = bs2->y - bs1->y;
	SLONG dz = bs2->z - bs1->z;

	SLONG dist  = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;
	SLONG ddist = (BARREL_SPHERE_DIST << 8) - dist;

	ddist /= 4;

	dx = dx * ddist / dist;
	dy = dy * ddist / dist;
	dz = dz * ddist / dist;

	bs1->dx -= dx;
	bs1->dy -= dy;
	bs1->dz -= dz;

	bs2->dx += dx;
	bs2->dy += dy;
	bs2->dz += dz;

	if (ddist > ((BARREL_SPHERE_DIST << 7) / 8))
	{
		//
		// The spheres are dangerously close together.
		//

		bs1->x -= dx * 2;
		bs1->y -= dy * 2;
		bs1->z -= dz * 2;

		bs2->x += dx * 2;
		bs2->y += dy * 2;
		bs2->z += dz * 2;
	}

	if (accelerate_the_spheres_apart_if_they_are_too_close_together)
	{
		if (ddist > ((BARREL_SPHERE_DIST << 7) / 8))
		{
			//
			// The spheres are dangerously close together.
			//

			bs1->x -= dx * 2;
			bs1->y -= dy * 2;
			bs1->z -= dz * 2;

			bs2->x += dx * 2;
			bs2->y += dy * 2;
			bs2->z += dz * 2;
		}
	}
}


void BARREL_process_normal(Thing *p_barrel)
{
	ASSERT(p_barrel->Class == CLASS_BARREL);

	GameCoord newpos;

	Barrel *bb = p_barrel->Genus.Barrel;

	if (bb->flag & BARREL_FLAG_STACKED)
	{
		if (bb->on)
		{
			//
			// Do nothing unless the barrel we are stacked on has started.
			//

			Thing *p_stackedon = TO_THING(bb->on);

			ASSERT(p_stackedon->Class == CLASS_BARREL);

			if (p_stackedon->Genus.Barrel->flag & BARREL_FLAG_STACKED)
			{
				//
				// We are okay.
				//
			}
			else
			{
				//
				// Make the barrel start moving.
				//

				BARREL_convert_stationary_to_moving(p_barrel);
			}
		}

		return;
	}

	if (bb->flag & BARREL_FLAG_STILL)
	{
		//
		// No processing required.
		//

		return;
	}

	//
	// Process each sphere of the barrel.
	//

	ASSERT(WITHIN(bb->bs + 0, 2, BARREL_MAX_SPHERES - 1));
	ASSERT(WITHIN(bb->bs + 1, 2, BARREL_MAX_SPHERES - 1));

	BARREL_Sphere *bs1 = &BARREL_sphere[bb->bs + 0];
	BARREL_Sphere *bs2 = &BARREL_sphere[bb->bs + 1];

	bb->flag &= ~BARREL_FLAG_GROUNDED;

	if (bb->flag & BARREL_FLAG_HELD)
	{
		//
		// Dont process the barrel.
		//
	}
	else
	{
		BARREL_process_sphere(p_barrel, bb, bs1);
		BARREL_process_sphere(p_barrel, bb, bs2);

		if (bb->flag & BARREL_FLAG_CANS)
		{
			//
			// Should the cans spill out of the barrel?
			//

			if (bb->flag & BARREL_FLAG_GROUNDED)
			{
				if (bs1->still &&
					bs2->still)
				{
					//
					// Make sure the barrel is not standing up.
					//

					SLONG dtilt1 = abs(p_barrel->Draw.Mesh->Tilt -  512);
					SLONG dtilt2 = abs(p_barrel->Draw.Mesh->Tilt - 1536);

					if (MIN(dtilt1, dtilt2) < 224)
					{
						//
						// Standing up...
						//
					}
					else
					{
						bb->flag &= ~BARREL_FLAG_CANS;

						{
							SLONG angle;
							SLONG cx;
							SLONG cz;

							angle = p_barrel->Draw.Mesh->Angle;

							cx = p_barrel->WorldPos.X >> 8;
							cz = p_barrel->WorldPos.Z >> 8;

							cx += SIN(angle) >> 11;
							cz += COS(angle) >> 11;

							DIRT_create_cans(cx, cz, angle);
						}
					}
				}
			}
		}
	}

	//
	// Make the spheres be the correct distance apart.
	//

	BARREL_push_apart(
		bs1,
		bs2,
		TRUE);

	//
	// The barrel is positioned at the average of the two spheres.
	//

	newpos.X = bs1->x + bs2->x >> 1;
	newpos.Y = bs1->y + bs2->y >> 1;
	newpos.Z = bs1->z + bs2->z >> 1;

	move_thing_on_map(p_barrel, &newpos);

	//
	// Work out the yaw and pitch of the barrel.
	//

	SLONG dx = bs2->x - bs1->x;
	SLONG dy = bs2->y - bs1->y;
	SLONG dz = bs2->z - bs1->z;

	SLONG dxdz = QDIST2(abs(dx),abs(dz));

	if (dxdz < 0x100)
	{
		//
		// Dont work out the yaw - we dont have enough info.
		//
	}
	else
	{
		p_barrel->Draw.Mesh->Angle = calc_angle(dx,dz);
	}

	p_barrel->Draw.Mesh->Tilt = calc_angle(dy, dxdz);

	if (bb->type == BARREL_TYPE_BIN || p_barrel->Draw.Mesh->ObjectId == 145)
	{
		p_barrel->Draw.Mesh->Tilt += 1024;
		p_barrel->Draw.Mesh->Tilt &= 2047;
	}

	if (bb->flag & BARREL_FLAG_GROUNDED)
	{
		//
		// Make the barrels be able to stand up.
		//

		SLONG dtilt1 = abs(p_barrel->Draw.Mesh->Tilt -  512);
		SLONG dtilt2 = abs(p_barrel->Draw.Mesh->Tilt - 1536);

		if (MIN(dtilt1, dtilt2) < 224)
		{
			if ((bb->type == BARREL_TYPE_CONE || bb->type == BARREL_TYPE_BIN) && (bs1->y > bs2->y))
			{
				//
				// Cones and bins only stand up one way...
				//
			}
			else
			{
				//
				// Make the two spheres accelerate towards one another in (dx,dz) only.
				//

				dx = (bs2->x - bs1->x) / 4;
				dz = (bs2->z - bs1->z) / 4;

				bs1->x += dx;
				bs1->z += dz;

				bs2->x -= dx;
				bs2->z -= dz;

				bs1->dx /=2;
				bs1->dz /=2;

				bs2->dx /=2;
				bs2->dz /=2;
			}
		}
	}

	//
	// Can we stop processing this barrel?
	//

	if (bs1->still > 64 &&
		bs2->still > 64)
	{
		//
		// Both spheres have been still for a while- we can stop
		// processing the barrel.
		//

		BARREL_convert_moving_to_stationary(p_barrel);
	}
#ifndef PSX
#ifndef TARGET_DC
	if (ControlFlag&&allow_debug_keys)
	{
		AENG_world_line(
			bs1->x >> 8,
			bs1->y >> 8,
			bs1->z >> 8,
			8,
			0x0022ff22,
			bs2->x >> 8,
			bs2->y >> 8,
			bs2->z >> 8,
			8,
			0x00ff4444,
			TRUE);
	}
#endif
#endif
}

UWORD BARREL_alloc(
		SLONG type,
		SLONG prim,
		SLONG x,
		SLONG z,
		SLONG waypoint)
{
	SLONG i;
	SLONG y;
	SLONG on;
	SLONG ony;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	DrawMesh *dm;
	Thing    *p_thing;
	Thing    *p_found;
	Barrel   *bb;

	SLONG     num;

	//
	// We need to get a thing, a barrel and a drawmesh.
	//

	if (!WITHIN(BARREL_barrel_upto, 0, BARREL_MAX_BARRELS - 1))
	{
		//
		// No more barrels left!
		//

		return NULL;
	}

	bb = &BARREL_barrel[BARREL_barrel_upto++];

	//
	// Now get the thing.
	//

	p_thing = alloc_thing(CLASS_BARREL);
	if(!p_thing)
	{
		Thing	*p_del;
		SLONG	c0;
		SLONG	best_dist=0x7fffffff;
		for(c0=1;c0<MAX_THINGS;c0++)
		{
			p_del=TO_THING(c0);
			if(p_del->Class==CLASS_BARREL)
			{
				SLONG	dx,dz,dist;

				dx=x-(p_del->WorldPos.X>>8);
				dz=z-(p_del->WorldPos.Z>>8);

				dist=QDIST2(abs(dx),abs(dz));

				if(dist<best_dist)
				{
					p_thing=p_del;
					best_dist=dist;
				}
			}
		}
	}

	if (p_thing)
	{
		//
		// And finally get the drawmesh
		//

		dm = alloc_draw_mesh();

		if (dm)
		{
			p_thing->Class        = CLASS_BARREL;
			p_thing->Flags        = 0;
			p_thing->State        = STATE_NORMAL;
			p_thing->StateFn      = BARREL_process_normal;
			p_thing->Genus.Barrel = bb;
			p_thing->DrawType     = DT_MESH;
			p_thing->Draw.Mesh    = dm;
			p_thing->Velocity     = waypoint;

			dm->Angle    = Random() & 2047;
			dm->Tilt     = 512;
			dm->Roll     = 0;		// Wierd huh?!
			dm->ObjectId = prim;
			dm->Hm       = 0;
			dm->Cache    = 0;

			if (type == BARREL_TYPE_BIN || prim == 145)
			{
				dm->Tilt = 1536;
			}

			bb->type   = type;
			bb->flag   = BARREL_FLAG_STACKED;
			bb->on     = NULL;	// NULL => stacked on the ground
			bb->bs     = NULL;

			if (bb->type == BARREL_TYPE_BIN)
			{
				bb->flag |= BARREL_FLAG_RUBBISH | BARREL_FLAG_CANS;	// Bins contain rubbish.
			}

			//
			// Look for a barrel we should be stacked up on top of.
			//

			num = THING_find_box(
					x - BARREL_STACK_RADIUS,
					z - BARREL_STACK_RADIUS,
					x + BARREL_STACK_RADIUS,
					z + BARREL_STACK_RADIUS,
					found_barrel,
					BARREL_MAX_FIND,
					1 << CLASS_BARREL);

			y  = PAP_calc_map_height_at(x,z) + (BARREL_HEIGHT / 2);
			on = NULL;

			if (type == BARREL_TYPE_BURNING) {
				Pyro* pyro;

				pyro=PYRO_create(p_thing->WorldPos,PYRO_IMMOLATE)->Genus.Pyro;
				if(pyro)
				{
					pyro->victim=p_thing;
					pyro->Flags=PYRO_FLAGS_FLICKER; // immolate faces
				}
			}

			if (type == BARREL_TYPE_CONE)
			{
				//
				// Cones don't stack- and they aren't as high off the ground...
				//

				y -= 15;
			}
			else
			{
				for (i = 0; i < num; i++)
				{
					p_found = TO_THING(found_barrel[i]);

					dx = abs((p_found->WorldPos.X >> 8) - x);
					dz = abs((p_found->WorldPos.Z >> 8) - z);

					dist = QDIST2(dx,dz);

					if (dist < BARREL_STACK_RADIUS)
					{
						ony = (p_found->WorldPos.Y >> 8) + BARREL_HEIGHT;

						if (ony > y)
						{
							y  = ony;
							on = found_barrel[i];
						}
					}
				}
			}

			bb->on              = on;
			p_thing->WorldPos.X = x << 8;
			p_thing->WorldPos.Y = y << 8;
			p_thing->WorldPos.Z = z << 8;

			p_thing->WorldPos.X += (Random() & 0x3ff) - 0x1ff;
			p_thing->WorldPos.Y += (Random() & 0x3ff) - 0x1ff;
			p_thing->WorldPos.Z += (Random() & 0x3ff) - 0x1ff;

			add_thing_to_map(p_thing);

			return THING_NUMBER(p_thing);
		}
		else
		{
			//
			// Free up the thing and the barrel.
			//

			BARREL_barrel_upto -= 1;
			free_thing(p_thing);

			return NULL;
		}
	}
	else
	{
		//
		// Free up the barrel.
		//

		BARREL_barrel_upto -= 1;

		return NULL;
	}
}

/*

void BARREL_position_on_hands(Thing *p_barrel, Thing *p_person)
{
	ASSERT(p_barrel->Class == CLASS_BARREL);
	ASSERT(p_person->Class == CLASS_PERSON);

	SLONG lhx;
	SLONG lhy;
	SLONG lhz;

	SLONG rhx;
	SLONG rhy;
	SLONG rhz;

	SLONG lrx;
	SLONG lry;
	SLONG lrz;

	SLONG rrx;
	SLONG rry;
	SLONG rrz;

	if (p_barrel->Genus.Barrel->flag & (BARREL_FLAG_STACKED | BARREL_FLAG_STILL))
	{
		//
		// Convert this barrel to a moving one.
		//

		BARREL_convert_stationary_to_moving(p_barrel);
	}

	//
	// Work out the positions of the two hands.
	//

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_HAND,
	   &lhx,
	   &lhy,
	   &lhz);

	lhx <<= 8;
	lhy <<= 8;
	lhz <<= 8;

	lhx += p_person->WorldPos.X;
	lhy += p_person->WorldPos.Y;
	lhz += p_person->WorldPos.Z;

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_RIGHT_HAND,
	   &rhx,
	   &rhy,
	   &rhz);

	rhx <<= 8;
	rhy <<= 8;
	rhz <<= 8;

	rhx += p_person->WorldPos.X;
	rhy += p_person->WorldPos.Y;
	rhz += p_person->WorldPos.Z;

	//
	// Work out the position of the elbows.
	//

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_RADIUS,
	   &lrx,
	   &lry,
	   &lrz);

	lrx <<= 8;
	lry <<= 8;
	lrz <<= 8;

	lrx += p_person->WorldPos.X;
	lry += p_person->WorldPos.Y;
	lrz += p_person->WorldPos.Z;

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_RIGHT_RADIUS,
	   &rrx,
	   &rry,
	   &rrz);

	rrx <<= 8;
	rry <<= 8;
	rrz <<= 8;

	rrx += p_person->WorldPos.X;
	rry += p_person->WorldPos.Y;
	rrz += p_person->WorldPos.Z;

	//
	// Position the barrel spheres beyond the two hands.
	//

	ASSERT(WITHIN(p_barrel->Genus.Barrel->bs + 0, 2, BARREL_MAX_SPHERES - 1));
	ASSERT(WITHIN(p_barrel->Genus.Barrel->bs + 1, 2, BARREL_MAX_SPHERES - 1));

	BARREL_Sphere *bs1 = &BARREL_sphere[p_barrel->Genus.Barrel->bs + 0];
	BARREL_Sphere *bs2 = &BARREL_sphere[p_barrel->Genus.Barrel->bs + 1];

	//
	// Rememober the old position of the barrel spheres so we can work out their velocity.
	//

	SLONG old_bs1_x = bs1->x;
	SLONG old_bs1_y = bs1->y;
	SLONG old_bs1_z = bs1->z;

	SLONG old_bs2_x = bs2->x;
	SLONG old_bs2_y = bs2->y;
	SLONG old_bs2_z = bs2->z;

	//
	// The new position of the barrels.
	//

	bs1->x = lhx + (lhx - lrx << 1);
	bs1->y = lhy + (lhy - lry << 1);
	bs1->z = lhz + (lhz - lrz << 1);

	bs2->x = rhx + (rhx - rrx << 1);
	bs2->y = rhy + (rhy - rry << 1);
	bs2->z = rhz + (rhz - rrz << 1);

	BARREL_push_apart(
		bs1,
		bs2,
		FALSE);

	//
	// The velocity of the barrels.
	//

	bs1->dx = bs1->x - old_bs1_x;
	bs1->dy = bs1->y - old_bs1_y;
	bs1->dz = bs1->z - old_bs1_z;

	bs2->dx = bs2->x - old_bs2_x;
	bs2->dy = bs2->y - old_bs2_y;
	bs2->dz = bs2->z - old_bs2_z;

	//
	// Remember that this barrel is being held.
	//

	p_barrel->Genus.Barrel->flag |= BARREL_FLAG_HELD;
}

void BARREL_throw(Thing *p_barrel)
{
	//
	// Just mark the barrel as not being held and that will start
	// it processing again.
	//

	p_barrel->Genus.Barrel->flag &= ~BARREL_FLAG_HELD;

	//
	// Make the barrel go in the air a bit more.
	//

	ASSERT(WITHIN(p_barrel->Genus.Barrel->bs + 0, 2, BARREL_MAX_SPHERES - 1));
	ASSERT(WITHIN(p_barrel->Genus.Barrel->bs + 1, 2, BARREL_MAX_SPHERES - 1));

	BARREL_Sphere *bs1 = &BARREL_sphere[p_barrel->Genus.Barrel->bs + 0];
	BARREL_Sphere *bs2 = &BARREL_sphere[p_barrel->Genus.Barrel->bs + 1];

	bs1->dy += 0x1000;
	bs2->dy += 0x1000;
}

*/

#ifndef PSX
GameCoord BARREL_fire_pos(Thing *p_barrel)
{
	SLONG sx;
	SLONG sy;
	SLONG sz;
	SLONG sradius;

	BARREL_Sphere *bs;

	GameCoord ans;

	ASSERT(p_barrel->Class == CLASS_BARREL);

	if (p_barrel->Genus.Barrel->flag & (BARREL_FLAG_STILL|BARREL_FLAG_STACKED))
	{
		BARREL_stacked_sphere(
			p_barrel,
			0,
		   &sx,
		   &sy,
		   &sz,
		   &sradius);

		ans.X = sx;
		ans.Y = sy;
		ans.Z = sz;
	}
	else
	{
		ASSERT(WITHIN(p_barrel->Genus.Barrel->bs, 2, BARREL_MAX_SPHERES - 1));

		bs = &BARREL_sphere[p_barrel->Genus.Barrel->bs];

		ans.X = bs->x;
		ans.Y = bs->y;
		ans.Z = bs->z;
	}

	return ans;
}
#endif

void BARREL_dissapear(Thing *p_barrel)
{
	//
	// If any barrel is stacked on this one...
	//

	UWORD found[8];
	SLONG num_found;

	num_found = THING_find_sphere(
					p_barrel->WorldPos.X >> 8,
					p_barrel->WorldPos.Y >> 8,
					p_barrel->WorldPos.Z >> 8,
					0x200,
					found,
					8,
					1 << CLASS_BARREL);

	SLONG i;

	Thing *p_found;

	for (i = 0; i < num_found; i++)
	{
		p_found = TO_THING(found[i]);

		if (p_found == p_barrel)
		{
			continue;
		}

		if (p_found->Genus.Barrel->flag & BARREL_FLAG_STACKED)
		{
			if (p_found->Genus.Barrel->on == THING_NUMBER(p_barrel))
			{
				//
				// This barrel was stacked on the one that has dissapeared!
				//

				BARREL_convert_stationary_to_moving(p_found);
			}
		}
	}

	//
	// Tell the waypoint that created the barrel that there is
	// no longer a barrel.
	//

	if (p_barrel->Velocity)
	{
		ASSERT(WITHIN(p_barrel->Velocity, 1, EWAY_way_upto - 1));

		ASSERT(EWAY_way[p_barrel->Velocity].ed.type == EWAY_DO_CREATE_BARREL);

		EWAY_way[p_barrel->Velocity].ed.arg1 = NULL;
	}


	if (p_barrel->Genus.Barrel->flag & (BARREL_FLAG_STACKED | BARREL_FLAG_STILL))
	{
		//
		// The barrel doesn't have any sphere structures.
		//
	}
	else
	{
		//
		// Give up the barrel's spheres.
		//

		BARREL_convert_moving_to_stationary(p_barrel);
	}

	remove_thing_from_map(p_barrel);
	free_draw_mesh(p_barrel->Draw.Mesh);
	free_thing(p_barrel);

	if (NET_PERSON(0)->Genus.Person->Target == THING_NUMBER(p_barrel))
	{
		//
		// Make the player choose a new target.
		//

		NET_PERSON(0)->Genus.Person->Target = NULL;
	}
}


void BARREL_shoot(
		Thing *p_barrel,
		Thing *p_shooter)
{
	ULONG in_the_air;
	SWORD wave;

	if ( (p_barrel->Genus.Barrel->flag & BARREL_FLAG_STILL) &&
		!(p_barrel->Genus.Barrel->flag & BARREL_FLAG_STACKED))
	{
		in_the_air = FALSE;
	}
	else
	{
		in_the_air = TRUE;
	}


	GameCoord barrelpos = p_barrel->WorldPos;

	wave=S_EXPLODE_SMALL;
	if (!(Random()&3)) wave++; // 25% chance of a bigger bang than usual
	MFX_play_xyz(THING_NUMBER(p_barrel),wave,0,barrelpos.X,barrelpos.Y,barrelpos.Z);

	if (p_barrel->Genus.Barrel->type != BARREL_TYPE_CONE &&
		p_barrel->Genus.Barrel->type != BARREL_TYPE_BIN)
	{
		if (p_shooter->Genus.Person->Target == THING_NUMBER(p_barrel))
		{
			//
			// Stop this person targetting the barrel... because the
			// barrel is going!
			//

			p_shooter->Genus.Person->Target = NULL;
		}

		BARREL_dissapear(p_barrel);

		/*

		if (in_the_air)
		{
			SLONG i;
			GameCoord pos = barrelpos;

			PARTICLE_Add(pos.X,pos.Y,pos.Z, 0, 0, 0, POLY_PAGE_SMOKECLOUD, 2, 0xFFFFFFFF, PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE|PFLAG_RESIZE, 100, 440+(Random()&0x7f), 1, 3, 50);

			PARTICLE_Add(pos.X,pos.Y,pos.Z, 0, 0, 0, POLY_PAGE_EXPLODE1-(Random()&1), 2, 0xFFFFFF, PFLAG_SPRITEANI|PFLAG_RESIZE, 20, 120+(Random()&0x7f), 1, 0, 30);
			PARTICLE_Add(pos.X,pos.Y,pos.Z, 0, 0, 0, POLY_PAGE_EXPLODE1-(Random()&1), 2, 0x7fFFFFFF, PFLAG_SPRITEANI|PFLAG_RESIZE, 20, 120+(Random()&0x7f), 1, 0, 40);
			for (i=0;i<25;i++) {
			  PARTICLE_Add(pos.X,pos.Y,pos.Z, ((Random()&0x1f)-0xf)<<6, (Random()&0x1f)<<6, ((Random()&0x1f)-0xf)<<6, POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE|PFLAG_INVALPHA, 240, 20+(Random()&0x1f), 1, 3+(Random()&3), 0);
			  if (Random()&3)
				PARTICLE_Add(pos.X,pos.Y,pos.Z, ((Random()&0x1f)-0xf)<<8, (Random()&0x1f)<<8, ((Random()&0x1f)-0xf)<<8, POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE|PFLAG_INVALPHA|PFLAG_BOUNCE, 240, 20+(Random()&0x1f), 1, 2+(Random()&3), 0);
			  else
				PARTICLE_Add(pos.X,pos.Y,pos.Z, ((Random()&0x1f)-0xf)<<12, (Random()&0x1f)<<8, ((Random()&0x1f)-0xf)<<12, POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE|PFLAG_INVALPHA, 240, 20+(Random()&0x1f), 1, 3, 0);
			}
		}
		else
		{
			PYRO_construct(
				barrelpos,
			   -1,
				256);
		}

		*/
#ifdef PSX
		POW_create(
			(in_the_air) ? POW_CREATE_LARGE : POW_CREATE_LARGE_SEMI,
			barrelpos.X,
			barrelpos.Y,
			barrelpos.Z,
			0,0,0);
#else
		PYRO_create(barrelpos,PYRO_FIREBOMB);
#endif

		PCOM_oscillate_tympanum(
			PCOM_SOUND_BANG,
			p_shooter,
			barrelpos.X >> 8,
			barrelpos.Y >> 8,
			barrelpos.Z >> 8);

		create_shockwave(
			barrelpos.X >> 8,
			barrelpos.Y >> 8,
			barrelpos.Z >> 8,
			0x200,
			250,
			p_shooter);

		BARREL_hit_with_sphere(
			barrelpos.X          >> 8,
			barrelpos.Y - 0x2000 >> 8,
			barrelpos.Z          >> 8,
			0x60);
	}
	else
	{
		BARREL_hit_with_sphere(
			barrelpos.X + (Random() & 0x1fff) - 0x0fff >> 8,
			barrelpos.Y                       - 0x1400 >> 8,
			barrelpos.Z + (Random() & 0x1fff) - 0x0fff >> 8,
			0x15);
	}
}


SLONG BARREL_is_stacked(Thing *p_barrel)
{
	return p_barrel->Genus.Barrel->flag & BARREL_FLAG_STACKED;
}
