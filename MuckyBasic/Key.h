//
// Keyboard handling.
//

#ifndef _KEY_
#define _KEY_


//
// Every key has a byte flag to say whether it is on or off.
//

extern volatile UBYTE KEY_on[256];
extern volatile SLONG KEY_inkey;	// The ASCII value of the last key pressed.
extern volatile SLONG KEY_shift;

//
// KEY shift can have the following bits set.
//

#define KEY_SHIFT 	(1 << 0)
#define KEY_ALT	  	(1 << 1)
#define KEY_CONTROL	(1 << 2)

//
// The scancodes of all the keys...
//

#define	KEY_ESCAPE		0x01
#define	KEY_1			0x02
#define	KEY_2			0x03
#define	KEY_3			0x04
#define	KEY_4			0x05
#define	KEY_5			0x06
#define	KEY_6			0x07
#define	KEY_7			0x08
#define	KEY_8			0x09
#define	KEY_9			0x0a
#define	KEY_0			0x0b
#define	KEY_MINUS		0x0c
#define	KEY_EQUAL		0x0d
#define	KEY_BACKSPACE	0x0e
#define	KEY_TAB			0x0f
#define	KEY_Q			0x010
#define	KEY_W			0x011
#define	KEY_E			0x012
#define	KEY_R			0x013
#define	KEY_T			0x014
#define	KEY_Y			0x015
#define	KEY_U			0x016
#define	KEY_I			0x017
#define	KEY_O			0x018
#define	KEY_P			0x019
#define	KEY_LSBRACKET	0x01a
#define	KEY_RSBRACKET	0x01b
#define	KEY_RETURN		0x01c
#define	KEY_LCONTROL	0x01d
#define	KEY_RCONTROL   (0x01d + 0x80)
#define	KEY_A			0x01e
#define	KEY_S			0x01f
#define	KEY_D			0x020
#define	KEY_F			0x021
#define	KEY_G			0x022
#define	KEY_H			0x023
#define	KEY_J			0x024
#define	KEY_K			0x025
#define	KEY_L			0x026
#define	KEY_COLON		0x027
#define	KEY_QUOTE		0x028
#define	KEY_QUOTE2		0x029
#define	KEY_LSHIFT		0x02a
#define	KEY_HASH		0x02b
#define	KEY_BACKSLASH	0x056
#define	KEY_Z			0x02c
#define	KEY_X			0x02d
#define	KEY_C			0x02e
#define	KEY_V			0x02f
#define	KEY_B			0x030
#define	KEY_N			0x031
#define	KEY_M			0x032
#define	KEY_COMMA		0x033
#define	KEY_POINT		0x034
#define	KEY_SLASH		0x035
#define	KEY_RSHIFT		0x036
#define	KEY_LALT		0x038
#define KEY_RALT       (0x038 + 0x80)
#define	KEY_SPACE		0x039
#define	KEY_CAPS		0x03a
#define	KEY_F1			0x03b
#define	KEY_F2			0x03c
#define	KEY_F3			0x03d
#define	KEY_F4			0x03e
#define	KEY_F5			0x03f
#define	KEY_F6			0x040
#define	KEY_F7			0x041
#define	KEY_F8			0x042
#define	KEY_F9			0x043
#define	KEY_F10			0x044
#define	KEY_F11			0x057
#define	KEY_F12			0x058

#define	KEY_HOME		(0x47 + 0x80)
#define	KEY_UP			(0x48 + 0x80)
#define	KEY_PAGEUP		(0x49 + 0x80)
#define	KEY_LEFT		(0x4b + 0x80)
#define	KEY_RIGHT		(0x4d + 0x80)
#define	KEY_END			(0x4f + 0x80)
#define	KEY_DOWN		(0x50 + 0x80)
#define	KEY_PAGEDOWN	(0x51 + 0x80)
#define	KEY_INSERT		(0x52 + 0x80)
#define	KEY_DELETE		(0x53 + 0x80)

#define	KEY_NUM_LOCK	0x045	
#define	KEY_PMINUS		0x04a		
#define	KEY_PADD		0x04e
#define	KEY_PSLASH		(0x035 + 0x80)
#define	KEY_ASTERISK	0x037	
#define	KEY_PDOT		0x053	
#define	KEY_ENTER		(0x01c + 0x80)	

#define KEY_P7			0x047
#define KEY_P8			0x048
#define KEY_P9			0x049
#define KEY_P4			0x04b
#define KEY_P5			0x04c
#define KEY_P6			0x04d
#define KEY_P1			0x04f
#define KEY_P2			0x050
#define KEY_P3			0x051
#define KEY_P0			0x052


#endif