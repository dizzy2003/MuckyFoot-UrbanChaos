//
// Moving walkable faces.
//

#ifndef _WMOVE_
#define _WMOVE_

//#include	"c:\fallen\headers\memory.h"

typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;
	
} WMOVE_Point;



typedef struct
{
	UWORD face4;	// Index into the prim_faces4[] array for the walkable face.
	
	//
	// The position of the walkable face last frame. All WMOVE faces are rectangles.
	// Only three points are given. The last point is implicit.
	//

	WMOVE_Point last[3];

	UWORD thing;	// The thing that this face is attached to.
	UWORD number;	// If a thing has more than one face associated with it, this is the number of the face.

} WMOVE_Face;

#define RWMOVE_MAX_FACES 192
#define WMOVE_MAX_FACES	(save_table[SAVE_TABLE_WMOVE].Maximum)
//#define WMOVE_MAX_FACES 192

extern	WMOVE_Face *WMOVE_face; //[WMOVE_MAX_FACES];
extern	SLONG       WMOVE_face_upto;


//
// Initialises everything.
//

void WMOVE_init(void);


//
// Creates a moving walkable face attached to the given thing.
//

void WMOVE_create(Thing *);


//
// Removes all WMOVE_faces belonging to things of the given class.
//

#if 0
void WMOVE_remove(UBYTE which_class);
#endif

//
// Moves all the moving walkable faces.
//

void WMOVE_process(void);


//
// If somebody is standing on a walkable face, this gives their
// new world position so they can stay in the same place relative
// to the walkable face if it has moved.
//

void WMOVE_relative_pos(
		UBYTE  wmove_index,	// The WMOVE face stood on.
		SLONG  last_x,
		SLONG  last_y,
		SLONG  last_z,
		SLONG *now_x,
		SLONG *now_y,
		SLONG *now_z,
		SLONG *now_dangle);


//
// Draws the walkable faces using AENG_world_line()
// 

void WMOVE_draw(void);




#endif
