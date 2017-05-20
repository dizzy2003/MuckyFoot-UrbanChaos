//
// New indoors.
//

#ifndef _ND_
#define _ND_


//
// The directions leading out of a navsquare.
//

#define ND_DIR_XS 0
#define ND_DIR_XL 1
#define ND_DIR_ZS 2
#define ND_DIR_ZL 3

//
// The navbits. Two bits for each direction.
//

#define ND_NAV_TYPE_WALL	0
#define ND_NAV_TYPE_NORMAL	1
#define ND_NAV_TYPE_DOORWAY	2
#define ND_NAV_TYPE_UNUSED	3


//
// Each storey has a rectangular array of squares- the bounding box
// of its floorplan.
//

typedef struct
{
	UBYTE texture;	// Bottom 6 bits are the texture, top 2 bits are the flip.
	UBYTE nav;

} ND_Square;

#define ND_MAX_SQUARES 2048

ND_Square ND_square[ND_MAX_SQUARES];
SLONG     ND_square_upto;


//
// External walls. The first point of the first wall is the (x1,z1) of the floorplan
// bounding box.  This last wall has this point as its endpoint.
//

typedef struct
{
	UBYTE x1;	// End point.
	UBYTE z1;
	UWORD texture;

} ND_Exwall;

#define ND_MAX_EXWALLS 64

ND_Exwall ND_exwall[ND_MAX_EXWALLS];
SLONG     ND_exwall_upto;


//
// Internal walls.
//

typedef struct
{
	UBYTE x1;
	UBYTE z1;
	UBYTE x2;
	UBYTE z2;

	UWORD texture;

} ND_Inwall;

#define ND_MAX_INWALLS 64

ND_Inwall ND_inwall[ND_MAX_INWALLS];
SLONG     ND_inwall_upto;


//
// The exits out of a floor.
//

#define ND_EXIT_TYPE_OUTSIDE 0
#define ND_EXIT_TYPE_UP		 1
#define ND_EXIT_TYPE_DOWN    2
#define ND_EXIT_TYPE_ROOF	 3

typedef struct
{
	UBYTE type;
	UBYTE dir;
	UBYTE x;
	UBYTE z;

} ND_Exit;

#define ND_MAX_EXITS 256

ND_Exit ND_exit[ND_MAX_EXITS];
SLONG   ND_exit_upto;


//
// Each storey.
//

typedef struct
{
	//
	// This inclusive bounding box of the floorplan.
	//

	UBYTE x1;	// These are squares- not points.
	UBYTE z1;
	UBYTE x2;
	UBYTE z2;

	//
	// The floorplan.
	//

	UBYTE size_x;
	UBYTE size_z;
	UWORD square;		// Index in the ND_square[] array

	UWORD dbuilding;	// The building whose inside this is.

	SBYTE alt;			// The altitude of this floor.  In MAV units- i.e. 4 per mapsquare.
	UBYTE padding;

	UBYTE inwall_num;
	UBYTE inwall_index;

	UBYTE exwall_num;
	UBYTE exwall_index;

	UBYTE exit_index;
	UBYTE exit_num

} ND_Floor;

#define ND_MAX_FLOORS 64

ND_Floor ND_floor[ND_MAX_FLOORS];
SLONG    ND_floor_upto;



#endif
