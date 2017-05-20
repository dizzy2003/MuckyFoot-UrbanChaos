
#include "game.h"

#define WALLHUG_C

#include "wallhug.h"

#undef  ERROR
#define ERROR(err) {TRACE("Wallhug error...\n"); TRACE(err); TRACE("\n"); ASSERT(0);}

wallhug_waypoint wallhug_dirn_steps[4] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

//----------------------------------------------------------------------------

#define STEP_DIRN(waypoint, dirn)				\
{												\
	if (dirn != WALLHUG_NORTH &&				\
		dirn != WALLHUG_EAST &&					\
		dirn != WALLHUG_SOUTH &&				\
		dirn != WALLHUG_WEST)					\
	{											\
		ERROR("Step in which direction?");		\
	}											\
												\
	(waypoint).x += wallhug_dirn_steps[dirn].x;	\
	(waypoint).y += wallhug_dirn_steps[dirn].y;	\
}

#define MAX_LOOKAHEAD 4 // for line-of-sight stuff

//----------------------------------------------------------------------------

static SLONG dx, dy, bresval;
static ULONG y_dirn, x_dirn;
static BOOL x_longer;

ULONG wallhug_current_count;
UBYTE wallhug_last_hugstart;
UBYTE wallhug_last_handed;
UBYTE wallhug_last_dirn;
ULONG wallhug_last_hug_count;
BOOL wallhug_looking_for_last = FALSE;

//----------------------------------------------------------------------------
// set up the info for the bresenham line-draw

static void bresenham_start(wallhug_waypoint start,
							wallhug_waypoint end)
{
	SLONG xdiff, ydiff;

	xdiff = (ULONG)end.x; xdiff -= (ULONG)start.x;
	ydiff = (ULONG)end.y; ydiff -= (ULONG)start.y;

	if (abs(xdiff) > abs(ydiff)) x_longer = TRUE;
	else						 x_longer = FALSE;


	// work out which direction the line is, for stepping along the x or 
	// the y.

	if (xdiff > 0) x_dirn = WALLHUG_EAST;
	else		   x_dirn = WALLHUG_WEST;

	if (ydiff > 0) y_dirn = WALLHUG_SOUTH;
	else		   y_dirn = WALLHUG_NORTH;


	// if the line is horizontal or vertical, just force the 
	// bresenham stuff to always return the same direction

	if (start.x == end.x) x_dirn = y_dirn;
	if (start.y == end.y) y_dirn = x_dirn;


	// and do the bresenham thingies

	dx = abs(xdiff); dy = abs(ydiff); bresval = dx / 2 + dy / 2;
}

//----------------------------------------------------------------------------
// return the direction for the next step of the current bresenham draw

static ULONG bresenham()
{
	if (bresval >= dx)
	{
		bresval -= dx;
		return y_dirn;
	}

	bresval += dy;
	return x_dirn;
}

//----------------------------------------------------------------------------
// the hugger is in the process of hugging a wall. this steps it along.

inline void wallhug_hugstep(wallhug_info *hugger)
{
	// has this hugger failed already?

	if (hugger->dirn == WALLHUG_FAILED_DIRN) return;


	// is there a wall in front? if so, turn so that your hand is touching
	// it. so if you're left-handed, turn right, and vice versa.

	if (WALLHUG_WALL_IN_WAY(hugger->current.x, hugger->current.y, hugger->dirn))
	{
		hugger->dirn = WALLHUG_ADDMOD4(hugger->dirn, -hugger->handed);

		// and write out a waypoint

		hugger->path.waypoints[hugger->path.length] = hugger->current;
		hugger->path.length++;

		if (hugger->path.length > WALLHUG_MAX_PTS - 1)
		{
			hugger->dirn = WALLHUG_FAILED_DIRN;
			return;
		}
	}
	else
	{
		// it's clear to step forward
		STEP_DIRN(hugger->current, hugger->dirn);

		// check if your hand will still be touching a wall one step in
		// the future.

		// define the side your hand sticks out

		{
			ULONG hugside = WALLHUG_ADDMOD4(hugger->dirn, hugger->handed);

			// if there isn't, then you should turn towards your hugside

			if (!WALLHUG_WALL_IN_WAY(hugger->current.x, hugger->current.y, hugside))
			{
				hugger->dirn = hugside;

				// and write out a waypoint

				hugger->path.waypoints[hugger->path.length] = hugger->current;
				hugger->path.length++;

				if (hugger->path.length > WALLHUG_MAX_PTS - 1)
				{
					hugger->dirn = WALLHUG_FAILED_DIRN;
					return;
				}
			}
		}
	}


	// now check if you can release the wall and set off towards your
	// destination again.

	// first check: if the plane of the wall you're hugging is between you
	// and your destination, forget it.

	switch(WALLHUG_ADDMOD4(hugger->dirn, hugger->handed))
	{
		case WALLHUG_NORTH:
			if (hugger->path.end.y < hugger->current.y) return;
			break;

		case WALLHUG_EAST:
			if (hugger->path.end.x > hugger->current.x) return;
			break;

		case WALLHUG_SOUTH:
			if (hugger->path.end.y > hugger->current.y) return;
			break;

		case WALLHUG_WEST:
			if (hugger->path.end.x < hugger->current.x) return;
			break;

#if DEBUG == 1
		default:
			ERROR("Wall being hugged is an invalid direction");
#endif
	}

	
	// next check: are you facing towards the destination?
	// i.e. if the plane of your back is between you and your destination, 
	// forget it.

	switch(hugger->dirn)
	{
		case WALLHUG_NORTH:
			if (hugger->path.end.y > hugger->current.y) return;
			break;

		case WALLHUG_EAST:
			if (hugger->path.end.x < hugger->current.x) return;
			break;

		case WALLHUG_SOUTH:
			if (hugger->path.end.y < hugger->current.y) return;
			break;

		case WALLHUG_WEST:
			if (hugger->path.end.x > hugger->current.x) return;
			break;

#if DEBUG == 1
		default:
			ERROR("Hugger is facing an invalid direction");
#endif
	}


	// third check: are you closer to the destination than you were when
	// you started hugging this bit of wall? this is defined as being not
	// further away on either axis where there is a difference between the 
	// start and end points. also, you've got to be not equal to the start on
	// the axis that has the greater difference.
	// also, you've got to be not beyond the destination.

	if (hugger->old.x <= hugger->path.end.x)
	{
		if (hugger->current.x < hugger->old.x) return;
		if (hugger->current.x > hugger->path.end.x) return;
	}

	if (hugger->old.x >= hugger->path.end.x)
	{
		if (hugger->current.x > hugger->old.x) return;
		if (hugger->current.x < hugger->path.end.x) return;
	}

	if (hugger->old.y <= hugger->path.end.y)
	{
		if (hugger->current.y < hugger->old.y) return;
		if (hugger->current.y > hugger->path.end.y) return;
	}
	
	if (hugger->old.y >= hugger->path.end.y)
	{
		if (hugger->current.y > hugger->old.y) return;
		if (hugger->current.y < hugger->path.end.y) return;
	}

	if (hugger->old.x == hugger->current.x &&
		hugger->old.y == hugger->current.y) return;

	// otherwise, you can release the wall.

	hugger->dirn = WALLHUG_DONE;
}

//----------------------------------------------------------------------------
// true if there's a direct line-of-sight from start to end

static BOOL line_of_sight(wallhug_waypoint start, wallhug_waypoint end)
{
	wallhug_waypoint current = start;
	ULONG dirn;

	bresenham_start(start, end);

	while (current.x != end.x || current.y != end.y)
	{
		dirn = bresenham();

		if (WALLHUG_WALL_IN_WAY(current.x, current.y, dirn)) return FALSE;

		STEP_DIRN(current, dirn);
	}

	return TRUE;
}

//----------------------------------------------------------------------------

inline static BOOL huggers_met_again(wallhug_info *huggers)
{
	wallhug_waypoint one_ahead;

	if (huggers[0].dirn == WALLHUG_FAILED_DIRN || huggers[0].dirn == WALLHUG_DONE) return FALSE;

	one_ahead = huggers[0].current;
	STEP_DIRN(one_ahead, huggers[0].dirn);

	if (huggers[1].current.x == one_ahead.x &&
		huggers[1].current.y == one_ahead.y &&
		WALLHUG_ADDMOD4(huggers[0].dirn, 2) == huggers[1].dirn)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//----------------------------------------------------------------------------

inline BOOL wallhug_add_huggers_path(wallhug_path *path, wallhug_info *successful_hugger)
{
	ULONG c1;

	if (successful_hugger->path.length + path->length + 2 > WALLHUG_MAX_PTS) return 0;

	for (c1 = 0; c1 < successful_hugger->path.length; c1++)
	{
		path->waypoints[path->length] =
			successful_hugger->path.waypoints[c1];
		path->length++;
	}

	path->waypoints[path->length] = successful_hugger->current;
	path->length++;

	return 1;
}

//----------------------------------------------------------------------------
					
static BOOL line_of_sight_cleanup(wallhug_path *path, ULONG first_waypoint)
{
	BOOL deleted_waypoint, done_anything_at_all = FALSE;
	wallhug_waypoint start;
	ULONG finalised, walker;


	do
	{
		// finalised = the next waypoint in the finalised version of the path.
		// walker is the next waypoint being considered.

		walker = finalised = first_waypoint;

		if (first_waypoint == 0) start = path->start;
		else					 start = path->waypoints[first_waypoint - 1];

		for (deleted_waypoint = FALSE; walker < path->length;)
		{
			ULONG lookahead;

			for (lookahead = MAX_LOOKAHEAD; lookahead > 0; lookahead--)
			{
				if (walker + lookahead < path->length &&
					line_of_sight(start, path->waypoints[walker + lookahead]))
				{
					walker += lookahead;
					deleted_waypoint = TRUE;
					done_anything_at_all = TRUE;
					goto found_line;
				}
			}

			// no line of sight to later waypoints - just go for this one

			start = path->waypoints[finalised] = path->waypoints[walker];
			finalised++;
			walker++;

	   	found_line:;
		}

		path->length = finalised;
	}
	while (deleted_waypoint);

	return done_anything_at_all;
}

//----------------------------------------------------------------------------
// removes redundant waypoints from an already calculated path

ULONG wallhug_cleanup(wallhug_path *path, ULONG retval)
{
	wallhug_waypoint start;
	ULONG finalised, walker, lookahead_done;
	ULONG count = 10;
	ULONG changed_waypoint = 0;

	// keep iterating the following until no further waypoints deleted.


   line_of_sight_stuff:

	line_of_sight_cleanup(path, changed_waypoint);

	if (!--count)
	{
		TRACE("Something bad has happened in the wallhug.c!\n");

		return retval;
	}


	// another post-process. for each waypoint, navigate from the one before
	// to the one after, and if that new path does not include the middle
	// waypoint, replace the waypoint with the new path.

	{
		ULONG c1, c2;
		wallhug_path new_path;

		for (c1 = 0; c1 + 1 < path->length; c1++)
		{
			if (c1 == 0)
			{
				new_path.start = path->start;
			}
			else
			{
				new_path.start = path->waypoints[c1 - 1];
			}

			new_path.end = path->waypoints[c1 + 1];

			if (wallhug_trivial(&new_path) == WALLHUG_FAILED) continue;


			for (c2 = 0; c2 + 1 < new_path.length; c2++)
			{
				if (new_path.waypoints[c2].x == path->waypoints[c1].x &&
					new_path.waypoints[c2].y == path->waypoints[c1].y)
				{
					goto non_silly_waypoint;
				}
			}


			// ok, the waypoint c1 is not really necessary - replace it
			// with the waypoints in new_path.

			if (new_path.length + path->length - 2 > WALLHUG_MAX_PTS)
			{
				continue;		// too many waypoints
			}


			// move the remaining waypoints in the path out of the way

			memmove((UBYTE*)path->waypoints + c1 + new_path.length,
					(UBYTE*)path->waypoints + c1 + 2,
					(path->length - c1 - 2) * sizeof(wallhug_waypoint));


			// and copy in the new waypoints

			memcpy((UBYTE*)path->waypoints + c1,
				   (UBYTE*)new_path.waypoints,
				   new_path.length * sizeof(wallhug_waypoint));

			path->length = new_path.length + path->length - 2;

			if (c1 < MAX_LOOKAHEAD) changed_waypoint = 0;
			else					changed_waypoint = c1 - MAX_LOOKAHEAD;

			goto line_of_sight_stuff;

		   non_silly_waypoint:;
		}
	}

	return retval;
}

//----------------------------------------------------------------------------
// returns the number of steps taken

ULONG wallhug_tricky(wallhug_path *path)
{
	ULONG retval;

	// first get a simple answer. even if the path fails, we want to optimise
	// the path, because we'll return a path that gets you close.

	retval = wallhug_trivial(path);

	return wallhug_cleanup(path, retval);
}

//----------------------------------------------------------------------------
// carries on the path from current

ULONG wallhug_continue_trivial(wallhug_path *path, wallhug_waypoint current, ULONG max_count)
{
	wallhug_waypoint start = current;
	ULONG dirn;

#if DEBUG == 1
	if (path->end.x >= WALLHUG_WIDTH || path->end.y >= WALLHUG_HEIGHT)
	{
		ERROR("Wallhugging has been asked for an invalid path");
	}
#endif


	// boldly set off on a straight line for the destination

	bresenham_start(start, path->end);


	for (; wallhug_current_count < max_count &&
		   (current.x != path->end.x || current.y != path->end.y);
		 wallhug_current_count++)
	{
		// get the next step of the current line

		dirn = bresenham();


		// will this step make you crash into a wall?

		if (WALLHUG_WALL_IN_WAY(current.x, current.y, dirn))
		{
			// hit a wall - must start hugging. Output a waypoint, then
			// set up a left-handed and a right-handed hugger

			if (wallhug_looking_for_last)
			{
				// also note which waypoint nr it was when you started hugging.
				wallhug_last_hugstart  = path->length;
				wallhug_last_hug_count = wallhug_current_count;
				wallhug_last_dirn	   = dirn;
			}

			wallhug_info huggers[2];
			
			path->waypoints[path->length] = current;
			path->length++;

			huggers[0].current		= current;
			huggers[1].current		= current;

			huggers[0].old			= current;
			huggers[1].old			= current;

			huggers[0].handed		= -1;
			huggers[1].handed 		=  1;

			huggers[0].dirn			= WALLHUG_ADDMOD4(dirn,  1);
			huggers[1].dirn			= WALLHUG_ADDMOD4(dirn, -1);

			huggers[0].path.start 	= start;
			huggers[1].path.start 	= start;
			huggers[0].path.end   	= path->end;
			huggers[1].path.end   	= path->end;
			huggers[0].path.length	= 0;
			huggers[1].path.length	= 0;


			// and set them off hugging, until one succeeds or both fail.
			while(wallhug_current_count < max_count)
			{
				wallhug_hugstep(huggers + 0);

				if (huggers_met_again(huggers)) goto fail_hugging_and_return;

				wallhug_hugstep(huggers + 1);

				if (huggers_met_again(huggers)) goto fail_hugging_and_return;

				// check if either of the huggers have decided to let go
				if (huggers[0].dirn == WALLHUG_DONE)
				{
					if (!wallhug_add_huggers_path(path, &huggers[0])) goto fail_hugging_and_return;
					if (wallhug_looking_for_last) wallhug_last_handed = -1; // note which hand the hugger was using.
					break;
				}
				if (huggers[1].dirn == WALLHUG_DONE)
				{
					if (!wallhug_add_huggers_path(path, &huggers[1])) goto fail_hugging_and_return;
					if (wallhug_looking_for_last) wallhug_last_handed = 1; // note which hand the hugger was using.
					break;
				}

				// if both have failed, it's bad.
				if (huggers[0].dirn == WALLHUG_FAILED_DIRN &&
					huggers[1].dirn == WALLHUG_FAILED_DIRN)
				{
					goto fail_hugging_and_return;
				}

				wallhug_current_count++;
			}

			if (wallhug_current_count == max_count) goto fail_hugging_and_return;

			// must restart all the shit from here.

			current = path->waypoints[path->length - 1];
			start   = current;
			bresenham_start(start, path->end);
		}
		else
		{
			// can still walk along the current line - just go for it.

			STEP_DIRN(current, dirn);
		}
	}

	if (path->length >= WALLHUG_MAX_PTS - 2) return WALLHUG_FAILED;

	if (current.x == path->end.x && current.y == path->end.y)
	{
		path->waypoints[path->length] = current;
		path->length++;
		return wallhug_current_count;
	}
	else return WALLHUG_FAILED;


	// extra stuff...

fail_hugging_and_return:

	// put a waypoint on the end of the path, that's where you started
	// hugging.

	path->waypoints[path->length] = current;
	path->length++;
	return WALLHUG_FAILED;
}

//----------------------------------------------------------------------------
// returns the number of steps taken

ULONG wallhug_trivial(wallhug_path *path)
{
	path->length = 0; // initialise path.

	wallhug_current_count = 0;

	if (wallhug_looking_for_last)
	{
		wallhug_last_hugstart = WALLHUG_INVALID_WAYPOINT; // so we can tell later on if we never hugged.
	}

	return wallhug_continue_trivial(path, path->start, WALLHUG_MAX_COUNT);
}

