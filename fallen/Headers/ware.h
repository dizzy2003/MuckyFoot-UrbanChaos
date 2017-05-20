//
// Warehouses- and you can go into them!
//

#ifndef _WARE_
#define _WARE_



//
// Warehouses have their own private MAVigation system and they know
// where their entrances are.
//

//
// The different directions the warehouse doors can be.
//

#define WARE_DOOR_XS 0
#define WARE_DOOR_XL 1
#define WARE_DOOR_ZS 2
#define WARE_DOOR_ZL 3

#define WARE_MAX_DOORS 4

typedef struct
{
	struct
	{
		UBYTE out_x;
		UBYTE out_z;
		UBYTE in_x;
		UBYTE in_z;

	}     door[WARE_MAX_DOORS];	// Upto four doors.
	UBYTE door_upto;

	UBYTE minx;
	UBYTE minz;
	UBYTE maxx;	// Inclusive
	UBYTE maxz;	// Inclusive

	UBYTE nav_pitch;		// The pitch of the WARE_mav array
	UWORD nav;				// Index into the WARE_mav array for this warehouse's MAVigation data.
	UWORD building;			// This building this warehouse is for.
	UWORD height;			// Index into the WARE_height array for this warehouse's MAV_height data
	UWORD rooftex;			// Index into the WARE_rooftex array for the roof-top textures of the warehouse
	UBYTE ambience;			// Sets the ambience sound to play while inside the warehouse
	UBYTE padding;			// Upto 32 bytes?

} WARE_Ware;

#define WARE_MAX_WARES 32

extern WARE_Ware *WARE_ware;//[WARE_MAX_WARES];
extern UWORD     WARE_ware_upto;


//
// The warehouse MAVigation data. This is where we store all the
// 2D MAV_nav arrays for the warehouses.
//

#define WARE_MAX_NAVS 4096

extern UWORD *WARE_nav;//[WARE_MAX_NAVS];
extern UWORD  WARE_nav_upto;


//
// The warehouses MAV_height data. 
//

#define WARE_MAX_HEIGHTS 8192

extern SBYTE *WARE_height;//[WARE_MAX_HEIGHTS];
extern UWORD  WARE_height_upto;

//
// The rooftop textures
//

#define WARE_MAX_ROOFTEXES 4096

extern UWORD *WARE_rooftex;//[WARE_MAX_ROOFTEXES];
extern UWORD  WARE_rooftex_upto;



//
// Set to the index of the warehouse if we are inside a warehouse or
// NULL if we are not.
//

extern UBYTE WARE_in;

//
// Goes through the map looking for warehouses. When it finds one it creates a new
// WARE_ware structure and makes a link to it from that building.  i.e. it calculates
// MAV info for that building and works out where its doors are.  It also sets the
// PAP_LO_FLAG_WAREHOUSE for all PAP_lo squares with square inside warehouses.
//
// CALL THIS FUNCTION BEFORE YOU CALL MAV_precalculate!
//

void WARE_init(void);


//
// Returns TRUE if the mapsquare is contained within the warehouse. Beware! There are
// cases when this function will get it wrong... but it isn't often!
// 

SLONG WARE_in_floorplan(UBYTE ware, UBYTE x, UBYTE z);

//
// Returns which warehouse contains the given mapsqure or NULL if no warehouse does.
//

SLONG WARE_which_contains(UBYTE x, UBYTE z);

//
// Returns whether the given (x,y,z) is 'underground' inside the warehouse. i.e. if
// it inside a crate. If returns TRUE if the point is outside the warehouse aswell.
//

SLONG WARE_inside(UBYTE ware, SLONG x, SLONG y, SLONG z);


//
// Returns the caps for going from the square in the given direction.
//

UBYTE WARE_get_caps(
		UBYTE ware,
		UBYTE x,
		UBYTE z,
		UBYTE dir);

//
// Returns the height at the given location inside a warehouse.
//

SLONG WARE_calc_height_at(UBYTE ware, SLONG x, SLONG z);



//
// Sets up the game to be inside the given building. It sets WARE_in and invalidates
// all the cached lighting.
//

void WARE_enter(SLONG building);
void WARE_exit (void);


//
// Does a mavigation for a person to enter the given warehouse.
// Does a mavigation for a person inside a warehouse.
// Does a mavigation for a person to exit the warehouse.
//

MAV_Action WARE_mav_enter (Thing *p_person, UBYTE ware, UBYTE caps);
MAV_Action WARE_mav_inside(Thing *p_person, UBYTE dest_x, UBYTE dest_z, UBYTE caps);
MAV_Action WARE_mav_exit  (Thing *p_person, UBYTE caps);



//
// Draws debug info for the warehouses.
//

void WARE_debug(void);



#endif
