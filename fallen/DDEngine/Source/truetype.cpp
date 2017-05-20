// truetype.cpp
//
// TrueType font drawing code

// This module handles drawing text to the screen in any given TrueType font
// It draws text to a 1280x64 bitmap, then scales it down and antialiases it into a texture
// A list is maintained of all the text draw calls (with wrapping parameters, etc.) made in the last frame,
// the single lines of text these map to and where they are stored in the texture cache
//
// At the beginning of a frame, all text draw calls are marked Pending
// Every call to DrawTextTT is stored; if it already was in the list, it is marked Active.  If not, it is measured and
// placed into the list.
// At the end of a frame, the process goes:
//
// 1. Remove from the draw list any text no longer required; free its cachelines etc.
// 2. If any drawing is required:
// 2a. Create the DC and set it up
// 2b. Measure the text and break it into lines
// 2c. Draw each line of text onto the shadow screen
// 2d. Close the memory DC and lock the shadow surface
// 2e. Blit from the shadow surface to the texture surfaces
// 2f. Unlock everything
// 3. Draw polygons to blit the text to the screen

#include <DDLib.h>
#include "texture.h"
#ifndef TARGET_DC
#include <mbctype.h>	// MBCS crap
#include <mbstring.h>	// more MBCS crap
#endif

#ifdef TARGET_DC
#include "target.h"
#endif

#include "truetype.h"
#include "polypoint.h"
#include "env.h"

#define ANTIALIAS_BY_HAND		1										// antialias by hand?
#define	AA_SIZE					(ANTIALIAS_BY_HAND ? 2 : 1)				// multiplier
#define	NUM_TT_PAGES			4										// number of 256x256 texture pages to allocate
#define	MIN_TT_HEIGHT			8										// minimum height of a line of text
#define	MAX_TT_HEIGHT			64										// maximum height of a line of text
#define MAX_LINES_PER_PAGE		(256 / MIN_TT_HEIGHT)					// maximum number of cachelines per texture page
#define MAX_CACHELINES			(MAX_LINES_PER_PAGE * NUM_TT_PAGES)		// maximum number of cachelines
#define MAX_TEXTCOMMANDS		32										// maximum number of current & pending text commands
#define TYPEFACE				NULL									// typeface name

static int					FontHeight;						// height of font

static IDirectDrawSurface4*	pShadowSurface;					// drawing surface
static IDirectDrawPalette*	pShadowPalette;					// palette for surface
static HFONT				hFont;							// font for DC
static HFONT				hMidFont;						// 3/4 font
static HFONT				hSmallFont;						// 1/2 font
static HFONT				hOldFont;						// old font for DC

static D3DTexture			Texture[NUM_TT_PAGES];			// texture pages
static CacheLine			Cache[MAX_CACHELINES];			// cache lines
static int					NumCacheLines;					// actual number of cache lines
static UWORD				PixMapping[256];				// maps 8bpp greyscale to texture format

static TextCommand			Commands[MAX_TEXTCOMMANDS];		// text commands

// measure a TextCommand
static void MeasureTextCommand(TextCommand* tcmd);

// run a TextCommand
static void DoTextCommand(TextCommand* tcmd);

// draw and cache a single line of text
static void CreateTextLine(char* string, int nchars, int width, int x, int y, TextCommand* owner);

// allocate a new cache line
static int NewCacheLine();

// copy from 8bpp to texture
static void CopyToCache(CacheLine* cptr, UBYTE* sptr, int spitch, int width);

// blit text across
static void BlitText();

// draw the textures to the screen
static void ShowTextures();

// blit an area of the current texture to the screen
static void TexBlit(int x1, int y1, int x2, int y2, int dx, int dy, ULONG rgb, ULONG scale);

// copy the shadow buffer & textures to the screen
static void ShowDebug();

// TT_Init
//
// create:
//
// a drawing surface
// the Shift-JIS font
// the texture pages

void TT_Init()
{
	int	ii;

	if (pShadowSurface)	return;

	FontHeight = ENV_get_value_number("FontHeight", 32, "TrueType");
	if (FontHeight < MIN_TT_HEIGHT)		FontHeight = MIN_TT_HEIGHT;
	if (FontHeight > MAX_TT_HEIGHT)		FontHeight = MAX_TT_HEIGHT;

#ifndef TARGET_DC
	// set MBCS support
	_setmbcp(_MB_CP_LOCALE);	// set locale-specific codepage
#endif

	// create the memory surface for drawing
	HRESULT			hres;
	DDSURFACEDESC2	desc;

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_CAPS;
	desc.dwWidth = 640 * AA_SIZE;
	desc.dwHeight = FontHeight * AA_SIZE;
	desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
	desc.ddpfPixelFormat.dwRGBBitCount = 8;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_OWNDC | DDSCAPS_SYSTEMMEMORY;
	desc.ddsCaps.dwCaps2 = 0;
	desc.ddsCaps.dwCaps3 = 0;
	desc.ddsCaps.dwCaps4 = 0;

	hres = the_display.lp_DD4->CreateSurface(&desc, &pShadowSurface, NULL);

	ASSERT(!FAILED(hres));

	// create palette
	PALETTEENTRY	palette[256];

	for (ii = 0; ii < 256; ii++)
	{
		palette[ii].peRed = ii;
		palette[ii].peGreen = ii;
		palette[ii].peBlue = ii;
		palette[ii].peFlags = NULL;
	}

	hres = the_display.lp_DD4->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE, palette, &pShadowPalette, NULL);
	ASSERT(!FAILED(hres));

	hres = pShadowSurface->SetPalette(pShadowPalette);
	ASSERT(!FAILED(hres));

#ifdef TARGET_DC
	// Got to do it the long way round for DreamCast - don't know why.
	LOGFONT logfont;
	logfont.lfHeight			= FontHeight * AA_SIZE;
	logfont.lfWeight			= 0;
	logfont.lfEscapement		= 0;
	logfont.lfOrientation		= 0;
	logfont.lfWeight			= FW_BOLD;
	logfont.lfItalic			= FALSE;
	logfont.lfUnderline			= FALSE;
	logfont.lfStrikeOut			= FALSE;
	logfont.lfCharSet			= SHIFTJIS_CHARSET;
	logfont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
	logfont.lfQuality			= DEFAULT_QUALITY;
	logfont.lfPitchAndFamily	= FF_DONTCARE | DEFAULT_PITCH;
	logfont.lfFaceName[0]		= TEXT('\0');
	hFont = CreateFontIndirect ( &logfont );
#else //#ifdef TARGET_DC
	// create the font - hey, a function with fourteen parameters!
	hFont = CreateFont(FontHeight * AA_SIZE,
						0,0,0,
						FW_BOLD,									// weight
						FALSE,FALSE,FALSE,
						SHIFTJIS_CHARSET,							// charset
						OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,							// quality
						FF_DONTCARE | DEFAULT_PITCH,
						TYPEFACE);									// typeface name
#endif //#else //#ifdef TARGET_DC

	ASSERT(hFont);

	// create the texture cache
	NumCacheLines = 0;

	for (ii = 0; ii < NUM_TT_PAGES; ii++)
	{
		// create a page
		Texture[ii].CreateUserPage(256, TRUE);

		// create cacheline mappings
		for (int jj = 0; jj + FontHeight <= 256; jj += FontHeight)
		{
			Cache[NumCacheLines].owner = NULL;
			Cache[NumCacheLines].texture = &Texture[ii];
			Cache[NumCacheLines].y = jj;

			NumCacheLines++;
		}
	}

	// set up pixmap
	int		rr = Texture[0].mask_red;
	int		lr = Texture[0].shift_red;
	int		rg = Texture[0].mask_green;
	int		lg = Texture[0].shift_green;
	int		rb = Texture[0].mask_blue;
	int		lb = Texture[0].shift_blue;
	int		ra = Texture[0].mask_alpha;
	int		la = Texture[0].shift_alpha;

	for (ii = 0; ii < 256; ii++)
	{
		PixMapping[ii] = 0;
		PixMapping[ii] |= (255 >> rr) << lr;
		PixMapping[ii] |= (255 >> rg) << lg;
		PixMapping[ii] |= (255 >> rb) << lb;
		PixMapping[ii] |= (ii >> ra) << la;
	}

	// initialize the textcommands
	for (ii = 0; ii < MAX_TEXTCOMMANDS; ii++)
	{
		Commands[ii].validity = Free;
	}

	// initialize the DC (since it's owned, this is remembered)
	HDC		hDC;

	hres = pShadowSurface->GetDC(&hDC);
	ASSERT(!FAILED(hres));

	// prepare the DC
	SetBkColor(hDC, RGB(0,0,0));
	SetBkMode(hDC, OPAQUE);
#ifndef TARGET_DC
	SetTextAlign(hDC, TA_LEFT | TA_TOP | TA_NOUPDATECP);
	SetTextCharacterExtra(hDC, ENV_get_value_number("ExtraSpaceX", 0, "TrueType"));
#endif
	SetTextColor(hDC, RGB(255,255,255));

	HFONT		hOldFont = (HFONT)SelectObject(hDC, hFont);
	ASSERT(hOldFont);

	hres = pShadowSurface->ReleaseDC(hDC);
	ASSERT(!FAILED(hres));
}

// TT_Term
//
// release the resources

void TT_Term()
{
	if (!pShadowSurface)	return;

	// unprepare the DC
	HDC		hDC;
	HRESULT	hres;

	hres = pShadowSurface->GetDC(&hDC);
	ASSERT(!FAILED(hres));

	SelectObject(hDC, hOldFont);

	hres = pShadowSurface->ReleaseDC(hDC);
	ASSERT(!FAILED(hres));

	pShadowSurface->SetPalette(NULL);

	pShadowPalette->Release();
	pShadowPalette = NULL;

	pShadowSurface->Release();
	pShadowSurface = NULL;

	DeleteObject(hFont);
	hFont = NULL;

	for (int ii = 0; ii < NUM_TT_PAGES; ii++)
	{
		Texture[ii].Destroy();
		Texture[ii].Type = D3DTEXTURE_TYPE_UNUSED;
	}
}

// DrawTextTT
//
// exported function to draw formatted text

int DrawTextTT(char* string, int x, int y, int rx, int scale, ULONG rgb, int command, long* width)
{
	int	nbytes;

	ASSERT(rx > x);

	if (scale > 256)	scale = 256;	// no zooming up, it looks poo

	// check string length
	nbytes = strlen(string);
	ASSERT(nbytes < MAX_TT_TEXT - 1);

	// look for command
	TextCommand*	tcmd = Commands;
	TextCommand*	best = NULL;
	bool			exact = false;

	for (int ii = 0; ii < MAX_TEXTCOMMANDS; ii++)
	{
		if (tcmd->validity == Pending)
		{
			if ((tcmd->x == x) && (tcmd->y == y) && (tcmd->rx == rx) && (tcmd->command == command) && (tcmd->nbytes == nbytes))
			{
				if (!strcmp(string, tcmd->data))
				{
					// exact match
					best = tcmd;
					exact = true;
					break;
				}
			}
		}
		else if (tcmd->validity == Free)
		{
			if (!best)
			{
				best = tcmd;
			}
		}

		tcmd++;
	}

	if (!best)	return y;	// no space

	if (!exact)
	{
		best->x = x;
		best->y = y;
		best->rx = rx;
		best->scale = scale;
		best->command = command;
		best->nbytes = nbytes;
		strcpy(best->data, string);
#ifdef TARGET_DC
		best->nchars = strlen(string);
#else
		best->nchars = _mbslen((unsigned char*)string);
#endif
		best->in_cache = false;
		MeasureTextCommand(best);
	}

	best->rgb = rgb;
	best->validity = Current;

	if (width)	*width = best->x + best->fwidth;

	return y + best->lines * FontHeight * scale / 256;
}

// GetTextHeightTT
//
// get height of TT text

int GetTextHeightTT()
{
	return FontHeight;
}

// PreFlipTT
//
// end a frame

void PreFlipTT()
{
	if (!pShadowSurface)	return;		// not initialized

	int		ii;
	bool	work = false;

	// release all unused text commands
	for (ii = 0; ii < MAX_TEXTCOMMANDS; ii++)
	{
		if (Commands[ii].validity == Pending)									Commands[ii].validity = Free;
		else if ((Commands[ii].validity == Current) && !Commands[ii].in_cache)	work = true;
	}

	// check cachelines and release if owned by a deleted text command
	for (ii = 0; ii < NumCacheLines; ii++)
	{
		if (Cache[ii].owner && (Cache[ii].owner->validity == Free))		Cache[ii].owner = NULL;
	}

	// draw text if there is any to do
	if (work)
	{
		TextCommand*	tcmd = Commands;

		for (ii = 0; ii < MAX_TEXTCOMMANDS; ii++)
		{
			if ((tcmd->validity == Current) && !tcmd->in_cache)
			{
				DoTextCommand(tcmd);
				tcmd->in_cache = true;
			}
			tcmd++;
		}
	}

	// set all Current commands to Pending
	for (ii = 0; ii < MAX_TEXTCOMMANDS; ii++)
	{
		if (Commands[ii].validity == Current)	Commands[ii].validity = Pending;
	}

	// copy to the screen
	BlitText();

#ifdef _DEBUG
	if (ControlFlag)	ShowDebug();
#endif
}

// MeasureTextCommand
//
// count how many lines is in a text command, and the maximum width

static void MeasureTextCommand(TextCommand* tcmd)
{
	char*	string = tcmd->data;
	int		clen = tcmd->nchars;

	HDC		hDC;
	HRESULT	res;

	res = pShadowSurface->GetDC(&hDC);
	ASSERT(!FAILED(res));

	tcmd->lines = 0;
	tcmd->fwidth = tcmd->rx - tcmd->x;

	while (clen)
	{
		SIZE	size;
		int		chars;

#ifdef TARGET_DC
		static TCHAR cTempString[64+1];
		ASSERT ( strlen ( string ) < 64 );
		textConvertCharToUni ( cTempString, string );
		GetTextExtentExPoint(hDC,cTempString,clen,tcmd->fwidth*AA_SIZE,&chars,NULL,&size);
#else
		GetTextExtentExPoint(hDC,string,clen,tcmd->fwidth*AA_SIZE,&chars,NULL,&size);
#endif

		if ((chars < clen) && (clen - chars < 3))	chars = clen - 3;
		ASSERT(chars);
		if (!chars)	break;

		tcmd->lines++;
		clen -= chars;
#ifdef TARGET_DC
		string += chars;
#else
		string = (char*)_mbsninc((unsigned char*)string, chars);
#endif
	}

	res = pShadowSurface->ReleaseDC(hDC);
	ASSERT(!FAILED(res));
}

// DoTextCommand
//
// run a text command - split into lines, draw and cache them

static void DoTextCommand(TextCommand* tcmd)
{
	char*	string = tcmd->data;
	int		clen = tcmd->nchars;
	int		y = tcmd->y;

	while (clen)
	{
		// set up the DC so we can measure strings etc.
		HDC		hDC;
		HRESULT	res;

		res = pShadowSurface->GetDC(&hDC);
		ASSERT(!FAILED(res));

		// measure string
		SIZE	size;
		int		chars;

#ifdef TARGET_DC
		static TCHAR cTempString[64+1];
		ASSERT ( strlen ( string ) < 64 );
		textConvertCharToUni ( cTempString, string );
		GetTextExtentExPoint(hDC,cTempString,clen,tcmd->fwidth*AA_SIZE,&chars,NULL,&size);
#else
		GetTextExtentExPoint(hDC,string,clen,tcmd->fwidth*AA_SIZE,&chars,NULL,&size);
#endif

		// fix up for 1 or 2 characters over
		if ((chars < clen) && (clen - chars < 3))	chars = clen - 3;

		ASSERT(chars);
		if (!chars)	return;

		// get width
#ifdef TARGET_DC
		GetTextExtentExPoint(hDC,cTempString,chars,0,NULL,NULL,&size);
#else
		GetTextExtentExPoint(hDC,string,chars,0,NULL,NULL,&size);
#endif
		int		width = size.cx / AA_SIZE;

		// release the DC
		res = pShadowSurface->ReleaseDC(hDC);
		ASSERT(!FAILED(res));

#ifdef _DEBUG
		static char buffer[256];
		memcpy(buffer, string, chars);
		buffer[chars] = '\0';
		TRACE("Line: %s\n", buffer);
#endif

		// draw this line
		switch (tcmd->command)
		{
		case LeftJustify:
			CreateTextLine(string, chars, width, tcmd->x, y, tcmd);
			break;

		case RightJustify:
			CreateTextLine(string, chars, width, tcmd->rx - width, y, tcmd);
			break;

		case Centred:
			CreateTextLine(string, chars, width, (tcmd->x + tcmd->rx - width * tcmd->scale / 256) / 2, y, tcmd);
			break;
			
		default:
			ASSERT(0);
			break;
		}

		y += FontHeight * tcmd->scale >> 8;
		clen -= chars;
#ifdef TARGET_DC
		string += chars;
#else
		string = (char*)_mbsninc((unsigned char*)string, chars);
#endif
	}
}

// CreateTextLine
//
// create a TextLine and draw it into the DC

static void CreateTextLine(char* string, int nchars, int width, int x, int y, TextCommand* owner)
{
	ASSERT(width <= 640);

	// set up the DC
	HDC		hDC;
	HRESULT	res;

	res = pShadowSurface->GetDC(&hDC);
	ASSERT(!FAILED(res));

	// set up drawing rectangle
	RECT	rect;

	rect.left = 0;
	rect.right = width * AA_SIZE;
	rect.top = 0;
	rect.bottom = FontHeight * AA_SIZE;

	// draw text
#ifdef TARGET_DC
	static TCHAR cTempString[64+1];
	ASSERT ( strlen ( string ) < 64 );
	textConvertCharToUni ( cTempString, string );
	res = ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, cTempString, nchars, NULL);
#else
	res = ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, string, nchars, NULL);
#endif
	ASSERT ( res != 0 );

	res = pShadowSurface->ReleaseDC(hDC);
	ASSERT(!FAILED(res));

	//
	// copy to cache
	//

	// lock the surface
	DDSURFACEDESC2 ddsdesc;

	InitStruct(ddsdesc);

	res = pShadowSurface->Lock(NULL, &ddsdesc, DDLOCK_WAIT, NULL);
	ASSERT(!FAILED(res));

	UBYTE*	sptr = (UBYTE*)ddsdesc.lpSurface;
	int		spitch = ddsdesc.lPitch;

	// copy to cache
	int	line;
	int	seg = 0;

	while (width >= 0)
	{
		ASSERT(seg < 3);

		line = NewCacheLine();
		if (line != -1)
		{
			CopyToCache(&Cache[line], sptr, spitch, width);
			Cache[line].owner = owner;
			Cache[line].sx = x;
			Cache[line].sy = y;
		}

		width -= 256;
		x += owner->scale;
		seg++;
		sptr += 256 * AA_SIZE;
	}

	// unlock the surface
	res = pShadowSurface->Unlock(NULL);
	ASSERT(!FAILED(res));
}

// NewCacheLine
//
// allocate a new cache line, and return its index

static int NewCacheLine()
{
	for (int ii = 0; ii < NumCacheLines; ii++)
	{
		if (!Cache[ii].owner)	return ii;
	}
	return -1;
}

// CopyToCache
//
// copy from the shadow surface to a texture

static void CopyToCache(CacheLine* cptr, UBYTE* sptr, int spitch, int width)
{
	if (width > 256)	width = 256;

	HRESULT	res;
	UWORD*	dptr;
	SLONG	dpitch;

	res = cptr->texture->LockUser(&dptr, &dpitch);
	if (FAILED(res))	return;

	dpitch /= 2;
	dptr += cptr->y * dpitch;

#if ANTIALIAS_BY_HAND
	for (int y = 0; y < FontHeight; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int	acc = sptr[0] + sptr[1] + sptr[spitch + 0] + sptr[spitch + 1];
			dptr[x] = PixMapping[acc / 4];
			sptr += 2;
		}
		sptr += (spitch-width)*2;
		dptr += dpitch;
	}
#else
	for (int y = 0; y < FontHeight; y++)
	{
		for (int x = 0; x < width; x++)
		{
			dptr[x] = PixMapping[sptr[x]];
		}
		dptr += dpitch;
		sptr += spitch;
	}
#endif

	cptr->texture->UnlockUser();
	cptr->width = width;
}

// BlitText
//
// blit all the text on

static void BlitText()
{
	int			ii;
	D3DTexture*	ctex = NULL;
	CacheLine*	cptr = Cache;

	// go through in texture order
	for (ii = 0; ii < NumCacheLines; ii++)
	{
		if (cptr->owner)
		{
			// set render states
			if (!ctex)
			{
				BEGIN_SCENE;

				// Fixme! I need to be updated if this ever gets called - TomF.
				ASSERT ( FALSE );

				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE, FALSE);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			}
			if (ctex != cptr->texture)
			{
				ctex = cptr->texture;
				REALLY_SET_TEXTURE(ctex->GetD3DTexture());
			}

			// blit area
			TexBlit(0, cptr->y, cptr->width, cptr->y + FontHeight, cptr->sx, cptr->sy, cptr->owner->rgb, cptr->owner->scale);
		}
		cptr++;
	}

	if (ctex)
	{
		END_SCENE;
	}
}

// ShowTextures
//
// draw to screen

static void ShowTextures()
{
	BEGIN_SCENE;

	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE, FALSE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

	REALLY_SET_TEXTURE(Texture[0].GetD3DTexture());

	TexBlit(0,0,256,256,0,FontHeight*AA_SIZE,0xFFFFFFFF,256);

	END_SCENE;
}

// TexBlit
//
// "blit" from texture to screen

static void TexBlit(int x1, int y1, int x2, int y2, int dx, int dy, ULONG rgb, ULONG scale)
{
	PolyPoint2D		vert[4], *vp = vert;
	
	float	u1 = float(x1) / 256;
	float	u2 = float(x2) / 256;
	float	v1 = float(y1) / 256;
	float	v2 = float(y2) / 256;

	int		width = (x2 - x1) * scale / 256;
	int		height = (y2 - y1) * scale / 256;

	vp->SetSC(dx,dy);
	vp->SetColour(rgb);
	vp->SetSpecular(0xFF000000);
	vp->SetUV(u1,v1);
	vp++;

	vp->SetSC(dx + width,dy);
	vp->SetColour(rgb);
	vp->SetSpecular(0xFF000000);
	vp->SetUV(u2,v1);
	vp++;

	vp->SetSC(dx + width,dy + height);
	vp->SetColour(rgb);
	vp->SetSpecular(0xFF000000);
	vp->SetUV(u2,v2);
	vp++;

	vp->SetSC(dx,dy + height);
	vp->SetColour(rgb);
	vp->SetSpecular(0xFF000000);
	vp->SetUV(u1,v2);
	vp++;

	static WORD	indices[6] = { 0, 3, 1, 1, 2, 3 };

	//HRESULT	res = DRAW_INDEXED_PRIMITIVE(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, vert->GetTLVert(), 4, indices, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
	HRESULT	res = DRAW_INDEXED_PRIMITIVE(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, (D3DTLVERTEX*)vert, 4, indices, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
	ASSERT(!FAILED(res));
}

// ShowDebug
//
// display shadow screen & textures

static void ShowDebug()
{
	UWORD	mapping[256];

	for (int ii = 0; ii < 256; ii++)
	{
		UBYTE	col = ii >> 3;
		mapping[ii] = (col << 10) | (col << 5) | col;
	}

	// lock the shadow surface
	DDSURFACEDESC2 ddsdesc;
	HRESULT        ret;

	InitStruct(ddsdesc);

	ret = pShadowSurface->Lock(NULL, &ddsdesc, DDLOCK_WAIT, NULL);

	if (FAILED(ret))	return;

	UBYTE*	sptr = (UBYTE*)ddsdesc.lpSurface;
	int		spitch = ddsdesc.lPitch;

	UWORD*	dptr = (UWORD*)the_display.screen_lock();
	int		dpitch = the_display.screen_pitch / 2;

	for (int y = 0; y < FontHeight * AA_SIZE; y++)
	{
		for (int x = 0; x < 640; x++)
		{
			dptr[x] = mapping[sptr[x]];
		}
		sptr += spitch;
		dptr += dpitch;
	}

	the_display.screen_unlock();
	pShadowSurface->Unlock(NULL);

	ShowTextures();
}
