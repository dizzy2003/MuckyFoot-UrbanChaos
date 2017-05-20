//
// Platforms are moving prims.  You can walk on the bounding box of the
// walkable faces of the prim.
//

#include "game.h"
#include "eway.h"
#include "ob.h"
#include "plat.h"
#include "statedef.h"
#include "wmove.h"
#include "animate.h"
#include "psystem.h"
#include "panel.h"
#ifndef PSX
#include "poly.h"
#include "drawxtra.h"
#endif



Plat  *PLAT_plat; //[PLAT_MAX_PLATS];
SLONG PLAT_plat_upto;

//
// The different states a plat can be in.
//

#define PLAT_STATE_NONE  0
#define PLAT_STATE_GOTO  1	// Heading towards a waypoint.
#define PLAT_STATE_PAUSE 2	// Waiting at a waypoint.
#define PLAT_STATE_STOP  3	// Never move again.

//
// Private flags for a plat.
//

#define PLAT_FLAG_LOCK_X (1 << 5)
#define PLAT_FLAG_LOCK_Y (1 << 6)
#define PLAT_FLAG_LOCK_Z (1 << 7)

#ifndef PSX
void PLAT_init()
{
	memset(PLAT_plat, 0, sizeof(Plat)*PLAT_MAX_PLATS);

	PLAT_plat_upto = 1;
}
#endif

void PLAT_process(Thing *p_thing)
{	
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG way_x;
	SLONG way_y;
	SLONG way_z;

	SLONG waypoint;
	SLONG millisecs;
	SLONG ticks;

	SLONG wspeed;
	SLONG speed;
	SLONG move;
	SLONG len;
	SLONG overlen;
	
	GameCoord newpos;

	//
	// How many millisecs this frame?
	//

	millisecs = 50 * TICK_RATIO >> TICK_SHIFT;
	ticks     =  5 * TICK_RATIO >> TICK_SHIFT;

	ASSERT(p_thing->Class = CLASS_PLAT);

	Plat     *plat = p_thing->Genus.Plat;
	DrawMesh *dm   = p_thing->Draw.Mesh;

	switch(plat->state)
	{
		case PLAT_STATE_NONE:

			//
			// What should we be doing?
			//

			switch(plat->move)
			{
				case PLAT_MOVE_STILL:
					
					//
					// We are doing nothing- but that is what we are meant to be doing.
					//

					break;

				case PLAT_MOVE_PATROL:
				case PLAT_MOVE_PATROL_RAND:

					//
					// Wait for a while.
					//

					plat->waypoint = NULL;
					plat->state    = PLAT_STATE_PAUSE;
					plat->counter  = 1 * 1000;

					break;

				default:
					ASSERT(0);
					break;
			}


			break;

		case PLAT_STATE_GOTO:

			//
			// Bodge rocket-exhaust in here
			//
#ifndef PSX
			if (plat->flag&PLAT_FLAG_BODGE_ROCKET)
			{
				PARTICLE_Add(p_thing->WorldPos.X+(((Random()&0xff)-0x7f)<<7),p_thing->WorldPos.Y,p_thing->WorldPos.Z+(((Random()&0xff)-0x7f)<<7),
					((Random()&0xff)-0x7f)<<2,0,((Random()&0xff)-0x7f)<<2,
					POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7FFFFFFF,
					PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE,
					300,70,1,1,2);
				PARTICLE_Add(p_thing->WorldPos.X+(((Random()&0xff)-0x7f)<<5),p_thing->WorldPos.Y,p_thing->WorldPos.Z+(((Random()&0xff)-0x7f)<<5),
					((Random()&0xff)-0x7f)<<2,0,((Random()&0xff)-0x7f)<<2,
					POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0x7FFFFFFF,
					PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE,
					300,70,1,1,-2);
				BLOOM_draw(p_thing->WorldPos.X>>8,p_thing->WorldPos.Y>>8,p_thing->WorldPos.Z>>8,
					0,-0xff,0,0x00ffffff,BLOOM_BEAM|BLOOM_LENSFLARE);
				
			}
#endif

			//
			// What direction should we be going in?
			//

			if (ControlFlag)
			{
				waypoint = 0;
			}

			EWAY_get_position(
				plat->waypoint,
			   &way_x,
			   &way_y,
			   &way_z);

			dx = way_x - (p_thing->WorldPos.X >> 8);
			dy = way_y - (p_thing->WorldPos.Y >> 8);
			dz = way_z - (p_thing->WorldPos.Z >> 8);

			     if (plat->flag & PLAT_FLAG_LOCK_X) {dy = 0; dz = 0;}
			else if (plat->flag & PLAT_FLAG_LOCK_Y) {dz = 0; dx = 0;}
			else if (plat->flag & PLAT_FLAG_LOCK_Z) {dx = 0; dy = 0;}

			len = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

			//
			// Accelerate to our desired speed.
			//

			wspeed = plat->wspeed;
			speed  = plat->speed;

			if (len < 0x100) {wspeed >>= 2;}
			if (len < 0x80 ) {wspeed >>= 2;}
			if (len < 0x40 ) {wspeed >>= 2;}

			if (wspeed < 3) {wspeed = 3;}

			if (abs(speed - wspeed) < ticks)
			{
				speed = wspeed;
			}
			else
			if (speed > wspeed)
			{
				speed -= ticks;
			}
			else
			{
				speed += ticks;
			}

			SATURATE(speed, 0, 255);

			plat->speed = speed;

			//
			// How far should we move this gameturn?
			//

			move = speed * TICK_RATIO >> TICK_SHIFT;

			//
			// Have we arrived?
			//

			if (len <= ((move >> 8) + 4))
			{
				//
				// We have arrived. Wait for a while before moving onto the next waypoint.
				//

				plat->state   = PLAT_STATE_PAUSE;
				plat->counter = EWAY_get_delay(plat->waypoint, 2 * 1000);

				if (plat->counter == 10000)
				{
					//
					// This amount of time (10 seconds) actually means wait here forever!
					//

					plat->state = PLAT_STATE_STOP;
				}
			}
			else
			{
				overlen = (move << 8) / len;
				dx      = dx * overlen;
				dy      = dy * overlen;
				dz      = dz * overlen;

				dx = dx * TICK_RATIO >> TICK_SHIFT;
				dy = dy * TICK_RATIO >> TICK_SHIFT;
				dz = dz * TICK_RATIO >> TICK_SHIFT;

				SATURATE(dy, -8 << 8, +8 << 8);

				newpos.X = p_thing->WorldPos.X + dx;
				newpos.Y = p_thing->WorldPos.Y + dy;
				newpos.Z = p_thing->WorldPos.Z + dz;

				move_thing_on_map(p_thing, &newpos);

				//
				// Look for people to push out of the way.
				//

				{
					#define PLAT_MAX_FIND 8

					SLONG       i;
					SLONG       num;
					THING_INDEX found[PLAT_MAX_FIND];
					Thing      *p_person;

					PrimInfo *pi = get_prim_info(dm->ObjectId);

					num = THING_find_sphere(
							p_thing->WorldPos.X >> 8,
							p_thing->WorldPos.Y >> 8,
							p_thing->WorldPos.Z >> 8,
							pi->radius + 0x100,
							found,
							PLAT_MAX_FIND,
							THING_FIND_PEOPLE);

					SLONG y_bot = p_thing->WorldPos.Y >> 8;
					SLONG y_top = p_thing->WorldPos.Y >> 8;
	
					y_top += pi->maxy;
					y_bot += pi->miny;

					for (i = 0; i < num; i++)
					{
						p_person = TO_THING(found[i]);

						if (p_person->State == STATE_DEAD ||
							p_person->State == STATE_DYING)
						{
							continue;
						}

						if (WITHIN(p_person->WorldPos.Y >> 8, y_bot - 0x100, y_top + 0x100))
						{
							//
							// Check for collision with this person by pretending they are
							// moving- not the prim.
							//

							SLONG x1, y1, z1;
							SLONG x2, y2, z2;

							x1 = x2 = p_person->WorldPos.X;
							y1 = y2 = p_person->WorldPos.Y;
							z1 = z2 = p_person->WorldPos.Z;

							if (slide_along_prim(
									dm->ObjectId,
									p_thing->WorldPos.X >> 8,
									p_thing->WorldPos.Y >> 8,
									p_thing->WorldPos.Z >> 8,
									dm->Angle,
									x1, y1, z1,
								   &x2,
								   &y2,
								   &z2,
									10,
									FALSE,
									FALSE))
							{

#ifndef PSX
								extern SLONG playing_level(const CBYTE *name);  // eway.cpp
								if (playing_level("botanicc.ucm") &&
#else
								if ((wad_level==19) &&
#endif
									(p_person->SubState == SUB_STATE_STANDING_JUMP_FORWARDS  ||
									 p_person->SubState == SUB_STATE_STANDING_JUMP_BACKWARDS ||
									 p_person->SubState == SUB_STATE_STANDING_JUMP           ||
									 p_person->SubState == SUB_STATE_RUNNING_JUMP))
								{
									if (WITHIN(p_person->WorldPos.Y >> 8, y_bot, y_top))
									{
										p_person->WorldPos.Y = p_thing->WorldPos.Y + (pi->maxy << 8);
									}
								}
								else
								{
									//
									// If this person is underneath the prim.
									//

									y_top = y_bot - 0x40;
									y_bot = y_bot - 0x80;

									if (WITHIN(p_person->WorldPos.Y >> 8, y_bot, y_top))
									{
										// PANEL_new_text(NULL, 1000, "Die Die Die!");
										
										set_face_thing(p_person, p_thing);

										p_person->Genus.Person->Health = 0;

										set_person_dead(
											p_person,
											NULL,
											PERSON_DEATH_TYPE_OTHER,
											FALSE,
 											0);
									}
								}
							}
						}
					}
				}
			}

			break;

		case PLAT_STATE_PAUSE:

			if (plat->counter <= millisecs)
			{
				//
				// Finished pausing...
				//

				plat->counter = 0;

				//
				// What now?
				// 

				switch(plat->state)
				{
					case PLAT_MOVE_STILL:
						
						//
						// We are doing nothing- but that is what we are meant to be doing.
						//

						break;

					case PLAT_MOVE_PATROL:
					case PLAT_MOVE_PATROL_RAND:

						if (plat->waypoint == NULL)
						{
							//
							// Look for the nearest waypoint of our colour and group.
							//

							plat->waypoint = EWAY_find_nearest_waypoint(
												p_thing->WorldPos.X >> 8,
												p_thing->WorldPos.Y >> 8,
												p_thing->WorldPos.Z >> 8,
												plat->colour,
												plat->group);
						}

						waypoint = EWAY_find_waypoint(
										plat->waypoint + 1,
										EWAY_DONT_CARE,
										plat->colour,
										plat->group,
										TRUE);

						if (waypoint == EWAY_NO_MATCH)
						{
							//
							// Couldn't find a waypoint! Wait a while before trying again.
							//

							plat->waypoint = NULL;
							plat->state    = PLAT_STATE_PAUSE;
							plat->counter  = 2 * 1000;
						}
						else
						{
							//
							// Start going to this waypoint.
							//

							plat->waypoint = waypoint;
							plat->state    = PLAT_STATE_GOTO;
							plat->speed    = 0;

							EWAY_get_position(
								waypoint,
							   &way_x,
							   &way_y,
							   &way_z);

							plat->flag &= ~(PLAT_FLAG_LOCK_X | PLAT_FLAG_LOCK_Y | PLAT_FLAG_LOCK_Z);

							if (plat->flag & PLAT_FLAG_LOCK_MOVE)
							{
								//
								// Which direction shall we lock?
								//

								dx = abs((p_thing->WorldPos.X >> 8) - way_x);
								dy = abs((p_thing->WorldPos.Y >> 8) - way_y);
								dz = abs((p_thing->WorldPos.Z >> 8) - way_z);

								     if (dx > dy && dx > dz) {plat->flag |= PLAT_FLAG_LOCK_X;}
								else if (dy > dz && dy > dx) {plat->flag |= PLAT_FLAG_LOCK_Y;}
								else                         {plat->flag |= PLAT_FLAG_LOCK_Z;}
							}
						}

						break;

					default:
						ASSERT(0);
						break;
				}
			}
			else
			{
				plat->counter -= millisecs;
			}

			break;

		case PLAT_STATE_STOP:
			break;

		default:
			ASSERT(0);
			break;
	}
}





UWORD PLAT_create(
		UBYTE colour,
		UBYTE group,
		UBYTE move,
		UBYTE flag,
		UBYTE speed,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z)
{
	SLONG mx;
	SLONG mz;

	SLONG x1;
	SLONG z1;

	SLONG x2;
	SLONG z2;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG score;

	SLONG   best_score;
	OB_Info best_oi;

	OB_Info  *oi;
	Thing    *p_thing;
	Plat     *plat;
	DrawMesh *dm;

	if (!WITHIN(PLAT_plat_upto, 1, PLAT_MAX_PLATS - 1))
	{
		//
		// No more plats left.
		//

		return NULL;
	}

	//
	// How far we look for obs.
	//

	#define PLAT_LOOK (0x300)

	x1 = world_x - PLAT_LOOK >> PAP_SHIFT_LO;
	z1 = world_z - PLAT_LOOK >> PAP_SHIFT_LO;

	x2 = world_x + PLAT_LOOK >> PAP_SHIFT_LO;
	z2 = world_z + PLAT_LOOK >> PAP_SHIFT_LO;

	SATURATE(x1, 0, PAP_SIZE_LO - 1);
	SATURATE(z1, 0, PAP_SIZE_LO - 1);

	SATURATE(x2, 0, PAP_SIZE_LO - 1);
	SATURATE(z2, 0, PAP_SIZE_LO - 1);
	
	//
	// Search for the best ob.
	//

	best_score = INFINITY;

	for (mx = x1; mx <= x2; mx++)
	for (mz = z1; mz <= z2; mz++)
	{
		for (oi = OB_find(mx,mz); oi->prim; oi++)
		{	
			dx = oi->x - world_x;
			dy = oi->y - world_y;
			dz = oi->z - world_z;

			score = abs(dx) + abs(dz) + (abs(dy) >> 2);

			if (score < best_score)
			{	
				best_score =  score;
				best_oi    = *oi;
			}
		}
	}

	if (best_score == INFINITY)
	{
		//
		// Couldn't find a nearby ob.
		//

		return NULL;
	}

	//
	// Create a new PLAT thing.
	//

	dm = alloc_draw_mesh();

	if (dm == NULL)
	{
		//
		// No more draw mesh structures.
		//
		ASSERT(0);

		return NULL;
	}

	p_thing = alloc_thing(CLASS_PLAT);

	if (p_thing == NULL)
	{
		//
		// No more things left!
		// 

		return NULL;
	}

	plat = &PLAT_plat[PLAT_plat_upto++];

	//
	// Now we have our separate places where we need to put data :(, we can
	// start filling out the values.
	//
	
	p_thing->Class      = CLASS_PLAT;
	p_thing->WorldPos.X = best_oi.x << 8;
	p_thing->WorldPos.Y = best_oi.y << 8;
	p_thing->WorldPos.Z = best_oi.z << 8;
	p_thing->Flags      = 0;
	p_thing->State      = STATE_NORMAL;
	p_thing->StateFn    = PLAT_process;
	p_thing->DrawType   = DT_MESH;
	p_thing->Draw.Mesh  = dm;
	p_thing->Genus.Plat = plat;
	
	dm->Angle    = best_oi.yaw;
	dm->Tilt     = 0;
	dm->Roll     = 0;
	dm->ObjectId = best_oi.prim;
	dm->Cache    = 0;
	dm->Hm       = 0;

	plat->used     = TRUE;
	plat->colour   = colour;
	plat->group    = group;
	plat->move     = move;
	plat->state    = PLAT_STATE_NONE;
	plat->counter  = 0;
	plat->waypoint = 0;
	plat->speed    = 0;
	plat->wspeed   = speed;
	plat->flag     = flag;
	
	//
	// Put down on the mapwho.
	//

	add_thing_to_map(p_thing);

	//
	// Remove this ob.
	//

	OB_remove(&best_oi);

	//
	// Create a movable walkable face for this moving platform.
	//

	WMOVE_create(p_thing);


	return THING_NUMBER(p_thing);
}


