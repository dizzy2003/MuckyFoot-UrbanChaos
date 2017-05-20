#define _OS_CPP_

#include <windows.h>
#include <windowsx.h>
#include <zmouse.h>

#include <ddraw.h>
#include <d3d.h>
#include <d3dtypes.h>
#include <dinput.h>
#include <dsound.h>

#include <stdarg.h>
#include <string.h>

#include "d3denum.h"		// Direct 3D sample library
#include "d3dframe.h"

#include "resource.h"

#include "always.h"
#include "clip.h"
#include "key.h"
#include "os.h"
#include "tga.h"
#include "matrix.h"
#include "wave.h"

#include <mmreg.h>
#include <msacm.h>

//
// The entrypoint into the actual game.
//

extern void MAIN_main(void);

HINSTANCE OS_this_instance;
HINSTANCE OS_last_instance;
LPSTR	  OS_command_line;
int       OS_start_show_state;

CBYTE    *OS_application_name = "muckyBASIC Virtual Machine";

//
// Our window class.
//

WNDCLASSEX OS_wcl;

//
// Our window handle and menu handle.
//

HWND  OS_window_handle;

//
// The DirectX 6 framework library class.
//

UBYTE OS_frame_is_fullscreen;
UBYTE OS_frame_is_hardware;

CD3DFramework OS_frame;


//
// The number of poly's drawn last frame.
//

SLONG OS_poly_count_transformed;
SLONG OS_poly_count_drawn;


// ========================================================
//
// THE SCREEN
//
// ========================================================

float OS_screen_width;
float OS_screen_height;


// ========================================================
//
// KEY HANDLING STUFF
//
// ========================================================

//
// The keys that are held down.
//

volatile UBYTE KEY_on[256];
volatile SLONG KEY_inkey;
volatile SLONG KEY_shift;


// ========================================================
//
// JOYSTICK STUFF
//
// ========================================================

IDirectInput        *OS_joy_direct_input;
IDirectInputDevice  *OS_joy_input_device;
IDirectInputDevice2 *OS_joy_input_device2;	// We need this newer interface to poll the joystick.

float OS_joy_x;
float OS_joy_y;
SLONG OS_joy_x_range_min;
SLONG OS_joy_x_range_max;
SLONG OS_joy_y_range_min;
SLONG OS_joy_y_range_max;
ULONG OS_joy_button;		// The buttons that are currently down
ULONG OS_joy_button_down;	// The buttons that have just been pressed
ULONG OS_joy_button_up;		// The buttons that have just been released


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

	CoCreateInstance(
	    CLSID_DirectInput8,
		NULL,
		CLSCTX_INPROC_SERVER,
	    IID_IDirectInput8W,
	    (void **) &OS_joy_direct_input);

	hr = OS_joy_direct_input->Initialize(OS_this_instance, DIRECTINPUT_VERSION);
	
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
								DI8DEVTYPE_JOYSTICK,
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
							OS_window_handle,
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

void OS_joy_poll(void)
{
	HRESULT hr;

	if (OS_joy_direct_input  == NULL ||
		OS_joy_input_device  == NULL ||
		OS_joy_input_device2 == NULL)
	{
		//
		// No joystick detected.
		//

		OS_joy_x           = 0.0F;
		OS_joy_y           = 0.0F;
		OS_joy_button      = 0;
		OS_joy_button_down = 0;
		OS_joy_button_up   = 0;

		return;
	}

	SLONG acquired_already = FALSE;

  try_again_after_acquiring:;

	{
		DIJOYSTATE js;

		//
		// We acquired the joystick okay.  Poll the joystick to
		// update its state.
		//

		OS_joy_input_device2->Poll();

		//
		// Finally get the state of the joystick.
		//

		hr = OS_joy_input_device->GetDeviceState(sizeof(js), &js);

		if (hr == DIERR_NOTACQUIRED ||
			hr == DIERR_INPUTLOST)
		{
			if (acquired_already)
			{
				//
				// Oh dear!
				//

				OS_joy_x           = 0.0F;
				OS_joy_y           = 0.0F;
				OS_joy_button      = 0;
				OS_joy_button_down = 0;
				OS_joy_button_up   = 0;

				return;
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
					OS_joy_x           = 0.0F;
					OS_joy_y           = 0.0F;
					OS_joy_button      = 0;
					OS_joy_button_down = 0;
					OS_joy_button_up   = 0;

					return;
				}
			}
		}

		if (!FAILED(hr))
		{
			//
			// Axis movment normalised to between -1.0F and +1.0F
			//

			SLONG dx = OS_joy_x_range_max - OS_joy_x_range_min;
			SLONG dy = OS_joy_y_range_max - OS_joy_y_range_min;

			OS_joy_x = 0.0F;
			OS_joy_y = 0.0F;

			if (dx) {OS_joy_x = float(js.lX - OS_joy_x_range_min) * 2.0F / float(dx) - 1.0F;}
			if (dy) {OS_joy_y = float(js.lY - OS_joy_y_range_min) * 2.0F / float(dy) - 1.0F;}

			//
			// The buttons.
			//

			SLONG i;

			ULONG last = OS_joy_button;
			ULONG now  = 0;

			for (i = 0; i < 32; i++)
			{
				if (js.rgbButtons[i] & 0x80)
				{
					now |= 1 << i;
				}
			}

			OS_joy_button      = now;
			OS_joy_button_down = now  & ~last;
			OS_joy_button_up   = last & ~now;
		}
	}
}


// ========================================================
//
// WAVEFORM CONVERSION
//
// ========================================================

//
// Returns the converted memory, allocated with malloc().
//

void OS_decompress_sound(
		WAVEFORMATEX *source_format,
		void         *source_data,
		SLONG         source_num_bytes,
		WAVEFORMATEX *dest_format,
		void        **dest_data,
		ULONG        *dest_num_bytes)
{
	//
	// Open the conversion stream.
	//

	HACMSTREAM has;

	switch(acmStreamOpen(
		   &has,
			NULL,
		    source_format,
		    dest_format,
			NULL,
			NULL,
			0,
			ACM_STREAMOPENF_NONREALTIME))
	{
		case ACMERR_NOTPOSSIBLE:   TRACE("The requested operation cannot be performed.\n"); break; 
		case MMSYSERR_INVALFLAG:   TRACE("At least one flag is invalid. 			  \n"); break;
		case MMSYSERR_INVALHANDLE: TRACE("The specified handle is invalid. 			  \n"); break;
		case MMSYSERR_INVALPARAM:  TRACE("At least one parameter is invalid. 		  \n"); break;
		case MMSYSERR_NOMEM:       TRACE("The system is unable to allocate resources. \n"); break;
	}

	//
	// Work out how many bytes we need.
	//

	if (acmStreamSize(
			has,
			source_num_bytes,
			dest_num_bytes,
			ACM_STREAMSIZEF_SOURCE))
	{
		return;
	}

	//
	// Allocate the memory.
	//

   *dest_data = (void *) malloc(*dest_num_bytes);

	if (!*dest_data)
	{
		return;
	}

	memset(*dest_data, 0, *dest_num_bytes);

	//
	// Prepare the source and destination buffers.
	//
	
	ACMSTREAMHEADER ash;

	memset(&ash, 0, sizeof(ash));

	ash.cbStruct    = sizeof(ash);
	ash.pbSrc       = (LPBYTE) source_data;
	ash.cbSrcLength = source_num_bytes;
	ash.pbDst       = (LPBYTE) *dest_data;
	ash.cbDstLength = *dest_num_bytes;

	if (acmStreamPrepareHeader(
			has,
		   &ash,
			0))
	{
		return;
	}

	//
	// Do the conversion.
	//
	
	if (acmStreamConvert(
			has,
		   &ash,
			0))
	{
		return;
	}

	//
	// Unprepare the header and close the stream.
	//

	if (acmStreamUnprepareHeader(
			has,
		   &ash,
			0))
	{
		return;
	}

   *dest_num_bytes = ash.cbDstLengthUsed;	

	if (acmStreamClose(has, 0))
	{
		return;
	} 
}





// ========================================================
//
// SOUND STUFF
//
// ========================================================

//
// Dsound globals.
//

SLONG                   OS_sound_valid;
LPDIRECTSOUND           OS_sound_dsound;
LPDIRECTSOUNDBUFFER     OS_sound_primary;
LPDIRECTSOUND3DLISTENER OS_sound_listener;
SLONG                   OS_sound_changed;	// TRUE => we need to commit deferred settings.


//
// Each dsound sound.
//

typedef struct os_sound
{
	SLONG                 type;
	CBYTE                 fname[256];
	LPDIRECTSOUNDBUFFER   buffer;
	LPDIRECTSOUND3DBUFFER buffer3d;

} OS_Sound;

#define OS_MAX_SOUNDS 256

OS_Sound OS_sound[OS_MAX_SOUNDS];


void OS_sound_init(void)
{
	//
	// Create direct sound.
	//
 
    if (DirectSoundCreate(NULL, &OS_sound_dsound, NULL) != DS_OK)
	{
        return;
	}
 
	//
    // Set a co-op level.
	//
 
	if (OS_sound_dsound->SetCooperativeLevel(OS_window_handle, DSSCL_PRIORITY) != DS_OK)
	{
		return;
	}
 
	//
    // Get our primary buffer.
	//

	DSBUFFERDESC dsbd;

	memset(&dsbd, 0, sizeof(dsbd));

	dsbd.dwSize  = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
	
	if (OS_sound_dsound->CreateSoundBuffer(&dsbd, &OS_sound_primary, NULL) != DS_OK)
	{
		return;
	}
 
	//
	// Setup the format to 16-bit 22khz.
	//
 
    WAVEFORMATEX wfx;

    memset(&wfx, 0, sizeof(wfx));

    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels	    = 2;
    wfx.nSamplesPerSec  = 22050;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
 
	if (OS_sound_primary->SetFormat(&wfx) != DS_OK)
	{
		return;
	}

	//
	// Create the listener for 3D sounds.
	//

    if (OS_sound_primary->QueryInterface(IID_IDirectSound3DListener, (void ** ) &OS_sound_listener) != DS_OK)
	{
		return;
	}

	//
	// Set listener defaults...
	//

	OS_sound_listener->SetDopplerFactor(2.0F, DS3D_DEFERRED);
	OS_sound_listener->SetRolloffFactor(2.0F, DS3D_DEFERRED);

	OS_sound_changed = TRUE;

	//
	// Make sure that the primary buffer is always active. That way we don't
	// get nasty clicks when we go in or out of silence.
	//

	OS_sound_primary->Play(0, 0, DSBPLAY_LOOPING);

	//
	// DSound is initialised ok.
	//

	OS_sound_valid = TRUE;
}


OS_Sound *OS_sound_create(CBYTE *fname, SLONG type)
{
	SLONG i;

	unsigned int bytes_read;
	
	OS_Sound *os;

	WAVEFORMATEX *pwfx;           // Wave format info
	HMMIO         hmmio;          // File handle
	MMCKINFO      mmckinfoData;   // Chunk info
	MMCKINFO      mmckinfoParent; // Parent chunk info

	if (!OS_sound_valid)
	{
		return NULL;
	}

	//
	// Do we already have this sound?
	//

	for (i = 0; i < OS_MAX_SOUNDS; i++)
	{
		os = &OS_sound[i];

		if (stricmp(fname, os->fname) == 0)
		{
			//
			// Return this sound!
			// 

			return os;
		}
	}


	//
	// Get a new sample structure.
	//

	for (i = 0; i < OS_MAX_SOUNDS; i++)
	{
		os = &OS_sound[i];

		if (os->type == OS_SOUND_TYPE_UNUSED)
		{
			goto found_spare_sound;
		}
	}

	//
	// No more sounds available.
	//

	return NULL;

  found_spare_sound:;

	strcpy(os->fname, fname);
	os->type = type;

	//
	// Open the wave file.
	//

	if (WaveOpenFile(fname, &hmmio, &pwfx, &mmckinfoParent) != 0)
	{
		return NULL;
	}

	if (WaveStartDataRead(&hmmio, &mmckinfoData, &mmckinfoParent) != 0)
	{
	    WaveCloseReadFile(&hmmio, &pwfx);

		return NULL;
	}

	//
	// If the format is not PCM, then convert to PCM format.
	//

	if (pwfx->wFormatTag != WAVE_FORMAT_PCM)
	{
		SLONG num_source_bytes = mmckinfoData.cksize;

		//
		// Read the data into a temporary buffer.
		//

		void *src = (void *) malloc(mmckinfoData.cksize);

		if (WaveReadFile(
				hmmio,              // file handle
				mmckinfoData.cksize,// no. of bytes to read
				(UBYTE *) src,      // destination
				&mmckinfoData,      // file chunk info 
				&bytes_read))       // actual no. of bytes read
		{
			WaveCloseReadFile(&hmmio, &pwfx);

			return NULL;
		}

		//
		// Convert!
		//

		WAVEFORMATEX destwfx;

		destwfx.wFormatTag      = WAVE_FORMAT_PCM;
		destwfx.wBitsPerSample  = 16;
		destwfx.nChannels       = pwfx->nChannels;
		destwfx.nSamplesPerSec  = pwfx->nSamplesPerSec;
		destwfx.nBlockAlign     = destwfx.wBitsPerSample / 8 * destwfx.nChannels;
	    destwfx.nAvgBytesPerSec = destwfx.nSamplesPerSec * destwfx.nBlockAlign;
		destwfx.cbSize          = 0;

		void *dest_data = NULL;
		ULONG dest_num_bytes;

		OS_decompress_sound(
		    pwfx,
			src,
			num_source_bytes,
		   &destwfx,
		   &dest_data,
		   &dest_num_bytes);
		
		if (!dest_data)
		{
			return NULL;
		}

		//
		// Create the sound buffer with the destination format.
		//

		DSBUFFERDESC dsbdesc;

		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 

		dsbdesc.dwSize        =  sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags       =  DSBCAPS_LOCSOFTWARE;
		dsbdesc.dwBufferBytes =  dest_num_bytes;
		dsbdesc.lpwfxFormat   = &destwfx;

		switch(type)
		{
			case OS_SOUND_TYPE_2D:
				dsbdesc.dwFlags |= DSBCAPS_CTRLVOLUME;
				break;

			case OS_SOUND_TYPE_3D:
				dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
				dsbdesc.dwFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;
				break;

			default:
				ASSERT(0);
				break;
		}

		if (OS_sound_dsound->CreateSoundBuffer(&dsbdesc, &os->buffer, NULL) != DS_OK)
		{
			WaveCloseReadFile(&hmmio, &pwfx);

			return NULL;
		}

		//
		// Lock the sound buffer.
		//

		void *data;
		ULONG num_bytes;

		if (os->buffer->Lock(
				0,              // Offset of lock start
				0,              // Size of lock; ignored in this case
				&data,			// Address of lock start
				&num_bytes,     // Number of bytes locked
				NULL,           // Wraparound start; not used
				NULL,           // Wraparound size; not used
				DSBLOCK_ENTIREBUFFER) != DS_OK)
		{
			WaveCloseReadFile(&hmmio, &pwfx);

			return NULL;
		}
		
		ASSERT(num_bytes == dest_num_bytes);

		//
		// Copy over the converted data.
		//

		memcpy(data, dest_data, dest_num_bytes);

		//
		// Free up memory.
		//

		free(src);
		free(dest_data);

		//
		// Unlock the buffer and close the wave file.
		//

		os->buffer->Unlock(data, num_bytes, NULL, 0);

		WaveCloseReadFile(&hmmio, &pwfx);

	}
	else
	{
		//
		// Create a sound buffer- now that we know the format of the wave.
		//

		DSBUFFERDESC dsbdesc;

		memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 

		dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags       = DSBCAPS_STATIC;
		dsbdesc.dwBufferBytes = mmckinfoData.cksize;
		dsbdesc.lpwfxFormat   = pwfx;

		switch(type)
		{
			case OS_SOUND_TYPE_2D:
				dsbdesc.dwFlags |= DSBCAPS_CTRLVOLUME;
				break;

			case OS_SOUND_TYPE_3D:
				dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
				dsbdesc.dwFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;
				break;

			default:
				ASSERT(0);
				break;
		}

		if (OS_sound_dsound->CreateSoundBuffer(&dsbdesc, &os->buffer, NULL) != DS_OK)
		{
			WaveCloseReadFile(&hmmio, &pwfx);

			return NULL;
		}

		//
		// Lock the sound buffer.
		//

		void *data;
		ULONG num_bytes;

		if (os->buffer->Lock(
				0,              // Offset of lock start
				0,              // Size of lock; ignored in this case
				&data,			// Address of lock start
				&num_bytes,     // Number of bytes locked
				NULL,           // Wraparound start; not used
				NULL,           // Wraparound size; not used
				DSBLOCK_ENTIREBUFFER) != DS_OK)
		{
			WaveCloseReadFile(&hmmio, &pwfx);

			return NULL;
		}
		
		//
		// Read in the sound data.
		//

		if (WaveReadFile(
				hmmio,              // file handle
				num_bytes,          // no. of bytes to read
				(UBYTE *) data,     // destination
				&mmckinfoData,      // file chunk info 
				&bytes_read))       // actual no. of bytes read
		{
			WaveCloseReadFile(&hmmio, &pwfx);

			return NULL;
		}

		//
		// Unlock the buffer and close the wave file.
		//

		os->buffer->Unlock(data, num_bytes, NULL, 0);

		WaveCloseReadFile(&hmmio, &pwfx);
	}

	//
	// If this buffer is 3D, then get the 3D interface.
	//
	
	os->buffer3d = NULL;

	if (os->type == OS_SOUND_TYPE_3D)
	{
		if (FAILED(os->buffer->QueryInterface(IID_IDirectSound3DBuffer, (void **) &os->buffer3d)))
		{
			return NULL;
		}

		os->buffer3d->SetMinDistance(4.0F, DS3D_DEFERRED);
	}

	return os;
}

OS_Sound *OS_sound_create(UWORD *data, SLONG num_samples, SLONG type)
{
	SLONG i;

	OS_Sound *os;

	if (!OS_sound_valid)
	{
		return NULL;
	}

	//
	// Get a new sample structure.
	//

	for (i = 0; i < OS_MAX_SOUNDS; i++)
	{
		os = &OS_sound[i];

		if (os->type == OS_SOUND_TYPE_UNUSED)
		{
			goto found_spare_sound;
		}
	}

	//
	// No more sounds available.
	//

	return NULL;

  found_spare_sound:;

	strcpy(os->fname, "Generated");

	os->type = OS_SOUND_TYPE_2D;

	//
	// Create a sound buffer.
	//

	DSBUFFERDESC dsbdesc;
	WAVEFORMATEX pwfx;           // Wave format info

	//
	// Setup the wave format
	//

	memset(&pwfx, 0, sizeof(pwfx));

	pwfx.cbSize          = sizeof(pwfx);
	pwfx.wFormatTag      = WAVE_FORMAT_PCM;
	pwfx.nChannels       = 1;
	pwfx.nSamplesPerSec  = 22050;
	pwfx.nAvgBytesPerSec = 22050 * sizeof(UWORD);
	pwfx.nBlockAlign     = 2;
	pwfx.wBitsPerSample  = 16;

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 

	dsbdesc.dwSize        =  sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags       =  DSBCAPS_STATIC;
	dsbdesc.dwBufferBytes =  num_samples * sizeof(UWORD);
	dsbdesc.lpwfxFormat   = &pwfx;

	switch(type)
	{
		case OS_SOUND_TYPE_2D:
			dsbdesc.dwFlags |= DSBCAPS_CTRLVOLUME;
			break;

		case OS_SOUND_TYPE_3D:
			dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
			dsbdesc.dwFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;
			break;

		default:
			ASSERT(0);
			break;
	}

	if (OS_sound_dsound->CreateSoundBuffer(&dsbdesc, &os->buffer, NULL) != DS_OK)
	{
		return NULL;
	}

	//
	// Lock the sound buffer.
	//

	void *locked_data;
	ULONG locked_bytes;

	if (os->buffer->Lock(
			0,              // Offset of lock start
			0,              // Size of lock; ignored in this case
			&locked_data,	// Address of lock start
			&locked_bytes,  // Number of bytes locked
			NULL,           // Wraparound start; not used
			NULL,           // Wraparound size; not used
			DSBLOCK_ENTIREBUFFER) != DS_OK)
	{
		return NULL;
	}
	
	//
	// Copy over the sound data.
	//

	memcpy(locked_data, data, locked_bytes);

	//
	// Unlock the buffer and close the wave file.
	//

	os->buffer->Unlock(locked_data, locked_bytes, NULL, 0);

	//
	// If this buffer is 3D, then get the 3D interface.
	//
	
	os->buffer3d = NULL;

	if (os->type == OS_SOUND_TYPE_3D)
	{
		if (FAILED(os->buffer->QueryInterface(IID_IDirectSound3DBuffer, (void **) &os->buffer3d)))
		{
			return NULL;
		}

		os->buffer3d->SetMinDistance(4.0F, DS3D_DEFERRED);
	}

	return os;
}



void OS_sound_play(OS_Sound *os, SLONG flag)
{
	if (!OS_sound_valid || os == NULL || os->buffer == NULL)
	{
		return;
	}
	
	//
	// What's this buffer doing now?
	//

	ULONG status;

	os->buffer->GetStatus(&status);

	if (status == DSBSTATUS_PLAYING ||
		status == DSBSTATUS_LOOPING)
	{
		if (flag & OS_SOUND_FLAG_INTERRUPT)
		{
			os->buffer->SetCurrentPosition(0);
		}
	}
	else
	{
		os->buffer->Play(0, 0, (flag & OS_SOUND_FLAG_LOOP) ? DSBPLAY_LOOPING : 0);
	}
}


void OS_sound_stop(OS_Sound *os)
{
	if (!OS_sound_valid || os == NULL || os->buffer == NULL)
	{
		return;
	}

	os->buffer->Stop();
}

void OS_sound_finish()
{
}

void OS_sound_2d_set_volume(OS_Sound *os, float volume)
{
	if (!OS_sound_valid || os == NULL || os->buffer == NULL)
	{
		return;
	}

	volume = sqrt(volume);
	volume = sqrt(volume);
	volume = sqrt(volume);
	volume = sqrt(volume);

	SLONG long_volume = SLONG(DSBVOLUME_MIN + volume * (DSBVOLUME_MAX - DSBVOLUME_MIN));

	os->buffer->SetVolume(long_volume);
}


void OS_sound_3d_set_range(OS_Sound *os, float min, float max)
{
	if (!OS_sound_valid || os == NULL || os->buffer == NULL)
	{
		return;
	}

	os->buffer3d->SetMinDistance(min, DS3D_DEFERRED);
	os->buffer3d->SetMaxDistance(max, DS3D_DEFERRED);

	OS_sound_changed = TRUE;
}

void OS_sound_3d_set_position(
		OS_Sound *os,
		float x,
		float y,
		float z,
		float dx,
		float dy,
		float dz)
{
	if (!OS_sound_valid || os == NULL || os->buffer == NULL)
	{
		return;
	}

	os->buffer3d->SetPosition( x, y, z, DS3D_DEFERRED);
	os->buffer3d->SetVelocity(dx,dy,dz, DS3D_DEFERRED);

	OS_sound_changed = TRUE;
}



void OS_sound_listener_set(
		float x,
		float y,
		float z,
		float dx,
		float dy,
		float dz,
		float yaw,
		float pitch,
		float roll)
{
	float matrix[9];

	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	OS_sound_listener->SetPosition( x,  y,  z, DS3D_DEFERRED);
	OS_sound_listener->SetVelocity(dx, dy, dz, DS3D_DEFERRED);

	OS_sound_listener->SetOrientation(
		matrix[6],
		matrix[7],
		matrix[8],
		matrix[3],
		matrix[4],
		matrix[5],
		DS3D_DEFERRED);

	OS_sound_changed = TRUE;
}



void OS_sound_update_changes()
{
	if (OS_sound_changed)
	{
		OS_sound_listener->CommitDeferredSettings();

		OS_sound_changed = FALSE;
	}
}





// ========================================================
//
// TEXTURE STUFF 
//
// ========================================================

//
// The directories where we load textures from.
//

#define OS_MAX_TEXPATHS 256

CBYTE OS_texpath[OS_MAX_TEXPATHS][64];
SLONG OS_texpath_upto;
SLONG OS_texpath_last;	// The last texture path we used successfully.


//
// All the textures available and the path we found them in.
//

typedef struct
{
	SLONG path;		// -1 => Not used.
	CBYTE fname[28];

} OS_Texname;

#define OS_MAX_TEXNAMES 1024

OS_Texname OS_texname[OS_MAX_TEXNAMES];
SLONG      OS_texname_upto;



//
// The pixel formats for each of our OS_TEXTURE_FORMATs
// 

typedef struct
{
	SLONG valid;

	DDPIXELFORMAT ddpf;

	SLONG mask_r;
	SLONG mask_g;
	SLONG mask_b;
	SLONG mask_a;

	SLONG shift_r;
	SLONG shift_g;
	SLONG shift_b;
	SLONG shift_a;

} OS_Tformat;

OS_Tformat OS_tformat[OS_TEXTURE_FORMAT_NUMBER];


//
// Our texture pages.
//

typedef struct os_texture
{
	CBYTE name[_MAX_PATH];
	UBYTE format;
	UBYTE inverted;
	UWORD padding;
	UWORD width;
	UWORD height;

	DDSURFACEDESC2       ddsd;
	LPDIRECTDRAWSURFACE4 ddsurface;
	LPDIRECT3DTEXTURE2   ddtx;

	OS_Texture *next;

} OS_Texture;

//
// They are stored in a linked list and dynamically allocated.
// 

OS_Texture *OS_texture_head;



//
// Fills in the texture path info from the given file.
//

#define OS_BASE_TEXTURE_PATH "Textures\\"

void OS_init_texpaths()
{
	SLONG i;

	WIN32_FIND_DATA fd;
	HANDLE          handle;
	CBYTE           search_path[256];
	OS_Texname     *otn;
	CBYTE          *ch;

	//
	// We always have "Textures\\" in our path.
	//

	strcpy(OS_texpath[0], OS_BASE_TEXTURE_PATH);

	OS_texpath_upto = 1;

	//
	// Use any additional directories inside the base directory.
	//

	for (i = 0; i < OS_texpath_upto; i++)
	{
		sprintf(search_path, "%s*.*", OS_texpath[i]);

		handle = FindFirstFile(
					search_path,
				   &fd);

		if (handle != INVALID_HANDLE_VALUE)
		{
			while(1)
			{
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					//
					// Found another directory.
					//

					if (fd.cFileName[0] == '.')
					{
						//
						// Ignore the crappy "." and ".." directories
						//
					}
					else
					{
						ASSERT(WITHIN(OS_texpath_upto, 0, OS_MAX_TEXPATHS - 1));

						sprintf(OS_texpath[OS_texpath_upto], "%s%s\\", OS_texpath[i], fd.cFileName);

						OS_texpath_upto += 1;
					}
				}

				if (!FindNextFile(handle, &fd))
				{
					//
					// Nothing more in the "Textures\\" directory.
					//

					FindClose(handle);

					break;
				}
			}
		}
	}

	//
	// Find all the texture filenames.
	//

	for (i = 0; i < OS_texpath_upto; i++)
	{
		sprintf(search_path, "%s\\*.tga", OS_texpath[i]);

		handle = FindFirstFile(search_path, &fd);

		if (handle != INVALID_HANDLE_VALUE)
		{
			while(1)
			{
				ASSERT(WITHIN(OS_texname_upto, 0, OS_MAX_TEXNAMES - 1));

				otn = &OS_texname[OS_texname_upto++];

				otn->path = i;

				//
				// Copy over just the filename.
				//

				strncpy(otn->fname, fd.cFileName, 27);

				if (!FindNextFile(handle, &fd))
				{
					//
					// Nothing more in this path.
					//

					FindClose(handle);

					break;
				}
			}
		}
	}
}



//
// Returns the number of bits set in 'mask' with a rather cunning algorithm.
//

SLONG OS_bit_count(ULONG mask)
{
	SLONG ans;

	for (ans = 0; mask; mask &= mask - 1, ans += 1);

	return ans;
}


//
// The texture enumeration function.
//

HRESULT CALLBACK OS_texture_enumerate_pixel_formats(
					LPDDPIXELFORMAT lpddpf,
					LPVOID          context)
{
	SLONG format;

	OS_Tformat *otf = (OS_Tformat *) malloc(sizeof(OS_Tformat));

	if (otf == NULL)
	{
		//
		// Oh dear!
		//

		return D3DENUMRET_CANCEL;
	}

	//
	// Is this one of the formats we are interested in?
	//

	if (lpddpf->dwFlags & DDPF_RGB)
	{
		//
		// We are only interested in 16-bpp RGB modes.
		//

		if (lpddpf->dwRGBBitCount == 16)
		{
			if (lpddpf->dwFlags & DDPF_ALPHAPIXELS)
			{
				SLONG alphabits;

				//
				// Could be 1555 or 4444
				//

				alphabits = OS_bit_count(lpddpf->dwRGBAlphaBitMask);

				if (alphabits == 1)
				{
					//
					// Must be 1555
					//

					OS_tformat[OS_TEXTURE_FORMAT_1555].valid =  TRUE;
					OS_tformat[OS_TEXTURE_FORMAT_1555].ddpf  = *lpddpf;
				}
				else
				if (alphabits == 4)
				{
					//
					// Must be 4444
					//

					OS_tformat[OS_TEXTURE_FORMAT_4444].valid =  TRUE;
					OS_tformat[OS_TEXTURE_FORMAT_4444].ddpf  = *lpddpf;
				}
			}
			else
			{
				//
				// This is a good RGB pixel format.
				//

				OS_tformat[OS_TEXTURE_FORMAT_RGB].valid =  TRUE;
				OS_tformat[OS_TEXTURE_FORMAT_RGB].ddpf  = *lpddpf;
			}
		}
	}
	else
	if (lpddpf->dwFlags & DDPF_LUMINANCE)
	{
		if (lpddpf->dwFlags & DDPF_ALPHAPIXELS)
		{
			//
			// We only want luminance- not luminance and alpha.
			//
		}
		else
		{
			if (lpddpf->dwLuminanceBitCount == 8)
			{
				//
				// This is what we want. An 8-bit luminance format.
				//

				OS_tformat[OS_TEXTURE_FORMAT_8].valid =  TRUE;
				OS_tformat[OS_TEXTURE_FORMAT_8].ddpf  = *lpddpf;
			}
		}
	}

	//
	// Ask for another texture format.
	//

	return D3DENUMRET_OK;
}


//
// Given the bitmask for a colour in a pixel format, it calculates the mask and
// shift so that you can construct a pixel in pixel format given its RGB values.
// The formula is...
//
//	PIXEL(r,g,b) = ((r >> mask) << shift) | ((g >> mask) << shift) | ((b >> mask) << shift);
// 
// THIS ASSUMES that r,g,b are 8-bit values.
//

void OS_calculate_mask_and_shift(
		ULONG  bitmask,
		SLONG *mask,
		SLONG *shift)
{
	SLONG i;
	SLONG b;
	SLONG num_bits  =  0;
	SLONG first_bit = -1;

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

	ASSERT(first_bit != -1 && num_bits != 0);

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


CBYTE *OS_texture_full_path(CBYTE *fname)
{
	SLONG i;

	static CBYTE fullpath[MAX_PATH];


	//sprintf(fullpath, "Textures\\qmark.tga");

	//return fullpath;

	for (i = 0; i < OS_texname_upto; i++)
	{
		if (_stricmp(OS_texname[i].fname, fname) == 0)
		{
			//
			// Found the texture!
			//

			sprintf(fullpath, "%s\\%s", OS_texpath[OS_texname[i].path], fname);

			return fullpath;
		}
	}

	//
	// No such texture.
	//

	return NULL;
}

//
// The buffer we use to load TGAs into.
//

TGA_Pixel OS_texture_tga[OS_TEXTURE_MAX_SIZE * OS_TEXTURE_MAX_SIZE];

OS_Texture *OS_texture_create(CBYTE *fname, SLONG invert)
{
	SLONG       format;
	OS_Texture *ot;
	OS_Tformat *best_otf;
	TGA_Info    ti;
	CBYTE      *fullpath;

	//
	// Do we already have this texture?
	//

	for (ot = OS_texture_head; ot; ot = ot->next)
	{
		if (strcmp(fname, ot->name) == 0)
		{
			if (ot->inverted == invert)
			{
				return ot;
			}
		}
	}

	//
	// Find the filename.
	//

	fullpath = OS_texture_full_path(fname);

	if (!fullpath)
	{
		OS_error("Could not find the texture file: \"%s\"\nanywhere inside the Textures directory.\nUsing texture \"qmark.tga\" instead.", fname);

		fullpath = OS_BASE_TEXTURE_PATH"qmark.tga";
	}

	//
	// Try to load in the TGA.
	//

	ti = TGA_load(fullpath, OS_TEXTURE_MAX_SIZE, OS_TEXTURE_MAX_SIZE, OS_texture_tga);

	if (!ti.valid                    ||
		OS_bit_count(ti.width)  != 1 ||
		OS_bit_count(ti.height) != 1)
	{
		if (!ti.valid)
		{
			OS_error("Error loading tga file \"%s\".\n", fname);
		}
		else
		{
			OS_error("Cannot load \"%s\"\nBoth the width and height of a texture must be a power of 2.\ni.e. 64,128,256,512,...\nUsing qmark.tga instead.", fname);
		}

		fullpath = OS_BASE_TEXTURE_PATH"qmark.tga";

		ti = TGA_load(fullpath, OS_TEXTURE_MAX_SIZE, OS_TEXTURE_MAX_SIZE, OS_texture_tga);

		if (!ti.valid                    ||
			OS_bit_count(ti.width)  != 1 ||
			OS_bit_count(ti.height) != 1)
		{
			//
			// Error loading qmark.tga!
			//

			return NULL;
		}
	}

	//
	// Find the best texture format.
	//

	if (ti.flag & TGA_FLAG_CONTAINS_ALPHA)
	{
		if (ti.flag & TGA_FLAG_ONE_BIT_ALPHA)
		{
			format = OS_TEXTURE_FORMAT_1555;
		}
		else
		{
			format = OS_TEXTURE_FORMAT_4444;
		}
	}
	else
	if (ti.flag & TGA_FLAG_GRAYSCALE)
	{
		if (OS_tformat[OS_TEXTURE_FORMAT_8].valid)
		{
			//
			// This card has a luminance only texture format.
			//

			format = OS_TEXTURE_FORMAT_8;
		}
		else
		{
			//
			// Use the RGB format as the next-best thing.
			//

			format = OS_TEXTURE_FORMAT_RGB;
		}
	}
	else
	{
		//
		// A normal RGB texture
		//

		format = OS_TEXTURE_FORMAT_RGB;
	}

	best_otf = &OS_tformat[format];

	if (!best_otf->valid)
	{
		//
		// No good texture format.
		//

		return NULL;
	}

	//
	// Create a new texture.
	//

	ot = (OS_Texture *) malloc(sizeof(OS_Texture));
	
	if (ot == NULL)
	{
		//
		// It's really not worth checking for this... but anyway!
		//

		return NULL;
	}	

	strncpy(ot->name, fname, _MAX_PATH);

	ot->format   = format;
	ot->inverted = invert;

	//
	// Create a managed texture surface.
	//

	memset(&ot->ddsd, 0, sizeof(ot->ddsd));

	ot->ddsd.dwSize  = sizeof(DDSURFACEDESC2);
	ot->ddsd.dwFlags =
				DDSD_CAPS        |
				DDSD_HEIGHT      |
				DDSD_WIDTH       |
				DDSD_MIPMAPCOUNT |
				DDSD_PIXELFORMAT;
	ot->ddsd.dwWidth         = ti.width;
	ot->ddsd.dwHeight        = ti.height;
	ot->ddsd.dwMipMapCount   = 1;
	ot->ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	ot->ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_HINTSTATIC;
	ot->ddsd.ddpfPixelFormat = best_otf->ddpf;

	VERIFY(OS_frame.GetDirectDraw()->CreateSurface(
										&ot->ddsd,
										&ot->ddsurface,
										NULL) == DD_OK);

	if (invert)
	{
		SLONG i;
		SLONG j;

		TGA_Pixel *tp;

		//
		// Invert the texture.
		//

		tp = OS_texture_tga;

		for (i = 0; i < ti.width; i++)
		{
			for (j = 0; j < ti.height; j++)
			{
				tp->alpha = 255 - tp->alpha;
				tp->red   = 255 - tp->red;
				tp->green = 255 - tp->green;
				tp->blue  = 255 - tp->blue;

				tp += 1;
			}
		}
	}

	//
	// Lock the surface.
	//

	DDSURFACEDESC2 ddsd;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	VERIFY(ot->ddsurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL) == DD_OK);

	//
	// Copy the tga data into the surface.
	//

	if (format != OS_TEXTURE_FORMAT_8)
	{
		SLONG     i;
		SLONG     j;
		UWORD     pixel_our;
		TGA_Pixel pixel_tga;
		UWORD    *wscreen = (UWORD *) ddsd.lpSurface;

		//
		// 16 bits per pixel.
		//

		for (i = 0; i < ti.width; i++)
		{
			for (j = 0; j < ti.height; j++)
			{
				pixel_tga = OS_texture_tga[i + j * ti.width];
				pixel_our = 0;
				
				pixel_our |= (pixel_tga.red   >> best_otf->mask_r) << best_otf->shift_r;
				pixel_our |= (pixel_tga.green >> best_otf->mask_g) << best_otf->shift_g;
				pixel_our |= (pixel_tga.blue  >> best_otf->mask_b) << best_otf->shift_b;

				if (best_otf->ddpf.dwFlags & DDPF_ALPHAPIXELS)
				{
					pixel_our |= (pixel_tga.alpha >> best_otf->mask_a) << best_otf->shift_a;
				}

				wscreen[i + j * (ddsd.lPitch >> 1)] = pixel_our;
			}
		}
	}
	else
	{
		SLONG  i;
		SLONG  j;
		UBYTE *wscreen = (UBYTE *) ddsd.lpSurface;

		//
		// 8 bits per pixel.
		//

		for (i = 0; i < ti.width; i++)
		{
			for (j = 0; j < ti.height; j++)
			{
				wscreen[i + j * ddsd.lPitch] = OS_texture_tga[i + j * ti.width].red;
			}
		}
	}
	
	//
	// Unlock the surface.
	//

	ot->ddsurface->Unlock(NULL);

	//
	// Query the texture interface from the surface.
	//

	VERIFY(ot->ddsurface->QueryInterface(IID_IDirect3DTexture2, (void **) &ot->ddtx) == DD_OK);

	//
	// Insert this texture into the array.
	//

	ot->next        = OS_texture_head;
	OS_texture_head = ot;

	//
	// Remember the size!
	//

	ot->width  = ti.width;
	ot->height = ti.height;

	return ot;
}


OS_Texture *OS_texture_create(SLONG width, SLONG height, SLONG format)
{
	OS_Texture *ot;
	OS_Tformat *otf;

	//
	// Powers of two?
	//

	if (OS_bit_count(width ) > 1 ||
		OS_bit_count(height) > 1)
	{
		return NULL;
	}

	//
	// Make sure this texture is not too big.
	// 

	{
		D3DDEVICEDESC dh;
		D3DDEVICEDESC ds;

		memset(&dh, 0, sizeof(dh));
		memset(&ds, 0, sizeof(ds));

		dh.dwSize = sizeof(dh);
		ds.dwSize = sizeof(ds);

		VERIFY(OS_frame.GetD3DDevice()->GetCaps(&dh, &ds) == D3D_OK);

		if (width  > dh.dwMaxTextureWidth ||
			height > dh.dwMaxTextureHeight)
		{
			return NULL;
		}
	}

	if (!OS_tformat[format].valid)
	{
		//
		// The requested texture format does not exist. Is there
		// another one we can try?
		//

		switch(format)
		{
			case OS_TEXTURE_FORMAT_8:    format = OS_TEXTURE_FORMAT_RGB;  break;
			case OS_TEXTURE_FORMAT_1555: format = OS_TEXTURE_FORMAT_4444; break;
			case OS_TEXTURE_FORMAT_4444: format = OS_TEXTURE_FORMAT_1555; break;
		}

		if (!OS_tformat[format].valid)
		{
			//
			// We have no suitable texture format.
			//

			return NULL;
		}
	}

	//
	// The texture format we are going to use.
	//

	otf = &OS_tformat[format];

	//
	// Create a new texture.
	//

	ot = (OS_Texture *) malloc(sizeof(OS_Texture));
	
	if (ot == NULL)
	{
		//
		// It's really not worth checking for this... but anyway!
		//

		return NULL;
	}	

	sprintf(ot->name, "Generated");

	ot->format = format;
	ot->width  = width;
	ot->height = height;

	//
	// Create a managed texture surface.
	//

	memset(&ot->ddsd, 0, sizeof(ot->ddsd));

	ot->ddsd.dwSize  = sizeof(DDSURFACEDESC2);
	ot->ddsd.dwFlags =
				DDSD_CAPS        |
				DDSD_HEIGHT      |
				DDSD_WIDTH       |
				DDSD_MIPMAPCOUNT |
				DDSD_PIXELFORMAT;
	ot->ddsd.dwWidth         = width;
	ot->ddsd.dwHeight        = height;
	ot->ddsd.dwMipMapCount   = 1;
	ot->ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	ot->ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_HINTDYNAMIC;
	ot->ddsd.ddpfPixelFormat = otf->ddpf;

	if (OS_frame.GetDirectDraw()->CreateSurface(
										&ot->ddsd,
										&ot->ddsurface,
										NULL) != DD_OK)
	{
		//
		// Oh dear...
		//

		free(ot);

		return NULL;
	}

	
	//
	// The surface probably contains junk...
	//

	//
	// Query the texture interface from the surface.
	//

	VERIFY(ot->ddsurface->QueryInterface(IID_IDirect3DTexture2, (void **) &ot->ddtx) == DD_OK);

	//
	// Insert this texture into the array.
	//

	ot->next        = OS_texture_head;
	OS_texture_head = ot;

	return ot;
}



void OS_texture_finished_creating()
{
	/*

	SLONG i;

	OS_Texture *ot;
	OS_Point    op;
	UWORD       index[3];

	//
	// Go through all the textures and draw something with them.
	//

	OS_scene_begin();
	OS_init_renderstates();

	for (ot = OS_texture_head; ot; ot = ot->next)
	{
		OS_page_lock(ot);

		for (i = 0; i < 3; i++)
		{
			op.x    = frand() * OS_screen_width;
			op.y    = frand() * OS_screen_height;
			op.z    = 0.5F;
			op.rhw  = 0.5F;
			op.clip = OS_CLIP_TRANSFORMED;

			index[i] = OS_page_add_point(ot, &op, 0x00000000, 0x00000000, frand(), frand(), 0.0F);
		}

		OS_page_add_triangle(ot, index[0], index[1], index[2]);
		OS_page_unlock(ot);
		OS_page_draw(ot, OS_TEXTURE_TYPE_DOUBLESIDED | OS_TEXTURE_TYPE_ZALWAYS);
	}

	OS_scene_end();
	OS_show();

	*/
}


SLONG OS_texture_width(OS_Texture *ot)
{
	if (ot == NULL)
	{
		return 0;
	}

	return ot->width;
}

SLONG OS_texture_height(OS_Texture *ot)
{
	if (ot == NULL)
	{
		return 0;
	}

	return ot->height;
}



SLONG  OS_bitmap_format;		// OS_TEXTURE_FORMAT_*
UWORD *OS_bitmap_uword_screen;	// For 16-bit formats.
SLONG  OS_bitmap_uword_pitch;	// Pitch in UWORDS
UBYTE *OS_bitmap_ubyte_screen;	// For the grayscale format.
SLONG  OS_bitmap_ubyte_pitch;	// Pitch in UBYTES
SLONG  OS_bitmap_width;
SLONG  OS_bitmap_height;
SLONG  OS_bitmap_mask_r;
SLONG  OS_bitmap_mask_g;
SLONG  OS_bitmap_mask_b;
SLONG  OS_bitmap_mask_a;
SLONG  OS_bitmap_shift_r;
SLONG  OS_bitmap_shift_g;
SLONG  OS_bitmap_shift_b;
SLONG  OS_bitmap_shift_a;

void OS_texture_lock(OS_Texture *ot)
{
	OS_Tformat *otf;

	HRESULT res;

	DDSURFACEDESC2 ddsd;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	VERIFY((res = ot->ddsurface->Lock(
					NULL,
				   &ddsd,
					DDLOCK_WAIT      |
					DDLOCK_WRITEONLY |
					DDLOCK_NOSYSLOCK,
					NULL)) == DD_OK);

	ASSERT(WITHIN(ot->format, 0, OS_TEXTURE_FORMAT_NUMBER - 1));

	otf = &OS_tformat[ot->format];

	if (ot->format == OS_TEXTURE_FORMAT_8)
	{
		//
		// 8-bits per pixel.
		//

		OS_bitmap_ubyte_screen = (UBYTE *) ddsd.lpSurface;
		OS_bitmap_ubyte_pitch  = ddsd.lPitch;

		OS_bitmap_uword_screen = NULL;
		OS_bitmap_uword_pitch  = 0;
	}
	else
	{
		OS_bitmap_ubyte_screen = NULL;
		OS_bitmap_ubyte_pitch  = 0;

		OS_bitmap_uword_screen = (UWORD *) ddsd.lpSurface;
		OS_bitmap_uword_pitch  = ddsd.lPitch >> 1;
	}

	OS_bitmap_format  = ot->format;
	OS_bitmap_width   = ddsd.dwWidth;
	OS_bitmap_height  = ddsd.dwHeight;
	OS_bitmap_mask_r  = otf->mask_r;
	OS_bitmap_mask_g  = otf->mask_g;
	OS_bitmap_mask_b  = otf->mask_b;
	OS_bitmap_mask_a  = otf->mask_a;
	OS_bitmap_shift_r = otf->shift_r;
	OS_bitmap_shift_g = otf->shift_g;
	OS_bitmap_shift_b = otf->shift_b;
	OS_bitmap_shift_a = otf->shift_a;
}

void OS_texture_unlock(OS_Texture *ot)
{
	//
	// Unlock the surface.
	//

	ot->ddsurface->Unlock(NULL);
}


SLONG OS_texture_blit_from_backbuffer(OS_Texture *ot, SLONG x, SLONG y)
{
	RECT src_rect;

	src_rect.top  = y;
	src_rect.left = x;
	src_rect.bottom = y + ot->height;
	src_rect.right  = x + ot->width;

	if (ot->ddsurface->Blt(NULL, OS_frame.GetBackBuffer(), &src_rect, DDBLT_WAIT, NULL) == DD_OK)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




// ========================================================
//
// PIPELINE SETUP AND VALIDATION
//
// ========================================================



void OS_init_renderstates()
{
	LPDIRECT3DDEVICE3 d3d = OS_frame.GetD3DDevice();

	//
	// Setup renderstates.
	// 

	d3d->SetRenderState(D3DRENDERSTATE_SHADEMODE,          D3DSHADE_GOURAUD);
	d3d->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_DITHERENABLE,       TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_SPECULARENABLE,     TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_SUBPIXEL,           TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_ZENABLE,            TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_ZFUNC,              D3DCMP_LESSEQUAL);
	d3d->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,       TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_CULLMODE,           D3DCULL_CCW);
	d3d->SetRenderState(D3DRENDERSTATE_FOGENABLE,          FALSE);
	d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,   FALSE);
	d3d->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,    FALSE);

	if (KEY_on[KEY_A])
	{
		d3d->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
	}
	else
	{
		d3d->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
	}

	//
	// Setup pipeline for one-texture gouraud shaded.
	//

	d3d->SetTextureStageState(0, D3DTSS_COLOROP,       D3DTOP_MODULATE);
	d3d->SetTextureStageState(0, D3DTSS_COLORARG1,     D3DTA_TEXTURE);
	d3d->SetTextureStageState(0, D3DTSS_COLORARG2,     D3DTA_DIFFUSE);
    d3d->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);

	d3d->SetTextureStageState(1, D3DTSS_COLOROP,       D3DTOP_DISABLE);
	d3d->SetTextureStageState(1, D3DTSS_COLORARG1,     D3DTA_TEXTURE);
	d3d->SetTextureStageState(1, D3DTSS_COLORARG2,     D3DTA_CURRENT);
    d3d->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

	d3d->SetTextureStageState(2, D3DTSS_COLOROP,       D3DTOP_DISABLE);

	d3d->SetTextureStageState(0, D3DTSS_MINFILTER,     D3DTFG_LINEAR);
	d3d->SetTextureStageState(0, D3DTSS_MAGFILTER,     D3DTFG_LINEAR);
	d3d->SetTextureStageState(0, D3DTSS_ADDRESS,       D3DTADDRESS_WRAP);

	d3d->SetTextureStageState(1, D3DTSS_MINFILTER,     D3DTFG_LINEAR);
	d3d->SetTextureStageState(1, D3DTSS_MAGFILTER,     D3DTFG_LINEAR);
	d3d->SetTextureStageState(1, D3DTSS_ADDRESS,       D3DTADDRESS_WRAP);

	//
	// No alpha.
	//

	d3d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	d3d->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	d3d->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}



//
// Works out how to setup the pipeline for additive and multiplicitive
// multi-texturing.
// 

#define OS_METHOD_NUMBER_MUL 2

SLONG OS_pipeline_method_mul;

void OS_pipeline_calculate()
{
	ULONG num_passes;

	LPDIRECT3DDEVICE3 d3d = OS_frame.GetD3DDevice();

	OS_pipeline_method_mul = 0;

	OS_Texture *ot1 = OS_texture_create(32, 32, OS_TEXTURE_FORMAT_RGB);
	OS_Texture *ot2 = OS_texture_create(32, 32, OS_TEXTURE_FORMAT_RGB);

	while(1)
	{
		OS_init_renderstates();

		d3d->SetTexture(0, ot1->ddtx);
		d3d->SetTexture(1, ot2->ddtx);

		switch(OS_pipeline_method_mul)
		{
			case 1:

				d3d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

				d3d->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
				
				break;

			case 0:

				d3d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

				d3d->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

				d3d->SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				d3d->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
				d3d->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);

				break;
			
			default:

				//
				// Didn't find any way to do mulitexturing!
				//

				break;
		}

		if (OS_pipeline_method_mul == OS_METHOD_NUMBER_MUL)
		{
			//
			// No multitexturing! :(
			//

			break;
		}

		if (d3d->ValidateDevice(&num_passes) == D3D_OK)
		{
			if (num_passes != 0)
			{
				//
				// Found a methed for doing additive multi-texturing.
				//

				OS_string("Validated %d with %d passes\n", OS_pipeline_method_mul, num_passes);

				break;
			}
		}

		OS_pipeline_method_mul += 1;
	}

	OS_string("Multitexture method %d\n", OS_pipeline_method_mul);
}


void OS_change_renderstate_for_type(ULONG draw)
{
	LPDIRECT3DDEVICE3 d3d = OS_frame.GetD3DDevice();

	if (draw & OS_DRAW_ADD)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		d3d->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_ONE);
		d3d->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_ONE);
	}

	if (draw & OS_DRAW_MULTIPLY)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		d3d->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_DESTCOLOR);
		d3d->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_SRCCOLOR);
	}

	if (draw & OS_DRAW_MULBYONE)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		d3d->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_DESTCOLOR);
		d3d->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_ZERO);
	}

	if (draw & OS_DRAW_CLAMP)
	{
		d3d->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
	}

	if (draw & OS_DRAW_DECAL)
	{
		d3d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	if (draw & OS_DRAW_TRANSPARENT)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		d3d->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_ZERO);
		d3d->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_ONE);
	}

	if (draw & OS_DRAW_DOUBLESIDED)
	{
		d3d->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	}

	if (draw & OS_DRAW_NOZWRITE)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}

	if (draw & OS_DRAW_ALPHAREF)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ALPHAFUNC,D3DCMP_NOTEQUAL);
		d3d->SetRenderState(D3DRENDERSTATE_ALPHAREF,0);
		d3d->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,TRUE);

		//
		// Make sure the alpha from the texture gets through.
		//

		d3d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		d3d->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	}

	if (draw & OS_DRAW_ZREVERSE)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_GREATEREQUAL);
	}

	if (draw & OS_DRAW_ZALWAYS)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
	}

	if (draw & OS_DRAW_CULLREVERSE)
	{
		d3d->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
	}

	if (draw & OS_DRAW_NODITHER)
	{
		d3d->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
	}

	if (draw & OS_DRAW_ALPHABLEND)
	{
		d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		d3d->SetRenderState(D3DRENDERSTATE_SRCBLEND,         D3DBLEND_SRCALPHA);
		d3d->SetRenderState(D3DRENDERSTATE_DESTBLEND,        D3DBLEND_INVSRCALPHA);

		d3d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		d3d->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		d3d->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	}

	if (draw & OS_DRAW_TEX_NONE)
	{
		d3d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
		d3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	}

	if (draw & OS_DRAW_TEX_MUL)
	{
		switch(OS_pipeline_method_mul)
		{
			case 1:

				if (draw & OS_DRAW_DECAL)
				{
					//
					// Don't multiply by diffuse colour...
					//

					d3d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
					d3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
				}
				else
				{
					d3d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					d3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				}

				d3d->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
				
				break;

			case 0:

				d3d->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

				d3d->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				d3d->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);

				if (draw & OS_DRAW_DECAL)
				{
					//
					// Don't multiply by diffuse colour...
					//
				}
				else
				{
					d3d->SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					d3d->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
					d3d->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
				}

				break;
			
			default:
				break;
		}
	}

	if (draw & OS_DRAW_NOFILTER)
	{
		d3d->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_POINT);
		d3d->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);

		d3d->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFG_POINT);
		d3d->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_POINT);
	}
}


void OS_undo_renderstate_type_changes(void)
{
	LPDIRECT3DDEVICE3 d3d = OS_frame.GetD3DDevice();

	d3d->SetTextureStageState(0, D3DTSS_COLOROP,       D3DTOP_MODULATE);
	d3d->SetTextureStageState(0, D3DTSS_COLORARG1,     D3DTA_TEXTURE);
	d3d->SetTextureStageState(0, D3DTSS_COLORARG2,     D3DTA_DIFFUSE);

	d3d->SetTextureStageState(1, D3DTSS_COLOROP,       D3DTOP_DISABLE);
	d3d->SetTextureStageState(1, D3DTSS_COLORARG1,     D3DTA_TEXTURE);
	d3d->SetTextureStageState(1, D3DTSS_COLORARG2,     D3DTA_CURRENT);

	d3d->SetTextureStageState(2, D3DTSS_COLOROP,       D3DTOP_DISABLE);

	d3d->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);

	d3d->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	d3d->SetRenderState(D3DRENDERSTATE_CULLMODE,         D3DCULL_CCW);
	d3d->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,     TRUE);
	d3d->SetRenderState(D3DRENDERSTATE_ZFUNC,            D3DCMP_LESSEQUAL);
	d3d->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,  FALSE);
	d3d->SetRenderState(D3DRENDERSTATE_DITHERENABLE,     TRUE);

	d3d->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	d3d->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);

	d3d->SetTextureStageState(1, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	d3d->SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
}

// ========================================================
//
// WINDOWS STUFF
//
// ========================================================

void OS_string(CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[512];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	OutputDebugString(message);
}


SLONG OS_game_start_tick_count;

SLONG OS_ticks(void)
{
	return GetTickCount() - OS_game_start_tick_count;
}

void  OS_ticks_reset()
{
	OS_game_start_tick_count = GetTickCount();
}


SLONG OS_mhz;

//
// Returns TRUE if the processor has support for the RDTSC instruction.
// 

SLONG OS_has_rdtsc(void)
{
	SLONG res;

	_asm
	{
		mov		eax, 0
		cpuid
		mov		res, eax
	}

	if (res == 0)
	{
		//
		// This is an old 486!
		//

		return FALSE;
	}

	//
	// Check the processor feature info.
	//

	_asm
	{
		mov		eax, 1
		cpuid
		mov		res, edx
	}

	if (res & (1 << 4))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//
// Returns the number of processor ticks since the processor was reset / 65536
//

ULONG OS_rdtsc(void)
{
	ULONG hi;
	ULONG lo;

	_asm
	{
		rdtsc
		mov		hi, edx
		mov		lo, eax
	}

	ULONG ans;

	ans  = lo >> 16;
	ans |= hi << 16;

	return ans;
}


void OS_work_out_mhz(void)
{
	if (OS_has_rdtsc())
	{
		SLONG tick;
		ULONG tsc1 = OS_rdtsc();

		//
		// Wait a second.
		//

		tick = OS_ticks();

		while(OS_ticks() < tick + 1000);

		ULONG tsc2 = OS_rdtsc();

		float persec = float(tsc2 - tsc1);

		persec *= 65536.0F / 1000000.0F;

		OS_mhz = SLONG(persec + 0.5F);
	}
	else
	{
		//
		// It must be a 486... lets say its 66Mhz and be hopeful.
		//

		OS_mhz = 66;
	}
}

SLONG OS_processor_mhz(void)
{
	return OS_mhz;
}


void OS_error(CBYTE *fmt, ...)
{
	if (!OS_frame_is_fullscreen)
	{
		//
		// Work out the real message.
		//

		CBYTE   message[512];
		va_list	ap;

		va_start(ap, fmt);
		vsprintf(message, fmt, ap);
		va_end  (ap);

		MessageBox(
			OS_window_handle,
			message,
			"Error",
			MB_ICONERROR | MB_OK);

	}
}


SLONG OS_is_archive_bit_set(CBYTE *fname)
{
	SLONG attribs;

	attribs = GetFileAttributes(fname);

	if (attribs == -1)
	{
		//
		// Error!
		//

		return FALSE;
	}
	else
	{
		if (attribs & FILE_ATTRIBUTE_ARCHIVE)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

void OS_clear_archive_bit(CBYTE *fname)
{
	SLONG attribs;

	attribs = GetFileAttributes(fname);

	if (attribs == -1)
	{
		//
		// Error!
		//

		return;
	}
	else
	{
		attribs &= ~FILE_ATTRIBUTE_ARCHIVE;

		SetFileAttributes(fname, attribs);
	}
}




// ========================================================
//
// MOUSE STUFF
//
// ========================================================

void OS_mouse_get(SLONG *x, SLONG *y)
{
	POINT point;

	GetCursorPos(&point);
	
	if (!OS_frame_is_fullscreen)
	{
		RECT rect;

		//
		// (point.x,point.y) is in screen coordinates. We want it to
		// be relative to the client rectangle of our window.
		// 

		ScreenToClient(
			OS_window_handle,
		   &point);
	}

	*x = point.x;
	*y = point.y;
}

void OS_mouse_set(SLONG  x, SLONG  y)
{
	if (!OS_frame_is_fullscreen)
	{
		POINT point;

		point.x = x;
		point.y = y;

		//
		// (x,y) is in relative to the client rectangle of our window
		// but SetCurorPos wandt screen coordinates.
		// 

		ClientToScreen(
			OS_window_handle,
		   &point);
	}

	SetCursorPos(x, y);
}

SLONG OS_message_handler_request_quit;

LRESULT CALLBACK OS_message_handler(
					HWND   window_handle,
					UINT   message_type,
					WPARAM param_w,
					LPARAM param_l)
{
	UBYTE scancode;

	switch(message_type)
	{
		case WM_PAINT:

			//
			// The user callback function does all the screen drawing.
			// Do enough to appease windows.
			//

			HDC  		device_context_handle;
			PAINTSTRUCT	paintstruct;

			device_context_handle = BeginPaint(window_handle, &paintstruct);
			EndPaint(window_handle, &paintstruct);

			return 0;

		case WM_CHAR:
			KEY_inkey  = param_w;
			break;

		case WM_KEYDOWN: 
		case WM_KEYUP:	 

			//
			// Keyboard stuff.
			//

			scancode  = (param_l >> 16) & 0xff;
			scancode |= (param_l >> 17) & 0x80;

			if (message_type == WM_KEYDOWN)
			{
				KEY_on[scancode] = 1;
			}
			else
			{
				KEY_on[scancode] = 0;
			}
			
			//
			// Alt keys don't work.
			//

			KEY_on[KEY_LALT] = 0;
			KEY_on[KEY_RALT] = 0;

			//
			// Check for shift/alt/control keys.
			//

			KEY_shift = 0;

			if (KEY_on[KEY_LSHIFT  ] || KEY_on[KEY_RSHIFT  ]) KEY_shift |= KEY_SHIFT;
			if (KEY_on[KEY_LCONTROL] || KEY_on[KEY_RCONTROL]) KEY_shift |= KEY_CONTROL;
			if (KEY_on[KEY_LALT    ] || KEY_on[KEY_RALT    ]) KEY_shift |= KEY_ALT;

			if (KEY_on[KEY_ESCAPE])
			{
				OS_message_handler_request_quit = TRUE;
			}

			break;

		case WM_MOVE:

			//
			// Tell the frame about the new position of the window.
			//

			OS_frame.Move(LOWORD(param_l),HIWORD(param_l));
			
			//
			// Fall through to the default handling.
			//

			break;

		case WM_CLOSE:
			OS_message_handler_request_quit = TRUE;
			return 0;

		default:
			break;
	}


	//
	// Just let windows do its normal thing.
	//

	return DefWindowProc(
				window_handle,
				message_type,
				param_w,
				param_l);

}


void OS_process_messages()
{
	MSG msg;
	int ret;

	while(1)
	{
		//
		// Poll the joystick.
		//

		OS_joy_poll();

		//
		// Activate the changes made to the direct sound settings...
		//

		OS_sound_update_changes();

		//
		// Wait for a message to be available.
		//

		WaitMessage();

		while(PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			ret = GetMessage(&msg, NULL, 0, 0);

			if (ret == 0 || ret == -1)
			{
				//
				// Kill the game!
				//

				return;
			}

			TranslateMessage(&msg);
			DispatchMessage (&msg);
		}

		if (OS_message_handler_request_quit)
		{
			OS_message_handler_request_quit = FALSE;

			//
			// The window has been told to close!
			//

			return;
		}
	}
}

//
// Valid devices.
//

typedef struct
{
	D3DEnum_DriverInfo *driver;
	D3DEnum_DeviceInfo *device;
	D3DEnum_ModeInfo   *mode;		// NULL => Use windowed mode.

} OS_Mode;

#define OS_MAX_MODES 16

OS_Mode OS_mode[OS_MAX_MODES];
SLONG   OS_mode_upto;
SLONG   OS_mode_sel;

//
// Finds the valid devices from the D3DEnumerated choice and set
// the OS_mode_sel to the default.
//

void OS_mode_init()
{
	SLONG i;

	D3DEnum_DriverInfo *vi;
	D3DEnum_DeviceInfo *ci;
	D3DEnum_ModeInfo   *mi; 	// NULL => Use windowed mode.

	OS_mode_upto = 0;
	OS_mode_sel  = 0;

	SLONG lookfor640x480;
	SLONG lookfor512x384;

	//
	// Find all valid modes.
	//

	for (vi = D3DEnum_GetFirstDriver(); vi; vi = vi->pNext)
	{
		for (ci = vi->pFirstDevice; ci; ci = ci->pNext)
		{
			if (ci->bIsHardware)
			{
				//
				// Found a hardware device.
				//

				if (WITHIN(OS_mode_upto, 0, OS_MAX_MODES - 1))
				{
					OS_mode[OS_mode_upto].driver = vi;
					OS_mode[OS_mode_upto].device = ci;
					OS_mode[OS_mode_upto].mode   = NULL;
					
					#ifdef NDEBUG

					lookfor512x384 = FALSE;
					lookfor640x480 = TRUE;

					#else

					lookfor512x384 = FALSE;
					lookfor640x480 = !ci->bWindowed;

					#endif

					if (lookfor512x384)
					{
						//
						// Look for the first 512x384 mode.
						//

						for (mi = ci->pFirstMode; mi; mi = mi->pNext)
						{
							if (mi->ddsd.dwWidth  == 512 &&
								mi->ddsd.dwHeight == 384)
							{
								OS_mode[OS_mode_upto].mode = mi;

								//
								// We already have our mode.
								//

								lookfor640x480 = FALSE;

								break;
							}
						}
					}

					if (lookfor640x480)
					{
						//
						// Look for the first 640x480 mode.
						//

						for (mi = ci->pFirstMode; mi; mi = mi->pNext)
						{
							if (mi->ddsd.dwWidth  == 640 &&
								mi->ddsd.dwHeight == 480)
							{
								OS_mode[OS_mode_upto].mode = mi;

								break;
							}
						}
					}

					if (OS_mode[OS_mode_upto].mode == NULL)
					{
						//
						// Make sure this device support windowed mode!
						//

						if (OS_mode[OS_mode_upto].device->bWindowed)
						{
							//
							// We are ok.
							//
						}
						else
						{
							//
							// Use the first available mode.
							//

							OS_mode[OS_mode_upto].mode = OS_mode[OS_mode_upto].device->pFirstMode;
						}
					}

					OS_mode_upto += 1;
				}
			}
		}
	}

	#ifndef NDEBUG

	//
	// In debug build choose the first windowed mode.
	//

	for (i = 0; i < OS_mode_upto; i++)
	{
		if (OS_mode[i].device->bWindowed)
		{
			OS_mode_sel = i;
		}
	}

	#else

	//
	// In release build choose the last mode.
	//

	OS_mode_sel = OS_mode_upto - 1;

	#endif
}


//
// Adds the modes for the current selection to the combo box.
//

void OS_mydemo_setup_mode_combo(HWND combo_handle, SLONG mode)
{
	SLONG index;

	ASSERT(WITHIN(mode, 0, OS_mode_upto - 1));

	//
	// Clear all old modes.
	//

	SendMessage(combo_handle, CB_RESETCONTENT, 0, 0);

	//
	// Add each mode.
	//

	D3DEnum_ModeInfo *mi;

	if (OS_mode[mode].device->bWindowed)
	{
		index = SendMessage(combo_handle, CB_ADDSTRING, 0, (LPARAM) "In a window");

		SendMessage(combo_handle, CB_SETITEMDATA, (WPARAM) index, (LPARAM) NULL);

		if (NULL == OS_mode[mode].mode)
		{
			//
			// This is the current selection.
			//

			SendMessage(combo_handle, CB_SETCURSEL, index, 0);
		}
	}

	for (mi = OS_mode[mode].device->pFirstMode; mi; mi = mi->pNext)
	{
		index = SendMessage(combo_handle, CB_ADDSTRING, 0, (LPARAM) mi->strDesc);

		SendMessage(combo_handle, CB_SETITEMDATA, (WPARAM) index, (LPARAM) mi);

		if (mi == OS_mode[mode].mode)
		{
			//
			// This is the current selection.
			//

			SendMessage(combo_handle, CB_SETCURSEL, index, 0);
		}
	}
}



//
// The callback function for the MyDemo dialog box.
//

#define OS_MYDEMO_RUN	1
#define OS_MYDEMO_EXIT	2

BOOL CALLBACK OS_mydemo_proc(
				HWND   dialog_handle,
				UINT   message_type,
				WPARAM param_w,
				LPARAM param_l)
{
	SLONG i;
	SLONG d;
	SLONG res;
	SLONG index;

	RECT rect;

	HWND combo_handle;

	D3DEnum_DriverInfo *vi;
	D3DEnum_DeviceInfo *ci;

	switch(message_type)
	{
		case WM_INITDIALOG:

			//
			// Fill out the list boxes with the correct values.  First find
			// all compatible
			//

			combo_handle = GetDlgItem(dialog_handle, IDC_COMBO_DRIVER);

			for (i = 0; i < OS_mode_upto; i++)
			{
				SendMessage(combo_handle, CB_ADDSTRING, 0, (LPARAM) OS_mode[i].driver->strDesc);
			}

			//
			// Set the current selection.
			//

			SendMessage(combo_handle, CB_SETCURSEL, OS_mode_sel, 0);

			//
			// Add the modes for the current selection.
			//

			combo_handle = GetDlgItem(dialog_handle, IDC_COMBO_MODE);

			OS_mydemo_setup_mode_combo(combo_handle, OS_mode_sel);

			{
				CBYTE mhz[64];

				sprintf(mhz, "Detected a %dMhz processor", OS_mhz);

				//
				// What speed processor have we detected?
				//

				SetWindowText(GetDlgItem(dialog_handle, IDC_PROCESSOR), mhz);
			}

			//
			// Run the egg physics by default.
			//

			CheckDlgButton(dialog_handle, IDC_RUN_PATCH_TEST, BST_CHECKED);

			return TRUE;

		case WM_COMMAND:

			switch(LOWORD(param_w))
			{
				case IDOK:
					EndDialog(dialog_handle, OS_MYDEMO_RUN);
					return TRUE;

				case IDCANCEL:
					EndDialog(dialog_handle, OS_MYDEMO_EXIT);
					return TRUE;

				case IDC_COMBO_DRIVER:

					switch(HIWORD(param_w))
					{
						case CBN_SELCHANGE:
							
							//
							// Change the list of modes.
							//

							OS_mode_sel = SendMessage((HWND) param_l, CB_GETCURSEL, 0, 0);

							ASSERT(WITHIN(OS_mode_sel, 0, OS_mode_upto - 1));

							OS_mydemo_setup_mode_combo(
								GetDlgItem(dialog_handle, IDC_COMBO_MODE),
								OS_mode_sel);

							break;
					}
					
					break;

				case IDC_COMBO_MODE:

					switch(HIWORD(param_w))
					{
						case CBN_SELCHANGE:
							
							//
							// Update the current mode.
							//

							index = SendMessage((HWND) param_l, CB_GETCURSEL, 0, 0);

							ASSERT(WITHIN(OS_mode_sel, 0, OS_mode_upto - 1));

							OS_mode[OS_mode_sel].mode = (D3DEnum_ModeInfo * /* We hope */) SendMessage((HWND) param_l, CB_GETITEMDATA, (WPARAM) index, 0);

							break;
					}
					
					break;
			}

			break;

		case WM_CLOSE:
			EndDialog(dialog_handle, OS_MYDEMO_EXIT);
			return TRUE;
	}

	return FALSE;
}


//
// The game thread's function and HANDLE.
//

HANDLE OS_game_thread_handle;
DWORD  OS_game_thread_id;

DWORD WINAPI OS_game_thread_function(void *data)
{
	MAIN_main();

	OS_message_handler_request_quit = TRUE;
	OS_game_thread_handle           = FALSE;
	OS_game_thread_id               = FALSE;
	
	//
	// Send a windows message to our window to wake it up so it'll exit.
	//

	PostMessage(OS_window_handle, WM_CLOSE, 0, 0);

	return 0;
}




//
// The entry point of the program.
//

int WINAPI WinMain(
			HINSTANCE this_instance,
			HINSTANCE last_instance,
			LPSTR	  command_line,
			int       start_show_state)
{
	HRESULT res;

	//
	// Remember the arguments passed to this function.
	//
	
	OS_this_instance	= this_instance;
	OS_last_instance	= last_instance;
	OS_command_line		= command_line;
	OS_start_show_state	= start_show_state;

	OS_wcl.hInstance		= this_instance;
	OS_wcl.lpszClassName	= OS_application_name;
	OS_wcl.lpfnWndProc		= OS_message_handler;
	OS_wcl.style			= 0;
	OS_wcl.cbSize			= sizeof(WNDCLASSEX);
	OS_wcl.cbClsExtra		= 0;
	OS_wcl.cbWndExtra		= 0;
	OS_wcl.lpszMenuName		= NULL;
	OS_wcl.hIcon			= LoadIcon(this_instance, MAKEINTRESOURCE(IDI_ICON1));
	OS_wcl.hIconSm			= NULL;//LoadIcon(this_instance, MAKEINTRESOURCE(IDI_ICON1));
	OS_wcl.hCursor			= LoadCursor(NULL, IDC_ARROW);
	OS_wcl.hbrBackground	= (HBRUSH) GetStockObject(NULL_BRUSH);

	//
	// Register the window class.
	//
	
	if (RegisterClassEx(&OS_wcl) == 0)
	{
		//
		// Could not register the window class!
		//

		return 0;
	}

	//
	// The window size in windowed mode.
	//

	#define OS_WINDOW_WIDTH  640
	#define OS_WINDOW_HEIGHT 480

	//
	// Create a window 
	//

	RECT rect;

	rect.left   = 100 + 0;
	rect.right  = 100 + OS_WINDOW_WIDTH;
	rect.top    = 100 + 0;
	rect.bottom = 100 + OS_WINDOW_HEIGHT;

	AdjustWindowRect(
		&rect,
		 WS_CAPTION,
		 FALSE);

	OS_window_handle = CreateWindow(
						OS_application_name,
						OS_application_name,
						WS_CAPTION | WS_SYSMENU,
						50,
						50,
						rect.right  - rect.left,
						rect.bottom - rect.top,
						NULL,
						NULL,
						this_instance,
						NULL);

	//
	// Make sure it worked.
	//

	if (OS_window_handle == 0)
	{
		return 0;
	}

	//
	// Initialise joystick control.
	//

	OS_joy_init();

	//
	// Find out the speed of the machine we are running on.
	//

	OS_work_out_mhz();

	//
	// Enumerate the devices.
	//

	D3DEnum_EnumerateDevices(NULL);

	D3DEnum_DriverInfo *di = D3DEnum_GetFirstDriver();

	//
	// Find valid modes.
	//

	OS_mode_init();

	if (OS_mode_upto == 0)
	{
		//
		// No valid screen mode!
		//

		MessageBox(
			OS_window_handle,
			"Could not find a 3d accelerator card.  Make sure that you have DirectX 6.0 or higher installed.", "Beat", MB_ICONERROR | MB_OK);

		exit(1);
	}

  have_another_go:;

	//
	// Promt the user to choose a mode.
	//

	switch(DialogBox(
			OS_this_instance,
			MAKEINTRESOURCE(IDD_DRIVERS),
			OS_window_handle,
			OS_mydemo_proc))
	{
		case OS_MYDEMO_RUN:
			break;

		case OS_MYDEMO_EXIT:

			//
			// Close gracefully...
			// 

			D3DEnum_FreeResources();

			return 0;

		default:
			exit(1);
			break;
	}

	{
		GUID           *driver;
		GUID           *device;
		DDSURFACEDESC2 *display_mode;
		BOOL            is_windowed;
		BOOL            is_hardware;

		#if WE_USE_THE_DEFAULT_DIALOG_BOX		

		//
		// Prompt the user for a device and a driver.
		//

		INT ret = D3DEnum_UserDlgSelectDriver(OS_window_handle, TRUE);

		D3DEnum_GetSelectedDriver(
			&driver,
			&device,
			&display_mode,
			&is_windowed,
			&is_hardware);

		OS_frame_is_fullscreen = !is_windowed;
		OS_frame_is_hardware   =  is_hardware;

		#else

		driver = OS_mode[OS_mode_sel].driver->pGUID;
		device = OS_mode[OS_mode_sel].device->pGUID;

		if (OS_mode[OS_mode_sel].mode)
		{
			display_mode = &OS_mode[OS_mode_sel].mode->ddsd;

			OS_frame_is_fullscreen = TRUE;
			OS_frame_is_hardware   = TRUE;
		}
		else
		{
			display_mode = NULL;

			OS_frame_is_fullscreen = FALSE;
			OS_frame_is_hardware   = TRUE;
		}

		#endif

		//
		// Initialise the framework.
		//

		DWORD flags;

		flags  = 0;
		flags |= D3DFW_BACKBUFFER;
		flags |= D3DFW_ZBUFFER;

		if (OS_frame_is_fullscreen)
		{
			flags |= D3DFW_FULLSCREEN;
		}

		if (OS_frame_is_fullscreen)
		{
			//
			// Hide the mouse.
			// 

			ShowCursor(FALSE);

			//
			// Makes the window not redraw itself when we go fullscreen.
			//

			SetWindowLong(OS_window_handle, GWL_STYLE, WS_POPUP);
		}

		res = OS_frame.Initialize(
						OS_window_handle,
						driver,
						device,
						display_mode,
						flags);
		
		if (res == S_OK)
		{
			if (!OS_frame_is_fullscreen)
			{
				//
				// Show our window!
				//

				ShowWindow(OS_window_handle, SW_SHOW);
			}

			//
			// Enumerate texture formats.
			//

			{
				int i;
				
				OS_Tformat *otf;

				//
				// Find the texture formats.
				//

				OS_frame.GetD3DDevice()->EnumTextureFormats(OS_texture_enumerate_pixel_formats, NULL);

				//
				// The pixel format of the screen.
				//

				{
					DDSURFACEDESC2 ddsd;

					memset(&ddsd, 0, sizeof(ddsd));

					ddsd.dwSize = sizeof(ddsd);

					if (OS_frame.GetBackBuffer()->GetSurfaceDesc(&ddsd) == DD_OK)
					{
						OS_tformat[OS_TEXTURE_FORMAT_SCREEN].valid = TRUE;
						OS_tformat[OS_TEXTURE_FORMAT_SCREEN].ddpf  = ddsd.ddpfPixelFormat;
					}
				}

				//
				// Set the masks and shifts for each texture format.
				//

				for (i = 0; i < OS_TEXTURE_FORMAT_NUMBER; i++)
				{
					otf = &OS_tformat[i];

					if (i == OS_TEXTURE_FORMAT_8)
					{
						//
						// We don't have to set the masks and shifts for grayscale textures.
						//

						continue;
					}

					if (otf->valid)
					{
						//
						// Calculate the masks and shifts.
						//

						OS_calculate_mask_and_shift(otf->ddpf.dwRBitMask, &otf->mask_r, &otf->shift_r);
						OS_calculate_mask_and_shift(otf->ddpf.dwGBitMask, &otf->mask_g, &otf->shift_g);
						OS_calculate_mask_and_shift(otf->ddpf.dwBBitMask, &otf->mask_b, &otf->shift_b);
									 
						if (otf->ddpf.dwFlags & DDPF_ALPHAPIXELS)
						{
							OS_calculate_mask_and_shift(otf->ddpf.dwRGBAlphaBitMask, &otf->mask_a, &otf->shift_a);
						}
					}
				}
			}

			//
			// What is the screen-res?
			//

			RECT *dimensions = OS_frame.GetViewportRect();

			OS_screen_width  = float(dimensions->right);
			OS_screen_height = float(dimensions->bottom);

			//
			// Work out how to multi-texture, find all the texture directories,
			// initailise the sound system and setup the floating point unit.
			//

			ftol_init();
			OS_pipeline_calculate();
			OS_init_texpaths();
			OS_sound_init();

			//
			// Time relative to the beginning of the program.
			//

			OS_game_start_tick_count = GetTickCount();

			//
			// Start the game.
			//

			OS_game_thread_handle = CreateThread(
										NULL,
										0,
										OS_game_thread_function,
										NULL,
										0,
									   &OS_game_thread_id);

			if (OS_game_thread_handle)
			{
				//
				// Make the thread get less time than the message loop!
				//

				SetThreadPriority(
					OS_game_thread_handle,
					THREAD_PRIORITY_BELOW_NORMAL);

				//
				// This thread processes messages while the game
				// thread runs the game.
				//

				OS_process_messages();

				if (OS_game_thread_handle)
				{
					//
					// Exit the game thread.
					//

					TerminateThread(
						OS_game_thread_handle,
						0);
				}
			}

			//
			// Clean up.
			//

			OS_frame.DestroyObjects();

			OS_sound_finish();
		}
		else
		{
			if (OS_frame_is_fullscreen)
			{
				//
				// Show the mouse.
				// 

				ShowCursor(TRUE);
			}

			//
			// Could not set that mode!
			//

			CBYTE *err;

			if (res == D3DFWERR_NOZBUFFER)
			{
				err = "There was not enough memory to create the zbuffer. Try using a lower resolution mode.";

			}
			else
			{
				err = "Could not setup the display using those settings. Try changing driver or using another mode.";
			}

			MessageBox(
				OS_window_handle,
				err,
				"Beat",
				MB_ICONERROR | MB_OK);

			//
			// Have another go...
			//

			goto have_another_go;
		}
	}

	//
	// Free all enumeration resources.
	//

	D3DEnum_FreeResources();

	return 0;
}


// ========================================================
//
// ROTATING POINTS
//
// ========================================================

//
// The camera and the screen.
//

float OS_cam_x;
float OS_cam_y;
float OS_cam_z;
float OS_cam_aspect;
float OS_cam_lens;
float OS_cam_view_dist;
float OS_cam_over_view_dist;
float OS_cam_matrix[9];
float OS_cam_view_matrix[9];
float OS_cam_view_matrix_plus_local_rot[9];

float OS_cam_screen_x1;
float OS_cam_screen_y1;
float OS_cam_screen_x2;
float OS_cam_screen_y2;

float OS_cam_screen_width;
float OS_cam_screen_height;
float OS_cam_screen_mid_x;
float OS_cam_screen_mid_y;
float OS_cam_screen_mul_x;
float OS_cam_screen_mul_y;

void OS_camera_set(
		float world_x,
		float world_y,
		float world_z,
		float view_dist,
		float yaw,
		float pitch,
		float roll,
		float lens,
		float screen_x1,
		float screen_y1,
		float screen_x2,
		float screen_y2)
{
	float matrix[9];

	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	OS_camera_set(
		world_x,
		world_y,
		world_z,
		view_dist,
		matrix,
		lens,
		screen_x1,
		screen_y1,
		screen_x2,
		screen_y2);
}

void OS_camera_set(
		float world_x,
		float world_y,
		float world_z,
		float view_dist,
		float matrix[9],
		float lens,
		float screen_x1,
		float screen_y1,
		float screen_x2,
		float screen_y2)
{
	OS_cam_screen_x1 = screen_x1 * OS_screen_width;
	OS_cam_screen_y1 = screen_y1 * OS_screen_height;
	OS_cam_screen_x2 = screen_x2 * OS_screen_width;
	OS_cam_screen_y2 = screen_y2 * OS_screen_height;

	OS_cam_screen_width  = OS_cam_screen_x2 - OS_cam_screen_x1;
	OS_cam_screen_height = OS_cam_screen_y2 - OS_cam_screen_y1;
	OS_cam_screen_mid_x  = OS_cam_screen_x1 + OS_cam_screen_width  * 0.5F;
	OS_cam_screen_mid_y  = OS_cam_screen_y1 + OS_cam_screen_height * 0.5F;
	OS_cam_screen_mul_x  = OS_cam_screen_width  * 0.5F / OS_ZCLIP_PLANE;
	OS_cam_screen_mul_y  = OS_cam_screen_height * 0.5F / OS_ZCLIP_PLANE;

	OS_cam_x = world_x;
	OS_cam_y = world_y;
	OS_cam_z = world_z;

	OS_cam_lens           = lens;
	OS_cam_view_dist      = view_dist;
	OS_cam_over_view_dist = 1.0F / view_dist;
	OS_cam_aspect         = OS_cam_screen_height / OS_cam_screen_width;

	memcpy(OS_cam_matrix,      matrix, sizeof(OS_cam_matrix));
	memcpy(OS_cam_view_matrix, matrix, sizeof(OS_cam_matrix));

	MATRIX_skew(
		OS_cam_matrix,
		OS_cam_aspect,
		OS_cam_lens,
		OS_cam_over_view_dist);		// Shrink the matrix down so the furthest point has a view distance z of 1.0F
}



OS_Trans OS_trans[OS_MAX_TRANS];
SLONG    OS_trans_upto;

void OS_transform(
		float     world_x,
		float     world_y,
		float     world_z,
		OS_Trans *os)
{
	os->x = world_x - OS_cam_x;
	os->y = world_y - OS_cam_y;
	os->z = world_z - OS_cam_z;

	MATRIX_MUL(
		OS_cam_matrix,
		os->x,
		os->y,
		os->z);

	os->clip = OS_CLIP_ROTATED;

	if (os->z < OS_ZCLIP_PLANE)
	{
		os->clip |= OS_CLIP_NEAR;

		return;
	}
	else
	if (os->z > 1.0F)
	{
		os->clip |= OS_CLIP_FAR;

		return;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		os->Z = OS_ZCLIP_PLANE / os->z;

		os->X = OS_cam_screen_mid_x + OS_cam_screen_mul_x * os->x * os->Z;
		os->Y = OS_cam_screen_mid_y - OS_cam_screen_mul_y * os->y * os->Z;

		//
		// Set the clipping flags.
		//

		os->clip |= OS_CLIP_TRANSFORMED;

		     if (os->X < 0.0F           ) {os->clip |= OS_CLIP_LEFT;}
		else if (os->X > OS_screen_width) {os->clip |= OS_CLIP_RIGHT;}

		     if (os->Y < 0.0F            ) {os->clip |= OS_CLIP_TOP;}
		else if (os->Y > OS_screen_height) {os->clip |= OS_CLIP_BOTTOM;}

		return;
	}
}

float OS_local_world_x;
float OS_local_world_y;
float OS_local_world_z;
float OS_local_world_matrix[9];
float OS_local_view_x;
float OS_local_view_y;
float OS_local_view_z;
float OS_local_matrix_plus_camera[9];

void OS_local_position_and_rotation(
		float world_x,
		float world_y,
		float world_z,
		float local_rotation_matrix[9])
{
	//
	// Store the local rotation and position.
	//

	OS_local_world_x = world_x;
	OS_local_world_y = world_y;
	OS_local_world_z = world_z;

	memcpy(OS_local_world_matrix, local_rotation_matrix, sizeof(OS_local_world_matrix));

	MATRIX_TRANSPOSE(OS_local_world_matrix);

	//
	// Premultiply this matrix by the camera matrix.
	//

	MATRIX_3x3mul(OS_local_matrix_plus_camera,       OS_cam_matrix,      OS_local_world_matrix);
	MATRIX_3x3mul(OS_cam_view_matrix_plus_local_rot, OS_cam_view_matrix, OS_local_world_matrix);

	//
	// Put the centre of the object into viewspace.
	//

	OS_local_view_x = world_x - OS_cam_x;
	OS_local_view_y = world_y - OS_cam_y;
	OS_local_view_z = world_z - OS_cam_z;

	MATRIX_MUL(
		OS_cam_matrix,
		OS_local_view_x,
		OS_local_view_y,
		OS_local_view_z);
}

void OS_transform_local(
		float     local_x,
		float     local_y,
		float     local_z,
		OS_Trans *os)
{
	//
	// Work out the posiiton of this point in viewspace.
	//

	MATRIX_MUL(
		OS_local_matrix_plus_camera,
		local_x,
		local_y,
		local_z);

	os->x = local_x + OS_local_view_x;
	os->y = local_y + OS_local_view_y;
	os->z = local_z + OS_local_view_z;

	os->clip = OS_CLIP_ROTATED;

	if (os->z < OS_ZCLIP_PLANE)
	{
		os->clip |= OS_CLIP_NEAR;

		return;
	}
	else
	if (os->z > 1.0F)
	{
		os->clip |= OS_CLIP_FAR;

		return;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		os->Z = OS_ZCLIP_PLANE / os->z;

		os->X = OS_cam_screen_mid_x + OS_cam_screen_mul_x * os->x * os->Z;
		os->Y = OS_cam_screen_mid_y - OS_cam_screen_mul_y * os->y * os->Z;

		//
		// Set the clipping flags.
		//

		os->clip |= OS_CLIP_TRANSFORMED;

		     if (os->X < 0.0F           ) {os->clip |= OS_CLIP_LEFT;}
		else if (os->X > OS_screen_width) {os->clip |= OS_CLIP_RIGHT;}

		     if (os->Y < 0.0F            ) {os->clip |= OS_CLIP_TOP;}
		else if (os->Y > OS_screen_height) {os->clip |= OS_CLIP_BOTTOM;}

		return;
	}
}




// ========================================================
//
// DRAWING STUFF
//
// ========================================================

void OS_clear_screen(UBYTE r, UBYTE g, UBYTE b, float z)
{
	ULONG colour = (r << 16) | (g << 8) | (b << 0);
	
	HRESULT ret = OS_frame.GetViewport()->Clear2(
								1,
								(D3DRECT *) OS_frame.GetViewportRect(),
								D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
								colour,
								z,
								0);
}



SLONG OS_in_scene;

void OS_scene_begin()
{
	ASSERT(!OS_in_scene);

	OS_frame.GetD3DDevice()->BeginScene();

	OS_in_scene = TRUE;

	//
	// Set the render states to their default values.
	//

	OS_init_renderstates();
}

void OS_scene_end()
{
	ASSERT(OS_in_scene);

	OS_frame.GetD3DDevice()->EndScene();

	OS_in_scene = FALSE;
}


SLONG OS_fps;
SLONG OS_last_time;
SLONG OS_last_frame_count;
SLONG OS_frame_count;

SLONG OS_fps_get()
{
	SLONG now;

	now             = OS_ticks();
	OS_frame_count += 1;

	if (now >= OS_last_time + 1000)
	{
		OS_fps              = OS_frame_count - OS_last_frame_count;
		OS_last_frame_count = OS_frame_count;
		OS_last_time        = now;
	}

	return OS_fps;
}

void OS_fps_draw()
{
	SLONG i;

	float x1;
	float y1;
	float x2;
	float y2;

	float tick;

	SLONG now;

	now             = OS_ticks();
	OS_frame_count += 1;

	if (now >= OS_last_time + 1000)
	{
		OS_fps              = OS_frame_count - OS_last_frame_count;
		OS_last_frame_count = OS_frame_count;
		OS_last_time        = now;
	}

	OS_Buffer *ob = OS_buffer_new();

	for (i = 0; i < OS_fps; i++)
	{
		switch((i + 1) % 10)
		{
			case 0:
				tick = 8.0F;
				break;

			case 5:
				tick = 5.0F;
				break;

			default:
				tick = 3.0F;
				break;
		}


		x1 = 5.0F + i * 2.0F;
		y1 = 5.0F;
		x2 = 5.0F + i * 2.0F + 1.0F;
		y2 = 5.0F + tick;

		x1 /= OS_screen_width;
		y1 /= OS_screen_height;
		x2 /= OS_screen_width;
		y2 /= OS_screen_height;

		OS_buffer_add_sprite(
			ob,
			x1,
			y1,
			x2,
			y2,
			0.0F, 1.0F,
			0.0F, 1.0F,
			0.0F,
			0x00ffffff,
			0x00000000,
			OS_FADE_BOTTOM);
	}

	OS_buffer_draw(ob, NULL);
}


void OS_show()
{
	if (OS_in_scene)
	{
		OS_scene_end();
	}

	if (OS_frame_is_fullscreen)
	{
		//
		// Do we blit or do we flip?
		// 

		if (1)
		{
			//
			// Flip.
			// 

			OS_frame.ShowFrame();
		}
		else
		{
			//
			// Blit.
			//

			LPDIRECTDRAWSURFACE4 fb = OS_frame.GetFrontBuffer();
			LPDIRECTDRAWSURFACE4 bb = OS_frame.GetBackBuffer();

			fb->Blt(NULL, bb, NULL, DDBLT_WAIT, NULL);
		}
	}
	else
	{
		OS_frame.ShowFrame();
	}

	OS_poly_count_drawn       = 0;
	OS_poly_count_transformed = 0;
}

//
// Our flexible vertex format.
//

typedef struct
{
	float sx;
	float sy;
	float sz;
	float rhw;
	ULONG colour;
	ULONG specular;
	float tu1;
	float tv1;
	float tu2;
	float tv2;

} OS_Flert;

#define OS_FLERT_FORMAT	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)

// ========================================================
//
// BUFFER STUFF
//
// ========================================================

typedef struct os_buffer
{
	SLONG num_flerts;
	SLONG num_indices;

	SLONG max_flerts;
	SLONG max_indices;

	OS_Flert *flert;
	UWORD    *index;

	OS_Buffer *next;

} OS_Buffer;

OS_Buffer *OS_buffer_free;


//
// Creates a new buffer.
//

OS_Buffer *OS_buffer_create(void)
{
	//
	// Allocate the buffer.
	//

	OS_Buffer *ob = (OS_Buffer *) malloc(sizeof(OS_Buffer));

	//
	// Initialise the buffer.
	//

	ob->max_flerts  = 256;
	ob->max_indices = 1024;

	ob->num_flerts  = 0;
	ob->num_indices = 1;

	ob->flert = (OS_Flert *) malloc(sizeof(OS_Flert) * ob->max_flerts );
	ob->index = (UWORD    *) malloc(sizeof(UWORD   ) * ob->max_indices);

	memset(ob->flert, 0, sizeof(OS_Flert) * ob->max_flerts );
	memset(ob->index, 0, sizeof(UWORD   ) * ob->max_indices);

	ob->next = NULL;

	return ob;
}

//
// Gets a buffer from the linked list of free buffers. If the free list is empty,
// it creates a new one.
//

OS_Buffer *OS_buffer_get(void)
{
	OS_Buffer *ans;

	if (OS_buffer_free)
	{
		ans            = OS_buffer_free;
		OS_buffer_free = OS_buffer_free->next;
		ans->next      = NULL;
	}
	else
	{
		ans = OS_buffer_create();
	}

	return ans;
}

//
// Returns a buffer to the free list.
//

void OS_buffer_give(OS_Buffer *ob)
{
	ob->next       = OS_buffer_free;
	OS_buffer_free = ob;
}

OS_Buffer *OS_buffer_new(void)
{
	OS_Buffer *ob = OS_buffer_get();

	ob->num_indices = 0;
	ob->num_flerts  = 1;

	return ob;
}

//
// Grows the size of the flert array.
//

void OS_buffer_grow_flerts(OS_Buffer *ob)
{
	ob->max_flerts *= 2;

	ob->flert = (OS_Flert *) realloc(ob->flert, ob->max_flerts * sizeof(OS_Flert));
}

void OS_buffer_grow_indices(OS_Buffer *ob)
{
	ob->max_indices *= 2;

	ob->index = (UWORD *) realloc(ob->index, ob->max_indices * sizeof(UWORD));
}

void OS_buffer_add_vert(OS_Buffer *ob, OS_Vert *ov)
{
	OS_Trans *ot;
	OS_Flert *of;

	//
	// Make sure we've got enough room for another vertex.
	//

	if (ob->num_flerts >= ob->max_flerts)
	{
		//
		// We need a bigger buffer.
		//

		OS_buffer_grow_flerts(ob);
	}

	ASSERT(WITHIN(ov->trans, 0, OS_MAX_TRANS - 1));

	of = &ob->flert[ob->num_flerts];
	ot = &OS_trans [ov->trans];

	//
	// Create the new tlvertex.
	//

	of->sx       = ot->X;
	of->sy       = ot->Y;
	of->sz       = 1.0F - ot->Z; //ot->z;
	of->rhw      = ot->Z;
	of->colour   = ov->colour;
	of->specular = ov->specular;
	of->tu1      = ov->u1;
	of->tv1      = ov->v1;
	of->tu2      = ov->u2;
	of->tv2      = ov->v2;

	//
	// Store the index of the flert inside the vertex.
	//

	ov->index = ob->num_flerts++;
}


//
// Returns the colour 'v' along from colour1 to colour2 where 0.0 <= v <= 1.0
//

ULONG OS_interpolate_colour(float v, ULONG colour1, ULONG colour2)
{
	SLONG rb1, rb2, drb, rba;
	SLONG ga1, ga2, dga, gaa;

	union
	{
		float vfloat;
		ULONG vfixed8;
	};

	SLONG answer;

	//
	// Early outs.
	//

	if (colour1 == colour2) {return colour1;}
	
	if (v < 0.01F) {return colour1;}
	if (v > 0.99F) {return colour2;}

	//
	// Work out how much to interpolate along in fixed point 8.
	//

	vfloat   = 32768.0F + v;
	vfixed8 &= 0xff;

	//
	// Red and blue.
	//

	rb1 = colour1 & (0x00ff00ff);
	rb2 = colour2 & (0x00ff00ff);

	if (rb1 != rb2)
	{
		//
		// Do interpolation of red and blue simultaneously.
		//

		drb   = rb2 - rb1;
		drb  *= vfixed8;
		drb >>= 8;
		rba   = rb1 + drb;
		rba  &= 0x00ff00ff;
	}
	else
	{
		//
		// No need to interpolated red and blue.
		//

		rba = rb1;
	}

	//
	// Green and alpha.
	//
	
	ga1 = colour1 >> 8;
	ga2 = colour2 >> 8;

	ga1 &= 0x00ff00ff;
	ga2 &= 0x00ff00ff;

	if (ga1 != ga2)
	{
		//
		// Do interpolationg of red and blue simultaneously.
		//

		dga  = ga2 - ga1;
		dga *= vfixed8;
		gaa  = (ga1 << 8) + dga;
		gaa &= 0xff00ff00;
	}
	else
	{
		//
		// No need to interpolate green and alpha.
		//

		gaa = ga1 << 8;
	}

	answer = rba | gaa;

	return answer;
}



// ========================================================
//
// Functions to pass the the CLIP module.
//

void OS_clip_helper_interpolate(void *new_point, void *point1, void *point2, float along)
{
	OS_Vert *ov1 = (OS_Vert *) point1;
	OS_Vert *ov2 = (OS_Vert *) point2;
	OS_Vert *ovn = (OS_Vert *) new_point;

	ASSERT(WITHIN(ov1->trans, 0, OS_trans_upto - 1));
	ASSERT(WITHIN(ov2->trans, 0, OS_trans_upto - 1));

	OS_Trans *ot1 = &OS_trans[ov1->trans];
	OS_Trans *ot2 = &OS_trans[ov2->trans];

	//
	// We need a new OS_Trans for the new point.
	//

	ASSERT(WITHIN(OS_trans_upto, 0, OS_MAX_TRANS - 1));

	OS_Trans *otn = &OS_trans[OS_trans_upto];

	otn->x = ot1->x + along * (ot2->x - ot1->x);
	otn->y = ot1->y + along * (ot2->y - ot1->y);
	otn->z = OS_ZCLIP_PLANE;
	otn->Z = 1.0F;
	otn->X = OS_cam_screen_mid_x + OS_cam_screen_mul_x * otn->x;
	otn->Y = OS_cam_screen_mid_y - OS_cam_screen_mul_y * otn->y;

	otn->clip = OS_CLIP_TRANSFORMED | OS_CLIP_ROTATED;

		 if (otn->X < 0.0F           ) {otn->clip |= OS_CLIP_LEFT;}
	else if (otn->X > OS_screen_width) {otn->clip |= OS_CLIP_RIGHT;}

		 if (otn->Y < 0.0F            ) {otn->clip |= OS_CLIP_TOP;}
	else if (otn->Y > OS_screen_height) {otn->clip |= OS_CLIP_BOTTOM;}

	//
	// Now build the new OS_Vert.
	//

	ovn->colour   = OS_interpolate_colour(along, ov1->colour,   ov2->colour  );
	ovn->specular = OS_interpolate_colour(along, ov1->specular, ov2->specular);
	ovn->index    = NULL;
	ovn->trans    = OS_trans_upto;
	ovn->u1       = ov1->u1 + along * (ov2->u1 - ov1->u1);
	ovn->v1       = ov1->v1 + along * (ov2->v1 - ov1->v1);
	ovn->u2       = ov1->u2 + along * (ov2->u2 - ov1->u2);
	ovn->v2       = ov1->v2 + along * (ov2->v2 - ov1->v2);

	OS_trans_upto += 1;
}

float OS_clip_helper_signed_distance(void *point)
{
	OS_Vert  *ov;
	OS_Trans *ot;
	
	ov = (OS_Vert *) point;

	ASSERT(WITHIN(ov->trans, 0, OS_MAX_TRANS - 1));

	ot = &OS_trans[ov->trans];

	return ot->z - OS_ZCLIP_PLANE;
}

//
// ========================================================



void OS_buffer_add_triangle(
		OS_Buffer *ob,
		OS_Vert   *ov1,
		OS_Vert   *ov2,
		OS_Vert   *ov3)
{
	OS_poly_count_transformed += 1;

	ULONG clip_and =
			OS_trans[ov1->trans].clip &
			OS_trans[ov2->trans].clip &
			OS_trans[ov3->trans].clip;
	
	if (clip_and & OS_CLIP_TRANSFORMED)
	{
		if (clip_and & OS_CLIP_OFFSCREEN)
		{
			//
			// The triangle is completely off-screen.
			// 

			return;
		}
		else
		{
			if (ov1->index == NULL) {OS_buffer_add_vert(ob, ov1);}
			if (ov2->index == NULL) {OS_buffer_add_vert(ob, ov2);}
			if (ov3->index == NULL) {OS_buffer_add_vert(ob, ov3);}

			//
			// Add this triangle. All the points are transformed and at least
			// one is on screen.
			//

			if (ob->num_indices + 3 > ob->max_indices)
			{
				//
				// Need a bigger buffer.
				//

				OS_buffer_grow_indices(ob);
			}

			ob->index[ob->num_indices++] = ov1->index;
			ob->index[ob->num_indices++] = ov2->index;
			ob->index[ob->num_indices++] = ov3->index;

			return;
		}
	}
	else
	{
		ULONG clip_or =
				OS_trans[ov1->trans].clip |
				OS_trans[ov2->trans].clip |
				OS_trans[ov3->trans].clip;

		if (clip_or & OS_CLIP_TRANSFORMED)
		{
			//
			// This triangle needs to be zclipped.
			//

			if (clip_or & OS_CLIP_FAR)
			{
				//
				// Reject triangles on the far clip plane.
				//
			}
			else
			{
				void  *clip_tri[3];
				void **clip_ans;
				SLONG  clip_num;

				clip_tri[0] = ov1;
				clip_tri[1] = ov2;
				clip_tri[2] = ov3;

				clip_ans = clip_tri;
				clip_num = 3;

				CLIP_do(
				   &clip_ans,
				   &clip_num,
					sizeof(OS_Vert),
					OS_clip_helper_interpolate,
					OS_clip_helper_signed_distance);

				ASSERT(
					clip_num == 3 ||
					clip_num == 4);

				//
				// Recurse... lazily!
				//

				ov1 = (OS_Vert *) ((clip_ans)[0]);
				ov2 = (OS_Vert *) ((clip_ans)[1]);
				ov3 = (OS_Vert *) ((clip_ans)[2]);

				ASSERT(WITHIN(ov1->trans, 0, OS_trans_upto - 1));
				ASSERT(WITHIN(ov2->trans, 0, OS_trans_upto - 1));
				ASSERT(WITHIN(ov3->trans, 0, OS_trans_upto - 1));

				{
					SLONG i;

					for (i = 0; i < clip_num; i++)
					{
						OS_Vert  *ov = (OS_Vert  *) (clip_ans[i]);
						OS_Trans *ot = &OS_trans[ov->trans];

						ASSERT(ot->clip & OS_CLIP_TRANSFORMED);
						ASSERT(ot->z >= OS_ZCLIP_PLANE);
					}
				}

				clip_and = 
					OS_trans[ov1->trans].clip &
					OS_trans[ov2->trans].clip &
					OS_trans[ov3->trans].clip;

				if (clip_and & OS_CLIP_OFFSCREEN)
				{
					//
					// The triangle is completely off-screen.
					// 
				}
				else
				{
					if (ov1->index == NULL) {OS_buffer_add_vert(ob, ov1);}
					if (ov2->index == NULL) {OS_buffer_add_vert(ob, ov2);}
					if (ov3->index == NULL) {OS_buffer_add_vert(ob, ov3);}

					//
					// Add this triangle. All the points are transformed and at least
					// one is on screen.
					//

					if (ob->num_indices + 3 > ob->max_indices)
					{
						//
						// Need a bigger buffer.
						//

						OS_buffer_grow_indices(ob);
					}

					ob->index[ob->num_indices++] = ov1->index;
					ob->index[ob->num_indices++] = ov2->index;
					ob->index[ob->num_indices++] = ov3->index;
				}

				if (clip_num == 4)
				{
					ov2 = (OS_Vert *) ((clip_ans)[2]);
					ov3 = (OS_Vert *) ((clip_ans)[3]);
					
					clip_and = 
						OS_trans[ov1->trans].clip &
						OS_trans[ov2->trans].clip &
						OS_trans[ov3->trans].clip;

					if (clip_and & OS_CLIP_OFFSCREEN)
					{
						//
						// The triangle is completely off-screen.
						// 
					}
					else
					{
						if (ov1->index == NULL) {OS_buffer_add_vert(ob, ov1);}
						if (ov2->index == NULL) {OS_buffer_add_vert(ob, ov2);}
						if (ov3->index == NULL) {OS_buffer_add_vert(ob, ov3);}

						//
						// Add this triangle. All the points are transformed and at least
						// one is on screen.
						//

						if (ob->num_indices + 3 > ob->max_indices)
						{
							//
							// Need a bigger buffer.
							//

							OS_buffer_grow_indices(ob);
						}

						ob->index[ob->num_indices++] = ov1->index;
						ob->index[ob->num_indices++] = ov2->index;
						ob->index[ob->num_indices++] = ov3->index;
					}
				}
			}

			return;
		}
		else
		{
			//
			// The whole triangle is zclipped one way or another. We assume that
			// a single triangle is not going to span both the near and far zclip
			// planes...
			//

			return;
		}
	}
}


void OS_buffer_add_sprite(
		OS_Buffer *ob,
		float x1,			// Normalised to 0.0F - 1.0F
		float y1,			// Normalised to 0.0F - 1.0F
		float x2,			// Normalised to 0.0F - 1.0F
		float y2,			// Normalised to 0.0F - 1.0F
		float u1, float v1,
		float u2, float v2,
		float z,
		ULONG colour,
		ULONG specular,
		ULONG fade)
{
	SLONG i;

	OS_Flert *of;

	//
	// Enough room in our buffer?
	//

	if (ob->num_indices + 6 > ob->max_indices) {OS_buffer_grow_indices(ob);}
	if (ob->num_flerts  + 4 > ob->max_flerts ) {OS_buffer_grow_flerts (ob);}

	//
	// Add four vertices.
	//
	
	for (i = 0; i < 4; i++)
	{
		of = &ob->flert[ob->num_flerts + i];

		of->sx       = ((i & 1) ? x1 : x2) * OS_screen_width;
		of->sy       = ((i & 2) ? y1 : y2) * OS_screen_height;
		of->sz       = z;
		of->rhw      = 0.5F;
		of->colour   = colour;
		of->specular = specular;
		of->tu1      = ((i & 1) ? u1 : u2);
		of->tv1      = ((i & 2) ? v1 : v2);
		of->tu2      = ((i & 1) ? u1 : u2);
		of->tv2      = ((i & 2) ? v1 : v2);
	}

	if (fade)
	{
		if (fade & OS_FADE_TOP)
		{
			ob->flert[ob->num_flerts + 2].colour = 0x00000000;
			ob->flert[ob->num_flerts + 3].colour = 0x00000000;
		}

		if (fade & OS_FADE_BOTTOM)
		{
			ob->flert[ob->num_flerts + 0].colour = 0x00000000;
			ob->flert[ob->num_flerts + 1].colour = 0x00000000;
		}

		if (fade & OS_FADE_LEFT)
		{
			ob->flert[ob->num_flerts + 1].colour = 0x00000000;
			ob->flert[ob->num_flerts + 3].colour = 0x00000000;
		}

		if (fade & OS_FADE_RIGHT)
		{
			ob->flert[ob->num_flerts + 0].colour = 0x00000000;
			ob->flert[ob->num_flerts + 2].colour = 0x00000000;
		}
	}

	//
	// Add two triangles.
	//

	ob->index[ob->num_indices + 0] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 1] = ob->num_flerts + 1;
	ob->index[ob->num_indices + 2] = ob->num_flerts + 3;

	ob->index[ob->num_indices + 3] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 4] = ob->num_flerts + 3;
	ob->index[ob->num_indices + 5] = ob->num_flerts + 2;

	ob->num_indices += 6;
	ob->num_flerts  += 4;
}



void OS_buffer_add_multitex_sprite(
		OS_Buffer *ob,
		float x1,			// Normalised to 0.0F - 1.0F
		float y1,			// Normalised to 0.0F - 1.0F
		float x2,			// Normalised to 0.0F - 1.0F
		float y2,			// Normalised to 0.0F - 1.0F
		float u11, float v11,
		float u12, float v12,
		float u21, float v21,
		float u22, float v22,
		float z,
		ULONG colour,
		ULONG specular,
		ULONG fade)
{
	SLONG i;

	OS_Flert *of;

	//
	// Enough room in our buffer?
	//

	if (ob->num_indices + 6 > ob->max_indices) {OS_buffer_grow_indices(ob);}
	if (ob->num_flerts  + 4 > ob->max_flerts ) {OS_buffer_grow_flerts (ob);}

	//
	// Add four vertices.
	//
	
	for (i = 0; i < 4; i++)
	{
		of = &ob->flert[ob->num_flerts + i];

		of->sx       = ((i & 1) ? x1 : x2) * OS_screen_width;
		of->sy       = ((i & 2) ? y1 : y2) * OS_screen_height;
		of->sz       = z;
		of->rhw      = 0.5F;
		of->colour   = colour;
		of->specular = specular;
		of->tu1      = ((i & 1) ? u11 : u12);
		of->tv1      = ((i & 2) ? v11 : v12);
		of->tu2      = ((i & 1) ? u21 : u22);
		of->tv2      = ((i & 2) ? v21 : v22);
	}

	if (fade)
	{
		if (fade & OS_FADE_TOP)
		{
			ob->flert[ob->num_flerts + 2].colour = 0x00000000;
			ob->flert[ob->num_flerts + 3].colour = 0x00000000;
		}

		if (fade & OS_FADE_BOTTOM)
		{
			ob->flert[ob->num_flerts + 0].colour = 0x00000000;
			ob->flert[ob->num_flerts + 1].colour = 0x00000000;
		}

		if (fade & OS_FADE_LEFT)
		{
			ob->flert[ob->num_flerts + 1].colour = 0x00000000;
			ob->flert[ob->num_flerts + 3].colour = 0x00000000;
		}

		if (fade & OS_FADE_RIGHT)
		{
			ob->flert[ob->num_flerts + 0].colour = 0x00000000;
			ob->flert[ob->num_flerts + 2].colour = 0x00000000;
		}
	}

	//
	// Add two triangles.
	//

	ob->index[ob->num_indices + 0] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 1] = ob->num_flerts + 1;
	ob->index[ob->num_indices + 2] = ob->num_flerts + 3;

	ob->index[ob->num_indices + 3] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 4] = ob->num_flerts + 3;
	ob->index[ob->num_indices + 5] = ob->num_flerts + 2;

	ob->num_indices += 6;
	ob->num_flerts  += 4;
}



void OS_buffer_add_sprite_rot(
		OS_Buffer *ob,
		float x_mid,
		float y_mid,
		float size,			// As a percentage of the width of the screen.
		float angle,
		float u1, float v1,
		float u2, float v2,
		float z,
		ULONG colour,
		ULONG specular,
		float tu1, float tv1,
		float tu2, float tv2)
{
	SLONG i;

	OS_Flert *of;

	float dx = sin(angle) * size;
	float dy = cos(angle) * size;

	float x;
	float y;
	
	x_mid *= OS_screen_width;
	y_mid *= OS_screen_height;

	//
	// Enough room in our buffer?
	//

	if (ob->num_indices + 6 > ob->max_indices) {OS_buffer_grow_indices(ob);}
	if (ob->num_flerts  + 4 > ob->max_flerts ) {OS_buffer_grow_flerts (ob);}

	//
	// Add four vertices.
	//
	
	for (i = 0; i < 4; i++)
	{
		of = &ob->flert[ob->num_flerts + i];
		
		x = 0.0F;
		y = 0.0F;

		if (i & 1)
		{
			x += dx;
			y += dy;
		}
		else
		{
			x -= dx;
			y -= dy;
		}

		if (i & 2)
		{
			x += -dy;
			y += +dx;
		}
		else
		{
			x -= -dy;
			y -= +dx;
		}

		x *= OS_screen_width;
		y *= OS_screen_height * 1.33F;

		x += x_mid;
		y += y_mid;

		of->sx       = x;
		of->sy       = y;
		of->sz       = z;
		of->rhw      = 0.5F;
		of->colour   = colour;
		of->specular = specular;
		of->tu1      = ((i & 1) ?  u1 :  u2);
		of->tv1      = ((i & 2) ?  v1 :  v2);
		of->tu2      = ((i & 1) ? tu1 : tu2);
		of->tv2      = ((i & 2) ? tv1 : tv2);
	}

	//
	// Add two triangles.
	//

	ob->index[ob->num_indices + 0] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 1] = ob->num_flerts + 1;
	ob->index[ob->num_indices + 2] = ob->num_flerts + 3;

	ob->index[ob->num_indices + 3] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 4] = ob->num_flerts + 3;
	ob->index[ob->num_indices + 5] = ob->num_flerts + 2;

	ob->num_indices += 6;
	ob->num_flerts  += 4;
}


void OS_buffer_add_sprite_arbitrary(
		OS_Buffer *ob,
		float x1,			// Normalised to 0.0F - 1.0F
		float y1,			// Normalised to 0.0F - 1.0F
		float x2,			// Normalised to 0.0F - 1.0F
		float y2,			// Normalised to 0.0F - 1.0F
		float x3,			// Normalised to 0.0F - 1.0F
		float y3,			// Normalised to 0.0F - 1.0F
		float x4,			// Normalised to 0.0F - 1.0F
		float y4,			// Normalised to 0.0F - 1.0F
		float u1, float v1,
		float u2, float v2,
		float u3, float v3,
		float u4, float v4,
		float z,
		ULONG colour,
		ULONG specular)
{
	OS_buffer_add_sprite_arbitrary(
		ob,
		x1, y1,
		x2, y2,
		x3, y3,
		x4, y4,
		u1, v1,
		u2, v2,
		u3, v3,
		u4, v4,
		z,z,z,z,
		colour,
		specular);
}


void OS_buffer_add_sprite_arbitrary(
		OS_Buffer *ob,
		float x1,			// Normalised to 0.0F - 1.0F
		float y1,			// Normalised to 0.0F - 1.0F
		float x2,			// Normalised to 0.0F - 1.0F
		float y2,			// Normalised to 0.0F - 1.0F
		float x3,			// Normalised to 0.0F - 1.0F
		float y3,			// Normalised to 0.0F - 1.0F
		float x4,			// Normalised to 0.0F - 1.0F
		float y4,			// Normalised to 0.0F - 1.0F
		float u1, float v1,
		float u2, float v2,
		float u3, float v3,
		float u4, float v4,
		float z1,
		float z2,
		float z3,
		float z4,
		ULONG colour,
		ULONG specular)
{
	SLONG i;

	float x;
	float y;
	float u;
	float v;
	float z;

	OS_Flert *of;

	//
	// Enough room in our buffer?
	//

	if (ob->num_indices + 6 > ob->max_indices) {OS_buffer_grow_indices(ob);}
	if (ob->num_flerts  + 4 > ob->max_flerts ) {OS_buffer_grow_flerts (ob);}

	//
	// Add four vertices.
	//
	
	for (i = 0; i < 4; i++)
	{
		of = &ob->flert[ob->num_flerts + i];

		switch(i)
		{
			case 0:
				x = x1;
				y = y1;
				u = u1;
				v = v1;
				z = z1;
				break;

			case 1:
				x = x2;
				y = y2;
				u = u2;
				v = v2;
				z = z2;
				break;

			case 2:
				x = x3;
				y = y3;
				u = u3;
				v = v3;
				z = z3;
				break;

			case 3:
				x = x4;
				y = y4;
				u = u4;
				v = v4;
				z = z4;
				break;
			
			default:
				ASSERT(0);
				break;
		}		

		x *= OS_screen_width;
		y *= OS_screen_height;

		of->sx       = x;
		of->sy       = y;
		of->sz       = z;
		of->rhw      = 0.5F;
		of->colour   = colour;
		of->specular = specular;
		of->tu1      = u;
		of->tv1      = v;
		of->tu2      = u;
		of->tv2      = v;
	}

	//
	// Add two triangles.
	//

	ob->index[ob->num_indices + 0] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 1] = ob->num_flerts + 1;
	ob->index[ob->num_indices + 2] = ob->num_flerts + 3;

	ob->index[ob->num_indices + 3] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 4] = ob->num_flerts + 3;
	ob->index[ob->num_indices + 5] = ob->num_flerts + 2;

	ob->num_indices += 6;
	ob->num_flerts  += 4;
}


void OS_buffer_add_line_3d(
		OS_Buffer *ob,
		float X1,
		float Y1,
		float X2,
		float Y2,
		float width,
		float u1, float v1,
		float u2, float v2,
		float z1,
		float z2,
		ULONG colour,
		ULONG specular)
{
	SLONG i;

	OS_Flert *of;

	float dx;
	float dy;
	float len;
	float overlen;

	float x;
	float y;
	
	//
	// Enough room in our buffer?
	//

	if (ob->num_indices + 6 > ob->max_indices) {OS_buffer_grow_indices(ob);}
	if (ob->num_flerts  + 4 > ob->max_flerts ) {OS_buffer_grow_flerts (ob);}

	//
	// The width of the line.
	//
	
	dx = X2 - X1;
	dy = Y2 - Y1;

	len     = qdist2(fabs(dx),fabs(dy));
	overlen = width * OS_screen_width / len;

	dx *= overlen;
	dy *= overlen;

	//
	// Add four vertices.
	//
	
	for (i = 0; i < 4; i++)
	{
		of = &ob->flert[ob->num_flerts + i];
		
		x = 0.0F;
		y = 0.0F;

		if (i & 1)
		{
			x += -dy;
			y += +dx;
		}
		else
		{
			x -= -dy;
			y -= +dx;
		}

		if (i & 2)
		{
			x += X1;
			y += Y1;
		}
		else
		{
			x += X2;
			y += Y2;
		}

		of->sx       = x;
		of->sy       = y;
		of->sz       = ((i & 2) ? z1 : z2);
		of->rhw      = 0.5F;
		of->colour   = colour;
		of->specular = specular;
		of->tu1      = ((i & 1) ? u1 : u2);
		of->tv1      = ((i & 2) ? v1 : v2);
		of->tu2      = ((i & 1) ? u1 : u2);
		of->tv2      = ((i & 2) ? v1 : v2);
	}

	//
	// Add two triangles.
	//

	ob->index[ob->num_indices + 0] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 1] = ob->num_flerts + 1;
	ob->index[ob->num_indices + 2] = ob->num_flerts + 3;

	ob->index[ob->num_indices + 3] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 4] = ob->num_flerts + 3;
	ob->index[ob->num_indices + 5] = ob->num_flerts + 2;

	ob->num_indices += 6;
	ob->num_flerts  += 4;
}




void OS_buffer_add_line_2d(
		OS_Buffer *ob,
		float x1,
		float y1,
		float x2,
		float y2,
		float width,
		float u1, float v1,
		float u2, float v2,
		float z,
		ULONG colour,
		ULONG specular)
{
	SLONG i;

	OS_Flert *of;

	float dx;
	float dy;
	float len;
	float overlen;

	float x;
	float y;
	
	//
	// Enough room in our buffer?
	//

	if (ob->num_indices + 6 > ob->max_indices) {OS_buffer_grow_indices(ob);}
	if (ob->num_flerts  + 4 > ob->max_flerts ) {OS_buffer_grow_flerts (ob);}

	//
	// The width of the line.
	//
	
	x1 *= OS_screen_width;
	y1 *= OS_screen_height;

	x2 *= OS_screen_width;
	y2 *= OS_screen_height;

	dx = x2 - x1;
	dy = y2 - y1;

	len     = qdist2(fabs(dx),fabs(dy));
	overlen = width * OS_screen_width / len;

	dx *= overlen;
	dy *= overlen;

	//
	// Add four vertices.
	//
	
	for (i = 0; i < 4; i++)
	{
		of = &ob->flert[ob->num_flerts + i];
		
		x = 0.0F;
		y = 0.0F;

		if (i & 1)
		{
			x += -dy;
			y += +dx;
		}
		else
		{
			x -= -dy;
			y -= +dx;
		}

		if (i & 2)
		{
			x += x1;
			y += y1;
		}
		else
		{
			x += x2;
			y += y2;
		}

		of->sx       = x;
		of->sy       = y;
		of->sz       = z;
		of->rhw      = 0.5F;
		of->colour   = colour;
		of->specular = specular;
		of->tu1      = ((i & 1) ? u1 : u2);
		of->tv1      = ((i & 2) ? v1 : v2);
		of->tu2      = ((i & 1) ? u1 : u2);
		of->tv2      = ((i & 2) ? v1 : v2);
	}

	//
	// Add two triangles.
	//

	ob->index[ob->num_indices + 0] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 1] = ob->num_flerts + 1;
	ob->index[ob->num_indices + 2] = ob->num_flerts + 3;

	ob->index[ob->num_indices + 3] = ob->num_flerts + 0;
	ob->index[ob->num_indices + 4] = ob->num_flerts + 3;
	ob->index[ob->num_indices + 5] = ob->num_flerts + 2;

	ob->num_indices += 6;
	ob->num_flerts  += 4;
}



void OS_buffer_draw(
		OS_Buffer  *ob,
		OS_Texture *ot1,
		OS_Texture *ot2,
		ULONG       draw)
{
	LPDIRECT3DDEVICE3 d3d = OS_frame.GetD3DDevice();

	if (ob->num_flerts == 0)
	{
		//
		// Empty buffer!
		//

		OS_buffer_give(ob);

		return;
	}

	if (!OS_in_scene)
	{
		OS_scene_begin();
	}

	if (ot1 == NULL)
	{
		//
		// No texturing.
		//

		draw |= OS_DRAW_TEX_NONE;
	}
	else
	{
		//
		// Make this texture the input into the pipeline.
		//
		
		d3d->SetTexture(0, ot1->ddtx);
	}

	if (ot2)
	{
		//
		// Make this texture the input into the second stage of the pipeline.
		//

		d3d->SetTexture(1, ot2->ddtx);
	}

	//
	// Update renderstates.
	//

	if (draw != OS_DRAW_NORMAL)
	{
		OS_change_renderstate_for_type(draw);
	}

	{
		//
		// Check that this will be okay.
		//
	
		ULONG num_passes;

		if (d3d->ValidateDevice(&num_passes) != D3D_OK)
		{
			OS_string("Validation failed: draw = 0x%x\n", draw);
		}
	}

	//
	// The number of polys actually drawn.
	//
	
	OS_poly_count_drawn += ob->num_indices / 3;

	//
	// Draw the triangles.
	//

	d3d->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST,
			OS_FLERT_FORMAT,
			ob->flert,
			ob->num_flerts,
			ob->index,
			ob->num_indices,
			D3DDP_DONOTUPDATEEXTENTS);

	//
	// Put state back to normal.
	//

	if (draw != OS_DRAW_NORMAL)
	{
		OS_undo_renderstate_type_changes();
	}

	//
	// Returns the buffer to the free list.
	//

	OS_buffer_give(ob);
}


void OS_screenshot(CBYTE *fname)
{
	SLONG r;
	SLONG g;
	SLONG b;

	SLONG x;
	SLONG y;

	ULONG  pixel;
	UWORD *u_screen;
	SLONG  pitch;
	ULONG *l_screen;

	CBYTE ourname[128];

	SLONG width  = ftol(OS_screen_width );
	SLONG height = ftol(OS_screen_height);

	TGA_Pixel *tga;

	DDSURFACEDESC2 ddsd;

	tga = (TGA_Pixel *) malloc(sizeof(TGA_Pixel) * width * height);

	if (!tga)
	{
		return;
	}

	//
	// Lock the backbuffer.
	//

	memset(&ddsd, 0, sizeof(ddsd));
		 
	ddsd.dwSize = sizeof(ddsd);

	OS_frame.GetBackBuffer()->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

	//
	// Screen format.
	//

	SLONG mask_r;
	SLONG mask_g;
	SLONG mask_b;

	SLONG shift_r;
	SLONG shift_g;
	SLONG shift_b;

	OS_calculate_mask_and_shift(ddsd.ddpfPixelFormat.dwRBitMask, &mask_r, &shift_r);
	OS_calculate_mask_and_shift(ddsd.ddpfPixelFormat.dwGBitMask, &mask_g, &shift_g);
	OS_calculate_mask_and_shift(ddsd.ddpfPixelFormat.dwBBitMask, &mask_b, &shift_b);

	//
	// How many bpp?
	//

	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		u_screen = (UWORD *) ddsd.lpSurface;
		l_screen = NULL;
		pitch    = ddsd.lPitch >> 1;
	}
	else
	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		u_screen = NULL;
		l_screen = (ULONG *) ddsd.lpSurface;
		pitch    = ddsd.lPitch >> 2;
	}
	else
	{
		return;
	}

	//
	// Transfer pixels to buffer.
	//

	for (x = 0; x < width;  x++)
	for (y = 0; y < height; y++)
	{
		if (u_screen)
		{
			pixel = u_screen[x + y * pitch];
		}
		else
		{
			pixel = l_screen[x + y * pitch];
		}

		r = ((pixel >> shift_r) << mask_r) & 0xff;
		g = ((pixel >> shift_g) << mask_g) & 0xff;
		b = ((pixel >> shift_b) << mask_b) & 0xff;

		tga[x + y * width].red   = r;
		tga[x + y * width].green = g;
		tga[x + y * width].blue  = b;
		tga[x + y * width].alpha = 0;
	}

	//
	// Unlock screen.
	//
	
	OS_frame.GetBackBuffer()->Unlock(NULL);

	//
	// Get the filename.
	//

	if (!fname)
	{
		SLONG i;

		for (i = 1; i < 1000; i++)
		{
			sprintf(ourname, "ScreenShot\\shot%03d.tga", i);

			if (fopen(ourname, "rb") == NULL)
			{
				//
				// Found an unused filename.
				//

				fname = ourname;

				goto found_name;
			}
		}

		//
		// Could not get a filename.
		//

		return;

	  found_name:;
	}

	TGA_save(
		fname,
		width,
		height,
		tga,
		FALSE);

	//
	// Free up tga memory.
	//

	free(tga);
}






