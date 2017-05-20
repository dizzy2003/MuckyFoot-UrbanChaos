//	MapView.h
//	Guy Simmons, 12th August 1998.

#ifndef	MAPVIEW_H
#define	MAPVIEW_H

#include	"Mission.h"


//---------------------------------------------------------------

extern SLONG			cam_x,
						cam_y,
						cam_z,
						cam_yaw,
						cam_pitch,
						cam_focus_x,
						cam_focus_y,
						cam_focus_z,
						cam_focus_dist,
						cam_matrix[9],
						cam_forward[3],
						cam_left[3];

//	The mouse.
extern SLONG			mouse_valid,
						mouse_over,
						mouse_world_x,
						mouse_world_y,
						mouse_world_z,
						mouse_waypoint;

extern EventPoint		*hilited_ep,
						*selected_ep,
						*link_start_ep;

extern UBYTE			link_mode;

extern SLONG			zone_colours[ZF_NUM];

//---------------------------------------------------------------

BOOL	init_map_view(void);
void	fini_map_view(void);
void	process_view_wind(void);

//---------------------------------------------------------------

void   WaypointCaption(EventPoint *ep);
CBYTE *WaypointTitle(EventPoint *ep, CBYTE *msg);
CBYTE *WaypointExtra(EventPoint *ep, CBYTE *msg);

//---------------------------------------------------------------

BOOL TypeHasProperties(SLONG type);
BOOL HasProperties(EventPoint *ep);
SLONG OpenProperties(EventPoint *ep);
void CleanProperties(EventPoint *ep);


#endif
