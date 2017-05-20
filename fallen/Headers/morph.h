//
// Morphing keyframes.
//

#ifndef _MORPH_
#define _MORPH_


//
// Loads all the morphs in.
//

void MORPH_load(void);


//
// Accessing the morphs.
//

#define MORPH_PIGEON_WALK1	  0
#define MORPH_PIGEON_WALK2	  1
#define MORPH_PIGEON_STAND	  0
#define MORPH_PIGEON_PECK	  0
#define MORPH_PIGEON_HEADCOCK 0
#define MORPH_NUMBER		  2

typedef struct
{
	SWORD x;
	SWORD y;
	SWORD z;
	
} MORPH_Point;

MORPH_Point *MORPH_get_points    (SLONG morph);
SLONG        MORPH_get_num_points(SLONG morph); 


#endif
