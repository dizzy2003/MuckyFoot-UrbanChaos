// ComTab.cpp
// Guy Simmons, 9th March 1998.

#include	"Editor.hpp"
#include	"EdCom.h"

#include	"..\..\Headers\Game.h"
#include	"..\..\Headers\Command.h"

//---------------------------------------------------------------

#define	MAX_VIEW_LISTS		15
#define	MAX_VIEW_COMMANDS	24

#define	LISTS_X				2
#define	LISTS_Y				32
#define	LISTS_WIDTH			150
#define	LISTS_HEIGHT		((MAX_VIEW_LISTS*8)+3)

#define	COM_LIST_X			2
#define	COM_LIST_Y			226
#define	COM_LIST_WIDTH		(280)
#define	COM_LIST_HEIGHT		((MAX_VIEW_COMMANDS*8)+3)

#define	COM_NAME_X			2
#define	COM_NAME_Y			(COM_LIST_Y-14)
#define	COM_NAME_WIDTH		LISTS_WIDTH

#define	CTRL_NEW_COMLIST	1
#define	CTRL_LISTS_SLIDER	2
#define	CTRL_COMLIST_EDIT	3
#define	CTRL_COMLIST_SLIDER	4
#define	CTRL_NEW_COMMAND	5

#define	UPDATE_NONE			0
#define	UPDATE_ALL			1
#define	UPDATE_LISTS_BOX	2
#define	UPDATE_CURRENT_LIST	3

#define	MAX_FIELDS			4
#define	FIELD_1_WIDTH		94
#define	FIELD_2_WIDTH		62
#define	FIELD_3_WIDTH		62
#define	FIELD_4_WIDTH		62

#include	"ComTab.def"

extern	CBYTE	*class_text[],
				*genus_text[][10],
				*condition_text[],
				*command_text[],
				*s_command_text[];

//---------------------------------------------------------------


UWORD	command_field_widths[][MAX_FIELDS]	=
{
	{	COM_LIST_WIDTH-2,	0,				0,				0				},	//	COM_NONE		
	{	FIELD_1_WIDTH,		0,				FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_ATTACK_PLAYER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_ATTACK_THING
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_ATTACK_GROUP
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_ATTACK_CLASS
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_DEFEND_PLAYER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_DEFEND_THING
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_DEFEND_GROUP
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_DEFEND_CLASS
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_PATROL_WAYPOINT
	{	FIELD_1_WIDTH,		0,				FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_START_TIMER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_WAIT_FOR_TRIGGER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_WAIT_FOR_CLIST
	{	FIELD_1_WIDTH,		0,				FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	COM_FOLLOW_PLAYER
	{	0,					0,				0,				0				}
};

CommandTab			*the_com_tab;

//---------------------------------------------------------------
//	Callback functions.
//---------------------------------------------------------------

void	draw_comlists_box(void)
{
	the_com_tab->DrawListsBox();
}

void	draw_comlist_box(void)
{
	the_com_tab->DrawCurrentList();
}

//---------------------------------------------------------------
//	CommandTab
//---------------------------------------------------------------

CommandTab::CommandTab()
{
	CurrentComList	=	NULL;
	the_com_tab		=	this;
	TabMode			=	COM_MODE_NONE;

	InitControlSet(com_tab_def);

	((CHSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->SetUpdateFunction(draw_comlists_box);
	((CHSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->SetUpdateFunction(draw_comlist_box);
}

//---------------------------------------------------------------

CommandTab::~CommandTab()
{

}

//---------------------------------------------------------------

void	CommandTab::DrawTabContent(void)
{
	SLONG			message_height,
					message_width;
	EdRect			message_rect;


	SetTabDrawArea();
	ClearTab();

	DrawControlSet();

	DrawListsBox();
	DrawCurrentList();

	switch(TabMode)
	{
		case	COM_MODE_NONE:
			break;
		case	COM_MODE_SELECT_THING:
			message_height	=	QTStringHeight()+12;
			message_width	=	QTStringWidth("Select A Thing")+12;
			message_rect.SetRect(
									150-(message_width>>1),
									220-(message_height>>1),
									message_width,
									message_height
								);
			message_rect.FillRect(RED_COL);
			message_rect.HiliteRect(HILITE_COL,LOLITE_COL);
			QuickTextC	(
							message_rect.GetLeft()+6,
							message_rect.GetTop()+6,
							"Select A Thing",
							0
						);
			break;
		case	COM_MODE_SELECT_WAYPOINT:
			message_height	=	QTStringHeight()+12;
			message_width	=	QTStringWidth("Select A Waypoint")+12;
			message_rect.SetRect(
									150-(message_width>>1),
									220-(message_height>>1),
									message_width,
									message_height
								);
			message_rect.FillRect(RED_COL);
			message_rect.HiliteRect(HILITE_COL,LOLITE_COL);
			QuickTextC	(
							message_rect.GetLeft()+6,
							message_rect.GetTop()+6,
							"Select A Waypoint",
							0
						);
		case	COM_MODE_SELECT_SWITCH:
			message_height	=	QTStringHeight()+12;
			message_width	=	QTStringWidth("Select A Trigger")+12;
			message_rect.SetRect(
									150-(message_width>>1),
									220-(message_height>>1),
									message_width,
									message_height
								);
			message_rect.FillRect(RED_COL);
			message_rect.HiliteRect(HILITE_COL,LOLITE_COL);
			QuickTextC	(
							message_rect.GetLeft()+6,
							message_rect.GetTop()+6,
							"Select A Trigger",
							0
						);
			break;
	}
}

//---------------------------------------------------------------

void	CommandTab::UpdateTab(UBYTE update_level)
{
	if(update_level)
	{
		if(LockWorkScreen())
		{
			switch(update_level)
			{
				case	UPDATE_ALL:
					DrawTabContent();
					break;
				case	UPDATE_LISTS_BOX:
					DrawListsBox();
					break;
				case	UPDATE_CURRENT_LIST:
					DrawCurrentList();
					break;
			}
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
	}
}

//---------------------------------------------------------------

UWORD	CommandTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	UBYTE			update		=	UPDATE_NONE;
	UWORD			select_pos;
	ULONG			control_id	=	0;
	EditComList		*the_comlist;
	MFPoint			local_point;


	if(TabMode)
	{
		return	0;
	}
	if(TabData)
	{
		switch(DataField)
		{
			case	1:
				DataCommand->Data1	=	TabData;
				break;
			case	2:
				DataCommand->Data2	=	TabData;
				break;
			case	3:
				DataCommand->Data3	=	TabData;
				break;
		}
		TabData	=	0;
	}

	SetTabDrawArea();

	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			select_pos	=	ListsHilitePos(&local_point);
			if(select_pos)
			{
				CurrentComList	=	HilitetedList(select_pos);
				if(CurrentComList)
				{
					((CEditText*)GetControlPtr(CTRL_COMLIST_EDIT))->SetEditString(CurrentComList->ComListName);
					if(CurrentComList->CommandCount>MAX_VIEW_COMMANDS)
					{
						((CVSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->SetValueRange(0,CurrentComList->CommandCount-MAX_VIEW_COMMANDS);
					}
					else
					{
						((CVSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->SetValueRange(0,0);
					}
				}
			}
			else
			{
				local_point	=	*clicked_point;
				control_id	=	HandleControlSetClick(flags,&local_point);
				HandleControl(control_id);
			}
			update	=	UPDATE_ALL;
			break;
		case	RIGHT_CLICK:
			select_pos	=	ListsHilitePos(&local_point);
			if(select_pos)
			{
				the_comlist	=	HilitetedList(select_pos);
				if(the_comlist)
				{
					DoComListPopup(&local_point,the_comlist);
					update	=	UPDATE_ALL;
				}
			}
			else
			{
				select_pos	=	CurrentListHilitePos(&local_point);
				if(select_pos)
				{
					DoCommandPopup(&local_point,select_pos);
					update	=	UPDATE_ALL;
				}
			}
			break;
	}

	UpdateTab(update);

	return	0;
}

//---------------------------------------------------------------

void	CommandTab::HandleTab(MFPoint *current_point)
{
	UBYTE		update	=	UPDATE_NONE;
	EdRect		command_rect,
				lists_rect;
	MFPoint		local_point;
	static BOOL	cleanup	=	FALSE;
	

	if(TabMode)
	{
		cleanup	=	TRUE;
		return;
	}
	if(TabData)
	{
		switch(DataField)
		{
			case	1:
				DataCommand->Data1	=	TabData;
				break;
			case	2:
				DataCommand->Data2	=	TabData;
				break;
			case	3:
				DataCommand->Data3	=	TabData;
				break;
		}
		TabData	=	0;
	}

	ModeTab::HandleTab(current_point);

	local_point	=	*current_point;
	GlobalToLocal(&local_point);

	command_rect.SetRect(COM_LIST_X,COM_LIST_Y,COM_LIST_WIDTH,COM_LIST_HEIGHT);
	lists_rect.SetRect(LISTS_X,LISTS_Y,LISTS_WIDTH,LISTS_HEIGHT);
	if(lists_rect.PointInRect(&local_point) && !cleanup)
	{
		update	=	UPDATE_LISTS_BOX;
		cleanup	=	TRUE;
	}
	else if(command_rect.PointInRect(&local_point) && !cleanup)
	{
		update	=	UPDATE_CURRENT_LIST;
		cleanup	=	TRUE;
	}

	if(update==UPDATE_NONE && cleanup)
	{
		update	=	UPDATE_ALL;
		cleanup	=	FALSE;
	}

	UpdateTab(update);
}

//---------------------------------------------------------------

void	CommandTab::HandleControl(UWORD control_id)
{
	SLONG		control	=	control_id&0xff;


	switch(control)
	{
		case	CTRL_NEW_COMLIST:
			CurrentComList	=	alloc_ed_comlist();
			if(CurrentComList)
			{
				// set up the new command list.
				if(ed_comlist_count>MAX_VIEW_LISTS)
				{
					((CVSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->SetValueRange(0,ed_comlist_count-MAX_VIEW_LISTS);
				}
				((CEditText*)GetControlPtr(CTRL_COMLIST_EDIT))->SetEditString(CurrentComList->ComListName);

				// Allocate the first command.
				add_command(CurrentComList,alloc_ed_command());
			}
			break;
		case	CTRL_LISTS_SLIDER:
			break;
		case	CTRL_COMLIST_EDIT:
			strcpy(CurrentComList->ComListName,((CEditText*)GetControlPtr(CTRL_COMLIST_EDIT))->GetEditString());
			break;
		case	CTRL_COMLIST_SLIDER:
			break;
		case	CTRL_NEW_COMMAND:
			if(CurrentComList)
			{
				// Create a new command.
				add_command(CurrentComList,alloc_ed_command());
				if(CurrentComList->CommandCount>MAX_VIEW_COMMANDS)
				{
					((CVSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->SetValueRange(0,CurrentComList->CommandCount-MAX_VIEW_COMMANDS);
				}
			}
			break;
	}
}

//---------------------------------------------------------------

void	CommandTab::DoComListPopup(MFPoint *clicked_point,EditComList *the_comlist)
{
	ULONG			control_id		=	0;
	CPopUp			*the_control	=	0;


	comlist_popup_def.ControlLeft	=	clicked_point->X+4;
	comlist_popup_def.ControlTop	=	clicked_point->Y-4;

	comlist_popup_def.TheMenuDef	=	comlist_popup;
	the_control		=	new CPopUp(&comlist_popup_def);
	control_id		=	the_control->TrackControl(clicked_point);

	switch(control_id>>8)
	{
		case	0:
			break;
		case	1:
			if(CurrentComList==the_comlist)
			{
				CurrentComList	=	NULL;
				((CEditText*)GetControlPtr(CTRL_COMLIST_EDIT))->SetEditString("");
			}
			if(ed_comlist_count>MAX_VIEW_LISTS)
			{
				((CVSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->SetValueRange(0,(ed_comlist_count-MAX_VIEW_LISTS)-1);
			}
			free_ed_comlist(the_comlist);
			break;
	}
}

//---------------------------------------------------------------

void	CommandTab::DoCommandPopup(MFPoint *clicked_point,UWORD select_pos)
{
	UBYTE			field;
	ULONG			control_id		=	0;
	CPopUp			*the_control	=	0;
	EditCommand		*the_command;
	EditCondList	*the_cond_list;


	the_command	=	HilitetedCommand(select_pos);
	if(the_command)
	{
		command_popup_def.ControlLeft	=	clicked_point->X+4;
		command_popup_def.ControlTop	=	clicked_point->Y-4;

		field	=	(select_pos&0x00ff)-1;
		if(field==3)
		{
			switch(the_command->Data2)
			{
				case	COM_S_NONE:
					command_popup_def.TheMenuDef	=	generic_popup2;
					break;
				case	COM_S_UNTIL_TRIGGER:
					command_popup_def.TheMenuDef	=	select_switch_popup2;
					break;
				case	COM_S_UNTIL_CLIST:
					command_popup_def.TheMenuDef	=	select_clist_popup;
					break;
				case	COM_S_WHILE_TRIGGER:
					command_popup_def.TheMenuDef	=	select_switch_popup2;
					break;
				case	COM_S_WHILE_CLIST:
					command_popup_def.TheMenuDef	=	select_clist_popup;
					break;
			}
		}
		else
			command_popup_def.TheMenuDef	=	command_defs[the_command->CommandType][field];

		the_control		=	new CPopUp(&command_popup_def);
		control_id		=	(the_control->TrackControl(clicked_point))>>8;

		if(control_id<=5)
			CommonCommandOptions(control_id,the_command);
		else
		{
			// Set the data fields.
			if(field)
			{
				if(field==2)
				{
					the_command->Data2	=	control_id-6;
					the_command->Data3	=	0;
				}
				else if(field==3)
				{
					switch(the_command->Data2)
					{
						case	COM_S_NONE:
							break;
						case	COM_S_UNTIL_TRIGGER:
						case	COM_S_WHILE_TRIGGER:
							TabMode		=	COM_MODE_SELECT_SWITCH;
							DataCommand	=	the_command;
							DataField	=	3;
							break;
						case	COM_S_UNTIL_CLIST:
						case	COM_S_WHILE_CLIST:
							the_cond_list	=	SelectConditionList();
							if(the_cond_list)
							{
								the_command->Data3	=	ED_CONLIST_NUMBER(the_cond_list);
							}
							break;
					}
				}
				else
				{
					switch(the_command->CommandType)
					{
						case	COM_NONE:
							break;
						case	COM_ATTACK_PLAYER:
							break;
						case	COM_ATTACK_THING:
							TabMode		=	COM_MODE_SELECT_THING;
							DataCommand	=	the_command;
							DataField	=	1;
							break;
						case	COM_ATTACK_GROUP:
							break;
						case	COM_ATTACK_CLASS:
							break;
						case	COM_DEFEND_PLAYER:
							break;
						case	COM_DEFEND_THING:
							TabMode		=	COM_MODE_SELECT_THING;
							DataCommand	=	the_command;
							DataField	=	1;
							break;
						case	COM_DEFEND_GROUP:
							break;
						case	COM_DEFEND_CLASS:
							break;
						case	COM_PATROL_WAYPOINT:
							TabMode		=	COM_MODE_SELECT_WAYPOINT;
							DataCommand	=	the_command;
							DataField	=	1;
							break;
						case	COM_START_TIMER:
							break;
						case	COM_WAIT_FOR_TRIGGER:
							TabMode		=	COM_MODE_SELECT_SWITCH;
							DataCommand	=	the_command;
							DataField	=	1;
							break;
						case	COM_WAIT_FOR_CLIST:
							the_cond_list	=	SelectConditionList();
							if(the_cond_list)
							{
								the_command->Data1	=	ED_CONLIST_NUMBER(the_cond_list);
							}
							break;
						case	COM_FOLLOW_PLAYER:
							break;
					}
				}
			}
			else
			{
				the_command->CommandType	=	control_id-6;
			}
		}
	}
}

//---------------------------------------------------------------

void	CommandTab::CommonCommandOptions(ULONG id,EditCommand *the_command)
{
	switch(id)
	{
		case	0:	// NULL.
			break;
		case	1:	// Delete Condition.
			if(the_command)
			{
				if(CurrentComList->CommandCount>MAX_VIEW_COMMANDS)
				{
					((CVSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->SetValueRange(0,(CurrentComList->CommandCount>MAX_VIEW_COMMANDS)-1);
				}
			}
			remove_command(CurrentComList,the_command);
			free_ed_command(the_command);
			break;
		case	2:	// Blank.
			break;
		case	3:	// Group Commands.
			break;
		case	4:	// Ungroup Commands.
			break;
		case	5:	// Blank.
			break;
	}
}

//---------------------------------------------------------------

EditCondList	*CommandTab::SelectConditionList(void)
{
	BOOL			exit		=	FALSE;
	UBYTE			update		=	2;
	UWORD			select_pos;
	SLONG			c0;
	ControlSet		select_set;
	EditCondList	*current_list,
					*hilited_list,
					*selected_list	=	NULL;
	EdRect			bounds_rect,
					item_rect,
					lists_rect;
	MFPoint			current_point;


	SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
	bounds_rect.SetRect	(
							(WorkScreenPixelWidth-(LISTS_WIDTH+20))>>1,
							(WorkScreenHeight-(LISTS_HEIGHT+7))>>1,
							LISTS_WIDTH+20,
							LISTS_HEIGHT+7
						);
	select_set.ControlSetBounds(&bounds_rect);
	select_set.InitControlSet(select_clist_def);
	if(ed_clist_count>MAX_VIEW_LISTS)
	{
		((CVSlider*)select_set.GetControlPtr(1))->SetValueRange(0,ed_clist_count-MAX_VIEW_LISTS);
	}
	((CVSlider*)select_set.GetControlPtr(1))->SetCurrentValue(0);

	while(SHELL_ACTIVE && !exit)
	{
		SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
		current_point.X	=	MouseX;
		current_point.Y	=	MouseY;

		if(select_set.HandleControlSet(&current_point))
			update	=	1;

		lists_rect.SetRect(LISTS_X,LISTS_Y-30,LISTS_WIDTH,LISTS_HEIGHT);
		current_point.X	-=	bounds_rect.GetLeft();
		current_point.Y	-=	bounds_rect.GetTop();
		select_pos	=	0;
		if(lists_rect.PointInRect(&current_point))
		{
			for(c0=0;c0<MAX_VIEW_LISTS;c0++)
			{
				// Create a bounding rect for the list item.
				item_rect.SetRect	(
										lists_rect.GetLeft()+2,
										lists_rect.GetTop()+1+(c0*QTStringHeight()),
										lists_rect.GetWidth()-2,
										QTStringHeight()
									);
				if(item_rect.PointInRect(&current_point))
				{
					select_pos	=	c0+1;
					update		=	2;
					break;
				}
			}

		}

		if(LeftMouse.ButtonState)
		{
			//	If we clicked on a hilited item return that item.
			if(select_pos)
			{
				selected_list	=	hilited_list;
				exit			=	TRUE;
			}
			else
			{
				//	Handle the control.
				SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
				current_point.X	=	MouseX;
				current_point.Y	=	MouseY;
				select_set.HandleControlSetClick(LEFT_CLICK,&current_point);
				update	=	2;
			}
			LeftMouse.ButtonState	=	0;
		}
		if(LastKey==KB_ESC)
		{
			exit	=	1;
			LastKey	=	0;
		}

		if(update)
		{
			if(LockWorkScreen())
			{
				if(update==2)
				{
					SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
					bounds_rect.FillRect(CONTENT_COL);
					bounds_rect.HiliteRect(HILITE_COL,LOLITE_COL);
					SetWorkWindowBounds(bounds_rect.GetLeft(),bounds_rect.GetTop(),bounds_rect.GetWidth(),bounds_rect.GetHeight());

					lists_rect.SetRect(LISTS_X,LISTS_Y-30,LISTS_WIDTH,LISTS_HEIGHT);
					lists_rect.FillRect(ACTIVE_COL);

					// Skip the beginning of the list to match the slider bar position.
					current_list	=	clists;
					c0	=	((CVSlider*)select_set.GetControlPtr(1))->GetCurrentValue();
					while(current_list && c0)
					{
						c0--;
						current_list	=	current_list->Next;
					}

					c0	=	0;
					hilited_list	=	NULL;
					while(current_list && c0<MAX_VIEW_LISTS)
					{
						// Create a bounding rect for the list text.
						item_rect.SetRect	(
												lists_rect.GetLeft()+2,
												lists_rect.GetTop()+1+(c0*QTStringHeight()),
												lists_rect.GetWidth()-2,
												QTStringHeight()+1
											);

						// Hilite the selected one.
						if(select_pos==(c0+1))
						{
							item_rect.FillRect(HILITE_COL);
							hilited_list	=	current_list;
						}

						QuickTextC(item_rect.GetLeft(),item_rect.GetTop(),current_list->CListName,0);

						c0++;
						current_list	=	current_list->Next;
					}

					lists_rect.HiliteRect(LOLITE_COL,HILITE_COL);
				}

				if(update<=2)
				{
					select_set.DrawControlSet();
				}

				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
			update	=	0;
		}
	}
	select_set.FiniControlSet();

	return	selected_list;
}

//---------------------------------------------------------------

void	CommandTab::DrawListsBox(void)
{
	UWORD			select_pos;
	SLONG			c0;
	EditComList		*current_list;
	EdRect			item_rect,
					lists_rect;
	MFPoint			local_point;


	SetTabDrawArea();

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;
	GlobalToLocal(&local_point);

	select_pos	=	ListsHilitePos(&local_point);

	lists_rect.SetRect(LISTS_X,LISTS_Y,LISTS_WIDTH,LISTS_HEIGHT);
	lists_rect.FillRect(ACTIVE_COL);

	// Skip the beginning of the list to match the slider bar position.
	current_list	=	comlists;
	c0	=	((CVSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->GetCurrentValue();
	while(current_list && c0)
	{
		c0--;
		current_list	=	current_list->Next;
	}

	c0	=	0;
	while(current_list && c0<MAX_VIEW_LISTS)
	{
		// Create a bounding rect for the list text.
		item_rect.SetRect	(
								lists_rect.GetLeft()+2,
								lists_rect.GetTop()+1+(c0*QTStringHeight()),
								lists_rect.GetWidth()-2,
								QTStringHeight()+1
							);

		// Hilite the selected one.
		if(select_pos==(c0+1))
			item_rect.FillRect(HILITE_COL);
		else if(CurrentComList==current_list)
			item_rect.FillRect(SELECT_COL);

		QuickTextC(item_rect.GetLeft(),item_rect.GetTop(),current_list->ComListName,0);

		c0++;
		current_list	=	current_list->Next;
	}
	lists_rect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

void	CommandTab::DrawCurrentList(void)
{
	CBYTE			field_text[MAX_FIELDS][64];
	UWORD			select_pos;
	SLONG			c0,c1,
					x_pos;
	EditCommand		*current_command;
	EdRect			field_rects[MAX_FIELDS],
					list_rect;
	MFPoint			local_point;


	SetTabDrawArea();

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;
	GlobalToLocal(&local_point);

	select_pos	=	CurrentListHilitePos(&local_point);

	list_rect.SetRect(COM_LIST_X,COM_LIST_Y,COM_LIST_WIDTH,COM_LIST_HEIGHT);
	list_rect.FillRect(ACTIVE_COL);

	if(CurrentComList)
	{
		// Skip the beginning of the list to match the slider bar position.
		current_command	=	CurrentComList->CommandList;
		if(current_command)
		{
			c0	=	((CVSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->GetCurrentValue();
			while(current_command && c0)
			{
				c0--;
				current_command	=	current_command->Next;
			}

			c0	=	0;
			while(current_command && c0<MAX_VIEW_COMMANDS)
			{
				// Create bounding rects for all the condition fields.
				x_pos	=	list_rect.GetLeft()+2;
				for(c1=0;c1<MAX_FIELDS;c1++)
				{
					field_rects[c1].SetRect	(
												x_pos,
												list_rect.GetTop()+1+(c0*QTStringHeight()),
												command_field_widths[current_command->CommandType][c1],
												QTStringHeight()+1
											);
					x_pos	+=	command_field_widths[current_command->CommandType][c1];
				}

				// Hilite the field that the mouse is currently over.
				if((select_pos>>8)==(c0+1))
				{
					field_rects[(select_pos&0x00ff)-1].FillRect(HILITE_COL);
				}


				// Clear all the field text.
				ZeroMemory(field_text,sizeof(field_text));

				// Set the commands field text
				sprintf(&field_text[0][0],"%s",command_text[current_command->CommandType]);

				// & it's data.
				switch(current_command->CommandType)
				{
					case	COM_NONE:
						break;
					case	COM_ATTACK_PLAYER:
						break;
					case	COM_ATTACK_THING:
						sprintf(&field_text[1][0],"Thing %d",current_command->Data1);
						break;
					case	COM_ATTACK_GROUP:
						break;
					case	COM_ATTACK_CLASS:
						break;
					case	COM_DEFEND_PLAYER:
						break;
					case	COM_DEFEND_THING:
						sprintf(&field_text[1][0],"Thing %d",current_command->Data1);
						break;
					case	COM_DEFEND_GROUP:
						break;
					case	COM_DEFEND_CLASS:
						break;
					case	COM_PATROL_WAYPOINT:
						sprintf(&field_text[1][0],"Waypoint %d",current_command->Data1);
						break;
					case	COM_START_TIMER:
						break;
					case	COM_WAIT_FOR_TRIGGER:
						sprintf(&field_text[1][0],"%d",current_command->Data1);
						break;
					case	COM_WAIT_FOR_CLIST:
						if(current_command->Data1)
						{
							sprintf(&field_text[1][0],"%s",edit_clists[current_command->Data1].CListName);
						}
						else
						{
							sprintf(&field_text[1][0],"None");
						}
						break;
					case	COM_FOLLOW_PLAYER:
						break;
				}

				if(current_command->CommandType)
				{
					//	Now set the secondary commands field text
					sprintf(&field_text[2][0],"%s",s_command_text[current_command->Data2]);

					//	& it's data.
					switch(current_command->Data2)
					{
						case	COM_S_NONE:
							break;
						case	COM_S_UNTIL_TRIGGER:
						case	COM_S_WHILE_TRIGGER:
 							sprintf(&field_text[3][0],"    %d",current_command->Data3);
							break;
						case	COM_S_UNTIL_CLIST:
						case	COM_S_WHILE_CLIST:
							if(current_command->Data3)
							{
								sprintf(&field_text[3][0],"%s",edit_clists[current_command->Data3].CListName);
							}
							else
							{
								sprintf(&field_text[3][0],"None");
							}
							break;
							break;
					}
				}

				// Draw all field text.
				for(c1=0;c1<MAX_FIELDS;c1++)
				{
					QuickTextC	(
									field_rects[c1].GetLeft(),
									field_rects[c1].GetTop(),
									field_text[c1],
									0
								);
				}

				c0++;
				current_command	=	current_command->Next;
			}
		}
	}
	list_rect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

UWORD	CommandTab::ListsHilitePos(MFPoint *current_point)
{
	UWORD			c0;
	EdRect			item_rect,
					lists_rect;

	
	lists_rect.SetRect(LISTS_X,LISTS_Y,LISTS_WIDTH,LISTS_HEIGHT);

	if(lists_rect.PointInRect(current_point))
	{
		for(c0=0;c0<MAX_VIEW_LISTS;c0++)
		{
			// Create a bounding rect for the list item.
			item_rect.SetRect	(
									lists_rect.GetLeft()+2,
									lists_rect.GetTop()+1+(c0*QTStringHeight()),
									lists_rect.GetWidth()-2,
									QTStringHeight()
								);
			if(item_rect.PointInRect(current_point))
			{
				return	c0+1;
			}
		}
	}
	return	0;
}

//---------------------------------------------------------------

EditComList	*CommandTab::HilitetedList(UWORD select_pos)
{
	ULONG			c0;
	EditComList		*current_list;

	
	current_list	=	comlists;

	//	Skip the beginning of the list to match the slider bar position.
	c0	=	((CVSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->GetCurrentValue();
	while(current_list && c0)
	{
		c0--;
		current_list	=	current_list->Next;
	}

	//	Now skip to the hilited item.
	select_pos--;
	while(current_list && select_pos)
	{
		select_pos--;
		current_list	=	current_list->Next;
	}

	return	current_list;
}

//---------------------------------------------------------------

UWORD	CommandTab::CurrentListHilitePos(MFPoint *current_point)
{
	UWORD			c0,c1;
	SLONG			x_pos;
	EditCommand		*current_command;
	EdRect			command_rect,
					field_rects[MAX_FIELDS],
					list_rect;


	if(CurrentComList)
	{
		list_rect.SetRect(COM_LIST_X,COM_LIST_Y,COM_LIST_WIDTH,COM_LIST_HEIGHT);

		if(list_rect.PointInRect(current_point))
		{
			for(c0=0;c0<MAX_VIEW_COMMANDS;c0++)
			{
				// Get the current command.
				current_command	=	HilitetedCommand((c0+1)<<8);

				if(current_command)
				{
					// Create a bounding rect for the command field.
					command_rect.SetRect	(
												list_rect.GetLeft()+2,
												list_rect.GetTop()+1+(c0*QTStringHeight()),
												list_rect.GetWidth()-2,
												QTStringHeight()
											);
					if(command_rect.PointInRect(current_point))
					{
						// Create bounding rects for the command fields.
						x_pos	=	list_rect.GetLeft()+2;
						for(c1=0;c1<MAX_FIELDS;c1++)
						{
							field_rects[c1].SetRect	(
														x_pos,
														list_rect.GetTop()+1+(c0*QTStringHeight()),
														command_field_widths[current_command->CommandType][c1],
														QTStringHeight()+1
													);
							x_pos	+=	command_field_widths[current_command->CommandType][c1];

							if(field_rects[c1].PointInRect(current_point))
							{
								return	((c0+1)<<8)|(c1+1);
							}
						}
					}
				}
			}
		}
	}
	return	0;
}

//---------------------------------------------------------------

EditCommand	*CommandTab::HilitetedCommand(UWORD select_pos)
{
	ULONG			c0;
	EditCommand		*current_command	=	NULL;


	if(CurrentComList)
	{
		current_command	=	CurrentComList->CommandList;

		//	Skip the beginning of the list to match the slider bar position.
		c0	=	((CVSlider*)GetControlPtr(CTRL_COMLIST_SLIDER))->GetCurrentValue();
		while(current_command && c0)
		{
			c0--;
			current_command	=	current_command->Next;
		}

		//	Now skip to the hilited command.
		select_pos	=	(select_pos>>8)-1;
		while(current_command && select_pos)
		{
			select_pos--;
			current_command	=	current_command->Next;
		}
	}

	return	current_command;
}

//---------------------------------------------------------------

