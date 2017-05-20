//
// A thrust ship.
//

#ifndef _SHIP_
#define _SHIP_


//
// The size of the arena the ships are in.
//

#define SHIP_ARENA (32.0F)



//
// The ship flags.
// 

#define SHIP_FLAG_USED				(1 << 0)
#define SHIP_FLAG_ACTIVE            (1 << 1)	// If a ship is not active, then it is in the process of joining the game.
#define SHIP_FLAG_LOCAL             (1 << 2)	// TRUE => This is ship belonging to the local player.
#define SHIP_FLAG_TRACTORING		(1 << 3)	// This ship has an active tractor beam.
#define SHIP_FLAG_COLLIDED			(1 << 4)	// Collided with the landscape last process
#define SHIP_FLAG_KEY_THRUST		(1 << 5)
#define SHIP_FLAG_KEY_LEFT			(1 << 6)
#define SHIP_FLAG_KEY_RIGHT			(1 << 7)
#define SHIP_FLAG_KEY_TRACTOR_BEAM	(1 << 8)	// The player has pressed the tractor beam key.


//
// The ships.
//

typedef struct
{
	UWORD flag;
	UBYTE tb;		// The tractor beam if (flag & SHIP_FLAG_TRACTOR_BEAM)
	UBYTE red;
	UBYTE green;
	UBYTE blue;
	UWORD padding;
	CBYTE name[32];
	SLONG active;	// The gameturn when this ship becomes active.
	float x;
	float y;
	float angle;
	float dx;
	float dy;
	float mass;		// Defaults to 1.0F
	float power;	// Defaults to 1.0F
	float hash;

} SHIP_Ship;

#define SHIP_MAX_SHIPS 16

extern SHIP_Ship SHIP_ship[SHIP_MAX_SHIPS];


//
// Initialises the ship module.
// 

void SHIP_init(void);


//
// Creates a new ship.  Returns NULL on failure.
//

SHIP_Ship *SHIP_create(
				float  x,
				float  y,
				float  mass  = 1.0F,
				float  power = 1.0F,
				UBYTE  red   = 255,
				UBYTE  green = 255,
				UBYTE  blue  = 100,
				CBYTE *name  = "Player");

//
// Flags all the ship's that come alive on
// the given gameturn as being alive.
//

void SHIP_flag_active(SLONG gameturn);


//
// Processes one gameturn of all the ships. The gameturn the
// ships are being processed at. It isn't necessarily GAME_turn
// if rollback has occured.
//

void SHIP_process_all(void);


//
// Draws all the ships.
//

void SHIP_draw_all(float mid_x, float mid_y, float zoom);



#endif
