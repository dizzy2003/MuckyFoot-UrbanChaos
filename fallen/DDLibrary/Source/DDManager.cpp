//	DDManager.cpp
//	Guy Simmons, 12th November 1997.

#ifdef TARGET_DC
#define	INITGUID
#endif

#include	"DDLib.h"
#include	"fallen/headers/env.h"
#include	"fallen/headers/game.h"
#include	<tchar.h>
#ifdef	TARGET_DC
#include "target.h"
#endif


DDDriverManager		the_manager;

//---------------------------------------------------------------
//
//	Callback functions.
//
//---------------------------------------------------------------

BOOL WINAPI DriverEnumCallback	(
									GUID FAR	*lpGuid,
									LPTSTR		lpDesc,
									LPTSTR		lpName,
									LPVOID		lpExtra
								)
{
    CallbackInfo		*the_info;
    DDDriverInfo		*new_driver;
	HRESULT				result;


    if(!lpExtra)
    {
        // Programming error, invalid pointer
        return	DDENUMRET_OK;
    }

    the_info	=	(CallbackInfo*)lpExtra;

    // Get Pointer to driver info
    new_driver	=	MFnew<DDDriverInfo>();
    if(!new_driver)
    {
        // Error, Not enough memory to create driver
        return	DDENUMRET_OK;
    }

    // Setup the driver.
    result	=	new_driver->Create(lpGuid,lpName,lpDesc);
    if(FAILED(result))
    {
        // Error, Driver setup failed
        return	DDENUMRET_OK;
    }

	// Add To Global Driver List
	result	=	the_manager.AddDriver(new_driver);
	if(FAILED(result))
	{
		// Error, Driver Create Failed
		return	DDENUMRET_OK;
	}

    // Increment driver count
    the_info->Count++;

    // Success
    return	DDENUMRET_OK;
}

//---------------------------------------------------------------

HRESULT WINAPI ModeEnumCallback (
									LPDDSURFACEDESC2 lpDDSurfDesc,
									LPVOID           lpExtra
								)
{
    CallbackInfo	*the_info;
    DDDriverInfo	*the_driver;
	DDModeInfo		*new_mode;
	HRESULT			result;


    if(!lpExtra)
    {
        return DDENUMRET_OK;
    }

    the_info	=	(CallbackInfo*)lpExtra;
    the_driver	=	(DDDriverInfo*)the_info->Extra;

    if(!the_driver)
    {
        return DDENUMRET_OK;
    }

    if(!lpDDSurfDesc)
    {
        return DDENUMRET_CANCEL;
    }

    // Double check structure size
	ASSERT(lpDDSurfDesc->dwSize == sizeof(*lpDDSurfDesc));

	// Create Mode node
	new_mode	=	MFnew<DDModeInfo>();
	if(!new_mode)
	{
        // Error, not enough memory to store mode info
        return DDENUMRET_OK;
	}

    // Copy surface description
    new_mode->ddSurfDesc	=	*lpDDSurfDesc;

	// Add Mode to Driver Mode List
	result	=	the_driver->AddMode(new_mode);
	if(FAILED(result))
	{
        return DDENUMRET_OK;
	}

    // Update mode count
    the_info->Count++;

    return DDENUMRET_OK;
}

//---------------------------------------------------------------

HRESULT WINAPI DeviceEnumCallback	(
										LPGUID          lpGuid,
										LPTSTR          lpName,
										LPTSTR          lpDesc,
										LPD3DDEVICEDESC lpHalDevice,
										LPD3DDEVICEDESC lpHelDevice,
										LPVOID          lpExtra
									)
{
	CallbackInfo	*the_info;
	D3DDeviceInfo	*new_device;
	DDDriverInfo	*the_driver;
	HRESULT         result;

    if(!lpExtra)
    {
        // Programming error, invalid pointer
        return DDENUMRET_OK;
    }

    the_info	=	(CallbackInfo*)lpExtra;
    the_driver	=	(DDDriverInfo*)the_info->Extra;

    if(!the_driver)
    {
        // Programming Error, invalid pointer
        return DDENUMRET_OK;
    }

	// Create D3D Device node
	new_device	=	MFnew<D3DDeviceInfo>();
	if(!new_device)
	{
        // Not Enough memory to create D3D device node
        return DDENUMRET_OK;
	}

    // Initialize D3D Device info
    result	=	new_device->Create(lpGuid,lpName,lpDesc,lpHalDevice,lpHelDevice);
    if(FAILED(result))
    {
        // Error
        return DDENUMRET_OK;
    }

	//	If this is a hardware device then set the drivers hardware pointer.
	if(new_device->IsHardware())
	{
		the_driver->hardware_guid	=	*lpGuid;

		if (lpHalDevice->dwDeviceRenderBitDepth & DDBD_16) {the_driver->DriverFlags |= DD_DRIVER_RENDERS_TO_16BIT;}
		if (lpHalDevice->dwDeviceRenderBitDepth & DDBD_32) {the_driver->DriverFlags |= DD_DRIVER_RENDERS_TO_32BIT;}
	}

	// Add to Driver D3D Device list
	result	=	the_driver->AddDevice(new_device);
	if(FAILED(result))
	{
        // Error
        return DDENUMRET_OK;
	}

    // Update D3D device Driver count
    the_info->Count++;

    return DDENUMRET_OK;
}

//---------------------------------------------------------------

HRESULT WINAPI	TextureFormatEnumCallback	(
												LPDDPIXELFORMAT  lpTextureFormat,
												LPVOID			 lpExtra
											)
{
	CallbackInfo	*the_info;
	D3DDeviceInfo	*the_device;
	DDModeInfo		*new_format;
	HRESULT			result;


	if(!lpExtra)
	{
        // Programming error, invalid pointer
        return	DDENUMRET_OK;
	}

	the_info	=	(CallbackInfo*)lpExtra;
	the_device	=	(D3DDeviceInfo*)the_info->Extra;

	if(!the_device)
	{
		// Programming error, invalid pointer
		return	DDENUMRET_OK;
	}

	if(!lpTextureFormat)
	{
        // Error, invalid pointer
        return DDENUMRET_CANCEL;
	}

	// Double check structure size
	ASSERT(lpTextureFormat->dwSize == sizeof(*lpTextureFormat));

	// Create format node
	new_format	=	MFnew<DDModeInfo>();
	if(!new_format)
	{
		// Error, not enough memory to store format info
		return DDENUMRET_OK;
	}

	// Copy texture format description
	InitStruct(new_format->ddSurfDesc);

	new_format->ddSurfDesc.dwFlags = DDSD_PIXELFORMAT;
	new_format->ddSurfDesc.ddpfPixelFormat = *lpTextureFormat;

	// Add format to D3D device format list
	result	=	the_device->AddFormat(new_format);
	if(FAILED(result))
	{
		// Error, not enough memory to store mode info
		MFdelete(new_format);
		return	DDENUMRET_OK;
	}

	// Update format count
	the_info->Count++;

	return	DDENUMRET_OK;
}

//---------------------------------------------------------------

#ifndef TARGET_DC
HRESULT WINAPI ZFormatEnumCallback(LPDDPIXELFORMAT lpZFormat, LPVOID lpExtra)
{
	CallbackInfo	*the_info;
	D3DDeviceInfo	*the_device;
	DDModeInfo		*new_format;
	HRESULT			result;

	if(!lpExtra)
	{
        // Programming error, invalid pointer
        return	DDENUMRET_OK;
	}

	the_info	=	(CallbackInfo*)lpExtra;
	the_device	=	(D3DDeviceInfo*)the_info->Extra;

	if(!the_device)
	{
		// Programming error, invalid pointer
		return	DDENUMRET_OK;
	}

	if(!lpZFormat)
	{
        // Error, invalid pointer
        return DDENUMRET_CANCEL;
	}

	if (!the_info->Count)
	{
		//
		// Just in case there is no 16-bit zbuffer...
		//

		the_device->SetZFormat(lpZFormat);
	}


	//
	// Every card has a 16-bit zbuffer! That is what we want... and no stencil buffer!
	//

	if (lpZFormat->dwZBufferBitDepth == 16 &&
		lpZFormat->dwStencilBitDepth == 0)
	{
		the_device->SetZFormat(lpZFormat);
	}

	/*

	if (!the_device->IsHardware() && (lpZFormat->dwZBufferBitDepth != 16))
	{
		return DDENUMRET_OK;
	}

	// Double check structure size
	ASSERT(lpZFormat->dwSize == sizeof(*lpZFormat));

	// tell device
	if (!the_info->Count || (lpZFormat->dwZBufferBitDepth > the_device->GetZFormat()->dwZBufferBitDepth))
	{
		the_device->SetZFormat(lpZFormat);
	}

	*/

	// Update format count
	the_info->Count++;

	return	DDENUMRET_OK;
}
#endif

//---------------------------------------------------------------

SLONG	FlagsToBitDepth(SLONG flags)
{
	if(flags&DDBD_1)
		return	1;
	else if(flags&DDBD_2)
		return 2L;
	else if(flags&DDBD_4)
		return 4L;
	else if(flags&DDBD_8)
		return 8L;
	else if(flags&DDBD_16)
		return 16L;
	else if(flags&DDBD_24)
		return 24L;
	else if(flags&DDBD_32)
		return 32L;
	else
		return 0L;
}

ULONG	FlagsToMask(SLONG flags)
{
	if (flags & DDBD_1)		return	0x01;
	if (flags & DDBD_2)		return	0x03;
	if (flags & DDBD_4)		return	0x07;
	if (flags & DDBD_8)		return	0xFF;
	if (flags & DDBD_16)	return	0xFFFF;
	if (flags & DDBD_24)	return	0xFFFFFF;
	if (flags & DDBD_32)	return	0xFFFFFFFF;

	return 0;
}

//---------------------------------------------------------------

SLONG	BitDepthToFlags(SLONG bpp)
{
	switch(bpp)
	{
		case 1:		return	DDBD_1;
		case 2:		return	DDBD_2;
		case 4:		return	DDBD_4;
		case 8:		return	DDBD_8;
		case 16:	return	DDBD_16;
		case 24:	return	DDBD_24;
		case 32:	return	DDBD_32;
		default:	return	0; //ERROR.
	}
}

//---------------------------------------------------------------

BOOL	IsPalettized(LPDDPIXELFORMAT lp_dd_pf)
{
	if(!lp_dd_pf)
	{
        // Error,
		return FALSE;
	}

	if(lp_dd_pf->dwFlags&DDPF_PALETTEINDEXED1)
		return TRUE;

	if(lp_dd_pf->dwFlags&DDPF_PALETTEINDEXED2)
		return TRUE;

	if(lp_dd_pf->dwFlags&DDPF_PALETTEINDEXED4)
		return TRUE;

	if(lp_dd_pf->dwFlags&DDPF_PALETTEINDEXED8)
		return TRUE;

	// Not palettized
	return	FALSE;
}

//---------------------------------------------------------------

BOOL	GetDesktopMode	(
							DDDriverInfo	*the_driver,
							LPGUID			D3D_guid,
							DDModeInfo		**the_mode,
							D3DDeviceInfo	**the_device
						)
{
	SLONG			w,h,bpp;
	HDC				hdc;
	HWND			hDesktop;
	DDModeInfo		*new_mode;
	D3DDeviceInfo	*new_device,
					*next_best_device;


	// Check Parameters
	if((!the_driver) || (!the_mode) || (!the_device))
		return FALSE;

	// Get Desktop Mode info
	hDesktop	=	GetDesktopWindow();
	hdc			=	GetDC(hDesktop);

	w	=	GetDeviceCaps(hdc,HORZRES);
	h	=	GetDeviceCaps(hdc,VERTRES);
	bpp	=	GetDeviceCaps(hdc,PLANES) * GetDeviceCaps(hdc,BITSPIXEL);

	ReleaseDC(hDesktop,hdc);

	// Get Mode
	new_mode	=	the_driver->FindMode(w,h,bpp,0,NULL);
	if(!new_mode)
		return FALSE;

	// Get Compatible Device
	new_device	=	the_driver->FindDeviceSupportsMode(D3D_guid,new_mode,&next_best_device);
	if(!new_device)
	{
		if(!next_best_device)
			return FALSE;
		new_device	=	next_best_device;
	}

	// Save results
	*the_mode	=	new_mode;
	*the_device	=	new_device;

	// Success
	return TRUE;
}

//---------------------------------------------------------------

BOOL	GetFullscreenMode	(
								DDDriverInfo	*the_driver,
								GUID			*D3D_guid,
								SLONG			w,
								SLONG			h,
								SLONG			bpp,
								SLONG			refresh,
								DDModeInfo		**the_mode,
								D3DDeviceInfo	**the_device
							)
{
	D3DDeviceInfo	*new_device,
					*next_best_device;
	DDModeInfo		*new_mode,
					*next_best_mode;


	// Check Parameters
	if((!the_driver) || (!the_mode) || (!the_device))
		return FALSE;

	// Get D3D Device
	new_device	=	the_driver->FindDevice(D3D_guid,&next_best_device);
	if(!new_device)
	{
		if(!next_best_device)
			return	FALSE;
		new_device	=	next_best_device;
	}

	// Double check requested mode parameters
	if((w==0) || (h==0) || (bpp==0))
	{
		// Pick a reasonable full screen default
		// Most Hardware devices support 16 bpp,
		// many don't support 8 bpp, so pick 16
		w		=	DEFAULT_WIDTH;
		h		=	DEFAULT_HEIGHT;
		bpp		=	DEFAULT_DEPTH;
	}

	// Get Compatible Mode
	new_mode	=	the_driver->FindModeSupportsDevice(w,h,bpp,refresh,new_device,&next_best_mode);
	if(!new_mode)
	{
		if(!next_best_mode)
			return	FALSE;
		new_mode	=	next_best_mode;
	}

	// Save results
	*the_mode	=	new_mode;
	*the_device	=	new_device;

	// Success
	return TRUE;
}

//---------------------------------------------------------------
//
//	Validate functions.
//
//---------------------------------------------------------------

DDDriverInfo	*ValidateDriver(GUID *DD_guid)
{
	DDDriverInfo	*new_driver,
					*next_best_driver;


    // Find Driver matching specified GUID
    new_driver	=	the_manager.FindDriver(DD_guid, &next_best_driver);
    if(new_driver)
	{
		// Exact match
		return	new_driver;
	}

	// Return next best match (or failure)
	return	next_best_driver;
}

//---------------------------------------------------------------

D3DDeviceInfo	*ValidateDevice	(
									DDDriverInfo	*the_driver,
									GUID			*D3D_guid,
									DDModeInfo		*the_filter
								)
{
	D3DDeviceInfo	*new_device,
					*next_best_device;


	// Check Parameters
	if(!the_driver)
		return FALSE;

	if(!the_filter)
	{
		new_device	=	the_driver->FindDevice(D3D_guid, &next_best_device);
	}
	else
	{
		// Filter device against mode
		new_device	=	NULL; //the_driver->FindDeviceSupportsMode(D3D_guid, the_filter, &next_best_device);
	}

    if(new_device)
	{
		// Exact match
		return	new_device;
	}

	// Return next best match (or failure)
	return	next_best_device;
}

//---------------------------------------------------------------

DDModeInfo	*ValidateMode	(
								DDDriverInfo	*the_driver,
								DWORD			w,
								DWORD			h,
								DWORD			bpp,
								DWORD			refresh,
								D3DDeviceInfo	*the_filter
							)
{
	DDModeInfo		*new_mode,
					*next_best_mode;

	// Check Parameters
	if(!the_driver)
		return FALSE;

	if(!the_filter)
		new_mode	=	the_driver->FindMode(w, h, bpp, refresh, &next_best_mode);
	else
	{
		// Filter mode against D3D device compatiblity
		new_mode	=	the_driver->FindModeSupportsDevice(w, h, bpp, refresh, the_filter, &next_best_mode);
	}

    if(new_mode)
	{
		// Exact match
		return	new_mode;
	}

	// Return next best match (or failure)
	return	next_best_mode;
}

//---------------------------------------------------------------
//
//	CLASS	:	DDModeInfo
//
//---------------------------------------------------------------

DDModeInfo::DDModeInfo()
{
	InitStruct(ddSurfDesc);
	Prev	=	NULL;
	Next	=	NULL;
}

DDModeInfo::DDModeInfo(const DDSURFACEDESC & ddDesc)
{
	CopyMemory(&ddSurfDesc,(const void *)&ddDesc,sizeof (ddSurfDesc));
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));
	Prev	=	NULL;
	Next	=	NULL;
}

//---------------------------------------------------------------

DDModeInfo::~DDModeInfo()
{
	Prev	=	NULL;
	Next	=	NULL;
}

//---------------------------------------------------------------

SLONG	DDModeInfo::GetWidth(void)
{
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));

	// Check that Pixel format is valid
	if(!(ddSurfDesc.dwFlags & DDSD_WIDTH))
		return	0;

	// Get Bits Per Pixel
	return	ddSurfDesc.dwWidth;
}

//---------------------------------------------------------------

SLONG	DDModeInfo::GetHeight(void)
{
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));

	// Check that Pixel format is valid
	if(!(ddSurfDesc.dwFlags & DDSD_HEIGHT))
		return 0L;

	// Get Bits Per Pixel
	return	ddSurfDesc.dwHeight;
}

//---------------------------------------------------------------

SLONG	DDModeInfo::GetBPP(void)
{
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));
	ASSERT(ddSurfDesc.ddpfPixelFormat.dwSize == sizeof(ddSurfDesc.ddpfPixelFormat));

	// Check that Pixel format is valid
	if(!(ddSurfDesc.dwFlags & DDSD_PIXELFORMAT))
		return	0;

	// Assume it is RGB
	return	ddSurfDesc.ddpfPixelFormat.dwRGBBitCount;
}

//---------------------------------------------------------------

HRESULT DDModeInfo::GetMode(SLONG *w,SLONG *h,SLONG *bpp,SLONG *refresh)
{
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));
	ASSERT(ddSurfDesc.ddpfPixelFormat.dwSize == sizeof(ddSurfDesc.ddpfPixelFormat));

	// Check that width is valid
	if(!(ddSurfDesc.dwFlags & DDSD_WIDTH))
		return	DDERR_GENERIC;

	// Check that height is valid
	if(!(ddSurfDesc.dwFlags & DDSD_HEIGHT))
		return	DDERR_GENERIC;

	// Check that Pixel format is valid
	if(!(ddSurfDesc.dwFlags & DDSD_PIXELFORMAT))
		return	DDERR_GENERIC;

	// Get Width, height, BPP
	*w			=	(SLONG)ddSurfDesc.dwWidth;
	*h			=	(SLONG)ddSurfDesc.dwHeight;
	*bpp		=	(SLONG)ddSurfDesc.ddpfPixelFormat.dwRGBBitCount;
	*refresh	=	0L;

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

BOOL	DDModeInfo::ModeSupported(D3DDeviceInfo *the_device)
{
	SLONG		bpp,
				depths,
				depth_flags;


	// Check Parameters
	if(!the_device)
		return FALSE;

	// Make sure D3D device supports this mode
	bpp			=	GetBPP();
	depth_flags	=	BitDepthToFlags(bpp);
	depths		=	0;

	// Get Supported Bit Depths for this D3D device
	if(the_device->IsHardware())
		depths	=	the_device->d3dHalDesc.dwDeviceRenderBitDepth;
	else
		depths	=	the_device->d3dHelDesc.dwDeviceRenderBitDepth;

	if(depths & depth_flags)
	{
		// Supported !!!
		return TRUE;
	}

	// Not Supported !!!
	return FALSE;
}

//---------------------------------------------------------------

BOOL	DDModeInfo::Match(SLONG w,SLONG h,SLONG bpp)
{
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));

	// Check for width & height match.
	if((ddSurfDesc.dwWidth==(DWORD)w) && (ddSurfDesc.dwHeight==(DWORD)h))
	{
		// Check for bpp match.
		if(ddSurfDesc.ddpfPixelFormat.dwRGBBitCount==(DWORD)bpp)
		{
			// Check for palettized mode.
			if(bpp<=8 && !IsPalettized(&ddSurfDesc.ddpfPixelFormat))
			{
				return	FALSE;
			}

			return	TRUE;
		}
	}

    return FALSE;
}

//---------------------------------------------------------------

BOOL	DDModeInfo::Match(SLONG bpp)
{
	ASSERT(ddSurfDesc.dwSize == sizeof(ddSurfDesc));

	// Check for bpp match.
	if(ddSurfDesc.ddpfPixelFormat.dwRGBBitCount==(DWORD)bpp)
	{
		// Check for palettized mode.
		if(bpp<=8 && !IsPalettized(&ddSurfDesc.ddpfPixelFormat))
		{
			return	FALSE;
		}

		return	TRUE;
	}


	return FALSE;
}

//---------------------------------------------------------------
//
//	CLASS	:	D3DDeviceInfo
//
//---------------------------------------------------------------

D3DDeviceInfo::D3DDeviceInfo()
{
	D3DFlags		=	0;

	szName			=	NULL;
	szDesc			=	NULL;

	InitStruct(d3dHalDesc);
	InitStruct(d3dHelDesc);

	FormatCount		=	0;
	FormatList		=	NULL;
	FormatListEnd	=	NULL;
	OpaqueTexFmt	=	NULL;
	AlphaTexFmt		=	NULL;
#ifndef TARGET_DC
	CanDoModulateAlpha = false;
	CanDoDestInvSourceColour = false;
#endif

	Prev			=	NULL;
	Next			=	NULL;

	memset(&ZFormat, 0, sizeof(ZFormat));
}

//---------------------------------------------------------------

D3DDeviceInfo::~D3DDeviceInfo()
{
	Destroy();
}

//---------------------------------------------------------------

HRESULT	D3DDeviceInfo::Create	(
									LPGUID          lpD3DGuid,
									LPTSTR          lpD3DName,
									LPTSTR          lpD3DDesc,
									LPD3DDEVICEDESC lpD3DHal,
									LPD3DDEVICEDESC lpD3DHel
								)
{
	ULONG			str_len,
					str_size;
	LPTSTR			szTemp;


	// Copy GUID
    if(!lpD3DGuid)
        return DDERR_INVALIDPARAMS;
    guid	=	*lpD3DGuid;

	// Copy Name
    if(!lpD3DName)
		szTemp	=	TEXT("UNKNOWN");
    else
        szTemp	=	lpD3DName;

    str_len	=	_tcslen(szTemp);
	str_size=	(str_len+1)*sizeof(TCHAR);
	szName	=	(LPTSTR)MemAlloc(str_size);
	if(szName)
		_tcscpy(szName,szTemp);

    // Copy Description
    if(!lpD3DDesc)
		szTemp	=	TEXT("UNKNOWN");
    else
        szTemp	=	lpD3DDesc;

    str_len	=	_tcslen(szTemp);
	str_size=	(str_len+1)*sizeof(TCHAR);
	szDesc	=	(LPTSTR)MemAlloc(str_size);
	if(szDesc)
		_tcscpy(szDesc,szTemp);

    // Copy D3D info
    if(lpD3DHal)
        d3dHalDesc	=	*lpD3DHal;

    if (lpD3DHel)
        d3dHelDesc	=	*lpD3DHel;

	// Mark Texture format list as not loaded
	FormatCount	=	0;

	// Mark as valid
    ValidOn();

    return DD_OK;
}

//---------------------------------------------------------------

void	D3DDeviceInfo::Destroy (void)
{
	// Destroy Texture Formats
//	DestroyFormats();

	// Clean up strings
	if(szDesc)
	{
		MemFree(szDesc);
		szDesc	=	NULL;
	}

	if(szName)
	{
		MemFree(szName);
		szName	=	NULL;
	}

	Prev	=	NULL;
	Next	=	NULL;
}

//---------------------------------------------------------------

void D3DDeviceInfo::CheckCaps(LPDIRECT3DDEVICE3 the_device)
{
	D3DDEVICEDESC	hw;
	D3DDEVICEDESC	sw;
	HRESULT			rc;

	InitStruct(hw);
	InitStruct(sw);

	rc = the_device->GetCaps(&hw, &sw);
	if (FAILED(rc))	return;

#ifndef TARGET_DC
	if (hw.dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA)
	{
		CanDoModulateAlpha = true;
		TRACE("Card can do MODULATEALPHA\n");
	}
	else
	{
		CanDoModulateAlpha = false;
		TRACE("Card *cannot* do MODULATEALPHA\n");
	}

	if (hw.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCCOLOR)
	{
		CanDoDestInvSourceColour = true;
		TRACE("Card can do INVSRCCOLOR\n");
	}
	else
	{
		CanDoDestInvSourceColour = false;
		TRACE("Card *cannot* do INVSRCCOLOR\n");
	}


	if ((hw.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_SRCCOLOR) &&
		(hw.dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_DESTCOLOR))
	{
		CanDoAdamiLighting = true;
		TRACE("Card can do ADAMI LIGHTING\n");
	}
	else
	{
		CanDoAdamiLighting = false;
		TRACE("Card *cannot* do ADAMI LIGHTING\n");
	}

	SLONG adami_lighting = ENV_get_value_number("Adami_lighting", -1, "Render");

	if (adami_lighting == -1)
	{
		ENV_set_value_number("Adami_lighting", 1, "Render");

		adami_lighting = 1;
	}

	if (adami_lighting == 0)
	{
		CanDoAdamiLighting = false;

		TRACE("Overriding ADAMI LIGHTING\n");
	}
#endif


}

// Notes:
//
// RagePro *cannot* do MODULATEALPHA but *can* do INVSRCCOLOR

//---------------------------------------------------------------

HRESULT	D3DDeviceInfo::LoadFormats(LPDIRECT3DDEVICE3 the_d3d_device)
{
	CallbackInfo	callback_info;
	HRESULT			result;


	// Have we already loaded the texture formats
	if(!FormatsLoaded())
	{
		// Check Parameters
		if(!the_d3d_device)
		{
			result	=	DDERR_GENERIC;
			// Output error.
			return	result;
		}

		// Enumerate all Texture Formats for this device
		callback_info.Result	=	TRUE;
		callback_info.Count		=	0L;
		callback_info.Extra		=	(void*)this;

		result	=	the_d3d_device->EnumTextureFormats(TextureFormatEnumCallback,(void *)&callback_info);
		if(FAILED(result))
		{
			// Error
			// Output error.
			return	result;
		}

		// Double check count
		if((!callback_info.Result) || (callback_info.Count==0) || (FormatCount!=callback_info.Count))
		{
			result	=	DDERR_GENERIC;
			// Output error.
			return	result;
		}

		// discover our preferred texture formats
		FindOpaqueTexFmt();
		FindAlphaTexFmt();

		// Mark texture formats as loaded
		TurnFormatsLoadedOn();
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

void D3DDeviceInfo::FindOpaqueTexFmt()
{
	OpaqueTexFmt = NULL;

	SLONG	best_score = 0;

	for (DDModeInfo* mi = FormatList; mi; mi = mi->Next)
	{
		if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_RGB)
		{
			//
			// True colour...
			//

			if (mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount >= 16)
			{
				SLONG score  = 0x100;
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
					OpaqueTexFmt = mi;
				}
			}
		}
	}

	ASSERT(OpaqueTexFmt);
}

//---------------------------------------------------------------

extern void OS_calculate_mask_and_shift(ULONG bitmask, SLONG* mask,	SLONG* shift);

void D3DDeviceInfo::FindAlphaTexFmt()
{
	SLONG try_shift_alpha;
	SLONG try_shift_red;
	SLONG try_shift_green;
	SLONG try_shift_blue;

	SLONG try_mask_alpha;
	SLONG try_mask_red;
	SLONG try_mask_green;
	SLONG try_mask_blue;

	AlphaTexFmt = NULL;

	SLONG	best_score = 0;

	for (DDModeInfo* mi = FormatList; mi; mi = mi->Next)
	{
		if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_RGB)
		{
			if (mi->ddSurfDesc.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
			{
				if (mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount >= 16)
				{
					SLONG	score;

					//
					// Find out how many bits there are for each component.
					//

					OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRGBAlphaBitMask, &try_mask_alpha, &try_shift_alpha);
					OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwRBitMask       , &try_mask_red,   &try_shift_red  );
					OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwGBitMask       , &try_mask_green, &try_shift_green);
					OS_calculate_mask_and_shift(mi->ddSurfDesc.ddpfPixelFormat.dwBBitMask       , &try_mask_blue,  &try_shift_blue );

					//
					// Rate this texture. We prefer the same number of bits per element
					// and then go for the format with the least bits per pixel to save
					// memory.
					//

					if (try_mask_alpha == try_mask_red   &&
						try_mask_alpha == try_mask_green &&
						try_mask_alpha == try_mask_blue)
					{
						score = 0x400;
					}
					else if (try_mask_red == try_mask_green &&
							 try_mask_red == try_mask_blue)
					{
						score = 0x300;
					}
					else
					{
						score = 0x200;
					}

					if (try_mask_alpha == 7) // i.e. only 1 bit of alpha
					{
						score = 0x100;
					}

					score -= mi->ddSurfDesc.ddpfPixelFormat.dwRGBBitCount;

					if (score > best_score)
					{
						best_score   = score;
						AlphaTexFmt = mi;
					}
				}
			}
		}
	}

	ASSERT(AlphaTexFmt);
}

//---------------------------------------------------------------

HRESULT	D3DDeviceInfo::LoadZFormats(LPDIRECT3D3 d3d)
{
#ifdef TARGET_DC
	// The DC pretends to have a 32-bit Z-buffer, which is fine.
	DDPIXELFORMAT ZFormat;
	ZeroMemory ( &ZFormat, sizeof ( ZFormat ) );
	ZFormat.dwSize = sizeof ( ZFormat );
	ZFormat.dwFlags = DDPF_ZBUFFER;
	ZFormat.dwZBufferBitDepth = 32;
	SetZFormat(&ZFormat);
#else
	CallbackInfo	callback_info;
	HRESULT			result;

	// Enumerate all Z formats for this device
	callback_info.Result	=	TRUE;
	callback_info.Count		=	0;
	callback_info.Extra		=	(void*)this;

	result = d3d->EnumZBufferFormats(guid, ZFormatEnumCallback,(void*)&callback_info);
	if (FAILED(result))
	{
		return result;
	}

	if (!callback_info.Result || !callback_info.Count)
	{
		result = DDERR_GENERIC;
		return result;
	}
#endif

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	D3DDeviceInfo::DestroyFormats(void)
{
	DDModeInfo		*current_format,
					*next_format;


	if(FormatsLoaded())
	{
		current_format	=	FormatList;

		while(current_format)
		{
			next_format	=	current_format->Next;

			MFdelete(current_format);

			current_format	=	next_format;
		}

		FormatCount		=	0;
		FormatList		=	NULL;
		FormatListEnd	=	NULL;

		// Mark as unloaded
		TurnFormatsLoadedOff();
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	D3DDeviceInfo::AddFormat(DDModeInfo *the_format)
{
	// Check Parameters
	if(!the_format)
	{
		// Error, Invalid parameters
		return	DDERR_INVALIDPARAMS;
	}

	// Add Format to end of list.
	the_format->Prev	=	FormatListEnd;
	the_format->Next	=	NULL;

	// Update list end.
	if(FormatListEnd)
		FormatListEnd->Next	=	the_format;
	FormatListEnd	=	the_format;

	// Update List.
	if(!FormatList)
		FormatList	=	the_format;

	FormatCount++;

	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	D3DDeviceInfo::DelFormat(DDModeInfo	*the_format)
{
	return DD_OK;
}

//---------------------------------------------------------------

BOOL	D3DDeviceInfo::IsHardware(void)
{
	SLONG	colour_model;


	colour_model	=	d3dHalDesc.dcmColorModel;
	if(colour_model)
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------

BOOL	D3DDeviceInfo::Match(GUID *the_guid)
{
    if(the_guid==NULL)
        return	FALSE;

    if(!IsValid())
        return	FALSE;

    if(*the_guid!=guid)
        return	FALSE;

    // Success
    return TRUE;
}

//---------------------------------------------------------------

DDModeInfo	*D3DDeviceInfo::FindFormat	(
											SLONG			bpp,
											DDModeInfo		**next_best_format,
											DDModeInfo		*start
										)
{
	DDModeInfo		*current_format;


	// Get Starting node
	if(!start)
		current_format	=	FormatList;
	else
		current_format	=	start;

	if(next_best_format)
		*next_best_format	=	current_format;

	// Search format list for best match
	while(current_format)
	{
		if(current_format->Match(bpp))
		{
			return	current_format;
		}
		current_format	=	current_format->Next;
	}

	// Failure, user may use lpNextBest instead
	return NULL;
}

//---------------------------------------------------------------
//
//	CLASS	:	DDDriverInfo
//
//---------------------------------------------------------------

DDDriverInfo::DDDriverInfo()
{
	DriverFlags	=	0;

	InitStruct(ddHalCaps);
	InitStruct(ddHelCaps);

	ModeCount		=	0;
	ModeList		=	NULL;
	ModeListEnd		=	NULL;

	DeviceCount		=	0;
	DeviceList		=	NULL;
	DeviceListEnd	=	NULL;

	Next		=	NULL;
	Prev		=	NULL;
}

//---------------------------------------------------------------

DDDriverInfo::~DDDriverInfo()
{
	Destroy();
}

//---------------------------------------------------------------

HRESULT DDDriverInfo::Create(
								GUID	*lpGuid,
								LPTSTR	lpDriverName,
								LPTSTR	lpDriverDesc
							)
{
	ULONG			str_len,
					str_size;
    HRESULT         result;
    LPDIRECTDRAW    lpDD	=	NULL;
    LPDIRECTDRAW4   lpDD4	=	NULL;
    LPDIRECT3D3     lpD3D	=	NULL;
	LPTSTR			szTemp;


	if(IsValid())
    {
		// Programmer Error, already valid, call Fini to cleanup
        return FALSE;
    }

	// Copy GUID
    if(!lpGuid)
        PrimaryOn();
    else
        guid	=	*lpGuid;

    // Copy Name
    if(!lpDriverName)
		szTemp	=	TEXT("UNKNOWN");
    else
		szTemp	=	lpDriverName;

	str_len	=	_tcslen(szTemp);
	str_size=	(str_len+1)*sizeof(TCHAR);
	szName	=	(LPTSTR)MemAlloc(str_size);
	if(szName)
		_tcscpy(szName,szTemp);

    // Copy Desc
    if(!lpDriverDesc)
		szTemp	=	TEXT("UNKNOWN");
    else
		szTemp	=	lpDriverDesc;

	str_len	=	_tcslen(szTemp);
	str_size=	(str_len+1)*sizeof(TCHAR);
	szDesc	=	(LPTSTR)MemAlloc(str_size);
	if(szDesc)
		_tcscpy(szDesc,szTemp);

    // Create DirectDraw Object
    result	=	DirectDrawCreate(lpGuid,&lpDD,NULL);		  //BCleak
    if(FAILED(result))
    {
        // Error
        goto	cleanup;
    }

    // Get The DirectDraw4 Interface
    result	=	lpDD->QueryInterface(IID_IDirectDraw4,(void **)&lpDD4);
    if(FAILED(result))
    {
        // Error
        goto	cleanup;
    }

    // Get The Direct3D Interface
    result	=	lpDD->QueryInterface(IID_IDirect3D3,(void **)&lpD3D);
    if(FAILED(result))
    {
        // Error
        goto	cleanup;
    }

    // Get The Driver Caps
    result	=	lpDD4->GetCaps(&ddHalCaps,&ddHelCaps);
    if(FAILED(result))
    {
        // Error
        goto	cleanup;
    }

	// Enumerate all D3D Devices for this driver
	DeviceCount	=	0;
	result	=	LoadDevices(lpD3D);
	if(FAILED(result))
		goto	cleanup;


	// Enumerate all Modes for this DD Driver
	ModeCount	=	0;
	result		=	LoadModes(lpDD4);
	if(FAILED(result))
		goto	cleanup;

	//
	// Does this driver have less than 4 meg of video memory?
	//

	{
		DDCAPS ddcaps;

		memset(&ddcaps, 0, sizeof(ddcaps));

		ddcaps.dwSize = sizeof(ddcaps);

		lpDD4->GetCaps(&ddcaps, NULL);

		SLONG total = ddcaps.dwVidMemTotal;

		if (total < 5 * 1024 * 1024)
		{
			DriverFlags |= DD_DRIVER_LOW_MEMORY;
		}
	}

    // Mark as Valid Driver.
    ValidOn();

    // Success.
    result	=	DD_OK;

cleanup:
	// Cleanup the Interfaces before leaving
    if(lpD3D)
    {
        lpD3D->Release();
        lpD3D	=	NULL;
    }

    if(lpDD4)
    {
        lpDD4->Release();
        lpDD4	=	NULL;
    }

    if(lpDD)
    {
        lpDD->Release();
        lpDD	=	NULL;
    }

	return	result;
}

//---------------------------------------------------------------

void	DDDriverInfo::Destroy(void)
{
	// Destroy all Modes and Devices.
	DestroyDevices();
	DestroyModes();

	// Clean up strings
	if(szDesc)
	{
		MemFree(szDesc);
		szDesc	=	NULL;
	}

	if(szName)
	{
		MemFree(szName);
		szName	=	NULL;
	}

	Prev	=	NULL;
	Next	=	NULL;

    ValidOff();
}

//---------------------------------------------------------------

BOOL	DDDriverInfo::Match(GUID *the_guid)
{
	if(!IsValid())
        return	FALSE;

	if(!the_guid)
    {
		/*
		if (the_display.IsFullScreen())
		{
			if(!IsPrimary())
				return	TRUE;
		}
		else
		*/
		{
			if(IsPrimary())
				return	TRUE;
		}
    }
    else
    {
		if(*the_guid==guid)
            return	TRUE;
    }

    return FALSE;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::LoadModes(LPDIRECTDRAW4 lpDD4)
{
	CallbackInfo	callback_info;
	HRESULT			result;


	// Have we already loaded the modes
	if(!ModesLoaded())
	{
		// Check Parameters
		if(!lpDD4)
		{
			result	=	DDERR_GENERIC;
			return result;
		}

		// Enumerate all modes for this driver.
		callback_info.Result	=	TRUE;
		callback_info.Count		=	0L;
		callback_info.Extra		=	(void*)this;

		result	=	lpDD4->EnumDisplayModes(0L,NULL,&callback_info,ModeEnumCallback);
		if(FAILED(result))
		{
			return result;
		}

		// Double check count.
		if((!callback_info.Result) || (callback_info.Count == 0) || (ModeCount != callback_info.Count))
		{
			result	=	DDERR_GENERIC;
			return result;
		}

		// Mark Modes as loaded
		TurnModesLoadedOn();
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::DestroyModes(void)
{
	DDModeInfo		*current_mode,
					*next_mode;


	current_mode	=	ModeList;
	while(current_mode)
	{
		next_mode		=	current_mode->Next;

		MFdelete(current_mode);

		current_mode	=	next_mode;
	}

	ModeCount	=	0;
	ModeList	=	NULL;
	ModeListEnd	=	NULL;

	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::AddMode(DDModeInfo *the_mode)
{
	if(!the_mode)
	{
		// Error, Invalid parameters
		return	DDERR_INVALIDPARAMS;
	}

	{
		// Add Mode to end of List.
		the_mode->Prev	=	ModeListEnd;
		the_mode->Next	=	NULL;

		// Update list end.
		if(ModeListEnd)
			ModeListEnd->Next=	the_mode;
		ModeListEnd		=	the_mode;

		// Update list.
		if(!ModeList)
			ModeList		=	the_mode;

		//
		// Is this a 16-bit, a 24-bit or a 32-bit mode?
		//

		if (the_mode->GetBPP() == 16)
		{
			if (DriverFlags & DD_DRIVER_RENDERS_TO_16BIT)
			{
				DriverFlags |= DD_DRIVER_SUPPORTS_16BIT;
			}
		}
		else
		if (the_mode->GetBPP() == 32)
		{
			if (DriverFlags & DD_DRIVER_RENDERS_TO_32BIT)
			{
				DriverFlags |= DD_DRIVER_SUPPORTS_32BIT;
			}
		}

		//
		// What about the resolution?
		//

		if (the_mode->GetWidth()  ==  320 && the_mode->GetHeight() == 240) {DriverFlags |= DD_DRIVER_MODE_320;}
		if (the_mode->GetWidth()  ==  512 && the_mode->GetHeight() == 384) {DriverFlags |= DD_DRIVER_MODE_512;}
		if (the_mode->GetWidth()  ==  640 && the_mode->GetHeight() == 480) {DriverFlags |= DD_DRIVER_MODE_640;}
		if (the_mode->GetWidth()  ==  800 && the_mode->GetHeight() == 600) {DriverFlags |= DD_DRIVER_MODE_800;}
		if (the_mode->GetWidth()  == 1024 && the_mode->GetHeight() == 768) {DriverFlags |= DD_DRIVER_MODE_1024;}


		ModeCount++;
	}

	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::DeleteMode(DDModeInfo *the_mode)
{
	return	DD_OK;
}


//---------------------------------------------------------------

DDModeInfo	*DDDriverInfo::FindMode	(
										SLONG			w,
										SLONG			h,
										SLONG			bpp,
										SLONG			refresh,
										DDModeInfo		**next_best,
										DDModeInfo		*start_mode
									)
{
	DDModeInfo		*current_mode;


	// Get Starting node
	if(!start_mode)
		current_mode	=	ModeList;
	else
		current_mode	=	start_mode;

	if(next_best)
		*next_best		=	current_mode;

	// Search mode list for best match
    while(current_mode)
	{
        if(current_mode->Match(w,h,bpp))
        {
			return	current_mode;
        }
        else if	(
					current_mode->Match	(
											DEFAULT_WIDTH,
											DEFAULT_HEIGHT,
											8
										)
				)
        {
			if(next_best)
				*next_best	=	current_mode;
        }
		current_mode	=	current_mode->Next;
    }

    // Failure, user may use lpNextBest instead
    return NULL;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::LoadDevices(LPDIRECT3D3 lpD3D3)
{
	CallbackInfo	callback_info;
	HRESULT			result;


	// Have we already loaded the D3D Devices for this driver.
	if(!DevicesLoaded())
	{
		// Check Parameters
		if (! lpD3D3)
		{
			result	=	DDERR_GENERIC;
			return	result;
		}

		// Enumerate all D3D Devices for this driver.
		callback_info.Result	=	TRUE;
		callback_info.Count		=	0L;
		callback_info.Extra		=	(void*)this;

		result	=	lpD3D3->EnumDevices(DeviceEnumCallback, &callback_info);
		if(FAILED(result))
		{
			return	result;
		}

		// Double check count.
		if((!callback_info.Result) || (callback_info.Count==0) || (DeviceCount!=callback_info.Count))
		{
			result	=	DDERR_GENERIC;
			return	result;
		}

		// Mark Devices as loaded
		TurnDevicesLoadedOn();
	}

	// Success
	return DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::DestroyDevices(void)
{
	D3DDeviceInfo	*current_device,
					*next_device;


	current_device	=	DeviceList;
	while(current_device)
	{
		next_device		=	current_device->Next;

		MFdelete(current_device);

		current_device	=	next_device;
	}

	DeviceCount		=	0;
	DeviceList		=	NULL;
	DeviceListEnd	=	NULL;

	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::AddDevice(D3DDeviceInfo *the_device)
{
	if(!the_device)
	{
		// Error, Invalid parameters
		return	DDERR_INVALIDPARAMS;
	}

	// Add Device to end of List.
	the_device->Prev	=	DeviceListEnd;
	the_device->Next	=	NULL;

	// Update list end.
	if(DeviceListEnd)
		DeviceListEnd->Next	=	the_device;
	DeviceListEnd		=	the_device;

	// Update list.
	if(!DeviceList)
		DeviceList			=	the_device;

	DeviceCount++;

	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverInfo::DeleteDevice(D3DDeviceInfo *the_device)
{
	return	DD_OK;
}

//---------------------------------------------------------------

D3DDeviceInfo	*DDDriverInfo::FindDevice	(
												GUID			*the_guid,
												D3DDeviceInfo	**next_best,
												D3DDeviceInfo	*start_device
											)
{
	D3DDeviceInfo	*current_device,
					*first_device		=	NULL,
					*hardware_device	=	NULL,
					*mmx_device			=	NULL,
					*rgb_device			=	NULL;


	if(next_best)
		*next_best	=	NULL;

	// Get Root
	if(!start_device)
		current_device	=	DeviceList;
	else
		current_device	=	start_device;

	first_device		=	current_device;

	// Search mode list for best match
    while(current_device)
	{
		if(!SOFTWARE)
		{
			if(current_device->Match(the_guid))
				return	current_device;



			if(current_device->IsHardware())
				if(!hardware_device)
					hardware_device	=	current_device;

		}

		if(current_device->guid==IID_IDirect3DRGBDevice)
			if(!rgb_device)
			{
				rgb_device		=	current_device;
				if(SOFTWARE)
					return	current_device; //added by MD to force software
			}

		if(current_device->guid==IID_IDirect3DMMXDevice)
			if(!mmx_device)
			{
				mmx_device		=	current_device;
				if(SOFTWARE)
					return	current_device; //added by MD to force software
			}

		current_device	=	current_device->Next;
    }

	if(next_best)
	{
		if(hardware_device)
			*next_best	=	hardware_device;
		else if(rgb_device)
			*next_best	=	rgb_device;
		else if(mmx_device)
			*next_best	=	mmx_device;
		else if(first_device)
			*next_best	=	first_device;
	}

    // Failure, user may use lpNextBest instead
    return NULL;
}

//---------------------------------------------------------------

D3DDeviceInfo	*DDDriverInfo::FindDeviceSupportsMode	(
															GUID			*the_guid,
															DDModeInfo		*the_mode,
															D3DDeviceInfo	**next_best_device,
															D3DDeviceInfo	*start_device
														)
{
	D3DDeviceInfo		*current_device;


	// Check parameters
	if(!the_mode)
	{
		// Error, Invalid parameters
		if(next_best_device)
			*next_best_device	=	NULL;
		return	NULL;
	}

	// Get Root
	if(!start_device)
		current_device	=	DeviceList;
	else
		current_device	=	start_device;

	if(next_best_device)
	{
		if(the_mode->ModeSupported(current_device))
			*next_best_device	=	current_device;
	}

	//	Search the device list for correct device.
    while(current_device)
	{
		if(current_device->Match(the_guid))
		{

//			if(SOFTWARE==0 || !current_device->IsHardware()) //accept this mode if software not required or if it is software_required and not hardware
			if(the_mode->ModeSupported(current_device))
				return	current_device;
		}
		else if(current_device->IsHardware())
        {
//			if(SOFTWARE==0) // if asking for software dont let any hardware drivers through the net
			if(next_best_device)
			{
				if(the_mode->ModeSupported(current_device))
					*next_best_device	=	current_device;
			}

        }

		else if(the_mode->ModeSupported(current_device))
		{
			if(next_best_device)
				*next_best_device=	current_device;
		}
		current_device	=	current_device->Next;
    }

    // Failure, user may use lpNextBest instead
    return NULL;
}

//---------------------------------------------------------------

DDModeInfo	*DDDriverInfo::FindModeSupportsDevice	(
														SLONG			w,
														SLONG			h,
														SLONG			bpp,
														SLONG			refresh,
														D3DDeviceInfo	*the_device,
														DDModeInfo		**next_best,
														DDModeInfo		*start_mode
													)
{
	DDModeInfo		*current_mode;


	// Check parameters
	if(!the_device)
	{
		// Error, Invalid parameters
		if(next_best)
			*next_best	=	NULL;
		return NULL;
	}

	// Get Root
	if(!start_mode)
		current_mode	=	ModeList;
	else
		current_mode	=	start_mode;

	if(next_best)
	{
		if(current_mode->ModeSupported(the_device))
			*next_best	=	current_mode;
	}

	// Search mode list for best match
    while(current_mode)
	{
        if(current_mode->Match(w,h,bpp))
        {
			if(current_mode->ModeSupported(the_device))
				return	current_mode;
        }
        else if	(
					current_mode->Match	(
											DEFAULT_WIDTH,
											DEFAULT_HEIGHT,
											DEFAULT_DEPTH
										)
				)
        {
			if(next_best)
			{
				if(current_mode->ModeSupported(the_device))
					*next_best	=	current_mode;
			}
        }
		else if(current_mode->ModeSupported(the_device))
		{
			if(next_best)
				*next_best	=	current_mode;
		}
		current_mode	=	current_mode->Next;
    }

    // Failure, user may use lpNextBest instead
    return	NULL;
}

//---------------------------------------------------------------

GUID	*DDDriverInfo::GetGuid(void)
{
	if(IsPrimary())
		return	NULL;
	else
		return	&guid;
}

//---------------------------------------------------------------
//
//	CLASS	:	DDDriverManager
//
//---------------------------------------------------------------

DDDriverManager::DDDriverManager()
{
	ManagerFlags		=	0;
	DriverCount			=	0;
	DriverList			=	NULL;
	DriverListEnd		=	NULL;

	CurrDriver			=	NULL;
	CurrMode			=	NULL;
	CurrDevice			=	NULL;
	CurrTextureFormat	=	NULL;
}

//---------------------------------------------------------------

DDDriverManager::~DDDriverManager()
{
	Fini();
}

//---------------------------------------------------------------

HRESULT	DDDriverManager::Init(void)
{
	HRESULT			result;


	if(!IsInitialised())
	{
		result	=	LoadDrivers();
		if(FAILED(result))
			return	result;

		InitOn();
	}
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverManager::Fini(void)
{
	if(IsInitialised())
	{
		DestroyDrivers();

		InitOff();
	}
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT DDDriverManager::LoadDrivers(void)
{
	CallbackInfo	callback_info;
	HRESULT			result;


    // Initialize all valid drivers in system
    callback_info.Result	=	TRUE;
    callback_info.Count		=	0L;
    callback_info.Extra		=	(void*)NULL;

    result	=	DirectDrawEnumerate(DriverEnumCallback,&callback_info);
	if(FAILED(result))
	{
		return result;
	}

	// Double check count.
	if((!callback_info.Result) || (callback_info.Count == 0) || (DriverCount != callback_info.Count))
	{
		result	=	DDERR_GENERIC;
		return result;
	}

	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT DDDriverManager::DestroyDrivers(void)
{
	DDDriverInfo	*current_driver,
					*next_driver;


	current_driver	=	DriverList;
	while(current_driver)
	{
		next_driver		=	current_driver->Next;

		MFdelete(current_driver);

		current_driver	=	next_driver;
	}
	return	DD_OK;
}

//---------------------------------------------------------------

HRESULT	DDDriverManager::AddDriver(DDDriverInfo *the_driver)
{
	if(!the_driver)
	{
		// Error, Invalid parameters
		return	DDERR_INVALIDPARAMS;
	}

	// Add driver to list.
	the_driver->Prev	=	DriverListEnd;
	the_driver->Next	=	NULL;

	// Update list end.
	if(DriverListEnd)
		DriverListEnd->Next	=	the_driver;
	DriverListEnd	=	the_driver;

	// Update list.
	if(!DriverList)
		DriverList			=	the_driver;

	DriverCount++;

	return	DD_OK;
}

//---------------------------------------------------------------

DDDriverInfo	*DDDriverManager::FindDriver(GUID *the_guid, DDDriverInfo **next_best, DDDriverInfo *start_driver)
{
	DDDriverInfo	*current_driver;

/*
#ifdef DEBUG
    if(!IsInitialized())
    {
        // Error, not initialized
        return NULL;
    }
#endif
*/
    // Get Start node
	if(!start_driver)
		current_driver	=	the_manager.DriverList;
	else
		current_driver	=	start_driver;

	if(next_best)
		*next_best	=	current_driver;


	//
	// Find all the drivers.
	//

	SLONG i;

	DDDriverInfo *driver[10];
	SLONG         driver_upto = 0;

	while(current_driver)
	{
		ASSERT(driver_upto < 10);

		driver[driver_upto++] = current_driver;

		current_driver = current_driver->Next;
	}

	DDDriverInfo *choice1 = NULL;
	DDDriverInfo *choice2 = NULL;

extern int Video3DMode;

	if (Video3DMode == 1)	// select voodoo first
	{
		for (i = 0; i < driver_upto; i++)
		{
			if (the_guid)
			{
				if (driver[i]->Match(the_guid))
				{
					choice1 = driver[i];
				}
				else
				{
					choice2 = driver[i];
				}
			}
			else
			{
				if (driver[i]->IsPrimary())
				{
					choice2 = driver[i];	// Primary device 2nd choice for debug mode
				}
				else
				{
					choice1 = driver[i];
				}
			}
		}

		if (!choice1) {choice1 = choice2;}
		if (!choice2) {choice2 = choice1;}

		current_driver = choice1;

		if (next_best)
		{
			*next_best = choice2;
		}

		return current_driver;
	}
	else
	{
		for (i = 0; i < driver_upto; i++)
		{
			if (the_guid)
			{
				if (driver[i]->Match(the_guid))
				{
					choice1 = driver[i];
				}
				else
				{
					choice2 = driver[i];
				}
			}
			else
			{
				if (driver[i]->IsPrimary())
				{
					choice1 = driver[i];	// Primary device 1st choice for debug mode
				}
				else
				{
					choice2 = driver[i];
				}
			}
		}

		if (!choice1) {choice1 = choice2;}
		if (!choice2) {choice2 = choice1;}

		current_driver = choice1;

		if (next_best)
		{
			*next_best = choice2;
		}

		return current_driver;
	}




	/*

	while(current_driver)
	{
		if(current_driver->Match(the_guid))
		{
			// Success
			return	current_driver;
		}
		else if(current_driver->IsPrimary())
		{
			if(next_best)
				*next_best	=	current_driver;
		}
		current_driver	=	current_driver->Next;
	}

	*/

    // Failure, user could use next best instead
    return NULL;
}

//---------------------------------------------------------------
/*
DDDriverInfo	*DDDriverManager::FindDriver(DDCAPS *hal,DDCAPS *hel,DDDriverInfo **next_best,DDDriverInfo *start_driver)
{
    DDDriverInfo	*current_driver, lpNextDrv;


    // Get Start node
	if(!start_driver)
		current_driver	=	the_manager.DriverList;
	else
		current_driver	=	start_driver;

	if(next_best)
		*next_best	=	current_driver;

	//
	// Find all the drivers.
	//

	SLONG i;

	DDDriverInfo *driver[10];
	SLONG         driver_upto = 0;

	while(current_driver)
	{
		ASSERT(driver_upto < 10);

		driver[driver_upto++] = current_driver;

		current_driver = current_driver->Next;
	}

	DDDriverInfo *choice1 = NULL;
	DDDriverInfo *choice2 = NULL;

	#ifdef NDEBUG

	for (i = 0; i < driver_upto; i++)
	{
		if (driver[i]->IsPrimary())
		{
			choice2 = driver[i];	// Primary device 2nd choice for debug mode
		}
		else
		{
			choice1 = driver[i];
		}
	}

	if (!choice1) {choice1 = choice2;}
	if (!choice2) {choice2 = choice1;}

	current_driver = choice1;

	if (next_best)
	{
		*next_best = choice2;
	}

	return current_driver;

	#else

	for (i = 0; i < driver_upto; i++)
	{
		if (driver[i]->IsPrimary())
		{
			choice1 = driver[i];	// Primary device 1st choice for debug mode
		}
		else
		{
			choice2 = driver[i];
		}
	}

	if (!choice1) {choice1 = choice2;}
	if (!choice2) {choice2 = choice1;}

	current_driver = choice1;

	if (next_best)
	{
		*next_best = choice2;
	}

	return current_driver;

	#endif

//	while(current_driver)
//	{
//		if(current_driver->Match(hal,hel))
//		{
//			// Success
//			return	current_driver;
//		}
//		else if(current_driver->IsPrimary())
//		{
//			if(next_best)
//				*next_best	=	current_driver;
//		}
//
//		current_driver	=	current_driver->Next;
//	}

    // Failure, user could use next best instead
    return NULL;
}

*/

//---------------------------------------------------------------
