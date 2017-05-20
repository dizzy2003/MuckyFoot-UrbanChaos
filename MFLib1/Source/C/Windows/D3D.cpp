// Draw3D.cpp	-	Windows.
// Guy Simmons, 8th May 1997.


#define	INITGUID

#include	<MFHeader.h>


BOOL					DeviceFound		=	FALSE,
						GotD3D2			=	FALSE,
						HasHardware		=	FALSE;
ULONG					RequestFlags	=	0;
SLONG					DeviceBitDepth;
D3DDEVICEDESC			d3d_DeviceDesc;
GUID					GUIDDevice;
IDirect3DDevice2		*lp_D3D_Device;
LPDIRECT3D				lp_D3D			=	NULL;
LPDIRECT3D2				lp_D3D_2		=	NULL;
LPDIRECTDRAWSURFACE		lp_DD_ZBuffer;

HRESULT WINAPI	EnumDeviceCallback(LPGUID,LPTSTR,LPTSTR,LPD3DDEVICEDESC,LPD3DDEVICEDESC,LPVOID);

//---------------------------------------------------------------

void	SetupD3D2(void)
{
    HRESULT			dd_result; 


	if(!GotD3D2)
	{
		dd_result	=	lp_DD->QueryInterface(IID_IDirect3D,(LPVOID*)&lp_D3D);
		if(dd_result==DD_OK)
		{
			dd_result	=	lp_D3D->QueryInterface(IID_IDirect3D2,(LPVOID*)&lp_D3D_2);
			if(dd_result==DD_OK)
			{
				LogText("D3D2 installed.\n");
				GotD3D2	=	TRUE;
			}
		}
	}
	return;
}

//---------------------------------------------------------------

void	ResetD3D2(void)
{
    HRESULT			dd_result; 


	if(lp_DD_ZBuffer)
	{
		dd_result	=	lp_DD_ZBuffer->Release();
		if(dd_result!=DD_OK)
		{
			LogText("error: %ld - Unable to release ZBuffer\n",dd_result&0xff);
		}
	}

	if(lp_D3D_2)
	{
		dd_result	=	lp_D3D_2->Release();
		if(dd_result!=DD_OK)
		{
			LogText("error: %ld - Unable to release Direct3D2\n",dd_result&0xff);
		}
	}
	if(lp_D3D)
	{
		dd_result	=	lp_D3D->Release();
		if(dd_result!=DD_OK)
		{
			LogText("error: %ld - Unable to release Direct3D\n",dd_result&0xff);
		}
	}
}

//---------------------------------------------------------------

BOOL	ChooseD3DDevice(ULONG flags)
{
    DDSURFACEDESC	dd_sd; 
    HRESULT			dd_result; 


	if(GotD3D2)
	{
	    ZeroMemory(&dd_sd,sizeof(dd_sd));
	    dd_sd.dwSize		=	sizeof(dd_sd);
	    dd_result			=	lp_DD_FrontSurface->GetSurfaceDesc(&dd_sd);
		if(dd_result==DD_OK)
		{	 
		    switch(dd_sd.ddpfPixelFormat.dwRGBBitCount)
		    { 
		        case	1:	DeviceBitDepth	=	DDBD_1;break;
		        case	2:	DeviceBitDepth	=	DDBD_2;break;
		        case	4:	DeviceBitDepth	=	DDBD_4;break;
		        case	8:	DeviceBitDepth	=	DDBD_8;break;
		        case	16:	DeviceBitDepth	=	DDBD_16;break;
		        case	24:	DeviceBitDepth	=	DDBD_24;break;
		        case	32:	DeviceBitDepth	=	DDBD_32;break;
		        default:	DeviceBitDepth	=	0;
		    } 		 
		    DeviceFound			=	FALSE; 
			RequestFlags		=	flags;
		    dd_result			=	lp_D3D_2->EnumDevices(EnumDeviceCallback,NULL);

			if(dd_result==D3D_OK && DeviceFound)
			{
				LogText("Got required device.\n");

				memset(&dd_sd, 0, sizeof(DDSURFACEDESC));

				dd_sd.dwSize	=	sizeof(DDSURFACEDESC);
				dd_sd.dwFlags	=	DDSD_WIDTH  |
									DDSD_HEIGHT |
									DDSD_CAPS   |
									DDSD_ZBUFFERBITDEPTH;
				dd_sd.dwWidth	=	WorkScreenPixelWidth;
				dd_sd.dwHeight	=	WorkScreenHeight;

				if(HasHardware)
				{
					LogText("ZBuffer in Video memory\n");
					dd_sd.ddsCaps.dwCaps	=	 DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
				}
				else
				{
					LogText("ZBuffer in System memory\n");
					dd_sd.ddsCaps.dwCaps	=	 DDSCAPS_ZBUFFER | DDSCAPS_SYSTEMMEMORY;
				}

				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth & DDBD_32)	{	dd_sd.dwZBufferBitDepth=32;LogText("32 bit zbuffer\n");	}
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth & DDBD_24)	{	dd_sd.dwZBufferBitDepth=24;LogText("24 bit zbuffer\n");	}
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth & DDBD_16)	{	dd_sd.dwZBufferBitDepth=16;LogText("16 bit zbuffer\n");	}
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth & DDBD_8 )	{	dd_sd.dwZBufferBitDepth=8;LogText("8 bit zbuffer\n");	}
/*
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth&DDBD_32)	{	dd_sd.dwZBufferBitDepth=32;	}
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth&DDBD_24)	{	dd_sd.dwZBufferBitDepth=24;	}
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth&DDBD_16)	{	dd_sd.dwZBufferBitDepth=16;	}
				if(d3d_DeviceDesc.dwDeviceZBufferBitDepth&DDBD_8 )	{	dd_sd.dwZBufferBitDepth=8;	}
*/
				dd_result	=	lp_DD->CreateSurface(&dd_sd,&lp_DD_ZBuffer,NULL);
				if(dd_result!=DD_OK)
				{
					LogText("error: %ld - Can't create ZBuffer\n",dd_result&0xffff);
					goto	exit_false;
				}

				dd_result	=	lp_DD_BackSurface->AddAttachedSurface(lp_DD_ZBuffer);
				if(dd_result!=DD_OK)
				{
					LogText("Can't attach ZBuffer\n");
					goto	exit_false;
				}

				dd_result	=	lp_D3D_2->CreateDevice(GUIDDevice,lp_DD_BackSurface,&lp_D3D_Device);
				if(dd_result!=DD_OK)
				{
					LogText("Can't create direct draw device\n");
					goto	exit_false;
				}
				return	TRUE;
			}
		}
	}
exit_false:
	return	FALSE;
} 

//---------------------------------------------------------------

HRESULT WINAPI	EnumDeviceCallback(
									LPGUID			lpGUID,
									LPTSTR			lpszDeviceDesc,
									LPTSTR           lpszDeviceName, 
									LPD3DDEVICEDESC lpd3dHWDeviceDesc, 
									LPD3DDEVICEDESC lpd3dSWDeviceDesc, 
									LPVOID          lpUserArg
									) 
{
    LPD3DDEVICEDESC		lp_d3d_device_desc;


    lpUserArg	=	lpUserArg;

	HasHardware	=	(lpd3dHWDeviceDesc->dcmColorModel!=0);

	// Do we want hardware & do we have hardware?
	if(RequestFlags&D3D_HARDWARE && !HasHardware)
		return	D3DENUMRET_OK;

	lp_d3d_device_desc	=	(HasHardware	?	lpd3dHWDeviceDesc : lpd3dSWDeviceDesc);

	// Do we want a ramp device & do we have a ramp device?
	if(!(RequestFlags&D3D_RAMP) && lp_d3d_device_desc->dcmColorModel==D3DCOLOR_MONO)
		return	D3DENUMRET_OK;

	// Check the device supports the correct rendering depth.
    if(!(lp_d3d_device_desc->dwDeviceRenderBitDepth&DeviceBitDepth))
        return D3DENUMRET_OK; 

	// Check the device supports everything we want it to.
    if(lp_d3d_device_desc->dcmColorModel==D3DCOLOR_MONO)	// Ramp device.
	{
		if	(
				(RequestFlags&D3D_GOURAUD) &&
				!(lp_d3d_device_desc->dpcTriCaps.dwShadeCaps&D3DPSHADECAPS_COLORGOURAUDMONO)
			)
			return	D3DENUMRET_OK;		// No Ramp gouraud shading.
	}
	else												// RGB device.
	{
		if	(
				(RequestFlags&D3D_GOURAUD) &&
				!(lp_d3d_device_desc->dpcTriCaps.dwShadeCaps&D3DPSHADECAPS_COLORGOURAUDRGB)
			)
			return	D3DENUMRET_OK;		// No RGB gouraud shading.
	}

    DeviceFound	=	TRUE;
    CopyMemory(&GUIDDevice,lpGUID,sizeof(GUID));
//    strcpy(DeviceDesc, lpszDeviceDesc);
//    strcpy(DeviceName, lpszDeviceName);
	CopyMemory(&d3d_DeviceDesc,lp_d3d_device_desc,sizeof(D3DDEVICEDESC));

	LogText("DeviceDesc - %s\nDeviceName - %s\n\n",lpszDeviceDesc,lpszDeviceName);

	return D3DENUMRET_CANCEL; 
}

//---------------------------------------------------------------
/*
HRESULT WINAPI	EnumDeviceCallback(
									LPGUID			lpGUID,
									LPTSTR			lpszDeviceDesc,
									LPTSTR           lpszDeviceName, 
									LPD3DDEVICEDESC lpd3dHWDeviceDesc, 
									LPD3DDEVICEDESC lpd3dSWDeviceDesc, 
									LPVOID          lpUserArg
									) 
{
    LPD3DDEVICEDESC		lpd3dDeviceDesc; 
 

    lpUserArg	=	lpUserArg;

#ifdef	_DEBUG
	HardwareDevice     =	FALSE;
#else
	// If there is no hardware support the color model is zero. 
    HardwareDevice	=	(0 != lpd3dHWDeviceDesc->dcmColorModel); 
#endif

    lpd3dDeviceDesc	=	(HardwareDevice	?	lpd3dHWDeviceDesc : 
											lpd3dSWDeviceDesc); 
 
#ifdef	_DEBUG
	if(HardwareDevice)
		return	D3DENUMRET_OK;
#else
	if(!HardwareDevice)
		return	D3DENUMRET_OK;
#endif

	LogText("DeviceDesc - %s\nDeviceName - %s\n\n",lpszDeviceDesc,lpszDeviceName);

	// Does the device render at the depth we want? 
    if(!(lpd3dDeviceDesc->dwDeviceRenderBitDepth&device_bit_depth))
    { 
		// If not, skip this device. 
 
        return D3DENUMRET_OK; 
    } 
 
	// The device must support Gouraud-shaded triangles. 
    if(D3DCOLOR_MONO == lpd3dDeviceDesc->dcmColorModel)
    { 
        if (!(lpd3dDeviceDesc->dpcTriCaps.dwShadeCaps & 
              D3DPSHADECAPS_COLORGOURAUDMONO)) 
        { 
			// No Gouraud shading. Skip this device. 
			LogText("No Gouraud shading\n");

            return D3DENUMRET_OK; 
        } 
    } 
    else 
    { 
        if(!(lpd3dDeviceDesc->dpcTriCaps.dwShadeCaps & 
              D3DPSHADECAPS_COLORGOURAUDRGB)) 
        { 
			// No Gouraud shading. Skip this device. 
			LogText("No Gouraud shading\n");
 
            return D3DENUMRET_OK; 
        } 
    } 
#ifdef	_DEBUG
    if	(
			!HardwareDevice && !DeviceFound && 
			(D3DCOLOR_RGB == lpd3dDeviceDesc->dcmColorModel)
		)
    { 
		// If this is software RGB and we already have found 
		// a software monochromatic renderer, we are not 
		// interested. Skip this device. 
 
//        return D3DENUMRET_OK; 

		// This is a device we are interested in. Save the details. 
		DeviceFound	=	TRUE;
		CopyMemory(&guid_device, lpGUID, sizeof(GUID));
		strcpy(DeviceDesc, lpszDeviceDesc);
		strcpy(DeviceName, lpszDeviceName);

		CopyMemory(&d3d_HWDeviceDesc, lpd3dHWDeviceDesc, 
				   sizeof(D3DDEVICEDESC)); 
		CopyMemory(&d3d_SWDeviceDesc, lpd3dSWDeviceDesc, 
				   sizeof(D3DDEVICEDESC)); 

		CopyMemory(&d3d_DeviceDesc, lpd3dDeviceDesc,
				   sizeof(D3DDEVICEDESC)); 

        return D3DENUMRET_CANCEL; 
    }
#else
	// This is a device we are interested in. Save the details. 
    DeviceFound	=	TRUE;
    CopyMemory(&guid_device, lpGUID, sizeof(GUID));
    strcpy(DeviceDesc, lpszDeviceDesc);
    strcpy(DeviceName, lpszDeviceName);

    CopyMemory(&d3d_HWDeviceDesc, lpd3dHWDeviceDesc, 
               sizeof(D3DDEVICEDESC)); 
    CopyMemory(&d3d_SWDeviceDesc, lpd3dSWDeviceDesc, 
               sizeof(D3DDEVICEDESC)); 

    CopyMemory(&d3d_DeviceDesc, lpd3dDeviceDesc,
               sizeof(D3DDEVICEDESC)); 
#endif

#ifdef	_DEBUG 
//        return D3DENUMRET_CANCEL; 
    return D3DENUMRET_OK; 
#else
	// If this is a hardware device, we have found 
	// what we are looking for. 
    if(HardwareDevice)
	{
		LogText("We have a hardware device\n");
        return D3DENUMRET_CANCEL; 
	}
 
	// Otherwise, keep looking.  
    return D3DENUMRET_OK; 
#endif
}
*/
//---------------------------------------------------------------
