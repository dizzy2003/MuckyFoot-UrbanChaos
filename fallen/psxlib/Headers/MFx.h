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
#include "c:\fallen\headers\structs.h"
#include "c:\fallen\headers\thing.h"

#define		MFX_LOOPED		(1)				// loop the wave infinitely
#define		MFX_MOVING		(2)				// update the source's coords automatically
#define		MFX_REPLACE		(4)				// play, replacing whatever's there and blatting the queue
#define		MFX_OVERLAP		(8)				// allocate a new channel if current is occupied
#define		MFX_QUEUED		(16)			// queue this up for later
#define		MFX_CAMERA		(32)			// attach to camera
#define		MFX_HI_PRIORITY	(64)			// these are for A3d not for us as they...
#define		MFX_LO_PRIORITY	(128)			// ...determine priority for funky HW channels
#define		MFX_LOCKY		(256)			// don't adjust Y coord when cam-ing or move-ing

#define		MFX_SHOT		(512)
#define		MFX_EXPLODE		(1024)
#define		MFX_PLAYER		(2048)
#define		MFX_EARLY_OUT	(4096)
#define		MFX_SHORT_QUEUE	(8192)
#define		MFX_NEVER_OVERLAP (16384)

#define		MFX_ENV_NONE	(0)
#define		MFX_ENV_ALLEY	(1)
#define		MFX_ENV_SEWER	(2)
#define		MFX_ENV_STADIUM	(3)

#define		MFX_CHANNEL_ALL	(0x010000)
#define		MFX_WAVE_ALL	(0x010000)

#define		MFX_PAIRED_TRK1	(1)
#define		MFX_PAIRED_TRK2	(2)

//----- transport functions -----

void	MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z);
void	MFX_play_pos(UWORD channel_id, ULONG wave, ULONG flags, GameCoord* position);
void	MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p);
void	MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags);
void	MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags);

void	MFX_stop(SLONG channel_id, ULONG wave);
void	MFX_stop_attached(Thing *p);

//----- audio processing functions -----

void	MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend);
void	MFX_set_wave(UWORD channel_id, ULONG wave, ULONG new_wave);
void	MFX_set_xyz(UWORD channel_id, ULONG wave, SLONG x, SLONG y, SLONG z);
void	MFX_set_pos(UWORD channel_id, ULONG wave, GameCoord* position);
void	MFX_set_gain(UWORD channel_id, ULONG wave, UBYTE gain);

//----- listener & environment -----

void	MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch);

void	MFX_set_environment(SLONG env_type);

//----- sound library functions -----

void	MFX_load_wave_list(CBYTE *path,CBYTE *script_file);
void	MFX_load_wave_list(CBYTE *names[]=0);				// load list from array
void	MFX_load_wave_file(CBYTE *wave_file);
void	MFX_free_wave_list();

//----- general system stuff -----

void	MFX_render();

//----- Here this must go, because nowhere else would be right for it.

void	MUSIC_init_level(SLONG world);

void MFX_Conv_wait();
SLONG MFX_Conv_play(SLONG waypoint,SLONG conv,SLONG conv_off);
void MFX_Init_Speech(SLONG level);

#endif
