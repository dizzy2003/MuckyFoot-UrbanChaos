// Effect.cpp
// Guy Simmons, 8th December 1997.

#include	"Game.h"

#define	SPEED		OnFace
#define	RADIUS		Index

void	process_effect(Thing *e_thing);

//---------------------------------------------------------------

void	init_effect(Thing *e_thing)
{
	e_thing->DrawType			=	DT_EFFECT;
	e_thing->SPEED	=	128;
	e_thing->RADIUS	=	-((e_thing->SPEED*50)/64);

	e_thing->StateFn				=	(void(*)(Thing*))process_effect;
	
	add_thing_to_map(e_thing);
}

//---------------------------------------------------------------

void	process_effect(Thing *e_thing)
{
	e_thing->SPEED	=	(e_thing->SPEED*50)/64;
	if(e_thing->SPEED)
		e_thing->RADIUS	+=	e_thing->SPEED;
	else
	{
		remove_thing_from_map(e_thing);
		free_thing(e_thing);
	}
}

//---------------------------------------------------------------
