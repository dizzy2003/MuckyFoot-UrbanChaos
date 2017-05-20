// ModeTab.cpp
// Guy Simmons, 18th February 1997.

#include	"Editor.hpp"


//---------------------------------------------------------------

ModeTab::ModeTab()
{
	SetLastTabLink(NULL);
	SetNextTabLink(NULL);
	SetExternalUpdatePtr(0);
}

//---------------------------------------------------------------

void ModeTab::SetupModeTab(CBYTE *the_title,UWORD id,EdRect *bounding_rect,ULONG *update_ptr)
{
	Title		=	the_title;
	SetTabArea(bounding_rect);
	SetTabID(id);
	SetExternalUpdatePtr(update_ptr);
}

//---------------------------------------------------------------

void ModeTab::SetTabArea(EdRect *bounding_rect)
{
	if(LastModeTab)
	{
		if(Title)
		{
			TitleRect.SetRect	(
									LastModeTab->TitleRight()+1,
									bounding_rect->GetTop(),
									QTStringWidth(Title)+6,
									QTStringHeight()+6
								);
		}
	}
	else
	{
		if(Title)
		{
			TitleRect.SetRect	(
									bounding_rect->GetLeft(),
									bounding_rect->GetTop(),
									QTStringWidth(Title)+6,
									QTStringHeight()+6
								);
		}
	}
	ContentRect.SetRect	(
							bounding_rect->GetLeft(),
							TitleRect.GetBottom(),
							bounding_rect->GetWidth(),
							bounding_rect->GetHeight()-TitleRect.GetHeight()
						);
	ControlSetBounds(&ContentRect);
}

//---------------------------------------------------------------

void ModeTab::MoveTabArea(EdRect *bounding_rect)
{
	SLONG		offset_x;


	offset_x	=	bounding_rect->GetLeft()-ContentRect.GetLeft();
	TitleRect.MoveRect(TitleRect.GetLeft()+offset_x,bounding_rect->GetTop());
	ContentRect.MoveRect(bounding_rect->GetLeft(),TitleRect.GetBottom());
	ControlSetBounds(&ContentRect);
}

//---------------------------------------------------------------

void	ModeTab::SetTabDrawArea(void)
{
	SetWorkWindowBounds	(
							ContentLeft()+1,
							ContentTop()+1,
							ContentWidth()-2,
							ContentHeight()-2
						);
}

//---------------------------------------------------------------

void	ModeTab::ClearTab(void)
{
	EdRect		clear_rect;

	clear_rect.SetRect(0,0,ContentWidth()-1,ContentHeight()-1);
	clear_rect.FillRect(CONTENT_COL);
}

//---------------------------------------------------------------

void ModeTab::DrawTab(void)
{
	EdRect		title_rect;


	SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);

	TitleRect.FillRect(CONTENT_COL);
	if(GetStateFlags())
	{
		ContentRect.HiliteRect(HILITE_COL,LOLITE_COL);
		TitleRect.HiliteRect(HILITE_COL,LOLITE_COL);
		DrawHLineC	(
						TitleRect.GetLeft()+1,
						TitleRect.GetRight()-1,
						TitleRect.GetBottom(),
						CONTENT_COL
					);
		QuickTextC	(
						TitleRect.GetLeft()+3,
						TitleRect.GetTop()+3,
						Title,
						0
					);

		DrawTabContent();
	}
	else
	{
		title_rect	=	TitleRect;
		title_rect.ShrinkRect(1,0);
		title_rect.OffsetRect(0,1);
		title_rect.HiliteRect(HILITE_COL,LOLITE_COL);
		QuickTextC	(
						title_rect.GetLeft()+2,
						title_rect.GetTop()+3,
						Title,
						0
					);
	}
}

//---------------------------------------------------------------

void ModeTab::DrawTabContent(void)
{
	EdRect		content_rect;


	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	QuickTextC	(
					ContentRect.GetLeft()+4,
					ContentRect.GetTop()+4,
					"No DrawTabContent function.",0
				);
}

//---------------------------------------------------------------

void ModeTab::HandleTab(MFPoint *current_point)
{
	HandleControlSet(current_point);
}

//---------------------------------------------------------------

UWORD ModeTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	clicked_point	=	clicked_point;
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			break;
		case	RIGHT_CLICK:
			break;
	}
	return	0;
}

//---------------------------------------------------------------
