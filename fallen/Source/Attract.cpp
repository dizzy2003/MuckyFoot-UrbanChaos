// Attract.cpp
// Guy Simmons, 20th November 1997.

#include "Game.h"
#include "cam.h"
#ifndef	PSX
#include "c:\fallen\ddlibrary\headers\ddlib.h"
#include "font2d.h"
#include "poly.h"
#include "panel.h"
#else
#include "c:\fallen\psxeng\headers\psxeng.h"
#include "c:\fallen\psxeng\headers\poly.h"
#include "c:\ps\psx\include\ctrller.h"
extern ControllerPacket PAD_Input1,PAD_Input2;
#endif
#include "startscr.h"
#include "briefing.h"
#define	DEMO
#include "attract.h"
#include "env.h"
#include "overlay.h"
#include "sound.h"
#include "mfx.h"
#include "eway.h"
#include "interfac.h"
#include "xlat_str.h"
#include "frontend.h"
#include "statedef.h"
#include "DCLowLevel.h"
//#include "console.h"

// undef this to get the old menus back...
#define NEW_FRONTEND	1

//---------------------------------------------------------------
#ifndef PSX
CBYTE	demo_text[]	=	"Urban Chaos utilises a ground breaking graphics engine which includes 3D volumetric\n"
						"fog, true wall hugging shadows, atomic matter simulation for real-time physical\n"
						"modelling of dynamic object collisions and so provides the perfect environment \n"
						"for incredible feats of acrobatic skill and total scenery interaction. \n\n"
						"The plot of Urban Chaos is still a closely guarded secret but revolves around end\n"
						"of the millennium chaos as predictions for Armageddon become more than mere \n"
						"fantasy.\n\n"
						"Urban Chaos is being developed by Mucky Foot one of the UK's hottest new \n"
						"development teams and will be published world-wide by Eidos Interactive.";
#endif
//---------------------------------------------------------------

extern	SLONG	stat_killed_thug;
extern	SLONG	stat_killed_innocent;
extern	SLONG	stat_arrested_thug;
extern	SLONG	stat_arrested_innocent;


#define	MAX_PLAYBACKS	3

CBYTE	*playbacks[]	=
{
	"Data\\Game.pkt",
	"Data\\Game2.pkt",
	"Data\\Game3.pkt"
};
SLONG	current_playback	=	0;

//extern ULONG get_hardware_input(UWORD type);

extern CBYTE		*playback_name;
SLONG go_into_game;
UBYTE auto_advance = 0;


//---------------------------------------------------------------

// Attract mode is the default game state, basically this function
// cycles between playing the intro, showing the high score table &
// showing the 'PRESS START' bits.

void	LoadBackImage(UBYTE *image_data);
void	AENG_demo_attract(SLONG x,SLONG y,CBYTE *text);
extern BOOL		text_fudge;
extern	SLONG	do_start_menu(void);

#ifndef	PSX
extern DIJOYSTATE			the_state;
#endif

SLONG	any_button_pressed(void)
{
#ifndef	PSX

	if(ReadInputDevice())
	{
		if (the_state.rgbButtons[0] ||
			the_state.rgbButtons[1] ||
			the_state.rgbButtons[2] ||
			the_state.rgbButtons[3] ||
			the_state.rgbButtons[4] ||
			the_state.rgbButtons[5] ||
			the_state.rgbButtons[6] ||
			the_state.rgbButtons[7] ||
			the_state.rgbButtons[8]	||
			the_state.rgbButtons[9])
		{
//			GAME_STATE &= ~GS_ATTRACT_MODE;
//			GAME_STATE |=  GS_PLAY_GAME;
			return(1);
		}
	}
	return(0);
#endif

}

void	game_attract_mode(void)
{
	float			y;
	UBYTE			*image_mem	=	NULL,
					*image		=	NULL;
#ifndef PSX
	SLONG			height,
					image_size;
	MFFileHandle	image_file;
	SLONG           dont_leave_for_a_while = 25;


	//
	// make sure lighting is ok
	//
#ifndef TARGET_DC
extern	void NIGHT_init();
	NIGHT_init();
#endif



	if (auto_advance) {
		go_into_game = TRUE;
		auto_advance=0;
		GAME_STATE &= ~GS_ATTRACT_MODE;
		GAME_STATE |=  GS_PLAY_GAME;
	} else go_into_game = FALSE;
/*
	image_size	=	640*480*3;
	image_mem	=	(UBYTE*)MemAlloc(image_size);
	if(image_mem)
	{
		image_file	=	FileOpen("Data\\gamelogo.tga");
		if(image_file>0)
		{
			FileSeek(image_file,SEEK_MODE_BEGINNING,18);
			image	=	image_mem+(640*479*3);
			for(height=480;height;height--,image-=(640*3))
			{
				FileRead(image_file,image,640*3);
			}
			FileClose(image_file);
		}
	}
*/
//	InitBackImage("e3title.tga");

	// load our sound fx; for the menu sound's benefit
	// (later, do a cut down version w/ only the menu sounds for speed)
/*	#ifndef USE_A3D
		LoadWaveList("Data\\SFX\\1622\\","Data\\SFX\\Samples.txt");
	#else
		A3DLoadWaveList("Data\\SFX\\1622\\","Data\\SFX\\Samples.txt");
	#endif
*/
	MFX_load_wave_list();


	bool bReinitBecauseOfLanguageChange = FALSE;
reinit_because_of_language_change:


#ifdef TARGET_DC
	// If the softreset combo was pressed, go straight to the title screen.
	if ( g_bDreamcastABXYStartComboPressed )
	{
		g_bDreamcastABXYStartComboPressed = FALSE;
		FRONTEND_init ( TRUE );
	}
	else
	{
		FRONTEND_init ( bReinitBecauseOfLanguageChange );
	}
#else
	FRONTEND_init ( bReinitBecauseOfLanguageChange )
#endif


	bReinitBecauseOfLanguageChange = FALSE;

	//
	// Create a surface from this image.
	//

//	the_display.create_background_surface(image_mem);




#if 1
	LPDIRECT3DDEVICE3 dev = the_display.lp_D3D_Device;
	HRESULT hres;


	D3DVIEWPORT2 vp;
	vp.dwSize = sizeof ( vp );
	vp.dwX = the_display.ViewportRect.x1;
	vp.dwY = the_display.ViewportRect.y1;
	vp.dwWidth = the_display.ViewportRect.x2 - the_display.ViewportRect.x1;
	vp.dwHeight = the_display.ViewportRect.y2 - the_display.ViewportRect.y1;
	vp.dvClipX = (float)vp.dwX;
	vp.dvClipY = (float)vp.dwY;
	vp.dvClipWidth = (float)vp.dwWidth;
	vp.dvClipHeight = (float)vp.dwHeight;
	vp.dvMinZ = 0.0f;
	vp.dvMaxZ = 1.0f;
	hres = the_display.lp_D3D_Viewport->SetViewport2 ( &vp );


	dev->SetRenderState(D3DRENDERSTATE_FILLMODE,			D3DFILL_SOLID);
	dev->SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA,		FALSE);
	dev->SetRenderState(D3DRENDERSTATE_SHADEMODE,			D3DSHADE_GOURAUD);
	dev->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE,	TRUE);
	dev->SetRenderState(D3DRENDERSTATE_DITHERENABLE,		TRUE);
	dev->SetRenderState(D3DRENDERSTATE_SPECULARENABLE,		FALSE);
	dev->SetRenderState(D3DRENDERSTATE_SUBPIXEL,			TRUE);
	dev->SetRenderState(D3DRENDERSTATE_ZENABLE,				TRUE);
	dev->SetRenderState(D3DRENDERSTATE_ZFUNC,				D3DCMP_LESSEQUAL);
	dev->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,		TRUE);
	dev->SetRenderState(D3DRENDERSTATE_CULLMODE,			D3DCULL_NONE);
	dev->SetRenderState(D3DRENDERSTATE_FOGENABLE,			FALSE);
	dev->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,		FALSE);
	dev->SetRenderState(D3DRENDERSTATE_ALPHAFUNC,			D3DCMP_ALWAYS);
	dev->SetRenderState(D3DRENDERSTATE_ANTIALIAS,			D3DANTIALIAS_NONE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP,		D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1,		D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2,		D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP,		D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1,		D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2,		D3DTA_DIFFUSE);
    dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	dev->SetTextureStageState(0, D3DTSS_MINFILTER,		D3DTFN_LINEAR);
	dev->SetTextureStageState(0, D3DTSS_MAGFILTER,		D3DTFG_LINEAR);
	dev->SetTextureStageState(0, D3DTSS_MIPFILTER,		D3DTFP_NONE);
	dev->SetTextureStageState(0, D3DTSS_ADDRESS,		D3DTADDRESS_WRAP);
	//dev->SetTexture ( 0, NULL );
extern LPDIRECT3DTEXTURE2	TEXTURE_get_handle(SLONG page);
extern SLONG TEXTURE_page_water;
	dev->SetTexture ( 0, TEXTURE_get_handle(TEXTURE_page_water) );
	
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	dev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,	FALSE);
	dev->SetRenderState(D3DRENDERSTATE_SRCBLEND,			D3DBLEND_ONE);
	dev->SetRenderState(D3DRENDERSTATE_DESTBLEND,			D3DBLEND_ZERO);
#endif






	y	=	500;
	while(SHELL_ACTIVE&&(GAME_STATE&GS_ATTRACT_MODE))
	{
#ifdef TARGET_DC
		Sleep(1);
#endif
		if (dont_leave_for_a_while > 0)
		{
			dont_leave_for_a_while -= 1;
		}


#ifdef	EDITOR
/*		if(ControlFlag && Keys[KB_E])
		{
			GAME_STATE	=	GS_EDITOR;
		}*/
#endif

#ifdef	DEMO
		if(ControlFlag && Keys[KB_Q])
		{
			GAME_STATE	=	0;
			LastKey	=	0;
		}
/*
		else if(Keys[KB_SPACE] || Keys[KB_ENTER] || go_into_game)
		{
			GAME_STATE	&=	~GS_ATTRACT_MODE;
			GAME_STATE	|=	GS_PLAY_GAME;
			break;
		}
*/


		if (dont_leave_for_a_while == 0)
		{
/*			if(any_button_pressed())
			{
				GAME_STATE &= ~GS_ATTRACT_MODE;
				GAME_STATE |=  GS_PLAY_GAME;
				go_into_game=TRUE;
			}*/
		}

#else
		if(LastKey==KB_ESC || (ControlFlag && Keys[KB_Q]))
		{
			GAME_STATE	=	0;
			LastKey	=	0;
		}
		/*
		else if(Keys[KB_SPACE] || Keys[KB_ENTER])
		{
			GAME_STATE	&=	~GS_ATTRACT_MODE;
			GAME_STATE	|=	GS_PLAY_GAME;
		}
		*/
#endif
/*		if (Keys[KB_N])
		{
			Keys[KB_N] = 0;

			//
			// Go the network configuration screen.
			//

			GAME_STATE = GS_CONFIGURE_NET;
		}*/

		//	Record a game.
extern	BOOL	allow_debug_keys;
		if(allow_debug_keys)
		{
			if(ControlFlag && LastKey==KB_R)
			{
				LastKey	=	0;
				GAME_STATE	&= ~GS_ATTRACT_MODE;
				GAME_STATE	|=	GS_PLAY_GAME;
				go_into_game =  TRUE;

				GAME_STATE	|=	GS_RECORD;
			}

			//	Playback a game.
			if(ControlFlag && LastKey==KB_P)
			{
				LastKey	=	0;
				GAME_STATE	&= ~GS_ATTRACT_MODE;
				GAME_STATE	|=	GS_PLAY_GAME;
				go_into_game =  TRUE;

				GAME_STATE	|=	GS_PLAYBACK;
			}
		}

#ifdef TARGET_DC
		// Always clear the screen. Not a real clear, it just triggers a render.
		AENG_clear_screen();
#else
		if (y < 57.0F)
		{
			AENG_clear_screen();
		}
#endif

 		//LoadBackImage(image_mem);
//		the_display.blit_background_surface();

		text_fudge	=	TRUE;
//		y	-=	0.25f;
//		y--;
		if(Keys[KB_LEFT]||Keys[KB_RIGHT]||Keys[KB_UP]||Keys[KB_DOWN]||Keys[KB_SPACE]||Keys[KB_ENTER]) y=500;



//		AENG_demo_attract(0,y,demo_text);
		{
			SLONG	res;

#ifdef NEW_FRONTEND	
			res=FRONTEND_loop();
#else
			res=do_start_menu();
#endif

//			res=STARTS_PLAYBACK;

			if(res)
			{
				switch(res)
				{
/*
				case	1:
				case	2:
				case	3:
				case	4:
				case	5:
				case	6:
				case	7:
				case	8:
						GAME_STATE	&= ~GS_ATTRACT_MODE;
						GAME_STATE	|=	GS_PLAY_GAME;
						go_into_game =  -res;
						break;
				case	9:
						GAME_STATE	=	0;
						LastKey	=	0;
						break;
*/



					case	STARTS_PSX:
#ifdef EDITOR
void	make_all_wads(void);
						make_all_wads();
#endif
						break;
					case	STARTS_START:
#ifdef OBEY_SCRIPT
						if (BRIEFING_select()>0)
#endif
						{
						GAME_STATE	&= ~GS_ATTRACT_MODE;
						GAME_STATE	|=	GS_PLAY_GAME;
						go_into_game =  TRUE;

#ifdef TARGET_DC
						// Unload all the frontend gubbins.
						FRONTEND_unload();
#endif


						ATTRACT_loadscreen_init();

						// Stop everything.
						stop_all_fx_and_music(TRUE);

						// but play the loading music, coz it's all in memory.
						// (No, should already be played, and stopping then starting it
						// makes it huccup.)
						//DCLL_memstream_play();


extern void	init_joypad_config(void);
						init_joypad_config(); // incase it's been changed in front end
						}
						break;
					case	STARTS_PLAYBACK:
						GAME_STATE	&= ~GS_ATTRACT_MODE;
						GAME_STATE	|=	GS_PLAY_GAME;
						GAME_STATE	|=	GS_PLAYBACK;
						go_into_game =  TRUE;
						break;
					case	STARTS_EDITOR:
						GAME_STATE	=	GS_EDITOR;
						break;
					case	STARTS_MULTI:

						NET_init();
						GAME_STATE=GS_CONFIGURE_NET;

						break;
					case	STARTS_EXIT:
						GAME_STATE	=	0;
						LastKey	=	0;
						break;
					case	STARTS_HOST:
						break;
					case	STARTS_JOIN:
						break;
					case	STARTS_LANGUAGE_CHANGE:
						// A language change has been made -
						// reload all the language stuff and go to the main menu.


#ifdef TARGET_DC
						// The only place this can happen is at the title screen.
						// And after the title screen, we play the intro movie.

						// Free up as much memory as we can - the movie needs it!
extern void FRONTEND_scr_unload_theme ( void );
						FRONTEND_scr_unload_theme();
						stop_all_fx_and_music();
						the_display.RunCutscene( 0, ENV_get_value_number("lang_num", 0, "" ) );
#endif

						// Loading screen for a second.
						ATTRACT_loadscreen_init();


						// This goto makes the compiler very unhappy - it seems to get very confused.
						//goto reinit_because_of_language_change;
						bReinitBecauseOfLanguageChange = TRUE;
						break;

				}
			}
			
		}

		if(y<57.0F)
		{
			AENG_fade_out((57 - SLONG(y)) * 15);
		}


		/*
		if(y<40.0f)
		{
			text_fudge	=	FALSE;
			GAME_STATE	|=	(GS_PLAYBACK|GS_PLAY_GAME);
			playback_name	=	playbacks[current_playback];

			go_into_game = FALSE;	// Go into game could be set by game_loop()!!!
			game_loop();
			GAME_STATE	=	GS_ATTRACT_MODE;
			y	=	490.0f;
			current_playback++;
			if(current_playback>=MAX_PLAYBACKS)
				current_playback	=	0;
			dont_leave_for_a_while = 25;
		}
		*/


#ifndef TARGET_DC
		extern void lock_frame_rate(SLONG fps);

		lock_frame_rate(60); // because 30 is so slow i want to gnaw my own liver out rather than go through the menus
#endif

		if ( ( GAME_STATE & GS_PLAY_GAME ) == 0 )
		{
	 		AENG_flip();
		}

		if ( bReinitBecauseOfLanguageChange )
		{
			// Crappy compiler workaround.
			break;
		}
	}


	if ( bReinitBecauseOfLanguageChange )
	{
		// Crappy compiler workaround.
		goto reinit_because_of_language_change;
	}



	//	Fade to logo.
	if(GAME_STATE&GS_PLAY_GAME)
	{
		if (go_into_game)
		{
#ifdef TARGET_DC
			// Unload all the frontend gubbins.
			FRONTEND_unload();
#endif

/*			AENG_clear_screen();
			AENG_fade_out(255);*/
			ShowBackImage(FALSE);
			AENG_flip();

		}
		else
		{
			SLONG c0;

//			ASSERT(0);
/*
			c0 = (57 - SLONG(y)) * 17;

			SATURATE(c0, 0, 255);

			c0 -= c0 % 15;

			for(;c0<256;c0+=15)
			{
				AENG_clear_screen();
 				LoadBackImage(image_mem); //image_mem not valid
				AENG_demo_attract(0,(SLONG)y,demo_text);
				AENG_fade_out((UBYTE)c0);
				AENG_flip();
			}
*/
		}
	}

	text_fudge	=	FALSE;
/*
	the_display.destroy_background_surface();

	if(image_mem)
	{
		MemFree(image_mem);
	}
*/
	ResetBackImage();
#endif
}

//---------------------------------------------------------------
#ifdef PSX
extern FONT2D_DrawString(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);
#endif

inline void printf2d(SLONG x, SLONG &y, CBYTE *fmt, ...) {
#ifndef PSX
	CBYTE msg[_MAX_PATH];
#else
	CBYTE msg[128];
#endif
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end  (ap);

	FONT2D_DrawString(msg,x,y,0x00ff00,256,POLY_PAGE_FONT2D);
#ifndef PSX
	y+=20;
#else
	y+=14;
#endif
}


#if 0

// urgh.
SLONG PrimaryScore, SecondaryScore, BonusScore;
#define EWAY_FLAG_ACTIVE	(1 << 0)
#ifndef	PSX
void ScoresCalc(void) {
	SLONG i, pts;
	EWAY_Way *ew;

	PrimaryScore=200;
	SecondaryScore=BonusScore=0;
	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
		if ((ew->flag&EWAY_FLAG_ACTIVE)&&(ew->ed.type==EWAY_DO_OBJECTIVE)) {
			pts = ew->ed.arg2 * 10;
			switch (ew->ed.subtype) {
//			case EWAY_SUBTYPE_OBJECTIVE_MAIN:		PrimaryScore+=pts;   break;
			case EWAY_SUBTYPE_OBJECTIVE_SUB:		SecondaryScore+=pts; break;
			case EWAY_SUBTYPE_OBJECTIVE_ADDITIONAL: BonusScore+=pts;	 break;
			}
		}
	}
}
#endif

#endif

#ifdef PSX
#define SCORE_SPACER 14
#else
#define SCORE_SPACER 20
#endif

extern	SLONG	stat_killed_thug;
extern	SLONG	stat_killed_innocent;
extern	SLONG	stat_arrested_thug;
extern	SLONG	stat_arrested_innocent;
extern	SLONG	stat_count_bonus;
extern	SLONG	stat_start_time,stat_game_time;

void ScoresDraw(void) 
{
#ifndef PSX

	SLONG y=35;
	SLONG count=0,count_bonus=0,count_bonus_left=0,c0;
	SLONG	ticks,h,m,s;
	ticks=stat_game_time;

	h=ticks/(1000*60*60);
	ticks-=h*1000*60*60;
	m=ticks/(1000*60);
	ticks-=m*1000*60;
	s=ticks/(1000);

	for(c0=0;c0<MAX_THINGS;c0++)
	{
		Thing	*p_thing;

		p_thing=TO_THING(c0);

		if(p_thing->Class==CLASS_PERSON)
		{
			if(p_thing->State!=STATE_DEAD)
			{
				switch(p_thing->Genus.Person->PersonType)
				{
 					case PERSON_THUG_RASTA:
					case PERSON_THUG_GREY:
					case PERSON_THUG_RED:
					case PERSON_MIB1:
					case PERSON_MIB2:
					case PERSON_MIB3:
						count++;
						break;


				}
			}

		}
		if(p_thing->Class==CLASS_SPECIAL)
		{
			if(p_thing->Genus.Special->SpecialType==SPECIAL_TREASURE)
			{
				count_bonus_left++;
			}
		}

	}






	CBYTE	str[128];

//	POLY_frame_init(FALSE, FALSE);


extern	SLONG	playing_real_mission(void);
	
	if(playing_real_mission())
	{
		if (NET_PERSON(0)->Genus.Person->PersonType != PERSON_DARCI &&
			NET_PERSON(0)->Genus.Person->PersonType != PERSON_ROPER)
		{
			//
			// This only works for Darci or Roper!
			//
		}
		else
		{

			FONT2D_DrawString(XLAT_str(X_WON_KILLED)	,10+2,300+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(XLAT_str(X_WON_ARRESTED)	,10+2,320+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(XLAT_str(X_WON_AT_LARGE)	,10+2,340+2,0x000000,256,POLY_PAGE_FONT2D);

			FONT2D_DrawString(XLAT_str(X_WON_KILLED)	,10,300,0xffffff,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(XLAT_str(X_WON_ARRESTED)	,10,320,0xffffff,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(XLAT_str(X_WON_AT_LARGE)	,10,340,0xffffff,256,POLY_PAGE_FONT2D);
		}

		FONT2D_DrawString(XLAT_str(X_WON_BONUS_FOUND)	,10+2,360+2,0x000000,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(XLAT_str(X_WON_BONUS_MISSED)	,10+2,380+2,0x000000,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(XLAT_str(X_WON_TIMETAKEN)		,10+2,400+2,0x000000,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(XLAT_str(X_WON_MUCKYTIME)		,10+2,420+2,0x000000,256,POLY_PAGE_FONT2D);

		FONT2D_DrawString(XLAT_str(X_WON_BONUS_FOUND)	,10,360,0xffffff,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(XLAT_str(X_WON_BONUS_MISSED)	,10,380,0xffffff,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(XLAT_str(X_WON_TIMETAKEN)		,10,400,0xffffff,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(XLAT_str(X_WON_MUCKYTIME)		,10,420,0xffffff,256,POLY_PAGE_FONT2D);

	//	FONT2D_DrawString(XLAT_str(X_WON_BESTTIME)	,10,420,0xffffff,256,POLY_PAGE_FONT2D);

		if (NET_PERSON(0)->Genus.Person->PersonType != PERSON_DARCI &&
			NET_PERSON(0)->Genus.Person->PersonType != PERSON_ROPER)
		{
			//
			// This only works for Darci or Roper!
			//
		}
		else
		{
			sprintf(str,": %d",stat_killed_thug);
			FONT2D_DrawString(str	,300+2,300+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(str	,300  ,300  ,0xffffff,256,POLY_PAGE_FONT2D);

		//	sprintf(str,": %d",stat_killed_innocent);
		//	FONT2D_DrawString(str	,300,300,0xffffff,256,POLY_PAGE_FONT2D);

			sprintf(str,": %d",stat_arrested_thug);
			FONT2D_DrawString(str	,300+2,320+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(str	,300  ,320  ,0xffffff,256,POLY_PAGE_FONT2D);

			sprintf(str,": %d",count);
			FONT2D_DrawString(str	,300+2,340+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(str	,300  ,340  ,0xffffff,256,POLY_PAGE_FONT2D);
		}

		sprintf(str,": %d",stat_count_bonus);
		FONT2D_DrawString(str	,300+2,360+2,0x000000,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(str	,300  ,360  ,0xffffff,256,POLY_PAGE_FONT2D);

		sprintf(str,": %d",count_bonus_left);
		FONT2D_DrawString(str	,300+2,380+2,0x000000,256,POLY_PAGE_FONT2D);
		FONT2D_DrawString(str	,300  ,380  ,0xffffff,256,POLY_PAGE_FONT2D);

		CBYTE code[32];

		{
			typedef struct
			{
				CBYTE *level_fname;
				CBYTE *level_name;
				SLONG  mins;
				SLONG  secs;
				CBYTE *who;
			 
			} Mime;	// A mucky time!

#if 0
// Times that shipped with the PC. Or was it the PSX?
			static Mime mucky_times[] =
			{
				{"\\FTutor1.ucm"	,"Combat Tutorial",				0,	0				},
				{"\\Assault1.ucm"	,"Physical Training",			0,	0				},
				{"\\testdrive1a.ucm","Driving Bronze",				0,	0				},
				{"\\fight1.ucm"		,"Combat Bronze",				0,	0				},
				{"\\police1.ucm"	,"RTA",							0,  55,	"Mark Rose" },
				{"\\testdrive2.ucm"	,"Driving Silver",				0,	0				},
				{"\\fight2.ucm"		,"Combat Silver",				0,	0				},
				{"\\police2.ucm"	,"The Jump",					1,	28,	"Mark Rose"	},
				{"\\testdrive3.ucm"	,"Driving Gold",				0,	0				},
				{"\\Bankbomb1.ucm"	,"Nitro Car",					4,	23, "Mike B."	},
				{"\\police3.ucm"	,"Gun Hunt",					1,	41,	"Fin"		},
				{"\\Gangorder2.ucm"	,"Rat Catch",					4,	00, "Mike B."	},
				{"\\police4.ucm"	,"Trouble in the Park",			1,	18,	"Mike B."	},
				{"\\bball2.ucm"		,"Gatecrasher",					0,	28,	"Mark Rose"	},
				{"\\wstores1.ucm"	,"Arms Breaker",				2,	51,	"Fin"		},
				{"\\e3.ucm"			,"Media Trouble",				1,	05, "Mark A."	},
				{"\\westcrime1.ucm"	,"Urban Shakedown",				4,	13,	"Mike B."	},
				{"\\carbomb1.ucm"	,"Auto-Destruct",				2,	48, "Mike B."	},
				{"\\botanicc.ucm"	,"Grim Gardens",				3,	46,	"Marie"		},
				{"\\Semtex.ucm"		,"Semtex",						8,	26,	"Mark Rose"	},
				{"\\AWOL1.ucm"		,"Cop Killers",					4,	27,	"Mark Rose"	},
				{"\\mission2.ucm"	,"Southside Offensive",			4,	48,	"Marie."	},
				{"\\park2.ucm"		,"Psycho Park",					5,	02, "Marie"		},
				{"\\MIB.ucm"		,"The Fallen",					7,	44,	"Marie"		},
				{"\\skymiss2.ucm"	,"Stern Revenge",				6,	32, "Mike B."	},
				{"\\factory1.ucm"	,"Transmission Terminated",		7,	 9,	"Dave"		},
				{"\\estate2.ucm"	,"Estate of Emergency",			4,	30,	"Fin"		},
				{"\\Stealtst1.ucm"	,"Seek, Sneak and Seize",		0,	46, "Mark A."	},
				{"\\snow2.ucm"		,"Target UC",					2,  06,	"Mark Rose"	},
				{"\\Gordout1.ucm"	,"Headline Hostage",			2,	00,	"Justin mucky(hands) Amore"	},
				{"\\Baalrog3.ucm"	,"Insane Assault",				1,	23,	"Disky"		},
				{"\\Finale1.ucm"	,"Day Of Reckoning",			5,	48,	"Dave"		},
				{"\\Gangorder1.ucm"	,"Assassin",					2,	13,	"Marie"		},
				{"\\FreeCD1.ucm"	,"Demo map",					1,	35, "Justin mucky(hands) Amore"},
				{NULL}
			};
#else
// DC times, starting from scratch.
			static Mime mucky_times[] =
			{
				{"\\FTutor1.ucm"	,"Combat Tutorial",				 0,	 0				},
				{"\\Assault1.ucm"	,"Physical Training",			 0,	 0				},
				{"\\testdrive1a.ucm","Driving Bronze",				 0,	 0				},
				{"\\fight1.ucm"		,"Combat Bronze",				 0,	 0				},
				{"\\police1.ucm"	,"RTA",							 1,  5,	"Alex Hood" },
				{"\\testdrive2.ucm"	,"Driving Silver",				 0,	 0				},
				{"\\fight2.ucm"		,"Combat Silver",				 0,	 0				},
				{"\\police2.ucm"	,"The Jump",					 1,	32,	"Joe Neate"	},
				{"\\testdrive3.ucm"	,"Driving Gold",				 0,	 0				},
				{"\\Bankbomb1.ucm"	,"Nitro Car",					 5,	 0, "Kenny Tang"	},
				{"\\police3.ucm"	,"Gun Hunt",					 1,	37,	"Alex Hood"	},
				{"\\Gangorder2.ucm"	,"Rat Catch",					 3,	14, "David Babajee"	},
				{"\\police4.ucm"	,"Trouble in the Park",			 1,	24,	"Kaoski"	},
				{"\\bball2.ucm"		,"Gatecrasher",					 0,	45,	"Tom Forsyth"	},
				{"\\wstores1.ucm"	,"Arms Breaker",				 4,	 3,	"Joe Neate"		},
				{"\\e3.ucm"			,"Media Trouble",				 5,	54, "Kwesi Moodie"	},
				{"\\westcrime1.ucm"	,"Urban Shakedown",				 6,	 4,	"Alex Hood"	},
				{"\\carbomb1.ucm"	,"Auto-Destruct",				 4,	18, "David Babajee"	},
				{"\\botanicc.ucm"	,"Grim Gardens",				 6,	29,	"Kwesi Moodie"		},
				{"\\Semtex.ucm"		,"Semtex",						16,	 3,	"Kaoski"	},
				{"\\AWOL1.ucm"		,"Cop Killers",					10,	30,	"Kenny Tang"	},
				{"\\mission2.ucm"	,"Southside Offensive",			 7,	49,	"Alex Hood"	},
				{"\\park2.ucm"		,"Psycho Park",					 6,	45, "Tom Forsyth"		},
				{"\\MIB.ucm"		,"The Fallen",					26,	53,	"Phil Maskell"		},
				{"\\skymiss2.ucm"	,"Stern Revenge",				 9,	28, "David Babajee"	},
				{"\\factory1.ucm"	,"Transmission Terminated",		 8,	35,	"Kwesi Moodie"		},
				{"\\estate2.ucm"	,"Estate of Emergency",			 8,	14,	"Phil Maskell"		},
				{"\\Stealtst1.ucm"	,"Seek, Sneak and Seize",		 5,	28, "Kaoski"	},
				{"\\snow2.ucm"		,"Target UC",					 2, 26,	"Kaoski"	},
				{"\\Gordout1.ucm"	,"Headline Hostage",			 5,	42,	"Kenny Tang"	},
				{"\\Baalrog3.ucm"	,"Insane Assault",				 2,	38,	"Mike Diskett"		},
				{"\\Finale1.ucm"	,"Day Of Reckoning",			40,	00,	"Beat me"		},
				{"\\Gangorder1.ucm"	,"Assassin",					 5,	21,	"Joe Neate"		},
				{"\\FreeCD1.ucm"	,"Demo map",					40,	00, "Beat me"},
				{"\\Album1.ucm"		,"Breakout!",					15,	38, "Tom Forsyth"},
				{NULL}
			};
#endif

			CBYTE par[128];

			sprintf(par, "No time yet. Email TomF your time!");

			SLONG i = 0;

			while(1)
			{
				Mime *mm = &mucky_times[i];
				
				if (!mm->level_fname)
				{
					break;
				}

				extern CBYTE ELEV_fname_level[];

				if (strstr(ELEV_fname_level, mm->level_fname) && mm->who)
				{
					SLONG mucky_time;
					SLONG their_time;

					mucky_time = mm->mins * 60 + mm->secs;
					their_time = (h * 60 + m) * 60 + s;

					extern BOOL dkeys_have_been_used;

extern bool g_bPunishMePleaseICheatedOnThisLevel;
					if (their_time >= mucky_time || dkeys_have_been_used || g_bPunishMePleaseICheatedOnThisLevel)
					{
						// No code for you!
						code[0] = '\000';
					}
					else
					{
						SLONG hash;
						
						hash  = (i + 1) * (m + 1) * (s + 1) * 3141;
						hash  = hash % 12345;
						hash += 0x9a2f;

#ifdef TARGET_DC
						// Invert the code for the DC to stop cheating scum
						// sending in the PC times.
						hash ^= 0xffff;
#endif

						sprintf(code, ": CODE <%X> ", hash);
					}

					sprintf(par, ": (00:%02d:%02d)  %s", mm->mins, mm->secs, mm->who);

					break;
				}

				i++;
			}

			sprintf(str,": (%02d:%02d:%02d) %s",h,m,s, code);
			FONT2D_DrawString(str	,300+2,400+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(str	,300  ,400  ,0xffffff,256,POLY_PAGE_FONT2D);

			FONT2D_DrawString(par	,300+2,420+2,0x000000,256,POLY_PAGE_FONT2D);
			FONT2D_DrawString(par	,300  ,420  ,0xffffff,256,POLY_PAGE_FONT2D);
		}

	//	sprintf(str,": %d",stat_arrested_innocent);
	//	FONT2D_DrawString(str	,300,300,0xffffff,256,POLY_PAGE_FONT2D);
	}


//	POLY_frame_draw(FALSE, FALSE);

#endif

}

#if 0

void level_won(void)
{
	SLONG tick1 = GetTickCount();
	SLONG tick2;
	SLONG timet;

	set_person_idle(NET_PERSON(0));

	ScoresCalc();

	CONSOLE_text(" ");
/*
#ifndef PSX
	CONSOLE_text("PRESS SPACE TO CONTINUE");
#else
	CONSOLE_text("PRESS "STR_CROSS" TO CONTINUE");
#endif
	*/
	CONSOLE_text(XLAT_str(X_PRESS_TO_CONTINUE));
	while(SHELL_ACTIVE&&(GAME_STATE&GS_LEVEL_WON))
	{
#ifndef	PSX


		ULONG input = get_hardware_input(INPUT_TYPE_JOY); // 1 << 1 => Joystick

		if(LastKey==KB_SPACE || LastKey==KB_ESC || (input & (INPUT_MASK_ACTION|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_JUMP|INPUT_MASK_KICK|INPUT_MASK_PUNCH)))
		{
			GAME_STATE	=	0;
			LastKey		=	0;
		}
#else
		if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		{
			GAME_STATE  =   0;
		}

#endif
//		process_things(1);
#ifdef	OLD_CAM
		{

			SLONG cam_x;
			SLONG cam_y;
			SLONG cam_z;

			SLONG cam_yaw;
			SLONG cam_pitch;
			SLONG cam_roll;

			float radians_yaw;
			float radians_pitch;
			float radians_roll;

			CAM_get(
				&cam_x,
				&cam_y,
				&cam_z,
				&cam_yaw,
				&cam_pitch,
				&cam_roll,
				&radians_yaw,
				&radians_pitch,
				&radians_roll);

			AENG_set_camera_radians(
				 cam_x,
				 cam_y,
				 cam_z,
				 radians_yaw,
				 radians_pitch,
				 radians_roll);
		}
#endif
/*
		if (GAME_FLAGS & GF_INDOORS)
		{
//			AENG_draw_inside();
		}
		else
		{
			AENG_draw_city();
		}
*/
		extern SLONG draw_3d;

		AENG_draw(draw_3d);


		OVERLAY_handle();
		CONSOLE_draw();

//		ScoresDraw();

		AENG_flip();

		//
		// Lock frame-rate to 30 FPS
		//
#ifndef PSX		
		do {
			tick2 = GetTickCount();
			timet = tick2 - tick1;
		} while (timet < (1000 / 30));

		tick1 = tick2;
#endif
		GAME_TURN += 1;

	}
#ifndef PSX
	STARTSCR_notify_gameover(1);
#endif
//	if (BRIEFING_next_mission()) auto_advance=1;


}

//---------------------------------------------------------------

void	level_lost(void)
{
	SLONG tick1;
	SLONG tick2;
	SLONG timet;

	CONSOLE_text(" ");
//	CONSOLE_text(" - G A M E   O V E R -",40000);
	CONSOLE_text(XLAT_str(X_GAME_OVER),40000);
/*
#ifndef PSX
	CONSOLE_text("PRESS SPACE TO CONTINUE");
#else
	CONSOLE_text("PRESS "STR_CROSS" TO CONTINUE");
#endif
*/
	CONSOLE_text(XLAT_str(X_PRESS_TO_CONTINUE));
	tick1 = GetTickCount();

	while(SHELL_ACTIVE&&(GAME_STATE&GS_LEVEL_LOST))
	{
#ifndef	PSX

		ULONG input = get_hardware_input(INPUT_TYPE_JOY); // 1 << 1 => Joystick

		if(LastKey==KB_SPACE || LastKey==KB_ESC || (input & (INPUT_MASK_ACTION|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_JUMP|INPUT_MASK_KICK|INPUT_MASK_PUNCH)))
		{
			GAME_STATE	=	0;
			LastKey		=	0;
		}
#else
		if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		{
			GAME_STATE  =   0;
			wad_level   =   0;
		}

#endif

//		process_things(1);
#ifdef	OLD_CAM
		{
			SLONG cam_x;
			SLONG cam_y;
			SLONG cam_z;

			SLONG cam_yaw;
			SLONG cam_pitch;
			SLONG cam_roll;

			float radians_yaw;
			float radians_pitch;
			float radians_roll;

			CAM_get(
				&cam_x,
				&cam_y,
				&cam_z,
				&cam_yaw,
				&cam_pitch,
				&cam_roll,
				&radians_yaw,
				&radians_pitch,
				&radians_roll);

			AENG_set_camera_radians(
				 cam_x,
				 cam_y,
				 cam_z,
				 radians_yaw,
				 radians_pitch,
				 radians_roll);
		}
#endif

		extern SLONG draw_3d;

		AENG_draw(draw_3d);
//		ANEG_draw_messages();
		OVERLAY_handle();
		CONSOLE_draw();

		AENG_flip();

		//
		// Lock frame-rate to 30 FPS
		//
#ifndef PSX		
		while(1)
		{
			tick2 = GetTickCount();
			timet = tick2 - tick1;

			if (timet > (1000 / 30))
			{
				break;
			}
		}

		tick1 = tick2;
#endif
		GAME_TURN += 1;
	}
#ifndef PSX
	STARTSCR_notify_gameover(0);
#endif
}

#endif

//---------------------------------------------------------------


void ATTRACT_loadscreen_init(void)
{

	PANEL_disable_screensaver ( TRUE );

#ifdef TARGET_DC

extern void POLY_ClearAllPages ( void );
	POLY_ClearAllPages();

	// Flip once to flush the rendering (e.g. leaves will be left in).
	the_display.lp_D3D_Viewport->Clear2 ( 0, NULL, D3DCLEAR_TARGET, 0x0, 0.0f, 0 );
	the_display.lp_D3D_Device->EndScene();
	the_display.lp_DD_FrontSurface->Flip ( NULL, DDFLIP_WAIT );

	// Clear both buffers.
	the_display.lp_D3D_Device->BeginScene();
	the_display.lp_D3D_Viewport->Clear2 ( 0, NULL, D3DCLEAR_TARGET, 0x0, 0.0f, 0 );
	the_display.lp_D3D_Device->EndScene();
	the_display.lp_DD_FrontSurface->Flip ( NULL, DDFLIP_WAIT );

	the_display.lp_D3D_Device->BeginScene();
	the_display.lp_D3D_Viewport->Clear2 ( 0, NULL, D3DCLEAR_TARGET, 0x0, 0.0f, 0 );
	the_display.lp_D3D_Device->EndScene();
	the_display.lp_DD_FrontSurface->Flip ( NULL, DDFLIP_WAIT );

	// Really horrible hack.
void	FRONTEND_scr_img_load_into_screenfull(CBYTE *name, CompressedBackground *screen);
	FRONTEND_scr_img_load_into_screenfull ( "e3load.tga", &(the_display.lp_DD_Background) );
	UnpackBackground ( (BYTE*)( the_display.lp_DD_Background ), the_display.lp_DD_BackSurface );
	//AENG_flip();
	the_display.lp_DD_FrontSurface->Flip ( NULL, DDFLIP_WAIT );

	the_display.lp_DD_BackSurface->Blt ( NULL, the_display.lp_DD_FrontSurface, NULL, DDBLT_WAIT, NULL );

	the_display.lp_DD_FrontSurface->Flip ( NULL, DDFLIP_WAIT );

	//InitBackImage("e3load.tga");
	//ShowBackImage(FALSE);
	//AENG_flip();

	// Then free up the compressed image data again.
	ResetBackImage();


#else

#ifndef PSX
	InitBackImage("e3load.tga");
	ShowBackImage(FALSE);
	AENG_flip();
	// Er... why do this again????
	InitBackImage("e3load.tga");
	ShowBackImage(FALSE);
	AENG_flip();
#endif

#endif
}


#ifdef TARGET_DC

// None of the graphics systems are up at the moment,
// so just draw straight to the front buffer.
void ATTRACT_loadscreen_draw(SLONG completion)	// completion is in 8-bit fixed point from 0 to 256.
{
	#define START_R 50
	#define START_G 59
	#define START_B	80

	#define END_R 210
	#define END_G 216
	#define END_B 208

	SLONG along;

	SLONG r;
	SLONG g;
	SLONG b;

	SLONG i;

	SLONG colour;

	HRESULT hres;


#if 1

	DDSURFACEDESC2 ddsd;
	memset ( &ddsd, 0, sizeof ( ddsd ) );
	ddsd.dwSize = sizeof ( ddsd );

	hres = the_display.lp_DD_FrontSurface->Lock ( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL );

	for (i = 0; i < (completion >> 3); i += 1)
	{
		r = START_R + (END_R - START_R) * i >> 5;
		g = START_G + (END_G - START_G) * i >> 5;
		b = START_B + (END_B - START_B) * i >> 5;

		// Tailored to 565.
		colour = ((r & 0xf8) << 8) | ((g & 0xfc ) << 3) | (b >> 3);
		//colour |= colour << 16;

		RECT rect;
		rect.left   = 0  + i * 20;
		rect.right  = 20 + i * 20;
		// Got to move this higher for NTSC, or it falls off the bottom of the screen.
		if ( eDisplayType == DT_NTSC )
		{
			rect.top    = 435;
			rect.bottom = 455;
		}
		else
		{
			rect.top    = 455;
			rect.bottom = 475;
		}

		// Two black lines.
		for ( int iY = rect.top; iY < rect.top+2; iY++ )
		{
			WORD *wPtr = (WORD*)( (char *)ddsd.lpSurface + iY * ddsd.lPitch );
			wPtr += rect.left;
			for ( int iX = rect.left; iX < rect.right; iX++ )
			{
				*wPtr++ = 0;
			}
		}

		for ( iY = rect.top+2; iY < rect.bottom; iY++ )
		{
			WORD *wPtr = (WORD*)( (char *)ddsd.lpSurface + iY * ddsd.lPitch );
			wPtr += rect.left;
			for ( int iX = rect.left; iX < rect.right - 2; iX++ )
			{
				*wPtr++ = colour;
			}
			// And two black pixels.
			*wPtr++ = 0;
			*wPtr++ = 0;
		}

	}

	hres = the_display.lp_DD_FrontSurface->Unlock ( NULL );


#else

	DDBLTFX ddbltfx;
	memset ( &ddbltfx, 0, sizeof ( ddbltfx ) );
	ddbltfx.dwSize = sizeof ( ddbltfx );

	for (i = 0; i < (completion >> 3); i += 1)
	{
		r = START_R + (END_R - START_R) * i >> 5;
		g = START_G + (END_G - START_G) * i >> 5;
		b = START_B + (END_B - START_B) * i >> 5;

		// Tailored to 565.
		colour = ((r & 0xf8) << 8) | ((g & 0xfc ) << 3) | (b >> 3);
		colour |= colour << 16;

		RECT rect;
		rect.left   = 5  + i * 20;
		rect.right  = 23 + i * 20;
		rect.top    = 455;
		rect.bottom = 475;

		ddbltfx.dwFillColor = colour;

		HRESULT hres = the_display.lp_DD_BackSurface->Blt ( &rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_DDFX, &ddbltfx );

		hres++;

	}

	AENG_flip();

#endif

}


#else

void ATTRACT_loadscreen_draw(SLONG completion)	// completion is in 8-bit fixed point from 0 to 256.
{
#ifndef	PSX
	ShowBackImage(FALSE);
	PANEL_draw_completion_bar(completion);
	AENG_flip();
#endif
}

#endif

