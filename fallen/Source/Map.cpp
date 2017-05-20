// Map.cpp
// Guy Simmons, 22nd October 1997.

#include	"Game.h"


//---------------------------------------------------------------
#if !defined(PSX) && !defined(TARGET_DC)
void	init_map(void)
{
	memset((UBYTE*)MAP,0,sizeof(MAP));
}
#endif
//---------------------------------------------------------------



SLONG MAP_light_get_height(SLONG x, SLONG z)
{
	return 0;
}

LIGHT_Colour MAP_light_get_light(SLONG x, SLONG z)
{
#ifdef TARGET_DC
	// Shouldn't be using this, apparently.
	ASSERT ( FALSE );
#endif

	MapElement *me;

	me = &MAP[MAP_INDEX(x,z)];

	return me->Colour;
}

void MAP_light_set_light(SLONG x, SLONG z, LIGHT_Colour colour)
{
#ifdef TARGET_DC
	// Shouldn't be using this, apparently.
	ASSERT ( FALSE );
#endif

	MapElement *me;

	me = &MAP[MAP_INDEX(x,z)];

	me->Colour = colour;
}


LIGHT_Map MAP_light_map = 
{
	MAP_WIDTH,
	MAP_HEIGHT,
	MAP_light_get_height,
	MAP_light_get_light,
	MAP_light_set_light
};




