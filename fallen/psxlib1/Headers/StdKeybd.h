// StdKeybd.h
// Guy Simmons, 18th December 1997.

#ifndef	STD_KEYBD_H
#define	STD_KEYBD_H

//---------------------------------------------------------------

// Row1
#define	KB_ESC			0x01
#define	KB_1			0x02
#define	KB_2			0x03
#define	KB_3			0x04
#define	KB_4			0x05
#define	KB_5			0x06
#define	KB_6			0x07
#define	KB_7			0x08
#define	KB_8			0x09
#define	KB_9			0x0a
#define	KB_0			0x0b
#define	KB_MINUS		0x0c
#define	KB_PLUS			0x0d
#define	KB_BS			0x0e

// Row2
#define	KB_TAB			0x0f
#define	KB_Q			0x10
#define	KB_W			0x11
#define	KB_E			0x12
#define	KB_R			0x13
#define	KB_T			0x14
#define	KB_Y			0x15
#define	KB_U			0x16
#define	KB_I			0x17
#define	KB_O			0x18
#define	KB_P			0x19
#define	KB_LBRACE		0x1a
#define	KB_RBRACE		0x1b
#define	KB_ENTER		0x1c

// Row3
#define	KB_LCONTROL		0x1d
#define	KB_CAPSLOCK		0x3a
#define	KB_A			0x1e
#define	KB_S			0x1f
#define	KB_D			0x20
#define	KB_F			0x21
#define	KB_G			0x22
#define	KB_H			0x23
#define	KB_J			0x24
#define	KB_K			0x25
#define	KB_L			0x26
#define	KB_COLON		0x27
#define	KB_QUOTE		0x28
#define	KB_TILD			0x29

// Row4
#define	KB_LSHIFT		0x2a
#define	KB_BACKSLASH	0x2b
#define	KB_Z			0x2c
#define	KB_X			0x2d
#define	KB_C			0x2e
#define	KB_V			0x2f
#define	KB_B			0x30
#define	KB_N			0x31
#define	KB_M			0x32
#define	KB_COMMA		0x33
#define	KB_POINT		0x34
#define	KB_FORESLASH	0x35
#define	KB_RSHIFT		0x36
#define	KB_LALT			0x38
#define	KB_SPACE		0x39
#define	KB_RALT			(0x38+0x80)
#define	KB_RCONTROL		(0x1d+0x80)

// Function key row.
#define	KB_F1			0x3b
#define	KB_F2			0x3c
#define	KB_F3			0x3d
#define	KB_F4			0x3e

#define	KB_F5			0x3f
#define	KB_F6			0x40
#define	KB_F7			0x41
#define	KB_F8			0x42

#define	KB_F9			0x43
#define	KB_F10			0x44
#define	KB_F11			0x57
#define	KB_F12			0x58

#define	KB_PRTSC		(0x37+0x80)
#define	KB_SCROLLLOCK	0x46
//#define	KB_PAUSE		????

// Edit pad.
#define	KB_INS			(0x52+0x80)
#define	KB_HOME			(0x47+0x80)
#define	KB_PGUP			(0x49+0x80)
#define	KB_DEL			(0x53+0x80)
#define	KB_END			(0x4f+0x80)
#define	KB_PGDN			(0x51+0x80)

// Cursor pad.
#define	KB_LEFT			(0x4b+0x80)
#define	KB_UP			(0x48+0x80)
#define	KB_RIGHT		(0x4d+0x80)
#define	KB_DOWN			(0x50+0x80)

// Key pad.
#define	KB_NUMLOCK		0x45
#define	KB_PSLASH		(0x35+0x80)
#define	KB_ASTERISK		0x37
#define	KB_PMINUS		0x4a
#define	KB_P7			0x47
#define	KB_P8			0x48
#define	KB_P9			0x49
#define	KB_PPLUS		0x4e
#define	KB_P4			0x4b
#define	KB_P5			0x4c
#define	KB_P6			0x4d
#define	KB_P1			0x4f
#define	KB_P2			0x50
#define	KB_P3			0x51
#define	KB_PENTER		(0x1c+0x80)
#define	KB_P0			0x52
#define	KB_PPOINT		0x53


extern volatile UBYTE	AltFlag,
						ControlFlag,
						ShiftFlag;
extern volatile UBYTE	Keys[256],
						LastKey;

//---------------------------------------------------------------

#endif
