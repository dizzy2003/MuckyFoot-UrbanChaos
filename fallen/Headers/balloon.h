//
// Balloons.
//

#ifndef _BALLOON_
#define _BALLOON_



//
// The points of a balloon.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;
	SLONG dy;
	SLONG dz;

} BALLOON_Point;

#define BALLOON_POINTS_PER_BALLOON 4


#define BALLOON_TYPE_UNUSED	0
#define BALLOON_TYPE_YELLOW 1
#define BALLOON_TYPE_RED    2
#define BALLOON_TYPE_NUMBER 3

typedef struct
{
	UBYTE type;
	UBYTE next;		// The next balloon in the linked list of balloons attached to this person.
	UWORD yaw;		// 0xffff => This balloon is unused.
	UWORD pitch;
	UWORD thing;	// The thing this balloon is attached to.
	BALLOON_Point bp[BALLOON_POINTS_PER_BALLOON];

} BALLOON_Balloon;

#define BALLOON_MAX_BALLOONS 32

extern BALLOON_Balloon *BALLOON_balloon;//[BALLOON_MAX_BALLOONS];
extern SLONG           BALLOON_balloon_upto;



//
// Deletes all the balloons.
//

void BALLOON_init(void);


//
// Attaches a balloon to a thing.
// 

UBYTE BALLOON_create(
		UWORD thing,
		UBYTE type);

//
// Releases the balloon. When the balloon has floated up enough it
// dissapears and a new person can use it.
//

void BALLOON_release(UBYTE balloon);

//
// Sees if there is a spare balloon for the given person to grab.
// If there is it makes the person grab it.
//

void BALLOON_find_grab(UWORD thing);


//
// Processes all the balloons.
//

void BALLOON_process(void);



#endif
