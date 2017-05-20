//
// Pigeons!
//

// Never used, even in the PC version.
#ifndef TARGET_DC


#include "game.h"
#include "pigeon.h"
#include "statedef.h"

//
// The functions called when a pigeon is in each state.
//

void PIGEON_fn_init  (Thing *);
void PIGEON_fn_normal(Thing *);

StateFunction PIGEON_state_function[] =
{
	{STATE_INIT,	PIGEON_fn_init	},
	{STATE_NORMAL,	PIGEON_fn_normal}
};

//
// Pigeon substates.
//

#define PIGEON_SUBSTATE_NONE	 0
#define PIGEON_SUBSTATE_WAIT	 1		// Do nothing.
#define PIGEON_SUBSTATE_PECK	 2		// Pecking food.
#define PIGEON_SUBSTATE_WALK	 3		// Walking along.
#define PIGEON_SUBSTATE_FLEE	 4		// Avoiding something on the gorund.
#define PIGEON_SUBSTATE_FLY 	 5		// Flying upto a perch.
#define PIGEON_SUBSTATE_PERCH    6		// Sitting on a perch.
#define PIGEON_SUBSTATE_LAND	 7		// Flying to a point on the ground.
#define PIGEON_SUBSTATE_NUMBER	 8

//
// In perch mode, the top two bits mean...
//

#define PIGEON_PERCH_WAIT			0
#define PIGEON_PERCH_SHUFFLE_LEFT	1
#define PIGEON_PERCH_SHUFFLE_RIGHT	2


void PIGEON_init_wait (Thing *pigeon);
void PIGEON_init_peck (Thing *pigeon);
void PIGEON_init_walk (Thing *pigeon);
void PIGEON_init_flee (Thing *pigeon, Thing *from);
void PIGEON_init_fly  (Thing *pigeon, Thing *from);
void PIGEON_init_perch(Thing *pigeon);					// Only call from FLY mode.
void PIGEON_init_land (Thing *pigeon);

//
// Returns the point 'along' along the given colvect,
// where along goes from 0 - 256.
// 

void PIGEON_find_pos_along_vect(SLONG vect, SLONG along, SLONG *x, SLONG *y, SLONG *z)
{
	CollisionVect *cv;

	ASSERT(WITHIN(vect, 1, next_col_vect - 1));

	cv = &col_vects[vect];

	*x  = cv->X[0] + (along * (cv->X[1] - cv->X[0]) >> 8);
	*y  = cv->Y[0] + (along * (cv->Y[1] - cv->Y[0]) >> 8);
	*z  = cv->Z[0] + (along * (cv->Z[1] - cv->Z[0]) >> 8);

 	*y += cv->PrimExtra << 6;
}


//
// Find a colvect for the pigeon to perch on. Returns NULL
// if there is no suitable colvect nearby.
//

UWORD PIGEON_find_perch(Thing *pigeon, UWORD ignore_this_vect)
{
#ifdef TARGET_DC
	// Shouldn't be using this, apparently.
	ASSERT ( FALSE );
#endif
	SLONG x;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG nx1;
	SLONG nz1;
	SLONG nx2;
	SLONG nz2;

	SLONG len;
	SLONG dist1;
	SLONG dist2;

	SLONG list;
	SLONG vect;

	SLONG score;

	SLONG best_vect;
	SLONG best_score;

	CollisionVectLink *cvl;
	CollisionVect     *cv;
	MapElement        *me;

	//
	// Find the box in which to look for colvects.
	//

	#define PIGEON_PERCH_SEARCH 4

	mx = pigeon->WorldPos.X >> 16;
	mz = pigeon->WorldPos.Z >> 16;

	nx1 = mx - PIGEON_PERCH_SEARCH;
	nz1 = mz - PIGEON_PERCH_SEARCH;
	nx2 = mx + PIGEON_PERCH_SEARCH;
	nz2 = mz + PIGEON_PERCH_SEARCH;

	SATURATE(nx1, 0, MAP_WIDTH - 1);
	SATURATE(nx2, 0, MAP_WIDTH - 1);

	SATURATE(nz1, 0, MAP_HEIGHT - 1);
	SATURATE(nz2, 0, MAP_HEIGHT - 1);

	//
	// Look for the best colvect in the bounding box.
	//

	best_score = INFINITY;	// Lower scores are better!

	for (x = nx1; x <= nx2; x++)
	for (z = nz1; z <= nz2; z++)
	{
		me = &MAP[MAP_INDEX(x,z)];

		//
		// Go through this mapsquare looking for collision vectors.
		//

		list = me->ColVectHead;

		while(list)
		{
			ASSERT(WITHIN(list, 1, next_col_vect_link - 1));

			cvl  = &col_vects_links[list];
			vect =  cvl->VectIndex;
			
			//
			// The col vect hanging off this link.
			//

			ASSERT(WITHIN(vect, 1, next_col_vect - 1));

			if (vect != ignore_this_vect)
			{
				cv = &col_vects[vect];

				dx = abs(cv->X[1] - cv->X[0]);
				dz = abs(cv->Z[1] - cv->Z[0]);

				len = QDIST2(dx,dz);

				if (len < 256)
				{
					//
					// Ignore colvects that are too small.
					//
				}
				else
				{
					//
					// How far is this colvect from the pigeon?
					//

					dx = cv->X[0] - (pigeon->WorldPos.X>>8);
					dz = cv->Z[0] - (pigeon->WorldPos.Z>>8);

					dist1 = dx*dx + dz*dz;

					dx = cv->X[1] - (pigeon->WorldPos.X>>8);
					dz = cv->Z[1] - (pigeon->WorldPos.Z>>8);

					dist2 = dx*dx + dz*dz;

					score = MIN(dist1, dist2);

					if (score < best_score)
					{
						best_vect  = vect;
						best_score = score;
					}
				}
			}

			list = cvl->Next;
		}
	}

	if (best_score < INFINITY)
	{
		return best_vect;
	}
	else
	{
		//
		// No nearby colvects!
		//

		return NULL;
	}
}




// ========================================================
//
// Initialising the various substates.
//
// ========================================================

void PIGEON_init_wait(Thing *pigeon)
{
	Animal *animal = ANIMAL_get_animal(pigeon);

	//
	// Use the counter for how long we should wait.
	//

	animal->counter  = rand() & 0x1f;
	animal->counter += 0x10;
	animal->substate = PIGEON_SUBSTATE_WAIT;
}

void PIGEON_init_walk(Thing *pigeon)
{
	SLONG dx;
	SLONG dz;

	SLONG angle;

	Animal   *animal = ANIMAL_get_animal  (pigeon);
	DrawMesh *dm     = ANIMAL_get_drawmesh(pigeon);

	//
	// Pick a direction to walk in.
	//

	angle = rand() & 2047;

	//
	// Turn to face in this direction.
	//

	dm->Angle = angle;

	//
	// How long should we walk for?
	//

	animal->counter  = rand() & 0x1f;
	animal->counter += 0xf;

	//
	// Start walking!
	//

	animal->substate = PIGEON_SUBSTATE_WALK;
}

void PIGEON_init_peck(Thing *pigeon)
{
	Animal *animal = ANIMAL_get_animal(pigeon);

	//
	// Just make the pigeon jump up and down for now!
	// But for how long?
	//

	#define PIGEON_PECK_TIME 32

	animal->counter = PIGEON_PECK_TIME;

	//
	// Start pecking!
	//

	animal->substate = PIGEON_SUBSTATE_PECK;
}

void PIGEON_init_fly(Thing *pigeon, Thing *from)
{
	SLONG dx;
	SLONG dz;

	SLONG dist;
	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	SLONG angle;
	UWORD ignore_vect;

	//
	// Find a perch.
	//

	UWORD vect;

	Animal   *animal = ANIMAL_get_animal  (pigeon);
	DrawMesh *dm     = ANIMAL_get_drawmesh(pigeon);

	//
	// Dont fly to the same colvect you are already on.
	//

	if (animal->substate == PIGEON_SUBSTATE_PERCH)
	{
		ignore_vect = animal->other_index;
	}
	else
	{
		ignore_vect = NULL;
	}

	vect = PIGEON_find_perch(pigeon, ignore_vect);

	if (vect)
	{
		//
		// Start flying to somewhere along the otp of this vect.
		//

		animal->substate    = PIGEON_SUBSTATE_FLY;
		animal->other_index = vect;
		animal->along       = Random();
		animal->counter     = 0;
		animal->starty      = pigeon->WorldPos.Y >> 8;
		animal->map_x       = pigeon->WorldPos.X >> 16;
		animal->map_z       = pigeon->WorldPos.Z >> 16;

		//
		// How far does the pigeon start from its destination?
		//

		PIGEON_find_pos_along_vect(
			animal->other_index,
			animal->along,
		   &dest_x,
		   &dest_y,
		   &dest_z);

		dx = dest_x - (pigeon->WorldPos.X >> 8);
		dz = dest_z - (pigeon->WorldPos.Z >> 8);

		dist = QDIST2(abs(dx),abs(dz));

		animal->dist = dist;
	}
	else
	{
		//
		// Nowhere to fly to, just flee.
		//

		PIGEON_init_flee(pigeon, from);
	}
}

void PIGEON_init_flee(Thing *pigeon, Thing *from)
{
	SLONG dx;
	SLONG dz;
	SLONG angle;

	Animal   *animal = ANIMAL_get_animal  (pigeon);
	DrawMesh *dm     = ANIMAL_get_drawmesh(pigeon);

	//
	// Where do we run from?
	//

	dx = (from->WorldPos.X - pigeon->WorldPos.X)>>8;
	dz = (from->WorldPos.Z - pigeon->WorldPos.Z)>>8;

	angle  = 1024 - Arctan(dx, dz);
	angle += rand() & 0x3f;
	angle -= 0x1f;
	angle &= 2047;

	//
	// How long to we run away for?
	//

	#define PIGEON_FLEE_TIME 0x20

	dm->Angle       = angle;
	animal->counter = PIGEON_FLEE_TIME;

	//
	// Start fleeing.
	//

	animal->substate = PIGEON_SUBSTATE_FLEE;

	MSG_add("Init flee");
}

//
// Only call this function when you've finished flying.
//

void PIGEON_init_perch(Thing *pigeon)
{
	SLONG dx;
	SLONG dz;

	SLONG angle;

	Animal   *animal = ANIMAL_get_animal  (pigeon);
	DrawMesh *dm     = ANIMAL_get_drawmesh(pigeon);

	ASSERT(animal->substate == PIGEON_SUBSTATE_FLY);	

	CollisionVect *cv;

	ASSERT(WITHIN(animal->other_index, 1, next_col_vect - 1));

	cv = &col_vects[animal->other_index];

	//
	// Face pointing out of the colvect you have perched on.
	//

	dx = cv->X[1] - cv->X[0];
	dz = cv->Z[1] - cv->Z[0];

	angle  = 512 - Arctan(dx, dz);

	dm->Angle = angle;

	//
	// The top 2 bits of counter mean either, wait, shuffle left or shuffle right.
	//

	animal->counter  = Random() & 31;
	animal->counter += 32;
	animal->counter |= PIGEON_PERCH_WAIT << 6;		// Wait...

	animal->substate = PIGEON_SUBSTATE_PERCH;
}


void PIGEON_init_land(Thing *pigeon)
{
}


//
// Decides what the pigeon should do next.
//

void PIGEON_start_doing_something(Thing *pigeon)
{
	switch(Random() & 0x3)
	{
		case 0:
		case 1:
			PIGEON_init_peck(pigeon);
			break;

		case 2:
			PIGEON_init_walk(pigeon);
			break;

		case 3:
			PIGEON_init_wait(pigeon);
			break;

		default:
			ASSERT(0);
			break;
	}
}




// ========================================================
//
// The substate processing functions.
//
// ========================================================

void PIGEON_process_wait(Thing *pigeon)
{
	Animal *animal = ANIMAL_get_animal(pigeon);

	if (animal->counter == 0)
	{
		//
		// Don't wait any longer... do something else.
		//

		PIGEON_start_doing_something(pigeon);
	}
	else
	{
		animal->counter -= 1;
	}
}


void PIGEON_process_peck(Thing *pigeon)
{
	Animal *animal = ANIMAL_get_animal(pigeon);

	animal->counter -= 1;

	//
	// Have a bit of standing around at the end of pecking.
	//

	#define PIGEON_PECK_SLEEP 8

	if (animal->counter >= PIGEON_PECK_SLEEP)
	{
		if (animal->counter & 0x1)
		{
			pigeon->WorldPos.Y += 0x10<<8;
		}
		else
		{
			pigeon->WorldPos.Y -= 0x10<<8;
		}
	}

	if (animal->counter == 0)
	{
		//
		// Enough pecking... do something else now.
		//

		PIGEON_start_doing_something(pigeon);
	}
}

void PIGEON_process_walk(Thing *pigeon)
{
	SLONG dx;
	SLONG dz;

	Animal   *animal = ANIMAL_get_animal  (pigeon);
	DrawMesh *dm     = ANIMAL_get_drawmesh(pigeon);

	//
	// Have we arrived yet?
	//

	animal->counter -= 1;

	if (animal->counter == 0)
	{
		//
		// We have arrived. What do we do now?
		//

		PIGEON_start_doing_something(pigeon);
	}
	else
	{
		//
		// Move the pigeon.
		//

		GameCoord new_pos = pigeon->WorldPos;

		dx = SIN(dm->Angle) >> 6;
		dz = COS(dm->Angle) >> 6;

		new_pos.X -= dx;
		new_pos.Z -= dz;

		dx  *= TICK_RATIO;
		dz  *= TICK_RATIO;

		dx >>= TICK_SHIFT;
		dz >>= TICK_SHIFT;

		move_thing_on_map(pigeon, &new_pos);
	}
}

void PIGEON_process_flee(Thing *pigeon)
{
	SLONG dx;
	SLONG dz;

	Animal   *animal = ANIMAL_get_animal  (pigeon);
	DrawMesh *dm     = ANIMAL_get_drawmesh(pigeon);

	//
	// Time to stop fleeing?
	//

	animal->counter -= 1;

	if (animal->counter == 0)
	{
		PIGEON_start_doing_something(pigeon);
	}
	else
	{
		//
		// Move the pigeon.
		//

		GameCoord new_pos = pigeon->WorldPos;

		if (animal->counter >= PIGEON_FLEE_TIME / 2)
		{
			dx = SIN(dm->Angle) >> 4;
			dz = COS(dm->Angle) >> 4;
		}
		else
		{
			dx = SIN(dm->Angle) >> 5;
			dz = COS(dm->Angle) >> 5;
		}

		dx  *= TICK_RATIO;
		dz  *= TICK_RATIO;

		dx >>= TICK_SHIFT;
		dz >>= TICK_SHIFT;

		new_pos.X -= dx;
		new_pos.Z -= dz;

		move_thing_on_map(pigeon, &new_pos);
	}
}

void PIGEON_process_fly(Thing *pigeon)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG yangle;
	SLONG angle;
	SLONG dangle;
	SLONG speed;
	SLONG dist;

	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	SLONG vel_x;
	SLONG vel_z;

	GameCoord new_pos;

	Animal *animal = ANIMAL_get_animal(pigeon);
	DrawMesh *dm   = ANIMAL_get_drawmesh(pigeon);

	//
	// Where are we flying to?
	//

	PIGEON_find_pos_along_vect(
		animal->other_index,
		animal->along,
	   &dest_x,
	   &dest_y,
	   &dest_z);

	e_draw_3d_line(
		pigeon->WorldPos.X >> 8,
		pigeon->WorldPos.Y >> 8,
		pigeon->WorldPos.Z >> 8,
		dest_x,
		dest_y,
		dest_z);

	//
	// Turn the pigeon towards its destination.
	//

	dx = dest_x - (pigeon->WorldPos.X >> 8);
	dy = dest_y - (pigeon->WorldPos.Y >> 8);
	dz = dest_z - (pigeon->WorldPos.Z >> 8);

	dist = QDIST2(abs(dx),abs(dz));

	angle  = -Arctan(dx, dz);
	angle &=  2047;

	dangle = angle - dm->Angle;

	if (dangle >  1024) {dangle -= 2048;}
	if (dangle < -1024) {dangle += 2048;}

	dangle >>= 3;

	SATURATE(dangle, -100, +100);

	dm->Angle += dangle;
	dm->Angle &= 2047;

	//
	// How fast is the pigeon moving.
	//

	#define PIGEON_FLY_ACCEL_SPEED 15
	#define PIGEON_FLY_DECEL_SPEED 32

	#define PIGEON_FLY_ACCEL_TIME 8
	#define PIGEON_FLY_DECEL_TIME 16

	if (animal->counter < PIGEON_FLY_ACCEL_TIME)
	{
		speed = PIGEON_FLY_ACCEL_SPEED;
		animal->counter += 1;
	}
	else
	{
		speed = PIGEON_FLY_DECEL_SPEED;
	}

	//
	// Are we too close for our dangle?
	//

	if (abs(dangle) > 10)
	{
		if (dist < 256) {speed >>= 1;}
		if (dist < 128) {speed >>= 1;}
	}

	if (dist < 90) {speed >>= 1;}
	if (dist < 50) {speed >>= 1;}

	//
	// Find the new pigeon position in x,z
	//

	vel_x = SIN(dm->Angle) * speed >> 8;
	vel_z = COS(dm->Angle) * speed >> 8;

	vel_x  *= TICK_RATIO;
	vel_z  *= TICK_RATIO;

	vel_x >>= TICK_SHIFT;
	vel_z >>= TICK_SHIFT;

	new_pos.X = pigeon->WorldPos.X - vel_x;
	new_pos.Z = pigeon->WorldPos.Z - vel_z;

	//
	// Find the new pigeon position in y.
	//

	yangle = 512 - (dist * 512) / animal->dist;

	SATURATE(yangle, 0, 512);

	new_pos.Y   = animal->starty;
	new_pos.Y  += (dest_y - animal->starty) * SIN(yangle) >> 16;
	new_pos.Y <<= 8;

	//
	// Move the pigeon.
	//
	
	move_thing_on_map(pigeon, &new_pos);

	if (dist < PIGEON_FLY_DECEL_SPEED)
	{
		//
		// Start perching.
		//

		PIGEON_init_perch(pigeon);
	}
}

void PIGEON_process_perch(Thing *pigeon)
{
	UBYTE doing;
	UBYTE howlong;
	UBYTE oldalong;

	Animal *animal = ANIMAL_get_animal(pigeon);

	doing    = animal->counter >> 6;
	howlong  = animal->counter & 0x3f;

	if (howlong == 0)
	{
		switch(Random() & 0x3)
		{
			case 0:
				howlong = 32 + (Random() & 31);
				doing   = PIGEON_PERCH_WAIT;
				break;

			case 1:
				howlong = 32 + (Random() & 31);
				doing   = PIGEON_PERCH_SHUFFLE_LEFT;
				break;

			case 2:
				howlong = 32 + (Random() & 31);
				doing   = PIGEON_PERCH_SHUFFLE_RIGHT;
				break;

			case 3:

				//
				// It is too scary to return to where we were
				// (animal->map_x, animal->map_z)?
				//

				PIGEON_init_land(pigeon);
				return;
		}
	}
	else
	{
		howlong -= 1;
	}

	switch(doing)
	{
		case PIGEON_PERCH_WAIT:
			break;

		case PIGEON_PERCH_SHUFFLE_LEFT:

			if (animal->along == 0)
			{
				doing = PIGEON_PERCH_SHUFFLE_RIGHT;
			}
			else
			{
				animal->along -= 1;
			}
			
			break;

		case PIGEON_PERCH_SHUFFLE_RIGHT:

			if (animal->along == 255)
			{
				doing = PIGEON_PERCH_SHUFFLE_LEFT;
			}
			else
			{
				animal->along += 1;
			}
			
			break;

		default:
			ASSERT(0);
			break;
	}

	animal->counter  = howlong;
	animal->counter |= doing << 6;

	//
	// Move the pigeon.
	//

	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	PIGEON_find_pos_along_vect(
		animal->other_index,
		animal->along,
	   &dest_x,
	   &dest_y,
	   &dest_z);

	GameCoord new_pos;

	new_pos.X = dest_x << 8;
	new_pos.Y = dest_y << 8;
	new_pos.Z = dest_z << 8;

	move_thing_on_map(pigeon, &new_pos);
}

void PIGEON_process_land(Thing *pigeon)
{
	Animal *animal = ANIMAL_get_animal(pigeon);

}

// ========================================================
//
// The state functions.
//
// ========================================================

void PIGEON_fn_init(Thing *pigeon)
{
	Animal *animal = ANIMAL_get_animal(pigeon);
	DrawMesh *dm   = ANIMAL_get_drawmesh(pigeon);

	dm->Angle = 0;
	dm->Tilt  = 0;
	dm->Roll  = 0;

	//
	// Start being a normal pigeon.
	//

	set_state_function(pigeon, STATE_NORMAL);

	//
	// Start doing something.
	//

	PIGEON_start_doing_something(pigeon);

}

void PIGEON_fn_normal(Thing *pigeon)
{
	SLONG i;

	Thing  *p_scary;
	Animal *scary_animal;

	Animal *animal = ANIMAL_get_animal(pigeon);

	switch(animal->substate)
	{
		case PIGEON_SUBSTATE_NONE:
			break;
		case PIGEON_SUBSTATE_WAIT:	PIGEON_process_wait (pigeon); break;
		case PIGEON_SUBSTATE_PECK:	PIGEON_process_peck (pigeon); break;
		case PIGEON_SUBSTATE_WALK:	PIGEON_process_walk (pigeon); break;
		case PIGEON_SUBSTATE_FLEE:	PIGEON_process_flee (pigeon); break;
		case PIGEON_SUBSTATE_FLY:	PIGEON_process_fly  (pigeon); break;
		case PIGEON_SUBSTATE_PERCH:	PIGEON_process_perch(pigeon); break;
		case PIGEON_SUBSTATE_LAND:	PIGEON_process_land (pigeon); break;
			break;

		default:
			ASSERT(0);
			break;
	}

	if (animal->substate == PIGEON_SUBSTATE_FLY)
	{
		//
		// You can't scare flying pigeons.
		//

		return;
	}

	//
	// Is the pigeon near anything scary?
	//

	#define PIGEON_MAX_SCARY 32

	THING_INDEX scary[PIGEON_MAX_SCARY];
	SLONG       scary_upto;

	//
	// How far to look for something scary.
	//

	#define PIGEON_SCARY_RADIUS (0xd0)

	scary_upto = THING_find_sphere(
					pigeon->WorldPos.X >> 8,
					pigeon->WorldPos.Y >> 8,
					pigeon->WorldPos.Z >> 8,
					PIGEON_SCARY_RADIUS,
					scary,
					PIGEON_MAX_SCARY,
					THING_FIND_MOVING);

	for (i = 0; i < scary_upto; i++)
	{
		p_scary = TO_THING(scary[i]);

		switch(p_scary->Class)
		{
			case CLASS_ANIMAL:

				scary_animal = ANIMAL_get_animal(p_scary);

				//
				// A pigeon is scared of any animal apart from a pigeon.
				//

				if (scary_animal->AnimalType != ANIMAL_PIGEON)
				{
					PIGEON_init_fly(pigeon, p_scary);
				}
				
				break;

			case CLASS_PERSON:
			case CLASS_PROJECTILE:
				PIGEON_init_fly(pigeon, p_scary);
				break;

			default:
				ASSERT(0);
				break;
		}
	}
}


#endif //#ifndef TARGET_DC



