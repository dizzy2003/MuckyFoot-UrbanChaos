// Display.cpp
// Guy Simmons, 13th November 1997.

#include	"DDLib.h"
#include	"fallen/headers/demo.h"
#include	"fallen/headers/interfac.h"
#include	"BinkClient.h"
#include	"env.h"
#include	"fallen/headers/xlat_str.h"

#include "poly.h"
#include "vertexbuffer.h"
#include "polypoint.h"
#include "renderstate.h"
#include "polypage.h"
#include "gdisplay.h"
#include "panel.h"
#include	"fallen/headers/game.h"

#ifdef TARGET_DC
#include "target.h"
#include "platutil.h"
#include "DCLowLevel.h"
#include "sg_mww.h"
#endif


//
// From mfx_miles.h...
//

extern void init_my_dialog (HWND hWnd);
extern void my_dialogs_over(HWND hWnd);


SLONG				RealDisplayWidth;
SLONG				RealDisplayHeight;
SLONG				DisplayBPP;
Display				the_display;
volatile SLONG		hDDLibStyle		=	NULL,
					hDDLibStyleEx	=	NULL;
volatile HWND		hDDLibWindow	=	NULL;
volatile HMENU		hDDLibMenu		=	NULL;

int					Video3DMode = -1;			// 0 = primary, 1 = secondary, 2 = software, -1 = unknown
bool				VideoTrueColour = false;	// true = 24-bit, false = 16-bit
int					VideoRes = -1;				// 0 = 320x240, 1 = 512x384, 2= 640x480, 3 = 800x600, 4 = 1024x768, -1 = unknown

enumDisplayType eDisplayType;



#ifdef TARGET_DC
// Texture to hold the background, coz blits don't seem to work.
LPDIRECTDRAWSURFACE4 lpBackgroundCache = NULL;
LPDIRECTDRAWSURFACE4 lpBackgroundCache2 = NULL;
//static LPDIRECT3DTEXTURE2 lpBackgroundCacheTexture = NULL;
#endif


//---------------------------------------------------------------
 UBYTE			*image_mem	=	NULL,*image		=	NULL;


#ifdef DEBUG
HRESULT WINAPI EnumSurfacesCallbackFunc(
  LPDIRECTDRAWSURFACE4 lpDDSurface,
  LPDDSURFACEDESC2 lpDDSurfaceDesc,
  LPVOID lpContext )
{
	TRACE ( "Surf: width %i height %i bpp %i", lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount );
	if ( lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_COMPRESSED )
	{
		TRACE ( " compressed" );
	}
	else
	{
		TRACE ( " uncompressed" );
	}
	TRACE ( "\n" );
	return DDENUMRET_OK;
}


#endif





// ========================================================
//
// MOVIE STUFF!
//
// ========================================================


#include "mmstream.h"	// Multimedia stream interfaces
#include "amstream.h"	// DirectShow multimedia stream interfaces
#include "ddstream.h"	// DirectDraw multimedia stream interfaces

//extern ULONG get_hardware_input(UWORD type);

#ifndef TARGET_DC

void RenderStreamToSurface(IDirectDrawSurface *pSurface, IMultiMediaStream *pMMStream, IDirectDrawSurface *back_surface)
{
	IMediaStream            *pPrimaryVidStream;
	IDirectDrawMediaStream  *pDDStream;
 	IDirectDrawStreamSample *pSample;
	RECT                     rect;
	RECT                     midrect;
	DDSURFACEDESC            ddsd;

 	pMMStream->GetMediaStream(MSPID_PrimaryVideo, &pPrimaryVidStream);
	ASSERT ( pPrimaryVidStream != NULL );
 	pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&pDDStream);
	ASSERT ( pDDStream != NULL );

	/*

	InitStruct(ddsd);

	pSurface->GetSurfaceDesc(&ddsd);

	if (pDDStream->SetFormat(&ddsd, NULL) == S_OK)
	{
	}
	else
	{
		return;
	}

	*/

	InitStruct(ddsd);

	pDDStream->GetFormat(&ddsd, NULL, NULL, NULL);

	/*

	ddsd.dwWidth  = 640;
	ddsd.dwHeight = 480;

	if (pDDStream->SetFormat(&ddsd, NULL) == S_OK)
	{
	}
	else
	{
		return;
	}

	*/

 	midrect.top    = 420 - (ddsd.dwWidth  >> 1);
	midrect.left   = 240 - (ddsd.dwHeight >> 1);
	midrect.bottom = midrect.top  + ddsd.dwHeight;
 	midrect.right  = midrect.left + ddsd.dwWidth;

 	rect.top    = 0;
	rect.left   = 0;
	rect.bottom = ddsd.dwHeight;
 	rect.right  = ddsd.dwWidth;

// 	pDDStream->CreateSample(pSurface, NULL, 0, &pSample);
 	pDDStream->CreateSample(back_surface, &rect, 0, &pSample);
	ASSERT ( pSample != NULL );
	pMMStream->SetState(STREAMSTATE_RUN);


	while (pSample->Update(0, NULL, NULL, NULL) == S_OK)
	{
		if (FAILED(pSurface->Blt(
				NULL,
				back_surface,
			   &rect,
				DDBLT_WAIT,
				NULL)))
		{
			pSurface->Blt(
			   &midrect,
				back_surface,
			   &rect,
				DDBLT_WAIT,
				NULL);
		}

		MSG msg;

		if (PeekMessage(
			   &msg,
				hDDLibWindow,
				WM_KEYDOWN,
				WM_KEYDOWN,
				PM_REMOVE))
		{
			//
			// User has pressed a key.
			//

			break;
		}

		ULONG input = get_hardware_input(INPUT_TYPE_JOY);	// 1 << 1 ==> INPUT_TYPE_JOY

		if (input & (INPUT_MASK_JUMP|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION))
		{
			break;
		}
	}

	pMMStream->SetState(STREAMSTATE_STOP);

	int i;
	i = pSample->Release();
	ASSERT ( i == 0 );
	i = pDDStream->Release();
	ASSERT ( i == 0 );
	i = pPrimaryVidStream->Release();
	ASSERT ( i == 0 );
}

#else //#ifndef TARGET_DC

void RenderStreamToSurface(IDirectDrawSurface *pSurface, IMultiMediaStream *pMMStream, IDirectDrawSurface *back_surface, IDirectDrawSurface *pddsTexture=NULL )
{
	IMediaStream            *pPrimaryVidStream;
	IDirectDrawMediaStream  *pDDStream;
 	IDirectDrawStreamSample *pSample;
	RECT                     rect;
	RECT                     scrrect;
	RECT                     texrect;
	DDSURFACEDESC            ddsd;

 	HRESULT hres;

	hres = pMMStream->GetMediaStream(MSPID_PrimaryVideo, &pPrimaryVidStream);
 	hres = pPrimaryVidStream->QueryInterface(IID_IDirectDrawMediaStream, (void **)&pDDStream);

	/*

	InitStruct(ddsd);

	pSurface->GetSurfaceDesc(&ddsd);

	if (pDDStream->SetFormat(&ddsd, NULL) == S_OK)
	{
	}
	else
	{
		return;
	}

	*/

	InitStruct(ddsd);

	hres = pDDStream->GetFormat(&ddsd, NULL, NULL, NULL);

 	scrrect.top    = ( 480 - 640 * ddsd.dwHeight / ddsd.dwWidth ) >> 1;
	scrrect.left   = 0;
	scrrect.bottom = 480 - scrrect.top;
 	scrrect.right  = 640;

	texrect.left	= 0;
	texrect.right	= 1024;
	texrect.top		= 0;
	texrect.bottom	= 512;

 	rect.top    = 0;
	rect.left   = 0;
	rect.bottom = ddsd.dwHeight;
 	rect.right  = ddsd.dwWidth;

	if ( pddsTexture != NULL )
	{
		// We stream to a texture and draw that on the screen each frame.
		// This does the dithering for us, which looks a lot prettier.

		D3DTLVERTEX tlvVertex[4];
		tlvVertex[0].dvSX			= (float)(scrrect.left);
		tlvVertex[0].dvSY			= (float)(scrrect.top);
		tlvVertex[0].dvTU			= (float)(rect.left) / (float)(texrect.right);
		tlvVertex[0].dvTV			= (float)(rect.top) / (float)(texrect.bottom);
		tlvVertex[0].dvSZ			= 0.5f;
		tlvVertex[0].dvRHW			= 0.5f;
		tlvVertex[0].dcColor		= 0xffffffff;
		tlvVertex[0].dcSpecular		= 0x00000000;

		tlvVertex[1].dvSX			= (float)(scrrect.right);
		tlvVertex[1].dvSY			= (float)(scrrect.top);
		tlvVertex[1].dvTU			= (float)(rect.right) / (float)(texrect.right);
		tlvVertex[1].dvTV			= (float)(rect.top) / (float)(texrect.bottom);
		tlvVertex[1].dvSZ			= 0.5f;
		tlvVertex[1].dvRHW			= 0.5f;
		tlvVertex[1].dcColor		= 0xffffffff;
		tlvVertex[1].dcSpecular		= 0x00000000;

		tlvVertex[2].dvSX			= (float)(scrrect.right);
		tlvVertex[2].dvSY			= (float)(scrrect.bottom);
		tlvVertex[2].dvTU			= (float)(rect.right) / (float)(texrect.right);
		tlvVertex[2].dvTV			= (float)(rect.bottom) / (float)(texrect.bottom);
		tlvVertex[2].dvSZ			= 0.5f;
		tlvVertex[2].dvRHW			= 0.5f;
		tlvVertex[2].dcColor		= 0xffffffff;
		tlvVertex[2].dcSpecular		= 0x00000000;

		tlvVertex[3].dvSX			= (float)(scrrect.left);
		tlvVertex[3].dvSY			= (float)(scrrect.bottom);
		tlvVertex[3].dvTU			= (float)(rect.left) / (float)(texrect.right);
		tlvVertex[3].dvTV			= (float)(rect.bottom) / (float)(texrect.bottom);
		tlvVertex[3].dvSZ			= 0.5f;
		tlvVertex[3].dvRHW			= 0.5f;
		tlvVertex[3].dcColor		= 0xffffffff;
		tlvVertex[3].dcSpecular		= 0x00000000;


		IDirect3DTexture2 *pd3dtTexture = NULL;

		hres = pddsTexture->QueryInterface ( IID_IDirect3DTexture2, (void **)&pd3dtTexture );
		ASSERT ( SUCCEEDED ( hres ) );

		// Set up the D3D engine.
		the_display.lp_D3D_Device->SetTexture ( 0, pd3dtTexture );
		//the_display.lp_D3D_Device->SetTexture ( 0, NULL );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,	D3DTOP_MODULATE );
		//the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,	D3DTOP_SELECTARG2 );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE );
		//the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAOP,	D3DTOP_SELECTARG2 );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_TEXCOORDINDEX, 0 );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_MAGFILTER,	D3DTFG_LINEAR );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_MINFILTER,	D3DTFN_LINEAR );
		the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_MIPFILTER,	D3DTFP_NONE );

		the_display.lp_D3D_Device->SetTextureStageState ( 1, D3DTSS_COLOROP,	D3DTOP_DISABLE );
		the_display.lp_D3D_Device->SetTextureStageState ( 1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE );

		the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_ALPHATESTENABLE, FALSE );
		the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_FOGENABLE, FALSE );
		the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );

		// End any previous scene (coz it's picky).
		hres = the_display.lp_D3D_Device->EndScene();
		ASSERT ( SUCCEEDED ( hres ) );

 		hres = pDDStream->CreateSample(pddsTexture, &rect, DDSFF_PROGRESSIVERENDER, &pSample);
		hres = pMMStream->SetState(STREAMSTATE_RUN);

		while ((hres = pSample->Update(0, NULL, NULL, NULL)) == S_OK)
		{

			// Draw the texture.

			hres = the_display.lp_D3D_Device->BeginScene();
			ASSERT ( SUCCEEDED ( hres ) );

			hres = the_display.lp_D3D_Viewport->Clear ( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER );
			ASSERT ( SUCCEEDED ( hres ) );

			hres = the_display.lp_D3D_Device->DrawPrimitive ( D3DPT_TRIANGLEFAN,
															  D3DVT_TLVERTEX,
															  tlvVertex,
															  4,
															  D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS );
			ASSERT ( SUCCEEDED ( hres ) );

			hres = the_display.lp_D3D_Device->EndScene();
			ASSERT ( SUCCEEDED ( hres ) );

			hres = pSurface->Flip ( NULL, 0 );

			ULONG input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_REMAP_START_BUTTON | INPUT_TYPE_GONEDOWN );
			if (input & (INPUT_MASK_JUMP|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION))
			{
				break;
			}
		}

		// Let the texture go.
		the_display.lp_D3D_Device->SetTexture ( 0, NULL );
		int i = pd3dtTexture->Release();

		// And start a new scene, or you'll confuse the rest of the game.
		hres = the_display.lp_D3D_Device->BeginScene();

	}
	else
	{

 		hres = pDDStream->CreateSample(back_surface, &rect, 0, &pSample);
		hres = pMMStream->SetState(STREAMSTATE_RUN);

		while (pSample->Update(0, NULL, NULL, NULL) == S_OK)
		{

	#if 0
			if (FAILED(pSurface->Blt(
					NULL,
					back_surface,
				   &rect,
					DDBLT_WAIT,
					NULL)))
			{
				hres = pSurface->Blt(
				   &midrect,
					back_surface,
				   &rect,
					DDBLT_WAIT,
					NULL);
			}
	#else
			hres = pSurface->Flip ( NULL, 0 );
	#endif

			ULONG input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_REMAP_START_BUTTON | INPUT_TYPE_GONEDOWN );
			if (input & (INPUT_MASK_JUMP|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION))
			{
				break;
			}
		}
	}

	hres = pMMStream->SetState(STREAMSTATE_STOP);
	hres = pSample->Release();
	hres = pDDStream->Release();
	hres = pPrimaryVidStream->Release();

}

#endif //#else //#ifndef TARGET_DC


#include "ddraw.h"      // DirectDraw interfaces
#include "mmstream.h"   // Multimedia stream interfaces
#include "amstream.h"   // DirectShow multimedia stream interfaces
#include "ddstream.h"   // DirectDraw multimedia stream interfaces


void RenderFileToMMStream(const char * szFileName, IMultiMediaStream **ppMMStream, IDirectDraw *pDD)
{
	IAMMultiMediaStream *pAMStream = NULL;

	HRESULT hres;
#ifdef TARGET_DC
 	hres = CoCreateInstance(
		CLSID_AMMultiMediaStream,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IAMMultiMediaStream,
		(void **)&pAMStream);
	if ( FAILED ( hres ) )
	{
		switch ( hres )
		{
		case REGDB_E_CLASSNOTREG:
			hres++;
			break;
		case CLASS_E_NOAGGREGATION:
			hres++;
			break;
		}

		ASSERT ( FALSE );
		// Not good.
		return;
	}
#else
 	CoCreateInstance(
		CLSID_AMMultiMediaStream,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IAMMultiMediaStream,
		(void **)&pAMStream);
#endif

	WCHAR wPath[MAX_PATH];		// Wide (32-bit) string name


#ifdef DEBUG
	// HACK. We don't have any movies apart from this one at the moment.
	//szFileName = "\\CD-ROM\\fallen\\dshow\\pcintro_withsound.avi";
#endif


	MultiByteToWideChar(
		CP_ACP,
		0,
		szFileName,
		-1,
		wPath,
		sizeof(wPath) / sizeof(wPath[0]));

 	//hres = pAMStream->Initialize(STREAMTYPE_READ, AMMSF_NOGRAPHTHREAD, NULL);
	hres = pAMStream->Initialize(STREAMTYPE_READ, 0, NULL);
	if ( FAILED ( hres ) )
	{
		switch ( hres )
		{
		case E_ABORT:
			hres++;
			break;
		case E_INVALIDARG:
			hres++;
			break;
		case E_NOINTERFACE:
			hres++;
			break;
		case E_OUTOFMEMORY:
			hres++;
			break;
		case E_POINTER:
			hres++;
			break;
		}
	}
 	hres = pAMStream->AddMediaStream(pDD, &MSPID_PrimaryVideo, 0, NULL);
 	hres = pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, AMMSF_ADDDEFAULTRENDERER, NULL);
 	hres = pAMStream->OpenFile(wPath, 0);

	*ppMMStream = pAMStream;
}



#ifdef TARGET_DC



#if 0
// Never used - always use PlayQuickMovie
int _CRTAPI1 do_intro(void)
{

#if 0
	DDSURFACEDESC       ddsd;
 	IDirectDraw        *pDD;
	IDirectDrawSurface *pSurface;
 	IMultiMediaStream  *pMMStream;

	IDirectDrawSurface *back_surface;

	HRESULT hres = CoInitializeEx(NULL,COINIT_MULTITHREADED);

	if (SUCCEEDED(the_display.lp_DD_FrontSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&pSurface)) &&
		SUCCEEDED(the_display.lp_DD_BackSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&back_surface)))
	{


#if 1
		// Do it forever for the moment.
		while ( TRUE )
		{
			RenderFileToMMStream("\\CD-ROM\\fallen\\PCcutscene_300.avi", &pMMStream, the_display.lp_DD);
			RenderStreamToSurface(pSurface, pMMStream, back_surface);
			pMMStream->Release();

			ULONG input = get_hardware_input ( INPUT_TYPE_JOY | INPUT_TYPE_REMAP_DPAD | INPUT_TYPE_REMAP_BUTTONS | INPUT_TYPE_REMAP_START_BUTTON );
			if (input & (INPUT_MASK_JUMP|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION))
			{
				break;
			}
		}
#else
		RenderFileToMMStream("data\\eidos.avi", &pMMStream, the_display.lp_DD);
		RenderStreamToSurface(pSurface, pMMStream, back_surface);
		pMMStream->Release();

		RenderFileToMMStream("data\\urban_final.mpeg", &pMMStream, the_display.lp_DD);
		RenderStreamToSurface(pSurface, pMMStream, back_surface);
		pMMStream->Release();
#endif

		pSurface->Release();
	}

	CoUninitialize();

	// On DC, we need to free DLLs explicitely.
	CoFreeUnusedLibraries();
#endif

	return 0;
}
#endif


#else

int do_intro(void)
{
#if 0
	DDSURFACEDESC       ddsd;
 	IDirectDraw        *pDD;
	IDirectDrawSurface *pSurface;
 	IMultiMediaStream  *pMMStream;

	IDirectDrawSurface *back_surface;

#ifdef TARGET_DC
					CoInitializeEx(NULL,COINIT_MULTITHREADED);
#else
                	CoInitialize(NULL);
#endif

	/*

 	DirectDrawCreate(NULL, &pDD, NULL);
 	pDD->SetCooperativeLevel(GetDesktopWindow(), DDSCL_NORMAL);
	InitStruct(ddsd);
	ddsd.dwFlags        = DDSD_CAPS;
 	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
 	pDD->CreateSurface(&ddsd, &pPrimarySurface, NULL);

	*/


 	//RenderStreamToSurface(pPrimarySurface, pMMStream);

	// get a IDirectDrawSurface from the IDirectDrawSurface4 since M$ don't seem to have
	// bothered updating the DX Media SDK ...

	if (SUCCEEDED(the_display.lp_DD_FrontSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&pSurface)) &&
		SUCCEEDED(the_display.lp_DD_BackSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&back_surface)))
	{
		RenderFileToMMStream("data\\eidos.avi", &pMMStream, the_display.lp_DD);
		RenderStreamToSurface(pSurface, pMMStream, back_surface);
		pMMStream->Release();

		RenderFileToMMStream("data\\urban_final.mpeg", &pMMStream, the_display.lp_DD);
		RenderStreamToSurface(pSurface, pMMStream, back_surface);
		pMMStream->Release();

		pSurface->Release();
	}

	/*

	pPrimarySurface->Release();
	pDD->Release();

	*/

	CoUninitialize();

	return 0;
#else
	return 0;
#endif
}


#endif



#ifndef TARGET_DC
// Use PlayQuickMovie instead please.
int do_only_game_intro(void)
{
#if 0
	DDSURFACEDESC       ddsd;
 	IDirectDraw        *pDD;
	IDirectDrawSurface *pSurface;
 	IMultiMediaStream  *pMMStream;

	IDirectDrawSurface *back_surface;

#ifdef TARGET_DC
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
#else
	CoInitialize(NULL);
#endif

	// get a IDirectDrawSurface from the IDirectDrawSurface4 since M$ don't seem to have
	// bothered updating the DX Media SDK ...

	if (SUCCEEDED(the_display.lp_DD_FrontSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&pSurface)) &&
		SUCCEEDED(the_display.lp_DD_BackSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&back_surface)))
	{
		RenderFileToMMStream("data\\urban_final.mpeg", &pMMStream, the_display.lp_DD);
		RenderStreamToSurface(pSurface, pMMStream, back_surface);
		pMMStream->Release();

		pSurface->Release();
	}

	CoUninitialize();

	return 0;
#else
	return 0;
#endif
}
#endif


extern	CBYTE	DATA_DIR[];

void	InitBackImage(CBYTE *name)
{
	MFFileHandle	image_file;
	SLONG	height;
	CBYTE	fname[200];

	sprintf(fname,"%sdata\\%s",DATA_DIR,name);


#ifdef TARGET_DC

#if !USE_COMPRESSED_BACKGROUNDS
#error Must use compressed backgrounds on DC.
#endif


	// Horrible cheat - use ".BGS" version instead.
void	FRONTEND_scr_img_load_into_screenfull(CBYTE *name, CompressedBackground *screen);
	FRONTEND_scr_img_load_into_screenfull ( name, &(the_display.lp_DD_Background) );


#else



 	if(image_mem==0)
	{
		image_mem	=	(UBYTE*)MemAlloc(640*480*3);
	}

	if(image_mem)
	{
		image_file	=	FileOpen(fname);
		if(image_file>0)
		{
			FileSeek(image_file,SEEK_MODE_BEGINNING,18);
			image	=	image_mem+(640*479*3);
			for(height=480;height;height--,image-=(640*3))
			{
				FileRead(image_file,image,640*3);
			}
			FileClose(image_file);
		}
		the_display.create_background_surface(image_mem);
	}




	// Save out the DC .BGS version by cheating horribly.
	LPDIRECTDRAWSURFACE4 lpJunk = NULL;
extern void FRONTEND_scr_img_load_into_screenfull(CBYTE *name, LPDIRECTDRAWSURFACE4 *screen);
	FRONTEND_scr_img_load_into_screenfull ( name, &lpJunk );

	lpJunk->Release();

#endif



}

#if USE_COMPRESSED_BACKGROUNDS
void UseBackSurface(CompressedBackground use)
#else
void UseBackSurface(LPDIRECTDRAWSURFACE4 use)
#endif
{
	the_display.use_this_background_surface(use);
}




#if USE_COMPRESSED_BACKGROUNDS
CompressedBackground m_lpLastBackground = NULL;
#else
LPDIRECTDRAWSURFACE4 m_lpLastBackground = NULL;
#endif




void	ResetBackImage(void)
{
	the_display.destroy_background_surface();
#ifdef TARGET_DC
	ASSERT ( image_mem == NULL );
#endif
	if(image_mem)
	{
		MemFree(image_mem);
		image_mem=0;
	}

#ifdef TARGET_DC
	// And flush the background cache(s).
	the_display.lp_DD_Background_use_instead = NULL;
	m_lpLastBackground = NULL;
#endif

}


void	ShowBackImage(bool b3DInFrame)
{
    the_display.blit_background_surface(b3DInFrame);
}





//---------------------------------------------------------------

SLONG	OpenDisplay(ULONG width, ULONG height, ULONG depth, ULONG flags)
{
	HRESULT			result;

	result	=	the_manager.Init();
	if(FAILED(result))
	{
		return	-1;
	}

extern HINSTANCE	hGlobalThisInst;
extern void GraphicsDialog(HINSTANCE hInst, HWND hWnd);

	GraphicsDialog(hGlobalThisInst, hDDLibWindow);

extern int	Video3DMode;
extern bool	VideoTrueColour;
extern int	VideoRes;

	depth = VideoTrueColour ? 32 : 16;
	switch (VideoRes)
	{
	case 0:		width = 320; height = 240; break;
	case 1:		width = 512; height = 384; break;
	case 2:		width = 640; height = 480; break;
	case 3:		width = 800; height = 600; break;
	case 4:		width = 1024; height = 768; break;
	}

	if(flags&FLAGS_USE_3D)
		the_display.Use3DOn();

#ifndef TARGET_DC
	if(flags&FLAGS_USE_WORKSCREEN)
		the_display.UseWorkOn();
#endif

	// Voodoo must be fullscreen
	if (Video3DMode == 1)
		the_display.FullScreenOn();

	// 32-bit colour must also be fullscreen
	if (VideoTrueColour)
		the_display.FullScreenOn();

	result = SetDisplay(width,height,depth);

// The DC picks specific times to show movies.
#ifndef TARGET_DC
	if (SUCCEEDED(result))
	{
		//
		// Show a movie!
		//

		do_intro();
	}
#endif

	return result;
}

//---------------------------------------------------------------

SLONG	CloseDisplay(void)
{
	the_display.Fini();
	the_manager.Fini();

	return	1;
}

//---------------------------------------------------------------

SLONG	SetDisplay(ULONG width,ULONG height,ULONG depth)
{
	HRESULT		result;

	RealDisplayWidth	=	width;
	RealDisplayHeight	=	height;
	DisplayBPP			=	depth;
	result	=	the_display.ChangeMode(width,height,depth,0);
	if(FAILED(result))
		return	-1;

	//	Attempt to change the device to hardware.
	if(!the_display.IsFullScreen()|| SOFTWARE)
	{
		result	=	the_display.ChangeDevice(&the_display.CurrDriver->hardware_guid,NULL);
		if(FAILED(result))
			return	-1;
	}

	// tell the polygon engine
	PolyPage::SetScaling(float(width)/float(DisplayWidth), float(height)/float(DisplayHeight));

	return	0;
}

//---------------------------------------------------------------

SLONG			ClearDisplay(UBYTE r,UBYTE g,UBYTE b)
{
	DDBLTFX		dd_bltfx;

	dd_bltfx.dwSize			=	sizeof(dd_bltfx);
	dd_bltfx.dwFillColor	=	0;

	the_display.lp_DD_FrontSurface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&dd_bltfx);

	return	0;
}

struct RGB_565
{
	UWORD	R	:	5,
			G	:	6,
			B	:	5;
};

struct RGB_555
{
	UWORD	R	:	5,
			G	:	5,
			B	:	5;
};


void	LoadBackImage(UBYTE *image_data)
{
	ASSERT(0);

	UWORD			pixel,
					*surface_mem;
	SLONG			try_count,
					height,
					pitch,
					width;
	DDSURFACEDESC2	dd_sd;
	HRESULT			result;
	RGB_565			rgb;


//	if(the_display.lp_DD_BackSurface)
	{
		InitStruct(dd_sd);
		try_count		=	0;
do_the_lock:
		result			=	the_display.lp_DD_BackSurface->Lock(NULL,&dd_sd,DDLOCK_WAIT|DDLOCK_NOSYSLOCK,NULL);
		switch(result)
		{
			case	DD_OK:
				pitch		=	dd_sd.lPitch>>1;
				surface_mem	=	(UWORD*)dd_sd.lpSurface;
				for(height=0;(unsigned)height<dd_sd.dwHeight;height++,surface_mem+=pitch)
				{
					for(width=0;(unsigned)width<dd_sd.dwWidth;width++)
					{
						pixel	=	the_display.GetFormattedPixel(*(image_data+2),*(image_data+1),*(image_data+0));
						*(surface_mem+width)	=	pixel;
						image_data	+=	3;
					}
				}
				try_count	=	0;
do_the_unlock:
				result	=	the_display.lp_DD_BackSurface->Unlock(NULL);
				switch(result)
				{
					case	DDERR_SURFACELOST:
						try_count++;
						if(try_count<10)
						{
							the_display.Restore();
							goto	do_the_unlock;
						}
						break;
				}
				break;
			case	DDERR_SURFACELOST:
				try_count++;
				if(try_count<10)
				{
					the_display.Restore();
					goto	do_the_lock;
				}
				break;
			default:
				try_count++;
				if(try_count<10)
					goto	do_the_lock;
		}
	}
}

//---------------------------------------------------------------

UBYTE	tga_header[]	=	{
								0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00, 0x80, 0x02, 0xe0, 0x01,
								0x10, 0x01

							};
void	DumpBackToTGA(CBYTE *tga_name)
{
	UWORD			*rgb_fudge,
					*surface_mem;
	SLONG			height,
					pitch,
					width;
	DDSURFACEDESC2	dd_sd;
	HRESULT			result;
	MFFileHandle	tga_handle;
	RGB_565			rgb565;
	RGB_555			rgb555[640];


	if(the_display.lp_DD_BackSurface)
	{
		InitStruct(dd_sd);
		result	=	the_display.lp_DD_BackSurface->Lock(NULL,&dd_sd,DDLOCK_WAIT|DDLOCK_NOSYSLOCK,NULL);
		switch(result)
		{
			case	DD_OK:
				pitch		=	dd_sd.lPitch>>1;
				surface_mem	=	(UWORD*)dd_sd.lpSurface+(479*pitch);
				tga_handle	=	FileCreate(tga_name,TRUE);
				if(tga_handle)
				{
					rgb_fudge	=	(UWORD*)&rgb565;
					FileWrite(tga_handle,&tga_header,sizeof(tga_header));
					for(height=0;height<480;height++,surface_mem-=pitch)
					{
						for(width=0;width<640;width++)
						{
							*rgb_fudge		=	*(surface_mem+width);
							rgb555[width].R	=	rgb565.R;
							rgb555[width].G	=	rgb565.G>>1;
							rgb555[width].B	=	rgb565.B;
						}
						FileWrite(tga_handle,&rgb555,sizeof(rgb555));
					}
					FileClose(tga_handle);
				}
				result	=	the_display.lp_DD_BackSurface->Unlock(NULL);
				break;
			case	DDERR_SURFACELOST:
				the_display.Restore();
				break;
		}
	}
}

void DumpBackToRaw()
{
	SLONG i;
	SLONG x;
	SLONG y;

	UBYTE red;
	UBYTE green;
	UBYTE blue;

	CBYTE fname[32];

	FILE *handle;


	//
	// Find the first available file.
	//

	for (i = 0; i < 100; i++)
	{
		sprintf(fname, "c:\\tmp\\shot%03d.raw", i);

		handle = MF_Fopen(fname, "rb");

		if (handle)
		{
			//
			// This file already exists...
			//

			MF_Fclose(handle);
		}
		else
		{
			handle = MF_Fopen(fname, "wb");

			if (handle)
			{
				goto found_file;
			}
			else
			{
				return;
			}
		}
	}

	//
	// All thousand filenames are used up!
	//

	return;

  found_file:;

	//
	// Save out the raw.
	//

	for (y = 0; y < the_display.screen_height; y++)
	{
		for (x = 0; x < the_display.screen_width; x++)
		{
			the_display.GetPixel(
				x,
				y,
			   &red,
			   &green,
			   &blue);

			fputc(red,   handle);
			fputc(green, handle);
			fputc(blue,  handle);
		}
	}

	//
	// Close the file.
	//

	MF_Fclose(handle);
}


//---------------------------------------------------------------

Display::Display()
{
	DisplayFlags	=	0;
	CurrDevice		=	NULL;
	CurrDriver		=	NULL;
	CurrMode		=	NULL;

	CreateZBufferOn();
#if defined(NDEBUG) || defined (TARGET_DC)
	FullScreenOn();
#endif

	lp_DD_Clipper		=	NULL;
	lp_DD_FrontSurface	=	NULL;
	lp_DD_BackSurface	=	NULL;
#ifndef TARGET_DC
	lp_DD_WorkSurface	=	NULL;
#endif
	lp_DD_ZBuffer		=	NULL;
	lp_CurrPalette		=	NULL;
	lp_SysPalette		=	NULL;
	lp_D3D_Black        =   NULL;
	lp_D3D_White        =   NULL;
	lp_D3D_User         =   NULL;
	lp_DD_GammaControl	=	NULL;


	BackColour		=	BK_COL_BLACK;
	TextureList		=	NULL;
}

//---------------------------------------------------------------

Display::~Display()
{
	Fini();
}

//---------------------------------------------------------------

HRESULT	Display::Init(void)
{
	HRESULT		result;
	static bool	run_fmv = false;

#ifndef TARGET_DC
	SOFTWARE = (Video3DMode == 2) ? 1 : 0;
#endif

	if(!IsInitialised())
	{
		if((!hDDLibWindow) || (!IsWindow(hDDLibWindow)))
		{
			result	=	DDERR_GENERIC;
			// Output error.
			return	result;
		}
#define	HIDE_MOUSE	1
#ifdef	HIDE_MOUSE
#ifndef TARGET_DC
		if (IsFullScreen())
		{
			// hide the cursor
			ShowCursor(FALSE);
		}
#endif
#endif

		// Create DD/D3D Interface objects.
		result	=	InitInterfaces();
		if(FAILED(result))
			goto	cleanup;

		// Attach the window to the DD interface.
		result	=	InitWindow();
		if(FAILED(result))
			goto	cleanup;

		// Set the Mode.
		result	=	InitFullscreenMode();
		if(FAILED(result))
			goto	cleanup;

		// Create Front Surface & Palette.
		result	=	InitFront();
		if(FAILED(result))
			goto	cleanup;

		// Create Back surface & D3D device.
		result	=	InitBack();
		if(FAILED(result))
			goto	cleanup;

		if (background_image_mem)
		{
			create_background_surface(background_image_mem);
		}
		else
		{
		}

		InitOn();

#ifndef TARGET_DC
#ifndef VERSION_DEMO
		// run the FMV
		if (!run_fmv)
		{
			RunFMV();
			run_fmv = true;
		}
#endif
#endif

		return	DD_OK;

cleanup:
		Fini();
		return	DDERR_GENERIC;

	}
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::Fini(void)
{
	// Cleanup
	toGDI();

	if (lp_DD_Background)
	{
#if USE_COMPRESSED_BACKGROUNDS
		MemFree ( lp_DD_Background );
#else //#if USE_COMPRESSED_BACKGROUNDS
		lp_DD_Background->Release();
#endif //#else //#if USE_COMPRESSED_BACKGROUNDS
		lp_DD_Background = NULL;
	}
    FiniBack();
    FiniFront();
	FiniFullscreenMode ();
	FiniWindow ();
	FiniInterfaces();

	InitOff();
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::GenerateDefaults(void)
{
	D3DDeviceInfo	*new_device;
	DDDriverInfo	*new_driver;
	DDModeInfo		*new_mode;
	HRESULT			result;


	new_driver	=	ValidateDriver(NULL);
	if(!new_driver)
    {
        // Error, invalid DD Guid
		result	=	DDERR_GENERIC;;
		DebugText("GenerateDefaults: unable to get default driver\n");
		// Output error.
		return	result;
	}

	if(IsFullScreen())
	{
		// Get D3D device and compatible mode
		if	(
				!GetFullscreenMode	(
										new_driver,
										NULL,
										DEFAULT_WIDTH,
										DEFAULT_HEIGHT,
										DEFAULT_DEPTH,
										0,
										&new_mode,
										&new_device
									)
			)
		{
			result	=	DDERR_GENERIC;
			DebugText("GenerateDefaults: unable to get default screen mode\n");
			return	result;
		}
	}
	else
	{
		// Get Desktop mode and compatible D3D device
		if	(
				!GetDesktopMode	(
									new_driver,
									NULL,
									&new_mode,
									&new_device
								)
			)
		{
			result	=	DDERR_GENERIC;
			DebugText("GenrateDefaults: unable to get default screen mode\n");
			return	result;
		}
	}

	// Return results
	CurrDriver	=	new_driver;
	CurrMode	=	new_mode;
	CurrDevice	=	new_device;

	// Success.
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::InitInterfaces(void)
{
    GUID			*the_guid;
    HRESULT         result;


    // Do we have a current DD Driver
    if(!CurrDriver)
    {
		// So, Grab the Primary DD driver.
		CurrDriver	=	ValidateDriver(NULL);
		if(!CurrDriver)
		{
			// Error, No current Driver
			result	=	DDERR_GENERIC;
			// Output error.
			return	result;
		}
    }

    // Get DD Guid.
    the_guid	=	CurrDriver->GetGuid();

    // Create DD interface
    result	=	DirectDrawCreate(the_guid,&lp_DD,NULL);
    if(FAILED(result))
    {
        // Error
		// Output error.
		goto	cleanup;
    }

    // Get DD4 interface
	result	=	lp_DD->QueryInterface((REFIID)IID_IDirectDraw4,(void **)&lp_DD4);
	if(FAILED(result))
	{
        // Error
		// Output error.

#ifdef TARGET_DC
		ASSERT(FALSE);
#else
		// Inform User that they Need DX 6.0 installed
        MessageBox(hDDLibWindow,TEXT("Need DirectX 6.0 or greater to run"), TEXT("Error"),MB_OK);
#endif
        goto	cleanup;
    }

    // Get D3D interface
	result	=	lp_DD4->QueryInterface((REFIID)IID_IDirect3D3,(void **)&lp_D3D);
	if(FAILED(result))
    {
        // Error
		// Output error.
        goto	cleanup;
    }

	// Mark this stage as done
	TurnValidInterfaceOn();

    // Success
    return DD_OK;

cleanup:
    // Failure
    FiniInterfaces ();

    return	result;
}

//---------------------------------------------------------------

HRESULT	Display::FiniInterfaces(void)
{
	// Mark this stage as invalid
	TurnValidInterfaceOff ();

    // Release Direct3D Interface
	if(lp_D3D)
	{
		lp_D3D->Release();
		lp_D3D	=	NULL;
	}

    // Release DirectDraw4 Interface
	if(lp_DD4)
	{
		lp_DD4->Release();
		lp_DD4	=	NULL;
	}

    // Release DirectDraw Interface
	if(lp_DD)
	{
		lp_DD->Release();
		lp_DD	=	NULL;
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::InitWindow(void)
{
	HRESULT		result;
	SLONG		flags;


	// Check Initialization
	if((!hDDLibWindow) || (!IsWindow(hDDLibWindow)))
	{
		// Error, we need a valid window to continue
		result	=	DDERR_GENERIC;
		// Output error.
		return	result;
	}

#ifdef TARGET_DC
	// Must be this on DC.
	flags = DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
#else //#ifdef TARGET_DC
    // Get Cooperative Flags
	if(IsFullScreen())
	{
	    flags	=	DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_FPUSETUP;
	}
	else
	{
		flags	=	DDSCL_NORMAL | DDSCL_FPUSETUP;
	}
#endif //#else //#ifdef TARGET_DC

    // Set Cooperative Level
    result	=	lp_DD4->SetCooperativeLevel(hDDLibWindow,flags);
	if(FAILED(result))
	{
        // Error
		// Output error.
		return	result;
    }

	// init VB
	VB_Init();
	TheVPool->Create(lp_D3D, !CurrDevice->IsHardware());

	// Success
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::FiniWindow(void)
{
	HRESULT		result;

	VB_Term();

#ifndef TARGET_DC
	if(lp_DD4)
	{
		result	=	lp_DD4->SetCooperativeLevel(hDDLibWindow,DDSCL_NORMAL|DDSCL_FPUSETUP);
		if(FAILED(result))
		{
			// Error
			// Output error.
			return	result;
		}
	}
#endif

	// Success
	return DD_OK;
}

//---------------------------------------------------------------



#define	FMV1a	"eidos"
#define	FMV1b	"logo24"
#define	FMV2	"pcintro_withsound"
#define FMV3	"new_pccutscene%d_300"


#ifndef TARGET_DC
// this function returns true to continue or false to end the movie

static LPDIRECTDRAWSURFACE4		fmv_primary;
static LPDIRECTDRAWSURFACE4		fmv_secondary;

static bool bink_flipper()
{
	fmv_primary->Blt(NULL, fmv_secondary, NULL, DDBLT_WAIT, NULL);

	ULONG input = get_hardware_input(INPUT_TYPE_JOY) | get_hardware_input(INPUT_TYPE_KEY);

	if (input & (INPUT_MASK_JUMP|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION))
	{
		return false;
	}

	return true;
}



static void PlayMovie(int type)
{
	LPDIRECTDRAW	lpDD;
	LPDIRECTDRAW4	lpDD4;
	HRESULT			res;

	// init DirectDraw
    if (FAILED(DirectDrawCreate(NULL, &lpDD, NULL)))	return;
	if (FAILED(lpDD->QueryInterface((REFIID)IID_IDirectDraw4, (void**)&lpDD4)))
	{
		lpDD->Release();
		return;
	}

	if (FAILED(lpDD4->SetCooperativeLevel(hDDLibWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_FPUSETUP)))
	{
		lpDD4->Release();
		lpDD->Release();
		return;
	}

	// clear the current screen
	DDSURFACEDESC2		dd_sd;
	InitStruct(dd_sd);

	dd_sd.dwFlags			=	DDSD_CAPS;
	dd_sd.ddsCaps.dwCaps	=	DDSCAPS_PRIMARYSURFACE;

	if (SUCCEEDED(lpDD4->CreateSurface(&dd_sd, &fmv_primary, NULL)))
	{
		DDBLTFX bfx;
		InitStruct(bfx);

		bfx.dwFillColor = 0;

		fmv_primary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &bfx);

		fmv_primary->Release();
	}

	// go to 640x480xlots of colours
	TRACE("Trying 640x480x24\n");
	if (FAILED(lpDD4->SetDisplayMode(640,480,24,0,0)))
	{
		TRACE("Trying 640x480x32\n");
		if (FAILED(lpDD4->SetDisplayMode(640,480,32,0,0)))
		{
			TRACE("Trying 640x480x16\n");
			if (FAILED(lpDD4->SetDisplayMode(640,480,16,0,0)))
			{
				goto error;
			}
		}
	}

#ifndef NDEBUG
	ShowCursor(FALSE);
#endif

	// create a primary surface
	InitStruct(dd_sd);

	dd_sd.dwFlags			=	DDSD_CAPS;
	dd_sd.ddsCaps.dwCaps	=	DDSCAPS_PRIMARYSURFACE;

	if (SUCCEEDED(lpDD4->CreateSurface(&dd_sd, &fmv_primary, NULL)))
	{
		DDBLTFX bfx;
		InitStruct(bfx);

		bfx.dwFillColor = 0;

		fmv_primary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &bfx);

		// create a back buffer
		InitStruct(dd_sd);
		fmv_primary->GetSurfaceDesc(&dd_sd);
		dd_sd.ddsCaps.dwCaps	=	0;

		if (SUCCEEDED(lpDD4->CreateSurface(&dd_sd, &fmv_secondary, NULL)))
		{
			DDBLTFX bfx;
			InitStruct(bfx);

			bfx.dwFillColor = 0;

			fmv_secondary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &bfx);

			IDirectDrawSurface*	lpdds;

			if (SUCCEEDED(fmv_secondary->QueryInterface(IID_IDirectDrawSurface, (void**)&lpdds)))
			{
				if (!type)
				{
					BinkPlay("bink\\" FMV1a ".bik", lpdds, bink_flipper);
					BinkPlay("bink\\" FMV1b ".bik", lpdds, bink_flipper);
					BinkPlay("bink\\" FMV2  ".bik", lpdds, bink_flipper);
				}
				else
				{
					char	filename[MAX_PATH];
					sprintf(filename, "bink\\" FMV3, type);
					TRACE("Playing %s\n", filename);
					BinkPlay(filename, lpdds, bink_flipper);
				}
				lpdds->Release();
			}

			fmv_secondary->Release();
		}

		fmv_primary->Release();
	}

#ifndef NDEBUG
	ShowCursor(TRUE);
#endif

error:
	lpDD4->Release();
	lpDD->Release();
}


LPDIRECTDRAWSURFACE4 mirror;

static bool quick_flipper()
{
	the_display.lp_DD_FrontSurface->Blt(&the_display.DisplayRect,mirror,NULL,DDBLT_WAIT,NULL);

	return true;
}

#endif //#ifndef TARGET_DC



#ifdef TARGET_DC


#if 0
// DirectArseShow version.

void PlayQuickMovie(SLONG type, SLONG language)
{

//#ifdef DEBUG
	// Not just now thanks.
	//return;
//#endif

	// Use DirectShow on DC.
	DDSURFACEDESC       ddsd;
 	IDirectDraw        *pDD;
	IDirectDrawSurface *pSurface;
 	IMultiMediaStream  *pMMStream;

	IDirectDrawSurface *back_surface;

	HRESULT hres = CoInitializeEx(NULL,COINIT_MULTITHREADED);

	if (SUCCEEDED(the_display.lp_DD_FrontSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&pSurface)) &&
		SUCCEEDED(the_display.lp_DD_BackSurface->QueryInterface(IID_IDirectDrawSurface, (void**)&back_surface)))
	{

		pSurface->AddRef();
		int i = pSurface->Release();
		ASSERT ( i == 1 );
		back_surface->AddRef();
		i = back_surface->Release();
		ASSERT ( i == 1 );


#if 0
		// Create a texture to render to.
		DDSURFACEDESC ddsd;
		InitStruct(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.dwWidth = 1024;
		ddsd.dwHeight = 512;
		//ddsd.dwWidth  = 512;
		//ddsd.dwHeight = 256;

		ddsd.ddsCaps.dwCaps	 = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
		//ddsd.ddsCaps.dwCaps2 = 0;
		InitStruct(ddsd.ddpfPixelFormat);
		ddsd.ddpfPixelFormat.dwFourCC = 0;
#if 0
		// Can't get 32bpp to work
		ddsd.ddpfPixelFormat.dwFlags = DDPF_ALPHAPIXELS | DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
		ddsd.ddpfPixelFormat.dwRBitMask			= 0x00ff0000;
		ddsd.ddpfPixelFormat.dwGBitMask			= 0x0000ff00;
		ddsd.ddpfPixelFormat.dwBBitMask			= 0x000000ff;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= 0xff000000;
#else
		// Can't get 32bpp to work
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwRBitMask			= 0xf800;
		ddsd.ddpfPixelFormat.dwGBitMask			= 0x07e0;
		ddsd.ddpfPixelFormat.dwBBitMask			= 0x001f;
		ddsd.ddpfPixelFormat.dwRGBAlphaBitMask	= 0x0000;
#endif


		IDirectDrawSurface *pddsTexture;
		hres = the_display.lp_DD->CreateSurface(&ddsd, &pddsTexture, NULL);
		ASSERT ( SUCCEEDED ( hres ) );
#else
		// No texturing gubbins.
		IDirectDrawSurface *pddsTexture = NULL;
#endif


		char	filename[MAX_PATH];
		char *pcDirectory;

		switch ( language )
		{
		case 1:
			pcDirectory = "\\CD-ROM\\fallen\\dshow_french\\";
			break;
		default:
			ASSERT ( FALSE );
			/* FALLTHROUGH */
		case 0:
			pcDirectory = "\\CD-ROM\\fallen\\dshow\\";
			break;
		}


		if ( type == -1 )
		{
			// These two don't have language-specific versions.
			// Eidos & MF logo FMVs
			ASSERT ( language == 0 );

			// Play this one (Eidos).
#if 0
			RenderFileToMMStream("\\CD-ROM\\fallen\\dshow\\" FMV1a ".avi", &pMMStream, the_display.lp_DD);
			RenderStreamToSurface(pSurface, pMMStream, back_surface, pddsTexture);
			i = pMMStream->Release();
			ASSERT ( i == 0 );
#endif

			// Then play this one (Muckyfoot).
			sprintf(filename, "\\CD-ROM\\fallen\\dshow\\" FMV1b ".avi" );
		}
		else if ( type == 0 )
		{
			// The intro movie.
			sprintf(filename, "%s" FMV2 ".avi", pcDirectory );
		}
		else
		{
			sprintf(filename, "%s" FMV3 ".avi", pcDirectory, type );
		}



#if 1
#ifdef DEBUG
		// Show lots of them.
#if 1
		RenderFileToMMStream("\\CD-ROM\\fallen\\dshow\\pccutscene1_300_25_100.avi", &pMMStream, the_display.lp_DD);
		// This seems to be needed, sadly.
		Sleep ( 500 );
		RenderStreamToSurface(pSurface, pMMStream, back_surface, pddsTexture);
		i = pMMStream->Release();
		ASSERT ( i == 0 );
#endif

#if 1
		RenderFileToMMStream("\\CD-ROM\\fallen\\dshow_french\\PCcutscene1_TEST_320_15C.avi", &pMMStream, the_display.lp_DD);
		// This seems to be needed, sadly.
		Sleep ( 500 );
		RenderStreamToSurface(pSurface, pMMStream, back_surface, pddsTexture);
		i = pMMStream->Release();
		ASSERT ( i == 0 );
#endif
#endif
#endif


#if 0
// Not just yet.
		TRACE("Playing %s\n", filename);
		RenderFileToMMStream(filename, &pMMStream, the_display.lp_DD);
		RenderStreamToSurface(pSurface, pMMStream, back_surface, pddsTexture);
		i = pMMStream->Release();
		ASSERT ( i == 0 );

#endif

		if ( pddsTexture != NULL )
		{
			i = pddsTexture->Release();
			ASSERT ( i == 0 );
		}
		i = pSurface->Release();
		ASSERT ( i == 0 );
		i = back_surface->Release();
		ASSERT ( i == 0 );
	}
	else
	{
		ASSERT ( FALSE );
	}


	hres = the_display.lp_D3D_Device->EndScene();
	ASSERT ( SUCCEEDED ( hres ) );


	CoUninitialize();


	// Incant this spell to get the DC to GIVE ME BACK MY MEMORY YOU BASTARD
	// It is important to _believe_ this will work. If you question it, it will fail.
	// You have been warned. Ask not for whom the memory vanishes...

	Sleep(4);
	CoFreeUnusedLibraries();
	Sleep(4);
	CoFreeUnusedLibraries();

}

#else


static bool bMWInitialised = FALSE;



static bool bThereWasAnErrorInMW = FALSE;

static void	errMWErrFunc(void *errobj, Sint32 errcode)
{
	bThereWasAnErrorInMW = TRUE;

	return;
}



void PlayQuickMovie(SLONG type, SLONG language, bool bAllowButtonsToExit )
{

#if 0
#ifdef DEBUG
	// Not just now thanks.
	return;
#endif
#endif




	// CRI had better rule.
	char	filename[MAX_PATH];
	char *pcDirectory;
	char *pcSubDir;

	switch ( language )
	{
	case 1:
		pcDirectory = "\\CD-ROM\\fallen\\dshowfr\\";
		break;
	default:
		ASSERT ( FALSE );
		/* FALLTHROUGH */
	case 0:
		pcDirectory = "\\CD-ROM\\fallen\\dshow\\";
		break;
	}



	bool bSixtyHertz;

	// 50 or 60Hz?
	BYTE bFormat = GetVideoOutputFormat();
	switch ( bFormat )
	{
	case VIDFMT_NTSC_RGB:
	case VIDFMT_NTSC:
	case VIDFMT_VGA:
		// NTSC 60Hz or VGA 60Hz.
		bSixtyHertz = TRUE;
		break;

	case VIDFMT_PAL_RGB:
	case VIDFMT_PAL:
	case VIDFMT_PAL_M_RGB:
	case VIDFMT_PAL_M:
	case VIDFMT_PAL_N_RGB:
	case VIDFMT_PAL_N:
		// PAL 50Hz.
		bSixtyHertz = FALSE;
		break;

	default:
		ASSERT ( FALSE );
		break;
	}

	if ( bSixtyHertz )
	{
		// Don't have any 60Hz ones yet.
		//pcSubDir = "Sixty\\";
		pcSubDir = "Fifty\\";
	}
	else
	{
		pcSubDir = "Fifty\\";
	}


	switch ( type )
	{
	case -3:
		ASSERT ( language == 0 );
		// CRI logo.
		sprintf ( filename, "%s%ssfdlogoq.sfd", pcDirectory, pcSubDir );
		break;
	case -2:
		ASSERT ( language == 0 );
		// Eidos logo.
		sprintf ( filename, "%s%seidos.sfd", pcDirectory, pcSubDir );
		break;
	case -1:
		ASSERT ( language == 0 );
		// MF logo.
		sprintf ( filename, "%s%slogo24.sfd", pcDirectory, pcSubDir );
		break;
	case 0:
		// Intro movie.
		sprintf ( filename, "%s%spcintro_withsound.sfd", pcDirectory, pcSubDir );
		break;
	case 1:
		// Cutscene 1 (Bane's office).
		sprintf ( filename, "%s%spccutscene1_300.sfd", pcDirectory, pcSubDir );
		break;
	case 2:
		// Cutscene 2 (before Day of Reckoning).
		// No speech - use the English.
		sprintf ( filename, "\\CD-ROM\\fallen\\dshow\\%spccutscene2_300.sfd", pcSubDir );
		break;
	case 3:
		// Cutscene 3 (Day of Reckoning won).
		// No speech - use the English.
		sprintf ( filename, "\\CD-ROM\\fallen\\dshow\\%spccutscene3_300.sfd", pcSubDir );
		break;
	default:
		ASSERT ( FALSE );
		break;
	}


	// Test this one instead.
	//strcpy ( filename, "\\CD-ROM\\fallen\\dshow\\pcintro_withsound.sfd" );


#if 0
	// Sample movie settings.
#define MOVIE_FTYPE	 MWE_PLY_FTYPE_SFD
#define MOVIE_BPS    (1024 * 1024 * 4)
#define MOVIE_WIDTH	 (352)
#define MOVIE_HEIGHT (240)
#define MOVIE_NFRM	 (3)
#else
#define MOVIE_FTYPE	 MWE_PLY_FTYPE_SFD
// Yes, the movies are actually only 3800 bits/sec, but any lower than
// this and it starts to break up. Bloody thing.
#define MOVIE_BPS    (1024 * 1024 * 5)
//#define MOVIE_WIDTH	 (640)
//#define MOVIE_HEIGHT (480)
#define MOVIE_WIDTH	 (320)
#define MOVIE_HEIGHT (240)
//#define MOVIE_WIDTH	 (512)
//#define MOVIE_HEIGHT (256)
#define MOVIE_NFRM	 (3)
#endif


// Doesn't work - don't set it.
#define HALVE_FRAME_RATE 0



	// Init CRI stuff.

	// Imported from DCLowLevel.
extern LPDIRECTSOUND g_pds;


	ASSERT ( the_display.lp_DD4 != NULL );
	ASSERT ( the_display.lp_DD_FrontSurface != NULL );
	ASSERT ( the_display.lp_DD_BackSurface != NULL );
	ASSERT ( g_pds != NULL );


	MWS_PLY_IPRM_SFW	iprm;

	memset(&iprm, 0, sizeof(MWS_PLY_IPRM_SFW));
	iprm.winhn    = NULL;
	iprm.ddraw    = the_display.lp_DD4;
	iprm.dds_prim = the_display.lp_DD_FrontSurface;
	iprm.dds_back = the_display.lp_DD_BackSurface;
	iprm.dsnd     = g_pds;


	// 50 or 60Hz?
	if ( bSixtyHertz )
	{
#if HALVE_FRAME_RATE
		iprm.vfreq    = MWE_PLY_VFREQ_NTSC / 2;
#else
		iprm.vfreq    = MWE_PLY_VFREQ_NTSC;
#endif
	}
	else
	{
#if HALVE_FRAME_RATE
		iprm.vfreq    = MWE_PLY_VFREQ_PAL / 2;
#else
		iprm.vfreq    = MWE_PLY_VFREQ_PAL;
#endif
	}


	mwPlyInitSfw(&iprm);

	mwPlyEntryErrFunc(errMWErrFunc, NULL);

	//bMWInitialised = TRUE;



	// Do this and discard to stop held-down buttons skipping past (all too easy to do).
	get_hardware_input(INPUT_TYPE_JOY | INPUT_TYPE_GONEDOWN);


	// Allocate the work memory.
	DWORD dwWorkSpaceSize = mwPlyCalcWorkSfw(MOVIE_FTYPE, MOVIE_BPS,
								MOVIE_WIDTH, MOVIE_HEIGHT, MOVIE_NFRM);


	Sint8 *pcMWWorkSpace = (Sint8*)VirtualAlloc(NULL, dwWorkSpaceSize, MEM_COMMIT, PAGE_READWRITE);
	if ( pcMWWorkSpace == NULL )
	{
		// Out of space for FMV.
		ASSERT ( FALSE );
	}
	else
	{

		// Turn off VMU screen updates - they are waaaay too slow.
		SetVMUScreenUpdateEnable ( FALSE );


		// Draw black screens.
		DDBLTFX ddbf;
		ddbf.dwSize      = sizeof(DDBLTFX);
		ddbf.dwFillColor = 0;
		the_display.lp_DD_BackSurface->Blt(NULL, NULL, NULL,
								DDBLT_WAIT | DDBLT_COLORFILL, &ddbf);
		the_display.lp_DD_FrontSurface->Flip(NULL, DDFLIP_WAIT);
		the_display.lp_DD4->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
		the_display.lp_DD_BackSurface->Blt(NULL, NULL, NULL,
								DDBLT_WAIT | DDBLT_COLORFILL, &ddbf);



// Tries to "catch" dropped video frames.
// Actually, at the moment, it just spots them.
#define CATCH_DROPPED_FRAMES 1



		MWPLY ply;

		// Start video.
		MWS_PLY_CPRM_SFW	cprm;

		memset(&cprm, 0, sizeof(cprm));
		cprm.ftype        = MOVIE_FTYPE;
		cprm.max_bps      = MOVIE_BPS;
		cprm.max_width    = MOVIE_WIDTH;
		cprm.max_height   = MOVIE_HEIGHT;
		cprm.nfrm_pool_wk = MOVIE_NFRM;
		cprm.wksize       = dwWorkSpaceSize;
		cprm.work         = pcMWWorkSpace;

		ply = mwPlyCreateSfw(&cprm);
		ASSERT ( ply != NULL );

		// Filename is in ASCII, not unicode.
		mwPlyStartFname ( ply, (signed char *)filename );


#if CATCH_DROPPED_FRAMES
		// Wait for a VBLANK.
		the_display.lp_DD4->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
		DWORD dwNumFramesIDrew = 0;
		DWORD dwNumFramesIActuallyDrew = 0;
		DWORD dwStartTime = timeGetTime();
		DWORD dwLastFrameError = 0;
#endif





		// Play video.
		MSG				msg;
		bool			bStopIt = FALSE;

		while ( !bStopIt )
		{
			mwExecMainServer();

			// Flush messages (but ignore them).
			if (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (!GetMessage(&msg, NULL, 0, 0))
				{
					// Panic.
					ASSERT ( FALSE );
					break;
				}
				//TranslateMessage(&msg);
				//DispatchMessage(&msg);
			}


			// Only scan input every now and then.
			static iInputCountdown = 0;
			if ( ( ( iInputCountdown++ ) & 3 ) == 0 )
			{
				ULONG input = get_hardware_input(INPUT_TYPE_JOY | INPUT_TYPE_GONEDOWN);
				if ( bAllowButtonsToExit )
				{
					if (input & (INPUT_MASK_JUMP|INPUT_MASK_START|INPUT_MASK_SELECT|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION))
					{
						bStopIt = TRUE;
					}
				}
				else
				{
					// Not allowed to exit this one, but read the input anyway
					// (there's stuff that needs doing, e.g. soft reset handling).
				}
			}

			if (mwPlyGetStat(ply) == MWE_PLY_STAT_PLAYEND)
			{
				// End of movie.
				bStopIt = TRUE;
			}

			if ( bThereWasAnErrorInMW )
			{
				ASSERT ( FALSE );
				bStopIt = TRUE;
			}

			// Flip the screen.
			the_display.lp_DD_FrontSurface->Flip(NULL, 0);

			// V sync.
			the_display.lp_DD4->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
#if HALVE_FRAME_RATE
			// Double wait.
			the_display.lp_DD4->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
#endif


#if CATCH_DROPPED_FRAMES
			dwNumFramesIDrew++;
			dwNumFramesIActuallyDrew++;

			// The >>4 is to stop the calculations ovberflowing.
			DWORD dwTimeDifference = ( timeGetTime() - dwStartTime ) >> 4;
			DWORD dwNumberOfFramesThatShouldHaveBeenShown = ( dwTimeDifference * iprm.vfreq ) / ( ( 1000 * 1000 ) >> 4 );
			int iThisError = (int)dwNumberOfFramesThatShouldHaveBeenShown - (int)dwNumFramesIDrew;

			if ( iThisError > 1 )
			{
				if ( dwNumFramesIActuallyDrew < 200 )
				{
					// Bedding in period - for the first chunk of frames,
					// there is a big delay, which should not be corrected for.
					TRACE ( "Bedding in error %i\n", iThisError );
					dwNumFramesIDrew += iThisError - 1;
				}
				else
				{
					// Try to correct the error by doing another frame.
					TRACE ( "Correcting error %i\n", iThisError );
					mwExecMainServer();

					// This is a massive tweak thing. For some reason, this over-corrects
					// a fair bit. So pretend we drew more than we did.
					dwNumFramesIDrew += iThisError >> 1;
					dwNumFramesIActuallyDrew++;
				}
			}

#endif


		}

		TRACE ( "Done\n" );



		// Stop video.

		// Draw black screens again.
		ddbf.dwSize      = sizeof(DDBLTFX);
		ddbf.dwFillColor = 0;
		the_display.lp_DD_BackSurface->Blt(NULL, NULL, NULL,
								DDBLT_WAIT | DDBLT_COLORFILL, &ddbf);
		the_display.lp_DD_FrontSurface->Flip(NULL, DDFLIP_WAIT);
		the_display.lp_DD4->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
		the_display.lp_DD_BackSurface->Blt(NULL, NULL, NULL,
								DDBLT_WAIT | DDBLT_COLORFILL, &ddbf);

		mwPlyStop(ply);

		mwPlyDestroy(ply);
	}

	// End-of-day cleanup.
	mwPlyFinishSfw();


	// Turn VMU screen updates back on.
	SetVMUScreenUpdateEnable ( TRUE );


	if ( pcMWWorkSpace != NULL )
	{
		VirtualFree ( pcMWWorkSpace , 0, MEM_RELEASE );
	}


#if 0
	// Incant this spell to get the DC to GIVE ME BACK MY MEMORY YOU BASTARD
	// It is important to _believe_ this will work. If you question it, it will fail.
	// You have been warned. Ask not for whom the memory vanishes...

	Sleep(4);
	CoFreeUnusedLibraries();
	Sleep(4);
	CoFreeUnusedLibraries();
#endif

}

#endif



#else //#ifdef TARGET_DC
void PlayQuickMovie(SLONG type, SLONG language_ignored, bool bIgnored )
{
	DDSURFACEDESC2 back;
	DDSURFACEDESC2 mine;

	//
	// Must create a 640x480 surface with the same pixel format as the
	// primary surface.
	//

	InitStruct(back);

	the_display.lp_DD_BackSurface->GetSurfaceDesc(&back);

	//
	// Create the mirror surface in system memory.
	//

	InitStruct(mine);

	mine.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	mine.dwWidth         = 640;
	mine.dwHeight        = 480;
	mine.ddpfPixelFormat = back.ddpfPixelFormat;
	mine.ddsCaps.dwCaps	 = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	HRESULT result = the_display.lp_DD4->CreateSurface(&mine, &mirror, NULL);

	IDirectDrawSurface *lpdds;

	if (SUCCEEDED(mirror->QueryInterface(IID_IDirectDrawSurface, (void**)&lpdds)))
	{
		if (!type)
		{
			BinkPlay("bink\\" FMV1a ".bik", lpdds, quick_flipper);
			BinkPlay("bink\\" FMV1b ".bik", lpdds, quick_flipper);
			BinkPlay("bink\\" FMV2  ".bik", lpdds, quick_flipper);
		}
		else
		{
			char	filename[MAX_PATH];
			sprintf(filename, "bink\\" FMV3 ".bik", type);
			TRACE("Playing %s\n", filename);
			BinkPlay(filename, lpdds, quick_flipper);
		}
	}

	mirror->Release();
}
#endif //#else //#ifdef TARGET_DC


#ifndef TARGET_DC
void Display::RunFMV()
{
	if (!hDDLibWindow)	return;

	// should we run it?
	if (!ENV_get_value_number("play_movie", 1, "Movie"))	return;

	PlayQuickMovie(0,0,TRUE);
}
#endif


void Display::RunCutscene(int which, int language, bool bAllowButtonsToExit)
{
	//extern void FRONTEND_scr_unload_theme(void);
	//FRONTEND_scr_unload_theme();

	//Fini();

	PlayQuickMovie(which, language, bAllowButtonsToExit);

	//Init();
}


//---------------------------------------------------------------

HRESULT	Display::InitFullscreenMode(void)
{
	SLONG		flags	=	0,
				style,
				w,h,bpp,refresh;
	HRESULT		result;


	// Check Initialization
	if((!CurrMode) || (!lp_DD4))
	{
		// Error, we need a valid mode and DirectDraw 2 interface to proceed
		result	=	DDERR_GENERIC;
		DebugText("InitFullScreenMode: invalid initialization\n");
		return	result;
	}

	// DC is always fullscreen
#ifndef TARGET_DC
	// Do window mode setup.
	if(!IsFullScreen())
	{
		// Set window style.
		style	=	GetWindowStyle(hDDLibWindow);
		style	&=	~WS_POPUP;
		style	|=	WS_OVERLAPPED|WS_CAPTION|WS_THICKFRAME|WS_MINIMIZEBOX;
		SetWindowLong(hDDLibWindow,GWL_STYLE,style);

		// Save Surface Rectangle.
		DisplayRect.left   = 0;
		DisplayRect.top    = 0;
		DisplayRect.right  = RealDisplayWidth;
		DisplayRect.bottom = RealDisplayHeight;

		AdjustWindowRectEx	(
								&DisplayRect,
						        GetWindowStyle(hDDLibWindow),
						        GetMenu(hDDLibWindow) != NULL,
						        GetWindowExStyle(hDDLibWindow)
        					);
		SetWindowPos(
						hDDLibWindow,
    	    			NULL,
						0,0,
						DisplayRect.right-DisplayRect.left,
						DisplayRect.bottom-DisplayRect.top,
						SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW
					);

		GetClientRect(hDDLibWindow,&the_display.DisplayRect);
	    ClientToScreen(hDDLibWindow,(LPPOINT)&the_display.DisplayRect);
	    ClientToScreen(hDDLibWindow,(LPPOINT)&the_display.DisplayRect+1);

		// Success
		TurnValidFullscreenOn();
		return	DD_OK;
	}


	// Get the shell menu & window style.
	hDDLibMenu		=	GetMenu(hDDLibWindow);
	hDDLibStyle		=	GetWindowLong(hDDLibWindow,GWL_STYLE);
	hDDLibStyleEx	=	GetWindowLong(hDDLibWindow,GWL_EXSTYLE);

#else //#ifndef TARGET_DC
	hDDLibMenu		=	0;
	hDDLibStyle		=	0;
	hDDLibStyleEx	=	0;
#endif //#else //#ifndef TARGET_DC


#ifdef TARGET_DC
	// We know which mode we want, and what to try.

	DisplayRect.left   = 0;
	DisplayRect.top    = 0;
	DisplayRect.right  = 640;
	DisplayRect.bottom = 480;


	// Find out which modes we are allowed to support.
	// This si from the command-line
extern LPSTR lpszGlobalArgs;




	// Er.. just do some non-display-related regional checking here as well.
	BYTE bCountry = FirmwareGetCountryCode();


	if ( NULL != strstr ( lpszGlobalArgs, "EUROPE_ONLY" ) )
	{
		// This is a European build, and should only work there.
		if ( bCountry != COUNTRY_EUROPE )
		{
			TRACE ( "EUROPE_ONLY - rebooting\n" );
			ASSERT ( FALSE );
			// Reset to Boot ROM.
			ResetToFirmware();
		}
	}
	if ( NULL != strstr ( lpszGlobalArgs, "AMERICA_ONLY" ) )
	{
		// This is an American build, and should only work there.
		if ( bCountry != COUNTRY_AMERICA )
		{
			TRACE ( "AMERICA_ONLY - rebooting\n" );
			ASSERT ( FALSE );
			// Reset to Boot ROM.
			ResetToFirmware();
		}
	}




	BYTE bFormat = GetVideoOutputFormat();
	switch ( bFormat )
	{
	case VIDFMT_NTSC_RGB:
	case VIDFMT_NTSC:
		// NTSC 60Hz.

#if 0
		if ( NULL != strstr ( lpszGlobalArgs, "NO_NTSC" ) )
		{
			TRACE ( "NO_NTSC set, and this is an NTSC mode - rebooting\n" );
			ASSERT ( FALSE );
			// Reset to Boot ROM.
			ResetToFirmware();
		}
#endif


		result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 30, 0 );
		if(SUCCEEDED(result))
		{
			eDisplayType = DT_NTSC;
			TurnValidFullscreenOn();
			return result;
		}
		break;

	case VIDFMT_PAL_RGB:
	case VIDFMT_PAL:
	case VIDFMT_PAL_M_RGB:
	case VIDFMT_PAL_M:
	case VIDFMT_PAL_N_RGB:
	case VIDFMT_PAL_N:
		// PAL 50Hz.
		// Try a stretched one.

#if 0
		if ( NULL != strstr ( lpszGlobalArgs, "NO_PAL" ) )
		{
			TRACE ( "NO_PAL set, and this is a PAL mode - rebooting\n" );
			ASSERT ( FALSE );
			// Reset to Boot ROM.
			ResetToFirmware();
		}
#endif

		result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 25, DDSDM_PALHEIGHTRATIO_1_100 );
		if(SUCCEEDED(result))
		{
			eDisplayType = DT_PAL;
			TurnValidFullscreenOn();
			return result;
		}

		// Wacky. Try normal then.
		result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 25, 0 );
		if(SUCCEEDED(result))
		{
			eDisplayType = DT_PAL;
			TurnValidFullscreenOn();
			return result;
		}
		break;

	case VIDFMT_VGA:
		// VGA 60Hz.

#if 0
		if ( NULL != strstr ( lpszGlobalArgs, "NO_VGA" ) )
		{
			TRACE ( "NO_VGA set, and this is a VGA mode - rebooting\n" );
			ASSERT ( FALSE );
			// Reset to Boot ROM.
			ResetToFirmware();
		}
#endif


		result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 60, 0 );
		if(SUCCEEDED(result))
		{
#ifdef DEBUG
			// For testing US settings.
			eDisplayType = DT_NTSC;
#else
			eDisplayType = DT_VGA;
#endif
			TurnValidFullscreenOn();
			return result;
		}
		break;

	default:
		ASSERT ( FALSE );
		break;
	}


	ASSERT ( FALSE );

#if 0
#if 0
	// Bloody thing doesn't work - corrupts my textures and puts fuzz on the screen.

	// Try stretched PAL 50Hz.interlaced
    result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 25, DDSDM_PALHEIGHTRATIO_1_066 );
    if(SUCCEEDED(result))
	{
		TurnValidFullscreenOn();
		return result;
	}
#endif

	// NTSC 60Hz interlaced
    result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 30, 0 );
    if(SUCCEEDED(result))
	{
		TurnValidFullscreenOn();
		return result;
	}

	// Non-stretched PAL 50Hz interlaced
    result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 25, 0 );
    if(SUCCEEDED(result))
	{
		TurnValidFullscreenOn();
		return result;
	}

	// VGA 60Hz
    result	=	lp_DD4->SetDisplayMode ( 640, 480, 16, 60, 0 );
    if(SUCCEEDED(result))
	{
		TurnValidFullscreenOn();
		return result;
	}
#endif

	ASSERT ( FALSE );

#else


	// Calculate Mode info
	CurrMode->GetMode(&w,&h,&bpp,&refresh);

	// Special check for mode 320 x 200 x 8
	if((w==320) && (h==200) && (bpp==8))
    {
		// Make sure we use Mode 13 instead of Mode X
		flags	=	DDSDM_STANDARDVGAMODE;
    }

    // Set Requested Fullscreen mode
    result	=	lp_DD4->SetDisplayMode(w,h,bpp,refresh,flags);
    if(SUCCEEDED(result))
	{
		// Save Surface Rectangle
		DisplayRect.left   = 0;
		DisplayRect.top    = 0;
		DisplayRect.right  = w;
		DisplayRect.bottom = h;

		// Success
		TurnValidFullscreenOn();
		return result;
	}

	DebugText("SetDisplayMode failed (%d x %d x %d), trying (640 x 480 x %d)\n", w, h, bpp, bpp);

	// Don't give up!
	// Try 640 x 480 x bpp mode instead
	if((w!=DEFAULT_WIDTH || h!=DEFAULT_HEIGHT))
    {
		w	=	DEFAULT_WIDTH;
		h	=	DEFAULT_HEIGHT;

		CurrMode	=	ValidateMode(CurrDriver,w,h,bpp,0,CurrDevice);
		if(CurrMode)
		{
			result	=	lp_DD4->SetDisplayMode(w,h,bpp,0,0);
			if(SUCCEEDED(result))
			{
				// Save Surface Rectangle
				DisplayRect.left   = 0;
				DisplayRect.top    = 0;
				DisplayRect.right  = w;
				DisplayRect.bottom = h;

				// Success
				TurnValidFullscreenOn();
				return	result;
			}
		}
	}

	// Keep trying
	// Try 640 x 480 x 16 mode instead
	if(bpp!=DEFAULT_DEPTH)
    {
		DebugText("SetDisplayMode failed (640 x 480 x %d), trying (640 x 480 x 16)\n", bpp);
		bpp	=	DEFAULT_DEPTH;

		CurrMode	=	ValidateMode(CurrDriver,w,h,bpp,0,CurrDevice);
		if(CurrMode)
		{
			result	=	lp_DD4->SetDisplayMode(w,h,bpp,0,0);
			if(SUCCEEDED(result))
			{
				// Save Surface Rectangle
				DisplayRect.left   = 0;
				DisplayRect.top    = 0;
				DisplayRect.right  = w;
				DisplayRect.bottom = h;

				// Success
				TurnValidFullscreenOn();
				return	result;
			}
		}
	}
	// Failure
	// Output error.
	return	result;

#endif

}

//---------------------------------------------------------------

HRESULT	Display::FiniFullscreenMode(void)
{
	TurnValidFullscreenOff ();

	// Restore original desktop mode
	if(lp_DD4)
		lp_DD4->RestoreDisplayMode();

	// Success
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

void calculate_mask_and_shift(
		ULONG  bitmask,
		SLONG *mask,
		SLONG *shift)
{
	SLONG i;
	SLONG b;
	SLONG num_bits  =  0;
	SLONG first_bit = -1;

	LogText(" bitmask %x \n",bitmask);

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

	if (first_bit == -1 ||
		num_bits  ==  0)
	{
		//
		// This is bad!
		//

		LogText(" poo mask shift  first bit %d num_bits %d \n",first_bit,num_bits);
		TRACE("No valid masks and shifts.\n");
		LogText("No valid masks and shifts.\n");

		*mask  = 0;
		*shift = 0;

		return;
	}

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





//---------------------------------------------------------------

HRESULT	Display::InitFront(void)
{
	DDSURFACEDESC2	dd_sd;
	HRESULT			result;


    // Check Initialization
	if((!CurrMode) || (!lp_DD4))
	{
		// Error, Need a valid mode and DD interface to proceed
		result	=	DDERR_GENERIC;
		// Output error.
		return	result;
	}

	// Note:  There is no need to fill in width, height, bpp, etc.
	//        This was taken care of in the SetDisplayMode call.

	// Setup Surfaces caps for a front buffer and back buffer
	InitStruct(dd_sd);

#ifndef TARGET_DC
	if(IsFullScreen() && CurrDevice->IsHardware())
#endif
	{
		//
		// Fullscreen harware.
		//

		dd_sd.dwFlags			=	DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		dd_sd.ddsCaps.dwCaps	=	DDSCAPS_COMPLEX|DDSCAPS_FLIP|DDSCAPS_PRIMARYSURFACE|DDSCAPS_3DDEVICE;
		dd_sd.dwBackBufferCount	=	1;
	}
#ifndef TARGET_DC
	else
	{
		//
		// In a window or software mode.
		//

		dd_sd.dwFlags			=	DDSD_CAPS;
		dd_sd.ddsCaps.dwCaps	=	DDSCAPS_PRIMARYSURFACE;
	}
#endif

	// Create Primary surface
	result	=	lp_DD4->CreateSurface(&dd_sd,&lp_DD_FrontSurface,NULL);
	if(FAILED(result))
	{
		// Error
		DebugText("InitFront: unable to create front surface result %d\n",result);
		return	result;
	}

#ifndef TARGET_DC
	// Create and attach palette, if necessary.
	result	=	InitPalette();
	if(FAILED(result))
		return	result;
#endif

#ifndef TARGET_DC
	if (!the_display.IsFullScreen())
	{
		// Create and attach clipper, if necessary.
		result	=	InitClipper();
		if(FAILED(result))
			return	result;
	}
#endif

	// No gamma control on DC.
#ifndef TARGET_DC
	// create gamma control
	result = lp_DD_FrontSurface->QueryInterface(IID_IDirectDrawGammaControl, (void**)&lp_DD_GammaControl);
	if (FAILED(result))
	{
		lp_DD_GammaControl = NULL;
		TRACE("No gamma\n");
	}
	else
	{
		TRACE("Gamma control OK\n");

		int	black, white;

		GetGamma(&black, &white);
		SetGamma(black, white);
	}
#endif

	// Mark as Valid
	TurnValidFrontOn();

	//
	// We need to work out the pixel format of the front buffer.
	//

	InitStruct(dd_sd);

	lp_DD_FrontSurface->GetSurfaceDesc(&dd_sd);

	//
	// It must be an RGB mode!
	//

	ASSERT(dd_sd.ddpfPixelFormat.dwFlags & DDPF_RGB);

	//
	// Work out the masks and shifts for each colour component.
	//

	calculate_mask_and_shift(dd_sd.ddpfPixelFormat.dwRBitMask, &mask_red,   &shift_red);
	calculate_mask_and_shift(dd_sd.ddpfPixelFormat.dwGBitMask, &mask_green, &shift_green);
	calculate_mask_and_shift(dd_sd.ddpfPixelFormat.dwBBitMask, &mask_blue,  &shift_blue);

	LogText(" mask red %x shift red %d \n",mask_red,shift_red);
	LogText(" mask green %x shift green %d \n",mask_green,shift_green);
	LogText(" mask blue %x shift blue %d \n",mask_blue,shift_blue);

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::FiniFront(void)
{
	// Mark as Invalid
	TurnValidFrontOff();

	// Cleanup clipper.
	FiniClipper();

	// Cleanup palette.
	FiniPalette();

	// release gamma control
	if (lp_DD_GammaControl)
	{
		lp_DD_GammaControl->Release();
		lp_DD_GammaControl = NULL;
	}

	// Release Front Surface Object
	if(lp_DD_FrontSurface)
	{
		lp_DD_FrontSurface->Release();
		lp_DD_FrontSurface	=	NULL;
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::InitPalette(void)
{
	SLONG				c0,
						flags,
						pal_mem_size;
    DDSURFACEDESC2		dd_sd;
	HDC					hdc;
	HRESULT				result;


    // Destroy old palette
    FiniPalette();

    // Make sure we are properly intialized
    // for this to work
	if((!lp_DD4) || (!lp_DD_FrontSurface))
	{
        // Error, need a valid DD interfac and a primary surface to continue
		result	=	DDERR_GENERIC;
		// Output error.
		return	result;
	}

    // Get primary surface caps
	InitStruct(dd_sd);

	result	=	lp_DD_FrontSurface->GetSurfaceDesc(&dd_sd);
	if(FAILED(result))
	{
        // Error
		DebugText("InitPalette: unable to get FrontSurface description");
        return	result;
	}

    // Make sure it is a palettized surface
	if(!IsPalettized(&(dd_sd.ddpfPixelFormat)))
	{
        return DD_OK;
	}

	// Create and save System palette
	hdc	=	GetDC(NULL);
	PaletteSize	=	GetDeviceCaps(hdc,SIZEPALETTE);
	if(PaletteSize)
	{
		if(PaletteSize>256)
			PaletteSize	=	256;

        // Get memory for system palette
		lp_SysPalette	=	new PALETTEENTRY[PaletteSize];
		if(!lp_SysPalette)
		{
			ReleaseDC (NULL, hdc);

			// Error, not enough memory
			result	=	DDERR_GENERIC;
			DebugText("InitPalette: not enough memory for storing palette\n");
			goto	cleanup;
		}

		// Get Memory for current palette
		lp_CurrPalette	=	new	PALETTEENTRY[PaletteSize];
		if(!lp_CurrPalette)
		{
			ReleaseDC(NULL,hdc);

			// Error, not enough memory
			result	=	DDERR_GENERIC;
			DebugText("InitPalette: not enough memory for storing palette\n");
			goto	cleanup;
		}

		// Save system palette
		GetSystemPaletteEntries(hdc,0,PaletteSize,lp_SysPalette);

		// Copy system palette to temporary values
		pal_mem_size	=	PaletteSize*sizeof (PALETTEENTRY);
		CopyMemory(lp_CurrPalette,lp_SysPalette,pal_mem_size);
    }
	ReleaseDC(NULL,hdc);

	if(dd_sd.ddpfPixelFormat.dwFlags&DDPF_PALETTEINDEXED1)
	{
		flags	=	DDPCAPS_1BIT;

		// Bagsy the whole palette.
		for(c0= 0;c0<2;c0++)
			lp_CurrPalette[c0].peFlags	=	D3DPAL_FREE|PC_RESERVED;
	}
	else if(dd_sd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
	{
		flags = DDPCAPS_2BIT;

		// Bagsy the whole palette.
		for(c0=0;c0<4;c0++)
			lp_CurrPalette[c0].peFlags	=	D3DPAL_FREE|PC_RESERVED;
    }
	else if(dd_sd.ddpfPixelFormat.dwFlags&DDPF_PALETTEINDEXED4)
    {
		flags	=	DDPCAPS_4BIT;

		// Bagsy the whole palette except first & last colours.
		lp_CurrPalette[0].peFlags	=	D3DPAL_READONLY;
		lp_CurrPalette[15].peFlags	=	D3DPAL_READONLY;

		for(c0=1;c0<15;c0++)
			lp_CurrPalette[c0].peFlags	=	D3DPAL_FREE|PC_RESERVED;
    }
	else if(dd_sd.ddpfPixelFormat.dwFlags&DDPF_PALETTEINDEXED8)
    {
		flags	=	DDPCAPS_8BIT;

		// Bagsy the whole palette.
		lp_CurrPalette[0].peFlags	=	D3DPAL_READONLY;
		lp_CurrPalette[255].peFlags	=	D3DPAL_READONLY;

		for(c0=1;c0<254;c0++)
			lp_CurrPalette[c0].peFlags	=	D3DPAL_FREE|PC_RESERVED;
    }
    else
    {
		result	=	DDERR_GENERIC;
		DebugText("InitPalette:  unknown palette type\n");
		goto	cleanup;
    }

    // Create Primary Palette
	result	=	lp_DD4->CreatePalette	(
											flags,
											lp_CurrPalette,
											&lp_DD_Palette,
											NULL
										);
	if(FAILED(result))
	{
		DebugText("InitPalette: unable to create palette\n");
		goto	cleanup;
	}

	// Attach palette to primary surface
	result	=	lp_DD_FrontSurface->SetPalette(lp_DD_Palette);
	if(FAILED(result))
	{
        // Error
		DebugText("InitPalette: unable to attach palette to FrontSurface\n");
		goto	cleanup;
	}

    // Success
    return DD_OK;

cleanup:

	// Cleanup
	FiniPalette();
	return	result;
}

//---------------------------------------------------------------

HRESULT	Display::FiniPalette(void)
{
    // Clean up DD Palette object.
	if(lp_DD_Palette)
	{
		lp_DD_Palette->Release();
		lp_DD_Palette	=	NULL;
	}

	// Cleanup Current Palette
	if(lp_CurrPalette)
	{
		delete	[]lp_CurrPalette;
		lp_CurrPalette	=	NULL;
	}

    // Cleanup System Palette
    if(lp_SysPalette)
    {
        // Note:  Should we try and restore system palette here ?!?

        // Destroy system palette
        delete	[]lp_SysPalette;
        lp_SysPalette	=	NULL;
    }

	PaletteSize	=	0;

	// Success
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::InitBack(void)
{
	SLONG			mem_type,
					w,h;
	DDSCAPS2		dd_scaps;
	DDSURFACEDESC2  dd_sd;
	HRESULT			result;
	LPD3DDEVICEDESC	device_desc;


	// Check Initialization
	if	(
			(!hDDLibWindow) || (!IsWindow(hDDLibWindow)) ||
			(!CurrDevice) || (!CurrMode) ||
			(!lp_DD4) || (!lp_D3D) || (!lp_DD_FrontSurface)
		)
	{
        // Error, Not initialized properly before calling this method
		result	=	DDERR_GENERIC;
		DebugText("InitBack: invalid initialisation\n");
		return	result;
	}

	// Calculate the width & height.  This is useful for windowed mode & the z buffer.
	w		=	abs(DisplayRect.right-DisplayRect.left);
	h		=	abs(DisplayRect.bottom-DisplayRect.top);

	if(IsFullScreen() && CurrDevice->IsHardware())
	{
		// Get the back surface from the front surface.
		memset(&dd_scaps, 0, sizeof(dd_scaps));
		dd_scaps.dwCaps	=	DDSCAPS_BACKBUFFER;
		result			=	lp_DD_FrontSurface->GetAttachedSurface(&dd_scaps,&lp_DD_BackSurface);
		if(FAILED(result))
		{
			DebugText("InitBack: no attached surface\n");
			return	result;
		}
	}
	else
	{
		// Create the back surface.
		InitStruct(dd_sd);
		dd_sd.dwFlags		=	DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		dd_sd.dwWidth		=	w;
		dd_sd.dwHeight		=	h;
		dd_sd.ddsCaps.dwCaps=	DDSCAPS_OFFSCREENPLAIN|DDSCAPS_3DDEVICE;

		if (!CurrDevice->IsHardware())
		{
			dd_sd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}

		result				=	lp_DD4->CreateSurface(&dd_sd,&lp_DD_BackSurface,NULL);
		if(FAILED(result))
		{
			DebugText("InitBack: unable to create back surface\n");
			return	result;
		}
	}

	if(IsUse3D())
	{
		// Create and attach Z-buffer (optional)

		// Get D3D Device description
		if(CurrDevice->IsHardware())
		{
			// Hardware device - Z buffer on video ram.
			device_desc	=	&(CurrDevice->d3dHalDesc);
			mem_type	=	DDSCAPS_VIDEOMEMORY;
		}
		else
		{
			// Software device - Z buffer in system ram.
			device_desc	=	&(CurrDevice->d3dHelDesc);
			mem_type	=	DDSCAPS_SYSTEMMEMORY;
		}

		// Enumerate all Z formats associated with this D3D device
		result	=	CurrDevice->LoadZFormats(lp_D3D);
		if(FAILED(result))
		{
			// Error, no texture formats
			// Hope we can run OK without textures
			DebugText("InitBack: unable to load Z formats\n");
		}

#ifdef TARGET_DC
		// No real Z buffer - don't bother
#else
		// Can we create a Z buffer?
		if(IsCreateZBuffer() && device_desc && device_desc->dwDeviceZBufferBitDepth)
		{
			// Create the z-buffer.
			InitStruct(dd_sd);
#if 0
			dd_sd.dwFlags			=	DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_ZBUFFERBITDEPTH;
			dd_sd.ddsCaps.dwCaps	=	DDSCAPS_ZBUFFER | mem_type;
			dd_sd.dwWidth			=	w;
			dd_sd.dwHeight			=	h;
			dd_sd.dwZBufferBitDepth	=	FlagsToBitDepth(device_desc->dwDeviceZBufferBitDepth);
#else
			dd_sd.dwFlags			=	DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
			dd_sd.ddsCaps.dwCaps	=	DDSCAPS_ZBUFFER | mem_type;
			dd_sd.dwWidth			=	w;
			dd_sd.dwHeight			=	h;
			memcpy(&dd_sd.ddpfPixelFormat, CurrDevice->GetZFormat(), sizeof(DDPIXELFORMAT));
#endif
			result	=	lp_DD4->CreateSurface(&dd_sd,&lp_DD_ZBuffer,NULL);
			if(FAILED(result))
			{
				DebugText("InitBack: unable to create ZBuffer\n");
				dd_error(result);

				// Note: we may be able to continue without a z buffer
				// So don't exit
			}
			else
			{
				// Attach Z buffer to back surface.
				result	=	lp_DD_BackSurface->AddAttachedSurface(lp_DD_ZBuffer);
				if(FAILED(result))
				{
					DebugText("InitBack: unable to attach ZBuffer 1\n");

					if(lp_DD_ZBuffer)
					{
						lp_DD_ZBuffer->Release();
						lp_DD_ZBuffer	=	NULL;
					}

					// Note: we may be able to continue without a z buffer
					// So don't exit
				}
			}
		}
#endif

		//	Create the D3D device interface
		result	=	lp_D3D->CreateDevice(CurrDevice->guid,lp_DD_BackSurface,&lp_D3D_Device,NULL);
		if(FAILED(result))
		{
			DebugText("InitBack: unable to create D3D device\n");
			d3d_error(result);
			return	result;
		}

		// Enumerate all Texture formats associated with this D3D device
		result	=	CurrDevice->LoadFormats(lp_D3D_Device);
		if(FAILED(result))
		{
			// Error, no texture formats
			// Hope we can run OK without textures
			DebugText("InitBack: unable to load texture formats\n");
		}

		//	Create the viewport
		result	=	InitViewport();
		if(FAILED(result))
		{
			DebugText("InitBack: unable to init viewport\n");
			return	result;
		}

		// check the device caps
		CurrDevice->CheckCaps(lp_D3D_Device);


	}

#ifndef TARGET_DC
	if(IsUseWork())
	{
		// Create a work screen for user access.

		// Get D3D Device description.  We want a system ram surface regardless of the device type.
		if(CurrDevice->IsHardware())
		{
			// Hardware device - Z buffer on video ram.
			device_desc	=	&(CurrDevice->d3dHalDesc);
		}
		else
		{
			// Software device - Z buffer in system ram.
			device_desc	=	&(CurrDevice->d3dHelDesc);
		}

		// Can we create the surface?
		if(device_desc)
		{
			// Create the z-buffer.
			InitStruct(dd_sd);
			dd_sd.dwFlags			=	DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
			dd_sd.ddsCaps.dwCaps	=	DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
			dd_sd.dwWidth			=	w;
			dd_sd.dwHeight			=	h;
			result	=	lp_DD4->CreateSurface(&dd_sd,&lp_DD_WorkSurface,NULL);
			if(FAILED(result))
			{
				DebugText("InitBack: unable to create work surface\n");
				dd_error(result);
				return	result;
			}
			WorkScreenPixelWidth	=	w;
			WorkScreenWidth			=	w;
			WorkScreenHeight		=	h;
		}

	}
#endif

	// Mark as valid
	TurnValidBackOn();


#ifdef TARGET_DC
	InitBackCache();
#endif

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

#ifdef TARGET_DC

static bool bBackCacheInit = FALSE;

// Creates the background texture caches, which are only used in the frontend.
HRESULT	Display::InitBackCache(void)
{
	if ( bBackCacheInit )
	{
		// Already done.
		return DD_OK;
	}

	// Reset the cache.
	CompressedBackground m_lpLastBackground = NULL;


	ASSERT ( lpBackgroundCache == NULL );
	ASSERT ( the_display.lp_DD_Background_use_instead_texture == NULL );
	DDSURFACEDESC2 ddsd;
	DDSURFACEDESC2 back;
	HRESULT result;
	InitStruct(back);
	InitStruct(ddsd);

	lp_DD_BackSurface->GetSurfaceDesc(&back);

	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.dwWidth = 1;
	while ( ddsd.dwWidth < back.dwWidth )
	{
		ddsd.dwWidth <<= 1;
	}
	ddsd.dwHeight = 1;
	while ( ddsd.dwHeight < back.dwHeight )
	{
		ddsd.dwHeight <<= 1;
	}
	ddsd.ddpfPixelFormat = back.ddpfPixelFormat;
	ddsd.ddsCaps.dwCaps	 = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0;


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

	result = lp_DD4->CreateSurface(&ddsd, &lpBackgroundCache, NULL);
	if (FAILED(result))
	{
		ASSERT ( FALSE );
		lpBackgroundCache = NULL;
		return DDERR_OUTOFMEMORY;
	}
	//VERIFY(SUCCEEDED(lpBackgroundCache->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lpBackgroundCacheTexture)));
	VERIFY(SUCCEEDED(lpBackgroundCache->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&the_display.lp_DD_Background_use_instead_texture)));


	// ...and another one.
	ASSERT ( lpBackgroundCache2 == NULL );
	ASSERT ( the_display.lp_DD_Background_use_instead_texture2 == NULL );
	InitStruct(back);
	InitStruct(ddsd);

	lp_DD_BackSurface->GetSurfaceDesc(&back);

	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.dwWidth = 1;
	while ( ddsd.dwWidth < back.dwWidth )
	{
		ddsd.dwWidth <<= 1;
	}
	ddsd.dwHeight = 1;
	while ( ddsd.dwHeight < back.dwHeight )
	{
		ddsd.dwHeight <<= 1;
	}
	ddsd.ddpfPixelFormat = back.ddpfPixelFormat;
	ddsd.ddsCaps.dwCaps	 = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0;


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


	result = lp_DD4->CreateSurface(&ddsd, &lpBackgroundCache2, NULL);
#ifdef DEBUG
	if ( result == DDERR_OUTOFMEMORY )
	{
		HRESULT hres = lp_DD4->EnumSurfaces ( DDENUMSURFACES_DOESEXIST | DDENUMSURFACES_ALL, NULL, NULL, &EnumSurfacesCallbackFunc );

		ASSERT ( FALSE );
	}
#endif
	if (FAILED(result))
	{
		ASSERT ( FALSE );
		lpBackgroundCache2 = NULL;
		return DDERR_OUTOFMEMORY;
	}
	//VERIFY(SUCCEEDED(lpBackgroundCache->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&lpBackgroundCacheTexture)));
	VERIFY(SUCCEEDED(lpBackgroundCache2->QueryInterface(IID_IDirect3DTexture2,(LPVOID *)&the_display.lp_DD_Background_use_instead_texture2)));


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


	bBackCacheInit = TRUE;

	return ( DD_OK );
}


// Bins the background textures, which are only used in the frontend.
HRESULT	Display::FiniBackCache(void)
{
	if ( !bBackCacheInit )
	{
		// Already done.
		return DD_OK;
	}

	// Remember to release the textures.
	if ( the_display.lp_D3D_Device != NULL )
	{
		the_display.lp_D3D_Device->SetTexture ( 0, NULL );
		the_display.lp_D3D_Device->SetTexture ( 1, NULL );
		the_display.lp_D3D_Device->SetTexture ( 2, NULL );
		the_display.lp_D3D_Device->SetTexture ( 3, NULL );
	}

	if ( the_display.lp_DD_Background_use_instead_texture != NULL )
	{
		int res = the_display.lp_DD_Background_use_instead_texture->Release();
		ASSERT ( res == 1 );
		the_display.lp_DD_Background_use_instead_texture = NULL;
	}
	if ( lpBackgroundCache != NULL )
	{
		int res = lpBackgroundCache->Release();
		ASSERT ( res == 0 );
		lpBackgroundCache = NULL;
	}
	if ( the_display.lp_DD_Background_use_instead_texture2 != NULL )
	{
		int res = the_display.lp_DD_Background_use_instead_texture2->Release();
		ASSERT ( res == 1 );
		the_display.lp_DD_Background_use_instead_texture2 = NULL;
	}
	if ( lpBackgroundCache2 != NULL )
	{
		int res = lpBackgroundCache2->Release();
		ASSERT ( res == 0 );
		lpBackgroundCache2 = NULL;
	}

	bBackCacheInit = FALSE;

	return ( DD_OK );
}
#endif



HRESULT	Display::FiniBack(void)
{
#ifdef TARGET_DC
	// Clean up the background texture.
	FiniBackCache();
#endif

	// Mark as invalid
	TurnValidBackOff();

#ifndef TARGET_DC
	// Clean up the work screen stuff.
	if(IsUseWork())
	{
		// Release the work surface.
		if(lp_DD_WorkSurface)
		{
			lp_DD_WorkSurface->Release();
			lp_DD_WorkSurface	=	NULL;
		}
	}
#endif

	// Clean up the D3D stuff.
	if(IsUse3D())
	{
		// Cleanup viewport
		FiniViewport();

		// Release D3D Device
		if(lp_D3D_Device)
		{
			lp_D3D_Device->Release();
			lp_D3D_Device	=	NULL;
		}

		// Release Z Buffer
		if(lp_DD_ZBuffer)
		{
			// Detach Z-Buffer from back buffer
			if(lp_DD_BackSurface)
				lp_DD_BackSurface->DeleteAttachedSurface(0L,lp_DD_ZBuffer);

			// Release Z-Buffer
			lp_DD_ZBuffer->Release();
			lp_DD_ZBuffer	=	NULL;
		}
	}

	// Release back surface.
	if(lp_DD_BackSurface)
	{
		lp_DD_BackSurface->Release();
		lp_DD_BackSurface	=	NULL;
	}

	// Success
	return DD_OK;
}


//---------------------------------------------------------------

HRESULT	Display::InitClipper(void)
{
	HRESULT			result;


    // Check Initialization
	if((!CurrMode) || (!lp_DD4))
	{
		result	=	DDERR_GENERIC;
		DebugText("InitClipper: invalid initialisation\n");
		return	result;
	}

#ifndef TARGET_DC

	result	=	lp_DD4->CreateClipper(0,&lp_DD_Clipper,NULL);
	if(FAILED(result))
	{
		DebugText("InitClipper: unable to create clipper\n");
		dd_error(result);
		DebugText("\n");
		return	result;
	}

	// Set clipper window handle.
	result	=	lp_DD_Clipper->SetHWnd(0,hDDLibWindow);
	if(FAILED(result))
	{
		DebugText("InitClipper: unable to set window handle\n");
		dd_error(result);
		DebugText("\n");
		return	result;
	}

	// Attach clipper to front surface.
	result	=	lp_DD_FrontSurface->SetClipper(lp_DD_Clipper);
	if(FAILED(result))
	{
		DebugText("InitClipper: unable to attach clipper\n");
		dd_error(result);
		DebugText("\n");
		return	result;
	}
#else
	lp_DD_Clipper = 0;
#endif

	// Mark as Valid
	TurnValidClipperOn();

	// Success
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::FiniClipper(void)
{
	// Mark as Invalid
	TurnValidClipperOff();

#ifndef TARGET_DC
	// Release Primary Surface Object
	if(lp_DD_Clipper)
	{
		lp_DD_Clipper->Release();
		lp_DD_Clipper	=	NULL;
	}
#endif

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::InitViewport(void)
{
	D3DMATERIAL		material;
	HRESULT			result;

	// Check Initialization
	if((!lp_D3D) || (!lp_D3D_Device))
	{
		// Error, Not properly initialized before calling this method
		result	=	DDERR_GENERIC;
		// Output error.
		return	result;
	}

	// Create Viewport
	result	=	lp_D3D->CreateViewport(&lp_D3D_Viewport,NULL);
    if(FAILED(result))
    {
		// Output error.
		return	result;
	}

	// Attach viewport to D3D device
	result	=	lp_D3D_Device->AddViewport(lp_D3D_Viewport);
	if(FAILED(result))
    {
		lp_D3D_Viewport->Release();
		lp_D3D_Viewport	=	NULL;

        // Output error.
		return	result;
    }

	// Black material.
	result	=	lp_D3D->CreateMaterial(&lp_D3D_Black,NULL);
	if(FAILED(result))
	{
		DebugText("InitViewport: Error creating black material\n");
		return	result;
	}

	InitStruct(material);
	material.diffuse.r	=	D3DVAL(0.0);
	material.diffuse.g	=	D3DVAL(0.0);
	material.diffuse.b	=	D3DVAL(0.0);
	material.diffuse.a	=	D3DVAL(1.0);
	material.ambient.r	=	D3DVAL(0.0);
	material.ambient.g	=	D3DVAL(0.0);
	material.ambient.b	=	D3DVAL(0.0);
	material.dwRampSize	=	0;

	result	=	lp_D3D_Black->SetMaterial(&material);
	if(FAILED(result))
	{
		DebugText("InitViewport: Error setting black material\n");
		lp_D3D_Black->Release();
		lp_D3D_Black	=	NULL;
		return	result;
	}
	result	=	lp_D3D_Black->GetHandle(lp_D3D_Device,&black_handle);
	if(FAILED(result))
	{
		DebugText("InitViewport: Error getting black handle\n");
		lp_D3D_Black->Release();
		lp_D3D_Black	=	NULL;
		return	result;
	}

	// White material.
	result	=	lp_D3D->CreateMaterial(&lp_D3D_White,NULL);
	if(FAILED(result))
	{
		DebugText("InitViewport: Error creating white material\n");
		return	result;
	}
	material.diffuse.r	=	D3DVAL(1.0);
	material.diffuse.g	=	D3DVAL(1.0);
	material.diffuse.b	=	D3DVAL(1.0);
	material.diffuse.a	=	D3DVAL(1.0);
	material.ambient.r	=	D3DVAL(1.0);
	material.ambient.g	=	D3DVAL(1.0);
	material.ambient.b	=	D3DVAL(1.0);
	material.dwRampSize	=	0;

	result	=	lp_D3D_White->SetMaterial(&material);
	if(FAILED(result))
	{
		DebugText("InitViewport: Error setting white material\n");
		lp_D3D_White->Release();
		lp_D3D_White	=	NULL;
		return	result;
	}
	result	=	lp_D3D_White->GetHandle(lp_D3D_Device,&white_handle);
	if(FAILED(result))
	{
		DebugText("InitViewport: Error getting white handle\n");
		lp_D3D_White->Release();
		lp_D3D_White	=	NULL;
		return	result;
	}

	//
	// User material.
	//

	SetUserColour(255, 150, 255);

	// Set up Initial Viewport parameters
	result	=	UpdateViewport();
	if(FAILED(result))
	{
		lp_D3D_Device->DeleteViewport(lp_D3D_Viewport);
		lp_D3D_Viewport->Release();
		lp_D3D_Viewport	=	NULL;

		return	result;
	}

	// Mark as valid
	TurnValidViewportOn();

	/// Success
	return DD_OK;
}


//---------------------------------------------------------------

#ifndef TARGET_DC

void Display::SetUserColour(UBYTE red, UBYTE green, UBYTE blue)
{
	D3DMATERIAL		material;
	HRESULT			result;

	float r = (1.0F / 255.0F) * float(red);
	float g = (1.0F / 255.0F) * float(green);
	float b = (1.0F / 255.0F) * float(blue);

	if (lp_D3D_User)
	{
		lp_D3D_User->Release();
		lp_D3D_User	=	NULL;
		user_handle =   NULL;
	}

	result = lp_D3D->CreateMaterial(&lp_D3D_User,NULL);

	ASSERT(!FAILED(result));

	InitStruct(material);
	material.diffuse.r	=	D3DVAL(r);
	material.diffuse.g	=	D3DVAL(g);
	material.diffuse.b	=	D3DVAL(b);
	material.diffuse.a	=	D3DVAL(1.0);
	material.ambient.r	=	D3DVAL(r);
	material.ambient.g	=	D3DVAL(g);
	material.ambient.b	=	D3DVAL(b);
	material.dwRampSize	=	0;

	result = lp_D3D_User->SetMaterial(&material);

	ASSERT(!FAILED(result));

	result = lp_D3D_User->GetHandle(lp_D3D_Device,&user_handle);

	ASSERT(!FAILED(result));
}

#endif //#ifndef TARGET_DC


//---------------------------------------------------------------

HRESULT	Display::FiniViewport(void)
{
	// Mark as invalid
	TurnValidViewportOff();

	// Get rid of any loaded texture maps.
	FreeLoadedTextures();

	// Release materials.
	if(lp_D3D_Black)
	{
		lp_D3D_Black->Release();
		lp_D3D_Black	=	NULL;
		black_handle    =   NULL;
	}

	if(lp_D3D_White)
	{
		lp_D3D_White->Release();
		lp_D3D_White	=	NULL;
		white_handle    =   NULL;
	}

	if (lp_D3D_User)
	{
		lp_D3D_User->Release();
		lp_D3D_User		=	NULL;
		user_handle     =   NULL;
	}

	// Release D3D viewport
	if(lp_D3D_Viewport)
	{
		lp_D3D_Device->DeleteViewport(lp_D3D_Viewport);
        lp_D3D_Viewport->Release();
        lp_D3D_Viewport	=	NULL;
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::UpdateViewport(void)
{
	SLONG			s_w,s_h;
	HRESULT			result;
    D3DVIEWPORT2	d3d_viewport;


	// Check Parameters
	if((!lp_D3D_Device) || (!lp_D3D_Viewport))
	{
		// Not properly initialized before calling this method
		result	=	DDERR_GENERIC;
		// Output error.
		return	result;
	}

	// Get Surface Width and Height
	s_w	=	abs(DisplayRect.right - DisplayRect.left);
	s_h	=	abs(DisplayRect.bottom - DisplayRect.top);

    // Update Viewport
	InitStruct(d3d_viewport);
	d3d_viewport.dwX			=	0;
	d3d_viewport.dwY			=	0;
	d3d_viewport.dwWidth		=	s_w;
	d3d_viewport.dwHeight		=	s_h;
	d3d_viewport.dvClipX		=	0.0f;
	d3d_viewport.dvClipY		=	0.0f;
	d3d_viewport.dvClipWidth	=	(float)s_w;
	d3d_viewport.dvClipHeight	=	(float)s_h;
	d3d_viewport.dvMinZ			=	1.0f;
	d3d_viewport.dvMaxZ			=	0.0f;

	// Update Viewport
	result	=	lp_D3D_Viewport->SetViewport2(&d3d_viewport);
	if(FAILED(result))
	{
		// Output error.
		return	result;
	}

	// Update D3D device to use this viewport
	result	=	lp_D3D_Device->SetCurrentViewport(lp_D3D_Viewport);
	if(FAILED(result))
    {
		// Output error.
		return	result;
    }

	// Reload any pre-loaded textures.
	ReloadTextures();

	// Set the view port rect.
	ViewportRect.x1	=	0;
	ViewportRect.y1	=	0;
	ViewportRect.x2	=	s_w;
	ViewportRect.y2	=	s_h;

	// Set the background colour.
	switch(the_display.BackColour)
	{
		case	BK_COL_NONE:
			break;
		case	BK_COL_BLACK:	return	the_display.SetBlackBackground();
		case	BK_COL_WHITE:	return	the_display.SetWhiteBackground();
		case	BK_COL_USER:	return	the_display.SetUserBackground();
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::ChangeDriver	(
									GUID			*DD_guid,
									D3DDeviceInfo	*device_hint,
									DDModeInfo		*mode_hint
								)
{
	SLONG			w,h,bpp,refresh;
	D3DDeviceInfo	*new_device,
					*old_device;
	DDDriverInfo	*new_driver,
					*old_driver;
	DDModeInfo		*new_mode,
					*old_mode;
	GUID			*D3D_guid;
	HRESULT			result;

	DebugText("\nInto change driver\n");

	// Get New Driver
	new_driver	=	ValidateDriver(DD_guid);
	if(!new_driver)
	{
		// Error, invalid DD Guid
		result	=	DDERR_GENERIC;
		DebugText("ChangeDriver: invalid DD GUID\n");
        return	result;
	}

	// Get requested D3D device
	if(device_hint)
		D3D_guid	=	&(device_hint->guid);
	else if(CurrDevice)
		D3D_guid	=	&(CurrDevice->guid);
	else
		D3D_guid	=	NULL;

	// Get requested mode
	if(mode_hint)
		mode_hint->GetMode(&w,&h,&bpp,&refresh);
	else
	{
		// Default to 640 x 480 x 16
		w		=	DEFAULT_WIDTH;
		h		=	DEFAULT_HEIGHT;
		bpp		=	DEFAULT_DEPTH;
		refresh	=	0;
	}

	// Get new device and mode compatible with this driver
	if(!GetFullscreenMode(new_driver,D3D_guid,w,h,bpp,refresh,&new_mode,&new_device))
	{
		result	=	DDERR_GENERIC;
		DebugText("ChangeDriver: unable to find a valid D3D device & mode for this driver");
        return	result;
	}

	// Save old defaults
	old_driver	=	CurrDriver;
	old_mode	=	CurrMode;
	old_device	=	CurrDevice;

	// Destroy almost everything
	DebugText("Into Fini\n");
	Fini ();
	DebugText("Out of Fini\n");

	// Set new defaults
	CurrDriver	=	new_driver;
	CurrMode	=	new_mode;
	CurrDevice	=	new_device;

	// Re-create almost everything based on new driver, device, and mode
	DebugText("Into Init\n");
	result	=	Init();
	DebugText("Out of Init\n");
	if(FAILED(result))
	{
		// Try to restore old defaults
		Fini ();

		CurrDriver	=	old_driver;
		CurrMode	=	old_mode;
		CurrDevice	=	old_device;

		Init ();
		return result;
	}

//	// Notify the window of a successful change in Driver
//	SendMessage(hWindow, D3DWIN_CHANGED_DRIVER, 0, 0);

	// Notify a display change.
	DisplayChangedOn();

	// Success
    return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::ChangeDevice	(
									GUID		*D3D_guid,
									DDModeInfo	*mode_hint
								)
{
	SLONG			w,h,bpp,refresh;
	D3DDeviceInfo	*new_device,
					*old_device;
	DDDriverInfo	*old_driver;
	DDModeInfo	    *new_mode,
					*old_mode,
					*next_best_mode;
    HRESULT         result;

	// Check Parameters
	if(!D3D_guid)
	{
		result	=	DDERR_GENERIC;
		DebugText("Changedevice: invalid parameters\n");
        return	result;
	}

	// Check Initialization
	if(!IsValid() || (!lp_DD_BackSurface))
	{
		result	=	DDERR_GENERIC;
		DebugText("Changedevice: invalid initialization\n");
        return	result;
	}

	// Save Original State
	old_driver	=	CurrDriver;
	old_mode	=	CurrMode;
	old_device	=	CurrDevice;

	// Verify new D3D device belongs to current DD driver
	new_device	=	old_driver->FindDevice(D3D_guid, NULL);
	if(!new_device)
	{
		result	=	DDERR_GENERIC;
		// Output error.
        return	result;
	}

	//
	//	Step 1. Verify new D3D device is supported with current mode
	//
	if(mode_hint)
		new_mode	=	mode_hint;
	else
		new_mode	=	old_mode;
	if(!new_mode->ModeSupported(new_device))
	{
		// We are a full screen app, so we can do what we want
		// Pick a new mode that is compatible with this device
		new_mode->GetMode(&w,&h,&bpp,&refresh);

		new_mode	=	old_driver->FindModeSupportsDevice(w,h,bpp,0,new_device,&next_best_mode);
		if(!new_mode)
		{
			if(!next_best_mode)
			{
				// Error , no compatible mode found!!!
				result	=	DDERR_GENERIC;
				// Output error.
				return	result;
			}
			new_mode	=	next_best_mode;
		}
	}
	if(new_mode==old_mode)
		new_mode	=	NULL;


	//
	//	Step 2.  Destroy Old D3D Device (and mode)
	//
	FiniBack();
	if(new_mode)
	{
		FiniFront();
	}
	VB_Term();

	//
	//	Step 3. Create new D3D Device (new mode optional)
	//

	// Set new D3D device (and mode)
	if(new_mode)
		CurrMode	=	new_mode;
	CurrDevice	=	new_device;

	// Create new mode, if necessary
	if(new_mode)
	{
		// Change Mode
		result	=	InitFullscreenMode();
		if(FAILED(result))
		{
			// Try to restore original mode and device
			CurrDevice	=	old_device;
			CurrMode	=	old_mode;

			InitFullscreenMode();
			InitFront();
			InitBack();
			VB_Init();
			TheVPool->Create(lp_D3D, !CurrDevice->IsHardware());

			return	result;
		}

		// Create Primary
		result	=	InitFront();
		if(FAILED(result))
		{
			// Try to restore original mode and device
			CurrDevice	=	old_device;
			CurrMode	=	old_mode;

			InitFullscreenMode();
			InitFront();
			InitBack();
			VB_Init();
			TheVPool->Create(lp_D3D, !CurrDevice->IsHardware());

			return	result;
		}
	}

	// Create new D3D Device
	result	=	InitBack();
	if(FAILED(result))
	{
		// Try to restore original mode and device
		if(new_mode)
			CurrMode	=	old_mode;
		CurrDevice		=	old_device;

		if(new_mode)
		{
			FiniFront();
			InitFullscreenMode();
			InitFront();
			VB_Init();
			TheVPool->Create(lp_D3D, !CurrDevice->IsHardware());
		}

		result	=	InitBack();
		if(FAILED(result))
		{
			// Return Error
			// Output result.
			return	result;
		}
	}

	VB_Init();
	TheVPool->Create(lp_D3D, !CurrDevice->IsHardware());

	// Notify a display change.
	DisplayChangedOn();

    // Success
    return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::ChangeMode	(
								SLONG	w,
								SLONG	h,
								SLONG	bpp,
								SLONG	refresh
							)
{
	HRESULT			result;
	DDDriverInfo	*old_driver;
	DDModeInfo		*new_mode,
					*old_mode;
	D3DDeviceInfo	*new_device,
					*next_best_device,
					*old_device;


	// Check Initialization
	if((!hDDLibWindow) || (!IsWindow(hDDLibWindow)))
	{
		result	=	DDERR_GENERIC;
		DebugText("ChangeMode: main window not initialised\n");
        return	result;
	}

	if(!IsInitialised())
	{
		result	=	GenerateDefaults();
		if(FAILED(result))
		{
			result	=	DDERR_GENERIC;
			// Output error.
			return	result;
		}

		result	=	Init();
		if(FAILED(result))
		{
			result	=	DDERR_GENERIC;
			// Output error.
			return	result;
		}


	}

	old_driver	=	CurrDriver;
	old_mode	=	CurrMode;
	old_device	=	CurrDevice;


	//
	// Step 1. Get New Mode
	//
	// Find new mode corresponding to w, h, bpp
	new_mode	=	old_driver->FindMode(w, h, bpp, 0, NULL);
	if(!new_mode)
	{
		result	=	DDERR_GENERIC;
		DebugText("ChangeMode: mode not available with this driver\n");
        return	result;
	}

	//
	// Step 2.   Check if Device needs to be changed as well
	//
	if(new_mode->ModeSupported(old_device))
	{
		new_device	=	NULL;
	}
	else
	{
		new_device	=	old_driver->FindDeviceSupportsMode(&old_device->guid,new_mode,&next_best_device);
		if(!new_device)
		{
			if(!next_best_device)
			{
				// No D3D device is compatible with this new mode
				result	=	DDERR_GENERIC;
				DebugText("ChangeMode: No device is compatible with this mode\n");
				return	result;
			}
			new_device	=	next_best_device;
		}
	}

	//
	// Step 3.	Destroy current Mode
	//
	FiniBack();
	FiniFront();
//  FiniFullscreenMode ();		// Don't do this => unnecessary mode switch

	//
	// Step 4.  Create new mode
	//
	CurrMode	=	new_mode;
	if(new_device)
		CurrDevice	=	new_device;

	// Change Mode
	result	=	InitFullscreenMode();
	if(FAILED(result))
		return	result;

	// Create Primary Surface
	result	=	InitFront();
	if(FAILED(result))
	{
		DebugText("ChangeMode: Error in InitFront\n");

		// Try to restore old mode
		CurrMode	=	old_mode;
		CurrDevice	=	old_device;

		InitFullscreenMode();
		InitFront();
		InitBack();

		return	result;
	}

	// Create Render surface
	result	=	InitBack();
	if(FAILED(result))
	{
		DebugText("ChangeMode: Error in InitBack\n");

		FiniFront();
	//  FiniFullscreenMode ();		// Unnecessary mode switch

		// Try to restore old mode
		CurrMode	=	old_mode;
		CurrDevice	=	old_device;

		InitFullscreenMode();
		InitFront();
		InitBack();

		return	result;
	}

	//
	// Reload the background image.
	//

	if (background_image_mem)
	{
		create_background_surface(background_image_mem);
	}

	// Notify a display change.
	DisplayChangedOn();

	// Success
    return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::Restore(void)
{
	HRESULT		result;


	// Check Initialization
	if(!IsValid())
	{
		result	=	DDERR_GENERIC;
		DebugText("Restore: invalid initialisation\n");
        return	result;
	}

	// Restore Primary Surface
	if(lp_DD_FrontSurface)
    {
		result	=	lp_DD_FrontSurface->IsLost();
		if (FAILED(result))
		{
			result	=	lp_DD_FrontSurface->Restore();
			if(FAILED(result))
				return	result;
		}
	}

	// Restore Z Buffer
	if(lp_DD_ZBuffer)
	{
		result	=	lp_DD_ZBuffer->IsLost();
		if (FAILED(result))
		{
			result	=	lp_DD_ZBuffer->Restore();
			if(FAILED(result))
				return	result;
		}
	}

	// Restore Rendering surface
	if(lp_DD_BackSurface)
	{
		result	=	lp_DD_BackSurface->IsLost();
		if (FAILED(result))
		{
			result	=	lp_DD_BackSurface->Restore();
			if(FAILED(result))
				return	result;
		}
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::AddLoadedTexture(D3DTexture *the_texture)
{
#ifdef DEBUG
	// Check that this isn't a circular list and that this texture isn't already loaded.
	D3DTexture *t = TextureList;
	int iCountdown = 10000;
	while ( t != NULL )
	{
		ASSERT ( t != the_texture );
		t = t->NextTexture;
		iCountdown--;
		ASSERT ( iCountdown > 0 );
	}

#endif

	the_texture->NextTexture	=	TextureList;
	TextureList					=	the_texture;


	return	DD_OK;
}

//---------------------------------------------------------------

void Display::RemoveAllLoadedTextures(void)
{
	TextureList = NULL;
}


//---------------------------------------------------------------

HRESULT	Display::FreeLoadedTextures(void)
{
	D3DTexture		*current_texture;


	int iCountdown = 10000;

	current_texture	=	TextureList;
	while(current_texture)
	{
		D3DTexture *next_texture = current_texture->NextTexture;
		current_texture->Destroy();
		current_texture = next_texture;
		iCountdown--;
		if ( iCountdown == 0 )
		{
			// Oh dear - not good.
			ASSERT ( FALSE );
			break;
		}
	}

	return	DD_OK;
}

//---------------------------------------------------------------

static char	clumpfile[MAX_PATH] = "";
static size_t clumpsize = 0;

void SetLastClumpfile(char* file, size_t size)
{
	strcpy(clumpfile, file);
	clumpsize = size;
}

HRESULT	Display::ReloadTextures(void)
{
	D3DTexture		*current_texture;

#ifdef TARGET_DC
	ASSERT ( clumpfile[0] == '\0' );
#else
	if (clumpfile[0])
	{
		OpenTGAClump(clumpfile, clumpsize, true);
	}
#endif

	current_texture	=	TextureList;
	while(current_texture)
	{
		D3DTexture *next_texture = current_texture->NextTexture;
		current_texture->Reload();
		current_texture = next_texture;
	}

#ifdef TARGET_DC
	ASSERT ( clumpfile[0] == '\0' );
#else
	if (clumpfile[0])
	{
		CloseTGAClump();
	}
#endif

	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::toGDI(void)
{
	HRESULT		result;

/*
	// Restore system palette.
	if(lpddpPalette)
	{
		// Save the current palette
		hResult = lpddpPalette->GetEntries (0, 0, cPalette, lppeCurr);
		if (FAILED (hResult))
		{
			REPORTERR (hResult);
			return hResult;
		}

		// Restore the system palette into our device
		hResult = lpddpPalette->SetEntries (0, 0, cPalette, lppeSystem);
		if (FAILED (hResult))
		{
			REPORTERR (hResult);
			return hResult;
		}
	}
*/

	// Flip to GDI Surface
	if(lp_DD4)
	{
		result	=	lp_DD4->FlipToGDISurface();
		if(FAILED(result))
		{
			// Output error.
			return	result;
		}
	}

#ifndef TARGET_DC
	// Force window to redraw itself (on GDI surface).
	if((hDDLibWindow) && (IsWindow(hDDLibWindow)))
	{
		DrawMenuBar(hDDLibWindow);
		RedrawWindow(hDDLibWindow, NULL, NULL, RDW_FRAME);
	}
#endif

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	Display::fromGDI(void)
{
//	HRESULT		result;

/*
	// Restore current palette
	if (lpddpPalette)
	{
		hResult = lpddpPalette->SetEntries (0, 0, cPalette, lppeCurr);
		if (FAILED (hResult))
			return hResult;
	}
*/
	// Success
	return DD_OK;
}

//---------------------------------------------------------------

void	Display::MenuOn(void)
{
#ifndef TARGET_DC
	if(IsFullScreen())
	{
		// Set the window style
		SetMenu(hDDLibWindow,hDDLibMenu);
		SetWindowLong(hDDLibWindow,GWL_STYLE,hDDLibStyle);
		SetWindowLong(hDDLibWindow,GWL_EXSTYLE,hDDLibStyleEx);
	}
#endif
}

//---------------------------------------------------------------

void	Display::MenuOff(void)
{
#ifndef TARGET_DC
	if(IsFullScreen())
	{
		SetWindowLong(hDDLibWindow,GWL_STYLE,0);
		SetWindowLong(hDDLibWindow,GWL_EXSTYLE,0);
		SetMenu(hDDLibWindow,NULL);
	}
#endif
}

//---------------------------------------------------------------

#ifndef TARGET_DC
HRESULT	Display::ShowWorkScreen(void)
{
	return	lp_DD_FrontSurface->Blt(&DisplayRect,lp_DD_WorkSurface,NULL,DDBLT_WAIT,NULL);
}
#endif

//---------------------------------------------------------------
void *Display::screen_lock(void)
{
	if (DisplayFlags & DISPLAY_LOCKED)
	{
		//
		// Don't do anything if you try to lock the screen twice in a row.
		//

		TRACE("Locking the screen when it is already locked!\n");
	}
	else
	{
		DDSURFACEDESC2 ddsdesc;
		HRESULT        ret;

		InitStruct(ddsdesc);

		ret = lp_DD_BackSurface->Lock(NULL, &ddsdesc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);

		if (SUCCEEDED(ret))
		{
			screen_width  = ddsdesc.dwWidth;
			screen_height = ddsdesc.dwHeight;
			screen_pitch  = ddsdesc.lPitch;
			screen_bbp    = ddsdesc.ddpfPixelFormat.dwRGBBitCount;
			screen        = (UBYTE *) ddsdesc.lpSurface;

			DisplayFlags |= DISPLAY_LOCKED;
		}
		else
		{
			d3d_error(ret);

			screen = NULL;
		}
	}

	return screen;
}

void  Display::screen_unlock(void)
{
	if (DisplayFlags & DISPLAY_LOCKED)
	{
		lp_DD_BackSurface->Unlock(NULL);
	}

	screen        =  NULL;
	DisplayFlags &= ~DISPLAY_LOCKED;
}



void Display::PlotPixel(SLONG x, SLONG y, UBYTE red, UBYTE green, UBYTE blue)
{
	if (DisplayFlags & DISPLAY_LOCKED)
	{
		if (WITHIN(x, 0, screen_width  - 1) &&
			WITHIN(y, 0, screen_height - 1))
		{
			if (CurrMode->GetBPP() == 16)
			{
				UWORD *dest;

				UWORD pixel = GetFormattedPixel(red, green, blue);
				SLONG index = x + x + y * screen_pitch;

				dest    = (UWORD *) (&(screen[index]));
				dest[0] = pixel;
			}
			else
			{
				ULONG*	dest;
				ULONG	pixel = GetFormattedPixel(red, green, blue);
				SLONG	index = x*4 + y * screen_pitch;

				dest	= (ULONG*)(screen + index);
				dest[0] = pixel;
			}
		}
	}
	else
	{
		//
		// Do nothing if the screen is not locked.
		//

		TRACE("PlotPixel while screen is not locked.\n");
	}
}

void Display::PlotFormattedPixel(SLONG x, SLONG y, ULONG colour)
{
	if (DisplayFlags & DISPLAY_LOCKED)
	{
		if (WITHIN(x, 0, screen_width  - 1) &&
			WITHIN(y, 0, screen_height - 1))
		{
			if (CurrMode->GetBPP() == 16)
			{
				UWORD *dest;
				SLONG  index = x + x + y * screen_pitch;

				dest    = (UWORD *) (&(screen[index]));
				dest[0] = colour;
			}
			else
			{
				ULONG* dest;
				SLONG index = x*4 + y*screen_pitch;
				dest = (ULONG*)(screen + index);
				dest[0] = colour;
			}
		}
	}
	else
	{
		//
		// Do nothing if the screen is not locked.
		//

		TRACE("PlotFormattedPixel while screen is not locked.\n");
	}
}

void Display::GetPixel(SLONG x, SLONG y, UBYTE *red, UBYTE *green, UBYTE *blue)
{
	SLONG index;

	ULONG	colour;

	*red   = 0;
	*green = 0;
	*blue  = 0;

	if (DisplayFlags & DISPLAY_LOCKED)
	{
		if (WITHIN(x, 0, screen_width  - 1) &&
			WITHIN(y, 0, screen_height - 1))
		{
			if (CurrMode->GetBPP() == 16)
			{
				UWORD *dest;
				SLONG  index = x + x + y * screen_pitch;

				dest   = (UWORD *) (&(screen[index]));
				colour = dest[0];
			}
			else
			{
				ULONG *dest;
				SLONG	index = 4*x + y*screen_pitch;
				dest = (ULONG*)(screen + index);
				colour = dest[0];

			}

		   *red   = ((colour >> shift_red)   << mask_red)   & 0xff;
		   *green = ((colour >> shift_green) << mask_green) & 0xff;
		   *blue  = ((colour >> shift_blue)  << mask_blue)  & 0xff;
		}
	}
}



void Display::blit_back_buffer()
{
	POINT clientpos;
	RECT  dest;

	HRESULT res;

	if (the_display.IsFullScreen())
	{
		res = lp_DD_FrontSurface->Blt(NULL,lp_DD_BackSurface,NULL,DDBLT_WAIT,0);
	}
	else
	{
		clientpos.x = 0;
		clientpos.y = 0;

		ClientToScreen(
			hDDLibWindow,
			&clientpos);

		GetClientRect(
			hDDLibWindow,
		   &dest);

		dest.top    += clientpos.y;
		dest.left   += clientpos.x;
		dest.right  += clientpos.x;
		dest.bottom += clientpos.y;

		res = lp_DD_FrontSurface->Blt(&dest,lp_DD_BackSurface,NULL,DDBLT_WAIT,0);
	}

	if (FAILED(res))
	{
		dd_error(res);
	}
}


// create a background surface from a 640x480x24 image

void CopyBackground16(UBYTE* image_data, IDirectDrawSurface4* surface)
{
	DDSURFACEDESC2	mine;
	HRESULT			res;

	InitStruct(mine);
	res = surface->Lock(NULL, &mine, DDLOCK_WAIT, NULL);
	if (FAILED(res))	return;

	SLONG  pitch = mine.lPitch >> 1;
	UWORD *mem   = (UWORD *) mine.lpSurface;
	SLONG  width;
	SLONG  height;

	// stretch the image

	SLONG	sdx = 65536 * 640 / mine.dwWidth;
	SLONG	sdy = 65536 * 480 / mine.dwHeight;

	SLONG	lsy = -1;
	SLONG	sy = 0;
	UWORD*	lmem = NULL;

	for (height = 0; (unsigned)height < mine.dwHeight; height++)
	{
		UBYTE*	src = image_data + 640 * 3 * (sy >> 16);

		if ((sy >> 16) == lsy)
		{
			// repeat line
			memcpy(mem, lmem, mine.dwWidth * 2);
		}
		else
		{
			SLONG	sx = 0;

			for (width = 0; (unsigned)width < mine.dwWidth; width++)
			{
				UBYTE*	pp = src + 3 * (sx >> 16);

				mem[width] = the_display.GetFormattedPixel(pp[2], pp[1], pp[0]);

				sx += sdx;
			}
		}

		lmem = mem;
		lsy = sy >> 16;

		mem += pitch;
		sy += sdy;
	}

	surface->Unlock(NULL);
}



void CopyBackground32(UBYTE* image_data, IDirectDrawSurface4* surface)
{
	DDSURFACEDESC2	mine;
	HRESULT			res;

	InitStruct(mine);
	res = surface->Lock(NULL, &mine, DDLOCK_WAIT, NULL);
	if (FAILED(res))	return;

	SLONG  pitch = mine.lPitch >> 2;
	ULONG *mem   = (ULONG *)mine.lpSurface;
	SLONG  width;
	SLONG  height;

	// stretch the image

	SLONG	sdx = 65536 * 640 / mine.dwWidth;
	SLONG	sdy = 65536 * 480 / mine.dwHeight;

	SLONG	lsy = -1;
	SLONG	sy = 0;
	ULONG*	lmem = NULL;

	for (height = 0; (unsigned)height < mine.dwHeight; height++)
	{
		UBYTE*	src = image_data + 640 * 3 * (sy >> 16);

		if ((sy >> 16) == lsy)
		{
			// repeat line
			memcpy(mem, lmem, mine.dwWidth * 4);
		}
		else
		{
			SLONG	sx = 0;

			for (width = 0; (unsigned)width < mine.dwWidth; width++)
			{
				UBYTE*	pp = src + 3 * (sx >> 16);

				mem[width] = the_display.GetFormattedPixel(pp[2], pp[1], pp[0]);

				sx += sdx;
			}
		}

		lmem = mem;
		lsy = sy >> 16;

		mem += pitch;
		sy += sdy;
	}

	surface->Unlock(NULL);
}




#ifdef TARGET_DC
// This uses my funky pixel format.
// This only works to a 640x480,565 format image.
// Unpacks image_data into surface
#define UNPACKING_DEBUG_INFO 0
void UnpackBackground ( UBYTE* image_data, IDirectDrawSurface4* surface )
{
	DDSURFACEDESC2	mine;
	HRESULT			res;

	InitStruct(mine);
	mine.dwFlags = DDSD_PITCH;
	res = surface->Lock(NULL, &mine, DDLOCK_WAIT, NULL);
	if (FAILED(res))	return;

	SLONG  pitch = mine.lPitch >> 1;
	WORD *mem   = (WORD *)mine.lpSurface;
	SLONG  width;
	SLONG  height;

	// The two line buffers.
	WORD	wLine[2][640];
	int iCurLineBuffer = 0;
	int iPrevLineBuffer = 1;

	// Fill both lines with zeroes.

	for ( int iY = 0; iY < 480; iY++ )
	{
		// Fill one line buffer.
		for ( int iX = 0; iX < 640; iX++ )
		{
			UBYTE bData = *image_data++;

#if UNPACKING_DEBUG_INFO
			if ( bData & 0x80 )
			{
				image_data++;

				// White = raw data.
				wLine[iCurLineBuffer][iX] = 0xffff;
			}
			else
			{
				if ( bData & 0x40 )
				{
					// Red = horizontal lerp.
					wLine[iCurLineBuffer][iX] = 0xf800;
				}
				else
				{
					// Green = vertical lerp.
					wLine[iCurLineBuffer][iX] = 0x07e0;
				}
			}

#else //#if UNPACKING_DEBUG_INFO
			if ( bData & 0x80 )
			{
				// Top bit set - this and the next byte are raw 555 pixel data.
				WORD wPixel = ( bData << 8 ) | ( *image_data++ );
				// And convert to 565, not 555.
				wLine[iCurLineBuffer][iX] = ( ( wPixel & 0x7fe0 ) << 1 ) | ( wPixel &0x1f );
			}
			else
			{
				// Top bit clear.
				// Format is now:
				// 0 d rr gg bb
				WORD wPixel;
				if ( bData & 0x40 )
				{
					// d bit is set - use pixel above.
					if ( iY > 0 )
					{
						wPixel = wLine[iPrevLineBuffer][iX];
					}
					else
					{
						// This is valid.
						wPixel = 0;
					}
				}
				else
				{
					// d bit is clear - use pixel to left.
					if ( iX > 0 )
					{
						wPixel = wLine[iCurLineBuffer][iX-1];
					}
					else
					{
						// This is valid.
						wPixel = 0;
					}
				}
				// Now modify by the two-bit-fields for each colour.


				WORD wMask;
				WORD wBottomBit;
				WORD wComponent;

				// Red.
				wMask = 0xf800;
				wBottomBit = 0x0800;
				switch ( ( bData & 0x30 ) >> 4 )
				{
				case 0:
					// No change.
					break;
				case 1:
					// Plus 1 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 2:
					// Plus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 3:
					// Minus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) - wBottomBit - wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				}

				// Green.
				wMask = 0x07e0;
				wBottomBit = 0x0020;
				switch ( ( bData & 0x0c ) >> 2 )
				{
				case 0:
					// No change.
					break;
				case 1:
					// Plus 1 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 2:
					// Plus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 3:
					// Minus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) - wBottomBit - wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				}

				// Blue.
				wMask = 0x001f;
				wBottomBit = 0x0001;
				switch ( ( bData & 0x03 ) >> 0 )
				{
				case 0:
					// No change.
					break;
				case 1:
					// Plus 1 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 2:
					// Plus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 3:
					// Minus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) - wBottomBit - wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				}

				wLine[iCurLineBuffer][iX] = wPixel;

			}
#endif //#else //#if UNPACKING_DEBUG_INFO


		}
		// Dump that line into the surface.
#if 0
		memcpy ( mem, wLine[iCurLineBuffer], 640 * 2 );
#else
		for ( iX = 0; iX < 640; iX++ )
		{
			mem[iX] = wLine[iCurLineBuffer][iX];
		}
#endif
		mem += pitch;

		iPrevLineBuffer = iCurLineBuffer;
		iCurLineBuffer = 1 - iCurLineBuffer;
	}

	surface->Unlock(NULL);
}

#endif //#ifdef TARGET_DC


// This uses my funky pixel format.
// This only works to a 640x480,565 format image.
// Packs surface (640x480,565 pixels - raw data) into image_data.
// Returns the size of the packed version. You can pass in NULL for image_data,
// and it will just return the size, and not try to copy anything.
int PackBackground ( UBYTE* image_data, WORD *surface )
{
	// The two line buffers.
	WORD	wLine[2][640];
	int iCurLineBuffer = 0;
	int iPrevLineBuffer = 1;

	int iTotalSize = 0;

	for ( int iY = 0; iY < 480; iY++ )
	{
		for ( int iX = 0; iX < 640; iX++ )
		{

			WORD wPixel = surface[iX + 640 * iY];

			WORD wNeighbourPixel[2];
			// Find left pixel.
			if ( iX > 0 )
			{
				wNeighbourPixel[0] = wLine[iCurLineBuffer][iX - 1];
			}
			else
			{
				wNeighbourPixel[0] = 0;
			}

			// Find above pixel.
			if ( iY > 0 )
			{
				wNeighbourPixel[1] = wLine[iPrevLineBuffer][iX];
			}
			else
			{
				wNeighbourPixel[1] = 0;
			}


			// Now try both to see if they will work.
			BYTE bData;
			int iNeighbour;
			for ( iNeighbour = 0; iNeighbour < 2; iNeighbour++ )
			{
				bData = iNeighbour << 6;

				WORD wMask;
				WORD wShift;
				WORD wPrev;
				WORD wCur;

				// Try the red channel.
				wMask = 0xf800;
				wShift = 11;
				wPrev = ( wNeighbourPixel[iNeighbour] & wMask ) >> wShift;
				wCur  = ( wPixel & wMask ) >> wShift;
				switch ( wCur - wPrev )
				{
				case 0:
				case -1:
					// Cool. (the -1 is a fudge, but only loses a tiny bit of accuracy).
					bData |= ( 0 << 4 );
					break;
				case 1:
					// Yep.
					bData |= ( 1 << 4 );
					break;
				case 2:
					// Yep.
					bData |= ( 2 << 4 );
					break;
				case -2:
					// Yep.
					bData |= ( 3 << 4 );
					break;
				default:
					// No - out of range - try the next neighbour.
					continue;
				}

				// Try the green channel.
				wMask = 0x07e0;
				wShift = 5;
				wPrev = ( wNeighbourPixel[iNeighbour] & wMask ) >> wShift;
				wCur  = ( wPixel & wMask ) >> wShift;
				switch ( wCur - wPrev )
				{
				case 0:
				case -1:
					// Cool. (the -1 is a fudge, but only loses a tiny bit of accuracy).
					bData |= ( 0 << 2 );
					break;
				case 1:
					// Yep.
					bData |= ( 1 << 2 );
					break;
				case 2:
					// Yep.
					bData |= ( 2 << 2 );
					break;
				case -2:
					// Yep.
					bData |= ( 3 << 2 );
					break;
				default:
					// No - out of range - try the next neighbour.
					continue;
				}

				// Try the blue channel.
				wMask = 0x001f;
				wShift = 0;
				wPrev = ( wNeighbourPixel[iNeighbour] & wMask ) >> wShift;
				wCur  = ( wPixel & wMask ) >> wShift;
				switch ( wCur - wPrev )
				{
				case 0:
				case -1:
					// Cool. (the -1 is a fudge, but only loses a tiny bit of accuracy).
					bData |= ( 0 << 0 );
					break;
				case 1:
					// Yep.
					bData |= ( 1 << 0 );
					break;
				case 2:
					// Yep.
					bData |= ( 2 << 0 );
					break;
				case -2:
					// Yep.
					bData |= ( 3 << 0 );
					break;
				default:
					// No - out of range - try the next neighbour.
					continue;
				}

				// OK, that worked - stop looking.
				break;

			}

			if ( iNeighbour == 2 )
			{
				// Nope - have to store the pixel raw.
				// But convert to 555 first.
				if ( image_data != NULL )
				{
					// Format 565: rrrr rggg gggb bbbb
					// Convert to: 0rrr rrgg gggb bbbb
					// And set top bit to say it's raw data.
					image_data[0] = ( wPixel >> 9 ) | 0x80;
					image_data[1] = ( ( wPixel & 0x01c0 ) >> 1 ) | ( wPixel & 0x001f );
					image_data += 2;

				}
				iTotalSize += 2;

				// And remember to put this into the line buffer, remembering to chop off the bottom
				// bit of green, but we don't actually convert to 555.
				wLine[iCurLineBuffer][iX] = wPixel & 0xffdf;
			}
			else
			{
				// Cool - we could do it.
				if ( image_data != NULL )
				{
					*image_data++ = bData;
				}
				iTotalSize += 1;

				// And write back this version into the line buffer.

				WORD wPixel;
				if ( bData & 0x40 )
				{
					// d bit is set - use pixel above.
					wPixel = wNeighbourPixel[1];
				}
				else
				{
					// d bit is clear - use pixel to left.
					wPixel = wNeighbourPixel[0];
				}
				// Now modify by the two-bit-fields for each colour.

				WORD wMask;
				WORD wBottomBit;
				WORD wComponent;

				// Red.
				wMask = 0xf800;
				wBottomBit = 0x0800;
				switch ( ( bData & 0x30 ) >> 4 )
				{
				case 0:
					// No change.
					break;
				case 1:
					// Plus 1 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 2:
					// Plus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 3:
					// Minus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) - wBottomBit - wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				}

				// Green.
				wMask = 0x07e0;
				wBottomBit = 0x0020;
				switch ( ( bData & 0x0c ) >> 2 )
				{
				case 0:
					// No change.
					break;
				case 1:
					// Plus 1 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 2:
					// Plus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 3:
					// Minus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) - wBottomBit - wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				}

				// Blue.
				wMask = 0x001f;
				wBottomBit = 0x0001;
				switch ( ( bData & 0x03 ) >> 0 )
				{
				case 0:
					// No change.
					break;
				case 1:
					// Plus 1 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 2:
					// Plus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) + wBottomBit + wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				case 3:
					// Minus 2 (with wraparound)
					wComponent = ( wPixel & wMask ) - wBottomBit - wBottomBit;
					wPixel = ( wPixel & ~wMask ) | ( wComponent & wMask );
					break;
				}

				wLine[iCurLineBuffer][iX] = wPixel;
			}


		}
		iPrevLineBuffer = iCurLineBuffer;
		iCurLineBuffer = 1 - iCurLineBuffer;
	}

	return ( iTotalSize );

}




void CopyBackground(UBYTE* image_data, IDirectDrawSurface4* surface)
{
//#ifdef TARGET_DC
//	// Always use my wacky compressed format. ATF
//	UnpackBackground ( image_data, surface );
//#else

#if USE_COMPRESSED_BACKGROUNDS
	// This just won't work.
	ASSERT ( FALSE );
#endif

	if (the_display.CurrMode->GetBPP() == 16)	CopyBackground16(image_data, surface);
	else										CopyBackground32(image_data, surface);
//#endif
}




void PANEL_ResetDepthBodge ( void );

HRESULT			Display::Flip(LPDIRECTDRAWSURFACE4 alt,SLONG flags)
{
	extern void PreFlipTT();
	PreFlipTT();

	// Make sure panels and text work.
	PANEL_ResetDepthBodge();

	// Draw the screensaver (if any).
	PANEL_screensaver_draw();

	if(IsFullScreen() && CurrDevice->IsHardware())
	{
#ifdef TARGET_DC


#if 1
#ifdef TARGET_DC
		// Reset the viewport so that text, etc gets drawn even when in letterbox mode.

		{
			D3DVIEWPORT2 viewData;
			memset(&viewData, 0, sizeof(D3DVIEWPORT2));
			viewData.dwSize = sizeof(D3DVIEWPORT2);

			// A horrible hack for letterbox mode.
			viewData.dwWidth  = 640;
			viewData.dwHeight = 480;
			viewData.dwX = 0;
			viewData.dwY = 0;
			viewData.dvClipX  = -1.0f;
			viewData.dvClipY  =  1.0f;
			viewData.dvClipWidth  = 2.0f;
			viewData.dvClipHeight = 2.0f;
			viewData.dvMinZ = 0.0f;
			viewData.dvMaxZ = 1.0f;
			HRESULT hres = (the_display.lp_D3D_Viewport)->SetViewport2 ( &viewData );
		}

#endif
#endif


		// Draw the frame.
		POLY_frame_draw(TRUE, TRUE);

		// Flip.
		HRESULT hres = lp_DD_FrontSurface->Flip(alt,flags);

		// And start the next one.
		POLY_frame_init(FALSE, FALSE);

		return hres;

		//return DD_OK;
#else
		return	lp_DD_FrontSurface->Flip(alt,flags);
#endif
	}
	else
	{
#ifdef TARGET_DC
		// Bad dog!
		ASSERT ( FALSE );
#endif
		// LogText("left - %ld, right - %ld, top - %ld. bottom - %ld\n",DisplayRect.left,DisplayRect.right,DisplayRect.top,DisplayRect.bottom);
		return	lp_DD_FrontSurface->Blt(&DisplayRect,lp_DD_BackSurface,NULL,DDBLT_WAIT,NULL);
	}
}



#if USE_COMPRESSED_BACKGROUNDS
void Display::use_this_background_surface(CompressedBackground this_one)
#else
void Display::use_this_background_surface(LPDIRECTDRAWSURFACE4 this_one)
#endif
{
	lp_DD_Background_use_instead = this_one;
}



void Display::create_background_surface(UBYTE *image_data)
{

#if USE_COMPRESSED_BACKGROUNDS

	// Don't call this.
	ASSERT ( FALSE );


	// First convert to 565 format.
	WORD *pwTemp = (WORD *)MemAlloc ( 640 * 480 * 2 );
	ASSERT ( pwTemp != NULL );
	UBYTE *pbSrc = image_data;

	for ( int i = 0; i < 640*480; i++ )
	{
		// From 24-bit RGB to 565.
		*pwTemp  = ( ( *pbSrc++ ) & 0xf8 ) << 8;
		*pwTemp |= ( ( *pbSrc++ ) & 0xfc ) << 3;
		*pwTemp |= ( ( *pbSrc++ ) & 0xf8 ) >> 3;
		pwTemp++;
	}

	// See how big it is, compressed.
	int iSize = PackBackground ( NULL, pwTemp );

	TRACE ( "create_background_surface: Original 565 0x%x, now 0x%x, saving of %i percent\n", 640*480*2, iSize, (100*iSize)/(640*480*2) );

	if (lp_DD_Background)
	{
		MemFree ( lp_DD_Background );
		lp_DD_Background = NULL;
	}

	lp_DD_Background = MemAlloc ( iSize );

	PackBackground ( (UBYTE *)lp_DD_Background, pwTemp );

	// And free the 565 version.
	MemFree ( (void *)pwTemp );


#else //#if USE_COMPRESSED_BACKGROUNDS

	DDSURFACEDESC2 back;
	DDSURFACEDESC2 mine;

	//
	// Remember this if we have to reload.
	//

	background_image_mem = image_data;

	//
	// Incase we already have one!
	//

	if (lp_DD_Background)
	{
		lp_DD_Background->Release();
		lp_DD_Background = NULL;
	}

	lp_DD_Background_use_instead = NULL;

	//
	// Create a mirror surface to the back buffer.
	//

	InitStruct(back);

	lp_DD_BackSurface->GetSurfaceDesc(&back);

	//
	// Create the mirror surface in system memory.
	//

	InitStruct(mine);

	mine.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	mine.dwWidth         = back.dwWidth;
	mine.dwHeight        = back.dwHeight;
	mine.ddpfPixelFormat = back.ddpfPixelFormat;
	mine.ddsCaps.dwCaps	 = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	HRESULT result = lp_DD4->CreateSurface(&mine, &lp_DD_Background, NULL);

	if (FAILED(result))
	{
		lp_DD_Background = NULL;
		background_image_mem = NULL;
		return;
	}

	//
	// Copy the image into the surface...
	//

	CopyBackground(image_data, lp_DD_Background);

#endif //#else //#if USE_COMPRESSED_BACKGROUNDS


	return;
}




void Display::blit_background_surface(bool b3DInFrame)
{
#if USE_COMPRESSED_BACKGROUNDS
	CompressedBackground lpBG = NULL;
#else
	LPDIRECTDRAWSURFACE4 lpBG = NULL;
#endif

	if ( lp_DD_Background_use_instead != NULL )
	{
		lpBG = lp_DD_Background_use_instead;
	}
	else if ( lp_DD_Background != NULL )
	{
		lpBG = lp_DD_Background;
	}
	else
	{
		return;
	}


	HRESULT result;

#ifdef TARGET_DC
	{

		if ( !b3DInFrame )
		{
			// Won't have been done yet.
			//POLY_frame_init(FALSE,FALSE);
		}


		// Use a poly draw.
		ASSERT ( lpBackgroundCache != NULL );

		if ( lpBG != m_lpLastBackground )
		{
			// Get the texture handle.
			m_lpLastBackground = lpBG;


			static iCount = 0;


#if USE_COMPRESSED_BACKGROUNDS

			if ( lpBG != NULL )
			{
				UnpackBackground ( (UCHAR *)lpBG, lpBackgroundCache );
			}
			else
			{
				// Bum - black screen time :-(
				ASSERT ( FALSE );
			}
#else
			{
				// Copy the data to the texture cache thingie.
				RECT rect;
				rect.top = 0;
				rect.left = 0;
				rect.right = 640;
				rect.bottom = 480;
				HRESULT hres = lpBackgroundCache->Blt ( &rect, lpBG, &rect, DDBLT_WAIT, NULL );
				VERIFY(SUCCEEDED(hres));
			}
#endif
		}

		// ARGH! Got to use a poly draw instead. Useless machine.

		POLY_Point  pp[4];
		POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

		pp[0].colour=0xffffffff; pp[0].specular=0;
		pp[1].colour=0xffffffff; pp[1].specular=0;
		pp[2].colour=0xffffffff; pp[2].specular=0;
		pp[3].colour=0xffffffff; pp[3].specular=0;

		pp[0].X=0.0f; pp[0].Y=0.0f; pp[0].Z=0.0001f;
		pp[0].u=0.0f; pp[0].v=0.0f;

		pp[1].X=0.0f; pp[1].Y=480.0f; pp[1].Z=0.0001f;
		pp[1].u=0.0f; pp[1].v=480.0f / 512.0f;

		pp[2].X=640.0f; pp[2].Y=0.0f; pp[2].Z=0.0001f;
		pp[2].u=640.0f / 1024.0f; pp[2].v=0.0f;

		pp[3].X=640.0f; pp[3].Y=480.0f; pp[3].Z=0.0001f;
		pp[3].u=640.0f / 1024.0f; pp[3].v=480.0f / 512.0f;

		POLY_add_quad ( quad, POLY_PAGE_BACKGROUND_IMAGE, FALSE, TRUE );


		if ( !b3DInFrame )
		{
			// Draw the stuff now.
			//POLY_frame_draw(TRUE,TRUE);
		}

	}



#else
	{
		result = lp_DD_BackSurface->Blt(NULL,lpBG,NULL,DDBLT_WAIT,0);

		if (FAILED(result))
		{
			dd_error(result);
		}
	}
#endif
}

void Display::destroy_background_surface()
{
	if (lp_DD_Background)
	{
#if USE_COMPRESSED_BACKGROUNDS
		MemFree ( lp_DD_Background );
#else //#if USE_COMPRESSED_BACKGROUNDS
		lp_DD_Background->Release();
#endif //#else //#if USE_COMPRESSED_BACKGROUNDS
		lp_DD_Background = NULL;
	}


	background_image_mem = NULL;
}

bool Display::IsGammaAvailable()
{
	return (lp_DD_GammaControl != NULL);
}

// note: 0,256 is normal - white is *exclusive*

void Display::SetGamma(int black, int white)
{
	if (!lp_DD_GammaControl)	return;

	if (black < 0)			black = 0;
	if (white > 256)		white = 256;
	if (black > white - 1)	black = white - 1;

	ENV_set_value_number("BlackPoint", black, "Gamma");
	ENV_set_value_number("WhitePoint", white, "Gamma");

	DDGAMMARAMP	ramp;
	int			diff = white - black;

	black <<= 8;

	for (int ii = 0; ii < 256; ii++)
	{
		ramp.red[ii] = black;
		ramp.green[ii] = black;
		ramp.blue[ii] = black;

		black += diff;
	}

	lp_DD_GammaControl->SetGammaRamp(0, &ramp);
}

void Display::GetGamma(int* black, int* white)
{
	*black = ENV_get_value_number("BlackPoint", 0, "Gamma");
	*white = ENV_get_value_number("WhitePoint", 256, "Gamma");
}


//
// DIALOG BOX
//

static bool	         is_primary = false;
static bool          is_secondary = false;
static DDDriverInfo *primary_driver;
static DDDriverInfo *secondary_driver;



//
// Returns TRUE if the current Video3DMode support 16 or 32-bit colour.
//

#define SUPPORTS_16 (1 << 0)
#define SUPPORTS_32	(1 << 2)

SLONG CurrentVideo3DModeSupports(void)
{
	SLONG ans = 0;

	switch(Video3DMode)
	{
		case 0:

			ASSERT(primary_driver);

			if (primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_16BIT) {ans |= SUPPORTS_16;}
			if (primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT) {ans |= SUPPORTS_32;}

			break;

		case 1:

			ASSERT(secondary_driver);

			if (secondary_driver->DriverFlags & DD_DRIVER_SUPPORTS_16BIT) {ans |= SUPPORTS_16;}
			if (secondary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT) {ans |= SUPPORTS_32;}

			break;

		case 2:

			if (primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT)
			{
				ans = SUPPORTS_32;
			}
			else
			{
				ans = SUPPORTS_16;
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	return ans;
}



#ifndef TARGET_DC


static void InitDialog(HWND hWnd)
{
	// centre window
	RECT	winsize;
	RECT	scrsize;

	GetWindowRect(hWnd, &winsize);
	GetClientRect(GetDesktopWindow(), &scrsize);

	int		xoff = ((scrsize.right - scrsize.left) - (winsize.right - winsize.left)) / 2;
	int		yoff = ((scrsize.bottom - scrsize.top) - (winsize.bottom - winsize.top)) / 2;

	SetWindowPos(hWnd, NULL, xoff, yoff, 0,0, SWP_NOZORDER | SWP_NOSIZE);

	// localise this bastard
	CBYTE *lang=ENV_get_value_string("language", "Game");
	if (lang == NULL) {
		XLAT_load("text/lang_english.txt");
	} else {
		XLAT_load(lang);
		ENV_free_string(lang);
	}
	XLAT_init();
	//SetWindowText(hWnd,XLAT_str(X_GRAPHICS));
	SetDlgItemTextA(hWnd,IDC_GRAPHICS_OPTIONS,XLAT_str(X_GRAPHICS));
	SetDlgItemTextA(hWnd,IDC_STATIC_3DCARD,XLAT_str(X_3DCARD));
	SetDlgItemTextA(hWnd,IDC_STATIC_BITDEPTH,XLAT_str(X_NUM_COLOURS));
	SetDlgItemTextA(hWnd,IDC_STATIC_RES,XLAT_str(X_RESOLUTION));
	SetDlgItemTextA(hWnd,IDC_PRIMARY_3D,XLAT_str(X_USE_PRIMARY));
	SetDlgItemTextA(hWnd,IDC_SECONDARY_3D,XLAT_str(X_USE_SECONDARY));
	SetDlgItemTextA(hWnd,IDC_SOFTWARE_3D,XLAT_str(X_USE_SOFTWARE));
	SetDlgItemTextA(hWnd,IDC_COLOURS_16,XLAT_str(X_16BIT));
	SetDlgItemTextA(hWnd,IDC_COLOURS_32,XLAT_str(X_24BIT));
	SetDlgItemTextA(hWnd,IDC_NOSHOW,XLAT_str(X_DO_NOT_SHOW));
	SetDlgItemTextA(hWnd,IDOK,XLAT_str(X_OKAY));
	SetDlgItemTextA(hWnd,IDCANCEL,XLAT_str(X_EXIT));

	// get windows
	HWND	primary3D = GetDlgItem(hWnd, IDC_PRIMARY_3D);
	HWND	secondary3D = GetDlgItem(hWnd, IDC_SECONDARY_3D);
	HWND	software3D = GetDlgItem(hWnd, IDC_SOFTWARE_3D);

	HWND	colour16 = GetDlgItem(hWnd, IDC_COLOURS_16);
	HWND	colour32 = GetDlgItem(hWnd, IDC_COLOURS_32);

	HWND	res = GetDlgItem(hWnd, IDC_RESOLUTION);

	HWND	noshow = GetDlgItem(hWnd, IDC_NOSHOW);

	// set 3D card buttons
	EnableWindow(primary3D, is_primary);
	EnableWindow(secondary3D, is_secondary);

#ifdef VERSION_DEMO
	EnableWindow(software3D, FALSE);
#else
	if (is_primary || is_secondary)	EnableWindow(software3D, FALSE);
#endif

	//
	// Always have software available- just for now.
	//

#ifndef VERSION_DEMO
	EnableWindow(software3D, TRUE);
#endif

	if (Video3DMode == 1 && !is_secondary)
	{
		//
		// Argh! Use primary!
		//

		Video3DMode = 0;
	}

	//
	// Does the card support our bit depth?
	//

	SLONG supports = CurrentVideo3DModeSupports();

	if (VideoTrueColour == 1 && !(supports & SUPPORTS_32))
	{
		VideoTrueColour = 0;
	}

	if (VideoTrueColour == 0 && !(supports & SUPPORTS_16))
	{
		VideoTrueColour = 1;
	}

	EnableWindow(colour16, !!(supports & SUPPORTS_16));
	EnableWindow(colour32, !!(supports & SUPPORTS_32));

	SendMessage(primary3D,   BM_SETCHECK, (Video3DMode == 0) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(secondary3D, BM_SETCHECK, (Video3DMode == 1) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(software3D,  BM_SETCHECK, (Video3DMode == 2) ? BST_CHECKED : BST_UNCHECKED, 0);

	// set colour buttons
	SendMessage(colour16, BM_SETCHECK, !VideoTrueColour ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(colour32, BM_SETCHECK,  VideoTrueColour ? BST_CHECKED : BST_UNCHECKED, 0);

	// set resolutions

	switch(Video3DMode)
	{
		case 0:

			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"320 x 240");
			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"512 x 384");
			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"640 x 480");
			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"800 x 600");
			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"1024 x 768");

			break;

		case 1:

			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"512 x 384");
			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"640 x 480");

			if (secondary_driver->DriverFlags & DD_DRIVER_MODE_1024)
			{
				SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"800 x 600");

				SATURATE(VideoRes, 0, 2);
			}
			else
			{
				SATURATE(VideoRes, 0, 1);
			}

			break;

		case 2:

			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"320 x 240");
			SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"512 x 384");

			SATURATE(VideoRes, 0, 1);

			break;
	}

	SendMessage(res, CB_SETCURSEL, VideoRes, 0);

	// set no show
	//SendMessage(noshow, BM_SETCHECK, BST_UNCHECKED, 0);

	//
	// Get MFX_MILES.cpp to initialise the sound part of our dialog box.
	//

	init_my_dialog(hWnd);
}

static void FinishDialog(HWND hWnd)
{
	// get windows
	HWND	primary3D = GetDlgItem(hWnd, IDC_PRIMARY_3D);
	HWND	secondary3D = GetDlgItem(hWnd, IDC_SECONDARY_3D);
	HWND	software3D = GetDlgItem(hWnd, IDC_SOFTWARE_3D);

	HWND	colour16 = GetDlgItem(hWnd, IDC_COLOURS_16);
	HWND	colour32 = GetDlgItem(hWnd, IDC_COLOURS_32);

	HWND	res = GetDlgItem(hWnd, IDC_RESOLUTION);

	//HWND	noshow = GetDlgItem(hWnd, IDC_NOSHOW);

	if (SendMessage(primary3D, BM_GETCHECK, 0, 0))			Video3DMode = 0;
	else if (SendMessage(secondary3D, BM_GETCHECK, 0, 0))	Video3DMode = 1;
	else													Video3DMode = 2;

	if (SendMessage(colour16, BM_GETCHECK, 0, 0))			VideoTrueColour = false;
	else													VideoTrueColour = true;

	VideoRes = SendMessage(res, CB_GETCURSEL, 0, 0);

	if (Video3DMode == 1)
	{
		VideoRes += 1;
	}

	/*

	if (SendMessage(noshow, BM_GETCHECK, 0, 0))
	{
		ENV_set_value_number("run_video_dialog", 0, "Render");
	}

	*/

	my_dialogs_over(hWnd);
}

static BOOL CALLBACK dlgproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		InitDialog(hWnd);
		return TRUE;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
			case IDOK:
				FinishDialog(hWnd);
				EndDialog(hWnd, 0);
				return TRUE;

			case IDCANCEL:
				//
				// End everything!
				//

				extern void MilesTerm(void);

				MilesTerm();
				the_manager.Fini();

				exit(1);

			case IDC_PRIMARY_3D:

				{
					HWND colour16 = GetDlgItem(hWnd, IDC_COLOURS_16);
					HWND colour32 = GetDlgItem(hWnd, IDC_COLOURS_32);
					HWND res      = GetDlgItem(hWnd, IDC_RESOLUTION);

					//
					// Enable diable the colour buttons.
					//

					EnableWindow(colour16, !!(primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_16BIT));
					EnableWindow(colour32, !!(primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT));

					if (SendMessage(colour16, BM_GETCHECK, 0, 0))
					{
						if (!(primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_16BIT))
						{
							SendMessage(colour16, BM_SETCHECK, FALSE, 0);
							SendMessage(colour32, BM_SETCHECK, TRUE,  0);
						}
					}
					else
					{
						if (!(primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT))
						{
							SendMessage(colour16, BM_SETCHECK, TRUE,  0);
							SendMessage(colour32, BM_SETCHECK, FALSE, 0);
						}
					}

					// set resolutions

					SendMessage(res, CB_RESETCONTENT, 0,0);

					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"320 x 240");
					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"512 x 384");
					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"640 x 480");
					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"800 x 600");
					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"1024 x 768");

					SendMessage(res, CB_SETCURSEL, VideoRes, 0);

				}

				break;

			case IDC_SECONDARY_3D:

				{
					HWND colour16 = GetDlgItem(hWnd, IDC_COLOURS_16);
					HWND colour32 = GetDlgItem(hWnd, IDC_COLOURS_32);
					HWND res      = GetDlgItem(hWnd, IDC_RESOLUTION);

					//
					// Enable diable the colour buttons.
					//

					EnableWindow(colour16, !!(secondary_driver->DriverFlags & DD_DRIVER_SUPPORTS_16BIT));
					EnableWindow(colour32, !!(secondary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT));

					if (SendMessage(colour16, BM_GETCHECK, 0, 0))
					{
						if (!(secondary_driver->DriverFlags & DD_DRIVER_SUPPORTS_16BIT))
						{
							SendMessage(colour16, BM_SETCHECK, FALSE, 0);
							SendMessage(colour32, BM_SETCHECK, TRUE,  0);
						}
					}
					else
					{
						if (!(secondary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT))
						{
							SendMessage(colour16, BM_SETCHECK, TRUE,  0);
							SendMessage(colour32, BM_SETCHECK, FALSE, 0);
						}
					}

					// set resolutions

					SendMessage(res, CB_RESETCONTENT, 0,0);

					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"512 x 384");
					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"640 x 480");

					if (secondary_driver->DriverFlags & DD_DRIVER_MODE_1024)
					{
						SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"800 x 600");

						SATURATE(VideoRes, 0, 2);
					}
					else
					{
						SATURATE(VideoRes, 0, 1);
					}

					SendMessage(res, CB_SETCURSEL, VideoRes, 0);

					break;
				}

			case IDC_SOFTWARE_3D:

				{
					HWND colour16 = GetDlgItem(hWnd, IDC_COLOURS_16);
					HWND colour32 = GetDlgItem(hWnd, IDC_COLOURS_32);
					HWND res      = GetDlgItem(hWnd, IDC_RESOLUTION);

					if (primary_driver->DriverFlags & DD_DRIVER_SUPPORTS_32BIT)
					{
						#ifdef NDEBUG

						//
						// Enable and diable the colour buttons.
						//

						EnableWindow(colour16, FALSE);
						EnableWindow(colour32, TRUE);

						if (SendMessage(colour16, BM_GETCHECK, 0, 0))
						{
							SendMessage(colour16, BM_SETCHECK, FALSE, 0);
							SendMessage(colour32, BM_SETCHECK, TRUE,  0);
						}

						#else

						//
						// Enable all the colour buttons.
						//

						EnableWindow(colour16, TRUE);
						EnableWindow(colour32, TRUE);

						#endif
					}
					else
					{
						//
						// We have to use 16-bit software :(
						//

						EnableWindow(colour16, TRUE);
						EnableWindow(colour32, FALSE);

						SendMessage(colour16, BM_SETCHECK, TRUE,  0);
						SendMessage(colour32, BM_SETCHECK, FALSE, 0);

					}

					// set resolutions

					SendMessage(res, CB_RESETCONTENT, 0,0);

					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"320 x 240");
					SendMessage(res, CB_INSERTSTRING, -1, (LPARAM)"512 x 384");

					SATURATE(VideoRes, 0, 1);

					SendMessage(res, CB_SETCURSEL, VideoRes, 0);
				}

				break;
		}

		break;
	}

	return FALSE;
}


#endif //#ifndef TARGET_DC



//
// Returns TRUE if the driver has a harware mode that allowed alpha textures...
//

SLONG valid_driver(DDDriverInfo *drv)
{
#ifndef TARGET_DC
	D3DDeviceInfo *d3d;

	if (drv == NULL)
	{
		return FALSE;
	}

	for (d3d = drv->DeviceList; d3d; d3d = d3d->Next)
	{
		if (d3d->IsHardware())
		{
			if (d3d->d3dHalDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHA)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
#else
	return TRUE;
#endif
}


// GraphicsDialog
//
// do the dialog box

void GraphicsDialog(HINSTANCE hInst, HWND hWnd)
{
#ifdef TARGET_DC
	Video3DMode = -1;
	VideoTrueColour = false;
	VideoRes = -1;
#else
	// read existing config (if any)
	Video3DMode = ENV_get_value_number("video_card", -1, "Render");
	VideoTrueColour = ENV_get_value_number("video_truecolour", 0, "Render") ? true : false;
	VideoRes = ENV_get_value_number("video_res", -1, "Render");
#endif

	is_primary       = false;
	is_secondary     = false;
	primary_driver   = NULL;
	secondary_driver = NULL;

	// check for 1ry and 2ry cards
	DDDriverInfo*	drv = the_manager.DriverList;

	while (drv)
	{
		if (drv->IsPrimary())
		{
			is_primary     = true;
			primary_driver = drv;
		}
		else
		{
			is_secondary     = true;
			secondary_driver = drv;
		}

		drv = drv->Next;
	}

	//
	// Ban incompatible drivers?
	//

	if (!valid_driver(primary_driver  )) {is_primary   = FALSE; primary_driver   = NULL;}
	if (!valid_driver(secondary_driver)) {is_secondary = FALSE; secondary_driver = NULL;}

	// ban illegal settings
	if ((Video3DMode == 0) && !is_primary)		Video3DMode = -1;
	if ((Video3DMode == 1) && !is_secondary)	Video3DMode = -1;

	// set default card
	if (Video3DMode == -1)
	{
		if (is_primary)				Video3DMode = 0;
		else if (is_secondary)		Video3DMode = 1;
		else						Video3DMode = 2;
	}

#ifndef TARGET_DC
#ifdef VERSION_DEMO
	if (Video3DMode == 2)
	{
		MessageBox(NULL, "This demo requires a 3D accelerator (the release version works without one)", NULL, MB_ICONEXCLAMATION | MB_OK);
		exit(1);
	}
#endif
#endif

	// set default res
	if (VideoRes == -1)
	{
		if (Video3DMode < 2)	VideoRes = 2;
		else					VideoRes = 0;
	}

#ifndef TARGET_DC
	// run dialog
//	if (ENV_get_value_number("run_video_dialog", 1, "Render"))
	{
		DialogBox(hInst, MAKEINTRESOURCE(IDD_GRAPHICSDLG), hWnd, (DLGPROC)dlgproc);
	}
#else
	// I know what I want, thanks.
	Video3DMode = 0;	// Primary.
	VideoTrueColour = FALSE;
	VideoRes = 2;		// 640x480.
#endif

#ifndef TARGET_DC
	// store config
	ENV_set_value_number("video_card", Video3DMode, "Render");
	ENV_set_value_number("video_truecolour", VideoTrueColour ? 1 : 0, "Render");
	ENV_set_value_number("video_res", VideoRes, "Render");
#endif

	// look for illegal shit
	char*	err = NULL;

	/*

	if (Video3DMode == 1)
	{
		if (VideoTrueColour)		err = "Card does not support 24-bit colour";
	}
	else if (Video3DMode == 2)
	{
//		if (VideoTrueColour)		err = "Renderer does not support 24-bit colour";
	}

	*/

#ifndef TARGET_DC
	if (err)
	{
		MessageBox(NULL, err, NULL, MB_ICONEXCLAMATION);
		exit(1);
	}
#endif

	if (Video3DMode == 0)
	{
		the_manager.CurrDriver = primary_driver;
	}
	else
	if (Video3DMode == 1)
	{
		the_manager.CurrDriver = secondary_driver;
	}
}







