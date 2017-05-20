// Pausepsx.cpp
//
// This file replaces the PC pause mode file with a more playstation appropriate one.
// This loses all the Keys[] references and the joystick reads.
//
// This version should also be smaller than the PC version.

#include	"mfstdlib.h"
#include	"game.h"

#include	"ctrller.h"
#include	"interfac.h"

#include	<libcd.h>

#define PAUSED_KEY_START	(1 << 0)
#define PAUSED_KEY_UP		(1 << 1)
#define PAUSED_KEY_DOWN		(1 << 2)
#define PAUSED_KEY_OKAY  	(1 << 3)
#define PAUSED_KEY_CANCEL	(1 << 4)

#ifndef VERSION_DEMO
#define PAUSE_MENU_RESUME	0
#define PAUSE_MENU_RESTART	1
#define PAUSE_MENU_EXIT		2
#define PAUSE_MENU_SIZE		3
#define PAUSE_MENU_YORN		-1
#else
#define PAUSE_MENU_RESUME	0
#define PAUSE_MENU_EXIT		1
#define PAUSE_MENU_SIZE		2
#define PAUSE_MENU_YORN		-1
#endif

extern ControllerPacket	PAD_Input1,PAD_Input2;
extern UBYTE	psx_motor[];

#ifdef VERSION_USA
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Resume Level",
	"Restart Level",
	"Abandon Game"
};
#define PAUSE_STR "PAUSE MENU"
#define SURE_STR "Are you sure?"
#define YORN_STR STR_CROSS" Yes / "STR_TRI" No."
#define CTRL_REMOVED "Controller Removed"
#define CTRL_NOTSUPP "Controller Not Supported"
#endif

#ifdef VERSION_JAPAN
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Resume Level",
	"Restart Level",
	"Abandon Game"
};
#define PAUSE_STR "PAUSE MENU"
#define SURE_STR "Are you sure?"
#define YORN_STR STR_CROSS" Yes / "STR_TRI" No."
#define CTRL_REMOVED "Controller Removed"
#define CTRL_NOTSUPP "Controller Not Supported"
#endif

#ifdef VERSION_FRENCH
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Reprendre niveau",
	"Recommencer niveau",
	"Abandonner la partie"
};
#define PAUSE_STR "PAUSE"
#define SURE_STR "Etes-vous sûr(e) ?"
#define YORN_STR STR_CROSS" Oui / "STR_TRI" Non."
#define CTRL_REMOVED "Manette retirée"
#define CTRL_NOTSUPP "Manette non compatible"
#endif

#ifdef VERSION_GERMAN
// Next line is a total fucking hack, I'm not sure if Korea/Singapore version will be
// Gore enablable or not, so it might be German or French, so both versions are getting
// setup to ignore this section if VERSION_KOREA is defined.
#if !defined(VERSION_KOREA)&&!defined(VERSION_FRENCH)&&!defined(VERSION_DEMO)
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Level wiederaufnehmen",
	"Level neu starten",
	"Spiel verlassen"
};
#define PAUSE_STR "PAUSEN-MENU"
#define SURE_STR "Sind Sie sicher?"
#define YORN_STR STR_CROSS" Ja / "STR_TRI" Nein"
#define CTRL_REMOVED "Controller enfernt"
#define CTRL_NOTSUPP "Controller nicht unterstützt"
#endif
#endif

#ifdef VERSION_SPANISH
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Reanudar nivel",
	"Reiniciar nivel",
	"Abandonar partida"
};
#define PAUSE_STR "MENÚ DE PAUSA"
#define SURE_STR "¿Estás seguro ?"
#define YORN_STR STR_CROSS" Sì / "STR_TRI" No."
#define CTRL_REMOVED "El mando se ha quitado"
#define CTRL_NOTSUPP "Mando no soportado"
#endif

#ifdef VERSION_ITALIAN
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Riprendi livello",
	"Ricomincia livello",
	"Abbandona partita"
};
#define PAUSE_STR "MENU PAUSA"
#define SURE_STR "Sei sicuro?"
#define YORN_STR STR_CROSS" Sì / "STR_TRI" No"
#define CTRL_REMOVED "Controller rimosso"
#define CTRL_NOTSUPP "Controller non supportato"
#endif

#ifdef VERSION_KOREA
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Resume Level",
	"Restart Level",
	"Abandon Game"
};
#define PAUSE_STR "PAUSE MENU"
#define SURE_STR "Are you sure?"
#define YORN_STR STR_CROSS" Yes / "STR_TRI" No."
#define CTRL_REMOVED "Controller Removed"
#define CTRL_NOTSUPP "Controller Not Supported"
#endif

#ifdef VERSION_ENGLISH

#ifndef VERSION_DEMO
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Resume Level",
	"Restart Level",
	"Abandon Game"
};
#else
static CBYTE *pause_menu[PAUSE_MENU_SIZE] =
{
	"Resume Level",
	"Abandon Game"
};
#endif

#define PAUSE_STR "PAUSE MENU"
#define SURE_STR "Are you sure?"
#define YORN_STR STR_CROSS" Yes / "STR_TRI" No."
#define CTRL_REMOVED "Controller Removed"
#define CTRL_NOTSUPP "Controller Not Supported"
#endif

extern void DRAW2D_Box_Page(SLONG x,SLONG y,SLONG ox,SLONG oy,SLONG rgb);
extern void PANEL_draw_stats(Thing *who);
extern SLONG Wadmenu_MuckyTime;

SLONG PAUSE_handler() {
	SLONG i,text_colour,input=0,temp;
	static SLONG lastinput=0;
	static SLONG selected=0;
	static SLONG delay=10;
#ifdef VERSION_DEMO
extern demo_timeout;
	static SLONG demo_timer=20*demo_timeout;
#endif
	SLONG ans = FALSE;

	if (Keys[KB_ESC])
	{
		Keys[KB_ESC]=0;
		GAME_FLAGS ^= GF_PAUSED;
		selected=0;
		delay=10;
		if (GAME_FLAGS & GF_PAUSED)
		{
			MFX_Mute(1);
			CdControl(CdlPause,0,0);
#ifdef VERSION_DEMO
			demo_timer=20*demo_timeout;
#endif
		}
		else
		{
			MFX_Mute(0);
			CdControl(CdlReadS,0,0);
		}
	}

	if (!(GAME_FLAGS & GF_PAUSED)) return FALSE;

	psx_motor[0]=psx_motor[1]=0;

	if (PadKeyIsPressed(&PAD_Input1,PAD_LU))
		input|=PAUSED_KEY_UP;

	if (PadKeyIsPressed(&PAD_Input1,PAD_LD))
		input|=PAUSED_KEY_DOWN;

	if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
	{
		NET_PLAYER(0)->Genus.Player->InputDone|=(INPUT_MASK_PUNCH|INPUT_MASK_ACTION|INPUT_MASK_KICK|INPUT_MASK_JUMP);
		input|=PAUSED_KEY_OKAY;
	}

	if (delay)
	{
		delay--;
	}
	else 
		if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		{
			NET_PLAYER(0)->Genus.Player->InputDone|=(INPUT_MASK_PUNCH|INPUT_MASK_ACTION|INPUT_MASK_KICK|INPUT_MASK_JUMP);
			input|=PAUSED_KEY_CANCEL;
		}

	temp=input;
	input = input & (~lastinput);
	lastinput=temp;

	if (input&PAUSED_KEY_UP) 
	{
		selected--;
		if (selected<0) selected=PAUSE_MENU_SIZE-1;
	}
	if (input&PAUSED_KEY_DOWN) 
	{
		selected++;
		if (selected>=PAUSE_MENU_SIZE) selected=0;
	}

	if (input&PAUSED_KEY_OKAY)	
	{
		switch(selected) 
		{
		case PAUSE_MENU_RESUME:
			GAME_FLAGS ^= GF_PAUSED;
			delay=10;
			MFX_Mute(0);
			CdControl(CdlReadS,0,0);
			break;
#ifndef VERSION_DEMO
		case PAUSE_MENU_RESTART:
			//extern SLONG draw_3d;
			//draw_3d ^= 1;
			GAME_FLAGS &= ~GF_PAUSED;
			GAME_STATE  =  GS_REPLAY;
			ans         = TRUE;
			delay=10;
			MFX_Mute(0);
			break;
#endif
		case PAUSE_MENU_EXIT:
			selected=PAUSE_MENU_YORN;
			break;

		case PAUSE_MENU_YORN:
			GAME_FLAGS &= ~GF_PAUSED;
#ifndef VERSION_DEMO
			GAME_STATE = GS_LEVEL_LOST;
#else
			GAME_STATE = 0;
#endif
			delay=10;
			ans = TRUE;
			Wadmenu_MuckyTime=0;
			MFX_Mute(0);
		}
	}
	if (input&PAUSED_KEY_CANCEL)
	{
		delay=10;
		GAME_FLAGS &= ~GF_PAUSED;
		MFX_Mute(0);
		CdControl(CdlReadS,0,0);
	}

	// Timeout the pause back to the menu in the demo.

#ifdef VERSION_DEMO
	if (demo_timer==0)
		GAME_STATE = 0;
	demo_timer--;
#endif

	SLONG offset;
	SLONG text_size;

extern UBYTE PAD_Type;

	switch(PAD_Type)
	{
	case 0:
		FONT2D_DrawStringCentred(CTRL_REMOVED,DISPLAYCENTRE,160,0x00ff00,256);
		selected=0;
	case 4:
	case 7:
		break;
	default:
		FONT2D_DrawStringCentred(CTRL_NOTSUPP,DISPLAYCENTRE,160,0x00ff00,256);
		selected=0;
		break;
	}

	if (selected!=PAUSE_MENU_YORN)
	{
		FONT2D_DrawStringCentred(PAUSE_STR,DISPLAYCENTRE,80,0xffffff,256);

		for (i = 0; i < PAUSE_MENU_SIZE; i++)
		{
			if (selected == i)
				text_colour =  0xffff00;
			else
				text_colour = 0x00ff00;
	
			text_size   = 256;
	
			FONT2D_DrawStringCentred(
				pause_menu[i],
				DISPLAYCENTRE,
				96 + 18 * i,
				text_colour,
				text_size);
		}
	} 
	else
	{
		FONT2D_DrawStringCentred(pause_menu[PAUSE_MENU_EXIT],DISPLAYCENTRE,80,0xffffff,256);
		FONT2D_DrawStringCentred(SURE_STR,DISPLAYCENTRE,92,0x00ff00,256);
		FONT2D_DrawStringCentred(YORN_STR,DISPLAYCENTRE,116,0x00ff00,256);
	}

	DRAW2D_Box_Page(DISPLAYCENTRE-80,64,DISPLAYCENTRE+79,144,0x0f404040);

	PANEL_draw_stats(NET_PLAYER(0));

	FONT2D_DrawString(STR_CROSS" Select, "STR_TRI" Cancel.",216,216,0xffffff,256);


	return ans;
}




