//	WaypointSetup.h
//	Matthew Rosenfeld, 11th November 1998.

#ifndef	_WAYPOINTSETUP_H_
#define	_WAYPOINTSETUP_H_

#include	"Mission.h"


//---------------------------------------------------------------

void	do_waypoint_setup(EventPoint *ep);
CBYTE	*get_waypoint_message(EventPoint *ep, CBYTE *msg);

//---------------------------------------------------------------

#endif
