// env2.cpp
//
// ENV stuff #2

#include "game.h"
#include <MFStdLib.h>
#include "env.h"
#include "Interfac.h"
#include "menufont.h"

#ifdef TARGET_DC
#include "platutil.h"
#endif

CBYTE	inifile[_MAX_PATH];
CBYTE	strbuf[_MAX_PATH];


#ifndef TARGET_DC


void ENV_load(CBYTE *fname)
{
	GetCurrentDirectory(_MAX_PATH, inifile);
	if (inifile[strlen(inifile) - 1] != '\\')	strcat(inifile, "\\");
	strcat(inifile, fname);

	TRACE("Full INI file pathname = %s\n", inifile);

	SLONG local = GetPrivateProfileInt("MuckyFoot", "local", 0, inifile);

	if (local)
	{
		strcpy(inifile, "c:\\fallen.ini");
		TRACE("Using local INI file %s\n", inifile);
	}
}

CBYTE* ENV_get_value_string(CBYTE* name, CBYTE* section)
{
	GetPrivateProfileString(section, name, "", strbuf, _MAX_PATH, inifile);
	TRACE("[%s] %s = \"%s\"\n", section, name, strbuf);
	return strbuf[0] ? strbuf : NULL;
}

SLONG ENV_get_value_number(CBYTE* name, SLONG def, CBYTE* section)
{
	SLONG val = GetPrivateProfileInt(section, name, def, inifile);
	TRACE("[%s] %s = %d\n", section, name, val);
	if (stricmp(section, "Secret"))	ENV_set_value_number(name, val, section);	// don't write out "psx" key
	return val;
}

void ENV_set_value_string(CBYTE* name, CBYTE* value, CBYTE* section)
{
	WritePrivateProfileString(section, name, value, inifile);
}

void ENV_set_value_number(CBYTE* name, SLONG value, CBYTE* section)
{
	sprintf(strbuf, "%d", value);
	WritePrivateProfileString(section, name, strbuf, inifile);
}


#else //#ifndef TARGET_DC


// OK, have a list of pre-made registry names, and enum them.
// Then store the values in memory, and we can save that chunk out.
// We ignore the "section" bit.

//#define NUM_ENV_NAMES 10

char *pchNameList[] =
{
	"joypad_mode",
	"joypad_kick",
	"joypad_punch",
	"joypad_jump",
	"joypad_action",
	"joypad_move",
	"joypad_start",
	"joypad_select",
	"joypad_camera",
	"joypad_cam_left",
	"joypad_cam_right",
	"joypad_1stperson",

	"scanner_follows",
	"draw_distance",

	"ambient_volume",
	"music_volume",
	"fx_volume",

	"detail_stars",
	"detail_shadows",
	"detail_moon_reflection",
	"detail_people_reflection",
	"detail_puddles",
	"detail_dirt",
	"detail_mist",
	"detail_rain",
	"detail_skyline",
	"detail_filter",
	"detail_perspective",
	"detail_crinkles",

	"cheat_driving_bronze",

	"analogue_pad_mode",

	"play_movie",

	"panel_x",
	"panel_y",

	//"Mode",
	//"BitDepth",

	"lang_num",

	// Is vibration on or off, and a sub-enable for engines.
	"vibration_mode",
	"vibration_engine",

// Only ever add to the end of this list - order is important.
// If you do add to this, also add to ENV_init below.

	"",
};


BYTE bEnvValue[ sizeof(pchNameList) / sizeof(pchNameList[0]) ];


// Feed it the start of the env save/load block, and
// it loads the block and returns a pointer to the end of the block.
char *ENV_load ( char *pcData )
{
	int iNumStrings = (int)*pcData++;

	for ( int iStringNum = 0; iStringNum < iNumStrings; iStringNum++ )
	{
		bEnvValue[iStringNum] = *pcData++;
	}

	return ( pcData );
}

// Call once with pcData = NULL, and it returns the number of
// bytes it needs to save the data. Call it again with a pointer
// to the start of this data, and it will actually fill in the data
// (and return the number of bytes again).
int ENV_save ( char *pcData )
{
	// First see how many env variables there are.
	int iNumStrings = 0;
	while ( TRUE )
	{
		if ( pchNameList[iNumStrings ][0] == '\0' )
		{
			break;
		}
		iNumStrings++;
	}

	if ( pcData != NULL )
	{
		// Now write it out.
		*pcData++ = (BYTE)iNumStrings;
		for ( int iStringNum = 0; iStringNum < iNumStrings; iStringNum++ )
		{
			*pcData++ = bEnvValue[iStringNum];
		}
	}

	// Remember the extra byte for the number of strings!
	return ( iNumStrings + 1 );
}

// Unique to DC.
void ENV_init ( void )
{
	// Set up standard environment stuff, i.e. before a game has been loaded.


	// Set up Standard dreamcast mapping.
	ENV_set_value_number("joypad_mode",			1, "Joypad");
	// And set up the default custom mode.
	ENV_set_value_number("joypad_kick",			DI_DC_BUTTON_LTRIGGER, "Joypad");
	ENV_set_value_number("joypad_punch",		DI_DC_BUTTON_RTRIGGER, "Joypad");
	ENV_set_value_number("joypad_jump",			DI_DC_BUTTON_Y, "Joypad");
	ENV_set_value_number("joypad_action",		DI_DC_BUTTON_A, "Joypad");
	ENV_set_value_number("joypad_move",			DI_DC_BUTTON_UP, "Joypad");
	ENV_set_value_number("joypad_start",		DI_DC_BUTTON_START, "Joypad");
	ENV_set_value_number("joypad_select",		DI_DC_BUTTON_B, "Joypad");
	ENV_set_value_number("joypad_camera",		DI_DC_BUTTON_DOWN, "Joypad");
	ENV_set_value_number("joypad_cam_left",		DI_DC_BUTTON_LEFT, "Joypad");
	ENV_set_value_number("joypad_cam_right",	DI_DC_BUTTON_RIGHT, "Joypad");
	ENV_set_value_number("joypad_1stperson",	DI_DC_BUTTON_X, "Joypad");

	// Other stuff.
	ENV_set_value_number ( "scanner_follows",			1, "" );
	ENV_set_value_number ( "draw_distance",				22, "" );

	ENV_set_value_number ( "ambient_volume",			127, "" );
	ENV_set_value_number ( "music_volume",				127, "" );
	ENV_set_value_number ( "fx_volume",					127, "" );

	ENV_set_value_number ( "detail_stars",				0, "" );
	ENV_set_value_number ( "detail_shadows",			1, "" );
	ENV_set_value_number ( "detail_moon_reflection",	0, "" );
	ENV_set_value_number ( "detail_people_reflection",	0, "" );
	ENV_set_value_number ( "detail_puddles",			0, "" );
	ENV_set_value_number ( "detail_dirt",				1, "" );
	ENV_set_value_number ( "detail_mist",				1, "" );
	ENV_set_value_number ( "detail_rain",				1, "" );
	ENV_set_value_number ( "detail_skyline",			1, "" );
	ENV_set_value_number ( "detail_filter",				1, "" );
	ENV_set_value_number ( "detail_perspective",		1, "" );
	ENV_set_value_number ( "detail_crinkles",			0, "" );

	// Is the Bronze Diriving cheat on?
	ENV_set_value_number ( "cheat_driving_bronze",		0, "" );

	// Analog mode. Actually not analogue/digital, but relative/absoloute mode.
	ENV_set_value_number ( "analogue_pad_mode",			0, "" );


	ENV_set_value_number ( "play_movie",				1, "" );

#ifdef DEBUG
	// Where the panel is on the screen. Numbers are divided by four to fit into a byte.
	// This is in the corner for debug, coz I keep forgetting to move it there when taking screenshots.
	ENV_set_value_number ( "panel_x",					0 / 4, "" );
	ENV_set_value_number ( "panel_y",					(480-0) / 4, "" );
#else
	// Where the panel is on the screen. Numbers are divided by four to fit into a byte.
	ENV_set_value_number ( "panel_x",					32 / 4, "" );
	ENV_set_value_number ( "panel_y",					(480-32) / 4, "" );
#endif

#ifdef VERSION_FRENCH
	// Set the default, just to test.
	// Language number. 0 = English. 1 = French.
	ENV_set_value_number ( "lang_num",					1, "" );
#else
	// Get it from the firmware (boot rom setting).
	BYTE bLang = FirmwareGetLanguageCode();
	if ( bLang == LANGUAGE_FRENCH )
	{
		// Use French.
		ENV_set_value_number ( "lang_num", 1, "" );
	}
	else
	{
		// Everyone else gets English by default.
		ENV_set_value_number ( "lang_num", 0, "" );
	}
#endif


	// Now is as good a time as any to check for English/American
	BYTE bCountry = FirmwareGetCountryCode();
	if ( bCountry == COUNTRY_AMERICA )
	{
		// Then Yanks have VMUs.
		bWriteVMInsteadOfVMU = FALSE;
	}
	else
	{
		// Everyone else has VMs.
		bWriteVMInsteadOfVMU = TRUE;
	}


	ENV_set_value_number ( "vibration_mode",				1, "" );
	ENV_set_value_number ( "vibration_engine",				1, "" );


	//ENV_set_value_number ( "Mode",						0, "" );
	//ENV_set_value_number ( "BitDepth",					0, "" );
}


// Fudged.
CBYTE* ENV_get_value_string(CBYTE* name, CBYTE* section)
{
	// Only one string, the language. This is kludged for the XLAT system,
	// most things should take language settings from "lang_num".

	ASSERT ( strcmp ( name, "language" ) == 0 );

	switch ( ENV_get_value_number ( "lang_num", 0, "" ) )
	{
	case 0:
		// English.
		return ( "text\\lang_english.txt" );
		break;
	case 1:
		// French.
		return ( "text\\lang_french.txt" );
		break;
	default:
		ASSERT ( FALSE );
		return ( "text\\lang_english.txt" );
		break;
	}
}

SLONG ENV_get_value_number(CBYTE* name, SLONG def, CBYTE* section)
{
	int iStringNum = 0;
	while ( TRUE )
	{
		if ( pchNameList[iStringNum][0] == '\0' )
		{
			// Reached the end of the list. Oops.
			ASSERT ( FALSE );
			return ( def );
			break;
		}
		if ( 0 == stricmp ( pchNameList[iStringNum], name ) )
		{
			return ( bEnvValue[iStringNum] );
		}
		iStringNum++;
	}
	ASSERT ( FALSE );
	return ( def );
}

// Unused.
void ENV_set_value_string(CBYTE* name, CBYTE* value, CBYTE* section)
{
	ASSERT ( FALSE );
}

void ENV_set_value_number(CBYTE* name, SLONG value, CBYTE* section)
{
	int iStringNum = 0;
	// MUST be a byte value.
	ASSERT ( ( value >= 0 ) && ( value <= 255 ) );
	while ( TRUE )
	{
		if ( pchNameList[iStringNum][0] == '\0' )
		{
			// Reached the end of the list. Oops.
			ASSERT ( FALSE );
			return;
		}
		if ( 0 == stricmp ( pchNameList[iStringNum], name ) )
		{
			bEnvValue[iStringNum] = value;
			return;
		}
		iStringNum++;
	}
	ASSERT ( FALSE );
}


#endif //#else //#ifndef TARGET_DC

