//
// Bats/gargoles that circle an area and swoop to attack the player.
//

//
// Across a triangle...
//
//
//    dx     x1*v2 - x2*v1
//   ---- = ---------------
//    du     u1*v2 - u2*v1
//


#include "game.h"
#include "bat.h"
#include "animate.h"
#include "statedef.h"
#include "sound.h"
#include "eway.h"
#include "psystem.h"
#include "poly.h"
#include "mav.h"
#include "wand.h"
#include "spark.h"
#include "pcom.h"
#include "memory.h"
#include "night.h"


//
// The timer used by the bats
//

#define BAT_TICKS_PER_SECOND 160

//
// The flags.
//

#define BAT_FLAG_ATTACKED	(1 << 0)
#define BAT_FLAG_SYNC_FX 	(1 << 1)
#define BAT_FLAG_SYNC_FX2	(1 << 2)


//
// The different states a bat can be in.
//

#define BAT_STATE_IDLE			  0
#define BAT_STATE_GOTO			  1
#define BAT_STATE_CIRCLE		  2
#define BAT_STATE_ATTACK		  3
#define BAT_STATE_DYING			  4
#define BAT_STATE_DEAD			  5
#define BAT_STATE_GROUND		  6
#define BAT_STATE_RECOIL		  7
#define BAT_STATE_BALROG_WANDER   8
#define BAT_STATE_BALROG_ROAR     9
#define BAT_STATE_BALROG_FOLLOW	  10
#define BAT_STATE_BALROG_CHARGE   11
#define BAT_STATE_BALROG_SWIPE    12
#define BAT_STATE_BALROG_STOMP    13
#define BAT_STATE_BALROG_FIREBALL 14
#define BAT_STATE_BALROG_IDLE	  15
#define BAT_STATE_BANE_IDLE		  16
#define BAT_STATE_BANE_ATTACK     17
#define BAT_STATE_BANE_START      18
#define BAT_STATE_NUMBER          19

#ifndef PSX

CBYTE *BAT_state_name[BAT_STATE_NUMBER] =
{
	"Idle",
	"Goto",
	"Circle",
	"Attack",
	"Dying",
	"Dead",
	"Ground",
	"Recoil",
	"Balrog wander",
	"Balrog roar",
	"Balrog follow",
	"Balrog charge",
	"Balrog swipe",
	"Balrog stomp",
	"Balrog fireball",
	"Balrog idle",
	"Bane idle",
	"Bane attack",
	"Bane start"
};

#endif

#define BAT_SUBSTATE_NONE			0
#define BAT_SUBSTATE_CIRCLE_HOME	1
#define BAT_SUBSTATE_CIRCLE_TARGET	2
#define BAT_SUBSTATE_CIRCLE_WANT	3
#define BAT_SUBSTATE_GROUND_WAIT	4
#define BAT_SUBSTATE_GROUND_WAKE_UP	5
#define BAT_SUBSTATE_GROUND_FLY_UP	6
#define BAT_SUBSTATE_DEAD_INITIAL	7
#define BAT_SUBSTATE_DEAD_LOOP		8
#define BAT_SUBSTATE_DEAD_FINAL		9
#define BAT_SUBSTATE_YOMP_START		10
#define BAT_SUBSTATE_YOMP_MIDDLE	11
#define BAT_SUBSTATE_YOMP_END		12
#define BAT_SUBSTATE_FIREBALL_TURN	13
#define BAT_SUBSTATE_FIREBALL_FIRE	14


//
// The people bane is summoning.
//

#define BAT_SUMMON_NUM_BODIES 4

UWORD BAT_summon[BAT_SUMMON_NUM_BODIES];

void BAT_find_summon_people(Thing *p_thing)
{
	SLONG i;
	SLONG num;
	SLONG bodies;

	//
	// Look around for the bodies...
	//

	num = THING_find_sphere(
			p_thing->WorldPos.X >> 8,
			p_thing->WorldPos.Y >> 8,
			p_thing->WorldPos.Z >> 8,
			0x800,
			THING_array,
			THING_ARRAY_SIZE,
			1 << CLASS_PERSON);
	
	//
	// Clear out the body array.
	//

	memset(BAT_summon, 0, sizeof(BAT_summon));
	
	bodies = 0;

	for (i = 0; i < num; i++)
	{
		Thing *p_found = TO_THING(THING_array[i]);

		ASSERT(p_found->Class == CLASS_PERSON);

		if (p_found->Genus.Person->pcom_ai == PCOM_AI_NONE ||
			p_found->Genus.Person->pcom_ai == PCOM_AI_SUICIDE)
		{
			if (p_found->Genus.Person->PlayerID == 0)
			{
				//
				// This person will do!
				//

				ASSERT(WITHIN(bodies, 0, BAT_SUMMON_NUM_BODIES - 1));

				BAT_summon[bodies] = THING_array[i];

				bodies += 1;

				if (bodies == BAT_SUMMON_NUM_BODIES)
				{
					//
					// Found all our bodies!
					//

					break;
				}
			}
		}
	}
}

//
// Creates sparks from the given bat to the summoning people.
//

void BAT_process_bane_sparks(Thing *p_thing)
{
	SLONG i;

	static UWORD last = 0;

	last += 16 * TICK_RATIO >> TICK_SHIFT;

	if (last > 16 * 20 * 5)
	{
		last = 0;

		for (i = 0; i < BAT_SUMMON_NUM_BODIES; i++)
		{
			if (BAT_summon[i])
			{
				Thing *p_summon = TO_THING(BAT_summon[i]);

				SPARK_Pinfo p1;
				SPARK_Pinfo p2;
				
				p1.type   = SPARK_TYPE_LIMB;
				p1.flag   = 0;
				p1.person = THING_NUMBER(p_thing);
				p1.limb   = 0;

				p2.type   = SPARK_TYPE_LIMB;
				p2.flag   = 0;
				p2.person = THING_NUMBER(p_summon);
				p2.limb   = SUB_OBJECT_PELVIS;

				SPARK_create(
					&p1,
					&p2,
					255);
			}
		}
	}
}

#ifndef PSX
//
// Initialise the bats.
//

void BAT_init()
{
	memset(the_game.Bats, 0, sizeof(Bat) * BAT_MAX_BATS);

	BAT_COUNT = 0;
}
#endif


//
// Sets a BAT anim...
//

#define BAT_ANIM_BAT_FLY				1
#define BAT_ANIM_BAT_DIE				2
#define BAT_ANIM_GARGOYLE_WAIT			1
#define BAT_ANIM_GARGOYLE_WAKE_UP		15
#define BAT_ANIM_GARGOYLE_FLY_UP		3
#define BAT_ANIM_GARGOYLE_FLY			2
#define BAT_ANIM_GARGOYLE_FLY_FORWARDS	12
#define BAT_ANIM_GARGOYLE_ATTACK		4
#define BAT_ANIM_GARGOYLE_TAKE_HIT		14
#define BAT_ANIM_GARGOYLE_START_FALL	16
#define BAT_ANIM_GARGOYLE_FALL_LOOP		17
#define BAT_ANIM_GARGOYLE_HIT_GROUND	18
#define BAT_ANIM_BALROG_YOMP			2
#define BAT_ANIM_BALROG_IDLE			3
#define BAT_ANIM_BALROG_SWIPE			4
#define BAT_ANIM_BALROG_YOMP_START		5
#define BAT_ANIM_BALROG_YOMP_END		6
#define BAT_ANIM_BALROG_TURN			8
#define BAT_ANIM_BALROG_ROAR			9
#define BAT_ANIM_BALROG_STOMP			10
#define BAT_ANIM_BALROG_TAKE_HIT		11
#define BAT_ANIM_BALROG_DIE				12



#define BAT_ANIM_BANE_IDLE				2
#define BAT_ANIM_BANE_ATTACK			3

#define BAT_ANIM_GENERIC_FLY			(-1)	// An anim that changes depending on the type of bat!
#define BAT_ANIM_GENERIC_TAKE_HIT		(-2)

void BAT_set_anim(Thing *p_thing, SLONG anim)
{
	if (anim < 0)
	{
		static UBYTE generic_bat_anim[2][2] =
		{
			{BAT_ANIM_BAT_FLY,      BAT_ANIM_BAT_FLY},
			{BAT_ANIM_GARGOYLE_FLY, BAT_ANIM_GARGOYLE_TAKE_HIT}
		};

		ASSERT(WITHIN(p_thing->Genus.Bat->type - 1, 0, 1));
		ASSERT(WITHIN(-anim - 1, 0, 1));

		anim = generic_bat_anim[p_thing->Genus.Bat->type - 1][-anim - 1];
	}

	DrawTween *dt = p_thing->Draw.Tweened;

	if (dt->CurrentAnim == anim)
	{
		return;
	}

	dt->AnimTween	 =	0;
	dt->QueuedFrame	 =	NULL;
	dt->TheChunk	 = &anim_chunk[p_thing->Genus.Bat->type];
	dt->CurrentFrame =  anim_chunk[p_thing->Genus.Bat->type].AnimList[anim];
	dt->NextFrame	 =  dt->CurrentFrame->NextFrame;
	dt->CurrentAnim	 =  anim;
	dt->FrameIndex	 =  0;

	if (dt->NextFrame == NULL)
	{
		dt->NextFrame = dt->CurrentFrame->NextFrame;
	}

	p_thing->Genus.Bat->flag&=~(BAT_FLAG_SYNC_FX|BAT_FLAG_SYNC_FX2);
}



//
// Animates the bat. Returns TRUE if the current anim is over.
//

SLONG BAT_animate(Thing *p_thing)
{
	SLONG ret = FALSE;
	SLONG tween_step;

	DrawTween *dt = p_thing->Draw.Tweened;

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

		SLONG advance_keyframe(DrawTween *draw_info);

		ret |= advance_keyframe(dt);
	}

	return ret;
}


//
// Makes a bat turn towards his target. Returns the angle difference.
//

SLONG BAT_turn_to_target(Thing *p_thing)
{
	SLONG dx;
	SLONG dz;

	SLONG wangle;
	SLONG dangle;

	SLONG turn;

	Thing *p_target;
	Bat   *p_bat;

	ASSERT(p_thing->Class == CLASS_BAT);

	p_bat    = p_thing->Genus.Bat;
	p_target = TO_THING(p_bat->target);

	ASSERT(p_bat->target);

	/*

	AENG_world_line(
		p_thing->WorldPos.X >> 8,
		p_thing->WorldPos.Y >> 8,
		p_thing->WorldPos.Z >> 8,
		32,
		0xff0000,
		p_target->WorldPos.X >> 8,
		p_target->WorldPos.Y >> 8,
		p_target->WorldPos.Z >> 8,
		16,
		0x00ff00,
		TRUE);

	*/

	dx = p_target->WorldPos.X - p_thing->WorldPos.X >> 8;
	dz = p_target->WorldPos.Z - p_thing->WorldPos.Z >> 8;

	wangle = calc_angle(dx,dz);

	if (p_bat->type == BAT_TYPE_BALROG)
	{
		//
		// Oh dear! He's back-to-front!
		//

		wangle += 1024;
	}

	dangle = wangle - p_thing->Draw.Tweened->Angle;

	if (dangle > +1024) {dangle -= 2048;}
	if (dangle < -1024) {dangle += 2048;}

	turn = dangle >> 4;

	SATURATE(turn, -20, +20);

	p_thing->Draw.Tweened->Angle += turn;
	p_thing->Draw.Tweened->Angle &= 2047;

	return dangle;
}


//
// Makes the bat turn towards a given place. Returns the relative angle.
//

SLONG BAT_turn_to_place(Thing *p_thing, SLONG world_x, SLONG world_z)
{
	SLONG dx;
	SLONG dz;

	SLONG wangle;
	SLONG dangle;

	SLONG turn;

	Bat *p_bat;

	ASSERT(p_thing->Class == CLASS_BAT);

	p_bat = p_thing->Genus.Bat;
/*
	AENG_world_line(
		p_thing->WorldPos.X >> 8,
		p_thing->WorldPos.Y >> 8,
		p_thing->WorldPos.Z >> 8,
		32,
		0xff0000,
		world_x,
		PAP_calc_map_height_at(world_x,world_z),
		world_z,
		16,
		0xffffff,
		TRUE);
*/

	dx = world_x - (p_thing->WorldPos.X >> 8);
	dz = world_z - (p_thing->WorldPos.Z >> 8);

	wangle = calc_angle(dx,dz);

	if (p_bat->type == BAT_TYPE_BALROG)
	{
		//
		// Oh dear! He's back-to-front!
		//

		wangle += 1024;
	}

	dangle = wangle - p_thing->Draw.Tweened->Angle;

	if (dangle > +1024) {dangle -= 2048;}
	if (dangle < -1024) {dangle += 2048;}

	turn = dangle >> 4;

	SATURATE(turn, -20, +20);

	p_thing->Draw.Tweened->Angle += turn;
	p_thing->Draw.Tweened->Angle &= 2047;

	return dangle;
}


//
// Makes the bat emit a fireball.
//

void BAT_emit_fireball(Thing *p_thing)
{
	ASSERT(p_thing->Class == CLASS_BAT);

	Bat *p_bat = p_thing->Genus.Bat;

	ASSERT(p_bat->target);

	Thing *p_target = TO_THING(p_bat->target);

	SLONG dx;
	SLONG dy;
	SLONG dz;

	dx = p_target->WorldPos.X          - p_thing->WorldPos.X;
	dy = p_target->WorldPos.Y + 0x3000 - p_thing->WorldPos.Y;
	dz = p_target->WorldPos.Z          - p_thing->WorldPos.Z;

	SLONG dist = (QDIST3(abs(dx),abs(dy),abs(dz)) >> 8) + 1;

	dx = 0x40 * dx / dist;
	dy = 0x40 * dy / dist;
	dz = 0x40 * dz / dist;

	dy += Random() & 0x3ff;

	//
	// Fire a fireball!
	//

	PARTICLE_Add(
		p_thing->WorldPos.X,
		p_thing->WorldPos.Y + 0x5000,
		p_thing->WorldPos.Z,
		dx,
		dy,
		dz,
		POLY_PAGE_METEOR,
		2 + ((Random() & 0x3) << 2),
		0xffffffff,
		PFLAG_SPRITEANI | PFLAG_SPRITELOOP  | PFLAG_EXPLODE_ON_IMPACT | PFLAG_GRAVITY | PFLAG_LEAVE_TRAIL,
		100,
		160,
		1,
		1,
		1);

	MFX_play_thing(THING_NUMBER(p_thing),S_BALROG_FIREBALL,0,p_thing);
}


//
// Chooses a new state for the bat to be in.
//

void BAT_change_state(Thing *p_thing)
{
	ASSERT(p_thing->Class == CLASS_BAT);

	SLONG want_x;
	SLONG want_y;
	SLONG want_z;

	SLONG new_state;
	SLONG old_target;

	Bat   *p_bat = p_thing->Genus.Bat;
	Thing *darci;

	old_target = p_bat->target;

	SLONG dangle = 0;

	if (p_bat->type == BAT_TYPE_BALROG)
	{
		if (p_bat->target)
		{
			Thing *p_target = TO_THING(p_bat->target);

			extern SLONG is_person_dead(Thing *p_person);

			if (p_target->Class == CLASS_PERSON && is_person_dead(p_target))
			{
				//
				// Assume the Balrog killed this person.
				//

				extern UBYTE *EWAY_counter;

				if (EWAY_counter[8] < 255)
				{
					EWAY_counter[8] += 1;
				}
			}
		}

		//
		// Look for a target.
		//

		SLONG i;

		SLONG dx;
		SLONG dy;
		SLONG dz;
		SLONG dist;
		SLONG score;

		SLONG best_score = INFINITY;
		SLONG best_dist  = 0;
		SLONG best_index = NULL;

		Thing *p_found;

		//
		// Find all the people near us.
		//

		SLONG found_num = THING_find_sphere(
							p_thing->WorldPos.X >> 8,
							p_thing->WorldPos.Y >> 8,
							p_thing->WorldPos.Z >> 8,
							0xa00,
							THING_array,
							THING_ARRAY_SIZE,
							(1 << CLASS_PERSON) | (1 << CLASS_VEHICLE));

		//
		// Who is the best person to bully?
		//

		for (i = 0; i < found_num; i++)
		{
			p_found = TO_THING(THING_array[i]);

			if (p_found == p_thing)
			{
				//
				// Ignore yourself!
				//

				continue;
			}

			if (p_found->State == STATE_DEAD ||
				p_found->State == STATE_DYING)
			{
				//
				// Ignore dead people / vehicles.
				//

				continue;
			}

			if (p_found->Class == CLASS_PERSON)
			{
				extern SLONG is_person_ko(Thing *p_person);

				if (is_person_ko(p_found))
				{
					//
					// Knocked out people aren't fair game!
					//

					continue;
				}
			}

			//
			// Make sure we can get to the person.
			//

			MAV_do(
				p_thing->WorldPos.X >> 16,
				p_thing->WorldPos.Z >> 16,
				p_found->WorldPos.X >> 16,
				p_found->WorldPos.Z >> 16,
				MAV_CAPS_GOTO);

			if (MAV_do_found_dest)
			{
				//
				// Score this person/vehicle. Lower scores are better.
				//

				dx = abs(p_thing->WorldPos.X - p_found->WorldPos.X);
				dy = abs(p_thing->WorldPos.Y - p_found->WorldPos.Y);
				dz = abs(p_thing->WorldPos.Z - p_found->WorldPos.Z);

				dist = dx + dy + dy + dz >> 8;	// Difference in y is more important...

				score = dist;

				if (p_bat->target == THING_array[i])
				{
					//
					// Likes to keep the same target.
					//

					score >>= 1;
				}

				if (p_found->Genus.Person->PlayerID)
				{
					//
					// More likely to attack players.
					//

					score >>= 1;
				}


				if (best_score > score)
				{
					best_score = score;
					best_index = THING_array[i];
					best_dist  = dist;
				}
			}
		}

		p_bat->target = best_index;

		//
		// What state should the balrog be in now?
		//

		if (p_bat->target)
		{
			dangle = abs(BAT_turn_to_target(p_thing));

			if (p_bat->target != old_target)
			{
				//
				// Roar when you find a new target!
				// 

				new_state = BAT_STATE_BALROG_ROAR;
			}
			else
			{
				if (best_dist < 0x200 && dangle < 256)
				{
					new_state = BAT_STATE_BALROG_SWIPE;
				}
				else
				if (best_dist < 0x400)
				{
					if (Random() & 0x40)
					{
						new_state = BAT_STATE_BALROG_STOMP;
					}
					else
					{
						new_state = BAT_STATE_BALROG_FOLLOW;
					}
				}
				else
				{
					if (Random() & 0x20)
					{
						if ((Random() & 0x3) == 0)
						{
							new_state = BAT_STATE_BALROG_CHARGE;
						}
						else
						{
							new_state = BAT_STATE_BALROG_FIREBALL;
						}
					}
					else
					{
						new_state = BAT_STATE_BALROG_FOLLOW;
					}
				}
			}
		}
		else
		{
			if (p_bat->state == BAT_STATE_BALROG_FIREBALL ||
				p_bat->state == BAT_STATE_BALROG_CHARGE   ||
				p_bat->state == BAT_STATE_BALROG_SWIPE    ||
				p_bat->state == BAT_STATE_BALROG_STOMP)
			{
				//
				// Roar after a successful attack!
				// 

				new_state = BAT_STATE_BALROG_ROAR;
			}
			else
			if ((Random() & 0x3) == 0)
			{
				if (Random() & 0x2)
				{
					new_state = BAT_STATE_BALROG_ROAR;
				}
				else
				{
					new_state = BAT_STATE_BALROG_IDLE;
				}
			}
			else
			{
				//
				// Wander the city looking for targets.
				//

				new_state = BAT_STATE_BALROG_WANDER;
			}
		}
	}
	else
	if (p_bat->type == BAT_TYPE_BANE)
	{
		p_bat->target = NULL;

		if (BAT_summon[0] == NULL)
		{
			new_state = BAT_STATE_BANE_START;
		}
		else
		{
			if ((Random() & 0x3) == 0)
			{
				new_state = BAT_STATE_BANE_ATTACK;
			}
			else
			{
				new_state = BAT_STATE_BANE_IDLE;
			}
		}
	}
#ifndef PSX
	else
	{
		//
		// Is the player close by?
		//

		darci = NET_PERSON(0);

		if (THING_dist_between(darci, p_thing) < 0x600)
		{
			p_bat->target = THING_NUMBER(darci);
		}
		else
		{
			p_bat->target = NULL;
		}

		//
		// What state should be in now?
		//

		SLONG dx = p_thing->WorldPos.X - ((p_bat->home_x << 16) + 0x8000);
		SLONG dz = p_thing->WorldPos.Z - ((p_bat->home_z << 16) + 0x8000);

		if (abs(dx) > 0x40000 ||
			abs(dz) > 0x40000)
		{
			//
			// Too far from home...
			//

			new_state = BAT_STATE_GOTO;
		}
		else
		{
			new_state = Random() % 3;

			if (p_bat->type   == BAT_TYPE_GARGOYLE &&
				new_state     == BAT_STATE_CIRCLE  &&
				p_bat->target != NULL)
			{
				new_state = BAT_STATE_ATTACK;
			}
		}
	}
#endif
	//
	// Initialise the new state.
	//


	p_bat->state =  new_state;
	p_bat->flag &= ~BAT_FLAG_ATTACKED;

	switch(new_state)
	{
#ifndef PSX
		case BAT_STATE_IDLE:

			p_bat->timer = BAT_TICKS_PER_SECOND * (1 + (Random() & 0x1));

			BAT_set_anim(p_thing, BAT_ANIM_GENERIC_FLY);

			break;

		case BAT_STATE_GOTO:

			want_x = p_bat->home_x << 8;
			want_z = p_bat->home_z << 8;

			want_x += Random() & 0x3ff;
			want_z += Random() & 0x3ff;

			want_x -= 0x1ff;
			want_z -= 0x1ff;

			want_y  = PAP_calc_map_height_at(want_x, want_z) + 0x80 + (Random() & 0x7f);

			p_bat->want_x = want_x;
			p_bat->want_y = want_y;
			p_bat->want_z = want_z;

			p_bat->timer = BAT_TICKS_PER_SECOND * (2 + (Random() & 0x1));

			BAT_set_anim(p_thing, BAT_ANIM_GENERIC_FLY);

			break;

		case BAT_STATE_CIRCLE:

			if (p_bat->target)
			{
				p_bat->substate = BAT_SUBSTATE_CIRCLE_TARGET;
			}
			else
			{
				p_bat->substate = BAT_SUBSTATE_CIRCLE_HOME;
			}

			p_bat->timer = BAT_TICKS_PER_SECOND * (3 + (Random() & 0x3));

			BAT_set_anim(p_thing, BAT_ANIM_GENERIC_FLY);

			break;

		case BAT_STATE_ATTACK:

			ASSERT(p_bat->type == BAT_TYPE_GARGOYLE);

			BAT_set_anim(p_thing, BAT_ANIM_GARGOYLE_ATTACK);

			break;
#endif
		case BAT_STATE_BALROG_WANDER:

			{
				SLONG want_x;
				SLONG want_z;

				MAV_Action ma;

				WAND_get_next_place(
					p_thing,
				   &want_x,
				   &want_z);

				ma = MAV_do(
						p_thing->WorldPos.X >> 16,
						p_thing->WorldPos.Z >> 16,
						want_x >> 8,
						want_z >> 8,
						MAV_CAPS_GOTO);

				p_bat->want_x = (ma.dest_x << 8) + 0x80;
				p_bat->want_z = (ma.dest_z << 8) + 0x80;

				p_bat->timer = BAT_TICKS_PER_SECOND * (2 + (Random() & 0x1));

				BAT_set_anim(p_thing, BAT_ANIM_BALROG_YOMP_START);

				p_bat->substate = BAT_SUBSTATE_YOMP_START;
			}

			break;

		case BAT_STATE_BALROG_ROAR:

			//
			// We don't have a roar anim! Do a swipe instead...
			//

			BAT_set_anim(p_thing, BAT_ANIM_BALROG_ROAR);

			MFX_play_thing(THING_NUMBER(p_thing),S_BALROG_ROAR,0,p_thing);

			p_bat->dx = 0;
			p_bat->dz = 0;

			break;

		case BAT_STATE_BALROG_CHARGE:
		case BAT_STATE_BALROG_FOLLOW:

			ASSERT(p_bat->target);

			BAT_set_anim(p_thing, BAT_ANIM_BALROG_YOMP_START);

			p_bat->timer = BAT_TICKS_PER_SECOND * (2 + (Random() & 0x1));

			p_bat->substate = BAT_SUBSTATE_YOMP_START;

			break;

		case BAT_STATE_BALROG_SWIPE:
			BAT_set_anim(p_thing, BAT_ANIM_BALROG_SWIPE);
			MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_GROWL_START,S_BALROG_GROWL_END),0,p_thing);
			p_bat->dx = 0;
			p_bat->dz = 0;
			break;

		case BAT_STATE_BALROG_STOMP:
			BAT_set_anim(p_thing, BAT_ANIM_BALROG_STOMP);
			MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_GROWL_START,S_BALROG_GROWL_END),0,p_thing);
			p_bat->dx = 0;
			p_bat->dz = 0;
			break;

		case BAT_STATE_BALROG_FIREBALL:

			if (dangle < 256)
			{
				BAT_set_anim(p_thing, BAT_ANIM_BALROG_SWIPE);
					
				p_bat->substate = BAT_SUBSTATE_FIREBALL_FIRE;
			    MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_GROWL_START,S_BALROG_GROWL_END),0,p_thing);
			}
			else
			{
				BAT_set_anim(p_thing, BAT_ANIM_BALROG_TURN);

				p_bat->substate = BAT_SUBSTATE_FIREBALL_TURN;
			}
				
			p_bat->dx = 0;
			p_bat->dz = 0;

			break;

		case BAT_STATE_BALROG_IDLE:
			BAT_set_anim(p_thing, BAT_ANIM_BALROG_IDLE);
			p_bat->timer = BAT_TICKS_PER_SECOND * (2 + (Random() & 0x3));
			p_bat->dx = 0;
			p_bat->dz = 0;
			break;

		case BAT_STATE_BANE_IDLE:
			BAT_set_anim(p_thing, BAT_ANIM_BANE_IDLE);
			p_bat->timer = BAT_TICKS_PER_SECOND * (3 + (Random() & 0x3));
			p_bat->dx    = 0;
			p_bat->dz    = 0;
			break;

		case BAT_STATE_BANE_ATTACK:
			BAT_set_anim(p_thing, BAT_ANIM_BANE_ATTACK);
			p_bat->dx = 0;
			p_bat->dz = 0;
			break;

		case BAT_STATE_BANE_START:
			
			//
			// Look for the people to summon.
			//

			BAT_find_summon_people(p_thing);

			//
			// Make them start floating.
			//
	
			{
				SLONG i;

				for (i = 0; i < BAT_SUMMON_NUM_BODIES; i++)
				{
					if (BAT_summon[i])
					{
						Thing *p_summon = TO_THING(BAT_summon[i]);

						set_person_float_up(p_summon);
					}
				}
			}

			BAT_set_anim(p_thing, BAT_ANIM_BANE_IDLE);


			// make some funky noises

			MFX_play_thing(THING_NUMBER(p_thing),S_RECKONING_LOOP,MFX_LOOPED,p_thing);

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Slides a Balrog around buildings.
//

#define BAT_BALROG_WIDTH (0x3000)

void BAT_balrog_slide_along(
		SLONG  x1, SLONG  z1,
		SLONG *x2, SLONG *z2)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;
	ULONG collide;

	SLONG mx = *x2 >> 16;
	SLONG mz = *z2 >> 16;

	ASSERT(WITHIN(mx, 1, PAP_SIZE_HI - 2));
	ASSERT(WITHIN(mz, 1, PAP_SIZE_HI - 2));

	//
	// Where do we collide?
	// 

	#define BAT_COLLIDE_XS (1 << 0)
	#define BAT_COLLIDE_XL (1 << 1)
	#define BAT_COLLIDE_ZS (1 << 2)
	#define BAT_COLLIDE_ZL (1 << 3)
	#define BAT_COLLIDE_SS (1 << 4)
	#define BAT_COLLIDE_LS (1 << 5)
	#define BAT_COLLIDE_SL (1 << 6)
	#define BAT_COLLIDE_LL (1 << 7)

	MAV_Opt mo = MAV_opt[MAV_NAV(mx,mz)];

	//
	// The mapsquare edges.
	// 

	collide = 0;

	if (!(mo.opt[MAV_DIR_XS] & (MAV_CAPS_GOTO))) {collide |= BAT_COLLIDE_XS;}
	if (!(mo.opt[MAV_DIR_XL] & (MAV_CAPS_GOTO))) {collide |= BAT_COLLIDE_XL;}
	if (!(mo.opt[MAV_DIR_ZS] & (MAV_CAPS_GOTO))) {collide |= BAT_COLLIDE_ZS;}
	if (!(mo.opt[MAV_DIR_ZL] & (MAV_CAPS_GOTO))) {collide |= BAT_COLLIDE_ZL;}

	if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
	{
		//
		// The Balrog is on a building! :( Turn off MAPSQUARE collision.
		//
	}
	else
	{	
		if (PAP_2HI(mx + 1, mz + 0).Flags & PAP_FLAG_HIDDEN) {collide |= BAT_COLLIDE_XL;}
		if (PAP_2HI(mx - 1, mz + 0).Flags & PAP_FLAG_HIDDEN) {collide |= BAT_COLLIDE_XS;}
		if (PAP_2HI(mx + 0, mz + 1).Flags & PAP_FLAG_HIDDEN) {collide |= BAT_COLLIDE_ZL;}
		if (PAP_2HI(mx + 0, mz - 1).Flags & PAP_FLAG_HIDDEN) {collide |= BAT_COLLIDE_ZS;}
	}

	/*

	if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx - 1, mz)) > 1) {collide |= BAT_COLLIDE_XS;}
	if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx + 1, mz)) > 1) {collide |= BAT_COLLIDE_XL;}
	if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx, mz - 1)) > 1) {collide |= BAT_COLLIDE_ZS;}
	if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx, mz + 1)) > 1) {collide |= BAT_COLLIDE_ZL;}

	if (!(collide & (BAT_COLLIDE_XS | BAT_COLLIDE_ZS))) {if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx - 1, mz - 1)) > 1) {collide |= BAT_COLLIDE_SS;}}
	if (!(collide & (BAT_COLLIDE_XL | BAT_COLLIDE_ZS))) {if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx + 1, mz - 1)) > 1) {collide |= BAT_COLLIDE_LS;}}
	if (!(collide & (BAT_COLLIDE_XS | BAT_COLLIDE_ZL))) {if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx - 1, mz + 1)) > 1) {collide |= BAT_COLLIDE_SL;}}
	if (!(collide & (BAT_COLLIDE_XL | BAT_COLLIDE_ZL))) {if (abs(MAVHEIGHT(mx,mz) - MAVHEIGHT(mx + 1, mz + 1)) > 1) {collide |= BAT_COLLIDE_LL;}}

	*/

	//
	// Edges...
	// 

	if (collide & BAT_COLLIDE_XS)
	{
		//
		// Keep out of the (xsmall) of this square.
		//

		if ((*x2 & 0xffff) < BAT_BALROG_WIDTH)
		{
			*x2 &= ~0xffff;
			*x2 |=  BAT_BALROG_WIDTH;
		}
	}

	if (collide & BAT_COLLIDE_XL)
	{
		//
		// Keep out of the (xlarge) of this square.
		//

		if ((*x2 & 0xffff) > 0x10000 - BAT_BALROG_WIDTH)
		{
			*x2 &= ~0xffff;
			*x2 |=  0x10000 - BAT_BALROG_WIDTH;
		}
	}

	if (collide & BAT_COLLIDE_ZS)
	{
		//
		// Keep out of the (zsmall) of this square.
		//

		if ((*z2 & 0xffff) < BAT_BALROG_WIDTH)
		{
			*z2 &= ~0xffff;
			*z2 |=  BAT_BALROG_WIDTH;
		}
	}

	if (collide & BAT_COLLIDE_ZL)
	{
		//
		// Keep out of the (zlarge) of this square.
		//

		if ((*z2 & 0xffff) > 0x10000 - BAT_BALROG_WIDTH)
		{
			*z2 &= ~0xffff;
			*z2 |=  0x10000 - BAT_BALROG_WIDTH;
		}
	}

	//
	// Corners...
	// 

	if (collide & BAT_COLLIDE_SS)
	{
		dx = *x2 & 0xffff;
		dz = *z2 & 0xffff;

		dist = QDIST2(dx,dz) + 1;

		if (dist < BAT_BALROG_WIDTH)
		{
			dx = dx * BAT_BALROG_WIDTH / dist;
			dz = dz * BAT_BALROG_WIDTH / dist;
		}

		*x2 &= ~0xffff;
		*z2 &= ~0xffff;

		*x2 |= dx;
		*z2 |= dz;
	}

	if (collide & BAT_COLLIDE_LS)
	{
		dx = *x2 & 0xffff;
		dz = *z2 & 0xffff;

		dx = 0x10000 - dx;

		dist = QDIST2(dx,dz) + 1;

		if (dist < BAT_BALROG_WIDTH)
		{
			dx = dx * BAT_BALROG_WIDTH / dist;
			dz = dz * BAT_BALROG_WIDTH / dist;
		}

		dx = 0x10000 - dx;

		*x2 &= ~0xffff;
		*z2 &= ~0xffff;

		*x2 |= dx;
		*z2 |= dz;
	}

	if (collide & BAT_COLLIDE_SL)
	{
		dx = *x2 & 0xffff;
		dz = *z2 & 0xffff;

		dz = 0x10000 - dz;

		dist = QDIST2(dx,dz) + 1;

		if (dist < BAT_BALROG_WIDTH)
		{
			dx = dx * BAT_BALROG_WIDTH / dist;
			dz = dz * BAT_BALROG_WIDTH / dist;
		}

		dz = 0x10000 - dz;

		*x2 &= ~0xffff;
		*z2 &= ~0xffff;

		*x2 |= dx;
		*z2 |= dz;
	}

	if (collide & BAT_COLLIDE_LS)
	{
		dx = *x2 & 0xffff;
		dz = *z2 & 0xffff;

		dx = 0x10000 - dx;
		dz = 0x10000 - dz;

		dist = QDIST2(dx,dz) + 1;

		if (dist < BAT_BALROG_WIDTH)
		{
			dx = dx * BAT_BALROG_WIDTH / dist;
			dz = dz * BAT_BALROG_WIDTH / dist;
		}

		dz = 0x10000 - dz;

		*x2 &= ~0xffff;
		*z2 &= ~0xffff;

		*x2 |= dx;
		*z2 |= dz;
	}
}


//
// Processes a bat thing.
//

void BAT_normal(Thing *p_thing)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;

	SLONG ddx;
	SLONG ddz;

	SLONG want_x;
	SLONG want_y;
	SLONG want_z;

	SLONG want_dx;
	SLONG want_dy;
	SLONG want_dz;

	SLONG wangle;
	SLONG dangle;

	SLONG goto_x;
	SLONG goto_y;
	SLONG goto_z;

	SLONG ground;

	SLONG end;

	;;	 ;;											;;
	;;	 ;;											;;
	;;	 ;;											 ;;
	;;;;;;;	;;;;;  ;;	 ;;			;;	  ;	  ;;;;	 ;;
	;;	 ;;	;;		;;	  ;;;;;		;;;; ;;;  ;;	 ;;;
	;;	 ;;	;;;;;;	;;	  ;; ;;;	;; ;;; ;;  ;;;;	 ;;;
	;;  ;;	;;	 ;	;;	  ;;;;;		;;	   ;; ;;	 ;;
		;; ;;;;;	;;;;; ;;		;	  ;;  ;;;;;	  ;
			;   ;		  ;				  ;	;;;	  ;
											 ;		 ;;
				   ; ;;;;;;;;;;;;;;;;						;;
	; ; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	;  ; ;  ;;
				;;;;;;;;;;;;;;;;;;;;;;;;;;;;; ;			  ;;;;;;;;; ;;
								; ; ;;;;;; ;					;; ;; ;
	GameCoord newpos;				 ;;;;;;;					 ;;;	;
									   ;
	SLONG ticks = (BAT_TICKS_PER_SECOND / 20) * TICK_RATIO >> TICK_SHIFT;
										   
	ASSERT(p_thing->Class == CLASS_BAT);  ;;;;;					  ;; ;;
										  ;;;					  ;;;;
	Bat *p_bat = p_thing->Genus.Bat;								;
											;;						;;
	Thing *p_target;												 ;;

#ifndef PSX
	// make some batty sounds. if we're not a gargoyle. or a balrog. or bane.
	if (p_bat->type==BAT_TYPE_BAT)
	{
		if (!(Random()&0xff)) MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BAT_SQUEEK_START,S_BAT_SQUEEK_END),MFX_NEVER_OVERLAP,p_thing);
	}
#endif

	switch(p_bat->state)
	{
#ifndef PSX
		case BAT_STATE_IDLE:

			p_bat->dx -= p_bat->dx / 16;
			p_bat->dy -= p_bat->dy / 16;
			p_bat->dz -= p_bat->dz / 16;

			if (p_bat->target)
			{
				BAT_turn_to_target(p_thing);
			}

			end = BAT_animate(p_thing);

			break;

		case BAT_STATE_GOTO:

			want_dx = p_bat->want_x - (p_thing->WorldPos.X >> 8) << 5;
			want_dy = p_bat->want_y - (p_thing->WorldPos.Y >> 8) << 5;
			want_dz = p_bat->want_z - (p_thing->WorldPos.Z >> 8) << 5;

			SATURATE(want_dx, -0x2000, +0x2000);
			SATURATE(want_dy, -0x0800, +0x0800);
			SATURATE(want_dz, -0x2000, +0x2000);

			p_bat->dx += want_dx - p_bat->dx >> 4;
			p_bat->dy += want_dy - p_bat->dy >> 4;
			p_bat->dz += want_dz - p_bat->dz >> 4;

			p_bat->dx -= p_bat->dx / 16;
			p_bat->dy -= p_bat->dy / 16;
			p_bat->dz -= p_bat->dz / 16;

			if (p_bat->target)
			{
				BAT_turn_to_target(p_thing);
			}

			end = BAT_animate(p_thing);

			break;
					   
		case BAT_STATE_CIRCLE:

			//
			// Where do we want to go?
			//

			switch(p_bat->substate)
			{
				case BAT_SUBSTATE_CIRCLE_HOME:
					want_x = (p_bat->home_x << 8) + 0x80;
					want_z = (p_bat->home_z << 8) + 0x80;
					want_y = PAP_calc_map_height_at(want_x,want_z) + 0x100;
					break;

				case BAT_SUBSTATE_CIRCLE_WANT:
					want_x = p_bat->want_x;
					want_y = p_bat->want_y;
					want_z = p_bat->want_z;
					break;

				case BAT_SUBSTATE_CIRCLE_TARGET:

					ASSERT(p_bat->target);

					p_target = TO_THING(p_bat->target);

					switch(p_target->Class)
					{
						case CLASS_PERSON:

							calc_sub_objects_position(
								p_target,
								0,
								SUB_OBJECT_HEAD,
							   &want_x,
							   &want_y,
							   &want_z);

							want_x += p_target->WorldPos.X >> 8;
							want_y += p_target->WorldPos.Y >> 8;
							want_z += p_target->WorldPos.Z >> 8;

							break;

						case CLASS_BAT:
							want_x = p_target->WorldPos.X >> 8;
							want_y = p_target->WorldPos.Y >> 8;
							want_z = p_target->WorldPos.Z >> 8;
							break;

						default:
							ASSERT(0);
							break;
					}

					break;
			}

			//
			// Circle around (want_x,want_y,want_z). Turn towards there slowly.
			//

			dx = want_x - (p_thing->WorldPos.X >> 8);
			dz = want_z - (p_thing->WorldPos.Z >> 8);

			wangle = calc_angle(dx,dz);
			dangle = wangle - p_thing->Draw.Tweened->Angle;

			if (dangle > +1024) {dangle -= 2048;}
			if (dangle < -1024) {dangle += 2048;}

			if (p_bat->substate == BAT_SUBSTATE_CIRCLE_TARGET)
			{
				//
				// If we are flying towards our target... then swoop down to attack!
				//

				if (abs(dangle) < 300)
				{
					//
					// The swoop down... make sure you don't stop.
					//

					p_bat->timer += ticks;

					dy = want_y - (p_thing->WorldPos.Y >> 8);

					if (QDIST3(abs(dx),abs(dy),abs(dz)) < 0x40)
					{
						if(p_target->Class==CLASS_PERSON && p_target->Genus.Person->PlayerID && EWAY_stop_player_moving())
						{
							//
							// cut scene so don't hurt player
							//
						}
						else
						if (!(p_bat->flag & BAT_FLAG_ATTACKED))
						{
							//
							// Hurt our target!
							//

							PainSound(p_target);	// scream yer lungs out!

							p_target->Genus.Person->Health -= 60;

							if (p_target->Genus.Person->Health <= 0)
							{
								set_person_dead(
									p_target,
									NULL,
									PERSON_DEATH_TYPE_OTHER,
									FALSE,
									FALSE);
							}

							p_bat->flag |= BAT_FLAG_ATTACKED;	// Don't do this until the next swoop.
						}
					}
					else
					{
						p_bat->flag &= ~BAT_FLAG_ATTACKED;	// Allowed to attack again.
					}
				}
				else
				{
					want_y = p_bat->want_y;
				}
			}

			dangle >>= 2;
			dist     = QDIST2(abs(dx), abs(dz)) >> 4;

			if (dist < 10) {dist = 10;}

			SATURATE(dangle, -dist, +dist);

			p_thing->Draw.Tweened->Angle += dangle;
			p_thing->Draw.Tweened->Angle &= 2047;

			//
			// Fly in the direction you are facing.
			//

			want_dx = SIN(p_thing->Draw.Tweened->Angle) >> 2;
			want_dz = COS(p_thing->Draw.Tweened->Angle) >> 2;

			//want_dx += want_dx >> 1;
			//want_dz += want_dz >> 1;

			ddx = want_dx - p_bat->dx >> 4;
			ddz = want_dz - p_bat->dz >> 4;

			SATURATE(ddx, -0x800, +0x800);
			SATURATE(ddz, -0x800, +0x800);

			p_bat->dx += ddx;
			p_bat->dz += ddz;

			p_bat->dx -= p_bat->dx / 32;
			p_bat->dy -= p_bat->dy /  4;
			p_bat->dz -= p_bat->dz / 32;

			//
			// Do 'dy' differently!
			//

			dy = want_y - (p_thing->WorldPos.Y >> 8);

			p_bat->dy += dy << 2;

			/*

			AENG_world_line(
				p_thing->WorldPos.X >> 8,
				p_thing->WorldPos.Y >> 8,
				p_thing->WorldPos.Z >> 8,
				16,
				0xffffff,
				want_x,
				want_y,
				want_z,
				0,
				0xff0000,
				FALSE);

			*/

			end = BAT_animate(p_thing);

			break;

		case BAT_STATE_ATTACK:

			if (p_bat->target && !(p_bat->flag & BAT_FLAG_ATTACKED) && p_thing->Draw.Tweened->FrameIndex > 6)
			{
				BAT_emit_fireball(p_thing);

				p_bat->flag |= BAT_FLAG_ATTACKED;
			}

			if (p_bat->target)
			{
				BAT_turn_to_target(p_thing);
			}

			end          = BAT_animate(p_thing);
			p_bat->timer = 0;

			break;
#endif
		case BAT_STATE_DYING:

			switch(p_bat->substate)
			{
#ifndef	PSX
				case BAT_SUBSTATE_DEAD_INITIAL:

					//
					// Only gargoyles have this state for now...
					//

//					ASSERT(p_bat->type == BAT_TYPE_GARGOYLE); not anymore 

					if (BAT_animate(p_thing))
					{
						p_bat->substate = BAT_SUBSTATE_DEAD_LOOP;

						BAT_set_anim(p_thing, BAT_ANIM_GARGOYLE_FALL_LOOP);
					}

					break;

				case BAT_SUBSTATE_DEAD_LOOP:

					BAT_animate(p_thing);

					//
					// Fall out of the sky!
					//

					p_bat->dy -= 0x180;

					p_bat->dx -= p_bat->dx / 32;
					p_bat->dz -= p_bat->dz / 32;

					ground = PAP_calc_map_height_at(
									p_thing->WorldPos.X >> 8,
									p_thing->WorldPos.Z >> 8) << 8;

					if (p_thing->WorldPos.Y <= ground + 0x4000)
					{
						//
						// Hit the ground.
						// 

						if (p_bat->type == BAT_TYPE_BAT && abs(p_bat->dy) > 0x1000)
						{
							//
							// Bounce!
							//

							p_bat->dy = abs(p_bat->dy) >> 2;
						}
						else
						{
							//
							// Dead!
							// 

							p_bat->dx = 0;
							p_bat->dy = 0;
							p_bat->dz = 0;

							if (p_bat->type == BAT_TYPE_BAT)
							{
								p_bat->state = BAT_STATE_DEAD;
							}
							else
							{
								p_bat->substate = BAT_SUBSTATE_DEAD_FINAL;

								BAT_set_anim(p_thing, BAT_ANIM_GARGOYLE_HIT_GROUND);
							}
						}
					}

					break;
#endif
				case BAT_SUBSTATE_DEAD_FINAL:

					//
					// Only gargoyles and balrogshave this state for now...
					//

//					ASSERT(p_bat->type == BAT_TYPE_GARGOYLE); balrogs too

					if (BAT_animate(p_thing))
					{	
						p_bat->state = BAT_STATE_DEAD;
						p_thing->State             = STATE_DEAD;
					}

					break;

				default:
					ASSERT(0);
					break;
			}

			//
			// Don't change state!
			// 

			end = FALSE;

			break;
		case BAT_STATE_DEAD:

			//
			// NO MORE PROCESSING!
			//

			return;
//#ifndef PSX
		case BAT_STATE_GROUND:

			switch(p_bat->substate)
			{
				case BAT_SUBSTATE_GROUND_WAIT:

					{
						Thing *darci = NET_PERSON(0);

						//
						// Wake up when the player gets too near.
						// 

						SLONG dx = abs(darci->WorldPos.X - p_thing->WorldPos.X);
						SLONG dz = abs(darci->WorldPos.Z - p_thing->WorldPos.Z);

						SLONG dist = QDIST2(dx,dz);

						if (dist < 0xa0000)
						{
							BAT_set_anim(p_thing, BAT_ANIM_GARGOYLE_WAKE_UP);

							p_bat->substate = BAT_SUBSTATE_GROUND_WAKE_UP;
						}
					}

					break;

				case BAT_SUBSTATE_GROUND_WAKE_UP:

					if (BAT_animate(p_thing))
					{
						BAT_set_anim(p_thing, BAT_ANIM_GARGOYLE_FLY_UP);

						p_bat->substate = BAT_SUBSTATE_GROUND_FLY_UP;
					}

					break;

				case BAT_SUBSTATE_GROUND_FLY_UP:

					if (BAT_animate(p_thing))
					{
						BAT_change_state(p_thing);
					}

					break;

				default:
					ASSERT(0);
					break;
			}

			end          = FALSE;
			p_bat->timer = 0;

			break;
#ifndef PSX

		case BAT_STATE_RECOIL:
			end          = BAT_animate(p_thing);
			p_bat->timer = 0;
			break;
#endif
		case BAT_STATE_BALROG_WANDER:
			
			{
				SLONG dangle;

				//
				// Turn towards where you are going.
				//

				dangle = BAT_turn_to_place(
							p_thing,
							p_bat->want_x,
							p_bat->want_z);

				if (abs(dangle) > 256 || p_bat->substate == BAT_SUBSTATE_YOMP_END)
				{	
					//
					// Don't move!
					//

					p_bat->dx -= p_bat->dx >> 2;
					p_bat->dz -= p_bat->dz >> 2;
				}
				else
				{
					//
					// Accelerate to moving speed.
					//

					want_dx = -SIN(p_thing->Draw.Tweened->Angle) >> 4;
					want_dz = -COS(p_thing->Draw.Tweened->Angle) >> 4;

					want_dx += want_dx >> 1;
					want_dz += want_dz >> 1;

					want_dx -= want_dx >> 5;
					want_dz -= want_dz >> 5;

					p_bat->dx += (want_dx - p_bat->dx) >> 3;
					p_bat->dz += (want_dz - p_bat->dz) >> 3;
				}

				end = BAT_animate(p_thing);

				if (end)
				{
					if (p_bat->substate == BAT_SUBSTATE_YOMP_START)
					{
						end = FALSE;

						BAT_set_anim(p_thing, BAT_ANIM_BALROG_YOMP);

						p_bat->substate = BAT_SUBSTATE_YOMP_MIDDLE;
					}
					else
					if (p_bat->substate == BAT_SUBSTATE_YOMP_MIDDLE)
					{
						end = FALSE;

						if (p_bat->timer == 0)
						{
							BAT_set_anim(p_thing, BAT_ANIM_BALROG_YOMP_END);

							p_bat->substate = BAT_SUBSTATE_YOMP_END;
						}
					}
				}
			}

			break;

		case BAT_STATE_BALROG_ROAR:
			end          = BAT_animate(p_thing);
			p_bat->timer = 0;
			break;

		case BAT_STATE_BALROG_FOLLOW:
		case BAT_STATE_BALROG_CHARGE:

			{
				if (!p_bat->target)
				{
					//
					// Emergency! How has this happened? Quick fix for now!
					//

					end          = TRUE;
					p_bat->timer = 0;
				}
				else
				{
					ASSERT(p_bat->target); //triggered

					Thing *p_target = TO_THING(p_bat->target);

					SLONG dangle;

					//
					// Turn towards where you are going.
					//


					extern SLONG there_is_a_los_mav(
									SLONG x1, SLONG y1, SLONG z1,
									SLONG x2, SLONG y2, SLONG z2);

					if (!there_is_a_los_mav(
							p_thing ->WorldPos.X          >> 8,
							p_thing ->WorldPos.Y + 0x4000 >> 8,
							p_thing ->WorldPos.Z          >> 8,
							p_target->WorldPos.X          >> 8,
							p_target->WorldPos.Y + 0x4000 >> 8,
							p_target->WorldPos.Z          >> 8))
					{
						p_bat->want_x = p_target->WorldPos.X >> 8;
						p_bat->want_z = p_target->WorldPos.Z >> 8;
					}
					else
					{
						MAV_Action ma = MAV_do(
											p_thing ->WorldPos.X >> 16,
											p_thing ->WorldPos.Z >> 16,
											p_target->WorldPos.X >> 16,
											p_target->WorldPos.Z >> 16,
											MAV_CAPS_GOTO);

						p_bat->want_x = (ma.dest_x << 8) + 0x80;
						p_bat->want_z = (ma.dest_z << 8) + 0x80;
					}

					dangle = BAT_turn_to_place(
								p_thing,
								p_bat->want_x,
								p_bat->want_z);

					if (abs(dangle) > 256 || p_bat->substate == BAT_SUBSTATE_YOMP_END)
					{	
						//
						// Don't move!
						//

						p_bat->dx -= p_bat->dx >> 3;
						p_bat->dz -= p_bat->dz >> 3;
					}
					else
					{
						//
						// Accelerate to moving speed.
						//

						want_dx = -SIN(p_thing->Draw.Tweened->Angle) >> 4;
						want_dz = -COS(p_thing->Draw.Tweened->Angle) >> 4;

						want_dx += want_dx >> 1;
						want_dz += want_dz >> 1;

						want_dx -= want_dx >> 5;
						want_dz -= want_dz >> 5;

						p_bat->dx += (want_dx - p_bat->dx) >> 3;
						p_bat->dz += (want_dz - p_bat->dz) >> 3;
					}

					end = BAT_animate(p_thing);

					if (end)
					{
						if (p_bat->substate == BAT_SUBSTATE_YOMP_START)
						{
							end = FALSE;

							BAT_set_anim(p_thing, BAT_ANIM_BALROG_YOMP);

							p_bat->substate = BAT_SUBSTATE_YOMP_MIDDLE;
						}
						else
						if (p_bat->substate == BAT_SUBSTATE_YOMP_MIDDLE)
						{
							end = FALSE;

							if (p_bat->timer == 0)
							{
								BAT_set_anim(p_thing, BAT_ANIM_BALROG_YOMP_END);

								p_bat->substate = BAT_SUBSTATE_YOMP_END;
							}
						}
					}
				}
			}

			break;

		case BAT_STATE_BALROG_SWIPE:

			if (p_bat->target && !(p_bat->flag & BAT_FLAG_ATTACKED) && p_thing->Draw.Tweened->FrameIndex > 4)
			{
				create_shockwave(
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					0x200,
					100,
					p_thing);

				p_bat->flag |= BAT_FLAG_ATTACKED;
			}

			end          = BAT_animate(p_thing);
			p_bat->timer = 0;

			break;

		case BAT_STATE_BALROG_STOMP:

			if (p_bat->target && !(p_bat->flag & BAT_FLAG_ATTACKED) && p_thing->Draw.Tweened->FrameIndex > 12)
			{
				create_shockwave(
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					0x300,
					150,
					p_thing);

				p_bat->flag |= BAT_FLAG_ATTACKED;

				PYRO_create(p_thing->WorldPos,PYRO_DUSTWAVE);
			}

			end          = BAT_animate(p_thing);
			p_bat->timer = 0;

			break;

		case BAT_STATE_BALROG_FIREBALL:

			switch(p_bat->substate)
			{
				case BAT_SUBSTATE_FIREBALL_TURN:

					end = BAT_animate(p_thing);

					if (abs(BAT_turn_to_target(p_thing)) < 128)
					{
						if (end)
						{
							p_bat->substate = BAT_SUBSTATE_FIREBALL_FIRE;

							BAT_set_anim(p_thing, BAT_ANIM_BALROG_SWIPE);
						}
					}

					break;

				case BAT_SUBSTATE_FIREBALL_FIRE:

					if (p_bat->target && !(p_bat->flag & BAT_FLAG_ATTACKED) && p_thing->Draw.Tweened->FrameIndex > 4)
					{
						BAT_emit_fireball(p_thing);

						p_bat->flag |= BAT_FLAG_ATTACKED;
					}

					end          = BAT_animate(p_thing);
					p_bat->timer = 0;
					
					break;

				default:
					ASSERT(0);
					break;
			}

			break;

		case BAT_STATE_BALROG_IDLE:
			end       = BAT_animate(p_thing);
			break;

		case BAT_STATE_BANE_IDLE:
			end = BAT_animate(p_thing);

			//
			// Make sparks connect to the summoning people.
			//

			BAT_process_bane_sparks(p_thing);

			p_bat->glow += 64 * TICK_RATIO >> TICK_SHIFT;

			if (p_bat->glow > 0xff00)
			{
				p_bat->glow = 0xff00;
			}

			break;

		case BAT_STATE_BANE_ATTACK:

			if (!(p_bat->flag & BAT_FLAG_ATTACKED) && p_thing->Draw.Tweened->FrameIndex > 12)
			{
				create_shockwave(
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					0x300,
					150,
					p_thing);

				p_bat->flag |= BAT_FLAG_ATTACKED;

				PYRO_create(p_thing->WorldPos,PYRO_DUSTWAVE);

				{
					Thing *darci = NET_PERSON(0);

					extern SLONG is_person_dead(Thing *p_person);

					if (!is_person_dead(darci))
					{
						if (THING_dist_between(p_thing, NET_PERSON(0)) < 0x600)
						{
							if (there_is_a_los(
									p_thing->WorldPos.X          >> 8,
									p_thing->WorldPos.Y + 0xc000 >> 8,
									p_thing->WorldPos.Z          >> 8,
									darci->WorldPos.X          >> 8,
									darci->WorldPos.Y + 0x6000 >> 8,
									darci->WorldPos.Z          >> 8,
									0))
							{
								SPARK_Pinfo p1;
								SPARK_Pinfo p2;
								
								p1.type   = SPARK_TYPE_LIMB;
								p1.flag   = 0;
								p1.person = THING_NUMBER(p_thing);
								p1.limb   = 0;

								p2.type   = SPARK_TYPE_LIMB;
								p2.flag   = 0;
 								p2.person = THING_NUMBER(darci);
								p2.limb   = SUB_OBJECT_HEAD;

								SPARK_create(
									&p1,
									&p2,
									50);

								if (darci->State != STATE_DANGLING &&
									darci->State != STATE_JUMPING)
								{
									set_face_thing(
										darci,
										p_thing);
								}

								darci->Genus.Person->Health -= 50;

								if (darci->Genus.Person->Health <= 0)
								{
									set_person_dead(
										darci,
										NULL,
										PERSON_DEATH_TYPE_OTHER,
										FALSE,
										0);
								}
								else
								{
									set_person_recoil(
										darci,
										ANIM_HIT_FRONT_MID,
										0);
								}
							}
						}
					}
				}
			}

			end          = BAT_animate(p_thing);
			p_bat->timer = 0;

			break;

		case BAT_STATE_BANE_START:

			end          = BAT_animate(p_thing);
			p_bat->timer = 0;

			break;

		default:
			ASSERT(0);
			break;
	}

	newpos.X = p_thing->WorldPos.X + (p_bat->dx * TICK_RATIO >> TICK_SHIFT);
	newpos.Y = p_thing->WorldPos.Y + (p_bat->dy * TICK_RATIO >> TICK_SHIFT);
	newpos.Z = p_thing->WorldPos.Z + (p_bat->dz * TICK_RATIO >> TICK_SHIFT);

	if (p_bat->type == BAT_TYPE_BALROG)
	{
		//
		// The balrog collides with the MAVHEIGHT array.
		//

		BAT_balrog_slide_along(
			p_thing->WorldPos.X,
			p_thing->WorldPos.Z,
		   &newpos.X,
		   &newpos.Z);
		
		//
		// The balrog is always on the ground...
		//

		newpos.Y = PAP_calc_height_at(
						p_thing->WorldPos.X >> 8,
						p_thing->WorldPos.Z >> 8) + 0x40 << 8;
		NIGHT_dlight_move(p_bat->glow, newpos.X>>8, (newpos.Y>>8)+64, newpos.Z>>8);

		//
		// The balrog has Big Fucking Feet
		//

//		TRACE("frame %d   flags %d   anim %d\n",p_thing->Draw.Tweened->FrameIndex,p_bat->flag,p_thing->Draw.Tweened->CurrentAnim);
/*
#define BAT_ANIM_BALROG_YOMP			2
#define BAT_ANIM_BALROG_YOMP_START		5
#define BAT_ANIM_BALROG_YOMP_END		6
*/
		switch(p_thing->Draw.Tweened->CurrentAnim)
		{
		case BAT_ANIM_BALROG_YOMP:
			if (
				 (((p_thing->Draw.Tweened->FrameIndex>=1)&&(p_thing->Draw.Tweened->FrameIndex<6))
				||((p_thing->Draw.Tweened->FrameIndex>=11)&&(p_thing->Draw.Tweened->FrameIndex<16)))
				&&!(p_bat->flag&BAT_FLAG_SYNC_FX))
			{
			   MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_STEP_START,S_BALROG_STEP_END),0,p_thing);
			   p_bat->flag&=~BAT_FLAG_SYNC_FX2;
			   p_bat->flag|=BAT_FLAG_SYNC_FX;
			}
			if ((p_thing->Draw.Tweened->FrameIndex>=6)&&(p_thing->Draw.Tweened->FrameIndex<=11)&&!(p_bat->flag&BAT_FLAG_SYNC_FX2))
			{
			   MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_STEP_START,S_BALROG_STEP_END),0,p_thing);
			   p_bat->flag&=~BAT_FLAG_SYNC_FX;
			   p_bat->flag|=BAT_FLAG_SYNC_FX2;
			}
			break;
		case BAT_ANIM_BALROG_YOMP_END:
			if (
				 (((p_thing->Draw.Tweened->FrameIndex>=1)&&(p_thing->Draw.Tweened->FrameIndex<5))
				||((p_thing->Draw.Tweened->FrameIndex>=11)&&(p_thing->Draw.Tweened->FrameIndex<16)))
				&&!(p_bat->flag&BAT_FLAG_SYNC_FX))
			{
			   MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_STEP_START,S_BALROG_STEP_END),0,p_thing);
			   p_bat->flag&=~BAT_FLAG_SYNC_FX2;
			   p_bat->flag|=BAT_FLAG_SYNC_FX;
			}
			if ((p_thing->Draw.Tweened->FrameIndex>=5)&&(p_thing->Draw.Tweened->FrameIndex<=11)&&!(p_bat->flag&BAT_FLAG_SYNC_FX2))
			{
			   MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_BALROG_STEP_START,S_BALROG_STEP_END),0,p_thing);
			   p_bat->flag&=~BAT_FLAG_SYNC_FX;
			   p_bat->flag|=BAT_FLAG_SYNC_FX2;
			}
			break;
		}

	}

	move_thing_on_map(p_thing, &newpos);

	//
	// Too far from home?
	//

	dx = p_thing->WorldPos.X - ((p_bat->home_x << 16) + 0x8000);
	dz = p_thing->WorldPos.Z - ((p_bat->home_z << 16) + 0x8000);

	if (abs(dx) > 0x50000 ||
		abs(dz) > 0x50000)
	{
		//
		// Too far from home...
		//

		p_bat->timer >>= 1;
	}

	if (p_bat->timer <= ticks)
	{
		p_bat->timer = 0;

		if (end)
		{
			BAT_change_state(p_thing);
		}
	}
	else
	{
		p_bat->timer -= ticks;
	}

}







//
// Create a new bat.
//

THING_INDEX BAT_create(
				SLONG type,
				SLONG x,
				SLONG z,
				UWORD yaw)
{
	SLONG i;

	Thing     *p_thing;
	Bat       *p_bat;
	DrawTween *dt;

	//
	// Get a thing structure.
	//


	p_thing = alloc_thing(CLASS_BAT);

	if (p_thing == NULL)
	{
		//
		// No more things left!
		//

		return NULL;
	}

	//
	// Get a bat structure.
	//

	for (i = 0; i < BAT_MAX_BATS; i++)
	{
		p_bat = TO_BAT(i);

		if (p_bat->type == BAT_TYPE_UNUSED)
		{
			goto found_unused_bat;
		}
	}

	//
	// No more bat structures.
	//

	return NULL;

  found_unused_bat:;

	//
	// Allocate a drawtween structure.
	//

	dt = alloc_draw_tween(DT_ROT_MULTI);

	if (dt == NULL)
	{
		return NULL;
	}

#ifndef PSX
#ifndef TARGET_DC
	//
	// Initialise the thing.
	//
	if(anim_chunk[type].MultiObject[0]==0)
	{
extern	SLONG load_anim_prim_object(SLONG prim);
		load_anim_prim_object(type);
//		ASSERT(0);

	}
#else
	ASSERT(anim_chunk[type].MultiObject[0]!=0);
#endif
#endif

	p_thing->WorldPos.X = x << 8;
	p_thing->WorldPos.Z = z << 8;
	p_thing->WorldPos.Y = PAP_calc_map_height_at(x,z) << 8;

	p_thing->DrawType     = DT_ANIM_PRIM;
	p_thing->Draw.Tweened = dt;

	p_thing->Genus.Bat = p_bat;
	p_thing->State     = STATE_NORMAL;
	p_thing->SubState  = NULL;
	p_thing->StateFn   = BAT_normal;
	p_thing->Index	   = type;

	add_thing_to_map(p_thing);

	//
	// Initialise the draw tween (anim prim <type>, animation 1)
	//

	dt->Angle		 =	yaw;
	dt->Roll		 =	0;
	dt->Tilt		 =	0;
	dt->AnimTween	 =	0;
	dt->TweenStage	 =	0;
	dt->NextFrame	 =	NULL;
	dt->QueuedFrame	 =	NULL;
	dt->TheChunk	 = &anim_chunk[type];
	dt->CurrentFrame =  anim_chunk[type].AnimList[1];
	dt->NextFrame	 =  dt->CurrentFrame->NextFrame;
	dt->CurrentAnim	 =  1;
	dt->FrameIndex	 =  0;
	dt->Flags		 =  0;

	//
	// Initialise the bat.
	//

	p_bat->type     = type;
	p_bat->dx       = 0;
	p_bat->dy       = 0;
	p_bat->dz       = 0;
	p_bat->health   = 100;
	p_bat->home_x   = x >> 8;
	p_bat->home_z   = z >> 8;
	p_bat->target   = NULL;
	p_bat->timer    = 0;
	p_bat->flag     = 0;

	switch(type)
	{
#ifndef PSX
		case BAT_TYPE_BAT:
			p_bat->state         = BAT_STATE_IDLE;
			p_bat->substate      = BAT_SUBSTATE_NONE;
			p_thing->WorldPos.Y += 0x100 << 8;
			break;

		case BAT_TYPE_GARGOYLE:
			p_bat->state         = BAT_STATE_GROUND;
			p_bat->substate      = BAT_SUBSTATE_GROUND_WAIT;
			break;
#endif
		case BAT_TYPE_BALROG:
			p_bat->state      = BAT_STATE_BALROG_ROAR;
			p_bat->substate   = BAT_SUBSTATE_NONE;
			p_bat->health     = 255;
			BAT_set_anim(p_thing, BAT_ANIM_BALROG_ROAR);
			// let's set this mutha alight
			{
				Thing *pyro;
				pyro=PYRO_create(p_thing->WorldPos,PYRO_IMMOLATE);
				if (pyro)
				{
					pyro->Genus.Pyro->victim=p_thing;
					pyro->Genus.Pyro->Flags=PYRO_FLAGS_FLICKER;
				}
				//darci->Genus.Person->BurnIndex=PYRO_NUMBER(pyro->Genus.Pyro)+1;
			}
			// and cast some light nearby...
			p_bat->glow=NIGHT_dlight_create( p_thing->WorldPos.X>>8, p_thing->WorldPos.Y>>8, p_thing->WorldPos.Z>>8, 200, 32, 28, 10);

			break;

		case BAT_TYPE_BANE:
			p_bat->state         = BAT_STATE_BANE_IDLE;
			p_bat->substate      = BAT_SUBSTATE_NONE;
			p_bat->glow          = 0x7f00;
			p_thing->WorldPos.Y += 0x60 << 8;
			BAT_set_anim(p_thing, BAT_ANIM_BANE_IDLE);
			break;

		default:
			ASSERT(0);
			break;
	}

	return THING_NUMBER(p_thing);
}


void BAT_apply_hit(
		Thing *p_me,
		Thing *p_aggressor,
		SLONG  damage)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;

	ASSERT(p_me->Class == CLASS_BAT);

	if (p_me->Genus.Bat->type == BAT_TYPE_BANE)
	{
		return;
	}


	ASSERT(damage>=0);




	if (p_aggressor)
	{
		SLONG blood_x;
		SLONG blood_y;
		SLONG blood_z;

		SLONG towards;

		blood_x = p_me->WorldPos.X;
		blood_y = p_me->WorldPos.Y;
		blood_z = p_me->WorldPos.Z;

		switch(p_me->Genus.Bat->type)
		{
			case BAT_TYPE_BAT:      blood_y += 0x2000; towards = 0x40; break;
			case BAT_TYPE_GARGOYLE: blood_y += 0x5000; towards = 0x60; break;
			case BAT_TYPE_BALROG:   blood_y += 0x8000; towards = 0x90; break;

			default:
				ASSERT(0);
				break;
		}

		dx = p_aggressor->WorldPos.X - p_me->WorldPos.X;
		dz = p_aggressor->WorldPos.Z - p_me->WorldPos.Z;

		dx >>= 8;
		dz >>= 8;

		SLONG length;

		length = QDIST2(abs(dx),abs(dz)) + 1;

		dx = dx * towards / length;
		dz = dz * towards / length;

		blood_x += dx << 8;
		blood_z += dz << 8;

//#ifndef VERSION_GERMAN
		if(VIOLENCE)
		PARTICLE_Add(
			blood_x,
			blood_y,
			blood_z,
			0,0,0,
			POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7FFF0000,
			PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE,10,75,1,20,5);

//#endif

		if (p_me->Genus.Bat->type != BAT_TYPE_BALROG)
		{
			dx = p_aggressor->WorldPos.X - p_me->WorldPos.X;
			dy = p_aggressor->WorldPos.Y - p_me->WorldPos.Y;
			dz = p_aggressor->WorldPos.Z - p_me->WorldPos.Z;

			dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

			dx = (dx << 8) / dist;
			dy = (dy << 8) / dist;
			dz = (dz << 8) / dist;

			//
			// Knocked back by the hit.
			//

			p_me->Genus.Bat->dx -= dx << 4;
			p_me->Genus.Bat->dy -= dy << 4;
			p_me->Genus.Bat->dz -= dz << 4;
		}
	}

	//
	// Bats are nails!
	//

	damage >>= 1;

	if (p_me->Genus.Bat->type == BAT_TYPE_BALROG)
	{
		damage >>= 3;
	}

	damage += 1;

	if (p_me->Genus.Bat->health <= damage)
	{
		{
			//
			// Kill the bat.
			//

			p_me->Genus.Bat->health = 0;
			p_me->Genus.Bat->state  = BAT_STATE_DYING;

			if (p_me->Genus.Bat->type == BAT_TYPE_GARGOYLE)
			{
				BAT_set_anim(p_me, BAT_ANIM_GARGOYLE_START_FALL);

				p_me->Genus.Bat->substate = BAT_SUBSTATE_DEAD_INITIAL;
				p_me->State             = STATE_DEAD;
			}
			else
			if (p_me->Genus.Bat->type == BAT_TYPE_BALROG)
			{
				p_me->Genus.Bat->substate = BAT_SUBSTATE_DEAD_FINAL;

				p_me->Genus.Bat->dx = 0;
				p_me->Genus.Bat->dy = 0;
				p_me->Genus.Bat->dz = 0;
	

				BAT_set_anim(p_me, BAT_ANIM_BALROG_DIE);
				MFX_play_thing(THING_NUMBER(p_me),S_BALROG_DEATH,0,p_me);

			}
			else
			{
				p_me->Genus.Bat->substate = BAT_SUBSTATE_DEAD_LOOP;

				BAT_set_anim(p_me, BAT_ANIM_BAT_DIE);
				p_me->State             = STATE_DEAD;
			}
		}
	}
	else
	{
		p_me->Genus.Bat->health -= damage;

		if (p_me->Genus.Bat->type == BAT_TYPE_BALROG)
		{
			//
			// The balrog makes you his target!
			//
			
			if (p_aggressor && p_aggressor->Class == CLASS_PERSON)
			{
				p_me->Genus.Bat->target = THING_NUMBER(p_aggressor);

				//
				// Change state to attack!
				//

				if (p_me->Genus.Bat->state == BAT_STATE_BALROG_IDLE   ||
					p_me->Genus.Bat->state == BAT_STATE_BALROG_WANDER)
				{
					p_me->Genus.Bat->timer = 0;
				}
			}
		}
		else
		{
			//
			// React to being shot!
			// 

			BAT_set_anim(p_me, BAT_ANIM_GENERIC_TAKE_HIT);

			p_me->Genus.Bat->state = BAT_STATE_RECOIL;
		}
	}
}
