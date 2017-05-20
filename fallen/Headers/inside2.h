#ifndef	INSIDE2_H
#define	INSIDE2_H	1



#include "structs.h"



//
// defines
//

#define	MAX_INSIDE_RECT	2000
#define	MAX_INSIDE_MEM	64000
#define	MAX_INSIDE_STAIRS	(MAX_INSIDE_RECT*4)

#define	START_PAGE_FOR_FLOOR	8



#define	FLAG_DOOR_LEFT	(1<<4)
#define	FLAG_DOOR_UP	(1<<5)
#define	FLAG_INSIDE_STAIR_UP (1<<6)
#define	FLAG_INSIDE_STAIR_DOWN (1<<7)
#define	FLAG_INSIDE_STAIR (3<<6)

//
//structures
//


struct	InsideStorey
{
	UBYTE	MinX;           // bounding rectangle of floor
	UBYTE	MinZ;
	UBYTE	MaxX;
	UBYTE	MaxZ;
	UWORD	InsideBlock;    // index into inside_block (block of data of size bounding rect) data is room numbers 1..15 top 4 bits reserved
	UWORD	StairCaseHead;  // link list of stair structures for this floor
	UWORD	TexType;		// Inside style to use for floor
	UWORD	FacetStart;     // index into facets that make up this building
	UWORD	FacetEnd;		// Facet After last used facet for inside the floor
	SWORD	StoreyY;	    // Y co-ord could come in handy
	UWORD	Building;
	UWORD	Dummy[2];
};

struct	Staircase
{
	UBYTE	X,Z;         // pos of staircase
	UBYTE	Flags;       // flags for direction + up or down or both
	UBYTE	ID;          // padding
	SWORD	NextStairs;  // link to next stair structure
	SWORD	DownInside;  // link to next insidestorey for going downstairs
	SWORD	UpInside;  	 // link to next InsideStorey for going upstairs
};







//
//Data
//

extern	UBYTE	slide_inside_stair;



//extern	struct 	DInsideRect	inside_rect[MAX_INSIDE_RECT];

extern	UWORD	next_inside_storey;
extern	UWORD	next_inside_stair;
extern	SLONG	next_inside_block;



//
// Functions
//



extern	SLONG	get_inside_alt(SLONG	inside);
extern	SLONG find_inside_room(SLONG inside,SLONG x,SLONG z);
extern	SLONG find_inside_flags(SLONG inside,SLONG x,SLONG z);
extern	SLONG	person_slide_inside(SLONG inside,SLONG x1,SLONG y1,SLONG z1,SLONG *x2,SLONG *y2,SLONG *z2);
extern	void	stair_teleport_bodge(Thing *p_person);
extern	SLONG	find_stair_y(Thing *p_person,SLONG *y1,SLONG x,SLONG y,SLONG z,UWORD *new_floor);


// ========================================================
//
// INSIDE NAVIGATION
//
// ========================================================

#if 0
// Never used!
MAV_Action INSIDE2_mav_enter (Thing *p_person, SLONG inside, UBYTE caps);		// To enter the building with the given inside
MAV_Action INSIDE2_mav_inside(Thing *p_person, SLONG inside, UBYTE x, UBYTE z);	// Navigating within a floor
MAV_Action INSIDE2_mav_stair (Thing *p_person, SLONG inside, SLONG new_inside); // Going up the stairs to another floor
MAV_Action INSIDE2_mav_exit  (Thing *p_person, SLONG inside);					// Exit the building
#endif




#endif
