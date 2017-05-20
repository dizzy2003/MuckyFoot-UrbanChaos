// DDManager.h
// Guy Simmons, 12th November 1997.

#ifndef	DDMANAGER_H
#define	DDMANAGER_H

// #include	"Types.h"

//---------------------------------------------------------------

#define	DD_DRIVER_INIT			(1<<0)

#define	DD_DRIVER_VALID				(1<<0)
#define	DD_DRIVER_PRIMARY			(1<<1)
#define	DD_DRIVER_D3D				(1<<2)
#define	DD_DRIVER_M_LOADED			(1<<3)
#define	DD_DRIVER_D_LOADED			(1<<4)
#define DD_DRIVER_SUPPORTS_16BIT	(1<<5)	// Screen modes...
#define DD_DRIVER_SUPPORTS_32BIT	(1<<6)
#define DD_DRIVER_RENDERS_TO_16BIT	(1<<7)	// Harware rendering...
#define DD_DRIVER_RENDERS_TO_32BIT	(1<<8)
#define DD_DRIVER_MODE_320			(1<<9)
#define DD_DRIVER_MODE_512			(1<<10)
#define DD_DRIVER_MODE_640			(1<<11)
#define DD_DRIVER_MODE_800			(1<<12)
#define DD_DRIVER_MODE_1024			(1<<13)
#define DD_DRIVER_LOW_MEMORY		(1<<14)

#define	D3D_DEVICE_VALID		(1<<0)
#define D3D_DEVICE_F_LOADED		(1<<1)

//---------------------------------------------------------------

typedef struct
{
    BOOL		Result;		// Success/Failure
    DWORD		Count;		// Current count
    void		*Extra;		// Current Driver/Device/Etc.
}CallbackInfo;

//---------------------------------------------------------------

class	DDModeInfo;
class	D3DDeviceInfo;
class	DDDriverInfo;
class	DDDriverManager;

//---------------------------------------------------------------

SLONG			FlagsToBitDepth(SLONG flags);
ULONG			FlagsToMask(SLONG flags);
SLONG			BitDepthToFlags(SLONG bpp);
BOOL			IsPalettized(LPDDPIXELFORMAT lp_dd_pf);
BOOL			GetDesktopMode(DDDriverInfo	*the_driver,LPGUID D3D_guid,DDModeInfo **the_mode,D3DDeviceInfo **the_device);
BOOL			GetFullscreenMode(DDDriverInfo *the_driver,GUID *D3D_guid,SLONG w,SLONG h,SLONG bpp,SLONG refresh,DDModeInfo **the_mode,D3DDeviceInfo **the_device);
DDDriverInfo	*ValidateDriver(GUID *DD_guid);
D3DDeviceInfo	*ValidateDevice(DDDriverInfo *the_driver,GUID *D3D_guid,DDModeInfo *the_filter=NULL);
DDModeInfo		*ValidateMode(DDDriverInfo	*the_driver,DWORD w,DWORD h,DWORD bpp,DWORD refresh,D3DDeviceInfo *the_filter=NULL);

//---------------------------------------------------------------

class	DDModeInfo
{
	private:
	protected:
	public:
		DDSURFACEDESC2	ddSurfDesc;						// Complete Surface Description
		DDModeInfo		*Next,							// Next Node.
						*Prev;							// Prev Node.

						DDModeInfo();
						DDModeInfo(const DDSURFACEDESC & ddDesc);
						~DDModeInfo();

		SLONG			GetWidth(void);
		SLONG			GetHeight(void);
		SLONG			GetBPP(void);
		HRESULT			GetMode(SLONG *w,SLONG *h,SLONG *bpp,SLONG *refresh);
		BOOL			ModeSupported(D3DDeviceInfo *the_device);
		BOOL			Match(SLONG w,SLONG h,SLONG bpp);
		BOOL			Match(SLONG bpp);
};

//---------------------------------------------------------------

class	D3DDeviceInfo
{
	private:
	protected:
	public:
		ULONG           D3DFlags;					// Flags

		// D3D Info
		GUID			guid;						// GUID
		LPTSTR			szName;						// Driver Name
		LPTSTR			szDesc;						// Driver Description
		D3DDEVICEDESC	d3dHalDesc;					// HAL info
		D3DDEVICEDESC	d3dHelDesc;					// HEL info

		// Texture Formats
		ULONG			FormatCount;				// Count of Texture Formats
		DDModeInfo		*FormatList,				// List of Texture Formats.
						*FormatListEnd;

		DDModeInfo*		OpaqueTexFmt;				// preferred format for opaque textures
		DDModeInfo*		AlphaTexFmt;				// preferred format for alpha textures

		// Z Format
		DDPIXELFORMAT	ZFormat;
		
		// caps
#ifndef TARGET_DC
		bool			CanDoModulateAlpha;			// can do TBLEND_MODULATEALPHA mode?
		bool			CanDoDestInvSourceColour;	// can do BLEND_SRCCOLOR?
		bool			CanDoAdamiLighting;			// can do Adami lighting?
		bool			CanDoForsythLighting;		// can do Forsyth lighting?
#endif

		// Node Info
		D3DDeviceInfo	*Next,						// Next Node
						*Prev;						// Prev Node


							D3DDeviceInfo();
							~D3DDeviceInfo();
	
		// Methods
		HRESULT				Create(LPGUID lpD3DGuid,LPTSTR lpD3DName,LPTSTR lpD3DDesc,LPD3DDEVICEDESC lpD3DHal,LPD3DDEVICEDESC lpD3DHel);
		void				Destroy(void);

		BOOL				IsHardware(void);
		BOOL				Match(GUID *the_guid);

		inline	BOOL		IsValid(void)				{	return	D3DFlags&D3D_DEVICE_VALID;		}
		inline	void		ValidOn(void)				{	D3DFlags	|=	D3D_DEVICE_VALID;		}
		inline	void		ValidOff(void)				{	D3DFlags	&=	~D3D_DEVICE_VALID;		}

		// Texure Format Methods
		HRESULT				LoadFormats(LPDIRECT3DDEVICE3 the_d3d_device);
		HRESULT				DestroyFormats(void);
		HRESULT				AddFormat(DDModeInfo *the_format);
		HRESULT				DelFormat(DDModeInfo *the_format);

		void				FindOpaqueTexFmt();
		void				FindAlphaTexFmt();

		DDModeInfo			*FindFormat(SLONG bpp,DDModeInfo **next_best_format,DDModeInfo *start=NULL);

		inline	SLONG		CountFormats(void)			{	return	FormatCount;					}
		inline	BOOL		FormatsLoaded(void)			{	return	((D3DFlags&D3D_DEVICE_F_LOADED) ? TRUE : FALSE);	}
		inline	void		TurnFormatsLoadedOn(void)	{	D3DFlags	|=	D3D_DEVICE_F_LOADED;	}
		inline	void		TurnFormatsLoadedOff(void)	{	D3DFlags	&=	~D3D_DEVICE_F_LOADED;	}

		// Z Format methods
		HRESULT				LoadZFormats(LPDIRECT3D3 d3d);
		void				SetZFormat(LPDDPIXELFORMAT lpPF)	{ memcpy(&ZFormat, lpPF, sizeof(ZFormat)); }
		LPDDPIXELFORMAT		GetZFormat()						{ return &ZFormat; }

		// caps methods
		void				CheckCaps(LPDIRECT3DDEVICE3 the_device);
#ifdef TARGET_DC
		bool				ModulateAlphaSupported()			{ return TRUE; }
		bool				DestInvSourceColourSupported()		{ return TRUE; }
		bool				AdamiLightingSupported()			{ return FALSE; }
#else
		bool				ModulateAlphaSupported()			{ return CanDoModulateAlpha; }
		bool				DestInvSourceColourSupported()		{ return CanDoDestInvSourceColour; }
		bool				AdamiLightingSupported()			{ return CanDoAdamiLighting; }
#endif

/*
    LPDDModeInfo FindFormat (LPDDPIXELFORMAT lpddsd, 
						     LPDDModeInfo * lpNextBest,
						     LPDDModeInfo lpStartFormat = NULL);

    DWORD EnumFormats (const D3DDEV_ENUMINFO & eiInfo);
*/
};

//---------------------------------------------------------------

class	DDDriverInfo
{
	private:
	protected:
	public:
		ULONG			DriverFlags;					// D3D Driver flags

		// Driver info.
		GUID            guid,                           // guid, if any
						hardware_guid;
		LPTSTR          szName;							// name
		LPTSTR          szDesc;							// description
		DDCAPS          ddHalCaps;                      // Hardware caps
		DDCAPS          ddHelCaps;                      // Emulation caps

		// Mode Info
		ULONG			ModeCount;						// Count of Modes.
		DDModeInfo		*ModeList,						// List of Modes.
						*ModeListEnd;

		// D3D Info
		ULONG			DeviceCount;					// Count of D3D devices.
		D3DDeviceInfo	*DeviceList,					// List of D3D Devices.
						*DeviceListEnd;

		DDDriverInfo		*Next,
							*Prev;

							DDDriverInfo();
							~DDDriverInfo();

		// Driver methods.
		HRESULT				Create(GUID	*lpGuid,LPTSTR	lpDriverName,LPTSTR	lpDriverDesc);
		void				Destroy(void);

		BOOL				Match(GUID *the_guid);

		GUID				*GetGuid(void);

		inline	BOOL		IsValid(void)				{	return	DriverFlags&DD_DRIVER_VALID;	}
		inline	void		ValidOn(void)				{	DriverFlags		|=	DD_DRIVER_VALID;	}
		inline	void		ValidOff(void)				{	DriverFlags		&=	~DD_DRIVER_VALID;	}

		inline	BOOL		IsPrimary(void)				{	return	DriverFlags&DD_DRIVER_PRIMARY;	}
		inline	void		PrimaryOn(void)				{	DriverFlags		|=	DD_DRIVER_PRIMARY;	}
		inline	void		PrimaryOff(void)			{	DriverFlags		&=	~DD_DRIVER_PRIMARY;	}

		inline	BOOL		IsD3D(void)					{	return	DriverFlags&DD_DRIVER_D3D;		}
		inline	void		D3DOn(void)					{	DriverFlags		|=	DD_DRIVER_D3D;		}
		inline	void		D3DOff(void)				{	DriverFlags		&=	~DD_DRIVER_D3D;		}


		// Mode methods.
		HRESULT				LoadModes(LPDIRECTDRAW4 lpDD4);
		HRESULT				DestroyModes(void);

		HRESULT				AddMode(DDModeInfo *the_mode);
		HRESULT				DeleteMode(DDModeInfo *the_mode);
		DDModeInfo			*FindMode(SLONG w,SLONG h,SLONG bpp,SLONG refresh,DDModeInfo **next_best=NULL,DDModeInfo *start_mode=NULL);

		inline	SLONG		CountModes(void)			{	return ModeCount;						}
		inline	BOOL		ModesLoaded(void)			{	return ((DriverFlags&DD_DRIVER_M_LOADED) ? TRUE : FALSE);	}
		inline	void		TurnModesLoadedOn(void)		{	DriverFlags		|=	DD_DRIVER_M_LOADED;	}
		inline	void		TurnModesLoadedOff(void)	{	DriverFlags		&=	~DD_DRIVER_M_LOADED;}



		// Device methods.
		HRESULT				LoadDevices(LPDIRECT3D3 lpD3D3);
		HRESULT				DestroyDevices(void);

		HRESULT				AddDevice(D3DDeviceInfo *the_device);
		HRESULT				DeleteDevice(D3DDeviceInfo *the_device);
		D3DDeviceInfo		*FindDevice(GUID *the_guid, D3DDeviceInfo **next_best,D3DDeviceInfo *start_device=NULL);
		D3DDeviceInfo		*FindDeviceSupportsMode(GUID *the_guid,DDModeInfo *the_mode,D3DDeviceInfo **next_best_device,D3DDeviceInfo *start_device=NULL);
		DDModeInfo			*FindModeSupportsDevice(SLONG w, SLONG h, SLONG bpp,SLONG refresh,D3DDeviceInfo *the_device,DDModeInfo **next_best,DDModeInfo *start_device=NULL);

		inline	SLONG		CountDevices(void)			{	return DeviceCount;						}
		inline	BOOL		DevicesLoaded(void)			{	return ((DriverFlags&DD_DRIVER_D_LOADED) ? TRUE : FALSE);	}
		inline	void		TurnDevicesLoadedOn(void)	{	DriverFlags		|=	DD_DRIVER_D_LOADED;	}
		inline	void		TurnDevicesLoadedOff(void)	{	DriverFlags		&=	~DD_DRIVER_D_LOADED;}

};

//---------------------------------------------------------------

class	DDDriverManager
{
	private:
	protected:
	public:
		ULONG				ManagerFlags,				// Global flags
							DriverCount;				// Count of DD Drivers
		DDDriverInfo		*DriverList,				// List of DD Device Drivers.
							*DriverListEnd;

		DDDriverInfo		*CurrDriver;				// Pointer to current DD Device driver
		DDModeInfo			*CurrMode;					// Pointer to current mode in DD driver
		D3DDeviceInfo		*CurrDevice;				// Pointer to current D3D device
		DDModeInfo			*CurrTextureFormat;			// Pointer to current Texture Format


							DDDriverManager();
							~DDDriverManager();

		// Methods.
		HRESULT				Init(void);
		HRESULT				Fini(void);

		HRESULT				LoadDrivers(void);
		HRESULT				DestroyDrivers(void);

		HRESULT				AddDriver(DDDriverInfo *the_driver);
		DDDriverInfo		*FindDriver(GUID *guid,DDDriverInfo **next_best,DDDriverInfo *start_driver=NULL);
		DDDriverInfo		*FindDriver(DDCAPS *hal,DDCAPS *hel,DDDriverInfo **next_best,DDDriverInfo *start_driver=NULL);

		inline	BOOL		IsInitialised(void)			{	return	ManagerFlags&DD_DRIVER_INIT;		}
		inline	void		InitOn(void)				{	ManagerFlags	|=	DD_DRIVER_INIT;			}
		inline	void		InitOff(void)				{	ManagerFlags	&=	~DD_DRIVER_INIT;		}
};

extern DDDriverManager		the_manager;

//---------------------------------------------------------------

#define	InitStruct(s)		{ ZeroMemory(&(s), sizeof(s)); (s).dwSize = sizeof(s); }

#endif
