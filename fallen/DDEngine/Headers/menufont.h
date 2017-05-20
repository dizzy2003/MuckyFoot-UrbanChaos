/************************************************************
 *
 *   menufont.h
 *   2D proportional-font text writer with poncey afterfx
 *
 */

#ifndef _MENUFONT_H_
#define _MENUFONT_H_

#include "MFStdlib.h"

#define MENUFONT_FUTZING	(1)
#define MENUFONT_HALOED		(2)
#define MENUFONT_GLIMMER	(4)
#define MENUFONT_CENTRED	(8)
#define MENUFONT_FLANGED	(16)
#define MENUFONT_SINED		(32)
#define MENUFONT_ONLY 		(64)
#define MENUFONT_RIGHTALIGN	(128)
#define MENUFONT_HSCALEONLY	(256)
#define MENUFONT_SUPER_YCTR	(512)
#define MENUFONT_SHAKE		(1024)

void	MENUFONT_Load(CBYTE *fn, SLONG page, CBYTE *fontlist);
void	MENUFONT_Page(SLONG page);
void	MENUFONT_Draw(SWORD x, SWORD y, UWORD scale, CBYTE *msg, SLONG rgb, UWORD flags, SWORD max=-1);
void	MENUFONT_Draw_floats(float x, float y, UWORD scale, CBYTE *msg, SLONG rgb, UWORD flags);
void	MENUFONT_Free();
void	MENUFONT_Dimensions(CBYTE *fn, SLONG &x, SLONG &y, SWORD max=-1, SWORD scale=256);
SLONG	MENUFONT_CharFit(CBYTE *fn, SLONG x, UWORD scale=256);
SLONG	MENUFONT_CharWidth(CBYTE fn, UWORD scale=256);
#ifdef TARGET_DC
void MENUFONT_Draw_Selection_Box(SWORD x, SWORD y, UWORD scale, CBYTE *msg, SLONG rgb, UWORD flags, SWORD max=-1);
#endif



struct CharData {
	float x,y,ox,oy;	 // fractional texture coordinates. live with it.
	UBYTE width, height; 
	UBYTE xofs, yofs;    // offsets, hm.
};


extern CharData FontInfo[256];


#ifdef TARGET_DC
// The Yanks call them VMUs, Europeans call them VMs. Madness.
// Set this to TRUE if you're a European.
extern bool bWriteVMInsteadOfVMU;
#endif


// ========================================================
//
// Cool line-fade-in text.
//
// ========================================================

//
// Clears any old fadein data.
//

void MENUFONT_fadein_init(void);

//
// Sets the x pixel fadein position.  Text will only appear to the left of this
// line.
//

void MENUFONT_fadein_line(SLONG x);	// x is in 8-bit fixed point

//
// Draws centred text where a fade 255 is opaque and 0 is transparent.
//

void MENUFONT_fadein_draw(SLONG x, SLONG y, UBYTE fade, CBYTE *msg);




#endif
