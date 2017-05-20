//
// Fire!
//

#ifndef _FIRE_
#define _FIRE_


//
// Gets rid of all the fire.
// 

void FIRE_init(void);

//
// Creates a new bit of fire.
//

void FIRE_create(
		UWORD x,
		SWORD y,
		UWORD z,
		UBYTE size,
		UBYTE life);	// 0 => Live for ever, 1 => Live for for ages, 255 => live a very short time.

//
// Processes all the fire.
//

void FIRE_process(void);

//
// How to draw fire: The fire is given as a series of flames whose
// points are given in the following order.
//
//						8
//					   /|
//					  /	|
//					 /	|
//				   6/	|7
//				   #----#
//				   |	 \
//				  4#------#5
//				  /		 /
//			     /		/
//			   2#------#3
//				|	   |
//				#------#
//			   0		1
//
// In other words, it is a triangle strip!
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	UBYTE u;
	UBYTE v;
	UBYTE alpha;
	UBYTE shit;

} FIRE_Point;

typedef struct
{
	SLONG       num_points;
	FIRE_Point *point;

} FIRE_Info;

void       FIRE_get_start(UBYTE z, UBYTE x_min, UBYTE x_max);
FIRE_Info *FIRE_get_next (void);		// NULL => No more fire here.



#endif
