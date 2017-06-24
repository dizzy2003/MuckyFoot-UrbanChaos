//	Sound.h
//	Guy Simmons, 8th May 1998.

#ifndef	SOUND_H
#define	SOUND_H

#include "Structs.h"
#include "MFX.h"

#if !defined(NO_SOUND) && defined(A3D_SOUND)
#define USE_A3D
#endif

// because the versions in MFStdLib.h are misspelt...
#define	WAVE_PLAY_INTERRUPT		0
#define	WAVE_PLAY_NO_INTERRUPT	1
#define	WAVE_PLAY_INTERRUPT_LOOPS 4

#define	WORLD_TYPE_CITY_POP		1
#define	WORLD_TYPE_CITY_UNPOP	2
#define	WORLD_TYPE_FOREST		3
#define	WORLD_TYPE_SNOW			4
#define	WORLD_TYPE_DESERT		5

extern	SWORD	world_type;

//---------------------------------------------------------------
/*
void	play_quicker_wave(SLONG sample);
void	play_quick_wave(Thing *p_thing,SLONG sample,SLONG mode);
SLONG	play_quick_wave_xyz(SLONG x,SLONG y,SLONG z,SLONG sample,SLONG id,SLONG mode);
SLONG	play_quick_wave_old(WaveParams *wave,SLONG sample,SLONG id,SLONG mode);
SLONG	play_ambient_wave(SLONG sample,SLONG id,SLONG mode,SLONG range=256, UBYTE flags=0);
SLONG	play_object_wave(SLONG channel, Thing *p_thing,SLONG sample,SLONG mode);
void	wave_move(SLONG channel, SLONG x, SLONG y, SLONG z);
SLONG	SOUND_query_current_wave(SLONG id);
BOOL	SOUND_query_looped(SLONG id);*/
void	play_glue_wave(UWORD type, UWORD id, SLONG x=0, SLONG y=0, SLONG z=0);
void	process_ambient_effects(void);
void	process_weather(void);

//void	NewLoadWaveList(CBYTE *names[]=0);

void	SOUND_reset();
void	SOUND_SewerPrecalc();
void	SOUND_InitFXGroups(CBYTE *fn);

void	PainSound(Thing *p_thing);
void	EffortSound(Thing *p_thing);
void	MinorEffortSound(Thing *p_thing);
void	ScreamFallSound(Thing *p_thing);
void	StopScreamFallSound(Thing *p_thing);
void	SOUND_Curious(Thing *p_thing);
UBYTE	SOUND_Gender(Thing *p_thing);


//SLONG	SOUND_Range(SLONG start, SLONG end);

#ifndef PSX

#ifdef DODGYPSXIFY
extern BOOL dodgy_psx_mode;
#endif

inline SLONG SOUND_Range(SLONG start, SLONG end) {
#ifdef DODGYPSXIFY
	if (dodgy_psx_mode) return start;
#endif
	SLONG diff=(end-start)+1;
	return start+(rand()%diff);
}

#else
#define SOUND_Range(start,end) (start)
#endif


#ifndef PSX

typedef	UWORD	SOUNDFXG[2];

extern  UBYTE *SOUND_FXMapping;//[1024]; // blahblah
extern  SOUNDFXG *SOUND_FXGroups;//[128][2]; // blahblah
#else
extern  UBYTE SOUND_FXMapping[512]; // blahblah
extern  UWORD SOUND_FXGroups[8][2]; // blahblah
#endif

//---------------------------------------------------------------
// pull in the automatically-generated sound header
#include "sound_id.h"

//---------------------------------------------------------------

#define	WIND_REF			MAX_THINGS+100
#define	WEATHER_REF			(WIND_REF+1)
#define	THUNDER_REF			(WIND_REF+2)
#define	SIREN_REF			(WIND_REF+3)
#define	AMBIENT_EFFECT_REF	(WIND_REF+4)
#define MUSIC_REF			(WIND_REF+5)

//---------------------------------------------------------------

#ifdef USE_A3D

//extern void	A3DLoadWaveList(CBYTE *path,CBYTE *file);
//extern void	A3DFreeWaveList(void);
//extern void A3DRender(void);
extern void A3D_Check_Init(void);

#endif


//
// Loads in sounds needed for just this level. Call after
// everything has already been loaded.
//

void SOUND_load_needed_sounds(void);




#endif
