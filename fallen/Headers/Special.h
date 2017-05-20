//	Special.h
//	Guy Simmons, 28th March 1998.

#ifndef	SPECIAL_H
#define	SPECIAL_H


#define SPECIAL_AMMO_IN_A_PISTOL  15
#define SPECIAL_AMMO_IN_A_SHOTGUN  8
#define SPECIAL_AMMO_IN_A_GRENADE  3
#define SPECIAL_AMMO_IN_A_AK47    30


//---------------------------------------------------------------

#define	RMAX_SPECIALS	260
#define	MAX_SPECIALS	(save_table[SAVE_TABLE_SPECIAL].Maximum)

#define	SPECIAL_NONE		 0
#define	SPECIAL_KEY			 1
#define SPECIAL_GUN			 2
#define SPECIAL_HEALTH		 3
#define SPECIAL_BOMB		 4
#define SPECIAL_SHOTGUN		 5
#define SPECIAL_KNIFE		 6
#define SPECIAL_EXPLOSIVES	 7
#define SPECIAL_GRENADE		 8
#define SPECIAL_AK47		 9
#define SPECIAL_MINE		 10
#define SPECIAL_THERMODROID  11
#define SPECIAL_BASEBALLBAT  12
#define SPECIAL_AMMO_PISTOL  13
#define SPECIAL_AMMO_SHOTGUN 14
#define SPECIAL_AMMO_AK47    15
#define SPECIAL_KEYCARD      16
#define SPECIAL_FILE         17
#define SPECIAL_FLOPPY_DISK  18
#define SPECIAL_CROWBAR      19
#define SPECIAL_VIDEO        20
#define SPECIAL_GLOVES       21
#define SPECIAL_WEEDAWAY	 22
#define SPECIAL_TREASURE	 23
#define SPECIAL_CARKEY_RED   24
#define SPECIAL_CARKEY_BLUE  25
#define SPECIAL_CARKEY_GREEN 26
#define SPECIAL_CARKEY_BLACK 27
#define SPECIAL_CARKEY_WHITE 28
#define SPECIAL_WIRE_CUTTER	 29
#define SPECIAL_NUM_TYPES    30

//
// Info about every special
//

#define SPECIAL_GROUP_USEFUL			1
#define SPECIAL_GROUP_ONEHANDED_WEAPON	2
#define SPECIAL_GROUP_TWOHANDED_WEAPON	3
#define SPECIAL_GROUP_STRANGE			4
#define SPECIAL_GROUP_AMMO				5
#define SPECIAL_GROUP_COOKIE			6

typedef struct
{
	CBYTE *name;	// Why not eh?
	UBYTE  prim;
	UBYTE  group;

} SPECIAL_Info;

extern SPECIAL_Info SPECIAL_info[SPECIAL_NUM_TYPES];

#define SPECIAL_SUBSTATE_NONE       0
#define SPECIAL_SUBSTATE_ACTIVATED  1		// For a bomb or a mine or a grenade
#define SPECIAL_SUBSTATE_IS_DIRT    2		// For an activated mine. 'waypoint' 
#define SPECIAL_SUBSTATE_PROJECTILE 3

//---------------------------------------------------------------

typedef struct
{
	COMMON(SpecialType)

	THING_INDEX	NextSpecial,
				OwnerThing;

	UWORD ammo;		// The amount of ammo this thing has or the countdown to going off for an activated mine.

	UWORD waypoint;	// The index of the waypoint that created this special- or NULL
					// if it wasn't created by a waypoint.

					// For an activate MINE in SPECIAL_SUBSTATE_IS_DIRT, this is the index of the DIRT_dirt
					// that is processing the movement of the mine.

	//
	// These are for the thermodroids.
	// 

//	UBYTE home_x;
//	UBYTE home_z;
//	UBYTE goto_x;
//	UBYTE goto_z;

	UWORD counter;
	UWORD timer;	// The countdown timer for grenades. 16*20 ticks per second.

} Special;

typedef Special* SpecialPtr;

//---------------------------------------------------------------

void init_specials(void);

//
// Creates an item.  'waypoint' is the index of the waypoint that created
// the item or NULL if this item was not created by a waypoint.  When the item
// is collected, the waypoint that created the item is notified.
//

Thing *alloc_special(
		UBYTE type,
		UBYTE substate,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		UWORD waypoint);


//
// Removes the special item from a person.
//

void special_drop(Thing *p_special, Thing *p_person);


//
// Returns the special if the person own a special of the given type or
// NULL if the person isn't carrying a special of that type.
//

Thing *person_has_special(Thing *p_person, SLONG special_type);


//
// Giving a person specials.  person_get_item() removes the special from the map and does everything...
//

SLONG should_person_get_item(Thing *p_person, Thing *p_special); // Ignores distance to the special- consider only if that person already has a special like that and whether she can carry it.
void  person_get_item       (Thing *p_person, Thing *p_special);


//
// Primes the given grenade. You have 6 seconds until it blows up.
//

void SPECIAL_prime_grenade(Thing *p_special);

//
// Throws a grenade. The grenade must be being USED and OWNED by a person.
//

void SPECIAL_throw_grenade(Thing *p_special);

/*

// You can't throw mines now. They explode the moment you go near them.

//
// Throws an activated mine. The mine must be in the SpecialList
// of the person.  It creates a bit of MINE dirt, takes the special out
// of the person's SpecialList and puts the special in substate IS_DIRT.
//

void SPECIAL_throw_mine(Thing *p_special);

*/

//
// If this person has some explosives. It primes the explosives and
// places them on the map.
//

void SPECIAL_set_explosives(Thing *p_person);

//---------------------------------------------------------------



#endif
