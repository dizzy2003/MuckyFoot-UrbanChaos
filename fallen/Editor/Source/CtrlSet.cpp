// CtrlSet.cpp
// Guy Simmons, 25th November 1996.

#include	"Editor.hpp"


//---------------------------------------------------------------
// Private member functions
//---------------------------------------------------------------

void ControlSet::AddControl(Control *the_control)
{
	if(the_control)
	{
		if(CurrentControl == NULL)			// Start of list?
		{
			CurrentControl	=	the_control;
			ControlList		=	CurrentControl;
		}
		else
		{
			CurrentControl->SetNextControl(the_control);
			the_control->SetLastControl(CurrentControl);
			CurrentControl	=	the_control;
		}
		ControlCount++;
		CurrentControl->SetID(ControlCount);
	}
}

//---------------------------------------------------------------
// Public member functions
//---------------------------------------------------------------

ControlSet::ControlSet(ControlDef *defs)
{
	ControlList		=	NULL;
	CurrentControl	=	NULL;
	ControlCount	=	0;

	InitControlSet(defs);
}

//---------------------------------------------------------------

ControlSet::ControlSet()
{
	ControlList		=	NULL;
	CurrentControl	=	NULL;
	ControlCount	=	0;
}

//---------------------------------------------------------------

ControlSet::~ControlSet()
{
	FiniControlSet();
}

//---------------------------------------------------------------

void	ControlSet::InitControlSet(ControlDef *defs)
{
	FiniControlSet();
	while(defs->ControlType)
	{
		switch(defs->ControlType)
		{
			case	BUTTON:
				AddControl(new CButton(defs));
				break;
			case	RADIO_BUTTON:
				AddControl(new CRadioButton(defs));
				break;
			case	CHECK_BOX:
				AddControl(new CCheckBox(defs));
				break;
			case	STATIC_TEXT:
				AddControl(new CStaticText(defs));
				break;
			case	EDIT_TEXT:
				AddControl(new CEditText(defs));
				break;
			case	PULLDOWN_MENU:
				AddControl(new CPullDown(defs));
				break;
			case	POPUP_MENU:
				break;
			case	H_SLIDER:
				AddControl(new CHSlider(defs));
				break;
			case	V_SLIDER:
				AddControl(new CVSlider(defs));
				break;
		}
		defs++;
	}
}

//---------------------------------------------------------------

void	ControlSet::FiniControlSet(void)
{
	Control		*current_control,
				*next_control;

	
	current_control	=	ControlList;
	while(current_control)
	{
		next_control	=	current_control->GetNextControl();
		switch(current_control->GetType())
		{
			case	BUTTON:
				delete (CButton*)current_control;
				break;
			case	RADIO_BUTTON:
				delete (CRadioButton*)current_control;
				break;
			case	CHECK_BOX:
				delete (CCheckBox*)current_control;
				break;
			case	STATIC_TEXT:
				delete (CStaticText*)current_control;
				break;
			case	EDIT_TEXT:
				delete (CEditText*)current_control;
				break;
			case	PULLDOWN_MENU:
				delete (CPullDown*)current_control;
				break;
			case	POPUP_MENU:
				break;
			case	H_SLIDER:
				delete	(CHSlider*)current_control;
				break;
			case	V_SLIDER:
				delete	(CVSlider*)current_control;
				break;
		}
		current_control	=	next_control;
	}
	ControlList		=	NULL;
	CurrentControl	=	NULL;
	ControlCount	=	0;
}

//---------------------------------------------------------------

void	ControlSet::SetControlDrawArea(void)
{
	SetWorkWindowBounds	(
							SetRect.GetLeft()+1,
							SetRect.GetTop()+1,
							SetRect.GetWidth()-2,
							SetRect.GetHeight()-2
						);
}

//---------------------------------------------------------------

void	ControlSet::FillControlDrawArea(ULONG colour)
{
	DrawBoxC(0,0,WorkWindowWidth-1,WorkWindowHeight-1,colour);
}

//---------------------------------------------------------------

void	ControlSet::HiliteControlDrawArea(ULONG hilite,ULONG lolite)
{
	DrawVLineC(WorkWindowWidth-1,0,WorkWindowHeight-1,lolite);
	DrawHLineC(0,WorkWindowWidth-1,WorkWindowHeight-1,lolite);
	DrawVLineC(0,0,WorkWindowHeight-1,hilite);
	DrawHLineC(0,WorkWindowWidth-1,0,hilite);
}

//---------------------------------------------------------------

void	ControlSet::DrawControlSet(void)
{
	Control		*current_control;
	MFRect		ww_rect;


	// Backup current work window rect.
	ww_rect	=	WorkWindowRect;

	// Do the draw.
	SetControlDrawArea();
	current_control	=	ControlList;
	while(current_control)
	{
		current_control->DrawControl();
		current_control	=	current_control->GetNextControl();
	}
/*
	// Restore old work window rect.
	SetWorkWindowBounds	(
							ww_rect.Left,
							ww_rect.Top,
							ww_rect.Width,
							ww_rect.Height
						);
*/
}

//---------------------------------------------------------------


UBYTE ControlSet::HandleControlSet(MFPoint *current_point)
{
	UBYTE		in_text		=	0,
				result		=	0,
				update		=	0;
	Control		*current_control;
	MFPoint		local_point;


	if(PointInControlSet(current_point))
	{
		local_point	=	*current_point;
		GlobalToLocal(&local_point);
		current_control	=	ControlList;
		while(current_control)
		{
			if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
			{
				if(!(current_control->GetFlags()&CONTROL_HILITED))
				{
					current_control->HiliteControl(&local_point);
					SetStateFlags((UBYTE)(GetStateFlags()|CS_CLEANUP));
					update	=	1;
					result	=	1;
					break;
				}
				if(current_control->GetType()==EDIT_TEXT)
					in_text	=	1;
			}
			else
			{
				if(current_control->GetFlags()&CONTROL_HILITED)
				{
					current_control->UnHiliteControl();
					SetStateFlags((UBYTE)(GetStateFlags()|CS_CLEANUP));
					update	=	1;
				}
			}
			current_control	=	current_control->GetNextControl();
		}

		if(in_text)
		{
//			LbMouseSetPointerHotspot(2,3);
//			ChangeMouseSprite(INTERFACE_POINTER(2));
		}
		else
		{
//			LbMouseSetPointerHotspot(0,0);
//			ChangeMouseSprite(INTERFACE_POINTER(1));
		}
	}
	else if(GetStateFlags()&CS_CLEANUP)
	{
		current_control	=	ControlList;
		while(current_control)
		{
			if(current_control->GetFlags()&CONTROL_HILITED)
			{
				current_control->UnHiliteControl();
				update	=	1;
			}
			current_control	=	current_control->GetNextControl();
		}
//		ChangeMouseSprite(INTERFACE_POINTER(1));
//		LbMouseSetPointerHotspot(0,0);

		SetStateFlags((UBYTE)(GetStateFlags()&~(CS_CLEANUP)));
	}

	if(update)
	{
		if(LockWorkScreen())
		{
			DrawControlSet();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
	}

	return	result;
}

//---------------------------------------------------------------

UWORD ControlSet::HandleControlSetClick(UBYTE flags,MFPoint *clicked_point)
{
	UWORD		control_id;
	Control		*current_control;


	if(PointInControlSet(clicked_point))
	{
		switch(flags)
		{
			case	NO_CLICK:
				break;
			case	LEFT_CLICK:
				GlobalToLocal(clicked_point);
				current_control	=	ControlList;
				while(current_control)
				{
					if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(clicked_point))
					{
						SetControlDrawArea();
						control_id	=	current_control->TrackControl(clicked_point);
						return	control_id;
					}
					current_control	=	current_control->GetNextControl();
				}
				break;
			case	RIGHT_CLICK:
				break;
		}
	}
	return	0;
}

//---------------------------------------------------------------

UWORD ControlSet::HandleControlSetKey(UBYTE the_key)
{
	Control		*current_control;


	current_control	=	ControlList;
	while(current_control)
	{
		if(!(current_control->GetFlags()&CONTROL_INACTIVE))
		{
			if(current_control->GetType()==PULLDOWN_MENU || current_control->GetType()==POPUP_MENU)
			{
				// Loop through menu items.

			}
			else
			{
				if(the_key==current_control->GetHotKey())
				{
					SetControlDrawArea();
					current_control->TrackKey();
					return	current_control->GetID();
				}
			}
		}
		current_control	=	current_control->GetNextControl();
	}
	return	0;
}

//---------------------------------------------------------------

Control	*ControlSet::GetControlPtr(UWORD id)
{
	UWORD		c0;
	Control		*current_control;

	
	current_control	=	ControlList;
	for(c0=1;c0<id;c0++,current_control=current_control->GetNextControl());
	return	current_control;
}

//---------------------------------------------------------------

void ControlSet::SetControlState(UWORD id,UBYTE state)
{
	UWORD		c0;
	Control		*current_control;

	
	current_control	=	GetControlList();
	for(c0=1;c0<id;c0++,current_control=current_control->GetNextControl());

	switch(state)
	{
		case	CTRL_SELECTED:
			current_control->SetFlags((UBYTE)(current_control->GetFlags()|CONTROL_SELECTED));
			break;
		case	CTRL_DESELECTED:
			current_control->SetFlags((UBYTE)(current_control->GetFlags()&~(CONTROL_SELECTED)));
			break;
		case	CTRL_ACTIVE:
			current_control->SetFlags((UBYTE)(current_control->GetFlags()&~(CONTROL_INACTIVE)));
			break;
		case	CTRL_INACTIVE:
			current_control->SetFlags((UBYTE)(current_control->GetFlags()|CONTROL_INACTIVE));
			break;
	}
}

//---------------------------------------------------------------

UBYTE ControlSet::GetControlState(UWORD id)
{
	UWORD		c0;
	Control		*current_control;

	
	current_control	=	GetControlList();
	for(c0=1;c0<id;c0++,current_control=current_control->GetNextControl());

	if(current_control->GetFlags()&CONTROL_SELECTED)
		return	CTRL_SELECTED;
	else
		return	CTRL_DESELECTED;
}

//---------------------------------------------------------------

void ControlSet::ToggleControlSelectedState(UWORD id)
{
	UWORD		c0;
	Control		*current_control;

	
	current_control	=	GetControlList();
	for(c0=1;c0<id;c0++,current_control=current_control->GetNextControl());

	current_control->SetFlags((UBYTE)(current_control->GetFlags()^CONTROL_SELECTED));
}

//---------------------------------------------------------------

void ControlSet::ToggleControlActiveState(UWORD id)
{
	UWORD		c0;
	Control		*current_control;

	
	current_control	=	GetControlList();
	for(c0=1;c0<id;c0++,current_control=current_control->GetNextControl());

	current_control->SetFlags((UBYTE)(current_control->GetFlags()^CONTROL_INACTIVE));
}

//---------------------------------------------------------------

void ControlSet::SetMenuItemState(UWORD id,UWORD item,UBYTE state)
{
	UWORD		c0;
	Control		*current_control;
	CPullDown	*the_menu;

	
	current_control	=	GetControlList();
	for(c0=1;c0<id;c0++,current_control=current_control->GetNextControl());

	the_menu	=	(CPullDown*)current_control;

	switch(state)
	{
		case	CTRL_ACTIVE:
			the_menu->SetItemFlags(item,(UBYTE)(the_menu->GetItemFlags(item)&~(MENU_INACTIVE)));
			break;
		case	CTRL_INACTIVE:
			the_menu->SetItemFlags(item,(UBYTE)(the_menu->GetItemFlags(item)|MENU_INACTIVE));
			break;
	}
}

//---------------------------------------------------------------

void ControlSet::SetPopUpItemState(CPopUp *the_popup,UWORD item,UBYTE state)
{
	switch(state)
	{
		case	CTRL_ACTIVE:
			the_popup->SetItemFlags(item,(UBYTE)(the_popup->GetItemFlags(item)&~(MENU_INACTIVE)));
			break;
		case	CTRL_INACTIVE:
			the_popup->SetItemFlags(item,(UBYTE)(the_popup->GetItemFlags(item)|MENU_INACTIVE));
			break;
	}
}

//---------------------------------------------------------------
