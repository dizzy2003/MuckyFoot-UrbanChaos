// DIManager.cpp
// Guy Simmons, 19th February 1998

#include	"DDLib.h"
#ifndef TARGET_DC
#include	"FFManager.h"
#else
#include	"maplusag.h"
#endif

#ifdef TARGET_DC
#include "target.h"
#endif

// TEMP
DIJOYSTATE			the_state;


#ifdef TARGET_DC	// This whole file is just for Dreamcast now...

DIDeviceInfo		*primary_device	=	NULL;
DIDriverManager		the_input_manager;

UBYTE last_type;
UBYTE last_sub_type;



#ifdef DEBUG
// Enable some tracing.
#define SHARON TRACE

#else

// Don't dump info.
#define SHARON sizeof

#endif





#if ENABLE_REMAPPING
#ifdef TARGET_DC

enum eAxisMapping
{
	MY_DC_X_AXIS = 0,
	MY_DC_Y_AXIS = 1
};

// These had better agree with the ones in interfac.h, or you're toast.
enum eButtonMapping
{
	MY_DC_BUTTON_A			= 1,
	MY_DC_BUTTON_B			= 0,
	MY_DC_BUTTON_C			= 11,
	MY_DC_BUTTON_D			= 13,
	MY_DC_BUTTON_X			= 8,
	MY_DC_BUTTON_Y			= 7,
	MY_DC_BUTTON_Z			= 12,
	MY_DC_BUTTON_START		= 2,
	MY_DC_BUTTON_UP			= 3,
	MY_DC_BUTTON_DOWN		= 4,
	MY_DC_BUTTON_LEFT		= 5,
	MY_DC_BUTTON_RIGHT		= 6,
	MY_DC_BUTTON_RTRIGGER	= 9,
	MY_DC_BUTTON_LTRIGGER	= 10,
	// General analogue buttons.
	MY_DC_BUTTON_AN3		= 14,
	MY_DC_BUTTON_AN4		= 15,
	MY_DC_BUTTON_AN5		= 16,
	MY_DC_BUTTON_AN6		= 17,
	// Another direction pad. I think.
	MY_DC_BUTTON_2_UP		= 18,
	MY_DC_BUTTON_2_DOWN		= 19,
	MY_DC_BUTTON_2_LEFT		= 20,
	MY_DC_BUTTON_2_RIGHT	= 21,

	// Special one - any unmapped buttons are remapped here. Don't read it!
	MY_DC_BUTTON_BOGUS		= 31,
};

#endif
#endif


//---------------------------------------------------------------

extern HINSTANCE			hGlobalThisInst;

//---------------------------------------------------------------
//
//	User functions.
//
//---------------------------------------------------------------


// Clears the current primary device, and does everything
// needed to start searching for thenext one to press a button again.
void ClearPrimaryDevice ( void )
{
	SHARON ( "ClearPrimaryDevice\n" );
	primary_device = NULL;
}


// Used for a simple count.
int m_iNumMapleDevices;



// Forces a rescan of devices. Any new devices will be added to the available list.
// This does not reassign the primary - if you want to do that,
// call ClearPrimaryDevice().
// If anything new was found, or anything existing was removed, returns TRUE.
bool RescanDevices ( void )
{
	bool bChanged = FALSE;
	HRESULT result = the_input_manager.LoadDevices ( &bChanged );
	return bChanged;
}


// Often called after a RescanDevices - deletes any missing devices.
void DeleteInvalidDevice ( void )
{
	DIDeviceInfo	*current_device,
					*next_device;

	current_device	=	the_input_manager.DeviceList;
	while(current_device)
	{
		next_device		=	current_device->Next;

		if ( !(current_device->IsValid()) )
		{
			// Destroy it then
			the_input_manager.DestroyDevice ( current_device );
		}
		current_device	=	next_device;
	}
}


BOOL	GetInputDevice ( UBYTE type, UBYTE sub_type, bool bActuallyGetOne )
{

	SHARON ( "GetInputDevice\n" );

	last_type = type;
	last_sub_type = sub_type;

	if ( bActuallyGetOne )
	{
#ifdef TARGET_DC
		// Tsk tsk.
		ASSERT ( FALSE );
#endif

		DIDeviceInfo *the_device;
		ZeroMemory(&the_state,sizeof(the_state));
		the_device	=	the_input_manager.FindDevice(type,sub_type,NULL);
		if ( the_device != NULL )
		{
			return ( the_device->GetThisDevice ( type ) );
		}
	}
	else
	{
		return TRUE;
	}
}


//---------------------------------------------------------------

// Returns TRUE if there are any devices connected,
// whether or not any of them is the primary.
BOOL AreAnyDevicesConnected ( void )
{
	if ( the_input_manager.DeviceList != NULL )
	{
		ASSERT ( the_input_manager.DeviceCount != 0 );
		return TRUE;
	}
	else
	{
		//ASSERT ( the_input_manager.DeviceCount == 0 );
		return FALSE;
	}
}



//---------------------------------------------------------------

BOOL	ReadInputDevice(void)
{
	BOOL			read_it	=	FALSE;
	HRESULT			result;

	if ( primary_device ==  NULL )
	{
#ifdef TARGET_DC
		// No current primary, so keep scanning existing controllers
		// for a one to press a button, then make that the primary.
		primary_device = the_input_manager.FindFirstWithButtonPressed ( last_type, last_sub_type );
		if ( primary_device != NULL )
		{
			// Go on to read it.
		}
		else
		{
			// None has pressed a button yet, which is fine.
			return FALSE;
		}
#else //#ifdef TARGET_DC
		primary_device = the_input_manager.FindDevice(JOYSTICK,0,NULL);
		if (primary_device == NULL)
		{
			return FALSE;
		}
		else
		{
			return primary_device->GetThisDevice(JOYSTICK);
		}
#endif //#else //#ifdef TARGET_DC
	}


	if ( primary_device != NULL )
	{
#ifdef TARGET_DC
		ASSERT ( !primary_device->NeedsPoll() );
#else
		//if(primary_device->NeedsPoll())
		{
			result	=	primary_device->lpdiInputDevice->Poll();
			switch(result)
			{
				case	DIERR_INPUTLOST:
				case	DIERR_NOTACQUIRED:
					result	=	primary_device->lpdiInputDevice->Acquire();

					if (result != DI_OK)
					{
						return read_it;
					}

					//if(FAILED(result))
					//	return read_it;
					break;
				case	DIERR_NOTINITIALIZED:
					break;
			}
		}
#endif

#if ENABLE_REMAPPING
		DIJOYSTATE temp_state;
		result	=	primary_device->lpdiInputDevice->GetDeviceState(sizeof(temp_state),&temp_state);
#else
		result	=	primary_device->lpdiInputDevice->GetDeviceState(sizeof(the_state),&the_state);
#endif
		switch(result)
		{
			default:
				SHARON ( "Primary Device fell over in some way\n" );
				result	=	primary_device->lpdiInputDevice->Acquire();

				if (result != DI_OK)
				{
#ifdef TARGET_DC
					// Start searching for primary devices again.
					ClearPrimaryDevice();
#endif
					return read_it;
				}

				//if(FAILED(result))
				//	return read_it;

				break;
			case	DI_OK:

#if ENABLE_REMAPPING
				// Remap from DI internals to my standard mappings.

				// Can't be arsed to remap the axis.
				// But, if there aren't enough (e.g. lightgun, which has no axis),
				// set them to mid-way to stop them interfereing.

				if ( primary_device->NumAxis >= 1 )
				{
					the_state.lX = temp_state.lX;
				}
				else
				{
					the_state.lX = 128;
				}
				if ( primary_device->NumAxis >= 2 )
				{
					the_state.lY = temp_state.lY;
				}
				else
				{
					the_state.lY = 128;
				}

				for ( int i = 0; i < primary_device->NumButtons; i++ )
				{
					ASSERT ( primary_device->ButtonMappings[i] != (UBYTE) -1 );
					the_state.rgbButtons[primary_device->ButtonMappings[i]] = temp_state.rgbButtons[i];
				}
#endif

				read_it	=	TRUE;
				break;
		}
	}
	return	read_it;
}




//---------------------------------------------------------------
//
//	Callback functions.
//
//---------------------------------------------------------------

BOOL CALLBACK	DIDeviceEnumCallback	(
											LPCDIDEVICEINSTANCE	lpDIDevice,
											LPVOID				lpExtra
										)
{
    CallbackInfo		*the_info;
    DIDeviceInfo		*new_device;
	HRESULT				result;

	//SHARON ( "DIDeviceEnumCallback\n" );

    if(!lpExtra)
    {
        // Programming error, invalid pointer
        return	DIENUM_STOP;
    }

    the_info	=	(CallbackInfo*)lpExtra;


	// See if we already have this device.
	DIDeviceInfo *pCurDev = the_input_manager.DeviceList;
	while ( pCurDev != NULL )
	{
		if ( IsEqualGUID ( pCurDev->guidInstance, lpDIDevice->guidInstance ) )
		{
			// Already got this one - skip it.
			pCurDev->ValidOn();
			return	DIENUM_CONTINUE;
		}

		pCurDev = pCurDev->Next;
	}

#ifdef DEBUG
	SHARON ( "DIDeviceEnumCallback: found something new.\n" );
	char pcTemp[200];
	textConvertUniToChar ( pcTemp, lpDIDevice->tszProductName );
	SHARON ( "FFWBP:Device says it's a <%s>\n", pcTemp );
#endif

    // create DeviceInfo instance.
    new_device	=	MFnew<DIDeviceInfo>();
    if(!new_device)
    {
        // Error, Not enough memory to create DeviceInfo.
        return	DIENUM_STOP;
    }
	
    // Setup the device info.
    result	=	new_device->Create(lpDIDevice);
    if(FAILED(result))
    {
        // Error, DeviceInfo setup failed.
        return	DIENUM_STOP;
    }

	// Add To Global DeviceInfo List
	result	=	the_input_manager.AddDevice(new_device);
	if(FAILED(result))
	{
        // Error, DeviceInfo setup failed.
		return	DIENUM_STOP;
	}

	new_device->ValidOn();

    // Increment device count.
    the_info->Count++;

    // Success.
	return	DIENUM_CONTINUE;
}

//---------------------------------------------------------------
//
//	CLASS	:	DIDeviceInfo
//
//---------------------------------------------------------------

DIDeviceInfo::DIDeviceInfo()
{
	SHARON ( "DIDeviceInfo creator\n" );
	DeviceFlags	=	0;
	NumButtons  =	0;
	NumAxis		=	0;
	PortNumber	=	-1;
	pFirstVMU	=	NULL;

#if ENABLE_REMAPPING
	AxisMappings[0] = -1;
	AxisMappings[1] = -1;
	for ( int i = 0; i < 32; i++ )
	{
		ButtonMappings[i] = -1;
	}
#endif

	Next		=	NULL;
	Prev		=	NULL;

}

//---------------------------------------------------------------

DIDeviceInfo::~DIDeviceInfo()
{
	SHARON ( "DIDeviceInfo destructor\n" );
	Destroy();
}

//---------------------------------------------------------------

HRESULT	DIDeviceInfo::Create(LPCDIDEVICEINSTANCE lpDIDevice)
{
	HRESULT					result;
	LPDIRECTINPUTDEVICE		lp_di_device;

	SHARON ( "DIDeviceInfo::Create\n" );


	// Copy the device type.
	DeviceType		=	LOBYTE(lpDIDevice->dwDevType);
	DeviceSubType	=	HIBYTE(lpDIDevice->dwDevType);

#ifndef TARGET_DC
	// Copy the instance name.
	strcpy(Instance,lpDIDevice->tszInstanceName);

	// Copy the product name.
	strcpy(Product,lpDIDevice->tszProductName);
#else

	// Copy the instance name.
	textConvertUniToChar(Instance,lpDIDevice->tszInstanceName);

	// Copy the product name.
	textConvertUniToChar(Product,lpDIDevice->tszProductName);

#endif

	// Copy the GUID.
	guidInstance = lpDIDevice->guidInstance;

	// Now create the device.
	result	=	the_input_manager.lp_DI->CreateDevice	(
															lpDIDevice->guidInstance,
															&lp_di_device,
															NULL
														);
	if(FAILED(result))
		return	result;

	result	=	lp_di_device->QueryInterface(IID_IDirectInputDevice2,(LPVOID*)&lpdiInputDevice);
	di_error(result);
	if(SUCCEEDED(result))
		ValidOn();


	// Get the port number of the device
	DIPROPDWORD         dipropdword;
	dipropdword.diph.dwSize       = sizeof(DIPROPDWORD);
	dipropdword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipropdword.diph.dwObj = 0;
	dipropdword.diph.dwHow = DIPH_DEVICE;
	result = lpdiInputDevice->GetProperty(DIPROP_PORTNUMBER, &dipropdword.diph);
	ASSERT (SUCCEEDED(result));
	PortNumber = (UBYTE)dipropdword.dwData;

#ifdef TARGET_DC
	// Get the device's direction.
	result = lpdiInputDevice->GetProperty(DIPROP_EXPDIRECTION, &dipropdword.diph);
	ASSERT (SUCCEEDED(result));
	dwDirection = dipropdword.dwData;
#endif

	lp_di_device->Release();

	return	result;
}

//---------------------------------------------------------------

void	DIDeviceInfo::Destroy(void)
{
	SHARON ( "DIDeviceInfo::Destroy\n" );
	if(IsValid())
		lpdiInputDevice->Release();

	Prev	=	NULL;
	Next	=	NULL;

	ValidOff();
}




//---------------------------------------------------------------




// The EnumDeviceObjects callback.
BOOL DIDeviceInfo::DIEnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE pDIDOI)
{
    if ((LOBYTE(LOWORD(pDIDOI->dwType)) & DIDFT_AXIS) != 0)
    {
#if ENABLE_REMAPPING
		if ( NumAxis < 2 )
		{
			if (IsEqualGUID(pDIDOI->guidType, GUID_XAxis))
			{
				AxisMappings[NumAxis] = MY_DC_X_AXIS;
			}
			else
			{
				AxisMappings[NumAxis] = MY_DC_Y_AXIS;
			}
		}
#endif
        NumAxis++;
    }
    else if ((LOBYTE(LOWORD(pDIDOI->dwType)) & DIDFT_BUTTON) != 0)
    {
#if ENABLE_REMAPPING
		if ( NumButtons < 32 )
		{
			int iMapping = -1;
			switch ( pDIDOI->wUsage )
			{
			case USAGE_A_BUTTON      : iMapping = MY_DC_BUTTON_A; break;
			case USAGE_B_BUTTON      : iMapping = MY_DC_BUTTON_B; break;
			case USAGE_C_BUTTON      : iMapping = MY_DC_BUTTON_C; break;
			case USAGE_D_BUTTON      : iMapping = MY_DC_BUTTON_D; break;
			case USAGE_START_BUTTON  : iMapping = MY_DC_BUTTON_START; break;
			case USAGE_LA_BUTTON     : iMapping = MY_DC_BUTTON_LEFT; break;
			case USAGE_RA_BUTTON     : iMapping = MY_DC_BUTTON_RIGHT; break;
			case USAGE_DA_BUTTON     : iMapping = MY_DC_BUTTON_DOWN; break;
			case USAGE_UA_BUTTON     : iMapping = MY_DC_BUTTON_UP; break;
			case USAGE_X_BUTTON      : iMapping = MY_DC_BUTTON_X; break;
			case USAGE_Y_BUTTON      : iMapping = MY_DC_BUTTON_Y; break;
			case USAGE_Z_BUTTON      : iMapping = MY_DC_BUTTON_Z; break;
			case USAGE_LB_BUTTON     : iMapping = MY_DC_BUTTON_2_LEFT; break;
			case USAGE_RB_BUTTON     : iMapping = MY_DC_BUTTON_2_RIGHT; break;
			case USAGE_DB_BUTTON     : iMapping = MY_DC_BUTTON_2_DOWN; break;
			case USAGE_UB_BUTTON     : iMapping = MY_DC_BUTTON_2_UP; break;
			case USAGE_RTRIG_BUTTON  : iMapping = MY_DC_BUTTON_RTRIGGER; break;
			case USAGE_LTRIG_BUTTON  : iMapping = MY_DC_BUTTON_LTRIGGER; break;
			case USAGE_AN3_BUTTON    : iMapping = MY_DC_BUTTON_AN3; break;
			case USAGE_AN4_BUTTON    : iMapping = MY_DC_BUTTON_AN4; break;
			case USAGE_AN5_BUTTON    : iMapping = MY_DC_BUTTON_AN5; break;
			case USAGE_AN6_BUTTON    : iMapping = MY_DC_BUTTON_AN6; break;
			default:
				SHARON ( "Eh? Unknown button number\n" );
				break;
			}
			ButtonMappings[NumButtons] = (UBYTE)iMapping;
		}
#endif
		NumButtons++;
    }
    else
	{
		SHARON ( "Ooh - it's a wacky joypad object!\n" );
	}

    return(TRUE);
}




// Useful stub to convert from a callback to something more sane.
BOOL CALLBACK
DIEnumDeviceObjectsProcStub(LPCDIDEVICEOBJECTINSTANCE pDIDOI, LPVOID pvContext)
{
    DIDeviceInfo *pDevice = (DIDeviceInfo *)pvContext;
    return(pDevice->DIEnumDeviceObjectsProc(pDIDOI));
}




BOOL DIDeviceInfo::GetThisDevice ( UBYTE type )
{
	SHARON ( "DIDeviceInfo::GetThisDevice\n" );

	DIDEVCAPS			di_dcaps;
	HRESULT				result;


	DWORD coopflags;
	// Set up the coop level.
/*	result	=	this->lpdiInputDevice->SetCooperativeLevel(
																hDDLibWindow,
																DISCL_NONEXCLUSIVE	|
																DISCL_FOREGROUND
															); */
	if (type==JOYSTICK) {
		// Force Feedback *requires* exclusive mode
		coopflags = DISCL_EXCLUSIVE	|	DISCL_FOREGROUND;
	} else {
		// Probably keyboard or mouse -- use nonexclusively
		coopflags = DISCL_NONEXCLUSIVE	|	DISCL_FOREGROUND;
	}


	result	=	lpdiInputDevice->SetCooperativeLevel(
																	hDDLibWindow,
																	coopflags
																);
	if(FAILED(result))
		return	FALSE;

	// Set the data format.  Fudged for Joysticks for now.
	result	=	lpdiInputDevice->SetDataFormat(&c_dfDIJoystick);
	if(FAILED(result))
		return	FALSE;

	// Get the device capabilities, mainly to find out if we need to poll.
	InitStruct(di_dcaps);

	result	=	lpdiInputDevice->GetCapabilities(&di_dcaps);
	if(FAILED(result))
		return	FALSE;

	if(di_dcaps.dwFlags&DIDC_POLLEDDATAFORMAT)
	{
#ifdef TARGET_DC
		ASSERT ( FALSE );
#endif
		this->NeedsPollOn();
	}



	//ASSERT ( di_dcaps.dwButtons < 256 );
	//NumButtons = (UBYTE)di_dcaps.dwButtons;




    // EnumObjects is how we determine how many buttons and axes this controller
    // has.  By interpreting this data correctly we can automatically work with
    // new controllers that have a different number of buttons, etc.
    // DirectInput will call our DIEnumDeviceObjectsProcStub (which will then
    // call our DIEnumDeviceObjectsProc) once for each object (button or axis)
    // on the device.
	NumButtons  =	0;
	NumAxis		=	0;
#if ENABLE_REMAPPING
	AxisMappings[0] = -1;
	AxisMappings[1] = -1;
	for ( int i = 0; i < 32; i++ )
	{
		ButtonMappings[i] = -1;
	}
#endif

	result = lpdiInputDevice->EnumObjects(DIEnumDeviceObjectsProcStub, this, 0);
	if ( FAILED ( result ) )
	{
		return FALSE;
	}

    result = lpdiInputDevice->SetDataFormat(&c_dfDIJoystick);
	if ( FAILED ( result ) )
	{
		return FALSE;
	}


	// Anything that has the word "fishing" in its title is reporting crappy things - 
	// ignore all slightly odd buttons and both triggers for this device.

	if ( ( strstr ( Product, "fishing" ) != NULL ) ||
		 ( strstr ( Product, "FISHING" ) != NULL ) ||
		 ( strstr ( Product, "Fishing" ) != NULL ) )
	{
		for ( int j = 0; j < 32; j++ )
		{
			switch ( ButtonMappings[j] )
			{
			case MY_DC_BUTTON_C				: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_Z				: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_2_LEFT		: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_2_RIGHT		: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_2_DOWN		: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_2_UP			: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_RTRIGGER		: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_LTRIGGER		: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_AN3			: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_AN4			: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_AN5			: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			case MY_DC_BUTTON_AN6			: ButtonMappings[j] = MY_DC_BUTTON_BOGUS; break;
			default:
				// Fine.
				break;
			}
		}
	}


	// Initially acquire the device.
	result	=	lpdiInputDevice->Acquire();
	if(FAILED(result))
		return	FALSE;

	return	TRUE;
}





//---------------------------------------------------------------
//
//	CLASS	:	DIDriverManager
//
//---------------------------------------------------------------

DIDriverManager::DIDriverManager()
{
	ManagerFlags		=	0;
	DeviceCount			=	0;
	DeviceList			=	NULL;
	DeviceListEnd		=	NULL;
	bVMUScreenUpdatesEnabled = TRUE;
}

//---------------------------------------------------------------

DIDriverManager::~DIDriverManager()
{
	Fini();
}

//---------------------------------------------------------------

HRESULT	DIDriverManager::Init(void)
{
	SHARON ( "DIDriverManager::Init\n");
	HRESULT			result;


	if(!IsInitialised())
	{
#if 1
		result	=	DirectInputCreate	(
											hGlobalThisInst,
											DIRECTINPUT_VERSION,
											&lp_DI,
											NULL
										);
#else

		CoInitialize(NULL);

		CoCreateInstance(
			CLSID_DirectInput8,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IDirectInput8W,
			(void **) &lp_DI);

		result = lp_DI->Initialize(hGlobalThisInst, DIRECTINPUT_VERSION);

		/*


		result	=	DirectInputCreateEx	(
											hGlobalThisInst,
											DIRECTINPUT_VERSION,
											IID_IDirectInput2,
											&lp_DI,
											NULL
										);
		*/
#endif
		if(FAILED(result))
		{
			switch(result)
			{
				case DIERR_BETADIRECTINPUTVERSION:
					return result;
					break;

				case DIERR_OLDDIRECTINPUTVERSION:
					return result;
					break;

				default:
					break;
			}

			return result;
		}

		result	=	LoadDevices();
		if(FAILED(result))
			return	result;

#ifndef TARGET_DC
		the_ff_manager = new FFManager();
#endif

		InitOn();
	}
	return	DI_OK;
}

//---------------------------------------------------------------

HRESULT	DIDriverManager::Fini(void)
{
	SHARON ( "DIDriverManager::Fini\n" );
	if(IsInitialised())
	{
#ifndef TARGET_DC
		MFdelete(the_ff_manager);
		the_ff_manager=NULL;
#endif

		DestroyAllDevices();

		InitOff();
	}
	return	DI_OK;
}

//---------------------------------------------------------------

// Can be called multiple times - will find any new devices and add them, but retain any existing ones.
// If pbChanged is non-NULL, it will be set to TRUE if there are any new devices.
// Note - DC - it WILL bin any VMU devices and remake them.
HRESULT DIDriverManager::LoadDevices ( bool *pbChanged )
{
	//SHARON ( "DIDriverManager::LoadDevices\n" );

	CallbackInfo	callback_info;
	HRESULT			result;

    // Initialize all valid drivers in system
    callback_info.Result	=	TRUE;
    callback_info.Count		=	0L;
    callback_info.Extra		=	(void*)NULL;

	int iOldDeviceCount = DeviceCount;

	// First set all the devices to invalid - the enum rout will set them to valid.
	DIDeviceInfo *pCurDev = the_input_manager.DeviceList;
	while ( pCurDev != NULL )
	{
		pCurDev->ValidOff();
		pCurDev = pCurDev->Next;
	}


	ASSERT ( lp_DI != NULL );
	result	=	lp_DI->EnumDevices(NULL, DIDeviceEnumCallback, &callback_info, DIEDFL_ALLDEVICES/* DIEDFL_ATTACHEDONLY */);
	if(FAILED(result))
	{
		return result;
	}

	// Double check count.
	if((!callback_info.Result) || (DeviceCount != callback_info.Count))
	{
		result	=	DDERR_GENERIC;
	}


	// Clean up if the primary got removed.
	if ( ( primary_device != NULL ) && ( !(primary_device->IsValid()) ) )
	{
		primary_device = NULL;
	}


	if ( pbChanged != NULL )
	{
		if ( DeviceCount > (unsigned)iOldDeviceCount )
		{
			// Yes, something new.
			*pbChanged = TRUE;
		}
		// See if any got removed.
		pCurDev = the_input_manager.DeviceList;
		while ( pCurDev != NULL )
		{
			if ( !(pCurDev->IsValid()) )
			{
				*pbChanged = TRUE;
			}
			pCurDev = pCurDev->Next;
		}
	}


#ifdef TARGET_DC
	// And scan for the Maple devices that hang off them.
	// Note that there is no need to change *pbChanged,
	// coz removing and adding VMUs causes controller disconnect/reconnect anyway,
	// which should be spotted.
	// Oh - removing a VMU doesn't cause a disconnect - so we do have to count them.
	int iPrevNumVMUs = m_iNumMapleDevices;
	int iNewNumVMUs = ScanForVMUs();
	ASSERT ( iNewNumVMUs == m_iNumMapleDevices );
	if ( iPrevNumVMUs != iNewNumVMUs )
	{
		if ( pbChanged != NULL )
		{
			*pbChanged = TRUE;
		}
	}
#endif

	return	result;
}

//---------------------------------------------------------------

HRESULT DIDriverManager::DestroyAllDevices(void)
{
	SHARON ( "DIDriverManager::DestroyDevices\n" );

	DIDeviceInfo	*current_device,
					*next_device;

	current_device	=	DeviceList;
	while(current_device)
	{
		next_device		=	current_device->Next;
		DestroyDevice ( current_device );
		current_device	=	next_device;
	}

	ASSERT ( DeviceList == NULL );
	ASSERT ( DeviceListEnd == NULL );

	return	DI_OK;
}

//---------------------------------------------------------------

HRESULT DIDriverManager::DestroyDevice ( DIDeviceInfo *the_device )
{
	SHARON ( "DIDriverManager::DestroyDevices\n" );

	DIDeviceInfo	*current_device;

	current_device	=	DeviceList;
	while(current_device)
	{
		if ( current_device == the_device )
		{
			if ( current_device->Prev == NULL )
			{
				ASSERT ( DeviceList == current_device );
				DeviceList = current_device->Next;
			}
			else
			{
				ASSERT ( DeviceList != current_device );
				current_device->Prev->Next = current_device->Next;
			}

			if ( current_device->Next == NULL )
			{
				ASSERT ( DeviceListEnd == current_device );
				DeviceListEnd = current_device->Prev;
			}
			else
			{
				ASSERT ( DeviceListEnd != current_device );
				current_device->Next->Prev = current_device->Prev;
			}

			MFdelete(current_device);

			break;
		}

		current_device	=	current_device->Next;
	}

	return	DI_OK;
}

//---------------------------------------------------------------

HRESULT	DIDriverManager::AddDevice(DIDeviceInfo *the_device)
{
	SHARON ( "DIDriverManager::AddDevice\n" );

	if(!the_device)
	{
		// Error, Invalid parameters
		return	DIERR_INVALIDPARAM;
	}

	// Add device to list.
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

	return	DI_OK;
}

//---------------------------------------------------------------

DIDeviceInfo *DIDriverManager::FindDevice(UBYTE type,UBYTE sub_type,DIDeviceInfo **next_best,DIDeviceInfo *start_device)
{
	SHARON ( "DIDriverManager::FindDevice\n" );

    DIDeviceInfo	*current_device;


    // Get Start node
	if(!start_device)
		current_device	=	DeviceList;
	else
		current_device	=	start_device;

	if(next_best)
		*next_best	=	current_device;

	while(current_device)
	{
		if(current_device->DeviceType==type)
		{
			if(sub_type)
			{
				if(current_device->DeviceSubType==sub_type)
				{
					// Success.
					return	current_device;
				}
				else
				{
					if(next_best)
						*next_best	=	current_device;
				}
			}
			else
			{
				// Success.
				return	current_device;
			}

		}
		current_device	=	current_device->Next;
	}
    
    // Failure, user could use next best instead
    return	NULL;
}







static DWORD dwLastTimeWeSentPicciesToVMUs = 0;
// In milliseconds.
#define SEND_VMU_PICCIE_EVERY 2000

// Sega says I have to.
#define LOOK_FOR_START_NOT_JUST_ANY_BUTTON 1

DIDeviceInfo *DIDriverManager::FindFirstWithButtonPressed ( UBYTE type, UBYTE sub_type )
{
    DIDeviceInfo	*current_device;


	// Scan for new devices.
	LoadDevices();


	if ( ( timeGetTime() - dwLastTimeWeSentPicciesToVMUs ) > SEND_VMU_PICCIE_EVERY )
	{
		if ( bVMUScreenUpdatesEnabled )
		{
			// Send the "press any button" bitmap to the screens of all the devices.
			DIDeviceInfo *current_device = the_input_manager.DeviceList;
			while ( current_device != NULL )
			{
				MapleVMU *pVMU = current_device->pFirstVMU;
				while ( pVMU != NULL )
				{
					if ( pVMU->type == MDT_LCD )
					{
extern VMU_Screen *pvmuscreenPressStart;
						pVMU->Lcd_WriteScreen ( pvmuscreenPressStart, TRUE );
					}
					pVMU = pVMU->pNextVMU;
				}
				current_device = current_device->Next;
			}
		}
		dwLastTimeWeSentPicciesToVMUs = timeGetTime();
	}




	current_device	=	DeviceList;

	while(current_device)
	{
		bool bGoodDevice = FALSE;

		if(current_device->DeviceType==type)
		{
			if(sub_type)
			{
				if(current_device->DeviceSubType==sub_type)
				{
					bGoodDevice = TRUE;
				}
			}
			else
			{
				// Success.
				bGoodDevice = TRUE;
			}
		}

		if ( bGoodDevice )
		{
#ifdef TARGET_DC
			ASSERT ( !current_device->NeedsPoll() );
#else
			// Yep, scan this device for a pressed button.
			if(current_device->NeedsPoll())
			{
				HRESULT result	=	current_device->lpdiInputDevice->Poll();
				switch(result)
				{
					case	DIERR_INPUTLOST:
					case	DIERR_NOTACQUIRED:
						result	=	current_device->lpdiInputDevice->Acquire();
						if (result != DI_OK)
						{
							// Don't care.
						}
						result	=	current_device->lpdiInputDevice->Poll();
						if (result != DI_OK)
						{
							// Don't care.
						}
						break;
					case	DIERR_NOTINITIALIZED:
						// Don't care.
						break;
				}
			}
#endif

			DIJOYSTATE dijoyState;

			HRESULT result	=	current_device->lpdiInputDevice->GetDeviceState(sizeof(dijoyState),&dijoyState);
			switch(result)
			{
				default:
					result	=	current_device->lpdiInputDevice->Acquire();
					if (result != DI_OK)
					{
						// OK, maybe it hasn't been set up.
						if ( !current_device->GetThisDevice ( last_type ) )
						{
							// Nope - really stuffed. Right - bin this device - probably doesn't exist any more.
						    DIDeviceInfo *next_device = current_device->Next;
							SHARON ( "FFWBP:Device 0x%x is toast - destroying it\n", current_device );
							SHARON ( "FFWBP:Device said it was a <%s>\n", current_device->Product );
							DestroyDevice ( current_device );

							current_device = next_device;
							continue;
						}
					}
					break;
				case DI_OK:

					// See if any of the buttons were pressed.
					for ( int i = 0; i < current_device->NumButtons ; i++ )
					{
						if ( ( dijoyState.rgbButtons[i] & 0x80 ) != 0 )
						{
#if LOOK_FOR_START_NOT_JUST_ANY_BUTTON
							// Ignore non-Start buttons.
							if ( current_device->ButtonMappings[i] == MY_DC_BUTTON_START )
							{
#else
							// Ignore ones remapped to BOGUS.
							if ( current_device->ButtonMappings[i] != MY_DC_BUTTON_BOGUS )
							{
#endif

								// Got one.
								SHARON ( "FFWBP:Found down button %i on 0x%x\n", i, current_device );
								SHARON ( "FFWBP:Device says it's a <%s>\n", current_device->Product );

	#ifdef DEBUG
								{
									// Dump out the full controller/VMU info.
									SHARON ( "Controller/VMU dump.\n" );
									DIDeviceInfo *curdev = DeviceList;
									while(curdev)
									{
										SHARON ( "Port %i: device <%s>, %i axis, %i buttons.\n", curdev->PortNumber, curdev->Product, curdev->NumAxis, curdev->NumButtons );
										MapleVMU *pVMU = curdev->pFirstVMU;
										while ( pVMU != NULL )
										{
											SHARON ( "  Maple device number %i, type <", pVMU->iEnumNumber );

											if ( ( pVMU->type & MDT_CONTROLLER ) != 0 ) SHARON ( "controller " );
											if ( ( pVMU->type & MDT_STORAGE    ) != 0 ) SHARON ( "storage " );
											if ( ( pVMU->type & MDT_LCD        ) != 0 ) SHARON ( "lcd " );
											if ( ( pVMU->type & MDT_TIMER      ) != 0 ) SHARON ( "timer " );
											if ( ( pVMU->type & MDT_AUDIO_IN   ) != 0 ) SHARON ( "audio-in " );
											if ( ( pVMU->type & MDT_LIGHTGUN   ) != 0 ) SHARON ( "lightgun " );
											if ( ( pVMU->type & MDT_VIBRATION  ) != 0 ) SHARON ( "vibration " );

											// In theory, no device should have more than one bit set.
											ASSERT ( ( pVMU->type & ( pVMU->type - 1 ) ) == 0 );

											SHARON ( ">\n" );

											pVMU = pVMU->pNextVMU;
										}
										curdev = curdev->Next;
									}
									SHARON ( "Dump end.\n" );
								}
	#endif


								// Clear all VMU screens, apart from the ones on the device we are using.
								// On that, put the standard logo.
								DIDeviceInfo *curdev = DeviceList;
								while ( curdev != NULL )
								{
									MapleVMU *pVMU = curdev->pFirstVMU;
									while ( pVMU != NULL )
									{
										if ( pVMU->type == MDT_LCD )
										{
											BYTE bTest[192];
											if ( curdev == current_device )
											{
												// Display the UC logo.
	extern VMU_Screen *pvmuscreenUCLogo;
												pVMU->Lcd_WriteScreen ( pvmuscreenUCLogo, TRUE );
											}
											else
											{
												// Blank all the other screens.
												memset ( bTest, 0, 192 );
												pVMU->Lcd_WriteScreen ( bTest, TRUE );
											}

										}
										pVMU = pVMU->pNextVMU;
									}
									curdev = curdev->Next;
								}
								return ( current_device );
							}
						}
					}
					break;
			}
			
		}

		current_device	=	current_device->Next;
	}
    
    // None found.
    return ( NULL );
}





#ifdef TARGET_DC


// VMU file handling library.




BOOL CALLBACK FlashEnumProc(LPCMAPLEDEVICEINSTANCE pmdi, LPVOID pvContext)
{
    DWORD dwPort = pmdi->dwPort;
	DIDriverManager *pthis = (DIDriverManager *)pvContext;


	bool bAdded = FALSE;
	DIDeviceInfo *current_device = pthis->DeviceList;
	while(current_device)
	{
		if ( current_device->PortNumber == dwPort )
		{
			// Add this device to the controller.
			MapleVMU *pVMU = MFnew<MapleVMU>();
			ASSERT ( pVMU != NULL );

			pVMU->type = pmdi->devType;
			pVMU->iEnumNumber = pmdi->dwDevNum - 1;
			pVMU->guid = pmdi->guidDevice;
			pVMU->pNextVMU = current_device->pFirstVMU;

			DWORD dwControllerDirection = DIPROP_EXTRACT_EXPDIRECTION ( current_device->dwDirection, pmdi->dwDevNum );
			if ( ( ( pmdi->dwDirection == MAPLEDEV_EXPDIR_TOP ) && ( dwControllerDirection == DIPROP_EXPDIR_TOP ) ) ||
				 ( ( pmdi->dwDirection == MAPLEDEV_EXPDIR_BOTTOM ) && ( dwControllerDirection == DIPROP_EXPDIR_BOTTOM ) ) )
			{
				// Directions match, so the VMU is upside down.
				pVMU->bUpsideDown = TRUE;
			}
			else
			{
				// Directions don't match, so they're not.
				pVMU->bUpsideDown = TRUE;
			}

			current_device->pFirstVMU = pVMU;

			m_iNumMapleDevices++;

			bAdded = TRUE;
			break;
		}
		current_device = current_device->Next;
	}

	ASSERT ( bAdded );

    return(TRUE);
}





// Scan for VMUs, and attach any you find to the respective controllers.
// Returns the number of VMUs found.
int DIDriverManager::ScanForVMUs()
{
	// Clean out the Maple devices.
	m_iNumMapleDevices = 0;
	DIDeviceInfo *current_device = DeviceList;
	while(current_device)
	{
		while ( current_device->pFirstVMU != NULL )
		{
			MapleVMU *pVMU = current_device->pFirstVMU;
			current_device->pFirstVMU = pVMU->pNextVMU;

			MFdelete(pVMU);
		}
		current_device = current_device->Next;
	}

	MapleEnumerateDevices ( MDT_ALLDEVICES, FlashEnumProc, (void *)this, 0 );

	return ( m_iNumMapleDevices );

}






// Call these to make sure you've grabbed the interface pointer.
void MapleVMU::EnsureDevicePtr ( void )
{
	if ( pUnknown != NULL )
	{
		// Already done.
		return;
	}

	HRESULT hres;
	switch ( type )
	{
	case MDT_STORAGE    :
		hres = MapleCreateDevice ( &guid, (IUnknown **)&pFlash );
		break;
	case MDT_LCD        :
		hres = MapleCreateDevice ( &guid, (IUnknown **)&pLcd );
		break;
	case MDT_TIMER      :
		hres = MapleCreateDevice ( &guid, (IUnknown **)&pTimer );
		break;
	case MDT_VIBRATION  :
		hres = MapleCreateDevice ( &guid, (IUnknown **)&pVib );
		break;
	case MDT_CONTROLLER :
	case MDT_AUDIO_IN   :
	case MDT_LIGHTGUN   :
	default:
		ASSERT ( FALSE );
		pUnknown = NULL;
		return;
		break;
	}

	if ( FAILED ( hres ) )
	{
		// Nads.
		ASSERT ( FALSE );
		pUnknown = NULL;
	}

}

// Write this standard 48x32 bitmap to the LCD.
// Data format is 6x32 bytes, like you'd expect, except that the
// VMU is upside down & back to front of course!
// If bQueue is TRUE, this always works.
// If bQueue is FALSE, if there is a problem, like it's busy,
//	then it can fail, and the return is FALSE;
// If the LCD screen is not a standard type, then it does its best,
//	or fails and returns FALSE.

bool MapleVMU::Lcd_WriteScreen ( void *pvData, bool bQueue )
{
	ASSERT ( type == MDT_LCD );
	EnsureDevicePtr();

	if ( !pLcd->IsStandardLcd() )
	{
		// Bleuch. No.
		return FALSE;
	}

	HRESULT hres;
	if ( pLcdBuffer == NULL )
	{
		ASSERT ( dwLcdBufferId == 0 );
		hres = pLcd->GetLcdBuffer ( &pLcdBuffer, &dwLcdBufferId, 192 );
		if ( FAILED ( hres ) )
		{
			return FALSE;
		}
	}
	else
	{
		ASSERT ( dwLcdBufferId != 0 );
	}

	memcpy ( pLcdBuffer, pvData, 192 );

	hres = pLcd->SendLcdBuffer ( dwLcdBufferId, 0, 0, 0, NULL );
	if ( FAILED ( hres ) )
	{
		return FALSE;
	}

	return TRUE;
}


// Same, but with a VMU_Screen argument.
bool MapleVMU::Lcd_WriteScreen ( VMU_Screen *pvmuScreen, bool bQueue )
{
	ASSERT ( type == MDT_LCD );
	EnsureDevicePtr();

	ASSERT ( pvmuScreen != NULL );
	RotateVMUScreen ( bUpsideDown, pvmuScreen);

	return Lcd_WriteScreen ( (void *)(pvmuScreen->bData), bQueue );
}


// Checks to see if the VMU has been formatted.
bool Flash_CheckVMUOK ( MapleVMU *pthis )
{
	ASSERT ( pthis->type == MDT_STORAGE );
	if ( ( pthis == NULL ) || ( pthis->pFlash == NULL ) )
	{
		return FALSE;
	}
	HRESULT hres = pthis->pFlash->CheckFormat();
	if ( FAILED ( hres ) )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


// Max number of entries.
#define MAX_DIRECTORY_ENTRIES 50
// Lenght of longest name
#define MAX_DIRECTORY_LENGTH MAX_FLASH_FILE_NAME

int m_iNumDirectoryFiles;
char m_szDir[MAX_DIRECTORY_ENTRIES+1][MAX_DIRECTORY_LENGTH+1];
char *m_pszDirectory[MAX_DIRECTORY_ENTRIES+1];


// The directory callback.
BOOL Flash_GetDirectoryCallback ( LPFLASHDEVICE pIFlashDevice, FSFILEID fsfileid, LPCFSFILEDESC lpcfsfiledesc, void *pvContext )
{
	// When called by FastEnumFlashFiles, only the filename is valid in lpcfsfiledesc.
	if ( m_iNumDirectoryFiles < MAX_DIRECTORY_ENTRIES )
	{
		if ( strlen ( lpcfsfiledesc->szFileName ) < MAX_DIRECTORY_LENGTH - 1 )
		{
			// Add this filename to the list.
			strncpy ( m_szDir[m_iNumDirectoryFiles], lpcfsfiledesc->szFileName, MAX_DIRECTORY_LENGTH );
			m_pszDirectory[m_iNumDirectoryFiles] = m_szDir[m_iNumDirectoryFiles];
			m_iNumDirectoryFiles++;
		}
		else
		{
			// Long filename - sod it, coz we're only interested in ours anyway, which are all 8.3 format.
			ASSERT ( FALSE );
		}

		// Always terminate with an empty string.
		m_szDir[m_iNumDirectoryFiles][0] = '\0';
		m_pszDirectory[m_iNumDirectoryFiles] = NULL;
	}

	return TRUE;
}



// Get a directory of the flash device. Return is an array of strings.
// YOU DO NOT OWN THIS ARRAY. If you want to use it or store it,
// copy it. The array may change/move next time you do any Maple call.
// A return value of NULL indicates an error.
char **MapleVMU::Flash_GetDirectory ( void )
{
	ASSERT ( type == MDT_STORAGE );
	EnsureDevicePtr();
	if ( !Flash_CheckVMUOK ( this ) )
	{
		// Don't auto-format.
		return NULL;
	}

	m_iNumDirectoryFiles = 0;

	HRESULT hres = pFlash->FastEnumFlashFiles ( Flash_GetDirectoryCallback, NULL );
	if ( FAILED ( hres ) )
	{
		return NULL;
	}

	return m_pszDirectory;
}


// Returns the number of free blocks in this VMU.
// If there is an error, -1 is returned.
int MapleVMU::Flash_GetFreeBlocks ( void )
{
	ASSERT ( type == MDT_STORAGE );
	EnsureDevicePtr();
	if ( !Flash_CheckVMUOK ( this ) )
	{
		// Don't auto-format.
		return -1;
	}

	FSDEVICEDESC fsdevdesc;
	fsdevdesc.dwSize = sizeof ( fsdevdesc );
	fsdevdesc.dwFlags = FSDD_FREE_BLOCKS;
	HRESULT hres = pFlash->GetDeviceDesc ( &fsdevdesc );
	if ( FAILED ( hres ) )
	{
		return -1;
	}

	return (signed int)( fsdevdesc.dwFreeBlocks );

}


// Opens the given flash file.
// NULL if it failed.
LPFLASHFILE OpenVMUFile ( MapleVMU *pthis, char *pName )
{
	LPFLASHFILE pFile;

	if ( ( pthis == NULL ) || ( pthis->pFlash == NULL ) )
	{
		return NULL;
	}
	HRESULT hres = pthis->pFlash->OpenFlashFileByName ( &pFile, pName );
	if ( FAILED ( hres ) )
	{
		return NULL;
	}
	else
	{
		return pFile;
	}
}



// Get the size of the given file. If it doesn't exist, the result is -1.
DWORD MapleVMU::Flash_GetFileSize ( char *pcFilename )
{
	ASSERT ( type == MDT_STORAGE );
	EnsureDevicePtr();
	if ( !Flash_CheckVMUOK ( this ) )
	{
		// Don't auto-format.
		return -1;
	}

	LPFLASHFILE pFile = OpenVMUFile ( this, pcFilename );
	if ( pFile == NULL )
	{
		return -1;
	}
	else
	{
		DWORD dwSize;
		FSFILEDESC desc;

		ZeroMemory ( &desc, sizeof ( desc ) );
		desc.dwSize = sizeof ( desc );
		desc.dwFlags = FSFD_TOTAL_BYTES;

		HRESULT hres = pFile->GetFileDesc ( &desc );
		if ( FAILED ( hres ) )
		{
			dwSize = -1;
		}
		else
		{
			dwSize = desc.dwTotalBytes;
		}

		pFile->Release();

		return dwSize;
	}
}

// Reads the given file into pvData, which is of size dwSizeOfData.
// Return is TRUE on success, FALSE on failure.
bool MapleVMU::Flash_ReadFile ( char *pcFilename, void *pvData, DWORD dwSizeOfData )
{
	ASSERT ( type == MDT_STORAGE );
	EnsureDevicePtr();
	if ( !Flash_CheckVMUOK ( this ) )
	{
		// Don't auto-format.
		return FALSE;
	}

	// Reading past the end of the file is an error, so first
	// we need to find the size of the file.
	DWORD dwFileSize = Flash_GetFileSize ( pcFilename );
	if ( dwFileSize == -1 )
	{
		// File doesn't exist or something.
		return FALSE;
	}

	// Don't read past the end.
	if ( dwSizeOfData > dwFileSize )
	{
		dwSizeOfData = dwFileSize;
	}

	LPFLASHFILE pFile = OpenVMUFile ( this, pcFilename );
	if ( pFile == NULL )
	{
		// This should never happen if we checked the size with GetFileSize.
		//ASSERT ( FALSE );
		return FALSE;
	}
	else
	{
		HRESULT hres = pFile->Read ( 0, dwSizeOfData, (BYTE *)pvData );
		if ( FAILED ( hres ) )
		{
			pFile->Release();
			return FALSE;
		}
		else
		{
			pFile->Release();
			return TRUE;
		}
	}
}

// Creates the given file & writes the given data to it.
// pcGameName is the game name you wish to be tagged onto the file. Must be less than 16 chars.
// pcComment is any comment you wish to be tagged onto the file. Must be less than 16 chars.
// If the file already exists, it is deleted.
// If there is not enough space on the device, the call will fail.
// Return is TRUE on success, FALSE on failure.
bool MapleVMU::Flash_WriteFile ( char *pcFilename, char *pcGameName, char *pcComment, void *pvData, DWORD dwSizeOfData,
								 char *pcIconPalette, char *pcIconData )
{
	ASSERT ( type == MDT_STORAGE );
	EnsureDevicePtr();
	if ( !Flash_CheckVMUOK ( this ) )
	{
		// Don't auto-format.
		return FALSE;
	}

	if ( strlen ( pcFilename ) > ( MAX_FLASH_FILE_NAME - 1 ) )
	{
		// Too long.
		ASSERT ( FALSE );
		return FALSE;
	}

	FSFILEDESC desc;
	ZeroMemory ( &desc, sizeof ( desc ) );
	desc.dwSize = sizeof ( desc );
	desc.dwFlags = FSFD_CREATE_FILE;

	//FSFD_CREATE_FILE equals these flags:
	//FSFD_BYTES_REQUIRED
	//FSFD_FILE_NAME
	//FSFD_VMS_COMMENT
	//FSFD_BOOT_ROM_COMMENT
	//FSFD_GAME_NAME
	//FSFD_STATUS
	//FSFD_COPY
	//FSFD_FILEICON 

	desc.dwBytesRequired = dwSizeOfData;
	strncpy ( desc.szFileName, pcFilename, MAX_FLASH_FILE_NAME-1 );
	desc.szFileName[MAX_FLASH_FILE_NAME-1] = '\0';

	if ( pcGameName != NULL )
	{
		char pcTemp[MAX_VMS_COMMENT];
		DWORD dwMaxLength = MIN ( MAX_VMS_COMMENT, MIN ( MAX_BOOT_ROM_COMMENT, MAX_GAME_NAME ) );
		dwMaxLength--;
		strncpy ( pcTemp, pcGameName, dwMaxLength );
		pcTemp[dwMaxLength] = '\0';

		// If pcComment is NULL, we use the game name anyway.
		strcpy ( desc.szVMSComment, pcTemp );
		strcpy ( desc.szBootROMComment, pcTemp );
		strcpy ( desc.szGameName, pcTemp );
	}
	else
	{
		desc.szVMSComment[0] = '\0';
		desc.szBootROMComment[0] = '\0';
		desc.szGameName[0] = '\0';
	}

	if ( pcComment != NULL )
	{
		char pcTemp[MAX_VMS_COMMENT];
		DWORD dwMaxLength = MIN ( MAX_VMS_COMMENT, MIN ( MAX_BOOT_ROM_COMMENT, MAX_GAME_NAME ) );
		dwMaxLength--;
		strncpy ( pcTemp, pcComment, dwMaxLength );
		pcTemp[dwMaxLength] = '\0';

		strcpy ( desc.szVMSComment, pcTemp );
	}

	desc.bStatus = FS_STATUS_DATA_FILE;
	desc.bCopy = FS_COPY_ENABLED;

	desc.fsfileicon.bAnimationFrames = 1;
	desc.fsfileicon.bAnimationDelay = 1;
	if ( pcIconPalette != NULL )
	{
		ASSERT ( pcIconData != NULL );
		memcpy ( desc.fsfileicon.palette, pcIconPalette, 16*2 );
		memcpy ( desc.fsfileicon.pixelsFrame1, pcIconData, 32*16 );
	}
	else
	{
		ASSERT ( pcIconData == NULL );
		memset ( desc.fsfileicon.palette, 0, 16*2 );
		memset ( desc.fsfileicon.pixelsFrame1, 0, 32*16 );
	}


	LPFLASHFILE pFile = NULL;

	HRESULT hres = pFlash->CreateFlashFile ( &pFile, &desc );
	if ( FAILED ( hres ) )
	{
		// May already exist - try deleting it.
		pFile = OpenVMUFile ( this, pcFilename );
		if ( pFile == NULL )
		{
			// No, doesn't exist - just a general failure.
			return FALSE;
		}
		else
		{
			// Does exist - bin it.
			hres = pFile->Delete();
			pFile->Release();
			if ( FAILED ( hres ) )
			{
				// Double nads - file exists but can't be deleted????
				return FALSE;
			}
			else
			{
				// Good. Now create it again.
				HRESULT hres = pFlash->CreateFlashFile ( &pFile, &desc );
				if ( FAILED ( hres ) )
				{
					// Nope - maybe we're out of space or something.
					return FALSE;
				}
			}
		}
	}

	// If we got here, the file was crated successfully.
	ASSERT ( pFile != NULL );
	hres = pFile->Write ( 0, dwSizeOfData, (BYTE *)pvData );
	if ( FAILED ( hres ) )
	{
		pFile->Release();
		return FALSE;
	}

	// And flush the controller.
	hres = pFile->Flush();
	if ( FAILED ( hres ) )
	{
		pFile->Release();
		return FALSE;
	}

	pFile->Release();
	return TRUE;

}



// Allows or disallows the automatic update of VMU screens.
// This can be quite slow, and for time-critical things (e.g.
// video playbacl), you'll want to turn it off. Do turn it back on
// afterwards though.
void SetVMUScreenUpdateEnable ( bool bEnable )
{
	the_input_manager.bVMUScreenUpdatesEnabled = bEnable;
}



// Gets the first VMU on the primary controller. If it can't find it,
// it takes the first VMU on the first controller it finds.
MapleVMU *FindFirstVMUOnCurrentController ( void )
{
	if ( primary_device != NULL )
	{
		MapleVMU *pVMU = primary_device->pFirstVMU;
		while ( pVMU != NULL )
		{
			if ( pVMU->type == MDT_STORAGE )
			{
				return pVMU;
			}
			pVMU = pVMU->pNextVMU;
		}
	}

	// Not found on primary, or no primary.
	DIDeviceInfo *curdev = the_input_manager.DeviceList;
	while ( curdev != NULL )
	{
		MapleVMU *pVMU = curdev->pFirstVMU;
		while ( pVMU != NULL )
		{
			if ( pVMU->type == MDT_STORAGE )
			{
				return pVMU;
			}
			pVMU = pVMU->pNextVMU;
		}
		curdev = curdev->Next;
	}

	// No VMUs at all.
	return NULL;

}


// Tries to find a memory VMU at slot iVMUNum on controller iCtrlNum.
// If not, returns NULL. Controllers are numbered 0-3, VMU numbers 0-1.
MapleVMU *FindMemoryVMUAt ( int iCtrlNum, int iVMUNum )
{
	// Not found on primary, or no primary.
	DIDeviceInfo *curdev = the_input_manager.DeviceList;
	while ( curdev != NULL )
	{
		if ( curdev->PortNumber == iCtrlNum )
		{
			MapleVMU *pVMU = curdev->pFirstVMU;
			while ( pVMU != NULL )
			{
				if ( pVMU->type == MDT_STORAGE )
				{
					if ( pVMU->iEnumNumber == iVMUNum )
					{
						return pVMU;
					}
				}
				pVMU = pVMU->pNextVMU;
			}
		}
		curdev = curdev->Next;
	}

	// Not found.
	return ( NULL );
}



// A routine to rotate a VMU screen.
// This not just a vertical flip, it needs to do a horizontal flip as well.
// This converts the VMU screen to the desired orientation,
// so it may not actually do anything!

static BYTE bFlipNibble[16] = 
{
	0x0,	// 0000 -> 0000
	0x8,	// 0001 -> 1000
	0x4,	// 0010 -> 0100
	0xc,	// 0011 -> 1100
	0x2,	// 0100 -> 0010
	0xa,	// 0101 -> 1010
	0x6,	// 0110 -> 0110
	0xe,	// 0111 -> 1110
	0x1,	// 1000 -> 0001
	0x9,	// 1001 -> 1001
	0x5,	// 1010 -> 0101
	0xd,	// 1011 -> 1101
	0x3,	// 1100 -> 0011
	0xb,	// 1101 -> 1011
	0x7,	// 1110 -> 0111
	0xf,	// 1111 -> 1111
};

void RotateVMUScreen ( bool bRotated, VMU_Screen *pvmuScreen )
{
	if ( bRotated != pvmuScreen->bRotated )
	{
		pvmuScreen->bRotated = bRotated;

		// OK, they're really both sources and both dests. Sue me.
		BYTE *pbSrc = &(pvmuScreen->bData[0]);
		BYTE *pbDst = &(pvmuScreen->bData[32*6-1]);
		for ( int i = 0; i < 32*3; i++ )
		{
			// Reverse the bits
			BYTE bData1 = *pbSrc;
			BYTE bData2 = *pbDst;

			bData1 = ( bFlipNibble[bData1&0xf] << 4 ) | bFlipNibble[bData1>>4];
			bData2 = ( bFlipNibble[bData2&0xf] << 4 ) | bFlipNibble[bData2>>4];

			// And swap them over.
			*pbSrc = bData2;
			*pbDst = bData1;

			pbSrc++;
			pbDst--;
		}
	}
}


// A routine for converting a TGA to a VMU screen bitmap.
bool CreateVMUScreenFromTGA ( char *pchName, VMU_Screen **ppvmuScreen )
{
	TGA_Pixel pPixelData[32*48];

	ASSERT ( *ppvmuScreen == NULL );

	// Load the savegame icon from disk.
extern TGA_Info TGA_load_from_file(const CBYTE *file, SLONG max_width, SLONG max_height, TGA_Pixel* data, BOOL bCanShrink);
	TGA_Info tga_info = TGA_load_from_file ( pchName, 48, 32, pPixelData, FALSE );
	if ( tga_info.valid )
	{
		// Create the screen.
		*ppvmuScreen = MFnew<VMU_Screen>();
		ASSERT ( *ppvmuScreen != NULL );

		// Convert to black and while.
		UBYTE *pbDst = (**ppvmuScreen).bData;
		TGA_Pixel *pcSrc = pPixelData;
		for ( int iY = 0; iY < 32; iY++ )
		{
			for ( int iX1 = 0; iX1 < 6; iX1++ )
			{
				UBYTE bThisByte = 0;
				for ( int iX2 = 0; iX2 < 8; iX2++ )
				{
					bThisByte <<= 1;
					// Just take the red channel!
					if ( pcSrc->red < 128 )
					{
						bThisByte |= 1;
					}
					pcSrc++;
				}
				*pbDst++ = bThisByte;
			}
		}

		(*ppvmuScreen)->bRotated = FALSE;

		return TRUE;
	}
	else
	{
		ASSERT ( FALSE );
		return FALSE;
	}
}



// Displays the given screen on all the screen devices on the current controller.
// This is what you call to display an image.
// If there is no primary, it doesn't do anything - the "press any key" screen should be up.
bool WriteLCDScreenToCurrentController ( VMU_Screen *pvmuScreen )
{
	if ( primary_device == NULL )
	{
		return FALSE;
	}

	MapleVMU *pVMU = primary_device->pFirstVMU;
	while ( pVMU != NULL )
	{
		if ( pVMU->type == MDT_LCD )
		{
			pVMU->Lcd_WriteScreen ( pvmuScreen, TRUE );
		}
		pVMU = pVMU->pNextVMU;
	}

	return TRUE;
}



GUID m_guidCurrentVMU = GUID_NULL;

// Returns the current VMU. If it can't be found any more, and
// bFindNextBest is TRUE, it tries to find the first one on the
// primary, and then tries to find the first one on anything.
// If bFindNextBest is FALSE, it just returns NULL.
MapleVMU *FindCurrentStorageVMU ( bool bFindNextBest )
{
	if ( !IsEqualGUID ( m_guidCurrentVMU, GUID_NULL ) )
	{
		// Look for the current one.
		DIDeviceInfo *curdev = the_input_manager.DeviceList;
		while ( curdev != NULL )
		{
			MapleVMU *pVMU = curdev->pFirstVMU;
			while ( pVMU != NULL )
			{
				if ( pVMU->type == MDT_STORAGE )
				{
					if ( pVMU->guid == m_guidCurrentVMU  )
					{
						return pVMU;
					}
				}
				pVMU = pVMU->pNextVMU;
			}
			curdev = curdev->Next;
		}
	}

	// Not found, or no current.
	if ( bFindNextBest )
	{
		// Find the first on the primary.
		if ( primary_device != NULL )
		{
			MapleVMU *pVMU = primary_device->pFirstVMU;
			while ( pVMU != NULL )
			{
				if ( pVMU->type == MDT_STORAGE )
				{
					return pVMU;
				}
				pVMU = pVMU->pNextVMU;
			}
		}

		// OK, well try all the other devices.
		DIDeviceInfo *curdev = the_input_manager.DeviceList;
		while ( curdev != NULL )
		{
			MapleVMU *pVMU = curdev->pFirstVMU;
			while ( pVMU != NULL )
			{
				if ( pVMU->type == MDT_STORAGE )
				{
					return pVMU;
				}
				pVMU = pVMU->pNextVMU;
			}
			curdev = curdev->Next;
		}
	}

	return NULL;
}


// Sets the current storage VMU. If NULL, there will be no current VMU.
void SetCurrentStorageVMU ( MapleVMU *pVMU )
{
	if ( pVMU == NULL )
	{
		m_guidCurrentVMU = GUID_NULL;
	}
	else
	{
		ASSERT ( pVMU->type == MDT_STORAGE );
		m_guidCurrentVMU = pVMU->guid;
	}
}





// Tries to find a vibration VMU on the primary. If there are two (unlikely),
// it retruns the first one. If there are none, it returns NULL.
// Yes, I know they're not actually VMUs that vibrate.
MapleVMU *FindFirstVibratorOnCurrentController ( void )
{
	if ( primary_device != NULL )
	{
		MapleVMU *pVMU = primary_device->pFirstVMU;
		while ( pVMU != NULL )
		{
			if ( pVMU->type == MDT_VIBRATION )
			{
				// Found one!
				return pVMU;
			}
			pVMU = pVMU->pNextVMU;
		}
	}

	return NULL;
}



bool m_bVibrationsEnabled = TRUE;

void SetVibrationEnable ( bool bEnabled )
{
	m_bVibrationsEnabled = bEnabled;
}


// Make the vibrator in the primary device vibrate with the given
// characteristics. Returns TRUE if it works, or FALSE if not.
// The most common cause of FALSE is that another vibration
// is already happening, or was set off very recently.

static HANDLE m_hVibrationEvent = NULL;

bool Vibrate ( float fFrequency, float fStartPower, float fShrinkTime, bool bEnsureThisHappens )
{
	if ( !m_bVibrationsEnabled )
	{
		// Shan't.
		return FALSE;
	}

	MapleVMU *pVib = FindFirstVibratorOnCurrentController();
	if ( pVib != NULL )
	{
		ASSERT ( pVib->type == MDT_VIBRATION );
		pVib->EnsureDevicePtr();

		// The Fishing rod doesn't like me creating a vibration device, so at least make sure it doesn't crash.
		// Can alos happen if you rip the pack out at just the wrong time.
		if ( pVib->pVib == NULL )
		{
			return FALSE;
		}

		if ( !pVib->Vib_bGotDevInfo )
		{
			// Get the device info.
			UINT iTemp1 = 1;
			UINT iTemp2;
			VIB_INFO vibInfo;
			HRESULT hres = pVib->pVib->GetVibInfo ( &iTemp1, &iTemp2, &vibInfo );
			ASSERT ( SUCCEEDED ( hres ) );

			pVib->Vib_fMinFreq = ( vibInfo.minFrequency + 1 ) * (float)VIB_FREQ_INCREMENT_HZ;
			pVib->Vib_fMaxFreq = ( vibInfo.maxFrequency + 1 ) * (float)VIB_FREQ_INCREMENT_HZ;

			pVib->Vib_bGotDevInfo = TRUE;
		}

		if ( m_hVibrationEvent == NULL )
		{
			// Set up the event.
		    m_hVibrationEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		}

		VIB_SETTINGS vibset;

		if ( fStartPower > 1.0f )
		{
			fStartPower = 1.0f;
		}
		else if ( fStartPower < 0.0f )
		{
			// Ok then, I won't.
			return FALSE;
		}

		if ( fFrequency > pVib->Vib_fMaxFreq )
		{
			fFrequency = pVib->Vib_fMaxFreq;
		}
		else if ( fFrequency < pVib->Vib_fMinFreq )
		{
			if ( fFrequency < 0.0f )
			{
				// Ok then, I won't.
				return FALSE;
			}
			else
			{
				fFrequency = pVib->Vib_fMinFreq;
			}
		}


		vibset.sourceId = 1;
		vibset.bContinuousVib = FALSE;
		vibset.initialPower = (BYTE)( fStartPower * (float)VIB_POWER_MAX );
		vibset.direction = VIB_DIRECTION_FORWARD;
		vibset.frequency = (BYTE)( fFrequency * ( 1.0f / (float)VIB_FREQ_INCREMENT_HZ ) - 1.0f + 0.4999f );

		if ( fShrinkTime > 0.0f )
		{
			// Shrinking with time, e.g. explosion.
			vibset.vibType = VIB_TYPE_CONVERGENT;
			// This says how many waves go before the power goes down by one.
			vibset.wavesPerStep = (BYTE)( ( ( fShrinkTime * fFrequency ) / ( fStartPower * (float)VIB_POWER_MAX ) ) + 0.5f );
			if ( vibset.wavesPerStep == 0 )
			{
				// 0 makes it not work.
				vibset.wavesPerStep = 1;
			}

			TRACE ( "Vibrated with freq %f, power %f, shrinktime %f\n", fFrequency, fStartPower, fShrinkTime );
		}
		else
		{
			// One-shot, e.g. gunshots.
			vibset.vibType = VIB_TYPE_CONSTANT;
			vibset.wavesPerStep = 0;

			TRACE ( "Vibrated with freq %f, power %f, one-shot.\n", fFrequency, fStartPower, fShrinkTime );
		}


		// We have a manual reset event that was created in the signaled state.
		// So, the first time through here, the Wait will return immediately.
		// We pass that event to Vibrate.  Because the parameter is not NULL,
		// Vibrate will return quickly with VIBERR_PENDING.  We then go off 
		// and do other things.  When the command to the device finishes, 
		// the Api will signal our event.  So, the next time we get here 
		// that Wait will return immediately again.  If, however, we tried to
		// call the api again very quickly (before it finished the last call)
		// we'd wait here until the previous call was finished.  The calls
		// shouldn't take more than 2 frames to complete, and it's unlikely
		// that we'll tell the device to vibrate twice in that time frame (a
		// human won't be able to feel changes that quick), so this is okay.

		// Waiting two frames sounds pretty bad though, and may happen if,
		// for example, two people fire guns at the same time. Well, if it happens,
		// I'll bin the waiting one.
 

		// First make sure the previous send is finished.
		DWORD dwWaitReturn;
		if ( bEnsureThisHappens )
		{
			// Use quite a long timeout - this is an important vibration
			// (e.g. a large, long-lasting one, such as an explosion).
			dwWaitReturn = WaitForSingleObject ( m_hVibrationEvent, 100 );
		}
		else
		{
			dwWaitReturn = WaitForSingleObject ( m_hVibrationEvent, 0 );
		}

		if ( dwWaitReturn == WAIT_TIMEOUT )
		{
			// Nads - not going to hang around any longer.
			// There was already a vibro command being sent, so tough -
			// this one gets ditched.
			return FALSE;
		}
    
		ResetEvent ( m_hVibrationEvent );

		// Start the vibration.
		HRESULT hres = pVib->pVib->Vibrate ( 1, &vibset, m_hVibrationEvent );
		if ( hres == VIBERR_DEVICEUNPLUGGED )
		{
			// Since there was an error, our event was never signaled.
			SetEvent ( m_hVibrationEvent );
			return FALSE;
		}
		else if ( ( hres != VIB_OK ) && ( hres != VIBERR_PENDING ) )
		{
			// Since there was an error, our event was never signaled.
			SetEvent ( m_hVibrationEvent );
			return FALSE;
		}

		return TRUE;

	}

	return FALSE;
}




#endif //#ifdef TARGET_DC



//---------------------------------------------------------------


#else	// The new PC implementation....




// ========================================================
//
// JOYSTICK STUFF
//
// ========================================================

IDirectInput        *OS_joy_direct_input;
IDirectInputDevice  *OS_joy_input_device;
IDirectInputDevice2 *OS_joy_input_device2;	// We need this newer interface to poll the joystick.

SLONG OS_joy_x_range_min;
SLONG OS_joy_x_range_max;
SLONG OS_joy_y_range_min;
SLONG OS_joy_y_range_max;

//
// The callback function for enumerating joysticks.
//

BOOL CALLBACK OS_joy_enum(
		LPCDIDEVICEINSTANCE instance, 
        LPVOID              context )
{
    HRESULT             hr;
    LPDIRECTINPUTDEVICE pDevice;

	//
    // Get an interface to the joystick.
	//

    hr = OS_joy_direct_input->CreateDevice(
								instance->guidInstance,
							   &OS_joy_input_device,
							    NULL);

    if (FAILED(hr))
	{
		//
		// Cant use this joystick for some reason!
		//

		OS_joy_input_device  = NULL;
		OS_joy_input_device2 = NULL;

        return DIENUM_CONTINUE;
	}

	//
    // Query for the IDirectInputDevice2 interface.
	// We need this to poll the joystick.
	//

    OS_joy_input_device->QueryInterface(
							IID_IDirectInputDevice2, 
							(LPVOID *) &OS_joy_input_device2);

	//
	// No need to find another joystick!
	//

    return DIENUM_STOP;
}

//
// Initialises the joystick.
//

void OS_joy_init(void)
{
	HRESULT hr;

	//
	// Initialise everything.
	//

	OS_joy_direct_input  = NULL;
	OS_joy_input_device  = NULL;
	OS_joy_input_device2 = NULL;

	//
	// Create the direct input object.
	//

	CoInitialize(NULL);

#ifdef	MAD_AM_I

	CoCreateInstance(
	    CLSID_DirectInput8,
		NULL,
		CLSCTX_INPROC_SERVER,
	    IID_IDirectInput8W,
	    (void **) &OS_joy_direct_input);

	extern HINSTANCE hGlobalThisInst;
	
	hr = OS_joy_direct_input->Initialize(hGlobalThisInst, DIRECTINPUT_VERSION);
#else
			return;
#endif
	
	if (hr != DI_OK)
	{
		if (hr == DIERR_BETADIRECTINPUTVERSION)
		{
			return;
		}

		if (hr == DIERR_OLDDIRECTINPUTVERSION)
		{
			return;
		}

		return;
	}

	/*

    hr = DirectInputCreateEx(
			OS_this_instance,
			DIRECTINPUT_VERSION,
		   &OS_joy_direct_input,
			NULL);

    if (FAILED(hr)) 
	{
		//
		// No direct input!
		//

        return;
	}

	*/

	//
	// Find a joystick.
	//
	
    hr = OS_joy_direct_input->EnumDevices(
								DIDEVTYPE_JOYSTICK,
								OS_joy_enum,
								NULL,
								DIEDFL_ATTACHEDONLY);

	if (OS_joy_input_device  == NULL ||
		OS_joy_input_device2 == NULL)
	{
		//
		// The joystick wasn't properly found.
		// 

		OS_joy_input_device  = NULL;
		OS_joy_input_device2 = NULL;

		return;
	}

	//
	// So we can get the nice 'n' simple joystick data format.
	//

    OS_joy_input_device->SetDataFormat(&c_dfDIJoystick);

	//
	// Grab the joystick exclusively when our window in the foreground.
	//

    OS_joy_input_device->SetCooperativeLevel(
							hDDLibWindow,
							DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	//
	// What is the range of the joystick?
	//

	DIPROPRANGE diprg; 

	//
	// In x...
	// 

    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.diph.dwObj        = DIJOFS_X;
    diprg.lMin              = 0;
    diprg.lMax              = 0;

	OS_joy_input_device->GetProperty(
								DIPROP_RANGE,
							   &diprg.diph);

	OS_joy_x_range_min = diprg.lMin;
	OS_joy_x_range_max = diprg.lMax;

	//
	// In y...
	// 

    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.diph.dwObj        = DIJOFS_Y;
    diprg.lMin              = 0;
    diprg.lMax              = 0;

	OS_joy_input_device->GetProperty(
								DIPROP_RANGE,
							   &diprg.diph);

	OS_joy_y_range_min = diprg.lMin;
	OS_joy_y_range_max = diprg.lMax;

}

//
// Polls the joystick.
//

SLONG OS_joy_poll(void)
{
	HRESULT hr;

	if (OS_joy_direct_input  == NULL ||
		OS_joy_input_device  == NULL ||
		OS_joy_input_device2 == NULL)
	{
		//
		// No joystick detected.
		//

		memset(&the_state, 0, sizeof(the_state));

		return FALSE;
	}

	SLONG acquired_already = FALSE;

  try_again_after_acquiring:;

	{
		//
		// We acquired the joystick okay.  Poll the joystick to
		// update its state.
		//

		OS_joy_input_device2->Poll();

		//
		// Finally get the state of the joystick.
		//

		hr = OS_joy_input_device->GetDeviceState(sizeof(the_state), &the_state);

		if (hr == DIERR_NOTACQUIRED ||
			hr == DIERR_INPUTLOST)
		{
			if (acquired_already)
			{
				//
				// Oh dear!
				//

				memset(&the_state, 0, sizeof(the_state));

				return FALSE;
			}
			else
			{
				hr = OS_joy_input_device->Acquire();

				if (hr == DI_OK)
				{
					acquired_already = TRUE;

					goto try_again_after_acquiring;
				}
				else
				{
					memset(&the_state, 0, sizeof(the_state));

					return FALSE;
				}
			}
		}
	}

	return TRUE;
}



BOOL GetInputDevice(UBYTE type, UBYTE sub_type, bool bActuallyGetOne)
{
	if (type == JOYSTICK && bActuallyGetOne)
	{
		if (!OS_joy_direct_input)
		{
			OS_joy_init();
		}
	}

	if (OS_joy_input_device &&
		OS_joy_input_device2)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL ReadInputDevice()
{
	return OS_joy_poll();
}



#endif











