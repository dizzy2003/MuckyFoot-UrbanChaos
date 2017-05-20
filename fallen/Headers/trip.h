//
// Trip wires.
//

#ifndef _TRIP_
#define _TRIP_


//
// When x1 == this magic number means the trip wire is deactivated.
// 

#define TRIP_X1_DEACTIVATED 0xffff


//
// The trip-wires.
//

typedef struct
{
	UBYTE along;
	UBYTE wait;
	UWORD counter;
	SWORD y;
	UWORD x1;	// If this equals TRIP_X1_DEACTIVATED then the tripwire isn't drawn or processed
	UWORD z1;
	UWORD x2;
	UWORD z2;

} TRIP_Wire;

#define TRIP_MAX_WIRES 32

extern	TRIP_Wire *TRIP_wire;//[TRIP_MAX_WIRES];
extern	SLONG     TRIP_wire_upto;


//
// Gets rid of the tripwire.
//

void TRIP_init(void);


//
// Creates a new length of tripwire. Returns the index of the
// trip-wire or NULL if it cant create another tripwire.
//
// If an identical tripwire already exists, it just returns
// the index of that tripwire.
//

UBYTE TRIP_create(
		SLONG y,
		SLONG x1,
		SLONG z1,
		SLONG x2,
		SLONG z2);

//
// Processes the trip-wires.
//

void TRIP_process(void);


//
// Returns TRUE if the given tripwire is activated.
//

SLONG TRIP_activated(UBYTE tripwire);


//
// Deactivates the given tripwire: stops it being drawn and processed.
//

void TRIP_deactivate(UBYTE tripwire);


//
// Drawing the trip-wires.
//

typedef struct
{
	SLONG y;
	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;
	UWORD counter;
	UBYTE along;	// How far along the beam is stopped due to someone being in the way.
	UBYTE padding;

} TRIP_Info;

void       TRIP_get_start(void);
TRIP_Info *TRIP_get_next (void);	// Returns NULL if there are no more trips left.



#endif
