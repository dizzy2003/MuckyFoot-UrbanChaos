//
// File I/O for game (M.C.Diskett)
//

#include "game.h"
#include "pap.h"
#include "sound.h"
#include "ob.h"
#include "supermap.h"
#include "c:\fallen\editor\headers\thing.h"
#include	"io.h"
#include	"eway.h"
#include "c:\fallen\headers\inside2.h"
#include	"memory.h"
#include "c:\fallen\headers\noserver.h"
#ifdef FS_ISO9660
#include <libcd.h>
#include <ctype.h>
#endif

//		STOREY_TYPE_NORMAL

#ifndef	PSX

#include	"math.h"
void	skip_load_a_multi_prim(MFFileHandle	handle);

#else

//
// PSX include
//
#include "libsn.h"
extern	void			TEXTURE_choose_set(SLONG number);

#define	MAX_PATH	128
#define	FILE	SLONG

#define	MFFileHandle	SLONG
#define	FILE_OPEN_ERROR	(-1)
#define	SEEK_MODE_CURRENT	(1)

extern	SLONG	SpecialOpen(CBYTE *name);
extern	SLONG	SpecialRead(SLONG handle,UBYTE *ptr,SLONG s1);
extern	SLONG	SpecialSeek(SLONG handle,SLONG mode,SLONG size);

#define	FileOpen(x)		SpecialOpen(x)
#define	FileClose(x)	SpecialClose(x)
#define	FileCreate(x,y)	ASSERT(0)
#define	FileRead(h,a,s) SpecialRead(h,(UBYTE*)a,s)
#define	FileWrite(h,a,s) ASSERT(0)
#define	FileSeek(h,m,o) SpecialSeek(h,m,o)

#define	MF_Fopen(x,y)		SpecialOpen(x)
#define	MF_Fclose(x)		SpecialClose(x)
#define	fread(a,s1,s2,h) SpecialRead(h,a,s1*s2)

#endif

//#include "math.h"
extern	CBYTE	texture_style_names[200][21];
extern	void	fix_style_names(void);
SLONG	load_a_multi_prim(CBYTE *name);
void	create_kline_bottle(void);
SLONG	load_anim_system(struct GameKeyFrameChunk *p_chunk,CBYTE	*name,SLONG type=0);
SLONG load_anim_prim_object(SLONG prim);

#ifdef EDITOR
extern	CBYTE	inside_names[64][20];
#endif

#ifndef PSX
#ifdef	NO_SERVER
CBYTE	EXTRAS_DIR[100]="data\\textures";
CBYTE	PRIM_DIR[100]="server\\prims";
CBYTE	DATA_DIR[100]="";
CBYTE	LEVELS_DIR[100]="";
CBYTE	TEXTURE_WORLD_DIR[100]="";
#else

CBYTE	EXTRAS_DIR[100]="data\\textures";
CBYTE	PRIM_DIR[100]="u:\\urbanchaos\\prims";
CBYTE	DATA_DIR[100]="";
CBYTE	LEVELS_DIR[100]="";
CBYTE	TEXTURE_WORLD_DIR[100]="";
#endif

#else

struct	FileSystem2
{
	UBYTE	*filemem;
	ULONG	fileindex;
	ULONG	filelen;
};

struct	FileSystem2 file_system[5];
SLONG	file_handle=1;

SLONG	SpecialSize(SLONG handle)
{
	return file_system[handle].filelen;
}

#if 1
extern char *GDisp_Bucket;
#else
extern char GDisp_Bucket[];
#endif

SLONG	SpecialOpen(CBYTE *name)
{
	SLONG	handle;
	struct	FileSystem2 *fs;

#ifndef FS_ISO9660
	fs=&file_system[file_handle];
	handle=PCopen(name,0,0);
	if(handle!=FILE_OPEN_ERROR)
	{

		fs->filelen=PClseek(handle,0,2);
		fs->fileindex=0;


		fs->filemem=(UBYTE*)&GDisp_Bucket[BUCKET_MEM-fs->filelen];
		ASSERT(fs->filemem);
		PClseek(handle,0,0);
		PCread(handle,fs->filemem,fs->filelen);
		PCclose(handle);
		file_handle++;
		return(file_handle-1);
	}
	else
		return(handle);
#else
extern char cd_file_buffer[];
extern SLONG MFX_Seek_delay;

	CdlFILE cfile;
	char *p;

	sprintf(cd_file_buffer,"\\%s;1",name);

	p=cd_file_buffer;
	while(*p) *p++=toupper(*p);

	MFX_Seek_delay=INFINITY;
	if (CdSearchFile(&cfile,cd_file_buffer)==0)
		return FILE_OPEN_ERROR;

	fs=&file_system[file_handle];
	fs->filelen=cfile.size;
	fs->fileindex=0;
	fs->filemem=(UBYTE*)&GDisp_Bucket[BUCKET_MEM-(fs->filelen+2048)];

	ASSERT(fs->filemem);

	CdReadFile(cd_file_buffer,(ULONG*)fs->filemem,fs->filelen);
	CdReadSync(0,cd_file_buffer);

	file_handle++;
	return(file_handle-1);
#endif
}

SLONG	SpecialRead(SLONG handle,UBYTE *ptr,SLONG s1)
{
	SLONG	c0;
	struct	FileSystem2 *fs;

	fs=&file_system[handle];

	if(fs->fileindex>=fs->filelen) return FILE_OPEN_ERROR;
	

	for(c0=0;c0<s1;c0++)
	{
		*ptr++=fs->filemem[fs->fileindex++];
	}

	return(s1);
}


SLONG	SpecialSeek(SLONG handle,SLONG mode,SLONG size)
{
	SLONG	c0,max;
	struct	FileSystem2 *fs;

	fs=&file_system[handle];

	if(mode==0)
		fs->fileindex=size;

	if(mode==1)
		fs->fileindex+=size;

	if(mode==2)
		fs->fileindex=fs->filelen+size;
	return(size);
}

SLONG	SpecialClose(SLONG handle)
{
	struct	FileSystem2 *fs;

	fs=&file_system[handle];

	ASSERT(handle==file_handle-1);
//	MemFree(fs->filemem);
	fs->filemem=0;
	file_handle--;
extern SLONG MFX_Seek_delay;
	MFX_Seek_delay=20;
	return(1);
}

#endif

#ifndef PSX


UWORD	local_next_prim_point;
UWORD	local_next_prim_face4;
UWORD	local_next_prim_face3;
UWORD	local_next_prim_object;
UWORD	local_next_prim_multi_object;


void	record_prim_status(void)
{

	local_next_prim_point = next_prim_point;       
	local_next_prim_face4 = next_prim_face4;       
	local_next_prim_face3 = next_prim_face3;       
	local_next_prim_object = next_prim_object;      
	local_next_prim_multi_object=next_prim_multi_object;
}

void	revert_to_prim_status(void)
{

	next_prim_point =      local_next_prim_point;       
	next_prim_face4 =      local_next_prim_face4;       
	next_prim_face3 =      local_next_prim_face3;       
	next_prim_object =     local_next_prim_object;      
	next_prim_multi_object=local_next_prim_multi_object;
}



SLONG	find_colour(UBYTE *the_palette,SLONG r,SLONG g,SLONG b)
{
	SLONG	found	=	-1;
	SLONG	dist	=	0x7fffffff,
			c0,
			dist2,
			tr,
			tg,
			tb;

	if(r>255)
		r=255;
	if(g>255)
		g=255;
	if(b>255)
		b=255;

	for(c0=0;c0<256;c0++)
	{
		tr	=	*the_palette++;
		tg	=	*the_palette++;
		tb	=	*the_palette++;

		tr	-=	r;
		tg	-=	g;
		tb	-=	b;

		dist2=	abs(tr*tr)*1+abs(tg*tg)*1+abs(tb*tb)*1; //we notice differences in red more than green more than blue1
		if(dist2<dist)
		{
			found	=	c0;
			dist	=	dist2;
			if(dist<8)
				return(c0);
		}
	}
	return(found);
}


void	change_extension(CBYTE	*name,CBYTE *add,CBYTE *new_name)
{
	SLONG	c0=0;
	while(name[c0])
	{
		new_name[c0]=name[c0];
		if(name[c0]=='.')
		{
			new_name[c0+1]=add[0];
			new_name[c0+2]=add[1];
			new_name[c0+3]=add[2];
			new_name[c0+4]=0;
			return;
		}
		c0++;
	}
	new_name[c0]='.';
	new_name[c0+1]=add[0];
	new_name[c0+2]=add[1];
	new_name[c0+3]=add[2];
	new_name[c0+4]=0;
}


void	load_texture_instyles(UBYTE editor, UBYTE world)
{
	UWORD	temp,temp2;
	SLONG	save_type=1;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	CBYTE   fname[MAX_PATH];

	//
	// Which file do we try to load?
	//


	sprintf(fname, "%sinstyle.tma", TEXTURE_WORLD_DIR);

	handle=FileOpen(fname);

	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&save_type,4);

		FileRead(handle,(UBYTE*)&temp,2);
		FileRead(handle,(UBYTE*)&temp2,2);
		FileRead(handle,(UBYTE*)&inside_tex[0][0],sizeof(UBYTE)*temp*temp2);
		FileRead(handle,(UBYTE*)&temp,2);
		FileRead(handle,(UBYTE*)&temp2,2);

		if(editor)
		{
#ifdef	EDITOR
			FileRead(handle,(UBYTE*)&inside_names[0][0],temp*temp2);
#endif
		}
		else
			FileSeek(handle,SEEK_MODE_CURRENT,temp*temp2);



		FileClose(handle);
	}

}
#endif

#ifndef PSX
void	load_texture_styles(UBYTE editor, UBYTE world)
{
	UWORD	temp,temp2;
	SLONG	save_type=1;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	CBYTE   fname[MAX_PATH];

	//
	// Which file do we try to load?
	//

#ifndef PSX
	sprintf(fname, "%sstyle.tma", TEXTURE_WORLD_DIR);
//	sprintf(fname, "u:\\urbanchaos\\textures\\world%d\\style.tma", world);
#else
	sprintf(fname, "data\\textures\\world%d\\style.pma",world);
#endif

	handle=FileOpen(fname);

	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&save_type,4);
		if(save_type>1)
		{

			if(save_type<4)
			{
				FileRead(handle,(UBYTE*)&temp,2);
				FileSeek(handle,SEEK_MODE_CURRENT,sizeof(struct TextureInfo)*8*8*temp);
			//	FileRead(handle,(UBYTE*)&temp,2);
			//	FileRead(handle,(UBYTE*)&texture_info[0],sizeof(struct TextureInfo)*8*8*temp);
			}


			FileRead(handle,(UBYTE*)&temp,2);
			FileRead(handle,(UBYTE*)&temp2,2);
			ASSERT(temp==200);
			if(save_type<5)
			{
				SLONG	c0,c1;
				ASSERT(temp2==8);
				for(c0=0;c0<temp;c0++)
				{
					FileSeek(handle,SEEK_MODE_CURRENT,sizeof(struct TXTY)*3);
					FileRead(handle,(UBYTE*)&textures_xy[c0][0],sizeof(struct TXTY)*(temp2-3));
				}
			}
			else
			{
				ASSERT(temp2==5);
				FileRead(handle,(UBYTE*)&textures_xy[0][0],sizeof(struct TXTY)*temp*temp2);
			}

			FileRead(handle,(UBYTE*)&temp,2);
			FileRead(handle,(UBYTE*)&temp2,2);
			ASSERT(temp==200);
			ASSERT(temp2==21);
			if(editor)
			{
#ifdef	EDITOR
				FileRead(handle,(UBYTE*)&texture_style_names[0][0],temp*temp2);
				fix_style_names();
#endif
			}
			else
				FileSeek(handle,SEEK_MODE_CURRENT,temp*temp2);


			ASSERT(save_type>2);
			FileRead(handle,(UBYTE*)&temp,2);
			FileRead(handle,(UBYTE*)&temp2,2);
			if(save_type<5)
			{
				SLONG	c0;
				for(c0=0;c0<temp;c0++)
				{
					ASSERT(temp2==8);
					FileSeek(handle,SEEK_MODE_CURRENT,3);
					FileRead(handle,(UBYTE*)&textures_flags[c0][0],sizeof(UBYTE)*(temp2-3));

				}


			}
			else
			{
				if(temp2!=5)
				{
					SLONG	c0,c1;
					FileClose(handle);
//					memset((UBYTE*)&textures_flags[0][0],0,temp*5);
					for(c0=0;c0<temp;c0++)
					for(c1=0;c1<5;c1++)
					{
						textures_flags[c0][c1]=POLY_GT;
					}
					return;
				}
				LogText(" read flags x %d z %d\n",temp,temp2);
				FileRead(handle,(UBYTE*)&textures_flags[0][0],sizeof(UBYTE)*temp*temp2);
			}

/*
			if(save_type>2)
			{
				FileRead(handle,(UBYTE*)&temp,2);
				FileRead(handle,(UBYTE*)&temp2,2);
				LogText(" read flags x %d z %d\n",temp,temp2);
				FileRead(handle,(UBYTE*)&textures_flags[0][0],sizeof(UBYTE)*temp*temp2);
			}
			else
			{
				SLONG	x,z;
				for(x=0;x<200;x++)
				{
					for(z=0;z<8;z++)
					{
//						textures_flags[x][z]=POLY_GT;
					}
				}
			}
*/
		}
		FileClose(handle);
	}

}

#ifndef TARGET_DC

SLONG load_anim_prim_object(SLONG prim)
{
	CBYTE fname[130];
#ifdef	PSX
	FILE handle;
#else
	FILE *handle;
#endif
	ASSERT(WITHIN(prim, 0, 255));


	if(anim_chunk[prim].MultiObject[0])
		return(0);

	sprintf(fname, "anim%03d.all", prim);
/*
	handle = MF_Fopen(fname, "rb");

	if (!handle)
	{
		return FALSE;
	}
	MF_Fclose(handle);
*/

	if(prim>=next_anim_chunk)
		next_anim_chunk=prim+1;

	return(load_anim_system(&anim_chunk[prim],fname));
//	return(TRUE);
}

extern	SLONG	save_psx;

void load_needed_anim_prims()
{
	SLONG c0;

	//
	// Anim prims are loaded as they are found on the map- but this
	// doesn't do any harm because load_anim_prim_object() only
	// loads the prim if it isn't already loaded.
	//

	for(c0=1;c0<MAX_PRIMARY_THINGS;c0++)
	{
		if(TO_THING(c0)->Class	==	CLASS_ANIM_PRIM)
		{
			load_anim_prim_object(TO_THING(c0)->Index);
			DebugText(" next_prim_point %d primface3 %d primface4 %d   load ANIMprim %d \n",next_prim_point,next_prim_face3,next_prim_face4,TO_THING(c0)->Index);
		}



	}

	//
	// We always need the bat, gargoyle and balrog and bane (in the final levels anyway!)
	//


	if(!save_psx)
	{
		//load_anim_prim_object(1);	  //bat
		//load_anim_prim_object(2); //gargoyle		// Never used!

		extern UBYTE this_level_has_the_balrog;
		extern UBYTE this_level_has_bane;

		if (this_level_has_the_balrog)
		{
			load_anim_prim_object(3); //balrog
		}

		if (this_level_has_bane)
		{
			load_anim_prim_object(4); //bane
		}
	}

//	load_anim_prim_object(3); //balrog
	#ifdef BIKE
	load_anim_prim_object(9);
			DebugText(" next_prim_point %d primface3 %d primface4 %d   load ANIMprim %d \n",next_prim_point,next_prim_face3,next_prim_face4,9);
	load_anim_prim_object(12);
			DebugText(" next_prim_point %d primface3 %d primface4 %d   load ANIMprim %d \n",next_prim_point,next_prim_face3,next_prim_face4,12);
	#endif
}

void load_level_anim_prims(void)
{
	SLONG	i;
	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
		if(ew->ed.type==EWAY_DO_CREATE_ANIMAL)
		{
			SLONG	anim;
			anim=0;
			switch(ew->ed.subtype)
			{
				
				case EWAY_SUBTYPE_ANIMAL_BAT:
					if(save_psx)
						continue;
					anim=1;
					break;
				case EWAY_SUBTYPE_ANIMAL_GARGOYLE:
					anim=2;
					break;
				case EWAY_SUBTYPE_ANIMAL_BALROG:
					anim=3;
					break;
				case EWAY_SUBTYPE_ANIMAL_BANE:
					anim=4;
					break;
			}
			if(anim_chunk[anim].AnimList==0)
				load_anim_prim_object(anim);
		}
	}
}

void	load_game_map(CBYTE *name)
{
	UWORD	i;
	UWORD	temp;
	SLONG	save_type=1, ob_size;
	SWORD	x,z;
	SWORD	c0;
	MapElement	me;
	Thing	th;
	struct	MapThingPSX	*t_mthing;

	UWORD	count1,count2,count3,count4;
	CBYTE	fname[100];

	sprintf(fname,"%s%s",DATA_DIR,name);

	if(save_psx)
	{
		fname[strlen(fname)-3]='p';
		if(!FileExists(fname))
			fname[strlen(fname)-3]='i';
	}


	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	handle=FileOpen(fname);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&save_type,4);

		// extra 'ob' info
		if (save_type>23) {
			FileRead(handle,(UBYTE*)&ob_size,4);
		}

		FileRead(handle,(UBYTE*)&PAP_2HI(0,0),sizeof(PAP_Hi)*PAP_SIZE_HI*PAP_SIZE_HI);

extern	UWORD WARE_roof_tex[PAP_SIZE_HI][PAP_SIZE_HI];

		if(save_psx&&save_type>=26)
		{
			ULONG	check;

			//
			//PSX has the remaped rooftop textures stuck in the pam
			//

			FileRead(handle,(UBYTE*)&check,4);
			ASSERT(check==sizeof(UWORD)*PAP_SIZE_HI * PAP_SIZE_HI);

			FileRead(handle,WARE_roof_tex, sizeof(UWORD)*PAP_SIZE_HI * PAP_SIZE_HI);
			FileRead(handle,(UBYTE*)&check,4);
			ASSERT(check==666);
		}
		else
		{
			memset((UBYTE*)WARE_roof_tex,0,sizeof(UWORD)*PAP_SIZE_HI * PAP_SIZE_HI);
		}


		//
		// Clear the mapwho data in the low-res map.
		//

		for (x = 0; x < PAP_SIZE_LO; x++)
		for (z = 0; z < PAP_SIZE_LO; z++)
		{
			PAP_2LO(x,z).MapWho = 0;
		}


		if(save_type==18)
		{
			FileRead(handle,(UBYTE*)&temp,sizeof(temp));
			for(c0=0;c0<temp;c0++)
			{
				struct	MapThingPSX	map_thing;
				t_mthing=&map_thing;
				FileRead(handle,(UBYTE*)&map_thing,sizeof(struct MapThingPSX));

				switch(t_mthing->Type)
				{
					case	MAP_THING_TYPE_ANIM_PRIM:		
						//
						// Now add an animating prim to the universe
						//

	extern	void	create_anim_prim(SLONG x,SLONG y,SLONG z,SLONG prim, SLONG yaw);

						load_anim_prim_object(t_mthing->IndexOther);
						create_anim_prim(t_mthing->X,t_mthing->Y,t_mthing->Z,t_mthing->IndexOther,t_mthing->AngleY);
						break;
				}
			}
		}
		else if(save_type>18)
		{
			FileRead(handle,(UBYTE*)&temp,sizeof(temp));
			for(c0=0;c0<temp;c0++)
			{
				struct	LoadGameThing	map_thing;
				FileRead(handle,(UBYTE*)&map_thing,sizeof(struct LoadGameThing));

				switch(map_thing.Type)
				{
					case	MAP_THING_TYPE_ANIM_PRIM:		
						//
						// Now add an animating prim to the universe
						//

	extern	void	create_anim_prim(SLONG x,SLONG y,SLONG z,SLONG prim, SLONG yaw);

						load_anim_prim_object(map_thing.IndexOther);
						create_anim_prim(map_thing.X,map_thing.Y,map_thing.Z,map_thing.IndexOther,map_thing.AngleY);
						break;
				}
			}
		}

		//
		// All this ob nonsense are the objects on the map (like lamposts)
		//

		if(save_type<23)
		{

			FileRead(handle,(UBYTE*)&OB_ob_upto,sizeof(OB_ob_upto));
			FileRead(handle,(UBYTE*)&OB_ob[0],sizeof(OB_Ob)*OB_ob_upto);

			//
			// Notice that strangely they have their very own mapwho
			//

			FileRead(handle,(UBYTE*)&OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
		}

		for (i = 1; i < OB_ob_upto; i++)
		{
			OB_ob[i].flags &= ~OB_FLAG_DAMAGED;
			OB_ob[i].flags &= ~OB_FLAG_RESERVED1;
			OB_ob[i].flags &= ~OB_FLAG_RESERVED2;
		}

		//
		// load super map
		//

		load_super_map(handle,save_type);

		//
		// Load all the prim objects 
		//

		OB_load_needed_prims();
		load_needed_anim_prims();
	DebugText("Julyb npp %d npf3 %d name %s\n",next_prim_point,next_prim_face3,name);


		if (save_type >= 20)
		{
			SLONG texture_set;

			FileRead(handle,(UBYTE*)&texture_set,sizeof(texture_set));

			ASSERT(WITHIN(texture_set, 0, 21));

			TEXTURE_choose_set(texture_set);
			TEXTURE_SET=texture_set;
			world_type=0;
			if(texture_set==5)
			{
//				ASSERT(0);
				world_type=WORLD_TYPE_SNOW;
			}
			else
			if(texture_set==1)
			{
//				ASSERT(0);
				world_type=WORLD_TYPE_FOREST;
			}
		}
		else
		{
			TEXTURE_choose_set(1);
		}
		if (save_type >= 25)
		{
			FileRead(handle,(UBYTE*)psx_textures_xy,2*200*5);
		}

		FileClose(handle);
	}
#ifdef EDITOR
void	load_tex_remap(CBYTE *name);
	load_tex_remap(name);
#endif
	DebugText("Julyc npp %d npf3 %d \n",next_prim_point,next_prim_face3);

}

/*
void	add_point(SLONG x,SLONG y,SLONG z)
{
	prim_points[next_prim_point].X=x;
	prim_points[next_prim_point].Y=y;
	prim_points[next_prim_point].Z=z;
	next_prim_point++;
}
#define	CHEIGHT1	80
#define	CHEIGHT2	160
void	build_car_prim(void)
{
	SLONG sp[5];

	sp[0]=next_prim_point;

	add_point(-128,0,-30);
	add_point(-128,CHEIGHT,-30);

	add_point(-128,0,-30);
	add_point(-128,CHEIGHT,-30);



}
*/

SLONG	load_all_prims(CBYTE	*name)
{
	SLONG			c0,point;
	MFFileHandle	handle;
	CBYTE			file_name[64];
	UWORD	dummy[100];
	UWORD	check_it;

/* fucked because primpoint size changed	
	sprintf(file_name,"data\\%s",name);
	handle	=	FileOpen(file_name);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&next_prim_point,sizeof(UWORD));
		FileRead(handle,(UBYTE*)&next_prim_face4,sizeof(UWORD));
		FileRead(handle,(UBYTE*)&next_prim_face3,sizeof(UWORD));
		FileRead(handle,(UBYTE*)&next_prim_object,sizeof(UWORD));

		FileRead(handle,(UBYTE*)dummy,sizeof(UWORD)*10);

		FileRead(handle,(UBYTE*)prim_points,sizeof(struct PrimPoint)*next_prim_point);
		FileRead(handle,(UBYTE*)prim_faces4,sizeof(struct PrimFace4)*next_prim_face4);
		FileRead(handle,(UBYTE*)prim_faces3,sizeof(struct PrimFace3)*next_prim_face3);
		FileRead(handle,(UBYTE*)prim_objects,sizeof(struct PrimObject)*next_prim_object);
		FileClose(handle);
#ifdef	EDITOR
extern	void	record_prim_status(void);
		record_prim_status();
#endif
		//create_kline_bottle();
		return(1);
	}
*/
	return(0);
	
}


//
// Loads in the given prim object if it is not already loaded.
// Returns FALSE on failure.
//


SLONG load_prim_object(SLONG prim)
{
	SLONG i;
	SLONG j;

	SLONG num_points;
	SLONG num_faces3;
	SLONG num_faces4;

	PrimObject *po;
	PrimFace3  *f3;
	PrimFace4  *f4;

	CBYTE fname[156];
	UWORD	save_type;
	UWORD	file_type=1;

	MFFileHandle handle;

	if(prim==15)
	{
		LogText("hello");
	}

	ASSERT(WITHIN(prim, 0, 265));

//	ASSERT(prim!=238);
	po = &prim_objects[prim];

	//
	// Is this prim already in memory?
	//

	if (po->StartPoint||prim==238)
	{
		//
		// Don't load twice!
		//

		return TRUE;
	}

	sprintf(fname, "%s\\nprim%03d.prm", PRIM_DIR,prim);

	handle = FileOpen(fname);
	if(handle==FILE_OPEN_ERROR)
	{

		sprintf(fname, "%s\\prim%03d.prm", PRIM_DIR,prim);


		handle = FileOpen(fname);
		if(handle==FILE_OPEN_ERROR)
		{
			//
			// Oh dear!
			// 
			po->StartPoint=0;
			po->EndPoint=0;
			po->StartFace3=0;
			po->EndFace3=0;
			po->StartFace4=0;
			po->EndFace4=0;
			return FALSE;
		}
		file_type=0;
	}

	//
	// Load in the prim object.
	//

	if(file_type==1)
	{
		FileRead(handle,&save_type, sizeof(save_type));
		FileRead(handle,&prim_names[prim],32);
		if(FileRead(handle,&prim_objects[prim], sizeof(PrimObject)) != sizeof(PrimObject))
			goto file_error;
	}
	else
	{
		struct	PrimObjectOld	oldprim;
		if (FileRead(handle,&oldprim, sizeof(PrimObjectOld)) != sizeof(PrimObjectOld))
			goto file_error;
		prim_objects[prim].coltype=oldprim.coltype;
		prim_objects[prim].damage=oldprim.damage;
		prim_objects[prim].EndFace3=oldprim.EndFace3;
		prim_objects[prim].EndFace4=oldprim.EndFace4;
		prim_objects[prim].StartFace3=oldprim.StartFace3;
		prim_objects[prim].StartFace4=oldprim.StartFace4;
		prim_objects[prim].EndPoint=oldprim.EndPoint;
		prim_objects[prim].StartPoint=oldprim.StartPoint;
		prim_objects[prim].shadowtype=oldprim.shadowtype;
		prim_objects[prim].flag=oldprim.flag;
		memcpy(prim_names[prim],oldprim.ObjectName,32);
		save_type=oldprim.Dummy[3];
	}

	num_points = po->EndPoint - po->StartPoint;
	num_faces3 = po->EndFace3 - po->StartFace3;
	num_faces4 = po->EndFace4 - po->StartFace4;
	ASSERT(num_points>=0);
	ASSERT(num_faces3>=0);
	ASSERT(num_faces4>=0);



	//
	// Enough memory?
	// 

	if (next_prim_point + num_points > MAX_PRIM_POINTS ||
		next_prim_face3 + num_faces3 > MAX_PRIM_FACES3 ||
		next_prim_face4 + num_faces4 > MAX_PRIM_FACES4)
	{
		FileClose(handle);

		po->StartPoint = po->EndPoint = 0;
		po->StartFace3 = po->EndFace3 = 0;
		po->StartFace4 = po->EndFace4 = 0;
		ASSERT(0);

		return FALSE;
	}

	//
	// Load in the point and faces data.
	//

	if(save_type-PRIM_START_SAVE_TYPE==1)
	{
		if (FileRead(handle,&prim_points[next_prim_point], sizeof(PrimPoint)*num_points) != sizeof(PrimPoint)*num_points) 
			goto file_error;
	}
	else
	{
		SLONG	c0;
		struct	OldPrimPoint pp;
		for(c0=0;c0<num_points;c0++)
		{
			if (FileRead(handle,&pp, sizeof(OldPrimPoint)) != sizeof(OldPrimPoint)) 
				goto file_error;

			prim_points[c0+next_prim_point].X=(SWORD)pp.X;
			prim_points[c0+next_prim_point].Y=(SWORD)pp.Y;
			prim_points[c0+next_prim_point].Z=(SWORD)pp.Z;
			

		}
	}
	if (FileRead(handle,&prim_faces3[next_prim_face3], sizeof(PrimFace3)*num_faces3) != sizeof(PrimFace3)*num_faces3) goto file_error;
	if (FileRead(handle,&prim_faces4[next_prim_face4], sizeof(PrimFace4)*num_faces4) != sizeof(PrimFace4)*num_faces4) goto file_error;

	FileClose(handle);

	/*

	if (prim == PRIM_OBJ_BALLOON)
	{
		for (i = 0; i < num_points; i++)
		{
			prim_points[next_prim_point + i].X /= 2;
			prim_points[next_prim_point + i].Y /= 2;
			prim_points[next_prim_point + i].Z /= 2;
		}
	}

	*/

	//
	// Fix the point indices inside the prim faces
	// 

	for (i = 0; i < num_faces3; i++)
	{
		f3 = &prim_faces3[next_prim_face3 + i];

		for (j = 0; j < 3; j++)
		{
			f3->Points[j] += next_prim_point - po->StartPoint;
		}
	}

	for (i = 0; i < num_faces4; i++)
	{
		f4 = &prim_faces4[next_prim_face4 + i];

		for (j = 0; j < 4; j++)
		{
			f4->Points[j] += next_prim_point - po->StartPoint;
		}
	}

	//
	// Setup everything.
	//

	po->StartPoint = next_prim_point;
	po->StartFace3 = next_prim_face3;
	po->StartFace4 = next_prim_face4;

	po->EndPoint = po->StartPoint + num_points;
	po->EndFace3 = po->StartFace3 + num_faces3;
	po->EndFace4 = po->StartFace4 + num_faces4;

	next_prim_point += num_points;
	next_prim_face3	+= num_faces3;
	next_prim_face4	+= num_faces4;

	//
	// Hard code the items!
	// 

	switch(prim)
	{
		case PRIM_OBJ_ITEM_HEALTH:
		case PRIM_OBJ_ITEM_GUN:
		case PRIM_OBJ_ITEM_KEY:
			po->flag |= PRIM_FLAG_ITEM;
			break;
	}

	//
	// All ok.
	//
	DebugText(" next_prim_point %d primface3 %d primface4 %d   load prim %d \n",next_prim_point,next_prim_face3,next_prim_face4,prim);

	return TRUE;

  file_error:;
	DebugText("FAILED next_prim_point %d primface3 %d primface4 %d   load prim %d \n",next_prim_point,next_prim_face3,next_prim_face4,prim);

	//
	// An error occurred.
	//

	FileClose(handle);

	return FALSE;
}


//
// Loads in all the individual prim objects.
//

void load_all_individual_prims(void)
{
	SLONG i;

	clear_prims();

	for (i = 1; i < 266; i++)
	{
		load_prim_object(i);
	}
	if(next_prim_object<266)
		next_prim_object=266;
	//
	// now load the animating objects
	//

	for (i = 1; i < 256; i++)
	{
		load_anim_prim_object(i);
	}
}






//---------------------------------------------------------
#ifndef	PSX
void	read_object_name(FILE *file_handle,CBYTE *dest_string)
{
	CBYTE		the_char	=	0;
	SLONG	count=0;


	// Read up to the first quote.
	while(the_char!='\"' && count++<100)
	{
		fscanf(file_handle,"%c",&the_char);
		
		//if(the_char)
		//	TRACE("%c",&the_char);
	}

	// Get the first character.
	fscanf(file_handle,"%c",&the_char);

	// Read until the next quote.
	count=0;
	while(the_char!='\"'&&count++<100)
	{
		*(dest_string++)	=	the_char;
		fscanf(file_handle,"%c",&the_char);
	}
	*dest_string	=	0;
}

//---------------------------------------------------------------
SLONG	key_frame_count,current_element;
SLONG	x_centre,y_centre,z_centre;

void	load_frame_numbers(CBYTE *vue_name,UWORD *frames,SLONG max_frames)
{
	CBYTE	name[200];
	SLONG	len;
#ifdef	PSX
	FILE f_handle;
#else
	FILE *f_handle;
#endif
	SLONG	result=0;
	SLONG	val,val2,index=0;

	memset((UBYTE*)frames,0,sizeof(UWORD)*max_frames);

	strcpy(name,vue_name);
	len=strlen(name);
	name[len-3]='T';
	name[len-2]='X';
	name[len-1]='T';
	LogText(" load frames >%s< \n",name);

	f_handle	=	MF_Fopen(name,"r");
	if(f_handle)
	{
		CBYTE	string[100];
		do
		{

			result	=	fscanf(f_handle,"%s",&string[0]);
			if(result>=0)
			{
				if(string[0]=='*')
				{
					//
					// load the rest of the frames
					//
						for(;val<max_frames;val++)
						{
							if(frames[val]==0)
							{
								frames[val]=index+1;  
								index++;
							}
//							else
//								ASSERT(0);

						}
				}
				else
				if(string[0]=='-')
				{
					//
					// load a range of frames
					//
					result	=	sscanf(&string[1],"%d",&val);
					result	=	fscanf(f_handle,"%d",&val2);

					if((result>=0) && (val2<max_frames) && val<val2)
					{
						for(;val<=val2;val++)
						{
							frames[val]=index+1;
							index++;

						}
					}

				}
				else
				{
					result	=	sscanf(&string[0],"%d",&val);

					if( (result>=0) && (val<max_frames) )
					{
						//
						// This records the order for frames, so if we need to know where 24 should be in the list you simply inquire at frames[24]
						//
						//
						LogText(" val %d at pos %d \n",val,index+1);
						frames[val]=index+1;
						index++;
					}
				}
			}
		}
		while(result>=0);
		MF_Fclose(f_handle);
	}
	else
	{
		LogText(" open error 1a, NO .txt for VUE\n");
		for(index=0;index<max_frames;index++)
		{
			frames[index]=index+1;
		}
	}
}

void	invert_mult(struct Matrix33 *mat,struct PrimPoint *pp)
{
	Matrix33	temp_mat;
	SLONG	i,j;
	SLONG	x,y,z;

	for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	{
		temp_mat.M[i][j]=mat->M[j][i];
	}

//	LogText(" len before %d \n",SDIST3(pp->X,pp->Y,pp->Z));

	x = (pp->X * temp_mat.M[0][0])+(pp->Y * temp_mat.M[0][1])+(pp->Z * temp_mat.M[0][2])>>15; 
	y = (pp->X * temp_mat.M[1][0])+(pp->Y * temp_mat.M[1][1])+(pp->Z * temp_mat.M[1][2])>>15; 
	z = (pp->X * temp_mat.M[2][0])+(pp->Y * temp_mat.M[2][1])+(pp->Z * temp_mat.M[2][2])>>15;
//	LogText(" len after %d \n",SDIST3(x,y,z));

	pp->X=x;
	pp->Y=y;
	pp->Z=z;
}

extern	CBYTE	*body_part_names[];
void	sort_multi_object(struct KeyFrameChunk *the_chunk)
{
	SLONG					c0,c1,c2,
							so,eo,
							sp,ep;
	struct KeyFrameElement	*the_element;
	struct PrimObject		*p_obj;
	struct	KeyFrame		*the_keyframe;
	SLONG					multi;

	//LogText(" key frame count %d \n",key_frame_count);
  /*
	for(c0=0;c0<key_frame_count;c0++)
	{
		the_keyframe=&the_chunk->KeyFrames[c0];
		the_element	=	the_keyframe->FirstElement;
//		LogText(" frame %d  dxyz (%d,%d,%d) elementcount %d\n",c0,the_keyframe->Dx,the_keyframe->Dy,the_keyframe->Dz,the_keyframe->ElementCount);
		for(c1=0;c1<the_keyframe->ElementCount;c1++,the_element++)
		{
 			the_element->OffsetX-=the_keyframe->Dx<<2;
// 			the_element->OffsetY-=the_keyframe->Dy<<2;
 			the_element->OffsetZ-=the_keyframe->Dz<<2;
			the_element->Parent=c0;
		}
	}
	*/

#ifdef	EDITOR

	for(multi=the_chunk->MultiObject;multi<=the_chunk->MultiObjectEnd;multi++)
	{
		so=prim_multi_objects[multi].StartObject;
		eo=prim_multi_objects[multi].EndObject;
		p_obj		=	&prim_objects[prim_multi_objects[multi].StartObject];

		if(the_chunk->ElementCount==15)
		{
			for(c0=so;c0<eo;c0++,p_obj++)
			{
				sp	=	p_obj->StartPoint;
				ep	=	p_obj->EndPoint;
				//
				// Look at object name to find relevant ele
				//
				for(c1=0;c1<MAX_BODY_BITS;c1++)
				{
					if(body_part_names[c1]==0)
						break;
//					if( !memcmp( body_part_names[c1],p_obj->ObjectName,strlen( body_part_names[c1] ) ) )
					if( !memcmp( body_part_names[c1],prim_names[c0],strlen( body_part_names[c1] ) ) )
					{
						the_element	=	&the_chunk->KeyFrames[0].FirstElement[c1];
						for(c2=sp;c2<ep;c2++)
						{
							prim_points[c2].X	-=	the_element->OffsetX;
							prim_points[c2].Y	-=	the_element->OffsetY;
							prim_points[c2].Z	-=	the_element->OffsetZ;
							invert_mult(&the_element->Matrix,&prim_points[c2]);
						}
						break;
					}
				}
			}
		}
		else
		{
		// old system before object names became very very important

			p_obj		=	&prim_objects[prim_multi_objects[multi].StartObject];
			the_element	=	the_chunk->KeyFrames[0].FirstElement;
			for(c0=0;c0<the_chunk->ElementCount;c0++,p_obj++,the_element++)
			{
				sp	=	p_obj->StartPoint;
				ep	=	p_obj->EndPoint;
		//		LogText("SIZE part %d    offset %d %d %d \n",c0,the_element->OffsetX,the_element->OffsetY,the_element->OffsetZ);

				for(c1=sp;c1<ep;c1++)
				{
					prim_points[c1].X	-=	the_element->OffsetX;
					prim_points[c1].Y	-=	the_element->OffsetY;
					prim_points[c1].Z	-=	the_element->OffsetZ;
					invert_mult(&the_element->Matrix,&prim_points[c1]);
				}
		//		the_element->OffsetX-=
			}
		}
	}
#endif
}

void	set_default_people_types(struct	KeyFrameChunk *the_chunk)
{
	SLONG	c0,c1;

	for(c0=0;c0<20;c0++)
	{
		strcpy(the_chunk->PeopleNames[c0],"Undefined");
		for(c1=0;c1<MAX_BODY_BITS;c1++)
		{
			the_chunk->PeopleTypes[c0].BodyPart[c1]=c1;


		}
	}
}



void	make_compress_matrix(struct KeyFrameElement	*the_element,struct Matrix33 *matrix)
{
	ULONG	encode;
	SLONG	u,v,w;
/*
	LogText(" compress>>6 %x %x %x \n",matrix->M[0][0]>>6,matrix->M[0][1]>>6,matrix->M[0][2]>>6);
	LogText(" compress>>6<<? %x %x %x \n",(((matrix->M[0][0]>>6))<<20),(((matrix->M[0][1]>>6))<<10),(((matrix->M[0][2]>>6))<<0));
	LogText(" compress>>6<<?&? %x %x %x \n",((matrix->M[0][0]>>6)<<20)&CMAT0_MASK,((matrix->M[0][1]>>6)<<10)&CMAT1_MASK,((matrix->M[0][2]>>6)<<0)&CMAT2_MASK);
	LogText(" compress %d %d %d ",matrix->M[0][0],matrix->M[0][1],matrix->M[0][2]);

	u=(((the_element->CMatrix.M[0]&CMAT0_MASK)<<2)>>22);
	v=(((the_element->CMatrix.M[0]&CMAT1_MASK)<<12)>>22);
	w=(((the_element->CMatrix.M[0]&CMAT2_MASK)<<22)>>22);

  LogText(" into %d %d %d all %x\n",u<<6,v<<6,w<<6,the_element->CMatrix.M[0]);

	LogText(" into %d \n",the_element->CMatrix.M[0]&CMAT1_MASK);
	LogText(" into<<2 %d \n",the_element->CMatrix.M[0]&CMAT1_MASK<<12);
	LogText(" into>>20 %d \n",((the_element->CMatrix.M[0]&CMAT1_MASK)<<12)>>22);
	LogText(" into>>20 %d \n",(((the_element->CMatrix.M[0]&CMAT1_MASK)<<12)>>22));
	LogText(" into>>20)<<6 %d \n",((((the_element->CMatrix.M[0]&CMAT1_MASK)<<12)>>22))<<6);
*/
	the_element->CMatrix.M[0]=((((matrix->M[0][0]>>6))<<20)&CMAT0_MASK)+((((matrix->M[0][1]>>6))<<10)&CMAT1_MASK)+((((matrix->M[0][2]>>6))<<0)&CMAT2_MASK);
	the_element->CMatrix.M[1]=((((matrix->M[1][0]>>6))<<20)&CMAT0_MASK)+((((matrix->M[1][1]>>6))<<10)&CMAT1_MASK)+((((matrix->M[1][2]>>6))<<0)&CMAT2_MASK);
	the_element->CMatrix.M[2]=((((matrix->M[2][0]>>6))<<20)&CMAT0_MASK)+((((matrix->M[2][1]>>6))<<10)&CMAT1_MASK)+((((matrix->M[2][2]>>6))<<0)&CMAT2_MASK);
}

void	normalise_max_matrix(float fe_matrix[3][3],float *x,float *y,float *z)
{
	float	len;
	SLONG	h,w;

	len=fe_matrix[0][0]*fe_matrix[0][0];
	len+=fe_matrix[0][1]*fe_matrix[0][1];
	len+=fe_matrix[0][2]*fe_matrix[0][2];

	len=sqrt(len);

	for(h=0;h<3;h++)
	{

		fe_matrix[h][0]=fe_matrix[h][0]/len;
		fe_matrix[h][1]=fe_matrix[h][1]/len;
		fe_matrix[h][2]=fe_matrix[h][2]/len;
	}


//	*x=*x/len;
//	*y=*y/len;
//	*z=*z/len;

}


//************************************************************************************************
//************************************************************************************************
//!! JCL Delete Me!


/*
SLONG	jp_total = 0;
SLONG	jpg[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

UBYTE	*jp_counts;

void	jcl_setup()
{
	jp_counts = new UBYTE[256*256*256];
	ASSERT(jp_counts);

	ZeroMemory(jp_counts, 256*256*256);
}

void	jcl_process_offset_check(SLONG x, SLONG y, SLONG z)
{
	jp_total += 3;

	SLONG	c0;

	for (c0 = 0; c0 < 16; c0 ++)
	{
		if (abs(x) > (1 << c0)) jpg[c0]++;
		if (abs(y) > (1 << c0)) jpg[c0]++;
		if (abs(z) > (1 << c0)) jpg[c0]++;
	}

	if ((abs(x) < 127) &&
		(abs(y) < 127) &&
		(abs(z) < 127))
	{
		jp_counts[((x + 128) * 256 * 256) + ((y + 128) * 256) + (z + 128)] ++;
	}
}

void	jcl_fini()
{
	SLONG	counts[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	SLONG	c0;

	for (c0 = 0; c0 < 256*256*256; c0++)
	{
		SLONG v = jp_counts[c0];
		if (v > 15)
			v = 15;

		counts[v] ++;
	}

	__asm int 3;

	delete [] jp_counts;
}*/

//************************************************************************************************
//************************************************************************************************

#ifndef PSX
#ifndef TARGET_DC

void	load_multi_vue(struct	KeyFrameChunk *the_chunk,float shrink_me)
{
	CBYTE					temp_string[512],
							transform_name[32];
	SLONG					c0,c1,
							last_frame=0,
							frame=-1,
							result;
	float					fe_matrix[3][3],
							fe_offset_x,
							fe_offset_y,
							fe_offset_z;
#ifdef	PSX
	FILE					f_handle;
#else
	FILE					*f_handle;
#endif
	struct Matrix33			temp_matrix;
	struct KeyFrame			*the_key_frame;
	struct KeyFrameElement	*the_element;
	SLONG	pivot;
	UWORD					frame_id[4501]; //more than 3000 frames, I don't think so.
	SLONG	funny_fanny=0;
	SLONG	c2;

	//!JCL delete me
//	jcl_setup();

	if(the_chunk->ElementCount!=15)
		funny_fanny=1;

	set_default_people_types(the_chunk);

	LogText("read VUE %s \n",the_chunk->VUEName);
	f_handle	=	MF_Fopen(the_chunk->VUEName,"r");
	if(f_handle)
	{

		the_chunk->FirstElement=&the_elements[current_element];
		the_chunk->KeyFrameCount	=	0;
		load_frame_numbers(the_chunk->VUEName,frame_id,4500);
		do
		{
			result	=	fscanf(f_handle,"%s",temp_string);
			if(!strcmp(temp_string,"frame"))
			{
				SLONG	read_frame;
				fscanf(f_handle,"%d",&read_frame);
				//LogText("read %d, pos %d\n",read_frame,frame_id[read_frame]);
				if(frame_id[read_frame]==0)
				{
					//skip this frame because it aint in list
//					LogText(" skipping %d \n",read_frame);

				}
				else
				{
					//frame++;

					frame=frame_id[read_frame]-1;

					if(frame==234)
					{
//						ASSERT(0);
					}

					if(frame>last_frame)
						last_frame=frame;
//					LogText(" read data into frame %d \n",frame);
					the_key_frame				=	&the_chunk->KeyFrames[frame];
					the_key_frame->ChunkID		=	0;
					the_key_frame->FrameID		=	frame;
					the_key_frame->TweenStep	=	4;
					the_key_frame->ElementCount	=	the_chunk->ElementCount;
					the_key_frame->FirstElement	=	&the_elements[current_element];
//					LogText(" Read a keyframe elecount %d \n",the_key_frame->ElementCount);
					for(c0=0;c0<the_key_frame->ElementCount;)
					{
//poo						LogText("element c0 %d out of %d \n",c0,the_key_frame->ElementCount);
						fscanf(f_handle,"%s",temp_string);	// Read the 'transform' bit.
						if(!strcmp(temp_string,"transform"))
						{
							read_object_name(f_handle,transform_name);
							for(c2=0;c2<strlen(transform_name);c2++)
							{
								transform_name[c2]=tolower(transform_name[c2]);
							}

							LogText(" object name %s \n",transform_name);
							if((!strcmp(transform_name,"lfoot"))||(!strcmp(transform_name,"pivot")))
								pivot=1;
							else
								pivot=0;

							for(c1=0;c1<the_key_frame->ElementCount;c1++)
							{
								SLONG	offset;
								if(funny_fanny)
									offset=0;
								else
									offset=2;

//								if(!memcmp(transform_name,prim_objects[prim_multi_objects[the_chunk->MultiObject].StartObject+c1].ObjectName,strlen(transform_name)-offset))
								if(!memcmp(transform_name,prim_names[prim_multi_objects[the_chunk->MultiObject].StartObject+c1],strlen(transform_name)-offset))
									break;
								//
								// find the element to assign it to
								//
							}


							fscanf	(
										f_handle,
										"%f %f %f %f %f %f %f %f %f %f %f %f",
										&fe_matrix[0][0],&fe_matrix[0][1],&fe_matrix[0][2],
										&fe_matrix[1][0],&fe_matrix[1][1],&fe_matrix[1][2],
										&fe_matrix[2][0],&fe_matrix[2][1],&fe_matrix[2][2],
										&fe_offset_x,&fe_offset_y,&fe_offset_z
									);

//							normalise_max_matrix(fe_matrix,&fe_offset_x,&fe_offset_y,&fe_offset_z);

		//this matrix has been fiddled so y=-z && z=y
							/*
							if(funny_fanny)
							{
								fe_matrix[0][0]/=18.091;
								fe_matrix[0][1]/=18.091;
								fe_matrix[0][2]/=18.091;

								fe_matrix[1][0]/=18.091;
								fe_matrix[1][1]/=18.091;
								fe_matrix[1][2]/=18.091;

								fe_matrix[2][0]/=18.091;
								fe_matrix[2][1]/=18.091;
								fe_matrix[2][2]/=18.091;
							} */

							temp_matrix.M[0][0]	=	(SLONG)(fe_matrix[0][0]*(1<<15));
							temp_matrix.M[0][2]	=	(SLONG)(fe_matrix[1][0]*(1<<15));
							temp_matrix.M[0][1]	=	(SLONG)(fe_matrix[2][0]*(1<<15)); //-ve md

							temp_matrix.M[2][0]	=	(SLONG)(fe_matrix[0][1]*(1<<15));
							temp_matrix.M[2][2]	=	(SLONG)(fe_matrix[1][1]*(1<<15));
							temp_matrix.M[2][1]	=	(SLONG)(fe_matrix[2][1]*(1<<15)); //-ve md

							temp_matrix.M[1][0]	=	(SLONG)(fe_matrix[0][2]*(1<<15));
							temp_matrix.M[1][2]	=	(SLONG)(fe_matrix[1][2]*(1<<15));
							temp_matrix.M[1][1]	=	(SLONG)(fe_matrix[2][2]*(1<<15)); //not -ve md
							{
								SLONG	dx,dy;
								for(dx=0;dx<3;dx++)
								{
									for(dy=0;dy<3;dy++)
									{
/*
										if(temp_matrix.M[dx][dy] == 32768)
										{
											temp_matrix.M[dx][dy] = 32767;
										}

										if(temp_matrix.M[dx][dy] == -32768)
										{
											temp_matrix.M[dx][dy] = -32767;
										}
*/
										SATURATE(temp_matrix.M[dx][dy],-32767,32767);


//										ASSERT(temp_matrix.M[dx][dy] <32768 && temp_matrix.M[dx][dy]>=-32768);

									}
								}
							}


							the_element	=	&the_elements[(current_element+c1)];
		//					current_element++;
							memcpy(&the_element->Matrix,&temp_matrix,sizeof(struct Matrix33));

							make_compress_matrix(the_element,&temp_matrix);

		/*
		// Guy - Added scaling to multi objects.
							the_element->OffsetX	=	(SLONG)fe_offset_x;
							the_element->OffsetY	=	-(SLONG)fe_offset_z;
							the_element->OffsetZ	=	(SLONG)fe_offset_y;
		*/
							the_element->OffsetX	=	(SLONG)((fe_offset_x*GAME_SCALE)/(GAME_SCALE_DIV*shrink_me));
							the_element->OffsetY	=	(SLONG)((fe_offset_z*GAME_SCALE)/(GAME_SCALE_DIV*shrink_me)); // -ve md
							the_element->OffsetZ	=	(SLONG)((fe_offset_y*GAME_SCALE)/(GAME_SCALE_DIV*shrink_me));
							the_element->Next		=	current_element;

							the_element->OffsetX	-=	x_centre;
							the_element->OffsetY	-=	y_centre;
							the_element->OffsetZ	-=	z_centre;

	//						the_element->OffsetX>>=2; //TWEEN_OFFSET_SHIFT;
	//						the_element->OffsetY>>=2; //TWEEN_OFFSET_SHIFT;
	//						the_element->OffsetZ>>=2; //TWEEN_OFFSET_SHIFT;

							//!jcl - delete me!
//							jcl_process_offset_check(the_element->OffsetX, the_element->OffsetZ, the_element->OffsetZ);

							if(pivot)
							{
								the_key_frame->Dx=the_element->OffsetX>>2;
								the_key_frame->Dy=the_element->OffsetY>>2;
								the_key_frame->Dz=the_element->OffsetZ>>2;
	//							LogText("PIVOT frame %d  dx %d dy %d dz %d \n",frame,the_key_frame->Dx,the_key_frame->Dy,the_key_frame->Dz);
							}

							c0++;
						}
					}
					current_element		+=	(the_key_frame->ElementCount);
					the_element->Next	=	0;
				}

			}
//			LogText(" duff >%s<\n",temp_string);

		}while(result>=0);
		MF_Fclose(f_handle);
		the_chunk->LastElement=&the_elements[current_element];
	}
	the_chunk->KeyFrameCount	=	last_frame;
	sort_multi_object(the_chunk);

	//! jcl delete me
//	jcl_fini();
}


SLONG	load_anim_mesh(CBYTE *fname,float scale)
{
	SLONG	ele_count;
#ifdef	EDITOR
extern	SLONG read_multi_asc(CBYTE *asc_name,UBYTE flag,float scale);
	ele_count=read_multi_asc(fname,0,scale);
#else
	ele_count=load_a_multi_prim(fname); //ele_count bug
#endif

	return(ele_count);

}

void	load_key_frame_chunks(KeyFrameChunk *the_chunk,CBYTE *vue_name,float scale)
{
	SLONG		c0;
	SLONG		ele_count=0;


	x_centre	=	0;
	y_centre	=	0;
	z_centre	=	0;

	the_chunk->KeyFrameCount	=	0;
	strcpy(the_chunk->VUEName,"Data\\");
	strcat(the_chunk->VUEName,vue_name);
	strcpy(the_chunk->ASCName,the_chunk->VUEName);
	strcpy(the_chunk->ANMName,the_chunk->VUEName);
	c0=0;
	while(the_chunk->ASCName[c0]!='.' && the_chunk->ASCName[c0]!=0)c0++;
	if(the_chunk->ASCName[c0]=='.')
	{
		the_chunk->ASCName[c0+1]	=	'A';
		the_chunk->ASCName[c0+2]	=	'S';
		the_chunk->ASCName[c0+3]	=	'C';
		the_chunk->ASCName[c0+4]	=	0;

		the_chunk->ANMName[c0+1]	=	'A';
		the_chunk->ANMName[c0+2]	=	'N';
		the_chunk->ANMName[c0+3]	=	'M';
		the_chunk->ANMName[c0+4]	=	0;
	}
	the_chunk->MultiObject	=	next_prim_multi_object;
	ele_count=load_anim_mesh(the_chunk->ASCName,scale);
	if(ele_count)
	{
		SLONG	ret=1,count=0;

		while(ret)
		{
			SLONG	in;

			in='1'+count;
			the_chunk->ASCName[5]=in;
			ret=load_anim_mesh(the_chunk->ASCName,scale);
			count++;
		}
	}
	the_chunk->MultiObjectEnd	=	next_prim_multi_object-1;
	the_chunk->MultiObjectStart	=	the_chunk->MultiObject;
	if(ele_count)
	{
//		if(ele_count>15)
//			ele_count=15;
//md change
//		the_chunk->ElementCount	=	prim_multi_objects[the_chunk->MultiObject].EndObject-prim_multi_objects[the_chunk->MultiObject].StartObject;
		the_chunk->ElementCount	=	ele_count; //prim_multi_objects[the_chunk->MultiObject].EndObject-prim_multi_objects[the_chunk->MultiObject].StartObject;
		LogText(" element count %d \n",the_chunk->ElementCount);

				// Fudgy bit for centering.
		{
			SLONG	multi;
extern SLONG		x_centre,
			y_centre,
			z_centre;
SLONG				c1,
			sp,ep;
struct PrimObject	*p_obj;

			LogText("SIZE edutils   center %d %d %d \n",x_centre,y_centre,z_centre);

			
			for(multi=the_chunk->MultiObjectStart;multi<=the_chunk->MultiObjectEnd;multi++)
			for(c0=prim_multi_objects[multi].StartObject;c0<prim_multi_objects[multi].EndObject;c0++)
			{
				p_obj   =	&prim_objects[c0];
				sp		=	p_obj->StartPoint;
				ep		=	p_obj->EndPoint;

				for(c1=sp;c1<ep;c1++)
				{
					prim_points[c1].X	-=	x_centre;
					prim_points[c1].Y	-=	y_centre;
					prim_points[c1].Z	-=	z_centre;
				}
			}
		}				

		load_multi_vue(the_chunk,scale);
#ifdef	EDITOR
extern	void	load_chunk_texture_info(KeyFrameChunk *the_chunk);
		load_chunk_texture_info(the_chunk);
#endif
	}
}
#endif
#endif

/*
fucked one
LCTI obj 122 has f4 6 f3 31 
LCTI obj 123 has f4 0 f3 16 
LCTI obj 124 has f4 2 f3 8 
LCTI obj 125 has f4 3 f3 6 
LCTI obj 126 has f4 17 f3 77 
LCTI obj 127 has f4 1 f3 11 
LCTI obj 128 has f4 1 f3 14 
LCTI obj 129 has f4 4 f3 6 
LCTI obj 130 has f4 2 f3 9 
LCTI obj 131 has f4 2 f3 12 
LCTI obj 132 has f4 3 f3 8 
LCTI obj 133 has f4 7 f3 20 
LCTI obj 134 has f4 1 f3 14 
LCTI obj 135 has f4 2 f3 8 
LCTI obj 136 has f4 3 f3 6 
LCTI obj 137 has f4 0 f3 0 


fucked one
LCTI obj 122 has f4 6 f3 31 
LCTI obj 123 has f4 0 f3 16 
LCTI obj 124 has f4 2 f3 8 
LCTI obj 125 has f4 3 f3 6 
LCTI obj 126 has f4 17 f3 77 
LCTI obj 127 has f4 1 f3 11 
LCTI obj 128 has f4 1 f3 14 
LCTI obj 129 has f4 4 f3 6 
LCTI obj 130 has f4 2 f3 9 
LCTI obj 131 has f4 2 f3 12 
LCTI obj 132 has f4 3 f3 8 
LCTI obj 133 has f4 7 f3 20 
LCTI obj 134 has f4 1 f3 14 
LCTI obj 135 has f4 2 f3 8 
LCTI obj 136 has f4 3 f3 6 
LCTI obj 137 has f4 0 f3 0 

fucked but right sex 
LCTI obj 122 has f4 6 f3 31 
LCTI obj 123 has f4 0 f3 16 
LCTI obj 124 has f4 2 f3 8 
LCTI obj 125 has f4 3 f3 6 
LCTI obj 126 has f4 17 f3 77 
LCTI obj 127 has f4 1 f3 11 
LCTI obj 128 has f4 1 f3 14 
LCTI obj 129 has f4 4 f3 6 
LCTI obj 130 has f4 2 f3 9 
LCTI obj 131 has f4 2 f3 12 
LCTI obj 132 has f4 3 f3 8 
LCTI obj 133 has f4 7 f3 20 
LCTI obj 134 has f4 1 f3 14 
LCTI obj 135 has f4 2 f3 8 
LCTI obj 136 has f4 3 f3 6 
LCTI obj 137 has f4 0 f3 0 

right but wrong sex
LCTI obj 122 has f4 6 f3 31 
LCTI obj 123 has f4 0 f3 16 
LCTI obj 124 has f4 1 f3 10 
LCTI obj 125 has f4 3 f3 6 
LCTI obj 126 has f4 18 f3 75 
LCTI obj 127 has f4 0 f3 13 
LCTI obj 128 has f4 0 f3 16 
LCTI obj 129 has f4 4 f3 6 
LCTI obj 130 has f4 2 f3 9 
LCTI obj 131 has f4 2 f3 12 
LCTI obj 132 has f4 3 f3 8 
LCTI obj 133 has f4 5 f3 24 
LCTI obj 134 has f4 1 f3 14 
LCTI obj 135 has f4 2 f3 8 
LCTI obj 136 has f4 3 f3 6 
LCTI obj 137 has f4 0 f3 0 

*/
#endif
#ifndef PSX
#ifndef TARGET_DC

void	read_a_prim(SLONG prim,MFFileHandle	handle,SLONG	save_type)
{
	SLONG	c0;
	SLONG	sf3,ef3,sf4,ef4,sp,ep;
	SLONG	dp;


	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&prim_names[next_prim_object],32);//sizeof(prim_objects[0].ObjectName));

//		LogText(" name %s becomes prim %d\n",prim_objects[next_prim_object].ObjectName,next_prim_object);

		FileRead(handle,(UBYTE*)&sp,sizeof(sp));
		FileRead(handle,(UBYTE*)&ep,sizeof(ep));

		LogText(" no points %d \n",ep-sp);


		for(c0=sp;c0<ep;c0++)
		{
			if(save_type>3)
			{
				FileRead(handle,(UBYTE*)&prim_points[next_prim_point+c0-sp],sizeof(struct PrimPoint));
			}
			else
			{
				struct	OldPrimPoint	pp;
				FileRead(handle,(UBYTE*)&pp,sizeof(struct OldPrimPoint));

				prim_points[next_prim_point+c0-sp].X=(SWORD)pp.X;
				prim_points[next_prim_point+c0-sp].Y=(SWORD)pp.Y;
				prim_points[next_prim_point+c0-sp].Z=(SWORD)pp.Z;

			}
		}
		

		dp=next_prim_point-sp;// was 10 but is now 50 so need to add 40 to all point indexs

		FileRead(handle,(UBYTE*)&sf3,sizeof(sf3));
		FileRead(handle,(UBYTE*)&ef3,sizeof(ef3));
		LogText(" no face3 %d \n",ef3-sf3);
		for(c0=sf3;c0<ef3;c0++)
		{
			FileRead(handle,(UBYTE*)&prim_faces3[next_prim_face3+c0-sf3],sizeof(struct PrimFace3));
			prim_faces3[next_prim_face3+c0-sf3].Points[0]+=dp;
			prim_faces3[next_prim_face3+c0-sf3].Points[1]+=dp;
			prim_faces3[next_prim_face3+c0-sf3].Points[2]+=dp;
		}


		FileRead(handle,(UBYTE*)&sf4,sizeof(sf4));
		FileRead(handle,(UBYTE*)&ef4,sizeof(ef4));
		LogText(" no face4 %d \n",ef4-sf4);
		for(c0=sf4;c0<ef4;c0++)
		{
			FileRead(handle,(UBYTE*)&prim_faces4[next_prim_face4+c0-sf4],sizeof(struct PrimFace4));
			prim_faces4[next_prim_face4+c0-sf4].Points[0]+=dp;
			prim_faces4[next_prim_face4+c0-sf4].Points[1]+=dp;
			prim_faces4[next_prim_face4+c0-sf4].Points[2]+=dp;
			prim_faces4[next_prim_face4+c0-sf4].Points[3]+=dp;

			prim_faces4[next_prim_face4+c0-sf4].FaceFlags &= ~FACE_FLAG_WALKABLE;
		}

		prim_objects[next_prim_object].StartPoint=next_prim_point;
		prim_objects[next_prim_object].EndPoint=next_prim_point+ep-sp;

		prim_objects[next_prim_object].StartFace3=next_prim_face3;
		prim_objects[next_prim_object].EndFace3=next_prim_face3+ef3-sf3;

		prim_objects[next_prim_object].StartFace4=next_prim_face4;
		prim_objects[next_prim_object].EndFace4=next_prim_face4+ef4-sf4;
	
		next_prim_point+=ep-sp;
		next_prim_face3+=ef3-sf3;
		next_prim_face4+=ef4-sf4;

		next_prim_object++;
	}

}
#endif
#endif
#ifndef	PSX
#ifndef TARGET_DC

//extern	struct	PrimMultiObject	prim_multi_objects[];

SLONG	load_a_multi_prim(CBYTE *name)
{
	SLONG			c0;
	MFFileHandle	handle;
	SLONG			save_type=0;
	SLONG			so,eo;
	CBYTE			ext_name[80];

	change_extension(name,"moj",ext_name);
//	sprintf(ext_name,"data/%s",ext_name);
	handle	=	FileOpen(ext_name);
	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

		FileRead(handle,(UBYTE*)&so,sizeof(so));
		FileRead(handle,(UBYTE*)&eo,sizeof(eo));

		prim_multi_objects[next_prim_multi_object].StartObject=next_prim_object;
		prim_multi_objects[next_prim_multi_object].EndObject=next_prim_object+(eo-so);
		LogText(" load multi prim  no object %d \n",eo-so);
		for(c0=so;c0<eo;c0++)
			read_a_prim(c0,handle,save_type);

		FileClose(handle);
		next_prim_multi_object++;
		return(next_prim_multi_object-1);
	}
	else
		return(0);
}




SLONG	find_matching_face(struct	PrimPoint	*p1,struct	PrimPoint	*p2,struct	PrimPoint	*p3,UWORD prim)
{
	SLONG	c0,sf,ef,point;
	sf=prim_objects[prim].StartFace4;
	ef=prim_objects[prim].EndFace4;

	for(c0=sf;c0<=ef;c0++)
	{
		if(prim_points[prim_faces4[c0].Points[0]].X==p1->X&&
		   prim_points[prim_faces4[c0].Points[0]].Y==p1->Y&&
		   prim_points[prim_faces4[c0].Points[0]].Z==p1->Z&&
		   prim_points[prim_faces4[c0].Points[1]].X==p2->X&&
		   prim_points[prim_faces4[c0].Points[1]].Y==p2->Y&&
		   prim_points[prim_faces4[c0].Points[1]].Z==p2->Z&&
		   prim_points[prim_faces4[c0].Points[2]].X==p3->X&&
		   prim_points[prim_faces4[c0].Points[2]].Y==p3->Y&&
		   prim_points[prim_faces4[c0].Points[2]].Z==p3->Z)
		   {
				return(c0);
		   }
	}
	return(-1);
}

extern	void	add_point(SLONG x,SLONG y,SLONG z);
extern	struct	PrimFace4*	create_a_quad(UWORD p1,UWORD p0,UWORD p3,UWORD p2,SWORD	texture_style,SWORD texture_piece);
extern	SLONG	build_prim_object(SLONG sp,SLONG sf3,SLONG sf4);
extern	void save_prim_asc(UWORD prim,UWORD version);

void	create_kline_bottle(void)
{
	float	x,y,z,u,v;
	float	sqrt_2,a=1.0; //what the fuck should a be
	struct	PrimFace4	*p_f4;

	float	step=PI/10.0; // low poly version

	return; //switch it off for now

/*
	SLONG	sp[10000],count=0,inner_count=0;
	SLONG	c0,c1;
	SLONG	sf3,sf4,stp;

	stp=next_prim_point;
	sf3=next_prim_face3;
	sf4=next_prim_face4;

	sqrt_2=sqrt(2.0);


	for(u=-1.0*PI;u<(1.0*PI)+(step/2.0);u+=step,count++)
	{
		sp[count]=next_prim_point;
		for(v=-1.0*PI;v<(1.0*PI)+(step/2.0);v+=step)
		{
//			x = cos(u)*(cos(u/2.0)*(sqrt_2+cos(v))+(sin(u/2.0)*sin(v)*cos(v)));
//			y = sin(u)*(cos(u/2.0)*(sqrt_2+cos(v))+(sin(u/2.0)*sin(v)*cos(v)));
//			z = -1.0*sin(u/2.0)*(sqrt_2+cos(v))+cos(u/2.0)*sin(v)*cos(v);

			  x = (a+cos(u/2.0)*sin(v)-sin(u/2.0)*sin(2.0*v))*cos(u);

			  y = (a+cos(u/2.0)*sin(v)-sin(u/2.0)*sin(2.0*v))*sin(u);

			  z = sin(u/2.0)*sin(v)+cos(u/2.0)*sin(2.0*v);


			add_point((SLONG)(x*200.0),(SLONG)(y*200.0),(SLONG)(z*200.0));
		}
	}

	for(c0=0;c0<count-1;c0++)
	for(c1=0;c1<count-1;c1++)
	{
//		if((c0+c1)&1)
		{
			p_f4=create_a_quad(sp[c0]+c1,sp[c0]+c1+1,sp[c0+1]+c1,sp[c0+1]+c1+1,0,0);
			p_f4->DrawFlags=1+(1<<6); //POLY_G; gourad
			p_f4->Col=((c0+c1)&1)*50+50;
		}
	}
	
	build_prim_object(stp,sf3,sf4);
	save_prim_asc(next_prim_object-1,0);
	*/
}

void load_palette(CBYTE *palette)
{
#ifdef	PSX
	FILE					handle;
#else
	FILE					*handle;
#endif

	handle = MF_Fopen(palette, "rb");

#ifdef	EDITOR
extern	UBYTE	*pals[];
	pals[0]=(UBYTE*)ENGINE_palette;
#endif

	if (handle == NULL)
	{
		TRACE("Could not open palette file.\n");

		goto file_error;
	}

	if (fread(ENGINE_palette, 1, sizeof(ENGINE_palette), handle) != sizeof(ENGINE_palette))
	{
		TRACE("Error reading palette file.\n");

		MF_Fclose(handle);

		goto file_error;
	}

	MF_Fclose(handle);

	return;

  file_error:;

	SLONG i;

	for (i = 0; i < 256; i++)
	{
		ENGINE_palette[i].red   = rand();
		ENGINE_palette[i].green = rand();
		ENGINE_palette[i].blue  = rand();
	}
}



//
// no meshes
//

// mesh

// 

extern	void	write_a_prim(SLONG prim,MFFileHandle	handle);

SLONG	save_insert_a_multi_prim(MFFileHandle	handle,SLONG multi)
{
	SLONG			c0,point;
	CBYTE			file_name[64];
	SLONG			save_type=4;
	SLONG			so,eo;
	CBYTE			ext_name[80];
#ifdef	EDITOR
	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileWrite(handle,(UBYTE*)&save_type,sizeof(save_type));

		so=prim_multi_objects[multi].StartObject;
		eo=prim_multi_objects[multi].EndObject;

		FileWrite(handle,(UBYTE*)&so,sizeof(so));
		FileWrite(handle,(UBYTE*)&eo,sizeof(eo));

		for(c0=so;c0<eo;c0++)
			write_a_prim(c0,handle);

		return(1);
	}
#endif
	return(0);
	
}

SLONG	save_insert_game_chunk(MFFileHandle	handle,struct GameKeyFrameChunk *p_chunk)
{
	SLONG	save_type=5;
	SLONG	temp;
	UWORD	check=666;
#ifdef	EDITOR

	FileWrite(handle,(UBYTE*)&save_type,sizeof(save_type));

	FileWrite(handle,(UBYTE*)&p_chunk->ElementCount,sizeof(p_chunk->ElementCount));

	FileWrite(handle,(UBYTE*)&p_chunk->MaxPeopleTypes,sizeof(p_chunk->MaxPeopleTypes));
	FileWrite(handle,(UBYTE*)&p_chunk->MaxAnimFrames,sizeof(p_chunk->MaxAnimFrames));
	FileWrite(handle,(UBYTE*)&p_chunk->MaxElements,sizeof(p_chunk->MaxElements));
	FileWrite(handle,(UBYTE*)&p_chunk->MaxKeyFrames,sizeof(p_chunk->MaxKeyFrames));
	FileWrite(handle,(UBYTE*)&p_chunk->MaxFightCols,sizeof(p_chunk->MaxFightCols));

//	FileWrite(handle,(UBYTE*)&p_chunk->TweakSpeeds[0],p_chunk->MaxAnimFrames);
//	FileWrite(handle,(UBYTE*)&check,2);

	//
	// save the people types
	//
//	temp=MAX_PEOPLE_TYPES;
//	FileWrite(handle,(UBYTE*)&temp,sizeof(temp));
	FileWrite(handle,(UBYTE*)&p_chunk->PeopleTypes[0],sizeof(struct BodyDef)*p_chunk->MaxPeopleTypes);
	FileWrite(handle,(UBYTE*)&check,2);
	//
	// save the keyframe linked lists
	//
	temp=(SLONG)&p_chunk->AnimKeyFrames[0];
	FileWrite(handle,(UBYTE*)&temp,sizeof(temp));
	FileWrite(handle,(UBYTE*)&p_chunk->AnimKeyFrames[0],sizeof(struct GameKeyFrame)*p_chunk->MaxKeyFrames);
	FileWrite(handle,(UBYTE*)&check,2);

	//
	// save the anim elements
	//
	temp=(SLONG)&p_chunk->TheElements[0];
	FileWrite(handle,(UBYTE*)&temp,sizeof(temp));
	FileWrite(handle,(UBYTE*)&p_chunk->TheElements[0],sizeof(struct GameKeyFrameElement)*p_chunk->MaxElements);
	check=666;
	FileWrite(handle,(UBYTE*)&check,2);

	//
	// save the animlist
	//
//	temp=200;
//	FileWrite(handle,(UBYTE*)&temp,sizeof(temp));
	FileWrite(handle,(UBYTE*)&p_chunk->AnimList[0],sizeof(struct GameKeyFrame*)*p_chunk->MaxAnimFrames);
	check=666;
	FileWrite(handle,(UBYTE*)&check,2);
	check=666;

//	if(p_chunk->AnimKeyFrames[30])
//		if(p_chunk->AnimKeyFrames[30].Fight)
//			DebugText(" animkeyframes[30].fight %x  next %x \n",p_chunk->AnimKeyFrames[30].Fight,p_chunk->AnimKeyFrames[30].Fight->Next);

	temp=(SLONG)&p_chunk->FightCols[0];
	FileWrite(handle,(UBYTE*)&temp,sizeof(temp));

//	temp=p_chunk->MaxFightCols;
//	FileWrite(handle,(UBYTE*)&p_chunk->MaxFightCols,sizeof(p_chunk->MaxFightCols));
	FileWrite(handle,(UBYTE*)&p_chunk->FightCols[0],sizeof(struct GameFightCol)*p_chunk->MaxFightCols);
	FileWrite(handle,(UBYTE*)&check,2);

#endif
	return(1);

}

SLONG	save_anim_system(struct GameKeyFrameChunk *p_chunk,CBYTE	*name)
{
	SLONG			c0,point;
	MFFileHandle	handle;
	CBYTE			file_name[64];
	SLONG			save_type=5;
	SLONG			so,eo;
	CBYTE			ext_name[80];
	SLONG			count;
#ifdef	EDITOR

	change_extension(name,"all",ext_name);
	handle	=	FileCreate(ext_name,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileWrite(handle,(UBYTE*)&save_type,sizeof(save_type));

		count=0;
		while(p_chunk->MultiObject[count])
		{
			count++;
		}

		FileWrite(handle,(UBYTE*)&count,sizeof(count));

		count=0;
		while(p_chunk->MultiObject[count])
		{
			save_insert_a_multi_prim(handle,p_chunk->MultiObject[count]);
			count++;
		}
		save_insert_game_chunk(handle,p_chunk);

		FileClose(handle);
		return(1);
	}
#endif
	return(0);
	
}
#endif
#endif
#ifndef TARGET_DC
#ifndef PSX
SLONG	load_insert_game_chunk(MFFileHandle	handle,struct GameKeyFrameChunk *p_chunk)
{
	SLONG	save_type=0,c0;
	SLONG	temp;
	ULONG	addr1,addr2,a_off,ae_off;
	ULONG	af_off,addr3;
	UWORD	check;

	struct GameKeyFrameElementBig *temp_mem;

	

	FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

	ASSERT(save_type>1);
	

	FileRead(handle,(UBYTE*)&p_chunk->ElementCount,sizeof(p_chunk->ElementCount));

	FileRead(handle,(UBYTE*)&p_chunk->MaxPeopleTypes,sizeof(p_chunk->MaxPeopleTypes));
	FileRead(handle,(UBYTE*)&p_chunk->MaxAnimFrames,sizeof(p_chunk->MaxAnimFrames));
	FileRead(handle,(UBYTE*)&p_chunk->MaxElements,sizeof(p_chunk->MaxElements));
	FileRead(handle,(UBYTE*)&p_chunk->MaxKeyFrames,sizeof(p_chunk->MaxKeyFrames));
	FileRead(handle,(UBYTE*)&p_chunk->MaxFightCols,sizeof(p_chunk->MaxFightCols));

	if(p_chunk->MaxPeopleTypes)
		p_chunk->PeopleTypes=(struct BodyDef*)MemAlloc(MAX_PEOPLE_TYPES*sizeof(struct BodyDef));

	if(p_chunk->MaxKeyFrames)
		p_chunk->AnimKeyFrames=(struct GameKeyFrame*)MemAlloc((p_chunk->MaxKeyFrames+500)*sizeof(struct GameKeyFrame));

	if(p_chunk->MaxAnimFrames)
		p_chunk->AnimList=(struct GameKeyFrame**)MemAlloc((p_chunk->MaxAnimFrames+200)*sizeof(struct GameKeyFrame *));

	if(p_chunk->MaxElements)
		p_chunk->TheElements=(struct GameKeyFrameElement*)MemAlloc((p_chunk->MaxElements+500*15)*sizeof(struct GameKeyFrameElement));

	temp_mem=(struct GameKeyFrameElementBig*)MemAlloc(p_chunk->MaxElements*sizeof(struct GameKeyFrameElementBig));

	if(p_chunk->MaxFightCols)
		p_chunk->FightCols=(struct GameFightCol*)MemAlloc((p_chunk->MaxFightCols+50)*sizeof(struct GameFightCol));

//	if(p_chunk->MaxAnimFrames)
//		p_chunk->TweakSpeeds=(UBYTE*)MemAlloc(p_chunk->MaxAnimFrames);

//	if(save_type>=3)
//	{
//		FileRead(handle,(UBYTE*)&p_chunk->TweakSpeeds[0],p_chunk->MaxAnimFrames);
//		FileRead(handle,&check,2);
//		ASSERT(check==666);
//
//	}

	//
	// Load the people types
	//
	//FileRead(handle,(UBYTE*)&temp,sizeof(temp));
	FileRead(handle,(UBYTE*)&p_chunk->PeopleTypes[0],sizeof(struct BodyDef)*p_chunk->MaxPeopleTypes);
	FileRead(handle,&check,2);
	ASSERT(check==666);
	//
	// Load the keyframe linked lists
	//
	FileRead(handle,(UBYTE*)&addr1,sizeof(addr1));
	FileRead(handle,(UBYTE*)&p_chunk->AnimKeyFrames[0],sizeof(struct GameKeyFrame)*p_chunk->MaxKeyFrames);
	FileRead(handle,&check,2);
	ASSERT(check==666);

	//
	// Load the anim elements
	//

#ifndef ULTRA_COMPRESSED_ANIMATIONS
	FileRead(handle,(UBYTE*)&addr2,sizeof(addr2));
	FileRead(handle,(UBYTE*)&p_chunk->TheElements[0],sizeof(struct GameKeyFrameElement)*p_chunk->MaxElements);
	FileRead(handle,&check,2);
	ASSERT(check==666);
#else




//#error  // need to load in a different file, or bodge the sizeof command above.
	FileRead(handle,(UBYTE*)&addr2,sizeof(addr2));
	//JCL - convert compressed to ultra compressed.
	for (c0 = 0; c0 < p_chunk->MaxElements; c0++)
	{


		FileRead(handle,(UBYTE*)&temp_mem[c0],sizeof(struct GameKeyFrameElementBig));
		CMatrix33 c = *((CMatrix33 *)(&temp_mem[c0])); //!er....
		SetCMatrix(&p_chunk->TheElements[c0], &c);
		p_chunk->TheElements[c0].OffsetX=(temp_mem[c0].OffsetX)&0xff;
		p_chunk->TheElements[c0].OffsetY=(temp_mem[c0].OffsetY)&0xff;
		p_chunk->TheElements[c0].OffsetZ=(temp_mem[c0].OffsetZ)&0xff;

	}	
	FileRead(handle,&check,2);
	ASSERT(check==666);



#endif



	//
	// Load the animlist
	//
//	FileRead(handle,(UBYTE*)&temp,sizeof(temp));
	FileRead(handle,(UBYTE*)&p_chunk->AnimList[0],sizeof(struct GameKeyFrame *)*p_chunk->MaxAnimFrames);
	FileRead(handle,&check,2);
	ASSERT(check==666);

	FileRead(handle,(UBYTE*)&addr3,sizeof(addr3));

	FileRead(handle,(UBYTE*)&p_chunk->FightCols[0],sizeof(struct GameFightCol)*p_chunk->MaxFightCols);
	FileRead(handle,&check,2);
	ASSERT(check==666);

	if(save_type<3)
	{
		SLONG	c0;
		//
		// convert anim speeds to step sizes rather than step counts
		//

		for(c0=0;c0<p_chunk->MaxKeyFrames;c0++)
		{
			p_chunk->AnimKeyFrames[c0].TweenStep=(256/(p_chunk->AnimKeyFrames[c0].TweenStep+1))>>1;

		}


	}


	if(save_type>4)
	{
extern	void	convert_keyframe_to_pointer(GameKeyFrame *p,GameKeyFrameElement *p_ele,GameFightCol *p_fight,SLONG count);
extern	void	convert_animlist_to_pointer(GameKeyFrame **p,GameKeyFrame *p_anim,SLONG count);
extern	void	convert_fightcol_to_pointer(GameFightCol *p,GameFightCol *p_fight,SLONG count);

  		convert_keyframe_to_pointer(p_chunk->AnimKeyFrames,p_chunk->TheElements,p_chunk->FightCols,p_chunk->MaxKeyFrames);
		convert_animlist_to_pointer(p_chunk->AnimList,p_chunk->AnimKeyFrames,p_chunk->MaxAnimFrames);
		convert_fightcol_to_pointer(p_chunk->FightCols,p_chunk->FightCols,p_chunk->MaxFightCols);
	}
	else
	{


		LogText("PSX1 game chunk  max animkeyframes %d max theelements %d max animlist %d\n",p_chunk->MaxKeyFrames,p_chunk->MaxElements,temp);

		// was at 100 now at 10, a_off =-90 so we take 90 off each stored address

		a_off=((ULONG)&p_chunk->AnimKeyFrames[0])-addr1;
		ae_off=((ULONG)&p_chunk->TheElements[0])-addr2;
		af_off=((ULONG)&p_chunk->FightCols[0])-addr3;
		for(c0=0;c0<p_chunk->MaxKeyFrames;c0++)
		{
			ULONG	a;

			a=(ULONG)p_chunk->AnimKeyFrames[c0].NextFrame;
			a+=a_off;
			if(p_chunk->AnimKeyFrames[c0].NextFrame)
				p_chunk->AnimKeyFrames[c0].NextFrame=(struct GameKeyFrame*)a;

			a=(ULONG)p_chunk->AnimKeyFrames[c0].PrevFrame;
			a+=a_off;
			if(p_chunk->AnimKeyFrames[c0].PrevFrame)
				p_chunk->AnimKeyFrames[c0].PrevFrame=(struct GameKeyFrame*)a;

	//		p_chunk->AnimKeyFrames[c0].Fight=0;

			a=(ULONG)p_chunk->AnimKeyFrames[c0].FirstElement;
			a+=ae_off;
			p_chunk->AnimKeyFrames[c0].FirstElement=(struct GameKeyFrameElement*)a;
			DebugText(" fight %x addr3 %x \n",p_chunk->AnimKeyFrames[c0].Fight,addr3);

			a=(ULONG)p_chunk->AnimKeyFrames[c0].Fight;
			if( a!=0&& a<(ULONG)addr3)
			{
				DebugText(" fight %x <addr3 %x abort\n",p_chunk->AnimKeyFrames[c0].Fight,addr3);
				a=0;
				p_chunk->AnimKeyFrames[c0].Fight=0;
			}
			a+=af_off;

			if(p_chunk->AnimKeyFrames[c0].Fight)
			{
				struct	GameFightCol	*p_fight;
				ULONG	offset;
				DebugText(" fight on animkeyframe %d \n",c0);

				offset=(ULONG)p_chunk->AnimKeyFrames[c0].Fight;
				offset-=(ULONG)addr3;
				offset/=sizeof(struct GameFightCol);
				DebugText(" p_chunk->AnimKeyFrames[%d].Fight %x   offset %d fixed \n",c0,p_chunk->AnimKeyFrames[c0].Fight,offset);


				p_chunk->AnimKeyFrames[c0].Fight=(struct GameFightCol*)a;

				p_fight=(struct GameFightCol*)a;
				p_fight->Next=0;

				while(p_fight->Next)
				{
					offset=(ULONG)p_fight->Next;
					offset-=(ULONG)addr3;
					offset/=sizeof(struct GameFightCol);
					DebugText(" p_fight->Next %x   offset %d fixed \n",p_fight->Next,offset);
					if(offset>p_chunk->MaxFightCols)
					{
						DebugText(" error error offset>max %d \n",p_chunk->MaxFightCols);
						p_fight->Next=0;
					}
					else
					{
						a=(ULONG)p_fight->Next;
						a+=af_off;
						p_fight->Next=0; //(struct GameFightCol*)a;
						p_fight=p_fight->Next;
					}
				}
			}
		}

		
		a_off=((ULONG)&p_chunk->AnimKeyFrames[0])-addr1;
		for(c0=0;c0<p_chunk->MaxAnimFrames;c0++)
		{
			ULONG	a;

			a=(ULONG)p_chunk->AnimList[c0];
			a+=a_off;
			p_chunk->AnimList[c0]=(struct GameKeyFrame*)a;
		}
	}

	MemFree((void*)temp_mem);
	return(1);



}

SLONG	load_insert_a_multi_prim(MFFileHandle	handle)
{
	SLONG			c0;
	SLONG			save_type=0;
	SLONG			so,eo;
//	CBYTE			ext_name[80];

	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

		FileRead(handle,(UBYTE*)&so,sizeof(so));
		FileRead(handle,(UBYTE*)&eo,sizeof(eo));

		prim_multi_objects[next_prim_multi_object].StartObject=next_prim_object;
		prim_multi_objects[next_prim_multi_object].EndObject=next_prim_object+(eo-so);
		LogText(" load multi prim  no object %d \n",eo-so);
		for(c0=so;c0<eo;c0++)
			read_a_prim(c0,handle,save_type);

		next_prim_multi_object++;
		return(next_prim_multi_object-1);
	}
	else
		return(0);
}

extern	ULONG	DONT_load;
SLONG	load_anim_system(struct GameKeyFrameChunk *p_chunk,CBYTE	*name,SLONG type)
{
	SLONG			c0,point;
	MFFileHandle	handle;
//	CBYTE			file_name[64];
	SLONG			save_type=0;
//	SLONG			so,eo;
	CBYTE			ext_name[100];
	SLONG			count;
	UBYTE	load;

	CBYTE	fname[100];

	sprintf(fname,"%sdata\\%s",DATA_DIR,name);

extern	void	free_game_chunk(GameKeyFrameChunk *the_chunk);
	free_game_chunk(p_chunk);

	DebugText(" load chunk name %s\n",fname);
	change_extension(fname,"all",ext_name);
	handle	=	FileOpen(ext_name);
	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

		if(save_type>2)
		{

			FileRead(handle,(UBYTE*)&count,sizeof(count));

			for(c0=0;c0<count;c0++)
			{
				switch(type)
				{
					case	0:
						p_chunk->MultiObject[c0]=load_insert_a_multi_prim(handle);
						break;
					case	1:
						// darci 
						switch(c0)
						{
							case	0: //darci
								load=1;
								break;
							case	1: //slag
								load=!(DONT_load&(1<<PERSON_SLAG_TART));
								break;
							case	2: //fat slag
								load=!(DONT_load&(1<<PERSON_SLAG_FATUGLY));
								break;
							case	3: //hostage
								load=!(DONT_load&(1<<PERSON_HOSTAGE));
								break;
							default:
								load=1;
								break;
						}
						if(load)
							p_chunk->MultiObject[c0]=load_insert_a_multi_prim(handle);
						else
						{
							skip_load_a_multi_prim(handle);
							p_chunk->MultiObject[c0]=1;
						}
						break;
					case	2:
						//thug   // 0 rasta 1 grey 2 red 3 miller 4 cop 5 mib 6 tramp 7 civ variety
						
						switch(c0)
						{
							case	0: //rasta
							case	1: //grey
							case	2: //red
							case	4: //cop
							case	7: //civ variety
								load=1;
								break;
							case	3: //miller (mechanic)
								load=!(DONT_load&(1<<PERSON_MECHANIC));
								break;
							case	5: //MIB
								load=!(DONT_load&(1<<PERSON_MIB1));
								break;
							case	6: //tramp
								load=!(DONT_load&(1<<PERSON_TRAMP));
								break;
							default:
								load=1;
								break;
						}
						if(load)
							p_chunk->MultiObject[c0]=load_insert_a_multi_prim(handle);
						else
						{
							skip_load_a_multi_prim(handle);
							p_chunk->MultiObject[c0]=1;
						}
						break;

				}
				DebugText(" next_prim_point %d primface3 %d primface4 %d   load ANIMSYSTEM part %d \n",next_prim_point,next_prim_face3,next_prim_face4,c0);
			}
			p_chunk->MultiObject[c0]=0;
		}
		else
		{
			p_chunk->MultiObject[0]=load_insert_a_multi_prim(handle);
			p_chunk->MultiObject[1]=0;
			DebugText(" next_prim_point %d primface3 %d primface4 %d   load ANIMSYSTEM partb %d \n",next_prim_point,next_prim_face3,next_prim_face4,0);
		}
		load_insert_game_chunk(handle,p_chunk);

		FileClose(handle);
		return(1);
	}
	else
	{
		DebugText("\n ERROR failed to open \n");
	}
	return(0);
	
}
SLONG	load_append_game_chunk(MFFileHandle	handle,struct GameKeyFrameChunk *p_chunk,SLONG start_frame)
{
	SLONG	save_type=0,c0;
	SLONG	temp;
	ULONG	addr1,addr2,a_off,ae_off;
	ULONG	af_off,addr3;
	UWORD	check;

	struct GameKeyFrameElementBig *temp_mem;

	SLONG	ElementCount;
	SWORD	MaxPeopleTypes;
	SWORD	MaxKeyFrames;
	SWORD	MaxAnimFrames;
	SWORD	MaxFightCols;
	SLONG	MaxElements;

	ULONG	error;
	

	FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

	ASSERT(save_type>1);
	

	//
	//
	//
	FileRead(handle,(UBYTE*)&ElementCount,sizeof(p_chunk->ElementCount));
	FileRead(handle,(UBYTE*)&MaxPeopleTypes,sizeof(p_chunk->MaxPeopleTypes));
	FileRead(handle,(UBYTE*)&MaxAnimFrames,sizeof(p_chunk->MaxAnimFrames));
	FileRead(handle,(UBYTE*)&MaxElements,sizeof(p_chunk->MaxElements));
	FileRead(handle,(UBYTE*)&MaxKeyFrames,sizeof(p_chunk->MaxKeyFrames));
	FileRead(handle,(UBYTE*)&MaxFightCols,sizeof(p_chunk->MaxFightCols));

	//
	// memory should have been allocated over when the .all was loaded
	//

	//
	// Load the people types
	//
	//FileRead(handle,(UBYTE*)&temp,sizeof(temp));
//	FileRead(handle,(UBYTE*)&p_chunk->PeopleTypes[0],sizeof(struct BodyDef)*p_chunk->MaxPeopleTypes);
	FileSeek(handle,SEEK_MODE_CURRENT,sizeof(struct BodyDef)*MaxPeopleTypes);
	FileRead(handle,&check,2);
	ASSERT(check==666);
	//
	// Load the keyframe linked lists
	//
	FileRead(handle,(UBYTE*)&addr1,sizeof(addr1));

	FileRead(handle,(UBYTE*)&p_chunk->AnimKeyFrames[p_chunk->MaxKeyFrames],sizeof(struct GameKeyFrame)*MaxKeyFrames);
	FileRead(handle,&check,2);
	ASSERT(check==666);

	//
	// Load the anim elements
	//

#ifndef ULTRA_COMPRESSED_ANIMATIONS
	FileRead(handle,(UBYTE*)&addr2,sizeof(addr2));
	FileRead(handle,(UBYTE*)&p_chunk->TheElements[p_chunk->MaxElements],sizeof(struct GameKeyFrameElement)*MaxElements);
	FileRead(handle,&check,2);
	ASSERT(check==666);
#else




#endif



	//
	// Load the animlist
	//
	FileRead(handle,(UBYTE*)&p_chunk->AnimList[start_frame],sizeof(struct GameKeyFrame *)*MaxAnimFrames);
	FileRead(handle,&check,2);
	ASSERT(check==666);

	FileRead(handle,(UBYTE*)&addr3,sizeof(addr3));

	FileRead(handle,(UBYTE*)&p_chunk->FightCols[p_chunk->MaxFightCols],sizeof(struct GameFightCol)*MaxFightCols);
	FileRead(handle,&check,2);
	ASSERT(check==666);



	if(save_type>4)
	{
extern	void	convert_keyframe_to_pointer(GameKeyFrame *p,GameKeyFrameElement *p_ele,GameFightCol *p_fight,SLONG count);
extern	void	convert_animlist_to_pointer(GameKeyFrame **p,GameKeyFrame *p_anim,SLONG count);
extern	void	convert_fightcol_to_pointer(GameFightCol *p,GameFightCol *p_fight,SLONG count);

  		convert_keyframe_to_pointer(&p_chunk->AnimKeyFrames[p_chunk->MaxKeyFrames],&p_chunk->TheElements[p_chunk->MaxElements],&p_chunk->FightCols[p_chunk->MaxFightCols],MaxKeyFrames);
		convert_animlist_to_pointer(&p_chunk->AnimList[start_frame],&p_chunk->AnimKeyFrames[p_chunk->MaxKeyFrames],MaxAnimFrames);
		convert_fightcol_to_pointer(&p_chunk->FightCols[p_chunk->MaxFightCols],&p_chunk->FightCols[p_chunk->MaxFightCols],MaxFightCols);
	}


//	p_chunk->ElementCount+=ElementCount;
	p_chunk->MaxAnimFrames=start_frame+MaxAnimFrames;
	p_chunk->MaxElements+=MaxElements;
	p_chunk->MaxKeyFrames+=MaxKeyFrames;
	p_chunk->MaxFightCols+=MaxFightCols;



	return(1);



}

void	skip_a_prim(SLONG prim,MFFileHandle	handle,SLONG	save_type)
{
	SLONG	c0;
	SLONG	sf3,ef3,sf4,ef4,sp,ep;
	SLONG	dp;


	if(handle!=FILE_OPEN_ERROR)
	{
//		FileRead(handle,(UBYTE*)&prim_names[next_prim_object],32);//sizeof(prim_objects[0].ObjectName));
		FileSeek(handle,SEEK_MODE_CURRENT,32);

//		LogText(" name %s becomes prim %d\n",prim_objects[next_prim_object].ObjectName,next_prim_object);

		FileRead(handle,(UBYTE*)&sp,sizeof(sp));
		FileRead(handle,(UBYTE*)&ep,sizeof(ep));

		ASSERT(ep-sp>=0 && ep-sp<10000);


		for(c0=sp;c0<ep;c0++)
		{
//			FileRead(handle,(UBYTE*)&prim_points[next_prim_point+c0-sp],sizeof(struct PrimPoint));
			FileSeek(handle,SEEK_MODE_CURRENT,sizeof(struct PrimPoint));
		}
		


		FileRead(handle,(UBYTE*)&sf3,sizeof(sf3));
		FileRead(handle,(UBYTE*)&ef3,sizeof(ef3));
		ASSERT(ef3-sf3>=0 && ef3-sf3<10000);
		for(c0=sf3;c0<ef3;c0++)
		{
//			FileRead(handle,(UBYTE*)&prim_faces3[next_prim_face3+c0-sf3],sizeof(struct PrimFace3));
			FileSeek(handle,SEEK_MODE_CURRENT,sizeof(struct PrimFace3));
		}


		FileRead(handle,(UBYTE*)&sf4,sizeof(sf4));
		FileRead(handle,(UBYTE*)&ef4,sizeof(ef4));
		ASSERT(ef4-sf4>=0 && ef4-sf4<10000);
		for(c0=sf4;c0<ef4;c0++)
		{
//			FileRead(handle,(UBYTE*)&prim_faces4[next_prim_face4+c0-sf4],sizeof(struct PrimFace4));
			FileSeek(handle,SEEK_MODE_CURRENT,sizeof(struct PrimFace4));
		}

	}

}

void	skip_load_a_multi_prim(MFFileHandle	handle)
{
	SLONG			c0;
	SLONG			save_type=0;
	SLONG			so,eo;

	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

		FileRead(handle,(UBYTE*)&so,sizeof(so));
		FileRead(handle,(UBYTE*)&eo,sizeof(eo));

		for(c0=so;c0<eo;c0++)
			skip_a_prim(c0,handle,save_type);
	}
}

SLONG	append_anim_system(struct GameKeyFrameChunk *p_chunk,CBYTE	*name,SLONG start_anim,SLONG load_mesh)
{
	SLONG			c0,point;
	MFFileHandle	handle;
//	CBYTE			file_name[64];
	SLONG			save_type=0;
//	SLONG			so,eo;
	CBYTE			ext_name[100];
	SLONG			count;

	CBYTE	fname[100];

	sprintf(fname,"%sdata\\%s",DATA_DIR,name);

extern	void	free_game_chunk(GameKeyFrameChunk *the_chunk);
//	free_game_chunk(p_chunk);

	DebugText(" APPEND chunk name %s\n",fname);
	change_extension(fname,"all",ext_name);
	handle	=	FileOpen(ext_name);
	if(handle!=FILE_OPEN_ERROR)
	{
		SLONG	start_ob=0;

		while(p_chunk->MultiObject[start_ob])
		{
			start_ob++;
		}

		

		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));

		FileRead(handle,(UBYTE*)&count,sizeof(count));

		ASSERT(start_ob+count<11);

		for(c0=0;c0<count;c0++)
		{
			if(load_mesh)
				p_chunk->MultiObject[start_ob+c0]=load_insert_a_multi_prim(handle);
			else
				skip_load_a_multi_prim(handle);
//			DebugText(" next_prim_point %d primface3 %d primface4 %d   load ANIMSYSTEM part %d \n",next_prim_point,next_prim_face3,next_prim_face4,c0);
		}
		if(c0+start_ob<10)
			p_chunk->MultiObject[c0+start_ob]=0;

		load_append_game_chunk(handle,p_chunk,start_anim);

		FileClose(handle);
		return(1);
	}
	else
	{
		DebugText("\n ERROR failed to open \n");
	}
	return(0);
	
}

#endif
#endif

#endif
#endif

