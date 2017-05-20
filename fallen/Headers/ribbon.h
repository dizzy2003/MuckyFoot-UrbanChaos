//
// ribbon.h
// special effects: smoke, fire, wheelspray... all sorts
// matthew rosenfeld 10 nov 1998
//

#ifndef _RIBBON_H_
#define _RIBBON_H_

#include "MFStdLib.h"
#include "Structs.h"

#define MAX_RIBBONS			64
#define MAX_RIBBON_SIZE		32

#define RIBBON_FLAG_USED	1
#define RIBBON_FLAG_FADE	2
#define RIBBON_FLAG_SLIDE	4
#define	RIBBON_FLAG_DOUBLED	8
#define	RIBBON_FLAG_CONVECT	16
#define	RIBBON_FLAG_IALPHA	32


struct Ribbon {
	SLONG		Flags;
	SLONG		Page;			// POLY_PAGE
	SLONG		Life;			// -1 is forever
	SLONG		RGB;			// colour
	UBYTE		Size;			// Max number of points allowed
	UBYTE		Head;			// Where the next point will be added
	UBYTE		Tail;			// Where points will be eroded away from
	UBYTE		Scroll;			// Offset for looped textures
	UBYTE		FadePoint;		// Number of steps across which fadeout occurs
	UBYTE		SlideSpeed;		// Offset applied to Scroll
	UBYTE		TextureU;		// How many loops for width of strip
	UBYTE		TextureV;		// How many steps each loop applies to
	GameCoord	Points[MAX_RIBBON_SIZE];
};




void	RIBBON_init();
void	RIBBON_draw();
void	RIBBON_process();
SLONG	RIBBON_alloc(SLONG flags, UBYTE max_segments, SLONG page, SLONG life=-1, UBYTE fade=0, UBYTE scroll=0, UBYTE u=1, UBYTE v=0, SLONG rgb=0xFFFFFF);
void	RIBBON_free(SLONG ribbon);
void	RIBBON_extend(SLONG ribbon, SLONG x, SLONG y, SLONG z);
SLONG	RIBBON_length(SLONG ribbon);
void	RIBBON_life(SLONG ribbon, SLONG life);


#endif
