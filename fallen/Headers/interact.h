#ifndef INTERACT_H
#define INTERACT_H

#include "building.h"

//
// Defines
//

#ifdef TARGET_DC
#define MAX_ANIM_CHUNKS 16 //256
#define MAX_GAME_CHUNKS 5 //256
#else
#define MAX_ANIM_CHUNKS 16 //256
#define MAX_GAME_CHUNKS 5 //256
#endif

//
// Structs
//


//
// Data
//
/*
extern	struct KeyFrame	*anim_array[300],
						*cop_array[300],
						*darci_array[300],
						*van_array[20],
						*roper_array[300],
						*thug_array[300];
*/

extern	struct GameKeyFrame	*global_anim_array[4][450];

extern	struct KeyFrameChunk 	*test_chunk;
#if !defined(PSX) && !defined(TARGET_DC)
extern	struct KeyFrameChunk 	test_chunk2,
								test_chunk3,
								thug_chunk;
#endif

extern	struct KeyFrameElement	*the_elements;
extern	struct GameKeyFrameChunk 	game_chunk[MAX_GAME_CHUNKS];
extern	struct GameKeyFrameChunk anim_chunk[MAX_ANIM_CHUNKS];


//
// The bounding boxes of the animating prims.
// 

typedef struct
{
	SLONG minx;
	SLONG miny;
	SLONG minz;

	SLONG maxx;
	SLONG maxy;
	SLONG maxz;

} AnimPrimBbox;

extern AnimPrimBbox anim_prim_bbox[MAX_ANIM_CHUNKS];






//
// Functions
//

extern SLONG find_grab_face(
				SLONG  x,
				SLONG  y,
				SLONG  z,
				SLONG  radius,
				SLONG  dy,
				SLONG  angle,
				SLONG *grab_x,
				SLONG *grab_y,
				SLONG *grab_z,
				SLONG *grab_angle,
				SLONG ignore_faces_from_this_building,
				SLONG trench,
				SLONG *type,
				Thing *p_person);


#if !defined(PSX) && !defined(TARGET_DC)
extern SLONG find_grab_face_in_sewers(
				SLONG  x,
				SLONG  y,
				SLONG  z,
				SLONG  radius,
				SLONG  dy,
				SLONG  angle,
				SLONG *grab_x,
				SLONG *grab_y,
				SLONG *grab_z,
				SLONG *grab_angle);
#endif

extern	void	calc_sub_objects_position(Thing *p_mthing,SLONG tween,UWORD object,SLONG *x,SLONG *y,SLONG *z);
extern	void	calc_sub_objects_position_keys(Thing *p_mthing,SLONG tween,UWORD object,SLONG *x,SLONG *y,SLONG *z,struct GameKeyFrame *frame1,struct GameKeyFrame *frame2);
extern	void	calc_sub_objects_position_global(GameKeyFrame *cur_frame,GameKeyFrame *next_frame,SLONG tween,UWORD object,SLONG *x,SLONG *y,SLONG *z);


//
// Returns the height of the cable at the given length along it.
// The length goes from 1 to CABLE_ALONG_MAX (defined in building.h)
//

SLONG find_cable_y_along(struct DFacet *p_facet, SLONG along); // 0 <= along <= CABLE_MAX_ALONG


#endif
