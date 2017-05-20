#ifndef _EXTRA_
#define _EXTRA_

//
// Extra stuff like puddles and mist.
//

#define EXTRA_TYPE_NONE		0
#define EXTRA_TYPE_PUDDLE	1
#define EXTRA_TYPE_MIST		2

typedef struct
{
	UBYTE type;
	UBYTE height;
	UBYTE detail;
	UBYTE shit1;
	UWORD x;
	UWORD z;
	UWORD angle;
	UWORD radius;
	SLONG shit2;
	SLONG shit3;

} EXTRA_Thing;

#define EXTRA_MAX_THINGS 256

extern EXTRA_Thing EXTRA_thing[EXTRA_MAX_THINGS];


//
// Creates something of the given type with the default
// settings at (x,z).  If something of that type already
// exists there, then it is deleted.
//

#define EXTRA_SELECT_DIST 256	// How close two things of the same type can be from eachother.

void EXTRA_create_or_delete(SLONG type, SLONG x, SLONG z);



#endif
