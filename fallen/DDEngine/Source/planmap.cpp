#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "aeng.h"
#include "game.h"
#include "fallen/headers/pap.h"
#include "fallen/headers/road.h"
#include "planmap.h"




#define	EDGE_LEFT	(1<<0)
#define	EDGE_TOP	(1<<1)
#define	EDGE_RIGHT	(1<<2)
#define	EDGE_BOTTOM	(1<<3)

extern	UBYTE	player_visited[16][128];



UBYTE	*screenmem;
SLONG	clip_left,clip_right,clip_top,clip_bot;



void	draw_quick_rect(SLONG csx,SLONG csy,SLONG pixelw,SLONG red,SLONG green,SLONG blue)
{
	SLONG	right,bot;
	SLONG	dx,dy;
	UBYTE	*mem;
	ULONG	mod;

	if(csy>=clip_bot || csx>=clip_right)
		return;

	right=csx+pixelw;
	bot  =csy+pixelw;

	if(bot>=clip_bot)
		bot=clip_bot-1;

	if(csy<clip_top)
	{
		csy+=clip_top-csy;
	}

	if(csx<clip_left)
	{
		csx=clip_left;
	}

	if(right>=clip_right)
	{
		right=clip_right-1;
	}

	mod= (640*3) -(right-csx)*3;
	mem = &screenmem[csx*3+csy*640*3];

	for(dy=csy;dy<bot;dy++)
	{

		for(dx=csx;dx<right;dx++)
		{
			*mem++=red;
			*mem++=green;
			*mem++=blue;
		}
		mem+=mod;
	}
}

void	draw_shadow_rect(SLONG csx,SLONG csy,SLONG pixelw,SLONG red,SLONG green,SLONG blue,SLONG shadow,SLONG edge)
{
	SLONG	dx,dy,px,py;
	SLONG	r,g,b;
	UBYTE	clipped=1;





	//
	// Optimise later
	//


	if(shadow==0 && edge==0)
	{
		draw_quick_rect(csx,csy,pixelw,red,green,blue);
		return;
	}


	if(csx>clip_left && csx+pixelw < clip_right && csy>clip_top && csy+pixelw<clip_bot)
		clipped=0;



	for(dx=0;dx<pixelw;dx++)
	{

		for(dy=0;dy<pixelw;dy++)
		{
			r=red;
			g=green;
			b=blue;

			px=csx+dx;
			py=csy+dy;

			if(clipped)
			{
				if(px<clip_left||px>=clip_right||py<clip_top||py>=clip_bot)
					continue;
			}

			if(dx==0&& (edge&EDGE_LEFT))
			{
				r=0;
				g=0;
				b=0;
			}
/*
			if(dx==pixelw-1&& (edge&EDGE_RIGHT))
			{
				r=0;
				g=0;
				b=0;
			}
*/
			if(dy==0&& (edge&EDGE_TOP))
			{
				r=0;
				g=0;
				b=0;
			}
/*
			if(dy==pixelw-1&& (edge&EDGE_BOTTOM))
			{
				r=0;
				g=0;
				b=0;
			}
*/


			switch(shadow)
			{
				case	0:
					break;
				case	1:
					//   ..X
					//	 .oX
					//	 ,oX

					if(dx+dy<pixelw && dx<(pixelw>>1))
					{
						r>>=1;
						g>>=1;
						b>>=1;
					}
					break;

				case	6:
				case	2:
					//   ,ox
					//	 ,ox
					//	 ,ox
					if(dx<(pixelw>>1))
					{
						r>>=1;
						g>>=1;
						b>>=1;
					}

					break;
				case	3:
					//   ,ox
					//	 ,oo
					//	 .,,
					if(dx<(pixelw>>1) && dy>(pixelw>>1))
					{
						r>>=1;
						g>>=1;
						b>>=1;
					}
					break;
				case	4:
					//   xxx
					//	 ooo
					//	 ,,,
					if(dy>(pixelw>>1))
					{
						r>>=1;
						g>>=1;
						b>>=1;
					}
					break;
				case	5:
					//   ...
					//	 ..
					//	 ...
					if(dx<(pixelw>>1) || dy>(pixelw>>1))
					{
						r>>=1;
						g>>=1;
						b>>=1;
					}
					break;
				case	7:
					//   ...
					//	 ...
					//	 ...
					if(dx+dy>pixelw && dy>(pixelw>>1))
					{
						r>>=1;
						g>>=1;
						b>>=1;
					}
					break;

			}

			 screenmem[px*3+2+py*640*3]=(UBYTE)r;
			 screenmem[px*3+1+py*640*3]=(UBYTE)g;
			 screenmem[px*3+0+py*640*3]=(UBYTE)b;
		}
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

void	map_beacon_draw(SLONG x,SLONG z,ULONG col,ULONG flag,UWORD dir)
{
	UBYTE	radius;
	SLONG	screen_pitch;
	SLONG	mx,mz;
	SLONG	size=3;

	mx=x>>8;
	mz=z>>8;

	//
	// how many pixels per mapwho
	//

	if(!(player_visited[mx>>3][mz]&(1<<(mx&7))))
	{
		if (flag & BEACON_FLAG_BEACON)
		{
			//
			// Always draw beacons.
			//
		}
		else
		{
			return;
		}
	}

	get_screen_xy(&x,&z);

	if (flag & BEACON_FLAG_BEACON)
	{
		if(GAME_TURN&1)
			size=6;
		else
			size=3;
	}

	draw_shadow_rect(x-size,z-size,size<<1,0,0,0 ,0,0);
	draw_shadow_rect(x-size+1,z-size+1,(size-1)<<1,col>>16,(col&0xff00)>>8,(col&0xff) ,0,0);

	if (flag & BEACON_FLAG_POINTY)
	{
		SLONG dx = -SIN(dir);
		SLONG dz = -COS(dir);

		SLONG px;
		SLONG pz;

		px = x + (dx >> 14);
		pz = z + (dz >> 14);

		size -= 1;

		draw_shadow_rect(px-size,pz-size,size<<1,0,0,0 ,0,0);

		px = x + (dx >> 13);
		pz = z + (dz >> 13);

		size -= 1;

		draw_shadow_rect(px-size,pz-size,size<<1,0,0,0 ,0,0);
	}
}

void plan_view_shot(SLONG wx,SLONG wz,SLONG pixelw,SLONG sx,SLONG sy,SLONG w,SLONG h,UBYTE *mem)
{
	SLONG	minx,maxx,minz,maxz;
	SLONG	lminx,lmaxx,lminz,lmaxz;
	SLONG	x,z,csx,csy,c0;
	SLONG	r,g,b,shadow;
	SLONG	edge;
	UBYTE	*m;

	MFFileHandle	image_file;
	SLONG	height;
	UBYTE  *image;

extern UBYTE* image_mem;
	memcpy(mem,image_mem,640*480*3);
	// yay for disk caches :-p


	screen_width=w;
	screen_height=h;
	screen_x=sx;
	screen_y=sy;
	screen_pitch=(screen_width<<8)/pixelw;
	screen_mx=wx;
	screen_mz=wz;
	block_size=pixelw;

	m=mem;


/*	for(c0=0;c0<640*480;c0++)
	{
		*m++=244;
		*m++=231;
		*m++=177;
	}*/

	clip_left=sx;
	clip_right=sx+w;
	clip_top=sy;
	clip_bot=sy+h;
	screenmem=mem;

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
							r = 100;
							g = 100;
							b = 100;

							draw_shadow_rect(csx,csy,pixelw,r,g,b,0,edge);
						}
						else
						{
							shadow = PAP_2HI(x,z).Flags & 0x7;

							r = 177;
							g = 231;
							b = 244;

							if (shadow)
							{
								draw_shadow_rect(csx,csy,pixelw,r,g,b,shadow,edge);
							}
						}




					}
					else
					{
						r=((PAP_2HI(x,z).Height))+140;
						if(r>255)
							r=255;
						g=r;
						b=r;
						if(player_visited[x>>3][z]&(1<<(x&7)))
						{
	//						r=255;
						}

						shadow = PAP_2HI(x,z).Flags & 0x7;
						draw_shadow_rect(csx,csy,pixelw,r,g,b,shadow,edge);
					}
				}
				else
				;//	draw_quick_rect(csx,csy,pixelw,0,0,0);

			}
			csx+=pixelw;
		}
		csx=sx;
		csy+=pixelw;
	}


}



