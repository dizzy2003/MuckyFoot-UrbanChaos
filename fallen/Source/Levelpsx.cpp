#ifdef	PSX
// Level.cpp
// Guy Simmons, 29th January 1998.

#include	"Game.h"
#include	"Command.h"


UWORD			comlist_map[MAX_COMLISTS],
				conlist_map[MAX_CLISTS],
				waypoint_map[MAX_WAYPOINTS];
THING_INDEX		thing_map[MAX_THINGS];

void	store_player_pos(ThingDef *the_def);

#define	MFFileHandle	SLONG
//---------------------------------------------------------------

BOOL	load_thing_def(MFFileHandle the_file)
{

	return	TRUE;
}

//---------------------------------------------------------------

BOOL	load_waypoint_def(MFFileHandle the_file)
{

	return	TRUE;
}

//---------------------------------------------------------------

BOOL	load_condition_def(MFFileHandle the_file,ConditionDef *the_def)
{

	return	TRUE;
}

//---------------------------------------------------------------

BOOL	load_command_def(MFFileHandle the_file,CommandDef *the_def)
{
	return	TRUE;
}

//---------------------------------------------------------------

BOOL	load_clist_def(MFFileHandle the_file)
{
	return	TRUE;
}

//---------------------------------------------------------------

BOOL	load_comlist_def(MFFileHandle the_file)
{
	return	TRUE;
}

//---------------------------------------------------------------

BOOL	load_level(ULONG level)
{
	return	TRUE;
}

//---------------------------------------------------------------
#endif