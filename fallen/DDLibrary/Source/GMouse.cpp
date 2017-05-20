// GMouse.cpp
// Guy Simmons, 11th December 1997.

#include	"DDLib.h"


//---------------------------------------------------------------



volatile UBYTE		MouseMoved		=	0,
					LeftButton		=	0,
					MiddleButton	=	0,
					RightButton		=	0;
volatile SLONG		MouseX,
					MouseY;

volatile SLONG		MouseDX,MouseDY;

SLONG	OldMouseX,OldMouseY;
					
volatile LastMouse	LeftMouse		=	{	0,0,0,{	0,0	}	},
					MiddleMouse		=	{	0,0,0,{	0,0	}	},
					RightMouse		=	{	0,0,0,{	0,0	}	};
volatile MFPoint	MousePoint		=	{	0,0				};


//---------------------------------------------------------------

LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	switch(code)
	{
		case	WM_MOUSEMOVE:
			MouseX			=	LOWORD(lParam);
			MouseY			=	HIWORD(lParam);

			MouseDX			=	MouseX-OldMouseX;
			MouseDY			=	MouseY-OldMouseY;
			MousePoint.X	=	MouseX;
			MousePoint.Y	=	MouseY;
			MouseMoved		=	1;

			OldMouseX=MouseX;
			OldMouseY=MouseY;

			break;
		case	WM_RBUTTONUP:
			RightButton		=	0;
			break;
		case	WM_RBUTTONDOWN:
			RightButton		=	1;
			if(!RightMouse.ButtonState)
			{
				RightMouse.ButtonState	=	1;
				RightMouse.MouseX		=	LOWORD(lParam);
				RightMouse.MouseY		=	HIWORD(lParam);
				RightMouse.MousePoint.X	=	LOWORD(lParam);
				RightMouse.MousePoint.Y	=	HIWORD(lParam);
			}
			break;
		case	WM_RBUTTONDBLCLK:
			break;
		case	WM_LBUTTONUP:
			LeftButton		=	0;
			break;
		case	WM_LBUTTONDOWN:
			LeftButton		=	1;
			if(!LeftMouse.ButtonState)
			{
				LeftMouse.ButtonState	=	1;
				LeftMouse.MouseX		=	LOWORD(lParam);
				LeftMouse.MouseY		=	HIWORD(lParam);
				LeftMouse.MousePoint.X	=	LOWORD(lParam);
				LeftMouse.MousePoint.Y	=	HIWORD(lParam);
			}
			break;
		case	WM_LBUTTONDBLCLK:
			break;
		case	WM_MBUTTONUP:
			MiddleButton	=	0;
			break;
		case	WM_MBUTTONDOWN:
			MiddleButton	=	1;
			if(!MiddleMouse.ButtonState)
			{
				MiddleMouse.ButtonState	=	1;
				MiddleMouse.MouseX		=	LOWORD(lParam);
				MiddleMouse.MouseY		=	HIWORD(lParam);
				MiddleMouse.MousePoint.X=	LOWORD(lParam);
				MiddleMouse.MousePoint.Y=	HIWORD(lParam);
			}
			break;
		case	WM_MBUTTONDBLCLK:
			break;
	}
	return	FALSE;
}


void	RecenterMouse(void)
{
	RECT	client_rect;
	POINT	p;

	GetWindowRect(hDDLibWindow,&client_rect);
//	Get mouse position relative to engine window.

	SetCursorPos((client_rect.left+client_rect.right)>>1,(client_rect.top+client_rect.bottom)>>1);
	p.x=(client_rect.left+client_rect.right)>>1;
	p.y=(client_rect.top+client_rect.bottom)>>1;

	ScreenToClient(hDDLibWindow,&p);


	OldMouseX=p.x;
	OldMouseY=p.y;

}


//---------------------------------------------------------------
