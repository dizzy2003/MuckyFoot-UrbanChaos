// Switch.cpp
// Guy Simmons, 9th March 1998.

#include	"Game.h"
#include	"statedef.h"
#include	"Switch.h"

StateFunction	switch_functions[]	=
{
	{	SWITCH_NONE,		NULL				},
	{	SWITCH_PLAYER,		fn_switch_player	},
	{	SWITCH_THING,		fn_switch_thing		},
	{	SWITCH_GROUP,		fn_switch_group		},
	{	SWITCH_CLASS,		fn_switch_class		}
};

//---------------------------------------------------------------

void	init_switches(void)
{
	memset((UBYTE*)SWITCHES,0,sizeof(SWITCHES));
	SWITCH_COUNT	=	0;
}

//---------------------------------------------------------------

Thing	*alloc_switch(UBYTE type)
{
	SLONG			c0;
	Switch			*new_switch;
	Thing			*switch_thing	=	NULL;


	// Run through the switch array & find an unused one.
	for(c0=0;c0<MAX_SWITCHES;c0++)
	{
		if(SWITCHES[c0].SwitchType==SWITCH_NONE)
		{
			switch_thing	=	alloc_thing(CLASS_SWITCH);
			if(switch_thing)
			{
				new_switch	=	TO_SWITCH(c0);
				new_switch->SwitchType	=	type;
				new_switch->Thing		=	THING_NUMBER(switch_thing);

				switch_thing->Genus.Switch	=	new_switch;

				switch_thing->StateFn	=	switch_functions[type].StateFn;
				switch_thing->State		=	STATE_NORMAL;
			}
			break;
		}
	}
	return	switch_thing;
}

//---------------------------------------------------------------

void	free_switch(Thing *switch_thing)
{
	// Set the person type to none & free the thing.
	switch_thing->Genus.Switch->SwitchType	=	SWITCH_NONE;
	remove_thing_from_map(switch_thing);
	free_thing(switch_thing);
}

//---------------------------------------------------------------

THING_INDEX	create_switch(void)
{
	/*
	Switch			*the_switch;
	Thing			*s_thing;
	THING_INDEX		thing_number	=	0;


	s_thing			=	alloc_switch((UBYTE)the_def->Genus);
	if(s_thing)
	{
		s_thing->WorldPos.X	=	the_def->X<<8;
		s_thing->WorldPos.Y	=	the_def->Y<<8;
		s_thing->WorldPos.Z	=	the_def->Z<<8;

		the_switch			=	s_thing->Genus.Switch;
		the_switch->Radius	=	the_def->Data[0]*the_def->Data[0];
		the_switch->Scanee	=	(UWORD)the_def->Data[1];

		thing_number	=	THING_NUMBER(s_thing);
	}
	return	thing_number;
	*/

	return 0;
}

//---------------------------------------------------------------

void	process_switch_sphere(Thing *s_thing,GameCoord *scanee_coord)
{
	SLONG		distance;
	Switch		*the_switch;


	the_switch		=	s_thing->Genus.Switch;
	distance		=	SDIST3	(
									(scanee_coord->X-s_thing->WorldPos.X)>>8,
									(scanee_coord->Y-s_thing->WorldPos.Y)>>8,
									(scanee_coord->Z-s_thing->WorldPos.Z)>>8
								);
	if(distance<=the_switch->Radius)
	{
		the_switch->Flags	|=	SWITCH_FLAGS_TRIGGERED;
	}
	else if(the_switch->Flags&SWITCH_FLAGS_RESET)
	{
		the_switch->Flags	&=	~SWITCH_FLAGS_TRIGGERED;
	}
}

//---------------------------------------------------------------

void	fn_switch_player(Thing *s_thing)
{
	process_switch_sphere(s_thing,&NET_PERSON(0)->WorldPos);
}

//---------------------------------------------------------------

void	fn_switch_thing(Thing *s_thing)
{
	process_switch_sphere(s_thing,&TO_THING(s_thing->Genus.Switch->Scanee)->WorldPos);
}

//---------------------------------------------------------------

void	fn_switch_group(Thing *s_thing)
{
}

//---------------------------------------------------------------

void	fn_switch_class(Thing *s_thing)
{
	/*

	ULONG		distance;
	Switch		*the_switch;
	Thing		*nearest_thing;


	the_switch		=	s_thing->Genus.Switch;
	distance		=	the_switch->Radius;
	nearest_thing	=	nearest_class(s_thing,(1<<the_switch->Scanee),&distance);

	if(nearest_thing)
	{
		the_switch->Flags	|=	SWITCH_FLAGS_TRIGGERED;
	}
	else if(the_switch->Flags&SWITCH_FLAGS_RESET)
	{
		the_switch->Flags	&=	~SWITCH_FLAGS_TRIGGERED;
	}
	*/
}

//---------------------------------------------------------------
