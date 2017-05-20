//
// Water.
//

#ifndef WATER_H
#define WATER_H



//
// Gets rid of all water.
//

void WATER_init(void);


//
// Creates a square of water at the given height.
//

void WATER_add(SLONG map_x, SLONG map_z, SLONG height);


//
// Effects the water as if something was moving through it.
//

void WATER_gush(SLONG x1, SLONG z1, SLONG x2, SLONG z2);


//
// Processes the water.
//

void WATER_process(void);


//
// Drawing the water...
//

//
// For storing the 'transformed point index' with each
// of the water points.
//

void  WATER_point_index_clear_all(void);
UWORD WATER_point_index_get      (UWORD p_index);
void  WATER_point_index_set      (UWORD p_index, UWORD index);

//
// Returns the first water face in the linked list above a square.
// Returns the next face... NULL => the end of the list.
//

UWORD WATER_get_first_face(SLONG x, SLONG z);
UWORD WATER_get_next_face (UWORD f_index);

//
// Returns the points of the given face by filling in the array.
//

void WATER_get_face_points(UWORD f_index, UWORD p_index[4]);

//
// Returns the given point.
//

void WATER_get_point_pos(UWORD p_index, float *x, float *y, float *z);
void WATER_get_point_uvs(UWORD p_index, float *u, float *v, ULONG *colour);


#endif
