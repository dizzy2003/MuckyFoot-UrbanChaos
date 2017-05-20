// Main.cpp
// Guy Simmons, 10th February 1997.


//#ifdef	EDITOR
#include	"Editor.hpp"
//#endif

#include	"Stealth.h"
#include	"poly.h"
#include	"c:\fallen\headers\io.h"
#include	"c:\fallen\headers\noserver.h"


extern void	game(void);

extern SLONG editor_texture_set;


//---------------------------------------------------------------

GameTexture	game_textures[NUM_GAME_TEXTURES];


UWORD	page_lookup[64*8];

//---------------------------------------------------------------
SLONG	load_rect_into_page(UWORD *ptr,SLONG xpos,SLONG ypos,SLONG w,SLONG h,CBYTE *name)
{
	SLONG	x,y;
	TGA_Pixel	*tga;
	UWORD	col;
	TGA_Info	ret;

	tga=(TGA_Pixel*)MemAlloc( 4 * 256 * 256 );
	if(!tga)
		return(0);

	//	ret=TGA_load(name,256,256,(TGA_Pixel*)tga);
	ret=TGA_load(name,256,256,(TGA_Pixel*)tga,-1); 


	if(!ret.valid)
		return(0);
	
	for(y=0;y<h;y++)
	for(x=0;x<w;x++)
	{
		SLONG	px,py;
		px=x*ret.width/w;
		py=y*ret.height/h;
		col = the_display.GetFormattedPixel(tga[px+py*ret.height].red,tga[px+py*ret.height].green,tga[px+py*ret.height].blue);
		ptr[(xpos+x)+(ypos+y)*256]=col;
	}

	MemFree((void*)tga);
	return(1);
}
void	load_splitup_texture_page(UWORD *ptr,CBYTE *path,UWORD page)
{
	SLONG	x,y;
	CBYTE	name[256];
	SLONG	index=0;


	for(y=0;y<256;y+=32)
	for(x=0;x<256;x+=32)
	{

		sprintf(name,"%stex%03dhi.tga",path,page+index);
		if(load_rect_into_page(ptr,x,y,32,32,name)==0)
		{
			sprintf(name,"%stex%03d.tga",path,page+index);
				load_rect_into_page(ptr,x,y,32,32,name);

		}
		index++;
	}
	
}

extern	TGA_Info TGA_load_remap(const CBYTE *file,const CBYTE *pname,SLONG max_width,SLONG max_height,TGA_Pixel *data);

SLONG	load_rect_into_page_remap(UWORD *ptr,SLONG xpos,SLONG ypos,SLONG w,SLONG h,CBYTE *name,CBYTE *pname)
{
	SLONG	x,y;
	TGA_Pixel	*tga;
	UWORD	col;
	TGA_Info	ret;

	tga=(TGA_Pixel*)MemAlloc( 4 * 256 * 256 );
	if(!tga)
		return(0);
	ret=TGA_load_remap(name,pname,256,256,(TGA_Pixel*)tga);
	if(!ret.valid)
		return(0);
	
	for(y=0;y<h;y++)
	for(x=0;x<w;x++)
	{
		SLONG	px,py;
		px=x*ret.width/w;
		py=y*ret.height/h;
		col = the_display.GetFormattedPixel(tga[px+py*ret.height].red,tga[px+py*ret.height].green,tga[px+py*ret.height].blue);
		ptr[(xpos+x)+(ypos+y)*256]=col;
	}

	MemFree((void*)tga);
	return(1);
}

void	load_splitup_texture_page_remap(UWORD *ptr,CBYTE *path,UWORD page)
{
	SLONG	x,y;
	CBYTE	name[256];
	CBYTE	pname[256];
	SLONG	index=0;


	for(y=0;y<256;y+=32)
	for(x=0;x<256;x+=128)
	{

		sprintf(name,"%stex%03dhi.tga",path,page+index);

		load_rect_into_page(ptr,x,y,32,32,name);

		sprintf(pname,"%s%03da.pal",path,page+index);
		load_rect_into_page_remap(ptr,x+32,y,32,32,name,pname);

		sprintf(pname,"%s%03db.pal",path,page+index);
		load_rect_into_page_remap(ptr,x+64,y,32,32,name,pname);

		sprintf(pname,"%s%03dc.pal",path,page+index);
		load_rect_into_page_remap(ptr,x+96,y,32,32,name,pname);

		index++;
	}
	
}

extern	void	load_game_textures_psx(UBYTE flags);
void	load_game_textures(UBYTE flags)
{
	SLONG	c0;
	GameTexture		*the_texture;
	CBYTE	dir[128];
	UWORD	page;
	SLONG	i,j,count_pal=0;
#ifdef	NO_SERVER
	CBYTE	textures[]="server\\textures";
#else
	CBYTE	textures[]="u:\\urbanchaos\\textures";
#endif

	if(FileExists("psx.txt"))
	{
		sprintf(textures,"u:\\urbanchaos\\gary16");
	}

	sprintf(dir, "%s\\world%d\\",textures, editor_texture_set);
	strcpy(TEXTURE_WORLD_DIR,dir);

	for(i=0;i<21;i++)
	{

		switch(i)
		{
			case	0:
			case	1:
			case	2:
			case	3:
				if(!(flags&LOAD_UNSHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\world%d\\",textures, editor_texture_set);
				page=i*64;
				break;
			case	4:
			case	5:
			case	6:
			case	7:
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\",textures);
				page=i*64;
				break;
			case	8: //1 for insides
//				if(!(flags&LOAD_UNSHARED_TEXTURES))
//					continue;
				sprintf(dir, "%s\\world%d\\insides\\",textures, editor_texture_set);
				page=0;
				break;
			case	9:
			case	10: //2 for pepole
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\people\\",textures);
				page=(i-9)*64;
				break;
			case	11:
			case	12: // 3 for prims
			case	13:
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\prims\\",textures);
				page=(i-11)*64;
				ASSERT(page!=93);
				break;
			case	14:
			case	15:	 // 3 more for prims
			case	16:
			case	17:
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\prims\\",textures);
				page=(i-11)*64;
				ASSERT(page!=93);
				break;

			case	18: //people 2 continued
			case	19: //people 2 continued
			case	20: //people 2 continued
			case	21: //people 2 continued
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\people2\\",textures);
				page=(i-18)*64;
				break;


		}
		the_texture	= &game_textures[i];
		the_texture->TexturePtr	=	(UWORD*)MemAlloc(TEXTURE_PAGE_SIZE);
		if(the_texture->TexturePtr==0)
			ASSERT(0);
//		ASSERT(i!=16);

		memset((UBYTE*)the_texture->TexturePtr,0,256*256*2);
/*
		if(i==14)
		{
			//the_texture->TexturePtr=&game_textures[8];
			the_texture	= &game_textures[8];
		}
		if(i>14)
		{
			the_texture	= &game_textures[i];
		}
*/

		load_splitup_texture_page(the_texture->TexturePtr,dir,page);
//		if(i!=14)
		{
			the_texture->PalPtr	=	(UBYTE*)MemAlloc(256*3);
			tmaps[i]	=	the_texture->TexturePtr;
		}

		if(the_texture->PalPtr)
		{
			//
			// The palettes aren't loaded anymore- but just in case something
			// somewhere uses them- create a random palette.
			//

			for (j = 0; j < 256 * 3; j++)
			{
				the_texture->PalPtr[j] = rand();
			}

			pals[count_pal]	=	the_texture->PalPtr;
			count_pal++;
		}

	}

	//
	// Load the textures styles.
	//

void	load_texture_instyles(UBYTE editor, UBYTE world);
	load_texture_instyles(TRUE, editor_texture_set);
	load_texture_styles(TRUE, editor_texture_set);
	load_game_textures_psx(flags);
}

void	load_game_textures_psx(UBYTE flags)
{
	SLONG	c0;
	GameTexture		*the_texture;
	CBYTE	dir[128];
	UWORD	page;
	SLONG	i,j,count_pal=0;

#ifdef	NO_SERVER
	CBYTE	textures[]="server\\gary64";
#else
	CBYTE	textures[]="u:\\urbanchaos\\gary64";
#endif


	sprintf(dir, "%s\\world%d\\",textures, editor_texture_set);
//	strcpy(TEXTURE_WORLD_DIR,dir);

	for(i=0;i<8;i++)
	{

		switch(i)
		{
			case	0:
			case	1:
			case	2:
			case	3:
				if(!(flags&LOAD_UNSHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\world%d\\",textures, editor_texture_set);
				page=i*16;
				break;
			case	4:
			case	5:
			case	6:
			case	7:
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\",textures);
				page=i*64;
				break;
			case	8: //1 for insides
//				if(!(flags&LOAD_UNSHARED_TEXTURES))
//					continue;
				sprintf(dir, "%s\\world%d\\insides\\",textures, editor_texture_set);
				page=0;
				break;
			case	9:
			case	10: //2 for pepole
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\people\\",textures);
				page=(i-9)*64;
				break;
			case	11:
			case	12: // 3 for prims
			case	13:
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\prims\\",textures);
				page=(i-11)*64;
				ASSERT(page!=93);
				break;
			case	14: //special people2
				if(!(flags&LOAD_SHARED_TEXTURES))
					continue;
				sprintf(dir, "%s\\shared\\people2\\",textures);
				page-=14;
				break;

		}
		the_texture	= &game_textures[i+25];
		the_texture->TexturePtr	=	(UWORD*)MemAlloc(TEXTURE_PAGE_SIZE);
		if(the_texture->TexturePtr==0)
		{
			MessageBox(
				NULL,
				"You don't have enough free disk space! Try deleting some files and trying again.", "Out of disk space", MB_ICONERROR | MB_OK);

			ASSERT(0);
		}

		memset((UBYTE*)the_texture->TexturePtr,0,256*256*2);


		load_splitup_texture_page_remap(the_texture->TexturePtr,dir,page);
		the_texture->PalPtr	=	(UBYTE*)MemAlloc(256*3);
		tmaps[i+25]	=	the_texture->TexturePtr;
		

		if(the_texture->PalPtr)
		{
			//
			// The palettes aren't loaded anymore- but just in case something
			// somewhere uses them- create a random palette.
			//

			for (j = 0; j < 256 * 3; j++)
			{
				the_texture->PalPtr[j] = rand();
			}

			pals[count_pal]	=	the_texture->PalPtr;
			count_pal++;
		}

	}

	//
	// Load the textures styles.
	//

}

/*
void	load_game_textures(void)
{
	SLONG i;
	SLONG j;

	ULONG			count=0;
	ULONG			count_pal=0;
	GameTexture		*the_texture;
	TGA_Pixel			*temp_page;

	CBYTE fname[256];

	temp_page=(TGA_Pixel*)MemAlloc( 4 * 256 * 256 );
	if(temp_page)
	{
		for (i = 0; i < NUM_GAME_TEXTURES; i++)
		{
			if (i < 4)
			{
				//
				// This is a world texture.
				//

				sprintf(fname, "data\\textures\\world%d\\editor\\gen%02d.tga", editor_texture_set, i + 1);
			}
			else
			{
				//
				// This is a shared texture.
				//

				sprintf(fname, "data\\textures\\shared\\editor\\gen%02d.tga", i + 1);
			}

			the_texture	= &game_textures[i];

			the_texture->TexturePtr	=	(UWORD*)MemAlloc(TEXTURE_PAGE_SIZE);
			if(the_texture->TexturePtr)
			{
				SLONG	x,y;
				TGA_load(fname,256,256,(TGA_Pixel*)temp_page);
				for(y=0;y<256;y++)
				for(x=0;x<256;x++)
				{
					UWORD	col,r,g,b;
					r=temp_page[x+y*256].red;
					g=temp_page[x+y*256].green;
					b=temp_page[x+y*256].blue;

					col = the_display.GetFormattedPixel(r,g,b);
					the_texture->TexturePtr[x+y*256]=col;

				}
				tmaps[count]	=	the_texture->TexturePtr;
				count++;
			}

			the_texture->PalPtr	=	(UBYTE*)MemAlloc(256*3);

			if(the_texture->PalPtr)
			{
				//
				// The palettes aren't loaded anymore- but just in case something
				// somewhere uses them- create a random palette.
				//

				for (j = 0; j < 256 * 3; j++)
				{
					the_texture->PalPtr[j] = rand();
				}

				pals[count_pal]	=	the_texture->PalPtr;
				count_pal++;
			}
		}

		MemFree(temp_page);
	}
}
*/
//---------------------------------------------------------------

void	free_game_textures(UBYTE flags)
{
	SLONG i;

	GameTexture		*the_texture;

	for (i = 0; i < NUM_GAME_TEXTURES; i++)
	{
		switch(i)
		{
			case	0:
			case	1:
			case	2:
			case	3:
				if(!(flags&FREE_UNSHARED_TEXTURES))
					continue;
				break;
			case	4:
			case	5:
			case	6:
			case	7:
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	8: //1 for insides
				if(!(flags&FREE_UNSHARED_TEXTURES))
					continue;
				break;
			case	9:
			case	10: //2 for pepole
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	11:
			case	12: // 3 for prims
			case	13:
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	14: //special people2
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	15: //special people2
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;

			case	16:
			case	17:
			case	18:
			case	19:
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	20:
			case	21:
			case	22:
			case	23:
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	24: //1 for insides
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	25:
			case	26: //2 for pepole
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	27:
			case	28: // 3 for prims
			case	29:
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;
			case	30: //special people2
				if(!(flags&FREE_SHARED_TEXTURES))
					continue;
				break;

		}

		the_texture = &game_textures[i];

		if(the_texture->TexturePtr)
		{
			MemFree(the_texture->TexturePtr);
			the_texture->TexturePtr	=	0;
		}
		if(the_texture->PalPtr)
		{
			MemFree(the_texture->PalPtr);
			the_texture->PalPtr	=	0;

		}
	}
}

//---------------------------------------------------------------
