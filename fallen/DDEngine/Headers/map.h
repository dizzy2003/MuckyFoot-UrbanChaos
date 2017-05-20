//
// The new map screen
//


#ifndef _MAP_
#define _MAP_



//
// Initialises the map (gets rid of nav beacons... sets up view)
//

void MAP_init(void);


//
// Draws the map.
//

void MAP_draw(void);



//
// Adds a NAV beacon to the map.
// Removes a beacon from the map.
//

UBYTE MAP_beacon_create(SLONG x, SLONG z, SLONG index, THING_INDEX track_my_position);
void  MAP_beacon_remove(UBYTE beacon);


//
// Processes the map.
//

void MAP_process(void);



//
// Draws the quick on-screen map beacon direction indicator.
//

void MAP_draw_onscreen_beacons(void);



#endif

