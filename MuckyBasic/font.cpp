//
// A font! That's all there is to it.
//

#include "always.h"
#include "font.h"
#include "os.h"
#include "tga.h"


//
// Each of the letters
//

typedef struct
{
	float u;
	float v;
	float uwidth;

} FONT_Letter;

#define FONT_LETTER_HEIGHT 20

#define FONT_LOWERCASE		   0
#define FONT_UPPERCASE		  26
#define FONT_NUMBERS		  52
#define FONT_PUNCT_PLING	  62
#define FONT_PUNCT_DQUOTE	  63
#define FONT_PUNCT_POUND	  64
#define FONT_PUNCT_DOLLAR	  65
#define FONT_PUNCT_PERCENT	  66
#define FONT_PUNCT_POWER	  67
#define FONT_PUNCT_AMPERSAND  68
#define FONT_PUNCT_ASTERISK	  69
#define FONT_PUNCT_OPEN		  70
#define FONT_PUNCT_CLOSE	  71
#define FONT_PUNCT_COPEN	  72
#define FONT_PUNCT_CCLOSE	  73
#define FONT_PUNCT_SOPEN	  74
#define FONT_PUNCT_SCLOSE	  75
#define FONT_PUNCT_LT		  76
#define FONT_PUNCT_GT		  77
#define FONT_PUNCT_BSLASH	  78
#define FONT_PUNCT_FSLASH	  79
#define FONT_PUNCT_SEMICOLON  80
#define FONT_PUNCT_COLON	  81
#define FONT_PUNCT_QUOTE	  82
#define FONT_PUNCT_AT		  83
#define FONT_PUNCT_HASH		  84
#define FONT_PUNCT_TILDE	  85
#define FONT_PUNCT_QMARK	  86
#define FONT_PUNCT_MINUS	  87
#define FONT_PUNCT_EQUALS	  88
#define FONT_PUNCT_PLUS		  89
#define FONT_PUNCT_DOT		  90
#define FONT_PUNCT_COMMA	  91
#define FONT_PUNCT_UNDERSCORE 92
#define FONT_NUM_FOREIGN      66
#define FONT_NUM_LETTERS	  (93 + FONT_NUM_FOREIGN)

FONT_Letter FONT_letter[FONT_NUM_LETTERS];

//
// This is the order the punctuation characters come in.
//

CBYTE FONT_punct[] =
{
	"!\"£$%^&*(){}[]<>\\/;:'@#~?-=+.,_"

	"©ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüüýþÿ"
};


//
// The texture and another place where we store the font data
// apart from the texture!
//

OS_Texture *FONT_ot;
TGA_Pixel   FONT_data[256][256];



//
// Returns TRUE if it finds pixel data at (x,y)
// 

SLONG FONT_found_data(SLONG x, SLONG y)
{
	SLONG dy;

	SLONG px;
	SLONG py;

	ASSERT(WITHIN(x, 0, 255));

	for (dy = -15; dy <= 4; dy++)
	{
		px = x;
		py = y + dy;

		if (WITHIN(py, 0, 255))
		{
			if (FONT_data[py][px].alpha)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}



void FONT_init()
{
	SLONG i;
	SLONG y;
	SLONG x;
	SLONG line;

	FONT_Letter *fl;

	//
	// Load the texture.
	//

	FONT_ot = OS_texture_create("font_system.tga");

	//
	// Load in the font bitmap.
	//

	TGA_Info  ti;

	ti = TGA_load(
			"Textures\\System\\Fonts\\font_system.tga",
			256,
			256,
		   &FONT_data[0][0]);

	ASSERT(ti.valid);
	ASSERT(ti.width  == 256);
	ASSERT(ti.height == 256);

	//
	// Work out the position of each of the letters.
	//

	x    = 0;
	y    = 19;
	line = 0;

	for (i = 0; i < FONT_NUM_LETTERS; i++)
	{
		fl = &FONT_letter[i];

		//
		// Look for the start of the letter.
		//

		while(!FONT_found_data(x,y))
		{
			x += 1;

			if (x >= 256)
			{
				x     = 0;
				line += 1;
				y    += 22;

				if (y > 256)
				{
					return;
				}
			}
		}

		fl->u = float(x);
		fl->v = float(y);

		//
		// Look for the end of the letter.
		//

		x += 3;

		while(FONT_found_data(x,y))
		{
			x += 1;
		}

		fl->uwidth = (x - fl->u) * (1.0F / 256.0F);

		//
		// Convert the (u,v)s
		//

		fl->u *= 1.0F / 256.0F;
		fl->v *= 1.0F / 256.0F;

		fl->v -= 16.0F / 256.0F;
	}
}


//
// Returns the index of the given character
// 

SLONG FONT_get_index(CBYTE chr)
{
	SLONG letter;

	//
	// Find our letter index.
	//

	if (WITHIN(chr, 'a', 'z'))
	{
		letter = FONT_LOWERCASE + chr - 'a';
	}
	else
	if (WITHIN(chr, 'A', 'Z'))
	{
		letter = FONT_UPPERCASE + chr - 'A';
	}
	else
	if (WITHIN(chr, '0', '9'))
	{
		letter = FONT_NUMBERS + chr - '0';
	}
	else
	{
		//
		// Look for the punctuation letter.
		//

		letter = FONT_PUNCT_PLING;

		for (CBYTE *ch = FONT_punct; *ch && *ch != chr; ch++, letter++);
	}

	if (!WITHIN(letter, 0, FONT_NUM_LETTERS - 1))
	{
		letter = FONT_PUNCT_QMARK;
	}

	return letter;
}


SLONG FONT_char_is_valid(CBYTE ch)
{
	if (FONT_get_index(ch) == FONT_PUNCT_QMARK && ch != '?')
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}



float FONT_get_letter_width(CBYTE chr)
{
	SLONG letter;

	if (chr == ' ')
	{
		return 8.0F / 256.0F;
	}

	letter = FONT_get_index(chr);

	ASSERT(WITHIN(letter, 0, FONT_NUM_LETTERS - 1));

	return FONT_letter[letter].uwidth + (1.0F / 256.0F);
}



float FONT_draw_letter(
		OS_Buffer *ob,
		CBYTE      chr,
		float      x,
		float      y,
		ULONG      colour = 0xffffffff,
		ULONG      flag   = 0,
		float      scale  = 1.0F)
{
	SLONG letter;
	float width;

	FONT_Letter *fl;
	
	if (flag & FONT_FLAG_DROP_SHADOW)
	{
		FONT_draw_letter(
			ob,
			chr,
			x + 0.008F * scale,
			y + 0.008F * scale,
			(~colour) | (colour & 0xff000000),
			flag & ~FONT_FLAG_DROP_SHADOW,
			scale);
	}

	//
	// Space is a special case!
	// 

	if (chr == ' ')
	{
		width = (8.0F / 256.0F) * scale;
	}
	else
	{
		letter = FONT_get_index(chr);

		ASSERT(WITHIN(letter, 0, FONT_NUM_LETTERS - 1));

		fl = &FONT_letter[letter];

		width = fl->uwidth;

		OS_buffer_add_sprite(
			ob,
			x,
			y,
			x + fl->uwidth * scale,
			y + (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale,
			fl->u,
			fl->v,
			fl->u + fl->uwidth,
			fl->v + (FONT_LETTER_HEIGHT / 256.0F),
			0.0F,
			colour);
	}

	return (width + 1.0F / 256.0F) * scale;
}

//
// Returns the width of the given string.
//

float FONT_get_width(CBYTE *str, float scale)
{
	float ans = 0.0F;

	for (CBYTE *ch = str; *ch; ch++)
	{
		ans += FONT_get_letter_width(*ch) * scale;
	}

	return ans;
}





void FONT_draw(float start_x, float start_y, ULONG colour, ULONG flag, float scale, SLONG cursor, CBYTE *fmt, ...)
{
	CBYTE   message[4096];
	va_list	ap;

	if (fmt == NULL)
	{
		sprintf(message, "<NULL>");
	}
	else
	{
		va_start(ap, fmt);
		vsprintf(message, fmt, ap);
		va_end  (ap);
	}
	
	//
	// So that a scale of 1.0F is normal size.
	//

	scale *= 0.26F;
	 
	//
	// The buffer we use to hold the sprites.
	//

	OS_Buffer *ob = OS_buffer_new();

	//
	// Make sure the colour component has alpha- otherwise the
	// font will be invisible!
	//

	colour |= 0xff000000;

	float x = start_x;
	float y = start_y;

	if (flag & FONT_FLAG_JUSTIFY_CENTRE)
	{
		x -= FONT_get_width(message, scale) * 0.5F;
	}
	else
	if (flag & FONT_FLAG_JUSTIFY_RIGHT)
	{
		x -= FONT_get_width(message, scale);
	}

	CBYTE *ch = message;

	while(*ch)
	{
		if (iscntrl(*ch))
		{
			if (*ch == '\n')
			{
				x  = start_x;
				y += (FONT_LETTER_HEIGHT * 1.4F / 256.0F) * scale;
			}
		}
		else
		{
			if (cursor-- == 0)
			{
				//
				// Draw a cursor here.
				//

				{
					OS_Buffer *ob = OS_buffer_new();

					OS_buffer_add_sprite(
						ob,
						x, y, x + 0.01F * scale, y + (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale,
						0.0F, 0.0F,
						1.0F, 1.0F,
						0.0F,
						0xeeeeeff);

					OS_buffer_draw(ob, NULL, NULL, OS_DRAW_DOUBLESIDED | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE);
				}
			}

			x += FONT_draw_letter(ob, *ch, x, y, colour, flag, scale);
		}

		ch += 1;
	}

	if (cursor-- == 0)
	{
		//
		// Draw a cursor here.
		//

		{
			OS_Buffer *ob = OS_buffer_new();

			OS_buffer_add_sprite(
				ob,
				x, y, x + 0.01F * scale, y + (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale,
				0.0F, 0.0F,
				1.0F, 1.0F,
				0.0F,
				0xeeeeeff);

			OS_buffer_draw(ob, NULL, NULL, OS_DRAW_DOUBLESIDED | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE);
		}
	}

	OS_buffer_draw(ob, FONT_ot, NULL, OS_DRAW_DOUBLESIDED | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE | OS_DRAW_ALPHABLEND);
}
