// Move.cpp
// Guy Simmons, 27th March 1997.

#include	"Editor.hpp"
#include	"Structs.h"

#ifdef	DOGPOO
//---------------------------------------------------------------

SLONG	get_distance(Coord *position1,Coord *position2)
{
/*
	return	(
				Hypotenuse	(
								Hypotenuse	(
												position2->X-(SLONG)position1->X,
												position2->Z-(SLONG)position1->Z
											),
								position2->Y-(SLONG)position1->Y
							)
			);
*/
	return	0;
}

//---------------------------------------------------------------

SLONG	get_approx_distance(Coord *position1,Coord *position2)
{
	return	0;
}

//---------------------------------------------------------------

SLONG	get_distance_xz(Coord *position1,Coord *position2)
{
/*
	return	(
				Hypotenuse	(
								position2->X-position1->X,
								position2->Z-position1->Z
							)
			);
*/
	return	0;
}

//---------------------------------------------------------------

SLONG	get_angle_xz(Coord *position1,Coord *position2)
{
//	return(Arctan(position2->X-position1->X,position2->Z-position1->Z));
	return	0;
}

//---------------------------------------------------------------

SLONG	get_angle_yz(Coord *position1,Coord *position2)
{
//	return(Arctan(position2->Y-position1->Y,-get_distance_xz(position1,position2)));
	return	0;
}

//---------------------------------------------------------------
#endif