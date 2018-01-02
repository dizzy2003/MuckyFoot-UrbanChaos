

// most of this file is obsolete
// just a few torn scraps remain...

#ifndef EDITOR

#include "MFStdLib.h"

CBYTE STARTSCR_mission[_MAX_PATH] = {0};


#else

#include	"Game.h"
#include	"startscr.h"

#ifndef		PSX
#include	"poly.h"
#include	"text.h"
#include    "flamengine.h"
#include    "font3d.h"
#include    "D3DTexture.h"
#include    "GDisplay.h"
#include	"sound.h"
#include	"menufont.h"
#include	"texture.h"
#include	"drawxtra.h"
#include	"night.h"
#include	"io.h"
#include	"ob.h"
#include	"vertexbuffer.h"
#include	"polypoint.h"
#include	"renderstate.h"
#include	"polypage.h"
#include	"password.h"
#include	"interfac.h"
#include	"env.h"
//#include	"Shlwapi.h"

#include	"MFx.h"

#endif

#include	"widget.h"

// editor stuff being roped in for background view

#include	"gi.h"
#include	"engwind.h"
#include	"mapview.h"
#include	"xlat_str.h"

#define MISSION_SCRIPT "data\\urban.sty"

//#define USE_WIDGETS
//#define USE_PASSWORD
//#define USE_TIMEOUT
//#define E3_BODGE
#define TIMEOUT_YEAR	1999
#define TIMEOUT_MONTH	5
#define TIMEOUT_DAY		30
#define FORCE_FIRST_MISSION (0)

#if defined(USE_PASSWORD)
CBYTE magic_key[] = MAGIC_ARRAY;
#endif

CBYTE STARTSCR_mission[_MAX_PATH] = {0};
UBYTE STARTSCR_miss_id=0;
UBYTE STARTSCR_miss_autoadvance=0;
//CBYTE* STARTSCR_mission=0;

struct	StartMenuItemSimple	startmenu_special[]=
{
	{"INNER CITY",0,1,0,0},
	{"RESCUE ROPER",0,2,0,0},
	{"AIRBASE",0,3,0,0},
	{"HOTEL",0,4,0,0},
	{"DOCKS",0,5,0,0},
	{"WAREHOUSE",0,6,0,0},
	{"ROOFTOP",0,7,0,0},
	{"TESTBIKE",0,8,0,0},
	{"EXIT",0,9,0,0}
};

struct	StartMenuItemSimple	startmenu[]=
{
	{"START",0,STARTS_START,0,0},
	{"PLAYBACK ",0,STARTS_PLAYBACK,0,0},

	{"EDITOR",0,STARTS_EDITOR,0,0},
	{"REAL MENU",0,STARTS_LOAD,0,0},
	{"OPTIONS",2,0,0,0},
	{"MULTIPLAYER",0,STARTS_MULTI,0,0},

	{"EXIT",0,STARTS_EXIT,0,0},

	{"SOUND",4,0,0,0},
	{"GRAPHICS",5,0,0,0},
	{"INPUT",0,0,0,0},
	{"BACK",1,0,0,0,},

	{"HOST A GAME",0,STARTS_HOST,0,0},
	{"JOIN A GAME",0,STARTS_JOIN,0,0},
	{"BACK",1,0,0,0,}

};

struct	StartMenuItemComplex	startmenu2[]=
{
	{"SHADOWS",{"HIGH","MEDIUM","LOW"},0,0,0,0},
	{"DRAW DIST",{"FAR","MEDIUM","NEAR"},0,0,0,0},
	{"DIRT",{"RANCID","MUCKY","CLEAN"},0,0,0,0},
	{"REFLECTIONS",{"ON","OFF",0},0,0,0,0},
	{"RESOLUTION",{"HIGH","LOW",0},0,0,0,0},
	{"BACK",{0,0,0},2,0,0,0},

	{"SOUND QUALITY",{"HIGH","MEDIUM","LOW"},0,0,0,0},
	{"SOUND VOLUME",{"HIGH","MEDIUM","OFF"},0,0,0,0},
	{"MUSIC VOLUME",{"HIGH","MEDIUM","OFF"},0,0,0,0},
	{"BACK",{0,0,0},2,0,0,0}


};

struct MissionData {
	SLONG ObjID, GroupID, ParentID, ParentIsGroup;
	SLONG District;
	SLONG Type, Flags;
	CBYTE fn[255], ttl[255];
};

struct	StartMenu	start_menu[]=
{

	{0,0,0,0}, // dummy
	{0,7,0,0}, // Main Menu 
	{6,4,0,0}, // Options
	{10,3,0,0}, // multiplayer
	{6,4,0,1}, // sfx
	{0,6,0,1}, // graphics

};

static UBYTE mission_num;
static CHAR mission_name[_MAX_PATH]; // this is getting kinda skanky now

extern BOOL  text_fudge;
extern ULONG text_colour;

BOOL show_game = 1;

void	STARTSCR_plonk_logo(void) {
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

	pp[0].specular=pp[1].specular=pp[2].specular=pp[3].specular=0xff000000;
	pp[0].colour=pp[1].colour=pp[2].colour=pp[3].colour=0xFFFFFFFF;
	pp[0].Z=pp[1].Z=pp[2].Z=pp[3].Z=0.5f;
/*	pp[0].u=0.0;	pp[0].v=0.0;
	pp[1].u=1.0;	pp[1].v=0.0;
	pp[2].u=0.0;	pp[2].v=1.0;
	pp[3].u=1.0;	pp[3].v=1.0;*/
	pp[0].u=0.0;	pp[0].v=0.0;
	pp[1].u=0.628;	pp[1].v=0.0;
	pp[2].u=0.0;	pp[2].v=0.363;
	pp[3].u=0.628;	pp[3].v=0.363;

	pp[0].X=0;		pp[0].Y=0;
	pp[1].X=161;	pp[1].Y=0;
	pp[2].X=0;		pp[2].Y=93;
	pp[3].X=161;	pp[3].Y=93;

	POLY_add_quad(quad,POLY_PAGE_MENULOGO,FALSE,TRUE);
}


void	draw_a_menu(SLONG	menu)
{
	SLONG	c0;
	SLONG	y;
	SLONG	complex;


	text_fudge  = FALSE;

	y=200-20*start_menu[menu].Count;

	complex=start_menu[menu].Type;

	for(c0=0;c0<start_menu[menu].Count;c0++)
	{
		CBYTE	*str;

		if(c0==start_menu[menu].Current)
			text_colour = 0x00eeee00;
		else
			text_colour = 0x00eeeeff;

		if(!complex)
		{
			str=startmenu[c0+start_menu[menu].StartIndex].Str;
			if(str)
				draw_centre_text_at(320, y,str,0,0);
		}
		else
		{
			SLONG	item;
			CBYTE	*str;
			item=startmenu2[c0+start_menu[menu].StartIndex].Item;
			draw_text_at(150, y,startmenu2[c0+start_menu[menu].StartIndex].Str,0);
			
			str=startmenu2[c0+start_menu[menu].StartIndex].Strb[item];
			if(str)
				draw_text_at(350, y,str,0);

		}

		y+=20;
	}
}
#ifndef	PSX

#if 0
void	draw_a_3d_menu(Font3D &font, SLONG	menu) 
{
	SLONG	c0;
	SLONG	y;
	SLONG	complex;
	UBYTE   isthis;


	text_fudge  = FALSE;

	y=240-(25*start_menu[menu].Count);

	complex=start_menu[menu].Type;

	for(c0=0;c0<start_menu[menu].Count;c0++)
	{
		CBYTE	*str;

		isthis=(c0==start_menu[menu].Current);
		if(isthis)
			text_colour = 0xffffff;
		else
			text_colour = 0xff9f9f;

		if(!complex)
		{
			str=startmenu[c0+start_menu[menu].StartIndex].Str;
			if(str) font.DrawString(str,320,y,text_colour,2.5+(isthis*0.5f),isthis);

		}
		else
		{
			SLONG	item;
			CBYTE	*str;
			item=startmenu2[c0+start_menu[menu].StartIndex].Item;
//			draw_text_at(150, y,startmenu2[c0+start_menu[menu].StartIndex].Str,0);
			font.DrawString(startmenu2[c0+start_menu[menu].StartIndex].Str,200,y,text_colour,2.0+(isthis*0.5f),isthis);
			
			str=startmenu2[c0+start_menu[menu].StartIndex].Strb[item];
			if(str) font.DrawString(str,500,y,text_colour,2.0+(isthis*0.5f),isthis);

		}

		y+=50;
	}
}

#endif

//void MENUFONT_Draw_floats(float x, float y, UWORD scale, CBYTE *msg, SLONG rgb, UBYTE flags, UBYTE haloscale);


void	draw_a_new_menu(SLONG	menu, SLONG localctr) 
{
	SLONG	c0;
	SLONG	y,rgb;
	SLONG	complex;
	UBYTE   isthis;
	UWORD	flags;
	SLONG	gap=20;


	y=255-(gap*start_menu[menu].Count);

	complex=start_menu[menu].Type;

	for(c0=0;c0<start_menu[menu].Count;c0++)
	{
		CBYTE	*str;

		isthis=(c0==start_menu[menu].Current);
		rgb=(localctr<0xff) ? localctr : 0xff;
		rgb<<=24;
		rgb|=0x7fffcf;
		if(!complex)
		{
			str=startmenu[c0+start_menu[menu].StartIndex].Str;
			flags = MENUFONT_HSCALEONLY|MENUFONT_CENTRED | ( isthis ? MENUFONT_SHAKE : 0);
			if (localctr<0xff) 
			{
				if (str) 
					MENUFONT_Draw(436,y,256,str,rgb,flags);
				// I don't like these MikeD
//					MENUFONT_Draw_floats(388,y,localctr,str,rgb,flags);
			} 
			else
			{
				if (str) 
				{
					MENUFONT_Draw(388,y,256,str,rgb,flags);
				// I don't like these MikeD
					/*
					if (localctr<0x1ff) 
					{
						rgb=((0x1ff-localctr)>>1)<<24;
						rgb|=0x7fffcf;
						MENUFONT_Draw_floats(388,y,localctr,str,rgb,flags);
					}
					*/
				}
			}


		}
		else
		{
			SLONG	item;
			CBYTE	*str;
			item=startmenu2[c0+start_menu[menu].StartIndex].Item;
//			font.DrawString(startmenu2[c0+start_menu[menu].StartIndex].Str,200,y,text_colour,2.0+(isthis*0.5f),isthis);
			flags = MENUFONT_RIGHTALIGN | ( isthis ? MENUFONT_SHAKE : 0);
			MENUFONT_Draw(420,y,256,startmenu2[c0+start_menu[menu].StartIndex].Str,rgb,flags);
			
			flags = ( isthis ? MENUFONT_SHAKE : 0);
			str=startmenu2[c0+start_menu[menu].StartIndex].Strb[item];
			if (str)MENUFONT_Draw(436,y,256,str,rgb,flags);
//			if(str) font.DrawString(str,500,y,text_colour,2.0+(isthis*0.5f),isthis);

		}

		y+=gap+gap;
	}
}


#endif


#ifndef PSX

CBYTE* LoadAString(MFFileHandle &file, CBYTE *txt) {
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

void SaveAString(MFFileHandle &file, CBYTE *txt) {
	CBYTE *ptr=txt;
	CBYTE crlf[] = { 13, 10};

	FileWrite(file,txt,strlen(txt));
	FileWrite(file,crlf,2);
}


#endif



void LoadFont_CRT(void) 
{
//	MENUFONT_Load("font2.tga",POLY_PAGE_NEWFONT,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz$%&()+-\"?,.;:/#0123456789°·×_|}{÷");
//	MENUFONT_Load("data\\textures\\extras\\olyfont2.tga",POLY_PAGE_NEWFONT_INVERSE,"abcdefghijklmnopqrstuvwxyz0123456789.,!\":;'#$*-()[]\\/?");
#ifdef TARGET_DC
	MENUFONT_Load("olyfont2dc.tga",POLY_PAGE_NEWFONT_INVERSE,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$*-()[]\\/?");
#else
	MENUFONT_Load("olyfont2.tga",POLY_PAGE_NEWFONT_INVERSE,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$*-()[]\\/?");
#endif

void MENUFONT_MergeLower(void);	
	MENUFONT_MergeLower();
}

void LoadFont_LCD(void) {
//	MENUFONT_Load("data\\textures\\extras\\font3.tga",POLY_PAGE_NEWFONT_INVERSE,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz$%&()+-\"?,.;:/#0123456789°·×_|}{÷");
}

SLONG MainWidgetLoop(void);

extern SLONG FONT_TICK;
//extern ULONG get_hardware_input(UWORD type);
//#define	INPUT_TYPE_JOY	(1<<1)

#ifndef TARGET_DC
SLONG	MainE3BodgeLoop() {
	static int input = 0;
	static int ticker=0;
	int lastinput;
	static int screen=0;
	static int dont_play_movie=ENV_get_value_number("prevent_movie", 0);
	static int movie_delay=INFINITY;

	if (movie_delay==INFINITY)
	{
		movie_delay = ENV_get_value_number("delay_movie", 60) * 60;
	}

	lastinput=input;

	input=get_hardware_input(INPUT_TYPE_JOY);
	if ((input&&(input!=lastinput)&&(ticker<1))||(Keys[KB_SPACE])) {
		if ((input&(INPUT_MASK_JUMP|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION|INPUT_MASK_START))||Keys[KB_SPACE]) {
			Keys[KB_SPACE]=0;
			MFX_play_xyz(0,S_FOOTS_RUNG_START,MFX_OVERLAP,-150,0,1);
			MFX_play_xyz(0,S_FOOTS_RUNG_END,MFX_OVERLAP, 150,0,1);
			if (!screen) {
				InitBackImage("e3miss.tga");
				screen++;
				return 0;
			} else {
				InitBackImage("e3miss.tga");
				screen=0;
				strcpy(STARTSCR_mission,"levels\\e3.ucm");
				return STARTS_START;
			}
		}
		ticker=10;
	}
	if (((abs(ticker)&0xff)<128)&&!screen) {

			POLY_Point  pp[4];
			POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

			pp[0].specular=pp[1].specular=pp[2].specular=pp[3].specular=0xff000000;
			pp[0].colour=pp[1].colour=pp[2].colour=pp[3].colour=0xFF000000;
			pp[0].Z=pp[1].Z=pp[2].Z=pp[3].Z=0.5f;
			pp[0].u=0.0;	pp[0].v=0.0;
			pp[1].u=0.628;	pp[1].v=0.0;
			pp[2].u=0.0;	pp[2].v=0.363;
			pp[3].u=0.628;	pp[3].v=0.363;

			pp[0].X=406;		pp[0].Y=385;
			pp[1].X=406+144;	pp[1].Y=385;
			pp[2].X=406;		pp[2].Y=385+43;
			pp[3].X=406+144;	pp[3].Y=385+43;

			POLY_add_quad(quad,POLY_PAGE_MENULOGO,FALSE,TRUE);

	}
	ticker--;
	if (ticker==-movie_delay) { // approx every 2 mins or so
extern int do_only_game_intro(void);
		if (dont_play_movie!=1) do_only_game_intro();
		ticker=0;
	}
	return 0;
}
#endif //#ifndef TARGET_DC


SLONG	do_start_menu(void)
{
	SLONG res;

#ifdef	PSX
	return(STARTS_START);
#else
	static	SLONG menu=1;
	static  Flamengine fire("data\\testfire3.phy");
//	static  Font3D font("data\\font3d\\");
//	static  Font3D font("data\\font3d\\urban\\");
//	static  Font3D font("data\\font3d\\all\\",0.2);
	static  float i=1;
	static	SLONG tune_id=0;
	static	SLONG localctr = 0;
	static  BOOL realmenu=0;

	static  BOOL doneload=0;

	if (!doneload) {
		CBYTE *lang=ENV_get_value_string("language");
		doneload=1;
		if (!lang) lang="text\\lang_english.txt";
		XLAT_load(lang);
		XLAT_init();
		LoadFont_CRT();
#define	NO_MAP	1

#ifdef NO_MAP
		InitBackImage("TITLE LEAVES1.TGA");
		show_game=0;
#else
		GI_load_map("data\\gang1.iam");
		NIGHT_load_ed_file("data\\lighting\\gang1.lgt");
#endif

	}

//	tune_id=play_quick_wave_xyz(0,0,-5,S_TUNE_SEWER,tune_id,WAVE_PLAY_NO_INTERRUPT);

	fire.params.randomize=1;

	// Do this outside POLY_xxx block because it needs to, well, fuck things
	// up royally.
//    fire.Run();	

	POLY_frame_init(FALSE, FALSE);
//	draw_a_menu(menu);
//    the_display.blit_background_surface();
//	ShowBackImage();

extern void GI_render_view_into_backbuffer(
		SLONG cam_x,
		SLONG cam_y,
		SLONG cam_z,
		SLONG cam_yaw,
		SLONG cam_pitch,
		SLONG cam_roll);
extern void calc_camera_pos(void);

		if (show_game)
		{
			cam_focus_x		=	64 << 8;
			cam_focus_z		=	64 << 8;
			cam_focus_dist	=	12 << 8;
			cam_pitch		=	1800;
			cam_yaw			=	256+(SIN(FONT_TICK & 2047)>>8);
			calc_camera_pos();
			GI_render_view_into_backbuffer	(
												cam_x,
												cam_y,
												cam_z,
												cam_yaw,
												cam_pitch,
												0
											);
		} else {
			ShowBackImage();
		}

	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);

#ifndef E3_BODGE
	//STARTSCR_plonk_logo();
#else
	res=MainE3BodgeLoop();
	POLY_frame_draw(FALSE,TRUE);
	return res;
#endif
	
#ifdef USE_WIDGETS
//void test_the_widgets(void);

//	test_the_widgets();
	res=MainWidgetLoop();
#else
	if (realmenu) {
		res=MainWidgetLoop();
		POLY_frame_draw(FALSE,TRUE);
		MFX_set_listener(0,0,0,0,0,0);
		MFX_render();
		return res;
	} else 
		draw_a_new_menu(menu, localctr+=6);
#endif

	FONT_TICK++;

//	fire.Blit();

//	draw_a_3d_menu(font,menu);


	POLY_frame_draw(FALSE,TRUE);

	MFX_set_listener(0,0,0,0,0,0);
	MFX_render();

#ifndef USE_WIDGETS

	if (realmenu) 
		return res;

	if(Keys[KB_ENTER])
	{
		Keys[KB_ENTER]=0;
		localctr=0;
//		play_quick_wave_xyz(-300,0,300,S_MANHOLE_COVER,0,0);
//		play_quick_wave_xyz( 300,1,300,S_MANHOLE_COVER,0,0);
		MFX_play_xyz(0,S_MANHOLE_COVER,MFX_OVERLAP,-300,0,300);
		MFX_play_xyz(0,S_MANHOLE_COVER,MFX_OVERLAP, 300,1,300);
		if(start_menu[menu].Type)
		{
			if(startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].NextMenu)
			{
				menu=startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].NextMenu;
				start_menu[menu].Current=0;
			}
			else
			if(startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Action)
			{
				return(startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Action);
			}
		}
		else
		{
			if(startmenu[start_menu[menu].Current+start_menu[menu].StartIndex].NextMenu)
			{
				menu=startmenu[start_menu[menu].Current+start_menu[menu].StartIndex].NextMenu;
				start_menu[menu].Current=0;
			}
			else
			if(startmenu[start_menu[menu].Current+start_menu[menu].StartIndex].Action)
			{
				if (startmenu[start_menu[menu].Current+start_menu[menu].StartIndex].Action==STARTS_LOAD)
				{
					realmenu=1;
				}
				else
				{
					return(startmenu[start_menu[menu].Current+start_menu[menu].StartIndex].Action);
				}
			}
		}
	}
	if(start_menu[menu].Type)
	{
		if(Keys[KB_LEFT])
		{
			Keys[KB_LEFT]=0;
			if(startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Item)
				startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Item--;
		}
		else
		if(Keys[KB_RIGHT])
		{
			SLONG	item;
			Keys[KB_RIGHT]=0;
			item=startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Item;

			if(item<2 && startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Strb[item+1])
				startmenu2[start_menu[menu].Current+start_menu[menu].StartIndex].Item++;

		}
	}

	if(Keys[KB_UP])
	{
		Keys[KB_UP]=0;
		if(start_menu[menu].Current)
			start_menu[menu].Current--;
//		play_quicker_wave(S_FOOTS_RUNG_START);
//		play_quick_wave_xyz(-150,0,1,S_FOOTS_RUNG_START,0,0);
//		play_quick_wave_xyz( 150,0,1,S_FOOTS_RUNG_END,0,0);
		MFX_play_xyz(0,S_FOOTS_RUNG_START,MFX_OVERLAP,-150,0,1);
		MFX_play_xyz(0,S_FOOTS_RUNG_END,MFX_OVERLAP, 150,0,1);
	}
	if(Keys[KB_DOWN])
	{
		Keys[KB_DOWN]=0;
		start_menu[menu].Current++;
		if(start_menu[menu].Current>=start_menu[menu].Count)
			start_menu[menu].Current--;
//		play_quick_wave_xyz(-150,0,-(30+(rand()&0x7f)),S_FOOTS_RUNG_START,0,0);
//		play_quick_wave_xyz( 150,0,-(30+(rand()&0x7f)),S_FOOTS_RUNG_START+1,0,0);
//		play_quicker_wave(S_FOOTS_RUNG_START);
		MFX_play_xyz(0,S_FOOTS_RUNG_START,MFX_OVERLAP,-150,0,1);
		MFX_play_xyz(0,S_FOOTS_RUNG_END,MFX_OVERLAP, 150,0,1);
	}
#else
	return res;
#endif

	return(0);
#endif
}



void test_the_widgets(void) {
	static Form* form=NULL;
	Widget* widget;

	if (!form) {
		form=FORM_Create("main menu",NULL,205,105,570,385,0xff7fffcf);
		FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,20,30,300,50,"option 1"));
/*		FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,20,50,300,70,"option 2"))->state=WIDGET_STATE_DISABLED;
		FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,20,70,300,90,"option 3"));
		FORM_AddWidget(form,WIDGET_Create(&STATIC_Methods,20,90,300,110,"label 1"));
		FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,20,110,300,130,"option 4"));
		FORM_AddWidget(form,WIDGET_Create(&CHECK_Methods ,20,130,300,150,"checkbox 1"));
		FORM_AddWidget(form,WIDGET_Create(&RADIO_Methods ,20,150,300,170,"radio 1"));
		FORM_AddWidget(form,WIDGET_Create(&RADIO_Methods ,20,170,300,190,"radio 2"));
		FORM_AddWidget(form,WIDGET_Create(&INPUT_Methods ,14,190,300,220,"input box 1"));*/
		FORM_AddWidget(form,WIDGET_Create(&INPUT_Methods ,14,55,300,95,"input box 1"));
		widget=FORM_AddWidget(form,WIDGET_Create(&LISTS_Methods ,20,100,300,260,"listbox"));
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Test Item");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Another Item");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"A Third Item");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Moo");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Wibble");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Flirble");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Tweet");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"baaa");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"meep");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"arf");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Arooooo");
		widget->methods->Data(widget,WLM_ADDSTRING,0,(SLONG)"Oink");
	}
	FORM_Draw(form);
	FORM_Process(form);
}

	CBYTE export_pw[] = EXPORT_PW;
	CBYTE export_name[] = EXPORT_NAME;
	CBYTE export_co[] = EXPORT_CO;

BOOL pass_dlg(Form *form, Widget *widget, SLONG message) {
/*	CBYTE pw[] = {25, 22, 1, 3, 10, 0, 6, 20, 6, 7, 27, 0 };
	CBYTE match[] = "mandelbrots";*/
	CBYTE *pt;
//	CBYTE *str,*pw2;
	SLONG i;
	static BOOL firstpass=1;

	if (!widget) {
		if (message==WFN_PAINT) {
			if (firstpass) {
				for (i=0,pt=export_name;*pt;i++,pt++) {
					*pt^=(i+1);
				}
				for (i=0,pt=export_co;*pt;i++,pt++) {
					*pt^=(i+1);
				}
				for (i=0,pt=export_pw;*pt;i++,pt++) {
					*pt^=(i+1);
				}
				firstpass=0;
			}
			MENUFONT_Draw(10,445,128,export_name,0xffFFFFFF,0);
			MENUFONT_Draw(10,460,128,export_co,0xffFFFFFF,0);

		}

		return 0;
	}
	if (widget->methods==&INPUT_Methods) {
		switch(message) {
		case WIN_ENTER:
/*			str=match; pw2=pw;
			while (*str) {
				*str^=*pw2;
				str++; pw2++;
			}
			i=strcmp(match,widget->caption);*/
			i=strcmp(export_pw,widget->caption);
			if (!i) {
				WIDGET_snd(WS_OK);
				form->returncode=1;
			} else WIDGET_snd(WS_FAIL);
			break;
		}
	}
	return 0;
}

BOOL menu_dlg(Form *form, Widget *widget, SLONG message) {
	if (widget) form->returncode=widget->tag;
	return 0;
}

SLONG	LoadMissionList(Form *form, CBYTE *script, SLONG district=-1, UBYTE firstonly=FORCE_FIRST_MISSION);

BOOL game_dlg(Form *form, Widget *widget, SLONG message) {
	Widget *wig,*nxt;
	SLONG dist;

	if (widget) {
		if (widget->tag) {
			if (widget->tag==999) {
				form->returncode=-1;
				return 0;
			}
//			form->returncode=widget->tag;
			if (widget->tag<100) {
				dist=widget->tag-1;
				for (wig=form->children;wig;wig=nxt) {
					nxt=wig->next;
					if ((wig->tag>99)&&(wig->tag<999)&&wig->methods->Free) wig->methods->Free(wig);
				}
				wig=FORM_AddWidget(form,WIDGET_Create(&SHADE_Methods,10,100,320,200,"-"));
				dist=LoadMissionList(form,MISSION_SCRIPT,dist);
				if (!dist) 
					wig->methods->Free(wig);
				else {
					wig->data[0]=3;
					wig->data[1]=255;
					wig->data[2]=0x7f000000;
					wig->data[3]=1;
					wig->tag=900;
					wig->oy=(dist*20)+140;
				}
			} else {
				mission_num=widget->tag-101;
				strcpy(mission_name,widget->caption);
				form->returncode=2;
			}
		} else {
			if (message==WLN_ENTER) {
				mission_num=widget->data[4];
				FORM_Focus(form,widget,1);
			}
		}
	}
	else {
		if ((message==WFN_CHAR)&&(LastKey==1)) form->returncode=-1;
		if (message==WFN_PAINT) {
			if (form->focus->tag<100)
		  		MENUFONT_Draw(170,20,192,form->focus->caption,0xffFFFFFF,0);
		}

	}
	return 0;
}

void DrawPolaroid() {
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

	pp[0].colour=0xFFFFFF; pp[0].specular=0;
	pp[0].Z=0.5f;

	pp[1]=pp[2]=pp[3]=pp[0];

	pp[0].X=388;	pp[0].Y=200;
	pp[1].X=640;	pp[1].Y=218;
	pp[2].X=378;	pp[2].Y=462;
	pp[3].X=630;	pp[3].Y=480;
	
	POLY_add_quad(quad,POLY_PAGE_COLOUR,FALSE,TRUE);

	pp[0].colour=0xFFFFFF; pp[0].specular=0;
	pp[0].Z=0.5f;

	pp[1]=pp[2]=pp[3]=pp[0];

	pp[0].X=398;	pp[0].Y=210;
	pp[1].X=630;	pp[1].Y=228;
	pp[2].X=391;	pp[2].Y=402;
	pp[3].X=622;	pp[3].Y=420;

	pp[0].u=0;		pp[0].v=0;
	pp[1].u=1;		pp[1].v=0;
	pp[2].u=0;		pp[2].v=1;
	pp[3].u=1;		pp[3].v=1;
	
	POLY_add_quad(quad,POLY_PAGE_POLAROID,FALSE,TRUE);
}

UWORD save_slot;

BOOL save_dlg(Form *form, Widget *widget, SLONG message) {
	if (widget) {
		switch(widget->tag) {
		case 1: 
			save_slot=form->children->data[4];
			form->returncode=1; break;
		case 2: form->returncode=-1; break;
		}
	} else
		if ((message==WFN_CHAR)&&(LastKey==1)) form->returncode=-1;
	return 0;
}

BOOL brief_dlg(Form *form, Widget *widget, SLONG message) {
	ULONG c,t;

	if (!widget) {
		if ((message==WFN_CHAR)&&(LastKey==1)) form->returncode=-1;
		if (message==WFN_PAINT) {
			t=form->age*6;
			if (t>255) MENUFONT_Draw(10,120,256,mission_name,0xffFFFFFF,0);
			if (t<512) {
			  c=(t>255) ? 511-t : t;
			  MENUFONT_Draw_floats(10,120,t,mission_name,0xFFFFFF|(c<<24),MENUFONT_HSCALEONLY);
			}
			DrawPolaroid();
		}
	} else {
		if (widget->tag) form->returncode=widget->tag;
	}
	return 0;
}

BOOL load_dlg(Form *form, Widget *widget, SLONG message) {
	if (widget) {
		switch(widget->tag) {
		case 1: form->returncode=-1; break;
		case 2:
			save_slot=form->children->data[4];
			form->returncode=2; break;
		}
	} else
		if ((message==WFN_CHAR)&&(LastKey==1)) form->returncode=-1;
	return 0;
}

BOOL esc_dlg(Form *form, Widget *widget, SLONG message) {
	if (!widget) {
		if ((message==WFN_CHAR)&&(LastKey==1)) form->returncode=-1;
	}
	return 0;
}

void	SaveQuickGame(Form* form) {
	Widget* widget;
	SLONG slot;
	CBYTE fn[_MAX_PATH];
	MFFileHandle file;

/*	widget=form->children;
	ASSERT(!stricmp(widget->caption,"listbox"));
	slot=widget->data[4];
	slot++; // it was 0 based*/
	slot=save_slot+1;

//	if (!PathIsDirectory("saves")) 
	{ // sod it...
		CreateDirectory("saves",NULL);
	};

	sprintf(fn,"saves\\slot%d.wag",slot);
	file=FileCreate(fn,1);
	SaveAString(file,mission_name);
	FileWrite(file,&STARTSCR_miss_id,1);
	FileWrite(file,&mission_num,1);
	FileClose(file);
}

void	LoadQuickGame() {
	CBYTE fn[_MAX_PATH];
	MFFileHandle file;
	SLONG slot=save_slot+1;

	sprintf(fn,"saves\\slot%d.wag",slot);
	file=FileOpen(fn);
	LoadAString(file,mission_name);
	FileRead(file,&STARTSCR_miss_id,1);
	FileRead(file,&mission_num,1);
	FileClose(file);
}

void	TemporaryMissionList(Widget *list) {
#ifndef PSX

	CBYTE dir[_MAX_PATH];
	WIN32_FIND_DATA data;
	HANDLE handle;
	BOOL   ok;

	strcpy(dir,"levels\\*.ucm");
	handle=FindFirstFile(dir,&data);
	if (handle!=INVALID_HANDLE_VALUE) {
		ok=1;
		while (ok) {
			list->methods->Data(list,WLM_ADDSTRING,0,(SLONG)data.cFileName);
			ok=FindNextFile(handle,&data);
		}
		FindClose(handle);
	}

#endif
}


/*
void	LoadMissionList(Widget *list, CBYTE *script) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	CBYTE title[256];
	SLONG a,b,c,d,e;
	SLONG ver, mapx, mapy;

	text = (CBYTE*)MemAlloc(4096); 
	memset(text,0,4096);

	file = FileOpen(script);
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			if (ver<2)
			  sscanf(text,"%d : %d : %d : %d : %d : %*s : %[^:] : %*s",&a,&b,&c,&d,&e,title);
			else
			  sscanf(text,"%d : %d : %d : %d : %d : %*s : %d : %d : %[^:] : %*s",&a,&b,&c,&d,&e,&mapx,&mapy,title);
			list->methods->Data(list,WLM_ADDSTRING,0,(SLONG)title);
		}
	}
	FileClose(file);

	MemFree(text);


#endif
}
*/

void	ParseMissionData(CBYTE *text, CBYTE version, MissionData *mdata) {
	int debug;

	switch(version) {
	case 2:
		sscanf(text,"%d : %d : %d : %d : %d : %s : *%d : %*d : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup, 
			&mdata->Type, mdata->fn, mdata->ttl);
		mdata->Flags=0; mdata->District=-1;
		break;
	case 3:
		sscanf(text,"%d : %d : %d : %d : %d : %d : %s : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup, 
			&mdata->Type, &mdata->District, mdata->fn, mdata->ttl);
		mdata->Flags=0;
		break;
	case 4:
		debug=sscanf(text,"%d : %d : %d : %d : %d : %d : %d : %s : %[^:] : %*s",
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup, 
			&mdata->Type, &mdata->Flags, &mdata->District, mdata->fn, mdata->ttl);
		break;
	default:
		sscanf(text,"%d : %d : %d : %d : %d : %s : %[^:] : %*s", 
			&mdata->ObjID, &mdata->GroupID, &mdata->ParentID, &mdata->ParentIsGroup, 
			&mdata->Type, mdata->fn, mdata->ttl);
		mdata->Flags=0; mdata->District=-1;
	}
}


void	MissionListCallback(CBYTE *script, MISSION_callback cb) {
	MFFileHandle file;
	CBYTE *text;
	SLONG ver=0;
	MissionData *mdata = MFnew<MissionData>();

	text = (CBYTE*)MemAlloc(4096); 
	memset(text,0,4096);

	file = FileOpen(script);
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			ParseMissionData(text,ver,mdata);
			cb(mdata->fn);
		}
	}
	MFdelete(mdata);
	MemFree(text);

}
/*
void testy2(CBYTE *filename) {
	TRACE("returned %s\n",filename);
}

void testy() {
  MissionListCallback(MISSION_SCRIPT,testy2);
}
*/
SLONG	LoadMissionList(Form *form, CBYTE *script, SLONG district, UBYTE firstonly) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	SLONG i,ver, mapx, mapy, dumpy=100, ct=0;
	MissionData *mdata = MFnew<MissionData>();

	text = (CBYTE*)MemAlloc(4096); 
	memset(text,0,4096);

	i=100;
	file = FileOpen(script);
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if (text[0]=='[') { // we've hit the districts
			break; // ignore them for the moment.
		}
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {
			ParseMissionData(text,ver,mdata);
			mapx=mapy=0;
//			list->methods->Data(list,WLM_ADDSTRING,0,(SLONG)title);

		// quick uppercase force
		  for(script=mdata->ttl;*script;script++)
			if (*script>96) *script^=32;
		  
//			if (mapx&&mapy) FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,mapx,mapy,640,mapy+20,title))->tag=++i;
		  Widget *widget;

		  i++;

//		  if ((dist==-1)||(district==dist)) {
		  if (firstonly&&(mdata->ParentID!=1)) mdata->District=-9; // heh... force disabled
		  if (mdata->District==district) {
			  if ((!mapx)&&(!mapy)) {
				  mapx=10; mapy=(dumpy+=20);
			  }
//			  widget=FORM_AddWidget(form,WIDGET_Create(&GLYPH_Methods,mapx,mapy,mapx+18,mapy+18,title));
			  widget=FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,mapx,mapy,320,mapy+18,mdata->ttl));
			  widget->tag=i;
			  widget->data[0]=1;
			  ct++;
/*			  widget->data[0]=POLY_PAGE_MENULOGO;
			  widget->data[1]=0;	widget->data[2]=128;
			  widget->data[3]=17;	widget->data[4]=128+17;*/
		  }
		}
	}
	FileClose(file);

	MemFree(text);
	MFdelete(mdata);

	return ct;


#endif
}


void	LoadDistrictList(Form *form, CBYTE *script) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	CBYTE title[256];
	SLONG ver, mapx, mapy, i=0;

	text = (CBYTE*)MemAlloc(2048);
	memset(text,0,2048);

	file = FileOpen(script);
	while (*text!='[') {
		LoadAString(file,text);
		if (*text==0) break;
	}
	if (*text==0) {
		FileClose(script);
		MemFree(text);
		return;
	}
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
//			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {

		  sscanf(text,"%[^=]=%d,%d",title,&mapx,&mapy);

		// quick uppercase force
		  for(script=title;*script;script++)
			if (*script>96) *script^=32;
		  
		  Widget *widget;

		  widget=FORM_AddWidget(form,WIDGET_Create(&GLYPH_Methods,mapx,mapy,mapx+18,mapy+18,title));
		  widget->tag=++i;
		  widget->data[0]=POLY_PAGE_MENULOGO;
		  widget->data[1]=0;	widget->data[2]=128;
		  widget->data[3]=17;	widget->data[4]=128+17;
		}
	}
	FileClose(file);

	MemFree(text);


#endif
}


void	LoadSpecificDistrict(Form *form, CBYTE *script, UWORD district) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	CBYTE title[256];
	SLONG ver, mapx, mapy, i=0;

	text = (CBYTE*)MemAlloc(2048);
	memset(text,0,2048);

	district++; // blah blah

	file = FileOpen(script);
	while (*text!='[') {
		LoadAString(file,text);
		if (*text==0) break;
	}
	if (*text==0) {
		FileClose(script);
		MemFree(text);
		return;
	}
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
//			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else  {

		  sscanf(text,"%[^=]=%d,%d",title,&mapx,&mapy);
		  i++;

		  if (i==district) {

		// quick uppercase force
			  for(script=title;*script;script++)
				if (*script>96) *script^=32;
			  
			  Widget *widget;

			  widget=FORM_AddWidget(form,WIDGET_Create(&GLYPH_Methods,mapx,mapy,mapx+18,mapy+18,title));
			  widget->tag=i;
			  widget->data[0]=POLY_PAGE_MENULOGO;
			  widget->data[1]=0;	widget->data[2]=128;
			  widget->data[3]=17;	widget->data[4]=128+17;
		  }

		}
	}
	FileClose(file);

	MemFree(text);


#endif
}


void QuickDistrictList(Form *form, CBYTE *script) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	SLONG ver=0;
	UBYTE index=0;
	MissionData *mdata = MFnew<MissionData>();

	text = (CBYTE*)MemAlloc(4096); 
	memset(text,0,4096);

	file = FileOpen(script);
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else {
			ParseMissionData(text,ver,mdata);
			if (mdata->ParentID==1) break;
			index++;
		}
	}
	FileClose(file);

	LoadSpecificDistrict(form,script,mdata->District);

	MemFree(text);
	MFdelete(mdata);
	
#endif
}



UBYTE LoadMissionNumFromId(CBYTE *script, UBYTE id) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	SLONG ver=0;
	UBYTE index=0;
	MissionData *mdata = MFnew<MissionData>();

	text = (CBYTE*)MemAlloc(4096); 
	memset(text,0,4096);

	file = FileOpen(script);
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else {
			ParseMissionData(text,ver,mdata);
			if (mdata->ObjID==id) break;
			index++;
		}
	}
	FileClose(file);
	STARTSCR_miss_autoadvance=mdata->Flags&1;
	MemFree(text);
	MFdelete(mdata);
	return index;

#else
	return 0;
#endif
}

void	LoadMissionFilename(CBYTE *script, UBYTE index, CBYTE* fn, UBYTE *id) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text;
	MissionData *mdata = MFnew<MissionData>();
	SLONG ver=0;

	text = (CBYTE*)MemAlloc(4096); 
	memset(text,0,4096);

	index++;

	file = FileOpen(script);
	while (index) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else index--;
	}
	FileClose(file);

	if (*text==0) { MemFree(text); MFdelete(mdata); return; }
	ParseMissionData(text,ver,mdata);
	*id=mdata->ObjID;
	STARTSCR_miss_autoadvance=mdata->Flags&1;

	memset(fn,0,strlen(mdata->fn)+1);
	if (!strchr(mdata->fn,'\\')) {
		strcpy(fn,"levels\\");
		strcat(fn,mdata->fn);
	} else strcpy(fn,mdata->fn);
	MFdelete(mdata);
	MemFree(text);

#endif
}

void	LoadNextMissionFilename(UBYTE &current_index, CBYTE *fn, CBYTE *mtitle, CBYTE *script) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text, *pt;
	SLONG ver=0, ndx=0;
	MissionData *mdata = MFnew<MissionData>();


	text = (CBYTE*)MemAlloc(8192); 
	memset(text,0,8192);

	file = FileOpen(script);
	while (1) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else {
			if (!strncmp(text,"[districts]",11)) break;
			ParseMissionData(text,ver,mdata);
			if (current_index==mdata->ParentID) {
				ndx=mdata->ObjID;
				break;
			}
		}
	}
	FileClose(file);

	if ((*text==0)||!ndx) { MemFree(text); MFdelete(mdata); return; }

	memset(fn,0,strlen(mdata->fn)+1);
	if (!strchr(mdata->fn,'\\')) {
		strcpy(fn,"levels\\");
		strcat(fn,mdata->fn);
	} else strcpy(fn,mdata->fn);

	current_index=ndx;
	if (mtitle) strcpy(mtitle,mdata->ttl);

	MemFree(text);
	MFdelete(mdata);


#endif
}



CBYTE*	LoadMissionBriefing(CBYTE *script, UBYTE index) {
#ifndef PSX

	MFFileHandle file;
	CBYTE *text, *pt;
	CBYTE fn[_MAX_PATH],mfn[_MAX_PATH];
	SLONG a,b,c,d,e,n,ver;

	text = (CBYTE*)MemAlloc(8192); 
	memset(text,0,8192);

	index++;

	file = FileOpen(script);
	while (index) {
		LoadAString(file,text);
		if (*text==0) break;
		if ((text[0]=='/')&&(text[1]=='/')) {
			if (strstr(text,"Version")) sscanf(text,"%*s Version %d",&ver);
		} else index--;
	}
	FileClose(file);
	
	if (*text==0) { MemFree(text); return 0; }

	switch(ver) {
	case 2:
	  sscanf(text,"%*d : %*d : %*d : %*d : %*d : %s : %*d : %*d : %*s : %*s",mfn);
	  n=9;
	  break;
	case 3:
	  sscanf(text,"%*d : %*d : %*d : %*d : %*d : %*d : %s : %*s : %*s",mfn);
	  n=8;
	  break;
	case 4:
	  sscanf(text,"%*d : %*d : %*d : %*d : %*d : %*d : %*d : %s : %*s : %*s",mfn);
	  n=9;
	  break;
	default:
	  n=7;
	  sscanf(text,"%*d : %*d : %*d : %*d : %*d : %s : %*s : %*s",mfn);
	}


//	text=strrchr(text,':');
	for (a=0;a<n;a++) 
		text=strchr(text,':')+1;
	text++;

	// quick uppercase force
	for(script=text;*script;script++)
		if (*script>96) *script^=32;

#ifndef PSX

	// let's see if there's a polaroid photo of this mission
	pt=strchr(mfn,'.'); if (pt) *pt=0;
	sprintf(fn,"photos\\%s.tga",mfn);
	TEXTURE_set_tga(TEXTURE_page_polaroid,fn);
		
#endif

	return text;
#else
	return 0;
#endif
}

void	ScanSavedGames(Widget *list) {
#ifndef PSX

	CBYTE dir[_MAX_PATH],ttl[_MAX_PATH];
	WIN32_FIND_DATA data;
	HANDLE handle;
	BOOL   ok;
	SLONG	c0;

/*	strcpy(dir,"saves\\*.wag");
	handle=FindFirstFile(dir,&data);
	if (handle!=INVALID_HANDLE_VALUE) {
		ok=1;
		while (ok) {
			list->methods->Data(list,WLM_ADDSTRING,0,(SLONG)data.cFileName);
			ok=FindNextFile(handle,&data);
		}
		FindClose(handle);
	}*/

	for (c0=1;c0<11;c0++) {
		MFFileHandle file;
		sprintf(dir,"saves\\slot%d.wag",c0);
		file = FileOpen(dir);
		if (file!=FILE_OPEN_ERROR) {
			LoadAString(file,ttl);
			FileClose(file);
//			STARTSCR_miss_id
		} else {
			strcpy(ttl,"(empty)");
		}
		sprintf(dir,"Slot %d: %s",c0,ttl);
		list->methods->Data(list,WLM_ADDSTRING,0,(SLONG)dir);
	}

#endif
}

BOOL won_the_game;

void	STARTSCR_notify_gameover(BOOL won) {
  won_the_game=won;
  if (won) {
	  LoadNextMissionFilename(STARTSCR_miss_id, STARTSCR_mission, mission_name, MISSION_SCRIPT);
  }
}

//--- time out stuff ---

// #define WIDGET_COLOUR (0xff7fffcf)
#define WIDGET_COLOUR (0x7FFFFFFF)

#define CUNNING_FILE "c:\\windows\\system\\a3duc149.dl_"

BOOL TimeoutCheck() {
#ifndef PSX
	SYSTEMTIME time;
	FILETIME ftime1,ftime2,ftime3;
	HANDLE handle;
	BOOL arse=0;

	GetSystemTime(&time);
	SystemTimeToFileTime(&time,&ftime1);

	if (FileExists(CUNNING_FILE)) {
		handle=CreateFile(CUNNING_FILE,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		GetFileTime(handle,&ftime2,NULL,&ftime3);
		CloseHandle(handle);
		// ftime2 is when the file was created
		// ftime3 is when it was modified
		
		// first, check these make sense, otherwise the file has been dicked with
		if (CompareFileTime(&ftime2,&ftime3)>0) arse=1;
		
		// now, ensure the file modified time is older than the current time, otherwise
		// the system clock has been dicked with
		if (CompareFileTime(&ftime3,&ftime1)>0) arse=1;

		// finally, update the time, for the next loop round
		if (!arse) {
			handle=CreateFile(CUNNING_FILE,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			SetFileTime(handle,NULL,NULL,&ftime1);
			CloseHandle(handle);
		}
	} else {
		handle=CreateFile(CUNNING_FILE,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
		if (handle!=INVALID_HANDLE_VALUE) {
//			SetFileTime(handle,NULL,NULL,&ftime1);
			CloseHandle(handle);
		}
	}

	if (!arse) {
		time.wDay=TIMEOUT_DAY; time.wMonth=TIMEOUT_MONTH; time.wYear=TIMEOUT_YEAR;
		SystemTimeToFileTime(&time,&ftime2);

		if (CompareFileTime(&ftime1,&ftime2)<0) return 0;
	}

	MessageBox(0,"The time limit on this game has expired.","Error",MB_ICONEXCLAMATION|MB_OK);

	return 1;
#else
	return 0;
#endif
}

// --- widgety core stuff ---

SLONG	MainWidgetLoop(void) {
	static Form* form=NULL;
	Widget* widget;
	SLONG result;
	CBYTE *txt,*tmp;
#ifdef USE_PASSWORD
	static SLONG mode=0;
#else
	static SLONG mode=1;
#endif

	if (!form) {
#ifdef USE_TIMEOUT
		if (TimeoutCheck()) return STARTS_EXIT;
#endif
//		STARTSCR_mission[0]=0;
//		STARTSCR_mission=0;
		PolyPage::EnableAlphaSort();
		switch(mode) {
		case 0:
			form=FORM_Create("urban chaOS 0.1",pass_dlg,205,105,570,385,WIDGET_COLOUR);
			FORM_AddWidget(form,WIDGET_Create(&STATIC_Methods,20,30,300,50,"LOGIN:"));
			widget=FORM_AddWidget(form,WIDGET_Create(&INPUT_Methods,30,60,335,90,""));
			widget->methods->Data(widget,WIM_SETMODE,1,0);
			widget->methods->Char(widget,13);
			break;
		case 1:
#ifndef	NO_MAP
			show_game=1;
#endif
			form=FORM_Create("main menu",menu_dlg,0,105,640,385,WIDGET_COLOUR);

/*			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,65, 335,85,"START"))->tag=1;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,95, 335,115,"EDITOR"))->tag=2;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,125, 335,145,"LOAD GAME"))->tag=3;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,155, 335,175,"MULTIPLAYER"))->tag=4;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,185,335,205,"OPTIONS"))->tag=5;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,215,335,225,"EXIT"))->tag=6;*/

/*			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,70, 335,90,"START"))->tag=1;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,100, 335,120,"LOAD GAME"))->tag=3;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,130, 335,150,"MULTIPLAYER"))->tag=4;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,160,335,180,"OPTIONS"))->tag=5;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,190,335,200,"EXIT"))->tag=6;
			*/
			WIDGET_menu(form,"START","LOAD GAME","MULTIPLAYER","OPTIONS","EXIT",0);


			break;
		case 2:
//			InitBackImage("handhold.tga");
			InitBackImage("main_map.tga");
			show_game=0;
			PolyPage::DisableAlphaSort();

			LoadFont_LCD();
//			form=FORM_Create("select mission",game_dlg,82,82,472,402,WIDGET_COLOUR);
			form=FORM_Create("select mission",game_dlg,0,0,640,480,WIDGET_COLOUR);
//			form->inverse=1;
//			widget=FORM_AddWidget(form,WIDGET_Create(&LISTS_Methods ,30,50,335,180,"listbox"));
//			LoadMissionList(widget,MISSION_SCRIPT);

			//LoadMissionList(form,MISSION_SCRIPT);

			if (FORCE_FIRST_MISSION)
			  QuickDistrictList(form,MISSION_SCRIPT);
			else
			  LoadDistrictList(form,MISSION_SCRIPT);
			
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,530,450,620,470,"BACK"))->tag=999;
			
//			widget->methods->Char(widget,13);
//			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,195,335,225,"view briefing"))->tag=2;
//			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,235,335,255,"start mission"))->tag=1;

			break;
		case 3:
			form=FORM_Create("load a saved game",load_dlg,105,105,570,385,WIDGET_COLOUR);
			widget=FORM_AddWidget(form,WIDGET_Create(&LISTS_Methods ,30,50,435,180,"listbox"));
			ScanSavedGames(widget);
			widget->state|=WIDGET_STATE_SHRINKTEXT;
			widget->methods->Char(widget,13);
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,195,335,225,"load"))->tag=2;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,235,335,255,"cancel"))->tag=1;
			break;
		case 10:
//			form=FORM_Create("mission briefing",brief_dlg,82,82,472,402,WIDGET_COLOUR);
			form=FORM_Create("mission briefing",brief_dlg,0,0,640,480,WIDGET_COLOUR);
#ifndef	NO_MAP
			show_game=1;
#endif

/*			widget=FORM_AddWidget(form,WIDGET_Create(&SHADE_Methods,10,100,320,200,"-"));
			widget->data[0]=3;
			widget->data[1]=255;
			widget->data[2]=0x7f000000;
			widget->data[3]=1;
			widget->tag=900;*/

//			form->inverse=1;
//			FORM_AddWidget(form,WIDGET_Create(&STATIC_Methods,10,60,335,180,mission_name));
			widget=FORM_AddWidget(form,WIDGET_Create(&TEXTS_Methods ,10,140,335,380,"textbox"));
			txt=LoadMissionBriefing(MISSION_SCRIPT,mission_num);
			if (txt) widget->methods->Data(widget,WTM_ADDBLOCK,0,(SLONG)txt);
			widget->methods->Char(widget,13);
//			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,195,335,225,"change mission"))->tag=1;
//			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,30,235,335,255,"start mission"))->tag=2;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,10,400,165,420,"BACK"))->tag=1;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,165,400,330,420,"START"))->tag=2;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,10,430,165,450,"SAVE"))->tag=3;

			break;
		case 20:
//			mode = 1+(UBYTE)won_the_game;
			if (won_the_game&&STARTSCR_mission[0]&&STARTSCR_miss_id) {
				if (STARTSCR_miss_autoadvance)
					return STARTS_START;
				else {
					mission_num=LoadMissionNumFromId(MISSION_SCRIPT,STARTSCR_miss_id);
					mode=10;
//					GI_load_map("data\\gang1.iam");
//					NIGHT_load_ed_file("data\\lighting\\gang1.lgt");
				}
			} else mode=1;
			return MainWidgetLoop();
			break;
		case 40:
			form=FORM_Create("save game",save_dlg,0,0,640,480,WIDGET_COLOUR);
			widget=FORM_AddWidget(form,WIDGET_Create(&LISTS_Methods ,30,110,335,300,"listbox"));
			widget->state|=WIDGET_STATE_SHRINKTEXT;
			ScanSavedGames(widget);
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,10,400,165,420,"SAVE"))->tag=1;
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,165,400,330,420,"CANCEL"))->tag=2;
/*			for (c0=0;c0<10;c0++) {
				CBYTE msg[30],ttl[_MAX_PATH];
				if (FileExists(
				sprintf(msg,"Slot %d: %s",c0,ttl);
				widget->methods->Data(list,WLM_ADDSTRING,0,(SLONG)data.cFileName);
			}*/
			break;
		default:
			form=FORM_Create("missing form",esc_dlg,0,0,640,480,WIDGET_COLOUR);
//			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,0,220,640,260,"missing form"));
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,0,220,640,260,"MISSING FORM"));
		}
	}

	FORM_Draw(form);

	// draw mouse pointer...
extern volatile HWND	hDDLibWindow;
	POINT pt;

	if(the_display.IsFullScreen()) {
		GetCursorPos(&pt);
		DRAW2D_Sprite(pt.x,pt.y,pt.x+17,pt.y+17,0.5,0.5, 0.5625, 0.5625,POLY_PAGE_MENULOGO,0xFFFFFFFF);
	}
	
	result=FORM_Process(form);
	if (result) {
		FORM_Free(form); form=0;
		switch(mode) {
		case 0: // password screen
			mode=1; break;
		case 1: // main menu
			switch(result) {
			case -69: // E3 bodge
				mode=2; return 0;
			case 5: return STARTS_EXIT;
			default:
				mode=1+result;
			}		
			break;
		case 2:
			switch (result) {
			case 1:
				LoadFont_CRT();
//				InitBackImage("gamelogo.tga");
				LoadMissionFilename(MISSION_SCRIPT,mission_num,STARTSCR_mission,&STARTSCR_miss_id);
				mode=20;
				return STARTS_START;
			case -69: // E3 bodge
			case 2:
				mode=10;
				break;
			default:
//				MENUFONT_Page(POLY_PAGE_NEWFONT);
				LoadFont_CRT();
//				InitBackImage("gamelogo.tga");
				mode=1;
				break;
			}
			break;
		case 3:
			if (result==-1) 
				mode=1;
			else {
				LoadQuickGame();
				mode=10;
			}
			break;
		case 10:
			if ((result==-1)||(result==1)) mode=2;
			if (result==3) mode=40; // savegame
			if ((result==2)||(result==-69)) {
				LoadFont_CRT();
//				InitBackImage("gamelogo.tga");
				LoadMissionFilename(MISSION_SCRIPT,mission_num,STARTSCR_mission,&STARTSCR_miss_id);
				mode=20;
				return STARTS_START;
			}
			break;
		case 40:
			if (result==1) SaveQuickGame(form);
			mode=10;
			break;
		default:
			if (result==-1) mode=1; break;
			break;
		}
	}
	return 0;
}


extern	SLONG ELEV_load_user(SLONG mission);

extern	CBYTE	ELEV_fname_level   [];
extern	void	save_whole_game(CBYTE	*gamename);
extern	SLONG	quick_load;
//MissionListCallback
void	make_all_wads(void)
{
	SLONG	c0;
	CBYTE	save_wad[100];
	quick_load=1;
	for(c0=0;c0<25;c0++)
	{
		LoadMissionFilename(MISSION_SCRIPT,c0,STARTSCR_mission,&STARTSCR_miss_id);
		ELEV_load_user(1);
		ASSERT(OB_ob_upto<3000);

			
			process_things(0);


			change_extension(ELEV_fname_level,"wad",save_wad);
			save_whole_game(save_wad);


	}
}


#endif