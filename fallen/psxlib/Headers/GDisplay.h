// Display.h
// Guy Simmons, 13th November 1997.

#ifndef	DISPLAY_H
#define	DISPLAY_H


#include "libgpu.h"
#include "libgte.h"

//---------------------------------------------------------------

#define	SHELL_ACTIVE			(LibShellActive())
#define	SHELL_CHANGED			(LibShellChanged())

//---------------------------------------------------------------

#define	DEFAULT_WIDTH			(320)
#define	DEFAULT_HEIGHT			(240)

//---------------------------------------------------------------

SLONG			OpenDisplay(ULONG width, ULONG height, ULONG depth, ULONG flags);
SLONG			CloseDisplay(void);
SLONG			SetDisplay(ULONG width,ULONG height,ULONG depth);
void			ShellPaused(void);
void			ShellPauseOn(void);
void			ShellPauseOff(void);
void			DumpBackToTGA(CBYTE *tga_name);
void			DumpBackToRaw(void);	


//---------------------------------------------------------------

#define	DISPLAY_INIT		(1<<0)
#define	DISPLAY_PAUSE		(1<<1)
#define	DISPLAY_PAUSE_ACK	(1<<2)
#define DISPLAY_LOCKED		(1<<3)

#define	BK_COL_NONE			0
#define	BK_COL_BLACK		1
#define	BK_COL_WHITE		2
#define	BK_COL_USER			3


#define OTLEN	10  // 4096 buckets
#define OTSIZE	(1<<OTLEN)
#define PANEL_OTZ	(OTSIZE-1)
#define FLARE_OTZ	(OTSIZE-2)

extern	UWORD	psx_tpages[22];
extern	UWORD	psx_tpages_clut[16];

#define	SCREEN_WIDTH	512
#define	SCREEN_HEIGHT	250



// I've reduced bucket memory to a mear 64K instead of 80K 
// I've checked and found this to be a reasonable size given the
// usage on Botanic is only 54836bytes.

//#define BUCKET_MEM	81920
//#define BUCKET_MEM	(72*1024)

typedef struct {		
	DRAWENV		Draw;			/* drawing environment */
	DISPENV		Disp;			/* display environment */
	ULONG		*ot;			/* ordering table */
	UBYTE		*PrimMem;		/* Bucket Memory Try using one lot. */ //[BUCKET_MEM];
} DB;



typedef	struct
{

		UBYTE					*CurrentPrim;
		DB						DisplayBuffers[2];		/* packet double buffer */
		DB						*CurrentDisplayBuffer;


		ULONG					BackColour,
								PaletteSize;
	    RECT					DisplayRect;				// Current surface rectangle.

		SLONG					screen_width;
		SLONG                   screen_height;
		SLONG                   screen_pitch;
		ULONG					Max_Used;				/* Maximum bucket memory used */


}Display;

//---------------------------------------------------------------

extern SLONG			DisplayWidth,
						DisplayHeight,
						DisplayBPP;
extern Display			the_display;




//---------------------------------------------------------------

#endif

