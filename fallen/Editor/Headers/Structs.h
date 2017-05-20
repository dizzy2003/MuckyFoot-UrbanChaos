// Structs.h
// Guy Simmons, 27th March 1997.


#ifndef	STRUCTS_H
#define	STRUCTS_H

#include	"DarkCity.h"


//---------------------------------------------------------------

typedef	struct
{
	SLONG		X,
				Y,
				Z;
}Coord;

//---------------------------------------------------------------

/*
typedef	struct
{
	Matrix33		*TheMatrix;
}Object3D;

typedef	struct
{
	
}Object2D;


typedef	struct
{
	UBYTE				DrawType;
	KeyFrameElement		*AnimElements,
						*NextAnimElements;
	union
	{
		Object3D		
		Object	
	}DrawType;
}Draw;
*/

typedef	struct
{
	ULONG		DrawType;
}Draw;

//---------------------------------------------------------------

typedef	struct
{
	SBYTE		Class,
				State;
	ULONG		Flags;
	SLONG		Child,
				Parent;
	SLONG		LinkChild,
				LinkParent;

	Draw		Draw;
}Thing;

//---------------------------------------------------------------

typedef	struct
{
	float			Altitude;
	void			*MapWho;

}MapElement;

//---------------------------------------------------------------

#endif
