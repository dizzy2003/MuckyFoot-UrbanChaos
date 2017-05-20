//
// Crappy barrels with kludged physics
//

#ifndef _BARREL_
#define _BARREL_

//
// needed by memory.h
//
#define BARREL_MAX_SPHERES 	80 // The maximum number of moving barrels * 2
#define BARREL_MAX_BARRELS (300)

#define TO_BARREL(t)				(&BARREL_barrel[t])
#define	BARREL_NUMBER(t)			(COMMON_INDEX)(t-TO_BARREL(0))


//
// Spheres that are moving use two sphere structures...
//

typedef struct
{
	SLONG x;	// -INFINITY => Unused
	SLONG y;
	SLONG z;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SWORD still;	// The number of gameturns that this sphere has been below a threshold speed.
	UWORD radius;	// In the large coordinate system- even thought it is a UWORD!

} BARREL_Sphere;

struct Barrel
{
	UBYTE type;
	UBYTE flag;
	UWORD on;		// For stacked barrels it is the barrel you are supported by (NULL => on the ground)
	UWORD bs;		// For moving barrels its an index to 2 BARREL_Spheres in the BARREL_sphere[] array.
};

//
// Lets keep thing.h happy.
//

//struct  Barrel;
typedef Barrel *BarrelPtr;


//
// Initialises the barrels.
//

void BARREL_init(void);

extern	BARREL_Sphere *BARREL_sphere; //[BARREL_MAX_SPHERES];
extern	Barrel *BARREL_barrel;//[BARREL_MAX_BARRELS];

extern	SLONG         BARREL_sphere_last;	  // MARK!!! WTF, you usuall call thing BLAH_blah_upto
extern	SLONG  BARREL_barrel_upto;


//
// Creates a new barrel. If you create a barrel over a previous one- they
// stack up on top of eachother.
//

#define BARREL_TYPE_NORMAL	0
#define BARREL_TYPE_CONE	1
#define BARREL_TYPE_BURNING	2
#define BARREL_TYPE_BIN     3

UWORD BARREL_alloc(
		SLONG type,
		SLONG prim,
		SLONG x,
		SLONG z,
		SLONG waypoint);	// The waypoint that creates this barrel or NULL

//
// Returns the position on a burning barrel for where the flame should start.
//

GameCoord BARREL_fire_pos(Thing *p_barrel);

//
// For a moving vehicle- for instance.
// 

void BARREL_hit_with_prim(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw);

void BARREL_hit_with_sphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG radius);

//
// Call when the barrel is shot at by the given person.
//

void BARREL_shoot(
		Thing *p_barrel,
		Thing *p_shooter);

//
// Lets people pick up barrels.  This function positions the given barrel
// so that it looks like the person is holding the barrel.  It also stops
// the barrel processing itself. To start the barrel physics again, call
// BARREL_throw()
//

void BARREL_position_on_hands(Thing *p_barrel, Thing *p_person);
void BARREL_throw            (Thing *p_barrel);



#endif
