//
// Objects (prims) on the map.
//

#ifndef _OB_
#define _OB_

#include "pap.h"

// defines

#define OB_MAX_OBS			2048
#define OB_SIZE				PAP_SIZE_LO
#define OB_MAX_PER_SQUARE	31

#define	OB_FLAG_ON_FLOOR	(1<<0)
#define	OB_FLAG_SEARCHABLE	(1<<1)
//#define	OB_FLAG_INSIDE		(1<<2)	// This flag is deceased. It is an ex-flag.
#define OB_FLAG_NOT_ON_PSX	(1<<2)
#define OB_FLAG_DAMAGED		(1<<3)
#define OB_FLAG_WAREHOUSE	(1<<4)	// This OB is inside a warehouse
#define	OB_FLAG_HIDDEN_ITEM (1<<5)

#define OB_FLAG_RESERVED1	(1<<6)	// These two flags mean special things for
#define OB_FLAG_RESERVED2	(1<<7)	// damaged prims.

//structures 

typedef struct
{
	UWORD index : 11;
	UWORD num   : 5;
	
} OB_Mapwho;

//
// Info about the objects in lo-res mapwho square (x,z). The array is
// terminated by the prim being zero.
//

typedef struct
{
	UWORD prim;			// 0 => No more info.
	UWORD x;
	SWORD y;
	UWORD z;
	UWORD yaw;
	UWORD pitch;
	UWORD roll;
	UWORD index;		// The index of this object 
	UBYTE crumple;		// How crumpled up this object is from 0 - 4 inclusive
	UBYTE InsideIndex;
	UBYTE Room;
	UBYTE flags;

} OB_Info;

//
// The objects.
//

typedef struct
{
	SWORD y;       
	UBYTE x;
	UBYTE z;
	UBYTE prim;
	UBYTE yaw;
	UBYTE flags;
	UBYTE InsideIndex;

} OB_Ob;

//
// Data
//

typedef	OB_Mapwho  OB_workaround[OB_SIZE];
extern OB_workaround *OB_mapwho; //[OB_SIZE][OB_SIZE];
extern OB_Ob     *OB_ob;//[OB_MAX_OBS];
extern SLONG     OB_ob_upto;


//
// Initialise all the furniture.
//

void OB_init(void);


//
// Loads the prims objects needed to render the OBjects.
//

void OB_load_needed_prims(void);


//
// Places some furniture.
//

void OB_create(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,
		SLONG prim,
		UBYTE flag,
		UWORD Inside,
		UBYTE Room);

//
// Returns an array of all the prims found on the lo-res mapsquare.
// The array is terminated with an object having a NULL prim.
//

OB_Info *OB_find(SLONG lo_map_x, SLONG lo_map_z);
OB_Info *OB_find_inside(SLONG x, SLONG z,SLONG indoors);


//
// Finds the nearest object whose prim object contains one
// of the given flags.  Returns FALSE if no object was found
// in the range.
//

#define	FIND_OB_TRIPWIRE		(1 <<  9)
#define FIND_OB_SWITCH_OR_VALVE	(1 << 10)

SLONG OB_find_type(
		SLONG  mid_x,
		SLONG  mid_y,
		SLONG  mid_z,
		SLONG  max_range,
		ULONG  prim_flags,
		SLONG *ob_x,
		SLONG *ob_y,
		SLONG *ob_z,
		SLONG *ob_yaw,
		SLONG *ob_prim,
		SLONG *ob_index);

//
// Removes the given OB_ob. Only the index and (x,z) of the
// OB_Info need be valid- or you can pass the OB_Info got
// back from a call to OB_Find.
//

void OB_remove(OB_Info *oi);

//
// Checks a movement vector against the given object
// returns a change in angle so make the moving thing avoid
// the furniture. (Either +1 or -1).
//

SLONG OB_avoid(
		SLONG ob_x,
		SLONG ob_y,
		SLONG ob_z,
		SLONG ob_yaw,
		SLONG ob_prim,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);


//
// Damages the prim due to a force from the given location.
//

void OB_damage(
		SLONG  index,			// The index of this object,
		SLONG  from_dx,			// The position of the damaging thing relative to the object.
		SLONG  from_dz,		
		SLONG  ob_x,			// The position of the ob!
		SLONG  ob_z,
		Thing *p_aggressor);	// Who caused the damage or NULL if you don't know

//
// Returns TRUE if there is a prim at (x,y,z)
//

SLONG OB_inside_prim(SLONG x, SLONG y, SLONG z);


//
// Processes the objects. Not much happens here except that damaged fire
// hydrants spout water.
//

void OB_process(void);


//
// Converts all dustbin prims into barrels.
//

void OB_convert_dustbins_to_barrels(void);


//
// Makes sure that all the switches are the correct
// height above the ground.
//

void OB_make_all_the_switches_be_at_the_proper_height(void);


//
// Adds the object walkable faces.
//

void OB_add_walkable_faces(void);

SLONG OB_find_min_y(SLONG prim);
OB_Info *OB_find_index(SLONG  mid_x,SLONG  mid_y,SLONG  mid_z,SLONG  max_range, SLONG must_be_searchable);
//SLONG OB_find_index(SLONG  mid_x,SLONG  mid_y,SLONG  mid_z,SLONG  max_range);

#endif
