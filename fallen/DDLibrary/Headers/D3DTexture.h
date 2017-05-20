// D3DTexture.h
// Guy Simmons, 29th November 1997.

#ifndef	D3DTEXTURE_H
#define	D3DTEXTURE_H

#ifndef TARGET_DC

// PC

// this makes absolutely fuck-all difference to speed (tested)
//#define TEX_EMBED		// must be set the same in PolyPage.h
// Don't page.
#define USE_FANCY_TEXTURE_PAGES_PLEASE_BOB 1
#else

// DREAMCAST

// But it makes the VQ much more efficient, so do it!
#define TEX_EMBED		// must be set the same in PolyPage.h
// And use the new snazzy system.
#define USE_FANCY_TEXTURE_PAGES_PLEASE_BOB 1
#endif


#ifndef TARGET_DC
// Create VQ'd textures and save them out. Only on the PC version, obviously.
//#define SAVE_MY_VQ_TEXTURES_PLEASE_BOB defined
#endif



// Call after doing lots of loading.
void NotGoingToLoadTexturesForAWhileNowSoYouCanCleanUpABit ( void );

// handle = file opened with FileOpen
// dwSize = number of bytes to load.
// return = where the file was loaded.
// You _must_ copy this or do something with it before loading another file.
void *FastLoadFileSomewhere ( MFFileHandle handle, DWORD dwSize );




#include	"tga.h"
#include	"FileClump.h"

//---------------------------------------------------------------

struct	Char
{
	SLONG		X,
				Y,
				Height,
				Width;
};

//---------------------------------------------------------------

struct	Font
{
	SLONG			StartLine;
	Char			CharSet[96];
	Font			*NextFont;
};

//---------------------------------------------------------------

#define	D3D_TEXTURE_FONT		(1<<0)
#define D3D_TEXTURE_FONT2		(1<<1)	// texture is a font with red borders

//---------------------------------------------------------------

#define D3DTEXTURE_TYPE_UNUSED	0
#define D3DTEXTURE_TYPE_8		1
#define D3DTEXTURE_TYPE_TGA		2
#define D3DTEXTURE_TYPE_USER	3		// The user updates the contents- it doesn't have a file.


#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
// First number is size of subtextures, second is number of subtextures in page.
// 3x3 means the page is 4x as big in each direction, with lots of pixel padding.
#define D3DPAGE_NONE			0
#define D3DPAGE_64_3X3			1
#define D3DPAGE_64_4X4			2
#define D3DPAGE_32_3X3			3
#define D3DPAGE_32_4X4			4

#endif



class	D3DTexture
{
	public:

#ifdef TARGET_DC
		CBYTE					texture_name[64];
#else
		CBYTE					palette_name[256];
		CBYTE					texture_name[256];
#endif
		ULONG					ID;		// texture ID for FileClump
		// Allow the texture to be shrunk or replaced with junk for faster loading.
		BOOL					bCanShrink;
		SLONG					TextureFlags;
		Font					*FontList;
		LPDIRECT3DTEXTURE2		lp_Texture;
		LPDIRECTDRAWSURFACE4	lp_Surface;
#ifndef TARGET_DC
		LPDIRECTDRAWPALETTE		lp_Palette;
		HRESULT					Reload_8   (void);
#endif
		HRESULT					Reload_TGA (void);
		HRESULT					Reload_user(void);
		BOOL					DontBotherLoadingInSoftwareMode;
		BOOL					GreyScale;
		BOOL					UserWantsAlpha;	// The user page needs an alpha-channel.
#ifdef TEX_EMBED
#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
		//char *name;			// Texture file name.
		UBYTE bPagePos;			// Position in page.
		UBYTE bPageType;		// One of D3DPAGE_xxx
		WORD wPageNum;			// The D3Dpage this is in.
#else
		UBYTE					TexOffset;		// 128 + (0-15) for the subareas of a texture page else zero
		UBYTE					Padding[3];		// MSVC BUG!!!!
		D3DTexture*				TexSource;		// if TexOffset & 128, it's the source D3DTexture
#endif
#endif

		D3DTexture				*NextTexture;


		D3DTexture()							{
													TextureFlags	=	0;
													FontList		=	NULL;
													NextTexture=NULL;
													#ifndef TARGET_DC
													lp_Palette=NULL;
													#endif
													lp_Surface=NULL;
													lp_Texture=NULL;
													Type = D3DTEXTURE_TYPE_UNUSED;
#ifdef TEX_EMBED
#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
													wPageNum = -1;	// None.
													bPagePos = 0;
													bPageType = D3DPAGE_NONE;
#else
													TexOffset = 0;
													TexSource = this;
#endif
#endif
													// IT WOULD FUCKING HELP IF SOME OF THESE ACTUALLY GOT SET UP WITH DEFAULTS
													DontBotherLoadingInSoftwareMode = FALSE;
													GreyScale = FALSE;
													UserWantsAlpha = FALSE;
													ID = -1;
													bCanShrink = FALSE;
												}

		//
		// The format used.
		// 

#ifdef TARGET_DC
		UBYTE mask_red;
		UBYTE mask_green;
		UBYTE mask_blue;
		UBYTE mask_alpha;

		UBYTE shift_red;
		UBYTE shift_green;
		UBYTE shift_blue;
		UBYTE shift_alpha;
#else
		SLONG mask_red;
		SLONG mask_green;
		SLONG mask_blue;
		SLONG mask_alpha;

		SLONG shift_red;
		SLONG shift_green;
		SLONG shift_blue;
		SLONG shift_alpha;
#endif

		SLONG		Type;
		SLONG       size;			// The size in pixels of the texture page.
		SLONG       ContainsAlpha;	// TRUE or FALSE? Always false for TYPE_8 textures.

#ifndef TARGET_DC
		HRESULT		LoadTexture8  (CBYTE *tex_file,CBYTE *pal_file);
#endif
		HRESULT		LoadTextureTGA(CBYTE *tga_file,ULONG texid,BOOL bCanShrink=TRUE);

		HRESULT		ChangeTextureTGA(CBYTE *tga_file);

		HRESULT		CreateUserPage(SLONG size, BOOL i_want_an_alpha_channel);	// Power of two between 32 and 256 inclusive
		HRESULT     LockUser      (UWORD **bitmap, SLONG *pitch);				// Returns the texture page on success. The pitch is in bytes!
		void        UnlockUser	  (void);

		HRESULT		Reload(void);
		HRESULT		Destroy(void);

		HRESULT		CreateFonts(TGA_Info *tga_info,TGA_Pixel *tga_data);
		Font		*GetFont(SLONG id);

		// resets texture page for loading
		static void	BeginLoading();

		//
		// Makes the texture grey-scale. If it is already loaded, it
		// is re-loaded.
		//

		void set_greyscale(BOOL is_greyscale);

		LPDIRECT3DTEXTURE2		GetD3DTexture()			{ return lp_Texture; }
#ifndef TARGET_DC
		LPDIRECTDRAWPALETTE		GetPalette(void)		{ return lp_Palette; }
#endif
		LPDIRECTDRAWSURFACE4	GetSurface(void)		{ return lp_Surface; }
#ifdef TEX_EMBED
#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
		void					GetTexOffsetAndScale ( float *pfUScale, float *pfUOffset, float *pfVScale, float *pfVOffset );
#else //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

		UBYTE					GetTexOffset()			{ return TexOffset; }
		D3DTexture*				GetTexSource()			{ return TexSource; }
#endif //#else //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
#endif

#ifndef TARGET_DC
		HRESULT					SetColorKey(SLONG flags,LPDDCOLORKEY key)	{
																					if (lp_Surface)
																					{
																						return	lp_Surface->SetColorKey(flags,key);
																					}
																					else
																					{
																						return DDERR_GENERIC;
																					}
																				}
#endif

		inline	BOOL		IsFont(void)				{	return	TextureFlags&D3D_TEXTURE_FONT;	}
		inline	void		FontOn(void)				{	TextureFlags	|=	D3D_TEXTURE_FONT;	}
		inline	void		FontOff(void)				{	TextureFlags	&=	~D3D_TEXTURE_FONT;	}

		inline	BOOL		IsFont2(void)				{	return	TextureFlags&D3D_TEXTURE_FONT2;	}
		inline	void		Font2On(void)				{	TextureFlags	|=	D3D_TEXTURE_FONT2;	}
		inline	void		Font2Off(void)				{	TextureFlags	&=	~D3D_TEXTURE_FONT2;	}
};



#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
class D3DPage
{
public:
	UBYTE		bPageType;			// One of D3DPAGE_xxx
	UBYTE		bNumTextures;		// Number of textures in page.
	char		*pcDirectory;		// The page's directory. THIS IS STATIC - don't free it.
	char		*pcFilename;		// The page's filename. THIS IS STATIC - don't free it.
	char		**ppcTextureList;	// A pointer to an array of pointers to strings of the texture names :-)
	D3DTexture	*pTex;				// The texture page texture itself.
	//D3DTexture	*pTextures[16];		// The textures in this page, in order.

	D3DPage ( void )
	{
		bPageType = 0;
		bNumTextures = 0;
		pcDirectory = NULL;
		pcFilename = NULL;
		pTex = NULL;
		ppcTextureList = NULL;
	}

	// Call this when linking a standard D3DTexture to the page - it will demand-load the page's texture.
	void D3DPage::EnsureLoaded ( void );
	// Call this when unloading everything.
	void D3DPage::Unload ( void );

};
#endif


//---------------------------------------------------------------




#endif


