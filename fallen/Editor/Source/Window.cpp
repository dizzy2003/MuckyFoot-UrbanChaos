// Window.cpp
// Guy Simmons, 18th January 1997.

#include	"Editor.hpp"

#ifdef	EDITOR

//---------------------------------------------------------------

void Window::SetupWindow(CBYTE *title,ULONG flags,SLONG x,SLONG y,SLONG width,SLONG height)
{
	Flags			=	flags;
	StateFlags		=	0;

	TabList			=	NULL;
	CurrentTab		=	NULL;

	if(Flags&HAS_TITLE)
	{
		Title	=	title;
	}
	ConstrainHeight((SLONG*)&height);
	SetRect(x,y,width,height);
	ControlAreaHeight	=	0;
	ControlAreaWidth	=	0;
	SetAreaSizes();
}

//---------------------------------------------------------------

void Window::SetContentDrawArea(void)
{
	SetWorkWindowBounds	(
							ContentRect.GetLeft()+1,
							ContentRect.GetTop()+1,
							ContentRect.GetWidth()-2,
							ContentRect.GetHeight()-2
						);
}

//---------------------------------------------------------------

void Window::FillContent(ULONG the_colour)
{
	EdRect	fill_rect;

	fill_rect.SetRect(0,0,ContentWidth()-2,ContentHeight()-2);
	fill_rect.FillRect(the_colour);
}

//---------------------------------------------------------------

void Window::ClearContent(void)
{
	EdRect	clear_rect;

	clear_rect.SetRect(0,0,ContentWidth()-1,ContentHeight()-1);
	clear_rect.FillRect(ContentColour);
}

//---------------------------------------------------------------

void Window::DrawWindowFrame(void)
{
	SLONG		title_left,
				title_right,
				title_top,
				title_height,
				title_width;


	SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
	FillRect(CONTENT_COL);
	HiliteRect(HILITE_COL,LOLITE_COL);

	if(Flags&HAS_TITLE)
	{
		if(StateFlags&ACTIVE)
			TitleRect.FillRect(ACTIVE_COL);
		TitleRect.HiliteRect(LOLITE_COL,HILITE_COL);

		title_height	=	QTStringHeight();
		title_width		=	QTStringWidth(Title);
		title_left		=	TitleRect.GetLeft()+4;
		title_right		=	TitleRect.GetWidth()-64;
		title_top		=	TitleRect.GetTop()+((TitleRect.GetHeight()-title_height)>>1);

		if(title_width > title_right)
		{
			QuickText(title_left+title_right,title_top,"...",TEXT_COL);
		}
		QuickText(title_left,title_top,Title,TEXT_COL);
	}
	if(Flags&HAS_ICONS)
	{
		IconRect.HiliteRect(LOLITE_COL,HILITE_COL);
		TopIcons.DrawIcons();
	}
	if(Flags&HAS_HSCROLL)
	{
		HScrollRect.HiliteRect(LOLITE_COL,HILITE_COL);
	}
	if(Flags&HAS_VSCROLL)
	{
		VScrollRect.HiliteRect(LOLITE_COL,HILITE_COL);
	}
	ContentRect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

void Window::DrawWindowContent(void)
{
	SetContentDrawArea();
	if(Flags&HAS_CONTROLS)
		DrawControls();
	DrawContent();
	DrawGrowBox();
}

//---------------------------------------------------------------

void Window::DrawWindow(void)
{
//	if(Update && LockWorkScreen())
	if(LockWorkScreen())
	{
		DrawWindowFrame();
		DrawWindowContent();
		UnlockWorkScreen();
		Update	=	0;
	}
//	SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);
//	ShowWorkWindow(0);
}

//---------------------------------------------------------------

void Window::DrawControls(void)
{
	ModeTab		*current_tab;


	if(TabList)
	{
		current_tab	=	TabList;
		while(current_tab)
		{
			current_tab->DrawTab();
			current_tab	=	current_tab->GetNextTabLink();
		}
	}
	else if(WindowControls.GetControlCount())
	{
		WindowControls.DrawControlSet();
	}
	else
	{
		QuickText(2,2,"This module has no Tabs or Controls",TEXT_COL);
	}
}

//---------------------------------------------------------------

void Window::DrawContent(void)
{
	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);

	QuickText(2,2,"This module has no DrawContent function",TEXT_COL);
}

//---------------------------------------------------------------

void Window::MoveWindow(SLONG x,SLONG y)
{
	MoveRect(x,y);
	SetAreaSizes();
}

//---------------------------------------------------------------

void Window::SizeWindow(SLONG dx,SLONG dy)
{
	SLONG		height;

	height	=	GetHeight()+dy;
	ConstrainHeight(&height);
	SetRect(GetLeft(),GetTop(),GetWidth()+dx,height);
	SetAreaSizes();
}

//---------------------------------------------------------------

ULONG Window::WhereInWindow(MFPoint *the_point)
{
	ModeTab		*clicked_tab	=	0,
				*current_tab;


	if(PointInRect(the_point))
	{
		if(Flags&HAS_TITLE)
		{
			if(TitleRect.PointInRect(the_point))
				return	IN_TITLE;
		}
		else
		{
			if(TitleRect.PointInRect(the_point))
				return	IN_TITLE;
		}

		if(Flags&HAS_ICONS)
		{
			if(IconRect.PointInRect(the_point))
				return	IN_ICONS;
		}
		if(Flags&HAS_GROW)
		{
			if(GrowRect.PointInRect(the_point))
				return	IN_GROW;
		}
		if(Flags&HAS_HSCROLL)
		{
			if(HScrollRect.PointInRect(the_point))
				return	IN_HSCROLL;
		}
		if(Flags&HAS_VSCROLL)
		{
			if(VScrollRect.PointInRect(the_point))
				return	IN_VSCROLL;
		}
		if(Flags&HAS_CONTROLS)
		{
			if(ControlRect.PointInRect(the_point))
			{
				current_tab	=	TabList;
				while(current_tab)
				{
					if(current_tab->PointInTitle(the_point))
					{
						clicked_tab	=	current_tab;
						if(clicked_tab)
						{
							BringTabToFront(clicked_tab);
							if(LockWorkScreen())
							{
								DrawWindowContent();
								UnlockWorkScreen();
							}
							ShowWorkScreen(0);
						}
						return	OUTSIDE_WINDOW;
					}
					current_tab	=	current_tab->GetNextTabLink();
				}
				return	IN_CONTROLS;
			}
		}
		if(ContentRect.PointInRect(the_point))
			return	IN_CONTENT;
		return	IN_WINDOW;
	}
	return	OUTSIDE_WINDOW;
}

//---------------------------------------------------------------

void Window::ConstrainHeight(SLONG *new_height)
{
	if(*new_height<8)
		*new_height	=	8;
	if(Flags&HAS_TITLE)
	{
		if(*new_height < 28)
		{
			*new_height	=	28;
		}
		if(Flags&(HAS_GROW|HAS_HSCROLL))
		{
			if(*new_height < 40)
				*new_height	=	40;
		}
	}
	else if(Flags&(HAS_GROW|HAS_HSCROLL))
	{
		if(*new_height < 20)
			*new_height	=	20;
	}
}

//---------------------------------------------------------------

void Window::SetAreaSizes(void)
{
	ModeTab		*current_tab;
	SLONG	content_offset=3;


	if(Flags&HAS_TITLE)
	{
		TitleRect.SetRect(GetLeft()+3,GetTop()+3,GetWidth()-6,18);
	}
	else
	{
		TitleRect.SetRect(GetLeft(),GetTop(),GetWidth()-6,5);
		content_offset=0;
	}

	if(Flags&HAS_ICONS)
	{
		IconRect.SetRect(GetLeft()+3,TitleRect.GetBottom()+3,GetWidth()-6,25);
		TopIcons.SetRect(GetLeft()+3,TitleRect.GetBottom()+3,GetWidth()-6,25);
	}
	else
	{
		IconRect.SetRect(GetLeft(),TitleRect.GetBottom()+3,0,0);
	}

	if(Flags&HAS_CONTROLS)
	{
		ControlRect.SetRect(GetLeft()+3,IconRect.GetBottom()+3,ControlAreaWidth,ControlAreaHeight);
		current_tab	=	TabList;
		while(current_tab)
		{
			current_tab->MoveTabArea(&ControlRect);
			current_tab	=	current_tab->GetNextTabLink();
		}
		if(ControlRect.GetBottom() > GetBottom())
		{
			SetRect(GetLeft(),GetTop(),GetWidth(),GetHeight()+(ControlRect.GetBottom()-GetBottom())+3);
		}
	}
	else
	{
		ControlRect.SetRect(GetLeft(),0,0,0);
	}
	WindowControls.ControlSetBounds(&ControlRect);

	if(Flags&HAS_GROW)
	{
		GrowRect.SetRect(GetRight()-14,GetBottom()-14,14,14);
	}
	else
	{
		GrowRect.SetRect(0,0,0,0);
	}
	if(Flags&HAS_HSCROLL)
	{
		if(Flags&(HAS_VSCROLL|HAS_GROW))
		{
			HScrollRect.SetRect(GetLeft()+3,GetBottom()-12,GetWidth()-18,10);
		}
		else
		{
			HScrollRect.SetRect(GetLeft()+3,GetBottom()-12,GetWidth()-6,10);
		}
	}
	else
	{
		HScrollRect.SetRect(GetLeft(),GetBottom(),0,0);
	}
	if(Flags&HAS_VSCROLL)
	{
		if(Flags&(HAS_HSCROLL|HAS_GROW))
		{
			VScrollRect.SetRect(GetRight()-12,IconRect.GetBottom()+3,10,(GetBottom()-14)-(IconRect.GetBottom()+3));
		}
		else
		{
			VScrollRect.SetRect(GetRight()-12,IconRect.GetBottom()+3,10,(GetBottom()-2)-(IconRect.GetBottom()+3));
		}
	}
	else
	{
		VScrollRect.SetRect(GetRight(),IconRect.GetBottom()+3,0,0);
	}

	ContentRect.SetRect	(
							ControlRect.GetRight()+3,
							IconRect.GetBottom()+content_offset,
							(VScrollRect.GetLeft()-2)-(ControlRect.GetRight()+3),
							(HScrollRect.GetTop()-2)-(IconRect.GetBottom()+content_offset)
						);
	Update				=	1;
}

//---------------------------------------------------------------

void Window::DrawGrowBox(void)
{
	if(Flags&HAS_GROW)
	{
		SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);
		if(!(Flags&(HAS_HSCROLL|HAS_VSCROLL)))
		{
			GrowRect.FillRect(CONTENT_COL);
			DrawHLineC	(
							GrowRect.GetLeft()-1,
							ContentRect.GetRight(),
							GrowRect.GetTop()-1,
							HILITE_COL
						);
			DrawVLineC	(
							GrowRect.GetLeft()-1,
							GrowRect.GetTop()-1,
							ContentRect.GetBottom(),
							HILITE_COL
						);
		}
		DrawMonoBSprite(GrowRect.GetLeft(),GrowRect.GetTop(),INTERFACE_SPRITE(GROW_ICON),0);
		SetContentDrawArea();
	}
}

//---------------------------------------------------------------

void Window::BringTabToFront(ModeTab *the_tab)
{
	if(the_tab == CurrentTab)
		return;							// Already at the end of list (Front of display).


	if(the_tab->GetLastTabLink())
	{									// Not at start of list.
		the_tab->GetLastTabLink()->SetNextTabLink(the_tab->GetNextTabLink());
		the_tab->GetNextTabLink()->SetLastTabLink(the_tab->GetLastTabLink());
	}
	else
	{									// At start of list.
		TabList		=	the_tab->GetNextTabLink();
		TabList->SetLastTabLink(NULL);
	}
	the_tab->SetLastTabLink(NULL);
	the_tab->SetNextTabLink(NULL);
	AddTab(the_tab);
}

//---------------------------------------------------------------

void Window::BringTabIDToFront(UWORD id)
{
	ModeTab		*current_tab;

	
	current_tab	=	TabList;
	while(current_tab)
	{
		if(current_tab->GetTabID()==id)
		{
			BringTabToFront(current_tab);
			return;
		}
		current_tab	=	current_tab->GetNextTabLink();
	}
}

//---------------------------------------------------------------

void	Window::ActivateNextTab(void)
{
	ModeTab		*clicked_tab;


	if(TabList)
	{
		if(CurrentTab->GetNextTabLink())
		{
			clicked_tab	=	CurrentTab->GetNextTabLink();
		}
		else
		{
			clicked_tab	=	TabList;
		}
		BringTabToFront(clicked_tab);
		if(LockWorkScreen())
		{
			DrawWindowContent();
			UnlockWorkScreen();
		}
		ShowWorkScreen(0);
	}
}

//---------------------------------------------------------------

void	Window::ActivateLastTab(void)
{
	ModeTab		*clicked_tab,
				*current_tab;


	if(TabList)
	{
		if(CurrentTab->GetLastTabLink())
		{
			clicked_tab	=	CurrentTab->GetLastTabLink();
		}
		else
		{
			current_tab	=	TabList;
			while(current_tab->GetNextTabLink())
				current_tab	=	current_tab->GetNextTabLink();
			clicked_tab	=	current_tab;
		}
		BringTabToFront(clicked_tab);
		if(LockWorkScreen())
		{
			DrawWindowContent();
			UnlockWorkScreen();
		}
		ShowWorkScreen(0);
	}
}

//---------------------------------------------------------------

void Window::AddTab(ModeTab *the_tab)
{
	if(CurrentTab == NULL)			// Start of list?
	{
		TabList		=	the_tab;
		CurrentTab	=	TabList;
		CurrentTab->SetStateFlags(TAB_ACTIVE);
	}
	else
	{
		CurrentTab->SetStateFlags(0);
		CurrentTab->SetNextTabLink(the_tab);
		the_tab->SetLastTabLink(CurrentTab);
		CurrentTab	=	the_tab;
		CurrentTab->SetStateFlags(TAB_ACTIVE);
	}
}

//---------------------------------------------------------------

void Window::HandleTab(MFPoint *current_point)
{
	CurrentTab->HandleTab(current_point);
}

//---------------------------------------------------------------

#endif
