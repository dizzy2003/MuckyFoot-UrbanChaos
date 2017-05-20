// EdWay.h
// Guy Simmons, 6th February 1998.

#ifndef	ED_WAY_H
#define	ED_WAY_H

//---------------------------------------------------------------
// Waypoint definition.

#define	MAX_EDIT_WAYPOINTS		1000

struct EditWaypoint
{
	BOOL		Used;
	UWORD		Next,
				Prev;
	SLONG		X,Y,Z;
};

extern	ULONG				ed_waypoint_count;
extern	EditWaypoint		edit_waypoints[MAX_EDIT_WAYPOINTS];

//---------------------------------------------------------------

void	init_ed_waypoints(void);
UWORD	alloc_ed_waypoint(void);
void	free_ed_waypoint(UWORD wp_index);
void	link_next_waypoint(UWORD link_wp,UWORD next_wp);
void	link_prev_waypoint(UWORD link_wp,UWORD prev_wp);
void	pack_waypoints(UWORD *map_table);

//---------------------------------------------------------------

#endif
