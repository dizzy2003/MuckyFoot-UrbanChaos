//
// frontend.cpp
//
// matthew rosenfeld 8 july 99
//
// this is our new front end thingy to replace the hideous startscr.cpp
//


//#define BIN_STUFF_PLEASE_BOB I have been defined
//#define FORCE_STUFF_PLEASE_BOB I have been defined
#ifdef DEBUG
//#define BIN_BACKGROUNDS_PLEASE_BOB I have been defined
#else
//#define BIN_BACKGROUNDS_PLEASE_BOB I have been defined
#endif

#ifndef TARGET_DC
// On a PC, you need an exit. On a console, you don't.
#define WANT_AN_EXIT_MENU_ITEM defined
// Keyboard not currently supported on DC - might be in future.
#define WANT_A_KEYBOARD_ITEM define
// On DC, the "Start" button is not allowed to be remapped.
#define WANT_A_START_JOYSTICK_ITEM defined

#else

// On the DC, we need a title & language selection screen.
#define WANT_A_TITLE_SCREEN defined

#endif

#ifndef PSX

#include	"demo.h"
#include	"DDLib.h"
#include "frontend.h"
#include "xlat_str.h"
#include "menufont.h"
#include "font2d.h"
#include "env.h"
#include "drive.h"
#include "snd_type.h"
#include "sound.h"
#include "MFX.h"
#include "MFX_Miles.h"
#include "music.h"
#include "poly.h"
#include "drawxtra.h"
#include "fmatrix.h"
#include    "fallen/DDLibrary/headers/D3DTexture.h"
#include    "fallen/DDLibrary/headers/GDisplay.h"
#include	"fallen/DDEngine/headers/polypage.h"
#include "io.h"
#include "truetype.h"
#include "fallen/ddlibrary/headers/dclowlevel.h"
#include "panel.h"

#ifdef TARGET_DC
#include <platutil.h>
#include <ceddcdrm.h>
#include <segagdrm.h>
#endif

#include "interfac.h"
extern	BOOL	allow_debug_keys;


#ifndef TARGET_DC
UBYTE build_dc = TRUE;		// Set to TRUE to save out all the dreamcast DAD files.
UBYTE build_dc_mission;
#endif

//----------------------------------------------------------------------------
// EXTERNS
//

extern SLONG FontPage;
extern UBYTE InkeyToAscii[];
extern UBYTE InkeyToAsciiShift[];
extern CBYTE STARTSCR_mission[_MAX_PATH];
extern DIJOYSTATE			the_state;

extern void	init_joypad_config(void);

void DreamCastCredits ( void );
void	FRONTEND_display ( void );

//----------------------------------------------------------------------------
// DEFINES
//

#define ALLOW_JOYPAD_IN_FRONTEND


// font sizes, where 256 is "normal".
#define BIG_FONT_SCALE (192)
#define BIG_FONT_SCALE_FRENCH ( 176 );

#ifdef TARGET_DC
#define BIG_FONT_SCALE_SELECTED (256)
#endif
#define SMALL_FONT_SCALE (256)
// small font being larger DOES make sense because they're different bitmaps.

// In milliseconds.
#ifdef DEBUG
// For debugging the looper
#define AUTOPLAY_FMV_DELAY (1000 * 10)
#else
#define AUTOPLAY_FMV_DELAY (1000 * 60 * 2)
#endif

// #define ALLOW_DANGEROUS_OPTIONS

//#define MISSION_SCRIPT "data\\urban.sty"
// see below...

#if 0
#define	STARTS_START	1
#define	STARTS_EDITOR	2
#define	STARTS_LOAD		3
#define	STARTS_EXIT		4
#define STARTS_LANGUAGE_CHANGE 10
#else
#include "startscr.h"
#endif

// just know someone's gonna want me to take it back out again soooo.....
#ifndef VERSION_DEMO
  //    ^^^^ see, i was right
  #define ANNOYING_HACK_FOR_SIMON		1
#endif


#define FE_MAINMENU			( 1)
#define FE_MAPSCREEN		( 2)
#define FE_MISSIONSELECT	( 3)
#define FE_MISSIONBRIEFING	( 4)
#define FE_LOADSCREEN		( 5)
#define FE_SAVESCREEN		( 6)
#define FE_CONFIG			( 7)
#define FE_CONFIG_VIDEO		( 8)
#define FE_CONFIG_AUDIO		( 9)
#ifdef WANT_A_KEYBOARD_ITEM
#define FE_CONFIG_INPUT_KB  (10)
#endif
#define FE_CONFIG_INPUT_JP  (11)
#define FE_CONFIG_OPTIONS	(13)
#ifdef WANT_AN_EXIT_MENU_ITEM
#define FE_QUIT				(12)
#endif
#define FE_SAVE_CONFIRM		(14)
#ifdef WANT_A_TITLE_SCREEN
#define FE_TITLESCREEN		(15)
#define FE_LANGUAGESCREEN	(16)
#define FE_CHANGE_LANGUAGE	(17)
#endif
#ifdef TARGET_DC
#define FE_VMU_SELECT		(18)
#endif

#ifdef WANT_AN_EXIT_MENU_ITEM
#define FE_NO_REALLY_QUIT	(-1)
#endif
#define FE_BACK				(-2)
#define FE_START			(-3)
#define FE_EDITOR			(-4)
#define FE_CREDITS			(-5)
#ifdef TARGET_DC
#define FE_CHANGE_JOYPAD	(-6)
#endif

#define OT_BUTTON			( 1)
#define OT_BUTTON_L			( 2)
#define OT_SLIDER			( 3)
#define OT_MULTI			( 4)
#define OT_KEYPRESS			( 5)
#define OT_LABEL			( 6)
#define OT_PADPRESS			( 7)
#define OT_RESET			( 8)
#define OT_PADMOVE			( 9)

#define	MC_YN				(CBYTE*)( 1)
#define	MC_SCANNER			(CBYTE*)( 2)
#ifdef TARGET_DC
#define	MC_JOYPAD_MODE		(CBYTE*)( 3)
#define	MC_ANALOGUE_MODE	(CBYTE*)( 4)
#endif

//----------------------------------------------------------------------------
// STRUCTS
//

struct MissionData {
	SLONG	ObjID, GroupID, ParentID, ParentIsGroup;
	SLONG	District;
	SLONG	Type, Flags;
	CBYTE	fn[255], ttl[255], brief[4096];
};

struct RawMenuData {
	UBYTE	Menu;
	UBYTE	Type;
//	CBYTE*	Label;
	SWORD	Label;
	CBYTE*	Choices;
	SLONG	Data;
};

struct MenuData {
	UBYTE	Type;
	UBYTE	LabelID;
	CBYTE*	Label;
	CBYTE*	Choices;
	SLONG	Data;
	UWORD	X,Y;
};

struct MenuStack {
	UBYTE		mode;
	UBYTE		selected;
	SWORD		scroll;
	CBYTE*		title;
};

struct MenuState {
	UBYTE		items;
	UBYTE		selected;
	SWORD		scroll;
	MenuStack	stack[10];
	UBYTE		stackpos;
	SBYTE		mode;
	SWORD		base;
	CBYTE*		title;
};

struct Kibble {
	SLONG	page;
	SLONG	x, y;
	SWORD	dx,dy;
	SWORD	r, t, p, rd, td, pd;
	UBYTE	type, size;
	ULONG	rgb;
};

struct MissionCache {
	CBYTE name[255];
	UBYTE id;
	UBYTE district;
};



//
// This is the order we recommend the missions be played in...
//

CBYTE *suggest_order[] =
{
	"testdrive1a.ucm",
	"FTutor1.ucm",
	"Assault1.ucm",
	"police1.ucm",
	"police2.ucm",
	"Bankbomb1.ucm",
	"police3.ucm",
	"Gangorder2.ucm",
	"police4.ucm",
	"bball2.ucm",
	"wstores1.ucm",
	"e3.ucm",
	"westcrime1.ucm",
	"carbomb1.ucm",
	"botanicc.ucm",
	"Semtex.ucm",
	"AWOL1.ucm",
	"mission2.ucm",
	"park2.ucm",
	"MIB.ucm",
	"skymiss2.ucm",
	"factory1.ucm",
	"estate2.ucm",
	"Stealtst1.ucm",
	"snow2.ucm",
	"Gordout1.ucm",
	"Baalrog3.ucm",
	"Finale1.ucm",

	"Gangorder1.ucm",
	"FreeCD1.ucm",
	"jung3.ucm",

	"fight1.ucm",
	"fight2.ucm",
	"testdrive2.ucm",
	"testdrive3.ucm",

	"Album1.ucm",

	"!"
};





//
// The script... biggest one at the moment is 17k
//

#define SCRIPT_MEMORY (20 * 1024)

CBYTE  loaded_in_script[SCRIPT_MEMORY];
CBYTE *loaded_in_script_read_upto;

void CacheScriptInMemory(CBYTE *script_fname)
{
	//FILE *handle = MF_Fopen(script_fname, "rb");
	// Er... it's a TEXT FILE
	FILE *handle = MF_Fopen(script_fname, "rt");

	ASSERT(handle);

	if (handle)
	{
		int iLoaded = fread(loaded_in_script, 1, SCRIPT_MEMORY, handle);
		ASSERT ( iLoaded < sizeof(loaded_in_script) );
		// Stupid thing doesn't always terminate with a
		// zero, and sometimes writes crap past the end.
		loaded_in_script[iLoaded] = '\0';

		MF_Fclose(handle);
	}
}

void FileOpenScript(void)
{
	loaded_in_script_read_upto = loaded_in_script;
}

CBYTE *LoadStringScript(CBYTE *txt) {
	CBYTE *ptr=txt;

	ASSERT(loaded_in_script_read_upto);

	*ptr=0;
	while (1) {

		*ptr = *loaded_in_script_read_upto++;

		if (*ptr == 0) {
			return txt;
		};
		if (*ptr==10) break;
		ptr++;
	}
	*(++ptr)=0;
	return txt;
}

void FileCloseScript(void)
{
	loaded_in_script_read_upto = NULL;
}


//----------------------------------------------------------------------------
// CONSTANTS
//




RawMenuData raw_menu_data[] = {
	{		FE_MAINMENU,	OT_BUTTON,	X_START,		0,	FE_MAPSCREEN		},
#ifndef VERSION_DEMO
	{				  0,	OT_BUTTON,	X_LOAD_GAME,	0,	FE_LOADSCREEN		},
	{				  0,	OT_BUTTON,	X_SAVE_GAME,	(CBYTE*)1,	FE_SAVESCREEN		},
#endif
	{				  0,	OT_BUTTON,	X_OPTIONS,		0,	FE_CONFIG			},
#if !defined(NDEBUG) && !defined(TARGET_DC)
	// sheer MADNESS!
	{				  0,	OT_BUTTON,	X_LOAD_UCM, 	0,	FE_START			},
	{				  0,	OT_BUTTON,	X_EDITOR,		0,  FE_EDITOR			},
#endif
#ifndef VERSION_DEMO
	{				  0,	OT_BUTTON,	X_CREDITS,		0,	FE_CREDITS			},
#endif
#ifdef WANT_AN_EXIT_MENU_ITEM
	{				  0,	OT_BUTTON,	X_EXIT,			0,	FE_QUIT				},
#endif
#ifdef TARGET_DC
    {     FE_LOADSCREEN,    OT_BUTTON,  X_VMU_SELECT,   0,  FE_VMU_SELECT       },
    {                 0,    OT_BUTTON,  X_CANCEL,       0,  FE_BACK             },

    {     FE_SAVESCREEN,    OT_BUTTON,  X_VMU_SELECT,   0,  FE_VMU_SELECT       },
    {                 0,    OT_BUTTON,  X_CANCEL,       0,  FE_MAPSCREEN        },

    {     FE_VMU_SELECT,    OT_BUTTON,  X_CANCEL,       0,  FE_BACK             },
#else
    {     FE_LOADSCREEN,    OT_BUTTON,  X_CANCEL,       0,  FE_BACK             },
    {     FE_SAVESCREEN,    OT_BUTTON,  X_CANCEL,       0,  FE_MAPSCREEN        },
#endif
#ifdef TARGET_DC
	{	FE_SAVE_CONFIRM,    OT_LABEL,	X_OVERWRITE_SURE,0,	0					},
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_MAPSCREEN     	},
	{				  0,	OT_BUTTON,	X_CANCEL,		0,	FE_BACK				},
#else
	{	FE_SAVE_CONFIRM,    OT_LABEL,	X_ARE_YOU_SURE ,0,	0					},
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_MAPSCREEN     	},
	{				  0,	OT_BUTTON,	X_CANCEL,		0,	FE_BACK				},
#endif

#ifdef TARGET_DC
	{		  FE_CONFIG,	OT_BUTTON,	X_GENERAL,		0,	FE_CONFIG_OPTIONS	},
#else
	{		  FE_CONFIG,	OT_BUTTON,	X_OPTIONS,		0,	FE_CONFIG_OPTIONS	},
#endif
	{		          0,	OT_BUTTON,	X_GRAPHICS,		0,	FE_CONFIG_VIDEO		},
	{				  0,	OT_BUTTON,	X_SOUND,		0,	FE_CONFIG_AUDIO		},
#ifdef WANT_A_KEYBOARD_ITEM
	{				  0,	OT_BUTTON,	X_KEYBOARD,		0,	FE_CONFIG_INPUT_KB	},
#endif
	{				  0,	OT_BUTTON,	X_JOYPAD,		0,	FE_CONFIG_INPUT_JP	},
#ifdef TARGET_DC
	{				  0,	OT_BUTTON,	X_CHANGE_JOYPAD,0,	FE_CHANGE_JOYPAD	},
#endif
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK				},
	{	FE_CONFIG_AUDIO,	OT_SLIDER,	X_VOLUME,		0,	128					},
	{				  0,	OT_SLIDER,	X_AMBIENT,		0,	128					},
	{				  0,	OT_SLIDER,	X_MUSIC,		0,	128					},
#ifdef TARGET_DC
	{				  0,	OT_MULTI,	X_STEREO,	MC_YN,	1					},
#endif
#ifdef M_SOUND
#ifdef ALLOW_DANGEROUS_OPTIONS
	{				  0,	 OT_MULTI,	X_DRIVERS,	    0,	128					},
#endif
#endif
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK				},
#ifdef WANT_A_KEYBOARD_ITEM
	{FE_CONFIG_INPUT_KB,  OT_KEYPRESS,	X_LEFT,			0,	0,					},
	{				  0,  OT_KEYPRESS,	X_RIGHT,		0,	0,					},
	{				  0,  OT_KEYPRESS,	X_FORWARDS,		0,	0,					},
	{				  0,  OT_KEYPRESS,	X_BACKWARDS,	0,	0,					},
	{				  0,  OT_KEYPRESS,	X_PUNCH,		0,	0,					},
	{				  0,  OT_KEYPRESS,	X_KICK,			0,	0,					},
	{				  0,  OT_KEYPRESS,	X_ACTION,		0,	0,					},
//	{				  0,  OT_KEYPRESS,	X_RUN,			0,	0,					},
	{				  0,  OT_KEYPRESS,	X_JUMP,			0,	0,					},
#ifdef WANT_A_START_JOYSTICK_ITEM
	{				  0,  OT_KEYPRESS,	X_START,		0,	0,					},
#endif
	{				  0,  OT_KEYPRESS,	X_SELECT,		0,	0,					},
	{				  0,	 OT_LABEL,	X_CAMERA,		0,	0,					},
	{				  0,  OT_KEYPRESS,	X_JUMP,			0,	0,					},
	{				  0,  OT_KEYPRESS,	X_LEFT,			0,	0,					},
	{				  0,  OT_KEYPRESS,	X_RIGHT,		0,	0,					},
	{				  0,  OT_KEYPRESS,	X_LOOK_AROUND,	0,	0,					},
	{				  0,     OT_RESET,  X_RESET_DEFAULT,0,  0,					},
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK,			},
#endif


#ifdef TARGET_DC

	{FE_CONFIG_INPUT_JP,     OT_MULTI,	X_PAD_MODE,		MC_JOYPAD_MODE,	0,		},
	{                 0,     OT_LABEL,	X_PAD_CUSTOM,	0,	0,					},
	{				  0,  OT_PADPRESS,	X_PUNCH,		0,	0,					},
	{                 0,  OT_PADPRESS,	X_KICK,			0,	0,					},
	{				  0,  OT_PADPRESS,	X_JUMP,			0,	0,					},
	{				  0,  OT_PADPRESS,	X_ACTION,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_PAD_WALK,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_SELECT,		0,	0,					},
	{				  0,	 OT_LABEL,	X_CAMERA,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_PAD_MODE,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_LEFT,			0,	0,					},
	{				  0,  OT_PADPRESS,	X_RIGHT,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_LOOK_AROUND,	0,	0,					},
	{				  0,     OT_RESET,  X_RESET_DEFAULT,0,  0,					},

	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK,			},

#else //#ifdef TARGET_DC

	{FE_CONFIG_INPUT_JP,  OT_PADPRESS,	X_KICK,			0,	0,					},
	{				  0,  OT_PADPRESS,	X_PUNCH,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_JUMP,			0,	0,					},
	{				  0,  OT_PADPRESS,	X_ACTION,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_RUN,			0,	0,					},
#ifdef WANT_A_START_JOYSTICK_ITEM
	{				  0,  OT_PADPRESS,	X_START,		0,	0,					},
#endif
	{				  0,  OT_PADPRESS,	X_SELECT,		0,	0,					},
	{				  0,	 OT_LABEL,	X_CAMERA,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_JUMP_CAM,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_LEFT,			0,	0,					},
	{				  0,  OT_PADPRESS,	X_RIGHT,		0,	0,					},
	{				  0,  OT_PADPRESS,	X_LOOK_AROUND,	0,	0,					},
	{				  0,     OT_RESET,  X_RESET_DEFAULT,0,  0,					},

	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK,			},

#endif //#else //#ifdef TARGET_DC


//	{	FE_CONFIG_VIDEO,	OT_SLIDER,	X_DETAIL,		0,	128,				},
#ifdef ALLOW_DANGEROUS_OPTIONS
	{   FE_CONFIG_VIDEO,     OT_LABEL,  X_GRAPHICS,     0,  1,                  },
	{                 0,     OT_MULTI,  X_RESOLUTION,   0,  1,                  },
	{                 0,     OT_MULTI,  X_DRIVERS,	    0,  1,                  },
	{				  0,	 OT_MULTI,  X_COLOUR_DEPTH, 0,  1,					},
#endif
//	{                 0,     OT_LABEL,  X_DETAIL,	    0,  1,                  },
#ifdef TARGET_DC
	{   FE_CONFIG_VIDEO,     OT_MULTI,  X_SHADOWS,	MC_YN,  1,                  },
//	{                 0,     OT_MULTI,  X_PUDDLES,	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_DIRT, 	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_MIST, 	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_RAIN, 	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_SKYLINE,	MC_YN,  1,                  },
//	{				  0,	 OT_MULTI,  X_CRINKLES, MC_YN,  1,					},
//	{				  0,	 OT_LABEL,	X_REFLECTIONS  ,0,	0					},
//	{                 0,     OT_MULTI,  X_MOON,		MC_YN,  1,				    },
//	{                 0,     OT_MULTI,  X_PEOPLE,	MC_YN,  1,					},
//	{				  0,     OT_LABEL,  X_TEXTURE_MAP  ,0,  0,                  },
//	{                 0,     OT_MULTI,  X_PERSPECTIVE,MC_YN,1,                  },
//	{                 0,     OT_MULTI,  X_BILINEAR, MC_YN,  1,                  },
//	{				  0,	OT_SLIDER,	"Gamma",		0,	128,				},
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK,			},
#else
	{   FE_CONFIG_VIDEO,     OT_MULTI,  X_STARS,	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_SHADOWS,	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_PUDDLES,	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_DIRT, 	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_MIST, 	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_RAIN, 	MC_YN,  1,                  },
	{                 0,     OT_MULTI,  X_SKYLINE,	MC_YN,  1,                  },
	{				  0,	 OT_MULTI,  X_CRINKLES, MC_YN,  1,					},
	{				  0,	 OT_LABEL,	X_REFLECTIONS  ,0,	0					},
	{                 0,     OT_MULTI,  X_MOON,		MC_YN,  1,				    },
	{                 0,     OT_MULTI,  X_PEOPLE,	MC_YN,  1,					},
	{				  0,     OT_LABEL,  X_TEXTURE_MAP  ,0,  0,                  },
	{                 0,     OT_MULTI,  X_PERSPECTIVE,MC_YN,1,                  },
	{                 0,     OT_MULTI,  X_BILINEAR, MC_YN,  1,                  },
//	{				  0,	OT_SLIDER,	"Gamma",		0,	128,				},
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK,			},
#endif
#ifdef TARGET_DC
	{ FE_CONFIG_OPTIONS,	 OT_MULTI,	X_TRACK,MC_SCANNER,  0					},
	{				  0,	 OT_MULTI,	X_CONTROLS,MC_ANALOGUE_MODE,	0    	},
	{				  0,	 OT_MULTI,	X_VIBRATION,MC_YN,	0    	            },
	{				  0,	 OT_MULTI,	X_VIBRATION_ENG,MC_YN,	0    	            },
	{				  0,	OT_PADMOVE,	X_PANEL,		0,	0    				},
#ifdef WANT_A_TITLE_SCREEN
	{				  0,	 OT_BUTTON,	X_LANGUAGE,		0,  FE_LANGUAGESCREEN	},
#endif
#else
	{ FE_CONFIG_OPTIONS,	 OT_LABEL,	X_SCANNER,      0,  0					},
	{				  0,	 OT_MULTI,	X_TRACK,MC_SCANNER,	0       			},
#endif
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_BACK,			},
#ifdef WANT_AN_EXIT_MENU_ITEM
	{			FE_QUIT,	 OT_LABEL,	X_ARE_YOU_SURE ,0,	0					},
	{				  0,	OT_BUTTON,	X_OKAY,			0,	FE_NO_REALLY_QUIT	},
	{				  0,	OT_BUTTON,	X_CANCEL,		0,	FE_BACK				},
#endif

#ifdef WANT_A_TITLE_SCREEN
	{ FE_LANGUAGESCREEN,	OT_BUTTON,	X_ENGLISH,		0,	FE_CHANGE_LANGUAGE	},
	{				  0,	OT_BUTTON,	X_FRENCH,		0,	FE_CHANGE_LANGUAGE	},
	{	 FE_TITLESCREEN,	OT_BUTTON,	X_GAME_NAME,	0,	FE_MAINMENU         },
#endif

	{				 -1,			0,				    0,	0					},
};

CBYTE menu_choice_yesno[20];// = { "no\0yes" };
CBYTE menu_choice_scanner[255];
#ifdef TARGET_DC
CBYTE menu_choice_joypad_mode[50];
CBYTE menu_choice_analogue_mode[50];
#ifdef WANT_A_TITLE_SCREEN
CBYTE menu_choice_language[50];
#endif
#endif

#if TARGET_DC
// Underlines, not spaces.
CBYTE* menu_back_names[] = { "title_leaves1.tga", "title_rain1.tga",
	   					     "title_snow1.tga", "title_blood1.tga" };
CBYTE* menu_map_names[]  = { "map_leaves_darci.tga", "map_rain_darci.tga",
						     "map_snow_darci.tga", "map_blood_darci.tga" };
CBYTE* menu_brief_names[]= { "briefing_leaves_darci.tga", "briefing_rain_darci.tga",
						     "briefing_snow_darci.tga", "briefing_blood_darci.tga" };
CBYTE* menu_config_names[]= { "config_leaves.tga", "config_rain.tga",
						     "config_snow.tga", "config_blood.tga" };
#else
CBYTE* menu_back_names[] = { "title leaves1.tga", "title rain1.tga",
	   					     "title snow1.tga", "title blood1.tga" };
CBYTE* menu_map_names[]  = { "map leaves darci.tga", "map rain darci.tga",
						     "map snow darci.tga", "map blood darci.tga" };
CBYTE* menu_brief_names[]= { "briefing leaves darci.tga", "briefing rain darci.tga",
						     "briefing snow darci.tga", "briefing blood darci.tga" };
CBYTE* menu_config_names[]= { "config leaves.tga", "config rain.tga",
						     "config snow.tga", "config blood.tga" };
#endif

CBYTE frontend_fonttable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$*-()[]\\/?����������";

/*ULONG FRONTEND_leaf_colours[4] =
	{
		0x332d1d,
		0x243224,
		0x123320,
		0x332f07
	};*/
ULONG FRONTEND_leaf_colours[4] =
	{
		0x665a3a,
		0x486448,
		0x246640,
		0x665E0E
	};



//----------------------------------------------------------------------------
// GLOBALS
//

MenuData  menu_data[50]; // max menu items...
MenuState menu_state;
CBYTE	  menu_buffer[2048];
BOOL	  grabbing_key=0;
BOOL	  grabbing_pad=0;
BOOL	  m_bMovingPanel = FALSE;
#ifdef TARGET_DC
// Dynamically allocated instead.
Kibble	  *kibble = NULL;
#else
Kibble	  kibble[512];
#endif
UBYTE	  kibble_off[512];
SLONG	  fade_state=0;
UBYTE	  fade_mode=1;
SLONG	  fade_rgb=0x000000;
SBYTE	  menu_mode_queued=0;
UBYTE	  menu_theme=1;
UBYTE	  menu_thrash=0; // cunningly thrash the stack...
SWORD	  districts[40][3]; // x, y, id
SWORD	  district_count=0;
SWORD	  district_selected=0;
SWORD     district_flash=0;
UBYTE	  district_valid[40];
SWORD	  mission_count=0;
SWORD	  mission_selected=0;
UBYTE	  mission_hierarchy[60];
MissionCache mission_cache[60];
#ifdef	NDEBUG
UBYTE	  complete_point=0;
#else
UBYTE	  complete_point=0;
#endif
UBYTE	  mission_launch=0;
UBYTE	  previous_mission_launch=0;
BOOL	  ragepro_sucks=0;
BOOL	  cheating=0;
SLONG	  MidX=0, MidY;
float	  ScaleX, ScaleY; // bwahahaha... and lo! the floats creep in! see the extent of my evil powers! muahahaha!  *cough*  er...
#ifdef DEBUG
// Allow saves from init, just so I can test the damn things.
UBYTE	  AllowSave=1;
#else
UBYTE	  AllowSave=0;
#endif
SLONG	  CurrentVidMode=0;
UBYTE	  CurrentBitDepth=16;
UBYTE	  save_slot;
UBYTE	  bonus_this_turn = 0;
UBYTE	  bonus_state = 0;

CBYTE	  the_script_file[MAX_PATH];
#define MISSION_SCRIPT	the_script_file

UBYTE	  IsEnglish=0;

char *pcSpeechLanguageDir = "talk2\\";

DWORD dwAutoPlayFMVTimeout = 0;


#ifdef TARGET_DC
int iNextSuggestedMission = 0;
int g_iLevelNumber;
#endif

// Kludge!
SLONG	  GammaIndex;

// That's not a kludge. THIS is a kludge.
bool m_bGoIntoSaveScreen = FALSE;


BOOL bCanChangeJoypadButtons = FALSE;


#ifdef TARGET_DC
#define QUICK_INFO_MAX_LEN 40
char pcQuickInfo[QUICK_INFO_MAX_LEN+1] = "";
DWORD dwQuickInfotimeGetTimeExpires = 0;
bool bQuickInfoReplaceWithSegasMadMessage = FALSE;
#endif


//
// The three screen images for each theme are now preloaded.
//


#ifdef TARGET_DC
// If you're short of memory, ditch one.
//#define ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB defined
#endif


#if USE_COMPRESSED_BACKGROUNDS

CompressedBackground screenfull_back = NULL;
CompressedBackground screenfull_map = NULL;
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
CompressedBackground screenfull_config = NULL;
#endif
CompressedBackground screenfull_brief = NULL;
#ifdef WANT_A_TITLE_SCREEN
CompressedBackground screenfull_title = NULL;
#endif


//
// The surface we use to do swipes\fades.
//

CompressedBackground screenfull = NULL;

#else

LPDIRECTDRAWSURFACE4 screenfull_back = NULL;
LPDIRECTDRAWSURFACE4 screenfull_map = NULL;
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
LPDIRECTDRAWSURFACE4 screenfull_config = NULL;
#endif
LPDIRECTDRAWSURFACE4 screenfull_brief = NULL;
#ifdef WANT_A_TITLE_SCREEN
LPDIRECTDRAWSURFACE4 screenfull_title = NULL;
#endif

//
// The surface we use to do swipes\fades.
//

LPDIRECTDRAWSURFACE4 screenfull = NULL;

#endif






//----------------------------------------------------------------------------
// FUNCTIONS
//

//--- tools ---

#ifdef TARGET_DC

// TRUE = normal behaviour - reset on door open.
// FALSE = ignore door resets.
void ChangeDoorResetStatus ( bool bAllowReset )
{
	// And unlock the disk door reset thingie.
	HANDLE handle = CreateFile(TEXT("\\Device\\CDROM0"), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
    if ( handle == INVALID_HANDLE_VALUE )
	{
		ASSERT ( FALSE );
		return;
	}

	DWORD dwDummy;
	SEGACD_DOOR_BEHAVIOR sdb;
	if ( bAllowReset )
	{
		sdb.dwBehavior = SEGACD_DOOR_REBOOT;
	}
	else
	{
		sdb.dwBehavior = SEGACD_DOOR_NOTIFY_APP;
	}
	DeviceIoControl ( handle, IOCTL_SEGACD_SET_DOOR_BEHAVIOR,
						&sdb, sizeof ( sdb ),
						NULL, 0,
						&dwDummy, NULL );

	CloseHandle ( handle );
}

#endif



#ifndef TARGET_DC
// define this to save backgrounds out again in DC file format.
#define SAVE_MY_BACKGROUNDS_PLEASE_BOB defined
#endif


#if USE_COMPRESSED_BACKGROUNDS

void	FRONTEND_scr_add(CompressedBackground *screen, UBYTE *image_data)
{
	// First convert to 565 format.
	WORD *pwTemp = (WORD *)MemAlloc ( 640 * 480 * 2 );
	ASSERT ( pwTemp != NULL );
	UBYTE *pbSrc = image_data;

	for ( int i = 0; i < 640*480; i++ )
	{
		// From 24-bit RGB to 565.
		*pwTemp  = ( ( *pbSrc++ ) & 0xf8 ) << 8;
		*pwTemp |= ( ( *pbSrc++ ) & 0xfc ) << 3;
		*pwTemp |= ( ( *pbSrc++ ) & 0xf8 ) >> 3;
		pwTemp++;
	}

	// See how big it is, compressed.
	int iSize = PackBackground ( NULL, pwTemp );

	TRACE ( "FRONTEND_scr_add: Original 565 0x%x, now 0x%x, saving of %i percent\n", 640*480*2, iSize, (100*iSize)/(640*480*2) );

	if (*screen)
	{
		MemFree ( *screen );
		*screen = NULL;
	}

	*screen = MemAlloc ( iSize );

	PackBackground ( (UBYTE *)*screen, pwTemp );

	// And free the 565 version.
	MemFree ( (void *)pwTemp );
}

#else //#if USE_COMPRESSED_BACKGROUNDS

void	FRONTEND_scr_add(LPDIRECTDRAWSURFACE4 *screen, UBYTE *image_data)
{
	DDSURFACEDESC2 back;
	DDSURFACEDESC2 mine;

	// Remember this if we have to reload.

	// Create a mirror surface to the back buffer.

	InitStruct(back);

	the_display.lp_DD_BackSurface->GetSurfaceDesc(&back);



	//
	// Create the mirror surface in system memory.
	//

	InitStruct(mine);

	mine.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	mine.dwWidth         = back.dwWidth;
	mine.dwHeight        = back.dwHeight;
	mine.ddpfPixelFormat = back.ddpfPixelFormat;
#ifdef TARGET_DC
	// No software blits on DC, but I'll do it myself.
	mine.ddsCaps.dwCaps	 = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
#else
	mine.ddsCaps.dwCaps	 = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;



#ifdef SAVE_MY_BACKGROUNDS_PLEASE_BOB
	// Force the format to be 640x480x565, because that's what the DC needs.
	mine.dwWidth = 640;
	mine.dwHeight = 480;
	mine.ddpfPixelFormat.dwRBitMask = 0xf800;
	mine.ddpfPixelFormat.dwGBitMask = 0x07e0;
	mine.ddpfPixelFormat.dwBBitMask = 0x001f;
#endif


#endif

	HRESULT result = the_display.lp_DD4->CreateSurface(&mine, screen, NULL);


	if (FAILED(result))
	{
		// Probably out of memory.
		ASSERT ( FALSE );
		*screen = NULL;
		return;
	}

	// Copy the image into the surface...

extern void CopyBackground(UBYTE* image_data, IDirectDrawSurface4* surface);

	CopyBackground(image_data, *screen);

	return;
}
#endif //#else //#if USE_COMPRESSED_BACKGROUNDS





#if USE_COMPRESSED_BACKGROUNDS

#ifndef TARGET_DC
#error Compressed backgrounds only work on the DC at the moment.
#endif

void	FRONTEND_scr_img_load_into_screenfull(CBYTE *name, CompressedBackground *screen)
{
	MFFileHandle	image_file;
	SLONG	height;
	CBYTE	fname[200];
	UBYTE*	image;
	UBYTE  *image_data;

	//if (screenfull) FRONTEND_scr_del();

	if ( *screen != NULL )
	{
		MemFree ( *screen );
		*screen = NULL;
	}

	// Look for the preprocessed version.
	sprintf(fname,"%sdata\\%s",DATA_DIR,name);
	// Change the extension.
	char *pchTemp = fname + strlen ( fname ) - 4;
	ASSERT ( pchTemp[0] == '.' );
	ASSERT ( pchTemp[1] == 't' );
	ASSERT ( pchTemp[2] == 'g' );
	ASSERT ( pchTemp[3] == 'a' );
	pchTemp[1] = 'b';
	pchTemp[2] = 'g';
	pchTemp[3] = 's';

	// .BGS - BackGroundScreen.

	MFFileHandle handle = FileOpen ( fname );
	if ( handle == FILE_OPEN_ERROR )
	{
		ASSERT ( FALSE );
	}
	else
	{
		SLONG slSize = FileSize ( handle );

		*screen = MemAlloc ( slSize );
		if ( *screen == NULL )
		{
			// Out of memory
			TRACE ( "Out of memory for screen %s\n", fname );
			return;
		}

		SLONG res = FileRead ( handle, *screen, slSize );
		ASSERT ( res == slSize );

		FileClose ( handle );
	}
}

#else //#if USE_COMPRESSED_BACKGROUNDS

void	FRONTEND_scr_img_load_into_screenfull(CBYTE *name, LPDIRECTDRAWSURFACE4 *screen)
{
	MFFileHandle	image_file;
	SLONG	height;
	CBYTE	fname[200];
	UBYTE*	image;
	UBYTE  *image_data;

	//if (screenfull) FRONTEND_scr_del();

	*screen = NULL;


	sprintf(fname,"%sdata\\%s",DATA_DIR,name);

	image_data =	(UBYTE*)MemAlloc(640*480*3);

	if(image_data)
	{

		image_file	=	FileOpen(fname);
		if(image_file>0)
		{
			TRACE ( "Loading background <%s>\n", fname );
			FileSeek(image_file,SEEK_MODE_BEGINNING,18);
			image	=	image_data +(640*479*3);
			for(height=480;height;height--,image-=(640*3))
			{
				FileRead(image_file,image,640*3);
			}
			FileClose(image_file);
		}
		else
		{
			TRACE ( "Unable to load background <%s>\n", fname );
		}

		FRONTEND_scr_add(screen, image_data);

		MemFree(image_data);


#ifndef TARGET_DC
#ifdef SAVE_MY_BACKGROUNDS_PLEASE_BOB
		// Save the screen out - should already be in 565.


		// First see if we need to make a copy, or if it already exists.
		sprintf(fname,"%sdata\\%s",DATA_DIR,name);
		// Change the extension.
		char *pchTemp = fname + strlen ( fname ) - 4;
		ASSERT ( pchTemp[0] == '.' );
		ASSERT ( pchTemp[1] == 't' );
		ASSERT ( pchTemp[2] == 'g' );
		ASSERT ( pchTemp[3] == 'a' );
		pchTemp[1] = 'b';
		pchTemp[2] = 'g';
		pchTemp[3] = 's';
		// .BGS - BackGroundScreen. Wank, eh? But there's no format, it's just 640x480, 565 pixels, raw.


		// If it already exists, don't bother.
		MFFileHandle handle = FileCreate ( fname, FALSE );
		if ( handle == FILE_CREATION_ERROR )
		{
			// Probably already exists.
			TRACE ( "Couldn't save Dreamcast background <%s> - might already exist\n", fname );
			return;
		}




		// First make a linear-mem copy.
		WORD *pwTemp = (WORD *)MemAlloc ( 640 * 480 * 2 );
		ASSERT ( pwTemp != NULL );

		DDSURFACEDESC2 mine;
		InitStruct(mine);
		HRESULT result = (*screen)->Lock ( NULL, &mine, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL );
		ASSERT ( result == DD_OK );

		char *src = (char *)( mine.lpSurface );
		char *dst = (char *)pwTemp;
		for ( int i = 0; i < 480; i++ )
		{
			memcpy ( dst, src, 640 * 2 );
			src += mine.lPitch;
			dst += 640 * 2;
		}

		(*screen)->Unlock ( NULL );



		// See how big it is, compressed.
extern int PackBackground ( UBYTE* image_data, WORD *surface );
		int iSize = PackBackground ( NULL, pwTemp );

		TRACE ( "FRONTEND_scr_img_load_into_screenfull: Original 565 0x%x, now 0x%x, saving of %i percent\n", 640*480*2, iSize, (100*iSize)/(640*480*2) );

		void *pvCompressed = MemAlloc ( iSize );
		ASSERT ( pvCompressed != NULL );

		PackBackground ( (UBYTE *)pvCompressed, pwTemp );

		// And free the linear version.
		MemFree ( (void *)pwTemp );



		int res = FileWrite ( handle, pvCompressed, iSize );
		ASSERT ( res == iSize );
		FileClose ( handle );

		TRACE ( "Saved Dreamcast background <%s>\n", fname );

		MemFree ( pvCompressed );

#endif
#endif





	}
	else
	{
		TRACE ( "No memory to load background <%s>\n", fname );
	}
}


#endif //#else //#if USE_COMPRESSED_BACKGROUNDS




void FRONTEND_scr_unload_theme()
{
	the_display.lp_DD_Background_use_instead = NULL;


#ifdef TARGET_DC
	// Recover moderately gracefully from having your bollocks stamped on.
#ifdef WANT_A_TITLE_SCREEN
	if ( ( screenfull_title != NULL ) && ( screenfull_title == screenfull_back ) )
	{
		ASSERT ( FALSE );
		screenfull_title = NULL;
	}
#endif
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	if ( ( screenfull_config != NULL ) && ( screenfull_config == screenfull_back ) )
	{
		ASSERT ( FALSE );
		screenfull_config = NULL;
	}
#endif
	if ( ( screenfull_brief != NULL ) && ( screenfull_brief == screenfull_map ) )
	{
		ASSERT ( FALSE );
		screenfull_brief = NULL;
	}
	if ( ( screenfull_map != NULL ) && ( screenfull_map == screenfull_back ) )
	{
		ASSERT ( FALSE );
		screenfull_map = NULL;
	}
#endif


#if USE_COMPRESSED_BACKGROUNDS

	if (screenfull_back  ) {MemFree(screenfull_back)  ; screenfull_back   = NULL;}
	if (screenfull_map   ) {MemFree(screenfull_map)   ; screenfull_map    = NULL;}
	if (screenfull_brief ) {MemFree(screenfull_brief) ; screenfull_brief  = NULL;}
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	if (screenfull_config) {MemFree(screenfull_config); screenfull_config = NULL;}
#endif
#ifdef WANT_A_TITLE_SCREEN
	if (screenfull_title) {MemFree(screenfull_title); screenfull_title = NULL;}
#endif


#else

	if (screenfull_back  ) {screenfull_back  ->Release(); screenfull_back   = NULL;}
	if (screenfull_map   ) {screenfull_map   ->Release(); screenfull_map    = NULL;}
	if (screenfull_brief ) {screenfull_brief ->Release(); screenfull_brief  = NULL;}
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	if (screenfull_config) {screenfull_config->Release(); screenfull_config = NULL;}
#endif
#ifdef WANT_A_TITLE_SCREEN
	if (screenfull_title) {screenfull_title->Release(); screenfull_title = NULL;}
#endif

#endif

	screenfull = NULL;

}


void FRONTEND_scr_new_theme(
		CBYTE *fname_back,
		CBYTE *fname_map,
		CBYTE *fname_brief,
		CBYTE *fname_config)
{
	SLONG last = 1;

	// Stop all music while we load stuff from disk.
	stop_all_fx_and_music(TRUE);




	if (the_display.lp_DD_Background_use_instead == screenfull_back  ) {last = 1;}
	if (the_display.lp_DD_Background_use_instead == screenfull_map   ) {last = 2;}
	if (the_display.lp_DD_Background_use_instead == screenfull_brief ) {last = 3;}
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	if (the_display.lp_DD_Background_use_instead == screenfull_config) {last = 4;}
#endif
#ifdef WANT_A_TITLE_SCREEN
	if (the_display.lp_DD_Background_use_instead == screenfull_title) {last = 5;}
#endif


	FRONTEND_scr_unload_theme();

	FRONTEND_scr_img_load_into_screenfull(fname_back  , &screenfull_back  );
	FRONTEND_scr_img_load_into_screenfull(fname_map   , &screenfull_map   );
	FRONTEND_scr_img_load_into_screenfull(fname_brief , &screenfull_brief );
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	FRONTEND_scr_img_load_into_screenfull(fname_config, &screenfull_config);
#endif
#ifdef WANT_A_TITLE_SCREEN

	// Always the same - doesn't change with theme.
	// Does change with nationality though!
extern LPSTR lpszGlobalArgs;
	if ( NULL != strstr ( lpszGlobalArgs, "NO_PAL" ) )
	{
		// An NTSC build, so load the Yank screen instead.
		FRONTEND_scr_img_load_into_screenfull("DCtitlepage_us.tga", &screenfull_title);
	}
	else
	{
		FRONTEND_scr_img_load_into_screenfull("DCtitlepage_uk.tga", &screenfull_title);
	}
#endif


#ifdef TARGET_DC
	if ( screenfull_map == NULL )
	{
		// Shit - out of memory. Panic.
		ASSERT ( FALSE );
		screenfull_map = screenfull_back;
	}
	if ( screenfull_brief == NULL )
	{
		// Shit - out of memory. Panic.
		ASSERT ( FALSE );
		screenfull_brief = screenfull_map;
	}
#ifndef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	if ( screenfull_config == NULL )
	{
		// Shit - out of memory. Panic.
		ASSERT ( FALSE );
		screenfull_config = screenfull_back;
	}
#endif
#ifdef WANT_A_TITLE_SCREEN
	if ( screenfull_title == NULL )
	{
		// Shit - out of memory. Panic.
		ASSERT ( FALSE );
		screenfull_title = screenfull_back;
	}
#endif
#endif

	switch(last)
	{
		case 1: the_display.lp_DD_Background_use_instead = screenfull_back;   break;
		case 2:	the_display.lp_DD_Background_use_instead = screenfull_map;	  break;
		case 3:	the_display.lp_DD_Background_use_instead = screenfull_brief;  break;
#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
		// On DC, use the normal background, then draw an alpha poly over it.
		// FIXME - not actually drawing the alpha poly yet.
		case 4:	the_display.lp_DD_Background_use_instead = screenfull_back; break;
#else
		case 4:	the_display.lp_DD_Background_use_instead = screenfull_config; break;
#endif
#ifdef WANT_A_TITLE_SCREEN
		case 5:	the_display.lp_DD_Background_use_instead = screenfull_title; break;
#endif
		default:
			ASSERT ( FALSE );
			break;
	}


	// And then restart the music.
	MUSIC_mode(MUSIC_MODE_FRONTEND);

}


void FRONTEND_restore_screenfull_surfaces(void)
{
	FRONTEND_scr_new_theme(
		menu_back_names  [menu_theme],
		menu_map_names   [menu_theme],
		menu_brief_names [menu_theme],
		menu_config_names[menu_theme]);
}



#ifdef TARGET_DC



VMU_Screen *pvmuscreenAmmo = NULL;
VMU_Screen *pvmuscreenMFLogo = NULL;
VMU_Screen *pvmuscreenPressStart = NULL;
VMU_Screen *pvmuscreenSaved = NULL;
VMU_Screen *pvmuscreenUCLogo = NULL;
VMU_Screen *pvmuscreenWait = NULL;



// Shows the given screen.
// If the screen is NULL, it shops the default screen.
// It will only show the default screen every two seconds or so,
// so a displayed screen will be replaced in two seconds by the default screen,
// or immediately by a non-default screen.
#define SEND_VMU_PICCIE_EVERY 3000
void FRONTEND_show_VMU_screen ( VMU_Screen *screen )
{
	static DWORD dwLastTimeWeSentScreenToVMU = 0;
	static int iLastDefaultScreen = 0;

	if ( screen == NULL )
	{
		if ( ( timeGetTime() - dwLastTimeWeSentScreenToVMU ) < SEND_VMU_PICCIE_EVERY )
		{
			// Nope - don't do it yet.
			return;
		}

		// Pick a default screen.
		iLastDefaultScreen++;
		if ( iLastDefaultScreen >= 2 )
		{
			iLastDefaultScreen = 0;
		}

		switch ( iLastDefaultScreen )
		{
		case 0:
			screen = pvmuscreenMFLogo;
			break;
		case 1:
			screen = pvmuscreenUCLogo;
			break;
		}
	}

	dwLastTimeWeSentScreenToVMU = timeGetTime();

	WriteLCDScreenToCurrentController ( screen );
}




void FRONTEND_MakeQuickMessage ( char *pcText, int iMilliSecsToShowFor, bool bReplaceWithSegasMadMessage = FALSE )
{
	strncpy ( pcQuickInfo, pcText, QUICK_INFO_MAX_LEN );
	pcQuickInfo[QUICK_INFO_MAX_LEN] = '\0';
	dwQuickInfotimeGetTimeExpires = timeGetTime() + iMilliSecsToShowFor;
	bQuickInfoReplaceWithSegasMadMessage = bReplaceWithSegasMadMessage;
}

#endif



void	FRONTEND_ParseMissionData(CBYTE *text, CBYTE version, MissionData *mdata) {
	UWORD a,n;
	switch(version) {
	case 2:
		sscanf(text,"%d : %d : %d : %d : %d : %s : *%d : %*d : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup,
			&mdata->Type, mdata->fn, mdata->ttl);
		mdata->Flags=0; mdata->District=-1; n=9;
		break;
	case 3:
		sscanf(text,"%d : %d : %d : %d : %d : %d : %s : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup,
			&mdata->Type, &mdata->District, mdata->fn, mdata->ttl);
		mdata->Flags=0; n=8;
		break;
	case 4:
		sscanf(text,"%d : %d : %d : %d : %d : %d : %d : %s : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup,
			&mdata->Type, &mdata->Flags, &mdata->District, mdata->fn, mdata->ttl);
		n=9;
		break;
	default:
		sscanf(text,"%d : %d : %d : %d : %d : %s : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup,
			&mdata->Type, mdata->fn, mdata->ttl);
		mdata->Flags=0; mdata->District=-1; n=7;
	}
	for (a=0;a<n;a++)
	{
		text=strchr(text,':')+1;
	}
	strcpy(mdata->brief,text+1);

	// Change all @ signs to \r (linefeeds). Are we ugly yet mummy?
	char *pch = mdata->brief;
	while ( *pch != '\0' )
	{
		if ( *pch == '@' )
		{
			*pch = '\r';
		}
		pch++;
	}

	text=mdata->brief+strlen(mdata->brief)-2;
	if (*text==13) *text=0;
}

#ifndef TARGET_DC
CBYTE* FRONTEND_LoadString(MFFileHandle &file, CBYTE *txt) {
	CBYTE *ptr=txt;

	*ptr=0;
	while (1) {
		if (FileRead(file,ptr,1)==FILE_READ_ERROR) {
			*ptr=0; return txt;
		};
		if (*ptr==10) break;
		ptr++;
	}
	*(++ptr)=0;
	return txt;
}

void FRONTEND_SaveString(MFFileHandle &file, CBYTE *txt) {
	CBYTE *ptr=txt;
	CBYTE crlf[] = { 13, 10};

	FileWrite(file,txt,strlen(txt));
	FileWrite(file,crlf,2);
}
#endif



SLONG FRONTEND_AlterAlpha(SLONG rgb, SWORD add, SBYTE shift) {

#ifdef TARGET_DC
	// Instead of darker/lighter, we change size.
	return rgb;

#else

	SLONG alpha=rgb>>24;
	alpha<<=shift;
	alpha+=add;
	if (alpha>0xff) alpha=0xff;
	if (alpha<0) alpha=0;
	rgb&=0xffffff;
	return rgb|(alpha<<24);

#endif

}


// Recenters (vertically) whatever menu has been put down.
void FRONTEND_recenter_menu ( void )
{
	MenuData *md=menu_data;
	int iY = 0;
	for ( int i = 0; i < menu_state.items; i++ )
	{
		md->Y = iY;
		md++;
		iY += 50;
	}

	menu_state.base = ( 480 - menu_state.items * 50 ) >> 1;
	if ( menu_state.base < 100 )
	{
		menu_state.base = 100;
	}
	menu_state.scroll = 0;
}


#ifdef TARGET_DC
ULONG	FRONTEND_fix_rgb(ULONG rgb, BOOL sel)
{
	rgb=fade_rgb;
	if (sel) rgb=FRONTEND_AlterAlpha(rgb,0,1);
	return rgb;
}
#else
ULONG	FRONTEND_fix_rgb(ULONG rgb, BOOL sel)
{
	if (ragepro_sucks) {	// but the Rage Pro sucks, even if you don't have one in your machine
		rgb>>=24;
		if (sel) rgb<<=1;
		rgb|=(rgb<<8)|(rgb<<16);
		rgb|=0xff000000;
	} else {
		rgb=fade_rgb;
		if (sel) rgb=FRONTEND_AlterAlpha(rgb,0,1);
	}
	return rgb;
}
#endif

//--- drawy stuff ---

#define RandStream(s) ((UWORD)((s = ((s*69069)+1) )>>7))

void	FRONTEND_draw_title(SLONG x, SLONG y, SLONG cutx, CBYTE *str, BOOL wibble, BOOL r_to_l) {
#ifdef TARGET_DC
	SLONG rgb=wibble?0xffffffff:0x70ffffff;
#else
	SLONG rgb=wibble?(fade_rgb<<1)|0xffffff:fade_rgb|0xffffff;
#endif
	SLONG seed=*str;
	SWORD xo=0, yo=0;


#ifdef TARGET_DC
	// Scan for the word "VMU".
	// Mark the "U" as not to be drawn.
	CBYTE *pDontDrawThisLetter = NULL;
	if ( bWriteVMInsteadOfVMU )
	{
		pDontDrawThisLetter = strstr ( str, "VMU" );
		if ( pDontDrawThisLetter != NULL )
		{
			// Point to the U
			pDontDrawThisLetter += 2;
		}
	}
#endif




	for (;*str;str++) {
		if (!wibble)
		{
#ifdef TARGET_DC
			// Ugh - don't like this wibble.
			xo=4;
			yo=4;
#else
			xo=(RandStream(seed)&0x1f)-0xf;
			yo=4;
#endif
		}

#ifdef TARGET_DC
		if ( pDontDrawThisLetter == str )
		{
			// Ignore this letter.
			ASSERT ( *str == 'U' );
		}
		else
#endif
		{
			if (((r_to_l)&&(x>cutx))||((!r_to_l)&&(x<cutx))) {
				MENUFONT_Draw(x+xo,y+yo,BIG_FONT_SCALE+(wibble<<5),str,rgb,0,1);
			}
			x+=(MENUFONT_CharWidth(*str,BIG_FONT_SCALE))-2;
		}
	}

}

void	FRONTEND_init_xition ( void ) {
	MidX=RealDisplayWidth/2;
	MidY=RealDisplayHeight/2;
	ScaleX=MidX/64.0f;
	ScaleY=MidY/64.0f;
	switch (menu_state.mode) {
	case FE_MAPSCREEN:

		screenfull = screenfull_map;

		//FRONTEND_scr_img(menu_map_names[menu_theme]);
		break;
	case FE_MAINMENU:
		screenfull = screenfull_back;

		//FRONTEND_scr_img(menu_map_names[menu_theme]);
		break;
#ifdef WANT_A_TITLE_SCREEN
	case FE_TITLESCREEN:
	case FE_LANGUAGESCREEN:
		screenfull = screenfull_title;
		break;
#endif
	case FE_LOADSCREEN:
	case FE_SAVESCREEN:
	case FE_CONFIG:
#ifdef TARGET_DC
	case FE_VMU_SELECT:
#endif

#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
		screenfull = screenfull_back;
#else
		screenfull = screenfull_config;
#endif

		//FRONTEND_scr_img(menu_config_names[menu_theme]);
		break;
	default:
		if (menu_state.mode>=100)
		{
			screenfull = screenfull_brief;

			//FRONTEND_scr_img(menu_brief_names[menu_theme]);
		}
		else
		{
#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
			screenfull = screenfull_back;
#else
			screenfull = screenfull_config;
#endif
		}
	}
}



#if USE_COMPRESSED_BACKGROUNDS
CompressedBackground lpFRONTEND_show_xition_LastBlit = NULL;
#else //#if USE_COMPRESSED_BACKGROUNDS
LPDIRECTDRAWSURFACE4 lpFRONTEND_show_xition_LastBlit = NULL;
#endif //#else //#if USE_COMPRESSED_BACKGROUNDS

void	FRONTEND_show_xition() {
	RECT rc;

	bool bDoBlit = FALSE;

	if (menu_state.mode>=100)
	{
		rc.top=MidY-(fade_state*ScaleY); rc.bottom=MidY+(fade_state*ScaleY); // set to 3.75 ...
		rc.left=MidX-(fade_state*ScaleX); rc.right=MidX+(fade_state*ScaleX); // set to 5...
		if ( rc.left < rc.right )
		{
			bDoBlit = TRUE;
		}
	}
	else
	{
		// This is a great switch statement!
		//switch (menu_state.mode) {
		//case FE_LOADSCREEN:
		//case FE_SAVESCREEN:
		//case FE_MAPSCREEN:
		//case FE_CONFIG:
		//case FE_MAINMENU:
		//default:
			rc.top=0; rc.bottom=RealDisplayHeight;
			rc.left=0;
			if (RealDisplayWidth==640)
			{
				rc.right=fade_state*10;
			}
			else
			{
				rc.right=fade_state*10*RealDisplayWidth/640;
			}
			if ( rc.right > 0 )
			{
				bDoBlit = TRUE;
			}
			//break;
		//}
	}


	if ( bDoBlit )
	{
#ifndef TARGET_DC
		the_display.lp_DD_BackSurface->Blt(&rc,screenfull,&rc,DDBLT_WAIT,0);
#else

extern LPDIRECTDRAWSURFACE4 lpBackgroundCache2;

		HRESULT result;

		// Use a poly draw.

		ASSERT ( lpBackgroundCache2 != NULL );

#if USE_COMPRESSED_BACKGROUNDS
		if ( lpFRONTEND_show_xition_LastBlit != screenfull )
		{
			lpFRONTEND_show_xition_LastBlit = screenfull;
			if ( screenfull != NULL )
			{
				UnpackBackground ( (UCHAR*)screenfull, lpBackgroundCache2 );
			}
			else
			{
				// Bum - black screen time :-(
				ASSERT ( FALSE );
			}
		}

#else //#if USE_COMPRESSED_BACKGROUNDS
		if ( lpFRONTEND_show_xition_LastBlit != screenfull )
		{
			// Get the texture handle.
			lpFRONTEND_show_xition_LastBlit = screenfull;
			// Copy the data to the texture cache thingie.
			RECT rect;
			rect.top = 0;
			rect.left = 0;
			rect.right = 640;
			rect.bottom = 480;
			if ( screenfull == NULL )
			{
				// Nadgers. Black screen time.
				ASSERT ( FALSE );
			}
			else
			{
				HRESULT hres = lpBackgroundCache2->Blt ( &rect, screenfull, &rect, DDBLT_WAIT, NULL );
				VERIFY(SUCCEEDED(hres));
			}
		}
#endif //#else //#if USE_COMPRESSED_BACKGROUNDS

		ASSERT ( lpFRONTEND_show_xition_LastBlit != NULL );

		POLY_Point  pp[4];
		POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

		pp[0].colour=0xffffffff; pp[0].specular=0;
		pp[1].colour=0xffffffff; pp[1].specular=0;
		pp[2].colour=0xffffffff; pp[2].specular=0;
		pp[3].colour=0xffffffff; pp[3].specular=0;

		pp[0].X=rc.left; pp[0].Y=rc.top; pp[0].Z=0.0002f;
		pp[0].u=rc.left / 1024.0f; pp[0].v=rc.top / 512.0f;

		pp[1].X=rc.left; pp[1].Y=rc.bottom; pp[1].Z=0.0002f;
		pp[1].u=rc.left / 1024.0f; pp[1].v=rc.bottom / 512.0f;

		pp[2].X=rc.right; pp[2].Y=rc.top; pp[2].Z=0.0002f;
		pp[2].u=rc.right / 1024.0f; pp[2].v=rc.top / 512.0f;

		pp[3].X=rc.right; pp[3].Y=rc.bottom; pp[3].Z=0.0002f;
		pp[3].u=rc.right / 1024.0f; pp[3].v=rc.bottom / 512.0f;

		POLY_add_quad ( quad, POLY_PAGE_BACKGROUND_IMAGE2, FALSE, TRUE );

#endif
	}

}

extern UBYTE* image_mem;

void	FRONTEND_stop_xition()
{
	switch(menu_state.mode)
	{
#ifdef WANT_AN_EXIT_MENU_ITEM
		case FE_QUIT:
#endif
		case FE_MAINMENU:
			UseBackSurface(screenfull_back);
			break;
#ifdef WANT_A_TITLE_SCREEN
		case FE_TITLESCREEN:
		case FE_LANGUAGESCREEN:
			UseBackSurface(screenfull_title);
			break;
#endif
		case FE_CONFIG:
		case FE_CONFIG_VIDEO:
		case FE_CONFIG_AUDIO:
#ifdef WANT_A_KEYBOARD_ITEM
		case FE_CONFIG_INPUT_KB:
#endif
		case FE_CONFIG_INPUT_JP:
		case FE_LOADSCREEN:
		case FE_SAVESCREEN:
#ifdef TARGET_DC
		case FE_VMU_SELECT:
#endif
#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
			UseBackSurface(screenfull_back);
#else
			UseBackSurface(screenfull_config);
#endif
			break;
		case FE_MAPSCREEN:
			UseBackSurface(screenfull_map);
			break;
		default:
			if (menu_state.mode>=100)
			{
				UseBackSurface(screenfull_brief);
			}
			else
			{
#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
				UseBackSurface(screenfull_back);
#else
				UseBackSurface(screenfull_config);
#endif
			}
			break;
	}

	/*

	if (screenfull) {
		ResetBackImage();
		the_display.lp_DD_Background=screenfull;
		image_mem=screenimage;
		screenfull=NULL; screenimage=NULL;
	}

	*/

/*	if (menu_state.mode==FE_MAPSCREEN) InitBackImage("MAP SELECT LEAVES.tga");
	if (menu_state.mode>=100) InitBackImage("MISSION BRIEF LEAVES.tga");
	FRONTEND_scr_del();*/
}


void	FRONTEND_draw_button(SLONG x, SLONG y, UBYTE which, UBYTE flash = FALSE) {
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };
	float u,v,w,h;
	UBYTE size=(which<4)?64:32;
	UBYTE grow;

	if (flash)
	{
		grow = 8;

		if (GetTickCount() & 0x200)
		{
			which = 7;
		}
	}
	else
	{
		grow = 0;
	}

	switch (which) {
	case 0: case 4: u=0.0; v=0.0; w=0.5; h=0.5;	break;
	case 1: case 5: u=0.5; v=0.0; w=1.0; h=0.5;	break;
	case 2: case 6: u=0.0; v=0.5; w=0.5; h=1.0;	break;
	case 3: case 7: u=0.5; v=0.5; w=1.0; h=1.0;	break;
	}

	pp[0].colour=(which<4)?0xffFFFFFF:(fade_rgb<<1)|0xffffff; pp[0].specular=0; pp[0].Z=0.5;
	pp[1]=pp[2]=pp[3]=pp[0];

	pp[0].X=x-(grow>>1); pp[0].Y=y-grow;
	pp[0].u=u; pp[0].v=v;

	pp[1].X=x+(grow>>1)+size; pp[1].Y=y-grow;
	pp[1].u=w; pp[1].v=v;

	pp[2].X=x; pp[2].Y=y+size;
	pp[2].u=u; pp[2].v=h;

	pp[3].X=x+size; pp[3].Y=y+size;
	pp[3].u=w; pp[3].v=h;

	POLY_add_quad(quad,(which<4)?POLY_PAGE_BIG_BUTTONS:POLY_PAGE_TINY_BUTTONS,FALSE,TRUE);
}

#define KIBBLE_Z 0.5

void	FRONTEND_kibble_draw() {
	UWORD c0;
	Kibble*k;
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };
	SLONG matrix[9],x,y,z;

	ASSERT ( kibble != NULL );

	for (c0=0,k=kibble;c0<512;c0++,k++)
	  if (k->type>0) {

		pp[0].colour=(k->rgb) | 0xff000000; pp[0].specular=0;
		pp[1]=pp[2]=pp[3]=pp[0];

		FMATRIX_calc(matrix, k->r, k->t, k->p);

		x=- k->size;
		y=- k->size;
		z=0;
		FMATRIX_MUL(matrix,x,y,z);
		pp[0].X=(k->x>>8)+x; pp[0].Y=(k->y>>8)+y; pp[0].Z=KIBBLE_Z;
		pp[0].u=0; pp[0].v=0;

		x=+ k->size;
		y=- k->size;
		z=0;
		FMATRIX_MUL(matrix,x,y,z);
		pp[1].X=(k->x>>8)+x; pp[1].Y=(k->y>>8)+y; pp[1].Z=KIBBLE_Z;
		pp[1].u=1; pp[1].v=0;

		x=- k->size;
		y=+ k->size;
		z=0;
		FMATRIX_MUL(matrix,x,y,z);
		pp[2].X=(k->x>>8)+x; pp[2].Y=(k->y>>8)+y; pp[2].Z=KIBBLE_Z;
		pp[2].u=0; pp[2].v=1;

		x=+ k->size;
		y=+ k->size;
		z=0;
		FMATRIX_MUL(matrix,x,y,z);
		pp[3].X=(k->x>>8)+x; pp[3].Y=(k->y>>8)+y; pp[3].Z=KIBBLE_Z;
		pp[3].u=1; pp[3].v=1;

		POLY_add_quad(quad,k->page,FALSE,TRUE);

	}
}


// Oh yuk this is pants - really could look better.
void	FRONTEND_DrawSlider(MenuData *md) {
	SLONG y;
	ULONG rgb=FRONTEND_fix_rgb(fade_rgb,0);
	y=md->Y+menu_state.base-menu_state.scroll;
	DRAW2D_Box(320,y-2,610,y+2,rgb,0,192);
	DRAW2D_Box(337,y-4,341,y+4,rgb,0,192);
	DRAW2D_Box(337+255,y-4,341+255,y+4,rgb,0,192);
	DRAW2D_Box(337+(md->Data),y-8,341+(md->Data),y+8,rgb,0,192);
}

void	FRONTEND_DrawMulti(MenuData *md, ULONG rgb) {
	SLONG x,y,dy,c0;
	CBYTE *str;
	//ULONG rgb=FRONTEND_fix_rgb(fade_rgb,0);
	dy=md->Y+menu_state.base-menu_state.scroll;
	str=md->Choices;
	c0=md->Data&0xff;

	if (!str) return;

	while ((*str)&&c0--) {
		str+=strlen(str)+1;
	}
#ifndef TARGET_DC
	if (IsEnglish)
#endif
	{
		MENUFONT_Dimensions(str,x,y,-1,BIG_FONT_SCALE);
		if (320+x>630)
		{
			if (320+(x>>1)>630)
			{
				c0=MENUFONT_CharFit(str,310,128);
				MENUFONT_Draw(320,dy-15,128,str,rgb,0,c0);
				MENUFONT_Draw(320,dy+15,128,str+c0,rgb,0);
			}
			else
			{
				MENUFONT_Draw(620-(x>>1),dy,128,str,rgb,0);
			}
		}
		else
		{
			MENUFONT_Draw(620-x,dy,BIG_FONT_SCALE,str,rgb,0);
		}
	}
#ifndef TARGET_DC
	else
	{
		rgb=FRONTEND_fix_rgb(fade_rgb,1);
		FONT2D_DrawStringRightJustify(str,620,dy,rgb,SMALL_FONT_SCALE + 64,POLY_PAGE_FONT2D);
	}
#endif
}

void	FRONTEND_DrawKey(MenuData *md) {
	SLONG x,y,dy,c0,rgb;
	CBYTE key;
	CBYTE str[25];
	rgb=FRONTEND_fix_rgb(fade_rgb,(grabbing_key&&((menu_data+menu_state.selected==md)&&((GetTickCount()&0x7ff)<0x3ff))));
	dy=md->Y+menu_state.base-menu_state.scroll;
/*	switch (md->Data) {
	case KB_LEFT:
		c0=md->Data &~ 0x80;
		c0<<=16;
		c0|=1<<24;
		GetKeyNameText(c0,str,20);
		break;
//		strcpy(str,"Left");			break;
	case KB_RIGHT:
		strcpy(str,"Right");		break;
	case KB_UP:
		strcpy(str,"Up");			break;
	case KB_DOWN:
		strcpy(str,"Down");			break;
	case KB_ENTER:
		strcpy(str,"Enter");		break;
	case KB_SPACE:
		strcpy(str,"Space");		break;
	case KB_LSHIFT:
		strcpy(str,"Left Shift");	break;
	case KB_RSHIFT:
		strcpy(str,"Right Shift");	break;
	case KB_LALT:
		strcpy(str,"Left Alt");		break;
	case KB_RALT:
		strcpy(str,"Right Alt");	break;
	case KB_LCONTROL:
		strcpy(str,"Left Ctrl");	break;
	case KB_RCONTROL:
		strcpy(str,"Right Ctrl");break;
	case KB_TAB:
		strcpy(str,"Tab");			break;
	case KB_END:
		strcpy(str,"End");			break;
	case KB_HOME:
		strcpy(str,"Home");			break;
	case KB_DEL:
		strcpy(str,"Delete");		break;
	case KB_INS:
		strcpy(str,"Insert");		break;
	case KB_PGUP:
		strcpy(str,"Page Up");		break;
	case KB_PGDN:
		strcpy(str,"Page Down");	break;
	case 0:
		strcpy(str,"Unused");		break;
	default:
		//key=InkeyToAscii[md->Data];
		//str[0]=key; str[1]=0;
		GetKeyNameText(md->Data<<16,str,20);
//		if (!stricmp(str,"Circumflex")) strcpy(str,"^");
	}*/

	c0=md->Data;
	if (c0&0x80)
	{
		c0=md->Data &~ 0x80;
		c0<<=16;
		c0|=1<<24;
	} else {
		c0<<=16;
	}

#ifdef TARGET_DC
	strcpy ( str, "FixMePlease" );
#else
	GetKeyNameText(c0,str,25);
#endif

	if (IsEnglish)
	{
		MENUFONT_Dimensions(str,x,y,-1,BIG_FONT_SCALE);
		MENUFONT_Draw(620-x,dy,BIG_FONT_SCALE,str,rgb,0);
	}
	else
	{
		FONT2D_DrawStringRightJustify(str,620,dy,rgb,SMALL_FONT_SCALE,POLY_PAGE_FONT2D);
	}
}

#ifdef TARGET_DC

void	FRONTEND_DrawPad(MenuData *md) {
	SLONG x,y,dy,c0,rgb;
	CBYTE str[20];
	rgb=FRONTEND_fix_rgb(fade_rgb,(grabbing_pad&&((menu_data+menu_state.selected==md)&&((GetTickCount()&0x3ff)<0x1ff))));
	dy=md->Y+menu_state.base-menu_state.scroll;
	if (md->Data==31)
	{
		// Unused.
		sprintf(str,"%s",XLAT_str(X_EMPTY));
		MENUFONT_Dimensions(str,x,y,-1,BIG_FONT_SCALE);
		MENUFONT_Draw(620-x,dy,BIG_FONT_SCALE,str,rgb,0);
	}
	else
	{
		// Draw the button icon
		DWORD dwPage = -1;
		switch ( md->Data )
		{
		case DI_DC_BUTTON_A			: dwPage = POLY_PAGE_JOYPAD_A;		break;
		case DI_DC_BUTTON_B			: dwPage = POLY_PAGE_JOYPAD_B;		break;
		case DI_DC_BUTTON_C			: dwPage = POLY_PAGE_JOYPAD_C;		break;
		case DI_DC_BUTTON_X			: dwPage = POLY_PAGE_JOYPAD_X;		break;
		case DI_DC_BUTTON_Y			: dwPage = POLY_PAGE_JOYPAD_Y;		break;
		case DI_DC_BUTTON_Z			: dwPage = POLY_PAGE_JOYPAD_Z;		break;
		case DI_DC_BUTTON_RTRIGGER	: dwPage = POLY_PAGE_JOYPAD_R;		break;
		case DI_DC_BUTTON_LTRIGGER	: dwPage = POLY_PAGE_JOYPAD_L;		break;
		case DI_DC_BUTTON_UP		: dwPage = POLY_PAGE_JOYPAD_PAD_U;	break;
		case DI_DC_BUTTON_DOWN		: dwPage = POLY_PAGE_JOYPAD_PAD_D;	break;
		case DI_DC_BUTTON_LEFT		: dwPage = POLY_PAGE_JOYPAD_PAD_L;	break;
		case DI_DC_BUTTON_RIGHT		: dwPage = POLY_PAGE_JOYPAD_PAD_R;	break;
		default:
			// Might be Start - don't have graphics, and shouldn't need them!
			ASSERT ( FALSE );
			break;
		}

		if ( dwPage == -1 )
		{
			// Button that we don't have the graphics for.
			sprintf(str,"%s %d",XLAT_str(X_BUTTON),md->Data);
			MENUFONT_Dimensions(str,x,y,-1,BIG_FONT_SCALE);
			MENUFONT_Draw(620-x,dy,BIG_FONT_SCALE,str,rgb,0);
		}
		else
		{
			DWORD dwRGB = 0xffe0e0e0;
			if ( grabbing_pad && ( menu_data + menu_state.selected == md ) && ( (GetTickCount()&0x3ff)<0x1ff ) )
			{
				// Flash it.
				dwRGB = 0xff505050;
			}

			POLY_Point  pp[4];
			POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

			// The size of the button in pixels.
			const float fSize = 48.0f * 0.5f;

			pp[0].colour=dwRGB;
			pp[0].specular=0;
			pp[0].Z=0.7f;
			pp[1]=pp[2]=pp[3]=pp[0];

			pp[0].X=620 - fSize * 2.0f; pp[0].Y=dy - fSize;
			pp[0].u=0.0f; pp[0].v=0.0f;

			pp[1].X=620 - fSize * 2.0f; pp[1].Y=dy + fSize;
			pp[1].u=0.0f; pp[1].v=1.0f;

			pp[2].X=620; pp[2].Y=dy - fSize;
			pp[2].u=1.0f; pp[2].v=0.0f;

			pp[3].X=620; pp[3].Y=dy + fSize;
			pp[3].u=1.0f; pp[3].v=1.0f;

			POLY_add_quad(quad,dwPage,FALSE,TRUE);
		}
	}
}

#else

void	FRONTEND_DrawPad(MenuData *md) {
	SLONG x,y,dy,c0,rgb;
	CBYTE str[20];
	rgb=FRONTEND_fix_rgb(fade_rgb,(grabbing_pad&&((menu_data+menu_state.selected==md)&&((GetTickCount()&0x7ff)<0x3ff))));
	dy=md->Y+menu_state.base-menu_state.scroll;
	if (md->Data<31) sprintf(str,"%s %d",XLAT_str(X_BUTTON),md->Data); else strcpy(str,"Unused");
	MENUFONT_Dimensions(str,x,y,-1,BIG_FONT_SCALE);
	MENUFONT_Draw(620-x,dy,BIG_FONT_SCALE,str,rgb,0);
}

#endif


//--- kibbly stuff ---

void	FRONTEND_kibble_init_one(Kibble*k, UBYTE type) {

	SLONG kibble_index = k - kibble;

	ASSERT ( kibble != NULL );

	ASSERT(WITHIN(kibble_index, 0, 511));

#ifndef FORCE_STUFF_PLEASE_BOB
	if (kibble_off[kibble_index])
	{
		return;
	}
#endif

	if (!(type&128)) {
		if ((menu_state.mode==FE_MAPSCREEN)||(menu_state.mode>=100)) return;
	}
	switch(type&0x7f) {
	case 1:
		k->dx=(20+(Random()&15))<<5;
		k->dy=0;
		k->page=POLY_PAGE_BIG_LEAF;
		k->rgb=FRONTEND_leaf_colours[Random()&3];
		k->x=(-10-(Random()&0x1ff))<<8;
		k->y=((Random()%520)-20)<<8;
		k->size=35+(Random()&0x1f);
		k->r=Random()&2047; k->t=Random()&2047; k->p=0;
		k->rd=1+(Random()&7); k->td=1+(Random()&7); k->pd=0;
		k->type=type;
		break;
	case 2:
		k->dx=k->dy=(20+(Random()&15))<<5;
		k->page=POLY_PAGE_BIG_RAIN;
		k->rgb=0x3f7f7fff;
		if (Random()&1) {
			k->x=(-10-(Random()&0x1ff))<<8;
			k->y=((Random()%520)-20)<<8;
		} else {
			k->x=((Random()%680)-20)<<8;
			k->y=(-10-(Random()&0x1ff)-20)<<8;
		}
		k->size=25+(Random()&0x1f);
		k->r=0; k->t=0; k->p=1792;
		k->rd=0; k->td=0; k->pd=0;
		k->type=type;
		break;
	case 3:
		k->dx=(15+(Random()&15))<<5;
		k->dy=(5+(Random()&15))<<5;
		k->page=POLY_PAGE_SNOWFLAKE;
		k->rgb=0xffafafff;
		if (Random()&1) {
			k->x=(-10-(Random()&0x1ff))<<8;
			k->y=((Random()%520)-20)<<8;
		} else {
			k->x=((Random()%680)-20)<<8;
			k->y=(-10-(Random()&0x1ff)-20)<<8;
		}
		k->size=25+(Random()&0x1f);
		k->r=0; k->t=0; k->p=0;
		k->rd=1; k->td=2; k->pd=0;
		k->type=type;
		break;
	}
}

void	FRONTEND_kibble_init() {
	UWORD c0, densities[] = { 25, 255, 40, 10 };
	Kibble*k;

#ifndef TARGET_DC
	if (SOFTWARE)
	{
		densities[0] = 20;
		densities[1] = 175;
		densities[2] = 30;
		densities[3] = 10;
	}
	else
#endif
	{
		densities[0] = 25;
		densities[1] = 255;
		densities[2] = 40;
		densities[3] = 10;
	}

#ifdef TARGET_DC
	// Allocate the kibble
	if ( kibble == NULL )
	{
		kibble = (Kibble *)MemAlloc ( sizeof ( Kibble ) * 512 );
		ASSERT ( kibble != NULL );
	}
	ZeroMemory(kibble,( sizeof ( Kibble ) * 512 ) );
#else
	ZeroMemory(kibble,sizeof(kibble));
#endif

	for (c0=0,k=kibble;c0<densities[menu_theme];c0++,k++) FRONTEND_kibble_init_one(k,menu_theme+1);
}


#ifdef TARGET_DC
void	FRONTEND_kibble_destroy()
{
	// Bin the kibble.
	if ( kibble != NULL )
	{
		MemFree ( kibble );
		kibble = NULL;
	}
}
#endif


void	FRONTEND_kibble_flurry() {
	UWORD n, c0, densities[4];
	Kibble*k;

	ASSERT ( kibble != NULL );

#ifndef TARGET_DC
	if (SOFTWARE)
	{
		densities[0] = 50;
		densities[1] = 200;
		densities[2] = 50;
		densities[3] = 10;
	}
	else
#endif
	{
		densities[0] = 125;
		densities[1] = 512;
		densities[2] = 125;
		densities[3] = 10;
	}

	n=densities[menu_theme];

	for (c0=0,k=kibble;c0<n;c0++,k++)
	  if (!k->type) {
		  switch(menu_theme) {
		  case 0:
			  FRONTEND_kibble_init_one(k,1|128);
			  k->dx=(25+(Random()&15))<<5;
			  k->x=(-60-(Random()&0xff))<<8;
			  k->y=(Random()%480)<<8;
			  k->size=5+(Random()&0x1f);
			  k->r=Random()&2047; k->t=Random()&2047;
			  k->rd=1+(Random()&7); k->td=1+(Random()&7);
			  //k->type|=128;
			  break;
		  case 1:
			  FRONTEND_kibble_init_one(k,2|128);
			  //k->type|=128;
			  break;
		  case 2:
			  FRONTEND_kibble_init_one(k,3|128);
			  //k->type|=128;
			  break;
		  }
	}
}

void	FRONTEND_kibble_process() {
	SLONG c0;
	Kibble*k;

	static SLONG last = 0;
	static SLONG now  = 0;

	ASSERT ( kibble != NULL );

	now = GetTickCount();

	if (last < now - 250)
	{
		last = now - 250;
	}

#ifndef FORCE_STUFF_PLEASE_BOB
	if (now > last + 100)
	{
		//
		// Front-end running at less than 10 fps! Turn off a random kibble!
		//

		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
		kibble_off[rand() & 0x1ff] = TRUE;
	}
	else
	if (now < last + 50)
	{
		//
		// More than 20 fps...
		//

		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
		kibble_off[rand() & 0x1ff] = FALSE;
	}
#endif

	SLONG i;
	SLONG num_on;

	for (i = 0, num_on = 0; i < 512; i++) {if (!kibble_off[i]) {num_on += 1;}}

	while(last < now)
	{
		//
		// Process at 40 frames a second.
		//

		last += 1000 / 40;

		for (c0=0,k=kibble;c0<512;c0++,k++)
		  if (k->type>0) {
			  k->x+=k->dx; k->y+=k->dy;
			  k->r+=k->rd; k->t+=k->td;
			  k->r&=2047; k->t&=2047;
			  switch (k->type) {
			  case 1:
				k->dy++;
				k->dx++;
				if ((k->y>>8)>240) k->dy-=Random()%((k->y-240)>>14);
	//			if ((k->x>>8)+k->size<10) FRONTEND_kibble_init_one(k,1);
				if ((k->x>>8)-k->size>650) FRONTEND_kibble_init_one(k,1);
				break;
			  case 129:
	//			if ((k->x>>8)<10) k->type=0;
				if ((k->x>>8)-k->size>650) k->type=0;
			  case 3:
			  case 131:
				{
					SWORD x=k->x>>8, y=k->y>>8;
					k->dx++;
					if ((x>320)&&(x<480)) k->dx-=Random()%((k->x-320)>>14);
					if ((y>240)&&(y<280)) k->dy-=Random()%((k->y-240)>>14);
				}
			  case 2:
			  case 130:
				  if ((((k->x>>8)-k->size)>640)||(((k->y>>8)-k->size)>480))
					  if (k->type<128)
						  FRONTEND_kibble_init_one(k,k->type&127);
					  else
						  k->type=0;
				  break;
			  }
		  }
	}
}

//--- filing stuff ---

void FRONTEND_fetch_title_from_id(CBYTE *script, CBYTE *ttl, UBYTE id) {
	CBYTE *text;
	SLONG ver;
	MissionData *mdata = MFnew<MissionData>();

	*ttl=0;

	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);

	// Default value = no valid mission.
	// This can happen when saving a game that has been fully completed -
	// no next suggested mission.
	strcpy ( ttl, "Urban Chaos" );


	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			FRONTEND_ParseMissionData(text,ver,mdata);

			if (mdata->ObjID==id) {
				strcpy(ttl,mdata->ttl);
				break;
			}
		}
	}
	FileCloseScript();

	MemFree(text);
	MFdelete(mdata);

	// Bin any trailing spaces.
	char *pEnd = ttl + strlen ( ttl ) - 1;
	while ( *pEnd == ' ' )
	{
		*pEnd = '\0';
		pEnd--;
	}

}

UBYTE	best_found[50][4];
void	init_best_found(void)
{
	memset(&best_found[0][0],50*4,0);
}


#ifndef TARGET_DC


bool FRONTEND_save_savegame(CBYTE *mission_name, UBYTE slot) {
	CBYTE fn[_MAX_PATH];
	MFFileHandle file;
	UBYTE version=3;

	CreateDirectory("saves",NULL);

	sprintf(fn,"saves\\slot%d.wag",slot);
	file=FileCreate(fn,1);
	FRONTEND_SaveString(file,mission_name);
	FileWrite(file,&complete_point,1);
	FileWrite(file,&version,1);
	FileWrite(file,&the_game.DarciStrength,1);
	FileWrite(file,&the_game.DarciConstitution,1);
	FileWrite(file,&the_game.DarciSkill,1);
	FileWrite(file,&the_game.DarciStamina,1);
	FileWrite(file,&the_game.RoperStrength,1);
	FileWrite(file,&the_game.RoperConstitution,1);
	FileWrite(file,&the_game.RoperSkill,1);
	FileWrite(file,&the_game.RoperStamina,1);
	FileWrite(file,&the_game.DarciDeadCivWarnings,1);
	// mark, if you add stuff again, please remember to inc. the version number
	FileWrite(file,mission_hierarchy,60);

	FileWrite(file,&best_found[0][0],50*4);

	FileClose(file);

	return TRUE;
}

bool FRONTEND_load_savegame(UBYTE slot) {
	CBYTE fn[_MAX_PATH];
	MFFileHandle file;
	UBYTE version=0;

	sprintf(fn,"saves\\slot%d.wag",slot);
	file=FileOpen(fn);
	FRONTEND_LoadString(file,fn);
	FileRead(file,&complete_point,1);
	FileRead(file,&version,1);	// yes, i know, strange place to have version number.
								// it's for Historical Reasons(tm).
	if (version) {
		FileRead(file,&the_game.DarciStrength,1);
		FileRead(file,&the_game.DarciConstitution,1);
		FileRead(file,&the_game.DarciSkill,1);
		FileRead(file,&the_game.DarciStamina,1);
		FileRead(file,&the_game.RoperStrength,1);
		FileRead(file,&the_game.RoperConstitution,1);
		FileRead(file,&the_game.RoperSkill,1);
		FileRead(file,&the_game.RoperStamina,1);
		FileRead(file,&the_game.DarciDeadCivWarnings,1);
	}
	if (version>1)
	{
		FileRead(file,mission_hierarchy,60);
	}
	if (version>2)
	{
		FileRead(file,&best_found[0][0],50*4);
	}
	FileClose(file);

	return TRUE;
}


void FRONTEND_find_savegames ( bool bGreyOutEmpties=FALSE, bool bCheckSaveSpace=FALSE )
{
	CBYTE dir[_MAX_PATH],ttl[_MAX_PATH];
	WIN32_FIND_DATA data;
	HANDLE handle;
	BOOL   ok;
	SLONG	c0;
	MenuData *md=menu_data;
	CBYTE *str=menu_buffer;
	SLONG x,y,y2=0;
	FILETIME time, high_time={0,0};

	for (c0=1;c0<11;c0++)
	{
		md->Type=OT_BUTTON;
		//md->Data=0;
		md->Data=FE_SAVE_CONFIRM;
		// Not greyed.
		md->Choices = (CBYTE*)0;

		MFFileHandle file;
		sprintf(dir,"saves\\slot%d.wag",c0);
		file = FileOpen(dir);
		GetFileTime(file,NULL,NULL,&time);
		if (file!=FILE_OPEN_ERROR)
		{
			FRONTEND_LoadString(file,ttl);
			FileClose(file);
		}
		else
		{
			strcpy(ttl,XLAT_str(X_EMPTY));
			if ( bGreyOutEmpties )
			{
				// Grey this out then.
				md->Choices = (CBYTE*)1;
			}
		}
		sprintf(dir,"%d: %s",c0,ttl);
		md->Label=str;
		strcpy(str,ttl);
		str+=strlen(str)+1;
		MENUFONT_Dimensions(md->Label,x,y,-1,BIG_FONT_SCALE);
		md->X=320-(x>>1);
		md->Y=y2;
		y2+=50;
		if (CompareFileTime(&time,&high_time)>0)
		{
			high_time = time;
			menu_state.selected=menu_state.items;
		}
		md++;
		menu_state.items++;
	}
}


#else //#ifndef TARGET_DC



#define MAX_SIZE_SAVEGAME_FILES 1024
char m_pcSaveData[MAX_SIZE_SAVEGAME_FILES];


bool m_bLoadedIconSavePic = FALSE;
BYTE m_pcIconSavePicData[32*16];
WORD m_pcIconSavePicPalette[16];


// Set to 1 for utter madness.
#define NEED_A_SAVING_LOADING_MESSAGE 1

bool FRONTEND_save_savegame ( int iMissionID, UBYTE slot )
{
	CBYTE fn[_MAX_PATH];
	UBYTE version=4;


#if NEED_A_SAVING_LOADING_MESSAGE
	// Display a QuickMessage.
	FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_SAVING ), 3000 );

	// Then fake up a screen cycling so it gets displayed.
	the_display.Flip ( NULL, DDFLIP_WAIT );
	FRONTEND_display();
	the_display.Flip ( NULL, DDFLIP_WAIT );
	FRONTEND_display();

	// Mark the time - wait for a second.
	DWORD dwTimeout = timeGetTime() + 1000;
#endif


	if ( iMissionID == -1 )
	{
		iMissionID = 255;
	}
	ASSERT ( ( iMissionID <= 255 ) && ( iMissionID >= 0 ) );

	//sprintf(fn,"URBAN_%02i.SAV",slot);
	//MUST be this format, according to Sega.
	sprintf(fn,"UR_CHAOS.%03i",slot);



	char *pcCurPtr;
	DWORD dwSize;

#define DC_FILE_WRITE(data,size) memcpy ( (void*)pcCurPtr, (void *)(data), size ); pcCurPtr += (size); dwSize += (size)

	pcCurPtr = m_pcSaveData;
	dwSize = 0;


	//file=FileCreate(fn,1);
	DC_FILE_WRITE(&version,1);
	DC_FILE_WRITE(&iMissionID,1);
	DC_FILE_WRITE(&complete_point,1);
	DC_FILE_WRITE(&the_game.DarciStrength,1);
	DC_FILE_WRITE(&the_game.DarciConstitution,1);
	DC_FILE_WRITE(&the_game.DarciSkill,1);
	DC_FILE_WRITE(&the_game.DarciStamina,1);
	DC_FILE_WRITE(&the_game.RoperStrength,1);
	DC_FILE_WRITE(&the_game.RoperConstitution,1);
	DC_FILE_WRITE(&the_game.RoperSkill,1);
	DC_FILE_WRITE(&the_game.RoperStamina,1);
	DC_FILE_WRITE(&the_game.DarciDeadCivWarnings,1);
	// mark, if you add stuff again, please remember to inc. the version number
	DC_FILE_WRITE(mission_hierarchy,60);

	DC_FILE_WRITE(&best_found[0][0],50*4);

	// Version 4+ stuff.
	// Env save.
	int iEnvSize = ENV_save ( pcCurPtr );
	pcCurPtr += iEnvSize;
	dwSize += iEnvSize;


	ASSERT ( dwSize < MAX_SIZE_SAVEGAME_FILES );

	// Save to the current VMU.
	MapleVMU *pVMU = FindCurrentStorageVMU ( FALSE );
	if ( pVMU != NULL )
	{
		// Use the mission name as a comment.
		CBYTE ttl[_MAX_PATH];
		ttl[0] = '\0';
		FRONTEND_fetch_title_from_id(MISSION_SCRIPT,ttl,iMissionID);

		if ( ttl[0] == '\0' )
		{
			// Oops. Use something sensible.
			strcpy ( ttl, "Urban Chaos" );
		}

#if NEED_A_SAVING_LOADING_MESSAGE
		// Lock out the reset-on-door-open thing.
		ChangeDoorResetStatus ( FALSE );
#endif


		bool bRes = pVMU->Flash_WriteFile ( fn, "Urban Chaos", ttl, m_pcSaveData, dwSize, (char *)m_pcIconSavePicPalette, (char *)m_pcIconSavePicData );


#if NEED_A_SAVING_LOADING_MESSAGE
		// Now wait for a second
		// so that the user has time to read the "saving" message that will be
		// obsolete by the time they read it. Bloody standards.
		while ( TRUE )
		{
			if ( ( ( dwTimeout - timeGetTime() ) & 0x80000000 ) != 0 )
			{
				break;
			}
			TRACE ( "w" );
			Sleep(100);
		}
		TRACE ( "\n" );

		// Lock out the reset-on-door-open thing.
		ChangeDoorResetStatus ( TRUE );
#endif




		if ( bRes )
		{
			FRONTEND_show_VMU_screen ( pvmuscreenSaved );
			FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_SAVED ), 3000 );
		}
		else
		{
			FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_SAVE_FAILED ), 3000 );
		}
		return bRes;
	}
	else
	{
		// Save failed - VMU not present, or VMU full or something.
		FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_SAVE_FAILED ), 3000 );
		return FALSE;
	}


#ifdef DEBUG
	ASSERT (FALSE);
	// This is a debug rout to fill the VMU with junk, so we can test the "full VMU" action.
	pVMU = FindFirstVMUOnCurrentController();
	if ( pVMU != NULL )
	{
		CBYTE ttl[_MAX_PATH];
		bool bRes = TRUE;
		int iCount = 1;
		while ( bRes )
		{
			sprintf ( fn, "junk%04i.xxx", iCount );
			strcpy ( ttl, "Urban Chaos" );
			bRes = pVMU->Flash_WriteFile ( fn, "Urban Chaos", ttl, m_pcSaveData, dwSize, NULL, NULL );
			iCount++;
		}
	}

#endif

}


bool FRONTEND_load_savegame(UBYTE slot)
{
	CBYTE fn[_MAX_PATH];
	UBYTE version=0;
	int iMissionID;



#if NEED_A_SAVING_LOADING_MESSAGE
	// Display a QuickMessage.
	ChangeDoorResetStatus ( FALSE );

	FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_LOADING ), 3000 );

	// Then fake up a few screen cycles so it gets displayed.
	the_display.Flip ( NULL, DDFLIP_WAIT );
	FRONTEND_display();
	the_display.Flip ( NULL, DDFLIP_WAIT );
	FRONTEND_display();

	// Mark the time - wait for a second.
	DWORD dwTimeout = timeGetTime() + 1000;

#endif


	//sprintf(fn,"URBAN_%02i.SAV",slot);
	//MUST be this format, according to Sega.
	sprintf(fn,"UR_CHAOS.%03i",slot);


	MapleVMU *pVMU = FindCurrentStorageVMU ( FALSE );
	if ( pVMU == NULL )
	{
		// No VMUs. Nadgers.
		ASSERT ( FALSE );
		FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_LOAD_FAILED ), 3000 );
		return FALSE;
	}

	bool bRes = pVMU->Flash_ReadFile ( fn, m_pcSaveData, MAX_SIZE_SAVEGAME_FILES );


#if NEED_A_SAVING_LOADING_MESSAGE
	// Now wait for a second
	// so that the user has time to read the "saving" message that will be
	// obsolete by the time they read it. Bloody standards.

	// No, don't ask me why we need to ensure game _loads_ succeed before ... um ... resetting the machine.
	// Just fucking do it.
	while ( TRUE )
	{
		if ( ( ( dwTimeout - timeGetTime() ) & 0x80000000 ) != 0 )
		{
			break;
		}
		TRACE ( "w" );
		Sleep(100);
	}
	TRACE ( "\n" );

	ChangeDoorResetStatus ( TRUE );
#endif


	if ( !bRes )
	{
		// File doesn't exist, or VMU's been removed or something.
		// Happens if the user slects an (EMPTY) file (though they shouldn't be able to).
		ASSERT ( FALSE );
		FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_LOAD_FAILED ), 3000 );
		return FALSE;
	}

	char *pcCurPtr;

#define DC_FILE_READ(data,size) memcpy ( (void*)(data), (void *)(pcCurPtr), size ); pcCurPtr += (size)

	pcCurPtr = m_pcSaveData;


	DC_FILE_READ(&version,1);
	// Only v3 and above should exist for DC!
	ASSERT ( version >= 3 );

	DC_FILE_READ(&iMissionID,1);
	DC_FILE_READ(&complete_point,1);

	if ( version >= 1 )
	{
		DC_FILE_READ(&the_game.DarciStrength,1);
		DC_FILE_READ(&the_game.DarciConstitution,1);
		DC_FILE_READ(&the_game.DarciSkill,1);
		DC_FILE_READ(&the_game.DarciStamina,1);
		DC_FILE_READ(&the_game.RoperStrength,1);
		DC_FILE_READ(&the_game.RoperConstitution,1);
		DC_FILE_READ(&the_game.RoperSkill,1);
		DC_FILE_READ(&the_game.RoperStamina,1);
		DC_FILE_READ(&the_game.DarciDeadCivWarnings,1);
	}
	if ( version >= 2 )
	{
		DC_FILE_READ(mission_hierarchy,60);
	}
	if ( version >= 3 )
	{
		DC_FILE_READ(&best_found[0][0],50*4);
	}
	if ( version >= 4 )
	{
		// Load the environment vars.
		// But preserve the language setting!
		BYTE bLanguage = ENV_get_value_number ( "lang_num", 0, "" );
		pcCurPtr = ENV_load ( pcCurPtr );
		ENV_set_value_number ( "lang_num", bLanguage, "" );
	}

	FRONTEND_MakeQuickMessage ( XLAT_str ( X_GAME_LOADED ), 3000 );
	return TRUE;

}


#define SIZE_OF_VMU_SAVE_FILE_IN_BLOCKS 2

void FRONTEND_find_savegames ( bool bGreyOutEmpties=FALSE, bool bCheckSaveSpace=FALSE )
{
	CBYTE dir[_MAX_PATH];
	CBYTE ttl[_MAX_PATH];
	WIN32_FIND_DATA data;
	HANDLE handle;
	BOOL   ok;
	SLONG	c0;
	MenuData *md=menu_data;
	CBYTE *str=menu_buffer;
	SLONG x,y,y2=0;
	FILETIME time, high_time={0,0};
	int iBestMissionIDSoFar = -2;
	int iBestMissionIDSoFarPosition = -1;

	menu_state.selected = 0;


	int iBigFontScale = BIG_FONT_SCALE;
	if ( !IsEnglish )
	{
		iBigFontScale = BIG_FONT_SCALE_FRENCH;
	}



	bool bSomethingChanged = TRUE;
	while ( bSomethingChanged )
	{
		bSomethingChanged = RescanDevices();
		if ( bSomethingChanged )
		{
			// Delete any removed devices.
			DeleteInvalidDevice();
		}
	}

	// Find the current VMU.
	MapleVMU *pVMU = FindCurrentStorageVMU ( TRUE );
	// And actually set whichever was found to be the current one.
	SetCurrentStorageVMU ( pVMU );

	bool bMarkEmptiesAsVMUFull = FALSE;
	if ( bCheckSaveSpace && ( pVMU != NULL ) )
	{
		// Check how much space is on this VMU.
		int iFreeBlocks = pVMU->Flash_GetFreeBlocks();
		if ( iFreeBlocks < SIZE_OF_VMU_SAVE_FILE_IN_BLOCKS )
		{
			// No - not enough space - grey out the empty slots.
			bMarkEmptiesAsVMUFull = TRUE;
		}
	}

	int iFirstBlankSlot = -1;

	// If pVMU is NULL, it will be dealt with below.
	if ( pVMU == NULL )
	{
		md->Type=OT_BUTTON;
		//md->Data=0;
		md->Data=FE_SAVE_CONFIRM;
		// Just one entry.
		strcpy(str,XLAT_str(X_VMU_NOT_PRESENT));
		// Grey this out.
		md->Choices = (CBYTE*)1;
		md->Label=str;
		str+=strlen(str)+1;
		MENUFONT_Dimensions(md->Label,x,y,-1,iBigFontScale);
		md->X=320-(x>>1);
		md->Y=y2;
		y2+=50;

		md++;
		menu_state.items++;

		// And select the item _after_ which will be "select VMU"
		menu_state.selected=menu_state.items;
	}
	else
	{
		bool bSetSelected = FALSE;
		for (c0=1;c0<11;c0++)
		{
			md->Type=OT_BUTTON;
			//md->Data=0;
			md->Data=FE_SAVE_CONFIRM;
			// Not greyed.
			md->Choices = (CBYTE*)0;
			md->LabelID = 0;

			//sprintf(dir,"URBAN_%02i.SAV",c0);
			//MUST be this format, according to Sega.
			sprintf(dir,"UR_CHAOS.%03i",c0);


			// NOTE! -1 is a perfectly good value - it means all missions have been done.
			BYTE bMissionID = -2;
			if ( pVMU != NULL )
			{
				bool bRes = pVMU->Flash_ReadFile ( dir, m_pcSaveData, MAX_SIZE_SAVEGAME_FILES );
				if ( bRes )
				{
					char *pcCurPtr;

	//#define DC_FILE_READ(data,size) memcpy ( (void*)(data), (void *)(pcCurPtr), size ); pcCurPtr += (size); dwSize += (size)

					pcCurPtr = m_pcSaveData;


					BYTE version;
					DC_FILE_READ(&version,1);
					// Only v3 and above should exist for DC!
					ASSERT ( version >= 3 );

					DC_FILE_READ(&bMissionID,1);

					// Don't need the rest...
				}
				if ( bMissionID == (BYTE)-2 )
				{
					// Failed for some reason.
					strcpy(ttl,XLAT_str(X_EMPTY));

					// Indicate that there is no existing game.
					md->Data=FE_MAPSCREEN;

					if ( bGreyOutEmpties || bMarkEmptiesAsVMUFull )
					{
						// Grey this out then.
						md->Choices = (CBYTE*)1;
						if ( bMarkEmptiesAsVMUFull )
						{
							// And change the words to VMU FULL
							strcpy(ttl,XLAT_str(X_VMU_FULL));
							md->LabelID = X_VMU_FULL;
						}
					}
					else if ( iFirstBlankSlot == -1 )
					{
						iFirstBlankSlot = menu_state.items;
					}
				}
				else
				{
					FRONTEND_fetch_title_from_id(MISSION_SCRIPT,ttl,bMissionID);
					if ( ttl[0] == '\0' )
					{
						// Oops. Use something sensible.
						strcpy ( ttl, "Urban Chaos" );
					}
				}
			}

			//sprintf(dir,"%d: %s",c0,ttl);
			md->Label=str;
			strcpy(str,ttl);
			str+=strlen(str)+1;
			MENUFONT_Dimensions(md->Label,x,y,-1,iBigFontScale);
			md->X=320-(x>>1);
			md->Y=y2;
			y2+=50;
	#if 0
			if (CompareFileTime(&time,&high_time)>0)
			{
				high_time = time;
				bSetSelected = TRUE;
				menu_state.selected=menu_state.items;
			}
	#else
			if ( bMissionID != (BYTE)-2 )
			{
				if ( bGreyOutEmpties )
				{
					// Loading, so pick the _highest_ mission number.
					if ( ( bMissionID > iBestMissionIDSoFar ) || ( bMissionID == (BYTE)-1 ) )
					{
						iBestMissionIDSoFarPosition = menu_state.items;
						iBestMissionIDSoFar = bMissionID;
					}
				}
				else
				{
					// Saving, so pick the _lowest_ mission number.
					if ( bMissionID < iBestMissionIDSoFar )
					{
						iBestMissionIDSoFarPosition = menu_state.items;
						iBestMissionIDSoFar = bMissionID;
					}
				}
				bSetSelected = TRUE;
				menu_state.selected=menu_state.items;
			}
	#endif

			md++;
			menu_state.items++;
		}
		if ( !bSetSelected )
		{
			// Nothing valid - select the "select VMU" option at the bottom that
			// hasn't been added yet, but will be.
			menu_state.selected=menu_state.items;
		}
		else
		{
			if ( iBestMissionIDSoFarPosition != -1 )
			{
				menu_state.selected = iBestMissionIDSoFarPosition;
			}
		}

		if ( iFirstBlankSlot != -1 )
		{
			// There was a valid first blank slot, and we can select it. So do so by default.
			menu_state.selected = iFirstBlankSlot;
		}
	}
}


// Hack hack hackety hack.
UBYTE m_ubVMUListCtrl[8];
UBYTE m_ubVMUListSlot[8];


// Build the menu of VMU slots.
void FRONTEND_find_VMUs ( void )
{
	CBYTE ttl[_MAX_PATH];
	WIN32_FIND_DATA data;
	HANDLE handle;
	BOOL   ok;
	SLONG	c0;
	MenuData *md=menu_data;
	CBYTE *str=menu_buffer;
	SLONG x,y,y2=0;
	FILETIME time, high_time={0,0};

	menu_state.selected = 0;

	bool bSomethingChanged = TRUE;
	while ( bSomethingChanged )
	{
		bSomethingChanged = RescanDevices();
		if ( bSomethingChanged )
		{
			// Delete any removed devices.
			DeleteInvalidDevice();
		}
	}

	MapleVMU *pvmuCurrent = FindCurrentStorageVMU ( TRUE );


	// Find the VMUs connected.
	int iNumVMUsTotal = 0;
	for ( int iCtrlNum = 0; iCtrlNum < 4; iCtrlNum++ )
	{
		for ( int iVMUNum = 0; iVMUNum < 2; iVMUNum++ )
		{
			// See if this VMU slot exists/is filled.
extern MapleVMU *FindMemoryVMUAt ( int iCtrlNum, int iVMUNum );
			MapleVMU *pVMU = FindMemoryVMUAt ( iCtrlNum, iVMUNum );

			if ( pVMU != NULL )
			{
				iNumVMUsTotal++;

				md->Type=OT_BUTTON;
				md->Data=FE_MAINMENU;		// Special marker.
				// Not greyed.
				md->Choices = (CBYTE*)0;
				md->Label=str;

				strcpy(ttl,XLAT_str(X_VMU_CONTROLLER_SLOT));
				sprintf ( str, ttl, "ABCD"[iCtrlNum], "12"[iVMUNum] );
				str+=strlen(str)+1;
				MENUFONT_Dimensions(md->Label,x,y,-1,BIG_FONT_SCALE);
				md->X=320-(x>>1);
				md->Y=y2;
				y2+=50;

				if ( pvmuCurrent == pVMU )
				{
					// Set the current one.
					menu_state.selected=menu_state.items;
				}

				m_ubVMUListCtrl[menu_state.items] = iCtrlNum;
				m_ubVMUListSlot[menu_state.items] = iVMUNum;

				md++;
				menu_state.items++;
			}
		}
	}

	if ( iNumVMUsTotal == 0 )
	{
		// Tell the user that there are no vmus around.
		md->Type=OT_BUTTON;
		md->Data=FE_BACK;
		// Greyed out.
		md->Choices = (CBYTE*)1;
		md->Label=str;

		strcpy(str,XLAT_str(X_VMU_NOT_PRESENT));
		str+=strlen(str)+1;
		MENUFONT_Dimensions(md->Label,x,y,-1,BIG_FONT_SCALE);
		md->X=320-(x>>1);
		md->Y=y2;
		y2+=50;

		md++;
		menu_state.items++;

		// Make the Cancel the default item.
		menu_state.selected=menu_state.items;


	}

	FRONTEND_recenter_menu();
}

#endif //#else //#ifndef TARGET_DC




CBYTE*	FRONTEND_MissionFilename(CBYTE *script, UBYTE i) {
	MFFileHandle file;
	CBYTE *text, *str=menu_buffer;
	SLONG ver;
	MissionData *mdata = MFnew<MissionData>();
	MenuData *md=menu_data;
	SLONG x,y,y2=0;

	*str=0;

	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);

	i++;

	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			FRONTEND_ParseMissionData(text,ver,mdata);

			if (mdata->District==districts[district_selected][2])
			{
				//
				// Only missions available to play are put in the
				// list nowadays...
				//

				if (mission_hierarchy[mdata->ObjID] & 4)
				{
					i--;
				}
			}
			if (!i) {
				strcpy(str,mdata->fn);
				mission_launch=mdata->ObjID;
				break;
			}

		}
	}
	FileCloseScript();

	MemFree(text);
	MFdelete(mdata);

	return str;

}

void	FRONTEND_MissionHierarchy(CBYTE *script) {
	MFFileHandle file;
	SLONG best_score;
	CBYTE *text;
	SLONG ver;
	MissionData *mdata = MFnew<MissionData>();
	MenuData *md=menu_data;
	UBYTE i=0, j, flag;
	UBYTE newtheme;

	bonus_this_turn = 0;

	// this is always called whenever the complete_point changes, soooo...
	newtheme=3;
	if (complete_point<24) newtheme=2;
	if (complete_point<16) newtheme=1;
	if (complete_point<8) newtheme=0;
	if (newtheme!=menu_theme) {

		menu_theme = newtheme;

		FRONTEND_scr_new_theme(
			menu_back_names  [menu_theme],
			menu_map_names   [menu_theme],
			menu_brief_names [menu_theme],
			menu_config_names[menu_theme]);

		switch(menu_state.mode) {
		case FE_MAINMENU:
			//InitBackImage(menu_back_names[menu_theme]);

			UseBackSurface(screenfull_back);

			break;
		case FE_CONFIG:
		case FE_CONFIG_VIDEO:
		case FE_CONFIG_AUDIO:
		case FE_CONFIG_OPTIONS:
#ifdef WANT_A_KEYBOARD_ITEM
		case FE_CONFIG_INPUT_KB:
#endif
		case FE_CONFIG_INPUT_JP:
		case FE_LOADSCREEN:
		case FE_SAVESCREEN:
#ifdef TARGET_DC
		case FE_VMU_SELECT:
#endif
			//InitBackImage(menu_config_names[menu_theme]);
#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
			UseBackSurface(screenfull_back);
#else
			UseBackSurface(screenfull_config);
#endif
			break;
		case FE_MAPSCREEN:
			//InitBackImage(menu_map_names[menu_theme]);
			UseBackSurface(screenfull_map);
			break;
		default:
			UseBackSurface(screenfull_brief);
			//InitBackImage(menu_brief_names[menu_theme]);
		}
		FRONTEND_kibble_init();
	}



	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);

//	ZeroMemory(mission_hierarchy,sizeof(mission_hierarchy));
	ZeroMemory(district_valid,sizeof(district_valid));
	mission_hierarchy[1]=3; // the root; 1 - exists, 2 - complete, 4 - waiting

#ifdef ANNOYING_HACK_FOR_SIMON

	// this hack does an end run around the hierarchy system
	// it forces fight1, assault1, & testdrive1a (the bronze fighting, driving test and
	// assault course) to be complete before allowing police1 to open up

	SLONG fightID = -1, assaultID = -1, testdriveID = -1, policeID = -1, fight2ID = -1, testdrive3ID = -1;
	SLONG bonusID1 = -1, bonusID2 = -1, bonusID3 = -1;
	SLONG secretIDbreakout = -1, estateID = -1;

	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else
		{
			FRONTEND_ParseMissionData(text,ver,mdata);

			_strlwr(mdata->fn);

			if (strstr(mdata->fn,"ftutor1.ucm"))
			{
				fightID=mdata->ObjID;
			}

			if (strstr(mdata->fn,"assault1.ucm"))
			{
				assaultID=mdata->ObjID;
			}

			if (strstr(mdata->fn,"testdrive1a.ucm"))
			{
				testdriveID=mdata->ObjID;
			}

			if (strstr(mdata->fn,"police1.ucm"))
			{
				policeID=mdata->ObjID;
			}

			if (strstr(mdata->fn,"fight2.ucm"))
			{
				fight2ID=mdata->ObjID;
			}

			if (strstr(mdata->fn,"testdrive3.ucm"))
			{
				testdrive3ID=mdata->ObjID;
			}

			if (strstr(mdata->fn,"gangorder1.ucm"))
			{
				bonusID1=mdata->ObjID;
			}
			if (strstr(mdata->fn,"gangorder2.ucm"))
			{
				bonusID2=mdata->ObjID;
			}
			if (strstr(mdata->fn,"bankbomb1.ucm"))
			{
				bonusID3=mdata->ObjID;
			}

			if (strstr(mdata->fn,"estate2.ucm"))
			{
				estateID=mdata->ObjID;
			}

#ifdef TARGET_DC
			if (strstr(mdata->fn,"album1.ucm"))
			{
				// Breakout! - a secret mission.
				secretIDbreakout = mdata->ObjID;
			}
#endif
		}
	}
	FileCloseScript();

	ASSERT(WITHIN(fightID     , 0, 39));
	ASSERT(WITHIN(fight2ID    , 0, 39));
	ASSERT(WITHIN(assaultID   , 0, 39));
	ASSERT(WITHIN(testdriveID , 0, 39));
	ASSERT(WITHIN(testdrive3ID, 0, 39));
	ASSERT(WITHIN(policeID    , 0, 39));
	ASSERT(WITHIN(bonusID1	  , 0, 39));
	ASSERT(WITHIN(bonusID2	  , 0, 39));
	ASSERT(WITHIN(bonusID3	  , 0, 39));
	ASSERT(WITHIN(estateID	  , 0, 39));
#ifdef TARGET_DC
	ASSERT(WITHIN(secretIDbreakout, 0, 39));
#endif

#endif

#if 0
	best_score        = -INFINITY;
#else
	best_score        = 1000;
#endif
	district_flash    =  -1;
	district_selected =  0;
#ifdef TARGET_DC
	iNextSuggestedMission = -1;
#endif



	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			FRONTEND_ParseMissionData(text,ver,mdata);

			flag=mission_hierarchy[mdata->ObjID]|1; // exists

//			this dodgy method is no longer used, cos it sucks.
//			if (mdata->ObjID<=complete_point) flag|=2; // complete
//			instead, the entire hierarchy is preserved in savegames and
//			completing a mission sets the appropriate flag. ie, the proper way.


#ifdef TARGET_DC
			// If Estate of Emergency has been done, and the secret environment var is set, then
			// expose Breakout to the world....
			// Also, you have to be playing in English. Sorry.
			if ( IsEnglish && ( ( mission_hierarchy[estateID] & 2 ) != 0 ) && ENV_get_value_number ( "cheat_driving_bronze", 0, "" ) )
			{
				// Hurrah!
				mission_hierarchy[secretIDbreakout] |= 4;
			}
			else
			{
				// Nope - not yet.
				mission_hierarchy[secretIDbreakout] = 0;
			}
#endif



#ifndef VERSION_DEMO
			if (mission_hierarchy[mdata->ParentID]&2)
#endif
			{
#ifndef VERSION_DEMO
				if (mdata->ObjID == fight2ID && menu_theme < 1)
				{
					//
					// Ignore this mission until the first theme...
					//
				}
				else
				if (mdata->ObjID == testdrive3ID && menu_theme < 2)
				{
					//
					// Ignore this mission until the last theme...
					//
				}
				else
#endif
				{
					for (j=0;j<40;j++) {
						if (districts[j][2]==mdata->District) {
							district_valid[j]|=1;
							if ( !(mission_hierarchy[mdata->ObjID]&2) &&
								 (mission_hierarchy[mdata->ObjID]&4) )
							{
								district_valid[j]|=2; // highlight zones with ready missions
							}

							/*

							if ((!district_valid[district_selected])||(mdata->ObjID==complete_point+1))
							{
								district_flash=district_selected=j;

								// there's a good chance this is the mission we wanna attempt now
							}

							*/

							break;
						}
					}

					flag|=4; // available
				}
			}

#ifdef ANNOYING_HACK_FOR_SIMON
			if (mdata->ObjID==policeID) {
				if (  (mission_hierarchy[fightID]&2)
					&&(mission_hierarchy[assaultID]&2)
					&&(mission_hierarchy[testdriveID]&2))
					flag|=4;
				else
				{
					flag&=~4;

					//
					// Search for the district for this mission.
					//

					for (j = 0; j < 40; j++)
					{
						if (districts[j][2] == mdata->District)
						{
							district_valid[j] = 0;
						}
					}
				}
			}

#endif


			if (complete_point>=40) // bodge!
				flag|=2;

			mission_hierarchy[mdata->ObjID]=flag;

			if ((flag & 4) && !(flag & 2))	// 4 means available and 2 means complete
			{
				//
				// This mission is active. Where abouts is it in the suggest_order[].
				// The later it is in the suggest order, the later the mission is.
				//
#ifndef VERSION_DEMO
				if ((mdata->ObjID==bonusID1)&&!(bonus_state & 1))
				{
					bonus_state|=1;
					bonus_this_turn = 1;
				}
				if ((mdata->ObjID==bonusID2)&&!(bonus_state & 2))
				{
					bonus_state|=2;
					bonus_this_turn = 1;
				}
				if ((mdata->ObjID==bonusID3)&&!(bonus_state & 4))
				{
					bonus_state|=4;
					bonus_this_turn = 1;
				}
#endif
				SLONG order = 0;

				while(1)
				{
					if (stricmp(mdata->fn, suggest_order[order]) == 0)
					{
						//
						// Found the mission!
						//

#if 0
						// No, this is selecting them in the wrong order.
						if (order > best_score)
#else
						if (order < best_score)
#endif
						{
							best_score = order;
#ifdef TARGET_DC
							// For the savegame.
							iNextSuggestedMission = mdata->ObjID;
#endif

							//
							// Find which district has this id.
							//

							for (j = 0; j < 40; j++)
							{
								if (districts[j][2] == mdata->District)
								{
									district_flash    = j;
									district_selected = j;

									goto found_mission;
								}
							}
						}
					}

					order += 1;

					if (suggest_order[order][0] == '!')
					{
						break;
					}

				  found_mission:;
				}
			}
		}
	}
	FileCloseScript();


#if 0
	if ( !IsEnglish )
	{
		// Never show breakout in French.
		mission_hierarchy[secretIDbreakout] = 0;
	}
#else

	// If Estate of Emergency has been done, and the secret environment var is set, then
	// expose Breakout to the world....
	// Also, you have to be playing in English. Sorry.
	if ( IsEnglish && ( ( mission_hierarchy[estateID] & 2 ) != 0 ) && ENV_get_value_number ( "cheat_driving_bronze", 0, "" ) )
	{
		// Hurrah!
		mission_hierarchy[secretIDbreakout] |= 4;
	}
	else
	{
		// Nope - not yet.
		mission_hierarchy[secretIDbreakout] = 0;
	}

#endif


	MemFree(text);
	MFdelete(mdata);
}

CBYTE	*brief_wav[]=
{
	"none", //0
	"none", //1
	"none", //2
	"none", //3
	"none", //4
	"none", //5
	"policem1.wav",  //6
	"policem.wav",   //7
	"policem.wav",   //8
	"policem2.wav",   //9
	"none", //10
	"none", //11
	"policem3.wav", //12
	"none", //13
	"policem4.wav", //14
	"policem5.wav", //15
	"policem6.wav",  //16
	"policem7.wav",   //17
	"policem8.wav",   //18
	"policem9.wav",   //19
	"policem10.wav", //20
	"policem11.wav", //21
	"policem12.wav", //22
	"policem13.wav", //23
	"policem14.wav", //24
	"policem15.wav", //25
	"policem16.wav",  //26
	"policem17.wav",   //27
	"policem18.wav",   //28
	"roperm19.wav",   //29
	"roperm20.wav",   //30
	"roperm21.wav",   //31
//	"roperm22.wav",   //32
	"roperm23.wav",   //33
	"roperm24.wav",   //34
	""

};
void	FRONTEND_MissionBrief(CBYTE *script, UBYTE i) {
	MFFileHandle file;
	CBYTE *text, *str=menu_buffer;
	SLONG ver;
	MissionData *mdata = MFnew<MissionData>();
	MenuData *md=menu_data;
	SLONG x,y,y2=0;

	*str=0;
	i++;

	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);

	MUSIC_mode(0);

	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			FRONTEND_ParseMissionData(text,ver,mdata);

			if (mdata->District==districts[district_selected][2])
			{
				//
				// Only missions available to play are put in the
				// list nowadays...
				//

				if (mission_hierarchy[mdata->ObjID] & 4)
				{
					i--;
				}
			}

			if (!i) {
				strcpy(str,mdata->brief);
				str+=strlen(str)+1;
				strcpy(str,mdata->ttl);
				menu_state.title=str;
				break;
			}

		}
	}
	FileCloseScript();

	MemFree(text);

	if ( ( mdata->ObjID ) && ( mdata->ObjID<34 ) &&
		 ( 0 != strcmp ( brief_wav[mdata->ObjID], "none" ) ) )
	{
		CBYTE path[_MAX_PATH];
		//MFX_QUICK_wait();
		strcpy(path,GetSpeechPath());



#ifdef TARGET_DC
		strcat(path,pcSpeechLanguageDir);
#else
		// On PC it's always talk2.
		strcat ( path, "talk2\\" );
#endif
		strcat(path,brief_wav[mdata->ObjID]);

		MFX_QUICK_play(path, FALSE);

		// And hang until the thing has actually started playing.
		// Or five seconds, whichever is shorter.
		// The seek time can be really quite high.
		// NOTE! Some of them don't have briefings, so watch it!
		// Things seem to hang nastily if we don't let the briefing happen.
		// Fuck knows why, and I can't get it when debugging, only when
		// emulating a real disk. Which sucks quite badly.
		DWORD dwTimeout = timeGetTime() + 5000;
		while ( TRUE )
		{
extern bool DCLL_stream_has_started_streaming ( void );
			if ( DCLL_stream_has_started_streaming() )
			{
				break;
			}
			if ( ( ( dwTimeout - timeGetTime() ) & 0x80000000 ) != 0 )
			{
				// Timeout!
				TRACE ( "TIMEOUT!" );
				break;
			}
			TRACE ( "w" );
			Sleep(100);
		}
		TRACE ( "w\n" );

		//
		// Set the MEMSTREAM volume to the music volume.
		//

		//extern void MFX_play_at_stream_volume(UWORD, ULONG, ULONG);

		//MFX_play_at_stream_volume(0, S_FRONT_END_LOOP_EDIT, MFX_LOOPED);


#if 1
		// And then wait for half a second anyway, just in case.
		dwTimeout = timeGetTime() + 500;
		while ( TRUE )
		{
			if ( ( ( dwTimeout - timeGetTime() ) & 0x80000000 ) != 0 )
			{
				break;
			}
			TRACE ( "W" );
			Sleep(100);
		}
		TRACE ( "W\n" );
#endif
	}
	else
	{
		// No briefing - stop the normal music.
		MFX_QUICK_stop();
	}


	MFdelete(mdata);


	DCLL_memstream_play();

}


void	FRONTEND_MissionList(CBYTE *script, UBYTE district) {
/*	MFFileHandle file;
	CBYTE *text, *str=menu_buffer;
	SLONG ver;
	MissionData *mdata = MFnew<MissionData>();
//	MenuData *md=menu_data;
//	SLONG x,y,y2=0;
//	UBYTE i=100;

	//district--;

	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);
	mission_count=mission_selected=0;
	file = FileOpen(script);
	while (1) {
		FRONTEND_LoadString(file,text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			FRONTEND_ParseMissionData(text,ver,mdata);
			if (mdata->District==district) {
				*str=mdata->ObjID; str++;
				strcpy(str,mdata->ttl);
				str+=strlen(mdata->ttl)+1;
				mission_count++;
			}

		}
	}
	FileClose(file);

	MemFree(text);
	MFdelete(mdata);
*/
	UBYTE i=0;
	CBYTE *str=menu_buffer;

	mission_count=mission_selected=0;

	while (*mission_cache[i].name)
	{
		if (mission_cache[i].district == district)
		{
			if (mission_hierarchy[mission_cache[i].id] & 4)
			{
				//
				// This mission is available to play.
				//

				mission_selected = mission_count;

				//
				// First byte is the mission ID
				//

				*str++ = mission_cache[i].id;

				//
				// Then comes the mission name.
				//

				strcpy(str,mission_cache[i].name);

				str+=strlen(mission_cache[i].name)+1;

				mission_count++;
			}
		}

		i++;
	}

	/*

	//
	// Default to selecting the last available mission.
	//

	mission_selected = mission_count - 1;

	if (mission_selected < 0)
	{
		mission_selected = 0;
	}

	*/
}

void	FRONTEND_CacheMissionList(CBYTE *script) {
	MFFileHandle file;
	CBYTE *text, *str;
	SLONG ver;
	MissionData *mdata = MFnew<MissionData>();
	UBYTE i=0;

	//district--;

	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);
	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			FRONTEND_ParseMissionData(text,ver,mdata);
			mission_cache[i].district=mdata->District;
			mission_cache[i].id=mdata->ObjID;
			strcpy(mission_cache[i].name,mdata->ttl);
			i++;
		}
	}
	FileCloseScript();

	MemFree(text);
	MFdelete(mdata);

	//*mission_cache[i].name=0;

}


void	FRONTEND_districts(CBYTE *script) {
	MFFileHandle file;
	CBYTE *text, *str=menu_buffer;
	SLONG ver, mapx=0, mapy=0;
	MenuData *md=menu_data;
	SLONG x,y;
	UBYTE i=0,ct,index=0;
	SWORD temp_dist[40][3];
	UBYTE crap_remap[640][10];

	text = (CBYTE*)MemAlloc(4096);
	memset(text,0,4096);

	district_count=0;
	//district_selected=0;
	ZeroMemory(crap_remap,sizeof(crap_remap));

	FileOpenScript();
	while (1) {
		LoadStringScript(text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			i=1;
		} else {
			if (i) {
			  ct=sscanf(text,"%[^=]=%d,%d",str,&mapx,&mapy);
			  if (strlen(str)&&mapx&&mapy&&(ct==3)) {
				  temp_dist[district_count][0]=mapx-16;
				  temp_dist[district_count][1]=mapy-32;
				  temp_dist[district_count][2]=index;
				  crap_remap[mapx][0]++;
				  crap_remap[mapx][ crap_remap[mapx][0] ]=district_count;
				  //districts[district_count][0]=mapx;
				  //districts[district_count][1]=mapy;
				  district_count++;
			  }
			  index++;
			} else
				if ((text[0]=='/')&&(text[1]=='/')) {
					if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
				}

		}
	}
	FileCloseScript();

	if (district_count) {
		i=0;
		for (x=0;x<640;x++)
			for (y=1;y<=crap_remap[x][0];y++) {
				districts[i][0]=temp_dist[crap_remap[x][y]][0];
				districts[i][1]=temp_dist[crap_remap[x][y]][1];
				//districts[i][2]=crap_remap[x][y];
				districts[i][2]=temp_dist[crap_remap[x][y]][2];
				i++;
			}
		FRONTEND_MissionList(script,districts[district_selected][2]);
	}


	MemFree(text);

}


CBYTE*	FRONTEND_gettitle(UBYTE mode, UBYTE selection) {
	RawMenuData *pt=raw_menu_data;
	while (pt->Menu!=mode) pt++;
	for (;selection;selection--,pt++);
	return XLAT_str_ptr(pt->Label);
}

void	FRONTEND_easy(UBYTE mode) {
	SLONG x,y,y2=0;
	RawMenuData *pt=raw_menu_data;
	MenuData *md=menu_data+menu_state.items;

	if (menu_state.items) y2=(md-1)->Y+50;


	int iBigFontScale = BIG_FONT_SCALE;
	if ( !IsEnglish )
	{
		if ( ( menu_state.mode == FE_SAVESCREEN ) ||
			 ( menu_state.mode == FE_LOADSCREEN ) ||
			 ( menu_state.mode == FE_SAVE_CONFIRM ) )
		{
			// Reduce it a bit to fit long French words on screen.
			iBigFontScale = BIG_FONT_SCALE_FRENCH;
		}
	}



	while (pt->Menu!=mode) pt++;
	while ((pt->Menu==mode)||!pt->Menu) {
		md->Type=pt->Type;
		//XLAT_str(pt->Label,md->Label);
		md->LabelID=pt->Label;
		md->Label=XLAT_str_ptr(pt->Label);
		md->Data=pt->Data;
		switch (md->Type) {
		case OT_BUTTON:
			md->Choices=pt->Choices; // secret code... :P
			// Fallthrough.
		case OT_LABEL:
			MENUFONT_Dimensions(md->Label,x,y,-1,iBigFontScale);
			md->X=320-(x>>1);
			break;
		case OT_MULTI:
			md->X=30;
			if (pt->Choices==MC_YN) {
				md->Choices=menu_choice_yesno;
				md->Data|=(2<<8);
			}
			else if (pt->Choices==MC_SCANNER) {
				md->Choices=menu_choice_scanner;
				md->Data|=(2<<8);
			}
#ifdef TARGET_DC
			else if (pt->Choices==MC_ANALOGUE_MODE) {
				md->Choices=menu_choice_analogue_mode;
				md->Data|=(2<<8);
			}
			else if (pt->Choices==MC_JOYPAD_MODE) {
				md->Choices=menu_choice_joypad_mode;
				md->Data|=((NUM_OF_JOYPAD_MODES+1)<<8);
			}
#endif
			break;
		default:
			md->X=30;
		}
		md->Y=y2;
		y2+=50;
		md++;
		pt++;
		menu_state.items++;
	}
	for (md=menu_data;md->Type==OT_LABEL;md++,menu_state.selected++);
#if 0
	// Move to FRONTEND_recenter_menu.
	menu_state.base=240-(menu_state.items*25);
	if (menu_state.base<50) menu_state.base=50;
#endif

	FRONTEND_recenter_menu();
}

//----------------------------------------------------------------------------
// STORE/RESTORE OPTIONS DATA
//

// Video

UBYTE LabelToIndex(SLONG label)
{
	switch(label) {
		case X_STARS:		return 0;
		case X_SHADOWS:		return 1;
		case X_PUDDLES:		return 4;
		case X_DIRT:		return 5;
		case X_MIST:		return 6;
		case X_RAIN:		return 7;
		case X_SKYLINE:		return 8;
		case X_CRINKLES:	return 11;
		case X_MOON:		return 2;
		case X_PEOPLE:		return 3;
		case X_BILINEAR:	return 9;
		case X_PERSPECTIVE:	return 10;
	}
	return 19;
}

void FRONTEND_restore_video_data()
{
	int data[20]; // int for compatability :P
	UBYTE i,j;
#ifdef TARGET_DC
	AENG_get_detail_levels( /*data,*/ data+1, /*data+2, data+3,*/ data+4, data+5, data+6, data+7, data+8, /*data+9, data+10,*/ data+11);
#else
	AENG_get_detail_levels( data, data+1, data+2, data+3, data+4, data+5, data+6, data+7, data+8, data+9, data+10, data+11);
#endif
	for (i=0;i<menu_state.items;i++)
	switch(menu_data[i].LabelID)
	{
/*		case X_RESOLUTION:
			CurrentVidMode=menu_data[i].Data&0xff;
			ShellPauseOn();
			switch(CurrentVidMode)
			{
			case 0:
				SetDisplay(640,480,16);
				break;
			case 1:
				SetDisplay(800,600,16);
				break;
			case 2:
				SetDisplay(1024,768,16);
				break;
			}
			ShellPauseOff();
			break;
		case X_DRIVERS:
			// er...
			break;*/
		case X_STARS:
		case X_SHADOWS:
		case X_PUDDLES:
		case X_DIRT:
		case X_MIST:
		case X_RAIN:
		case X_SKYLINE:
		case X_CRINKLES:
		case X_MOON:
		case X_PEOPLE:
		case X_PERSPECTIVE:
		case X_BILINEAR:
			j=LabelToIndex(menu_data[i].LabelID);
			data[j]|=menu_data[i].Data&0xff00;
			menu_data[i].Data=data[j];
			break;
	}

}

void FRONTEND_store_video_data()
{
	int data[20], i, mode, bit_depth;

	// Get the current ones.
#ifdef TARGET_DC
	AENG_get_detail_levels( /*data,*/ data+1, /*data+2, data+3,*/ data+4, data+5, data+6, data+7, data+8, /*data+9, data+10,*/ data+11);
#else
	AENG_get_detail_levels( data, data+1, data+2, data+3, data+4, data+5, data+6, data+7, data+8, data+9, data+10, data+11);
#endif

	// Override with menu entries:
	for (i=0;i<menu_state.items;i++)
	switch(menu_data[i].LabelID)
	{
		case X_RESOLUTION:
			mode=menu_data[i].Data&0xff;
			break;
		case X_COLOUR_DEPTH:
			bit_depth=menu_data[i].Data&0xff ? 32 : 16;
			break;
		case X_DRIVERS:
			// er...
			break;
		case X_STARS:
		case X_SHADOWS:
		case X_PUDDLES:
		case X_DIRT:
		case X_MIST:
		case X_RAIN:
		case X_SKYLINE:
		case X_CRINKLES:
		case X_MOON:
		case X_PEOPLE:
		case X_PERSPECTIVE:
		case X_BILINEAR:
			data[LabelToIndex(menu_data[i].LabelID)]=menu_data[i].Data&0xff;
			break;
		case X_LOW:

			break;
	}
#ifndef TARGET_DC
	ENV_set_value_number("Mode",mode,"Video");
	ENV_set_value_number("BitDepth",bit_depth,"Video");
#endif
#ifdef TARGET_DC
	AENG_set_detail_levels(/*data[0],*/data[1],/*data[2],data[3],*/data[4],data[5],data[6],data[7],data[8],/*data[9],data[10],*/data[11] );
#else
	AENG_set_detail_levels(data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11] );
#endif

#ifdef ALLOW_DANGEROUS_OPTIONS

	if ((mode!=CurrentVidMode)||(bit_depth!=CurrentBitDepth))
	{
		CurrentVidMode=mode;
		CurrentBitDepth=bit_depth;
		ShellPauseOn();
		switch(CurrentVidMode)
		{
		case 0:
			SetDisplay(640,480,bit_depth);
			break;
		case 1:
			SetDisplay(800,600,bit_depth);
			break;
		case 2:
			SetDisplay(1024,768,bit_depth);
			break;
		case 3:
			SetDisplay(320,240,bit_depth);
			break;
		case 4:
			SetDisplay(512,384,bit_depth);
			break;
		}
		ShellPauseOff();
	}

#endif

}


#ifdef M_SOUND

void	set_miles_drivers() {
	MFX_3D *prov;
	UBYTE   c0;
	UWORD	ct;
	CBYTE  *str=menu_buffer;

	ct=Get3DProviderList(&prov);
	menu_data[3].Data=Get3DProvider()|(ct<<8);
	for (c0=0;c0<ct;c0++) {
		strcpy(str,prov->name);
		str+=strlen(str)+1;
		prov++;
	}
	*str=0;
	menu_data[3].Choices=menu_buffer;
}

#endif


#ifndef TARGET_DC

void	FRONTEND_do_drivers() {
	SLONG			result, count=0, selected=0;
	ChangeDDInfo	*change_info;
	DDDriverInfo	*current_driver=0,
					*driver_list;
	GUID			*DD_guid;
	TCHAR			szBuff[80];
	CBYTE			*str=menu_buffer, *str_tmp;

	switch (RealDisplayWidth)
	{
	case 640:
		CurrentVidMode=0;
		break;
	case 800:
		CurrentVidMode=1;
		break;
	case 1024:
		CurrentVidMode=2;
		break;
	case 320:
		CurrentVidMode=3;
		break;
	case 512:
		CurrentVidMode=4;
		break;

	default:
		CurrentVidMode=0;
		break;
	}
	CurrentBitDepth=DisplayBPP;

	strcpy(str,"640x480");
	str+=strlen(str)+1;
	strcpy(str,"800x600");
	str+=strlen(str)+1;
	strcpy(str,"1024x768");
	str+=strlen(str)+1;
	strcpy(str,"320x240");
	str+=strlen(str)+1;
	strcpy(str,"512x384");
	str+=strlen(str)+1;
	str_tmp=str;
	menu_data[1].Data=CurrentVidMode|(5<<8);
	menu_data[1].Choices=menu_buffer;

	strcpy(str,"16 bit");
	str+=strlen(str)+1;
	strcpy(str,"32 bit");
	str+=strlen(str)+1;
	menu_data[3].Data=((UBYTE)(CurrentBitDepth==32))|(2<<8);
	menu_data[3].Choices=str_tmp;
	str_tmp=str;

	selected=0;

	current_driver=the_manager.CurrDriver;

	// Dump Driver list to Combo Box
	driver_list	=	the_manager.DriverList;
	while(driver_list)
	{
		if(driver_list->IsPrimary())
			wsprintf(szBuff,TEXT("%s (Primary)"),driver_list->szName);
		else
			wsprintf(szBuff,TEXT("%s"),driver_list->szName);

		// Add String to multi-choice
		strcpy(str,szBuff);
		str+=strlen(str)+1;

/*		// Set up pointer to driver for this item
		SendDlgItemMessage(hDlg, IDC_DRIVERS, CB_SETITEMDATA, (WPARAM)result, (LPARAM)(void *)driver_list);*/

		// Is it the current Driver
		if(current_driver==driver_list)
			selected=count;

		count++;
		driver_list	=	driver_list->Next;
	}
	menu_data[2].Data=selected|(count<<8);
	menu_data[2].Choices=str_tmp;

/*	UBYTE   c0;
	UWORD	ct;
	CBYTE  *str=menu_buffer;

	ct=Get3DProviderList(&prov);
	menu_data[3].Data=Get3DProvider()|(ct<<8);
	for (c0=0;c0<ct;c0++) {
		strcpy(str,prov->name);
		str+=strlen(str)+1;
		prov++;
	}
	*str=0;
	menu_data[3].Choices=menu_buffer;*/
}

void	FRONTEND_gamma_update() {
/*	if ((menu_state.selected==11)||(menu_state.selected==12)) {
		the_display.SetGamma(menu_data[11].Data, menu_data[12].Data);
	}*/
	if (menu_state.selected==GammaIndex)
		the_display.SetGamma(menu_data[GammaIndex].Data&0xff, 256);
}

#else //#ifndef TARGET_DC

// Spoof them.
void	FRONTEND_do_drivers() {
}

void	FRONTEND_gamma_update() {
}


#endif //#else //#ifndef TARGET_DC

void	FRONTEND_do_gamma() {
	SLONG x,y,y2=0;
	MenuData keepsafe;
	MenuData *md=menu_data+menu_state.items-1;

	keepsafe=*md; // we're going to insert the extra items before the last ("go back") item

	md->Label=XLAT_str_ptr(X_GAMMA);
	MENUFONT_Dimensions(md->Label,x,y,-1,BIG_FONT_SCALE);
	md->Type=OT_LABEL;
	md->X=320-(x>>1);
	y2=md->Y; md++; y2+=50;

	md->Label=XLAT_str_ptr(X_LOW);
	md->LabelID=X_LOW;
	GammaIndex=md-menu_data;
	MENUFONT_Dimensions(md->Label,x,y,-1,BIG_FONT_SCALE);
	md->Type=OT_SLIDER;
	md->X=30;
	md->Y=y2; md++; y2+=50;

/*	md->Label=XLAT_str_ptr(X_HIGH);
	MENUFONT_Dimensions(md->Label,x,y);
	md->Type=OT_SLIDER;
	md->X=30;
	md->Y=y2; md++; y2+=50;*/

	*md=keepsafe;
	md->Y=y2;

	md++; y2+=50;

	menu_state.items+=2; //3;

	int a,b;
	the_display.GetGamma(&a,&b);
	menu_data[GammaIndex].Data=a;
//	menu_data[12].Data=b;

}

void	FRONTEND_mode(SBYTE mode, bool bDoTransition=TRUE) {


#ifdef TARGET_DC
	if ( mode == FE_MAINMENU )
	{
		// OK, we finally got to the main menu, so disable the immediate soft reset behaviour.
		// From here on, it gets handled explicitely by some code elsewhere in frontend.cpp,
		// rather than the code in interfac.cpp.
extern bool g_bDreamcastABXYStartShouldGoToBootRomImmediately;
		g_bDreamcastABXYStartShouldGoToBootRomImmediately = FALSE;
	}
#endif


	if (menu_state.mode >= 100 && mode < 100)
	{
		//
		// Moving from the briefing screen... reinitialise the music.
		//

		DCLL_memstream_stop();

		//MFX_stop(0, S_FRONT_END_LOOP_EDIT);

#ifdef TARGET_DC
		MFX_QUICK_play("data\\sfx\\1622DC\\GeneralMusic\\FrontLoopMONO.wav",TRUE,0,0,0);
#else
		MFX_QUICK_play("data\\sfx\\1622\\GeneralMusic\\FrontLoopMONO.wav",TRUE,0,0,0);
#endif
	}


	// Reset this now.
	dwAutoPlayFMVTimeout = timeGetTime() + AUTOPLAY_FMV_DELAY;



	SBYTE last=menu_state.mode;
	fade_mode=1;
	ZeroMemory(menu_data,sizeof(menu_data));
	menu_state.items=0;
	if (mode==-2) {
		if (menu_state.stackpos>0) {
			menu_state.stackpos--;
			mode=menu_state.stack[menu_state.stackpos].mode;
			menu_state.scroll=menu_state.stack[menu_state.stackpos].scroll;
			menu_state.selected=menu_state.stack[menu_state.stackpos].selected;
			menu_state.title=menu_state.stack[menu_state.stackpos].title;
		} else {
			mode=menu_state.mode;
		}
	} else {
		if (menu_thrash) {
				menu_state.stack[menu_state.stackpos].mode=menu_thrash;
				menu_state.stack[menu_state.stackpos].scroll=0;
				menu_state.stack[menu_state.stackpos].selected=0;
				menu_state.stack[menu_state.stackpos].title=0;
				menu_state.stackpos++;
				menu_thrash=0;
		} else
			if (menu_state.mode!=-1) {
				menu_state.stack[menu_state.stackpos].mode=menu_state.mode;
				menu_state.stack[menu_state.stackpos].scroll=menu_state.scroll;
				menu_state.stack[menu_state.stackpos].selected=menu_state.selected;
				menu_state.stack[menu_state.stackpos].title=menu_state.title;
				menu_state.stackpos++;
			}
		menu_state.selected=0;
	}

	switch ( mode )
	{
	case FE_MAPSCREEN:
		menu_state.title=XLAT_str_ptr(X_START);
		break;
#ifdef WANT_A_TITLE_SCREEN
	case FE_TITLESCREEN:
	case FE_LANGUAGESCREEN:
#endif
	case FE_MAINMENU:
		// No title.
		menu_state.title=NULL;
		break;
	case FE_LOADSCREEN:
		menu_state.title=XLAT_str_ptr(X_LOAD_GAME);
		break;
	case FE_SAVESCREEN:
		menu_state.title=XLAT_str_ptr(X_SAVE_GAME);
		break;

	#ifdef TARGET_DC
	case FE_VMU_SELECT:
		menu_state.title=XLAT_str_ptr(X_VMU_SELECT);
		break;
	#endif

	default:
		if ( menu_state.stackpos && ( mode < 100 ) )
		{
			menu_state.title=FRONTEND_gettitle(menu_state.stack[menu_state.stackpos-1].mode,menu_state.stack[menu_state.stackpos-1].selected);
		}
		else
		{
			menu_state.title=0;
		}
		break;
	}

	menu_state.mode=mode;

	if (mode>=100) {
		//InitBackImage("TITLE LEAVES1.TGA");
		FRONTEND_init_xition();
		FRONTEND_MissionBrief(MISSION_SCRIPT,mode-100);
//		MUSIC_mode(MUSIC_MODE_BRIEFING|MUSIC_MODE_FORCE);
		return;
	}
	switch(mode) {
	case FE_MAPSCREEN:
		//InitBackImage("MAP SELECT LEAVES.TGA");
		FRONTEND_init_xition();
		FRONTEND_districts(MISSION_SCRIPT);
		//FRONTEND_MissionList(MISSION_SCRIPT);
		//MUSIC_mode(MUSIC_MODE_FRONTEND);
		break;
	case FE_MAINMENU:
		//InitBackImage(menu_back_names[menu_theme]);
		//UseBackSurface(screenfull_back);


		FRONTEND_init_xition();

		FRONTEND_easy(mode);

		// Always reset the stack in the main menu.
		menu_state.stackpos = 0;

		MUSIC_mode(MUSIC_MODE_FRONTEND);
		if (AllowSave) menu_data[2].Choices=NULL;
		break;
#ifdef WANT_A_TITLE_SCREEN
	case FE_TITLESCREEN:
		FRONTEND_init_xition();

		FRONTEND_easy(mode);

		// Always reset the stack in the title screen.
		menu_state.stackpos = 0;

		MUSIC_mode(MUSIC_MODE_FRONTEND);

		// "Reset" the controller so that "Press Start Button" is displayed.
		ClearPrimaryDevice();
		break;

	case FE_LANGUAGESCREEN:
		//UseBackSurface(screenfull_title);

		FRONTEND_init_xition();

		FRONTEND_easy(mode);

		// Always reset the stack in the title screen.
		menu_state.stackpos = 0;

		MUSIC_mode(MUSIC_MODE_FRONTEND);

		// Set the default selection to the current setting.
		menu_state.selected = ENV_get_value_number ( "lang_num", 0, "" );

		break;
#endif
	case FE_SAVESCREEN:
		AllowSave=1;
		if (!menu_state.stackpos)
		{
			//InitBackImage(menu_config_names[menu_theme]);
#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
			UseBackSurface(screenfull_back);
#else
			UseBackSurface(screenfull_config);
#endif
		}
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_find_savegames ( FALSE, TRUE );
		FRONTEND_easy(mode);
		menu_state.title=XLAT_str_ptr(X_SAVE_GAME);
		break;
	case FE_LOADSCREEN:
		FRONTEND_find_savegames ( TRUE, FALSE );
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
		break;
#ifdef TARGET_DC
	case FE_VMU_SELECT:
		FRONTEND_find_VMUs();
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
		break;
#endif
	case FE_CONFIG:
		FRONTEND_init_xition();
		FRONTEND_easy(mode);
		break;
#ifdef WANT_AN_EXIT_MENU_ITEM
	case FE_QUIT:
		FRONTEND_easy(mode);
		menu_state.title=XLAT_str_ptr(X_EXIT);
		break;
#endif
	case FE_CONFIG_VIDEO:
		{
		int a,b,c,d,e,f,g,h,i,j,k; // <-- int for compatability with the prototype in aeng.h *shrug*
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
#ifndef TARGET_DC
#ifdef ALLOW_DANGEROUS_OPTIONS
		FRONTEND_do_drivers();
#endif
#endif
		if (the_display.IsGammaAvailable())
			FRONTEND_do_gamma();
		FRONTEND_restore_video_data();
		}
		break;
	case FE_CONFIG_AUDIO:
		{
		SLONG fx,amb,mus;
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
		// now put in correct values...

/*

#if defined(M_SOUND) || defined(DC_SOUND)
#else
		fx = 127;
		amb = 127;
		mus = 127;
#endif
*/

		MFX_get_volumes(&fx,&amb,&mus);



		menu_data[0].Data=fx<<1;
		menu_data[1].Data=amb<<1;
		menu_data[2].Data=mus<<1;
#ifdef TARGET_DC
		// Load this from the BOOT rom.
		if ( FirmwareGetSoundMode() == SOUND_MODE_STEREO )
		{
			menu_data[3].Data = 1 | (2<<8);
		}
		else
		{
			menu_data[3].Data = 0 | (2<<8);
		}
#endif
#ifdef M_SOUND
#ifdef ALLOW_DANGEROUS_OPTIONS
		set_miles_drivers();
#endif
#endif
		}
		break;
#ifdef WANT_A_KEYBOARD_ITEM
	case FE_CONFIG_INPUT_KB:
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
		// now put in correct values...
		menu_data[0].Data = ENV_get_value_number("keyboard_left",		203, "Keyboard");
		menu_data[1].Data = ENV_get_value_number("keyboard_right",		205, "Keyboard");
		menu_data[2].Data = ENV_get_value_number("keyboard_forward",	200, "Keyboard");
		menu_data[3].Data = ENV_get_value_number("keyboard_back",		208, "Keyboard");
		menu_data[4].Data = ENV_get_value_number("keyboard_punch",		 44, "Keyboard");
		menu_data[5].Data = ENV_get_value_number("keyboard_kick",		 45, "Keyboard");
		menu_data[6].Data = ENV_get_value_number("keyboard_action",		 46, "Keyboard");
//		menu_data[7].Data = ENV_get_value_number("keyboard_run",		 47, "Keyboard");
		menu_data[7].Data = ENV_get_value_number("keyboard_jump",		 57, "Keyboard");
		menu_data[8].Data = ENV_get_value_number("keyboard_start",		 15, "Keyboard");
		menu_data[9].Data = ENV_get_value_number("keyboard_select",	 28, "Keyboard");
		// gap for label
		menu_data[11].Data = ENV_get_value_number("keyboard_camera",	207, "Keyboard");
		menu_data[12].Data = ENV_get_value_number("keyboard_cam_left",	211, "Keyboard");
		menu_data[13].Data= ENV_get_value_number("keyboard_cam_right",	209, "Keyboard");
		menu_data[14].Data= ENV_get_value_number("keyboard_1stperson",	 30, "Keyboard");
		break;
#endif
	case FE_CONFIG_INPUT_JP:
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
#ifndef TARGET_DC
		menu_data[0].Data = ENV_get_value_number("joypad_kick",		4, "Joypad");
		menu_data[1].Data = ENV_get_value_number("joypad_punch",	3, "Joypad");
		menu_data[2].Data = ENV_get_value_number("joypad_jump",		0, "Joypad");
		menu_data[3].Data = ENV_get_value_number("joypad_action",	1, "Joypad");
		menu_data[4].Data = ENV_get_value_number("joypad_move",		7, "Joypad");
		menu_data[5].Data = ENV_get_value_number("joypad_start",	8, "Joypad");
		menu_data[6].Data = ENV_get_value_number("joypad_select",	2, "Joypad");
		// gap for label
		menu_data[8].Data = ENV_get_value_number("joypad_camera",	6, "Joypad");
		menu_data[9].Data = ENV_get_value_number("joypad_cam_left",	9, "Joypad");
		menu_data[10].Data = ENV_get_value_number("joypad_cam_right",10, "Joypad");
		menu_data[11].Data =ENV_get_value_number("joypad_1stperson",5, "Joypad");
#else
		// Set up the buttons from the environment.
		{
			int iMode = ENV_get_value_number("joypad_mode",		0, "Joypad");
			INTERFAC_SetUpJoyPadButtons ( iMode );
			// And set up the menu to match.
			menu_data[0].Data = iMode | ((NUM_OF_JOYPAD_MODES+1)<<8);
			// Gap for label.
			menu_data[2].Data  = joypad_button_use[JOYPAD_BUTTON_PUNCH];
			menu_data[3].Data  = joypad_button_use[JOYPAD_BUTTON_KICK];
			menu_data[4].Data  = joypad_button_use[JOYPAD_BUTTON_JUMP];
			menu_data[5].Data  = joypad_button_use[JOYPAD_BUTTON_ACTION];
			menu_data[6].Data  = joypad_button_use[JOYPAD_BUTTON_MOVE];
			menu_data[7].Data  = joypad_button_use[JOYPAD_BUTTON_SELECT];
			// Gap for label.
			menu_data[9].Data = joypad_button_use[JOYPAD_BUTTON_CAMERA];
			menu_data[10].Data = joypad_button_use[JOYPAD_BUTTON_CAM_LEFT];
			menu_data[11].Data = joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT];
			menu_data[12].Data = joypad_button_use[JOYPAD_BUTTON_1STPERSON];
		}
#endif
		break;
	case FE_CONFIG_OPTIONS:
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
#ifdef TARGET_DC
		menu_data[0].Data|=ENV_get_value_number("scanner_follows",		1, "Game");
		menu_data[1].Data|=ENV_get_value_number("analogue_pad_mode",	0, "Game");
		menu_data[2].Data|=ENV_get_value_number("vibration_mode",		1, "Game");
		menu_data[3].Data|=ENV_get_value_number("vibration_engine",		1, "Game");
#else
		menu_data[1].Data|=ENV_get_value_number("scanner_follows",		1, "Game");
#endif
		break;
	case FE_SAVE_CONFIRM:
		if ( bDoTransition )
		{
			FRONTEND_init_xition();
		}
		FRONTEND_easy(mode);
		menu_state.scroll=0;
		menu_state.title=XLAT_str_ptr(X_SAVE_GAME);
		break;
	}

	FRONTEND_recenter_menu();
}




void	FRONTEND_draw_districts() {
	UBYTE i,j,id;
	SWORD x,y;
	CBYTE *str;
	UWORD fade;
	ULONG rgb;

	if (bonus_this_turn)
	{
		if (bonus_this_turn==1)
		{
			bonus_this_turn++;
			MFX_play_stereo(0,S_TUNE_BONUS,0);
		}
		fade = (64 - fade_state) << 2;
		if (IsEnglish)
		{
			str="Bonus mission unlocked!"; // XLAT_str(X_BONUS_MISSION_UNLOCKED)
			FONT2D_DrawStringCentred(str,322,10,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
#ifdef TARGET_DC
			// A bit darker please.
			FONT2D_DrawStringCentred(str,322,6,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
			FONT2D_DrawStringCentred(str,318,10,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
			FONT2D_DrawStringCentred(str,318,6,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
#endif
			FONT2D_DrawStringCentred(str,320,8,0xffffff,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
		}
	}


	fade = (64 - fade_state) << 2;

	{
		CBYTE	str2[200];
		sprintf(str2,"%s: %03d  %s: %03d  %s: %03d  %s: %03d\n",XLAT_str_ptr(X_CON_INCREASED),the_game.DarciConstitution,XLAT_str_ptr(X_STA_INCREASED),the_game.DarciStamina,XLAT_str_ptr(X_STR_INCREASED),the_game.DarciStrength,XLAT_str_ptr(X_REF_INCREASED),the_game.DarciSkill);
		int iYpos;
		if ( eDisplayType == DT_NTSC )
		{
			// Move it further up so it's on screen for the yanks.
			iYpos = 434;
		}
		else
		{
			iYpos = 454;
		}
		FONT2D_DrawStringCentred(str2,320+2,iYpos+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);
#ifdef TARGET_DC
		// A bit darker please.
		FONT2D_DrawStringCentred(str2,320+2,iYpos-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);
		FONT2D_DrawStringCentred(str2,320-2,iYpos-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);
		FONT2D_DrawStringCentred(str2,320-2,iYpos+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);
#endif
		FONT2D_DrawStringCentred(str2,320  ,iYpos  ,0xffffff,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);
	}

	for (i=0;i<district_count;i++) {
		switch(district_valid[i]) {
		case 1:
			FRONTEND_draw_button(districts[i][0],districts[i][1],(UBYTE)(i==district_selected)|4,i==district_flash);
			break;
		case 2:
		case 3:
			FRONTEND_draw_button(districts[i][0],districts[i][1],(i==district_selected)?5:6,i==district_flash);
			break;
		}

		/*

		{
			CBYTE num[16];

			sprintf(num, "%d", i);

			FONT2D_DrawString(num, districts[i][0],districts[i][1],0xffffff);
		}

		*/

		if (i==district_selected)
		{
			y = districts[i][1];
			x = districts[i][0];

			str = menu_buffer;

			for (j = 0; j < mission_count; j++)
			{
				id = *str++;

				//
				// What colour do we draw this bit of text?
				//

				if (j == mission_selected)
				{
					rgb = 0xffffff;
				}
				else
				{
					rgb = 0x667788;
				}

			  /*

			  rgb=FRONTEND_fix_rgb(fade_rgb,j==mission_selected);
			  fade=(j==mission_selected)?0:127;


			  */


				fade = (64 - fade_state) << 2;

				if (fade > 255) fade = 255;

				if (mission_hierarchy[id] & 4) // 4 => available.
				{
					if (x>320)
					{
						FONT2D_DrawStringRightJustify(str,x+18+2,y+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
#ifdef TARGET_DC
						// A bit darker please.
						FONT2D_DrawStringRightJustify(str,x+18-2,y+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
						FONT2D_DrawStringRightJustify(str,x+18-2,y-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
						FONT2D_DrawStringRightJustify(str,x+18+2,y-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
#endif
						FONT2D_DrawStringRightJustify(str,x+18,y,rgb,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);

						//
						// Draw a line through completed missions!
						//

// These are just tweaked values - dunno why they work/don't work. Just accept it.
#define STRIKETHROUGH_HANGOVER_L 5
#define STRIKETHROUGH_HANGOVER_R 25

						if (mission_hierarchy[id] & 2)	// 2 => complete
						{
							FONT2D_DrawStrikethrough(
								FONT2D_leftmost_x + 2 - STRIKETHROUGH_HANGOVER_L,
								x                 + 2 + STRIKETHROUGH_HANGOVER_R,
								y                 + 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, FALSE);

#ifdef TARGET_DC
							// Darker.
							FONT2D_DrawStrikethrough(
								FONT2D_leftmost_x - 2 - STRIKETHROUGH_HANGOVER_L,
								x                 - 2 + STRIKETHROUGH_HANGOVER_R,
								y                 + 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
							FONT2D_DrawStrikethrough(
								FONT2D_leftmost_x - 2 - STRIKETHROUGH_HANGOVER_L,
								x                 - 2 + STRIKETHROUGH_HANGOVER_R,
								y                 - 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
							FONT2D_DrawStrikethrough(
								FONT2D_leftmost_x + 2 - STRIKETHROUGH_HANGOVER_L,
								x                 + 2 + STRIKETHROUGH_HANGOVER_R,
								y                 - 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
#endif

							FONT2D_DrawStrikethrough(
								FONT2D_leftmost_x + 0 - STRIKETHROUGH_HANGOVER_L,
								x				  + 0 + STRIKETHROUGH_HANGOVER_R,
								y                ,
								rgb,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
						}
					}
					else
					{
						FONT2D_DrawString(str,x+32+2,y+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
#ifdef TARGET_DC
						// A bit darker please.
						FONT2D_DrawString(str,x+32-2,y+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
						FONT2D_DrawString(str,x+32-2,y-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
						FONT2D_DrawString(str,x+32+2,y-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D, fade);
#endif
						FONT2D_DrawString(str,x+32,y,rgb,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,          fade);

						//
						// Draw a line through completed missions!
						//

#undef STRIKETHROUGH_HANGOVER_L
#undef STRIKETHROUGH_HANGOVER_R
// These are just tweaked values - dunno why they work/don't work. Just accept it.
#define STRIKETHROUGH_HANGOVER_L (-15)
#define STRIKETHROUGH_HANGOVER_R 30

						if (mission_hierarchy[id] & 2)	// 2 => complete
						{
							FONT2D_DrawStrikethrough(
								x                  + 2 - STRIKETHROUGH_HANGOVER_L,
								FONT2D_rightmost_x + 2 + STRIKETHROUGH_HANGOVER_R,
								y                  + 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, FALSE);
#ifdef TARGET_DC
						// A bit darker please.
							FONT2D_DrawStrikethrough(
								x                  - 2 - STRIKETHROUGH_HANGOVER_L,
								FONT2D_rightmost_x - 2 + STRIKETHROUGH_HANGOVER_R,
								y                  + 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
							FONT2D_DrawStrikethrough(
								x                  - 2 - STRIKETHROUGH_HANGOVER_L,
								FONT2D_rightmost_x - 2 + STRIKETHROUGH_HANGOVER_R,
								y                  - 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
							FONT2D_DrawStrikethrough(
								x                  + 2 - STRIKETHROUGH_HANGOVER_L,
								FONT2D_rightmost_x + 2 + STRIKETHROUGH_HANGOVER_R,
								y                  - 2,
								0x000000,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
#endif

							FONT2D_DrawStrikethrough(
								x				   - STRIKETHROUGH_HANGOVER_L,
								FONT2D_rightmost_x + STRIKETHROUGH_HANGOVER_R,
								y,
								rgb,
								256,
								POLY_PAGE_FONT2D,
								fade, TRUE);
						}
					}
				}

				str+=strlen(str)+1;
#ifdef TRUETYPE
				y += GetTextHeightTT() * 3/4;
#else
				y+=20;
#endif
			}
		}
	}
}


void FRONTEND_shadowed_text ( char *pcString, int iX, int iY, DWORD dwColour )
{
		FONT2D_DrawString(
			pcString,
			iX+2,
			iY+2,
			0,
			256,
			POLY_PAGE_FONT2D,
			0);

		FONT2D_DrawString(
			pcString,
			iX-1,
			iY-1,
			0,
			256,
			POLY_PAGE_FONT2D,
			0);

		FONT2D_DrawString(
			pcString,
			iX+1,
			iY+1,
			( dwColour & 0xfefefefe ) >> 1,
			256,
			POLY_PAGE_FONT2D,
			0);

		FONT2D_DrawString(
			pcString,
			iX,
			iY,
			dwColour,
			256,
			POLY_PAGE_FONT2D,
			0);
}


void	FRONTEND_display()
{
	UBYTE i;
	SLONG rgb, x,x2,y;
	MenuData *md=menu_data;
	UBYTE whichmap[]={2,0,1,3};
	UBYTE arrow=0;


	DumpTracies();


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
#endif



	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET);
//#ifdef TARGET_DC
	// Must do these inside a frame on DC!
	//POLY_frame_init(FALSE, FALSE);
//#endif
	ShowBackImage();
	if ((fade_mode&3)==1) FRONTEND_show_xition();
#ifndef TARGET_DC
	POLY_frame_init(FALSE, FALSE);
#endif
#ifdef WANT_A_TITLE_SCREEN
	if ( ( menu_state.mode == FE_TITLESCREEN ) || ( menu_state.mode == FE_LANGUAGESCREEN ) )
	{
#if 0
		// Title screen - draw misc copyright notices on screen.
		FRONTEND_shadowed_text ( XLAT_str_ptr(X_COPYRIGHT_1), 32, 32+00, 0xffd0d0d0 );
		FRONTEND_shadowed_text ( XLAT_str_ptr(X_COPYRIGHT_2), 32, 32+20, 0xffd0d0d0 );
		FRONTEND_shadowed_text ( XLAT_str_ptr(X_COPYRIGHT_3), 32+20, 32+38, 0xffd0d0d0 );
		FRONTEND_shadowed_text ( XLAT_str_ptr(X_COPYRIGHT_4), 32, 32+60, 0xffd0d0d0 );
#endif
	}
	else
#endif
	{
		FRONTEND_kibble_draw();
		//POLY_frame_draw(FALSE, FALSE);
	}


	int iBigFontScale = BIG_FONT_SCALE;
	if ( !IsEnglish )
	{
		if ( ( menu_state.mode == FE_SAVESCREEN ) ||
			 ( menu_state.mode == FE_LOADSCREEN ) ||
			 ( menu_state.mode == FE_SAVE_CONFIRM ) )
		{
			// Reduce it a bit to fit long French words on screen.
			iBigFontScale = BIG_FONT_SCALE_FRENCH;
		}
	}


#ifdef TARGET_DC
	// And a special kludge for the vibro pack options.
	static bool bVibrationPackPresent = TRUE;
	if ( menu_state.mode == FE_CONFIG_OPTIONS )
	{
		static int iVibroScanCountdown = 0;
		if ( ( ( iVibroScanCountdown++ ) & 0xf ) == 0 )
		{
			// Flush the VMU device "cache".
			RescanDevices();
			// and rescan primary for a vibro pack.
			if ( FindFirstVibratorOnCurrentController() == NULL )
			{
				bVibrationPackPresent = FALSE;
			}
			else
			{
				bVibrationPackPresent = TRUE;
			}
		}
	}
#endif


	//POLY_frame_init(FALSE, FALSE);
	for (i=0;i<menu_state.items;i++,md++) {
		y=md->Y+menu_state.base-menu_state.scroll;
		if ((y>=100)&&(y<=400)) {
			rgb=FRONTEND_fix_rgb(fade_rgb,i==menu_state.selected);
			if ((md->Type==OT_BUTTON)&&(md->Choices==(CBYTE*)1)) // 'greyed out'
			{
				SLONG rgbtemp=rgb&0xff000000;
				//rgb>>=25; //rgb<<=1;
				rgb=(rgb&0xff)>>1;
				rgb|=(rgb<<8)|(rgb<<16);
				rgb|=rgbtemp;
			}

#ifdef TARGET_DC
			if ( !bVibrationPackPresent &&
				 ( ( md->LabelID == X_VIBRATION ) ||
				   ( md->LabelID == X_VIBRATION_ENG ) ) )
			{
				// Grey this option out - no vibro pack.
				SLONG rgbtemp=rgb&0xff000000;
				//rgb>>=25; //rgb<<=1;
				rgb=(rgb&0xff)>>1;
				rgb|=(rgb<<8)|(rgb<<16);
				rgb|=rgbtemp;
			}

			// Detect the (VMU full) message and alternate between that
			// and (2 blocks required).
			char *pString = md->Label;
			int iXPos = md->X;
			if ( ( md->LabelID == X_VMU_FULL ) && ( ( timeGetTime() & 2048 ) == 0 ) )
			{
				// Change it to the other one.
				pString = XLAT_str ( X_VMU_X_BLOCKS_REQUIRED );
				SLONG iX, iY;
				MENUFONT_Dimensions(pString,iX,iY,-1,iBigFontScale);
				iXPos=320-(iX>>1);
			}
			MENUFONT_Draw(iXPos,y,iBigFontScale,pString,rgb,0);
#else
			MENUFONT_Draw(md->X,y,iBigFontScale,md->Label,rgb,0);
#endif

#ifdef TARGET_DC
			if ( i==menu_state.selected )
			{
				// Draw a rectangle around the text.
				MENUFONT_Draw_Selection_Box(md->X,y,iBigFontScale,md->Label,rgb,0);
			}
#endif
			switch (md->Type) {
			case OT_SLIDER:		FRONTEND_DrawSlider(md);	break;
			case OT_MULTI:		FRONTEND_DrawMulti(md,rgb);		break;
			case OT_KEYPRESS:	FRONTEND_DrawKey(md);		break;
			case OT_PADPRESS:	FRONTEND_DrawPad(md);		break;
			}
		} else {
			if (i==menu_state.selected) { // better do some scrolling
#ifdef TARGET_DC
				// As the man say, it's so slow I want to knaw out my own liver rather than wait for this.
				if (y<100) menu_state.scroll-=20;
				if (y>400) menu_state.scroll+=20;
#else
				if (y<100) menu_state.scroll-=10;
				if (y>400) menu_state.scroll+=10;
#endif
			}
			if (y<100) arrow|=1;
			if (y>400) arrow|=2;
		}
	}
	if (arrow&1) // draw a "more..." arrow at the top of the screen
	{
		DRAW2D_Tri(320,50, 335,65, 305,65,fade_rgb,0);
	}
	if (arrow&2) // draw a "more..." arrow at the bottom of the screen
	{
		DRAW2D_Tri(320,430, 335,415, 305,415,fade_rgb,0);
	}
	if (menu_state.title && (menu_state.mode != FE_MAINMENU)) {
		BOOL dir;
		x2=SIN(fade_state<<3)>>10;
		switch(menu_state.mode) {
		case FE_MAPSCREEN:
			MENUFONT_Dimensions(menu_state.title,x,y,-1,BIG_FONT_SCALE);
			x=560-x;
			x2=(x2*10)-63;
			dir=0;
			break;
		default:
			x=80;
			x2=642-(x2*10);
			dir=1;
			break;
		}
		FontPage=POLY_PAGE_NEWFONT;
		FRONTEND_draw_title(x+2,44,x2,menu_state.title,0,dir);
#ifndef TARGET_DC
		POLY_frame_draw(FALSE, FALSE);
		POLY_frame_init(FALSE, FALSE);
#endif
		FontPage=POLY_PAGE_NEWFONT_INVERSE;
		FRONTEND_draw_title(x,40,x2,menu_state.title,1,dir);
#ifndef TARGET_DC
		POLY_frame_draw(FALSE, FALSE);
		POLY_frame_init(FALSE, FALSE);
#endif
		FRONTEND_draw_button(x2,8,whichmap[menu_theme]);
	}
	if ((menu_state.mode==FE_MAPSCREEN)&&((fade_mode==2)||(fade_state==63))) FRONTEND_draw_districts();
	if ((menu_state.mode>=100)&&*menu_buffer) {
		//DRAW2D_Box(0,48,640,432,fade_rgb&0xff000000,1,127);
#ifndef TARGET_DC
		POLY_frame_draw(FALSE, FALSE);
		POLY_frame_init(FALSE, FALSE);
#endif
		x=SIN(fade_state<<3)>>10;
		FRONTEND_draw_button(642-(x*10),8,whichmap[menu_theme]);
		FONT2D_DrawStringWrapTo(menu_buffer,20+2,100+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,255-(fade_state<<2),402);
#ifdef TARGET_DC
		// Darker Egor - MUCH darker!
		FONT2D_DrawStringWrapTo(menu_buffer,20-2,100-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,255-(fade_state<<2),398);
		FONT2D_DrawStringWrapTo(menu_buffer,20-2,100+2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,255-(fade_state<<2),398);
		FONT2D_DrawStringWrapTo(menu_buffer,20+2,100-2,0x000000,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,255-(fade_state<<2),402);
#endif
		FONT2D_DrawStringWrapTo(menu_buffer,20,100,fade_rgb,SMALL_FONT_SCALE,POLY_PAGE_FONT2D,255-(fade_state<<2),400);
	}

	if ( m_bMovingPanel )
	{
		// Display the panel at the current position.

		// REMEMBER THAT THESE NUMBERS ARE DIVIDED BY 4!
		int iXPos = ENV_get_value_number ( "panel_x", 32 / 4, "" );
		int iYPos = ENV_get_value_number ( "panel_y", (480 - 32) / 4, "" );

extern void PANEL_draw_quad( float left,float top,float right,float bottom,SLONG page,ULONG colour,
				float u1,float v1,float u2,float v2);

		PANEL_draw_quad(
			(float)( iXPos * 4 + 0 ),
			(float)( iYPos * 4 - 165 ),
			(float)( iXPos * 4 + 212 ),
			(float)( iYPos * 4 - 0 ),
			POLY_PAGE_LASTPANEL_ALPHA,
			0xffffffff,
			0.0F,
			90.0F / 256.0F,
			212.0F / 256.0F,
			1.0F);
	}


#ifdef TARGET_DC
	// Draw a "quick info" text screen.
	if ( pcQuickInfo[0] != '\0' )
	{

#if 1
// This blows goats. But there you go.
		if ( bQuickInfoReplaceWithSegasMadMessage )
		{
			// The big essay.
			// Centre it on all sides.
			SLONG iXSize[3], iYSize = 0;
			char *pcString[3];
			if ( !IsEnglish )
			{
				pcString[0] = "Une manette vient d'etre"; 		// NOTE! The E in etre should be �, but the font doesn't have it, and it's all in caps anyway.
				if ( bWriteVMInsteadOfVMU )
				{
					pcString[1] = "retir�e ou une VM est";
				}
				else
				{
					pcString[1] = "retir�e ou une VMU est";
				}
				pcString[2] = "en cours de d�tection";
			}
			else
			{
				pcString[0] = "The controller has been";
				if ( bWriteVMInsteadOfVMU )
				{
					pcString[1] = "removed or a VM";
				}
				else
				{
					pcString[1] = "removed or a VMU";
				}
				pcString[2] = "is being recognised";
			}

			for ( int i = 0; i < 3; i++ )
			{
				SLONG iYTemp;
				MENUFONT_Dimensions ( pcString[i], iXSize[i], iYTemp, 640, BIG_FONT_SCALE );
				iYSize += iYTemp;
			}

			// Darken the whole damn screen.
			PANEL_draw_quad(
				0.0f,
				0.0f,
				640.0f,
				480.0f,
				POLY_PAGE_ALPHA,
				0xc0000000,
				0.0f,
				0.0f,
				0.0f,
				0.0f);

			MENUFONT_Draw ( (640-iXSize[0] ) >> 1, (480-iYSize ) >> 1, BIG_FONT_SCALE, pcString[0], 0xffffffff, 0 );
			MENUFONT_Draw ( (640-iXSize[1] ) >> 1, (480-0      ) >> 1, BIG_FONT_SCALE, pcString[1], 0xffffffff, 0 );
			MENUFONT_Draw ( (640-iXSize[2] ) >> 1, (480+iYSize ) >> 1, BIG_FONT_SCALE, pcString[2], 0xffffffff, 0 );

		}
		else
#endif
		{
			// Put at almost bottom centre - got to avoid the outer 16 pixels.
			// Also avoid the ConStaStrRef stuff as well.
			SLONG iXSize, iYSize;
			MENUFONT_Dimensions ( pcQuickInfo, iXSize, iYSize, 640, BIG_FONT_SCALE );

			// If on the title screen, move it to the top.
			int iYpos;
			if ( ( menu_state.mode == FE_TITLESCREEN ) || ( menu_state.mode == FE_LANGUAGESCREEN ) )
			{
				iYpos = 32+16;
			}
			else
			{
				iYpos = 480-32-16;
			}
			MENUFONT_Draw_Selection_Box	( (640-iXSize ) >> 1, iYpos, BIG_FONT_SCALE, pcQuickInfo, 0xffffffff, 0 );
			MENUFONT_Draw				( (640-iXSize ) >> 1, iYpos, BIG_FONT_SCALE, pcQuickInfo, 0xffffffff, 0 );
		}

		if ( (signed)( timeGetTime() - dwQuickInfotimeGetTimeExpires ) > 0 )
		{
			pcQuickInfo[0] = '\0';
			bQuickInfoReplaceWithSegasMadMessage = FALSE;
		}
	}
#endif






#ifdef TARGET_DC
	// Just for non-final builds.
	static CBYTE version_number[64] = "";

	if ( version_number[0] == '\0' )
	{
		CBYTE ts[40] = __DATE__;
		float vn;

		CBYTE *month[12] =
		{
			"Jan",
			"Feb",
			"Mar",
			"Apr",
			"May",
			"Jun",
			"Jul",
			"Aug",
			"Sep",
			"Oct",
			"Nov",
			"Dec"
		};

		SLONG i;

		vn = 0.0F;

		for (i = 0; i < 12; i++)
		{
			if (toupper(ts[0]) == toupper(month[i][0]) &&
				toupper(ts[1]) == toupper(month[i][1]) &&
				toupper(ts[2]) == toupper(month[i][2]))
			{
				vn = i - 8.0F;
			}
		}

		SLONG day = atoi(ts + 4);

		vn += day * 0.03F;

		SLONG year = atoi(ts + 7);

		vn += (year - 1999) * 12;

		sprintf(version_number, "Version %.2f", vn );
	}


	static bool bShowVersionNumInFrontEnd = FALSE;
extern int g_iCheatNumber;
	if ( g_iCheatNumber == 0x1a1a0002 )
	{
		bShowVersionNumInFrontEnd = TRUE;
		g_iCheatNumber = 0;
	}

	if ( bShowVersionNumInFrontEnd )
	{
		FONT2D_DrawString(
			version_number,
			20,
			20,
			0xffffffff);
	}

#endif




#ifndef TARGET_DC
	POLY_frame_draw(FALSE, FALSE);
#endif

}


void	FRONTEND_storedata() {
	switch(menu_state.mode) {
	case FE_CONFIG_VIDEO:
		FRONTEND_store_video_data();
		break;
	case FE_CONFIG_AUDIO:
		MFX_stop(WEATHER_REF,MFX_WAVE_ALL);
		//MFX_stop(SIREN_REF,MFX_WAVE_ALL);
//		MFX_stop(MFX_CHANNEL_ALL,MFX_WAVE_ALL);
//		MFX_free_wave_list();

		MFX_set_volumes(menu_data[0].Data>>1,menu_data[1].Data>>1,menu_data[2].Data>>1);
#ifdef TARGET_DC
		if ( ( menu_data[3].Data & 0xff ) == 0 )
		{
			// Mono.
			SetFirmwareValues ( DWORD_DONT_CHANGE, BYTE_DONT_CHANGE, BYTE_DONT_CHANGE, SOUND_MODE_MONO, BYTE_DONT_CHANGE );
		}
		else
		{
			// Stereo
			SetFirmwareValues ( DWORD_DONT_CHANGE, BYTE_DONT_CHANGE, BYTE_DONT_CHANGE, SOUND_MODE_STEREO, BYTE_DONT_CHANGE );
		}
#endif

#ifdef ALLOW_DANGEROUS_OPTIONS
		Set3DProvider(menu_data[3].Data&0xff);
#endif


		break;

#ifdef WANT_A_KEYBOARD_ITEM
	case FE_CONFIG_INPUT_KB:
		ENV_set_value_number("keyboard_left",		menu_data[0].Data, "Keyboard");
		ENV_set_value_number("keyboard_right",		menu_data[1].Data, "Keyboard");
		ENV_set_value_number("keyboard_forward",	menu_data[2].Data, "Keyboard");
		ENV_set_value_number("keyboard_back",		menu_data[3].Data, "Keyboard");
		ENV_set_value_number("keyboard_punch",		menu_data[4].Data, "Keyboard");
		ENV_set_value_number("keyboard_kick",		menu_data[5].Data, "Keyboard");
		ENV_set_value_number("keyboard_action",		menu_data[6].Data, "Keyboard");
//		ENV_set_value_number("keyboard_run",		menu_data[7].Data, "Keyboard");
		ENV_set_value_number("keyboard_jump",		menu_data[7].Data, "Keyboard");
		ENV_set_value_number("keyboard_start",		menu_data[8].Data, "Keyboard");
		ENV_set_value_number("keyboard_select",		menu_data[9].Data, "Keyboard");
		// gap for label
		ENV_set_value_number("keyboard_camera",		menu_data[11].Data, "Keyboard");
		ENV_set_value_number("keyboard_cam_left",	menu_data[12].Data, "Keyboard");
		ENV_set_value_number("keyboard_cam_right",	menu_data[13].Data , "Keyboard");
		ENV_set_value_number("keyboard_1stperson",	menu_data[14].Data, "Keyboard");
		break;
#endif

	case FE_CONFIG_INPUT_JP:
#ifdef TARGET_DC
		// Save the preset mode.
		ENV_set_value_number("joypad_mode", (menu_data[0].Data & 0xff) , "Joypad");
		if ( ( menu_data[0].Data & 0xff ) == 0 )
		{
			// Custom mode. Save the custom buttons too.
			ENV_set_value_number("joypad_punch",		menu_data[2].Data, "Joypad");
			ENV_set_value_number("joypad_kick",			menu_data[3].Data, "Joypad");
			ENV_set_value_number("joypad_jump",			menu_data[4].Data, "Joypad");
			ENV_set_value_number("joypad_action",		menu_data[5].Data, "Joypad");
			ENV_set_value_number("joypad_move",			menu_data[6].Data, "Joypad");
			ENV_set_value_number("joypad_select",		menu_data[7].Data, "Joypad");
			// gap for label
			ENV_set_value_number("joypad_camera",		menu_data[9].Data, "Joypad");
			ENV_set_value_number("joypad_cam_left",		menu_data[10].Data, "Joypad");
			ENV_set_value_number("joypad_cam_right",	menu_data[11].Data , "Joypad");
			ENV_set_value_number("joypad_1stperson",	menu_data[12].Data, "Joypad");
		}
#else
		ENV_set_value_number("joypad_kick",			menu_data[0].Data, "Joypad");
		ENV_set_value_number("joypad_punch",		menu_data[1].Data, "Joypad");
		ENV_set_value_number("joypad_jump",			menu_data[2].Data, "Joypad");
		ENV_set_value_number("joypad_action",		menu_data[3].Data, "Joypad");
		ENV_set_value_number("joypad_move",			menu_data[4].Data, "Joypad");
		ENV_set_value_number("joypad_start",		menu_data[5].Data, "Joypad");
		ENV_set_value_number("joypad_select",		menu_data[6].Data, "Joypad");
		// gap for label
		ENV_set_value_number("joypad_camera",		menu_data[8].Data, "Joypad");
		ENV_set_value_number("joypad_cam_left",		menu_data[9].Data, "Joypad");
		ENV_set_value_number("joypad_cam_right",	menu_data[10].Data , "Joypad");
		ENV_set_value_number("joypad_1stperson",	menu_data[11].Data, "Joypad");
#endif
		break;

	case FE_CONFIG_OPTIONS:
#ifdef TARGET_DC
		ENV_set_value_number("scanner_follows",		menu_data[0].Data&1, "Game");
		ENV_set_value_number("analogue_pad_mode",	menu_data[1].Data&1, "Game");
		ENV_set_value_number("vibration_mode",		menu_data[2].Data&1, "Game");
		ENV_set_value_number("vibration_engine",	menu_data[3].Data&1, "Game");
#else
		ENV_set_value_number("scanner_follows",		menu_data[1].Data&1, "Game");
#endif
		break;

	}
}

BOOL	FRONTEND_ValidMission(SWORD sel) {
	CBYTE *str=menu_buffer;
	UBYTE id=*str;

	while (sel) {
		sel--;
		str++;
		str+=strlen(str)+1;
		id=*str;
	}
	return (BOOL)(mission_hierarchy[id]&4);
}




UBYTE	FRONTEND_input() {
	UBYTE scan, any_button=0;
#ifndef TARGET_DC
	// DC input system does this for us.
	static SLONG last_input=0;
#endif
	static UBYTE last_button=0;
	static UBYTE first_pad=1;

	SLONG input=0;


#ifdef TARGET_DC
	if ( ( menu_state.mode == FE_SAVESCREEN ) || ( menu_state.mode == FE_LOADSCREEN ) || ( menu_state.mode == FE_VMU_SELECT ) )
	{
		// While these are shown, we much watch the VMUs and controllers.
		// If any change state, the correct screen needs refreshing.
		bool bSomethingChanged = RescanDevices();
		if ( bSomethingChanged )
		{
			// Delete any removed devices.
			DeleteInvalidDevice();

			fade_mode=1;
			// Re-enter this screen.
			FRONTEND_mode ( menu_state.mode, FALSE );
			// Pop the stack.
			menu_state.stackpos--;
#if 0
			switch ( menu_state.mode )
			{
			case FE_SAVESCREEN:
				menu_state.title=XLAT_str_ptr(X_SAVE_GAME);
				break;
			case FE_LOADSCREEN:
				menu_state.title=XLAT_str_ptr(X_LOAD_GAME);
				break;
			case FE_VMU_SELECT:
				menu_state.title=XLAT_str_ptr(X_VMU_SELECT);
				break;
			default:
				ASSERT ( FALSE );
				break;
			}
#endif
			FRONTEND_kibble_flurry();
			return ( 0 );
		}
	}
#endif

#ifdef TARGET_DC
extern DIDeviceInfo *primary_device;
extern BOOL AreAnyDevicesConnected ( void );

	if ( primary_device == NULL )
	{
		if ( AreAnyDevicesConnected() )
		{
			// No current controller, but there is one connected. Tell them to press start.
			FRONTEND_MakeQuickMessage ( XLAT_str ( X_CONTROLLER_REMOVED ), 100 );
		}
		else
		{
			// No controllers at all.
			// Display Sega's mad message.
			FRONTEND_MakeQuickMessage ( XLAT_str ( X_NO_CONTROLLER_CONNECTED ), 100, TRUE );
		}
	}
#endif


#ifdef TARGET_DC
	if ( m_bMovingPanel )
	{
		// While the action button is held down, the analog stick or D-pad
		// move the panel display around the screen.
		// Wait until a new button goes down (not the one that just selected this option).
		// Also need to remap so that we ignore the normal mappings, otherwise buttons that
		// don't have any current mapping don't trigger anything!
		DWORD dwInput = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_REMAP_START_BUTTON );

		if ( ( dwInput & INPUT_MASK_DOMENU ) == 0 )
		{
			// They stopped pressing the "do" button. Stop doing this.
			m_bMovingPanel = FALSE;
		}
		else
		{
			// OK, still moving the panel.
			// REMEMBER THAT THESE NUMBERS ARE DIVIDED BY 4!
			int iXPos = ENV_get_value_number ( "panel_x", 32 / 4, "" );
			int iYPos = ENV_get_value_number ( "panel_y", (480 - 32) / 4, "" );

			// Move the panel
#define PANEL_MOVE_SHIFT 3

			if ( dwInput & INPUT_MASK_LEFT )
			{
				int iHowMuch = 0x20 - ( ( dwInput >> 18 ) & 0x7f );
				iHowMuch >>= PANEL_MOVE_SHIFT;
				// Small possibility that it can be -ve (if digital stick is used, but analog centres slighty to right)
				if ( iHowMuch < 0 )
				{
					iHowMuch = 0;
				}
				// Always at least 1 (also allows the digital stick to do slides).
				iHowMuch++;
				iXPos -= iHowMuch;
				if ( iXPos < 0 )
				{
					iXPos = 0;
				}
			}

			if ( dwInput & INPUT_MASK_RIGHT )
			{
				int iHowMuch = ( ( dwInput >> 18 ) & 0x7f ) - 0x5f;
				iHowMuch >>= PANEL_MOVE_SHIFT;
				// Small possibility that it can be -ve (if digital stick is used, but analog centres slighty to right)
				if ( iHowMuch < 0 )
				{
					iHowMuch = 0;
				}
				// Always at least 1 (also allows the digital stick to do slides).
				iHowMuch++;
				iXPos += iHowMuch;
				if ( iXPos > ( ( 640 - 212 ) / 4 ) )
				{
					iXPos = ( 640 - 212 ) / 4;
				}
			}

			if ( dwInput & INPUT_MASK_FORWARDS )
			{
				int iHowMuch = 0x20 - ( ( dwInput >> 25 ) & 0x7f );
				iHowMuch >>= PANEL_MOVE_SHIFT;
				// Small possibility that it can be -ve
				if ( iHowMuch < 0 )
				{
					iHowMuch = 0;
				}
				// Always at least 1 (also allows the digital stick to do slides).
				iHowMuch++;
				iYPos -= iHowMuch;
				if ( iYPos < ( 165 / 4 ) )
				{
					iYPos = 165 / 4;
				}
			}

			if ( dwInput & INPUT_MASK_BACKWARDS )
			{
				int iHowMuch = ( ( dwInput >> 25 ) & 0x7f ) - 0x5f;
				iHowMuch >>= PANEL_MOVE_SHIFT;
				// Small possibility that it can be -ve
				if ( iHowMuch < 0 )
				{
					iHowMuch = 0;
				}
				// Always at least 1 (also allows the digital stick to do slides).
				iHowMuch++;
				iYPos += iHowMuch;
				if ( iYPos > ( 480 / 4 ) )
				{
					iYPos = 480 / 4;
				}
			}

			// Store the values back.
			ASSERT ( ( iXPos >= 0 ) && ( iXPos < 256 ) );
			ASSERT ( ( iYPos >= 0 ) && ( iYPos < 256 ) );
			ENV_set_value_number ( "panel_x", iXPos, "" );
			ENV_set_value_number ( "panel_y", iYPos, "" );
		}
	}
	else
#endif

#ifdef TARGET_DC
	if (grabbing_pad)
#else
	if (grabbing_pad&&!last_input)
#endif
	{
		UBYTE i,j;
		MenuData *item=menu_data+menu_state.selected;
#ifndef TARGET_DC
		ReadInputDevice();
#else
		// Wait until a new button goes down (not the one that just selected this option).
		// Also need to remap so that we ignore the normal mappings, otherwise buttons that
		// don't have any current mapping don't trigger anything!
		input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_GONEDOWN | INPUT_TYPE_REMAP_START_BUTTON );
		if ( input == 0 )
		{
			return 0;
		}
#endif
#ifndef TARGET_DC
		if (Keys[KB_ESC]||(input&INPUT_MASK_CANCEL))
#else
		// On DC, use "start" as the cancel button, since we're not allowed to map anything onto it anyway.
		if ( input & INPUT_MASK_START )
#endif
		{
			Keys[KB_ESC]=0;
#if 0
			// Should we not just leave this as it is, rather than setting it to "unused"?
			item->Data=31;
#endif
			grabbing_pad=0;
		}
		else
		{
			for (i=0;i<32;i++)
			{
				if (the_state.rgbButtons[i] & 0x80)
				{
					for (j=0;j<menu_state.items;j++)
					{
						if (menu_data[j].Data==i)
						{
							menu_data[j].Data=31;
						}
					}
					item->Data=i;
	#ifndef TARGET_DC
					last_button=1;
	#else
					last_button=0;
	#endif
					grabbing_pad=0;
					break;
				}
			}
		}
		return 0;
	} else {

#ifdef ALLOW_JOYPAD_IN_FRONTEND


// these are in interfac.cpp -- but the NOISE_TOLERANCE there, while appropriate for the game,
// is SHITTY for the menu. so here's a new one.

#define	AXIS_CENTRE			32768
#define	NOISE_TOLERANCE		4096
#define	AXIS_MIN			(AXIS_CENTRE-NOISE_TOLERANCE)
#define	AXIS_MAX			(AXIS_CENTRE+NOISE_TOLERANCE)

//extern ULONG	get_hardware_input(UWORD type);
#ifdef TARGET_DC
		input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_GONEDOWN | INPUT_TYPE_REMAP_START_BUTTON );
#else
		input = get_hardware_input(INPUT_TYPE_JOY);
#endif


#ifdef TARGET_DC
		// On the title screen, if there is a primary device,
		// then we want to get onto the next screen immediately.
		// So fake up a button A press. What a kludge!
		if ( menu_state.mode == FE_TITLESCREEN )
		{
			if ( primary_device != NULL )
			{
				// The is a primary, so you must have pressed "Start",
				// since the title screen resets the current device.
				input = INPUT_MASK_DOMENU;
			}
		}
#endif



// On the DC, the standard input is just fine.
#ifndef TARGET_DC

//		TRACE("raw input: %d -- ",input);
		input&=~(INPUT_MASK_LEFT|INPUT_MASK_RIGHT|INPUT_MASK_FORWARDS|INPUT_MASK_BACKWARDS);
		if(the_state.lX>AXIS_MAX)
		{
			input |= INPUT_MASK_RIGHT;
		}
		else if(the_state.lX<AXIS_MIN)
		{
			input |= INPUT_MASK_LEFT;
		}

		// let's not allow diagonals, they're silly.

		if(the_state.lY>AXIS_MAX)
		{
			input&=~(INPUT_MASK_LEFT|INPUT_MASK_RIGHT);
			input|=INPUT_MASK_BACKWARDS;
		}
		else if(the_state.lY<AXIS_MIN)
		{
			input&=~(INPUT_MASK_LEFT|INPUT_MASK_RIGHT);
			input|=INPUT_MASK_FORWARDS;
		}
#endif //#ifndef TARGET_DC

#ifndef TARGET_DC
		if (input==last_input) {
			input=0;
			any_button=0;
		}
		else
#endif
		{
#ifndef TARGET_DC
			last_input=input;
#endif
#ifndef TARGET_DC
			for (scan=0;scan<8;scan++)
				any_button |= the_state.rgbButtons[scan];
			// Supress very first movement - PC joysticks have a
			// strange habit of doing one movement on boot-up for some reason.
			if (first_pad)
			{
				if (any_button)
					first_pad=0;
				else
				if (input& (INPUT_MASK_LEFT|INPUT_MASK_RIGHT|INPUT_MASK_FORWARDS|INPUT_MASK_BACKWARDS)) {
					first_pad=0;
					input=0;
				}
			}
#else
			// On DC, this has already been done - appropriate buttons are mapped to punch.
			any_button = ( ( input & INPUT_MASK_DOMENU ) != 0 ) ? 1 : 0;
#endif
			if (last_button)
			{
				if (!any_button)
					last_button=0;
				else
					any_button=0;
			}
		}
//		TRACE("proc'd input: %d\n",input);

#endif

	}


#ifdef TARGET_DC
	// Sense ABXYStart
	if ( g_bDreamcastABXYStartComboPressed )
	{
		g_bDreamcastABXYStartComboPressed = FALSE;
#ifdef WANT_A_TITLE_SCREEN
		// Go to the title screen instead of the menu screen.
		if ( menu_state.mode != FE_TITLESCREEN )
		{
			// Go to the main menu.
			menu_mode_queued=FE_TITLESCREEN;
			fade_mode=2|4;
			//FRONTEND_kibble_flurry();
			return FE_TITLESCREEN;
		}
#else
		if ( menu_state.mode != FE_MAINMENU )
		{
			// Go to the main menu.
			menu_mode_queued=FE_MAINMENU;
			fade_mode=2|4;
			FRONTEND_kibble_flurry();
			return FE_MAINMENU;
		}
#endif
		else
		{
			// Already at main menu - bin everything and quit to BIOS boot screen.
			ASSERT ( FALSE );
			ResetToFirmware();
		}
	}
#endif

	if (grabbing_key&&LastKey) {
		MenuData *item=menu_data+menu_state.selected;
//		CBYTE key=(Keys[KB_LSHIFT]||Keys[KB_RSHIFT]) ?  InkeyToAsciiShift[LastKey] : InkeyToAscii[LastKey];
/*		switch(LastKey){
		case KB_LEFT: case KB_RIGHT: case KB_UP: case KB_DOWN: case KB_ENTER: case KB_SPACE:
		case KB_LSHIFT: case KB_RSHIFT: case KB_LALT: case KB_RALT: case KB_LCONTROL: case KB_RCONTROL:
		case KB_TAB: case KB_END: case KB_HOME: case KB_DEL: case KB_INS: case KB_PGUP: case KB_PGDN:
			item->Data=LastKey;
			Keys[LastKey]=0;
			break;
		case KB_ESC: break;
		default:
			item->Data=key;
		}*/
		if (LastKey!=KB_ESC) {
			UBYTE j;
			for (j=0;j<menu_state.items;j++)
				if (menu_data[j].Data==LastKey) menu_data[j].Data=0;
			item->Data=LastKey;

//			CBYTE moo[20];
//			GetKeyNameText(LastKey<<16,moo,20);

		}
		Keys[LastKey]=0;
		grabbing_key=0;
		//Keys[KB_ESC]=Keys[KB_UP]=Keys[KB_DOWN]=Keys[KB_LEFT]=Keys[KB_RIGHT]=Keys[KB_ENTER]=0;
		return 0;
	}
	if(allow_debug_keys)
	{
		if (Keys[KB_1]||Keys[KB_2]||Keys[KB_3]||Keys[KB_4])
		{
			if (Keys[KB_1]) { Keys[KB_1]=0; menu_theme=0; }
			if (Keys[KB_2]) { Keys[KB_2]=0; menu_theme=1; }
			if (Keys[KB_3]) { Keys[KB_3]=0; menu_theme=2; }
			if (Keys[KB_4]) { Keys[KB_4]=0; menu_theme=3; }
			//InitBackImage(menu_back_names[menu_theme]);
			UseBackSurface(screenfull_back);

			FRONTEND_kibble_init();
		}
	}

#ifndef TARGET_DC
	if (Keys[KB_END])
	{
		Keys[KB_END]=0;
		MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
		menu_state.selected=menu_state.items-1;
		if (menu_state.mode==FE_MAPSCREEN) mission_selected=mission_count-1;
		while (((menu_data+menu_state.selected)->Type==OT_LABEL)||(((menu_data+menu_state.selected)->Type==OT_BUTTON)&&((menu_data+menu_state.selected)->Choices==(CBYTE*)1)))
			menu_state.selected--;
	}
	if (Keys[KB_HOME])
	{
		Keys[KB_HOME]=0;
		MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
		menu_state.selected=0;
		if (menu_state.mode==FE_MAPSCREEN) mission_selected=0;
		while (((menu_data+menu_state.selected)->Type==OT_LABEL)||(((menu_data+menu_state.selected)->Type==OT_BUTTON)&&((menu_data+menu_state.selected)->Choices==(CBYTE*)1)))
			menu_state.selected++;
	}
#endif
	if (Keys[KB_UP]||(input&INPUT_MASK_FORWARDS)) {
		Keys[KB_UP]=0;
		MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
		if (menu_state.selected>0) menu_state.selected--;
			else menu_state.selected=menu_state.items-1;
		while (((menu_data+menu_state.selected)->Type==OT_LABEL)||(((menu_data+menu_state.selected)->Type==OT_BUTTON)&&((menu_data+menu_state.selected)->Choices==(CBYTE*)1))) {
			if (menu_state.selected>0) menu_state.selected--;
			else menu_state.selected=menu_state.items-1;
		}
		if ((menu_state.mode==FE_MAPSCREEN)&&mission_selected) mission_selected--;
	}
	if (Keys[KB_DOWN]||(input&INPUT_MASK_BACKWARDS)) {
		Keys[KB_DOWN]=0;
		MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
		if (menu_state.selected<menu_state.items-1) menu_state.selected++;
			else menu_state.selected=0;
		while (((menu_data+menu_state.selected)->Type==OT_LABEL)||(((menu_data+menu_state.selected)->Type==OT_BUTTON)&&((menu_data+menu_state.selected)->Choices==(CBYTE*)1))) {
			if (menu_state.selected<menu_state.items-1) menu_state.selected++;
			else menu_state.selected--;
		}
		if ((menu_state.mode==FE_MAPSCREEN)&&(mission_selected<mission_count-1)&&(FRONTEND_ValidMission(mission_selected+1))) mission_selected++;
	}

	if (Keys[KB_ENTER ] ||
		Keys[KB_SPACE ] ||
		Keys[KB_PENTER] ||
		any_button
		)
	{
		Keys[KB_ENTER ] = 0;
		Keys[KB_SPACE ] = 0;
		Keys[KB_PENTER] = 0;
		MenuData *item=menu_data+menu_state.selected;

		if (fade_mode!=2) MFX_play_stereo(0,S_MENU_CLICK_END,MFX_REPLACE);
		FRONTEND_stop_xition();
#ifdef TARGET_DC
		// Only go here if we are overwriting a file.
		if ((menu_state.mode==FE_SAVESCREEN)&&(item->Data==FE_SAVE_CONFIRM))
#else
		if ((menu_state.mode==FE_SAVESCREEN)&&(item->Data!=FE_BACK))
#endif
		{
			save_slot=menu_state.selected+1;
//			item->LabelID=X_SAVE_GAME;
/*			menu_mode_queued=FE_SAVE_CONFIRM;
			fade_mode=2;
			FRONTEND_kibble_flurry();
			return 0;*/
		}
#ifdef TARGET_DC
		if ((menu_state.mode==FE_VMU_SELECT)&&(item->Data==FE_MAINMENU))
		{
			// Set this VMU to be the current one.
			MapleVMU *pVMU = FindMemoryVMUAt ( m_ubVMUListCtrl[menu_state.selected], m_ubVMUListSlot[menu_state.selected] );
			SetCurrentStorageVMU ( pVMU );
			item->Data = FE_BACK;
		}

		if ((menu_state.mode==FE_CONFIG_OPTIONS)&&(item->LabelID==X_VIBRATION))
		{
			// Remember, this is the state it is currently, so we are turning it on.
			if ( ( item->Data & 0x1 ) == 0 )
			{
				// Set off a vibration.
				SetVibrationEnable ( TRUE );
				Vibrate ( 10.0f, 1.0f, 0.5f );
			}
			else
			{
				//Turn the Engine Vibration off as well.
				item[1].Data &= ~1;
			}
		}
		if ((menu_state.mode==FE_CONFIG_OPTIONS)&&(item->LabelID==X_VIBRATION_ENG))
		{
			// Remember, this is the state it is currently, so we are turning it on.
			if ( ( item->Data & 0x1 ) == 0 )
			{
				//Turn all vibrations on as well.
				item[-1].Data |= 1;
				// Set off a vibration.
				SetVibrationEnable ( TRUE );
				Vibrate ( 10.0f, 1.0f, 0.5f );
			}
		}

#endif
		if ((menu_state.mode==FE_SAVESCREEN)&&(item->Data==FE_MAPSCREEN))
		{
			menu_mode_queued=FE_MAPSCREEN;
			menu_state.stackpos=0;
			menu_thrash=FE_MAINMENU;
		}
#ifdef TARGET_DC
		// If not overwriting a file, we come straight here without a confirm.
		if ( ( (menu_state.mode==FE_SAVESCREEN)&&(item->Data==FE_MAPSCREEN) )
		   ||( (menu_state.mode==FE_SAVE_CONFIRM)&&(item->Data==FE_MAPSCREEN) ) )
#else
		if ((menu_state.mode==FE_SAVE_CONFIRM)&&(item->Data==FE_MAPSCREEN))
#endif
		{
			if ( item->LabelID == X_CANCEL )
			{
				// Cancel was pressed - just exit.
				menu_mode_queued=FE_MAPSCREEN;
				fade_mode=2;
				FRONTEND_kibble_flurry();
				menu_state.stackpos=0;
				menu_thrash=FE_MAINMENU;
				return 0;
			}

#ifdef TARGET_DC
			if ( menu_state.mode==FE_SAVESCREEN )
			{
				// Which slot was it?
				save_slot=menu_state.selected+1;
			}

			// Find the next "recommended" mission.
			FRONTEND_MissionHierarchy(MISSION_SCRIPT);
			bool bSuccess = FRONTEND_save_savegame(iNextSuggestedMission,save_slot);
#else
			CBYTE ttl[_MAX_PATH];
			FRONTEND_fetch_title_from_id(MISSION_SCRIPT,ttl,mission_launch);
			bool bSuccess = FRONTEND_save_savegame(ttl,save_slot);
#endif
			if ( bSuccess )
			{
				menu_mode_queued=FE_MAPSCREEN;
				fade_mode=2;
				FRONTEND_kibble_flurry();
				menu_state.stackpos=0;
				menu_thrash=FE_MAINMENU;
				return 0;
			}
			else
			{
				// Failed. Just buzz and return to the main menu.
				// I hope this "no" sound is a decent one - sound doesn't work yet!
				MFX_play_stereo(0,S_MENU_CLICK_END,MFX_REPLACE);
				menu_mode_queued=FE_MAPSCREEN;
				fade_mode=2;
				FRONTEND_kibble_flurry();
				menu_state.stackpos=0;
				menu_thrash=FE_MAINMENU;
				return 0;
			}
		}
		if (menu_state.mode==FE_MAPSCREEN) {
			if (mission_count>0) {
				menu_mode_queued=100+mission_selected;
				fade_mode=2;
				FRONTEND_kibble_flurry();
			}
			return 0;
		}

		if (menu_state.mode>=100)
		{
			// Starting a mission.
			// Stop any briefing.
			MFX_QUICK_stop(TRUE);
			return FE_START;
		}
		if ((menu_state.mode==FE_LOADSCREEN)&&(item->Data==FE_SAVE_CONFIRM))
		{
			bool bSuccess = FRONTEND_load_savegame(menu_state.selected+1);
			if ( bSuccess )
			{
				FRONTEND_MissionHierarchy(MISSION_SCRIPT);
				menu_mode_queued=FE_MAPSCREEN;
				fade_mode=2;
				FRONTEND_kibble_flurry();
				return 0;
			}
			else
			{
				// Failed the load. For now, do nothing.
				// For example, can be if they select an "(EMPTY)" slot.
				// I hope this "no" sound is a decent one - sound doesn't work yet!
				MFX_play_stereo(0,S_MENU_CLICK_END,MFX_REPLACE);
				// Quit back to the main menu.
				menu_mode_queued=FE_MAINMENU;
				fade_mode=2;
				FRONTEND_kibble_flurry();
				return 0;
			}
		}
		switch (item->Type) {
		case OT_MULTI:
			// Cycle through all options.
			item->Data = ( item->Data & ~0xff ) | ( ( item->Data & 0xff ) + 1 );
			if ( ( item->Data & 0xff ) >= ( item->Data >> 8 ) )
			{
				item->Data &= ~0xff;
			}
			break;
		case OT_KEYPRESS:
			grabbing_key=1;
			LastKey=0;
			break;
		case OT_PADPRESS:
			if ( bCanChangeJoypadButtons )
			{
				grabbing_pad=1;
				//LastKey=0;
#ifndef TARGET_DC
				last_input = 0;
#endif
			}
			else
			{
				// Can't change button - using a predefined setup.
				// I hope this "no" sound is a decent one - sound doesn't work yet!
				MFX_play_stereo(0,S_MENU_CLICK_END,MFX_REPLACE);
			}
			break;
		case OT_PADMOVE:
			// Enter pad-move mode.
			m_bMovingPanel = TRUE;
			break;
		case OT_BUTTON:
		case OT_BUTTON_L:
			if (menu_state.mode==FE_START) return FE_LOADSCREEN;
#ifdef WANT_A_TITLE_SCREEN

#if 0
			if (menu_state.mode==FE_TITLESCREEN)
			{
			}
#endif
			if (menu_state.mode==FE_LANGUAGESCREEN)
			{
				// Select the language based on button press, and go to the main menu.
				switch ( item->LabelID )
				{
				case X_ENGLISH:
					ENV_set_value_number ( "lang_num", 0, "" );
					break;
				case X_FRENCH:
					ENV_set_value_number ( "lang_num", 1, "" );
					break;
				default:
					break;
				}
				mission_launch = 0;
				return FE_CHANGE_LANGUAGE;
			}
#endif
#ifdef WANT_AN_EXIT_MENU_ITEM
			//if (item->Data==-1) return -1;
			if (item->Data==FE_NO_REALLY_QUIT) return -1;
#endif
			//if (item->Data==-2) FRONTEND_storedata();
			if (item->Data==FE_BACK) FRONTEND_storedata();
			if (item->Data==FE_START) return FE_LOADSCREEN;
			// temp thingy:
			//if (item->Data==FE_MAPSCREEN) return FE_START;
			if (item->Data==FE_EDITOR) return FE_EDITOR;
			if (item->Data==FE_CREDITS) return FE_CREDITS;
#ifdef TARGET_DC
			if (item->Data==FE_CHANGE_JOYPAD)
			{
				// Wait until the button goes back up, or this joypad
				// will immediately be selected again!
				while ( 0 != ( INPUT_MASK_ALL_BUTTONS & get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_REMAP_START_BUTTON ) ) )
				{
				}

				// Change the active joypad.
				ClearPrimaryDevice();
				// And pretend it did an "FE_BACK".
				menu_mode_queued=FE_BACK;
				FRONTEND_storedata();
				fade_mode=2|4;
				FRONTEND_kibble_flurry();
				return 0;
			}
#endif

			FRONTEND_kibble_flurry();

			//FRONTEND_mode(item->Data);
			menu_mode_queued=item->Data;
			fade_mode=2|((item->Data==FE_BACK)?4:0);
			break;
		case OT_RESET:
			switch(menu_state.mode){
#ifdef WANT_A_KEYBOARD_ITEM
			case FE_CONFIG_INPUT_KB:
				menu_data[0].Data = 203;
				menu_data[1].Data = 205;
				menu_data[2].Data = 200;
				menu_data[3].Data = 208;
				menu_data[4].Data = 44;
				menu_data[5].Data = 45;
				menu_data[6].Data = 46;
				//menu_data[7].Data = 47;
				menu_data[7].Data = 57;
				menu_data[8].Data = 15;
				menu_data[9].Data = 28;
				// gap for label
				menu_data[11].Data = 207;
				menu_data[12].Data = 211;
				menu_data[13].Data= 209;
				menu_data[14].Data= 30;
				break;
#endif
			case FE_CONFIG_INPUT_JP:
#ifdef TARGET_DC
				menu_data[0].Data = 1 | (5<<8);
				// gap for label
				menu_data[2].Data = 9;
				menu_data[3].Data = 10;
				menu_data[4].Data = 7;
				menu_data[5].Data = 1;
				menu_data[6].Data = 3;
				menu_data[7].Data = 0;
				// gap for label
				menu_data[9].Data = 4;
				menu_data[10].Data = 5;
				menu_data[11].Data = 6;
				menu_data[12].Data = 8;
#else
				menu_data[0].Data = 4;
				menu_data[1].Data = 3;
				menu_data[2].Data = 0;
				menu_data[3].Data = 1;
				menu_data[4].Data = 7;
				menu_data[5].Data = 8;
				menu_data[6].Data = 2;
				// gap for label
//				menu_data[7].Data = 99;
				menu_data[8].Data = 6;
				menu_data[9].Data = 9;
				menu_data[10].Data = 10;
				menu_data[11].Data = 5;
#endif
				break;
			}
			break;
		}
	}
	if (Keys[KB_LEFT]||(input&INPUT_MASK_LEFT)) {
		Keys[KB_LEFT]=0;
		MenuData *item=menu_data+menu_state.selected;
		if ((item->Type==OT_SLIDER)&&(item->Data>0))
		{
#ifdef TARGET_DC
			// Analogue slidyness!
			// The analog stuff has been masked out for GONEDOWN, so get it again.
			DWORD dwInput = get_last_input ( 0 );
			int iHowMuch = 0x20 - ( ( dwInput >> 18 ) & 0x7f );
			iHowMuch >>= 3;
			// Small possibility that it can be -ve (if digital stick is used, but analog centres slighty to right)
			if ( iHowMuch < 0 )
			{
				iHowMuch = 0;
			}
			// Always at least 1 (also allows the digital stick to do slides).
			iHowMuch++;
			if ( item->Data > iHowMuch )
			{
				item->Data -= iHowMuch;
			}
			else
			{
				item->Data = 0;
			}
			// And allow this to autorepeat.
			allow_input_autorepeat();
#else
			item->Data--;
			if (item->Data>0)
			{
				item->Data--;
			}
#endif
		}



		if ((menu_state.mode==FE_MAPSCREEN)&&district_selected) {
			scan=district_selected-1;
			while ((scan>0)&&!district_valid[scan]) scan--;
			if (district_valid[scan]) {
				district_selected=scan;
				FRONTEND_MissionList(MISSION_SCRIPT,districts[district_selected][2]);
				MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
			}
		}


// No normal options can be changed by left/right - too easy to do on DC joypads.
#ifndef TARGET_DC

#ifdef TARGET_DC
		if ((menu_state.mode==FE_CONFIG_OPTIONS)&&(item->LabelID==X_VIBRATION))
		{
			// Remember, this is the state it is currently, so we are turning it off.
			if ( ( item->Data & 0x1 ) == 1 )
			{
				// Turn the Engine Vibration off as well.
				item[1].Data &= ~1;
			}
		}
#endif
		if ((item->Type==OT_MULTI)&&((item->Data&0xff)>0)) {
			item->Data--;
			MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
		}
#endif

		if (menu_state.mode==FE_CONFIG_VIDEO) FRONTEND_gamma_update();
		if ((menu_state.mode==FE_CONFIG_AUDIO)&&!menu_state.selected) {
			MFX_play_stereo(1,S_TRAFFIC_CONE,0);
			//MFX_set_gain(1,S_TRAFFIC_CONE,255);
		}

	}
	if (Keys[KB_RIGHT]||(input&INPUT_MASK_RIGHT)) {
		Keys[KB_RIGHT]=0;
		MenuData *item=menu_data+menu_state.selected;
		if ((item->Type==OT_SLIDER)&&(item->Data<255))
		{
#ifdef TARGET_DC
			// Analogue slidyness!
			// The analog stuff has been masked out for GONEDOWN, so get it again.
			DWORD dwInput = get_last_input ( 0 );
			int iHowMuch = ( ( dwInput >> 18 ) & 0x7f ) - 0x5f;
			iHowMuch >>= 3;
			// Small possibility that it can be -ve (if digital stick is used, but analog centres slighty to right)
			if ( iHowMuch < 0 )
			{
				iHowMuch = 0;
			}
			// Always at least 1 (also allows the digital stick to do slides).
			iHowMuch++;
			if ( item->Data < ( 256 - iHowMuch ) )
			{
				item->Data += iHowMuch;
			}
			else
			{
				item->Data = 255;
			}
			// And allow this to autorepeat.
			allow_input_autorepeat();
#else
			item->Data++;
			if (item->Data<255)
			{
				item->Data++;
			}
#endif
		}



		if ((menu_state.mode==FE_MAPSCREEN)&&(district_selected<district_count-1)) {
			scan=district_selected+1;
			while ((scan<district_count-1)&&!district_valid[scan]) scan++;
			if (district_valid[scan]) {
				district_selected=scan;
				FRONTEND_MissionList(MISSION_SCRIPT,districts[district_selected][2]);
				MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
			}
		}


// No normal options can be changed by left/right - too easy to do on DC joypads.
#ifndef TARGET_DC


#ifdef TARGET_DC
		if ((menu_state.mode==FE_CONFIG_OPTIONS)&&(item->LabelID==X_VIBRATION))
		{
			// Remember, this is the state it is currently, so we are turning it on.
			if ( ( item->Data & 0x1 ) == 0 )
			{
				// Set off a vibration.
				SetVibrationEnable ( TRUE );
				Vibrate ( 10.0f, 1.0f, 0.5f );
			}
		}
		if ((menu_state.mode==FE_CONFIG_OPTIONS)&&(item->LabelID==X_VIBRATION_ENG))
		{
			// Remember, this is the state it is currently, so we are turning it on.
			if ( ( item->Data & 0x1 ) == 0 )
			{
				// Turn on general vibrations as well.
				item[-1].Data |= 0x1;
			}
		}
#endif
		if ((item->Type==OT_MULTI)&&((item->Data&0xff)<(item->Data>>8)-1)) {
			item->Data++;
			MFX_play_stereo(1,S_MENU_CLICK_START,MFX_REPLACE);
		}
#endif

		if (menu_state.mode==FE_CONFIG_VIDEO) FRONTEND_gamma_update();
		if ((menu_state.mode==FE_CONFIG_AUDIO)&&!menu_state.selected) {
			MFX_play_stereo(1,S_TRAFFIC_CONE,0);
			//MFX_set_gain(1,S_TRAFFIC_CONE,255);
		}

	}
	if (Keys[KB_ESC]||(input&INPUT_MASK_CANCEL)) {
		Keys[KB_ESC]=0;
//		if (menu_state.mode>=100)
//			MFX_QUICK_stop();
		if (fade_mode!=6) MFX_play_stereo(1,S_MENU_CLICK_END,MFX_REPLACE);
		if (fade_mode==2) // cancel a transition
		{
			fade_mode=1;
			menu_mode_queued=menu_state.mode;
			return 0;
		}
		//FRONTEND_scr_del();
		if (menu_state.stackpos)
		{
			switch(menu_state.mode)
			{
			case FE_CONFIG_VIDEO: // eidos want ESC to store opts
				FRONTEND_store_video_data();
				break;
			case FE_CONFIG_AUDIO:
				MFX_stop(WEATHER_REF,MFX_WAVE_ALL);
				//MFX_stop(SIREN_REF,MFX_WAVE_ALL);
				MFX_set_volumes(menu_data[0].Data>>1,menu_data[1].Data>>1,menu_data[2].Data>>1);
				break;
			}

			// Store any options settings.
			FRONTEND_storedata();

			menu_mode_queued=FE_BACK;
		}
		else
		{
#ifdef WANT_AN_EXIT_MENU_ITEM
			menu_mode_queued=FE_QUIT;
#else
			menu_mode_queued=FE_MAINMENU;
#endif
		}

		if ((menu_state.mode==FE_SAVESCREEN)&&!menu_state.stackpos) {
			menu_thrash=FE_MAINMENU;
			menu_mode_queued=FE_MAPSCREEN;
		}
		fade_mode=6;

		FRONTEND_kibble_flurry();
	}

#ifdef TARGET_DC
	// Fudge for the joypad stuff.
	if ( menu_state.mode == FE_CONFIG_INPUT_JP )
	{
		MenuData *item=menu_data+menu_state.selected;
		if ( item->Type == OT_MULTI )
		{
			// There is only one MULTI on this screen - the "Mode" one.

			if ( ( item->Data & 0xff ) == 0 )
			{
				// The custom mode - they can edit joypad settings.
				bCanChangeJoypadButtons = TRUE;
			}
			else
			{
				// A predefined mode - they can't edit joypad settings.
				bCanChangeJoypadButtons = FALSE;
			}


			// See if the mode has changed.
			static int iLastMode = -1;

			if ( iLastMode != ( item->Data & 0xff ) )
			{
				iLastMode = ( item->Data & 0xff );

				// Update the shown joypad settings with the selected mode.
				INTERFAC_SetUpJoyPadButtons ( iLastMode );
				// Gap for label.
				menu_data[2].Data  = joypad_button_use[JOYPAD_BUTTON_PUNCH];
				menu_data[3].Data  = joypad_button_use[JOYPAD_BUTTON_KICK];
				menu_data[4].Data  = joypad_button_use[JOYPAD_BUTTON_JUMP];
				menu_data[5].Data  = joypad_button_use[JOYPAD_BUTTON_ACTION];
				menu_data[6].Data  = joypad_button_use[JOYPAD_BUTTON_MOVE];
				menu_data[7].Data  = joypad_button_use[JOYPAD_BUTTON_SELECT];
				// Gap for label.
				menu_data[9].Data = joypad_button_use[JOYPAD_BUTTON_CAMERA];
				menu_data[10].Data = joypad_button_use[JOYPAD_BUTTON_CAM_LEFT];
				menu_data[11].Data = joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT];
				menu_data[12].Data = joypad_button_use[JOYPAD_BUTTON_1STPERSON];
			}
		}
	}

#endif


	return 0;
}



//--- externally accessible ---


void	FRONTEND_init ( bool bGoToTitleScreen )
{

	static bool bFirstTime = TRUE;


	dwAutoPlayFMVTimeout = timeGetTime() + AUTOPLAY_FMV_DELAY;

	// Stop the music while loading stuff.
	stop_all_fx_and_music(TRUE);
	//MFX_QUICK_stop();

	// These two are so that when a mission ends with a camera still active, the music
	// stuff actually works properly. Obscure or what?
extern UBYTE EWAY_conv_active;
extern SLONG EWAY_cam_active;
	EWAY_conv_active = FALSE;
	EWAY_cam_active = FALSE;

	TICK_RATIO = (1 << TICK_SHIFT);

	// Set up the current language
	switch ( ENV_get_value_number ( "lang_num", 0, "" ) )
	{
	case 0:
		pcSpeechLanguageDir = "talk2\\";
		break;
	case 1:
		pcSpeechLanguageDir = "talk2_french\\";
		break;
	default:
		ASSERT ( FALSE );
		break;
	}


	// Reset the transition buffer's contents.
	lpFRONTEND_show_xition_LastBlit = NULL;


	#ifdef TARGET_DC


	// Load the icon pic data if it hasn't already been done.
	// Yes, this isn't the _best_ place to put it... :-)
	if ( !m_bLoadedIconSavePic )
	{
		TGA_Pixel pPixelData[32*32];

		// Load the savegame icon from disk.
extern TGA_Info TGA_load_from_file(const CBYTE *file, SLONG max_width, SLONG max_height, TGA_Pixel* data, BOOL bCanShrink);
		TGA_Info tga_info = TGA_load_from_file ( "server\\textures\\extras\\dc\\saved_data_icon2.tga", 32, 32, pPixelData, FALSE );
		if ( tga_info.valid )
		{
			// Convert to 16 colours. There MUST only be 16 colours!
			int iNumColours = 0;

			TGA_Pixel *pcSrc = pPixelData;
			BYTE *pcDest = m_pcIconSavePicData;
			for ( int y = 0; y < 32; y++ )
			{
				// Convert a line into bytes first.
				BYTE pcLine[32];
				BYTE *pcDest1 = pcLine;
				for ( int x = 0; x < 32; x++ )
				{
					// Find the 4444 colour (set alpha to full).
					WORD wColour = 0xf000;
					// Read the colours.
					wColour |= ( ( pcSrc->red   ) & 0xf0 ) << 4;
					wColour |= ( ( pcSrc->green ) & 0xf0 );
					wColour |= ( ( pcSrc->blue  ) & 0xf0 ) >> 4;
					pcSrc++;

					// Search for it.
					for ( int i = 0; i < iNumColours; i++ )
					{
						if ( wColour == m_pcIconSavePicPalette[i] )
						{
							break;
						}
					}
					if ( i == iNumColours )
					{
						// Didn't find it - add it.
						ASSERT ( iNumColours < 16 );
						m_pcIconSavePicPalette[iNumColours] = wColour;
						iNumColours++;
					}

					*pcDest1++ = i;
				}

				// Now pack into 4bpp.
				pcDest1 = pcLine;
				for ( x = 0; x < 16; x++ )
				{
					*pcDest = (*pcDest1++) << 4;
					*pcDest++ |= (*pcDest1++);
				}
			}

			m_bLoadedIconSavePic = TRUE;
		}
	}

	#endif


#ifdef TARGET_DC
	// Need to reinit the frontend stuff.
	FRONTEND_scr_new_theme(
		menu_back_names  [menu_theme],
		menu_map_names   [menu_theme],
		menu_brief_names [menu_theme],
		menu_config_names[menu_theme]);

#ifdef ONLY_USE_THREE_BACKGROUNDS_PLEASE_BOB
	UseBackSurface(screenfull_back);
	screenfull = screenfull_back;
#else
	UseBackSurface(screenfull_config);
	screenfull = screenfull_config;
#endif
#endif






#ifdef SAVE_MY_BACKGROUNDS_PLEASE_BOB
	// Convert _all_ the screens used, to make sure we don't miss any.
	// The act of loading them also converts them.
	InitBackImage("e3load.tga");
	InitBackImage("deadcivs.tga");
	InitBackImage("DCtitlepage_uk.tga");
	InitBackImage("DCtitlepage_us.tga");

	InitBackImage("briefing blood darci.tga");
	InitBackImage("map blood darci.tga");
	InitBackImage("config blood.tga");
	InitBackImage("title blood1.tga");

	InitBackImage("briefing leaves darci.tga");
	InitBackImage("map leaves darci.tga");
	InitBackImage("config leaves.tga");
	InitBackImage("title leaves1.tga");

	InitBackImage("briefing rain darci.tga");
	InitBackImage("map rain darci.tga");
	InitBackImage("config rain.tga");
	InitBackImage("title rain1.tga");

	InitBackImage("briefing snow darci.tga");
	InitBackImage("map snow darci.tga");
	InitBackImage("config snow.tga");
	InitBackImage("title snow1.tga");
#endif



#if 0
#ifndef FORCE_STUFF_PLEASE_BOB
	//
	// Turn off every even kibble.
	//

	for (SLONG kib = 0; kib < 512; kib += 2)
	{
		kibble_off[kib] = TRUE;
	}
#endif
#endif


	CBYTE *str, *lang=ENV_get_value_string("language");

#ifdef VERSION_FRENCH
	// Kludge for the DC converter
	if (!lang) lang="text\\lang_french.txt";
#else
	if (!lang) lang="text\\lang_english.txt";
#endif
	XLAT_load(lang);
	XLAT_init();

	IsEnglish=!stricmp(XLAT_str(X_THIS_LANGUAGE_IS),"English");

	str=menu_choice_yesno;
	strcpy(str,XLAT_str(X_NO));
	str+=strlen(str)+1;
	strcpy(str,XLAT_str(X_YES));

	strcpy(MISSION_SCRIPT,"data\\");
	lang=XLAT_str(X_THIS_LANGUAGE_IS);
	if (strcmp(lang,"English")==0)
		strcat(MISSION_SCRIPT,"urban");
	else
		strcat(MISSION_SCRIPT,lang);
	strcat(MISSION_SCRIPT,".sty");

	//
	// Load the mission script into memory so we don't have
	// to scan a file all the time!
	//

	CacheScriptInMemory(MISSION_SCRIPT);

#ifdef TARGET_DC
	MENUFONT_Load("olyfont2dc.tga",POLY_PAGE_NEWFONT_INVERSE,frontend_fonttable);
#else
	MENUFONT_Load("olyfont2.tga",POLY_PAGE_NEWFONT_INVERSE,frontend_fonttable);
#endif

void MENUFONT_MergeLower(void);
	MENUFONT_MergeLower();

	menu_theme = 0;

	//InitBackImage(menu_back_names[menu_theme]);
	UseBackSurface(screenfull_back);

#ifndef TARGET_DC
	// Er... we call FRONTEND_kibble_init just below. Whatever....
	ZeroMemory(kibble,sizeof(kibble));
#endif
	ZeroMemory(&menu_state,sizeof(menu_state));
	if (!complete_point) ZeroMemory(mission_hierarchy,60);
	menu_state.mode=-1;

	FRONTEND_CacheMissionList(MISSION_SCRIPT);

	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);

	ragepro_sucks=!the_display.GetDeviceInfo()->ModulateAlphaSupported();

	ZeroMemory(menu_choice_scanner,255);
	XLAT_str(X_CAMERA,menu_choice_scanner);
	lang=menu_choice_scanner+strlen(menu_choice_scanner)+1;
	XLAT_str(X_CHARACTER,lang);

#ifdef TARGET_DC
	// Joypad mode looks like "Custom A B C D" etc.
	ZeroMemory(menu_choice_joypad_mode,50);
	XLAT_str(X_PAD_CUSTOM,menu_choice_joypad_mode);
	char *pchar = menu_choice_joypad_mode + strlen ( menu_choice_joypad_mode ) + 1;
	for ( int iCount = 0; iCount < NUM_OF_JOYPAD_MODES; iCount++ )
	{
		*pchar++ = 'A' + iCount;
		*pchar++ = '\0';
	}

	ZeroMemory(menu_choice_analogue_mode,50);
	XLAT_str(X_TURN,menu_choice_analogue_mode);
	lang=menu_choice_analogue_mode+strlen(menu_choice_analogue_mode)+1;
	XLAT_str(X_MOVE,lang);

#ifdef WANT_A_TITLE_SCREEN
	ZeroMemory(menu_choice_language,50);
	XLAT_str(X_ENGLISH,menu_choice_language);
	lang=menu_choice_language+strlen(menu_choice_language)+1;
	XLAT_str(X_FRENCH,lang);
#endif
#endif

	MUSIC_mode(MUSIC_MODE_FRONTEND);

	FRONTEND_scr_new_theme(
		menu_back_names  [menu_theme],
		menu_map_names   [menu_theme],
		menu_brief_names [menu_theme],
		menu_config_names[menu_theme]);

	if ( !bFirstTime )
	{
		UseBackSurface(screenfull_back);
	}

//#if defined(M_SOUND) || defined(DC_SOUND)
	SLONG fx,amb,mus;
	MFX_get_volumes(&fx,&amb,&mus);
	MUSIC_gain(mus);
//#endif

	FRONTEND_districts(MISSION_SCRIPT);
	FRONTEND_MissionHierarchy(MISSION_SCRIPT);
	FRONTEND_kibble_init();


//extern bool g_bGoToCreditsPleaseGameHasFinished;


#ifdef WANT_A_TITLE_SCREEN
	if ( bGoToTitleScreen )
	{
		// Start off in the title screen instead.
		FRONTEND_mode(FE_TITLESCREEN);
	}
	else if ( bFirstTime )
	{
		// Start off in the language screen instead.
		FRONTEND_mode(FE_LANGUAGESCREEN);
	}
	else
#endif
	if ( m_bGoIntoSaveScreen )
	{
		// Just won a mission - going into save game.
		FRONTEND_mode(FE_SAVESCREEN);
		m_bGoIntoSaveScreen = FALSE;
	}
	else
	{
		// Frontend menu.
		FRONTEND_mode(FE_MAINMENU);
	}



	// Stop all the music - about to start it again, properly.
	stop_all_fx_and_music();
	MUSIC_reset();
	MUSIC_mode(0);
	MUSIC_mode_process();
	MUSIC_mode(MUSIC_MODE_FRONTEND);


	if (mission_launch)
	{
		// Just won a mission - going into save game.
		return;
	}

	{
		CBYTE fname[256];

		//
		// Load in the front-end specific sound
		//

		#ifdef TARGET_DC
		sprintf(fname, "Data\\Sfx\\1622DC\\%s", sound_list[S_FRONT_END_LOOP_EDIT]);
		#else
		sprintf(fname, "Data\\Sfx\\1622\\%s", sound_list[S_FRONT_END_LOOP_EDIT]);
		#endif

		DCLL_memstream_load(fname);

		//
		// Start playing the music!
		//

		#ifdef TARGET_DC
		MFX_QUICK_play("data\\sfx\\1622DC\\GeneralMusic\\FrontLoopMONO.wav",TRUE,0,0,0);
		#else
		MFX_QUICK_play("data\\sfx\\1622\\GeneralMusic\\FrontLoopMONO.wav",TRUE,0,0,0);
		#endif
	}

	bFirstTime = FALSE;
}

void	FRONTEND_level_lost()
{
	mission_launch=previous_mission_launch;
	// Start up the kibble again.
	FRONTEND_kibble_init();
	ZeroMemory(&menu_state,sizeof(menu_state));
	menu_state.mode=-1;
//	FRONTEND_MissionHierarchy(MISSION_SCRIPT);
	FRONTEND_mode(FE_MAINMENU);
}

void	FRONTEND_level_won()
{

	ASSERT(mission_launch<50);


	// update hierarchy data.
	mission_hierarchy[mission_launch]|=2; // complete



extern bool g_bPunishMePleaseICheatedOnThisLevel;
	if ( !g_bPunishMePleaseICheatedOnThisLevel )
	{
		// time to update roper/darci doohickeys
		if (1)//NET_PERSON(0)->Genus.Person->PersonType==PERSON_DARCI)
		{
			SLONG	found;
			found=NET_PLAYER(0)->Genus.Player->Constitution - the_game.DarciConstitution;
			ASSERT(found>=0);

			if(found>best_found[mission_launch][0])
			{
				the_game.DarciConstitution+=found-best_found[mission_launch][0];
				best_found[mission_launch][0]=found;
			}

			found=NET_PLAYER(0)->Genus.Player->Strength - the_game.DarciStrength;
			ASSERT(found>=0);

			if(found>best_found[mission_launch][1])
			{
				the_game.DarciStrength+=found-best_found[mission_launch][1];
				best_found[mission_launch][1]=found;
			}

			found=NET_PLAYER(0)->Genus.Player->Stamina - the_game.DarciStamina;
			ASSERT(found>=0);

			if(found>best_found[mission_launch][2])
			{
				the_game.DarciStamina+=found-best_found[mission_launch][2];
				best_found[mission_launch][2]=found;
			}

			found=NET_PLAYER(0)->Genus.Player->Skill - the_game.DarciSkill;
			ASSERT(found>=0);

			if(found>best_found[mission_launch][3])
			{
				the_game.DarciSkill+=found-best_found[mission_launch][3];
				best_found[mission_launch][3]=found;
			}
	/*
			the_game.DarciConstitution=NET_PLAYER(0)->Genus.Player->Constitution;
			the_game.DarciStrength=NET_PLAYER(0)->Genus.Player->Strength;
			the_game.DarciStamina=NET_PLAYER(0)->Genus.Player->Stamina;
			the_game.DarciSkill=NET_PLAYER(0)->Genus.Player->Skill;
	*/
		}
		else
		{
	#ifdef TARGET_DC
			// Not used.
			ASSERT ( FALSE );
	#else
			the_game.RoperConstitution=NET_PLAYER(0)->Genus.Player->Constitution;
			the_game.RoperStrength=NET_PLAYER(0)->Genus.Player->Strength;
			the_game.RoperStamina=NET_PLAYER(0)->Genus.Player->Stamina;
			the_game.RoperSkill=NET_PLAYER(0)->Genus.Player->Skill;
	#endif
		}
	}


	// Start up the kibble again.
	FRONTEND_kibble_init();
	ZeroMemory(&menu_state,sizeof(menu_state));
	menu_state.mode=-1;
	if (complete_point<mission_launch) complete_point=mission_launch;
	//mission_launch++;
	FRONTEND_MissionHierarchy(MISSION_SCRIPT);
/*  hierarchy does both of these now
	InitBackImage(menu_back_names[menu_theme]);
	FRONTEND_kibble_init();*/
	FRONTEND_mode(FE_SAVESCREEN);
	m_bGoIntoSaveScreen = TRUE;
}

void FRONTEND_playambient3d(SLONG channel, SLONG wave_id, SLONG flags, UBYTE height = 0)
{
	SLONG	angle = Random() & 2047;

	SLONG	x = (COS(angle) << 4);
	SLONG	y = 0;
	SLONG	z = (SIN(angle) << 4);

	if (height == 1)	y += (512 + (Random() & 1023)) << 8;

	MFX_play_xyz(channel, wave_id, 0, x, y, z);
//	MFX_set_gain(channel, wave_id,255);
}


void	FRONTEND_sound() {
	static SLONG siren_time=100;
	SLONG wave_id;

	MFX_play_ambient(WEATHER_REF,S_WIND_START,MFX_LOOPED|MFX_QUEUED);
//	MFX_play_ambient(MUSIC_REF,S_TUNE_DRIVING,MFX_LOOPED);
	MFX_set_gain(WEATHER_REF,S_AMBIENCE_END,255);
//	MFX_set_gain(MUSIC_REF,S_TUNE_FRONTEND,menu_data[2].Data>>1);
	MUSIC_gain(menu_data[2].Data>>1);
//#if defined(M_SOUND) || defined(DC_SOUND)
	MFX_set_volumes(menu_data[0].Data>>1,menu_data[1].Data>>1,menu_data[2].Data>>1);
//#endif

	// Update the music volume.
//extern void DCLL_stream_volume(float volume);
//	DCLL_stream_volume ( 1.0f );


	siren_time--;
	if(siren_time<0)
	{
		wave_id = S_SIREN_START + (Random() % 4);
		siren_time = 300 + ((Random() & 0xFFFF) >> 5);
		FRONTEND_playambient3d(SIREN_REF, wave_id, 0, 1);
	}
	MFX_set_listener(0,0,0,0,0,0);
	MFX_render();
}


void	FRONTEND_diddle_stats()
{
#ifndef	FINAL
#ifndef TARGET_DC
	SWORD stat_up = ENV_get_value_number("stat_up",		0, "Secret");
	stat_up*=(mission_launch-1);

	the_game.DarciConstitution=stat_up;
	the_game.DarciStrength=stat_up;
	the_game.DarciStamina=stat_up;
	the_game.DarciSkill=stat_up;
	the_game.RoperConstitution=stat_up;
	the_game.RoperStrength=stat_up;
	the_game.RoperStamina=stat_up;
	the_game.RoperSkill=stat_up;
#endif
#endif

}

void	FRONTEND_diddle_music()
{
	TRACE("STARTSCR_mission: %s\n",STARTSCR_mission);
	MUSIC_bodge_code=0;
	if (strstr(STARTSCR_mission,"levels\\fight")||strstr(STARTSCR_mission,"levels\\FTutor"))
		MUSIC_bodge_code=1;
	else
		if (strstr(STARTSCR_mission,"levels\\Assault"))
			MUSIC_bodge_code=2;
		else
			if (strstr(STARTSCR_mission,"levels\\testdrive"))
				MUSIC_bodge_code=3;
			else
				if (strstr(STARTSCR_mission,"levels\\Finale1"))
					MUSIC_bodge_code=4;


}

UBYTE this_level_has_the_balrog;
UBYTE this_level_has_bane;
UBYTE	is_semtex=0;

SBYTE	FRONTEND_loop() {
	SBYTE res;

	static SLONG last = 0;
	static SLONG now = 0;

	SLONG millisecs;

	now       = GetTickCount();

	if (last < now - 250)
	{
		last = now - 250;
	}

	millisecs = now - last;
	last      = now;


#ifdef TARGET_DC
	// Display the default VMU screen.
	FRONTEND_show_VMU_screen ( NULL );
#endif

	//
	// How fast should the fade state fade?
	//

	SLONG fade_speed = (millisecs >> 3);

	if (fade_speed < 1)
	{
		fade_speed = 1;
	}

	switch(fade_mode&3)
	{
		case 1:
			if (fade_state<63)
			{
				fade_state += fade_speed;

				if (fade_state > 63)
				{
					fade_state = 63;
				}
			}
			else
			{
				FRONTEND_stop_xition();

				fade_mode = 0;
			}
			break;

		case 2:
			if (fade_state>0)
			{
				fade_state -= fade_speed;

				if (fade_state < 0)
				{
					fade_state = 0;
				}
			}
			else
			{
				FRONTEND_mode(menu_mode_queued);
			}
			break;
	}
#ifdef TARGET_DC
	// We don't fade, we change size.
	fade_rgb=0xFFFFFFFF;
#else
	fade_rgb=(((SLONG)fade_state*2)<<24)|0xFFFFFF;
#endif





#ifdef TARGET_DC
	if ( menu_state.mode == FE_TITLESCREEN )
	{
		// Handle the auto-playing of the FMV.
		// Also disable the screensaver by prodding it
		// every time.

		if ( ( ( dwAutoPlayFMVTimeout - timeGetTime() ) & 0x80000000 ) != 0 )
		{
			// Timed out.
			stop_all_fx_and_music();
			the_display.RunCutscene( 0, ENV_get_value_number("lang_num", 0, "" ) );

			// Clear the controler again (in case they pressed a button).


			// Start the music again.
			MFX_QUICK_play("data\\sfx\\1622DC\\GeneralMusic\\FrontLoopMONO.wav",TRUE,0,0,0);

			// Start the music again.
			// Doesn't seem to work.
			//MUSIC_mode(MUSIC_MODE_FRONTEND);

			// "Reset" the controller so that "Press Start Button" is displayed.
			ClearPrimaryDevice();

			// Two minutes wait on the title screen.
			dwAutoPlayFMVTimeout = timeGetTime() + AUTOPLAY_FMV_DELAY;
		}

		// Pretend the screensaver got an input to disable it.
extern DWORD g_dwLastInputChangeTime;
		g_dwLastInputChangeTime = timeGetTime();
	}
#endif





#ifdef WANT_A_TITLE_SCREEN
	// No kibble on the title screen.
	if ( ( menu_state.mode == FE_TITLESCREEN ) || ( menu_state.mode == FE_LANGUAGESCREEN ) )
	{
		// Remove any existing kibble.
		FRONTEND_kibble_init();
	}
	else
#endif
	{
		FRONTEND_kibble_process();
	}

#ifndef TARGET_DC
	PolyPage::DisableAlphaSort();
#endif
	FRONTEND_display();
	if ((menu_state.mode==FE_CONFIG_AUDIO)&&(fade_mode==0))
	{
		FRONTEND_sound();
	}
	else
	{
		MFX_set_listener(0,0,0,0,0,0);
		MFX_render();
	}
#ifndef TARGET_DC
	PolyPage::EnableAlphaSort();
#endif
	res=FRONTEND_input();
	//MUSIC_process();
	// This was commented out - why????
	// Aha - because otherwise, after a briefing, the music starts up over the memstream music.
	MUSIC_mode_process();


	// dodgy hidden keys

#ifndef VERSION_DEMO

#ifndef TARGET_DC
//#ifndef NDEBUG
	if(ControlFlag && ShiftFlag)
	{
		if (Keys[KB_PPLUS])
		{ Keys[KB_PPLUS]=0; complete_point++; FRONTEND_MissionHierarchy(MISSION_SCRIPT); cheating=1; }
		if (Keys[KB_ASTERISK])
		{ Keys[KB_ASTERISK]=0; complete_point=40; FRONTEND_MissionHierarchy(MISSION_SCRIPT); cheating=1; }
	}
//#endif

#else


#ifdef DEBUG
	// Just for Tom's debugging.
	if (
		(the_state.rgbButtons[DI_DC_BUTTON_A])&&
		(the_state.rgbButtons[DI_DC_BUTTON_B])&&
		(the_state.rgbButtons[DI_DC_BUTTON_X])&&
		(the_state.rgbButtons[DI_DC_BUTTON_Y]))
	{
		if (the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER])
		{ complete_point++; FRONTEND_MissionHierarchy(MISSION_SCRIPT); cheating=1; }
		if (the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER])
		{ complete_point=40; FRONTEND_MissionHierarchy(MISSION_SCRIPT); cheating=1; }
	}
#endif

	// Proper harder to find cheat.
extern int g_iCheatNumber;
	if ( g_iCheatNumber == 0x1a1a0001 )
	{
		// Expose all missions.
		g_iCheatNumber = 0;
		if (the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER])
		{ complete_point=40; FRONTEND_MissionHierarchy(MISSION_SCRIPT); cheating=1; }
	}

#endif

#endif




#ifdef WANT_AN_EXIT_MENU_ITEM
	if (res==FE_NO_REALLY_QUIT) return STARTS_EXIT;
#endif
	if (res==FE_EDITOR)			return STARTS_EDITOR;
	if (res==FE_LOADSCREEN)		return STARTS_START;
#ifdef WANT_A_TITLE_SCREEN
	if (res==FE_CHANGE_LANGUAGE) return STARTS_LANGUAGE_CHANGE;
#endif
#ifndef TARGET_DC
	if (res==FE_START || build_dc)
#else
	if (res==FE_START)
#endif
	{
		//
		// Start playing!!!
		//

		struct
		{
			CBYTE *mission;
			SLONG  dontload;
			SLONG  has_balrog;

		} whattoload[] =
		{
			{"testdrive1a.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"FTutor1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"Assault1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"police1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"police2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"Bankbomb1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"police3.ucm", (1 << PERSON_MIB1) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"Gangorder2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"police4.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"bball2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"wstores1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"e3.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | 0, /* balrog ? */ FALSE},
			{"westcrime1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"carbomb1.ucm", (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"botanicc.ucm", (1 << PERSON_MIB1) | 0, /* balrog ? */ FALSE},
			{"Semtex.ucm", (1 << PERSON_MIB1) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"AWOL1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"mission2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"park2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"MIB.ucm", (1 << PERSON_TRAMP) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"skymiss2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"factory1.ucm", (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"estate2.ucm", (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"Stealtst1.ucm", (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"snow2.ucm", (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"Gordout1.ucm", (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"Baalrog3.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ TRUE },
			{"Finale1.ucm", (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"Gangorder1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"FreeCD1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"jung3.ucm", (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | 0, /* balrog ? */ FALSE},
			{"fight1.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"fight2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"testdrive2.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"testdrive3.ucm", (1 << PERSON_MIB1) | (1 << PERSON_TRAMP) | (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},
			{"Album1.ucm", (1 << PERSON_SLAG_TART) | (1 << PERSON_SLAG_FATUGLY) | (1 << PERSON_HOSTAGE) | (1 << PERSON_MECHANIC) | 0, /* balrog ? */ FALSE},

			{"!"}
		};

		//
		// What level are we loading?
		//

		SLONG index_into_the_whattoload_array;

#ifndef TARGET_DC
		if (build_dc)
		{
		//
			// This bit of code saves out DAD files for all the missions...
			//

			if (suggest_order[build_dc_mission][0] == '!')
			{
				//
				// All done!
				//

				exit(0);
			}

			strcpy(STARTSCR_mission,"levels\\");
			strcat(STARTSCR_mission,suggest_order[build_dc_mission]);

			index_into_the_whattoload_array = build_dc_mission;

			build_dc_mission += 1;
		}
		else
#endif
		{

			previous_mission_launch=mission_launch;
			strcpy(STARTSCR_mission,"levels\\");
			strcat(STARTSCR_mission,FRONTEND_MissionFilename(MISSION_SCRIPT,menu_state.mode-100));
	//		strcpy(STARTSCR_mission,"c:\\master~1\\italian\\bankbomb1.ucm");

			index_into_the_whattoload_array = -1;

			SLONG i;

			for (i = 0; whattoload[i].mission[0] != '!'; i++)
			{
				if (strcmp(FRONTEND_MissionFilename(MISSION_SCRIPT,menu_state.mode-100), whattoload[i].mission) == 0)
				{
					ASSERT(index_into_the_whattoload_array == -1);

					index_into_the_whattoload_array = i;
				}
			}

			ASSERT(index_into_the_whattoload_array != -1);
		}

		//
		// What should we or shouldn't we load?
		//

		ASSERT(WITHIN(index_into_the_whattoload_array, 0, 35));

#ifndef TARGET_DC
		extern ULONG DONT_load;

		this_level_has_the_balrog = whattoload[index_into_the_whattoload_array].has_balrog;
		this_level_has_bane       = (index_into_the_whattoload_array == 27);	// Just in the finale...
		DONT_load                 = whattoload[index_into_the_whattoload_array].dontload;

		//
		// This makes us load all people. Comment it out to load only the
		// people the level needs.
		//

		DONT_load = 0;

		// If doing all levels, don't use DONT_load. The two don't mix.
		ASSERT ( !DONT_load || !build_dc );

		//
		// Does this level have violence?
		//
#endif
		if(index_into_the_whattoload_array==20) //semtex  wetback
		{
			is_semtex=1;
		}
		else
		{
			is_semtex=0;
		}

		if (index_into_the_whattoload_array == 31 ||
			index_into_the_whattoload_array == 32 ||
			index_into_the_whattoload_array == 1)
		{
			//
			// No violence.
			//

			VIOLENCE = FALSE;
		}
		else
		{
			//
			// Default violence.
			//

			VIOLENCE = TRUE;
		}

		if (cheating) FRONTEND_diddle_stats();

#ifdef TARGET_DC
		g_iLevelNumber = mission_launch;
#endif

		FRONTEND_diddle_music();
		menu_state.stackpos = 0;
		menu_thrash         = 0;
		return STARTS_START;
	}

	if (res == FE_CREDITS)
	{
		if (SOFTWARE && 0)
		{
			//
			// This won't work in software... we need other credits :(
			//
		}
		else
		{

#ifdef TARGET_DC

			DreamCastCredits();

#else
#if 0
			extern void OS_hack(void);

			OS_hack();
#endif
#endif

			MUSIC_mode(MUSIC_MODE_FRONTEND);
			FRONTEND_kibble_init();
		}
	}

	return 0;
}





#ifdef TARGET_DC
// Unload frontend gubbins to save memory.
// Can be safely called multiple times.
void FRONTEND_unload ( void )
{
	FRONTEND_scr_unload_theme();
	FRONTEND_kibble_destroy();
}
#endif




#endif




