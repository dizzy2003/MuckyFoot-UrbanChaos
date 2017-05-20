// Draw2D.c
// Guy Simmons, 7th October 1996.


#include	<MFHeader.h>

UBYTE			*WorkWindow;
SLONG			WorkWindowHeight,
	 			WorkWindowWidth;
MFRect			WorkWindowRect;

//---------------------------------------------------------------

extern	void	DrawBox8(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void	DrawBoxC8(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void	DrawBox16(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void	DrawBoxC16(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void	DrawBox32(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
extern	void	DrawBoxC32(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);

extern	void	DrawLine8(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	DrawLineC8(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	DrawLine16(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	DrawLineC16(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	DrawLine32(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
extern	void	DrawLineC32(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);

extern	void	DrawHLine8(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	DrawHLineC8(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	DrawHLine16(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	DrawHLineC16(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	DrawHLine32(SLONG x1,SLONG x2,SLONG y,ULONG colour);
extern	void	DrawHLineC32(SLONG x1,SLONG x2,SLONG y,ULONG colour);

extern	void	DrawVLine8(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void	DrawVLineC8(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void	DrawVLine16(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void	DrawVLineC16(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void	DrawVLine32(SLONG x,SLONG y1,SLONG y2,ULONG colour);
extern	void	DrawVLineC32(SLONG x,SLONG y1,SLONG y2,ULONG colour);

extern	void	DrawPoint8(MFPoint *the_point,ULONG colour);
extern	void	DrawPointC8(MFPoint *the_point,ULONG colour);
extern	void	DrawPoint16(MFPoint *the_point,ULONG colour);
extern	void	DrawPointC16(MFPoint *the_point,ULONG colour);
extern	void	DrawPoint32(MFPoint *the_point,ULONG colour);
extern	void	DrawPointC32(MFPoint *the_point,ULONG colour);

extern	void	DrawPixel8(SLONG x,SLONG y,ULONG colour);
extern	void	DrawPixelC8(SLONG x,SLONG y,ULONG colour);
extern	void	DrawPixel16(SLONG x,SLONG y,ULONG colour);
extern 	void	DrawPixelC16(SLONG x,SLONG y,ULONG colour);
extern	void	DrawPixel32(SLONG x,SLONG y,ULONG colour);
extern	void	DrawPixelC32(SLONG x,SLONG y,ULONG colour);

extern	void	QuickText8(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern 	void	QuickTextC8(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void	QuickChar8(SLONG x,SLONG y,CBYTE the_char,ULONG colour);
extern	void	QuickCharC8_16_32(SLONG x,SLONG y,CBYTE the_char,ULONG colour);
extern	void	QuickText16(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern 	void	QuickTextC16(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void	QuickChar16(SLONG x,SLONG y,CBYTE the_char,ULONG colour);
extern	void	QuickText32(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern 	void	QuickTextC32(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
extern	void	QuickChar32(SLONG x,SLONG y,CBYTE the_char,ULONG colour);

extern 	void	DrawBSprite8(SLONG x,SLONG y,BSprite *the_sprite);
extern 	void	DrawBSprite16(SLONG x,SLONG y,BSprite *the_sprite);
extern 	void	DrawBSprite32(SLONG x,SLONG y,BSprite *the_sprite);

extern	void	DrawBSpriteC8(SLONG x,SLONG y,BSprite *the_sprite);
extern	void	DrawBSpriteC16(SLONG x,SLONG y,BSprite *the_sprite);
extern	void	DrawBSpriteC32(SLONG x,SLONG y,BSprite *the_sprite);

extern	void	DrawMonoBSprite8(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
extern	void	DrawMonoBSprite16(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
extern	void	DrawMonoBSprite32(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);

extern	void	DrawMonoBSpriteC8(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
extern	void	DrawMonoBSpriteC16(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
extern	void	DrawMonoBSpriteC32(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
	     
//---------------------------------------------------------------
// Set Function pointers for this display depth
void (*DrawBox)(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);
void (*DrawBoxC)(SLONG x,SLONG y,SLONG width,SLONG height,ULONG colour);

void (*DrawLine)(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
void (*DrawLineC)(SLONG x1,SLONG y1,SLONG x2,SLONG y2,ULONG colour);
void (*DrawHLine)(SLONG x1,SLONG x2,SLONG y,ULONG colour);
void (*DrawHLineC)(SLONG x1,SLONG x2,SLONG y,ULONG colour);
void (*DrawVLine)(SLONG x,SLONG y1,SLONG y2,ULONG colour);
void (*DrawVLineC)(SLONG x,SLONG y1,SLONG y2,ULONG colour);

void (*DrawPoint)(MFPoint *the_point,ULONG colour);
void (*DrawPointC)(MFPoint *the_point,ULONG colour);


void  (*DrawPixel)(SLONG x,SLONG y,ULONG colour);
void  (*DrawPixelC)(SLONG x,SLONG y,ULONG colour);

void  (*QuickText)(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
void  (*QuickTextC)(SLONG x,SLONG y,CBYTE *the_string,ULONG colour);
void  (*QuickChar)(SLONG x,SLONG y,CBYTE the_char,ULONG colour);
void  (*QuickCharC)(SLONG x,SLONG y,CBYTE the_char,ULONG colour);

void	(*DrawBSprite)(SLONG x,SLONG y,BSprite *the_sprite);
void	(*DrawBSpriteC)(SLONG x,SLONG y,BSprite *the_sprite);

void	(*DrawMonoBSprite)(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
void	(*DrawMonoBSpriteC)(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);

void	SetDrawFunctions(ULONG depth)
{
	switch (depth)
	{
		case 8:
			DrawBox=DrawBox8;
			DrawBoxC=DrawBoxC8;

			DrawLine=DrawLine8;
			DrawLineC=DrawLineC8;
			DrawHLine=DrawHLine8;
			DrawHLineC=DrawHLineC8;
			DrawVLine=DrawVLine8;
			DrawVLineC=DrawVLineC8;

			DrawPoint=DrawPoint8;
			DrawPointC=DrawPointC8;

			DrawPixel=DrawPixel8;
			DrawPixelC=DrawPixelC8;

			QuickText =QuickText8;
			QuickTextC=QuickTextC8;
			QuickChar =QuickChar8;
			QuickCharC=QuickCharC8_16_32;

			DrawBSprite=DrawBSprite8;
			DrawBSpriteC=DrawBSpriteC8;

			DrawMonoBSprite=DrawMonoBSprite8;
			DrawMonoBSpriteC=DrawMonoBSpriteC8;

			WorkScreenDepth=1;
			break;
		case 15:
		case 16:
			DrawBox=DrawBox16;
			DrawBoxC=DrawBoxC16;

			DrawLine=DrawLine16;
			DrawLineC=DrawLineC8;
			DrawHLine=DrawHLine16;
			DrawHLineC=DrawHLineC16;
			DrawVLine=DrawVLine16;
			DrawVLineC=DrawVLineC16;

			DrawPoint=DrawPoint16;
			DrawPointC=DrawPointC16;

			DrawPixel=DrawPixel16;
			DrawPixelC=DrawPixelC16;

			QuickText =QuickText16;
			QuickTextC=QuickTextC16;
			QuickChar =QuickChar16;
			QuickCharC=QuickCharC8_16_32;

			DrawBSprite=DrawBSprite16;
			DrawBSpriteC=DrawBSpriteC16;

			DrawMonoBSprite=DrawMonoBSprite16;
			DrawMonoBSpriteC=DrawMonoBSpriteC16;

			WorkScreenDepth=2;
			break;
		case 24:
		case 32:
			DrawBox=DrawBox32;
			DrawBoxC=DrawBoxC32;

			DrawLine=DrawLine32;
			DrawLineC=DrawLineC8;
			DrawHLine=DrawHLine32;
			DrawHLineC=DrawHLineC32;
			DrawVLine=DrawVLine32;
			DrawVLineC=DrawVLineC32;

			DrawPoint=DrawPoint32;
			DrawPointC=DrawPointC32;

			DrawPixel=DrawPixel32;
			DrawPixelC=DrawPixelC32;

			QuickText =QuickText32;
			QuickTextC=QuickTextC32;
			QuickChar =QuickChar32;
			QuickCharC=QuickCharC8_16_32;

			DrawBSprite=DrawBSprite32;
			DrawBSpriteC=DrawBSpriteC32;

			DrawMonoBSprite=DrawMonoBSprite32;
			DrawMonoBSpriteC=DrawMonoBSpriteC32;

			WorkScreenDepth=4;
			break;
	}
}

//---------------------------------------------------------------

void	SetWorkWindowBounds(SLONG left, SLONG top, SLONG width, SLONG height)
{
	if((left+width)>=WorkScreenPixelWidth)
	{
		width	-=	(left+width)-WorkScreenPixelWidth;
		if(width<1)
		{
			left	=	0;
			width	=	1;
		}
	}
	if((top+height)>=WorkScreenHeight)
	{
		height	-=	(top+height)-WorkScreenHeight;
		if(height<1)
		{
			top		=	0;
			height	=	1;
		}
	}
	WorkWindowRect.Left		=	left;
	WorkWindowRect.Top		=	top;
	WorkWindowRect.Right	=	(left+width)-1;
	WorkWindowRect.Bottom	=	(top+height)-1;
	WorkWindowRect.Width	=	width;
	WorkWindowRect.Height	=	height;

	WorkWindowHeight		=	height;
	WorkWindowWidth			=	width;

	SetWorkWindow();
}

//---------------------------------------------------------------

MFPoint	*GlobalToLocal(MFPoint *the_point)
{
	the_point->X	-=	WorkWindowRect.Left;
	the_point->Y	-=	WorkWindowRect.Top;

	return	the_point;
}

//---------------------------------------------------------------

void	GlobalXYToLocal(SLONG *x,SLONG *y)
{
	*x	-=	WorkWindowRect.Left;
	*y	-=	WorkWindowRect.Top;
}

//---------------------------------------------------------------
