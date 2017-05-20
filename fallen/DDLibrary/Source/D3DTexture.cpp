// D3DTexture.cpp
// Guy Simmons, 29th November 1997.

#include	"DDLib.h"
#include	"tga.h"

#ifdef TARGET_DC
#include	"target.h"
#endif

#ifndef VERIFY
#ifdef NDEBUG
#define VERIFY(x) x
#else
#define VERIFY(x) {ASSERT(x);}
#endif
#endif

#ifdef TEX_EMBED
static D3DTexture*				EmbedSource = NULL;
static LPDIRECTDRAWSURFACE4		EmbedSurface = NULL;
static LPDIRECT3DTEXTURE2		EmbedTexture = NULL;
static UBYTE					EmbedOffset = 0;
#endif


extern void POLY_reset_render_states ( void );


// Do VQ compression if the size is over this value.
//
// Note: 32x32x2 = 2048, but 32x32/4+2048 = 2304
// But:  64x64x2 = 8192, but 64x64/4+2048 = 3072,
//
// So under 32x32, the VQ size is actually greater.
// However, if mipmapped, they would normally take:
// 32x32x2*(4/3) =  2730, but 32x32*(4/3)/4+2048 = 2389
// 64x64x2*(4/3) = 10922, but 64x64*(4/3)/4+2048 = 3413,
//
// So when mipmapping, VQ is a win even at 32x32. At 16x16 however:
// 16x16x2*(4/3) = 682, but 16x16*(4/3)/4+2048 = 2389
// ... as expected.

#define VQ_MINIMUM_SIZE 32

// Now that mipmapped VQs are on disk, they load even faster than normal stuff.
// So there's no reason not to.
//#ifdef DEBUG
//#define DONT_VQ_MY_TEXTURES_PLEASE_BOB
//#else
//#define DONT_VQ_MY_TEXTURES_PLEASE_BOB
//#endif


#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
// The structure that holds the mapping from page to texture.
#define MAX_TEXTURES_PER_PAGE 16
struct PageToTextureMap
{
	int iType;		// One of the D3DPAGE_xxx types.
	D3DTexture *d3dTexture[MAX_TEXTURES_PER_PAGE];
	LPDIRECT3DTEXTURE2		lp_Texture;
	LPDIRECTDRAWSURFACE4	lp_Surface;
};

// The structure that defines where each texture goes in which page.
struct TextureToPageMap
{
	char *name;			// Texture file name.
	int iPageNum;		// The page this is in.
	int iPagePos;		// Position in page.
};

// And now the list of textures in pages.
// Pages are separated by strings starting with "***" and then a page type, which is one of:
// "64_3", "64_4", "32_3", "32_4", etc, then a colon followed by the directory of the page.
// Each page can only be composed of textures in a single directory. This is not a big
// restriction. Then comes the actual filename of the page image. If this file doesn't exist
// then (theoretically), one is created automagically. You must _have_ a filename, because
// it gets saved out.
// Then the list of texture names, without leading directory. There doesn't have to
// be the full number of textures in a page - the list can end early. Can't have more of course.
// Textures are listed going across the texture first for 4x4s, then down a line, e.g.:
//
// 1234
// 5678
// 9abc
// def0
//
// For historical reasons, 3x3 chunks are listed the other way round:
//
// 147
// 258
// 369
//
// Confusing eh? :-)
//
// If a filename is NULL or "", then that position is skipped.
// The end comes at "***END"


char *TexturePageList[] = {

#if 0
	"***64_4:server\\textures\\shared\\people3\\", "server\\textures\\shared\\people3\\person_purple.tga",
	"femchest1.tga",
	"",
	"",
	"femarse1.tga",
	"seam1.tga",
	"",
	"front1.tga",
	"fembak1.tga",
	"femshoo1.tga",
	"",
	"crotch1.tga",
	"hattop1.tga",
	"leg1.tga",
	"",
	"",
	"hatside1.tga",

	"***64_4:server\\textures\\shared\\people3\\", "server\\textures\\shared\\people3\\person_red.tga",
	"femchest2.tga",
	"",
	"",
	"femarse2.tga",
	"seam2.tga",
	"",
	"front5.tga",
	"fembak2.tga",
	"femshoo2.tga",
	"",
	"crotch5.tga",
	"hattop5.tga",
	"leg5.tga",
	"",
	"",
	"hatside5.tga",



	// NOTE! Not the correct page texture!
	"***64_4:server\\textures\\shared\\people3\\", "server\\textures\\shared\\people3\\person_page3.tga",
	"femchest3.tga",
	"",
	"",
	"femarse3.tga",
	"seam3.tga",
	"",
	"front4.tga",
	"fembak3.tga",
	"femshoo3.tga",
	"",
	"crotch4.tga",
	"hattop4.tga",
	"leg4.tga",
	"",
	"",
	"hatside4.tga",

	// NOTE! Not the correct page texture!
	"***64_4:server\\textures\\shared\\people3\\", "server\\textures\\shared\\people3\\person_page4.tga",
	"",
	"",
	"",
	"",
	"",
	"",
	"front2.tga",
	"",
	"",
	"",
	"crotch2.tga",
	"hattop2.tga",
	"leg2.tga",
	"",
	"",
	"hatside2.tga",

	// NOTE! Not the correct page texture!
	"***64_4:server\\textures\\shared\\people3\\", "server\\textures\\shared\\people3\\person_page5.tga",
	"",
	"",
	"",
	"",
	"",
	"",
	"front3.tga",
	"",
	"",
	"",
	"crotch3.tga",
	"hattop3.tga",
	"leg3.tga",
	"",
	"",
	"hatside3.tga",
#endif

	"***64_4:server\\textures\\shared\\people\\", "server\\textures\\shared\\people\\page_darci1.tga",
	"tex064.tga",
	"tex077.tga",
	"tex081.tga",
	"tex080.tga",
	"tex065.tga",
	"tex069.tga",
	"tex082.tga",
	"tex078.tga",
	"tex067.tga",
	"tex068.tga",
	"tex070.tga",
	"tex066.tga",
	"tex062.tga",
	"tex061.tga",
	"tex079.tga",
	"tex063.tga",

	"***64_4:server\\textures\\extras\\", "server\\textures\\extras\\page_misc_alpha.tga",
	"bludsplt.tga",
	"bigleaf.tga",
	"tinybutt.tga",
	"footprint.tga",
	"leaf.tga",
	"rubbish.tga",
	"",
	"",
	"",
	"",
	"",
	"",
	"raindrop.tga",

#if 1
	"***64_4:server\\textures\\extras\\DC\\", "server\\textures\\extras\\DC\\page_joybutts.tga",
	"button_A.tga",
	"button_B.tga",
	"button_C.tga",
	"button_X.tga",
	"button_Y.tga",
	"button_Z.tga",
	"button_LEFT.tga",
	"button_RIGHT.tga",
	"button_PADLEFT.tga",
	"button_PADRIGHT.tga",
	"button_PADDOWN.tga",
	"button_PADUP.tga",
#endif

	"***END"
};

//
// The texture page lists for each mission...
//

#ifdef TARGET_DC

CBYTE *fname_for_packed_array[] =
{
	"PackedTextures\\testdrive1a\\array_testdrive1a.dat",
	"PackedTextures\\FTutor1\\array_FTutor1.dat",
	"PackedTextures\\Assault1\\array_Assault1.dat",
	"PackedTextures\\police1\\array_police1.dat",
	"PackedTextures\\police2\\array_police2.dat",
	"PackedTextures\\Bankbomb1\\array_Bankbomb1.dat",
	"PackedTextures\\police3\\array_police3.dat",
	"PackedTextures\\Gangorder2\\array_Gangorder2.dat",
	"PackedTextures\\police4\\array_police4.dat",
	"PackedTextures\\bball2\\array_bball2.dat",
	"PackedTextures\\wstores1\\array_wstores1.dat",
	"PackedTextures\\e3\\array_e3.dat",
	"PackedTextures\\westcrime1\\array_westcrime1.dat",
	"PackedTextures\\carbomb1\\array_carbomb1.dat",
	"PackedTextures\\botanicc\\array_botanicc.dat",
	"PackedTextures\\Semtex\\array_Semtex.dat",
	"PackedTextures\\AWOL1\\array_AWOL1.dat",
	"PackedTextures\\mission2\\array_mission2.dat",
	"PackedTextures\\park2\\array_park2.dat",
	"PackedTextures\\MIB\\array_MIB.dat",
	"PackedTextures\\skymiss2\\array_skymiss2.dat",
	"PackedTextures\\factory1\\array_factory1.dat",
	"PackedTextures\\estate2\\array_estate2.dat",
	"PackedTextures\\Stealtst1\\array_Stealtst1.dat",
	"PackedTextures\\snow2\\array_snow2.dat",
	"PackedTextures\\Gordout1\\array_Gordout1.dat",
	"PackedTextures\\Baalrog3\\array_Baalrog3.dat",
	"PackedTextures\\Finale1\\array_Finale1.dat",
	"PackedTextures\\Gangorder1\\array_Gangorder1.dat",
	"PackedTextures\\FreeCD1\\array_FreeCD1.dat",
	"PackedTextures\\jung3\\array_jung3.dat",
	"PackedTextures\\fight1\\array_fight1.dat",
	"PackedTextures\\fight2\\array_fight2.dat",
	"PackedTextures\\testdrive2\\array_testdrive2.dat",
	"PackedTextures\\testdrive3\\array_testdrive3.dat",
	"PackedTextures\\Album1\\array_Album1.dat",
};


#endif



#define MAX_NUM_D3DPAGES 200

int iNumD3DPages = 0;
D3DPage pageD3D[MAX_NUM_D3DPAGES];


#endif





static DWORD dwSizeOfFastLoadBuffer = 0;
void *pvFastLoadBuffer = NULL;

inline void *GetMeAFastLoadBufferAtLeastThisBigPlease ( DWORD dwSize )
{
	if ( dwSizeOfFastLoadBuffer < dwSize )
	{
		if ( pvFastLoadBuffer != NULL )
		{
			VirtualFree ( pvFastLoadBuffer, NULL, MEM_RELEASE );
		}
		// Grow slightly more than needed to prevent hammering.
		dwSizeOfFastLoadBuffer = ( dwSize * 5 / 4 + 1024 );
		// Ensure it's 4k-aligned.
		dwSizeOfFastLoadBuffer = ( ( dwSizeOfFastLoadBuffer + 4095 ) & ~4095 );

		TRACE ( "Growing FastLoad buffer to 0x%x bytes\n", dwSizeOfFastLoadBuffer );

		pvFastLoadBuffer = VirtualAlloc ( NULL, dwSizeOfFastLoadBuffer, MEM_COMMIT, PAGE_READWRITE );
		ASSERT ( pvFastLoadBuffer != NULL );
	}
	return ( pvFastLoadBuffer );
}


void NotGoingToLoadTexturesForAWhileNowSoYouCanCleanUpABit ( void )
{
	if ( pvFastLoadBuffer != NULL )
	{
		TRACE ( "Freeing FastLoad buffer\n" );
		VirtualFree ( pvFastLoadBuffer, NULL, MEM_RELEASE );
		pvFastLoadBuffer = NULL;
		dwSizeOfFastLoadBuffer = 0;
	}
}

inline void *FastLoadFileSomewhere ( MFFileHandle handle, DWORD dwSize )
{
	void *pvData = GetMeAFastLoadBufferAtLeastThisBigPlease ( dwSize );
	ASSERT ( pvData != NULL );

	DWORD dwAlignedFileSize = dwSize & ( ~4095 );
	// DMA read
	DWORD dwRead = 0;
	if ( dwAlignedFileSize > 0 )
	{
		dwRead = FileRead ( handle, pvData, dwAlignedFileSize );
	}
	// Finish off with PIO or whatever.
	if ( dwSize - dwAlignedFileSize > 0 )
	{
		dwRead += FileRead ( handle, (void *)( (char *)pvData + dwAlignedFileSize ), dwSize - dwAlignedFileSize  );
	}

	ASSERT ( dwRead == dwSize );

	return ( pvData );
}





//
// Loads a 'cpp' file of disk that's been converted to
// a 'dat' file with the Martin utility!
//

CBYTE **D3DTEXTURE_cpp_array;
CBYTE  *D3DTEXTURE_cpp_buffer;

void D3DTEXTURE_load_cpp_array(CBYTE *fname)
{
	CBYTE **ans;
	SLONG   ans_max;
	SLONG   ans_upto;
	
	CBYTE  *buffer;
	SLONG   buffer_max;
	SLONG   buffer_upto;
	
	//
	// Initialise answer.
	//

	D3DTEXTURE_cpp_array  = NULL;
	D3DTEXTURE_cpp_buffer = NULL;
	
	//
	// Open file.
	//

	FILE *handle = MF_Fopen(fname, "rb");

	if (handle == NULL)
	{
		ASSERT(0);

		return;
	}

	//
	// Read in our memory requirements.
	//

	if (fread(&ans_max,    sizeof(SLONG), 1, handle) != 1) goto file_error;
	if (fread(&buffer_max, sizeof(SLONG), 1, handle) != 1) goto file_error;

	//
	// Allocate memory.
	//

	ans_upto = 0;
	ans      = (CBYTE **) MemAlloc(sizeof(CBYTE *) * ans_max);

	buffer_upto = 0;
	buffer      = (CBYTE *) MemAlloc(sizeof(CBYTE) * buffer_max);

	ASSERT(ans   );
	ASSERT(buffer);

	memset(ans,    0, sizeof(CBYTE *) * ans_max   );
	memset(buffer, 0, sizeof(CBYTE  ) * buffer_max);

	//
	// Load in the indices and the buffer.
	//

	if (fread(ans,    sizeof(SLONG), ans_max,    handle) != ans_max   ) goto file_error;
	if (fread(buffer, sizeof(CBYTE), buffer_max, handle) != buffer_max) goto file_error;

	//
	// Convert indices to pointers...
	// 

	SLONG i;

	for (i = 0; i < ans_max; i++)
	{
		ans[i] = buffer + SLONG(ans[i]);
	}

	//
	// Return the result.
	//

	D3DTEXTURE_cpp_array  = ans;
	D3DTEXTURE_cpp_buffer = buffer;

	MF_Fclose(handle);

	return;

  file_error:

	ASSERT(0);	

	MF_Fclose(handle);

	return;

}











#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
static bool m_bTexturePagesInitialised = FALSE;


void FreeAllD3DPages ( void )
{
	for ( int i = 0; i < iNumD3DPages; i++ )
	{
		pageD3D[i].Unload();
	}
	iNumD3DPages = 0;

	// And free the array memory.
	if ( D3DTEXTURE_cpp_array != NULL )
	{
		MemFree(D3DTEXTURE_cpp_array );
		D3DTEXTURE_cpp_array  = NULL;
	}

	if ( D3DTEXTURE_cpp_buffer != NULL )
	{
		MemFree(D3DTEXTURE_cpp_buffer);
		D3DTEXTURE_cpp_buffer = NULL;
	}

	// And redo all the render states and sorting.
	POLY_reset_render_states();

}


#endif

















void D3DTexture::BeginLoading()
{
	SLONG first_time = TRUE;

#ifdef TEX_EMBED
	EmbedSource = NULL;
	EmbedSurface = NULL;
	EmbedTexture = NULL;
	EmbedOffset = 0;

#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
	//if ( !m_bTexturePagesInitialised )


	// Unload everything first.
	FreeAllD3DPages();

	ASSERT ( iNumD3DPages == 0 );

	while(1)
	{
		// Done once only.
		//m_bTexturePagesInitialised = TRUE;

		// Scan my text list, initialising the D3DPages as we go.
		char **ppcList = TexturePageList;
		int iPageType = 0;
		int iPagePos = 0;
		int iSafetyTimeout = 1000;

		if (first_time)
		{
			//
			// Don't use TexturePageList, use the right one for the current mission.
			//

			int i = 0;
			bool bFound = FALSE;

			while(1)
			{
				extern CBYTE  ELEV_fname_level[];
				extern CBYTE *suggest_order[];

				if (suggest_order[i][0] == '!')
				{
					break;
				}

				if (strcmp(ELEV_fname_level + 7, suggest_order[i]) == 0)
				{
					//
					// Found our mission...
					//

					D3DTEXTURE_load_cpp_array(fname_for_packed_array[i]);

					//ppcList = texture_page_lists_for_each_mission[i];

					ppcList = D3DTEXTURE_cpp_array;

					//goto found_mission;
					bFound = TRUE;
					break;
				}

				i++;
			}

			if ( !bFound )
			{
				// Not found - this happens on the frontend screen for example.
				// Carry on.
				first_time = FALSE;
				continue;
			}

		  //found_mission:;
		}

		while ( TRUE )
		{

			iSafetyTimeout--;
			if ( iSafetyTimeout <= 0 )
			{
				// Oops.
				ASSERT ( FALSE );
				break;
			}

			// This should be a command.
			char *pcCurString = *ppcList;
			ppcList++;
			ASSERT ( ( pcCurString[0] == '*' ) && ( pcCurString[1] == '*' ) && ( pcCurString[2] == '*' ) );
			if ( 0 == stricmp ( pcCurString+3, "END" ) )
			{
				// End of list.
				break;
			}

			// Find the page type
			if ( 0 == strncmp ( pcCurString, "***64_4:", 8 ) )
			{
				iPageType = D3DPAGE_64_4X4;
			}
			else if ( 0 == strncmp ( pcCurString, "***32_4:", 8 ) )
			{
				iPageType = D3DPAGE_32_4X4;
			}
			else if ( 0 == strncmp ( pcCurString, "***64_3:", 8 ) )
			{
				iPageType = D3DPAGE_64_3X3;
			}
			else if ( 0 == strncmp ( pcCurString, "***32_3:", 8 ) )
			{
				iPageType = D3DPAGE_32_3X3;
			}
			else
			{
				ASSERT ( FALSE );
			}


			// Set up the texture page.
			pageD3D[iNumD3DPages].bPageType = (UBYTE)iPageType;
			pageD3D[iNumD3DPages].bNumTextures = 0;
			pageD3D[iNumD3DPages].pcDirectory = pcCurString + 8;
			// Next string is the filename of the page.
			pageD3D[iNumD3DPages].pcFilename = *ppcList++;
			pageD3D[iNumD3DPages].ppcTextureList = ppcList;
			pageD3D[iNumD3DPages].pTex = NULL;

			// See how many textures there are.
			while ( (*ppcList)[0] != '*' )
			{
				//pageD3D[iNumD3DPages].pTextures[pageD3D[iNumD3DPages].bNumTextures] = NULL;
				pageD3D[iNumD3DPages].bNumTextures++;
				ppcList++;
			}

			switch ( iPageType )
			{
			case D3DPAGE_64_4X4:
			case D3DPAGE_32_4X4:
				ASSERT ( pageD3D[iNumD3DPages].bNumTextures <= 16 );
				break;
			case D3DPAGE_64_3X3:
			case D3DPAGE_32_3X3:
				ASSERT ( pageD3D[iNumD3DPages].bNumTextures <= 9 );
				break;
			default:
				ASSERT ( FALSE );
				break;
			}

			ASSERT ( iNumD3DPages < MAX_NUM_D3DPAGES );
			iNumD3DPages++;
		}

		// Texture pages are not actually loaded until a texture on them
		// is loaded - it's all demand-loaded. Hooray!

		if (first_time)
		{
			first_time = FALSE;
		}
		else
		{
			break;
		}
	}
#endif


#endif

	// And redo all the render states and sorting.
	POLY_reset_render_states();

}




#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

void D3DPage::EnsureLoaded ( void )
{
	if ( this->pTex != NULL )
	{
		// Cool - already done.
		return;
	}

	// OK, we need to load this up.
	this->pTex = MFnew<D3DTexture>();
	ASSERT ( this->pTex != NULL );

// Set to 1 to allow null or missing filenames
#define JUST_TESTING 0

	HRESULT hres = this->pTex->LoadTextureTGA ( this->pcFilename, -1, TRUE );
	if ( FAILED(hres) )
	{
#ifdef TARGET_DC
		// File missing.
#if !JUST_TESTING
		ASSERT ( FALSE );
#else
		// Just testing, so don't die, just don't page.
#endif
		this->pTex = NULL;
#else
		// File probably not there - autogenerate it on the PC and save it out.
		// For now, do nothing.
		this->pTex = NULL;
#endif
	}
}


void D3DPage::Unload ( void )
{
	if ( this->pTex != NULL )
	{
		ASSERT ( pTex != NULL );
		pTex->Destroy();
		MFdelete(pTex);
		pTex = NULL;
	}
}


#endif //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB


#undef JUST_TESTING




#ifdef TEX_EMBED
#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
void D3DTexture::GetTexOffsetAndScale ( float *pfUScale, float *pfUOffset, float *pfVScale, float *pfVOffset )
{
	switch ( bPageType )
	{
	case D3DPAGE_NONE:
		*pfUScale = 1.0f;
		*pfVScale = 1.0f;
		*pfUOffset = 0.0f;
		*pfVOffset = 0.0f;
		break;
	case D3DPAGE_64_3X3:
	case D3DPAGE_32_3X3:
		// Arranged with 32-pixel gaps between textures, and
		// the textures are right up against the edge.
		// So along the edge, you have 64 texels, 32 pagging, 64 texels, 32 padding, 64 texels.
		// So the offsets are 0.0, 0.375, 0.75
		*pfUScale = 0.25f;
		*pfVScale = 0.25f;
		*pfUOffset = 0.375f * (float)( bPagePos % 3 );
		*pfVOffset = 0.375f * (float)( bPagePos / 3 );
		break;
	case D3DPAGE_64_4X4:
	case D3DPAGE_32_4X4:
		// Edge-to-edge packing.
		*pfUScale = 0.25f;
		*pfVScale = 0.25f;
		*pfUOffset = 0.25f * (float)( bPagePos & 0x3 );
		*pfVOffset = 0.25f * (float)( bPagePos >> 2 );
		break;
	default:
		ASSERT ( FALSE );
		break;
	}

}

#endif //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
#endif //#ifdef TEX_EMBED


#ifndef TARGET_DC

//---------------------------------------------------------------

HRESULT	D3DTexture::LoadTexture8(CBYTE *tex_file,CBYTE *pal_file)
{
	HRESULT		result;

	ASSERT(Type == D3DTEXTURE_TYPE_UNUSED);

	ASSERT(0);	// the TGA clumper doesn't deal with 8bpp textures

	lp_Texture = NULL;
#ifndef TARGET_DC
	lp_Palette = NULL;
#endif
	lp_Surface = NULL;


	// Check parameters.
	if((!tex_file) || (!pal_file))
	{
		// Invalid parameters.
		return	DDERR_GENERIC;
	}

	strcpy(texture_name,tex_file);
#ifdef TARGET_DC
	// We don't support 8-bit sources.
	ASSERT(FALSE);
#else
	strcpy(palette_name,pal_file);
#endif

	Type = D3DTEXTURE_TYPE_8;

	result = Reload();

	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to load texture\n");
	}

	//
	// Finally let the display driver know about this texture page.
	//

	the_display.AddLoadedTexture(this);

	return	DD_OK;
}

#endif //#ifndef TARGET_DC


HRESULT	D3DTexture::ChangeTextureTGA(CBYTE *tga_file) {

	if (Type != D3DTEXTURE_TYPE_UNUSED)
	{
		//
		// There must be one already loaded.
		//
		Destroy();
		strcpy(texture_name, tga_file);
		Reload();

		return DD_OK;
	}
	return	DDERR_GENERIC;
}

HRESULT	D3DTexture::LoadTextureTGA(CBYTE *tga_file, ULONG id,BOOL bCanShrink)
{
	HRESULT		result;

	if (Type != D3DTEXTURE_TYPE_UNUSED)
	{
		//
		// Already loaded.
		//

		return DD_OK;
	}

	lp_Texture = NULL;
#ifndef TARGET_DC
	lp_Palette = NULL;
#endif
	lp_Surface = NULL;

	this->bCanShrink = bCanShrink;

	// Check parameters.
	if(!tga_file)
	{
		// Invalid parameters.
		return	DDERR_GENERIC;
	}

	strcpy(texture_name, tga_file);
	ID = id;

	Type = D3DTEXTURE_TYPE_TGA;

	result = Reload();

	if(FAILED(result))
	{
		DebugText("LoadTextureTGA: unable to load texture\n");
		return ( result );
	}

	//
	// Finally let the display driver know about this texture page.
	//

	the_display.AddLoadedTexture(this);

	return	DD_OK;
}


HRESULT D3DTexture::CreateUserPage(SLONG texture_size, BOOL i_want_an_alpha_channel)
{
	HRESULT result;

	ASSERT(Type == D3DTEXTURE_TYPE_UNUSED);

	lp_Texture = NULL;
#ifndef TARGET_DC
	lp_Palette = NULL;
#endif
	lp_Surface = NULL;
	UserWantsAlpha       = i_want_an_alpha_channel;

	//
	// A user page.
	//

	size = texture_size;

	//
	// Reload it... or rather, re-create it, or even create it in the first place!
	//

	Type = D3DTEXTURE_TYPE_USER;

	result = Reload();

	if (FAILED(result))
	{
		DebugText("Could not create user page.\n");
	}

	//
	// Let the display driver know about this texture page.
	//

	the_display.AddLoadedTexture(this);

	return DD_OK;
}



//---------------------------------------------------------------


//
// Given the bitmask for a colour in a pixel format, it calculates the mask and
// shift so that you can construct a pixel in pixel format given its RGB values.
// The formula is...
//
//	PIXEL(r,g,b) = ((r >> mask) << shift) | ((g >> mask) << shift) | ((b >> mask) << shift);
// 
// THIS ASSUMES that r,g,b are 8-bit values.
//

void OS_calculate_mask_and_shift(
		ULONG  bitmask,
		SLONG *mask,
		SLONG *shift)
{
	SLONG i;
	SLONG b;
	SLONG num_bits  =  0;
	SLONG first_bit = -1;

	for (i = 0, b = 1; i < 32; i++, b <<= 1)
	{
		if (bitmask & b)
		{
			num_bits += 1;

			if (first_bit == -1)
			{
				//
				// We have found the first bit.
				//

				first_bit = i;
			}
		}
	}

	ASSERT(first_bit != -1 && num_bits != 0);

	*mask  = 8 - num_bits;
	*shift = first_bit;

	if (*mask < 0)
	{
		//
		// More than 8 bits per colour component? May
		// as well support it!
		//

		*shift -= *mask;
		*mask   =  0;
	}
}


HRESULT D3DTexture::Reload_TGA(void)
{
	//SLONG i;

	D3DDeviceInfo *current_device;

	DDModeInfo *mi;

	//SLONG bpp;

	DDSURFACEDESC2			dd_sd;

	int iMipmapLevel;

	TRACE ("Tex<%s>\n", texture_name);


#ifdef TARGET_DC



#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB



	// Extract the directory and file name.
#define MAX_DIRECTORY_LENGTH 63
	char pcDirectory[MAX_DIRECTORY_LENGTH+1];
	char *pcFilename;
	// Find the last backslash.
	char *pcTemp = texture_name;
	while ( *pcTemp != '\0' )
	{
		if ( *pcTemp == '\\' )
		{
			pcFilename = pcTemp + 1;
		}
		pcTemp++;
	}
	ASSERT ( pcFilename - texture_name < MAX_DIRECTORY_LENGTH );
	strncpy ( pcDirectory, texture_name, ( pcFilename - texture_name ) );
	pcDirectory[pcFilename - texture_name] = '\0';


	// Shouldn't start with a slash, but should end with one.
	ASSERT ( pcDirectory[0] != '\\' );
	ASSERT ( pcDirectory[strlen(pcDirectory)-1] == '\\' );


	// Spot the special texture that _shouldn't_ be paged at the moment.
	// These are thug jackets and civillian clothes.
	bool bOKToPage = TRUE;

#define DONT_PAGE_IF_ITS(dont_directory, dont_name) if ( ( 0 == strcmp ( pcDirectory, dont_directory ) ) && ( 0 == strcmp ( pcFilename, dont_name ) ) ) { bOKToPage = FALSE; }

	// Jacket 1 (brown).
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people\\", "tex021.tga" );			// 64+21
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people\\", "tex022.tga" );			// 64+22
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people\\", "tex024.tga" );			// 64+24
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people\\", "tex025.tga" );			// 64+25

	// Jacket 2 (green)
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex002.tga" );			// 10*64+2
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex003.tga" );			// 10*64+3
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex004.tga" );			// 10*64+4
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex005.tga" );			// 10*64+5

	// Jacket 3 (purple)
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex032.tga" );			// 10*64+32
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex033.tga" );			// 10*64+33
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex036.tga" );			// 10*64+36
	DONT_PAGE_IF_ITS ( "server\\textures\\shared\\people2\\", "tex037.tga" );			// 10*64+37

#undef DONT_PAGE_IF_ITS

	// Civillians (that's all that is in this directory).
	if ( ( 0 == strcmp ( pcDirectory, "server\\textures\\shared\\people3\\" ) ) ) { bOKToPage = FALSE; }


	if ( !bOKToPage )
	{
		// Load as normal.
		TRACE ( "Didn't page <%s>", texture_name );
 		wPageNum = -1;
		bPagePos = 0;
		bPageType = D3DPAGE_NONE;
	}
	else
	{
		// Look for this texture's name in the texture page list.
		for ( int i = 0; i < iNumD3DPages; i++ )
		{
			if ( 0 == stricmp ( pcDirectory, pageD3D[i].pcDirectory ) )
			{
				// Got a match. Look for the texture name.
				char **ppList = pageD3D[i].ppcTextureList;
				for ( int j = 0; j < (int)pageD3D[i].bNumTextures; j++ )
				{
					if ( 0 == stricmp ( *ppList, pcFilename ) )
					{
						// Found a match.

						// Make sure the page is loaded.
						pageD3D[i].EnsureLoaded();

 						wPageNum = i;
						bPagePos = j;
						bPageType = pageD3D[i].bPageType;

						if ( pageD3D[i].pTex != NULL )
						{
							lp_Texture = pageD3D[i].pTex->lp_Texture;
							lp_Surface = pageD3D[i].pTex->lp_Surface;
							if ( lp_Texture != NULL )
							{
								lp_Texture->AddRef();
							}
							else
							{
								ASSERT ( FALSE );
							}
							if ( lp_Surface != NULL )
							{
								lp_Surface->AddRef();
							}
							else
							{
								ASSERT ( FALSE );
							}
							ContainsAlpha = pageD3D[i].pTex->ContainsAlpha;
							mask_red    = pageD3D[i].pTex->mask_red    ;
							mask_green  = pageD3D[i].pTex->mask_green  ;
							mask_blue   = pageD3D[i].pTex->mask_blue   ;
							mask_alpha  = pageD3D[i].pTex->mask_alpha  ;
							shift_red   = pageD3D[i].pTex->shift_red   ;
							shift_green = pageD3D[i].pTex->shift_green ;
							shift_blue  = pageD3D[i].pTex->shift_blue  ;
							shift_alpha = pageD3D[i].pTex->shift_alpha ;
						}
						else
						{
							// Nads. Well, just continue for now.
							goto found_and_continue;
							ASSERT ( FALSE );
						}
						return DD_OK;
					}
					ppList++;
				}
			}
		}
		// Didn't find it - load as normal.
 		wPageNum = -1;
		bPagePos = 0;
		bPageType = D3DPAGE_NONE;

found_and_continue:;
	}

#endif


	if ( bCanShrink )
	{

// Mipmaps chew too much memory for now, and shrunk down they look ugly.
//#define MIPMAP_PLEASE_BOB
//#define MIPMAP_SHRINK_PLEASE_BOB 1

		// See if the "mvq" file is there, and load it if it is,
		// rather than calculating it, which takes forever.
		char new_filename[100];
		strcpy ( new_filename, texture_name );
		// Change the ".tga" extension to a ".mvq" (Mipmapped VQ).
		char *pch = new_filename;
		while ( *pch != '\0' )
		{
			if ( ( pch[0] == 't' ) &&
				 ( pch[1] == 'g' ) &&
				 ( pch[2] == 'a' ) )
			{
				pch[0] = 'm';
				pch[1] = 'v';
				pch[2] = 'q';
				break;
			}
			pch++;
		}


		// This is bodge!
		char *new_new_filename = new_filename;
		if ( 0 == strnicmp ( new_filename, "c:\\fallen\\", strlen ( "c:\\fallen\\" ) ) )
		{
			// Strip this off.
			new_new_filename = new_filename + strlen ( "c:\\fallen\\" );
		}


		MFFileHandle handle = FileOpen ( new_new_filename );
		if ( handle != FILE_OPEN_ERROR )
		{
			// Yep - load it.

			// Header is:
			// byte: version number - currently always 1
			// byte: size as a power of two, e.g. 5 = 32x32.
			// byte: pixel format. 0 = 565, 1 = 4444.
			// byte: padding - ignored
			// Then comes 2k of codebook, then the mipmap levels, smallest level first.
			// The smallest level is only a single byte and is padded with another byte before it.


			// Bin old texture stuff (whatever that means).
			Destroy();


			// Load it all at once.
			int iFileSize = FileSize ( handle );
			ASSERT ( iFileSize > 0 );
			ASSERT ( iFileSize < 1000000 );


#if 0
			char *pcFile = (char *)MemAlloc ( iFileSize );
			ASSERT ( pcFile != NULL );
			char *pcCurFilePos = pcFile;

			int iRead = FileRead ( handle, (void *)pcFile, iFileSize );
			ASSERT ( iRead == iFileSize );
#else

			char *pcFile = (char *)FastLoadFileSomewhere ( handle, iFileSize );
			char *pcCurFilePos = pcFile;

#endif

			FileClose ( handle );


			// OK, check the header.

			// Version.
			ASSERT ( pcFile[0] == 1 );

			// Log2 size.
			size = 1 << pcFile[1];
#ifdef MIPMAP_SHRINK_PLEASE_BOB
			// Shrink it.
			size >>= MIPMAP_SHRINK_PLEASE_BOB;
#endif

			// Format
			// the mask and shift values are not actually used, but might as well fill them in.
			switch ( pcFile[2] )
			{
			case 0:
				// 565.
				ContainsAlpha = 0;
				mask_red    = 5;
				mask_green  = 6;
				mask_blue   = 5;
				mask_alpha  = 0;
				shift_red   = 11;
				shift_green = 5;
				shift_blue  = 0;
				shift_alpha = 0;
				mi = the_display.GetDeviceInfo()->OpaqueTexFmt;
				break;
			case 1:
				// 4444.
				ContainsAlpha = 1;
				mask_red    = 4;
				mask_green  = 4;
				mask_blue   = 4;
				mask_alpha  = 4;
				shift_red   = 8;
				shift_green = 4;
				shift_blue  = 0;
				shift_alpha = 12;
				mi = the_display.GetDeviceInfo()->AlphaTexFmt;
				break;
			default:
				// Unknown
				ASSERT ( FALSE );
				break;
			}


			// pcFile[3] is padding.

			pcCurFilePos += 4;




			// OK, now create a VQ'd mipmapped surface and fill it.

			int iNumMipmapLevels = 0;
			DWORD dwSize = size;
			// Only mipmap down to 2x2 for VQs.
			while ( dwSize > 1 )
			{
				dwSize >>= 1;
				iNumMipmapLevels++;
			}
#ifdef MIPMAP_SHRINK_PLEASE_BOB
			// There are actually more than this in the file.
			iNumMipmapLevels += MIPMAP_SHRINK_PLEASE_BOB;
#endif




			// The VQ format is something I've basically guessed at!
			// But it seems to work.
			DDSURFACEDESC2			dd_sd2;

			dd_sd2 = mi->ddSurfDesc;

			dd_sd2.dwSize  = sizeof(dd_sd2);
	#ifdef MIPMAP_PLEASE_BOB
			dd_sd2.dwFlags =
				DDSD_CAPS   |
				DDSD_HEIGHT |
				DDSD_WIDTH  |
				DDSD_PIXELFORMAT |
				DDSD_MIPMAPCOUNT;
	#else
			dd_sd2.dwFlags =
				DDSD_CAPS   |
				DDSD_HEIGHT |
				DDSD_WIDTH  |
				DDSD_PIXELFORMAT;
	#endif
			dd_sd2.dwWidth  = size;
			dd_sd2.dwHeight = size;
	#ifdef MIPMAP_PLEASE_BOB
			dd_sd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE| DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
	#else
			dd_sd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	#endif
			dd_sd2.ddsCaps.dwCaps2 = 0;
			dd_sd2.ddsCaps.dwCaps3 = 0;
			dd_sd2.ddsCaps.dwCaps4 = 0;
			dd_sd2.dwTextureStage = 0;
	#ifdef MIPMAP_PLEASE_BOB
			dd_sd2.dwMipMapCount = iNumMipmapLevels;
#ifdef MIPMAP_SHRINK_PLEASE_BOB
			dd_sd2.dwMipMapCount -= MIPMAP_SHRINK_PLEASE_BOB;
#endif

	#else
			dd_sd2.dwMipMapCount = 0;
	#endif

			dd_sd2.ddpfPixelFormat.dwFlags |= DDPF_COMPRESSED;

			VERIFY(SUCCEEDED(the_display.lp_DD4->CreateSurface(&dd_sd2, &lp_Surface, NULL)));
			VERIFY(SUCCEEDED(lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture)));



			// Now lock the surface.
			dd_sd2.dwSize = sizeof(dd_sd);
			dd_sd2.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
			HRESULT	res = lp_Surface->Lock(NULL, &dd_sd2, 0, NULL);
			ASSERT(SUCCEEDED(res));


			// Fill the codebook.
			WORD *pwDst = (WORD *)dd_sd2.lpSurface;
			for ( int i = 0; i < 256; i++ )
			{
				pwDst[0] = *((WORD*)pcCurFilePos );
				pcCurFilePos += 2;
				pwDst[1] = *((WORD*)pcCurFilePos );
				pcCurFilePos += 2;
				pwDst[2] = *((WORD*)pcCurFilePos );
				pcCurFilePos += 2;
				pwDst[3] = *((WORD*)pcCurFilePos );
				pcCurFilePos += 2;
				pwDst += 4;
			}


			// OK, the mipmaps are stored in the top level surface (no GetAttachedSurface or any of that
			// rubbish), and stored smallest-first.

			// Now fill in the mipmaps - they use the same codebook.
			UCHAR *pPtr = (UCHAR *) dd_sd2.lpSurface + 2048;
#ifdef MIPMAP_PLEASE_BOB
			// Special-case the first level. It's only a single byte, but writes to
			// vidmem must be WORD-aligned.
			*(WORD *)pPtr = (WORD)*pcCurFilePos;
			pPtr += 2;
#endif
			pcCurFilePos += 2;


			// Now do the rest.
			dwSize = 4;
			for ( iMipmapLevel = iNumMipmapLevels - 2; iMipmapLevel >= 0; iMipmapLevel-- )
			{
#ifndef MIPMAP_PLEASE_BOB
				if ( iMipmapLevel != 0 )
				{
					// No mipmapping - skip all but the last level.
					pcCurFilePos += ( (dwSize * dwSize) / 4 );
				}
				else
#endif
#ifdef MIPMAP_SHRINK_PLEASE_BOB
				if ( iMipmapLevel < MIPMAP_SHRINK_PLEASE_BOB )
				{
					// Skip the larger levels.
					pcCurFilePos += ( (dwSize * dwSize) / 4 );
				}
				else
#endif
				{
					WORD *pwTemp = (WORD *)pcCurFilePos;
					for ( int i = (dwSize * dwSize) / 4; i > 0; i -= 2 )
					{
						*(WORD*)pPtr = *pwTemp;
						pwTemp++;
						pPtr+=2;
					}

					pcCurFilePos = (char *)pwTemp;
				}
				dwSize <<= 1;
			}

			ASSERT ( ( pcCurFilePos - pcFile ) == iFileSize );
#if 0
			MemFree ( pcFile );
#endif

			VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));



			static int iTexLoadCount = 0;
			TRACE(":");
			iTexLoadCount++;
			if ( ( iTexLoadCount & 0x3f ) == 0 )
			{
				TRACE("\n");
			}

			return DD_OK;

		}

	}
		
#endif

	TGA_Info   ti;
	TGA_Pixel *tga;

	//HRESULT result;

	//
	// Allocate memory for the texture.
	//

	tga = (TGA_Pixel *) MemAlloc (256 * 256 * sizeof(TGA_Pixel));

	if (tga == NULL)
	{
		TRACE("Not enough MAIN memory to load tga %s\n", texture_name);

		return DDERR_GENERIC;
	}

	//
	// Load the texture.
	//
	ti = TGA_load(
			texture_name,
			256,
			256,
			tga,
			ID,
			bCanShrink);

	if (!ti.valid)
	{
		//
		// Invalid tga.
		//

		TRACE("TGA %s is invalid\n", texture_name);
		//ASSERT ( FALSE );
		MemFree(tga);
		return DDERR_GENERIC;
	}

	if (ti.width != ti.height)
	{
		TRACE("TGA %s is not square\n", texture_name);
		MemFree(tga);
		return DDERR_GENERIC;
	}

#if 0
	if (ti.width != 32 &&
		ti.width != 64 &&
		ti.width != 128 &&
		ti.width != 256)
#else
	// Tsk tsk - how do we test for powers of two? :-)
	if ( ( ti.width & ( ti.width - 1 ) ) != 0 )
#endif
	{
		TRACE("TGA %s is not a valid size", texture_name);
		MemFree(tga);
		return DDERR_GENERIC;
	}

	#if WE_WANT_ALL_TEXTURES_TO_BE_256_x_256

	if (ti.width != 256)
	{
		TRACE("Texture \"%s\" (%d x %d) is an invalid size", texture_name, ti.width, ti.height);
		MemFree(tga);
		return DDERR_GENERIC;
	}

	#endif

	if (the_manager.CurrDriver && (the_manager.CurrDriver->DriverFlags & DD_DRIVER_LOW_MEMORY))
	{
		if (strstr(texture_name, "multifont") ||
			strstr(texture_name, "PCdisplay") ||
			strstr(texture_name, "olyfont"))
		{
			//
			// Don't halve these textures...
			//
		}
		else
		{
			//
			// Shall we halve the size of this texture? YEAH!
			//

#ifndef TARGET_DC
			extern void SW_halfsize(TGA_Pixel *tga, SLONG size);

			SW_halfsize(tga, ti.width);

			ti.width  >>= 1;
			ti.height >>= 1;

			tga = (TGA_Pixel *) realloc(tga, ti.width * ti.height * sizeof(TGA_Pixel));
#endif
		}
	}

	size = ti.width;
	
	//
	// Get the current device.
	//

	current_device = the_display.GetDeviceInfo();

	if (!current_device)
	{
		TRACE("No device!\n");

		return DDERR_GENERIC;
	}

#ifdef TARGET_DC
	static int iTexLoadCount = 0;
	TRACE(".");
	iTexLoadCount++;
	if ( ( iTexLoadCount & 0x3f ) == 0 )
	{
		TRACE("\n");
	}
#else
	TRACE("texture = %s\n", this->texture_name);
#endif

	//
	// Does this texture page contain alpha?
	//

	ContainsAlpha = ti.contains_alpha;
	
	//
	// Find the best texture format.
	// 

	if (ContainsAlpha)
	{
		mi = current_device->AlphaTexFmt;
	}
	else
	{
		mi = current_device->OpaqueTexFmt;
	}

	//
	// Use the best texture format.
	// 

	SLONG dwMaskR, dwMaskG, dwMaskB, dwMaskA;
	SLONG dwShiftR, dwShiftG, dwShiftB, dwShiftA;

	OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRBitMask, &dwMaskR, &dwShiftR );
	OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwGBitMask, &dwMaskG, &dwShiftG );
	OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwBBitMask, &dwMaskB, &dwShiftB );

	if (ContainsAlpha)
	{
		OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRGBAlphaBitMask, &dwMaskA, &dwShiftA);
	}
	mask_red = (UBYTE)dwMaskR;
	mask_green = (UBYTE)dwMaskG;
	mask_blue = (UBYTE)dwMaskB;
	mask_alpha = (UBYTE)dwMaskA;
	shift_red = (UBYTE)dwShiftR;
	shift_green = (UBYTE)dwShiftG;
	shift_blue = (UBYTE)dwShiftB;
	shift_alpha = (UBYTE)dwShiftA;

#ifndef TARGET_DC
#ifdef SAVE_MY_VQ_TEXTURES_PLEASE_BOB
	// Saving textures, so force the format to be the DC's one!
	if (ContainsAlpha)
	{
		mask_red    = 4;
		mask_green  = 4;
		mask_blue   = 4;
		mask_alpha  = 4;
		shift_red   = 8;
		shift_green = 4;
		shift_blue  = 0;
		shift_alpha = 12;
	}
	else
	{
		mask_red    = 3;
		mask_green  = 2;
		mask_blue   = 3;
		mask_alpha  = 8;
		shift_red   = 11;
		shift_green = 5;
		shift_blue  = 0;
		shift_alpha = 0;
	}
#endif
#endif



	//
	// Get rid of the old texture stuff.
	//

	Destroy();

	//	Guy.	Do all the font mapping stuff here.	
	if(IsFont())
	{
		CreateFonts(&ti,tga);

		//	Change the outline colour to black.
		SLONG		size	=	(ti.width*ti.height);

		while(size--)
		{
			if	(
					(tga+size)->red==0xff	&&
					(tga+size)->green==0	&&
					(tga+size)->blue==0xff
				)
			{
				(tga+size)->red		=	0;
				(tga+size)->green	=	0;
				(tga+size)->blue	=	0;
			}
		}
	}

	// replace red-only pixels with black
	//
	// WITHOUT AFFECTING BLACK'S ALPHA-CHANNELS. ATF.
	if (IsFont2())
	{
		SLONG	size = (ti.width * ti.height);

		while (size--)
		{
			if ((tga[size].green == 0) && (tga[size].blue == 0) && (tga[size].red > 128 ) )
			{
				tga[size].red = 0;
				tga[size].alpha = 0;
			}
		}
	}

	int	interlace;
	int	xoff,yoff;

#ifdef TARGET_DC
	bool bVQPlease = FALSE;
#ifndef DONT_VQ_MY_TEXTURES_PLEASE_BOB
	if ( bCanShrink && ( ti.width >= VQ_MINIMUM_SIZE ) && ( ti.height >= VQ_MINIMUM_SIZE ) )
	{
		bVQPlease = TRUE;
		dd_sd = mi->ddSurfDesc;
		dd_sd.lpSurface = MemAlloc ( ti.width * ti.height * 2 );
		interlace = ti.width;
		xoff = yoff = 0;
#ifdef TEX_EMBED
#if !USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
		TexSource = this;
		TexOffset = 0;
#endif
#endif
	}
	else
#endif
#endif
	{
#ifdef TEX_EMBED
#if !USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
	if ((ti.width == 64) && (ti.height == 64) && !ContainsAlpha)
	{
		if (EmbedSurface)
		{
			TexOffset = 128 + EmbedOffset++;
			TexSource = EmbedSource;
			lp_Surface = EmbedSurface;
			lp_Texture = EmbedTexture;
			lp_Surface->AddRef();
			lp_Texture->AddRef();

			if (EmbedOffset == 16)
			{
				EmbedSource = NULL;
				EmbedSurface = NULL;
				EmbedTexture = NULL;
				EmbedOffset = 0;
			}

			xoff = (TexOffset & 3) * 64;
			yoff = ((TexOffset >> 2) & 3) * 64;
		}
		else
		{
			dd_sd = mi->ddSurfDesc;

			dd_sd.dwSize  = sizeof(dd_sd);
			dd_sd.dwFlags =
				DDSD_CAPS   |
				DDSD_HEIGHT |
				DDSD_WIDTH  |
				DDSD_PIXELFORMAT;
			dd_sd.dwWidth  = 256;
			dd_sd.dwHeight = 256;
			dd_sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
#ifdef TARGET_DC
			dd_sd.ddsCaps.dwCaps2 = 0;
#else
			dd_sd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#endif
			dd_sd.dwTextureStage = 0;

			VERIFY(SUCCEEDED(the_display.lp_DD4->CreateSurface(&dd_sd, &lp_Surface, NULL)));
			VERIFY(SUCCEEDED(lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture)));

			TexOffset = 128;
			TexSource = this;

			EmbedSource = this;
			EmbedSurface = lp_Surface;
			EmbedTexture = lp_Texture;
			EmbedOffset = 1;

			xoff = yoff = 0;
		}
		interlace = 256;
	}
	else
#endif
#endif
	{
		dd_sd = mi->ddSurfDesc;

		dd_sd.dwSize  = sizeof(dd_sd);
		dd_sd.dwFlags =
			DDSD_CAPS   |
			DDSD_HEIGHT |
			DDSD_WIDTH  |
			DDSD_PIXELFORMAT;
		dd_sd.dwWidth  = ti.width;
		dd_sd.dwHeight = ti.height;
		dd_sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
#ifdef TARGET_DC
		dd_sd.ddsCaps.dwCaps2 = 0;
#else
		dd_sd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#endif
		dd_sd.dwTextureStage = 0;

		VERIFY(SUCCEEDED(the_display.lp_DD4->CreateSurface(&dd_sd, &lp_Surface, NULL)));
		VERIFY(SUCCEEDED(lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture)));

		interlace = ti.width;
		xoff = yoff = 0;

#ifdef TEX_EMBED
#if !USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
		TexSource = this;
		TexOffset = 0;
#endif
#endif
	}
	}

	//
	// Lock the surface.
	//


#ifdef TARGET_DC
	if ( !bVQPlease )
#endif
	{
		dd_sd.dwSize = sizeof(dd_sd);
		HRESULT	res = lp_Surface->Lock(NULL, &dd_sd, 0, NULL);
		ASSERT(SUCCEEDED(res));
	}

	//
	// Copy the texture in
	//
	// ASSUMES 16 or 32-bits PER PIXEL!
	//

	{
		UWORD *wscreenw = (UWORD *) dd_sd.lpSurface;
		ULONG *wscreenl = (ULONG *) dd_sd.lpSurface;

		SLONG i;
		SLONG j;
		ULONG pixel;

		SLONG red;
		SLONG green;
		SLONG blue;

		SLONG bright;

		for (j = 0; j < ti.height; j++)
		for (i = 0; i < ti.width;  i++)
		{
			pixel = 0;

			red   = tga[i + j * ti.width].red;
			green = tga[i + j * ti.width].green;
			blue  = tga[i + j * ti.width].blue;
	
			/*

			//
			// Add some gamma!
			//

			red   = 256 - ((256 - red)   * (256 - red)   >> 8);
			green = 256 - ((256 - green) * (256 - green) >> 8);
			blue  = 256 - ((256 - blue)  * (256 - blue)  >> 8);

			if (red   > 255) {red   = 255;}
			if (green > 255) {green = 255;}
			if (blue  > 255) {blue  = 255;}

			*/

			if (GreyScale)
			{
				bright = (red + green + blue) * 85 >> 8;

				red   = bright;
				green = bright;
				blue  = bright;
			}

			pixel |= (red   >> mask_red  ) << shift_red;
			pixel |= (green >> mask_green) << shift_green;
			pixel |= (blue  >> mask_blue ) << shift_blue;

#define	ISPIXEL(x,y)	(tga[(x) + (y) * ti.width].red | tga[(x) + (y) * ti.width].green | tga[(x) + (y) * ti.width].blue)

			if (ContainsAlpha)
			{
				pixel |= (tga[i + j * ti.width].alpha >> mask_alpha) << shift_alpha;

				if (!pixel && !ISPIXEL(i,j))
				{
					// this is a bit bad ... we want to copy the nearest texel across
					int	i2,j2;

					if ((i - 1 >= 0) && ISPIXEL(i - 1, j))
					{
						i2 = i - 1;
						j2 = j;
					}
					else if ((i + 1 < ti.width) && ISPIXEL(i + 1, j))
					{
						i2 = i + 1;
						j2 = j;
					}
					else if ((j - 1 >= 0) && ISPIXEL(i, j - 1))
					{
						i2 = i;
						j2 = j - 1;
					}
					else if ((j + 1 < ti.height) && ISPIXEL(i, j + 1))
					{
						i2 = i;
						j2 = j + 1;
					}
					else if ((i - 1 >= 0) && (j - 1 >= 0) && ISPIXEL(i - 1, j - 1))
					{
						i2 = i - 1;
						j2 = j - 1;
					}
					else if ((i - 1 >= 0) && (j + 1 < ti.height) && ISPIXEL(i - 1, j + 1))
					{
						i2 = i - 1;
						j2 = j + 1;
					}
					else if ((i + 1 < ti.width) && (j - 1 >= 0) && ISPIXEL(i + 1, j - 1))
					{
						i2 = i + 1;
						j2 = j - 1;
					}
					else if ((i + 1 < ti.width) && (j + 1 < ti.height) && ISPIXEL(i + 1, j + 1))
					{
						i2 = i + 1;
						j2 = j + 1;
					}
					else
					{
						i2 = i;
						j2 = j;
					}

					red   = tga[i2 + j2 * ti.width].red;
					green = tga[i2 + j2 * ti.width].green;
					blue  = tga[i2 + j2 * ti.width].blue;
			
					pixel |= (red   >> mask_red  ) << shift_red;
					pixel |= (green >> mask_green) << shift_green;
					pixel |= (blue  >> mask_blue ) << shift_blue;
				}
			}

			if (dd_sd.ddpfPixelFormat.dwRGBBitCount == 32)
			{
				wscreenl[i + xoff + (j + yoff) * interlace] = pixel;
			}
			else
			{
				wscreenw[i + xoff + (j + yoff) * interlace] = (WORD)pixel;
			}
		}
	}

	//
	// Unlock the surface.
	//


#ifndef TARGET_DC



#ifdef SAVE_MY_VQ_TEXTURES_PLEASE_BOB
extern UBYTE save_out_the_vqs;

	//if (save_out_the_vqs)
	{

	// VQ and mipmap this texture and save the data format out.

	// Try to create the file first for an early-out.
	char new_filename[100];
	strcpy ( new_filename, texture_name );
	// Change the ".tga" extension to a ".mvq" (Mipmapped VQ).
	char *pch = new_filename;
	while ( *pch != '\0' )
	{
		if ( ( pch[0] == 't' ) &&
			 ( pch[1] == 'g' ) &&
			 ( pch[2] == 'a' ) )
		{
			pch[0] = 'm';
			pch[1] = 'v';
			pch[2] = 'q';
			break;
		}
		pch++;
	}

	MFFileHandle handle = FileCreate ( new_filename, FALSE );
	if ( handle == FILE_CREATION_ERROR )
	{
		// Probably already exists - don't update it.
		goto dont_save_mvq;
	}


	{

		// OK, now create a VQ'd mipmapped surface and fill it.
		int iNumMipmapLevels = 0;
		DWORD dwSize = ti.height;
		ASSERT ( ti.height == ti.width );
		ASSERT ( ti.height < 512 );
		// Only mipmap down to 2x2 for VQs.
		while ( dwSize > 1 )
		{
			dwSize >>= 1;
			iNumMipmapLevels++;
		}



		// USe the DC's pixel format, not whatever the PC uses.
		DDPIXELFORMAT ddpf_DC;
		if (ContainsAlpha)
		{
			ddpf_DC.dwRBitMask			= 0x0f00;
			ddpf_DC.dwGBitMask			= 0x00f0;
			ddpf_DC.dwBBitMask			= 0x000f;
			ddpf_DC.dwRGBAlphaBitMask	= 0xf000;
		}
		else
		{
			ddpf_DC.dwRBitMask			= 0xf800;
			ddpf_DC.dwGBitMask			= 0x07e0;
			ddpf_DC.dwBBitMask			= 0x001f;
			ddpf_DC.dwRGBAlphaBitMask	= 0x0000;
		}



		UWORD *(pwdst[10]);
		UCHAR *(pcTexels[10]);
		// This should be big enough...
		UCHAR *pcRealTexelBlock = (UCHAR *)MemAlloc ( ( ti.height * ti.height / 4 ) * 2 + 10*4 );
		ASSERT ( pcRealTexelBlock != NULL );
		//UCHAR pcRealTexelBlock[(512*512/4) * 2 + 10*4];
		// DWORD align it.
		UCHAR *pcTexelBlock = (UCHAR *)( ( (DWORD)pcRealTexelBlock + 3 ) & ~3 );

		dwSize = ti.height;
		//UCHAR pcpwdstBlock[512*512*2*2];
		UCHAR *pcpwdstBlock = (UCHAR *)MemAlloc ( 512*512*2*2 );
		ASSERT ( pcpwdstBlock != NULL );
		UCHAR *pcTemp = pcpwdstBlock;
		for ( int i = 0; i < iNumMipmapLevels; i++ )
		{
			//pwdst[i] = (UWORD *)MemAlloc ( 2 * dwSize * dwSize );
			pwdst[i] = (UWORD *)pcTemp;
			pcTemp += 2 * dwSize * dwSize;
			ASSERT ( pwdst[i] != NULL );
			pcTexels[i] = pcTexelBlock;
			pcTexelBlock += dwSize * dwSize / 4;
			pcTexelBlock = (UCHAR *)( ( (DWORD)pcTexelBlock + 3 ) & ~3 );
			if ( i > 0 )
			{
				// Create the mipmap by a cheesy box filter of the previous level.
				// FIXME - do a better filter.
				UWORD *pwsrc1 = pwdst[i-1];
				UWORD *pwsrc2 = pwdst[i-1] + dwSize * 2;
				UWORD *pwdst1 = pwdst[i];
				for ( int iY = 0; iY < (signed)dwSize; iY++ )
				{
					for ( int iX = 0; iX < (signed)dwSize; iX++ )
					{
						UWORD wR, wG, wB, wA;
						UWORD wSrc1 = *pwsrc1++;
						UWORD wSrc2 = *pwsrc1++;
						UWORD wSrc3 = *pwsrc2++;
						UWORD wSrc4 = *pwsrc2++;
						// Assume the blue channel is the bottom one.
						ASSERT ( ( ddpf_DC.dwRGBAlphaBitMask & 3 ) == 0 );
						ASSERT ( ( ddpf_DC.dwRBitMask & 3 ) == 0 );
						ASSERT ( ( ddpf_DC.dwGBitMask & 3 ) == 0 );
						ASSERT ( ( ddpf_DC.dwBBitMask & 0xc000 ) == 0 );
						wR  = (wSrc1 & (UWORD)ddpf_DC.dwRBitMask) >> 2;
						wR += (wSrc2 & (UWORD)ddpf_DC.dwRBitMask) >> 2;
						wR += (wSrc3 & (UWORD)ddpf_DC.dwRBitMask) >> 2;
						wR += (wSrc4 & (UWORD)ddpf_DC.dwRBitMask) >> 2;
						// Add half an LSB.
						wR += ( (UWORD)ddpf_DC.dwRBitMask >> 1) & ~(UWORD)ddpf_DC.dwRBitMask;
						wR &= ddpf_DC.dwRBitMask;
						wG  = (wSrc1 & (UWORD)ddpf_DC.dwGBitMask) >> 2;
						wG += (wSrc2 & (UWORD)ddpf_DC.dwGBitMask) >> 2;
						wG += (wSrc3 & (UWORD)ddpf_DC.dwGBitMask) >> 2;
						wG += (wSrc4 & (UWORD)ddpf_DC.dwGBitMask) >> 2;
						wG += ( (UWORD)ddpf_DC.dwGBitMask >> 1) & ~(UWORD)ddpf_DC.dwGBitMask;
						wG &= ddpf_DC.dwGBitMask;
						wB  = (wSrc1 & (UWORD)ddpf_DC.dwBBitMask);
						wB += (wSrc2 & (UWORD)ddpf_DC.dwBBitMask);
						wB += (wSrc3 & (UWORD)ddpf_DC.dwBBitMask);
						wB += (wSrc4 & (UWORD)ddpf_DC.dwBBitMask);
						wR += ( (UWORD)ddpf_DC.dwBBitMask << 1) & ~( (UWORD)ddpf_DC.dwBBitMask << 2 );
						wB >>= 2;
						wB &= ddpf_DC.dwBBitMask;
						wA  = (wSrc1 & (UWORD)ddpf_DC.dwRGBAlphaBitMask) >> 2;
						wA += (wSrc2 & (UWORD)ddpf_DC.dwRGBAlphaBitMask) >> 2;
						wA += (wSrc3 & (UWORD)ddpf_DC.dwRGBAlphaBitMask) >> 2;
						wA += (wSrc4 & (UWORD)ddpf_DC.dwRGBAlphaBitMask) >> 2;
						wA += ( (UWORD)ddpf_DC.dwRGBAlphaBitMask >> 1) & ~(UWORD)ddpf_DC.dwRGBAlphaBitMask;
						wA &= (UWORD)ddpf_DC.dwRGBAlphaBitMask;
						*pwdst1++ = wR | wG | wB | wA;
					}
					// Skip a source line.
					pwsrc1 += dwSize * 2;
					pwsrc2 += dwSize * 2;
				}
			}
			else
			{
				// Copy from the source.
				memcpy ( pwdst[0], dd_sd.lpSurface, 2 * dwSize * dwSize );
			}
			dwSize >>= 1;
		}
		ASSERT ( dwSize == 1 );


		dwSize = ti.height;
		//UWORD pwTemp[512*512];
		UWORD *pwTemp = (UWORD *)MemAlloc ( 512 * 512 * 2);
		ASSERT ( pwTemp != NULL );
		for ( i = 0; i < iNumMipmapLevels; i++ )
		{
			// Swizzle and pack into 2x2 blocks.
			UWORD *pwsrc = (UWORD *) pwdst[i];
			UWORD *pwdst1 = (UWORD *) pwTemp;
			for ( int iY = 0; iY < (signed)dwSize; iY++ )
			{
				for ( int iX = 0; iX < (signed)dwSize; iX++ )
				{
					int iOffset = 0;
					iOffset |= ( iY << 0 ) & 0x00001;
					iOffset |= ( iX << 1 ) & 0x00002;
					iOffset |= ( iY << 1 ) & 0x00004;
					iOffset |= ( iX << 2 ) & 0x00008;
					iOffset |= ( iY << 2 ) & 0x00010;
					iOffset |= ( iX << 3 ) & 0x00020;
					iOffset |= ( iY << 3 ) & 0x00040;
					iOffset |= ( iX << 4 ) & 0x00080;
					iOffset |= ( iY << 4 ) & 0x00100;
					iOffset |= ( iX << 5 ) & 0x00200;
					iOffset |= ( iY << 5 ) & 0x00400;
					iOffset |= ( iX << 6 ) & 0x00800;
					iOffset |= ( iY << 6 ) & 0x01000;
					iOffset |= ( iX << 7 ) & 0x02000;
					iOffset |= ( iY << 7 ) & 0x04000;
					iOffset |= ( iX << 8 ) & 0x08000;
					iOffset |= ( iY << 8 ) & 0x10000;
					iOffset |= ( iX << 9 ) & 0x20000;
					pwdst1[iOffset] = *pwsrc++;
				}
			}
			// And copy it back in.
			memcpy ( pwdst[i], pwTemp, 2 * dwSize * dwSize );
			dwSize >>= 1;
		}
		MemFree ( pwTemp );

		//memset ( &(pwdst[ti.height * ti.width / 2]), 0, ti.width * ti.height );


		ULONG *pdwCodeBook1 = (ULONG *)MemAlloc ( 256 * 4 );
		ASSERT ( pdwCodeBook1 != NULL );
		ULONG *pdwCodeBook2 = (ULONG *)MemAlloc ( 256 * 4 );
		ASSERT ( pdwCodeBook2 != NULL );
		//ULONG pdwCodeBook1[256];
		//ULONG pdwCodeBook2[256];
		int iNumEntries = 0;



		// Let's do this another way. Scan multiple times, with smaller and smaller tolerance levels
		// until we run out of codebook space.
		for ( int iPrecision = 0; iPrecision < 10; iPrecision++ )
		{
			ULONG dwMask1, dwMask2;
	#define BINARY_DAMMIT(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)((a<<15)|(b<<14)|(c<<13)|(d<<12)|(e<<11)|(f<<10)|(g<<9)|(h<<8)|(i<<7)|(j<<6)|(k<<5)|(l<<4)|(m<<3)|(n<<2)|(o<<1)|(p<<0))
			if ( ContainsAlpha )
			{
				// RRRRGGGGBBBBAAAA
				switch ( iPrecision )
				{
				case 0: dwMask1 = BINARY_DAMMIT(1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0); break;
				case 1: dwMask1 = BINARY_DAMMIT(1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0); break;
				case 2: dwMask1 = BINARY_DAMMIT(1,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0); break;
				case 3: dwMask1 = BINARY_DAMMIT(1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0); break;
				case 4: dwMask1 = BINARY_DAMMIT(1,1,0,0,1,1,1,0,1,1,0,0,1,1,0,0); break;
				case 5: dwMask1 = BINARY_DAMMIT(1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0); break;
				case 6: dwMask1 = BINARY_DAMMIT(1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0); break;
				case 7: dwMask1 = BINARY_DAMMIT(1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0); break;
				case 8: dwMask1 = BINARY_DAMMIT(1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0); break;
				case 9: dwMask1 = BINARY_DAMMIT(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1); break;
				}
			}
			else
			{
				// RRRRRGGGGGGBBBBB
				switch ( iPrecision )
				{
				case 0: dwMask1 = BINARY_DAMMIT(1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0); break;
				case 1: dwMask1 = BINARY_DAMMIT(1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0); break;
				case 2: dwMask1 = BINARY_DAMMIT(1,1,0,0,0,1,1,0,0,0,0,1,1,0,0,0); break;
				case 3: dwMask1 = BINARY_DAMMIT(1,1,0,0,0,1,1,1,0,0,0,1,1,0,0,0); break;
				case 4: dwMask1 = BINARY_DAMMIT(1,1,1,0,0,1,1,1,0,0,0,1,1,1,0,0); break;
				case 5: dwMask1 = BINARY_DAMMIT(1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,0); break;
				case 6: dwMask1 = BINARY_DAMMIT(1,1,1,1,0,1,1,1,1,0,0,1,1,1,1,0); break;
				case 7: dwMask1 = BINARY_DAMMIT(1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0); break;
				case 8: dwMask1 = BINARY_DAMMIT(1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0); break;
				case 9: dwMask1 = BINARY_DAMMIT(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1); break;
				}
			}
			// Replicate this texel mask into the other four.
			dwMask1 |= dwMask1 << 16;
			dwMask2 = dwMask1;
	#undef BINARY_DAMMIT


			// Now scan the texture, making any codebook entries that don't exist.
			// FIXME - at the moment, the mipmap levels are not scanned - maybe I should.
			ULONG *pdwsrc = (ULONG *)pwdst[0];
			int iChunk;
			for ( iChunk = 0; iChunk < (ti.width * ti.height) / 4; iChunk++ )
			{
				ULONG dwSrc1 = pdwsrc[iChunk*2];
				ULONG dwSrc2 = pdwsrc[iChunk*2+1];
				ULONG dwSrc1Mask = dwSrc1 & dwMask1;
				ULONG dwSrc2Mask = dwSrc2 & dwMask2;
				// See if this chunk or something close is already in the codebook.
				for ( int j = 0; j < iNumEntries; j++ )
				{
					if ( ( ( pdwCodeBook1[j] & dwMask1 ) == dwSrc1Mask ) && ( ( pdwCodeBook2[j] & dwMask2 ) == dwSrc2Mask ) )
					{
						// Joy.
						break;
					}
				}
				if ( j == iNumEntries )
				{
					// Didn't find a match.
					if ( iNumEntries < 256 )
					{
						// Cool - just grow the table.
						pdwCodeBook1[iNumEntries] = dwSrc1;
						pdwCodeBook2[iNumEntries] = dwSrc2;
						iNumEntries++;
					}
					else
					{
						// No more entries - exit.
						goto no_more_entries_PC;
					}
				}
			}

		}
	no_more_entries_PC:

		// OK, filled the codebook, so rescan and find the closest matches for each texel,
		// this time using a decent geometric error, not masks!

		// Actually, do a loop of (a) find the closest entries, then (b) make the entries the average of
		// the things that use them.
		// On the last time round, all the mipmap levels do (a), and (b) is not done.

		for ( int iNumTimesRoundTheLoop = 2; iNumTimesRoundTheLoop >= 0; iNumTimesRoundTheLoop-- )
		{
			// Refind the closest match for each chunk.
			// FIXME - this only does the top mipmap level.
			dwSize = ti.height;
			int iLevelsThisTime;
			if ( iNumTimesRoundTheLoop == 0 )
			{
				// Last time round the loop, do all mipmap levels.
				iLevelsThisTime = iNumMipmapLevels;
			}
			else
			{
				// Just do the top one.
				iLevelsThisTime = 1;
			}
			for ( iMipmapLevel = 0; iMipmapLevel < iLevelsThisTime; iMipmapLevel++ )
			{
				ULONG *pdwsrc = (ULONG *)pwdst[iMipmapLevel];
				UCHAR *pcdst = pcTexels[iMipmapLevel];
				for ( int iChunk = 0; iChunk < (signed)(dwSize * dwSize) / 4; iChunk++ )
				{
					ULONG dwSrc1 = pdwsrc[iChunk*2];
					ULONG dwSrc2 = pdwsrc[iChunk*2+1];

					int iClosest = -1;
					int iClosestVal = 100000000;
					if ( ContainsAlpha )
					{
						for ( i = 0; i < iNumEntries; i++ )
						{
							if ( ( pdwCodeBook1[i] == dwSrc1 ) && ( pdwCodeBook2[i] == dwSrc2 ) )
							{
								// Perfect match.
								iClosest = i;
								break;
							}

							// RRRR GGGG BBBB AAAA
							int iVal, iVal1, iVal2, iVal3, iVal4;
							iVal1 = ( ( pdwCodeBook1[i] >> 24 ) & 0xf0 ) - ( ( dwSrc1 >> 24 ) & 0xf0 );
							iVal2 = ( ( pdwCodeBook1[i] >> 20 ) & 0xf0 ) - ( ( dwSrc1 >> 20 ) & 0xf0 );
							iVal3 = ( ( pdwCodeBook1[i] >> 16 ) & 0xf0 ) - ( ( dwSrc1 >> 16 ) & 0xf0 );
							iVal4 = ( ( pdwCodeBook1[i] >> 12 ) & 0xf0 ) - ( ( dwSrc1 >> 12 ) & 0xf0 );
							iVal  = ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
							iVal1 = ( ( pdwCodeBook1[i] >>  8 ) & 0xf0 ) - ( ( dwSrc1 >>  8 ) & 0xf0 );
							iVal2 = ( ( pdwCodeBook1[i] >>  4 ) & 0xf0 ) - ( ( dwSrc1 >>  4 ) & 0xf0 );
							iVal3 = ( ( pdwCodeBook1[i] >>  0 ) & 0xf0 ) - ( ( dwSrc1 >>  0 ) & 0xf0 );
							iVal4 = ( ( pdwCodeBook1[i] <<  4 ) & 0xf0 ) - ( ( dwSrc1 <<  4 ) & 0xf0 );
							iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
							iVal1 = ( ( pdwCodeBook2[i] >> 24 ) & 0xf0 ) - ( ( dwSrc2 >> 24 ) & 0xf0 );
							iVal2 = ( ( pdwCodeBook2[i] >> 20 ) & 0xf0 ) - ( ( dwSrc2 >> 20 ) & 0xf0 );
							iVal3 = ( ( pdwCodeBook2[i] >> 16 ) & 0xf0 ) - ( ( dwSrc2 >> 16 ) & 0xf0 );
							iVal4 = ( ( pdwCodeBook2[i] >> 12 ) & 0xf0 ) - ( ( dwSrc2 >> 12 ) & 0xf0 );
							iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
							iVal1 = ( ( pdwCodeBook2[i] >>  8 ) & 0xf0 ) - ( ( dwSrc2 >>  8 ) & 0xf0 );
							iVal2 = ( ( pdwCodeBook2[i] >>  4 ) & 0xf0 ) - ( ( dwSrc2 >>  4 ) & 0xf0 );
							iVal3 = ( ( pdwCodeBook2[i] >>  0 ) & 0xf0 ) - ( ( dwSrc2 >>  0 ) & 0xf0 );
							iVal4 = ( ( pdwCodeBook2[i] <<  4 ) & 0xf0 ) - ( ( dwSrc2 <<  4 ) & 0xf0 );
							iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
							if ( iVal < iClosestVal )
							{
								iClosestVal = iVal;
								iClosest = i;
							}
						}
						ASSERT ( iClosest != -1 );
						*pcdst++ = (UBYTE)iClosest;
					}
					else
					{
						for ( i = 0; i < iNumEntries; i++ )
						{
							if ( ( pdwCodeBook1[i] == dwSrc1 ) && ( pdwCodeBook2[i] == dwSrc2 ) )
							{
								// Perfect match.
								iClosest = i;
								break;
							}

							// RRRR RGGG GGGB BBBB
							int iVal, iVal1, iVal2, iVal3;
							iVal1 = ( ( pdwCodeBook1[i] >> 24 ) & 0xf8 ) - ( ( dwSrc1 >> 24 ) & 0xf8 );
							iVal2 = ( ( pdwCodeBook1[i] >> 19 ) & 0xfc ) - ( ( dwSrc1 >> 19 ) & 0xfc );
							iVal3 = ( ( pdwCodeBook1[i] >> 13 ) & 0xf8 ) - ( ( dwSrc1 >> 13 ) & 0xf8 );
							iVal  = ( ( iVal1 * iVal1 ) << 2 ) + ( iVal2 * iVal2 ) + ( ( iVal3 * iVal3 ) << 2 );
							iVal1 = ( ( pdwCodeBook1[i] >>  8 ) & 0xf8 ) - ( ( dwSrc1 >>  8 ) & 0xf8 );
							iVal2 = ( ( pdwCodeBook1[i] >>  3 ) & 0xfc ) - ( ( dwSrc1 >>  3 ) & 0xfc );
							iVal3 = ( ( pdwCodeBook1[i] <<  3 ) & 0xf8 ) - ( ( dwSrc1 <<  3 ) & 0xf8 );
							iVal += ( ( iVal1 * iVal1 ) << 2 ) + ( iVal2 * iVal2 ) + ( ( iVal3 * iVal3 ) << 2 );
							iVal1 = ( ( pdwCodeBook2[i] >> 24 ) & 0xf8 ) - ( ( dwSrc2 >> 24 ) & 0xf8 );
							iVal2 = ( ( pdwCodeBook2[i] >> 19 ) & 0xfc ) - ( ( dwSrc2 >> 19 ) & 0xfc );
							iVal3 = ( ( pdwCodeBook2[i] >> 13 ) & 0xf8 ) - ( ( dwSrc2 >> 13 ) & 0xf8 );
							iVal += ( ( iVal1 * iVal1 ) << 2 ) + ( iVal2 * iVal2 ) + ( ( iVal3 * iVal3 ) << 2 );
							iVal1 = ( ( pdwCodeBook2[i] >>  8 ) & 0xf8 ) - ( ( dwSrc2 >>  8 ) & 0xf8 );
							iVal2 = ( ( pdwCodeBook2[i] >>  3 ) & 0xfc ) - ( ( dwSrc2 >>  3 ) & 0xfc );
							iVal3 = ( ( pdwCodeBook2[i] <<  3 ) & 0xf8 ) - ( ( dwSrc2 <<  3 ) & 0xf8 );
							iVal += ( ( iVal1 * iVal1 ) << 2 ) + ( iVal2 * iVal2 ) + ( ( iVal3 * iVal3 ) << 2 );
							if ( iVal < iClosestVal )
							{
								iClosestVal = iVal;
								iClosest = i;
							}
						}
						ASSERT ( iClosest != -1 );
						*pcdst++ = (UBYTE)iClosest;
					}
				}
				dwSize >>= 1;
			}

			//if ( iNumTimesRoundTheLoop > 0 )
			{

				// Now make the entries be the average of the texels that use them.
				// FIXME - only looks at the top mipmap level.
				dwSize = ti.height;
				ULONG *pdwsrc = (ULONG *)pwdst[0];
				UCHAR *pcdst = pcTexels[0];

				for ( i = 0; i < iNumEntries; i++ )
				{
					float fA0 = 0.0f;
					float fR0 = 0.0f;
					float fG0 = 0.0f;
					float fB0 = 0.0f;
					float fA1 = 0.0f;
					float fR1 = 0.0f;
					float fG1 = 0.0f;
					float fB1 = 0.0f;
					float fA2 = 0.0f;
					float fR2 = 0.0f;
					float fG2 = 0.0f;
					float fB2 = 0.0f;
					float fA3 = 0.0f;
					float fR3 = 0.0f;
					float fG3 = 0.0f;
					float fB3 = 0.0f;
					int iNumTexels = 0;

					for ( int iChunk = 0; iChunk < (signed)(dwSize * dwSize) / 4; iChunk++ )
					{
						if ( pcdst[iChunk] == i )
						{
							ULONG dwSrc1 = pdwsrc[iChunk*2];
							ULONG dwSrc2 = pdwsrc[iChunk*2+1];
							iNumTexels++;

							if ( ContainsAlpha )
							{
								fA0 += (float)( ( dwSrc1 >> 28 ) & 0xf );
								fR0 += (float)( ( dwSrc1 >> 24 ) & 0xf );
								fG0 += (float)( ( dwSrc1 >> 20 ) & 0xf );
								fB0 += (float)( ( dwSrc1 >> 16 ) & 0xf );
								fA1 += (float)( ( dwSrc1 >> 12 ) & 0xf );
								fR1 += (float)( ( dwSrc1 >>  8 ) & 0xf );
								fG1 += (float)( ( dwSrc1 >>  4 ) & 0xf );
								fB1 += (float)( ( dwSrc1 >>  0 ) & 0xf );
								fA2 += (float)( ( dwSrc2 >> 28 ) & 0xf );
								fR2 += (float)( ( dwSrc2 >> 24 ) & 0xf );
								fG2 += (float)( ( dwSrc2 >> 20 ) & 0xf );
								fB2 += (float)( ( dwSrc2 >> 16 ) & 0xf );
								fA3 += (float)( ( dwSrc2 >> 12 ) & 0xf );
								fR3 += (float)( ( dwSrc2 >>  8 ) & 0xf );
								fG3 += (float)( ( dwSrc2 >>  4 ) & 0xf );
								fB3 += (float)( ( dwSrc2 >>  0 ) & 0xf );
							}
							else
							{
								fR0 += (float)( ( dwSrc1 >> 27 ) & 0x1f );
								fG0 += (float)( ( dwSrc1 >> 21 ) & 0x3f );
								fB0 += (float)( ( dwSrc1 >> 16 ) & 0x1f );
								fR1 += (float)( ( dwSrc1 >> 11 ) & 0x1f );
								fG1 += (float)( ( dwSrc1 >>  5 ) & 0x3f );
								fB1 += (float)( ( dwSrc1 >>  0 ) & 0x1f );
								fR2 += (float)( ( dwSrc2 >> 27 ) & 0x1f );
								fG2 += (float)( ( dwSrc2 >> 21 ) & 0x3f );
								fB2 += (float)( ( dwSrc2 >> 16 ) & 0x1f );
								fR3 += (float)( ( dwSrc2 >> 11 ) & 0x1f );
								fG3 += (float)( ( dwSrc2 >>  5 ) & 0x3f );
								fB3 += (float)( ( dwSrc2 >>  0 ) & 0x1f );
							}
						}
					}

					float fTemp = 1.0f / (float)iNumTexels;
					fA0 *= fTemp;
					fR0 *= fTemp;
					fG0 *= fTemp;
					fB0 *= fTemp;
					fA1 *= fTemp;
					fR1 *= fTemp;
					fG1 *= fTemp;
					fB1 *= fTemp;
					fA2 *= fTemp;
					fR2 *= fTemp;
					fG2 *= fTemp;
					fB2 *= fTemp;
					fA3 *= fTemp;
					fR3 *= fTemp;
					fG3 *= fTemp;
					fB3 *= fTemp;

					if ( ContainsAlpha )
					{
						pdwCodeBook1[i]  = ( (int)fA0 << 28 ) | ( (int)fR0 << 24 ) | ( (int)fG0 << 20 ) | ( (int)fB0 << 16 );
						pdwCodeBook1[i] |= ( (int)fA1 << 12 ) | ( (int)fR1 <<  8 ) | ( (int)fG1 <<  4 ) | ( (int)fB1 <<  0 );
						pdwCodeBook2[i]  = ( (int)fA2 << 28 ) | ( (int)fR2 << 24 ) | ( (int)fG2 << 20 ) | ( (int)fB2 << 16 );
						pdwCodeBook2[i] |= ( (int)fA3 << 12 ) | ( (int)fR3 <<  8 ) | ( (int)fG3 <<  4 ) | ( (int)fB3 <<  0 );
					}
					else
					{
						pdwCodeBook1[i]  = ( (int)fR0 << 27 ) | ( (int)fG0 << 21 ) | ( (int)fB0 << 16 );
						pdwCodeBook1[i] |= ( (int)fR1 << 11 ) | ( (int)fG1 <<  5 ) | ( (int)fB1 <<  0 );
						pdwCodeBook2[i]  = ( (int)fR2 << 27 ) | ( (int)fG2 << 21 ) | ( (int)fB2 << 16 );
						pdwCodeBook2[i] |= ( (int)fR3 << 11 ) | ( (int)fG3 <<  5 ) | ( (int)fB3 <<  0 );
					}
				}
			}
		}



		// Now bung it all out to file, including a header.
		// Remember to cope with endianness changes.
	//#define CONVERT_WORD_ENDIANNESS(thing) ( (WORD)( ( ( (thing) & 0x00ff ) << 8 ) | ( ( (thing) & 0xff00 ) >> 8 ) ) )
	//#define CONVERT_DWORD_ENDIANNESS(thing) ( (DWORD)( ( ( (thing) & 0x000000ff ) << 24 ) | ( ( (thing) & 0x0000ff00 ) << 8 ) | ( ( (thing) & 0x00ff0000 ) >> 8 ) | ( ( (thing) & 0xff000000 ) >> 24 ) ) )
	//#define CONVERT_DWORD_ENDIANNESS(thing) ( (DWORD)( ( ( (thing) & 0x000000ff ) << 8 ) | ( ( (thing) & 0x0000ff00 ) >> 8 ) | ( ( (thing) & 0x00ff0000 ) << 8 ) | ( ( (thing) & 0xff000000 ) >> 8 ) ) )
	#define CONVERT_DWORD_ENDIANNESS(thing) (thing)

	#if 0
		char new_filename[100];
		strcpy ( new_filename, texture_name );
		// Change the ".tga" extension to a ".mvq" (Mipmapped VQ).
		char *pch = new_filename;
		while ( *pch != '\0' )
		{
			if ( ( pch[0] == 't' ) &&
				 ( pch[1] == 'g' ) &&
				 ( pch[2] == 'a' ) )
			{
				pch[0] = 'm';
				pch[1] = 'v';
				pch[2] = 'q';
				break;
			}
			pch++;
		}

		MFFileHandle handle = FileCreate ( new_filename, TRUE );
		ASSERT ( handle != INVALID_HANDLE_VALUE );
	#endif

		// Header is:
		// byte: version number - currently always 1
		// byte: size as a power of two, e.g. 5 = 32x32.
		// byte: pixel format. 0 = 565, 1 = 4444.
		// byte: padding - ignored
		// Then comes 2k of codebook, then the mipmap levels, smallest level first.
		// The smallest level is only a single byte and is padded with another byte before it.

		int written;
		char cTemp = 1;
		written = FileWrite ( handle, (void *)&cTemp, 1 );
		ASSERT ( written == 1 );

		char cSize = 0;
		while ( ( 1 << cSize ) < ti.height )
		{
			cSize++;
		}
		ASSERT ( ( 1 << cSize ) == ti.height );
		ASSERT ( ( 1 << cSize ) == ti.width );
		written = FileWrite ( handle, (void *)&cSize, 1 );
		ASSERT ( written == 1 );

		char cFormat;
		if ( ContainsAlpha )
		{
			cFormat = 1;
		}
		else
		{
			cFormat = 0;
		}
		written = FileWrite ( handle, (void *)&cFormat, 1 );
		ASSERT ( written == 1 );

		// Junk padding.
		written = FileWrite ( handle, (void *)&cFormat, 1 );
		ASSERT ( written == 1 );


		// Write the codebook into the file.
		for ( i = 0; i < 256; i++ )
		{
			DWORD dwTemp;
			dwTemp = CONVERT_DWORD_ENDIANNESS ( pdwCodeBook1[i] );
			written = FileWrite ( handle, (void *)&dwTemp, 4 );
			ASSERT ( written == 4 );
			dwTemp = CONVERT_DWORD_ENDIANNESS ( pdwCodeBook2[i] );
			written = FileWrite ( handle, (void *)&dwTemp, 4 );
			ASSERT ( written == 4 );
			//*pdwdst++ = pdwCodeBook1[i];
			//*pdwdst++ = pdwCodeBook2[i];
		}


		// OK, the mipmaps are stored in the top level surface (no GetAttachedSurface or any of that
		// rubbish), and stored smallest-first.

		// Now fill in the mipmaps - they use the same codebook.
		//UCHAR *pPtr = (UCHAR *) dd_sd2.lpSurface + 2048;
		// Special-case the first level. It's only a single byte, but writes to
		// vidmem must be WORD-aligned.
		//*(WORD *)pPtr = (WORD)pcTexels[iNumMipmapLevels - 1][0];
		//pPtr += 2;

		// single byte of padding.
		cTemp = 0;
		written = FileWrite ( handle, (void *)&cTemp, 1 );
		ASSERT ( written == 1 );

		cTemp = pcTexels[iNumMipmapLevels - 1][0];
		written = FileWrite ( handle, (void *)&cTemp, 1 );
		ASSERT ( written == 1 );


		// Now do the rest.
		dwSize = 4;
		for ( iMipmapLevel = iNumMipmapLevels - 2; iMipmapLevel >= 0; iMipmapLevel-- )
		{
			unsigned char *pTemp = pcTexels[iMipmapLevel];

			int iNumBytes = (dwSize * dwSize) / 4;
			written = FileWrite ( handle, (void *)pTemp, iNumBytes );
			ASSERT ( written == iNumBytes );

			//for ( int i = (dwSize * dwSize) / 4; i > 0; i-- )
			//{
			//	*(WORD*)pPtr = *pTemp;
			//	pTemp++;
			//	pPtr+=2;
			//}

			//memcpy ( pPtr, pcTexels[iMipmapLevel], (dwSize * dwSize) / 4 );
			//memset ( pPtr, iMipmapLevel + 2, dwSize * dwSize / 4 );
			//pPtr += dwSize * dwSize / 4;

			//VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));

			dwSize <<= 1;
		}

		OutputDebugString ( "Saved MVQ version\n" );

		FileClose ( handle );

		MemFree ( pdwCodeBook2 );
		MemFree ( pdwCodeBook1 );
		MemFree ( pcpwdstBlock );
		MemFree ( pcRealTexelBlock );
	}


dont_save_mvq:;
	}

#endif //#ifdef SAVE_MY_VQ_TEXTURES_PLEASE_BOB

	VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));






















#else
	if ( !bVQPlease )
	{
		VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));
	}
	else
	{






		// OK, now create a VQ'd mipmapped surface and fill it.

		int iNumMipmapLevels = 0;
		DWORD dwSize = ti.height;
		ASSERT ( ti.height == ti.width );
		ASSERT ( ti.height < 512 );
		// Only mipmap down to 2x2 for VQs.
		while ( dwSize > 1 )
		{
			dwSize >>= 1;
			iNumMipmapLevels++;
		}



		// The VQ format is something I've basically guessed at!
		// But it seems to work.
		DDSURFACEDESC2			dd_sd2;

		dd_sd2 = mi->ddSurfDesc;

		dd_sd2.dwSize  = sizeof(dd_sd2);
#ifdef MIPMAP_PLEASE_BOB
		dd_sd2.dwFlags =
			DDSD_CAPS   |
			DDSD_HEIGHT |
			DDSD_WIDTH  |
			DDSD_PIXELFORMAT |
			DDSD_MIPMAPCOUNT;
#else
		dd_sd2.dwFlags =
			DDSD_CAPS   |
			DDSD_HEIGHT |
			DDSD_WIDTH  |
			DDSD_PIXELFORMAT;
#endif
		dd_sd2.dwWidth  = ti.width;
		dd_sd2.dwHeight = ti.height;
#ifdef MIPMAP_PLEASE_BOB
		dd_sd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE| DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
#else
		dd_sd2.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
#endif
		dd_sd2.ddsCaps.dwCaps2 = 0;
		dd_sd2.ddsCaps.dwCaps3 = 0;
		dd_sd2.ddsCaps.dwCaps4 = 0;
		dd_sd2.dwTextureStage = 0;
#ifdef MIPMAP_PLEASE_BOB
		dd_sd2.dwMipMapCount = iNumMipmapLevels;
#else
		dd_sd2.dwMipMapCount = 0;
#endif

		dd_sd2.ddpfPixelFormat.dwFlags |= DDPF_COMPRESSED;

		VERIFY(SUCCEEDED(the_display.lp_DD4->CreateSurface(&dd_sd2, &lp_Surface, NULL)));
		VERIFY(SUCCEEDED(lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture)));



#if 0
		// See if the "mvq" file is there, and load it if it is,
		// rather than calculating it, which takes forever.

		char new_filename[100];
		strcpy ( new_filename, texture_name );
		// Change the ".tga" extension to a ".mvq" (Mipmapped VQ).
		char *pch = new_filename;
		while ( *pch != '\0' )
		{
			if ( ( pch[0] == 't' ) &&
				 ( pch[1] == 'g' ) &&
				 ( pch[2] == 'a' ) )
			{
				pch[0] = 'm';
				pch[1] = 'v';
				pch[2] = 'q';
				break;
			}
			pch++;
		}

		MemFree ( dd_sd.lpSurface );

		MFFileHandle handle = FileOpen ( new_filename );
		if ( handle != INVALID_HANDLE_VALUE )
		{
			// Yep - load it.

			// Header is:
			// byte: version number - currently always 1
			// byte: size as a power of two, e.g. 5 = 32x32.
			// byte: pixel format. 0 = 565, 1 = 4444.
			// byte: padding - ignored
			// Then comes 2k of codebook, then the mipmap levels, smallest level first.
			// The smallest level is only a single byte and is padded with another byte before it.


			// Load it all at once.
			int iFileSize = FileSize ( handle );
			ASSERT ( iFileSize > 0 );
			ASSERT ( iFileSize < 1000000 );
			char *pcFile = (char *)MemAlloc ( iFileSize );
			ASSERT ( pcFile != NULL );
			char *pcCurFilePos = pcFile;

			int iRead = FileRead ( handle, (void *)pcFile, iFileSize );
			ASSERT ( iRead == iFileSize );

			FileClose ( handle );


			// OK, check the header.

			// Version.
			ASSERT ( pcFile[0] == 1 );

			// Log2 size.
			ASSERT ( ( 1 << pcFile[1] ) == ti.height );
			ASSERT ( ( 1 << pcFile[1] ) == ti.width );

			// Format
			if ( ContainsAlpha )
			{
				ASSERT ( pcFile[2] == 1 );
			}
			else
			{
				ASSERT ( pcFile[2] == 0 );
			}

			// pcFile[3] is padding.

			pcCurFilePos += 4;



			// Now lock the surface.
			dd_sd2.dwSize = sizeof(dd_sd);
			dd_sd2.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
			HRESULT	res = lp_Surface->Lock(NULL, &dd_sd2, 0, NULL);
			ASSERT(SUCCEEDED(res));


			// Write the codebook.
			WORD *pwDst = (WORD *)dd_sd2.lpSurface;
			for ( int i = 0; i < 256; i++ )
			{
				pwDst[1] = *((WORD *)pcCurFilePos);
				pcCurFilePos += 2;
				pwDst[0] = *((WORD *)pcCurFilePos);
				pcCurFilePos += 2;
				pwDst[3] = *((WORD *)pcCurFilePos);
				pcCurFilePos += 2;
				pwDst[2] = *((WORD *)pcCurFilePos);
				pcCurFilePos += 2;
				pwDst += 4;
			}


			// OK, the mipmaps are stored in the top level surface (no GetAttachedSurface or any of that
			// rubbish), and stored smallest-first.

			// Now fill in the mipmaps - they use the same codebook.
			UCHAR *pPtr = (UCHAR *) dd_sd2.lpSurface + 2048;
#ifdef MIPMAP_PLEASE_BOB
			// Special-case the first level. It's only a single byte, but writes to
			// vidmem must be WORD-aligned.
			*(WORD *)pPtr = (WORD)*pcCurFilePos;
			pPtr += 2;
#endif
			pcCurFilePos += 2;


			// Now do the rest.
			dwSize = 4;
			for ( iMipmapLevel = iNumMipmapLevels - 2; iMipmapLevel >= 0; iMipmapLevel-- )
			{
#ifndef MIPMAP_PLEASE_BOB
				if ( iMipmapLevel != 0 )
				{
					// No mipmapping - skip all but the last level.
					pcCurFilePos += ( (dwSize * dwSize) / 4 );
				}
				else
#endif
				{
					WORD *pwTemp = (WORD *)pcCurFilePos;
					for ( int i = (dwSize * dwSize) / 4; i > 0; i -= 2 )
					{
						*(WORD*)pPtr = *pwTemp;
						pwTemp++;
						pPtr+=2;
					}

					pcCurFilePos = (char *)pwTemp;
				}
				dwSize <<= 1;
			}

			ASSERT ( ( pcCurFilePos - pcFile ) == iFileSize );
			MemFree ( pcFile );

			VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));

		}
		else
#endif
		{

#ifdef DEBUG
			// Oh dear - we'll have to actually do the work.
			char cTemp[200];
			// Should have been an .mvq file present - fix it!
			sprintf ( cTemp, "Had to mipmap/VQ \"%s\" manually\n", texture_name );
			TRACE ( cTemp );
#endif


			UWORD *(pwdst[10]);
			UCHAR *(pcTexels[10]);
			// This should be big enough...
			UCHAR *pcRealTexelBlock = (UCHAR *)MemAlloc ( ( ti.height * ti.height / 4 ) * 2 + 10*4 );
			ASSERT ( pcRealTexelBlock != NULL );
			//UCHAR pcRealTexelBlock[(512*512/4) * 2 + 10*4];
			// DWORD align it.
			UCHAR *pcTexelBlock = (UCHAR *)( ( (DWORD)pcRealTexelBlock + 3 ) & ~3 );

			dwSize = ti.height;
			//UCHAR pcpwdstBlock[512*512*2*2];
			UCHAR *pcpwdstBlock = (UCHAR *)MemAlloc ( 512*512*2*2 );
			ASSERT ( pcpwdstBlock != NULL );
			UCHAR *pcTemp = pcpwdstBlock;
			for ( int i = 0; i < iNumMipmapLevels; i++ )
			{
				//pwdst[i] = (UWORD *)MemAlloc ( 2 * dwSize * dwSize );
				pwdst[i] = (UWORD *)pcTemp;
				pcTemp += 2 * dwSize * dwSize;
				ASSERT ( pwdst[i] != NULL );
				pcTexels[i] = pcTexelBlock;
				pcTexelBlock += dwSize * dwSize / 4;
				pcTexelBlock = (UCHAR *)( ( (DWORD)pcTexelBlock + 3 ) & ~3 );
				if ( i > 0 )
				{
					// Create the mipmap by a cheesy box filter of the previous level.
					// FIXME - do a better filter.
					UWORD *pwsrc1 = pwdst[i-1];
					UWORD *pwsrc2 = pwdst[i-1] + dwSize * 2;
					UWORD *pwdst1 = pwdst[i];
					for ( int iY = 0; iY < (signed)dwSize; iY++ )
					{
						for ( int iX = 0; iX < (signed)dwSize; iX++ )
						{
							UWORD wR, wG, wB, wA;
							UWORD wSrc1 = *pwsrc1++;
							UWORD wSrc2 = *pwsrc1++;
							UWORD wSrc3 = *pwsrc2++;
							UWORD wSrc4 = *pwsrc2++;
							// Assume the blue channel is the bottom one.
							ASSERT ( ( dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask & 3 ) == 0 );
							ASSERT ( ( dd_sd2.ddpfPixelFormat.dwRBitMask & 3 ) == 0 );
							ASSERT ( ( dd_sd2.ddpfPixelFormat.dwGBitMask & 3 ) == 0 );
							ASSERT ( ( dd_sd2.ddpfPixelFormat.dwBBitMask & 0xc000 ) == 0 );
							wR  = (wSrc1 & (UWORD)dd_sd2.ddpfPixelFormat.dwRBitMask) >> 2;
							wR += (wSrc2 & (UWORD)dd_sd2.ddpfPixelFormat.dwRBitMask) >> 2;
							wR += (wSrc3 & (UWORD)dd_sd2.ddpfPixelFormat.dwRBitMask) >> 2;
							wR += (wSrc4 & (UWORD)dd_sd2.ddpfPixelFormat.dwRBitMask) >> 2;
							// Add half an LSB.
							wR += ( (UWORD)dd_sd2.ddpfPixelFormat.dwRBitMask >> 1) & ~(UWORD)dd_sd2.ddpfPixelFormat.dwRBitMask;
							wR &= dd_sd2.ddpfPixelFormat.dwRBitMask;
							wG  = (wSrc1 & (UWORD)dd_sd2.ddpfPixelFormat.dwGBitMask) >> 2;
							wG += (wSrc2 & (UWORD)dd_sd2.ddpfPixelFormat.dwGBitMask) >> 2;
							wG += (wSrc3 & (UWORD)dd_sd2.ddpfPixelFormat.dwGBitMask) >> 2;
							wG += (wSrc4 & (UWORD)dd_sd2.ddpfPixelFormat.dwGBitMask) >> 2;
							wG += ( (UWORD)dd_sd2.ddpfPixelFormat.dwGBitMask >> 1) & ~(UWORD)dd_sd2.ddpfPixelFormat.dwGBitMask;
							wG &= dd_sd2.ddpfPixelFormat.dwGBitMask;
							wB  = (wSrc1 & (UWORD)dd_sd2.ddpfPixelFormat.dwBBitMask);
							wB += (wSrc2 & (UWORD)dd_sd2.ddpfPixelFormat.dwBBitMask);
							wB += (wSrc3 & (UWORD)dd_sd2.ddpfPixelFormat.dwBBitMask);
							wB += (wSrc4 & (UWORD)dd_sd2.ddpfPixelFormat.dwBBitMask);
							wR += ( (UWORD)dd_sd2.ddpfPixelFormat.dwBBitMask << 1) & ~( (UWORD)dd_sd2.ddpfPixelFormat.dwBBitMask << 2 );
							wB >>= 2;
							wB &= dd_sd2.ddpfPixelFormat.dwBBitMask;
							wA  = (wSrc1 & (UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask) >> 2;
							wA += (wSrc2 & (UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask) >> 2;
							wA += (wSrc3 & (UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask) >> 2;
							wA += (wSrc4 & (UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask) >> 2;
							wA += ( (UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask >> 1) & ~(UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask;
							wA &= (UWORD)dd_sd2.ddpfPixelFormat.dwRGBAlphaBitMask;
							*pwdst1++ = wR | wG | wB | wA;
						}
						// Skip a source line.
						pwsrc1 += dwSize * 2;
						pwsrc2 += dwSize * 2;
					}
				}
				else
				{
					// Copy from the source.
					memcpy ( pwdst[0], dd_sd.lpSurface, 2 * dwSize * dwSize );
				}
				dwSize >>= 1;
			}
	#ifdef MIPMAP_PLEASE_BOB
			ASSERT ( dwSize == 1 );
	#endif


			dwSize = ti.height;
			//UWORD pwTemp[512*512];
			UWORD *pwTemp = (UWORD *)MemAlloc ( 512 * 512 * 2);
			ASSERT ( pwTemp != NULL );
			for ( i = 0; i < iNumMipmapLevels; i++ )
			{
				// Swizzle and pack into 2x2 blocks.
				UWORD *pwsrc = (UWORD *) pwdst[i];
				UWORD *pwdst1 = (UWORD *) pwTemp;
				for ( int iY = 0; iY < (signed)dwSize; iY++ )
				{
					for ( int iX = 0; iX < (signed)dwSize; iX++ )
					{
						int iOffset = 0;
						iOffset |= ( iY << 0 ) & 0x00001;
						iOffset |= ( iX << 1 ) & 0x00002;
						iOffset |= ( iY << 1 ) & 0x00004;
						iOffset |= ( iX << 2 ) & 0x00008;
						iOffset |= ( iY << 2 ) & 0x00010;
						iOffset |= ( iX << 3 ) & 0x00020;
						iOffset |= ( iY << 3 ) & 0x00040;
						iOffset |= ( iX << 4 ) & 0x00080;
						iOffset |= ( iY << 4 ) & 0x00100;
						iOffset |= ( iX << 5 ) & 0x00200;
						iOffset |= ( iY << 5 ) & 0x00400;
						iOffset |= ( iX << 6 ) & 0x00800;
						iOffset |= ( iY << 6 ) & 0x01000;
						iOffset |= ( iX << 7 ) & 0x02000;
						iOffset |= ( iY << 7 ) & 0x04000;
						iOffset |= ( iX << 8 ) & 0x08000;
						iOffset |= ( iY << 8 ) & 0x10000;
						iOffset |= ( iX << 9 ) & 0x20000;
						pwdst1[iOffset] = *pwsrc++;
					}
				}
				// And copy it back in.
				memcpy ( pwdst[i], pwTemp, 2 * dwSize * dwSize );
				dwSize >>= 1;
			}
			MemFree ( pwTemp );

			//memset ( &(pwdst[ti.height * ti.width / 2]), 0, ti.width * ti.height );


			ULONG *pdwCodeBook1 = (ULONG *)MemAlloc ( 256 * 4 );
			ASSERT ( pdwCodeBook1 != NULL );
			ULONG *pdwCodeBook2 = (ULONG *)MemAlloc ( 256 * 4 );
			ASSERT ( pdwCodeBook2 != NULL );
			//ULONG pdwCodeBook1[256];
			//ULONG pdwCodeBook2[256];
			int iNumEntries = 0;



			// Let's do this another way. Scan multiple times, with smaller and smaller tolerance levels
			// until we run out of codebook space.
			for ( int iPrecision = 0; iPrecision < 10; iPrecision++ )
			{
				ULONG dwMask1, dwMask2;
	#define BINARY_DAMMIT(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)((a<<15)|(b<<14)|(c<<13)|(d<<12)|(e<<11)|(f<<10)|(g<<9)|(h<<8)|(i<<7)|(j<<6)|(k<<5)|(l<<4)|(m<<3)|(n<<2)|(o<<1)|(p<<0))
				if ( ContainsAlpha )
				{
					// RRRRGGGGBBBBAAAA
					switch ( iPrecision )
					{
					case 0: dwMask1 = BINARY_DAMMIT(1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0); break;
					case 1: dwMask1 = BINARY_DAMMIT(1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0); break;
					case 2: dwMask1 = BINARY_DAMMIT(1,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0); break;
					case 3: dwMask1 = BINARY_DAMMIT(1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0); break;
					case 4: dwMask1 = BINARY_DAMMIT(1,1,0,0,1,1,1,0,1,1,0,0,1,1,0,0); break;
					case 5: dwMask1 = BINARY_DAMMIT(1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0); break;
					case 6: dwMask1 = BINARY_DAMMIT(1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0); break;
					case 7: dwMask1 = BINARY_DAMMIT(1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,0); break;
					case 8: dwMask1 = BINARY_DAMMIT(1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0); break;
					case 9: dwMask1 = BINARY_DAMMIT(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1); break;
					}
				}
				else
				{
					// RRRRRGGGGGGBBBBB
					switch ( iPrecision )
					{
					case 0: dwMask1 = BINARY_DAMMIT(1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0); break;
					case 1: dwMask1 = BINARY_DAMMIT(1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0); break;
					case 2: dwMask1 = BINARY_DAMMIT(1,1,0,0,0,1,1,0,0,0,0,1,1,0,0,0); break;
					case 3: dwMask1 = BINARY_DAMMIT(1,1,0,0,0,1,1,1,0,0,0,1,1,0,0,0); break;
					case 4: dwMask1 = BINARY_DAMMIT(1,1,1,0,0,1,1,1,0,0,0,1,1,1,0,0); break;
					case 5: dwMask1 = BINARY_DAMMIT(1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,0); break;
					case 6: dwMask1 = BINARY_DAMMIT(1,1,1,1,0,1,1,1,1,0,0,1,1,1,1,0); break;
					case 7: dwMask1 = BINARY_DAMMIT(1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0); break;
					case 8: dwMask1 = BINARY_DAMMIT(1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0); break;
					case 9: dwMask1 = BINARY_DAMMIT(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1); break;
					}
				}
				// Replicate this texel mask into the other four.
				dwMask1 |= dwMask1 << 16;
				dwMask2 = dwMask1;
	#undef BINARY_DAMMIT


				// Now scan the texture, making any codebook entries that don't exist.
				// FIXME - at the moment, the mipmap levels are not scanned - maybe I should.
				ULONG *pdwsrc = (ULONG *)pwdst[0];
				int iChunk;
				for ( iChunk = 0; iChunk < (ti.width * ti.height) / 4; iChunk++ )
				{
					ULONG dwSrc1 = pdwsrc[iChunk*2];
					ULONG dwSrc2 = pdwsrc[iChunk*2+1];
					ULONG dwSrc1Mask = dwSrc1 & dwMask1;
					ULONG dwSrc2Mask = dwSrc2 & dwMask2;
					// See if this chunk or something close is already in the codebook.
					for ( int j = 0; j < iNumEntries; j++ )
					{
						if ( ( ( pdwCodeBook1[j] & dwMask1 ) == dwSrc1Mask ) && ( ( pdwCodeBook2[j] & dwMask2 ) == dwSrc2Mask ) )
						{
							// Joy.
							break;
						}
					}
					if ( j == iNumEntries )
					{
						// Didn't find a match.
						if ( iNumEntries < 256 )
						{
							// Cool - just grow the table.
							pdwCodeBook1[iNumEntries] = dwSrc1;
							pdwCodeBook2[iNumEntries] = dwSrc2;
							iNumEntries++;
						}
						else
						{
							// No more entries - exit.
							goto no_more_entries;
						}
					}
				}

			}
	no_more_entries:

			// OK, filled the codebook, so rescan and find the closest matches for each texel,
			// this time using a decent geometric error, not masks!

			// Actually, do a loop of (a) find the closest entries, then (b) make the entries the average of
			// the things that use them.
			// On the last time round, all the mipmap levels do (a), and (b) is not done.

			for ( int iNumTimesRoundTheLoop = 1; iNumTimesRoundTheLoop >= 0; iNumTimesRoundTheLoop-- )
			{
				// Refind the closest match for each chunk.
				// FIXME - this only does the top mipmap level.
				dwSize = ti.height;
				int iLevelsThisTime;
				if ( iNumTimesRoundTheLoop == 0 )
				{
					// Last time round the loop, do all mipmap levels.
					iLevelsThisTime = iNumMipmapLevels;
				}
				else
				{
					// Just do the top one.
					iLevelsThisTime = 1;
				}
				for ( iMipmapLevel = 0; iMipmapLevel < iLevelsThisTime; iMipmapLevel++ )
				{
					ULONG *pdwsrc = (ULONG *)pwdst[iMipmapLevel];
					UCHAR *pcdst = pcTexels[iMipmapLevel];
					for ( int iChunk = 0; iChunk < (signed)(dwSize * dwSize) / 4; iChunk++ )
					{
						ULONG dwSrc1 = pdwsrc[iChunk*2];
						ULONG dwSrc2 = pdwsrc[iChunk*2+1];

						int iClosest = -1;
						int iClosestVal = 100000000;
						if ( ContainsAlpha )
						{
							for ( i = 0; i < iNumEntries; i++ )
							{
								if ( ( pdwCodeBook1[i] == dwSrc1 ) && ( pdwCodeBook2[i] == dwSrc2 ) )
								{
									// Perfect match.
									iClosest = i;
									break;
								}

								// RRRR GGGG BBBB AAAA
								int iVal, iVal1, iVal2, iVal3, iVal4;
								iVal1 = ( ( pdwCodeBook1[i] >> 24 ) & 0xf0 ) - ( ( dwSrc1 >> 24 ) & 0xf0 );
								iVal2 = ( ( pdwCodeBook1[i] >> 20 ) & 0xf0 ) - ( ( dwSrc1 >> 20 ) & 0xf0 );
								iVal3 = ( ( pdwCodeBook1[i] >> 16 ) & 0xf0 ) - ( ( dwSrc1 >> 16 ) & 0xf0 );
								iVal4 = ( ( pdwCodeBook1[i] >> 12 ) & 0xf0 ) - ( ( dwSrc1 >> 12 ) & 0xf0 );
								iVal  = ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
								iVal1 = ( ( pdwCodeBook1[i] >>  8 ) & 0xf0 ) - ( ( dwSrc1 >>  8 ) & 0xf0 );
								iVal2 = ( ( pdwCodeBook1[i] >>  4 ) & 0xf0 ) - ( ( dwSrc1 >>  4 ) & 0xf0 );
								iVal3 = ( ( pdwCodeBook1[i] >>  0 ) & 0xf0 ) - ( ( dwSrc1 >>  0 ) & 0xf0 );
								iVal4 = ( ( pdwCodeBook1[i] <<  4 ) & 0xf0 ) - ( ( dwSrc1 <<  4 ) & 0xf0 );
								iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
								iVal1 = ( ( pdwCodeBook2[i] >> 24 ) & 0xf0 ) - ( ( dwSrc2 >> 24 ) & 0xf0 );
								iVal2 = ( ( pdwCodeBook2[i] >> 20 ) & 0xf0 ) - ( ( dwSrc2 >> 20 ) & 0xf0 );
								iVal3 = ( ( pdwCodeBook2[i] >> 16 ) & 0xf0 ) - ( ( dwSrc2 >> 16 ) & 0xf0 );
								iVal4 = ( ( pdwCodeBook2[i] >> 12 ) & 0xf0 ) - ( ( dwSrc2 >> 12 ) & 0xf0 );
								iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
								iVal1 = ( ( pdwCodeBook2[i] >>  8 ) & 0xf0 ) - ( ( dwSrc2 >>  8 ) & 0xf0 );
								iVal2 = ( ( pdwCodeBook2[i] >>  4 ) & 0xf0 ) - ( ( dwSrc2 >>  4 ) & 0xf0 );
								iVal3 = ( ( pdwCodeBook2[i] >>  0 ) & 0xf0 ) - ( ( dwSrc2 >>  0 ) & 0xf0 );
								iVal4 = ( ( pdwCodeBook2[i] <<  4 ) & 0xf0 ) - ( ( dwSrc2 <<  4 ) & 0xf0 );
								iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 ) + ( iVal4 * iVal4 );
								if ( iVal < iClosestVal )
								{
									iClosestVal = iVal;
									iClosest = i;
								}
							}
							ASSERT ( iClosest != -1 );
							*pcdst++ = (UBYTE)iClosest;
						}
						else
						{
							for ( i = 0; i < iNumEntries; i++ )
							{
								if ( ( pdwCodeBook1[i] == dwSrc1 ) && ( pdwCodeBook2[i] == dwSrc2 ) )
								{
									// Perfect match.
									iClosest = i;
									break;
								}

								// RRRR RGGG GGGB BBBB
								int iVal, iVal1, iVal2, iVal3;
								iVal1 = ( ( pdwCodeBook1[i] >> 24 ) & 0xf8 ) - ( ( dwSrc1 >> 24 ) & 0xf8 );
								iVal2 = ( ( pdwCodeBook1[i] >> 19 ) & 0xfc ) - ( ( dwSrc1 >> 19 ) & 0xfc );
								iVal3 = ( ( pdwCodeBook1[i] >> 13 ) & 0xf8 ) - ( ( dwSrc1 >> 13 ) & 0xf8 );
								iVal  = ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 );
								iVal1 = ( ( pdwCodeBook1[i] >>  8 ) & 0xf8 ) - ( ( dwSrc1 >>  8 ) & 0xf8 );
								iVal2 = ( ( pdwCodeBook1[i] >>  3 ) & 0xfc ) - ( ( dwSrc1 >>  3 ) & 0xfc );
								iVal3 = ( ( pdwCodeBook1[i] <<  3 ) & 0xf8 ) - ( ( dwSrc1 <<  3 ) & 0xf8 );
								iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 );
								iVal1 = ( ( pdwCodeBook2[i] >> 24 ) & 0xf8 ) - ( ( dwSrc2 >> 24 ) & 0xf8 );
								iVal2 = ( ( pdwCodeBook2[i] >> 19 ) & 0xfc ) - ( ( dwSrc2 >> 19 ) & 0xfc );
								iVal3 = ( ( pdwCodeBook2[i] >> 13 ) & 0xf8 ) - ( ( dwSrc2 >> 13 ) & 0xf8 );
								iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 );
								iVal1 = ( ( pdwCodeBook2[i] >>  8 ) & 0xf8 ) - ( ( dwSrc2 >>  8 ) & 0xf8 );
								iVal2 = ( ( pdwCodeBook2[i] >>  3 ) & 0xfc ) - ( ( dwSrc2 >>  3 ) & 0xfc );
								iVal3 = ( ( pdwCodeBook2[i] <<  3 ) & 0xf8 ) - ( ( dwSrc2 <<  3 ) & 0xf8 );
								iVal += ( iVal1 * iVal1 ) + ( iVal2 * iVal2 ) + ( iVal3 * iVal3 );
								if ( iVal < iClosestVal )
								{
									iClosestVal = iVal;
									iClosest = i;
								}
							}
							ASSERT ( iClosest != -1 );
							*pcdst++ = (UBYTE)iClosest;
						}
					}
					dwSize >>= 1;
				}

				//if ( iNumTimesRoundTheLoop > 0 )
				{

					// Now make the entries be the average of the texels that use them.
					// FIXME - only looks at the top mipmap level.
					dwSize = ti.height;
					ULONG *pdwsrc = (ULONG *)pwdst[0];
					UCHAR *pcdst = pcTexels[0];

					for ( i = 0; i < iNumEntries; i++ )
					{
						float fA0 = 0.0f;
						float fR0 = 0.0f;
						float fG0 = 0.0f;
						float fB0 = 0.0f;
						float fA1 = 0.0f;
						float fR1 = 0.0f;
						float fG1 = 0.0f;
						float fB1 = 0.0f;
						float fA2 = 0.0f;
						float fR2 = 0.0f;
						float fG2 = 0.0f;
						float fB2 = 0.0f;
						float fA3 = 0.0f;
						float fR3 = 0.0f;
						float fG3 = 0.0f;
						float fB3 = 0.0f;
						int iNumTexels = 0;

						for ( int iChunk = 0; iChunk < (signed)(dwSize * dwSize) / 4; iChunk++ )
						{
							if ( pcdst[iChunk] == i )
							{
								ULONG dwSrc1 = pdwsrc[iChunk*2];
								ULONG dwSrc2 = pdwsrc[iChunk*2+1];
								iNumTexels++;

								if ( ContainsAlpha )
								{
									fA0 += (float)( ( dwSrc1 >> 28 ) & 0xf );
									fR0 += (float)( ( dwSrc1 >> 24 ) & 0xf );
									fG0 += (float)( ( dwSrc1 >> 20 ) & 0xf );
									fB0 += (float)( ( dwSrc1 >> 16 ) & 0xf );
									fA1 += (float)( ( dwSrc1 >> 12 ) & 0xf );
									fR1 += (float)( ( dwSrc1 >>  8 ) & 0xf );
									fG1 += (float)( ( dwSrc1 >>  4 ) & 0xf );
									fB1 += (float)( ( dwSrc1 >>  0 ) & 0xf );
									fA2 += (float)( ( dwSrc2 >> 28 ) & 0xf );
									fR2 += (float)( ( dwSrc2 >> 24 ) & 0xf );
									fG2 += (float)( ( dwSrc2 >> 20 ) & 0xf );
									fB2 += (float)( ( dwSrc2 >> 16 ) & 0xf );
									fA3 += (float)( ( dwSrc2 >> 12 ) & 0xf );
									fR3 += (float)( ( dwSrc2 >>  8 ) & 0xf );
									fG3 += (float)( ( dwSrc2 >>  4 ) & 0xf );
									fB3 += (float)( ( dwSrc2 >>  0 ) & 0xf );
								}
								else
								{
									fR0 += (float)( ( dwSrc1 >> 27 ) & 0x1f );
									fG0 += (float)( ( dwSrc1 >> 21 ) & 0x3f );
									fB0 += (float)( ( dwSrc1 >> 16 ) & 0x1f );
									fR1 += (float)( ( dwSrc1 >> 11 ) & 0x1f );
									fG1 += (float)( ( dwSrc1 >>  5 ) & 0x3f );
									fB1 += (float)( ( dwSrc1 >>  0 ) & 0x1f );
									fR2 += (float)( ( dwSrc2 >> 27 ) & 0x1f );
									fG2 += (float)( ( dwSrc2 >> 21 ) & 0x3f );
									fB2 += (float)( ( dwSrc2 >> 16 ) & 0x1f );
									fR3 += (float)( ( dwSrc2 >> 11 ) & 0x1f );
									fG3 += (float)( ( dwSrc2 >>  5 ) & 0x3f );
									fB3 += (float)( ( dwSrc2 >>  0 ) & 0x1f );
								}
							}
						}

						float fTemp = 1.0f / (float)iNumTexels;
						fA0 *= fTemp;
						fR0 *= fTemp;
						fG0 *= fTemp;
						fB0 *= fTemp;
						fA1 *= fTemp;
						fR1 *= fTemp;
						fG1 *= fTemp;
						fB1 *= fTemp;
						fA2 *= fTemp;
						fR2 *= fTemp;
						fG2 *= fTemp;
						fB2 *= fTemp;
						fA3 *= fTemp;
						fR3 *= fTemp;
						fG3 *= fTemp;
						fB3 *= fTemp;

						if ( ContainsAlpha )
						{
							pdwCodeBook1[i]  = ( (int)fA0 << 28 ) | ( (int)fR0 << 24 ) | ( (int)fG0 << 20 ) | ( (int)fB0 << 16 );
							pdwCodeBook1[i] |= ( (int)fA1 << 12 ) | ( (int)fR1 <<  8 ) | ( (int)fG1 <<  4 ) | ( (int)fB1 <<  0 );
							pdwCodeBook2[i]  = ( (int)fA2 << 28 ) | ( (int)fR2 << 24 ) | ( (int)fG2 << 20 ) | ( (int)fB2 << 16 );
							pdwCodeBook2[i] |= ( (int)fA3 << 12 ) | ( (int)fR3 <<  8 ) | ( (int)fG3 <<  4 ) | ( (int)fB3 <<  0 );
						}
						else
						{
							pdwCodeBook1[i]  = ( (int)fR0 << 27 ) | ( (int)fG0 << 21 ) | ( (int)fB0 << 16 );
							pdwCodeBook1[i] |= ( (int)fR1 << 11 ) | ( (int)fG1 <<  5 ) | ( (int)fB1 <<  0 );
							pdwCodeBook2[i]  = ( (int)fR2 << 27 ) | ( (int)fG2 << 21 ) | ( (int)fB2 << 16 );
							pdwCodeBook2[i] |= ( (int)fR3 << 11 ) | ( (int)fG3 <<  5 ) | ( (int)fB3 <<  0 );
						}
					}
				}
			}


			dd_sd2.dwSize = sizeof(dd_sd);
			dd_sd2.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;
			HRESULT	res = lp_Surface->Lock(NULL, &dd_sd2, 0, NULL);
			ASSERT(SUCCEEDED(res));

			// Copy the codebook into the surface.
			ULONG *pdwdst = (ULONG *)dd_sd2.lpSurface;
			for ( i = 0; i < 256; i++ )
			{
				*pdwdst++ = pdwCodeBook1[i];
				*pdwdst++ = pdwCodeBook2[i];
			}


	#ifdef MIPMAP_PLEASE_BOB

			// OK, the mipmaps are stored in the top level surface (no GetAttachedSurface or any of that
			// rubbish), and stored smallest-first.

			// Now fill in the mipmaps - they use the same codebook.
			UCHAR *pPtr = (UCHAR *) dd_sd2.lpSurface + 2048;
			// Special-case the first level. It's only a single byte, but writes to
			// vidmem must be WORD-aligned.
			*(WORD *)pPtr = (WORD)pcTexels[iNumMipmapLevels - 1][0];
			pPtr += 2;
			// Now do the rest.
			dwSize = 4;
			for ( iMipmapLevel = iNumMipmapLevels - 2; iMipmapLevel >= 0; iMipmapLevel-- )
			{



				WORD *pTemp = (WORD *)pcTexels[iMipmapLevel];
				for ( int i = (dwSize * dwSize) / 4; i > 0; i -= 2 )
				{
					*(WORD*)pPtr = *pTemp;
					pTemp++;
					pPtr+=2;
				}

				//memcpy ( pPtr, pcTexels[iMipmapLevel], (dwSize * dwSize) / 4 );
				//memset ( pPtr, iMipmapLevel + 2, dwSize * dwSize / 4 );
				//pPtr += dwSize * dwSize / 4;

				//VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));

				dwSize <<= 1;
			}

	#else

			// And copy the data in. This MUST be done this way, because
			// byte writes to videomemory do not work! They get interpreted
			// as word writes, with random junk in the other byte!
			UCHAR *pPtr = (UCHAR *) dd_sd2.lpSurface + 2048;
			WORD *pTemp = (WORD *)pcTexels[0];
			for ( i = (ti.width * ti.height) / 4; i > 0; i -= 2 )
			{
				*(WORD*)pPtr = *pTemp;
				pTemp++;
				pPtr+=2;
			}
	#endif
			MemFree ( pdwCodeBook2 );
			MemFree ( pdwCodeBook1 );
			MemFree ( pcpwdstBlock );
			MemFree ( pcRealTexelBlock );
			MemFree ( dd_sd.lpSurface );

			VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));
		}

	}
	#endif

	MemFree(tga);


	return	DD_OK;
}




#ifndef TARGET_DC

HRESULT	D3DTexture::Reload_8(void)
{
	UBYTE					temp_palette[768],
							*surface_mem;
	SLONG					bytes_read,
							c0,
							pitch;
	D3DDeviceInfo			*current_device;
	DDModeInfo				*the_format,*next_best_format;
	DDSURFACEDESC2			dd_sd;
	HANDLE					file_handle;
	HANDLE					t_handle;
	HANDLE					p_handle;
	HRESULT					result;
	PALETTEENTRY			the_palette[256];
	SLONG                   score;
	SLONG					best_score;




	// Check parameters.
	if(strlen(texture_name)<=0 || strlen(texture_name)<=0)
	{
		// Invalid parameters.
		return	DDERR_GENERIC;
	}

	// Get the current device.
	current_device	=	the_display.GetDeviceInfo();
	if(!current_device)
	{
		// No current device.
		return	DDERR_GENERIC;
	}

	//
	// There is never any alpha in an 8-bit palettized texture.
	//

	ContainsAlpha = FALSE;

	// ====================================================
	// MARKS HACKED-IN CODE!
	// ====================================================


	//
	// Find an 8-bit palettized texture format.
	//

	DDModeInfo *mi;
	
	the_format = NULL;

	for (mi = current_device->FormatList; mi; mi = mi->Next)
	{
		if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		{
			the_format = mi;
			
			//
			// Use the old code to load the 8-bit textures.
			//

			goto load_8bit;
		}
	}

	//
	// Look for a suitable 16-bit format.
	//
	
	best_score = 0;
	the_format = NULL;

	for (mi = current_device->FormatList; mi; mi = mi->Next)
	{
		if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_RGB)
		{
			//
			// True colour...
			//

			if (mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount == 16)
			{
				//
				// And 16-bit.
				//

				score = 5;
								
				if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
				{
					//
					// Knock off a bit for alpha
					//
				
					score -= 1;
				}
				
				if (score > best_score)
				{
					best_score = score;
					the_format = mi;
				}
			}
		}
	}

	if (the_format == NULL)
	{
		//
		// We really are in the shit now! There aren't any texture
		// formats for us to choose from.
		//

		return DDERR_GENERIC;
	}

	//
	// Open the texture and palette files.
	//

	t_handle = CreateFile(
					texture_name,
					(GENERIC_READ|GENERIC_WRITE),
					(FILE_SHARE_READ|FILE_SHARE_WRITE),
					NULL,
					OPEN_EXISTING,
					0,
					NULL);

	if (t_handle == NULL)
	{
		DebugText("Could not open texture file.\n");
		return DDERR_GENERIC;
	}

	p_handle = CreateFile(
					palette_name,
					(GENERIC_READ|GENERIC_WRITE),
					(FILE_SHARE_READ|FILE_SHARE_WRITE),
					NULL,
					OPEN_EXISTING,
					0,
					NULL);

	if (p_handle == NULL)
	{
		DebugText("Could not open palette file.\n");
		return DDERR_GENERIC;
	}

	//
	// We have to convert the texture into the given format.
	//

	OS_calculate_mask_and_shift(the_format->ddSurfDesc.ddpfPixelFormat.dwRBitMask ,&mask_red,   &shift_red  );
	OS_calculate_mask_and_shift(the_format->ddSurfDesc.ddpfPixelFormat.dwGBitMask ,&mask_green, &shift_green);
	OS_calculate_mask_and_shift(the_format->ddSurfDesc.ddpfPixelFormat.dwBBitMask ,&mask_blue,  &shift_blue );
	//
	// Get rid of the old texture stuff.
	//
	
	Destroy();

	//
	// Create
	//

	dd_sd = the_format->ddSurfDesc;

	dd_sd.dwSize  = sizeof(dd_sd);
	dd_sd.dwFlags =
		DDSD_CAPS   |
		DDSD_HEIGHT |
		DDSD_WIDTH  |
		DDSD_PIXELFORMAT;
	dd_sd.dwWidth  = 256;
	dd_sd.dwHeight = 256;
	dd_sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
#ifdef TARGET_DC
	dd_sd.ddsCaps.dwCaps2 = 0;
#else
	dd_sd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#endif

	VERIFY(SUCCEEDED(the_display.lp_DD4->CreateSurface(&dd_sd, &lp_Surface, NULL)));

	//
	// Lock
	//

	VERIFY(SUCCEEDED(lp_Surface->Lock(NULL, &dd_sd, 0, NULL)));

	//
	// Load in the palette.
	//

	ReadFile(p_handle, temp_palette, 768, (LPDWORD) &bytes_read, NULL);

	//
	// Copy the texture in
	//

	{
		UWORD pixel_our;
		UBYTE colour;
		UBYTE red;
		UBYTE green;
		UBYTE blue;
		UWORD *wscreen = (UWORD *) dd_sd.lpSurface;
		UBYTE  line[256];
		SLONG  i;
		SLONG  j;

		//
		// 16 bits per pixel.
		//

		for (i = 0; i < 256; i++)
		{
			ReadFile(t_handle, line, 256, (LPDWORD) &bytes_read, NULL);

			for (j = 0; j < 256; j++)
			{
				colour = line[j];

				red   = temp_palette[colour * 3 + 0];
				green = temp_palette[colour * 3 + 1];
				blue  = temp_palette[colour * 3 + 2];

				pixel_our = 0;
				
				pixel_our |= (red   >> mask_red  ) << shift_red;
				pixel_our |= (green >> mask_green) << shift_green;
				pixel_our |= (blue  >> mask_blue ) << shift_blue;

				wscreen[i * 256 + j] = pixel_our;
			}
		}
	}

	//
	// Close the files.
	//

	CloseHandle(t_handle);
	CloseHandle(p_handle);

	//
	// Unlock
	//

   	VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));

	// Get d3d texture interface.
	result	=	lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture);

	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to create texture\n");
		dd_error(result);
		goto	cleanup;
	}

	//
	// Success.
	//

	return	DD_OK;




	// ====================================================
	// END OF MARKS HACKED-IN CODE
	// ====================================================


	//
	// Found an 8-bit palettized texture format.
	//

  load_8bit:;


	// Get rid of the old texture stuff.
	Destroy();

	//
	// Set up the texture.
	//

	// Set up the surface description.
	dd_sd			=	the_format->ddSurfDesc;
	dd_sd.dwFlags	=	DDSD_CAPS 	|
						DDSD_HEIGHT |
						DDSD_WIDTH 	|
						DDSD_PIXELFORMAT;
	dd_sd.dwWidth	=	256;
	dd_sd.dwHeight	=	256;
	dd_sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
#ifdef TARGET_DC
	dd_sd.ddsCaps.dwCaps2 = 0;
#else
	dd_sd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#endif

	// Create the main texture surface.
	result	=	the_display.lp_DD4->CreateSurface	(
														&dd_sd,
														&lp_Surface,
														NULL
													);
	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to create main texture surface\n");
		dd_error(result);
		goto	cleanup;
	}

	// Read in the bit map.
	file_handle	=	CreateFile	(
									texture_name,
									(GENERIC_READ|GENERIC_WRITE),
									(FILE_SHARE_READ|FILE_SHARE_WRITE),
									NULL,
									OPEN_EXISTING,
									0,
									NULL
    	                   		);
	if(file_handle == NULL)
	{
		DebugText("LoadTexture8: unable to open palette file\n");
		goto	cleanup;
	}

	// Read the bit map directly into the texture surface.
	result	=	lp_Surface->Lock(NULL,&dd_sd,DDLOCK_WAIT|DDLOCK_NOSYSLOCK,NULL);
	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to lock temp texture surface\n");
		dd_error(result);
		goto	cleanup;
	}
	pitch		=	dd_sd.lPitch;
	surface_mem	=	(UBYTE*)dd_sd.lpSurface;
	for(c0=0;c0<256;c0++,surface_mem+=pitch)
	{
		if(ReadFile(file_handle,surface_mem,256,(LPDWORD)&bytes_read,NULL)==FALSE)
		{
			DebugText("LoadTexture8: unable to read texture file\n");
			goto	cleanup;
		}
	}
	lp_Surface->Unlock(NULL);
	CloseHandle(file_handle);

	//
	// Set up the palette.
	//

	// Read the palette into temp storage.
	file_handle	=	CreateFile	(
									palette_name,
									(GENERIC_READ|GENERIC_WRITE),
									(FILE_SHARE_READ|FILE_SHARE_WRITE),
									NULL,
									OPEN_EXISTING,
									0,
									NULL
    	                   		);
	if (file_handle == NULL)
	{
		DebugText("LoadTexture8: unable to open palette file\n");
		goto	cleanup;
	}
	if(ReadFile(file_handle,temp_palette,768,(LPDWORD)&bytes_read,NULL)==FALSE)
	{
		DebugText("LoadTexture8: unable to read palette file\n");
		goto	cleanup;
	}
	CloseHandle(file_handle);

	// Set up palette entries.
	ZeroMemory(the_palette,sizeof(the_palette));
	for(c0=0;c0<256;c0++)
	{
		the_palette[c0].peRed	=	temp_palette[(c0*3)+0];
		the_palette[c0].peGreen	=	temp_palette[(c0*3)+1];
		the_palette[c0].peBlue	=	temp_palette[(c0*3)+2];
	}

    // Create the texture palette.
	result	=	the_display.lp_DD4->CreatePalette	(
														DDPCAPS_8BIT|DDPCAPS_ALLOW256,
														the_palette,
														&lp_Palette,
														NULL
													);
	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to create texture palette\n");
		dd_error(result);
		goto	cleanup;
	}

	// Attach palette to texture surface.
	result	=	lp_Surface->SetPalette(lp_Palette);
	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to attach texture palette\n");
		dd_error(result);
//		goto	cleanup;
	}


	// Get d3d texture interface.
	result	=	lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture);
	if(FAILED(result))
	{
		DebugText("LoadTexture8: unable to create texture\n");
		dd_error(result);
		goto	cleanup;
	}

	// Success.
	return	DD_OK;

cleanup:

	// Cleanup.
	Destroy();
	return	result;
}

#endif //#ifndef TARGET_DC



HRESULT D3DTexture::Reload_user()
{
	D3DDeviceInfo *current_device;

	SLONG       score;
	DDModeInfo *mi;
	SLONG       best_score;
	DDModeInfo *best_mi;

	//SLONG bpp;

	SLONG try_shift_alpha;
	SLONG try_shift_red;
	SLONG try_shift_green;
	SLONG try_shift_blue;
		  
	SLONG try_mask_alpha;
	SLONG try_mask_red;
	SLONG try_mask_green;
	SLONG try_mask_blue;

	DDSURFACEDESC2 dd_sd;
	HRESULT        result;

	//
	// Get the current device.
	//

	current_device = the_display.GetDeviceInfo();

	if (!current_device)
	{
		TRACE("No device!\n");

		return DDERR_GENERIC;
	}

	best_score = 0;
	best_mi    = NULL;

	if (UserWantsAlpha)
	{
		//
		// Find the texture format with the most bits of alpha.
		//

		for (mi = current_device->FormatList; mi; mi = mi->Next)
		{
			if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_RGB)
			{
				if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
				{
					if (mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount == 16)
					{
						//
						// Find out how many bits there are for each component.
						//

						OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRGBAlphaBitMask, &try_mask_alpha, &try_shift_alpha);
						OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRBitMask       , &try_mask_red,   &try_shift_red  );
						OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwGBitMask       , &try_mask_green, &try_shift_green);
						OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwBBitMask       , &try_mask_blue,  &try_shift_blue );

						score  = (32 - try_mask_alpha) << 8;
						score /= mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount;

						if (score > best_score)
						{
							best_score = score;
							best_mi    = mi;
						}
					}
				}
			}
		}
	}
	else
	{
		//
		// Find a 5:6:5 or similar format.
		//

		for (mi = current_device->FormatList; mi; mi = mi->Next)
		{
			if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_RGB)
			{
				//
				// True colour...
				//

				if (mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount >= 16)
				{
					score  = 0x100;
					score -= mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount;
									
					if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
					{
						//
						// Knock off score for alpha
						//
					
						score -= 1;
					}
					
					if (score > best_score)
					{
						best_score = score;
						best_mi    = mi;
					}
				}
			}
		}
	}

	if (best_mi == NULL)
	{
		//
		// Couldn't find a suitable texture format.
		//

		TRACE("Could not find texture format for the user texture page\n");
		return DDERR_GENERIC;
	}

	mi = best_mi;

	SLONG dwMaskR, dwMaskG, dwMaskB, dwMaskA;
	SLONG dwShiftR, dwShiftG, dwShiftB, dwShiftA;

	OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRBitMask, &dwMaskR, &dwShiftR );
	OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwGBitMask, &dwMaskG, &dwShiftG );
	OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwBBitMask, &dwMaskB, &dwShiftB );

	if (UserWantsAlpha)
	{
		OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRGBAlphaBitMask, &dwMaskA, &dwShiftA);
	}
	else
	{
		dwMaskA = 0;
		dwShiftA = 0;
	}
	mask_red = (UBYTE)dwMaskR;
	mask_green = (UBYTE)dwMaskG;
	mask_blue = (UBYTE)dwMaskB;
	mask_alpha = (UBYTE)dwMaskA;
	shift_red = (UBYTE)dwShiftR;
	shift_green = (UBYTE)dwShiftG;
	shift_blue = (UBYTE)dwShiftB;
	shift_alpha = (UBYTE)dwShiftA;


	//
	// Get rid of the old texture stuff.
	//

	Destroy();

	//
	// The surface
	//

	dd_sd = mi->ddSurfDesc;

	dd_sd.dwSize  = sizeof(dd_sd);
	dd_sd.dwFlags =
		DDSD_CAPS   |
		DDSD_HEIGHT |
		DDSD_WIDTH  |
		DDSD_PIXELFORMAT;
	dd_sd.dwWidth  = size;
	dd_sd.dwHeight = size;
	dd_sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
#ifdef TARGET_DC
	dd_sd.ddsCaps.dwCaps2 = 0;
#else
	dd_sd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
#endif

	VERIFY(SUCCEEDED(the_display.lp_DD4->CreateSurface(&dd_sd, &lp_Surface, NULL)));

	//
	// Get d3d texture interface.
	//

	result = lp_Surface->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lp_Texture);

	if(FAILED(result))
	{
		TRACE("ReloadTextureUser: Could not get the texture interface.\n");
		Destroy();
		return DDERR_GENERIC;
	}

	//
	// Success.
	//

	return	DD_OK;

}

HRESULT D3DTexture::LockUser(UWORD **bitmap, SLONG *pitch)
{
	DDSURFACEDESC2 dd_sd;

//	ASSERT(Type == D3DTEXTURE_TYPE_USER);

	InitStruct(dd_sd);

	if (lp_Surface == NULL || FAILED(lp_Surface->Lock(NULL, &dd_sd, DDLOCK_WAIT, NULL)))
	{
		return DDERR_GENERIC;
	}
	else
	{
		*bitmap = (UWORD *) dd_sd.lpSurface;
		*pitch  = dd_sd.lPitch;

		return DD_OK;
	}
}

void D3DTexture::UnlockUser()
{
//	ASSERT(Type == D3DTEXTURE_TYPE_USER);

   	VERIFY(SUCCEEDED(lp_Surface->Unlock(NULL)));
}

HRESULT D3DTexture::Reload(void)
{
	Font		*current_font,
				*next_font;
	HRESULT		ans;


	//
	// erk ... we have to call the POLY engine from here
	// this hook needs calling when the textures are reloaded
	// en masse, but tracking down each point in the game where
	// this happens is tricky ...
	// so there's a cheeky little call here ...
	//
	POLY_reset_render_states();

	if(IsFont())
	{
		current_font	=	FontList;
		while(current_font)
		{
			next_font	=	current_font->NextFont;
			MFdelete(current_font);
			current_font	=	next_font;
		}
		FontList	=	NULL;
	}

	if (DontBotherLoadingInSoftwareMode)
	{
		//
		// Hey- no use loading this texture if we ain't gonna use it!
		//

		return D3D_OK;
	}

	//
	// Always fail in software mode!
	//

#if 0
	if (!the_display.CurrDevice->d3dHalDesc.dcmColorModel)
	{
		//
		//  We are in software.
		//

		return DDERR_GENERIC;
	}
#endif

	switch(Type)
	{
		case D3DTEXTURE_TYPE_8:
#ifdef TARGET_DC
			// No support for paletted textures, thanks.
			ASSERT ( FALSE );
#else
			ans = Reload_8();
#endif
			break;

		case D3DTEXTURE_TYPE_TGA:
			ans = Reload_TGA();
			break;

		case D3DTEXTURE_TYPE_USER:
			ans = Reload_user();
			break;

		default:
			ASSERT(0);
			break;
	}

	return ans;
}



//---------------------------------------------------------------

HRESULT	D3DTexture::Destroy(void)
{
	int a = 0, b = 0, c = 0, d = 0;


	Font		*current_font,
				*next_font;


	//	Get rid of fonts.
	if(IsFont())
	{
		current_font	=	FontList;
		while(current_font)
		{
			next_font	=	current_font->NextFont;
			MFdelete(current_font);
			current_font	=	next_font;
		}
		FontList	=	NULL;
	}

	// Release texture.
	if(lp_Texture)
	{
		DebugText("Releasing texture\n");
		a = lp_Texture->Release();
		DebugText("Done\n");
		lp_Texture			=	NULL;
	}

#ifndef TARGET_DC
	// Release palette.
	if(lp_Palette)
	{
		DebugText("Releasing palette\n");
		b = lp_Palette->Release();
		DebugText("Done\n");
		lp_Palette	=	NULL;
	}
#endif

	// Release surface.
	if(lp_Surface)
	{
		DebugText("Releasing surface\n");
		c = lp_Surface->Release();
		DebugText("Done\n");
		lp_Surface	=	NULL;
	}

//#define	I_LIKE_TO_ASSERT	1

#ifdef	I_LIKE_TO_ASSERT
	ASSERT(a == 0 && b == 0 && c == 0 && d == 0);
#endif

//	TextureHandle = NULL;

	return	DD_OK;
}

//---------------------------------------------------------------

#define	MATCH_TGA_PIXELS(p1,p2)		((p1)->red==(p2)->red&&(p1)->green==(p2)->green&&(p1)->blue==(p2)->blue)

BOOL	scan_for_baseline(TGA_Pixel **line_ptr,TGA_Pixel *underline,TGA_Info *info,SLONG *y_ptr)
{
	while(*y_ptr<info->height)
	{
		if(MATCH_TGA_PIXELS(*line_ptr,underline))
		{
			//	Got the baseline so drop to the next line.
			*y_ptr		+=	1;
			*line_ptr	+=	info->width;
			return	TRUE;
		}

		*y_ptr		+=	1;
		*line_ptr	+=	info->width;
	}
	return	FALSE;
}

//---------------------------------------------------------------

HRESULT	D3DTexture::CreateFonts(TGA_Info *tga_info,TGA_Pixel *tga_data)
{
	SLONG		current_char,
				char_x,char_y,
				char_height,char_width,
				tallest_char;
	Font		*the_font;
	TGA_Pixel	underline,
				*current_line,
				*current_pixel;


	//	Scan down the image looking for the underline.
	underline.red	=	0xff;
	underline.green	=	0x00;
	underline.blue	=	0xff;
	current_line	=	tga_data;
	char_y			=	0;
	if(scan_for_baseline(&current_line,&underline,tga_info,&char_y))
	{
map_font:
		//	Found a font baseline so map it.
		the_font		=	MFnew<Font>();
		if(FontList)
		{
			the_font->NextFont	=	FontList;
			FontList			=	the_font;
		}
		else
		{
			the_font->NextFont	=	NULL;
			FontList			=	the_font;
		}

		current_char	=	0;
		char_x			=	0;
		tallest_char	=	1;
		
		while(current_char<93)
		{
			//	Scan across to find the width of char.
			char_width		=	0;
			current_pixel	=	current_line+char_x;
			while(!MATCH_TGA_PIXELS(current_pixel,&underline))
			{
				current_pixel++;
				char_width++;
				
				//	Reached the end of the line.
				if(char_x+char_width>=tga_info->width)
				{
					//	Find the next baseline.
					char_y			+=	tallest_char+1;
					if(char_y>=tga_info->height)
						return	DDERR_GENERIC;
					current_line	=	tga_data+(char_y*tga_info->width);

					if(!scan_for_baseline(&current_line,&underline,tga_info,&char_y))
						return	DDERR_GENERIC;

					char_x			=	0;
					tallest_char	=	1;
					char_width		=	0;
					current_pixel	=	current_line;
				}
			}

			//	Now scan down to find the height of the char
			char_height		=	0;
			current_pixel	=	current_line+char_x;
			while(!MATCH_TGA_PIXELS(current_pixel,&underline))
			{
				current_pixel	+=	tga_info->width;
				char_height++;

				//	Reached the bottom of the page.
				if(char_height>=tga_info->height)
					return	DDERR_GENERIC;
			}

			the_font->CharSet[current_char].X		=	char_x;
			the_font->CharSet[current_char].Y		=	char_y;
			the_font->CharSet[current_char].Width	=	char_width;
			the_font->CharSet[current_char].Height	=	char_height;

			char_x	+=	char_width+1;
			if(tallest_char<char_height)
				tallest_char	=	char_height;

			current_char++;
		}

		//	Find out if there's another font in this file.
		char_y			+=	tallest_char+1;
		if(char_y>=tga_info->height)
			return	DDERR_GENERIC;
		current_line	=	tga_data+(char_y*tga_info->width);

		if(scan_for_baseline(&current_line,&underline,tga_info,&char_y))
			goto	map_font;
	}

	return	DD_OK;
}

//---------------------------------------------------------------

Font	*D3DTexture::GetFont(SLONG id)
{
	Font		*current_font;


	current_font	=	FontList;
	while(id && current_font)
	{
		current_font	=	current_font->NextFont;
		id--;
	}
	return	current_font;
}

//---------------------------------------------------------------


void D3DTexture::set_greyscale(BOOL is_greyscale)
{
	if (is_greyscale != GreyScale)
	{
		GreyScale = is_greyscale;

		if (Type != D3DTEXTURE_TYPE_UNUSED)
		{
			Reload();
		}
	}
}





