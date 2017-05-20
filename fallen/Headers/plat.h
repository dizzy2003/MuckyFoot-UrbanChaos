//
// Platforms are moving prims.  You can walk on the bounding box of the
// walkable faces of the prim.
//

#ifndef _PLAT_
#define _PLAT_

//
// The platforms linked from the things.
// 

struct Plat
{
	UBYTE used;
	UBYTE colour;
	UBYTE group;
	UBYTE move;

	UBYTE state;
	UBYTE wspeed;		// The speed you want to go at.
	UBYTE speed;		// The speed you are going at.
	UBYTE flag;
	UWORD counter;		// In millisecs...
	UWORD waypoint;

};


#define RPLAT_MAX_PLATS 32
#define	PLAT_MAX_PLATS		(save_table[SAVE_TABLE_PLATS].Maximum)

#define	TO_PLAT(x)		&PLAT_plat[x]
#define	PLAT_NUMBER(x)	(UWORD)(x-TO_PLAT(0))

extern	Plat  *PLAT_plat; //[PLAT_MAX_PLATS];

//
// So we can have a pointer from the thing structure.
//

typedef struct Plat *PlatPtr;

extern	SLONG PLAT_plat_upto;

//
// Initialises all platform info.
//

void PLAT_init(void);


//
// Looks for the nearest ob to the given location. It removes that ob
// and in its place puts an identical platform.  Returns the THING_NUMBER
// of the new PLAT thing or NULL if the call failed.
//

#define PLAT_MOVE_STILL         0
#define PLAT_MOVE_PATROL		1
#define PLAT_MOVE_PATROL_RAND	2

#define PLAT_FLAG_LOCK_MOVE	   (1 << 0)
#define PLAT_FLAG_LOCK_ROT     (1 << 1)
#define PLAT_FLAG_BODGE_ROCKET (1 << 2)

UWORD PLAT_create(
		UBYTE colour,
		UBYTE group,
		UBYTE move,
		UBYTE flag,
		UBYTE speed,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z);





#endif
