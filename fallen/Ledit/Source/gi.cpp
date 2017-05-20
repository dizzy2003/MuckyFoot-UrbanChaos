//
// Interface between the game and the light editor.
//

#include <MFStdLib.h>
#include "game.h"
#include "gi.h"
#include "morph.h"
#include "night.h"
#include "ob.h"
#include "trip.h"
#include "io.h"
#include "animtmap.h"
#include "dirt.h"
#include "mist.h"
#include "puddle.h"
#include "road.h"
#include "drip.h"
#include "shadow.h"
#include "interfac.h"
#include "mav.h"
#include "ns.h"
#include "elev.h"
#include "fc.h"


//
// The current view.
//

SLONG GI_view;


void GI_init()
{
	FC_init();
	AENG_init();
	MORPH_load();
	ANIM_init();

	GI_view = 0;
}


SLONG GI_load_map(CBYTE *name)
{

	//
	// This is important stuff... I guess.
	//

	void global_load(void);	// Defined in game.cpp loads stuff that gets screwed up by the editor.

//	global_load(); // this gets called almost immediately in ELEV_game_init()


	//
	// Load our map.
	//

	ELEV_game_init(
		name,
		NULL,
		NULL,
		NULL);

	//
	// Start off looking at the city.
	//

	GI_view = GI_VIEW_CITY;

	return TRUE;	// Nothing ever goes wrong... honest!
}


void GI_set_view(SLONG view)
{
	switch(view)
	{
		case GI_VIEW_CITY:

			if (GI_view == GI_VIEW_SEWERS)
			{
				//
				// Clean up sewer stuff and initialise the city view.
				//

				GAME_FLAGS &= ~GF_SEWERS;

				NS_cache_fini();
				DIRT_init(100, 3, 3, INFINITY, INFINITY, INFINITY, INFINITY);
			}

			break;

		case GI_VIEW_SEWERS:

			if (GI_view == GI_VIEW_CITY)
			{
				//
				// Clean up city stuff and initialise the sewer view.
				//

				GAME_FLAGS |= GF_SEWERS;

				DIRT_init(0, 0, 0, INFINITY, INFINITY, INFINITY, INFINITY);
				NIGHT_destroy_all_cached_info();
				NS_cache_init();
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	GI_view = view;
}


void GI_render_view_into_backbuffer(
		SLONG cam_x,
		SLONG cam_y,
		SLONG cam_z,
		SLONG cam_yaw,
		SLONG cam_pitch,
		SLONG cam_roll)
{
	if (GAME_FLAGS & GF_SEWERS)
	{
		//
		// Animate the water.
		// 

		DIRT_set_focus(cam_x,cam_z,0x800);
		DIRT_process();
	}

	//
	// Increase the gameturn, otherwise facets aren't drawn!
	// 

	GAME_TURN += 1;

	AENG_set_camera(
		 cam_x,
		 cam_y,
		 cam_z,
		 cam_yaw,
		 cam_pitch,
		 cam_roll);

	AENG_draw(FALSE);
}


SLONG GI_get_pixel_world_pos(
		SLONG  sx,
		SLONG  sy,
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG inside)
{
	return AENG_raytraced_position(
				 sx,
				 sy,
				 world_x,
				 world_y,
				 world_z,
				 inside);
}


ULONG GI_light_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight)
{
	ULONG ans;

	ans = AENG_light_draw(
				sx,
				sy,
				lx,
				ly,
				lz,
				colour,
				highlight);
	
	return ans;		// Make sure our flags are the same as the AENG flags!
}



void GI_fini()
{
	AENG_fini();
}


//---------------------------------------------------------------
//	GUY.
//---------------------------------------------------------------

ULONG GI_waypoint_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE highlight)
{
	ULONG ans;


	ans = AENG_waypoint_draw(
				sx,
				sy,
				lx,
				ly,
				lz,
				colour,
				highlight);
	
	return ans;		// Make sure our flags are the same as the AENG flags!
}

//---------------------------------------------------------------

ULONG GI_rad_trigger_draw(
		SLONG sx,
		SLONG sy,
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG rad,
		ULONG colour,
		UBYTE highlight)
{
	ULONG ans;

	ans = AENG_rad_trigger_draw(
				sx,
				sy,
				lx,
				ly,
				lz,
				rad,
				colour,
				highlight);

	return ans;		// Make sure our flags are the same as the AENG flags!
}

//---------------------------------------------------------------

void GI_groundsquare_draw(
		SLONG lx,
		SLONG ly,
		SLONG lz,
		ULONG colour,
		UBYTE polyinit)
{
	AENG_groundsquare_draw(lx,ly,lz,colour,polyinit);
}

//---------------------------------------------------------------
