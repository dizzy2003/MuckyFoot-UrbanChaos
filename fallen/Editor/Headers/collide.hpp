#ifndef	COLLISION_H
#define	COLLISION_H			1

//#include "fallen/headers/collide.h"
//DEFINES

// STRUCTS


//data
//collision info


// FUNCTIONS

extern	void	calc_collision_info(struct ColInfo *p_col);
//extern	void	calc_collision_info(SLONG left,SLONG right,SLONG top,SLONG bottom,SLONG depth,SLONG flag);

extern	void	clear_all_col_info(void);

extern	void	interface_thing(void);
extern	void	init_thing(void);
extern	SLONG	calc_height_at(SLONG x,SLONG z);
extern	calc_things_height(struct MapThing *p_thing);


//
// Line intersection. Line segments that share a point count as
// intersecting.
//

#define	DONT_INTERSECT    0
#define	DO_INTERSECT      1
#define COLLINEAR         2

SLONG lines_intersect(
			SLONG x1, SLONG y1, SLONG x2, SLONG y2,
			SLONG x3, SLONG y3, SLONG x4, SLONG y4,
			SLONG *x,
			SLONG *y);


#endif
