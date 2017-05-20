// CondTab.cpp
// Guy Simmons, 16th March 1998.

#include	"Editor.hpp"
#include	"EdCom.h"

#include	"..\..\Headers\Game.h"
#include	"..\..\Headers\Command.h"


//---------------------------------------------------------------

#define	MAX_VIEW_LISTS		15
#define	MAX_VIEW_CONDS		24

#define	LISTS_X				2
#define	LISTS_Y				32
#define	LISTS_WIDTH			150
#define	LISTS_HEIGHT		((MAX_VIEW_LISTS*8)+3)

#define	CON_LIST_X			2
#define	CON_LIST_Y			226
#define	CON_LIST_WIDTH		(280)
#define	CON_LIST_HEIGHT		((MAX_VIEW_CONDS*8)+3)

#define	CON_NAME_X			2
#define	CON_NAME_Y			(CON_LIST_Y-14)
#define	CON_NAME_WIDTH		LISTS_WIDTH

#define	CTRL_NEW_CLIST		1
#define	CTRL_LISTS_SLIDER	2
#define	CTRL_CLIST_EDIT		3
#define	CTRL_CLIST_SLIDER	4
#define	CTRL_NEW_COND		5

#define	UPDATE_NONE			0
#define	UPDATE_ALL			1
#define	UPDATE_LISTS_BOX	2
#define	UPDATE_CURRENT_LIST	3

#define	MAX_FIELDS			4
#define	FIELD_1_WIDTH		112
#define	FIELD_2_WIDTH		56
#define	FIELD_3_WIDTH		56
#define	FIELD_4_WIDTH		56

#include	"CondTab.def"

extern	CBYTE	*class_text[];
extern	CBYTE	*genus_text[][10];
extern	CBYTE	*condition_text[];

//---------------------------------------------------------------

UWORD	field_widths[][MAX_FIELDS]	=
{
	{	CON_LIST_WIDTH-2,	0,				0,				0				},	//	CON_NONE
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	0,				0				},	//	CON_THING_DEAD
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	0,				0				},	//	CON_ALL_GROUP_DEAD
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	CON_PERCENT_GROUP_DEAD
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	0				},	//	CON_THING_NEAR_PLAYER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	0				},	//	CON_GROUP_NEAR_PLAYER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	0				},	//	CON_CLASS_NEAR_PLAYER
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	CON_THING_NEAR_THING
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	CON_GROUP_NEAR_THING
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	FIELD_4_WIDTH	},	//	CON_CLASS_NEAR_THING
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	0				},	//	CON_CLASS_COUNT
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	FIELD_3_WIDTH,	0				},	//	CON_GROUP_COUNT
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	0,				0				},	//	CON_SWITCH_TRIGGERED
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	0,				0				},	//	CON_TIME
	{	FIELD_1_WIDTH,		FIELD_2_WIDTH,	0,				0				},	//	CON_CLIST_FULFILLED
	{	0,					0,				0,				0				}
};
ConditionTab		*the_con_tab;

//---------------------------------------------------------------
//	Callback functions.
//---------------------------------------------------------------

void	draw_lists_box(void)
{
	the_con_tab->DrawListsBox();
}

void	draw_clist_box(void)
{
	the_con_tab->DrawCurrentList();
}

//---------------------------------------------------------------
//	ConditionTab
//---------------------------------------------------------------

ConditionTab::ConditionTab()
{
	CurrentCList	=	NULL;
	the_con_tab		=	this;
	TabMode			=	COND_MODE_NONE;

	InitControlSet(cond_tab_def);

	((CHSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->SetUpdateFunction(draw_lists_box);
	((CHSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->SetUpdateFunction(draw_clist_box);
}

//---------------------------------------------------------------

ConditionTab::~ConditionTab()
{
}

//---------------------------------------------------------------

void	ConditionTab::DrawTabContent(void)
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
		case	COND_MODE_NONE:
			break;
		case	COND_MODE_SELECT_THING:
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
		case	COND_MODE_SELECT_SWITCH:
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

void	ConditionTab::UpdateTab(UBYTE update_level)
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

UWORD	ConditionTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	UBYTE			update		=	UPDATE_NONE;
	UWORD			select_pos;
	ULONG			control_id	=	0;
	EditCondList	*the_clist;
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
				DataCondition->Data1	=	TabData;
				break;
			case	2:
				DataCondition->Data2	=	TabData;
				break;
			case	3:
				DataCondition->Data3	=	TabData;
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
				CurrentCList	=	HilitetedList(select_pos);
				if(CurrentCList)
				{
					((CEditText*)GetControlPtr(CTRL_CLIST_EDIT))->SetEditString(CurrentCList->CListName);
					if(CurrentCList->ConditionCount>MAX_VIEW_CONDS)
					{
						((CVSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->SetValueRange(0,CurrentCList->ConditionCount-MAX_VIEW_CONDS);
					}
					else
					{
						((CVSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->SetValueRange(0,0);
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
				the_clist	=	HilitetedList(select_pos);
				if(the_clist)
				{
					DoCListPopup(&local_point,the_clist);
					update	=	UPDATE_ALL;
				}
			}
			else
			{
				select_pos	=	CurrentListHilitePos(&local_point);
				if(select_pos)
				{
					DoConditionPopup(&local_point,select_pos);
					update	=	UPDATE_ALL;
				}
			}
			break;
	}

	UpdateTab(update);

	return	0;
}

//---------------------------------------------------------------

void	ConditionTab::HandleTab(MFPoint *current_point)
{
	UBYTE		update	=	UPDATE_NONE;
	EdRect		condition_rect,
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
				DataCondition->Data1	=	TabData;
				break;
			case	2:
				DataCondition->Data2	=	TabData;
				break;
			case	3:
				DataCondition->Data3	=	TabData;
				break;
		}
		TabData	=	0;
	}

	ModeTab::HandleTab(current_point);

	local_point	=	*current_point;
	GlobalToLocal(&local_point);

	condition_rect.SetRect(CON_LIST_X,CON_LIST_Y,CON_LIST_WIDTH,CON_LIST_HEIGHT);
	lists_rect.SetRect(LISTS_X,LISTS_Y,LISTS_WIDTH,LISTS_HEIGHT);
	if(lists_rect.PointInRect(&local_point) && !cleanup)
	{
		update	=	UPDATE_LISTS_BOX;
		cleanup	=	TRUE;
	}
	else if(condition_rect.PointInRect(&local_point) && !cleanup)
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

void	ConditionTab::HandleControl(UWORD control_id)
{
	SLONG		control	=	control_id&0xff;


	switch(control)
	{
		case	CTRL_NEW_CLIST:
			CurrentCList	=	alloc_ed_clist();
			if(CurrentCList)
			{
				// set up the new condition list.
				if(ed_clist_count>MAX_VIEW_LISTS)
				{
					((CVSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->SetValueRange(0,ed_clist_count-MAX_VIEW_LISTS);
				}
				((CEditText*)GetControlPtr(CTRL_CLIST_EDIT))->SetEditString(CurrentCList->CListName);

				// Allocate the first condition.
				add_condition(CurrentCList,alloc_ed_condition());
			}
			break;
		case	CTRL_LISTS_SLIDER:
			break;
		case	CTRL_CLIST_EDIT:
			strcpy(CurrentCList->CListName,((CEditText*)GetControlPtr(CTRL_CLIST_EDIT))->GetEditString());			
			break;
		case	CTRL_CLIST_SLIDER:
			break;
		case	CTRL_NEW_COND:
			if(CurrentCList)
			{
				// Create a new condition.
				add_condition(CurrentCList,alloc_ed_condition());
				if(CurrentCList->ConditionCount>MAX_VIEW_CONDS)
				{
					((CVSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->SetValueRange(0,CurrentCList->ConditionCount-MAX_VIEW_CONDS);
				}
			}
			break;
	}
}

//---------------------------------------------------------------

void	ConditionTab::DoCListPopup(MFPoint *clicked_point,EditCondList *the_clist)
{
	ULONG			control_id		=	0;
	CPopUp			*the_control	=	0;


	clist_popup_def.ControlLeft	=	clicked_point->X+4;
	clist_popup_def.ControlTop	=	clicked_point->Y-4;

	if(the_clist==win_conditions || the_clist==lose_conditions)
	{
		clist_popup[0].ItemFlags	=	MENU_INACTIVE;
	}
	else
	{
		clist_popup[0].ItemFlags	=	0;
	}

	clist_popup_def.TheMenuDef	=	clist_popup;
	the_control		=	new CPopUp(&clist_popup_def);
	control_id		=	the_control->TrackControl(clicked_point);

	switch(control_id>>8)
	{
		case	0:
			break;
		case	1:
			if(CurrentCList==the_clist)
			{
				CurrentCList	=	NULL;
				((CEditText*)GetControlPtr(CTRL_CLIST_EDIT))->SetEditString("");
			}
			if(ed_clist_count>MAX_VIEW_LISTS)
			{
				((CVSlider*)GetControlPtr(CTRL_LISTS_SLIDER))->SetValueRange(0,(ed_clist_count-MAX_VIEW_LISTS)-1);
			}
			free_ed_clist(the_clist);
			break;
	}
}

//---------------------------------------------------------------

void	ConditionTab::DoConditionPopup(MFPoint *clicked_point,UWORD select_pos)
{
	UBYTE			field;
	ULONG			control_id		=	0;
	CPopUp			*the_control	=	0;
	EditCondition	*the_condition;
	EditCondList	*the_cond_list;


	the_condition	=	HilitetedCondition(select_pos);
	if(the_condition)
	{
		condition_popup_def.ControlLeft	=	clicked_point->X+4;
		condition_popup_def.ControlTop	=	clicked_point->Y-4;

		field	=	(select_pos&0x00ff)-1;
		condition_popup_def.TheMenuDef	=	condition_defs[the_condition->ConditionType][field];

		the_control		=	new CPopUp(&condition_popup_def);
		control_id		=	(the_control->TrackControl(clicked_point))>>8;

		if(control_id<=5)
			CommonConditionOptions(control_id,the_condition);
		else
		{
			// Set the data fields.
			if(field)
			{
				switch(the_condition->ConditionType)
				{
					case	CON_NONE:
						break;
					case	CON_THING_DEAD:
						TabMode			=	COND_MODE_SELECT_THING;
						DataCondition	=	the_condition;
						DataField		=	1;
						break;
					case	CON_ALL_GROUP_DEAD:
						the_condition->Data1	=	control_id-6;
						break;
					case	CON_PERCENT_GROUP_DEAD:
						the_condition->Data1	=	control_id-6;
						break;
					case	CON_THING_NEAR_PLAYER:
						TabMode			=	COND_MODE_SELECT_THING;
						DataCondition	=	the_condition;
						DataField		=	1;
						break;
					case	CON_GROUP_NEAR_PLAYER:
						the_condition->Data1	=	(control_id-6)+1;
						break;
					case	CON_CLASS_NEAR_PLAYER:
						the_condition->Data1	=	(control_id-6)+1;
						break;
					case	CON_THING_NEAR_THING:
						if(field<=2)
						{
							TabMode			=	COND_MODE_SELECT_THING;
							DataCondition	=	the_condition;
							DataField		=	field;
						}
						break;
					case	CON_GROUP_NEAR_THING:
						if(field==1)
						{
							the_condition->Data1	=	control_id-6;
						}
						else if(field==2)
						{
							TabMode			=	COND_MODE_SELECT_THING;
							DataCondition	=	the_condition;
							DataField		=	2;
						}
						break;
					case	CON_CLASS_NEAR_THING:
						if(field==1)
						{
							the_condition->Data1	=	(control_id-6)+1;
						}
						else if(field==2)
						{
							TabMode			=	COND_MODE_SELECT_THING;
							DataCondition	=	the_condition;
							DataField		=	2;
						}
						break;
					case	CON_CLASS_COUNT:
						break;
					case	CON_GROUP_COUNT:
						break;
					case	CON_SWITCH_TRIGGERED:
						TabMode			=	COND_MODE_SELECT_SWITCH;
						DataCondition	=	the_condition;
						DataField		=	1;
						break;
					case	CON_TIME:
						break;
					case	CON_CLIST_FULFILLED:
						if(field==1)
						{
							the_cond_list	=	SelectConditionList();
							if(the_cond_list)
							{
								the_condition->Data1	=	ED_CONLIST_NUMBER(the_cond_list);
							}
						}
						break;
				}
			}
			else
			{
				the_condition->ConditionType	=	control_id-6;
			}
		}
	}
}

//---------------------------------------------------------------

void	ConditionTab::CommonConditionOptions(ULONG id,EditCondition *the_condition)
{
	switch(id)
	{
		case	0:	// NULL.
			break;
		case	1:	// Delete Condition.
			if(the_condition)
			{
				if(CurrentCList->ConditionCount>MAX_VIEW_CONDS)
				{
					((CVSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->SetValueRange(0,(CurrentCList->ConditionCount>MAX_VIEW_CONDS)-1);
				}
			}
			remove_condition(CurrentCList,the_condition);
			free_ed_condition(the_condition);
			break;
		case	2:	// Blank.
			break;
		case	3:	// Group Conditions.
			break;
		case	4:	// Ungroup Conditons.
			break;
		case	5:	// Blank.
			break;
	}
}

//---------------------------------------------------------------

EditCondList	*ConditionTab::SelectConditionList(void)
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

void	ConditionTab::DrawListsBox(void)
{
	UWORD			select_pos;
	SLONG			c0;
	EditCondList	*current_list;
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
	current_list	=	clists;
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
		else if(CurrentCList==current_list)
			item_rect.FillRect(SELECT_COL);

		QuickTextC(item_rect.GetLeft(),item_rect.GetTop(),current_list->CListName,0);

		c0++;
		current_list	=	current_list->Next;
	}
	lists_rect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

void	ConditionTab::DrawCurrentList(void)
{
	CBYTE			field_text[MAX_FIELDS][64];
	UWORD			select_pos;
	SLONG			c0,c1,
					x_pos;
	EditCondition	*current_condition;
	EdRect			field_rects[MAX_FIELDS],
					list_rect;
	MFPoint			local_point;


	SetTabDrawArea();

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;
	GlobalToLocal(&local_point);

	select_pos	=	CurrentListHilitePos(&local_point);

	list_rect.SetRect(CON_LIST_X,CON_LIST_Y,CON_LIST_WIDTH,CON_LIST_HEIGHT);
	list_rect.FillRect(ACTIVE_COL);

	if(CurrentCList)
	{
		// Skip the beginning of the list to match the slider bar position.
		current_condition	=	CurrentCList->ConditionList;
		if(current_condition)
		{
			c0	=	((CVSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->GetCurrentValue();
			while(current_condition && c0)
			{
				c0--;
				current_condition	=	current_condition->Next;
			}

			c0	=	0;
			while(current_condition && c0<MAX_VIEW_CONDS)
			{
				// Create bounding rects for all the condition fields.
				x_pos	=	list_rect.GetLeft()+2;
				for(c1=0;c1<MAX_FIELDS;c1++)
				{
					field_rects[c1].SetRect	(
												x_pos,
												list_rect.GetTop()+1+(c0*QTStringHeight()),
												field_widths[current_condition->ConditionType][c1],
												QTStringHeight()+1
											);
					x_pos	+=	field_widths[current_condition->ConditionType][c1];
				}

				// Hilite the field that the mouse is currently over.
				if((select_pos>>8)==(c0+1))
				{
					field_rects[(select_pos&0x00ff)-1].FillRect(HILITE_COL);
				}


				// Clear all the field text.
				ZeroMemory(field_text,sizeof(field_text));

				// Set the condition field text.
				sprintf(&field_text[0][0],"%s",condition_text[current_condition->ConditionType]);

				// Now set the data fields text.
				switch(current_condition->ConditionType)
				{
					case	CON_NONE:
						break;
					case	CON_THING_DEAD:
						sprintf(&field_text[1][0],"Thing %d",current_condition->Data1);
						break;
					case	CON_ALL_GROUP_DEAD:
						sprintf(&field_text[1][0],"Group %d",current_condition->Data1);
						break;
					case	CON_PERCENT_GROUP_DEAD:
						sprintf(&field_text[1][0],"Group %d",current_condition->Data1);
						sprintf(&field_text[2][0],"%d%%",current_condition->Data2);
						break;
					case	CON_THING_NEAR_PLAYER:
						sprintf(&field_text[1][0],"Thing %d",current_condition->Data1);
						sprintf(&field_text[2][0],"%d",current_condition->Data3);
						break;
					case	CON_GROUP_NEAR_PLAYER:
						sprintf(&field_text[1][0],"Group %d",current_condition->Data1);
						sprintf(&field_text[2][0],"%d",current_condition->Data3);
						break;
					case	CON_CLASS_NEAR_PLAYER:
						sprintf(&field_text[1][0],"%s",class_text[current_condition->Data1]);
						sprintf(&field_text[2][0],"%d",current_condition->Data3);
						break;
					case	CON_THING_NEAR_THING:
						sprintf(&field_text[1][0],"Thing %d",current_condition->Data1);
						sprintf(&field_text[2][0],"Thing %d",current_condition->Data2);
						sprintf(&field_text[3][0],"%d",current_condition->Data3);
						break;
					case	CON_GROUP_NEAR_THING:
						sprintf(&field_text[1][0],"Group %d",current_condition->Data1);
						sprintf(&field_text[2][0],"Thing %d",current_condition->Data2);
						sprintf(&field_text[3][0],"%d",current_condition->Data3);
						break;
					case	CON_CLASS_NEAR_THING:
						sprintf(&field_text[1][0],"%s",class_text[current_condition->Data1]);
						sprintf(&field_text[2][0],"Thing %d",current_condition->Data2);
						sprintf(&field_text[3][0],"%d",current_condition->Data3);
						break;
					case	CON_CLASS_COUNT:
						sprintf(&field_text[1][0],"%s",class_text[current_condition->Data1]);
						sprintf(&field_text[2][0],"%d",current_condition->Data2);
						break;
					case	CON_GROUP_COUNT:
						sprintf(&field_text[1][0],"Group %s",genus_text[current_condition->Data3][current_condition->Data1]);
						sprintf(&field_text[2][0],"%d",current_condition->Data2);
						break;
					case	CON_SWITCH_TRIGGERED:
						sprintf(&field_text[1][0],"%d",current_condition->Data1);
						break;
					case	CON_TIME:
						sprintf(&field_text[1][0],"0");
						break;
					case	CON_CLIST_FULFILLED:
						if(current_condition->Data1)
						{
							sprintf(&field_text[1][0],"%s",edit_clists[current_condition->Data1].CListName);
						}
						else
						{
							sprintf(&field_text[1][0],"None");
						}
						break;
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
				current_condition	=	current_condition->Next;
			}
		}
	}
	list_rect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

UWORD	ConditionTab::ListsHilitePos(MFPoint *current_point)
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

EditCondList	*ConditionTab::HilitetedList(UWORD select_pos)
{
	ULONG			c0;
	EditCondList	*current_list;

	
	current_list	=	clists;

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

UWORD	ConditionTab::CurrentListHilitePos(MFPoint *current_point)
{
	UWORD			c0,c1;
	SLONG			x_pos;
	EditCondition	*current_condition;
	EdRect			condition_rect,
					field_rects[MAX_FIELDS],
					list_rect;


	if(CurrentCList)
	{
		list_rect.SetRect(CON_LIST_X,CON_LIST_Y,CON_LIST_WIDTH,CON_LIST_HEIGHT);

		if(list_rect.PointInRect(current_point))
		{
			for(c0=0;c0<MAX_VIEW_CONDS;c0++)
			{
				// Get the current condition.
				current_condition	=	HilitetedCondition((c0+1)<<8);

				if(current_condition)
				{
					// Create a bounding rect for the condition field.
					condition_rect.SetRect	(
												list_rect.GetLeft()+2,
												list_rect.GetTop()+1+(c0*QTStringHeight()),
												list_rect.GetWidth()-2,
												QTStringHeight()
											);
					if(condition_rect.PointInRect(current_point))
					{
						// Create bounding rects for the condition fields.
						x_pos	=	list_rect.GetLeft()+2;
						for(c1=0;c1<MAX_FIELDS;c1++)
						{
							field_rects[c1].SetRect	(
														x_pos,
														list_rect.GetTop()+1+(c0*QTStringHeight()),
														field_widths[current_condition->ConditionType][c1],
														QTStringHeight()+1
													);
							x_pos	+=	field_widths[current_condition->ConditionType][c1];

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

EditCondition	*ConditionTab::HilitetedCondition(UWORD select_pos)
{
	ULONG			c0;
	EditCondition	*current_condition	=	NULL;


	if(CurrentCList)
	{
		current_condition	=	CurrentCList->ConditionList;

		//	Skip the beginning of the list to match the slider bar position.
		c0	=	((CVSlider*)GetControlPtr(CTRL_CLIST_SLIDER))->GetCurrentValue();
		while(current_condition && c0)
		{
			c0--;
			current_condition	=	current_condition->Next;
		}

		//	Now skip to the hilited condition.
		select_pos	=	(select_pos>>8)-1;
		while(current_condition && select_pos)
		{
			select_pos--;
			current_condition	=	current_condition->Next;
		}
	}

	return	current_condition;
}

//---------------------------------------------------------------

