// ThingTab.cpp
// Guy Simmons, 15th January 1998.

#include	"Editor.hpp"


//---------------------------------------------------------------

#define	CTRL_CLASS_MENU			1
#define	CTRL_CLASS_TEXT			2
#define	CTRL_CLASS_CHECKS		3		

#define	CTRL_GENUS_MENU			1
#define	CTRL_GENUS_TEXT			2
#define	CTRL_ITEM_3				3
#define	CTRL_ITEM_4				4

#define	UPDATE_NONE			0
#define	UPDATE_ALL			1
#define	UPDATE_CLASS_SET	2

#define	MAX_VIEW_LISTS		15
#define	LISTS_X				2
#define	LISTS_Y				2
#define	LISTS_WIDTH			150
#define	LISTS_HEIGHT		((MAX_VIEW_LISTS*8)+3)

#include	"ThingTab.def"

//---------------------------------------------------------------

extern	CBYTE	*class_text[];
extern	CBYTE	*genus_text[][10];

//---------------------------------------------------------------

ThingTab::ThingTab()
{
	CurrentClass	=	0;
	CurrentGenus	=	0;
	CurrentThing	=	0;
	ThingFlags		=	0xffff;
	TabData			=	0;
	TabMode			=	THING_MODE_NONE;

	InitControlSet(thing_tab_def);
	UpdateCheckBoxes();
	CurrentSetRect.SetRect(0,30,ContentRect.GetWidth(),100);
}

//---------------------------------------------------------------

ThingTab::~ThingTab()
{
}

//---------------------------------------------------------------

void	ThingTab::DrawTabContent(void)
{
	SLONG			message_height,
					message_width;
	EdRect			message_rect;

	
	SetTabDrawArea();
	ClearTab();

	DrawControlSet();
	DrawClassSet();

	switch(TabMode)
	{
		case	THING_MODE_NONE:
			break;
		case	THING_MODE_SELECT_THING:
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
		case	THING_MODE_SELECT_SWITCH:
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

void	ThingTab::UpdateTab(UBYTE update_level)
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
				case	UPDATE_CLASS_SET:
					DrawClassSet();
					break;
			}
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
	}
}

//---------------------------------------------------------------

UWORD	ThingTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	UBYTE		update	=	UPDATE_NONE;
	ULONG		control_id	=	0;
	MFPoint		local_point;


	if(TabMode)
	{
		return	0;
	}
	if(TabData)
	{
		*DataPtr	=	TabData;
		TabData		=	0;
		update		=	UPDATE_ALL;
		UpdateClassInfo();
	}

	SetTabDrawArea();

	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			local_point	=	*clicked_point;
			control_id	=	HandleControlSetClick(flags,&local_point);
			HandleControl(control_id);

			local_point	=	*clicked_point;
			control_id	=	CurrentSet.HandleControlSetClick(flags,&local_point);
			if(CurrentClass==CLASS_BUILDING)
				HandleBuildingControl(control_id);
			else
				HandleClassControl(control_id);
			update	=	UPDATE_ALL;
			break;
		case	RIGHT_CLICK:
			break;
	}

	UpdateTab(update);

	return	0;
}

//---------------------------------------------------------------

void	ThingTab::HandleTab(MFPoint *current_point)
{
	UBYTE		update	=	UPDATE_NONE;
	static BOOL	cleanup	=	FALSE;

	
	if(TabMode)
	{
		cleanup	=	TRUE;
		return;
	}
	if(TabData)
	{
		*DataPtr	=	TabData;
		TabData		=	0;
		update		=	UPDATE_ALL;
		UpdateClassInfo();
	}

	ModeTab::HandleTab(current_point);
	
	if(CurrentSet.HandleControlSet(current_point))
		cleanup	=	TRUE;

	if(update==UPDATE_NONE && cleanup)
	{
		update	=	UPDATE_ALL;
		cleanup	=	FALSE;
	}

	UpdateTab(update);
}

//---------------------------------------------------------------

void	ThingTab::HandleControl(UWORD control_id)
{
	SLONG		control	=	control_id&0xff;


	if(control)
	{
		if((control) > CTRL_CLASS_CHECKS)
		{
			ThingFlags	^=	(1<<(control-CTRL_CLASS_CHECKS));
			UpdateCheckBoxes();
			
		}
		else
		{
			switch(control)
			{
				case	CTRL_CLASS_MENU:
					CurrentClass	=	(control_id>>8)-1;
					CurrentGenus	=	0;
					break;
			}
			UpdateTabInfo();
			UpdateClassInfo();
		}
	}
}

//---------------------------------------------------------------

void	ThingTab::HandleClassControl(UWORD control_id)
{
	EditComList		*the_list;


	if(control_id)
	{
		switch(control_id&0xff)
		{
			case	CTRL_GENUS_MENU:
				CurrentGenus	=	(control_id>>8);
				if(CurrentClass==CLASS_PERSON)
				{
					CurrentGenus	+=	PERSON_ROPER;
				}
				else if(CurrentClass==CLASS_SWITCH)
				{
					class_defs[CLASS_SWITCH]	=	switch_defs[CurrentGenus];
					UpdateTabInfo();
				}

				if(CurrentThing)
				{
//					map_things[CurrentThing].Genus	=	CurrentGenus;
				}
				break;
			case	CTRL_GENUS_TEXT:
				break;
			case	CTRL_ITEM_3:
				switch(CurrentClass)
				{
					case	CLASS_PLAYER:
						break;
					case	CLASS_CAMERA:
						break;
					case	CLASS_BUILDING:
						break;
					case	CLASS_PERSON:
						the_list	=	SelectCommandList();
						if(the_list)
						{
							map_things[CurrentThing].Data[0]	=	(SLONG)(the_list-edit_comlists);
						}
						break;
					case	CLASS_SWITCH:
						if(CurrentGenus==SWITCH_THING)
						{
							TabMode		=	THING_MODE_SELECT_THING;
							DataPtr		=	&map_things[CurrentThing].Data[1];
						}
						break;
					case	CLASS_VEHICLE:

						//
						// MAKE IT LIKE PEOPLE!
						//

						the_list	=	SelectCommandList();
						if(the_list)
						{
							map_things[CurrentThing].Data[0]	=	(SLONG)(the_list-edit_comlists);
						}
						break;
					case	CLASS_SPECIAL:
						break;
				}
				break;
			case	CTRL_ITEM_4:
				break;
		}
	}
	UpdateClassInfo();
}

//---------------------------------------------------------------

void	ThingTab::HandleBuildingControl(UWORD control_id)
{
	MapThing		*t_mthing;


	if(control_id)
	{
		t_mthing	=	TO_MTHING(CurrentThing);
		switch(control_id&0xff)
		{
			case	0:
				break;
			case	1:
				break;
			case	2:
				t_mthing->EditorFlags	^=	0x01;
				CurrentSet.SetControlState(2,(t_mthing->EditorFlags&0x01 ? CTRL_SELECTED : CTRL_DESELECTED));
				break;
			case	3:
				TabMode		=	THING_MODE_SELECT_SWITCH;
				DataPtr		=	(SLONG*)&map_things[CurrentThing].EditorData;
				break;
		}
	}
}

//---------------------------------------------------------------

EditComList	*ThingTab::SelectCommandList(void)
{
	BOOL			exit		=	FALSE;
	UBYTE			update		=	2;
	UWORD			select_pos;
	SLONG			c0;
	ControlSet		select_set;
	EditComList		*current_list,
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
	select_set.InitControlSet(select_command_def);
	if(ed_comlist_count>MAX_VIEW_LISTS)
	{
		((CVSlider*)select_set.GetControlPtr(1))->SetValueRange(0,ed_comlist_count-MAX_VIEW_LISTS);
	}
	((CVSlider*)select_set.GetControlPtr(1))->SetCurrentValue(0);

	while(SHELL_ACTIVE && !exit)
	{
		SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
		current_point.X	=	MouseX;
		current_point.Y	=	MouseY;

		if(select_set.HandleControlSet(&current_point))
			update	=	1;

		lists_rect.SetRect(LISTS_X,LISTS_Y,LISTS_WIDTH,LISTS_HEIGHT);
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

					lists_rect.SetRect(LISTS_X,LISTS_Y,LISTS_WIDTH,LISTS_HEIGHT);
					lists_rect.FillRect(ACTIVE_COL);

					// Skip the beginning of the list to match the slider bar position.
					current_list	=	comlists;
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

						QuickTextC(item_rect.GetLeft(),item_rect.GetTop(),current_list->ComListName,0);

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

void	ThingTab::DrawClassSet(void)
{
	EdRect		bounds_rect;


	bounds_rect.SetRect(ContentLeft()+3,ContentTop()+40,ContentWidth()-6,100);
	CurrentSet.ControlSetBounds(&bounds_rect);
	CurrentSet.SetControlDrawArea();
	CurrentSet.FillControlDrawArea(CONTENT_COL);
	CurrentSet.DrawControlSet();
	CurrentSet.HiliteControlDrawArea(LOLITE_COL,LOLITE_COL);

	SetTabDrawArea();
}

//---------------------------------------------------------------

void	ThingTab::UpdateTabInfo(void)
{
	MapThing		*t_mthing;


	((CStaticText*)GetControlPtr(CTRL_CLASS_TEXT))->SetString1(class_text[CurrentClass]);

	CurrentSet.InitControlSet(class_defs[CurrentClass]);

	if(CurrentClass==CLASS_BUILDING && CurrentThing)
	{
		t_mthing	=	TO_MTHING(CurrentThing);
		CurrentSet.SetControlState(2,(t_mthing->EditorFlags&0x01 ? CTRL_SELECTED : CTRL_DESELECTED));
	}

	UpdateTab(UPDATE_ALL);
}

//---------------------------------------------------------------

void	ThingTab::UpdateClassInfo(void)
{
	CBYTE			text[64];
	MapThing		*t_mthing;


	if(CurrentClass==CLASS_BUILDING && CurrentThing)
	{
		t_mthing	=	TO_MTHING(CurrentThing);

		//	Show the building ID.
		sprintf(text,"%ld",t_mthing->BuildingList);
		((CStaticText*)CurrentSet.GetControlPtr(1))->SetString1(text);

		//	Set the 'Locked' check box state
		CurrentSet.SetControlState(2,(t_mthing->EditorFlags&0x01 ? CTRL_SELECTED : CTRL_DESELECTED));

		//	Show the current unlock switch.		
		if(t_mthing->EditorData)
			sprintf(text,"%ld",t_mthing->EditorData);
		else
			sprintf(text,"None");
		((CStaticText*)CurrentSet.GetControlPtr(4))->SetString1(text);
	}
	else if(CurrentClass==CLASS_SWITCH)
	{
		class_defs[CLASS_SWITCH]	=	switch_defs[CurrentGenus];
		UpdateTabInfo();
		((CStaticText*)CurrentSet.GetControlPtr(CTRL_GENUS_TEXT))->SetString1(genus_text[CurrentClass][CurrentGenus]);
	}
	else if(CurrentThing)
		((CStaticText*)CurrentSet.GetControlPtr(CTRL_GENUS_TEXT))->SetString1(genus_text[CurrentClass][CurrentGenus]);


	switch(CurrentClass)
	{
		case	CLASS_PLAYER:
			break;
		case	CLASS_CAMERA:
			break;
		case	CLASS_BUILDING:

			break;
		case	CLASS_PERSON:
			if(map_things[CurrentThing].Class==CLASS_PERSON)
			{
				if(CurrentThing)
				{
					CurrentSet.SetControlState(CTRL_ITEM_3,CTRL_ACTIVE);
					sprintf(text,"%s",edit_comlists[map_things[CurrentThing].Data[0]].ComListName);
				}
				else
				{
					CurrentSet.SetControlState(CTRL_ITEM_3,CTRL_INACTIVE);
					sprintf(text,"None");
				}
				((CStaticText*)CurrentSet.GetControlPtr(CTRL_ITEM_4))->SetString1(text);
			}
			break;
		case	CLASS_SWITCH:
			switch(CurrentGenus)
			{
				case	SWITCH_NONE:
					break;
				case	SWITCH_PLAYER:
					break;
				case	SWITCH_THING:
					if(map_things[CurrentThing].Genus==SWITCH_THING)
					{
						if(CurrentThing)
						{
							CurrentSet.SetControlState(CTRL_ITEM_3,CTRL_ACTIVE);
							sprintf(text,"%ld",map_things[CurrentThing].Data[1]);
						}
						else
						{
							CurrentSet.SetControlState(CTRL_ITEM_3,CTRL_INACTIVE);
							sprintf(text,"0");
						}
						((CStaticText*)CurrentSet.GetControlPtr(CTRL_ITEM_4))->SetString1(text);
					}
					break;
				case	SWITCH_GROUP:
					break;
				case	SWITCH_CLASS:
					break;
			}
			break;
		case	CLASS_VEHICLE:

			if(map_things[CurrentThing].Class==CLASS_VEHICLE)
			{
				if(CurrentThing)
				{
					CurrentSet.SetControlState(CTRL_ITEM_3,CTRL_ACTIVE);
					sprintf(text,"%s",edit_comlists[map_things[CurrentThing].Data[0]].ComListName);
				}
				else
				{
					CurrentSet.SetControlState(CTRL_ITEM_3,CTRL_INACTIVE);
					sprintf(text,"None");
				}
				((CStaticText*)CurrentSet.GetControlPtr(CTRL_ITEM_4))->SetString1(text);
			}

			break;
		case	CLASS_SPECIAL:
			break;
	}

	UpdateTab(UPDATE_CLASS_SET);
}

//---------------------------------------------------------------

void	ThingTab::UpdateCheckBoxes(void)
{
	SLONG		c0;


	for(c0=0;c0<11;c0++)
	{
		SetControlState(CTRL_CLASS_CHECKS+c0,(ThingFlags&(1<<c0) ? CTRL_SELECTED : CTRL_DESELECTED));
	}
}

//---------------------------------------------------------------

