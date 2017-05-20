//
// Splashes...
//

#ifndef _DRIP_
#define _DRIP_


#define DRIP_FLAG_PUDDLES_ONLY		(1)

//
// Gets rid of all the drips.
//

void DRIP_init(void);

//
// Creates a new drip at (x,y,z)
//

void DRIP_create(
		UWORD x,
		SWORD y,
		UWORD z,
		UBYTE flags);

//
// Creates a new drip at (x,y,z) only if (x,y,z)
// is in a puddle.
//

void DRIP_create_if_in_puddle(
		UWORD x,
		SWORD y,
		UWORD z);

//
// Makes all the drips get bigger and fade out.
//

void DRIP_process(void);


//
// Drawing the drips...
//

typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;
	UBYTE size;
	UBYTE fade; // 255 => opaque, 0 => transparent.
	UBYTE flags;
	
} DRIP_Info;

void       DRIP_get_start(void);
DRIP_Info *DRIP_get_next (void); // NULL => no more drips.


#endif
