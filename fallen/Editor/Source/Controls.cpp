// Controls.cpp
// Guy Simmons, 12th November 1996

#include	"Editor.hpp"

#ifdef	EDITOR

//---------------------------------------------------------------

void Control::DrawControl(void)
{
}

//---------------------------------------------------------------

UWORD Control::TrackControl(MFPoint *down_point)
{
	UBYTE		start_flags,
				update;
	MFPoint		current_point,
				start_point;


	start_flags		=	Flags;
	start_point.X	=	MouseX;
	start_point.Y	=	MouseY;
	Flags			=	(UBYTE)(start_flags|CONTROL_CLICKED);
	update			=	1;
	while(SHELL_ACTIVE && LeftButton)
	{
		current_point.X	=	down_point->X+(MouseX-start_point.X);
		current_point.Y	=	down_point->Y+(MouseY-start_point.Y);

		if(PointInControl(&current_point))
		{
			if(!(Flags&CONTROL_CLICKED))
			{
				Flags	=	(UBYTE)(start_flags|CONTROL_CLICKED);
				update 	=	1;
			}
		}
		else
		{
			if(Flags&CONTROL_CLICKED)
			{
				Flags	=	(UBYTE)(start_flags&~(CONTROL_HILITED));
				update	=	1;
			}
		}

		if(update)
		{
			if(LockWorkScreen())
			{
				DrawControl();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);
		}
		update			=	0;
	}

	if(Flags&CONTROL_CLICKED)
	{
		Flags	&=	~(CONTROL_CLICKED);
		return	GetID();
	}
	else
		return	0;
}

//---------------------------------------------------------------

void	Control::TrackKey(void)
{
	Flags	|=	CONTROL_HILITED;
	if(LockWorkScreen())
	{
		DrawControl();
		UnlockWorkScreen();
		ShowWorkWindow(0);
	}	
	while(SHELL_ACTIVE && Keys[GetHotKey()]);
	Flags	&=	~CONTROL_HILITED;
}

//---------------------------------------------------------------

void	Control::HiliteControl(MFPoint *current_point)
{
	Flags	|=	CONTROL_HILITED;
	current_point	=	current_point;
}

//---------------------------------------------------------------

void	Control::UnHiliteControl(void)
{
	Flags	&=	~(CONTROL_HILITED);
}

//---------------------------------------------------------------
// Button.
//---------------------------------------------------------------

CButton::CButton(ControlDef *the_def)
{
	SLONG		width;


	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(BUTTON);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	width	=	the_def->ControlWidth;
	if(width < (QTStringWidth(GetTitle())+6))
		width	=	QTStringWidth(GetTitle())+6;

	SetRect(the_def->ControlLeft,the_def->ControlTop,width,QTStringHeight()+5);
}

//---------------------------------------------------------------

void CButton::DrawControl(void)
{
	ULONG		text_colour;
	SLONG		text_x,
				text_y;
	CBYTE	*ptr;

	text_x	=	GetLeft()+((GetWidth()>>1)-(QTStringWidth(GetTitle())>>1));
	text_y	=	GetTop()+((GetHeight()>>1)-(QTStringHeight()>>1));
	if(GetFlags()&CONTROL_INACTIVE)
	{
		text_colour	=	LOLITE_COL;
		HiliteRect(HILITE_COL,LOLITE_COL);
	}
	else
	{
		text_colour	=	0;
		if(GetFlags()&CONTROL_HILITED)
			FillRect(HILITE_COL);
		else
			FillRect(CONTENT_COL);

		if(GetFlags()&CONTROL_CLICKED)
		{
			HiliteRect(LOLITE_COL,HILITE_COL);
			text_x++;
			text_y++;
		}
		else
			HiliteRect(HILITE_COL,LOLITE_COL);
	}

	ptr=GetTitle();
	if(ptr[0]=='$')
	{
		SLONG	sprite;
		sprite=(ptr[1]-'0')*10+(ptr[2]-'0');
		DrawMonoBSprite(GetLeft()+7,GetTop()+3,INTERFACE_SPRITE(sprite),0);
	}
	else
		QuickTextC(text_x,text_y,GetTitle(),text_colour);
}

//---------------------------------------------------------------
// Radio button.
//---------------------------------------------------------------

CRadioButton::CRadioButton(ControlDef *the_def)
{
	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(RADIO_BUTTON);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,10,10);
}

//---------------------------------------------------------------

void CRadioButton::DrawControl(void)
{
	ULONG	text_colour;
	SLONG	text_x,
			text_y;


	text_x	=	GetRight()+4;
	text_y	=	GetTop()+((GetHeight()>>1)-(QTStringHeight()>>1));
	if(GetFlags()&CONTROL_INACTIVE)
	{
		text_colour	=	LOLITE_COL;
//		LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON4),CONTENT_COL);
	}
	else
	{
		text_colour	=	0;
/*
		if(GetFlags()&CONTROL_HILITED)
			LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON4),HILITE_COL);
		else
			LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON4),ACTIVE_COL);
*/
/*
		if(GetFlags()&CONTROL_CLICKED)
			LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON3),ACTIVE_COL);
		else if(GetFlags()&CONTROL_SELECTED)
			LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON3),0);
*/
	}
//	LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON1),LOLITE_COL);
//	LbSpriteDrawOneColour(GetLeft(),GetTop(),INTERFACE_SPRITE(RADIO_ICON2),HILITE_COL);
	QuickTextC(text_x,text_y,GetTitle(),text_colour);
}

//---------------------------------------------------------------
// Check box.
//---------------------------------------------------------------

CCheckBox::CCheckBox(ControlDef *the_def)
{
	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(CHECK_BOX);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,10,10);
}

//---------------------------------------------------------------

void CCheckBox::DrawControl(void)
{
	ULONG	text_colour;
	SLONG	text_x,
			text_y;


	text_x	=	GetRight()+4;
	text_y	=	GetTop()+((GetHeight()>>1)-(QTStringHeight()>>1));

	if(GetFlags()&CONTROL_INACTIVE)
	{
		text_colour	=	LOLITE_COL;
		FillRect(CONTENT_COL);
	}
	else
	{
		text_colour	=	0;
		if(GetFlags()&CONTROL_HILITED)
			FillRect(HILITE_COL);
		else
			FillRect(ACTIVE_COL);

		if(GetFlags()&CONTROL_CLICKED)
			DrawMonoBSpriteC(GetLeft(),GetTop(),INTERFACE_SPRITE(CHECK_ICON),ACTIVE_COL);
		else if(GetFlags()&CONTROL_SELECTED)
			DrawMonoBSpriteC(GetLeft(),GetTop(),INTERFACE_SPRITE(CHECK_ICON),0);

	}
	HiliteRect(LOLITE_COL,HILITE_COL);
	QuickTextC(text_x,text_y,GetTitle(),text_colour);
}

//---------------------------------------------------------------
// Static Text.
//---------------------------------------------------------------

CStaticText::CStaticText(ControlDef *the_def)
{
	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(STATIC_TEXT);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,QTStringWidth(GetTitle()),10);

	SetString1("");
	SetString2("");
}

//---------------------------------------------------------------

void CStaticText::DrawControl(void)
{
	CBYTE	text[256];


	sprintf(text,"%s%s%s",GetTitle(),String1,String2);
	FillRect(CONTENT_COL);
	QuickTextC(GetLeft(),GetTop()+((GetHeight()>>1)-(QTStringHeight()>>1)),text,0);
}

//---------------------------------------------------------------
// Edit Text.
//---------------------------------------------------------------

CEditText::CEditText(ControlDef *the_def)
{
	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(EDIT_TEXT);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,the_def->ControlWidth,QTStringHeight()+5);

	SetEditString("");
	CursorPos		=	0;
	TextX			=	GetLeft()+2;
}

//---------------------------------------------------------------

void CEditText::DrawControl(void)
{
	ULONG		text_colour;
	CBYTE		temp_string[EDIT_TEXT_LENGTH];
	SLONG		c0,
				cursor_x,
				text_x,
				text_y;
	MFTime		the_time;


	text_x	=	GetRight()+4;
	text_y	=	GetTop()+((GetHeight()>>1)-(QTStringHeight()>>1));

	if(GetFlags()&CONTROL_INACTIVE)
	{
		text_colour	=	LOLITE_COL;
		FillRect(CONTENT_COL);
	}
	else
	{
		text_colour	=	0;
		if(GetFlags()&CONTROL_HILITED)
			FillRect(HILITE_COL);
		else
			FillRect(ACTIVE_COL);
	}

	HiliteRect(LOLITE_COL,HILITE_COL);
	QuickTextC(text_x,text_y,GetTitle(),text_colour);

	if(GetFlags()&CONTROL_CLICKED)
	{
		strncpy(temp_string,EditText,CursorPos);
		temp_string[CursorPos]	=	0;
		cursor_x	=	TextX+QTStringWidth(temp_string);
		if(cursor_x>(GetRight()-2))
		{
			TextX		-=	cursor_x-(GetRight()-1);
			cursor_x	=	(GetRight()-1);
		}
		else if(cursor_x<(GetLeft()+2))
		{
			TextX		+=	(GetLeft()+2)-cursor_x;
			cursor_x	=	(GetLeft()+2);
		}
	}

	text_x	=	TextX;
	for(c0=0;c0<strlen(EditText);c0++)
	{
		if(text_x>=(GetLeft()+2))
		{
			if((text_x+QTCharWidth(EditText[c0])+1)<=(GetRight()-1))
			{
				QuickCharC(text_x,text_y,EditText[c0],text_colour);
			}
			else
				break;
		}
		text_x	+=	QTCharWidth(EditText[c0])+1;
	}

	if(GetFlags()&CONTROL_CLICKED)
	{
		Time(&the_time);
		if(the_time.MSeconds>500)
		{
			DrawVLine(cursor_x,GetTop()+1,GetBottom()-1,LOLITE_COL);
			SetFlags((UBYTE)(GetFlags()|CONTROL_SHOW_EXTRA));
		}
		else
		{
			SetFlags((UBYTE)(GetFlags()&~(CONTROL_SHOW_EXTRA)));
		}
	}
}

//---------------------------------------------------------------

extern UBYTE InkeyToAscii[];
extern UBYTE InkeyToAsciiShift[];

UWORD CEditText::TrackControl(MFPoint *down_point)
{
	CBYTE		temp_string[EDIT_TEXT_LENGTH],
				the_char;
	UBYTE		last_state	=	0;
	ULONG		count,
				update		=	0;
	SLONG		text_x;
	MFPoint		current_point,
				start_point;


//	ChangeMouseSprite(INTERFACE_POINTER(2));
//	LbMouseSetPointerHotspot(2,3);

	start_point.X	=	MouseX;
	start_point.Y	=	MouseY;
	SelectStart		=	0;
	SelectEnd		=	strlen(EditText);
	SetFlags(CONTROL_CLICKED);
	LeftButton	=	1;		// This is a fudge to force the CursorPos to be set.
	while(SHELL_ACTIVE && (GetFlags()&CONTROL_CLICKED))
	{
		if(LeftButton || RightButton)
		{
			current_point.X	=	down_point->X+(MouseX-start_point.X);
			current_point.Y	=	down_point->Y+(MouseY-start_point.Y);
			if(PointInControl(&current_point))
			{
				for(count=0,text_x=TextX;count<strlen(EditText);count++)
				{
					if(current_point.X>text_x && current_point.X<(text_x+QTCharWidth(EditText[count])+1))
					{
						CursorPos	=	count;
						break;
					}
					text_x	+=	QTCharWidth(EditText[count])+1;
				}
/*
				while(MLeftButton || MRightButton)
				{
//					SET0(63,63,63);
//					SET0(0,0,0);
				}
				LeftButton	=	0;
				RightButton	=	0;
*/
			}
			else
				break;
		}

		current_point.X	=	down_point->X+(MouseX-start_point.X);
		current_point.Y	=	down_point->Y+(MouseY-start_point.Y);
		if(PointInControl(&current_point))
			SetFlags((CONTROL_CLICKED|CONTROL_HILITED));
		else
			SetFlags(CONTROL_CLICKED);

		if(LastKey)
		{
			if(LastKey==KB_ENTER)
			{
				LastKey	=	0;
				update			=	1;
				break;
			}
			else if(LastKey==KB_ESC)
			{
				Keys[KB_ESC]	=	1;
				update			=	1;
				break;
			}
			else if(LastKey==KB_TAB)
			{
				Keys[KB_TAB]	=	1;
				update			=	1;
				break;
			}
			else if(Keys[KB_RIGHT])
			{
				CursorPos++;
				if(CursorPos>=EDIT_TEXT_LENGTH)
					CursorPos	=	EDIT_TEXT_LENGTH-1;
				else if(CursorPos>strlen(EditText))
					CursorPos	=	strlen(EditText);
				update			=	1;
			}
			else if(Keys[KB_LEFT])
			{
				CursorPos--;
				if(CursorPos<0)
					CursorPos	=	0;
				update			=	1;
			}
			else if(Keys[KB_BS])
			{
				count	=	CursorPos;
				CursorPos--;
				if(CursorPos<0)
					CursorPos	=	0;
				else
				{
					TextX	+=	QTCharWidth(EditText[CursorPos])+1;
					if(TextX>(GetLeft()+2))
						TextX	=	(GetLeft()+2);

					do
					{
						EditText[count-1]	=	EditText[count];
					}while(EditText[count++]);
				}
				update			=	1;
			}
			else if(Keys[KB_DEL])
			{
				if(CursorPos<strlen(EditText))
				{
					count	=	CursorPos;
					do
					{
						EditText[count]	=	EditText[count+1];
					}while(EditText[count++]);
					update			=	1;
				}
			}
			else if(Keys[KB_HOME])
			{
				CursorPos	=	0;
				update			=	1;
			}
			else if(Keys[KB_END])
			{
				CursorPos	=	strlen(EditText);
				update			=	1;
			}
			else
			{
				if(Keys[KB_LSHIFT] || Keys[KB_RSHIFT])
					the_char	=	InkeyToAsciiShift[LastKey];
				else
					the_char	=	InkeyToAscii[LastKey];

				if(the_char && strlen(EditText)<(EDIT_TEXT_LENGTH-1))
				{
					strcpy(temp_string,&EditText[CursorPos]);
					EditText[CursorPos]	=	the_char;
					CursorPos++;
					strcpy(&EditText[CursorPos],temp_string);
					update			=	1;
				}
				the_char		=	0;
				temp_string[0]	=	0;
			}
			LastKey	=	0;
		}


		if(LockWorkScreen())
		{
			DrawControl();
			UnlockWorkScreen();
		}

		if(GetFlags()!=last_state)
		{
			update		=	1;
			last_state	=	GetFlags();
		}

		if(update)
		{
			ShowWorkWindow(0);
			update			=	0;
		}
	}
	SetFlags((UBYTE)(GetFlags()&~(CONTROL_CLICKED)));
	DrawControl();
	ShowWorkWindow(0);

	return	GetID();
}

//---------------------------------------------------------------
// Pulldown Menu.
//---------------------------------------------------------------

CPullDown::CPullDown(ControlDef *the_def)
{
	SLONG		item_count	=	0,
				height,menu_height,width;
	MenuDef2	*current_menu_def;
	EdRect		item_rect;


	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(PULLDOWN_MENU);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	width	=	the_def->ControlWidth;
	if(width < (QTStringWidth(GetTitle())+6+8))
		width	=	QTStringWidth(GetTitle())+6+8;
	SetRect(the_def->ControlLeft,the_def->ControlTop,width,QTStringHeight()+5);

	TheMenu	=	0;
	if(the_def->TheMenuDef)
	{
		// Find widest menu item.
		height				=	GetHeight();
		width				=	0;
		current_menu_def	=	the_def->TheMenuDef;
		while(*(current_menu_def->ItemText) != '!')
		{
			if(width < QTStringWidth(current_menu_def->ItemText))
				width	=	QTStringWidth(current_menu_def->ItemText);
			current_menu_def++;
		}
		width	+=	4;

		// Set up menu items.
		item_rect.SetRect(GetRight()+1,GetTop(),width,0);
		current_menu_def	=	the_def->TheMenuDef;
		menu_height			=	0;
		while(*(current_menu_def->ItemText) != '!')
		{
			item_count++;
			current_menu_def->ItemID	=	(UBYTE)item_count;
			if(*(current_menu_def->ItemText) == '^')
			{
				current_menu_def->ItemFlags	=	MENU_SEPERATOR;
				item_rect.SetRect(item_rect.GetLeft(),item_rect.GetBottom()+1,width,(height>>1)-1);
				menu_height	+=	(height>>1)-1;
			}
			else if(*(current_menu_def->ItemText) == '@')
			{
				current_menu_def->ItemFlags	=	MENU_INACTIVE;
				item_rect.SetRect(item_rect.GetLeft(),item_rect.GetBottom()+1,width,height-1);
				menu_height	+=	height-1;
			}
			else
			{
				current_menu_def->ItemFlags	=	MENU_NORMAL;
				item_rect.SetRect(item_rect.GetLeft(),item_rect.GetBottom()+1,width,height-1);
				menu_height	+=	height-1;
			}
			current_menu_def->ItemRect	=	item_rect;
			current_menu_def++;
		}
		current_menu_def->ItemFlags	=	MENU_END;
		TheMenu	=	the_def->TheMenuDef;
		ItemsRect.SetRect(item_rect.GetLeft(),GetTop(),width,menu_height+1);
	}
}

//---------------------------------------------------------------

void CPullDown::DrawControl(void)
{
	ULONG		text_colour;
	SLONG		draw_x,
				draw_y;
	MenuDef2	*current_menu_def;


	draw_x	=	GetLeft()+(((GetWidth()-8)>>1)-(QTStringWidth(GetTitle())>>1));
	draw_y	=	GetTop()+((GetHeight()>>1)-(QTStringHeight()>>1));
	if(GetFlags()&CONTROL_INACTIVE)
	{
		QuickTextC(draw_x,draw_y,GetTitle(),LOLITE_COL);
		HiliteRect(HILITE_COL,LOLITE_COL);
	}
	else
	{
		if(GetFlags()&CONTROL_HILITED)
			FillRect(HILITE_COL);
		else
			FillRect(CONTENT_COL);

		QuickTextC(draw_x,draw_y,GetTitle(),0);
		if(GetFlags()&CONTROL_CLICKED)
		{
			if(TheMenu)
			{
				ItemsRect.FillRect(CONTENT_COL);
				current_menu_def	=	TheMenu;
				do
				{
					if(current_menu_def->ItemFlags&MENU_HILITED)
					{
						current_menu_def->ItemRect.FillRect(HILITE_COL);
					}
					else
					{
						current_menu_def->ItemRect.FillRect(CONTENT_COL);
					}
					if(current_menu_def->ItemFlags&MENU_SEPERATOR)
					{
						draw_x	=	current_menu_def->ItemRect.GetLeft();
						draw_y	=	current_menu_def->ItemRect.GetTop()+(current_menu_def->ItemRect.GetHeight()>>1);
						DrawHLineC(draw_x,current_menu_def->ItemRect.GetRight(),draw_y+1,LOLITE_COL);
						DrawHLineC(draw_x,current_menu_def->ItemRect.GetRight(),draw_y+2,HILITE_COL);
					}
					else
					{
						draw_x	=	current_menu_def->ItemRect.GetLeft()+2;
						draw_y	=	current_menu_def->ItemRect.GetTop()+((current_menu_def->ItemRect.GetHeight()>>1)-(QTStringHeight()>>1));

						if(current_menu_def->ItemFlags&MENU_INACTIVE)
							text_colour	=	LOLITE_COL;
						else
							text_colour	=	0;
						QuickTextC(draw_x,draw_y,current_menu_def->ItemText,text_colour);
					}
					current_menu_def++;
				}while(current_menu_def->ItemFlags!=MENU_END);
				ItemsRect.HiliteRect(HILITE_COL,LOLITE_COL);
			}
		}
		HiliteRect(HILITE_COL,LOLITE_COL);
		DrawMonoBSpriteC(GetLeft()+(GetWidth()-10),GetTop()+2,INTERFACE_SPRITE(MENU_ICON),0);
	}
}

//---------------------------------------------------------------

UWORD CPullDown::TrackControl(MFPoint *down_point)
{
	UBYTE		menu_id		=	0;
	SLONG		y_offset	=	0;
	MenuDef2	*current_menu_def;
	MFPoint		current_point,
				last_point,
				start_point;


	// Do the menu stuff.
	start_point.X	=	MouseX;
	start_point.Y	=	MouseY;
	SetFlags(CONTROL_CLICKED);
	while(SHELL_ACTIVE && LeftButton)
	{
		current_point.X	=	down_point->X+(MouseX-start_point.X);
		current_point.Y	=	down_point->Y+(MouseY-start_point.Y);
		if(current_point.X!=last_point.X || current_point.Y!=last_point.Y)
		{
			if(PointInControl(&current_point))
				SetFlags((CONTROL_CLICKED|CONTROL_HILITED));
			else
				SetFlags(CONTROL_CLICKED);

			if(TheMenu)
			{
				menu_id	=	0;
				current_menu_def	=	TheMenu;
				do
				{
					if(!(current_menu_def->ItemFlags&MENU_SEPERATOR))
					{
						if	(
								!(current_menu_def->ItemFlags&MENU_INACTIVE) &&
								current_menu_def->ItemRect.PointInRect(&current_point)
							)
						{
							current_menu_def->ItemFlags	|=	MENU_HILITED;
							menu_id	=	current_menu_def->ItemID;
						}
						else
							current_menu_def->ItemFlags	&=	~(MENU_HILITED);
					}
					current_menu_def++;
				}while(current_menu_def->ItemFlags!=MENU_END);
			}

			MFRect			backup_rect;

			backup_rect	=	WorkWindowRect;
			SetWorkWindowBounds	(
									backup_rect.Left,
									backup_rect.Top,
									WorkScreenPixelWidth-backup_rect.Left,
									WorkScreenHeight-backup_rect.Top
								);
			if(LockWorkScreen())
			{
				DrawControl();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);
			SetWorkWindowBounds(backup_rect.Left,backup_rect.Top,backup_rect.Width,backup_rect.Height);
			last_point	=	current_point;
/*
			if(LockWorkScreen())
			{
				DrawControl();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);
			last_point	=	current_point;
*/
		}
	}
	SetFlags((UBYTE)(GetFlags()&~(CONTROL_CLICKED)));

	if(menu_id)
		return	(UWORD)((menu_id<<8)|GetID());
	else
		return	0;
}

//---------------------------------------------------------------
// Popup Menu.
//---------------------------------------------------------------

CPopUp::CPopUp(ControlDef *the_def)
{
	SLONG		item_count	=	0,
				height,menu_height,width;
	MenuDef2	*current_menu_def;
	EdRect		item_rect;


	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetType(POPUP_MENU);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,0,QTStringHeight()+6);

	TheMenu	=	0;
	if(the_def->TheMenuDef)
	{
		// Find dimensions of menu.
		height				=	GetHeight();
		menu_height			=	0;
		width				=	0;
		current_menu_def	=	the_def->TheMenuDef;
		while(*(current_menu_def->ItemText) != '!')
		{
			// Find widest item.
			if(width < QTStringWidth(current_menu_def->ItemText))
				width	=	QTStringWidth(current_menu_def->ItemText);
			
			// Keep a tally of the total height.
			if(*(current_menu_def->ItemText) == '^')
				menu_height	+=	(height>>1)-1;
			else if(*(current_menu_def->ItemText) == '~')
				menu_height	+=	height-1;
			else
				menu_height	+=	height-1;

			current_menu_def++;
		}
		width	+=	4+10;

		// If the menu is going to end up outside the WorkWindow we need to reposition it.
		if((GetLeft()+width)>WorkWindowWidth)
		{
			MoveRect(GetLeft()-(width+4),GetTop());
		}
		if((GetTop()+menu_height)>WorkWindowHeight)
		{
			MoveRect(GetLeft(),GetTop()-((GetTop()+menu_height)-WorkWindowHeight));
		}

		// Set up menu items.
		item_rect.SetRect(GetRight()+1,GetTop(),width,0);
		current_menu_def	=	the_def->TheMenuDef;
		menu_height			=	0;
		while(*(current_menu_def->ItemText) != '!')
		{
			item_count++;
			current_menu_def->ItemID	=	(UBYTE)item_count;
			if(*(current_menu_def->ItemText) == '^')
			{
				current_menu_def->ItemFlags	|=	MENU_SEPERATOR;
				item_rect.SetRect(item_rect.GetLeft(),item_rect.GetBottom()+1,width,(height>>1)-1);
				menu_height	+=	(height>>1)-1;
			}
			else if(*(current_menu_def->ItemText) == '~')
			{
				current_menu_def->ItemFlags	|=	MENU_CHECK;
				item_rect.SetRect(item_rect.GetLeft(),item_rect.GetBottom()+1,width,height-1);
				menu_height	+=	height-1;
			}
			else
			{
				current_menu_def->ItemFlags	|=	MENU_NORMAL;
				item_rect.SetRect(item_rect.GetLeft(),item_rect.GetBottom()+1,width,height-1);
				menu_height	+=	height-1;
			}
			current_menu_def->ItemRect	=	item_rect;
			current_menu_def++;
		}
		current_menu_def->ItemFlags	=	MENU_END;
		TheMenu	=	the_def->TheMenuDef;
		ItemsRect.SetRect(item_rect.GetLeft(),GetTop(),width,menu_height+1);
	}
}

//---------------------------------------------------------------

void CPopUp::DrawControl(void)
{
	ULONG		text_colour;
	SLONG		draw_x,
				draw_y;
	MenuDef2	*current_menu_def;


	if(GetFlags()&CONTROL_CLICKED)
	{
		if(TheMenu)
		{
			ItemsRect.FillRect(CONTENT_COL);
			current_menu_def	=	TheMenu;
			do
			{
				if(current_menu_def->ItemFlags&MENU_HILITED)
				{
					current_menu_def->ItemRect.FillRect(HILITE_COL);
				}
				else
				{
					current_menu_def->ItemRect.FillRect(CONTENT_COL);
				}
				if(current_menu_def->ItemFlags&MENU_SEPERATOR)
				{
					draw_x	=	current_menu_def->ItemRect.GetLeft();
					draw_y	=	current_menu_def->ItemRect.GetTop()+(current_menu_def->ItemRect.GetHeight()>>1);
					DrawHLineC(draw_x,current_menu_def->ItemRect.GetRight(),draw_y,LOLITE_COL);
					DrawHLineC(draw_x,current_menu_def->ItemRect.GetRight(),draw_y+1,HILITE_COL);
				}
				else
				{
					draw_x	=	current_menu_def->ItemRect.GetLeft()+2;
					draw_y	=	current_menu_def->ItemRect.GetTop()+((current_menu_def->ItemRect.GetHeight()>>1)-(QTStringHeight()>>1));

					if(current_menu_def->ItemFlags&MENU_INACTIVE)
						text_colour	=	LOLITE_COL;
					else
						text_colour	=	0;
					if(current_menu_def->ItemFlags&MENU_CHECK)
					{
						if(current_menu_def->ItemFlags&MENU_CHECK_MASK)
							DrawMonoBSpriteC(draw_x,draw_y,INTERFACE_SPRITE(CHECK_ICON),text_colour);
						QuickTextC(draw_x+10,draw_y,current_menu_def->ItemText+1,text_colour);
					}
					else
						QuickTextC(draw_x+10,draw_y,current_menu_def->ItemText,text_colour);
				}
				current_menu_def++;
			}while(current_menu_def->ItemFlags!=MENU_END);
			ItemsRect.HiliteRect(HILITE_COL,LOLITE_COL);
		}
	}
}

//---------------------------------------------------------------

UWORD CPopUp::TrackControl(MFPoint *down_point)
{
	UBYTE		menu_id	=	0;
	MenuDef2	*current_menu_def;
	MFPoint		current_point,
				last_point,
				start_point;


	start_point.X	=	MouseX;
	start_point.Y	=	MouseY;
	SetFlags(CONTROL_CLICKED);
	while(SHELL_ACTIVE && (LeftButton || RightButton))
	{
		current_point.X	=	down_point->X+(MouseX-start_point.X);
		current_point.Y	=	down_point->Y+(MouseY-start_point.Y);
		if(current_point.X!=last_point.X || current_point.Y!=last_point.Y || LeftMouse.ButtonState)
		{
			if(PointInControl(&current_point))
				SetFlags((CONTROL_CLICKED|CONTROL_HILITED));
			else
				SetFlags(CONTROL_CLICKED);

			if(TheMenu)
			{
				menu_id	=	0;
				current_menu_def	=	TheMenu;
				do
				{
					if(!(current_menu_def->ItemFlags&MENU_SEPERATOR))
					{
						if(!(current_menu_def->ItemFlags&MENU_INACTIVE) && current_menu_def->ItemRect.PointInRect(&current_point))
						{
							current_menu_def->ItemFlags	|=	MENU_HILITED;
							menu_id	=	current_menu_def->ItemID;
							if(LeftMouse.ButtonState && current_menu_def->ItemFlags&MENU_CHECK)
							{
								current_menu_def->ItemFlags	^=	MENU_CHECK_MASK;
								if(current_menu_def->ItemFlags&MENU_CHECK_MASK)
								if(current_menu_def->MutualExclusiveID)
								{
									MenuDef2	*current_menu_def2;
									current_menu_def2	=	TheMenu;
									do
									{
										if((current_menu_def2->MutualExclusiveID==current_menu_def->MutualExclusiveID)&&
											(current_menu_def2!=current_menu_def))
										{
											current_menu_def2->ItemFlags&=~MENU_CHECK_MASK;
											
										}
										
										current_menu_def2++;
									}while(current_menu_def2->ItemFlags!=MENU_END);
									
								}
								LeftMouse.ButtonState	=	0;
							}
						}
						else
							current_menu_def->ItemFlags	&=	~(MENU_HILITED);
					}
					current_menu_def++;
				}while(SHELL_ACTIVE && (current_menu_def->ItemFlags!=MENU_END));
			}

			if(LockWorkScreen())
			{
				DrawControl();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);
			last_point	=	current_point;
		}
	}
	SetFlags((UBYTE)(GetFlags()&~(CONTROL_CLICKED)));

	if(menu_id && TheMenu[menu_id-1].ItemFlags&MENU_CHECK)
	{
//		TheMenu[menu_id-1].ItemFlags	^=	MENU_CHECK_MASK;
	}

	LeftMouse.ButtonState	=	0;

	return	(UWORD)((menu_id<<8)|GetID());
}

//---------------------------------------------------------------

void CPopUp::SetItemState(UWORD item,UBYTE state)
{
	switch(state)
	{
		case	CTRL_ACTIVE:
			SetItemFlags(item,(UBYTE)(GetItemFlags(item)&~(MENU_INACTIVE)));
			break;
		case	CTRL_INACTIVE:
			SetItemFlags(item,(UBYTE)(GetItemFlags(item)|MENU_INACTIVE));
			break;
	}
}

//---------------------------------------------------------------
// HSlider Menu.
//---------------------------------------------------------------

CHSlider::CHSlider(ControlDef *the_def)
{
	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetDragFlags(0);
	SetLeftButtonFlags(0);
	SetRightButtonFlags(0);
	SetType(H_SLIDER);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,the_def->ControlWidth,SLIDER_SIZE);
	LeftButtonRect.SetRect(GetLeft(),GetTop(),GetHeight(),GetHeight());
	RightButtonRect.SetRect(GetRight()-9,GetTop(),GetHeight(),GetHeight());
	DragRect.SetRect(LeftButtonRect.GetRight()+1,GetTop()+1,40,GetHeight()-2);

	SetValueStep(1);
	SetValueRange(0,0);
	SetCurrentValue(0);
	CurrentDrag	=	0;
	SetUpdateFunction(0);
}

//---------------------------------------------------------------

void	CHSlider::DrawControl(void)
{
	CBYTE		text[16];
	UBYTE		draw_colour;
	SLONG		sprite_x,
				sprite_y,
				text_x,
				text_y;


	// Draw drag rect.
	if(DragStep)
	{
		FillRect(ACTIVE_COL);
		if(DragFlags&CONTROL_HILITED)
			DragRect.FillRect(HILITE_COL);
		else
			DragRect.FillRect(CONTENT_COL);
	//	if(DragFlags&CONTROL_SHOW_EXTRA)
		{
			sprintf(text,"%d",CurrentValue);
			text_x	=	DragRect.GetLeft()+1+((DragRect.GetWidth()-QTStringWidth(text))>>1);
			text_y	=	DragRect.GetTop()+((DragRect.GetHeight()-QTStringHeight())>>1);
			QuickTextC(text_x,text_y,text,0);
		}
		DragRect.HiliteRect(HILITE_COL,LOLITE_COL);
		draw_colour	=	0;
	}
	else
	{
		FillRect(CONTENT_COL);
		draw_colour	=	LOLITE_COL;
	}

	HiliteRect(LOLITE_COL,HILITE_COL);

	// Draw left button.
	if(LeftButtonFlags&CONTROL_HILITED)
		LeftButtonRect.FillRect(HILITE_COL);
	else
		LeftButtonRect.FillRect(CONTENT_COL);
	sprite_x	=	LeftButtonRect.GetLeft()+2;
	sprite_y	=	LeftButtonRect.GetTop()+2;
	if(LeftButtonFlags&CONTROL_CLICKED)
	{
		LeftButtonRect.HiliteRect(LOLITE_COL,HILITE_COL);
		sprite_x++;
		sprite_y++;
	}
	else
		LeftButtonRect.HiliteRect(HILITE_COL,LOLITE_COL);	
	DrawMonoBSpriteC(sprite_x,sprite_y,INTERFACE_SPRITE(LEFT_ICON),draw_colour);

	// Draw right button.
	if(RightButtonFlags&CONTROL_HILITED)
		RightButtonRect.FillRect(HILITE_COL);
	else
		RightButtonRect.FillRect(CONTENT_COL);
	sprite_x	=	RightButtonRect.GetLeft()+2;
	sprite_y	=	RightButtonRect.GetTop()+2;
	if(RightButtonFlags&CONTROL_CLICKED)
	{
		RightButtonRect.HiliteRect(LOLITE_COL,HILITE_COL);
		sprite_x++;
		sprite_y++;
	}
	else
		RightButtonRect.HiliteRect(HILITE_COL,LOLITE_COL);
	DrawMonoBSpriteC(sprite_x,sprite_y,INTERFACE_SPRITE(RIGHT_ICON),draw_colour);
}

//---------------------------------------------------------------

void	CHSlider::HiliteControl(MFPoint *current_point)
{
	UnHiliteControl();
	if(DragStep)
	{
		if(DragRect.PointInRect(current_point))
			DragFlags	|=	CONTROL_HILITED;
		else if(LeftButtonRect.PointInRect(current_point))
			LeftButtonFlags	|=	CONTROL_HILITED;
		else if(RightButtonRect.PointInRect(current_point))
			RightButtonFlags	|=	CONTROL_HILITED;
		SetFlags((UBYTE)(GetFlags()|CONTROL_HILITED));
	}
}

//---------------------------------------------------------------

void	CHSlider::UnHiliteControl(void)
{
	DragFlags			&=	~(CONTROL_HILITED);
	LeftButtonFlags		&=	~(CONTROL_HILITED);
	RightButtonFlags	&=	~(CONTROL_HILITED);
	SetFlags((UBYTE)(GetFlags()&~(CONTROL_HILITED)));
}

//---------------------------------------------------------------

UWORD	CHSlider::TrackControl(MFPoint *down_point)
{
	UBYTE		start_flags,
				update;
	SLONG		drag_x,
				new_drag_x,
				start_time;
	MFPoint		current_point,
				last_point,
				start_point;
	MFTime		the_time;


	if(DragStep)
	{
		update	=	1;
		start_point.X	=	MouseX;
		start_point.Y	=	MouseY;
		if(DragRect.PointInRect(down_point))
		{
			drag_x			=	DragRect.GetLeft();
			last_point.X	=	down_point->X;
			while(SHELL_ACTIVE && LeftButton)
			{
				current_point.X	=	down_point->X+(MouseX-start_point.X);
				
				if(current_point.X!=last_point.X)
				{
					drag_x		+=	current_point.X-last_point.X;
					new_drag_x	=	(drag_x-MinDrag)*ValueStep;

					SetCurrentValue((new_drag_x<<16)/DragStep);

					update		=	1;
					last_point	=	current_point;
				}

				if(update)
				{
					if(LockWorkScreen())
					{
						DrawControl();
						if(update_function)
							update_function();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
					update			=	0;
				}
			}
			return	GetID();
		}
		else if(LeftButtonRect.PointInRect(down_point))
		{
			start_time		=	GetTickCount()/251;
			start_flags		=	LeftButtonFlags;
			while(SHELL_ACTIVE && LeftButton)
			{
				current_point.X	=	down_point->X+(MouseX-start_point.X);
				current_point.Y	=	down_point->Y+(MouseY-start_point.Y);

				if(LeftButtonRect.PointInRect(&current_point))
				{
					if(!(LeftButtonFlags&CONTROL_CLICKED))
					{
						LeftButtonFlags	=	(UBYTE)(start_flags|CONTROL_CLICKED);
						CurrentValue	-=	ValueStep;
						in_range(CurrentValue,MinValue,MaxValue);
						CurrentDrag		-=	DragStep;
						in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
						DragRect.MoveRect(MinDrag+(CurrentDrag>>16),DragRect.GetTop());
						update			=	1;
					}
					else
					{
						if((GetTickCount()/251)>(start_time+2))
						{
							CurrentValue	-=	ValueStep;
							in_range(CurrentValue,MinValue,MaxValue);
							CurrentDrag		-=	DragStep;
							in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
							DragRect.MoveRect(MinDrag+(CurrentDrag>>16),DragRect.GetTop());
							update			=	2;
							start_time		=	(GetTickCount()/251)-6;
						}
					}
				}
				else
				{
					if(LeftButtonFlags&CONTROL_CLICKED)
					{
						LeftButtonFlags	=	(UBYTE)(start_flags&~(CONTROL_HILITED));
						update			=	1;
					}
				}

				if(update)
				{
					if(LockWorkScreen())
					{
						DrawControl();
						if(update==2)
							if(update_function)
								update_function();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
					update			=	0;
				}
			}
			LeftButtonFlags	&=	~(CONTROL_CLICKED);
			return	GetID();
		}
		else if(RightButtonRect.PointInRect(down_point))
		{
			start_time		=	GetTickCount()/251;
			start_flags		=	RightButtonFlags;
			while(SHELL_ACTIVE && LeftButton)
			{
				current_point.X	=	down_point->X+(MouseX-start_point.X);
				current_point.Y	=	down_point->Y+(MouseY-start_point.Y);

				if(RightButtonRect.PointInRect(&current_point))
				{
					if(!(RightButtonFlags&CONTROL_CLICKED))
					{
						RightButtonFlags	=	(UBYTE)(start_flags|CONTROL_CLICKED);
						CurrentValue	+=	ValueStep;
						in_range(CurrentValue,MinValue,MaxValue);
						CurrentDrag		+=	DragStep;
						in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
						DragRect.MoveRect(MinDrag+(CurrentDrag>>16),DragRect.GetTop());
						update			=	1;
					}
					else
					{
						if((GetTickCount()/251)>(start_time+2))
						{
							CurrentValue	+=	ValueStep;
							in_range(CurrentValue,MinValue,MaxValue);
							CurrentDrag		+=	DragStep;
							in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
							DragRect.MoveRect(MinDrag+(CurrentDrag>>16),DragRect.GetTop());
							update			=	2;
							start_time		=	(GetTickCount()/251)-6;
						}
					}
				}
				else
				{
					if(RightButtonFlags&CONTROL_CLICKED)
					{
						RightButtonFlags	=	(UBYTE)(start_flags&~(CONTROL_HILITED));
						update			=	1;
					}
				}

				if(update)
				{
					if(LockWorkScreen())
					{
						DrawControl();
						if(update==2)
							if(update_function)
								update_function();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
					update			=	0;
				}
			}
			RightButtonFlags	&=	~(CONTROL_CLICKED);
			return	GetID();
		}
	}
	return	0;
}

//---------------------------------------------------------------

BOOL	CHSlider::PointInControl(MFPoint *the_point)
{
	if(DragRect.PointInRect(the_point))
	{
		if(LeftButtonFlags&CONTROL_HILITED || RightButtonFlags&CONTROL_HILITED)
			UnHiliteControl();
		return	TRUE;
	}
	else if(LeftButtonRect.PointInRect(the_point))
	{
		if(DragFlags&CONTROL_HILITED)
			UnHiliteControl();
		return	TRUE;
	}
	else if(RightButtonRect.PointInRect(the_point))
	{
		if(DragFlags&CONTROL_HILITED)
			UnHiliteControl();
		return	TRUE;
	}
	else
		return	FALSE;
}

//---------------------------------------------------------------

void	CHSlider::SetupDrag(void)
{
	MinDrag		=	LeftButtonRect.GetRight()+1;
	MaxDrag		=	(RightButtonRect.GetLeft())-DragRect.GetWidth();
	if(MaxValue-MinValue)
		DragStep	=	((MaxDrag-MinDrag)<<16)/((MaxValue-MinValue)/ValueStep);
	else
		DragStep	=	0;
}

//---------------------------------------------------------------

void	CHSlider::SetCurrentValue(SLONG value)
{
	CurrentValue	=	value;	
	if(ValueStep)
		CurrentValue	-=	CurrentValue%ValueStep;
	in_range(CurrentValue,MinValue,MaxValue);
	if(DragStep)
	{
		CurrentDrag		=	(CurrentValue/ValueStep)*DragStep;
		in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
		DragRect.MoveRect(MinDrag+(CurrentDrag>>16),DragRect.GetTop());
	}
}

//---------------------------------------------------------------
// VSlider Menu.
//---------------------------------------------------------------

CVSlider::CVSlider(ControlDef *the_def)
{
	SetLastControl(NULL);
	SetNextControl(NULL);

	SetFlags(0);
	SetDragFlags(0);
	SetTopButtonFlags(0);
	SetBottomButtonFlags(0);
	SetType(V_SLIDER);
	SetTitle(the_def->Title);
	SetHotKey(the_def->HotKey);

	SetRect(the_def->ControlLeft,the_def->ControlTop,SLIDER_SIZE,the_def->ControlHeight);
	TopButtonRect.SetRect(GetLeft(),GetTop(),SLIDER_SIZE,SLIDER_SIZE);
	BottomButtonRect.SetRect(GetLeft(),GetBottom()-(SLIDER_SIZE-1),SLIDER_SIZE,SLIDER_SIZE);
	DragRect.SetRect(GetLeft()+1,TopButtonRect.GetBottom()+1,GetWidth()-2,40);

	SetValueStep(1);
	SetValueRange(0,0);
	SetCurrentValue(0);
	CurrentDrag	=	0;
	SetUpdateFunction(0);
}

//---------------------------------------------------------------

void	CVSlider::DrawControl(void)
{
	UBYTE		draw_colour;
	SLONG		sprite_x,
				sprite_y,
				text_x,
				text_y;


	// Draw drag rect.
	if(DragStep)
	{
		FillRect(ACTIVE_COL);
	
		if(DragFlags&CONTROL_HILITED)
			DragRect.FillRect(HILITE_COL);
		else
			DragRect.FillRect(CONTENT_COL);
		DragRect.HiliteRect(HILITE_COL,LOLITE_COL);
		draw_colour	=	0;
	}
	else
	{
		FillRect(CONTENT_COL);
		draw_colour	=	LOLITE_COL;
	}

	HiliteRect(LOLITE_COL,HILITE_COL);

	// Draw top button.
	if(TopButtonFlags&CONTROL_HILITED)
		TopButtonRect.FillRect(HILITE_COL);
	else
		TopButtonRect.FillRect(CONTENT_COL);
	sprite_x	=	TopButtonRect.GetLeft()+2;
	sprite_y	=	TopButtonRect.GetTop()+2;
	if(TopButtonFlags&CONTROL_CLICKED)
	{
		TopButtonRect.HiliteRect(LOLITE_COL,HILITE_COL);
		sprite_x++;
		sprite_y++;
	}
	else
		TopButtonRect.HiliteRect(HILITE_COL,LOLITE_COL);	
	DrawMonoBSpriteC(sprite_x,sprite_y,INTERFACE_SPRITE(UP_ICON),draw_colour);

	// Draw bottom button.
	if(BottomButtonFlags&CONTROL_HILITED)
		BottomButtonRect.FillRect(HILITE_COL);
	else
		BottomButtonRect.FillRect(CONTENT_COL);
	sprite_x	=	BottomButtonRect.GetLeft()+2;
	sprite_y	=	BottomButtonRect.GetTop()+2;
	if(BottomButtonFlags&CONTROL_CLICKED)
	{
		BottomButtonRect.HiliteRect(LOLITE_COL,HILITE_COL);
		sprite_x++;
		sprite_y++;
	}
	else
		BottomButtonRect.HiliteRect(HILITE_COL,LOLITE_COL);
	DrawMonoBSpriteC(sprite_x,sprite_y,INTERFACE_SPRITE(DOWN_ICON),draw_colour);
}

//---------------------------------------------------------------

void	CVSlider::HiliteControl(MFPoint *current_point)
{
	UnHiliteControl();
	if(DragStep)
	{
		if(DragRect.PointInRect(current_point))
			DragFlags	|=	CONTROL_HILITED;
		else if(TopButtonRect.PointInRect(current_point))
			TopButtonFlags	|=	CONTROL_HILITED;
		else if(BottomButtonRect.PointInRect(current_point))
			BottomButtonFlags	|=	CONTROL_HILITED;
		SetFlags((UBYTE)(GetFlags()|CONTROL_HILITED));
	}
}

//---------------------------------------------------------------

void	CVSlider::UnHiliteControl(void)
{
	DragFlags			&=	~(CONTROL_HILITED);
	TopButtonFlags		&=	~(CONTROL_HILITED);
	BottomButtonFlags	&=	~(CONTROL_HILITED);
	SetFlags((UBYTE)(GetFlags()&~(CONTROL_HILITED)));
}

//---------------------------------------------------------------

UWORD	CVSlider::TrackControl(MFPoint *down_point)
{
	UBYTE		start_flags,
				update;
	SLONG		drag_y,
				new_drag_y,
				start_time;
	MFPoint		current_point,
				last_point,
				start_point;
	MFTime		the_time;


	if(DragStep)
	{
		update	=	1;
		start_point.X	=	MouseX;
		start_point.Y	=	MouseY;
		if(DragRect.PointInRect(down_point))
		{
			drag_y			=	DragRect.GetTop();
			last_point.Y	=	down_point->Y;
			while(SHELL_ACTIVE && LeftButton)
			{
				current_point.Y	=	down_point->Y+(MouseY-start_point.Y);
				
				if(current_point.Y!=last_point.Y)
				{
					drag_y		+=	current_point.Y-last_point.Y;
					new_drag_y	=	(drag_y-MinDrag)*ValueStep;

					SetCurrentValue((new_drag_y<<16)/DragStep);

					update		=	1;
					last_point	=	current_point;
				}

				if(update)
				{
					if(LockWorkScreen())
					{
						DrawControl();
						if(update_function)
							update_function();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
					update			=	0;
				}
			}
			return	GetID();
		}
		else if(TopButtonRect.PointInRect(down_point))
		{
			start_time		=	GetTickCount()/251;
			start_flags		=	TopButtonFlags;
			while(SHELL_ACTIVE && LeftButton)
			{
				current_point.X	=	down_point->X+(MouseX-start_point.X);
				current_point.Y	=	down_point->Y+(MouseY-start_point.Y);

				if(TopButtonRect.PointInRect(&current_point))
				{
					if(!(TopButtonFlags&CONTROL_CLICKED))
					{
						TopButtonFlags	=	(UBYTE)(start_flags|CONTROL_CLICKED);
						CurrentValue	-=	ValueStep;
						in_range(CurrentValue,MinValue,MaxValue);
						CurrentDrag		-=	DragStep;
						in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
						DragRect.MoveRect(DragRect.GetLeft(),MinDrag+(CurrentDrag>>16));
						update			=	2;
					}
					else
					{
						if((GetTickCount()/251)>(start_time+2))
						{
							CurrentValue	-=	ValueStep;
							in_range(CurrentValue,MinValue,MaxValue);
							CurrentDrag		-=	DragStep;
							in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
							DragRect.MoveRect(DragRect.GetLeft(),MinDrag+(CurrentDrag>>16));
							update			=	2;
							start_time		=	(GetTickCount()/251)-6;
						}
					}
				}
				else
				{
					if(TopButtonFlags&CONTROL_CLICKED)
					{
						TopButtonFlags	=	(UBYTE)(start_flags&~(CONTROL_HILITED));
						update			=	1;
					}
				}

				if(update)
				{
					if(LockWorkScreen())
					{
						DrawControl();
						if(update==2)
							if(update_function)
								update_function();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
					update			=	0;
				}
			}
			TopButtonFlags	&=	~(CONTROL_CLICKED);
			return	GetID();
		}
		else if(BottomButtonRect.PointInRect(down_point))
		{
			start_time		=	GetTickCount()/251;
			start_flags		=	BottomButtonFlags;
			while(SHELL_ACTIVE && LeftButton)
			{
				current_point.X	=	down_point->X+(MouseX-start_point.X);
				current_point.Y	=	down_point->Y+(MouseY-start_point.Y);

				if(BottomButtonRect.PointInRect(&current_point))
				{
					if(!(BottomButtonFlags&CONTROL_CLICKED))
					{
						BottomButtonFlags	=	(UBYTE)(start_flags|CONTROL_CLICKED);
						CurrentValue	+=	ValueStep;
						in_range(CurrentValue,MinValue,MaxValue);
						CurrentDrag		+=	DragStep;
						in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
						DragRect.MoveRect(DragRect.GetLeft(),MinDrag+(CurrentDrag>>16));
						update			=	2;
					}
					else
					{
						if((GetTickCount()/251)>(start_time+2))
						{
							CurrentValue	+=	ValueStep;
							in_range(CurrentValue,MinValue,MaxValue);
							CurrentDrag		+=	DragStep;
							in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
							DragRect.MoveRect(DragRect.GetLeft(),MinDrag+(CurrentDrag>>16));
							update			=	2;
							start_time		=	(GetTickCount()/251)-6;
						}
					}
				}
				else
				{
					if(BottomButtonFlags&CONTROL_CLICKED)
					{
						BottomButtonFlags	=	(UBYTE)(start_flags&~(CONTROL_HILITED));
						update			=	1;
					}
				}

				if(update)
				{
					if(LockWorkScreen())
					{
						DrawControl();
						if(update==2)
							if(update_function)
								update_function();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
					update			=	0;
				}
			}
			BottomButtonFlags	&=	~(CONTROL_CLICKED);
			return	GetID();
		}
	}
	return	0;
}

//---------------------------------------------------------------

BOOL	CVSlider::PointInControl(MFPoint *the_point)
{
	if(DragRect.PointInRect(the_point))
	{
		if(TopButtonFlags&CONTROL_HILITED || BottomButtonFlags&CONTROL_HILITED)
			UnHiliteControl();
		return	TRUE;
	}
	else if(TopButtonRect.PointInRect(the_point))
	{
		if(DragFlags&CONTROL_HILITED)
			UnHiliteControl();
		return	TRUE;
	}
	else if(BottomButtonRect.PointInRect(the_point))
	{
		if(DragFlags&CONTROL_HILITED)
			UnHiliteControl();
		return	TRUE;
	}
	else
		return	FALSE;
}

//---------------------------------------------------------------

void	CVSlider::SetupDrag(void)
{
	MinDrag		=	TopButtonRect.GetBottom()+1;
	MaxDrag		=	(BottomButtonRect.GetTop())-DragRect.GetHeight();
	if(MaxValue-MinValue)
		DragStep	=	((MaxDrag-MinDrag)<<16)/((MaxValue-MinValue)/ValueStep);
	else
		DragStep	=	0;

	SetCurrentValue(CurrentValue);
}

//---------------------------------------------------------------

void	CVSlider::SetCurrentValue(SLONG value)
{
	CurrentValue	=	value;	
	if(ValueStep)
		CurrentValue	-=	CurrentValue%ValueStep;
	in_range(CurrentValue,MinValue,MaxValue);
	if(DragStep)
	{
		CurrentDrag		=	(CurrentValue/ValueStep)*DragStep;
		in_range(CurrentDrag,0,(MaxDrag-MinDrag)<<16);
		DragRect.MoveRect(DragRect.GetLeft(),MinDrag+(CurrentDrag>>16));
	}
}

//---------------------------------------------------------------

#endif
