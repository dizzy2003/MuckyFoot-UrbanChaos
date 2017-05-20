// Mouse.h
// Guy Simmons, 19th February 1997.

#ifndef	_MOUSE_H_
#define	_MOUSE_H_


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

extern	void	RemoveMouse(void);
extern	void	PlaceMouse(void);

#endif
