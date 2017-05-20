//
// tracks.h
// tyre tracks, footprints...
// 22 sept 98
//

#ifndef _TRACKS_H_
#define _TRACKS_H_

#include "MFStdLib.h"
#include "Structs.h"
#include "game.h"

// this is how many track slots are available on the given system
// using BUILD_PSX means that it'll be 50 both on the PSX, *AND* when Mike builds PSX
// nads on his PC.

#ifndef BUILD_PSX
#define TRACK_BUFFER_LENGTH		300
#else
#define TRACK_BUFFER_LENGTH		50
#endif



#define TRACK_SURFACE_NONE			0
#define	TRACK_SURFACE_MUDDY			1
#define	TRACK_SURFACE_WATER			2
#define	TRACK_SURFACE_ONSNOW		3

#define TRACK_TYPE_TYRE				1
#define TRACK_TYPE_TYRE_SKID		2
#define	TRACK_TYPE_LEFT_PRINT		3
#define	TRACK_TYPE_RIGHT_PRINT		4

#define	TRACK_FLAGS_FLIPABLE		1
#define	TRACK_FLAGS_SPLUTTING		2
#define TRACK_FLAGS_INVALPHA		4

struct Track {
//	SLONG	x,y,z; // not required -- here to debug...

	SLONG	dx,dy,dz;
	SLONG	page,colour;
	THING_INDEX	thing; //Thing*	thing; //miked did this
	SWORD	sx,sz;
	UWORD	padtolong;
	UBYTE	flip;
	UBYTE	flags;
	UBYTE	splut;
	UBYTE	splutmax;
};

typedef Track* TrackPtr;

extern	Track	*tracks;//[TRACK_BUFFER_LENGTH];
extern	UWORD	track_head,track_tail,track_eob;

#define	TO_TRACK(x)		(&tracks[x])
#define	TRACK_NUMBER(x)	(UWORD)(x-TO_TRACK(0))



void TRACKS_InitOnce(SWORD size=TRACK_BUFFER_LENGTH);
void TRACKS_Reset(SWORD size=TRACK_BUFFER_LENGTH);
void TRACKS_Draw();
void TRACKS_DrawTrack(Thing *p_thing);

// Figure out the offsets given the width
void TRACKS_CalcDiffs(Track &track, UBYTE width);

// Add a track unit supplying exact parameters one by one
void TRACKS_AddQuad(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, SLONG page, SLONG colour, UBYTE width, UBYTE flip, UBYTE flags);

// Add a track unit supplying a completed track entry
void TRACKS_AddTrack(Track &track);

// Add a track unit "intelligently" supplying coordinates and a type of track
UWORD TRACKS_Add(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, UBYTE type, UWORD last);

// What grounds is at XZ?
SLONG TRACKS_GroundAtXZ(SLONG X, SLONG Z);

// Make something bleed
void TRACKS_Bleed(Thing *bleeder);
void TRACKS_Bloodpool(Thing *bleeder);

#endif