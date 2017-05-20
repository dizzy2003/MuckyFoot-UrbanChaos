//
// Collections of little sparkley projectiles.
//

#ifndef _GLITTER_
#define _GLITTER_



//
// Remove all glitter.
//

void GLITTER_init(void);



//
// Creates a new collection of sparkles... a glitter!
//

#define GLITTER_FLAG_DXPOS	 (1 << 0)	// Sparkle 'dx' must be positive.
#define GLITTER_FLAG_DXNEG	 (1 << 1)	// Sparkle 'dx' must be negative.
#define GLITTER_FLAG_DYPOS	 (1 << 2)	// Sparkle 'dy' must be positive.
#define GLITTER_FLAG_DYNEG	 (1 << 3)	// Sparkle 'dy' must be negative.
#define GLITTER_FLAG_DZPOS	 (1 << 4)	// Sparkle 'dz' must be positive.
#define GLITTER_FLAG_DZNEG	 (1 << 5)	// Sparkle 'dz' must be negative.
#define GLITTER_FLAG_USED	 (1 << 6)	// Private...
#define GLITTER_FLAG_DESTROY (1 << 7)	// Private...

UBYTE GLITTER_create(
		UBYTE flag,
		UBYTE map_x,
		UBYTE map_z,
		ULONG colour);

//
// Destroys the glitter when all its spark die off.
//

void  GLITTER_destroy(UBYTE glitter);

//
// Adds a sparkle to a glitter.
//

void GLITTER_add(
		UBYTE glitter,
		SLONG x,
		SLONG y,
		SLONG z);


void GLITTER_process(void);




//
// Each sparkle is a line segment...
//

typedef struct
{
	SLONG x1;
	SLONG y1;
	SLONG z1;
	SLONG x2;
	SLONG y2;
	SLONG z2;
	ULONG colour;
	
} GLITTER_Info;

void          GLITTER_get_start(UBYTE xmin, UBYTE xmax, UBYTE z);
GLITTER_Info *GLITTER_get_next(void);



#endif

