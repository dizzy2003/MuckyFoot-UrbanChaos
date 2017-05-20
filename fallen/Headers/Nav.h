//
// 2d navigation around the city.
//

#ifndef NAV_H
#define NAV_H


//
// Initiliases the navigation system from the current map.
// It invalidates all the current waypoints.
//

void NAV_init(void);


//
// The waypoints are given in a linked list. They don't have
// the start point but do have the end point.
//

#define NAV_WAYPOINT_FLAG_LAST		(1 << 0)
#define NAV_WAYPOINT_FLAG_PULLUP	(1 << 1)	// If you hit a col-vect going to this waypoint, then do a pull-up.

typedef struct
{
	UWORD x;
	UWORD z;
	UBYTE flag;
	UBYTE length;	// The number of waypoints after this one.
	UWORD next;

} NAV_Waypoint;

#define NAV_MAX_WAYPOINTS 512

extern NAV_Waypoint NAV_waypoint[NAV_MAX_WAYPOINTS];

//
// Returns a pointer to the given waypoint.
//

#ifndef NDEBUG
void NAV_waypoint_check(UWORD index);
#define NAV_WAYPOINT(index) (NAV_waypoint_check(index), &NAV_waypoint[index])
#else
#define NAV_WAYPOINT(index) (&NAV_waypoint[index])
#endif



//
// Returns a 2D navigation path. 
//

#define NAV_FLAG_PULLUP	(1 << 0)	// This person can do pull-ups.

UWORD NAV_do(UWORD x1, UWORD z1, UWORD x2, UWORD z2, UBYTE flag);


//
// Gives back a waypoint.
//

void NAV_waypoint_give(UWORD index);


//
// Gives back the whole nav path.
//

void NAV_path_give(UWORD index);


//
// Draws a navigation path.
//

void NAV_path_draw(UWORD startx, UWORD startz, UWORD path);



#endif
