// SaveTab.cpp
// Guy Simmons, 26th January 1998.

#include	"Editor.hpp"
#include	"EdCom.h"
#include	"Edway.h"

#include	"SaveTab.def"

#include	"..\..\Headers\Game.h"
#include	"..\..\Headers\Level.h"
#include	"..\..\Headers\Command.h"

//---------------------------------------------------------------

#define	CTRL_LOAD_BUTTON		1
#define	CTRL_SAVE_BUTTON		2

//---------------------------------------------------------------

SaveTab::SaveTab()
{
	CBYTE		lev_name[32];


	CurrentLevel	=	0;
	HilitedLevel	=	0;
	SaveState		=	TRUE;

	InitControlSet(save_tab_def);

	MapLevels();

	sprintf(lev_name,"Level %d",CurrentLevel);
	((CStaticText*)GetControlPtr(3))->SetString1(lev_name);
}

//---------------------------------------------------------------

SaveTab::~SaveTab()
{
}

//---------------------------------------------------------------

void	SaveTab::DrawTabContent(void)
{
	SetTabDrawArea();
	ClearTab();

	DrawControlSet();

	DrawLevelBox();
}

//---------------------------------------------------------------

UWORD	SaveTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	CBYTE		lev_name[32];
	UWORD		select_pos;
	MFPoint		local_point;

	
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			local_point	=	*clicked_point;
			GlobalToLocal(&local_point);
			select_pos	=	LevelHilitePos(&local_point);
			if(select_pos)
			{
				CurrentLevel	=	select_pos-1;
				sprintf(lev_name,"Level %d",CurrentLevel);
				((CStaticText*)GetControlPtr(3))->SetString1(lev_name);
			}
			else
			{
				local_point	=	*clicked_point;
				switch(HandleControlSetClick(flags,&local_point))
				{
					case	0:
						break;
					case	CTRL_LOAD_BUTTON:
						LoadLevel();
						break;
					case	CTRL_SAVE_BUTTON:
						SaveLevel();
						break;
				}
			}
			break;
		case	RIGHT_CLICK:
			break;
	}
	return	0;
}

//---------------------------------------------------------------

void	SaveTab::HandleTab(MFPoint *current_point)
{
	UBYTE		update	=	0;
	UWORD		select_pos;
	MFPoint		local_point;
	static BOOL	cleanup	=	FALSE;

	
	ModeTab::HandleTab(current_point);

	local_point	=	*current_point;
	GlobalToLocal(&local_point);

	select_pos	=	LevelHilitePos(&local_point);
	if(select_pos)
	{
		update	=	1;
		cleanup	=	TRUE;
	}

	if(!update && cleanup)
	{
		update	=	1;
		cleanup	=	FALSE;
	}

	if(update)
	{
		if(LockWorkScreen())
		{
			DrawLevelBox();

			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
		update	=	0;
	}
}

//---------------------------------------------------------------

void	SaveTab::HandleControl(UWORD control_id)
{

}

//---------------------------------------------------------------

void	SaveTab::DrawLevelBox(void)
{
	CBYTE		lev_name[32];
	UWORD		select_pos;
	SLONG		c0,c1,
				level_count		=	0,
				x_offset,
				y_offset,
				x_step,
				y_step;
	EdRect		item_rect,
				levels_rect;
	MFPoint		local_point;


	SetTabDrawArea();

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;
	GlobalToLocal(&local_point);

	select_pos	=	LevelHilitePos(&local_point);

	x_step		=	QTStringWidth("Level")+30;
	y_step		=	QTStringHeight()+4;
	x_offset	=	(ContentWidth()-(5*x_step))>>1;
	y_offset	=	(ContentHeight()-(20*y_step))-10;

	levels_rect.SetRect(x_offset,y_offset,(5*x_step),(20*y_step));
	levels_rect.FillRect(CONTENT_COL);

   	for(c0=0;c0<20;c0++)
	{
		for(c1=0;c1<5;c1++)
		{
			item_rect.SetRect	(
									x_offset+(c1*x_step),
									y_offset+(c0*y_step)+((y_step-QTStringHeight())>>1),
									x_step,
									y_step
								);
			if(select_pos==(level_count+1))
				item_rect.FillRect(HILITE_COL);
			else if(level_count==CurrentLevel)
				item_rect.FillRect(SELECT_COL);
			else if(GetMapBit(level_count))
				item_rect.FillRect(ACTIVE_COL);

			sprintf(lev_name,"Level %d",level_count);
			QuickTextC	(
							item_rect.GetLeft()+4,
							item_rect.GetTop(),
							lev_name,
							0
						);
			level_count++;
		}
	}

	levels_rect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

UWORD	SaveTab::LevelHilitePos(MFPoint *current_point)
{
	SLONG		c0,c1,
				level_count	=	0,
				x_offset,
				y_offset,
				x_step,
				y_step;
	EdRect		item_rect,
				levels_rect;


	x_step		=	QTStringWidth("Level")+30;
	y_step		=	QTStringHeight()+4;
	x_offset	=	(ContentWidth()-(5*x_step))>>1;
	y_offset	=	(ContentHeight()-(20*y_step))-10;
	levels_rect.SetRect(x_offset,y_offset,(5*x_step),(20*y_step));

	if(levels_rect.PointInRect(current_point))
	{
   		for(c0=0;c0<20;c0++)
		{
			for(c1=0;c1<5;c1++)
			{
				item_rect.SetRect	(
										x_offset+(c1*x_step),
										y_offset+(c0*y_step)+((y_step-QTStringHeight())>>1),
										x_step,
										y_step
									);
				if(item_rect.PointInRect(current_point))
				{
					return	level_count+1;
				}

				level_count++;
			}
		}
	}
	return	0;
}

//---------------------------------------------------------------

void	SaveTab::MapLevels(void)
{
	CBYTE		level_name[256];
	ULONG		c0;


	ZeroMemory(LevelsMap,sizeof(LevelsMap));
	for(c0=0;c0<128;c0++)
	{
		sprintf(level_name,"\\Fallen\\Levels\\Level%3.3d.lev",c0);
		if(FileExists(level_name))
			SetMapBit(c0);
	}
}

//---------------------------------------------------------------

UWORD	comlist_mapping_table[MAX_EDIT_COMLISTS],
		conlist_mapping_table[MAX_EDIT_CLISTS],
		thing_mapping_table[MAX_MAP_THINGS],
		waypoint_mapping_table[MAX_EDIT_WAYPOINTS];

void	SaveTab::LoadLevel(void)
{
	CBYTE				level_name[256];
	UBYTE				version;
	UWORD				ed_thing,
						new_thing,
						new_waypoint;
	ULONG				c0,c1,
						clist_count,
						thing_count,
						waypoint_count;
	Alert				save_alert;
	CommandDef			the_com_def;
	CommandListDef		the_comlist_def;
	ConditionDef		the_con_def;
	ConditionListDef	the_conlist_def;
	EditCommand			*the_command;
	EditComList			*the_comlist;
	EditCondition		*the_condition;
	EditCondList		*the_conlist;
	MFFileHandle		load_file;
	ThingDef			the_t_def;
	WaypointDef			the_w_def;


	if(SaveState==FALSE)
	{
		if(!save_alert.HandleAlert("The current level has not been saved.","Do you wish to continue?"))
		{
			RequestUpdate();
			return;
		}
		RequestUpdate();
	}

	// Delete all of the existing things.
	for(c0=0;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==MAP_THING_TYPE_ED_THING)
		{
			delete_thing(c0);
		}
	}


	init_ed_waypoints();
	init_ed_clists();
	init_ed_comlists();

	ZeroMemory(comlist_mapping_table,sizeof(comlist_mapping_table));
	ZeroMemory(conlist_mapping_table,sizeof(conlist_mapping_table));
	ZeroMemory(thing_mapping_table,sizeof(thing_mapping_table));
	ZeroMemory(waypoint_mapping_table,sizeof(waypoint_mapping_table));

	sprintf(level_name,"\\Fallen\\Levels\\Level%3.3d.lev",CurrentLevel);
	load_file	=	FileOpen(level_name);
	if(load_file!=FILE_OPEN_ERROR)
	{
		// Get file version.
		FileRead(load_file,&version,sizeof(version));

		if(version==0)
		{
//
//	Load Things.
//
			// Get the thing count.
			FileRead(load_file,&thing_count,sizeof(thing_count));

			for(c0=0;c0<thing_count;c0++)
			{
				// Read in the thing def.
				FileRead(load_file,&the_t_def.Version,sizeof(the_t_def.Version));
				if(the_t_def.Version==0)
				{
					FileRead(load_file,&the_t_def.Class,sizeof(the_t_def.Class));
					FileRead(load_file,&the_t_def.Genus,sizeof(the_t_def.Genus));
					FileRead(load_file,&the_t_def.X,sizeof(the_t_def.X));
					FileRead(load_file,&the_t_def.Y,sizeof(the_t_def.Y));
          			FileRead(load_file,&the_t_def.Z,sizeof(the_t_def.Z));
          			FileRead(load_file,&the_t_def.CommandRef,sizeof(the_t_def.CommandRef));
          			FileRead(load_file,&the_t_def.Data,sizeof(the_t_def.Data));
          			FileRead(load_file,&the_t_def.EdThingRef,sizeof(the_t_def.EdThingRef));
				}

				if(the_t_def.Class==CLASS_BUILDING)
				{
					ed_thing	=	building_list[the_t_def.Data[0]].ThingIndex;
					map_things[ed_thing].BuildingList	=	the_t_def.Data[0];
					map_things[ed_thing].EditorFlags	=	the_t_def.Data[1];
					map_things[ed_thing].EditorData		=	the_t_def.Data[2];
				}
				else
				{
					// Allocate & set up the thing.
					new_thing	=	find_empty_map_thing();
					if(new_thing)
					{
 						map_things[new_thing].X				=	the_t_def.X;
 						map_things[new_thing].Y				=	the_t_def.Y;
 						map_things[new_thing].Z				=	the_t_def.Z;
						map_things[new_thing].Class			=	the_t_def.Class;
						map_things[new_thing].Genus			=	the_t_def.Genus;
						map_things[new_thing].CommandRef	=	the_t_def.CommandRef;
						map_things[new_thing].Data[0]		=	the_t_def.Data[0];
						map_things[new_thing].Data[1]		=	the_t_def.Data[1];
						map_things[new_thing].Type			=	MAP_THING_TYPE_ED_THING;

						add_thing_to_edit_map(the_t_def.X>>8,the_t_def.Z>>8,new_thing);

						//	Set up mapping table.
						thing_mapping_table[the_t_def.EdThingRef]	=	new_thing;
					}
				}
			}

//
//	Load Waypoints.
//
			// Get the waypoint count.
			FileRead(load_file,&waypoint_count,sizeof(waypoint_count));

			for(c0=0;c0<waypoint_count;c0++)
			{
				// Read in the waypoint def.
				FileRead(load_file,&the_w_def.Version,sizeof(the_w_def.Version));
				if(the_w_def.Version==0)
				{
					FileRead(load_file,&the_w_def.Next,sizeof(the_w_def.Next));
					FileRead(load_file,&the_w_def.Prev,sizeof(the_w_def.Prev));
					FileRead(load_file,&the_w_def.X,sizeof(the_w_def.X));
					FileRead(load_file,&the_w_def.Y,sizeof(the_w_def.Y));
          			FileRead(load_file,&the_w_def.Z,sizeof(the_w_def.Z));
          			FileRead(load_file,&the_w_def.EdWaypointRef,sizeof(the_w_def.EdWaypointRef));
				}

				//	Allocate & set up the waypoint
				new_waypoint	=	alloc_ed_waypoint();
				if(new_waypoint)
				{
					edit_waypoints[new_waypoint].Next	=	the_w_def.Next;
					edit_waypoints[new_waypoint].Prev	=	the_w_def.Prev;
					edit_waypoints[new_waypoint].X		=	the_w_def.X;
					edit_waypoints[new_waypoint].Y		=	the_w_def.Y;
					edit_waypoints[new_waypoint].Z		=	the_w_def.Z;
				}

				//	Set up the mapping table.
				waypoint_mapping_table[the_w_def.EdWaypointRef]	=	new_waypoint;
			}
//
//	Remap the Waypoints.
//
			for(c0=1;c0<MAX_EDIT_WAYPOINTS;c0++)
			{
				if(edit_waypoints[c0].Used)
				{
					edit_waypoints[c0].Next	=	waypoint_mapping_table[edit_waypoints[c0].Next];
					edit_waypoints[c0].Prev	=	waypoint_mapping_table[edit_waypoints[c0].Prev];
				}
			}

//
//	Load Conditions.
//
			//	Get the condition list count.
			FileRead(load_file,&clist_count,sizeof(clist_count));

			for(c0=0;c0<clist_count;c0++)
			{
				//	Read in condition list def.
				FileRead(load_file,&the_conlist_def.Version,sizeof(the_conlist_def.Version));
				if(the_conlist_def.Version==0)
				{
					FileRead(load_file,&the_conlist_def.ListName[0],sizeof(the_conlist_def.ListName));
					FileRead(load_file,&the_conlist_def.ConditionCount,sizeof(the_conlist_def.ConditionCount));
					FileRead(load_file,&the_conlist_def.EdConListRef,sizeof(the_conlist_def.EdConListRef));

					//	Create a condition list.
					the_conlist	=	alloc_ed_clist();
					conlist_mapping_table[the_conlist_def.EdConListRef]	=	ED_CONLIST_NUMBER(the_conlist);
					memcpy(the_conlist->CListName,the_conlist_def.ListName,sizeof(the_conlist->CListName));

					for(c1=0;c1<the_conlist_def.ConditionCount;c1++)
					{
						//	Read in the condition def.
						FileRead(load_file,&the_con_def.Version,sizeof(the_con_def.Version));
						if(the_con_def.Version==0)
						{
							FileRead(load_file,&the_con_def.Flags,sizeof(the_con_def.Flags));
							FileRead(load_file,&the_con_def.ConditionType,sizeof(the_con_def.ConditionType));
							FileRead(load_file,&the_con_def.GroupRef,sizeof(the_con_def.GroupRef));
							FileRead(load_file,&the_con_def.Data1,sizeof(the_con_def.Data1));
							FileRead(load_file,&the_con_def.Data2,sizeof(the_con_def.Data2));
							FileRead(load_file,&the_con_def.Data3,sizeof(the_con_def.Data3));

							// create the condition.
							the_condition	=	alloc_ed_condition();
							add_condition(the_conlist,the_condition);

							the_condition->Flags			=	the_con_def.Flags;
							the_condition->ConditionType	=	the_con_def.ConditionType;
							the_condition->GroupRef			=	the_con_def.GroupRef;
							the_condition->Data1			=	the_con_def.Data1;
							the_condition->Data2			=	the_con_def.Data2;
							the_condition->Data3			=	the_con_def.Data3;
						}
					}
				}
			}
//
//	Remap the conditions.
//
			for(c0=1;c0<MAX_EDIT_CONDITIONS;c0++)
			{
				if(edit_conditions[c0].Used)
				{
					the_condition	=	&edit_conditions[c0];

					//	Remap the Thing reference data.
					switch(the_condition->ConditionType)
					{
						case	CON_NONE:
							break;
						case	CON_THING_DEAD:
							the_condition->Data1	=	thing_mapping_table[the_condition->Data1];
							break;
						case	CON_ALL_GROUP_DEAD:
							break;
						case	CON_PERCENT_GROUP_DEAD:
							break;
						case	CON_THING_NEAR_PLAYER:
							the_condition->Data1	=	thing_mapping_table[the_condition->Data1];
							the_condition->Data2	=	(UWORD)65536;
							break;
						case	CON_GROUP_NEAR_PLAYER:
							break;
						case	CON_CLASS_NEAR_PLAYER:
							break;
						case	CON_THING_NEAR_THING:
							the_condition->Data1	=	thing_mapping_table[the_condition->Data1];
							the_condition->Data2	=	thing_mapping_table[the_condition->Data2];
							the_condition->Data3	=	(UWORD)65536;
							break;
						case	CON_GROUP_NEAR_THING:
							break;
						case	CON_CLASS_NEAR_THING:
							break;
						case	CON_CLASS_COUNT:
							break;
						case	CON_GROUP_COUNT:
							break;
						case	CON_SWITCH_TRIGGERED:
							the_condition->Data1	=	thing_mapping_table[the_condition->Data1];
							break;
						case	CON_TIME:
							break;
						case	CON_CLIST_FULFILLED:
							the_condition->Data1	=	conlist_mapping_table[the_condition->Data1];
							break;
					}
				}
			}
//
//	Load Commands.
//
			//	Get the command list count.
			FileRead(load_file,&clist_count,sizeof(clist_count));

			for(c0=0;c0<clist_count;c0++)
			{
				//	Read in command list def.
				FileRead(load_file,&the_comlist_def.Version,sizeof(the_comlist_def.Version));
				if(the_comlist_def.Version==0)
				{
					FileRead(load_file,&the_comlist_def.ListName[0],sizeof(the_comlist_def.ListName));
					FileRead(load_file,&the_comlist_def.CommandCount,sizeof(the_comlist_def.CommandCount));
					FileRead(load_file,&the_comlist_def.EdComListRef,sizeof(the_comlist_def.EdComListRef));

					//	Create a condition list.
					the_comlist	=	alloc_ed_comlist();
					comlist_mapping_table[the_comlist_def.EdComListRef]	=	ED_COMLIST_NUMBER(the_comlist);
					memcpy(the_comlist->ComListName,the_comlist_def.ListName,sizeof(the_comlist->ComListName));

					for(c1=0;c1<the_comlist_def.CommandCount;c1++)
					{
						//	Read in the condition def.
						FileRead(load_file,&the_com_def.Version,sizeof(the_com_def.Version));
						if(the_com_def.Version==0)
						{
							FileRead(load_file,&the_com_def.Flags,sizeof(the_com_def.Flags));
							FileRead(load_file,&the_com_def.CommandType,sizeof(the_com_def.CommandType));
							FileRead(load_file,&the_com_def.GroupRef,sizeof(the_com_def.GroupRef));
							FileRead(load_file,&the_com_def.Data1,sizeof(the_com_def.Data1));
							FileRead(load_file,&the_com_def.Data2,sizeof(the_com_def.Data2));
							FileRead(load_file,&the_com_def.Data3,sizeof(the_com_def.Data3));

							// create the condition.
							the_command	=	alloc_ed_command();
							add_command(the_comlist,the_command);

							the_command->Flags			=	the_com_def.Flags;
							the_command->CommandType	=	the_com_def.CommandType;
							the_command->GroupRef		=	the_com_def.GroupRef;
							the_command->Data1			=	the_com_def.Data1;
							the_command->Data2			=	the_com_def.Data2;
							the_command->Data3			=	the_com_def.Data3;
						}
					}
				}
			}
//
//	Remap the commands.
//
			for(c0=1;c0<MAX_EDIT_COMMANDS;c0++)
			{
				if(edit_commands[c0].Used)
				{
					the_command	=	&edit_commands[c0];

					switch(the_command->CommandType)
					{
						case	COM_NONE:
							break;
						case	COM_ATTACK_PLAYER:
							break;
						case	COM_ATTACK_THING:
						case	COM_DEFEND_THING:
						case	COM_WAIT_FOR_TRIGGER:
							the_command->Data1	=	thing_mapping_table[the_command->Data1];
							break;
						case	COM_ATTACK_GROUP:
							break;
						case	COM_ATTACK_CLASS:
							break;
						case	COM_DEFEND_PLAYER:
							break;
						case	COM_DEFEND_GROUP:
							break;
						case	COM_DEFEND_CLASS:
							break;
						case	COM_PATROL_WAYPOINT:
							break;
						case	COM_START_TIMER:
							break;
						case	COM_WAIT_FOR_CLIST:
							the_command->Data1	=	conlist_mapping_table[the_command->Data1];
							break;
						case	COM_FOLLOW_PLAYER:
							break;
					}

					switch(the_command->Data2)
					{
						case	COM_S_NONE:
							break;
						case	COM_S_UNTIL_TRIGGER:
						case	COM_S_WHILE_TRIGGER:
							the_command->Data3	=	thing_mapping_table[the_command->Data3];
							break;
						case	COM_S_UNTIL_CLIST:
						case	COM_S_WHILE_CLIST:
							the_command->Data3	=	conlist_mapping_table[the_command->Data3];
							break;
					}
				}
			}

//
//	Remap Thing references.
//
			for(c0=0;c0<MAX_MAP_THINGS;c0++)
			{
				if(map_things[c0].Type==MAP_THING_TYPE_ED_THING)
				{
					switch(map_things[c0].Class)
					{
						case	CLASS_NONE:
							break;
						case	CLASS_PLAYER:
							break;
						case	CLASS_CAMERA:
							break;
						case	CLASS_PROJECTILE:
							break;
						case	CLASS_BUILDING:
							break;
						case	CLASS_PERSON:
							if(map_things[c0].Data[0])
							{
								map_things[c0].Data[0]	=	comlist_mapping_table[map_things[c0].Data[0]];
							}
							break;
						case	CLASS_FURNITURE:
							break;
						case	CLASS_SWITCH:
							if(map_things[c0].Genus==SWITCH_THING)
							{
								map_things[c0].Data[1]	=	thing_mapping_table[map_things[c0].Data[1]];
							}
							break;
						case	CLASS_VEHICLE:
							break;
						case	CLASS_SPECIAL:
							break;
					}
				}
				else if(map_things[c0].Type==MAP_THING_TYPE_BUILDING)
				{
					map_things[c0].EditorData	=	thing_mapping_table[map_things[c0].EditorData];
				}
			}
		}

		FileClose(load_file);
	}
}

//---------------------------------------------------------------

#define	LEVEL_VERSION			0
#define	T_DEF_VERSION			0
#define	W_DEF_VERSION			0
#define	CONL_DEF_VERSION		0
#define	CON_DEF_VERSION			0
#define	COML_DEF_VERSION		0
#define	COM_DEF_VERSION			0

void	SaveTab::SaveLevel(void)
{
	CBYTE				level_name[256];
	UBYTE				version	=	LEVEL_VERSION;
	UWORD				map_table[MAX_EDIT_WAYPOINTS];
	ULONG				thing_count		=	0;
	SLONG				c0;
	CommandDef			current_command;
	CommandListDef		current_comlist;
	ConditionDef		current_condition;
	ConditionListDef	current_clist;
	EditCommand			*the_command;
	EditCondition		*the_condition;
	MFFileHandle		save_file;
	ThingDef			current_thing;
	WaypointDef			current_waypoint;


	sprintf(level_name,"\\Fallen\\Levels\\Level%3.3d.lev",CurrentLevel);
	save_file	=	FileCreate(level_name,TRUE);
	if(save_file!=FILE_CREATION_ERROR)
	{
		// Write file version.
		FileWrite(save_file,&version,sizeof(version));
//
//	Save Things.
//
		// Leave a gap for the thing count.
		FileWrite(save_file,&thing_count,sizeof(thing_count));

		// Go through all the map things & save them as thing defs.
		for(c0=0;c0<MAX_MAP_THINGS;c0++)
		{
			if(map_things[c0].Type==MAP_THING_TYPE_ED_THING)
			{
				// Now create the def.
				current_thing.Version		=	T_DEF_VERSION;
				current_thing.Class			=	map_things[c0].Class;
				current_thing.Genus			=	map_things[c0].Genus;
				current_thing.X				=	map_things[c0].X;
				current_thing.Y				=	map_things[c0].Y;
				current_thing.Z				=	map_things[c0].Z;
				current_thing.CommandRef	=	map_things[c0].CommandRef;
				current_thing.Data[0]		=	map_things[c0].Data[0];
				current_thing.Data[1]		=	map_things[c0].Data[1];
				current_thing.EdThingRef	=	c0;
				FileWrite(save_file,&current_thing,sizeof(ThingDef));
				thing_count++;
			}
			else if(map_things[c0].Type==MAP_THING_TYPE_BUILDING)
			{
				//	Create a def for a building.
				current_thing.Version		=	T_DEF_VERSION;
				current_thing.Class			=	CLASS_BUILDING;
				current_thing.Genus			=	0;
				current_thing.X				=	map_things[c0].X;
				current_thing.Y				=	map_things[c0].Y;
				current_thing.Z				=	map_things[c0].Z;
				current_thing.CommandRef	=	0;
				current_thing.Data[0]		=	map_things[c0].BuildingList;
				current_thing.Data[1]		=	map_things[c0].EditorFlags;
				current_thing.Data[2]		=	map_things[c0].EditorData;
				current_thing.EdThingRef	=	c0;
				FileWrite(save_file,&current_thing,sizeof(ThingDef));
				thing_count++;
			}
		}
//
//	Save Waypoints.
//
		//	Write out the number of waypoints.
		FileWrite(save_file,&ed_waypoint_count,sizeof(ed_waypoint_count));

		// Go through all the waypoints & save them as defs.
		for(c0=0;c0<MAX_EDIT_WAYPOINTS;c0++)
		{
			if(edit_waypoints[c0].Used)
			{
				current_waypoint.Version		=	W_DEF_VERSION;
				current_waypoint.Next			=	edit_waypoints[c0].Next;
				current_waypoint.Prev			=	edit_waypoints[c0].Prev;
				current_waypoint.X				=	edit_waypoints[c0].X;
				current_waypoint.Y				=	edit_waypoints[c0].Y;
				current_waypoint.Z				=	edit_waypoints[c0].Z;
				current_waypoint.EdWaypointRef	=	c0;
				FileWrite(save_file,&current_waypoint,sizeof(WaypointDef));
			}
		}

//
//	Save Conditions.
//
		//	Write out the number of condition lists.
		FileWrite(save_file,&ed_clist_count,sizeof(ed_clist_count));

		//	Save out all condition lists.
		for(c0=1;c0<MAX_EDIT_CLISTS;c0++)
		{
			if(edit_clists[c0].Used)
			{
				current_clist.Version	=	CONL_DEF_VERSION;
				memcpy(current_clist.ListName,edit_clists[c0].CListName,32);
				current_clist.ConditionCount	=	edit_clists[c0].ConditionCount;
				current_clist.EdConListRef		=	c0;
				FileWrite(save_file,&current_clist,sizeof(current_clist));

				// Save out all the conditions for the list.
				the_condition	=	edit_clists[c0].ConditionList;
				while(the_condition)
				{
					current_condition.Version		=	CON_DEF_VERSION;
					current_condition.Flags			=	the_condition->Flags;
					current_condition.ConditionType	=	the_condition->ConditionType;
					current_condition.GroupRef		=	the_condition->GroupRef;
					current_condition.Data1			=	the_condition->Data1;
					current_condition.Data2			=	the_condition->Data2;
					current_condition.Data3			=	the_condition->Data3;
					FileWrite(save_file,&current_condition,sizeof(current_condition));

					the_condition	=	the_condition->Next;
				}
			}
		}

//
//	Save Commands.
//
		//	Write out the number of command lists.
		FileWrite(save_file,&ed_comlist_count,sizeof(ed_comlist_count));

		//	Save out all command lists.
		for(c0=1;c0<MAX_EDIT_COMLISTS;c0++)
		{
			if(edit_comlists[c0].Used)
			{
				current_comlist.Version	=	COML_DEF_VERSION;
				memcpy(current_comlist.ListName,edit_comlists[c0].ComListName,32);
				current_comlist.CommandCount	=	edit_comlists[c0].CommandCount;
				current_comlist.EdComListRef	=	c0;
				FileWrite(save_file,&current_comlist,sizeof(current_comlist));

				// Save out all the commands for the list.
				the_command	=	edit_comlists[c0].CommandList;
				while(the_command)
				{
					current_command.Version			=	COM_DEF_VERSION;
					current_command.Flags			=	the_command->Flags;
					current_command.CommandType		=	the_command->CommandType;
					current_command.GroupRef		=	the_command->GroupRef;
					current_command.Data1			=	the_command->Data1;
					current_command.Data2			=	the_command->Data2;
					current_command.Data3			=	the_command->Data3;
					FileWrite(save_file,&current_command,sizeof(current_command));

					the_command	=	the_command->Next;
				}
			}
		}

		// Seek back to the thing count & write it out.
		FileSeek(save_file,SEEK_MODE_BEGINNING,sizeof(version));
		FileWrite(save_file,&thing_count,sizeof(thing_count));

		FileClose(save_file);
	}

	SaveState	=	TRUE;
	MapLevels();
}

//---------------------------------------------------------------
