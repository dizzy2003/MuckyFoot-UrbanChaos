//
// Texture handling.
//

#define __MSC__

#include "game.h"
#include <glide.h>
#include "gltexture.h"


//
// Hmm...
//

extern UWORD floor_texture_sizes[];


#define TEXTURE_MAX_TEXTURES 1024
#define TEXTURE_NUM_STANDARD 10

#define TEXTURE_NORM_SIZE	32
#define TEXTURE_FIDD_SIZE	36

#define TEXTURE_NORM_SQUARES	8
#define TEXTURE_FIDD_SQUARES	7


//
// The texture pages.
// 

#define TEXTURE_TYPE_NONE					0
#define TEXTURE_TYPE_TRILINEAR				1
#define TEXTURE_TYPE_ADDITIVE_NOZWRITE		2
#define TEXTURE_TYPE_ALPHABLEND				3
#define TEXTURE_TYPE_ALPHABLEND_NOZWRITE	4


typedef struct
{
	ULONG type;

	GrTexInfo textinfo;

	ULONG address_tmu0;
	ULONG address_tmu1;

} TEXTURE_Texture;

TEXTURE_Texture TEXTURE_texture[TEXTURE_MAX_TEXTURES];

SLONG TEXTURE_page_num_standard;

SLONG TEXTURE_page_fog;
SLONG TEXTURE_page_moon;
SLONG TEXTURE_page_clouds;
SLONG TEXTURE_page_water;
SLONG TEXTURE_page_puddle;
SLONG TEXTURE_page_shadow;
SLONG TEXTURE_page_detail;

//
// The number of textures loaded.
//

SLONG TEXTURE_num_textures;



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

	static const float base_u = 0.0F;
	static const float base_v = 0.0F;

	static const float base_size = 256.0F;

	num = texture & 0x3ff;

	trot  = (texture >> 0xa) & 0x3;
	tflip = (texture >> 0xc) & 0x3;
	tsize = (texture >> 0xe) & 0x3;

	//
	// The page is easy!
	//

   *page = num;

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

SLONG TEXTURE_get_fiddled_position(
		SLONG  square_u,
		SLONG  square_v,
		SLONG  page,
		float *u,
		float *v)
{
	SLONG num;

	{
		num = square_u + square_v * TEXTURE_NORM_SQUARES + page * (TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES);

		square_u = num % 7; num /= 7;
		square_v = num % 7; num /= 7;
		page     = num;
			

		*u = float(square_u * TEXTURE_FIDD_SIZE + (TEXTURE_FIDD_SIZE - TEXTURE_NORM_SIZE >> 1));
		*v = float(square_v * TEXTURE_FIDD_SIZE + (TEXTURE_FIDD_SIZE - TEXTURE_NORM_SIZE >> 1));
	}

	return page;
}


void TEXTURE_load()
{
	SLONG i;

	Gu3dfInfo fileinfo;

	ULONG texture_mem_required;

	ULONG address_tmu0 = grTexMinAddress(GR_TMU0);
	ULONG address_tmu1 = grTexMinAddress(GR_TMU1);

	CBYTE name_3df[32];

	TEXTURE_Texture *tt;

	//
	// Load the individual pages.
	//

	#define TEXTURE_NUM_FIDDLED (8 * 8 * 9)

	for (i = 0; i < TEXTURE_NUM_FIDDLED; i++)
	{
		tt = &TEXTURE_texture[i];

		tt->type         = TEXTURE_TYPE_TRILINEAR;
		tt->address_tmu0 = 0;
		tt->address_tmu1 = 0;

		sprintf(name_3df, "3dfx\\splitup\\3df\\tex%03d.3df", i);

		if (gu3dfGetInfo(name_3df, &fileinfo))
		{
			fileinfo.data = malloc(fileinfo.mem_required);

			if (fileinfo.data)
			{
				if (gu3dfLoad(name_3df, &fileinfo))
				{
					//
					// The file has been loaded into memory.
					//

					tt->textinfo.smallLodLog2    = fileinfo.header.small_lod;
					tt->textinfo.largeLodLog2    = fileinfo.header.large_lod;
					tt->textinfo.aspectRatioLog2 = fileinfo.header.aspect_ratio;
					tt->textinfo.format          = fileinfo.header.format;
					tt->textinfo.data            = fileinfo.data;

					//
					// Download into TMU0.
					//

					texture_mem_required = grTexTextureMemRequired(
												 GR_MIPMAPLEVELMASK_ODD,
												&tt->textinfo);

					if (address_tmu0 + texture_mem_required > grTexMaxAddress(GR_TMU0))
					{
						TRACE("Not enough texture memory in TMU0 to load %s\n", name_3df);
					}
					else
					{
						grTexDownloadMipMap(
							GR_TMU0,
							address_tmu0,
							GR_MIPMAPLEVELMASK_ODD,
						   &tt->textinfo);

						tt->address_tmu0 = address_tmu0;
						address_tmu0    += texture_mem_required;
					}

					//
					// Download into TMU1.
					//

					texture_mem_required = grTexTextureMemRequired(
												 GR_MIPMAPLEVELMASK_EVEN,
												&tt->textinfo);

					if (address_tmu1 + texture_mem_required > grTexMaxAddress(GR_TMU1))
					{
						TRACE("Not enough texture memory in TMU1 to load %s\n", name_3df);
					}
					else
					{
						grTexDownloadMipMap(
							GR_TMU1,
							address_tmu1,
							GR_MIPMAPLEVELMASK_EVEN,
						   &tt->textinfo);

						tt->address_tmu1 = address_tmu1;
						address_tmu1    += texture_mem_required;
					}

				}
		
				free(fileinfo.data);
			}
		}
		else
		{
			TRACE("Could not load %s\n", name_3df);
		}
	}

	TEXTURE_page_num_standard = TEXTURE_NUM_FIDDLED + 0;

	//
	// The extra texture pages are all put in TMU1
	//

	#define TEXTURE_NUM_EXTRA 6

	TEXTURE_page_fog    = TEXTURE_NUM_FIDDLED + 0;
	TEXTURE_page_moon   = TEXTURE_NUM_FIDDLED + 1;
	TEXTURE_page_clouds = TEXTURE_NUM_FIDDLED + 2;
	TEXTURE_page_water  = TEXTURE_NUM_FIDDLED + 3;
	TEXTURE_page_puddle = TEXTURE_NUM_FIDDLED + 4;
	TEXTURE_page_detail = TEXTURE_NUM_FIDDLED + 5;

	CBYTE *extra_name[TEXTURE_NUM_EXTRA] =
	{
		"3dfx\\fog.3df",
		"3dfx\\moon.3df",
		"3dfx\\clouds.3df",
		"3dfx\\water.3df",
		"3dfx\\puddle.3df",
		"3dfx\\detail.3df"
	};

	ULONG  extra_type[TEXTURE_NUM_EXTRA] =
	{
		TEXTURE_TYPE_ADDITIVE_NOZWRITE,
		TEXTURE_TYPE_ALPHABLEND_NOZWRITE,
		TEXTURE_TYPE_ADDITIVE_NOZWRITE,
		TEXTURE_TYPE_ALPHABLEND_NOZWRITE,
		TEXTURE_TYPE_ALPHABLEND,
		TEXTURE_TYPE_NONE
	};

	for (i = 0; i < TEXTURE_NUM_EXTRA; i++)
	{
		tt = &TEXTURE_texture[TEXTURE_NUM_FIDDLED + i];

		tt->type         = extra_type[i];
		tt->address_tmu0 = 0;
		tt->address_tmu1 = 0;

		if (gu3dfGetInfo(extra_name[i], &fileinfo))
		{
			fileinfo.data = malloc(fileinfo.mem_required);

			if (fileinfo.data)
			{
				if (gu3dfLoad(extra_name[i], &fileinfo))
				{
					//
					// The file has been loaded into memory.
					//

					tt->textinfo.smallLodLog2    = fileinfo.header.small_lod;
					tt->textinfo.largeLodLog2    = fileinfo.header.large_lod;
					tt->textinfo.aspectRatioLog2 = fileinfo.header.aspect_ratio;
					tt->textinfo.format          = fileinfo.header.format;
					tt->textinfo.data            = fileinfo.data;

					//
					// Download into TMU1.
					//

					texture_mem_required = grTexTextureMemRequired(
												 GR_MIPMAPLEVELMASK_BOTH,
												&tt->textinfo);

					if (address_tmu1 + texture_mem_required > grTexMaxAddress(GR_TMU1))
					{
						TRACE("Not enough texture memory in TMU1 to load %s\n", name_3df);
					}
					else
					{
						grTexDownloadMipMap(
							GR_TMU1,
							address_tmu1,
							GR_MIPMAPLEVELMASK_BOTH,
						   &tt->textinfo);

						tt->address_tmu1 = address_tmu1;
						address_tmu1    += texture_mem_required;
					}
				}
		
				free(fileinfo.data);
			}
		}
		else
		{
			TRACE("Could not load %s\n", extra_name[i]);
		}
	}

	TEXTURE_num_textures = TEXTURE_NUM_FIDDLED + TEXTURE_NUM_EXTRA;

	//
	// Set NCC table 0 as the default.
	//

	grTexNCCTable(GR_TEXTABLE_NCC0);
}


//
// The states we can be in.
//

#define TEXTURE_STATE_TRILINEAR			0
#define TEXTURE_STATE_DETAIL			1
#define TEXTURE_STATE_ADDITIVE_NOZWRITE	2

SLONG TEXTURE_current_page_tmu0 = -1;
SLONG TEXTURE_current_page_tmu1 = -1;
SLONG TEXTURE_current_state     = -1;

void TEXTURE_set_page(SLONG page, SLONG multi)
{
	TEXTURE_Texture *tt;

	ASSERT(WITHIN(page, 0, TEXTURE_num_textures - 1));

	tt = &TEXTURE_texture[page];

	//
	// What state should we be in and which pages should we be using.
	//

	SLONG state;
	SLONG page_tmu0;
	SLONG page_tmu1;
	ULONG evenodd_tmu0;
	ULONG evenodd_tmu1;

	switch(tt->type)
	{
		case TEXTURE_TYPE_TRILINEAR:

			if (multi == TEXTURE_MULTI_DETAIL)
			{
				state        = TEXTURE_STATE_DETAIL;
				page_tmu0    = page;
				page_tmu1    = TEXTURE_page_detail;
				evenodd_tmu0 = GR_MIPMAPLEVELMASK_ODD;
				evenodd_tmu1 = GR_MIPMAPLEVELMASK_BOTH;
			}
			else
			{
				state        = TEXTURE_STATE_TRILINEAR;
				page_tmu0    = page;
				page_tmu1    = page;
				evenodd_tmu0 = GR_MIPMAPLEVELMASK_ODD;
				evenodd_tmu1 = GR_MIPMAPLEVELMASK_EVEN;
			}

			break;

		case TEXTURE_TYPE_ADDITIVE_NOZWRITE:

			state        = TEXTURE_STATE_ADDITIVE_NOZWRITE;
			page_tmu0    = TEXTURE_current_page_tmu0;
			page_tmu1    = TEXTURE_page_fog;
			evenodd_tmu0 = GR_MIPMAPLEVELMASK_BOTH;
			evenodd_tmu1 = GR_MIPMAPLEVELMASK_BOTH;

			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Make sure the texture pages are correct.
	//

	if (page_tmu0 != TEXTURE_current_page_tmu0)
	{
		ASSERT(WITHIN(page_tmu0, 0, TEXTURE_num_textures - 1));

		TEXTURE_Texture *tt_tmu0 = &TEXTURE_texture[page_tmu0];

		grTexSource(
			GR_TMU0,
			tt_tmu0->address_tmu0,
			evenodd_tmu0,
		   &tt_tmu0->textinfo);

		TEXTURE_current_page_tmu0 = page_tmu0;
	}

	if (page_tmu1 != TEXTURE_current_page_tmu1)
	{
		ASSERT(WITHIN(page_tmu1, 0, TEXTURE_num_textures - 1));

		TEXTURE_Texture *tt_tmu1 = &TEXTURE_texture[page_tmu1];

		grTexSource(
			GR_TMU1,
			tt_tmu1->address_tmu1,
			evenodd_tmu1,
		   &tt_tmu1->textinfo);

		TEXTURE_current_page_tmu1 = page_tmu1;
	}

	//
	// Make sure the state is correct.
	//

	if (state != TEXTURE_current_state)
	{
		switch(state)
		{
			case TEXTURE_STATE_TRILINEAR:

				grDepthMask(FXTRUE);

				grTexCombine(
					GR_TMU0,
					GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL, GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION,
					GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL, GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION,
					FXFALSE,
					FXFALSE);

				grAlphaBlendFunction(
					GR_BLEND_ONE,
					GR_BLEND_ZERO,
					GR_BLEND_ONE,
					GR_BLEND_ZERO);

				grTexMipMapMode(
					GR_TMU0,
					GR_MIPMAP_NEAREST,
					FXTRUE);

				grTexMipMapMode(
					GR_TMU1,
					GR_MIPMAP_NEAREST,
					FXTRUE);

				break;

			case TEXTURE_STATE_DETAIL:

				grDepthMask(FXTRUE);

				grTexCombine(
					GR_TMU0,
					GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,
					GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR,
					FXFALSE,
					FXFALSE);

				grAlphaBlendFunction(
					GR_BLEND_ONE,
					GR_BLEND_ZERO,
					GR_BLEND_ONE,
					GR_BLEND_ZERO);

				grTexMipMapMode(
					GR_TMU0,
					GR_MIPMAP_DISABLE,
					FXTRUE);

				grTexMipMapMode(
					GR_TMU1,
					GR_MIPMAP_DISABLE,
					FXTRUE);

				//
				// Detail-texturing constants.
				//

				grTexDetailControl(
					GR_TMU0,
					1,
					6,
					1.0F);

				break;

			case TEXTURE_STATE_ADDITIVE_NOZWRITE:

				grDepthMask(FXFALSE);

				grTexCombine(
					GR_TMU0,
					GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
					GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
					FXFALSE,
					FXFALSE);

				grAlphaBlendFunction(
					GR_BLEND_ONE,
					GR_BLEND_ONE,
					GR_BLEND_ZERO,
					GR_BLEND_ZERO);

				grTexMipMapMode(
					GR_TMU0,
					GR_MIPMAP_DISABLE,
					FXTRUE);

				grTexMipMapMode(
					GR_TMU1,
					GR_MIPMAP_DISABLE,
					FXTRUE);

				break;

			default:
				ASSERT(0);
				break;
		}

		TEXTURE_current_state = state;
	}
}

void TEXTURE_init_states()
{
	TEXTURE_current_page_tmu0 = -1;
	TEXTURE_current_page_tmu1 = -1;
	TEXTURE_current_state     = -1;	
}


void TEXTURE_fix_prim_textures()
{
	SLONG i;
	SLONG j;

	PrimFace3 *f3;
	PrimFace4 *f4;

	SLONG page;

	SLONG av_u;
	SLONG av_v;

	SLONG base_u;
	SLONG base_v;

	for (i = 1; i < next_prim_face3; i++)
	{
		f3 = &prim_faces3[i];

		av_u = (f3->UV[0][0] + f3->UV[1][0] + f3->UV[2][0]) * 85 >> 8;
		av_v = (f3->UV[0][1] + f3->UV[1][1] + f3->UV[2][1]) * 85 >> 8;

		av_u /= TEXTURE_NORM_SIZE;
		av_v /= TEXTURE_NORM_SIZE;

		base_u = av_u * TEXTURE_NORM_SIZE;
		base_v = av_v * TEXTURE_NORM_SIZE;

		//
		// All coordinates relative to the base now...
		//

		for (j = 0; j < 3; j++)
		{
			f3->UV[j][0] -= base_u;
			f3->UV[j][1] -= base_v;

			SATURATE(f3->UV[j][0], 0, 32);
			SATURATE(f3->UV[j][1], 0, 32);

			if (f3->UV[j][0] == 31) {f3->UV[j][0] = 32;}
			if (f3->UV[j][1] == 31) {f3->UV[j][1] = 32;}
		}

		page = av_u + av_v * TEXTURE_NORM_SQUARES + f3->TexturePage * TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES;

		SATURATE(page, 0, 575);

		//
		// The 9th and 10th bits of page go in the top two bits of UV[0][0]!
		//

		f3->UV[0][0]   |= (page >> 2) & 0xc0;
		f3->TexturePage = (page >> 0) & 0xff;
	}

	for (i = 1; i < next_prim_face4; i++)
	{
		f4 = &prim_faces4[i];

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
			f4->UV[j][0] -= base_u;
			f4->UV[j][1] -= base_v;

			SATURATE(f4->UV[j][0], 0, 32);
			SATURATE(f4->UV[j][1], 0, 32);

			if (f4->UV[j][0] == 31) {f4->UV[j][0] = 32;}
			if (f4->UV[j][1] == 31) {f4->UV[j][1] = 32;}
		}

		page = av_u + av_v * TEXTURE_NORM_SQUARES + f4->TexturePage * TEXTURE_NORM_SQUARES * TEXTURE_NORM_SQUARES;

		SATURATE(page, 0, 575);

		//
		// The 9th and 10th bits of page go in the top two bits of UV[0][0]!
		//

		f4->UV[0][0]   |= (page >> 2) & 0xc0;
		f4->TexturePage = (page >> 0) & 0xff;
	}
}
