// Alert.cpp
// Guy Simmons, 11th April 1997.

#include	"Editor.hpp"


#define	BOX_WIDTH		250
#define	BOX_HEIGHT		100


ControlDef	buttons[]	=
{
	{	BUTTON,			KB_ENTER,	"Okay",		10,					BOX_HEIGHT-26,	50,	0			},
	{	BUTTON,			KB_ESC,		"Cancel",	BOX_WIDTH-(70-10),	BOX_HEIGHT-26,	50,	0			},

	{	0	}
};

//---------------------------------------------------------------

Alert::Alert(CBYTE *text1,CBYTE *text2)
{
	SetRect	(
				(WorkScreenPixelWidth-BOX_WIDTH)>>1,
				(WorkScreenHeight-BOX_HEIGHT)>>1,
				BOX_WIDTH,
				BOX_HEIGHT
			);
	AlertSet.InitControlSet(buttons);
	AlertSet.ControlSetBounds(this);
	HandleAlert(text1,text2);
}

Alert::Alert()
{
	SetRect	(
				(WorkScreenPixelWidth-BOX_WIDTH)>>1,
				(WorkScreenHeight-BOX_HEIGHT)>>1,
				BOX_WIDTH,
				BOX_HEIGHT
			);
	AlertSet.InitControlSet(buttons);
	AlertSet.ControlSetBounds(this);
}

//---------------------------------------------------------------

Alert::~Alert()
{
	
}

//---------------------------------------------------------------

BOOL	Alert::HandleAlert(CBYTE *text1,CBYTE *text2)
{
	UBYTE		control_id,
				update	=	1;
	SLONG		temp_x,
				temp_y,
				temp_height,
				temp_width,
				text1_x,
				text1_y,
				text2_x,
				text2_y;
	MFPoint		mouse_point;



	temp_x		=	WorkWindowRect.Left;
	temp_y		=	WorkWindowRect.Top;
	temp_height	=	WorkWindowRect.Height;
	temp_width	=	WorkWindowRect.Width;

	SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);

	if(text1)
	{
		text1_x	=	GetLeft()+((BOX_WIDTH-QTStringWidth(text1))>>1);
		text1_y	=	GetTop()+20;
	}

	if(text2)
	{
		text2_x	=	GetLeft()+((BOX_WIDTH-QTStringWidth(text2))>>1);
		text2_y	=	text1_y+20;
	}

	if(LockWorkScreen())
	{
		FillRect(CONTENT_COL);
		HiliteRect(HILITE_COL,LOLITE_COL);

		if(text1)
			QuickTextC(text1_x,text1_y,text1,TEXT_COL);
		if(text2)
			QuickTextC(text2_x,text2_y,text2,TEXT_COL);

		AlertSet.SetControlDrawArea();
		AlertSet.DrawControlSet();

		UnlockWorkScreen();
		ShowWorkScreen(0);
	}

	while(SHELL_ACTIVE)
	{
		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;
		AlertSet.HandleControlSet(&mouse_point);

		if(LeftMouse.ButtonState)
		{
			mouse_point.X			=	LeftMouse.MouseX;
			mouse_point.Y			=	LeftMouse.MouseY;
			LeftMouse.ButtonState	=	0;
			control_id				=	(UBYTE)AlertSet.HandleControlSetClick(LEFT_CLICK,&mouse_point);
			if(control_id==1)
				return	TRUE;
			else if(control_id==2)
				return	FALSE;
		}
		else if(LastKey)
		{
			control_id				=	(UBYTE)AlertSet.HandleControlSetKey(LastKey);
			if(control_id)
			{
				LastKey	=	0;
				if(control_id==1)
					return	TRUE;
				else if(control_id==2)
					return	FALSE;
			}
		}
	}
	SetWorkWindowBounds(temp_x,temp_y,temp_width,temp_height);
	return	FALSE;
}

//---------------------------------------------------------------
