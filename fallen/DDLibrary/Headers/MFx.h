//
// MFx.h
//
// Muckyfoot sound fx api for A3D / PSX
//
// (same header, different cpp)
//
#ifndef _mfx_h_
#define _mfx_h_


#include "MFStdLib.h"
#include "structs.h"
#include "thing.h"

// set this to enable forcing the PC to only play FX available on the PSX
#define DODGYPSXIFY 1
// with this clear, the code to do it is taken out completely, for speed
// with this set, the code will look in the config.ini to see whether or not to PSXify sound.


#define		MFX_LOOPED		(1)				// loop the wave infinitely
#define		MFX_MOVING		(2)				// update the source's coords automatically
#define		MFX_REPLACE		(4)				// play, replacing whatever's there and blatting the queue
#define		MFX_OVERLAP		(8)				// allocate a new channel if current is occupied
#define		MFX_QUEUED		(16)			// queue this up for later
#define		MFX_CAMERA		(32)			// attach to camera
#define		MFX_HI_PRIORITY	(64)			// these are for A3d not for us as they...
#define		MFX_LO_PRIORITY	(128)			// ...determine priority for funky HW channels
#define		MFX_LOCKY		(256)			// don't adjust Y coord when cam-ing or move-ing
#define		MFX_SHORT_QUEUE (512)			// for sync-ing music -- queue for later, but replace the top of the queue
#define		MFX_PAIRED_TRK1 (1024)			// when this chan starts playing, trigger the next chan in sync
#define		MFX_PAIRED_TRK2 (2048)			// don't start this wave until the previous chan triggers it
#define		MFX_EARLY_OUT	(4096)			// mostly for music, 'breaks out' early from the wave, also works on loops
#define		MFX_NEVER_OVERLAP (8192)		// if anything at all playing on the channel, leave it

#define		MFX_ENV_NONE	(0)
#define		MFX_ENV_ALLEY	(1)
#define		MFX_ENV_SEWER	(2)
#define		MFX_ENV_STADIUM	(3)

#define		MFX_CHANNEL_ALL		(0x010000)
#define		MFX_WAVE_ALL		(0x010000)
// Excludes the memstream sounds.
#define		MFX_WAVE_ALMOST_ALL	(0x010001)

//----- volume functions

void	MFX_get_volumes(SLONG* fx, SLONG* amb, SLONG* mus);	// all 0 to 127
void	MFX_set_volumes(SLONG fx, SLONG amb, SLONG mus);

//----- transport functions -----

void	MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z);
//void	MFX_play_pos(UWORD channel_id, ULONG wave, ULONG flags, GameCoord* position);
void	MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p);
void	MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags);
UBYTE	MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags);

void	MFX_stop(SLONG channel_id, ULONG wave);
void	MFX_stop_attached(Thing *p);

//----- audio processing functions -----

void	MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend);
void	MFX_set_gain(UWORD channel_id, ULONG wave, UBYTE gain);
void	MFX_set_queue_gain(UWORD channel_id, ULONG wave, UBYTE gain);
//void	MFX_set_wave(UWORD channel_id, ULONG wave, ULONG new_wave);
//void	MFX_set_xyz(UWORD channel_id, ULONG wave, SLONG x, SLONG y, SLONG z);
//void	MFX_set_pos(UWORD channel_id, ULONG wave, GameCoord* position);
//void	MFX_set_channel_gain(UWORD channel_id, UBYTE gain);

//----- listener & environment -----

void	MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch);
//void	MFX_set_environment(SLONG env_type);

//----- sound library functions -----

//void	MFX_load_wave_list(CBYTE *path,CBYTE *script_file); // load list from a text file
void	MFX_load_wave_list(CBYTE *names[]=0);				// load list from array
//void	MFX_load_wave_file(CBYTE *wave_file);				// load file
void	MFX_free_wave_list();								// free list

//----- querying information back -----

UWORD	MFX_get_wave(UWORD channel_id, UBYTE index=0);

//----- general system stuff -----

void	MFX_render ( void );

//----- init stuff (why the fuck there isn't MFX_init() and MFX_term() I'll never understand.  Someone, somewhere in this company should take a dictionary out and look up the word "design".  It's under D.)

void	MilesInit(HINSTANCE hInstance, HWND hWnd);
void	MilesTerm();


// Mikes bodge stuff to get conversation in

SLONG	MFX_QUICK_play(CBYTE *str, SLONG loop, SLONG x=0, SLONG y=0, SLONG z=0);	// should be low-res (24.8) coordinates
void	MFX_QUICK_wait(void);
void	MFX_QUICK_stop ( bool bAllowMemstream = FALSE );
SLONG	MFX_QUICK_still_playing(void);

#endif
