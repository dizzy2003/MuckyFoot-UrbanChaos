//
// A grappling hook.
//

#include "game.h"
#include "hook.h"
#include "pap.h"
#include "fmatrix.h"


//
// Each point of the string.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SWORD dx;
	SWORD dy;
	SWORD dz;
	UWORD alive;

} HOOK_Point;

#define HOOK_NUM_POINTS 256

HOOK_Point HOOK_point[HOOK_NUM_POINTS];

//
// How far each hook point wants to be from its neighbours.
// The gravity acceleration on the points.
//

#define HOOK_POINT_DIST		( 0x800)
#define HOOK_POINT_GRAVITY	(-0x80)
#define HOOK_POINT_FRICTION	( 2)
#define HOOK_POINT_ATTRACT	( 2)
#define HOOK_POINT_JUMP		( 0x1800)

#define HOOK_GRAPPLE_GRAVITY  (-0x80)

//
// How far the string is unreeled to.
//

UBYTE HOOK_reeled;

//
// The current state
//

UBYTE HOOK_state;

//
// The pitch the grappling hook is currently being swung through.
//

SLONG HOOK_grapple_yaw;
SLONG HOOK_grapple_pitch;

SLONG HOOK_spin_x;
SLONG HOOK_spin_y;
SLONG HOOK_spin_z;
SLONG HOOK_spin_speed;

//
// Processing after we've gone still.
//

SLONG HOOK_countdown;



SLONG HOOK_get_state()
{
	return HOOK_state;
}


//
// Put the string in a loop at (x,z).
//

#define HOOK_LOOP_RADIUS (0x2000)
#define HOOK_LOOP_ANGLE	 ((1436 * 2048) / 36000)		// 14.36 degrees...
#define HOOK_LOOP_RAISE  (0x0)

void HOOK_make_loop(SLONG x, SLONG z)
{
	SLONG i;
	SLONG y;
	SLONG dy;
	SLONG angle;
	SLONG dx;
	SLONG dz;
	SLONG mx;
	SLONG mz;
	SLONG ground;

	HOOK_Point *hp;

	dy     = 0;
	angle  = 0;
	ground = PAP_calc_height_at(x,z) << 8;

	mx = x << 8;
	mz = z << 8;

	for (i = HOOK_NUM_POINTS - 1; i >= 0; i--)
	{
		hp = &HOOK_point[i];

		dx = SIN(angle) * HOOK_LOOP_RADIUS >> 16;
		dz = COS(angle) * HOOK_LOOP_RADIUS >> 16;

		hp->x     = mx     + dx;
		hp->z     = mz     + dz;
		hp->y     = ground + dy;
		hp->dx    = 0;
		hp->dy    = 0;
		hp->dz    = 0;
		hp->alive = FALSE;

		mx += SIN(i << 2) >> 11;
		mz += COS(i << 2) >> 11;

		dy    += HOOK_LOOP_RAISE;
		angle += HOOK_LOOP_ANGLE;

		angle &= 2047;
	}
}

void HOOK_init(
		SLONG x,
		SLONG z)
{
	//
	// Loop up the string.
	//

	HOOK_make_loop(x,z);

	//
	// Good start angle.
	//

	HOOK_grapple_pitch = 512;
}

//
// Processes the points starting at the given point.
//

void HOOK_process_points(SLONG start_point)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG ddx;
	SLONG ddy;
	SLONG ddz;

	SLONG dist;
	SLONG ddist;
	SLONG ground;

	HOOK_Point *hp;
	HOOK_Point *hp_near;

	//
	// Process all the points.
	//

	i = start_point;

	while(1)
	{
		ASSERT(WITHIN(i, 1, HOOK_NUM_POINTS - 1));

		hp      = &HOOK_point[i];
		hp_near = &HOOK_point[i - 1];

		//
		// The link of this point to its predecessor.
		//

		dx = hp_near->x - hp->x;
		dy = hp_near->y - hp->y;
		dz = hp_near->z - hp->z;

		dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

		if (dist > HOOK_POINT_JUMP)
		{
			//
			// Too far from the previous point- jump to nearer
			// the previous point.
			//

			//
			// Guard against overflows!
			// 

			hp->x += (dx * (dist - HOOK_POINT_JUMP >> 8)) / (dist >> 8);
			hp->y += (dy * (dist - HOOK_POINT_JUMP >> 8)) / (dist >> 8);
			hp->z += (dz * (dist - HOOK_POINT_JUMP >> 8)) / (dist >> 8);

			dx = hp_near->x - hp->x;
			dy = hp_near->y - hp->y;
			dz = hp_near->z - hp->z;

			dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;
		}

		ddist   = dist;
		ddist  -= HOOK_POINT_DIST;
		ddist >>= HOOK_POINT_ATTRACT;

		ddx = (dx * ddist) / dist;
		ddy = (dy * ddist) / dist;
		ddz = (dz * ddist) / dist;

		hp->dx += ddx;
		hp->dy += ddy;
		hp->dz += ddz;

		if (i < HOOK_NUM_POINTS - 1)
		{
			hp_near = &HOOK_point[i + 1];

			//
			// The link of this point to its next point.
			//
		
			dx = hp_near->x - hp->x;
			dy = hp_near->y - hp->y;
			dz = hp_near->z - hp->z;

			dist    = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;
			ddist   = dist;
			ddist  -= HOOK_POINT_DIST;
			ddist >>= HOOK_POINT_ATTRACT;

			if (dist < HOOK_POINT_JUMP)
			{
				ddx   = dx * ddist / dist;
				ddy   = dy * ddist / dist;
				ddz   = dz * ddist / dist;

				hp->dx += ddx;
				hp->dy += ddy;
				hp->dz += ddz;
			}
		}

		//
		// Gravity.
		// 

		hp->dy += HOOK_POINT_GRAVITY;

		//
		// Friction.
		//

		hp->dx -= hp->dx >> HOOK_POINT_FRICTION;
		hp->dy -= hp->dy >> HOOK_POINT_FRICTION;
		hp->dz -= hp->dz >> HOOK_POINT_FRICTION;

		//
		// Actually move the point.
		//

		hp->x += hp->dx;
		hp->y += hp->dy;
		hp->z += hp->dz;
						
		//
		// Don't go underground.
		//

		ground = PAP_calc_map_height_at(hp->x >> 8, hp->z >> 8) << 8;

		if (hp->y < ground)
		{
			dy = ground - hp->y;

			if (dy < 0x2000)
			{
				hp->y   = ground;
				hp->dy  = 0;

				if (abs(hp->dx) + abs(hp->dz) < 256)
				{
					hp->dx >>= 2;
					hp->dz >>= 2;
				}
			}
			else
			{
				//
				// Sliding along a wall?
				//

				ground = PAP_calc_map_height_at((hp->x - hp->dx) >> 8, hp->z >> 8) << 8;

				if (hp->y > ground)
				{
					hp->x  -= hp->dx;
					hp->dx  = 0;
				}
				else
				{
					hp->z  -= hp->dz;
					hp->dz  = 0;
				}
			}
		}

		if (i == HOOK_reeled)
		{
			if (HOOK_reeled == HOOK_NUM_POINTS - 1)
			{
				break;
			}
			else
			{
				//
				// Too far from the next point? Should it unreel?
				//

				if (ddist > 0)
				{
					HOOK_reeled += 1;
				}
				else
				{
					break;
				}
			}
		}

		i += 1;
	}
}


//
// Spins the hook about the given point.
//

void HOOK_spin(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG speed_or_minus_pitch)
{
	SLONG i;

	//
	// The number of points from where darci hold the string
	// to where the grappling hook is.
	//

	#define HOOK_NUM_HOLD 5

	/*

	if (HOOK_state != HOOK_STATE_SPINNING)
	{
		//
		// Create a loop of string on the ground (x,z) 
		//

		HOOK_make_loop(x,z);

		//
		// We've started using the hook.
		//

		HOOK_state  = HOOK_STATE_SPINNING;
		HOOK_reeled = HOOK_NUM_HOLD + 1;

		//
		// Put the string in the correct pose.
		//

		HOOK_point[HOOK_NUM_HOLD].x = x << 8;
		HOOK_point[HOOK_NUM_HOLD].y = y << 8;
		HOOK_point[HOOK_NUM_HOLD].z = z << 8;
		
		for (i = 0; i < 16; i++)
		{
			HOOK_process_points(HOOK_NUM_HOLD + 1);
		}

		if (speed_or_minus_pitch > 0)
		{
			//
			// Start with the grappling hook hanging down.
			//

			HOOK_grapple_pitch = 1536;
		}
		else
		{
			HOOK_grapple_pitch = -(speed_or_minus_pitch) & 2047;
		}	

		//
		// A remotely sensible value so that (old_x,old_y,old_z)
		// wont have complete garbage in it the first time around.
		//

		HOOK_point[0].x = x << 8;
		HOOK_point[0].y = y << 8;
		HOOK_point[0].z = z << 8;
	}

	*/

	HOOK_spin_x      = x << 8;
	HOOK_spin_y      = y << 8;
	HOOK_spin_z      = z << 8;
	HOOK_grapple_yaw = yaw;
	HOOK_state       = HOOK_STATE_SPINNING;
	HOOK_reeled      = HOOK_NUM_HOLD + 1;
	
	if (speed_or_minus_pitch > 0)
	{
		HOOK_spin_speed = speed_or_minus_pitch;
	}
	else
	{
		HOOK_spin_speed    = 0;
		HOOK_grapple_pitch = -(speed_or_minus_pitch) & 2047;
	}
}

void HOOK_release()
{
	ASSERT(HOOK_state == HOOK_STATE_SPINNING);

	HOOK_state = HOOK_STATE_FLYING;
}


void HOOK_process_flying()
{
	SLONG speed;
	SLONG ground;
	SLONG odd;
	SLONG dy;

	//
	// Process the grappling hook (point 0)
	//

	/*

	HOOK_grapple_yaw   += HOOK_spin_speed >> 1;
	HOOK_grapple_pitch += HOOK_spin_speed >> 1;

	if (HOOK_spin_speed > 0)
	{
		HOOK_spin_speed -= 1;
	}

	*/

	HOOK_point[0].dy += HOOK_GRAPPLE_GRAVITY;

	HOOK_point[0].x += HOOK_point[0].dx * TICK_RATIO >> TICK_SHIFT;
	HOOK_point[0].y += HOOK_point[0].dy * TICK_RATIO >> TICK_SHIFT;
	HOOK_point[0].z += HOOK_point[0].dz * TICK_RATIO >> TICK_SHIFT;

	ground = PAP_calc_map_height_at(
				HOOK_point[0].x >> 8,
				HOOK_point[0].z >> 8) << 8;

	if (HOOK_point[0].y < ground)
	{
		dy = ground - HOOK_point[0].y;

		if (dy < 0x4000)
		{
			//
			// A vertical bounce.
			//

			HOOK_point[0].y   = ground;
			HOOK_point[0].dy  = abs(HOOK_point[0].dy);

			HOOK_point[0].dx /= 2;
			HOOK_point[0].dy /= 2;
			HOOK_point[0].dz /= 2;

			speed   = abs(HOOK_point[0].dx);
			speed  += abs(HOOK_point[0].dy);
			speed  += abs(HOOK_point[0].dz);
			speed >>= 4;
			speed  += 1;

			odd  = rand() % speed;
			odd -= speed >> 1;

			HOOK_point[0].dx += odd << 3;

			odd  = rand() % speed;
			odd -= speed >> 1;

			HOOK_point[0].dz += odd << 3;

			HOOK_spin_speed = rand() & 0x1f;
		}
		else
		{
			//
			// Bouncing off a wall.
			//

			SLONG check_x;
			SLONG check_z;
			SLONG check_height;
			SLONG bounced_x = FALSE;
			SLONG bounced_z = FALSE;

			//
			// Bounced in x?
			// 

			check_x = HOOK_point[0].x - HOOK_point[0].dx >> 8;
			check_z = HOOK_point[0].z                    >> 8;

			check_height = PAP_calc_map_height_at(
								check_x,
								check_z) << 8;

			bounced_x = (check_height < HOOK_point[0].y);

			//
			// Bounced in z?
			//

			check_x = HOOK_point[0].x					  >> 8;
			check_z = HOOK_point[0].z  - HOOK_point[0].dz >> 8;

			check_height = PAP_calc_map_height_at(
								check_x,
								check_z) << 8;

			bounced_z = (check_height < HOOK_point[0].y);

			//
			// Do the bouncing.
			//
		
			if (bounced_x)
			{
				HOOK_point[0].dx = -HOOK_point[0].dx;
				HOOK_point[0].x +=  HOOK_point[0].dx;
			}
					
			if (bounced_z)
			{
				HOOK_point[0].dz = -HOOK_point[0].dz;
				HOOK_point[0].z +=  HOOK_point[0].dz;
			}

			HOOK_spin_speed = rand() & 0x1f;
		}
	}

	//
	// Process all the points.
	//

	HOOK_process_points(1);

	//
	// Is the grapple moving?
	//

	speed  = abs(HOOK_point[0].dx);
	speed += abs(HOOK_point[0].dy);
	speed += abs(HOOK_point[0].dz);

	if (speed < 0x100)
	{
		//
		// Hook has stopped moving.
		//

		HOOK_state     = HOOK_STATE_STILL;
		HOOK_countdown = 512;
	}
}


void HOOK_process_spinning()
{
	SLONG i;

	SLONG vector[3];

	SLONG old_x;
	SLONG old_y;
	SLONG old_z;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG px;
	SLONG py;
	SLONG pz;

	//
	// Set the grapple going.
	//

	old_x = HOOK_point[0].x;
	old_y = HOOK_point[0].y;
	old_z = HOOK_point[0].z;

	//
	// Work out the position of the grapple and the points connecting
	// it to Darci's hand.
	//

	FMATRIX_vector(
		vector,
		HOOK_grapple_yaw,
		HOOK_grapple_pitch);

	vector[0] = vector[0] * HOOK_POINT_DIST >> 16;
	vector[1] = vector[1] * HOOK_POINT_DIST >> 16;
	vector[2] = vector[2] * HOOK_POINT_DIST >> 16;

	px = HOOK_spin_x;
	py = HOOK_spin_y;
	pz = HOOK_spin_z;

	for (i = HOOK_NUM_HOLD; i >= 0; i--)
	{
		HOOK_point[i].x = px;
		HOOK_point[i].y = py;
		HOOK_point[i].z = pz;

		px += vector[0];
		py += vector[1];
		pz += vector[2];
	}

	dx = ((HOOK_point[0].x - old_x) * (1 << (TICK_SHIFT))) / TICK_RATIO;
	dy = ((HOOK_point[0].y - old_y) * (1 << (TICK_SHIFT))) / TICK_RATIO;
	dz = ((HOOK_point[0].z - old_z) * (1 << (TICK_SHIFT))) / TICK_RATIO;

	HOOK_point[0].dx = dx;
	HOOK_point[0].dy = dy;
	HOOK_point[0].dz = dz;

	//
	// Process the points.
	//

	HOOK_process_points(HOOK_NUM_HOLD + 1);

	//
	// Spin!
	//

	HOOK_grapple_pitch -= HOOK_spin_speed;
	HOOK_grapple_pitch &= 2047;
}



void HOOK_process()
{
	switch(HOOK_state)
	{
		case HOOK_STATE_STILL:

			//
			// Process for a while after coming to a standstill-
			// to give the string a chance to settle.
			//

			if (HOOK_countdown)
			{
				HOOK_process_flying();
				HOOK_countdown -= 1;
			}

			break;

		case HOOK_STATE_SPINNING:
			HOOK_process_spinning();
			break;

		case HOOK_STATE_FLYING:
			HOOK_process_flying();
			HOOK_process_flying();
			break;

		default:
			ASSERT(0);
			break;
	}
}






// ========================================================
//
// DRAWING THE HOOK.
//
// ========================================================

void HOOK_pos_grapple(
		SLONG *x,
		SLONG *y,
		SLONG *z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll)
{
	*x = HOOK_point[0].x;
	*y = HOOK_point[0].y + 0x1000;
	*z = HOOK_point[0].z;

	*yaw   = HOOK_grapple_yaw;
	*pitch = HOOK_grapple_pitch;
	*roll  = 0;
}

void HOOK_pos_point(SLONG point,
		SLONG *x,
		SLONG *y,
		SLONG *z)
{
	ASSERT(WITHIN(point, 0, HOOK_NUM_POINTS - 1));

	*x = HOOK_point[point].x;
	*y = HOOK_point[point].y;
	*z = HOOK_point[point].z;
}
