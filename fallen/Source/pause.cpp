/************************************************************
 *
 *   pause.cpp
 *   Handles pause mode
 * 
 *   Prolly overkill giving it one of these all of it's own
 *   but game.cpp is messy enuff already...
 *
 */


#ifndef TARGET_DC


#include	"mfstdlib.h"
#include	"game.h"
#include	"xlat_str.h"

#ifndef	PSX
#include	"c:\fallen\ddengine\headers\poly.h"
#include    "C:\fallen\DDLibrary\headers\D3DTexture.h"
#include    "C:\fallen\DDLibrary\headers\GDisplay.h"
#include    "c:\fallen\ddengine\headers\font3d.h"
#include    "c:\fallen\ddengine\headers\font2d.h"
#endif

#ifdef TARGET_DC
#include "target.h"
#endif

#ifndef	PSX
extern DIJOYSTATE			the_state;
#endif

#define	AXIS_CENTRE			32768
#define	NOISE_TOLERANCE		1024
#define	AXIS_MIN			(AXIS_CENTRE-NOISE_TOLERANCE)
#define	AXIS_MAX			(AXIS_CENTRE+NOISE_TOLERANCE)

#define NORMAL_COLOUR  0xff9f9f
#define SELECT_COLOUR  0xffffff

#if 0
static Font3D font("data\\font3d\\all\\",0.25);
#endif
static SWORD  selected;

#define PAUSED_KEY_START	(1 << 0)
#define PAUSED_KEY_UP		(1 << 1)
#define PAUSED_KEY_DOWN		(1 << 2)
#define PAUSED_KEY_OKAY  	(1 << 3)

#define PAUSE_MENU_RESUME	0
#define PAUSE_MENU_RESTART	1
#define PAUSE_MENU_EXIT		2
#define PAUSE_MENU_SIZE		3
/*
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Resume Level",
	"Restart Level",
	"Abandon Game"
};
*/


SLONG PAUSE_handler() {
	SLONG i,text_colour,input,temp;
	static SLONG lastinput=0;
	SLONG ans = FALSE;


#ifndef	PSX

	input=0;
extern BOOL ReadInputDevice(void);
	if(ReadInputDevice())
	{
/*
		if (the_state.rgbButtons[8] ||
			the_state.rgbButtons[9]) {
			input |= PAUSED_KEY_START;
		}
*/

		if (the_state.lY > AXIS_MAX) input |= PAUSED_KEY_DOWN;

		if (the_state.lY < AXIS_MIN) 
			input |= PAUSED_KEY_UP;

		for (i = 0; i < 8; i++)	{
			if (the_state.rgbButtons[i]) input |= PAUSED_KEY_OKAY;
		}
	}

#endif

	temp=input;
	input = input & (~lastinput);
	lastinput=temp;

    if (Keys[KB_ESC]) {
		Keys[KB_ESC]=0;
		input|=PAUSED_KEY_START;
	}

	if (input&PAUSED_KEY_START)	{
		GAME_FLAGS ^= GF_PAUSED;
		selected = 0;
	}

	if (!(GAME_FLAGS & GF_PAUSED)) return FALSE;


	if (Keys[KB_UP]) {
		Keys[KB_UP]=0;
		input |= PAUSED_KEY_UP;
	}
	if (Keys[KB_DOWN]) {
		Keys[KB_DOWN]=0;
		input |= PAUSED_KEY_DOWN;
	}
	if (Keys[KB_SPACE]|Keys[KB_ENTER]) {
		Keys[KB_SPACE]=Keys[KB_ENTER]=0;
		input |= PAUSED_KEY_OKAY;
	}

	if (input&PAUSED_KEY_UP) {
		selected--;
		if (selected<0) selected=PAUSE_MENU_SIZE-1;
	}
	if (input&PAUSED_KEY_DOWN) {
		selected++;
		if (selected>=PAUSE_MENU_SIZE) selected=0;
	}

	if (input&PAUSED_KEY_OKAY)	{
		switch(selected) {
		case PAUSE_MENU_RESUME:
			GAME_FLAGS ^= GF_PAUSED;
			break;

		case PAUSE_MENU_RESTART:
			//extern SLONG draw_3d;
			//draw_3d ^= 1;
			GAME_FLAGS &= ~GF_PAUSED;
			GAME_STATE  =  GS_REPLAY;
			ans         = TRUE;
			OutputDebugString("Restart\n");
			break;

		case PAUSE_MENU_EXIT:
			GAME_FLAGS &= ~GF_PAUSED;
			GAME_STATE	=	0;
			ans         = TRUE;
			break;
		}
	}

	#ifndef	PSX

	POLY_frame_init(FALSE,FALSE);

	PANEL_draw_quad(
		320 - 170,
		150 - 20,
		320 + 170,
		150 + 40 * PAUSE_MENU_SIZE + 20,
		POLY_PAGE_ALPHA_OVERLAY,
		0x88000000);

#ifndef TARGET_DC
	POLY_frame_draw(FALSE,TRUE);
	POLY_frame_init(FALSE,FALSE);
#endif

	SLONG offset;
	SLONG text_size;

	for (i = 0; i < PAUSE_MENU_SIZE; i++)
	{
		if (selected == i)
		{
			text_colour =  0xffffff;
			text_size   =  768;
			offset      = -8;
		}
		else
		{
			text_colour = 0x00ff00;
			text_size   = 512;
			offset      = 0;
		}

		text_size   = 512;
		offset      = 0;

		FONT2D_DrawStringCentred(
//			pause_menu[i],
			XLAT_str(X_RESUME_LEVEL+i),
			320,
			150 + 40 * i + offset,
			text_colour,
			text_size);
	}

	POLY_frame_draw(FALSE,TRUE);

	#endif

	return ans;

	/*

	// temporary! replace!

	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);
	
	POLY_frame_init(FALSE, FALSE);

	//font.DrawString(startmenu2[c0+start_menu[menu].StartIndex].Str,200,y,text_colour,2.0+(isthis*0.5f),isthis);
	font.DrawString("PAUSED",320,50,NORMAL_COLOUR,3.0,0);

	for (i=0;i<PAUSE_MENU_SIZE;i++) {
		if (selected==i)
			text_colour = SELECT_COLOUR;
		else
			text_colour = NORMAL_COLOUR;
		font.DrawString(pause_menu[i],320,200+(60*i),text_colour,3.0+(selected==i),(selected==i));
	}

	POLY_frame_draw(FALSE,TRUE);

	*/

}




#endif //#ifndef TARGET_DC



