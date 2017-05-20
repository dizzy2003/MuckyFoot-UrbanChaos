//	WSpace.h
//	Guy Simmons, 11th August 1998.

#ifndef	WSPACE_H
#define	WSPACE_H

//---------------------------------------------------------------

#define	ET_NONE			0
#define	ET_ROOT			1
#define	ET_MAP			2
#define	ET_MISSION		3
#define	ET_LMAP			4
#define	ET_WAYPOINT		5
#define	ET_BRIEF		6

#define	IM_ROOT			0
#define	IM_MAP			6
#define	IM_MISSION		1
#define	IM_LMAP			7
#define	IM_WAYPOINT		3
#define	IM_BRIEF		4
#define IM_SEWER		5
/*
#define	IM_ROOT			5
#define	IM_MAP			4
#define	IM_MISSION		1
#define	IM_LMAP			0
#define	IM_WAYPOINT		0
#define	IM_BRIEF		0
#define IM_SEWER		3
*/
#define IM_MESSAGE		9
#define IM_ITEM			10
#define IM_CAMERA		11
#define IM_PLAYER		12
#define IM_TRAP			13
#define	IM_SOUND		14
#define IM_MAPEXIT		15
#define	IM_COP			16
#define IM_BOMB			17
#define IM_BROKEN		2

struct	WSElement
{
	UBYTE			ElementType;
	UWORD			EventPointRef,
					MapRef,
					MissionRef;
	HTREEITEM		TreeItem;
};

//---------------------------------------------------------------

extern BOOL			workspace_changed;
extern HWND			ws_tree,wpt_tree;

BOOL	init_workspace(HWND parent);
void	fini_workspace(void);
void	handle_ws_context(POINT *click_point);
BOOL	handle_ws_dblclk(POINT *click_point);
void	handle_ws_select(WSElement *the_element);
BOOL	get_element_at_point(POINT *click_point,WSElement **the_element);
void	ws_add_map(void);
void	ws_new_mission(void);
void	ws_del_mission(void);
void	ws_add_light_map(void);
void	ws_add_citsez_map(void);
BOOL	create_workspace(void);
BOOL	close_workspace(void);
BOOL	load_workspace(BOOL load_default_workspace);
BOOL	save_workspace(void);

//---------------------------------------------------------------
// waypointy stuff...
BOOL	init_wptlist(HWND parent);
void	fini_wptlist(void);
void	reset_wptlist(void);
void	fill_wptlist(Mission *mish);
HTREEITEM	ws_root_waypoint(CBYTE *msg, SLONG type, LPARAM param);
void	ws_add_waypoint(EventPoint *ep);
void	ws_set_waypoint(EventPoint *ep, CBYTE ndx);
void	ws_sel_waypoint(EventPoint *ep);
void	ws_del_waypoint(EventPoint *ep);


//---------------------------------------------------------------

#endif
