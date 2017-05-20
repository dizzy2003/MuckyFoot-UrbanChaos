// Draw2D.h
// Guy Simmons, 7th October 1996.

#ifndef _DRAW2D_H_
#define _DRAW2D_H_

#ifndef	_MF_TYPES_H_
	#include	<MFTypes.h>
#endif

#ifndef	_DISPLAY_H_
	#include	<Display.h>
#endif


typedef struct
{
	SLONG		X,
				Y;
}MFPoint;

typedef struct
{
	SLONG		Left,
				Top,
				Right,
				Bottom,
				Width,
				Height;
}MFRect;

inline BOOL		XYInRect(SLONG x,SLONG y,MFRect *the_rect)			{	if(x>=the_rect->Left&&y>=the_rect->Top&&x<=the_rect->Right&&y<=the_rect->Bottom)return TRUE;else return FALSE;	}
inline BOOL		PointInRect(MFPoint *the_point,MFRect *the_rect)	{	if(the_point->X>=the_rect->Left&&the_point->Y>=the_rect->Top&&the_point->X<=the_rect->Right&&the_point->Y<=the_rect->Bottom)return TRUE;else return FALSE;	}

//---------------------------------------------------------------
// Draw2D.c

extern UBYTE			*WorkWindow;
extern SLONG			WorkWindowHeight,
	 					WorkWindowWidth;
extern MFRect			WorkWindowRect;

void			SetWorkWindowBounds(SLONG left, SLONG top, SLONG width, SLONG height);
MFPoint			*GlobalToLocal(MFPoint *the_point);
void			GlobalXYToLocal(SLONG *x,SLONG *y);
inline void		SetWorkWindow(void)	{	WorkWindow=(WorkScreen+WorkWindowRect.Left*WorkScreenDepth+(WorkWindowRect.Top*WorkScreenWidth));	}

//---------------------------------------------------------------
// DrawBox.c

extern	void (*DrawBox)(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void (*DrawBoxC)(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);

//---------------------------------------------------------------
// DrawLine.c

extern	void (*DrawLine)(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void (*DrawLineC)(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void (*DrawHLine)(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void (*DrawHLineC)(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void (*DrawVLine)(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void (*DrawVLineC)(SLONG x,SLONG y1,SLONG y2,ULONG colour);


//---------------------------------------------------------------
// DrawPoint.c

extern	void (*DrawPoint)(MFPoint *the_point,ULONG colour);
extern	void (*DrawPointC)(MFPoint *the_point,ULONG colour);

//---------------------------------------------------------------
// DrawPixel.c

extern	void  (*DrawPixel)(SLONG x,SLONG y,ULONG colour);
extern	void  (*DrawPixelC)(SLONG x,SLONG y,ULONG colour);

//---------------------------------------------------------------
// DrawRect.c

void			DrawRect(MFRect *the_rect,ULONG colour);
void			DrawRectC(MFRect *the_rect,ULONG colour);

//---------------------------------------------------------------
// QuickText.c

extern UBYTE	*CharTable[];


extern	void  (*QuickText)(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void  (*QuickTextC)(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void  (*QuickChar)(SLONG x,SLONG y,CBYTE the_char,ULONG colour);
extern	void  (*QuickCharC)(SLONG x,SLONG y,CBYTE the_char,ULONG colour);

SLONG			QTStringWidth(CBYTE *the_string);
inline SLONG	QTStringHeight(void)				{	return 8;	}
inline SLONG	QTCharWidth(CBYTE the_char)			{	return (CharTable[the_char])[0];	}
inline SLONG	QTCharHeight(CBYTE the_char)		{	return (CharTable[the_char])[1];	}

//---------------------------------------------------------------

#endif