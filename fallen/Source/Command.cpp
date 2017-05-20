// Command.cpp
// Guy Simmons, 8th February 1998.

#include	"Game.h"
#include	"Command.h"
#include	"StateDef.h"

// All this is temporary, I will try & replace it with a comprehensive
// command system for all enemies.

//---------------------------------------------------------------
//	Waypoint stuff.
//---------------------------------------------------------------

ULONG				waypoint_count;
Waypoint			waypoints[MAX_WAYPOINTS];

//---------------------------------------------------------------

void	init_waypoints(void)
{
	waypoint_count	=	0;
	memset((UBYTE*)waypoints,0,sizeof(waypoints));
}

//---------------------------------------------------------------

UWORD	alloc_waypoint(void)
{
	UWORD		c0;


	for(c0=1;c0<MAX_WAYPOINTS;c0++)
	{
		if(!waypoints[c0].Used)
		{
			waypoints[c0].Used	=	TRUE;
			waypoints[c0].Next	=	0;
			waypoints[c0].Prev	=	0;

			waypoint_count++;
			return	c0;
		}
	}
	return	0;
}

//---------------------------------------------------------------

void	free_waypoint(UWORD wp_index)
{
	UWORD		next_index,
				prev_index;


	next_index	=	waypoints[wp_index].Next;
	prev_index	=	waypoints[wp_index].Prev;
	if(prev_index)
		waypoints[prev_index].Next	=	next_index;
	
	if(next_index)
		waypoints[next_index].Prev	=	prev_index;

	waypoints[wp_index].Used	=	FALSE;
	waypoint_count--;
}

//---------------------------------------------------------------
//	Condition Stuff.
//---------------------------------------------------------------

ULONG			con_list_count,
				condition_count;
Condition		conditions[MAX_CONDITIONS];
ConditionList	con_lists[MAX_CLISTS];

//---------------------------------------------------------------

void	init_clists(void)
{
	con_list_count	=	0;
//	ZeroMemory(con_lists,sizeof(con_lists));
	memset((UBYTE*)con_lists,0,sizeof(con_lists));

	init_conditions();
}
//---------------------------------------------------------------

ConditionList	*alloc_clist(void)
{
	ULONG		c0;


	for(c0=1;c0<MAX_CLISTS;c0++)
	{
		if(!con_lists[c0].Used)
		{
			con_lists[c0].Used				=	TRUE;

			con_lists[c0].ConditionCount	=	0;
			con_lists[c0].Flags				=	0;
			con_lists[c0].TheList			=	0;
			con_lists[c0].TheListEnd		=	0;

			con_list_count++;
			return	&con_lists[c0];
		}
	}

	return	NULL;
}

//---------------------------------------------------------------

void	add_condition(ConditionList *the_list,Condition *the_condition)
{
	if(!the_list || !the_condition)
	{
		// Error.
		return;
	}

	//	Add to list.
	the_condition->Prev	=	the_list->TheListEnd;
	the_condition->Next	=	NULL;

	//	Update list end.
	if(the_list->TheListEnd)
		the_list->TheListEnd->Next	=	the_condition;
	the_list->TheListEnd		=	the_condition;

	//	Update list.
	if(!the_list->TheList)
		the_list->TheList	=	the_condition;

	the_list->ConditionCount++;
}

//---------------------------------------------------------------

void	init_conditions(void)
{
	condition_count	=	0;
//	ZeroMemory(conditions,sizeof(conditions));
	memset((UBYTE*)conditions,0,sizeof(conditions));
	
}

//---------------------------------------------------------------

Condition	*alloc_condition(void)
{
	ULONG		c0;


	for(c0=1;c0<MAX_CONDITIONS;c0++)
	{
		if(!conditions[c0].Used)
		{
			conditions[c0].Used			=	TRUE;

			conditions[c0].Flags		=	0;
			conditions[c0].ConditionType=	0;
			conditions[c0].Data1		=	0;
			conditions[c0].Data2		=	0;
			conditions[c0].Data3		=	0;
			conditions[c0].GroupRef		=	0;

			conditions[c0].Next			=	NULL;
			conditions[c0].Prev			=	NULL;

			condition_count++;

			return	&conditions[c0];
		}
	}
	return	NULL;
}


//---------------------------------------------------------------
//	Command Stuff.
//---------------------------------------------------------------

ULONG			com_list_count,
				command_count;
Command			commands[MAX_COMMANDS];
CommandList		com_lists[MAX_COMLISTS];

//---------------------------------------------------------------

void	init_comlists(void)
{
	com_list_count	=	0;
//	ZeroMemory(com_lists,sizeof(com_lists));
	memset((UBYTE*)com_lists,0,sizeof(com_lists));

	init_commands();
}
//---------------------------------------------------------------

CommandList	*alloc_comlist(void)
{
	ULONG		c0;


	for(c0=1;c0<MAX_COMLISTS;c0++)
	{
		if(!com_lists[c0].Used)
		{
			com_lists[c0].Used				=	TRUE;

			com_lists[c0].CommandCount		=	0;
			com_lists[c0].Flags				=	0;
			com_lists[c0].TheList			=	0;
			com_lists[c0].TheListEnd		=	0;

			com_list_count++;
			return	&com_lists[c0];
		}
	}

	return	NULL;
}

//---------------------------------------------------------------

void	add_command(CommandList *the_list,Command *the_command)
{
	if(!the_list || !the_command)
	{
		// Error.
		return;
	}

	//	Add to list.
	the_command->Prev	=	the_list->TheListEnd;
	the_command->Next	=	NULL;

	//	Update list end.
	if(the_list->TheListEnd)
		the_list->TheListEnd->Next	=	the_command;
	the_list->TheListEnd		=	the_command;

	//	Update list.
	if(!the_list->TheList)
		the_list->TheList	=	the_command;

	the_list->CommandCount++;
}

//---------------------------------------------------------------

void	init_commands(void)
{
	command_count	=	0;
//	ZeroMemory(commands,sizeof(commands));
	memset((UBYTE*)commands,0,sizeof(commands));
}

//---------------------------------------------------------------

Command	*alloc_command(void)
{
	ULONG		c0;


	for(c0=1;c0<MAX_COMMANDS;c0++)
	{
		if(!commands[c0].Used)
		{
			commands[c0].Used			=	TRUE;

			commands[c0].Flags			=	0;
			commands[c0].CommandType	=	0;
			commands[c0].Data1			=	0;
			commands[c0].Data2			=	0;
			commands[c0].Data3			=	0;
			commands[c0].GroupRef		=	0;

			commands[c0].Next			=	NULL;
			commands[c0].Prev			=	NULL;

			command_count++;

			return	&commands[c0];
		}
	}
	return	NULL;
}

//---------------------------------------------------------------
//	Condition processing.
//---------------------------------------------------------------

BOOL	process_condition(Condition *the_condition)
{
	BOOL		result	=	FALSE;
	SLONG		distance;
	GameCoord	*start_coord,*end_coord;
	Switch		*the_switch;


	switch(the_condition->ConditionType)
	{
		case	CON_NONE:
			break;
		case	CON_THING_DEAD:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
				if(TO_THING(the_condition->Data1)->State==STATE_DYING || TO_THING(the_condition->Data1)->State==STATE_DEAD)
				{
					the_condition->Flags	=	CONDITION_TRUE;
					result					=	TRUE;
				}
			}
			break;
		case	CON_ALL_GROUP_DEAD:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
			}
			break;
		case	CON_PERCENT_GROUP_DEAD:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
			}
			break;
		case	CON_THING_NEAR_PLAYER:
			start_coord	=	&NET_PERSON(0)->WorldPos;
			end_coord	=	&TO_THING(the_condition->Data1)->WorldPos;
			distance	=	SDIST3	(
										(start_coord->X-end_coord->X)>>8,
										(start_coord->Y-end_coord->Y)>>8,
										(start_coord->Z-end_coord->Z)>>8
									);
			if(distance<=the_condition->Data2)
			{
				result	=	TRUE;
			}
			break;
		case	CON_GROUP_NEAR_PLAYER:
			break;
		case	CON_CLASS_NEAR_PLAYER:
			break;
		case	CON_THING_NEAR_THING:
			start_coord	=	&TO_THING(the_condition->Data1)->WorldPos;
			end_coord	=	&TO_THING(the_condition->Data2)->WorldPos;
			distance	=	SDIST3	(
										(start_coord->X-end_coord->X)>>8,
										(start_coord->Y-end_coord->Y)>>8,
										(start_coord->Z-end_coord->Z)>>8
									);
			if(distance<=the_condition->Data3)
			{
				result	=	TRUE;
			}
			break;
		case	CON_GROUP_NEAR_THING:
			break;
		case	CON_CLASS_NEAR_THING:
			break;
		case	CON_CLASS_COUNT:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
			}
			break;
		case	CON_GROUP_COUNT:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
			}
			break;
		case	CON_SWITCH_TRIGGERED:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
				the_switch	=	TO_THING(the_condition->Data1)->Genus.Switch;
				if(the_switch->Flags&SWITCH_FLAGS_TRIGGERED)
				{
					if(!(the_switch->Flags&SWITCH_FLAGS_RESET))
						the_condition->Flags	=	CONDITION_TRUE;
					result	=	TRUE;
				}
			}
			break;
		case	CON_TIME:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
			}
			break;
		case	CON_CLIST_FULFILLED:
			if(the_condition->Flags&CONDITION_TRUE)
				result	=	TRUE;
			else
			{
			}
			break;
	}
	return	result;
}

//---------------------------------------------------------------

void	process_condition_lists(void)
{
	BOOL		list_fulfilled;
	ULONG		c0;
	Condition	*the_condition;


	for(c0=0;c0<MAX_CLISTS;c0++)
	{
		if(con_lists[c0].Used)
		{
			list_fulfilled	=	TRUE;
			the_condition	=	con_lists[c0].TheList;
			while(the_condition)
			{
				if(!process_condition(the_condition))
				{
					list_fulfilled	=	FALSE;
					break;
				}
				the_condition	=	the_condition->Next;
			}
			if(list_fulfilled)
				con_lists[c0].Flags	=	LIST_TRUE;
		}
	}
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//
// Below here interpret people command lists.
//
//
//---------------------------------------------------------------
//---------------------------------------------------------------
extern	void	init_person_command(Thing *p_person);


void	init_person_command_list(Thing *p_person)
{
	p_person->Genus.Person->Command=0;
	
	if(p_person->Genus.Person->ComList)
	{
		struct	Command	*com;

		com=p_person->Genus.Person->ComList->TheList;
		p_person->Genus.Person->Command=COMMAND_NUMBER(com);
		p_person->Genus.Person->CommandRef=0;
		init_person_command(p_person);
	}
	else
	{
		p_person->Genus.Person->Command=0;
	}

}
extern	void	set_person_mav_to_xz(Thing *p_person,SLONG x,SLONG z);

void	advance_person_command(Thing *p_person)
{
	struct	Command	*com;

//	p_person->Genus.Person->Command=0;

	
	com=TO_COMMAND(p_person->Genus.Person->Command);
	switch(com->CommandType)
	{
		case	COM_PATROL_WAYPOINT:
			if(p_person->Genus.Person->NavIndex)
			{
				SLONG	dest_x,dest_z;
				SLONG	index;

				p_person->Genus.Person->NavIndex =	waypoints[p_person->Genus.Person->NavIndex].Next;
				if(p_person->Genus.Person->NavIndex)
				{
					index=p_person->Genus.Person->NavIndex;

					dest_x=waypoints[index].X;
					dest_z=waypoints[index].Z;
					set_person_mav_to_xz(p_person,dest_x,dest_z);
					return;
				}

			}

			break;
	}
	if(com&&com->Next)
	{
		p_person->Genus.Person->Command=COMMAND_NUMBER(com->Next);
	}
	else
	{
		p_person->Genus.Person->Command=0;
	}
	if(p_person->Genus.Person->Command)
		init_person_command(p_person);
}
	


