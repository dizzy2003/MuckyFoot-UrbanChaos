// EditMod.cpp
// Guy Simmons, 19th February 1997.

#include	"Editor.hpp"


//---------------------------------------------------------------

EditorModule::EditorModule()
{
	SetLastModuleLink(NULL);
	SetNextModuleLink(NULL);
	SetExternalUpdatePtr(0);

	EscapeFlag	=	FALSE;
}

//---------------------------------------------------------------

void EditorModule::SetupModule(void)
{
	SetupWindow("This module has no SetupModule function",(HAS_TITLE),0,0,300,50);
}

//---------------------------------------------------------------

void EditorModule::MoveModule(MFPoint *clicked_point)
{
	SLONG		x_diff,
				y_diff;
	EdRect		last_rect,
				temp_rect;


	SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);

	x_diff	=	clicked_point->X-GetLeft();
	y_diff	=	clicked_point->Y-GetTop();
	temp_rect.SetRect(GetLeft(),GetTop(),GetWidth(),GetHeight());
	last_rect	=	temp_rect;
	while(SHELL_ACTIVE && LeftButton)
	{
		temp_rect.SetRect(MouseX-x_diff,MouseY-y_diff,temp_rect.GetWidth(),temp_rect.GetHeight());
		if(temp_rect.GetLeft()<0)
			temp_rect.MoveRect(0,temp_rect.GetTop());
		if(temp_rect.GetTop()<20)
			temp_rect.MoveRect(temp_rect.GetLeft(),20);
		if(temp_rect.GetRight()>=WorkScreenPixelWidth)
			temp_rect.MoveRect(WorkScreenPixelWidth-temp_rect.GetWidth(),temp_rect.GetTop());
		if(temp_rect.GetBottom()>=WorkScreenHeight)
			temp_rect.MoveRect(temp_rect.GetLeft(),WorkScreenHeight-temp_rect.GetHeight());

		if	(
				temp_rect.GetLeft()!=last_rect.GetLeft() ||
				temp_rect.GetTop()!=last_rect.GetTop() ||
				temp_rect.GetRight()!=last_rect.GetRight() ||
				temp_rect.GetBottom()!=last_rect.GetBottom()
			)
		{

			if(LockWorkScreen())
			{
				temp_rect.OutlineInvertedRect();
				UnlockWorkScreen();
			}
			ShowWorkScreen(0);
			if(LockWorkScreen())
			{
				temp_rect.OutlineInvertedRect();
				UnlockWorkScreen();
			}
			last_rect	=	temp_rect;
		}
	}
	MoveWindow(temp_rect.GetLeft(),temp_rect.GetTop());
}

//---------------------------------------------------------------

void EditorModule::SizeModule(MFPoint *clicked_point)
{
	SLONG		height,
				x_diff,
				y_diff;
	EdRect		last_rect,
				temp_rect;


	SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);

	temp_rect.SetRect(GetLeft(),GetTop(),GetWidth(),GetHeight());
	last_rect	=	temp_rect;
	while(SHELL_ACTIVE && LeftButton)
	{
		x_diff	=	MouseX-clicked_point->X;
		y_diff	=	MouseY-clicked_point->Y;

		if(GetRight()+x_diff>=WorkWindowWidth)
			x_diff	=	(WorkScreenPixelWidth-1)-GetRight();
		if(GetBottom()+y_diff>=WorkScreenHeight)
			y_diff	=	(WorkScreenHeight-1)-GetBottom();

		height	=	GetHeight()+y_diff;
		ConstrainHeight(&height);
		temp_rect.SetRect(GetLeft(),GetTop(),GetWidth()+x_diff,height);
		if	(
				temp_rect.GetLeft()!=last_rect.GetLeft() ||
				temp_rect.GetTop()!=last_rect.GetTop() ||
				temp_rect.GetRight()!=last_rect.GetRight() ||
				temp_rect.GetBottom()!=last_rect.GetBottom()
			)
		{
			if(LockWorkScreen())
			{
				temp_rect.OutlineInvertedRect();
				UnlockWorkScreen();
			}
			ShowWorkScreen(0);
			if(LockWorkScreen())
			{
				temp_rect.OutlineInvertedRect();
				UnlockWorkScreen();
			}
			last_rect	=	temp_rect;
		}
	}
	SizeWindow(x_diff,y_diff);
}

//---------------------------------------------------------------

void EditorModule::HandleContentClick(UBYTE flags,MFPoint *clicked_point)
{
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(WhereInWindow(clicked_point) == IN_CONTENT)
			{
				// Left click in content.
			}
			break;
		case	RIGHT_CLICK:
			if(WhereInWindow(clicked_point) == IN_CONTENT)
			{
				// Right click in content.
			}
			break;
	}
}

//---------------------------------------------------------------

void EditorModule::HandleControlClick(UBYTE flags,MFPoint *clicked_point)
{
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(WhereInWindow(clicked_point) == IN_CONTROLS)
			{
				// Left click in content.
			}
			break;
		case	RIGHT_CLICK:
			if(WhereInWindow(clicked_point) == IN_CONTROLS)
			{
				// Right click in content.
			}
			break;
	}
}

//---------------------------------------------------------------

void EditorModule::HandleModule(void)
{
	
}

//---------------------------------------------------------------
