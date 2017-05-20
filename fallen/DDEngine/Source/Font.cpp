//
// Cheapo font stuff...
//

#include <MFStdLib.h>
#include <DDLib.h>
#include "font.h"
#include <math.h>

#define _____ 0x00
#define ____x 0x01
#define ___x_ 0x02
#define ___xx 0x03
#define __x__ 0x04
#define __x_x 0x05
#define __xx_ 0x06
#define __xxx 0x07
#define _x___ 0x08
#define _x__x 0x09
#define _x_x_ 0x0a
#define _x_xx 0x0b
#define _xx__ 0x0c
#define _xx_x 0x0d
#define _xxx_ 0x0e
#define _xxxx 0x0f
#define x____ 0x10
#define x___x 0x11
#define x__x_ 0x12
#define x__xx 0x13
#define x_x__ 0x14
#define x_x_x 0x15
#define x_xx_ 0x16
#define x_xxx 0x17
#define xx___ 0x18
#define xx__x 0x19
#define xx_x_ 0x1a
#define xx_xx 0x1b
#define xxx__ 0x1c
#define xxx_x 0x1d
#define xxxx_ 0x1e
#define xxxxx 0x1f

typedef struct
{
	UBYTE bit[FONT_HEIGHT];
	UBYTE width;

} FONT_Char;

//
// Capital letters.
//

FONT_Char FONT_upper[26] =
{
	{
		{
			_xx__,
			x__x_,
			x__x_,
			x__x_,
			xxxx_,
			x__x_,
			x__x_,
			_____,
			_____
		},
		4
	},

	{
		{
			xxx__,
			x__x_,
			x__x_,
			xxx__,
			x__x_,
			x__x_,
			xxx__,
			_____,
			_____
		},
		4
	},

	{
		{
			_xxx_,
			x____,
			x____,
			x____,
			x____,
			x____,
			_xxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			xxx__,
			_____,
			_____
		},
		4
	},

	{
		{
			xxxx_,
			x____,
			x____,
			xxx__,
			x____,
			x____,
			xxxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			xxxx_,
			x____,
			x____,
			xxx__,
			x____,
			x____,
			x____,
			_____,
			_____
		},
		4
	},

	{
		{
			_xxx_,
			x____,
			x____,
			x____,
			x_xx_,
			x__x_,
			_xxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			x__x_,
			x__x_,
			x__x_,
			xxxx_,
			x__x_,
			x__x_,
			x__x_,
			_____,
			_____
		},
		4
	},

	{
		{
			xxx__,
			_x___,
			_x___,
			_x___,
			_x___,
			_x___,
			xxx__,
			_____,
			_____
		},
		3
	},

	{
		{
			xxxx_,
			___x_,
			___x_,
			___x_,
			___x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			x____,
			x__x_,
			x_x__,
			xx___,
			xx___,
			x_x__,
			x__x_,
			_____,
			_____
		},
		4
	},

	{
		{
			x____,
			x____,
			x____,
			x____,
			x____,
			x____,
			xxxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			x___x,
			xx_xx,
			x_x_x,
			x___x,
			x___x,
			x___x,
			x___x,
			_____,
			_____
		},
		5
	},

	{
		{
			x___x,
			x___x,
			xx__x,
			x_x_x,
			x__xx,
			x___x,
			x___x,
			_____,
			_____
		},
		5
	},

	{
		{
			_xx__,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			xxx__,
			x____,
			x____,
			_____,
			_____
		},
		4
	},

	{
		{
			_xx__,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			x_xx_,
			_xx__,
			___x_,
			_____
		},
		4
	},

	{
		{
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			xxx__,
			x_x__,
			x__x_,
			_____,
			_____
		},
		4
	},

	{
		{
			_xx__,
			x__x_,
			x____,
			_x___,
			__x__,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			xxx__,
			_x___,
			_x___,
			_x___,
			_x___,
			_x___,
			_x___,
			_____,
			_____
		},
		3
	},

	{
		{
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			_x_x_,
			__xx_,
			_____,
			_____
		},
		4
	},

	{
		{
			x___x,
			x___x,
			x___x,
			x___x,
			x_x_x,
			xx_xx,
			x___x,
			_____,
			_____
		},
		5
	},

	{
		{
			x___x,
			x___x,
			_x_x_,
			__x__,
			_x_x_,
			x___x,
			x___x,
			_____,
			_____
		},
		5
	},

	{
		{
			x__x_,
			x__x_,
			x__x_,
			_x_x_,
			__xx_,
			__x__,
			xx___,
			_____,
			_____
		},
		4
	},

	{
		{
			xxxx_,
			___x_,
			___x_,
			__x__,
			_x___,
			x____,
			xxxx_,
			_____,
			_____
		},
		4
	} 
};

//
// Lowercase
//

FONT_Char FONT_lower[26] =
{
	{
		{
			_____,
			_____,
			_xx__,
			___x_,
			_xxx_,
			x__x_,
			_xxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			x____,
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			xxx__,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			_xx__,
			x____,
			x____,
			x____,
			_xx__,
			_____,
			_____
		},
		3
	},

	{
		{
			_____,
			___x_,
			_xxx_,
			x__x_,
			x__x_,
			x__x_,
			_xxx_,
			_____,
			_____ 
		},
		4
	},

	{
		{
			_____,
			_____,
			_xx__,
			x__x_,
			xxxx_,
			x____,
			_xxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_xx__,
			x____,
			x____,
			xx___,
			x____,
			x____,
			_____,
			_____
		},
		3
	},

	{
		{
			_____,
			_____,
			_xxx_,
			x__x_,
			x__x_,
			x__x_,
			_xxx_,
			___x_,
			_xx__
		},
		4
	},

	{
		{
			_____,
			x____,
			x____,
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_x___,
			_____,
			xx___,
			_x___,
			_x___,
			_x___,
			_____,
			_____
		},
		2
	},

	{
		{
			_____,
			_x___,
			_____,
			xx___,
			_x___,
			_x___,
			_x___,
			_x___,
			x____
		},
		2
	},

	{
		{
			_____,
			x____,
			x____,
			x____,
			x_x__,
			xx___,
			x_x__,
			_____,
			_____
		},
		3
	},

	{
		{
			_____,
			xx___,
			_x___,
			_x___,
			_x___,
			_x___,
			_x___,
			_____,
			_____
		},
		2
	},

	{
		{
			_____,
			_____,
			xx_x_,
			x_x_x,
			x___x,
			x___x,
			x___x,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			_xx__,
			x__x_,
			x__x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			xxx__,
			x__x_,
			x__x_,
			x__x_,
			xxx__,
			x____,
			x____
		},
		4
	},

	{
		{
			_____,
			_____,
			_xxx_,
			x__x_,
			x__x_,
			x__x_,
			_xxx_,
			___xx,
			___x_
		},
		4
	},

	{
		{
			_____,
			_____,
			x_xx_,
			xx___,
			x____,
			x____,
			x____,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			_xxx_,
			x____,
			_xx__,
			___x_,
			xxx__,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			x____,
			x____,
			xxx__,
			x____,
			x____,
			_xx__,
			_____,
			_____
		},
		3
	},

	{
		{
			_____,
			_____,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			_xxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			x__x_,
			x__x_,
			x__x_,
			_x_x_,
			__xx_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			x___x,
			x___x,
			x___x,
			x_x_x,
			_x_x_,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			x___x,
			_x_x_,
			__x__,
			_x_x_,
			x___x,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			x__x_,
			x__x_,
			x__x_,
			x__x_,
			_xxx_,
			___x_,
			_xx__
		},
		4
	},

	{
		{
			_____,
			_____,
			xxxx_,
			___x_,
			__x__,
			_x___,
			xxxx_,
			_____,
			_____
		},
		4
	}
};

//
// The numbers...
//

FONT_Char FONT_number[10] =
{

	{
		{
			_xx__,
			x__x_,
			x_xx_,
			xx_x_,
			x__x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			_x___,
			xx___,
			_x___,
			_x___,
			_x___,
			_x___,
			xxx__,
			_____,
			_____
		},
		3
	},

	{
		{
			_xx__,
			x__x_,
			___x_,
			__x__,
			_x___,
			x____,
			xxxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			_xx__,
			x__x_,
			___x_,
			__x__,
			___x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			__x__,
			_xx__,
			x_x__,
			x_x__,
			xxxx_,
			__x__,
			__x__,
			_____,
			_____
		},
		4
	},

	{
		{
			xxxx_,
			x____,
			x____,
			xxx__,
			___x_,
			___x_,
			xxx__,
			_____,
			_____
		},
		4
	},

	{
		{
			__x__,
			_x___,
			x____,
			xxx__,
			x__x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			xxxx_,
			___x_,
			___x_,
			__x__,
			_x___,
			_x___,
			_x___,
			_____,
			_____
		},
		4
	},

	{
		{
			_xx__,
			x__x_,
			x__x_,
			_xx__,
			x__x_,
			x__x_,
			_xx__,
			_____,
			_____
		},
		4
	},

	{
		{
			_xx__,
			x__x_,
			x__x_,
			_xxx_,
			___x_,
			___x_,
			_xx__,
			_____,
			_____
		},
		4
	} 
};

//
// Various punctuation...
//

#define FONT_PUNCT_DOT 		0
#define FONT_PUNCT_COMMA	1
#define FONT_PUNCT_QMARK	2
#define FONT_PUNCT_PLING	3
#define FONT_PUNCT_QUOTES	4
#define FONT_PUNCT_OPEN		5
#define FONT_PUNCT_CLOSE	6
#define FONT_PUNCT_PLUS		7
#define FONT_PUNCT_MINUS	8
#define FONT_PUNCT_EQUAL	9
#define FONT_PUNCT_HASH		10
#define FONT_PUNCT_PCENT	11
#define FONT_PUNCT_STAR		12
#define FONT_PUNCT_BSLASH	13
#define FONT_PUNCT_FSLASH	14
#define FONT_PUNCT_COLON	15
#define FONT_PUNCT_SCOLON	16
#define FONT_PUNCT_APOST	17
#define FONT_PUNCT_AMPER	18
#define FONT_PUNCT_POUND	19
#define FONT_PUNCT_DOLLAR	20
#define FONT_PUNCT_LT		21
#define FONT_PUNCT_GT		22
#define FONT_PUNCT_AT		23
#define FONT_PUNCT_UNDER	24
#define FONT_PUNCT_NUMBER	25

FONT_Char FONT_punct[FONT_PUNCT_NUMBER] =
{
	{
		{
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			x____,
			_____,
			_____
		},
		1
	},

	{
		{
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			x____,
			x____,
			_____
		},
		2
	},

	{
		{
			_xx__,
			x__x_,  
			___x_,
			_xx__,
			_x___,
			_____,
			_x___,
			_____,
			_____
		},
		4
	},

	{
		{
			x____,
			x____,
			x____,
			x____,
			x____,
			_____,
			x____,
			_____,
			_____
		},
		1
	},

	{
		{
			x_x__,
			x_x__,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____  
		},
		3
	},

	{
		{
			_____,
			_x___,
			x____,
			x____,
			x____,
			x____,
			_x___,
			_____,
			_____
		},
		2
	},

	{
		{
			_____,
			x____,
			_x___,
			_x___,
			_x___,
			_x___,
			x____,
			_____,
			_____
		},
		2
	},

	{
		{
			_____,
			_____,
			__x__,
			__x__,
			xxxxx,
			__x__,
			__x__,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			_____,
			_____,
			xxxxx,
			_____,
			_____,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			_____,
			xxxxx,
			_____,
			xxxxx,
			_____,
			_____,
			_____
		},
		5 
	},

	{
		{
			_____,
			_____,
			_x_x_,
			xxxxx,
			_x_x_,
			xxxxx,
			_x_x_,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			xx__x,
			x__x_,
			__x__,
			_x__x,
			x__xx,
			_____,
			_____
		},
		5
	},

	{
		{
			_____,
			__x__,
			x_x_x,
			_xxx_,
			xxxxx,
			_xxx_,
			x_x_x,
			__x__,
			_____
		},
		5
	},

	{
		{
			x____,
			x____,
			_x___,
			_x___,
			__x__,
			__x__,
			___x_,
			___x_,
			_____
		},
		4
	},

	{
		{
			___x_,
			___x_,
			__x__,
			__x__,
			_x___,
			_x___,
			x____,
			x____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			_____,
			x____,
			_____,
			x____,
			_____,
			_____,
			_____
		},
		1
	},

	{
		{
			_____,
			_____,
			_____,
			x____,
			_____,
			x____,
			x____,
			_____,
			_____
		},
		1
	},

	{
		{
			_____,
			x____,
			x____,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____
		},
		1
	},

	{
		{
			_xx__,
			x__x_,
			x_x__,
			_x___,
			x_x_x,
			x__x_,
			_xx_x,
			_____,
			_____
		},
		5
	},

	{
		{
			_xx__,
			x__x_,
			x____,
			_x___,
			xxx__,
			_x___,
			xxxx_,
			_____,
			_____
		},
		4
	},

	{
		{
			_____,
			__x__,
			_xxx_,
			x_x__,
			_xxx_,
			__x_x,
			_xxx_,
			__x__,
			_____
		},
		5
	},

	{
		{
			_____,
			___x_,
			__x__,
			_x___,
			x____,
			_x___,
			__x__,
			___x_,
			_____
		},
		4
	},

	{
		{
			_____,
			x____,
			_x___,
			__x__,
			___x_,
			__x__,
			_x___,
			x____,
			_____
		},
		4
	},

	{
		{
			_____,
			_____,
			_xxx_,
			x___x,
			x_xxx,
			x_xx_,
			x____,
			_xxxx,
			_____
		},
		5
	},

	{
		{
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			xxxx_,
			_____
		},
		4
	}
};

/* 
	{
		{
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____,
			_____
		},
		4
	},

*/


//
// Draws a character with the given RGB at (x,y).
//

SLONG FONT_draw_coloured_char(
		SLONG sx,
		SLONG sy,
		UBYTE red,
		UBYTE green,
		UBYTE blue,
		CBYTE ch)
{
	SLONG b;
	SLONG x;
	SLONG y;

	FONT_Char *fc;

	if (ch == ' ')
	{
		return 3;
	}
	if (ch >= 'A' && ch <= 'Z')
	{
		fc = &FONT_upper[ch - 'A'];
	}
	else
	if (ch >= 'a' && ch <= 'z')
	{
		fc = &FONT_lower[ch - 'a'];
	}
	else
	if (ch >= '0' && ch <= '9')
	{
		fc = &FONT_number[ch - '0'];
	}
	else
	{
		switch(ch)
		{
			case '.':  fc = &FONT_punct[FONT_PUNCT_DOT 	 ]; break;
			case ',':  fc = &FONT_punct[FONT_PUNCT_COMMA ]; break;
			case '?':  fc = &FONT_punct[FONT_PUNCT_QMARK ]; break;
			case '!':  fc = &FONT_punct[FONT_PUNCT_PLING ]; break;
			case '"':  fc = &FONT_punct[FONT_PUNCT_QUOTES]; break;
			case '(':  fc = &FONT_punct[FONT_PUNCT_OPEN	 ]; break;
			case ')':  fc = &FONT_punct[FONT_PUNCT_CLOSE ]; break;
			case '+':  fc = &FONT_punct[FONT_PUNCT_PLUS	 ]; break;
			case '-':  fc = &FONT_punct[FONT_PUNCT_MINUS ]; break;
			case '=':  fc = &FONT_punct[FONT_PUNCT_EQUAL ]; break;
			case '#':  fc = &FONT_punct[FONT_PUNCT_HASH	 ]; break;
			case '%':  fc = &FONT_punct[FONT_PUNCT_PCENT ]; break;
			case '*':  fc = &FONT_punct[FONT_PUNCT_STAR	 ]; break;
			case '\\': fc = &FONT_punct[FONT_PUNCT_BSLASH]; break;
			case '/':  fc = &FONT_punct[FONT_PUNCT_FSLASH]; break;
			case ':':  fc = &FONT_punct[FONT_PUNCT_COLON ]; break;
			case ';':  fc = &FONT_punct[FONT_PUNCT_SCOLON]; break;
			case '\'': fc = &FONT_punct[FONT_PUNCT_APOST ]; break;
			case '&':  fc = &FONT_punct[FONT_PUNCT_AMPER ]; break;
			case '£':  fc = &FONT_punct[FONT_PUNCT_POUND ]; break;
			case '$':  fc = &FONT_punct[FONT_PUNCT_DOLLAR]; break;
			case '<':  fc = &FONT_punct[FONT_PUNCT_LT    ]; break;
			case '>':  fc = &FONT_punct[FONT_PUNCT_GT    ]; break;
			case '@':  fc = &FONT_punct[FONT_PUNCT_AT    ]; break;
			case '_':  fc = &FONT_punct[FONT_PUNCT_UNDER ]; break;
			
			default:   fc = &FONT_punct[FONT_PUNCT_QMARK ]; break;
		}
	}

	if (sy < -FONT_HEIGHT || sy >= the_display.screen_height ||
		sx < -FONT_WIDTH  || sx >= the_display.screen_width)
	{
		//
		// The character is off-screen.
		//
	}
	else
	{
		for (y = 0; y < FONT_HEIGHT; y++)
		{
			for (b = 0x10, x = 0; x < 5; x++, b >>= 1)
			{
				if (fc->bit[y] & b)
				{
					the_display.PlotPixel(sx + x, sy + y, red, green, blue);
				}
			}
		}
	}

	return fc->width;
}
//
// Tab spacing...
//

#define FONT_TAB 16

SLONG FONT_draw_coloured_text(
		SLONG x,
		SLONG y,
		UBYTE red,
		UBYTE green,
		UBYTE blue,
		CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[FONT_MAX_LENGTH];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// Draw the message!
	//

	CBYTE  *ch;
	SLONG   xstart = x;

	for (ch = message; *ch; ch++)
	{
		if (*ch == '\t')
		{
			x += FONT_TAB;
			x &= ~(FONT_TAB - 1);
		}
		else
		{
			x += FONT_draw_coloured_char(x, y, red, green, blue, *ch) + 1;
		}

	}

	return x - xstart;
}



SLONG FONT_draw(SLONG x, SLONG y, CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[FONT_MAX_LENGTH];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// Draw the message!
	//

	CBYTE  *ch;
	SLONG   xstart = x;

	for (ch = message; *ch; ch++)
	{
		if (*ch == '\t')
		{
			x +=  (FONT_TAB);
			x &= ~(FONT_TAB - 1);
		}
		else
		{
			     FONT_draw_coloured_char(x + 1, y + 1, 255,   0, 0, *ch);
			x += FONT_draw_coloured_char(x + 0, y + 0, 255, 255, 0, *ch) + 1;
		}
	}

	return x - xstart;
}



//
// The buffer.
// 

#define FONT_BUFFER_SIZE (1024 * 8)

CBYTE  FONT_buffer[FONT_BUFFER_SIZE];
CBYTE *FONT_buffer_upto;

typedef struct
{
	SLONG  x;
	SLONG  y;
	UBYTE  r;
	UBYTE  g;
	UBYTE  b;
	UBYTE  s;
	CBYTE *m;
	
} FONT_Message;

#define FONT_MAX_MESSAGES 256

FONT_Message FONT_message[FONT_MAX_MESSAGES];
SLONG        FONT_message_upto;


void FONT_buffer_add(
		SLONG  x,
		SLONG  y,
		UBYTE  r,
		UBYTE  g,
		UBYTE  b,
		UBYTE  s,
		CBYTE *fmt, ...)
{
	FONT_Message *fm;

	//
	// Work out the real message.
	//

	CBYTE   message[FONT_MAX_LENGTH];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// So we dont have to have an init() function.
	// 

	if (FONT_buffer_upto == NULL)
	{
		FONT_buffer_upto = &FONT_buffer[0];
	}

	if (!WITHIN(FONT_message_upto, 0, FONT_MAX_MESSAGES - 1))
	{
		//
		// No more messages.
		//

		return;
	}
	
	//
	// Copy the message to the FONT_buffer.
	// 

	if (FONT_buffer_upto + strlen(message) + 1 > &FONT_buffer[FONT_BUFFER_SIZE])
	{
		//
		// Not enough room in the buffer.
		//

		return;
	}

	strcpy(FONT_buffer_upto, message);

	//
	// Build the message.
	//

	fm = &FONT_message[FONT_message_upto++];	

	fm->x = x;
	fm->y = y;
	fm->r = r;
	fm->g = g;
	fm->b = b;
	fm->s = s;
	fm->m = FONT_buffer_upto;

	FONT_buffer_upto += strlen(message) + 1;
}

void FONT_buffer_draw()
{
	SLONG i;
	SLONG x;
	SLONG y;

	CBYTE *ch;

	FONT_Message *fm;

	if (FONT_message_upto == 0)
	{
		return;
	}

	if (the_display.screen_lock())
	{
		for (i = 0; i < FONT_message_upto; i++)
		{
			fm = &FONT_message[i];

			x = fm->x;
			y = fm->y;

			for (ch = fm->m; *ch; ch++)
			{
				if (*ch == '\t')
				{
					x +=  (FONT_TAB);
					x &= ~(FONT_TAB - 1);
				}
				else
				if (*ch == '\n')
				{
					x  = fm->x;
					y += 10;
				}
				else
				{
					if (fm->s)
					{
							 FONT_draw_coloured_char(x + 1, y + 1, fm->r >> 1, fm->g >> 1, fm->b >> 1, *ch);
						x += FONT_draw_coloured_char(x + 0, y + 0, fm->r,      fm->g,      fm->b,      *ch) + 1;
					}
					else
					{
						x += FONT_draw_coloured_char(x + 0, y + 0, fm->r, fm->g, fm->b, *ch) + 1;
					}
				}
			}
		}

		the_display.screen_unlock();
	}

	FONT_buffer_upto  = &FONT_buffer[0];
	FONT_message_upto =  0;
}


void FONT_draw_speech_bubble_text(
		SLONG x,
		SLONG y,
		UBYTE red,
		UBYTE green,
		UBYTE blue,
		CBYTE *fmt, ...)
{

	//
	// Work out the real message.
	//

	CBYTE   message[FONT_MAX_LENGTH];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// How long is the message.
	//	

	SLONG length = FONT_draw_coloured_text(-100, -100, 0, 0, 0, message);
	SLONG width  = SLONG(sqrt(float(length) * 13.0F));

	//
	// Draw the message pretend to see how many lines it has.
	//

	CBYTE *ch;
	SLONG  cw;
	SLONG  w = 0;
	SLONG  lines = 0;

	for (ch = message; *ch; ch++)
	{
		w += FONT_draw_coloured_char(-100, -100, 0, 0, 0, *ch) + 1;

		if (w > width)
		{
			w      = 0;
			lines += 1;
		}
	}


	//
	// Work out where we should start drawing the text.
	//

	SLONG xstart = x;
	SLONG ystart = y - lines * 10;

	//
	// Draw the message.
	//

	x = xstart;
	y = ystart;

	for (ch = message; *ch; ch++)
	{
		cw = FONT_draw_coloured_char(x, y, red, green, blue, *ch) + 1;

		w += cw;
		x += cw;

		if (w > width)
		{
			w  = 0;
			x  = xstart;
			y += 10;
		}
	}
}
