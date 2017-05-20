//
// The map draw.
//

#ifndef _PLANMAP_
#define _PLANMAP_



//
// Draws the plan view of the city.
// 

void plan_view_shot(SLONG wx,SLONG wz,SLONG pixelw,SLONG sx,SLONG sy,SLONG w,SLONG h,UBYTE *mem);


//
// Draws a dot on the map.
//

#define BEACON_FLAG_BEACON (1 << 0)	// This is a beacon
#define BEACON_FLAG_POINTY (1 << 1)	// This is a player- takes direction from 'dir'

void map_beacon_draw(
		SLONG x,
		SLONG z,
		ULONG col,
		ULONG flags,
		UWORD dir);





#endif
