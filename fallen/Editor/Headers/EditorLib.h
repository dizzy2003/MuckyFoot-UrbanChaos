// EditorLib.h
// Guy Simmons, 10th December 1997.

#ifndef	EDITOR_LIB_H
#define	EDITOR_LIB_H

#include	<MFStdLib.h>
#include	"DDLib.h"

void	Time(struct MFTime *the_time);

//---------------------------------------------------------------
// Draw2D

extern UBYTE			*CharTable[];
extern UBYTE			*WorkWindow;
extern SLONG			WorkWindowHeight,
	 					WorkWindowWidth;
extern MFRect			WorkWindowRect;

void			SetWorkWindowBounds(SLONG left, SLONG top, SLONG width, SLONG height);
MFPoint			*GlobalToLocal(MFPoint *the_point);
void			GlobalXYToLocal(SLONG *x,SLONG *y);

extern	void	(*DrawBox)(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void	(*DrawBoxC)(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);

extern	void	DrawCircle(SLONG x,SLONG y,SLONG radius,ULONG colour);
extern	void	DrawCircleC(SLONG x,SLONG y,SLONG radius,ULONG colour);

extern	void	(*DrawLine)(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	(*DrawLineC)(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	(*DrawHLine)(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	(*DrawHLineC)(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	(*DrawVLine)(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void	(*DrawVLineC)(SLONG x,SLONG y1,SLONG y2,ULONG colour);

extern	void	(*DrawPoint)(MFPoint *the_point,ULONG colour);
extern	void	(*DrawPointC)(MFPoint *the_point,ULONG colour);

extern	void	(*DrawPixel)(SLONG x,SLONG y,ULONG colour);
extern	void	(*DrawPixelC)(SLONG x,SLONG y,ULONG colour);

extern	void	DrawRect(MFRect *the_rect,ULONG colour);
extern	void	DrawRectC(MFRect *the_rect,ULONG colour);

extern	void	(*QuickText)(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void	(*QuickTextC)(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void	(*QuickChar)(SLONG x,SLONG y,CBYTE the_char,ULONG colour);
extern	void	(*QuickCharC)(SLONG x,SLONG y,CBYTE the_char,ULONG colour);

SLONG			QTStringWidth(CBYTE *the_string);
inline SLONG	QTStringHeight(void)				{	return 8;	}
inline SLONG	QTCharWidth(CBYTE the_char)			{	return (CharTable[the_char])[0];	}
inline SLONG	QTCharHeight(CBYTE the_char)		{	return (CharTable[the_char])[1];	}

void	SetDrawFunctions(ULONG depth);
void	ShowWorkWindow(ULONG flags);

//---------------------------------------------------------------
// Palette

#define	FADE_IN			1<<0
#define	FADE_OUT		1<<1

extern UBYTE					CurrentPalette[256*3];

void	InitPalettes(void);
SLONG	CreatePalettes(void);
void	DestroyPalettes(void);
void	RestorePalettes(void);
void	SetPalette(UBYTE *the_palette);
SLONG	FindColour(UBYTE *pal,SLONG r,SLONG g,SLONG b);

//---------------------------------------------------------------
// Sprites

#define	END_LINE				0x00
#define	COPY_PIXELS				0x01
#define	SKIP_PIXELS				0x02
#define	DUPLICATE_PIXELS		0x03
#define	FINISHED				0x04

typedef struct
{
	UBYTE		*SpriteData;
	UWORD		SpriteHeight;
	UWORD		SpriteWidth;
}BSprite;


extern	void	(*DrawBSprite)(SLONG x,SLONG y,BSprite *the_sprite);
extern	void	(*DrawBSpriteC)(SLONG x,SLONG y,BSprite *the_sprite);
extern	void	(*DrawMonoBSprite)(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
extern	void	(*DrawMonoBSpriteC)(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);

void	DrawBSpritePal16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
void	DrawBSpritePal32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
void	DrawBSpritePalC16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
void	DrawBSpritePalC32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
void	SetupBSprites(BSprite *sprite_ref,UBYTE *sprite_data);

//---------------------------------------------------------------

#endif

