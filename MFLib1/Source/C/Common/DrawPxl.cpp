// DrawPixel.c
// Guy Simmons, 7th October 1996.


#include	<MFHeader.h>


//---------------------------------------------------------------

void	DrawPixel8(SLONG x,SLONG y,ULONG colour)
{
	*(WorkWindow+x+(y*WorkScreenWidth))	=	(UBYTE)colour;
}

void	DrawPixel16(SLONG x,SLONG y,ULONG colour)
{
	UWORD	*ptr;
	ptr  = (UWORD*)WorkWindow+x+(y*WorkScreenWidth>>1);
	*ptr = (UWORD)colour;
}

void	DrawPixel32(SLONG x,SLONG y,ULONG colour)
{
	ULONG	*ptr;
	ptr  = (ULONG*)WorkWindow+x+(y*WorkScreenWidth>>2);
	*ptr = (ULONG)colour;
}

//---------------------------------------------------------------

void	DrawPixelC8(SLONG x,SLONG y,ULONG colour)
{
	if(x>=0 && x<WorkWindowWidth && y>=0 && y<WorkWindowHeight)
	{
		*(WorkWindow+x+(y*WorkScreenWidth))	=	(UBYTE)colour;
	}
}

void	DrawPixelC16(SLONG x,SLONG y,ULONG colour)
{
	if(x>=0 && x<WorkWindowWidth && y>=0 && y<WorkWindowHeight)
	{
		UWORD	*ptr;
		ptr  = (UWORD*)WorkWindow+x+(y*WorkScreenWidth>>1);
		*ptr = (UWORD)colour;
	}
}

void	DrawPixelC32(SLONG x,SLONG y,ULONG colour)
{
	if(x>=0 && x<WorkWindowWidth && y>=0 && y<WorkWindowHeight)
	{
		ULONG	*ptr;
		ptr  = (ULONG*)WorkWindow+x+(y*WorkScreenWidth>>2);
		*ptr = (ULONG)colour;
	}
}

//---------------------------------------------------------------
