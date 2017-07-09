/************************************************************
 *
 *   font2d.cpp
 *   2D text writer
 *
 */

#ifndef TARGET_DC
#include <string>
#endif
#include "font2d.h"
#include "fallen/ddlibrary/headers/tga.h"
#include "noserver.h"
#include <DDLib.h>
#include "truetype.h"
#include "game.h"

#ifdef _DEBUG
#define	PERHAPS		if (!Keys[KB_J])
#else
#define	PERHAPS		if (1)
#endif


//
// Works out the position and width of each letter.
//

//#define FONT2D_LETTER_HEIGHT 21
#define FONT2D_LETTER_HEIGHT 16

#define FONT2D_LOWERCASE		 0
#define FONT2D_UPPERCASE		26
#define FONT2D_NUMBERS			52
#define FONT2D_PUNCT_PLING		62
#define FONT2D_PUNCT_DQUOTE		63
#define FONT2D_PUNCT_POUND		64
#define FONT2D_PUNCT_DOLLAR		65
#define FONT2D_PUNCT_PERCENT	66
#define FONT2D_PUNCT_POWER		67
#define FONT2D_PUNCT_AMPERSAND	68
#define FONT2D_PUNCT_ASTERISK	69
#define FONT2D_PUNCT_OPEN		70
#define FONT2D_PUNCT_CLOSE		71
#define FONT2D_PUNCT_COPEN		72
#define FONT2D_PUNCT_CCLOSE		73
#define FONT2D_PUNCT_SOPEN		74
#define FONT2D_PUNCT_SCLOSE		75
#define FONT2D_PUNCT_LT			76
#define FONT2D_PUNCT_GT			77
#define FONT2D_PUNCT_BSLASH		78
#define FONT2D_PUNCT_FSLASH		79
#define FONT2D_PUNCT_COLON		80
#define FONT2D_PUNCT_SEMICOLON	81
#define FONT2D_PUNCT_QUOTE		82
#define FONT2D_PUNCT_AT			83
#define FONT2D_PUNCT_HASH		84
#define FONT2D_PUNCT_TILDE		85
#define FONT2D_PUNCT_QMARK		86
#define FONT2D_PUNCT_MINUS		87
#define FONT2D_PUNCT_EQUALS		88
#define FONT2D_PUNCT_PLUS		89
#define FONT2D_PUNCT_DOT		90
#define FONT2D_PUNCT_COMMA		91
#define FONT2D_GERMAN_CHARS     10
#define FONT2D_FRENCH_CHARS		14
#define FONT2D_SPANISH_CHARS	11
#define FONT2D_ITALIAN_CHARS	12
#define FONT2D_NUM_LETTERS		(92 + FONT2D_GERMAN_CHARS + FONT2D_FRENCH_CHARS + FONT2D_SPANISH_CHARS + FONT2D_ITALIAN_CHARS)

FONT2D_Letter FONT2D_letter[FONT2D_NUM_LETTERS];

//
// This is the order the punctuation characters come in.
//

CBYTE FONT2D_punct[] =
{
	"!\"�$%^&*(){}[]<>\\/:;'@#_?-=+.,"

	//
	// German characters in decimal and octal!
	//
	// No, Mark, let's do this right.
	// (a) We can type them in directly and this can be physically matched against the bitmap
	// (b) We can type them *all* in, instead of missing a few

	"����������"

	//
	// French characters
	//

	"��������������"

	//
	// Spanish
	//
	// 161,191,216,225,228,233,237,241,243,248,250
	//

	"�����������"

	//
	// Italian
	//
	// 192,200,204,210,217,236,242,249
	//
	//

	"������������"
};





// ARGH! Can't get this sodding type right. Do it with a typedef.
typedef TGA_Pixel MyArrayType[256][256];
MyArrayType *FONT2D_data;

//
// Returns TRUE if it finds pixel data at (x,y)
//

SLONG FONT2D_found_data(SLONG x, SLONG y)
{
	SLONG dy;

	SLONG px;
	SLONG py;

	ASSERT ( FONT2D_data != NULL );

	ASSERT(WITHIN(x, 0, 255));

	for (dy = -14; dy <= 3; dy++)
	{
		px = x;
		py = y + dy;

		if (WITHIN(py, 0, 255))
		{
			if (((*FONT2D_data)[py][px]).alpha)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}



void FONT2D_init(SLONG font_id)
{
	TGA_Info  ti;

#ifdef	NO_SERVER
	#define TEXTURE_EXTRA_DIR "server\\textures\\extras\\"
#else
	#define TEXTURE_EXTRA_DIR "u:\\urbanchaos\\textures\\extras\\"
#endif


	CBYTE fname[256];


	FONT2D_data = (MyArrayType *)MemAlloc ( sizeof ( TGA_Pixel ) * 256 * 256 );
	ASSERT ( FONT2D_data != NULL );


#ifdef TARGET_DC
	sprintf(fname, "%s%s", TEXTURE_EXTRA_DIR, "multifontPC.tga");
#else
	if (SOFTWARE)
	{
		sprintf(fname, "%s%s", TEXTURE_EXTRA_DIR, "multifontPC640.tga");
	}
	else
	{
		sprintf(fname, "%s%s", TEXTURE_EXTRA_DIR, "multifontPC.tga");
	}
#endif


	ti = TGA_load(
			fname,
			256,
			256,
		    &((*FONT2D_data)[0][0]),
		    font_id, FALSE);

	ASSERT(ti.valid);
	ASSERT(ti.width  == 256);
	ASSERT(ti.height == 256);

	SLONG i;
	SLONG y;
	SLONG x;
	SLONG line;

	FONT2D_Letter *fl;

	#define FONT2D_NUM_BASELINES 8

	SLONG baseline[FONT2D_NUM_BASELINES] =
	{
		17, 37, 57, 77, 97, 117, 137, 157
	};


	x    = 0;
	y    = baseline[0];
	line = 0;

	for (i = 0; i < FONT2D_NUM_LETTERS; i++)
	{
		fl = &FONT2D_letter[i];

		//
		// Look for the start of the letter.
		//

		while(!FONT2D_found_data(x,y))
		{
			x += 1;

			if (x >= 256)
			{
				x     = 0;
				line += 1;

				if (!WITHIN(line, 0, FONT2D_NUM_BASELINES - 1))
				{
					return;
				}

				y = baseline[line];
			}
		}

		fl->u = x;
		fl->v = y;

		//
		// Look for the end of the letter.
		//

		x += 1;

		while(FONT2D_found_data(x,y))
		{
			x += 1;
		}

		fl->width = x + 1.0F - fl->u;

		//
		// Convert the (u,v)s
		//

		fl->u *= 1.0F / 256.0F;
		fl->v *= 1.0F / 256.0F;

		fl->v -= 14.0F / 256.0F;
	}
	MemFree ( FONT2D_data );
	FONT2D_data = NULL;
}


//
// Returns the index of the given character
//

SLONG FONT2D_GetIndex(CBYTE chr)
{
	SLONG letter;

	//
	// Remap certain characters first
	//

	if (chr=='�') chr='E';

	//
	// Find our letter index.
	//

	if (WITHIN(chr, 'a', 'z'))
	{
		letter = FONT2D_LOWERCASE + chr - 'a';
	}
	else
	if (WITHIN(chr, 'A', 'Z'))
	{
		letter = FONT2D_UPPERCASE + chr - 'A';
	}
	else
	if (WITHIN(chr, '0', '9'))
	{
		letter = FONT2D_NUMBERS + chr - '0';
	}
	else
	{
		//
		// Look for the punctuation letter.
		//

		letter = FONT2D_PUNCT_PLING;

		for (CBYTE *ch = FONT2D_punct; *ch && *ch != chr; ch++, letter++);
	}

	if (!WITHIN(letter, 0, FONT2D_NUM_LETTERS - 1))
	{
		letter = FONT2D_PUNCT_QMARK;
	}

	return letter;
}



SLONG FONT2D_GetLetterWidth(CBYTE chr)
{
	SLONG letter;

	if ( ( chr == ' ' ) || ( chr == '�' ) )
	{
		// � is a non-wrapping space.
		return 8;
	}

	letter = FONT2D_GetIndex(chr);

	ASSERT(WITHIN(letter, 0, FONT2D_NUM_LETTERS - 1));

	return FONT2D_letter[letter].width + 1;
}



void PANEL_draw_quad(
		float left,
		float top,
		float right,
		float bottom,
		SLONG page,
		ULONG colour = 0xffffffff,
		float u1 = 0.0F,
		float v1 = 0.0F,
		float u2 = 1.0F,
		float v2 = 1.0F);


SLONG FONT2D_DrawLetter(CBYTE chr, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade)
{
	SLONG letter;

	FONT2D_Letter *fl;

	//
	// Space is a special case!
	//

	if (chr == -110)
	{
		//
		// Meant to be a single quote!
		//

		chr = 39;
	}

	// � is a non-wrapping space.
	if (chr == ' ' || chr == '\n' || chr == '\r' || chr == '\t' || chr=='�' )
	{
		return 8 * scale >> 8;
	}

	if (chr == 'y')
	{
		//
		// Wierd eh! The bottom of the 'y' leaks over into the earlier letter.
		//

		x -= 2 * scale >> 8;
	}

	letter = FONT2D_GetIndex(chr);

	ASSERT(WITHIN(letter, 0, FONT2D_NUM_LETTERS - 1));

	fl = &FONT2D_letter[letter];

	SATURATE(fade, 0, 255);

	PANEL_draw_quad(
		x,
		y - 3,
		x + (fl->width * scale >> 8),
		y + (15 * scale >> 8),
		page,
		(rgb & 0xffffff) | ((255 - fade) << 24),
		fl->u,
		fl->v,
		fl->u + float(fl->width) * (1.0F / 256.0F),
		fl->v + 18.0F * (1.0F / 256.0F));

	if (chr == 'y')
	{
		return ((fl->width + 1) * scale >> 8) - (2 * scale >> 8);
	}
	else
	{
		return (fl->width + 1) * scale >> 8;
	}
}


/*



const CBYTE fontlist[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$@*()[]\\/? +-";

FONT2D_DrawLetter(CBYTE chr, ULONG x, ULONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade) {
	UBYTE ndx=strchr(fontlist,chr)-fontlist;
	float u,v;
	SLONG fade2;

	u=(ndx&7); u*=0.125;
	ndx>>=3; v=ndx; v*=0.125;
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

	pp[0].specular=pp[1].specular=pp[2].specular=pp[3].specular=0xff000000;
	if (fade) {
		if (fade<32) {
			fade2=(32-fade)<<3;
			pp[0].colour=pp[1].colour=(rgb & 0x00FFFFFF) | (0xff<<24);
			pp[2].colour=pp[3].colour=(rgb & 0x00FFFFFF) | (fade2<<24);
		} else {
			fade2=((64-fade)<<3)-1;
			pp[0].colour=pp[1].colour=(rgb & 0x00FFFFFF) | (fade2<<24);
			pp[2].colour=pp[3].colour=rgb & 0x00FFFFFF;
		}
	} else {
		pp[0].colour=pp[1].colour=pp[2].colour=pp[3].colour=rgb | (0xff<<24);
	}
	pp[0].u=u;			pp[0].v=v;
	pp[1].u=u+0.125;	pp[1].v=v;
	pp[2].u=u;			pp[2].v=v+0.125;
	pp[3].u=u+0.125;	pp[3].v=v+0.125;
	pp[0].X=x;			pp[0].Y=y;
	pp[1].X=x+scale;	pp[1].Y=y;
	pp[2].X=x;			pp[2].Y=y+scale;
	pp[3].X=x+scale;	pp[3].Y=y+scale;
	pp[0].Z=pp[1].Z=pp[2].Z=pp[3].Z=0.5f;
	POLY_add_quad(quad,page,FALSE,TRUE);
}

*/


SLONG FONT2D_rightmost_x;
SLONG FONT2D_leftmost_x;


#define MAKE_FADE_RGB(RGB, FADE)	((RGB & 0x00FFFFFF) | ((255 - FADE) << 24))

void FONT2D_DrawString(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade)
{
	UBYTE i;

	if (str == NULL)
	{
		str = "Null string";
	}

#ifdef TRUETYPE
	PERHAPS
	{
		SATURATE(fade, 0, 255);
		DrawTextTT(str, x, y, 640, scale, MAKE_FADE_RGB(rgb, fade), LeftJustify);
		return;
	}
#endif

	FONT2D_rightmost_x = x;

	for (i=0;i<strlen(str);i++)
	{
		if (str[i] == '\t')
		{
			x &= ~127;
			x +=  128;
		}
		else
		{
			x += FONT2D_DrawLetter(str[i],x,y,rgb,scale,page,fade);
		}

		if (x > FONT2D_rightmost_x)
		{
			FONT2D_rightmost_x = x;
		}
	}
}

void FONT2D_DrawString_NoTrueType(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade)
{
	UBYTE i;

	if (str == NULL)
	{
		str = "Null string";
	}

	for (i=0;i<strlen(str);i++)
	{
		if (str[i] == '\t')
		{
			x &= ~127;
			x +=  128;
		}
		else
		{
			x += FONT2D_DrawLetter(str[i],x,y,rgb,scale,page,fade);
		}
	}
}



#ifdef TARGET_DC
// Done in the sodding header, like it should have been.
#else
// WHAT THE FUCK ARE YOU COCKSUCKERS DOING?!?!?!?!
// JUST CALL THE OTHER FUCKING FUNCTION
// Idiots. I have to work with idiots.
// Fine. Fill your memory up with shit then. See if I care.

SLONG FONT2D_DrawStringWrap(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade)
{
	if (str == NULL)
	{
		str = "Null string";
	}

#ifdef TRUETYPE
	PERHAPS
	{
		int	width;
		SATURATE(fade, 0, 255);
		return DrawTextTT(str, x, y, 600, scale, MAKE_FADE_RGB(rgb, fade), LeftJustify, &FONT2D_rightmost_x);
	}
#endif

	SLONG i;
	SLONG xbase = x;
	SLONG xlook;
	SLONG len = strlen(str);

	CBYTE *ch;

	FONT2D_rightmost_x = x + 8;

	for (i = 0; i < len; i++)
	{
		if (str[i] == ' ')
		{
			x += FONT2D_GetLetterWidth(' ');

			//
			// Should we go onto the next line?
			//

			ch    = &str[i + 1];
			xlook = x;

			while(*ch && *ch != ' ')
			{
				xlook += FONT2D_GetLetterWidth(*ch);
				ch    += 1;
			}

			if (xlook >= 600)
			{
				//
				// The next space or EOL is beyond the end of the screen.
				// Start a new line.
				//

				x  = xbase;
				y += FONT2D_LETTER_HEIGHT - 1;
			}
		}
		else
		{
			x += FONT2D_DrawLetter(str[i],x,y,rgb,scale,page,fade);

			if (x > FONT2D_rightmost_x)
			{
				FONT2D_rightmost_x = x;
			}
		}
	}

	return y;
}
#endif


SLONG FONT2D_DrawStringWrapTo(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade, SWORD span)
{
	if (str == NULL)
	{
		str = "Null string";
	}

#ifdef TRUETYPE
	PERHAPS
	{
		SATURATE(fade, 0, 255);
		return DrawTextTT(str, x, y, span, scale, MAKE_FADE_RGB(rgb, fade), LeftJustify);
	}
#endif

	SLONG i;
	SLONG xbase = x;
	SLONG xlook;
	SLONG len = strlen(str);

	CBYTE *ch;

	FONT2D_rightmost_x = x + 8;

	for (i = 0; i < len; i++)
	{
		if (str[i] == ' ')
		{
			x += FONT2D_GetLetterWidth(' ');

			//
			// Should we go onto the next line?
			//

			ch    = &str[i + 1];
			xlook = x;

			while(*ch && *ch != ' ')
			{
				xlook += FONT2D_GetLetterWidth(*ch);
				ch    += 1;
			}

			if (xlook >= span)
			{
				//
				// The next space or EOL is beyond the end of the screen.
				// Start a new line.
				//

				x  = xbase;
				y += FONT2D_LETTER_HEIGHT - 1;
			}
		}
		else
		{
			if (str[i]==13) {
				x=xbase; y+=FONT2D_LETTER_HEIGHT - 1;
			} else {
				x += FONT2D_DrawLetter(str[i],x,y,rgb,scale,page,fade);
			}

			if (x > FONT2D_rightmost_x)
			{
				FONT2D_rightmost_x = x;
			}
		}
	}

	return y;
}


SLONG FONT2D_DrawStringRightJustify(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade, bool bDontDraw )
{
	if (str == NULL)
	{
		str = "Null string";
	}

#ifdef TRUETYPE
	PERHAPS
	{
		SATURATE(fade, 0, 255);
		return DrawTextTT(str, 10, y, x, scale, MAKE_FADE_RGB(rgb, fade), RightJustify);
	}
#endif


	//str = "Jeez, Miles. I'm a damn ROOKIE an' even I gotta laugh at THAT!";


	SLONG i;
	SLONG drawn_upto;
	SLONG xbase = x - ((scale >> 4) - 7);
	SLONG xlook;
	SLONG len = strlen(str);

	CBYTE *ch;
	CBYTE  backup;

	drawn_upto = 0;
	x          = xbase;

	FONT2D_leftmost_x = x;

	for (i = 0; i <= len; i++)
	{
		if (str[i] == ' ' || str[i] == '\000')
		{
			if (str[i] == '\000')
			{
				//
				// Draw the last line.
				//

				xlook = -INFINITY;
			}
			else
			{
				//
				// Should we draw this line and go onto the next line?
				//

				ch    = &str[i + 1];
				xlook = x;

				while(*ch && *ch != ' ')
				{
					xlook -= FONT2D_GetLetterWidth(*ch);
					ch    += 1;
				}
			}

			if (xlook < 10)
			{
				//
				// Draw upto this space.
				//

				backup = str[i];
				str[i] = '\000';

				if ( !bDontDraw )
				{
					FONT2D_DrawString(
					   &str[drawn_upto],
						x,
						y,
						rgb,
						scale,
						page,
						fade);
				}

				str[i] = backup;

				drawn_upto = i + 1;
				y         += FONT2D_LETTER_HEIGHT - 1;
				x          = xbase + FONT2D_GetLetterWidth(str[i]);
			}
		}

		x -= FONT2D_GetLetterWidth(str[i]);

		if (x < FONT2D_leftmost_x)
		{
			FONT2D_leftmost_x = x;
		}

	}

	return y;
}


SLONG FONT2D_DrawStringRightJustifyNoWrap(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade)
{
	if (str == NULL)
	{
		str = "Null string";
	}

	CBYTE *ch;

	for (ch = str; *ch; ch++)
	{
		x -= FONT2D_GetLetterWidth(*ch);
	}

	FONT2D_DrawString(str, x, y, rgb, scale, page, fade);

	return 0;
}


//POLY_PAGE_FONT2D
void FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y, ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade)
{
	if (str == NULL)
	{
		str = "Null string";
	}

	SLONG screen_size;
	SLONG len;
	SLONG str_width,str_height;

	POLY_Point  mid;



	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &mid);

	if (mid.IsValid())
	{
		CBYTE *ch;

		float x = mid.X;
		float y = mid.Y;

		float width;
		SLONG scale = SLONG(mid.Z * 6 * 256);

		SLONG letter;

		//
		// Work out the length of the string.
		//

		len = 0;

		for (ch = str; *ch; ch++)
		{
			len += FONT2D_GetLetterWidth(*ch) * scale >> 8;
		}

		x -= len / 2;

		FONT2D_DrawString_NoTrueType(
			str,
			x,y,
			0xFFFFFF,
			scale,
			POLY_PAGE_FONT2D,
			0);

		/*

		for (ch = str; *ch; ch++)
		{
			//
			// Draw each letter in turn.
			//

			if (*ch == ' ')
			{
				width = 8.0F;
			}
			else
			{
				SLONG letter;

				FONT2D_Letter *fl;

				letter = FONT2D_GetIndex(*ch);

				ASSERT(WITHIN(letter, 0, FONT2D_NUM_LETTERS - 1));

				fl = &FONT2D_letter[letter];

				SATURATE(fade, 0, 255);

				PANEL_draw_quad(
					x,
					y,
					x + float(fl->width) * scale,
					y + float(FONT2D_LETTER_HEIGHT) * scale,
					POLY_PAGE_FONT2D,
					(rgb & 0xffffff) | ((255 - fade) << 24),
					fl->u,
					fl->v,
					fl->u + float(fl->width) * (1.0F / 256.0F),
					fl->v + float(FONT2D_LETTER_HEIGHT) * (1.0F / 256.0F));

				width = fl->width + 1;
			}

			x += width * scale;
		}

		*/

		/*




		len=strlen(str);
		screen_size = (SLONG)(POLY_world_length_to_screen(text_size) * mid.Z);
		str_width=(screen_size*len)>>1;
		str_height=(screen_size)>>1;

		if (mid.X + str_width < 0 ||
			mid.X - str_width > POLY_screen_width ||
			mid.Y + str_height < 0 ||
			mid.Y - str_height > POLY_screen_height)
		{
			//
			// Off screen.
			//
		}
		else
		{
			FONT2D_DrawString(str, ((ULONG)mid.X)-str_width,((ULONG)mid.Y)-str_height , rgb, screen_size, POLY_PAGE_FONT2D, fade);

		}

		*/
	}
}


void FONT2D_DrawStringCentred(CBYTE*chr, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade)
{
	SLONG length;
	CBYTE *ch;

#ifdef TRUETYPE
	PERHAPS
	{
		SATURATE(fade, 0, 255);
		if (x < 320)
		{
			DrawTextTT(chr, 0, y, x*2, scale, MAKE_FADE_RGB(rgb, fade), Centred);
		}
		else
		{
			DrawTextTT(chr, x*2 - 640, y, 640, scale, MAKE_FADE_RGB(rgb, fade), Centred);
		}
		return;
	}
#endif

	//
	// Work out the length of the string.
	//

	length = 0;

	for (ch = chr; *ch; ch++)
	{
		length += FONT2D_GetLetterWidth(*ch) * scale >> 8;




	}

	x -= length / 2;

	FONT2D_DrawString(
		chr,
		x, y,
		rgb,
		scale,
		page,
		fade);
}




float PANEL_GetNextDepthBodge ( void );

SLONG DST_offset1;
SLONG DST_offset2;



void FONT2D_DrawStrikethrough(SLONG x1, SLONG x2, SLONG y, ULONG rgb, SLONG scale, SLONG page, SLONG fade, bool bUseLastOffset)
{
	FONT2D_Letter *fl;

	SLONG letter = FONT2D_GetIndex('-');

	fl = &FONT2D_letter[letter];

	SLONG offset;

	//
	// Work out the 'random' offset.
	//

	SLONG offset1;
	SLONG offset2;

	if ( bUseLastOffset )
	{
		offset1 = DST_offset1;
		offset2 = DST_offset2;
	}
	else
	{
		SLONG rx = x1 + x2;
		SLONG ry = y;

#if 0
		if (rgb == 0)
		{
			rx -= 2;
			ry -= 2;
		}
#endif

		offset1   = (ry / 5);
		offset1  %= 9;

		offset2   = offset1 * (ry / 20);
		offset1 >>= 3;
		offset2  %= 11;

		DST_offset1 = offset1;
		DST_offset2 = offset2;
	}

	{
		POLY_Point  pp  [4];
		POLY_Point *quad[4];

		float fWDepthBodge = PANEL_GetNextDepthBodge();
		float fZDepthBodge = 1.0f - fWDepthBodge;

		pp[0].X        = x1;
		pp[0].Y        = y - 3 + offset1 - 1;
		pp[0].z        = fZDepthBodge;
		pp[0].Z        = fWDepthBodge;
		pp[0].u        = fl->u;
		pp[0].v        = fl->v;
		pp[0].colour   = rgb | ((255 - fade) << 24);
		pp[0].specular = 0xff000000;

		pp[1].X        = x2;
		pp[1].Y        = y - 3 + offset2 - 3;
		pp[1].z        = fZDepthBodge;
		pp[1].Z        = fWDepthBodge;
		pp[1].u        = fl->u + float(fl->width) * (1.0F / 256.0F);
		pp[1].v        = fl->v;
		pp[1].colour   = rgb | ((255 - fade) << 24);
		pp[1].specular = 0xff000000;

		pp[2].X        = x1;
		pp[2].Y        = y + (15 * scale >> 8) + offset1 - 1;
		pp[2].z        = fZDepthBodge;
		pp[2].Z        = fWDepthBodge;
		pp[2].u        = fl->u;
		pp[2].v        = fl->v + 18.0F * (1.0F / 256.0F);
		pp[2].colour   = rgb | ((255 - fade) << 24);
		pp[2].specular = 0xff000000;

		pp[3].X        = x2;
		pp[3].Y        = y + (15 * scale >> 8) + offset2 - 3;
		pp[3].z        = fZDepthBodge;
		pp[3].Z        = fWDepthBodge;
		pp[3].u        = fl->u + float(fl->width) * (1.0F / 256.0F);
		pp[3].v        = fl->v + 18.0F * (1.0F / 256.0F);
		pp[3].colour   = rgb | ((255 - fade) << 24);
		pp[3].specular = 0xff000000;

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		POLY_add_quad(quad, page, FALSE, TRUE);
	}

}





