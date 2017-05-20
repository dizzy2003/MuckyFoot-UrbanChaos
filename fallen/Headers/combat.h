#ifndef	COMBAT_H

#define	COMBAT_H	1

//
// Defines
//

#define	HIT_TYPE_GUN_SHOT_H		(1)	
#define	HIT_TYPE_GUN_SHOT_M		(2)	
#define	HIT_TYPE_GUN_SHOT_L		(3)	
#define	HIT_TYPE_PUNCH_H		(4)	
#define	HIT_TYPE_PUNCH_M		(5)	
#define	HIT_TYPE_PUNCH_L		(6)	
#define	HIT_TYPE_KICK_H			(7)	
#define	HIT_TYPE_KICK_M			(8)	
#define	HIT_TYPE_KICK_L			(9)	

#define	HIT_TYPE_GUN_SHOT_PISTOL	(10)	
#define	HIT_TYPE_GUN_SHOT_SHOTGUN	(11)	
#define	HIT_TYPE_GUN_SHOT_AK47		(12)	

#define	MAX_HISTORY		20
#define	MAX_MOVES		16  //must be even


#define	COMBAT_NONE		0
#define	COMBAT_PUNCH	1
#define	COMBAT_KICK		2
#define	COMBAT_KNIFE	3
//
// Structs
//


struct	ComboHistory
{
	UWORD		Owner;
	SBYTE		Power[MAX_MOVES];
	SBYTE		Moves[MAX_MOVES];
	UWORD		Times[MAX_MOVES];
	UBYTE		Result[MAX_MOVES];
	UWORD		LastUsed;
	UBYTE		Index;
	UBYTE		Count;

};


struct	BlockingHistory
{
	UWORD	Owner;				//who this blocking history is for
	UBYTE	Attack[MAX_MOVES];  // attack move performed
	UBYTE	Flags[MAX_MOVES];   // did it hit, did I block it
	UWORD	Times[MAX_MOVES];   // at what time did it occur
	UWORD	Perp[MAX_MOVES];    // who perpetrated the attack
	UWORD	LastUsed;           // when was this whoile structure used 
	UBYTE	Index;
	UBYTE	Count;
	
};

//
// Owner is the person under attack by multiple foes
// This structure has slots for angles that enemies are attacking from
struct	GangAttack
{
	UWORD	Owner;					//who this gang attack is for
	UWORD	Perp[8];          // who is attacking in each of the eight compass points

	UWORD	LastUsed;           // when was this whole structure used 
	UBYTE	Index;
	UBYTE	Count;
};

//
// Data
//

extern	struct ComboHistory combo_histories[MAX_HISTORY];
extern	struct GangAttack gang_attacks[MAX_HISTORY];

//
// Functions
//

extern	SLONG	get_combat_type_for_node(UBYTE current_node);
extern	SLONG	get_anim_and_node_for_action(UBYTE current_node,UBYTE action,UWORD *new_anim);
extern	SLONG	apply_violence(Thing *p_thing);
extern	SLONG	apply_hit_to_person(Thing *p_thing,SLONG angle,SLONG type,SLONG damage,Thing *p_aggressor,struct GameFightCol *fight);


//
// Looks for a target in the given direction relative to the given person.
// Returns a position and angle the person would like to be to have an optimum
// fighting stance against the person.
//

#define FIND_DIR_FRONT		1	// For a frontal attack
#define FIND_DIR_BACK		2	// For a backwards attack
#define FIND_DIR_LEFT		3	// For a left attack
#define FIND_DIR_RIGHT		4	// For a right attack
#define FIND_DIR_TURN_LEFT	5	// For turning to attack a person on your left  from the front
#define FIND_DIR_TURN_RIGHT	6	// For turning to attack a person on your right from the front 

#define	FIND_DIR_MASK		(0xff)

#define	FIND_DIR_DONT_TURN	(1<<10)

SLONG find_attack_stance(
		Thing     *p_person,
		SLONG      attack_direction,
		SLONG      attack_distance, // Desired distance from a person to attack them. 8-bits per mapsquare.
		Thing    **stance_target,
		GameCoord *stance_position,	// 16-bits per mapsquare position at the desired distance.
		SLONG     *stance_angle);

//
// Turns so you are facing someone in the given direction.
//

SLONG turn_to_target(
		Thing *p_person,
		SLONG  find_dir);

//
// Looks for someone in front of you to punch, turns and positions yourself
// to punch them and then does the punch- the same except you kick.
//

SLONG turn_to_target_and_punch(Thing *p_person);
SLONG turn_to_target_and_kick (Thing *p_person);


//
// Finds the best anims for the person to punch or kick someone. If no
// anim is any good the funcion either returns NULL, or, if (flag &
// FUND_BEST_USE_DEFAULT) it returns a default punch/kick.
//

#define FIND_BEST_USE_DEFAULT	(1 << 0)

SLONG find_best_punch(Thing *p_person, ULONG flag);
SLONG find_best_kick (Thing *p_person, ULONG flag);



//
// If somebody is attacking the given person, it returns a pointer to
// the attacker. If there is more than one attacker- it picks the nearest
// and returns him.
//

Thing *is_person_under_attack(Thing *p_person);
Thing *is_anyone_nearby_alive(Thing *p_person);		// The nearest person alive to you.



#endif


