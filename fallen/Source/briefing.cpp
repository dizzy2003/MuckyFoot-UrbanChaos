//
// briefing.cpp
// mission selection, briefings
// 14 dec 98
//

#include	"game.h"

#ifndef	PSX
#include "fallen/ddengine/headers/font3d.h"
#include "fallen/ddengine/headers/poly.h"
#include "fallen/ddengine/headers/aeng.h"
#endif
#include "briefing.h"
#include "fallen/DDLibrary/headers/D3DTexture.h"
#include "fallen/DDLibrary/headers/GDisplay.h"
#include "sound.h"
#include "io.h"

//------------------------------------------------------------------------
// Temporary Blatantly Hackish Stuff

static UBYTE completed_missions = 0;
static SWORD current_selection = -1;
CBYTE BRIEFING_mission_filename[_MAX_PATH];

CBYTE *mission_names[] = {
	"FIRST ASSIGNMENT",
	"RESCUE ROPER",
	"AIRBASE",
	"DEFUSE",
	"GANG STORAGE",
	"GANG SHIPMENTS",
	"ZOOJACKING",
	"POLICE STATION"
};

CBYTE *mission_filenames[] = {
	"mission1.ucm",
	"breakin1.ucm",
	"air1b.ucm",
	"hot1amiss1.ucm",
	"area5b.ucm",
	"storage.ucm",
	"zoo.ucm",
	"police.ucm"
};

UBYTE mission_ranges[][2] = {
	{ 0, 0 },
	{ 1, 1 },
	{ 2, 2 },
	{ 3, 3 },
	{ 4, 5 },
	{ 4, 5 },
	{ 6, 7 },
	{ 6, 7 },
};


//------------------------------------------------------------------------
// Load and save game data

CBYTE BRIEFING_load_game(void) {
	MFFileHandle handle;
	SLONG		 test;

	handle = FileOpen("savegame.dat");
	test=(SLONG)handle;
	if (test<0) return 0; // Could not open file
	FileRead(handle,&completed_missions,1);
	FileClose(handle);
	return 1;
}

CBYTE BRIEFING_save_game(void) {
	MFFileHandle     handle;
	SLONG			 test;

	handle = FileCreate("savegame.dat",1);
	test=(SLONG)handle;
	if (test<0) return 0; // Could not open file
	FileWrite(handle,&completed_missions,1);
	FileClose(handle);
	return 1;
}

//------------------------------------------------------------------------
// Draw a crappy text menu of the missions

void BRIEFING_menu(void) {
/*
	SLONG	c0;
	SLONG	y;
	UBYTE   is_this;
	UBYTE	min,max;
	SLONG	text_colour;
	static  Font3D font("data\\font3d\\all\\",(float)0.2);

	min=mission_ranges[completed_missions][0];
	max=mission_ranges[completed_missions][1];

	y=240-(25*(max+1));

	for (c0=0;c0<=max;c0++) {
		text_colour = (c0<min) ? 0x010201 : 0x020101;
		is_this=(c0==current_selection);
		if (is_this)
			text_colour*=0x7f;
		else
			text_colour*=0x50;

		font.DrawString(mission_names[c0],320,y,text_colour,2.5+(is_this*0.5f),is_this);
		y+=50;
	}

	font.DrawString("L LOAD",320,420,0xFFFFFF,2.0,0);
	font.DrawString("S SAVE",320,440,0xFFFFFF,2.0,0);
*/
}

//------------------------------------------------------------------------
// Select a mission according to the script. Return the filename

SBYTE BRIEFING_select(void) {
	SLONG wave;

	if (current_selection==-1) {
		current_selection=mission_ranges[completed_missions][0];
		BRIEFING_mission_filename[0]=0;
	}

	do {
#ifdef USE_A3D
//	A3DCamera(0,0,0,0,0,0);
//	A3DRender();
#endif
	ReadInputDevice();
	AENG_clear_screen();
	POLY_frame_init(FALSE, FALSE);
	ShowBackImage();
	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);
	BRIEFING_menu();
	POLY_frame_draw(FALSE,TRUE);

	AENG_flip();

	if(Keys[KB_ENTER])
	{
		Keys[KB_ENTER]=0;
//		play_quick_wave_xyz(-300,0,300,S_MANHOLE_COVER,0,0);
//		play_quick_wave_xyz( 300,1,300,S_MANHOLE_COVER,0,0);
		strcpy(BRIEFING_mission_filename,LEVELS_DIR);
		strcpy(BRIEFING_mission_filename,"levels\\");
		strcat(BRIEFING_mission_filename,mission_filenames[current_selection]);
		current_selection=-1;
		return 1;
	}
	if(Keys[KB_UP])
	{
		Keys[KB_UP]=0;
		if(current_selection) current_selection--;
//		play_quick_wave_xyz(-150,0,1,S_FOOTS_RUNG_START,0,0);
//		play_quick_wave_xyz( 150,0,1,S_FOOTS_RUNG_END,0,0);
	}
	if(Keys[KB_DOWN])
	{
		Keys[KB_DOWN]=0;
		current_selection++;
		if(current_selection>mission_ranges[completed_missions][1]) current_selection--;
//		play_quick_wave_xyz(-150,0,-(30+(rand()&0x7f)),S_FOOTS_RUNG_START,0,0);
//		play_quick_wave_xyz( 150,0,-(30+(rand()&0x7f)),S_FOOTS_RUNG_START+1,0,0);
	}
	if(Keys[KB_ESC]) {
		Keys[KB_ESC]=0;
		return -1;
	}
	if(Keys[KB_PPLUS]) {
		Keys[KB_PPLUS]=0;
		completed_missions++;
	}
	if (Keys[KB_L]) {
		Keys[KB_L]=0;
		if (BRIEFING_load_game())
			wave=S_EXPLODE_START+1;
//		else
//			wave=S_ARGH;
//		play_quick_wave_xyz(-150,0,-(30+(rand()&0x7f)),wave,0,0);
//		play_quick_wave_xyz( 150,0,-(30+(rand()&0x7f)),wave,0,0);
	}
	if (Keys[KB_S]) {
		Keys[KB_S]=0;

		if (BRIEFING_save_game())
			wave=S_EXPLODE_END;
//		else
//			wave=S_ARGH;

//		play_quick_wave_xyz(-150,0,-(30+(rand()&0x7f)),wave,0,0);
//		play_quick_wave_xyz( 150,0,-(30+(rand()&0x7f)),wave,0,0);
	}

	} while (SHELL_ACTIVE); // smirk

	return 0;



}

SBYTE BRIEFING_next_mission(void) {
	completed_missions++;
	if (completed_missions>=sizeof(mission_names)/4) return 0;
	BRIEFING_select();
	return 1;
}
