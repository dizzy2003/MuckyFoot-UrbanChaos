//
// Menus from within the game.
//

#include "game.h"
#include "gamemenu.h"
#include "panel.h"
#include "poly.h"
#include "menufont.h"
#include "xlat_str.h"
#include "mfx.h"
#include "sound_id.h"
#include "music.h"
#include "memory.h"
#ifdef TARGET_DC
#include "interfac.h"
#include "DIManager.h"
#include "frontend.h"
#endif

//
// Externs
//

extern void	process_things_tick(SLONG frame_rate_independant);



//
// The current menu.
//

#define GAMEMENU_MENU_TYPE_NONE     0
#define GAMEMENU_MENU_TYPE_PAUSE    1
#define GAMEMENU_MENU_TYPE_WON      2
#define GAMEMENU_MENU_TYPE_LOST     3
#define GAMEMENU_MENU_TYPE_SURE     4
#define GAMEMENU_MENU_TYPE_NUMBER   5

SLONG GAMEMENU_menu_type;
SLONG GAMEMENU_menu_selection;


//
// The darken the background fadein.
//

SLONG GAMEMENU_background;


//
// The text fadein x position (in 8-bit fixed point)
//

SLONG GAMEMENU_fadein_x;


//
// The game slowdown.
//

SLONG GAMEMENU_slowdown;



//
// Wait some time before declaring a level won or lost.
//

SLONG GAMEMENU_wait;



//
// The reason you lost the level.
//

CBYTE *GAMEMENU_level_lost_reason;



#ifdef TARGET_DC
bool bDontShowThePauseScreen = FALSE;
#endif



//
// Each of the menus.
//

typedef struct
{
	UBYTE word[8];	// word[0] is the title, the rest are the menus.
 
} GAMEMENU_Menu;

GAMEMENU_Menu GAMEMENU_menu[GAMEMENU_MENU_TYPE_NUMBER] =
{
	{NULL},
	{X_GAME_PAUSED, X_RESUME_LEVEL, X_RESTART_LEVEL, X_ABANDON_GAME},
	{X_LEVEL_COMPLETE},
	{X_LEVEL_LOST, X_RESTART_LEVEL,  X_ABANDON_GAME},
	{X_ARE_YOU_SURE, X_OKAY, X_CANCEL}
};



//
// Initialises a new menu.
//

void GAMEMENU_initialise(SLONG menu)
{
	if (GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_NONE || menu == GAMEMENU_MENU_TYPE_NONE)
	{
		MENUFONT_fadein_init();

		GAMEMENU_background     = 0;
		GAMEMENU_fadein_x       = 0;
		GAMEMENU_slowdown       = 0x10000;
		GAMEMENU_menu_selection = 1;

		ResetSmoothTicks();
	}
	else
	{
		//
		// Moving from one menu to another. Just fade the text. Keep the
		// background and slowdown the same.
		//

		GAMEMENU_fadein_x = 0;
	}

	GAMEMENU_menu_type = menu;
}


void GAMEMENU_init()
{
	MENUFONT_fadein_init();

	GAMEMENU_background         = 0;
	GAMEMENU_fadein_x           = 0;
	GAMEMENU_slowdown           = 0x10000;
	GAMEMENU_menu_selection     = 1;
	GAMEMENU_wait               = 0;
	GAMEMENU_level_lost_reason  = NULL;

	GAMEMENU_initialise(GAMEMENU_MENU_TYPE_NONE);
}




SLONG GAMEMENU_process()
{
	SLONG i;

	//
	// The number of milliseconds since the last call to GAMEMENU_process.
	//

	SLONG millisecs;

	#ifndef PSX

	static SLONG tick_last = 0;
	static SLONG tick_now = 0;
	
	tick_now  = GetTickCount();

	millisecs = tick_now - tick_last;
	tick_last = tick_now;

	#else

	millisecs = 50;	// 20 fps

	#endif

	//
	// Make sure nothing is too extreme...
	//

	SATURATE(millisecs, 10, 200);


#ifdef TARGET_DC
	ULONG input;
	if (GAMEMENU_menu_type != GAMEMENU_MENU_TYPE_NONE)
	{
		// There is a menu up, so poll using the "menu" mappings.
		if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
		{
			// Do NOT use INPUT_TYPE_REMAP_START_BUTTON, because start needs to deactivate pause, NOT select the current option.
			// Also, don't remap X and Y - they are used for showing a static screen.
			input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_GONEDOWN | INPUT_TYPE_REMAP_NOT_X_Y );

			// Now see if X+Y are pressed.
extern DIJOYSTATE the_state;
			if ( ( ( the_state.rgbButtons[DI_DC_BUTTON_X] & 0x80 ) != 0 ) && ( ( the_state.rgbButtons[DI_DC_BUTTON_Y] & 0x80 ) != 0 ) )
			{
				bDontShowThePauseScreen = TRUE;
			}
		}
		else
		{
			input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_REMAP_START_BUTTON | INPUT_TYPE_GONEDOWN );

			// Freeze out input while the feadeing happens when the level is won or lost.
			// Otherwise it's too easy to skip through it by accident.
			if ( ( ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_WON ) || 
				   ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_LOST ) ) &&
				 ( GAMEMENU_wait < (64 * 20 * 4) ) )
			{
				// Bin the input.
				TRACE ( "I:" );
				input = 0;
			}

		}
	}
	else
	{
		// No menu, so just get the last input that was polled.
		input = get_last_input ( INPUT_TYPE_GONEDOWN );
	}
#endif

	//
	// Automatically go into these menus.
	//
	
	if (GAME_STATE & (GS_LEVEL_LOST | GS_LEVEL_WON))
	{
		if (MFX_QUICK_still_playing() && !MUSIC_is_playing())
		{
			//
			// Somebody is saying something...
			//
		}
		else
		{
			GAMEMENU_wait += 64 * TICK_RATIO >> TICK_SHIFT;
		}
	}

	if (GAMEMENU_wait > (64 * 20 * 2) && GAMEMENU_menu_type != GAMEMENU_MENU_TYPE_SURE)
	{
		//
		// Bring up lost\won box after a 3 second delay.
		//

		if (GAME_STATE & GS_LEVEL_LOST)
		{
			if (GAMEMENU_menu_type != GAMEMENU_MENU_TYPE_LOST)
			{
				GAMEMENU_initialise(GAMEMENU_MENU_TYPE_LOST);
				if (NET_PERSON(0)->Genus.Person->Health>0)
					MUSIC_mode(MUSIC_MODE_CHAOS);
				else
					MUSIC_mode(MUSIC_MODE_GAMELOST);
			}
		}
		else
		if (GAME_STATE & GS_LEVEL_WON)
		{
			if (GAMEMENU_menu_type != GAMEMENU_MENU_TYPE_WON)
			{
				GAMEMENU_initialise(GAMEMENU_MENU_TYPE_WON);
				MUSIC_mode(MUSIC_MODE_GAMEWON);
			}
		}

		if (GAME_STATE&(GS_LEVEL_WON|GS_LEVEL_LOST))
		{
			if (GAMEMENU_is_paused()) 
				process_things_tick(1);
			MUSIC_mode_process();
		}

		//
		// Make sure that we're not in widescreen!
		//

		extern SLONG EWAY_cam_freeze;
		extern UBYTE GAME_cut_scene;

		EWAY_cam_freeze = FALSE;
		GAME_cut_scene  = FALSE;
	}

#ifdef TARGET_DC
	if ( Keys[KB_ESC] || ( input & ( INPUT_MASK_START ) ) )
#else
	if (Keys[KB_ESC])
#endif
	{
		Keys[KB_ESC] = 0;


#ifdef TARGET_DC
		bDontShowThePauseScreen = FALSE;
#endif


		switch(GAMEMENU_menu_type)
		{
			case GAMEMENU_MENU_TYPE_NONE:
				// Stop all sounds

				// Which one of these should I use?
				MFX_stop ( MFX_CHANNEL_ALL, MFX_WAVE_ALL );
				//stop_all_fx_and_music();

				// And enter the pause menu.
				GAMEMENU_initialise(GAMEMENU_MENU_TYPE_PAUSE);
				break;

			case GAMEMENU_MENU_TYPE_PAUSE:
				GAMEMENU_initialise(GAMEMENU_MENU_TYPE_NONE);
				break;

			case GAMEMENU_MENU_TYPE_WON:
				return GAMEMENU_DO_NEXT_LEVEL;

			case GAMEMENU_MENU_TYPE_LOST:
				return GAMEMENU_DO_CHOOSE_NEW_MISSION;

			case GAMEMENU_MENU_TYPE_SURE:
				GAMEMENU_initialise(GAMEMENU_MENU_TYPE_PAUSE);
				break;

			default:
				ASSERT(0);
				break;
		}
	}


#ifdef TARGET_DC
	// If there is no primary (i.e. a controller was removed), go into the pause menu.
extern DIDeviceInfo *primary_device;
	if ( ( primary_device == NULL ) && ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_NONE ) )
	{
		GAMEMENU_initialise(GAMEMENU_MENU_TYPE_PAUSE);
	}
#endif

	if (GAMEMENU_menu_type != GAMEMENU_MENU_TYPE_NONE)
	{
		//
		// The fade and slowdown.
		//

		GAMEMENU_background += millisecs >> 0;
		GAMEMENU_slowdown   -= millisecs << 6;
		GAMEMENU_fadein_x   += millisecs << 7;

		//
		// Make the text fade in after the background.
		//

		if (GAMEMENU_background < 400)
		{
			GAMEMENU_fadein_x = 0;
		}

		SATURATE(GAMEMENU_background, 0,      640);
		SATURATE(GAMEMENU_slowdown,   0,  0x10000);
		SATURATE(GAMEMENU_fadein_x,   0, 800 << 8);	// More than 640 so the rightmost part of the screen is fully faded in...

		if (GAMEMENU_background > 200)
		{
			//
			// Control the menu.
			//

#ifdef TARGET_DC
			if ( Keys[KB_UP] || ( input & INPUT_MASK_FORWARDS ) )
#else
			if (Keys[KB_UP])
#endif
			{
				Keys[KB_UP] = 0;

				GAMEMENU_menu_selection -= 1;

				if (GAMEMENU_menu_selection < 1)
				{
					for (i = 1; i <= 7; i++)
					{
						if ( GAMEMENU_menu[GAMEMENU_menu_type].word[i] != NULL )
						{
							GAMEMENU_menu_selection = i;
						}
					}
				}

				MFX_play_stereo(0,S_MENU_CLICK_START,MFX_REPLACE);
			}

#ifdef TARGET_DC
			if ( Keys[KB_DOWN] || ( input & INPUT_MASK_BACKWARDS ) )
#else
			if (Keys[KB_DOWN])
#endif
			{
				Keys[KB_DOWN] = 0;

				GAMEMENU_menu_selection += 1;

				if ( ( GAMEMENU_menu_selection > 7 ) || ( GAMEMENU_menu[GAMEMENU_menu_type].word[GAMEMENU_menu_selection] == NULL ) )
				{
					GAMEMENU_menu_selection = 1;
				}

				MFX_play_stereo(0,S_MENU_CLICK_START,MFX_REPLACE);
			}

			SATURATE(GAMEMENU_menu_selection, 1, 7);

			if (GAMEMENU_menu[GAMEMENU_menu_type].word[GAMEMENU_menu_selection] == NULL)
			{
				GAMEMENU_menu_selection -= 1;

				if ( GAMEMENU_menu_selection == 0 )
				{
					GAMEMENU_menu_selection = 1;
				}
			}

#ifdef TARGET_DC
			// Don't allow selections when the pause screen is hidden -
			// people don't know what they're selecting. Start will exit from
			// pause, and they can go back in if they want to do something.
			if ( ( Keys[KB_ENTER] || Keys[KB_SPACE] || Keys[KB_PENTER] || ( input & INPUT_MASK_DOMENU ) ) && !bDontShowThePauseScreen )
#else
			if (Keys[KB_ENTER] || Keys[KB_SPACE] || Keys[KB_PENTER])
#endif
			{
				Keys[KB_ENTER ] = 0;
				Keys[KB_SPACE ] = 0;
				Keys[KB_PENTER] = 0;



				MFX_play_stereo(1,S_MENU_CLICK_END,MFX_REPLACE);

				switch(GAMEMENU_menu[GAMEMENU_menu_type].word[GAMEMENU_menu_selection])
				{
					case NULL:
						return GAMEMENU_DO_NEXT_LEVEL;

					case X_RESUME_LEVEL:
						GAMEMENU_initialise(GAMEMENU_MENU_TYPE_NONE);
						break;

					case X_RESTART_LEVEL:
						return GAMEMENU_DO_RESTART;

					case X_ABANDON_GAME:
						GAMEMENU_initialise(GAMEMENU_MENU_TYPE_SURE);
						break;

					case X_OKAY:
						return GAMEMENU_DO_CHOOSE_NEW_MISSION;

					case X_CANCEL:
						GAMEMENU_initialise(GAMEMENU_MENU_TYPE_PAUSE);
						break;

#ifndef TARGET_DC
					case X_SAVE_GAME:
						MEMORY_quick_save();
						GAMEMENU_initialise(GAMEMENU_MENU_TYPE_NONE);
						break;

					case X_LOAD_GAME:
						if (!MEMORY_quick_load())
						{
							return GAMEMENU_DO_RESTART;
						}
						break;
#endif

					default:
						ASSERT(0);
						break;
				}
			}
		}
	}

	return GAMEMENU_DO_NOTHING;
}



SLONG GAMEMENU_is_paused()
{
	return GAMEMENU_slowdown == 0;
}


SLONG GAMEMENU_slowdown_mul()
{
	return GAMEMENU_slowdown >> 8;
}


void GAMEMENU_set_level_lost_reason(CBYTE *reason)
{
	GAMEMENU_level_lost_reason = reason;
}


//
// Draws all the game menu stuff.
//



void GAMEMENU_draw()
{
	SLONG i;

	if (GAMEMENU_menu_type== GAMEMENU_MENU_TYPE_NONE)
	{
		return;
	}

#ifdef TARGET_DC
	if ( bDontShowThePauseScreen )
	{
		return;
	}
#endif


	//
	// Darken the screen.
	//


#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif
	PANEL_darken_screen(GAMEMENU_background);
#ifndef TARGET_DC
	POLY_frame_draw(FALSE,FALSE);
#endif


	//
	// Draw the menu.
	//

#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif

	MENUFONT_fadein_line(GAMEMENU_fadein_x);
	MENUFONT_fadein_draw(320, 100, 255, XLAT_str(GAMEMENU_menu[GAMEMENU_menu_type].word[0]));

	bool bDrawMainPartOfMenu = TRUE;

	if (GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_LOST)
	{
		if (GAMEMENU_level_lost_reason)
		{
			//
			// Draw the reason why we lost.
			//

			// This looks crap. The text is too big, it's all
			// in capitals and it doesn't fit on the screen.

			// MENUFONT_fadein_draw(320, 120, 255, GAMEMENU_level_lost_reason);
		}
	}
	else
	if (GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_WON)
	{
		//
		// draw the stats
		//

extern void ScoresDraw(void);	// From attract

		ScoresDraw();
	}
#ifdef TARGET_DC
	else
	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		static iFlash = 0;
extern DIDeviceInfo *primary_device;
extern BOOL AreAnyDevicesConnected ( void );
		if ( primary_device == NULL )
		{
			UBYTE bMyFade = 255;
			if ( ( iFlash & 0x10 ) == 0 )
			{
				bMyFade = 128;
			}

			if ( AreAnyDevicesConnected() )
			{
				// No current controller, but there is one connected. Tell them to press start.
				MENUFONT_fadein_draw(
					320,
					350,
					bMyFade,
					XLAT_str ( X_CONTROLLER_REMOVED ) );
			}
			else
			{
				// No controllers at all.
#ifdef SEGA_ARE_CONSIDERATE_KIND_AND_HELPFUL_AND_WISH_NOT_TO_MAKE_MY_GAME_FUCKING_UGLY
				MENUFONT_fadein_draw(
					320,
					350,
					bMyFade,
					XLAT_str ( X_NO_CONTROLLER_CONNECTED ) );
#else
				// But they're not, and they do.
				char *pcString[4];
				if ( !IsEnglish )
				{
					pcString[0] = "Une manette vient";
					pcString[1] = "d'etre retirée ou";		// NOTE! The E in etre should be ê, but the font doesn't have it, and it's all in caps anyway.
					if ( bWriteVMInsteadOfVMU )
					{
						pcString[2] = "une VM est en";
					}
					else
					{
						pcString[2] = "une VMU est en";
					}
					pcString[3] = "cours de détection";
				}
				else
				{
					pcString[0] = "The controller";
					pcString[1] = "has been removed";
					if ( bWriteVMInsteadOfVMU )
					{
						pcString[2] = "or a VM is";
					}
					else
					{
						pcString[2] = "or a VMU is";
					}
					pcString[3] = "being recognised";
				}


				MENUFONT_fadein_draw( 320, 155 +   0, bMyFade, pcString[0] );
				MENUFONT_fadein_draw( 320, 155 +  40, bMyFade, pcString[1] );
				MENUFONT_fadein_draw( 320, 155 +  80, bMyFade, pcString[2] );
				MENUFONT_fadein_draw( 320, 155 + 120, bMyFade, pcString[3] );
				bDrawMainPartOfMenu = FALSE;
#endif
			}

			iFlash++;
		}
	}


#endif


	if ( bDrawMainPartOfMenu )
	{
		for (i = 1; i < 8; i++)
		{
			if (GAMEMENU_menu[GAMEMENU_menu_type].word[i])
			{
				MENUFONT_fadein_draw(
					320,
					155 + i * 40,
					(i == GAMEMENU_menu_selection) ? 255 : 128,
					XLAT_str(GAMEMENU_menu[GAMEMENU_menu_type].word[i]));
			}
		}
	}

#ifndef TARGET_DC
	POLY_frame_draw(FALSE,FALSE);
#endif
}
