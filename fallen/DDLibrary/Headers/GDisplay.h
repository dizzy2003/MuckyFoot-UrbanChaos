// Display.h
// Guy Simmons, 13th November 1997.

#ifndef	DISPLAY_H
#define	DISPLAY_H

#include	"DDManager.h"


#ifdef TARGET_DC
#define USE_COMPRESSED_BACKGROUNDS 1
#else
#define USE_COMPRESSED_BACKGROUNDS 0
#endif

//---------------------------------------------------------------

#define	SHELL_ACTIVE			(LibShellActive())
#define	SHELL_CHANGED			(LibShellChanged())
#define	FLAGS_USE_3D			(1<<1)
#define	FLAGS_USE_WORKSCREEN	(1<<2)

//---------------------------------------------------------------

#define	DEFAULT_WIDTH			(640)
#define	DEFAULT_HEIGHT			(480)
#define	DEFAULT_DEPTH			(16)


enum enumDisplayType
{
	DT_PAL,
	DT_NTSC,
	DT_VGA,
};

extern enumDisplayType eDisplayType;


//---------------------------------------------------------------

#if USE_COMPRESSED_BACKGROUNDS
typedef void *CompressedBackground;
#endif


extern	void	InitBackImage(CBYTE *name);
#if USE_COMPRESSED_BACKGROUNDS
extern	void UseBackSurface(CompressedBackground use);
#else
extern	void UseBackSurface(LPDIRECTDRAWSURFACE4 use);
#endif
extern	void	ResetBackImage(void);
// Set b3DInFrame to FALSE if there is no 3D going on, i.e. blits will work on the DC.
// Ignored for the PC.
extern	void	ShowBackImage(bool b3DInFrame = TRUE);

SLONG			OpenDisplay(ULONG width, ULONG height, ULONG depth, ULONG flags);
SLONG			CloseDisplay(void);
SLONG			SetDisplay(ULONG width,ULONG height,ULONG depth);
SLONG			ClearDisplay(UBYTE r,UBYTE g,UBYTE b);
void			ShellPaused(void);
void			ShellPauseOn(void);
void			ShellPauseOff(void);
void			DumpBackToTGA(CBYTE *tga_name);
void			DumpBackToRaw(void);	// To shot\

//---------------------------------------------------------------

#define	DISPLAY_INIT		(1<<0)
#define	DISPLAY_PAUSE		(1<<1)
#define	DISPLAY_PAUSE_ACK	(1<<2)
#define DISPLAY_LOCKED		(1<<3)

#define	BK_COL_NONE			0
#define	BK_COL_BLACK		1
#define	BK_COL_WHITE		2
#define	BK_COL_USER			3



class	Display
{
	private:
	protected:
		enum
		{
			DWF_FULLSCREEN			=	(1<<0),
			DWF_VISIBLE				=	(1<<1),
			DWF_ZBUFFER				=	(1<<2),
			DWF_ACTIVE				=	(1<<3),
			DWF_USE_3D				=	(1<<4),
			DWF_USE_WORK			=	(1<<5),
			DWF_DISPLAY_CHANGED		=	(1<<6),
			DWF_TEXTURES_INVALID	=	(1<<7)
		}Attributes;

		enum
		{
			DWF_VALID_INTERFACE		=	(1<<0),
			DWF_VALID_FULLSCREEN	=	(1<<1),
			DWF_VALID_FRONT			=	(1<<2),
			DWF_VALID_BACK			=	(1<<3),
			DWF_VALID_WORK			=	(1<<4),
			DWF_VALID_CLIPPER		=	(1<<5),
			DWF_VALID_VIEWPORT		=	(1<<6),

			DWF_VALID				=	DWF_VALID_INTERFACE		|
										DWF_VALID_FULLSCREEN	|
										DWF_VALID_FRONT			|
										DWF_VALID_BACK
		}Validates;

		volatile SLONG			AttribFlags,				// Attribute flags.
								ValidFlags,					// Validation flags.
								PauseCount;



		inline	void TurnValidInterfaceOn(void)				{	ValidFlags	|=	DWF_VALID_INTERFACE;		}
		inline	void TurnValidInterfaceOff(void)			{	ValidFlags	&=	~DWF_VALID_INTERFACE;		}

		inline	void TurnValidFullscreenOn(void)			{	ValidFlags	|=	DWF_VALID_FULLSCREEN;		}
		inline	void TurnValidFullscreenOff(void)			{	ValidFlags	&=	~DWF_VALID_FULLSCREEN;		}

		inline	void TurnValidFrontOn(void)					{	ValidFlags	|=	DWF_VALID_FRONT;			}
		inline	void TurnValidFrontOff(void)				{	ValidFlags	&=	~DWF_VALID_FRONT;			}

		inline	void TurnValidBackOn(void)					{	ValidFlags	|=	DWF_VALID_BACK;				}
		inline	void TurnValidBackOff(void)					{	ValidFlags	&=	~DWF_VALID_BACK;			}

		inline	void TurnValidWorkOn(void)					{	ValidFlags	|=	DWF_VALID_WORK;				}
		inline	void TurnValidWorkOff(void)					{	ValidFlags	&=	~DWF_VALID_WORK;			}

		inline	void TurnValidClipperOn(void)				{	ValidFlags	|=	DWF_VALID_CLIPPER;			}
		inline	void TurnValidClipperOff(void)					{	ValidFlags	&=	~DWF_VALID_CLIPPER;			}

		inline	void TurnValidViewportOn(void)				{	ValidFlags	|=	DWF_VALID_VIEWPORT;			}
		inline	void TurnValidViewportOff(void)				{	ValidFlags	&=	~DWF_VALID_VIEWPORT;		}

		inline	BOOL			IsValidDefaults(void)		{	return	((CurrDriver && CurrMode && CurrDevice) ? TRUE : FALSE);	}
		inline	BOOL			IsValidInterface(void)		{	return	((ValidFlags&DWF_VALID_INTERFACE) ? TRUE : FALSE);			}
		inline	BOOL			IsValidFullscreen(void)		{	return	((ValidFlags&DWF_VALID_FULLSCREEN) ? TRUE : FALSE);			}
		inline	BOOL			IsValidFront(void)			{	return	((ValidFlags&DWF_VALID_FRONT) ? TRUE : FALSE);				}
		inline	BOOL			IsValidBack(void)			{	return	((ValidFlags&DWF_VALID_BACK) ? TRUE : FALSE);				}
		inline	BOOL			IsValidWork(void)			{	return	((ValidFlags&DWF_VALID_WORK) ? TRUE : FALSE);				}
		inline	BOOL			IsValidClipper(void)		{	return	((ValidFlags&DWF_VALID_CLIPPER) ? TRUE : FALSE);			}
		inline	BOOL			IsValidViewport(void)		{	return	((ValidFlags&DWF_VALID_VIEWPORT) ? TRUE : FALSE);			}

		UBYTE *background_image_mem;

#ifdef TARGET_DC
		bool					bInScene;
		LPDIRECTDRAWSURFACE4	psurfBackHandle;
#endif

	public:
		ULONG					BackColour,
								PaletteSize;
		D3DDeviceInfo			*CurrDevice;				// Current Device
		D3DMATERIALHANDLE		black_handle,
								white_handle,
								user_handle;
		D3DRECT					ViewportRect;
		D3DTexture				*TextureList;
		DDDriverInfo			*CurrDriver;				// Current Driver
		DDModeInfo				*CurrMode;					// Current Mode
		GUID					*DDGuid;
		LPDIRECT3D3				lp_D3D;
		LPDIRECT3DDEVICE3		lp_D3D_Device;
		LPDIRECT3DMATERIAL3		lp_D3D_Black,
								lp_D3D_White,
								lp_D3D_User;
		LPDIRECT3DVIEWPORT3		lp_D3D_Viewport;
		LPDIRECTDRAW			lp_DD;
		LPDIRECTDRAW4			lp_DD4;
		LPDIRECTDRAWCLIPPER     lp_DD_Clipper;
		LPDIRECTDRAWPALETTE		lp_DD_Palette;
		LPDIRECTDRAWSURFACE4	lp_DD_FrontSurface,
								lp_DD_BackSurface,
#ifndef TARGET_DC
								lp_DD_WorkSurface,
#endif
								lp_DD_ZBuffer;
#if USE_COMPRESSED_BACKGROUNDS
		CompressedBackground	lp_DD_Background,
								lp_DD_Background_use_instead;
#else
		LPDIRECTDRAWSURFACE4	lp_DD_Background,
								lp_DD_Background_use_instead;
#endif
#ifdef TARGET_DC
		LPDIRECT3DTEXTURE2		lp_DD_Background_use_instead_texture;
		LPDIRECT3DTEXTURE2		lp_DD_Background_use_instead_texture2;
#endif
		IDirectDrawGammaControl*	lp_DD_GammaControl;
		PALETTEENTRY			*lp_SysPalette,
								*lp_CurrPalette;
	    RECT					DisplayRect;				// Current surface rectangle.

		//
		// Pixel format variables.
		//

		SLONG					mask_red;
		SLONG					mask_green;
		SLONG					mask_blue;
		SLONG					shift_red;
		SLONG					shift_green;
		SLONG					shift_blue;

		//
		// For writing directly to the back-buffer. These
		// are only valid when the screen is locked.
		//

		SLONG					screen_width;
		SLONG                   screen_height;
		SLONG                   screen_pitch;
		SLONG                   screen_bbp;
		UBYTE			       *screen;

		volatile ULONG			DisplayFlags;

								Display();
								~Display();

		// Methods.
		HRESULT					Init(void);
		HRESULT					Fini(void);

		HRESULT					GenerateDefaults(void);

		HRESULT					InitInterfaces(void);
		HRESULT					FiniInterfaces(void);

		HRESULT					InitWindow(void);
		HRESULT					FiniWindow(void);

		HRESULT					InitFullscreenMode(void);
		HRESULT					FiniFullscreenMode(void);

		HRESULT					InitFront(void);
		HRESULT					FiniFront(void);

		HRESULT					InitPalette(void);
		HRESULT					FiniPalette(void);

		HRESULT					InitBack(void);
		HRESULT					FiniBack(void);

#ifdef TARGET_DC
		HRESULT					InitBackCache(void);
		HRESULT					FiniBackCache(void);
#endif

		HRESULT					InitViewport(void);
		HRESULT					FiniViewport(void);
		HRESULT					UpdateViewport(void);

		HRESULT					InitWork(void);
		HRESULT					FiniWork(void);

		HRESULT					InitClipper(void);
		HRESULT					FiniClipper(void);

		void					RunFMV();
		void					RunCutscene(int which, int language=0, bool bAllowButtonsToExit=TRUE );

		HRESULT					ChangeDriver(GUID *DD_guid,D3DDeviceInfo *device_hint,DDModeInfo *mode_hint);
		HRESULT					ChangeDevice(GUID *D3D_guid,DDModeInfo *mode_hint);
		HRESULT					ChangeMode(SLONG w,SLONG h,SLONG bpp,SLONG refresh);

		bool					IsGammaAvailable();
		void					SetGamma(int black = 0, int white = 256);
		void					GetGamma(int* black, int* white);

		HRESULT					Restore(void);

		HRESULT					AddLoadedTexture(D3DTexture *the_texture);	// Remember this texture
		void					RemoveAllLoadedTextures(void);				// Forget all textures
		HRESULT					FreeLoadedTextures(void);					// Destroy each texture you know
		HRESULT					ReloadTextures(void);

		HRESULT					toGDI(void);
		HRESULT					fromGDI(void);

		void                   *screen_lock  (void);
		void                    screen_unlock(void);

		void					MenuOn(void);
		void					MenuOff(void);
#ifndef TARGET_DC
		HRESULT					ShowWorkScreen(void);
#endif

		void					blit_back_buffer(void);
	
#ifdef TARGET_DC
		// Sod the user colour background stuff - it's junk.
		// Something always gets blitted over it anyway.
		void					SetUserColour(UBYTE red, UBYTE green, UBYTE blue) {}

		inline	HRESULT			SetBlackBackground(void)	{	return	DD_OK;	}
		inline	HRESULT			SetWhiteBackground(void)	{	return	DD_OK;	}
		inline	HRESULT			SetUserBackground(void)		{	return	DD_OK;	}
		inline	HRESULT			BeginScene(void)			{
																//TRACE("BeginScene ");
																if ( bInScene )
																{
																	return ( DD_OK );
																}
																else
																{
																	bInScene = TRUE;
																	return	lp_D3D_Device->BeginScene();
																}
															}
		inline	HRESULT			EndScene(void)				{	/*TRACE("EndScene ");*/ bInScene = FALSE; return	lp_D3D_Device->EndScene();	}
		inline	HRESULT			ClearViewport(void)			{
																HRESULT hres = lp_D3D_Viewport->Clear	(
																									1,
																									&ViewportRect,
																									D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
																								);
																return hres;
															}
#else //#ifdef TARGET_DC

		void					SetUserColour(UBYTE red, UBYTE green, UBYTE blue);

		inline	HRESULT			SetBlackBackground(void)	{	BackColour=BK_COL_BLACK;	return	lp_D3D_Viewport->SetBackground(black_handle);	}
		inline	HRESULT			SetWhiteBackground(void)	{	BackColour=BK_COL_WHITE;	return	lp_D3D_Viewport->SetBackground(white_handle);	}
		inline	HRESULT			SetUserBackground(void)		{	BackColour=BK_COL_USER;		return	lp_D3D_Viewport->SetBackground(user_handle);	}
		inline	HRESULT			BeginScene(void)			{	return	lp_D3D_Device->BeginScene();					}
		inline	HRESULT			EndScene(void)				{	return	lp_D3D_Device->EndScene();						}
		inline	HRESULT			ClearViewport(void)			{
																return	lp_D3D_Viewport->Clear	(
																									1,
																									&ViewportRect,
																									D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER
																								);
															}
#endif //#else //#ifdef TARGET_DC

		inline	HRESULT			SetRenderState(D3DRENDERSTATETYPE type,SLONG state)
															{
																return	lp_D3D_Device->SetRenderState(type,state);
															}
		inline	HRESULT			SetTexture(LPDIRECT3DTEXTURE2 tex)
															{
																ASSERT ( lp_D3D_Device != NULL );
																return lp_D3D_Device->SetTexture(0, tex);
															}
		inline	HRESULT			SetTextureState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD dw)
															{
																return lp_D3D_Device->SetTextureStageState(stage, type, dw);
															}
		
		HRESULT			Flip(LPDIRECTDRAWSURFACE4 alt,SLONG flags);

		inline	HRESULT			DrawPrimitive(D3DPRIMITIVETYPE p_type,DWORD dwVertexDesc,LPVOID v,DWORD v_count,DWORD flags)
															{
																return	lp_D3D_Device->DrawPrimitive(p_type,dwVertexDesc,v,v_count,flags);
															}
		inline	HRESULT			DrawIndexedPrimitive(D3DPRIMITIVETYPE p_type,DWORD dwVertexDesc,LPVOID v,DWORD v_count,LPWORD i,DWORD i_count,DWORD flags)
															{
																return	lp_D3D_Device->DrawIndexedPrimitive(p_type,dwVertexDesc,v,v_count,i,i_count,flags);
															}


		inline	BOOL			IsValid(void)				{	return	(((ValidFlags&DWF_VALID)==DWF_VALID) ? TRUE : FALSE);	}

		inline	BOOL			IsDisplayChanged(void)		{	return	(((AttribFlags&DWF_DISPLAY_CHANGED)==DWF_DISPLAY_CHANGED) ? TRUE : FALSE);	}
		inline	void			DisplayChangedOn(void)		{	AttribFlags		|=	DWF_DISPLAY_CHANGED;	}
		inline	void			DisplayChangedOff(void)		{	AttribFlags		&=	~DWF_DISPLAY_CHANGED;	}

		inline	BOOL			IsTexturesInvalid(void)		{	return	(((AttribFlags&DWF_TEXTURES_INVALID)==DWF_TEXTURES_INVALID) ? TRUE : FALSE);}
		inline	void			TexturesInvalidOn(void)		{	AttribFlags		|=	DWF_TEXTURES_INVALID;	}
		inline	void			TexturesInvalidOff(void)	{	AttribFlags		&=	~DWF_TEXTURES_INVALID;	}

		inline	BOOL			IsFullScreen(void)			{	return	(AttribFlags&DWF_FULLSCREEN);		}
		inline	void			FullScreenOn(void)			{	AttribFlags		|=	DWF_FULLSCREEN;			}
		inline	void			FullScreenOff(void)			{	AttribFlags		&=	~DWF_FULLSCREEN;		}

		inline	BOOL			IsCreateZBuffer(void)		{	return	(AttribFlags&DWF_ZBUFFER);			}
		inline	void			CreateZBufferOn(void)		{	AttribFlags		|=	DWF_ZBUFFER;			}
		inline	void			CreateZBufferOff(void)		{	AttribFlags		&=	~DWF_ZBUFFER;			}

		inline	BOOL			IsUse3D(void)				{	return	(AttribFlags&DWF_USE_3D);			}
		inline	void			Use3DOn(void)				{	AttribFlags		|=	DWF_USE_3D;				}
		inline	void			Use3DOff(void)				{	AttribFlags		&=	~DWF_USE_3D;			}

#ifndef TARGET_DC
		inline	BOOL			IsUseWork(void)				{	return	(AttribFlags&DWF_USE_WORK);			}
		inline	void			UseWorkOn(void)				{	AttribFlags		|=	DWF_USE_WORK;			}
		inline	void			UseWorkOff(void)			{	AttribFlags		&=	~DWF_USE_WORK;			}
#endif
		
		inline	BOOL			IsInitialised(void)			{	return	DisplayFlags&DISPLAY_INIT;			}
		inline	void			InitOn(void)				{	DisplayFlags	|=	DISPLAY_INIT;			}
		inline	void			InitOff(void)				{	DisplayFlags	&=	~DISPLAY_INIT;			}

		inline DDDriverInfo		*GetDriverInfo(void)		{	return	CurrDriver;							}
		inline DDModeInfo		*GetModeInfo(void)			{	return	CurrMode;							}
		inline D3DDeviceInfo	*GetDeviceInfo(void)		{	return	CurrDevice;							}

		inline ULONG			GetFormattedPixel(UBYTE r, UBYTE g, UBYTE b) {return ((r >> mask_red) << shift_red) | ((g >> mask_green) << shift_green) | ((b >> mask_blue) << shift_blue);}

		//
		// The screen must be locked for these functions to work!
		//

		void					PlotPixel         (SLONG x, SLONG y, UBYTE red, UBYTE green, UBYTE blue);
		void					PlotFormattedPixel(SLONG x, SLONG y, ULONG colour);
		void                    GetPixel          (SLONG x, SLONG y, UBYTE *red, UBYTE *green, UBYTE *blue);

		//
		// A background picture... The image must be the same dimensions as the
		// back buffer and the image data must remain valid while the surface
		// is alive.
		//

		void create_background_surface(UBYTE *image_data);	// Must be same dimensions as back buffer!
#if USE_COMPRESSED_BACKGROUNDS
		void use_this_background_surface(CompressedBackground this_one);
#else
		void use_this_background_surface(LPDIRECTDRAWSURFACE4 this_one);
#endif
		void blit_background_surface(bool b3DInFrame = TRUE);
		void destroy_background_surface(void);
};


#if USE_COMPRESSED_BACKGROUNDS
// This uses my funky pixel format.
// This only works to a 640x480,565 format image.
// Unpacks image_data into surface
extern void UnpackBackground ( UBYTE* image_data, IDirectDrawSurface4* surface );
// This uses my funky pixel format.
// This only works to a 640x480,565 format image.
// Packs surface (640x480,565 pixels - raw data) into image_data.
// Returns the size of the packed version. You can pass in NULL for image_data,
// and it will just return the size, and not try to copy anything.
extern int PackBackground ( UBYTE* image_data, WORD *surface );
#endif


//---------------------------------------------------------------

// they're macros now!
#define	DisplayWidth	640
#define DisplayHeight	480

extern SLONG			RealDisplayWidth;
extern SLONG			RealDisplayHeight;
extern SLONG			DisplayBPP;
extern Display			the_display;
extern volatile SLONG	hDDLibStyle,
						hDDLibStyleEx;
extern volatile	HMENU	hDDLibMenu;
extern volatile HWND	hDDLibWindow;

//---------------------------------------------------------------

#endif

