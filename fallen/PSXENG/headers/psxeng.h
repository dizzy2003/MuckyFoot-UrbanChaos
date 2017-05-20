//
// Another engine.
//

#ifndef _PSXENG_H
#define _PSXENG_H
#define USE_A3D

#ifdef VERSION_DPC
#define VERSION_ENGLISH
#define VERSION_DEMO
#define VERSION_GERMAN
#endif

#ifdef VERSION_DCD
#define VERSION_ENGLISH
#define VERSION_DEMO
#define FS_ISO9660
#define VERSION_GERMAN
#endif

#ifdef VERSION_RPC
#define VERSION_ENGLISH
#define VERSION_REVIEW
#endif

#ifdef VERSION_RCD
#define VERSION_ENGLISH
#define VERSION_REVIEW
#define FS_ISO9660
#endif

#ifdef VERSION_RIC
#define VERSION_ITALIAN
#define VERSION_REVIEW
#define FS_ISO9660
#endif

#ifdef VERSION_RSC
#define VERSION_SPANISH
#define VERSION_REVIEW
#define FS_ISO9660
#endif

#ifdef VERSION_JCD
#define VERSION_JAPAN
#define FS_ISO9660
#endif

#ifdef VERSION_KCD
#define VERSION_KOREA
#define FS_ISO9660
#endif

#ifdef FS_CD
#undef FS_CD
#define FS_ISO9660
#define FINAL
#endif

#define BUILD_PSX
#define VERSION_LORES
#define NDEBUG

#ifdef VERSION_GERMAN
#define AREA_CODE "E"
#define PRODUCT_CODE "SLES-02355"
#define VERSION_PAL
#define DEFAULT_LANGUAGE	4
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\german.txt"
#define LANG_ROOT_NAME "german"
#define LOADING_NAME "DATA\\LOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.BIN"
#ifndef VERSION_DEMO
#define LEVEL_WON "Level abgeschlossen"
#define LEVEL_LOST "Level verloren"
#define SEARCHING "Suche läuft"
#define COMPLETE ""
#define CONFIG_CANCEL "Drücken Sie L1, R1, START und SELECT, um abzubrechen"
#define NEW_LEVELS 1
#endif
#endif

#ifdef VERSION_FRENCH
#define AREA_CODE "E"
#define PRODUCT_CODE "SLES-02354"
#define VERSION_PAL
#define DEFAULT_LANGUAGE	1
#define VERSION_GERMAN
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\french.txt"
#define LANG_ROOT_NAME "french"
#define LOADING_NAME "DATA\\LOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.BIN"
#define LEVEL_WON "Niveau terminé"
#define LEVEL_LOST "Niveau perdu"
#define SEARCHING "Recherche"
#define COMPLETE ""
#define CONFIG_CANCEL "Appuyez sur L1, R1, START et SELECT pour annuler"
#define NEW_LEVELS 1
#endif

#ifdef VERSION_SPANISH
#define AREA_CODE "E"
#define PRODUCT_CODE "SLES-02071"
#define VERSION_PAL
#define DEFAULT_LANGUAGE	2
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\spanish.txt"
#define LANG_ROOT_NAME "spanish"
#define LOADING_NAME "DATA\\SOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.SPN"
#define LEVEL_WON "Nivel completo"
#define LEVEL_LOST "Nivel fallado"
#define SEARCHING "Registrando"
#define COMPLETE "Completo"
#define CONFIG_CANCEL "Pulsa L1+R1+START+SELECT para cancelar"
#endif

#ifdef VERSION_ITALIAN
#define AREA_CODE "E"
#define PRODUCT_CODE "SLES-02071"
#define VERSION_PAL
#define DEFAULT_LANGUAGE	3
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\italian.txt"
#define LANG_ROOT_NAME "italian"
#define LOADING_NAME "DATA\\IOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.ITL"
#define LEVEL_WON "Livello completato"
#define LEVEL_LOST "Livello fallito"
#define SEARCHING "Ricerca"
#define COMPLETE "completato"
#define CONFIG_CANCEL "Premi L1+R1+START+SELECT per annullare"
#endif


#ifdef VERSION_USA
#define AREA_CODE "A"
#define PRODUCT_CODE "SLUS-01019"
#define VERSION_NTSC
#define DEFAULT_LANGUAGE	0
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\english.txt"
#define LANG_ROOT_NAME "urbana"
#define LOADING_NAME "DATA\\LOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.BIN"
#define CONFIG_CANCEL "Press L1, R1, START and SELECT to cancel"
#endif

#ifdef VERSION_JAPAN
#define AREA_CODE "I"
#define PRODUCT_CODE "SLES-02071"
#define VERSION_NTSC
#define DEFAULT_LANGUAGE	0
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\japan.txt"
#define LANG_ROOT_NAME "urban"
#define LOADING_NAME "DATA\\LOADING%d.TIM"
#define CONFIG_CANCEL "Press L1, R1, START and SELECT to cancel"
#define OVERLAY_NAME "FRONTEND.BIN"
// Until we get Kanji back in and we know what's going on with it, we'll undefine it
// so that it wont do any good.
#undef VERSION_KANJI
#endif

#ifdef VERSION_KOREA
#define AREA_CODE "I"
#define PRODUCT_CODE "SLES-02071"
#define VERSION_NTSC
#define VERSION_DEFINED
#define VERSION_GERMAN		// HACKY: Makes the game think it's running German Version
#define DEFAULT_LANGUAGE	0
#define LANG_FILE_NAME "data\\japan.txt"
#define LANG_ROOT_NAME "urban"
#define LOADING_NAME "DATA\\LOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.BIN"
#define CONFIG_CANCEL "Press L1, R1, START and SELECT to cancel"
// Until we get Kanji back in and we know what's going on with it, we'll undefine it
// so that it wont do any good.
#undef VERSION_KANJI
#endif

#ifdef VERSION_ENGLISH
#define AREA_CODE "E"
#define PRODUCT_CODE "SLES-02071"
#define VERSION_PAL
#define DEFAULT_LANGUAGE	0
#define VERSION_DEFINED
#define LANG_FILE_NAME "data\\english.txt"
#define LANG_ROOT_NAME "urban"
#define LOADING_NAME "DATA\\LOADING%d.TIM"
#define OVERLAY_NAME "FRONTEND.BIN"
#define CONFIG_CANCEL "Press L1, R1, START and SELECT to cancel"
//#define NEW_LEVELS 1
#endif

#ifdef VERSION_DEMO
#define LANG_ROOT_NAME "urban\\urband"
#define LANG_FILE_NAME "urban\\english.txt"
#endif

#ifndef VERSION_DEFINED
#error Change your makefile to define an official version!
#endif

#ifdef VERSION_LORES
#define DISPLAYWIDTH 320
#define DISPLAYCENTRE 160
#else
#define DISPLAYWIDTH 512
#define DISPLAYCENTRE 256
#endif

#define OVERLAY_SIZE	(62*1024)

#if 0
#ifndef VERSION_KANJI
#define BUCKET_MEM	(77*1024)
#else
#define BUCKET_MEM	(80*1024)
#endif
#else
extern SLONG bucket_mem_size;
extern SLONG total_mem_size;
#define BUCKET_MEM	(bucket_mem_size)
#endif

#define CURRENT_VERSION 2

#include	"inline_c.h"
#include	"gtemac.h"
#include	"c:\fallen\psxlib\headers\GDisplay.h"
#include	"night.h"
#include	"poly.h"
//
// Call once at the start of the whole program.
//

#define	ALT_SHIFT	(3)
void AENG_init(void);

#define MAX_LEVELS 35

//
// This function makes a local copy of the prim points for
// the engine's own use.
//

typedef struct
{
	float X;
	float Y;
	float Z;

} SVector_F;
  
extern SVector_F AENG_dx_prim_points[];

void AENG_create_dx_prim_points(void);

// Input Device Structure used for Control Options

#define PAD_FLAG_NONE		0
#define PAD_FLAG_DEBOUNCE	1
#define PAD_FLAG_DELAY		2
#define PAD_FLAG_DELAY_MASK 0xfc

typedef struct
{
	struct PadData
	{
		int name;
		UWORD pad_button;
		SLONG input_mask;
		UBYTE pad_flags;
		UBYTE pad_delay;
	} data[14];
} PadInfo;

extern PadInfo *PAD_Current;

//
// After you have loaded all the prims, call this function. It
// fixes the texture coordinates of the prims if the engine has
// fiddled with the texture pages.
// 

void TEXTURE_fix_prim_textures(SLONG flag);

//
// Given a texture square coordinate on a page, it returns the
// page and texture square coordinates of the fiddled position.
//
// 'square_u' and 'square_v' should be between 0 and 7, and the
// fiddled position are returned.
//

SLONG TEXTURE_get_fiddled_position(
		SLONG  square_u,
		SLONG  square_v,
		SLONG  page,
		float *u,
		float *v);

//
// Initialises a new frame.
// Draws the engine.
//

#define abs(x) MyAbs(x)

inline SLONG MyAbs(SLONG a) { return (a<0)?-a:a; };

void AENG_set_camera(SLONG world_x,SLONG world_y,SLONG world_z,SLONG yaw,SLONG pitch,SLONG roll);

void AENG_set_camera_radians(SLONG world_x,SLONG world_y,SLONG world_z,SLONG yaw,SLONG pitch,SLONG roll);

//
// Draws world lines.
//

void AENG_world_line(SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2, SLONG sort_to_front);

//
// Older engine compatability.
//

void AENG_e_draw_3d_line           (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void AENG_e_draw_3d_line_dir       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void AENG_e_draw_3d_line_col       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void AENG_e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void AENG_e_draw_3d_mapwho         (SLONG x1,SLONG z1);
void AENG_e_draw_3d_mapwho_y       (SLONG x1,SLONG y1,SLONG z1);
void SPRITE_draw_rotated(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort,
		SLONG rotate,
		SLONG fade);

#define e_draw_3d_line            AENG_e_draw_3d_line
#define	e_draw_3d_line_dir        AENG_e_draw_3d_line_dir
#define	e_draw_3d_line_col        AENG_e_draw_3d_line_col
#define	e_draw_3d_line_col_sorted AENG_e_draw_3d_line_col_sorted
#define	e_draw_3d_mapwho          AENG_e_draw_3d_mapwho
#define	e_draw_3d_mapwho_y        AENG_e_draw_3d_mapwho_y

//
// The new engines.
// 

void AENG_draw       (SLONG info);
void AENG_draw_inside(void);

//
// The scanner.
// 

void AENG_draw_scanner(
		SLONG screen_x1,
		SLONG screen_y1,
		SLONG screen_x2,
		SLONG screen_y2,
		SLONG map_x,
		SLONG map_z,
		SLONG map_zoom,		// The number of pixels per mapsquare in fixed-point 8.
		SLONG map_angle);

//
// Draws the messages and the FPS stuff to the screen.
//

void ANEG_draw_messages(void);

//
// Flips/blits the back-buffer.
//

void AENG_flip(void);
void AENG_blit(void);

//
// Adds a message to the message system.
//

//void MSG_add(CBYTE *message, ...);

//
// Drawing stuff straight to the screen...
//

void  AENG_clear_screen(void);
SLONG AENG_lock(void);
SLONG FONT_draw(SLONG x, SLONG y, CBYTE *text, ...);
void  AENG_unlock(void);


//
// Call once at the end of the whole program.
// 

extern	void AENG_fini(void);

extern	void CONSOLE_font(CBYTE *fontpath, SLONG scale);
extern	void CONSOLE_draw(void);
extern	void CONSOLE_text(CBYTE *text, SLONG delay=4000 );	// Delay in milliseconds
extern	void CONSOLE_clear();
extern	void CONSOLE_scroll();

extern	void AENG_loadbar(SLONG percent);
extern	void draw_boxout_text(SLONG x,SLONG y,SLONG w,CBYTE *message,SLONG font_id);
//
// Messages that appear at specific points on the screen- If you place
// a message at the same place as an older message, the new message
// replaces that older one.
//

extern	void CONSOLE_text_at(SLONG  x,SLONG  y,SLONG  delay,CBYTE *fmt, ...);

extern	void	draw_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id);
extern	void	draw_centre_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id,SLONG flag);

extern	void	PANEL_draw_health_bar(SLONG x,SLONG y,SLONG percentage);
extern	void PANEL_draw_timer(SLONG time, SLONG x, SLONG y);
extern	void	PANEL_draw_face(SLONG x,SLONG y,SLONG face,SLONG size);
extern	void	AENG_draw_col_tri(SLONG x0,SLONG y0,SLONG col0,SLONG x1,SLONG y1,SLONG col1,SLONG x2,SLONG y2,SLONG col2,SLONG layer);
extern	void PANEL_draw_buffered(void);
extern  SLONG SPRITE_draw(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort);

extern void SPRITE_draw_tex(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		UBYTE u,
		UBYTE v,
		UBYTE w,				   
		UBYTE h,
		SLONG sort);

extern	void	PANEL_finish(void);
extern	void FONT_buffer_draw(void);

void	ShowBackImage(void);
void	LoadBackImage(CBYTE *name);
void	InitBackImage(CBYTE *name);
void	ResetBackImage(void);

extern void SetDisplayClear(SLONG clear);





extern	DB		*old_buffer;
extern	void	*start_of_bucket_ram;
extern	void	*first_used_bucket_ram;
extern	void	*last_used_bucket_ram;
//extern	SLONG	flipped;

//#define	 OLD_FLIP	1
/*
inline	void	check_prim_ptr(void **x)
{
#ifdef	OLD_FLIP
	//
	// empty, so you better optimise me out
	//
#else
	if(*x>the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-100)
		*x=the_display.CurrentDisplayBuffer->PrimMem;

	if( (*x>first_used_bucket_ram) && *x<(first_used_bucket_ram+100))
	{
		DrawSync(0);   // we have caught up with the still being drawn buckets, so issue a draw sync

		first_used_bucket_ram=last_used_bucket_ram;
		//
		// 
		//
//		first_used_bucket_ram=0;//the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM;
//		flipped=0;
	}
	if( (*x>first_used_bucket_ram) && *x<(first_used_bucket_ram+200))
	{
		ASSERT(0);
	}

	ASSERT(*x<last_used_bucket_ram-60 || *x>=last_used_bucket_ram); //check we havent caught up with ourselves
#endif
}
*/

extern	void	*danger_point;
extern	UBYTE	danger_point_type;
extern	volatile	SLONG draw_state __attribute__((section(".rdata")));
extern	void	do_danger(void **x);

inline	void	check_prim_ptr(void **x)
{

	if(*x>danger_point)
	{
		do_danger(x);
	}

}


//extern	void	check_prim_ptr(void **x);
	

// Common PSX Engine inlines

//
// This is optimised for objects that can't be far away like people and prims
//

// these are upto 12 blocks away  // z=0 -> 768 (12 blocks * 256 /4)
inline	SLONG get_z_sort_near(SLONG z)
{

//	ASSERT(z<1000);

	if(z>768)
		z=768;

	if(z<0)
		z=0;
//	if(z>=OTSIZE)
//		z=(OTSIZE-1);
	z=(OTSIZE-1)-z;
	return(z);

}
// upto max dist 18 ish to 25 ish
inline	SLONG get_z_sort(SLONG z)
{

	if(z>768)
	{
		z=((z-768)>>2)+768;
		if(z>=OTSIZE)
			z=(OTSIZE-1);
	}
//	ASSERT(z<OTSIZE);

	if(z<0)
		z=0;
	z=(OTSIZE-1)-z;
	return(z);

}

inline SLONG MIN4(SLONG a,SLONG b,SLONG c,SLONG d)
{
	int min=a;
	if (b<min)
		min=b;
	if (c<min)
		min=c;
	if (d<min)
		return d;
	else
		return min;
}

inline SLONG MAX4(SLONG a,SLONG b,SLONG c,SLONG d)
{
	int max=a;
	if (b>max)
		max=b;
	if (c>max)
		max=c;
	if (d>max)
		return d;
	else
		return max;
}

inline SLONG MAX3(SLONG a,SLONG b,SLONG c)
{
	int max=a;
	if (b>max)
		max=b;
	if (c>max)
		return c;
	else
		return max;
}

inline SLONG MF_NormalClip(SLONG a,SLONG b,SLONG c)
{
	SLONG tmp;
	gte_NormalClip(a,b,c,&tmp);
	return tmp;
}

#define getPSXClut(page) 	getClut(960+((page&3)<<4),256+(page>>2))
#define getPSXTPage(page)	psx_tpages[(page)>>6]
#define getPSXU(page)		(((page)&7)<<5)
#define getPSXV(page)		(((page)&0x38)<<2)
#define getPSXU2(page)		(((page)&12)<<4)
#define getPSXV2(page)		(((page)&0x30)<<2)
#define getPSXClut2(page) 	getClut(960+((page&3)<<4),256+(page>>2))

// Extras palettes can go right on the end of the current palettes, best place for them (64 spare 
// that aren't ever going to be used since they represent the position of the palette page)

#define getPSXClutE(page)	getClut(448+((page&3)<<4),256+(page>>2))
#define getPSXTPageE(page)	psx_tpages[16]

// Panel page used for all the panel graphics, now distanced from the
// The sky, which means I can get much more in.

#define getPSXClutP(page)	getClut(448+((page&3)<<4),256+32+(page>>2))
#define getPSXTPageP(page)	psx_tpages[18]

// Sky palettes and textures are going to cause fucking problems ATM
// I know where the sky is going to be, but not where I'm going to get the palettes from. * FIXED *

#define getPSXClutS(page)	getClut(448+((page&3)<<4),256+16+(page>>2))
#define getPSXTPageS(page)	psx_tpages[17]

// Text texture page

#define getPSXClutT(page)	getClut(448+((page&3)<<4),256+48+(page>>2))
#define getPSXTPageT(page)	psx_tpages[19]

//#define getPSXClut2(page) 	getClut(960+((page&3)<<4),256+((page&0x3f7)>>2))

#define getPSXCol(fade,val,rgb) ( ( (((UWORD)(fade)*(UWORD)(val)) + (128-(UWORD)(fade))*(UWORD)(rgb))) >> 7 )

#define setPSXRGB0(p,fade,r,g,b) { \
	p->r0=getPSXCol(fade,r,NIGHT_amb_red);   \
	p->g0=getPSXCol(fade,g,NIGHT_amb_green);   \
	p->b0=getPSXCol(fade,b,NIGHT_amb_blue); }

#define setPSXRGB1(p,fade,r,g,b) { \
	p->r1=getPSXCol(fade,r,NIGHT_amb_red);   \
	p->g1=getPSXCol(fade,g,NIGHT_amb_green);   \
	p->b1=getPSXCol(fade,b,NIGHT_amb_blue); }

#define setPSXRGB2(p,fade,r,g,b) { \
	p->r2=getPSXCol(fade,r,NIGHT_amb_red);   \
	p->g2=getPSXCol(fade,g,NIGHT_amb_green);   \
	p->b2=getPSXCol(fade,b,NIGHT_amb_blue); }

#define setPSXRGB3(p,fade,r,g,b) { \
	p->r3=getPSXCol(fade,r,NIGHT_amb_red);   \
	p->g3=getPSXCol(fade,g,NIGHT_amb_green);   \
	p->b3=getPSXCol(fade,b,NIGHT_amb_blue); }



#define getPSXFade(fade) 	(128-((fade)>>5))
//#define getPSXFade(fade) 	((NIGHT_flag & NIGHT_FLAG_DAYTIME)?(128+((fade)>>5)):(128-((fade)>>5)))
#define getPSXFade_night(fade) 	(128-((fade)>>5))
#define getPSXFade_day(fade) 	(128-((fade)>>5))

#define DOPRIM(z,p)	addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
//#define DOPRIM(z,p)	{ASSERT(p<the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM);addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);}
#define ALLOCPRIM(p,s) { check_prim_ptr((void**)&the_display.CurrentPrim);p=(s *)the_display.CurrentPrim;the_display.CurrentPrim+=sizeof(s); }
//#define ALLOCPRIM(p,s) { p=(s *)the_display.CurrentPrim;the_display.CurrentPrim+=sizeof(s); }
#define DEALLOCPRIM(s) the_display.CurrentPrim-=sizeof(s);

#define LOWORD(l) ((l)&0xffff)
#define HIWORD(l) ((l)>>16)

#define	MAX_PATH	128
#define	FILE	SLONG

#define	MFFileHandle	SLONG
#define	FILE_OPEN_ERROR	(-1)
#define	SEEK_MODE_CURRENT	(1)

extern	SLONG	SpecialOpen(CBYTE *name);
extern	SLONG	SpecialRead(SLONG handle,UBYTE *ptr,SLONG s1);
extern	SLONG	SpecialSeek(SLONG handle,SLONG mode,SLONG size);
extern  SLONG	SpecialClose(SLONG handle);
extern	SLONG	SpecialSize(SLONG handle);

#define	FileOpen(x)		SpecialOpen(x)
#define	FileClose(x)	SpecialClose(x)
#define	FileCreate(x,y)	ASSERT(0)
#define	FileRead(h,a,s) SpecialRead(h,(UBYTE*)a,s)
#define	FileWrite(h,a,s) ASSERT(0)
#define	FileSeek(h,m,o) SpecialSeek(h,m,o)
#define FileSize(h)		SpecialSize(h)

#define	fopen(x,y)		SpecialOpen(x)
#define	fclose(x)		SpecialClose(x)
#define	fread(a,s1,s2,h) SpecialRead(h,a,s1*s2)

extern void TEXTURE_choose_set(SLONG number);
extern void	PCReadFile(CBYTE *name,UBYTE *addr,ULONG len);
extern int vsprintf(char *str,const char *fmt,va_list arg);
extern void MF_ClearOTag(ULONG *table,SLONG size);
extern void	PSX_SetShock(UBYTE fast,UBYTE slow);
extern void PANEL_inventory(Thing *darci, Thing *player);

extern UBYTE MAP_beacon_create(SLONG x, SLONG z, SLONG index, UWORD track_thing);

#define PANEL_WHO_DARCI 0
#define PANEL_WHO_ROPER 1

#define BreakStart()
#define BreakTime(x)
#define BreakFrame()
#define BreakEnd(x)

extern void PANEL_new_text(Thing *who, SLONG delay, CBYTE *fmt, ...);
extern FONT2D_DrawString(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);
extern void FONT2D_DrawStringCentred(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);

extern wad_level;


#ifdef VERSION_NTSC
#define PSX_TIMER_VBLANK 17
#else
#define PSX_TIMER_VBLANK 20
#endif	

// Font special characters

#define STR_BLOCK	"\x7f"
#define STR_TRI		"\x81"
#define STR_SQUARE	"\x8D"
#define STR_CIRCLE	"\x8E"
#define STR_CROSS	"\x8F"
#define STR_LEFT	"\x90"
#define STR_RIGHT	"\x9D"
#define STR_UP		"\x9E"
#define STR_DOWN	"\xA0"

#define IMAGE_LEVEL_LOST		0
#define IMAGE_LEVEL_COMPLETE	1

typedef struct {
	UWORD	page;
	UBYTE	x,y;
	UBYTE	w,h;
} ImageInfo;

extern UBYTE floor_lum[32][32];
extern SBYTE lum_off_x;
extern SBYTE lum_off_z;

#define LUMI(x,z) floor_lum[(x)-lum_off_x][(z)-lum_off_z]
#define SQR(x) ((x)*(x))

//
// MIKED, for writing uv's to drawlist structues, with a whole long write at a time
//
#define	LONG_set_BBW(p,i,b1,b2,w)	((ULONG*)p)[i]=((w<<16)|(b2<<8)|(b1))

inline UBYTE MAKELUMI(UWORD col,UBYTE lumi)
{
	if (lumi==0)
		return col;
	col+=lumi;
	if (col>128)
		return 128;
	else
		return col;
}

#define SetLumi(x,z,lum) floor_lum[x][z]=(lum)

extern void Wadmenu_gameover(int won);
extern void PANEL_new_help_message(CBYTE *fmt, ...);
extern void	PSXOverLay(CBYTE *name,ULONG len);
extern void MAP_beacon_remove(UBYTE beacon);
extern void PANEL_fadeout_init();
extern void PANEL_fadeout_start();
extern void PANEL_fadeout_draw();
extern SLONG PANEL_fadeout_finished();
extern SLONG GetTickCount();
extern void POLY2D_TextImage(SLONG image,SLONG x,SLONG y,SLONG col);

#define	MUSIC_FLAG_FADE_IN	1
#define MUSIC_FLAG_FADE_OUT	2
#define MUSIC_FLAG_LOOPED	4
#define MUSIC_FLAG_QUEUED	8
#define MUSIC_FLAG_OVERRIDE 16
extern UBYTE MUSIC_play(UWORD wave,UBYTE flags);
extern void MUSIC_stop(BOOL fade);
extern SLONG MFX_Conv_playing;
extern void MFX_Conv_stop();
extern void MFX_Mute(SLONG mute);

#define MFX_FLAG_SLIDER		(1024)
#define MFX_FLAG_SEARCHER	(512)

#define LogText
#define DebugText

#define MFX_QUICK_stop() MFX_Conv_stop();
#define MFX_QUICK_still_playing() (MFX_Conv_playing?1:0)

#ifdef VERSION_NTSC
#define MIN_SCREEN_Y	0
#define MAX_SCREEN_Y	16
#else
#define MIN_SCREEN_Y	16
#define MAX_SCREEN_Y	48
#endif

#define INPUT_MASK_RUN	(INPUT_MASK_MOVE|INPUT_MASK_FORWARDS)
#define WADMENU_PAD_CANCEL	(PAD_FLT|PAD_FRT|PAD_START|PAD_SEL)
extern SLONG PANEL_icon_time;
extern void	GDisp_SetupBucketMem(char *addr,SLONG size);

#define COLLIDE_can_i_fastnav(x,z) (!(WITHIN((x), 0, PAP_SIZE_HI - 1) || !WITHIN((z), 0, PAP_SIZE_HI - 1) || (COLLIDE_fastnav[x][(z) >> 3] & (1 << ((z) & 0x7)))))

#endif


