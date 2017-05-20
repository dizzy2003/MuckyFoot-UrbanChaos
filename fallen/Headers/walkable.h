#ifndef	WALKABLE_H
#define	WALKABLE_H	1


//
// defines
//
#define	calc_height_on_face(x,z,face,new_y) get_height_on_face_quad64_at(x,z,face,new_y)	

#define	FIND_ANYFACE			1
#define	FIND_FACE_BELOW			2
#define	FIND_FACE_NEAR_BELOW	3
//
//structures
//






//
//Data
//



//extern	struct 	DInsideRect	inside_rect[MAX_INSIDE_RECT];


//
// Functions
//




//extern	SLONG	calc_height_on_face(SLONG x,SLONG z,SLONG face,SLONG *new_y);
extern	SLONG	get_height_on_face_quad64_at(SLONG rx, SLONG rz,SWORD face,SLONG *height);
extern	SLONG	find_height_for_this_pos(	SLONG  x,SLONG  z,	SLONG *ret_face);
extern	SLONG	calc_height_on_rface(SLONG x, SLONG z,SWORD	face,SLONG *new_y);



//
// Looks for a roof face over the given mapsquare.
// If it finds one it deletes it.
//

void WALKABLE_remove_rface(UBYTE map_x, UBYTE map_z);


#endif