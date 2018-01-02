
#include	"Editor.hpp"

#include	"engine.h"
#include	"coltab.hpp"
//#include	"collide.hpp"
#include	"map.h"
#include    "game.h"
#include	"light.h"
#include	"extra.h"
#include	"animtmap.h"
#include	"pap.h"
#include	"ob.h"
#include	"supermap.h"
#include	"io.h"
#include	"memory.h"

#define	SET_TEXTURE_ROCKS(t)	{t.X=4;t.Y=4;t.Width=2;t.Height=2;t.Page=0;t.DrawFlags=(POLY_FLAG_GOURAD|POLY_FLAG_TEXTURED);}

extern	void load_palette(CBYTE *palette);
extern	SLONG	calc_edit_height_at(SLONG x,SLONG z);
extern	SLONG	next_inside; //building.cpp

extern	SLONG	editor_texture_set;
extern	UWORD	page_count[];
extern	UWORD	moved_from[16*64];
extern	UWORD	moved_to[16*64];

extern	UWORD	*psx_remap;//[128];
extern	UWORD	page_remap[];

struct	MapBlock2
{
	SLONG	X;
	SLONG	Y;
	SLONG	Z;
	struct	DepthStrip	*MapPtr;
	SLONG	AngleX; //wont need this I suspect
	SLONG	AngleY;
	SLONG	AngleZ; //wont need this I suspect

	SLONG	MapWidth;
	SLONG	MapHeight;
	SLONG	MapDepth; // (unchangeable?)

	CBYTE	*name;
};

// UBYTE	texture_sizes[]={8,16,32,64,96,128,160,192};

SWORD	face_selected_list[MAX_EDIT_FACE_LIST];
SWORD	next_face_selected=1;

extern SLONG	save_psx;


struct	Light	d_lights[MAX_D_LIGHTS];
UWORD	next_d_light=1;

//extern	ULONG		WorkWindowHeight,   //just for now as application does not get these
//					WorkWindowWidth; 
extern	void	create_bucket_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col);
extern	void	create_bucket_3d_line_whole(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col);


//struct	EditMapElement	edit_map_eles[65000];



SLONG	edit_turn;
/*
CBYTE	prim_names[200][13]=
{
	"testzoo.asc",
	"testzoo.asc",
};
*/

// EditFace	edit_face;
EditFace		hilited_face,
				selected_face;
struct SVector	selected_prim_xyz;


struct	EditInfo	edit_info;	



void	swap_maps()
{
	CBYTE	name[128];

	SLONG	create=1;

	if(ShiftFlag)
		create=0;


	sprintf(name,"data\\swap%d.map",edit_info.MapID);
	save_map(name,1);
	edit_info.MapID++;
	edit_info.MapID&=1;
	sprintf(name,"data\\swap%d.map",edit_info.MapID);
	if(!load_map(name))
	{
		clear_map();
	}
	else
	{
		if(create)		
			create_city(BUILD_MODE_EDITOR);
	}
}




//		if(dx+mx>0&&dx+mx<EDIT_MAP_WIDTH&&dz+mz>0&&dz+mz<EDIT_MAP_DEPTH)
SLONG	on_edit_map(SLONG x,SLONG z)
{
	if(edit_info.Clipped)
	{
		if((x<<8)>edit_info.MinX && (x<<8)<edit_info.MaxX && (z<<8)>edit_info.MinZ && (z<<8)<edit_info.MaxZ)
		{
			return(1);
		}
		else
		{
			return(0);
		}

	}
	else
	{
		if(x>=0&&x<EDIT_MAP_WIDTH&&z>=0&&z<EDIT_MAP_DEPTH)
		{
			return(1);
		}
		else
		{
			return(0);
		}
	}
}

void	free_edit_memory(void)
{
	SLONG	c0;
	for(c0=0;c0<MAX_WALLS;c0++)
	{
		if(wall_list[c0].Textures&&wall_list[c0].Tcount)
		{
			MemFree((void*)wall_list[c0].Textures);
			wall_list[c0].Textures=0;
			wall_list[c0].Tcount=0;
		}
	}
}



SLONG	face_is_in_list(SWORD face)
{
	SLONG	c0;
	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]==face)
			return(c0);
	}
	return(0);
	
}

void	add_face_to_list(SWORD face)
{
	SLONG	c0;


	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]==face)
		{
//			LogText(" face %d removed from pos %d nfs %d \n",face,c0,next_face_selected-1);
			for(;c0<next_face_selected;c0++)
			{
				face_selected_list[c0]=face_selected_list[c0+1];
			}
			next_face_selected--;
			if(face<0)
				prim_faces3[-face].FaceFlags&=~FACE_FLAG_OUTLINE;
			else
				prim_faces4[face].FaceFlags&=~FACE_FLAG_OUTLINE;
			return;
		}
	}
	if(next_face_selected>=MAX_EDIT_FACE_LIST)
		return;

//	LogText(" face %d added to pos %d  \n",face,next_face_selected);

	face_selected_list[next_face_selected]=face;						
	if(face<0)
		prim_faces3[-face].FaceFlags|=FACE_FLAG_OUTLINE;
	else
		prim_faces4[face].FaceFlags|=FACE_FLAG_OUTLINE;
	next_face_selected++;
	edit_info.TileFlag=0;
}



SLONG	add_a_light(SWORD i,SLONG x,SLONG y,SLONG z)
{
	if(next_d_light<MAX_D_LIGHTS)
	{
		d_lights[next_d_light].Intensity=i;
		d_lights[next_d_light].X=x;
		d_lights[next_d_light].Y=y;
		d_lights[next_d_light++].Z=z;
		return(1);
	}
	return(0);

}

void	clear_lights(void)
{
	next_d_light=1;
}


//insert_cube((engine.X>>8)>>ELE_SHIFT,(engine.Y>>8)>>ELE_SHIFT,(engine.Z>>8)>>ELE_SHIFT);
extern	UWORD	apply_ambient_light_to_object(UWORD object,SLONG lnx,SLONG lny,SLONG lnz,UWORD intense);

SLONG	place_prim_at(UWORD prim,SLONG x,SLONG y,SLONG z)
{
	UWORD	map_thing;
	struct	MapThing	*p_mthing;

	y=0;

	map_thing=find_empty_map_thing();
	if(!map_thing)
		return(0);
	add_thing_to_edit_map(x>>ELE_SHIFT,z>>ELE_SHIFT,map_thing);
	p_mthing=TO_MTHING(map_thing);
	p_mthing->X=x;
	p_mthing->Y=y;
	p_mthing->Z=z;

	p_mthing->Type=MAP_THING_TYPE_PRIM;
//	p_mthing->IndexOther=copy_prim_to_end(prim,0,map_thing);
	p_mthing->IndexOther=prim; //copy_prim_to_start(prim,0,map_thing);
	p_mthing->IndexOrig=prim;
	if(edit_info.Inside)
		p_mthing->Flags|=FLAG_EDIT_PRIM_INSIDE;
	else
		p_mthing->Flags&=~FLAG_EDIT_PRIM_INSIDE;
//	apply_ambient_light_to_object(prim,edit_info.amb_dx,edit_info.amb_dy,edit_info.amb_dz,edit_info.amb_bright);

	return(map_thing);
}

SLONG	place_anim_prim_at(UWORD prim,SLONG x,SLONG y,SLONG z)
{
	UWORD	map_thing;
	struct	MapThing	*p_mthing;

	y=0;

	map_thing=find_empty_map_thing();
	if(!map_thing)
		return(0);
	add_thing_to_edit_map(x>>ELE_SHIFT,z>>ELE_SHIFT,map_thing);
	p_mthing=TO_MTHING(map_thing);
	p_mthing->X=x;
	p_mthing->Y=y;
	p_mthing->Z=z;

	p_mthing->Type=MAP_THING_TYPE_ANIM_PRIM;
//	p_mthing->IndexOther=copy_prim_to_end(prim,0,map_thing);
	p_mthing->IndexOther=prim; //copy_prim_to_start(prim,0,map_thing);
	p_mthing->IndexOrig=prim;
//	apply_ambient_light_to_object(prim,edit_info.amb_dx,edit_info.amb_dy,edit_info.amb_dz,edit_info.amb_bright);

	return(map_thing);
}

SLONG	is_thing_on_map(SLONG index)
{					
	struct MapThing *p_thing;
	SLONG	map;

	p_thing=TO_MTHING(index);
	map=edit_map[p_thing->X>>ELE_SHIFT][p_thing->Z>>ELE_SHIFT].MapThingIndex;
	while(map)
	{
		if(map==index)
			return(1);
		map=map_things[map].MapChild;
	}
	return(0);
}

void	build_tims(UWORD	next_texture);

UWORD	is_road[]=
{
	323,324,325,326,327,328,331,332,333,334,340,341,342,343,348,349,350,351,352,353,354,355,356,0
};

extern	void	move_texture(UWORD from,UWORD to);
extern	UWORD	get_split_bits(UWORD tex);


void	save_game_map(CBYTE *name)
{
	UWORD	temp1,temp2,temp3,temp;
	SLONG	save_type=26, ob_size;
	SLONG	x,y,z;
	SLONG	c0;
	MapElement	me;
	Thing	th;
	struct	MapThing	*t_mthing;
	SLONG	next_texture=64*4;
	UWORD	*tex_map_psx;

	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	MFFileHandle	jandle	=	FILE_OPEN_ERROR;


	
	//
	// Change the extension of 'name'...
	//

	CBYTE  gamename[256];
	CBYTE *ch;

	strcpy(gamename, name);

	for (ch = gamename; *ch; ch++);
	while(*ch != '.') {ch--;}
	ch++;

	if(save_psx)
		*ch++ = 'p';
	else
		*ch++ = 'i';
	*ch++ = 'a';
	*ch++ = 'm';
	*ch++ = '\000';

	memset((CBYTE*)moved_from,0,16*64*2);
	memset((CBYTE*)moved_to,0,16*64*2);



	jandle=FileCreate(gamename,1);
	if(jandle!=FILE_CREATION_ERROR) {
		// I'm evil. eeeeeeevilllle.
extern void	save_ob_ob(MFFileHandle	handle);
		save_ob_ob(jandle);
		FileClose(jandle);
	}

	handle=FileCreate(gamename,1);
	if(handle!=FILE_CREATION_ERROR)
	{
		PAP_Hi pap_hi;
		PAP_Lo pap_lo;
		SLONG	c0;

		FileWrite(handle,(UBYTE*)&save_type,4);

extern	void	add_flat_roof_to_pap(void);
		add_flat_roof_to_pap();

		ob_size = sizeof(OB_ob_upto) + (sizeof(OB_Ob)*OB_ob_upto) + (sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
		FileWrite(handle,(UBYTE*)&ob_size,4);

		memset(&me,0,sizeof(me));

		c0=0;
		while(is_road[c0])
		{
//			ASSERT(is_road[c0]!=340);
			moved_to[is_road[c0]]=next_texture;
			moved_from[next_texture]=is_road[c0];

			move_texture(is_road[c0],next_texture+25*64);

			next_texture++;
			c0++;
		}
		tex_map_psx=(UWORD*)MemAlloc(128*128*2);

		for(x=0;x<MAP_WIDTH;x++)
		for(z=0;z<MAP_HEIGHT;z++)
		{
			if(x<PAP_SIZE_HI && z<PAP_SIZE_HI)
			{
				if(save_psx)
				{
					UWORD	texture;
					UWORD	split;

					split=get_split_bits(edit_map[x][z].Texture);
					texture=edit_map[x][z].Texture&0x3ff;
					edit_map[x][z].Texture&=~0xc000;


					//if(page_count[texture]) //no effect surely
					{


						if(moved_to[texture])
						{
							// previously moved

							pap_hi.Texture		   =(edit_map[x][z].Texture&(~0x3ff))|(moved_to[texture])|split;


						}
						else
						{
							pap_hi.Texture		   =(edit_map[x][z].Texture&(~0x3ff))|(next_texture)|split;

							//
							// this location contains a texture from ...
							//
							moved_from[next_texture]=texture;
							moved_to[texture]=next_texture;

							move_texture(texture,next_texture+25*64);

							next_texture++;
						}
					}
					//
					// now the alternative texture layer (for roof tops)
					//
					split=get_split_bits(tex_map[x][z]);
					texture=tex_map[x][z]&0x3ff;
					tex_map[x][z]&=~0xc000;

					if(moved_to[texture])
					{
						// previously moved

						tex_map_psx[x*128+z]		   =(tex_map[x][z]&(~0x3ff))|(moved_to[texture])|split;


					}
					else
					{
						tex_map_psx[x*128+z]		   =(tex_map[x][z]&(~0x3ff))|(next_texture)|split;

						//
						// this location contains a texture from ...
						//
						moved_from[next_texture]=texture;
						moved_to[texture]=next_texture;

						move_texture(texture,next_texture+25*64);

						next_texture++;
					}



				}
				else
				{

					pap_hi.Texture			 = edit_map[x][z].Texture;
				}
				pap_hi.Flags			 = edit_map[x][z].Flags;
				pap_hi.Alt				 = edit_map[x][z].Y;
				pap_hi.Height			  =PAP_hi[x][z].Height;
				if(edit_map[x][z].Walkable==-1)
					pap_hi.Flags   |= FLOOR_ANIM_TMAP;

				if(edit_map[x][z].Flags&FLOOR_IS_ROOF)
				{
					pap_hi.Flags   |= PAP_FLAG_ROOF_EXISTS;
				}
			}
			else
			{
				memset(&pap_hi,0,sizeof(pap_hi));
			}
			FileWrite(handle,(UBYTE*)&pap_hi,sizeof(pap_hi));

		}
		if(save_psx)
		{

			ULONG	check;

			check=128*128*2;

			FileWrite(handle,(UBYTE*)&check,4);
			FileWrite(handle,(UBYTE*)tex_map_psx,128*128*2);
			check=666;
			FileWrite(handle,(UBYTE*)&check,4);

		}
		MemFree((void*)tex_map_psx);

		temp=0;
		for(c0=0;c0<MAX_MAP_THINGS;c0++)
		{
			t_mthing=&map_things[c0];

			switch(t_mthing->Type)
			{

				case	MAP_THING_TYPE_ANIM_PRIM:		
					temp++;
					break;

			}
		}

		FileWrite(handle,(UBYTE*)&temp,sizeof(temp));
		for(c0=0;c0<MAX_MAP_THINGS;c0++)
		{
			struct	LoadGameThing	io_thing;
			t_mthing=&map_things[c0];

			switch(t_mthing->Type)
			{
				case	MAP_THING_TYPE_ANIM_PRIM:		
					io_thing.Type=t_mthing->Type;
					io_thing.X=t_mthing->X;
					io_thing.Y=t_mthing->Y;
					io_thing.Z=t_mthing->Z;
					io_thing.AngleY=t_mthing->AngleY;
					io_thing.IndexOther=t_mthing->IndexOther;
					FileWrite(handle,(UBYTE*)&io_thing,sizeof(struct LoadGameThing));
					break;
			}
		}

		//
		// save all the ob'sa we just made
		//
/* //see supermap save_ob_ob
		FileWrite(handle,(UBYTE*)&OB_ob_upto,sizeof(OB_ob_upto));
		FileWrite(handle,(UBYTE*)&OB_ob[0],sizeof(OB_Ob)*OB_ob_upto);
		FileWrite(handle,(UBYTE*)&OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
*/



		extern	void	save_super_map(MFFileHandle	handle);
		save_super_map(handle);
		FileWrite(handle,(UBYTE*)&editor_texture_set,sizeof(editor_texture_set));


		{
			SLONG	si,pi;
			for(si=0;si<200;si++)
			{
				for(pi=0;pi<5;pi++)
				{
					SLONG	flip;
					SLONG	page;

					page=textures_xy[si][pi].Page*64+(textures_xy[si][pi].Tx)+(textures_xy[si][pi].Ty)*8;
					if(page>=8*64)
						page=1;
					page=page_remap[page]-1;
//					ASSERT(((page)>>6)<4);
					psx_textures_xy[si][pi]=(page&0xff);
//					ASSERT(psx_textures_xy[si][pi]!=0x1b);

					flip=textures_xy[si][pi].Flip;
					flip^=(page>>14)&3;

					psx_textures_xy[si][pi]|=flip<<14;
				}
			}
			FileWrite(handle,(UBYTE*)psx_textures_xy,2*200*5);
		}
		FileClose(handle);
	}

	{
		//
		// Save out a special extras file.
		//

		FILE *handle = fopen("data\\game.ext", "wb");

		if (handle)
		{
			fwrite(EXTRA_thing,1,sizeof(EXTRA_thing),handle);
			fclose(handle);
		}
	}


	if(save_psx)
	{
void	save_texture_styles_psx(UBYTE world);
		save_texture_styles_psx(editor_texture_set);
		build_tims(next_texture);
	}
	else
	{
//extern	void	save_texture_styles(UBYTE world);
//		save_texture_styles(editor_texture_set);
	}



	//
	// Create the game.map aswell...
	//

//	CopyFile(gamename, "data\\game.map", FALSE);
}


void	save_tex_remap(CBYTE *name)
{
	CBYTE	name2[128];
	SLONG	c0;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	UWORD	type=1,count=64*8;

	strcpy(name2,name);

	for(c0=0;c0<128;c0++)
	{
		if(name2[c0]=='.')
		{
			name2[c0+1]='r';
			break;
		}
	}

	handle=FileCreate(name2,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)&type,sizeof(type));
		FileWrite(handle,(UBYTE*)&count,sizeof(count));

		FileWrite(handle,(UBYTE*)&page_remap,sizeof(UWORD)*count);
	}
	FileClose(handle);
	
}

void	load_tex_remap(CBYTE *name)
{
	CBYTE	name2[128];
	SLONG	c0;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	UWORD	type=0,count=64*8;

	strcpy(name2,name);

	for(c0=0;c0<128;c0++)
	{
		if(name2[c0]=='.')
		{
			name2[c0+1]='r';
			name2[c0+2]='a';
			name2[c0+3]='p';
			break;
		}
	}

	handle=FileOpen(name2);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&type,sizeof(type));
		FileRead(handle,(UBYTE*)&count,sizeof(count));

		FileRead(handle,(UBYTE*)&page_remap,sizeof(UWORD)*count);
		if(type==0)
		{
			SLONG	c0;
			UWORD	page;
			for(c0=0;c0<count;c0++)
			{
				page=page_remap[c0];
				page&=~(3<<14);
				if(page_remap[c0]&(1<<14));
					page|=1<<15;
				if(page_remap[c0]&(1<<15));
					page|=1<<14;
			}
		}
	}
	else
		memset(page_remap,0,64*8*2);

	for(c0=0;c0<128;c0++)
		psx_remap[c0]=page_remap[c0];

	FileClose(handle);
	
}

void	save_map(CBYTE	*name,SLONG quick)
{
	UWORD	temp;
	SLONG	save_type=27;
	SLONG	c0;

	MFFileHandle	handle	=	FILE_OPEN_ERROR;


	if(quick==0)
	{
		SLONG	c0;
		SLONG	hide_roof=0;

		CBYTE	bakname0[128];
		CBYTE	bakname1[128];
		CBYTE	bakname2[128];

		strcpy(bakname0,name);
		strcpy(bakname1,name);
		strcpy(bakname2,name);
		for(c0=0;c0<strlen(bakname0);c0++)
		{
			if(bakname0[c0]=='.')
				break;
		}
		c0+=3;
		bakname2[c0]='2';
		bakname1[c0]='1';
		bakname0[c0]='0';

		CopyFile(bakname1,bakname2,0);
		CopyFile(bakname0,bakname1,0);
		CopyFile(name,bakname0,0);

		hide_roof=edit_info.HideMap&4;
		edit_info.HideMap&=~4;
		create_city(BUILD_MODE_EDITOR);
		edit_info.HideMap|=hide_roof;

	}

	for(c0=0;c0<MAX_PRIM_FACES4;c0++)
		prim_faces4[c0].FaceFlags&=~FACE_FLAG_OUTLINE;

	handle=FileCreate(name,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)&save_type,4);
		FileWrite(handle,(UBYTE*)edit_map,sizeof(struct DepthStrip)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);

		FileWrite(handle,(UBYTE*)tex_map,sizeof(UWORD)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);

		FileWrite(handle,(UBYTE*)edit_map_roof_height,sizeof(SBYTE)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);
		
		FileWrite(handle,(UBYTE*)&end_prim_point,sizeof(UWORD));
		FileWrite(handle,(UBYTE*)&end_prim_face4,sizeof(UWORD));
		FileWrite(handle,(UBYTE*)&end_prim_face3,sizeof(UWORD));
		FileWrite(handle,(UBYTE*)&end_prim_object,sizeof(UWORD));

		temp=MAX_PRIM_POINTS-end_prim_point;
		FileWrite(handle,(UBYTE*)&temp,sizeof(UWORD));

		temp=MAX_PRIM_FACES4-end_prim_face4;
		FileWrite(handle,(UBYTE*)&temp,sizeof(UWORD));

		temp=MAX_PRIM_FACES3-end_prim_face3;
		FileWrite(handle,(UBYTE*)&temp,sizeof(UWORD));

		temp=MAX_PRIM_OBJECTS-end_prim_object;
		FileWrite(handle,(UBYTE*)&temp,sizeof(UWORD));

		FileWrite(handle,(UBYTE*)&prim_points[end_prim_point] ,sizeof(struct PrimPoint) *(MAX_PRIM_POINTS - end_prim_point));
		FileWrite(handle,(UBYTE*)&prim_faces4[end_prim_face4] ,sizeof(struct PrimFace4) *(MAX_PRIM_FACES4 - end_prim_face4 ));
		FileWrite(handle,(UBYTE*)&prim_faces3[end_prim_face3] ,sizeof(struct PrimFace3) *(MAX_PRIM_FACES3 - end_prim_face3 ));
		FileWrite(handle,(UBYTE*)&prim_objects[end_prim_object],sizeof(struct PrimObject)*(MAX_PRIM_OBJECTS- end_prim_object));
		FileWrite(handle,(UBYTE*)&background_prim,sizeof(UWORD));



// Moved to the end for versions 26+
//		FileWrite(handle,(UBYTE*)&map_things[0],sizeof(struct MapThing)*MAX_MAP_THINGS);
		FileWrite(handle,(UBYTE*)&edit_info.amb_dx,4*5);
		FileWrite(handle,(UBYTE*)&next_col_info,2);
		FileWrite(handle,(UBYTE*)col_info,sizeof(struct ColInfo)*next_col_info);


		temp=MAX_WINDOWS;
		FileWrite(handle,(UBYTE*)&temp,2);
		temp=MAX_WALLS;
		FileWrite(handle,(UBYTE*)&temp,2);
		temp=MAX_STOREYS;
		FileWrite(handle,(UBYTE*)&temp,2);
		temp=MAX_BUILDINGS;
		FileWrite(handle,(UBYTE*)&temp,2);

		FileWrite(handle,(UBYTE*)window_list,sizeof(struct FWindow)*MAX_WINDOWS);
		for(c0=0;c0<MAX_WALLS;c0++)
		{
			FileWrite(handle,(UBYTE*)&wall_list[c0],sizeof(struct FWall)*1);
			if(wall_list[c0].Tcount&&wall_list[c0].Textures)
			{
				FileWrite(handle,(UBYTE*)wall_list[c0].Textures,wall_list[c0].Tcount);
			}
			if(wall_list[c0].Tcount2&&wall_list[c0].Textures2)
			{
				FileWrite(handle,(UBYTE*)wall_list[c0].Textures2,wall_list[c0].Tcount2);
			}
		}

		FileWrite(handle,(UBYTE*)&storey_list[0],sizeof(struct FStorey)*MAX_STOREYS);
		FileWrite(handle,(UBYTE*)building_list,sizeof(struct FBuilding)*MAX_BUILDINGS);


		temp=next_inside;
		FileWrite(handle,(UBYTE*)&temp,sizeof(temp));
		FileWrite(handle,(UBYTE*)&room_ids[0],sizeof(struct RoomID)*temp);



		FileWrite(handle,(UBYTE*)EXTRA_thing,sizeof(EXTRA_thing));

		FileWrite(handle,(UBYTE*)&editor_texture_set,sizeof(editor_texture_set));

		FileWrite(handle,(UBYTE*)&map_things[0],sizeof(struct MapThing)*MAX_MAP_THINGS);

		FileClose(handle);
	}
	save_tex_remap(name);

	if(!quick)
	{
//		export_tex(name);

		save_game_map(name);
	}
}

void	set_things_faces(SWORD thing)
{
	SLONG	c0;
	struct	PrimObject	*p_obj;

	p_obj    =	&prim_objects[map_things[thing].IndexOther];
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		prim_faces4[c0].ThingIndex=thing;
	}
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		prim_faces3[c0].ThingIndex=thing;
	}
}

void	clear_map(void)
{
	SLONG	x,z;
	background_prim=0;

	next_prim_point=1;
	next_prim_face4=1;
	next_prim_face3=1;
	next_prim_object=1;
	next_prim_multi_object=1;

	end_prim_point=MAX_PRIM_POINTS-1;
	end_prim_face4=MAX_PRIM_FACES4-1;
	end_prim_face3=MAX_PRIM_FACES3-1;
	end_prim_object=MAX_PRIM_OBJECTS-1;
	end_prim_multi_object=MAX_PRIM_MOBJECTS-1;

	next_face_selected=1;
	next_d_light=1;

//	next_col_vect=1;
//	next_col_vect_link=1;

	memset((UBYTE*)&edit_map[0][0],0,sizeof(struct DepthStrip)*EDIT_MAP_WIDTH*EDIT_MAP_HEIGHT);
//	memset((UBYTE*)&edit_map_eles[0],0,sizeof(EditMapElement)*65000);
	memset((UBYTE*)&map_things[0],0,sizeof(struct MapThing)*MAX_MAP_THINGS);

	memset((UBYTE*)wall_list,0,sizeof(struct FWall)*MAX_WALLS);
	memset((UBYTE*)storey_list,0,sizeof(struct FStorey)*MAX_STOREYS);
	memset((UBYTE*)building_list,0,sizeof(struct FBuilding)*MAX_BUILDINGS);



}


SLONG	fix_storey(SLONG	storey,SLONG	building,UBYTE magnify)
{
	SLONG	roof,wall;
	SLONG	some_walls=0;
	while(storey)
	{
		storey_list[storey].BuildingHead=building;
		if(magnify&1)
		{
			storey_list[storey].DX<<=1;
			storey_list[storey].DZ<<=1;
		}
		if(magnify&2)
		{
			storey_list[storey].DY=(storey_list[storey].DY*4)/5;
		}

//		roof=storey_list[storey].Roof;
//		if(roof)
//		{
//			fix_storey(roof,building,magnify);
//		}

/*
		wall=storey_list[roof].WallHead;
		LogText("Build %d  storey %d roofhead %d \n",c0,roof,wall); 
		while(wall)
		{
			LogText("r wall %d set to storey %d\n",wall,storey);
			wall_list[wall].StoreyHead=roof;
			wall_list[wall].DX<<=1;
			wall_list[wall].DZ<<=1;
			wall=wall_list[wall].Next;

			
		}
*/

		wall=storey_list[storey].WallHead;
		//LogText("building %d storey %d wallhead %d \n",c0,storey,wall); 
		while(wall)
		{
			some_walls=1;
//			LogText("wall %d set to storey %d\n",wall,storey);
			wall_list[wall].StoreyHead=storey;
			if(magnify&1)
			{
				wall_list[wall].DX<<=1;
				wall_list[wall].DZ<<=1;
			}
			if(magnify&1)
			{
				wall_list[wall].DY=(wall_list[wall].DY*4)/5;
			}
			wall=wall_list[wall].Next;
			
		}
		storey=storey_list[storey].Next;
	}

	return(some_walls);
}


void	fix_buildings(UBYTE	magnify)
{
	SLONG	c0;	
	SLONG	roof,storey,wall;

	for(c0=1;c0<MAX_STOREYS;c0++)
	{
		if(storey_list[c0].Next)
		{
			storey_list[storey_list[c0].Next].Prev=c0;
		}
	}
	for(c0=1;c0<MAX_BUILDINGS;c0++)
	{
		if(building_list[c0].BuildingFlags)
		{
			storey=building_list[c0].StoreyHead;
			if(storey)
			{
				SLONG	walls;
				walls=fix_storey(storey,c0,magnify);
				if(walls==0)
				{
					LogText(" building without walls %d\n",c0);
					building_list[c0].BuildingFlags=0;

				}
			}
			else
			{
				LogText(" building without storey %d\n",c0);
				building_list[c0].BuildingFlags=0;

			}
		}
	}
}


void	save_texture_styles(UBYTE world)
{
	UWORD	temp,temp2;
	SLONG	save_type=5;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;

	CBYTE fname[MAX_PATH];

	sprintf(fname, "%sstyle.tma", TEXTURE_WORLD_DIR);
//	sprintf(fname, "u:\\urbanchao\\textures\\world%d\\style.tma", world);

	handle=FileCreate(fname,1);

	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)&save_type,4);
		temp=9;		//how many texture_pages

//		FileWrite(handle,(UBYTE*)&temp,2);
//		FileWrite(handle,(UBYTE*)&texture_info[0],sizeof(struct TextureInfo)*8*8*temp);
		temp=200;
		temp2=5;
		FileWrite(handle,(UBYTE*)&temp,2);
		FileWrite(handle,(UBYTE*)&temp2,2);
		FileWrite(handle,(UBYTE*)&textures_xy[0][0],sizeof(struct TXTY)*temp*temp2);
		temp=200;
		temp2=21;
		FileWrite(handle,(UBYTE*)&temp,2);
		FileWrite(handle,(UBYTE*)&temp2,2);
		FileWrite(handle,(UBYTE*)&texture_style_names[0][0],temp*temp2);
		if(save_type>2)
		{
			temp=200;
			temp2=5;
			FileWrite(handle,(UBYTE*)&temp,2);
			FileWrite(handle,(UBYTE*)&temp2,2);
			FileWrite(handle,(UBYTE*)&textures_flags[0][0],sizeof(UBYTE)*temp*temp2);
		}

		FileClose(handle);
	}

}

void	save_texture_styles_psx(UBYTE world)
{
	UWORD	temp,temp2;
	SLONG	save_type=5;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;

	CBYTE fname[MAX_PATH];
	SLONG	si,pi;
	struct	TXTY	temp_t[200][5];

	
	for(si=0;si<200;si++)
	{
		for(pi=0;pi<5;pi++)
		{
			SLONG	page;

			page=textures_xy[si][pi].Page*64+(textures_xy[si][pi].Tx)+(textures_xy[si][pi].Ty)*8;
			if(page>=8*64)
				page=1;
			page=page_remap[page]-1;
//			ASSERT((page>>6)<4);
			temp_t[si][pi].Page=page&0xff;//(page>>6)&0xf;
//			textures_xy[si][pi].Tx=(page&7);//<<5;
//			textures_xy[si][pi].Ty=((page>>3)&7);//<<5;
			temp_t[si][pi].Flip=textures_xy[si][pi].Flip;
			temp_t[si][pi].Flip^=(page>>14)&3;

			

		}
	}



	sprintf(fname, "%sstyle.pma", TEXTURE_WORLD_DIR);

	handle=FileCreate(fname,1);

	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)&save_type,4);
		temp=9;		//how many texture_pages

		temp=200;
		temp2=5;
		FileWrite(handle,(UBYTE*)&temp,2);
		FileWrite(handle,(UBYTE*)&temp2,2);
		FileWrite(handle,(UBYTE*)&temp_t[0][0],sizeof(struct TXTY)*temp*temp2);
		temp=200;
		temp2=21;
		FileWrite(handle,(UBYTE*)&temp,2);
		FileWrite(handle,(UBYTE*)&temp2,2);
		FileWrite(handle,(UBYTE*)&texture_style_names[0][0],temp*temp2);
		if(save_type>2)
		{
			temp=200;
			temp2=5;
			FileWrite(handle,(UBYTE*)&temp,2);
			FileWrite(handle,(UBYTE*)&temp2,2);
			FileWrite(handle,(UBYTE*)&temp_t[0][0],sizeof(UBYTE)*temp*temp2);
		}

		FileClose(handle);
	}
}

void	fix_style_names(void)
{
	SLONG	c0,c1;
	for(c0=0;c0<200;c0++)
	{
		for(c1=0;c1<21;c1++)
		{
			if(texture_style_names[c0][c1]<33&&texture_style_names[c0][c1+1]<33)
			{
				texture_style_names[c0][c1]=0;
				break;
			}
		}
	}
}



//typical map data breakdown cumlative data used
/*
 load map data\COLMAP.MAP 
 after map 131072 
 after prim points 426636 
 after prim face4 732908 
 after prim face3 786696 
 after prim objects 809208 
 after prims 809208 
 after things 973210 
 after col_info 973276 
 after windows 2413284    1.5 Mb (unused)
 after walls 2863284 
 after storeys 2933284 
 after buildings 2959284 
 total size read 2959284 
*/


void	reset_floor_flags()
{
	SLONG	dx,dz;
	for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
	{
		for(dz=0;dz<EDIT_MAP_DEPTH;dz++)
		{
			edit_map[dx][dz].Flags&=~FLOOR_HIDDEN;
		}
	}

}


CBYTE	strips[128][128];

void	get_a_strip(ULONG	minsize,ULONG maxsize,UBYTE	dir)
{
	SLONG	dx,dz,x,z;
	UWORD	tex,texs;
	dx=0;
	for(x=0;x<EDIT_MAP_WIDTH;x++)
	{
		dx=0;
		for(z=0;z<EDIT_MAP_WIDTH;z+=dx)
		{
			if(strips[x][z]==0)
			{
				texs=edit_map[x][z].Texture&0x3fff;
				for(dx=0;dx<EDIT_MAP_WIDTH-x;dx++)
				{
					
					tex=edit_map[x][z+dx].Texture&0x3fff;
					if(tex!=texs || strips[x][z+dz])
						break;


				}
				if(dx>minsize && dx<maxsize)
				{
					strips[x][z]=1;

				}
			}

		}
	}

}

void	do_strips()
{
	memset(strips,0,128*128);
}

void	process_map()
{
	SLONG	dx,dz,x,z;
	SLONG	count,find;
	UWORD	textures[64],tc[130],tcz[130];
	SLONG	maxcount=0;
	UWORD	tex,texs;
	CBYTE	str[256];
	memset(tc,0,128);
	for(x=0;x<EDIT_MAP_WIDTH;x+=8)
	{
		for(z=0;z<EDIT_MAP_WIDTH;z+=8)
		{
			count=0;
			
			for(dx=0;dx<8;dx++)
			{
				for(dz=0;dz<8;dz++)
				{
					UWORD	tex;
					tex=edit_map[x+dx][z+dz].Texture&1023;
					find=0;
					while(find<count)
					{
						if(textures[find]==tex)
							break;

						find++;
					}
					ASSERT(find<8*8);
					if(find==count)
					{
						textures[find]=tex;
						count=find+1;
					}
				}
			}
			if(count>maxcount)
				maxcount=count;
			tc[count]++;
		}
	}

	sprintf(str,"max8x8= %d",maxcount);
	QuickTextC(70,70,str,0);
/*
	sprintf(str," 0.%d 1.%d 2.%d 3.%d 4.%d 5.%d 6.%d 7.%d 8.%d 9.%d 10.%d",tc[0],tc[1],tc[2],tc[3],tc[4],tc[5],tc[6],tc[7],tc[8],tc[9],tc[10]);
	QuickTextC(0,90,str,0);

	sprintf(str," 1.%d 2.%d 3.%d 4.%d 5.%d 6.%d 7.%d 8.%d 9.%d 10.%d",tc[11],tc[12],tc[13],tc[14],tc[15],tc[16],tc[17],tc[18],tc[19],tc[20]);
	QuickTextC(0,110,str,0);
*/

	memset(tc,0,2*129);
	memset(tcz,0,2*129);
	for(z=0;z<EDIT_MAP_WIDTH;z++)
	{
		dx=0;
		for(x=0;x<EDIT_MAP_WIDTH;x+=dx)
		{
			texs=edit_map[x][z].Texture&0x3fff;
			for(dx=0;dx<EDIT_MAP_WIDTH-x;dx++)
			{
				tex=edit_map[x+dx][z].Texture&0x3fff;
				if(tex!=texs)
					break;


			}
			ASSERT(dx!=0);
			ASSERT(dx<=128);
			tc[dx]++;

		}
	}

	dx=0;
	for(x=0;x<EDIT_MAP_WIDTH;x++)
	{
		dx=0;
		for(z=0;z<EDIT_MAP_WIDTH;z+=dx)
		{
			texs=edit_map[x][z].Texture&0x3fff;
			for(dx=0;dx<EDIT_MAP_WIDTH-x;dx++)
			{
				tex=edit_map[x][z+dx].Texture&0x3fff;
				if(tex!=texs)
					break;


			}
			ASSERT(dx!=0);
			ASSERT(dx<=128);
			tcz[dx]++;

		}
	}

	ULONG	tx,tz;
	for(x=1;x<=128;x++)
	{
		tx+=x*tc[x];
		tz+=x*tcz[x];
		sprintf(str," (%d)%d,%d",x,tc[x],tcz[x]);
		QuickTextC(((x-1)&7)*60,90+((x-1)>>3)*20,str,0);

	}
	x=128;
		sprintf(str,"tx %d tz %d",tx,tz);
		QuickTextC((x&7)*60,90+(x>>3)*20,str,0);






}

SLONG	load_map(CBYTE	*name)
{
	UWORD	temp_end_prim_point;
	UWORD	temp_end_prim_face4;
	UWORD	temp_end_prim_face3;
	UWORD	temp_end_prim_object;

	UWORD	no_prim_point;
	UWORD	no_prim_face4;
	UWORD	no_prim_face3;
	UWORD	no_prim_object;
	SLONG	save_type=1;
	UWORD	temp[4];
	SLONG	c0;
	SLONG	size=0;
	SLONG	x,z;
	SLONG	load_ok=0;
	struct	TinyStrip
	{
		UWORD	MapThingIndex;
	//	UWORD	Depth[EDIT_MAP_DEPTH];
		UWORD	ColVectHead;
	//	UWORD	Dummy1;
		UWORD	Texture;
		SWORD	Bright;
	}tinyfloor;


//	clear_map();
//extern	void	load_game_map(void);
//	load_game_map();
//	return;

	SLONG old_texture_set = editor_texture_set;

	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	handle=FileOpen(name);
	if(handle!=FILE_OPEN_ERROR)
	{
		SLONG	dx,dz;
		PAP_clear();

		LogText(" load map %s \n",name);
		FileRead(handle,(UBYTE*)&save_type,4);

		if(save_type<=8)
		{
			for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
			{
				for(dz=0;dz<EDIT_MAP_DEPTH;dz++)
				{
					size+=FileRead(handle,(UBYTE*)&tinyfloor,sizeof(struct TinyStrip));
					edit_map[dx][dz].MapThingIndex=tinyfloor.MapThingIndex;
					edit_map[dx][dz].ColVectHead=tinyfloor.ColVectHead;
					edit_map[dx][dz].Texture=tinyfloor.Texture;
					edit_map[dx][dz].Bright=tinyfloor.Bright;
				}
			}
		}
		else
		{
			size+=FileRead(handle,(UBYTE*)edit_map,sizeof(struct DepthStrip)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);
		}



		if(save_type>19)
		{
			FileRead(handle,(UBYTE*)tex_map,sizeof(UWORD)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);
		}
		else
		{
			memset((UBYTE*)tex_map,0,sizeof(UWORD)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);

		}


		//
		// clean the muck out the mapwho
		//

		for(x=0;x<EDIT_MAP_WIDTH;x++)
		for(z=0;z<EDIT_MAP_DEPTH;z++)
		{
			edit_map[x][z].MapThingIndex=0;
		}

		if(save_type>18)
		{
			FileRead(handle,(UBYTE*)edit_map_roof_height,sizeof(SBYTE)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);
		}
		else
		{
			memset((UBYTE*)&edit_map_roof_height[0][0],0,sizeof(SBYTE)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);
		}


		if (save_type < 13)
		{
			//
			// This is where I changed FLOOR_HEIGHT_SHIFT;
			//

			for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
			{
				for(dz=0;dz<EDIT_MAP_DEPTH;dz++)
				{
					edit_map[dx][dz].Y <<= 2;
				}
			}
		}


		reset_floor_flags();

		LogText(" after map %d \n",size);

		size+=FileRead(handle,(UBYTE*)&temp_end_prim_point,sizeof(UWORD));
		size+=FileRead(handle,(UBYTE*)&temp_end_prim_face4,sizeof(UWORD));
		size+=FileRead(handle,(UBYTE*)&temp_end_prim_face3,sizeof(UWORD));
		size+=FileRead(handle,(UBYTE*)&temp_end_prim_object,sizeof(UWORD));

		size+=FileRead(handle,(UBYTE*)&no_prim_point ,sizeof(UWORD));
		size+=FileRead(handle,(UBYTE*)&no_prim_face4 ,sizeof(UWORD));
		size+=FileRead(handle,(UBYTE*)&no_prim_face3 ,sizeof(UWORD));
		size+=FileRead(handle,(UBYTE*)&no_prim_object,sizeof(UWORD));

		end_prim_point =MAX_PRIM_POINTS -no_prim_point;
		end_prim_face4 =MAX_PRIM_FACES4 -no_prim_face4;
		end_prim_face3 =MAX_PRIM_FACES3 -no_prim_face3;
		end_prim_object=MAX_PRIM_OBJECTS-no_prim_object;

		if(save_type<24)
		{
			SLONG	c0;
			struct	OldPrimPoint	pp;
			for(c0=0;c0<no_prim_point;c0++)
			{
				size+=FileRead(handle,(UBYTE*)&pp ,sizeof(struct OldPrimPoint) );

				prim_points[end_prim_point+c0].X=(SWORD)pp.X;
				prim_points[end_prim_point+c0].Y=(SWORD)pp.Y;
				prim_points[end_prim_point+c0].Z=(SWORD)pp.Z;
			}

		}
		else
		{
			size+=FileRead(handle,(UBYTE*)&prim_points[end_prim_point ] ,sizeof(struct PrimPoint) *(no_prim_point));
		}
		LogText(" after prim points %d \n",size);
		size+=FileRead(handle,(UBYTE*)&prim_faces4[end_prim_face4 ] ,sizeof(struct PrimFace4) *(no_prim_face4 ));
		LogText(" after prim face4 %d \n",size);
		size+=FileRead(handle,(UBYTE*)&prim_faces3[end_prim_face3 ] ,sizeof(struct PrimFace3) *(no_prim_face3 ));
		LogText(" after prim face3 %d \n",size);


		if(save_type<27)
		{
			SLONG	c0;
			struct	PrimObjectOld oldprim;
			for(c0=0;c0<no_prim_object;c0++)
			{
				FileRead(handle,&oldprim, sizeof(PrimObjectOld));

				prim_objects[end_prim_object+c0].coltype=oldprim.coltype;
				prim_objects[end_prim_object+c0].damage=oldprim.damage;
				prim_objects[end_prim_object+c0].EndFace3=oldprim.EndFace3;
				prim_objects[end_prim_object+c0].EndFace4=oldprim.EndFace4;
				prim_objects[end_prim_object+c0].StartFace3=oldprim.StartFace3;
				prim_objects[end_prim_object+c0].StartFace4=oldprim.StartFace4;
				prim_objects[end_prim_object+c0].EndPoint=oldprim.EndPoint;
				prim_objects[end_prim_object+c0].StartPoint=oldprim.StartPoint;
				prim_objects[end_prim_object+c0].shadowtype=oldprim.shadowtype;
				prim_objects[end_prim_object+c0].flag=oldprim.flag;
//				memcpy(prim_names[prim],oldprim.ObjectName,32);

			}
		}
		else
		{

			size+=FileRead(handle,(UBYTE*)&prim_objects[end_prim_object],sizeof(struct PrimObject)*(no_prim_object));
			LogText(" after prim objects %d \n",size);
		}

		LogText(" after prims %d \n",size);

		
		for(c0=end_prim_face3+1;c0<MAX_PRIM_FACES3;c0++)
		{
			prim_faces3[c0].Points[0]+=-temp_end_prim_point+end_prim_point;
			prim_faces3[c0].Points[1]+=-temp_end_prim_point+end_prim_point;
			prim_faces3[c0].Points[2]+=-temp_end_prim_point+end_prim_point;
		}
		for(c0=end_prim_face4+1;c0<MAX_PRIM_FACES4;c0++)
		{
			prim_faces4[c0].Points[0]+=-temp_end_prim_point+end_prim_point;
			prim_faces4[c0].Points[1]+=-temp_end_prim_point+end_prim_point;
			prim_faces4[c0].Points[2]+=-temp_end_prim_point+end_prim_point;
			prim_faces4[c0].Points[3]+=-temp_end_prim_point+end_prim_point;
//			if(prim_faces4[c0].TexturePage==1)
//				prim_faces4[c0].TexturePage=0;
		}
		for(c0=end_prim_object+1;c0<MAX_PRIM_OBJECTS;c0++)
		{
			prim_objects[c0].StartPoint+=-temp_end_prim_point+end_prim_point;
			prim_objects[c0].EndPoint  +=-temp_end_prim_point+end_prim_point;

			prim_objects[c0].StartFace3+=-temp_end_prim_face3+end_prim_face3;
			prim_objects[c0].EndFace3  +=-temp_end_prim_face3+end_prim_face3;

			prim_objects[c0].StartFace4+=-temp_end_prim_face4+end_prim_face4;
			prim_objects[c0].EndFace4  +=-temp_end_prim_face4+end_prim_face4;
		}

		size+=FileRead(handle,(UBYTE*)&background_prim,sizeof(UWORD));
		background_prim=0;

		if (save_type<26) {
			size+=FileRead(handle,(UBYTE*)&map_things[0],sizeof(struct MapThing)*MAX_MAP_THINGS);

			for(c0=1;c0<MAX_MAP_THINGS;c0++)
			{
				map_things[c0].MapChild=0;
				map_things[c0].MapParent=0;
				if(map_things[c0].Type==MAP_THING_TYPE_PRIM||
				   map_things[c0].Type==MAP_THING_TYPE_ANIM_PRIM)
				{
					add_thing_to_edit_map(map_things[c0].X>>ELE_SHIFT,map_things[c0].Z>>ELE_SHIFT,c0);

					//map_things[c0].IndexOther+=-temp_end_prim_object+end_prim_object;
					//set_things_faces(c0);
				}
				else
				{
					delete_thing(c0);
				}
			}
		}


			size+=FileRead(handle,(UBYTE*)&edit_info.amb_dx,4*5);
			size+=FileRead(handle,(UBYTE*)&next_col_info,2);
			size+=FileRead(handle,(UBYTE*)col_info,sizeof(struct ColInfo)*next_col_info);
		LogText(" after col_info %d \n",size);

		size+=FileRead(handle,(UBYTE*)&temp[0],2);
		size+=FileRead(handle,(UBYTE*)&temp[1],2);
		size+=FileRead(handle,(UBYTE*)&temp[2],2);
		size+=FileRead(handle,(UBYTE*)&temp[3],2);

		size+=FileRead(handle,(UBYTE*)window_list,sizeof(struct FWindow)*temp[0]);
		LogText(" after windows %d \n",size);

		for(c0=0;c0<MAX_WALLS;c0++)
		{
			if(wall_list[c0].Textures&&wall_list[c0].Tcount)
			{
				MemFree((void*)wall_list[c0].Textures);
				wall_list[c0].Textures=0;
				wall_list[c0].Tcount=0;
			}
			if(wall_list[c0].Textures2&&wall_list[c0].Tcount2)
			{
				MemFree((void*)wall_list[c0].Textures2);
				wall_list[c0].Textures2=0;
				wall_list[c0].Tcount2=0;
			}
		}


		if(save_type>=17)
		{
			for(c0=0;c0<MAX_WALLS;c0++)
			{
				size+=FileRead(handle,(UBYTE*)&wall_list[c0],sizeof(struct FWall)*1);
				if(wall_list[c0].Tcount&&wall_list[c0].Textures)
				{

					wall_list[c0].Textures=(UBYTE*)MemAlloc(wall_list[c0].Tcount);
					ASSERT(wall_list[c0].Textures);

					size+=FileRead(handle,(UBYTE*)wall_list[c0].Textures,wall_list[c0].Tcount);
				}
				if(save_type>21)
				{
					if(wall_list[c0].Tcount2&&wall_list[c0].Textures2)
					{

						wall_list[c0].Textures2=(UBYTE*)MemAlloc(wall_list[c0].Tcount2);
						ASSERT(wall_list[c0].Textures2);

						size+=FileRead(handle,(UBYTE*)wall_list[c0].Textures2,wall_list[c0].Tcount2);
					}
				}
				else
				{
					wall_list[c0].Tcount2=0;
					wall_list[c0].Textures2=0;

				}
			}
			size+=FileRead(handle,(UBYTE*)storey_list,sizeof(struct FStorey)*temp[2]);
			LogText(" after storeys %d \n",size);
			if(save_type<21)
			{
				for(c0=0;c0<temp[2];c0++)
				{
					storey_list[c0].InsideStorey=0;
				}
			}
			if(save_type<23)
			{
				for(c0=0;c0<temp[2];c0++)
				{
					storey_list[c0].InsideIDIndex=0;
				}
			}

		}
		else
		{


			size+=FileRead(handle,(UBYTE*)wall_list,sizeof(struct FWall)*temp[1]);
			LogText(" after walls %d \n",size);
			size+=FileRead(handle,(UBYTE*)storey_list,sizeof(struct FStorey)*temp[2]);
			LogText(" after storeys %d \n",size);


			if(save_type<21)
			{
				for(c0=0;c0<temp[2];c0++)
				{
					storey_list[c0].InsideStorey=0;
				}
			}

		}

		size+=FileRead(handle,(UBYTE*)building_list,sizeof(struct FBuilding)*temp[3]);
		//LogText(" after buildings %d \n",size);


		if(save_type>22)
		{
			UWORD	temp;
			SLONG	c0,stair;

			FileRead(handle,(UBYTE*)&temp,sizeof(temp));
			next_inside=temp;
			FileRead(handle,(UBYTE*)&room_ids[0],sizeof(struct RoomID)*temp);
			for(c0=1;c0<next_inside;c0++)
			{
				for(stair=0;stair<MAX_STAIRS_PER_FLOOR;stair++)
				{
					if(room_ids[c0].StairFlags[stair])
					{
						if(room_ids[c0].StairsX[stair]>127)
						{
							room_ids[c0].StairsX[stair]=127;
							room_ids[c0].StairFlags[stair]=0;
						}
						if(room_ids[c0].StairsY[stair]>127)
						{
							room_ids[c0].StairsY[stair]=127;
							room_ids[c0].StairFlags[stair]=0;
						}
					}
				}

			}

		}

		//
		// cables now have a dangle factor built in
		//
		if(save_type<25)
		{
			for(c0=0;c0<MAX_STOREYS;c0++)
			{
				if(storey_list[c0].StoreyFlags)
				{
					SLONG	wall;

					if(storey_list[c0].StoreyType==STOREY_TYPE_CABLE)
					{
						wall=storey_list[c0].WallHead;
						while(wall)
						{
							wall_list[wall].TextureStyle2=4;
							wall=wall_list[wall].Next;
						}

					}
					
				}
			}
		}




		//LogText(" total size read %d \n",size);
		if(save_type<8)
			offset_buildings(64<<ELE_SHIFT,0,64<<ELE_SHIFT);

		if(save_type==10)
			fix_buildings(2); // make sure next/prev tally by resetting the prev field 
		else
		if(save_type<10)
			fix_buildings(3); // make sure next/prev tally by resetting the prev field 
		else
			fix_buildings(0); // make sure next/prev tally by resetting the prev field 
		if(save_type==12)
		{
			SLONG	c0;
			for(c0=0;c0<MAX_STOREYS;c0++)
			{
				if(storey_list[c0].StoreyType==STOREY_TYPE_LADDER)
				{
					storey_list[c0].Height<<=2;
				}
			}
		}
		if(save_type<17)
		{
			SLONG	c0;
			for(c0=0;c0<MAX_WALLS;c0++)
			{
				wall_list[c0].Textures=0;
				wall_list[c0].Tcount=0;
			}
		}

		if (save_type >= 14)
		{
			FileRead(handle,(UBYTE*)EXTRA_thing,sizeof(EXTRA_thing));
		}

		if (save_type >= 21)
		{
			FileRead(handle,(UBYTE*)&editor_texture_set,sizeof(editor_texture_set));
		}
		else
		{
			editor_texture_set = 1;
		}

		//
		// check the storey link list of this building all actually belong to this building
		// this is to fix bug of duplicate faces caused by screwy building link list
		{
			SLONG	c0;
			for(c0=1;c0<MAX_BUILDINGS;c0++)
			{
				SLONG	storey;
				if(building_list[c0].StoreyHead)
				{
					storey=building_list[c0].StoreyHead;
					while(storey)
					{

						if(storey_list[storey].Next)
						if(storey_list[storey_list[storey].Next].BuildingHead!=c0)
						{
							storey_list[storey].Next=0;
						}

						storey=storey_list[storey].Next;
					}

				}
			}
		}


		if (save_type >= 26) {
			size+=FileRead(handle,(UBYTE*)&map_things[0],sizeof(struct MapThing)*MAX_MAP_THINGS);

			for(c0=1;c0<MAX_MAP_THINGS;c0++)
			{
				map_things[c0].MapChild=0;
				map_things[c0].MapParent=0;
				if(map_things[c0].Type==MAP_THING_TYPE_PRIM||
				   map_things[c0].Type==MAP_THING_TYPE_ANIM_PRIM)
				{
					add_thing_to_edit_map(map_things[c0].X>>ELE_SHIFT,map_things[c0].Z>>ELE_SHIFT,c0);

					//map_things[c0].IndexOther+=-temp_end_prim_object+end_prim_object;
					//set_things_faces(c0);
				}
				else
				{
					delete_thing(c0);
				}
			}
		}


		FileClose(handle);

		load_tex_remap(name);

		{
			SLONG	index;
			struct	MapThing	*p_thing;
			index=background_prim;
			while(index)
			{
				p_thing=TO_MTHING(index);
				p_thing->Type=MAP_THING_TYPE_PRIM;
				index=p_thing->IndexNext;
			}
		}
		load_ok=1;
	}

	//
	// Load the correct set of textures for this map.
	//

	void update_modules    (void);

	if (editor_texture_set != old_texture_set)
	{
		free_game_textures(FREE_UNSHARED_TEXTURES);
		load_game_textures(LOAD_UNSHARED_TEXTURES);

		update_modules();
	}

	{
		SLONG	index,count=0;
		LogText(" *************************************\n");
		index=edit_map[48][45].MapThingIndex;
		LogText(" on map cell [48][45] = index %d \n",index);
		while(index&&count++<100)
		{
			LogText(" index %d type %d \n",index,map_things[index].Type);
			index=map_things[index].MapChild;
		}
		LogText(" *************************************\n");
	}

	return(load_ok);

}

void	setup_ambient(SLONG dx,SLONG dy,SLONG dz,SLONG bright,SLONG flags)
{
	edit_info.amb_dx=dx;
	edit_info.amb_dz=dz;
	edit_info.amb_dy=dy;
	edit_info.amb_bright=bright;
	edit_info.amb_flags=flags;
}



UWORD	is_it_clockwise(struct SVector *res,SLONG p1,SLONG p2,SLONG p3)
{
	SLONG	z;
	SLONG	vx,vy,wx,wy;
	
	vx=res[p2].X-res[p1].X;
	wx=res[p3].X-res[p2].X;
	vy=res[p2].Y-res[p1].Y;
	wy=res[p3].Y-res[p2].Y;
	z=vx*wy-vy*wx;

	if(z>0)
		return	1;
	else
		return	0;
}


void	set_quad_buckets_texture(struct	BucketQuad	*p_bucket,struct	TextureBits *texture)
{
	UBYTE	sx,sy,w,h;

	sx=texture->X<<3;
	sy=texture->Y<<3;
	w=texture_sizes[texture->Width];
	h=texture_sizes[texture->Height];
	setUV4(p_bucket,sx,sy,sx+w-1,sy,sx,sy+h-1,sx+w-1,sy+h-1,(SWORD)texture->Page);
}


inline	void	insert_bucket_vect(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SWORD col)
{
	SLONG	flag_and,flag_or;
	SLONG	az;
	struct BucketVect	*p_bucket;

	if(current_bucket_pool>=end_bucket_pool)
		return;

	p_bucket =(struct BucketVect*)current_bucket_pool;

	az=(z1+z2)>>1;

	setPolyGVect(p_bucket);
	setXY2(p_bucket,x1,y1,x2,y2);
	p_bucket->Col=col;
	add_bucket(p_bucket,az-150);
	current_bucket_pool+=sizeof(struct BucketVect);

}

UWORD	is_it_clockwise_xy(SLONG x1,SLONG y1,SLONG x2,SLONG y2,SLONG x3,SLONG y3)
{
	SLONG	z;
	SLONG	vx,vy,wx,wy;
	
	vx=x2-x1; //point2->X-point1->X;
	wx=x3-x2; //point3->X-point2->X;
	vy=y2-y1; //point2->Y-point1->Y;
	wy=y3-y2; //point3->Y-point2->Y;
	z=vx*wy-vy*wx;

	if(z>0)
		return	1;
	else
		return	0;
}

inline	struct BucketQuad *insert_quad(SLONG	*flags,struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG p4,struct TextureBits t)
{
	SLONG	flag_and,flag_or;
	SLONG	az;
	struct	BucketQuad	*p_bucket;

	if(current_bucket_pool>=end_bucket_pool)
		return(0);

	p_bucket=(struct BucketQuad*)current_bucket_pool;

	az=(res[p1].Z+res[p2].Z+res[p3].Z+res[p4].Z)>>2;

	flag_and=flags[p1]&flags[p2]&flags[p3]&flags[p4];
	flag_or=flags[p1]|flags[p2]|flags[p3]|flags[p4];
	if(!is_it_clockwise_xy(res[p1].X,res[p1].Y,res[p2].X,res[p2].Y,res[p3].X,res[p3].Y))
	{
		if( ((flag_or&EF_BEHIND_YOU)==0) && !(flag_and & EF_CLIPFLAGS))
		{
	//		setPolyGT4(p_bucket);
			setPolyType4(p_bucket,t.DrawFlags);
//			setPolyType4(p_bucket,t.DrawFlags|POLY_FLAG_SEMI_TRANS);
			setCol4(p_bucket,0xff);
			setZ4(p_bucket,res[p4].Z,res[p3].Z,res[p1].Z,res[p2].Z);
			setXY4(p_bucket,res[p4].X,res[p4].Y,res[p3].X,res[p3].Y,res[p1].X,res[p1].Y,res[p2].X,res[p2].Y);
			set_quad_buckets_texture(p_bucket,&t);
			setShade4(p_bucket,128,128,128,128);
			p_bucket->DebugInfo=0;
			p_bucket->DebugFlags=0;
			add_bucket(p_bucket,az);
			current_bucket_pool+=sizeof(struct	BucketQuad);
			return(p_bucket);
		}
		else
			return(0);
	}
	else
		return(0);
}

inline	struct BucketTri *insert_tri(SLONG	*flags,struct SVector *res,SLONG p1,SLONG p2,SLONG p3)
{
	SLONG	flag_and,flag_or;
	struct	BucketTri	*p_bucket;

	if(current_bucket_pool>=end_bucket_pool)
		return(0);

	p_bucket=(struct BucketTri*)current_bucket_pool;

		flag_and=flags[p1]&flags[p2]&flags[p3];
		flag_or=flags[p1]|flags[p2]|flags[p3];
		if( ((flag_or&EF_BEHIND_YOU)==0) && !(flag_and & EF_CLIPFLAGS))
		{
			setPolyGT3(p_bucket);
			setXY3(p_bucket,res[p1].X,res[p1].Y,res[p2].X,res[p2].Y,res[p3].X,res[p3].Y);
			setUV3(p_bucket,64+32,32,64+64,32,64+64,64,1);
			setShade3(p_bucket,64,128,128);
			p_bucket->DebugInfo=0;
			add_bucket(p_bucket,res[0].Z);
			current_bucket_pool+=sizeof(struct	BucketTri);
			return(++p_bucket);
		}
		return(p_bucket);
		return(0);
}

#define	TSHIFT	8
UBYTE	check_big_point_triangle(SLONG x,SLONG y,SLONG ux,SLONG uy,SLONG vx,SLONG vy,SLONG wx,SLONG wy)
{
	SLONG	s,t,top,bot,res;

	GlobalXYToLocal(&x,&y);
	top	=	(y-uy)*(wx-ux)+(ux-x)*(wy-uy);
	bot	=	(vy-uy)*(wx-ux)-(vx-ux)*(wy-uy);

	
//	if(next_col_column<5)
//		printf(" top %d bot %d \n",top,bot);

	if(bot==0)
		return 0;

	s=(top<<TSHIFT)/bot;
	if(s<0)
		return 0;
	if((wx-ux)==0)
		t=((y<<TSHIFT)-(uy<<TSHIFT)-s*(vy-uy))/(wy-uy);
	else
		t=((x<<TSHIFT)-(ux<<TSHIFT)-s*(vx-ux))/(wx-ux);
	if(t<0)
		return 0;

	res=s+t;
	if( res<(1<<TSHIFT))
	{
//		if(next_col_column<5)
//			printf(" s %d t %d \n",s>>6,t>>6);

		return	1;  // point inside triangle
	}
	else
		return	0;  // point outside triangle
}


// p1   p2
//
// p2   p3

BOOL	check_mouse_over_prim_quad(struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG p4,SLONG face)
{
	SLONG	az;


	az	=	(res[p1].Z+res[p2].Z+res[p3].Z+res[p4].Z)>>2;
	if(!is_it_clockwise(res,p3,p2,p1))
	{
		if	(
				check_big_point_triangle(MouseX,MouseY,res[p1].X,res[p1].Y,res[p2].X,res[p2].Y,res[p3].X,res[p3].Y)	||
				check_big_point_triangle(MouseX,MouseY,res[p2].X,res[p2].Y,res[p4].X,res[p4].Y,res[p3].X,res[p3].Y)
			)
		{
			if(az < hilited_face.Z||hilited_face.EditTurn!=editor_turn)
			{
				hilited_face.Face		=	face;
				hilited_face.EditTurn	=	editor_turn;
				hilited_face.Z			=	az;
				hilited_face.PEle		=	(struct EditMapElement*)-1;
				hilited_face.Bucket		=	(struct BucketHead*)current_bucket_pool;
				return	TRUE;
			}
		}
	}
	return	FALSE;
}

BOOL	check_mouse_over_floor_quad(SLONG x1,SLONG y1,SLONG x2,SLONG y2,SLONG x3,SLONG y3, SLONG x4,SLONG y4,SLONG face,SLONG az)
{
	if	(
			check_big_point_triangle(MouseX,MouseY,x1,y1,x2,y2,x3,y3)	||
			check_big_point_triangle(MouseX,MouseY,x2,y2,x4,y4,x3,y3)
		)
	{
		if(az < hilited_face.Z||hilited_face.EditTurn!=editor_turn)
		{
			hilited_face.Face		=	face;
			hilited_face.EditTurn	=	editor_turn;
			hilited_face.Z			=	az;
			hilited_face.PEle		=	(struct EditMapElement*)-2;
			hilited_face.Bucket		=	(struct BucketHead*)current_bucket_pool;
			return	TRUE;
		}
	}
	return	FALSE;
}






BOOL	check_mouse_over_prim_tri(struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG face)
{
	SLONG	az;


	az	=	(res[p1].Z+res[p2].Z+res[p3].Z)/3;
	if(!is_it_clockwise(res,p3,p2,p1))
	{
		if(check_big_point_triangle(MouseX,MouseY,res[p1].X,res[p1].Y,res[p2].X,res[p2].Y,res[p3].X,res[p3].Y))
		{
			if(az < hilited_face.Z||hilited_face.EditTurn!=editor_turn)
			{
				hilited_face.Face		=	-face;
				hilited_face.EditTurn	=	editor_turn;
				hilited_face.Z			=	az;
				hilited_face.PEle		=	(struct EditMapElement*)-1;
				hilited_face.Bucket		=	(struct BucketHead*)current_bucket_pool;
				return	TRUE;
			}
		}
	}
	return	FALSE;
}

void	check_mouse_quad(struct EditMapElement *p_ele,struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG p4,SLONG wx,SLONG wy,SLONG wz,SLONG face)
{
	SLONG	az;
	static	SLONG count;
	if(hilited_face.EditTurn!=editor_turn)
	{
		count=0;
	}
/*
	if(editor.TexturePriority)
	{
		if(MouseX<256&&MouseY<256)
			return;
	}
*/

	az=(res[p1].Z+res[p2].Z+res[p3].Z+res[p4].Z)>>2;
	if(!is_it_clockwise(res,p1,p2,p3))
	{
		if(check_big_point_triangle(MouseX,MouseY,res[p3].X,res[p3].Y,res[p2].X,res[p2].Y,res[p1].X,res[p1].Y)||
		   check_big_point_triangle(MouseX,MouseY,res[p1].X,res[p1].Y,res[p4].X,res[p4].Y,res[p3].X,res[p3].Y))
		{
			if(az < hilited_face.Z||hilited_face.EditTurn!=editor_turn)
			{
				count++;
				hilited_face.MapX=wx;				
				hilited_face.MapY=wy;				
				hilited_face.MapZ=wz;				
				hilited_face.Face=face;
				hilited_face.EditTurn=editor_turn;
				hilited_face.PEle=p_ele;
				hilited_face.Z=az;
				hilited_face.Bucket	=	0; //(struct BucketHead*)current_bucket_pool;
			}
		}
	}
}



struct	DisplayTypes
{
	SLONG	Width,Height,Depth;

};

struct DisplayTypes display_types[]=
{
	{320,200,8},
	{640,480,8},
	{800,600,8},
	{1024,768,8},
	{1280,1024,8},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0},
};

void	gamut_fiddle(void)
{

	UBYTE	temp_pal[768];
	SLONG	c0,temp;
	static	SLONG	gamut=0;
	UBYTE	*pal;
	
	if((pal=PALETTE)==0)
		return;

	if(Keys[KB_F12])
	{
		gamut+=ShiftFlag?-1:+1;
		if(gamut<-256)
			gamut=-256;
		if(gamut>256)
			gamut=256;
		for(c0=0;c0<256*3;c0++)
		{
			temp=(pal[c0]+gamut);
			if(temp>255)
				temp=255;
			if(temp<0)
				temp=0;

			temp_pal[c0]=temp;
		}
		SetPalette(temp_pal);
	}
}

ULONG	engine_keys_scroll_game(void)
{
   	SLONG	dx=0,dy=0,dz=0;
	ULONG	change=0;
	SLONG	scale;

	if(ShiftFlag)
	{
		if(Keys[KB_RIGHT])
		{
/*
			dx=(SIN( ((engine.AngleY>>8)+2048+512)&2047)*80)>>16;
			dz=(COS( ((engine.AngleY>>8)+2048+512)&2047)*80)>>16;
			change=1;
*/
			dx=100;
			change=1;
		}
		else if(Keys[KB_LEFT])
		{
/*
			dx=-(SIN( ((engine.AngleY>>8)+2048+512)&2047)*80)>>16;
			dz=-(COS( ((engine.AngleY>>8)+2048+512)&2047)*80)>>16;
			change=1;
*/
			dx=-100;
			change=1;
		}
		if(Keys[KB_UP])
		{
			dy=100;
			change=1;
		}

		if(Keys[KB_DOWN])
		{
			dy=-1000;
			change=1;
		}
	}
	else
	{
		if(Keys[KB_RIGHT])
		{
/*
			engine.AngleY-=2048<<1;
			engine.AngleY=((engine.AngleY+(2048<<8))&((2048<<8)-1));
	//		if(engine.AngleY< (1536<<8) && (engine.AngleY>512<<8))
	//			engine.AngleY=512<<8;
			change=1;
*/
			dx=(SIN( ((engine.AngleY>>8)+2048+512)&2047)*200)>>16;
			dz=(COS( ((engine.AngleY>>8)+2048+512)&2047)*200)>>16;
			change=1;
		}
		else if(Keys[KB_LEFT])
		{
/*
			engine.AngleY+=2048<<1;
			engine.AngleY=((engine.AngleY+(2048<<8))&((2048<<8)-1));
	//		if(engine.AngleY< (1536<<8) && (engine.AngleY>512<<8))
	//			engine.AngleY=1536<<8;
			change=1;
*/
			dx=-(SIN( ((engine.AngleY>>8)+2048+512)&2047)*200)>>16;
			dz=-(COS( ((engine.AngleY>>8)+2048+512)&2047)*200)>>16;

			change=1;
		}
		
		if(Keys[KB_UP])
		{
			dx=-(SIN( ((engine.AngleY>>8)+2048)&2047)*150)>>16;
			dz=-(COS( ((engine.AngleY>>8)+2048)&2047)*150)>>16;
			change=1;
		}

		if(Keys[KB_DOWN])
		{
			dx=(SIN( ((engine.AngleY>>8)+2048)&2047)*150)>>16;
			dz=(COS( ((engine.AngleY>>8)+2048)&2047)*150)>>16;
			change=1;
		}
	}
	if(change)
	{
		scale=3000-(engine.Scale+1000);
		scale>>=8;
		if(scale<=0)
			scale=1;

		dx*=scale;
		dz*=scale;

		dx>>=2;
		dz>>=2;

		engine.X+=dx<<8;
		engine.Y+=dy<<8;
		engine.Z+=dz<<8;
	}
	return(change);
	
}

SLONG	calc_step_size(void)
{
	SLONG	scale;
 	scale=3000-(engine.Scale+1000);

	if(scale<1)
		scale=1;

	scale<<=6;
	return(scale);

}

ULONG	engine_keys_scroll(void)
{
	SLONG	update=0;
	SLONG	step_size;

	step_size=calc_step_size();

	if(ControlFlag)
	{
		
		if(Keys[KB_1])
		{
			engine.Z=0;
			update	=	1;
		}
		if(Keys[KB_2])
		{
			engine.Z=64<<8;
			update	=	1;
		}
		if(Keys[KB_3])
		{
			engine.Z=128<<8;
			update	=	1;
		}
		if(Keys[KB_4])
		{
			engine.Z=(128+64)<<8;
			update	=	1;
		}
	}

	if(Keys[KB_LEFT]) //&&!(ShiftFlag))
	{
		Keys[KB_LEFT]=0;
		engine.X-=step_size;
		update	=	1;
	}
	if(Keys[KB_RIGHT]) //&&!(ShiftFlag))
	{
		Keys[KB_RIGHT]=0;
		engine.X+=step_size;
		update	=	1;
	}
	if(Keys[KB_UP]&&!(ShiftFlag))
	{
		Keys[KB_UP]=0;
		engine.Y-=step_size;
		update	=	1;
	}
	if(Keys[KB_DOWN]&&!(ShiftFlag))
	{
		Keys[KB_DOWN]=0;
		engine.Y+=step_size;
		update	=	1;
	}
	if(Keys[KB_INS]||(Keys[KB_UP]&&ShiftFlag))
	{
		Keys[KB_INS]=0;
		engine.Z-=step_size;
//		if( ((engine.Z>>ELE_SHIFT)>>8)<0)
//		engine.Z=0;
		update	=	1;
	}
	if(Keys[KB_PGUP]||(Keys[KB_DOWN]&&ShiftFlag))
	{
		Keys[KB_PGUP]=0;
		engine.Z+=step_size;
//		if( ((engine.Z>>ELE_SHIFT)>>8)>16)
//			engine.Z=16<<(ELE_SHIFT+8);
		update	=	1;
	}


	return(update);
}

ULONG	engine_keys_scroll_plan(void)
{
	SLONG	update=0;
	SLONG	step_size=256<<8;
//	step_size=calc_step_size();

	if(ShiftFlag)
		step_size<<=2;

	if(Keys[KB_LEFT]) //&&!(ShiftFlag))
	{
		Keys[KB_LEFT]=0;
		engine.X-=step_size;
		update	=	1;
	}
	if(Keys[KB_RIGHT]) //&&!(ShiftFlag))
	{
		Keys[KB_RIGHT]=0;
		engine.X+=step_size;
		update	=	1;
	}
	if(Keys[KB_UP]&&!(ShiftFlag))
	{
		Keys[KB_UP]=0;
		engine.Z-=step_size;
		update	=	1;
	}
	if(Keys[KB_DOWN]&&!(ShiftFlag))
	{
		Keys[KB_DOWN]=0;
		engine.Z+=step_size;
		update	=	1;
	}
	if(Keys[KB_PGUP]||(Keys[KB_UP]&&ShiftFlag))
	{
		Keys[KB_PGUP]=0;
		Keys[KB_UP]=0;
		engine.Y-=step_size;
//		if( ((engine.Z>>ELE_SHIFT)>>8)<0)
//		engine.Z=0;
		update	=	1;
	}
	if(Keys[KB_PGDN]||(Keys[KB_DOWN]&&ShiftFlag))
	{
		Keys[KB_PGDN]=0;
		Keys[KB_DOWN]=0;
		engine.Y+=step_size;
//		if( ((engine.Z>>ELE_SHIFT)>>8)>16)
//			engine.Z=16<<(ELE_SHIFT+8);
		update	=	1;
	}
	return(update);
}

ULONG	engine_keys_spin(void)
{
	SLONG	update=0;
	gamut_fiddle();
	if(Keys[KB_DEL])
	{
		engine.AngleY+=2048;
		engine.AngleY+=2048;
		engine.AngleY+=2048;
		engine.AngleY+=2048;
		engine.AngleY=((engine.AngleY+(2048<<8))&((2048<<8)-1));
//		if(engine.AngleY< (1536<<8) && (engine.AngleY>512<<8))
//			engine.AngleY=512<<8;
		update	=	1;
	}

	if(Keys[(0x51 + 0x80)])
	{
		engine.AngleY-=2048;
		engine.AngleY-=2048;
		engine.AngleY-=2048;
		engine.AngleY-=2048;
		engine.AngleY=((engine.AngleY+(2048<<8))&((2048<<8)-1));
//		if(engine.AngleY< (1536<<8) && (engine.AngleY>512<<8))
//			engine.AngleY=1536<<8;
		update	=	1;
	}
		
	if(Keys[KB_HOME])
	{
		engine.AngleX+=2048;
		engine.AngleX+=2048;
		engine.AngleX+=2048;
		engine.AngleX+=2048;
		engine.AngleX=((engine.AngleX+(2048<<8))&((2048<<8)-1));
//		if(engine.AngleX< (1536<<8) && (engine.AngleX>512<<8))
//			engine.AngleX=512<<8;
		update	=	1;
	}

	if(Keys[KB_END])
	{
		engine.AngleX-=2048;
		engine.AngleX-=2048;
		engine.AngleX-=2048;
		engine.AngleX-=2048;
		engine.AngleX=((engine.AngleX+(2048<<8))&((2048<<8)-1));
//		if(engine.AngleX< (1536<<8) && (engine.AngleX>512<<8))
//			engine.AngleX=1536<<8;
		update	=	1;
	}
	return(update);
}

ULONG	engine_keys_zoom(void)
{
	if(Keys[KB_I])
	{
		engine.Scale+=ShiftFlag?4:64;
		return(1);
	}
	if(Keys[KB_O])
	{
		engine.Scale-=ShiftFlag?4:64;
		if(engine.Scale<1)
			engine.Scale=1;
		return(1);
	}
	return(0);
}

ULONG	editor_user_interface(UBYTE type)
{
	ULONG	update	=	0;
	static	ULONG	res=1;
	static	UWORD	current_texture_l=0;
	static	UWORD	current_texture_r=0;
	CBYTE	str[100];
/*	
	if(Keys[KB_R]&&!ControlFlag)
	{
		Keys[KB_R]=0;
loop:
		res++;
		if(display_types[res].Width==0)
			res=0;
		if(SetDisplay(display_types[res].Width,display_types[res].Height,display_types[res].Depth)!=NoError)
			goto	loop;
		return(1);
	}
*/
	switch(type)
	{
		case	0:
				update|=engine_keys_scroll();
				update|=engine_keys_spin();
				update|=engine_keys_zoom();
				break;

		case	1:
				update|=engine_keys_scroll_game();
				update|=engine_keys_spin();
				update|=engine_keys_zoom();
				break;
		case	2:
				update|=engine_keys_scroll_plan();
				update|=engine_keys_spin();
//				update|=engine_keys_zoom();
				break;

		
	}
	return	update;
}

void	draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col)
{
	struct	SVector	p1,p2;
	struct	SVector	res1,res2;
	SLONG	temp;
	SLONG	f1,f2;


	temp=engine.ClipFlag;
	engine.ClipFlag=0;

	p1.X=x1;
	p1.Y=y1;
	p1.Z=z1;

	p2.X=x2;
	p2.Y=y2;
	p2.Z=z2;
	
	f1=rotate_point_gte(&p1,&res1);
	f2=rotate_point_gte(&p2,&res2);
	if(!( (f1&f2) & EF_CLIPFLAGS))
		DrawLineC(res1.X,res1.Y,res2.X,res2.Y,col);
	engine.ClipFlag=temp;
}

void	draw_3d_text(SLONG x1,SLONG y1,SLONG z1,CBYTE *str,UBYTE col)
{
	struct	SVector	p1;
	struct	SVector	res1,res2;

	p1.X=x1;
	p1.Y=y1;
	p1.Z=z1;

	rotate_point_gte(&p1,&res1);
	QuickTextC(res1.X,res1.Y,str,col);
}

void	create_bucket_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col)
{
	struct	SVector	points;
	struct	SVector	res;
	SLONG	sx,sy,sz;
	SLONG	c0;
	SLONG	prx,pry,prz;
	sx=x2-x1;
	sy=y2-y1;
	sz=z2-z1;

	sx/=32;
	sy/=32;
	sz/=32;

	points.X=x1;
	points.Y=y1;
	points.Z=z1;
	rotate_point_gte(&points,&res);
	prx=res.X;
	pry=res.Y;
	prz=res.Z;
	for(c0=0;c0<32;c0++)
	{
		points.X+=sx;
		points.Y+=sy;
		points.Z+=sz;
		rotate_point_gte(&points,&res);
		insert_bucket_vect(prx,pry,prz,res.X,res.Y,res.Z,col);
		prx=res.X;
		pry=res.Y;
		prz=res.Z;
	}
}


void	create_bucket_3d_line_whole(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col)
{
	struct	SVector	point[2];
	struct	SVector	res[2];
	//SLONG	sx,sy,sz,c0;
	ULONG	f1,f2;
	point[0].X=x1;
	point[0].Y=y1;
	point[0].Z=z1;
	f1=rotate_point_gte(&point[0],&res[0]);

	point[1].X=x2;
	point[1].Y=y2;
	point[1].Z=z2;
	f2=rotate_point_gte(&point[1],&res[1]);
	f1=f1&f2;
	if(!(f1 & EF_CLIPFLAGS))
//		insert_bucket_vect(res[0].X,res[0].Y,res[0].Z,res[1].X,res[1].Y,res[1].Z,col);
		insert_bucket_vect(res[0].X,res[0].Y,-5000,res[1].X,res[1].Y,-5000,col);
}


void	draw_grid2(void)
{
	
	SLONG	x,y;
	SLONG	left,right,top,bottom;
	SLONG	col=LOLITE_COL;
	SLONG	z;
	SLONG	numb=32;
/*
	z=(engine.Z>>8)+5000;
  	left=((engine.X>>8)-numb*HALF_ELE_SIZE)&(~(HALF_ELE_SIZE-1));
	right=((engine.X>>8)+numb*HALF_ELE_SIZE)&(~(HALF_ELE_SIZE-1));
	top=((engine.Y>>8)-numb*HALF_ELE_SIZE)&(~(HALF_ELE_SIZE-1));
	bottom=((engine.Y>>8)+numb*HALF_ELE_SIZE)&(~(HALF_ELE_SIZE-1));

	for (x=left;x<right ;x+=HALF_ELE_SIZE )
	{
		create_bucket_3d_line_whole(x,top,z,x,bottom,z,col);
	}
	for (y=top;y<bottom ;y+=HALF_ELE_SIZE )
	{
		create_bucket_3d_line_whole(left,y,z,right,y,z,col);
	}
*/
	z=10000;
  	left=0;
	right=WorkWindowWidth;
	top=0;
	bottom=WorkWindowHeight;

	for (x=left;x<right ;x+=64 )
	{
  		insert_bucket_vect(x,top,z,x,bottom,z,col);
	}
	for (y=top;y<bottom ;y+=64 )
	{
	  	insert_bucket_vect(left,y,z,right,y,z,col);
	}
}

void	draw_cube_hilight(void)
{
	SLONG	x,y,z;

	x=(engine.X>>8);
	y=(engine.Y>>8);
	z=(engine.Z>>8);

	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);

	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);

	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,WHITE_COL);
	
}

void	draw_editor_grid(void)
{
	struct	SVector	points[8];
	struct	SVector	res[8];
	SLONG	c0;
	SLONG	x,y,z;
	if(edit_info.GridOn)
	{
		draw_grid2();
		return;
	}


	x=(engine.X>>8);
	y=(engine.Y>>8);
	z=(engine.Z>>8);

	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE*8,z-HALF_ELE_SIZE,x-HALF_ELE_SIZE,y+HALF_ELE_SIZE*8,z-HALF_ELE_SIZE,1);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y-HALF_ELE_SIZE*8,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE*8,z-HALF_ELE_SIZE,1);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y-HALF_ELE_SIZE*8,z+HALF_ELE_SIZE,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE*8,z+HALF_ELE_SIZE,1);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE*8,z+HALF_ELE_SIZE,x-HALF_ELE_SIZE,y+HALF_ELE_SIZE*8,z+HALF_ELE_SIZE,1);

	create_bucket_3d_line(x-HALF_ELE_SIZE*8,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE*8,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE,1);
	create_bucket_3d_line(x-HALF_ELE_SIZE*8,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,x+HALF_ELE_SIZE*8,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE,1);
	create_bucket_3d_line(x-HALF_ELE_SIZE*8,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,x+HALF_ELE_SIZE*8,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE,1);
	create_bucket_3d_line(x-HALF_ELE_SIZE*8,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,x+HALF_ELE_SIZE*8,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE,1);

	create_bucket_3d_line(x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE*8,x-HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE*8,1);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z-HALF_ELE_SIZE*8,x+HALF_ELE_SIZE,y-HALF_ELE_SIZE,z+HALF_ELE_SIZE*8,1);
	create_bucket_3d_line(x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE*8,x+HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE*8,1);
	create_bucket_3d_line(x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z-HALF_ELE_SIZE*8,x-HALF_ELE_SIZE,y+HALF_ELE_SIZE,z+HALF_ELE_SIZE*8,1);

#ifdef	POO	
	points[0].X=x-HALF_ELE_SIZE;
	points[0].Y=y-HALF_ELE_SIZE*8;
	points[0].Z=z-HALF_ELE_SIZE;

	points[1].X=x+HALF_ELE_SIZE;
	points[1].Y=y-HALF_ELE_SIZE*8;
	points[1].Z=z-HALF_ELE_SIZE;
	
	points[2].X=x+HALF_ELE_SIZE;
	points[2].Y=y+HALF_ELE_SIZE*8;
	points[2].Z=z-HALF_ELE_SIZE;

	points[3].X=x-HALF_ELE_SIZE;
	points[3].Y=y+HALF_ELE_SIZE*8;
	points[3].Z=z-HALF_ELE_SIZE;

	points[4].X=x-HALF_ELE_SIZE;
	points[4].Y=y-HALF_ELE_SIZE*8;
	points[4].Z=z+HALF_ELE_SIZE;

	points[5].X=x+HALF_ELE_SIZE;
	points[5].Y=y-HALF_ELE_SIZE*8;
	points[5].Z=z+HALF_ELE_SIZE;

	points[6].X=x+HALF_ELE_SIZE;
	points[6].Y=y+HALF_ELE_SIZE*8;
	points[6].Z=z+HALF_ELE_SIZE;

	points[7].X=x-HALF_ELE_SIZE;
	points[7].Y=y+HALF_ELE_SIZE*8;
	points[7].Z=z+HALF_ELE_SIZE;

	for(c0=0;c0<8;c0++)
	{
		//transform all points for this Object
		rotate_point_gte(&points[c0],&res[c0]);
	}
	DrawLineC(res[0].X,res[0].Y,res[3].X,res[3].Y,1);
	DrawLineC(res[1].X,res[1].Y,res[2].X,res[2].Y,1);
	DrawLineC(res[5].X,res[5].Y,res[6].X,res[6].Y,1);
	DrawLineC(res[4].X,res[4].Y,res[7].X,res[7].Y,1);

	points[0].X=x-HALF_ELE_SIZE*8;
	points[0].Y=y-HALF_ELE_SIZE;
	points[0].Z=z-HALF_ELE_SIZE;

	points[1].X=x+HALF_ELE_SIZE*8;
	points[1].Y=y-HALF_ELE_SIZE;
	points[1].Z=z-HALF_ELE_SIZE;
	
	points[2].X=x+HALF_ELE_SIZE*8;
	points[2].Y=y+HALF_ELE_SIZE;
	points[2].Z=z-HALF_ELE_SIZE;

	points[3].X=x-HALF_ELE_SIZE*8;
	points[3].Y=y+HALF_ELE_SIZE;
	points[3].Z=z-HALF_ELE_SIZE;

	points[4].X=x-HALF_ELE_SIZE*8;
	points[4].Y=y-HALF_ELE_SIZE;
	points[4].Z=z+HALF_ELE_SIZE;

	points[5].X=x+HALF_ELE_SIZE*8;
	points[5].Y=y-HALF_ELE_SIZE;
	points[5].Z=z+HALF_ELE_SIZE;

	points[6].X=x+HALF_ELE_SIZE*8;
	points[6].Y=y+HALF_ELE_SIZE;
	points[6].Z=z+HALF_ELE_SIZE;

	points[7].X=x-HALF_ELE_SIZE*8;
	points[7].Y=y+HALF_ELE_SIZE;
	points[7].Z=z+HALF_ELE_SIZE;

	for(c0=0;c0<8;c0++)
	{
		//transform all points for this Object
		rotate_point_gte(&points[c0],&res[c0]);
	}
	DrawLineC(res[0].X,res[0].Y,res[1].X,res[1].Y,1);
	DrawLineC(res[3].X,res[3].Y,res[2].X,res[2].Y,1);
	DrawLineC(res[4].X,res[4].Y,res[5].X,res[5].Y,1);
	DrawLineC(res[7].X,res[7].Y,res[6].X,res[6].Y,1);

	points[0].X=x-HALF_ELE_SIZE;
	points[0].Y=y-HALF_ELE_SIZE;
	points[0].Z=z-HALF_ELE_SIZE*8;

	points[1].X=x+HALF_ELE_SIZE;
	points[1].Y=y-HALF_ELE_SIZE;
	points[1].Z=z-HALF_ELE_SIZE*8;
	
	points[2].X=x+HALF_ELE_SIZE;
	points[2].Y=y+HALF_ELE_SIZE;
	points[2].Z=z-HALF_ELE_SIZE*8;
							   
	points[3].X=x-HALF_ELE_SIZE;
	points[3].Y=y+HALF_ELE_SIZE;
	points[3].Z=z-HALF_ELE_SIZE*8;

	points[4].X=x-HALF_ELE_SIZE;
	points[4].Y=y-HALF_ELE_SIZE;
	points[4].Z=z+HALF_ELE_SIZE*8;

	points[5].X=x+HALF_ELE_SIZE;
	points[5].Y=y-HALF_ELE_SIZE;
	points[5].Z=z+HALF_ELE_SIZE*8;

	points[6].X=x+HALF_ELE_SIZE;
	points[6].Y=y+HALF_ELE_SIZE;
	points[6].Z=z+HALF_ELE_SIZE*8;

	points[7].X=x-HALF_ELE_SIZE;
	points[7].Y=y+HALF_ELE_SIZE;
	points[7].Z=z+HALF_ELE_SIZE*8;

	for(c0=0;c0<8;c0++)
	{
		//transform all points for this Object
		rotate_point_gte(&points[c0],&res[c0]);
	}
	DrawLineC(res[0].X,res[0].Y,res[4].X,res[4].Y,1);
	DrawLineC(res[1].X,res[1].Y,res[5].X,res[5].Y,1);
	DrawLineC(res[2].X,res[2].Y,res[6].X,res[6].Y,1);
	DrawLineC(res[3].X,res[3].Y,res[7].X,res[7].Y,1);

#endif
}

void	set_screen_box(SLONG x,SLONG y,SLONG z, EdRect *rect,SLONG w,SLONG h)
{

	struct	SVector	p,res;
	SLONG	temp;
	temp=engine.ClipFlag;
	engine.ClipFlag=0;
	p.X=x;
	p.Y=y;
	p.Z=z;
	rotate_point_gte(&p,&res);
	rect->SetRect(res.X-w,res.Y-h,w*2,h*2);
	engine.ClipFlag=temp;
}

void	calc_things_screen_box(SLONG	map_thing,EdRect *rect)
{
	struct	MapThing	*p_mthing;

	p_mthing=TO_MTHING(map_thing);
	switch(p_mthing->Type)
	{
		case	MAP_THING_TYPE_ANIM_PRIM:
			set_screen_box(p_mthing->X,p_mthing->Y,p_mthing->Z,rect,20,20);
			break;
		case	MAP_THING_TYPE_PRIM:
			//3ds Prim Mesh 
			set_camera_angledy(p_mthing->AngleY);
			calc_prims_screen_box(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z,rect);
			set_camera_angledy(0);

			break;
		case	MAP_THING_TYPE_LIGHT:
			set_screen_box(p_mthing->X,p_mthing->Y,p_mthing->Z,rect,10,10);
			break;

		case	MAP_THING_TYPE_SPRITE:
		case	MAP_THING_TYPE_AGENT:
			break;

	}
}

SLONG	hilight_map_things(UWORD type)
{
	SLONG	dx,dy,dz;
	SLONG	mx,my,mz;
	UWORD	index;
	EdRect	prim_rect;
	struct	MapThing	*p_mthing;
	static	UBYTE col=250;
	SLONG	screen_change=0;
	col++;
	if(col==0)
		col=250;

	
	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

	for(dz=-28;dz<28;dz++)
	for(dx=-28;dx<28;dx++)
	{
//		if(dx+mx>0&&dx+mx<EDIT_MAP_WIDTH&&dz+mz>0&&dz+mz<EDIT_MAP_DEPTH)
		if(on_edit_map(dx+mx,dz+mz))
		{
			
			index=edit_map[(dx+mx)][(dz+mz)].MapThingIndex;
			while(index)
			{
				if(map_things[index].Type==type)
				{							   
					calc_things_screen_box(index,&prim_rect);
					prim_rect.OutlineRect(col);
					{
						CBYTE	str[100];
						sprintf(str,"%d",map_things[index].IndexOther);
						QuickTextC(prim_rect.GetLeft()+1,prim_rect.GetTop(),str,0);
						QuickTextC(prim_rect.GetLeft(),prim_rect.GetTop(),str,255);
					}
					screen_change=1;
				}
				index=map_things[index].MapChild;
			}
		}
	}
	return(screen_change);
}

SLONG	hilight_map_backgrounds(UWORD type)
{
	EdRect	prim_rect;
	SWORD	index;
	struct	MapThing	*p_thing;
	SLONG	screen_change=0;
	static	UBYTE col=250;
	col++;
	if(col==0)
		col=250;

	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		calc_things_screen_box(index,&prim_rect);
		LogText(" hilight back l %d t %d w %d h %d\n",prim_rect.GetLeft(),prim_rect.GetTop(),prim_rect.GetWidth(),prim_rect.GetHeight());
		prim_rect.OutlineRect(col);
		index=p_thing->IndexNext;
		screen_change=1;
	}
	return(screen_change);
}

SLONG	select_map_backgrounds(MFPoint *mouse,UWORD type)
{
	EdRect	prim_rect;
	SWORD	index;
	static	UBYTE col=250;
	struct	MapThing	*p_thing;

	col++;
	if(col==0)
		col=250;

	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		calc_things_screen_box(index,&prim_rect);
		if(prim_rect.PointInRect(mouse))
			return(index);
		index=p_thing->IndexNext;
	}
	return(0);
}


SLONG	select_map_things(MFPoint *mouse,UWORD type)
{
	SLONG	dx,dy,dz;
	SLONG	mx,my,mz;
	UWORD	index;
	EdRect	prim_rect;
	struct	MapThing	*p_mthing;
	static	UBYTE col=0;
	SLONG	screen_change=0;
	col++;
	
	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

	for(dz=-32;dz<32;dz++)
	for(dx=-32;dx<32;dx++)
	{
//		if(dx+mx>0&&dx+mx<EDIT_MAP_WIDTH&&dz+mz>0&&dz+mz<EDIT_MAP_DEPTH)
		if(on_edit_map(dx+mx,dz+mz))
		{
			
			index=edit_map[(dx+mx)][(dz+mz)].MapThingIndex;
			while(index)
			{
				if(map_things[index].Type==type)
				{
					
					calc_things_screen_box(index,&prim_rect);
					if(prim_rect.PointInRect(mouse))
						return(index);
				}
				index=map_things[index].MapChild;
			}
		}
	}
	return(0);
}


extern struct KeyFrameChunk 	*test_chunk;
extern void	draw_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat);

void	draw_map_thing(SLONG	map_thing)
{
	SLONG				c0,c1;
	struct Matrix33		r_matrix;
	struct MapThing		*p_mthing;
	struct	GameKeyFrameChunk *the_chunk;


	p_mthing	=	TO_MTHING(map_thing);
	switch(p_mthing->Type)
	{
		case	MAP_THING_TYPE_ANIM_PRIM:
extern	void	draw_anim_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct GameKeyFrameElement *anim_info,struct GameKeyFrameElement *anim_info_next,struct Matrix33 *rot_mat);
			//			break;
			rotate_obj	(
							p_mthing->AngleX,
							p_mthing->AngleY,
							p_mthing->AngleZ,
							&r_matrix
						);
			{
				the_chunk=&anim_chunk[p_mthing->IndexOther];
				for(c1=0,c0=prim_multi_objects[the_chunk->MultiObject[0]].StartObject;c0<prim_multi_objects[the_chunk->MultiObject[0]].EndObject;c0++,c1++)
				{
					draw_anim_prim_tween	(
										c0,
										p_mthing->X,p_mthing->Y,p_mthing->Z,
										0,
										&the_chunk->AnimList[1]->FirstElement[c1],
										&the_chunk->AnimList[1]->FirstElement[c1],
										//&p_mthing->AnimElements[c1],
										//&p_mthing->NextAnimElements[c1],
										&r_matrix
									);
				}
			}

			break;
		case	MAP_THING_TYPE_PRIM:
			//3ds Prim Mesh 
//			engine.AngleDY=p_mthing->AngleY;
			set_camera_angledy(p_mthing->AngleY);

//			if(p_mthing->Flags&FLAG_EDIT_PRIM_ON_FLOOR || (prim_objects[p_mthing->IndexOther].flag & PRIM_FLAG_ON_FLOOR) )

			/*

			if((prim_objects[p_mthing->IndexOther].flag & PRIM_FLAG_ON_FLOOR) )
			{

				SLONG	px,py,pz,y;

extern	void	find_things_min_point(SLONG drag,SLONG *px,SLONG *py,SLONG *pz);

				find_things_min_point(p_mthing->IndexOther,&px,&py,&pz);

extern	SLONG find_alt_for_this_pos(SLONG  x,SLONG  z);
				y=find_alt_for_this_pos(p_mthing->X,p_mthing->Z);
//				y=calc_edit_height_at(p_mthing->X,p_mthing->Z);
				y-=py;
				p_mthing->Y=y;
			}
			else
			if(prim_objects[p_mthing->IndexOther].flag & PRIM_FLAG_JUST_FLOOR) 
			{

				SLONG	px,py,pz,y;

extern	void	find_things_min_point(SLONG drag,SLONG *px,SLONG *py,SLONG *pz);

				find_things_min_point(p_mthing->IndexOther,&px,&py,&pz);

				y=calc_edit_height_at(p_mthing->X,p_mthing->Z);
				y-=py;
				p_mthing->Y=y;
			}
			*/

			draw_a_prim_at(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z,1);
			set_camera_angledy(0);
			//engine.AngleDY=0;
			break;
		case	MAP_THING_TYPE_BUILDING:
			draw_a_building_at(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z);
			break;
		case	MAP_THING_TYPE_MULTI_PRIM:
			draw_a_multi_prim_at(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z);
			break;

		case	MAP_THING_TYPE_ROT_MULTI:
#ifdef	DOGPOO
			//			break;
			rotate_obj	(
							p_mthing->AngleX,
							p_mthing->AngleY,
							p_mthing->AngleZ,
							&r_matrix
						);
			//if(p_mthing->AnimElements&&p_mthing->NextAnimElements)
			if(p_mthing->CurrentFrame&&p_mthing->NextFrame)
			{
				for(c1=0,c0=prim_multi_objects[test_chunk->MultiObject].StartObject;c0<prim_multi_objects[test_chunk->MultiObject].EndObject;c0++,c1++)
				{
/*					if(c1==1)
					{
extern UBYTE	store_pos;
						store_pos	=	1;
					}
*/
					draw_prim_tween	(
										c0,
										p_mthing->X,p_mthing->Y,p_mthing->Z,
										p_mthing->TweenStage,
										&p_mthing->CurrentFrame->FirstElement[c1],
										&p_mthing->NextFrame->FirstElement[c1],
										//&p_mthing->AnimElements[c1],
										//&p_mthing->NextAnimElements[c1],
										&r_matrix
									);
				}
			}
#endif
			break;
		case	MAP_THING_TYPE_SPRITE:
		case	MAP_THING_TYPE_AGENT:
			break;

	}
}

extern SLONG	play_x,play_y,play_z;


void	scan_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG *mid_x,SLONG *mid_y,SLONG *mid_z)
{
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;


	p_obj    =	&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
	
	*mid_x=0;
	*mid_y=0;
	*mid_z=0;

	for(c0=sp;c0<ep;c0++)
	{
		//transform all points for this Object
		*mid_x+=prim_points[c0].X+x;
		*mid_y+=prim_points[c0].Y+y;
		*mid_z+=prim_points[c0].Z+z;
	}

	if(ep-sp)
	{
		*mid_x/=(ep-sp);
		*mid_y/=(ep-sp);
		*mid_z/=(ep-sp);
	}
}

void	scan_a_prim_at_dist(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG mid_x,SLONG mid_y,SLONG mid_z,SLONG *dist)
{
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG 	dx,dy,dz;
	SLONG	c_dist;

	*dist=-999999;
	p_obj    =	&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
	

	for(c0=sp;c0<ep;c0++)
	{
		//transform all points for this Object

		dx=abs(prim_points[c0].X+x-mid_x);
		dy=abs(prim_points[c0].Y+y-mid_y);
		dz=abs(prim_points[c0].Z+z-mid_z);

		c_dist=sqrl(SDIST3(dx,dy,dz));
		if(c_dist>*dist)
			*dist=c_dist;
	}

}


void	zoom_map_onto_screen(void)
{
	SWORD	index;
	struct	MapThing	*p_thing;
	SLONG	mid_x=0,mid_y=0,mid_z=0,mx,my,mz,count=0;
	SLONG	dist,b_dist=-999999;

	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		scan_a_prim_at(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z,&mx,&my,&mz);
		mid_x+=mx;
		mid_y+=my;
		mid_z+=mz;
		count++;
		index=p_thing->IndexNext;
	}
	if(count)
	{
		mid_x/=count;
		mid_y/=count;
		mid_z/=count;
	}
	engine.X=mid_x<<8;
	engine.Y=mid_y<<8;
	engine.Z=mid_z<<8;

	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		scan_a_prim_at_dist(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z,mid_x,mid_y,mid_z,&dist);
		if(dist>b_dist)
			b_dist=dist;
		index=p_thing->IndexNext;
	}
	// <<5*scale)>>16

//	pos=((b_dist<<5)*scale)>>16;

	engine.Scale=(200<<11)/(b_dist);
//	LogText(" zoom map dist= %d newscale %d = %d\n",b_dist,engine.Scale,((b_dist<<5)*engine.Scale)>>16);

}


void	draw_linked_background(void)
{
	SWORD	index;
	struct	MapThing	*p_thing;
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		draw_a_prim_at(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z,1);
		index=p_thing->IndexNext;
	}
}

#define	CLIPNEG(x)	if(x<0)x=0
#define	SHADOW_SIZE	48


//   1   2
//
//   3   4

SLONG	add_floor_tri_to_bucket(SLONG	x1,SLONG	y1,SLONG	z1,SLONG	x2,SLONG	y2,SLONG	z2,SLONG	x3,SLONG	y3,SLONG	z3,struct	DepthStrip	*p_map,SLONG s1,SLONG s2,SLONG s3,SLONG shadow_flag,SLONG tx1,SLONG ty1,SLONG tx2,SLONG ty2,SLONG tx3,SLONG ty3,SLONG page)
{
	SLONG	az;
	if(current_bucket_pool > end_bucket_pool)
		return(0);
	az=z1;
	setPolyType3(
					current_bucket_pool,
					POLY_GT
				);




	setXY3	(
				(struct BucketTri*)current_bucket_pool,
						x1,y1,
						x2,y2,
						x3,y3

			);

	setUV3(	(struct BucketTri*)current_bucket_pool,tx1,ty1,tx2,ty2,tx3,ty3,page);

	setZ3((struct BucketTri*)current_bucket_pool,z1,z2,z3);


	if(shadow_flag)
	{
		s1-=SHADOW_SIZE;
		s2-=SHADOW_SIZE;
		s3-=SHADOW_SIZE;
		CLIPNEG(s1);
		CLIPNEG(s2);
		CLIPNEG(s3);
	}

	setShade3((struct BucketTri*)current_bucket_pool,s1,s2,s3);
	((struct BucketTri*)current_bucket_pool)->DebugInfo=z1;
	((struct BucketTri*)current_bucket_pool)->DebugFlags=0;
	add_bucket((void *)current_bucket_pool,az+300);


	current_bucket_pool	+=	sizeof(struct BucketTri);
	return(0);

}

#define	SET_TX_TY(x1,y1,x2,y2,x3,y3,x4,y4) tx1=x1;ty1=y1;tx2=x2;ty2=y2;tx3=x3;ty3=y3;tx4=x4;ty4=y4;

SLONG	add_floor_face_to_bucket(SLONG	x1,SLONG	y1,SLONG	z1,SLONG	x2,SLONG	y2,SLONG	z2,SLONG	x3,SLONG	y3,SLONG	z3,SLONG	x4,SLONG	y4,SLONG	z4,struct	DepthStrip	*p_map,SLONG s1,SLONG s2,SLONG s3,SLONG s4,UWORD tex)
{
	SLONG	az;
	UBYTE	tx,ty,tsize,page;
	SLONG	shadow;
	SLONG	ret=0;
	UBYTE	tx1,ty1,tx2,ty2,tx3,ty3,tx4,ty4;

	az=z1;

	if(current_bucket_pool > end_bucket_pool)
		return(0);

	if(check_mouse_over_floor_quad(x1,y1,x2,y2,x3,y3,x4,y4,0,az))
	{
		ret=1;
	}

	if(p_map->Walkable==-1)
	{
		struct	AnimTmap	*p_a;
		SLONG	cur;

		p_a=&anim_tmaps[p_map->Texture];
		cur=p_a->Current;
		SET_TX_TY(p_a->UV[cur][0][0],p_a->UV[cur][0][1],p_a->UV[cur][1][0],p_a->UV[cur][1][1],p_a->UV[cur][2][0],p_a->UV[cur][2][1],p_a->UV[cur][3][0],p_a->UV[cur][3][1]);
		page=p_a->Page[cur];
		tsize=32;

	}
	else
	{

		tx=((struct	MiniTextureBits*)(&tex))->X<<5;
		ty=((struct	MiniTextureBits*)(&tex))->Y<<5;
		page=((struct	MiniTextureBits*)(&tex))->Page;
		tsize=31; //floor_texture_sizes[((struct	MiniTextureBits*)(&tex))->Size]-1;
		switch(((struct	MiniTextureBits*)(&tex))->Rot)
		{
			case	0:		
				SET_TX_TY(tx,ty,tx+tsize,ty,tx,ty+tsize,tx+tsize,ty+tsize);
				break;
			case	1:		
				SET_TX_TY(tx+tsize,ty,tx+tsize,ty+tsize,tx,ty,tx,ty+tsize);
				break;
			case	2:	
				SET_TX_TY(tx+tsize,ty+tsize,tx,ty+tsize,tx+tsize,ty,tx,ty);
				break;
			case	3:	
				SET_TX_TY(tx,ty+tsize,tx,ty,tx+tsize,ty+tsize,tx+tsize,ty);
				break;
		}
	}


	shadow=p_map->Flags&FLOOR_SHADOW_TYPE;

	if(shadow)
	{
		switch(shadow)
		{
			case	1: // all shadow
				s1-=SHADOW_SIZE;
				s2-=SHADOW_SIZE;
				s3-=SHADOW_SIZE;
				s4-=SHADOW_SIZE;
				CLIPNEG(s1);
				CLIPNEG(s2);
				CLIPNEG(s3);
				CLIPNEG(s4);
				break;
			case	2:
				//   .
				//  ..
				// ...
				add_floor_tri_to_bucket(x2,y2,z2,x4,y4,z4,x3,y3,z3,p_map,s2,s4,s3,1,tx2,ty2,tx4,ty4,tx3,ty3,page);
				add_floor_tri_to_bucket(x2,y2,z2,x3,y3,z3,x1,y1,z1,p_map,s2,s3,s1,0,tx2,ty2,tx3,ty3,tx1,ty1,page);
				return(ret);
			case	3:
				// . 
				// ..
				// ...
				add_floor_tri_to_bucket(x1,y1,z1,x4,y4,z4,x3,y3,z3,p_map,s1,s4,s3,1,tx1,ty1,tx4,ty4,tx3,ty3,page);
				add_floor_tri_to_bucket(x1,y1,z1,x2,y2,z2,x4,y4,z4,p_map,s1,s2,s4,0,tx1,ty1,tx2,ty2,tx4,ty4,page);
				return(ret);
			case	4:
				// ... 
				// ..
				// .
				add_floor_tri_to_bucket(x1,y1,z1,x2,y2,z2,x3,y3,z3,p_map,s1,s2,s3,1,tx1,ty1,tx2,ty2,tx3,ty3,page);
				add_floor_tri_to_bucket(x2,y2,z2,x4,y4,z4,x3,y3,z3,p_map,s2,s4,s3,0,tx2,ty2,tx4,ty4,tx3,ty3,page);
				return(ret);
			case	5:
				// ... 
				//  ..
				//   .
				add_floor_tri_to_bucket(x1,y1,z1,x2,y2,z2,x4,y4,z4,p_map,s1,s2,s4,1,tx1,ty1,tx2,ty2,tx4,ty4,page);
				add_floor_tri_to_bucket(x1,y1,z1,x4,y4,z4,x3,y3,z3,p_map,s1,s4,s3,0,tx1,ty1,tx4,ty4,tx3,ty3,page);
				return(ret);
		}
	}
	
	
	setPolyType4(
					current_bucket_pool,
					POLY_GT
				);


	setCol4	(
				(struct BucketQuad*)current_bucket_pool,
				1
			);


	setXY4	(
				(struct BucketQuad*)current_bucket_pool,
						x1,y1,
						x2,y2,
						x3,y3,
						x4,y4

			);

//	if(SelectFlag)
//		do_quad_clip_list(c0,p0,p1,p2,p3);
	setUV4(	(struct BucketQuad*)current_bucket_pool,tx1,ty1,tx2,ty2,tx3,ty3,tx4,ty4,page);
	//	tx,ty,tx+tsize,ty,tx,ty+tsize,tx+tsize,ty+tsize,page);

	setZ4((struct BucketQuad*)current_bucket_pool,z1,z2,z3,z4);

	

//			setShade4((struct BucketQuad*)current_bucket_pool,p_f4->Bright[0],p_f4->Bright[1],p_f4->Bright[2],p_f4->Bright[3]);
	setShade4((struct BucketQuad*)current_bucket_pool,s1,s2,s3,s4);
	((struct BucketQuad*)current_bucket_pool)->DebugInfo=z1;
	((struct BucketQuad*)current_bucket_pool)->DebugFlags=0;
	add_bucket((void *)current_bucket_pool,az+300);




/*
	if(check_mouse_over_prim_quad(global_res,p0,p1,p2,p3,c0))
	{
		selected_prim_xyz.X	=	x;
		selected_prim_xyz.Y	=	y;
		selected_prim_xyz.Z	=	z;
	}
*/
	current_bucket_pool	+=	sizeof(struct BucketQuad);
	return(ret);
}

#define SWAP_ROW() 					\
{									\
	struct SVector *temp_ptr_sv;	\
	ULONG *temp_ptr_flag;			\
	temp_ptr_sv=prev_row_ptr;		\
	prev_row_ptr=row_ptr;			\
	row_ptr=temp_ptr_sv;			\
	temp_ptr_flag=prev_row_flag_ptr;	\
	prev_row_flag_ptr=row_flag_ptr;		\
	row_flag_ptr=temp_ptr_flag;		\
}	


#define	DRAW_WIDTH	24
void	draw_map_floor(void)
{
	SLONG	dx,dz;
	SLONG	mx,my,mz;
	struct	SVector	point,row[2][66];
	ULONG	row_flags[2][66];
	struct	SVector	*row_ptr;
	ULONG	*row_flag_ptr;
	struct	SVector	*prev_row_ptr;
	ULONG	*prev_row_flag_ptr;
	SLONG	row_count;
	UWORD	texture;

//	LogText(" ddraw mmaap floorr  \n");


	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;


	row_ptr=&row[0][0];
	row_flag_ptr=&row_flags[0][0];

	for(row_count=0,dx=-DRAW_WIDTH,dz=-DRAW_WIDTH;dx<DRAW_WIDTH;dx++,row_count++)
	{
		point.X=dx*ELE_SIZE+(mx<<ELE_SHIFT);
		point.Y=edit_map[mx+dx][mz+dz].Y<<FLOOR_HEIGHT_SHIFT;
		point.Z=(dz*ELE_SIZE)+(mz<<ELE_SHIFT); //(engine.Z>>8);

		row_flag_ptr[row_count]=rotate_point_gte(&point,&row_ptr[row_count]);
	}

	prev_row_ptr=row_ptr;
	prev_row_flag_ptr=row_flag_ptr;

	row_ptr=&row[1][0];
	row_flag_ptr=&row_flags[1][0];

	for(dz=(-DRAW_WIDTH)+1;dz<DRAW_WIDTH;dz++)
	{
	
//		dz=(-DRAW_WIDTH)+1;
		for(row_count=0,dx=-DRAW_WIDTH;dx<DRAW_WIDTH;dx++,row_count++)
		{
			SLONG	flag_and,flag_or;
			SLONG	index;
			if(row_count==0)
			{
				point.X=(dx*ELE_SIZE)+(mx<<ELE_SHIFT);
				point.Y=edit_map[mx+dx][mz+dz].Y<<FLOOR_HEIGHT_SHIFT;
		//		point.Y=0;
				point.Z=(dz*ELE_SIZE)+(mz<<ELE_SHIFT); //(engine.Z>>8);
				*row_flag_ptr=rotate_point_gte(&point,row_ptr);
			}						  
			else
	//		if(dx+mx-1>0&&dx+mx+1<EDIT_MAP_WIDTH&&dz+mz-1>0&&dz+mz+1<EDIT_MAP_DEPTH)
			{
	//			LogText(" draw map floor dx %d dz %d row_count %d \n",dx,dz,row_count);
				point.X=(dx*ELE_SIZE)+(mx<<ELE_SHIFT);
				point.Y=edit_map[mx+dx][mz+dz].Y<<FLOOR_HEIGHT_SHIFT;
				//point.Y=0;
				point.Z=(dz*ELE_SIZE)+(mz<<ELE_SHIFT); //(engine.Z>>8);
				row_flag_ptr[row_count]=rotate_point_gte(&point,&row_ptr[row_count]);
				
//				if(dx+mx-1>0&&dx+mx+1<EDIT_MAP_WIDTH&&dz+mz-1>0&&dz+mz+1<EDIT_MAP_DEPTH)

				if( on_edit_map(dx+mx-1,dz+mz-1) )
				if( (!(edit_map[(dx+mx-1)][(dz+mz-1)].Flags&FLOOR_HIDDEN) ) ||edit_info.NoHidden==0)
				{
					flag_and=row_flag_ptr[row_count]&row_flag_ptr[row_count-1]&prev_row_flag_ptr[row_count]&prev_row_flag_ptr[row_count-1];
					flag_or=row_flag_ptr[row_count]|row_flag_ptr[row_count-1]|prev_row_flag_ptr[row_count]|prev_row_flag_ptr[row_count-1];

					if( ((flag_or&EF_BEHIND_YOU)==0) && !(flag_and & EF_CLIPFLAGS) && (flag_and & EF_TRANSLATED))
					{
						SLONG	x1,x2,x3,x4;
						SLONG	y1,y2,y3,y4;
						SLONG	z1;


						x2=prev_row_ptr[row_count-1].X;
						y2=prev_row_ptr[row_count-1].Y;

						x1=prev_row_ptr[row_count].X;
						y1=prev_row_ptr[row_count].Y;

						x4=row_ptr[row_count-1].X;
						y4=row_ptr[row_count-1].Y;

						x3=row_ptr[row_count].X;
						y3=row_ptr[row_count].Y;

						z1=prev_row_ptr[row_count-1].Z;
	//					LogText(" ddraw floorr ttile %d %d \n",dx,dz);
						if(edit_info.RoofTex)
						{
							texture=tex_map[dx+mx-1][dz+mz-1];
						}
						else
						{
							texture=edit_map[dx+mx-1][dz+mz-1].Texture;
						}
						if(add_floor_face_to_bucket(x2,y2,z1,x1,y1,0,x4,y4,0,x3,y3,0,&edit_map[dx+mx-1][dz+mz-1],edit_map[dx+mx-1][dz+mz-1].Bright,edit_map[dx+mx][dz+mz-1].Bright,edit_map[dx+mx-1][dz+mz].Bright,edit_map[dx+mx][dz+mz].Bright,texture))
						{

							selected_prim_xyz.X	=	dx+mx-1;
							selected_prim_xyz.Y	=	0;
							selected_prim_xyz.Z	=	dz+mz-1;

							hilited_face.MapX=dx+mx-1;
							hilited_face.MapY=0; //dx+mx-1;
							hilited_face.MapZ=dz+mz-1;
						}
	//					DrawLineC(x1,y1,x2,y2,255);
	//					DrawLineC(x1,y1,x3,y3,255);

					}
				}
			}
			
		}
		SWAP_ROW()
	}
	
}

void	find_map_clip(SLONG *minx,SLONG *maxx,SLONG *minz,SLONG *maxz)
{
	SLONG	dx,dy,dz;
	SLONG	mx,my,mz;

	SLONG	min_x=9999999,max_x=-9999999;
	SLONG	min_z=9999999,max_z=-9999999;

	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;
	for(dx=-32;dx<=32;dx+=32)
	for(dz=-32;dz<32;dz++)
	{
		struct	SVector	point,res;
		SLONG	x,z;
		ULONG	flags;

		x=dx*ELE_SIZE+(mx<<ELE_SHIFT);
		z=(dz*ELE_SIZE)+(mz<<ELE_SHIFT);

		point.X=x; //dx*ELE_SIZE+(mx<<ELE_SHIFT);
		point.Y=edit_map[mx+dx][mz+dz].Y<<FLOOR_HEIGHT_SHIFT;
		point.Z=z; //(dz*ELE_SIZE)+(mz<<ELE_SHIFT);

		flags=rotate_point_gte(&point,&res);


		if( flags&(EF_BEHIND_YOU|EF_CLIPFLAGS))
		{

		}
		else
		if(flags & EF_TRANSLATED)
		{
			if(z>max_z)
			{
				max_z=z;
			}
			if(z<min_z)
			{
				min_z=z;
			}

		}
	}
	dz=0;
	for(dz=-32;dz<=32;dz+=32)
	for(dx=-32;dx<32;dx++)
	{
		struct	SVector	point,res;
		SLONG	x,z;
		ULONG	flags;

		x=dx*ELE_SIZE+(mx<<ELE_SHIFT);
		z=(dz*ELE_SIZE)+(mz<<ELE_SHIFT);

		point.X=x; //dx*ELE_SIZE+(mx<<ELE_SHIFT);
		point.Y=edit_map[mx+dx][mz+dz].Y<<FLOOR_HEIGHT_SHIFT;
		point.Z=z; //(dz*ELE_SIZE)+(mz<<ELE_SHIFT);

		flags=rotate_point_gte(&point,&res);


		if( flags&(EF_BEHIND_YOU|EF_CLIPFLAGS))
		{

		}
		else
		if(flags & EF_TRANSLATED)
		{
			if(x>max_x)
			{
				max_x=x;
			}
			if(x<min_x)
			{
				min_x=x;
			}
		}
	}

	min_x-=512;
	max_x+=512;

	min_z-=512;
	max_z+=512;


	if(min_x<0)
		min_x=0;

	if(min_z<0)
		min_z=0;

	if(max_x>=EDIT_MAP_WIDTH<<8)
		max_x=(EDIT_MAP_WIDTH<<8)-1;

	if(max_z>=EDIT_MAP_WIDTH<<8)
		max_z=(EDIT_MAP_WIDTH<<8)-1;

	*minx=min_x;
	*maxx=max_x;

	*minz=min_z;
	*maxz=max_z;
}

void	draw_editor_map(ULONG flags)
{
	SLONG	dx,dy,dz;
	SLONG	mx,my,mz;
	struct	EditMapElement	*p_ele;
	UWORD	index;

	animate_texture_maps();
//	LogText(" draw editor \n");
	
	engine.TrueY=engine.Y;
	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;
//#ifdef	POO
	draw_map_floor();


	for(dz=-32;dz<32;dz++)
	for(dx=-32;dx<32;dx++)
	{
//		if(dx+mx>0&&dx+mx<EDIT_MAP_WIDTH&&dz+mz>0&&dz+mz<EDIT_MAP_DEPTH)
		if(on_edit_map(dx+mx,dz+mz))
		{
			
			index=edit_map[(dx+mx)][(dz+mz)].MapThingIndex;
			while(index)
			{
//				LogText(" draw index %d at dx %d dz %d (%d,%d)\n",index,dx,dz,mx+dx,mz+dz);
				draw_map_thing(index);
				index=map_things[index].MapChild;
			}
		}

//		if(!flags)
/*
		{
			index=edit_map[(dx+mx)][(dz+mz)].ColVectHead;
			if(index)
				draw_col_vects(index);
		}
*/

/*
		for(dz=-12;dz<12;dz++)
		if( dz+mz>=0 && dz+mz<EDIT_MAP_DEPTH )
		{
			index=edit_map[(dx+mx)][(dy+my)].Depth[(dz+mz)];
//			draw_map_thing(index);
			if(index)
			{
				p_ele=&edit_map_eles[index];
				switch(p_ele->CubeType.Prim)
				{
					case	0:
						//error
						break;
					case	CUBE_TYPE_FULL:
						draw_cube_ele_at((dx+mx)<<ELE_SHIFT,(dy+my)<<ELE_SHIFT,(dz+mz)<<ELE_SHIFT,p_ele);
						break;
					case	CUBE_TYPE_SLOPE_LR:
						draw_slope_lr_ele_at((dx+mx)<<ELE_SHIFT,(dy+my)<<ELE_SHIFT,(dz+mz)<<ELE_SHIFT,p_ele);
						break;
					case	CUBE_TYPE_STEPS_LR:
						draw_steps_lr_ele_at((dx+mx)<<ELE_SHIFT,(dy+my)<<ELE_SHIFT,(dz+mz)<<ELE_SHIFT,p_ele);
						break;
					case	CUBE_TYPE_LEDGE1:
						draw_ledge1_ele_at((dx+mx)<<ELE_SHIFT,(dy+my)<<ELE_SHIFT,(dz+mz)<<ELE_SHIFT,p_ele);
						break;
				}
			}
		}
*/

	}

//	if(background_prim)
//		draw_linked_background();

//	animate_and_draw_chap();

//	if(!flags)
//		draw_editor_grid();
//	draw_cube_hilight();		
	{
		SLONG	x,y,z;
		CBYTE	str[100];

		x=(engine.X>>8)>>ELE_SHIFT;
		y=(engine.Y>>8)>>ELE_SHIFT;
		z=(engine.Z>>8)>>ELE_SHIFT;

		sprintf(str," x %d y %d z %d ",x,y,z);
		QuickText(20,20,str,0);
		
	}
void	process_map();
		process_map();


//#endif
//	test_poly();
//	editor_user_interface();
/*
	if(editor.TexturePriority)
		render_view(1);
	if(editor.TexturePriority)
		show_texture(0,0,0,0);
*/
	if(SelectFlag<5)
		SelectFlag=0;
}


/* --------------------------- 3DSRDR.C -------------------------------
    .3DS file format exerciser v1.2.
    Written by Javier Arevalo, AKA Jare/Iguana.
    I compile this with Watcom/32, but I guess it should work with
        any compiler and OS combination for which the typedefs are
        valid i.e. any that I know for PCs... Try it and see.
        Oh, and also check the #pragma pack() thing.

    - DISCLAIMER -

    I hope I have not broken any patents or trade secrets by releasing
        this info. This is purely a mind exercise to break into a file
        format that is quite useful to know. As far as I have been told
        a file format is not subject to anything such as copyright or
        patent, so I have done this because I believe I'm allowed to.

    I PLACE THIS FILE IN THE PUBLIC DOMAIN, SO EVERYTHING CONTAINED HERE
        IS TOTALLY FREE FOR YOU TO EXPLORE AND USE. I DISCLAIM ANY AND ALL
        EVENTS COMING OUT OF ANY POSSIBLE USE (OR LACK OF USE) OR EXISTANCE
        OF THIS FILE. I WON'T BE LIABLE FOR ANYTHING RELATED TO THIS FILE,
        OR ANY PRIOR OR FUTURE VERSION OF IT.

    All trademarks mentioned are property of their respective holders.

    - Merits -

    Heavily based on info on the file 3DS_08.TXT by Jim Pitts
      (jp5@ukc.ac.uk)

    Basic material-related stuff digged up by Jare.
    Track info stuff too.

    Thanks to the Egerter brothers of WGT fame and to Walken/Impact studios
        for extra info, Rex Deathstar for support. And definitely to
        Xanthome/Darkzone for you know why. And of course, respect to
        Avatar/Legend Design for being here before all of us.

    For a cool example of actual reading of 3DS files, look no
        further than 3DSCO20.ZIP by Mats Byggmastar aka. MRI. I
        personally prefer using a table-driven modification of this
        code, but both approaches are quite ok and his is much faster
        to write and follow.

    Now only lack is someone to explain how to make use of all this
        stuff i.e. how exactly is data stored, how spline interpolations
        are performed, what are those things called quaternions, etc. And
        also, maybe, dig the rest of the chunks until we are actually able
        to write 3DS files instead of just being bored reading. There's
        lots to do.

    If you decide to work on this further, please make your findings
        public like we have already done, ok? Upload it to
        x2ftp.oulu.fi, THE place for programming info, and/or to
        ftp.cdrom.com. But please PUBLISH it!

    - Change log -

    V 1.2:
        - Added change log to have some idea what's going on.
        - Added pivot point reading inside tracks stuff.
        - Info about spline flags on keyframes.
        - Added face edge visibility info.
        - Finally!! Those flags that mark when the texture is wrapping
          around inside a face. This happens when you apply spherical
          or cylindrical coordinates, the faces along the 0§ axis don't
          get proper mapping coords. Someone describe how to fix this?
        - Added -quiet parm, only displays minimal chunk info.
        - Object parent number is stored in CHUNK_TRACKOBJNAME.
          This makes reference to the node number in CHUNK_OBJNUMBER.
        - Object number changed to unsigned. Parent 65535 means none.
        - Added CHUNK_PRJ and CHUNK_MLI to allow inspecting .PRJ and
          .MLI files (they're basically the same chunks as .3DS).
        - Added banner to identify myself, and disclaimer for "just in
          case" possibilities.
        - Corrected possible bug when chunklen == 0 (it was not a
          chunk).
        - Added several name descriptions of chunks. Use diff to find
          all the new chunks.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef PI
#define PI 3.141592687
#endif

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

typedef signed char  sbyte;
typedef signed short sword;
typedef signed long  sdword;

#pragma pack(2)

#define	printf LogText

typedef struct {
    word    id;
    dword   len;
} TChunkHeader, *PChunkHeader;

#pragma pack()

enum {
    CHUNK_RGBF      = 0x0010,
    CHUNK_RGBB      = 0x0011,
//    CHUNK_RBGB2     = 0x0012,       // ?? NOT HLS.

    CHUNK_PRJ       = 0xC23D,
    CHUNK_MLI       = 0x3DAA,

    CHUNK_MAIN      = 0x4D4D,
        CHUNK_OBJMESH   = 0x3D3D,
            CHUNK_BKGCOLOR  = 0x1200,
            CHUNK_AMBCOLOR  = 0x2100,
            CHUNK_OBJBLOCK  = 0x4000,
                CHUNK_TRIMESH   = 0x4100,
                    CHUNK_VERTLIST  = 0x4110,
                    CHUNK_FACELIST  = 0x4120,
                    CHUNK_FACEMAT   = 0x4130,
                    CHUNK_MAPLIST   = 0x4140,
                    CHUNK_SMOOLIST  = 0x4150,
                    CHUNK_TRMATRIX  = 0x4160,
                CHUNK_LIGHT     = 0x4600,
                    CHUNK_SPOTLIGHT = 0x4610,
                CHUNK_CAMERA    = 0x4700,
                CHUNK_HIERARCHY = 0x4F00,
        CHUNK_VIEWPORT  = 0x7001,
        CHUNK_MATERIAL  = 0xAFFF,
            CHUNK_MATNAME   = 0xA000,
            CHUNK_AMBIENT   = 0xA010,
            CHUNK_DIFFUSE   = 0xA020,
            CHUNK_SPECULAR  = 0xA030,
            CHUNK_TEXTURE   = 0xA200,
            CHUNK_BUMPMAP   = 0xA230,
            CHUNK_MAPFILE   = 0xA300,
        CHUNK_KEYFRAMER = 0xB000,
            CHUNK_AMBIENTKEY    = 0xB001,
            CHUNK_TRACKINFO = 0xB002,
                CHUNK_TRACKOBJNAME  = 0xB010,
                CHUNK_TRACKPIVOT    = 0xB013,
                CHUNK_TRACKPOS      = 0xB020,
                CHUNK_TRACKROTATE   = 0xB021,
                CHUNK_TRACKSCALE    = 0xB022,
                CHUNK_OBJNUMBER     = 0xB030,
            CHUNK_TRACKCAMERA = 0xB003,
                CHUNK_TRACKFOV  = 0xB023,
                CHUNK_TRACKROLL = 0xB024,
            CHUNK_TRACKCAMTGT = 0xB004,
            CHUNK_TRACKLIGHT  = 0xB005,
            CHUNK_TRACKLIGTGT = 0xB006,
            CHUNK_TRACKSPOTL  = 0xB007,
            CHUNK_FRAMES    = 0xB008,


};

// ------------------------------------

    // Forward declaration.
void ChunkReader(FILE *f, int ind, long p);

void SkipReader(FILE *f, int ind, long p) 
{
}

void RGBFReader (FILE *f, int ind, long p) {
    float c[3];
    if (fread(&c, sizeof(c), 1, f) != 1) return;
    printf("%*s    Red: %f, Green: %f, Blue: %f\n", ind, "", c[0], c[1], c[2]);
}

void RGBBReader (FILE *f, int ind, long p) {
    byte c[3];
    if (fread(&c, sizeof(c), 1, f) != 1) return;
    printf("%*s    Red: %d, Green: %d, Blue: %d\n", ind, "", c[0], c[1], c[2]);
}

void ASCIIZReader (FILE *f, int ind, long p) {
    int c;

        // Read ASCIIZ name
    while ( (c = fgetc(f)) != EOF && c != '\0')
        putchar(c);
    printf("\"\n");
}

void ObjBlockReader (FILE *f, int ind, long p) {
    int c;

        // Read ASCIIZ object name
    printf("%*sObject name \"", ind, "");
    ASCIIZReader(f, ind, p);
        // Read rest of chunks inside this one.
    ChunkReader(f, ind, p);
}

void VertListReader (FILE *f, int ind, long p) {
    word nv;
    float c[3];

    if (fread(&nv, sizeof(nv), 1, f) != 1) return;
    printf("%*sVertices: %d\n", ind, "", nv);
    while (nv-- > 0) {
        if (fread(&c, sizeof(c), 1, f) != 1) return;
        printf("%*s    X: %f, Y: %f, Z: %f\n", ind, "", c[0], c[1], c[2]);
    }
}

void FaceListReader (FILE *f, int ind, long p) {
    word nv;
    word c[3];
    word flags;

    if (fread(&nv, sizeof(nv), 1, f) != 1) return;
    printf("%*sFaces: %d\n", ind, "", nv);
    while (nv-- > 0) {
        if (fread(&c, sizeof(c), 1, f) != 1) return;
        if (fread(&flags, sizeof(flags), 1, f) != 1) return;
        printf("%*s  A %d, B %d, C %d, 0x%X:",
               ind, "", c[0], c[1], c[2], flags);
//        printf("%*s    AB: %d, BC: %d, CA: %d, UWrap %d, VWrap %d\n",
//               ind, "",
        printf(" AB %d BC %d CA %d UWrap %d VWrap %d\n",
               (flags & 0x04) != 0, (flags & 0x02) != 0, (flags & 0x01) != 0,
               (flags & 0x08) != 0, (flags & 0x10) != 0);
    }
        // Read rest of chunks inside this one.
    ChunkReader(f, ind, p);
}

void FaceMatReader (FILE *f, int ind, long p) {
    int c;
    word n, nf;

        // Read ASCIIZ material name
    printf("%*sMaterial name for faces: \"", ind, "");
    ASCIIZReader(f, ind, p);

    if (fread(&n, sizeof(n), 1, f) != 1) return;
    printf("%*sFaces with this material: %d\n", ind, "", n);
    while (n-- > 0) {
        if (fread(&nf, sizeof(nf), 1, f) != 1) return;
        printf("%*s    Face %d\n",
               ind, "", nf);
    }
}

void MapListReader (FILE *f, int ind, long p) {
    word nv;
    float c[2];

    if (fread(&nv, sizeof(nv), 1, f) != 1) return;
    printf("%*sVertices: %d\n", ind, "", nv);
    while (nv-- > 0) {
        if (fread(&c, sizeof(c), 1, f) != 1) return;
        printf("%*s    U: %f, V: %f\n", ind, "", c[0], c[1]);
    }
}

void SmooListReader (FILE *f, int ind, long p) {
    dword s;
    int i;

    while (ftell(f) < p) {
        if (fread(&s, sizeof(s), 1, f) != 1) return;
        printf("%*sSmoothing groups: ", ind, "");
        for (i = 0; i < 32; i++)
            if (s & (1 << i))
                printf("%d, ", i + 1);
        printf("\n");
    }
}

void TrMatrixReader(FILE *f, int ind, long p) {
    float rot[9];
    float trans[3];
    if (fread(&rot, sizeof(rot), 1, f) != 1) return;
    printf("%*sRotation matrix:\n", ind, "");
    printf("%*s    %f, %f, %f\n", ind, "", rot[0], rot[1], rot[2]);
    printf("%*s    %f, %f, %f\n", ind, "", rot[3], rot[4], rot[5]);
    printf("%*s    %f, %f, %f\n", ind, "", rot[6], rot[7], rot[8]);
    if (fread(&trans, sizeof(trans), 1, f) != 1) return;
    printf("%*sTranslation matrix: %f, %f, %f\n",
           ind, "", trans[0], trans[1], trans[2]);
}

void LightReader(FILE *f, int ind, long p) {
    float c[3];
    if (fread(&c, sizeof(c), 1, f) != 1) return;
    printf("%*s    X: %f, Y: %f, Z: %f\n", ind, "", c[0], c[1], c[2]);
        // Read rest of chunks inside this one.
    ChunkReader(f, ind, p);
}

void SpotLightReader(FILE *f, int ind, long p) {
    float c[5];
    if (fread(&c, sizeof(c), 1, f) != 1) return;
    printf("%*s    Target X: %f, Y: %f, Z: %f; Hotspot %f, Falloff %f\n",
           ind, "", c[0], c[1], c[2], c[3], c[4]);
}
 
void CameraReader(FILE *f, int ind, long p) {
    float c[8];
    if (fread(&c, sizeof(c), 1, f) != 1) return;
    printf("%*s    Position: X: %f, Y: %f, Z: %f\n", ind, "", c[0], c[1], c[2]);
    printf("%*s    Target: X: %f, Y: %f, Z: %f\n", ind, "", c[3], c[4], c[5]);
    printf("%*s    Bank: %f, Lens: %f\n", ind, "", c[6], c[7]);
}

void MatNameReader (FILE *f, int ind, long p) {
    int c;

        // Read ASCIIZ object name
    printf("%*sMaterial name \"", ind, "");
    ASCIIZReader(f, ind, p);
}

void MapFileReader(FILE *f, int ind, long p) {
    int c;

        // Read ASCIIZ filename
    printf("%*sMap filename \"", ind, "");
    ASCIIZReader(f, ind, p);
}

void FramesReader(FILE *f, int ind, long p) {
    dword c[2];
    if (fread(&c, sizeof(c), 1, f) != 1) return;
    printf("%*s    Start: %ld, End: %ld\n",
           ind, "", c[0], c[1]);
}

void TrackObjNameReader(FILE *f, int ind, long p) {
    int c;
    word w[2];
    word parent;

        // Read ASCIIZ name
    printf("%*sTrack object name \"", ind, "");
    ASCIIZReader(f, ind, p);
    if (fread(&w, sizeof(w), 1, f) != 1) return;
    if (fread(&parent, sizeof(parent), 1, f) != 1) return;
    printf("%*sObject name data: Flags 0x%X, 0x%X, Parent %d\n",
           ind, "", w[0], w[1], parent);
}

void PivotPointReader(FILE *f, int ind, long p) {
    float pos[3];

    if (fread(&pos, sizeof(pos), 1, f) != 1) return;
    printf("%*s  Pivot at X: %f, Y: %f, Z: %f\n",
           ind, "",
           pos[0], pos[1], pos[2]);
}

    /* Key info flags for position, rotation and scaling:
        Until I know the meaning of each bit in flags I assume all mean
        a following float data.
    */

        // NOTE THIS IS NOT A CHUNK, but A PART OF SEVERAL CHUNKS
void SplineFlagsReader(FILE *f, int ind, word flags) {
    int i;
    float dat;

    for (i = 0; i < 16; i++) 
	{
        static const char *flagnames[] = {
            "SPLINETension",
            "SPLINEContinuity",
            "SPLINEBias",
            "SPLINEEase To",
            "SPLINEEase From",
        };
        if (flags & (1 << i)) {
            if (fread(&dat, sizeof(dat), 1, f) != 1) return;
            if (i < sizeof(flagnames)/sizeof(*flagnames))
                printf("%*s             %-15s = %f\n",
                       ind, "", flagnames[i], dat);
            else
                printf("%*s             %-15s = %f\n",
                       ind, "", "Unknown", dat);
        }
    }
}

void TrackPosReader(FILE *f, int ind, long p) {
    word n, nf;
    float pos[3];
    word unkown;
    word flags;

    fseek(f, 10, SEEK_CUR);
    if (fread(&n, sizeof(n), 1, f) != 1) return;
    printf("%*sPosition keys: %d\n", ind, "", n);
    fseek(f, 2, SEEK_CUR);
    while (n-- > 0) {
        if (fread(&nf, sizeof(nf), 1, f) != 1) return;
        if (fread(&unkown, sizeof(unkown), 1, f) != 1) return;
        if (fread(&flags, sizeof(flags), 1, f) != 1) return;
        printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
        SplineFlagsReader(f, ind, flags);
        if (fread(&pos, sizeof(pos), 1, f) != 1) return;
        printf("%*s             X: %f, Y: %f, Z: %f\n",
               ind, "", pos[0], pos[1], pos[2]);
    }
}

void TrackRotReader(FILE *f, int ind, long p) {
    word n, nf;
    float pos[4];
    word unkown;
    word flags;

    fseek(f, 10, SEEK_CUR);
    if (fread(&n, sizeof(n), 1, f) != 1) return;
    printf("%*sRotation keys: %d\n", ind, "", n);
    fseek(f, 2, SEEK_CUR);
    while (n-- > 0) 
	{
        if (fread(&nf, sizeof(nf), 1, f) != 1) 
		{
			printf(" error nf\n");
			return;
		}
        if (fread(&unkown, sizeof(unkown), 1, f) != 1) 
		{
			printf(" error unknown\n");
			return;
		}
        if (fread(&flags, sizeof(flags), 1, f) != 1)
		{
			printf(" error flags\n");
			return;
		}
		printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);

        SplineFlagsReader(f, ind, flags);
        if (fread(&pos, sizeof(pos), 1, f) != 1) 
		{
			printf(" error pos\n");
			return;
		}
        printf("%*s             Angle: %f§, X: %f, Y: %f, Z: %f\n",ind, "", pos[0], pos[1], pos[2], pos[3]);
    }
}

void TrackScaleReader(FILE *f, int ind, long p) {
    word n, nf;
    float pos[3];
    word unkown;
    word flags;

    fseek(f, 10, SEEK_CUR);
    if (fread(&n, sizeof(n), 1, f) != 1) return;
    printf("%*sScale keys: %d\n", ind, "", n);
    fseek(f, 2, SEEK_CUR);
    while (n-- > 0) {
        if (fread(&nf, sizeof(nf), 1, f) != 1) return;
        if (fread(&unkown, sizeof(unkown), 1, f) != 1) return;
        if (fread(&flags, sizeof(flags), 1, f) != 1) return;
        printf("%*s  Frame %3d: Flags 0x%X\n", ind, "", nf, flags);
        SplineFlagsReader(f, ind, flags);
        if (fread(&pos, sizeof(pos), 1, f) != 1) return;
        printf("%*s            X: %f, Y: %f, Z: %f\n",
               ind, "", pos[0], pos[1], pos[2]);
    }
}

void ObjNumberReader(FILE *f, int ind, long p) {
    word n;

    if (fread(&n, sizeof(n), 1, f) != 1) return;
    printf("%*sObject number: %d\n", ind, "", n);
}


// ------------------------------------

struct {
    word id;
    const char *name;
    void (*func)(FILE *f, int ind, long p);
} ChunkNames[] = {
    {CHUNK_RGBF,        "RGB float",        RGBFReader},
    {CHUNK_RGBB,        "RGB byte",         RGBBReader},

    {CHUNK_PRJ,         "Project",          NULL},
    {CHUNK_MLI,         "Material Library", NULL},

    {CHUNK_MAIN,        "Main",             NULL},
    {CHUNK_OBJMESH,     "Object Mesh",      NULL},
    {CHUNK_BKGCOLOR,    "Background color", NULL},
    {CHUNK_AMBCOLOR,    "Ambient color",    NULL},
    {CHUNK_OBJBLOCK,    "Object Block",     ObjBlockReader},
    {CHUNK_TRIMESH,     "Tri-Mesh",         NULL},
    {CHUNK_VERTLIST,    "Vertex list",      VertListReader},
    {CHUNK_FACELIST,    "Face list",        FaceListReader},
    {CHUNK_FACEMAT,     "Face material",    FaceMatReader},
    {CHUNK_MAPLIST,     "Mappings list",    MapListReader},
    {CHUNK_SMOOLIST,    "Smoothings",       SmooListReader},
    {CHUNK_TRMATRIX,    "Matrix",           TrMatrixReader},
    {CHUNK_LIGHT,       "Light",            LightReader},
    {CHUNK_SPOTLIGHT,   "Spotlight",        SpotLightReader},
    {CHUNK_CAMERA,      "Camera",           CameraReader},
    {CHUNK_HIERARCHY,   "Hierarchy",        NULL},

    {CHUNK_VIEWPORT,    "Viewport info",    NULL},
    {CHUNK_MATERIAL,    "Material",         NULL},
    {CHUNK_MATNAME,     "Material name",    MatNameReader},
    {CHUNK_AMBIENT,     "Ambient color",    NULL},
    {CHUNK_DIFFUSE,     "Diffuse color",    NULL},
    {CHUNK_SPECULAR,    "Specular color",   NULL},
    {CHUNK_TEXTURE,     "Texture map",      NULL},
    {CHUNK_BUMPMAP,     "Bump map",         NULL},
    {CHUNK_MAPFILE,     "Map filename",     MapFileReader},

    {CHUNK_KEYFRAMER,   "Keyframer data",   NULL},
    {CHUNK_AMBIENTKEY,  "Ambient key",      NULL},
    {CHUNK_TRACKINFO,   "Track info",       NULL},
    {CHUNK_FRAMES,      "Frames",           FramesReader},
    {CHUNK_TRACKOBJNAME,"Track Obj. Name",  TrackObjNameReader},
    {CHUNK_TRACKPIVOT,  "Pivot point",      PivotPointReader},
    {CHUNK_TRACKPOS,    "Position keys",    TrackPosReader},
    {CHUNK_TRACKROTATE, "Rotation keys",    TrackRotReader},
    {CHUNK_TRACKSCALE,  "Scale keys",       TrackScaleReader},
    {CHUNK_OBJNUMBER,   "Object number",    ObjNumberReader},

    {CHUNK_TRACKCAMERA, "Camera track",             NULL},
    {CHUNK_TRACKCAMTGT, "Camera target track",      NULL},
    {CHUNK_TRACKLIGHT,  "Pointlight track",         NULL},
    {CHUNK_TRACKLIGTGT, "Pointlight target track",  NULL},
    {CHUNK_TRACKSPOTL,  "Spotlight track",          NULL},
    {CHUNK_TRACKFOV,    "FOV track",                NULL},
    {CHUNK_TRACKROLL,   "Roll track",               NULL},
};

int FindChunk(word id) {
    int i;
    for (i = 0; i < sizeof(ChunkNames)/sizeof(ChunkNames[0]); i++)
        if (id == ChunkNames[i].id)
            return i;
    return -1;
}

// ------------------------------------

int Verbose = 0;
int Quiet   = 0;

void ChunkReader(FILE *f, int ind, long p) {
	TChunkHeader h;
	int n;
	long pc;
	
	while (ftell(f) < p) 
	{
		pc = ftell(f);
		if (fread(&h, sizeof(h), 1, f) != 1) 
			return;
		if (h.len == 0) 
			return;
		n = FindChunk(h.id);
		if (n < 0) 
		{
			if (Verbose)
				printf("%*sUnknown chunk: 0x%04X, offset 0x%lX, size: %d bytes.\n",	ind, "", h.id, pc, h.len);
			fseek(f, pc + h.len, SEEK_SET);
		} 
		else 
		{
			if (!Quiet || ChunkNames[n].func == NULL)
				printf("%*sChunk type \"%s\", offset 0x%lX, size %d bytes\n",ind, "", ChunkNames[n].name, pc, h.len);
			pc = pc + h.len;
			if (ChunkNames[n].func != NULL)
				ChunkNames[n].func(f, ind + 2, pc);
			else
			{
				LogText(" Skip ,because NO CODE \n");
				ChunkReader(f, ind + 2, pc);
			}
			fseek(f, pc, SEEK_SET);
		}
		if (ferror(f))
			break;
	}
}

// ------------------------------------


void read_3ds(void) 
{
    FILE *f;
    long p;
	return;

    f = fopen("darci1.3ds", "rb");
    if (f == NULL) 
	{
        printf("Can't open %s!\n");
		return;
    }


        // Find file size.
    fseek(f, 0, SEEK_END);
    p = ftell(f);
    fseek(f, 0, SEEK_SET);
        // Go!
    ChunkReader(f, 0, p);
}


struct	TinyXZ	radius_pool[MAX_RADIUS*4*MAX_RADIUS*2];
struct	TinyXZ	*radius_ptr[MAX_RADIUS+2];

void	build_radius_info(void)
{
	SBYTE	*grid;
	SLONG	dx,dz;
	struct	TinyXZ	*ptr_rad;
	SLONG	actual_radius,radius,radius_offset,old_radius=0;
	SLONG	angle;
	SLONG	sum_count=0;
	SLONG	count=0;

	ptr_rad=radius_pool;


	grid=(SBYTE*)MemAlloc((MAX_RADIUS+1)*(MAX_RADIUS+1)*4);
	if(grid)
	{

		for(radius=(MAX_RADIUS<<2);radius>3;radius--)
		{
			if((radius>>2)!=old_radius)
			{
				old_radius=radius>>2;
//				LogText(" radius %d max_radius %d \n",radius>>2,MAX_RADIUS);
				radius_ptr[(radius>>2)]=ptr_rad;

			}
			for(angle=0;angle<2048;angle+=4)
			{
				for(radius_offset=-4;radius_offset<4;radius_offset++)
				{
					
					dx=(SIN(angle)*(radius+radius_offset))>>(16+2);
					dz=(COS(angle)*(radius+radius_offset))>>(16+2);
					actual_radius=Root(SDIST2(dx,dz));
					if(actual_radius==(radius>>2))
					{
						if(grid[(dx+MAX_RADIUS)+(dz+MAX_RADIUS)*(MAX_RADIUS*2)]!=-1)
						{
							grid[(dx+MAX_RADIUS)+(dz+MAX_RADIUS)*(MAX_RADIUS*2)]=-1;
							ptr_rad->Dx=dx;
							ptr_rad->Dz=dz;
							ptr_rad->Angle=angle;
							ptr_rad++;
						}
					}
				}
			}
		}
		radius_ptr[0]=ptr_rad;
		MemFree(grid);
	}
	for(radius=1;radius<MAX_RADIUS;radius++)
	{
//		LogText("count=%d \n",count);
//		LogText(" rad %d ->",radius);
		ptr_rad=radius_ptr[radius];
		count=0;
		while(ptr_rad<radius_ptr[radius-1])
		{
//			LogText("[%d](%d,%d) ",ptr_rad->Angle,ptr_rad->Dx,ptr_rad->Dz);
			ptr_rad++;
			count++;
		}
		sum_count+=count;
//		LogText("\n");
	}
//		LogText("count=%d sum %d\n",count,sum_count);
}

void	init_editor(void)
{
	SLONG	x,z;


	load_palette("data\\tex01.pal");
	build_radius_info();
	init_poly_system();	
//	clear_map();
//	read_3ds();
	PAP_clear();

	clear_map();
extern	void	init_map(void);
	init_map();
/*
	edit_map_eles[0].CubeType.Prim=255;
	edit_map_eles[1].CubeType.Prim=CUBE_TYPE_FULL;
	edit_map_eles[1].CubeFlags=CUBE_FLAG_ALL;

	for(x=0;x<EDIT_MAP_WIDTH;x++)
	{
		for(z=0;z<EDIT_MAP_DEPTH;z++)
		{
			insert_cube(x,126,z);
		}
	}
*/
	memset((UBYTE*)tex_map,0,sizeof(UWORD)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH);

	engine.X=(((EDIT_MAP_WIDTH<<(ELE_SHIFT-1))+0*HALF_ELE_SIZE)<<8);
	engine.Y=(((1<<ELE_SHIFT)+0*HALF_ELE_SIZE)<<8);
	engine.Z=(((EDIT_MAP_DEPTH<<(ELE_SHIFT-1))+0*HALF_ELE_SIZE)<<8);

	edit_info.amb_dx=-128;
	edit_info.amb_dy=-126;
	edit_info.amb_dz=-40;
	edit_info.amb_bright=255;
	edit_info.amb_flags=0;
	edit_info.amb_offset=0;
	edit_info.GridOn=1;
	edit_info.FlatShade=0;

	edit_info.RoofTex=0;
	edit_info.MapID=0;

	edit_info.TileFlag=0;
	edit_info.TileScale=100;
	edit_info.Inside=0;
	edit_info.HideMap=0;

	next_dbuilding=1;
	next_dwalkable=1;
	next_dfacet=1;
	next_dstyle=1;
	next_facet_link=1;
	facet_link_count=0;


	DeleteFile("data\\swap0.map");
	DeleteFile("data\\swap1.map");

	load_all_individual_prims();

	calc_prim_info();
void	clear_build_stuff(void);
	clear_build_stuff();
}


extern TinyXZ	radius_pool[MAX_RADIUS*4*MAX_RADIUS*2];
extern TinyXZ	*radius_ptr[MAX_RADIUS+2];



void	draw_quick_map(void)
{
	SLONG	radius;
	SLONG	dx,dz,cdx,cdz;
	struct	SVector point,ret_point;
	struct	TinyXZ	*ptr_rad;
	SLONG	clip_flags;

	struct	SVector		buffer_points[MAX_RADIUS*(MAX_RADIUS+1)*4];
	ULONG	buffer_flags[MAX_RADIUS*(MAX_RADIUS+1)*4];
	ULONG	*ptr_flag;
	struct	SVector		*ptr;
	SLONG	mx,my,mz;
	UWORD	texture;

	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

//	LogText(" draw arround (%d,%d,%d)\n",mx,my,mz);



	memset((UBYTE*)buffer_flags,0,MAX_RADIUS*(MAX_RADIUS+1)*4*4);

	ptr=&buffer_points[MAX_RADIUS+MAX_RADIUS*MAX_RADIUS*2];

	point.X=(mx<<ELE_SHIFT);
	point.Y=edit_map[mx][mz].Y<<FLOOR_HEIGHT_SHIFT;
	point.Z=(mz<<ELE_SHIFT); //(engine.Z>>8);

	clip_flags=rotate_point_gte(&point,ptr);
	buffer_flags[MAX_RADIUS+MAX_RADIUS*MAX_RADIUS*2]=clip_flags;

	for(radius=1;radius<MAX_RADIUS;radius++)
	{
		ptr_rad=radius_ptr[radius];
		while(ptr_rad<radius_ptr[radius-1])
		{
			//SLONG	mx,mz;
			cdx=ptr_rad->Dx;
			cdz=ptr_rad->Dz;

			dx=(cdx*ELE_SIZE);
			dz=(cdz*ELE_SIZE);

			cdx+=MAX_RADIUS;
			cdz+=MAX_RADIUS;

			ptr=&buffer_points[cdx+cdz*MAX_RADIUS*2];

			point.X=dx+(mx<<ELE_SHIFT);

			mx=cdx+mx-MAX_RADIUS;
			mz=cdz+mz-MAX_RADIUS;
			if(mx>=0&&mx<EDIT_MAP_WIDTH&& mz>=0 && mz<EDIT_MAP_DEPTH)
				point.Y=edit_map[cdx+mx-MAX_RADIUS][cdz+mz-MAX_RADIUS].Y<<FLOOR_HEIGHT_SHIFT;
			else
				point.Y=0;
			point.Z=dz+(mz<<ELE_SHIFT); //(engine.Z>>8);


			clip_flags=rotate_point_gte(&point,ptr);
//			if( ((clip_flags&EF_BEHIND_YOU)==0) && !(clip_flags & EF_CLIPFLAGS))
			{
				buffer_flags[cdx+cdz*MAX_RADIUS*2]=clip_flags;
			}
			ptr_rad++;
		}
	}

	ptr_flag=buffer_flags;
	for(dz=0;dz<MAX_RADIUS*2;dz++)
	for(dx=0;dx<MAX_RADIUS*2;dx++)
	{
		SLONG	flag_and,flag_or;
		SLONG	index;

//		if(dx+mx-MAX_RADIUS-1>=0&&dx+mx-MAX_RADIUS<EDIT_MAP_WIDTH&&dz+mz-MAX_RADIUS-1>=0&&dz+mz-MAX_RADIUS<EDIT_MAP_DEPTH)

		if(on_edit_map(dx+mx-MAX_RADIUS-1,dz+mz-MAX_RADIUS-1))
		{
			
			index=edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].MapThingIndex;
			while(index)
			{
	//			LogText(" draw QUICK index %d at dx %d dz %d (%d,%d)\n",index,dx,dz,mx+dx,mz+dz);
				draw_map_thing(index);
				index=map_things[index].MapChild;
			}

			//index=edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].ColVectHead;
			//if(!index)

//EC HIDDEN_FLOOR
			if(!(edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].Flags&FLOOR_HIDDEN))
			{
				flag_and=(*ptr_flag) & (*(ptr_flag+1)) & (*(ptr_flag+MAX_RADIUS*2)) & (*(ptr_flag+1+MAX_RADIUS*2));
				flag_or =(*ptr_flag) | (*(ptr_flag+1)) | (*(ptr_flag+MAX_RADIUS*2)) | (*(ptr_flag+1+MAX_RADIUS*2));
		//		if(*ptr_flag&&*(ptr_flag+1)&&*(ptr_flag+MAX_RADIUS*2)&&*(ptr_flag+1+MAX_RADIUS*2))
				if( ((flag_or&EF_BEHIND_YOU)==0) && !(flag_and & EF_CLIPFLAGS) && (flag_and & EF_TRANSLATED))
				{
						SLONG	x1,x2,x3,x4;
						SLONG	y1,y2,y3,y4;
						SLONG	z1;
						x2=buffer_points[dx+dz*MAX_RADIUS*2].X;
						x1=buffer_points[dx+dz*MAX_RADIUS*2+1].X;
						x4=buffer_points[dx+(dz+1)*MAX_RADIUS*2].X;
						x3=buffer_points[dx+(dz+1)*MAX_RADIUS*2+1].X;
						y2=buffer_points[dx+dz*MAX_RADIUS*2].Y;
						y1=buffer_points[dx+dz*MAX_RADIUS*2+1].Y;
						y4=buffer_points[dx+(dz+1)*MAX_RADIUS*2].Y;
						y3=buffer_points[dx+(dz+1)*MAX_RADIUS*2+1].Y;
						z1=buffer_points[dx+dz*MAX_RADIUS*2].Z;
						if(edit_info.RoofTex)
						{
							texture=tex_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)];
						}
						else
						{
							texture=edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].Texture;
						}
						add_floor_face_to_bucket(x2,y2,z1,x1,y1,0,x4,y4,0,x3,y3,0,&edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)],
							edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].Bright,
							edit_map[(dx+mx-MAX_RADIUS+1)][(dz+mz-MAX_RADIUS)].Bright,
							edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS+1)].Bright,
							edit_map[(dx+mx-MAX_RADIUS+1)][(dz+mz-MAX_RADIUS+1)].Bright,texture);

	//				DrawLineC(buffer_points[dx+dz*MAX_RADIUS*2].X,buffer_points[dx+dz*MAX_RADIUS*2].Y,buffer_points[dx+dz*MAX_RADIUS*2+1].X,buffer_points[dx+dz*MAX_RADIUS*2+1].Y,255);
	//				DrawLineC(buffer_points[dx+dz*MAX_RADIUS*2+1].X,buffer_points[dx+dz*MAX_RADIUS*2+1].Y,buffer_points[dx+(dz+1)*MAX_RADIUS*2+1].X,buffer_points[dx+(dz+1)*MAX_RADIUS*2+1].Y,255);
				}
			}

		}
		ptr_flag++;
	
	}

}

//struct	QuickMap	quick_map[128*128];






