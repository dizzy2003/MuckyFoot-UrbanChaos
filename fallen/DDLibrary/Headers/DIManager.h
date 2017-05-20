// DIManager.h
// Guy Simmons, 19th February 1998.

#ifndef	DIMANAGER_H
#define	DIMANAGER_H

#ifdef TARGET_DC
#include "mapledev.h"
#include "lcd.h"
#include "vib.h"
#include "perstore.h"
#include "maptimer.h"
#endif

//---------------------------------------------------------------

#define	MOUSE			DIDEVTYPE_MOUSE
#define	KEYBOARD		DIDEVTYPE_KEYBOARD 
#define	JOYSTICK		DIDEVTYPE_JOYSTICK

//---------------------------------------------------------------

#define	DI_DRIVER_INIT			(1<<0)

#define	DI_DEVICE_VALID			(1<<0)
#define	DI_DEVICE_NEEDS_POLL	(1<<1)


#ifdef TARGET_DC
// DC needs to cope with different controllers
#define ENABLE_REMAPPING 1
#else
// PC doesn't
#define ENABLE_REMAPPING 0
#endif

//---------------------------------------------------------------

// If bActuallyGetOne is FALSE, then just the current types are set up, no device is actually grabbed.
void	ClearPrimaryDevice ( void );
BOOL	GetInputDevice ( UBYTE type, UBYTE sub_type, bool bActuallyGetOne = TRUE );
BOOL	ReadInputDevice(void);



#ifdef TARGET_DC








#ifdef TARGET_DC
struct VMU_Screen
{
	bool	bRotated;			// TRUE if this has been rotated.
	BYTE	bData[32*6];		// The bitmap data itself.
};

// Roattes a VMU screen. bRotated is the state you want the screen to be in.
// The rout will then rotate if necessary.
void RotateVMUScreen ( bool bRotated, VMU_Screen *pvmuScreen );

// A routine for converting a TGA to a VMU screen bitmap.
bool CreateVMUScreenFromTGA ( char *pchName, VMU_Screen **ppvmuScreen );

// Displays the given screen on all the screen devices on the current controller.
// If there is no primary, it displays it on all of the controllers.
// This is what you generally call to display an image.
bool WriteLCDScreenToCurrentController ( VMU_Screen *pvmuScreen );

#endif





// A VMU device.
class MapleVMU
{
public:
	MapleVMU				*pNextVMU;			// The next VMU on this device.

	MAPLEDEVTYPE			type;				// What sort of device is this?
	GUID					guid;				// The device's GUID.
	int						iEnumNumber;		// What number device is this on this controller (0/1)?
	bool					bUpsideDown;		// TRUE if a VMU screen is upside down or not.
	union
	{
		IUnknown			*pUnknown;			// Generic pointer
		//PLGUN				pLgun;				// Light gun interface.
		PLCD				pLcd;				// LCD interface.
		LPFLASHDEVICE		pFlash;				// Flash memory device
		PTMR				pTimer;				// Timer device
		PVIB				pVib;				// Vibration device
	};

	DWORD					dwLcdBufferId;		// ID for the LCD buffer.
	BYTE					*pLcdBuffer;		// The LCD buffer;

	bool					Vib_bGotDevInfo;	// Have we got the device's data yet?
	float					Vib_fMinFreq;		// Minimum vibration freq.
	float					Vib_fMaxFreq;		// Maximum vibration freq.

	MapleVMU ( void )
	{
		pNextVMU = NULL;
		type = 0;
		iEnumNumber = -1;
		pUnknown = NULL;
		bUpsideDown = TRUE;
		dwLcdBufferId = 0;
		pLcdBuffer = NULL;
		Vib_bGotDevInfo = FALSE;
		Vib_fMinFreq = 0.5f;
		Vib_fMaxFreq = 20.0f;
	}

	~MapleVMU ( void )
	{
		if ( pLcdBuffer != NULL )
		{
			ASSERT ( type == MDT_LCD );
			ASSERT ( pLcd != NULL );
			ASSERT ( dwLcdBufferId != 0 );
			pLcd->FreeLcdBuffer ( dwLcdBufferId );
			dwLcdBufferId = 0;
			pLcdBuffer = NULL;
		}
		if ( pUnknown != NULL )
		{
			// Whatever it was, release it.
			pUnknown->Release();
		}
	}

	// Call these to make sure you've grabbed the interface pointer.
	void EnsureDevicePtr ( void );

	// Write this standard 48x32 bitmap to the LCD.
	// Data format is 3x32 bytes, like you'd expect.
	// If bQueue is TRUE, this (almost) always works.
	// If bQueue is FALSE, if there is a problem, like it's busy,
	//	then it can fail, and the return is FALSE;
	// If the LCD screen is not a standard type, then it does its best,
	//	or fails and returns FALSE.
	bool Lcd_WriteScreen ( void *pvData, bool bQueue );


	// Write this screen to the LCD device.
	// The screen will be rotated as needed.
	// If bQueue is TRUE, this (almost) always works.
	// If bQueue is FALSE, if there is a problem, like it's busy,
	//	then it can fail, and the return is FALSE;
	// If the LCD screen is not a standard type, then it does its best,
	//	or fails and returns FALSE.
	bool Lcd_WriteScreen ( VMU_Screen *pvmuScreen, bool bQueue );



	// Get a directory of the flash device. Return is an array of strings.
	// YOU DO NOT OWN THIS ARRAY. If you want to use it or store it,
	// copy it. The array may change/move next time you do any Maple call.
	char **Flash_GetDirectory ( void );

	// Get the size of the given file. If it doesn't exist, the result is -1.
	DWORD Flash_GetFileSize ( char *pcFilename );

	// Reads the given file into pvData, which is of size dwSizeOfData.
	// Return is TRUE on success, FALSE on failure.
	bool Flash_ReadFile ( char *pcFilename, void *pvData, DWORD dwSizeOfData );

	// Creates the given file & writes the given data to it.
	// pcGameName is the game name you wish to be tagged onto the file. Must be less than 16 chars.
	// pcComment is any comment you wish to be tagged onto the file. Must be less than 16 chars.
	// If the file already exists, it is deleted.
	// If there is not enough space on the device, the call will fail.
	// Return is TRUE on success, FALSE on failure.
	bool Flash_WriteFile ( char *pcFilename, char *pcGameName, char *pcComment, void *pvData, DWORD dwSizeOfData,
								 char *pcIconPalette, char *pcIconData );

	// Returns the number of free blocks in this VMU.
	// If there is an error, -1 is returned.
	int Flash_GetFreeBlocks ( void );


};




//---------------------------------------------------------------

class	DIDeviceInfo
{
	private:
	protected:
	public:
		CBYTE				Instance[MAX_PATH],			// Device instance name.
							Product[MAX_PATH];			// Device product name.
		UBYTE				DeviceType,					// Device type, e.g. Joystick
							DeviceSubType;				// Device sub type. e.g. Traditional, Gamepad etc.
		UBYTE				NumButtons;					// Number of buttons on this device.
		UBYTE				NumAxis;					// Number of axis on this device.

		UBYTE				PortNumber;					// Port number, if known.

#if ENABLE_REMAPPING
		UBYTE				AxisMappings[2];			// Mapping from DI stuff to my standardised ones.
		UBYTE				ButtonMappings[32];			// A mapping from DI stuff to my standardised input mapping.
#endif

		ULONG				DeviceFlags;				// DI Device flags

		GUID				guidInstance;

		DIDeviceInfo		*Next,
							*Prev;
		LPDIRECTINPUTDEVICE2	lpdiInputDevice;		// DI Device.

#ifdef TARGET_DC
		// The VMUs connected to this device.
		MapleVMU			*pFirstVMU;					// The first VMU on this system.
		DWORD				dwDirection;				// The slot direction field.
#endif

							DIDeviceInfo();
							~DIDeviceInfo();

		// DeviceInfo methods.
		HRESULT				Create(LPCDIDEVICEINSTANCE lpDIDevice);
		void				Destroy(void);

		inline	BOOL		IsValid(void)				{	return	DeviceFlags&DI_DEVICE_VALID;	}
		inline	void		ValidOn(void)				{	DeviceFlags		|=	DI_DEVICE_VALID;	}
		inline	void		ValidOff(void)				{	DeviceFlags		&=	~DI_DEVICE_VALID;	}

		inline	BOOL		NeedsPoll(void)				{	return	DeviceFlags&DI_DEVICE_NEEDS_POLL;	}
		inline	void		NeedsPollOn(void)			{	DeviceFlags		|=	DI_DEVICE_NEEDS_POLL;	}
		inline	void		NeedsPollOff(void)			{	DeviceFlags		&=	~DI_DEVICE_NEEDS_POLL;	}

		BOOL				GetThisDevice ( UBYTE type );
		BOOL				DIEnumDeviceObjectsProc ( LPCDIDEVICEOBJECTINSTANCE pDIDOI );


};

//---------------------------------------------------------------

class	DIDriverManager
{
	private:
	protected:
	public:
		ULONG				ManagerFlags,				// Global flags
							DeviceCount;				// Count of DI Devices.

		bool				bVMUScreenUpdatesEnabled;

		DIDeviceInfo		*DeviceList,				// List of DI Device.
							*DeviceListEnd;
		LPDIRECTINPUT		lp_DI;						// The DirectInput object.

							DIDriverManager();
							~DIDriverManager();

		// Methods.
		HRESULT				Init(void);
		HRESULT				Fini(void);

		HRESULT				LoadDevices ( bool *pbChanged=NULL );
		HRESULT				DestroyAllDevices(void);
		HRESULT				DestroyDevice(DIDeviceInfo *the_device);

		HRESULT				AddDevice(DIDeviceInfo *the_driver);
		DIDeviceInfo		*FindDevice(UBYTE type,UBYTE sub_type,DIDeviceInfo **next_best,DIDeviceInfo *start_device=NULL);

		DIDeviceInfo		*FindFirstWithButtonPressed ( UBYTE type, UBYTE sub_type );

		int					ScanForVMUs ( void );

		inline	BOOL		IsInitialised(void)			{	return	ManagerFlags&DI_DRIVER_INIT;		}
		inline	void		InitOn(void)				{	ManagerFlags	|=	DI_DRIVER_INIT;			}
		inline	void		InitOff(void)				{	ManagerFlags	&=	~DI_DRIVER_INIT;		}
};

extern DIDriverManager		the_input_manager;




// Allows or disallows the automatic update of VMU screens.
// This can be quite slow, and for time-critical things (e.g.
// video playbacl), you'll want to turn it off. Do turn it back on
// afterwards though.
void SetVMUScreenUpdateEnable ( bool bEnable );


// Gets the first VMU on the primary controller. If it can't find it,
// it takes the first VMU on the first controller it finds.
MapleVMU *FindFirstVMUOnCurrentController ( void );


// Tries to find a memory VMU at slot iVMUNum on controller iCtrlNum.
// If not, returns NULL. Controllers are numbered 0-3, VMU numbers 0-1.
MapleVMU *FindMemoryVMUAt ( int iCtrlNum, int iVMUNum );


// Forces a rescan of devices. Any new devices will be added to the available list.
// This does not reassign the primary - if you want to do that,
// call ClearPrimaryDevice().
// If anything new was found, or anything existing was removed, returns TRUE.
bool RescanDevices ( void );

// Often called after a RescanDevices - deletes any missing devices.
void DeleteInvalidDevice ( void );

// Returns the current VMU. If it can't be found any more, and
// bFindNextBest is TRUE, it tries to find the first one on the
// primary, and then tries to find the first one on anything.
// If bFindNextBest is FALSE, it just returns NULL.
MapleVMU *FindCurrentStorageVMU ( bool bFindNextBest );

// Sets the current storage VMU. If NULL, there will be no current VMU.
void SetCurrentStorageVMU ( MapleVMU *pVMU );



// Tries to find a vibration VMU on the primary. If there are two (unlikely),
// it retruns the first one. If there are none, it returns NULL.
// Yes, I know they're not actually VMUs that vibrate.
MapleVMU *FindFirstVibratorOnCurrentController ( void );


// Turn vibrations on or off.
void SetVibrationEnable ( bool bEnabled );


// Make the vibrator in the primary device vibrate with the given
// characteristics. Returns TRUE if it works, or FALSE if not.
// The most common cause of FALSE is that another vibration
// is already happening, or was set off very recently.
//
// fFrequency is in Hz.
// fStartPower is from 1.0 (max) to 0.0
// fShrinkTime is the time in seconds for the power to shrink to 0.
//		If this is 0, the effect will be a one-shot one - a single jolt, basically.
// bEnsureThisHappens - set this to TRUE to wait for up to a tenth of a second
//		for this to be activated. This is for important thing.
bool Vibrate ( float fFrequency, float fStartPower, float fShrinkTime, bool bEnsureThisHappens=FALSE );



#endif

//---------------------------------------------------------------





#endif
