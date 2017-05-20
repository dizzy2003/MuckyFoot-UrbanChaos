//
// The whole game is based on waypoints!
//

#ifndef _EWAY_
#define _EWAY_


#define EWAY_MAX_CONDS 128
#define EWAY_MAX_WAYS  512
#define EWAY_MAX_EDEFS 150
#define EWAY_MAX_TIMERS 32

#define EWAY_MESS_BUFFER_SIZE 16384
#define EWAY_MAX_MESSES 128

//
// eway ingame runtime flags
//
#define EWAY_FLAG_ACTIVE	(1 << 0)
#define EWAY_FLAG_COUNTDOWN	(1 << 1)		// A countdown to become inactive
#define EWAY_FLAG_DEAD		(1 << 2)		// Don't process any more.
#define EWAY_FLAG_TRIGGERED (1 << 3)
#define EWAY_FLAG_GOTITEM	(1 << 4)
#define EWAY_FLAG_FINISHED	(1 << 5)		// When a conversation is finished, this flag is set in the waypoint.
#define EWAY_FLAG_CLEARED	(1 << 6)
#define EWAY_FLAG_WHY_LOST  (1 << 7)		// This waypoint going active means the level is lost.


// ========================================================
//
// CONDITIONS TO MAKE TRIGGERS GO ACTIVE.
//
// ========================================================

#define EWAY_COND_FALSE			  		0
#define EWAY_COND_TRUE			  		1
#define EWAY_COND_PROXIMITY		  		2	// arg1 is the radius for proximity triggers.
#define EWAY_COND_TRIPWIRE		  		3
#define EWAY_COND_PRESSURE		  		4
#define EWAY_COND_CAMERA		  		5
#define EWAY_COND_SWITCH		  		6
#define EWAY_COND_TIME			  		7	// arg1 is the time for time triggers
#define EWAY_COND_DEPENDENT		  		8	// If another waypoint is active. arg1 is the trigger ID for DEPENDENT conditions.
#define EWAY_COND_BOOL_AND		  		9
#define EWAY_COND_BOOL_OR		  		10
#define EWAY_COND_COUNTDOWN		  		11	// arg1 is the ID of waypoint with dependency. arg2 is hundreths of a second to wait
#define EWAY_COND_COUNTDOWN_SEE	  		12	// A countdown that appears on-screen.
#define EWAY_COND_PERSON_DEAD	  		13	// For person dead triggers, arg1 is the ID of the node that creates the person.
#define EWAY_COND_PERSON_NEAR	  		14	// Some given person nearby.  Person near triggers have the radius in arg2 and ID of the node in arg1. (ID == NULL => Anybody)	
#define EWAY_COND_CAMERA_AT		  		15	// EWAY_DO_CAMERA_WAYPOINT waypoints must have this condition set.
#define EWAY_COND_PLAYER_CUBE	  		16	// The player enters a cube.
#define EWAY_COND_A_SEE_B		  		17	// For A_SEE_B_, arg1 is person doing the seeing, arg2 is person being looked at.
#define EWAY_COND_GROUP_DEAD      		18
#define EWAY_COND_HALF_DEAD       		19	// When a person (creating waypoint given by arg) is half dead...
#define EWAY_COND_PERSON_USED	  		20	// When the player walks up to person (given by arg) and 'uses' them
#define EWAY_COND_ITEM_HELD		  		21	// When the player holds the item created by waypoint 'arg'.
#define EWAY_COND_RADIUS_USED	  		22	// When the player presses the 'use' button inside of the radius- converts automatically to EWAY_COND_PRIM_ACTIVATED if there is a nearby swith or valve.
#define EWAY_COND_PRIM_DAMAGED	  		23	// When the nearest OB gets damaged.
#define EWAY_COND_CONVERSE_END	  		24	// When a conversation is over (arg is the waypoint with the conversation)
#define EWAY_COND_COUNTER_GTEQ	  		25	// When counter 'arg1' is >= 'arg2'
#define EWAY_COND_PERSON_ARRESTED 		26	// When the person (creating waypoint given by arg) is arrested
#define EWAY_COND_PLAYER_CUBOID			27	// Player cube triggers have the x-radius in the arg1 and the z-radius in the arg2.
#define EWAY_COND_KILLED_NOT_ARRESTED	28	// When the person is killed. Arresting the person doesn't count.
#define EWAY_COND_CRIME_RATE_GTEQ		29	// When the crime rate is >= arg1.
#define EWAY_COND_CRIME_RATE_LTEQ		30	// When the crime rate is <= arg1.
#define EWAY_COND_IS_MURDERER			31	// TRUE when the person given by arg1 has killed someone.
#define EWAY_COND_PERSON_IN_VEHICLE		32	// Arg1 is the person, arg2 is the vehicle. If arg2 == NULL, then any vehicle will do!
#define EWAY_COND_THING_RADIUS_DIR		33	// When the person/vehicle given by arg 1 enters the radius (arg2 in 1/4 blocks) and points roughly in the direction of the waypoint... 
#define EWAY_COND_SPECIFIC_ITEM_HELD	34	// When a particular item is held. 'arg1' gives waypoint that creates item.
#define EWAY_COND_RANDOM				35	// arg1 is the dependency waypoint
#define EWAY_COND_PLAYER_FIRED_GUN		36	// Returns TRUE if the player has fired a gun.
#define EWAY_COND_PRIM_ACTIVATED		37	// Looks for a switch or a valve and monitors it... radius in arg1
#define EWAY_COND_DARCI_GRABBED			38	// True if the player has someone in a hold.
#define EWAY_COND_PUNCHED_AND_KICKED    39	// arg1 is the dependency for when to start counting.
#define EWAY_COND_MOVE_RADIUS_DIR		40	// A thing (arg1) in radius pointing in roughtly the right direction.
#define EWAY_COND_AFTER_FIRST_GAMETURN  41	// TRUE from the second gameturn onwards.


typedef struct eway_conddef
{
	UBYTE type;		// EWAY_COND_*
	UBYTE negate;	// TRUE => negate the output of this condition.
	
	UWORD arg1;
	UWORD arg2;

	//
	// For boolean conditions only.
	//

	struct eway_conddef *bool_arg1;
	struct eway_conddef *bool_arg2;

} EWAY_Conddef;

//
// The conditions.
//

typedef struct
{
	UWORD type;
	UWORD arg;
	
} EWAY_Stay;

typedef struct
{
	UBYTE type;
	UBYTE negate;
	UWORD arg1;
	UWORD arg2;

} EWAY_Cond;

typedef struct
{
	UBYTE  type;
	UBYTE  subtype;
	UWORD  arg1;
	UWORD  arg2;

} EWAY_Do;


typedef struct
{
	UWORD id;
	UBYTE colour;
	UBYTE group;
	UBYTE flag;
	UBYTE yaw;
	UWORD timer;		// For going inactive.
	UWORD x;
	SWORD y;
	UWORD z;

	UBYTE index;
	UBYTE ware;				// which warehouse the waypoint is in (or NULL if the waypoint isn't
							// inside a warehouse.  Set when you call EWAY_work_out_which_ones_are_in_warehouses()
	EWAY_Cond ec;
	EWAY_Stay es;
	EWAY_Do   ed;

} EWAY_Way;


// ========================================================
//
// ONCE A WAYPOINT HAS GONE ACTIVE IT BEHAVE IN DIFFERENT WAYS.
//
// ========================================================

#define EWAY_STAY_ALWAYS	 1	// Always stays active
#define EWAY_STAY_WHILE		 2	// Stay active only which the triggering event is still hapenning
#define EWAY_STAY_WHILE_TIME 3	// Like while, but waits some time before going inactive: arg = time (100 = 1 second)
#define EWAY_STAY_TIME		 4	// Stays active for a cerain amount of time: arg = time (100 = 1 second)
#define EWAY_STAY_AFTER_TIME 5	// Stays inactive for a certain amount of time.
#define EWAY_STAY_DIE        6	// Goes inactive and dies.


// ========================================================
//
// WHAT A TRIGGER DOES ONCE IT HAS GONE ACTIVE.
//
// ========================================================

#define EWAY_DO_NOTHING			 0	// arg1 is delay in 10ths of a second
#define EWAY_DO_CREATE_PLAYER	 1
#define EWAY_DO_CREATE_ANIMAL	 2
#define EWAY_DO_CREATE_ENEMY     3
#define EWAY_DO_CREATE_ITEM		 4
#define EWAY_DO_CREATE_VEHICLE	 5	// arg1 is the key to unlock the vehicle. arg2 is who to track (for a helicopter)
#define EWAY_DO_SOUND_ALARM		 6
#define EWAY_DO_CONTROL_DOOR	 7
#define EWAY_DO_EXPLODE			 8  // subtype and arg1 are the parameter to PYRO_construct()
#define EWAY_DO_MESSAGE			 9	// arg1 = the message number- to be defined later by EWAY_set_message() arg2 is the waypoint of who is saying the message or one of the EWAY_MESSAGE_WHO_* defines
#define EWAY_DO_ELECTRIFY_FENCE	 10
#define EWAY_DO_CAMERA_CREATE	 11	// arg1 is speed, arg2 is delay in tenths of a second.
#define EWAY_DO_CAMERA_WAYPOINT  12 // arg1 is speed, arg2 is delay in tenths of a second.
#define EWAY_DO_CAMERA_TARGET	 13
#define EWAY_DO_MISSION_FAIL	 14
#define EWAY_DO_MISSION_COMPLETE 15
#define EWAY_DO_CHANGE_ENEMY     16 // Changes a person's ai,bent,move,group and colour.  arg1 = the ID of the waypoint that creates the person to change.
#define EWAY_DO_CREATE_PLATFORM  17 // arg1 is speed and arg2 contains PLAT_FLAG flags.
#define EWAY_DO_CREATE_BOMB      18
#define EWAY_DO_ACTIVATE_PRIM	 19 // Sets an anim-prim doing an animation. (animation in the subtype)
#define EWAY_DO_SOUND_EFFECT	 20
#define EWAY_DO_NAV_BEACON  	 21 // arg1 is the message number- to be defined later by EWAY_set_message(). arg2 is who to track or 0 if its a position beacon.
#define EWAY_DO_SPOT_FX			 22 // Water fountains and the like
#define EWAY_DO_WATER_SPOUT      23	// Oops... Matt already did it.
#define EWAY_DO_KILL_WAYPOINT    24 // Stops a waypoint being processed.  arg1 is the waypoint to kill.
#define EWAY_DO_OBJECTIVE        25 // Subtype is EWAY_SUBTYPE_OBJECTIVE_*,  arg1 is the message number, arg2 is score / 10
#define EWAY_DO_GROUP_LIFE       26 // Makes all waypoints of its colour/group come alive
#define EWAY_DO_GROUP_DEATH		 27 // Makes all waypoints of its colour/group die
#define EWAY_DO_CONVERSATION     28 // Sets off a cutscene between two people. Subtype is the message number. arg1 is person 1, arg2 is person 2
#define EWAY_DO_INCREASE_COUNTER 29	// Increases counter 'subtype'
#define EWAY_DO_EMIT_STEAM		 30 // Emits steam. Choreography given by arg1. (HI)speed/steps/range(LO) are 6:4:6 of arg2. Subtype gives the direction
#define EWAY_DO_TRANSFER_PLAYER  31 // 'arg1' is the waypoint that has created the person who is the new player.
#define EWAY_DO_AUTOSAVE		 32 //
#define EWAY_DO_CREATE_BARREL    33 // Creates a barrel. Type given by 'subtype'- one of BARREL_TYPE_* from barrel.h
#define EWAY_DO_LOCK_VEHICLE     34 // Locks or unlocks the vehicle. subtype is EWAY_SUBTYPE_VEHICLE_LOCK or EWAY_SUBTYPE_VEHICLE_UNLOCK, and arg1 is the waypoint that created the vehicle.
#define EWAY_DO_GROUP_RESET		 36 // Sets all waypoints of its group/colour to alive and unactivated
#define EWAY_DO_VISIBLE_COUNT_UP 37	// When activated starts counting up a timer
#define EWAY_DO_RESET_COUNTER	 38	// Resets counter subtype. If subtype == 0, all counters are reset.
#define EWAY_DO_CUTSCENE		 39
#define EWAY_DO_CREATE_MIST		 40
#define EWAY_DO_CHANGE_ENEMY_FLG 41 // Changes a person's ai,bent,move,group and colour.  arg1 = the ID of the waypoint that creates the person to change.
#define EWAY_DO_STALL_CAR		 42	// Makes the car unable to move.
#define EWAY_DO_EXTEND_COUNTDOWN 43	// arg1 is the ID of the visible countdown see, arg2 is time in seconds.
#define EWAY_DO_MOVE_THING		 44 // Moves a thing to where the waypoint is, thing given by arg1
#define EWAY_DO_MAKE_PERSON_PEE  45 // Arg1 is the person to pee
#define EWAY_DO_CONE_PENALTIES   46	// Hitting a cone gives you a time penalty.
#define EWAY_DO_SIGN             47 // Flash up sign 'arg1' with flip 'arg2 as defined in panel.h
#define EWAY_DO_WAREFX           48 // Set up the ambience in a warehouse
#define EWAY_DO_NO_FLOOR         49
#define EWAY_DO_SHAKE_CAMERA     50
#define EWAY_DO_AMBIENT_CONV     51
#define EWAY_DO_END_OF_WORLD	 52

#define EWAY_SUBTYPE_ANIMAL_BAT		 1
#define EWAY_SUBTYPE_ANIMAL_GARGOYLE 2
#define EWAY_SUBTYPE_ANIMAL_BALROG   3
#define EWAY_SUBTYPE_ANIMAL_BANE	 4

#define EWAY_SUBTYPE_VEHICLE_VAN		1
#define EWAY_SUBTYPE_VEHICLE_HELICOPTER	2
#define EWAY_SUBTYPE_VEHICLE_BIKE		3
#define EWAY_SUBTYPE_VEHICLE_CAR		4
#define EWAY_SUBTYPE_VEHICLE_TAXI		5
#define EWAY_SUBTYPE_VEHICLE_POLICE		6
#define EWAY_SUBTYPE_VEHICLE_AMBULANCE	7
#define EWAY_SUBTYPE_VEHICLE_MEATWAGON	8
#define EWAY_SUBTYPE_VEHICLE_SEDAN		9
#define EWAY_SUBTYPE_VEHICLE_JEEP		10
#define EWAY_SUBTYPE_VEHICLE_WILDCATVAN 11

#define EWAY_SUBTYPE_CAMERA_TARGET_PLACE 1		// Looks at the position of the waypoint.
#define EWAY_SUBTYPE_CAMERA_TARGET_THING 2		// Looks at a thing created near the waypoint.
#define EWAY_SUBTYPE_CAMERA_TARGET_NEAR	 3		// Looks for an interesting thing nearby.

#define EWAY_SUBTYPE_CAMERA_LOCK_PLAYER	   (1 << 0)	// Stops Darci moving while the camera is active.
#define EWAY_SUBTYPE_CAMERA_LOCK_DIRECTION (1 << 1) // Keeps the direction it starts with.
#define EWAY_SUBTYPE_CAMERA_CANT_INTERRUPT (1 << 2)	// The player can't interrupt this camera.

#define EWAY_SUBTYPE_OBJECTIVE_MAIN       1
#define EWAY_SUBTYPE_OBJECTIVE_SUB	      2
#define EWAY_SUBTYPE_OBJECTIVE_ADDITIONAL 3

#define EWAY_SUBTYPE_STEAM_FORWARD 0
#define EWAY_SUBTYPE_STEAM_UP	   1
#define EWAY_SUBTYPE_STEAM_DOWN    2

#define EWAY_SUBTYPE_VEHICLE_LOCK   0
#define EWAY_SUBTYPE_VEHICLE_UNLOCK 1

#define EWAY_ARG_ITEM_FOLLOW_PERSON		1	// the item is created by a person not where the wpt is
#define EWAY_ARG_ITEM_STASHED_IN_PRIM	2	// the item is hidden inside a prim and created when searched

#define EWAY_MESSAGE_WHO_RADIO	    NULL
#define EWAY_MESSAGE_WHO_STREETNAME 0xffff
#define EWAY_MESSAGE_WHO_TUTORIAL   0xfffe

typedef struct
{
	UBYTE pcom_ai;
	UBYTE pcom_move;
	UBYTE pcom_has;	// look in pcom.h for the PCOM_HAS #defines
	UBYTE drop;		// The SPECIAL_* type that this person drops when he dies.
	UBYTE pcom_bent;
	UBYTE ai_skill;
	UBYTE zone;
	UBYTE padding;
	UWORD follow;	// For PCOM_MOVE_FOLLOW- who they should follow.
	UWORD ai_other;	// For bodyguards and assasins, this is the ID of the waypoint that creates the
					// person you are to protect/kill.  For fight test dummies it is a bitfield for
					// what makes that person die.

} EWAY_Edef;


extern	EWAY_Cond	*EWAY_cond;//[EWAY_MAX_CONDS];
extern	EWAY_Way	*EWAY_way; //[EWAY_MAX_WAYS];
extern	EWAY_Edef	*EWAY_edef; //[EWAY_MAX_EDEFS];
extern	UWORD		*EWAY_timer;//[EWAY_MAX_TIMERS];
extern	CBYTE		**EWAY_mess; //[EWAY_MAX_MESSES];
extern	CBYTE		*EWAY_mess_buffer; //[EWAY_MESS_BUFFER_SIZE];

extern	SLONG   EWAY_cond_upto;
extern	SLONG   EWAY_way_upto;
extern	SLONG   EWAY_edef_upto;
extern	SLONG	EWAY_mess_buffer_upto;
extern	SLONG	EWAY_mess_upto;
extern	SLONG	EWAY_timer_upto;

//
// The number of counters we have.
//

#define EWAY_MAX_COUNTERS 10

//
// Initialises all the waypoints.
//

void EWAY_init(void);

//
// Creating waypoints.
//

void EWAY_create(
		SLONG identifier,				// A unique number that names the waypoint- same as arg for DEPENDENT conditions.
		SLONG colour,
		SLONG group,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG yaw,
		EWAY_Conddef *ecd,
		EWAY_Do      *ed,
		EWAY_Stay    *es,
		EWAY_Edef    *ee,				// Used only for waypoints that create enemies.
		SLONG         unreferenced,		// TRUE => This waypoint is not referred to from any other one.
		SLONG         kludge_index,		// For EWAY_DO_CONVERSATION ... the message number.
		UWORD magic_index);				// For EWAY_DO_CONVERSATION ... the sample identification

//
// Defines the given message.
//

SLONG EWAY_set_message(
		UBYTE  number,
		CBYTE *message);

//
// Call this function once you've created all the waypoints. It remaps
// and precalculates stuff.  The CRIME_RATE_SCORE_MUL is calculated here.
//

void EWAY_created_last_waypoint(void);

//
// Call this function once the MAV_height array and the warehouses have
// been initialised.
//

void EWAY_work_out_which_ones_are_in_warehouses(void);


//
// Processes the waypoints.
//

void EWAY_process(void);

//
// If this functions returns TRUE, then the camera cutscene system
// wants the player so stop being able to move.
//

SLONG EWAY_stop_player_moving(void);

//
// Informs the EWAY module that an item created by the given waypoint															
// has been picked up by the player.
//

void EWAY_item_pickedup(SLONG waypoint);


//
// Returns the position of the given waypoint.
// 

void EWAY_get_position(
		SLONG  waypoint,
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z);

//
// Returns the delay associated with the waypoint. If that waypoint
// does not have a delay- it return the default value given.
//
// Values are in milliseconds.
//

SLONG EWAY_get_delay(SLONG waypoint, SLONG default_delay);

//
// Returns the angle associated with the given waypoint.
// Returns the person associated with the given waypoint or NULL if there is no such person.
// Returns the warehouse the waypoint is in or NULL if the waypoint isn't in a warehouse.
//

UWORD EWAY_get_angle    (SLONG waypoint);
UWORD EWAY_get_person   (SLONG waypoint);
UBYTE EWAY_get_warehouse(SLONG waypoint);


//
// Returns the index of the first waypoint of the given
// group and colour whose index is >= to the given index. The
// search wraps around.
//
// Returns EWAY_NO_MATCH if it couldn't find a waypoint
// of the given group/colour/do.
//
// To find the first waypoint pass an index of EWAY_FIND_FIRST
//

#define EWAY_DONT_CARE	(-1)
#define EWAY_NO_MATCH	(-1)
#define EWAY_FIND_FIRST (0xffff)

SLONG EWAY_find_waypoint(
		SLONG index,
		SLONG whatdo,	// od EWAY_DONT_CARE
		SLONG colour,	// or EWAY_DONT_CARE
		SLONG group,	// or EWAY_DONT_CARE
		UBYTE only_active);


//
// Returns the index of a random waypoint with the given group / colour.
// It won't return the given wapoint index.
//

SLONG EWAY_find_waypoint_rand(
		SLONG not_this_index,
		SLONG colour,	// or EWAY_DONT_CARE
		SLONG group,	// or EWAY_DONT_CARE
		UBYTE only_active);

//
// Finds the nearest waypoint of the given (group/colour)
// to the given spot.
//
// Returns EWAY_NO_MATCH if it couldn't find a waypoint
// of the given group/colour
// 

SLONG EWAY_find_nearest_waypoint(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG colour,	// or EWAY_DONT_CARE
		SLONG group);	// or EWAY_DONT_CARE

//
// Returns TRUE if the EWAY module wants to capture the camera.
// It so, then it gives the camera aswell.
// 

SLONG EWAY_grab_camera(
		SLONG *cam_x,
		SLONG *cam_y,
		SLONG *cam_z,
		SLONG *cam_yaw,
		SLONG *cam_pitch,
		SLONG *cam_roll,
		SLONG *cam_lens);	// lens in 16-bit fixed point

//
// Returns the index of the warehouse the EWAY_camera should be inside
// or NULL if the camera is outside.
//

UBYTE EWAY_camera_warehouse(void);


//
// Returns whether or not the given waypoint is active or not.
//

SLONG EWAY_is_active(SLONG waypoint);


//
// Tells the waypoint system that the player has pressed 'action' near
// a person whose 'PERSON_FLAG_USEABLE' flag is set.  Returns TRUE if
// the IS_PERSON_USED waypoint is now going to trigger off a message.
//

SLONG EWAY_used_person(UWORD t_index);


//
// Returns TRUE if a conversation is in progress. It tells you
// who is speaking to eachother too.
//

SLONG EWAY_conversation_happening(
		THING_INDEX *person_a,
		THING_INDEX *person_b);


//
// Tells the waypoint system that the player has activated a switch
// or a valve.
//

void EWAY_prim_activated(SLONG ob_index);


//
// Tells the waypoint system to deduct a time penalty to all
// active visible countdown timers.
//

void EWAY_deduct_time_penalty(SLONG time_to_deduct_in_hundreths_of_a_second);


// ========================================================
// 
// SO THE WAYPOINT SYSTEM KNOWS WHICH MOVES DARCI HAS DONE.
//
// ========================================================

//
// OR in these bits when Darci does one of the moves.
//

#define EWAY_DARCI_MOVE_PUNCH (1 << 0)
#define EWAY_DARCI_MOVE_KICK  (1 << 1)

extern UBYTE EWAY_darci_move;



// ========================================================
//
// THE MESSAGES THAT FAKE WANDERING PEOPLE SAY
//
// ========================================================

//
// Call only after you've called EWAY_set_message() as much
// as you want to!
//

void EWAY_load_fake_wander_text(CBYTE *fname);

//
// Returns one of the random texts loaded in.
// 

#define EWAY_FAKE_MESSAGE_NORMAL  0
#define EWAY_FAKE_MESSAGE_ANNOYED 1
#define EWAY_FAKE_MESSAGE_GUILTY  2

CBYTE *EWAY_get_fake_wander_message(SLONG type);

// ========================================================
//
// THE TUTORIAL HELP MESSAGES
//
// ========================================================

//
// When the waypoint system triggers a tutorial help message,
// this pointer is set from NULL to the tutorial string.  You
// must set it back to NULL yourself.
//

extern CBYTE *EWAY_tutorial_string;
extern SLONG  EWAY_tutorial_counter;	// Set to 0 when EWAY_tutorial_string is set... so you can tell how long its been when the message triggered.



// ========================================================
//
// USING THE EWAY CAMERA YOURSELF
//
// ========================================================

//
// Makes the eway camera look at the given thing and stops the player
// from moving.  It picks a nice camera angle for it.  It makes the
// EWAY module grab the camera from FC.
//

void EWAY_cam_look_at(Thing *p_thing);
void EWAY_cam_converse(Thing *p_thing, Thing *p_listener);

//
// Relinquishes control to the FC cam and lets the player move again.
//

void EWAY_cam_relinquish(void);



// ========================================================
//
// NASTY HACK!
//
// ========================================================

//
// Looks for a waypoint that created the given person and if there
// isn't one, makes one up!
//

SLONG EWAY_find_or_create_waypoint_that_created_person(Thing *p_person);


#endif





