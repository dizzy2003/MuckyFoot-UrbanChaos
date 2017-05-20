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

#define FONT_LETTER_HEIGHT 21

#define FONT_LOWERCASE		 0
#define FONT_UPPERCASE		 26
#define FONT_NUMBERS		 52
#define FONT_PUNCT_PLING	 62
#define FONT_PUNCT_DQUOTE	 63
#define FONT_PUNCT_POUND	 64
#define FONT_PUNCT_DOLLAR	 65
#define FONT_PUNCT_PERCENT	 66
#define FONT_PUNCT_POWER	 67
#define FONT_PUNCT_AMPERSAND 68
#define FONT_PUNCT_ASTERISK	 69
#define FONT_PUNCT_OPEN		 70
#define FONT_PUNCT_CLOSE	 71
#define FONT_PUNCT_COPEN	 72
#define FONT_PUNCT_CCLOSE	 73
#define FONT_PUNCT_SOPEN	 74
#define FONT_PUNCT_SCLOSE	 75
#define FONT_PUNCT_LT		 76
#define FONT_PUNCT_GT		 77
#define FONT_PUNCT_BSLASH	 78
#define FONT_PUNCT_FSLASH	 79
#define FONT_PUNCT_COLON	 80
#define FONT_PUNCT_SEMICOLON 81
#define FONT_PUNCT_QUOTE	 82
#define FONT_PUNCT_AT		 83
#define FONT_PUNCT_HASH		 84
#define FONT_PUNCT_TILDE	 85
#define FONT_PUNCT_QMARK	 86
#define FONT_PUNCT_MINUS	 87
#define FONT_PUNCT_EQUALS	 88
#define FONT_PUNCT_PLUS		 89
#define FONT_PUNCT_DOT		 90
#define FONT_PUNCT_COMMA	 91
#define FONT_NUM_FOREIGN     66
#define FONT_NUM_LETTERS	 (92 + FONT_NUM_FOREIGN)

FONT_Letter FONT_letter[FONT_NUM_LETTERS];

//
// This is the order the punctuation characters come in.
//

CBYTE FONT_punct[] =
{
	"!\"£$%^&*(){}[]<>\\/:;'@#~?-=+.,"

	"©ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüüýþÿ"
};


//
// The texture and another place where we store the font data
// apart from the texture!
//

OS_Texture *FONT_ot;
TGA_Pixel   FONT_data[256][256];


//
// Where the next character would have been drawn.
//

float FONT_end_x;
float FONT_end_y;



//
// Returns TRUE if it finds pixel data at (x,y)
// 

SLONG FONT_found_data(SLONG x, SLONG y)
{
	SLONG dy;

	SLONG px;
	SLONG py;

	ASSERT(WITHIN(x, 0, 255));

	for (dy = -16; dy <= 4; dy++)
	{
		px = x;
		py = y + dy;

		if (WITHIN(py, 0, 255))
		{
			if (FONT_data[255- py][px].alpha > 32)
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

	FONT_ot = OS_texture_create("font.tga");

	//
	// Load in the font bitmap.
	//

	TGA_Info  ti;

	ti = TGA_load(
			"Textures\\font.tga",
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
		ULONG      colour  = 0xffffffff,
		float      scale   = 1.0F,
		float      shimmer = 0.0F,
		SLONG      italic  = FALSE)
{
	SLONG letter;
	float width;
	float lean;

	FONT_Letter *fl;

	//
	// How much the character leans...
	//

	lean = (italic) ? scale * 0.02F : 0.0F;

	//
	// Space is a special case!
	// 

	if (chr == ' ')
	{
		width = (10.0F / 256.0F) * scale;
	}
	else
	{
		letter = FONT_get_index(chr);

		ASSERT(WITHIN(letter, 0, FONT_NUM_LETTERS - 1));

		fl = &FONT_letter[letter];

		width = fl->uwidth;

		if (shimmer == 0.0F)
		{
			/*

			OS_buffer_add_sprite(
				ob,
				x,
				y,
				x + fl->uwidth * scale,
				y + (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale,
				fl->u,
				1.0F - (fl->v),
				fl->u + fl->uwidth,
				1.0F - (fl->v + (FONT_LETTER_HEIGHT / 256.0F)),
				0.0F,
				colour);
			*/

			OS_buffer_add_sprite_arbitrary(
				ob,
				x + lean, y,
				x + lean + fl->uwidth * scale, y,
				x,
				y + (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale,
				x + fl->uwidth * scale,
				y + (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale,
				fl->u,
				1.0F - (fl->v),
				fl->u + fl->uwidth,
				1.0F - (fl->v),
				fl->u,
				1.0F - (fl->v + (FONT_LETTER_HEIGHT / 256.0F)),
				fl->u + fl->uwidth,
				1.0F - (fl->v + (FONT_LETTER_HEIGHT / 256.0F)),
				0.0F,
				colour);
		}
		else
		{
			SLONG i;

			#define FONT_SHIMMER_SEGS   12
			#define FONT_SHIMMER_DANGLE 0.8F
			#define FONT_SHIMMER_AMOUNT 0.01F

			float dx_last;
			float dx_now;
			float v  = fl->v;
			float dy = (FONT_LETTER_HEIGHT * 1.33F / 256.0F) * scale / FONT_SHIMMER_SEGS;
			float dv = (FONT_LETTER_HEIGHT * 1.00F / 256.0F)         / FONT_SHIMMER_SEGS;
			float angle = OS_ticks() * 0.004F;
			float dlean = lean * (1.0F / FONT_SHIMMER_SEGS);

			//
			// Scale the amount we shimmer by...
			//

			shimmer *= scale;
			shimmer *= FONT_SHIMMER_AMOUNT;

			//
			// Draw in horizontal strips...
			//

			dx_last = sin(angle - FONT_SHIMMER_DANGLE) * shimmer + lean;

			for (i = 0; i < FONT_SHIMMER_SEGS; i++)
			{
				lean  -= dlean;
				dx_now = sin(angle) * shimmer + lean;

				OS_buffer_add_sprite_arbitrary(
					ob,
					x + dx_last, y,      x + fl->uwidth * scale + dx_last, y,
					x + dx_now,  y + dy, x + fl->uwidth * scale + dx_now , y + dy,
					fl->u, 1.0F - v,
					fl->u + fl->uwidth, 1.0F - v,
					fl->u, 1.0F - (v + dv), 
					fl->u + fl->uwidth, 1.0F - (v + dv),
					0.0F,
					colour);

				y      += dy;
				v      += dv;
				dx_last = dx_now;
				angle  += FONT_SHIMMER_DANGLE;
			}
		}
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







void FONT_draw(SLONG flag, float start_x, float start_y, ULONG colour, float scale, SLONG cursor, float shimmer, CBYTE *fmt, ...)
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

	scale *= 0.5F;
	   
	//
	// The buffer we use to hold the sprites.
	//

	OS_Buffer *ob = OS_buffer_new();

	//
	// Make sure the colour component has alpha- otherwise the
	// font will be invisible!
	//

	SLONG alpha;
	
	SATURATE(shimmer, 0.0F, 1.0F);

	alpha   = ftol(255.0F * (1.0F - shimmer));
	colour |= alpha << 24;

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
		if (*ch == '\n')
		{
			x  = start_x;
			y += (FONT_LETTER_HEIGHT + 1.0F) * scale;
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

					OS_buffer_draw(ob, NULL, NULL);
				}
			}

			x += FONT_draw_letter(ob, *ch, x, y, colour, scale, shimmer, flag & FONT_FLAG_ITALIC);
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

			OS_buffer_draw(ob, NULL, NULL);
		}
	}

	OS_buffer_draw(ob, FONT_ot, NULL, OS_DRAW_DOUBLESIDED | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE | OS_DRAW_ALPHABLEND);

	//
	// Where the next character would have been drawn.
	//

	FONT_end_x = x;
	FONT_end_y = y;
}
