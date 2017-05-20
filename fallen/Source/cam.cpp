//
// An 'intelligent' camera?
//

#ifdef	DOG_POO

#include "game.h"
#include "cam.h"
#include "fmatrix.h"
#include "animate.h"
#include "mav.h"
#include "sample.h"
#include "statedef.h"
#include "sound.h"
#include "ns.h"
#include "hook.h"
#include "eway.h"

#ifdef	PSX
#include "libgpu.h"
#endif

// remove float calcs from psx version
#ifndef	PSX
#include <math.h>
#endif
#include "mav.h"

//
// The camera.
//

SLONG  CAM_mode;
SLONG  CAM_zoom;
SLONG  CAM_lens;	// Not used by the camera module - merely a hint to the engine.
SLONG  CAM_pos_x;	// 16-bits per mapsquare
SLONG  CAM_pos_y;
SLONG  CAM_pos_z;
SLONG  CAM_vel_x;
SLONG  CAM_vel_y;
SLONG  CAM_vel_z;
SLONG  CAM_want_valid;
SLONG  CAM_want_different;
SLONG  CAM_want_dx;		// The camera desired offset from the focus thing.
SLONG  CAM_want_dy;
SLONG  CAM_want_dz;
SLONG  CAM_want_angle;	// The current angle used to calc (want_dx,want_dy,want_dz)
SLONG  CAM_yaw;
SLONG  CAM_pitch;
SLONG  CAM_roll;
float  CAM_radians_yaw;
float  CAM_radians_pitch;
float  CAM_radians_roll;
Thing *CAM_focus;
SLONG  CAM_focus_yaw;
SLONG  CAM_focus_x;
SLONG  CAM_focus_y;
SLONG  CAM_focus_z;
SLONG  CAM_collide;
SLONG  CAM_dyaw;	// For first person...
SLONG  CAM_dpitch;
SLONG  CAM_rotate;
SLONG  CAM_rotate_dist;
SLONG  CAM_shake;
SLONG  CAM_behind_up;
SLONG  CAM_type;

//
// When the camera looks at a thing, it looks at a point
// this much above its position.
//

#define CAM_LOOK_UP (0x6000)

//
// How high above the focus thing we look from.
//

#ifdef	PSX
#define CAM_BEHIND_UP (CAM_focus->Class == CLASS_VEHICLE ? 0x14000 : CAM_behind_up)
#else
#define CAM_BEHIND_UP (CAM_focus->Class == CLASS_VEHICLE ? 0x14000 : CAM_behind_up)
#endif

void CAM_set_focus    (Thing *focus)     
{
	CAM_focus     = focus;
	CAM_want_valid = FALSE;
	CAM_want_different = FALSE;
}
void CAM_set_mode     (SLONG  mode)      {CAM_mode      = mode;  CAM_want_valid = FALSE; CAM_want_different = FALSE;}
void CAM_set_zoom     (SLONG  zoom)      {CAM_zoom      = zoom;  CAM_want_valid = FALSE; CAM_want_different = FALSE;}
void CAM_set_behind_up(SLONG  behind_up) {CAM_behind_up = behind_up;}
void CAM_set_collision(SLONG  col)       {CAM_collide   = col;}
void CAM_set_pos(
		SLONG world_x, 
		SLONG world_y, 
		SLONG world_z)
{
	CAM_pos_x = world_x << 8;
	CAM_pos_y = world_y << 8;
	CAM_pos_z = world_z << 8;
}

void CAM_set_angle(
		SLONG yaw,
		SLONG pitch,
		SLONG roll)
{
	//if (CAM_mode == CAM_MODE_FIRST_PERSON ||
	//	CAM_mode == CAM_MODE_THIRD_PERSON)
	{
		CAM_yaw   = yaw;
		CAM_pitch = pitch;
		CAM_roll  = roll;

		CAM_radians_yaw   = float(yaw)   * 2.0F * PI / 2048.0F;
		CAM_radians_pitch = float(pitch) * 2.0F * PI / 2048.0F;
		CAM_radians_roll  = float(roll)  * 2.0F * PI / 2048.0F;
	}
}

void CAM_set_dangle(SLONG  dyaw, SLONG  dpitch)
{
	CAM_dpitch = dpitch;
	CAM_dyaw   = dyaw;
}
 
void CAM_get_dangle(SLONG *dyaw, SLONG *dpitch)
{
	*dyaw   = CAM_dyaw;
	*dpitch = CAM_dpitch;
}

float CAM_get_ryaw(void)
{
	float	yaw;

	yaw=CAM_yaw   * 2.0F * PI / 2048.0F;

	
	return(yaw);
}


void CAM_set_type(SLONG type)
{
	switch(type)
	{
		default:
		case CAM_TYPE_STANDARD:
			CAM_set_mode(CAM_MODE_NORMAL);
			CAM_set_zoom(0x600);
			CAM_set_behind_up(0x20000);
			CAM_lens = CAM_LENS_WIDEANGLE;
			CAM_set_collision(TRUE);
			break;

		case CAM_TYPE_LOWER:
			CAM_set_mode(CAM_MODE_NORMAL);
			CAM_set_zoom(0x420);
			CAM_set_behind_up(0x8000);
			CAM_lens = CAM_LENS_NORMAL;
			CAM_set_collision(TRUE);
			break;

		case CAM_TYPE_BEHIND:

			CAM_set_mode(CAM_MODE_BEHIND);
			CAM_set_zoom(0x340 * 180 / 256);
			CAM_set_behind_up(0x18000 * 180 / 256);
			CAM_lens = CAM_LENS_WIDEANGLE;
			CAM_set_collision(FALSE);

			/*
			CAM_set_mode(CAM_MODE_NORMAL);
			CAM_set_zoom(0x200);
			CAM_set_behind_up(0x50000);
			CAM_lens = CAM_LENS_WIDEANGLE;
			*/
			break;

		case CAM_TYPE_WIDE:
			CAM_set_mode(CAM_MODE_NORMAL);
			CAM_set_zoom(0x380);
			CAM_set_behind_up(0x38000);
			CAM_lens = CAM_LENS_WIDEANGLE;
			CAM_set_collision(TRUE);
			break;

		case CAM_TYPE_ZOOM:
			CAM_set_mode(CAM_MODE_NORMAL);
			CAM_set_zoom(0x520);
			CAM_set_behind_up(0x38000);
			CAM_lens = CAM_LENS_NORMAL;
			CAM_set_collision(TRUE);
			break;

//		default:
//			ASSERT(0);
//			break;
	}

	CAM_type = type;
}

SLONG CAM_get_type()
{
	return CAM_type;
}





void CAM_set_shake(UBYTE amount)
{
	CAM_shake = amount;
}

//
// Works out the yaw of our focus thing.
//

void CAM_work_out_focus_yaw(void)
{
	switch(CAM_focus->Class)
	{
		case CLASS_PERSON:

			if (CAM_focus->Flags & FLAG_PERSON_DRIVING)
			{
				CAM_focus_yaw = TO_THING(CAM_focus->Genus.Person->InCar)->Genus.Vehicle->Angle;
			}
			else
			{
				CAM_focus_yaw = CAM_focus->Draw.Tweened->Angle;
	 			if(CAM_focus->Genus.Person->Action==ACTION_SIDE_STEP)
				{
					CAM_focus_yaw+=1024;
					CAM_focus_yaw&=2047;
				}
			}

			break;

		case CLASS_VEHICLE:
			CAM_focus_yaw = CAM_focus->Genus.Vehicle->Angle;
			break;

		default:

			ASSERT(0);
			break;
	}
}

//
// Works out where we should at.
//

void CAM_work_out_focus_pos(void)
{
	void calc_sub_objects_position(
			Thing *p_mthing,
			SLONG  tween,
			UWORD  object,
			SLONG *x,
			SLONG *y,
			SLONG *z);

	switch(CAM_focus->Class)
	{
		case CLASS_PERSON:
/*
			if(CAM_focus->Draw.Tweened->PersonID==1)
			{
				//
				// got your gun out
				//

				if(CAM_focus->Genus.Person->Target)
				{
					Thing	*p_thing;
					p_thing=TO_THING(CAM_focus->Genus.Person->Target);

					CAM_focus_x=p_thing->WorldPos.X;
					CAM_focus_y=p_thing->WorldPos.Y;
					CAM_focus_z=p_thing->WorldPos.Z;
					CAM_focus_y += 0x1000;
				}

			}
			else
*/

			switch(CAM_focus->Genus.Person->Action)
			{
				case ACTION_DANGLING:
				case ACTION_PULL_UP:
				case ACTION_CLIMBING:
				case ACTION_STANDING_JUMP:
				case ACTION_DROP_DOWN:

					//
					// Focus just above pelvis
					//

					calc_sub_objects_position(
						CAM_focus,
						CAM_focus->Draw.Tweened->AnimTween,
						0,			 // 0 is pelvis
					   &CAM_focus_x,
					   &CAM_focus_y,
					   &CAM_focus_z);

					CAM_focus_x <<= 8;
					CAM_focus_y <<= 8;
					CAM_focus_z <<= 8;
	
					CAM_focus_x += CAM_focus->WorldPos.X;
					CAM_focus_y += CAM_focus->WorldPos.Y;
					CAM_focus_z += CAM_focus->WorldPos.Z;

					CAM_focus_y += 0x1000;

					break;

				case ACTION_HOP_BACK:
				case ACTION_SIDE_STEP:
				case ACTION_FLIP_LEFT:
				case ACTION_FLIP_RIGHT:

					//
					// Take (x,z) from the position of the pelvis.
					//

					calc_sub_objects_position(
						CAM_focus,
						CAM_focus->Draw.Tweened->AnimTween,
						0,			 // 0 is pelvis
					   &CAM_focus_x,
					   &CAM_focus_y,
					   &CAM_focus_z);

					CAM_focus_x <<= 8;
					CAM_focus_z <<= 8;

					CAM_focus_x += CAM_focus->WorldPos.X;
					CAM_focus_y  = CAM_focus->WorldPos.Y + CAM_LOOK_UP;
					CAM_focus_z += CAM_focus->WorldPos.Z;

					break;

				default:

					CAM_focus_x  = CAM_focus->WorldPos.X;
					CAM_focus_y  = CAM_focus->WorldPos.Y;
					CAM_focus_z  = CAM_focus->WorldPos.Z;

					CAM_focus_y += CAM_LOOK_UP;

					break;
			}

			break;

		default:

			CAM_focus_x  = CAM_focus->WorldPos.X;
			CAM_focus_y  = CAM_focus->WorldPos.Y;
			CAM_focus_z  = CAM_focus->WorldPos.Z;

			CAM_focus_y += CAM_LOOK_UP;

			break;
	}
}

//
// The distance of the camera from the focus.
//

SLONG CAM_dist_from_focus_xz(void)
{
	SLONG dx;
	SLONG dz;

	SLONG dist;

	dx = CAM_focus_x - CAM_pos_x;
	dz = CAM_focus_z - CAM_pos_z;

	dist = QDIST2(abs(dx), abs(dz)) >> 8;

	return dist;
}


//
// Sets the angle of the camera to look at the focus thing.
// If swoop then the camera gradually turns to look at the thing.
//

void CAM_look_at_thing(SLONG swoop)
{
	SLONG dpitch;
	SLONG want_pitch;

	float dradians_yaw;
	float dradians_pitch;

	float want_radians_yaw;
	float want_radians_pitch;

	SLONG dx = CAM_focus_x - CAM_pos_x >> 8;
	SLONG dy = CAM_focus_y - CAM_pos_y >> 8;
	SLONG dz = CAM_focus_z - CAM_pos_z >> 8;

	//
	// When Darci is throwing the grappling hook, look at
	// the grappling hook not at Darci,
	//

	static int hook_focus_x = 0;
	static int hook_focus_y = 0;
	static int hook_focus_z = 0;

	if(CAM_focus->Draw.Tweened->PersonID==1&&(CAM_focus->SubState==SUB_STATE_AIM_GUN||CAM_focus->SubState==SUB_STATE_SHOOT_GUN))
	{
		//
		// got your gun out
		//

		if(CAM_focus->Genus.Person->Target)
		{
			Thing	*p_thing;
			p_thing=TO_THING(CAM_focus->Genus.Person->Target);

			dx=p_thing->WorldPos.X-CAM_pos_x;
			dy=p_thing->WorldPos.Y-CAM_pos_y;
			dz=p_thing->WorldPos.Z-CAM_pos_z;
			dx>>=8;
			dy>>=8;
			dz>>=8;
		}
	}
	else
	{
#ifndef PSX
		SLONG state = HOOK_get_state();

		if (state == HOOK_STATE_SPINNING)
		{
			hook_focus_x = CAM_focus_x;
			hook_focus_y = CAM_focus_y;
			hook_focus_z = CAM_focus_z;
		}

		if (state == HOOK_STATE_FLYING)
		{
			//
			// Look at the hook as it flys through the air- until darci moves.
			//

			if (hook_focus_x == CAM_focus_x &&
				hook_focus_y == CAM_focus_y &&
				hook_focus_z == CAM_focus_z)
			{
				SLONG hook_x;
				SLONG hook_y;
				SLONG hook_z;
				SLONG hook_yaw;
				SLONG hook_pitch;
				SLONG hook_roll;

				HOOK_pos_grapple(
					&hook_x,
					&hook_y,
					&hook_z,
					&hook_yaw,
					&hook_pitch,
					&hook_roll);

				dx = hook_x - CAM_pos_x >> 8;
				dy = hook_y - CAM_pos_y >> 8;
				dz = hook_z - CAM_pos_z >> 8;
			}
		}
#endif
	}

	SLONG dxz = QDIST2(abs(dx),abs(dz));

	//
	// Look at the right part of the thing.
	//

	CAM_yaw     = Arctan(dx,dz); //was -ve
	want_pitch  = Arctan(dy,dxz);
	want_pitch &= 2047;
	CAM_roll    = 1024;

	if (swoop)
	{
		dpitch = want_pitch - CAM_pitch;

		if (dpitch >  1024) {dpitch -= 2048;}
		if (dpitch < -1024) {dpitch += 2048;}
		
		CAM_pitch += dpitch / 8;
	}
	else
	{
		CAM_pitch = want_pitch;
	}

	CAM_yaw   = (CAM_yaw   + 2048) & 2047;
	CAM_pitch = (CAM_pitch + 2048) & 2047;

	//
	// Radians are more accurate...
	//

	#ifndef	PSX
	want_radians_yaw   = atan2(dx,dz);
	want_radians_pitch = atan2(dy,dxz) + 0.1F;
	#else
	want_radians_yaw   = ((float) CAM_yaw)    * (2.0F * PI / 2048.0F);
	want_radians_pitch = ((float) want_pitch) * (2.0F * PI / 2048.0F);
	#endif

	CAM_radians_roll = 0;

	if (swoop)
	{
		dradians_yaw = want_radians_yaw - CAM_radians_yaw;

		if (dradians_yaw >  PI) {dradians_yaw -= 2.0F * PI;}
		if (dradians_yaw < -PI) {dradians_yaw += 2.0F * PI;}

		CAM_radians_yaw += dradians_yaw * 0.5F;

		dradians_pitch = want_radians_pitch - CAM_radians_pitch;

		if (dradians_pitch >  PI) {dradians_pitch -= 2.0F * PI;}
		if (dradians_pitch < -PI) {dradians_pitch += 2.0F * PI;}

		CAM_radians_pitch += dradians_pitch * 0.125F;
		CAM_radians_yaw    = want_radians_yaw;
	}
	else
	{
		CAM_radians_yaw   = want_radians_yaw;
		CAM_radians_pitch = want_radians_pitch;
	}
}

//
// Moves the camera towards the given destination. It slides along walls,
// accelerates and deccelerates...
//

void CAM_move_towards(
		SLONG dest_x,
		SLONG dest_y,
		SLONG dest_z)
{
	SLONG x1, x2;
	SLONG y1, y2;
	SLONG z1, z2;

	SLONG dx   = dest_x - CAM_pos_x;
	SLONG dy   = dest_y - CAM_pos_y;
	SLONG dz   = dest_z - CAM_pos_z;
	SLONG dist = QDIST3(abs(dx),abs(dy),abs(dz));

	#define CAM_MIN_DELTA (0x4000)

	if (dx > 0) {dx -= CAM_MIN_DELTA; if (dx < 0) {dx = 0;}}
	if (dx < 0) {dx += CAM_MIN_DELTA; if (dx > 0) {dx = 0;}}

	if (dz > 0) {dz -= CAM_MIN_DELTA; if (dz < 0) {dz = 0;}}
	if (dz < 0) {dz += CAM_MIN_DELTA; if (dz > 0) {dz = 0;}}


	CAM_vel_x += dx / 32;
	CAM_vel_y += dy / 32;
	CAM_vel_z += dz / 32;

	//
	// Damp the camera movement.
	//

	CAM_vel_x -= CAM_vel_x / 4;
	CAM_vel_y -= CAM_vel_y / 4;
	CAM_vel_z -= CAM_vel_z / 4;

	if (abs(CAM_vel_x) < 4) {CAM_vel_x = 0;}
	if (abs(CAM_vel_y) < 4) {CAM_vel_y = 0;}
	if (abs(CAM_vel_z) < 4) {CAM_vel_z = 0;}

	//
	// The movement vector.
	//

	x1 = CAM_pos_x;
	y1 = CAM_pos_y;
	z1 = CAM_pos_z;

	x2 = CAM_pos_x + CAM_vel_x;
	y2 = CAM_pos_y + CAM_vel_y;
	z2 = CAM_pos_z + CAM_vel_z;

	if (CAM_collide&&INDOORS_INDEX==0)
	{
		//
		// Move, but dont go through walls.
		//

		#define CAM_RADIUS 128

		slide_along(
			x1,  y1,  z1,
		   &x2, &y2, &z2,
			SLIDE_ALONG_DEFAULT_EXTRA_WALL_HEIGHT,
			CAM_RADIUS);
	}
	
	//
	// Move the camera.
	//

	CAM_pos_x = x2;
	CAM_pos_y = y2;
	CAM_pos_z = z2;
}


void CAM_process_crappy      (void);
void CAM_process_normal      (void);
void CAM_process_stationary  (void);
void CAM_process_behind      (void);
void CAM_process_first_person(void);
void CAM_process_third_person(void);

void CAM_process_crappy()
{
	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

//	if (Keys[KB_5]) {Keys[KB_5] = 0; CAM_want_valid = FALSE; CAM_want_different = TRUE;}

	if (!CAM_want_valid)
	{
		//
		// Work out a new relative position to look 
		// at the focus thing from.
		//
	
		SLONG i;

		SLONG best_score;
		SLONG best_angle;
		SLONG best_dx;
		SLONG best_dy;
		SLONG best_dz;

		SLONG dx;
		SLONG dy;
		SLONG dz;
		SLONG angle;
		SLONG score;
		SLONG dist;
		SLONG dist_score;
		SLONG dangle;
		SLONG dangle_score;

		SLONG fx = CAM_focus_x >> 8;
		SLONG fy = CAM_focus_y >> 8;
		SLONG fz = CAM_focus_z >> 8;

		SLONG x1;
		SLONG y1;
		SLONG z1;

		SLONG x2;
		SLONG y2;
		SLONG z2;

		best_score = -INFINITY;

		#define CAM_CHECK_ANGLES 4

		struct
		{
			SLONG los;
			SLONG dist;	// 8-bit proportion of CAM_zoom
			SLONG dx;
			SLONG dy;
			SLONG dz;

		} los_check[CAM_CHECK_ANGLES];

		for (angle = 0, i = 0; angle < 2048; angle += 2048 / CAM_CHECK_ANGLES, i++)
		{
			dx = SIN(angle) * CAM_zoom >> 8;
			dy = CAM_BEHIND_UP;
			dz = COS(angle) * CAM_zoom >> 8;

			//
			// Does this offset have a line of sight to the player?
			//

			x1 = fx;
			y1 = fy;
			z1 = fz;

			x2 = fx + (dx >> 8);
			y2 = fy + (dy >> 8);
			z2 = fz + (dz >> 8);
			   
			if (there_is_a_los(x1,y1,z1, x2,y2,z2)||INDOORS_INDEX)
			{
				//
				// Yes! That's good.
				// 

				los_check[i].los  = TRUE;
				los_check[i].dist = 256;
				los_check[i].dx   = dx;
				los_check[i].dy   = dy;
				los_check[i].dz   = dz;
			}
			else
			{
				//
				// So there isn't a los... How far did we get before we
				// hit a wall?
				//

				los_check[i].dx  = los_failure_x - x1 << 8;
				los_check[i].dy  = los_failure_y - y1 << 8;
				los_check[i].dz  = los_failure_z - z1 << 8;
				
				//
				// Score depending on how near that is to a wall.
				//

				los_check[i].dist = QDIST2(abs(los_check[i].dx),abs(los_check[i].dz)) / CAM_zoom;

				SATURATE(los_check[i].dist, 0, 256);

				if (los_check[i].dist > 128)
				{
					//
					// Let this count as a los.
					//

					los_check[i].los = TRUE;
				}
				else
				{
					los_check[i].los = FALSE;
				}
			}
		}

		for (angle = 0, i = 0; angle < 2048; angle += 2048 / CAM_CHECK_ANGLES, i++)
		{
			dx = los_check[i].dx;
			dy = los_check[i].dy;
			dz = los_check[i].dz;

			//
			// Score this offset.
			//

			score = 0;

			//
			// Does this offset have a line of sight to the player?
			//

			if (los_check[i].los)
			{
				score += 0x200;

				//
				// Is there a good los?
				//

				if (los_check[(i + 1) & (CAM_CHECK_ANGLES - 1)].los) {score += 0x100;}
				if (los_check[(i - 1) & (CAM_CHECK_ANGLES - 1)].los) {score += 0x100;}
			}
			else
			{
				dist_score = (dist << 8) / CAM_zoom;

				SATURATE(dist_score, 0, 256);

				dist_score  -= 128;
				dist_score <<= 1;

				score += dist_score;
			}

			//
			// Can the camera move there okay?
			//

			x1 = CAM_pos_x >> 8;
			y1 = CAM_pos_y >> 8;
			z1 = CAM_pos_z >> 8;

			x2 = CAM_pos_x + dx >> 8;
			y2 = CAM_pos_y + dy >> 8;
			z2 = CAM_pos_z + dz >> 8;

			if (there_is_a_los(x1,y1,z1, x2,y2,z2)||INDOORS_INDEX)
			{
				score += 0x200;
			}

			//
			// Dont move too far.
			//

			dist =
				abs(x2 - x1) +
				abs(y2 - y1) +
				abs(z2 - z1);

			dist_score = -dist >> 5;
			
			score += dist_score;

			//
			// Is this the same angle we are already at?
			// 

			if (angle == CAM_want_angle)
			{
				if (CAM_want_different)
				{
					score = -100000;
				}
				else
				{
					score += 0x100;
				}
			}

			//
			// We like looking in the same direction as our focus thing.
			//

			dangle = CAM_focus_yaw - angle;

			if (dangle >  1024) {dangle -= 2048;}
			if (dangle < -1024) {dangle += 2048;}

			dangle_score = 0x100 - abs(dangle << 1);

			if (dangle_score > 0)
			{
				score += dangle_score;

				//
				// If there is los, then this is a really good direction!
				//

				if (los_check[i].los)
				{
					score += dangle_score << 1;
				}
			}

			#if ISNT_THIS_THE_SAME_AS_MOVING_TOO_FAR

			//
			// We dont like going through large angles.
			//

			dangle = angle - CAM_want_angle;

			if (dangle >  1024) {dangle -= 2048;}
			if (dangle < -1024) {dangle += 2048;}

			if (abs(dangle) > 512)
			{
				score -= (abs(dangle) - 512) >> 2;
			}

			#endif

			//
			// Remember the best.
			//

			if (score > best_score)
			{
				best_dx    = dx;
				best_dy    = dy;
				best_dz    = dz;
				best_score = score;
				best_angle = angle;
			}
		}

		//
		// The new relative offset we want.
		//

		CAM_want_dx    = best_dx;
		CAM_want_dy    = best_dy;
		CAM_want_dz    = best_dz;
		CAM_want_angle = best_angle;
		CAM_want_valid = TRUE;
	}

	static SLONG focus_x = 0;
	static SLONG focus_z = 0;

	if (focus_x != CAM_focus->WorldPos.X ||
		focus_z != CAM_focus->WorldPos.Z)
	{
		focus_x  = CAM_focus->WorldPos.X;
		focus_z  = CAM_focus->WorldPos.Z;

		//
		// Always recalculate the angle when our focus is moving.
		//

		CAM_want_valid     = FALSE;
		CAM_want_different = FALSE;
	}

	//
	// Absolute position we want to be.
	//	

	dest_x  = CAM_focus_x;
	dest_y  = CAM_focus_y;
	dest_z  = CAM_focus_z;

	dest_x += CAM_want_dx;
	dest_y += CAM_want_dy;
	dest_z += CAM_want_dz;

	CAM_move_towards(
		dest_x,
		dest_y,
		dest_z);

	//
	// Point at our focus thing.
	//

	CAM_look_at_thing(TRUE);

}

//
// The game-state sensitive los function used by the camera.
//

SLONG CAM_los_fail_x;
SLONG CAM_los_fail_y;
SLONG CAM_los_fail_z;

SLONG CAM_los(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2)
{
	SLONG ans;

	if(INDOORS_INDEX)
	{
		ans=1;
	}
#if !defined(TARGET_DC) && !defined(TARGET_DC)
	else
	if (GAME_FLAGS & GF_SEWERS)
	{
		ans = NS_there_is_a_los(
				x1, y1, z1,
				x2, y2, z2);

		CAM_los_fail_x = NS_los_fail_x;
		CAM_los_fail_y = NS_los_fail_y;
		CAM_los_fail_z = NS_los_fail_z;
	}
#endif
	else
	{
		ans = there_is_a_los(
				x1, y1, z1,
				x2, y2, z2);

		CAM_los_fail_x = los_failure_x;
		CAM_los_fail_y = los_failure_y;
		CAM_los_fail_z = los_failure_z;
	}

	return ans;
}


void CAM_process_normal()
{
	SLONG x;
	SLONG y;
	SLONG z;

	SLONG x1, x2, x3;
	SLONG y1, y2, y3;
	SLONG z1, z2, z3;

	SLONG old_x2;
	SLONG old_y2;
	SLONG old_z2;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG rx;
	SLONG rz;

	SLONG mdx;
	SLONG mdy;
	SLONG mdz;

	SLONG dist;
	SLONG ddist;

	SLONG lookx;
	SLONG lookz;

	SLONG xforce;
	SLONG yforce;
	SLONG zforce;
	
	SLONG want_height;
	SLONG want_dist_min;
	SLONG want_dist_max;
	
	SLONG behind;
	SLONG hit;
	
	SLONG focus_moving;

	static SLONG focus_x = 0;
	static SLONG focus_z = 0;

	if (focus_x != CAM_focus_x ||
		focus_z != CAM_focus_z)
	{
		focus_x  = CAM_focus_x;
		focus_z  = CAM_focus_z;

		focus_moving = TRUE;

	}
	else
	{
	 	if(CAM_focus->Genus.Person->Action==ACTION_SIDE_STEP)
			focus_moving = TRUE;
		else
			focus_moving = FALSE;
	}

	if (CAM_focus->Class                == CLASS_PERSON &&
		CAM_focus->Genus.Person->Action == ACTION_CLIMBING)
	{
		//
		// Stay put and dramatically look up at the player.
		//
	}
	else
	{
		//
		// The vector the focus thing is looking down.
		//

		lookx = -SIN(CAM_focus_yaw) >> 8;
		lookz = -COS(CAM_focus_yaw) >> 8;

		//
		// This is the height we want to be at.
		//

		want_height = CAM_focus->WorldPos.Y + CAM_LOOK_UP + CAM_BEHIND_UP;

		//
		// Make sure our vertical position is always good.
		//

		dy = want_height - CAM_pos_y;

		CAM_vel_y += dy / 128;

		//
		// How far are we from the focus thing in (x,z)?
		//

		dx = CAM_focus_x - CAM_pos_x;
		dz = CAM_focus_z - CAM_pos_z;

		dist  = QDIST2(abs(dx), abs(dz)) >> 8;

		if (dist < 0x80)
		{
			dist = 0x80;
		}

		//
		// If the camera is far away from where it wants to be in 'y', then move
		// closer towards the focus thing. This will happen if the character jumps
		// of a high buidling for instance. The camera will want to go into more
		// of a plan view.
		//

		if (abs(dy) < 0x10000)
		{
			want_dist_min = CAM_zoom - 0x80;
			want_dist_max = CAM_zoom;

			if (CAM_focus->Class    == CLASS_PERSON &&
				CAM_focus->SubState == SUB_STATE_GRAPPLE)
			{
				want_dist_min = 1500;	// ?!
			}
		}
		else
		{
			SLONG move_in;

			//
			// How much closer do we move into the character?
			// 
			//

			move_in   = abs(dy) - 0x10000;
			move_in >>= 9;
			move_in   = 256 - move_in;

			if (move_in < 64)
			{
				move_in = 64;
			}

			want_dist_min = (CAM_zoom - 0x80) * move_in >> 8;
			want_dist_max = (CAM_zoom       ) * move_in >> 8;
		}

		/*
		if (!focus_moving)
		{
			//
			// Don't get too near a stationary focus thing.
			//

			want_dist_min += 0x180;
			want_dist_max += 0x180;
		}
		*/

		//
		// Dont be too far from the focus thing.
		//

		ddist  = dist - want_dist_max;
		
		if (ddist > 0)
		{
			//
			// We must accelerate towards our focus thing.
			//
			
			mdx = (dx / dist) * ddist;
			mdz = (dz / dist) * ddist;

			CAM_vel_x += mdx / 32;
			CAM_vel_z += mdz / 32;

			CAM_vel_x += mdx / 64;
			CAM_vel_z += mdz / 64;

			//CAM_vel_x += mdx / 128;
			//CAM_vel_z += mdz / 128;

			//CAM_vel_x += mdx / 256;
			//CAM_vel_z += mdz / 256;
		}

		//
		// Dont be too close to the focus thing.
		//

		ddist = dist - want_dist_min;

		if (ddist < 0)
		{
			//
			// We must accelerate away from our focus thing.
			//
			
			mdx = (dx / dist) * ddist;
			mdy = (dy / dist) * ddist;
			mdz = (dz / dist) * ddist;

			CAM_vel_x += mdx / 32;
			CAM_vel_y += mdy / 32;
			CAM_vel_z += mdz / 32;

			CAM_vel_x += mdx / 64;
			CAM_vel_y += mdy / 64;
			CAM_vel_z += mdz / 64;
		}

		//
		// Is there anything behind the focus thing?
		//

		x1 = CAM_focus_x >> 8;
		y1 = CAM_focus_y >> 8;
		z1 = CAM_focus_z >> 8;

		x2 = x1 - (lookx << 1);
		y2 = y1;
		z2 = z1 - (lookz << 1);

		if ((!CAM_collide)||INDOORS_INDEX)
		{
			behind = FALSE;
		}
		else
		{
			behind = !CAM_los(
						x1,y1,z1,
						x2,y2,z2);
		}

		if (!behind)
		{
			//
			// The focus thing is not standing with her back to a wall.
			// Rotate around the focus thing to a point behind it.
			//

			//
			// Normalise dx,dz to 256.
			//

			mdx = dx / dist;
			mdz = dz / dist;

			//
			// Would the camera collide with something?
			//

			x2 = x1 = CAM_pos_x >> 8;
			y2 = y1 = CAM_pos_y >> 8;
			z2 = z1 = CAM_pos_z >> 8;

			x2 += mdz;
			z2 -= mdx;

			if ((!CAM_collide) || INDOORS_INDEX)
			{
				hit = FALSE;
			}
			else
			{
				hit = !CAM_los(
						x1,y1,z1,
						x2,y2,z2);
			}

			if (!hit)
			{
				SLONG cprod = mdx*lookz - mdz*lookx >> 8;

				cprod *= 3;	// WAS 2!

				if (Keys[KB_END])
				{
					cprod *= 16;
				}

				if (focus_moving)
				{
					//
					// Rotate faster if the focus is moving.
					//

					cprod *= 4;

					if (CAM_collide && INDOORS_INDEX==0)
					{
						//
						// Rotate even faster if we cant see where our focus thing is going.
						//

						x1 = CAM_focus_x >> 8;
						y1 = CAM_focus_y >> 8;
						z1 = CAM_focus_z >> 8;

						x2 = x1 + lookx;
						y2 = y1;
						z2 = z1 + lookz;

						if (!CAM_los(
								x1,y1,z1,
								x2,y2,z2))
						{
							x2 = CAM_los_fail_x;
							y2 = CAM_los_fail_y;
							z2 = CAM_los_fail_z;
						}

						x1 = CAM_pos_x >> 8;
						y1 = CAM_pos_y >> 8;
						z1 = CAM_pos_z >> 8;

						if (!CAM_los(
								x1,y1,z1,
								x2,y2,z2))
						{
							cprod *= 4;
						}
					}
				}

				CAM_vel_x += mdz * cprod >> 8;
				CAM_vel_z -= mdx * cprod >> 8;

				CAM_vel_x += mdx * cprod >> 11;
				CAM_vel_z += mdz * cprod >> 11;
			}
		}

		if (CAM_collide && INDOORS_INDEX==0)
		{
			//
			// Repel yourself from buildings using the MAV_height array.
			//
			
			x = CAM_pos_x >> 8;
			y = CAM_pos_y >> 8;
			z = CAM_pos_z >> 8;

			xforce = 0;
			yforce = 0;
			zforce = 0;

			SLONG ax;
			SLONG ay;
			SLONG az;

			ax = CAM_focus_x - CAM_pos_x >> 11;
			ay = CAM_focus_y - CAM_pos_y >> 11;
			az = CAM_focus_z - CAM_pos_z >> 11;
#ifndef PSX
			if (GAME_FLAGS & GF_SEWERS)
			{
				for (SLONG i = 0; i < 8; i++)
				{
					if (NS_inside(x, y, z))
					{
						//
						// Do nowt...
						//
					}
					else
					{
						if (NS_inside(x - 0x100, y, z)) {xforce += 0x100 - (x & 0xff);}
						if (NS_inside(x + 0x100, y, z)) {xforce -=         (x & 0xff);}

						if (NS_inside(x, y - 0x100, z)) {yforce += 0x100 - (y & 0xff);}
						if (NS_inside(x, y + 0x100, z)) {yforce -=         (y & 0xff);}

						if (NS_inside(x, y, z - 0x100)) {zforce += 0x100 - (z & 0xff);}
						if (NS_inside(x, y, z + 0x100)) {zforce -=         (z & 0xff);}
					}

					x += ax;
					y += ay;
					z += az;
				}
			}
			else
#endif
			{
				for (SLONG i = 0; i < 8; i++)
				{
					if (MAV_inside(x, y, z))
					{
						//
						// Do nowt...
						//
					}
					else
					{
						if (MAV_inside(x - 0x100, y, z)) {xforce += 0x100 - (x & 0xff);}
						if (MAV_inside(x + 0x100, y, z)) {xforce -=         (x & 0xff);}

						if (MAV_inside(x, y - 0x100, z)) {yforce += 0x100 - (y & 0xff);}
						if (MAV_inside(x, y + 0x100, z)) {yforce -=         (y & 0xff);}

						if (MAV_inside(x, y, z - 0x100)) {zforce += 0x100 - (z & 0xff);}
						if (MAV_inside(x, y, z + 0x100)) {zforce -=         (z & 0xff);}
					}

					x += ax;
					y += ay;
					z += az;
				}
			}

			CAM_vel_x += xforce << 1;
			CAM_vel_y += yforce << 1;
			CAM_vel_z += zforce << 1;

			CAM_vel_x += xforce;
//			CAM_vel_y += yforce;
			CAM_vel_z += zforce;
		}
	}

	//
	// Rotation about the focus thing.
	//

	if (CAM_rotate)
	{
		dx = CAM_focus_x - CAM_pos_x;
		dz = CAM_focus_z - CAM_pos_z;

		dist   = QDIST2(abs(dx),abs(dz));
		dist >>= 8;

		if (dist == 0) {dist = 1;}

		dx /= dist;
		dz /= dist;

		rx = -dz * SIGN(CAM_rotate);
		rz =  dx * SIGN(CAM_rotate);

		x1 = CAM_pos_x >> 8;
		y1 = CAM_pos_y >> 8;
		z1 = CAM_pos_z >> 8;

		x2 = x1 + (rx << 0);
		y2 = y1;
		z2 = z1 + (rz << 0);

		x3  = CAM_focus_x >> 8;
		y3  = CAM_focus_y >> 8;
		z3  = CAM_focus_z >> 8;

		y3 += 0x80;

		if ((CAM_los(x1,y1,z1, x2,y2,z2) &&
			 CAM_los(x3,y3,z3, x2,y2,z2)))
		{
			SLONG dx;
			SLONG dz;
			SLONG ddist;
			SLONG rot_dist;

			CAM_pos_x += rx * abs(CAM_rotate) << 1;
			CAM_pos_z += rz * abs(CAM_rotate) << 1;

			dx    = CAM_focus_x - CAM_pos_x >> 8;
			dz    = CAM_focus_z - CAM_pos_z >> 8;
			dist  = sqrt(dx*dx + dz*dz);
			ddist = dist - CAM_rotate_dist;

			CAM_pos_x += ((dx * ddist) / dist) << 8;
			CAM_pos_z += ((dz * ddist) / dist) << 8;
		}
		else
		{
			CAM_rotate = 0;
			
			play_quicker_wave(S_PISTOL_DRY);
		}

		CAM_rotate -= CAM_rotate / 8;

		if (abs(CAM_rotate) < 8)
		{
			CAM_rotate = 0;
		}
	}

	//
	// Damp the camera movement.
	//

	SLONG damp_mul;

	damp_mul = 64 * TICK_RATIO >> TICK_SHIFT;

	//
	// Try and keep the damping to about 20...
	// 

	damp_mul -= (damp_mul - 32) * 200 >> 8;

	if (damp_mul > 50)
	{
		//
		// Not more than 50!
		// 

		damp_mul -= (damp_mul - 50) * 200 >> 8;
	}

	if (abs(CAM_vel_x) < 8) {CAM_vel_x = 0;}
	if (abs(CAM_vel_y) < 8) {CAM_vel_y = 0;}
	if (abs(CAM_vel_z) < 8) {CAM_vel_z = 0;}

	CAM_vel_x -= CAM_vel_x * damp_mul >> 8;
	CAM_vel_y -= CAM_vel_y * damp_mul >> 8;
	CAM_vel_z -= CAM_vel_z * damp_mul >> 8;

	//
	// The movement vector.
	//

	x1 = CAM_pos_x;
	y1 = CAM_pos_y;
	z1 = CAM_pos_z;

	x2 = CAM_pos_x + (CAM_vel_x * TICK_RATIO >> TICK_SHIFT);
	y2 = CAM_pos_y + (CAM_vel_y * TICK_RATIO >> TICK_SHIFT);
	z2 = CAM_pos_z + (CAM_vel_z * TICK_RATIO >> TICK_SHIFT);

	old_x2 = x2;
	old_y2 = y2;
	old_z2 = z2;

	//
	// Move the camera.
	//

	CAM_pos_x = x2;
	CAM_pos_y = y2;
	CAM_pos_z = z2;

	//
	// Point at our focus thing.
	//

	CAM_look_at_thing(TRUE);
}



void CAM_process_stationary()
{
	//
	// Just point at our focus thing.
	//

	CAM_look_at_thing(TRUE);
}

void CAM_process_behind()
{
	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	SLONG along;

	//
	// We want to be at a point behind our focus thing.
	//

	CAM_want_dx = SIN(CAM_focus_yaw) * CAM_zoom >> 8;
	CAM_want_dz = COS(CAM_focus_yaw) * CAM_zoom >> 8;

	CAM_want_dy = CAM_BEHIND_UP;

	//
	// Absolute position we want to be.
	//	

	dest_x  = CAM_focus_x;
	dest_y  = CAM_focus_y;
	dest_z  = CAM_focus_z;

	dest_x += CAM_want_dx;
	dest_y += CAM_want_dy;
	dest_z += CAM_want_dz;

	CAM_move_towards(
		dest_x,
		dest_y,
		dest_z);

	//
	// Point at our focus thing.
	//

	CAM_look_at_thing(TRUE);
}

void CAM_process_shoot_normal()
{
	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	SLONG along;

	//
	// We want to be at a point behind our focus thing.
	//

	CAM_want_dx = SIN(CAM_focus_yaw) * CAM_zoom >> 8;
	CAM_want_dz = COS(CAM_focus_yaw) * CAM_zoom >> 8;

	CAM_want_dy = CAM_BEHIND_UP>>2;

	//
	// Absolute position we want to be.
	//	

	dest_x  = CAM_focus_x;
	dest_y  = CAM_focus_y;
	dest_z  = CAM_focus_z;

	dest_x += CAM_want_dx;
	dest_y += CAM_want_dy;
	dest_z += CAM_want_dz;

	CAM_move_towards(
		dest_x,
		dest_y,
		dest_z);

	//
	// shooting camera twice as responsive
	//
	CAM_move_towards(
		dest_x,
		dest_y,
		dest_z);

	//
	// Point at our focus thing.
	//

	CAM_look_at_thing(TRUE);
}

void CAM_process_fight_normal()
{
	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	SLONG along;

	//
	// We want to be at a point behind our focus thing.
	//

	CAM_want_dx = SIN((CAM_focus_yaw-512)&2047) * CAM_zoom >> 8;
	CAM_want_dz = COS((CAM_focus_yaw-512)&2047) * CAM_zoom >> 8;

	CAM_want_dy = CAM_BEHIND_UP>>2;

	//
	// Absolute position we want to be.
	//	

	dest_x  = CAM_focus_x;
	dest_y  = CAM_focus_y;
	dest_z  = CAM_focus_z;

	dest_x += CAM_want_dx;
	dest_y += CAM_want_dy;
	dest_z += CAM_want_dz;

	CAM_move_towards(
		dest_x,
		dest_y,
		dest_z);

	//
	// shooting camera twice as responsive
	//
/*
	CAM_move_towards(
		dest_x,
		dest_y,
		dest_z);
*/

	//
	// Point at our focus thing.
	//

	CAM_look_at_thing(TRUE);
}

SLONG CAM_div_move = 8;
SLONG CAM_div_turn = 4;

void CAM_process_first_person()
{
	SLONG px;
	SLONG py;
	SLONG pz;

	if (CAM_focus->Class == CLASS_PERSON)
	{
		calc_sub_objects_position(
			CAM_focus,
			CAM_focus->Draw.Tweened->AnimTween,
			11,
		   &px,
		   &py,
		   &pz);

		px <<= 8;
		py <<= 8;
		pz <<= 8;

		px += CAM_focus->WorldPos.X;
		py += CAM_focus->WorldPos.Y;
		pz += CAM_focus->WorldPos.Z;
	}
	else
	{
		px = CAM_focus_x;
		py = CAM_focus_y + (CAM_LOOK_UP + 0x30);
		pz = CAM_focus_z;
	}

	SLONG want_x = px;
	SLONG want_y = py;
	SLONG want_z = pz;

	SLONG want_yaw   = (CAM_dyaw + CAM_focus_yaw + 1024) & 2047;
	SLONG want_pitch = (CAM_dpitch)                      & 2047;

	SLONG dx = want_x - CAM_pos_x;
	SLONG dy = want_y - CAM_pos_y;
	SLONG dz = want_z - CAM_pos_z;

	CAM_pos_x += dx / CAM_div_move;
	CAM_pos_y += dy / CAM_div_move;
	CAM_pos_z += dz / CAM_div_move;

	SLONG dist = abs(dx) + abs(dy) + abs(dz);

	if (dist < 0x4000)
	{
		SLONG dyaw   = want_yaw   - CAM_yaw;
		SLONG dpitch = want_pitch - CAM_pitch;

		if (dyaw >  1024) {dyaw -= 2048;}
		if (dyaw < -1024) {dyaw += 2048;}

		if (dpitch >  1024) {dpitch -= 2048;}
		if (dpitch < -1024) {dpitch += 2048;}

		CAM_yaw   += dyaw   / CAM_div_turn;
		CAM_pitch += dpitch / CAM_div_turn;
		CAM_roll   = 0;

		//
		// In radians now...
		//

		CAM_radians_yaw   = float(CAM_yaw)   * (2.0F * PI / 2048.0F);
		CAM_radians_pitch = float(CAM_pitch) * (2.0F * PI / 2048.0F);
		CAM_radians_roll  = 0.0F;
	}
	else
	if (dist < 0x10000)
	{
		CAM_pos_x = want_x;
		CAM_pos_y = want_y;
		CAM_pos_z = want_z;

		CAM_yaw   = want_yaw;
		CAM_pitch = want_pitch;

		//
		// In radians now...
		//

		CAM_radians_yaw   = float(CAM_yaw)   * (2.0F * PI / 2048.0F);
		CAM_radians_pitch = float(CAM_pitch) * (2.0F * PI / 2048.0F);
		CAM_radians_roll  = 0.0F;
	}
	else
	{
		CAM_focus_x = want_x;
		CAM_focus_y = want_y;
		CAM_focus_z = want_z;

		CAM_look_at_thing(TRUE);
	}
}

void CAM_process_third_person()
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG matrix[9];

	//
	// Position yourself to look at the focus thing given
	// the current camera angles and zoom.
	//

	FMATRIX_calc(
		matrix,
		CAM_yaw,
		CAM_pitch,
		CAM_roll);

	dx = matrix[6] * CAM_zoom >> 8;
	dy = matrix[7] * CAM_zoom >> 8;
	dz = matrix[8] * CAM_zoom >> 8;

	CAM_pos_x  = CAM_focus_x - dx;
	CAM_pos_y  = CAM_focus_y - dy;
	CAM_pos_z  = CAM_focus_z - dz;
}


void CAM_process_topper()
{
	//
	// Put the camera above our focus thing.
	//


}



void CAM_process()
{
	CAM_work_out_focus_yaw();
	CAM_work_out_focus_pos();

	//
	// Is the camera too far from the focus thing?
	//

	SLONG dx = CAM_focus_x - CAM_pos_x;
	SLONG dy = CAM_focus_y - CAM_pos_y;
	SLONG dz = CAM_focus_z - CAM_pos_z;

	SLONG dist = QDIST3(abs(dx),abs(dy),abs(dz));

	if (dist > 0x10000 * 32)
	{
		//
		// Jump to near the focus thing.
		//

		CAM_pos_x = CAM_focus_x + 0x10000;
		CAM_pos_y = CAM_focus_y + 0x30000;
		CAM_pos_z = CAM_focus_z + 0x10000;
	}

	if (CAM_focus->Class == CLASS_VEHICLE)
	{
		CAM_process_behind();
	}
	else
	{
		switch(CAM_mode)
		{
			case CAM_MODE_CRAPPY:
				CAM_process_crappy();
				break;

			case CAM_MODE_NORMAL:
				CAM_process_normal();
				break;

			case CAM_MODE_STATIONARY:
				CAM_process_stationary();
				break;

			case CAM_MODE_SHOOT_NORMAL:
				CAM_process_shoot_normal();
				break;

			case CAM_MODE_FIGHT_NORMAL:
				CAM_process_fight_normal();
				break;

			case CAM_MODE_BEHIND:
				CAM_process_behind();
				break;

			case CAM_MODE_FIRST_PERSON:
				CAM_process_first_person();
				break;

			case CAM_MODE_THIRD_PERSON:
				CAM_process_third_person();
				break;
/*
			case CAM_MODE_TOPPER:
				CAM_process_topper();
				break;
*/

			default:
				ASSERT(0);
				break;
		}
	}
}


void CAM_rotate_left()
{
	if (CAM_rotate == 0)
	{
		//
		// Remember the camera's distance from the focus thing. Try to keep
		// this distance when we turn.
		//

		CAM_rotate_dist = CAM_dist_from_focus_xz();
	}

	if (CAM_rotate > -32)
	{
		CAM_rotate -= 128;
	}
}

void CAM_rotate_right()
{
	if (CAM_rotate == 0)
	{
		//
		// Remember the camera's distance from the focus thing. Try to keep
		// this distance when we turn.
		//

		CAM_rotate_dist = CAM_dist_from_focus_xz();
	}

	if (CAM_rotate < 32)
	{
		CAM_rotate += 128;
	}
}


SLONG CAM_get_mode() {return CAM_mode;}
SLONG CAM_get_zoom() {return CAM_zoom;}

void CAM_get(
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll,
		float *radians_yaw,
		float *radians_pitch,
		float *radians_roll)
{
	SLONG lens;

	if (EWAY_grab_camera(
			world_x,
			world_y,
			world_z,
			yaw,
			pitch,
			roll,
		   &lens))
	{
	   *world_x >>= 8;
	   *world_y >>= 8;
	   *world_z >>= 8;

	   *radians_yaw   = float(*yaw)   * (2.0F * PI / 2048.0F);
	   *radians_pitch = float(*pitch) * (2.0F * PI / 2048.0F);
	   *radians_roll  = float(*roll)  * (2.0F * PI / 2048.0F);
	
		return;
	}

	*world_x       = CAM_pos_x >> 8;
	*world_y       = CAM_pos_y >> 8;
	*world_z       = CAM_pos_z >> 8;
	*yaw           = CAM_yaw;
	*pitch         = CAM_pitch;
	*roll          = CAM_roll;
	*radians_yaw   = CAM_radians_yaw;
	*radians_pitch = CAM_radians_pitch;
	*radians_roll  = CAM_radians_roll;

	if (CAM_shake)
	{
		*world_x += rand() % CAM_shake;
		*world_y += rand() % CAM_shake;
		*world_z += rand() % CAM_shake;

		*world_x -= CAM_shake >> 1;
		*world_y -= CAM_shake >> 1;
		*world_z -= CAM_shake >> 1;
	}
}

void CAM_get_psx(
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll,
		SLONG *radians_yaw,
		SLONG *radians_pitch,
		SLONG *radians_roll)
{
	*world_x       = CAM_pos_x >> 8;
	*world_y       = CAM_pos_y >> 8;
	*world_z       = CAM_pos_z >> 8;
	*yaw           = CAM_yaw;
	*pitch         = CAM_pitch;
	*roll          = CAM_roll;
	*radians_yaw   = (SLONG)(CAM_radians_yaw*65536);
	*radians_pitch = (SLONG)(CAM_radians_pitch*65536);
	*radians_roll  = (SLONG)(CAM_radians_roll*65536);

	if (CAM_shake)
	{
		*world_x += rand() % CAM_shake;
		*world_y += rand() % CAM_shake;
		*world_z += rand() % CAM_shake;

		*world_x -= CAM_shake >> 1;
		*world_y -= CAM_shake >> 1;
		*world_z -= CAM_shake >> 1;
	}
}

void CAM_get_pos(
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z)
{
	*world_x = CAM_pos_x >> 8;
	*world_y = CAM_pos_y >> 8;
	*world_z = CAM_pos_z >> 8;
}

void CAM_get_dpos(
		SLONG *dpos_x,
		SLONG *dpos_y,
		SLONG *dpos_z,
		SLONG *yaw,
		SLONG *pitch)
{
	*dpos_x = CAM_pos_x - CAM_focus_x >> 8;
	*dpos_y = CAM_pos_y - CAM_focus_y >> 8;
	*dpos_z = CAM_pos_z - CAM_focus_z >> 8;
	*yaw    = CAM_yaw;
	*pitch  = CAM_pitch;
}

void CAM_set_dpos(
		SLONG dpos_x,
		SLONG dpos_y,
		SLONG dpos_z,
		SLONG yaw,
		SLONG pitch)
{
	CAM_pos_x = (dpos_x << 8) + CAM_focus_x;
	CAM_pos_y = (dpos_y << 8) + CAM_focus_y;
	CAM_pos_z = (dpos_z << 8) + CAM_focus_z;
	CAM_yaw   = yaw;
	CAM_pitch = pitch;

	//
	// In radians now...
	//

	CAM_radians_yaw   = float(CAM_yaw)   * (2.0F * PI / 2048.0F);
	CAM_radians_pitch = float(CAM_pitch) * (2.0F * PI / 2048.0F);

	CAM_look_at_thing(FALSE);
}

void CAM_set_to_leave_sewers_position(Thing *darci)
{
	SLONG x;
	SLONG y;
	SLONG z;

	SLONG vector[3];
	
	//
	// Which way is Darci facing?
	// 

	FMATRIX_vector(
		vector,
		darci->Draw.Tweened->Angle,
		0);

	//
	// A little behind Darci, and lot up.
	//

	CAM_pos_x  = darci->WorldPos.X + (vector[0] * 3 >> 1);
	CAM_pos_y  = darci->WorldPos.Y + (vector[1] * 3 >> 1);
	CAM_pos_z  = darci->WorldPos.Z + (vector[2] * 3 >> 1);

	CAM_pos_y += 0x50000;

	//
	// No swooping to look at the thing.
	//

	CAM_look_at_thing(FALSE);
}

#endif