/*
** File: Wadmenu.cpp
**
** Temporary file that generates a menu of Wad files that can be loaded into the PSX version
** of Urban Chaos
**
** Ha! I Lied, this is now becoming the nexus of a new menu system for the entire game,
** so far it supports Multiple layer menus, slider bars, and function calling menu items
** the original menu will of course need rewriting to display in the correct format 
** (overlaid onto a map of Union City and surrounding areas). Done.
**
** Now includes MEMORY CARD save and load features, options menus and all the gubbins
** that go to make up the frontend, and when combined with MCARD, MDEC and WADPART
** goes on to form the entire frontend for the game.
**
** Now supports auto FMV on completion of levels that require it. Of course there aren't
** any levels that support it, although we have 3 cutscene FMVs that need sorting into
** the levels. Done - forces the FMV via the very hacky use of if (wad_level==X) which
** of course doesn't make for good design, it would have been better to stick the FMV in
** the level directory as INTRO.STR and OUTRO.STR which could have been checked for when
** loading the level and played accordingly, oh well next time hey!
**
** Also features code generation code for all those players lucky enough to beat our best
** times on any of the levels.
**
*/
#include <ctype.h>

#include "game.h"
#include "psxeng.h"
#include "ctrller.h"

#include "libpad.h"
#include "libcd.h"
#include "libmcrd.h"

#include "c:\fallen\psxlib\headers\mfx.h"

#include "c:\fallen\headers\sound_id.h"

#include "c:\fallen\headers\interfac.h"	  

#include "wadstr.h" 

#include "mcard.h"

#include "xlat_str.h"

#include "wadpart.h"

#include "music.h"

#ifdef VERSION_PAL
#ifndef	MIKE
#include "libcrypt.h"
#endif
#endif

#ifdef VERSION_KANJI
#include "kanji.h"
#endif

typedef struct 
{
	UWORD	label;
	UWORD	tpage;
	UWORD	clut;
	UWORD	width;
	UWORD	height;
	UBYTE	u,v;
} W_Image;

#include "fendi.h"

extern void AENG_screen_shot(SLONG width);

typedef struct WadMenuItem {
	int name;
	int type;
	union {
		void (*func_void)(struct WadMenuItem *item);
		char *(*func_str)(TIM_IMAGE *);
		struct WadMenuItem *menu;
		int	*value;
	} ptr;
	int info;
	int last;
} WadMenuItem;

typedef struct {
	char *fname;
	int	 frames;
} WadMenuFMV;

WadMenuFMV Wadmenu_FMV[]={
	{"\\STR\\EIDOS.STR;1",371},
	{"\\STR\\MUCKY.STR;1",448},
	{"\\STR\\INTRO.STR;1",4898},
	{"\\STR\\PSXCUT1.STR;1",627},
	{"\\STR\\PSXCUT2.STR;1",755},
	{"\\STR\\PSXCUT3.STR;1",679}
};

extern SBYTE f_width[];
extern SBYTE f_descend[];
extern SLONG stat_game_time;

UBYTE Wadmenu_Backdrop[]={0,0,0,0,0,1,0,0,1,0,3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,3,3,3,3};

typedef struct {
	SLONG	time;
	char	*player;
} ParTimeData;

#define TIME(h,m,s) (((h)*3600)+((m)*60)+(s))

ParTimeData Wadmenu_ParTime[]={
	{TIME(0,0,0),"No One"},
	{TIME(0,0,0),"No One"},
	{TIME(0,0,0),"No One"},
	{TIME(0,0,0),"No One"},
	{TIME(0,0,0),"No One"},
	{TIME(0,0,55),"Mark Rose"},
	{TIME(0,0,0),"No One"},
	{TIME(0,0,0),"No One"},
	{TIME(0,1,28),"Mark Rose"},
	{TIME(0,0,0),"No One"},
	{TIME(0,4,23),"Mike B."},
	{TIME(0,1,41),"Fin"},
	{TIME(0,4,0),"Mike B."},
	{TIME(0,1,18),"Mike B."},
	{TIME(0,0,37),"Disky"},
	{TIME(0,2,51),"Fin"},
	{TIME(0,1,5),"Mark A."},
	{TIME(0,4,13),"Mike B."},
	{TIME(0,2,56),"Mark Rose"},
	{TIME(0,3,42),"Marie"},
	{TIME(0,8,26),"Mark Rose"},
	{TIME(0,4,27),"Mark Rose"},
	{TIME(0,4,48),"Marie"},
	{TIME(0,5,2),"Marie"},
	{TIME(0,8,44),"Marie"},
	{TIME(0,6,32),"Mike B."},
	{TIME(0,7,18),"Fin"},
	{TIME(0,4,30),"Fin"},
	{TIME(0,0,46),"Mark A."},
	{TIME(0,1,19),"Mark Rose"},
	{TIME(0,2,0),"Justin"},
	{TIME(0,0,0),"No One"},
	{TIME(0,1,23),"Disky"},
	{TIME(0,5,48),"Dave"},
	{TIME(0,2,13),"Marie"},
	{TIME(0,0,0),"No One"}
};

extern SLONG Wadmenu_MuckyTime;
extern CBYTE Wadmenu_MuckyName[];

#define WADMENU_FMV_EIDOS	0
#define WADMENU_FMV_MUCKY	1
#define WADMENU_FMV_INTRO	2
#define WADMENU_FMV_CUT1	3
#define WADMENU_FMV_CUT2	4
#define WADMENU_FMV_CUT3	5

extern void Wadmenu_DoParticleElement(SLONG mode);
extern void Wadmenu_ParticleSync(TIM_IMAGE *tim);
extern void Wadmenu_Image(SLONG id,SLONG x,SLONG y,SLONG scale,SLONG colour);
extern SLONG Wadmenu_GetDistrict(TIM_IMAGE *tim);
extern void Wadmenu_Backboard(SLONG id,SLONG x,SLONG y);
extern void Wadmenu_NewMenu(WadMenuItem *menu,SLONG selected);
extern void Wadmenu_BackMenu(WadMenuItem *menu,SLONG selected);
extern void Wadmenu_ParticleEffect(SLONG flag,SLONG data1,SLONG data2);
extern void Wadmenu_DrawIconicStuff(SLONG flag);
extern void Wadmenu_InitStats();
extern void Wadmenu_AutoLoadInfo();
extern void Wadmenu_Code(TIM_IMAGE *tim);

#ifndef VERSION_DEMO
#define WADMENU_MUSIC_MODE	MUSIC_MODE_BRIEFING
#else
#define WADMENU_MUSIC_MODE	MUSIC_MODE_CRAWLING
#endif

#define WADMENU_ITEM_X		256
#define WADMENU_ITEM_Y		80
#define WADMENU_MCARD_X		256
#define WADMENU_MCARD_Y1	64
#define WADMENU_MCARD_Y2	128
#define WADMENU_MCARD_Y3	160
#define WADMENU_TITLE_X		288
#define WADMENU_TITLE_Y		32
#define WADMENU_KEYS_X		496
#define WADMENU_KEYS_Y		212
#define WADMENU_BRIEF_X		32
#define WADMENU_BRIEF_Y		56
#define WADMENU_ARROW_X		(32+328)
#define WADMENU_DIST_X		16
#define WADMENU_DIST_Y		200

#define WADMENU_BUTTON_X	16
#define WADMENU_BUTTON_Y	16

#define WADMENU_YORN_X		256
#define WADMENU_YORN_Y1		64
#define WADMENU_YORN_Y2		128

#define WADMENU_EFFECT_MAP		1
#define WADMENU_EFFECT_BRIEF	2
#define WADMENU_EFFECT_MENU		4
#define WADMENU_EFFECT_PAD		8
#define WADMENU_EFFECT_TOBLACK	16
#define WADMENU_EFFECT_BADCOP	32

#define WADMENU_EFFECT_IN	128
#define WADMENU_EFFECT_OUT	256


#define WADMENU_EFFECT_LEN	32

#define WADMENU_MAP_IN		(WADMENU_EFFECT_MAP|WADMENU_EFFECT_IN)
#define WADMENU_MAP_OUT		(WADMENU_EFFECT_MAP|WADMENU_EFFECT_OUT)

#define WADMENU_BRIEF_IN	(WADMENU_EFFECT_BRIEF|WADMENU_EFFECT_IN)
#define WADMENU_BRIEF_OUT	(WADMENU_EFFECT_BRIEF|WADMENU_EFFECT_OUT)

#define WADMENU_MENU_IN		(WADMENU_EFFECT_MENU|WADMENU_EFFECT_IN)
#define WADMENU_MENU_OUT	(WADMENU_EFFECT_MENU|WADMENU_EFFECT_OUT)

#define WADMENU_PAD_IN		(WADMENU_EFFECT_PAD|WADMENU_EFFECT_IN)
#define WADMENU_PAD_OUT		(WADMENU_EFFECT_PAD|WADMENU_EFFECT_OUT)

#define WADMENU_BADCOP_IN	(WADMENU_EFFECT_BADCOP|WADMENU_EFFECT_IN)
#define WADMENU_BADCOP_OUT	(WADMENU_EFFECT_BADCOP|WADMENU_EFFECT_OUT)

#define WADMENU_BACKDROP	(Wadmenu_Backdrop[wad_level])

#define WADMENU_BUTTON		(WADMENU_BUTTON_GREEN+WADMENU_BACKDROP)

#define WADMENU_MENU_SCREEN 0
#define WADMENU_MAP_SCREEN	1
#define WADMENU_MISSION_SCREEN 2
#define WADMENU_PAD_SCREEN 4
#define WADMENU_BRIEF_SCREEN 8
#define WADMENU_BADCOP_SCREEN 16

typedef struct {
	char name[32];
	SLONG opened;
	UWORD x;
	UWORD y;
} W_District;

W_District *Wadmenu_district;

SLONG Wadmenu_districts;
SLONG Wadmenu_sel_district;
SLONG Wadmenu_Cheat=0;

UBYTE MCARD_Header[]={
	0x53, 0x43,
	0x11,
	0x01,

	0x82, 0x74, 0x82, 0x92, 0x82, 0x82, 0x82, 0x81, 0x82, 0x8e, 0x81, 0x40, 0x82, 0x62, 0x82, 0x88,
	0x82, 0x81, 0x82, 0x8f, 0x82, 0x93, 0x81, 0x40, 0x82, 0x72, 0x82, 0x81, 0x82, 0x96, 0x82, 0x85,
	0x81, 0x40, 0x82, 0x66, 0x82, 0x81, 0x82, 0x8d, 0x82, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

UWORD MCARD_Clut[]={
 	0x7fff, 0x0c63, 0x1ce7, 0x0846, 0x2552, 0x1d0f, 0x18ed, 0x10aa,
	0x2994, 0x2151, 0x0c88, 0x0023, 0x00ee, 0x0ca2, 0x24a4, 0x8000
};

ULONG MCARD_Icon[]={
	0x94457a3b, 0xff7dd594, 0x945abbbf, 0xffbac176, 0x597fbbbf, 0xfffacb17, 0x547bffff, 0xfffffbbb,
	0x79732f1f, 0xffb132f1, 0x7457a33f, 0xfffbbbb3, 0x7445456a, 0xb76457b3, 0x64848857, 0x34488833,
	0x48844887, 0x35484437, 0x75455447, 0xb3755631, 0xf3a64451, 0xf33a773f, 0x76544563, 0xfb33aa33,
	0x5448457b, 0xbfb33336, 0x1a7777ab, 0xbbbbbbb3, 0x4888663e, 0xbbbbbb36, 0xa7777a1e, 0xbbbbb3fb
};

extern void	draw_centre_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id,SLONG flag);
extern void	draw_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id);
extern void	PCReadFile(CBYTE *name,UBYTE *addr,ULONG len);

extern int sfx_volume;
extern int music_volume;
extern int speech_volume;
extern int vid_bright;
extern int sound_mode;
extern int pad_config;
extern int vibra_mode;
extern int Wadmenu_Levelwon;
extern int screen_x;
extern int screen_y;
extern UBYTE Wadmenu_Video;
extern CBYTE* Wadmenu_CivMess;
// All the stats we can update during the game.
extern SWORD Wadmenu_Current_Con;
extern SWORD Wadmenu_Current_Ref;
extern SWORD Wadmenu_Current_Sta;
extern SWORD Wadmenu_Current_Str;
extern SWORD Wadmenu_Citations;

extern UBYTE Wadmenu_Display;

#define MAX_BRIEFING	48

typedef struct {
	short	objectid;
	short	groupid;
	char	parent;
	char	pig;
	char	type;
	char	district;
	char	mission[24];
	char	title[32];
	char	brief[20][48];
} BriefEntry;

BriefEntry *brief;

char *brief_mem;

int wad_used;

extern int wad_level;

extern int sel_language;

//char *level_ext[]={"nad","fad","sad","iad","nad","nad"};

extern ControllerPacket	PAD_Input1,PAD_Input2;
extern void AENG_flip2(ULONG *back_image);

int Wadmenu_texthigh=0x007f7f7f;
int Wadmenu_texttrans=0x003f3f3f;
int Wadmenu_textlow=0x001f1f1f;

#define MENU_TEXTHIGH	Wadmenu_texthigh
#define MENU_TEXTTRANS	Wadmenu_texttrans
#define MENU_TEXTLOW	Wadmenu_textlow

#define WADMENU_TEXT_NORMAL	0
#define WADMENU_TEXT_BIG	1
#define WADMENU_TEXT_CENTRE 2
#define WADMENU_TEXT_RIGHT	4
#define WADMENU_TEXT_SHADOW 8
#define WADMENU_TEXT_BOXOUT 16

#define WADMENU_TEXT_TITLE	(WADMENU_TEXT_BIG|WADMENU_TEXT_CENTRE)
#define WADMENU_TEXT_ITEM	(WADMENU_TEXT_BIG|WADMENU_TEXT_CENTRE)
#define WADMENU_TEXT_KEYS	(WADMENU_TEXT_RIGHT)
#define WADMENU_TEXT_MCARD	(WADMENU_TEXT_CENTRE)
#define WADMENU_TEXT_ICON	(WADMENU_TEXT_NORMAL)
#define WADMENU_TEXT_MISSION (WADMENU_TEXT_CENTRE|WADMENU_TEXT_BIG)

#ifdef VERSION_ENGLISH
#define WADMENU_FILETEMPLATE		"levels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_ITEM
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_ITEM
#endif

#ifdef VERSION_FRENCH
#define WADMENU_FILETEMPLATE		"levels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_CENTRE
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_CENTRE
#endif

#ifdef VERSION_GERMAN
#define WADMENU_FILETEMPLATE		"levels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_CENTRE
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_CENTRE
#endif

#ifdef VERSION_USA
#define WADMENU_FILETEMPLATE		"levels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_ITEM
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_ITEM
#endif

#ifdef VERSION_JAPAN
#define WADMENU_FILETEMPLATE		"levels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_ITEM
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_ITEM
#endif

#ifdef VERSION_KOREA
#define WADMENU_FILETEMPLATE		"levels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_ITEM
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_ITEM
#endif

#ifdef VERSION_ITALIAN
#define WADMENU_FILETEMPLATE		"ilevels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_CENTRE
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_CENTRE
#endif

#ifdef VERSION_SPANISH
#define WADMENU_FILETEMPLATE		"slevels%d\\level%02d\\level.nad"
#define WADMENU_TEXT_MCARD2 WADMENU_TEXT_CENTRE
#define WADMENU_TEXT_MCARD3 WADMENU_TEXT_CENTRE
#endif

#ifdef VERSION_DEMO
#define WADMENU_FILETEMPLATE		"urban\\level%02d\\level.nad"
#endif

extern SLONG	text_width(CBYTE *message,SLONG font_id,SLONG *char_count);
extern void DRAW2D_Box_Page(SLONG x,SLONG y,SLONG ox,SLONG oy,SLONG rgb);

extern char level_done[48];

extern UBYTE Wadmenu_PadType;

void Wadmenu_ReadWads(char *fname)
{
	if (brief_mem)
		MemFree((void *)brief_mem);
	
	brief_mem=(char *)MemAlloc((sizeof(BriefEntry)*MAX_BRIEFING)+4);

	PCReadFile(fname,(UBYTE *)brief_mem,(sizeof(BriefEntry)*MAX_BRIEFING)+4);

	wad_used=*(SLONG *)brief_mem;
	brief=(BriefEntry*)&brief_mem[4];
}

void Wadmenu_ClearWads(void)
{
	if (brief_mem)
	{
		MemFree((void *)brief_mem);
		brief_mem=0;
	}
}

extern SLONG MDEC_Play(char *fname,int len,int lang);

#define Wadmenu_PlayFMV(fmv) MDEC_Play(Wadmenu_FMV[fmv].fname,Wadmenu_FMV[fmv].frames,fmv);

#define MAX_IN_ONE		8

char	Wadmenu_level[MAX_IN_ONE];
SLONG	Wadmenu_levels_used;

char	Wadmenu_group[48];

void Wadmenu_BuildAvailable()
{
	SLONG i;

	Wadmenu_levels_used=0;

	for(i=0;i<48;i++)
	{
		Wadmenu_group[i]=1;
	}

	for(i=0;i<wad_used;i++)
	{
		Wadmenu_group[brief[i].groupid]&=level_done[i+1];
	}

	for(i=0;i<wad_used;i++)
	{
		if ((brief[i].type==0))
			if (brief[i].pig)
			{
				if (Wadmenu_group[brief[i].parent])
					level_done[i+1]=1;
			}
			else
			{
				if (level_done[brief[i].parent-1])
					level_done[i+1]=1;
			}
	}

	for(i=0;i<wad_used;i++)
	{
		if (!level_done[i+1])
		{
			if (brief[i].pig)
			{
				if (Wadmenu_group[brief[i].parent])
					Wadmenu_level[Wadmenu_levels_used++]=i;
			} 
			else
			{
				if (level_done[brief[i].parent-1])
					Wadmenu_level[Wadmenu_levels_used++]=i;
			}
		}
	}
}

char *Wadmenu_char_table="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$*-()[]\\/?";
char *Wadmenu_char_lookup=
	"        "
	"        "
	"        "
	"        "
	"        "
	"        "
	"        "
	"        "
	"AAAAAA C"
	"EEEEIIII"
	"DNOOOOOX"
	"OUUUUY  "
	"AAAAAA C"
	"EEEEIIII"
	" NOOOOO "
	"OUUUUY Y";

char Wadmenu_char_width[]={
	20,20,18,18,15,15,18,16,
	14,16,20,15,22,20,20,20,
	20,20,20,20,20,20,22,20,
	20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20};

UBYTE wadmenu_f_width[]={
	5 ,2 ,5 ,11,8 ,12,10,2,
	5 ,5 ,8 ,8 ,2 ,8, 3 ,5,
	8 ,3 ,8 ,8 ,8 ,8 ,8 ,8,
	8 ,8 ,2 ,2 ,7 ,6 ,7 ,7,
	11,8 ,8 ,8 ,8 ,6 ,7 ,10,
	8 ,2 ,6 ,7 ,7 ,11,8 ,11,
	8 ,11,9 ,8 ,8 ,8 ,8 ,10,
	8 ,8 ,6 ,4 ,5 ,4 ,6 ,11,
	3 ,7 ,6 ,6 ,6 ,7 ,6 ,6 ,
	6 ,2 ,3 ,6 ,2 ,10,6 ,7 ,
	6 ,6 ,6 ,6 ,5 ,6 ,7 ,10,
	8 ,7 ,6 ,5 ,2 ,5 ,0 ,10,
	0 ,12,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,0 ,0 ,0 ,0 ,11,10,9 ,
	11,0 ,2 ,0 ,0 ,0 ,0 ,0 ,
	0 ,0 ,0 ,0 ,0 ,11,12,0 ,
	12,2 ,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,12,0 ,0 ,0 ,0 ,12,0 ,
	0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,0 ,0 ,0 ,0 ,0 ,0 ,8 ,
	8 ,8 ,8 ,8 ,8 ,8 ,11,9 ,
	6 ,6 ,6 ,6 ,2 ,2 ,2 ,2 ,
	8 ,8 ,11,11,11,11,11,9 ,
	11,8 ,8 ,8 ,8 ,8 ,7 ,10,
	6 ,6 ,6 ,6 ,6 ,6 ,12,7 ,
	6 ,6 ,6 ,6 ,3 ,3 ,4 ,5 ,
	8 ,6 ,6 ,6 ,6 ,6 ,6 ,7 ,
	6 ,6 ,6 ,6 ,6 ,7 ,6 ,7

};

SLONG Wadmenu_text_width2(CBYTE *message)
{
	CBYTE *p=message;
	SLONG width=0;

	while(*p)
	{
		if (*p>31)
			width+=wadmenu_f_width[*p-32]+1;
		p++;
	}
	return width;
}

void Wadmenu_draw_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id)
{
#ifndef VERSION_KANJI
	SPRT *p;
	DR_TPAGE *tp;
	UBYTE* m=(UBYTE*)message;
	SLONG x0=x,y0=y;
	SLONG c;

	ALLOCPRIM(tp,DR_TPAGE);
	setDrawTPage(tp,0,1,getTPage(0,0,896,256));
	while (*m)
	{
		switch(*m)
		{
		case '\t':
			x0=(x0+64)&0xfc0;
			if (x0>512)
			{
				x0=x;
				y0+=10;
			}
			break;
		case 10:
		case 13:
			x0=x;
			y0+=10;
			break;
		case 32:
			x0+=6;
			break;
		default:
			c=*m-32;

			ALLOCPRIM(p,SPRT);
			setSprt(p);
			setXY0(p,x0,y0+f_descend[c]);
			p->w=wadmenu_f_width[c];
			p->h=8;

			setUV0(p,(c&7)*12,(c&0xf8));
			if (font_id&&((c<96)||(c>111)))
			{
				if (font_id&0xff000000)
					setSemiTrans(p,1);
				setRGB0(p,(font_id>>16)&0xff,(font_id>>8)&0xff,font_id&0xff);
			}
			else
				setRGB0(p,128,128,128);
			p->clut=getPSXClut(896);
//			p->tpage=getPSXTPage(896);
			DOPRIM(PANEL_OTZ,p);
			x0+=wadmenu_f_width[c]+1;
			break;
		}
		m++;
	}
	DOPRIM(PANEL_OTZ,tp);
#else
#if 0
	POLY_FT4 *p;
	SLONG Kanji_found=Kanji_next;

	Kanji_found=Kanji_FindString(message);

	// This may seem strange, but all valid addresses are negative except those
	// on the scratch pad, and since scratch pad strings are sprintf'd in functions
	// these are unlikely to be the same next time around.

	if ((SLONG)message>0)
		Kanji_JunkString(Kanji_found);

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	// Cheat to fucking buggery here, since were SJIS double byte character string
	// and each character is 16 pixels wide we
	setXYWH(p,x,y,8*strlen(message),16);
	setUVWH(p,0,16*Kanji_found,8*strlen(message)-1,15);
	setRGB0(p,(font_id>>16)&0xff,(font_id>>8)&0xff,font_id&0xff);
	p->tpage=getTPage(0,0,Kanji_buffer_x,Kanji_buffer_y);
	p->tpage=getClut(Kanji_clut_x,Kanji_clut_y);
	DOPRIM(PANEL_OTZ,p);
#else
	Kanji_string(x,y,(UWORD*)message,font_id,256);
#endif
#endif
}


#ifndef VERSION_KANJI
SLONG Wadmenu_draw_char(SLONG x,SLONG y,unsigned char c,SLONG colour)
{
	POLY_FT4 *p;
	int c0;

	if (c==' ')
		return 16;

	if (c<128)
		c0=(int)strchr(Wadmenu_char_table,toupper(c));
	else
		c0=(int)strchr(Wadmenu_char_table,Wadmenu_char_lookup[c-128]);

	if (c0)
	{
		c0-=(int)Wadmenu_char_table;

		ALLOCPRIM(p,POLY_FT4);
		setPolyFT4(p);
		setXYWH(p,x,y,23,23);
		setUVWH(p,(c0&7)*24,(c0>>3)*24,23,23);
		setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,colour&0xff);
		if (colour&0xff000000)
			setSemiTrans(p,1);
		p->clut=getPSXClut(448);
		p->tpage=getPSXTPage(448);
		DOPRIM(PANEL_OTZ,p);
		return Wadmenu_char_width[c0];
	}
	return 0;
}

void Wadmenu_new_text(SLONG x,SLONG y,char *str,SLONG colour)
{
	SLONG x0=x;
	unsigned char *c=(unsigned char *)str;

	while(*c)
		x0+=Wadmenu_draw_char(x0,y,*c++,colour);
}

int Wadmenu_text_width(char *str)
{
	SLONG w=0;
	char *c=str;

	while(*c)
	{
		if (*c==32)
			w+=16;
		else
		{
			SLONG c0=(int)strchr(Wadmenu_char_table,toupper(*c));
			if (c0)
				w+=Wadmenu_char_width[c0-(int)Wadmenu_char_table];
		}
		c++;
	}
	return w;
}
#endif

void Wadmenu_draw_text(SLONG x,SLONG y,char *str,SLONG colour,SLONG flags)
{
	SLONG tx,ty,count;
	SLONG tw;
	SLONG th;

#ifndef VERSION_KANJI
	if (flags&WADMENU_TEXT_BIG)
	{
		tw=Wadmenu_text_width(str);
		th=12;
	}
	else
	{
		tw=Wadmenu_text_width2(str);
		th=6;
	}
#else
	tw=text_width(str,colour,&count);
	th=8;
#endif
	if (flags&WADMENU_TEXT_CENTRE)
	{
		tx=x-(tw>>1);
		ty=y-th;
	}
	else if (flags&WADMENU_TEXT_RIGHT)
	{
		tx=x-tw;
		ty=y;
	}
	else
	{
		tx=x;
		ty=y;
	}
#ifndef VERSION_KANJI
	if (flags&WADMENU_TEXT_BIG)
	{
		Wadmenu_new_text(tx,ty,str,colour);
		if (flags&WADMENU_TEXT_SHADOW)
			Wadmenu_new_text(tx+2,ty+2,str,0x000001);
	}
	else
#endif
	{
		Wadmenu_draw_text_at(tx,ty,str,colour);
		if (flags&WADMENU_TEXT_SHADOW)
			Wadmenu_draw_text_at(tx+1,ty+1,str,0x000001);
	}
	if (flags&WADMENU_TEXT_BOXOUT)
		DRAW2D_Box_Page(tx-1,ty-1,tx+tw,ty+(th<<1),0x000000);
}

void draw_line(int x1,int y1,int x2,int y2,int colour)
{
	LINE_F2 *p;
	ALLOCPRIM(p,LINE_F2);
	setLineF2(p);
	setXY2(p,x1,y1,x2,y2);

	if (colour&0xff000000)
		setSemiTrans(p,1);

	setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,colour&0xff);

	DOPRIM(PANEL_OTZ,p);
}
/*
void Wadmenu_DrawItems(int selected,int moff)
{
	int line,c0,c0,max_w=0;

	line=WADMENU_ITEM_Y;

	Wadmenu_draw_text(250,60,STR_UP,(moff>0)?MENU_TEXTHIGH:MENU_TEXTTRANS,WADMENU_TEXT_ICON);
	for(c0=moff;(c0<Wadmenu_levels_used)&&(line<164);c0++)
	{
		Wadmenu_draw_text(WADMENU_ITEM_X,line,brief[Wadmenu_level[c0]].title,(c0==selected)?MENU_TEXTHIGH:MENU_TEXTTRANS,WADMENU_TEXT_MISSION);
		line+=24;
	}
	Wadmenu_draw_text(250,164,STR_DOWN,(c0==Wadmenu_levels_used)?MENU_TEXTTRANS:MENU_TEXTHIGH,WADMENU_TEXT_ICON);
}
*/
extern char wadmenu_filename[32];
extern SLONG MFX_Conv_playing;
extern SWORD music_current_level;
extern SLONG MFX_music_int;
extern SLONG MFX_Speech_End;

int Wadmenu_title;
int Wadmenu_oldtitle;
int Wadmenu_Brief_End;

#define BRIEF_MAX_SIZE	8448

UBYTE level_brief[]={0,0,0,0,1,0,0,2,0,0,3,0,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,0};

#ifndef VERSION_DEMO
char *Wadmenu_GetWad(TIM_IMAGE *tim)
{
	int selected=0,i;
	int done=-1;
	int pad_key=0;
	int offset=0;
	static int moff=0;


//	Wadmenu_BuildAvailable();

	while(done!=3)
	{
//		Wadmenu_ParticleSync(tim);

		Wadmenu_DoParticleElement(wad_level);

		switch(done)
		{
		case -1:
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				done=0;
			break;
		case 0:
		case 1:
			selected=Wadmenu_GetDistrict(tim);
			if (selected==-1)
			{
				GAME_TURN=0;
				return NULL;
			}
			else
			{
				done=2;
				offset=0;
				Wadmenu_ParticleEffect(WADMENU_BRIEF_IN,0,0);
				MFX_Conv_play(level_brief[selected],0,0);
			}

			break;
		case 2:
			Wadmenu_ParticleSync(tim);

//			Wadmenu_BriefSync(selected);
			
			Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,brief[selected].title,MENU_TEXTHIGH,WADMENU_TEXT_TITLE);

			if (offset)
				Wadmenu_draw_text(WADMENU_ARROW_X,WADMENU_BRIEF_Y,STR_UP,(GAME_TURN&8)?MENU_TEXTLOW:MENU_TEXTHIGH,WADMENU_TEXT_NORMAL);

			for(i=0;i<15;i++)
				if (brief[selected].brief[i+offset][0])
					Wadmenu_draw_text(WADMENU_BRIEF_X,WADMENU_BRIEF_Y+(10*i),brief[selected].brief[i+offset],MENU_TEXTHIGH,WADMENU_TEXT_NORMAL);

			if ((offset<5)&&(brief[selected].brief[15+offset][0]))
				Wadmenu_draw_text(WADMENU_ARROW_X,WADMENU_BRIEF_Y+140,STR_DOWN,(GAME_TURN&8)?MENU_TEXTHIGH:MENU_TEXTLOW,WADMENU_TEXT_NORMAL);

			Wadmenu_DrawIconicStuff(WADMENU_MISSION_SCREEN);

			if (pad_key==0)
			{
				if (PadKeyIsPressed(&PAD_Input1,PAD_LU)&&(offset>0))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					offset--;
					pad_key=5;
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_LD)&&(offset<5))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					if (brief[selected].brief[15+offset][0])
						offset++;
					pad_key=5;
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD)&&(brief[selected].type!=0))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
//					Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_LOADING),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
					done=3;
					Wadmenu_ParticleEffect(WADMENU_BRIEF_OUT,0,0);
				} 
				else if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					selected=0;
					done=0;
					offset=0;
					pad_key=10;
					Wadmenu_ParticleEffect(WADMENU_BRIEF_OUT,0,0);
					MFX_Conv_stop();
//					Wadmenu_BriefStop();
				}
			} else
				pad_key--;
#ifndef FS_ISO9660
			if (PadKeyIsPressed(&PAD_Input1,PAD_RR))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				AENG_screen_shot(512);
				pad_key=5;
			}
#endif

			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_SEL_CAN),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
			break;
		}
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	AENG_flip2(tim->paddr);
	GAME_TURN=0;

	wad_level=selected+1;
	Wadmenu_MuckyTime=Wadmenu_ParTime[wad_level].time;
	strcpy(Wadmenu_MuckyName,Wadmenu_ParTime[wad_level].player);

	//strcpy(wadmenu_filename,brief[selected].mission);
	sprintf(wadmenu_filename,WADMENU_FILETEMPLATE,wad_level/10,wad_level);
	return (wadmenu_filename);
}
#else
char *Wadmenu_GetWad(TIM_IMAGE *tim)
{
	int selected=34,i;
	int done=-1;
	int pad_key=0;
	int offset=0;
	static int moff=0;


//	Wadmenu_BuildAvailable();

	while(done!=3)
	{
//		Wadmenu_ParticleSync(tim);

		Wadmenu_DoParticleElement(wad_level);

		switch(done)
		{
		case -1:
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
			{
				done=0;
				Wadmenu_ParticleEffect(WADMENU_BRIEF_IN,0,0);
			}
			break;
		case 0:
			Wadmenu_ParticleSync(tim);

//			Wadmenu_BriefSync(selected);
			
			Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,brief[selected].title,MENU_TEXTHIGH,WADMENU_TEXT_TITLE);

			if (offset)
				Wadmenu_draw_text(WADMENU_ARROW_X,WADMENU_BRIEF_Y,STR_UP,(GAME_TURN&8)?MENU_TEXTLOW:MENU_TEXTHIGH,WADMENU_TEXT_NORMAL);

			for(i=0;i<15;i++)
				if (brief[selected].brief[i+offset][0])
					Wadmenu_draw_text(WADMENU_BRIEF_X,WADMENU_BRIEF_Y+(10*i),brief[selected].brief[i+offset],MENU_TEXTHIGH,WADMENU_TEXT_NORMAL);

			if ((offset<5)&&(brief[selected].brief[15+offset][0]))
				Wadmenu_draw_text(WADMENU_ARROW_X,WADMENU_BRIEF_Y+140,STR_DOWN,(GAME_TURN&8)?MENU_TEXTHIGH:MENU_TEXTLOW,WADMENU_TEXT_NORMAL);

			Wadmenu_DrawIconicStuff(WADMENU_MISSION_SCREEN);

			if (pad_key==0)
			{
				if (PadKeyIsPressed(&PAD_Input1,PAD_LU)&&(offset>0))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					offset--;
					pad_key=5;
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_LD)&&(offset<5))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					if (brief[selected].brief[15+offset][0])
						offset++;
					pad_key=5;
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD)&&(brief[selected].type!=0))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
//					Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_LOADING),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
					done=3;
					Wadmenu_ParticleEffect(WADMENU_BRIEF_OUT,0,0);
				} 
				else if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					GAME_TURN=0;
					Wadmenu_ParticleEffect(WADMENU_BRIEF_OUT,0,0);
					MFX_Conv_stop();
					return NULL;
				}
			} else
				pad_key--;

			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_SEL_CAN),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
			break;
		}
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	AENG_flip2(tim->paddr);
	GAME_TURN=0;

	wad_level=35;
	Wadmenu_MuckyTime=Wadmenu_ParTime[wad_level].time;
	strcpy(Wadmenu_MuckyName,Wadmenu_ParTime[wad_level].player);

	//strcpy(wadmenu_filename,brief[selected].mission);
	sprintf(wadmenu_filename,WADMENU_FILETEMPLATE,wad_level);
	return (wadmenu_filename);
}
#endif

#define WADMENU_TYPE_VOID	0
#define WADMENU_TYPE_STR	1
#define WADMENU_TYPE_MENU	2
#define WADMENU_TYPE_SLIDER	3
#define WADMENU_TYPE_BACK	4
#define WADMENU_TYPE_MENUNORET 5

extern WadMenuItem MainMenu[];

int audio_title[2]={WAD_AUD_MONO,WAD_AUD_STEREO};

void Wadmenu_MonoStereo(WadMenuItem *item)
{
	sound_mode=1-sound_mode;
	item->name=audio_title[sound_mode];
}

typedef struct {
	int name;
	SLONG config;
	SLONG flag;
} PadSetup;

PadSetup pad_info[14]={
	{(WAD_PAD_RUN),			INPUT_MASK_RUN,PAD_FLAG_NONE},
	{(WAD_PAD_LEFT),		INPUT_MASK_LEFT,PAD_FLAG_NONE},
	{(WAD_PAD_RIGHT),		INPUT_MASK_RIGHT,PAD_FLAG_NONE},
	{(WAD_PAD_BACK),		INPUT_MASK_BACKWARDS,PAD_FLAG_NONE},
	                      
	{(WAD_PAD_KICK),		INPUT_MASK_KICK,PAD_FLAG_NONE},
	{(WAD_PAD_PUNCH),		INPUT_MASK_PUNCH,PAD_FLAG_NONE},
	{(WAD_PAD_ACTION),		INPUT_MASK_ACTION,PAD_FLAG_NONE},
	{(WAD_PAD_JUMP),		INPUT_MASK_JUMP,PAD_FLAG_NONE},
	                      
	{(WAD_PAD_FPMODE),		INPUT_MASK_CAMERA,PAD_FLAG_NONE},
	{(WAD_PAD_NEWTARG),		INPUT_MASK_STEP_RIGHT,PAD_FLAG_NONE},
	{(WAD_CAM_LEFT),		-KB_DEL,PAD_FLAG_DEBOUNCE},
	{(WAD_CAM_RIGHT),		-KB_PGDN,PAD_FLAG_DEBOUNCE},
	                      
	{(WAD_PAD_INV),			INPUT_MASK_SELECT,PAD_FLAG_NONE},
	{(WAD_PAD_PAUSE),		-KB_ESC,PAD_FLAG_DEBOUNCE}
};

extern PadInfo pad_cfg0,pad_cfg1,pad_cfg2,pad_cfg3,pad_free;

PadInfo *pad_cfg[5]={&pad_cfg0,&pad_cfg1,&pad_cfg2,&pad_cfg3,&pad_free};
extern PadInfo *PAD_Current;

#define PAD_POSITION_LEFT	0
#define PAD_POSITION_CENTRE	1
#define PAD_POSITION_RIGHT	2


DVECTOR key_loc[]={
	{200,127},{168,146},{214,154},{200,163},
	{312,127},{292,135},{340,146},{312,163},
	{334,95},{177,95},{334,110},{177,110},
	{108,192},{108,204}
};

UBYTE key_just[]={
	PAD_POSITION_RIGHT,		PAD_POSITION_RIGHT,		PAD_POSITION_LEFT,		PAD_POSITION_RIGHT,
	PAD_POSITION_LEFT,		PAD_POSITION_RIGHT,		PAD_POSITION_LEFT,		PAD_POSITION_LEFT,
	PAD_POSITION_LEFT,		PAD_POSITION_RIGHT,		PAD_POSITION_LEFT,		PAD_POSITION_RIGHT,
	PAD_POSITION_LEFT,		PAD_POSITION_LEFT
};

extern void Wadpart_Box(SLONG x,SLONG y,SLONG w,SLONG h,SLONG colour);

void Wadmenu_DrawIconicStuff(SLONG flag)
{
	if (flag & WADMENU_MAP_SCREEN)
	{
		Wadmenu_Image(WADMENU_BUTTON,448,WADMENU_BUTTON_Y,256,0x7f7f7f);
		Wadmenu_Image(WADMENU_DARCI_MAP,0,0,256,0x7f7f7f);
		Wadmenu_Backboard(WADMENU_MAP_BACK,0,20);
	}

	if (flag & WADMENU_MISSION_SCREEN)
	{
		Wadmenu_Image(WADMENU_BUTTON,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,256,0x7f7f7f);
		Wadmenu_Image(WADMENU_DARCI_MISS,354,0,256,0x7f7f7f);
		Wadpart_Box(0,20,512,200,0x404040);
	}

	if (flag & WADMENU_PAD_SCREEN)
	{
		Wadmenu_Image(WADMENU_BUTTON,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,256,0x7f7f7f);
		Wadmenu_Image(WADMENU_PAD_TOP,160,64,256,0x7f7f7f);
		Wadmenu_Image(WADMENU_PAD_FRONT,152,128,256,0x7f7f7f);
	}

	if (flag == WADMENU_MENU_SCREEN)
		Wadmenu_Image(WADMENU_BUTTON,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,256,0x7f7f7f);

	if (flag & WADMENU_BRIEF_SCREEN)
	{
		Wadmenu_Image(WADMENU_BUTTON,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,256,0x7f7f7f);
	}

	if (flag == WADMENU_BADCOP_SCREEN)
	{
		Wadmenu_Image(WADMENU_BUTTON,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,256,0x7f7f7f);
		Wadmenu_Image(WADMENU_ROPER_MAP,256,0,256,0x7f7f7f);
		Wadpart_Box(0,20,512,200,0x404040);
	}
	switch(Wadmenu_PadType)
	{
	case 0:
		Wadmenu_draw_text(256,192,W_(WAD_CTRL_NONE),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
	case 4:
	case 7:
		break;
	default:
		Wadmenu_draw_text(256,192,W_(WAD_CTRL_UNSUP),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
		break;
	}

}

void Wadmenu_ParticleEffect(SLONG flag,SLONG data1,SLONG data2)
{
	if (flag & WADMENU_EFFECT_IN)
	{
		if (flag & WADMENU_EFFECT_MAP)
			Wadpart_AddImageParticle(WADMENU_BUTTON,0x7f7f7f,448,256,448,WADMENU_BUTTON_Y,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
		else
			Wadpart_AddImageParticle(WADMENU_BUTTON,0x7f7f7f,512+WADMENU_BUTTON_X,WADMENU_BUTTON_Y,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
	}
	if (flag & WADMENU_EFFECT_OUT)
	{
		if (flag & WADMENU_EFFECT_MAP)
			Wadpart_AddImageParticle(WADMENU_BUTTON,0x7f7f7f,448,WADMENU_BUTTON_Y,448,-64,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
		else
			Wadpart_AddImageParticle(WADMENU_BUTTON,0x7f7f7f,WADMENU_BUTTON_X,WADMENU_BUTTON_Y,WADMENU_BUTTON_X,-64,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
	}

	if (flag & WADMENU_EFFECT_MAP)
	{
		if (flag & WADMENU_EFFECT_IN)
		{
			Wadpart_AddBoardParticle(WADMENU_MAP_BACK,512,20,0,20,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddImageParticle(WADMENU_DARCI_MAP,0x7f7f7f,-192,0,-1,0,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
		}
		else
		{
			Wadpart_AddBoardParticle(WADMENU_MAP_BACK,0,20,-512,20,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddImageParticle(WADMENU_DARCI_MAP,0x7f7f7f,0,0,-192,0,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
		}
	}

	if (flag & WADMENU_EFFECT_BRIEF)
	{
		if (flag & WADMENU_EFFECT_IN)
		{
			Wadpart_AddImageParticle(WADMENU_DARCI_MISS,0x7f7f7f,512,0,354,0,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddRectParticle(0,20,512,200,0x000000,WADPART_FLAG_FADEIN);
		}
		else
		{
			Wadpart_AddImageParticle(WADMENU_DARCI_MISS,0x7f7f7f,354,0,512,0,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddRectParticle(0,20,512,200,0x404040,WADPART_FLAG_FADE);
		}
	}

	if (flag & WADMENU_EFFECT_MENU)
	{
		if (flag & WADMENU_EFFECT_IN)
			Wadmenu_BackMenu((WadMenuItem *)data1,data2);
		else
			Wadmenu_NewMenu((WadMenuItem *)data1,data2);
	}

	if (flag & WADMENU_EFFECT_PAD)
	{
		if (flag & WADMENU_EFFECT_IN)
		{
			Wadpart_AddImageParticle(WADMENU_PAD_TOP,0x7f7f7f,-192,64,160,64,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddImageParticle(WADMENU_PAD_FRONT,0x7f7f7f,512,128,152,128,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
		}
		else
		{
			Wadpart_AddImageParticle(WADMENU_PAD_TOP,0x7f7f7f,160,64,512,64,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddImageParticle(WADMENU_PAD_FRONT,0x7f7f7f,152,128,-204,128,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
		}
	}
	if (flag & WADMENU_EFFECT_BADCOP)
	{
		if (flag & WADMENU_EFFECT_IN)
		{
			Wadpart_AddImageParticle(WADMENU_ROPER_MAP,0x7f7f7f,512,0,256,0,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddRectParticle(0,20,512,200,0x000000,WADPART_FLAG_FADEIN);
		} 
		else
		{
			Wadpart_AddImageParticle(WADMENU_ROPER_MAP,0x7f7f7f,256,0,512,0,WADMENU_EFFECT_LEN,WADPART_FLAG_NORMAL);
			Wadpart_AddRectParticle(0,20,512,200,0x404040,WADPART_FLAG_FADE);
		}
	}
}

void Wadmenu_PadDrawConfig(PadInfo *info,SLONG free)
{
	int i,id;
	int line=80;
	char cstr[40];
	SLONG c,width;

	if (pad_config!=4)
	{
		sprintf(cstr,W_(WAD_CFG_NUMBER),pad_config);
		Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,cstr,MENU_TEXTHIGH,WADMENU_TEXT_TITLE);
	}
	else
		Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(WAD_CFG_FREE),MENU_TEXTHIGH,WADMENU_TEXT_TITLE);

	for(i=0;i<14;i++)
	{
		if (info->data[i].name)
		{
			width=Wadmenu_text_width2(W_(info->data[i].name));

			switch(key_just[i])
			{
			case PAD_POSITION_LEFT:
				Wadmenu_draw_text(key_loc[i].vx,key_loc[i].vy,W_(info->data[i].name),MENU_TEXTHIGH,WADMENU_TEXT_SHADOW);
				break;
			case PAD_POSITION_CENTRE:
				Wadmenu_draw_text(key_loc[i].vx,key_loc[i].vy+6,W_(info->data[i].name),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE|WADMENU_TEXT_SHADOW);
				break;
			case PAD_POSITION_RIGHT:
				Wadmenu_draw_text(key_loc[i].vx,key_loc[i].vy,W_(info->data[i].name),MENU_TEXTHIGH,WADMENU_TEXT_RIGHT|WADMENU_TEXT_SHADOW);
				break;
			}
		}
	}

		id = PadInfoMode(0,InfoModeIdTable,1);
		Wadmenu_draw_text(48,192,W_(WAD_PAD_START),MENU_TEXTHIGH,WADMENU_TEXT_SHADOW);
		Wadmenu_draw_text(48,204,W_(WAD_PAD_SELECT),MENU_TEXTHIGH,WADMENU_TEXT_SHADOW);

	if (!free)
	{
		if ((id==7))
		{
			if (vibra_mode)
				Wadmenu_draw_text(368,188,W_(WAD_VIB_ON),MENU_TEXTHIGH,WADMENU_TEXT_SHADOW);
			else
				Wadmenu_draw_text(368,188,W_(WAD_VIB_OFF),MENU_TEXTHIGH,WADMENU_TEXT_SHADOW);
		}
	}

	Wadmenu_DrawIconicStuff(WADMENU_PAD_SCREEN);
}

SLONG Wadmenu_FindKey(SLONG pad)
{
	pad=pad^0xffff;
	switch(pad)
	{
	case PAD_LU:
		return 0;
	case PAD_LL:
		return 1;
	case PAD_LR:
		return 2;
	case PAD_LD:
		return 3;
	case PAD_RU:
		return 4;
	case PAD_RL:
		return 5;
	case PAD_RR:
		return 6;
	case PAD_RD:
		return 7;
	case PAD_FLT:
		return 8;
	case PAD_FRT:
		return 9;
	case PAD_FLB:
		return 10;
	case PAD_FRB:
		return 11;
#ifndef VERSION_USA
	case PAD_START:
		return 12;
#endif
	case PAD_SEL:
		return 13;
	case WADMENU_PAD_CANCEL:
		return 0xdead;
	}
	return -1;
}

void Wadmenu_PadConfigFree(TIM_IMAGE *tim)
{
	int awaiting=1,k;
	int key=0,i,done=0;
	pad_config=4;
	char cstr[40];
	UBYTE used[14];

	for(i=0;i<14;i++)
	{
		used[i]=0;
		pad_free.data[i].name=0;
	}
#if VERSION_USA
	used[12]=1;
	pad_free.data[12].name=pad_info[13].name;
	pad_free.data[12].pad_button=PAD_START^0xffff;
	pad_free.data[12].pad_delay=0;
	pad_free.data[12].pad_flags=pad_info[13].flag;
	pad_free.data[12].input_mask=pad_info[13].config;
#endif

	while(!done)
	{
		Wadmenu_PadDrawConfig(pad_cfg[4],1);
		Wadmenu_draw_text(256,56,CONFIG_CANCEL,MENU_TEXTHIGH,WADMENU_TEXT_CENTRE|WADMENU_TEXT_SHADOW);
		if (awaiting)
		{
			if (!PadKeyPressed(&PAD_Input1))
				awaiting=0;
			else
				if (Wadmenu_FindKey(PAD_Input1.data.pad)==0xdead)
				{
					MFX_play_ambient(1,S_ITEM_REVEALED,MFX_REPLACE);
					pad_config=0;
					for(i=0;i<14;i++)
						pad_free.data[i].name=0;
					done=1;
				}

#ifndef VERSION_USA
			if (key==14)
#else
			if (key>12)
#endif
				done=1;
		} 
		else
		{
			sprintf(cstr,W_(WAD_CFG_PRESS),W_(pad_info[key].name));
			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,cstr,MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
			if (PadKeyPressed(&PAD_Input1))
			{
				k=Wadmenu_FindKey(PAD_Input1.data.pad);
				switch(k)
				{
				case 0xdead:
					MFX_play_ambient(1,S_ITEM_REVEALED,MFX_REPLACE);
					pad_config=0;
					for(i=0;i<14;i++)
						pad_free.data[i].name=0;
					key=14;
					done=1;
//					awaiting=1;
					break;
				case -1:
					//MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					awaiting=0;
					break;
				default:
					if (used[k])
					{
						MFX_play_ambient(0,S_ITEM_REVEALED,MFX_REPLACE);
						awaiting=1;
						break;
					}
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					pad_free.data[k].name=pad_info[key].name;
					pad_free.data[k].input_mask=pad_info[key].config;
					pad_free.data[k].pad_delay=0;
					pad_free.data[k].pad_flags=pad_info[key].flag;
					pad_free.data[k].pad_button=PAD_Input1.data.pad^0xffff;
					used[k]=1;
					key++;
					awaiting=1;
				}
			}
		}

		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
}

void Wadmenu_BadCop(TIM_IMAGE *tim,CBYTE *mess)
{
	SLONG line,x;
	CBYTE buf[80];
	CBYTE *p,*p2;

	int awaiting=1;

	Wadmenu_ParticleSync(tim);
	Wadmenu_ParticleEffect(WADMENU_BADCOP_IN,0,0);

	while(1)
	{
		Wadmenu_ParticleSync(tim);
		Wadmenu_DoParticleElement(wad_level);
		line=96;
		p=mess;
		while(*p)
		{
			p2=buf;
			x=16;
			while((*p)&&(x<320))
			{
				x+=wadmenu_f_width[*p-32]+1;
				*p2++=*p;
				p++;
			}
			// Find last space
			while((*p)&&(*p!=32))
			{
				p--;
				p2--;
			}
			*p2=0;
			if (*p)
				p++;

			Wadmenu_draw_text(16,line,buf,0x7f7f7f,WADMENU_TEXT_NORMAL);
			line+=10;
		}
		Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_SEL_SELECT),0x7f7f7f,WADMENU_TEXT_KEYS);

		if (awaiting)
		{
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				awaiting=0;
		}
		else
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
			{
				GAME_TURN=0;
				Wadmenu_ParticleEffect(WADMENU_BADCOP_OUT,0,0);
				return;
			}
		}
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		Wadmenu_DrawIconicStuff(WADMENU_BADCOP_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		GAME_TURN++;
	}
}


SLONG Wadmenu_DoYorN(TIM_IMAGE *tim,SLONG title)
{
	int awaiting=1;

	Wadmenu_ParticleSync(tim);

	while(1)
	{
		Wadmenu_DoParticleElement(wad_level);
		Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(title),MENU_TEXTHIGH,WADMENU_TEXT_ITEM);
		Wadmenu_draw_text(WADMENU_YORN_X,WADMENU_YORN_Y1,W_(WAD_MEM_SURE),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE|WADMENU_TEXT_BIG);
		Wadmenu_draw_text(WADMENU_YORN_X,WADMENU_YORN_Y2,W_(WAD_MEM_YORN),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);

		if (awaiting)
		{
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				awaiting=0;
		}
		else
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				return 1;
			if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				return 0;
		}
		Wadmenu_DrawIconicStuff(WADMENU_MENU_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		GAME_TURN++;
	}
	GAME_TURN=0;
}

typedef struct {
	SLONG	text;
	SWORD	delay;
	SWORD	flags;
} CreditData;

#define CREDIT_NORMAL		0x0000
#define CREDIT_LARGE		0x0001
#define CREDIT_IMAGE		0x0002
#define CREDIT_TERMINAL		0x8000

CreditData *Wadmenu_credit;

char *Wadmenu_DoCredits(TIM_IMAGE *tim)
{
	int eoc=0;
	int awaiting=1;
	int cred_off=0;
	int delay;
	CBYTE *chr_buf;

//	credit=(CreditData*)MemAlloc(6144);

	delay=Wadmenu_credit[0].delay;

	chr_buf=(CBYTE*)Wadmenu_credit;
	
#ifndef VERSION_DEMO
	MUSIC_bodge_code=4;
#endif

	Wadmenu_ParticleSync(tim);

	while(!eoc)
	{
		if (delay)
			delay--;
		else
		{
			if (Wadmenu_credit[cred_off].flags&CREDIT_TERMINAL)
				eoc=1;
			else if (Wadmenu_credit[cred_off].flags&CREDIT_LARGE)
				Wadpart_AddStringParticle(&chr_buf[Wadmenu_credit[cred_off].text],0x7f7f7f,256,256,256,-16,272,WADMENU_TEXT_BIG|WADMENU_TEXT_CENTRE);
			else
				Wadpart_AddStringParticle(&chr_buf[Wadmenu_credit[cred_off].text],0x7f7f7f,256,256,256,-16,272,WADMENU_TEXT_CENTRE|WADMENU_TEXT_SHADOW);
			cred_off++;
			delay=Wadmenu_credit[cred_off].delay;
		}

		if (awaiting)
		{
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				awaiting=0;
		}
		else
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				eoc=1;
		}
		Wadmenu_Image(WADMENU_ROPER_MISS,364,0,256,0x7f7f7f);
		Wadmenu_Image(WADMENU_DARCI_MAP,0,0,256,0x7f7f7f);
		MUSIC_mode_process();
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		GAME_TURN++;
	}
#ifndef VERSION_DEMO
	MUSIC_bodge_code=0;
#endif
	GAME_TURN=0;
	Wadpart_Init();
//	MFX_Init_Speech(0);

	return NULL;
}


char *Wadmenu_PadConfig(TIM_IMAGE *tim)
{
	int awaiting=1;
	int done;
	int pad_key=0;
	int pad_hold=pad_config;

	Wadmenu_ParticleEffect(WADMENU_PAD_IN,0,0);
	Wadmenu_ParticleSync(tim);

	done=0;

	while(done==0)
	{
		Wadmenu_PadDrawConfig(pad_cfg[pad_config],0);

		if (awaiting)
		{
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				awaiting=0;
		}
		else
		{
			if (pad_key==0)
			{
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				{
					if ((pad_config!=4)||(pad_free.data[0].name!=0))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						done=1;
					}
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					pad_config=pad_hold;
					done=2;
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_RR))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					Wadmenu_PadConfigFree(tim);
					pad_key=5;
				}

				if (PadKeyIsPressed(&PAD_Input1,PAD_RL))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					vibra_mode=1-vibra_mode;
					if (vibra_mode)
						PSX_SetShock(0,255);
					pad_key=10;
				}

				if (PadKeyIsPressed(&PAD_Input1,PAD_LL))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					pad_config--;
					if (pad_config<0)
						pad_config=4;
					pad_key=5;
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_LR))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					pad_config++;
					if (pad_config==5)
						pad_config=0;
					pad_key=5;
				}
#ifndef FS_ISO9660
				if (PadKeyIsPressed(&PAD_Input1,PAD_FRT))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					AENG_screen_shot(512);
					pad_key=5;
				}
#endif

			}
			else
				pad_key--;
		}
		if ((pad_config==4)&&(pad_free.data[0].name==0))
			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y-12,W_(WAD_CAN_CANCEL),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
		else
			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y-12,W_(WAD_SEL_CAN),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
		Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_PAD_CONFIG),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);

		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	Wadmenu_ParticleEffect(WADMENU_PAD_OUT,0,0);
	return NULL;
}

#ifndef FS_ISO9660
extern void	setup_textures(int world);

char *Wadmenu_ViewTims(TIM_IMAGE *tim)
{
	int done=0;
	int awaiting=1;
	int mode=0;
	int textures=0;
	int page;
	int pad_key=0;
	int x,y,tex;

	char cstr[22];
	SLONG offset_y=0;

	Wadmenu_ParticleSync(tim);

	while(!done)
	{

		pollhost();
		Wadmenu_DoParticleElement(wad_level);

		if (awaiting)
		{
			awaiting=PadKeyIsPressed(&PAD_Input1,PAD_RD);
		}
		else
		{
			switch (mode)
			{
			case	0:
				Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(WAD_MENU_TIMS),MENU_TEXTHIGH,WADMENU_TEXT_ITEM);
				sprintf(cstr,STR_LEFT" Texture World: %02d "STR_RIGHT,textures);
				Wadmenu_draw_text(256,60,"Select a texture world to view",MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
				Wadmenu_draw_text(256,80,cstr,MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
				if (pad_key==0)
				{
					if (PadKeyIsPressed(&PAD_Input1,PAD_LL)&&(textures>0))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						pad_key=5;
						textures--;
					}
					if (PadKeyIsPressed(&PAD_Input1,PAD_LR)&&(textures<33))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						pad_key=5;
						textures++;
					}
					if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						mode=1;
						page=0;
						awaiting=1;
//						printf("Loading Textures.");
						setup_textures(textures);
//						printf("Loaded.");
					}
					if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						awaiting=1;
						mode=3;
					}
				} else
					pad_key--;
				break;
			case	1:
				Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(WAD_MENU_TIMS),MENU_TEXTHIGH,WADMENU_TEXT_ITEM);
				sprintf(cstr,STR_LEFT" Page: %02d "STR_RIGHT,page+1);
				draw_centre_text_at(256,192,cstr,MENU_TEXTHIGH,0);
				tex=page*64;
				if(page==16)
				{
					POLY_FT4 *p;
					ALLOCPRIM(p,POLY_FT4);
					setPolyFT4(p);
					setXYWH(p,32,48,450,128);
					setRGB0(p,128,128,128);
					p->tpage=getTPage(2,0,960,256);
					p->clut=0;
					setUVWH(p,0,offset_y,63,64);
					DOPRIM(PANEL_OTZ,p);
					tex++;

				}
				else
				{
					for(y=0;y<4;y++)
						for(x=0;x<16;x++)
						{
							POLY_FT4 *p;
							ALLOCPRIM(p,POLY_FT4);
							setPolyFT4(p);
							setXYWH(p,x*32,(y*32)+48,32,32);
							setRGB0(p,128,128,128);
							if (page<4)
							{
								p->tpage=getPSXTPage(tex);
								p->clut=getPSXClut2(tex);
								setUVWH(p,getPSXU2(tex),getPSXV2(tex),63,63);
							}
							else
							{
								p->tpage=getPSXTPage(tex);
								p->clut=getPSXClut(tex);
								setUVWH(p,getPSXU(tex),getPSXV(tex),31,31);
							}
							DOPRIM(PANEL_OTZ,p);
							tex++;
						}
				}
				if (pad_key==0)
				{
					if (PadKeyIsPressed(&PAD_Input1,PAD_LD))
						offset_y++;
					if (PadKeyIsPressed(&PAD_Input1,PAD_LU))
						offset_y--;
					if (PadKeyIsPressed(&PAD_Input1,PAD_LL)&&(page>0))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						page--;
						pad_key=5;
					}
					if (PadKeyIsPressed(&PAD_Input1,PAD_LR)&&(page<16))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						page++;
						pad_key=5;
					}
					if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						mode=3;
						awaiting=1;
					}
					if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
					{
						MFX_play_ambient(0,S_PISTOL_DRY,0);
						mode=0;
						awaiting=1;
					}
				} else
					pad_key--;
				break;
			case	3:
				done=1;
				break;
			}
		}
		Wadmenu_draw_text(496,204,W_(WAD_SEL_CAN),MENU_TEXTHIGH,WADMENU_TEXT_RIGHT);

		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	setup_textures(0);

	return NULL;
}
#endif

char *Wadmenu_VideoPosition(TIM_IMAGE *tim)
{
	int awaiting=1;
	int done=0;
	int x,y;
//	char tmp[16];

	Wadmenu_ParticleSync(tim);

	while(!done)
	{
		draw_line(16,16,495,16,0xffffff);
		draw_line(16,16,16,223,0xffffff);
		draw_line(16,223,495,223,0xffffff);
		draw_line(495,16,495,223,0xffffff);

		Wadmenu_draw_text(256,80,W_(WAD_SCR_ADJUST1),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
		Wadmenu_draw_text(256,100,W_(WAD_SCR_ADJUST2),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
//		sprintf(tmp,"%d,%d",the_display.DisplayBuffers[0].Disp.screen.x,the_display.DisplayBuffers[0].Disp.screen.y);
//		Wadmenu_draw_text(48,48,tmp,MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
		Wadmenu_draw_text(492,192,W_(WAD_SEL_SELECT),MENU_TEXTHIGH,WADMENU_TEXT_RIGHT);

		if (awaiting)
		{
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				awaiting=0;
		}
		else
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,0);
				done=1;
			}

			if (PadKeyIsPressed(&PAD_Input1,PAD_LL)&&(screen_x>-20))
				screen_x--;
			if (PadKeyIsPressed(&PAD_Input1,PAD_LR)&&(screen_x<20))
				screen_x++;
			if (PadKeyIsPressed(&PAD_Input1,PAD_LU)&&(screen_y>MIN_SCREEN_Y))
				screen_y--;
			if (PadKeyIsPressed(&PAD_Input1,PAD_LD)&&(screen_y<MAX_SCREEN_Y))
				screen_y++;

			the_display.DisplayBuffers[0].Disp.screen.x=screen_x;
			the_display.DisplayBuffers[0].Disp.screen.y=screen_y;
			the_display.DisplayBuffers[1].Disp.screen.x=screen_x;
			the_display.DisplayBuffers[1].Disp.screen.y=screen_y;
		}

		Wadmenu_DrawIconicStuff(WADMENU_MENU_SCREEN);
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	return NULL;
}

#ifndef FS_ISO9660

void WadMenu_SaveLanguage(int lang)
{
	char langname[32];
	char *alloc=(char *)MemAlloc(8192);
	char *p=&alloc[4*(WAD_MAX_ITEMS+2)]; // Directly after the size of the file
	long *p2=(long *)alloc;
	long handle,i;

	sprintf(langname,"data\\wadlang.%03d",lang);

	*p2++=WAD_MAX_ITEMS;
	for(i=0;i<WAD_MAX_ITEMS;i++)
	{
		*p2++=((SLONG)p-(SLONG)alloc)-(4*(WAD_MAX_ITEMS+2));
		strcpy(p,W_(i));
		p+=strlen(W_(i))+1;
	}
	*p2=(SLONG)p-(SLONG)alloc;

	handle=PCcreat(langname,0);
	PCwrite(handle,alloc,*p2);
	PCclose(handle);

	MemFree(alloc);
}
	
#endif

	

void WadMenu_LoadLanguage(int lang)
{
	char langname[48];
	long *p=(long *)&Wad_Str[1],i;

#ifndef VERSION_DEMO
	sprintf(langname,"data\\wadlang.%03d",lang);
#else
	sprintf(langname,"urban\\wadlang.000");
#endif
	PCReadFile(langname,(UBYTE*)Wad_Str,4096);

	// Add the base of the file onto all the pointers at the start of the file

	for(i=0;i<WAD_MAX_ITEMS;i++,p++)
		*p+=(long)Wad_Str+(4*(WAD_MAX_ITEMS+2));
}

#ifndef VERSION_DEMO
extern char *Wadmenu_LoadGame(TIM_IMAGE *tim);
extern char *Wadmenu_SaveGame(TIM_IMAGE *tim);
#endif

#if 0
void WadMenu_Language(WadMenuItem *item)
{
	// Add code to load the appropriate language file into memory here.
	sel_language=item->info;
//	WadMenu_LoadLanguage(sel_language);
}
#endif

char *Wadmenu_NewGame(TIM_IMAGE *tim)
{
	memset((void*)level_done,Wadmenu_Cheat,48);
	level_done[0]=1;
	Wadmenu_InitStats();
	return Wadmenu_GetWad(tim);
}

WadMenuItem VolumeMenu[]={
	{(WAD_MENU_FXVOL),	WADMENU_TYPE_SLIDER,(void (*)(WadMenuItem *))&sfx_volume,0,0},
	{(WAD_MENU_MUSVOL),	WADMENU_TYPE_SLIDER,(void (*)(WadMenuItem *))&music_volume,-1,0},
	{(WAD_MENU_SPEECH), WADMENU_TYPE_SLIDER,(void (*)(WadMenuItem *))&speech_volume,5,0},
	{(WAD_MENU_RETURN),	WADMENU_TYPE_BACK,NULL,0,1},
	{(WAD_MENU_VOLUME),	0,0,0,0}
};

WadMenuItem AudioMenu[]={
	{(WAD_AUD_STEREO),	WADMENU_TYPE_VOID,(void (*)(WadMenuItem *))Wadmenu_MonoStereo,0,0},
	{(WAD_MENU_VOLUME),	WADMENU_TYPE_MENU,(void (*)(WadMenuItem *))VolumeMenu,0,0},
	{(WAD_MENU_RETURN),	WADMENU_TYPE_BACK,(void (*)(WadMenuItem *))NULL,0,1},
	{(WAD_MENU_AUDIO),	0,0,0,0}
};

#if 0
WadMenuItem LanguageMenu[]={
	{(WAD_LANG_ENG),		WADMENU_TYPE_VOID,(void (*)(WadMenuItem *))&WadMenu_Language,0,0},
	{(WAD_LANG_FRENCH),		WADMENU_TYPE_VOID,(void (*)(WadMenuItem *))&WadMenu_Language,1,0},
	{(WAD_LANG_SPAN),		WADMENU_TYPE_VOID,(void (*)(WadMenuItem *))&WadMenu_Language,2,0},
	{(WAD_LANG_ITAL),		WADMENU_TYPE_VOID,(void (*)(WadMenuItem *))&WadMenu_Language,3,0},
	{(WAD_MENU_RETURN),		WADMENU_TYPE_BACK,NULL,0,1},
	{(WAD_MENU_LANG),		0,0,0,0}
};
#endif

WadMenuItem Config[]={
	{(WAD_MENU_AUDIO),		WADMENU_TYPE_MENU,(void (*)(WadMenuItem *))AudioMenu,0,0},
	{(WAD_MENU_POS),		WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_VideoPosition,0,0},
	{(WAD_MENU_PAD),		WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_PadConfig,0,0},
#if 0
	{(WAD_MENU_LANG),		WADMENU_TYPE_MENU,(void (*)(WadMenuItem *))LanguageMenu,0,0},
#endif
	{(WAD_MENU_RETURN),		WADMENU_TYPE_BACK,(void (*)(WadMenuItem *))NULL,0,1},
	{(WAD_MENU_CONFIG),		0,0,0,0}
};

WadMenuItem MainMenu[]={
	{(WAD_MENU_NEW),		WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_NewGame,0,0},
#ifndef VERSION_DEMO
	{(WAD_MENU_LOAD),		WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_LoadGame,0,0},
#endif
#ifdef MIKE
	{(WAD_MENU_TIMS),		WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_ViewTims,0,0},
#endif
	{(WAD_MENU_CONFIG),		WADMENU_TYPE_MENU,(void (*)(WadMenuItem *))Config,0,0},
	{(WAD_MENU_CREDITS),	WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_DoCredits,0,1},
	{(WAD_MENU_MAINTITLE),	0,0,0,0}

};

#ifndef VERSION_DEMO
WadMenuItem EndOfLevelMenu[]={
	{(WAD_MENU_SAVE),		WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_SaveGame,0,0},
	{(WAD_MENU_CONTINUE),	WADMENU_TYPE_STR,(void (*)(WadMenuItem *))Wadmenu_GetWad,0,0},
	{(WAD_MENU_CONFIG),		WADMENU_TYPE_MENU,(void (*)(WadMenuItem *))Config,0,0},
	{(WAD_MENU_ENDGAME),	WADMENU_TYPE_MENUNORET,(void (*)(WadMenuItem *))MainMenu,0,1},
	{(WAD_MENU_EOLTITLE),	0,0,0,0}
};
#endif

void Wadmenu_DrawMenu(WadMenuItem *menu,int selected,int stack)
{
	int colour;
	int i=0;
	int line=WADMENU_ITEM_Y;

	do
	{
		colour=(i==selected)?MENU_TEXTHIGH:MENU_TEXTTRANS;
		switch(menu[i].type)
		{
		case WADMENU_TYPE_VOID:
		case WADMENU_TYPE_STR:
		case WADMENU_TYPE_MENU:
		case WADMENU_TYPE_BACK:
		case WADMENU_TYPE_MENUNORET:
			Wadmenu_draw_text(WADMENU_ITEM_X,line,W_(menu[i].name),colour,WADMENU_TEXT_ITEM);
			line+=24;
			break;
		case WADMENU_TYPE_SLIDER:
			Wadmenu_draw_text(WADMENU_ITEM_X,line,W_(menu[i].name),colour,WADMENU_TEXT_ITEM);
#ifndef VERSION_KANJI
			Wadmenu_draw_text(189+(*menu[i].ptr.value>>1),line+20,STR_BLOCK,colour,WADMENU_TEXT_CENTRE);
#else
			DRAW2D_Box_Page(189+(*menu[i].ptr.value>>1),line+14,196+(*menu[i].ptr.value>>1),line+25,colour);
#endif
			draw_line(192,line+18,320,line+18,colour<<1);
			line+=36;
			break;
		}
	} while(menu[i++].last==0);
	Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(menu[i].name),MENU_TEXTHIGH,WADMENU_TEXT_ITEM);

	if (stack)
		Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_SEL_CAN),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
	else
		Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_SEL_SELECT),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
}	

extern void MDEC_VideoSet(int width,int height);

extern UBYTE Video_Played;
extern UBYTE Eidos_Played;

#ifndef VERSION_DEMO
#define PAD_KEY(x) { pad_key=x; time_out=GAME_TURN+1800; }
#else
extern SLONG demo_timeout;

#define PAD_KEY(x) { pad_key=x; time_out=GAME_TURN+(demo_timeout*25); }
#endif

void Wadmenu_Image(SLONG id,SLONG x,SLONG y,SLONG scale,SLONG colour)
{
	POLY_FT4 *p;
	SLONG aw,ah;
	W_Image *w=&image_list[id];

	aw=(w->width*scale)>>8;
	ah=(w->height*scale)>>8;

	ALLOCPRIM(p,POLY_FT4);

	SetPolyFT4(p);
	setXYWH(p,x,y,aw,ah);

	setUVWH(p,w->u,w->v,w->width,w->height);
	p->tpage=w->tpage;
	p->clut=w->clut;

	setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,colour&0xff);

	if (colour&0xff000000)
	{
		setSemiTrans(p,1);
		if (colour>0)
			p->tpage&=~(3<<5);
	}

	DOPRIM(PANEL_OTZ-1,p);

}

void Wadmenu_Backboard(SLONG id,SLONG x,SLONG y)
{
	POLY_FT4 *p;
	SLONG aw,ah;
	W_Image *w=&image_list[id];

	ALLOCPRIM(p,POLY_FT4);

	SetPolyFT4(p);
	setXYWH(p,x,y,256,200);

	setUVWH(p,0,0,255,200);
	p->tpage=w->tpage;
	p->clut=w->clut;

	setRGB0(p,128,128,128);

	DOPRIM(1,p);

	if (x<256)
	{
		ALLOCPRIM(p,POLY_FT4);
		SetPolyFT4(p);
		setXYWH(p,x+256,y,256,200);
		setUVWH(p,0,0,255,200);
		p->tpage=getTPage(1,0,640,256);
		p->clut=w->clut;
		setRGB0(p,128,128,128);

		DOPRIM(1,p);
	}
}


void Wadmenu_BackMenu(WadMenuItem *menu,SLONG selected)
{
	SLONG i=0;
	SLONG line=WADMENU_ITEM_Y;

	do
	{
		if (i==selected)
		{
			Wadpart_AddTextParticle(menu[i].name,MENU_TEXTHIGH,(i&1)?768:-256,line,
				WADMENU_ITEM_X,line,WADMENU_EFFECT_LEN,WADMENU_TEXT_TITLE);
		} else
		{
			Wadpart_AddTextParticle(menu[i].name,MENU_TEXTTRANS,(i&1)?768:-256,line,
				WADMENU_ITEM_X,line,WADMENU_EFFECT_LEN,WADMENU_TEXT_TITLE);
		}

		if (menu[i].type==WADMENU_TYPE_SLIDER)
			line+=36;
		else
			line+=24;


	} while (menu[i++].last==0);

	Wadpart_AddTextParticle(menu[i].name,MENU_TEXTHIGH,768,WADMENU_TITLE_Y,
							WADMENU_TITLE_X,WADMENU_TITLE_Y,WADMENU_EFFECT_LEN,WADMENU_TEXT_TITLE);
}


void Wadmenu_NewMenu(WadMenuItem *menu,SLONG selected)
{
	SLONG i=0;
	SLONG line=WADMENU_ITEM_Y;

	do
	{
		if (i==selected)
		{
			Wadpart_AddTextParticle(menu[i].name,MENU_TEXTHIGH,WADMENU_ITEM_X,line,
				(i&1)?-256:768,line,WADMENU_EFFECT_LEN,WADMENU_TEXT_TITLE);
		} else
		{
			Wadpart_AddTextParticle(menu[i].name,MENU_TEXTTRANS,WADMENU_ITEM_X,line,
				(i&1)?-256:768,line,WADMENU_EFFECT_LEN,WADMENU_TEXT_TITLE);
		}

		if (menu[i].type==WADMENU_TYPE_SLIDER)
			line+=36;
		else
			line+=24;

	} while (menu[i++].last==0);

	Wadpart_AddTextParticle(menu[i].name,MENU_TEXTHIGH,WADMENU_TITLE_X,WADMENU_TITLE_Y,WADMENU_TITLE_X,-32,WADMENU_EFFECT_LEN,WADMENU_TEXT_TITLE);
//	Wadmenu_oldtitle=menu[i].name;
}

SLONG	part_leaf_col[]={
	0x00634040,
	0x00406340,
	0x004f4f40,
	0x00405432
};

void Wadmenu_DoParticleElement(SLONG mode)
{
	switch(Wadmenu_Backdrop[mode])
	{
	case 0:
		if ((GAME_TURN&7)==0)
			Wadpart_AddLeafParticle(WADMENU_IMAGE_LEAF,part_leaf_col[Random()&3],-(Random()&0xff),(Random()&0xff)-32,128+(Random()&0x1ff),WADPART_FLAG_LEAF);
		break;
	case 1:
		// Rain goes in here.
		if ((GAME_TURN&3)==0)
			Wadpart_AddRainParticle(WADMENU_IMAGE_DROP,Random()&0x1ff,-(Random()&0x3f)-32,0,1024,256,12+(Random()&0x1f),WADPART_FLAG_RAIN);
		break;
	case 2:
		if ((GAME_TURN&3)==0)
			Wadpart_AddImageParticle(WADMENU_IMAGE_SNOW,0xff7f7f7f,Random()&0x1ff,-(Random()&0xff),0,0,512,WADPART_FLAG_SNOW);
		break;
	case 3:
//		if ((GAME_TURN&240)==0)
//		{
//			Wadpart_AddBloodParticle((GAME_TURN&256)?512:-32,(Random()&0x3f)+128,(GAME_TURN&256)?-(Random()&0xff):(Random()&0xff),-(Random()&0x1f)-16,(Random()&0xff)+128,WADPART_FLAG_BLOOD);
//			Wadpart_AddBloodParticle((GAME_TURN&256)?512:-32,(Random()&0x3f)+128,(GAME_TURN&256)?-(Random()&0xff):(Random()&0xff),-(Random()&0x1f)-16,(Random()&0xff)+128,WADPART_FLAG_BLOOD);
//		}
		break;
	}
}

void Wadmenu_Box_Page(SLONG x,SLONG y,SLONG x2,SLONG y2,SLONG rgb,SLONG rgb2)
{
	LINE_F3 *p;
	ALLOCPRIM(p,LINE_F3);
	setLineF3(p);
	setXY3(p,x,y,x2,y,x2,y2);
	setRGB0(p,(rgb2>>16)&0xff,(rgb2>>8)&0xff,rgb2&0xff);
	DOPRIM(PANEL_OTZ,p);
	ALLOCPRIM(p,LINE_F3);
	setLineF3(p);
	setXY3(p,x2,y2,x,y2,x,y);
	setRGB0(p,(rgb2>>16)&0xff,(rgb2>>8)&0xff,rgb2&0xff);
	DOPRIM(PANEL_OTZ,p);
	DRAW2D_Box_Page(x,y,x2,y2,rgb);
}

SLONG Wadmenu_District_New(SLONG dist)
{
	SLONG i;

	for(i=0;i<wad_used;i++)
	{
		if ((brief[i].district==dist)&&!level_done[i+1])
			return 1;
	}
	return 0;
}

void Wadmenu_DrawDistrict(SLONG selected)
{
	SLONG i,line,x,flags,c,max_w;
	// Draw rest of display stuff

	// Only display the district name in English or NTSC versions
#if defined(VERSION_ENGLISH)||defined(VERSION_NTSC)
	Wadmenu_draw_text(WADMENU_DIST_X,WADMENU_DIST_Y,Wadmenu_district[Wadmenu_sel_district].name,MENU_TEXTHIGH,WADMENU_TEXT_BIG);
#endif

	Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_MISS_KEYS),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);
	line=Wadmenu_district[Wadmenu_sel_district].y-20;
	x=Wadmenu_district[Wadmenu_sel_district].x-4;

	flags=WADMENU_TEXT_NORMAL;

	if (x>288)
		flags|=WADMENU_TEXT_RIGHT;
	else
		x+=24;

	max_w=0;

	for(i=0;i<Wadmenu_levels_used;i++,line+=12)
	{
		if (level_done[Wadmenu_level[i]+1])
			Wadmenu_draw_text(x,line,brief[Wadmenu_level[i]].title,i==selected?0x007f00:0x003f00,flags);
		else
			Wadmenu_draw_text(x,line,brief[Wadmenu_level[i]].title,i==selected?MENU_TEXTHIGH:MENU_TEXTTRANS,flags);
		max_w=MAX(max_w,Wadmenu_text_width2(brief[Wadmenu_level[i]].title));
	}
	if (flags&WADMENU_TEXT_RIGHT)
		Wadmenu_Box_Page(x-(max_w+4),Wadmenu_district[Wadmenu_sel_district].y-24,x-1,line,0xff000000,0xffffff);
	else
		Wadmenu_Box_Page(x-4,Wadmenu_district[Wadmenu_sel_district].y-24,x+max_w-1,line,0xff000000,0xffffff);

	Wadmenu_Image(WADMENU_PIN_GREEN,Wadmenu_district[Wadmenu_sel_district].x-12,Wadmenu_district[Wadmenu_sel_district].y-24,256,0x7f7f7f);
	for(i=0;i<48;i++)
		if (Wadmenu_district[i].opened&&(i!=Wadmenu_sel_district))
			Wadmenu_Image(Wadmenu_District_New(i)?WADMENU_PIN_BLUE:WADMENU_PIN_RED,Wadmenu_district[i].x-12,Wadmenu_district[i].y-24,256,0x7f7f7f);
}


void Wadmenu_CheatLevels(SLONG cheat)
{
	Wadmenu_Cheat=cheat;
}


void Wadmenu_FindDistricts()
{
	SLONG i;

	for(i=0;i<28;i++)
		Wadmenu_district[i].opened=0;

	for(i=0;i<wad_used;i++)
	{
		if (level_done[brief[i].parent-1])
		{
			Wadmenu_district[brief[i].district].opened=1;
			Wadmenu_sel_district=brief[i].district;
		}
	}
}

void Wadmenu_LevelByDistrict(SLONG dist)
{
	SLONG i;

	Wadmenu_levels_used=0;

	for(i=0;i<wad_used;i++)
	{
		if ((level_done[brief[i].parent-1])&&(brief[i].district==dist))
			Wadmenu_level[Wadmenu_levels_used++]=i;
	}
}

SLONG Wadmenu_SwitchDistrict(SLONG dist,SLONG dir)
{
	SLONG closest=INFINITY;
	SLONG close_dist=-1;
	SLONG i;

	for(i=0;i<28;i++)
	{
		if ((i!=dist)&&Wadmenu_district[i].x&&Wadmenu_district[i].opened)
		{
			SLONG new_dist=dir*(Wadmenu_district[i].x-Wadmenu_district[dist].x);
			if ((new_dist>0)&&(new_dist<closest))
			{
				closest=new_dist;
				close_dist=i;
			}
		}
	}
	return close_dist;
}

SLONG Wadmenu_GetDistrict(TIM_IMAGE *tim)
{
	SLONG done = 0,pad_key=0;

	SLONG cur_x,cur_y,new_x,new_y,selected;

	Wadmenu_ParticleEffect(WADMENU_MAP_IN,0,0);

	Wadmenu_FindDistricts();

	Wadmenu_LevelByDistrict(Wadmenu_sel_district);

	selected=Wadmenu_levels_used-1;

	Wadmenu_ParticleSync(tim);

	cur_x=Wadmenu_district[Wadmenu_sel_district].x;
	cur_y=Wadmenu_district[Wadmenu_sel_district].y;
	new_x=cur_x;
	new_y=cur_y;

	while(!done)
	{
//		Wadmenu_DoParticleElement(wad_level);

		MUSIC_mode(WADMENU_MUSIC_MODE);
		Wadmenu_DrawDistrict(selected);

		if (pad_key)
		{
			pad_key--;
		}
		else 
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_LU))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				if (selected>0)
					selected--;
				pad_key=5;
			}

			if (PadKeyIsPressed(&PAD_Input1,PAD_LD))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				if (selected<(Wadmenu_levels_used-1))
					selected++;
				pad_key=5;
			}

			if (PadKeyIsPressed(&PAD_Input1,PAD_LL))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				SLONG sel=Wadmenu_SwitchDistrict(Wadmenu_sel_district,-1);
				if (sel>=0)
				{
					Wadmenu_sel_district=sel;
					new_x=Wadmenu_district[Wadmenu_sel_district].x;
					new_y=Wadmenu_district[Wadmenu_sel_district].y;
					Wadmenu_LevelByDistrict(Wadmenu_sel_district);
					selected=Wadmenu_levels_used-1;
					pad_key=5;
				}
			}

			if (PadKeyIsPressed(&PAD_Input1,PAD_LR))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				SLONG sel=Wadmenu_SwitchDistrict(Wadmenu_sel_district,1);
				if (sel>=0)
				{
					Wadmenu_sel_district=sel;
 					new_x=Wadmenu_district[Wadmenu_sel_district].x;
					new_y=Wadmenu_district[Wadmenu_sel_district].y;
					Wadmenu_LevelByDistrict(Wadmenu_sel_district);
					selected=Wadmenu_levels_used-1;
					pad_key=5;
				}
			}
#ifndef FS_ISO9660
			if (PadKeyIsPressed(&PAD_Input1,PAD_RR))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				AENG_screen_shot(512);
				pad_key=5;
			}
#endif

				
			if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				done=1;
			}

			if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				Wadmenu_ParticleEffect(WADMENU_MAP_OUT,0,0);
				return -1;
			}

		}

		Wadmenu_DrawIconicStuff(WADMENU_MAP_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	Wadmenu_ParticleEffect(WADMENU_MAP_OUT,0,0);

	return Wadmenu_level[selected];
}

void Wadmenu_ParticleSync(TIM_IMAGE *tim)
{
	while(!Wadpart_Sync())
	{
		Wadmenu_DoParticleElement(wad_level);

		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
}

extern void	setup_textures(int world);

void Wadmenu_LoadingScreen(TIM_IMAGE *tim)
{
	char fname[20];
	ULONG *Back_Image;

	Back_Image=(ULONG*)MemAlloc(152*1024);

	MDEC_VideoSet(DisplayWidth,DisplayHeight);

#ifndef VERSION_DEMO
	sprintf(fname,LOADING_NAME,WADMENU_BACKDROP);
#else
	sprintf(fname,"URBAN\\DEMOLOAD.TIM");
#endif

	PCReadFile(fname,(UBYTE*)Back_Image,640*241);

	if(OpenTIM((ULONG*)Back_Image)==0)
	{
		ReadTIM(&tim[0]);
	}

//	Wadmenu_draw_text(296,192,"LOADING.",MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_RIGHT);
	ClearOTag(the_display.DisplayBuffers[0].ot,OTSIZE);
	ClearOTag(the_display.DisplayBuffers[1].ot,OTSIZE);
//	Wadmenu_draw_text(296,192,W_(WAD_LOADING),MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_RIGHT);
	AENG_flip2(tim[0].paddr);
//	Wadmenu_draw_text(296,192,W_(WAD_LOADING),MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_RIGHT);
	AENG_flip2(tim[0].paddr);

	MemFree((void *)Back_Image);
}

#ifdef VERSION_DEMO

#define DEMO_BACKNAME "urban\\demoend.tim"

void Wadmenu_DemoSplash(void)
{
	char fname[24];
	ULONG *Back_Image,i;
	TIM_IMAGE tim;

	MUSIC_init_level(0);
	MFX_render();

//	SetupMemory();

extern UBYTE my_heap[];

	Back_Image=(ULONG*)&my_heap[131072];//(ULONG*)MemAlloc(1024*241);

	sprintf(fname,DEMO_BACKNAME);
	PCReadFile(fname,(UBYTE*)Back_Image,1024*241);

	if(OpenTIM((ULONG*)Back_Image)==0)
	{
		ReadTIM(&tim);
	}

	MDEC_VideoSet(512,240);

//	Wadmenu_draw_text(296,192,"LOADING.",MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_RIGHT);
	ClearOTag(the_display.DisplayBuffers[0].ot,OTSIZE);
	ClearOTag(the_display.DisplayBuffers[1].ot,OTSIZE);
//	Wadmenu_draw_text(296,192,W_(WAD_LOADING),MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_RIGHT);
	AENG_flip2(tim.paddr);
//	Wadmenu_draw_text(296,192,W_(WAD_LOADING),MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_RIGHT);
	AENG_flip2(tim.paddr);

	for(i=0;i<150;i++)
		VSync(0);

//	MemFree((void *)Back_Image);
}

void Wadmenu_Features(void)
{
	char fname[24];
	ULONG *Back_Image,i,scrn;
	TIM_IMAGE tim;

extern UBYTE my_heap[];

//	SetupMemory();

	Back_Image=(ULONG*)&my_heap[131072];//(ULONG*)MemAlloc(1024*241); //James I had to cast this to compile MikeD

	for(scrn=1;scrn<5;scrn++)
	{

		sprintf(fname,"URBAN\\FEATURE%d.TIM",scrn);
		PCReadFile(fname,(UBYTE*)Back_Image,1024*241);

		if(OpenTIM((ULONG*)Back_Image)==0)
		{
			ReadTIM(&tim);
		}

		MDEC_VideoSet(512,240);

		ClearOTag(the_display.DisplayBuffers[0].ot,OTSIZE);
		ClearOTag(the_display.DisplayBuffers[1].ot,OTSIZE);
		AENG_flip2(tim.paddr);
		AENG_flip2(tim.paddr);

		for(i=0;i<demo_timeout*12;i++)
		{
			VSync(0);
			// Skip the lot if we press a key in this mode.
			if (PadKeyPressed(&PAD_Input1))
			{
				i=demo_timeout*12;
				scrn=5;
			}
		}
	}
}
#endif

char *Wadmenu_AttractMenu(void)
{
	WadMenuItem *menu=MainMenu;
	char *str=0;
	int pad_key=0;
	int stack=0;
	int selected=0;
#ifndef VERSION_DEMO
	int time_out=1800;
#else
extern SLONG demo_timeout,demo_mode;

#ifdef VERSION_PAL
	int time_out=demo_timeout*25;
#else
	int time_out=demo_timeout*30;
#endif
#endif
	WadMenuItem *menu_stack[4];
	char fname[20];

	ULONG *Back_Image;
	ULONG *Pad_Image;
	TIM_IMAGE	tim[2];

	ClearOTag(the_display.DisplayBuffers[0].ot,OTSIZE);
	ClearOTag(the_display.DisplayBuffers[1].ot,OTSIZE);
#ifdef VERSION_KANJI
	Kanji_Init(960,0);
#endif
	WadMenu_LoadLanguage(sel_language);

	/*
	if (!Wadmenu_Display)
	{

		Wadmenu_LoadingScreen(tim);
		sprintf(wadmenu_filename,"level%d\\level%02d\\level.%s",wad_level/10,wad_level,level_ext[sel_language]);

		return wadmenu_filename;
	}
	*/

	// Only want to play the intro if we're here first time, or on a loop around the frontend.
	// When we come back from the game we want to check to see if we're playing an
	// End Of Level FMV
	if (Video_Played==0)
	{
		if (Eidos_Played==0)
		{
#if	!defined(MIKE)&&!defined(VERSION_DEMO)
			Video_Played=Wadmenu_PlayFMV(WADMENU_FMV_EIDOS);
			if (!Video_Played)
  				Video_Played=Wadmenu_PlayFMV(WADMENU_FMV_MUCKY);
#endif
			Eidos_Played++;
		}
#if !defined(MIKE)&&!defined(VERSION_DEMO)
		if (!Video_Played)
			Wadmenu_PlayFMV(WADMENU_FMV_INTRO);
#endif
		Video_Played=1;
	} else if (Wadmenu_Video)
	{
#if !defined(MIKE)&&!defined(VERSION_DEMO)
	  	Wadmenu_PlayFMV(Wadmenu_Video);
#endif
		Wadmenu_Video=0;
	}

#ifdef VERSION_DEMO
	if (demo_mode==1)
	{
		Wadmenu_Features();
		return NULL;
	}
#endif

	Wadmenu_LoadingScreen(tim);

	// Whoopse, had to move this, dont want to play FMV at 512x240
	setup_textures(0);

	Wadmenu_ReadWads(LANG_ROOT_NAME".pst");
	Wadmenu_district=MemAlloc(2048);
	PCReadFile(LANG_ROOT_NAME".DST",(UBYTE*)Wadmenu_district,2048);

	Back_Image=(ULONG*)MemAlloc(258*1024);
#ifndef VERSION_DEMO
	sprintf(fname,"data\\back%d.tim",WADMENU_BACKDROP);
#else
	sprintf(fname,"urban\\back0.tim");
#endif
	PCReadFile(fname,(UBYTE*)Back_Image,258*1024);

//	Pad_Image=(ULONG*)MemAlloc(258*1024);
//	ASSERT(Pad_Image);
//	PCReadFile("data\\padconf.tim",(UBYTE*)Pad_Image,258*1024);

//	PSX_SetShock(0,128);
	
	if(OpenTIM((ULONG*)Back_Image)==0)
	{
		ReadTIM(&tim[0]);
	}

//	if (OpenTIM((ULONG*)Pad_Image)==0)
//	{
//		ReadTIM(&tim[1]);
//	}

	Wadmenu_credit=(CreditData*)MemAlloc(6144);

#ifndef VERSION_DEMO
	PCReadFile("DATA\\CREDITS.CRD",(UBYTE*)Wadmenu_credit,6144);
#else
	PCReadFile("URBAN\\CREDITS.CRD",(UBYTE*)Wadmenu_credit,6144);
#endif

	MUSIC_init_level(0);
	// I actually want to make sure that music is
	MFX_Init_Speech(0);

	Wadpart_Init();

	// Move this to right before we start displaying.

	RECT r={0,0,512,512};

	ClearImage(&r,0,0,0);
	
	MDEC_VideoSet(512,DisplayHeight);

#ifndef VERSION_DEMO
	if (wad_level==33)
		Wadmenu_DoCredits(&tim[0]);

	if (Wadmenu_MuckyTime&&(Wadmenu_MuckyTime>(stat_game_time/1000)))
	{
		Wadmenu_Code(&tim[0]);
	}

	if (Wadmenu_CivMess)
	{
		Wadmenu_BadCop(&tim[0],Wadmenu_CivMess);
		Wadmenu_CivMess=NULL;
		if (Wadmenu_Citations==4)
			Wadmenu_Levelwon=0;
	}

	if (Wadmenu_Levelwon)
		menu=EndOfLevelMenu;
#endif

	Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);

	while(!str)
	{
		Wadmenu_ParticleSync(&tim[0]);

		Wadmenu_DrawMenu(menu,selected,stack);

		Wadmenu_DoParticleElement(wad_level);

		if (pad_key==0)
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_LU))
			{
				if (MFX_Conv_playing&&menu[selected].info)
					MFX_Conv_stop();
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				if (selected>0)
					selected--;
				PAD_KEY(5);
			}
			if (PadKeyIsPressed(&PAD_Input1,PAD_LD))
			{
				if (MFX_Conv_playing&&menu[selected].info)
					MFX_Conv_stop();
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				if (!menu[selected].last)
					selected++;
				PAD_KEY(5);
			}
#ifndef FS_ISO9660
			if (PadKeyIsPressed(&PAD_Input1,PAD_RR))
			{
				MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
				AENG_screen_shot(512);
				PAD_KEY(5);
			}
#endif

			switch(menu[selected].type)
			{
			case WADMENU_TYPE_VOID:
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					if (menu[selected].ptr.value)
						menu[selected].ptr.func_void(&menu[selected]);
					PAD_KEY(5);
				} 
				else if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					if (stack)
					{
						Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
						selected=0;
						menu=menu_stack[--stack];
						Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					}
					PAD_KEY(10);
				}
				break;

			case WADMENU_TYPE_STR:
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				{
					if (menu[selected].ptr.value)
					{
						Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
						MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
						str=menu[selected].ptr.func_str(&tim[menu[selected].info]);
#ifndef VERSION_DEMO
						if (str=="LOADED")
						{
							str=NULL;
							menu=EndOfLevelMenu;
							Wadmenu_Levelwon=1;
						}
#endif
						if (str==NULL)
							Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
						PAD_KEY(5);
					}
				}
				else if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					if (stack)
					{
						Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
						selected=0;
						menu=menu_stack[--stack];
						Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					}
					PAD_KEY(10);
				}
#ifndef VERSION_DEMO
				if (PadKeyIsPressed(&PAD_Input1,PAD_FLT)&&
					PadKeyIsPressed(&PAD_Input1,PAD_FRT)&&
					PadKeyIsPressed(&PAD_Input1,PAD_SEL)&&
					PadKeyIsPressed(&PAD_Input1,PAD_START))
				{
					MFX_play_ambient(0,S_DARCI_ARREST,0);
					Wadmenu_CheatLevels(1);
					pad_key=10;
					Wadpart_AddTextParticle(WAD_CHEAT_ON,MENU_TEXTHIGH,256,192,256,192,20,WADMENU_TEXT_CENTRE|WADPART_FLAG_AMBIENT);
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_FLB)&&
					PadKeyIsPressed(&PAD_Input1,PAD_FRB)&&
					PadKeyIsPressed(&PAD_Input1,PAD_SEL)&&
					PadKeyIsPressed(&PAD_Input1,PAD_START))
				{
					MFX_play_ambient(0,S_DARCI_SCREAM_NO_START,0);
					Wadmenu_CheatLevels(0);
					pad_key=10;
					Wadpart_AddTextParticle(WAD_CHEAT_OFF,MENU_TEXTHIGH,256,192,256,192,20,WADMENU_TEXT_CENTRE|WADPART_FLAG_AMBIENT);
				}
#endif
				break;
			case WADMENU_TYPE_MENU:
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				{
					Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					menu_stack[stack++]=menu;
					menu=menu[selected].ptr.menu;
					selected=0;
					Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					PAD_KEY(10);
				}
				else if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					if (stack)
					{
						Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
						selected=0;
						menu=menu_stack[--stack];
						Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					}
					PAD_KEY(10);
				}

#ifdef VERSION_KANJI
				if (PadKeyIsPressed(&PAD_Input1,PAD_RL))
				{
					Kanji_Debug();
				}
#endif
				break;

			case WADMENU_TYPE_SLIDER:
				// Hack, made -1 represent music (which is of course already playing)
				if (menu[selected].info>0)
				{
					if (!MFX_Conv_playing)
						MFX_Conv_play(menu[selected].info,0,0);
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_LL))
				{
					if (*menu[selected].ptr.value>0)
						*menu[selected].ptr.value-=8;
					// Only play SFX if we're showing sfx volume slider
					if (menu[selected].info==0)
						MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					PAD_KEY(2);
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_LR))
				{
					if (*menu[selected].ptr.value<248)
						*menu[selected].ptr.value+=8;
					// Only play SFX if we're showing sfx volume slider
					if (menu[selected].info==0)
						MFX_play_ambient(0,S_PISTOL_DRY,MFX_REPLACE);
					PAD_KEY(2);
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					if (MFX_Conv_playing)
						MFX_Conv_stop();
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					if (stack)
					{
						Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
						selected=0;
						menu=menu_stack[--stack];
						Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					}
					PAD_KEY(10);
				}
				break;
			case WADMENU_TYPE_BACK:
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				{
					Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					menu=menu_stack[--stack];
					selected=0;
					Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					PAD_KEY(10);
				}
				if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
				{
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					if (stack)
					{
						Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
						selected=0;
						menu=menu_stack[--stack];
						Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					}
					PAD_KEY(10);
				}
				break;
			case WADMENU_TYPE_MENUNORET:
				if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
				{
					Wadmenu_ParticleEffect(WADMENU_MENU_OUT,(SLONG)menu,selected);
					MFX_play_ambient(0,S_PISTOL_DRY,0);
					if (Wadmenu_DoYorN(&tim[0],WAD_MENU_ENDGAME))
					{
						menu=menu[selected].ptr.menu;
						selected=0;
						Wadmenu_Levelwon=0;
					}
					Wadmenu_ParticleEffect(WADMENU_MENU_IN,(SLONG)menu,selected);
					PAD_KEY(10);
				}
				break;
			}
		}
		else
			pad_key--;

		Wadmenu_DrawIconicStuff(WADMENU_MENU_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim[0].paddr);

		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;

		if (Wadmenu_Levelwon)
			time_out++;

		if (GAME_TURN==time_out)
		{
			Video_Played=0;
			str="MDEC";
		}
	}

	// Crappy sync to sync out the end without adding any new particle effects

	while(!Wadpart_Sync())
	{
		Wadpart_Render();
		AENG_flip2(tim[0].paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
	}

	MemFree(Wadmenu_district);
	Wadmenu_ClearWads();
	PAD_Current=pad_cfg[pad_config];
	GAME_TURN=0;
	Wadmenu_Levelwon=0;
	MemFree(Back_Image);
	MemFree(Wadmenu_credit);

	if (strlen(str)>6)
	{
#if !defined(MIKE)&&!defined(VERSION_DEMO)
		if (wad_level==24)
			Wadmenu_PlayFMV(WADMENU_FMV_CUT1);
		if (wad_level==33)
			Wadmenu_PlayFMV(WADMENU_FMV_CUT3);
#endif
		Wadmenu_LoadingScreen(tim);
	}
	MDEC_VideoSet(DisplayWidth,DisplayHeight);

#ifdef VERSION_KANJI
	Kanji_Init(320,0);
#endif
	MUSIC_stop(false);
	MUSIC_mode_process();
	MFX_render();
	return str;
}

void Wadmenu_Introduction(void)
{
   	RECT r={0,0,512,512};

#ifndef VERSION_DEMO
	Wadmenu_AutoLoadInfo();
#endif
	vibra_mode=0;
void	InitVideo(void);
	InitVideo();

	ClearImage(&r,0,0,0);

extern void Host_VbRoutine();

	OpenDisplay(640,480,16,FLAGS_USE_3D|FLAGS_USE_WORKSCREEN);
	VSyncCallback(Host_VbRoutine);

extern UBYTE my_heap[];

	GDisp_SetupBucketMem(&my_heap[OVERLAY_SIZE],65536);
	XLAT_load(LANG_FILE_NAME);
	XLAT_init();
}

#define SAVE_MODE_EXIT		-1
#define SAVE_MODE_START		0
#define SAVE_MODE_CHECK		1
#define SAVE_MODE_NOCARD	2
#define SAVE_MODE_FIND		3
#define SAVE_MODE_FORMAT	4
#define SAVE_MODE_SAVEYORN	5
#define SAVE_MODE_SAVEWAIT	6
#define SAVE_MODE_DONE		7
#define SAVE_MODE_FINISH	8
#define SAVE_MODE_FULL		9
#define SAVE_MODE_NOSAVE	10
#define SAVE_MODE_ERROR		11
#define SAVE_MODE_RESTART	12
#define SAVE_MODE_DOFORM	13
#define SAVE_MODE_CREATE	14
#define SAVE_MODE_DOSAVE	15
#define SAVE_MODE_CREATE2	16
#define SAVE_MODE_DOFORM2	17

#define LOAD_MODE_EXIT		-1
#define LOAD_MODE_START		0
#define LOAD_MODE_CHECK		1
#define LOAD_MODE_NOCARD	2
#define LOAD_MODE_FIND		3
#define LOAD_MODE_FORMAT	4
#define LOAD_MODE_LOADYORN	5
#define LOAD_MODE_LOADWAIT	6
#define LOAD_MODE_DONE		7
#define LOAD_MODE_FINISH	8
#define LOAD_MODE_FULL		9
#define LOAD_MODE_NOSAVE	10
#define LOAD_MODE_ERROR		11
#define LOAD_MODE_RESTART	12

#define CHECK_INIT			0
#define CHECK_FIND_CARD		1
#define CHECK_CHECK_SPACE	2
#define CHECK_CHECK_EXISTS	3
#define CHECK_GIVE_RESULTS	4
#define CHECK_RETRY_START	5
#define CHECK_RETRY_WAIT	6
#define CHECK_LOAD_WAIT		7

#define SAVE_SHORT(s) *save_ptr++=(s)
#define SAVE_BLK(s,t) memcpy((void*)save_ptr,(void*)s,sizeof(t)); save_ptr+=sizeof(t)/2
#define LOAD_SHORT(s) s=*load_ptr++
#define LOAD_BLK(s,t) memcpy((void*)s,(void*)load_ptr,sizeof(t)); load_ptr+=sizeof(t)/2

/*
SLONG Wadmenu_MCARD_Checking()
{
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_CHECK),MENU_TEXTHIGH,WADMENU_TEXT_MCARD|WADMENU_TEXT_BIG);
	return MCARD_Status();
}

SLONG Wadmenu_MCARD_DoOrExit(SLONG mode,SLONG message1,SLONG message2,SLONG modeyes,SLONG modeno)
{
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(message1),MENU_TEXTHIGH,WADMENU_TEXT_MCARD|WADMENU_TEXT_BIG);
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y2,W_(message2),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);

	if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		return modeyes;
	if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		return modeno;

	return mode;
}

SLONG Wadmenu_MCARD_DoTwoLine(SLONG mode,SLONG message1,SLONG message2,SLONG message3,SLONG message4,SLONG modeyes,SLONG modeno)
{
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(message1),MENU_TEXTHIGH,WADMENU_TEXT_MCARD|WADMENU_TEXT_BIG);
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1+24,W_(message2),MENU_TEXTHIGH,WADMENU_TEXT_MCARD|WADMENU_TEXT_BIG);
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1+48,W_(message3),MENU_TEXTHIGH,WADMENU_TEXT_MCARD|WADMENU_TEXT_BIG);
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y2+32,W_(message4),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);

	if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		return modeyes;
	if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		return modeno;

	return mode;
}

SLONG Wadmenu_MCARD_YorN(SLONG mode,SLONG message,SLONG modeyes,SLONG modeno)
{
	return Wadmenu_MCARD_DoOrExit(mode,message,WAD_MEM_YORN,modeyes,modeno);
}

SLONG Wadmenu_MCARD_SaveMode(SLONG status,SLONG inmode)
{
	SLONG mode=inmode;
		switch(MCARD_STATUS(status))
		{
		case MCARD_NOT_AVAILABLE:
		case MCARD_NO_CARD:
			mode=SAVE_MODE_NOCARD;
			break;
		case MCARD_OKAY:
			switch(status)
			{
			case MCARD_OKAY:
				mode=SAVE_MODE_FIND;
				break;
			case MCARD_WRITE_OKAY:
				mode=SAVE_MODE_DONE;
				break;
			case MCARD_FIND_OKAY:
				mode=SAVE_MODE_EXISTS;
				break;
			case MCARD_SPACE_OKAY:
				mode=SAVE_MODE_SAVEYORN;
			}
			break;
		case MCARD_NOT_FORMATTED:
			mode=SAVE_MODE_FORMAT;
			break;
		case MCARD_NOT_FOUND:
			mode=SAVE_MODE_SPACE;
			break;
		case MCARD_EXISTS:
			mode=SAVE_MODE_EXISTS;
			break;
		case MCARD_FULL:
			mode=SAVE_MODE_FULL;
			break;
		case MCARD_ERROR:
			mode=SAVE_MODE_ERROR;
			break;
		case MCARD_NEW_CARD:
			mode=SAVE_MODE_RESTART;
			break;
		}
	return mode;
}
*/

#ifndef VERSION_DEMO

#define WAD_FILE_NAME "B" AREA_CODE PRODUCT_CODE "URBANSV"

/*
SLONG Wadmenu_MCARD_FindFile(SLONG status)
{
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_CHECK),MENU_TEXTHIGH,WADMENU_TEXT_MCARD|WADMENU_TEXT_BIG);
	draw_centre_text_at(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
	if (status==MCARD_OKAY)
	{
		return MCARD_FindFile(WAD_FILE_NAME);
	} else
		return MCARD_Status();
}
*/

int check_mode;

void Wadmenu_MCARD_CheckMode(SLONG mode)
{
	check_mode=mode;
}


SLONG Wadmenu_MCARD_DoSave(SWORD *buffer)
{
	SWORD *save_ptr=buffer;
	SLONG i,version=CURRENT_VERSION;
	SLONG func,res;

	SAVE_BLK(MCARD_Header,MCARD_Header);
	SAVE_BLK(MCARD_Clut,MCARD_Clut);
	SAVE_BLK(MCARD_Icon,MCARD_Icon);

	SAVE_SHORT(version);
	SAVE_SHORT(sfx_volume);
	SAVE_SHORT(music_volume);
	SAVE_SHORT(speech_volume);
	SAVE_SHORT(sound_mode);
	SAVE_SHORT(pad_config);
	SAVE_SHORT(vibra_mode);
	SAVE_SHORT(screen_x);
	SAVE_SHORT(screen_y);
	SAVE_SHORT(Wadmenu_Citations);

	SAVE_BLK(&pad_free,PadInfo);

	for(i=0;i<MAX_LEVELS;i++)
	{
		SAVE_SHORT(level_done[i]);
	}

	SAVE_SHORT(Wadmenu_Current_Con);
	SAVE_SHORT(Wadmenu_Current_Ref);
	SAVE_SHORT(Wadmenu_Current_Sta);
	SAVE_SHORT(Wadmenu_Current_Str);

	Wadmenu_MCARD_CheckMode(CHECK_LOAD_WAIT);

	MemCardSync(0,&func,&res);
	MemCardWriteFile(0x00,WAD_FILE_NAME,(ULONG*)buffer,0,1024);
}

SLONG Wadmenu_MCARD_DoLoad(SWORD *buffer)
{
	SLONG func,res;

	Wadmenu_MCARD_CheckMode(CHECK_LOAD_WAIT);
	MemCardSync(0,&func,&res);
	MemCardReadFile(0x00,WAD_FILE_NAME,(ULONG*)buffer,0,1024);
}

void Wadmenu_MCARD_EndLoad(SWORD *buffer)
{
	SWORD *load_ptr=&buffer[128];
	SLONG i,pad_value,save_version;

	LOAD_SHORT(save_version);
	if (save_version!=CURRENT_VERSION)
	{
		// This will not be a problem for the actual players
		// Since they wont even notice this (the full game
		// will never have more than one version number)
		memset((void*)level_done,Wadmenu_Cheat,48);
		level_done[0]=1;
		Wadmenu_InitStats();
		return;
	}
	LOAD_SHORT(sfx_volume);
	LOAD_SHORT(music_volume);
	LOAD_SHORT(speech_volume);
	LOAD_SHORT(sound_mode);
	LOAD_SHORT(pad_config);
	LOAD_SHORT(vibra_mode);

	LOAD_SHORT(screen_x);
	LOAD_SHORT(screen_y);
	LOAD_SHORT(Wadmenu_Citations);

	LOAD_BLK(&pad_free,PadInfo);

	for(i=0;i<MAX_LEVELS;i++)
	{
		LOAD_SHORT(level_done[i]);
	}

	the_display.DisplayBuffers[0].Disp.screen.x=screen_x;
	the_display.DisplayBuffers[1].Disp.screen.x=screen_x;
	the_display.DisplayBuffers[0].Disp.screen.y=screen_y;
	the_display.DisplayBuffers[1].Disp.screen.y=screen_y;

	LOAD_SHORT(Wadmenu_Current_Con);
	LOAD_SHORT(Wadmenu_Current_Ref);
	LOAD_SHORT(Wadmenu_Current_Sta);
	LOAD_SHORT(Wadmenu_Current_Str);

	AudioMenu[0].name=audio_title[sound_mode];
}

// Wadmenu_MCARD_Check
//
// This is the core of the new system, everything goes through here
// this checks everything about the card before allowing the user
// to continue with anything.

struct DIRENTRY MCARD_dir[15];
SLONG MCARD_files;

SLONG Wadmenu_MCARD_Check(SLONG mode)
{
	static int space;
	static int exists;
	SLONG func,res,i;

	switch(mode)
	{
	case	LOAD_MODE_START:
		check_mode=CHECK_INIT;
		break;
	case	LOAD_MODE_DONE:
	case	LOAD_MODE_FINISH:
	case	LOAD_MODE_ERROR:
		// Added the following below so they can't be interupted by memory card
		// not formatted messages.
	case	SAVE_MODE_DOFORM:
	case	SAVE_MODE_DOFORM2:
	case	SAVE_MODE_CREATE:
	case	SAVE_MODE_CREATE2:
		return mode;
	}
 
	switch(check_mode)
	{
	case	CHECK_INIT:
		space=15;
		exists=0;
		if (!MemCardAccept(0x00))
			ASSERT(0);
		check_mode=CHECK_FIND_CARD;
	case	CHECK_FIND_CARD:
		switch(MemCardSync(1,&func,&res))
		{
		case	0:
			return LOAD_MODE_CHECK;
		case	1:
			switch(res)
			{
			case 0:
				check_mode=CHECK_CHECK_SPACE;
				break;
			case 1:
				check_mode=CHECK_RETRY_START;
				return LOAD_MODE_NOCARD;
			case 2:
				check_mode=CHECK_RETRY_START;
				return LOAD_MODE_NOCARD;
			case 3:
				check_mode=CHECK_INIT;
				break;
			case 4:
				check_mode=CHECK_RETRY_START;
				return LOAD_MODE_FORMAT;
			}
			break;
		case	-1:
			ASSERT(0);
		}
	case	CHECK_CHECK_SPACE:
		switch(MemCardGetDirentry(0x00,"*",MCARD_dir,&MCARD_files,0,15))
		{
		case	0:
			for(i=0;i<MCARD_files;i++)
				space-=(MCARD_dir[i].size+8191)>>13;
			check_mode=CHECK_CHECK_EXISTS;
			break;
		case	1:
			check_mode=CHECK_RETRY_START;
			return LOAD_MODE_NOCARD;
		case	2:
			check_mode=CHECK_RETRY_START;
			return LOAD_MODE_NOCARD;
		case	3:
			check_mode=CHECK_RETRY_START;
			return LOAD_MODE_FORMAT;
		}
		break;
	case	CHECK_CHECK_EXISTS:
		switch(MemCardGetDirentry(0x00,WAD_FILE_NAME,MCARD_dir,&MCARD_files,0,15))
		{
		case	0:
			exists=MCARD_files;
			check_mode=CHECK_GIVE_RESULTS;
			break;
		case	1:
			check_mode=CHECK_RETRY_START;
			return LOAD_MODE_NOCARD;
		case	2:
			check_mode=CHECK_RETRY_START;
			return LOAD_MODE_NOCARD;
		case	3:
			check_mode=CHECK_RETRY_START;
			return LOAD_MODE_FORMAT;
		}
		break;
	case	CHECK_GIVE_RESULTS:
		check_mode=CHECK_RETRY_START;
		if (exists)
			return LOAD_MODE_LOADYORN;
		if (space==0)
			return LOAD_MODE_FULL;
		return LOAD_MODE_NOSAVE;
	case	CHECK_RETRY_START:
		if (!MemCardExist(0x00))
			ASSERT(0);
		check_mode=CHECK_RETRY_WAIT;
	case	CHECK_RETRY_WAIT:
		switch(MemCardSync(1,&func,&res))
		{
		case 0:
			break;
		case 1:
			switch(res)
			{
			case	0:
				check_mode=CHECK_RETRY_START;
				break;
			case	1:
				check_mode=CHECK_RETRY_START;
				return	LOAD_MODE_NOCARD;
			case	2:
				check_mode=CHECK_RETRY_START;
				return	LOAD_MODE_ERROR;
			case	3:
				check_mode=CHECK_INIT;
				return	LOAD_MODE_CHECK;
			}
		}
	case	CHECK_LOAD_WAIT:
		switch(MemCardSync(1,&func,&res))
		{
		case	0:
			break;
		case	1:
			switch(res)
			{
			case	0:
				return LOAD_MODE_DONE;
			case	1:
				check_mode=CHECK_RETRY_START;
				return LOAD_MODE_ERROR;
			case	2:
				check_mode=CHECK_RETRY_START;
				return LOAD_MODE_ERROR;
			case	3:
				check_mode=CHECK_INIT;
				return LOAD_MODE_CHECK;
			case	4:
				check_mode=CHECK_RETRY_START;
				return LOAD_MODE_NOSAVE;
			}
		}
	}
	return mode;
}

// Two new functions for memory card handling these check
// the controller button presses and set the mode dependant
// on weither it is pressed or not.

SWORD	MCard_message;

SLONG Wadmenu_MCARD_Select(SLONG mode,SLONG selmode)
{
	static holding=1;

	if (MCard_message==0)
		MCard_message=WAD_SEL_SELECT;
	else
		MCard_message=WAD_SEL_CAN;

	if (holding)
	{
		if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
			holding=0;
	}
	else
	{
		if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		{
			holding=1;
			return selmode;
		}
	}
	return mode;
}

SLONG Wadmenu_MCARD_Cancel(SLONG mode,SLONG canmode)
{
	static holding=1;

	if (MCard_message==0)
		MCard_message=WAD_CAN_CANCEL;
	else
		MCard_message=WAD_SEL_CAN;
	if (holding)
	{
		if (!PadKeyIsPressed(&PAD_Input1,PAD_RU))
			holding=0;
	}
	else
	{
		if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		{
			holding=1;
			return canmode;
		}
	}
	return mode;
}

SLONG Wadmenu_MCARD_YorN(SLONG mode,SLONG selmode,SLONG canmode)
{
	MCard_message=0;
	static holding=1;

	if (holding)
	{
		if (!PadKeyIsPressed(&PAD_Input1,PAD_RU)&&!PadKeyIsPressed(&PAD_Input1,PAD_RD))
			holding=0;
	}
	else
	{
		if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		{
			holding=1;
			return canmode;
		}
		if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		{
			holding=1;
			return selmode;
		}
	}
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_YORN),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
	return mode;
}

#ifdef VERSION_NTSC
SLONG Wadmenu_MCARD_NorY(SLONG mode,SLONG selmode,SLONG canmode)
{
	MCard_message=0;
	static holding=1;

	if (holding)
	{
		if (!PadKeyIsPressed(&PAD_Input1,PAD_RU)&&!PadKeyIsPressed(&PAD_Input1,PAD_RD))
			holding=0;
	}
	else
	{
		if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		{
			holding=1;
			return canmode;
		}
		if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		{
			holding=1;
			return selmode;
		}
	}
	// Bodge this line to have the correct text since it's only for American we can make this
	// in english only, saves pissing about with wadlang files now.
	Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,STR_TRI" Yes, "STR_CROSS" No",MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
	return mode;
}
#endif

SLONG Wadmenu_MCARD_Create(SLONG mode,char *fname)
{
	SLONG res;

	res=MemCardCreateFile(0x00,fname,1);
	switch(res)
	{
	case 0:
		return mode;
	case 1:
		return SAVE_MODE_NOCARD;
	case 2:
		return SAVE_MODE_ERROR;
	case 4:
		return SAVE_MODE_FORMAT;
	case 6:
		return mode;
	case 7:
		return SAVE_MODE_FULL;
	}
	return mode;
}

SLONG Wadmenu_MCARD_Format(SLONG mode)
{
	SLONG res;
	res=MemCardFormat(0x00);
	switch(res)
	{
	case 0:
		return mode;
	case 1:
		return SAVE_MODE_NOCARD;
	case 2:
		return SAVE_MODE_ERROR;
	}
	return mode;
}


// Save game.

char *Wadmenu_SaveGame(TIM_IMAGE *tim)
{

	SLONG	mode=SAVE_MODE_START;
	SLONG	status;
	SWORD	*Wadmenu_MCARD_Buffer=(SWORD*)MemAlloc(1024);
	SWORD	First_Change=1;
	SLONG	func,res;
	
	Wadmenu_ParticleSync(tim);

	while (mode!=SAVE_MODE_EXIT)
	{
		Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(WAD_MENU_SAVE),MENU_TEXTHIGH,WADMENU_TEXT_ITEM);
		Wadmenu_DoParticleElement(wad_level);

		MCard_message=0;

		mode=Wadmenu_MCARD_Check(mode);

		switch(mode)
		{
 		case	SAVE_MODE_CHECK:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_CHECK),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			mode=Wadmenu_MCARD_Cancel(mode,SAVE_MODE_EXIT);
			break;
		case	SAVE_MODE_NOCARD:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_NOCARD),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_Cancel(mode,SAVE_MODE_EXIT);
			break;
		case	SAVE_MODE_FORMAT:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_NOFORM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y2,W_(WAD_MEM_FORMAT),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
#ifdef VERSION_NTSC
			mode=Wadmenu_MCARD_NorY(mode,SAVE_MODE_DOFORM,SAVE_MODE_EXIT);
#else
			mode=Wadmenu_MCARD_YorN(mode,SAVE_MODE_DOFORM,SAVE_MODE_EXIT);
#endif
			if (mode==SAVE_MODE_DOFORM)
			{
				Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			}
			break;
		case	SAVE_MODE_SAVEYORN:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_EXISTS),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_YorN(mode,SAVE_MODE_DOSAVE,SAVE_MODE_EXIT);
			if (mode==SAVE_MODE_DOSAVE)
			{
				Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			}
			break;
		case	SAVE_MODE_SAVEWAIT:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVE),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			break;
		case	SAVE_MODE_DONE:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVEOK),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			mode=Wadmenu_MCARD_Select(mode,SAVE_MODE_EXIT);
			break;
		case	SAVE_MODE_FULL:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_FULL),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			mode=Wadmenu_MCARD_Cancel(mode,SAVE_MODE_EXIT);
			break;
		case	SAVE_MODE_ERROR:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_ERROR1),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_Select(mode,SAVE_MODE_START);
			mode=Wadmenu_MCARD_Cancel(mode,SAVE_MODE_EXIT);
			break;
		case	SAVE_MODE_DOFORM:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_DOFORM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			mode=SAVE_MODE_DOFORM2;
			break;
		case	SAVE_MODE_DOFORM2:
			mode=Wadmenu_MCARD_Format(SAVE_MODE_CREATE);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVE),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			break;
		case	SAVE_MODE_NOSAVE:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVEYN),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_YorN(mode,SAVE_MODE_CREATE,SAVE_MODE_EXIT);
			if (mode==SAVE_MODE_CREATE)
			{
				Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			}
			break;
		case	SAVE_MODE_CREATE:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVE),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			mode=SAVE_MODE_CREATE2;
			break;
		case	SAVE_MODE_CREATE2:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVE),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);

			if (MemCardSync(1,&func,&res)==0)
				break;
			mode=Wadmenu_MCARD_Create(SAVE_MODE_SAVEWAIT,WAD_FILE_NAME);
			if (mode!=SAVE_MODE_SAVEWAIT)
				break;
			Wadmenu_MCARD_DoSave(Wadmenu_MCARD_Buffer);
			mode=SAVE_MODE_SAVEWAIT;
			break;
		case	SAVE_MODE_DOSAVE:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_SAVE),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			
			if (MemCardSync(1,&func,&res)==0)
				break;
			Wadmenu_MCARD_DoSave(Wadmenu_MCARD_Buffer);
			mode=SAVE_MODE_SAVEWAIT;
			break;
		}
		if (MCard_message)
			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(MCard_message),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);

		Wadmenu_DrawIconicStuff(WADMENU_MENU_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}
	MemFree(Wadmenu_MCARD_Buffer);
	GAME_TURN=0;
	return 0;

}

// Load game, now rewritten to be much less modal and much more flexible.

char *Wadmenu_LoadGame(TIM_IMAGE *tim)
{
	SLONG	mode=LOAD_MODE_START;
	SLONG	status,loaded=0;
	SWORD	*Wadmenu_MCARD_Buffer=(SWORD*)MemAlloc(1024);
	
//	MCARD_Init();

	Wadmenu_ParticleSync(tim);

	while (mode!=LOAD_MODE_EXIT)
	{
		Wadmenu_DoParticleElement(wad_level);
		Wadmenu_draw_text(WADMENU_TITLE_X,WADMENU_TITLE_Y,W_(WAD_MENU_LOAD),MENU_TEXTHIGH,WADMENU_TEXT_ITEM);
		
		MCard_message=0;

		mode=Wadmenu_MCARD_Check(mode);

		switch(mode)
		{
		case	LOAD_MODE_CHECK:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_CHECK),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			mode=Wadmenu_MCARD_Cancel(mode,LOAD_MODE_EXIT);
			break;
		case	LOAD_MODE_NOCARD:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_NOCARD),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_Cancel(mode,LOAD_MODE_EXIT);
			break;
		case	LOAD_MODE_FORMAT:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_NOFORM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_Cancel(mode,LOAD_MODE_EXIT);
			break;
		case	LOAD_MODE_LOADYORN:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_LOADYN),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_YorN(mode,LOAD_MODE_LOADWAIT,LOAD_MODE_EXIT);
			if (mode==LOAD_MODE_LOADWAIT)
				Wadmenu_MCARD_DoLoad(Wadmenu_MCARD_Buffer);
			break;
		case	LOAD_MODE_LOADWAIT:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_LOAD),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y3,W_(WAD_MEM_DONTREM),MENU_TEXTHIGH,WADMENU_TEXT_MCARD);
			break;
		case	LOAD_MODE_DONE:
			Wadmenu_MCARD_EndLoad(Wadmenu_MCARD_Buffer);
			mode=LOAD_MODE_FINISH;
		case	LOAD_MODE_FINISH:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_LOADOK),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			mode=Wadmenu_MCARD_Select(mode,LOAD_MODE_EXIT);
			loaded=1;
			break;
		case	LOAD_MODE_FULL:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_FULL),MENU_TEXTHIGH,WADMENU_TEXT_MCARD3);
			mode=Wadmenu_MCARD_Cancel(mode,LOAD_MODE_EXIT);
			break;
		case	LOAD_MODE_NOSAVE:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_NOSAVE),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_Cancel(mode,LOAD_MODE_EXIT);
			break;
		case	LOAD_MODE_ERROR:
			Wadmenu_draw_text(WADMENU_MCARD_X,WADMENU_MCARD_Y1,W_(WAD_MEM_ERROR2),MENU_TEXTHIGH,WADMENU_TEXT_MCARD2);
			mode=Wadmenu_MCARD_Select(mode,LOAD_MODE_START);
			mode=Wadmenu_MCARD_Cancel(mode,LOAD_MODE_EXIT);
			break;
		}
		if (MCard_message)
			Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(MCard_message),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);

		Wadmenu_DrawIconicStuff(WADMENU_MENU_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		MFX_render();
		GAME_TURN++;
	}

	MemFree(Wadmenu_MCARD_Buffer);
//	MCARD_Final();
	GAME_TURN=0;
	if (loaded)
		return "LOADED";
	else
		return NULL;
}
#endif

void Wadmenu_InitStats()
{
	Wadmenu_Citations=0;
	Wadmenu_Current_Con=0;
	Wadmenu_Current_Ref=0;
	Wadmenu_Current_Sta=0;
	Wadmenu_Current_Str=0;
}

#ifndef VERSION_DEMO
void Wadmenu_AutoLoadInfo()
{
	ULONG func,res;
	SWORD *buffer;

#ifdef VERSION_PAL
// Hack around this
#if	!defined(MIKE)&&!defined(VERSION_DEMO)
	func=0;
	CdControl(CdlSetmode,(unsigned char*)&func,0);
	VSync(6);
	ReadCrypt();

	while(!GOTKEY) VSync(0);
#endif
#endif

	buffer=(SWORD*)MemAlloc(1024);

	do {
		MemCardAccept(0x00);

		MemCardSync(0,&func,&res);
	} while (res==0x03);

	// If we have an error exit, we're autoloading here.
	if (res!=0x00)
		return;

	MemCardReadFile(0x00,WAD_FILE_NAME,(ULONG*)buffer,0,1024);

	MemCardSync(0,&func,&res);

	// Once again any errors aren't going to show up here, we're doing
	// this anonymously (the game doesnt require a memory card, but will
	// remember if you have a saved game.
	if (res!=0x00)
		return;
	Wadmenu_MCARD_EndLoad(buffer);
	MemFree((void*)buffer);

}


void Wadmenu_Encrypt(CBYTE *code, SLONG level)
{
	SLONG i;
	CBYTE *pack1,*pack2,*pack;
	SLONG seed;

	// Build a random seed specifically for this level

	seed=level+((31^level)*35)+(level*35*35);
	seed+=seed*35*35*35;

	SetSeed(seed);

	// Encryption of the code goes here.

	for(i=0;i<10;i++)
		code[i]=(code[i]+Random())%26;


	// Encode back into ascii

	for(i=0;i<10;i++)
		code[i]+=65;

	// Finally split into two groups of 5 characters

	for(i=9;i>4;i--)
		code[i+1]=code[i];
	code[5]=' ';
	code[11]=0;
}

void Wadmenu_GenerateCode(CBYTE *code,SLONG level,SLONG time)
{
	SLONG h,m,s,i;

	// First we need to build the string to encrypt.

	h=time/3600;
	m=(time/60)%60;
	s=time%60;
	sprintf(code,"%d:%02d:%02d:%c%c",h,m,s,((h+m+s)%26),((h^m^s)&0x0f));

	// We want the numbers to become alpha characters now.

	for(i=0;i<8;i++)
	{
		code[i]-=48;
	}

	// Now we can encrypt it here, based on the level number
	Wadmenu_Encrypt(code,level);
}

void Wadmenu_Code(TIM_IMAGE *tim)
{
	char code[12];
	//char str[40];

	int awaiting=1;

	Wadmenu_GenerateCode(code,wad_level,stat_game_time/1000);

	Wadmenu_ParticleSync(tim);
	Wadmenu_ParticleEffect(WADMENU_BRIEF_IN,0,0);

	while(1)
	{
		Wadmenu_ParticleSync(tim);
		Wadmenu_DoParticleElement(wad_level);

		Wadmenu_draw_text(256,32,W_(WAD_CODE_CON),MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_CENTRE);
		Wadmenu_draw_text(160,96,W_(WAD_CODE_BEATEN),MENU_TEXTHIGH,WADMENU_TEXT_CENTRE);
		Wadmenu_draw_text(160,124,W_(WAD_CODE_CODE),MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_CENTRE);
		Wadmenu_draw_text(160,160,code,MENU_TEXTHIGH,WADMENU_TEXT_BIG|WADMENU_TEXT_CENTRE);

		Wadmenu_draw_text(WADMENU_KEYS_X,WADMENU_KEYS_Y,W_(WAD_SEL_SELECT),MENU_TEXTHIGH,WADMENU_TEXT_KEYS);

		if (awaiting)
		{
			if (!PadKeyIsPressed(&PAD_Input1,PAD_RD))
				awaiting=0;
		}
		else
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
			{
				GAME_TURN=0;
				Wadmenu_ParticleEffect(WADMENU_BRIEF_OUT,0,0);
				return;
			}
		}
		MUSIC_mode(WADMENU_MUSIC_MODE);
		MUSIC_mode_process();
		Wadmenu_DrawIconicStuff(WADMENU_MISSION_SCREEN);
		Wadpart_Render();
		AENG_flip2(tim->paddr);
		GAME_TURN++;
	}
}
#endif