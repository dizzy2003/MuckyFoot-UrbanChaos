#include	"game.h"
#include	"c:\fallen\headers\supermap.h"
#include	"c:\fallen\headers\pap.h"
#include	"c:\fallen\headers\inside2.h"
#include	"c:\fallen\headers\ob.h"
#ifndef	PSX
#include	"c:\fallen\editor\headers\Editor.hpp"
#endif
#include	"memory.h"
#include	"mav.h"
#include	"noserver.h"
#ifndef	PSX
#include	"c:\fallen\ddengine\headers\texture.h"
#include	"c:\fallen\headers\env.h"
#endif

#ifdef EDITOR
extern	void	copyfile_to_level(CBYTE *str);
extern	TGA_Info TGA_load_psx(const CBYTE *file,SLONG        max_width,SLONG        max_height,UBYTE   *data,UBYTE *pal);
#endif
extern	UBYTE	roper_pickup;


#ifdef TARGET_DC
#include "target.h"
#endif

/*
"Performance Analyser"  -PA

This piece of kit really is the most fantastic programming tool that has ever existed on any platform.
I really can't complement it enough. 

Programmers thrive on information, games development is sort of like having a black box with mysterious inner workings, you change the stuff you put into the box (the code) then you watch the different results that get displayed on the screen. And you have to deduce what's going on in the black box from this poke it and see what happens sort of trial and error. Of course it's not quite this bad, as the black box usually has lots of documentation about what's inside it. But when you cosider the black box that is the playstation contains multiple processors all doing millions of things a second, and all interacting with each other you can see that no matter how much documentation you have it's not going to tell you what your particular bit of code is doing.

And this is where the PA comes in.

It basically tells you exactly what each part of the playstation is doing in time steps of the order of a millionth of a second and it does it in nice colour coded histograms that let you zoom in and out without loss of information. At its furthest zoom out, (seeing the whole of a gameturn on screen at once) you can see exactly how much work the polygon drawing hardware (gpu)  was doing compared to the processor. The perfect game would have both running to capacity, with neither waiting for the other. So the PA lets you tweak the code until it as nearly as possible reaches this state of perfection.

The PA has a few other nifty features, it's operated by a foot peddle, so to the uninitialised it looks like your operating it by mind control. The PA is constantly storing information about the running of your program ( or even released games, so you can have a snoop on the opposition)  and when you press the trigger it dumps the information it has stored so far (upto about 8 60ths of a second worth), so when you get a glitch onscreen you press the trigger and it dumps the information showing you what lead upto the glitch (and hopefully the cause).
Another cool trick it has, is it can show you polygon overwrite, it shows you how many times each on screen pixel has been written or read. Everytime a polygon causes a pixel to be drawn over a previously drawn pixel, your wasting gpu time, minimal polygon overwrite is another of these states of perfection a good engine strives to achieve.

The PA easily doubled the speed of Urban Chaos on the PSX after just 3 weeks of use.


The only other trick up the playstation sleave is it's age means that the are a lot of very experienced programmers out there who are capable of pushing it to the limit. 

The dreamcast could run at very high resolutions, but the great equaliser is the NTSC/PAL/SECAM television standards which limit the resolutions of consoles  to 640x256 (higher than this requires interlace mode which looks nasty) So on screen resolution the PSX,N64 and Dreamcast are playing on a level pitch (until HD TV arrives)

Mike.

P.S I hope this is the sort of thing you wanted, I also hope I haven't crossed any Sony NDA boundaries



This piece of kit really is the most fantastic programming tool that has ever existed on any platform.
I really can't complement it enough. 

Programmers thrive on information, games development is sort of like having a black box with mysterious inner workings, you change the stuff you put into the box (the code) then you watch the different results that get displayed on the screen. And you have to deduce what's going on in the black box from this poke it and see what happens sort of trial and error. Of course it's not quite this bad, as the black box usually has lots of documentation about what's inside it. But when you cosider the black box that is the playstation contains multiple processors all doing millions of things a second, and all interacting with each other you can see that no matter how much documentation you have it's not going to tell you what your particular bit of code is doing.

And this is where the PA comes in.

It basically tells you exactly what each part of the playstation is doing in time steps of the order of a millionth of a second and it does it in nice colour coded histograms that let you zoom in and out without loss of information. At its furthest zoom out, (seeing the whole of a gameturn on screen at once) you can see exactly how much work the polygon drawing hardware (gpu)  was doing compared to the processor. The perfect game would have both running to capacity, with neither waiting for the other. So the PA lets you tweak the code until it as nearly as possible reaches this state of perfection.

The PA has a few other nifty features, it's operated by a foot peddle, so to the uninitialised it looks like your operating it by mind control. The PA is constantly storing information about the running of your program ( or even released games, so you can have a snoop on the opposition)  and when you press the trigger it dumps the information it has stored so far (upto about 8 60ths of a second worth), so when you get a glitch onscreen you press the trigger and it dumps the information showing you what lead upto the glitch (and hopefully the cause).
Another cool trick it has, is it can show you polygon overwrite, it shows you how many times each on screen pixel has been written or read. Everytime a polygon overwrites 
  */

SLONG	find_connect_wall(SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG *connect_storey,SLONG storey);
SLONG	add_dfacet(SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG y,SLONG count,SLONG style_index,SLONG storey_type,SLONG flags,SLONG offsety,SLONG block_height);
SLONG	add_painted_textures(UBYTE *t,SLONG tcount,SLONG style);


extern	SLONG	save_psx;
extern SLONG editor_texture_set;

ULONG	level_index=0;


//
// temporary!
//
//UBYTE	inside_mem[MAX_INSIDE_MEM];




SLONG	next_dbuilding=1;
SLONG	next_dwalkable=1;
SLONG	next_dfacet=1;
SLONG	next_dstyle=1;
SWORD	next_facet_link=1;
SWORD	next_paint_mem=1;
SWORD	next_dstorey=1;
SWORD	facet_link_count=0;

SLONG	next_inside_mem=1;
//SLONG	next_inside_rect=1;

UBYTE SUPERMAP_counter[2];
extern	SLONG	TEXTURE_set;

#define	NO_MOVE_UP		1
#define	NO_MOVE_RIGHT	2
#define	NO_MOVE_DOWN	4
#define	NO_MOVE_LEFT	8
#define	DOOR_UP		16
#define	DOOR_RIGHT	32
#define	DOOR_DOWN	64
#define	DOOR_LEFT	128

#ifdef EDITOR
#ifndef	PSX

//
// returns inside_index && room id for position in world
//
UWORD	calc_inside_for_xyz(SLONG x,SLONG y,SLONG z,UWORD *room)
{
	SLONG	c0;
	SLONG	mx,mz;
	mx=x>>8;
	mz=z>>8;
	for(c0=1;c0<next_inside_storey;c0++)
	{
		if(mx>=inside_storeys[c0].MinX)
		if(mx<inside_storeys[c0].MaxX)
		if(mz>=inside_storeys[c0].MinZ)
		if(mz<inside_storeys[c0].MaxZ)
		{
			//
			// in x,z range
			//


			if(y>inside_storeys[c0].StoreyY && y<inside_storeys[c0].StoreyY+256)
			{
				SLONG	r;

				r=find_inside_room(c0,mx,mz);

				if(r)
				{
					*room=r;
					return(c0);
				}
			}
		}
	}

	*room=0;
	return(0);
}


void	add_wall(UBYTE *map,SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG door_flag)
{
	SLONG	dx,dz;

//	if(x1==15616&&x2==15616&&z1==14592&&z2==14080)
//		ASSERT(0);

	dx=x2-x1;
	dz=z2-z1;

	if(dx&&dz)
	{
		//
		// inside walls should be orthogonal
		//
		return;
	}

	dx>>=8;
	dz>>=8;

	if(dx)
	{
		SLONG	off_x;

		if(dx<0)
		{
			off_x=-256;
			x1=x2;
			dx=-dx;
		}
		else
		{
			off_x=256;
		}

		while(dx>0)
		{

			map[(x1>>8)+(z1>>8)*128]|=NO_MOVE_UP;
			if((z1>>8)-1>=0)
				map[(x1>>8)-128+(z1>>8)*128]|=NO_MOVE_DOWN;
			if(door_flag)
			{
				map[(x1>>8)+(z1>>8)*128]|=DOOR_UP;
				if((z1>>8)-1>=0)
					map[(x1>>8)-128+(z1>>8)*128]|=DOOR_DOWN;
			}
			x1+=256; //off_x;
			dx--; //=off_x;
		}

	}

	if(dz)
	{

		SLONG	off_z;

		if(dz<0)
		{
			z1=z2;
			dz=-dz;
			off_z=-256;
		}
		else
		{
			off_z=256;
		}

		while(dz>0)
		{

			map[(x1>>8)+(z1>>8)*128]|=NO_MOVE_LEFT;
			if((x1>>8)-1>=0)
				map[(x1>>8)-1+(z1>>8)*128]|=NO_MOVE_RIGHT;
			if(door_flag)
			{
				map[(x1>>8)+(z1>>8)*128]|=DOOR_LEFT;
				if((x1>>8)-1>=0)
					map[(x1>>8)-1+(z1>>8)*128]|=DOOR_RIGHT;
			}
			z1+=256; //off_z;
			dz--; //=off_z;
		}

	}
}

UBYTE	*fill_map;
UBYTE	*fill_col;
SLONG	fill_minx,fill_maxx,fill_minz,fill_maxz;

void	flood_fill_at(UBYTE x,UBYTE z,UBYTE id)
{

	DebugText(" flood %d %d with %d right %d left %d up %d down %d\n",x,z,id,fill_col[x+z*128]&NO_MOVE_RIGHT,fill_col[x+z*128]&NO_MOVE_LEFT,fill_col[x+z*128]&NO_MOVE_UP,fill_col[x+z*128]&NO_MOVE_DOWN);
	fill_map[x+z*128]=id;
	if(x+1<128 && x+1<fill_maxx)
		if( (!(fill_col[x+z*128]&NO_MOVE_RIGHT)) && fill_map[x+1+z*128]!=id)
		{
			DebugText("R \n");
			flood_fill_at(x+1,z,id);
		}
	if(x-1>=0 && x-1>=fill_minx)
		if( (!(fill_col[x+z*128]&NO_MOVE_LEFT)) && fill_map[x-1+z*128]!=id)
		{
			DebugText("L \n");
			flood_fill_at(x-1,z,id);
		}
	if(z+1<128 && z+1<fill_maxz)
		if( (!(fill_col[x+z*128]&NO_MOVE_DOWN)) && fill_map[x+128+z*128]!=id)
		{
			DebugText("D \n");
			flood_fill_at(x,z+1,id);
		}
	if(z-1>=0 && z-1>=fill_minz)
		if( (!(fill_col[x+z*128]&NO_MOVE_UP)) && fill_map[x-128+z*128]!=id)
		{
			DebugText("U \n");
			flood_fill_at(x,z-1,id);
		}
}


void	flood_fill_rooms(SLONG storey)
{
	SLONG	c0;
	SLONG	id=1;
	SLONG	x,z,px,pz;
	SLONG	inside;
	inside=storey_list[storey].InsideIDIndex;

	for(c0=0;c0<MAX_ROOMS_PER_FLOOR;c0++)
	{
		if(room_ids[inside].Flag[c0])
		{
			flood_fill_at(room_ids[inside].X[c0],room_ids[inside].Y[c0],c0+1);
		}
	}

}

SLONG	build_inside_data(SLONG minx,SLONG minz,SLONG maxx,SLONG maxz,SLONG y,SLONG offset_y)
{
	SLONG	x,z;
	inside_storeys[next_inside_storey].InsideBlock=next_inside_block;
	for(z=minz;z<maxz;z++)
	{
		for(x=minx;x<maxx;x++)
		{
			inside_block[next_inside_block++]=fill_map[x+z*128];
		}
	}
	inside_storeys[next_inside_storey].MinX=minx;
	inside_storeys[next_inside_storey].MinZ=minz;
	inside_storeys[next_inside_storey].MaxX=maxx;
	inside_storeys[next_inside_storey].MaxZ=maxz;
	inside_storeys[next_inside_storey].StairCaseHead=0;
	inside_storeys[next_inside_storey].FacetStart=next_dfacet;
	inside_storeys[next_inside_storey].StoreyY=y+offset_y;

	next_inside_storey++;
	return(next_inside_storey-1);

}

SLONG	add_stairs_to_inside(SLONG inside,UBYTE x,UBYTE z,UBYTE flags,UWORD next_floor,UWORD prev_floor)
{
	UWORD	next;
	next=inside_storeys[inside].StairCaseHead;

	inside_storeys[inside].StairCaseHead=next_inside_stair;

	inside_stairs[next_inside_stair].NextStairs=next;
	inside_stairs[next_inside_stair].UpInside=next_floor;
	inside_stairs[next_inside_stair].DownInside=prev_floor;
	inside_stairs[next_inside_stair].X=x;
	inside_stairs[next_inside_stair].Z=z;

	inside_stairs[next_inside_stair].Flags=flags;

	next_inside_stair++;
	return(next_inside_stair-1);

}

void set_stair_link(SLONG start_inside,SLONG end_inside,SLONG link_stair,SLONG y,SLONG up)
{
	SLONG	c0;
	SLONG	stair;

	for(c0=start_inside;c0<end_inside;c0++)
	{
		SLONG	dy;

		dy=inside_storeys[c0].StoreyY-y;
		if((up && dy>0&& dy<300) || ((!up) && dy<0&& dy>-300))
		{
			stair=inside_storeys[c0].StairCaseHead;
			while(stair)
			{
				if(inside_stairs[stair].X==inside_stairs[link_stair].X && inside_stairs[stair].Z==inside_stairs[link_stair].Z && (inside_stairs[stair].Flags&0xc)==(inside_stairs[link_stair].Flags&0xc) )
				{
					if(up && inside_stairs[stair].Flags&STAIR_FLAG_DOWN)
					{
						inside_stairs[link_stair].UpInside=c0;
					}
					else
					if((!up) && inside_stairs[stair].Flags&STAIR_FLAG_UP)
					{
						inside_stairs[link_stair].DownInside=c0;

					}
				}
				stair=inside_stairs[stair].NextStairs;
			}
		}
	}
}

void link_inside_stairs(SLONG start_inside,SLONG end_inside)
{
	SLONG	c0;
	SLONG	stair;

	for(c0=start_inside;c0<end_inside;c0++)
	{
		stair=inside_storeys[c0].StairCaseHead;
		while(stair)
		{
			if(inside_stairs[stair].UpInside==0)
				set_stair_link(start_inside,end_inside,stair,inside_storeys[c0].StoreyY,1);
			if(inside_stairs[stair].DownInside==0)
				set_stair_link(start_inside,end_inside,stair,inside_storeys[c0].StoreyY,0);

			stair=inside_stairs[stair].NextStairs;
		}

	}
}

void	add_inside_facets(SLONG storey,SLONG offset_y)
{
	SLONG	px,pz,x,z,wall;
	SLONG	outside_storey;
	SLONG	pass1=1;

	outside_storey=storey;


	while(storey)
	{
		SLONG	style_index,count;
		UBYTE	*texture,*texture2,count2;
		SLONG	height;
		wall=storey_list[storey].WallHead;
		px=storey_list[storey].DX;
		pz=storey_list[storey].DZ;



		while(wall)
		{

			x=wall_list[wall].DX;
			z=wall_list[wall].DZ;


//***************

			style_index=next_dstyle;
			dstyles[next_dstyle++]=wall_list[wall].TextureStyle;
			ASSERT(next_dstyle<MAX_DSTYLES);


			texture=wall_list[wall].Textures;
			count=wall_list[wall].Tcount;

			if(texture&&count)
			{
				dstyles[next_dstyle-1]=-add_painted_textures(texture,count,dstyles[next_dstyle-1]);
			}

			height=storey_list[storey].Height;

			if(storey_list[storey].StoreyFlags&FLAG_ISTOREY_DOOR)
			{
				add_dfacet(
					px,pz,
					x,z,
					storey_list[outside_storey].DY,
					4,
					style_index,
					STOREY_TYPE_INSIDE_DOOR,
					0,		// No facet flags 
					offset_y,
					64);	//need an offset_y
			}
			else
			{
				add_dfacet(
					px,pz,
					x,z,
					storey_list[outside_storey].DY,
					4,
					style_index,
					pass1?STOREY_TYPE_OINSIDE:STOREY_TYPE_INSIDE,
					0,		// No facet flags.
					offset_y,
					64);	//need an offset_y
			}

			texture2=wall_list[wall].Textures2;
			count2=wall_list[wall].Tcount2;

			if(pass1)
			{
				next_dstyle--;
			}
			if(texture2&&count2)
			{
				dstyles[next_dstyle++]=wall_list[wall].TextureStyle2;
				dstyles[next_dstyle-1]=-add_painted_textures(texture2,count2,dstyles[next_dstyle-1]);

				dfacets[next_dfacet-1].FacetFlags|=FACET_FLAG_2SIDED;

			}
			else
			if(wall_list[wall].TextureStyle2!=wall_list[wall].TextureStyle )
			{
				dstyles[next_dstyle++]=wall_list[wall].TextureStyle2;
				dfacets[next_dfacet-1].FacetFlags|=FACET_FLAG_2SIDED;
			}
			else
			if(pass1)
			{
				next_dstyle++;

			}


//***************

							
			px=x;
			pz=z;

			wall=wall_list[wall].Next;
		}
		if(pass1)
		{
			storey=storey_list[storey].InsideStorey;
			pass1=0;
		}
		else
		{
			storey=storey_list[storey].Next;
		}
	}


}
/*
void	flood_fill_behind_doors(SLONG storey)
{
	SLONG	wall;
	SLONG	id=1;
	SLONG	x,z,px,pz;
	storey=storey_list[storey].InsideStorey;

	while(storey)
	{
		if(storey_list[storey].StoreyFlags&FLAG_ISTOREY_DOOR)
		{
			wall=storey_list[storey].WallHead;
			px=storey_list[storey].DX;
			pz=storey_list[storey].DZ;

			if(wall)
			{
				SLONG	dx,dz;

				x=wall_list[wall].DX;
				z=wall_list[wall].DZ;

				dx=px-x;
				dz=pz-z;

				DebugText(" door dx %d dz %d at %d,%d\n",dx,dz,px>>8,pz>>8);


				if(dx>0)
					flood_fill_at(x>>8,(z-1)>>8,id);
				if(dx<0)
					flood_fill_at((px)>>8,(pz)>>8,id);
				if(dz>0)
					flood_fill_at((x-1)>>8,z>>8,id);
				if(dz<0)
					flood_fill_at((px)>>8,(pz)>>8,id);
				id++;

			}
		}
		storey=storey_list[storey].Next;
	}
}
*/
void	set_door_bits(SLONG storey)
{
	SLONG	wall;
	SLONG	px,pz;
	SLONG	x,z;

	while(storey)
	{

		if(storey_list[storey].StoreyFlags&FLAG_ISTOREY_DOOR)
		{
			wall=storey_list[storey].WallHead;
			px=storey_list[storey].DX;
			pz=storey_list[storey].DZ;

			if(wall)
			{
				SLONG	dx,dz;

				x=wall_list[wall].DX;
				z=wall_list[wall].DZ;

				dx=px-x;
				dz=pz-z;

				DebugText(" door dx %d dz %d at %d,%d\n",dx,dz,px>>8,pz>>8);


				if(dx>0)
				{

					fill_map[(x>>8)+(z>>8)*128]|=FLAG_DOOR_UP;
				}
				if(dx<0)
				{
					fill_map[(px>>8)+(pz>>8)*128]|=FLAG_DOOR_UP;

				}
				if(dz>0)
				{
					fill_map[(x>>8)+(z>>8)*128]|=FLAG_DOOR_LEFT;
				}
				if(dz<0)
				{
					fill_map[(px>>8)+(pz>>8)*128]|=FLAG_DOOR_LEFT;
				}

				px=x;
				pz=z;

			}
		}
		storey=storey_list[storey].Next;
	}
}
void	set_stair_bits(SLONG room)
{
	SLONG	c0;

	for(c0=0;c0<MAX_STAIRS_PER_FLOOR;c0++)
	{
		if(room_ids[room].StairFlags[c0])
		{
			SLONG	sx,sz;
			SLONG	bits=0;

			sx=room_ids[room].StairsX[c0];
			sz=room_ids[room].StairsY[c0];

//			fill_map[sx+(sz)*128]|=FLAG_INSIDE_STAIR;

			if(room_ids[room].StairFlags[c0]&STAIR_FLAG_DOWN)
			{
				bits|=FLAG_INSIDE_STAIR_DOWN;
			}
			if(room_ids[room].StairFlags[c0]&STAIR_FLAG_UP)
			{
				bits|=FLAG_INSIDE_STAIR_UP;

			}

			switch(GET_STAIR_DIR(room_ids[room].StairFlags[c0]))
			{
				case	0:
					fill_map[sx+(sz-1)*128]|=bits;
					fill_map[sx+(sz-2)*128]|=bits;

					//n
					break;
				case	1:
					fill_map[sx+(sz)*128]|=bits;
					fill_map[sx+1+(sz)*128]|=bits;

					//e
					break;
				case	2:
					fill_map[sx-1+(sz)*128]|=bits;
					fill_map[sx-1+(sz+1)*128]|=bits;

					//s
					break;
				case	3:
					fill_map[sx-1+(sz-1)*128]|=bits;
					fill_map[sx-2+(sz-1)*128]|=bits;
					//w
					break;

			}
		}
	}

}

SLONG	create_inside_rect(SLONG storey,SLONG offset_y)
{

	UBYTE	*rect,*dir;
	SLONG	max_x=0,max_z=0,min_x=0x7fffffff,min_z=0x7fffffff;
	SLONG	wall,px,pz,x,z;
	SLONG	orig_storey=storey;
	SLONG	pass1=1;
	SLONG	inside_storey,inside_room,outside_storey;
	SLONG	inside_index,c0;

	outside_storey=storey;
	inside_storey=storey_list[storey].InsideStorey;
	inside_room=storey_list[storey].InsideIDIndex;

	rect=(UBYTE*)MemAlloc(128*128);
	dir=(UBYTE*)MemAlloc(128*128);

	fill_map=rect;
	fill_col=dir;

//	storey=storey_list[storey].InsideStorey;

	while(storey)
	{
		wall=storey_list[storey].WallHead;
		px=storey_list[storey].DX;
		pz=storey_list[storey].DZ;

		if(px<min_x)
			min_x=px;
		if(px>max_x)
			max_x=px;
		if(pz<min_z)
			min_z=pz;
		if(pz>max_z)
			max_z=pz;


		while(wall)
		{

			x=wall_list[wall].DX;
			z=wall_list[wall].DZ;

			if(x<min_x)
				min_x=x;
			if(x>max_x)
				max_x=x;
			if(z<min_z)
				min_z=z;
			if(z>max_z)
				max_z=z;


			add_wall(dir,x,z,px,pz,storey_list[storey].StoreyFlags&FLAG_ISTOREY_DOOR);
			DebugText(" col after add wall %d,%d to %d,%d \n",x,z,px,pz);
			{
				SLONG	x,z;
				for(z=min_z>>8;z<=max_z>>8;z++)
				{
					for(x=min_x>>8;x<=max_x>>8;x++)
					{
						DebugText("%x ",fill_col[x+z*128]);

					}
					DebugText("\n");

				}
			}

			px=x;
			pz=z;

			wall=wall_list[wall].Next;
		}
		if(pass1)
		{
			storey=storey_list[storey].InsideStorey;
			pass1=0;
		}
		else
		{
			storey=storey_list[storey].Next;
		}
	}
	fill_minx=min_x>>8;
	fill_minz=min_z>>8;
	fill_maxx=max_x>>8;
	fill_maxz=max_z>>8;
	flood_fill_rooms(orig_storey);
	set_door_bits(storey_list[orig_storey].InsideStorey);

	set_stair_bits(inside_room);





	inside_index=build_inside_data(fill_minx,fill_minz,fill_maxx,fill_maxz,storey_list[outside_storey].DY,offset_y);

	add_inside_facets(outside_storey,offset_y);
	inside_storeys[inside_index].FacetEnd=next_dfacet;
	inside_storeys[inside_index].TexType=room_ids[storey_list[outside_storey].InsideIDIndex].FloorType;
	inside_storeys[inside_index].Building=next_dbuilding;

	//
	// Create the stair structures for this floor
	//
	for(c0=0;c0<MAX_STAIRS_PER_FLOOR;c0++)
	{
		
		if(room_ids[inside_room].StairFlags[c0])
		{
			add_stairs_to_inside(inside_index,room_ids[inside_room].StairsX[c0],room_ids[inside_room].StairsY[c0],room_ids[inside_room].StairFlags[c0],0,0);
		}
	}



	DebugText(" minx %d minz %d \n",fill_minx,fill_minz);
	DebugText(" maxx %d maxz %d \n",fill_maxx,fill_maxz);

	{
		SLONG	x,z;

		DebugText("FLOOD \n");
		for(z=min_z>>8;z<max_z>>8;z++)
		{
			for(x=min_x>>8;x<=max_x>>8;x++)
			{
				DebugText("%x ",fill_map[x+z*128]);

			}
			DebugText("\n");

		}
		for(z=min_z>>8;z<max_z>>8;z++)
		{
			for(x=min_x>>8;x<=max_x>>8;x++)
			{
				DebugText("%x ",fill_col[x+z*128]);

			}
			DebugText("\n");

		}

	}

	MemFree(rect);
	MemFree(dir);
	return(inside_index);
}

extern	UWORD	page_remap[];

SLONG	add_painted_textures(UBYTE *t,SLONG tcount,SLONG style)
{
	SLONG	c0=0,count=0;

	if(next_dstorey>MAX_DSTOREYS-5)
		return(0);

	dstoreys[next_dstorey].Style=style;

	dstoreys[next_dstorey].Index=next_paint_mem;

	while(c0<tcount)
	{
		if(t[c0])
			count=c0+1;
		c0++;
	}

	if(count!=0)
	{
		dstoreys[next_dstorey].Count=count;
		memcpy(&paint_mem[next_paint_mem],t,count);
		for(c0=0;c0<count;c0++)
		{
			SLONG	page;
			page=paint_mem[next_paint_mem+c0]&0x7f;
//			page=page_remap[page];

		}
		next_paint_mem+=count;
		next_dstorey++;
		return(next_dstorey-1);
	}
	else
	{
		return(-style);

	}
}

SLONG	add_painted_textures_reverse(UBYTE *t,SLONG tcount,SLONG style,SLONG len)
{
	SLONG	c0=0,count=0;

	if(next_dstorey>MAX_DSTOREYS-5)
		return(0);

	dstoreys[next_dstorey].Style=style;

	dstoreys[next_dstorey].Index=next_paint_mem;

	while(c0<tcount)
	{
		if(t[c0])
			count=c0+1;
		c0++;
	}

	if(count!=0)
	{
		dstoreys[next_dstorey].Count=count;
		memcpy(&paint_mem[next_paint_mem],t,count);
		for(c0=0;c0<count;c0++)
		{
			SLONG	page;
			page=paint_mem[next_paint_mem+c0]&0x7f;
//			page=page_remap[page];

		}
		next_paint_mem+=count;
		next_dstorey++;
		return(next_dstorey-1);
	}
	else
	{
		return(-style);

	}
}


void	clear_storey_data(UWORD building)
{
	SLONG	wall,storey;

	storey=building_list[building].StoreyHead;
	while(storey)
	{
		storey_list[storey].StoreyFlags&=~FLAG_STOREY_FACET_LINKED;
		
		switch(storey_list[storey].StoreyType)
		{
			case	STOREY_TYPE_NORMAL:
			case	STOREY_TYPE_FENCE:
			case	STOREY_TYPE_FENCE_FLAT:
			case	STOREY_TYPE_FENCE_BRICK:
			case	STOREY_TYPE_OUTSIDE_DOOR:
				wall=storey_list[storey].WallHead;
				while(wall)
				{
					wall_list[wall].WallFlags&=~FLAG_WALL_FACET_LINKED;
					wall=wall_list[wall].Next;
				}

				break;
			case	STOREY_TYPE_LADDER:
				/*

				wall=find_wall_for_fe(storey_list[storey].DX,storey_list[storey].DZ,building_list[building].StoreyHead);
				if(wall>=0)
					wall_for_ladder[wall]=storey;
				mark_map_with_ladder(storey);
				*/
				break;
		}
		storey=storey_list[storey].Next;
	}

}


SLONG	is_building_worth_saving(SLONG building)
{
	SLONG	storey;
	storey=building_list[building].StoreyHead;

	while(storey)
	{

		if(storey_list[storey].WallHead)
			return(1);

		storey=storey_list[storey].Next;
	}
	return(0);
}


void	find_minmax_alt_along_vect(SLONG *min_y,SLONG *max_y,SLONG x1,SLONG x2,SLONG z1,SLONG z2)
{
	SLONG	dx,dz,len;

	dx=x2-x1;
	dz=z2-z1;

	len=Root(dx*dx+dz*dz);

	if(len==0)
	{
		min_y=0;
		max_y=0;
		return;
	}

	dx=(dx<<10)/len;
	dz=(dz<<10)/len;

	x1<<=10;
	z1<<=10;
	len=len>>7;

	*min_y=9999999;
	*max_y=-9999999;
	while(len>=0)
	{
		SLONG	h;
		h=PAP_calc_height_at(x1>>10,z1>>10)<<3;
		if(h>*max_y)
			*max_y=h;
		if(h<*min_y)
			*min_y=h;
		len--;
		x1+=dx;
		z1+=dz;
	}
}

SLONG	add_dfacet(SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG y,SLONG count,SLONG style_index,SLONG storey_type,SLONG facet_flags,SLONG offsety,SLONG block_height)
{
	struct	DFacet	*p_facet;
	SLONG	min_y,max_y;
	SLONG	offsety_unclip;

	offsety_unclip=offsety;
	offsety&=(~63);

	

	if(next_dfacet>MAX_DFACETS-5)
		return(0);

	ASSERT(storey_type != STOREY_TYPE_PARTITION);
//	ASSERT(next_dfacet!=2335);
/*
	if(y==0)
		if(z1>>8==24 && z2>>8==24)
		if(x1>>8==16 && x2>>8==10)
			ASSERT(0);

	if(y==0)
		if(z1>>8==24 && z2>>8==24)
		if(x2>>8==16 && x1>>8==10)
			ASSERT(0);
*/
	if(storey_type==STOREY_TYPE_NORMAL)
	{
		SLONG	c0;

		if(x1<(4<<8) && x2<(4<<8) && z1>z2)
			return(0);

		if(x1>(124<<8) && x2>(124<<8) && z1<z2)
			return(0);

		if(z1>(124<<8) && z2>(124<<8) && x1>x2)
			return(0);

		if(z1<(4<<8) && z2<(4<<8) && x1<x2)
			return(0);

		for(c0=1;c0<next_dfacet;c0++)
		{
			p_facet=&dfacets[c0];
//			if(next_dfacet==2335 && c0==155)
//				ASSERT(0);

			if(p_facet->FacetType==STOREY_TYPE_NORMAL)
			{
				if(p_facet->x[0]==x1>>8)
				   if(p_facet->z[0]==z1>>8)
				   if(p_facet->x[1]==x2>>8)
				   if(p_facet->z[1]==z2>>8)
				   if(p_facet->Height==count)
				   if(p_facet->Y[0]==y+offsety)
				   if(p_facet->Y[1]==y+offsety)
				{
//					ASSERT(0);
				}

			}

		}
	}

	p_facet=&dfacets[next_dfacet++];
	p_facet->FHeight=0;
	p_facet->FacetType=storey_type;


	if(p_facet->FacetType == STOREY_TYPE_LADDER)
	{
		if(y==0)
		{
			//
			// Ladders at zero altitude are required to hug the floor
			//

			p_facet->Y[0]=PAP_calc_height_at(x1,z1);
			p_facet->Y[1]=p_facet->Y[0];
		}
		else
		{
			p_facet->Y[0]=y+offsety;
			p_facet->Y[1]=y+offsety;
		}
	}
	else
	if (p_facet->FacetType == STOREY_TYPE_FENCE_FLAT ||
		p_facet->FacetType == STOREY_TYPE_FENCE      ||
		p_facet->FacetType == STOREY_TYPE_FENCE_BRICK||
		p_facet->FacetType == STOREY_TYPE_OUTSIDE_DOOR)
	{
		if (facet_flags & FACET_FLAG_ONBUILDING)
		{
			//
			// Look for buildings?
			//

			p_facet->Y[0] = y + offsety;
			p_facet->Y[1] = y + offsety;
		}
		else
		{
			//
			// Fences hug the ground- but not the roads.
			//

			p_facet->Y[0]=PAP_calc_height_at(x1,z1);
			p_facet->Y[1]=PAP_calc_height_at(x2,z2);
		}
	}
	else
	if(y==0 &&((facet_flags&FACET_FLAG_HUG_FLOOR)==0))
	{
		SLONG	h;
		// search along facet length looking for strage alt's

		find_minmax_alt_along_vect(&min_y,&max_y,x1,x2,z1,z2);

//		if(min_y!=max_y || (offsety&63))
//			p_facet->FHeight=1;// just a flag to say have foundation

//		min_y>>=3;
//		max_y>>=3;

		//
		// massive bug, min_y mmax_y are in different scale to offset_y
		//
		h=offsety_unclip-min_y;
		p_facet->FHeight=(h+63)>>6;

		p_facet->Y[0]=y+(offsety);
		p_facet->Y[1]=y+(offsety);
	}
	else
	{
		p_facet->Y[0]=y+(offsety);
		p_facet->Y[1]=y+(offsety);
	}

	p_facet->x[0] = x1 >> 8;
	p_facet->x[1] = x2 >> 8;

	p_facet->z[0]        = z1 >> 8;
	p_facet->z[1]        = z2 >> 8;
	p_facet->Height      = count;
	p_facet->StyleIndex  = style_index;
	p_facet->FacetFlags  = facet_flags;
	p_facet->Building    = next_dbuilding;
	p_facet->BlockHeight = (block_height>>4); //in 16ths
	p_facet->Counter[0]  = 0;
	p_facet->Counter[1]  = 0;
	p_facet->Open        = 0;

	return(next_dfacet-1);
}

void	add_dbuilding(SLONG x,SLONG y,SLONG z,SLONG sf,SLONG ef,SLONG building)
{
	struct	DBuilding	*p_build;

	if(next_dbuilding>MAX_DBUILDINGS-5)
		return;

	p_build=&dbuildings[next_dbuilding++];
	p_build->X=x;
	p_build->Y=y;
	p_build->Z=z;

	p_build->StartFacet=sf;
	p_build->EndFacet=ef;
	p_build->Walkable=building_list[building].Walkable;

	p_build->Type = building_list[building].BuildingType;
	p_build->Ware = NULL;

	p_build->Counter[0] = 0;
	p_build->Counter[1] = 0;
}

SLONG	find_slow_connect_wall(SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG y,SLONG *connect_storey,SLONG building,SLONG height) //,UBYTE **texture,SLONG *count)
{
	SLONG	found=0;
	SLONG	wall;
	SLONG	fx1,fz1,fx2,fz2;
	SLONG	storey;

	storey=building_list[building].StoreyHead;

	while(storey)
	{
		switch(storey_list[storey].StoreyType)
		{
			case	STOREY_TYPE_NORMAL:
				if(storey_list[storey].Height==height)
				if(storey_list[storey].DY==y+height) //+256)
				{
					fx1=storey_list[storey].DX;
					fz1=storey_list[storey].DZ;
					
					wall=storey_list[storey].WallHead;
					while(wall)
					{
						fx2=wall_list[wall].DX;
						fz2=wall_list[wall].DZ;
						if(fx1==x1&&fz1==z1&&fx2==x2&&fz2==z2)
						{
							*connect_storey=storey;
							return(wall);
						}
						fx1=fx2;
						fz1=fz2;


						wall=wall_list[wall].Next;
					}
				}

				break;
			case	STOREY_TYPE_FENCE:
			case	STOREY_TYPE_FENCE_BRICK:
			case	STOREY_TYPE_FENCE_FLAT:
			case	STOREY_TYPE_OUTSIDE_DOOR:
				break;
		}
		storey=storey_list[storey].Next;
	}

	return(0);

}

void	create_cable_dfacet(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG wall)
{
	struct	DFacet	*p_facet;
	SLONG	len,dx,dy,dz,count;
	SLONG	c0;
	SLONG	step_angle1,step_angle2;

	y1+=PAP_calc_height_at(x1,z1);
	y2+=PAP_calc_height_at(x2,z2);

	dx=abs(x2-x1);
	dy=abs(y2-y1);
	dz=abs(z2-z1);

	len=QDIST3(dx,dy,dz);
	count=(len<<1)/ELE_SIZE;
	count+=2;

	dx=(x2-x1);
	dy=(y2-y1);
	dz=(z2-z1);

	if(dy==0)
	{

		step_angle1=1024/count;
		step_angle2=-step_angle1;

	}
	else
	{
		SLONG	c1,c2;
		SLONG	m;
		SLONG	d1,d2;
		if(len==0)
			len=1;
		m=(abs(dy)*190)/len;

		c1=128;  //== 0.5   along 
		c2=128;  //=0.5


		if(dy<0)
		{	  

			//
			// its going down hill so slow stepangle1 and fast step angle2
			//
			c1=c1+m;
			c2=c2-m;


		}
		else
		{
			c1=c1-m;
			c2=c2+m;
		}
		if(c1<0)
			c1=0;
		if(c1>256)
			c1=256;
		if(c2<0)
			c2=0;
		if(c2>256)
			c2=256;

		d1=((count*c1)>>8);
		d2=((count*c2)>>8);

		if(d1==0)
			d1=1;

		if(d2==0)
			d2=1;

		step_angle1=512/d1;
		step_angle2=-512/d2;

	}

	p_facet=&dfacets[next_dfacet++];
	p_facet->Height=count;

	p_facet->StyleIndex=(SWORD)step_angle1;
	p_facet->Building=(SWORD)step_angle2;

	p_facet->x[0] = x1 >> 8;
	p_facet->x[1] = x2 >> 8;

	p_facet->Y[0] = y1;
	p_facet->Y[1] = y2;

	p_facet->z[0] = z1 >> 8;
	p_facet->z[1] = z2 >> 8;

	p_facet->FacetType  = STOREY_TYPE_CABLE;
	p_facet->FacetFlags = FACET_FLAG_UNCLIMBABLE;//FACET_FLAG_CABLE|FACET_FLAG_UNCLIMBABLE;

	p_facet->FHeight=wall_list[wall].TextureStyle2;
}

void	create_dfacets_for_building(SLONG building)
{
	SLONG	x1,z1,x2,z2,y1,y2;
	SLONG	storey,wall;
	SLONG	connect_wall,connect_count,connect_storey,prev_wall;
	SLONG	style_index;
	SLONG	height;
	SLONG	offset_y;
	UBYTE	*texture;
	SLONG	count;
	SLONG	start_y;
	SLONG	inside;
	SLONG	facet;
	SLONG	start_inside,end_inside;
	SLONG	temp_next_dfacet;
	SLONG   facet_flags;
	SLONG	circular;

	offset_y=building_list[building].OffsetY;

	temp_next_dfacet=next_dfacet;

	clear_storey_data(building);

	start_inside=next_inside_storey;
	
	storey=building_list[building].StoreyHead;
	if(storey_list[storey].StoreyType==STOREY_TYPE_NORMAL)
	{
		circular=is_storey_circular(storey);
	}
	else
	{
		circular=1;
	}

	while(storey)
	{
		if(building==6 || building==109)
		{
			DebugText(" building %d storey %d Build %d next %d y %d\n",building,storey,storey_list[storey].BuildingHead,storey_list[storey].Next,storey_list[storey].DY);
		}
//		if(storey_list[storey].InsideStorey||storey_list[storey].InsideIDIndex)
//			inside=create_inside_rect(storey,offset_y);

//		ASSERT(storey!=169);

		inside = 0;

		switch(storey_list[storey].StoreyType)
		{
			case	STOREY_TYPE_NORMAL:

				x1=storey_list[storey].DX;
				z1=storey_list[storey].DZ;

				wall=storey_list[storey].WallHead;


				while(wall)
				{
					SLONG	facet_flags=0;
//					ASSERT(next_dfacet!=64);
					x2=wall_list[wall].DX;
					z2=wall_list[wall].DZ;
					if((wall_list[wall].WallFlags&FLAG_WALL_FACET_LINKED)==0)
					{
						SLONG	height;
						if(!circular)
						{
							facet_flags |= FACET_FLAG_UNCLIMBABLE;
						}
						if(!(wall_list[wall].WallFlags & FLAG_WALL_CLIMBABLE))
						{
							facet_flags |= FACET_FLAG_UNCLIMBABLE;
						}
						else
							facet_flags=0;



						style_index=next_dstyle;
						wall_list[wall].WallFlags|=FLAG_WALL_FACET_LINKED;
						dstyles[next_dstyle++]=wall_list[wall].TextureStyle;
						ASSERT(next_dstyle<MAX_DSTYLES);

						texture=wall_list[wall].Textures;
						count=wall_list[wall].Tcount;

						if(texture&&count)
						{
							dstyles[next_dstyle-1]=-add_painted_textures(texture,count,dstyles[next_dstyle-1]);

						}
						
						if (building_list[building].BuildingType == BUILDING_TYPE_WAREHOUSE  )
						{
							SLONG	count2;
							UBYTE	*texture2;

							dstyles[next_dstyle++]=wall_list[wall].TextureStyle2;

							texture2=wall_list[wall].Textures2;
							count2=wall_list[wall].Tcount2;

							if(texture2&&count2)
							{
								dstyles[next_dstyle-1]=-add_painted_textures(texture2,count2,dstyles[next_dstyle-1]);


							}
						}



	//					connect_wall=find_connect_wall(x1,z1,x2,z2,&connect_storey,storey);
						height=storey_list[storey].Height;
						prev_wall=wall;
						connect_wall=find_slow_connect_wall(x1,z1,x2,z2,storey_list[storey].DY,&connect_storey,building,height); //,&texture,&count);
						connect_count=1;
						if(connect_wall)
						{
		//					LogText(" found a connect wall %d\n",connect_wall);
						
							while(connect_wall)
							{

								SLONG	ty;

								connect_count++;
								wall_list[connect_wall].WallFlags|=FLAG_WALL_FACET_LINKED;
								dstyles[next_dstyle++]=wall_list[connect_wall].TextureStyle;
								texture=wall_list[connect_wall].Textures;
								count=wall_list[connect_wall].Tcount;

								if(texture&&count)
								{
									dstyles[next_dstyle-1]=-add_painted_textures(texture,count,dstyles[next_dstyle-1]);

								}

								if (building_list[building].BuildingType == BUILDING_TYPE_WAREHOUSE)
								{
									SLONG	count2;
									UBYTE	*texture2;

									dstyles[next_dstyle++]=wall_list[connect_wall].TextureStyle2;

									texture2=wall_list[connect_wall].Textures2;
									count2=wall_list[connect_wall].Tcount2;

									if(texture2&&count2)
									{
										dstyles[next_dstyle-1]=-add_painted_textures(texture2,count2,dstyles[next_dstyle-1]);

										//dfacets[next_dfacet-1].FacetFlags|=FACET_FLAG_2SIDED;

									}
								}

								prev_wall=connect_wall;
								connect_wall=find_slow_connect_wall(x1,z1,x2,z2,storey_list[connect_storey].DY,&connect_storey,building,height); //,&texture,&count);
			//					connect_wall=find_connect_wall(x1,z1,x2,z2,&connect_storey,connect_storey);

							}
						}

						if (building_list[building].BuildingType == BUILDING_TYPE_WAREHOUSE)
						{
							facet_flags |= FACET_FLAG_2TEXTURED;
						}

						if(wall_list[prev_wall].WallFlags&FLAG_WALL_BARB_TOP)
						{
							facet_flags |= FACET_FLAG_BARB_TOP;

						}

						if(!circular)
						{
							facet_flags |= FACET_FLAG_2SIDED;
							facet_flags |= FACET_FLAG_HUG_FLOOR;

						}
						facet=add_dfacet(
							x1,z1,
							x2,z2,
							storey_list[storey].DY,
							connect_count*4,
							style_index,
							storey_list[storey].StoreyType,
							facet_flags,			// No facet flags.
							offset_y,
							height);

						if(wall_list[wall].WallFlags&FLAG_WALL_RECESSED)
						{
							dfacets[facet].DStorey=inside;
							dfacets[facet].FacetType=STOREY_TYPE_DOOR;
						}

						if(!(wall_list[wall].WallFlags&FLAG_WALL_RECESSED))
						if (building_list[building].BuildingType == BUILDING_TYPE_WAREHOUSE)
						{
							//
							//add again other way round
							//
							facet=add_dfacet(
								x2,z2,
								x1,z1,
								storey_list[storey].DY,
								connect_count*4,
								style_index+1,            // use same texture info offset by 1
								storey_list[storey].StoreyType,
								facet_flags|FACET_FLAG_INSIDE,			// No facet flags.
								offset_y,
								height);

						}



					}

					x1=x2;
					z1=z2;

					wall=wall_list[wall].Next;
				}

				break;

			case	STOREY_TYPE_CABLE:

				x1=storey_list[storey].DX;
				y1=storey_list[storey].DY;
				z1=storey_list[storey].DZ;

				wall=storey_list[storey].WallHead;

				while(wall)
				{

					x2=wall_list[wall].DX;
					y2=wall_list[wall].DY;
					z2=wall_list[wall].DZ;
					
					create_cable_dfacet(x1,y1,z1,x2,y2,z2,wall);

					wall=wall_list[wall].Next;
					x1=x2;
					y1=y2;
					z1=z2;
				}

				break;

			case	STOREY_TYPE_FENCE:
				height=4;
				goto	do_fence;
			case	STOREY_TYPE_FENCE_BRICK:
				height=storey_list[storey].Height/64;
				goto	do_fence;

			case	STOREY_TYPE_OUTSIDE_DOOR:
			case	STOREY_TYPE_FENCE_FLAT:
				height=storey_list[storey].Height/64;				

do_fence:;
				x1=storey_list[storey].DX;
				z1=storey_list[storey].DZ;

				if(storey==75)
					LogText("hello");

				facet_flags = 0;

				if (storey_list[storey].ExtraFlags & FLAG_STOREY_EXTRA_UNCLIMBABLE)
					{facet_flags |= FACET_FLAG_UNCLIMBABLE;}
				if (storey_list[storey].ExtraFlags & FLAG_STOREY_EXTRA_ONBUILDING)
					{facet_flags |= FACET_FLAG_ONBUILDING;}
				if (storey_list[storey].ExtraFlags & FLAG_STOREY_EXTRA_90DEGREE)
					{facet_flags |= FACET_FLAG_90DEGREE;}

				wall=storey_list[storey].WallHead;
				while(wall)
				{
					dstyles[next_dstyle++]=wall_list[wall].TextureStyle;
					texture=wall_list[wall].Textures;
					count=wall_list[wall].Tcount;
					if(texture&&count)
					{
						dstyles[next_dstyle-1]=-add_painted_textures(texture,count,dstyles[next_dstyle-1]);

					}


					x2=wall_list[wall].DX;
					z2=wall_list[wall].DZ;

					add_dfacet(
						x1,z1,
						x2,z2,
						storey_list[storey].DY,
						height,
						next_dstyle-1,
						storey_list[storey].StoreyType,
						facet_flags,
						offset_y,
						256);

					x1=x2;
					z1=z2;

					wall=wall_list[wall].Next;
				}

				break;

			case	STOREY_TYPE_LADDER:
				height=storey_list[storey].Height;				

				x1=storey_list[storey].DX;
				z1=storey_list[storey].DZ;

				wall=storey_list[storey].WallHead;
				dstyles[next_dstyle++]=wall_list[wall].TextureStyle;
				x2=wall_list[wall].DX;
				z2=wall_list[wall].DZ;

				add_dfacet(
					x1,z1,
					x2,z2,
					storey_list[storey].DY,
					height,
					next_dstyle-1,
					storey_list[storey].StoreyType,
					FACET_FLAG_UNCLIMBABLE,		// No facet flags.
					offset_y,
					256);

				break;

			case	STOREY_TYPE_TRENCH:

				x1=storey_list[storey].DX;
				z1=storey_list[storey].DZ;

				wall=storey_list[storey].WallHead;
				while(wall)
				{
					dstyles[next_dstyle++]=wall_list[wall].TextureStyle;
					x2=wall_list[wall].DX;
					z2=wall_list[wall].DZ;

//					add_dfacet(x1,z1,x2,z2,storey_list[storey].DY,4,next_dstyle-1,storey_list[storey].StoreyType,storey_list[storey].StoreyFlags);

					add_dfacet(
						x2,z2,
						x1,z1,
					   -256,
						4,
						next_dstyle-1,
						storey_list[storey].StoreyType,
						FACET_FLAG_UNCLIMBABLE,	// Facet flags
						offset_y,
						256);

					x1=x2;
					z1=z2;

					wall=wall_list[wall].Next;
				}
				break;


		}
		storey=storey_list[storey].Next;
	}

	end_inside=next_inside_storey;
	link_inside_stairs(start_inside,end_inside);



	add_dbuilding(0,0,0,temp_next_dfacet,next_dfacet,building);
}
#endif
#endif

SLONG first_walkable_prim_point;
SLONG number_of_walkable_prim_points;

SLONG first_walkable_prim_face4;
SLONG number_of_walkable_prim_faces4;

#ifndef PSX
void	load_walkables(MFFileHandle	handle,SLONG save_type)
{
	SLONG	next_point=1,next_face4=1,next_face3=1;

	SLONG	c0,c1;

	FileRead(handle,&next_dwalkable,2);
	FileRead(handle,&next_roof_face4,2);
	FileRead(handle,&dwalkables[0],sizeof(struct DWalkable)*next_dwalkable);
	FileRead(handle,&roof_faces4[0],sizeof(struct RoofFace4)*next_roof_face4);


/*	
	FileRead(handle,&next_dwalkable,2);
	FileRead(handle,&next_point,2);
	FileRead(handle,&next_face3,2);
	FileRead(handle,&next_face4,2);
	FileRead(handle,&dwalkables[0],sizeof(struct DWalkable)*next_dwalkable);


	if(save_type>=22)
	{
		FileRead(handle,&prim_points[next_prim_point],sizeof(struct PrimPoint)*next_point);
	}
	else
	{
		struct	OldPrimPoint	pp;
		SLONG	c0;

		for(c0=0;c0<next_point;c0++)
		{

			FileRead(handle,(UBYTE*)&pp,sizeof(struct	OldPrimPoint));
			prim_points[next_prim_point+c0].X=(SWORD)pp.X;
			prim_points[next_prim_point+c0].Y=(SWORD)pp.Y;
			prim_points[next_prim_point+c0].Z=(SWORD)pp.Z;
		}
	}


	FileRead(handle,&prim_faces3[next_prim_face3],sizeof(struct PrimFace3)*next_face3);
	FileRead(handle,&prim_faces4[next_prim_face4],sizeof(struct PrimFace4)*next_face4);

	for(c0=0;c0<next_face3;c0++)
	{
		for(c1=0;c1<3;c1++)
			prim_faces3[c0+next_prim_face3].Points[c1]+=next_prim_point;
	}

	for(c0=0;c0<next_face4;c0++)
	{
		for(c1=0;c1<4;c1++)
			prim_faces4[c0+next_prim_face4].Points[c1]+=next_prim_point;
		prim_faces4[c0+next_prim_face4].FaceFlags&=~FACE_FLAG_OUTLINE;
	}

	for(c0=1;c0<next_dwalkable;c0++)
	{
		SLONG	face;
		dwalkables[c0].StartPoint+=next_prim_point;
		dwalkables[c0].EndPoint+=next_prim_point;

		dwalkables[c0].StartFace3+=next_prim_face3;
		dwalkables[c0].EndFace3+=next_prim_face3;

		dwalkables[c0].StartFace4+=next_prim_face4;
		dwalkables[c0].EndFace4+=next_prim_face4;

		for(face=dwalkables[c0].StartFace4;face<dwalkables[c0].EndFace4;face++)
		{
			for(c1=0;c1<4;c1++)
			{
				ASSERT(prim_faces4[face].Points[c1]>=dwalkables[c0].StartPoint);
				ASSERT(prim_faces4[face].Points[c1]<=dwalkables[c0].EndPoint);

			}
		}

	}
*/

	first_walkable_prim_point      = 0;//next_prim_point;
	number_of_walkable_prim_points = 0;//next_point;

	first_walkable_prim_face4      = 0;//next_prim_face4;
	number_of_walkable_prim_faces4 = 0;//next_face4;

//	next_prim_point+=next_point;
//	next_prim_face3+=next_face3;
//	next_prim_face4+=next_face4;

	{
		struct	RoofFace4 *p_roof;
		SLONG	c0;
		p_roof=&roof_faces4[0];

		for(c0=0;c0<next_roof_face4;c0++)
		{
			if(p_roof->DY[0] ||p_roof->DY[1] ||p_roof->DY[2])
				p_roof->RZ|=(1<<7);
			else
				p_roof->RZ&=127;
			p_roof++;
		}
	}


}
#ifdef EDITOR

SLONG	roof_split_funny(struct	RoofFace4	*p_roof)
{
	SLONG	y0,y1,y2,y3;

	y0=0;
	y1=p_roof->DY[0];
	y2=p_roof->DY[2];
	y3=p_roof->DY[1];


	if(y1==y2 && y2==y3 && y0!=y1)
	{
		return(1);
	}
	if(y0==y1 && y1==y2 && y3!=y1)
	{
		return(1);
	}
	return(0);

}
void	add_flat_roof_to_pap(void)
{
	SLONG	c0,f4,p;
//	struct	RoofFace4	*p_roof;
	struct	PrimFace4	*p_quad;

	for(c0=1;c0<next_dwalkable;c0++)
	{
		SLONG	start;
		SLONG	building;
		SLONG	offset_y;

		building=dwalkables[c0].Building;

		offset_y=building_list[building].OffsetY;
		start=next_roof_face4;
		for(f4=dwalkables[c0].StartFace4;f4<dwalkables[c0].EndFace4;f4++)
		{
			SLONG	tx,ty;
			p_quad=&prim_faces4[f4];


			if((prim_points[p_quad->Points[1]].Y-prim_points[p_quad->Points[0]].Y)>>ROOF_SHIFT==0)
			if((prim_points[p_quad->Points[3]].Y-prim_points[p_quad->Points[0]].Y)>>ROOF_SHIFT==0)
			if((prim_points[p_quad->Points[2]].Y-prim_points[p_quad->Points[0]].Y)>>ROOF_SHIFT==0)
			if(building_list[building].BuildingType != BUILDING_TYPE_WAREHOUSE && building_list[building].BuildingType != BUILDING_TYPE_CRATE_IN)
			{
				edit_map[prim_points[p_quad->Points[0]].X>>8][prim_points[p_quad->Points[0]].Z>>8].Flags|=FLOOR_IS_ROOF;//PAP_FLAG_ROOF_EXISTS;
				MAVHEIGHT(prim_points[p_quad->Points[0]].X>>8,prim_points[p_quad->Points[0]].Z>>8)=(prim_points[p_quad->Points[0]].Y>>6);
			}
		}
	}
}

void	save_walkables(MFFileHandle	handle)
{
	SLONG	c0,f4,p;
	struct	RoofFace4	*p_roof;
	struct	PrimFace4	*p_quad;

	next_roof_face4=1;

	p_roof=&roof_faces4[1];
	for(c0=1;c0<next_dwalkable;c0++)
	{
		SLONG	start;
		SLONG	building;
		SLONG	offset_y;

		building=dwalkables[c0].Building;

		offset_y=building_list[building].OffsetY;
		start=next_roof_face4;
		for(f4=dwalkables[c0].StartFace4;f4<dwalkables[c0].EndFace4;f4++)
		{
			SLONG	tx,ty;
			p_quad=&prim_faces4[f4];

			p_roof->RX=prim_points[p_quad->Points[0]].X>>8;
			p_roof->RZ=prim_points[p_quad->Points[0]].Z>>8;
			p_roof->Y=prim_points[p_quad->Points[0]].Y&(~63);

			p_roof->DY[0]=((prim_points[p_quad->Points[1]].Y&(~63))-(prim_points[p_quad->Points[0]].Y&(~63)))>>ROOF_SHIFT;
			p_roof->DY[1]=((prim_points[p_quad->Points[3]].Y&(~63))-(prim_points[p_quad->Points[0]].Y&(~63)))>>ROOF_SHIFT;
			p_roof->DY[2]=((prim_points[p_quad->Points[2]].Y&(~63))-(prim_points[p_quad->Points[0]].Y&(~63)))>>ROOF_SHIFT;

			if(roof_split_funny(p_roof))
				p_roof->RX|=(1<<7);

			if(p_roof->DY[0] ||p_roof->DY[1] ||p_roof->DY[2] || building_list[building].BuildingType == BUILDING_TYPE_WAREHOUSE || building_list[building].BuildingType == BUILDING_TYPE_CRATE_IN)
			{
				p_roof->RZ|=(1<<7);

				tx = (p_quad->UV[0][0] + p_quad->UV[1][0] + p_quad->UV[2][0] + p_quad->UV[3][0]) >> 2;
				ty = (p_quad->UV[0][1] + p_quad->UV[1][1] + p_quad->UV[2][1] + p_quad->UV[3][1]) >> 2;


				tx>>=5;
				ty>>=5;

	//			p_roof->TexturePage=p_quad->TexturePage*64+tx+ty*8;  //leaves some bits free to describe rotation
				p_roof->Next=0;
				p_roof->DrawFlags=0;
				p_roof++;
				next_roof_face4++;
				ASSERT(next_roof_face4<MAX_ROOF_FACE4);
			}


		}
		dwalkables[c0].StoreyY+=offset_y>>6;
		dwalkables[c0].StartFace4=start;
		dwalkables[c0].EndFace4=next_roof_face4;
	}

	FileWrite(handle,&next_dwalkable,2);
	FileWrite(handle,&next_roof_face4,2);
	FileWrite(handle,&dwalkables[0],sizeof(struct DWalkable)*next_dwalkable);
	FileWrite(handle,&roof_faces4[0],sizeof(struct RoofFace4)*next_roof_face4);


}
void	save_walkables_old(MFFileHandle	handle)
{
	struct	PrimPoint	*points;
	struct	PrimFace4	*faces4;
	struct	PrimFace3	*faces3;

	SLONG	next_point=1,next_face4=1,next_face3=1;
	SLONG	max_point=1,max_face3=1,max_face4=1;

	SLONG	c0;

	//
	//This fucks up if you save twice in a row
	//
	

	for(c0=1;c0<next_dwalkable;c0++)
	{
		max_point+=dwalkables[c0].EndPoint-dwalkables[c0].StartPoint;
		max_face3+=dwalkables[c0].EndFace3-dwalkables[c0].StartFace3;
		max_face4+=dwalkables[c0].EndFace4-dwalkables[c0].StartFace4;
		LogText("LEGO walkable %d  points %d faces4 %d \n",c0,dwalkables[c0].EndPoint-dwalkables[c0].StartPoint,dwalkables[c0].EndFace4-dwalkables[c0].StartFace4);
	}

	points=(struct PrimPoint *)MemAlloc(sizeof(struct PrimPoint)*max_point);
	faces4=(struct PrimFace4 *)MemAlloc(sizeof(struct PrimFace4)*max_face4);
	faces3=(struct PrimFace3 *)MemAlloc(sizeof(struct PrimFace3)*max_face3);

	next_point=0;
	next_face3=0;
	next_face4=0;

	if(points&&faces4&&faces3)
	{
		for(c0=1;c0<next_dwalkable;c0++)
		{
			SLONG	sp,ep,sf,ef;
			SLONG	copy_to=next_point;
			SLONG	c1,c2;

			sp=dwalkables[c0].StartPoint;
			ep=dwalkables[c0].EndPoint;

			dwalkables[c0].StartPoint=next_point;
			dwalkables[c0].EndPoint=next_point+ep-sp;

			for(c1=sp;c1<ep;c1++)
			{
				points[next_point].X  =(SLONG)AENG_dx_prim_points[c1].X;
				points[next_point].Y  =(SLONG)AENG_dx_prim_points[c1].Y;
				points[next_point++].Z=(SLONG)AENG_dx_prim_points[c1].Z;
				ASSERT(next_point<max_point);
			}

			sf=dwalkables[c0].StartFace3;
			ef=dwalkables[c0].EndFace3;

			dwalkables[c0].StartFace3=next_face3;
			dwalkables[c0].EndFace3=next_face3+ef-sf;

			for(c1=sf;c1<ef;c1++)
			{
				faces3[next_face3]=prim_faces3[c1];
				for(c2=0;c2<3;c2++)
					faces3[next_face3].Points[c2]+=copy_to-sp;
				next_face3++;
				ASSERT(next_face3<=max_face3);

			}

			sf=dwalkables[c0].StartFace4;
			ef=dwalkables[c0].EndFace4;

			dwalkables[c0].StartFace4=next_face4;
			dwalkables[c0].EndFace4=next_face4+ef-sf;

			for(c1=sf;c1<ef;c1++)
			{
				//
				// do some stuff
				//

				faces4[next_face4]=prim_faces4[c1];

				for(c2=0;c2<4;c2++)
				{

					ASSERT(faces4[next_face4].Points[c2]>=sp)
					ASSERT(faces4[next_face4].Points[c2]<=ep)

				}

				for(c2=0;c2<4;c2++)
				{
					faces4[next_face4].Points[c2]+=copy_to-sp;
				}

				next_face4++;
				ASSERT(next_face4<=max_face4);
			}
		}
	}

	FileWrite(handle,&next_dwalkable,2);
	FileWrite(handle,&next_point,2);
	FileWrite(handle,&next_face3,2);
	FileWrite(handle,&next_face4,2);
	FileWrite(handle,&dwalkables[0],sizeof(struct DWalkable)*next_dwalkable);
	FileWrite(handle,&points[0],sizeof(struct PrimPoint)*next_point);
	FileWrite(handle,&faces3[0],sizeof(struct PrimFace3)*next_face3);
	FileWrite(handle,&faces4[0],sizeof(struct PrimFace4)*next_face4);
	
	if(points)
	{
		MemFree(points);
	}

	if(faces4)
	{
		MemFree(faces4);
	}

	if(faces3)
	{
		MemFree(faces3);
	}

}
//
// inside ob-ob's need to figure out what their inside_index is
//
extern	SLONG	build_psx;
SLONG	save_to_psx(SLONG index)
{
	if(!build_psx)
		return(1);

	switch(index)
	{
		case	21:
		case	22:
		case	24:
		case	170:
		case	171:
		case	173:
		case	174:
		case	175:
		case	176:
		case	183:
			return(0);
		
	}
		return(1);
}

void	save_ob_ob(MFFileHandle	handle)
{
	UWORD	temp;
	SLONG	c0;
	struct	MapThing	*t_mthing;
		//
		// save the prims as OB's
		//
		OB_init();	//get rid of any ob's that allready exist
		temp=0;
		for(c0=0;c0<MAX_MAP_THINGS;c0++)
		{
			t_mthing=&map_things[c0];

			switch(t_mthing->Type)
			{

				case	MAP_THING_TYPE_PRIM:		
					{
						UWORD	inside=0,room=0;
						UBYTE	flag=0;

						/*

						//
						// This isn't the flavour of the month with the artists anymore.
						// 

						if(t_mthing->Flags&FLAG_EDIT_PRIM_ON_FLOOR || (prim_objects[t_mthing->IndexOther].flag & PRIM_FLAG_ON_FLOOR) )
						{
							flag=OB_FLAG_ON_FLOOR;
						}
						else
						if(prim_objects[t_mthing->IndexOther].flag & PRIM_FLAG_JUST_FLOOR )
						{
							flag=OB_FLAG_JUST_FLOOR;
						}
						*/

//						if(t_mthing->Flags&FLAG_EDIT_PRIM_INSIDE)
						{
							inside=calc_inside_for_xyz(t_mthing->X,t_mthing->Y,t_mthing->Z,&room);
						}

						//
						// for each prim in the thing list create an ob
						//
						if(t_mthing->IndexOther!=217&&save_to_psx(t_mthing->IndexOther))
						OB_create(
							t_mthing->X,
							t_mthing->Y,
							t_mthing->Z,
							t_mthing->AngleY,	// Yaw!
							0,
							0,
							t_mthing->IndexOther,flag,inside,room);	// PrimObject.
					} 

					break;
				case	MAP_THING_TYPE_ANIM_PRIM:		
					temp++;
					break;

			}
		}
		FileWrite(handle,(UBYTE*)&OB_ob_upto,sizeof(OB_ob_upto));
		FileWrite(handle,(UBYTE*)&OB_ob[0],sizeof(OB_Ob)*OB_ob_upto);
		FileWrite(handle,(UBYTE*)&OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);

}

void	save_super_map(MFFileHandle	handle)
{
	SLONG	c0;
//void	remap_textures_psx(void);
//	remap_textures_psx();


	next_dbuilding=1;
	next_dfacet=1;
	next_dstyle=1;
	next_paint_mem=1;
	next_dstorey=1;

	for(c0=1;c0<MAX_BUILDINGS;c0++)
	{
		if(is_building_worth_saving(c0))
		{
			create_dfacets_for_building(c0);
		}
	}

	FileWrite(handle,&next_dbuilding,2);
	FileWrite(handle,&next_dfacet,2);
	FileWrite(handle,&next_dstyle,2);

	FileWrite(handle,&next_paint_mem,2);
	FileWrite(handle,&next_dstorey,2);
	
	FileWrite(handle,&dbuildings[0],sizeof(struct DBuilding)*next_dbuilding);
	FileWrite(handle,&dfacets[0],sizeof(struct DFacet)*next_dfacet);
	FileWrite(handle,&dstyles[0],sizeof(UWORD)*next_dstyle);
	FileWrite(handle,&paint_mem[0],sizeof(UBYTE)*next_paint_mem);
	FileWrite(handle,&dstoreys[0],sizeof(struct DStorey)*next_dstorey);

//
//	Inside data
//

	FileWrite(handle,&next_inside_storey,sizeof(next_inside_storey));
	FileWrite(handle,&next_inside_stair,sizeof(next_inside_stair));
	FileWrite(handle,&next_inside_block,sizeof(next_inside_block));

	FileWrite(handle,&inside_storeys[0],sizeof(struct InsideStorey)*next_inside_storey);
	FileWrite(handle,&inside_stairs[0],sizeof(struct Staircase)*next_inside_stair);
	FileWrite(handle,&inside_block[0],sizeof(UBYTE)*next_inside_block);

	save_walkables(handle);
	save_ob_ob(handle);

}
#endif

void	load_super_map(MFFileHandle	handle,SLONG save_type)
{
	SLONG	c0;

	next_dbuilding=1;
	next_dfacet=1;
	next_dstyle=1;

#ifndef TARGET_DC
	INDOORS_INDEX=0;
	INDOORS_INDEX_NEXT=0;
#endif

	FileRead(handle,&next_dbuilding,2);
	FileRead(handle,&next_dfacet,2);
	FileRead(handle,&next_dstyle,2);

	if(save_type>=17)
	{
		FileRead(handle,&next_paint_mem,2);
		FileRead(handle,&next_dstorey,2);
	}
	
	FileRead(handle,&dbuildings[0],sizeof(struct DBuilding)*next_dbuilding);
	FileRead(handle,&dfacets[0],sizeof(struct DFacet)*next_dfacet);
	FileRead(handle,&dstyles[0],sizeof(UWORD)*next_dstyle);
	if(save_type>=17)
	{
		FileRead(handle,&paint_mem[0],sizeof(UBYTE)*next_paint_mem);
		FileRead(handle,&dstoreys[0],sizeof(struct DStorey)*next_dstorey);
	}

	for(c0=1;c0<next_dfacet;c0++)
	{
		dfacets[c0].Dfcache=0;
	}

	if(save_type>=21)
	{
		FileRead(handle,&next_inside_storey,sizeof(next_inside_storey));
		FileRead(handle,&next_inside_stair,sizeof(next_inside_stair));
		FileRead(handle,&next_inside_block,sizeof(next_inside_block));

		FileRead(handle,&inside_storeys[0],sizeof(struct InsideStorey)*next_inside_storey);
		FileRead(handle,&inside_stairs[0],sizeof(struct Staircase)*next_inside_stair);
		FileRead(handle,&inside_block[0],sizeof(UBYTE)*next_inside_block);
	}
	load_walkables(handle,save_type);

	if(save_type>=23)
	{
/*		SLONG OB_ob_temp;
		FileRead(handle,(UBYTE*)&OB_ob_temp,sizeof(OB_ob_upto));
		FileSeek(handle,SEEK_MODE_CURRENT,sizeof(OB_Ob)*OB_ob_temp);
		FileSeek(handle,SEEK_MODE_CURRENT,sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
*/
		FileRead(handle,(UBYTE*)&OB_ob_upto,sizeof(OB_ob_upto));
		FileRead(handle,(UBYTE*)&OB_ob[0],sizeof(OB_Ob)*OB_ob_upto);

		//
		// Notice that strangely they have their very own mapwho
		//

		FileRead(handle,(UBYTE*)&OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
	}
}


//
// build2.cpp doesn't have an h-file for it!
//

void add_facet_to_map(SLONG facet);

void add_sewer_ladder(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2,
		SLONG bottom,
		SLONG height,
		SLONG link)
{
	DFacet *df;

	if (!WITHIN(next_dfacet, 0, MAX_DFACETS - 1))
	{
		//
		// No more facets left.
		//

		return;
	}

	//
	// Hmm...!
	//

	if (next_dfacet == 0)
	{
		next_dfacet = 1;
	}

	//
	// Create the dfacet.
	// 

	df = &dfacets[next_dfacet];

	df->FacetType  = STOREY_TYPE_LADDER;
	df->FacetFlags = FACET_FLAG_IN_SEWERS;
	df->x[0]       = x1 >> 8;
	df->z[0]       = z1 >> 8;
	df->x[1]       = x2 >> 8;
	df->z[1]       = z2 >> 8;
	df->Y[0]       = bottom;
	df->Y[1]       = bottom;
	df->Height     = height;

	if (link)
	{
		df->FacetFlags |= FACET_FLAG_LADDER_LINK;
	}

	//
	// Add it to the map.
	//

	add_facet_to_map(next_dfacet);

	next_dfacet += 1;

	return;
}

/*
void create_super_dbuilding(SLONG building)
{
	next_dbuilding=1;
	next_dfacet=1;
	next_dstyle=1;

	create_dfacets_for_building(building);
}
*/







//#endif
SLONG find_electric_fence_dbuilding(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG range)
{
	SLONG dist;

	SLONG best_dist  = INFINITY;
	SLONG best_facet = NULL;

	SLONG mx;
	SLONG mz;

	SLONG mx1, mx2;
	SLONG mz1, mz2;

	SLONG f_list;
	SLONG facet;
	SLONG build;
	SLONG exit;

	mx1 = (world_x - range) >> PAP_SHIFT_LO;
	mz1 = (world_z - range) >> PAP_SHIFT_LO;
	mx2 = (world_x + range) >> PAP_SHIFT_LO;
	mz2 = (world_z + range) >> PAP_SHIFT_LO;

	SATURATE(mx1, 0, PAP_SIZE_LO - 1);
	SATURATE(mz1, 0, PAP_SIZE_LO - 1);
	SATURATE(mx2, 0, PAP_SIZE_LO - 1);
	SATURATE(mz2, 0, PAP_SIZE_LO - 1);

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		//
		// Check all the facets on this mapsquare.
		//

		f_list = PAP_2LO(mx,mz).ColVectHead;

		if (f_list)
		{
			exit = FALSE;

			while(!exit)
			{
				facet=facet_links[f_list];

				ASSERT(facet);

				if (facet < 0)
				{
					//
					// The last facet in the list for each square
					// is negative.
					// 

					facet = -facet;
					exit  =  TRUE;
				}
				
				dist = distance_to_line(
							dfacets[facet].x[0] << 8,
							dfacets[facet].z[0] << 8,
							dfacets[facet].x[1] << 8,
							dfacets[facet].z[1] << 8,
							world_x,
							world_z);

				if (dist < best_dist)
				{
					best_dist  = dist;
					best_facet = facet;
				}

				f_list++;
			}
		}
	}

	if (best_facet == NULL)
	{
		return NULL;
	}
	else
	{
		return dfacets[best_facet].Building;
	}
}
#endif
//
// Sets the state of the given electric fence dbuilding. It sets the
// flags in all the facets of the dbuilding.
//

void set_electric_fence_state(SLONG dbuilding, SLONG onoroff)
{
	SLONG facet;

	for (facet = dbuildings[dbuilding].StartFacet; facet < dbuildings[dbuilding].EndFacet; facet++)
	{
		if (onoroff)
		{
			dfacets[facet].FacetFlags |=  FACET_FLAG_ELECTRIFIED;
		}
		else
		{
			dfacets[facet].FacetFlags &= ~FACET_FLAG_ELECTRIFIED;
		}
	}
}

void SUPERMAP_counter_increase(UBYTE which)
{
	SUPERMAP_counter[which] += 1;

	if (SUPERMAP_counter[which] == 0)
	{
		SLONG i;

		//
		// It has wrapped around. We must clear the counter in every dfacet.
		// 

		for (i = 1; i < next_dfacet; i++)
		{
			dfacets[i].Counter[which] = 0;
		}

		SUPERMAP_counter[which] = 1;
	}
}











#ifndef	PSX

struct	Levels
{
	CBYTE *name;
	CBYTE *map_name;
	UWORD level;
	ULONG	dontload;
};

struct	Levels levels_quick[]=
{
//	{"finale1","Roof1.map",32},
	{"BAALROG3","Balbash1.map",31},
//	{"gang order1","gang1.map",33},
	{"","",0}
};

struct Levels levels_demo[] =
{
	{"FTutor1", "fight1.map",1,0},
	{"freecd1","freeoncd1.map",1,0},
	{"","",0}
};

struct	Levels levels2[]=
{
	{"skymiss2","skymap30.map",25,0}, //problem
	{"","",0}
};

struct	Levels levels[]=
{
	
	{"FTutor1","fight1.map",1,0},    //0

	{"assault1","assault.map",2,0},	 //1
	{"testdrive1a","oval1.map",3,0}, //2
	{"fight1","fight1.map",4,0},	 //3
	{"police1","disturb1.map",5,0},	 //4
	{"testdrive2","road4_2.map",6,0},//5
	{"fight2","fight1.map",7,0},	 //6

									 
	{"police2","disturb1.map",8,0},	 //7
	{"testdrive3","road4_3.map",9,0},//8
	{"bankbomb1","gang1.map",10,0},	 //9
	{"police3","disturb1.map",11,0}, //10
	{"gangorder2","gang1.map",12,0}, //11
	{"police4","disturb1.map",13,0}, //12
	{"bball2","bball2.map",14,0},	 //13

	{"wstores1","gpost3.map",15,(1<<PERSON_HOSTAGE)|(1<<PERSON_TRAMP)|(1<<PERSON_MIB1)}, //14
	// here
	{"\\e3","snocrime.map",16,(1<<PERSON_TRAMP)|(1<<PERSON_MIB1)},//15
	{"westcrime1","disturb1.map",17,0},//16
	{"carbomb1","gang1.map",18,0},//17
	{"botanicc","botanicc.map",19,0},//18
	{"semtex","snocrime.map",20,(1<<PERSON_SLAG_FATUGLY)|(1<<PERSON_HOSTAGE)|(1<<PERSON_MIB1)},//19
	{"awol1","gpost3.map",21,(1<<PERSON_SLAG_TART)|(1<<PERSON_SLAG_FATUGLY)|(1<<PERSON_HOSTAGE)|(1<<PERSON_MECHANIC)|(1<<PERSON_MIB1)}, //20
	{"mission2","snocrime.map",22,(1<<PERSON_SLAG_FATUGLY)|(1<<PERSON_HOSTAGE)|(1<<PERSON_TRAMP)|(1<<PERSON_MIB1)}, //21
	{"park2","parkypsx03.map",23,0},//22


	{"mib","southside.map",24,(1<<PERSON_HOSTAGE)|(1<<PERSON_TRAMP)}, //23
	{"skymiss2","skymap30.map",25,0}, //problem //24
	{"factory1","tv1.map",26,(1<<PERSON_SLAG_FATUGLY)|(1<<PERSON_HOSTAGE)|(1<<PERSON_TRAMP)}, //25
	{"estate2","nestate01.map",27,0},//26
	{"stealtst1","stealth1.map",28,0},//27
	{"snow2","snow2.map",29,0},
	{"gordout1","botanicc.map",30,0},
	{"jung3","jung3.map",31,0},
	{"BAALROG3","Balbash1.map",32,0},
	{"finale1","Rooftest2.map",33,0},		//problem
	{"Gangorder1","gang1.map",34,0},
	{"","",0}



};


//
// create PSX tims (oh my god)
//

UWORD	pals16[256][64];  // a single page holds all pals
UBYTE	psx_tim_page[256][128];


UWORD	moved_from[16*64];
UWORD	moved_to[16*64];

struct	TimStuff
{
	int	Stb,ClutX,ClutY,PageX,PageY;
};

struct	TimStuff tim_stuff[]=
{
	{0,0,500,512+64*0,0},
	{0,0,500,512+64*1,0},
	{0,0,500,512+64*2,0},
	{0,0,500,512+64*3,0},
	{0,0,500,512+64*4,0},
	{0,0,500,512+64*5,0},
	{0,0,500,512+64*6,0},
	{0,0,500,512+64*7,0},

	{0,0,500,512+64*0,256},
	{0,0,500,512+64*1,256},
	{0,0,500,512+64*2,256},
	{0,0,500,512+64*3,256},
	{0,0,500,512+64*4,256},
	{0,0,500,512+64*5,256},
	{0,0,500,512+64*6,256},
	{0,0,500,512+64*7,256},

	{1,0,501,448,0},
	{1,0,502,576,0},
	{1,0,503,704,0},
	{1,0,504,832,0},
	{1,0,505,320,256},
	{1,0,506,448,256},
	{1,0,507,576,256},
	{1,0,508,704,256},
	{1,0,509,832,256}

};


SLONG	load_alt_pal(char *fname,UBYTE *pal)
{
	FILE *handle,*phandle;
	UBYTE	remap_pal[256*4];
	SLONG	c0;


	//
	// Open the file.
	//

	phandle = MF_Fopen(fname, "rb");
	if (phandle == NULL)
	{
		goto file_error;
	}

	if (fread(&pal[0], 1, 24, phandle) != 24) 
			goto file_error;

	if (fread(&remap_pal[0], 1, 256*4, phandle) != 256*4) 
			goto file_error;

	{
		SLONG	pal1=0,pal2=0,c0;
		for(c0=0;c0<16*4;c0++)
			pal1+=remap_pal[c0];

		for(c0=240*4;c0<256*4;c0++)
			pal2+=remap_pal[c0];

		if(pal2>pal1)
		{

			//
			// shunt the pallette down
			//
			for(c0=0;c0<16*4;c0++)
			{
				remap_pal[c0]=remap_pal[240*4+c0];
			}
		}

	}

	for(c0=0;c0<256;c0++)
	{
		pal[c0*3+0]=remap_pal[c0*4+0];
		pal[c0*3+1]=remap_pal[c0*4+1];
		pal[c0*3+2]=remap_pal[c0*4+2];
	}


	return(1);
file_error:;
	return(0);
}

void	save_tim(char *fname,unsigned char dat[256][128],SLONG index,SLONG copy_tom)
{
	FILE *handle;
	SLONG	x,y;

	fname[strlen(fname)-2]='o';

	handle = MF_Fopen(fname, "wb");
	if(handle)
	{
		fwrite(&dat[0][0], 1,256*128, handle); 
		MF_Fclose(handle);
#ifdef EDITOR
		if(copy_tom)
			copyfile_to_level(fname);
#endif
	}


	fname[strlen(fname)-2]='i';


	handle = MF_Fopen(fname, "wb");

	struct	
	{
		SLONG	ID;
		SLONG	Flag;
	}clut_head;

	if (handle)
	{
		SLONG	bnum;

		clut_head.ID=0x10;
		clut_head.Flag=0; // 4 bit no cluts in tim.   //9; //0x10|1; // has clut & clut is 8bit
		fwrite(&clut_head, 1, sizeof(clut_head), handle);

		bnum=12+256*256;
		fwrite(&bnum, 1, sizeof(bnum), handle); // number of bytes
		bnum=((tim_stuff[index].PageY)<<16)+tim_stuff[index].PageX;
		fwrite(&bnum, 1, sizeof(bnum), handle); // dy dx
		bnum=(256<<16)+64; //64 16bit = 256 4 bits
		fwrite(&bnum, 1, sizeof(bnum), handle); // h  w

		for(y=0;y<256;y+=1)
		for(x=0;x<128;x+=4)
		{
			SLONG	data;

			data=(dat[y][x+3]<<24)+(dat[y][x+2]<<16)+(dat[y][x+1]<<8)+(dat[y][x]<<0);
			fwrite(&data, 1, sizeof(data), handle); // h  w
		}

		MF_Fclose(handle);
#ifdef EDITOR
		copyfile_to_level(fname);
#endif

	}

}

void	save_tim_pal16(char *fname,UWORD index,SLONG	dy,SLONG height)
{
	FILE *handle;
	int	x,y;

	ASSERT(height<256);
	handle = MF_Fopen(fname, "wb");

	struct	
	{
		int	ID;
		int	Flag;
	}clut_head;

	if (handle)
	{
		int	bnum;

		clut_head.ID=0x10;
		clut_head.Flag=2; // 2=15 bit ; // 4 bit no cluts in tim.   //9; //0x10|1; // has clut & clut is 8bit
		fwrite(&clut_head, 1, sizeof(clut_head), handle);



		bnum=12+256*height;
		fwrite(&bnum, 1, sizeof(bnum), handle); // number of bytes
		bnum=((tim_stuff[index].PageY+dy)<<16)+tim_stuff[index].PageX;
		fwrite(&bnum, 1, sizeof(bnum), handle); // dy dx
		bnum=(height<<16)+64; //64 16bit = 256 4 bits
		fwrite(&bnum, 1, sizeof(bnum), handle); // h  w

		for(y=0;y<height;y+=1)
		for(x=0;x<128;x+=4)
		{
			int	data;

			data=(pals16[y+dy][(x+2)>>1]<<16)+(pals16[y+dy][x>>1]);//(dat[y][x+3]<<24)+(dat[y][x+2]<<16)+(dat[y][x+1]<<8)+(dat[y][x]<<0);
			fwrite(&data, 1, sizeof(data), handle); // h  w
		}

		MF_Fclose(handle);
#ifdef EDITOR
		copyfile_to_level(fname);
#endif




	}
	//
	// tom file is raw data
	//
	fname[strlen(fname)-2]='o';
	handle = MF_Fopen(fname, "wb");
	if(handle)
	{
		fwrite(&pals16[0][0], 2, 256*64, handle); 
		MF_Fclose(handle);
	}
#ifdef EDITOR
	copyfile_to_level(fname);
#endif
}


void	copy_to_psx_tim(UBYTE *data,SLONG w,SLONG h,UBYTE *pal,SLONG tim_x,SLONG tim_y)
{
	SLONG	px,py;
	SLONG	a,b;


	for(py=0;py<h;py++)
	for(px=0;px<w;px+=2)
	{
		a=data[px+py*w];
		b=data[px+1+py*w];
/*
		if(a>=0xf0)
			a-=0xf0;
		if(b>=0xf0)
			b-=0xf0;
*/
		a&=0xf;
		b&=0xf;

		psx_tim_page[tim_y+py][tim_x+(px>>1)]=a|(b<<4);//data[px+py*w]|(data[px+1+py*w]<<4);
	}
}

#ifdef	NO_SERVER
	#define TEXTURE_DIR "server\\gary16\\"
#else
	#define TEXTURE_DIR "u:\\urbanchaos\\gary16\\"
#endif

void	make_psx_pal(UWORD page_id,UBYTE *pal)
{
	SLONG	c0;
	UWORD	col;
	UWORD	*psx_pal;

	ASSERT(page_id<256<<2);
//	if(page_id>13*64)
//		return;

	psx_pal=&pals16[page_id>>2][(page_id&3)*16];

	for(c0=0;c0<16;c0++)
	{
		SLONG	r,g,b;

		r=*pal++;
		g=*pal++;
		b=*pal++;

		if(r==255&&g==0&&b==255)
			col=0;
		else
		{

			col = (r)>>3;
			col|=((g)>>3)<<5;
			col|=((b)>>3)<<10;
			col|=1<<15;
		}
		*psx_pal++=col;
	}

}

#ifdef EDITOR 

void	copyfile_to_level(CBYTE *str)
{
	SLONG	fname;
	CBYTE	str2[100];
	SLONG	len=0;
	SLONG	c0;

	for(c0=strlen(str);c0>=0;c0--)
	{
		if(str[c0]=='\\')
		{
			fname=c0+1;
			break;
			

		}
		len++;
	}

	c0=0;
	while(levels[c0].level)
	{
		if(strcmp(levels[c0].map_name,edit_info.MapName)==0)
		{
			ULONG	ret;
			sprintf(str2,"c:\\levels\\%d\\%s",levels[c0].level,&str[fname]);
			if(CopyFile(str,str2,0)==0)
			{
				ASSERT(0);

				ret=GetLastError();
				DebugText("ret %d \n",ret);

			}
		}
		c0++;
	}
}

void	build_floor_tims(void)
{
	SLONG	page,x,y;
	CBYTE	str[128];
	UBYTE	data[32][32];
	UBYTE	pal[256*3];
	for(page=4;page<8;page++)
	{
		for(y=0;y<256;y+=32)
		for(x=0;x<256;x+=32)
		{
			UWORD	page_id=page*64+(y>>5)*8+(x>>5),file_page_id;

			//
			// the remap bit for all the moved floor textures
			//

			file_page_id=moved_from[page_id];

//			ASSERT(file_page_id!=354);

			if(file_page_id>=4*64)
			{
				sprintf(str,"server\\textures\\shared\\psx\\tex%03dhi.tga",file_page_id);
				if(!FileExists(str))
				{
					sprintf(str,"server\\textures\\shared\\psx\\tex%03d.tga",file_page_id);
					if(!FileExists(str))
						sprintf(str,TEXTURE_DIR"shared\\tex%03dhi.tga",file_page_id);
				}
			}
			else
			{
//				ASSERT(file_page_id!=247);
				sprintf(str,"server\\textures\\world%d\\psx\\tex%03dhi.tga",editor_texture_set,file_page_id);
				if(!FileExists(str))
				{
					sprintf(str,"server\\textures\\world%d\\psx\\tex%03d.tga",editor_texture_set,file_page_id);
					if(!FileExists(str))
						sprintf(str,TEXTURE_DIR"world%d\\tex%03dhi.tga",editor_texture_set,file_page_id);
				}
			}
			
			if(!FileExists(str))
			{
				if(file_page_id>=4*64)
				{
					sprintf(str,TEXTURE_DIR"shared\\tex%03d.tga",file_page_id);
				}
				else
				{
					sprintf(str,TEXTURE_DIR"world%d\\tex%03d.tga",editor_texture_set,file_page_id);
				}
				if(!FileExists(str))
					continue;
			}

			TGA_load_psx(str,32,32,&data[0][0],pal);

			make_psx_pal(page_id,pal);

			copy_to_psx_tim(&data[0][0],32,32,pal,x>>1,y);
		}

		sprintf(str,TEXTURE_DIR"world%d\\tex%02d.tim",editor_texture_set,page+1);
		save_tim(str,psx_tim_page,page,1);
//		copyfile_to_level(str);
	}
}

#ifdef	NO_SERVER
	#define TEXTURE64_DIR "server\\gary64\\"
#else
	#define TEXTURE64_DIR "u:\\urbanchaos\\gary64\\"
#endif


void	build_wall_tims(void)
{
	SLONG	page,x,y;
	CBYTE	str[128];
	UBYTE	data[64][64];
	UBYTE	pal[256*3];
	for(page=0;page<4;page++)
	{
		for(y=0;y<256;y+=64)
		for(x=0;x<256;x+=64)
		{
			UWORD	page_id=page*16+(y>>6)*4+(x>>6);

//			ASSERT(page_id!=0);

//			ASSERT(page_id!=0);

			//
			// the remap bit for all the moved floor textures
			//

//			page_id=moved_from[page_id];

			sprintf(str,TEXTURE64_DIR"world%d\\tex%03dhi.tga",editor_texture_set,page_id);
			
			if(!FileExists(str))
			{
				sprintf(str,TEXTURE64_DIR"world%d\\tex%03d.tga",editor_texture_set,page_id);

				if(!FileExists(str))
					continue;
			}

			TGA_load_psx(str,64,64,&data[0][0],pal);

			make_psx_pal(page_id*4,pal);

			sprintf(str,TEXTURE64_DIR"world%d\\%03da.pal",editor_texture_set,page_id);
			load_alt_pal(str,pal);
			make_psx_pal(page_id*4+1,pal);

			sprintf(str,TEXTURE64_DIR"world%d\\%03db.pal",editor_texture_set,page_id);
			load_alt_pal(str,pal);
			make_psx_pal(page_id*4+2,pal);

			sprintf(str,TEXTURE64_DIR"world%d\\%03dc.pal",editor_texture_set,page_id);
			load_alt_pal(str,pal);
			make_psx_pal(page_id*4+3,pal);

			copy_to_psx_tim(&data[0][0],64,64,pal,x>>1,y);
		}

		sprintf(str,TEXTURE_DIR"world%d\\tex%02d.tim",editor_texture_set,page+1);
		save_tim(str,psx_tim_page,page,1);
//		copyfile_to_level(str);
	}
}

//
// Editor saves out tims for wall and floor, floor is a random quantity
//
void	build_tims(UWORD	next_texture)
{
	CBYTE	str[256];
	FILE *handle;

	memset((UBYTE*)pals16,0xff,256*128);

	sprintf(str,TEXTURE_DIR"world%d\\tex16.tim",editor_texture_set);
	build_floor_tims();
	build_wall_tims();

	save_tim_pal16(str,15,0,128);
	sprintf(str,TEXTURE_DIR"world%d\\pages.dat",editor_texture_set);

	handle = MF_Fopen(str, "wb");
	{
		fwrite((UBYTE*)&next_texture, 1,2, handle);
	}
	MF_Fclose(handle);
	copyfile_to_level(str);

}
#endif

UWORD	prims_remap[30*64];





SLONG	remap_a_prim_face(UWORD page,SLONG *next_page,SLONG level_no,SLONG start_page)
{
#ifdef	EDITOR

	SLONG	new_page;
	CBYTE	str[256];
	UBYTE	data[64][64];
	UBYTE	pal[256*3];
	TGA_Info	ans;
	if(page==0)
		return(0);
//	ASSERT(page<10*64);

	if(prims_remap[page]==0)
	{
		page+=FACE_PAGE_OFFSET;

		if (page < 64 * 9)
		{
//			ASSERT(0); 523 goes wrong
		}
		else
		if (page < 64 * 11)
		{
			//
			// people
			//
			sprintf(str,TEXTURE_DIR"shared\\people\\tex%03d.tga",page-9*64);
		}
		else
		if (page < 64 * 18)
		{
			//
			// prims
			//
			sprintf(str,TEXTURE_DIR"shared\\prims\\tex%03d.tga",page-11*64);
		}
		else
		if(page<21*64)
		{
			//
			// people2
			//

//			ASSERT(page-18*64!=44);
			sprintf(str,TEXTURE_DIR"shared\\people2\\tex%03d.tga",page-18*64);
		}
		else
		{
			ASSERT(0);
		}


		ans=TGA_load_psx(str,32,32,&data[0][0],pal);
		if(!ans.valid)
			DebugText(" %s failed \n",str);

		make_psx_pal((*next_page)+start_page*64,pal);

		DebugText(" move to %d,%d\n",((*next_page)&7)<<4,(((*next_page)>>3)&7)<<5);
		copy_to_psx_tim(&data[0][0],32,32,pal,(((*next_page)&7)<<4),(((*next_page)>>3)&7)<<5);

		prims_remap[page-FACE_PAGE_OFFSET]=(*next_page)+(start_page*64)+1;  //+FACE_PAGE_OFFSET+1;
		new_page=(*next_page)+(start_page*64);//+FACE_PAGE_OFFSET;

		(*next_page)++;
		if(((*next_page)&63)==0)
		{
			//
			// gone over a page boundary so save tim
			//

	//sprintf(str,TEXTURE_DIR"world%d\\tex16.tim",editor_texture_set);
//			sprintf(str,TEXTURE_DIR"world%d\\tex%02d.tim",TEXTURE_set,7+1+((*next_page)>>6));
			sprintf(str,"c:\\levels\\%d\\tex%02d.tim",level_no,start_page-1+1+((*next_page)>>6));
			save_tim(str,psx_tim_page,start_page-1+((*next_page)>>6),0);
			memset(psx_tim_page,0xff,128*256);

		}
	}
	else
	{
		new_page=prims_remap[page]-1;
	}

	return(new_page);
#else
	return(0);
#endif
}


SLONG	setup_psx_jackets(SLONG level_no,SLONG next_page,SLONG start_page)
{

	remap_a_prim_face(1*64+21,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+2,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+32,&next_page,level_no,start_page);

	remap_a_prim_face(1*64+22,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+3,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+33,&next_page,level_no,start_page);
				
	remap_a_prim_face(1*64+24,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+4,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+36,&next_page,level_no,start_page);

	remap_a_prim_face(1*64+25,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+5,&next_page,level_no,start_page);
	remap_a_prim_face(10*64+37,&next_page,level_no,start_page);
	return(next_page);

}


SLONG	get_level_no(CBYTE *name)
{
	SLONG	p0,p1,c0=0;

	roper_pickup=0;

	while(levels[c0].level)
	{
		p0=strlen(name)-5;  //last letter of name
		p1=strlen(levels[c0].name)-1;
		for(;p1>=0;p1--,p0--)
		{
			CBYTE	comp;
			if(tolower(name[p0])!=tolower(levels[c0].name[p1]))
				break;
		}
		if(p1<0)
		{
//			if(levels[c0].level==33)
//				roper_pickup=1;

			level_index = c0;

			return(levels[c0].level);
		}
		c0++;
	}
	return(0);
}



UWORD	psx_start_page;
SLONG	build_tims_ingame(CBYTE *name)
{
	SLONG	c0;
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	SLONG	next_page=0,new_page;
	CBYTE	str[255];
	SLONG	level_no;
	UWORD	next_texture;

	FILE *handle;

	

	memset((UBYTE*)prims_remap,0,30*64*2);

	level_no=get_level_no(name);
	DebugText("PSX create tims for level%d\n",level_no);


//	sprintf(str,TEXTURE_DIR"world%d\\pages.dat",TEXTURE_SET);
	sprintf(str,"c:\\levels\\%d\\pages.dat",level_no);

	handle = MF_Fopen(str, "rb");
	if(handle)
	{
		//
		// find number f textures used by map
		//
		fread((UBYTE*)&next_texture, 1,2, handle); 
		MF_Fclose(handle);
		psx_start_page=next_texture;
	}
	else
		ASSERT(0);


	//
	// load in palette made so far
	//
//	sprintf(str,TEXTURE_DIR"world%d\\tex16.tom",TEXTURE_SET);
	sprintf(str,"c:\\levels\\%d\\tex16.tom",level_no);
	handle = MF_Fopen(str, "rb");
	if(handle)
	{
		fread(&pals16[0][0], 2, 256*64, handle);
		MF_Fclose(handle);
		memset((UBYTE*)&pals16[128][0],0xff,128*128);
	}
	else
		ASSERT(0);

//	sprintf(str,TEXTURE_DIR"world%d\\tex%02d.tom",TEXTURE_SET,1+(next_texture>>6));
	sprintf(str,"c:\\levels\\%d\\tex%02d.tom",level_no,1+(next_texture>>6));
	handle = MF_Fopen(str, "rb");
	if(handle)
	{
//		fwrite(&dat[0][0], 1,256*128, handle); 
		fread(&psx_tim_page[0][0], 1, 256*128, handle);
		MF_Fclose(handle);
	}
	else
		ASSERT(0);



	next_page=next_texture&63;

	next_page=setup_psx_jackets(level_no,next_page,next_texture>>6);



//	return;

//	memset((UBYTE*)pals16,0xff,256*128);

	for(c0=1;c0<next_prim_face3;c0++)
	{
		SLONG	page;
		p_f3 = &prim_faces3[c0];

		page   = p_f3->UV[0][0] & 0xc0;
		page <<= 2;
		page  |= p_f3->TexturePage;

		switch(page)
		{
			case	1*64+21:
			case	10*64+2:
			case	10*64+32:
				p_f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+21;

				break;

			case	1*64+22:
			case	10*64+3:
			case	10*64+33:
				p_f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+22;
				break;

			case	1*64+24:
			case	10*64+4:
			case	10*64+36:
				p_f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+24;
				break;


			case	1*64+25:
			case	10*64+5:
			case	10*64+37:
				p_f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+25;
				break;

		}
  
		new_page =	remap_a_prim_face(page,&next_page,level_no,(next_texture>>6));
		new_page-= (next_texture>>6)*64;

		p_f3->UV[0][0]   &= ~0xc0;
		p_f3->UV[0][0]   |= (new_page >> 2) & 0xc0;
		p_f3->TexturePage = (new_page >> 0) & 0xff;


	}

	for(c0=1;c0<next_prim_face4;c0++)
	{
		SLONG	page;
		p_f4 = &prim_faces4[c0];

		page   = p_f4->UV[0][0] & 0xc0;
		page <<= 2;
		page  |= p_f4->TexturePage;

		switch(page)
		{
			case	1*64+21:
			case	10*64+2:
			case	10*64+32:
				p_f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+21;

				break;

			case	1*64+22:
			case	10*64+3:
			case	10*64+33:
				p_f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+22;
				break;

			case	1*64+24:
			case	10*64+4:
			case	10*64+36:
				p_f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+24;
				break;


			case	1*64+25:
			case	10*64+5:
			case	10*64+37:
				p_f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
				page=1*64+25;
				break;

		}

		new_page =	remap_a_prim_face(page,&next_page,level_no,(next_texture>>6));
		new_page-= (next_texture>>6)*64;

		p_f4->UV[0][0]   &= ~0xc0;
		p_f4->UV[0][0]   |= (new_page >> 2) & 0xc0;
		p_f4->TexturePage = (new_page >> 0) & 0xff;


	}


//
// save last page (not full)
//


//	sprintf(str,TEXTURE_DIR"world%d\\tex%02d.tim",TEXTURE_set,8+1+((next_page)>>6));
	sprintf(str,"c:\\levels\\%d\\tex%02d.tim",level_no,(next_texture>>6)+1+((next_page)>>6));
	save_tim(str,psx_tim_page,(next_texture>>6)+((next_page)>>6),0);

//	sprintf(str,TEXTURE_DIR"world%d\\tex18.tim",TEXTURE_set);
	sprintf(str,"c:\\levels\\%d\\tex18.tim",level_no);  //MD16
	save_tim_pal16(str,15,0,255);//((next_texture&(~63))+next_page+3)>>2);


#ifdef TARGET_DC
	// For some unknown reason this won't compile on the DC. Shouldn't be used
	// on DC anyway, so...
	ASSERT(FALSE);
#else
	{
		CBYTE	str2[100];
		sprintf(str,"n:\\urbanchaos\\textures\\world%d\\sky.tga",TEXTURE_SET);
		sprintf(str2,"c:\\levels\\%d\\sky.tga",level_no);  //MD16
		if(CopyFile(str,str2,0)==0)
		{
			ASSERT(0);
		}
	}
#endif

	return(level_no);

}




void	move_texture(UWORD from,UWORD to)
{
#ifdef	EDITOR
	SLONG	x,y;
	SLONG	fpage,tpage,fx,fy,tx,ty;
	UWORD	*ptr_from,*ptr_to;

	fpage=(from>>6)&15;
	tpage=(to>>6)&31;

	fx=(from&7)<<5;
	fy=((from>>3)&7)<<5;

	tx=(to&7)<<5;
	ty=((to>>3)&7)<<5;

//	if(tpage<=30)
	{

		ptr_from=tmaps[fpage];
		ptr_to=tmaps[tpage];

		for(y=0;y<32;y++)
		for(x=0;x<32;x++)
		{
//			ptr_to[x+tx+(ty+y)*256]=ptr_from[fx+x+(fy+y)*256];
		}
	}
#endif

}
extern	UWORD	page_count[];

UWORD	get_split_bits(UWORD tex)
{
	switch(tex&0x3ff)
	{
		case	454:
//		case	456:
//		case	457:
			return(1<<14);
			break;


/*
		case	349:
		case	353:
		case	342:
		case	343:
		case	333:
		case	334:
		case	340:
			switch((tex&0xc00)>>10)
			{
				case	0:
					return(1<<15);
				case	1:
					return(1<<14);
				case	2:
					return(1<<15);
				case	3:
					return(1<<14);

			}
			break;
*/


	}
	return(0);

}

#ifdef EDITOR 
SLONG	remap_floor_psx(void)
{
	SLONG	x,z;
	SLONG	current=4*64;

	for(x=0;x<MAP_WIDTH;x++)
	for(z=0;z<MAP_HEIGHT;z++)
	{
		if(x<PAP_SIZE_HI && z<PAP_SIZE_HI)
		{
			UWORD	texture;
			UWORD	texture_all;
			UWORD	split_bits;

			edit_map[x][z].Texture&=~0xc000;
			texture_all=edit_map[x][z].Texture;

			texture=edit_map[x][z].Texture&0x3ff;

			if(page_count[texture])
			{
				//
				//it is used
				//

				edit_map[x][z].Texture&=~0x3ff;

				if(moved_to[texture])
				{
					// previously moved

					edit_map[x][z].Texture|=moved_to[texture]-1;
					split_bits=get_split_bits(texture_all);

					edit_map[x][z].Texture|=split_bits;

					


				}
				else
				{
					edit_map[x][z].Texture|=current;

					split_bits=get_split_bits(texture_all);
					edit_map[x][z].Texture|=split_bits;

					//
					// this location contains a texture from ...
					//
					moved_from[current]=texture+1;
					moved_to[texture]=current+1;

					//
					// build the texture pages
					//
					move_texture(texture,current+16*64);
					current++;
				}
			}
		}
	}
	return(current);
}

SLONG	remap_walls()
{
	return(0);

}
void	remap_textures_psx(void)
{
	
	//
	// 
	//
	memset(moved_from,0,64*8*2);
	memset(moved_to,0,64*8*2);

	remap_floor_psx();
	remap_walls();
}
#endif

BOOL	game_create_psx(CBYTE *mission_name);
BOOL	make_texture_clumps(CBYTE *mission_name);
extern	void		TesterText(CBYTE *error, ...);
extern	CBYTE	ELEV_fname_level   [];

//SLONG    MAV_opt_upto;


#ifndef TARGET_DC


SWORD	people_types[50];
ULONG	DONT_load=0; // nice global used for people loading
void	save_all_nads(void)
{
#ifdef	EDITOR
	SLONG	p0,p1,c0=0;
	SLONG	highest=0;

#if 0
	{
		UBYTE	data[32][32];
		UBYTE	pal[256*3];
		TGA_load_psx("c:\\fallen\\server\\textures\\world8\\psx\\tex215hi.tga",32,32,&data[0][0],pal);

		make_psx_pal(0,pal);

		copy_to_psx_tim(&data[0][0],32,32,pal,0,0);
	}
#endif

	while(levels[c0].level)
	{
		CBYTE	name[100];

		sprintf(name,"%s\\%s.ucm",ENV_get_value_string("level_dir","Secret"),levels[c0].name);
//#ifdef	MIKE
//		TesterText("Level: %s \n",levels[c0].name);
//		TRACE("aLevel: %s \n",levels[c0].name);
//#endif


		level_index=c0;

		if(levels[c0].level==33 || levels[c0].level==21)
			roper_pickup=1;
		else
			roper_pickup=0;

		memset((UBYTE*)people_types,0,100);
		DONT_load=levels[c0].dontload;

		strcpy(ELEV_fname_level,name);
		TRACE("bLevel: %s \n",levels[c0].name);
		game_create_psx(name);
		TRACE("cLevel: %s \n",levels[c0].name);
void	count_people_types(void);
		count_people_types();



		{
			SLONG	c1;
			DebugText("PEEPS ");

			for(c1=0;c1<16;c1++)
				DebugText("[%d]=%d ",c1,people_types[c1]);

			DebugText("\n");
		}
		DebugText(" level %s mav_opt %d\n",name,MAV_opt_upto);
		if(MAV_opt_upto>highest)
		{
			highest=MAV_opt_upto;
			DebugText("HIGHEST %d level %s mav_opt %d\n",highest,name,MAV_opt_upto);
		}
		c0++;
//		if(DONT_load)
//			break;
	}
#endif
}

#endif //#ifndef TARGET_DC


extern int	TEXTURE_create_clump;

void	make_all_clumps(void)
{
#ifdef TARGET_DC
	// One day, I need to write a clumper.
#else

	SLONG	p0,p1,c0=0;
	SLONG	highest=0;

	TEXTURE_create_clump = 1;

	TRACE("Doing front-end\n");
	TEXTURE_load_needed("levels\\frontend.ucm", 0, 256, 40);

	Levels*	lptr = levels;//_demo;

	while(lptr[c0].level)
	{
		TRACE("Doing level %s\n", lptr[c0].name);

		CBYTE	name[100];
		sprintf(name,"levels\\%s.ucm",lptr[c0].name);
		make_texture_clumps(name);
		DebugText(" level %s mav_opt %d\n",name,MAV_opt_upto);
		if(MAV_opt_upto>highest)
		{
			highest=MAV_opt_upto;
			DebugText("HIGHEST %d level %s mav_opt %d\n",highest,name,MAV_opt_upto);
		}
		c0++;
	}

	exit(0);
#endif
}

#endif





