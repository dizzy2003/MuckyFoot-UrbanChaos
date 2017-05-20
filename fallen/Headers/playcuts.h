//
// playcuts.h
// matthew rosenfeld 30 june 99
//
// plays back cutscenes made with cutscene.cpp in gedit
//

#ifndef _PLAYCUTS_H_
#define _PLAYCUTS_H_


#include "MFStdLib.h"
#include "Game.h"

//----------------------------------------------------------------------------
// DEFINES
//

#ifdef TARGET_DC
#define MAX_CUTSCENES			(1)
#define MAX_CUTSCENE_TRACKS		(1)
#define MAX_CUTSCENE_PACKETS	(1)
#define MAX_CUTSCENE_TEXT		(1)
#else
#define MAX_CUTSCENES			(20)
#define MAX_CUTSCENE_TRACKS		(20*15)
#define MAX_CUTSCENE_PACKETS	(20*15*128)
#define MAX_CUTSCENE_TEXT		(4096)
#endif

//----------------------------------------------------------------------------
// STRUCTS
//

struct CPData;
struct CPChannel;
struct CPPacket;

struct CPData {
	UBYTE			 version;
	UBYTE			 channelcount;
	UBYTE			 pad1, pad2;
	CPChannel		*channels;
};

struct CPChannel {
	UBYTE		 type;			// 0=unused, 1=character, 2=camera, 3=spot sound, 4=vfx
	UBYTE		 flags;			// come up with some later... :P
	UBYTE		 pad1,pad2;
	UWORD		 index;			// of the sound/character or the type of fx
	UWORD		 packetcount;	//
	CPPacket	*packets;		//
};

struct CPPacket {
	UBYTE		type;		// 0=unused, 1=animation, 2=action, 3=sound, 4=camerarec		0
	UBYTE		flags;		// come up with some later... :P
	UWORD		index;		// of animation, sound etc										4
	UWORD		start;		// time of packet start
	UWORD		length;		// natural packet length										8
	GameCoord	pos;		// location														20
	UWORD		angle,pitch;// no roll :P
};

//----------------------------------------------------------------------------
// EXTERNS (for the benefit of others)
//

extern UWORD		PLAYCUTS_cutscene_ctr;
extern UWORD		PLAYCUTS_track_ctr;
extern UWORD		PLAYCUTS_packet_ctr;
extern UWORD		PLAYCUTS_text_ctr;
#ifndef PSX
extern CPData		PLAYCUTS_cutscenes[MAX_CUTSCENES];
extern CPPacket		PLAYCUTS_packets[MAX_CUTSCENE_PACKETS];
extern CPChannel	PLAYCUTS_tracks[MAX_CUTSCENE_TRACKS];
extern CBYTE		PLAYCUTS_text_data[MAX_CUTSCENE_TEXT];
#else
extern CPData		*PLAYCUTS_cutscenes;
extern CPPacket		*PLAYCUTS_packets;
extern CPChannel	*PLAYCUTS_tracks;
extern CBYTE		*PLAYCUTS_text_data;
#endif

//----------------------------------------------------------------------------
// FUNCTION PROTOYPES
//

CPData* PLAYCUTS_Read(MFFileHandle handle);
void PLAYCUTS_Free(CPData *cutscene);
void PLAYCUTS_Play(CPData *cutscene);
void PLAYCUTS_Reset();



#endif