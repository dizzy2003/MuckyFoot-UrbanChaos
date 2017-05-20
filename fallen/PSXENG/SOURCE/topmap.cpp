//
//
//
//

#include "MFStdLib.h"
#include <stdlib.h>
#include "game.h"
#include "poly.h"
#include "mav.h"
#include "memory.h"

#include "psxeng.h"

SLONG MAP_Player_x;
SLONG MAP_Player_y;

#define MAP_SCREEN_X(sx) (cx+(((sx)>>4))-MAP_Player_x)
#define MAP_SCREEN_Y(sy) (cy+((((sy)*3)>>6))-MAP_Player_y)

#define TOPMAP_MAP_OTZ	0
#define TOPMAP_NAV_OTZ	1
#define TOPMAP_PULSE_OTZ 2
#define TOPMAP_WIN_OTZ	3
								    
extern	CBYTE	*EWAY_get_mess(SLONG index);    

inline UBYTE FadeCol(int x,int y)
{
	SLONG col=288-(SquareRoot0(SQR(288-x)+SQR(120-y))<<1);
	SATURATE(col,0,128);
	return (UBYTE)col;
}

#ifdef TOPMAP_BACK

void MAP_draw_solid(SLONG page,SLONG x,SLONG y)
{
	POLY_GT4 *p;
	SLONG rot,u,v,col;

	rot=(page>>10)&3;
	page=page&0x3ff;
	ALLOCPRIM(p,POLY_GT4);

	setPolyGT4(p);
	col=288-(SquareRoot0(SQR(288-x)+SQR(120-y))<<1);
	SATURATE(col,0,128);
	setRGB0(p,col,col,col);
	col=288-(SquareRoot0(SQR(272-x)+SQR(120-y))<<1);
	SATURATE(col,0,128);
	setRGB1(p,col,col,col);
	col=288-(SquareRoot0(SQR(288-x)+SQR(108-y))<<1);
	SATURATE(col,0,128);
	setRGB2(p,col,col,col);
	col=288-(SquareRoot0(SQR(272-x)+SQR(108-y))<<1);
	SATURATE(col,0,128);
	setRGB3(p,col,col,col);

	setXYWH(p,x,y,16,12);
	p->tpage=getPSXTPage(page);
	p->clut=getPSXClut(page);
	u=getPSXU(page);
	v=getPSXV(page);

	switch(rot)
	{
	case 0:
		setUV4(p,u,v,u+31,v,u,v+31,u+31,v+31);
		break;
	case 1:
		setUV4(p,u+31,v,u+31,v+31,u,v,u,v+31);
		break;
	case 2:
		setUV4(p,u+31,v+31,u,v+31,u+31,v,u,v);
		break;
	case 3:
		setUV4(p,u,v+31,u,v,u+31,v+31,u+31,v);
		break;
	}

	DOPRIM(TOPMAP_MAP_OTZ,p);
}

extern void draw_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id);
extern void	draw_centre_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id,SLONG flag);

#define STR_BLOCK	"\x07f"

void MAP_draw_dot(SLONG x,SLONG y,SLONG scale,SLONG angle,SLONG colour)
{
	POLY_G3 *p;
	SLONG colr,colg,colb,fade;
	// Move our point back
	x+=(rsin(angle<<1)*scale)>>13;
	y+=(rcos(angle<<1)*scale)>>13;

	fade=FadeCol(x,y);
	colr=(((colour>>16)&0xff)*fade)>>7;
	colg=(((colour>>8)&0xff)*fade)>>7;
	colb=(((colour)&0xff)*fade)>>7;

	ALLOCPRIM(p,POLY_G3);

	setPolyG3(p);
	setXY3(p,x-((rsin(angle<<1)*scale)>>12),y-((rcos(angle<<1)*scale)>>12),
		     x-((rcos(angle<<1)*scale)>>13),y+((rsin(angle<<1)*scale)>>13),
			 x+((rcos(angle<<1)*scale)>>13),y-((rsin(angle<<1)*scale)>>13));
	setRGB0(p,colr,colg,colb);
	setRGB1(p,colr>>2,colg>>2,colb>>2);
	setRGB2(p,colr>>2,colg>>2,colb>>2);
	DOPRIM(TOPMAP_NAV_OTZ,p);
//	draw_text_at(x-3,y-4,STR_BLOCK,colour);
}

//
// The pulses...
//

typedef struct
{
	SLONG life;		// 0 => unused
	ULONG colour;
	SLONG wx;
	SLONG wz;
	SLONG radius;

} MAP_Pulse;

#define MAP_MAX_PULSES 16

MAP_Pulse MAP_pulse[MAP_MAX_PULSES];



//
// Initialises the pulses
//

void MAP_pulse_init()
{
	memset(MAP_pulse, 0, sizeof(MAP_pulse));
}


//
// Creates a new pulse.
//

void MAP_pulse_create(SLONG wx, SLONG wz, ULONG colour)
{
	SLONG i;
	SLONG best_life  = INFINITY;
	SLONG best_pulse = INFINITY;

	for (i = 0; i < MAP_MAX_PULSES; i++)
	{
		if (best_life > MAP_pulse[i].life)
		{
			best_life  = MAP_pulse[i].life;
			best_pulse = i;
		}
	}

	MAP_Pulse *mp;

	ASSERT(WITHIN(best_pulse, 0, MAP_MAX_PULSES - 1));

	mp = &MAP_pulse[best_pulse];

	mp->life   = 256;
	mp->colour = colour;
	mp->wx     = wx;
	mp->wz     = wz;
	mp->radius = 16;
}

//
// Draws an individual pulse
//

void MAP_pulse_draw(SLONG wx, SLONG wz, SLONG radius, ULONG colour, UBYTE fade)
{
	SLONG x1,y1,x2,y2,u,v;
	POLY_FT4 *p;

	x1 = wx - radius;
	y1 = wz - radius;
	x2 = wx + (radius*3)>>2;
	y2 = wz + (radius*3)>>2;

	colour = 0xffffff;

	if (colour == 0)
	{
		//
		// Don't draw a black pulse...
		//

		return;
	}

	colour &= 0xffffff;
	colour |= fade << 24;

	//
	// Setup points
	//

	ALLOCPRIM(p,POLY_FT4);

	setPolyFT4(p);
	u=getPSXU(POLY_PAGE_CIRCLE);
	v=getPSXV(POLY_PAGE_CIRCLE);

	setXY4(p,x1,y1,x2,y1,x1,y2,x2,y2);
	setUV4(p,u,v,u+31,v,u,v+31,v+31,v+31);
	setSemiTrans(p,1);

	p->tpage=getPSXTPage(POLY_PAGE_CIRCLE);
	p->clut=getPSXClut(POLY_PAGE_CIRCLE);

	setRGB0(p,((colour>>16)*fade)>>8,(((colour>>8)&0xff)*fade)>>8,((colour&0xff)*fade)>>8);

	DOPRIM(TOPMAP_PULSE_OTZ,p);

}

//
// Draws all the pulses.
//

void MAP_pulse_draw_all(SLONG cx,SLONG cy)
{
	SLONG i;

	MAP_Pulse *mp;

	for (i = 0; i < MAP_MAX_PULSES; i++)
	{
		mp = &MAP_pulse[i];

		if (!mp->life)
		{
			continue;
		}

		MAP_pulse_draw(
			MAP_SCREEN_X(mp->wx),
			MAP_SCREEN_Y(mp->wz),
			mp->radius,
			mp->colour,
			mp->life);
	}
}

//
// Processes the pulses.
//

void MAP_process_pulses()
{
	SLONG i;

	MAP_Pulse *mp;

	static SLONG now;
	static SLONG last;

	now = GetTickCount();

	if (last < now - (1000 / 10))
	{
		last = now - (1000 / 10);
	}

	while(last < now)
	{
		//
		// Process at 20 fps...
		//

		last += (1000 / 20);

		for (i = 0; i < MAP_MAX_PULSES; i++)
		{
			mp = &MAP_pulse[i];

			if (!mp->life)
			{
				continue;
			}

			mp->life   -= 4;
			mp->radius += 1;

			if (mp->life < 0)
			{
				mp->life = 0;
			}
		}
	}
}


//
// Draws an arrow in the given direction...
//

void MAP_draw_arrow(SLONG angle, ULONG colour)
{
	SLONG x;
	SLONG y;

	SLONG dx = (rsin(angle)*150)>>12;
	SLONG dy = (rcos(angle)*110)>>12;

	SLONG u,v;

	POLY_FT4 *p;

	x = 288 + dx;
	y = 120 + dy;

	dx>>=4;
	dy>>=4;

	ALLOCPRIM(p,POLY_FT4);

	setPolyFT4(p);
	setXY4(p,x+dx-dy,y+dy+dx,x+dx+dy,y+dy-dx,x-dx-dy,y-dy+dx,x-dx+dy,y-dy-dx);
	u=getPSXU(POLY_PAGE_ARROW);
	v=getPSXV(POLY_PAGE_ARROW);
	setUV4(p,u,v,u+31,v,u,v+31,u+31,v+31);
	p->tpage=getPSXTPage(POLY_PAGE_ARROW);
	p->clut=getPSXClut(POLY_PAGE_ARROW);
	setRGB0(p,colour>>16,(colour>>8)&0xff,colour&0xff);

	DOPRIM(TOPMAP_NAV_OTZ,p);
}

#endif


//
// The map beacons.
//
/*
typedef struct
{
	UBYTE  used;
	UBYTE  counter;
	UWORD  track_thing;
	UWORD  index;
	UWORD  pad;
	SLONG  wx;
	SLONG  wz;

} MAP_Beacon;

#define MAP_MAX_BEACONS 9

MAP_Beacon MAP_beacon[MAP_MAX_BEACONS];
*/

//
// The map beacon colours.
//

#define MAP_MAX_BEACON_COLOURS 6

ULONG MAP_beacon_colour[MAP_MAX_BEACON_COLOURS] =
{
	0x7f7f1f,
	0x7f1f1f,
	0x1f7f1f,
	0x1f1f7f,
	0x7f7f7f,
	0x1f7f7f
};


//
// Initialise the beacons.
//

void MAP_beacon_init()
{
	memset(MAP_beacon, 0, sizeof(MAP_Beacon)*MAP_MAX_BEACONS);
}

//
// Creates a beacon
//

UBYTE MAP_beacon_create(SLONG x, SLONG z, SLONG index, UWORD track_thing)
{
	SLONG i;

	MAP_Beacon *mb;
extern	SLONG EWAY_mess_upto;
		ASSERT(index>=0 && index<EWAY_mess_upto);

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			mb->used         = TRUE;
			mb->counter      = 0;
			mb->track_thing  = track_thing;
			mb->index          = index;
			mb->wx           = x;
			mb->wz           = z;

			return i;
		}
	}

	return 0;
}

//
// Processes the beacons.
//

void MAP_process_beacons()
{
	SLONG i;

	MAP_Beacon *mb;

	static SLONG now;
	static SLONG last;

	now = GAME_TURN * (1000/30);

	if (last < now - (1000 / 10))
	{
		last = now - (1000 / 10);
	}

	while(last < now)
	{
		//
		// Process at 20 fps...
		//

		last += (1000 / 20);

		for (i = 1; i < MAP_MAX_BEACONS; i++)
		{
			mb = &MAP_beacon[i];

			if (!mb->used)
			{
				continue;
			}

			if (mb->track_thing)
			{
				Thing *p_track = TO_THING(mb->track_thing);

				mb->wx = p_track->WorldPos.X>>8;
				mb->wz = p_track->WorldPos.Z>>8;
			}

			mb->counter += 1;
#ifdef TOPMAP_BACK
			if (mb->counter >= 20)
			{
				MAP_pulse_create(
					mb->wx,
					mb->wz,
					MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS]);

				mb->counter = 0;
			}
#endif
		}
	}
}

#ifdef TOPMAP_BACK

void MAP_beacon_draw_all(SLONG cx,SLONG cy)
{
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG u,v;

	SLONG list = 120;

	SLONG dx;
	SLONG dy;
	SLONG dist;
	SLONG angle;

	SLONG colour;

	MAP_Beacon *mb;

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			continue;
		}

		x = MAP_SCREEN_X(mb->wx);
		y = MAP_SCREEN_Y(mb->wz);

		colour = QDIST2(abs(x-cx),abs(y-cy));

		if (colour>160)
		{
			angle = ratan2(x - cx, y - cy);

			colour &= 0xff;
			colour *= 4;
			colour  = 255 - colour;

			colour = MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS];

			MAP_draw_arrow(
				angle,
				colour);
		} else
		{
			POLY_FT4 *p;
			ALLOCPRIM(p,POLY_FT4);
			setPolyFT4(p);
			setXY4(p,x-8,y-6,x+8,y-6,x-8,y+6,x+8,y+6);
			u=getPSXU(POLY_PAGE_CIRCLE);
			v=getPSXV(POLY_PAGE_CIRCLE);
			setUV4(p,u,v,u+31,v,u,v+31,u+31,v+31);
			p->tpage=getPSXTPage(POLY_PAGE_CIRCLE);
			p->clut=getPSXClut(POLY_PAGE_CIRCLE);
			colour= MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS];
			setRGB0(p,colour>>16,(colour>>8)&0xff,colour&0xff);
			DOPRIM(TOPMAP_NAV_OTZ,p);
		}

		draw_text_at(16,list,EWAY_get_mess(mb->index),MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS]);

		list += 12;
	}
}
#endif

void MAP_beacon_remove(UBYTE beacon)
{
	ASSERT(WITHIN(beacon, 0, MAP_MAX_BEACONS - 1));

	MAP_beacon[beacon].used = FALSE;
}

#ifdef TOPMAP_BACK
extern void PANEL_inv_weapon(SLONG x, SLONG y, SLONG item, UBYTE who, SLONG alpha);
#define PANEL_ADDWEAPON(item) draw_list[draw_count++]=item

void MAP_draw_weapons(Thing *player)
{
	CBYTE draw_list[10];
	UBYTE draw_count=0;
	Thing *p_special;
	Thing *darci=NET_PERSON(0);

	int i,y;

	if (darci->Genus.Person->SpecialList)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special) 
		{
			ASSERT(p_special->Class == CLASS_SPECIAL);
			if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
				SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
				p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES)
			{
			  PANEL_ADDWEAPON(p_special->Genus.Special->SpecialType);
			}
			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}

	if (darci->Flags & FLAGS_HAS_GUN) {
		PANEL_ADDWEAPON(SPECIAL_GUN);
	}

	y=16;
	for(i=0;i<draw_count;i++)
	{
		PANEL_inv_weapon(32,y,draw_list[i],0,0x7f7f7f);
		y+=32;
	}
}

void MAP_draw()
{
	SLONG x,y,i,j;

	SLONG minx,miny;
	SLONG maxx,maxy;
	SLONG offx,offy,cx,cy;
	DR_AREA *p;
	RECT r;
	SLONG index;
	SLONG scale,colour,red,blue;
	Thing *p_thing;

	DrawSync(0);
	ClearOTag(the_display.CurrentDisplayBuffer->ot, OTSIZE);

	ALLOCPRIM(p,DR_AREA);
	SetDrawArea(p,&the_display.CurrentDisplayBuffer->Disp.disp);
	DOPRIM(TOPMAP_WIN_OTZ,p);

	x=NET_PERSON(0)->WorldPos.X>>16;
	y=NET_PERSON(0)->WorldPos.Z>>16;

	// Get offset from 0-16 from position
	offx=(NET_PERSON(0)->WorldPos.X-(x<<16))>>12;
	// Get offset from 0-12 for Z position
	offy=((NET_PERSON(0)->WorldPos.Z-(y<<16))*3)>>14;

	minx=x-10;
	if (minx<0) 
	{
		offx=0;
		minx=0;
	}
	miny=y-10;
	if (miny<0) 
	{
		miny=0;
		offy=0;
	}
	maxx=x+10;
	maxy=y+10;

	if (maxx>127) maxx=127;
	if (maxy>127) maxy=127;

	cx=128+(16*(x-minx));
	cy=(12*(y-miny));

	y=-offy;

	for(j=miny;j<=maxy;j++)
	{
		x=128-offx;
		for(i=minx;i<=maxx;i++)
		{
			MAP_draw_solid(PAP_2HI(i,j).Texture,x,y);
			x+=16;
		}
		y+=12;
	}
	r.x=128;
	r.w=319;
	r.y=the_display.CurrentDisplayBuffer->Disp.disp.y;
	r.h=239;
	ALLOCPRIM(p,DR_AREA);
	SetDrawArea(p,&r);
	DOPRIM(0,p);

	x=NET_PERSON(0)->WorldPos.X>>16;
	y=NET_PERSON(0)->WorldPos.Z>>16;

	MAP_Player_x=x=offx+(x<<4);
	MAP_Player_y=y=(y*12)+offy;

	minx >>= 2;
	miny >>= 2;
	maxx >>= 2;
	maxy >>= 2;

	for (j = miny; j <= maxy; j++)
	for (i = minx; i <= maxx; i++)
	{
		index = PAP_2LO(i,j).MapWho;

		while(index)
		{
			p_thing = TO_THING(index);

			switch(p_thing->Class)
			{
				case CLASS_PERSON:
					
					switch(p_thing->Genus.Person->PersonType)
					{
						case PERSON_DARCI:
						case PERSON_ROPER:
							scale  = (GAME_TURN<<4)&2047;
							scale  = (rsin(scale<<1)>>10)+8;
							colour = 0xffffffff;
							break;

						case PERSON_COP:

							red  = (GAME_TURN >> 3 ) & 0xff;
							blue = (GAME_TURN >> 3 ) & 0xff;

							colour  = 0xff000000;
							colour |= red << 16;
							colour |= blue;
							scale   = 8;
							break;

						case PERSON_THUG_RASTA:
						case PERSON_THUG_GREY:
						case PERSON_THUG_RED:
							scale  = 8;
							colour = 0xffff0000;
							break;

						case PERSON_SLAG_TART:
						case PERSON_SLAG_FATUGLY:
							scale  = 8;
							colour = 0xffffff00;
							break;

						case PERSON_CIV:
						case PERSON_MECHANIC:
							scale  = 8;
							colour = 0xff00ff00;
							break;

						case PERSON_HOSTAGE:
							scale  = 8;
							colour = 0xffff00ff;
							break;

						default:
							scale  = 8;
							colour = 0xff888888;
							break;
					}

					MAP_draw_dot(
						MAP_SCREEN_X(p_thing->WorldPos.X>>8),
						MAP_SCREEN_Y(p_thing->WorldPos.Z>>8),
						scale,
						p_thing->Draw.Tweened->Angle,
						colour);

					break;

				case CLASS_VEHICLE:

					MAP_draw_dot(
						MAP_SCREEN_X(p_thing->WorldPos.X>>8),
						MAP_SCREEN_Y(p_thing->WorldPos.Z>>8),
						16,
						p_thing->Genus.Vehicle->Angle,
						0xff0000ff);

					break;

				default:
					break;
			}

			index = p_thing->Child;
		}
	}
	
	MAP_pulse_draw_all(cx,cy);

	MAP_beacon_draw_all(cx,cy);

	//
	// Draw the weapons carried by Darci...
	//

	MAP_draw_weapons(NET_PERSON(0));
}

#endif