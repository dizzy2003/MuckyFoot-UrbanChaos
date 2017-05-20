// Mouse.cpp
// Guy Simmons, 19th February 1997.

#include	<MFHeader.h>



volatile UBYTE		MouseMoved		=	0,
					LeftButton		=	0,
					MiddleButton	=	0,
					RightButton		=	0;
volatile SLONG		MouseX,
					MouseY;
volatile LastMouse	LeftMouse		=	{	0,0,0,{	0,0	}	},
					MiddleMouse		=	{	0,0,0,{	0,0	}	},
					RightMouse		=	{	0,0,0,{	0,0	}	};
volatile MFPoint	MousePoint		=	{	0,0				};
