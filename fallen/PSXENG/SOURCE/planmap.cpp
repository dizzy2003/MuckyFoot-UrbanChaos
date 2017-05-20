#include <MFStdLib.h>
#include "psxeng.h"
#include "game.h"
#include "c:\fallen\headers\pap.h"
#include "c:\fallen\headers\road.h"


#define MAP_BACK	(0)
#define MAP_TEXT	(1)
#define MAP_OVERLAY (2)
#define MAP_MASK	(3)


#define	EDGE_LEFT	(1<<0)
#define	EDGE_TOP	(1<<1)
#define	EDGE_RIGHT	(1<<2)
#define	EDGE_BOTTOM	(1<<3)

#if 0
extern	UBYTE	player_visited[16][128];    

SLONG	clip_left,clip_right,clip_top,clip_bot;

void	draw_quick_rect(SONG page,SLONG csx,SLONG csy,SLONG pixelw)	   
{

	POLY_FT4 *p=(POLY_F4*)the_display.CurrentPrim;

	the_display.CurrentPrim+=sizeof(POLY_F4);

	setPolyFT4(p);

	p->tpage=getPSXTPage(page);
	p->clut=getPSXClut(page);

	setUVWH(p,getPSXU(page),getPSXV(page));
	setXYWH(p,csx,csy,pixelw,pixelw);
	setRGB0(p,128,128,128);

	DOPRIM(MAP_BACK,p);

}

/*
void	draw_edge(SLONG csx,SLONG csy,SLONG dsx,SLONG dsy)
{
	
	LINE_F2 *p=(LINE_F2*)the_display.CurrentPrim;
	the_display.CurrentPrim+=sizeof(LINE_F2);

	setLineF2(p);
	setXY2(p,csx,csy,dsx,dsy);
	setRGB0(p,0,0,0);
	DOPRIM(MAP_BACK,p);
	
}
*/

#define CORNER_TL (csx),(csy)
#define CORNER_TR (csx+pixelw-1),(csy)
#define CORNER_BL (csx),(csy+pixelw-1)
#define CORNER_BR (csx+pixelw-1),(csy+pixelw-1)

void draw_shadow_quad(SLONG csx,SLONG csy,SLONG pixelw,SLONG red,SLONG green,SLONG blue,SLONG tls,SLONG trs,SLONG bls,SLONG brs)
{
	
	POLY_G4 *p;
	p=(POLY_G4*)the_display.CurrentPrim;
	the_display.CurrentPrim+=sizeof(POLY_G4);

	setPolyG4(p);
	setXYWH(p,csx,csy,pixelw,pixelw);
	setRGB0(p,red>>tls,green>>tls,blue>>tls);
	setRGB1(p,red>>trs,green>>trs,blue>>trs);
	setRGB2(p,red>>bls,green>>bls,blue>>bls);
	setRGB3(p,red>>brs,green>>brs,blue>>brs);

	DOPRIM(MAP_BACK,p);
	
}

void	draw_shadow_rect(SLONG csx,SLONG csy,SLONG pixelw,SLONG red,SLONG green,SLONG blue,SLONG shadow,SLONG edge)
{

	if(shadow==0 && edge==0)
	{
		draw_quick_rect(csx,csy,pixelw,red,green,blue);
		return;
	}
	
//	if (edge&EDGE_LEFT)
//		draw_edge(csx,csy,csx,csy+pixelw);
//	if (edge&EDGE_RIGHT)
//		draw_edge(csx+pixelw-1,csy,csx+pixelw-1,csy+pixelw);
//	if (edge&EDGE_TOP)
//		draw_edge(csx,csy,csx+pixelw,csy);
//	if (edge&EDGE_BOTTOM)
//		draw_edge(csx,csy+pixelw-1,csx+pixelw,csy+pixelw-1);


	switch(shadow)
	{
		case	0:
			draw_quick_rect(csx,csy,pixelw,red,green,blue);
			break;
		case	1:
			draw_shadow_quad(csx,csy,pixelw,red,green,blue,2,0,0,0);
			break;
		case	6:
		case	2:
			draw_shadow_quad(csx,csy,pixelw,red,green,blue,2,0,2,0);
			break;
		case	3:
			draw_shadow_quad(csx,csy,pixelw,red,green,blue,0,0,2,0);
			break;
		case	4:
			draw_shadow_quad(csx,csy,pixelw,red,green,blue,0,0,2,2);
			break;
		case	5:
			draw_shadow_quad(csx,csy,pixelw,red,green,blue,2,0,2,2);
			break;
		case	7:
			draw_shadow_quad(csx,csy,pixelw,red,green,blue,0,0,0,2);
			break;
	}
}

UWORD	screen_x,screen_y,screen_width,screen_height,block_size,screen_mx,screen_mz;
SLONG	screen_pitch;


void	get_screen_xy(SLONG *x,SLONG *z)
{
	SLONG	rx,rz;

	rx=(*x);
	rz=(*z);

	rx-=screen_mx;
	rx*=block_size;
	rx>>=8;

	rx+=screen_x+(screen_width>>1);
	
	rz-=screen_mz;
	rz*=block_size; //screen_pitch;
	rz>>=8;

	rz+=screen_y+(screen_height>>1);

	*x=rx;
	*z=rz;

}

void	map_beacon_draw(SLONG x,SLONG z,ULONG col,ULONG flag)
{
	UBYTE	radius;
	SLONG	screen_pitch;
	SLONG	mx,mz;
	SLONG	size=4;

	mx=x>>8;
	mz=z>>8;

	//
	// how many pixels per mapwho
	//

	if(!(player_visited[mx>>3][mz]&(1<<(mx&7))) && flag==0)
		return;

	get_screen_xy(&x,&z);

	if(flag)
	{
		if(GAME_TURN&1)
			size=6;
		else
			size=4;
	}
		
	draw_shadow_rect(x-size+1,z-size+1,(size-2),col>>16,(col&0xff00)>>8,(col&0xff) ,0,0);
	draw_shadow_rect(x-size,z-size,size,0,0,0 ,0,0);

}


void plan_view_shot(SLONG wx,SLONG wz,SLONG pixelw,SLONG sx,SLONG sy,SLONG w,SLONG h)
{
	SLONG	minx,maxx,minz,maxz;
	SLONG	lminx,lmaxx,lminz,lmaxz;
	SLONG	x,z,csx,csy,c0;
	SLONG	r,g,b,shadow;
	SLONG	edge;
	POLY_F4 *p;

	screen_width=w;
	screen_height=h;
	screen_x=sx;
	screen_y=sy;
	screen_pitch=(screen_width<<8)/pixelw;
	screen_mx=wx;
	screen_mz=wz;
	block_size=pixelw;

	clip_left=sx;
	clip_right=sx+w;
	clip_top=sy;
	clip_bot=sy+h;

	csx=sx;
	csy=sy;

	minx=(wx>>8)-(w/(pixelw<<1))-1;
	maxx=(wx>>8)+(w/(pixelw<<1))+1;

	minz=(wz>>8)-(h/(pixelw<<1))-1;
	maxz=(wz>>8)+(h/(pixelw<<1))+1;

//	SATURATE(minx,0,127);
//	SATURATE(minz,0,127);
//	SATURATE(maxx,0,127);
//	SATURATE(maxz,0,127);

	for(z=minz;z<maxz;z++)
	{
		for(x=minx;x<maxx;x++)
		{
			
			if(x>1&&x<127&&z>1&&z<127)
			{
				if(player_visited[x>>3][z]&(1<<(x&7)))
				{
					SLONG	mh;
					edge=0;

					mh=PAP_2HI(x,z).Height;
					if(mh!=PAP_2HI(x-1,z).Height)
					{
						edge|=EDGE_LEFT;
					}
					if(mh!=PAP_2HI(x+1,z).Height)
					{
						edge|=EDGE_RIGHT;
					}
					if(mh!=PAP_2HI(x,z-1).Height)
					{
						edge|=EDGE_TOP;
					}
					if(mh!=PAP_2HI(x,z+1).Height)
					{
						edge|=EDGE_BOTTOM;
					}
					

					if((PAP_2HI(x,z).Flags&PAP_FLAG_HIDDEN)==0)
					{
						//
						// draw the floor
						//
						if (ROAD_is_road(x,z))
						{
							shadow=0;
						}
						else
						{
							shadow = PAP_2HI(x,z).Flags & 0x7;
						}
						r=177;
						g=231;
						b=244;

						if(shadow)
						{
							draw_shadow_rect(csx,csy,pixelw,r,g,b,shadow,edge);
						}

					}
					else
					{
						r=((PAP_2HI(x,z).Height))+140;
						if(r>255)
							r=255;
						g=r;
						b=r;

						shadow = PAP_2HI(x,z).Flags & 0x7;
						draw_shadow_rect(csx,csy,pixelw,r,g,b,shadow,edge);
					}
				}
				else
					draw_quick_rect(csx,csy,pixelw,0,0,0);
			}
			csx+=pixelw;
		}
		csx=sx;
		csy+=pixelw;
	}

	p=(POLY_F4*)the_display.CurrentPrim;
	the_display.CurrentPrim+=sizeof(POLY_F4);

	setPolyF4(p);
	setXYWH(p,sx,sy,w,h);
	setRGB0(p,177,231,244);

	DOPRIM(0,p);


}
#else
void plan_view_shot(SLONG wx,SLONG wz,SLONG pixelw,SLONG sx,SLONG sy,SLONG w,SLONG h)
{
}

#endif