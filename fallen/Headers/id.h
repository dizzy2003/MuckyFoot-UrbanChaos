//
// Interior design module.
//

#ifndef ID_H
#define ID_H


//
// The size of the box inside which the floorplan can be.
// The maximum dimension of any building.
//

#define ID_FLOOR_SIZE 256
#define ID_PLAN_SIZE  32


//
// Given the outline of a storey of a building, this module returns a floor plan
// of where the rooms and the furniture should go.
//

//
// First give the outline of the storey of the building. Does not work with overlapping
// or intersecting walls!
//

void ID_clear_floorplan(void);
void ID_set_outline(
		SLONG x1, SLONG z1, SLONG x2, SLONG z2,
		SLONG id,
		SLONG num_blocks);

//
// The 'get_type' function returns whether there is a door or a window or wall on
// the 'block''th blocks along the wall with the given 'id'
//

#define ID_BLOCK_TYPE_WALL		1
#define ID_BLOCK_TYPE_WINDOW	2
#define ID_BLOCK_TYPE_DOOR		3

void ID_set_get_type_func(SLONG (*get_type)(SLONG id, SLONG block));

//
// This function generates the floorplan for the given type of building.
//

#define ID_STAIR_TYPE_BOTTOM	1
#define ID_STAIR_TYPE_MIDDLE	2
#define ID_STAIR_TYPE_TOP		3

typedef struct
{
	UBYTE type;
	UBYTE id;
	UWORD shit;
	SLONG handle_up;		// The value returned when you go up the stairs.
	SLONG handle_down;		// The value returned when you go down the stairs.

	//
	// The two squares the stairs take up.  These are ordered
	// such that the stairs are on the left looking from
	// (x1,z1) to (x2,z2).
	//

	UBYTE x1;
	UBYTE z1;
	UBYTE x2;
	UBYTE z2;

} ID_Stair;

#define ID_STOREY_TYPE_HOUSE_GROUND			1
#define ID_STOREY_TYPE_HOUSE_UPPER			2
#define ID_STOREY_TYPE_OFFICE_GROUND		3
#define ID_STOREY_TYPE_OFFICE_UPPER			4
#define ID_STOREY_TYPE_WAREHOUSE			5
#define ID_STOREY_TYPE_APARTEMENT_GROUND	6
#define ID_STOREY_TYPE_APARTEMENT_UPPER		7

//
// If 'find_good_layout' then this function uses the seed and a large
// number derived from it to create many layouts, and then picks the
// best one, otherwise it always uses the given seed.  It returns the
// seed it used.
//
// The seed it uses always fits in a UWORD. If the result is negative
// then an error occured and it could not generate a floorplan for the
// building.
//

SLONG ID_generate_floorplan(
			SLONG    type, 
			ID_Stair stair[],
			SLONG num_stairs,
			UWORD seed,
			UBYTE find_good_layout,
			UBYTE furnished);

//
// Inserts collision vectors for the inside walls.
// Removes collision vectors for the inside walls.
//
// Always call these functions in pairs.
//

void ID_wall_colvects_insert(void);
void ID_wall_colvects_remove(void);

//
// Removes all the furniture and things created for the inside
// of the building.
//

void ID_remove_inside_things(void);


//
// Returs the room index of a mapsquare.  0 == NULL room index.
// This is a mapsquare, not a world position.
//

UBYTE ID_get_mapsquare_room(SLONG x, SLONG z);

//
// Returns the world camera position for the given room.
//

void ID_get_room_camera(UBYTE room, SLONG *x, SLONG *y, SLONG *z);

//
// If (x,z) is a position on a staircase from where you should
// go to another floor and handle is either up_handle
// or down_handle of the staircase you've come from.
//
// Reutrns -1, 0 or +1 depending on whether you go up, nowhere, or down.
//

SLONG ID_change_floor(
		SLONG  x,
		SLONG  z,
		SLONG *new_x,
		SLONG *new_z,
		SLONG *handle);

//
// Accessing the inside of the building.
//

void ID_get_floorplan_bounding_box(
		SLONG *x1,
		SLONG *z1,
		SLONG *x2,
		SLONG *z2);

//
// Returns info about a floor square.
//

SLONG ID_am_i_completely_outside(SLONG x, SLONG z);
SLONG ID_should_i_draw_mapsquare(SLONG x, SLONG z);
SLONG ID_get_mapsquare_texture  (SLONG x, SLONG z,
		float *u0, float *v0,
		float *u1, float *v1,
		float *u2, float *v2,
		float *u3, float *v3);

//
// For drawing rooms you are inside. First tell the ID module where you are
// and then it can tell you whether or not to draw a mapsquare.
//

void  ID_this_is_where_i_am(SLONG x, SLONG z);
SLONG ID_should_i_draw     (SLONG x, SLONG z);	// (x,z) must be in the bounding square of the floorplan.

//
// Returns index of the first face above a floor square. 0 => NULL index.
// Returns TRUE if the face is a quad, otherwise it is a triangle.
// Returns the texture of the face.
// Returns the next face in the linked list of faces above a floor square.
//

SLONG ID_get_first_face  (SLONG x, SLONG z);
SLONG ID_is_face_a_quad  (SLONG face);
SLONG ID_get_next_face   (SLONG face);

//
// Fills in the texture coordinates of the given face and
// returns the page.
//

SLONG ID_get_face_texture(SLONG face,
		float *u0, float *v0,
		float *u1, float *v1,
		float *u2, float *v2,
		float *u3, float *v3);

//
// Drawing the furniture inside a building...
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	UWORD prim;
	UWORD yaw;

} ID_Finfo;

SLONG     ID_get_num_furn(void);
ID_Finfo *ID_get_furn(SLONG number);	// Starting from zero.



//
// Points are two types. They are either structures with (x,y,z) position
// and an 'index' field, or they are structures with just an (x,z) mapsquare
// coordinate.
//

//
// Clears the 'index' field in all the points.
// Returns true if the point is just a mapsquare coordinate.
// Returns the mapsquare coord of the point.
// Returns the position of the point.
// Returns the value of the 'index' field of the point.
// Sets the value of the 'index' field of the point.
//

void  ID_clear_indices       (void);
SLONG ID_is_point_a_mapsquare(SLONG face, SLONG point);
void  ID_get_point_mapsquare (SLONG face, SLONG point, SLONG *x, SLONG *z);	// Map coordinates
void  ID_get_point_position  (SLONG face, SLONG point, SLONG *x, SLONG *y, SLONG *z); // ELE_SHIFT fixed point.
UWORD ID_get_point_index     (SLONG face, SLONG point);
void  ID_set_point_index	 (SLONG face, SLONG point, UWORD index);


// ########################################################
// ========================================================
//
// COLLISION STUFF...
//
// ========================================================
// ########################################################


//
// Returns TRUE if the vector collides with the insides.
//

SLONG ID_collide_3d(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2);

//
// Returns the height of the floor at (x,z)
// The coordinate must be inside the building.
//

SLONG ID_calc_height_at(SLONG x, SLONG z);


//
// Returns if the 2D vector on the ground collides with
// a wall. If it does, it returns the new end point of
// the vector that will let it 'slide' along the wall.
//

SLONG ID_collide_2d(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG Z2,
		SLONG  radius,
		SLONG *slide_x,
		SLONG *slide_z);


// ########################################################
// ========================================================
//
// EDITOR STUFF...
//
// ========================================================
// ########################################################

//
// Returns the position of all the inside walls and all the rooms.
//

typedef struct
{
	UBYTE door[4];	// 255 => No door along this wall, else the number of
					// the block with a door in it.
	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

} ID_Wallinfo;

typedef struct
{
	SLONG  x;		// A position inside the room.
	SLONG  z;
	CBYTE *what;	// A string describing the room.

} ID_Roominfo;

typedef struct
{
	SLONG x1;		// The two squares that contain the staircase.
	SLONG z1;
	SLONG x2;
	SLONG z2;

} ID_Stairinfo;

void ID_editor_start_get_rooms (void);
void ID_editor_start_get_walls (void);
void ID_editor_start_get_stairs(void);

//
// These functions return FALSE if there are no more rooms, walls
// or stairs, otherwise they fill out the given structure with
// info describing the next room, wall or staircase.
//

SLONG ID_editor_get_room (ID_Roominfo  *ans);
SLONG ID_editor_get_wall (ID_Wallinfo  *ans);
SLONG ID_editor_get_stair(ID_Stairinfo *ans);


#endif


