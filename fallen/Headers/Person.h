// Person.h
// Guy Simmons, 12th January 1998

#ifndef	PERSON_H
#define	PERSON_H

//---------------------------------------------------------------

#define	RMAX_PEOPLE	180 //150
#define	MAX_PEOPLE	(save_table[SAVE_TABLE_PEOPLE].Maximum)

#define	ANIM_TYPE_DARCI		0
#define	ANIM_TYPE_ROPER		1
#define	ANIM_TYPE_ROPER2	2
#define	ANIM_TYPE_CIV		3
//#define	ANIM_TYPE_VAN		5

/*

#define	PERSON_NONE				100
#define	PERSON_DARCI			0
#define	PERSON_ROPER			1
#define	PERSON_COP				2
#define	PERSON_THUG				3
#define	PERSON_CIV				4
#define	PERSON_CIV2				5
#define	PERSON_BOSS				6
#define	PERSON_SOLDIER			7
#define	PERSON_WORKMAN			8
#define	PERSON_THUG2			9

*/

#define PERSON_NONE			100	// For debug...
#define PERSON_DARCI		0
#define PERSON_ROPER		1
#define PERSON_COP			2
#define PERSON_CIV			3
#define PERSON_THUG_RASTA	4
#define PERSON_THUG_GREY	5
#define PERSON_THUG_RED		6
#define PERSON_SLAG_TART	7
#define PERSON_SLAG_FATUGLY	8
#define PERSON_HOSTAGE		9
#define PERSON_MECHANIC		10
#define PERSON_TRAMP		11
#define PERSON_MIB1			12
#define PERSON_MIB2			13
#define PERSON_MIB3			14
#define PERSON_NUM_TYPES	15	// Number of people types.





#define	FLAG_PERSON_NON_INT_M			(1<<0)
#define	FLAG_PERSON_NON_INT_C			(1<<1)
#define	FLAG_PERSON_LOCK_ANIM_CHANGE	(1<<2)
#define	FLAG_PERSON_GUN_OUT				(1<<3)
#define	FLAG_PERSON_DRIVING				(1<<4)	// InCar is THING_INDEX of bike being ridden.
#define	FLAG_PERSON_SLIDING				(1<<5)  // Sliding tackle
#define	FLAG_PERSON_NO_RETURN_TO_NORMAL	(1<<6)	// After doing your talk anim, don't return to normal processing
#define	FLAG_PERSON_HIT_WALL			(1<<7)
#define	FLAG_PERSON_USEABLE				(1<<8)	// If player presses use near this person, something happens!
#define	FLAG_PERSON_REQUEST_KICK		(1<<9)
#define	FLAG_PERSON_REQUEST_JUMP		(1<<10)
#define	FLAG_PERSON_NAV_TO_KILL			(1<<11)
#define	FLAG_PERSON_ON_CABLE			(1<<12)
#define FLAG_PERSON_GRAPPLING			(1<<13)	// Carrying the grappling hook.
#define FLAG_PERSON_HELPLESS			(1<<14)	// Incapable of doing any AI
#define FLAG_PERSON_CANNING				(1<<15)	// Carrying a coke-can
#define	FLAG_PERSON_REQUEST_PUNCH		(1<<16)
#define	FLAG_PERSON_REQUEST_BLOCK		(1<<17)
#define	FLAG_PERSON_FALL_BACKWARDS		(1<<18)
#define FLAG_PERSON_BIKING				(1<<19)	// InCar is THING_INDEX of bike being ridden.
#define FLAG_PERSON_PASSENGER			(1<<20)	// InCar is the THING_INDEX of the vehicle we are a passenger in.
#define FLAG_PERSON_HIT					(1<<21) // This person has been hit!
#define FLAG_PERSON_SPRINTING			(1<<22)
#define FLAG_PERSON_FELON				(1<<23)	// This person is wanted by the police

#define FLAG_PERSON_PEEING				(1<<24)
#define FLAG_PERSON_SEARCHED			(1<<25)	
#define FLAG_PERSON_ARRESTED			(1<<26)
#define FLAG_PERSON_BARRELING			(1<<27) // Holding a barrel.
#define FLAG_PERSON_MOVE_ANGLETO		(1<<28) // 
#define FLAG_PERSON_KO					(1<<29) // 
#define FLAG_PERSON_WAREHOUSE			(1<<30)	// This person is inside a warehouse given by their "Ware" field.
#define FLAG_PERSON_KILL_WITH_A_PURPOSE	(1<<31)	// An enemy is killing someone with ulterior motives.

#define	FLAG2_PERSON_LOOK				(1<<0) // This person is looking arround
#define FLAG2_SYNC_SOUNDFX				(1<<1) // Set once a sound has started playing for that anim
#define	FLAG2_PERSON_GUILTY				(1<<2)
#define FLAG2_PERSON_INVULNERABLE		(1<<3)
#define FLAG2_PERSON_IS_MURDERER		(1<<4)
#define FLAG2_PERSON_FAKE_WANDER		(1<<5)
#define FLAG2_PERSON_HOME_IN_WAREHOUSE	(1<<6)	// This person's (HomeX,HomeZ) is inside a warehouse
#define FLAG2_PERSON_CARRYING			(1<<7)	// This person's (HomeX,HomeZ) is inside a warehouse


//---------------------------------------------------------------

#define	PTIME(p)	(p->Genus.Person->GTimer)

typedef struct
{
	COMMON(PersonType)

	UBYTE	Action;
	UBYTE	SubAction;
	UBYTE	Ammo;
	UBYTE	PlayerID;		//4

	SWORD	Health;
	UWORD	Timer1;			//8

	THING_INDEX	Target;
	THING_INDEX	InWay;		//12

	THING_INDEX	InCar;				// Either a van/car or a bike depending on whether PERSON_FLAG_DRIVING or PERSON_FLAG_BIKING is set
	UWORD	NavIndex;		//16

	THING_INDEX	SpecialList;		// A linked list of special_things that the person is carrying.
	THING_INDEX SpecialUse;			// The special Darci is using at the moment. NULL => Nothing or pistol
	THING_INDEX SpecialDraw;		// While drawing an item- this is the special that you are getting out.
	UBYTE	pcom_colour;
	UBYTE   pcom_group;		//20


	MAV_Action	MA;			//24	//do we need this?

	SWORD	NavX,NavZ;		//28

	ULONG   sewerbits;		//32

	UBYTE	Power;			
	UBYTE	GTimer;
	UBYTE	Mode;
	UBYTE	GotoSpeed;		//36

	UWORD	HomeX;
	UWORD	HomeZ;			//40

	UWORD	GotoX;
	UWORD	GotoZ;			//44

	UWORD	muckyfootprint;			// Footprint status
	SWORD	Hold;			//48	// The index of the coke can, head or barrel you are holding.

	UWORD	UnderAttack;			// 0 => You are not under attack.
	SBYTE	Shove;					// Shove's a circling person one way or the other.
	UBYTE	Balloon;		//52	// The index of the balloon attached to this person's left hand...

	UWORD	OnFacet;
	UBYTE	BurnIndex;				// 1-based index to the pyro which is immolating the person
	SBYTE	CombatNode;		//56	// index into fight_tree,  your position in the web of available moves/combos

	// Inventory (total of 4xSLONG)
/*
	UBYTE	RightHandSlot;
	UBYTE	LeftHandSlot;

	UBYTE	ArmourSlot;
	UBYTE	BackpackSlots[5];
	UWORD	AmmoBullets;		// for any gun

	UWORD	AmmoRockets;		
	UWORD	AmmoGrenades;		

	UWORD	AmmoFuel;			// chainsaws, flamethrowers, etc
*/

	//
	// High-level AI
	//

	UBYTE   pcom_bent;
	UBYTE	pcom_ai;
	UBYTE	pcom_ai_state;
	UBYTE	pcom_ai_substate;	//60

	UWORD	pcom_ai_counter;
	UWORD	pcom_ai_arg;		//64

	UWORD   pcom_ai_other;				// Another variable you might need- the client for a bodyguard.
	UBYTE   pcom_ai_excar_state;
	UBYTE   pcom_ai_excar_substate; //68

	UWORD	pcom_ai_excar_arg;
	UBYTE   pcom_move;
	UBYTE	pcom_move_state;		//72

	UBYTE	pcom_move_substate;
	UBYTE	pcom_move_flag;
	UWORD	pcom_move_counter;		//76

	UWORD	pcom_move_arg;
	UWORD   pcom_move_follow;		//80 // The waypoint that creates person to follow for PCOM_MOVE_FOLLOW

	MAV_Action pcom_move_ma;		//84
/*
	#define PCOM_MESS_TYPE_SAY		1
	#define PCOM_MESS_TYPE_THOUGHT	2

	CBYTE  *pcom_mess;			// What this person is thinking or saying.

	UWORD   pcom_mess_timer;
	UWORD	pcom_mess_type;
*/
	UBYTE	AnimType;
	UBYTE	InsideRoom;
	UWORD	InsideIndex;			//88

	UWORD	pcom_ai_memory;			// Remembering someone important in your life- someone to kill or flee from.
	UBYTE	HomeYaw;
	UBYTE	Stamina;				//92

	UBYTE	AttackAngle;
	UBYTE	Escape;
	UBYTE	Ware;							// The warehouse this person is in.
	UBYTE   FightRating;			//96	// bit field
	SWORD	TargetX;
	SWORD	TargetZ;				//100

	SWORD	Agression;
	UBYTE	ammo_packs_pistol;
	UBYTE	GangAttack;				//104

//	UWORD	MorePadding;
	UBYTE	ammo_packs_shotgun;
	UBYTE	ammo_packs_ak47;
	UBYTE	drop;					// What this person drops when they die
	UBYTE	pcom_zone;				// For guards this is the zone they patrol or where they are restricted to

	UBYTE	pcom_lookat_what;		// PCOM_LOOKAT_*
	UBYTE	pcom_lookat_counter;	// How long before we stop looking at the interesting thing
	UWORD	pcom_lookat_index;		// The index of what you are looking at

	UWORD	Passenger;				// A linked list for passengers in a vehicle.
	UBYTE	Flags2;
	UBYTE	SlideOdd;				// A counter for how many consecutive gameturns this person has slid along something that isn't a wall or fence.

// using BUILD_PSX means that it'll be commented out both on the PSX, *AND* when Mike builds 
// PSX nads on his PC.
#ifndef BUILD_PSX
	GameCoord GunMuzzle;
#endif

} Person;

//
// how skillful this enemy is
//
#define	GET_SKILL(p)	((p)->Genus.Person->FightRating&0xf)
#define	SET_SKILL(p,s)	(p)->Genus.Person->FightRating=(((p)->Genus.Person->FightRating&0xf0)|s)

//
// how many people can attack at once
//
#define	GET_SKILL_GROUP(p)		(((p)->Genus.Person->FightRating&0x30)>>4)
#define	SET_SKILL_GROUP(p,s)	(p)->Genus.Person->FightRating=(((p)->Genus.Person->FightRating&(~0x30))|(s<<4))

//
// 2 bits I don't know what
//
#define	GET_SKILL_OTHER(p)		(((p)->Genus.Person->FightRating&0xc0)>>4)
#define	SET_SKILL_OTHER(p,s)	(p)->Genus.Person->FightRating=(((p)->Genus.Person->FightRating&(~0xc0))|(s<<6))


//
// stamina 
//

typedef	Person *PersonPtr;

//---------------------------------------------------------------
extern	SWORD	health[];

extern	GenusFunctions		people_functions[];
extern	StateFunction		generic_people_functions[];

void		init_persons(void);
Thing		*alloc_person(UBYTE type, UBYTE random_number);
void		free_person(Thing *person_thing);
THING_INDEX	create_person(
				SLONG type,
				SLONG random_number,
				SLONG x,
				SLONG y,
				SLONG z);


//
// Flags for set_person_drop_down().
//

#define PERSON_DROP_DOWN_KEEP_VEL	(1 << 0)		// Dont set velocity to backwards
#define PERSON_DROP_DOWN_OFF_FACE	(1 << 1)		// Falling off a face- so don't grab again.
#define PERSON_DROP_DOWN_KEEP_DY	(1 << 2)		// Don't set DY to falling downwards.
#define PERSON_DROP_DOWN_QUEUED		(1 << 3)		// Dont set velocity to backwards

//
// Modes a person can be in.
//

#define PERSON_MODE_RUN		0
#define PERSON_MODE_WALK	1
#define PERSON_MODE_SNEAK	2
#define PERSON_MODE_FIGHT	3
#define PERSON_MODE_SPRINT	4
#define PERSON_MODE_NUMBER	5

extern CBYTE *PERSON_mode_name[PERSON_MODE_NUMBER];

//
// Person speeds to set_person_goto_xz
//

#define PERSON_SPEED_WALK	1
#define PERSON_SPEED_RUN	2
#define PERSON_SPEED_SNEAK	3
#define PERSON_SPEED_SPRINT	4
#define PERSON_SPEED_YOMP	5
#define PERSON_SPEED_CRAWL	6


void	set_anim(Thing *p_person,SLONG anim);
void	tween_to_anim(Thing *p_person,SLONG anim);
void	queue_anim(Thing *p_person,SLONG anim);
void	set_person_draw_gun(Thing *p_person);
void	set_person_shoot(Thing *p_person,UWORD shoot_target);
void	set_person_gun_away(Thing *p_person);
void	set_person_flip(Thing *p_person,SLONG dir);
void	set_person_idle(Thing *p_person);
SLONG	set_person_punch(Thing *p_person);	// Automatically slashes if you have a knife out
SLONG	set_person_kick(Thing *p_person);
void	set_person_standing_jump(Thing *p_person);
void	set_person_running_jump(Thing *p_person);
void	set_person_pulling_up(Thing	*p_thing);
void	set_person_drop_down(Thing	*p_person,SLONG flag);
void	set_person_climb_ladder(Thing *p_person,UWORD storey);
void	set_thing_velocity(Thing *t_thing,SLONG vel);
void	set_person_running(Thing *p_person);
void	set_person_walking(Thing *p_person);
void	set_person_step_left(Thing *p_person);
void	set_person_step_right(Thing *p_person);
void	set_person_enter_vehicle(Thing *p_person,Thing *p_vehicle, SLONG door);
void	set_person_exit_vehicle(Thing *p_person);
void	set_person_mount_bike(Thing *p_person, Thing *p_bike);
void	set_person_dismount_bike(Thing *p_person);
void	set_person_grappling_hook_pickup(Thing *p_person);
void	set_person_grappling_hook_release(Thing *p_person);
void	set_person_can_pickup(Thing *p_person);					// Bends down to pickup a coke can or a head
void	set_person_can_release(Thing *p_person, SLONG power);	// Throws a coke can or a head. power = 0 - 256
void	set_person_special_pickup(Thing *p_person);				// Bends down to pick up a special
void	set_person_barrel_pickup(Thing *p_person);
void	set_person_recoil(Thing *p_person,SLONG anim,UBYTE flags);
void	set_person_goto_xz(Thing *p_person, SLONG x, SLONG z, SLONG speed);	// No mavigation- just walks there.
void	set_person_circle(Thing *p_person, Thing *p_target);	// Makes the person circle around someone.
void	general_process_person(Thing *p_person);
void	set_person_dead_normal(Thing *p_thing,Thing *p_aggressor,SLONG death_type,SLONG anim);
SLONG   set_person_land_on_fence(Thing *p_person,SLONG wall,SLONG set_pos,SLONG while_walking=0);

//
// Makes a person start/stop floating...
//

void set_person_float_up  (Thing *p_person);
void set_person_float_down(Thing *p_person);	// Only call when someone is already floating!


//
// This person is doing an animation initiated by PCOM_set_person_move_animation().
// All the state does is wait for the animation to finish and then go idle.
//

void set_person_do_a_simple_anim(Thing *p_person, SLONG anim);

//
// The person only draws the special if she has a special of that type.
//

void set_person_draw_item(Thing *p_person, SLONG special_type);
void set_person_item_away(Thing *p_person);
void set_person_standing_jump_forwards(Thing *p_person);
void set_person_standing_jump_backwards(Thing *p_person);
void set_person_walk_backwards(Thing *p_person);
void set_person_traverse(Thing	*p_person,SLONG right);
void set_person_fight_step(Thing *p_person,SLONG dir);
void set_person_block(Thing *p_person);
void set_person_croutch(Thing *p_person);
void set_person_crawling(Thing *p_person);
void set_person_idle_croutch(Thing *p_person);
void set_person_idle_uncroutch(Thing *p_person);
void set_person_ko_recoil(Thing *p_person,SLONG anim,UBYTE flags);

//
// Once a person is knocked out and lying on the ground.
//

#define PERSON_ON_HIS_FRONT		0
#define PERSON_ON_HIS_BACK		1
#define PERSON_ON_HIS_SOMETHING 2

SLONG person_is_lying_on_what(Thing *p_person);

//
// Makes a person sit down on a bench!
//

void set_person_sit_down(Thing *p_person);


//
// Entering a vehicle where you are just going to be a passenger.
// Door is the side of the car to get into.
//

void set_person_passenger_in_vehicle(Thing *p_person, Thing *p_vehicle, SLONG door);


//
// Sets the person sliding towards the given target.
//

void set_person_sliding_tackle(Thing *p_person, Thing *p_target);


//
// Makes the person turn slightly towards the target.
//

void turn_towards_thing(Thing *p_person, Thing *p_target);


//
// Kills the person in one of various ways!
//

#define PERSON_DEATH_TYPE_COMBAT     	   1
#define PERSON_DEATH_TYPE_LAND	     	   2	// Dying after falling off a high building. 'height' and 'behind' ignored
#define PERSON_DEATH_TYPE_OTHER		 	   3	// 'height' is ignored.
#define PERSON_DEATH_TYPE_STAY_ALIVE 	   4	// Gets up again! Not really dead!
#define PERSON_DEATH_TYPE_LEG_SWEEP	 	   5	// Gets up again! Not really dead!
#define PERSON_DEATH_TYPE_PRONE			   6
#define PERSON_DEATH_TYPE_STAY_ALIVE_PRONE 7	// Gets up again! Not really dead!
#define PERSON_DEATH_TYPE_COMBAT_PRONE     8
#define PERSON_DEATH_TYPE_SHOT_PISTOL      10
#define PERSON_DEATH_TYPE_SHOT_SHOTGUN     11
#define PERSON_DEATH_TYPE_SHOT_AK47    	   12
#define PERSON_DEATH_TYPE_GET_DOWN		   13	// Gets up again! Not really dead!

void set_person_dead(
		Thing *p_thing,
		Thing *p_aggressor,
		SLONG  death_type,
		SLONG  behind,
		SLONG  height);

//
// Makes a person immediately lie down on the ground injured.
// He is DEAD as far as the game is concerned...
//

void set_person_injured(Thing *p_person);


//
// Takes off 'hitpoints' from a person's health and knocks them down
// as if they'd taken a hit originiating from the given place.  They either
// die or get back up again.
//

void knock_person_down(
		Thing *p_person,
		SLONG  hitpoints,
		SLONG  origin_x,
		SLONG  origin_z,
		Thing *p_aggressor);	// or NULL if you don't know who's responsible

//
// Makes 'a' look at 'b'
//

SLONG set_face_thing(Thing *p_person_a,Thing *p_person_b);

//
// Makes the person look at the given place.
//

void set_face_pos(
		Thing *p_person,
		SLONG  world_x,
		SLONG  world_z);

//
// Returns what a person is standing on...
//

#define PERSON_ON_DUNNO	  0
#define PERSON_ON_WATER	  1
#define PERSON_ON_PRIM	  2
#define PERSON_ON_GRAVEL  3
#define PERSON_ON_WOOD	  4
#define PERSON_ON_GRASS	  5
#define PERSON_ON_METAL   6
#define PERSON_ON_SEWATER 7

SLONG person_is_on(Thing *p_person);


//
// Obvious really. Take into account distance, FOV and LOS.
//

SLONG can_i_see_player(Thing *p_person);
SLONG can_a_see_b     (Thing *p_person_a, Thing *p_thing_b,SLONG range=0,SLONG no_los=0);	// 'b' needn't be a person
SLONG can_i_see_place (Thing *p_person, SLONG x, SLONG y, SLONG z);


//
// Relative angle of the target from the person.
//

SLONG get_dangle(Thing *p_person, Thing *p_target);


//
// Works out how quickly a person's aim should improve depending
// on the distance to the target.
//

SLONG calc_dist_benefit_to_gun(SLONG dist);


// (JCL) return scale value for engine
SLONG	person_get_scale(Thing *t);


//---------------------------------------------------------------

#endif
