//
// Helicopters.
// wockawockawockawocka
//

#ifndef CHOPPER_H
#define CHOPPER_H

#define MAX_CHOPPERS	4

#define CHOPPER_NONE		0   // zilch, nada
#define CHOPPER_CIVILIAN	1   // basic no-weapons chopper
#define CHOPPER_NUMB		2

#define CHOPPER_substate_idle		0
#define CHOPPER_substate_takeoff	1
#define CHOPPER_substate_landing	2
#define CHOPPER_substate_landed		3
#define CHOPPER_substate_tracking   4
#define CHOPPER_substate_homing     5
#define CHOPPER_substate_patrolling 6


typedef struct
{
	Thing* thing;			// points at its thing. ooer.
	Thing* target;			// tracks this thing.
	GameCoord home;    		// copter's home -- keeps close to here if asked
	ULONG dist;				// generically useful when pathfinding, chasing, etc
	SLONG radius;			// keeps inside this area, then returns to home
	SLONG patrol;			// rotates for patrolling routines
	SLONG spotx,spotz;		// beam target coords
	SLONG spotdx,spotdz;	// beam target deltas
	SLONG dx,dy,dz;			// motion vector
//	SLONG channel;			// audio channel for wockawocka...
	SLONG victim;			// who to track, as an EP
	SWORD rx,ry,rz;			// rotation vector
	UWORD counter;          // various timings
	UWORD rotors;			// rotor rotation
	UWORD rotorspeed;		// and speed
	UWORD speed;			// preferred cruising speed
	UBYTE ChopperType;      // civilian, gunship, etc
	UBYTE rotorprim;        // which prim
	UBYTE substate;			// takeoff, landing, etc
	UBYTE light;			// brightness of beam
	UBYTE since_takeoff;	// How long in the air since takeoff.

	UBYTE padding;			// padding

}Chopper;

typedef Chopper* ChopperPtr;

void   init_choppers(void);
struct Thing *alloc_chopper(UBYTE type);
void   free_chopper (struct Thing *chopper_thing);


//
// The chopper state functions.
//

extern GenusFunctions CHOPPER_functions[CHOPPER_NUMB];

extern StateFunction CIVILIAN_state_function[];

void CHOPPER_init_state(Thing *chopper_thing, UBYTE new_state);

//
// Creates a chopper thing of the given type.  It puts it at the
// given position on the mapwho and puts it into state STATE_INIT.
//

Thing *CHOPPER_create(GameCoord pos, UBYTE type);

//
// Returns the Chopper structure associated with the given chopper thing.
// Returns the DrawMesh structure associted with the given chopper thing.
//

Chopper  *CHOPPER_get_chopper (Thing *chopper_thing);
DrawMesh *CHOPPER_get_drawmesh(Thing *chopper_thing);


#endif
