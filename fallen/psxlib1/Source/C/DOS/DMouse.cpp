#include	<MFHeader.h>
#include	"I86.h"
#include	"string.h"
#include	"stdio.h"
#include	"conio.h"
#include	"dos.h"


//*************************************MOUSE STUFF ****************

volatile UBYTE		LeftButton		=	0,
					MiddleButton	=	0,
					RightButton		=	0;
volatile SLONG		MouseX,
					MouseY;
volatile LastMouse	LeftMouse		=	{	0,0,0,{	0,0	}	},
					MiddleMouse		=	{	0,0,0,{	0,0	}	},
					RightMouse		=	{	0,0,0,{	0,0	}	};
volatile MFPoint	MousePoint		=	{	0,0				};


static	void	_interrupt _loadds far MouseInterrupt(int max, int mickey_x, int mickey_y);
struct	mouse_info
{
	SLONG	XMoveRatio;
	SLONG	YMoveRatio;
	SLONG	XSpriteOffset;
	SLONG	YSpriteOffset;
	UBYTE	Sprite[32*32];
};

struct	mouse_buffer
{
	UBYTE	Valid;
	SLONG	Width;
	SLONG	Height;
	ULONG	Offset;
	UBYTE	Buffer[32*32];
	
	SLONG	X;
	SLONG	Y;
	SLONG	XOffset;
	SLONG	YOffset;
};

static	mouse_setup=0;
static	struct	mouse_buffer	mbuffer;
static	struct	mouse_info		minfo;
static	volatile	SWORD		mouse_mickey_x = 0;
static	volatile	SWORD		mouse_mickey_y = 0;
static	mouse_int_setup=0;


void	clip_mouse(void)
{
	if(MouseX<0)
		MouseX=0;

	if(MouseY<0)
		MouseY=0;

	if(MouseX>WorkScreenWidth)
		MouseX=WorkScreenWidth;

	if(MouseY>WorkScreenHeight)
		MouseY=WorkScreenHeight;

}


static	void	_interrupt _loadds far MouseInterrupt(int event, int mx, int my)
{
	#pragma aux MouseInterrupt parm [EAX] [ESI] [EDI]
	
	static	SWORD	old_mx = 0;	// Old mickey counts
	static	SWORD	old_my = 0;
	static	SLONG	old_rx = 0; // Remainder from move ratio divisions
	static	SLONG	old_ry = 0;
	SLONG			dx, dy;
	SWORD			mickey_x, mickey_y;	// Got to move ESI/EDI into SWORD vars to make use of 16bit wrap
	UWORD			vesa_page;

	if (!mouse_setup)	// Remember may just be suspended!
	{
		return;
	}

	// Process button presses
	if (event & 2)
	{
		LeftButton		=	1;
		if(!LeftMouse.ButtonState)
		{
			LeftMouse.ButtonState	=	1;
			LeftMouse.MouseX		=	MouseX;
			LeftMouse.MouseY		=	MouseY;
			LeftMouse.MousePoint.X	=	MouseX;
			LeftMouse.MousePoint.Y	=	MouseY;
		}

	}

	if (event & 4)
	{
		LeftButton = 0;
	}

	if (event & 8)
	{
		RightButton		=	1;
		if(!RightMouse.ButtonState)
		{
			RightMouse.ButtonState	=	1;
			RightMouse.MouseX		=	MouseX;
			RightMouse.MouseY		=	MouseY;
			RightMouse.MousePoint.X	=	MouseX;
			RightMouse.MousePoint.Y	=	MouseY;
		}

	}

	if (event & 16)
	{
		RightButton		=	0;
	}
	
	if (event & 32)
	{
		MiddleButton	=	1;
		if(!MiddleMouse.ButtonState)
		{
			MiddleMouse.ButtonState	=	1;
			MiddleMouse.MouseX		=	MouseX;
			MiddleMouse.MouseY		=	MouseY;
			MiddleMouse.MousePoint.X=	MouseX;
			MiddleMouse.MousePoint.Y=	MouseY;
		}

	}
	if (event & 64)
	{
		MiddleButton	=	0;
	}
/*
	if (redraw_active)
	{
		// Remember new position
		mouse_mickey_x = (SWORD)mx;
		mouse_mickey_y = (SWORD)my;
		return;
	}
*/

	// Process movement
	mickey_x = (SWORD)mx;
	mickey_y = (SWORD)my;
	old_mx = mouse_mickey_x;
	old_my = mouse_mickey_y;
	mouse_mickey_x = mickey_x;
	mouse_mickey_y = mickey_y;
	
	dx = mouse_mickey_x - old_mx;
	if (dx > 0)
	{
		dx += old_rx;
		old_rx = dx % minfo.XMoveRatio;
		MouseX += dx / minfo.XMoveRatio;
	}
	else if (dx < 0)
	{
		MouseX += dx / minfo.XMoveRatio;
		old_rx += dx % minfo.XMoveRatio;
		if (old_rx < 0)
		{
			MouseX--;
			old_rx = minfo.XMoveRatio + (old_rx % minfo.XMoveRatio);
		}
	}

	dy = mouse_mickey_y - old_my;
	if (dy > 0)
	{
		dy += old_ry;
		old_ry = dy % minfo.YMoveRatio;
		MouseY += dy / minfo.YMoveRatio;
	}
	else if (dy < 0)
	{
		MouseY += dy / minfo.YMoveRatio;
		old_ry += dy % minfo.YMoveRatio;
		if (old_ry < 0)
		{
			MouseY--;
			old_ry = minfo.YMoveRatio + (old_ry % minfo.YMoveRatio);
		}
	}
	clip_mouse();
//	adjust_point((SLONG *)&lbDisplay.MMouseX, (SLONG *)&lbDisplay.MMouseY);
/*
	if (!lbScreenDirectAccessActive)
	{
		// Redraw mouse from/to phyiscal screen, saving original vesa page
		vesa_page = lbVesaPage;
		screen_remove(FALSE);
		screen_place();
		if (lbDisplay.VesaIsSetUp)
		{
			LbVesaSetPage(vesa_page);
		}
	}
*/
}



SLONG	SetupMouse(void)
{
	struct 	SREGS 	sregs ;
	union 	REGS 	inregs;
	union 	REGS 	outregs;
	
	if (mouse_setup)
	{
		return (0);		
	}

	if (!mouse_setup)
	{
		segread(&sregs);
		inregs.w.ax = 0;
		int386(0x33, &inregs, &outregs);
		if (outregs.w.ax != 0xffff)
 		{
			return (0);
		}
	}
	mouse_setup = 1;

	// Reset everything
	minfo.XMoveRatio = 1;
	minfo.YMoveRatio = 1;
	minfo.XSpriteOffset = 0;
	minfo.YSpriteOffset = 0;

	if (!mouse_int_setup)
	{
		// Install Interrupt handler
		inregs.w.ax = 0xC;
		inregs.w.cx = 0x7f;
		inregs.x.edx = FP_OFF(MouseInterrupt);
		sregs.es = FP_SEG(MouseInterrupt);
		int386x(0x33, &inregs, &outregs, &sregs);
		mouse_int_setup = 1;
	}		
	return (1);
}

SLONG	ResetMouse(void)
{
	union 	REGS 	inregs;
	union 	REGS 	outregs;

	if (!mouse_int_setup)
	{
		return (0);		
	}

	inregs.w.ax = 0;
	int386 (0x33, &inregs, &outregs);

	mouse_setup = 0;
	mouse_setup		  = 0;
	
	return (1);
}

