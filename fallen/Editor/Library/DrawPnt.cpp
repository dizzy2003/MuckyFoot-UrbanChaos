// DrawPoint.c
// Guy Simmons, 7th October 1996.


#include	"Editor.hpp"


//---------------------------------------------------------------

void	DrawPoint8(MFPoint *the_point,ULONG colour)
{
	*(WorkWindow+the_point->X+(the_point->Y*WorkScreenWidth))	=	(UBYTE)colour;
}

void	DrawPoint16(MFPoint *the_point,ULONG colour)
{
	UWORD	*ptr;
	ptr  = (UWORD*)WorkWindow+the_point->X+(the_point->Y*WorkScreenWidth>>1);
	*ptr = (UWORD)colour;

}

void	DrawPoint32(MFPoint *the_point,ULONG colour)
{
	ULONG	*ptr;
	ptr  = (ULONG*)WorkWindow+the_point->X+(the_point->Y*WorkScreenWidth>>2);
	*ptr = (ULONG)colour;
}

//---------------------------------------------------------------

void	DrawPointC8(MFPoint *the_point,ULONG colour)
{
	if	(
			the_point->X>=0					&&
			the_point->X<WorkWindowWidth	&&
			the_point->Y>=0					&&
			the_point->Y<WorkWindowHeight
		)
	{
		*(WorkWindow+the_point->X+(the_point->Y*WorkScreenWidth))	=	(UBYTE)colour;
	}
}

void	DrawPointC16(MFPoint *the_point,ULONG colour)
{
	UWORD	*ptr;
	if	(
			the_point->X>=0					&&
			the_point->X<WorkWindowWidth	&&
			the_point->Y>=0					&&
			the_point->Y<WorkWindowHeight
		)
	{
		ptr  = (UWORD*)WorkWindow+the_point->X+(the_point->Y*WorkScreenWidth>>1);
		*ptr = (UWORD)colour;
	}
}

void	DrawPointC32(MFPoint *the_point,ULONG colour)
{
	ULONG	*ptr;
	if	(
			the_point->X>=0					&&
			the_point->X<WorkWindowWidth	&&
			the_point->Y>=0					&&
			the_point->Y<WorkWindowHeight
		)
	{
		ptr  = (ULONG*)WorkWindow+the_point->X+(the_point->Y*WorkScreenWidth>>2);
		*ptr = (ULONG)colour;
	}
}

//---------------------------------------------------------------
