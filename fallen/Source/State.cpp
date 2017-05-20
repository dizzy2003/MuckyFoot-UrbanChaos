// State.cpp
// Guy Simmons, 4th January 1998.

#include	"Game.h"


//---------------------------------------------------------------

void	set_state_function(Thing *t_thing,UBYTE state)
{
	StateFunction		*functions	=	NULL;


	switch(t_thing->Class)
	{
		case	CLASS_NONE:
			break;
		case	CLASS_PLAYER:
			functions	=	player_functions[t_thing->Genus.Player->PlayerType].StateFunctions;
			break;
		case	CLASS_CAMERA:
			break;
		case	CLASS_PROJECTILE:
			break;
		case	CLASS_BUILDING:
			break;
		case	CLASS_PERSON:
			functions	=	people_functions[t_thing->Genus.Person->PersonType].StateFunctions;
			break;
		/*
		case	CLASS_FURNITURE:
			functions	=	FURN_statefunctions;
			break;
		*/
		case	CLASS_VEHICLE:
			functions	=	VEH_statefunctions;
			break;
		case	CLASS_ANIMAL:
#if !defined(PSX) && !defined(TARGET_DC)
			functions	=	ANIMAL_functions[t_thing->Genus.Animal->AnimalType].StateFunctions;
#endif
			break;
		case	CLASS_CHOPPER:
			functions	=	CHOPPER_functions[t_thing->Genus.Chopper->ChopperType].StateFunctions;
			break;
		case	CLASS_PYRO:
			functions	=	PYRO_functions[t_thing->Genus.Pyro->PyroType].StateFunctions;
			break;
		default:
			ASSERT(0);
	}

	if(functions)
	{
		if(state<5)
			ASSERT(functions[state].State == state);

		t_thing->StateFn	=	functions[state].StateFn;
		t_thing->State		=	state;
	}
}

void	set_generic_person_state_function(Thing *t_thing,UBYTE state)
{
	StateFunction		*functions	=	NULL;

/*
	switch(t_thing->Class)
	{
		case	CLASS_PLAYER:
		case	CLASS_PERSON:
			functions	=	people_functions[t_thing->Genus.Person->PersonType].StateFunctions;
			break;
		default:
			LogText(" error thing must be person \n");
	}
*/
//	if(functions)
	{
		t_thing->StateFn	=	generic_people_functions[state].StateFn;
		t_thing->State		=	state;
	}
}

void	set_generic_person_just_function(Thing *t_thing,UBYTE state)
{
	StateFunction		*functions	=	NULL;

	t_thing->StateFn	=	generic_people_functions[state].StateFn;
}

//---------------------------------------------------------------
