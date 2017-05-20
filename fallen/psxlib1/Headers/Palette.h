// Palette.h
// Guy Simmons, 11th February 1997.

#ifndef	_PALETTE_H_
#define	_PALETTE_H_

#ifndef	_MF_TYPES_H_
	#include	<MFTypes.h>
#endif

#define	FADE_IN			1<<0
#define	FADE_OUT		1<<1

extern UBYTE					CurrentPalette[256*3];

void	InitPalettes(void);
SLONG	CreatePalettes(void);
void	DestroyPalettes(void);
void	RestorePalettes(void);
void	SetPalette(UBYTE *the_palette);
SLONG	FindColour(UBYTE *pal,SLONG r,SLONG g,SLONG b);


#endif
