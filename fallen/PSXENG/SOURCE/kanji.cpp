/*
** Kanji.cpp
**
** Support for advanced Kanji rendering
** 
** This file supports an arse about method of doing
** Kanji Text, it has a temporary buffer of 256 16x16 blocks that
** are generated as required from the Kanji ROM font characters
**
** The image will be copied into VideoRam before the next frame starts
** drawing, the program then draws Kanji strings using 16x16 sprites (or POLY_FT4s)
** that are scaled and coloured like the Ascii fonts.
**
** We have a 1024byte lookup table that takes care of converting a 1bpp font image
** into a 4bpp image. This removes the need for the internal loop, which would slow down the
** entire operation by about 10-15 times.
**
*/


#include "MFStdLib.h"
#include <libgpu.h>
#include <libgte.h>
#include <libapi.h>

#include "psxeng.h"

// Skip everything if we aren't building a japanese version

#ifdef VERSION_KANJI



UWORD Kanji_lookup[256];

ULONG Kanji_decode[256];

UWORD Kanji_x,Kanji_y;

SLONG Kanji_next;

SLONG Kanji_GetChar(UWORD kanji)
{
	SLONG i;

	for(i=0;i<255;i++)
		if (Kanji_lookup[i]==kanji)
			return i;

	return -1;
}

void Kanji_Init(UWORD x,UWORD y)
{
	SLONG i;

	// Build the decode table

	for(i=0;i<256;i++)
	{
		SLONG v=0;
		SLONG j=0;
		for(j=0;j<8;j++)
			if (i&(1<<(7-j)))
				v|=0x1<<(j<<2);
		Kanji_decode[i]=v;
		Kanji_lookup[i]=0;
	}
	Kanji_next=0;
	Kanji_x=x;
	Kanji_y=y;
}

UBYTE Kanji_char(UWORD kanji)
{
	SLONG c=Kanji_GetChar(kanji);
	UBYTE *p;
	DR_LOAD *p2;
	SLONG i,j;
	RECT r;

	if (c==-1)
	{
		c=Kanji_next;
		p=(UBYTE*)Krom2RawAdd(kanji);
		Kanji_lookup[c]=kanji;
		Kanji_next=(Kanji_next+1)&0xff;
		if (p==(UBYTE*)-1)
			return c;
		r.x=Kanji_x+((c&0xf)<<2);
		r.y=Kanji_y+(c&0xf0);
		r.w=4;
		r.h=4;
		for(i=0;i<32;i+=8,r.y+=4)
		{
			ALLOCPRIM(p2,DR_LOAD);
			setDrawLoad(p2,&r);
			for(j=0;j<8;j++)
				p2->p[j]=Kanji_decode[p[i+j]];
			DOPRIM(PANEL_OTZ-1,p2);
		}
	}
	return c;
}

void Kanji_string(SLONG x,SLONG y,UWORD *str,SLONG col,SLONG scale)
{
	UWORD *p=str;
	POLY_FT4 *p2;
	UBYTE chr;


	while(*p)
	{
		chr=Kanji_char(*p++);
		ALLOCPRIM(p2,POLY_FT4);
		setPolyFT4(p2);
		setXYWH(p2,x,y,(scale>>4)-1,(scale>>4)-1);
		setUVWH(p2,(chr&0xf)<<4,(chr&0xf0),15,15);
		p2->tpage=getTPage(0,1,Kanji_x,Kanji_y);
//		p2->clut=getClut(0,240);
		p2->clut=getPSXClut(POLY_PAGE_FONT2D);
		setRGB0(p2,(col>>16)&0xff,(col>>8)&0xff,col&0xff);
		if (col&0xff000000)
			setSemiTrans(p2,1);
		DOPRIM(PANEL_OTZ,p2);
		x+=16;
	}

}

void Kanji_Debug()
{
	POLY_FT4 *p;
	ALLOCPRIM(p,POLY_FT4);

	setPolyFT4(p);
	setXYWH(p,256,0,255,255);
	setUVWH(p,0,0,255,255);
	p->tpage=getTPage(0,1,Kanji_x,Kanji_y);
	p->clut=getPSXClut(POLY_PAGE_FONT2D);
	DOPRIM(PANEL_OTZ,p);
}

#endif