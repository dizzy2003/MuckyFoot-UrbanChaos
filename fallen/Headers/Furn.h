//
// Furniture on the streets and inside houses.
//

#ifndef FURN_H
#define FURN_H


#define	FLAG_FURN_DRIVING	  (1<<0)
#define	FLAG_FURN_WHEEL1_GRIP (1<<1)
#define	FLAG_FURN_WHEEL2_GRIP (1<<2)
#define	FLAG_FURN_WHEEL3_GRIP (1<<3)
#define	FLAG_FURN_WHEEL4_GRIP (1<<4)

//
// The furniture structure. A static piece of furniture does not have
// a structure associated with it. (The pointer in the Thing structure
// is NULL). If the furniture starts moving, however, it allocates one
// of these structures and decallocates it once it stops.
//

typedef struct 
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dyaw;
	SLONG dpitch;
	SLONG droll;

	//
	// temp stuff for cars...

	//
	SWORD	Wheel; //steering wheel position
	SWORD	RAngle;
	SWORD	OverSteer;
	SWORD	DeltaOverSteer;
	SWORD	Compression[4]; // suspension extension *4
	SWORD	SpringDY[4];
	UWORD	Flags;
	UWORD	Driver;

	//
	// Command system stuff.
	//

	UWORD	Command;
	UWORD	Waypoint;

	//
	// Temp stuff for doors...
	//

	UWORD	closed_angle;
	UWORD   ajar;
}Furniture;

typedef	Furniture* FurniturePtr;
#define MAX_FURNITURE 10

//
// The furniture state functions... there is only one type
// of furniture.
//

extern StateFunction FURN_statefunctions[];

//
// Initialises the furniture structures.
// Clears a furniture thing.
//

void init_furniture(void);
void free_furniture(Thing *furniture_thing);
Furniture *FURN_alloc_furniture(void);


//
// Creates the furniture thing and puts it on the mapwho in
// a stationary position.
//

THING_INDEX FURN_create(
				SLONG x,
				SLONG y,
				SLONG z,
				SLONG yaw,
				SLONG pitch,
				SLONG roll,
				SLONG prim);

THING_INDEX VEHICLE_create(
				SLONG x,
				SLONG y,
				SLONG z,
				SLONG angle,
				SLONG prim);

//
// Turns a normal furniture thing into a door.
//

void FURN_turn_into_door(
		THING_INDEX furniture_thing,
		UWORD       closed_angle,
		UWORD		ajar,
		UBYTE		am_i_locked);



//
// Slides a movement vector of someone with the given radius
// against the bounding box of the fucniture.  It returns
// where you should slide to.  If 'dont_slide', then if the
// vector intersects with the box, instead of sliding along it,
// it zeros out the movement vector. It returns TRUE in this
// case.
//
// It assumes that the pitch of the furniture is zero!!!
//

SLONG FURN_slide_along(
		THING_INDEX thing,
		SLONG  x1, SLONG  y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  radius,
		SLONG  dont_slide);

//
// Checks a movement vector against a piece of furniture and
// returns a change in angle so make the moving thing avoid
// the furniture. (Either +1 or -1).
//

SLONG FURN_avoid(
		THING_INDEX thing,
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2);

//
// Starts modelling the furniture with hypermatter.
//

void FURN_hypermatterise(THING_INDEX thing);


//
// Applies the force to the given piece of furniture.
//

void FURN_push(
		THING_INDEX thing,
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2);


//
// For each furniture thing, scan for walkable faces, copy them,
// then index them to the thing.
//

void FURN_add_walkable(void);


#endif
