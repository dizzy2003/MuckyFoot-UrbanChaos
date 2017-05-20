//
// Person commands and high-level AI
//

#ifndef _PCOM_
#define _PCOM_


//
// The types of high-level AI.
//

#define PCOM_AI_NONE		 0	// Does nowt.
#define PCOM_AI_CIV			 1
#define PCOM_AI_GUARD		 2	// Protects an area- doesn't leave that area.
#define PCOM_AI_ASSASIN		 3	// ai_other is the waypoint that creates the thing to kill.
#define PCOM_AI_BOSS		 4	// Waits for everyone in his group to die and then kills the player
#define PCOM_AI_COP			 5
#define PCOM_AI_GANG		 6	// Attacks player on sight. A bit of a dude with an attitude problem.
#define PCOM_AI_DOORMAN		 7	// Looks for a nearby door and stops anyone going through it.
#define PCOM_AI_BODYGUARD	 8	// ai_other is the waypoint that creates the person to guard.
#define PCOM_AI_DRIVER		 9 	// Looks for a nearby car and starts driving it.
#define PCOM_AI_BDISPOSER	 10	// If he sees a bomb- he'll deactivate it.
#define PCOM_AI_BIKER		 11	// Finds a nearby bike and rides it.
#define PCOM_AI_FIGHT_TEST	 12  // hand to hand combat test person
#define PCOM_AI_BULLY		 13	// Kills anyone who isn't in his gang.
#define PCOM_AI_COP_DRIVER	 14	// A cop who drives a car. Will stop and get out if he sees trouble.
#define PCOM_AI_SUICIDE		 15	// Kills himself!
#define PCOM_AI_FLEE_PLAYER  16	// Makes sure he isn't too close to someone!
#define PCOM_AI_KILL_COLOUR	 17	// Kills everyone of the given colour on the map.
#define PCOM_AI_MIB			 18
#define PCOM_AI_BANE		 19
#define PCOM_AI_HYPOCHONDRIA 20
#define PCOM_AI_SHOOT_DEAD   21	// An assasin who always shoots- and fast too!
#define PCOM_AI_NUMBER		 22

#define PCOM_MOVE_STILL			1
#define PCOM_MOVE_PATROL		2	// Follow waypoints.
#define PCOM_MOVE_PATROL_RAND	3	// Walk in random order.
#define PCOM_MOVE_WANDER		4	// Wanders through the streets.
#define PCOM_MOVE_FOLLOW		5
#define PCOM_MOVE_WARM_HANDS	6	// Look for a nearby bit of fire. Stands around it and warms hands.
#define PCOM_MOVE_FOLLOW_ON_SEE 7
#define PCOM_MOVE_DANCE			8
#define PCOM_MOVE_HANDS_UP		9	// When still- puts his hands up
#define PCOM_MOVE_TIED_UP		10	// When still- tied up
#define PCOM_MOVE_NUMBER		11

#define PCOM_BENT_LAZY				(1 << 0)	// Moves slow
#define PCOM_BENT_DILIGENT			(1 << 1)	// Moves fast
#define PCOM_BENT_GANG				(1 << 2)	// Part of a gang. Protects other gang members of the same colour.
#define	PCOM_BENT_FIGHT_BACK		(1 << 3)
#define PCOM_BENT_ONLY_KILL_PLAYER	(1 << 4)    // Bullies only kill the player- not everyone they see.
#define PCOM_BENT_ROBOT				(1 << 5)	// Ignores everything and everyone
#define PCOM_BENT_RESTRICTED    	(1 << 6)    // Won't climb, jump etc
#define PCOM_BENT_PLAYERKILL		(1 << 7)	// Only the player can hurt this person
#define PCOM_BENT_NUMBER			8


//
// Don't worry about these...
//

//
// The ai states that the person can be in.
// 

#define PCOM_AI_STATE_PLAYER		0	// This is a player
#define PCOM_AI_STATE_NORMAL		1	// Doing nothing in particular.
#define PCOM_AI_STATE_INVESTIGATING	2	// Investigating a strange sound- not sure if there is a problem
#define PCOM_AI_STATE_SEARCHING		3	// Looking for an intruder- definitely something wrong.
#define PCOM_AI_STATE_KILLING		4	// Trying to kill the target.
#define PCOM_AI_STATE_SLEEPING		5
#define PCOM_AI_STATE_FLEE_PLACE	6
#define PCOM_AI_STATE_FLEE_PERSON	7
#define PCOM_AI_STATE_FOLLOWING		8
#define PCOM_AI_STATE_NAVTOKILL		9
#define PCOM_AI_STATE_HOMESICK		10
#define PCOM_AI_STATE_LOOKAROUND	11	// Looking around like a lighthouse for signs of trouble.
#define PCOM_AI_STATE_FINDCAR		12
#define PCOM_AI_STATE_BDEACTIVATE   13
#define PCOM_AI_STATE_LEAVECAR      14
#define PCOM_AI_STATE_SNIPE			15
#define PCOM_AI_STATE_WARM_HANDS	16
#define PCOM_AI_STATE_FINDBIKE		17
#define PCOM_AI_STATE_KNOCKEDOUT	18	// Waits until the person is idle again then resumes normal activity.
#define PCOM_AI_STATE_TAUNT			19	// Taunting someone the person wouldn't mind killing!
#define PCOM_AI_STATE_ARREST		20	// A cop is trying to reason with some naughty people
#define PCOM_AI_STATE_TALK			21
#define PCOM_AI_STATE_GRAPPLED		22	// While being held in a grapple
#define PCOM_AI_STATE_HITCH			23	// Get into a specific vehicle as a passenger
#define PCOM_AI_STATE_AIMLESS		24	// Looking for a car but can't find one!
#define PCOM_AI_STATE_HANDS_UP		25
#define PCOM_AI_STATE_SUMMON		26
#define PCOM_AI_STATE_GETITEM		27	// Going to pick up an item.
#define PCOM_AI_STATE_NUMBER		28

#define PCOM_AI_SUBSTATE_NONE			0
#define PCOM_AI_SUBSTATE_SUPRISED		1
#define PCOM_AI_SUBSTATE_WALKOVER		2
#define PCOM_AI_SUBSTATE_LOOK			3
#define PCOM_AI_SUBSTATE_PUNCHING		4
#define PCOM_AI_SUBSTATE_KICKING		5
#define PCOM_AI_SUBSTATE_LEGIT			6
#define PCOM_AI_SUBSTATE_HUNTING		7
#define PCOM_AI_SUBSTATE_AIMING			8
#define PCOM_AI_SUBSTATE_NOMOREAMMO		9
#define PCOM_AI_SUBSTATE_GOTOCAR		10
#define PCOM_AI_SUBSTATE_GETINCAR		11
#define PCOM_AI_SUBSTATE_GOTOBOMB   	12
#define PCOM_AI_SUBSTATE_CUTWIRES   	13
#define PCOM_AI_SUBSTATE_PARKCAR		14
#define PCOM_AI_SUBSTATE_LEAVECAR		15
#define PCOM_AI_SUBSTATE_GOTOFIRE		16
#define PCOM_AI_SUBSTATE_WARMUP     	17
#define PCOM_AI_SUBSTATE_GOTOBIKE		18
#define PCOM_AI_SUBSTATE_HUNTING_SLIDE	19
#define PCOM_AI_SUBSTATE_TALK_ASK		21
#define PCOM_AI_SUBSTATE_TALK_TELL		22
#define PCOM_AI_SUBSTATE_TALK_LISTEN	23
#define PCOM_AI_SUBSTATE_HITCHING		24
#define PCOM_AI_SUBSTATE_SUMMON_START	25
#define PCOM_AI_SUBSTATE_SUMMON_FLOAT   27
#define PCOM_AI_SUBSTATE_DRAW_H2H		28
#define PCOM_AI_SUBSTATE_CANTFIND		28
#define PCOM_AI_SUBSTATE_WAITING        29
#define PCOM_AI_SUBSTATE_NUMBER			30

//
// The things a person can have.
//

#define PCOM_HAS_GUN         (1 << 0)
#define PCOM_HAS_BALLOON     (1 << 1)
#define PCOM_HAS_SHOTGUN     (1 << 2)
#define PCOM_HAS_BASEBALLBAT (1 << 3)
#define PCOM_HAS_KNIFE       (1 << 4)
#define PCOM_HAS_AK47	     (1 << 5)
#define PCOM_HAS_GRENADE	 (1 << 6)


//
// What somebody can look at
//

#define PCOM_LOOKAT_NOTHING  0
#define PCOM_LOOKAT_THING    1		// lookat_index is thing index
#define PCOM_LOOKAT_WAYPOINT 2		// lookat_index is waypoint index
#define PCOM_LOOKAT_DIRT     3		// lookat_index is dirt index


//
// What makes a fight test dummy die even if he's invulnerable.
//

// THERE IS A NASTY COPY OF THESE DEFINES IN EnemySetup.cpp!

#define PCOM_COMBAT_SLIDE          (1 << 0)
#define PCOM_COMBAT_COMBO_PPP      (1 << 1)
#define PCOM_COMBAT_COMBO_KKK      (1 << 2)
#define PCOM_COMBAT_COMBO_ANY      (1 << 3)
#define PCOM_COMBAT_GRAPPLE_ATTACK (1 << 4)
#define PCOM_COMBAT_SIDE_KICK	   (1 << 5)
#define PCOM_COMBAT_BACK_KICK      (1 << 6)


//
// Call at the start of a new level before creating any new people.
//

void PCOM_init(void);



//
// Creates a person. Returns the THING_INDEX of the new
// person or NULL.
//

THING_INDEX PCOM_create_person(
				SLONG  type,
				SLONG  colour,
				SLONG  group,
				SLONG  ai,
				SLONG  ai_other,	// An extra arguement for some AI types.
				SLONG  ai_skill,
				SLONG  move,
				SLONG  move_follow,	// Index of the waypoint that creates the person to follow for PCOM_MOVE_FOLLOW
				SLONG  bent,
				SLONG  has,
				SLONG  drop,
				SLONG  zone,		// The bottom four bits give the zone patroled.
				SLONG  world_x,
				SLONG  world_y,
				SLONG  world_z,
				SLONG  yaw,
				SLONG  random,
				ULONG	flag1=0,
				ULONG	flag2=0);

THING_INDEX PCOM_create_player(
				SLONG  type,
				SLONG  has,
				SLONG  world_x,
				SLONG  world_y,
				SLONG  world_z,
				SLONG  id,
				SLONG  yaw);

//
// Changes the attributes of a person. If you change the ai to be bodyguard,
// be sure to call PCOM_set_protect_person() to say who he should protect.
//

void PCOM_change_person_attributes(
		Thing *p_person,
		SLONG  colour,
		SLONG  group,
		SLONG  ai,
		SLONG  ai_other,
		SLONG  move,
		SLONG  move_follow,	// Index of the waypoint that creates the person to follow for PCOM_MOVE_FOLLOW
		SLONG  bent,
		SLONG  yaw);

//
// Makes a sound that can alert people.
//

#define PCOM_SOUND_FOOTSTEP		1
#define PCOM_SOUND_UNUSUAL		2	// i.e. a coke can or kick/punch a wall.
#define PCOM_SOUND_HEY			3	// A guard wanting to know who someone is.
#define PCOM_SOUND_ALARM		4
#define PCOM_SOUND_FIGHT		5
#define PCOM_SOUND_GUNSHOT		6
#define PCOM_SOUND_DROP			7	// Darci landing after a drop of one block.
#define PCOM_SOUND_DROP_MED		8	// Darci after a large fall
#define PCOM_SOUND_DROP_BIG		9	// A scream of excrutiating pain after falling too far.
#define PCOM_SOUND_VAN			10
#define PCOM_SOUND_BANG			11	// An explosion going off
#define PCOM_SOUND_MINE			12	// A mine activating
#define PCOM_SOUND_LOOKINGATME	13	// What a bully says to someone to get their attention
#define PCOM_SOUND_WANKER		14	// A thug calling someone a wanker.
#define PCOM_SOUND_DRAW_GUN		15	// Something happens only if the see you because this isn't really a sound!
#define PCOM_SOUND_GRENADE_FLY	16	// A grenade flying through the air.
#define PCOM_SOUND_GRENADE_HIT	17	// A grenade hitting the ground.

void PCOM_oscillate_tympanum(
		SLONG  type,
		Thing *p_person,	// The person who caused the sound.
		SLONG  sound_x,		// The position of the sound.
		SLONG  sound_y,
		SLONG  sound_z,UBYTE store_it=1);


//
// Informs the PCOM ai system that the person has been knocked down
// (i.e. by a car or an explosion or something...)
//

void PCOM_knockdown_happened(Thing *p_person);


//
// Informs the AI that a person has been attacked by a another.
// The attack made contact- i.e. the victim recoiled.
//

void PCOM_attack_happened(
		Thing *p_victim,
		Thing *p_attacker);


void PCOM_attack_happened_but_missed(Thing *p_victim,Thing *p_attacker);

//
// Tells the AI that the person has been put into a grapple.
//

void PCOM_youre_being_grappled(
		Thing *p_victim,
		Thing *p_attacker);

//
// Processes a person.
//

void PCOM_process_person(Thing *p_person);

//
// If this person is jumping while navigating, but he needs to
// releasing 'forwards' to avoid overshooting... this function
// returns FALSE.
//
// All other times, it returns TRUE.
//

SLONG PCOM_jumping_navigating_person_continue_moving(Thing *p_person);

//
// Returns a string describing the state of the given person.
//

CBYTE *PCOM_person_state_debug(Thing *p_person);


//
// Makes the people talk to eachother.
//

void PCOM_make_people_talk_to_eachother(
		Thing *p_person_a,
		Thing *p_person_b,
		UBYTE  is_a_asking_a_question,
		UBYTE  stay_looking_at_eachother,
		UBYTE  make_the_person_talked_at_listen = TRUE);

//
// Makes two people stop talking to eachother.
//

void PCOM_stop_people_talking_to_eachother(
		Thing *p_person_a,
		Thing *p_person_b);


//
// Returns TRUE if a person is doing nothing interesting.  i.e. he
// is happy to talk to somebody or lookat around at something.
//

SLONG PCOM_person_doing_nothing_important(Thing *p_person);


//
// Returns TRUE if person_a does not like person_b... i.e. wants
// to kill him!
//

SLONG PCOM_person_a_hates_b(Thing *p_person_a, Thing *p_person_b);


//
// If the given person is trying to kill/shoot somebody in particular 
// then it returns that person.
//

THING_INDEX PCOM_person_wants_to_kill(Thing *p_person);


//
// Inform person a cop is aiming a gun at you
//

SLONG	PCOM_cop_aiming_at_you(Thing *p_person,Thing *p_cop);

//
// Informs a car driver that he should get out of his car and runaway from someone.
//

void PCOM_make_driver_run_away(Thing *p_driver, Thing *p_scary);


//
// Make the person face someone and do their talk to anim.
//

void PCOM_set_person_ai_talk_to(
		Thing *p_person,
		Thing *p_person_talked_at,
		UBYTE talk_substate,
		UBYTE stay_looking_at_eachother);


//
// If this person were to do a running jump now... how fast would
// he want to jump!!!
//

SLONG PCOM_if_i_wanted_to_jump_how_fast_should_i_do_it(Thing *p_person);


//
// tell near by cops that I'm naughty and that they should arrest me
//

SLONG	PCOM_call_cop_to_arrest_me(Thing *p_person,SLONG store_it);

#endif
