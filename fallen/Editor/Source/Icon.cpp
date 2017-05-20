// Icon.cpp
// Mike Diskett, 11th April 1997.


#include	"Editor.hpp"

#ifdef	EDITOR

#define		ICON_WIDTH	20
#define		ICON_HEIGHT	20

//---------------------------------------------------------------

void	WinBarIcon::DrawIcons(void)
{
	EdRect	box_it;
	SLONG	c0=0;
	SLONG	x;

	if(WindowIcons)
	{
		x=GetRight();
		while(WindowIcons[c0].ImageOff)
		{
			if(WindowIcons[c0].ImageOff==-1)
			{
				x-=8;				
				box_it.SetRect(x,GetTop()-2,4,27);
				box_it.HiliteRect(HILITE_COL,LOLITE_COL);
				
			}
			else
			{
				x-=ICON_WIDTH+4;				
				box_it.SetRect(x,GetTop()+2,ICON_WIDTH,ICON_HEIGHT);
				box_it.HiliteRect(HILITE_COL,LOLITE_COL);
				if(WindowIcons[c0].Flag)
					DrawBSprite(box_it.GetLeft(),box_it.GetTop(),INTERFACE_SPRITE(WindowIcons[c0].ImageOn));
				else
					DrawBSprite(box_it.GetLeft(),box_it.GetTop(),INTERFACE_SPRITE(WindowIcons[c0].ImageOff));
//					DrawMonoBSprite(box_it.GetLeft(),box_it.GetTop(),INTERFACE_SPRITE(WindowIcons[c0].ImageOff),0);
			}
			c0++;			
		}
	}
}
void	WinBarIcon::HandleIconClick(UBYTE flags,MFPoint *clicked_point)
{
	EdRect	box_it;
	SLONG	c0=0;
	SLONG	x;

	if(WindowIcons)
	{
		x=GetRight();
		while(WindowIcons[c0].ImageOff)
		{
			if(WindowIcons[c0].ImageOff==-1)
			{
				x-=8;				
			}
			else
			{
				x-=ICON_WIDTH+4;
				box_it.SetRect(x,GetTop()+2,ICON_WIDTH,ICON_HEIGHT);
				if(box_it.PointInRect(clicked_point))
				{
					//we have a click on an icon
					WindowIcons[c0].Flag^=1;
					if(WindowIcons[c0].Function)
						WindowIcons[c0].Function(c0);

				}
			}
			c0++;
		}
	}
}

void	WinBarIcon::InitIcons(struct AWindowIcon *p_icons)
{
	WindowIcons=p_icons;	
}

#endif
