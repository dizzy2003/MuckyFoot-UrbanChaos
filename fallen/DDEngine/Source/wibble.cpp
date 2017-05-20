#include "game.h"
#include <ddlib.h>


SLONG mul_y1;
SLONG mul_y2;
SLONG mul_g1;
SLONG mul_g2;

SLONG shift1;
SLONG shift2;

void WIBBLE_simple(
		SLONG x1, SLONG y1,
		SLONG x2, SLONG y2,
		UBYTE wibble_y1,
		UBYTE wibble_y2,
		UBYTE wibble_g1,
		UBYTE wibble_g2,
		UBYTE wibble_s1,
		UBYTE wibble_s2)
{
	SLONG y;
	SLONG offset;
	SLONG count;
	SLONG angle1;
	SLONG angle2;

	x1 = x1 * RealDisplayWidth / DisplayWidth;
	x2 = x2 * RealDisplayWidth / DisplayWidth;
	y1 = y1 * RealDisplayHeight / DisplayHeight;
	y2 = y2 * RealDisplayHeight / DisplayHeight;

	if (the_display.CurrMode->GetBPP() == 16)
	{
		UWORD *dest;
		UWORD *src;
		UBYTE *base = &the_display.screen[x1 + x1 + y1 * the_display.screen_pitch];

		for (y = y1; y < y2; y++)
		{
			angle1  = y         * wibble_y1;
			angle2  = y         * wibble_y2;
			angle1 += GAME_TURN * wibble_g1;
			angle2 += GAME_TURN * wibble_g2;

			angle1 &= 2047;
			angle2 &= 2047;

			offset  = SIN(angle1) * wibble_s1 >> 19;
			offset += COS(angle2) * wibble_s2 >> 19;

			if (offset == 0)
			{
				//
				// Easy!
				// 
			}
			else
			if (offset > 0)
			{
				dest = (UWORD *) base;
				src  = (UWORD *) base;
				src += offset;

				count = x2 - x1;

				while(count-- > 0)
				{
					*dest++ = *src++;
				}
			}
			else
			{
				count = x2 - x1;

				dest = (UWORD *) base;
				src  = (UWORD *) base;

				dest += count - 1;
				src  += count - 1;

				src += offset;

				while(count-- > 0)
				{
					*dest-- = *src--;
				}
			}

			base += the_display.screen_pitch;
		}
	}
	else
	{
		// cut-and-paste, but I don't care anymore
		ULONG *dest;
		ULONG *src;
		UBYTE *base = &the_display.screen[x1 * 4 + y1 * the_display.screen_pitch];

		for (y = y1; y < y2; y++)
		{
			angle1  = y         * wibble_y1;
			angle2  = y         * wibble_y2;
			angle1 += GAME_TURN * wibble_g1;
			angle2 += GAME_TURN * wibble_g2;

			angle1 &= 2047;
			angle2 &= 2047;

			offset  = SIN(angle1) * wibble_s1 >> 19;
			offset += COS(angle2) * wibble_s2 >> 19;

			if (offset == 0)
			{
				//
				// Easy!
				// 
			}
			else
			if (offset > 0)
			{
				dest = (ULONG *)base;
				src  = (ULONG *)base;
				src += offset;

				count = x2 - x1;

				while(count-- > 0)
				{
					*dest++ = *src++;
				}
			}
			else
			{
				count = x2 - x1;

				dest = (ULONG *)base;
				src  = (ULONG *)base;

				dest += count - 1;
				src  += count - 1;

				src += offset;

				while(count-- > 0)
				{
					*dest-- = *src--;
				}
			}

			base += the_display.screen_pitch;
		}
	}
}
