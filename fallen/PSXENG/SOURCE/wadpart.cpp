// wadpart.cpp
//
// Particle system for Wadmenu, this provides animating elements for
// the frontend.
//

#include <MFStdLib.h>
#include <libgte.h>
#include <libgpu.h>

#include "wadpart.h"

#include "Game.h"

#include "psxeng.h"

#define W_(x) Wad_Str[(x)+1]

extern char *Wad_Str[];
W_Particle Wadpart_Particle[WADPART_MAXPARTICLES];

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
#define WADMENU_TEXT_MISSION (WADMENU_TEXT_CENTRE)

extern void Wadmenu_draw_text(SLONG x,SLONG y,char *str,SLONG colour,SLONG flags);
extern SLONG Wadmenu_draw_char(SLONG x,SLONG y,unsigned char c,SLONG colour);
extern void Wadmenu_Image(SLONG id,SLONG x,SLONG y,SLONG scale,SLONG colour);


void Wadpart_RenderLeaf(W_Particle *part)
{
	POLY_FT4 *p;
	SVECTOR sv;
	VECTOR tv;
	MATRIX m;
	SVECTOR pp[4];
	VECTOR rp[4];
	SLONG flag,scale=part->scale;
	SLONG vx,vy;
	SLONG rot_add=((SLONG)part)&4095;

	vx=part->location.vx>>8;
	vy=part->location.vy>>8;

	ALLOCPRIM(p,POLY_FT4);

	sv.vx=part->velocity.vy+(rot_add<<2);
	sv.vy=part->velocity.vx+rot_add;
	sv.vz=4096-(part->life<<3)-(rot_add>>2);
	tv.vx=tv.vy=tv.vz=0;

	SetRotMatrix(RotMatrix(&sv,&m));
	SetTransMatrix(TransMatrix(&m,&tv));
	
	pp[0].vx=-(16*scale)>>8;
	pp[0].vy=-(16*scale)>>8;
	pp[0].vz=0;

	pp[1].vx=(16*scale)>>8;
	pp[1].vy=-(16*scale)>>8;
	pp[1].vz=0;

	pp[2].vx=-(16*scale)>>8;
	pp[2].vy=(16*scale)>>8;
	pp[2].vz=0;

	pp[3].vx=(16*scale)>>8;
	pp[3].vy=(16*scale)>>8;
	pp[3].vz=0;


	RotTrans(&pp[0],&rp[0],&flag);
	RotTrans(&pp[1],&rp[1],&flag);
	RotTrans(&pp[2],&rp[2],&flag);
	RotTrans(&pp[3],&rp[3],&flag);

	SetPolyFT4(p);

	setXY4(p,vx+rp[0].vx,vy+rp[0].vy,
			 vx+rp[1].vx,vy+rp[1].vy,
			 vx+rp[2].vx,vy+rp[2].vy,
			 vx+rp[3].vx,vy+rp[3].vy);

	setUVWH(p,0,40,31,31);

	setRGB0(p,(part->colour>>16)&0xff,(part->colour>>8)&0xff,part->colour&0xff);
	if (part->colour&0xff000000)
		setSemiTrans(p,1);

	p->tpage=getTPage(0,1,576,0);
	p->clut=getClut(512,252);

	DOPRIM(2,p);
}

void Wadpart_Init()
{
	SLONG i;

	for(i=0;i<WADPART_MAXPARTICLES;i++)
	{
		Wadpart_Particle[i].used=0;
	}
}
SLONG Wadpart_Sync()
{
	SLONG i;

	for(i=0;i<WADPART_MAXPARTICLES;i++)
		if ((Wadpart_Particle[i].used)&&!(Wadpart_Particle[i].flags&WADPART_FLAG_AMBIENT))
			return 0;
	return 1;
}

inline W_Particle *Wadpart_FindParticle()
{
	SLONG i;
	for(i=0;i<WADPART_MAXPARTICLES;i++)
		if (!Wadpart_Particle[i].used)
			return &Wadpart_Particle[i];
	return 0;
}

void Wadpart_AddTextParticle(SLONG text,SLONG colour,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_TEXT;
	p->value=text;
	p->colour=colour;
	p->life=life;
	p->scale=256;
	p->used=1;
	p->velocity.vx=((dx-x)<<8)/life;
	p->velocity.vy=((dy-y)<<8)/life;
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->flags=flags;
}

void Wadpart_AddStringParticle(char *text,SLONG colour,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_STRING;
	p->value=(SLONG)text;
	p->colour=colour;
	p->life=life;
	p->scale=256;
	p->used=1;
	p->velocity.vx=((dx-x)<<8)/life;
	p->velocity.vy=((dy-y)<<8)/life;
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->flags=flags;
}


void Wadpart_AddLeafParticle(SLONG image,SLONG colour,SLONG x,SLONG y,SLONG scale,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_LEAF;
	p->value=image;
	p->colour=colour;
	p->life=512;
	p->scale=scale;
	p->used=1;
	p->velocity.vx=p->velocity.vy=0;

	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->flags=flags;
}


void Wadpart_AddBloodParticle(SLONG x,SLONG y,SLONG vx,SLONG vy,SLONG scale,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_BLOOD;
	p->value=17;
	p->colour=0x7f7f7f7f;
	p->life=16;
	p->flags=flags|WADPART_FLAG_ACCEL;
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->velocity.vx=vx<<4;
	p->velocity.vy=vy<<4;
	p->scale=scale;
	p->used=1;
}

void Wadpart_AddImageParticle(SLONG image,SLONG colour,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_IMAGE;
	p->value=image;
	p->colour=colour;
	p->life=life;
	p->scale=256;
	p->used=1;
	if ((dx==0)&&(dy==0))
	{
		p->velocity.vx=p->velocity.vy=0;
	}
	else
	{
		p->velocity.vx=((dx-x)<<8)/life;
		p->velocity.vy=((dy-y)<<8)/life;
	}
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->flags=flags;
}

void Wadpart_AddBoardParticle(SLONG image,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_BOARD;
	p->value=image;
	p->colour=0x7f7f7f;
	p->life=life;
	p->scale=256;
	p->used=1;
	if ((dx==0)&&(dy==0))
	{
		p->velocity.vx=p->velocity.vy=0;
	}
	else
	{
		p->velocity.vx=((dx-x)<<8)/life;
		p->velocity.vy=((dy-y)<<8)/life;
	}
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->flags=flags;
}


extern char Wadmenu_char_width[];
extern char Wadmenu_char_table[];

void Wadpart_AddAnimParticle(SLONG image,SLONG x,SLONG y,SLONG vx,SLONG vy,SLONG scale,SLONG life,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_ANIM;
	p->value=image;
	p->colour=0x007f7f7f;
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->velocity.vx=vx;
	p->velocity.vy=vy;
	p->flags=flags;
	p->life=life;
	p->scale=scale;
	p->flags=flags|WADPART_FLAG_ANIMATED;
	p->used=1;
}

void Wadpart_AddRainParticle(SLONG image,SLONG x,SLONG y,SLONG vx,SLONG vy,SLONG scale,SLONG life,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_RAIN;
	p->value=image;
	p->colour=0xff3f3f3f;
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->velocity.vx=vx;
	p->velocity.vy=vy;
	p->flags=flags;
	p->life=life;
	p->scale=scale;
	p->flags=flags;
	p->used=1;
}
#ifndef VERSION_KANJI
extern int Wadmenu_text_width(char *str);
#endif

void Wadpart_AddCharExplode(SLONG text,SLONG colour,SLONG x,SLONG y,SLONG life,SLONG flags)
{
	W_Particle *p;
	SLONG x0=x;
	char *c=W_(text);
#ifndef VERSION_KANJI
	if (flags & WADMENU_TEXT_CENTRE)
		x0-=(Wadmenu_text_width(c)>>1);
#else
	if (flags & WADMENU_TEXT_CENTRE)
		x0-=4*strlen(c);
#endif

	while(*c)
	{
		SLONG c0=(int)strchr(Wadmenu_char_table,toupper(*c));

		if (*c==32)
		{
			x0+=16;
		} 
		else if (c0)
		{
			c0-=(int)Wadmenu_char_table;

			p=Wadpart_FindParticle();
			if (p)
			{
				p->type=WADPART_TYPE_CHAR;
				p->value=*c;
				p->colour=colour;
				p->scale=256;
				p->life=life;
				p->velocity.vx=-256+x0;
				p->velocity.vy=-256;
				p->flags=flags|WADPART_FLAG_GRAVITY|WADPART_FLAG_FADE|WADPART_FLAG_EXPAND;
				p->location.vx=x0<<8;
				p->location.vy=y<<8;
				p->used=1;
				x0+=Wadmenu_char_width[c0];
			}
		}
		c++;
	}
}

#define WADMENU_IMAGE_RIPPLE	15


void Wadpart_Animate(W_Particle *part)
{
	part->location.vx+=part->velocity.vx;
	part->location.vy+=part->velocity.vy;
	if (part->flags & WADPART_FLAG_ACCEL)
	{
		part->velocity.vx=(part->velocity.vx*17)>>4;
		part->velocity.vy=(part->velocity.vy*17)>>4;
	}					  
	if (part->flags & WADPART_FLAG_DECEL)
	{
		part->velocity.vx=(part->velocity.vx*15)>>4;
		part->velocity.vy=(part->velocity.vy*15)>>4;
	}
	if (part->flags & WADPART_FLAG_GRAVITY)
	{
		if (part->location.vy>(240<<8))
			part->used=0;
		else
			part->velocity.vy+=32;
	}
	if (part->flags & WADPART_FLAG_FADE)
	{
		part->colour-=0x020202;
	}
	if (part->flags & WADPART_FLAG_FADEIN)
	{
		part->colour+=0x020202;
	}
	if (part->flags & WADPART_FLAG_FLUTTER)
	{
		part->velocity.vx+=(Random()&31)-16;
		part->velocity.vy-=(Random()%48);
	}
	if (part->flags & WADPART_FLAG_PULSE)
	{
		if (Random()&1)
		{
			part->scale+=1;
			part->colour+=0x010101;
		}
		else
		{
			part->scale-=1;
			part->colour-=0x010101;
		}

	}
	if (part->flags & WADPART_FLAG_WIND)
	{
		if (part->location.vx>(512<<8))
			part->used=0;
		else
		{
			part->velocity.vx+=Random()&0x3f;
			part->velocity.vy-=Random()&0xf;
		}
	}

	if (part->flags & WADPART_FLAG_EXPAND)
	{
		part->location.vx-=64;
		part->location.vy-=64;
		part->scale+=4;
	}


	part->life--;

	if ((part->location.vy>(240<<8))&&(part->flags&WADPART_FLAG_GRAVITY))
		part->used=0;

	if (part->life==0)
	{
		switch(part->type)
		{
		case WADPART_TYPE_TEXT:
		case WADPART_TYPE_IMAGE:
		case WADPART_TYPE_CHAR:
		case WADPART_TYPE_LEAF:
		case WADPART_TYPE_ANIM:
		case WADPART_TYPE_BOARD:
		case WADPART_TYPE_RECT:
		case WADPART_TYPE_STRING:
			part->used=0;
			break;
		case WADPART_TYPE_BLOOD:
			part->type=WADPART_TYPE_IMAGE;
			part->life=64;
			part->colour=0x7f808080;
			part->velocity.vx=0;
			part->velocity.vy=part->scale>>4;
			part->flags=WADPART_FLAG_FADE|WADPART_FLAG_ACCEL|WADPART_FLAG_AMBIENT;
			break;
		case WADPART_TYPE_RAIN:
			part->used=0;
//			Wadpart_AddAnimParticle(WADMENU_IMAGE_SPLASH,part->location.vx>>8,part->location.vy>>8,0,0,256,32,WADPART_FLAG_SPLASH);
			Wadpart_AddImageParticle(WADMENU_IMAGE_RIPPLE,0xff7f7f7f,part->location.vx>>8,part->location.vy>>8,0,0,63,WADPART_FLAG_RIPPLE);
			break;
		}
	}

}

void Wadpart_AddRectParticle(SLONG x,SLONG y,SLONG w,SLONG h,SLONG colour,SLONG flags)
{
	W_Particle *p;

	p=Wadpart_FindParticle();

	if (p==0)
		return;

	p->type=WADPART_TYPE_RECT;
	p->life=32;
	p->location.vx=x<<8;
	p->location.vy=y<<8;
	p->value=(w<<16)+h;
	p->velocity.vx=0;//w<<8;
	p->velocity.vy=0;//h<<8;
	p->colour=colour;
	p->flags=flags;
	p->used=1;
}



void Wadpart_Box(SLONG x,SLONG y,SLONG w,SLONG h,SLONG colour)
{
	POLY_F4 *p;
	DR_TPAGE *tp;

	ALLOCPRIM(p,POLY_F4);
	setPolyF4(p);
	setXYWH(p,x,y,w,h);
	setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,colour&0xff);
	setSemiTrans(p,1);
	DOPRIM(1,p);

	ALLOCPRIM(tp,DR_TPAGE);
	setDrawTPage(tp,0,0,getTPage(0,2,512,0));
	DOPRIM(1,tp);

}

extern void Wadmenu_Backboard(SLONG id,SLONG x,SLONG y);

void Wadpart_Render(void)
{
	POLY_FT4 *p;
	SLONG i;
	W_Particle *part=Wadpart_Particle;

	for(i=0;i<WADPART_MAXPARTICLES;i++,part++)
	{
		if (part->used)
		{
			Wadpart_Animate(part);

			switch(part->type)
			{
			case	WADPART_TYPE_TEXT:
				Wadmenu_draw_text(part->location.vx>>8,part->location.vy>>8,W_(part->value),part->colour,part->flags&0xff);
				break;
			case	WADPART_TYPE_CHAR:
#ifndef VERSION_KANJI
				Wadmenu_draw_char(part->location.vx>>8,part->location.vy>>8,toupper(part->value),part->colour);
#endif
				break;
			case	WADPART_TYPE_IMAGE:
				Wadmenu_Image(part->value,part->location.vx>>8,part->location.vy>>8,part->scale,part->colour);
				break;
			case	WADPART_TYPE_LEAF:
				Wadpart_RenderLeaf(part);
				break;
			case	WADPART_TYPE_BLOOD:
				Wadmenu_Image(part->value,part->location.vx>>8,part->location.vy>>8,part->scale,part->colour);
				break;
			case	WADPART_TYPE_RAIN:
				Wadmenu_Image(part->value,part->location.vx>>8,part->location.vy>>8,part->scale,part->colour);
				break;
			case	WADPART_TYPE_BOARD:
				Wadmenu_Backboard(part->value,part->location.vx>>8,part->location.vy>>8);
				break;
			case	WADPART_TYPE_RECT:
				Wadpart_Box(part->location.vx>>8,part->location.vy>>8,part->value>>16,part->value&0xffff,part->colour);
				break;
			case	WADPART_TYPE_STRING:
				Wadmenu_draw_text(part->location.vx>>8,part->location.vy>>8,(char*)part->value,part->colour,part->flags&0xff);
				break;
			}
		}
	}
}