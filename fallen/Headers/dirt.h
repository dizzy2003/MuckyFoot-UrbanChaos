//
// Bits of dirt that get blown around. Dirt only exists
// around one focal point (the camera). If a bit of dirt gets
// too far away, then it teleports to somewhere on the
// edge on the focus point.
//

#ifndef DIRT_H
#define DIRT_H

#define DIRT_TYPE_UNUSED    0
#define DIRT_TYPE_LEAF	    1
#define DIRT_TYPE_CAN	    2
#define DIRT_TYPE_PIGEON    3
#define DIRT_TYPE_WATER	    4
#define DIRT_TYPE_HELDCAN   5
#define DIRT_TYPE_THROWCAN  6
#define DIRT_TYPE_HEAD      7
#define DIRT_TYPE_HELDHEAD  8
#define DIRT_TYPE_THROWHEAD 9
#define DIRT_TYPE_BRASS		10 // 10 was grenade and is now brass (ejected shells)
#define DIRT_TYPE_MINE		11
#define DIRT_TYPE_URINE		12
#define DIRT_TYPE_SPARKS    13
#define DIRT_TYPE_BLOOD     14
#define DIRT_TYPE_SNOW      15
#define DIRT_TYPE_NUMBER    16

typedef struct
{
	UBYTE type;
	UBYTE owner;
	UBYTE flag;
	UBYTE counter;

	union
	{
		struct	
		{
			UBYTE state;	// For pigeons only....
			UBYTE morph1;
			UBYTE morph2;
			UBYTE tween;
		}Pidgeon;
		struct
		{
			UWORD	col;
			UWORD	fade;	// Adding this shouldn't affect the struct size as Pidgeon is already larger...
		}Leaf;

		struct
		{
			UWORD prim;	// Which prim to use as the prim object.

		} Head;

		struct
		{
			UWORD timer;	// How long before we explode?

		} ThingWithTime;
	}UU;

	SWORD dyaw;
	SWORD dpitch;
	SWORD droll;

	SWORD yaw;
	SWORD pitch;
	SWORD roll;

	SWORD x;
	SWORD y;
	SWORD z;

	SWORD dx;
	SWORD dy;	// dy is now shifted up TICK_SHIFT bits to allow for frame-rate independent gravity ;)
	SWORD dz;

} DIRT_Dirt;


//
// Removes all dirt.
//

void DIRT_init(
		SLONG prob_leaf,		// Relative probabilites of each bit of dirt
		SLONG prob_can,
		SLONG prob_pigeon,
		SLONG pigeon_map_x1,	// The bounding box in which only pigeons are made.
		SLONG pigeon_map_z1,
		SLONG pigeon_map_x2,
		SLONG pigeon_map_z2);

//
// Sets the area of focus. If a bit of dirt is out of
// range then it dissapears and reappears somewhere on
// the edge of the focal range.
//

void DIRT_set_focus(										 
		SLONG x,
		SLONG z,
		SLONG radius);

//
// Makes the dirt react to a gust of wind. The gust happens
// at (x1,z1).  (x2,z2) gives strength and direction.
//

void DIRT_gust(
		Thing *p_thing,		// The thing that caused the gust.
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);

//
// Creates a wind blowing through the whole city.
//

void DIRT_gale(
		SLONG dx,
		SLONG dz);

//
// Creates a bit of water dirt.
//

void DIRT_new_water(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,
		SLONG dy,
		SLONG dz,
		SLONG dirt_type = DIRT_TYPE_WATER);


//
// Process the dirt.
//

void DIRT_process(void);


//
// Returns the distance of the nearest coke can. Returns INFINITY
// if it couldn't find a coke can.
//

SLONG DIRT_get_nearest_can_or_head_dist(SLONG x, SLONG y, SLONG z);


//
// Looks for the nearest coke can or head to the given person and attaches
// it to their right hand.
//

void DIRT_pick_up_can_or_head(Thing *p_person);

//
// Releases the given held coke can or head with the given power.
//

void DIRT_release_can_or_head(Thing *p_person, SLONG power);	// 0 <= power <= 256

//
// Beheads the given person. It takes the velocity from the position of
// the attacker.  If attacker == NULL, then a random velocity is given.
//

void DIRT_behead_person(Thing *p_person, Thing *p_attacker);

//
// Creates a grenade thrown by the person with the given velocity and time
// to detonation.  The ticks of the time to detonation is 16*20 ticks per second.
// 

void DIRT_create_grenade(Thing *p_person, SLONG ticks_to_go, SLONG power);

//
// Creates/Destroys a mine.
//

UWORD DIRT_create_mine (Thing *p_person);
void  DIRT_destroy_mine(UWORD dirt_mine);

//
// This person is trying to shoot a bit of dirt. It returns TRUE if
// the person shot anything.
//

SLONG DIRT_shoot(Thing *p_person);


//
// Makes a few newspapers and crisp packets appear and float around
// at the given spot.
//

void DIRT_create_papers(
		SLONG x,
		SLONG y,
		SLONG z);

void DIRT_create_cans(
		SLONG x,
		SLONG z,
		SLONG angle);


//
// The dirt module will tell you about each bit of dirt
// and how to draw it.
//

#ifdef	PSX
#define DIRT_MAX_DIRT (128)			// MAKE SURE ITS A POWER OF 2!
#else
#ifdef TARGET_DC
#define DIRT_MAX_DIRT (256)			// MAKE SURE ITS A POWER OF 2!
#else
#define DIRT_MAX_DIRT (1024)		// MAKE SURE ITS A POWER OF 2!
#endif
#endif

extern DIRT_Dirt DIRT_dirt[DIRT_MAX_DIRT];



#define DIRT_INFO_TYPE_UNUSED 0
#define DIRT_INFO_TYPE_LEAF	  1
#define DIRT_INFO_TYPE_PRIM	  2
#define DIRT_INFO_TYPE_MORPH  3
#define DIRT_INFO_TYPE_WATER  4
#define DIRT_INFO_TYPE_URINE  5
#define DIRT_INFO_TYPE_SPARKS 6
#define DIRT_INFO_TYPE_BLOOD  7
#define DIRT_INFO_TYPE_SNOW	  8

#define DIRT_FLAG_STILL		(1 << 0)
#define DIRT_FLAG_HIT_FLOOR	(1 << 1)
#define DIRT_FLAG_DELETE_OK (1 << 2)

typedef struct
{
	UBYTE type;
	UBYTE held;	// For PRIM types. This prim is being held.
	UBYTE morph1;
	UBYTE morph2;
	UWORD prim;
	UWORD tween; //used as a colour for leafs
	UWORD yaw;
	UWORD pitch;
	UWORD roll;
	SLONG x;
	SLONG y;
	SLONG z;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	
} DIRT_Info;

SLONG DIRT_get_info(SLONG which,DIRT_Info *ans);

//
// Tell the dirt module when a bit of dirt is off-screen.
// This helps the dirt module know which bits of dirt
// to delete.
//

void DIRT_mark_as_offscreen(SLONG which);

#define DIRT_MARK_AS_OFFSCREEN_QUICK(which) {DIRT_dirt[which].flag |= DIRT_FLAG_DELETE_OK;}


void DIRT_new_sparks(SLONG px, SLONG py, SLONG pz, UBYTE dir);


#endif
