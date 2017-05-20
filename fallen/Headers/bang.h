//
// Explosions!
//

#ifndef _BANG_
#define _BANG_


//
// Initialises all the bangs.
//

void BANG_init(void);

//
// Processes the bangs.
//

void BANG_process(void);

//
// Creates a new bang of the given type.
//

#define BANG_BIG	0

void BANG_create(
	SLONG type,
	SLONG x,
	SLONG y,
	SLONG z);

//
// Drawing the bangs requires a semi-sphere primitive in the engine 
// whose colour of the centre point is given and the colour on the edge
// should be black. The primitive should be additively blended.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;	// Gives the direction of the semi-sphere normalised to 256
	SLONG dy;
	SLONG dz;
	SLONG radius;
	UBYTE red;
	UBYTE green;
	UBYTE blue;
	UBYTE frame;
	
} BANG_Info;

void       BANG_get_start(UBYTE xmin, UBYTE xmax, UBYTE z);
BANG_Info *BANG_get_next(void);	// NULL => No more bangs.



#endif
