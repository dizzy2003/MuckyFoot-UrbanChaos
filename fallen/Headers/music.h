//
// music controller thingy
// matthew rosenfeld
// 14 june 99
//

#include "mfstdlib.h"

/*
#define	MUSIC_FLAG_FADE_IN	1
#define MUSIC_FLAG_FADE_OUT	2
#define MUSIC_FLAG_LOOPED	4
#define MUSIC_FLAG_QUEUED	8
#define MUSIC_FLAG_OVERRIDE 16

#define MUSIC_WAS_FUCKED	0
#define MUSIC_WAS_PLAYED	1
#define MUSIC_WAS_QUEUED	2
#define MUSIC_WAS_FADED		3

// play or queue a piece according to flags
// return value is a MUSIC_WAS flag
UBYTE MUSIC_play(UWORD wave, UBYTE flags);

// stop current piece playing, optionally fading out
void  MUSIC_stop(BOOL fade);

// find out the currently playing wave
UWORD MUSIC_wave();

// call this each game loop to keep things fading in and out nicely and stuff
void  MUSIC_process();

*/
// this is the 'max' gain, fade in/out will go from/to 0 from/to this value
void  MUSIC_gain(UBYTE gain);


/**********************************************************************************
 *
 *      Experimental new music system that hopefully doesn't suck
 */

// Curiously, I typed these in the order they came to mind: And there they are, sorted into
// priority order...

#define MUSIC_MODE_SILENT			(0)
#define MUSIC_MODE_DRIVING			(1)
#define	MUSIC_MODE_SPRINTING		(2)
#define MUSIC_MODE_CRAWLING			(3)
#define MUSIC_MODE_FIGHTING			(4)
#define MUSIC_MODE_TRAIN_COMBAT		(5)
#define MUSIC_MODE_TRAIN_DRIVING	(6)
#define MUSIC_MODE_TRAIN_ASSAULT	(7)
#define MUSIC_MODE_FINAL_RECKONING  (8)
#define MUSIC_MODE_TIMER			(9)
#define MUSIC_MODE_GAMELOST			(10)
#define MUSIC_MODE_GAMEWON 			(11)
#define MUSIC_MODE_BRIEFING			(12)
#define MUSIC_MODE_FRONTEND			(13)
#define MUSIC_MODE_CHAOS			(14)
#define MUSIC_MODE_FORCE   			(128)

void MUSIC_mode(UBYTE mode);
void MUSIC_mode_process();
void MUSIC_reset();

extern UBYTE music_mode_override;
extern UBYTE MUSIC_bodge_code;

extern SLONG MUSIC_is_playing(void);


/**********************************************************************************/


/*

  Looks like we need a few more features to the sound system for Music

 Overall Music Volume, Overall Sfx Volume   (music being most important at the moment its drowning out all sfx)

Need to be able to fade out a piece, (i.e fade out current and queue another to start when music has reached 0 volume )

Loop Music, so no queued music will play unless the current music is faded out

Stop Music (a variation on fade out I suppose)


Fade in a piece of music, queued piece fades in.

Enquirey function  which sample is playing on the music channel,



+ More currently unknown functions


*/