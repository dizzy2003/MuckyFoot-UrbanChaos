//
// Person commands and high-level AI
//

#include "game.h"
#include "collide.h"
#include "c:\fallen\headers\pcom.h"
#include "eway.h"
#include "mav.h"
#include "statedef.h"
#include "combat.h"
#include "sound.h"
#include "overlay.h"
#include "frontend.h"
#include "interfac.h"
#include "balloon.h"
#include "road.h"
#include "wand.h"
#include "guns.h"
#include "animate.h"
#include "ware.h"
#include "mfx.h"
#include "cnet.h"
#include "fc.h"
#include "ob.h"
#include "spark.h"
#ifndef PSX
#include "panel.h"
#endif

#include "memory.h"
extern UBYTE stealth_debug;
#ifndef PSX
extern BOOL allow_debug_keys;
#endif





//
// local prototypes
//

extern	UBYTE	combo_display;
void	push_into_attack_group_at_angle(Thing *p_person,SLONG gang,SLONG reqd_angle);
SLONG	remove_from_gang_attack(Thing *p_person,Thing *p_target);
void PCOM_set_person_ai_flee_person(Thing *p_person,Thing *p_scary);

void DriveCar(Thing* p_person);
void ParkCar(Thing* p_person);
void DriveBike(Thing* p_person);
void ParkBike(Thing* p_person);

//
// externs
//
extern	SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor);
extern	SLONG	am_i_a_thug(Thing *p_person);
extern	SLONG	person_normal_animate(Thing *p_person);
extern	SLONG dist_to_target_pelvis(Thing *p_person_a,Thing *p_person_b);
extern	void set_person_recircle(Thing *p_person);
extern	void	FC_kill_player_cam(Thing *p_thing);
extern	UBYTE	GAME_cut_scene;
extern	SLONG	is_person_dead(Thing *p_person);
extern	SLONG	is_person_ko(Thing *p_person);
extern	SLONG person_has_gun_out(Thing *p_person);
extern	SLONG	is_person_guilty(Thing *p_person);

extern	UBYTE	vehicle_random[];

extern SLONG there_is_a_los_mav(	// From collide.cpp
				SLONG x1, SLONG y1, SLONG z1,
				SLONG x2, SLONG y2, SLONG z2);





#ifndef PSX
CBYTE *PCOM_ai_state_name[PCOM_AI_STATE_NUMBER] = 
{
	"Player",
	"Normal",
	"Investigating",
	"Searching",
	"Killing",
	"Sleeping",
	"Flee Place",
	"Flee Person",
	"Following",
	"Navtokil",
	"Homesick",
	"Lookaround",
	"Findcar",
	"Deactivate bomb",
	"Leave car",
	"Snipe",
	"Warmhands",
	"Findbike",
	"Knocked out",
	"Taunt",
	"Arrest",
	"Talking",
	"Grappled",
	"Enter car as passenger",
	"Aimless",
	"Hands up",
	"Summon",
	"Get item"
};

CBYTE *PCOM_ai_substate_name[PCOM_AI_SUBSTATE_NUMBER] =
{
	"None",
	"Suprised",
	"Walkover",
	"Look",
	"Punching",
	"Kicking",
	"Leg-it!",
	"Hunting",
	"Aiming",
	"No more ammo",
	"Goto car",
	"Get in car",
	"Goto bomb",
	"Cut wires",
	"Park car",
	"Leave car",
	"Goto fire",
	"Warm up",
	"Goto bike",
	"Hunt Slide",
	"Talk ask",
	"Talk tell",
	"Talk listen",
	"Hitching",
	"Start summon",
	"Mid summon",
	"Draw h2h",
	"Can't find",
	"Waiting"
};

CBYTE *PCOM_ai_name[PCOM_AI_NUMBER] =
{
	"player",
	"civillian",
	"guard",
	"assasin",
	"boss",
	"cop",
	"gang",
	"doorman",
	"bodyguard",
	"driver",
	"bomb-disposer",
	"biker",
	"fight test",
	"bully",
	"cop driver",
	"suicide",
	"flee player",
	"kill colour",
	"M.I.B.",
	"Bane",
	"Hypochondria",
	"Shoot dead"
};

CBYTE *PCOM_bent_name[PCOM_BENT_NUMBER] =
{
	"Lazy ",
	"Diligent ",
	"Gang ",
	"Fight-back ",
	"Kill-just-the-player ",
	"Robotic ",
	"Restricted ",
	"Player-kill"
};

CBYTE *PCOM_move_name[PCOM_MOVE_NUMBER] =
{
	"NULL",
	"Still",
	"PATROL",
	"PATROL_RAND",
	"WANDER",
	"FOLLOW",
	"WARM_HANDS",
	"FOLLOW_ON_SEE",
	"DANCE",
	"HANDS_UP",
	"TIED_UP",
};

#endif

//
// The movement states a person can be in.
//

#define PCOM_MOVE_STATE_PLAYER			 0	// This is a player.
#define PCOM_MOVE_STATE_STILL			 1
#define PCOM_MOVE_STATE_GOTO_XZ			 2
#define PCOM_MOVE_STATE_GOTO_WAYPOINT	 3
#define PCOM_MOVE_STATE_GOTO_THING		 4
#define PCOM_MOVE_STATE_PAUSE			 5
#define PCOM_MOVE_STATE_ANIMATION		 6	// Doing some animation-related thing.
#define PCOM_MOVE_STATE_CIRCLE			 7	// Circle around the person- to beat him up.
#define PCOM_MOVE_STATE_DRIVETO			 8	// Driving a car to a waypoint.
#define PCOM_MOVE_STATE_FOLLOW			 9
#define PCOM_MOVE_STATE_PARK_CAR		 10	// Park a car at a waypoint.
#define PCOM_MOVE_STATE_DRIVE_DOWN       11	// Driving down a road.
#define PCOM_MOVE_STATE_BIKETO			 12	// Driving a bike to a waypoint.
#define PCOM_MOVE_STATE_PARK_BIKE		 13	// Park a bike.
#define PCOM_MOVE_STATE_BIKE_DOWN		 14	// Biking down a road.
#define PCOM_MOVE_STATE_GRAPPLE			 15	// Circle around the person- to beat him up.
#define PCOM_MOVE_STATE_GOTO_THING_SLIDE 16
#define PCOM_MOVE_STATE_WAIT_CIRCLE		 17	// Doing some animation-related thing.
#define PCOM_MOVE_STATE_PARK_CAR_ON_ROAD 18 // Parking a car as you drive down the road.
#define PCOM_MOVE_STATE_NUMBER			 19

#define PCOM_MOVE_SPEED_WALK	PERSON_SPEED_WALK
#define PCOM_MOVE_SPEED_RUN		PERSON_SPEED_RUN
#define PCOM_MOVE_SPEED_SNEAK	PERSON_SPEED_SNEAK
#define PCOM_MOVE_SPEED_YOMP	PERSON_SPEED_YOMP
#define PCOM_MOVE_SPEED_SPRINT	PERSON_SPEED_SPRINT

#ifndef PSX
CBYTE *PCOM_move_state_name[] =
{
	"Player",
	"Still",
	"Goto XZ",
	"Goto waypoint",
	"Goto thing",
	"Pause",
	"Animation",
	"Circle",
	"Driveto",
	"Follow",
	"Park car",
	"Drive down",
	"Biketo",
	"Park bike",
	"Bike down",
	"Number",
	"Grapple",
	"goto thing slide",
	"wait circle",
	"Unused",
	"Unused",
	"Unused"
};
#endif

//
// While mavigating, a person is either running/walking to a point or
// doing an action.
// 

#define PCOM_MOVE_SUBSTATE_NONE		0
#define PCOM_MOVE_SUBSTATE_GOTO		1
#define PCOM_MOVE_SUBSTATE_ACTION	2
#define PCOM_MOVE_SUBSTATE_GUNAWAY	3
#define PCOM_MOVE_SUBSTATE_GUNOUT	4
#define PCOM_MOVE_SUBSTATE_PUNCH	5
#define PCOM_MOVE_SUBSTATE_KICK		6
#define PCOM_MOVE_SUBSTATE_SHOOT	7
#define PCOM_MOVE_SUBSTATE_ANIM		8
#define PCOM_MOVE_SUBSTATE_GETINCAR	9
#define PCOM_MOVE_SUBSTATE_3PTURN	10
#define PCOM_MOVE_SUBSTATE_WAIT     11
#define PCOM_MOVE_SUBSTATE_LEAVECAR 12
#define PCOM_MOVE_SUBSTATE_ARREST   13
#define PCOM_MOVE_SUBSTATE_LOSMAV   14

#define PCOM_MOVE_FLAG_AVOID_LEFT	(1 << 0)
#define PCOM_MOVE_FLAG_AVOID_RIGHT	(1 << 1)

//
// What to do when we leave a car
//

#define PCOM_EXCAR_NORMAL		 0	// Substate and arg ignored
#define PCOM_EXCAR_FLEE_PERSON   1	// 'arg' is the person to flee from.
#define PCOM_EXCAR_ARREST_PERSON 2	// 'arg' is the person to arrest.
#define PCOM_EXCAR_NAVTOKILL	 3  // 'arg' is the person to kill.


//
// The distance from a point somebody must be to count as having
// arrived there.
//

#define PCOM_ARRIVE_DIST	(0x40)

//
// The pause counter uses this many ticks-per-gameturn and ticks-per-second.
//

#define PCOM_TICKS_PER_TURN	16
#define PCOM_TICKS_PER_SEC	(16 * 20)


//
// This is an array of people with PCOM_AI_GANG sorted by their colour.
//

#define PCOM_MAX_GANG_PEOPLE 64

THING_INDEX PCOM_gang_person[PCOM_MAX_GANG_PEOPLE];
SLONG       PCOM_gang_person_upto;

typedef struct
{
	UBYTE index;
	UBYTE number;

} PCOM_Gang;

#define PCOM_MAX_GANGS 16

PCOM_Gang PCOM_gang[PCOM_MAX_GANGS];


//
// For when we are looking for things.
//

#define PCOM_MAX_FIND 16

UWORD PCOM_found[PCOM_MAX_FIND];
SLONG PCOM_found_num;


//
// Prototypes...
//

void PCOM_set_person_ai_homesick(Thing *p_person);
SLONG	person_holding_2handed(Thing *p_person);




void PCOM_init(void)
{
	//
	// Clear all our gang info.
	//

//	ASSERT(0);

	memset(PCOM_gang, 0, sizeof(PCOM_Gang) * PCOM_MAX_GANGS);

	PCOM_gang_person_upto = 0;
}


void PCOM_add_gang_member(THING_INDEX person, UBYTE group)
{
	SLONG i;
	PCOM_Gang *pg;

	ASSERT(WITHIN(group, 0, PCOM_MAX_GANGS - 1));
	ASSERT(WITHIN(PCOM_gang_person_upto, 0, PCOM_MAX_GANG_PEOPLE - 1));
	if(PCOM_gang_person_upto>=PCOM_MAX_GANG_PEOPLE)
	{
		return;
	}

	// Mike added this assert, and it shouldn't be here, apparently.
	//if(TO_THING(person)->Genus.Person->PersonType==PERSON_COP)
	//	ASSERT(0);
	
	if(TO_THING(person)->Genus.Person->PersonType==PERSON_CIV)
		ASSERT(0);

	pg = &PCOM_gang[group];

	//
	// Do we need to shove anybody along?
	//

	if (pg->index + pg->number == PCOM_gang_person_upto)
	{
		//
		// No need to shove anyone along.
		//

		PCOM_gang_person[PCOM_gang_person_upto] = person;

		pg->number            += 1;
		PCOM_gang_person_upto += 1;
	}
	else
	{
		//
		// Shove along all the other indices...
		//

		for (i = PCOM_gang_person_upto - 1; i >= pg->index + pg->number; i--)
		{
			PCOM_gang_person[i + 1] = PCOM_gang_person[i];
		}

		for (i = 0; i < PCOM_MAX_GANGS; i++)
		{
			if (i == group)
			{
				continue;
			}

			if (PCOM_gang[i].index >= pg->index + pg->number)
			{
				PCOM_gang[i].index += 1;
			}
		}

		//
		// Insert our person in the gap.
		//

		PCOM_gang_person[pg->index + pg->number] = person;
		pg->number                              += 1;
		PCOM_gang_person_upto                   += 1;
	}
}

//
// Returns TRUE if a fake wandering person should attack Darci.
//

SLONG PCOM_should_fake_person_attack_darci(Thing *p_person)
{
	Thing *darci = NET_PERSON(0);

	if (darci->Genus.Person->Ware)
	{
		//
		// Don't attack Darci when she's in a warehouse.
		//

		return FALSE;
	}

	if (EWAY_stop_player_moving())
	{
		//
		// In a cutscene...
		//

		return FALSE;
	}

	if (p_person->Genus.Person->PersonType == PERSON_COP)
	{
		//
		// Cops always attack the thugs!
		//

		return TRUE;
	}

	PCOM_found_num = THING_find_sphere(
						darci->WorldPos.X >> 8,
						darci->WorldPos.Y >> 8,
						darci->WorldPos.Z >> 8,
						0x600,
						PCOM_found,
						PCOM_MAX_FIND,
						1 << CLASS_PERSON);

	SLONG  i;
	Thing *p_found;

	for (i = 0; i < PCOM_found_num ; i++)
	{
		p_found = TO_THING(PCOM_found[i]);

		if (p_found->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
		{
			continue;
		}

		switch(p_found->Genus.Person->PersonType)
		{
			case PERSON_THUG_RASTA:
			case PERSON_THUG_GREY:
			case PERSON_THUG_RED:
			case PERSON_MIB1:
			case PERSON_MIB2:
			case PERSON_MIB3:
				return FALSE;
		}
	}

	return TRUE;
}




//
// returns shoot delay in 10ths of a second
//

//
// not really PCOM_  becasue used by players elsewhere
//

SLONG get_rate_of_fire(Thing *p_person)
{
	Thing *p_special;


	if (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3)
	{
		//
		// MIB are ninja shooting machines with built-in AK47s.
		//

		return 20;
	}

	if(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		return 20;
	}
	else
	if (p_person->Genus.Person->SpecialUse)
	{
		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch (p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_SHOTGUN: return 30;
			case SPECIAL_AK47:    return 25;
			case SPECIAL_GRENADE: return 20;

			default:

				//
				// This isn't a weapon you shoot!
				//

				return 0;
		}
	}
	else
	{
		return 0 ; // not a gun
	}
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
//	@@@@@@@
//	   @  @			  @				 @
//	   @  @			  @				 @
//	   @  @@@	@@	  @	   @@   @@@	 @@@@
//	   @  @	 @ @  @	  @	     @ @	 @
//	   @  @	 @ @@@@	  @    @@@  @@@	 @
//	   @  @	 @ @	  @   @  @	   @ @
//	   @  @	 @	@@@	   @   @@@@ @@@	  @@@
//
//
//
//			@@								   @
//		   @				  @				   @
//		   @				  @				   @
//		   @				  @	  @			   @
//		   @@@ @  @	@ @	  @@@ @@@	  @@  @	@  @
//		   @   @  @	@@ @ @	  @	  @	 @	@ @@ @ @
//		   @   @  @	@  @ @	  @	  @	 @	@ @	 @ @
//		   @   @  @	@  @ @	  @	  @	 @	@ @	 @ 
//		   @	@@@	@  @  @@@  @@ @	  @@  @	 @ @


SLONG person_has_gun_or_grenade_out(Thing *p_person)
{
	if (p_person->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)
	{
		return(SPECIAL_GUN);
	}

	if (!p_person->Genus.Person->SpecialUse)
	{
		return FALSE;
	}

	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);


		if (p_special->Genus.Special->SpecialType == SPECIAL_SHOTGUN ||
			p_special->Genus.Special->SpecialType == SPECIAL_AK47	 ||
			p_special->Genus.Special->SpecialType == SPECIAL_GRENADE)
		{
			return(p_special->Genus.Special->SpecialType);
		}
	}

	return FALSE;
}	


//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////




//
// Returns the amount of time given in tenth's of a second.
//

inline SLONG PCOM_get_duration(SLONG tenths)
{
	SLONG ans;
	
	ans = tenths * (PCOM_TICKS_PER_SEC / 10);

	return ans;
}

//
// approximate
//
inline SLONG PCOM_get_duration100(SLONG hun)
{
	SLONG ans;
	
	ans = hun * (PCOM_TICKS_PER_SEC / 100);

	return ans;
}

//
// Returns a random amount of time between the start and end given
// in tenths of a second.
//

SLONG PCOM_get_random_duration(SLONG min, SLONG max)
{
	SLONG ans;

	if (max <= min)
	{
		return min;
	}

	//
	// Convert from 10ths of second to our tick stuff.
	//

	min = min * (PCOM_TICKS_PER_SEC / 10);
	max = max * (PCOM_TICKS_PER_SEC / 10);

	ans = min + Random() % (max - min);

	return ans;
}


//
// Returns the angle for the given MAV_DIR
//

UWORD PCOM_get_angle_for_dir(SLONG dir)
{
	static UWORD dir_to_angle[4]=
	{
		512,
		512+1024,
		0,
		1024
	};

	SLONG angle;

	ASSERT(WITHIN(dir, 0, 3));

	angle = dir_to_angle[dir];

	return angle;
}

//
// Returns a small vector in the given MAV_DIR
//

void PCOM_get_dx_dz_for_dir(SLONG dir, SLONG *dx, SLONG *dz)
{
	SLONG ans_x;
	SLONG ans_z;

	SLONG angle = PCOM_get_angle_for_dir(dir);

	ans_x = COS(angle) >> 11;
	ans_z = SIN(angle) >> 11;

   *dx = ans_x;
   *dz = ans_z;
}

//
// Returns the distance between two people- includes y spacing.
//

SLONG PCOM_get_dist_between(Thing *p_person_a, Thing *p_person_b)
{
	SLONG ax;
	SLONG ay;
	SLONG az;

	if (p_person_a->Class == CLASS_PERSON)
	{

		calc_sub_objects_position(
			p_person_a,
			p_person_a->Draw.Tweened->AnimTween,
			SUB_OBJECT_PELVIS,
		   &ax,
		   &ay,
		   &az);
	}
	else
	{
		ax = 0;
		ay = 0;
		az = 0;
	}

	ax += p_person_a->WorldPos.X >> 8;
	ay += p_person_a->WorldPos.Y >> 8;
	az += p_person_a->WorldPos.Z >> 8;
	
	SLONG bx;
	SLONG by;
	SLONG bz;

	if (p_person_b->Class == CLASS_PERSON)
	{
		calc_sub_objects_position(
			p_person_b,
			p_person_b->Draw.Tweened->AnimTween,
			SUB_OBJECT_PELVIS,
		   &bx,
		   &by,
		   &bz);
	}
	else
	{
		bx = 0;
		by = 0;
		bz = 0;
	}

	bx += p_person_b->WorldPos.X >> 8;
	by += p_person_b->WorldPos.Y >> 8;
	bz += p_person_b->WorldPos.Z >> 8;

	SLONG dx = abs(ax - bx);
	SLONG dy = abs(ay - by);
	SLONG dz = abs(az - bz);

	SLONG dist = QDIST2(dx,dz);

	if (dy >= 0x80) 
	{
		//
		// Must be on a different level- that counts as being very far away.
		//

		dist += dy << 2;
	}

	return dist;
}


//
// Returns how far a person is from home.
//

SLONG PCOM_get_dist_from_home(Thing *p_person)
{
	SLONG home_x = (p_person->Genus.Person->HomeX << 0);// + 0x80;
	SLONG home_z = (p_person->Genus.Person->HomeZ << 0);// + 0x80;

	SLONG dx = abs((p_person->WorldPos.X >> 8) - home_x);
	SLONG dz = abs((p_person->WorldPos.Z >> 8) - home_z);

	SLONG dist = QDIST2(dx,dz);

	return dist;
}


//
// Returns TRUE if you can try to do a fastnav to the given person.
//	

SLONG PCOM_should_i_try_to_los_mav_to_person(Thing *p_person, Thing *p_target)
{
	if (p_target->Class == CLASS_PERSON)
	{
		if (p_target->State == STATE_CLIMB_LADDER ||
			p_target->State == STATE_CLIMBING     ||
			p_target->State == STATE_DANGLING     ||
			p_target->State == STATE_JUMPING)
		{
			return FALSE;
		}

		SLONG dx = abs(p_target->WorldPos.X - p_person->WorldPos.X);
		SLONG dy = abs(p_target->WorldPos.Y - p_person->WorldPos.Y);
		SLONG dz = abs(p_target->WorldPos.Z - p_person->WorldPos.Z);

		if (dx < 0x30000 && dz < 0x30000 && dy < 0x10000)
		{
			return TRUE;
		}
	}

	return FALSE;
}

//
// Gives the UBYTE mapsquare position you navigate to, to
// go to the given person.
//

void PCOM_get_person_navsquare(
		Thing *p_person,
		SLONG *map_dest_x,
		SLONG *map_dest_z)
{
	ASSERT(p_person->Class == CLASS_PERSON);

	SLONG ans_x = p_person->WorldPos.X;
	SLONG ans_z = p_person->WorldPos.Z;

	if (p_person->State == STATE_CLIMB_LADDER ||
		p_person->State == STATE_CLIMBING     ||
		p_person->State == STATE_DANGLING     ||
		p_person->State == STATE_JUMPING)
	{
		if (p_person->State == STATE_CLIMBING)
		{
			ASSERT(WITHIN(p_person->Genus.Person->OnFacet, 1, next_dfacet - 1));

			if (dfacets[p_person->Genus.Person->OnFacet].FacetType == STOREY_TYPE_FENCE_BRICK)
			{
				//
				// This person won't be able to climb over the fence...
				//

				goto dont_mav_in_front_of_the_person;
			}
		}

		//
		// Navigate to the square in front of the person.
		//

		SLONG dx = SIN(p_person->Draw.Tweened->Angle);
		SLONG dz = COS(p_person->Draw.Tweened->Angle);

		ans_x -= dx;
		ans_z -= dz;

		//
		// Make it two squares for jumping...
		//

		if (p_person->State == STATE_JUMPING)
		{
			ans_x -= dx;
			ans_z -= dz;
		}
	}
	else
	if (is_person_ko(p_person))
	{
		SLONG pelvis_x;
		SLONG pelvis_y;
		SLONG pelvis_z;

		//
		// Go to the pelvis position.
		//

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			SUB_OBJECT_PELVIS,
		   &pelvis_x,
		   &pelvis_y,
		   &pelvis_z);

		pelvis_x <<= 8;
		pelvis_z <<= 8;

		pelvis_x += p_person->WorldPos.X;
		pelvis_z += p_person->WorldPos.Z;

		ans_x = pelvis_x;
		ans_z = pelvis_z;
	}

  dont_mav_in_front_of_the_person:;

   *map_dest_x = ans_x >> 16;
   *map_dest_z = ans_z >> 16;
}

//
// Gives the UBYTE mapsquare position you navigate to be
// get to the door of the given vehicle.
// 

void PCOM_get_vehicle_navsquare(
		Thing *p_vehicle,
		SLONG *map_dest_x,
		SLONG *map_dest_z,
		SLONG  i_am_a_passenger,
		Thing *p_person)
{
	SLONG dx;
	SLONG dz;

	SLONG ix1;
	SLONG iz1;
	SLONG dist1;

	SLONG ix2;
	SLONG iz2;
	SLONG dist2;

	extern void get_car_enter_xz(Thing *p_vehicle,SLONG door, SLONG *cx,SLONG *cz);

	//
	// Go to the nearest door.
	//

	get_car_enter_xz(p_vehicle,0,&ix1,&iz1);

	dx = abs((p_person->WorldPos.X >> 8) - ix1);
	dz = abs((p_person->WorldPos.Z >> 8) - iz1);

	dist1 = QDIST2(dx,dz);

	get_car_enter_xz(p_vehicle,1,&ix2,&iz2);

	dx = abs((p_person->WorldPos.X >> 8) - ix2);
	dz = abs((p_person->WorldPos.Z >> 8) - iz2);

	dist2 = QDIST2(dx,dz);

	//
	// (ix,iz) is now the position of the door.
	//

	if (dist1 < dist2)
	{
	   *map_dest_x = ix1 >> 8;
	   *map_dest_z = iz1 >> 8;
	}
	else
	{
	   *map_dest_x = ix2 >> 8;
	   *map_dest_z = iz2 >> 8;
	}

#ifndef TARGET_DC
	#ifndef NDEBUG
#ifndef	PSX
	AENG_world_line(
		p_person->WorldPos.X >> 8,
		p_person->WorldPos.Y >> 8,
		p_person->WorldPos.Z >> 8,
		32,
		0x000000,
       *map_dest_x,
		p_person->WorldPos.Y >> 8,
	   *map_dest_z,
		0,
		0xff0000,
		TRUE);
#endif
	#endif
#endif
}

//
// Positions the given person so he is in a good position to sit
// on the prim.  If 'dont_teleport' then if the person is too far
// from where he want to sit, this function returns FALSE and
// the person isn't moved.
//

SLONG PCOM_position_person_to_sit_on_prim(
		Thing *p_person,
		SLONG  prim,
		SLONG  prim_x,
		SLONG  prim_y,
		SLONG  prim_z,
		SLONG  prim_yaw,
		SLONG  dont_teleport)
{
	ASSERT(p_person->Class == CLASS_PERSON);

	SLONG dx;
	SLONG dz;
	SLONG away;

	PrimInfo *pi = get_prim_info(prim);

	GameCoord newpos;

	dx = SIN(prim_yaw & 2047) >> 8;
	dz = COS(prim_yaw & 2047) >> 8;

	p_person->Draw.Tweened->Angle = prim_yaw & 2047;

	away  = pi->minz;
	away -= 0x20;

	newpos.X = (prim_x << 8) + dx * away;
	newpos.Z = (prim_z << 8) + dz * away;

	if (dont_teleport)
	{
		//
		// Make sure we aren't too far from where we are going to teleport to.
		//

		dx = abs(newpos.X - p_person->WorldPos.X);
		dz = abs(newpos.Z - p_person->WorldPos.Z);

		if (dx + dz > 0x5000)
		{
			return FALSE;
		}
	}

	newpos.Y = /*PAP_calc_map_height_at(newpos.X >> 8, newpos.Z >> 8)*/ prim_y << 8;

	move_thing_on_map(p_person, &newpos);

	return TRUE;
}

//
// Returns the place a person should be running from- only
// call while fleeing.
//

void PCOM_get_flee_from_pos(
		Thing *p_person,
		SLONG *from_x,
		SLONG *from_z)
{
	SLONG ans_x;
	SLONG ans_z;

	switch(p_person->Genus.Person->pcom_ai_state)
	{
		case PCOM_AI_STATE_FLEE_PLACE:
			ans_x = (((p_person->Genus.Person->pcom_ai_arg >> 8) & 0xff) << 8) + 0x80;
			ans_z = (((p_person->Genus.Person->pcom_ai_arg >> 0) & 0xff) << 8) + 0x80;
			break;

		case PCOM_AI_STATE_FLEE_PERSON:
			ans_x = TO_THING(p_person->Genus.Person->pcom_ai_arg)->WorldPos.X >> 8;
			ans_z = TO_THING(p_person->Genus.Person->pcom_ai_arg)->WorldPos.Z >> 8;
			break;

		default:
			ASSERT(0);
			break;
	}

   *from_x = ans_x;
   *from_z = ans_z;
}

//
// Returns a movement person's destination.
//

void PCOM_get_person_dest(
		Thing *p_person,
		SLONG *dest_x,
		SLONG *dest_z)
{
	SLONG ans_x;
	SLONG ans_y;
	SLONG ans_z;

	Thing *p_thing;

	ans_x = p_person->WorldPos.X >> 8;
	ans_z = p_person->WorldPos.Z >> 8;

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:
			break;

		case PCOM_MOVE_STATE_GOTO_XZ:

			ans_x = (p_person->Genus.Person->pcom_move_arg >> 8) & 0xff;
			ans_z = (p_person->Genus.Person->pcom_move_arg >> 0) & 0xff;

			ans_x <<= 8;
			ans_z <<= 8;

			ans_x += 0x80;
			ans_z += 0x80;

			break;

		case PCOM_MOVE_STATE_GOTO_WAYPOINT:

			EWAY_get_position(
				p_person->Genus.Person->pcom_move_arg,
			   &ans_x,
			   &ans_y,
			   &ans_z);

			break;

		case PCOM_MOVE_STATE_GOTO_THING:

			p_thing = TO_THING(p_person->Genus.Person->pcom_move_arg);

			switch(p_thing->Class)
			{
				case CLASS_PERSON:

					PCOM_get_person_navsquare(
						p_thing,
					   &ans_x,
					   &ans_z);

					break;

				case CLASS_VEHICLE:

					PCOM_get_vehicle_navsquare(
						p_thing,
					   &ans_x,
					   &ans_z,
						p_thing->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HITCH,
						p_person);

					break;

				default:

					//
					// Use the position of the thing by default.
					//

					ans_x = p_thing->WorldPos.X >> 16;
					ans_z = p_thing->WorldPos.Z >> 16;

					break;
			}

			ans_x <<= 8;
			ans_z <<= 8;

			ans_x += 0x80;
			ans_z += 0x80;

			break;

		case PCOM_MOVE_STATE_PAUSE:
			break;

		case PCOM_MOVE_STATE_DRIVETO:
		case PCOM_MOVE_STATE_BIKETO:
			
			EWAY_get_position(
				p_person->Genus.Person->pcom_move_arg,
			   &ans_x,
			   &ans_y,
			   &ans_z);

			break;

		case PCOM_MOVE_STATE_DRIVE_DOWN:
		case PCOM_MOVE_STATE_BIKE_DOWN:
		case PCOM_MOVE_STATE_PARK_CAR_ON_ROAD:

			ROAD_get_dest(
				p_person->Genus.Person->pcom_move_arg >> 8,
				p_person->Genus.Person->pcom_move_arg & 0xff,
			   &ans_x,
			   &ans_z);

			break;

		case PCOM_MOVE_STATE_PARK_CAR:
		case PCOM_MOVE_STATE_PARK_BIKE:

			//
			// Trying to stop...
			//

			break;

		default:
			ASSERT(0);
			break;
	}

   *dest_x = ans_x;
   *dest_z = ans_z;

	return;
}


//
// The position for the given person's MAV_Action
//

void PCOM_get_mav_action_pos(
		Thing     *p_person,
		SLONG     *dest_x,
		SLONG     *dest_z)
{
	ASSERT(p_person->Genus.Person->pcom_move_substate != PCOM_MOVE_SUBSTATE_LOSMAV);

	SLONG dx;
	SLONG dz;

	SLONG ans_x = (p_person->Genus.Person->pcom_move_ma.dest_x << 8) + 0x80;
	SLONG ans_z = (p_person->Genus.Person->pcom_move_ma.dest_z << 8) + 0x80;

	if (p_person->Genus.Person->pcom_move_ma.action != MAV_ACTION_GOTO)
	{
		//
		// Take the direction of the MAV_Action into account.
		//

		PCOM_get_dx_dz_for_dir(
			p_person->Genus.Person->MA.dir,
		   &dx,
		   &dz);

		ans_x += dx;
		ans_z += dz;
	}

   *dest_x = ans_x;
   *dest_z = ans_z;
}


//
// Returns TRUE if the person is carrying any sort of gun.
//

SLONG PCOM_person_has_any_sort_of_gun(Thing *p_person)
{
	if (p_person->Flags & FLAGS_HAS_GUN)
	{
		return TRUE;
	}

	if (person_has_special(p_person, SPECIAL_SHOTGUN) ||
		person_has_special(p_person, SPECIAL_AK47)    ||
		person_has_special(p_person, SPECIAL_GRENADE))
	{
		return TRUE;
	}

	if (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3)
	{
		return TRUE;
	}

	return FALSE;
}

//
// Returns TRUE if the person has gun and ammo to use with it.
//

SLONG PCOM_person_has_any_sort_of_h2h(Thing *p_person)
{
	if(person_has_special(p_person, SPECIAL_KNIFE))
	{
		return(SPECIAL_KNIFE);
	}
	else
	if(person_has_special(p_person, SPECIAL_BASEBALLBAT))
	{
		return(SPECIAL_BASEBALLBAT);
	}
	else
	{
		return(0);
	}
}

SLONG PCOM_person_has_any_sort_of_gun_with_ammo(Thing *p_person)
{
	Thing *p_special;

	if (p_person->Flags & FLAGS_HAS_GUN)
	{
		if (p_person->Genus.Person->Ammo)
		{
			return TRUE;
		}
	}

	if (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3)
	{
		return TRUE;
	}
// lazy MARK!
	if ((p_special = person_has_special(p_person, SPECIAL_SHOTGUN)) ||
		(p_special = person_has_special(p_person, SPECIAL_AK47))    ||
		(p_special = person_has_special(p_person, SPECIAL_GRENADE)))
	{
		if (p_special->Genus.Special->ammo)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//
// Returns TRUE if there are people waiting to get into the given car
// near the car.
//

SLONG PCOM_are_there_people_who_want_to_enter(Thing *p_vehicle)
{
	SLONG  i;
	SLONG  num_found;
	Thing *p_found;

	num_found = THING_find_sphere(
					p_vehicle->WorldPos.X >> 8,
					p_vehicle->WorldPos.Y >> 8,
					p_vehicle->WorldPos.Z >> 8,
					0x400,
					THING_array,
					THING_ARRAY_SIZE,
					1 << CLASS_PERSON);
	
	for (i = 0; i < num_found; i++)
	{
		p_found = TO_THING(THING_array[i]);

		if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FINDCAR ||
			p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HITCH)
		{
			Thing *p_vehin = TO_THING(p_found->Genus.Person->pcom_ai_arg);

			if (p_vehin == p_vehicle)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


SLONG PCOM_person_doing_nothing_important(Thing *p_person)
{
	if(p_person->State==STATE_DYING||p_person->State==STATE_DEAD||p_person->State==STATE_CARRY)
		return(FALSE);

	if (p_person->State == STATE_MOVEING && (p_person->SubState == SUB_STATE_SIMPLE_ANIM_OVER || p_person->SubState == SUB_STATE_SIMPLE_ANIM))
	{
		if (p_person->Draw.Tweened->CurrentAnim == ANIM_SIT_DOWN ||
			p_person->Draw.Tweened->CurrentAnim == ANIM_SIT_IDLE)
		{
			//
			// If they're sitting down...
			//

			return TRUE;
		}
	}

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
		if(p_person->Genus.Person->pcom_ai==PCOM_AI_COP_DRIVER &&(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING) &&p_person->Genus.Person->pcom_move==PCOM_MOVE_WANDER)
			return(TRUE);

	if(p_person->Genus.Person->Flags & (FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C))
		return(FALSE);


	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_GOTO_XZ:
		case PCOM_MOVE_STATE_GOTO_WAYPOINT:
		case PCOM_MOVE_STATE_GOTO_THING:

			if (p_person->Genus.Person->pcom_move_substate == PCOM_MOVE_SUBSTATE_ACTION)
			{
				//
				// In the middle of doing a complicated moving.
				//

				return FALSE;
			}
	}

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
	{
		if(p_person->Genus.Person->pcom_ai==PCOM_AI_COP_DRIVER &&(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING) &&p_person->Genus.Person->pcom_move==PCOM_MOVE_WANDER)
			return TRUE;


		if (p_person->State == STATE_IDLE    ||
			p_person->State == STATE_GOTOING ||
			p_person->State == STATE_NORMAL)
		{
			return TRUE;
		}

		if (p_person->State    == STATE_GUN &&
			p_person->SubState == SUB_STATE_AIM_GUN)
		{
			//
			// Gun out... might be doing something important!
			//

			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
			{
				return TRUE;
			}
		}

		if (p_person->State == STATE_MOVEING)
		{
			if (p_person->SubState == SUB_STATE_SIMPLE_ANIM ||
				p_person->SubState == SUB_STATE_SIMPLE_ANIM_OVER)
			{
				return TRUE;
			}
		}
	}

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS ||
		p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK ||
		p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_INVESTIGATING ||
		p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING)
	{
		return TRUE;
	}

	return FALSE;
}


//
// Returns TRUE if the person has any kind of gun in his hand- ready to shoot.
//

SLONG PCOM_person_has_gun_in_hand(Thing *p_person)
{
	if (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3)
	{
		//
		// MIB are ninja shooting machines with built-in AK47s!
		//

		return TRUE;
	}

	if (p_person->State    == STATE_GUN &&
		p_person->SubState == SUB_STATE_DRAW_GUN)
	{	
		//
		// Still drawing a weapon.
		//

		return FALSE;
	}

	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		return TRUE;
	}

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		if (p_special->Genus.Special->SpecialType == SPECIAL_SHOTGUN ||
			p_special->Genus.Special->SpecialType == SPECIAL_AK47    ||
			p_special->Genus.Special->SpecialType == SPECIAL_GRENADE)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//
// Returns TRUE if the target has a gun pointed at you!
//

SLONG PCOM_target_could_shoot_me(Thing *p_person, Thing *p_shooter)
{
	if(can_a_see_b(p_shooter,p_person))
	if (PCOM_person_has_gun_in_hand(p_shooter))
	{
		SLONG dangle = get_dangle(p_shooter, p_person);

		#define PCOM_SHOOTME_DANGLE 256

		if (dangle <        PCOM_SHOOTME_DANGLE ||
			dangle > 2048 - PCOM_SHOOTME_DANGLE)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//
// Returns the distance of the person from the point.
// 

SLONG PCOM_person_dist_from(
		Thing *p_person,
		SLONG  world_x,
		SLONG  world_z)
{
	SLONG dx = abs((p_person->WorldPos.X >> 8) - world_x);
	SLONG dz = abs((p_person->WorldPos.Z >> 8) - world_z);

	SLONG dist = QDIST2(dx,dz);

	return dist;
}


//
// Finds a place near the given person where somebody might be
// hiding.  Returns NULL if it couldn't find anywhere.
//

SLONG PCOM_find_hiding_place(
		Thing *p_person,
		SLONG *hide_x,
		SLONG *hide_z)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG score;

	SLONG ndx;
	SLONG ndz;
	SLONG ndist;

	SLONG mid_x = p_person->WorldPos.X >> 16;
	SLONG mid_z = p_person->WorldPos.Z >> 16;

	SLONG mx;
	SLONG mz;

	SLONG best_x     =  0;
	SLONG best_z     =  0;
	SLONG best_score = -INFINITY;

	SLONG x1 = (p_person->WorldPos.X >> 8);
	SLONG y1 = (p_person->WorldPos.Y >> 8) + 0x60;
	SLONG z1 = (p_person->WorldPos.Z >> 8);

	SLONG x2;
	SLONG y2;
	SLONG z2;

	MAV_Action ma;

	PAP_Hi *ph;

	for (dx = -5; dx <= 5; dx++)
	for (dz = -5; dz <= 5; dz++)
	{
		mx = mid_x + dx;
		mz = mid_z + dz;

		if (WITHIN(mx, 0, PAP_SIZE_HI - 1) &&
			WITHIN(mz, 0, PAP_SIZE_HI - 1))
		{
			ph = &PAP_2HI(mx,mz);

			if (ph->Flags & PAP_FLAG_HIDDEN)
			{
				//
				// Inside a building.
				//
			}
			else
			{
				dist = abs(dx) + abs(dz);

				if (dist >= 2)
				{
					//
					// Could we see someone standing on this square?
					//

					x2 = (mx << 8) + 0x80;
					z2 = (mz << 8) + 0x80;

					y2 = PAP_calc_height_at(x2,z2) + 0x60;

					if (there_is_a_los(
							x1, y1, z1,
							x2, y2, z2,
							0))
					{
						//
						// We can see this square- there isn't anyone hiding here.
						//
					}
					else
					{
						ma = MAV_do(
								x1 >> 8,
								z1 >> 8,
								x2 >> 8,
								z2 >> 8,
								MAV_CAPS_DARCI);

						if (MAV_do_found_dest)
						{
							//
							// Somebody could be hiding here.
							//

							ndx   = ma.dest_x - mid_x;
							ndz   = ma.dest_z - mid_z;
							ndist = abs(ndx) + abs(ndz);

							if (ndist > dist + 2)
							{
								//
								// Its a very round-about way to get there.
								//
							}
							else
							{
								score = 0x1000 - (Random() % (dist << 4));

								if (score > best_score)
								{
									best_x     = x2;
									best_z     = z2;
									best_score = score;
								}
							}
						}
					}
				}
			}
		}
	}

   *hide_x = best_x;
   *hide_z = best_z;

	return (best_score > -INFINITY);
}

SLONG PCOM_player_is_doing_something_naughty(Thing *darci)
{
	SLONG map_x;
	SLONG map_z;

	//
	// She isn't allowed to be fighting anyone.
	//

	if (darci->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		return TRUE;
	}

	/*

	//
	// She isn't allowed to be on a naughty square- or trying to get to a
	// naughty square.
	//

	map_x = darci->WorldPos.X >> 16;
	map_z = darci->WorldPos.Z >> 16;

	if (PAP_2HI(map_x,map_z).Flags & PAP_FLAG_NAUGHTY)
	{
		return TRUE;
	}

	if (darci->Genus.Person->Action == ACTION_CLIMBING)
	{
		SLONG dx;
		SLONG dz;

		//
		// Is Darci climbing into somewhere naughty?
		//

		dx = -SIN(darci->Draw.Tweened->Angle);
		dz = -COS(darci->Draw.Tweened->Angle);

		map_x = darci->WorldPos.X + dx >> 16;
		map_z = darci->WorldPos.Z + dz >> 16;

		if (PAP_2HI(map_x,map_z).Flags & PAP_FLAG_NAUGHTY)
		{
			return TRUE;
		}
	}

	*/

	return FALSE;
}





//
// Makes everyone in this person's gang help the person in 
// his fight (if they can see him).
//

void PCOM_set_person_ai_kill_person(Thing *p_person, Thing *p_target, SLONG alert_gang = TRUE); // These two functions call eachother!

void PCOM_alert_my_gang_to_a_fight(Thing *p_person, Thing *p_target)
{
	SLONG i;

	PCOM_Gang *pg;
	Thing     *p_gang;

	ASSERT(WITHIN(p_person->Genus.Person->pcom_colour, 0, PCOM_MAX_GANGS - 1));
	ASSERT(p_person->Genus.Person->pcom_bent & PCOM_BENT_GANG);

	pg = &PCOM_gang[p_person->Genus.Person->pcom_colour];

	for (i = 0; i < pg->number; i++)
	{
		p_gang = TO_THING(PCOM_gang_person[pg->index + i]); //PCOM_gang_person[pg->index + i]==0 error    pg->index=2 pg->number=3 i=2

		if(p_gang->Class==CLASS_PERSON)
		if (p_gang != p_person)
		{
			SLONG dx = p_gang->WorldPos.X - p_target->WorldPos.X ;
			SLONG dz = p_gang - p_target;
			

			if (!(p_gang->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
			if (p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL        ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS    ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING     ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_SEARCHING     ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_TAUNT         ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_INVESTIGATING)
			{

				
					if(p_target->Genus.Person->PersonType==PERSON_DARCI)
					{
						if(p_gang->Genus.Person->PersonType==PERSON_CIV)
						{
							ASSERT(0);
						}
						if(p_gang->Genus.Person->PersonType==PERSON_COP)
						{
							ASSERT(0);
						}
					}

				//
				// This person isn't doing anything important. He can help in the fight.
				//

				PCOM_set_person_ai_kill_person(p_gang, p_target, FALSE);
			}
		}
	}
}
SLONG	am_i_a_thug(Thing *p_person);

void PCOM_alert_my_gang_to_flee(Thing *p_person, Thing *p_target)
{
	SLONG i;

	PCOM_Gang *pg;
	Thing     *p_gang;

	ASSERT(WITHIN(p_person->Genus.Person->pcom_colour, 0, PCOM_MAX_GANGS - 1));
	ASSERT(p_person->Genus.Person->pcom_bent & PCOM_BENT_GANG);

	pg = &PCOM_gang[p_person->Genus.Person->pcom_colour];

	for (i = 0; i < pg->number; i++)
	{
		p_gang = TO_THING(PCOM_gang_person[pg->index + i]); //PCOM_gang_person[pg->index + i]==0 error    pg->index=2 pg->number=3 i=2

		if(p_gang->Class==CLASS_PERSON)
		if(!am_i_a_thug(p_person)||(!(p_gang->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK)))
		if (p_gang != p_person)
		{
			if (!(p_gang->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
			if (p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL        ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS    ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING     ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_SEARCHING     ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_TAUNT         ||
				p_gang->Genus.Person->pcom_ai_state == PCOM_AI_STATE_INVESTIGATING)
			{
				//
				// This person isn't doing anything important. He can run away
				//

				PCOM_set_person_ai_flee_person(p_gang, p_target);
			}
		}
	}
}




// ========================================================
//
// SET PEOPLE MOVE STATES
//
// ========================================================

void PCOM_set_person_move_still(Thing *p_person)
{
	//
	// Make the person stand still.
	//

	set_person_idle(p_person);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state   = PCOM_MOVE_STATE_STILL;
	p_person->Genus.Person->pcom_move_counter = 0;
}

void PCOM_set_person_move_mav_to_xz(Thing *p_person, SLONG dest_x, SLONG dest_z, SLONG speed)
{
	SLONG caps;

	SLONG goal_x;
	SLONG goal_z;

	SLONG start_x;
	SLONG start_y;
	SLONG start_z;

	/*

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_FOOT,
	   &start_x,
	   &start_y,
	   &start_z);

	start_x += p_person->WorldPos.X >> 8;
	start_z += p_person->WorldPos.Z >> 8;

	start_x >>= 8;
	start_z >>= 8;
	
	*/

	start_x = p_person->WorldPos.X >> 16;
	start_z = p_person->WorldPos.Z >> 16;

	//
	// We can only go to mapsquares.
	//

	dest_x >>= 8;
	dest_z >>= 8;

	//
	// Store the destination.
	//

	p_person->Genus.Person->pcom_move_arg = (dest_x << 8) | (dest_z);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_GOTO_XZ;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	p_person->Genus.Person->pcom_move_counter  = 0;

	//
	// What is this person's caps?
	// 

	caps = (p_person->Genus.Person->pcom_bent & PCOM_BENT_RESTRICTED) ? MAV_CAPS_GOTO : MAV_CAPS_DARCI;

	if (p_person->Genus.Person->pcom_ai       == PCOM_AI_CIV)
	{
	
		if ((p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL && p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER) ||
			(p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PLACE || p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PERSON))
		{
			//
			// If this person is in a good place for a wandering civ, then he
			// isn't allowed to jump/vault/climb ladders.
			//

			if (WAND_square_is_wander(p_person->WorldPos.X >> 16, p_person->WorldPos.Z >> 16))
			{
				//
				// Restricted wandering capability.
				//
				
				caps = MAV_CAPS_GOTO;
			}
		}
	}

	UWORD nav_into_ware   = NULL;
	UBYTE nav_outof_ware  = FALSE;
	UBYTE nav_inside_ware = FALSE;

	//
	// If you are scared and in a warehouse- always run to the exit.
	// 

	if ((p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PLACE || 
	 	 p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PERSON))
	{
		if (p_person->Genus.Person->Ware)
		{
			nav_outof_ware = TRUE;
		}
	}

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK)
	{
		if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_HOME_IN_WAREHOUSE)
		{
			nav_into_ware = WARE_which_contains(
								p_person->Genus.Person->HomeX >> 8,
								p_person->Genus.Person->HomeZ >> 8);
		}
	}

	if (nav_into_ware && p_person->Genus.Person->Ware)
	{
		//
		// We want to navigate to a warehouse and we are already in one.
		//

		if (nav_into_ware == p_person->Genus.Person->Ware)
		{
			//
			// We are alreay in the right warehouse!
			//

			nav_into_ware   = NULL;
			nav_inside_ware = TRUE;
		}
		else
		{
			//
			// We are in the wrong warehouse!
			//

			nav_outof_ware = TRUE;
			nav_into_ware  = FALSE;
		}
	}

	if (nav_outof_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_exit(
												p_person,
												caps);
	}
	else
	if (nav_into_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_enter(
												p_person,
												nav_into_ware,
												caps);
	}
	else
	if (nav_inside_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_inside(
												p_person,
												dest_x,
												dest_z,
												caps);
	}
	else
	{
		//
		// Do the mavigate.
		//

		p_person->Genus.Person->pcom_move_ma = MAV_do(
												start_x,
												start_z,
												dest_x,
												dest_z,
												caps);
	}

	//
	// Our first goal.
	//

	PCOM_get_mav_action_pos(
		p_person,
	   &goal_x,
	   &goal_z);

	//
	// Get going.
	//

	set_person_goto_xz(
		p_person,
		goal_x,
		goal_z,
		speed);
}


void PCOM_set_person_move_mav_to_thing(Thing *p_person, Thing *p_target, SLONG speed)
{
	SLONG goal_x;
	SLONG goal_z;

	SLONG start_x;
	SLONG start_y;
	SLONG start_z;

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_FOOT,
	   &start_x,
	   &start_y,
	   &start_z);

	start_x += p_person->WorldPos.X >> 8;
	start_z += p_person->WorldPos.Z >> 8;

	start_x >>= 8;
	start_z >>= 8;

	SLONG dest_x;
	SLONG dest_z;

	UBYTE nav_outof_ware  = FALSE;
	UBYTE nav_inside_ware = FALSE;
	SLONG nav_into_ware   = NULL;

	UBYTE caps = (p_person->Genus.Person->pcom_bent & PCOM_BENT_RESTRICTED) ? MAV_CAPS_GOTO : MAV_CAPS_DARCI;

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_GOTO_THING;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	p_person->Genus.Person->pcom_move_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_move_counter  = 0;

	//
	// We might have to do warehouse navigation.
	//

	if (p_person->Genus.Person->Ware)
	{
		if (p_target->Class != CLASS_PERSON)
		{
			//
			// We should navigate out of the warehouse.
			//

			nav_outof_ware = TRUE;
		}
		else
		if (p_target->Genus.Person->Ware != p_person->Genus.Person->Ware)
		{
			//
			// The person we are mavigating to is inside a different warehouse
			// to me.  Leave the one we are in.
			//

			nav_outof_ware = TRUE;
		}
		else
		{
			//
			// Our target and I are in the same warehouse.
			//

			nav_inside_ware = TRUE;
		}
	}
	else
	if (p_target->Class == CLASS_PERSON && p_target->Genus.Person->Ware)
	{
		//
		// We are not in a warehouse but our target is.  Navigate to the
		// warehouse our target is in.
		//

		nav_into_ware = p_target->Genus.Person->Ware;
	}

	//
	// Do the mavigation call we require.
	//

	if (nav_into_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_enter(
												p_person,
												nav_into_ware,
												caps);
	}
	else
	if (nav_outof_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_exit(
												p_person,
												caps);
	}
	else
	{
		if (!nav_inside_ware && PCOM_should_i_try_to_los_mav_to_person(p_person, p_target))
		{
			//
			// We could try going into 'run towards the person' mode.
			//

			if (!there_is_a_los_mav(
					p_person->WorldPos.X          >> 8,
					p_person->WorldPos.Y + 0x4000 >> 8,
					p_person->WorldPos.Z          >> 8,
					p_target->WorldPos.X          >> 8,
					p_target->WorldPos.Y + 0x4000 >> 8,
					p_target->WorldPos.Z          >> 8))
			{
				//
				// Try just running towards the person and hoping!
				//

				p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_LOSMAV;

				//
				// Get going.
				//

				set_person_goto_xz(
					p_person,
					p_target->WorldPos.X >> 8,
					p_target->WorldPos.Z >> 8,
					speed);

				return;
			}
		}

		PCOM_get_person_dest(
			p_person,
		   &dest_x,
		   &dest_z);

		dest_x >>= 8;
		dest_z >>= 8;

		//
		// Stop it crashing when naving to a vehicle off the map (MikeD)
		//

		SATURATE(dest_x, 0, PAP_SIZE_HI - 1);
		SATURATE(dest_z, 0, PAP_SIZE_HI - 1);

		//
		// Do the mavigate.
		//

		if (nav_inside_ware)
		{
			p_person->Genus.Person->pcom_move_ma = WARE_mav_inside(
													p_person,
													dest_x,
													dest_z,
													caps);
		}
		else
		{
			p_person->Genus.Person->pcom_move_ma = MAV_do(
													start_x,
													start_z,
													dest_x,
													dest_z,
													caps);


		}
	}

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL)
	{
		if (p_person->Genus.Person->pcom_bent & PCOM_BENT_RESTRICTED)
		{
			if (!MAV_do_found_dest)
			{
				//
				// This person can't nav to his target... make him go home.
				//

				PCOM_set_person_ai_homesick(p_person);

				return;
			}
		}
	}

	//
	// Our first goal.
	//

	PCOM_get_mav_action_pos(
		p_person,
	   &goal_x,
	   &goal_z);

	//
	// Get going.
	//

	set_person_goto_xz(
		p_person,
		goal_x,
		goal_z,
		speed);
}


void PCOM_set_person_move_mav_to_waypoint(Thing *p_person, SLONG waypoint, SLONG speed)
{
	SLONG goal_x;
	SLONG goal_z;

	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	SLONG start_x;
	SLONG start_y;
	SLONG start_z;

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_FOOT,
	   &start_x,
	   &start_y,
	   &start_z);

	start_x += p_person->WorldPos.X >> 8;
	start_z += p_person->WorldPos.Z >> 8;

	start_x >>= 8;
	start_z >>= 8;

	UBYTE caps            = (p_person->Genus.Person->pcom_bent & PCOM_BENT_RESTRICTED) ? MAV_CAPS_GOTO : MAV_CAPS_DARCI;
	UBYTE eware           = EWAY_get_warehouse(waypoint);
	UBYTE nav_outof_ware  = FALSE;
	UBYTE nav_inside_ware = FALSE;
	SLONG nav_into_ware   = NULL;

	//
	// We can only go to mapsquares.
	//

	EWAY_get_position(
		waypoint,
	   &dest_x,
	   &dest_y,
	   &dest_z);

	dest_x >>= 8;
	dest_z >>= 8;

	//
	// Store the destination.
	//

	p_person->Genus.Person->pcom_move_arg = waypoint;

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_GOTO_WAYPOINT;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	p_person->Genus.Person->pcom_move_counter  = 0;

	//
	// We might have to do a warehouse navigation.
	// 

	if (p_person->Genus.Person->Ware)
	{
		if (eware != p_person->Genus.Person->Ware)
		{
			nav_outof_ware = TRUE;
		}
		else
		{
			//
			// The waypoint and the person are inside the same warehouse.
			//

			nav_inside_ware = TRUE;
		}
	}
	else
	{
		if (eware)
		{
			//
			// We are not in a warehouse but our target is.
			//

			nav_into_ware = eware;
		}
	}



	//
	// Do the mavigation call we require.
	//

	if (nav_into_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_enter(
												p_person,
												nav_into_ware,
												caps);
	}
	else
	if (nav_outof_ware)
	{
		p_person->Genus.Person->pcom_move_ma = WARE_mav_exit(
												p_person,
												caps);
	}
	else
	{
		//
		// Do the mavigate.
		//

		if (nav_inside_ware)
		{
			p_person->Genus.Person->pcom_move_ma = WARE_mav_inside(
													p_person,
													dest_x,
													dest_z,
													caps);
		}
		else
		{
			p_person->Genus.Person->pcom_move_ma = MAV_do(
													start_x,
													start_z,
													dest_x,
													dest_z,
													caps);
		}
	}

	//
	// Our first goal.
	//

	PCOM_get_mav_action_pos(
		p_person,
	   &goal_x,
	   &goal_z);

	//
	// Get going.
	//

	set_person_goto_xz(
		p_person,
		goal_x,
		goal_z,
		speed);
}


void PCOM_set_person_move_runaway(
		Thing *p_person,
		SLONG  from_x,
		SLONG  from_z)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG goal_x;
	SLONG goal_z;

	SLONG dest_x;
	SLONG dest_z;

	SLONG dist_me;
	SLONG dist_from;

	SLONG tries = 0;

	while(1)
	{
		//
		// Work out a place to MAV to that will (probably) make us run away.
		//

		dx = (p_person->WorldPos.X >> 8) - from_x;
		dz = (p_person->WorldPos.Z >> 8) - from_z;

		dist = abs(dx) + abs(dz) + 1;

		dx  = (dx << 13) / dist;
		dz  = (dz << 13) / dist;

		dx += (Random() & 0x7ff);
		dz += (Random() & 0x7ff);

		dx -= 0x400;
		dz -= 0x400;

		goal_x = (p_person->WorldPos.X >> 8) + dx;
		goal_z = (p_person->WorldPos.Z >> 8) + dz;

		//
		// Stay on the map.
		//

		if (goal_x < 0) {goal_x = -goal_x;}
		if (goal_z < 0) {goal_z = -goal_z;}

		if (goal_x > (PAP_SIZE_HI << PAP_SHIFT_HI)) {goal_x = 2 * (PAP_SIZE_HI << PAP_SHIFT_HI) - goal_x;}
		if (goal_z > (PAP_SIZE_HI << PAP_SHIFT_HI)) {goal_z = 2 * (PAP_SIZE_HI << PAP_SHIFT_HI) - goal_z;}

		//
		// Get going to that place.
		//

		PCOM_set_person_move_mav_to_xz(
			p_person,
			goal_x,
			goal_z,
			PCOM_MOVE_SPEED_RUN);

		//
		// Has this made us start running towards what we are scared of?
		//

		PCOM_get_mav_action_pos(
			p_person,
		   &dest_x,
		   &dest_z);

		dx = abs((p_person->WorldPos.X >> 8) - dest_x);
		dz = abs((p_person->WorldPos.Z >> 8) - dest_z);

		dist_me = QDIST2(dx,dz);

		dx = abs(from_x - dest_x);
		dz = abs(from_z - dest_z);

		dist_from = QDIST2(dx,dz);

		if (dist_from < dist_me)
		{
			//
			// Running towards what we are scared of!
			//

			tries += 1;

			if (tries < 3)
			{
				//
				// Have another go.
				//
			}
			else
			{
				//
				// Give up trying to find somewhere good to run to.
				// 

				break;
			}
		}
		else
		{
			//
			// We are running away properly.
			//

			break;
		}
	}
}

//
// For a person in a vehicle this function sets the pcom_move_substate to
// either PCOM_MOVE_SUBSTATE_GOTO or PCOM_MOVE_SUBSTATE_3PTURN depending on
// the angle of the vehicle relative to the direction they want to go in.
//

void PCOM_set_person_substate_goto_or_3pturn(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dest_x;
	SLONG dest_z;
	SLONG wangle;
	SLONG dangle;

	Thing *p_vehicle;

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	p_vehicle = TO_THING(p_person->Genus.Person->InCar);

	PCOM_get_person_dest(
		p_person,
	   &dest_x,
	   &dest_z);

	//
	// What direction do we want the car to face?
	//

	dx = dest_x - (p_vehicle->WorldPos.X >> 8);
	dz = dest_z - (p_vehicle->WorldPos.Z >> 8);

	wangle  = calc_angle(dx,dz);
	wangle += 1024;
	wangle &= 2047;

	dangle = wangle - p_vehicle->Genus.Vehicle->Angle;

	if (dangle < -1024) {dangle += 2048;}
	if (dangle > +1024) {dangle -= 2048;}

	if (abs(dangle) > 750)
	{
		//
		// Do a three-point turn.
		//

		p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_3PTURN;
	}
	else
	{
		//
		// Just go there.
		//

		p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	}
}

//
// Like the last function, but never does a 3-point turn - so we always go
// towards whichever node is ahead.  Can only be used for DRIVE_DOWN, not DRIVE_TO.
//

void PCOM_set_person_substate_goto(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dest_x;
	SLONG dest_z;
	SLONG wangle;
	SLONG dangle;

	Thing *p_vehicle;

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	p_vehicle = TO_THING(p_person->Genus.Person->InCar);

	PCOM_get_person_dest(
		p_person,
	   &dest_x,
	   &dest_z);

	//
	// What direction do we want the car to face?
	//

	dx = dest_x - (p_vehicle->WorldPos.X >> 8);
	dz = dest_z - (p_vehicle->WorldPos.Z >> 8);

	wangle  = calc_angle(dx,dz);
	wangle += 1024;
	wangle &= 2047;

	dangle = wangle - p_vehicle->Genus.Vehicle->Angle;

	if (dangle < -1024) {dangle += 2048;}
	if (dangle > +1024) {dangle -= 2048;}

	if (abs(dangle) > 750)
	{
		// switch the road nodes around
		p_person->Genus.Person->pcom_move_arg = (p_person->Genus.Person->pcom_move_arg << 8) | (p_person->Genus.Person->pcom_move_arg >> 8);

	}

	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
}

void PCOM_set_person_move_driveto(Thing *p_person, SLONG waypoint)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_DRIVETO;
	p_person->Genus.Person->pcom_move_arg      = waypoint;
	p_person->Genus.Person->pcom_move_counter  = 0;

	//
	// We have to decide whether to drive there or do a three point turn.
	//

	PCOM_set_person_substate_goto_or_3pturn(p_person);
}

void PCOM_set_person_move_park_car(Thing *p_person, SLONG waypoint)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_PARK_CAR;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_move_arg      = waypoint;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

void PCOM_set_person_move_drive_down(Thing *p_person, SLONG n1, SLONG n2)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_DRIVE_DOWN;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	p_person->Genus.Person->pcom_move_arg      = (n1 << 8) | n2;
	p_person->Genus.Person->pcom_move_counter  = 0;

	//
	// We have to decide whether to drive there or do a three point turn.
	//

	PCOM_set_person_substate_goto(p_person);
}

//
// If you are driving down a road- this function will park you on the
// road. If you are driving towards a waypoint, it will just stop you
// straight away.
//

void PCOM_set_person_move_park_car_on_road(Thing *p_person)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_DRIVE_DOWN)
	{
		//
		// Stop near the edge of the road.
		//

		p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_PARK_CAR_ON_ROAD;
		p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
		p_person->Genus.Person->pcom_move_counter  = 0;

		//
		// The 'pcom_move_arg' is the same because that contains the nodes of the road
		// we are driving along.
		//
	}
	else
	{
		//
		// Stop where you are.
		//

		p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_PARK_CAR;
		p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
		p_person->Genus.Person->pcom_move_arg      = NULL;
		p_person->Genus.Person->pcom_move_counter  = 0;
	}
}


#ifdef BIKE

void PCOM_set_person_move_biketo(Thing *p_person, SLONG waypoint)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_BIKETO;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	p_person->Genus.Person->pcom_move_arg      = waypoint;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

void PCOM_set_person_move_park_bike(Thing *p_person, SLONG waypoint)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_PARK_BIKE;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_move_arg      = waypoint;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

void PCOM_set_person_move_bike_down(Thing *p_person, SLONG n1, SLONG n2)
{
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_BIKE_DOWN;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
	p_person->Genus.Person->pcom_move_arg      = (n1 << 8) | n2;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

#endif


void PCOM_set_person_move_goto_thing_slide(Thing *p_person, Thing *p_target)
{
	//
	// Start sliding!
	// 
	if(am_i_a_thug(p_person))
	{
		//
		// only arrest thugs performing slide attacks?
		//
		PCOM_call_cop_to_arrest_me(p_person,1);
	}

	set_person_sliding_tackle(p_person, p_target);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_GOTO_THING_SLIDE;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_move_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_move_counter  = 0;
	
}


void PCOM_renav(Thing *p_person)
{
	SLONG dest_x;
	SLONG dest_y;
	SLONG dest_z;

	Thing *p_target;

	//
	// What are we doing?
	//

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_GOTO_XZ:

			dest_x = (p_person->Genus.Person->pcom_move_arg >> 8) & 0xff;
			dest_z = (p_person->Genus.Person->pcom_move_arg >> 0) & 0xff;

			dest_x <<= 8;
			dest_z <<= 8;

			dest_x += 0x80;
			dest_z += 0x80;

			PCOM_set_person_move_mav_to_xz(
				p_person,
				dest_x,
				dest_z,
				p_person->Genus.Person->GotoSpeed);

			break;

		case PCOM_MOVE_STATE_GOTO_THING:

			p_target = TO_THING(p_person->Genus.Person->pcom_move_arg);

			PCOM_set_person_move_mav_to_thing(
				p_person,
				p_target,
				p_person->Genus.Person->GotoSpeed);

			break;

		case PCOM_MOVE_STATE_GOTO_WAYPOINT:
			
			PCOM_set_person_move_mav_to_waypoint(
				p_person,
				p_person->Genus.Person->pcom_move_arg,
				p_person->Genus.Person->GotoSpeed);

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Returns TRUE if a person has finished navigating.
// 

SLONG PCOM_finished_nav(Thing *p_person)
{
	SLONG dest_x;
	SLONG dest_z;

	if (p_person->State == STATE_IDLE)
	{
		//
		// Although this person hasn't arrived, he isn't doing
		// anything so I guess he's finished!
		//

		return TRUE;
	}

	if (p_person->State == STATE_DANGLING)
	{
		//
		// Can't stop in the middle of a complicated manouvre!
		//

		return FALSE;
	}

	PCOM_get_person_dest(
		p_person,
	   &dest_x,
	   &dest_z);

	dest_x &= 0xffffff00;
	dest_z &= 0xffffff00;

	dest_x |= 0x80;
	dest_z |= 0x80;

	SLONG dist = PCOM_person_dist_from(
					p_person,
					dest_x,
					dest_z);

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING)
	{
		return (dist < 0x60);
	}
	else
	if (p_person->Genus.Person->SlideOdd > 20)
	{
		//
		// This person has been sliding for a while. If we pretend
		// he's arrived where he wants to go- maybe he'll stop.
		//

		if (dist < 0x100)
		{
			p_person->Genus.Person->SlideOdd = 1;

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return (dist < PCOM_ARRIVE_DIST);
	}
}

//
// Make the person stand still.
//

void PCOM_set_person_move_pause(Thing *p_person)
{	
	set_person_idle(p_person);

	//
	// Remember what we are doing and for how long.
	//

	p_person->Genus.Person->pcom_move_state   = PCOM_MOVE_STATE_PAUSE;
	p_person->Genus.Person->pcom_move_counter = 0;
}

//
// Sets the person doing an animation.
// 

void PCOM_set_person_move_animation(Thing *p_person, SLONG anim)
{
	set_person_do_a_simple_anim(p_person, anim);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_ANIM;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

//
// Makes the person do a punch move.
//

void PCOM_set_person_move_punch(Thing *p_person)
{
	turn_to_target_and_punch(p_person);

	//
	// Punching is effectively doing an animation.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_WAIT_CIRCLE;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_PUNCH;
	p_person->Genus.Person->pcom_move_counter  = 0;
}


//
// Makes the person do a kick move.
//

void PCOM_set_person_move_kick(Thing *p_person)
{
	turn_to_target_and_kick(p_person);

	//
	// Kicking is effectively doing an animation.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_WAIT_CIRCLE;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_KICK;
	p_person->Genus.Person->pcom_move_counter  = 0;
}


//
// Makes the person bend down to pick up a special.
// 

void PCOM_set_person_move_pickup_special(Thing *p_person, Thing *p_special)
{
	set_person_special_pickup(p_person);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_ANIM;
	p_person->Genus.Person->pcom_move_counter  = 0;
}



//
// Makes the person do an arrest!
//

UWORD find_arrestee    (Thing *p_person);
void  set_person_arrest(Thing *p_person, SLONG who_to_arrest);

void PCOM_set_person_move_arrest(Thing *p_person)
{
	UWORD index;

	index = PCOM_person_wants_to_kill(p_person);

	if (index == NULL)
	{
		index = find_arrestee(p_person);
	}

	if (index)
	{
		set_person_arrest(p_person, index);

		//
		// Arresting is effectively doing an animation.
		//

		p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_WAIT_CIRCLE;
		p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_ARREST;
		p_person->Genus.Person->pcom_move_counter  = 0;
	}
	else
	{
		PCOM_set_person_move_still(p_person);
	}
}

//
// Makes the person draw his gun.
//

void PCOM_set_person_move_draw_gun(Thing *p_person)
{
	Thing *p_special;

	//
	// Draws a shotgun/AK47 in favour of a pistol.
	//

	if ((p_special = person_has_special(p_person, SPECIAL_SHOTGUN)) && p_special->Genus.Special->ammo)
	{
		set_person_draw_item(p_person, SPECIAL_SHOTGUN);
	}
	else
	if ((p_special = person_has_special(p_person, SPECIAL_AK47)) && p_special->Genus.Special->ammo)
	{
		set_person_draw_item(p_person, SPECIAL_AK47);
	}
	else
	if ((p_special = person_has_special(p_person, SPECIAL_GRENADE)) && p_special->Genus.Special->ammo)
	{
		set_person_draw_item(p_person, SPECIAL_GRENADE);
	}
	else
	{
		set_person_draw_gun(p_person);
	}

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GUNOUT;
	p_person->Genus.Person->pcom_move_counter  = 0;
}
void PCOM_set_person_move_draw_h2h(Thing *p_person,SLONG special)
{
	Thing *p_special;

	{
		set_person_draw_item(p_person,special);
	}

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GUNOUT;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

//
// Makes the person put away his gun.
//

void PCOM_set_person_move_gun_away(Thing *p_person)
{
	if (p_person->Genus.Person->SpecialUse)
	{
		p_person->Genus.Person->SpecialUse = NULL;
		p_person->Draw.Tweened->PersonID&=  ~0xe0;
		//p_person->Draw.Tweened->PersonID   = 0;

		set_person_idle(p_person);
	}
	else
	{
		set_person_gun_away(p_person);
	}

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GUNAWAY;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

//
// Makes a person shoot his gun.
//

void PCOM_set_person_move_shoot(Thing *p_person)
{
	set_person_shoot(p_person,1);

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_SHOOT;
	p_person->Genus.Person->pcom_move_counter  = 0;
//	CONSOLE_text("shoot");
}

//
// do cardinal points first then 
//

UBYTE	gang_angle_priority[]={0,2,6,4,1,7,3,5};
extern	SLONG	get_gangattack(Thing *p_person);


void	check_players_gang(Thing *p_target)
{
	SLONG	gang;
	SLONG	c0,count=0;
	Thing	*p_person;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return;

	for(c0=0;c0<4;c0++)
	{
		if(gang_attacks[gang].Perp[c0])
		{

			p_person=TO_THING(gang_attacks[gang].Perp[c0]);
#ifndef PSX
//			 AENG_world_line_infinite(p_target->WorldPos.X>>8,p_target->WorldPos.Y>>8,p_target->WorldPos.Z>>8,3,0xffffff,p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,0,0xffffff,1);
#endif
			if(p_person->Genus.Person->PlayerID)
			{
				//
				// a player attacking me
				//

				if(p_person->Genus.Person->Mode!=PERSON_MODE_FIGHT)
				{
					remove_from_gang_attack(p_person,p_target);
				}

			}
		}
	}
}

UWORD	count_gang(Thing *p_target)
{
	SLONG	gang;
	SLONG	c0,count=0;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return(0);

	for(c0=0;c0<4;c0++)
	{
		if(gang_attacks[gang].Perp[c0])
		{
			Thing	*p_person;
			p_person=TO_THING(gang_attacks[gang].Perp[c0]);
#ifndef PSX
//			 AENG_world_line_infinite(p_target->WorldPos.X>>8,p_target->WorldPos.Y>>8,p_target->WorldPos.Z>>8,7,0xffff,p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,2,0xff0000,1);
#endif
			count++;
		}
	}
	return(count);
}

extern	SLONG dist_to_target(Thing *p_person_a,Thing *p_person_b);

UWORD	get_any_gang_member(Thing *p_target)
{
	SLONG	gang;
	SLONG	c0,count=0,ret;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return(0);

	for(c0=0;c0<4;c0++)
	{
		if(ret=gang_attacks[gang].Perp[c0])
		{
			if(dist_to_target(p_target,TO_THING(ret))<512)
			{
//				ASSERT(TO_THING(ret)->State!=STATE_DEAD);
//				ASSERT(TO_THING(ret)->State!=STATE_DYING||(TO_THING(ret)->Genus.Person->Flags&FLAG_PERSON_KO));

				return(ret);
			}
			
		}
	}
	return(0);
}

UWORD	get_nearest_gang_member(Thing *p_target)
{
	SLONG	gang;
	SLONG	c0,count=0,ret;
	SLONG	bdist=99999999,best_targ=0,dist;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return(0);

	for(c0=0;c0<4;c0++)
	{
		if(ret=gang_attacks[gang].Perp[c0])
		{
			if(!is_person_ko(TO_THING(ret)))
			if((dist=dist_to_target(p_target,TO_THING(ret)))<bdist)
			{
				best_targ=ret;
				bdist=dist;
			}
			
		}
	}
	return(best_targ);
}

UWORD	find_target_from_gang(Thing *p_target)
{
	SLONG	gang;
	SLONG	c0,perp;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return(0);

	if(perp=gang_attacks[gang].Perp[0])
		return(perp);

	if(perp=gang_attacks[gang].Perp[1])
		return(perp);

	if(perp=gang_attacks[gang].Perp[3])
		return(perp);

	if(perp=gang_attacks[gang].Perp[2])
		return(perp);

	return(0);
}

SLONG	remove_from_gang_attack(Thing *p_person,Thing *p_target)
{
	SLONG	gang;
	SLONG	c0;
	SLONG	removed=0;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return(0);

	for(c0=0;c0<4;c0++)
	{
		if(gang_attacks[gang].Perp[c0]==THING_NUMBER(p_person))
		{
			//
			// I'm allready in his list
			//

			gang_attacks[gang].Perp[c0]=0; //remove myself
			removed=1;
		}
	}
	return(removed);
}

void	scare_gang_attack(Thing *p_target)
{
	SLONG	gang;
	SLONG	c0;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return;

	for(c0=0;c0<4;c0++)
	{
		if(gang_attacks[gang].Perp[c0])
		{
			//
			// I'm allready in his list
			//

			TO_THING(gang_attacks[gang].Perp[c0])->Genus.Person->Agression=-55;
		}
	}
}

void	reset_gang_attack(Thing *p_target)
{
	UWORD	perps[4];
	Thing	*p_person;
	UWORD	gang;
	SLONG	c0;

	gang=p_target->Genus.Person->GangAttack;
	if(gang==0)
		return;

	for(c0=0;c0<4;c0++)
	{
		perps[c0]=gang_attacks[gang].Perp[c0];
		gang_attacks[gang].Perp[c0]=0;
	}

	
	for(c0=0;c0<4;c0++)
	{
		SLONG	dx,dz,reqd_angle;

		if(perps[c0])
		{
			p_person=TO_THING(perps[c0]);

			dx = p_target->WorldPos.X - p_person->WorldPos.X >> 8;
			dz = p_target->WorldPos.Z - p_person->WorldPos.Z >> 8;


			//
			// The angle between us and our target- where we want to push
			// into the gang structure.
			//

			reqd_angle   = calc_angle(dx,dz)+256;
			reqd_angle  &= 2047;
			reqd_angle >>= 9;

			push_into_attack_group_at_angle(p_person,(SLONG)gang,reqd_angle);

		}
	}


}
void	process_gang_attack(Thing *p_person,Thing *p_target)
{
	SLONG	gang;
	SLONG	c0;
	SLONG	left,right,lleft,rright;
	SLONG	me;
	SLONG	attack_count=0;

	me=THING_NUMBER(p_person);

	gang=p_target->Genus.Person->GangAttack;

	if(p_person->SubState==SUB_STATE_CIRCLING_CIRCLE)
	{
		//
		// I am attacking, if there's more than me attacking then make me backoff
		//
		for(c0=0;c0<4;c0++)
		{
			SLONG	perp;
			perp=gang_attacks[gang].Perp[c0];
			if(perp && perp!=me)
			{
				switch(TO_THING(perp)->SubState)
				{
					case	SUB_STATE_CIRCLING_CIRCLE:
						TO_THING(perp)->Genus.Person->Agression=-100;
						attack_count++;
						break;

				}
			}
		}
/*
		if(attack_count>=1)
		{
			p_person->Genus.Person->Agression=-60;
		}
*/

	}

	return;
/*

	for(c0=0;c0<4;c0++)
	{
		if(gang_attacks[gang].Perp[c0]==me)
		{
			left=gang_attacks[gang].Perp[(c0-1)&3];
			right=gang_attacks[gang].Perp[(c0+1)&3];
			if(left==0 && right==0)
				return;

			if(left==0&& right)
			{
				lleft=gang_attacks[gang].Perp[(c0-2)&3];
				if(lleft==0)
				{

					gang_attacks[gang].Perp[c0]=0;
					p_person->Genus.Person->AttackAngle=(c0-1)&3;
					gang_attacks[gang].Perp[(c0-1)&3]=me;

					//
					// change pos so make me backoff
					//
					p_person->Genus.Person->Agression=-60-(c0<<2);
				}
			}
			else
			if(left&&right==0)
			{
				rright=gang_attacks[gang].Perp[(c0+2)&3];
				if(rright==0)
				{
					gang_attacks[gang].Perp[c0]=0;
					p_person->Genus.Person->AttackAngle=(c0+1)&3;
					ASSERT(gang_attacks[gang].Perp[(c0+1)&3]==0);
					gang_attacks[gang].Perp[(c0+1)&3]=me;
					//
					// change pos so make me backoff
					//
					p_person->Genus.Person->Agression=-60-(c0<<2);
				}
			}
			return;


		}
	}
*/
}

void	push_into_attack_group_at_angle(Thing *p_person,SLONG gang,SLONG reqd_angle)
{
	SLONG	c0=4;
	Thing	*p_copy;

	MSG_add("try push in  at %d    [%d %d %d %d %d %d %d %d] \n",reqd_angle,gang_attacks[gang].Perp[0],gang_attacks[gang].Perp[1],gang_attacks[gang].Perp[2],gang_attacks[gang].Perp[3],gang_attacks[gang].Perp[4],gang_attacks[gang].Perp[5],gang_attacks[gang].Perp[6],gang_attacks[gang].Perp[7]);
	
	if(gang_attacks[gang].Perp[(reqd_angle)&3]!=0)
	for(c0=1;c0<=2;c0++)
	{
		ASSERT(gang_attacks[gang].Perp[(reqd_angle+c0)&3]!=THING_NUMBER(p_person));
		ASSERT(gang_attacks[gang].Perp[(reqd_angle-c0)&3]!=THING_NUMBER(p_person));

		if(gang_attacks[gang].Perp[(reqd_angle+c0)&3]==0)
		{
			//
			// go pos
			//
			MSG_add(" push in at position %d shoving %d peeps ",reqd_angle,c0);
			while(c0>0)
			{
				//
				// shunt everyone arround the circle
				//
				gang_attacks[gang].Perp[(reqd_angle+c0)&3]=gang_attacks[gang].Perp[(reqd_angle+c0-1)&3];
				p_copy=TO_THING(gang_attacks[gang].Perp[(reqd_angle+c0-1)&3]);

				p_copy->Genus.Person->AttackAngle=(reqd_angle+c0)&3;
				c0--;
			}

			break;
		}
		else
		if(gang_attacks[gang].Perp[(reqd_angle-c0+8)&3]==0)
		{
			//
			// go neg
			//
			MSG_add(" push in at position %d shoving NEG %d peeps ",reqd_angle,c0);
			while(c0>0)
			{
				
				//
				// shunt everyone arround the circle
				//
				gang_attacks[gang].Perp[(reqd_angle-c0+8)&3]=gang_attacks[gang].Perp[(reqd_angle-c0+1+8)&3];
				p_copy=TO_THING(gang_attacks[gang].Perp[(reqd_angle-c0+1+8)&3]);

				p_copy->Genus.Person->AttackAngle=(reqd_angle-c0+8)&3;
				c0--;
			}

//			gang_attacks[gang].Perp[(reqd_angle+c0)&3]=THING_NUMBER(p_person);
			break;
		}
		if(c0==4)
			MSG_add("FAILED to push in\n");
	}

	//
	// everywhere is full so use the angle you want with someone else there as well
	// or everyone has been pushed round to make room for you.

	gang_attacks[gang].Perp[reqd_angle&3]=THING_NUMBER(p_person);
	p_person->Genus.Person->AttackAngle=reqd_angle;

}

void	PCOM_new_gang_attack(Thing *p_person, Thing *p_target)
{
	SLONG gang;
	SLONG c0;

	SLONG reqd_angle;
	SLONG dx;
	SLONG dz;
	SLONG angle;

	dx = p_target->WorldPos.X - p_person->WorldPos.X >> 8;
	dz = p_target->WorldPos.Z - p_person->WorldPos.Z >> 8;


	//
	// The angle between us and our target- where we want to push
	// into the gang structure.
	//

	reqd_angle   = calc_angle(dx,dz);
	reqd_angle  &= 2047;
	reqd_angle >>= 9;

	//
	// Mike's new addition to the circeling system based on alloting angles
	// to bad guys.  Each target holds a structure for where each attacker should be.
	//

	if (p_target->Genus.Person->GangAttack == 0)
	{
		//
		// Create the gang structure for this target if she hasn't got one already.
		//

		gang = get_gangattack(p_target);
	}
	else
	{

		gang = p_target->Genus.Person->GangAttack;
	}

	//
	// Make sure we are not already in targets person gang attack structure.
	//

	for (c0 = 0; c0 < 4; c0++)
	{
		if (gang_attacks[gang].Perp[c0] == THING_NUMBER(p_person))
		{
			//
			// I'm allready in her list.
			//

			gang_attacks[gang].Perp[c0] = 0; //remove myself
		}
	}

	//
	// Now slot yourself into the gang structure.
	//

	if (gang_attacks[gang].Perp[reqd_angle] == 0)
	{
		//
		// my best angle is available hoorah
		//

		gang_attacks[gang].Perp[reqd_angle] = THING_NUMBER(p_person);
		p_person->Genus.Person->AttackAngle = reqd_angle;
	}
	else
	{
		//
		// no room so push in
		//

		push_into_attack_group_at_angle(
			p_person,
			gang,
			reqd_angle);
	}

}

//
// Makes the person circle around a target.
//

void PCOM_set_person_move_circle(Thing *p_person, Thing *p_target)
{
	set_person_circle(p_person, p_target);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state   = PCOM_MOVE_STATE_CIRCLE;
	p_person->Genus.Person->pcom_move_arg     = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_move_counter = 0;

	PCOM_new_gang_attack(p_person, p_target);
}

//
// Makes a person get in a car.
//

void PCOM_set_person_move_getincar(Thing *p_person, Thing *p_vehicle, SLONG am_i_a_passenger, SLONG door)
{
	ASSERT(door == 0 || door == 1);

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	if (am_i_a_passenger)
	{
		set_person_passenger_in_vehicle(p_person, p_vehicle, door);
	}
	else
	{
		set_person_enter_vehicle(p_person, p_vehicle, door);
	}

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GETINCAR;
	p_person->Genus.Person->pcom_move_arg      = THING_NUMBER(p_vehicle);
	p_person->Genus.Person->pcom_move_counter  = 0;
}

//
// Makes a person leave the car he is in.
//

void PCOM_set_person_move_leavecar(Thing *p_person)
{
	ASSERT(p_person->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER));

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	set_person_exit_vehicle(p_person);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_LEAVECAR;
	p_person->Genus.Person->pcom_move_arg      = 0;
	p_person->Genus.Person->pcom_move_counter  = 0;
}

//
// Go back to do what you normally do.
//

extern	void PCOM_set_person_ai_navtokill(Thing *p_person, Thing *p_target);
void PCOM_set_person_ai_normal(Thing *p_person)
{
	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_NORMAL;
	p_person->Genus.Person->pcom_ai_substate = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	PCOM_set_person_move_still(p_person);
}

//
// If a person gets knocked out (by some external event like a car or shockwave)
// then call this function. It put a person in a state where they wait until
// recovered and then go back to doing their normal thing.
//

void PCOM_set_person_ai_knocked_out(Thing *p_person)
{
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_KNOCKEDOUT;
	p_person->Genus.Person->pcom_ai_substate = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_ANIMATION;
	p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_ANIM;
	p_person->Genus.Person->pcom_move_counter  = 0;
}



//
// Make a cop try to arrest some naughty people.
// 

void PCOM_set_person_ai_arrest(Thing *p_person, Thing *p_target)
{
	if(p_target->Genus.Person->PersonType==PERSON_DARCI)
		ASSERT(0);

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_ARREST;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_ai_substate = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;
	
	//
	// Look at who you are arresting.
	//

	set_face_thing(
		p_person,
		p_target);

//	MFX_play_thing(THING_NUMBER(p_person),S_HELLOALLOALLO,MFX_REPLACE,p_person);
	MFX_play_thing(THING_NUMBER(p_person),SOUND_Range(S_COP_ARREST_START,S_COP_ARREST_END),MFX_REPLACE,p_person);

	PCOM_set_person_move_mav_to_thing(p_person, p_target, PCOM_MOVE_SPEED_RUN);
}




void PCOM_set_person_ai_kill_person(Thing *p_person, Thing *p_target, SLONG alert_gang)
{

	if(p_person->Genus.Person->PersonType==PERSON_CIV)
	{
		ASSERT(0);
	}
	if(p_target->Genus.Person->PersonType==PERSON_DARCI)
	{
		if(p_person->Genus.Person->PersonType==PERSON_COP)
			ASSERT(0);
	}

	if ((p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
		return;

	if(am_i_a_thug(p_person))
	{
		//
		// only arrest thugs?
		//
		if(!am_i_a_thug(p_target)) //thug kills innocent
			PCOM_call_cop_to_arrest_me(p_person,1);
	}


/*
	//
	// We are too busy with someone else
	//

	if(p_person->Genus.Person->pcom_ai_state    == PCOM_AI_STATE_KILLING)
		return;
	if(p_person->Genus.Person->State    == STATE_CIRCLING)
		return;
*/

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	//
	// Face the target.
	//

	set_face_thing(p_person, p_target);

	if (p_target->Genus.Person->PlayerID)
	{
		//
		// Wants to hit the player
		//

		track_enemy(p_person);
	}

	//
	// Circle around our target.
	//

	PCOM_set_person_move_circle(p_person, p_target);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_KILLING;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);

	//
	// Tell our target she is under attack.
	//

	if (alert_gang)
	{
		if (p_person->Genus.Person->pcom_bent & PCOM_BENT_GANG)
		{
			//
			// Alert other people in our gang. Let them know to
			// help me now I am in a fight.
			//

			PCOM_alert_my_gang_to_a_fight(p_person, p_target);
		}
	}

	//
	// Tell the low-level person system that this person has a target...
	//

	p_person->Genus.Person->Target = THING_NUMBER(p_target);
}

void PCOM_set_person_ai_homesick(Thing *p_person)
{
	SLONG home_x = (p_person->Genus.Person->HomeX << 0);// + 0x80;
	SLONG home_z = (p_person->Genus.Person->HomeZ << 0);// + 0x80;

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));

		p_person->Genus.Person->Target = NULL;
	}

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_HOMESICK;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NONE;

	PCOM_set_person_move_mav_to_xz(
		p_person, 
		home_x,
		home_z,
		PCOM_MOVE_SPEED_WALK);
}


void PCOM_set_person_ai_leavecar(Thing *p_person, SLONG excar_state, SLONG excar_substate, SLONG excar_arg)
{
	Thing *p_vehicle;

	ASSERT(p_person->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER));

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	p_vehicle = TO_THING(p_person->Genus.Person->InCar);

	//
	// This is what we are doing now...
	//

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_LEAVECAR;
	p_person->Genus.Person->pcom_ai_arg      = 0;
	p_person->Genus.Person->pcom_ai_substate = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	//
	// And this is what we'll do once we've got out of the car.
	//

	p_person->Genus.Person->pcom_ai_excar_state    = excar_state;
	p_person->Genus.Person->pcom_ai_excar_substate = excar_substate;
	p_person->Genus.Person->pcom_ai_excar_arg      = excar_arg;

	if (p_vehicle->Velocity && (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING))
	{
		//
		// Park the car.
		//

		PCOM_set_person_move_park_car_on_road(p_person);
	}
	else
	{
		//
		// Get out of the car.
		//

		PCOM_set_person_move_leavecar(p_person);
	}
}


//
// Makes the person go off and investiage an odd event.
//

void PCOM_set_person_ai_investigate(
		Thing *p_person,
		SLONG  odd_x,
		SLONG  odd_z)
{
	if ((p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
		return;
	//
	// Remember what we are doing.
	//
//	FC_cam[1].focus=p_person;
//	ASSERT(0);
	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	odd_x >>= 8;
	odd_z >>= 8;

	p_person->Genus.Person->pcom_ai_arg      = (odd_x << 8) | (odd_z);
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_INVESTIGATING;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_SUPRISED;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	//
	// Turn to face the unusual sound.
	//

	set_face_pos(
		p_person,
		(odd_x << 8) + 0x80,
		(odd_z << 8) + 0x80);

	//
	// Start off starting blankly at the sound.
	//

	PCOM_set_person_move_still(p_person);
}

//
// Makes the person try to run away.
//
void PCOM_set_person_ai_flee_place(
		Thing *p_person,
		SLONG  scary_x,	// The place where the scary thing is.
		SLONG  scary_z)
{
	if ((p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
		return;
	//
	// Make this person scream in terror!
	//
/*
	play_quick_wave(
		p_person,
		S_ARGH,
		WAVE_PLAY_INTERUPT);
*/
	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	if (p_person->Genus.Person->pcom_ai_state    == PCOM_AI_STATE_FLEE_PLACE &&
		p_person->Genus.Person->pcom_ai_substate == PCOM_AI_SUBSTATE_LEGIT)
	{
		//
		// Just update where we are running from.
		//

		scary_x >>= 8;
		scary_z >>= 8;

		p_person->Genus.Person->pcom_ai_arg = (scary_x << 8) | (scary_z);

		return;
	}

	//
	// Remember what we are doing.
	//

	scary_x >>= 8;
	scary_z >>= 8;

	p_person->Genus.Person->pcom_ai_arg      = (scary_x << 8) | (scary_z);
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_FLEE_PLACE;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_SUPRISED;
	p_person->Genus.Person->pcom_ai_counter  = 0;
	
	//
	// Turn to face the unusual thing.
	//

	set_face_pos(
		p_person,
		(scary_x << 8) + 0x80,
		(scary_z << 8) + 0x80);

	if (p_person->State == STATE_HIT_RECOIL)
	{
		//
		// Have to wait to finish recoiling...
		//
	}
	else
	{
		//
		// Start off starting blankly at the sound.
		//

		PCOM_set_person_move_still(p_person);
	}
}


void PCOM_set_person_ai_flee_person(Thing *p_person,Thing *p_scary)
{
	if (p_person->Genus.Person->pcom_ai != PCOM_AI_FLEE_PLAYER)
	{
		if(am_i_a_thug(p_person)&&((p_person->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK)))
		{
			return;
		}
	}

	if (p_person->Genus.Person->pcom_ai == PCOM_AI_CIV)
	{
		//
		// Civs remember who scared them.
		// 

		p_person->Genus.Person->pcom_ai_memory = THING_NUMBER(p_scary);

		if (p_person->Genus.Person->pcom_ai_state != PCOM_AI_STATE_FLEE_PERSON)
		{
			//
			// Make this person scream in terror!
			//

//			MFX_play_thing(THING_NUMBER(p_person),S_ARGH,MFX_REPLACE,p_person);
		}
	}

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	//
	// If this person is in a car, then they must get out first.
	//

	if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
	{
		PCOM_set_person_ai_leavecar(p_person, PCOM_EXCAR_FLEE_PERSON, 0, THING_NUMBER(p_scary));

		return;
	}

	if (p_person->Genus.Person->pcom_ai_state    == PCOM_AI_STATE_FLEE_PERSON &&
		p_person->Genus.Person->pcom_ai_substate == PCOM_AI_SUBSTATE_LEGIT)
	{
		//
		// Just update who we are running from.
		//

		p_person->Genus.Person->pcom_ai_arg = THING_NUMBER(p_scary);

		return;
	}

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_scary);
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_FLEE_PERSON;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_SUPRISED;
	p_person->Genus.Person->pcom_ai_counter  = 0;
	
	//
	// Turn to face the scary person.
	//

	set_face_pos(
		p_person,
		p_scary->WorldPos.X >> 8,
		p_scary->WorldPos.Z >> 8);

	if (p_person->State == STATE_HIT_RECOIL)
	{
		//
		// Have to wait to finish recoiling...
		//
	}
	else
	{
		//
		// Start off starting blankly at the dangerous person.
		//

		PCOM_set_person_move_still(p_person);
	}
}

void PCOM_set_person_ai_aimless(Thing *p_person)
{
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_AIMLESS;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_ai_arg      = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	PCOM_set_person_move_still(p_person);
}

//
// Goes into NAVTOKILL mode but starts off trying to shoot the target.
//

void PCOM_set_person_ai_navtokill_shoot(Thing *p_person, Thing *p_target)
{
	if(p_target->Genus.Person->PersonType==PERSON_DARCI)
	{
		if(p_person->Genus.Person->PersonType==PERSON_COP)
			ASSERT(0);
	}
	if(p_person->Genus.Person->PersonType==PERSON_CIV)
	{
		ASSERT(0);
	}
	PCOM_set_person_move_still(p_person);

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_NAVTOKILL;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_AIMING;
	p_person->Genus.Person->pcom_ai_counter  = 0;
	p_person->Genus.Person->Target           = THING_NUMBER(p_target);
}

SLONG PCOM_target_sprinting_towards_me(Thing *p_person, Thing *p_target)
{
	SLONG dangle = get_dangle(p_target, p_person);

	if (p_target->Genus.Person->Mode == PERSON_MODE_SPRINT)
	{
		#define PCOM_SPRINTATME_DANGLE 512

		if (dangle <        PCOM_SPRINTATME_DANGLE ||
			dangle > 2048 - PCOM_SPRINTATME_DANGLE)
		{
			return TRUE;
		}
	}

	return FALSE;
}


void PCOM_set_person_ai_navtokill(Thing *p_person, Thing *p_target)
{
	if(p_target->Genus.Person->PersonType==PERSON_DARCI)
	{
		if(p_person->Genus.Person->PersonType==PERSON_COP)
			ASSERT(0);
	}
	if(p_person->Genus.Person->PersonType==PERSON_CIV)
	{
//		ASSERT(0);
	}
	if ((p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
		return;

	if(am_i_a_thug(p_person))
	{
		//
		// only arrest thugs?
		//
		if(!am_i_a_thug(p_target)) //thug kills innocent
			PCOM_call_cop_to_arrest_me(p_person,1);
	}


	PCOM_set_person_move_mav_to_thing(p_person, p_target, PCOM_MOVE_SPEED_RUN);

	if (PCOM_target_could_shoot_me(p_person, p_target) )
	{
		//
		// What shall we do... run away, shoot back, or try and beat him up anyway?
		//

		if (PCOM_person_has_any_sort_of_gun(p_person) ||(am_i_a_thug(p_person)&&(p_person->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK)))
		{
			PCOM_set_person_ai_navtokill_shoot(p_person, p_target);

			return;
		}
		else
		{
			//
			// Gun vs no gun... 
			//

//			if(!am_i_a_thug(p_person)||(!(p_gang->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK)))
			if ((Random() & 0x4) && !(p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER) && (p_person->Genus.Person->PersonType!=PERSON_COP))
			{
				PCOM_set_person_ai_flee_person(p_person, p_target);
				
				return;				
			}
		}
	}
	else
	if (PCOM_target_sprinting_towards_me(p_person, p_target))
	{
		if (PCOM_person_has_any_sort_of_gun(p_person))
		{
			PCOM_set_person_ai_navtokill_shoot(p_person, p_target);

			return;
		}
	}

	if (p_person->Genus.Person->pcom_bent & PCOM_BENT_RESTRICTED)
	{
		//
		// If you are restricted movement and you can't get to your
		// target for certain then give up and go back to your normal
		// routine.
		//

		if (!MAV_do_found_dest)
		{
			PCOM_set_person_ai_normal(p_person);

			return;
		}
	}

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_NAVTOKILL;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING;
	p_person->Genus.Person->pcom_ai_counter  = 0;
	p_person->Genus.Person->Target           = THING_NUMBER(p_target);
}




void PCOM_set_person_ai_follow(Thing *p_person, Thing *p_target)
{
	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	PCOM_set_person_move_mav_to_thing(p_person, p_target, PERSON_SPEED_RUN);

	if (p_target->Genus.Person->PlayerID && p_person->Genus.Person->pcom_move == PCOM_MOVE_FOLLOW)
	{
		//
		// If a following person is going to have great difficulty finding
		// the player- then make the person start wandering around.
		//

		if (p_person->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD ||
			p_person->Genus.Person->pcom_ai == PCOM_AI_CIV)
		{
			SLONG dx = p_person->WorldPos.X - p_target->WorldPos.X >> 8;
			SLONG dy = p_person->WorldPos.Y - p_target->WorldPos.Y >> 8;
			SLONG dz = p_person->WorldPos.Z - p_target->WorldPos.Z >> 8;

			SLONG dist = abs(dx) + abs(dz) + abs(dy + dy);

			if (dist > 0x600)
			{
				if (!MAV_do_found_dest)
				{
					//
					// Dont try to follow the player if it's going to be
					// too difficult!
					//

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_CANTFIND;

					return;
				}
			}
		}
	}

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_FOLLOWING;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_ai_counter  = 0;
}

void PCOM_set_person_ai_findcar(Thing *p_person, UWORD car)
{
	SLONG speed;

	Thing *p_car;

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	if (car == NULL)
	{
		SLONG i;
		SLONG dx;
		SLONG dz;
		SLONG num;
		SLONG dist;
		SLONG best_dist = INFINITY;

		//
		// Look for a nearby car we can use.
		//

		num = THING_find_sphere(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Y >> 8,
				p_person->WorldPos.Z >> 8,
				0xc00,
				THING_array,
				THING_ARRAY_SIZE,
				1 << CLASS_VEHICLE);

		for (i = 0; i < num; i++)
		{
			p_car = TO_THING(THING_array[i]);

			ASSERT(p_car->Class == CLASS_VEHICLE);

			if (p_car->Genus.Vehicle->Driver)
			{
				//
				// Car already driven by someone.
				//
			}
			else
			if (p_car->State == STATE_DEAD)
			{
				//
				// Car is damaged.
				// 
			}
			else
			if (p_car->Genus.Vehicle->key != SPECIAL_NONE && !person_has_special(p_person, p_car->Genus.Vehicle->key))
			{
				//
				// This car is locked and we don't have the key.
				//
			}
			else
			{
				dx = abs(p_car->WorldPos.X - p_person->WorldPos.X);
				dz = abs(p_car->WorldPos.Z - p_person->WorldPos.Z);

				dist = dx + dz;

				if (dist < best_dist)
				{
					best_dist = dist;
					car       = THING_array[i];
				}
			}
		}
	}

	if (car == NULL)
	{
		//
		// Couldn't find a car- start wandering around aimlessly.
		//

		PCOM_set_person_ai_aimless(p_person);
	}
	else
	{
		//
		// Start navigating to the car door.
		//

		p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_FINDCAR;
		p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GOTOCAR;
		p_person->Genus.Person->pcom_ai_arg      = car;
		p_person->Genus.Person->pcom_ai_counter  = 0;

		if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
		{
			speed = PERSON_SPEED_WALK;
		}
		else
		{
			speed = PERSON_SPEED_RUN;
		}

		PCOM_set_person_move_mav_to_thing(
			p_person,
			TO_THING(car),
			speed);
	}
}

#ifdef BIKE

void PCOM_set_person_ai_findbike(Thing *p_person)
{
	SLONG bike;
	SLONG speed;

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	//
	// Look for a nearby bike.
	//

	bike = THING_find_nearest(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Y >> 8,
				p_person->WorldPos.Z >> 8,
				0xc00,
				1 << CLASS_BIKE);

	if (bike == NULL || TO_THING(bike)->Genus.Bike->mode != BIKE_MODE_PARKED)
	{
		//
		// Couldn't find a car- start wandering around aimlessly.
		//

		PCOM_set_person_ai_aimless(p_person);
	}
	else
	{
		Thing *p_bike = TO_THING(bike);

		//
		// If someone is driving the bike. Kill them so you can get onto it!
		//

		if (p_bike->Genus.Bike->driver)
		{
			ASSERT(TO_THING(p_bike->Genus.Bike->driver)->Class == CLASS_PERSON);

			//
			// This flag tell PCOM_process_killing() that this biker is killing
			// someone to nick his bike- rather than just to kill him.
			//

			p_person->Genus.Person->Flags |= FLAG_PERSON_KILL_WITH_A_PURPOSE;

			PCOM_set_person_ai_kill_person(p_person, TO_THING(p_bike->Genus.Bike->driver));
		}
		else
		{
			//
			// Start navigating to the bike.
			//

			p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_FINDBIKE;
			p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GOTOBIKE;
			p_person->Genus.Person->pcom_ai_arg      = bike;
			p_person->Genus.Person->pcom_ai_counter  = 0;

			if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
			{
				speed = PERSON_SPEED_WALK;
			}
			else
			{
				speed = PERSON_SPEED_RUN;
			}

			PCOM_set_person_move_mav_to_thing(
				p_person,
				TO_THING(bike),
				speed);
		}
	}
}

#endif

void PCOM_set_person_ai_bdeactivate(Thing *p_person, Thing *p_bomb)
{
	SLONG speed;

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_BDEACTIVATE;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_bomb);
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GOTOBOMB;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	//
	// Start navigating to the bomb.
	//

	if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
	{
		speed = PERSON_SPEED_WALK;
	}
	else
	{
		speed = PERSON_SPEED_RUN;
	}

	PCOM_set_person_move_mav_to_thing(
		p_person,
		p_bomb,
		speed);
}


void PCOM_set_person_ai_snipe(Thing *p_person, Thing *p_target)
{
	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}
	#ifndef NDEBUG

	//
	// All snipers must have a gun!
	//

	if (!PCOM_person_has_any_sort_of_gun(p_person))
	{
		#ifndef NDEBUG

		CONSOLE_text("Sniper doesn't have a gun. I'll give him one anyway.");

		#endif

		p_person->Flags |= FLAGS_HAS_GUN;
	}

	#endif

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_SNIPE;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LOOK;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_ai_counter  = 0;

//	PCOM_set_person_move_still(p_person);

	PCOM_set_person_move_draw_gun(p_person);

}


void PCOM_set_person_ai_warm_hands(Thing *p_person)
{
	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}
	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_WARM_HANDS;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GOTOFIRE;
	p_person->Genus.Person->pcom_ai_arg      = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	PCOM_set_person_move_pause(p_person);
}

void PCOM_set_person_ai_hands_up(Thing *p_person, Thing *p_cop)
{
	UWORD anim;

	//
	// Face the person you are being aimed at by
	//

	set_face_thing(p_person, p_cop);

	//
	// Which anim shall we do?
	//


	PCOM_set_person_move_animation(p_person, ANIM_HANDS_UP);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_HANDS_UP;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_cop);
	p_person->Genus.Person->pcom_ai_counter  = 0;
	p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;

}

void PCOM_set_person_ai_talk_to(Thing *p_person, Thing *p_person_talked_at, UBYTE talk_substate, UBYTE stay_looking_at_eachother)
{
	UWORD anim;

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	//
	// Face the person you are talking to.
	//

	set_face_thing(p_person, p_person_talked_at);

	//
	// Which anim shall we do?
	//

	switch(talk_substate)
	{
		case PCOM_AI_SUBSTATE_TALK_ASK:	   anim = ANIM_TALK_ASK;    break;
		case PCOM_AI_SUBSTATE_TALK_TELL:   anim = ANIM_TALK_TELL;   break;
		case PCOM_AI_SUBSTATE_TALK_LISTEN: anim = ANIM_TALK_LISTEN; break;

		default:
			ASSERT(0);
			break;
	}
	if(person_holding_2handed(p_person)&& p_person->Genus.Person->PersonType!=PERSON_ROPER)
		anim=ANIM_SHOTGUN_IDLE;

	PCOM_set_person_move_animation(p_person, anim);

	//
	// Remember what we are doing.
	//

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_TALK;
	p_person->Genus.Person->pcom_ai_substate = talk_substate;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_person_talked_at);
	p_person->Genus.Person->pcom_ai_counter  = 0;

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_NO_RETURN_TO_NORMAL;

	if (stay_looking_at_eachother)
	{
		p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
	}
}


void PCOM_set_person_ai_hitch(Thing *p_person, Thing *p_vehicle)
{
	SLONG speed;

	if (p_person->Genus.Person->Target)
	{
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	//
	// Start navigating to the car door.
	//

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_HITCH;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GOTOCAR;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_vehicle);
	p_person->Genus.Person->pcom_ai_counter  = 0;

	if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
	{
		speed = PERSON_SPEED_WALK;
	}
	else
	{
		speed = PERSON_SPEED_RUN;
	}

	PCOM_set_person_move_mav_to_thing(
		p_person,
		p_vehicle,
		speed);
}


//
// Makes a person start taunting someone.
//

void PCOM_set_person_ai_taunt(Thing *p_person, Thing *p_target)
{
	if ((p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
		return;
	//
	// cops shouldnt taunt, I've seen them do it with my own eyes MIKED
	//
	if(p_person->Genus.Person->PersonType==PERSON_COP)
		return;

	if(p_person->SubState==SUB_STATE_GRAPPLEE || p_person->SubState == SUB_STATE_GRAPPLE_HELD)
	{
		//
		// No taunt if your being grappled
		//

		return;
	}
	//
	// People can taunt in the middle of a fight!
	//

	if (p_person->Genus.Person->Target)
	{
		//
		// I disagree you should concentrate on the job MikeD.
		//
		//return;
		remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
	}

	if (PCOM_target_could_shoot_me(p_person, p_target))
	{
		//
		// What shall we do... run away, shoot back, or try and beat him up anyway?
		//

		if (PCOM_person_has_any_sort_of_gun(p_person)||(Random()&3)==0)
		{
			PCOM_set_person_ai_navtokill_shoot(p_person, p_target);

			return;
		}
		else
		{
			//
			// Gun vs no gun... 
			//

			if (Random() & 0x4)
			{
				PCOM_set_person_ai_flee_person(p_person, p_target);
				
				return;				
			}
		}
	}

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_TAUNT;
	p_person->Genus.Person->pcom_ai_arg      = THING_NUMBER(p_target);
	p_person->Genus.Person->pcom_ai_substate = 0;
	p_person->Genus.Person->pcom_ai_counter  = 0;

	//
	// Look at who you are taunting.
	//

	set_face_thing(
		p_person,
		p_target);

//	MFX_play_thing(THING_NUMBER(p_person),S_WANKER,MFX_REPLACE,p_person);

	PCOM_set_person_move_animation(p_person, ANIM_WANKER);
}

//
// The people the summoner is going to use!
// 

#ifdef PSX
#define PCOM_SUMMON_NUM_BODIES 2
#else
#define PCOM_SUMMON_NUM_BODIES 4
#endif

UWORD PCOM_summon[PCOM_SUMMON_NUM_BODIES];

void PCOM_set_person_ai_summon(Thing *p_person)
{
	SLONG i;
	SLONG num;
	SLONG bodies;

	//
	// Look around for the bodies...
	//

	num = THING_find_sphere(
			p_person->WorldPos.X >> 8,
			p_person->WorldPos.Y >> 8,
			p_person->WorldPos.Z >> 8,
			0x800,
			THING_array,
			THING_ARRAY_SIZE,
			1 << CLASS_PERSON);
	
	//
	// Clear out the body array.
	//

	memset(PCOM_summon, 0, sizeof(PCOM_summon));
	
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

				ASSERT(WITHIN(bodies, 0, PCOM_SUMMON_NUM_BODIES - 1));

				PCOM_summon[bodies] = THING_array[i];

				bodies += 1;

				if (bodies == PCOM_SUMMON_NUM_BODIES)
				{
					//
					// Found all our bodies!
					//

					break;
				}
			}
		}
	}

	//
	// Start floating!
	//

	set_person_float_up(p_person);

	p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_SUMMON;
	p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_SUMMON_START;
}

//
// Returns an item this person should pick up or NULL if this
// person is not near enough an item or already has one.
//
extern	BOOL PersonIsMIB(Thing* p_person);
/*
BOOL PersonIsMIB(Thing* p_person)
{
	return (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3);
};
*/
BOOL PersonIsMIB(Thing* p_person);

Thing *PCOM_is_there_an_item_i_should_get(Thing *p_person)
{
	UWORD ans;

	if (PersonIsMIB(p_person))
	{
		//
		// MIBs have an ak47 for an arm.
		//

		return NULL;
	}

	if (p_person->Genus.Person->SpecialList || (p_person->Flags & FLAGS_HAS_GUN))
	{
		//
		// This person has a special or a gun, so he doesn't
		// need to pick up anything... even health!
		//

		return NULL;
	}

	if (p_person->Genus.Person->Ware)
	{
		//
		// People in warehouses don't pickup specials...
		//

		return NULL;
	}

	ans = THING_find_nearest(
			p_person->WorldPos.X >> 8,
			p_person->WorldPos.Y >> 8,
			p_person->WorldPos.Z >> 8,
			0x300,
			1 << CLASS_SPECIAL);

	if (ans)
	{
		Thing *p_special = TO_THING(ans);

		switch(p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_GUN:
			case SPECIAL_SHOTGUN:
			case SPECIAL_AK47:
			case SPECIAL_BASEBALLBAT:
			case SPECIAL_KNIFE:

				//
				// These are the special types we can pickup.
				//

				break;

			default:
				return NULL;
		}

		//
		// Only pick up the special if the person can navigate to it ok.
		//

		if (!there_is_a_los_mav(
				p_person ->WorldPos.X          >> 8,
				p_person ->WorldPos.Y + 0x4000 >> 8,
				p_person ->WorldPos.Z          >> 8,
				p_special->WorldPos.X          >> 8,
				p_special->WorldPos.Y + 0x4000 >> 8,
				p_special->WorldPos.Z          >> 8))
		{
			return p_special;
		}
	}

	return NULL;
}

void PCOM_set_person_ai_getitem(Thing *p_person, Thing *p_special, SLONG move_speed, SLONG excar_state, SLONG excar_arg)
{
	p_person->Genus.Person->pcom_ai_state          = PCOM_AI_STATE_GETITEM;
	p_person->Genus.Person->pcom_ai_substate       = PCOM_AI_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_ai_arg            = THING_NUMBER(p_special);
	p_person->Genus.Person->pcom_ai_excar_state    = excar_state;
	p_person->Genus.Person->pcom_ai_excar_substate = NULL;
	p_person->Genus.Person->pcom_ai_excar_arg      = excar_arg;

	PCOM_set_person_move_mav_to_xz(
		p_person,
		p_special->WorldPos.X >> 8,
		p_special->WorldPos.Z >> 8,
		move_speed);
}


void PCOM_process_getitem(Thing *p_person)
{
	Thing *p_special = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_ANIMATION:
			break;

		case PCOM_MOVE_STATE_GOTO_XZ:

			//
			// Near enough to the special yet?
			//

			if ((PTIME(p_person) & 0x3)==0)
			{
				if (PCOM_finished_nav(p_person))
				{
					//
					// Bend down to pick up the special.
					//

					if (p_special->Flags & FLAGS_ON_MAPWHO)
					{
						PCOM_set_person_move_pickup_special(p_person, p_special);
					}
					else
					{
						//
						// Someone must have got the special!
						//

						PCOM_set_person_move_pause(p_person);
					}
				}
			}

			break;

		case PCOM_MOVE_STATE_PAUSE:
			
			if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(10))
			{
				//
				// Fall through!
				//
			}
			else
			{
				break;
			}

		case PCOM_MOVE_STATE_STILL:

			//
			// Go back to doing what 
			//

			switch(p_person->Genus.Person->pcom_ai_excar_state)
			{
				case PCOM_EXCAR_NORMAL:
					PCOM_set_person_ai_normal(p_person);
					break;

				case PCOM_EXCAR_NAVTOKILL:
					PCOM_set_person_ai_navtokill(p_person, TO_THING(p_person->Genus.Person->pcom_ai_excar_arg));
					break;

				default:
					ASSERT(0);
					break;
			}

			break;

		default:

			//
			// Emergency! I don't know what I'm doing!
			//

			ASSERT(0);

			PCOM_set_person_ai_normal(p_person);

			break;
	}
}


void PCOM_process_summon(Thing *p_person)
{
	SLONG i;

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_SUMMON_START:

			if (p_person->SubState == SUB_STATE_FLOAT_BOB)
			{
				//
				// Make everyone else start floating!
				//

				for (i = 0; i < PCOM_SUMMON_NUM_BODIES; i++)
				{
					if (PCOM_summon[i])
					{
						Thing *p_summon = TO_THING(PCOM_summon[i]);

						set_person_float_up(p_summon);

						SPARK_Pinfo p1;
						SPARK_Pinfo p2;
						
						static UBYTE limb[4] =
						{
							SUB_OBJECT_LEFT_HAND,
							SUB_OBJECT_RIGHT_HAND,
							SUB_OBJECT_LEFT_FOOT,
							SUB_OBJECT_RIGHT_FOOT,
						};

						p1.type   = SPARK_TYPE_LIMB;
						p1.flag   = 0;
						p1.person = THING_NUMBER(p_person);
						p1.limb   = limb[i];

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

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_SUMMON_FLOAT;
				p_person->Genus.Person->pcom_ai_counter  = 0;
			}

			break;

		case PCOM_AI_SUBSTATE_SUMMON_FLOAT:

			if (p_person->Genus.Person->pcom_ai_counter > PCOM_get_duration(50))
			{
				for (i = 0; i < PCOM_SUMMON_NUM_BODIES; i++)
				{
					if (PCOM_summon[i])
					{
						Thing *p_summon = TO_THING(PCOM_summon[i]);

						SPARK_Pinfo p1;
						SPARK_Pinfo p2;
						
						static UBYTE limb[4] =
						{
							SUB_OBJECT_LEFT_HAND,
							SUB_OBJECT_RIGHT_HAND,
							SUB_OBJECT_LEFT_FOOT,
							SUB_OBJECT_RIGHT_FOOT,
						};

						p1.type   = SPARK_TYPE_LIMB;
						p1.flag   = 0;
						p1.person = THING_NUMBER(p_person);
						p1.limb   = limb[i];

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

				p_person->Genus.Person->pcom_ai_counter = 0;
			}

			p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			//
			// We use the move counter for how long the player has been in the same
			// place near us for!
			//

			{
				Thing *darci = NET_PERSON(0);

				SLONG dx;
				SLONG dz;

				dx = abs(darci->WorldPos.X - p_person->WorldPos.X);
				dz = abs(darci->WorldPos.Z - p_person->WorldPos.Z);

				if (QDIST2(dx,dz) < 0x60000)
				{
					if ((darci->WorldPos.X >> 16) != (p_person->Genus.Person->pcom_ai_arg >> 8))
					{
						p_person->Genus.Person->pcom_move_counter >>= 1;
					}

					if ((darci->WorldPos.Z >> 16) != (p_person->Genus.Person->pcom_ai_arg & 0xff))
					{
						p_person->Genus.Person->pcom_move_counter >>= 1;
					}

					p_person->Genus.Person->pcom_ai_arg  =  darci->WorldPos.Z >> 16;
					p_person->Genus.Person->pcom_ai_arg |= (darci->WorldPos.X >> 16) & 0xff;

					p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
				}
				else
				{
					p_person->Genus.Person->pcom_move_counter = 0;
				}

				if (p_person->Genus.Person->pcom_move_counter >= PCOM_get_duration(20))
				{
					//
					// Electrocute Darci!
					//

					if (darci->State == STATE_IDLE   ||
						darci->State == STATE_MOVEING ||
						darci->State == STATE_GUN)
					{
						//
						// Electrocute her!
						//

						set_person_recoil(darci, ANIM_HIT_FRONT_MID, 0);

						darci->Genus.Person->Health -= 25;

						SPARK_Pinfo p1;
						SPARK_Pinfo p2;
						
						p1.type   = SPARK_TYPE_LIMB;
						p1.flag   = 0;
						p1.person = THING_NUMBER(p_person);
						p1.limb   = SUB_OBJECT_PELVIS;

						p2.type   = SPARK_TYPE_LIMB;
						p2.flag   = 0;
						p2.person = THING_NUMBER(darci);
						p2.limb   = SUB_OBJECT_PELVIS;

						SPARK_create(
							&p1,
							&p2,
							50);

						p_person->Genus.Person->pcom_move_counter = 0;
					}
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}



THING_INDEX PCOM_create_person(
				SLONG  type,
				SLONG  colour,
				SLONG  group,
				SLONG  ai,
				SLONG  ai_other,
				SLONG  ai_skill,
				SLONG  move,
				SLONG  move_follow,
				SLONG  bent,
				SLONG  pcom_has,
				SLONG  drop,
				SLONG  pcom_zone,
				SLONG  world_x,
				SLONG  world_y,
				SLONG  world_z,
				SLONG  yaw,
				SLONG  random,
				ULONG	flag1,
				ULONG	flag2)
{
	if (pcom_has & (PCOM_HAS_SHOTGUN | PCOM_HAS_KNIFE | PCOM_HAS_BASEBALLBAT))
	{
//		type = PERSON_DARCI; //EWAY_SUBTYPE_PLAYER_DARCI;//PERSON_DARCI;
	}

	if (pcom_has & PCOM_HAS_GUN)
	{
		if (drop == SPECIAL_GUN)
		{
			drop = 0;
		}
	}
	if (ai == PCOM_AI_ASSASIN)
	{
		TRACE("hello");
	}

	if (ai == PCOM_AI_ASSASIN && move == PCOM_MOVE_WANDER)
	{
		//
		// Make wandering assasins, follow who they are assasinating.
		//

		move        = PCOM_MOVE_FOLLOW;
		move_follow = ai_other;
	}


//	ASSERT(type!=PERSON_CIV);

	THING_INDEX p_index = create_person(
							type,
							random,
							world_x,
							world_y,
							world_z);

	if (p_index == NULL)
	{
#ifndef PSX
#ifndef NDEBUG
		PANEL_new_text(NULL, 10000, "Couldn't create person, PersonType %d ai %s", type, PCOM_ai_name[ai]);
#endif
#endif
	}
	else
	{
		Thing *p_person = TO_THING(p_index);

		if(type==PERSON_COP)
		{
			if(ai==PCOM_AI_DRIVER)
				ai=PCOM_AI_COP_DRIVER;
		}

		p_person->Draw.Tweened->Angle   = yaw;
		p_person->Genus.Person->HomeYaw = yaw >> 3;

		p_person->Genus.Person->pcom_colour = colour;
		p_person->Genus.Person->pcom_group  = group;
		p_person->Genus.Person->pcom_ai     = ai;
		p_person->Genus.Person->pcom_move   = move;
		p_person->Genus.Person->pcom_bent   = bent;
		p_person->Genus.Person->drop        = drop;
		p_person->Genus.Person->pcom_zone   = pcom_zone & 0xf;

		p_person->Genus.Person->pcom_ai_state     = PCOM_AI_STATE_NORMAL;
		p_person->Genus.Person->pcom_ai_substate  = PCOM_AI_SUBSTATE_NONE;
		p_person->Genus.Person->pcom_ai_arg       = NULL;
		p_person->Genus.Person->pcom_ai_other     = ai_other;

		p_person->Genus.Person->pcom_move_state   = PCOM_MOVE_STATE_STILL;
		p_person->Genus.Person->pcom_move_counter = 0;
		p_person->Genus.Person->pcom_move_arg     = 0;
		p_person->Genus.Person->pcom_move_follow  = move_follow;

		p_person->Genus.Person->FightRating       = 0;
		p_person->Genus.Person->Flags|=flag1;
		p_person->Genus.Person->Flags2|=flag2;

		//
		// Change guard in zone to violent youth in zone without telling Simon.
		//

		if (p_person->Genus.Person->pcom_zone &&
			p_person->Genus.Person->pcom_ai == PCOM_AI_GUARD)
		{
			p_person->Genus.Person->pcom_ai = PCOM_AI_GANG;
		}

		SET_SKILL(p_person,ai_skill);

		if (pcom_has & (PCOM_HAS_SHOTGUN | PCOM_HAS_GUN))
		{
//			p_person->Genus.Person->pcom_bent |=PCOM_BENT_KILL_ON_SIGHT;
		}

		if (pcom_has & PCOM_HAS_GUN)     {p_person->Flags |= FLAGS_HAS_GUN;}

#if NO_MORE_HAPPY_BALOONS
#if !defined(PSX) && !defined(TARGET_DC)
		if (pcom_has & PCOM_HAS_BALLOON) {p_person->Genus.Person->Balloon = BALLOON_create(p_index, BALLOON_TYPE_YELLOW);}
#endif
#endif
		if (pcom_has & PCOM_HAS_SHOTGUN)
		{

			//
			// Create a shotgun and give it to the person.
			//

			Thing *p_special = alloc_special(
									SPECIAL_SHOTGUN,
									SPECIAL_SUBSTATE_NONE,
									0, 0, 0, NULL);
			if (p_special)
			{
				if (should_person_get_item(p_person, p_special))
				{
					person_get_item     (p_person, p_special);
					set_person_draw_item(p_person, SPECIAL_SHOTGUN);
				}
			}
		}

		if (pcom_has & PCOM_HAS_AK47)
		{
			//
			// Create a shotgun and give it to the person.
			//

			Thing *p_special = alloc_special(
									SPECIAL_AK47,
									SPECIAL_SUBSTATE_NONE,
									0, 0, 0, NULL);

			if (p_special)
			{
				if (should_person_get_item(p_person, p_special))
				{
					person_get_item     (p_person, p_special);
					set_person_draw_item(p_person, SPECIAL_AK47);
				}
			}
		}

		if (pcom_has & PCOM_HAS_KNIFE)
		{
			//
			// Create a shotgun and give it to the person.
			//

			Thing *p_special = alloc_special(
									SPECIAL_KNIFE,
									SPECIAL_SUBSTATE_NONE,
									0, 0, 0, NULL);

			if (p_special)
			{
				if (should_person_get_item(p_person, p_special))
				{
					person_get_item     (p_person, p_special);
					set_person_draw_item(p_person, SPECIAL_KNIFE);
				}
			}
		}

		if (pcom_has & PCOM_HAS_BASEBALLBAT)
		{
			//
			// Create a baseball bat and give it to the person.
			//

			Thing *p_special = alloc_special(
									SPECIAL_BASEBALLBAT,
									SPECIAL_SUBSTATE_NONE,
									0, 0, 0, NULL);

			if (p_special)
			{
				if (should_person_get_item(p_person, p_special))
				{
					person_get_item     (p_person, p_special);
					set_person_draw_item(p_person, SPECIAL_BASEBALLBAT);
				}
			}
		}

		if (pcom_has & PCOM_HAS_GRENADE)
		{
			//
			// Create a grenade and give it to the person.
			//

			Thing *p_special = alloc_special(
									SPECIAL_GRENADE,
									SPECIAL_SUBSTATE_NONE,
									0, 0, 0, NULL);

			if (p_special)
			{
				if (should_person_get_item(p_person, p_special))
				{
					person_get_item     (p_person, p_special);
					set_person_draw_item(p_person, SPECIAL_GRENADE);
				}
			}
		}

		if (bent & PCOM_BENT_GANG)
		{
			PCOM_add_gang_member(p_index, colour);
		}
	}

	return p_index;
}

THING_INDEX PCOM_create_player(
				SLONG  type,
				SLONG  pcom_has,
				SLONG  world_x,
				SLONG  world_y,
				SLONG  world_z,
				SLONG  id,
				SLONG  yaw)
{
	Thing *p_person = create_player(
							type,
							world_x,
							world_y,
							world_z,
							id);

#ifndef PSX
	extern SLONG playing_level(const CBYTE *name);

	if (playing_level("skymiss2.ucm"))
#else
	if (wad_level==25)
#endif
	{
		pcom_has |= PCOM_HAS_SHOTGUN;
	}

	if (p_person)
	{
		p_person->Draw.Tweened->Angle = yaw;

		p_person->Genus.Person->pcom_ai   = PCOM_AI_NONE;
		p_person->Genus.Person->pcom_move = PCOM_MOVE_STILL;
		p_person->Genus.Person->pcom_bent = 0;

		p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_PLAYER;
		p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NONE;
		p_person->Genus.Person->pcom_move_state  = PCOM_MOVE_STATE_PLAYER;

		if (pcom_has & PCOM_HAS_GUN)     {p_person->Flags |= FLAGS_HAS_GUN;}

		if (pcom_has & PCOM_HAS_SHOTGUN)
		{

			//
			// Create a shotgun and give it to the person.
			//

			Thing *p_special = alloc_special(
									SPECIAL_SHOTGUN,
									SPECIAL_SUBSTATE_NONE,
									0, 0, 0, NULL);
			if (p_special)
			{
				if (should_person_get_item(p_person, p_special))
				{
					person_get_item     (p_person, p_special);
					set_person_draw_item(p_person, SPECIAL_SHOTGUN);
				}
			}
		}

#if !defined(PSX) && !defined(TARGET_DC)
		if (pcom_has & PCOM_HAS_BALLOON) {p_person->Genus.Person->Balloon = BALLOON_create(THING_NUMBER(p_person), BALLOON_TYPE_YELLOW);}
#endif

		return THING_NUMBER(p_person);
	}

	return NULL;
}

void PCOM_change_person_attributes(
		Thing *p_person,
		SLONG  colour,
		SLONG  group,
		SLONG  ai,
		SLONG  ai_other,
		SLONG  move,
		SLONG  move_follow,
		SLONG  bent,
		SLONG  yaw)
{
	if (is_person_dead(p_person))
	{
		return;
	}

	p_person->Genus.Person->pcom_colour = colour;
	p_person->Genus.Person->pcom_group  = group;
	p_person->Genus.Person->pcom_ai     = ai;
	p_person->Genus.Person->pcom_move   = move;
	p_person->Genus.Person->pcom_bent   = bent;

	p_person->Genus.Person->pcom_ai_state     = PCOM_AI_STATE_NORMAL;
	p_person->Genus.Person->pcom_ai_substate  = PCOM_AI_SUBSTATE_NONE;
	p_person->Genus.Person->pcom_ai_arg       = NULL;
	p_person->Genus.Person->pcom_ai_other     = ai_other;

	p_person->Genus.Person->pcom_move_state   = PCOM_MOVE_STATE_STILL;
	p_person->Genus.Person->pcom_move_counter = 0;
	p_person->Genus.Person->pcom_move_arg     = 0;
	p_person->Genus.Person->pcom_move_follow  = move_follow;

	if(p_person->Genus.Person->pcom_move == PCOM_MOVE_HANDS_UP)
	{
void	drop_current_gun(Thing *p_person,SLONG change_anim);

		drop_current_gun(p_person,0);
	}

	if (p_person->Genus.Person->pcom_move == PCOM_MOVE_STILL    ||
		p_person->Genus.Person->pcom_move == PCOM_MOVE_DANCE    ||
		p_person->Genus.Person->pcom_move == PCOM_MOVE_HANDS_UP ||
		p_person->Genus.Person->pcom_move == PCOM_MOVE_TIED_UP)
	{
		//
		// Change this person's home to be where he is now.
		//

		p_person->Genus.Person->HomeX = p_person->WorldPos.X >> 8;
		p_person->Genus.Person->HomeZ = p_person->WorldPos.Z >> 8;
		/*
		p_person->Genus.Person->HomeYaw = yaw >> 3;
		p_person->Draw.Tweened->Angle   = yaw;
		*/
	}

	PCOM_set_person_ai_normal(p_person);
}

//
// Returns the zone flags for the place a person is.
//

UBYTE PCOM_get_zone_for_position(Thing *p_person)
{
	UBYTE zone;

	SLONG dest_x;
	SLONG dest_z;

	PAP_Hi *ph;
	
	//
	// We don't just look at where this person is- we also consider where he is going...
	//

	PCOM_get_person_navsquare(
		p_person,
	   &dest_x,
	   &dest_z);

	ph = &PAP_2HI(dest_x,dest_z);

	ASSERT(PAP_FLAG_ZONE1 == (1 << 10));

	zone = ph->Flags >> 10;

	return zone;
}

UBYTE PCOM_get_zone_for_position(SLONG x, SLONG z)
{
	UBYTE zone;

	PAP_Hi *ph;

	ph = &PAP_2HI(x >> 8, z >> 8);

	ASSERT(PAP_FLAG_ZONE1 == (1 << 10));

	zone = ph->Flags >> 10;

	return zone;
}


//
// Returns a pointer to a player person you can see who is inside your zone.
//

Thing *PCOM_can_i_see_person_to_attack(Thing *p_person)
{
	Thing *p_target = NET_PERSON(0);

	if (p_target->State == STATE_DEAD  ||
		p_target->State == STATE_DYING ||
		stealth_debug) 
	{
		return NULL;
	}

	if (p_person->Genus.Person->pcom_zone)
	{
		//
		// The target must be on the same zone as us.
		//

		if (PCOM_get_zone_for_position(p_target) & p_person->Genus.Person->pcom_zone)
		{
			//
			// The person is in our zone!
			//
		}
		else
		{
			//
			// Ignore people who aren't in our zone.
			//

			return NULL;
		}
	}

	if (can_a_see_b(p_person, p_target))
	{
		return p_target;
	}

	return NULL;
}


//
// Returns a pointer to a person you can bully or NULL if you can't find
// anyone suitable.
//

Thing *PCOM_can_i_see_person_to_bully(Thing *p_person)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG score;

	SLONG  best_score = INFINITY;
	Thing *best_thing = NULL;

	Thing *p_found;

	//
	// Find all the people near us.
	//

	PCOM_found_num = THING_find_sphere(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						0x600,
						PCOM_found,
						PCOM_MAX_FIND,
						1 << CLASS_PERSON);

	//
	// Who is the best person to bully?
	//

	for (i = 0; i < PCOM_found_num; i++)
	{
		p_found = TO_THING(PCOM_found[i]);

		if (p_found == p_person)
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
			// Ignore dead people.
			//

			continue;
		}

		if (p_person->Genus.Person->pcom_bent & PCOM_BENT_ONLY_KILL_PLAYER)
		{
			if (!p_found->Genus.Person->PlayerID)
			{
				//
				// Only consider players.
				//

				continue;
			}
		}
		else
		{
			//
			// Only consider wandering civs and players.
			//

			if (p_found->Genus.Person->pcom_ai   == PCOM_AI_CIV &&
				p_found->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
			{
				//
				// This is a wandering civ.
				//
			}
			else
			if (p_found->Genus.Person->PlayerID)
			{
				//
				// This is the player.
				//
			}
			else
			{
				//
				// This could be someone important the mission.
				//

				continue;
			}
		}

		if (can_a_see_b(p_person, p_found))
		{
			//
			// Score this person. Lower scores are better.
			//

			dx = abs(p_person->WorldPos.X - p_found->WorldPos.X);
			dy = abs(p_person->WorldPos.Y - p_found->WorldPos.Y);
			dz = abs(p_person->WorldPos.Z - p_found->WorldPos.Z);

			score = dx + dy + dy + dz;	// Difference in y is more important...

			if (p_found->Genus.Person->PlayerID)
			{
				//
				// More likely to bully players.
				//

				score >>= 1;
			}

			if (best_score > score)
			{
				best_score = score;
				best_thing = p_found;
			}
		}
	}

	return best_thing;
}

#ifndef PSX
#define	MAX_ARREST_ME	100
#else
#define	MAX_ARREST_ME	5
#endif
Thing	*arrest_me[MAX_ARREST_ME];
UWORD	next_arrest=0;

void	init_arrest(void)
{
	next_arrest=0;
}


void	do_arrests(void)
{
	SLONG	c0;
	for(c0=0;c0<next_arrest;c0++)
	{
		PCOM_call_cop_to_arrest_me(arrest_me[c0],0);
	}

	next_arrest=0;
}

SLONG PCOM_call_cop_to_arrest_me(Thing *p_person,SLONG store_it)
{
	SLONG i;

	Thing *p_found;
	SLONG	found_cop=0;

#ifdef DEBUG
	if(p_person->Genus.Person->PersonType==PERSON_DARCI)
		ASSERT(0);
#endif

	if(store_it)
	{
		if(next_arrest>=MAX_ARREST_ME)
		{
			ASSERT(0);
			return(0);
		}

		arrest_me[next_arrest]=p_person;
		next_arrest++;
		return(0);
	}

	//
	// Find all the people near us.
	//

	if(p_person->Genus.Person->PersonType==PERSON_TRAMP)
		return(0);

	PCOM_found_num = THING_find_sphere(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						0x800,
						PCOM_found,
						PCOM_MAX_FIND,
						(1 << CLASS_PERSON)|(1<<CLASS_VEHICLE));

	//
	// Who is the best person to arrest me?
	//

	for (i = 0; i < PCOM_found_num; i++)
	{
		p_found = TO_THING(PCOM_found[i]);


		if (p_found == p_person)
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
			// Ignore dead people.
			//

			continue;
		}

		if(p_found->Class==CLASS_VEHICLE)
		{
			if(p_found->Genus.Vehicle->Driver)
			{
				p_found=TO_THING(p_found->Genus.Vehicle->Driver);

			}
			else
			{
				continue;
			}
		}

		if (p_found->Genus.Person->pcom_ai == PCOM_AI_COP ||
			p_found->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER)
		{
			//
			// found a cop type person
			//
//			 AENG_world_line_infinite(p_found->WorldPos.X>>8,p_found->WorldPos.Y>>8,p_found->WorldPos.Z>>8,7,0xfffff,p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,2,0xff0000,1);

			if(PCOM_person_doing_nothing_important(p_found))
			switch(p_found->Genus.Person->pcom_ai_state)
			{
				case PCOM_AI_STATE_INVESTIGATING:
				case PCOM_AI_STATE_NORMAL:
				case PCOM_AI_STATE_WARM_HANDS:
				case PCOM_AI_STATE_HOMESICK:
					if (can_a_see_b(p_person, p_found))
					{

						//
						// Make the target wanted by the police.
						//

						p_person->Genus.Person->Flags |= FLAG_PERSON_FELON;
						found_cop=1;

						if (p_found->Genus.Person->pcom_ai == PCOM_AI_COP ||p_found->Genus.Person->InCar==0)
						{
							PCOM_set_person_ai_arrest(p_found, p_person);
						}
						else
						{

							//
							// Exit the car and once you've got out, arrest the person.
							//
							PCOM_set_person_ai_leavecar(
								p_found,
								PCOM_EXCAR_ARREST_PERSON,
								0,
								THING_NUMBER(p_person));
						}
					}
					break;
			}
		}
	}

	return found_cop;

}

/*
Thing *PCOM_can_i_see_person_to_arrest(Thing *p_person)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG score;
	SLONG ignore_los = FALSE;

	SLONG  best_score = INFINITY;
	Thing *best_thing = NULL;

	Thing *p_found;

	//
	// Find all the people near us.
	//

	PCOM_found_num = THING_find_sphere(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						0x600,
						PCOM_found,
						PCOM_MAX_FIND,
						1 << CLASS_PERSON);

	//
	// Who is the best person to arrest?
	//

	for (i = 0; i < PCOM_found_num; i++)
	{
		p_found = TO_THING(PCOM_found[i]);

		ignore_los = FALSE;	// For people the player is fighting...

	  try_this_person_too:;

		if (p_found == p_person)
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
			// Ignore dead people.
			//

			continue;
		}

		if (p_found->Genus.Person->pcom_ai == PCOM_AI_COP ||
			p_found->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER)
		{
			//
			// Ignore other cops.
			//

			continue;
		}

		if ((p_found->Genus.Person->Flags          & FLAG_PERSON_FELON)       ||
			(p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING)   ||
			(p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL) ||
			(p_found->Genus.Person->PlayerID && p_found->Genus.Person->Mode == PERSON_MODE_FIGHT))
		{
			//
			// This person is worth arresting.
			//

			if (ignore_los || can_a_see_b(p_person, p_found))
			{
				//
				// Score this person. Lower scores are better.
				//

				dx = abs(p_person->WorldPos.X - p_found->WorldPos.X);
				dy = abs(p_person->WorldPos.Y - p_found->WorldPos.Y);
				dz = abs(p_person->WorldPos.Z - p_found->WorldPos.Z);

				score = dx + dy + dy + dz;	// Difference in y is more important...

				if (p_found->Genus.Person->PlayerID)
				{
					//
					// Less likely to arrest the player.
					//

					score <<= 1;
				}

				if (p_found->Genus.Person->pcom_ai == PCOM_AI_CIV && p_found->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
				{
					//
					// Very unlikely to arrest wandering civs!
					// 

					score <<= 2;
				}

				if (p_found->Genus.Person->Flags2 & FLAG2_PERSON_GUILTY)
				{
					//
					// Very likely to arrest guilty people.
					//

					score >>= 2;
				}

				if (best_score > score)
				{
					best_score = score;
					best_thing = p_found;
				}

				//
				// What if the cop can only see the player fighting but he can't
				// see who the player is fighting?
				//

				if (p_found->Genus.Person->PlayerID)
				{
					if (p_found->Genus.Person->Target)
					{
						Thing *p_target;

						//
						// Try this person too...
						//

						p_target = TO_THING(p_found->Genus.Person->Target);

						ASSERT(p_target->Class == CLASS_PERSON);

						p_found    = p_target;
						ignore_los = TRUE;

						goto try_this_person_too;
					}
				}
			}
		}
	}

	return best_thing;
}
*/

//
// Returns a pointer to a player person you can see.
//

Thing *PCOM_can_i_see_person_to_taunt(Thing *p_person)
{
	SLONG i;

	for (i = 0; i < NO_PLAYERS; i++)
	{
		Thing *p_target = NET_PERSON(0);

		if (p_target->State == STATE_DEAD  ||
			p_target->State == STATE_DYING ||
			stealth_debug) 
		{
			continue;
		}

		if (can_a_see_b(p_person, p_target))
		{
			return p_target;
		}
	}

	return NULL;
}


//
// Returns the next waypoint for a person to patrol.
//

SLONG PCOM_get_next_patrol_waypoint(Thing *p_person)
{
	SLONG waypoint;

	if (p_person->Genus.Person->pcom_move == PCOM_MOVE_PATROL)
	{
		waypoint = EWAY_find_waypoint(
						p_person->Genus.Person->pcom_move_arg + 1,
						EWAY_DONT_CARE,
						p_person->Genus.Person->pcom_colour,
						p_person->Genus.Person->pcom_group,
						TRUE);
	}
	else
	{
		waypoint = EWAY_find_waypoint_rand(
						p_person->Genus.Person->pcom_move_arg,
						p_person->Genus.Person->pcom_colour,
						p_person->Genus.Person->pcom_group,
						TRUE);
	}
	
	return waypoint;
}


//
// Processes a person who doesn't want to move who is in a car or on a bike.
//

void PCOM_process_driving_still(Thing *p_person)
{
	ASSERT(p_person->Genus.Person->Flags & (FLAG_PERSON_DRIVING | FLAG_PERSON_BIKING));

	Thing *p_vehicle;

	p_vehicle = TO_THING(p_person->Genus.Person->InCar);

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:

			if (p_vehicle->Velocity != 0)
			{
				//
				// We want to stop the car.
				//

				PCOM_set_person_move_park_car_on_road(p_person);
			}

			break;

		case PCOM_MOVE_STATE_PARK_CAR:
		case PCOM_MOVE_STATE_PARK_BIKE:
			
			if ((p_vehicle->Class == CLASS_VEHICLE && p_vehicle->Velocity == 0)
			
				#ifdef BIKE

				||
				(p_vehicle->Class == CLASS_BIKE    && BIKE_get_speed(p_vehicle) == 0)

				#endif
				
				)
			{
				//
				// We have come to a halt.
				//

				p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_STILL;
				p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
				p_person->Genus.Person->pcom_move_arg      = 0;
				p_person->Genus.Person->pcom_move_counter  = 0;
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Processes a person driving between waypoints who is either in a car
// or on a bike.
//

void PCOM_process_driving_patrol(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG waypoint;

	SLONG dest_x;
	SLONG dest_z;

	Thing *p_vehicle;

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:

			//
			// Must have just got into the car (or onto the bike)
			//

			waypoint = EWAY_find_nearest_waypoint(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							p_person->Genus.Person->pcom_colour,
							p_person->Genus.Person->pcom_group);

			if (waypoint != EWAY_NO_MATCH)
			{
				if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
				{
					PCOM_set_person_move_driveto(p_person, waypoint);
				}
				#ifdef BIKE
				else
				{
					PCOM_set_person_move_biketo(p_person, waypoint);
				}
				#else
				else
				{
					ASSERT(0);
				}
				#endif
			}

			break;

		case PCOM_MOVE_STATE_DRIVETO:
		case PCOM_MOVE_STATE_BIKETO:

			ASSERT(p_person->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_BIKING));

			//
			// Have we arrived at our next waypoint yet?
			//

			PCOM_get_person_dest(
				p_person,
			   &dest_x,
			   &dest_z);
			
			p_vehicle = TO_THING(p_person->Genus.Person->InCar);

			dx = dest_x - (p_vehicle->WorldPos.X >> 8);
			dz = dest_z - (p_vehicle->WorldPos.Z >> 8);

			dist = QDIST2(abs(dx),abs(dz));

			if (dist < 0x300)
			{
				if (EWAY_get_delay(p_person->Genus.Person->pcom_move_arg, 0) == 10 * 1000)
				{
					//
					// This waypoint has the maximum delay. That means that the driver
					// should stop wanting to move- i.e. he should park here.
					//

					if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
					{
						PCOM_set_person_move_park_car(p_person, p_person->Genus.Person->pcom_move_arg);
					}
					#ifdef BIKE
					else
					{
						PCOM_set_person_move_park_bike(p_person, p_person->Genus.Person->pcom_move_arg);
					}
					#else
					else
					{
						ASSERT(0);
					}
					#endif

					//
					// Dont move anymore.
					//

					p_person->Genus.Person->pcom_move = PCOM_MOVE_STILL;
				}
				else
				{
					//
					// Move onto the next waypoint.
					//

					if (p_person->Genus.Person->pcom_move == PCOM_MOVE_PATROL)
					{
						waypoint = EWAY_find_waypoint(
										p_person->Genus.Person->pcom_move_arg + 1,
										EWAY_DONT_CARE,
										p_person->Genus.Person->pcom_colour,
										p_person->Genus.Person->pcom_group,
										TRUE);
					}
					else
					{
						waypoint = EWAY_find_waypoint_rand(
										p_person->Genus.Person->pcom_move_arg,
										p_person->Genus.Person->pcom_colour,
										p_person->Genus.Person->pcom_group,
										TRUE);
					}

					if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
					{
						PCOM_set_person_move_driveto(p_person, waypoint);
					}
					#ifdef BIKE
					else
					{
						PCOM_set_person_move_biketo(p_person, waypoint);
					}
					#else
					else
					{
						ASSERT(0);
					}
					#endif
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Processes a person driving or biking aimlessly around the city.
//

void PCOM_process_driving_wander(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG dest_x;
	SLONG dest_z;

	SLONG n1;
	SLONG n2;

	SLONG wtn1;
	SLONG wtn2;

	Thing *p_vehicle;

	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
	{
		p_vehicle = TO_THING(p_person->Genus.Person->InCar);

		if (p_vehicle->Flags & FLAGS_IN_VIEW)
		{
			p_person->Genus.Person->sewerbits = 0;
		}
		else
		{
			p_person->Genus.Person->sewerbits++;
			if (p_person->Genus.Person->sewerbits > 50)
			{
				SLONG	dx,dz;
				Thing	*p_darci=NET_PERSON(0);
				dx=abs((p_person->WorldPos.X-p_darci->WorldPos.X)>>8);
				dz=abs((p_person->WorldPos.Z-p_darci->WorldPos.Z)>>8);
				if(QDIST2(dx,dz) >= (DRAW_DIST<<8))
				{
					SLONG	x,z,yaw;

extern SLONG WAND_find_good_start_point_for_car(SLONG* posx, SLONG* posz, SLONG* yaw, SLONG anywhere);

					if (WAND_find_good_start_point_for_car(&x, &z, &yaw, 0))
					{
						GameCoord	newpos;

						newpos.X = x << 8;
						newpos.Y = 0;	// calculated properly in reinit_vehicle()
						newpos.Z = z << 8;
						move_thing_on_map(p_vehicle, &newpos);
						p_person->Genus.Person->sewerbits = Random() & 15;
						p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_STILL;	// re-init the wander

						p_person->WorldPos = newpos;

//						p_vehicle->Draw.Mesh->Angle = yaw ^ 1024;

						Vehicle*	veh = p_vehicle->Genus.Vehicle;
						veh->Angle = yaw ^ 1024;

						#if PSX
						{
							p_vehicle->Genus.Vehicle->Type=vehicle_random[(Random()&15)];
						}
						#endif

						reinit_vehicle(p_vehicle);

//						return;
					}
				}
			}
		}
	}

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_DRIVE_DOWN:
		case PCOM_MOVE_STATE_BIKE_DOWN:

			p_vehicle = TO_THING(p_person->Genus.Person->InCar);

			if (ROAD_is_end_of_the_line(p_person->Genus.Person->pcom_move_arg & 0xff))
			{
				//
				// Have we gone off the map yet?
				//

				if (!WITHIN(p_vehicle->WorldPos.X, 0, PAP_SIZE_HI << 16) ||
					!WITHIN(p_vehicle->WorldPos.Z, 0, PAP_SIZE_HI << 16))
				{
					SLONG world_x = p_vehicle->WorldPos.X >> 8;
					SLONG world_z = p_vehicle->WorldPos.Z >> 8;
					SLONG nrn1;
					SLONG nrn2;
					SLONG nyaw;

					//
					// Find another node on the road.
					//

					ROAD_find_me_somewhere_to_appear(
						&world_x,
						&world_z,
						&nrn1,
						&nrn2,
						&nyaw);

					//
					// Re-init the car a bit... otherwise problems can occur due to the
					// sudden teleportation...
					//
					p_vehicle->Genus.Vehicle->oldX[0]=
						p_vehicle->Genus.Vehicle->oldX[1]=
						p_vehicle->Genus.Vehicle->oldX[2]=
						p_vehicle->Genus.Vehicle->oldX[3]=0;
					p_vehicle->Genus.Vehicle->oldZ[0]=
						p_vehicle->Genus.Vehicle->oldZ[1]=
						p_vehicle->Genus.Vehicle->oldZ[2]=
						p_vehicle->Genus.Vehicle->oldZ[3]=0;
					p_vehicle->Genus.Vehicle->Smokin=0;


					//
					// Move the vehicle to the new road.
					//

					GameCoord newpos;

					newpos.X = world_x                                    << 8;
					newpos.Z = world_z                                    << 8;
					newpos.Y = PAP_calc_height_at(world_x,world_z) + 0x40 << 8;

					move_thing_on_map(p_vehicle, &newpos);

					//
					// Pointing in the right direction.
					//

					if (p_vehicle->Class == CLASS_VEHICLE)
					{
						p_vehicle->Genus.Vehicle->Angle = nyaw;
					}
					#ifdef BIKE
					else
					{
						ASSERT(p_vehicle->Class == CLASS_BIKE);

						p_vehicle->Genus.Bike->yaw = nyaw;
					}
					#else
					else
					{
						ASSERT(0);
					}
					#endif

					//
					// Remember the new road we are going down.
					//

					p_person->Genus.Person->pcom_move_arg  = nrn1 << 8;
					p_person->Genus.Person->pcom_move_arg |= nrn2;
				}
			}
			else
			{              
				//
				// Have we reached the end of the road?
				//
				
				PCOM_get_person_dest(
					p_person,
				   &dest_x,
				   &dest_z);

				dx = dest_x - (p_vehicle->WorldPos.X >> 8);
				dz = dest_z - (p_vehicle->WorldPos.Z >> 8);

				dist = QDIST2(abs(dx),abs(dz));

#define	LEFT_TURN_DIST		0x380	// distance from junction before left turn
#define RIGHT_TURN_DIST		0x280	// distance from junction before right turn

				if (dist < LEFT_TURN_DIST)
				{
					// find next road to drive down
					if (!p_person->Genus.Person->InsideRoom)
					{
						ROAD_whereto_now(
							p_person->Genus.Person->pcom_move_arg >> 8,
							p_person->Genus.Person->pcom_move_arg & 0xff,
						   &wtn1,
						   &wtn2);

						p_person->Genus.Person->InsideRoom = wtn2;
					}

					// check turn
					if (ROAD_bend(p_person->Genus.Person->pcom_move_arg >> 8, p_person->Genus.Person->pcom_move_arg & 0xff, p_person->Genus.Person->InsideRoom) < 0)
					{
						// left turn - do it late
						dist += LEFT_TURN_DIST - RIGHT_TURN_DIST;
					}

					// do turn
					if (dist < LEFT_TURN_DIST)
					{
						if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
						{
							PCOM_set_person_move_drive_down(p_person, p_person->Genus.Person->pcom_move_arg & 0xff, p_person->Genus.Person->InsideRoom);
						}
						#ifdef BIKE
						else
						{
							PCOM_set_person_move_bike_down(p_person, p_person->Genus.Person->pcom_move_arg & 0xff, p_person->Genus.Person->InsideRoom);
						}
						#else
						else ASSERT(0);
						#endif
						p_person->Genus.Person->InsideRoom = 0;
					}
				}
				else
				{
					// ensure the next node is "none"
					p_person->Genus.Person->InsideRoom = 0;
				}
			}

			break;

		default:

			//
			// Find a road to start driving down.
			//

			ROAD_find(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Z >> 8,
			   &n1,
			   &n2);

			if (n1 && n2)
			{
				//
				// Which side of the road are we on?
				//

				dist = ROAD_signed_dist(
						n1,
						n2,
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Z >> 8);

				if (dist < 0)
				{
					//
					// We are on the wrong side of the road- go in the other direction.
					//

					SWAP(n1,n2);
				}

				if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
				{
					PCOM_set_person_move_drive_down(p_person, n1, n2);
				}
				#ifdef BIKE
				else
				{
					PCOM_set_person_move_bike_down(p_person, n1, n2);
				}
				#else
				else
				{
					ASSERT(0);
				}
				#endif
			}

			p_person->Genus.Person->InsideRoom = 0;	// InsideRoom = next node to drive to

			break;
	}
}

//
// Processes a person moving between patrol points.
//

void PCOM_process_patrol(Thing *p_person)
{
	SLONG waittime;
	SLONG waypoint;

	switch(p_person->Genus.Person->pcom_move_state)
	{
		default:
		case PCOM_MOVE_STATE_STILL:
			
			//
			// Find a waypoint to get to.
			//

			waypoint = EWAY_find_nearest_waypoint(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							p_person->Genus.Person->pcom_colour,
							p_person->Genus.Person->pcom_group);

			if (waypoint == EWAY_NO_MATCH)
			{
				//
				// Stay still for a while
				//

				PCOM_set_person_move_pause(p_person);
			}
			else
			{
				SLONG way_x;
				SLONG way_y;
				SLONG way_z;

				//
				// Too near to this waypoint?
				//

				EWAY_get_position(
					waypoint,
				   &way_x,
				   &way_y,
				   &way_z);

				SLONG dx = abs(way_x - (p_person->WorldPos.X >> 8));
				SLONG dy = abs(way_y - (p_person->WorldPos.Y >> 8));
				SLONG dz = abs(way_z - (p_person->WorldPos.Z >> 8));

				SLONG dist = QDIST3(dx,dy,dz);

				if (dist < 0x100)
				{
					//
					// We have to put the current waypoint into the persons 'pcom_move_arg'
					// because that is where PCOM_get_next_patrol_waypoint() expects it
					// to be.
					//

					p_person->Genus.Person->pcom_move_arg = waypoint;

					//
					// We are really close to this waypoint, lets start going to
					// the next one.
					//

					waypoint = PCOM_get_next_patrol_waypoint(p_person);
				}

				//
				// Go towards this waypoint.
				//

				PCOM_set_person_move_mav_to_waypoint(
					p_person,
					waypoint,
					(p_person->Genus.Person->pcom_bent & PCOM_BENT_DILIGENT) ? PCOM_MOVE_SPEED_RUN : PCOM_MOVE_SPEED_WALK);
			}

			break;

		case PCOM_MOVE_STATE_GOTO_WAYPOINT:

			//
			// Got to our waypoint yet?
			//

			if (PCOM_finished_nav(p_person))
			{
				//
				// If this waypoint has a delay of 10 seconds, then make that mean that the person
				// should stay here forever- i.e. go to PCOM_MOVE_STILL.
				//

				waypoint = p_person->Genus.Person->pcom_move_arg;

				if (EWAY_get_delay(waypoint, 0) == 10 * 1000)
				{
					SLONG way_x;
					SLONG way_y;
					SLONG way_z;

					//
					// The position of the waypoint we are going to stay at.
					//

					EWAY_get_position(
						waypoint,
					   &way_x,
					   &way_y,
					   &way_z);

					p_person->Genus.Person->pcom_move = PCOM_MOVE_STILL;
					p_person->Genus.Person->HomeX     = way_x;
					p_person->Genus.Person->HomeZ     = way_z;
					p_person->Genus.Person->HomeYaw   = EWAY_get_angle(waypoint) >> 3;

					PCOM_set_person_move_still(p_person);
				}
				else
				{
					//
					// Wait at this waypoint before going onto
					//

					PCOM_set_person_move_pause(p_person);

					//
					// Turn to face the direction of the waypoint.
					//

					p_person->Draw.Tweened->Angle = EWAY_get_angle(p_person->Genus.Person->pcom_move_arg);
				}
			}

			break;

		case PCOM_MOVE_STATE_PAUSE:

			//
			// Finished waiting?
			//

			waittime = PCOM_get_duration(EWAY_get_delay(p_person->Genus.Person->pcom_move_arg, 0) / 100);

			if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
			{
				waittime += waittime;
			}

			if (p_person->Genus.Person->pcom_move_counter >= waittime)
			{
				//
				// Move onto next waypoint.
				//

				waypoint = PCOM_get_next_patrol_waypoint(p_person);

				if (waypoint == EWAY_NO_MATCH || waypoint == p_person->Genus.Person->pcom_move_arg)
				{
					PCOM_set_person_move_pause(p_person);
				}
				else
				{
					PCOM_set_person_move_mav_to_waypoint(
						p_person,
						waypoint,
						(p_person->Genus.Person->pcom_bent & PCOM_BENT_DILIGENT) ? PCOM_MOVE_SPEED_RUN : PCOM_MOVE_SPEED_WALK);
				}
			}
			else
			{
				//
				// Turn to face the direction of the waypoint.
				//

				p_person->Draw.Tweened->Angle = EWAY_get_angle(p_person->Genus.Person->pcom_move_arg);
			}

			break;

		//
		// Default case is at the top of the switch statement!
		//
	}
}

SLONG	should_person_regen(Thing *p_person)
{
	SLONG	dx,dz;
	Thing	*p_darci=NET_PERSON(0);
	dx=abs((p_person->WorldPos.X-p_darci->WorldPos.X)>>8);
	dz=abs((p_person->WorldPos.Z-p_darci->WorldPos.Z)>>8);
	if(QDIST2(dx,dz)<(DRAW_DIST<<8))
		return(0);

	if(p_person->Genus.Person->InsideRoom>50)
		return(1);
	else
		return(0);



}

extern	ULONG	timer_bored;

SLONG	PCOM_do_regen(Thing *p_person)
{
	SLONG wand_x;
	SLONG wand_z;

	SLONG	nx,nz;

	if(NET_PERSON(0)->Genus.Person->Ware || EWAY_stop_player_moving())
	{
		//
		// Player is in a warehouse so why bother
		//
		return(0);
	}

	if (p_person->Genus.Person->Target)
	{
		Thing *p_target = TO_THING(p_person->Genus.Person->Target);

		remove_from_gang_attack(p_person, p_target);
	}

extern	SLONG	WAND_find_good_start_point(SLONG *mapx,SLONG *mapz);
	if(WAND_find_good_start_point(&nx,&nz))
	{
		GameCoord new_position;

		new_position.X = nx<<8;
		new_position.Y = PAP_calc_height_at(nx,nz)<<8;
		new_position.Z = nz<<8;
		move_thing_on_map(p_person, &new_position);
		p_person->Genus.Person->Flags&=~(FLAG_PERSON_KO | FLAG_PERSON_HELPLESS|FLAG_PERSON_WAREHOUSE|FLAG_PERSON_ARRESTED|FLAG_PERSON_ARRESTED|FLAG_PERSON_SEARCHED);
		p_person->Genus.Person->Ware=0;
		p_person->OnFace = 0;

		if((signed)timer_bored>(BOREDOM_RATE*5000) && BOREDOM_RATE!=255)// && BOREDOM_RATE)
//		if(0) //mdsoft
		{
			Thing	*darci;
			darci=NET_PERSON(0);

			if(darci->Genus.Person->PersonType != PERSON_DARCI && darci->Genus.Person->PersonType != PERSON_ROPER)
			{
				//
				//
				// darci is thug
				p_person->Genus.Person->pcom_ai     = PCOM_AI_COP;
				p_person->Genus.Person->pcom_bent   = 0;
				p_person->Genus.Person->PersonType=PERSON_COP;
				p_person->Draw.Tweened->MeshID	=4;
			}
			else
			{
				p_person->Genus.Person->pcom_ai     = PCOM_AI_GANG;
				p_person->Genus.Person->PersonType=PERSON_THUG_RASTA;
				p_person->Genus.Person->pcom_bent   = PCOM_BENT_FIGHT_BACK;
				p_person->Draw.Tweened->MeshID	=(Random()%3);
			}

			p_person->Genus.Person->pcom_ai_state     = PCOM_AI_STATE_NORMAL;
			p_person->Genus.Person->pcom_ai_substate  = PCOM_AI_SUBSTATE_NONE;
			p_person->Genus.Person->pcom_ai_arg       = NULL;
			p_person->Genus.Person->pcom_ai_other     = NULL;

			p_person->Genus.Person->pcom_move   = PCOM_MOVE_WANDER;
			p_person->Genus.Person->pcom_move_counter = 0;
			p_person->Genus.Person->pcom_move_arg     = 0;

			p_person->Genus.Person->InsideRoom=Random()&15;
			p_person->Draw.Tweened->PersonID = 0;
			p_person->Genus.Person->pcom_colour	=0;
			p_person->Genus.Person->pcom_group	=0;

			if(timer_bored&16)
				p_person->Flags |= FLAGS_HAS_GUN;
			else
				p_person->Flags &= ~FLAGS_HAS_GUN;

			p_person->Genus.Person->Health=200;

			PCOM_set_person_ai_navtokill(p_person,NET_PERSON(0));
			timer_bored=0;

//			PCOM_set_person_ai_kill_person(p_person,NET_PERSON(0),0);
		}
		else
		{
			p_person->Flags &= ~FLAGS_HAS_GUN;
			p_person->Genus.Person->pcom_ai     = PCOM_AI_CIV;
			p_person->Genus.Person->pcom_bent   = 0;

			p_person->Genus.Person->pcom_ai_state     = PCOM_AI_STATE_NORMAL;
			p_person->Genus.Person->pcom_ai_substate  = PCOM_AI_SUBSTATE_NONE;
			p_person->Genus.Person->pcom_ai_arg       = NULL;
			p_person->Genus.Person->pcom_ai_other     = NULL;

			p_person->Genus.Person->pcom_move   = PCOM_MOVE_WANDER;
			p_person->Genus.Person->pcom_move_counter = 0;
			p_person->Genus.Person->pcom_move_arg     = 0;
			p_person->Genus.Person->InsideRoom=Random()&15;

			p_person->Draw.Tweened->PersonID = 0;//6 + Random() % 4;
			p_person->Draw.Tweened->MeshID	=(Random()&1)+8;
			p_person->Genus.Person->pcom_colour	=Random()&3;
			p_person->Genus.Person->PersonType=PERSON_CIV;
			p_person->Genus.Person->Health=130;
			p_person->Flags &= ~FLAGS_HAS_GUN;

			WAND_get_next_place(
				p_person,
			   &wand_x,
			   &wand_z);

			//
			// Go there.
			//

			PCOM_set_person_move_mav_to_xz(
				p_person,
				wand_x,
				wand_z,
				PCOM_MOVE_SPEED_WALK);

			//
			// Find a new place to wander to every five seconds... so we don't get stuck.
			//

			p_person->Genus.Person->pcom_ai_counter = 0;
		}
		return(1);
	}
	return(0);
}

//
// Processes a person moving wandering around
//


void PCOM_process_wander(Thing *p_person)
{
	SLONG wand_x;
	SLONG wand_z;

	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
	{
		if (p_person->Genus.Person->PersonType == PERSON_THUG_RASTA ||
			p_person->Genus.Person->PersonType == PERSON_COP)
		{
			//
			// Fake wandering people shouldn't get too close to Darci if she is busy,
			// but attack her if she _is_ busy.
			//

			if ((PTIME(p_person) & 0xf) == 0)
			{
				if (PCOM_should_fake_person_attack_darci(p_person))
				{
					PCOM_set_person_ai_navtokill(p_person, NET_PERSON(0));

					return;
				}
				else
				{
					if (PCOM_get_dist_between(p_person, NET_PERSON(0)) < 0x100 * 15)
					{
						PCOM_set_person_ai_flee_person(p_person, NET_PERSON(0));

						return;
					}
				}
			}
		}
	}

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_GOTO_XZ:
			if(p_person->Genus.Person->Flags2&FLAG2_PERSON_FAKE_WANDER)
			if (p_person->Genus.Person->pcom_ai   == PCOM_AI_CIV )
			{
				if ((p_person->Flags & FLAGS_IN_VIEW))
				{
					p_person->Genus.Person->InsideRoom=0;
				}
				else
				{
					p_person->Genus.Person->InsideRoom++; //for test puposes

					if(should_person_regen(p_person)) //for test puposes
					{
						SLONG	nx,nz;
						p_person->Genus.Person->InsideRoom=240;

						if(PCOM_do_regen(p_person))
							return;
/*
extern	SLONG	WAND_find_good_start_point(SLONG *mapx,SLONG *mapz);
						if(WAND_find_good_start_point(&nx,&nz))
						{
							GameCoord new_position;

							new_position.X = nx<<8;
							new_position.Y = PAP_calc_height_at(nx,nz)<<8;
							new_position.Z = nz<<8;
							move_thing_on_map(p_person, &new_position);

							if(timer_bored>10000)
							{
								p_person->Genus.Person->pcom_ai     = PCOM_AI_GANG;
								p_person->Genus.Person->pcom_bent   = PCOM_BENT_FIGHT_BACK;

								p_person->Genus.Person->pcom_ai_state     = PCOM_AI_STATE_NORMAL;
								p_person->Genus.Person->pcom_ai_substate  = PCOM_AI_SUBSTATE_NONE;
								p_person->Genus.Person->pcom_ai_arg       = NULL;
								p_person->Genus.Person->pcom_ai_other     = NULL;

								p_person->Genus.Person->pcom_move_counter = 0;
								p_person->Genus.Person->pcom_move_arg     = 0;

								p_person->Genus.Person->InsideRoom=Random()&15;
								p_person->Draw.Tweened->PersonID = 0;//6 + Random() % 4;
								p_person->Draw.Tweened->MeshID	=0;//(Random()&1)+8;
								p_person->Genus.Person->pcom_colour	=Random()&3;
								PCOM_set_person_ai_kill_person(p_person,NET_PERSON(0),0);
							}
							else
							{
								p_person->Genus.Person->InsideRoom=Random()&15;
								p_person->Draw.Tweened->PersonID = 0;//6 + Random() % 4;
								p_person->Draw.Tweened->MeshID	=(Random()&1)+8;
								p_person->Genus.Person->pcom_colour	=Random()&3;

								goto	new_wander;
							}
						}
*/


					}
				}

			}

			//
			// Got to where we are going yet?
			//
			if (!PCOM_finished_nav(p_person) && p_person->Genus.Person->pcom_ai_counter < PCOM_get_duration(50))
			{
				//
				// Still on our way.
				//

				p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

				return;
			}

			if (p_person->Genus.Person->pcom_ai == PCOM_AI_ASSASIN)
			{
				PCOM_set_person_ai_normal(p_person);

				return;
			}

			if (p_person->Genus.Person->pcom_ai == PCOM_AI_DRIVER ||
				p_person->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER)
			{	
				PCOM_set_person_ai_findcar(p_person, NULL);

				return;
			}

			#ifdef BIKE
			if (p_person->Genus.Person->pcom_ai == PCOM_AI_BIKER)
			{	
				PCOM_set_person_ai_findbike(p_person);

				return;
			}
			#endif

			// FALLTHROUGH! To find a new place to wander to.

		default:
		case PCOM_MOVE_STATE_STILL:
//new_wander:;
			//
			// We do we wander to now?
			//

			WAND_get_next_place(
				p_person,
			   &wand_x,
			   &wand_z);

			//
			// Go there.
			//

			PCOM_set_person_move_mav_to_xz(
				p_person,
				wand_x,
				wand_z,
				PCOM_MOVE_SPEED_WALK);

			//
			// Find a new place to wander to every five seconds... so we don't get stuck.
			//

			p_person->Genus.Person->pcom_ai_counter = 0;

			break;
	}
}


//
// The AI for combat.
//

void PCOM_process_killing(Thing *p_person)
{
	Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);
	SLONG	quick_kick=0;

	if (p_person->State == STATE_JUMPING)
	{
		return;
	}

	if (p_person->State == STATE_DANGLING)
	{
		if (p_person->SubState == SUB_STATE_DROP_DOWN)
		{
			return;
		}
	}

	if ((PTIME(p_person) & 0x3) == 0)
	{
		if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
		{
			if (p_person->Genus.Person->PersonType == PERSON_THUG_RASTA ||
				p_person->Genus.Person->PersonType == PERSON_COP)
			{
				if (!PCOM_should_fake_person_attack_darci(p_person))
				{
					PCOM_set_person_ai_flee_person(p_person, NET_PERSON(0));

					return;
				}
			}
		}
	}

	//
	// If our target is dead, then go home.
	//

	if (p_target->State == STATE_DEAD)
	{
		//
		// We might be arresting this person...
		//

		if ((p_target->Genus.Person->Flags & FLAG_PERSON_ARRESTED) && p_target->SubState != SUB_STATE_DEAD_ARRESTED)
		{
			//
			// In which case we must Wait till she is actually arrested!
			//
		}
		else
		{
			//
			// Do what you normally do.
			//

 			PCOM_set_person_ai_normal(p_person);

			return;
		}
	}

	#ifdef BIKE

	//
	// Some people have ulterior motives for killing someone...
	//

	if (p_person->Genus.Person->Flags & FLAG_PERSON_KILL_WITH_A_PURPOSE)
	{
		//
		// For a biker killing someone so they can use his bike.
		//

		if (p_person->Genus.Person->pcom_ai == PCOM_AI_BIKER)
		{
			if (p_target->Genus.Person->Flags & FLAG_PERSON_BIKING)
			{
				//
				// We are trying to get our person off his bike.
				//
			}
			else
			{
				p_person->Genus.Person->Flags &= ~FLAG_PERSON_KILL_WITH_A_PURPOSE;

				//
				// Don't kill em any more.
				//

				remove_from_gang_attack(p_person, p_target);

				//
				// Get on our targets bike!
				//

				PCOM_set_person_ai_findbike(p_person);

				return;
			}
		}
	}

	#endif

	if (PTIME(p_person) & 0x1)
	{
		SLONG too_far;

		if (!is_person_ko(p_target))
		{
			//
			// Make sure we stay close so someone who starts pegging it!
			//

			if (p_target->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PERSON)
			{
				too_far = 0x150;
			}
			else
			{
				too_far = 0x250;
			}

			//
			// Too far from our target? Then renav.
			//

			if (PCOM_get_dist_between(p_person, p_target) > too_far ||
				!can_a_see_b(p_person, p_target) || !there_is_a_los_things(p_person,p_target,LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG|LOS_FLAG_IGNORE_PRIMS|LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
			{
				if (p_person->State == STATE_CIRCLING)
				{
					remove_from_gang_attack(p_person,p_target);
				}

				//
				// Go towards the player to try and beat him up.
				//

				PCOM_set_person_ai_navtokill(p_person, p_target);

				return;
			}
		}
	}

	if (p_target->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER)) 
	{
		//
		// If we have a gun then shoot the car...
		//

		if (PCOM_person_has_any_sort_of_gun_with_ammo(p_person))
		{
			//
			// Navtokill is what shoots!!!
			//

			PCOM_set_person_ai_navtokill(p_person, p_target);

			return;
		}
		else
		{
			//
			// No point in having a fist fight with a car!
			//

			PCOM_set_person_ai_taunt(p_person, p_target);

			return;
		}
	}

//	if (((GAME_TURN+(THING_NUMBER(p_target)<<2)) & 0x3f) == 0)
	if((PTIME(p_person)&0x3f)==0)
	{
		//
		// Is our target ignoring us?
		//

		if (p_target->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
		{
			//
			// Say something aggressive to him- to catch his attention.
			// 

//			MFX_play_thing(THING_NUMBER(p_person),S_WHATRYOULOOKINAT,MFX_REPLACE,p_person);
			//wh wh wh wh wh wh 
#ifndef PSX
			if (IsEnglish)
				MFX_play_thing(THING_NUMBER(p_person),SOUND_Range(S_WTHUG1_ALERT_START,S_WTHUG1_ALERT_START+1),MFX_REPLACE,p_person);
#endif
			PCOM_oscillate_tympanum(
				PCOM_SOUND_LOOKINGATME,
				p_person,
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Y >> 8,
				p_person->WorldPos.Z >> 8);
		}
	}

	//
	// Make a detour to pickup a gun?
	//
/*
	#if PSX
	if (((GAME_TURN + (THING_NUMBER(p_person) << 4)) & 0x7f) == 0)
	#else
	if (((GAME_TURN + (THING_NUMBER(p_person) << 5)) & 0xff) == 0)
	#endif
*/
	#if PSX
	if ((PTIME(p_person)&0x7f)==0)
	#else
	if ((PTIME(p_person)&0xff)==0)
	#endif
	{
		Thing *p_special = PCOM_is_there_an_item_i_should_get(p_person);

		if (p_special)
		{
			PCOM_set_person_ai_getitem(
				p_person,
				p_special,
				PCOM_MOVE_SPEED_RUN,
				PCOM_EXCAR_NAVTOKILL,
				p_person->Genus.Person->pcom_ai_arg);

			return;
		}
	}

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:

			PCOM_set_person_move_circle(p_person, p_target);

			//
			// Wait for between 0.5 to 1.0 seconds between attacks.  Increase
			// or decrease this number to make people easier to kill.
			//

			p_person->Genus.Person->pcom_ai_counter = PCOM_get_random_duration(15, 20);

			break;

		case PCOM_MOVE_STATE_PAUSE:
		case PCOM_MOVE_STATE_CIRCLE:

			//
			// All the AI code is done by fn_person_circle now.
			//

			break;

		case PCOM_MOVE_STATE_ANIMATION:
		case PCOM_MOVE_STATE_WAIT_CIRCLE:
		case PCOM_MOVE_STATE_GRAPPLE:
			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Processes a person running away from something.
//	i.e. PCOM_AI_STATE_FLEE_PLACE
//       PCOM_AI_STATE_FLEE_PERSON
//

void PCOM_process_fleeing(Thing *p_person)
{
	switch(p_person->Genus.Person->pcom_ai_substate)
	{	
		case PCOM_AI_SUBSTATE_SUPRISED:

			//
			// Finished looking dumb?
			//

			if (p_person->Genus.Person->pcom_ai_counter > PCOM_get_duration(10))
			{
				//
				// Start running away.
				//

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LEGIT;
				p_person->Genus.Person->pcom_ai_counter  = 0;

				//
				// Get the person moving!
				//

				{
					SLONG danger_x;
					SLONG danger_z;

					PCOM_get_flee_from_pos(
						p_person,
					   &danger_x,
					   &danger_z);

					PCOM_set_person_move_runaway(
						p_person,
						danger_x,
						danger_z);
				}
			}
			else
			{
				p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
			}

			break;

		case PCOM_AI_SUBSTATE_LEGIT:

			//
			// Let go of a balloon if you are carrying one.
			//

#if !defined(PSX) && !defined(TARGET_DC)
			if (p_person->Genus.Person->Balloon && p_person->Genus.Person->pcom_ai_counter > PCOM_get_duration(5))
			{
				BALLOON_release(p_person->Genus.Person->Balloon);
			}
#endif

			//
			// If we have got to where we were legging it to, then leg-it again.
			//

			if (PCOM_finished_nav(p_person) || p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				SLONG danger_x;
				SLONG danger_z;

				PCOM_get_flee_from_pos(
					p_person,
				   &danger_x,
				   &danger_z);

				PCOM_set_person_move_runaway(
					p_person,
					danger_x,
					danger_z);
			}

			p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

//			if ((GAME_TURN & 0xf) == 0)
			if ((PTIME(p_person) & 0xf) == 0)
			{
				SLONG danger_x;
				SLONG danger_z;

				PCOM_get_flee_from_pos(
					p_person,
				   &danger_x,
				   &danger_z);

				//
				// Are we far enough from what we are scared of?
				//

				SLONG dx = danger_x - (p_person->WorldPos.X >> 8);
				SLONG dz = danger_z - (p_person->WorldPos.Z >> 8);

				SLONG dist = abs(dx) + abs(dz);

				SLONG want_dist = 0x100 * 15;

				if (p_person->Genus.Person->pcom_ai_counter >= PCOM_get_duration(300))
				{
					want_dist = 0x100 * 3;
				}

				if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
				{
					if (p_person->Genus.Person->PersonType == PERSON_THUG_RASTA ||
						p_person->Genus.Person->PersonType == PERSON_COP)
					{
						want_dist = 0x100 * 20;
					}
				}

				if (dist > want_dist)
				{
					//
					// Nothing to be scared of any more- we can relax...
					//

					PCOM_set_person_move_still(p_person);

					//
					// ...and go back to doing what we were doing before.
					//	

					PCOM_set_person_ai_normal(p_person);
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Processes a person investigating something.
//

void PCOM_process_investigating(Thing *p_person)
{
	SLONG dist;

	SLONG before;
	SLONG after;

	SLONG hide_x;
	SLONG hide_z;

	SLONG sound_x;
	SLONG sound_z;

	//
	// The position of the sound we are investigating.
	//

	sound_x = (p_person->Genus.Person->pcom_ai_arg >> 8) & 0xff;
	sound_z = (p_person->Genus.Person->pcom_ai_arg >> 0) & 0xff;

	sound_x <<= 8;
	sound_z <<= 8;
	
	sound_x += 0x80;
	sound_z += 0x80;

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_SUPRISED:

			//
			// Enough time being suprised?
			//

			if (p_person->Genus.Person->pcom_ai_counter > PCOM_get_duration(10))
			{
				//
				// If the person can see where the sound came from then
				// there is no point walking over there.
				//

				SLONG sound_y = PAP_calc_map_height_at(sound_x, sound_z) + 0x60;

				if (there_is_a_los(
						p_person->WorldPos.X          >> 8,
						p_person->WorldPos.Y + 0x6000 >> 8,
						p_person->WorldPos.Z          >> 8,
						sound_x,
						sound_y + 0x80,
						sound_z,
						LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
				{
					//
					// Don't walk over.
					//

					PCOM_set_person_ai_normal(p_person);
				}
				else
				{
					//
					// Start walking to where the sound came from.
					//

					PCOM_set_person_move_mav_to_xz(
						p_person,
						sound_x,
						sound_z,
						PCOM_MOVE_SPEED_WALK);

					if (p_person->Genus.Person->pcom_bent & PCOM_BENT_RESTRICTED)
					{
						//
						// If you are restricted movement and you can't get to your
						// target for certain then give up and go back to your normal
						// routine.
						//

						if (!MAV_do_found_dest)
						{
							PCOM_set_person_ai_normal(p_person);

							return;
						}
					}

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_WALKOVER;
				}
			}
			else
			{
				p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
			}

			break;

		case PCOM_AI_SUBSTATE_WALKOVER:

			//
			// Are we where we want to be to investigate the sound?
			//

			dist = PCOM_person_dist_from(
						p_person,
						sound_x,
						sound_z);

			if (dist < PCOM_ARRIVE_DIST)
			{
				//
				// Start looking around for signs of trouble.
				//

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LOOK;
				p_person->Genus.Person->pcom_ai_counter  = 0;

				PCOM_set_person_move_still(p_person);
			}

			if (p_person->Genus.Person->pcom_move_state != PCOM_MOVE_STATE_GOTO_XZ)
			{
				PCOM_set_person_move_mav_to_xz(
					p_person,
					sound_x,
					sound_z,
					PCOM_MOVE_SPEED_WALK);

				// PANEL_new_help_message("Important! Tell Mark that lavender is blue.");
			}

			p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			break;

		case PCOM_AI_SUBSTATE_LOOK:

			before = p_person->Genus.Person->pcom_ai_counter;
			after  = p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			if (before < PCOM_get_duration(10) && after >= PCOM_get_duration(10))
			{
				//
				// Turn to the left.
				//

				p_person->Draw.Tweened->AngleTo -= 512;
				p_person->Draw.Tweened->Angle   -= 512;

				p_person->Draw.Tweened->AngleTo &= 2047;
				p_person->Draw.Tweened->Angle   &= 2047;
			}

			if (before < PCOM_get_duration(30) && after >= PCOM_get_duration(30))
			{
				//
				// Turn to the right.
				//

				p_person->Draw.Tweened->AngleTo += 1024;
				p_person->Draw.Tweened->Angle   += 1024;

				p_person->Draw.Tweened->AngleTo &= 2047;
				p_person->Draw.Tweened->Angle   &= 2047;
			}

			if (after > PCOM_get_duration(50))
			{
				//
				// Didn't find any shenanigans.
				//

				if (Random() & 0x1)
				{
					//
					// Go back home.
					//

					PCOM_set_person_ai_homesick(p_person);
				}
				else
				{
					//
					// Look for somewhere where somebody might be hiding.
					//

					if (PCOM_find_hiding_place(
							p_person,
						   &hide_x,
						   &hide_z))
					{
						//
						// Start walking to the hiding place.
						//

						PCOM_set_person_move_mav_to_xz(
							p_person,
							hide_x,
							hide_z,
							PCOM_MOVE_SPEED_WALK);

						p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_WALKOVER;

						hide_x >>= 8;
						hide_z >>= 8;

						p_person->Genus.Person->pcom_ai_arg = (hide_x << 8) | hide_z;
					}
					else
					{
						//
						// Couldn't see anywhere where somebody might be hiding- go home.
						//

						PCOM_set_person_ai_homesick(p_person);
					}
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// The speed the person wants to follow the target at.
//

SLONG PCOM_follow_speed(Thing *p_person, Thing *p_target)
{
	SLONG wantspeed;

	if (p_target->Genus.Person->PlayerID)
	{
		switch(p_target->Genus.Person->Mode)
		{
			case PERSON_MODE_RUN:    wantspeed = PERSON_SPEED_YOMP;  break;
			case PERSON_MODE_WALK:   wantspeed = PERSON_SPEED_WALK;  break;
			case PERSON_MODE_SNEAK:	 wantspeed = PERSON_SPEED_SNEAK; break;
			case PERSON_MODE_FIGHT:	 wantspeed = PERSON_SPEED_RUN;   break;
			case PERSON_MODE_SPRINT: wantspeed = PERSON_SPEED_RUN;   break;

			default:
				ASSERT(0);
				break;
		}

		if (p_target->SubState == SUB_STATE_CRAWLING     ||
			p_target->SubState == SUB_STATE_IDLE_CROUTCH ||
			p_target->SubState == SUB_STATE_IDLE_CROUTCHING)
		{
			wantspeed = PERSON_SPEED_CRAWL;
		}
	}
	else
	{
		wantspeed = p_target->Genus.Person->GotoSpeed;

		if (wantspeed != PERSON_SPEED_RUN   &&
			wantspeed != PERSON_SPEED_WALK  &&
			wantspeed != PERSON_SPEED_SNEAK &&
			wantspeed != PERSON_SPEED_YOMP  &&
			wantspeed != PERSON_SPEED_CRAWL)
		{
			wantspeed = PERSON_SPEED_RUN;
		}
	}

	return wantspeed;
}



void PCOM_process_following(Thing *p_person)
{
	SLONG dist;
	SLONG wantspeed;

	//
	// Who are we following?
	//

	Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	//
	// If our target is in a vehicle and we are not in it...
	//

	if (p_target->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER))
	{
		if (p_person->Genus.Person->Flags & FLAG_PERSON_PASSENGER)
		{
			//
			// We are in a car too... assume it is the right one!
			//

			ASSERT(p_person->Genus.Person->InCar == p_target->Genus.Person->InCar);

			return;
		}
		else
		{
			//
			// Get into the same car as who we are following.
			//

			PCOM_set_person_ai_hitch(p_person, TO_THING(p_target->Genus.Person->InCar));

			return;
		}
	}
	else
	{
		if (p_person->Genus.Person->Flags & FLAG_PERSON_PASSENGER)
		{
			//
			// We are in a car but our target isn't. Get out of the car.
			//

			PCOM_set_person_ai_leavecar(p_person, PCOM_EXCAR_NORMAL, 0, 0);

			return;
		}
	}

	if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_PAUSE)
	{
		//
		// Finished pausing?
		//

//		if ((GAME_TURN & 0x3) == 0)
		if ((PTIME(p_person) & 0x3) == 0)
		{
			SLONG wantdist = 0xa0;

			//
			// If our target is moving we want to be really close!
			//

			if (p_target->SubState == SUB_STATE_RUNNING)
			{
				wantdist = 0x40;
			}

			if (PCOM_get_dist_between(
					p_person,
					p_target) > wantdist)
			{
				wantspeed = PCOM_follow_speed(p_person, p_target);

				PCOM_set_person_move_mav_to_thing(
					p_person,
					p_target,
					wantspeed);

				return;
			}
			else
			{
				//
				// Crouch if our target crouches.
				//

				if (p_target->SubState == SUB_STATE_CRAWLING     ||
					p_target->SubState == SUB_STATE_IDLE_CROUTCH ||
					p_target->SubState == SUB_STATE_IDLE_CROUTCHING)
				{
					if (p_person->SubState == SUB_STATE_IDLE_CROUTCH ||
						p_person->SubState == SUB_STATE_IDLE_CROUTCHING)
					{
						//
						// We are croutching like our target already.
						//
					}
					else
					{
						set_person_croutch(p_person);
					}
				}
				else
				if (p_person->SubState == SUB_STATE_IDLE_CROUTCH ||
					p_person->SubState == SUB_STATE_IDLE_CROUTCHING)
				{
					//
					// We are croutching for no reason.
					//

					set_person_idle_uncroutch(p_person);
				}
				else
				{
					//
					// Pickup a weapon?
					//

					if (p_person->Genus.Person->pcom_ai != PCOM_AI_CIV)
					{
						#if PSX
//						if (((GAME_TURN + (THING_NUMBER(p_person) << 4)) & 0x3f) == 0)
						if ((PTIME(p_person)&0x3f)==0)
						#else
						//if (((GAME_TURN + (THING_NUMBER(p_person) << 5)) & 0x7f) == 0)
						if ((PTIME(p_person)&0x7f)==0)
						#endif
						{
							Thing *p_special = PCOM_is_there_an_item_i_should_get(p_person);

							if (p_special)
							{
								PCOM_set_person_ai_getitem(
									p_person,
									p_special,
									PCOM_MOVE_SPEED_RUN,
									PCOM_EXCAR_NORMAL,
									0);

								return;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		{
			//
			// Following people should run if they are too far from their target,
			// but if they are close enough, they should copy their target's
			// speed- i.e. walk or sneak.  Only take (dx,dz) into account though...
			//

			SLONG dx = abs(p_target->WorldPos.X - p_person->WorldPos.X >> 8);
			SLONG dz = abs(p_target->WorldPos.Z - p_person->WorldPos.Z >> 8);

			dist = QDIST2(dx,dz);

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_GOTO_THING && (p_person->Genus.Person->pcom_move_substate == PCOM_MOVE_SUBSTATE_GOTO || p_person->Genus.Person->pcom_move_substate == PCOM_MOVE_SUBSTATE_LOSMAV))
			{
				if (dist > 0x180)
				{
					if (p_person->Genus.Person->GotoSpeed != PERSON_SPEED_RUN)
					{
						//
						// Start running!
						//

						PCOM_set_person_move_mav_to_thing(p_person, p_target, PERSON_SPEED_RUN);
					}
					else
					{
					}
				}
				else
				if (dist < 0xc0)
				{
					//
					// Copy the targets movement.
					//

					wantspeed = PCOM_follow_speed(p_person, p_target);

					if (p_person->Genus.Person->GotoSpeed != wantspeed)
					{
						//
						// Get going at the new speed- except we are allowed to sprint
						// if our target isn't so we can keep up!
						//

						if (p_person->Genus.Person->GotoSpeed == PERSON_SPEED_RUN)
						{
							if (wantspeed == PERSON_SPEED_WALK ||
								wantspeed == PERSON_SPEED_SNEAK)
							{
								//
								// This is a big change of speed!
								//

								PCOM_set_person_move_mav_to_thing(p_person, p_target, wantspeed);
							}
						}
						else
						{
							PCOM_set_person_move_mav_to_thing(p_person, p_target, wantspeed);
						}
					}
				}

				//
				// Extra boost if you're lagging behind!
				//

				if (p_person->Velocity < p_target->Velocity)
				{
					p_person->Velocity += 1;
				}
			}
		}

		//
		// Don't stop moving if you're in the middle of doing some complicated thing.
		//

		if (p_person->State == STATE_GOTOING)
		{
			SLONG dx = abs(p_target->WorldPos.X - p_person->WorldPos.X >> 8);
			SLONG dz = abs(p_target->WorldPos.Z - p_person->WorldPos.Z >> 8);

			SLONG dist = QDIST2(dx,dz);

			SLONG wantdist;

			//
			// If our target is moving we want to be really close!
			//

			if (p_target->SubState == SUB_STATE_RUNNING)
			{
				wantdist = 0x40;
			}
			else
			{
				wantdist = 0x80;
			}

			if (dist < wantdist)
			{
				//
				// Pause for a while.
				//

				PCOM_set_person_move_pause(p_person);

				if (p_target->SubState == SUB_STATE_CRAWLING     ||
					p_target->SubState == SUB_STATE_IDLE_CROUTCH ||
					p_target->SubState == SUB_STATE_IDLE_CROUTCHING)
				{
					set_person_idle_croutch(p_person);
				}
			}
		}
	}
}


//
// Returns a place for the MIB to disappear and reappear to.
//

void PCOM_find_mib_appear_pos(
		Thing *p_mib,
		Thing *p_target,
		SLONG *appear_x,
		SLONG *appear_z)
{
	
}


#ifndef TARGET_DC
void	draw_view_line(Thing *p_person,Thing *p_target)
{
#ifdef PSX
	return;
#else
	SLONG	x1,y1,z1,x2,y2,z2;
	SLONG	dx,dy,dz;
	SLONG	len,step,count;



	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_HEAD,&x1,&y1,&z1);

	x1+=p_person->WorldPos.X>>8;
	y1+=p_person->WorldPos.Y>>8;
	z1+=p_person->WorldPos.Z>>8;

	calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_HEAD,&x2,&y2,&z2);

	x2+=p_target->WorldPos.X>>8;
	y2+=p_target->WorldPos.Y>>8;
	z2+=p_target->WorldPos.Z>>8;


	dx=x2-x1;
	dy=y2-y1;
	dz=z2-z1;

	len=QDIST3(abs(dx),abs(dy),abs(dz));

#ifndef PSX
	dx=(dx*20)/len;
	dy=(dy*20)/len;
	dz=(dz*20)/len;

	count=(len/20)>>1;
#else
	dx=(dx*40)/len;
	dy=(dy*40)/len;
	dz=(dz*40)/len;

	count=(len/40)>>1;
#endif

	for(step=0;step<count;step++)
	{
		//AENG_world_line(x1,y1,z1,2,0,x1+dx,y1+dy,z1+dz,2,0,TRUE);
		AENG_world_line(x1,y1,z1,20,0xffffff,x1+dx,y1+dy,z1+dz,20,0xffffff,TRUE);

		x1+=dx<<1;
		y1+=dy<<1;
		z1+=dz<<1;

	}
#endif
	

}
#endif


void PCOM_process_navtokill(Thing *p_person)
{
	SLONG dist;
	SLONG hit_distance;
	SLONG	special;
	
	Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	if(p_target->State==STATE_DEAD && p_target->Genus.Person->PlayerID)
	{
		//
		// the player is dead, he was my target!
		//

		PCOM_set_person_ai_investigate(p_person,p_target->WorldPos.X>>8,p_target->WorldPos.Z>>8);
		return;

	}

	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
		timer_bored=0;

	if(p_person->State==STATE_JUMPING)
		return;

	if ((PTIME(p_person) & 0x7) == 0)
	{	
		if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
		{
			//
			// If our target (i.e. Darci!) is doing something interesting,
			// then don't attack her!
			//

			if (!PCOM_should_fake_person_attack_darci(p_person))
			{
				PCOM_set_person_ai_flee_person(p_person, NET_PERSON(0));

				return;
			}
		}
	}

	//
	// The distance we have to be from our target to start hitting her.
	//

	hit_distance = 240;

	switch (p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_WAITING:

			//
			// Waiting for the player to get out of the cutscene...
			//

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				if (!EWAY_stop_player_moving())
				{
					PCOM_set_person_move_mav_to_thing(
						p_person,
						p_target,
						PCOM_MOVE_SPEED_RUN);

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING;
				}
				else
				if ((PTIME(p_person) & 0x3) == 0)
				{
					if ((Random() & 0x3) == 0)
					{
						//
						// Another taunt!
						//

						set_face_thing(
							p_person,
							p_target);

						PCOM_set_person_move_animation(p_person, ANIM_WANKER);
					}
				}
			}

			break;

		case PCOM_AI_SUBSTATE_HUNTING_SLIDE:

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				//
				// Finished doing our sliding tackle.
				//

				PCOM_set_person_move_mav_to_thing(
					p_person,
					p_target,
					PCOM_MOVE_SPEED_RUN);

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING;
			}

			break;

		case PCOM_AI_SUBSTATE_HUNTING:

			if (p_target == NET_PERSON(0))
			{
				if (EWAY_stop_player_moving())
				{
//					if (((GAME_TURN + THING_NUMBER(p_person)) & 0x1f) == 8)
					if (((PTIME(p_person)) & 0x1f) == 9)
					{
						//
						// Wait for the player to stop being in the cutscene.
						//

						set_face_thing(
							p_person,
							p_target);

						PCOM_set_person_move_animation(p_person, ANIM_WANKER);

						p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_WAITING;
					}

					return;
				}
			}


			if (p_person->State == STATE_IDLE ||
				p_person->State == STATE_GUN  ||
				p_person->State == STATE_GOTOING)
			{
				//
				// Don't navtokill outside your zone...
				//

				if (p_person->Genus.Person->pcom_zone)
				{
					if (!(p_person->Genus.Person->pcom_zone & PCOM_get_zone_for_position(p_target)))
					{
						//
						// Our target is no longer in our zone...
						//

						if ((p_person->Genus.Person->pcom_zone & PCOM_get_zone_for_position(p_person->Genus.Person->HomeX,p_person->Genus.Person->HomeZ)))
						{
							//
							// His Home is in the right zone
							//
							PCOM_set_person_ai_homesick(p_person);

							return;

						}
						else
						{
							//
							// This guys home isnt in his zone!, this is a level design error so clear his zone flag
							//

							p_person->Genus.Person->pcom_zone=0;


						}

					}
				}

				//
				// Make a detour to pickup a gun...
				//

				#if PSX
//				if (((GAME_TURN + (THING_NUMBER(p_person) << 4)) & 0x7f) == 0)
				if (((PTIME(p_person)) & 0x7f) == 0)
				#else
				if (((PTIME(p_person)) & 0xff) == 0)
//				if (((GAME_TURN + (THING_NUMBER(p_person) << 5)) & 0xff) == 0)
				#endif
				{
					Thing *p_special = PCOM_is_there_an_item_i_should_get(p_person);

					if (p_special)
					{
						PCOM_set_person_ai_getitem(
							p_person,
							p_special,
							PCOM_MOVE_SPEED_RUN,
							PCOM_EXCAR_NAVTOKILL,
							p_person->Genus.Person->pcom_ai_arg);

						return;
					}
				}

				if (PCOM_person_has_any_sort_of_gun_with_ammo(p_person) && !is_person_ko(p_target))
				{
					SLONG check_look;

					//
					// I have a gun so only go into fight mode at the last second
					//

					hit_distance >>= 1;

					//
					// How often we check we can see enemy.
					//		   

					/*
					if ((p_person->Genus.Person->pcom_bent & PCOM_BENT_KILL_ON_SIGHT))
					{
						check_look = PCOM_get_duration(2);
					}
					else
					*/
					{
						check_look = PCOM_get_duration(5) - PCOM_get_duration100(GET_SKILL(p_person) << 1);
					}

					//
					// Only check now and again to see if you can see enemy.
					//

					if (p_person->Genus.Person->pcom_ai_counter > check_look || p_person->Genus.Person->pcom_ai == PCOM_AI_SHOOT_DEAD)
					{
						if (p_target->Genus.Person->Mode == PERSON_MODE_FIGHT)// && !(p_person->Genus.Person->pcom_bent & PCOM_BENT_KILL_ON_SIGHT))
						{
							//
							// Don't shoot the player if she is fighting someone (unless you are
							// flagged as KILL_ON_SIGHT).
							//
						}
						else
						{
							//
							// If you are near enough and you can see your target, then take a pot-shot.
							//

							if (PCOM_get_dist_between(p_person, p_target) < 0x400 && can_a_see_b(p_person, p_target))
							{
								p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_AIMING;
								p_person->Genus.Person->pcom_ai_counter  = 0;

								PCOM_set_person_move_still(p_person);

								break;
							}
						}

						p_person->Genus.Person->pcom_ai_counter = 0;
					}
				}

				//
				// Near enough the player to attack?
				//

				dist = PCOM_get_dist_between(p_person, p_target);

				//
				// Fake killers, relieving boredom means there will only ever be one
				//
				if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
				{
					if(dist>30<<8 || EWAY_stop_player_moving())
					{
						//
						// I he gets too far from his target and not in view try a respawn
						//
						if (!(p_person->Flags & FLAGS_IN_VIEW))
						{
							p_person->Genus.Person->InsideRoom++; //for test puposes

							p_person->Genus.Person->InsideRoom=240;

							if(PCOM_do_regen(p_person))
								return;
						}
					}
				}

				if (p_person->SubState == SUB_STATE_RUNNING &&
					p_target->SubState == SUB_STATE_RUNNING)
				{
					if (p_person->Velocity >= 20 && dist < 0xc0)
					{
						if (p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
						{
							//
							// Fight test dummies don't perform sliding tackles.
							//
						}
						else
						{
							//
							// Fast enough and close enough. Do it less often to the player.
							//

//							if ((GAME_TURN & 0x7) == 0 || !p_target->Genus.Person->PlayerID)
							if ((PTIME(p_person) & 0x7) == 0 || !p_target->Genus.Person->PlayerID)
							{
								//
								// Must be able to see them to tackle them
								//

								if (can_a_see_b(p_person, p_target))
								{
									//
									// Near enough to try a sliding tackle!
									// 

									p_person->Velocity += 10;

									p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING_SLIDE;

									PCOM_set_person_move_goto_thing_slide(p_person,p_target);
								}
							}
						}
					}
				}
				else
				if (dist < hit_distance)
				{
					//
					// Don't hit him if he is at a different level to me.
					//

					if (abs(p_person->WorldPos.Y - p_target->WorldPos.Y) < 0x7000)
					{
						SLONG x1;
						SLONG y1;
						SLONG z1;

						SLONG x2;
						SLONG y2;
						SLONG z2;

						//
						// Start killing the target, only if there is a los between him and me.
						//

						x1 = p_person->WorldPos.X          >> 8;
						y1 = p_person->WorldPos.Y + 0x6000 >> 8;
						z1 = p_person->WorldPos.Z          >> 8;

						x2 = p_target->WorldPos.X          >> 8;
						y2 = p_target->WorldPos.Y + 0x6000 >> 8;
						z2 = p_target->WorldPos.Z          >> 8;

						if (there_is_a_los(
								x1, y1, z1,
								x2, y2, z2,
								LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG |
								LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
						{
							if ((p_target->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER)) && PCOM_person_has_any_sort_of_gun_with_ammo(p_person))
							{
								//
								// Shoot people in cars if you have a gun...
								//

								p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_AIMING;
								p_person->Genus.Person->pcom_ai_counter  = 0;

								PCOM_set_person_move_still(p_person);
							}
							else
							{
								//
								// If this person has a h2h weapon, he'll draw it now! 
								//

								special = PCOM_person_has_any_sort_of_h2h(p_person);

								if (p_person->Genus.Person->SpecialUse && TO_THING(p_person->Genus.Person->SpecialUse)->Genus.Special->SpecialType == special)
								{
									//
									// He already has this weapon drawn.
									//

									special = NULL;
								}

								if (special)
								{
									//
									// Draw a hand 2 hand combat weapon.
									//

									PCOM_set_person_move_draw_h2h(p_person,special);

									p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_DRAW_H2H;
								}
								else
								{
									if (p_target->Genus.Person->Flags & FLAG_PERSON_DRIVING)
									{
										if (PCOM_person_has_any_sort_of_gun_with_ammo(p_person))
										{
											//
											// Shoot people in cars if you have a gun...
											//

											p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_AIMING;
											p_person->Genus.Person->pcom_ai_counter  = 0;

											PCOM_set_person_move_still(p_person);
										}
										else
										{
											//
											// No point having a fist fight with a car!
											//

											PCOM_set_person_ai_taunt(p_person, p_target);
										}
									}
									else
									{
										PCOM_set_person_ai_kill_person(p_person, p_target);
									}
								}
							}
						}
					}
				}
			}

			break;

		case PCOM_AI_SUBSTATE_AIMING:

			if (p_target == NET_PERSON(0))
			{
				if (EWAY_stop_player_moving())
				{
//					if (((GAME_TURN + (THING_NUMBER(p_person) << 2)) & 0x1f) == 8)
					if ((PTIME(p_person) & 0x1f) == 8)
					{
						//
						// Wait for the player to stop being in the cutscene.
						//

						set_face_thing(
							p_person,
							p_target);

						PCOM_set_person_move_animation(p_person, ANIM_WANKER);

						p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_WAITING;
					}

					return;
				}
			}

			if (PCOM_person_has_any_sort_of_gun(p_person))
			{
				if (!PCOM_person_has_gun_in_hand(p_person))
				{
					if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
					{
						//
						// Draw your gun.
						//

						PCOM_set_person_move_draw_gun(p_person);
					}

					p_person->Genus.Person->pcom_ai_counter = 0;
				}
				else
				{
					if (p_target->Genus.Person->PlayerID && EWAY_stop_player_moving())
					{
						//
						// Don't shoot the player while there are stationary in a cutscene.
						//

						p_person->Genus.Person->pcom_ai_counter = 0;
					}

					//
					// How quickly we shoot.
					//

					SLONG shoot_time;
					
					shoot_time  = PCOM_get_duration   (get_rate_of_fire(p_person));
//
//					shoot_time -= PCOM_get_duration100(GET_SKILL(p_person)<<2);

					//
					// I've made poeple shooting less accurate... so they can fire more often.
					//

					shoot_time -= shoot_time >> 2;

					/*
					if (p_person->Genus.Person->pcom_bent & PCOM_BENT_KILL_ON_SIGHT)
					{
						//
						// Always shoot if you are kill on sight.
						//
					}
					else
					*/
					{
						if (p_target->Genus.Person->Mode == PERSON_MODE_FIGHT)
						{
							//
							// Don't shoot our target as often while he is in fight mode.
							// 

							if (p_target->Genus.Person->Target == THING_NUMBER(p_person))
							{
								//
								// This bloke is fighting me!
								//
							}
							else
							{
								shoot_time <<= 1;
							}
						}
					}

					//
					// If our target is running away from us!
					//

					if (p_target->Class == CLASS_PERSON && p_target->Genus.Person->Mode == PERSON_MODE_SPRINT)
					{
						//
						// Stop people being able to sprint through the levels!
						//

						p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
					}

					if (p_person->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD && p_target->Genus.Person->PlayerID==0)
					{
						shoot_time>>=2;
					}

					if(dist_to_target(p_person,p_target)< (10<<8) )
						track_gun_sight(p_target,shoot_time-p_person->Genus.Person->pcom_ai_counter);
//					ASSERT(dist_to_target(p_person,p_target)< (18<<8) );


#ifndef TARGET_DC
// (a) doesn't work and (b) doesn't look any good. So it's toast.
					if(p_target->Genus.Person->PlayerID)
					{
						draw_view_line(p_person,p_target);
					}
#endif

					if (p_person->Genus.Person->pcom_move_state    == PCOM_MOVE_STATE_ANIMATION &&
						p_person->Genus.Person->pcom_move_substate == PCOM_MOVE_SUBSTATE_SHOOT)
					{
						//
						// Already shooting!
						//

						p_person->Genus.Person->pcom_ai_counter = 0;
					}
					else
					{

//						if(p_person->Genus.Person->pcom_ai == PCOM_AI_SHOOT_DEAD && !is_person_ko(p_target))
//							shoot_time=0;

						if (p_person->Genus.Person->pcom_ai_counter > shoot_time || p_person->Genus.Person->pcom_ai == PCOM_AI_SHOOT_DEAD)
						{
							//
							// Time to shoot.
							//

							if (PCOM_get_dist_between(p_person, p_target) < 0x600 && can_a_see_b(p_person, p_target))
							{
								/*

								//
								// If you are being shot by a KILL_ON_SIGHT person, then you are
								// going to die! Make the camera go to the person who is shooting.
								//

								if (p_person->Genus.Person->pcom_bent & PCOM_BENT_KILL_ON_SIGHT)
								{
									if (GAME_cut_scene == 0)
									{
										if(p_person->Genus.Person->PlayerID)
										{
											FC_kill_player_cam(p_person);
										}
									}
								}
								*/

								//
								// Do we have any ammo left?
								//

								if (!PCOM_person_has_any_sort_of_gun_with_ammo(p_person))
								{
									//
									// No ammo
									//

									//
									// If you have no ammo then all this function does is play
									// a click sound.
									//

									set_person_shoot(p_person,1);

									//
									// Put your gun away.
									//

									PCOM_set_person_move_gun_away(p_person);

									p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NOMOREAMMO;
								}
								else
								{
									//
									// Take a shot!
									//

									PCOM_set_person_move_shoot(p_person);
								}
							}
							else
							{
	//							CONSOLE_text("Too far away or can't see player.");

								//
								// Hunt down the player again so we can try to take another shot.
								//

								p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING;

								PCOM_set_person_move_mav_to_thing(
									p_person,
									p_target,
									PCOM_MOVE_SPEED_RUN);
							}

							p_person->Genus.Person->pcom_ai_counter = 0;
						}
					}
				}

				//
				// Always face your target while aiming.
				//

				set_face_thing(p_person, p_target);
			}
			else
			{
				//
				// may have had the gun kicked out of hand so go into hunting mode
				//

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING;

				PCOM_set_person_move_mav_to_thing(
					p_person,
					p_target,
					PCOM_MOVE_SPEED_RUN);
			}

			break;

		case PCOM_AI_SUBSTATE_NOMOREAMMO:
			
			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				//
				// Finished putting our gun away- get hunting again!
				//

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HUNTING;

				PCOM_set_person_move_mav_to_thing(
					p_person,
					p_target,
					PCOM_MOVE_SPEED_RUN);
			}

			break;

		case PCOM_AI_SUBSTATE_DRAW_H2H:

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				//
				// Finished drawing our h2h weapon. Do we have to renav, or
				// can we just start killing?
				//

				if (PCOM_get_dist_between(p_person, p_target) < 0x100 && can_a_see_b(p_person, p_target))
				{
					PCOM_set_person_ai_kill_person(p_person, p_target);
				}
				else
				{
					PCOM_set_person_ai_navtokill(p_person, p_target);
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	if (p_target->State == STATE_DEAD)
	{
		//
		// Go back to do what you normally do.
		//

		PCOM_set_person_ai_normal(p_person);
	}

	p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
}


void PCOM_process_findcar(Thing *p_person)
{
	SLONG door;

	Thing *p_vehicle = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	ASSERT(p_vehicle->Class == CLASS_VEHICLE);

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_GOTOCAR:

			if (p_person->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER ||
				p_person->Genus.Person->pcom_ai == PCOM_AI_DRIVER)
			{
				//
				// These people don't want to get into a car driven by someone else!
				//

				if (p_vehicle->Genus.Vehicle->Driver)
				{
					PCOM_set_person_ai_findcar(p_person, NULL);
				}
			}

			{
				//
				// Get into the car if nobody can see you and you've been near to the
				// car for 20 gameturns.
				//

				if ((p_person->Flags & FLAGS_IN_VIEW) || PCOM_get_dist_between(p_person, p_vehicle) > 0x200)
				{
					p_person->Genus.Person->pcom_ai_counter = 0;
				}
				else
				{
					p_person->Genus.Person->pcom_ai_counter += 1;
				}

				extern SLONG in_right_place_for_car(Thing *p_person, Thing *p_vehicle, SLONG *door);
	
				door = 0;

				if (p_person->Genus.Person->pcom_ai_counter > 20 || in_right_place_for_car(p_person, p_vehicle, &door))
				{
					//
					// We can get into a car! Lets do it.
					//

					ASSERT(door == 0 || door == 1);

					PCOM_set_person_move_getincar(p_person, p_vehicle, FALSE, door);

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GETINCAR;
					p_person->Genus.Person->pcom_ai_counter  = 0;
				}
			}

			break;

		case PCOM_AI_SUBSTATE_GETINCAR:

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				//
				// We have got in the car... what now?
				//

//				if ( ((GAME_TURN & 0xf)==(THING_NUMBER(p_person)&0xf)) || PCOM_are_there_people_who_want_to_enter(p_vehicle))
				//
				// don't use ptime we want this to happen quick
				//
#ifdef	PSX
				if ( ((GAME_TURN & 0x7)==(THING_NUMBER(p_person)&0x7)) || PCOM_are_there_people_who_want_to_enter(p_vehicle))
#else
				if ( ((GAME_TURN & 0xf)==(THING_NUMBER(p_person)&0xf)) || PCOM_are_there_people_who_want_to_enter(p_vehicle))
#endif
//				if ( ((PTIME(p_person) & 0xf)==0) || PCOM_are_there_people_who_want_to_enter(p_vehicle))
				{
					// every 16 game turns is false, except what if we arent processed on that game turn due to time slice, so ignore bottom 2 bits

					//
					// Wait a while...
					// 
				}
				else
				{
					p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_NORMAL;
					p_person->Genus.Person->pcom_ai_substate = 0;
					p_person->Genus.Person->pcom_ai_counter  = 0;

					p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_STILL;
					p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
				}
			}
			else
			{
				if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(40))
				{
					//
					// We've spent a long time. Maybe there's a problem with getting into this car!
					//

					PCOM_set_person_ai_findcar(p_person, p_person->Genus.Person->pcom_ai_arg);
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

#ifdef BIKE

void PCOM_process_findbike(Thing *p_person)
{
	SLONG  dx;
	SLONG  dz;
	SLONG  dist;
	Thing *p_bike;

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_GOTOBIKE:

			p_bike = TO_THING(p_person->Genus.Person->pcom_ai_arg);
			
			//
			// Near enough to the bike yet?
			//

			dx = abs(p_bike->WorldPos.X - p_person->WorldPos.X >> 8);
			dz = abs(p_bike->WorldPos.Z - p_person->WorldPos.Z >> 8);

			dist = QDIST2(dx,dz);

			if (dist < 0x90)
			{
				//
				// Get onto the bike.
				// 

				set_person_mount_bike(p_person, p_bike);

				//
				// Do our normal thing.
				//

				p_person->Genus.Person->pcom_ai_state    = PCOM_AI_STATE_NORMAL;
				p_person->Genus.Person->pcom_ai_substate = 0;
				p_person->Genus.Person->pcom_ai_counter  = 0;

				p_person->Genus.Person->pcom_move_state    = PCOM_MOVE_STATE_STILL;
				p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_NONE;
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

#endif

//
// Somebody who is talking.
//

void PCOM_process_talk(Thing *p_person)
{
	if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
	{
		//
		// The person stopped talking.
		//

		if (p_person->SubState == SUB_STATE_SIMPLE_ANIM_OVER)
		{
			//
			// He was flagged as NO_RETURN_TO_NORMAL...
			//
		}
		else
		{
			PCOM_set_person_ai_normal(p_person);
		}
	}
}

void	PCOM_process_hands_up(Thing *p_person)
{
	Thing	*p_cop;
	p_cop=TO_THING(p_person->Genus.Person->pcom_ai_arg);

	p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;


	if(p_person->Genus.Person->pcom_ai_counter>PCOM_get_duration(20))
	{
		//
		// after 20 seconds start looking to see if we are safe now.
		//

		if(p_cop->Genus.Person->Target!=THING_NUMBER(p_person) || (p_cop->State!=STATE_GUN && p_cop->SubState!=SUB_STATE_RUNNING))
		{
			//
			// cop is now ignoring me
			//
			PCOM_set_person_ai_normal(p_person);
		}
	}
}


//
// Somebody who wants to get into a vehicle as a passenger.
//

void PCOM_process_hitch(Thing *p_person)
{
	SLONG door;

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_GOTOCAR:

			{
				Thing *p_vehicle = TO_THING(p_person->Genus.Person->pcom_ai_arg);

				ASSERT(p_vehicle->Class == CLASS_VEHICLE);

				//
				// Get into the car if nobody can see you and you've been near to the
				// car for 20 gameturns.
				//

				if ((p_person->Flags & FLAGS_IN_VIEW) || PCOM_get_dist_between(p_person, p_vehicle) > 0x200)
				{
					p_person->Genus.Person->pcom_ai_counter = 0;
				}
				else
				{
					p_person->Genus.Person->pcom_ai_counter += 1;
				}

				extern SLONG in_right_place_for_car(Thing *p_person, Thing *p_vehicle, SLONG *door);

				if (p_person->Genus.Person->pcom_ai_counter > 20 || in_right_place_for_car(p_person, p_vehicle, &door))
				{
					//
					// We can get into a car! Lets do it.
					//

					PCOM_set_person_move_getincar(p_person, p_vehicle, TRUE, door);

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_GETINCAR;
					p_person->Genus.Person->pcom_ai_counter  = 0;
				}
			}

			break;

		case PCOM_AI_SUBSTATE_GETINCAR:

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				//
				// We have got in the car...
				//

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_HITCHING;
			}

			break;

		case PCOM_AI_SUBSTATE_HITCHING:
			
			if (p_person->Genus.Person->pcom_move == PCOM_MOVE_FOLLOW)
			{
				SLONG  i_target = EWAY_get_person(p_person->Genus.Person->pcom_move_follow);
				Thing *p_target = TO_THING(i_target);

				ASSERT(p_target->Class == CLASS_PERSON);

				if (!(p_target->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER)))
				{
					PCOM_set_person_ai_leavecar(p_person, PCOM_EXCAR_NORMAL, 0, 0);

					return;
				}
			}
			else
			{
				//
				// What is the condition for exiting the vehicle?
				//

				ASSERT(0);
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}



//
// Somebody who is knocked out.
//

void PCOM_process_knockedout(Thing *p_person)
{
	if (!(p_person->Genus.Person->Flags & FLAG_PERSON_KO))
	{
		//
		// The person has recovered.
		//

		PCOM_set_person_ai_normal(p_person);
	}
}


void PCOM_process_taunt(Thing *p_person)
{
	Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	//
	// Always face who you are taunting.
	//

	set_face_thing(
		p_person,
		p_target);

//	if ((GAME_TURN & 0x7) == 0)
	if ((PTIME(p_person) & 0x7) == 0)
	{
		if (!can_a_see_b(p_person, p_target))
		{
			//
			// Don't taunt people you can't see!
			//

			PCOM_set_person_ai_normal(p_person);

			return;
		}
	}

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:
			
			if (p_person->Genus.Person->pcom_zone && (PCOM_get_zone_for_position(p_person) & p_person->Genus.Person->pcom_zone))
			{
				//
				// Is are target in our zone? If he is then kill him!
				//

				if (PCOM_get_zone_for_position(p_target) & p_person->Genus.Person->pcom_zone)
				{
					PCOM_set_person_ai_kill_person(p_person, p_target);

					return;
				}
			}
			else
			{
				//
				// Shall we kill him?
				// 

				if (Random() & 0x1)
				{
					PCOM_set_person_ai_kill_person(p_person, p_target);

					return;
				}
			}

			//
			// Pause for a while.
			//

			PCOM_set_person_move_pause(p_person);

			//
			// This is how long we are going to pause for.
			//

			p_person->Genus.Person->pcom_ai_counter = PCOM_get_duration(Random() & 0xf);

			break;

		case PCOM_MOVE_STATE_PAUSE:

			//
			// Finished pausing? 
			//

			if (p_person->Genus.Person->pcom_move_counter > p_person->Genus.Person->pcom_ai_counter)
			{
				if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
				{
					//
					// Lazy people dont taunt.
					//

					PCOM_set_person_move_still(p_person);
				}
				else
				{
					ASSERT(p_person->SubState!=SUB_STATE_GRAPPLEE);
					ASSERT(p_person->SubState!=SUB_STATE_GRAPPLE_HELD);

//					MFX_play_thing(THING_NUMBER(p_person),S_WANKER,MFX_REPLACE,p_person);

					PCOM_set_person_move_animation(p_person, ANIM_WANKER);
				}
			}

			break;

		case PCOM_MOVE_STATE_ANIMATION:

			if (p_person->Genus.Person->pcom_zone)
			{
				//
				// Is are target in our zone? If he is then kill him!
				//

				if (PCOM_get_zone_for_position(p_target) & p_person->Genus.Person->pcom_zone)
				{
					PCOM_set_person_ai_kill_person(p_person, p_target);

					return;
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}


//
// Processes a cop trying to arrest someone!
//

void PCOM_process_arrest(Thing *p_person)
{
	Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	if (p_target->State == STATE_DEAD)
	{
		//
		// No point arresting a dead person!
		//

		PCOM_set_person_ai_normal(p_person);

		return;
	}

	//
	// What if you are navigating to him!
	//

	/*

	//
	// Always face who you are arresting.
	//

	set_face_thing(
		p_person,
		p_target);

	*/


	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:
/*
			if(!is_person_ko(p_person))
			if (!p_target->Genus.Person->PlayerID)
			{
				ASSERT(p_person->SubState!=SUB_STATE_GRAPPLEE);
				ASSERT(p_person->SubState!=SUB_STATE_GRAPPLE_HELD);
				//
				// Make our target call us a wanker! (If they aren't a player!)
				//

				MFX_play_thing(THING_NUMBER(p_target),S_BLOODYPIGS,MFX_REPLACE,p_target);

				PCOM_set_person_ai_taunt(p_target, p_person);

				PCOM_oscillate_tympanum(
					PCOM_SOUND_WANKER,
					p_person,
					p_person->WorldPos.X >> 8,
					p_person->WorldPos.Y >> 8,
					p_person->WorldPos.Z >> 8);
			}
*/

			//
			// Wait for a while.
			// 
	
			PCOM_set_person_move_pause(p_person);

			break;

		case PCOM_MOVE_STATE_PAUSE:
			
			if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(10))
			{
				//
				// Start killing our target.
				//

				PCOM_set_person_ai_kill_person(
					p_person,
					p_target);

				return;
			}

			break;

		case PCOM_MOVE_STATE_GOTO_THING:
			
			//
			// Near enough yet?
			//

			if (PCOM_get_dist_between(p_person, p_target) < 0x200)
			{
				//
				// Stand still.
				// 

				PCOM_set_person_move_still(p_person);
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}


//
// Processes someone going home.
//

void PCOM_process_homesick(Thing *p_person)
{
	if (PCOM_finished_nav(p_person))
	{
		//
		// Do what you normally do at home.
		//

		PCOM_set_person_ai_normal(p_person);
	}
}

void PCOM_process_bdeactivate(Thing *p_person)
{
	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_GOTOBOMB:

			if (PCOM_finished_nav(p_person))
			{
				//
				// Got to the bomb. Start deactivating it now.
				// 

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_CUTWIRES;
				p_person->Genus.Person->pcom_ai_counter  = 0;

				//
				// Don't move while deactivating the bomb- its a tricky and intricate operation!
				//

				PCOM_set_person_move_still(p_person);
			}

			break;

		case PCOM_AI_SUBSTATE_CUTWIRES:

			//
			// Make the person turn- (to show he is doing something)
			//

			p_person->Draw.Tweened->Angle += 32;

			if (p_person->Genus.Person->pcom_ai_counter >= PCOM_get_duration(100))
			{
				//
				// Deactivated the bomb. (Hurrah!)
				//

				Thing *p_bomb = TO_THING(p_person->Genus.Person->pcom_ai_arg);

				p_bomb->SubState = SPECIAL_SUBSTATE_NONE;

				//
				// Do what you normally do now.
				//

				PCOM_set_person_ai_normal(p_person);
			}
			else
			{
				p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

void PCOM_process_leavecar(Thing *p_person)
{
	Thing *p_vehicle;

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_STILL:

			//
			// Finished getting out of the car.
			//

			switch(p_person->Genus.Person->pcom_ai_excar_state)
			{
				case PCOM_EXCAR_NORMAL:
					PCOM_set_person_ai_normal(p_person);
					break;

				case PCOM_EXCAR_FLEE_PERSON:
					PCOM_set_person_ai_flee_person(p_person, TO_THING(p_person->Genus.Person->pcom_ai_excar_arg));
					break;

				case PCOM_EXCAR_ARREST_PERSON:
					PCOM_set_person_ai_arrest(p_person, TO_THING(p_person->Genus.Person->pcom_ai_excar_arg));
					break;

				default:
					ASSERT(0);
					break;
			}

			break;

		case PCOM_MOVE_STATE_PARK_CAR:
		case PCOM_MOVE_STATE_PARK_CAR_ON_ROAD:
			
			ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

			p_vehicle = TO_THING(p_person->Genus.Person->InCar);

			if (p_vehicle->Velocity == 0)
			{
				//
				// We can start getting out the car now.
				//

				PCOM_set_person_move_leavecar(p_person);
			}

			break;

		case PCOM_MOVE_STATE_ANIMATION:
			break;

		default:
			ASSERT(0);
			break;
	}		
}


void PCOM_process_snipe(Thing *p_person)
{
	#ifndef PSX

	//
	// Some friendly debug code.
	// 

	if (p_person->Genus.Person->pcom_ai_arg == NULL)
	{
		//
		// Nobody to kill!
		//
#ifndef	FINAL
		//PANEL_new_text(p_person, 0, "Assassin (stand still) has no target");
#endif

		p_person->Genus.Person->pcom_ai = PCOM_AI_NONE;

		PCOM_set_person_ai_normal(p_person);

		return;
	}

	#endif

	Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_LOOK:

			//
			// Can we see our target?
			//

			p_person->Genus.Person->Target = NULL;

			if (!PCOM_person_has_any_sort_of_gun_with_ammo(p_person))
			{
				//
				// Not much to do without ammo.
				//
			}
			else
			{
				if (can_a_see_b(p_person, p_target))
				{	
					//
					// Take a shot.
					//

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_AIMING;
					p_person->Genus.Person->pcom_ai_counter  = 0;
				}
				else
				{
					p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
					p_person->Genus.Person->Target           = NULL;

/*
//
// Snipers keep your gun out MD
//
					if (p_person->Genus.Person->pcom_ai_counter > PCOM_get_duration(10))
					{
						//
						// No need to have your gun out.
						//

						if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
						{
							//
							// Put your gun away.
							//

							PCOM_set_person_move_gun_away(p_person);

							p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NOMOREAMMO;
						}
					}
*/
				}
			}

			break;

		case PCOM_AI_SUBSTATE_AIMING:

			p_person->Genus.Person->Target = THING_NUMBER(p_target);

			if (!person_has_gun_or_grenade_out(p_person))
			{
				//
				// Draw your gun.
				//

				if (p_person->Genus.Person->pcom_move_state != PCOM_MOVE_STATE_ANIMATION)
				{
					PCOM_set_person_move_draw_gun(p_person);
				}
			}
			else
			{
				if (PCOM_get_dist_between(p_person, p_target) < 0x600 && can_a_see_b(p_person, p_target))
				{
					//
					// How quickly we shoot.
					//

					SLONG shoot_time;
					
					shoot_time  = PCOM_get_duration   (get_rate_of_fire(p_person));
					shoot_time -= PCOM_get_duration100(GET_SKILL(p_person)<<2);

					if (p_person->Genus.Person->pcom_ai_counter > shoot_time || p_person->Genus.Person->pcom_ai == PCOM_AI_SHOOT_DEAD)
					{
						if (p_person->Genus.Person->Ammo == 0)
						{
							//
							// If you have no ammo then all this function does is play
							// a click sound.
							//

							set_person_shoot(p_person,1);

							//
							// Put your gun away.
							//

							PCOM_set_person_move_gun_away(p_person);

							p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_NOMOREAMMO;
						}
						else
						{
							//
							// Take a shot!
							//

							p_person->Genus.Person->Target=THING_NUMBER(p_target);
							PCOM_set_person_move_shoot(p_person);

							p_person->Genus.Person->pcom_ai_counter = 0;
						}
					}
					else
					{
						p_person->Genus.Person->pcom_ai_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
					}

					//
					// Always face the target while sniping.
					//

					set_face_thing(p_person, p_target);
				}
				else
				{
					//
					// Start looking again...
					//

					p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LOOK;
				}
			}

			break;

		case PCOM_AI_SUBSTATE_NOMOREAMMO:

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
			{
				//
				// Finished putting our gun away- get sniping again!
				//

				p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LOOK;
				p_person->Genus.Person->pcom_ai_counter  = 0;
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}


void PCOM_process_warm_hands(Thing *p_person)
{
	SLONG  i_fire;
	Thing *p_fire;

	switch(p_person->Genus.Person->pcom_ai_substate)
	{
		case PCOM_AI_SUBSTATE_GOTOFIRE:
			
			switch(p_person->Genus.Person->pcom_move_state)
			{
				case PCOM_MOVE_STATE_PAUSE:

					if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(10))
					{
						//
						// Look for a nearby fire.
						//

						i_fire = THING_find_nearest(
									p_person->WorldPos.X >> 8,
									p_person->WorldPos.Y >> 8,
									p_person->WorldPos.Z >> 8,
									0x300,
									1 << CLASS_PYRO);

						if (i_fire)
						{
							//
							// Start going to this bit of fire.
							//

							p_person->Genus.Person->pcom_ai_arg = i_fire;

							PCOM_set_person_move_mav_to_thing(p_person, TO_THING(i_fire), PCOM_MOVE_SPEED_WALK);
						}
						else
						{
							//
							// Carry on waiting for some nice warm fire.
							//

							PCOM_set_person_move_pause(p_person);
						}
					}

					break;

				case PCOM_MOVE_STATE_GOTO_THING:

					p_fire = TO_THING(p_person->Genus.Person->pcom_ai_arg);

					if (PCOM_get_dist_between(p_person, p_fire) < 0xa0)
					{
						//
						// Don't get too close! Start warming our hands about the fire.
						//

						p_person->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_WARMUP;
						set_face_thing(p_person, p_fire);

						PCOM_set_person_move_still(p_person);
					}

					break;
			}

			break;

		case PCOM_AI_SUBSTATE_WARMUP:

			p_fire = TO_THING(p_person->Genus.Person->pcom_ai_arg);

			if (p_fire->Class != CLASS_PYRO)
			{
				//
				// This fire must have gone out- better get looking for another one.
				//

				PCOM_set_person_ai_warm_hands(p_person);
			}
			else
			{
				//
				// Every now and again play a warm-your-hands animation.
				//
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}


SLONG person_drawn_recently(Thing *p_person)
{
	return p_person->Flags & FLAGS_IN_VIEW;
}

//
// Processes normal behaviour
//

void	PCOM_teleport_home(Thing *p_person)
{
	GameCoord	pos;

	pos.X=p_person->Genus.Person->HomeX<<8;
	pos.Y=p_person->WorldPos.Y;
	pos.Z=p_person->Genus.Person->HomeZ<<8;

	move_thing_on_map(p_person,&pos);

	p_person->Draw.Tweened->Angle = p_person->Genus.Person->HomeYaw << 3;

}

void PCOM_process_normal(Thing *p_person)
{
	UWORD  i_target;
	Thing *p_target;
	SLONG	dist;

	switch(p_person->Genus.Person->pcom_move)
	{
		case PCOM_MOVE_STILL:
		case PCOM_MOVE_DANCE:
		case PCOM_MOVE_HANDS_UP:
		case PCOM_MOVE_TIED_UP:

			if (p_person->State == STATE_MOVEING && (p_person->SubState == SUB_STATE_SIMPLE_ANIM || p_person->SubState == SUB_STATE_SIMPLE_ANIM_OVER))
			{
				//
				// This person is busy sitting down or dancing!
				//

				if (p_person->Genus.Person->pcom_move == PCOM_MOVE_STILL)
				{
					//
					// Still people shouldn't be doing an animation... unless they
					// are sitting down.
					//

					if (p_person->Draw.Tweened->CurrentAnim == ANIM_SIT_DOWN   ||
						p_person->Draw.Tweened->CurrentAnim == ANIM_SIT_IDLE ||
						p_person->Draw.Tweened->CurrentAnim == ANIM_IDLE_SCRATCH2)
					{
						//
						// Stay sitting down or wiping your brow.
						//
					}
					else
					{
						if ((PTIME(p_person) & 0x3) == 0)
//						if (((THING_NUMBER(p_person) + GAME_TURN) & 0x3) == 0)
						{
							//
							// Wipe your brow...
							//

							PCOM_set_person_move_animation(p_person, ANIM_IDLE_SCRATCH2);
						}
						else
						{
							//
							// Stand still without doing a silly animation.
							//

							PCOM_set_person_move_still(p_person);
						}
					}						
				}
			}
			else
			{
				//
				// Still people constantly check their distance from home
				//

				dist = PCOM_get_dist_from_home(p_person);

				if (dist > 256)
				{
					//
					// Go back home before standing around.
					//

					PCOM_set_person_ai_homesick(p_person);
				}
				else
				{
					//
					// Make sure they are facing in the right direction.
					//

					p_person->Draw.Tweened->Angle = p_person->Genus.Person->HomeYaw << 3;

					if (dist > PCOM_ARRIVE_DIST)
					{
						if (!person_drawn_recently(p_person))
						{
							PCOM_teleport_home(p_person);
						}
					}
					else
					{
						//
						// This person is at home. Does he want to do a special animation?
						//

						if (p_person->Genus.Person->pcom_move == PCOM_MOVE_DANCE    ||
							p_person->Genus.Person->pcom_move == PCOM_MOVE_HANDS_UP ||
							p_person->Genus.Person->pcom_move == PCOM_MOVE_TIED_UP)
						{
							UWORD anim;

							if (p_person->Genus.Person->pcom_move == PCOM_MOVE_DANCE)
							{
								//
								// Start dancing!
								//

								static UWORD dance_anim[4] =
								{
									ANIM_DANCE_BOOGIE,
									ANIM_DANCE_WOOGIE,
									ANIM_DANCE_HEADBANG,
									ANIM_DANCE_BOOGIE
								};

								anim = dance_anim[THING_NUMBER(p_person) & 0x3];
							}
							else
							{
								//
								// Tied up and hands up both do hands up...
								//

								anim = ANIM_HANDS_UP;
							}

							PCOM_set_person_move_animation(p_person, anim);

							//
							// This flag means that after the person has done the simple anim once, they
							// won't go to STATE_IDLE but will carry on animating in SUBSTATE SUB_STATE_SIMPLE_ANIM_OVER
							//

							p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
						}
						else
						if (p_person->Genus.Person->pcom_bent & PCOM_BENT_LAZY)
						{
							if ((p_person->SubState == SUB_STATE_SIMPLE_ANIM) ||
								(p_person->SubState == SUB_STATE_SIMPLE_ANIM_OVER))
							{
								// already sitting down ...

								break;
							}

//							if ((GAME_TURN & 0x3f) == 0)
							if ((PTIME(p_person) & 0x3f) == 0)
							{
								//
								// Look for a nearby bench or sofa thats not too far away.
								//

								#define PCOM_MAX_BENCH_WALK 0x200

								SLONG mx;
								SLONG mz;
								SLONG mx1;
								SLONG mz1;
								SLONG mx2;
								SLONG mz2;
								SLONG dx;
								SLONG dy;
								SLONG dz;
								SLONG dist;
								SLONG best_x;
								SLONG best_y;
								SLONG best_z;
								SLONG best_yaw;
								SLONG best_prim = NULL;
								SLONG best_dist = PCOM_MAX_BENCH_WALK;

								OB_Info *oi;

								mx1 = (p_person->WorldPos.X >> 8) - PCOM_MAX_BENCH_WALK >> PAP_SHIFT_LO;
								mz1 = (p_person->WorldPos.Z >> 8) - PCOM_MAX_BENCH_WALK >> PAP_SHIFT_LO;

								mx2 = (p_person->WorldPos.X >> 8) + PCOM_MAX_BENCH_WALK >> PAP_SHIFT_LO;
								mz2 = (p_person->WorldPos.Z >> 8) + PCOM_MAX_BENCH_WALK >> PAP_SHIFT_LO;

								for (mx = mx1; mx <= mx2; mx++)
								for (mz = mz1; mz <= mz2; mz++)
								{
									for (oi = OB_find(mx,mz); oi->prim; oi++)
									{
										if (oi->prim == PRIM_OBJ_PARK_BENCH ||
											oi->prim == PRIM_OBJ_SOFA       ||
											oi->prim == PRIM_OBJ_ARMCHAIR)
										{
											dx = oi->x - (p_person->WorldPos.X >> 8);
											dy = oi->y - (p_person->WorldPos.Y >> 8);
											dz = oi->z - (p_person->WorldPos.Z >> 8);

											dist = abs(dx) + abs(dz) + abs(dy + dy);

											if (best_dist > dist)
											{
												best_prim = oi->prim;
												best_dist = dist;
												best_x    = oi->x;
												best_y    = oi->y;
												best_z    = oi->z;
												best_yaw  = oi->yaw;
											}
										}
									}
								}

								if (best_prim)
								{
									//
									// This person want to sit on the prim! Teleport him to the right place.
									//

									if (PCOM_position_person_to_sit_on_prim(
											p_person,
											best_prim,
											best_x,
											best_y,
											best_z,
											best_yaw,
											person_drawn_recently(p_person)))
									{
										//
										// Sit him down.
										//

										PCOM_set_person_move_animation(p_person, ANIM_SIT_DOWN);

										//
										// This flag means that after the person has done the simple anim once, they
										// won't go to STATE_IDLE but will carry on animating in SUBSTATE SUB_STATE_SIMPLE_ANIM_OVER
										//

										p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
									}
								}
							}
						}
						else
						if (p_person->Genus.Person->pcom_ai == PCOM_AI_GUARD)
						{
							//
							// Make guards draw their gun! (As long as they haven't got a pistol)
							//

							if (PCOM_person_has_any_sort_of_gun(p_person) && !(p_person->Flags & FLAGS_HAS_GUN))
							{
								if (!PCOM_person_has_gun_in_hand(p_person))
								{
									if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_STILL)
									{
										//
										// Draw your gun.
										//

										PCOM_set_person_move_draw_gun(p_person);
									}
								}
							}
						}
					}
				}
			}

			break;

		case PCOM_MOVE_PATROL:
		case PCOM_MOVE_PATROL_RAND:
			PCOM_process_patrol(p_person);
			break;

		case PCOM_MOVE_WANDER:
			PCOM_process_wander(p_person);
			break;

		case PCOM_MOVE_FOLLOW:

			if (p_person->Genus.Person->pcom_ai_substate == PCOM_AI_SUBSTATE_CANTFIND)
			{
				if ((GAME_TURN & 0xf) == 0)
//				if ((PTIME(p_person) & 0xf) == 0)
				{
					i_target = EWAY_get_person(p_person->Genus.Person->pcom_move_follow);

					if (i_target)
					{
						if (can_a_see_b(p_person, TO_THING(i_target)))
						{
							PCOM_set_person_ai_follow(p_person, TO_THING(i_target));
						}
					}
					
				}
				else
				{
					PCOM_process_wander(p_person);
				}
			}
			else
			{
				i_target = EWAY_get_person(p_person->Genus.Person->pcom_move_follow);

				if (i_target)
				{
					PCOM_set_person_ai_follow(p_person, TO_THING(i_target));
				}
			}

			break;

		case PCOM_MOVE_WARM_HANDS:
			
			if (PCOM_get_dist_from_home(p_person) > 0x200)
			{
				//
				// Go back home before standing around- to make sure we find the
				// correct barrel to warm our hands over.
				//

				PCOM_set_person_ai_homesick(p_person);
			}
			else
			{
				PCOM_set_person_ai_warm_hands(p_person);
			}

			break;

		case PCOM_MOVE_FOLLOW_ON_SEE:

			//
			// If this person is near to who he is following, then change
			// state to PCOM_MOVE_FOLLOW.
			//

			i_target = EWAY_get_person(p_person->Genus.Person->pcom_move_follow);

			if (i_target)
			{
				if (PCOM_get_dist_between(p_person, TO_THING(i_target)) < 0x200)
				{
					if (can_a_see_b(p_person, TO_THING(i_target)))
					{
						//
						// Start following this person.
						//

						p_person->Genus.Person->pcom_move = PCOM_MOVE_FOLLOW;
					}
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Looks for an active bomb that the person can see.
//

UWORD PCOM_find_bomb(Thing *p_person)
{
	SLONG i;
	SLONG score;

	SLONG best_thing;
	SLONG best_score;

	Thing *p_found;

	PCOM_found_num = THING_find_sphere(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						0x300,
						PCOM_found,
						PCOM_MAX_FIND,
						1 << CLASS_SPECIAL);

	best_score = +INFINITY;
	best_thing =  NULL;

	for (i = 0; i < PCOM_found_num ; i++)
	{
		p_found = TO_THING(PCOM_found[i]);

		if (p_found->Genus.Special->SpecialType == SPECIAL_BOMB &&
			p_found->SubState                   == SPECIAL_SUBSTATE_ACTIVATED)
		{
			if (can_a_see_b(p_person, p_found))
			{
				score = THING_dist_between(p_person, p_found);

				if (score < best_score)
				{
					best_score = score;
					best_thing = PCOM_found[i];
				}
			}
		}
	}

	return best_thing;
}





//
// Does the default processing for a person.
//

void PCOM_process_default(Thing *p_person)
{
	switch(p_person->Genus.Person->pcom_ai_state)
	{
		case PCOM_AI_STATE_NORMAL:
			PCOM_process_normal(p_person);
			break;

		case PCOM_AI_STATE_INVESTIGATING:
			PCOM_process_investigating(p_person);
			break;

		case PCOM_AI_STATE_SEARCHING:
			break;

		case PCOM_AI_STATE_KILLING:
			PCOM_process_killing(p_person);
			break;

		case PCOM_AI_STATE_SLEEPING:
			break;

		case PCOM_AI_STATE_FLEE_PLACE:
		case PCOM_AI_STATE_FLEE_PERSON:
			PCOM_process_fleeing(p_person);
			break;

		case PCOM_AI_STATE_FOLLOWING:
			PCOM_process_following(p_person);
			break;

		case PCOM_AI_STATE_NAVTOKILL:
			PCOM_process_navtokill(p_person);
			break;

		case PCOM_AI_STATE_HOMESICK:
			PCOM_process_homesick(p_person);
			break;

		case PCOM_AI_STATE_LOOKAROUND:
			break;

		case PCOM_AI_STATE_FINDCAR:
			PCOM_process_findcar(p_person);
			break;

		case PCOM_AI_STATE_BDEACTIVATE:
			PCOM_process_bdeactivate(p_person);
			break;

		case PCOM_AI_STATE_LEAVECAR:
			PCOM_process_leavecar(p_person);
			break;

		case PCOM_AI_STATE_SNIPE:
			PCOM_process_snipe(p_person);
			break;

		case PCOM_AI_STATE_WARM_HANDS:
			PCOM_process_warm_hands(p_person);
			break;

		#ifdef BIKE
		case PCOM_AI_STATE_FINDBIKE:
			PCOM_process_findbike(p_person);
			break;
		#endif

		case PCOM_AI_STATE_KNOCKEDOUT:
			PCOM_process_knockedout(p_person);
			break;

		case PCOM_AI_STATE_TAUNT:
			PCOM_process_taunt(p_person);
			break;

		case PCOM_AI_STATE_ARREST:
			PCOM_process_arrest(p_person);
			break;

		case PCOM_AI_STATE_TALK:
			PCOM_process_talk(p_person);
			break;

		case PCOM_AI_STATE_HITCH:
			PCOM_process_hitch(p_person);
			break;

		case PCOM_AI_STATE_AIMLESS:
			PCOM_process_wander(p_person);
			break;

		case PCOM_AI_STATE_HANDS_UP:
			PCOM_process_hands_up(p_person);
			break;

		case PCOM_AI_STATE_GETITEM:
			PCOM_process_getitem(p_person);
			break;

		default:
			ASSERT(0);
			break;
	}
}

//
// Alerts all nearby MIB / Guards / Gangs to attack / fight test... including the person himself.
//

void PCOM_alert_nearby_mib_to_attack(Thing *p_person)
{
	{
		SLONG  i;
		SLONG  num_found;
		Thing *p_found;

		//
		// Alerts all nearby MIB to attack- including yourself!
		//

		num_found = THING_find_sphere(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						0x500,
						THING_array,
						THING_ARRAY_SIZE,
						1 << CLASS_PERSON);
		
		for (i = 0; i < num_found; i++)
		{
			p_found = TO_THING(THING_array[i]);

			if (p_found->Genus.Person->pcom_ai == PCOM_AI_MIB   ||
				p_found->Genus.Person->pcom_ai == PCOM_AI_GUARD ||
				p_found->Genus.Person->pcom_ai == PCOM_AI_GANG  ||
				p_found->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
			{
				if (!(p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS))
				{
					if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL   ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_INVESTIGATING)
					{
						PCOM_set_person_ai_navtokill(p_found, NET_PERSON(0));
					}
				}
			}
		}
	}
}


//
// Returns who a bodyguard should be attacking...
//

Thing *PCOM_find_bodyguard_victim(Thing *p_bodyguard, Thing *p_client)
{
	SLONG  i;
	SLONG  dx;
	SLONG  dy;
	SLONG  dz;
	SLONG  dist;
	SLONG  num_found;
	SLONG  best_score  = INFINITY;
	Thing *best_victim = NULL;
	Thing *p_found;

	num_found = THING_find_sphere(
					p_bodyguard->WorldPos.X >> 8,
					p_bodyguard->WorldPos.Y >> 8,
					p_bodyguard->WorldPos.Z >> 8,
					0x800,
					THING_array,
					THING_ARRAY_SIZE,
					1 << CLASS_PERSON);
	
	for (i = 0; i < num_found; i++)
	{
		p_found = TO_THING(THING_array[i]);

		if (is_person_dead(p_found))
		{
			continue;
		}

		if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING   ||
			p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL)
		{
			if (p_found->Genus.Person->pcom_ai_arg == THING_NUMBER(p_client) ||
				p_found->Genus.Person->pcom_ai_arg == THING_NUMBER(p_bodyguard))
			{
				//
				// This person is attacking me or my client. 
				//

				dx = p_found->WorldPos.X - p_bodyguard->WorldPos.X;
				dy = p_found->WorldPos.Y - p_bodyguard->WorldPos.Y;
				dz = p_found->WorldPos.Z - p_bodyguard->WorldPos.Z;

				dist = abs(dx) + abs(dz) + abs(dy << 1);

				if (p_client->Genus.Person->Target == THING_array[i])
				{
					//
					// If our client is already having a go at one of them be more
					// likely to attack one of the other ones.
					//

					dist <<= 2;
				}

				if (is_person_ko(p_found))
				{
					//
					// Knocked out people aren't much of a threat!
					//

					dist <<= 1;
				}

				if (dist < best_score)
				{
					best_score  = dist;
					best_victim = p_found;
				}
			}
		}
	}

	return best_victim;
}



void PCOM_process_state_change(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG home_x;
	SLONG home_z;

	SLONG bomb;

	Thing *p_target;
	SLONG  i_target;

	//
	// Nobody has their gun out by default...
	//

	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
	{
		if ((p_person->Flags & FLAGS_HAS_GUN) && (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT))
		{
			if (p_person->Genus.Person->pcom_move_state    != PCOM_MOVE_STATE_ANIMATION ||
				p_person->Genus.Person->pcom_move_substate != PCOM_MOVE_SUBSTATE_GUNAWAY)
			{
				PCOM_set_person_move_gun_away(p_person);
			}

			return;
		}
	}

	if ((p_person->Genus.Person->Flags          & FLAG_PERSON_DRIVING) &&
		 p_person->Genus.Person->pcom_ai       != PCOM_AI_DRIVER       &&
		 p_person->Genus.Person->pcom_ai       != PCOM_AI_COP_DRIVER   &&
		 p_person->Genus.Person->pcom_ai_state != PCOM_AI_STATE_LEAVECAR)
	{
		//
		// A non-driver doesn't want to be driving a car.
		//

		PCOM_set_person_ai_leavecar(p_person, PCOM_EXCAR_NORMAL, 0, 0);
	}

	switch(p_person->Genus.Person->pcom_ai)
	{
		case PCOM_AI_NONE:
			break;

		case PCOM_AI_CIV:

			PCOM_process_default(p_person);
/*
			if (((GAME_TURN + THING_NUMBER(p_person)) & 0xff) == 0)
			{
				if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL   ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS)
				{
					//
					// Civs remember nasty people.
					//

					if (p_person->Genus.Person->pcom_ai_memory && !EWAY_stop_player_moving())
					{
						Thing *p_nasty = TO_THING(p_person->Genus.Person->pcom_ai_memory);

						if (PCOM_get_dist_between(
								p_person,
								p_nasty) < 0x120)
						{
							PCOM_set_person_ai_talk_to(
								p_person,
								p_nasty,
								PCOM_AI_SUBSTATE_TALK_TELL,
								FALSE);

							if (p_nasty->Genus.Person->PlayerID && p_nasty->Genus.Person->PersonType == PERSON_DARCI)
							{

								PANEL_new_text(p_person, 4000, EWAY_get_fake_wander_message(EWAY_FAKE_MESSAGE_ANNOYED));
							}
						}
					}
				}
			}
*/

			break;

		case PCOM_AI_GUARD:

			switch(p_person->Genus.Person->pcom_ai_state)
			{
				case PCOM_AI_STATE_NAVTOKILL:

					{
						Thing *p_target = TO_THING(p_person->Genus.Person->pcom_ai_arg);

						if (PCOM_get_dist_between(p_person, p_target) > 20 * 0x100)
						{
							//
							// Too far from our target... just give up!
							//

							PCOM_set_person_ai_normal(p_person);

							return;
						}
					}

					if (p_person->Genus.Person->pcom_zone)
					{
						//
						// Just do normal processing for navtokill. The zone code will stop
						// us going too far away from where we are meant to be.
						//

						PCOM_process_navtokill(p_person);
					}
					else
					if (p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
					{
						//
						// Wandering people don't check how far they are from home.
						//

						PCOM_process_navtokill(p_person);
					}
					else
					{
						//
						// If the guard is too far from home, then return to guard whatever
						// he's guarding.
						//

						home_x = p_person->Genus.Person->HomeX; //<< 8;
						home_z = p_person->Genus.Person->HomeZ; //<< 8;

						dist = PCOM_person_dist_from(
									p_person,
									home_x,
									home_z);

						if (dist > 16 * 0x100)	// If more than sixteen mapsquares away
						{
							//
							// Start going home.
							//

							PCOM_set_person_ai_homesick(p_person);
						}
						else
						{
							//
							// Just do normal processing for navtokill.
							//

							PCOM_process_navtokill(p_person);
						}
					}

					break;

				default:
					PCOM_process_default(p_person);
					break;
			}

//			if ((GAME_TURN & 0x3) == 0)
			if ((PTIME(p_person) & 0x3) == 0)
			{
				SLONG look = FALSE;

				if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL     ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK)
				{
					look = TRUE;
				}

				if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_INVESTIGATING)
				{
					if (p_person->Genus.Person->pcom_ai_substate == PCOM_AI_SUBSTATE_SUPRISED)
					{
						look = (p_person->Genus.Person->pcom_ai_counter >= PCOM_get_duration(4));
					}
					else
					{
						look = TRUE;
					}
				}

				if (look)
				{
					//
					// Can you see the player?
					//

					p_target = PCOM_can_i_see_person_to_attack(p_person);

					if (p_target)
					{
						//
						// Tell nearby people to attack the player too!
						//

						PCOM_alert_nearby_mib_to_attack(p_person);
						if(p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
						{
							// cant navigate to enemy so taunt

							PCOM_set_person_ai_taunt(p_person, p_target);
						}
//						else //play wav anyway MikeD
						{

#ifndef PSX
/*
							if(((GAME_TURN+THING_NUMBER(p_person))&255)==0)
								MFX_play_thing(THING_NUMBER(p_person),S_HEY_YOU,MFX_REPLACE,p_person);
*/
#endif
						}
						PCOM_oscillate_tympanum(
							PCOM_SOUND_HEY,
							p_person,
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8);


						//
						// Put my health on screen in an available slot
						//

						//track_enemy(p_person);
					}
				}
			}

			break;

		case PCOM_AI_ASSASIN:
		case PCOM_AI_SHOOT_DEAD:

			if (p_person->Genus.Person->pcom_ai_other == NULL)
			{
				#ifndef NDEBUG

				CONSOLE_text("Assasin has no target");

				#endif
			}
			else
			{
				if (p_person->Genus.Person->pcom_ai_other == NULL)
				{
					#ifndef NDEBUG

					CONSOLE_text("Assasin waypoint has no person associated with it");

					#endif
				}
				else
				{

					i_target = EWAY_get_person(p_person->Genus.Person->pcom_ai_other);

					if (i_target)
					{
						p_target = TO_THING(i_target);

						if (is_person_dead(p_target))
						{
							//
							// Assasins change once they've killed their targets.
							//

							p_person->Genus.Person->pcom_ai    = PCOM_AI_GANG;
							p_person->Genus.Person->pcom_move  = PCOM_MOVE_WANDER;

							PCOM_set_person_ai_normal(p_person);

							return;
						}

						switch(p_person->Genus.Person->pcom_ai_state)
						{
							case PCOM_AI_STATE_NORMAL:
								
								switch(p_person->Genus.Person->pcom_move)
								{
									case PCOM_MOVE_STILL:
										PCOM_set_person_ai_snipe(p_person, p_target);
										break;

									case PCOM_MOVE_PATROL:
									case PCOM_MOVE_PATROL_RAND:
									case PCOM_MOVE_WANDER:
										#ifndef NDEBUG
										CONSOLE_text("An assasin on patrol or wander acts like a sniper");
										#endif
										p_person->Genus.Person->pcom_move = PCOM_MOVE_STILL;
										break;

									case PCOM_MOVE_FOLLOW:
										PCOM_set_person_ai_navtokill(p_person, p_target);
										break;
								}

								break;

							default:
								PCOM_process_default(p_person);
								break;
						}
					}
					else
					{
						//
						// This assasin has no target yet...
						//

						PCOM_process_default(p_person);
					}
				}
			}

			break;

		case PCOM_AI_BOSS:
			PCOM_process_default(p_person);
			break;

		case PCOM_AI_COP:
			PCOM_process_default(p_person);
			break;

		case PCOM_AI_GANG:

			PCOM_process_default(p_person);

			{
				SLONG look = FALSE;

				if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_INVESTIGATING)
				{
					if (p_person->Genus.Person->pcom_ai_substate == PCOM_AI_SUBSTATE_SUPRISED)
					{
						look = (p_person->Genus.Person->pcom_ai_counter >= PCOM_get_duration(4));
					}
					else
					{
						look = TRUE;
					}
				}

				if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL        ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS    ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK)
				{
					look = TRUE;
				}

				if (look)
				{
					ASSERT(p_person->SubState!=SUB_STATE_GRAPPLEE);
					ASSERT(p_person->SubState!=SUB_STATE_GRAPPLE_HELD);

					if ((PTIME(p_person) & 0x7) == 0)
//					if ((GAME_TURN & 0x7) == 0)
					{
						//
						// Can I see somebody worth taunting?
						//

						p_target = PCOM_can_i_see_person_to_taunt(p_person);

						if (p_target)
						{
							//
							// Start taunting the person. If the person makes any sort of
							// come on. Then the taunt AI will start killing the person.
							//

							PCOM_set_person_ai_taunt(p_person, p_target);

							PCOM_oscillate_tympanum(
								PCOM_SOUND_WANKER,
								p_person,
								p_person->WorldPos.X >> 8,
								p_person->WorldPos.Y >> 8,
								p_person->WorldPos.Z >> 8);

							return;
						}
					}
				}
			}

			break;

		case PCOM_AI_DOORMAN:
			PCOM_process_default(p_person);
			break;

		case PCOM_AI_BODYGUARD:

			{			
				UWORD  i_client = EWAY_get_person(p_person->Genus.Person->pcom_ai_other);
				Thing *p_client = NULL;

				if (i_client)
				{
					p_client = TO_THING(i_client);
				}

				p_person->Genus.Person->Flags2 &= ~FLAG2_PERSON_INVULNERABLE;

				if (p_client && p_client->Genus.Person->PlayerID)
				{
					SLONG dx = abs(p_client->WorldPos.X - p_person->WorldPos.X);
					SLONG dy = abs(p_client->WorldPos.Y - p_person->WorldPos.Y);
					SLONG dz = abs(p_client->WorldPos.Z - p_person->WorldPos.Z);

					SLONG dist = QDIST3(dx,dy,dz);

					if (dist > 20 * 0x10000)
					{
						//
						// More than twenty blocks from the player! Turn invulnerable.
						//

						p_person->Genus.Person->Flags2 |= FLAG2_PERSON_INVULNERABLE;
					}
				}

				switch(p_person->Genus.Person->pcom_ai_state)
				{
					case PCOM_AI_STATE_NORMAL:
					case PCOM_AI_STATE_FOLLOWING:

						{
							//
							// Is our client under attack?
							//

							if (p_client)
							{
								Thing *p_target = PCOM_find_bodyguard_victim(p_person, p_client);

								if (p_target)
								{
									//
									// What's you upto mate!
									//

	//								MFX_play_thing(THING_NUMBER(p_person),S_OIGUVNOR,MFX_REPLACE,p_person);

									//
									// Kill the person who is trying to kill our client.
									//

									PCOM_set_person_ai_navtokill(p_person, p_target);
								}
							}
						}

						//
						// Fall-through to default processing.
						//

					default:
						PCOM_process_default(p_person);
						break;
				}
			}

			break;

		case PCOM_AI_COP_DRIVER:

			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
			{
/*
				if ((GAME_TURN & 0x7) == 0)
				{
					p_target = PCOM_can_i_see_person_to_arrest(p_person);

					if (p_target)
					{
						//
						// Make the target wanted by the police.
						//

						p_target->Genus.Person->Flags |= FLAG_PERSON_FELON;

						//
						// Exit the car and once you've got out, arrest the person.
						//

						PCOM_set_person_ai_leavecar(
							p_person,
							PCOM_EXCAR_ARREST_PERSON,
							0,
							THING_NUMBER(p_target));
					}
				}
*/
			}

			//
			// FALLTHROUGH to driving code
			// 

		case PCOM_AI_DRIVER:
			
			switch(p_person->Genus.Person->pcom_ai_state)
			{
				case PCOM_AI_STATE_NORMAL:

					//
					// Are we in are car?
					//

					if (!(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING))
					{	
						//
						// Find any old car and get into it.
						//

						PCOM_set_person_ai_findcar(p_person, NULL);
					}
					else
					{
						//
						// Start driving around.
						//

						switch(p_person->Genus.Person->pcom_move)
						{
							case PCOM_MOVE_STILL:
								PCOM_process_driving_still(p_person);
								break;

							case PCOM_MOVE_PATROL:
							case PCOM_MOVE_PATROL_RAND:
								PCOM_process_driving_patrol(p_person);
								break;

							case PCOM_MOVE_FOLLOW:
								break;

							case PCOM_MOVE_WANDER:
								PCOM_process_driving_wander(p_person);
								break;

							default:
								ASSERT(0);
								break;
						}
					}

					break;

				default:
					PCOM_process_default(p_person);
					break;
			}

			break;

		case PCOM_AI_BDISPOSER:

			switch(p_person->Genus.Person->pcom_ai_state)
			{
				case PCOM_AI_STATE_NORMAL:

					PCOM_process_normal(p_person);

					//
					// If he can see a bomb- then go over to it and deactivate it!
					//

					bomb = PCOM_find_bomb(p_person);

					if (bomb)
					{
						PCOM_set_person_ai_bdeactivate(p_person, TO_THING(bomb));
					}

					break;

				case PCOM_AI_STATE_FOLLOWING:
					
					PCOM_process_following(p_person);

					//
					// If he can see a bomb- then go over to it and deactivate it!
					//

					bomb = PCOM_find_bomb(p_person);

					if (bomb)
					{
						PCOM_set_person_ai_bdeactivate(p_person, TO_THING(bomb));
					}

					break;

				default:
					PCOM_process_default(p_person);
					break;
			}

			break;

		case PCOM_AI_BIKER:
			
			switch(p_person->Genus.Person->pcom_ai_state)
			{
				case PCOM_AI_STATE_NORMAL:

					#ifdef BIKE

					//
					// Are we on a bike?
					//

					if (!(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING))
					{	
						//
						// Find a bike and get onto it.
						//

						PCOM_set_person_ai_findbike(p_person);
					}
					else

					#endif
					{
						//
						// Start driving around.
						//

						switch(p_person->Genus.Person->pcom_move)
						{
							case PCOM_MOVE_STILL:
								PCOM_process_driving_still(p_person);
								break;

							case PCOM_MOVE_PATROL:
							case PCOM_MOVE_PATROL_RAND:
								PCOM_process_driving_patrol(p_person);
								break;

							case PCOM_MOVE_FOLLOW:
								break;

							case PCOM_MOVE_WANDER:
								PCOM_process_driving_wander(p_person);
								break;

							default:
								ASSERT(0);
								break;
						}
					}

					break;

				default:
					PCOM_process_default(p_person);
					break;
			}

			break;

		case PCOM_AI_FIGHT_TEST:

			if (p_person->Genus.Person->pcom_ai_other & PCOM_COMBAT_COMBO_KKK)
			{
				combo_display=2;
			}
			else
			if (p_person->Genus.Person->pcom_ai_other & PCOM_COMBAT_COMBO_PPP)
			{
				combo_display=1;
			}

			PCOM_process_default(p_person);

			if (!(p_person->Genus.Person->pcom_bent & PCOM_BENT_ROBOT))
			{
				if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL    ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK  ||
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS)
				{
					if (PCOM_get_dist_between(p_person, NET_PERSON(0)) < 0x200)
					{
						PCOM_alert_nearby_mib_to_attack(p_person);	// Not just MIBs!
					}
				}
			}

			break;

		case PCOM_AI_BULLY:

//			if(Keys[KB_B])
//				FC_cam[0].focus	= p_person;

			PCOM_process_default(p_person);

			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL    ||
				p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING ||
				p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK  ||
				p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS)
			{
				if ((PTIME(p_person) & 0x7) == 0)
//				if ((GAME_TURN & 0x7) == 0)
				{
					//
					// Can I see somebody worth bullying?
					//

					p_target = PCOM_can_i_see_person_to_bully(p_person);

					if (p_target)
					{
						//
						// Start killing the person.
						//

						PCOM_set_person_ai_kill_person(p_person,  p_target);

						return;
					}
				}
			}

			break;

		case PCOM_AI_SUICIDE:

			//
			// The easiest AI! A random death.
			// 

			p_person->Genus.Person->Health = 0;

			set_person_dead(
				p_person,
				NULL,
				PERSON_DEATH_TYPE_SHOT_PISTOL,//was combat
				Random() & 0x1,
				Random() % 3);

			if (GAME_TURN < 32)
			{
				SLONG	c0;
				for(c0=0;c0<100;c0++)
				{
					if(p_person->StateFn)
						p_person->StateFn(p_person);

				}
			}

//			while(person_normal_animate(p_person)==0);
	
			break;

		case PCOM_AI_FLEE_PLAYER:

			switch(p_person->Genus.Person->pcom_ai_state)
			{	
				case PCOM_AI_STATE_NORMAL:
				case PCOM_AI_STATE_HOMESICK:

					//
					// Is this person too near Darci?
					//

					if (PCOM_get_dist_between(p_person, NET_PERSON(0)) < 0x600)
					{
						PCOM_set_person_ai_flee_person(p_person, NET_PERSON(0));
					}

					//
					// FALL-THROUGH TO DEFAULT PROCESSING!
					//

				default:
					PCOM_process_default(p_person);
					break;
			}

			break;

		case PCOM_AI_KILL_COLOUR:

			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
			{
				//
				// Look for someone of the given colour to kill!
				//

				UWORD list;

				SLONG  dist;
				SLONG  best_dist   = INFINITY;
				Thing *best_target = NULL;
				Thing *p_found;

				list = thing_class_head[CLASS_PERSON];

				SLONG hate_colour;
				SLONG hate_example;

				hate_example = EWAY_get_person(p_person->Genus.Person->pcom_ai_other);

				if (hate_example)
				{
					Thing *p_example = TO_THING(hate_example);

					ASSERT(p_example->Class == CLASS_PERSON);

					hate_colour = p_example->Genus.Person->pcom_colour;

					while(list)
					{
						p_found = TO_THING(list);

						list = p_found->NextLink;

						if (p_found->Genus.Person->pcom_colour == hate_colour && !is_person_dead(p_found) && !(p_found->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER))
						{
							if (p_person->Genus.Person->pcom_zone)
							{
								//
								// Ignore people that aren't in your zone.
								//

								if (!(PCOM_get_zone_for_position(p_found->WorldPos.X >> 8, p_found->WorldPos.Z >> 8) & p_person->Genus.Person->pcom_zone))
								{	
									continue;
								}
							}

							//
							// He hates this person!
							//

							dist = PCOM_get_dist_between(p_person, p_found);

							if (best_dist > dist)
							{
								best_dist   = dist;
								best_target = p_found;
							}
						}
					}

					if (best_target)
					{
						PCOM_set_person_ai_kill_person(p_person, best_target);
					}

					/*

					else
					{
						//
						// Nothing more for this person to do!
						//

						if (p_person->Genus.Person->PersonType == PERSON_COP)
						{
							p_person->Genus.Person->pcom_ai = PCOM_AI_COP;
						}
						else
						{
							p_person->Genus.Person->pcom_ai    = PCOM_AI_CIV;
							p_person->Genus.Person->pcom_bent |= PCOM_BENT_FIGHT_BACK;
						}
					}

					*/
				}
			}

			PCOM_process_default(p_person);

			break;

		case PCOM_AI_MIB:

			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
			{
				//
				// Kills the player on sight.
				//

				if (can_a_see_b(p_person, NET_PERSON(0))&&!stealth_debug)
				{
					PCOM_alert_nearby_mib_to_attack(p_person);
				}
			}

			PCOM_process_default(p_person);

			break;

		case PCOM_AI_BANE:
			
			if (p_person->Genus.Person->pcom_ai_state != PCOM_AI_STATE_SUMMON)
			{
				PCOM_set_person_ai_summon(p_person);
			}

			PCOM_process_summon(p_person);

			break;

		case PCOM_AI_HYPOCHONDRIA:

			//
			// Easy peasy AI!
			//

			set_person_injured(p_person);

			break;

		default:
			ASSERT(0);
			break;
	}

	#if SANITY_PREVAILED

	//
	// MASTER OVERRIDE! If you are racist and see someone of the colour
	// you hate...
	//

	if (p_person->Genus.Person->Flags & FLAG_PERSON_RACIST)
	{
		if (((THING_NUMBER(p_person) + GAME_TURN) & 0xf) == 0)
		{
			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL     ||
				p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS ||
				p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOW)
			{
				SLONG i;

				Thing *p_found;

				//
				// If you can see someone you hate...
				//

				PCOM_found_num = THING_find_sphere(
									p_person->WorldPos.X >> 8,
									p_person->WorldPos.Y >> 8,
									p_person->WorldPos.Z >> 8,
									0x600,
									PCOM_found,
									PCOM_MAX_FIND,
									1 << CLASS_PERSON);

				for (i = 0; i < PCOM_found_num; i++)
				{
					p_found = TO_THING(PCOM_found[i]);

					if (p_found->Genus.Person->colour == p_person->Genus.Person->hate_colour)
					{
						//
						// He hates this person!
						//

						if (can_a_see_b(p_person, p_found))
						{
							PCOM_set_person_ai_kill_person(p_person, p_found);
						}
					}
				}
			}
		}
	}

	#endif
}

//
// For a person driving a car.. this function looks ahead of the car and
// returns what actions you should take to avoid any obsticles.
//

#define PCOM_RUNOVER_STOP		(1 << 0)
#define PCOM_RUNOVER_BEEP_HORN	(1 << 1)
#define PCOM_RUNOVER_SHOUT_OUT	(1 << 2)
#define PCOM_RUNOVER_TURN_LEFT	(1 << 3)
#define PCOM_RUNOVER_TURN_RIGHT	(1 << 4)
#define PCOM_RUNOVER_SLOW_DOWN	(1 << 5)
#define PCOM_RUNOVER_RUNAWAY	(1 << 6)	// Get our of the car and peg-it!
#define PCOM_RUNOVER_REVERSE	(1 << 7)

Thing *PCOM_runover_scary_person;

// no messing here, -ve dangle means turning LEFT as most people would imagine it would

SLONG PCOM_find_runover_thing(Thing *p_person, SLONG dangle)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG px;
	SLONG pz;

	SLONG cx;
	SLONG cz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG what;
	SLONG dist;
	SLONG cprod;
	SLONG angle;

	#define PCOM_RUNOVER_FIND 8

	UWORD	found[PCOM_RUNOVER_FIND];
	SLONG	num;
	SLONG	velocity;

	Thing*		p_vehicle;
	Vehicle*	veh;
	Thing*		p_found;
	Vehicle*	v_found;

	ASSERT(p_person->Genus.Person->Flags & (FLAG_PERSON_DRIVING | FLAG_PERSON_BIKING));

	p_vehicle = TO_THING(p_person->Genus.Person->InCar);
	veh = p_vehicle->Genus.Vehicle;

	//
	// Find all the things in front of the vehicle.
	//

	num = VEH_find_runover_things(p_vehicle, found, PCOM_RUNOVER_FIND, dangle);

	//
	// The movement vector and speed of the vehicle.
	//

	switch(p_vehicle->Class)
	{
		case CLASS_VEHICLE:
			angle    = p_vehicle->Genus.Vehicle->Angle & 2047;
			velocity = Root((veh->VelX >> 4) * (veh->VelX >> 4) + (veh->VelZ >> 4) * (veh->VelZ >> 4)) >> 4;
			break;

		case CLASS_BIKE:
			angle    = p_vehicle->Draw.Mesh->Angle & 2047;
			velocity = 0;
			break;

		default:
			ASSERT(0);
			break;
	}

	dx = -SIN(angle) >> 8;
	dz = -COS(angle) >> 8;

	//
	// get some info
	//

	SLONG	wx = p_vehicle->WorldPos.X >> 8;
	SLONG	wz = p_vehicle->WorldPos.Z >> 8;

	SLONG	onroad = ROAD_is_road(wx >> 8, wz >> 8);

	// find nearest road
	SLONG	rn1;
	SLONG	rn2;

	if ((p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_DRIVE_DOWN) ||
		(p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_PARK_CAR_ON_ROAD))
	{
		rn1 = p_person->Genus.Person->pcom_move_arg & 0xFF;
		rn2 = (p_person->Genus.Person->pcom_move_arg >> 8) & 0xFF;
	}
	else
	{
		ROAD_find(wx, wz, &rn1, &rn2);
	}

	// get side
	SLONG	rd = ROAD_signed_dist(rn1, rn2, wx, wz);

	// find nearest node
	SLONG	nn;
	SLONG	nnd;

	nn = ROAD_nearest_node(rn1, rn2, wx, wz, &nnd);

#if 0
	SLONG	col;
	if (nnd < AT_JUNCTION)			col = 0xFF0000;
	else if (nnd < NEAR_JUNCTION)	col = 0xFFFF00;
	else							col = 0x00FF00;

	AENG_world_line(p_person->WorldPos.X >> 8, p_person->WorldPos.Y >> 8, p_person->WorldPos.Z >> 8, 16, col,
					p_person->WorldPos.X >> 8, (p_person->WorldPos.Y >> 8) + 0x100, p_person->WorldPos.Z >> 8, 0, col,
					TRUE);
#endif

	// set action to none
	what = 0;

	// if we're near a junction, add objects
	SLONG	orig_num = num;

	if (nnd < NEAR_JUNCTION)
	{
		SLONG	jx,jz;

 		ROAD_node_pos(nn, &jx, &jz);

		if ((jx - wx) * dx + (jz - wz) * dz > 0)
		{
			// approaching it
			num += THING_find_sphere(jx, p_person->WorldPos.Y >> 8, jz, JN_RADIUS_IN, found + num, PCOM_RUNOVER_FIND - num, (1 << CLASS_PERSON) | (1 << CLASS_VEHICLE));
			what = PCOM_RUNOVER_SLOW_DOWN;
		}
	}

	// remove self from list
	for (i = 0; i < num; i++)
	{
		if ((found[i] == THING_NUMBER(p_person)) || (found[i] == THING_NUMBER(p_vehicle)))
		{
			found[i] = found[--num];
		}
	}


	// check objects in list
	for (i = 0; i < num; i++)
	{
		p_found = TO_THING(found[i]);

#if 0
#define BLOCKED(C)	AENG_world_line(p_person->WorldPos.X >> 8, p_person->WorldPos.Y >> 8, p_person->WorldPos.Z >> 8, 16, C,		\
									p_found->WorldPos.X >> 8, p_found->WorldPos.Y >> 8, p_found->WorldPos.Z >> 8, 16, C, TRUE);			
#else
#define BLOCKED(C)
#endif

		if ((p_found->Class == CLASS_PERSON) && (p_found->State == STATE_DYING || p_found->State == STATE_DEAD))
		{
			// Ignore dead or dying people
			continue;
		}

		// if it's on a junction, check dot product
		if (num >= orig_num)
		{
			SLONG	vx = (p_found->WorldPos.X - p_person->WorldPos.X) >> 8;
			SLONG	vz = (p_found->WorldPos.Z - p_person->WorldPos.Z) >> 8;

			if (vx * dx + vz * dz < 0)	continue;	// behind you!
		}

		BLOCKED(0x0000FF);

		// if we're on the road ...
		if (onroad)
		{
			// then ignore off-road things
			if (!ROAD_is_road(p_found->WorldPos.X >> 16, p_found->WorldPos.Z >> 16))	continue;

			// find which road the thing is on
			SLONG	trn1;
			SLONG	trn2;

			ROAD_find(p_found->WorldPos.X >> 8, p_found->WorldPos.Z >> 8, &trn1, &trn2);

			// find nearest node
			SLONG	tnn;
			SLONG	tnnd;

			tnn = ROAD_nearest_node(trn1, trn2, p_found->WorldPos.X >> 8, p_found->WorldPos.Z >> 8, &tnnd);

			//
			// handle junctions
			//
//#if 0
			if ((nnd < AT_JUNCTION) && (p_found->Class == CLASS_VEHICLE))
			{
				// *on* the junction
				if ((tnnd < AT_JUNCTION) && (tnn == nn) && (ROAD_node_degree(nn) > 2))
				{
					// another car is on the junction
					if (p_vehicle < p_found)
					{
						// reverse back to make room
						BLOCKED(0x888888);
						return PCOM_RUNOVER_REVERSE;
					}
				}
				// ignore other cars when we're *on* the junction
				continue;
			}
//#endif

			if ((nnd < NEAR_JUNCTION) && (p_found->Class == CLASS_VEHICLE) &&
				(tnn ==nn) && (ROAD_node_degree(nn) > 2))
			{
				// near junction, looking at a car on the same junction, not a bend
				if (tnnd < AT_JUNCTION)
				{
					// stop if approaching a busy junction
					BLOCKED(0x888888);
					return PCOM_RUNOVER_STOP;
				}
				else if (tnnd < NEAR_JUNCTION)
				{
					// stop if both approaching and other car is nearer
					if (tnnd < nnd)
					{
						BLOCKED(0x888888);
						return PCOM_RUNOVER_STOP;
					}
				}
			}

			if (nnd > AT_JUNCTION)// && (abs(dangle) < 32))
			{
				// look at the side of the road we're both on
				// (only take this into account away from junctions, and when we aren't turning)
				if (((rn1 == trn1) && (rn2 == trn2)) ||
					((rn1 == trn2) && (rn2 == trn1)))
				{
					SLONG	trd = ROAD_signed_dist(rn1, rn2, p_found->WorldPos.X >> 8, p_found->WorldPos.Z >> 8);

					if (abs(trd - rd) > 0xC0)	continue;
				}
			}
		}

		switch(p_found->Class)
		{
			case CLASS_PERSON:
				if (p_found->OnFace)	break;	// ignore people on cars

				if ((veh->Type != VEH_TYPE_VAN) && (veh->Type != VEH_TYPE_AMBULANCE) && (veh->Type != VEH_TYPE_WILDCATVAN))
				{
					// can't hijack vans or ambulances
					if ((p_found->Genus.Person->Flags & FLAG_PERSON_GUN_OUT) ||
						(p_found->Genus.Person->SpecialUse))
					{
						// person has a gun out
						SLONG angle  = calc_angle(dx,dz);
						SLONG dangle = p_found->Draw.Tweened->Angle - angle;

						if (abs(dangle) < 256)
						{
							// pointing it in our direction
							if (veh->Flags & FLAG_VEH_SHOT_AT)
							{
								// and shooting it!
								if (Random() & 0x08)
								{
									// scared!
									PCOM_runover_scary_person = p_found;
									BLOCKED(0x888888);
									return PCOM_RUNOVER_STOP | PCOM_RUNOVER_RUNAWAY;
								}
								else
								{
									// not scared
									veh->Flags &= ~FLAG_VEH_SHOT_AT;
								}
							}
						}
					}
				}

				if (ROAD_is_zebra(
						p_found->WorldPos.X >> 16,
						p_found->WorldPos.Z >> 16))
				{
					// Always stop for people on zebra crossings...
					BLOCKED(0x888888);
					return PCOM_RUNOVER_STOP;
				}
				else
				{
					BLOCKED(0x888888);
					switch(((GAME_TURN) + THING_NUMBER(p_vehicle)) & 0x7f)
					{
						case 0:			return PCOM_RUNOVER_STOP | PCOM_RUNOVER_BEEP_HORN;
						case 25:		return PCOM_RUNOVER_STOP | PCOM_RUNOVER_SHOUT_OUT;
						default:		return PCOM_RUNOVER_STOP;
					}
				}


				break;

			case CLASS_VEHICLE:
				{
					v_found = p_found->Genus.Vehicle;

					SLONG vel = Root((v_found->VelX >> 4) * (v_found->VelX >> 4) + (v_found->VelZ >> 4) * (v_found->VelZ >> 4)) >> 4;

					//
					// Do we stop for a car in front of us or do we try and drive around?
					//

					SLONG avoid = FALSE;

					// avoid parked (driverless) cars and dead cars
					if (!v_found->Driver)														avoid = TRUE;
					if ((p_found->State == STATE_DYING) || (p_found->State == STATE_DEAD))		avoid = TRUE;

					if (p_person->Genus.Person->pcom_bent & PCOM_BENT_DILIGENT)
					{
						//
						// Diligent people are in a hurry!
						//

						avoid = TRUE;
					}

					if (avoid)
					{
						if (vel < (velocity >> 2))
						{
#if 0
							//
							// This car is parked or going much slower than us! Try and avoid it.
							// 

							px = p_found->WorldPos.X - p_vehicle->WorldPos.X >> 8;
							pz = p_found->WorldPos.Z - p_vehicle->WorldPos.Z >> 8;

							cprod = px*dz - pz*dx;
		
							if (cprod < 0)
							{
								what |= PCOM_RUNOVER_TURN_LEFT;
							}
							else
							{
								what |= PCOM_RUNOVER_TURN_RIGHT;
							}
#endif
							// no, just stop
							BLOCKED(0x888888);
							return PCOM_RUNOVER_STOP;
						}
						else //if (vel < velocity)
						{
							BLOCKED(0xFF0000);
							return PCOM_RUNOVER_STOP;
						}
					}
					else
					{
//						if (vel < velocity)
						{
							BLOCKED(0x00FF00);
							return PCOM_RUNOVER_STOP;
						}
					}
				}
				break;

			default:
				ASSERT(0);
				break;
		}
	}

	return what;
}



void PCOM_process_movement(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG what;
	SLONG dest_x;
	SLONG dest_z;
	SLONG goal_x;
	SLONG goal_z;
	SLONG ladder;
	SLONG wangle;
	SLONG dangle;
	SLONG wspeed;
	SLONG dspeed;
	SLONG dlane;

	SLONG renav = FALSE;

	Thing *p_vehicle;
	Thing *p_target;
	Thing *p_bike;

	#ifdef BIKE

	BIKE_Control bc;

	#endif

	SLONG steer;
	SLONG accel;

	//
	// Low-level movement.
	//

	if(p_person->State==STATE_DYING||p_person->State==STATE_DEAD)
		return;

	//
	// High-level movement states.
	//

	switch(p_person->Genus.Person->pcom_move_state)
	{
		case PCOM_MOVE_STATE_PLAYER:
			break;

		case PCOM_MOVE_STATE_STILL:

			if (p_person->SubState==SUB_STATE_GRAPPLE_HELD)
			{
				p_person->Genus.Person->pcom_move_state=PCOM_MOVE_STATE_GRAPPLE;
			}

			break;

		case PCOM_MOVE_STATE_GOTO_THING_SLIDE:

			//
			// Finished sliding?
			//

			if (p_person->State == STATE_IDLE ||
				p_person->State == STATE_GUN && p_person->SubState == SUB_STATE_AIM_GUN)
			{
				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_STILL;
			}
			else
			{
				//
				// Make sure we always slide towards our target.
				//

				ASSERT(p_person->Genus.Person->pcom_move_arg);

				turn_towards_thing(p_person, TO_THING(p_person->Genus.Person->pcom_move_arg));
			}

			break;

		case PCOM_MOVE_STATE_GOTO_XZ:
		case PCOM_MOVE_STATE_GOTO_WAYPOINT:
		case PCOM_MOVE_STATE_GOTO_THING:

			if (p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
			{
				break;
			}

			switch(p_person->Genus.Person->pcom_move_substate)
			{
				case PCOM_MOVE_SUBSTATE_GOTO:

					//
					// Arrived at our sub-goal?
					//

					PCOM_get_mav_action_pos(
						p_person,
					   &goal_x,
					   &goal_z);
#ifndef PSX
#ifndef TARGET_DC
					if (ControlFlag&&allow_debug_keys)
					{
						AENG_world_line(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							16,
							0x00ffff00,
							goal_x,
							p_person->WorldPos.Y >> 8,
							goal_z,
							0,
							0x000000ff,
							TRUE);
					}
#endif
#endif
/*
					goal_x &= 0xffffff00;
					goal_z &= 0xffffff00;

					goal_x |= 0x80;
					goal_z |= 0x80;
*/
					dist = PCOM_person_dist_from(
								p_person,
								goal_x,
								goal_z);

					if (dist < PCOM_ARRIVE_DIST)
					{	
						if (p_person->Genus.Person->pcom_move_ma.action == MAV_ACTION_GOTO)
						{
							//
							// Nothing more to do for this subgoal. Initialise a renavigation.
							//

							renav = TRUE;
						}
						else
						{
							//
							// We've arrived but we have to do another action. Make sure we are
							// pointing in the correct direction.
							//

							p_person->Draw.Tweened->AngleTo =
							p_person->Draw.Tweened->Angle   =
								PCOM_get_angle_for_dir(p_person->Genus.Person->pcom_move_ma.dir);

							//
							// Initialise the action.
							//

							switch(p_person->Genus.Person->pcom_move_ma.action)
							{
								case MAV_ACTION_JUMP:
								case MAV_ACTION_JUMPPULL:
								case MAV_ACTION_JUMPPULL2:
									set_person_running_jump(p_person);
									break;

								case MAV_ACTION_PULLUP:
									set_person_running_jump(p_person);
									break;

								case MAV_ACTION_CLIMB_OVER:
									set_person_running_jump(p_person);
									break;

								case MAV_ACTION_FALL_OFF:
									set_person_walking(p_person);
									break;

								case MAV_ACTION_LADDER_UP:
									
									//
									// Look for a nearby ladder.
									//

									ladder = find_nearby_ladder_colvect(p_person);

									if (ladder)
									{
										set_person_climb_ladder(p_person, ladder);
									}
									else
									{
										ASSERT(0);
									}

									break;

								default:
									ASSERT(0);
									break;
							}

							//
							// Process the action.
							//

							p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_ACTION;
						}
					}
					else
					{
						if (p_person->State    == STATE_DANGLING &&
							p_person->SubState == SUB_STATE_DANGLING)
						{
							//
							// Ended up falling and dangling off of somewhere.
							//

							set_person_pulling_up(p_person);
						}
						else
						if (p_person->Genus.Person->SlideOdd >= 50)
						{
							if (p_person->Genus.Person->pcom_ai       == PCOM_AI_CIV &&
								p_person->Genus.Person->pcom_move     == PCOM_MOVE_WANDER &&
								p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
							{
								//
								// Wandering civs don't jump!
								//
							}
							else
							{
								//
								// We've slid along something odd for 50 consecutive gameturns.
								// EMERGENCY! Take evasive action.
								//

								set_person_running_jump(p_person);

								p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_ACTION;
								p_person->Genus.Person->SlideOdd           = 0;
							}
						}
						else
						if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(20))
						{
							//
							// We had better renav! We might be in trouble.
							//

							if ((p_person->State == STATE_GOTOING)                                        ||
								(p_person->State == STATE_GUN && p_person->SubState == SUB_STATE_AIM_GUN) ||
								(p_person->State == STATE_IDLE))
							{
								renav = TRUE;
							}
							else
							{
								//
								// Don't renav in the middle of a complicated manouvre (like climbing a ladder or jumping)
								// 
							}
						}
					}

					break;

				case PCOM_MOVE_SUBSTATE_ACTION:
					
					switch(p_person->State)
					{
						case STATE_MOVEING:

							if (p_person->SubState == SUB_STATE_RUNNING)
							{
								//
								// Running after completing a jump.
								//

								renav = TRUE;
							}

							break;

						case STATE_IDLE:

							//
							// Stopped doing anything- it must be time for a renavigation!
							//

							renav = TRUE;

							break;

						case STATE_LANDING:
						case STATE_JUMPING:
						case STATE_FIGHTING:
						case STATE_FALLING:
						case STATE_USE_SCENERY:
						case STATE_DOWN:
						case STATE_HIT:
						case STATE_CHANGE_LOCATION:
						case STATE_DYING:
						case STATE_DEAD:
							break;

						case STATE_DANGLING:

							if (p_person->SubState == SUB_STATE_DANGLING)
							{
								switch(p_person->Genus.Person->pcom_move_ma.action)
								{
									case MAV_ACTION_JUMPPULL:
									case MAV_ACTION_JUMPPULL2:
									case MAV_ACTION_PULLUP:
										set_person_pulling_up(p_person);
										break;

									default:
										set_person_drop_down(p_person, PERSON_DROP_DOWN_OFF_FACE);
										break;
								}
							}
							else
							if (p_person->SubState == SUB_STATE_DANGLING_CABLE)
							{	
								//
								// Should never be on a cable!
								//

								set_person_drop_down(p_person, PERSON_DROP_DOWN_OFF_FACE);
							}

							break;

						case STATE_CLIMB_LADDER:
							
							//
							// If you've finished getting on the ladder,
							// then start climbing up it.
							//

							if (p_person->SubState == SUB_STATE_ON_LADDER)
							{
								p_person->SubState = SUB_STATE_CLIMB_UP_LADDER;
							}

							break;

						case STATE_HIT_RECOIL:
							break;

						case STATE_CLIMBING:

							if (p_person->SubState == SUB_STATE_STOPPING || p_person->SubState == SUB_STATE_CLIMB_AROUND_WALL)
							{
								if (p_person->Genus.Person->pcom_move_ma.action == MAV_ACTION_CLIMB_OVER)
								{
									//
									// Climb over.
									//

									p_person->SubState = SUB_STATE_CLIMB_UP_WALL;
								}
								else
								{
									//
									// Shouldn't be on a fence.
									//

									set_person_drop_down(p_person, 0);
								}
							}

							break;

						case STATE_GUN:

							if (p_person->SubState == SUB_STATE_AIM_GUN)
							{
								//
								// Stopped doing anything- it must be time for a renavigation!
								//

								renav = TRUE;
							}

							break;

						case STATE_SHOOT:
						case STATE_DRIVING:
						case STATE_NAVIGATING:
						case STATE_WAIT:
						case STATE_FIGHT:
						case STATE_STAND_UP:
						case STATE_MAVIGATING:
						case STATE_GRAPPLING:
							break;

						case STATE_GOTOING:
							break;

						default:
							ASSERT(0);
							break;
					}

					break;

				case PCOM_MOVE_SUBSTATE_LOSMAV:

					{
						ASSERT(p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_GOTO_THING);

						Thing *p_target = TO_THING(p_person->Genus.Person->pcom_move_arg);

						if (!PCOM_should_i_try_to_los_mav_to_person(p_person, p_target))
						{
							//
							// We should try proper mavigation!
							//

							renav = TRUE;
						}
						else
						{
							//
							// Update where this person want to be going to.
							//

							p_person->Genus.Person->GotoX = p_target->WorldPos.X >> 8;
							p_person->Genus.Person->GotoZ = p_target->WorldPos.Z >> 8;
						}

						if (p_person->State == STATE_IDLE)
						{
							//
							// Something happened to this bloke! Get him moving again.
							//

							renav = TRUE;
						}
					}

					break;

				default:
					ASSERT(0);
					break;
			}

			if (renav)
			{
				if (p_person->SubState == SUB_STATE_PULL_UP ||
					p_person->SubState == SUB_STATE_CLIMB_OFF_LADDER_TOP)
				{
					//
					// Don't renav while doing this...
					//
				}
				else
				{
					//
					// Time for a renavigation.
					//

					PCOM_renav(p_person);
				}
			}

			p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			break;

		case PCOM_MOVE_STATE_PAUSE:
			p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
			break;

		case PCOM_MOVE_STATE_WAIT_CIRCLE:
			if ((p_person->State   == STATE_IDLE && p_person->SubState != SUB_STATE_IDLE_CROUTCH_ARREST) ||
				(p_person->StateFn == NULL))
			{
				//
				// The animation is over.
				//

				set_person_recircle(p_person);
				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_CIRCLE;
			}
			else
			if(p_person->State   == STATE_FIGHTING && (p_person->SubState == SUB_STATE_GRAPPLE_HOLD || p_person->SubState == SUB_STATE_GRAPPLE_HELD))
			{
				//
				// The animation is over.
				//

				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_GRAPPLE;

			}
			break;

		case PCOM_MOVE_STATE_ANIMATION:

			p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			if ((p_person->State   == STATE_IDLE    && p_person->SubState != SUB_STATE_IDLE_CROUTCH_ARREST)	||
				(p_person->State   == STATE_GUN     && p_person->SubState == SUB_STATE_AIM_GUN)             ||
				(p_person->State   == STATE_MOVEING && p_person->SubState == SUB_STATE_INSIDE_VEHICLE)      ||
				(p_person->State   == STATE_MOVEING && p_person->SubState == SUB_STATE_SIMPLE_ANIM_OVER && p_person->Genus.Person->pcom_move != PCOM_MOVE_DANCE && p_person->Genus.Person->pcom_move != PCOM_MOVE_HANDS_UP && p_person->Genus.Person->pcom_move != PCOM_MOVE_TIED_UP) ||
				(p_person->StateFn == NULL))
			{
				//
				// The animation is over.
				//

				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_STILL;
			}
			else
			if(p_person->State == STATE_FIGHTING && (p_person->SubState == SUB_STATE_GRAPPLE_HOLD || p_person->SubState == SUB_STATE_GRAPPLE_HELD))
			{
				//
				// The animation is over.
				//

				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_GRAPPLE;
			}

			break;

		case PCOM_MOVE_STATE_GRAPPLE:
			if (p_person->SubState == SUB_STATE_GRAPPLE_HOLD)
			{
				p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

				if (p_person->Genus.Person->pcom_move_counter++ > p_person->Genus.Person->pcom_ai_counter)
				{
					if (p_person->Genus.Person->pcom_ai == PCOM_AI_COP ||
						p_person->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER)
					{
						//
						// Cops are Ninjas! They throw you to the ground and
						// then arrest you.
						//

						p_person->Genus.Person->Flags |= FLAG_PERSON_REQUEST_PUNCH;
					}
					else
					{
						p_person->Genus.Person->Flags |= FLAG_PERSON_REQUEST_KICK;
					}

					p_person->Genus.Person->pcom_move_counter  = 0;
					p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_ANIMATION;
				}
			}
			if (p_person->SubState == SUB_STATE_GRAPPLE_HELD)
			{
//				if((GAME_TURN+THING_NUMBER(p_person))&1)
				if((PTIME(p_person))&1)
				{
					p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;
				}
			}
			if (p_person->State == STATE_IDLE)
			{
				set_person_recircle(p_person); //, TO_THING(p_person->Genus.Person->pcom_move_arg));
				p_person->Genus.Person->pcom_move_state   = PCOM_MOVE_STATE_CIRCLE;
				p_person->Genus.Person->pcom_move_arg     = p_person->Genus.Person->Target;
				p_person->Genus.Person->pcom_move_counter = 0;



			}


			break;
		case PCOM_MOVE_STATE_CIRCLE:

			p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			if (p_person->State == STATE_IDLE || (p_person->State == STATE_GUN && p_person->SubState == SUB_STATE_AIM_GUN))
			{
				//
				// This person has probably been punched and has just recovered.
				//

				set_person_recircle(p_person); //, TO_THING(p_person->Genus.Person->pcom_move_arg));
			}
			else
			if((p_person->SubState == SUB_STATE_GRAPPLE_HELD))
			{
				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_GRAPPLE;

			}
			else
			if((p_person->SubState == SUB_STATE_GRAPPLE_HELD))
			{
				p_person->Genus.Person->pcom_move_state = PCOM_MOVE_STATE_GRAPPLE;

			}

			break;

		case PCOM_MOVE_STATE_PARK_CAR:

			ParkCar(p_person);

			break;

		case PCOM_MOVE_STATE_DRIVETO:
		case PCOM_MOVE_STATE_DRIVE_DOWN:
		case PCOM_MOVE_STATE_PARK_CAR_ON_ROAD:

			DriveCar(p_person);

			break;

		#ifdef BIKE

		case PCOM_MOVE_STATE_PARK_BIKE:

			ParkBike(p_person);

			break;

		case PCOM_MOVE_STATE_BIKETO:
		case PCOM_MOVE_STATE_BIKE_DOWN:

			DriveBike(p_person);

			break;

		#endif

		default:
			ASSERT(0);
			break;
	}
}



void PCOM_process_person(Thing *p_person)
{
	//
	// Do movement AI and low-level state stuff.
	//

	if (p_person->StateFn)
	{
		p_person->StateFn(p_person);
	}

	if (p_person->Genus.Person->PlayerID)
	{
		//
		// Keep track of how long the player has been idle.
		//

		if (p_person->State == STATE_IDLE)
		{
			p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;

			if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(30))
			{
				p_person->Genus.Person->pcom_move_counter = 0;

				UWORD pnear;

				//
				// Is Darci near someone dancing?
				//

				remove_thing_from_map(p_person);

				pnear = THING_find_nearest(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							0x200,
							1 << CLASS_PERSON);

				add_thing_to_map(p_person);

				if (pnear)
				{
					Thing *p_near;

					//
					// Is this person dancing?
					//

					p_near = TO_THING(pnear);

					ASSERT(p_near->Class == CLASS_PERSON);

					if (p_near->State == STATE_MOVEING && (p_near->SubState == SUB_STATE_SIMPLE_ANIM || p_near->SubState == SUB_STATE_SIMPLE_ANIM_OVER) && (p_near->Draw.Tweened->CurrentAnim == ANIM_DANCE_BOOGIE || p_near->Draw.Tweened->CurrentAnim == ANIM_DANCE_WOOGIE || p_near->Draw.Tweened->CurrentAnim == ANIM_DANCE_HEADBANG))
					{
						//
						// This person is dancing... dance with him!
						//

						set_face_thing(p_person, p_near);

						set_person_do_a_simple_anim(p_person, p_near->Draw.Tweened->CurrentAnim);

						p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
						p_person->Genus.Person->Action = ACTION_SIT_BENCH;
					}
				}
			}
		}
		else
		{
			p_person->Genus.Person->pcom_move_counter = 0;
		}

		//
		// Players don't need to do the rest of this stuff.
		//

		return; 
	}

	PCOM_process_movement(p_person);

	if (p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
	{
		//
		// Nothing this person can do.
		//

		return;
	} 

	if (p_person->State == STATE_DEAD ||
		p_person->State == STATE_DYING)
	{
		//
		// No AI after brain death. But maybe this person should be
		// resurrected. Like Jesus.
		//

		if (p_person->Genus.Person->pcom_ai   == PCOM_AI_CIV &&
			p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
		{
			//
			// Wandering civillians come back to life when you can't see them!
			// 

			if (!(p_person->Flags & FLAGS_IN_VIEW))
			{
				p_person->Genus.Person->pcom_ai_counter += 1;

				if (p_person->Genus.Person->pcom_ai_counter > 200)
				{
					//
					// Bring this person back to life. Put him at home.
					//

					GameCoord newpos;

					newpos.X = (p_person->Genus.Person->HomeX << 16) + 0x8000;
					newpos.Z = (p_person->Genus.Person->HomeZ << 16) + 0x8000;
					newpos.Y = PAP_calc_map_height_at(newpos.X >> 8, newpos.Z >> 8);

					extern SLONG plant_feet(Thing *p_person);	// in collide.cpp

					plant_feet(p_person);

					//
					// Give him back his health.
					//

					p_person->Genus.Person->Health = health[p_person->Genus.Person->PersonType];

					//
					// Make him start doing what he normally does.
					//
					p_person->Flags &= ~FLAGS_BURNING;
					p_person->Genus.Person->BurnIndex = 0;

					PCOM_set_person_ai_normal(p_person);
				}
			}
		}
	}
	else
	{
		//
		// Is it time to change what we are doing?
		//

		PCOM_process_state_change(p_person);
	}
#ifdef	NOT_USED
	if (PCOM_person_doing_nothing_important(p_person))
	{
		if (p_person->Genus.Person->pcom_lookat_what == PCOM_LOOKAT_NOTHING)
		{
		//	if (((GAME_TURN + THING_NUMBER(p_person)) & 0x1f) == 0)
			{
		//		if (Random() & 0x4)
				{
					//
					// Look around for something interesting. The Player will do!
					//

					p_person->Genus.Person->pcom_lookat_what    = PCOM_LOOKAT_THING;
					p_person->Genus.Person->pcom_lookat_counter = 128 + (Random() & 0x7f);
					p_person->Genus.Person->pcom_lookat_index   = THING_NUMBER(NET_PERSON(0));
				}
			}
		}
		else
		{
			if (p_person->Genus.Person->pcom_lookat_counter == 0)
			{
				p_person->Genus.Person->pcom_lookat_what  = PCOM_LOOKAT_NOTHING;
				p_person->Genus.Person->pcom_lookat_index = 0;
			}
			else
			{
				p_person->Genus.Person->pcom_lookat_counter -= 1;
			}
		}
	}
	else
	{
		p_person->Genus.Person->pcom_lookat_what    = PCOM_LOOKAT_NOTHING;
		p_person->Genus.Person->pcom_lookat_counter = 0;
		p_person->Genus.Person->pcom_lookat_index   = 0;
	}
#endif
}

struct	Noise
{
	UWORD	Type;
	UWORD	Person;
	SWORD	X,Y,Z;
};


#define	MAX_NOISE	4

SWORD	noise_count=0;

struct	Noise noises[MAX_NOISE+1];

void	init_noises(void)
{
	noise_count=0;
}

void	process_noises(void)
{
	SLONG	c0;
	for(c0=0;c0<noise_count;c0++)
	{
		 PCOM_oscillate_tympanum(
			 noises[c0].Type,
			 (noises[c0].Person) ? (TO_THING(noises[c0].Person)) : NULL,
			 noises[c0].X,
			 noises[c0].Y,
			 noises[c0].Z,0);
	}
	noise_count=0;
}


void PCOM_oscillate_tympanum(
		SLONG  type,
		Thing *p_person,	// The person who caused the sound.
		SLONG  sound_x,		// The position of the sound.
		SLONG  sound_y,
		SLONG  sound_z,
		UBYTE  store_it)
{
	SLONG i;

	SLONG       found_upto;
	SLONG		radius;

	Thing *p_found;

	if (stealth_debug&&(p_person==NET_PERSON(0))) return;

	if(store_it)
	{
		struct	Noise	*p_noise;


		if(noise_count>=MAX_NOISE)
		{
			ASSERT(0);
			return;
		}


		p_noise=&noises[noise_count];
		p_noise->Type=type;
		p_noise->Person = p_person ? (THING_NUMBER(p_person)) : 0;
		p_noise->X=(SWORD)sound_x;
		p_noise->Y=(SWORD)sound_y;
		p_noise->Z=(SWORD)sound_z;
		noise_count++;
		return;
	}

	//
	// The volume of each type of sound.
	//

	switch(type)
	{
		case PCOM_SOUND_FOOTSTEP:    radius = 0x280; break;
		case PCOM_SOUND_UNUSUAL:     radius = 0x600; break;
		case PCOM_SOUND_HEY:         radius = 0x600; break;
		case PCOM_SOUND_ALARM:       radius = 0x800; break;
		case PCOM_SOUND_FIGHT:       radius = 0x900; break;
		case PCOM_SOUND_GUNSHOT:     radius = 0xa00; break;
		case PCOM_SOUND_DROP:	     radius = 0x200; break;
		case PCOM_SOUND_DROP_MED:    radius = 0x400; break;
		case PCOM_SOUND_DROP_BIG:    radius = 0x600; break;
		case PCOM_SOUND_VAN:         radius = 0x180; break;
		case PCOM_SOUND_BANG:        radius = 0x700; break;
		case PCOM_SOUND_MINE:	     radius = 0x300; break;
		case PCOM_SOUND_LOOKINGATME: radius = 0x400; break;
		case PCOM_SOUND_WANKER:		 radius = 0x400; break;
		case PCOM_SOUND_DRAW_GUN:    radius = 0x600; break;
		case PCOM_SOUND_GRENADE_HIT: radius = 0x300; break;
		case PCOM_SOUND_GRENADE_FLY: radius = 0x300; break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Look for people to be effected by this sound.
	//

	found_upto = THING_find_sphere(
					sound_x,
					sound_y,
					sound_z,
					radius,
					THING_array,
					THING_ARRAY_SIZE,
					THING_FIND_PEOPLE);

	if (type == PCOM_SOUND_DRAW_GUN && p_person)
	{
		//
		// This isn't a sound- it is a visual thing so we must take out
		// all people who can't see the person who is drawing the gun.
		//

		for (i = 0; i < found_upto; i++)
		{
			p_found = TO_THING(THING_array[i]);

			if (!can_a_see_b(p_found, p_person))
			{
				THING_array[i] = THING_array[found_upto - 1];

				i          -= 1;
				found_upto -= 1;
			}
		}
	}

	for (i = 0; i < found_upto; i++)
	{
		p_found = TO_THING(THING_array[i]);

		if (p_found == p_person)
		{
			//
			// Don't disturb yourself!
			//

			continue;
		}

		if (p_person && p_person->Class == CLASS_PERSON)
		{
			if (p_person->Genus.Person->Ware != p_found->Genus.Person->Ware)
			{
				//
				// Ignore sounds in a warehouse you're not in; if you're in 
				// a warehouse, ignore sounds outside.
				//

				continue;
			}
		}	

		if (p_found->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
		{
			//
			// Can't do anything even if he is scared.
			//

			continue;
		}

		if (p_found->State == STATE_DEAD ||
			p_found->State == STATE_DYING)
		{
			//
			// Dead people ignore their tympanum.
			//

			continue;
		}

		if (p_found->Genus.Person->PlayerID)
		{
			//
			// Don't do anything to players.
			//

			continue;
		}

		if (p_found->Genus.Person->pcom_bent & PCOM_BENT_ROBOT)
		{
			//
			// Robotic people ignore sounds.
			//

			continue;
		}

		if (p_found->Genus.Person->pcom_zone)
		{	
			//
			// Ignore sounds that aren't in your zone.
			//

			if (!(PCOM_get_zone_for_position(sound_x, sound_z) & p_found->Genus.Person->pcom_zone))
			{	
				continue;
			}
		}

		switch(p_found->Genus.Person->pcom_move_state)
		{
			case PCOM_MOVE_STATE_GOTO_XZ:
			case PCOM_MOVE_STATE_GOTO_WAYPOINT:
			case PCOM_MOVE_STATE_GOTO_THING:

				if (p_found->Genus.Person->pcom_move_substate == PCOM_MOVE_SUBSTATE_ACTION)
				{
					//
					// In the middle of doing a complicated moving manouvre.
					//

					continue;
				}
		}

		if (type == PCOM_SOUND_VAN)
		{
			//
			// Only civilians are scared of cars...
			//

			if (p_found->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_BIKING))
			{
				//
				// but not civs driving.
				//
			}
			else
			{
				if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_CIV)
				{
					if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PLACE &&
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_SUBSTATE_SUPRISED)
					{
						//
						// Don't scare them again or they'll never run away.
						//
					}
					else
					{
						if (ROAD_is_zebra(
								p_found->WorldPos.X >> 16,
								p_found->WorldPos.Z >> 16))
						{
							//
							// Civs on zebra crossings have a sense of security.
							// 
						}
						else
						{
							PCOM_set_person_ai_flee_place(
								p_found,
								sound_x,
								sound_z);
							p_found->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LEGIT; //MD27oct
						}
					}
				}
			}
		}
		else
		if (type == PCOM_SOUND_GRENADE_FLY ||
			type == PCOM_SOUND_GRENADE_HIT)
		{
			//
			// Ignore this unless you can see the grenade.
			//

			if (!can_i_see_place(
					p_found,
					sound_x,
					sound_y,
					sound_z))
			{
				if (type == PCOM_SOUND_GRENADE_HIT)
				{
					//
					// Some people will investigate this sound...
					//

					if (p_found->Genus.Person->pcom_ai == PCOM_AI_GANG ||
						p_found->Genus.Person->pcom_ai == PCOM_AI_COP)
					{
						if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL     ||
							p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS)
						{
							PCOM_set_person_ai_investigate(
								p_found,
								sound_x,
								sound_z);
						}
					}
				}
			}
			else
			{
				if (/* This grenade is going to land near me... */ 1)
				{
					PCOM_set_person_ai_flee_place(
						p_found,
						sound_x,
						sound_z);
				}
			}
		}
		else
		{
			switch(p_found->Genus.Person->pcom_ai)
			{
				case PCOM_AI_FIGHT_TEST:
				case PCOM_AI_NONE:
					break;

				case PCOM_AI_CIV:

					//
					// Scared of gun-shots, explosions and fighting.
					//

					if (type == PCOM_SOUND_FIGHT && (p_found->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK))
					{
						//
						// Fight back civs aren't scared of fighting!
						//
					}
					else
					if (type == PCOM_SOUND_GUNSHOT     ||
						type == PCOM_SOUND_BANG	       ||
						type == PCOM_SOUND_MINE        ||
						type == PCOM_SOUND_LOOKINGATME ||
						type == PCOM_SOUND_DRAW_GUN)
					{
						if (p_person)
						{
							PCOM_set_person_ai_flee_person(
								p_found,
								p_person);
						}
						else
						{
							PCOM_set_person_ai_flee_place(
								p_found,
								sound_x,
								sound_z);
						}
						if(!VIOLENCE)
						{
							p_found->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LEGIT; //MD27oct
						}
					}

					break;

				case PCOM_AI_GUARD:

					if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL     ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK)
					{
						//
						// Go an investigate the sound.
						//

						PCOM_set_person_ai_investigate(
							p_found,
							sound_x,
							sound_z);
						SOUND_Curious(p_found);
					}

					break;

				case PCOM_AI_ASSASIN:
					break;

				case PCOM_AI_GANG:

					//
					// Thugs ain't scared of anything! But they are interested by things...
					//

				case PCOM_AI_COP:

					//
					// Ignore sounds made by Darci
					//

					if (p_person && p_person->Genus.Person->PersonType == PERSON_DARCI)
					{
						//
						// Cops ignore sounds made by Darci?
						//

						if (type == PCOM_SOUND_FOOTSTEP)
						{
							//
							// They ignore Darci's footsteps!
							//

							continue;
						}
					}

					if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL     ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK   ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_WARM_HANDS)
					{
						PCOM_set_person_ai_investigate(
							p_found,
							sound_x,
							sound_z);
					}

					break;
				
				case PCOM_AI_DOORMAN:
				case PCOM_AI_BODYGUARD:
				case PCOM_AI_DRIVER:
				case PCOM_AI_BDISPOSER:
				case PCOM_AI_BIKER:
				case PCOM_AI_BOSS:
				case PCOM_AI_BULLY:
				case PCOM_AI_COP_DRIVER:
				case PCOM_AI_SUICIDE:
				case PCOM_AI_FLEE_PLAYER:
				case PCOM_AI_KILL_COLOUR:
				case PCOM_AI_BANE:
				case PCOM_AI_SHOOT_DEAD:
					break;

				case PCOM_AI_MIB:

					if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL     ||
						p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_HOMESICK)
					{
						if (p_person && p_person->Genus.Person->PlayerID)
						{
							PCOM_alert_nearby_mib_to_attack(p_person);
						}
					}

					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}
}


void PCOM_youre_being_grappled(
		Thing *p_victim,
		Thing *p_attacker)
{
	
}

SLONG	on_same_side(Thing *p_victim,Thing *p_attacker)
{
	if(p_victim->Genus.Person->PersonType==PERSON_ROPER||p_victim->Genus.Person->PersonType==PERSON_DARCI||p_victim->Genus.Person->PersonType==PERSON_COP)
	{
		if(p_attacker->Genus.Person->PersonType==PERSON_ROPER||p_attacker->Genus.Person->PersonType==PERSON_DARCI||p_attacker->Genus.Person->PersonType==PERSON_COP)
		{
			return(1);
		}
	}
	return(0);
}

#if DARCI_HITS_COPS

//
// Returns TRUE if the player hit the cop on purpose.
// 

SLONG PCOM_player_hit_cop_on_purpose(Thing *p_cop, Thing *p_darci)
{
	if (p_darci->Genus.Person->Flags & FLAG_PERSON_FELON)
	{
		//
		// Darci is a known villain.
		// 

		return TRUE;
	}

	//
	// Is there somebody fighting Darci nearby?
	//

	Thing *p_attacker;
	
	p_attacker = is_person_under_attack(p_darci);

	if (p_attacker)
	{
		//
		// Somebody is out to get Darci... it was probably an accident!
		//

		if (p_attacker->Genus.Person->pcom_ai == PCOM_AI_COP ||
			p_attacker->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER)
		{
			//
			// Hold on! She is already fighting a cop! She must have hit
			// me on purpose.
			//

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	//
	// Am I fighting anyone?
	//

	if (p_cop->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING ||
		p_cop->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL)
	{
		//
		// I am busy hitting somebody- darci is probably trying to help me!
		//

		return FALSE;
	}

	return TRUE;
}

#endif


//
// you have been attacked
//
extern	void	set_person_fight_idle(Thing *p_person);

void PCOM_attack_happened(
		Thing *p_victim,
		Thing *p_attacker)
{
	if (p_victim->Genus.Person->PlayerID)
	{
		return;
	}

	if (p_attacker==NET_PERSON(0) && stealth_debug) return;

	if (p_victim->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
	{
		//
		// Can't do anything.
		//

		return;
	}
extern	SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor);
	if(!people_allowed_to_hit_each_other(p_victim,p_attacker))
	{
		//
		//dont retaliate if you cant hurt them
		//
		return;
	}


	//
	//	I am truly a great coder
	//  I am just having a day off, OK?
	//

	if (p_victim->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK)
		goto	fight;
	if (p_victim->Genus.Person->pcom_bent & PCOM_BENT_ROBOT)
		return;

	if(p_victim->SubState==SUB_STATE_GRAPPLE_HOLD||p_victim->SubState==SUB_STATE_GRAPPLE_ATTACK)
	{
		//
		// take hit while grappleing, hmmmmmm
		//

		//
		// better let go of person we are grappelling
		//


		set_person_fight_idle(TO_THING(p_victim->Genus.Person->Target));
//		return;
	}

	if(p_victim->SubState==SUB_STATE_GRAPPLE_HELD)
	{
		//
		// don't really have any sensible options here
		//
		return;
	}


	switch(p_victim->Genus.Person->pcom_ai)
	{
		case PCOM_AI_CIV:
			goto flee;
			break;

		case PCOM_AI_COP:
		case PCOM_AI_COP_DRIVER:
			
			/*

			if (p_attacker->Genus.Person->pcom_ai == PCOM_AI_COP ||
				p_attacker->Genus.Person->pcom_ai == PCOM_AI_COP_DRIVER)
			{
				//
				// A cop hitting another cop? It must have been
				// an accident.
				//

				return;
			}

			*/

			#if DARCI_HITS_COPS

			if (p_attacker->Genus.Person->PlayerID)
			{
				//
				// A cop has been hit by the player... was this accidental?
				//

				if (PCOM_player_hit_cop_on_purpose(p_victim, p_attacker))
				{
					//
					// Has she hit me on purpose recently?
					//

					if (p_victim->Genus.Person->UnderAttack == 0)
					{
						//
						// She hasn't recently hit me!
						//

						p_victim->Genus.Person->UnderAttack = 0xffff;

						return;
					}
				}
				else
				{
					//
					// Ignore all accidental hits.
					//

					return;
				}
			}

			#endif

			// FALL-THROUGH

		case PCOM_AI_GANG:
		case PCOM_AI_GUARD:
		case PCOM_AI_ASSASIN:
			goto	fight;
			break;

		case PCOM_AI_BODYGUARD:
			
			//
			// Ignore hits from who you are meant to be protecting.
			//

			if (THING_NUMBER(p_attacker) != EWAY_get_person(p_victim->Genus.Person->pcom_ai_other))
			{
				goto fight;
			}

			break;
	}

	return;

fight:

	//
	// If you are already fighting your attacker...
	//

	if (p_victim->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL ||
		p_victim->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING)
	{
		if (p_victim->Genus.Person->pcom_ai_arg == THING_NUMBER(p_attacker))
		{
			return;
		}
	}

	//
	// Don't retaliate against someone in your own gang.
	//

	if ((p_victim  ->Genus.Person->pcom_bent & PCOM_BENT_GANG) &&
		(p_attacker->Genus.Person->pcom_bent & PCOM_BENT_GANG) &&
		(p_victim  ->Genus.Person->pcom_colour == p_attacker->Genus.Person->pcom_colour))
	{
		//
		// Ignore the hit (it was a mistake!)
		//

		return;
	}

	//
	// retaliate
	//

	PCOM_set_person_ai_kill_person(p_victim,p_attacker);

	return;

flee:
	PCOM_set_person_ai_flee_person(p_victim,p_attacker);
	if(!VIOLENCE)
		p_victim->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LEGIT; //MD27oct

	return;
}

void PCOM_attack_happened_but_missed(Thing *p_victim,Thing *p_attacker)
{
	if (p_victim->Genus.Person->PlayerID)
	{
		return;
	}

	if (p_attacker==NET_PERSON(0) && stealth_debug) return;

	if (p_victim->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
	{
		//
		// Can't do anything.
		//

		return;
	}

	if(on_same_side(p_victim,p_attacker))
		return;

	//
	//	I am truly a great coder
	//  I am just having a day off, OK?
	//

	if (p_victim->Genus.Person->pcom_bent & PCOM_BENT_ROBOT)
		return;

	if(!people_allowed_to_hit_each_other(p_victim,p_attacker))
		return;


	if (p_victim->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK)
		goto	fight;


	switch(p_victim->Genus.Person->pcom_ai)
	{
		case PCOM_AI_CIV:
			goto flee;
			break;

		case PCOM_AI_COP:
		case PCOM_AI_COP_DRIVER:

			#if DARCI_HITS_COPS

/*			
			if (p_attacker->Genus.Person->pcom_ai == PCOM_AI_COP)
			{
				//
				// A cop hitting another cop? It must have been
				// an accident.
				//

				return;
			}
*/
			if (p_attacker->Genus.Person->PlayerID)
			{
				//
				// A cop has been hit by the player... was this accidental?
				//

				if (PCOM_player_hit_cop_on_purpose(p_victim, p_attacker))
				{
					//
					// Has she hit me on purpose recently?
					//

					if (p_victim->Genus.Person->UnderAttack == 0)
					{
						//
						// She hasn't recently hit me!
						//

						p_victim->Genus.Person->UnderAttack = 0xffff;
					}
				}
				else
				{
					//
					// Ignore all accidental hits.
					//

					return;
				}
			}

			#endif

			// FALL-THROUGH


		case PCOM_AI_GANG:
		case PCOM_AI_GUARD:
		case PCOM_AI_ASSASIN:
			goto	fight;
			break;

		case PCOM_AI_BODYGUARD:
			
			//
			// Ignore hits from who you are meant to be protecting.
			//

			if (THING_NUMBER(p_attacker) != EWAY_get_person(p_victim->Genus.Person->pcom_ai_other))
			{
				goto fight;
			}

			break;
	}

	return;

fight:

	//
	// If you are already fighting your attacker...
	//

	if (p_victim->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL ||
		p_victim->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING)
	{
		if (p_victim->Genus.Person->pcom_ai_arg == THING_NUMBER(p_attacker))
		{
			return;
		}
	}

	//
	// Don't retaliate against someone in your own gang.
	//

	if ((p_victim  ->Genus.Person->pcom_bent & PCOM_BENT_GANG) &&
		(p_attacker->Genus.Person->pcom_bent & PCOM_BENT_GANG) &&
		(p_victim  ->Genus.Person->pcom_colour == p_attacker->Genus.Person->pcom_colour))
	{
		//
		// Ignore the hit (it was a mistake!)
		//

		return;
	}

	//
	// retaliate
	//

	PCOM_set_person_ai_kill_person(p_victim,p_attacker);

	return;

flee:
	PCOM_set_person_ai_flee_person(p_victim,p_attacker);
	if(!VIOLENCE)
		p_victim->Genus.Person->pcom_ai_substate = PCOM_AI_SUBSTATE_LEGIT; //MD27oct
	return;
}

SLONG PCOM_jumping_navigating_person_continue_moving(Thing *p_person)
{
	if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_GOTO_XZ	     ||
		p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_GOTO_WAYPOINT ||
		p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_GOTO_THING)
	{
		//
		// Unless your doing a difficult jump that you'll really have to go for...
		//

		if (p_person->Genus.Person->pcom_move_ma.action != MAV_ACTION_JUMPPULL2)
		{
			SLONG goal_x;
			SLONG goal_z;

			PCOM_get_mav_action_pos(
				p_person,
			   &goal_x,
			   &goal_z);

			SLONG dx = goal_x - (p_person->WorldPos.X >> 8);
			SLONG dz = goal_z - (p_person->WorldPos.Z >> 8);

			SLONG dist = abs(dx) + abs(dz);

			if (dist < 0x100)
			{
				//
				// Stop moving or you'll overshoot.
				//

				return FALSE;
			}
		}
	}

	return TRUE;
}



void PCOM_knockdown_happened(Thing *p_person)
{
	//
	// This function doesn't do anything to the person. It just changes
	// state of the 'brain' to wait for recovery.
	//

	if (p_person->Genus.Person->pcom_ai_state   == PCOM_AI_STATE_KILLING   ||
		p_person->Genus.Person->pcom_ai_state   == PCOM_AI_STATE_NAVTOKILL ||
		p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_CIRCLE  ||
		p_person->State                         == STATE_CIRCLING)
	{
		//
		// if you do this to people who are attacking you, then when they get up they just wander off, rather than getting back into the fight
		//
	}
	else
	{
		PCOM_set_person_ai_knocked_out(p_person);
	}
}



#ifndef PSX
CBYTE  PCOM_debug_string[256];
#endif

CBYTE *PCOM_person_state_debug(Thing *p_person)
{
#ifndef PSX
	SLONG i;
	CBYTE bent[256];

	if (p_person->Genus.Person->PlayerID)
	{
		sprintf(
			PCOM_debug_string,
			"Player %d\n"
			"Pos (0x%x,0x%x) height 0x%x\n"
			"Warehouse %d\n",
			p_person->Genus.Person->PlayerID,
			p_person->WorldPos.X >> 8,
			p_person->WorldPos.Z >> 8,
			p_person->WorldPos.Y >> 8,
			p_person->Genus.Person->Ware);

		return PCOM_debug_string;
	}

	//
	// A string describing the person's characteristics.
	//

	bent[0] = '\000';

	for (i = 0; i < PCOM_BENT_NUMBER; i++)
	{
		if (p_person->Genus.Person->pcom_bent & (1 << i))
		{
			strcat(bent, PCOM_bent_name[i]);
		}
	}

	if (p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
	{
			strcat(bent, "Helpless ");
	}

	sprintf(
		PCOM_debug_string,
		"%s%s%s,ptype %d,mesh %d personid %d \nState  %s : %s\nMove   %s : %s\nPerson 0x%p(%d) action %d anim %d Y %d st %d subs %d Agr %d",
		bent,
		PCOM_ai_name[p_person->Genus.Person->pcom_ai],
		(p_person->Genus.Person->Ware) ? " ware" : "",
		p_person->Genus.Person->PersonType,
		p_person->Draw.Tweened->MeshID,
		p_person->Draw.Tweened->PersonID,
		PCOM_ai_state_name   [p_person->Genus.Person->pcom_ai_state],
		PCOM_ai_substate_name[p_person->Genus.Person->pcom_ai_substate],
		PCOM_move_name       [p_person->Genus.Person->pcom_move],
		PCOM_move_state_name [p_person->Genus.Person->pcom_move_state],
		p_person,THING_NUMBER(p_person),p_person->Genus.Person->Action,p_person->Draw.Tweened->CurrentAnim,
		p_person->WorldPos.Y>>8,
		p_person->State,
		p_person->SubState,
		p_person->Genus.Person->Agression);

	return PCOM_debug_string;
#else
	return (CBYTE*)0;
#endif
}


SLONG	PCOM_cop_aiming_at_you(Thing *p_person,Thing *p_cop)
{
	if (p_cop == NET_PERSON(0) && stealth_debug)
	{
		return 0;
	}

	if (p_person->Genus.Person->pcom_bent & PCOM_BENT_ROBOT)
	{
		return 0;
	}

	if (p_person->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD)
	{
		//
		// Bodyguards ignore being aimed at by the person they are guarding.
		//

		if (THING_NUMBER(p_cop) == EWAY_get_person(p_person->Genus.Person->pcom_ai_other))
		{
			return 0;
		}
	}

	if(is_person_guilty(p_person))
	{

		if(p_person->State==STATE_DYING||p_person->State==STATE_DEAD)
			return(0);



		//
		// don't put hands up because you are guilty and need to do something else
		//

		if (p_person->Genus.Person->pcom_ai_state != PCOM_AI_STATE_NAVTOKILL && p_person->Genus.Person->pcom_ai_state != PCOM_AI_STATE_FLEE_PERSON)
		{
			PCOM_set_person_ai_navtokill(p_person, p_cop);
			return(1);
		}
		else
		{
			return(0);
		}
/*
		if (PCOM_person_has_any_sort_of_gun(p_person))
		{
			//
			// you have a gun so kill the cop
			//
			PCOM_set_person_ai_navtokill_shoot(p_person, p_cop);

			return(1);
		}
		else
		if (!(p_person->Genus.Person->pcom_bent & PCOM_BENT_FIGHT_BACK) && p_person->Genus.Person->pcom_ai != PCOM_AI_ASSASIN)
		{
			//
			// no gun so run away
			//

			PCOM_set_person_ai_flee_person(p_person, p_cop);
			
			return(1);				
		}
*/


	}
	if(p_person->Genus.Person->PersonType==PERSON_ROPER||p_person->Genus.Person->PersonType==PERSON_DARCI||p_person->Genus.Person->PersonType==PERSON_COP)
	{
		//
		// cops darci and roper don't put hands up
		//
		return(0);
	}

/*
	if(p_person->Genus.Person->PersonType==PERSON_ROPER||p_person->Genus.Person->PersonType==PERSON_DARCI||p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY || p_person->Genus.Person->PersonType==PERSON_THUG_RASTA|| p_person->Genus.Person->PersonType==PERSON_THUG_RED)
	{
		//
		// Thugs don't hands up
		//
		return(0);
	}
*/

	if(PCOM_person_doing_nothing_important(p_person))//||(VIOLENCE==0&&p_person->Genus.Person->PersonType==PERSON_CIV)
	{
		PCOM_set_person_ai_hands_up(p_person,p_cop);
		return(1);
	}
	return(0);

}


void PCOM_make_people_talk_to_eachother(
		Thing *p_person_a,
		Thing *p_person_b,
		UBYTE  is_a_asking_a_question,
		UBYTE  stay_looking_at_eachother,
		UBYTE  make_the_person_talked_at_listen)
{
	SLONG substate;

	if (is_a_asking_a_question)
	{
		substate = PCOM_AI_SUBSTATE_TALK_ASK;
	}
	else
	{
		substate = PCOM_AI_SUBSTATE_TALK_TELL;
	}

	PCOM_set_person_ai_talk_to(p_person_a, p_person_b, substate, stay_looking_at_eachother);

	p_person_a->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M | FLAG_PERSON_NON_INT_C;

	if (make_the_person_talked_at_listen)
	{
		PCOM_set_person_ai_talk_to(p_person_b, p_person_a, PCOM_AI_SUBSTATE_TALK_LISTEN, stay_looking_at_eachother);
		p_person_b->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M | FLAG_PERSON_NON_INT_C;
	}
}


void PCOM_stop_people_talking_to_eachother(
		Thing *p_person_a,
		Thing *p_person_b)
{
	p_person_a->Genus.Person->Flags &= ~FLAG_PERSON_NO_RETURN_TO_NORMAL;
	p_person_b->Genus.Person->Flags &= ~FLAG_PERSON_NO_RETURN_TO_NORMAL;

	if (p_person_a->Genus.Person->pcom_ai_state == PCOM_AI_STATE_TALK) {PCOM_set_person_ai_normal(p_person_a);}
	if (p_person_b->Genus.Person->pcom_ai_state == PCOM_AI_STATE_TALK) {PCOM_set_person_ai_normal(p_person_b);}
}


SLONG PCOM_person_a_hates_b(Thing *p_person_a, Thing *p_person_b)
{
	ASSERT(p_person_a->Class == CLASS_PERSON);
	ASSERT(p_person_b->Class == CLASS_PERSON);

	if (p_person_a == p_person_b)
	{
		//
		// Nobody hates themselves.
		//

		return FALSE;
	}

	//
	// People in the same gang like eachother.
	//

	if (p_person_a->Genus.Person->pcom_bent & p_person_b->Genus.Person->pcom_bent & PCOM_BENT_GANG)
	{
		if (p_person_a->Genus.Person->pcom_colour ==
			p_person_b->Genus.Person->pcom_colour)
		{
			return FALSE;
		}
	}

	if (p_person_a->Genus.Person->PlayerID)
	{
		//
		// Players like their bodyguards.
		//

		if (p_person_b->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD && EWAY_get_person(p_person_b->Genus.Person->pcom_ai_other) == THING_NUMBER(p_person_a))
		{
			return FALSE;
		}

		//
		// Players like other players.
		//

		if (p_person_b->Genus.Person->PersonType == PERSON_DARCI ||
			p_person_b->Genus.Person->PersonType == PERSON_ROPER)
		{
			return FALSE;
		}
			
		//
		// Players like people who are following them.
		//

		if (p_person_b->Genus.Person->pcom_move == PCOM_MOVE_FOLLOW && EWAY_get_person(p_person_b->Genus.Person->pcom_move_follow) == THING_NUMBER(p_person_a))
		{
			return FALSE;
		}

		//
		// Players hate everybody else...
		//
		
		return TRUE;
	}

	if (p_person_a->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL ||
		p_person_a->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING   ||
		p_person_a->Genus.Person->pcom_ai_state == PCOM_AI_STATE_TAUNT     ||
		p_person_a->Genus.Person->pcom_ai_state == PCOM_AI_STATE_ARREST)
	{
		//
		// These people don't like the person they are dealing with.
		//

		if (p_person_a->Genus.Person->pcom_ai_arg == THING_NUMBER(p_person_b))
		{
			return TRUE;
		}
	}

	return FALSE;
}


THING_INDEX PCOM_person_wants_to_kill(Thing *p_person)
{
	if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL ||
		p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING)
	{
		return p_person->Genus.Person->pcom_ai_arg;
	}

	return NULL;
}

// ParkCar
//
// handler for PCOM_MOVE_STATE_PARK_CAR

void ParkCar(Thing* p_person)
{
	SLONG wangle;
	SLONG dangle;

	//
	// If you're going to do some driving- then you have to be in a car!
	// 
	
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	Thing* p_vehicle = TO_THING(p_person->Genus.Person->InCar);

	//
	// What direction do we want the car to face?
	//

	p_vehicle->Genus.Vehicle->IsAnalog = 0;
	p_vehicle->Genus.Vehicle->Steering = 0;

	if (p_person->Genus.Person->pcom_move_arg)
	{
		wangle = EWAY_get_angle(p_person->Genus.Person->pcom_move_arg);

		dangle = wangle - p_vehicle->Genus.Vehicle->Angle;

		if (dangle < -1024) {dangle += 2048;}
		if (dangle > +1024) {dangle -= 2048;}

		if (dangle < -8) {p_vehicle->Genus.Vehicle->Steering = -1;}
		if (dangle > +8) {p_vehicle->Genus.Vehicle->Steering = +1;}
	}

	//
	// Deccelerate.
	//

	if (WITHIN(p_vehicle->Velocity, -10, +10))
	{
		p_vehicle->Genus.Vehicle->DControl     = 0;
		p_vehicle->Velocity                    = 0;
//		p_vehicle->Genus.Vehicle->Acceleration = 0;
	}
	else
	if (p_vehicle->Velocity > 0)
	{
		p_vehicle->Genus.Vehicle->DControl = VEH_DECEL;
	}
	else
	{
		p_vehicle->Genus.Vehicle->DControl = VEH_ACCEL;
	}
}

// DriveCar
//
// AI for driving a car - states PCOM_MOVE_STATE_DRIVE_DOWN, PCOM_MOVE_STATE_PARK_CAR_ON_ROAD and PCOM_MOVE_STATE_DRIVETO

void DriveCar(Thing* p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG dest_x;
	SLONG dest_z;
	SLONG wangle;
	SLONG dangle;
	SLONG dlane;
	SLONG dspeed;
	SLONG wspeed;
	SLONG what;

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING);

	Thing* p_vehicle = TO_THING(p_person->Genus.Person->InCar);
	ASSERT(p_vehicle);

#ifndef PSX
#ifndef TARGET_DC
#ifndef NDEBUG
	extern Thing* SelectedThing;
	if (LeftButton && (SelectedThing == p_vehicle))
	{
		_asm int 3;
		LeftButton = 0;
	}
#endif
#endif
#endif

	if (p_person->Genus.Person->pcom_move_state != PCOM_MOVE_STATE_PARK_CAR_ON_ROAD)
	{
		//
		// If the vehicle collided last time it moved, then we had better do something.
		//

		if (p_vehicle->Flags & FLAGS_COLLIDED)
		{
			switch(p_person->Genus.Person->pcom_move_substate)
			{
				case PCOM_MOVE_SUBSTATE_GOTO:
					p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_3PTURN;
					p_person->Genus.Person->pcom_move_counter  = 0;
					break;

				case PCOM_MOVE_SUBSTATE_3PTURN:
					p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
					p_person->Genus.Person->pcom_move_counter  = 0;
					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}

	//
	// Where are we going?
	//

	PCOM_get_person_dest(
		p_person,
	   &dest_x,
	   &dest_z);

	//
	// If we are driving down a road...
	//

	if ((p_person->Genus.Person->pcom_move_state != PCOM_MOVE_STATE_DRIVETO) && 
		(p_person->Genus.Person->pcom_move_substate != PCOM_MOVE_SUBSTATE_3PTURN))
	{
		//
		// How far are we from the middle of the road?
		// 

		dist = ROAD_signed_dist(
				p_person->Genus.Person->pcom_move_arg >> 8,
				p_person->Genus.Person->pcom_move_arg & 0xff,
				p_vehicle->WorldPos.X >> 8,
				p_vehicle->WorldPos.Z >> 8);

		if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_PARK_CAR_ON_ROAD)
		{
			dlane = 0x300 - dist;
		}
		else
		{
			dlane = 0x180 - dist;
		}

		if (abs(dlane) > 0x80 || p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_PARK_CAR_ON_ROAD)
		{
			//
			// We are quite for from our lane. We should aim for the
			// middle of our lane.
			//

			SLONG rx1;
			SLONG rz1;

			SLONG rx2;
			SLONG rz2;

			SLONG rdest_x;
			SLONG rdest_z;
			
			ROAD_node_pos(
				p_person->Genus.Person->pcom_move_arg >> 8,
			   &rx1,
			   &rz1);

			ROAD_node_pos(
				p_person->Genus.Person->pcom_move_arg & 0xff,
			   &rx2,
			   &rz2);

			nearest_point_on_line(
				rx1, rz1,
				rx2, rz2,
				p_vehicle->WorldPos.X >> 8,
				p_vehicle->WorldPos.Z >> 8,
			   &rdest_x,
			   &rdest_z);

			SLONG drx = SIGN(rx2 - rx1) << 8;
			SLONG drz = SIGN(rz2 - rz1) << 8;

			rdest_x += drx;
			rdest_z += drz;

			rdest_x += drx;
			rdest_z += drz;

			rdest_x -= drz;
			rdest_z += drx;

			if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_PARK_CAR_ON_ROAD)
			{
				rdest_x -= drz;
				rdest_z += drx;

				rdest_x -= drz;
				rdest_z += drx;
			}

			dest_x = rdest_x;
			dest_z = rdest_z;
		}
	}

	//
	// What direction do we want the car to face?
	//

	dx = dest_x - (p_vehicle->WorldPos.X >> 8);
	dz = dest_z - (p_vehicle->WorldPos.Z >> 8);

	dangle = (p_vehicle->Genus.Vehicle->Angle - (calc_angle(dx,dz) + 1024)) & 2047;

	if (dangle >= 1024)		dangle -= 2048;

	//
	// What speed do we want to be going at?
	//

	if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_PARK_CAR_ON_ROAD)
	{
		if (abs(dangle) < 100 && abs(dlane) < 0x80)
		{
			wspeed = 0;
		}
		else
		{
			wspeed = VEH_SPEED_LIMIT >> 1;
		}
	}
	else
	{
		switch(p_person->Genus.Person->pcom_move_substate)
		{
			case PCOM_MOVE_SUBSTATE_GOTO:

				if (abs(dangle) > 100)
				{
					wspeed = VEH_SPEED_LIMIT - ((abs(dangle) - 100) >> 1);

					if (wspeed < 250)
					{
						wspeed = 250;
					}
				}
				else
				{
					wspeed = VEH_SPEED_LIMIT;
				}

				if (p_person->Genus.Person->pcom_bent & PCOM_BENT_DILIGENT)
				{
					wspeed += wspeed;
				}

				break;

			case PCOM_MOVE_SUBSTATE_3PTURN:

				//
				// Always uturn for a little while at least.
				//

				if (p_person->Genus.Person->pcom_move_counter < PCOM_get_duration(15))
				{
				}
				else
				{
					if (abs(dangle) < 500)
					{
						//
						// We can start driving normally.
						//

						p_person->Genus.Person->pcom_move_substate = PCOM_MOVE_SUBSTATE_GOTO;
					}
				}

				wspeed = -VEH_REVERSE_SPEED;
				dangle = -dangle;

				break;

			default:
				ASSERT(0);
				break;
		}
	}

	//
	// Are we going to crash into anyone?
	//

	what = PCOM_find_runover_thing(p_person, dangle);


	if (what & PCOM_RUNOVER_STOP)       {if (wspeed > 0) {wspeed = 0;}}
//	if (what & PCOM_RUNOVER_BEEP_HORN)  {MFX_play_thing(THING_NUMBER(p_person), S_BEEP,         MFX_REPLACE, p_person);}
//	if (what & PCOM_RUNOVER_SHOUT_OUT)  {MFX_play_thing(THING_NUMBER(p_person), S_GETOUTTHEWAY, MFX_REPLACE, p_person);}
	if (what & (PCOM_RUNOVER_BEEP_HORN|PCOM_RUNOVER_SHOUT_OUT))  {MFX_play_thing(THING_NUMBER(p_person), SOUND_Range(S_CAR_HORN_START,S_CAR_HORN_END), MFX_REPLACE, p_person);}

	if (what & PCOM_RUNOVER_SLOW_DOWN)  {if (wspeed > 0) {wspeed >>= 1;}}
	if (what & PCOM_RUNOVER_REVERSE)	{wspeed = -(wspeed >> 2); dangle = 0;}

	if (what & PCOM_RUNOVER_TURN_LEFT)  {p_person->Genus.Person->pcom_move_flag |= PCOM_MOVE_FLAG_AVOID_LEFT;  p_person->Genus.Person->pcom_move_counter = 0;}
	if (what & PCOM_RUNOVER_TURN_RIGHT) {p_person->Genus.Person->pcom_move_flag |= PCOM_MOVE_FLAG_AVOID_RIGHT; p_person->Genus.Person->pcom_move_counter = 0;}

	if (what & PCOM_RUNOVER_RUNAWAY)
	{
		//
		// The movement bit is updating the high-level ai- sentience
		// through complexity.
		//
		
		PCOM_set_person_ai_flee_person(p_person, PCOM_runover_scary_person);

		return;
	}

	//
	// Turn towards our destination.
	//

	p_vehicle->Genus.Vehicle->IsAnalog = 0;
	p_vehicle->Genus.Vehicle->Steering = 0;

	if (dangle > +8) {p_vehicle->Genus.Vehicle->Steering = +1;}
	if (dangle < -8) {p_vehicle->Genus.Vehicle->Steering = -1;}

	//
	// Accelerate... decellerate?
	//

	p_vehicle->Genus.Vehicle->DControl = 0;

	dspeed = p_vehicle->Velocity - wspeed;

	if (dspeed < -10) {p_vehicle->Genus.Vehicle->DControl = VEH_ACCEL;}
	if (dspeed > +10) {p_vehicle->Genus.Vehicle->DControl = VEH_DECEL;}

	// shift up if gay delinquent person
	if (p_person->Genus.Person->pcom_bent & PCOM_BENT_DILIGENT)
	{
//		p_vehicle->Genus.Vehicle->DControl |= VEH_FASTER;
	}

	//
	// Avoid people.
	//

	if (p_person->Genus.Person->pcom_move_flag & PCOM_MOVE_FLAG_AVOID_LEFT)  {p_vehicle->Genus.Vehicle->Steering = -1;}
	if (p_person->Genus.Person->pcom_move_flag & PCOM_MOVE_FLAG_AVOID_RIGHT) {p_vehicle->Genus.Vehicle->Steering = +1;}

	if (p_person->Genus.Person->pcom_move_flag & (PCOM_MOVE_FLAG_AVOID_LEFT | PCOM_MOVE_FLAG_AVOID_RIGHT))
	{
		SLONG avoid_time;

		if (p_vehicle->Velocity > 700)
		{
			avoid_time = PCOM_get_duration(1);
		}
		else
		{
			avoid_time = PCOM_get_duration(2);
		}

		if (p_person->Genus.Person->pcom_move_counter > avoid_time)
		{
			p_person->Genus.Person->pcom_move_flag &= ~(PCOM_MOVE_FLAG_AVOID_LEFT | PCOM_MOVE_FLAG_AVOID_RIGHT);
		}
	}
/*
	AENG_world_line_infinite(
		p_vehicle->WorldPos.X >> 8,
		p_vehicle->WorldPos.Y >> 8,
		p_vehicle->WorldPos.Z >> 8,
		32,
		0xffffff,
		dest_x,
		0,
		dest_z,
		0,
		0x000000,
		TRUE);
*/
	p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
}

#ifdef BIKE

// ParkBike
//
// AI for parking a bike

void ParkBike(Thing* p_person)
{
	SLONG wangle;
	SLONG dangle;

	BIKE_Control bc;

	SLONG steer;
	SLONG accel;

	//
	// Make sure we're on a bike.
	// 
	
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING);

	Thing* p_bike = TO_THING(p_person->Genus.Person->InCar);

	bc = BIKE_control_get(p_bike);

	steer = bc.steer;
	accel = bc.accel;

	//
	// What direction do we want the bike to face?
	//

	if (p_person->Genus.Person->pcom_move_arg)
	{
		wangle = EWAY_get_angle(p_person->Genus.Person->pcom_move_arg);

		dangle = wangle - p_bike->Genus.Vehicle->Angle;

		if (dangle < -1024) {dangle += 2048;}
		if (dangle > +1024) {dangle -= 2048;}

		if (dangle < -8) {steer += -10;}
		if (dangle > +8) {steer += +10;}
	}

	//
	// Deccelerate.
	//

	if (BIKE_get_speed(p_bike) > 0)
	{
		accel = -20;
	}
	else
	{
		accel = -0;
	}
	
	SATURATE(steer, -127, +127);
	SATURATE(accel, -127, +127);

	bc.accel = accel;
	bc.steer = steer;

	BIKE_control_set(p_bike, bc);
}

// DriveBike
//
// AI for driving a bike

void DriveBike(Thing* p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG what;
	SLONG dest_x;
	SLONG dest_z;
	SLONG wangle;
	SLONG dangle;
	SLONG wspeed;
	SLONG dspeed;
	SLONG dlane;

	BIKE_Control bc;

	SLONG steer;
	SLONG accel;

	//
	// Make sure we are on a bike
	// 
	
	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING);

	if (p_person->SubState != SUB_STATE_RIDING_BIKE)
	{
		//
		// We are getting onto or off the bike.
		//

		p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
		return;
	}

	Thing*	p_bike = TO_THING(p_person->Genus.Person->InCar);

	//
	// Where are we going?
	//

	PCOM_get_person_dest(
		p_person,
	   &dest_x,
	   &dest_z);

	//
	// If we are biking down a road...
	//

	if (p_person->Genus.Person->pcom_move_state == PCOM_MOVE_STATE_BIKE_DOWN)
	{
		//
		// How far are we from the middle of the road?
		//

		dist = ROAD_signed_dist(
				p_person->Genus.Person->pcom_move_arg >> 8,
				p_person->Genus.Person->pcom_move_arg & 0xff,
				p_bike->WorldPos.X >> 8,
				p_bike->WorldPos.Z >> 8);

		dlane = 0x100 - dist;

		if (abs(dlane) > (0x80 + (rand() & 0x7f)))
		{
			//
			// We are quite for from our lane. We should aim for the
			// middle of our lane.
			//

			SLONG rx1;
			SLONG rz1;

			SLONG rx2;
			SLONG rz2;
			
			ROAD_node_pos(
				p_person->Genus.Person->pcom_move_arg >> 8,
			   &rx1,
			   &rz1);

			ROAD_node_pos(
				p_person->Genus.Person->pcom_move_arg & 0xff,
			   &rx2,
			   &rz2);

#ifndef PSX
#ifndef TARGET_DC

			if (ControlFlag&&allow_debug_keys)
			{
				AENG_world_line_infinite(
					rx1, 0, rz1,
					32,
					0x008800,
					rx2, 0, rz2,
					0,
					0x004400,
					FALSE);
			}

#endif
#endif // PSX

			nearest_point_on_line(
				rx1, rz1,
				rx2, rz2,
				p_bike->WorldPos.X >> 8,
				p_bike->WorldPos.Z >> 8,
			   &dest_x,
			   &dest_z);

			SLONG drx = SIGN(rx2 - rx1) << 8;
			SLONG drz = SIGN(rz2 - rz1) << 8;

			dest_x += drz + drx;
			dest_z -= drx - drz;
		}
	}

	//
	// What direction do we want the bike to face?
	//

	dx = dest_x - (p_bike->WorldPos.X >> 8);
	dz = dest_z - (p_bike->WorldPos.Z >> 8);

	wangle  = calc_angle(dx,dz);
	wangle += 1024;
	wangle &= 2047;

	dangle = wangle - p_bike->Draw.Mesh->Angle;

	if (dangle < -1024) {dangle += 2048;}
	if (dangle > +1024) {dangle -= 2048;}

	//
	// What speed do we want to be going at?
	//

	if (abs(dangle) > 300)
	{
		wspeed = 600 - abs((dangle - 300) >> 1);

		if (wspeed < 150)
		{
			wspeed = 150;
		}
	}
	else
	{
		wspeed = 600;
	}

	//
	// Are we going to crash into anyone?
	//

	what = PCOM_find_runover_thing(p_person, -dangle);

	if (what & PCOM_RUNOVER_STOP)       {if (wspeed > 0) {wspeed = 0;}}
	if (what & PCOM_RUNOVER_BEEP_HORN)  {MFX_play_thing(THING_NUMBER(p_person), S_BEEP,MFX_REPLACE, p_person);}
#ifndef PSX
	if (what & PCOM_RUNOVER_SHOUT_OUT)  {MFX_play_thing(THING_NUMBER(p_person), S_GETOUTTHEWAY,MFX_REPLACE, p_person);}
#endif
	if (what & PCOM_RUNOVER_SLOW_DOWN)  {if (wspeed > 0) {wspeed >>= 1;}}

	if (what & PCOM_RUNOVER_TURN_LEFT)  {p_person->Genus.Person->pcom_move_flag |= PCOM_MOVE_FLAG_AVOID_LEFT;  p_person->Genus.Person->pcom_move_counter = 0;}
	if (what & PCOM_RUNOVER_TURN_RIGHT) {p_person->Genus.Person->pcom_move_flag |= PCOM_MOVE_FLAG_AVOID_RIGHT; p_person->Genus.Person->pcom_move_counter = 0;}

	if (what & PCOM_RUNOVER_RUNAWAY)
	{
		//
		// The movement bit is updating the high-level ai- sentience
		// through complexity.
		//

		PCOM_set_person_ai_flee_person(p_person, PCOM_runover_scary_person);

		return;
	}

	//
	// Turn towards our destination.
	//

	bc = BIKE_control_get(p_bike);

	steer = bc.steer;

	if (dangle < -8) {steer += -10;}
	if (dangle > +8) {steer += +10;}

	//
	// Accelerate... decellerate?
	//

	dspeed = wspeed - BIKE_get_speed(p_bike);

	accel  = bc.accel;

	if (dspeed < -10) {accel += -10;}
	if (dspeed > +10) {accel += +10;}

	//
	// Avoid people.
	//

	if (p_person->Genus.Person->pcom_move_flag & PCOM_MOVE_FLAG_AVOID_LEFT)  {steer += +18;}
	if (p_person->Genus.Person->pcom_move_flag & PCOM_MOVE_FLAG_AVOID_RIGHT) {steer += -18;}

	if (p_person->Genus.Person->pcom_move_flag & (PCOM_MOVE_FLAG_AVOID_LEFT | PCOM_MOVE_FLAG_AVOID_RIGHT))
	{
		if (p_person->Genus.Person->pcom_move_counter > PCOM_get_duration(1))
		{
			p_person->Genus.Person->pcom_move_flag &= ~(PCOM_MOVE_FLAG_AVOID_LEFT | PCOM_MOVE_FLAG_AVOID_RIGHT);
		}
	}
	
	SATURATE(steer, -127, +127);
	SATURATE(accel, -127, +127);

	bc.accel = accel;
	bc.steer = steer;

	BIKE_control_set(p_bike, bc);

#ifndef PSX
#ifndef TARGET_DC


	if (ControlFlag&&allow_debug_keys)
	{
		AENG_world_line_infinite(
			p_bike->WorldPos.X >> 8,
			p_bike->WorldPos.Y >> 8,
			p_bike->WorldPos.Z >> 8,
			32,
			0xffffff,
			dest_x,
			0,
			dest_z,
			0,
			0x000000,
			TRUE);
	}

#endif
#endif // PSX

	p_person->Genus.Person->pcom_move_counter += PCOM_TICKS_PER_TURN * TICK_RATIO >> TICK_SHIFT;
}

#endif


SLONG PCOM_if_i_wanted_to_jump_how_fast_should_i_do_it(Thing *p_person)
{
	if (!p_person->Genus.Person->PlayerID)
	{
		switch(p_person->Genus.Person->pcom_move_state)
		{
			case PCOM_MOVE_STATE_GOTO_XZ:
			case PCOM_MOVE_STATE_GOTO_WAYPOINT:
			case PCOM_MOVE_STATE_GOTO_THING:
				
				switch(p_person->Genus.Person->pcom_move_ma.action)
				{
					case MAV_ACTION_JUMP:
					case MAV_ACTION_PULLUP:
					case MAV_ACTION_CLIMB_OVER:
						return 24;
				}
		}
	}

	return 40;
}


void PCOM_make_driver_run_away(Thing *p_driver, Thing *p_scary)
{
	if (p_driver->Genus.Person->pcom_ai_state == PCOM_AI_STATE_LEAVECAR ||
		p_driver->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FLEE_PERSON)
	{
		return;
	}

	if (!(p_driver->Genus.Person->Flags & FLAG_PERSON_DRIVING))
	{
		//
		// Not driving a car!
		//

		PCOM_set_person_ai_flee_person(p_driver, p_scary);
	}
	else
	{
		PCOM_set_person_ai_leavecar(
			p_driver,
			PCOM_EXCAR_FLEE_PERSON,
			0,
			THING_NUMBER(p_scary));
	}
}

