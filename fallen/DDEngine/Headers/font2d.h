/************************************************************
 *
 *   font2d.h
 *   2D text writer
 *
 */

#ifndef _FONT_2D_H_
#define _FONT_2D_H_

#include "MFStdLib.h"
#include "poly.h"



void FONT2D_init(SLONG font_id);




typedef struct
{
	float u;
	float v;
	SLONG width;	// Width is in pixels. Divide by 256.0F to get width in texture coords.

} FONT2D_Letter;



//
// Draws the letter and returns the width of the letter in pixels.
// 

SLONG FONT2D_DrawLetter(CBYTE chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);

//
// Returns the width of the given letter in pixels.
// 

SLONG FONT2D_GetLetterWidth(CBYTE chr);


//
// Draws the string.
// 

void FONT2D_DrawString(CBYTE*chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);

//
// Draws text that won't go off the screen. Returns the y coordinate of the line it finished on.
// Sets FONT2D_rightmost_x to be the the coordinate after the rightmost character it drew.
//

extern SLONG FONT2D_rightmost_x;
extern SLONG FONT2D_leftmost_x;	// For right justify!

SLONG FONT2D_DrawStringWrapTo(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade, SWORD span);

#ifdef TARGET_DC
// Um.... guys.
inline SLONG FONT2D_DrawStringWrap(CBYTE*chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0)
{
	return FONT2D_DrawStringWrapTo ( chr, x, y, rgb, scale, page, fade, 600 );
}
#else
SLONG FONT2D_DrawStringWrap(CBYTE*chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);
#endif

//
// The text is right-justified and it wraps. The function returns the y coordinate of the
// line it finished on.
//

SLONG FONT2D_DrawStringRightJustify(CBYTE*chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0, bool bDontDraw=FALSE);

SLONG FONT2D_DrawStringRightJustifyNoWrap(CBYTE*chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);



//
// Draws a centre-justified string.
//

void FONT2D_DrawStringCentred(CBYTE*chr, SLONG x, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);



//
// Draws a strikethrough.
//

//void FONT2D_DrawStrikethrough(SLONG x1, SLONG x2, SLONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SLONG fade=0);
void FONT2D_DrawStrikethrough(SLONG x1, SLONG x2, SLONG y, ULONG rgb, SLONG scale, SLONG page, SLONG fade, bool bUseLastOffset=FALSE);



#endif

