//
// Simple ovals underneath people, barrels etc...
//

#ifndef _OVAL_
#define _OVAL_



//
// Creates an oval elongated in a given direction...
//

#define OVAL_TYPE_OVAL		0
#define OVAL_TYPE_SQUARE	1

void OVAL_add(
		SLONG x,	// 8 bits per mapsquare
		SLONG y,
		SLONG z,
		SLONG size,
		float elongate = 1.0F,
		float angle    = 0.0F,
		SLONG type     = OVAL_TYPE_OVAL);



#endif
