//
// Texture handling.
//

#include "game.h"
#include <DDLib.h>
#include "c:\fallen\ddlibrary\headers\tga.h"
#include "texture.h"
#include "c:\fallen\headers\animtmap.h"
#include "c:\fallen\headers\supermap.h"
#include "poly.h"
#include "pap.h"
#include "ns.h"
#include "memory.h"
#include "c:\fallen\headers\io.h"
#include "c:\fallen\headers\inside2.h"
#include "sound.h"
#include "c:\fallen\headers\noserver.h"
#include "ware.h"
#include "truetype.h"
#include "font2d.h"
#include "env.h"
#include "drive.h"
#include "c:\fallen\headers\attract.h"
#include "crinkle.h"
#ifdef TARGET_DC
#include "c:\fallen\ddlibrary\headers\GDisplay.h"
#endif
int	TEXTURE_create_clump = 0;

//
// The current texture set and the current texture directory
// for first four texture pages.
//

CBYTE TEXTURE_shared_dir [_MAX_PATH];
CBYTE TEXTURE_world_dir  [_MAX_PATH];
CBYTE TEXTURE_fx_inifile [_MAX_PATH];
CBYTE TEXTURE_shared_fx_inifile [_MAX_PATH];
CBYTE TEXTURE_prims_dir  [_MAX_PATH];
CBYTE TEXTURE_inside_dir [_MAX_PATH];
CBYTE TEXTURE_people_dir [_MAX_PATH];
CBYTE TEXTURE_people_dir2[_MAX_PATH];
SLONG TEXTURE_set;



//
// Are we using normal or fiddled pages?
//

SLONG TEXTURE_fiddled;

UWORD *TEXTURE_shadow_bitmap;
SLONG  TEXTURE_shadow_pitch;		// In bytes!
SLONG  TEXTURE_shadow_mask_red;
SLONG  TEXTURE_shadow_mask_green;
SLONG  TEXTURE_shadow_mask_blue;
SLONG  TEXTURE_shadow_mask_alpha;
SLONG  TEXTURE_shadow_shift_red;
SLONG  TEXTURE_shadow_shift_green;
SLONG  TEXTURE_shadow_shift_blue;
SLONG  TEXTURE_shadow_shift_alpha;


//
// Hmm...
//

extern UWORD floor_texture_sizes[];

#ifdef TARGET_DC
// Lower numbers so it actually fits.
#define TEXTURE_NUM_STANDARD (22 * 64)
#define TEXTURE_MAX_TEXTURES (TEXTURE_NUM_STANDARD+160)
#define TEXTURE_NORM_SIZE    32
#define TEXTURE_NORM_SQUARES 8
#define	PEOPLE3_ALT				21*64
#else
#define TEXTURE_NUM_STANDARD (22 * 64)
#define TEXTURE_MAX_TEXTURES (TEXTURE_NUM_STANDARD+160)
#define TEXTURE_NORM_SIZE    32
#define TEXTURE_NORM_SQUARES 8
#define	PEOPLE3_ALT				21*64
#endif

//
// Texutre pages that don't exist.
//

UBYTE TEXTURE_dontexist[TEXTURE_MAX_TEXTURES];

// Texture pages that are "needed", i.e. used by the frontend.
UBYTE TEXTURE_needed[TEXTURE_MAX_TEXTURES];

//
// The texture pages.
// 

D3DTexture TEXTURE_texture[TEXTURE_MAX_TEXTURES];

//
// The crinkle for each standard texture page.
//

CRINKLE_Handle TEXTURE_crinkle[22 * 64];


//
// The textures.
//

SLONG TEXTURE_page_num_standard;

#ifdef TARGET_DC
SLONG TEXTURE_page_background_use_instead;
SLONG TEXTURE_page_background_use_instead2;

SLONG TEXTURE_page_joypad_a;
SLONG TEXTURE_page_joypad_b;
SLONG TEXTURE_page_joypad_c;
SLONG TEXTURE_page_joypad_x;
SLONG TEXTURE_page_joypad_y;
SLONG TEXTURE_page_joypad_z;
SLONG TEXTURE_page_joypad_l;
SLONG TEXTURE_page_joypad_r;
SLONG TEXTURE_page_joypad_pad_l;
SLONG TEXTURE_page_joypad_pad_r;
SLONG TEXTURE_page_joypad_pad_d;
SLONG TEXTURE_page_joypad_pad_u;
#endif

SLONG TEXTURE_page_snowflake;
SLONG TEXTURE_page_sparkle;
SLONG TEXTURE_page_explode2;
SLONG TEXTURE_page_explode1;
SLONG TEXTURE_page_bigbang;
SLONG TEXTURE_page_face1;
SLONG TEXTURE_page_face2;
SLONG TEXTURE_page_face3;
SLONG TEXTURE_page_face4;
SLONG TEXTURE_page_face5;
SLONG TEXTURE_page_face6;
SLONG TEXTURE_page_fog;
SLONG TEXTURE_page_moon;
SLONG TEXTURE_page_clouds;
SLONG TEXTURE_page_water;
SLONG TEXTURE_page_puddle;
SLONG TEXTURE_page_drip;
SLONG TEXTURE_page_shadow;
SLONG TEXTURE_page_bang;
SLONG TEXTURE_page_font;
SLONG TEXTURE_page_logo;
SLONG TEXTURE_page_sky;
SLONG TEXTURE_page_flames;
SLONG TEXTURE_page_smoke;
SLONG TEXTURE_page_flame2;
SLONG TEXTURE_page_steam;
SLONG TEXTURE_page_menuflame;
SLONG TEXTURE_page_barbwire;
SLONG TEXTURE_page_font2d;
SLONG TEXTURE_page_dustwave;
SLONG TEXTURE_page_flames3;
SLONG TEXTURE_page_bloodsplat;
SLONG TEXTURE_page_bloom1;
SLONG TEXTURE_page_bloom2;
SLONG TEXTURE_page_hitspang;
SLONG TEXTURE_page_lensflare;
SLONG TEXTURE_page_envmap;
SLONG TEXTURE_page_tyretrack;
SLONG TEXTURE_page_winmap;
SLONG TEXTURE_page_leaf;
SLONG TEXTURE_page_raindrop;
SLONG TEXTURE_page_footprint;
SLONG TEXTURE_page_angel;
SLONG TEXTURE_page_devil;
SLONG TEXTURE_page_smoker;
SLONG TEXTURE_page_target;
SLONG TEXTURE_page_newfont;
SLONG TEXTURE_page_droplet;
SLONG TEXTURE_page_press1;
SLONG TEXTURE_page_press2;
SLONG TEXTURE_page_ic;
SLONG TEXTURE_page_ic2;
SLONG TEXTURE_page_lcdfont;
SLONG TEXTURE_page_smokecloud;
SLONG TEXTURE_page_menulogo;
SLONG TEXTURE_page_polaroid;
SLONG TEXTURE_page_bigbutton;
SLONG TEXTURE_page_bigleaf;
SLONG TEXTURE_page_bigrain;
SLONG TEXTURE_page_finalglow;
SLONG TEXTURE_page_tinybutt;
SLONG TEXTURE_page_tyretrack_alpha;
SLONG TEXTURE_page_people3;
SLONG TEXTURE_page_ladder;
SLONG TEXTURE_page_fadecat;
SLONG TEXTURE_page_fade_MF;
SLONG TEXTURE_page_shadowoval;
SLONG TEXTURE_page_rubbish;
SLONG TEXTURE_page_lastpanel;
SLONG TEXTURE_page_lastpanel2;
SLONG TEXTURE_page_sign;
SLONG TEXTURE_page_pcflamer;
SLONG TEXTURE_page_shadowsquare;
SLONG TEXTURE_page_litebolt;
SLONG TEXTURE_page_ladshad;
SLONG TEXTURE_page_meteor;
SLONG TEXTURE_page_splash;




// ========================================================
//
// THE DC PAGING SYSTEM
//

#define TEXTURE_ENABLE_DC_PACKING      0
#define TEXTURE_DC_PACK_POS_WHOLE_PAGE 42

typedef struct
{
	UBYTE page;
	UBYTE pos;	// 0 - 8 or TEXTURE_DC_PACK_POS_WHOLE_PAGE

} TEXTURE_DC_Pack;

TEXTURE_DC_Pack TEXTURE_DC_pack[256];

SLONG TEXTURE_DC_pack_page_upto;		// Which page we are packing into
SLONG TEXTURE_DC_pack_page_pos;			// The position on the pack page that is free (0 - 8)
SLONG TEXTURE_DC_pack_normal_upto;		// Goes up in increments of 2 so we have a gap after each page.

//
// Where normal textures start from.
//

#define TEXTURE_DC_NORMAL_START 128



//
// Initialise the packing.
//

void TEXTURE_DC_pack_init(void)
{
	memset(TEXTURE_DC_pack, 0, sizeof(TEXTURE_DC_pack));

	TEXTURE_DC_pack_page_upto   = 0;
	TEXTURE_DC_pack_page_pos    = 0;
	TEXTURE_DC_pack_normal_upto = TEXTURE_DC_NORMAL_START;
}

void TEXTURE_DC_pack_load_page(SLONG page)
{
	SLONG i;
	SLONG actual_page;
	CBYTE name_res64[64];

	sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_world_dir, page);

	//
	// The packing only works for the world textures.
	//

	ASSERT(WITHIN(page, 0, 255));

	//
	// Does this page get packed or does it become a normal page?
	//
	
	if (POLY_page_flag[page])
	{
		//
		// This is a normal page because it is drawn in a strange way. If
		// this page is drawn as the second pass of a 2-pass texture, then
		// it must be the actual_page after the preceeding page....
		//

		if (page > 0 && (POLY_page_flag[page - 1] & POLY_PAGE_FLAG_2PASS))
		{
			//
			// Find the preceeding page.
			//

			ASSERT(TEXTURE_DC_pack[page - 1].pos == TEXTURE_DC_PACK_POS_WHOLE_PAGE);

			actual_page = TEXTURE_DC_pack[page - 1].page + 1;
		}
		else
		{
			//
			// Use the next normal gap.
			//

			page                         = TEXTURE_DC_pack_normal_upto;
			TEXTURE_DC_pack_normal_upto += 2;
		}

		ASSERT(WITHIN(actual_page, 0, 255));

		TEXTURE_texture[actual_page].LoadTextureTGA(name_res64, actual_page);

		TEXTURE_DC_pack[page].page = actual_page;
		TEXTURE_DC_pack[page].pos  = TEXTURE_DC_PACK_POS_WHOLE_PAGE;
	}
	else
	{
		D3DTexture *tt;

		ASSERT(WITHIN(TEXTURE_DC_pack_page_upto, 0, TEXTURE_DC_NORMAL_START - 1));

		tt = &TEXTURE_texture[TEXTURE_DC_pack_page_upto];

		//
		// We can pack this texture.
		//

		if (TEXTURE_DC_pack_page_pos == 0)
		{
			//
			// Create a new 256x256 texture.
			//

			tt->CreateUserPage(256, FALSE);
		}

		//
		// Load the TGA.
		//

		TGA_Pixel *tga = (TGA_Pixel *) malloc(sizeof(TGA_Pixel) * 64 * 64);
		TGA_Info   ti  = TGA_load(name_res64, 64, 64, tga, 0, FALSE);

		if (ti.valid)
		{
			UWORD *bitmap;
			SLONG  pitch;

			//
			// Lock the texture and copy over the data into the texture.
			//

			tt->LockUser(&bitmap, &pitch);

			if (bitmap)
			{
				SLONG x;
				SLONG y;

				SLONG fx;
				SLONG fy;

				SLONG tx;
				SLONG ty;

				SLONG base_x;
				SLONG base_y;

				SLONG pixel;

				//
				// What is the base (x,y) for this pos?
				//

				ASSERT(WITHIN(TEXTURE_DC_pack_page_pos, 0, 8));

				base_x = (TEXTURE_DC_pack_page_pos % 3) * 64 + 32;
				base_y = (TEXTURE_DC_pack_page_pos / 3) * 64 + 32;
				
				//
				// The pitch is returned in bytes!
				//

				pitch >>= 1;

				//
				// Copy over the texture.
				//

				for (x = 0; x < 64; x++)
				for (y = 0; y < 64; y++)
				{
					fx = x;
					fy = y;

					tx = base_x + x;
					ty = base_y + y;

					pixel  = 0;
					pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
					pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
					pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

					bitmap[tx + ty * pitch] = pixel;
				}

				//
				// Now do the edges...
				//

				for (i = 0; i < 64; i++)
				{
					//
					// Top...
					//

					fx = i;
					fy = 0;

					tx = base_x + i;
					ty = base_y - 1;

					pixel  = 0;
					pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
					pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
					pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

					bitmap[tx + ty * pitch] = pixel;

					//
					// Bottom...
					//

					fx = i;
					fy = 63;

					tx = base_x + i;
					ty = base_y + 64;

					pixel  = 0;
					pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
					pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
					pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

					bitmap[tx + ty * pitch] = pixel;
					
					//
					// Left...
					//

					fx = 0;
					fy = i;

					tx = base_x - 1;
					ty = base_y + i;

					pixel  = 0;
					pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
					pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
					pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

					bitmap[tx + ty * pitch] = pixel;

					//
					// Right...
					//

					fx = 63;
					fy = i;

					tx = base_x + 64;
					ty = base_y + i;

					pixel  = 0;
					pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
					pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
					pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

					bitmap[tx + ty * pitch] = pixel;
				}

				//
				// Now do the corners.
				//

				//
				// Top left...
				//

				fx = 0;
				fy = 0;

				tx = base_x - 1;
				ty = base_y - 1;

				pixel  = 0;
				pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
				pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
				pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

				bitmap[tx + ty * pitch] = pixel;

				//
				// Bottom right...
				//

				fx = 63;
				fy = 63;

				tx = base_x + 64;
				ty = base_y + 64;

				pixel  = 0;
				pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
				pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
				pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

				bitmap[tx + ty * pitch] = pixel;
				
				//
				// Top right...
				//

				fx = 63;
				fy = 0;

				tx = base_x + 64;
				ty = base_y - 1;

				pixel  = 0;
				pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
				pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
				pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

				bitmap[tx + ty * pitch] = pixel;

				//
				// Bottom left...
				//

				fx = 0;
				fy = 63;

				tx = base_x - 1;
				ty = base_y + 64;

				pixel  = 0;
				pixel |= (tga[fx + fy * 64].red   >> tt->mask_red) << tt->mask_red;
				pixel |= (tga[fx + fy * 64].green >> tt->mask_red) << tt->mask_green;
				pixel |= (tga[fx + fy * 64].blue  >> tt->mask_red) << tt->mask_blue;

				bitmap[tx + ty * pitch] = pixel;

				tt->UnlockUser();
			}
		}

		free(tga);

		//
		// Remember where we've put this page.
		//

		TEXTURE_DC_pack[page].page = TEXTURE_DC_pack_page_upto;
		TEXTURE_DC_pack[page].pos  = TEXTURE_DC_pack_page_pos;

		//
		// Go onto the next hole...
		//

		TEXTURE_DC_pack_page_pos += 1;

		if (TEXTURE_DC_pack_page_pos >= 9)
		{
			TEXTURE_DC_pack_page_pos   = 0;
			TEXTURE_DC_pack_page_upto += 1;

			ASSERT(WITHIN(TEXTURE_DC_pack_page_upto, 0, TEXTURE_DC_NORMAL_START - 1));
		}
	}
}


























//
// Mark all standard textures as not-to-be-loaded in SOFTWARE mode.
//

void this_may_well_be_the_last_ever_function_call_put_into_the_game(void)
{
	SLONG i;

	if (!SOFTWARE)
	{
		return;
	}

	for (i = 0; i < TEXTURE_NUM_STANDARD; i++)
	{
		TEXTURE_texture[i].DontBotherLoadingInSoftwareMode = TRUE;
	}
}



//
// The number of textures.
//

SLONG TEXTURE_num_textures;

void TEXTURE_choose_set(SLONG number)
{
	SLONG i;
	CBYTE	textures[]="textures";

	if(FileExists("psx.txt"))
	{
		sprintf(textures,"gary16");
	}

	//
	// Set the directories.
	//


#ifdef	NO_SERVER
	sprintf(TEXTURE_inside_dir, "server\\%s\\world%d\\insides\\",textures, number);
	sprintf(TEXTURE_prims_dir,  "server\\%s\\shared\\prims\\",textures);
	sprintf(TEXTURE_people_dir, "server\\%s\\shared\\people\\",textures);
	sprintf(TEXTURE_people_dir2, "server\\%s\\shared\\people2\\",textures);
	sprintf(TEXTURE_world_dir,   "server\\%s\\world%d\\",textures, number);
	sprintf(TEXTURE_shared_dir,  "server\\%s\\shared\\",textures, number);
#else
	sprintf(TEXTURE_inside_dir,  "u:\\urbanchaos\\%s\\world%d\\insides\\",textures, number);
	sprintf(TEXTURE_prims_dir,   "u:\\urbanchaos\\%s\\shared\\prims\\",textures);
	sprintf(TEXTURE_people_dir,  "u:\\urbanchaos\\%s\\shared\\people\\",textures);
	sprintf(TEXTURE_people_dir2, "u:\\urbanchaos\\%s\\shared\\people2\\",textures);
	sprintf(TEXTURE_world_dir,   "u:\\urbanchaos\\%s\\world%d\\",textures, number);
	sprintf(TEXTURE_shared_dir,  "u:\\urbanchaos\\%s\\shared\\",textures);
#endif
	strcpy(TEXTURE_WORLD_DIR,TEXTURE_world_dir);
	sprintf(TEXTURE_fx_inifile, "%ssoundfx.ini", TEXTURE_world_dir);
	sprintf(TEXTURE_shared_fx_inifile, "%ssoundfx.ini", TEXTURE_shared_dir);
	SOUND_InitFXGroups(TEXTURE_shared_fx_inifile);

	if (number != TEXTURE_set)
	{
		//
		// Initialise all the crinkles.
		//

		CRINKLE_init();

		//
		// Free up all the world textures we have loaded so far.
		//


		for (i = 0; i < 64 * 4; i++)
		{
			if (TEXTURE_texture[i].Type != D3DTEXTURE_TYPE_UNUSED)
			{
				//
				// Free this texture.
				//

				TEXTURE_texture[i].Destroy();

				//
				// Mark as unused.
				//

				TEXTURE_texture[i].Type = D3DTEXTURE_TYPE_UNUSED;
			}
		}

		memset ( TEXTURE_dontexist,	0, sizeof ( TEXTURE_dontexist ) );
		memset ( TEXTURE_crinkle,	0, sizeof ( TEXTURE_crinkle ) );

		//
		// Load the new poly-page flags.
		//

		{
			CBYTE world_texture_flags[256];
			CBYTE shared_texture_flags[256];

			sprintf(world_texture_flags,  "%stextype.txt", TEXTURE_world_dir);
			sprintf(shared_texture_flags, "%stextype.txt", TEXTURE_shared_dir);

			POLY_init_texture_flags();
			POLY_load_texture_flags(world_texture_flags);
			POLY_load_texture_flags(shared_texture_flags);
#ifdef	NO_SERVER
			POLY_load_texture_flags("server\\textures\\shared\\prims\\textype.txt", 11 * 64);
#else
			POLY_load_texture_flags("u:\\urbanchaos\\textures\\shared\\prims\\textype.txt", 11 * 64);
#endif
		}

		//
		// Load in the style defs.
		//

		load_texture_styles(FALSE, number);
extern	void	load_texture_instyles(UBYTE editor, UBYTE world);
		load_texture_instyles(FALSE, number);
		TEXTURE_fix_texture_styles();

	}

	TEXTURE_set = number;
}





//
// Loads the highest res version of the given page that it can find.
//

bool	IndividualTextures = false;

static void TEXTURE_load_page(SLONG page)
{
	CBYTE name_res32[64];
	CBYTE name_res64[64];
	CBYTE name_res128[64];
#ifdef TARGET_DC
	CBYTE name_mvq32[64];
	CBYTE name_mvq64[64];
	CBYTE name_mvq128[64];
#endif
	CBYTE name_sex  [64] = "";
	CBYTE shortname_res32[14];
	CBYTE shortname_res64[14];
	CBYTE shortname_res128[14];
	SLONG fxref;

	FILE *exists32;
	FILE *exists64;
	FILE *exists128;

	ASSERT(TEXTURE_set);
	ASSERT(WITHIN(page, 0, TEXTURE_MAX_TEXTURES - 1));

	if (TEXTURE_dontexist[page])
	{
		return;
	}

	if (page < 64 * 4)
	{
		//
		// This is a world texture.
		//

		#if TEXTURE_ENABLE_DC_PACKING

		TEXTURE_DC_pack_load_page(page);

		return;

		#endif

		sprintf(name_res32, "%stex%03d.tga",   TEXTURE_world_dir, page);
		sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_world_dir, page);
		sprintf(name_res128,"%stex%03dto.tga", TEXTURE_world_dir, page);
		sprintf(name_sex,   "%ssex%03dhi.sex", TEXTURE_world_dir, page);

		sprintf(shortname_res32, "tex%03d.tga",   page);
		sprintf(shortname_res64, "tex%03dhi.tga", page);
		sprintf(shortname_res128,"tex%03dto.tga", page);

#ifndef TARGET_DC
		fxref=GetPrivateProfileInt("Textures",shortname_res64,-1,TEXTURE_fx_inifile);
		if (fxref==-1) {
		  fxref=GetPrivateProfileInt("Textures",shortname_res32,-1,TEXTURE_fx_inifile);
		}
#else //#ifndef TARGET_DC
		// Dreamcast - GetPrivateProfile* don't exist. Spoof them for now.
		fxref = -1;
#endif //#else //#ifndef TARGET_DC

#ifndef TARGET_DC
		if ( SOUND_FXMapping != NULL )
		{
			SOUND_FXMapping[page]=fxref;
		};
#endif

	}
	else
	if (page < 64 * 8)
	{
		//
		// This is a shared texture.
		//

		sprintf(name_res32, "%stex%03d.tga",   TEXTURE_shared_dir, page);
		sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_shared_dir, page);
		sprintf(name_res128,"%stex%03dto.tga", TEXTURE_shared_dir, page);
		sprintf(name_sex,   "%ssex%03dhi.sex", TEXTURE_shared_dir, page);
		sprintf(shortname_res32, "tex%03d.tga",   page);
		sprintf(shortname_res64, "tex%03dhi.tga", page);
		sprintf(shortname_res128,"tex%03dto.tga", page);

#ifndef TARGET_DC
		fxref=GetPrivateProfileInt("Textures",shortname_res64,-1,TEXTURE_shared_fx_inifile);
		if (fxref==-1) {
		  fxref=GetPrivateProfileInt("Textures",shortname_res32,-1,TEXTURE_shared_fx_inifile);
		}
#else //#ifndef TARGET_DC
		// Dreamcast - GetPrivateProfile* don't exist. Spoof them for now.
		fxref = -1;
#endif //#else //#ifndef TARGET_DC

#ifndef TARGET_DC
		if ( SOUND_FXMapping != NULL )
		{
			SOUND_FXMapping[page]=fxref;
		}
#endif
	}
	else
	if (page < 64 * 9)
	{
/*
//
// people2 has been promoted to 18+
//
		if(page>=64*8+32)
		{
			sprintf(name_res32, "%stex%03d.tga",   TEXTURE_people_dir2, page-64*8);
			sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_people_dir2, page-64*8);
		}
		else
*/
		{
			sprintf(name_res32, "%stex%03d.tga",   TEXTURE_inside_dir, page-64*8);
			sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_inside_dir, page-64*8);
			sprintf(name_res128,"%stex%03dto.tga", TEXTURE_inside_dir, page-64*8);
		}
	}
	else
	if (page < 64 * 11)
	{
		sprintf(name_res32, "%stex%03d.tga",   TEXTURE_people_dir, page-64*9);
		sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_people_dir, page-64*9);
		sprintf(name_res128,"%stex%03dto.tga", TEXTURE_people_dir, page-64*9);
		DebugText("people page %d offset %d \n",page/64,page-64*9);
	}
	else
	if (page < 64 * 18)
	{
		sprintf(name_res32, "%stex%03d.tga",   TEXTURE_prims_dir, page-64*11);
		sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_prims_dir, page-64*11);
		sprintf(name_res128,"%stex%03dto.tga", TEXTURE_prims_dir, page-64*11);
		DebugText("prims page %d offset %d fname %s\n",page/64,page-64*11,name_res64);
	}
	else
	if (page < 64 * 21)
	{
		sprintf(name_res32, "%stex%03d.tga",   TEXTURE_people_dir2, page-64*18);
		sprintf(name_res64, "%stex%03dhi.tga", TEXTURE_people_dir2, page-64*18);
		sprintf(name_res128,"%stex%03dto.tga", TEXTURE_people_dir2, page-64*18);
	}
	else
	{
		ASSERT(0);
	}


#ifdef TARGET_DC
	strcpy ( name_mvq32, name_res32 );
	char *pch = name_mvq32 + strlen ( name_res32 ) - 3;
	*pch++ = 'm';
	*pch++ = 'v';
	*pch++ = 'q';
	strcpy ( name_mvq64, name_res64 );
	pch = name_mvq64 + strlen ( name_res64 ) - 3;
	*pch++ = 'm';
	*pch++ = 'v';
	*pch++ = 'q';
	strcpy ( name_mvq128, name_res128 );
	pch = name_mvq128 + strlen ( name_res128 ) - 3;
	*pch++ = 'm';
	*pch++ = 'v';
	*pch++ = 'q';

#endif

	if (IndividualTextures || TEXTURE_create_clump)
	{
#ifdef TARGET_DC
		HRESULT hres;
		if ( FileExists(name_res128) || FileExists(name_mvq128) )
		{
			hres = TEXTURE_texture[page].LoadTextureTGA(name_res128, page);
		}
		else if ( FileExists(name_res64) || FileExists(name_mvq64) )
		{
			hres = TEXTURE_texture[page].LoadTextureTGA(name_res64, page);
		}
		else if ( FileExists(name_res32) || FileExists(name_mvq32) )
		{
			hres = TEXTURE_texture[page].LoadTextureTGA(name_res32, page);
		}
		else
		{
			hres = DDERR_INVALIDOBJECT;
		}

		if ( FAILED ( hres ) )
		{
			TRACE ( "Page %s not found", name_res32 );
			TEXTURE_dontexist[page] = TRUE;
		}


#else

		exists128 = MF_Fopen(name_res128, "rb");
		if (exists128)
		{
			MF_Fclose(exists128);
			TEXTURE_texture[page].LoadTextureTGA(name_res128, page);
		}
		else
		{
			exists64 = MF_Fopen(name_res64, "rb");

			if (exists64)
			{
				MF_Fclose(exists64);

				TEXTURE_texture[page].LoadTextureTGA(name_res64, page);
			}
			else
			{
				exists32 = MF_Fopen(name_res32, "rb");

				if (exists32)
				{
					MF_Fclose(exists32);

					TEXTURE_texture[page].LoadTextureTGA(name_res32, page);
				}
				else
				{
					//
					// This texture doesn't exist	!
					//
					DebugText(" cant find page %d name64 %s \n",page,name_res64);

					TEXTURE_dontexist[page] = TRUE;
				}
			}
		}
#endif
	}
	else
	{

#ifdef TARGET_DC
		HRESULT hres;
		if ( FileExists(name_res128) || FileExists(name_mvq128) )
		{
			hres = TEXTURE_texture[page].LoadTextureTGA(name_res128, page);
		}
		else if ( FileExists(name_res64) || FileExists(name_mvq64) )
		{
			hres = TEXTURE_texture[page].LoadTextureTGA(name_res64, page);
		}
		else if ( FileExists(name_res32) || FileExists(name_mvq32) )
		{
			hres = TEXTURE_texture[page].LoadTextureTGA(name_res32, page);
		}
		else
		{
			hres = DDERR_INVALIDOBJECT;
		}

		if ( FAILED ( hres ) )
		{
			TRACE ( "Page %s not found", name_res32 );
			ASSERT ( FALSE );
			TEXTURE_dontexist[page] = TRUE;
		}
#else

		if (DoesTGAExist(name_res64, page))
		{
			TEXTURE_texture[page].LoadTextureTGA(name_res64, page);
		}
		else
		if (DoesTGAExist(name_res32, page))
		{
			TEXTURE_texture[page].LoadTextureTGA(name_res32, page);
		}
		else
		{
			TEXTURE_dontexist[page] = TRUE;
		}
#endif
	}

	// crinkles

	if (page < 64 * 8)
	{
#ifdef TARGET_DC
		ASSERT ( IndividualTextures );
#endif
		if (IndividualTextures || TEXTURE_create_clump)
		{
			TEXTURE_crinkle[page] = CRINKLE_load(name_sex);
#ifdef TARGET_DC
			ASSERT ( !TEXTURE_create_clump );
#else
			if (TEXTURE_create_clump && TEXTURE_crinkle[page])
			{
				CRINKLE_write_bin(GetTGAClump(), TEXTURE_crinkle[page], TEXTURE_MAX_TEXTURES + page);
			}
#endif
		}
		else
		{
#ifdef TARGET_DC
			// Shouldn't get here.
			ASSERT ( FALSE );
#else
			if (!DoesTGAExist("", TEXTURE_MAX_TEXTURES + page))
			{
				TEXTURE_crinkle[page] = NULL;
			}
			else
			{
				TEXTURE_crinkle[page] = CRINKLE_read_bin(GetTGAClump(), TEXTURE_MAX_TEXTURES + page);
			}
#endif
		}
	}

	if (POLY_page_is_masked_self_illuminating(page))
	{
		if (TEXTURE_texture[page + 1].Type == D3DTEXTURE_TYPE_UNUSED)
		{
			TEXTURE_load_page(page + 1);
		}
	}
}


void TEXTURE_initialise_clumping(CBYTE *fname_level)
{

#ifdef TARGET_DC
	const int clumping = 0;
#else //#ifdef TARGET_DC

#undef	FINAL
#ifdef FINAL
	int	clumping = 1;
#else
	int	clumping = ENV_get_value_number("enable_clumps", 0, "TextureClumps");
#endif

#endif //#else //#ifdef TARGET_DC

	
extern void SetLastClumpfile(char* file, size_t size);	// in GDisplay.cpp, horrible bodge

	if (!clumping)
	{
		// load textures directly
		IndividualTextures = true;
		SetLastClumpfile("", 0);
	}
	else
	{

#ifdef TARGET_DC
		// Shouldn't be coming here.
		ASSERT ( FALSE );
#else

		// load textures from the clump
		char	filename[256];
		char*	leafname;

		do
		{
			leafname = fname_level;
			while (*fname_level && (*fname_level != '\\'))	fname_level++;
		} while (*fname_level++ == '\\');

		// write out
		sprintf(filename, "%sclumps\\", GetTexturePath());
		char*	fptr = filename + strlen(filename);
		while (*leafname != '.')		*fptr++ = *leafname++;
		strcpy(fptr, ".txc");

		OpenTGAClump(filename, TEXTURE_MAX_TEXTURES + 64*8, !TEXTURE_create_clump);
		IndividualTextures = false;
		SetLastClumpfile(filename, TEXTURE_MAX_TEXTURES + 64*8);
#endif
	}
}



void TEXTURE_load_needed(CBYTE*	fname_level,
						 int iStartCompletionBar,
						 int iEndCompletionBar,
						 int iNumberTexturesProbablyLoaded
						 )
{
	SLONG i,j,k;
	SLONG x;
	SLONG z;

	PrimFace3 *f3;
	PrimFace4 *f4;

	SLONG page;
	float u[4];
	float v[4];

	SLONG	c0,c1;

	MapElement *me;

extern UBYTE loading_screen_active;	// !



#define HOW_MANY_UPDATES 20
	int iNumTexturesLoaded = 0;
	int iNumTexturesToDoNextChunk = iNumberTexturesProbablyLoaded / HOW_MANY_UPDATES;
	int iCurChunkVal = iStartCompletionBar;

extern void ATTRACT_loadscreen_draw(SLONG completion);

#define LOADED_THIS_MANY_TEXTURES(numtex)														\
	iNumTexturesLoaded += numtex;																\
	while ( iNumTexturesLoaded > iNumTexturesToDoNextChunk )									\
	{																							\
		iNumTexturesToDoNextChunk += iNumberTexturesProbablyLoaded / HOW_MANY_UPDATES;			\
		iCurChunkVal += ( iEndCompletionBar - iStartCompletionBar ) / HOW_MANY_UPDATES;			\
		ATTRACT_loadscreen_draw ( iCurChunkVal );												\
	}



	TEXTURE_free();

	D3DTexture::BeginLoading();

	TEXTURE_initialise_clumping(fname_level);

#ifdef TARGET_DC
	bool bFrontEnd = FALSE;
	if ( ( fname_level == NULL ) || ( 0 == strcmp ( fname_level, "levels\\frontend.ucm" ) ) )
	{
		// Not actually a level - no level stuff loaded.
		bFrontEnd = TRUE;
	}
	// This should agree in theory.
	ASSERT ( loading_screen_active == !bFrontEnd );
#endif

	TEXTURE_load_page(1);

	//
	// Load all the unusual pages.
	//

	TEXTURE_page_num_standard    = TEXTURE_NUM_STANDARD + 0;

	TEXTURE_page_fog             = TEXTURE_NUM_STANDARD + 0;
	TEXTURE_page_moon            = TEXTURE_NUM_STANDARD + 1;
	TEXTURE_page_clouds          = TEXTURE_NUM_STANDARD + 2;
	TEXTURE_page_water           = TEXTURE_NUM_STANDARD + 3;
	TEXTURE_page_puddle          = TEXTURE_NUM_STANDARD + 4;
	TEXTURE_page_drip            = TEXTURE_NUM_STANDARD + 5;
	TEXTURE_page_shadow          = TEXTURE_NUM_STANDARD + 6;
	TEXTURE_page_bang            = TEXTURE_NUM_STANDARD + 7;
	TEXTURE_page_font            = TEXTURE_NUM_STANDARD + 8;
	TEXTURE_page_logo            = TEXTURE_NUM_STANDARD + 9;
	TEXTURE_page_sky             = TEXTURE_NUM_STANDARD + 10;
	TEXTURE_page_flames          = TEXTURE_NUM_STANDARD + 11;
	TEXTURE_page_smoke           = TEXTURE_NUM_STANDARD + 12;
	TEXTURE_page_flame2          = TEXTURE_NUM_STANDARD + 13;
	TEXTURE_page_steam           = TEXTURE_NUM_STANDARD + 14;
	TEXTURE_page_menuflame	     = TEXTURE_NUM_STANDARD + 15;
	TEXTURE_page_barbwire	     = TEXTURE_NUM_STANDARD + 16;
	TEXTURE_page_font2d		     = TEXTURE_NUM_STANDARD + 17;
	TEXTURE_page_face1		     = TEXTURE_NUM_STANDARD + 18;
	TEXTURE_page_face2		     = TEXTURE_NUM_STANDARD + 19;
	TEXTURE_page_face3		     = TEXTURE_NUM_STANDARD + 20;
	TEXTURE_page_face4		     = TEXTURE_NUM_STANDARD + 21;
	TEXTURE_page_face5		     = TEXTURE_NUM_STANDARD + 22;
	TEXTURE_page_face6		     = TEXTURE_NUM_STANDARD + 23;
	TEXTURE_page_bigbang	     = TEXTURE_NUM_STANDARD + 24;
	TEXTURE_page_dustwave	     = TEXTURE_NUM_STANDARD + 25;
	TEXTURE_page_flames3	     = TEXTURE_NUM_STANDARD + 26;
	TEXTURE_page_bloodsplat      = TEXTURE_NUM_STANDARD + 27;
	TEXTURE_page_bloom1          = TEXTURE_NUM_STANDARD + 28;
	TEXTURE_page_bloom2          = TEXTURE_NUM_STANDARD + 29;
	TEXTURE_page_hitspang        = TEXTURE_NUM_STANDARD + 30;
	TEXTURE_page_lensflare	     = TEXTURE_NUM_STANDARD + 31;
	TEXTURE_page_envmap          = TEXTURE_NUM_STANDARD + 32;
	TEXTURE_page_tyretrack	     = TEXTURE_NUM_STANDARD + 33;
	TEXTURE_page_winmap          = TEXTURE_NUM_STANDARD + 34;
	TEXTURE_page_leaf            = TEXTURE_NUM_STANDARD + 35;
	TEXTURE_page_raindrop        = TEXTURE_NUM_STANDARD + 36;
	TEXTURE_page_footprint       = TEXTURE_NUM_STANDARD + 37;
	TEXTURE_page_angel           = TEXTURE_NUM_STANDARD + 38;
	TEXTURE_page_devil           = TEXTURE_NUM_STANDARD + 39;
	TEXTURE_page_smoker		     = TEXTURE_NUM_STANDARD + 40;
	TEXTURE_page_target          = TEXTURE_NUM_STANDARD + 41;
	TEXTURE_page_newfont	     = TEXTURE_NUM_STANDARD + 42;
	TEXTURE_page_droplet         = TEXTURE_NUM_STANDARD + 43;
	TEXTURE_page_press1	         = TEXTURE_NUM_STANDARD + 44;
	TEXTURE_page_press2	         = TEXTURE_NUM_STANDARD + 45;
	TEXTURE_page_ic              = TEXTURE_NUM_STANDARD + 46;
	TEXTURE_page_ic2             = TEXTURE_NUM_STANDARD + 47;
	TEXTURE_page_explode1	     = TEXTURE_NUM_STANDARD + 48;
	TEXTURE_page_explode2	     = TEXTURE_NUM_STANDARD + 49;
	TEXTURE_page_lcdfont	     = TEXTURE_NUM_STANDARD + 50;
	TEXTURE_page_smokecloud	     = TEXTURE_NUM_STANDARD + 51;
	TEXTURE_page_menulogo	     = TEXTURE_NUM_STANDARD + 52;
	TEXTURE_page_polaroid	     = TEXTURE_NUM_STANDARD + 53;
	TEXTURE_page_sparkle	     = TEXTURE_NUM_STANDARD + 54;
	TEXTURE_page_bigbutton	     = TEXTURE_NUM_STANDARD + 55;
	TEXTURE_page_bigleaf  	     = TEXTURE_NUM_STANDARD + 56;
	TEXTURE_page_bigrain  	     = TEXTURE_NUM_STANDARD + 57;
	TEXTURE_page_finalglow       = TEXTURE_NUM_STANDARD + 58;
	TEXTURE_page_tinybutt	     = TEXTURE_NUM_STANDARD + 59;
	TEXTURE_page_tyretrack_alpha = TEXTURE_NUM_STANDARD + 60;
	TEXTURE_page_ladder          = TEXTURE_NUM_STANDARD + 61;
	TEXTURE_page_fadecat         = TEXTURE_NUM_STANDARD + 62;
	TEXTURE_page_shadowoval      = TEXTURE_NUM_STANDARD + 63;
	TEXTURE_page_rubbish         = TEXTURE_NUM_STANDARD + 64;
	TEXTURE_page_lastpanel       = TEXTURE_NUM_STANDARD + 65;
	TEXTURE_page_pcflamer		 = TEXTURE_NUM_STANDARD + 66;
	TEXTURE_page_sign            = TEXTURE_NUM_STANDARD + 67;
	TEXTURE_page_shadowsquare    = TEXTURE_NUM_STANDARD + 68;
	TEXTURE_page_lastpanel2      = TEXTURE_NUM_STANDARD + 69;
	TEXTURE_page_litebolt		 = TEXTURE_NUM_STANDARD + 70;
	TEXTURE_page_ladshad         = TEXTURE_NUM_STANDARD + 71;
	TEXTURE_page_meteor			 = TEXTURE_NUM_STANDARD + 72;
	TEXTURE_page_splash			 = TEXTURE_NUM_STANDARD + 73;
	TEXTURE_page_snowflake		 = TEXTURE_NUM_STANDARD + 74;
	TEXTURE_page_fade_MF         = TEXTURE_NUM_STANDARD + 75;

#ifdef TARGET_DC
	TEXTURE_page_joypad_a		 = TEXTURE_NUM_STANDARD + 76;
	TEXTURE_page_joypad_b		 = TEXTURE_NUM_STANDARD + 77;
	TEXTURE_page_joypad_c		 = TEXTURE_NUM_STANDARD + 78;
	TEXTURE_page_joypad_x		 = TEXTURE_NUM_STANDARD + 79;
	TEXTURE_page_joypad_y		 = TEXTURE_NUM_STANDARD + 80;
	TEXTURE_page_joypad_z		 = TEXTURE_NUM_STANDARD + 81;
	TEXTURE_page_joypad_l		 = TEXTURE_NUM_STANDARD + 82;
	TEXTURE_page_joypad_r		 = TEXTURE_NUM_STANDARD + 83;
	TEXTURE_page_joypad_pad_l	 = TEXTURE_NUM_STANDARD + 84;
	TEXTURE_page_joypad_pad_r	 = TEXTURE_NUM_STANDARD + 85;
	TEXTURE_page_joypad_pad_d	 = TEXTURE_NUM_STANDARD + 86;
	TEXTURE_page_joypad_pad_u	 = TEXTURE_NUM_STANDARD + 87;

	// Special-cased one.
	TEXTURE_page_background_use_instead = TEXTURE_NUM_STANDARD + 88;
	TEXTURE_page_background_use_instead2 = TEXTURE_NUM_STANDARD + 89;
#endif
	TEXTURE_num_textures         = TEXTURE_NUM_STANDARD + 90 + 20;

	

	//
	// Where we load the extra textures from.
	// 

#ifdef	NO_SERVER
	#define TEXTURE_EXTRA_DIR "server\\textures\\extras\\"
	#define TEXTURE_PEOPLE3_DIR "server\\textures\\shared\\people3\\"
#else
	#define TEXTURE_EXTRA_DIR "u:\\urbanchaos\\textures\\extras\\"
	#define TEXTURE_PEOPLE3_DIR "u:\\urbanchaos\\textures\\shared\\people3\\"

#endif

	//
	// Tell the font page its a font page.
	//

	TEXTURE_texture[TEXTURE_page_font].FontOn();
	TEXTURE_needed[TEXTURE_page_font] = 1;


	//
	// Tell the NEWFONT page it's a font page mark 2
	// (to remove red borders)
	//

	TEXTURE_texture[TEXTURE_page_lcdfont].Font2On();
	TEXTURE_needed[TEXTURE_page_lcdfont] = 1;

	TEXTURE_texture[TEXTURE_page_fog       ].LoadTextureTGA(TEXTURE_EXTRA_DIR"fog.tga", TEXTURE_page_fog);
	TEXTURE_texture[TEXTURE_page_moon      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"moon.tga", TEXTURE_page_moon);
	TEXTURE_texture[TEXTURE_page_clouds    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"clouds.tga", TEXTURE_page_clouds);
	TEXTURE_texture[TEXTURE_page_water     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"water.tga", TEXTURE_page_water);
	TEXTURE_texture[TEXTURE_page_puddle    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"puddle01.tga", TEXTURE_page_puddle);
LOADED_THIS_MANY_TEXTURES(5);
	TEXTURE_texture[TEXTURE_page_drip      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"drip.tga", TEXTURE_page_drip);
	TEXTURE_texture[TEXTURE_page_shadow    ].CreateUserPage(TEXTURE_SHADOW_SIZE, the_display.GetDeviceInfo()->DestInvSourceColourSupported() ? FALSE : TRUE);
	TEXTURE_texture[TEXTURE_page_bang      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"fireball.tga", TEXTURE_page_bang);
	TEXTURE_texture[TEXTURE_page_font      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"font.tga", TEXTURE_page_font,FALSE);
	TEXTURE_needed[TEXTURE_page_font] = 1;
LOADED_THIS_MANY_TEXTURES(5);
	//TEXTURE_texture[TEXTURE_page_logo      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"logo3.tga", TEXTURE_page_logo);
	{
		CBYTE	str[100];
//		sprintf(str,TEXTURE_EXTRA_DIR"sky%d.tga",Random()&3);
		sprintf(str, "%ssky.tga",   TEXTURE_world_dir);

		// Until the VQ quality improves, don't VQ the sky - it looks pretty grim.
		TEXTURE_texture[TEXTURE_page_sky       ].LoadTextureTGA(str,TEXTURE_page_sky,FALSE);
	}
//	TEXTURE_texture[TEXTURE_page_sky       ].LoadTextureTGA(TEXTURE_EXTRA_DIR"sky2.tga", TEXTURE_page_sky);

	TEXTURE_texture[TEXTURE_page_flames    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"flame1.tga", TEXTURE_page_flames);
	TEXTURE_texture[TEXTURE_page_smoke     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"smoke1.tga", TEXTURE_page_smoke);
//	TEXTURE_texture[TEXTURE_page_flame2    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"flame2.tga", TEXTURE_page_flame2);
	TEXTURE_texture[TEXTURE_page_flame2    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"explode4.tga", TEXTURE_page_flame2);
	TEXTURE_texture[TEXTURE_page_steam     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"fog_na.tga", TEXTURE_page_steam);
	TEXTURE_texture[TEXTURE_page_barbwire  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"barbed.tga", TEXTURE_page_barbwire);
LOADED_THIS_MANY_TEXTURES(6);

#ifdef TARGET_DC
	TEXTURE_texture[TEXTURE_page_font2d    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"multifontPC.tga", TEXTURE_page_font2d, FALSE);
#else
	if (SOFTWARE)
	{
		TEXTURE_texture[TEXTURE_page_font2d    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"multifontPC640.tga", TEXTURE_page_font2d);
	}
	else
	{
		TEXTURE_texture[TEXTURE_page_font2d    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"multifontPC.tga", TEXTURE_page_font2d, FALSE);
	}
#endif
	TEXTURE_needed[TEXTURE_page_font2d] = 1;

	//	TEXTURE_texture[TEXTURE_page_lcdfont   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"font3.tga", TEXTURE_page_
#ifdef TARGET_DC
	TEXTURE_texture[TEXTURE_page_lcdfont   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"olyfont2dc.tga", TEXTURE_page_lcdfont, FALSE);
#else
	TEXTURE_texture[TEXTURE_page_lcdfont   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"olyfont2.tga", TEXTURE_page_lcdfont, FALSE);
#endif
	TEXTURE_needed[TEXTURE_page_lcdfont] = 1;


	TEXTURE_texture[TEXTURE_page_lastpanel   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"PCdisplay.tga",    TEXTURE_page_lastpanel, FALSE);
	TEXTURE_needed[TEXTURE_page_lastpanel] = 1;

	FONT2D_init(TEXTURE_page_font2d);	// do it now so it's still in the CD-ROM cache
	TEXTURE_texture[TEXTURE_page_face1     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"face1.tga", TEXTURE_page_face1);
	TEXTURE_texture[TEXTURE_page_face2     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"face2.tga", TEXTURE_page_face2);
LOADED_THIS_MANY_TEXTURES(5);
	//TEXTURE_texture[TEXTURE_page_face3     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"face3.tga", TEXTURE_page_face3);
	//TEXTURE_texture[TEXTURE_page_face4     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"face4.tga", TEXTURE_page_face4);
	//TEXTURE_texture[TEXTURE_page_face5     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"face5.tga", TEXTURE_page_face5);
	//TEXTURE_texture[TEXTURE_page_face6     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"face6.tga", TEXTURE_page_face6);
	TEXTURE_texture[TEXTURE_page_bigbang   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"exp_gunk.tga", TEXTURE_page_bigbang);
//	TEXTURE_texture[TEXTURE_page_dustwave  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"dustwave.tga", TEXTURE_page_dustwave);
	TEXTURE_texture[TEXTURE_page_dustwave  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"shockwave.tga", TEXTURE_page_dustwave);
	TEXTURE_texture[TEXTURE_page_flames3   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"flame3.tga", TEXTURE_page_flames3);
	TEXTURE_texture[TEXTURE_page_bloodsplat].LoadTextureTGA(TEXTURE_EXTRA_DIR"bludsplt.tga", TEXTURE_page_bloodsplat);
//	TEXTURE_texture[TEXTURE_page_bloom1    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"bloom1.tga", TEXTURE_page_bloom1);
//	TEXTURE_texture[TEXTURE_page_bloom1    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"bloom3.tga", TEXTURE_page_bloom1);
	TEXTURE_texture[TEXTURE_page_bloom1    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"bloom4.tga", TEXTURE_page_bloom1);
LOADED_THIS_MANY_TEXTURES(5);
	TEXTURE_texture[TEXTURE_page_bloom2    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"bloom2.tga", TEXTURE_page_bloom2);
	TEXTURE_texture[TEXTURE_page_hitspang  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"hitspang.tga", TEXTURE_page_hitspang);
	TEXTURE_texture[TEXTURE_page_lensflare ].LoadTextureTGA(TEXTURE_EXTRA_DIR"lensflar.tga", TEXTURE_page_lensflare);
	TEXTURE_texture[TEXTURE_page_envmap    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"envmap.tga", TEXTURE_page_envmap);
	TEXTURE_texture[TEXTURE_page_tyretrack ].LoadTextureTGA(TEXTURE_EXTRA_DIR"tyremark.tga", TEXTURE_page_tyretrack);
LOADED_THIS_MANY_TEXTURES(5);
	TEXTURE_texture[TEXTURE_page_winmap    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"winmap.tga", TEXTURE_page_winmap);
	TEXTURE_texture[TEXTURE_page_leaf      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"leaf.tga", TEXTURE_page_leaf);
	TEXTURE_texture[TEXTURE_page_raindrop  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"raindrop.tga", TEXTURE_page_raindrop);
	TEXTURE_texture[TEXTURE_page_footprint ].LoadTextureTGA(TEXTURE_EXTRA_DIR"footprint.tga", TEXTURE_page_footprint);
	//TEXTURE_texture[TEXTURE_page_angel     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"angel.tga", TEXTURE_page_angel);
	//TEXTURE_texture[TEXTURE_page_devil     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"devil.tga", TEXTURE_page_devil);
	TEXTURE_texture[TEXTURE_page_smoker	   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"smoker2.tga", TEXTURE_page_smoker);
	TEXTURE_texture[TEXTURE_page_target	   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"targ1.tga", TEXTURE_page_target);
	//TEXTURE_texture[TEXTURE_page_newfont   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"multifontPC.tga", TEXTURE_page_newfont);
	TEXTURE_texture[TEXTURE_page_droplet   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"droplet.tga", TEXTURE_page_droplet);
LOADED_THIS_MANY_TEXTURES(7);
	//TEXTURE_texture[TEXTURE_page_press1    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"press1.tga", TEXTURE_page_press1);
	//TEXTURE_texture[TEXTURE_page_press2    ].LoadTextureTGA(TEXTURE_EXTRA_DIR"press2.tga", TEXTURE_page_press2);
	//TEXTURE_texture[TEXTURE_page_ic        ].LoadTextureTGA(TEXTURE_EXTRA_DIR"ic5.tga", TEXTURE_page_ic);
	//TEXTURE_texture[TEXTURE_page_ic2       ].LoadTextureTGA(TEXTURE_EXTRA_DIR"ic2_6.tga", TEXTURE_page_ic2);
	TEXTURE_texture[TEXTURE_page_explode1  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"explode1.tga", TEXTURE_page_explode1);
	TEXTURE_texture[TEXTURE_page_explode2  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"explode2.tga", TEXTURE_page_explode2);
	TEXTURE_texture[TEXTURE_page_smokecloud].LoadTextureTGA(TEXTURE_EXTRA_DIR"explode3.tga", TEXTURE_page_smokecloud);
	TEXTURE_texture[TEXTURE_page_menulogo  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"menulogo.tga", TEXTURE_page_menulogo);
	TEXTURE_needed[TEXTURE_page_menulogo] = 1;
LOADED_THIS_MANY_TEXTURES(4);
	//TEXTURE_texture[TEXTURE_page_polaroid  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"photos\\police1.tga", TEXTURE_page_polaroid);
	TEXTURE_texture[TEXTURE_page_sparkle   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"sparkle.tga", TEXTURE_page_sparkle);
	TEXTURE_texture[TEXTURE_page_pcflamer  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"PCflamer.tga", TEXTURE_page_pcflamer);
	TEXTURE_texture[TEXTURE_page_litebolt  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"litebolt2.tga", TEXTURE_page_litebolt);
	TEXTURE_texture[TEXTURE_page_splash	   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"splashALL.tga", TEXTURE_page_splash);
LOADED_THIS_MANY_TEXTURES(4);

	// frontend stuff...
	TEXTURE_texture[TEXTURE_page_bigbutton ].LoadTextureTGA(TEXTURE_EXTRA_DIR"bigbutt.tga", TEXTURE_page_bigbutton, FALSE);
	TEXTURE_needed [TEXTURE_page_bigbutton] = 1;
	TEXTURE_texture[TEXTURE_page_bigleaf   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"bigleaf.tga", TEXTURE_page_bigleaf);
	TEXTURE_needed [TEXTURE_page_bigleaf] = 1;
	TEXTURE_texture[TEXTURE_page_bigrain   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"raindrop2.tga", TEXTURE_page_bigrain);
	TEXTURE_needed [TEXTURE_page_bigrain] = 1;
	TEXTURE_texture[TEXTURE_page_tinybutt  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"tinybutt.tga", TEXTURE_page_tinybutt, FALSE);
	TEXTURE_needed [TEXTURE_page_tinybutt] = 1;
	TEXTURE_texture[TEXTURE_page_snowflake ].LoadTextureTGA(TEXTURE_EXTRA_DIR"snowflake.tga", TEXTURE_page_snowflake);
	TEXTURE_needed [TEXTURE_page_snowflake] = 1;

	TEXTURE_texture[TEXTURE_page_finalglow ].LoadTextureTGA(TEXTURE_EXTRA_DIR"finalglow.tga", TEXTURE_page_finalglow);
	TEXTURE_needed [TEXTURE_page_finalglow] = 1;

	// Used for the screensaver. Don't use the MVQ - it doesn't get 100% black.
	TEXTURE_texture[TEXTURE_page_fade_MF     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"fade_MF.tga",      TEXTURE_page_fade_MF, FALSE);
	TEXTURE_needed [TEXTURE_page_fade_MF] = 1;

LOADED_THIS_MANY_TEXTURES(7);


#ifdef TARGET_DC
	// The joypad button textures.
	TEXTURE_texture[TEXTURE_page_joypad_a].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_A.tga", TEXTURE_page_joypad_a);
	TEXTURE_needed [TEXTURE_page_joypad_a] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_b].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_B.tga", TEXTURE_page_joypad_b);
	TEXTURE_needed [TEXTURE_page_joypad_b] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_c].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_C.tga", TEXTURE_page_joypad_c);
	TEXTURE_needed [TEXTURE_page_joypad_c] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_x].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_X.tga", TEXTURE_page_joypad_x);
	TEXTURE_needed [TEXTURE_page_joypad_x] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_y].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_Y.tga", TEXTURE_page_joypad_y);
	TEXTURE_needed [TEXTURE_page_joypad_y] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_z].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_Z.tga", TEXTURE_page_joypad_z);
	TEXTURE_needed [TEXTURE_page_joypad_z] = 1;
LOADED_THIS_MANY_TEXTURES(6);
	TEXTURE_texture[TEXTURE_page_joypad_l].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_LEFT.tga",  TEXTURE_page_joypad_l);
	TEXTURE_needed [TEXTURE_page_joypad_l] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_r].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_RIGHT.tga", TEXTURE_page_joypad_r);
	TEXTURE_needed [TEXTURE_page_joypad_r] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_pad_l].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_PADLEFT.tga",  TEXTURE_page_joypad_pad_l);
	TEXTURE_needed [TEXTURE_page_joypad_pad_l] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_pad_r].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_PADRIGHT.tga", TEXTURE_page_joypad_pad_r);
	TEXTURE_needed [TEXTURE_page_joypad_pad_r] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_pad_d].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_PADDOWN.tga",  TEXTURE_page_joypad_pad_d);
	TEXTURE_needed [TEXTURE_page_joypad_pad_d] = 1;
	TEXTURE_texture[TEXTURE_page_joypad_pad_u].LoadTextureTGA(TEXTURE_EXTRA_DIR"DC\\button_PADUP.tga",    TEXTURE_page_joypad_pad_u);
	TEXTURE_needed [TEXTURE_page_joypad_pad_u] = 1;
LOADED_THIS_MANY_TEXTURES(6);
#endif


#ifdef TARGET_DC
	// Only loaded during an actual level.
	if ( !bFrontEnd )
#endif
	{

		TEXTURE_texture[TEXTURE_page_fadecat     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"fadecat.tga",      TEXTURE_page_fadecat, FALSE);

		TEXTURE_texture[TEXTURE_page_tyretrack_alpha	].LoadTextureTGA(TEXTURE_EXTRA_DIR"tyremark_alpha.tga", TEXTURE_page_tyretrack_alpha);

		TEXTURE_texture[TEXTURE_page_ladder      ].LoadTextureTGA(TEXTURE_EXTRA_DIR"secret.tga",       TEXTURE_page_ladder);
		TEXTURE_texture[TEXTURE_page_shadowoval  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"shadow.tga",       TEXTURE_page_shadowoval);
		TEXTURE_texture[TEXTURE_page_rubbish     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"rubbish.tga",      TEXTURE_page_rubbish);

LOADED_THIS_MANY_TEXTURES(5);

		// Done above.
		//TEXTURE_texture[TEXTURE_page_lastpanel   ].LoadTextureTGA(TEXTURE_EXTRA_DIR"PCdisplay.tga",    TEXTURE_page_lastpanel, FALSE);
		TEXTURE_texture[TEXTURE_page_lastpanel2  ].LoadTextureTGA(TEXTURE_EXTRA_DIR"PCdisplay01.tga",  TEXTURE_page_lastpanel2, FALSE);

		TEXTURE_texture[TEXTURE_page_sign        ].LoadTextureTGA(TEXTURE_EXTRA_DIR"signs.tga",        TEXTURE_page_sign);
		TEXTURE_texture[TEXTURE_page_shadowsquare].LoadTextureTGA(TEXTURE_EXTRA_DIR"shadowsquare.tga", TEXTURE_page_shadowsquare);
		TEXTURE_texture[TEXTURE_page_ladshad     ].LoadTextureTGA(TEXTURE_EXTRA_DIR"ladshad.tga",      TEXTURE_page_ladshad);
		TEXTURE_texture[TEXTURE_page_meteor		 ].LoadTextureTGA(TEXTURE_EXTRA_DIR"meteorALL.tga", TEXTURE_page_meteor);

LOADED_THIS_MANY_TEXTURES(5);

		//
		// Male civs
		//
		TEXTURE_page_people3	  = 21*64;
		TEXTURE_texture[TEXTURE_page_people3+0	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"crotch1.tga", TEXTURE_page_people3+0);
		TEXTURE_texture[TEXTURE_page_people3+1	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"crotch2.tga", TEXTURE_page_people3+1);
		TEXTURE_texture[TEXTURE_page_people3+2	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"crotch3.tga", TEXTURE_page_people3+2);
		TEXTURE_texture[TEXTURE_page_people3+3	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"front1.tga", TEXTURE_page_people3+3);
		TEXTURE_texture[TEXTURE_page_people3+4	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"front2.tga", TEXTURE_page_people3+4);
LOADED_THIS_MANY_TEXTURES(5);
		TEXTURE_texture[TEXTURE_page_people3+5	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"front3.tga", TEXTURE_page_people3+5);
		TEXTURE_texture[TEXTURE_page_people3+6	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"hatside1.tga", TEXTURE_page_people3+6);
		TEXTURE_texture[TEXTURE_page_people3+7	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"hatside2.tga", TEXTURE_page_people3+7);
		TEXTURE_texture[TEXTURE_page_people3+8	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"hatside3.tga", TEXTURE_page_people3+8);
		TEXTURE_texture[TEXTURE_page_people3+9	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"hattop1.tga", TEXTURE_page_people3+9);
		TEXTURE_texture[TEXTURE_page_people3+10	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"hattop2.tga", TEXTURE_page_people3+10);
LOADED_THIS_MANY_TEXTURES(5);
		TEXTURE_texture[TEXTURE_page_people3+11	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"hattop3.tga", TEXTURE_page_people3+11);
		TEXTURE_texture[TEXTURE_page_people3+12	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"leg1.tga", TEXTURE_page_people3+12);
		TEXTURE_texture[TEXTURE_page_people3+13	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"leg2.tga", TEXTURE_page_people3+13);
		TEXTURE_texture[TEXTURE_page_people3+14	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"leg3.tga", TEXTURE_page_people3+14);
LOADED_THIS_MANY_TEXTURES(4);

		//
		// Female Civs
		//
		TEXTURE_texture[TEXTURE_page_people3+15	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMARSE1.tga", TEXTURE_page_people3+15);
		TEXTURE_texture[TEXTURE_page_people3+16	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMARSE2.tga", TEXTURE_page_people3+16);
		TEXTURE_texture[TEXTURE_page_people3+17	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMARSE3.tga", TEXTURE_page_people3+17);

		TEXTURE_texture[TEXTURE_page_people3+18	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMCHEST1.tga", TEXTURE_page_people3+18);
		TEXTURE_texture[TEXTURE_page_people3+19	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMCHEST2.tga", TEXTURE_page_people3+19);
		TEXTURE_texture[TEXTURE_page_people3+20	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMCHEST3.tga", TEXTURE_page_people3+20);
LOADED_THIS_MANY_TEXTURES(6);
		TEXTURE_texture[TEXTURE_page_people3+21	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"SEAM1.tga", TEXTURE_page_people3+21);
		TEXTURE_texture[TEXTURE_page_people3+22	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"SEAM2.tga", TEXTURE_page_people3+22);
		TEXTURE_texture[TEXTURE_page_people3+23	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"SEAM3.tga", TEXTURE_page_people3+23);

		TEXTURE_texture[TEXTURE_page_people3+24	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMSHOO1.tga", TEXTURE_page_people3+24);
		TEXTURE_texture[TEXTURE_page_people3+25	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMSHOO2.tga", TEXTURE_page_people3+25);
		TEXTURE_texture[TEXTURE_page_people3+26	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMSHOO3.tga", TEXTURE_page_people3+26);
LOADED_THIS_MANY_TEXTURES(6);

		TEXTURE_texture[TEXTURE_page_people3+27	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMBAK1.tga", TEXTURE_page_people3+27);
		TEXTURE_texture[TEXTURE_page_people3+28	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMBAK2.tga", TEXTURE_page_people3+28);
		TEXTURE_texture[TEXTURE_page_people3+29	].LoadTextureTGA(TEXTURE_PEOPLE3_DIR"FEMBAK3.tga", TEXTURE_page_people3+29);
LOADED_THIS_MANY_TEXTURES(3);
	}



#ifdef TARGET_DC
	// Set this up, but don't do anything with it. The actual texture is overridden later.
	the_display.AddLoadedTexture(&TEXTURE_texture[TEXTURE_page_background_use_instead]);
	TEXTURE_needed [TEXTURE_page_background_use_instead] = 1;
	the_display.AddLoadedTexture(&TEXTURE_texture[TEXTURE_page_background_use_instead2]);
	TEXTURE_needed [TEXTURE_page_background_use_instead2] = 1;
#endif





#ifndef TARGET_DC
#if 1
	// Load and bin a load of textures used by the DC, but not the PC.
	// This makes sure they get converted to MVQ format.
	D3DTexture *pTex;

#define DO_DC_CONVERT(name) pTex = MFnew<D3DTexture>(); pTex->LoadTextureTGA ( (name), -1, TRUE ); MFdelete ( pTex )

	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button A.tga"			);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button B.tga"			);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button C.tga"			);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button X.tga"			);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button Y.tga"			);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button Z.tga"			);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button LEFT.tga"		);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button RIGHT.tga"		);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button PADLEFT.tga"		);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button PADRIGHT.tga"	);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button PADDOWN.tga"		);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\button PADUP.tga"		);
	DO_DC_CONVERT(TEXTURE_EXTRA_DIR"DC\\page_joybutts.tga"		);
	

	DO_DC_CONVERT("server\\textures\\shared\\people\\page_darci1.tga");
	DO_DC_CONVERT("server\\textures\\extras\\page_misc_alpha.tga");

#undef DO_DC_CONVERT

#endif
#endif



#if 0
	if (loading_screen_active)
	{
		ATTRACT_loadscreen_draw(70 * 256 / 100);
	}
#endif

	//
	// The video page.
	// 

//not used anymore?	TEXTURE_texture[86].CreateUserPage(TEXTURE_VIDEO_SIZE, FALSE);

	//
	// The flames on the main menu
	//

//not used anymore?    TEXTURE_texture[TEXTURE_page_menuflame].CreateUserPage(256,FALSE);


	//
	// The leaves!
	//
	// Fins' glows above the traffic cones...
	// The water droplets and the sparkles, the sewer water and the raindrops.
	// The man on the moon!
	// And the muckyfootprints
	// Texture 560 is one component of the digital timer.
	//

	TRACE("Loaded extras\n");

	//
	// The warehouse textures.
	//
#ifdef TARGET_DC
	// Only loaded during an actual level.
	if ( !bFrontEnd )
#endif
	{

		for (i = 0; i < WARE_rooftex_upto; i++)
		{
			TEXTURE_get_minitexturebits_uvs(
				WARE_rooftex[i],
			   &page,
				u + 0,
				v + 0,
				u + 1,
				v + 1,
				u + 2,
				v + 2,
				u + 3,
				v + 3);

			if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
			{
				//
				// We must load this texture.
				//

				TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
			}
		}

		/*

		//
		// do all the inside styles for now
		//

		for(c0=0;c0<64;c0++)
		{
			for(c1=0;c1<16;c1++)
			{
				page=inside_tex[c0][c1]+START_PAGE_FOR_FLOOR*64;

				if(page<8*64)
				if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
				{
					TEXTURE_load_page(page);

				}
			}
		}

		*/

		TRACE("Loaded inside styles\n");


		//
		// Load the individual pages that we need.
		//

		for (x = 0; x < MAP_WIDTH  - 1; x++)
		for (z = 0; z < MAP_HEIGHT - 1; z++)
		{
			TEXTURE_get_minitexturebits_uvs(
				PAP_2HI(x,z).Texture,
			   &page,
				u + 0,
				v + 0,
				u + 1,
				v + 1,
				u + 2,
				v + 2,
				u + 3,
				v + 3);

			ASSERT(WITHIN(page, 0, TEXTURE_page_num_standard - 1));

			if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
			{
				//
				// We must load this texture.
				//

				TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
			}
		}
	//	TEXTURE_load_page(156);
	//	TEXTURE_load_page(157);

		TRACE("Loaded floor textures\n");

#if 0
		if (loading_screen_active)
		{
			ATTRACT_loadscreen_draw(75 * 256 / 100);
		}
#endif

		for (i = 1; i < MAX_ANIM_TMAPS; i++)
		{
			struct	AnimTmap	*p_a;

			p_a=&anim_tmaps[i];

			for(k=0;k<MAX_TMAP_FRAMES;k++)
			{
				page   = p_a->UV[k][0][0] & 0xc0;
				page <<= 2;
				page  |= p_a->Page[k];
			}
		}

		TRACE("Loaded anim tmaps\n");

		//
		// force jacket alternatives to be loaded thugs
		//


		TEXTURE_load_page(18*64+2);
		TEXTURE_load_page(18*64+32);

		TEXTURE_load_page(18*64+3);
		TEXTURE_load_page(18*64+33);
LOADED_THIS_MANY_TEXTURES(4);

		TEXTURE_load_page(18*64+4);
		TEXTURE_load_page(18*64+36);

		TEXTURE_load_page(18*64+5);
		TEXTURE_load_page(18*64+37);
LOADED_THIS_MANY_TEXTURES(4);

		for (i = 1; i < next_prim_face3; i++)
		{
			f3 = &prim_faces3[i];

			page   = f3->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= f3->TexturePage;
			page+=FACE_PAGE_OFFSET;

			if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
			{
				//
				// We must load this texture.
				//

				TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
			}
		}

#if 0
		if (loading_screen_active)
		{
			ATTRACT_loadscreen_draw(80 * 256 / 100);
		}
#endif

		TRACE("Loaded prim 3s\n");

		for (i = 1; i < next_prim_face4; i++)
		{
			f4 = &prim_faces4[i];

			page   = f4->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= f4->TexturePage;
			page+=FACE_PAGE_OFFSET;

			if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
			{
				//
				// We must load this texture.
				//

				TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
			}
		}

		TRACE("Loaded prim 4s\n");

#if 0
		if (loading_screen_active)
		{
			ATTRACT_loadscreen_draw(85 * 256 / 100);
		}
#endif

		TEXTURE_load_page(156);

		for(i=1;i<next_dfacet;i++)
		{
			SLONG	c0,c1,c2;
			SLONG	style,dstyle;

				//ASSERT(dfacets[i].FacetType != STOREY_TYPE_OUTSIDE_DOOR);
			if (dfacets[i].FacetType == STOREY_TYPE_NORMAL ||
				dfacets[i].FacetType == STOREY_TYPE_INSIDE ||
				dfacets[i].FacetType == STOREY_TYPE_OINSIDE ||
				dfacets[i].FacetType == STOREY_TYPE_FENCE||
				dfacets[i].FacetType == STOREY_TYPE_FENCE_FLAT||
				dfacets[i].FacetType == STOREY_TYPE_LADDER||
				dfacets[i].FacetType == STOREY_TYPE_DOOR||
				dfacets[i].FacetType == STOREY_TYPE_OUTSIDE_DOOR||
				dfacets[i].FacetType == STOREY_TYPE_INSIDE_DOOR||
				dfacets[i].FacetType == STOREY_TYPE_FENCE_BRICK
				)
			{
				style = dfacets[i].StyleIndex;

				dstyle=dstyles[style];

				for(c0=0;c0<((dfacets[i].Height+3)>>2)*((dfacets[i].FacetFlags&FACET_FLAG_2SIDED)?2:1);c0++)
				{
					SLONG	dstyle;
					dstyle=dstyles[style+c0];

					if(dstyle>0)
					{
						for(c1=0;c1<TEXTURE_PIECE_NUMBER;c1++)
						{
							page=dx_textures_xy[dstyle][c1].Page;
							if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
							{
								//
								// We must load this texture.
								//

								TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
							}
						}
					}
					else
					{
						struct	DStorey *p_storey;
						SLONG	pos;

						p_storey=&dstoreys[-dstyle];

						for(pos=0;pos<p_storey->Count;pos++)
						{
							page=paint_mem[p_storey->Index+pos];
							if(page)
							if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
							{
								//
								// We must load this texture.
								//

								TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
							}

						}
						dstyle=p_storey->Style;
						if(dstyle>0)
						{
							for(c1=0;c1<TEXTURE_PIECE_NUMBER;c1++)
							{
								page=dx_textures_xy[dstyle][c1].Page;
								if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
								{
									//
									// We must load this texture.
									//

									TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
								}
							}
						}




					}
				}
			}
			if (dfacets[i].FacetType == STOREY_TYPE_LADDER)
			{
				SLONG	dstyle;
				dstyle=dstyles[dfacets[i].StyleIndex];
				if(dstyle>0)
				for(c1=0;c1<TEXTURE_PIECE_NUMBER;c1++)
				{
					page=dx_textures_xy[dstyle][c1].Page;
					if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
					{
						//
						// We must load this texture.
						//

						TEXTURE_load_page(page);
LOADED_THIS_MANY_TEXTURES(1);
					}
				}


			}
		}

		TRACE("Loaded facet pages\n");

#if 0
		if (loading_screen_active)
		{
			ATTRACT_loadscreen_draw(90 * 256 / 100);
		}
#endif

		/*

		//
		// The sewer pages.
		//

	#if !defined(TARGET_DC)
		for (i = 0; i < NS_PAGE_NUMBER; i++)
		{	
			page = NS_page[i].page;

			if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
			{
				TEXTURE_load_page(page);
			}
		}
	#endif

		TRACE("Loaded sewer pages\n");

		for(c0=0;c0<64;c0++)
		{
			for(c1=0;c1<16;c1++)
			{
				SLONG	page;
				page=inside_tex[c0][c1]+START_PAGE_FOR_FLOOR*64;

				if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
				{
					TEXTURE_load_page(page);
				}
			}
		}

		*/
	}


	TRACE("Finished loading everything\n");

#ifndef TARGET_DC
	CloseTGAClump();
#endif



#ifdef TRUETYPE
	TT_Init();
#endif
	
	#if 0	// Didn't work anyway...

	//
	// Draw a poly with each texture page to get all the data actually
	// downloaded to the card.
	//

	if (loading_screen_active)
	{
		BEGIN_SCENE;

		POLY_frame_init(FALSE, FALSE);

		{
			SLONG i;
			SLONG x;
			SLONG y;

			for (i = 0; i < POLY_NUM_PAGES; i++)
			{
				extern void PANEL_draw_quad(
								float left,
								float top,
								float right,
								float bottom,
								SLONG page,
								ULONG colour = 0xffffffff,
								float u1 = 0.0F,
								float v1 = 0.0F,
								float u2 = 1.0F,
								float v2 = 1.0F);

				x = (i & 0x7f) << 2;
				y = (i >> 6) << 3;

				PANEL_draw_quad(
					x, y,
					x + 2, y + 6,
					i);
			}
		}

		POLY_frame_draw(TRUE, TRUE);

		END_SCENE;

		ATTRACT_loadscreen_draw(90 * 256 / 100);
	}

	#endif

#ifndef TARGET_DC
	// this is a good point to estimate the
	// graphics card's capabilities
extern void AENG_guess_detail_levels();

	AENG_guess_detail_levels();
#endif


	// Guess what this does.
	NotGoingToLoadTexturesForAWhileNowSoYouCanCleanUpABit();


#if USE_TOMS_ENGINE_PLEASE_BOB
	// Start a new frame, so that the new textures get set up and sorted.
	// Yes, it is crufty why starting a new frame does this. But it's historical.
	// Just trust me, OK?
	POLY_frame_init(FALSE,FALSE);
#endif

}

void TEXTURE_load_needed_object(SLONG prim)
{
	SLONG i;
	SLONG page;

	PrimObject *po;
	PrimFace3  *f3;
	PrimFace4  *f4;

	po = &prim_objects[prim];

	for (i = po->StartFace3; i < po->EndFace3; i++)
	{
		f3 = &prim_faces3[i];

		page   = f3->UV[0][0] & 0xc0;
		page <<= 2;
		page  |= f3->TexturePage;

		if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
		{
			//
			// We must load this texture.
			//

			TEXTURE_load_page(page);
		}
	}

	for (i = po->StartFace4; i < po->EndFace4; i++)
	{
		f4 = &prim_faces4[i];

		page   = f4->UV[0][0] & 0xc0;
		page <<= 2;
		page  |= f4->TexturePage;

		if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
		{
			//
			// We must load this texture.
			//

			TEXTURE_load_page(page);
		}
	}
}



void TEXTURE_free()
{
	SLONG i;

	//
	// Initialise all the crinkles.
	//
	
	CRINKLE_init();

	for (i = 0; i < TEXTURE_num_textures; i++)
	{
		TEXTURE_texture[i].Destroy();
		TEXTURE_texture[i].Type = D3DTEXTURE_TYPE_UNUSED;
	}

	the_display.RemoveAllLoadedTextures();

	memset(TEXTURE_dontexist, 0, sizeof(TEXTURE_dontexist));
	memset ( TEXTURE_needed,	0, sizeof ( TEXTURE_needed ) );

	POLY_reset_render_states();

	TEXTURE_DC_pack_init();

#ifdef TRUETYPE
	TT_Term();
#endif
}


// Destroys all the non-needed (i.e. non-frontend) textures.
void TEXTURE_free_unneeded ( void )
{
	SLONG i;

	//
	// Initialise all the crinkles.
	//
	
	CRINKLE_init();

#ifdef DEBUG
	DDSCAPS2 ddsc;
	ddsc.dwCaps = DDSCAPS_TEXTURE;
	ddsc.dwCaps2 = 0;
	ddsc.dwCaps3 = 0;
	ddsc.dwCaps4 = 0;
	DWORD dwFree, dwTotal;
	HRESULT hres = the_display.lp_DD4->GetAvailableVidMem ( &ddsc, &dwTotal, &dwFree );
	TRACE ( "Memory before TEXTURE_free_unneeded: %ikb\n", dwFree / 1024 );
#endif

	for (i = 0; i < TEXTURE_num_textures; i++)
	{
		//if ( !TEXTURE_needed[i] )
		{
			TEXTURE_texture[i].Destroy();
			TEXTURE_texture[i].Type = D3DTEXTURE_TYPE_UNUSED;
			TEXTURE_dontexist[i] = 0;
		}
	}

	// Now free all the texture pages.
extern void FreeAllD3DPages ( void );
	FreeAllD3DPages();

#ifdef DEBUG
	ddsc.dwCaps = DDSCAPS_TEXTURE;
	ddsc.dwCaps2 = 0;
	ddsc.dwCaps3 = 0;
	ddsc.dwCaps4 = 0;
	dwFree, dwTotal;
	hres = the_display.lp_DD4->GetAvailableVidMem ( &ddsc, &dwTotal, &dwFree );
	ASSERT ( SUCCEEDED ( hres ) );
	TRACE ( "After freeing: %ikb\n", dwFree / 1024 );
#endif

	POLY_reset_render_states();
}


#if 0	// not DX6

D3DTEXTUREHANDLE TEXTURE_get_handle(SLONG page)
{
	D3DTEXTUREHANDLE ans;

	ASSERT(WITHIN(page, 0, TEXTURE_num_textures - 1));

	ans = TEXTURE_texture[page].GetTextureHandle();

	return ans;
}

#endif



LPDIRECT3DTEXTURE2 TEXTURE_get_handle(SLONG page)
{
#ifdef TARGET_DC
	if ( page == TEXTURE_page_background_use_instead )
	{
		// Special-cased for the background replacement.
		return ( the_display.lp_DD_Background_use_instead_texture );
	}
	else if ( page == TEXTURE_page_background_use_instead2 )
	{
		// Special-cased for the background replacement.
		return ( the_display.lp_DD_Background_use_instead_texture2 );
	}
#endif
	if ( page == -1 )
	{
		return ( NULL );
	}
#ifdef TARGET_DC
	ASSERT(WITHIN(page, 0, TEXTURE_num_textures - 1));
#endif
	return TEXTURE_texture[page].GetD3DTexture();
}





#ifdef TEX_EMBED
#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

D3DTexture *TEXTURE_get_D3DTexture(SLONG page)
{
	return &(TEXTURE_texture[page]);
}

#else //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

UBYTE TEXTURE_get_offset(SLONG page)
{
#ifdef TARGET_DC
	if ( ( page == TEXTURE_page_background_use_instead ) || ( page == TEXTURE_page_background_use_instead2 ) )
	{
		// Special-cased for the background replacement.
		return ( 0 );
	}
#endif
	if ( page == -1 )
	{
		return ( 0 );
	}
	ASSERT(WITHIN(page, 0, TEXTURE_num_textures - 1));
	return TEXTURE_texture[page].GetTexOffset();
}
#endif //#else //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

#endif

void TEXTURE_get_minitexturebits_uvs(
		UWORD           texture,
		SLONG          *page,
		float          *u0,
		float          *v0,
		float          *u1,
		float          *v1,
		float          *u2,
		float          *v2,
		float          *u3,
		float          *v3)
{
	SLONG tx;
	SLONG ty;
	SLONG tpage;
	SLONG trot;
	SLONG tflip;
	SLONG tsize;

	SLONG num;

	#if TEXTURE_ENABLE_DC_PACKING

	float base_u;
	float base_v;

	float base_size;

	#else

	static const float base_u = 0.0F;
	static const float base_v = 0.0F;

	static const float base_size = 1.0F;

	#endif

	num = texture & 0x3ff;

	trot  = (texture >> 0xa) & 0x3;
	tflip = (texture >> 0xc) & 0x3;
	tsize = (texture >> 0xe) & 0x3;

	//
	// The page is easy!
	//

   *page = num;

	#if TEXTURE_ENABLE_DC_PACKING

	if (num < 256)
	{
		//
		// Remap this texture?
		//

	   *page = TEXTURE_DC_pack[num].page;

		if (TEXTURE_DC_pack[num].pos == TEXTURE_DC_PACK_POS_WHOLE_PAGE)
		{
			base_u    = 0.0F;
			base_v    = 0.0F;
			base_size = 1.0F;
		}
		else
		{
			static struct
			{
				float base_u;
				float base_v;

			} pos_base[9] =
			{
				{0.125F + 0.25F * 0, 0.125F + 0.25F * 0},
				{0.125F + 0.25F * 1, 0.125F + 0.25F * 0},
				{0.125F + 0.25F * 2, 0.125F + 0.25F * 0},

				{0.125F + 0.25F * 0, 0.125F + 0.25F * 1},
				{0.125F + 0.25F * 1, 0.125F + 0.25F * 1},
				{0.125F + 0.25F * 2, 0.125F + 0.25F * 1},

				{0.125F + 0.25F * 0, 0.125F + 0.25F * 2},
				{0.125F + 0.25F * 1, 0.125F + 0.25F * 2},
				{0.125F + 0.25F * 2, 0.125F + 0.25F * 2},
			};

			ASSERT(WITHIN(TEXTURE_DC_pack[num].pos, 0, 8));

			base_u    = pos_base[TEXTURE_DC_pack[num].pos].base_u;
			base_v    = pos_base[TEXTURE_DC_pack[num].pos].base_v;
			base_size = 64.0F / 256.0F;
		}
	}
	
	#endif

	if (*page >= TEXTURE_page_num_standard)
	{
		*page = 0;
	}

	//
	// The texture coordinates depend of the rotation.
	//

	switch(trot)
	{
		case 0:
			*u0 = base_u;
			*v0 = base_v;
			*u1 = base_u + base_size;
			*v1 = base_v;
			*u2 = base_u;
			*v2 = base_v + base_size;
			*u3 = base_u + base_size;
			*v3 = base_v + base_size;
			break;

		case 1:
			*u2 = base_u;
			*v2 = base_v;
			*u0 = base_u + base_size;
			*v0 = base_v;
			*u3 = base_u;
			*v3 = base_v + base_size;
			*u1 = base_u + base_size;
			*v1 = base_v + base_size;
			break;

		case 2:
			*u3 = base_u;
			*v3 = base_v;
			*u2 = base_u + base_size;
			*v2 = base_v;
			*u1 = base_u;
			*v1 = base_v + base_size;
			*u0 = base_u + base_size;
			*v0 = base_v + base_size;
			break;

		case 3:
			*u1 = base_u;
			*v1 = base_v;
			*u3 = base_u + base_size;
			*v3 = base_v;
			*u0 = base_u;
			*v0 = base_v + base_size;
			*u2 = base_u + base_size;
			*v2 = base_v + base_size;
			break;	   
	}
}


extern	UWORD	local_next_prim_face3;
extern	UWORD	local_next_prim_face4;

void TEXTURE_fix_texture_styles(void)
{
	SLONG	style,piece;

	SLONG page;

	SLONG av_u;
	SLONG av_v;

	SLONG base_u;
	SLONG base_v;

	for(style=0;style<200;style++)
	{
		for(piece=0;piece<5;piece++)
		{
			base_u = textures_xy[style][piece].Tx*32;
			base_v = textures_xy[style][piece].Ty*32;

			av_u = base_u/TEXTURE_NORM_SIZE;
			av_v = base_v/TEXTURE_NORM_SIZE;
			
			page = av_u + av_v * TEXTURE_NORM_SQUARES + textures_xy[style][piece].Page * TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES;
			dx_textures_xy[style][piece].Page=page;
			dx_textures_xy[style][piece].Flip=textures_xy[style][piece].Flip;


		}
	}
}

void TEXTURE_fix_prim_textures()
{
	SLONG i;
	SLONG j;
	SLONG k;

	PrimFace3 *f3;
	PrimFace4 *f4;
	struct	AnimTmap	*p_a;

	SLONG page;

	SLONG av_u;
	SLONG av_v;
	SLONG u;
	SLONG v;

	SLONG base_u;
	SLONG base_v;

	for (i = 1; i < next_prim_face3; i++)
	{
		f3 = &prim_faces3[i];

		if (!(f3->FaceFlags & FACE_FLAG_FIXED))
		{
			av_u = (f3->UV[0][0] + f3->UV[1][0] + f3->UV[2][0]) / 3;
			av_v = (f3->UV[0][1] + f3->UV[1][1] + f3->UV[2][1]) / 3;

			av_u /= TEXTURE_NORM_SIZE;
			av_v /= TEXTURE_NORM_SIZE;

			base_u = av_u * TEXTURE_NORM_SIZE;
			base_v = av_v * TEXTURE_NORM_SIZE;

			//
			// All coordinates relative to the base now...
			//

			for (j = 0; j < 3; j++)
			{
				u  = f3->UV[j][0];
				v  = f3->UV[j][1];

				u -= base_u;
				v -= base_v;

				SATURATE(u, 0, 32);
				SATURATE(v, 0, 32);

				if (u == 31) {u = 32;}
				if (v == 31) {v = 32;}

				f3->UV[j][0] = u;
				f3->UV[j][1] = v;
			}

			page = av_u + av_v * TEXTURE_NORM_SQUARES + f3->TexturePage * TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES;
			f3->FaceFlags&=~FACE_FLAG_THUG_JACKET;

			switch(page)
			{
				//
				// pages 9, 10 , 11 are people
				//
				case	9*64+21:
				case	18*64+2:
				case	18*64+32:
//					ASSERT(0);
					f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+21;

					break;

				case	9*64+22:
				case	18*64+3:
				case	18*64+33:
					f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+22;
					break;

				case	9*64+24:
				case	18*64+4:
				case	18*64+36:
					f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+24;
					break;


				case	9*64+25:
				case	18*64+5:
				case	18*64+37:
					f3->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+25;
					break;

			}
			page-=FACE_PAGE_OFFSET;

//			DebugText(" saturate prim page tri page %d  \n",page);
			SATURATE(page, 0, 64*14);
//			ASSERT(page<64*8);

			//
			// The 9th and 10th bits of page go in the top two bits of UV[0][0]!
			//

			f3->UV[0][0]   |= (page >> 2) & 0xc0;
			f3->TexturePage = (page >> 0) & 0xff;

			//
			// Mark as fixed.
			//

			f3->FaceFlags |= FACE_FLAG_FIXED;
		}
	}

	for (i = 1; i < next_prim_face4; i++)
	{
		f4 = &prim_faces4[i];

		if(!(f4->FaceFlags&FACE_FLAG_ANIMATE))
		{
			if(!(f4->FaceFlags&FACE_FLAG_FIXED))
			{
				av_u = (f4->UV[0][0] + f4->UV[1][0] + f4->UV[2][0] + f4->UV[3][0]) >> 2;
				av_v = (f4->UV[0][1] + f4->UV[1][1] + f4->UV[2][1] + f4->UV[3][1]) >> 2;

				av_u /= TEXTURE_NORM_SIZE;
				av_v /= TEXTURE_NORM_SIZE;

				base_u = av_u * TEXTURE_NORM_SIZE;
				base_v = av_v * TEXTURE_NORM_SIZE;

				//
				// All coordinates relative to the base now...
				//

				for (j = 0; j < 4; j++)
				{
					u  = f4->UV[j][0];
					v  = f4->UV[j][1];

					u -= base_u;
					v -= base_v;

					SATURATE(u, 0, 32);
					SATURATE(v, 0, 32);

					if (u == 31) {u = 32;}
					if (v == 31) {v = 32;}

					f4->UV[j][0] = u;
					f4->UV[j][1] = v;
				}

				page = av_u + av_v * TEXTURE_NORM_SQUARES + f4->TexturePage * TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES;
			f4->FaceFlags&=~FACE_FLAG_THUG_JACKET;
			switch(page)
			{
				//
				// pages 9, 10 , 11 are people
				//
				case	9*64+21:
				case	18*64+2:
				case	18*64+32:
					f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+21;

					break;

				case	9*64+22:
				case	18*64+3:
				case	18*64+33:
					f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+22;
					break;

				case	9*64+24:
				case	18*64+4:
				case	18*64+36:
					f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+24;
					break;


				case	9*64+25:
				case	18*64+5:
				case	18*64+37:
					f4->FaceFlags|=FACE_FLAG_THUG_JACKET;
					page=9*64+25;
					break;

			}
				page-=FACE_PAGE_OFFSET;

				SATURATE(page, 0,14*64 );

				//
				// The 9th and 10th bits of page go in the top two bits of UV[0][0]!
				//

				f4->UV[0][0]   |= (page >> 2) & 0xc0;
				f4->TexturePage = (page >> 0) & 0xff;

				//
				// Mark as fixed.
				//

				f4->FaceFlags |= FACE_FLAG_FIXED;
			}
			else
			{
//				ASSERT(0);
			}
		}
	}

	for (i = 1; i < MAX_ANIM_TMAPS; i++)
	{
		p_a=&anim_tmaps[i];

		for(k=0;k<MAX_TMAP_FRAMES;k++)
		{
			av_u = (p_a->UV[k][0][0] + p_a->UV[k][1][0] + p_a->UV[k][2][0] + p_a->UV[k][3][0]) >> 2;
			av_v = (p_a->UV[k][0][1] + p_a->UV[k][1][1] + p_a->UV[k][2][1] + p_a->UV[k][3][1]) >> 2;

			av_u /= TEXTURE_NORM_SIZE;
			av_v /= TEXTURE_NORM_SIZE;

			base_u = av_u * TEXTURE_NORM_SIZE;
			base_v = av_v * TEXTURE_NORM_SIZE;

			//
			// All coordinates relative to the base now...
			//

			for (j = 0; j < 4; j++)
			{
				p_a->UV[k][j][0] -= base_u;
				p_a->UV[k][j][1] -= base_v;

				SATURATE(p_a->UV[k][j][0], 0, 32);
				SATURATE(p_a->UV[k][j][1], 0, 32);

				if (p_a->UV[k][j][0] == 31) {p_a->UV[k][j][0] = 32;}
				if (p_a->UV[k][j][1] == 31) {p_a->UV[k][j][1] = 32;}
			}

			page = av_u + av_v * TEXTURE_NORM_SQUARES + p_a->Page[k] * TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES;

			SATURATE(page, 0, 575);

			//
			// The 9th and 10th bits of page go in the top two bits of UV[0][0]!
			//

			p_a->UV[k][0][0]   |= (page >> 2) & 0xc0;
			p_a->Page[k] = (page >> 0) & 0xff;
		}
	}
}



#ifndef TARGET_DC
void TEXTURE_set_colour_key(SLONG page)
{
	DDCOLORKEY ck;

	return;
	

	ASSERT(WITHIN(page, 0, TEXTURE_num_textures - 1));

	if (TEXTURE_fiddled)
	{
		//
		// Ranges don't work :o(
		//

		ck.dwColorSpaceLowValue  = 0x00000000;
		ck.dwColorSpaceHighValue = 0x00000000;
 	}
	else
	{
		ck.dwColorSpaceLowValue  = 0;
		ck.dwColorSpaceHighValue = 0;
	}

	TEXTURE_texture[page].SetColorKey(DDCKEY_SRCBLT,&ck);
}
#endif



SLONG TEXTURE_shadow_lock(void)
{
	HRESULT res;

	res = TEXTURE_texture[TEXTURE_page_shadow].LockUser(
			&TEXTURE_shadow_bitmap,
			&TEXTURE_shadow_pitch);

	if (FAILED(res))
	{
		TEXTURE_shadow_bitmap = NULL;
		TEXTURE_shadow_pitch  = 0;

		return FALSE;
	}
	else
	{
		TEXTURE_shadow_mask_red	   = TEXTURE_texture[TEXTURE_page_shadow].mask_red;
		TEXTURE_shadow_mask_green  = TEXTURE_texture[TEXTURE_page_shadow].mask_green;
		TEXTURE_shadow_mask_blue   = TEXTURE_texture[TEXTURE_page_shadow].mask_blue;
		TEXTURE_shadow_mask_alpha  = TEXTURE_texture[TEXTURE_page_shadow].mask_alpha;
		TEXTURE_shadow_shift_red   = TEXTURE_texture[TEXTURE_page_shadow].shift_red;
		TEXTURE_shadow_shift_green = TEXTURE_texture[TEXTURE_page_shadow].shift_green;
		TEXTURE_shadow_shift_blue  = TEXTURE_texture[TEXTURE_page_shadow].shift_blue;
		TEXTURE_shadow_shift_alpha = TEXTURE_texture[TEXTURE_page_shadow].shift_alpha;

		return TRUE;
	}
}

void TEXTURE_shadow_unlock()
{
	TEXTURE_texture[TEXTURE_page_shadow].UnlockUser();
}

void TEXTURE_shadow_update(void)
{
}



void TEXTURE_set_greyscale(SLONG is_greyscale)
{
	SLONG i;

	for (i = 0; i < TEXTURE_MAX_TEXTURES; i++)
	{
		TEXTURE_texture[i].set_greyscale(is_greyscale);
	}
}




SLONG TEXTURE_86_lock()
{
	HRESULT res;

	res = TEXTURE_texture[86].LockUser(
			&TEXTURE_shadow_bitmap,
			&TEXTURE_shadow_pitch);

	if (FAILED(res))
	{
		TEXTURE_shadow_bitmap = NULL;
		TEXTURE_shadow_pitch  = 0;

		return FALSE;
	}
	else
	{
		TEXTURE_shadow_mask_red	   = TEXTURE_texture[86].mask_red;
		TEXTURE_shadow_mask_green  = TEXTURE_texture[86].mask_green;
		TEXTURE_shadow_mask_blue   = TEXTURE_texture[86].mask_blue;
		TEXTURE_shadow_mask_alpha  = TEXTURE_texture[86].mask_alpha;
		TEXTURE_shadow_shift_red   = TEXTURE_texture[86].shift_red;
		TEXTURE_shadow_shift_green = TEXTURE_texture[86].shift_green;
		TEXTURE_shadow_shift_blue  = TEXTURE_texture[86].shift_blue;
		TEXTURE_shadow_shift_alpha = TEXTURE_texture[86].shift_alpha;

		return TRUE;
	}
}

void TEXTURE_86_unlock()
{
	TEXTURE_texture[86].UnlockUser();
}

void TEXTURE_86_update()
{
}


void TEXTURE_set_tga(SLONG page, CBYTE *fn) {
	CBYTE fn2[_MAX_PATH];
	MFFileHandle file;

	strcpy(fn2,TEXTURE_EXTRA_DIR);
	strcat(fn2,fn);
	file = FileOpen(fn2);
	if (file!=FILE_OPEN_ERROR) {
		TEXTURE_texture[page].ChangeTextureTGA(fn2);
		FileClose(file);
	}
}

SLONG TEXTURE_liney;
SLONG TEXTURE_av_r;
SLONG TEXTURE_av_g;
SLONG TEXTURE_av_b;

#ifndef TARGET_DC
SLONG TEXTURE_looks_like(SLONG page)
{
	SLONG i;
	SLONG j;

	SLONG px;
	SLONG py;

	SLONG dx;
	SLONG dy;

	SLONG px1;
	SLONG py1;

	SLONG px2;
	SLONG py2;

	UWORD *bitmap;
	SLONG  pitch;

	UWORD pixel;

	SLONG r;
	SLONG g;
	SLONG b;

	SLONG r1;
	SLONG g1;
	SLONG b1;

	SLONG r2;
	SLONG g2;
	SLONG b2;

	SLONG av_r;
	SLONG av_g;
	SLONG av_b;

	SLONG diff_r;
	SLONG diff_g;
	SLONG diff_b;

	SLONG dir;

	SLONG diff;

	SLONG pdiff_r;
	SLONG pdiff_g;
	SLONG pdiff_b;

	SLONG diff_along;
	SLONG diff_left;
	SLONG diff_right;

	SLONG ddiff_l;
	SLONG ddiff_r;

	SLONG lines;

	ASSERT(WITHIN(page, 0, 511));

	D3DTexture *dt = &TEXTURE_texture[page];

	TEXTURE_liney = 0;
	TEXTURE_av_r  = 0;
	TEXTURE_av_g  = 0;
	TEXTURE_av_b  = 0;

	//
	// Try to lock the page.
	//

	if (SUCCEEDED(dt->LockUser(&bitmap, &pitch)))
	{
		//
		// Work out the average colour of the texture.
		//

		av_r = 0;
		av_g = 0;
		av_b = 0;

		for (px = 0; px < TEXTURE_texture[page].size; px++)
		for (py = 0; py < TEXTURE_texture[page].size; py++)
		{
			pixel = bitmap[px + py * pitch];

			r = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
			g = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
			b = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;

			av_r += r;
			av_g += g;
			av_b += b;
		}

		av_r /= dt->size * dt->size;
		av_g /= dt->size * dt->size;
		av_b /= dt->size * dt->size;

		TEXTURE_av_r = av_r;
		TEXTURE_av_g = av_g;
		TEXTURE_av_b = av_b;

		//
		// Is the texture green?
		//

		if (av_g > av_r && av_g > av_b)
		{
			SLONG rb;
			
			rb   = av_r + av_b;
			rb >>= 1;
			rb  += rb >> 1;

			if (av_g > rb)
			{
				//
				// This is a green texture- assume it is grass.
				//

				dt->UnlockUser();

				return TEXTURE_LOOK_GRASS;
			}
		}

		//
		// Get a number of the random variation inherent in the texture.
		//

		diff_r = 0;
		diff_g = 0;
		diff_b = 0;

		#define TEXTURE_SAMPLE_DIFF1 128
		#define TEXTURE_SAMPLE_DIFF2 4

		for (i = 0; i < TEXTURE_SAMPLE_DIFF1; i++)
		{
			px1 = rand() & (dt->size - 1);
			py1 = rand() & (dt->size - 1);

			pixel = bitmap[px1 + py1 * pitch];

			r1 = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
			g1 = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
			b1 = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;

			for (j = 0; j < TEXTURE_SAMPLE_DIFF2; j++)
			{
				px2 = px1 + (rand() & 0x7) - 0x3;
				py2 = py1 + (rand() & 0x7) - 0x3;

				px2 &= dt->size - 1;
				py2 &= dt->size - 1;

				pixel = bitmap[px2 + py2 * pitch];

				r2 = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
				g2 = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
				b2 = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;

				//
				// The difference in colour between the two pixels.
				//

				diff_r += abs(r2 - r1);
				diff_g += abs(g2 - g1);
				diff_b += abs(b2 - b1);
			}
		}

		diff_r /= TEXTURE_SAMPLE_DIFF1 * TEXTURE_SAMPLE_DIFF2;
		diff_g /= TEXTURE_SAMPLE_DIFF1 * TEXTURE_SAMPLE_DIFF2;
		diff_b /= TEXTURE_SAMPLE_DIFF1 * TEXTURE_SAMPLE_DIFF2;
		diff    = diff_r + diff_g + diff_b;

		//
		// Find out how many straight lines there are in the texture.
		//

		lines = 0;

		#define TEXTURE_SAMPLE_STRAIGHT1 2048
		#define TEXTURE_SAMPLE_STRAIGHT2 16

		static struct
		{
			SBYTE dx;
			SBYTE dy;

		} offset[4] =
		{
			{-1,0},
			{+1,0},
			{0,-1},
			{0,+1}
		};

		dir = 0;

		for (px = 0; px < dt->size; px++)
		for (py = 0; py < dt->size; py++)
		{
			px1 = px & (dt->size - 1);
			py1 = py & (dt->size - 1);

			pixel = bitmap[px1 + py1 * pitch];

			r1 = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
			g1 = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
			b1 = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;
			
			//
			// Work out the average difference in colour to the left/right/along the line.
			//

			diff_left  = 0;
			diff_right = 0;
			diff_along = 0;

			//
			// What direction do we scan for a straight line?
			//

			dx = offset[dir & 0x3].dx;
			dy = offset[dir & 0x3].dy;

			dir += 1;

			for (j = 0; j < TEXTURE_SAMPLE_STRAIGHT2; j++)
			{
				//
				// The difference in colour to the left...
				//

				px2 = px1 - (dy * 4);
				py2 = py1 + (dx * 4);

				px2 &= dt->size - 1;
				py2 &= dt->size - 1;

				pixel = bitmap[px2 + py2 * pitch];

				r2 = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
				g2 = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
				b2 = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;

				pdiff_r = abs(r2 - r1);
				pdiff_g = abs(g2 - g1);
				pdiff_b = abs(b2 - b1);

				diff_left += pdiff_r + pdiff_g + pdiff_b;

				//
				// The difference in colour to the right...
				//

				px2 = px1 + (dy * 4);
				py2 = py1 - (dx * 4);

				px2 &= dt->size - 1;
				py2 &= dt->size - 1;

				pixel = bitmap[px2 + py2 * pitch];

				r2 = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
				g2 = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
				b2 = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;

				pdiff_r = abs(r2 - r1);
				pdiff_g = abs(g2 - g1);
				pdiff_b = abs(b2 - b1);

				diff_right += pdiff_r + pdiff_g + pdiff_b;

				//
				// The difference in colour along the line.
				//

				px2 = px1 + dx;
				py2 = py1 + dy;

				px2 &= dt->size - 1;
				py2 &= dt->size - 1;

				pixel = bitmap[px2 + py2 * pitch];

				r2 = ((pixel >> dt->shift_red  ) & (0xff >> dt->mask_red  )) << dt->mask_red  ;
				g2 = ((pixel >> dt->shift_green) & (0xff >> dt->mask_green)) << dt->mask_green;
				b2 = ((pixel >> dt->shift_blue ) & (0xff >> dt->mask_blue )) << dt->mask_blue ;

				pdiff_r = abs(r2 - r1);
				pdiff_g = abs(g2 - g1);
				pdiff_b = abs(b2 - b1);

				diff_along += pdiff_r + pdiff_g + pdiff_b;

				//
				// The next pixel along the line...
				//

				px1 = px2;
				py1 = py2;

				r1 = r2;
				g1 = g2;
				b1 = b2;
			}

			diff_along /= TEXTURE_SAMPLE_STRAIGHT2;
			diff_left  /= TEXTURE_SAMPLE_STRAIGHT2;
			diff_right /= TEXTURE_SAMPLE_STRAIGHT2;

			//
			// A line to the left.
			//

			if (diff_left > 0x60 && diff_along < 0x10)
			{
				lines += 1;
			}

			//
			// A line to the right.
			//

			if (diff_right > 0x60 && diff_along < 0x10)
			{
				lines += 1;
			}
		}

		TEXTURE_liney = lines;

		dt->UnlockUser();

		//
		// Is the texture brown?
		//

		if (av_r > av_g && av_g > av_b)
		{
			//
			// Colours are in the right order... Make sure they aren't too similar.
			//

			if (av_r > (av_b + (av_b >> 1)))
			{
				//
				// This is a brown texture. How liney is it?
				//

				if (lines > 30)
				{
					//
					// It has some lines...
					//

					return TEXTURE_LOOK_ROAD;
				}
				else
				{
					return TEXTURE_LOOK_DIRT;
				}
			}
		}

		//
		// Is the texture white?
		//

		if (av_r > 215 && av_g > 215 && av_b > 215)
		{
			//
			// White.
			//

			if (lines > 30)
			{
				//
				// Too liney.
				//

				return TEXTURE_LOOK_ROAD;
			}
			else
			{
				//
				// Must be ice.
				//

				return TEXTURE_LOOK_SLIPPERY;
			}
		}

		//
		// Assume it is road!
		//

		return TEXTURE_LOOK_ROAD;
	}

	return TEXTURE_LOOK_ROAD;
}
#endif //#ifndef TARGET_DC

#define	PEOPLE3_CROTCH		0
#define	PEOPLE3_FRONT		3
#define	PEOPLE3_HAT_SIDE	6
#define	PEOPLE3_HAT_FRONT	9
#define	PEOPLE3_LEG			12

#define	PEOPLE3_F_ARSE		15
#define	PEOPLE3_F_SEAM		21
#define	PEOPLE3_F_BACK		27
#define	PEOPLE3_F_CHEST		18
#define	PEOPLE3_F_SHOE		24

UWORD	alt_texture[]=
{
/*
	0,0,0,0,0,0,0,0,  //0
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,   //64
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,   //128
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
*/

	0,0,0,0,0,0,0,0,	 //0
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,	 //64
	0,0,0,0,0,0,0,0,	 //72
	0,0,0,0,0,0,0,0,	 //80
	0,0,0,0,0,0,0,0,	 //88
	0,0,0,0,0,0,0,0,	 //96
	0,0,0,0,PEOPLE3_ALT+PEOPLE3_LEG,PEOPLE3_ALT+PEOPLE3_CROTCH,0,PEOPLE3_ALT+PEOPLE3_FRONT,	 //104
	PEOPLE3_ALT+PEOPLE3_HAT_SIDE,PEOPLE3_ALT+PEOPLE3_HAT_FRONT,PEOPLE3_ALT+PEOPLE3_F_ARSE,PEOPLE3_ALT+PEOPLE3_F_SEAM,PEOPLE3_ALT+PEOPLE3_F_SHOE,PEOPLE3_ALT+PEOPLE3_F_CHEST,PEOPLE3_ALT+PEOPLE3_F_BACK,0,	 //112
	0,0,0,0,0,0,0,0,	 //120

	0,0,0,0,0,0,0,0,	//128
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



};

