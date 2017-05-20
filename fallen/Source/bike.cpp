//
// Motorbikes.
//

#ifdef BIKE

#include "game.h"
#include "fmatrix.h"
#include "mav.h"
#include "pap.h"
#include "statedef.h"
#include "collide.h"
#include "dirt.h"
#include "mist.h"
#include "tracks.h"
#include "ribbon.h"
#include "psystem.h"
#include "road.h"

#include "poly.h"

#include "memory.h"
#include "mfx.h"
#include "sound_id.h"

extern	void	add_debug_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG colour);

//
// The radius a each wheel and how far apart they are
//

#define BIKE_WHEEL_SUS	   0x10
#define BIKE_WHEEL_RADIUS  0x18
#define BIKE_WHEEL_APART   0xa0
#define BIKE_WHEEL_COL     0x50		// The radius that the wheels collide with.


//
// The back wheel drives and the front wheel gets pushed along.
//

#define BIKE_FLAG_USED           (1 << 0)
#define BIKE_FLAG_ONGROUND_FRONT (1 << 1)
#define BIKE_FLAG_ONGROUND_BACK  (1 << 2)



BIKE_Bike *BIKE_bike; //[BIKE_MAX_BIKES];
SLONG      BIKE_bike_upto;


#ifndef PSX
void BIKE_init()
{
	memset(BIKE_bike, 0, sizeof(BIKE_bike) * BIKE_MAX_BIKES);

	BIKE_bike_upto = 1;
}
#endif
//
// Returns the friction at the given (x,z) 8-bits per mapsquare.
// The answer is given in 8-bit fixed point. 256 == 100% friction.
//

SLONG BIKE_get_ground_friction(SLONG x, SLONG z)
{
	SLONG mx = x >> 8;
	SLONG mz = z >> 8;

	switch(ROAD_get_mapsquare_type(mx,mz))
	{
		default:
		case ROAD_TYPE_TARMAC:
			return 200;

		case ROAD_TYPE_GRASS:
			return 80;

		case ROAD_TYPE_DIRT:
			return 80;

		case ROAD_TYPE_SLIPPERY:
			return 32;
	}
}




//
//					   /
//					  /   STEERING VECTOR
//					 /
//					/
//	BACK +---------+ FRONT
//
// This function returns how much of the steering vector you have to add onto the
// front point to make the back and front points be 'apart' distance apart from
// one another.  The steering vector should be normalised to 16-bit fixed point.
//

SLONG BIKE_steering_push(
		SLONG bx,
		SLONG bz,
		SLONG fx,
		SLONG fz,
		SLONG sx,		// Steering y-vector is assumed to be 0 and (x,z) should be normalised to 0x10000
		SLONG sz,
		SLONG clen)
{
	//															   | This is a vector we construct, p
	//															   |
	//															   |
	//															   |
	// Here's how we do it.										   |
	//													           +-
    //                                                          - #|
    //                                                       -   # |
    //                                                    -     #  |
    //                                                 -       #   |
    //                                              -         #    |
    //                                           -           #     |
    //                                        -             #      |
    //                                c    -               #       |
    //                                  -                 #        | |p|.x|s| = x(|p|.|s|)
    //                               -                   #         |
    //                            -           STEERING  # xs       |
    //                         -               VECTOR  #           |
    //                      -                         #            |
    //                   -                           #             |
    //                -                             #              |
    //             -                               #               |
    //          -                                 #             +--|
    //       -                                   #              |  |
	// BACK +###################################+------------------+-
	//					a						|				   |
	//										  FRONT   |a|.x|s| = x(|a|.|s|)
	//
	// By pythag. c^2 = (|a| + x(|a|.|s|))^2 + (x(|p|.|s|))^2
	//
	// We solve for x to get a quadratic that we plug through the formula,
	//
	//		x = (1/2a) * (-b +- Root(b^2 - 4ac))
	//
	// The vector, p, is perpendicular to vector a.
	//

	//
	// Make our vectors.
	//

	SLONG ax = fx - bx;
	SLONG az = fz - bz;

	//
	// The normalised version of a
	//

	SLONG alen = QDIST2(abs(ax),abs(az)) + 1;

	SLONG nax = DIV64(ax, alen);
	SLONG naz = DIV64(az, alen);

	//
	// The vector p
	// 

	SLONG npx = -naz;
	SLONG npz = +nax;

	//
	// The vector s sbould be normalised already.
	//

	SLONG adots = MUL64(nax,sx) + MUL64(naz,sz);
	SLONG pdots = MUL64(npx,sx) + MUL64(npz,sz);

	//
	// After solving the pythag formula for x we get the quadratic
	// Ax2 + Bx + C = 0 where A, B and C is...
	//

	SLONG A = MUL64(adots,adots) + MUL64(pdots,pdots);
	SLONG B = 2 * MUL64(adots,alen);
	SLONG C = MUL64(alen,alen) - MUL64(clen,clen);

	if (abs(A) < 0x20)
	{
		//
		// Oh dear- we are going to want to divide by this!
		//

		return 0;
	}

	//
	// Now plug these values into the formula.
	//

	SLONG b2m4ac = MUL64(B,B) - 4 * MUL64(A,C);

	//
	// We don't want any complex numbers!
	//

	SLONG plusorminus;

	if (b2m4ac <= 0)
	{
		plusorminus = 0;
	}
	else
	{
		plusorminus = Root(b2m4ac) << 8;
	}

	SLONG x1 = -B + plusorminus;
	SLONG x2 = -B - plusorminus;

	SLONG x;

	if (abs(x1) < abs(x2))
	{
		x = x1;
	}
	else
	{
		x = x2;
	}

	x = DIV64(x, 2 * A);

	//
	// And we finally have the answer.
	//

	return x;
}




//
// Processes a set of suspension.  All args in 16-bits per pixel.
// 

#define BIKE_GRAVITY (-0xc0)

SLONG BIKE_process_suspension(
		SLONG  sus_x,
		SLONG  sus_z,
		SLONG *frame_y,
		SLONG *frame_dy,
		SLONG *wheel_y,
		SLONG *wheel_dy)
{
	SLONG onground = FALSE;

	SLONG ground = PAP_calc_map_height_at(sus_x >> 8, sus_z >> 8) << 8;
	
	SLONG fy  = *frame_y;
	SLONG fdy = *frame_dy;
	SLONG wy  = *wheel_y;
	SLONG wdy = *wheel_dy;

	//
	// Add gravity.
	//

	fdy += BIKE_GRAVITY;
	wdy += BIKE_GRAVITY;

	//
	// How long is the suspension spring?
	//

	SLONG sdist  = fy - wy;
	SLONG sddist = (BIKE_WHEEL_SUS << 8) - sdist;

	//
	// The force of the suspension.
	//

	fdy += sddist / 8;	// Was 4
	wdy -= sddist / 8;	// Was 4

	//
	// Move the wheels and the frame.
	//

	fy += fdy;
	wy += wdy;

	SLONG ddy = fdy - wdy;

	wdy += ddy / 128;	// Was 64
	fdy -= ddy / 128;	// Was 64

	//
	// Treat wheels that are slightly above the ground as being on the ground
	// so we can grip better...
	//

	if (wy <= ground + (BIKE_WHEEL_RADIUS + 0x2 << 8))
	{
		onground = TRUE;
	}

	//
	// Make sure the wheel does not go underground.
	//

	if (wy <= ground + (BIKE_WHEEL_RADIUS << 8))
	{	
		wdy = abs(wdy) - (abs(wdy) >> 2);
		wy  = ground + (BIKE_WHEEL_RADIUS << 8);
	}

	//
	// Make sure the wheel and frame dont get too close.
	//

	if (fy < wy + (BIKE_WHEEL_SUS << 6))
	{
		fy = wy + (BIKE_WHEEL_SUS << 6);
	}
	
	//
	// Damp velocities.
	//

	fdy -= fdy / 512;	// Was 512
	wdy -= wdy / 512;	// Was 512

	//
	// Return the results.
	// 

   *frame_y  = fy;
   *frame_dy = fdy;
   *wheel_y  = wy;
   *wheel_dy = wdy;

	return onground;
}

//
// Finds bounding boxes and bounding sphere that will be collided with by
// BIKE_collide_sphere()
//

void BIKE_collide_init(Thing *p_bike)
{
	VEH_collide_find_things(
		p_bike->WorldPos.X >> 8,
		p_bike->WorldPos.Y >> 8,
		p_bike->WorldPos.Z >> 8,
		0x100,
		THING_NUMBER(p_bike));
}



//
// Collides the given sphere with the world. It pushes it away from
// buildings and fences.  Returns the number of collisions that occurred.
//

SLONG BIKE_collide_sphere(
		SLONG *sphere_x,
		SLONG *sphere_y,
		SLONG *sphere_z)
{
	SLONG i;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG sin_angle;
	SLONG cos_angle;
	SLONG matrix[4];

	SLONG collided = 0;

	SLONG map_x = *sphere_x >> 16;
	SLONG map_z = *sphere_z >> 16;

	SLONG sx = *sphere_x;
	SLONG sy = *sphere_y;
	SLONG sz = *sphere_z;

	SLONG lx;
	SLONG lz;

	SLONG maxup = (sy >> 8) + 0x50 >> 6;

	VEH_Col *vc;

	//
	// Collision with the sides of fences and buildings.
	//

	if (WITHIN(map_x, 0, PAP_SIZE_HI - 1) &&
		WITHIN(map_z, 0, PAP_SIZE_HI - 1))
	{
		MAV_Opt mo = MAV_opt[MAV_NAV(map_x,map_z)];

		//
		// The mapsquare edges.
		// 

		if (!(mo.opt[MAV_DIR_XS] & (MAV_CAPS_GOTO)) || (map_x > 0 && MAVHEIGHT(map_x - 1, map_z) > maxup))
		{
			//
			// The sphere shouldn't be near the (x-small) bit of this mapsquare.
			//

			if ((sx & 0xffff) < (BIKE_WHEEL_COL << 8))
			{
				sx &= ~0xffff;
				sx |=  BIKE_WHEEL_COL << 8;

				collided += 1;
			}
		}

		if (!(mo.opt[MAV_DIR_XL] & (MAV_CAPS_GOTO)) || (map_x < (PAP_SIZE_HI - 1) && MAVHEIGHT(map_x + 1, map_z) > maxup))
		{
			//
			// The sphere shouldn't be near the (x-large) bit of this mapsquare.
			//

			if ((sx & 0xffff) > 0x10000 - (BIKE_WHEEL_COL << 8))
			{
				sx &= ~0xffff;
				sx |=  0x10000 - (BIKE_WHEEL_COL << 8);

				collided += 1;
			}
		}

		if (!(mo.opt[MAV_DIR_ZS] & (MAV_CAPS_GOTO)) || (map_z > 0 && MAVHEIGHT(map_x, map_z - 1) > maxup))
		{
			//
			// The sphere shouldn't be near the (z-small) bit of this mapsquare.
			//

			if ((sz & 0xffff) < (BIKE_WHEEL_COL << 8))
			{
				sz &= ~0xffff;
				sz |=  BIKE_WHEEL_COL << 8;

				collided += 1;
			}
		}

		if (!(mo.opt[MAV_DIR_ZL] & (MAV_CAPS_GOTO)) || (map_z < (PAP_SIZE_HI - 1) && MAVHEIGHT(map_x, map_z + 1) > maxup))
		{
			//
			// The sphere shouldn't be near the (z-large) bit of this mapsquare.
			//

			if ((sz & 0xffff) > 0x10000 - (BIKE_WHEEL_COL << 8))
			{
				sz &= ~0xffff;
				sz |=  0x10000 - (BIKE_WHEEL_COL << 8);

				collided += 1;
			}
		}

		//
		// The mapsquare corners
		//

		if (map_x > 0)
		{
			if (map_z > 0)
			{
				if (MAVHEIGHT(map_x - 1, map_z - 1) >= maxup)
				{
					if ((sx & 0xffff) < (BIKE_WHEEL_COL << 8) &&
						(sz & 0xffff) < (BIKE_WHEEL_COL << 8))
					{
						sx &= ~0xffff;
						sz &= ~0xffff;

						sx |=  BIKE_WHEEL_COL << 8;
						sz |=  BIKE_WHEEL_COL << 8;

						collided += 1;
					}
				}
			}

			if (map_z < PAP_SIZE_HI - 1)
			{
				if (MAVHEIGHT(map_x - 1, map_z + 1) >= maxup)
				{
					if ((sx & 0xffff) <           (BIKE_WHEEL_COL << 8) &&
						(sz & 0xffff) > 0x10000 - (BIKE_WHEEL_COL << 8))
					{
						sx &= ~0xffff;
						sz &= ~0xffff;

						sx |=            (BIKE_WHEEL_COL << 8);
						sz |=  0x10000 - (BIKE_WHEEL_COL << 8);

						collided += 1;
					}
				}
			}
		}

		if (map_x < PAP_SIZE_HI - 1)
		{
			if (map_z > 0)
			{
				if (MAVHEIGHT(map_x + 1, map_z - 1) >= maxup)
				{
					if ((sx & 0xffff) > 0x10000 - (BIKE_WHEEL_COL << 8) &&
						(sz & 0xffff) <           (BIKE_WHEEL_COL << 8))
					{
						sx &= ~0xffff;
						sz &= ~0xffff;

						sx |=  0x10000 - (BIKE_WHEEL_COL << 8);
						sz |=            (BIKE_WHEEL_COL << 8);

						collided += 1;
					}
				}
			}

			if (map_z < PAP_SIZE_HI - 1)
			{
				if (MAVHEIGHT(map_x + 1, map_z + 1) >= maxup)
				{
					if ((sx & 0xffff) > 0x10000 - (BIKE_WHEEL_COL << 8) &&
						(sz & 0xffff) > 0x10000 - (BIKE_WHEEL_COL << 8))
					{
						sx &= ~0xffff;
						sz &= ~0xffff;

						sx |=  0x10000 - (BIKE_WHEEL_COL << 8);
						sz |=  0x10000 - (BIKE_WHEEL_COL << 8);

						collided += 1;
					}
				}
			}
		}
	}

	//
	// Collision with all the bounding boxes and cylinders found by 
	// VEH_collide_find_things().
	//

	for (i = 0; i < VEH_col_upto; i++)
	{
		vc = &VEH_col[i];

		switch(vc->type)
		{
			case VEH_COL_TYPE_BBOX:

				lx = sx >> 8;
				lz = sz >> 8;

				if (slide_around_box(
						vc->mid_x,
						vc->mid_z,
						vc->min_x,
						vc->min_z,
						vc->max_x,
						vc->max_z,
						vc->radius_or_yaw,
						BIKE_WHEEL_COL,
						lx,
						lz,
					   &lx,
					   &lz))
				{
					collided += 1;

					sx = lx << 8;
					sz = lz << 8;
				}

				break;

			case VEH_COL_TYPE_CYLINDER:

				dx = vc->mid_x - (sx >> 8);
				dz = vc->mid_z - (sz >> 8);

				dist = QDIST2(abs(dx),abs(dz)) + 1;

				if (dist < BIKE_WHEEL_COL + vc->radius_or_yaw)
				{
					dx = dx * (BIKE_WHEEL_COL + vc->radius_or_yaw) / dist;
					dz = dz * (BIKE_WHEEL_COL + vc->radius_or_yaw) / dist;

					sx = vc->mid_x - dx << 8;
					sz = vc->mid_z - dz << 8;

					collided += 1;
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}

   *sphere_x = sx;
   *sphere_y = sy;
   *sphere_z = sz;

	return collided;
}



//
// Steering/skidding/oversteer for the bike is done in 2D.  The y-coordinates
// of the wheels are completely independent.
//

void BIKE_process_normal(Thing *p_bike)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dxdz;
	SLONG vector[3];
	SLONG steer [3];
	SLONG friction;
	SLONG dprod;
	SLONG cprod;
	SLONG radx;
	SLONG radz;
	SLONG dist;
	SLONG ddist;
	SLONG along;
	SLONG front_old_x;
	SLONG front_old_z;
	SLONG steer_angle;
	SLONG ax;
	SLONG az;
	SLONG bx;
	SLONG bz;
	SLONG old_back_x;
	SLONG old_back_z;
	UBYTE go_again = 1;
	UBYTE surface;

	BIKE_Bike *bb = p_bike->Genus.Bike;

	if (bb->mode == BIKE_MODE_PARKED   ||
		bb->mode == BIKE_MODE_MOUNTING ||
		bb->mode == BIKE_MODE_DISMOUNTING)
	{
		go_again=0;
	}
	else
	{
//extern	void	MFX_set_pitch(UWORD channel_id, UWORD wave, SLONG pitchbend);

		if ((bb->flag & BIKE_FLAG_ONGROUND_BACK))
		{
			MFX_set_pitch(THING_NUMBER(p_bike),S_BIKE_IDLE, abs(BIKE_get_speed(p_bike))<<2);
		}
		else
		{
			MFX_set_pitch(THING_NUMBER(p_bike),S_BIKE_IDLE, abs(BIKE_get_speed(p_bike))<<3);
		}

		if (bb->flag & BIKE_FLAG_ONGROUND_BACK)
		{
			SLONG rspeed;

			rspeed  = QDIST2(abs(bb->back_dx), abs(bb->back_dz));
			surface = ROAD_get_mapsquare_type(
						bb->back_x >> 16,
						bb->back_z >> 16);

			if ((surface == ROAD_TYPE_DIRT && rspeed > 2000) || bb->SlideTimer)
			{
				bb->dirt += 2;
			}
		}
	}

again:;

	if (bb->dirt)
	{
		bb->dirt -= 1;

		if (bb->dirt > 10)
		{
			bb->dirt = 10;
		}

		SLONG i;
		SLONG rgb;
		
		rgb   = i << 4;
		rgb <<= 24;
		rgb  |= 0xc9b7a3;

		for (i = 0; i < bb->dirt; i++)
		{
			PARTICLE_Add(
				bb->back_x + (bb->back_dx * (Random() & 0xff) >> 8) + (Random() & 0x7ff) - 0x3ff,
				bb->back_y + (bb->back_dy * (Random() & 0xff) >> 8) + (Random() & 0x7ff) - 0x3ff  - 0x1000,
				bb->back_z + (bb->back_dz * (Random() & 0xff) >> 8) + (Random() & 0x7ff) - 0x3ff,
				(Random()&0xf)-0x7,
				20,
				(Random()&0xf)-0x7,
				POLY_PAGE_SMOKECLOUD2,
				2+((Random()&3)<<2),
				rgb,
				PFLAG_FADE|PFLAG_RESIZE|PFLAG_SPRITEANI|PFLAG_SPRITELOOP,40,25,1,10,4);
		}
	}

	//
	// Initialise collision for this bike.
	//

	BIKE_collide_init(p_bike);

	front_old_x = bb->front_x;
	front_old_z = bb->front_z;

	FMATRIX_vector(
		vector,
	   (p_bike->Genus.Bike->yaw + 1024) & 2047,
		0);

	if(bb->SlideTimer)
	{
		bb->SlideTimer-=TICK_RATIO;
		if(bb->SlideTimer<0)
			bb->SlideTimer=0;

	}

	if ((bb->flag & BIKE_FLAG_ONGROUND_BACK) && (bb->SlideTimer==0)) //||bb->accel>50))
	{
		//
		// The friction under the back wheel.
		//

		friction = BIKE_get_ground_friction(bb->back_x >> 8, bb->back_z >> 8);

		if (bb->mode == BIKE_MODE_PARKED   ||
			bb->mode == BIKE_MODE_MOUNTING ||
			bb->mode == BIKE_MODE_DISMOUNTING)
		{
			bb->back_dx = 0;
			bb->back_dz = 0;
		}
		else
		{
			//
			// Accelerate the back wheel.
			//

			bb->back_dx += (bb->accel * vector[0] >> 8) * (friction + 256) >> 13;
			bb->back_dz += (bb->accel * vector[2] >> 8) * (friction + 256) >> 13;
		
			//
			// Stop the wheel sliding on over the surface.
			//

			radx  = -vector[2];
			radz  =  vector[0];

			dprod   = MUL64(radx,bb->back_dx) + MUL64(radz,bb->back_dz);
			dprod  *= friction;
			dprod >>= 8;

			bb->back_dx -= radx * dprod >> 16;
			bb->back_dz -= radz * dprod >> 16;
		}
	}

	//
	// Move the back wheel.
	//

	old_back_x = bb->back_x;
	old_back_z = bb->back_z;

	{
		SLONG	tx,tz,dist;
		tx=bb->back_dx*TICK_RATIO>>TICK_SHIFT;
		tz=bb->back_dz*TICK_RATIO>>TICK_SHIFT;

		add_debug_line(bb->back_x>>8,bb->back_y>>8,bb->back_z>>8,bb->back_x+tx>>8,bb->back_y>>8,bb->back_z+tz>>8,0xff0000);

		ASSERT(QDIST2(abs(tx>>8),abs(tz>>8))<BIKE_WHEEL_APART);

		bb->back_x += tx; //bb->back_dx*TICK_RATIO>>TICK_SHIFT;
		bb->back_z += tz; //bb->back_dz*TICK_RATIO>>TICK_SHIFT;
	}

	//
	// Have the back wheel kick up leaves for that funky wheelspin effect
	//

	DIRT_gust(p_bike,bb->back_x>>8,bb->back_z>>8,old_back_x>>8,old_back_z>>8);
#ifndef PSX
	MIST_gust(old_back_x>>8,old_back_z>>8,bb->back_x>>8,bb->back_z>>8);
	
	// ribbon was used for rear light glow but it looked crap.
	// now it's going to try and thicken out the exhaust...
	if (bb->ribbon) {
		SLONG matrix[9];
		FMATRIX_calc(matrix, p_bike->Genus.Bike->yaw, -p_bike->Genus.Bike->pitch & 2047, BIKE_get_roll(p_bike));
		FMATRIX_TRANSPOSE(matrix);
		vector[2]=20; vector[1]=30; vector[0]=0;
		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);

/*		if (GAME_TURN&1)
		  RIBBON_extend(bb->ribbon,vector[0]+(p_bike->WorldPos.X>>8),vector[1]+(p_bike->WorldPos.Y>>8),vector[2]+(p_bike->WorldPos.Z>>8));
		else
		  RIBBON_extend(bb->ribbon,vector[0]+(p_bike->WorldPos.X>>8),vector[1]+(p_bike->WorldPos.Y>>8)+20,vector[2]+(p_bike->WorldPos.Z>>8));*/

		  RIBBON_extend(bb->ribbon,vector[0]+(p_bike->WorldPos.X>>8),vector[1]+(p_bike->WorldPos.Y>>8)-35,vector[2]+(p_bike->WorldPos.Z>>8));
		  RIBBON_extend(bb->ribbon,vector[0]+(p_bike->WorldPos.X>>8),vector[1]+(p_bike->WorldPos.Y>>8)+35,vector[2]+(p_bike->WorldPos.Z>>8));

		vector[2]=20; vector[1]=30; vector[0]=-30;
		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
		  RIBBON_extend(bb->ribbon2,vector[0]+(p_bike->WorldPos.X>>8),vector[1]+(p_bike->WorldPos.Y>>8),vector[2]+(p_bike->WorldPos.Z>>8));
		vector[2]=20; vector[1]=30; vector[0]=30;
		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
		  RIBBON_extend(bb->ribbon2,vector[0]+(p_bike->WorldPos.X>>8),vector[1]+(p_bike->WorldPos.Y>>8),vector[2]+(p_bike->WorldPos.Z>>8));

	}
//	TRACE("bike d's %d / %d \n",bb->back_dx,bb->back_dz);
#endif
//	if (GAME_TURN&1) 
	bb->tyrelast=TRACKS_Add(bb->back_x,bb->back_y,bb->back_z,bb->back_dx>>8,bb->back_dy>>8,bb->back_dz>>8,TRACK_TYPE_TYRE,bb->tyrelast);

	//
	// Make sure the back wheel doesn't go through a wall.
	//

	if (BIKE_collide_sphere(
			&bb->back_x,
			&bb->back_y,
			&bb->back_z))
	{
		bb->accel >>= 1;
	}

	//
	// Damping of steering
	//

	bb->steer -= bb->steer / (2*2);

	if (abs(bb->steer) < 2*2) {bb->steer = 0;}

	if ((bb->flag & (BIKE_FLAG_ONGROUND_FRONT|BIKE_FLAG_ONGROUND_BACK)) && bb->SlideTimer==0)
	{	
		bb->accel -= bb->accel / (8*2);
	}

	//
	// Bikes can't go backwards!
	//

	if (bb->accel > 0)
	{
		if (bb->accel < 4)
		{
			bb->accel = 0;
		}
	}
	else
	{
		bb->accel /= 2;
		bb->accel += 1;
	}

	//
	// Damping of the bikes movement...
	//

	if(bb->SlideTimer)
	{
		bb->back_dx -= bb->back_dx / (8*2);
		bb->back_dz -= bb->back_dz / (8*2);

		if (abs(bb->back_dx) < 8*2) {bb->back_dx = 0;}
		if (abs(bb->back_dz) < 8*2) {bb->back_dz = 0;}
	}
	else
	if ((bb->flag & BIKE_FLAG_ONGROUND_BACK))// && bb->SlideTimer==0)
	{
		bb->back_dx -= bb->back_dx / (8*2);
		bb->back_dz -= bb->back_dz / (8*2);

		if (abs(bb->back_dx) < 8*2) {bb->back_dx = 0;}
		if (abs(bb->back_dz) < 8*2) {bb->back_dz = 0;}
	}

	if (bb->flag & BIKE_FLAG_ONGROUND_FRONT)
	{
		//
		// What is the friction at the front wheel?
		//

		friction = BIKE_get_ground_friction(bb->front_x >> 8, bb->front_z >> 8);
	}
	else
	{
		//
		// No friction in midair.
		//

		friction = 0;
	}

	//
	// Make sure the front wheel is far enough away from the back wheel.
	//

	dx = bb->front_x - bb->back_x;
	dz = bb->front_z - bb->back_z;

	dist  = QDIST2(abs(dx),abs(dz));
	ddist = (BIKE_WHEEL_APART << 8) - dist;

	//
	// Move the front wheel along the line connecting the front and back
	// wheels. The percentage of the ddist we move along this line depends on
	// the friction under the front wheel.
	//

	along = ddist * (256 - friction) >> 12;

	{
		SLONG	tx,tz;
		tx=dx * along / ((dist >> 4) + 1);
		tz=dz * along / ((dist >> 4) + 1);

		
		add_debug_line(bb->front_x>>8,bb->front_y>>8,bb->front_z>>8,bb->front_x-tx>>8,bb->front_y>>8,bb->front_z-tz>>8,0xff0000);

		bb->front_x-=tx;
		bb->front_z-=tz;
	}

	//
	// Make sure the front wheel doesn't go through a wall.
	//

	SLONG before_col_x = bb->front_x;
	SLONG before_col_z = bb->front_z;

	SLONG col = BIKE_collide_sphere(
					&bb->front_x,
					&bb->front_y,
					&bb->front_z);

	if (col)
	{
		//
		// Slow down if we've collided.
		//

		if (bb->accel > 12)
		{
			bb->accel >>= 1;
		}
		
		//
		// Steer away from the wall.
		// 

		ax = bb->front_x - before_col_x >> 8;
		az = bb->front_z - before_col_z >> 8;

		bx = before_col_x - bb->back_x >> 8;
		bz = before_col_z - bb->back_z >> 8;

		if (QDIST2(abs(ax),abs(az)) > 128)
		{
			cprod = ax*bz - az*bx;

			if (cprod >= 0)
			{
				if (bb->steer < 100)
				{
					bb->steer += 20;
				}
			}
			else
			{
				if (bb->steer > -100)
				{
					bb->steer -= 20;
				}
			}
		}
	}

	//
	// Now work out the steering vector.
	//

	if (bb->flag & BIKE_FLAG_ONGROUND_FRONT)
	{
		steer_angle  = p_bike->Genus.Bike->yaw + 1024;
		steer_angle += bb->steer << 3;
		steer_angle &= 2047;
	}
	else
	{
		steer_angle = p_bike->Genus.Bike->yaw + 1024;
	}

	FMATRIX_vector(
		steer,
		steer_angle,
		0);

	//
	// Push the front wheel away from the back wheel along the steering vector.
	//

	along = BIKE_steering_push(
				bb->back_x,
				bb->back_z,
				bb->front_x,
				bb->front_z,
				steer[0],
				steer[2],
				BIKE_WHEEL_APART << 8);

	{
		SLONG	tx,tz;
		tx=MUL64(along, steer[0]);
		tz=MUL64(along, steer[2]);

		add_debug_line(bb->front_x>>8,bb->front_y>>8,bb->front_z>>8,bb->front_x+tx>>8,bb->front_y>>8,bb->front_z+tz>>8,0xff0000);

		bb->front_x += tx; //MUL64(along, steer[0]); //*TICK_RATIO>>TICK_SHIFT;
		bb->front_z += tz; //MUL64(along, steer[2]); //*TICK_RATIO>>TICK_SHIFT;
	}

	//
	// Has the front wheel of the bike gone into a wall?
	// Only check after a front wheel collision.
	//

	if (col)
	{
		SLONG alongx1;
		SLONG alongz1;
		SLONG alongx2;
		SLONG alongz2;

		//
		// Is the front wheel skewered through a fence?
		//

		SLONG map_dx = (bb->front_x >> 8) - (bb->back_x >> 8);
		SLONG map_dz = (bb->front_z >> 8) - (bb->back_z >> 8);

		if (abs(map_dx) > abs(map_dz))
		{	
			if ((bb->front_x >> 16) != (bb->back_x >> 16))
			{
				alongx1   =  MAX(bb->front_x, bb->back_x);
				alongx1 >>=  8;
				alongx1  &= ~0xff;
				alongx2   =  alongx1;

				alongz1 = bb->front_z >> 8;
				alongz2 = bb->back_z  >> 8;

				alongz1 &= ~0xff;
				alongz2 &= ~0xff;
				alongz2 +=  0x100;

				if (alongz2 < alongz1)
				{
					SWAP(alongz1, alongz2);
				}

				if (does_fence_lie_along_line(
						alongx1,
						alongz1,
						alongx2,
						alongz2))
				{
					col += 2;	// Make sure movement is not allowed.
				}
			}
		}
		else
		{
			if ((bb->front_z >> 16) != (bb->back_z >> 16))
			{
				alongz1   =  MAX(bb->front_z, bb->back_z);
				alongz1 >>=  8;
				alongz1  &= ~0xff;
				alongz2   =  alongz1;

				alongx1 = bb->front_x >> 8;
				alongx2 = bb->back_x  >> 8;

				alongx1 &= ~0xff;
				alongx2 &= ~0xff;
				alongx2 +=  0x100;

				if (alongx2 < alongx1)
				{
					SWAP(alongx1, alongx2);
				}

				if (does_fence_lie_along_line(
						alongx1,
						alongz1,
						alongx2,
						alongz2))
				{
					col += 2;	// Make sure movement is not allowed.
				}
			}
		}

		dy = PAP_calc_map_height_at(bb->front_x>>8,bb->front_z>>8) - (bb->back_y >> 8);

		if (abs(dy) >= 0x40 || col >= 2)
		{
			//
			// Dont allow movement!
			//

			bb->back_x = old_back_x;// - (bb->back_dx << 2);
			bb->back_z = old_back_z;// - (bb->back_dz << 2);

			bb->front_x = front_old_x;// - (bb->back_dx << 2);
			bb->front_z = front_old_z;// - (bb->back_dz << 2);

			if (abs(bb->back_dx) + abs(bb->back_dz) > 0x200)
			{
				bb->back_x -= bb->back_dx << 2;
				bb->back_z -= bb->back_dz << 2;

				bb->front_x -= bb->back_dx << 2;
				bb->front_z -= bb->back_dz << 2;

				bb->back_dx = -bb->back_dx;
				bb->back_dz = -bb->back_dz;

				bb->back_dx -= bb->back_dx >> 2;
				bb->back_dz -= bb->back_dz >> 2;
				
				if (abs(bb->back_dx) + abs(bb->back_dz) > 0x2000)
				{
					//
					// Make the bike bounce a bit.
					//

					bb->front_dy += 5000;
					bb->back_dy  += 3500;
				}
			}

			bb->accel >>= 2;
		}
	}

	//
	// Suspension on front and back.
	//

	bb->flag &= ~(BIKE_FLAG_ONGROUND_FRONT | BIKE_FLAG_ONGROUND_BACK);

	if (BIKE_process_suspension(
			bb->front_x,
			bb->front_z,
		   &bb->front_y,
		   &bb->front_dy,
		   &bb->wheel_y_front,
		   &bb->wheel_dy_front))
	{
		bb->flag |= BIKE_FLAG_ONGROUND_FRONT;
	}

	if (BIKE_process_suspension(
			bb->back_x,
			bb->back_z,
		   &bb->back_y,
		   &bb->back_dy,
		   &bb->wheel_y_back,
		   &bb->wheel_dy_back))
	{
		bb->flag |= BIKE_FLAG_ONGROUND_BACK;
	}

	if ( (bb->flag & BIKE_FLAG_ONGROUND_BACK) &&
		!(bb->flag & BIKE_FLAG_ONGROUND_FRONT))
	{
		bb->front_dy -= BIKE_GRAVITY / 2;
	}

	//
	// Make our drawmesh be at the orientation of our bike.
	//

	dx = bb->front_x - bb->back_x >> 8;
	dy = bb->front_y - bb->back_y >> 8;
	dz = bb->front_z - bb->back_z >> 8;

	dxdz = QDIST2(abs(dx),abs(dz));

	if (dxdz < 0x10)
	{
		//
		// Dont work out the yaw - we dont have enough info.
		//
	}
	else
	{
		p_bike->Genus.Bike->yaw  = calc_angle(dx,dz) + 1024;
		p_bike->Genus.Bike->yaw &= 2047;
	}

	p_bike->Genus.Bike->pitch = calc_angle(dy, dxdz);

	//
	// Position the bike at the average of the two wheels - but
	// nearer the back wheel!
	//

	GameCoord newpos;

	newpos.X = bb->front_x + bb->back_x + bb->back_x + bb->back_x >> 3;
	newpos.Y = bb->front_y + bb->back_y + bb->back_y + bb->back_y >> 3;
	newpos.Z = bb->front_z + bb->back_z + bb->back_z + bb->back_z >> 3;

	newpos.X = (bb->front_x + bb->back_x >> 1) - (bb->front_x - bb->back_x >> 2);
	newpos.Y = (bb->front_y + bb->back_y >> 1) - (bb->front_y - bb->back_y >> 2);
	newpos.Z = (bb->front_z + bb->back_z >> 1) - (bb->front_z - bb->back_z >> 2);

	move_thing_on_map(p_bike, &newpos);

	//
	// Hit the barrels!
	//

	BARREL_hit_with_sphere(
		newpos.X >> 8,
		newpos.Y >> 8,
		newpos.Z >> 8,
		0x80);

	/*

	THIS DOESNT WORK BECAUSE IT KNOCKS OUR BIKER OUT OF THE WAY!

	//
	// Knock a stationary person out of the way.
	//

	{
		SLONG col_person = THING_find_nearest(
								newpos.X >> 8,
								newpos.Y >> 8,
								newpos.Z >> 8,
								0x80,
								(1 << CLASS_PERSON));

		if (col_person)
		{
			slide_around_circle(
				newpos.X,
				newpos.Z,
				0x8000,
				TO_THING(col_person)->WorldPos.X,
				TO_THING(col_person)->WorldPos.Z,
			   &TO_THING(col_person)->WorldPos.X,
			   &TO_THING(col_person)->WorldPos.Z);
		}
	}

	*/

	//
	// Rotate the wheels.
	//

	{
		//
		// The wheels rotate depending on movement.
		//

		dx = abs(bb->front_x - front_old_x >> 8);
		dz = abs(bb->front_z - front_old_z >> 8);

		SLONG rspeed;
		
		rspeed  = QDIST2(dx,dz);
		rspeed += rspeed << 1;

		if (bb->accel < 0)
		{
			rspeed = -rspeed;
		}

		bb->wheel_rot_front += rspeed;
		bb->wheel_rot_back  += rspeed;

		if (rspeed)
		{
			
		}
	}

	//
	// The frame of animation of the bike.
	//

	switch(bb->mode)
	{
		case BIKE_MODE_PARKED:

			{
				DrawTween *dt = p_bike->Draw.Tweened;

				//
				// Should already be setup... we hope!
				//

				dt->Angle = bb->yaw;
				dt->Tilt  = bb->pitch;
			}

			break;

		case BIKE_MODE_MOUNTING:
		case BIKE_MODE_DISMOUNTING:

			{
				SLONG tween_step;

				DrawTween *dt = p_bike->Draw.Tweened;

				//
				// Animate the bike.
				//

				tween_step = dt->CurrentFrame->TweenStep << 1;
				tween_step = tween_step * TICK_RATIO >> TICK_SHIFT;

				if (tween_step <= 0)
				{
					tween_step = 1;
				}

				dt->AnimTween += tween_step;

				while (dt->AnimTween >= 256)
				{
					dt->AnimTween -= 256;

					if (dt->NextFrame)
					{
						dt->AnimTween = (dt->AnimTween * dt->NextFrame->TweenStep) / dt->CurrentFrame->TweenStep;
					}

					dt->FrameIndex++;

					if (dt->FrameIndex == 12)
					{
						MFX_play_thing(THING_NUMBER(p_bike),S_BIKE_IDLE,MFX_REPLACE|MFX_LOOPED|MFX_MOVING,p_bike);
					}

					SLONG advance_keyframe(DrawTween *draw_info);

					if (advance_keyframe(dt))
					{
						//
						// Anim over.
						//

						if (bb->mode == BIKE_MODE_MOUNTING)
						{
							bb->mode = BIKE_MODE_DRIVING;
						}
						else
						{
							bb->mode = BIKE_MODE_PARKED;
						}
					}
				}
			}

			break;

		case BIKE_MODE_DRIVING:

			{

				DrawTween *dt = p_bike->Draw.Tweened;

				dt->Angle        =  bb->yaw;
				dt->Tilt         = -bb->pitch & 2047;
				dt->Roll         =  BIKE_get_roll(p_bike);
				dt->AnimTween    =  0;
				dt->TweenStage   =  0;
				dt->QueuedFrame  =  0;
				dt->TheChunk     = &anim_chunk[12];
				dt->CurrentAnim  =  1;

				if (bb->steer == 0)
				{
					dt->CurrentFrame = anim_chunk[12].AnimList[1];
					dt->NextFrame    = anim_chunk[12].AnimList[1];
					dt->AnimTween    = 0;
				}
				else
				if (bb->steer < 0)
				{
					dt->CurrentFrame =  anim_chunk[12].AnimList[1];
					dt->NextFrame    =  anim_chunk[12].AnimList[4];
					dt->AnimTween    = -bb->steer << 3;
				}
				else
				{
					dt->CurrentFrame = anim_chunk[12].AnimList[1];
					dt->NextFrame    = anim_chunk[12].AnimList[6];
					dt->AnimTween    = bb->steer << 3;
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
	
	/*

	//
	// Debug line so we know where the wheels are.
	//

	AENG_world_line_infinite(
		bb->back_x >> 8,
		bb->back_y >> 8,
		bb->back_z >> 8,
		12,
		0xff0000,
		bb->front_x >> 8,
		bb->front_y >> 8,
		bb->front_z >> 8,
		8,
		0x44ff88,
		TRUE);

	AENG_world_line_infinite(
		bb->front_x >> 8,
		bb->front_y >> 8,
		bb->front_z >> 8,
		12,
		0x44ff88,
		bb->front_x + steer[0] >> 8,
		bb->front_y + steer[1] >> 8,
		bb->front_z + steer[2] >> 8,
		8,
		0xff00ff,
		TRUE);

	AENG_world_line(
		p_bike->WorldPos.X >> 8,
		p_bike->WorldPos.Y >> 8,
		p_bike->WorldPos.Z >> 8,
		16,
		0xffffff,
		(p_bike->WorldPos.X >> 8),
		(p_bike->WorldPos.Y >> 8) + 0x100,
		(p_bike->WorldPos.Z >> 8),
		0,
		0x5050f4,
		TRUE);

	*/
	if(go_again)
	{
		//
		// Process bikes that arent parked or mounting twice for extra speed
		//
		go_again=0;
		goto	again;
	}
}


#ifndef PSX

UWORD BIKE_create(
		SLONG x,
		SLONG z,
		SLONG yaw)
{
	Thing     *p_thing;
	DrawMesh  *dm;
	BIKE_Bike *bb;
	SLONG      vector[3];
	DrawTween *dt;

	//
	// We need to get a thing, bike and a drawmesh.
	//

	if (!WITHIN(BIKE_bike_upto, 1, BIKE_MAX_BIKES))
	{
		//
		// No bikes.
		//

		return NULL;
	}

	bb = &BIKE_bike[BIKE_bike_upto++];

	//
	// Get the thing.
	//

	p_thing = alloc_thing(CLASS_BIKE);

	if (p_thing)
	{
		dt = alloc_draw_tween(0);

		if (dt)
		{
			p_thing->Class        = CLASS_BIKE;
			p_thing->Flags        = 0;
			p_thing->State        = STATE_NORMAL;
			p_thing->StateFn      = BIKE_process_normal;
			p_thing->Genus.Bike   = bb;
			p_thing->DrawType     = DT_BIKE;
			p_thing->Draw.Tweened = dt;

			bb->mode  = BIKE_MODE_PARKED;
			bb->yaw   = yaw;
			bb->pitch = 0;
			bb->flag  = BIKE_FLAG_USED | BIKE_FLAG_ONGROUND_FRONT | BIKE_FLAG_ONGROUND_BACK;
			bb->accel = 0;
			bb->steer = 0;

			bb->back_x = x << 8;
			bb->back_z = z << 8;

			bb->back_y = PAP_calc_map_height_at(bb->back_x >> 8, bb->back_z >> 8) + BIKE_WHEEL_RADIUS + BIKE_WHEEL_SUS << 8;

			bb->back_dx = 0;
			bb->back_dy = 0;
			bb->back_dz = 0;

			p_thing->Index=9;
			dt->Angle        =  yaw;
			dt->Tilt         =  0;
			dt->Roll         =  0;
			dt->AnimTween    =  0;
			dt->TweenStage   =  0;
			dt->QueuedFrame  =  0;
			dt->TheChunk     = &anim_chunk[9];
			dt->CurrentAnim  =  1;
			dt->CurrentFrame =  anim_chunk[9].AnimList[1];
			dt->NextFrame    =  anim_chunk[9].AnimList[1];

			//
			// Put the front wheel in front of the back wheel.
			//

			FMATRIX_vector(vector, yaw, 0);

			bb->front_x = x + (vector[0] * BIKE_WHEEL_APART >> 16) << 8;
			bb->front_z = z + (vector[2] * BIKE_WHEEL_APART >> 16) << 8;

			bb->front_y = PAP_calc_map_height_at(bb->front_x >> 8, bb->front_z >> 8) + BIKE_WHEEL_RADIUS + BIKE_WHEEL_SUS  << 8;

			//
			// Position the bike between the two wheels.
			//

			p_thing->WorldPos.X = bb->back_x + bb->front_x >> 1;
			p_thing->WorldPos.Y = bb->back_y + bb->front_y >> 1;
			p_thing->WorldPos.Z = bb->back_z + bb->front_z >> 1;

			//
			// Put the bike on the mapwho.
			//

			add_thing_to_map(p_thing);

/*			if (NIGHT_flag & NIGHT_FLAG_DAYTIME) {
			  bb->ribbon=0;
			} else {
			  bb->ribbon=RIBBON_alloc(RIBBON_FLAG_FADE|RIBBON_FLAG_SLIDE|RIBBON_FLAG_IALPHA,16,POLY_PAGE_ADDITIVEALPHA,-1,8,5+(rand()&7),0,0,0x7F0000);
			}*/
#ifndef PSX
			 bb->ribbon=RIBBON_alloc(RIBBON_FLAG_FADE|RIBBON_FLAG_SLIDE|RIBBON_FLAG_IALPHA,10,POLY_PAGE_SMOKER,-1,8,5+(rand()&7),1, 0.2 ,0x6f6f6f);
			 bb->ribbon2=RIBBON_alloc(RIBBON_FLAG_FADE|RIBBON_FLAG_SLIDE|RIBBON_FLAG_IALPHA,10,POLY_PAGE_SMOKER,-1,8,5+(rand()&7),1, 0.2 ,0x6f6f6f);
#endif
			return THING_NUMBER(p_thing);
		}
		else
		{
			free_thing(p_thing);
		}
	}

	return NULL;
}

#endif

BIKE_Control BIKE_control_get(Thing *p_bike)
{
	ASSERT(p_bike->Class == CLASS_BIKE);

	BIKE_Control ans;

	ans.accel = p_bike->Genus.Bike->accel;
	ans.steer = p_bike->Genus.Bike->steer;

	return ans;
}

void BIKE_control_set(Thing *p_bike, BIKE_Control bc)
{
	SLONG speed  = BIKE_get_speed(p_bike);
	SLONG tspeed = 32 - abs(speed);

	SLONG	accel,steer;

	SATURATE(tspeed, 7, 25);

	ASSERT(p_bike->Class == CLASS_BIKE);
	
	if(bc.accel==0 && bc.brake)
	{

		bc.accel=-1;
	}
	accel=p_bike->Genus.Bike->accel + bc.accel*5; //was 20
	steer=p_bike->Genus.Bike->steer + bc.steer*tspeed;


	if(bc.wheelie)
	{
		SATURATE(accel,0,127)
	}
	else
	{
		SATURATE(accel,-40,50)
	}

	SATURATE(steer,-80,80)

	p_bike->Genus.Bike->accel=accel;
	p_bike->Genus.Bike->steer=steer;

	if(bc.accel<0 || bc.brake)
	{
		if(speed>3)
			p_bike->Genus.Bike->SlideTimer=1000; //half second if we process twice
	}

	if (bc.wheelie && p_bike->Genus.Bike->wheel_y_front < p_bike->Genus.Bike->wheel_y_back + 0x3000)
	{
		p_bike->Genus.Bike->wheel_dy_front -= BIKE_GRAVITY*4;
	}

}



SLONG BIKE_person_can_mount(Thing *p_person)
{
	SLONG ans = THING_find_nearest(
					p_person->WorldPos.X >> 8,
					p_person->WorldPos.Y >> 8,
					p_person->WorldPos.Z >> 8,
					0x100,
					1 << CLASS_BIKE);

	return ans;
}


BIKE_Drawinfo BIKE_get_drawinfo(Thing *p_bike)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	ASSERT(p_bike->Class == CLASS_BIKE);

	BIKE_Bike *bb = p_bike->Genus.Bike;

	dx = (bb->front_x - bb->back_x) * 7 >> 11;
	dy = (bb->front_y - bb->back_y) * 7 >> 11;
	dz = (bb->front_z - bb->back_z) * 7 >> 11;

	BIKE_Drawinfo ans;

	ans.yaw   =  p_bike->Genus.Bike->yaw;
	ans.pitch = -p_bike->Genus.Bike->pitch & 2047;
	ans.steer = (p_bike->Genus.Bike->yaw + (bb->steer << 3)) & 2047;

	//	ans.roll  =  BIKE_get_roll(p_bike);

	ans.roll = p_bike->Draw.Tweened->Roll;

	/*

	ans.front_x   = (p_bike->WorldPos.X >> 8) + dx;
	ans.front_y   = (p_bike->WorldPos.Y >> 8) + dy + (bb->wheel_y_front - bb->front_y >> 8) + (BIKE_WHEEL_SUS >> 1);
	ans.front_z   = (p_bike->WorldPos.Z >> 8) + dz;
	ans.front_rot = bb->wheel_rot_front;

	ans.back_x   = (bb->back_x       >> 8) + (dx >> 2);
	ans.back_y   = (bb->wheel_y_back >> 8) + (BIKE_WHEEL_SUS >> 1);
	ans.back_z   = (bb->back_z       >> 8) + (dz >> 2);
	ans.back_rot = (bb->wheel_rot_back);

	*/

	ans.front_x   = (bb->front_x       >> 8);
	ans.front_y   = (bb->wheel_y_front >> 8) + (BIKE_WHEEL_SUS >> 1);
	ans.front_z   = (bb->front_z       >> 8);
	ans.front_rot = bb->wheel_rot_front;

	ans.back_x   = (bb->back_x       >> 8);
	ans.back_y   = (bb->wheel_y_back >> 8) + (BIKE_WHEEL_SUS >> 1);
	ans.back_z   = (bb->back_z       >> 8);
	ans.back_rot = (bb->wheel_rot_back);


	return ans;
}


SLONG BIKE_get_roll(Thing *p_bike)
{
	ASSERT(p_bike->Class == CLASS_BIKE);

	SLONG roll;
//	SLONG	speed;

//	speed=BIKE_get_speed(p_bike);

//	speed=abs((speed>>2));


	roll  = -p_bike->Genus.Bike->steer <<2; //*speed;

	SATURATE(roll, -0x140, +0x140);

	roll &=  2047;

	return roll;
}

SLONG BIKE_get_speed(Thing *p_bike)
{
	ASSERT(p_bike->Class == CLASS_BIKE);

	SLONG dx;
	SLONG dz;
	SLONG speed;

	dx = abs(p_bike->Genus.Bike->back_dx) >> 8;
	dz = abs(p_bike->Genus.Bike->back_dz) >> 8;

	speed = QDIST2(dx,dz);

	return speed;
}


void BIKE_set_mounting(Thing *p_bike, Thing *p_person)
{
	p_bike->Genus.Bike->mode   = BIKE_MODE_MOUNTING;
	p_bike->Genus.Bike->driver = THING_NUMBER(p_person);

	//
	// Setup the drawtween ready for tweening.
	// 

	DrawTween *dt = p_bike->Draw.Tweened;

	dt->AnimTween    =  0;
	dt->TweenStage   =  0;
	dt->QueuedFrame  =  0;
	dt->TheChunk     = &anim_chunk[9];
	dt->CurrentAnim  =  2;
	dt->CurrentFrame =  anim_chunk[9].AnimList[2];
	dt->NextFrame    =  anim_chunk[9].AnimList[2]->NextFrame;
	dt->FrameIndex	 =  0;
}

void BIKE_set_parked(Thing *p_bike)
{
	DrawTween *dt = p_bike->Draw.Tweened;

	//
	// Put the bike in its parked state.
	//

	p_bike->Genus.Bike->mode   = BIKE_MODE_PARKED;
	p_bike->Genus.Bike->driver = NULL;

	dt->AnimTween    =  0;
	dt->TweenStage   =  0;
	dt->QueuedFrame  =  0;
	dt->TheChunk     = &anim_chunk[9];
	dt->CurrentAnim  =  1;
	dt->CurrentFrame =  anim_chunk[9].AnimList[1];
	dt->NextFrame    =  anim_chunk[9].AnimList[1];

	MFX_stop_attached(p_bike);

}

void BIKE_set_dismounting(Thing *p_bike)
{
	p_bike->Genus.Bike->mode = BIKE_MODE_DISMOUNTING;

	//
	// Setup the drawtween ready for tweening.
	// 

	DrawTween *dt = p_bike->Draw.Tweened;

	dt->AnimTween    =  0;
	dt->TweenStage   =  0;
	dt->QueuedFrame  =  0;
	dt->TheChunk     = &anim_chunk[9];
	dt->CurrentAnim  =  3;
	dt->CurrentFrame =  anim_chunk[9].AnimList[5];
	dt->NextFrame    =  anim_chunk[9].AnimList[5]->NextFrame;
	dt->FrameIndex	 =  0;
}


#endif
