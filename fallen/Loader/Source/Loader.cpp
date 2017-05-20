/*
** File:	Loader.cpp
**
** Purpose:	Loader is a simple PSX project that simply loads one of a choice
**			of executables dependant on a menu choice, for the moment this is
**			simply a choice of languages.
**
** Author:	James Watson
**			(C) 1999 Mucky Foot Productions Ltd.
*/

#define VERSION_CD

#include <MFStdLib.h>

#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include <libpad.h>
#include <libspu.h>
#include <ctrller.h>

#include <stdlib.h>
#include <ctype.h>

#ifndef VERSION_CD
#include <libsn.h>
#endif

#define OTSIZE	2048
#define BUCKETMEM 4096

#define STR_BLOCK	"\x7f"
#define STR_TRI		"\x81"
#define STR_SQUARE	"\x8D"
#define STR_CIRCLE	"\x8E"
#define STR_CROSS	"\x8F"
#define STR_LEFT	"\x90"
#define STR_RIGHT	"\x9D"
#define STR_UP		"\x9E"
#define STR_DOWN	"\xA0"

#ifdef VERSION_GERMAN
#define AUTO_SELECT 4
#endif

#ifdef VERSION_FRENCH
#define AUTO_SELECT 3
#endif

typedef struct {
	DISPENV		disp;
	DRAWENV		draw;
	ULONG		ot[OTSIZE];
	CBYTE		prim[BUCKETMEM];
	ULONG		*image;
} Display;

Display db[2];
SLONG CurrentDB;
CBYTE *prim_ptr;
CBYTE *mem_ptr=(CBYTE*)0x80160000;

#define ALLOCPRIM(p,t) { p=(t*)prim_ptr;prim_ptr+=sizeof(t); }
#define DOPRIM(o,p) addPrim(&db[CurrentDB].ot[o],p)

#define MALLOC(p,t,size) { p=(t*)mem_ptr; mem_ptr+=size; }

#define getPSXClut(page) 	getClut(960+((page&3)<<4),256+(page>>2))
#define getPSXTPage(page)	getTPage(0,0,960,0)

ControllerPacket PAD_Input1,PAD_Input2;
// Initialisation

void LOADER_Init()
{
	RECT r={0,0,512,512};

	CdInit();

	SpuInit();

	PadInitDirect((UBYTE*)&PAD_Input1,(UBYTE*)&PAD_Input2);
	PadStartCom();

	PAD_Input1.data.pad=0xffff;
	PAD_Input2.data.pad=0xffff;


	ResetGraph(0);		/* reset graphic subsystem (0:cold,1:warm) */
	SetGraphDebug(0);	/* set debug mode (0:off, 1:monitor, 2:dump) */
	InitGeom();			/* initialize geometry subsystem */

	ClearImage(&r,0,0,0);

	SetVideoMode(MODE_PAL);

	SetDefDrawEnv(&db[0].draw,0,0,512,256);
	SetDefDrawEnv(&db[1].draw,0,256,512,256);
	SetDefDispEnv(&db[0].disp,0,256,512,256);
	SetDefDispEnv(&db[1].disp,0,0,512,256);
	CurrentDB=0;
	db[0].draw.isbg=db[1].draw.isbg=1;
	PutDispEnv(&db[CurrentDB].disp);
	PutDrawEnv(&db[CurrentDB].draw);
	ClearOTagR(db[CurrentDB].ot,OTSIZE);
	prim_ptr=db[CurrentDB].prim;
	db[0].image=db[1].image=0;
	db[0].disp.screen.x=db[1].disp.screen.x=0;
	db[0].disp.screen.y=db[1].disp.screen.y=30;
	SetDispMask(1);
}

// This swaps display pages.

void LOADER_SwapDB()
{
	DrawSync(0);
	DrawOTag(&db[CurrentDB].ot[OTSIZE-1]);
	VSync(2);
	CurrentDB=1-CurrentDB;
	PutDispEnv(&db[CurrentDB].disp);
	PutDrawEnv(&db[CurrentDB].draw);
	ClearOTagR(db[CurrentDB].ot,OTSIZE);
	prim_ptr=db[CurrentDB].prim;

	if (db[CurrentDB].image)
		LoadImage(&db[CurrentDB].draw.clip,db[CurrentDB].image);
}

// Sets a background image to use on the pages

void LOADER_BackImage(ULONG *image)
{
	db[0].image=db[1].image=image;
	db[0].draw.isbg=db[1].draw.isbg=0;
}

char Wadmenu_char_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$*-()[]\\/?";
char Wadmenu_char_width[]={
	20,20,18,18,15,15,18,16,
	14,16,20,15,22,20,20,20,
	20,20,20,20,20,20,22,20,
	20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20,
	20,20,20,20,20,20,20,20};

typedef struct {
	char *name;		// Name of the Item, to display centred at the bottom of the screen when selected
	char *title;	// Title, to be displayed at the top when selected
	UBYTE u,v,w,h;	// Texture coordinates for image
	SWORD x,y,dw,dh;// Position to draw texture at
	char *fname;	// Name of executable to load
	char *loading;	// Loading to be displayed once you've selected a level
	char *select;	// Text to show for X to select.
	char *nopad;	// No pad message
	char *unsupp;	// Unsupported
} LoaderItem;

LoaderItem loader_item[]={
	{"English","Select Language",0,192,80,63,56,88,128,64,"FALLEN.EXE;1","LOADING","Press "STR_CROSS" to Select","Controller Removed","Unsupported Controller"},
	{"Italiano","Selezionare lingua",80,192,80,63,192,88,128,64,"IFALLEN.EXE;1","CARICAMENTO",STR_CROSS" Conferma","Controller rimosso","Controller non supportato"},
	{"ESPANOL","Seleccionar IDIOMA",160,192,80,63,328,88,128,64,"SFALLEN.EXE;1","CARGANDO","Pulsa "STR_CROSS" para seleccionar","El mando se ha quitado","Mando no soportado"},
	{"French","Select Language",0,0,0,0,0,0,0,0,"FALLEN.EXE;1","CHARGEMENT","Press "STR_CROSS" to Select","Controller Removed","Unsupported Controller"},
	{"German","Select Language",0,0,0,0,0,0,0,0,"FALLEN.EXE;1","LADEN","Press "STR_CROSS" to Select","Controller Removed","Unsupported Controller"}
};

#define LOADER_ITEMS 3

SLONG draw_char(SLONG x,SLONG y,char c,SLONG colour)
{
	POLY_FT4 *p;
	int c0;

	if (c==' ')
		return 16;

	c0=(int)strchr(Wadmenu_char_table,toupper(c));

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
		DOPRIM(0,p);
		return Wadmenu_char_width[c0];
	}
	return 0;
}

SBYTE f_descend[]={
	0,0,0,0,0,0,0,0,
	0,0,0,0,1,0,1,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,2,
	0,0,1,0,0,0,0,0,
	2,2,0,0,0,0,0,0,
	0,2,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,3,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,2,0,2
};

UBYTE wadmenu_f_width[]={
	5 ,2 ,5 ,11,8 ,12,10,2,
	5 ,5 ,8 ,8 ,2 ,8, 3 ,5,
	8 ,3 ,8 ,8 ,8 ,8 ,8 ,8,
	8 ,8 ,2 ,2 ,7 ,6 ,7 ,7,
	11,8 ,8 ,8 ,8 ,6 ,7 ,10,
	8 ,2 ,6 ,7 ,7 ,11,8 ,11,
	8 ,11,9 ,8 ,8 ,8 ,8 ,10,
	8 ,8 ,6 ,4 ,5 ,4 ,6 ,11,
	3 ,6 ,6 ,6 ,6 ,7 ,6 ,6 ,
	6 ,2 ,3 ,6 ,2 ,10,6 ,6 ,
	6 ,6 ,6 ,6 ,5 ,6 ,7 ,10,
	8 ,7 ,6 ,5 ,2 ,5 ,0 ,10,
	0 ,12,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,0 ,0 ,0 ,0 ,11,10,9 ,
	11,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,0 ,0 ,0 ,0 ,11,12,0 ,
	12,2 ,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,12,0 ,0 ,0 ,0 ,12,0 ,
	0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
	0 ,0 ,0 ,0 ,0 ,0 ,0 ,8 ,
	8 ,8 ,8 ,8 ,8 ,8 ,11,9 ,
	6 ,6 ,6 ,6 ,2 ,2 ,2 ,2 ,
	8 ,8 ,11,11,11,11,11,9 ,
	11,8 ,8 ,8 ,8 ,8 ,7 ,10,
	6 ,6 ,6 ,6 ,6 ,6 ,12,0 ,
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
				y0+=12;
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
			DOPRIM(0,p);
			p++;
			x0+=wadmenu_f_width[c]+1;
			break;
		}
		m++;
	}
	DOPRIM(0,tp);
}



SLONG draw_string(SLONG x,SLONG y,char *str,SLONG colour)
{
	SLONG x0=x;
	char *p=str;

	while(*p)
		x0+=draw_char(x0,y,*p++,colour);
}

int text_width(char *str)
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


void LOADER_Render(LoaderItem *item,SLONG sel)
{
	POLY_FT4 *p;

	if (sel)
	{
		draw_string(256-(text_width(item->title)>>1),32,item->title,0x7f7f7f);
		draw_string(256-(text_width(item->name)>>1),192,item->name,0x7f7f7f);
		Wadmenu_draw_text_at(480-Wadmenu_text_width2(item->select),216,item->select,0x7f7f7f);
	}

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	setXYWH(p,item->x,item->y,item->dw,item->dh);
	setUVWH(p,item->u,item->v,item->w,item->h);
	p->tpage=getPSXTPage(511);
	p->clut=getPSXClut(511);
	setRGB0(p,128,128,128);
	if (!sel)
		setSemiTrans(p,1);
	DOPRIM(0,p);
}

SLONG LOADER_DispMenu(SLONG selected)
{
	SLONG x,y,i;
	x=32;
	y=32;

	for(i=0;i<LOADER_ITEMS;i++)
		LOADER_Render(&loader_item[i],i==selected?1:0);

	switch(PadInfoMode(0,InfoModeCurID,0))
	{
	case 0:
		Wadmenu_draw_text_at(256-(Wadmenu_text_width2(loader_item[selected].nopad)>>1),176,loader_item[selected].nopad,0x7f7f7f);
	case 4:
	case 7:
		break;
	default:
		Wadmenu_draw_text_at(256-(Wadmenu_text_width2(loader_item[selected].nopad)>>1),176,loader_item[selected].unsupp,0x7f7f7f);
		break;
	}

		
}

void LoadFile(CBYTE *fname,ULONG *addr,SLONG len)
{
#ifdef VERSION_CD
	char str[80];
	sprintf(str,"\\%s;1",fname);

	CdReadFile(str,addr,(len+2047)&0xfffff800);
	CdReadSync(0,str);
#else
	char str[80];
	SLONG fh;
	sprintf(str,"c:\\urbancd\\%s",fname);
	fh=PCopen(str,0,0);
	PCread(fh,(char*)addr,len);
	PCclose(fh);
#endif
}

void LOADER_GetData()
{
	RECT rect={512,0,512,512};
	TIM_IMAGE tim;

	ULONG *TextureBuffer;

	MALLOC(TextureBuffer,ULONG,524288);
	LoadFile("LEVELS0\\LEVEL00\\TEXTURE0.TEX",TextureBuffer,524288);
	LoadImage(&rect,TextureBuffer);
	DrawSync(0);
	LoadFile("DATA\\BACK0.TIM",TextureBuffer,245784);
	OpenTIM(TextureBuffer);
	ReadTIM(&tim);
	LOADER_BackImage(tim.paddr);
}

typedef struct {
	char pack[16];
	void (*pc)();
} Header;

void LOADER_DoLoad(SLONG sel)
{
	Header *head=(Header*)0x8000f800;
	LOADER_SwapDB();
	draw_string(256-(text_width(loader_item[sel].loading)>>1),110,loader_item[sel].loading,0x7f7f7f);
	LOADER_SwapDB();
	draw_string(256-(text_width(loader_item[sel].loading)>>1),110,loader_item[sel].loading,0x7f7f7f);

	DrawSync(0);
	DrawOTag(&db[CurrentDB].ot[OTSIZE-1]);
	VSync(2);
	CurrentDB=1-CurrentDB;
	PutDispEnv(&db[CurrentDB].disp);
	PutDrawEnv(&db[CurrentDB].draw);
	ClearOTagR(db[CurrentDB].ot,OTSIZE);
	prim_ptr=db[CurrentDB].prim;


	// Stick code to load the main executable here.
	PadStopCom();
	LoadFile(loader_item[sel].fname,0x8000f800,1048576);
	head->pc();
}

// Main process loop

void LOADER_Process()
{
#ifndef AUTO_SELECT
	static SLONG selected=0;
	static SLONG awaiting=0;
	static SLONG timer=250;
	SLONG items;

	items=LOADER_DispMenu(selected);

	timer--;

	if (awaiting)
	{
		if (NoPadKeyPressed(&PAD_Input1))
			awaiting=0;
	} else
	{
		if (PadKeyIsPressed(&PAD_Input1,PAD_LL))
		{
			selected--;
			if (selected<0)
				selected+=LOADER_ITEMS;
			awaiting=1;
			timer=250;
		}
		if (PadKeyIsPressed(&PAD_Input1,PAD_LR))
		{
			selected++;
			if (selected==LOADER_ITEMS)
				selected=0;
			awaiting=1;
			timer=250;
		}
		if (PadKeyIsPressed(&PAD_Input1,PAD_RD)||(timer==0))
			LOADER_DoLoad(selected);
	}

	LOADER_SwapDB();
#else
	LOADER_DoLoad(AUTO_SELECT);
#endif
}

// Main function

void main()
{
	LOADER_Init();
	LOADER_SwapDB();
	LOADER_SwapDB();

	LOADER_GetData();

	while(1)
	{
		LOADER_Process();
	}
}