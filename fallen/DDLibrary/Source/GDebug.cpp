// Debug.cpp
// Guy Simmons, 15th November 1997.

#include	"DDLib.h"

#ifndef NDEBUG

HANDLE		LogFile;

//---------------------------------------------------------------

HANDLE	InitDebugLog(void)
{
	LogFile	=	CreateFile	(
								"DebugLog.txt",
								(GENERIC_READ|GENERIC_WRITE),
								(FILE_SHARE_READ|FILE_SHARE_WRITE),
								NULL,
								CREATE_ALWAYS,
								0,
								NULL
	                   		);
	if(LogFile==INVALID_HANDLE_VALUE)
	{
		LogFile	=	NULL;
	}
	return	LogFile;
}

//---------------------------------------------------------------

void	FiniDebugLog(void)
{
	if(LogFile)
	CloseHandle(LogFile);
}

//---------------------------------------------------------------

void	DebugText(CBYTE *error, ...)
{
	CBYTE 			buf[512];
	SLONG			bytes_written;
	va_list 		argptr;

	if(LogFile)
	{
		va_start(argptr,error); 
		vsprintf(buf, error,argptr); 
		va_end(argptr);

		WriteFile(LogFile,buf,strlen(buf),(LPDWORD)&bytes_written,NULL);
	}
}

//---------------------------------------------------------------

#define OutputDXError TRACE

void	dd_error(HRESULT dd_err)
{
	if(dd_err)
	{
		OutputDXError("DirectDraw Error: ");
		switch(dd_err)
		{
			case	DDERR_ALREADYINITIALIZED:			OutputDXError("DDERR_ALREADYINITIALIZED");break;
			case	DDERR_CANNOTATTACHSURFACE:			OutputDXError("DDERR_CANNOTATTACHSURFACE");break;
			case	DDERR_CANNOTDETACHSURFACE:			OutputDXError("DDERR_CANNOTDETACHSURFACE");break;
			case	DDERR_CURRENTLYNOTAVAIL:			OutputDXError("DDERR_CURRENTLYNOTAVAIL");break;
			case	DDERR_EXCEPTION:					OutputDXError("DDERR_EXCEPTION");break;
			case	DDERR_GENERIC:						OutputDXError("DDERR_GENERIC");break;
			case	DDERR_HEIGHTALIGN:					OutputDXError("DDERR_HEIGHTALIGN");break;
			case	DDERR_INCOMPATIBLEPRIMARY:			OutputDXError("DDERR_INCOMPATIBLEPRIMARY");break;
			case	DDERR_INVALIDCAPS:					OutputDXError("DDERR_INVALIDCAPS");break;
			case	DDERR_INVALIDCLIPLIST:				OutputDXError("DDERR_INVALIDCLIPLIST");break;
			case	DDERR_INVALIDMODE:					OutputDXError("DDERR_INVALIDMODE");break;
			case	DDERR_INVALIDOBJECT:				OutputDXError("DDERR_INVALIDOBJECT");break;
			case	DDERR_INVALIDPARAMS:				OutputDXError("DDERR_INVALIDPARAMS");break;
			case	DDERR_INVALIDPIXELFORMAT:			OutputDXError("DDERR_INVALIDPIXELFORMAT");break;
			case	DDERR_INVALIDRECT:					OutputDXError("DDERR_INVALIDRECT");break;
			case	DDERR_LOCKEDSURFACES:				OutputDXError("DDERR_LOCKEDSURFACES");break;
			case	DDERR_NO3D:							OutputDXError("DDERR_NO3D");break;
			case	DDERR_NOALPHAHW:					OutputDXError("DDERR_NOALPHAHW");break;
			case	DDERR_NOCLIPLIST:					OutputDXError("DDERR_NOCLIPLIST");break;
			case	DDERR_NOCOLORCONVHW:				OutputDXError("DDERR_NOCOLORCONVHW");break;
			case	DDERR_NOCOOPERATIVELEVELSET:		OutputDXError("DDERR_NOCOOPERATIVELEVELSET");break;
			case	DDERR_NOCOLORKEY:					OutputDXError("DDERR_NOCOLORKEY");break;
			case	DDERR_NOCOLORKEYHW:					OutputDXError("DDERR_NOCOLORKEYHW");break;
			case	DDERR_NODIRECTDRAWSUPPORT:			OutputDXError("DDERR_NODIRECTDRAWSUPPORT");break;
			case	DDERR_NOEXCLUSIVEMODE:				OutputDXError("DDERR_NOEXCLUSIVEMODE");break;
			case	DDERR_NOFLIPHW:						OutputDXError("DDERR_NOFLIPHW");break;
			case	DDERR_NOGDI:						OutputDXError("DDERR_NOGDI");break;
			case	DDERR_NOMIRRORHW:					OutputDXError("DDERR_NOMIRRORHW");break;
			case	DDERR_NOTFOUND:						OutputDXError("DDERR_NOTFOUND");break;
			case	DDERR_NOOVERLAYHW:					OutputDXError("DDERR_NOOVERLAYHW");break;
			case	DDERR_NORASTEROPHW:					OutputDXError("DDERR_NORASTEROPHW");break;
			case	DDERR_NOROTATIONHW:					OutputDXError("DDERR_NOROTATIONHW");break;
			case	DDERR_NOSTRETCHHW:					OutputDXError("DDERR_NOSTRETCHHW");break;
			case	DDERR_NOT4BITCOLOR:					OutputDXError("DDERR_NOT4BITCOLOR");break;
			case	DDERR_NOT4BITCOLORINDEX:			OutputDXError("DDERR_NOT4BITCOLORINDEX");break;
			case	DDERR_NOT8BITCOLOR:					OutputDXError("DDERR_NOT8BITCOLOR");break;
			case	DDERR_NOTEXTUREHW:					OutputDXError("DDERR_NOTEXTUREHW");break;
			case	DDERR_NOVSYNCHW:					OutputDXError("DDERR_NOVSYNCHW");break;
			case	DDERR_NOZBUFFERHW:					OutputDXError("DDERR_NOZBUFFERHW");break;
			case	DDERR_NOZOVERLAYHW:					OutputDXError("DDERR_NOZOVERLAYHW");break;
			case	DDERR_OUTOFCAPS:					OutputDXError("DDERR_OUTOFCAPS");break;
			case	DDERR_OUTOFMEMORY:					OutputDXError("DDERR_OUTOFMEMORY");break;
			case	DDERR_OUTOFVIDEOMEMORY:				OutputDXError("DDERR_OUTOFVIDEOMEMORY");break;
			case	DDERR_OVERLAYCANTCLIP:				OutputDXError("DDERR_OVERLAYCANTCLIP");break;
			case	DDERR_OVERLAYCOLORKEYONLYONEACTIVE:	OutputDXError("DDERR_OVERLAYCOLORKEYONLYONEACTIVE");break;
			case	DDERR_PALETTEBUSY:					OutputDXError("DDERR_PALETTEBUSY");break;
			case	DDERR_COLORKEYNOTSET:				OutputDXError("DDERR_COLORKEYNOTSET");break;
			case	DDERR_SURFACEALREADYATTACHED:		OutputDXError("DDERR_SURFACEALREADYATTACHED");break;
			case	DDERR_SURFACEALREADYDEPENDENT:		OutputDXError("DDERR_SURFACEALREADYDEPENDENT");break;
			case	DDERR_SURFACEBUSY:					OutputDXError("DDERR_SURFACEBUSY");break;
			case	DDERR_CANTLOCKSURFACE:				OutputDXError("DDERR_CANTLOCKSURFACE");break;
			case	DDERR_SURFACEISOBSCURED:			OutputDXError("DDERR_SURFACEISOBSCURED");break;
			case	DDERR_SURFACELOST:					OutputDXError("DDERR_SURFACELOST");break;
			case	DDERR_SURFACENOTATTACHED:			OutputDXError("DDERR_SURFACENOTATTACHED");break;
			case	DDERR_TOOBIGHEIGHT:					OutputDXError("DDERR_TOOBIGHEIGHT");break;
			case	DDERR_TOOBIGSIZE:					OutputDXError("DDERR_TOOBIGSIZE");break;
			case	DDERR_TOOBIGWIDTH:					OutputDXError("DDERR_TOOBIGWIDTH");break;
			case	DDERR_UNSUPPORTED:					OutputDXError("DDERR_UNSUPPORTED");break;
			case	DDERR_UNSUPPORTEDFORMAT:			OutputDXError("DDERR_UNSUPPORTEDFORMAT");break;
			case	DDERR_UNSUPPORTEDMASK:				OutputDXError("DDERR_UNSUPPORTEDMASK");break;
			case	DDERR_VERTICALBLANKINPROGRESS:		OutputDXError("DDERR_VERTICALBLANKINPROGRESS");break;
			case	DDERR_WASSTILLDRAWING:				OutputDXError("DDERR_WASSTILLDRAWING");break;
			case	DDERR_XALIGN:						OutputDXError("DDERR_XALIGN");break;
			case	DDERR_INVALIDDIRECTDRAWGUID:		OutputDXError("DDERR_INVALIDDIRECTDRAWGUID");break;
			case	DDERR_DIRECTDRAWALREADYCREATED:		OutputDXError("DDERR_DIRECTDRAWALREADYCREATED");break;
			case	DDERR_NODIRECTDRAWHW:				OutputDXError("DDERR_NODIRECTDRAWHW");break;
			case	DDERR_PRIMARYSURFACEALREADYEXISTS:	OutputDXError("DDERR_PRIMARYSURFACEALREADYEXISTS");break;
			case	DDERR_NOEMULATION:					OutputDXError("DDERR_NOEMULATION");break;
			case	DDERR_REGIONTOOSMALL:				OutputDXError("DDERR_REGIONTOOSMALL");break;
			case	DDERR_CLIPPERISUSINGHWND:			OutputDXError("DDERR_CLIPPERISUSINGHWND");break;
			case	DDERR_NOCLIPPERATTACHED:			OutputDXError("DDERR_NOCLIPPERATTACHED");break;
			case	DDERR_NOHWND:						OutputDXError("DDERR_NOHWND");break;
			case	DDERR_HWNDSUBCLASSED:				OutputDXError("DDERR_HWNDSUBCLASSED");break;
			case	DDERR_HWNDALREADYSET:				OutputDXError("DDERR_HWNDALREADYSET");break;
			case	DDERR_NOPALETTEATTACHED:			OutputDXError("DDERR_NOPALETTEATTACHED");break;
			case	DDERR_NOPALETTEHW:					OutputDXError("DDERR_NOPALETTEHW");break;
			case	DDERR_BLTFASTCANTCLIP:				OutputDXError("DDERR_BLTFASTCANTCLIP");break;
			case	DDERR_NOBLTHW:						OutputDXError("DDERR_NOBLTHW");break;
			case	DDERR_NODDROPSHW:					OutputDXError("DDERR_NODDROPSHW");break;
			case	DDERR_OVERLAYNOTVISIBLE:			OutputDXError("DDERR_OVERLAYNOTVISIBLE");break;
			case	DDERR_NOOVERLAYDEST:				OutputDXError("DDERR_NOOVERLAYDEST");break;
			case	DDERR_INVALIDPOSITION:				OutputDXError("DDERR_INVALIDPOSITION");break;
			case	DDERR_NOTAOVERLAYSURFACE:			OutputDXError("DDERR_NOTAOVERLAYSURFACE");break;
			case	DDERR_EXCLUSIVEMODEALREADYSET:		OutputDXError("DDERR_EXCLUSIVEMODEALREADYSET");break;
			case	DDERR_NOTFLIPPABLE:					OutputDXError("DDERR_NOTFLIPPABLE");break;
			case	DDERR_CANTDUPLICATE:				OutputDXError("DDERR_CANTDUPLICATE");break;
			case	DDERR_NOTLOCKED:					OutputDXError("DDERR_NOTLOCKED");break;
			case	DDERR_CANTCREATEDC:					OutputDXError("DDERR_CANTCREATEDC");break;
			case	DDERR_NODC:							OutputDXError("DDERR_NODC");break;
			case	DDERR_WRONGMODE:					OutputDXError("DDERR_WRONGMODE");break;
			case	DDERR_IMPLICITLYCREATED:			OutputDXError("DDERR_IMPLICITLYCREATED");break;
			case	DDERR_NOTPALETTIZED:				OutputDXError("DDERR_NOTPALETTIZED");break;
			case	DDERR_UNSUPPORTEDMODE:				OutputDXError("DDERR_UNSUPPORTEDMODE");break;
			case	DDERR_NOMIPMAPHW:					OutputDXError("DDERR_NOMIPMAPHW");break;
			case	DDERR_INVALIDSURFACETYPE:			OutputDXError("DDERR_INVALIDSURFACETYPE");break;
			case	DDERR_NOOPTIMIZEHW:					OutputDXError("DDERR_NOOPTIMIZEHW");break;
			case	DDERR_NOTLOADED:					OutputDXError("DDERR_NOTLOADED");break;
			case	DDERR_DCALREADYCREATED:				OutputDXError("DDERR_DCALREADYCREATED");break;
			case	DDERR_NONONLOCALVIDMEM:				OutputDXError("DDERR_NONONLOCALVIDMEM");break;
			case	DDERR_CANTPAGELOCK:					OutputDXError("DDERR_CANTPAGELOCK");break;
			case	DDERR_CANTPAGEUNLOCK:				OutputDXError("DDERR_CANTPAGEUNLOCK");break;
			case	DDERR_NOTPAGELOCKED:				OutputDXError("DDERR_NOTPAGELOCKED");break;
			case	DDERR_MOREDATA:						OutputDXError("DDERR_MOREDATA");break;
			case	DDERR_VIDEONOTACTIVE:				OutputDXError("DDERR_VIDEONOTACTIVE");break;
			case	DDERR_DEVICEDOESNTOWNSURFACE:		OutputDXError("DDERR_DEVICEDOESNTOWNSURFACE");break;
			case	DDERR_NOTINITIALIZED:				OutputDXError("DDERR_NOTINITIALIZED");break;
			default:									OutputDXError("Unknown - %ld",dd_err&0xffff);
		}
		OutputDXError("\n");
	}
}

//---------------------------------------------------------------

void	d3d_error(HRESULT dd_err)
{
	if(dd_err)
	{
		OutputDXError("Direct3D Error: ");
		switch(dd_err)
		{
			case	D3DERR_BADMAJORVERSION:			OutputDXError("D3DERR_BADMAJORVERSION");break;
			case	D3DERR_BADMINORVERSION:			OutputDXError("D3DERR_BADMINORVERSION");break;
			case	D3DERR_INVALID_DEVICE:			OutputDXError("D3DERR_INVALID_DEVICE");break;
			case	D3DERR_EXECUTE_CREATE_FAILED:	OutputDXError("D3DERR_EXECUTE_CREATE_FAILED");break;
			case	D3DERR_EXECUTE_DESTROY_FAILED:	OutputDXError("D3DERR_EXECUTE_DESTROY_FAILED");break;
			case	D3DERR_EXECUTE_LOCK_FAILED:		OutputDXError("D3DERR_EXECUTE_LOCK_FAILED");break;
			case	D3DERR_EXECUTE_UNLOCK_FAILED:	OutputDXError("D3DERR_EXECUTE_UNLOCK_FAILED");break;
			case	D3DERR_EXECUTE_LOCKED:			OutputDXError("D3DERR_EXECUTE_LOCKED");break;
			case	D3DERR_EXECUTE_NOT_LOCKED:		OutputDXError("D3DERR_EXECUTE_NOT_LOCKED");break;
			case	D3DERR_EXECUTE_FAILED:			OutputDXError("D3DERR_EXECUTE_FAILED");break;
			case	D3DERR_EXECUTE_CLIPPED_FAILED:	OutputDXError("D3DERR_EXECUTE_CLIPPED_FAILED");break;
			case	D3DERR_TEXTURE_NO_SUPPORT:		OutputDXError("D3DERR_TEXTURE_NO_SUPPORT");break;
			case	D3DERR_TEXTURE_CREATE_FAILED:	OutputDXError("D3DERR_TEXTURE_CREATE_FAILED");break;
			case	D3DERR_TEXTURE_DESTROY_FAILED:	OutputDXError("D3DERR_TEXTURE_DESTROY_FAILED");break;
			case	D3DERR_TEXTURE_LOCK_FAILED:		OutputDXError("D3DERR_TEXTURE_LOCK_FAILED");break;
			case	D3DERR_TEXTURE_UNLOCK_FAILED:	OutputDXError("D3DERR_TEXTURE_UNLOCK_FAILED");break;
			case	D3DERR_TEXTURE_LOAD_FAILED:		OutputDXError("D3DERR_TEXTURE_LOAD_FAILED");break;
			case	D3DERR_TEXTURE_SWAP_FAILED:		OutputDXError("D3DERR_TEXTURE_SWAP_FAILED");break;
			case	D3DERR_TEXTURE_LOCKED:			OutputDXError("D3DERR_TEXTURE_LOCKED");break;
			case	D3DERR_TEXTURE_NOT_LOCKED:		OutputDXError("D3DERR_TEXTURE_NOT_LOCKED");break;
			case	D3DERR_TEXTURE_GETSURF_FAILED:	OutputDXError("D3DERR_TEXTURE_GETSURF_FAILED");break;
			case	D3DERR_MATRIX_CREATE_FAILED:	OutputDXError("D3DERR_MATRIX_CREATE_FAILED");break;
			case	D3DERR_MATRIX_DESTROY_FAILED:	OutputDXError("D3DERR_MATRIX_DESTROY_FAILED");break;
			case	D3DERR_MATRIX_SETDATA_FAILED:	OutputDXError("D3DERR_MATRIX_SETDATA_FAILED");break;
			case	D3DERR_MATRIX_GETDATA_FAILED:	OutputDXError("D3DERR_MATRIX_GETDATA_FAILED");break;
			case	D3DERR_SETVIEWPORTDATA_FAILED:	OutputDXError("D3DERR_SETVIEWPORTDATA_FAILED");break;
			case	D3DERR_MATERIAL_CREATE_FAILED:	OutputDXError("D3DERR_MATERIAL_CREATE_FAILED");break;
			case	D3DERR_MATERIAL_DESTROY_FAILED:	OutputDXError("D3DERR_MATERIAL_DESTROY_FAILED");break;
			case	D3DERR_MATERIAL_SETDATA_FAILED:	OutputDXError("D3DERR_MATERIAL_SETDATA_FAILED");break;
			case	D3DERR_MATERIAL_GETDATA_FAILED:	OutputDXError("D3DERR_MATERIAL_GETDATA_FAILED");break;
			case	D3DERR_LIGHT_SET_FAILED:		OutputDXError("D3DERR_LIGHT_SET_FAILED");break;
			case	D3DERR_SCENE_IN_SCENE:			OutputDXError("D3DERR_SCENE_IN_SCENE");break;
			case	D3DERR_SCENE_NOT_IN_SCENE:		OutputDXError("D3DERR_SCENE_NOT_IN_SCENE");break;
			case	D3DERR_SCENE_BEGIN_FAILED:		OutputDXError("D3DERR_SCENE_BEGIN_FAILED");break;
			case	D3DERR_SCENE_END_FAILED:		OutputDXError("D3DERR_SCENE_END_FAILED");break;
			case	D3DERR_INBEGIN:					OutputDXError("D3DERR_INBEGIN");break;
			case	D3DERR_NOTINBEGIN:				OutputDXError("D3DERR_NOTINBEGIN");break;
			case	D3DERR_NOVIEWPORTS:				OutputDXError("D3DERR_NOVIEWPORTS");break;
			case	D3DERR_VIEWPORTDATANOTSET:		OutputDXError("D3DERR_VIEWPORTDATANOTSET");break;
			case	D3DERR_INVALIDCURRENTVIEWPORT:	OutputDXError("D3DERR_INVALIDCURRENTVIEWPORT");break;
			case	D3DERR_INVALIDPRIMITIVETYPE:	OutputDXError("D3DERR_INVALIDPRIMITIVETYPE");break;
			case	D3DERR_INVALIDVERTEXTYPE:		OutputDXError("D3DERR_INVALIDVERTEXTYPE");break;
			case	D3DERR_TEXTURE_BADSIZE:			OutputDXError("D3DERR_TEXTURE_BADSIZE");break;
			default:								dd_error(dd_err);
		}
		OutputDXError("\n");
	}
}

//---------------------------------------------------------------

void	di_error(HRESULT di_err)
{
	if(di_err)
	{
		OutputDXError("DirectInput Error: ");
		switch(di_err)
		{
			case	DIERR_OLDDIRECTINPUTVERSION:	OutputDXError("DIERR_OLDDIRECTINPUTVERSION");break;
			case	DIERR_BETADIRECTINPUTVERSION:	OutputDXError("DIERR_BETADIRECTINPUTVERSION");break;
			case	DIERR_BADDRIVERVER:				OutputDXError("DIERR_BADDRIVERVER");break;
			case	DIERR_DEVICENOTREG:				OutputDXError("DIERR_DEVICENOTREG");break;	
			case	DIERR_NOTFOUND:					OutputDXError("DIERR_NOTFOUND\nDIERR_OBJECTNOTFOUND\nDIERR_READONLY\nDIERR_HANDLEEXISTS");break;	
			case	DIERR_INVALIDPARAM:				OutputDXError("DIERR_INVALIDPARAM");break;
			case	DIERR_NOINTERFACE:				OutputDXError("DIERR_NOINTERFACE");break;	
			case	DIERR_GENERIC:					OutputDXError("DIERR_GENERIC");break;	
			case	DIERR_OUTOFMEMORY:				OutputDXError("DIERR_OUTOFMEMORY");break;		
			case	DIERR_UNSUPPORTED:				OutputDXError("DIERR_UNSUPPORTED");break;	
			case	DIERR_NOTINITIALIZED:			OutputDXError("DIERR_NOTINITIALIZED");break;	
			case	DIERR_ALREADYINITIALIZED:		OutputDXError("DIERR_ALREADYINITIALIZED");break;
			case	DIERR_NOAGGREGATION:			OutputDXError("DIERR_NOAGGREGATION");break;
			case	DIERR_OTHERAPPHASPRIO:			OutputDXError("DIERR_OTHERAPPHASPRIO");break;	
			case	DIERR_INPUTLOST:				OutputDXError("DIERR_INPUTLOST");break;
			case	DIERR_ACQUIRED:					OutputDXError("DIERR_ACQUIRED");break;		
			case	DIERR_NOTACQUIRED:				OutputDXError("DIERR_NOTACQUIRED");break;		
			case	E_PENDING:						OutputDXError("E_PENDING");break;	
			case	DIERR_INSUFFICIENTPRIVS:		OutputDXError("DIERR_INSUFFICIENTPRIVS");break;
			case	DIERR_DEVICEFULL:				OutputDXError("DIERR_DEVICEFULL");break;
			case	DIERR_MOREDATA:					OutputDXError("DIERR_MOREDATA");break;	
			case	DIERR_NOTDOWNLOADED:			OutputDXError("DIERR_NOTDOWNLOADED");break;	
			case	DIERR_HASEFFECTS:				OutputDXError("DIERR_HASEFFECTS");break;	
			case	DIERR_NOTEXCLUSIVEACQUIRED:		OutputDXError("DIERR_NOTEXCLUSIVEACQUIRED");break;
			case	DIERR_INCOMPLETEEFFECT:			OutputDXError("DIERR_INCOMPLETEEFFECT");break;
			case	DIERR_NOTBUFFERED:				OutputDXError("DIERR_NOTBUFFERED");break;
			case	DIERR_EFFECTPLAYING:			OutputDXError("DIERR_EFFECTPLAYING");break;
			default:								OutputDXError("Unknown - %ld",di_err&0xffff);
		}
		OutputDXError("\n");
	}
}

//---------------------------------------------------------------

#endif
