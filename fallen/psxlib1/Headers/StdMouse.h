// StdMouse.h
// Guy Simmons, 18th December 1997.

#ifndef	STD_MOUSE_H
#define	STD_MOUSE_H

//---------------------------------------------------------------

struct LastMouse
{
	SLONG		ButtonState,
				MouseX,
				MouseY;
	MFPoint		MousePoint;
};


extern volatile UBYTE		MouseMoved,
							LeftButton,
							MiddleButton,
							RightButton;
extern volatile SLONG		MouseX,
							MouseY;
extern volatile LastMouse	LeftMouse,
							MiddleMouse,
							RightMouse;
extern volatile MFPoint		MousePoint;

//---------------------------------------------------------------

#endif
