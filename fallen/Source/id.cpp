#include "game.h"
#include "id.h"
#include "maths.h"
#include "thing.h"
#include "ob.h"

#ifndef		PSX


//
// Do you want thin walls
//

#define YOU_WANT_THIN_WALLS	0


//
// The height of the ceiling
//

#define ID_CEILING_HEIGHT (0x100)
#define ID_WALL_WIDTH     (0x8)

//
// Points.
//

typedef struct
{
	UWORD x;		// 8-bits per mapsquare.
	UWORD z;
	UWORD y;
	UWORD index;	// A special field. It is NOT the index of the point, but a place where
					// the engine can store a value (the index of the transformed point.)

} ID_Point;

#define ID_MAX_POINTS 1024

ID_Point ID_point[ID_MAX_POINTS];
SLONG    ID_point_upto;

//
// Faces.
//

#define ID_FACE_FLAG_QUAD		(1 << 0)	// Otherwise it is a triange and point 3 is unused.

//
// Whether the n'th point is a packed (x,z) coordinate
// or an index into the point array.
//

#define ID_FACE_FLAG_ONFLOOR0	(1 << 4)
#define ID_FACE_FLAG_ONFLOOR1	(1 << 5)
#define ID_FACE_FLAG_ONFLOOR2	(1 << 6)
#define ID_FACE_FLAG_ONFLOOR3	(1 << 7)

typedef struct
{
	UWORD point[4];	// Each point is either an index into the point array,
					// If the corresponding flag is set, it is and (x,z)
					// coordinate of the floor point. x is in the LBS, z is
					// in the MSB.
	UWORD texture;
	UWORD flag;
	UWORD next;
	UWORD shit;

} ID_Face;

#ifdef	PSX
#define ID_MAX_FACES 512
#else
#define ID_MAX_FACES 2048
#endif

ID_Face ID_face[ID_MAX_FACES];
SLONG   ID_face_upto;


//
// The floorplan a square can be both inside and outside at the
// same time!
//

#define ID_FLOOR_FLAG_OUTSIDE   (1 << 0)
#define ID_FLOOR_FLAG_INSIDE    (1 << 1)
#define ID_FLOOR_FLAG_ON_WALL   (1 << 2)	// The bottom left hand corner of the square is on a wall.
#define ID_FLOOR_FLAG_P_IN	    (1 << 3)	// The bottom left hand corner of the square is inside.

#define ID_FLOOR_FLAG_WALL_XS   (1 << 4)
#define ID_FLOOR_FLAG_WALL_XL   (1 << 5)
#define ID_FLOOR_FLAG_WALL_ZS   (1 << 6)
#define ID_FLOOR_FLAG_WALL_ZL   (1 << 7)

#define ID_FLOOR_FLAG_DOORMAT   (1 << 8)	// An outside door opens onto this square.
#define ID_FLOOR_FLAG_INMAT     (1 << 9)	// This square connects to an inside-door.

#define ID_FLOOR_FLAG_FURNISHED (1 << 10)	// There is an item of furniture on this square.
#define ID_FLOOR_FLAG_STAIR		(1 << 11)	// There is a staircase here.



#define ID_FLOOR_ROOM_NULL 0

typedef struct
{
	UWORD flag;
	UBYTE room;		// The unique room number to which this floorsquare belongs.
	UBYTE texid;
	UWORD next;		// The index into the face array of the first face in the linked list.

} ID_Square;

ID_Square ID_floor[ID_PLAN_SIZE][ID_PLAN_SIZE];

//
// The bounding box of the floorplan.
//

SLONG ID_floor_x1;
SLONG ID_floor_z1;
SLONG ID_floor_x2;
SLONG ID_floor_z2;

//
// How to access the floor squares indirectly, so that
// you can pretend the array is 'ID_FLOOR_SIZE' big.
// You must only ask for squares within the bounding
// box of the floorplan.
//

#ifndef NDEBUG
#define ID_FLOOR(x,z) (ID_floor_slow((x),(z)))
#else
#define ID_FLOOR(x,z) (&ID_floor[(x) - ID_floor_x1][(z) - ID_floor_z1])
#endif

ID_Square *ID_floor_slow(SLONG x, SLONG z)
{
	SLONG sx;
	SLONG sz;

	ASSERT(WITHIN(x, ID_floor_x1, ID_floor_x2));
	ASSERT(WITHIN(z, ID_floor_z1, ID_floor_z2));

	sx = x - ID_floor_x1;
	sz = z - ID_floor_z1;

	ASSERT(WITHIN(sx, 0, ID_PLAN_SIZE - 1));
	ASSERT(WITHIN(sz, 0, ID_PLAN_SIZE - 1));

	return &ID_floor[sx][sz];
}

//
// The number of inside squares... the area of the building.
//

SLONG ID_floor_area;

//
// The walls
//

#define ID_WALL_T_OUTSIDE	1
#define ID_WALL_T_INSIDE	2

typedef struct
{
	UBYTE type;
	UBYTE door[3];		// For inside walls only. 255 => NULL

	UBYTE x1, z1;
	UBYTE x2, z2;
	SLONG id;
	SLONG num_blocks;
	
} ID_Wall;

#define ID_MAX_WALLS 64

ID_Wall ID_wall[ID_MAX_WALLS];
SLONG   ID_wall_upto;

//
// The 'get_type' function.
//

SLONG (*ID_get_type)(SLONG id, SLONG block);

//
// For working out the inside squares.
//

#define ID_LINK_T_ENTER	1
#define ID_LINK_T_LEAVE 2

typedef struct
{
	UBYTE type;
	UBYTE next;
	UWORD pos;	// 8-bit fixed point.

} ID_Link;

#define ID_MAX_LINKS 128

ID_Link ID_link[ID_MAX_LINKS];
SLONG   ID_link_upto;

//
// One linked list per z-row of floor square.
// 0 is the NULL index
//

UBYTE ID_edge[ID_PLAN_SIZE];

//
// How to access the EDGE list. It is relative to
// the bounding box, so it takes up less memory.
//

#ifndef NDEBUG
#define ID_EDGE(z) (ID_check_edge((z)), ID_edge[(z) - ID_floor_z1])
#else
#define ID_EDGE(z) (ID_edge[(z) - ID_floor_z1])
#endif

void ID_check_edge(SLONG z)
{
	SLONG sz;

	ASSERT(WITHIN(z, ID_floor_z1, ID_floor_z2));

	sz = z - ID_floor_z1;

	ASSERT(WITHIN(sz, 0, ID_PLAN_SIZE - 1));
}

//
// Info for each room.
//

#define ID_ROOM_TYPE_NONE		0
#define ID_ROOM_TYPE_LOO		1
#define ID_ROOM_TYPE_KITCHEN	2
#define ID_ROOM_TYPE_LOUNGE		3
#define ID_ROOM_TYPE_LOBBY		4
#define ID_ROOM_TYPE_DINING		5
#define ID_ROOM_TYPE_WAREHOUSE	6
#define ID_ROOM_TYPE_OFFICE		7
#define ID_ROOM_TYPE_MEETING	8
#define ID_ROOM_TYPE_CORRIDOR	9
#define ID_ROOM_TYPE_BEDROOM	10
#define ID_ROOM_TYPE_NUMBER		11

#define ID_CORRIDOR_LENGTH	4

#define ID_ROOM_FLAG_RECTANGULAR	(1 << 0)	// If the room is a perfect rectangle.
#define ID_ROOM_FLAG_CORRIDOR		(1 << 1)	// Long and thin. 1 by ID_CORRIDOR_LENGTH or more.
#define ID_ROOM_FLAG_LOBBY			(1 << 2)	// Connect to the outside by a door.
#define ID_ROOM_FLAG_STAIRWAY		(1 << 3)	// Room contains stairs.
#define ID_ROOM_FLAG_SEEABLE		(1 << 4)


typedef struct
{
	UBYTE type;
	UBYTE flag;

	UWORD cam_x;
	UWORD cam_z;

	UBYTE num_doors;	// The number of internal doors this room has.
	UBYTE flat;			// Which flat this room is part of.

	//
	// The bounding box of the room. The are mapsquares
	// not points so it is inclusive.
	// 

	UBYTE x1;
	UBYTE z1;
	UBYTE x2;
	UBYTE z2;

} ID_Room;

#define ID_MAX_ROOMS 32

ID_Room ID_room[ID_MAX_ROOMS];	// Room 0 is unused, it is the NULL room.
SLONG   ID_room_upto;
SLONG   ID_flat_upto;

//
// The floor textures for each room type.
// 

#define ID_MAX_FLOOR_TEXTURES ID_ROOM_TYPE_NUMBER

UWORD ID_floor_texture[ID_MAX_FLOOR_TEXTURES];

//
// The stairs...
//

ID_Stair *ID_stair;
SLONG     ID_num_stairs;

//
// The current type of storey 
//

SLONG ID_storey_type;


//
// The furniture placed down indoors.
//

typedef struct
{
	UWORD x;
	UWORD z;
	UBYTE prim;
	UBYTE yaw;

} ID_Furn;

#define ID_MAX_FURN 128

ID_Furn ID_furn[ID_MAX_FURN];
SLONG   ID_furn_upto;



//
// Hardcoded size of the doorframes.
//

#define ID_FRAME_START  (0x30)
#define ID_FRAME_END    (0xd0)
#define ID_FRAME_HEIGHT (0xd0)



//
// Returns the texture for the given room.
//
// 'poly_number' refers to the hard-coded door frame.  It is which
// quad in the door frame is needed.
//
//	 +-----------+
//	 |\	   1    /|
//	 | \       / |
//	 |	+-----+  |
//	 |	||   ||  |
//	 |2 ||   || 3|
//	 |	||   ||  |
//		 4	 5 
//
// It is ignored for other texture types.
//

#define ID_TEXTURE_TYPE_FLOOR	0
#define ID_TEXTURE_TYPE_WALL	1
#define ID_TEXTURE_TYPE_DOOR	2
#define ID_TEXTURE_TYPE_WINDOW	3
#define ID_TEXTURE_TYPE_WALLTOP	4

//
// The size of a square texture.
//

#define ID_TEXTURE_SIZE         (1.0F)
#define ID_TEXTURE_FRAME_START  (ID_TEXTURE_SIZE *  3.0F * 0.0625F)
#define ID_TEXTURE_FRAME_END    (ID_TEXTURE_SIZE * 12.0F * 0.0625F)
#define ID_TEXTURE_FRAME_HEIGHT (ID_TEXTURE_SIZE *  3.0F * 0.0625F)
#define ID_TEXTURE_FRAME_DEPTH  (ID_TEXTURE_SIZE *  3.0F * 0.0625F)

//
// The texture page for inside buildings.
//

#define ID_TEXTURE_PAGE 6

typedef struct
{
	float u0, v0;
	float u1, v1;
	float u2, v2;
	float u3, v3;

} ID_Texture;

#define ID_MAX_TEXTURES 6

ID_Texture ID_texture[ID_MAX_TEXTURES] =
{
	{0.0, 0.0, ID_TEXTURE_SIZE, 0.0, 0.0, ID_TEXTURE_SIZE, ID_TEXTURE_SIZE, ID_TEXTURE_SIZE},	// Square.

	{0.0, 0.0, ID_TEXTURE_SIZE, 0.0, ID_TEXTURE_FRAME_START, ID_TEXTURE_FRAME_HEIGHT, ID_TEXTURE_FRAME_END, ID_TEXTURE_FRAME_HEIGHT},
	{0.0, 0.0, ID_TEXTURE_FRAME_START, ID_TEXTURE_FRAME_HEIGHT, 0.0, ID_TEXTURE_SIZE, ID_TEXTURE_FRAME_START, ID_TEXTURE_SIZE},
	{ID_TEXTURE_FRAME_END, ID_TEXTURE_FRAME_HEIGHT, ID_TEXTURE_SIZE, 0.0, ID_TEXTURE_FRAME_END, ID_TEXTURE_SIZE, ID_TEXTURE_SIZE, ID_TEXTURE_SIZE},
	{ID_TEXTURE_FRAME_START, ID_TEXTURE_FRAME_HEIGHT, ID_TEXTURE_FRAME_START + ID_TEXTURE_FRAME_DEPTH, ID_TEXTURE_FRAME_HEIGHT, ID_TEXTURE_FRAME_START, ID_TEXTURE_SIZE, ID_TEXTURE_FRAME_START + ID_TEXTURE_FRAME_DEPTH, ID_TEXTURE_SIZE},
	{ID_TEXTURE_FRAME_END - ID_TEXTURE_FRAME_DEPTH, ID_TEXTURE_FRAME_HEIGHT, ID_TEXTURE_FRAME_END, ID_TEXTURE_FRAME_HEIGHT, ID_TEXTURE_FRAME_END - ID_TEXTURE_FRAME_DEPTH, ID_TEXTURE_SIZE, ID_TEXTURE_FRAME_END, ID_TEXTURE_SIZE}
};

//
// The texture UWORD is a packed (baseu, basev, poly_number)
// 

#define ID_GET_TEXTURE_UWORD(baseu, basev, poly_number) (((baseu) << 12) | ((basev) << 8) | (poly_number))
#define ID_GET_TEXTURE_BASEU(texture)                   (((texture) >> 12) & 0x0f)
#define ID_GET_TEXTURE_BASEV(texture)                   (((texture) >>  8) & 0x0f)
#define ID_GET_TEXTURE_POLYNUMBER(texture)				(((texture) >> 	0) & 0xff)

UWORD ID_get_texture(SLONG room_type, SLONG texture_type, SLONG poly_number)
{
	UWORD ans;

	SLONG base_u;
	SLONG base_v;

	switch(room_type)
	{
		case ID_ROOM_TYPE_LOO:		base_u = 0;	base_v = 0; break;
		case ID_ROOM_TYPE_KITCHEN:	base_u = 4;	base_v = 0; break;
		case ID_ROOM_TYPE_LOUNGE:	base_u = 0;	base_v = 1; break;
		case ID_ROOM_TYPE_LOBBY:	base_u = 4;	base_v = 1; break;
		case ID_ROOM_TYPE_DINING:	base_u = 0;	base_v = 2; break;
		case ID_ROOM_TYPE_WAREHOUSE:
		case ID_ROOM_TYPE_OFFICE:
		case ID_ROOM_TYPE_MEETING:
		case ID_ROOM_TYPE_CORRIDOR:
		case ID_ROOM_TYPE_BEDROOM:	base_u = 4;	base_v = 2; break;

		default:
			ASSERT(0);
			break;
	}

	if (texture_type == ID_TEXTURE_TYPE_DOOR)
	{
		ASSERT(WITHIN(poly_number, 1, 5));
	}
	else
	{
		poly_number = 0;	// The normal square.
	}

	if (texture_type != ID_TEXTURE_TYPE_WALLTOP)
	{
		//
		// The different texture types are in an ordered line
		// on the texture
		//

		base_u += texture_type;
	}
	else
	{
		//
		// For walltops, always use texture (7,7).
		//

		base_u = 7;
		base_v = 7;
	}

	ans = ID_GET_TEXTURE_UWORD(base_u, base_v, poly_number);

	return ans;

}

//
// Returns the page.
//

SLONG ID_get_texture_uvs(
		UWORD texture,
		float *u0, float *v0,
		float *u1, float *v1,
		float *u2, float *v2,
		float *u3, float *v3)
{
	float u;
	float v;

	SLONG base_u;
	SLONG base_v;
	SLONG poly_number;
	SLONG page = 0;
#ifndef	PSX

	ID_Texture *it;

	base_u      = ID_GET_TEXTURE_BASEU(texture);
	base_v      = ID_GET_TEXTURE_BASEV(texture);
	poly_number = ID_GET_TEXTURE_POLYNUMBER(texture);

	ASSERT(WITHIN(poly_number, 0, ID_MAX_TEXTURES - 1));

	it = &ID_texture[poly_number];

	page  = ID_TEXTURE_PAGE * 64;
	page += base_v          * 8;
	page += base_u          * 1;

   *u0 = it->u0;
   *v0 = it->v0;
   *u1 = it->u1;
   *v1 = it->v1;
   *u2 = it->u2;
   *v2 = it->v2;
   *u3 = it->u3;
   *v3 = it->v3;

#endif
    return page;
}





//
// Random numbers for the ID module.
//

ULONG ID_rand_seed;

inline void ID_srand(ULONG seed)
{
	ID_rand_seed = seed;
}

inline ULONG ID_grand(void)
{
	return ID_rand_seed;
}

inline UWORD ID_rand(void)
{
	UWORD ans;

	ID_rand_seed *= 328573;
	ID_rand_seed += 123456789;

	ans = ID_rand_seed >> 7;

	return ans;
}



//
// Returns the index of a point at (x,y,z). This function
// will fail if the point could be used as packed (x,z).
//

UWORD ID_get_point_index(UWORD x, UWORD y, UWORD z)
{
	SLONG i;
	UWORD ans;

	//
	// Make sure that this point could not have been
	// used as a packed (x,z).
	//

	ASSERT(!((x & 0xff) == 0 && (z & 0xff) == 0 && y == 0));

	//
	// Look for a point in the array already at this position.
	//

	for (i = ID_point_upto - 1; i >= 0; i--)
	{
		if (ID_point[i].x == x &&
			ID_point[i].y == y &&
			ID_point[i].z == z)
		{
			return i;
		}
	}

	//
	// We have to create a new point.
	//

	ASSERT(WITHIN(ID_point_upto, 0, ID_MAX_POINTS - 1));

	ans = ID_point_upto++;

	ID_point[ans].x = x;
	ID_point[ans].y = y;
	ID_point[ans].z = z;

	return ans;
}


//
// Adds faces to the floorplan. If 'quad' then it is a quad otherwise
// it is a triangle. It adds it to square (mx, mz)
//

void ID_add_face_to_square(
		SLONG mx,
		SLONG mz,
		UWORD x[4],
		UWORD y[4],
		UWORD z[4],
		UWORD texture,
		UBYTE quad)
{
	SLONG i;
	SLONG j;
	SLONG num_points;

	UWORD point[4];
	UWORD flag;

	ID_Square *is;

	ASSERT(WITHIN(ID_face_upto, 1, ID_MAX_FACES - 1));

	
	//
	// How many points?
	//

	if (quad)
	{
		flag       = ID_FACE_FLAG_QUAD;
		num_points = 4;
	}
	else
	{
		flag       = 0;
		num_points = 3;
	}

	#ifndef NDEBUG

	//
	// Make sure that no two points are the same.
	//

	for (i = 1; i < num_points; i++)
	{
		for (j = i - 1; j >= 0; j--)
		{
			if (x[i] == x[j] &&
				y[i] == y[j] &&
				z[i] == z[j])
			{
				ASSERT(0);
			}
		}
	}

	#endif


	//
	// The points.
	//

	for (i = 0; i < num_points; i++)
	{
		if ((x[i] & 0xff) == 0 && (z[i] & 0xff) == 0 &&	y[i] == 0)
		{
			//
			// We can use this point index as packed (x,z)
			//

			flag    |= ID_FACE_FLAG_ONFLOOR0 << i;
			point[i] = (x[i] >> 8) | (z[i] >> 0);
		}
		else
		{
			point[i] = ID_get_point_index(x[i], y[i], z[i]);
		}
	}

	//
	// The mapsquare we must add this face to.
	//

	is = ID_FLOOR(mx, mz);

	if (!(is->flag & ID_FLOOR_FLAG_INSIDE))
	{
		TRACE("Warning: Adding face to square that is not inside.\n");
	}

	ASSERT(WITHIN(ID_face_upto, 1, ID_MAX_FACES - 1));

	ID_face[ID_face_upto].point[0] = point[0];
	ID_face[ID_face_upto].point[1] = point[1];
	ID_face[ID_face_upto].point[2] = point[2];
	ID_face[ID_face_upto].point[3] = point[3];
	ID_face[ID_face_upto].flag     = flag;
	ID_face[ID_face_upto].texture  = texture;
	ID_face[ID_face_upto].next     = is->next;

	is->next = ID_face_upto++;

	return;
}


void ID_clear_floorplan(void)
{
	SLONG i;

	SLONG x;
	SLONG z;

	//
	// Mark the whole floor as outside.
	//

	for (x = 0; x < ID_PLAN_SIZE; x++)
	{
		for (z = 0; z < ID_PLAN_SIZE; z++)
		{
			ID_floor[x][z].flag  = ID_FLOOR_FLAG_OUTSIDE;
			ID_floor[x][z].room  = 0;
			ID_floor[x][z].texid = 0;
			ID_floor[x][z].next  = 0;
		}
	}

	//
	// Clear the bounding box.
	//

	ID_floor_x1 = +INFINITY;
	ID_floor_z1 = +INFINITY;
	ID_floor_x2 = -INFINITY;
	ID_floor_z2 = -INFINITY;

	//
	// Clear all the walls and the 'get_type' function.
	//

	ID_wall_upto = 0;
	ID_get_type  = NULL;

	//
	// Clear all points and faces.
	//

	ID_point_upto = 0;
	ID_face_upto  = 1;

	//
	// Initialise the floor textures.
	//

	for (i = 1; i < ID_ROOM_TYPE_NUMBER; i++)
	{
		ID_floor_texture[i] = ID_get_texture(i, ID_TEXTURE_TYPE_FLOOR, 0);
	}
}

void ID_set_outline(SLONG x1, SLONG z1, SLONG x2, SLONG z2, SLONG id, SLONG num_blocks)
{
	ASSERT(WITHIN(ID_wall_upto, 0, ID_MAX_WALLS - 1));

	ASSERT(WITHIN(x1, 0, ID_FLOOR_SIZE - 1));
	ASSERT(WITHIN(z1, 0, ID_FLOOR_SIZE - 1));
	ASSERT(WITHIN(x2, 0, ID_FLOOR_SIZE - 1));
	ASSERT(WITHIN(z2, 0, ID_FLOOR_SIZE - 1));

	//
	// Add this wall to the array of walls.
	//

	ID_wall[ID_wall_upto].type       = ID_WALL_T_OUTSIDE;
	ID_wall[ID_wall_upto].door[0]    = 255;
	ID_wall[ID_wall_upto].door[1]    = 255;
	ID_wall[ID_wall_upto].door[2]    = 255;
	ID_wall[ID_wall_upto].x1         = x1;
	ID_wall[ID_wall_upto].z1         = z1;
	ID_wall[ID_wall_upto].x2         = x2;
	ID_wall[ID_wall_upto].z2         = z2;
	ID_wall[ID_wall_upto].id         = id;
	ID_wall[ID_wall_upto].num_blocks = num_blocks;

	ID_wall_upto += 1;

	//
	// Update the bounding box of the walls.
	//

	if (x1 < ID_floor_x1) {ID_floor_x1 = x1;}
	if (x2 < ID_floor_x1) {ID_floor_x1 = x2;}
	if (x1 > ID_floor_x2) {ID_floor_x2 = x1;}
	if (x2 > ID_floor_x2) {ID_floor_x2 = x2;}
	
	if (z1 < ID_floor_z1) {ID_floor_z1 = z1;}
	if (z2 < ID_floor_z1) {ID_floor_z1 = z2;}
	if (z1 > ID_floor_z2) {ID_floor_z2 = z1;}
	if (z2 > ID_floor_z2) {ID_floor_z2 = z2;}
}

void ID_set_get_type_func(SLONG (*get_type)(SLONG id, SLONG block))
{
	ID_get_type = get_type;
}



//
// Collision stuff we should know about...
//


extern UWORD next_col_vect;
extern UWORD next_col_vect_link;

//void insert_collision_vect(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,UBYTE prim_type,UBYTE prim_extra,SWORD face);
void remove_collision_vect(UWORD vect);


//
// These two functions must be called in pairs.
//
// Inserts collision vectors for the walls.
// Removes collision vectros for the walls.
//

SLONG ID_colvect_old_next_col_vect;
SLONG ID_colvect_old_next_col_vect_link;
SLONG ID_colvect_stuff_valid;

void ID_wall_colvects_insert()
{
	SLONG i;
	SLONG j;
	
	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG changed;

	SLONG doorway_x;
	SLONG doorway_z;

	UBYTE wall_height_inside;
	UBYTE wall_height_outside;

	ID_Wall *iw;

	#define MAX_POINTS 12

	struct
	{
		SLONG x;
		SLONG z;
		SLONG dist;

	}     point[MAX_POINTS];
	SLONG point_upto;

	//
	// Remember the end of the col_vect and col_vect_link arrays for
	// when we get rid of these colvects.
	//

	ID_colvect_old_next_col_vect      = next_col_vect;
	ID_colvect_old_next_col_vect_link = next_col_vect_link;
	ID_colvect_stuff_valid            = TRUE;
	
	if (ID_storey_type == ID_STOREY_TYPE_WAREHOUSE)
	{
		wall_height_inside  = 1;
		wall_height_outside = 2;
	}
	else
	{
		wall_height_inside  = 1;
		wall_height_outside = 1;
	}

	//
	// Insert colvects for each wall.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		switch(iw->type)
		{
			case ID_WALL_T_OUTSIDE:
				
				insert_collision_vect(
					iw->x2 << 8, INDOORS_HEIGHT_FLOOR, iw->z2 << 8,
					iw->x1 << 8, INDOORS_HEIGHT_FLOOR, iw->z1 << 8,
					STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_BUT_A_VALUE_TO_PUT_IN_THE_PRIM_TYPE_FIELD_OF_COLVECTS_GENERATED_BY_INSIDE_BUILDINGS,
					wall_height_outside, 0);

				break;

			case ID_WALL_T_INSIDE:

				point[0].x = iw->x1 << 8;
				point[0].z = iw->z1 << 8;

				point[1].x = iw->x2 << 8;
				point[1].z = iw->z2 << 8;

				point_upto = 2;

				dx = SIGN(iw->x2 - iw->x1);
				dz = SIGN(iw->z2 - iw->z1);

				for (j = 0; j < 3; j++)
				{
					if (iw->door[j] != 255)
					{
						doorway_x  = iw->x1 + dx * iw->door[j] << 8;
						doorway_z  = iw->z1 + dz * iw->door[j] << 8;

						point[point_upto + 0].x = doorway_x + dx * ID_FRAME_START;
						point[point_upto + 0].z = doorway_z + dz * ID_FRAME_START;

						point[point_upto + 1].x = doorway_x + dx * ID_FRAME_END;
						point[point_upto + 1].z = doorway_z + dz * ID_FRAME_END;

						point_upto += 2;
					}
				}

				//
				// Find the distance of each point from point 1.
				//

				for (j = 0; j < point_upto; j++)
				{
					dx = point[j].x - (iw->x1 << 8);
					dz = point[j].z - (iw->z1 << 8);

					dist = abs(dx) + abs(dz);

					point[j].dist = dist;
				}

				//
				// Sort the points in order of distance from the first point.
				//
				
				do
				{
					changed = FALSE;

					for (j = 0; j < point_upto - 1; j++)
					{
						if (point[j + 0].dist > point[j + 1].dist)
						{
							SWAP(point[j + 0].x,    point[j + 1].x);
							SWAP(point[j + 0].z,    point[j + 1].z);
							SWAP(point[j + 0].dist, point[j + 1].dist);

							changed = TRUE;
						}
					}

				} while(changed);

				for (j = 0; j < point_upto - 1; j += 2)
				{
					insert_collision_vect(
						point[j + 0].x, INDOORS_HEIGHT_FLOOR, point[j + 0].z,
						point[j + 1].x, INDOORS_HEIGHT_FLOOR, point[j + 1].z,
						STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_BUT_A_VALUE_TO_PUT_IN_THE_PRIM_TYPE_FIELD_OF_COLVECTS_GENERATED_BY_INSIDE_BUILDINGS,
						wall_height_inside, 0);

					insert_collision_vect(
						point[j + 1].x, INDOORS_HEIGHT_FLOOR, point[j + 1].z,
						point[j + 0].x, INDOORS_HEIGHT_FLOOR, point[j + 0].z,
						STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_BUT_A_VALUE_TO_PUT_IN_THE_PRIM_TYPE_FIELD_OF_COLVECTS_GENERATED_BY_INSIDE_BUILDINGS,
						wall_height_inside, 0);
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}
}

void ID_wall_colvects_remove()
{
	SLONG i;

	ASSERT(ID_colvect_stuff_valid);

	for (i = ID_colvect_old_next_col_vect; i < next_col_vect; i++)
	{
		remove_collision_vect(i);
	}

	//
	// We are not using these any more...
	//

	next_col_vect          = ID_colvect_old_next_col_vect;
	next_col_vect_link     = ID_colvect_old_next_col_vect_link;
	ID_colvect_stuff_valid = FALSE;
}

//
// Returns TRUE if the given wall lies on the perimeter of the given
// room. If it does then the function gives back the section of wall
// that lies on the edge of the room. The sense of the wall segment
// is such that the room lies of the right of the wall.
//

SLONG ID_is_wall_on_room_perim(
		SLONG wall,
		SLONG room,
		SLONG *x1,
		SLONG *z1,
		SLONG *x2,
		SLONG *z2)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG dmx;
	SLONG dmz;

	SLONG wx1;
	SLONG wz1;
	SLONG wx2;
	SLONG wz2;

	SLONG length;

	SLONG found_start;
	SLONG flip_ends;

	ID_Wall *iw;

	ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

	iw = &ID_wall[wall];

	dx = iw->x2 - iw->x1;
	dz = iw->z2 - iw->z1;

	if (dx && dz)
	{
		//
		// Only outside walls should be non-orthogonal.
		//

		ASSERT(iw->type == ID_WALL_T_OUTSIDE);

		//
		// This line should only lie on the perimeter of
		// one room.
		//

		length = abs(dx) + abs(dz);

		mx = iw->x1 << 8;
		mz = iw->z1 << 8;

		mx += (dx << 7) / length;
		mz += (dz << 7) / length;

		dx = SIGN(dx);
		dz = SIGN(dz);

		dmx = -SIGN(dz);
		dmz =  SIGN(dx);

		mx += dmx;
		mz += dmz;

		mx >>= 8;
		mz >>= 8;

		//
		// This square should be inside and part
		// of a room.
		//

		ASSERT(ID_FLOOR(mx,mz)->flag & ID_FLOOR_FLAG_INSIDE);
		ASSERT(ID_FLOOR(mx,mz)->room != 0);
		
		if (ID_FLOOR(mx,mz)->room == room)
		{
			//
			// Outside lines are always in clockwise order.
			//

			*x1 = iw->x1;
			*z1 = iw->z1;
			*x2 = iw->x2;
			*z2 = iw->z2;

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	wx1 = iw->x1;
	wz1 = iw->z1;
	wx2 = iw->x2;
	wz2 = iw->z2;

	if (wx1 > wx2) {SWAP(wx1, wx2);}
	if (wz1 > wz2) {SWAP(wz1, wz2);}

	dx = SIGN(wx2 - wx1);
	dz = SIGN(wz2 - wz1);

	ASSERT(!dx || !dz);
	ASSERT(dx >= 0 && dz >= 0);

	found_start = FALSE;
	flip_ends   = FALSE;

	for (x = wx1, z = wz1; x != wx2 || z != wz2; x += dx, z += dz)
	{
		#define ID_ROOM_SQUARE(x,z,r) (WITHIN((x), ID_floor_x1, ID_floor_x2) && WITHIN((z), ID_floor_z1, ID_floor_z2) && ID_FLOOR((x),(z))->room == (r))

		if (dx == 1 && dz == 0)
		{
			if (ID_ROOM_SQUARE(x, z, room))
			{
				if (!found_start)
				{
					*x1 = x;
					*z1 = z;

					found_start = TRUE;
					flip_ends   = FALSE;
				}

				*x2 = x + 1;
				*z2 = z;
			}

			if (ID_ROOM_SQUARE(x, z - 1, room))
			{
				if (!found_start)
				{
					*x1 = x;
					*z1 = z;

					found_start = TRUE;
					flip_ends   = TRUE;
				}

				*x2 = x + 1;
				*z2 = z;
			}
		}
		else
		if (dx == 0 && dz == 1)
		{
			if (ID_ROOM_SQUARE(x, z, room))
			{
				if (!found_start)
				{
					*x1 = x;
					*z1 = z;

					found_start = TRUE;
					flip_ends   = TRUE;
				}

				*x2 = x;
				*z2 = z + 1;
			}

			if (ID_ROOM_SQUARE(x - 1, z, room))
			{
				if (!found_start)
				{
					*x1 = x;
					*z1 = z;

					found_start = TRUE;
					flip_ends   = FALSE;
				}

				*x2 = x;
				*z2 = z + 1;
			}
		}
	}

	if (flip_ends)
	{
		//
		// We have to flip the direction of the line segment,
		// to make the room be on the right-hand-side of
		// the line.
		//

		SWAP(*x1, *x2);
		SWAP(*z1, *z2);
	}

	return found_start;
}


//
// Creates faces for a mapsquare.
//

#define ID_WALL_FACES_OUT_WINDOW	1	// A window to the outside.
#define ID_WALL_FACES_OUT_FRAME		2	// A door to the outside.
#define ID_WALL_FACES_NORMAL		3
#define ID_WALL_FACES_FRAME			4	// A doorframe without a door.
#define ID_WALL_FACES_UPPER			5	// The upper storey of a warehouse.

//
// The function needs four control points.
//
// 0O--------O1		Given in this order.
//	|\	     |
//	|2O------O3
//	| 	     |
//	|	     |
//	|	     |
//	+--------+
//
// It fits the faces around the control points. Control points 1 and
// 2 are normally be on the corners of mapsquares.
//

void ID_create_mapsquare_faces(
		UBYTE map_x,
		UBYTE map_z,
		UWORD cpx[4],
		UWORD cpz[4],
		SLONG room_type,
		SLONG face_type,
		SLONG generate_top_wall_quad)
{
	SLONG i;

	UWORD x[4];
	UWORD y[4];
	UWORD z[4];

	UWORD texture;

	SLONG tu;
	SLONG tv;
	SLONG page;

	if (generate_top_wall_quad)
	{
		//
		// The top of the wall quad.
		//

		for (i = 0; i < 4; i++)
		{
			x[i] = cpx[i];
			z[i] = cpz[i];

			y[i] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

			if (face_type == ID_WALL_FACES_UPPER)
			{
				y[i] += 256;
			}
		}

		ID_add_face_to_square(map_x, map_z, x, y, z, ID_get_texture(room_type, ID_TEXTURE_TYPE_WALLTOP, 0), TRUE);
	}

	if (face_type == ID_WALL_FACES_FRAME ||
		face_type == ID_WALL_FACES_OUT_FRAME)
	{
		UWORD dpx[4];
		UWORD dpz[4];

		//
		// Hard-code a simple door-frame.
		//

		dpx[0] = cpx[0] + (((cpx[1] - cpx[0]) * ID_FRAME_START) >> 8);
		dpz[0] = cpz[0] + (((cpz[1] - cpz[0]) * ID_FRAME_START) >> 8);

		dpx[1] = cpx[0] + (((cpx[1] - cpx[0]) * ID_FRAME_END) >> 8);
		dpz[1] = cpz[0] + (((cpz[1] - cpz[0]) * ID_FRAME_END) >> 8);

		dpx[2] = cpx[2] + (((cpx[3] - cpx[2]) * ID_FRAME_START) >> 8);
		dpz[2] = cpz[2] + (((cpz[3] - cpz[2]) * ID_FRAME_START) >> 8);

		dpx[3] = cpx[2] + (((cpx[3] - cpx[2]) * ID_FRAME_END) >> 8);
		dpz[3] = cpz[2] + (((cpz[3] - cpz[2]) * ID_FRAME_END) >> 8);

		//
		// Here goes!
		//

		//
		// Sides of door frame.
		//

		x[0] = dpx[2];
		z[0] = dpz[2];
		y[0] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[1] = dpx[0];
		z[1] = dpz[0];
		y[1] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[2] = dpx[2];
		z[2] = dpz[2];
		y[2] = INDOORS_HEIGHT_FLOOR + 0;

		x[3] = dpx[0];
		z[3] = dpz[0];
		y[3] = INDOORS_HEIGHT_FLOOR + 0;

		ID_add_face_to_square(map_x, map_z, x, y, z, ID_get_texture(room_type, ID_TEXTURE_TYPE_DOOR, 4), TRUE);

		x[0] = dpx[1];
		z[0] = dpz[1];
		y[0] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[1] = dpx[3];
		z[1] = dpz[3];
		y[1] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[2] = dpx[1];
		z[2] = dpz[1];
		y[2] = INDOORS_HEIGHT_FLOOR + 0;

		x[3] = dpx[3];
		z[3] = dpz[3];
		y[3] = INDOORS_HEIGHT_FLOOR + 0;

		ID_add_face_to_square(map_x, map_z, x, y, z, ID_get_texture(room_type, ID_TEXTURE_TYPE_DOOR, 5), TRUE);

		//
		// Top of door frame.
		//

		x[0] = cpx[2];
		z[0] = cpz[2];
		y[0] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

		x[1] = cpx[3];
		z[1] = cpz[3];
		y[1] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

		x[2] = dpx[2];
		z[2] = dpz[2];
		y[2] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[3] = dpx[3];
		z[3] = dpz[3];
		y[3] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		ID_add_face_to_square(map_x, map_z, x, y, z, ID_get_texture(room_type, ID_TEXTURE_TYPE_DOOR, 1), TRUE);

		x[0] = cpx[2];
		z[0] = cpz[2];
		y[0] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

		x[1] = dpx[2];
		z[1] = dpz[2];
		y[1] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[2] = cpx[2];
		z[2] = cpz[2];
		y[2] = INDOORS_HEIGHT_FLOOR + 0;

		x[3] = dpx[2];
		z[3] = dpz[2];
		y[3] = INDOORS_HEIGHT_FLOOR + 0;

		ID_add_face_to_square(map_x, map_z, x, y, z, ID_get_texture(room_type, ID_TEXTURE_TYPE_DOOR, 2), TRUE);

		x[0] = dpx[3];
		z[0] = dpz[3];
		y[0] = INDOORS_HEIGHT_FLOOR + ID_FRAME_HEIGHT;

		x[1] = cpx[3];
		z[1] = cpz[3];
		y[1] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

		x[2] = dpx[3];
		z[2] = dpz[3];
		y[2] = INDOORS_HEIGHT_FLOOR + 0;

		x[3] = cpx[3];
		z[3] = cpz[3];
		y[3] = INDOORS_HEIGHT_FLOOR + 0;

		ID_add_face_to_square(map_x, map_z, x, y, z, ID_get_texture(room_type, ID_TEXTURE_TYPE_DOOR, 3), TRUE);
	}
	else
	{
		//
		// The front of the wall.
		//

		x[0] = cpx[2];
		z[0] = cpz[2];
		y[0] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

		x[1] = cpx[3];
		z[1] = cpz[3];
		y[1] = INDOORS_HEIGHT_FLOOR + ID_CEILING_HEIGHT;

		x[2] = cpx[2];
		z[2] = cpz[2];
		y[2] = INDOORS_HEIGHT_FLOOR + 0;

		x[3] = cpx[3];
		z[3] = cpz[3];
		y[3] = INDOORS_HEIGHT_FLOOR + 0;

		if (face_type == ID_WALL_FACES_UPPER)
		{
			for (i = 0; i < 4; i++)
			{
				y[i] += 256;
			}
		}

		if (face_type == ID_WALL_FACES_OUT_WINDOW)
		{
			texture = ID_get_texture(room_type, ID_TEXTURE_TYPE_WINDOW, 0);
		}
		else
		{
			texture = ID_get_texture(room_type, ID_TEXTURE_TYPE_WALL, 0);
		}

		ID_add_face_to_square(map_x, map_z, x, y, z, texture, TRUE);
	}
}



//
// Returns the perimeter of the given room.  The first
// point appears again at the end of the array.  Returns FALSE if
// it fails to find a perimeter!
//

typedef struct
{
	UBYTE x;
	UBYTE z;

} ID_Perim;

#define ID_MAX_PERIMS 32

ID_Perim ID_perim[ID_MAX_PERIMS];
SLONG    ID_perim_upto;

SLONG ID_calc_room_perim(SLONG room)
{
	SLONG i;

	SLONG sx1, sz1;
	SLONG sx2, sz2;

	SLONG tries;

	#define MAX_SEGS 32

	struct
	{
		UBYTE x1, z1;
		UBYTE x2, z2;

	} seg[MAX_SEGS];

	SLONG seg_upto = 0;

	//
	// Find all the lines that lie on the edge of the given room.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		if (ID_is_wall_on_room_perim(i, room,
				&sx1, &sz1,
				&sx2, &sz2))
		{
			ASSERT(WITHIN(seg_upto, 0, MAX_SEGS - 1));

			//
			// Remember all the segments.
			//

			seg[seg_upto].x1 = sx1;
			seg[seg_upto].z1 = sz1;
			seg[seg_upto].x2 = sx2;
			seg[seg_upto].z2 = sz2;

			seg_upto += 1;
		}
	}

	//
	// There must be at least three lines!
	//

	ASSERT(seg_upto >= 3);

	//
	// Build the perimeter.
	//

	ID_perim[0].x = seg[0].x1;
	ID_perim[0].z = seg[0].z1;

	ID_perim[1].x = seg[0].x2;
	ID_perim[1].z = seg[0].z2;

	ID_perim_upto = 1;

	tries = 0;

	while(
		ID_perim[ID_perim_upto].x != ID_perim[0].x ||
		ID_perim[ID_perim_upto].z != ID_perim[0].z)
	{
		//
		// Look for the next segment.
		//

		for (i = 0; i < seg_upto; i++)
		{
			if (seg[i].x1 == ID_perim[ID_perim_upto].x &&
				seg[i].z1 == ID_perim[ID_perim_upto].z)
			{
				ID_perim_upto += 1;

				if (!WITHIN(ID_perim_upto, 0, ID_MAX_PERIMS - 1))
				{
					//
					// ERROR! FAILURE!
					//

					return FALSE;
				}

				ID_perim[ID_perim_upto].x = seg[i].x2;
				ID_perim[ID_perim_upto].z = seg[i].z2;
			}
		}

		if (tries++ > ID_MAX_PERIMS)
		{
			//
			// ERROR! FAILURE!
			//

			return FALSE;
		}
	}

	ID_perim_upto += 1;

	return TRUE;
}


//
// Returns TRUE if it can find the perimeter for all the rooms!
//

SLONG ID_i_can_find_the_room_perims(void)
{
	SLONG i;

	for (i = 1; i < ID_room_upto; i++)
	{
		if (!ID_calc_room_perim(i))
		{
			return FALSE;
		}
	}

	return TRUE;
}


//
// Given the segment of wall on the perimeter of a room, this function returns
// the wall it is a part of, and two numbers (bwmul, bwadd) that you can use to map
// a part of the segment to the corresponding part of the wall.
//
//  Wallsegment number = (blocksegment number) * bwmul + bwadd
//
// It returns number of blocks in the segment.  If the segment is
// orthogonal, that is easy.  It the segment is non-othogonal, it must
// be a whole outside wall.  (Non-orthogonal outside walls are never
// split up.)
//

void ID_find_segment_wall(
		SLONG  x1,
		SLONG  z1,
		SLONG  x2,
		SLONG  z2,
		SLONG *wall,
		SLONG *bwmul,
		SLONG *bwadd,
		SLONG *length)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG wx1;
	SLONG wz1;

	SLONG wx2;
	SLONG wz2;

	ID_Wall *iw;

	//
	// Find the wall that this segment belongs to.  If the wall
	// is an outside wall, we fill in block_start and block_dir
	// that let us work out which block of the wall corresponds
	// to each block of the segment.
	//

	dx = x2 - x1;
	dz = z2 - z1;

	if (dx && dz)
	{
		//
		// This is a non-orthogonal so it must be an outside wall.
		//

		for (i = 0; i < ID_wall_upto; i++)
		{
			iw = &ID_wall[i];

			ASSERT(ID_wall[i].type == ID_WALL_T_OUTSIDE);

			if ((iw->x1 == x1 && iw->z1 == z1 && iw->x2 == x2 && iw->z2 == z2) ||
				(iw->x1 == x1 && iw->z1 == z1 && iw->x2 == x2 && iw->z2 == z2))
			{
				//
				// The length of the segment is the length of the wall.
				//

			   *wall   = i;
			   *bwmul  = 1;
			   *bwadd  = 0;
			   *length = iw->num_blocks;

				return;
			}
		}

		//
		// Failed to find a wall.
		//

		ASSERT(0);
	}
	else
	{
		for (i = 0; i < ID_wall_upto; i++)
		{
			iw = &ID_wall[i];

			if (dx == 0 && iw->x1 == x1 && iw->x2 == x2)
			{
				wz1 = iw->z1;
				wz2 = iw->z2;

				if (wz1 > wz2) {SWAP(wz1, wz2);}

				if (WITHIN(z1, wz1, wz2) &&
					WITHIN(z2, wz1, wz2))
				{
					//
					// This is the wall we want.
					//

				   *bwmul = SIGN(dz) * SIGN(iw->z2 - iw->z1);

					if (*bwmul == 1)
					{
						*bwadd = abs(z1 - iw->z1);
					}
					else
					{
						*bwadd = abs(z1 - iw->z1) - 1;
					}

					goto found_wall;
				}
			}

			if (dz == 0 && iw->z1 == z1 && iw->z2 == z2)
			{
				wx1 = iw->x1;
				wx2 = iw->x2;

				if (wx1 > wx2) {SWAP(wx1, wx2);}

				if (WITHIN(x1, wx1, wx2) &&
					WITHIN(x2, wx1, wx2))
				{
					//
					// This is the wall we want.
					//

				   *bwmul = SIGN(dx) * SIGN(iw->x2 - iw->x1);

					if (*bwmul == 1)
					{
						*bwadd = abs(x1 - iw->x1);
					}
					else
					{
						*bwadd = abs(x1 - iw->x1) - 1;
					}

					goto found_wall;
				}
			}
		}

		//
		// Failed to find a wall.
		//

		ASSERT(0);

	  found_wall:;

		//
		// The length of this segment.
		//

	  *wall   = i;
	  *length = MAX(abs(dx), abs(dz));

	   return;
	}
}
			






#if YOU_WANT_THIN_WALLS

//
// Adds faces for the given wall.
//

void ID_add_wall_faces(SLONG wall)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG x1, y1, z1;
	SLONG x2, y2, z2;

	SLONG dmx;
	SLONG dmz;

	SLONG mx;
	SLONG mz;

	UWORD px[4];
	UWORD py[4];
	UWORD pz[4];
	UWORD texture;
	UBYTE quad;

	ID_Wall *iw;

	ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

	iw = &ID_wall[wall];

	//
	// Each block in turn.
	//

	dx = iw->x2 - iw->x1 << 16;
	dz = iw->z2 - iw->z1 << 16;

	dx /= iw->num_blocks;
	dz /= iw->num_blocks;

	x = iw->x1 << 16;
	z = iw->z1 << 16;

	y1 = 0;
	y2 = ID_CEILING_HEIGHT << 8;	// Ceiling height is only 8-bit.

	//
	// A tiny vector pointing to the right of the wall.
	//

	dmx = SIGN(-dz);
	dmz = SIGN( dx);
	
	for (i = 0; i < iw->num_blocks; i++)
	{
		x1 = x;
		z1 = z;

		x2 = x + dx;
		z2 = z + dz;

		//
		// Which mapsquare?
		//

		mx = x1 + x2 >> 1;
		mz = z1 + z2 >> 1;
		
		mx += dmx;
		mz += dmz;

		mx >>= 16;
		mz >>= 16;

		//
		// Just add one quad per block for now.
		//

		px[0] = x1 >> 8;
		py[0] = y2 >> 8;
		pz[0] = z1 >> 8;

		px[1] = x2 >> 8;
		py[1] = y2 >> 8;
		pz[1] = z2 >> 8;

		px[2] = x1 >> 8;
		py[2] = y1 >> 8;
		pz[2] = z1 >> 8;

		px[3] = x2 >> 8;
		py[3] = y1 >> 8;
		pz[3] = z2 >> 8;

		texture = 15;
		quad    = TRUE;
		
		ID_add_face_to_square(mx, mz, px, py, pz, texture, quad);

		if (iw->type == ID_WALL_T_INSIDE)
		{
			//
			// Inside walls are double-sided.
			//

			mx = x1 + x2 >> 1;
			mz = z1 + z2 >> 1;

			mx -= dmx;
			mz -= dmz;

			mx >>= 16;
			mz >>= 16;

			//
			// Just add one quad per block for now.
			//

			px[0] = x2 >> 8;
			py[0] = y2 >> 8;
			pz[0] = z2 >> 8;

			px[1] = x1 >> 8;
			py[1] = y2 >> 8;
			pz[1] = z1 >> 8;

			px[2] = x2 >> 8;
			py[2] = y1 >> 8;
			pz[2] = z2 >> 8;

			px[3] = x1 >> 8;
			py[3] = y1 >> 8;
			pz[3] = z1 >> 8;

			texture = 14;
			quad    = TRUE;
			
			ID_add_face_to_square(mx, mz, px, py, pz, texture, quad);
		}

		//
		// Next block.
		//

		x += dx;
		z += dz;
	}
}

#else

//
// Adds faces for the given room.
//

void ID_add_room_faces(SLONG room)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG px;
	SLONG pz;

	SLONG nx;
	SLONG nz;

	SLONG dx;
	SLONG dz;

	SLONG vx;
	SLONG vz;

	SLONG mx;
	SLONG mz;

	SLONG bw;
	SLONG wall;
	SLONG bwmul;
	SLONG bwadd;
	SLONG length;
	SLONG vlen;
	SLONG result;

	UWORD cpx[4];
	UWORD cpz[4];

	SLONG first_cpx;
	SLONG first_cpz;

	SLONG last_cpx;
	SLONG last_cpz;

	SLONG block_type;
	SLONG wall_faces_type;

	ID_Wall *iw;

	//
	// Work out the perimeter of the room.
	//

	result = ID_calc_room_perim(room);

	//
	// This has to work!
	//

	ASSERT(result);

	//
	// Go around the perimeter.
	//

	for (i = 0; i < ID_perim_upto - 1; i++)
	{
		//
		// This line segment.
		//

		x1 = ID_perim[i + 0].x;
		z1 = ID_perim[i + 0].z;
		x2 = ID_perim[i + 1].x;
		z2 = ID_perim[i + 1].z;

		//
		// The previous point.
		//

		if (i == 0)
		{
			px = ID_perim[ID_perim_upto - 2].x;
			pz = ID_perim[ID_perim_upto - 2].z;
		}
		else
		{
			px = ID_perim[i - 1].x;
			pz = ID_perim[i - 1].z;
		}

		//
		// The next point.
		//

		if (i + 2 == ID_perim_upto)
		{
			nx = ID_perim[1].x;
			nz = ID_perim[1].z; 
		}
		else
		{
			nx = ID_perim[i + 2].x;
			nz = ID_perim[i + 2].z;
		}

		ID_find_segment_wall(x1, z1, x2, z2,
			&wall,
			&bwmul,
			&bwadd,
			&length);

		ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

		iw = &ID_wall[wall];

		//
		// Find the position of the first and last controls points that
		// are not along the wall. These will be at funny positions that
		// depend on the angle this segment makes with the previous and
		// the next segments.
		//

		//
		// We can use one of Mike's functions here.
		//

		void calc_new_corner_point(
				SLONG  x1, SLONG z1,
				SLONG  x2, SLONG z2,
				SLONG  x3, SLONG z3,
				SLONG  width,
				SLONG *res_x,
				SLONG *res_z);

		//
		// Use fixed-point 8 from now on.
		//

		ASSERT(ELE_SHIFT == 8);

		px <<= 8; pz <<= 8;
		x1 <<= 8; z1 <<= 8;
		x2 <<= 8; z2 <<= 8;
		nx <<= 8; nz <<= 8;

		calc_new_corner_point(x2, z2, x1, z1, px, pz, ID_WALL_WIDTH, &first_cpx, &first_cpz);
		calc_new_corner_point(nx, nz, x2, z2, x1, z1, ID_WALL_WIDTH, &last_cpx,  &last_cpz);

		dx = x2 - x1;
		dz = z2 - z1;

		dx /= length;
		dz /= length;

		vx = -dz * ID_WALL_WIDTH >> 8;
		vz =  dx * ID_WALL_WIDTH >> 8;

		//
		// Each block in turn.
		//

		x = x1;
		z = z1;

		for (j = 0; j < length; j++)
		{
			//
			// Which block along the wall does this segment lie?
			//
			
			bw = j * bwmul + bwadd;

			//
			// What is the type of this block?
			//

			if (iw->type == ID_WALL_T_OUTSIDE)
			{
				//
				// Use the call-back function.
				//

				block_type = ID_get_type(iw->id, bw);

				switch(block_type)
				{
					case ID_BLOCK_TYPE_WALL:   wall_faces_type = ID_WALL_FACES_NORMAL;     break;
					case ID_BLOCK_TYPE_WINDOW: wall_faces_type = ID_WALL_FACES_OUT_WINDOW; break;
					case ID_BLOCK_TYPE_DOOR:   wall_faces_type = ID_WALL_FACES_OUT_FRAME;  break;
					default:
						ASSERT(0);
						break;
				}
			}
			else
			{
				//
				// Just use the normal...
				//

				wall_faces_type = ID_WALL_FACES_NORMAL;

				if (iw->door[0] == bw ||
					iw->door[1] == bw ||
					iw->door[2] == bw)
				{
					wall_faces_type = ID_WALL_FACES_FRAME;
				}
			}

			cpx[0] = x;
			cpz[0] = z;

			cpx[1] = x + dx;
			cpz[1] = z + dz;

			cpx[2] = x + vx;
			cpz[2] = z + vz;

			cpx[3] = x + dx + vx;
			cpz[3] = z + dz + vz;

			//
			// The special cases at the beginning and end of the segment.
			//

			if (j == 0)
			{
				cpx[2] = first_cpx;
				cpz[2] = first_cpz;
			}

			if (j == length - 1)
			{
				cpx[3] = last_cpx;
				cpz[3] = last_cpz;
			}

			//
			// Add the faces around the control points to the mapsquare
			// inbetween control points 2 and 3.
			//

			mx = cpx[2] + cpx[3] >> 1;
			mz = cpz[2] + cpz[3] >> 1;

			mx >>= 8;
			mz >>= 8;

			ASSERT(ID_FLOOR(mx,mz)->room == room);

			
			if (ID_storey_type == ID_STOREY_TYPE_WAREHOUSE &&
				iw->type       == ID_WALL_T_OUTSIDE)
			{
				ID_create_mapsquare_faces(mx, mz, cpx, cpz, ID_room[room].type, wall_faces_type,     FALSE);
				ID_create_mapsquare_faces(mx, mz, cpx, cpz, ID_room[room].type, ID_WALL_FACES_UPPER, TRUE);
			}
			else
			{
				ID_create_mapsquare_faces(mx, mz, cpx, cpz, ID_room[room].type, wall_faces_type, TRUE);
			}

			x += dx;
			z += dz;
		}
	}
}

#endif


//
// Clears all the inside wall info.
//

void ID_clear_inside_walls()
{
	SLONG i;
	SLONG x;
	SLONG z;

	ID_Square *is;

	for (x = ID_floor_x1; x <= ID_floor_x2; x++)
	for (z = ID_floor_z1; z <= ID_floor_z2; z++)
	{
		is = ID_FLOOR(x, z);

		is->flag &= ~(
			ID_FLOOR_FLAG_ON_WALL   |
			ID_FLOOR_FLAG_DOORMAT   |
			ID_FLOOR_FLAG_INMAT     |
			ID_FLOOR_FLAG_FURNISHED |
			ID_FLOOR_FLAG_WALL_XS   |
			ID_FLOOR_FLAG_WALL_XL   |
			ID_FLOOR_FLAG_WALL_ZS   |
			ID_FLOOR_FLAG_WALL_ZL);

		is->room = 0;
	}

	ASSERT(WITHIN(ID_wall_upto, 0, ID_MAX_WALLS));

	while(ID_wall_upto > 0 && ID_wall[ID_wall_upto - 1].type == ID_WALL_T_INSIDE)
	{
		ID_wall_upto -= 1;
	}
}


//
// Picks a random place to start a wall.  It prefers
// corners.  Returns TRUE on sucess or FALSE if it
// couldn't find a suitable place.
//

SLONG ID_get_wall_start(SLONG *x, SLONG *z)
{
	SLONG wall;
	SLONG dx;
	SLONG dz;
	SLONG length;
	SLONG along;
	SLONG tries;

	//
	// How many tries do we have to find somewhere suitable?
	//

	#define ID_WALL_NUM_TRIES 3

	tries = ID_WALL_NUM_TRIES;

	while(tries--)
	{
		if ((ID_rand() & 0xff) < 128)
		{
			//
			// Choose the end of a wall.
			//

			wall = ID_rand() % ID_wall_upto;

			if (ID_rand() & 0x100)
			{
				*x = ID_wall[wall].x1;
				*z = ID_wall[wall].z1;
			}
			else
			{
				*x = ID_wall[wall].x2;
				*z = ID_wall[wall].z2;
			}

			return TRUE;
		}
		else
		{
			//
			// Choose somewhere along the wall.
			//

			wall = ID_rand() % ID_wall_upto;

			dx = ID_wall[wall].x2 - ID_wall[wall].x1;
			dz = ID_wall[wall].z2 - ID_wall[wall].z1;

			if (dx && dz && abs(dx) != abs(dz))
			{
				//
				// Don't start new walls along slanty ones, except
				// diagonals.
				//
			}
			else
			{
				length = MAX(abs(dx), abs(dz));
				along  = ID_rand() % length;

				dx = SIGN(dx);
				dz = SIGN(dz);
				
				*x = ID_wall[wall].x1 + along * dx;
				*z = ID_wall[wall].z1 + along * dz;

				return TRUE;
			}
		}
	}

	return FALSE;
}

//
// Puts in the collision bits for the given wall.
//

void ID_generate_collision_bits(SLONG wall)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG x1, z1;
	SLONG x2, z2;

	ID_Wall *iw;

	ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

	iw = &ID_wall[wall];

	x1 = iw->x1;
	z1 = iw->z1;

	x2 = iw->x2;
	z2 = iw->z2;

	if (x1 > x2) {SWAP(x1, x2);}
	if (z1 > z2) {SWAP(z1, z2);}

	dx = x2 - x1;
	dz = z2 - z1;

	dx = SIGN(dx);
	dz = SIGN(dz);

	if (dx && dz)
	{
		//
		// Don't insert ollision bits for non-orthogonal walls.
		//

		TRACE("Cannot insert collision bits for non-orthogonal walls.\n");

		return;
	}

	x = x1;
	z = z1;

	#define ID_IS_VALID(x,z) (WITHIN((x), ID_floor_x1, ID_floor_x2 - 1) && WITHIN((z), ID_floor_z1, ID_floor_z2 - 1))

	while(!(x == x2 && z == z2))
	{
		if (dx == 0 && dz == 1)
		{
			if (ID_IS_VALID(x, z)) 
			{
				ID_FLOOR(x, z)->flag |= ID_FLOOR_FLAG_WALL_XS;
			}

			if (ID_IS_VALID(x - 1, z))
			{
				ID_FLOOR(x - 1, z)->flag |= ID_FLOOR_FLAG_WALL_XL;
			}
		}
		else
		if (dx == 1 && dz == 0)
		{
			if (ID_IS_VALID(x, z)) 
			{
				ID_FLOOR(x, z)->flag |= ID_FLOOR_FLAG_WALL_ZS;
			}

			if (ID_IS_VALID(x, z - 1))
			{
				ID_FLOOR(x, z - 1)->flag |= ID_FLOOR_FLAG_WALL_ZL;
			}
		}
		else
		{
			ASSERT(0);
		}

		x += dx;
		z += dz;
	}
}

//
// Sets the ON_WALL bits for the given wall.
//

void ID_generate_on_wall_bits(SLONG wall)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG dxdz;

	SLONG mx;
	SLONG mz;

	SLONG x1, z1;
	SLONG x2, z2;

	ID_Wall *iw;

	ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

	iw = &ID_wall[wall];

	x1 = iw->x1;
	z1 = iw->z1;

	x2 = iw->x2;
	z2 = iw->z2;

	if (z1 == z2)
	{
		//
		// Easy!
		//

		if (x1 > x2) {SWAP(x1, x2);}

		for (x = x1; x <= x2; x++)
		{
			ID_FLOOR(x, z1)->flag |= ID_FLOOR_FLAG_ON_WALL;
		}

		return;
	}

	if (x1 == x2)
	{
		//
		// Easy!
		//

		if (z1 > z2) {SWAP(z1, z2);}

		for (z = z1; z <= z2; z++)
		{
			ID_FLOOR(x1, z)->flag |= ID_FLOOR_FLAG_ON_WALL;
		}

		return;
	}

	//
	// Slightly tricky.
	//

	if (z1 > z2)
	{
		SWAP(x1, x2);
		SWAP(z1, z2);
	}

	dx = x2 - x1 << 16;
	dz = z2 - z1 << 16;

	dxdz = DIV64(dx, dz);

	x = x1 << 16;

	for (z = z1; z <= z2; z++)
	{
		//
		// is (x, z) on a dot? Or near enough on a dot?
		//

		if (((x + 0x7f >> 8) & 0xff) == 0)
		{
			//
			// (x, z) is within 1/256'th of a mapsquare dot.
			//

			mx   = x + 0x7f;
			mx >>= 16;
			mz   = z;

			ID_FLOOR(mx, mz)->flag |= ID_FLOOR_FLAG_ON_WALL;
		}

		x += dxdz;
	}
}

void ID_find_a_camera_for_each_room(void)
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;
	SLONG n;
	SLONG choice;
	SLONG off_x;
	SLONG off_z;

	SLONG threes;
	SLONG ones;

	#define MAX_POSS 64

	struct
	{
		UWORD n;
		UWORD x;
		UWORD z;

	} poss[MAX_POSS];

	SLONG poss_upto;
	
	//
	// Count how many 'three' corners and how many
	// 'one' corners you have. 'Three' corners are
	// better than one corners.
	//

	#define IN_ROOM(i, x, z) (WITHIN((x), ID_floor_x1, ID_floor_x2 - 1) && WITHIN((z), ID_floor_z1, ID_floor_z2 - 1) && ID_FLOOR((x), (z))->room == (i))

	for (i = 1; i < ID_room_upto; i++)
	{
		threes    = 0;
		ones      = 0;
		poss_upto = 0;

		for (x = ID_floor_x1; x <= ID_floor_x2; x++)
		for (z = ID_floor_z1; z <= ID_floor_z2; z++)
		{
			//
			// Count how many squares touching point (x,z)
			// are in the given room.
			//

			n = 0;

			off_x = 0;
			off_z = 0;

			if (IN_ROOM(i, x - 0, z - 0)) {n++; off_x += 1; off_z += 1;}
			if (IN_ROOM(i, x - 1, z - 0)) {n++; off_x -= 1; off_z += 1;}
			if (IN_ROOM(i, x - 0, z - 1)) {n++; off_x += 1; off_z -= 1;}
			if (IN_ROOM(i, x - 1, z - 1)) {n++; off_x -= 1; off_z -= 1;}

			off_x <<= ELE_SHIFT - 2;
			off_z <<= ELE_SHIFT - 2;

			if (n == 1)
			{
				ASSERT(WITHIN(poss_upto, 0, MAX_POSS - 1));

				//
				// This is a 'one' corner of the room.
				//

				poss[poss_upto].n = 1;
				poss[poss_upto].x = (x << ELE_SHIFT) + off_x;
				poss[poss_upto].z = (z << ELE_SHIFT) + off_z;

				ones      += 1;
				poss_upto += 1;
			}

			if (n == 3)
			{
				ASSERT(WITHIN(poss_upto, 1, MAX_POSS - 1));

				//
				// This is a 'three' corner of the room.
				//

				poss[poss_upto].n = 3;
				poss[poss_upto].x = (x << ELE_SHIFT) + off_x;
				poss[poss_upto].z = (z << ELE_SHIFT) + off_z;

				threes    += 1;
				poss_upto += 1;
			}
		}

		//
		// If there is a 'three' corner then choose one of them.
		//

		if (threes > 0)
		{
			choice = ID_rand() % threes;

			//
			// Choose the j'th three corner.
			//

			for (j = 0; j < poss_upto; j++)
			{
				if (poss[j].n == 3)
				{
					if (choice == 0)
					{
						ID_room[i].cam_x = poss[j].x;
						ID_room[i].cam_z = poss[j].z;

						break;
					}
					else
					{
						choice -= 1;
					}
				}
			}
		}
		else
		{
			//
			// Pick a random corner.
			//

			choice = ID_rand() % ones;

			ID_room[i].cam_x = poss[choice].x;
			ID_room[i].cam_z = poss[choice].z;
		}
	}
}

SLONG ID_intersects_badly(SLONG x1, SLONG z1, SLONG x2, SLONG z2)
{
	SLONG i;
	SLONG ix;
	SLONG iz;
	SLONG dx;
	SLONG dz;

	ID_Wall *iw;

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		if (iw->type != ID_WALL_T_OUTSIDE)
		{
			//
			// Only outside walls are non-orthogonal, so these
			// are the only ones we have to check.
			//

			return FALSE;
		}

		dx = iw->x2 - iw->x1;
		dz = iw->z2 - iw->z1;

		if (dx && dz)
		{
			//
			// The lines are allowed to share end-points, even though
			// the lines_intersect function counts that as an
			// intersection.
			//

			if ((x1 == iw->x1 && z1 == iw->z1) ||
				(x1 == iw->x2 && z1 == iw->z2) ||
				(x2 == iw->x1 && z2 == iw->z1) ||
				(x2 == iw->x2 && z2 == iw->z2))
			{
				//
				// Okay to share endpoints.
				//
			}
			else
			{
				if (lines_intersect(
						x1, z1, x2, z2,
						iw->x1,
						iw->z1,
						iw->x2,
						iw->z2,
					   &ix,
					   &iz) == DO_INTERSECT)
				{
					return TRUE;
				}
			}
		}
	}

	//
	// This line does not intersect badly with any outside wall.
	//

	return FALSE;
}



//
// Calculate which squares belong in which rooms.
//

void ID_find_rooms(void)
{
	SLONG i;
	SLONG x;
	SLONG z;
	SLONG cx;
	SLONG cz;
	SLONG nx;
	SLONG nz;
	SLONG dx;
	SLONG dz;

	ID_Square *is;

	//
	// Clear old rooms.
	//

	ID_room_upto = 1;

	//
	// Make sure we can use UBYTEs
	//

	ASSERT(ID_FLOOR_SIZE <= 256);

	//
	// A queue for filling up rooms.
	//

	#define QUEUE_SIZE 64

	typedef struct
	{
		UBYTE x;
		UBYTE z;

	} Queue;
	
	Queue queue[QUEUE_SIZE];
	SLONG queue_head;
	SLONG queue_tail;

	#define QUEUE_EMPTY()	(queue_head == queue_tail)
	#define QUEUE_NEXT(q)	(((q) + 1) & (QUEUE_SIZE - 1))
	#define QUEUE_FULL()	(QUEUE_NEXT(queue_head) == queue_tail)
	#define QUEUE_ADD(a,b)	{queue[queue_head].x = (a); queue[queue_head].z = (b); queue_head = QUEUE_NEXT(queue_head);}
	#define QUEUE_TAIL()	(queue[queue_tail])
	#define QUEUE_REMOVE()	{queue_tail = QUEUE_NEXT(queue_tail);}

	struct
	{
		SBYTE dx;
		SBYTE dz;
		UBYTE flag;
		UBYTE shit;

	} udlr[4] =
	{
		{+1, 0, ID_FLOOR_FLAG_WALL_XL, 0},
		{-1, 0, ID_FLOOR_FLAG_WALL_XS, 0},
		{0, +1, ID_FLOOR_FLAG_WALL_ZL, 0},
		{0, -1, ID_FLOOR_FLAG_WALL_ZS, 0}
	};

	for (x = ID_floor_x1; x < ID_floor_x2; x++)
	for (z = ID_floor_z1; z < ID_floor_z2; z++)
	{
		is = ID_FLOOR(x,z);

		if (!(is->flag & ID_FLOOR_FLAG_INSIDE))
		{
			continue;
		}

		if (is->room == 0)
		{
			//
			// Make sure we haven't run out of rooms.
			//

			ASSERT(WITHIN(ID_room_upto, 1, ID_MAX_ROOMS - 1));

			//
			// This block has been unassigned.
			//

			queue_head = 0;
			queue_tail = 0;

			QUEUE_ADD(x, z);

			while(!QUEUE_EMPTY())
			{
				cx = QUEUE_TAIL().x;
				cz = QUEUE_TAIL().z;

				QUEUE_REMOVE();

				if (ID_FLOOR(cx, cz)->room == 0)
				{
					ID_FLOOR(cx, cz)->room = ID_room_upto;

					for (i = 0; i < 4; i++)
					{
						if (ID_FLOOR(cx,cz)->flag & udlr[i].flag)
						{
							//
							// Can't fill in this direction... there
							// is a wall in the way.
							//
						}
						else
						{
							if (QUEUE_FULL())
							{
								TRACE("Queue full!\n");
							}
							else
							{
								dx = udlr[i].dx;
								dz = udlr[i].dz;

								nx = cx + dx;
								nz = cz + dz;

								if (!WITHIN(nx, ID_floor_x1, ID_floor_x2 - 1) ||
									!WITHIN(nz, ID_floor_z1, ID_floor_z2 - 1) ||
									!(ID_FLOOR(nx, nz)->flag & ID_FLOOR_FLAG_INSIDE))
								{
									//
									// Invalid square.
									//
								}
								else
								{
									QUEUE_ADD(nx,nz);
								}
							}
						}
					}
				}
			}

			//
			// Finished with this room index.
			//

			ID_room_upto += 1;
		}
	}
}


//
// Generates room info. Works out the size of each room, whether it is
// a corridor or not, a lobby, this sort of thing.
//

void ID_generate_room_info(void)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG width;
	SLONG height;
	SLONG room;

	ID_Room   *ir;
	ID_Square *is;
	ID_Wall   *iw;
	ID_Stair  *it;

	//
	// Initialise each room.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ID_room[i].type      = ID_ROOM_TYPE_NONE;
		ID_room[i].num_doors = 0;

		ID_room[i].x1 = 255;
		ID_room[i].z1 = 255;
		ID_room[i].x2 = 0;
		ID_room[i].z2 = 0;

		ID_room[i].flag = 0;
	}

	//
	// Work out the bounding box of each room.
	//

	for (x = ID_floor_x1; x < ID_floor_x2; x++)
	for (z = ID_floor_z1; z < ID_floor_z2; z++)
	{
		is = ID_FLOOR(x,z);

		if (is->room)
		{
			ASSERT(WITHIN(is->room, 1, ID_room_upto - 1));

			if (x < ID_room[is->room].x1) {ID_room[is->room].x1 = x;}
			if (z < ID_room[is->room].z1) {ID_room[is->room].z1 = z;}
			if (x > ID_room[is->room].x2) {ID_room[is->room].x2 = x;}
			if (z > ID_room[is->room].z2) {ID_room[is->room].z2 = z;}
		}
	}

	//
	// Work out which rooms are rectangular of not.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		for (x = ID_room[i].x1; x <= ID_room[i].x2; x++)
		for (z = ID_room[i].z1; z <= ID_room[i].z2; z++)
		{
			if (ID_FLOOR(x,z)->room != i)
			{
				goto next_room;
			}
		}

		ID_room[i].flag |= ID_ROOM_FLAG_RECTANGULAR;

	  next_room:;
	}

	//
	// Work out which rooms count as corridors.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		if (ir->flag & ID_ROOM_FLAG_RECTANGULAR)
		{
			width  = ir->x2 - ir->x1 + 1;
			height = ir->z2 - ir->z1 + 1;

			if (((width  == 1 || width  == 2) && height >= ID_CORRIDOR_LENGTH * width ) ||
				((height == 1 || height == 2) && width  >= ID_CORRIDOR_LENGTH * height))
			{
				ir->flag |= ID_ROOM_FLAG_CORRIDOR;
			}
		}
	}

	//
	// Work out which rooms are lobbies. Go through all the outside
	// walls looking for a door. When we find one, work out which
	// square it leads into.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		if (iw->type != ID_WALL_T_OUTSIDE)
		{
			break;
		}

		for (j = 0; j < iw->num_blocks; j++)
		{
			if (ID_get_type(iw->id, j) == ID_BLOCK_TYPE_DOOR)
			{
				//
				// This is a door in an outside wall. Find the point (x,z)
				// just inside the door.
				//

				dx = (iw->x2 - iw->x1 << 8) / iw->num_blocks;
				dz = (iw->z2 - iw->z1 << 8) / iw->num_blocks;

				x = iw->x1 << 8;
				z = iw->z1 << 8;

				x += dx * j;
				z += dz * j;

				x += dx >> 1;
				z += dz >> 1;

				x += (-dz) >> 4;
				z += ( dx) >> 4;

				x >>= 8;
				z >>= 8;

				is = ID_FLOOR(x,z);

				//
				// Mark this square as being a doormat.
				//

				is->flag |= ID_FLOOR_FLAG_DOORMAT;
				
				//
				// The room that this square is in is a lobby.
				//

				room = ID_FLOOR(x,z)->room;

				ASSERT(WITHIN(room, 1, ID_room_upto - 1));

				ID_room[room].flag |= ID_ROOM_FLAG_LOBBY;
			}
		}
	}

	//
	// Works out which rooms contain stairs.
	//

	for (i = 0; i < ID_num_stairs; i++)
	{
		it = &ID_stair[i];

		room = ID_FLOOR(it->x1, it->z1)->room;

		ASSERT(WITHIN(room, 1, ID_room_upto - 1));

		ID_room[room].flag |= ID_ROOM_FLAG_STAIRWAY;
	}
}

//
// Splits up the rooms into flats. Each flat is a set of rooms 
// connected to each other without having to go into a ID_ROOM_TYPE_CORRIDOR.
//

void ID_find_flats(void)
{
	SLONG i;
	SLONG x;
	SLONG z;
	SLONG dx;
	SLONG dz;
	SLONG nx;
	SLONG nz;
	SLONG neighbour;
	SLONG changed;

	ID_Room *ir;

	//
	// Mark all rooms as not being in a flat.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		ir->flat = 0;
	}

	//
	// Assign the flats.
	//

	ID_flat_upto = 1;

	while(1)
	{
		//
		// Have all rooms apart from corridors been allocated a flat number?
		//

		for (i = 1; i < ID_room_upto; i++)
		{
			ir = &ID_room[i];

			if (ir->type != ID_ROOM_TYPE_CORRIDOR && ir->flat == 0)
			{
				ir->flat = ID_flat_upto++;

				goto more_rooms_to_do;
			}
		}

		return;
	  
	  more_rooms_to_do:;

		while(1)
		{
			changed = FALSE;

			for (i = 1; i < ID_room_upto; i++)
			{
				ir = &ID_room[i];

				if (ir->flat)
				{
					//
					// Search for connected rooms that aren't corridors and
					// make them part of this flat.
					// 

					for (x = ir->x1; x <= ir->x2; x++)
					for (z = ir->z1; z <= ir->z2; z++)
					{
						if (ID_FLOOR(x,z)->room == i)
						{
							for (dx = -1; dx <= +1; dx += 2)
							for (dz = -1; dz <= +1; dz += 2)
							{
								nx = x + dx;
								nz = z + dz;

								if (WITHIN(nx, ID_floor_x1, ID_floor_x2) &&
									WITHIN(nz, ID_floor_z1, ID_floor_z2))
								{
									neighbour = ID_FLOOR(nx,nz)->room;

									if (neighbour)
									{
										ASSERT(WITHIN(neighbour, 1, ID_room_upto - 1));

										if (neighbour != i                                   &&
											ID_room[neighbour].type != ID_ROOM_TYPE_CORRIDOR &&
											ID_room[neighbour].flat != ir->flat)
										{
											//
											// Mark this room as also being in our flat.
											//

											changed = TRUE;

											ID_room[neighbour].flat = ir->flat;
										}
									}
								}
							}
						}
					}
				}
			}

			if (!changed)
			{
				break;
			}
		}
	}
}

//
// Assigns the type of each room in the given flat.
//

void ID_assign_flat_room_types(SLONG flat)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG area;
	SLONG changed;

	SLONG biggest;
	SLONG smallest;
	SLONG middle;

	SLONG num_rooms;
	SLONG have_loo;

	UBYTE type;

	UBYTE size [ID_MAX_ROOMS];
	UBYTE order[ID_MAX_ROOMS];

	ID_Room *ir;

	//
	// Find all the rooms in this flat.
	//

	num_rooms = 0;

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		if (ir->flat == flat)
		{
			ASSERT(WITHIN(num_rooms, 0, ID_MAX_ROOMS - 1));

			dx = ir->x2 - ir->x1;
			dz = ir->z2 - ir->z1;

			area = dx * dz;

			if (area > 255) {area = 255;}

			order[num_rooms] = i;
			size [num_rooms] = area;

			num_rooms += 1;
		}
	}

	//
	// Sort the rooms in order of their size... smallest first.
	//

	do
	{
		changed = FALSE;

		for (i = 0; i < num_rooms - 1; i++)
		{
			if (size[i] > size[i + 1])
			{
				SWAP(size [i], size [i + 1]);
				SWAP(order[i], order[i + 1]);

				changed = TRUE;
			}
		}
	}
	while(changed);

	#ifndef _NDEBUG

	//
	// Are all the ordered rooms ok?
	//

	for (i = 0; i < num_rooms; i++)
	{
		ASSERT(WITHIN(order[i], 1, ID_room_upto - 1));
		ASSERT(ID_room[order[i]].flat == flat);
	}

	#endif

	biggest  = order[num_rooms - 1];
	smallest = order[0];

	
	//
	// Make the biggest room a lounge.
	//

	switch(num_rooms)
	{
		case 1:
		case 2:
			ID_room[smallest].type = ID_ROOM_TYPE_LOO;
			ID_room[biggest ].type = ID_ROOM_TYPE_LOUNGE;
			break;

		case 3:
			ID_room[smallest].type = ID_ROOM_TYPE_LOO;
			ID_room[order[1]].type = ID_ROOM_TYPE_BEDROOM;
			ID_room[biggest ].type = ID_ROOM_TYPE_LOUNGE;
			break;

		default:
			
			ASSERT(num_rooms > 3);

			ID_room[smallest].type = ID_ROOM_TYPE_LOO;
			ID_room[order[1]].type = ID_ROOM_TYPE_KITCHEN;
			ID_room[order[2]].type = ID_ROOM_TYPE_BEDROOM;

			for (i = 3; i < num_rooms; i++)
			{
				ID_room[order[i]].type = ID_ROOM_TYPE_LOUNGE;
			}

			break;
	}	
}


//
// Decides which room is which. It assumes the rooms are
// in a house.
//

void ID_assign_room_types(SLONG storey_type)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG corridor;

	SLONG area;
	SLONG changed;

	SLONG biggest;
	SLONG smallest;
	SLONG middle;

	SLONG num_rooms;
	SLONG have_loo;

	UBYTE type;

	UBYTE size [ID_MAX_ROOMS];
	UBYTE order[ID_MAX_ROOMS];

	ID_Room *ir;

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		ir->type = ID_ROOM_TYPE_NONE;

		dx = ir->x2 - ir->x1;
		dz = ir->z2 - ir->z1;

		area = dx * dz;

		if (area > 255) {area = 255;}

		order[i] = i;
		size [i] = area;
	}

	//
	// Sort the rooms in order of their size... smallest first.
	//

	do
	{
		changed = FALSE;

		for (i = 1; i < ID_room_upto - 1; i++)
		{
			if (size[i] > size[i + 1])
			{
				SWAP(size [i], size [i + 1]);
				SWAP(order[i], order[i + 1]);

				changed = TRUE;
			}
		}
	}
	while(changed);

	//
	// Allocate the rooms.
	//

	biggest   = order[ID_room_upto - 1];
	smallest  = order[1];
	num_rooms = ID_room_upto - 1;

	switch(storey_type)
	{
		case ID_STOREY_TYPE_HOUSE_GROUND:
		case ID_STOREY_TYPE_HOUSE_UPPER:
		case ID_STOREY_TYPE_OFFICE_GROUND:
		case ID_STOREY_TYPE_OFFICE_UPPER:

			//
			// Make the biggest room a lounge.
			//

			ASSERT(WITHIN(biggest, 1, ID_room_upto - 1));

			ID_room[biggest].type = ID_ROOM_TYPE_LOUNGE;

			num_rooms -= 1;

			//
			// Look for any room other than the biggeset connected
			// to the outside.  Make that room a lobby.
			//

			for (i = 1; i <= num_rooms; i++)
			{
				ASSERT(WITHIN(order[i], 1, ID_room_upto - 1));

				if (ID_room[order[i]].flag & ID_ROOM_FLAG_LOBBY)
				{
					ID_room[order[i]].type = ID_ROOM_TYPE_LOBBY;

					SWAP(size [i], size [num_rooms]);
					SWAP(order[i], order[num_rooms]);

					num_rooms -= 1;

					break;
				}
			}

			//
			// Is the smallest room a good size for a loo?
			//

			#define ID_LOO_AREA 4

			have_loo = (size[1] <= ID_LOO_AREA);

			if (num_rooms > 2)
			{
				//
				// Lots of rooms, might as well have a loo.
				//

				have_loo = TRUE;
			}

			if (have_loo)
			{
				//
				// Fill in the rooms in the given order.
				//

				for (i = 1; i <= num_rooms; i++)
				{
					switch(i)
					{
						case 1:	 type = ID_ROOM_TYPE_LOO;		break;
						case 2:	 type = ID_ROOM_TYPE_KITCHEN;	break;
						case 3:  type = ID_ROOM_TYPE_DINING;	break;
						default: type = ID_ROOM_TYPE_LOUNGE;	break;
					}

					ASSERT(WITHIN(order[i], 1, ID_room_upto - 1));

					ID_room[order[i]].type = type;
				}
			}
			else
			{
				for (i = 1; i <= num_rooms; i++)
				{
					ASSERT(WITHIN(order[i], 1, ID_room_upto - 1));

					if (i == 1)
					{
						ID_room[order[i]].type = ID_ROOM_TYPE_KITCHEN;
					}
					else
					{
						ID_room[order[i]].type = ID_ROOM_TYPE_DINING;
					}
				}
			}
			break;

		case ID_STOREY_TYPE_WAREHOUSE:
			
			//
			// Make the biggest room a warehouse.
			//

			ASSERT(WITHIN(biggest, 1, ID_room_upto - 1));

			ID_room[biggest].type = ID_ROOM_TYPE_WAREHOUSE;

			num_rooms -= 1;

			//
			// A room that leads onto the outside is a lobby.
			//

			for (i = 1; i <= num_rooms; i++)
			{
				ASSERT(WITHIN(order[i], 1, ID_room_upto - 1));

				ir = &ID_room[order[i]];

				for (x = ir->x1; x <= ir->x2; x++)
				for (z = ir->z1; z <= ir->z2; z++)
				{
					if (ID_FLOOR(x,z)->room == order[i])
					{
						if (ID_FLOOR(x,z)->flag & ID_FLOOR_FLAG_DOORMAT)
						{
							//
							// Make this room a lobby.
							//

							ir->type = ID_ROOM_TYPE_LOBBY;

							//
							// Take this room out of the list.
							//

							for (j = i; j < num_rooms; j++)
							{
								order[j] = order[j + 1];
								size [j] = size [j + 1];
							}

							i         -= 1;
							num_rooms -= 1;

							goto lobbify_next_room;
						}
					}
				}

			  lobbify_next_room:;
			}

			//
			// The smallest room is a loo, the middle ones are offices and
			// the largest is a meeting room... but we only have a meeting
			// room if there are at least 3 rooms.
			//

			for (i = 1; i <= num_rooms; i++)
			{
				type = ID_ROOM_TYPE_OFFICE;

				if (i == 1)                   {type = ID_ROOM_TYPE_LOO;}
				if (i != 2 && i == num_rooms) {type = ID_ROOM_TYPE_MEETING;}

				ASSERT(WITHIN(order[i], 1, ID_room_upto - 1));

				ID_room[order[i]].type = type;
			}

			break;

		case ID_STOREY_TYPE_APARTEMENT_GROUND:
		case ID_STOREY_TYPE_APARTEMENT_UPPER:

			//
			// The room that contains the stairs is the corridor.
			//

			if (ID_num_stairs == 0)
			{
				//
				// Argh! I'm confused if I can't make a corridor.
				//

				for (i = 1; i < ID_room_upto; i++)
				{
					ID_room[i].type = ID_ROOM_TYPE_LOO;
				}
			}
			else
			{
				corridor = ID_FLOOR(ID_stair[0].x1, ID_stair[0].z1)->room;

				ASSERT(WITHIN(corridor, 1, ID_room_upto - 1));

				ID_room[corridor].type = ID_ROOM_TYPE_CORRIDOR;

				//
				// Partition the rooms into flats.
				//

				ID_find_flats();

				//
				// Assign the room types for each flat independently.
				// 

				for (i = 1; i < ID_flat_upto; i++)
				{
					ID_assign_flat_room_types(i);
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
	
	//
	// Colour in the floor to match the room types.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		for (x = ir->x1; x <= ir->x2; x++)
		for (z = ir->z1; z <= ir->z2; z++)
		{
			ID_FLOOR(x,z)->texid = ir->type;
		}
	}
}

//
// Puts in connecting doors between rooms.  Returns FALSE
// on failure.
//

SLONG ID_make_connecting_doors(SLONG storey_type)
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;
	SLONG imx;
	SLONG imz;
	SLONG omx;
	SLONG omz;
	SLONG dmx;
	SLONG dmz;
	SLONG wx1, wx2;
	SLONG wz1, wz2;
	SLONG x1, z1;
	SLONG x2, z2;
	SLONG dx;
	SLONG dz;
	SLONG score;
	SLONG segment;
	SLONG length;
	SLONG roomtype;
	SLONG best_score;
	SLONG best_choice;
	UBYTE type_in;
	UBYTE type_out;
	UBYTE room_out;

	SLONG bw;
	SLONG wall;
	SLONG bwmul;
	SLONG bwadd;
	SLONG seglength;

	#define MAX_DOORCHOICES 64

	struct
	{
		UBYTE ix, iz;	// The inside-the-room block...
		UBYTE ox, oz;	// The outside block it leads to...
		SLONG score;
		UBYTE seglength;
		UBYTE wall;
		UBYTE wall_block;
		UBYTE room;

	} doorchoice[MAX_DOORCHOICES];

	SLONG doorchoice_upto;

	//
	// Remember which rooms connect to other rooms?
	// One bit for each (room x room).
	//

	UBYTE connected[ID_MAX_ROOMS][ID_MAX_ROOMS];

	//
	// Accessing the connected array.
	//

	#define DO_CONNECT(r1,r2)     {connected[(r1)][(r2)] = TRUE;}
	#define IS_CONNECTED(r1,r2)   (connected[(r1)][(r2)])
	#define	MARK_CONNECTED(r1,r2) {DO_CONNECT(r1,r2); DO_CONNECT(r2,r1);}

	ID_Room *ir;
	ID_Wall *iw;

	//
	// Mark all rooms as unconnected.
	//

	memset((UBYTE*)connected, 0, sizeof(connected));

	//
	// Go through all the rooms and decide where are door(s) should be.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		//
		// Loos and kitchens should only have one door each.
		//

		if (ID_room[i].type == ID_ROOM_TYPE_KITCHEN ||
			ID_room[i].type == ID_ROOM_TYPE_LOO)
		{
			if (ID_room[i].num_doors != 0)
			{
				continue;
			}
		}

		//
		// Never make doors from corridors, only _to_ corridors.
		//

		if (ID_room[i].type == ID_ROOM_TYPE_CORRIDOR)
		{
			continue;
		}

		//
		// Calculate the perimeter of the room.
		//

		if (!ID_calc_room_perim(i))
		{
			return FALSE;
		}

		//
		// Go around the perimeter and find all the choices
		// of door positions.
		//

		doorchoice_upto = 0;

		for (j = 0; j < ID_perim_upto - 1; j++)
		{
			//
			// This line segment.
			//

			x1 = ID_perim[j + 0].x << 8;
			z1 = ID_perim[j + 0].z << 8;
			x2 = ID_perim[j + 1].x << 8;
			z2 = ID_perim[j + 1].z << 8;

			dx = x2 - x1;
			dz = z2 - z1;

			if (dx && dz)
			{
				//
				// This line segment must be part of an outside wall
				// so we don't put any doors there.
				//
			}
			else
			{
				length = MAX(abs(dx), abs(dz)) >> 8;

				dx /= length;
				dz /= length;

				//
				// A tiny vector pointing to the right of the wall.
				//

				dmx = SIGN(-dz);
				dmz = SIGN( dx);

				//
				// Go through the wall segment and decide where to put a door.
				//

				x = x1;
				z = z1;

				segment = 0;

				while(x != x2 || z != z2)
				{
					//
					// This bit of the wall segment
					//

					wx1 = x;
					wz1 = z;

					wx2 = x + dx;
					wz2 = z + dz;

					//
					// Find the blocks on either side of
					// this bit of wall segment.
					//

					imx = wx1 + wx2 >> 1;
					imz = wz1 + wz2 >> 1;

					omx = wx1 + wx2 >> 1;
					omz = wz1 + wz2 >> 1;

					imx += dmx;
					imz += dmz;

					omx -= dmx;
					omz -= dmz;

					imx >>= 8;
					imz >>= 8;

					omx >>= 8;
					omz >>= 8;

					//
					// (imx,imz) is the block in the room and (omx,omz) is
					// the block outside on the other side of the room.
					//

					if (ID_FLOOR(imx,imz)->room != i) {return FALSE;}

					ASSERT(WITHIN(doorchoice_upto, 0, MAX_DOORCHOICES - 1));

					if (WITHIN(omx, ID_floor_x1, ID_floor_x2) &&
						WITHIN(omz, ID_floor_z1, ID_floor_z2))
					{
						room_out = ID_FLOOR(omx,omz)->room;

						if (room_out && room_out != i)
						{
							//
							// Find which block of wall this block of line segment corresponds to.
							//

							ID_find_segment_wall(x1 >> 8, z1 >> 8, x2 >> 8, z2 >> 8,
								&wall,
								&bwmul,
								&bwadd,
								&seglength);

							bw = segment * bwmul + bwadd;

							doorchoice[doorchoice_upto].ix = imx;
							doorchoice[doorchoice_upto].iz = imz;
							doorchoice[doorchoice_upto].ox = omx;
							doorchoice[doorchoice_upto].oz = omz;

							doorchoice[doorchoice_upto].wall       = wall;
							doorchoice[doorchoice_upto].wall_block = bw;
							doorchoice[doorchoice_upto].seglength  = seglength;
							doorchoice[doorchoice_upto].room       = room_out;

							doorchoice_upto += 1;
						}
					}

					x += dx;
					z += dz;

					segment += 1;
				}
			}
		}

		//
		// Rate each choice of door position.
		//

		type_in = ID_room[i].type;

		for (j = 0; j < doorchoice_upto; j++)
		{
			room_out = doorchoice[j].room;

			ASSERT(WITHIN(room_out, 1, ID_room_upto - 1));

			type_out = ID_room[room_out].type;

			//
			// The default score.
			//

			score  = ID_rand() & 0xff;
			score += 2000;

			//
			// GLOBALLY APPLICABLE SCORING...
			//

			//
			// Don't have more than one door between the same
			// two rooms.
			//

			if (IS_CONNECTED(i, room_out))
			{
				score -= 2500;
			}

			//
			// Kitchens and loos should only have one entrance to them.
			//

			if (type_out == ID_ROOM_TYPE_KITCHEN ||
				type_out == ID_ROOM_TYPE_LOO)
			{
				if (ID_room[room_out].num_doors)
				{
					score -= 2500;
				}
			}

			//
			// Dont have doors between kitchens and loos... ugh!
			//

			if ((type_in == ID_ROOM_TYPE_LOO     && type_out == ID_ROOM_TYPE_KITCHEN) ||
				(type_in == ID_ROOM_TYPE_KITCHEN && type_out == ID_ROOM_TYPE_LOO))
			{
				score -= 2500;
			}

			//
			// We like doors that go to lobbies and lounges, but not loos.
			//

			if (type_out == ID_ROOM_TYPE_LOBBY)  {score += 1500;}
			if (type_out == ID_ROOM_TYPE_LOUNGE) {score += 1000;}
			if (type_out == ID_ROOM_TYPE_LOO)    {score -= 500;}

			//
			// Bedrooms should only connect to lounges and lobbies.
			//

			if (type_in == ID_ROOM_TYPE_BEDROOM)
			{
				if (type_out == ID_ROOM_TYPE_LOUNGE ||
					type_out == ID_ROOM_TYPE_LOBBY)
				{
					score += 2500;
				}
				else
				{
					score -= 1000;
				}
			}

			if (type_in == ID_ROOM_TYPE_LOO)
			{
				//
				// Loos should have their door in their shorter wall.
				//

				score -= doorchoice[j].seglength * 150;
			}

			if ((type_in == ID_ROOM_TYPE_KITCHEN && type_out == ID_ROOM_TYPE_DINING) ||
				(type_in == ID_ROOM_TYPE_DINING  && type_out == ID_ROOM_TYPE_KITCHEN))
			{
				//
				// Go from kitchens to dining rooms.
				//

				score += 500;
			}
			
			//
			// We LOVE! doors going to warehouses.
			//

			if (type_out == ID_ROOM_TYPE_WAREHOUSE)
			{
				score += 4500;
			}

			if (type_out == ID_ROOM_TYPE_CORRIDOR)
			{
				//
				// There are only corridors in appartement blocks. Only lounges are allowed
				// to lead onto corridors, and they really enjoy doing it!
				//

				if (type_in == ID_ROOM_TYPE_LOUNGE)
				{
					score += 10000;
				}
				else
				{
					score  = -10000;
				}
			}

			doorchoice[j].score = score;
		}

		//
		// Find the best choice.
		//

		best_choice = 0;
		best_score  = 0;

		for (j = 0; j < doorchoice_upto; j++)
		{
			if (doorchoice[j].score > best_score)
			{
				best_score  = doorchoice[j].score;
				best_choice = j;
			}
		}

		if (best_score > 0)
		{
			ASSERT(WITHIN(best_choice, 0, doorchoice_upto - 1));

			//
			// Put a door in the wall at the best choice.
			//

			wall = doorchoice[best_choice].wall;
			bw   = doorchoice[best_choice].wall_block;

			ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

			iw = &ID_wall[wall];

			for (j = 0; j < 3; j++)
			{
				if (iw->door[j] == bw)
				{
					//
					// There is already a door here!
					// 

					break;
				}

				if (iw->door[j] == 255)
				{
					iw->door[j] = bw;

					break;
				}
			}

			//
			// Sort the doors in order of smallest first.
			//

			if (iw->door[0] > iw->door[1]) {SWAP(iw->door[0], iw->door[1]);}
			if (iw->door[1] > iw->door[2]) {SWAP(iw->door[1], iw->door[2]);}
			if (iw->door[0] > iw->door[1]) {SWAP(iw->door[0], iw->door[1]);}

			//
			// Increase the number of doors in each room.
			//

			ASSERT(WITHIN(doorchoice[best_choice].room, 1, ID_room_upto - 1));

			ID_room[doorchoice[best_choice].room].num_doors += 1;
			ID_room[i                           ].num_doors += 1;

			//
			// Mark the floor-squares as connecting to a inside-door.
			//

			ID_FLOOR(doorchoice[best_choice].ix, doorchoice[best_choice].iz)->flag |= ID_FLOOR_FLAG_INMAT;
			ID_FLOOR(doorchoice[best_choice].ox, doorchoice[best_choice].oz)->flag |= ID_FLOOR_FLAG_INMAT;

			//
			// Mark the rooms as connected.
			//

			MARK_CONNECTED(i, doorchoice[best_choice].room);
		}
	}

	return TRUE;
}

//
// Adds some furniture to the map.
//
// (x,z) in 8-bit fixed point.
// dir is an angle from 0 to 2047.
//

THING_INDEX ID_add_furniture(MAPCO16 x, MAPCO16 y, MAPCO16 z, SLONG dir, SLONG prim)
{
	ASSERT(WITHIN(x, ID_floor_x1 << 8, ID_floor_x2 << 8));
	ASSERT(WITHIN(z, ID_floor_z1 << 8, ID_floor_z2 << 8));

	//
	// Make sure the direction is normalised.
	//

	dir >>= 3;
	dir  &= 0xff;

	if (ID_furn_upto < ID_MAX_FURN)
	{
		ID_furn[ID_furn_upto].x    = x;
		ID_furn[ID_furn_upto].z    = z;
		ID_furn[ID_furn_upto].prim = prim;
		ID_furn[ID_furn_upto].yaw  = dir;

		ID_furn_upto += 1;
	}

	return NULL;

	/*

	//
	// Create an ob.
	//

	OB_create(x, y, z, dir, 0, 0, prim);

	return NULL;

	*/

	/*

	Thing *p_thing;

	THING_INDEX t_index;

	//
	// A rotatable prim...
	//

	t_index = FURN_create(x<<8, y<<8, z<<8, dir, 0, 0, prim);

	if (t_index)
	{
		//
		// Mark the thing as indoors generated.
		//

		p_thing = TO_THING(t_index);

		p_thing->Flags |= FLAGS_IN_BUILDING;
		p_thing->Flags |= FLAGS_INDOORS_GENERATED;
	}

	return t_index;

	*/
}

//
// Puts in the furniture for the doors.
//

void ID_fit_doors(void)
{
	SLONG i;
	SLONG j;

	SLONG dwx;
	SLONG dwz;

	SLONG mid_x;
	SLONG mid_z;

	SLONG x1, z1;
	SLONG x2, z2;

	SLONG rx;
	SLONG rz;

	SLONG room_in;
	SLONG room_out;

	SLONG door_x;
	SLONG door_y;
	SLONG door_z;

	THING_INDEX door_thing;

	#define ID_DOOR_NONE	0
	#define ID_DOOR_INOUT	1
	#define ID_DOOR_OUTIN	2

	SLONG door;
	SLONG ajar;	// 0 => Shut, 512 => open
	SLONG angle;
	
	ID_Wall *iw;

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		if (iw->type == ID_WALL_T_OUTSIDE)
		{
			continue;
		}

		dwx = SIGN(iw->x2 - iw->x1);
		dwz = SIGN(iw->z2 - iw->z1);

		//
		// Are there any doors along this wall?
		//

		for (j = 0; j < 3; j++)
		{
			if (iw->door[j] == 255)
			{
				continue;
			}

			x1 = iw->x1 + dwx * iw->door[j] << 8;
			z1 = iw->z1 + dwz * iw->door[j] << 8;

			x2 = iw->x1 + dwx << 8;
			z2 = iw->z1 + dwz << 8;

			//
			// What are the rooms either side of the door?
			//

			mid_x = x1 + x2 >> 1;
			mid_z = z1 + z2 >> 1;

			rx = mid_x + dwz >> 8;
			rz = mid_z - dwx >> 8;

			room_in = ID_FLOOR(rx,rz)->room;

			rx = mid_x - dwz >> 8;
			rz = mid_z + dwx >> 8;

			room_out = ID_FLOOR(rx,rz)->room;

			//
			// We should be going from one room to another.
			//

			ASSERT(room_in && room_out && room_in != room_out);
			ASSERT(WITHIN(room_in,  1, ID_room_upto - 1));
			ASSERT(WITHIN(room_out, 1, ID_room_upto - 1));

			//
			// If either of them is a corridor we should have
			// a closed door.  Bedrooms and toilets have doors that
			// may be open.  All other doorways are door-less.
			//

			if (ID_room[room_in ].type == ID_ROOM_TYPE_CORRIDOR ||
				ID_room[room_out].type == ID_ROOM_TYPE_CORRIDOR)
			{
				if (ID_room[room_in].type == ID_ROOM_TYPE_CORRIDOR)
				{
					door = ID_DOOR_OUTIN;
				}
				else
				{
					door = ID_DOOR_INOUT;
				}

				ajar = 0; // Shut.
			}
			else
			if (ID_room[room_in ].type == ID_ROOM_TYPE_BEDROOM || 
				ID_room[room_out].type == ID_ROOM_TYPE_BEDROOM ||
				ID_room[room_in ].type == ID_ROOM_TYPE_LOO     ||
				ID_room[room_out].type == ID_ROOM_TYPE_LOO)
			{
				if (ID_room[room_in].type == ID_ROOM_TYPE_BEDROOM ||
					ID_room[room_in].type == ID_ROOM_TYPE_LOO)
				{
					door = ID_DOOR_INOUT;
				}
				else
				{
					door = ID_DOOR_OUTIN;
				}

				//
				// Maybe open... maybe shut.
				//

				ajar = ID_rand() & 0x180;
			}
			else
			{
				door = ID_DOOR_NONE;
			}

			if (dwx ==  0 && dwz ==  1) {angle += 1536;}
			if (dwx ==  1 && dwz ==  0) {angle += 0;   }
			if (dwx ==  0 && dwz == -1) {angle += 512; }
			if (dwx == -1 && dwz ==  0) {angle += 1024;}

			switch(door)
			{
				case ID_DOOR_NONE:
					break;

				case ID_DOOR_INOUT:

					door_x = x1 + dwx * (ID_FRAME_END - 1);
					door_z = z1 + dwz * (ID_FRAME_END - 1);

					door_y = INDOORS_HEIGHT_FLOOR;

					door_thing = ID_add_furniture(
									door_x,
									door_y,
									door_z,
									(angle + 1024) & 2047,
									PRIM_OBJ_DOOR);

					if (door_thing)
					{
						FURN_turn_into_door(
							door_thing,
							angle,
							ajar,
							FALSE);
					}

					break;

				case ID_DOOR_OUTIN:

					door_x = x1 + dwx * (ID_FRAME_START + 1);
					door_z = z1 + dwz * (ID_FRAME_START + 1);

					door_y = INDOORS_HEIGHT_FLOOR;

					door_thing = ID_add_furniture(
									door_x,
									door_y,
									door_z,
									angle,
									PRIM_OBJ_DOOR);

					if (door_thing)
					{
						FURN_turn_into_door(
							door_thing,
							angle,
							ajar,
							FALSE);
					}

					break;
				
				default:
					ASSERT(0);
					break;
			}
		}
	}
}


//
// Puts in the furniture for the stairs.
//

void ID_fit_stairs(void)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG angle;

	SLONG stairx;
	SLONG stairy;
	SLONG stairz;
	SLONG stairdir;
	SLONG stairprim;

	ID_Stair *it;

	for (i = 0; i < ID_num_stairs; i++)
	{
		it = &ID_stair[i];

		//
		// Find the angle of the stairs (in 90 degrees)
		//

		dx = it->x2 - it->x1;
		dz = it->z2 - it->z1;

			 if (dx ==  0 && dz == +1) {angle = 2;}
		else if (dx == +1 && dz ==  0) {angle = 3;}
		else if (dx ==  0 && dz == -1) {angle = 0;}
		else if (dx == -1 && dz ==  0) {angle = 1;}
		else ASSERT(0);

		stairdir = angle * 512;

		//
		// The position of the stairs.
		//

		stairx  = it->x1 << 8;
		stairz  = it->z1 << 8;

		stairx += 0x80;
		stairz += 0x80;

		stairx += dx << 7;
		stairz += dz << 7;

		stairy = INDOORS_HEIGHT_FLOOR;

		if (it->type != ID_STAIR_TYPE_BOTTOM)
		{
			//
			// There is the bit going down.
			//

			stairy -= 0x40;
		}

		//
		// Find the object to use.
		//

		switch(it->type)
		{
			case ID_STAIR_TYPE_BOTTOM: stairprim = PRIM_OBJ_STAIR_BOTTOM; break;
			case ID_STAIR_TYPE_MIDDLE: stairprim = PRIM_OBJ_STAIR_MIDDLE; break;
			case ID_STAIR_TYPE_TOP:    stairprim = PRIM_OBJ_STAIR_TOP;    break;
			default:
				ASSERT(0);
				break;
		}

		//
		// Add it to the map.
		//

		ID_add_furniture(stairx, stairy, stairz, stairdir, stairprim);
	}
}




//
// Fits a kitchen out. Returns FALSE on failure.
//

SLONG ID_fit_kitchen(SLONG room)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG dmx;
	SLONG dmz;

	SLONG length;

	SLONG best_x1;
	SLONG best_z1;
	SLONG best_x2;
	SLONG best_z2;

	SLONG best_length;

	//
	// Find the perimeter of this room.
	//

	if (!ID_calc_room_perim(room))
	{
		return FALSE;
	}

	//
	// Look for the longest unbroken orthogonal wall.
	//

	best_length = -INFINITY;

	for (i = 0; i < ID_perim_upto - 1; i++)
	{
		x1 = ID_perim[i + 0].x;
		z1 = ID_perim[i + 0].z;
		x2 = ID_perim[i + 1].x;
		z2 = ID_perim[i + 1].z;

		dx = x2 - x1;
		dz = z2 - z1;

		if (dx && dz)
		{
			//
			// Ignore non-orthogonal walls.
			//
		}
		else
		{
			length = MAX(abs(dx), abs(dz));

			if (length > best_length)
			{
				//
				// Make sure that this is an unbroken wall.
				//

				dx = SIGN(dx);
				dz = SIGN(dz);

				dmx = -dz;
				dmz =  dx;

				x = x1;
				z = z1;

				while(x != x2 || z != z2)
				{
					mx = x + (x + dx) << 7;
					mz = z + (z + dz) << 7;

					mx += dmx;
					mz += dmz;

					mx >>= 8;
					mz >>= 8;

					ASSERT(ID_FLOOR(mx,mz)->room == room);

					//
					// If this square is a doorway, then it
					// doesn't count.
					//

					if (ID_FLOOR(mx,mz)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT))
					{
						goto broken_wall;
					}

					x += dx;
					z += dz;
				}

				best_length = length;

				best_x1 = x1;
				best_z1 = z1;
				best_x2 = x2;
				best_z2 = z2;
			  
			  broken_wall:;
			}
		}
	}

	//
	// Depending on the length, put in a different
	// fitted kitchen bit from (bx1,bz1) to (bx2, bz2).
	//

	//
	// If the kitchen is big enough, put a table in the
	// middle.
	//

	return TRUE;
}

//
// Returns FALSE on failure.
//

SLONG ID_fit_loo(SLONG room)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG dmx;
	SLONG dmz;

	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	SLONG score;

	SLONG bestx;
	SLONG bestz;
	SLONG bestdir;
	SLONG bestscore;
	SLONG bestmx1;
	SLONG bestmz1;
	SLONG bestmx2;
	SLONG bestmz2;

	SLONG dist;

	SLONG doorx;
	SLONG doorz;

	SLONG best_dist;
	SLONG best_loox;
	SLONG best_looz;
	SLONG best_loocorner;

	SLONG sink_map_x;
	SLONG sink_map_z;
	SLONG sinkx;
	SLONG sinky;
	SLONG sinkz;
	SLONG sink_dir;

	SLONG loox;
	SLONG looy;
	SLONG looz;
	SLONG loo_dir;

	SLONG showerx;
	SLONG showery;
	SLONG showerz;

	SLONG bathx;
	SLONG bathy;
	SLONG bathz;
	SLONG bathdir;

	SLONG floor_area;

	ID_Room *ir;

	ASSERT(WITHIN(room, 1, ID_room_upto - 1));
	ASSERT(ID_room[room].type == ID_ROOM_TYPE_LOO);

	ir = &ID_room[room];

	//
	// Where is the entrance to the loo?
	//

	for (x = ir->x1; x <= ir->x2; x++)
	for (z = ir->z1; z <= ir->z2; z++)
	{
		if (ID_FLOOR(x,z)->room == room)
		{
			if (ID_FLOOR(x,z)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT))
			{
				doorx = x;
				doorz = z;

				goto found_door;
			}
		}

	}

	TRACE("Couldn't find entrace to the loo\n");

	return FALSE;

  found_door:;

	//
	// Look for a corner furthest from the door to
	// put the loo in.
	//

	best_dist = -INFINITY;
	best_loox =  0;
	best_looz =  0;

	for (i = 0; i < 4; i++)
	{
		x = (i & 1) ? ir->x1 : ir->x2;
		z = (i & 2) ? ir->z1 : ir->z2;

		dx = x - doorx;
		dz = z - doorz;

		dist = dx*dx + dz*dz;

		if (dist > best_dist)
		{
			best_dist = dist;
			best_loox = x;
			best_looz = z;
		}
	}

	//
	// Put in the loo in square (best_loox, best_looz)
	//

	loox = (best_loox << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
	looz = (best_looz << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));

	looy = INDOORS_HEIGHT_FLOOR;

	//
	// Make the loo face into the room and not into a wall.
	//

	for (i = 0; i < 4; i++)
	{
		loo_dir = i * 512;

		mx = (loox - (SIN(loo_dir) >> 8)) >> 8;
		mz = (looz - (COS(loo_dir) >> 8)) >> 8;

		if (WITHIN(mx, ID_floor_x1, ID_floor_x2 - 1) &&
			WITHIN(mz, ID_floor_z1, ID_floor_z2 - 1))
		{
			if (ID_FLOOR(mx,mz)->room == room)
			{
				//
				// Use this direction for the loo and
				// (mx,mz) for the sink.
				//

				sink_map_x = mx;
				sink_map_z = mz;

				goto found_loo_dir;
			}
		}
	}

	//
	// Couldn't find a direction for the loo
	// to face!
	//

	loo_dir = 0;

	TRACE("Couldn't find a decent direction for the loo!\n");

  found_loo_dir:

	//
	// Mark the loo square as containing a piece of furniture.
	//

	ID_FLOOR(best_loox, best_looz)->flag |= ID_FLOOR_FLAG_FURNISHED;

	//
	// Create the loo.
	//

	ID_add_furniture(loox, looy, looz, loo_dir, PRIM_OBJ_LOO);

	//
	// Put the sink against the wall in the square in
	// front of the loo.
	//

	sinkx = (sink_map_x << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
	sinkz = (sink_map_z << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));

	for (i = 0; i < 4; i++)
	{
		sink_dir = i * 512;

		mx = (sinkx + (SIN(sink_dir) >> 8)) >> 8;
		mz = (sinkz + (COS(sink_dir) >> 8)) >> 8;

		if (!WITHIN(mx, ID_floor_x1, ID_floor_x2 - 1) ||
			!WITHIN(mz, ID_floor_z1, ID_floor_z2 - 1) ||
			 (ID_FLOOR(mx, mz)->room != room && !(ID_FLOOR(mx,mz)->flag & ID_FLOOR_FLAG_INMAT)))
		{
			//
			// We have found the direction of the sink that
			// would put it up against the wall.
			//

			goto found_sink_dir;
		}
	}

	//
	// Could not find a direction for the sink...
	// Choose a random one!
	//

	sink_dir = 512 * (ID_rand() & 0x3);

  found_sink_dir:

	//
	// Mark the sink square as containing a piece of furniture.
	//

	ID_FLOOR(sink_map_x, sink_map_z)->flag |= ID_FLOOR_FLAG_FURNISHED;
	
	//
	// Where do we put the sink?
	//

	sinkx += SIN(sink_dir) >> 10;
	sinkz += COS(sink_dir) >> 10;
	
	sinky = INDOORS_HEIGHT_FLOOR;

	//
	// Create the sink.
	//

	ID_add_furniture(sinkx, sinky, sinkz, sink_dir, PRIM_OBJ_SINK);

	//
	// If this is a big toilet, then try to put in a shower
	// and a bath.
	//

	floor_area = (ir->x2 - ir->x1 + 1) * (ir->z2 - ir->z1 + 1);

	if (floor_area < 4)
	{
		return TRUE;
	}

	for (i = 0; i < 4; i++)
	{
		x = (i & 1) ? ir->x1 : ir->x2;
		z = (i & 2) ? ir->z1 : ir->z2;

		if (ID_FLOOR(x,z)->room == room)
		{
			//
			// Look for a spare corner of the room.
			//

			if (!(ID_FLOOR(x,z)->flag &
					(ID_FLOOR_FLAG_INMAT     |
					 ID_FLOOR_FLAG_DOORMAT   |
					 ID_FLOOR_FLAG_FURNISHED |
					 ID_FLOOR_FLAG_STAIR)))
			{
				//
				// Where do we put the shower?
				//

				showerx = (x << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
				showerz = (z << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
				showery = INDOORS_HEIGHT_FLOOR;

				//
				// Put a shower here.
				//

				ID_add_furniture(showerx, showery, showerz, 0, PRIM_OBJ_SHOWER);

				//
				// Mark this square as furnished.
				//

				ID_FLOOR(x,z)->flag |= ID_FLOOR_FLAG_FURNISHED;

				break;
			}
		}
	}

	if (floor_area < 6)
	{
		return TRUE;
	}

	//
	// Calculate the perimeter of the room to find where
	// we can put the bath.
	//

	if (!ID_calc_room_perim(room))
	{
		return FALSE;
	}

	//
	// Go through the perimeter looking for somewhere
	// to put the bath.
	//

	bestscore = -INFINITY;

	for (i = 0; i < ID_perim_upto - 1; i++)
	{
		x1 = ID_perim[i + 0].x;
		z1 = ID_perim[i + 0].z;

		x2 = ID_perim[i + 1].x;
		z2 = ID_perim[i + 1].z;

		dx = x2 - x1;
		dz = z2 - z1;

		if (dx && dz)
		{
			//
			// Ignore this wall.
			//

			continue;
		}

		//
		// The wall has to be long enough.
		//

		if (abs(dx) >= 2 || abs(dz) >= 2)
		{
			//
			// A tiny vector pointing to the right of the wall.
			//

			dmx = SIGN(-dz);
			dmz = SIGN( dx);

			x = x1;
			z = z1;

			dx = SIGN(dx);
			dz = SIGN(dz);

			while(x + dx != x2 || z + dz != z2)
			{
				//
				// Check that there is room for the bath.
				//

				mx1 = (x << ELE_SHIFT) + (dx << (ELE_SHIFT - 1)) + dmx >> ELE_SHIFT;
				mz1 = (z << ELE_SHIFT) + (dz << (ELE_SHIFT - 1)) + dmz >> ELE_SHIFT;

				mx2 = (x + dx << ELE_SHIFT) + (dx << (ELE_SHIFT - 1)) + dmx >> ELE_SHIFT;
				mz2 = (z + dz << ELE_SHIFT) + (dz << (ELE_SHIFT - 1)) + dmz >> ELE_SHIFT;

				//
				// (mx1,mz1) and (mx2,mz2) are the squares that the bath would cover.
				//

				ASSERT(
					ID_FLOOR(mx1,mz1)->room == room &&
					ID_FLOOR(mx2,mz2)->room == room);
				
				if (!(ID_FLOOR(mx1,mz1)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)) &&
					!(ID_FLOOR(mx2,mz2)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)))
				{
					//
					// We could put the bath here. Is it a good place?
					//

					score = 256;

					//
					// Can't think of any heuristics for good placement!
					//

					score += ID_rand() & 0x7;

					if (score > bestscore)
					{
						//
						// Remember this place.
						//

						bestx     = x + dx << ELE_SHIFT;
						bestz     = z + dz << ELE_SHIFT;
						bestdir   = -Arctan(dx,dz);
						bestscore = score;
						bestmx1   = mx1;
						bestmz1   = mz1;
						bestmx2   = mx2;
						bestmz2   = mz2;
					}
				}

				x += dx;
				z += dz;
			}
		}
	}

	if (bestscore >= 0)
	{
		//
		// Where should we place down the sofa.
		//

		bathx   = bestx;
		bathz   = bestz;
		bathy   = INDOORS_HEIGHT_FLOOR;

		bathdir = bestdir;

		bathx  -= SIN((bathdir - 512) & 2047) >> 9;
		bathz  -= COS((bathdir - 512) & 2047) >> 9;

		//
		// Put down the bath.
		//

		ID_add_furniture(bathx, bathy, bathz, bathdir, PRIM_OBJ_BATH);

		//
		// Mark the bath squares as furnished.
		//

		ID_FLOOR(bestmx1, bestmz1)->flag |= ID_FLOOR_FLAG_FURNISHED;
		ID_FLOOR(bestmx2, bestmz2)->flag |= ID_FLOOR_FLAG_FURNISHED;
	}

	return TRUE;
}

//
// Returns FALSE on failure.
//

SLONG ID_fit_lounge(SLONG room)
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG dir;

	SLONG dx;
	SLONG dz;

	SLONG dmx;
	SLONG dmz;

	SLONG mx;
	SLONG mz;

	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	SLONG score;

	SLONG bestx;
	SLONG bestz;
	SLONG bestdir;
	SLONG bestscore;
	SLONG bestmx1;
	SLONG bestmz1;
	SLONG bestmx2;
	SLONG bestmz2;
	SLONG bestdmx;
	SLONG bestdmz;

	SLONG coffeemx1;
	SLONG coffeemz1;
	SLONG coffeemx2;
	SLONG coffeemz2;
	SLONG coffeex;
	SLONG coffeey;
	SLONG coffeez;
  	SLONG coffeedir;

	SLONG tvx;
	SLONG tvy;
	SLONG tvz;
	SLONG tvdir;

	SLONG sofax;
	SLONG sofay;
	SLONG sofaz;
	SLONG sofadir;

	SLONG bookx;
	SLONG booky;
	SLONG bookz;
	SLONG bookdir;

	SLONG treex;
	SLONG treey;
	SLONG treez;
	SLONG treedir;

	ID_Room *ir;

	ASSERT(WITHIN(room, 1, ID_room_upto - 1));
	ASSERT(ID_room[room].type == ID_ROOM_TYPE_LOUNGE);

	ir = &ID_room[room];

	//
	// Put a TV in one of the corners.
	//

	bestscore = -INFINITY;

	for (i = 0; i < 4; i++)
	{
		x = (i & 1) ? ir->x1 : ir->x2;
		z = (i & 2) ? ir->z1 : ir->z2;
		
		switch(i)
		{
			case 0: dir =  256; break;
			case 1: dir = 1792; break;
			case 2: dir =  768; break;
			case 3: dir = 1280; break;
		}

		if (ID_FLOOR(x,z)->room == room)
		{
			if (!(ID_FLOOR(x,z)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)))
			{
				score = ID_rand();

				if (score > bestscore)
				{
					bestscore = score;
					bestdir   = dir;
					bestx     = x;
					bestz     = z;
				}
			}
		}
	}

	if (bestscore >= 0)
	{
		//
		// Put a TV at (bestx, bestz).
		//

		tvx = (bestx << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
		tvz = (bestz << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
		tvy = INDOORS_HEIGHT_FLOOR;

		ID_add_furniture(tvx, tvy, tvz, bestdir, PRIM_OBJ_TV);

		//
		// Mark this square as furnished.
		//

		ID_FLOOR(bestx, bestz)->flag |= ID_FLOOR_FLAG_FURNISHED;
	}

	//
	// Calculate the perimeter of the room to find
	// walls where we can put a sofa.
	//

	if (!ID_calc_room_perim(room))
	{
		return FALSE;
	}

	//
	// Go through the perimeter looking for somewhere
	// to put a sofa.
	//

	bestscore = -INFINITY;

	for (i = 0; i < ID_perim_upto - 1; i++)
	{
		x1 = ID_perim[i + 0].x;
		z1 = ID_perim[i + 0].z;

		x2 = ID_perim[i + 1].x;
		z2 = ID_perim[i + 1].z;

		dx = x2 - x1;
		dz = z2 - z1;

		if (dx && dz)
		{
			//
			// Ignore this wall.
			//

			continue;
		}

		//
		// The wall has to be long enough.
		//

		if (abs(dx) >= 2 || abs(dz) >= 2)
		{
			//
			// A tiny vector pointing to the right of the wall.
			//

			dmx = SIGN(-dz);
			dmz = SIGN( dx);

			x = x1;
			z = z1;

			dx = SIGN(dx);
			dz = SIGN(dz);

			while(x + dx != x2 || z + dz != z2)
			{
				//
				// Check that there is room for the sofa.
				//

				mx1 = (x << ELE_SHIFT) + (dx << (ELE_SHIFT - 1)) + dmx >> ELE_SHIFT;
				mz1 = (z << ELE_SHIFT) + (dz << (ELE_SHIFT - 1)) + dmz >> ELE_SHIFT;

				mx2 = (x + dx << ELE_SHIFT) + (dx << (ELE_SHIFT - 1)) + dmx >> ELE_SHIFT;
				mz2 = (z + dz << ELE_SHIFT) + (dz << (ELE_SHIFT - 1)) + dmz >> ELE_SHIFT;

				//
				// (mx1,mz1) and (mx2,mz2) are the squares that the sofa would cover.
				//

				ASSERT(
					ID_FLOOR(mx1,mz1)->room == room &&
					ID_FLOOR(mx2,mz2)->room == room);
				
				if (!(ID_FLOOR(mx1,mz1)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)) &&
					!(ID_FLOOR(mx2,mz2)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)))
				{
					//
					// We could put the sofa here. Is it a good place to put the sofa?
					//

					score = 256;

					//
					// Make it face the TV
					//

					if (mx1 == (tvx >> 8)) {score -= 16;}
					if (mx2 == (tvx >> 8)) {score -= 16;}
					if (mz1 == (tvz >> 8)) {score -= 16;}
					if (mz2 == (tvz >> 8)) {score -= 16;}

					//
					// Avoid corners.
					//

					if (mx1 == ir->x1 || mz1 == ir->z1 ||
						mx1 == ir->x2 || mz1 == ir->z2 ||
						mx2 == ir->x1 || mz2 == ir->z1 ||
						mx2 == ir->x2 || mz2 == ir->z2)
					{
						score -= 8;
					}

					score += ID_rand() & 0x7;

					if (score > bestscore)
					{
						//
						// Remember this place.
						//

						bestx     = x + dx << ELE_SHIFT;
						bestz     = z + dz << ELE_SHIFT;
						bestdir   = -Arctan(dx,dz);
						bestscore = score;
						bestmx1   = mx1;
						bestmz1   = mz1;
						bestmx2   = mx2;
						bestmz2   = mz2;
						bestdmx   = dmx;
						bestdmz   = dmz;
					}
				}

				x += dx;
				z += dz;
			}
		}
	}

	if (bestscore >= 0)
	{
		//
		// Where should we place down the sofa.
		//

		sofax   = bestx;
		sofaz   = bestz;
		sofay   = INDOORS_HEIGHT_FLOOR;

		sofadir  = bestdir - 0x1f + (ID_rand() & 0x3f);
		sofadir &= 2047;

		sofax  -= SIN((sofadir - 512) & 2047) >> 9;
		sofaz  -= COS((sofadir - 512) & 2047) >> 9;

		//
		// Put down the sofa.
		//

		ID_add_furniture(sofax, sofay, sofaz, sofadir, PRIM_OBJ_SOFA);

		//
		// Mark the sofa squares as furnished.
		//

		ID_FLOOR(bestmx1, bestmz1)->flag |= ID_FLOOR_FLAG_FURNISHED;
		ID_FLOOR(bestmx2, bestmz2)->flag |= ID_FLOOR_FLAG_FURNISHED;

		//
		// Remember where we could put a coffee table.
		//

		coffeex = sofax	- (SIN((sofadir - 512) & 2047) >> 8);
		coffeez = sofaz	- (COS((sofadir - 512) & 2047) >> 8);
		coffeey = INDOORS_HEIGHT_FLOOR;

		coffeedir = sofadir;

		coffeemx1 = bestmx1 + bestdmx;
		coffeemz1 = bestmz1 + bestdmz;
		coffeemx2 = bestmx2 + bestdmx;
		coffeemz2 = bestmz2 + bestdmz;
	}

	//
	// Go through the perimeter looking for somewhere to put the bookcase.
	//

	bestscore = -INFINITY;

	for (i = 0; i < ID_perim_upto - 1; i++)
	{
		x1 = ID_perim[i + 0].x;
		z1 = ID_perim[i + 0].z;

		x2 = ID_perim[i + 1].x;
		z2 = ID_perim[i + 1].z;

		dx = x2 - x1;
		dz = z2 - z1;

		if (dx && dz)
		{
			//
			// Ignore this wall.
			//

			continue;
		}

		//
		// A tiny vector pointing to the right of the wall.
		//

		dmx = SIGN(-dz);
		dmz = SIGN( dx);

		x = x1;
		z = z1;

		dx = SIGN(dx);
		dz = SIGN(dz);

		while(x != x2 || z != z2)
		{
			mx = (x << ELE_SHIFT) + (dx << (ELE_SHIFT - 1)) + dmx >> ELE_SHIFT;
			mz = (z << ELE_SHIFT) + (dz << (ELE_SHIFT - 1)) + dmz >> ELE_SHIFT;

			ASSERT(ID_FLOOR(mx,mz)->room == room);

			//
			// (mx,mz) is the square we would put the bookcase in.
			//

			if (ID_FLOOR(mx,mz)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR))
			{
				//
				// This square is used already.
				//
			}
			else
			{
				//
				// How good is this square?
				//

				score = 256;

				//
				// Be on the same wall as the tv.
				//

				if (mx == (tvx >> 8) ||
					mz == (tvz >> 8))
				{
					score += 16;
				}

				//
				// Don't go in the corners.
				//

				if (mx == ir->x1) {score -= 8;}
				if (mz == ir->z1) {score -= 8;}
				if (mx == ir->x2) {score -= 8;}
				if (mz == ir->z2) {score -= 8;}

				score += ID_rand() & 0x7;
				
				if (score > bestscore)
				{
					bestx     = mx;
					bestz     = mz;
					bestdir   = -Arctan(dx, dz);
					bestscore = score;
				}
			}

			x += dx;
			z += dz;
		}
	}

	if (bestscore > 0)
	{
		//
		// Where do we put the bookcase?
		//

		bookx = (bestx << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
		bookz = (bestz << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
		booky = INDOORS_HEIGHT_FLOOR;

		bookdir  = bestdir - 0xf + (ID_rand() & 0x1f);
		bookdir &= 2047;

		dx = SIN((bookdir - 512) & 2047);
		dz = COS((bookdir - 512) & 2047);

		bookx += dx >> 9;
		bookz += dz >> 9;
		bookx -= dx >> 11;
		bookz -= dz >> 11;

		//
		// Place the bookcase.
		//

		ID_add_furniture(bookx, booky, bookz, bookdir, PRIM_OBJ_BOOKCASE);

		//
		// Mark this square as furnished.
		//

		ID_FLOOR(bestx, bestz)->flag |= ID_FLOOR_FLAG_FURNISHED;
	}

	//
	// Put a plant in the corner.
	//

	for (i = 0; i < 4; i++)
	{
		x = (i & 1) ? ir->x1 : ir->x2;
		z = (i & 2) ? ir->z1 : ir->z2;
		
		if (ID_FLOOR(x,z)->room == room)
		{
			if (ID_FLOOR(x,z)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR))
			{
				//
				// This square is used.
				//
			}
			else
			{
				//
				// Put a tree here!
				//

				treex   = (x << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
				treez   = (z << ELE_SHIFT) + (1 << (ELE_SHIFT - 1));
				treey   = INDOORS_HEIGHT_FLOOR;
				treedir = ID_rand() & 2047;

				ID_add_furniture(treex, treey, treez, treedir, PRIM_OBJ_TREE);

				//
				// Mark the tree square as furnished.
				//

				ID_FLOOR(x, z)->flag |= ID_FLOOR_FLAG_FURNISHED;

				break;
			}
		}
	}

	if (WITHIN(coffeemx1, ID_floor_x1, ID_floor_x2) &&
		WITHIN(coffeemz1, ID_floor_z1, ID_floor_z2) &&
		WITHIN(coffeemx2, ID_floor_x1, ID_floor_x2) &&
		WITHIN(coffeemz2, ID_floor_z1, ID_floor_z2))
	{
		//
		// Do we have enough room for a coffee table?
		//

		if ((ID_FLOOR(coffeemx1, coffeemz1)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)) ||
			(ID_FLOOR(coffeemx2, coffeemz2)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR)))
		{
			//
			// The squares that the coffee table would have taken are
			// used up.
			//
		}
		else
		{
			//
			// Put down the coffee table!
			//

			ID_add_furniture(coffeex, coffeey, coffeez, coffeedir, PRIM_OBJ_COFFEE_TABLE);

			//
			// Mark the squares as furnished.
			//

			ID_FLOOR(coffeemx1, coffeemz1)->flag |= ID_FLOOR_FLAG_FURNISHED;
			ID_FLOOR(coffeemx2, coffeemz2)->flag |= ID_FLOOR_FLAG_FURNISHED;
		}
	}
	else
	{
		MSG_add("Tried to put the coffee table outside the building!");
	}

	return TRUE;
}



//
// Puts down radiators. Returns FALSE on failure.
//

SLONG ID_fit_radiators()
{
	SLONG i;
	SLONG j;
	SLONG size;
	SLONG big;
	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;
	SLONG x;
	SLONG z;
	SLONG mx;
	SLONG mz;
	SLONG dmx;
	SLONG dmz;
	SLONG dx;
	SLONG dz;
	SLONG radx;
	SLONG rady;
	SLONG radz;
	SLONG raddir;

	SLONG wall;
	SLONG bwmul;
	SLONG bwadd;
	SLONG length;

	ID_Room *ir;

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		if (ir->type == ID_ROOM_TYPE_KITCHEN ||
			ir->type == ID_ROOM_TYPE_WAREHOUSE)
		{
			//
			// Kitchens and warehouses don't have radiators.
			//

			continue;
		}

		//
		// What is the size of the room.
		//

		size = (ir->x2 - ir->x1) * (ir->z2 - ir->z1);

		//
		// Depending on the size of the room, we want a small
		// or a big radiator.
		//

		#define ID_BIG_RADIATOR_SIZE 13

		big = (size >= ID_BIG_RADIATOR_SIZE);

		//
		// Find the perimeter of the room.
		//

		if (!ID_calc_room_perim(i))
		{
			return FALSE;
		}

		//
		// Go around the perimeter of the room looking for a
		// place to put a radiator.
		//

		for (j = 0; j < ID_perim_upto - 1; j++)
		{
			x1 = ID_perim[j + 0].x;
			z1 = ID_perim[j + 0].z;

			x2 = ID_perim[j + 1].x;
			z2 = ID_perim[j + 1].z;

			dx = SIGN(x2 - x1);
			dz = SIGN(z2 - z1);

			if (dx && dz)
			{
				//
				// Ignore non-orthogonal walls.
				//

				continue;
			}

			//
			// What wall is this segment a part of?
			//

			ID_find_segment_wall(
				 x1, z1,
				 x2, z2,
				&wall,
				&bwmul,
				&bwadd,
				&length);

			//
			// Don't put radiators on outside walls.
			//

			ASSERT(WITHIN(wall, 0, ID_wall_upto - 1));

			if (ID_wall[wall].type == ID_WALL_T_OUTSIDE)
			{
				continue;
			}

			//
			// A tiny vector pointing to the right of the wall.
			//

			dmx = SIGN(-dz);
			dmz = SIGN( dx);

			x = x1;
			z = z1;

			while(x != x2 || z != z2)
			{
				mx = x + (x + dx) << 7;
				mz = z + (z + dz) << 7;

				mx += dmx;
				mz += dmz;

				mx >>= 8;
				mz >>= 8;

				//
				// (mx,mz) is the block of the room just inside
				// the wall.
				//

				if (ID_FLOOR(mx,mz)->flag & (ID_FLOOR_FLAG_INMAT | ID_FLOOR_FLAG_DOORMAT | ID_FLOOR_FLAG_FURNISHED | ID_FLOOR_FLAG_STAIR))
				{
					//
					// This floor square is taken.
					//
				}
				else
				{
					//
					// We could put a radiator here!
					//

					radx = x + (x + dx) << 7;
					radz = z + (z + dz) << 7;

					radx += dmx << 4;
					radz += dmz << 4;
					rady  = INDOORS_HEIGHT_FLOOR;

					raddir = -Arctan(dx,dz);

					//
					// Put a radiator here half the time...
					//

					if (ID_rand() & 0x100)
					{
						ID_add_furniture(radx, rady, radz, raddir, PRIM_OBJ_RADIATOR);
					}
				}

				x += dx;
				z += dz;
			}
		}
	}

	return TRUE;
}


//
// Puts down furniture. Returns FALSE on failure.
//

void ID_place_furniture()
{
	SLONG i;

	ID_Room *ir;

	//
	// Put down the stairs.
	//
	
	ID_fit_stairs();

	//
	// Put down the furniture.
	// 

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		switch(ir->type)
		{
			case ID_ROOM_TYPE_LOO:
				ID_fit_loo(i);
				break;
			case ID_ROOM_TYPE_KITCHEN:
				ID_fit_kitchen(i);
				break;
			case ID_ROOM_TYPE_LOUNGE:
				ID_fit_lounge(i);
				break;
			case ID_ROOM_TYPE_LOBBY:
				break;
			case ID_ROOM_TYPE_DINING:
				break;
			case ID_ROOM_TYPE_WAREHOUSE:
				break;
			case ID_ROOM_TYPE_OFFICE:
				break;
			case ID_ROOM_TYPE_MEETING:
				break;
			case ID_ROOM_TYPE_BEDROOM:
				break;
			case ID_ROOM_TYPE_CORRIDOR:
				break;

			default:
				ASSERT(0);
		}
	}

	//
	// Put down radiators, pictures and doors.
	//

	ID_fit_radiators();
	ID_fit_doors();
}


//
// Returns TRUE if the given wall would go through
// the middle of some stairs.
//

SLONG ID_goes_through_stairs(SLONG x1, SLONG z1, SLONG x2, SLONG z2)
{
	SLONG i;

	SLONG mid_x;
	SLONG mid_z;

	SLONG dx;
	SLONG dz;

	ID_Stair *it;

	//
	// This function only works if the wall is orthogonal.
	//

	ASSERT(x2 == x1 || z2 == z1);

	//
	// Put the wall into 8-bit fixed point.
	//

	x1 <<= 8;
	z1 <<= 8;
	x2 <<= 8;
	z2 <<= 8;

	//
	// Order the wall...
	//
	
	if (x1 > x2) {SWAP(x1, x2);}
	if (z1 > z2) {SWAP(z1, z2);}

	dx = x2 - x1;
	dz = z2 - z1;

	for (i = 0; i < ID_num_stairs; i++)
	{
		it = &ID_stair[i];

		//
		// The middle point of the stairs in fixed-point 8.
		//

		mid_x = it->x1 + (it->x1 + 1) + it->x2 + (it->x2 + 1) << 6;
		mid_z = it->z1 + (it->z1 + 1) + it->z2 + (it->z2 + 1) << 6;

		//
		// The wall goes through the stairs if it passes
		// through (mid_x, mid_z).
		//

		if (dx == 0 && mid_x == x1)
		{
			if (WITHIN(mid_z, z1, z2))
			{
				return TRUE;
			}
		}
		
		if (dz == 0 && mid_z == z1)
		{
			if (WITHIN(mid_x, x1, x2))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}




SLONG ID_generate_inside_walls(SLONG storey_type)
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;
	SLONG dx;
	SLONG dz;
	SLONG cx;
	SLONG cz;
	SLONG nx;
	SLONG nz;
	SLONG x1, x2;
	SLONG z1, z2;
	SLONG a;
	SLONG b;
	SLONG num_walls;
	SLONG wall_start_ok;
	SLONG wall_starts = 0;
	SLONG forbid_x    = INFINITY;
	SLONG forbid_z    = INFINITY;

	ID_Square *is;

	struct
	{
		SLONG dx;
		SLONG dz;
		
	} dir[4];

	//
	// The on-wall bits for the outside walls.
	//

	for	(i = 0; i < ID_wall_upto; i++)
	{
		ID_generate_on_wall_bits(i);
	}

	//
	// Places where we definitely start walls from.
	// 

	#define ID_MAX_WALL_STARTS 4

	struct
	{
		SLONG x;
		SLONG z;

	}     wall_start[ID_MAX_WALL_STARTS];
	SLONG wall_start_upto = 0;

	//
	// How many walls shall we put in?
	//

	num_walls  = ID_floor_area >> 4;
	num_walls += 1;

	//
	// For an apartement block we find the two places we start
	// to make the corridor.
	//

	if (storey_type == ID_STOREY_TYPE_APARTEMENT_GROUND ||
		storey_type == ID_STOREY_TYPE_APARTEMENT_UPPER)
	{
		num_walls  = ID_floor_area >> 3;
		num_walls += 2;

		if (ID_num_stairs != 0)
		{
			struct
			{
				UBYTE onwall;
				UBYTE duplicate;
				UBYTE x;
				UBYTE z;

			} corner[8];

			//
			// The corners of all the stair squares.
			//

			for (i = 0; i < 4; i++)
			{
				corner[i * 2 + 0].x = ID_stair[0].x1 + (i &  1);
				corner[i * 2 + 0].z = ID_stair[0].z1 + (i >> 1);

				corner[i * 2 + 1].x = ID_stair[0].x2 + (i &  1);
				corner[i * 2 + 1].z = ID_stair[0].z2 + (i >> 1);

				corner[i * 2 + 0].duplicate = FALSE;
				corner[i * 2 + 1].duplicate = FALSE;

				corner[i * 2 + 0].onwall = ID_FLOOR(corner[i * 2 + 0].x, corner[i * 2 + 0].z)->flag & ID_FLOOR_FLAG_ON_WALL;
				corner[i * 2 + 1].onwall = ID_FLOOR(corner[i * 2 + 1].x, corner[i * 2 + 1].z)->flag & ID_FLOOR_FLAG_ON_WALL;
			}

			//
			// Mark two points at the same place as one another.
			//

			for (i = 0; i < 8; i++)
			{
				for (j = i + 1; j < 8; j++)
				{
					if (corner[i].x == corner[j].x &&
						corner[i].z == corner[j].z)
					{
						corner[i].duplicate = TRUE;
						corner[j].duplicate = TRUE;
					}
				}
			}

			//
			// Start walls at all non-duplicate corner points that
			// are on an outside wall.
			//

			for (i = 0; i < 8; i++)
			{
				if (corner[i].onwall && !corner[i].duplicate)
				{
					if (WITHIN(wall_start_upto, 0, ID_MAX_WALL_STARTS))
					{
						wall_start[wall_start_upto].x = corner[i].x;
						wall_start[wall_start_upto].z = corner[i].z;

						wall_start_upto += 1;
					}
				}
			}

			if (wall_start_upto > 2)
			{
				//
				// Take out any wall starts that dont share an x or z
				// and are two squares away from any other start point.
				//

			  start_this_again:;

				for (i = 0; i < wall_start_upto; i++)
				{
					for (j = 0; j < wall_start_upto; j++)
					{
						if (j == i)
						{
							continue;
						}

						dx = wall_start[j].x - wall_start[i].x;
						dz = wall_start[j].z - wall_start[i].z;

						if ((dx == 0 && abs(dz) == 2) ||
							(dz == 0 && abs(dx) == 2))
						{
							//
							// Keep this start point.
							//

							goto keep_start_point;
						}
					}

					//
					// Get rid of this start point.
					//

					wall_start[i].x = wall_start[wall_start_upto - 1].x;
					wall_start[i].z = wall_start[wall_start_upto - 1].z;

					wall_start_upto -= 1;

					//
					// Start again!
					// 

					goto start_this_again;

				  keep_start_point:;
				}
			}

			//
			// Make sure that no walls split up this corridor.
			//

			for (i = 0; i < wall_start_upto; i++)
			{
				for (j = i + 1; j < wall_start_upto; j++)
				{
					dx = wall_start[j].x - wall_start[i].x;
					dz = wall_start[j].z - wall_start[i].z;

					if (abs(dx) == 2)
					{
						forbid_x = wall_start[i].x + wall_start[j].x >> 1;
					}

					if (abs(dz) == 2)
					{
						forbid_z = wall_start[i].z + wall_start[j].z >> 1;
					}
				}
			}
		}
	}
	else
	if (storey_type == ID_STOREY_TYPE_WAREHOUSE)
	{
		//
		// Less walls in a warehouse.
		//

		num_walls >>= 1;
	}
	
	while(num_walls--)
	{
		if (wall_starts < wall_start_upto)
		{
			x             = wall_start[wall_starts].x;
			z             = wall_start[wall_starts].z;
			wall_start_ok = TRUE;
		}
		else
		{
			wall_start_ok = ID_get_wall_start(&x, &z);
		}

		wall_starts += 1;

		if (wall_start_ok)
		{
			//
			// Try all four directions in turn.
			//

			dir[0].dx =  0;
			dir[0].dz =  1;

			dir[1].dx =  0;
			dir[1].dz = -1;

			dir[2].dx =  1;
			dir[2].dz =  0;

			dir[3].dx = -1;
			dir[3].dz =  0;

			//
			// Randomize the order in which we
			// try the directions.
			//

			for (i = 0; i < 3; i++)
			{
				a = ID_rand() & 0x3;
				b = ID_rand() & 0x3;

				SWAP(dir[a].dx, dir[b].dx);
				SWAP(dir[a].dz, dir[b].dz);
			}

			for (i = 0; i < 4; i++)
			{
				cx = x + dir[i].dx;
				cz = z + dir[i].dz;

				if (!WITHIN(cx, ID_floor_x1, ID_floor_x2) ||
					!WITHIN(cz, ID_floor_z1, ID_floor_z2))
				{
					continue;
				}

				is = ID_FLOOR(cx, cz);

				if ( (is->flag & ID_FLOOR_FLAG_ON_WALL) ||
					!(is->flag & ID_FLOOR_FLAG_P_IN))
				{
					continue;
				}

				//
				// Valid direction!
				//

				dx = dir[i].dx;
				dz = dir[i].dz;

				goto found_direction;
			}

			//
			// No direction works :-( We can't do a wall from
			// here.
			//

			if (ID_rand() & 0x10)
			{
				num_walls += 1;
			}

			continue;

		  found_direction:

			//
			// Add the wall until we hit another one.
			//

			x1 = x;
			z1 = z;

			while(1)
			{
				if (!WITHIN(x + dx, ID_floor_x1, ID_floor_x2) ||
					!WITHIN(z + dz, ID_floor_z1, ID_floor_z2))
				{
					break;
				}

				x += dx;
				z += dz;

				if (ID_FLOOR(x,z)->flag & ID_FLOOR_FLAG_ON_WALL)
				{
					//
					// Hit a wall.
					//

					break;
				}

				if (!(ID_FLOOR(x,z)->flag & ID_FLOOR_FLAG_P_IN))
				{
					//
					// We have leaked outside.
					//

					break;
				}
			}

			//
			// The end of the wall.
			//
			
			x2 = x;
			z2 = z;

			//
			// We dont insert this wall if it intersects with the middle
			// of a non-othogonal outside wall, goes through a staircase,
			// or splits up the corridor of an apartement block.
			//

			#define BETWEEN(x,a,b) (((a) < (b)) ? WITHIN(x,a,b) : WITHIN(x,b,a))

			if (ID_intersects_badly   (x1, z1, x2, z2) ||
				ID_goes_through_stairs(x1, z1, x2, z2) ||
				BETWEEN(forbid_x, x1, x2)               ||
				BETWEEN(forbid_z, z1, z2))
			{
				//
				// Invalid wall.
				//
			}
			else
			{
				//
				// Insert this wall.
				//
				
				ASSERT(WITHIN(ID_wall_upto, 0, ID_MAX_WALLS - 1));

				ID_wall[ID_wall_upto].type       = ID_WALL_T_INSIDE;
				ID_wall[ID_wall_upto].door[0]    = 255;
				ID_wall[ID_wall_upto].door[1]    = 255;
				ID_wall[ID_wall_upto].door[2]    = 255;
				ID_wall[ID_wall_upto].x1         = x1;
				ID_wall[ID_wall_upto].z1         = z1;
				ID_wall[ID_wall_upto].x2         = x2;
				ID_wall[ID_wall_upto].z2         = z2;
				ID_wall[ID_wall_upto].num_blocks = MAX(abs(x2- x1), abs(z2 - z1));

				ID_wall_upto += 1;

				//
				// Put in the on_wall flags for this new wall.
				//

				ID_generate_on_wall_bits(ID_wall_upto - 1);
			}
		}
		else
		{
			//
			// Could not find a place to start a wall from.
			//
		}
	}

	//
	// Generate the collision bits for the inside walls.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		if (ID_wall[i].type == ID_WALL_T_INSIDE)
		{
			ID_generate_collision_bits(i);
		}
	}

	//
	// Work out the rooms and mark them as lobbies, corridors etc...
	//

	ID_find_rooms();
	ID_generate_room_info();
	ID_assign_room_types(storey_type);

	//
	// This can fail sometimes.
	//

	if (!ID_make_connecting_doors(storey_type))
	{
		return FALSE;
	}

	//
	// Put a camera in each room.
	//

	ID_find_a_camera_for_each_room();

	return TRUE;
}


//
// Returns true if the layout contains rooms accessible from 
// only one other room.
//

SLONG ID_is_there_a_room_accessible_from_only_one_other_room(void)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;
	SLONG nx;
	SLONG nz;

	SLONG nextroom;

	ID_Room *ir;

	struct
	{
		SBYTE dx;
		SBYTE dz;

	} dir[4] = {{1,0},{-1,0},{0,1},{0,-1}};

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		nextroom = 0;

		for (x = ir->x1; x <= ir->x2; x++)
		for (z = ir->z1; z <= ir->z2; z++)
		{
			if (ID_FLOOR(x,z)->room == i)
			{
				//
				// Look for a neighbouring room.
				//

				for (j = 0; j < 4; j++)
				{
					nx = x + dir[j].dx;
					nz = z + dir[j].dz;

					if (WITHIN(nx, ID_floor_x1, ID_floor_x2 - 1) &&
						WITHIN(nz, ID_floor_z1, ID_floor_z2 - 1))
					{
						if (ID_FLOOR(nx,nz)->room)
						{
							if (nextroom == 0)
							{
								//
								// Found the first neighbouring room.
								//

								nextroom = ID_FLOOR(nx,nz)->room;
							}
							else
							{
								if (ID_FLOOR(nx,nz)->room != nextroom)
								{
									//
									// Found another neighbouring room!
									//

									goto this_room_has_at_least_two_neighbouring_rooms;
								}
							}
						}
					}
				}
			}
		}

		//
		// This room only has one neighbouring room.
		//

		return TRUE;

	  this_room_has_at_least_two_neighbouring_rooms:;
	}

	return FALSE;
}

SLONG ID_score_layout_house_ground()
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;
	SLONG nx;
	SLONG nz;

	SLONG width;
	SLONG height;
	SLONG ratio;
	SLONG nextroom;
	SLONG room;

	SLONG found_corridor;
	SLONG score;

	ID_Room  *ir;
	ID_Stair *it;

	//
	// We favour rectangular rooms whose ratio of width to
	// height is the golden ratio.  We favour having on long
	// thin room... but only one.
	//

	found_corridor = FALSE;
	score          = 0;

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		width  = ir->x2 - ir->x1 + 1;
		height = ir->z2 - ir->z1 + 1;

		#if WE_WANT_CORRIDORS

		if ((width  == 1 && height >= 5) ||
			(height == 1 && width  >= 5))
		{
			//
			// Found a corridor. One corridor is good, two is bad.
			//

			if (found_corridor)
			{
				score -= 4000;
			}
			else
			{
				score += 2000;

				found_corridor = TRUE;
			}
		}
		else
		
		#endif

		{
			if (width  > height) {ratio = (width  << 8) / height;}
			if (height > width)	 {ratio = (height << 8) / width;}

			//
			// We want 414 ... The golden ratio (1.618) * 256
			//

			if (ratio > 414) {score += (150 + (414 - ratio)) * 10;}
			if (ratio < 414) {score += (150 + (ratio - 414)) * 10;}

			if (ir->flag & ID_ROOM_FLAG_RECTANGULAR)
			{
				score += 1000;
			}
		}
	}

	//
	// Penalise layouts with rooms only accessible from one other
	// room.
	//

	if (ID_is_there_a_room_accessible_from_only_one_other_room())
	{
		score  *= 3;
		score >>= 2;
	}

	//
	// We dont want stairs in the toilets!
	//

	for (i = 0; i < ID_num_stairs; i++)
	{
		it = &ID_stair[i];

		room = ID_FLOOR(it->x1, it->z1)->room;

		ASSERT(WITHIN(room, 1, ID_room_upto - 1));

		if (ID_room[room].type == ID_ROOM_TYPE_LOO)
		{
			score -= score >> 1;
		}
	}

	return score;
}

SLONG ID_score_layout_warehouse()
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG size;
	SLONG biggest;
	SLONG score;

	ID_Room *ir;

	//
	// We want the biggest room to be really big!
	//

	biggest = -1;

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		dx = ir->x2 - ir->x1 + 1;
		dz = ir->z2 - ir->z1 + 1;

		size = dx * dz;

		if (size > biggest) {biggest = size;}
	}

	score = biggest << 8;

	//
	// But we like lots of rooms too...
	//

	score += ID_room_upto << 10;

	//
	// Room that are too long and thin are bad.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		dx = ir->x2 - ir->x1 + 1;
		dz = ir->z2 - ir->z1 + 1;

		if (dx > dz * 3 ||
			dz > dx * 3)
		{
			score = -1;
		}
	}

	return score;
}

SLONG ID_score_layout_apartement()
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;
	
	SLONG nx;
	SLONG nz;
	SLONG nroom;

	ID_Room *ir;

	SLONG score = ID_rand() & 0xff;

	UBYTE flat_room[ID_MAX_ROOMS];

	//
	// Count the number of rooms in each flat.
	//

	for (i = 1; i < ID_flat_upto; i++)
	{
		flat_room[i] = 0;
	}

	for (i = 1; i < ID_room_upto; i++)
	{
		ASSERT(WITHIN(ID_room[i].flat, 0, ID_MAX_ROOMS - 1));

		flat_room[ID_room[i].flat] += 1;
	}

	//
	// It is good for each flat to have the same number of rooms
	// and more than two rooms.
	//

	for (i = 1; i < ID_flat_upto; i++)
	{
		for (j = i + 1; j < ID_flat_upto; j++)
		{
			if (flat_room[i] == flat_room[j] && flat_room[i] > 2)
			{
				score += 0x10000;
				score += flat_room[i] * 0x10000;
			}
		}
	}

	//
	// It is good to have rooms connected to eachother, and
	// especially connected to the lounge.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		if (ir->type != ID_ROOM_TYPE_LOUNGE &&
			ir->type != ID_ROOM_TYPE_CORRIDOR)
		{
			//
			// Add a bonus for rooms connected to the lounge.
			//

			for (x = ir->x1; x <= ir->x2; x++)
			for (z = ir->z1; z <= ir->z2; z++)
			{
				if (ID_FLOOR(x,z)->room == i)
				{
					for (dx = -1; dx <= 1; dx += 2)
					for (dz = -1; dz <= 1; dz += 2)
					{
						nx = x + dx;
						nz = z + dz;

						if (WITHIN(nx, ID_floor_x1, ID_floor_x2) &&
							WITHIN(nz, ID_floor_z1, ID_floor_z2))
						{
							nroom = ID_FLOOR(nx,nz)->room;

							if (nroom != i && nroom)
							{
								ASSERT(WITHIN(nroom, 1, ID_room_upto - 1));

								//
								// If we have a lounge as a neighbouring room,
								// then it is good.
								//

								score += 512;

								goto look_at_next_room;
							}
						}
					}
				}
			}
		}

	  look_at_next_room:;
	}


	//
	// We hate long thin rooms.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ir = &ID_room[i];

		if (ir->type != ID_ROOM_TYPE_CORRIDOR)
		{
			dx = ir->x2 - ir->x1 + 1;
			dz = ir->z2 - ir->z1 + 1;

			if (dx > dz * 2 ||
				dz > dx * 2)
			{
				score = 0;
			}
		}
	}

	return score;
}


SLONG ID_score_layout(SLONG storey_type)
{
	SLONG ans;

	switch(storey_type)
	{
		case ID_STOREY_TYPE_HOUSE_GROUND:
		case ID_STOREY_TYPE_HOUSE_UPPER:
		case ID_STOREY_TYPE_OFFICE_GROUND:
		case ID_STOREY_TYPE_OFFICE_UPPER:
			ans = ID_score_layout_house_ground();
			break;

		case ID_STOREY_TYPE_WAREHOUSE:
			ans = ID_score_layout_warehouse();
			break;

		case ID_STOREY_TYPE_APARTEMENT_GROUND:
		case ID_STOREY_TYPE_APARTEMENT_UPPER:
			ans = ID_score_layout_apartement();
			break;

		default:
			ASSERT(0);
			break;
	}

	return ans;
}



//
// Works out which squares are inside and outside.
//

void ID_calculate_in_squares(void)
{
	SLONG   i;
	SLONG   j;
	SLONG	pos;
	UBYTE   wall_type;
	SLONG	x;
	SLONG	z;
	SLONG   x1, z1;
	SLONG   x2, z2;
	SLONG	dx;
	SLONG	dz;
	SLONG   dxdz;
	UBYTE   next;
	UBYTE   next1;
	UBYTE   next2;
	UBYTE  *prev;

	ID_Wall *iw;

	//
	// First work out which squares are outside and which squares are inside. Any squares
	// that are even a little bit inside are counted as being inside.
	//

	//
	// Clear the linked lists.
	//

	for (i = 0; i < ID_PLAN_SIZE; i++)
	{
		ID_edge[i] = 0;	// The NULL index
	}

	ID_link_upto = 1;	// We don't use index 0 because that is the NULL index.

	//
	// Go through each outside wall and build up the linked lists for each z-row.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		ASSERT(WITHIN(i, 0, ID_MAX_WALLS - 1));

		iw = &ID_wall[i];

		if (iw->z1 == iw->z2)
		{
			//
			// Ignore this wall because it does not cross any x-lines.
			//

			continue;
		}
		
		//
		// The wall's coordinates in 16-bit fixed point.
		//

		x1 = iw->x1 << 16;
		z1 = iw->z1 << 16;
		x2 = iw->x2 << 16;
		z2 = iw->z2 << 16;
		
		if (z1 > z2)
		{
			wall_type = ID_LINK_T_ENTER;

			//
			// Always go from top to bottom.
			//

			SWAP(x1, x2);
			SWAP(z1, z2);
		}
		else
		{
			wall_type = ID_LINK_T_LEAVE;
		}

		//
		// Go through the wall.
		//

		dx = x2 - x1;
		dz = z2 - z1;

		dxdz = DIV64(dx, dz);

		x = x1;
		z = z1;

		while(z < z2)
		{
			switch(wall_type)
			{
				case ID_LINK_T_ENTER: pos = MIN(x, x + dxdz); break;
				case ID_LINK_T_LEAVE: pos = MAX(x, x + dxdz); break;

				default:
					ASSERT(0);
					break;
			}

			//
			// Create a new link
			//

			ASSERT(WITHIN(ID_link_upto, 1, ID_MAX_LINKS - 1));

			ID_link[ID_link_upto].type = wall_type;
			ID_link[ID_link_upto].pos  = pos >> 8;	// Only 8-bit fixed point.
			ID_link[ID_link_upto].next = 0;
            
			//
			// Insert it in the correct place for this linked list.
			//

			ASSERT(WITHIN(z >> 16, 0, ID_FLOOR_SIZE - 1));

			next =  ID_EDGE(z >> 16);
			prev = &ID_EDGE(z >> 16);

			while(1)
			{
				ASSERT(WITHIN(next, 0, ID_MAX_LINKS - 1));

				if (next == 0 || ID_link[next].pos >= ID_link[ID_link_upto].pos)
				{
					//
					// This is where we should insert the new link.
					//

					*prev                      = ID_link_upto;
					ID_link[ID_link_upto].next = next;

					break;
				}

				prev = &ID_link[next].next;
				next =  ID_link[next].next;
			}

			//
			// Finished with this link now.
			//

			ID_link_upto += 1;

			//
			// Go onto the next line.
			//

			x += dxdz;
			z += 0x10000;
		}
	}

	//
	// Go through the linked lists and mark squares as inside or outside.
	//

	for (z = ID_floor_z1; z < ID_floor_z2; z++)
	{
		ASSERT(WITHIN(z, 0, ID_FLOOR_SIZE - 1));

		next = ID_EDGE(z);

		//
		// There should always be at least two links because they should
		// come in pairs.
		//

		ASSERT(next != 0);

		while(next)
		{
			ASSERT(WITHIN(next, 1, ID_link_upto - 1));

			//
			// They should come in pairs.
			//

			next1 = next;
			next2 = ID_link[next1].next;

			//
			// The pairs should be start and end.
			//

			ASSERT(WITHIN(next1, 1, ID_link_upto - 1));
			ASSERT(WITHIN(next2, 1, ID_link_upto - 1));

			//
			// Mark the squares in-between these start and end points as being
			// inside.
			//

			x1 = ID_link[next1].pos;
			x2 = ID_link[next2].pos;

			for (x = x1 & 0xff00; x < x2; x += 0x100)
			{
				ASSERT(WITHIN(x >> 8, 0, ID_FLOOR_SIZE - 1));

				ID_FLOOR(x >> 8, z)->flag |= ID_FLOOR_FLAG_INSIDE;

				//
				// If the square is completely inside, then it is
				// not outside.
				//

				if (x >= x1 && x + 0x100 <= x2)
				{
					ID_FLOOR(x >> 8, z)->flag &= ~ID_FLOOR_FLAG_OUTSIDE;
				}
			}

			//
			// Go onto the next pair.
			//

			next = ID_link[next2].next;
		}
	}
}


//
// Works out which points are inside the building.
// Points on the wall count as being inside.
//
// Returns FALSE on failure.
//

SLONG ID_calculate_in_points(void)
{
	SLONG   i;
	SLONG   j;
	SLONG	pos;
	UBYTE   wall_type;
	SLONG	x;
	SLONG	z;
	SLONG   x1, z1;
	SLONG   x2, z2;
	SLONG	dx;
	SLONG	dz;
	SLONG   dxdz;
	UBYTE   next;
	UBYTE   next1;
	UBYTE   next2;
	UBYTE  *prev;

	ID_Wall *iw;

	//
	// Clear the linked lists.
	//

	for (i = 0; i < ID_PLAN_SIZE; i++)
	{
		ID_edge[i] = 0;	// The NULL index
	}

	ID_link_upto = 1;	// We don't use index 0 because that is the NULL index.

	//
	// Go through each outside wall and build up the linked lists for each z-row.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		ASSERT(WITHIN(i, 0, ID_MAX_WALLS - 1));

		iw = &ID_wall[i];

		if (iw->z1 == iw->z2)
		{
			//
			// Ignore this wall because it does not cross any x-lines.
			//

			continue;
		}
		
		//
		// The wall's coordinates in 16-bit fixed point.
		//

		x1 = iw->x1 << 16;
		z1 = iw->z1 << 16;
		x2 = iw->x2 << 16;
		z2 = iw->z2 << 16;
		
		if (z1 > z2)
		{
			wall_type = ID_LINK_T_ENTER;

			//
			// Always go from top to bottom.
			//

			SWAP(x1, x2);
			SWAP(z1, z2);
		}
		else
		{
			wall_type = ID_LINK_T_LEAVE;
		}

		//
		// Go through the wall.
		//

		dx = x2 - x1;
		dz = z2 - z1;

		dxdz = DIV64(dx, dz);

		x = x1;
		z = z1;

		while(z <= z2)
		{
			//
			// Create a new link
			//

			ASSERT(WITHIN(ID_link_upto, 1, ID_MAX_LINKS - 1));

			ID_link[ID_link_upto].type = wall_type;
			ID_link[ID_link_upto].pos  = x >> 8;	// Only 8-bit fixed point.
			ID_link[ID_link_upto].next = 0;
            
			//
			// Insert it in the correct place for this linked list.
			//

			ASSERT(WITHIN(z >> 16, 0, ID_FLOOR_SIZE - 1));

			next =  ID_EDGE(z >> 16);
			prev = &ID_EDGE(z >> 16);

			while(1)
			{
				ASSERT(WITHIN(next, 0, ID_MAX_LINKS - 1));

				if (next == 0 || ID_link[next].pos >= ID_link[ID_link_upto].pos)
				{
					//
					// Make sure that two links at the same pos appear in the
					// linked list in the order ENTER then LEAVE.
					//

					if (ID_link[next].pos  == ID_link[ID_link_upto].pos &&
						ID_link[next].type == ID_LINK_T_ENTER)
					{
						//
						// Insert it after this link.
						//
					}
					else
					{
						//
						// This is where we should insert the new link.
						//

						*prev                      = ID_link_upto;
						ID_link[ID_link_upto].next = next;

						break;
					}
				}

				prev = &ID_link[next].next;
				next =  ID_link[next].next;
			}

			//
			// Finished with this link now.
			//

			ID_link_upto += 1;

			//
			// Go onto the next line.
			//

			x += dxdz;
			z += 0x10000;
		}
	}

	//
	// Go through the linked lists and mark points as inside or outside.
	//

	for (z = ID_floor_z1; z <= ID_floor_z2; z++)
	{
		ASSERT(WITHIN(z, 0, ID_FLOOR_SIZE - 1));

		prev = &ID_EDGE(z);
		next =  ID_EDGE(z);

		// 
		// Take out ENTER/ENTER LEAVE/LEAVE sequences.
		//

		while(1)
		{
			ASSERT(WITHIN(next, 1, ID_link_upto - 1));

			next1 = next;
			next2 = ID_link[next].next;

			if (next1 == NULL || next2 == NULL)
			{
				break;
			}

			
			if (ID_link[next1].type == ID_LINK_T_ENTER &&
				ID_link[next2].type == ID_LINK_T_ENTER)
			{
				//
				// Use the first value...
				//

				ID_link[next1].next = ID_link[next2].next;

				next = next1;
			}
			else
			if (ID_link[next1].type == ID_LINK_T_LEAVE &&
				ID_link[next2].type == ID_LINK_T_LEAVE)
			{
				//
				// Use the second value.
				//

			   *prev = next2;

				next = next2;
			}
			else
			{
				prev = &ID_link[next1].next;
				next =  next2;
			}
		}

		//
		// Mark inside points...
		//

		next = ID_EDGE(z);

		while(next)
		{
			if (!WITHIN(next,               1, ID_link_upto - 1)) {return FALSE;}
			if (!WITHIN(ID_link[next].next, 1, ID_link_upto - 1)) {return FALSE;}

			next1 = next;
			next2 = ID_link[next].next;

			//
			// Mark the points in-between these start and end points as being
			// inside.
			//

			x1 = ID_link[next1].pos;
			x2 = ID_link[next2].pos;

			for (x = ID_floor_x1; x <= ID_floor_x2; x++)
			{
				if (WITHIN(x << 8, x1, x2))
				{
					ID_FLOOR(x, z)->flag |= ID_FLOOR_FLAG_P_IN;
				}
			}

			//
			// Go onto the next pair.
			//

			next = ID_link[next2].next;
		}
	}

	return TRUE;
}

SLONG ID_generate_floorplan(SLONG storey_type, ID_Stair stair[], SLONG num_stairs, UWORD seed, UBYTE find_good_layout, UBYTE furnished)
{
	SLONG   i;
	SLONG   j;
	SLONG	pos;
	UBYTE   wall_type;
	SLONG	x;
	SLONG	z;
	SLONG   x1, z1;
	SLONG   x2, z2;
	SLONG	dx;
	SLONG	dz;
	SLONG	cx;
	SLONG	cz;
	SLONG   dxdz;
	SLONG   best_score;
	UWORD   best_seed;
	UWORD   seed_used;
	UBYTE   next;
	UBYTE   next1;
	UBYTE   next2;
	UBYTE  *prev;
	SLONG   num_walls;
	SLONG   find_best_start;

	ID_Wall  *iw;
	ID_Stair *is;

	//
	// Remember where the stairs are.
	//

	ID_stair      = stair;
	ID_num_stairs = num_stairs;

	//
	// Remember the storey type.
	//

	ID_storey_type = storey_type;

	//
	// Make sure that the bounding box does not exceed the maximum size of
	// the floorplan.
	//

	ASSERT(ID_floor_x2 - ID_floor_x1 <= ID_PLAN_SIZE);
	ASSERT(ID_floor_z2 - ID_floor_z1 <= ID_PLAN_SIZE);

	//
	// Fail if there are any zero length walls.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		dx = iw->x2 - iw->x1;
		dz = iw->z2 - iw->z1;

		if (!(dx | dz))
		{
			//
			// A zero-length wall!
			//

			return -1;
		}
	}

	//
	// First work out which squares are outside and which squares are inside. Any squares
	// that are even a little bit inside are counted as being inside.
	//

	ID_calculate_in_squares();

	//
	// Work out which points are inside the building. Points on the wall count as
	// being inside.
	//

	if (!ID_calculate_in_points())
	{
		TRACE("Complete ID failure 1.\n");

		return -1;
	}

	//
	// Mark the staircases.
	//

	for (i = 0; i < ID_num_stairs; i++)
	{
		is = &ID_stair[i];

		ID_FLOOR(is->x1, is->z1)->flag |= ID_FLOOR_FLAG_STAIR;
		ID_FLOOR(is->x2, is->z2)->flag |= ID_FLOOR_FLAG_STAIR;
	}

	//
	// Work out how many squares are inside the building.
	//

	ID_floor_area = 0;

	for (x = ID_floor_x1; x < ID_floor_x2; x++)
	for (z = ID_floor_z1; z < ID_floor_z2; z++)
	{
		if (ID_FLOOR(x, z)->flag & ID_FLOOR_FLAG_INSIDE)
		{
			ID_floor_area += 1;
		}
	}

	//
	// Store the seed.
	//

	ID_srand(seed);

	if (find_good_layout)
	{
		//
		// We try putting wall in a few different ways, then
		// pick the best 
		//

		#define ID_MAX_FITS 32

		struct
		{
			UWORD seed;
			UWORD shit;
			SLONG score;

		} fit[ID_MAX_FITS];

		find_best_start = seed;

		for (i = 0; i < ID_MAX_FITS; i++)
		{
			//
			// Remember the seed that generates these walls... we only use
			// seeds that can fit into a UWORD.
			//

			fit[i].seed = find_best_start;

			//
			// Use the seed that has been truncated to a UWORD.
			//

			ID_srand(fit[i].seed);
			
			//
			// Generate the walls.
			//

			if (!ID_generate_inside_walls(storey_type))
			{
				//
				// This turned out very badly!
				//

				fit[i].score = -INFINITY;
			}
			else
			{
				//
				// Score this layout.
				//

				fit[i].score = ID_score_layout(storey_type);
			}

			//
			// Clear information.
			//

			ID_clear_inside_walls();

			//
			// Try the next seed.
			//

			find_best_start += 1;
			find_best_start &= 0xffff;
		}

		//
		// Which seed generates the best walls?
		//

		best_score = -INFINITY;
		best_seed  =  0;

		for (i = 0; i < ID_MAX_FITS; i++)
		{
			if (fit[i].score > best_score)
			{
				best_score = fit[i].score;
				best_seed  = fit[i].seed;
			}
		}

		if (best_score >= 0)
		{
			//
			// Use the best seed to generate the walls.
			//

			ID_srand(best_seed);
			ID_generate_inside_walls(storey_type);
			seed_used = best_seed;
		}
		else
		{
			//
			// What do we do now?
			//

			TRACE("Complete ID failure 2.\n");

			return -1;
		}
	}
	else
	{
		//
		// Use the seed we were given.
		//

		ID_generate_inside_walls(storey_type);

		seed_used = seed;
	}

	#if YOU_WANT_THIN_WALLS

	//
	// Add the wall faces into the mapwho.
	//

	for (i = 0; i < ID_wall_upto; i++)
	{
		ID_add_wall_faces(i);
	}

	#else

	//
	// Add the faces for each room.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ID_add_room_faces(i);
	}

	#endif

	ID_furn_upto = 0;

	if (furnished)
	{
		//
		// Add the furniture.
		//

		ID_place_furniture();
	}

	ASSERT(WITHIN(seed_used, 0, 65535));
	ASSERT(find_good_layout || seed == seed_used);

	return seed_used;
}


//
// Returns the position of (x,z) in stair space...
//

void ID_get_stair_space_position(
		SLONG  x,
		SLONG  z,
		SLONG  sx1,	// The first stair square.
		SLONG  sz1,
		SLONG  sx2, // The second stair square.
		SLONG  sz2,
		SLONG *stair_x,
		SLONG *stair_z)
{
	SLONG dx;
	SLONG dz;
	
	SLONG tx;
	SLONG tz;

	SLONG angle;
	SLONG origin_x;
	SLONG origin_z;

	SLONG matrix[4];

	dx = sx2 - sx1;
	dz = sz2 - sz1;

	//
	// Find the origin and angle of the stairs.
	//

	if (dx == 0 && dz == 1)
	{
		origin_x = sx1 + 0 << 8;
		origin_z = sz1 + 0 << 8;

		angle = 0;	// * 90 degrees...
	}
	else
	if (dx == 1 && dz == 0)
	{
		origin_x = sx1 + 0 << 8;
		origin_z = sz1 + 1 << 8;

		angle = 1;	// * 90 degrees...
	}
	else
	if (dx == 0 && dz == -1)
	{
		origin_x = sx1 + 1 << 8;
		origin_z = sz1 + 1 << 8;

		angle = 2; // * 90 degrees...
	}
	else
	if (dx == -1 && dz == 0)
	{
		origin_x = sx1 + 1 << 8;
		origin_z = sz1 + 0 << 8;

		angle = 3; // * 90 degrees...
	}
	else
	{
		ASSERT(0);
	}

	//
	// Find the 2d rotation matrix from angle.
	//
	//			|  cos a   sin a |
	// matrix = |				 |
	//			| -sin a   cos a |
	//

	angle *= 512;

	matrix[0] =  COS(angle);
	matrix[1] = -SIN(angle);
	matrix[2] =  SIN(angle);
	matrix[3] =  COS(angle);

	//
	// Find (sx,sz)... the position of (x,z) in stair-space.
	//

	tx = x - origin_x;
	tz = z - origin_z;

   *stair_x = MUL64(tx, matrix[0]) + MUL64(tz, matrix[1]);
   *stair_z = MUL64(tx, matrix[2]) + MUL64(tz, matrix[3]);

   //
   // This is WIERD!!! I don't know why.
   //

	*stair_x = 0x100 - *stair_x;
}

//
// Returns the position you teleport up to.
//

void ID_teleport_up_pos(ID_Stair *it, SLONG *tx, SLONG *tz)
{
	SLONG dx;
	SLONG dz;

	ASSERT(WITHIN(it, &ID_stair[0], &ID_stair[ID_num_stairs - 1]));

	dx = it->x2 - it->x1 << 8;
	dz = it->z2 - it->z1 << 8;

   *tx = (it->x2 << 8) + (dx >> 1) - (dz >> 2) + (dx >> 5);
   *tz = (it->z2 << 8) + (dz >> 1) + (dx >> 2) + (dz >> 5);

   *tx = (it->x2 << 8) + 0x80;
   *tz = (it->z2 << 8) + 0x80;
}

//
// Returns the position you teleport down to.
//

void ID_teleport_down_pos(ID_Stair *it, SLONG *tx, SLONG *tz)
{
	SLONG dx;
	SLONG dz;

	ASSERT(WITHIN(it, &ID_stair[0], &ID_stair[ID_num_stairs - 1]));

	dx = it->x2 - it->x1 << 8;
	dz = it->z2 - it->z1 << 8;

   *tx = (it->x2 << 8) - (dz >> 2) - (dx >> 5);
   *tz = (it->z2 << 8) + (dx >> 2) - (dz >> 5);

   *tx = (it->x1 << 8) + 0x80;
   *tz = (it->z1 << 8) + 0x80;
}

SLONG ID_change_floor(
		SLONG  x,
		SLONG  z,
		SLONG *new_x,
		SLONG *new_z,
		SLONG *handle)
{
	SLONG i;

	SLONG map_x = x >> 8;
	SLONG map_z = z >> 8;

	SLONG sx;
	SLONG sz;

	SLONG quadrant;
	SLONG ans;

	ID_Stair *it;

	if (ID_FLOOR(map_x, map_z)->flag & ID_FLOOR_FLAG_STAIR)
	{
		//
		// We are on some stairs. Which stair-case is it?
		//

		for (i = 0; i < ID_num_stairs; i++)
		{
			it = &ID_stair[i];

			if ((it->x1 == map_x && it->z1 == map_z) ||
				(it->x2 == map_x && it->z2 == map_z))
			{
				//
				// This is the set of stairs we are on. Find our
				// position in stair space.
				//

				ID_get_stair_space_position(
					x, z, 
					it->x1, it->z1,
					it->x2, it->z2,
				   &sx,
				   &sz);

				// The stairs are all the same size.
				//

				ASSERT(
					WITHIN(sx, 0, 0x100) &&
					WITHIN(sz, 0, 0x200));

				//
				// The right hand side is always a corridor.
				//

				if (sx >= 0x80)
				{
					return 0;
				}

				//
				// Which z-quadrant are we in?
				//

				quadrant = sz >> 7;

				//
				// Are we in the active quadrant?
				//

				if (quadrant == 2)
				{
					switch(it->type)
					{
						case ID_STAIR_TYPE_BOTTOM:
							ans = +1;
							break;
						case ID_STAIR_TYPE_MIDDLE:
							ans = (sz > 0x140) ? -1 : +1;
							break;
						case ID_STAIR_TYPE_TOP:
							ans = -1;
							break;
						default:
							ans = 0;
							ASSERT(0);
							break;
					}

					//
					// Where we are on the next storey.
					//

					if (ans == +1)
					{
						ID_teleport_up_pos(it, new_x, new_z);
					   *handle = it->handle_up;
					}
					else
					{
						ASSERT(ans == -1);

						ID_teleport_down_pos(it, new_x, new_z);
					   *handle = it->handle_down;
					}

					return ans;
				}

				return 0;
			}
		}

		//
		// We are on floor-square marked as being on some
		// stairs, but we can't find those stairs!
		//

		ASSERT(0);
	}
	else
	{
		//
		// You can only change floors if you are on some stairs!
		//
	}

	return 0;
}




void ID_get_floorplan_bounding_box(
		SLONG *x1,
		SLONG *z1,
		SLONG *x2,
		SLONG *z2)
{
	*x1 = ID_floor_x1;
	*z1 = ID_floor_z1;
	*x2 = ID_floor_x2;
	*z2 = ID_floor_z2;
}

SLONG ID_am_i_completely_outside(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, ID_FLOOR_SIZE - 1));
	ASSERT(WITHIN(z, 0, ID_FLOOR_SIZE - 1));

	//
	// Outside here means completely outside
	//

	return !(ID_FLOOR(x,z)->flag & ID_FLOOR_FLAG_INSIDE);
}

SLONG ID_get_mapsquare_texture(SLONG x, SLONG z,
		float *u0, float *v0,
		float *u1, float *v1,
		float *u2, float *v2,
		float *u3, float *v3)
{
	UWORD texture;
	SLONG page;

	ASSERT(WITHIN(ID_FLOOR(x,z)->texid, 0, ID_MAX_FLOOR_TEXTURES - 1));

	texture = ID_floor_texture[ID_FLOOR(x,z)->texid];

	SLONG i;
	ID_Stair *it;

	for (i = 0; i < ID_num_stairs; i++)
	{
		it = &ID_stair[i];

		if (it->x1 == x && it->z1 == z) {texture = 3;}
		if (it->x2 == x && it->z2 == z) {texture = rand() % ID_MAX_FLOOR_TEXTURES;}
	}

	page = ID_get_texture_uvs(texture, u0,v0, u1,v1, u2,v2, u3,v3);

	return page;
}

SLONG ID_should_i_draw_mapsquare(SLONG x, SLONG z)
{
	return !(ID_FLOOR(x,z)->flag & ID_FLOOR_FLAG_STAIR) || Keys[KB_8];
}


void ID_this_is_where_i_am(SLONG x, SLONG z)
{
	SLONG i;
	SLONG mx;
	SLONG mz;
	SLONG dx;
	SLONG dz;
	SLONG room;

	//
	// Mark all rooms an unseeable.
	//

	for (i = 1; i < ID_room_upto; i++)
	{
		ID_room[i].flag &= ~ID_ROOM_FLAG_SEEABLE;
	}

	if (WITHIN(x, ID_floor_x1, ID_floor_x2 - 1) &&
		WITHIN(z, ID_floor_z1, ID_floor_z2 - 1))
	{
		room = ID_FLOOR(x,z)->room;

		if (room == 0)
		{
			//
			// The player isn't inside any room!
			//
		}
		else
		{
			ASSERT(WITHIN(room, 1, ID_room_upto - 1));

			//
			// The room that the player is inside is seeable.
			//

			ID_room[room].flag |= ID_ROOM_FLAG_SEEABLE;

			//
			// If you are standing on a doorway, draw
			// the rooms connected via this door.
			//

			if (ID_FLOOR(x,z)->flag & ID_FLOOR_FLAG_INMAT)
			{
				for (i = 0; i < 4; i++)
				{
					dx = 0;
					dz = 0;

					switch(i)
					{
						case 0: dx = -1; break;
						case 1: dx = +1; break;
						case 2: dz = -1; break;
						case 3: dz = +1; break;
						default:
							ASSERT(0);
					}

					mx = x + dx;
					mz = z + dz;

					if (WITHIN(mx, ID_floor_x1, ID_floor_x2 - 1) &&
						WITHIN(mz, ID_floor_z1, ID_floor_z2 - 1))
					{
						room = ID_FLOOR(mx,mz)->room;

						if (room)
						{
							ASSERT(WITHIN(room, 0, ID_room_upto - 1));

							//
							// Mark this room as seeable too.
							//

							ID_room[room].flag |= ID_ROOM_FLAG_SEEABLE;
						}
					}
				}
			}
		}
	}
}

SLONG ID_should_i_draw(SLONG x, SLONG z)
{
	SLONG room;

	ASSERT(WITHIN(x, ID_floor_x1, ID_floor_x2 - 1));
	ASSERT(WITHIN(z, ID_floor_z1, ID_floor_z2 - 1));

	room = ID_FLOOR(x,z)->room;

	ASSERT(WITHIN(room, 0, ID_room_upto - 1));

	return (!Keys[KB_A] || (ID_room[room].flag & ID_ROOM_FLAG_SEEABLE));
}


//
// Accessing the faces.
//

SLONG ID_get_first_face(SLONG x, SLONG z)
{
	SLONG ans;

	ASSERT(WITHIN(x, ID_floor_x1, ID_floor_x2 - 1));
	ASSERT(WITHIN(z, ID_floor_z1, ID_floor_z2 - 1));

	ans = ID_FLOOR(x, z)->next;

	return ans;
}

SLONG ID_is_face_a_quad(SLONG face)
{
	SLONG ans;

	ASSERT(WITHIN(face, 1, ID_face_upto - 1));

	ans = ID_face[face].flag & ID_FACE_FLAG_QUAD;

	return ans;
}

UWORD ID_get_face_texture(SLONG face)
{
	UWORD ans;

	ASSERT(WITHIN(face, 1, ID_face_upto - 1));

	ans = ID_face[face].texture;

	return ans;
}

SLONG ID_get_next_face(SLONG face)
{
	SLONG ans;

	ASSERT(WITHIN(face, 1, ID_face_upto - 1));

	ans = ID_face[face].next;

	return ans;
}


//
// Accessing the points of the faces.
//

void ID_clear_indices()
{
	SLONG i;

	for (i = ID_point_upto - 1; i >= 0; i--)
	{
		ASSERT(WITHIN(i, 0, ID_MAX_POINTS - 1));

		ID_point[i].index = 0;
	}
}

SLONG ID_is_point_a_mapsquare(SLONG face, SLONG point)
{
	ASSERT(WITHIN(face,  1, ID_face_upto - 1));
	ASSERT(WITHIN(point, 0, 3 + ((ID_face[face].flag & ID_FACE_FLAG_QUAD) ? 1 : 0)));

	return (ID_face[face].flag & (ID_FACE_FLAG_ONFLOOR0 << point));
}

void ID_get_point_mapsquare(SLONG face, SLONG point, SLONG *x, SLONG *z)
{
	SLONG p_index;

	ASSERT(WITHIN(face,  1, ID_face_upto - 1));
	ASSERT(WITHIN(point, 0, 3 + ((ID_face[face].flag & ID_FACE_FLAG_QUAD) ? 1 : 0)));

	ASSERT(ID_is_point_a_mapsquare(face, point));

	p_index = ID_face[face].point[point];

	*x = p_index  & 0xff;
	*z = p_index >> 8;
}

void ID_get_point_position(SLONG face, SLONG point, SLONG *x, SLONG *y, SLONG *z)
{
	SLONG p_index;

	ASSERT(WITHIN(face,  1, ID_face_upto - 1));
	ASSERT(WITHIN(point, 0, 3 + ((ID_face[face].flag & ID_FACE_FLAG_QUAD) ? 1 : 0)));

	ASSERT(!ID_is_point_a_mapsquare(face, point));

	p_index = ID_face[face].point[point];

	ASSERT(WITHIN(p_index, 0, ID_point_upto - 1));

	*x = ID_point[p_index].x;
	*y = ID_point[p_index].y;
	*z = ID_point[p_index].z;

	//
	// Incase ELE_SHIFT stops being 8 and we need to
	// fix the shifts...
	//

	ASSERT(ELE_SHIFT == 8);
}

UWORD ID_get_point_index(SLONG face, SLONG point)
{
	SLONG p_index;
	UWORD ans;

	ASSERT(WITHIN(face,  1, ID_face_upto - 1));
	ASSERT(WITHIN(point, 0, 3 + ((ID_face[face].flag & ID_FACE_FLAG_QUAD) ? 1 : 0)));

	ASSERT(!ID_is_point_a_mapsquare(face, point));

	p_index = ID_face[face].point[point];

	ASSERT(WITHIN(p_index, 0, ID_point_upto - 1));

	ans = ID_point[p_index].index;

	return ans;
}

void ID_set_point_index(SLONG face, SLONG point, UWORD index)
{
	SLONG p_index;

	ASSERT(WITHIN(face,  1, ID_face_upto - 1));
	ASSERT(WITHIN(point, 0, 3 + ((ID_face[face].flag & ID_FACE_FLAG_QUAD) ? 1 : 0)));

	ASSERT(!ID_is_point_a_mapsquare(face, point));

	p_index = ID_face[face].point[point];

	ASSERT(WITHIN(p_index, 0, ID_point_upto - 1));

	ID_point[p_index].index = index;
}


UBYTE ID_get_mapsquare_room(SLONG x, SLONG z)
{
	UBYTE ans;

	if (!WITHIN(x, ID_floor_x1, ID_floor_x2 - 1) ||
		!WITHIN(z, ID_floor_z1, ID_floor_z2 - 1))
	{
		//
		// Outside the bounding box...
		//

		return 0;
	}

	if (!(ID_FLOOR(x, z)->flag & ID_FLOOR_FLAG_INSIDE))
	{
		//
		// Not inside the building.
		//

		return 0;
	}

	ans = ID_FLOOR(x,z)->room;

	return ans;
}

void ID_get_room_camera(UBYTE room, SLONG *x, SLONG *y, SLONG *z)
{
	ASSERT(WITHIN(room, 1, ID_room_upto - 1));

	*x = ID_room[room].cam_x;
	*z = ID_room[room].cam_z;

	*y = INDOORS_HEIGHT_CEILING - 0x10;
}


void ID_remove_inside_things(void)
{
	SLONG i;

	Thing *p_thing;

	#define ID_MAX_INSIDE_THINGS 64

	THING_INDEX inside_thing[ID_MAX_INSIDE_THINGS];
	SLONG       num_inside_things;

	num_inside_things = THING_find_box(
							ID_floor_x1,
							ID_floor_z1,
							ID_floor_x2,
							ID_floor_z2,
							inside_thing,
							ID_MAX_INSIDE_THINGS,
							THING_FIND_EVERYTHING);

	for (i = 0; i < num_inside_things; i++)
	{
		p_thing = TO_THING(inside_thing[i]);

		if (p_thing->Flags & FLAGS_INDOORS_GENERATED)
		{
			//
			// Remove this thing.
			//

			THING_kill(p_thing);
		}
	}
}


SLONG ID_get_face_texture(SLONG face,
		float *u0, float *v0,
		float *u1, float *v1,
		float *u2, float *v2,
		float *u3, float *v3)
{
	UWORD texture;
	SLONG page;

	ASSERT(WITHIN(face, 1, ID_face_upto - 1));

	texture = ID_face[face].texture;

	page = ID_get_texture_uvs(texture, u0,v0, u1,v1, u2,v2, u3,v3);

	return page;
}






// ########################################################
// ========================================================
//
// COLLISION CODE...
//
// ========================================================
// ########################################################


SLONG ID_collide_3d(
		SLONG x1, SLONG y1, SLONG z1,
		SLONG x2, SLONG y2, SLONG z2)
{
	return FALSE;
}


SLONG ID_calc_height_at(SLONG x, SLONG z)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG origin_x;
	SLONG origin_z;
	SLONG angle;

	SLONG tx;
	SLONG tz;

	SLONG sx;
	SLONG sz;

	SLONG ans;

	SLONG matrix[4];

	//
	// I can't be arsed to write ELE_SHIFT all the time, lets hope
	// it stays the same... I bet it will!
	//

	ASSERT(ELE_SHIFT == 8);

	SLONG x_map = x >> 8;
	SLONG z_map = z >> 8;

	ID_Square *is;
	ID_Stair  *it;

	is = ID_FLOOR(x_map, z_map);

	//
	// Make sure this square is inside the building.
	//

	ASSERT(is->flag & ID_FLOOR_FLAG_INSIDE);

	//
	// If we are not on some stairs, then it is easy!
	//

	if (!(is->flag & ID_FLOOR_FLAG_STAIR))
	{
		return INDOORS_HEIGHT_FLOOR;
	}

	//
	// We are on a square with some stairs. Which set
	// of stairs is it?
	//

	for (i = 0; i < ID_num_stairs; i++)
	{
		it = &ID_stair[i];

		if ((x_map == it->x1 && z_map == it->z1) ||
			(x_map == it->x2 && z_map == it->z2))
		{
			//
			// This is the set of stairs we want.
			//

			goto found_stairs;
		}
	}

	//
	// A very strange occurence! The floor flags says that there
	// are some stairs here, but we can't find a set of stairs
	// in the array here.
	//

	ASSERT(0);

	return INDOORS_HEIGHT_FLOOR;

  found_stairs:

	//
	// Find our position in stair-space.
	//

	ID_get_stair_space_position(
		x, z, 
		it->x1, it->z1,
		it->x2, it->z2,
	   &sx,
	   &sz);

	//
	// The stairs are all the same size.
	//

	ASSERT(
		WITHIN(sx, 0, 0x100) &&
		WITHIN(sz, 0, 0x200));

	//
	// The right hand side is always a corridor.
	//

	if (sx >= 0x80)
	{
		return INDOORS_HEIGHT_FLOOR;
	}

	//
	// From now on it depends on the type of stair...
	//

	switch(it->type)
	{
		case ID_STAIR_TYPE_BOTTOM:

			//	   _
			//	  /
			//  _/	  Ground

			ans = sz - 0x80 + INDOORS_HEIGHT_FLOOR;

			SATURATE(ans, INDOORS_HEIGHT_FLOOR, INDOORS_HEIGHT_CEILING);

			break;

		case ID_STAIR_TYPE_MIDDLE:

			//
			//  _/ _  Ground
			//	  /

			ans = sz - 0x80 + INDOORS_HEIGHT_FLOOR;

			SATURATE(ans, INDOORS_HEIGHT_FLOOR, INDOORS_HEIGHT_CEILING);

			if (sz >= 0x100)
			{
				ans -= INDOORS_HEIGHT_CEILING - INDOORS_HEIGHT_FLOOR;
			}

			break;

		case ID_STAIR_TYPE_TOP:

			//  _	_ Ground
			// 	   /
			//	  /

			if (WITHIN(sz, 0x80, 0x180))
			{
				ans = INDOORS_HEIGHT_FLOOR -0x100 + sz;
			}
			else
			{
				ans = INDOORS_HEIGHT_FLOOR;
			}

			break;
		default:
			ASSERT(0);
			break;
	}

	return ans;
}







SLONG ID_collide_2d(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG  radius,
		SLONG *slide_x,
		SLONG *slide_z)
{
	SLONG i;
	SLONG j;

	SLONG dx;
	SLONG dz;

	SLONG wx1;
	SLONG wz1;
	SLONG wx2;
	SLONG wz2;

	SLONG min;
	SLONG max;

	SLONG start;
	SLONG end;
	SLONG doorway_start;
	SLONG doorway_end;

	SLONG collided;

	ID_Wall *iw;

	//
	// Collision with the walls treated as sausages
	//

	collided = FALSE;

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		dx = iw->x2 - iw->x1;
		dz = iw->z2 - iw->z1;

		wx1 = iw->x1 << ELE_SHIFT;
		wz1 = iw->z1 << ELE_SHIFT;
		wx2 = iw->x2 << ELE_SHIFT;
		wz2 = iw->z2 << ELE_SHIFT;

		//
		// No diagonal walls nowadays.
		//

		ASSERT(dx == 0 || dz == 0);

		if (dx == 0)
		{
			if (WITHIN(x2, wx1 - radius + 1, wx1 + radius - 1))
			{
				min = wz1;
				max = wz2;

				if (max < min) {SWAP(min, max);}

				min -= radius - 1;
				max += radius - 1;

				if (WITHIN(z2, min, max))
				{
					//
					// Make sure the doors are sorted.
					//

					ASSERT(iw->door[0] <= iw->door[1]);
					ASSERT(iw->door[1] <= iw->door[2]);

					//
					// Find each of the sausages along this wall.
					//

					start = wz1;
					end   = wz2;

					for (j = 0; j < 3; j++)
					{
						if (iw->door[j] == 255)
						{
							//
							// No more doors.
							//

							break;
						}

						//
						// The z-range of the doorway.
						//

						if (dz > 0)
						{
							doorway_start = wz1 + (iw->door[j] << ELE_SHIFT) + 0x30;
							doorway_end   = wz1 + (iw->door[j] << ELE_SHIFT) + 0xd0;
						}
						else
						{
							doorway_start = wz1 - (iw->door[j] << ELE_SHIFT) - 0x30;
							doorway_end   = wz1 - (iw->door[j] << ELE_SHIFT) - 0xd0;
						}

						//
						// Collide against a sausage.
						//

						if (collide_against_sausage(
								wx1, start,
								wx1, doorway_start,
								radius,
								x1, z1,
								x2, z2,
								slide_x,
								slide_z))
						{
							collided = TRUE;

							x2 = *slide_x;
							z2 = *slide_z;

							//
							// You can only ever collide with one sausage from each wall.
							//

							goto try_the_next_wall;
						}

						start = doorway_end;
					}

					if (collide_against_sausage(
							wx1, start,
							wx1, end,
							radius,
							x1, z1,
							x2, z2,
							slide_x,
							slide_z))
					{
						collided = TRUE;

						x2 = *slide_x;
						z2 = *slide_z;

						//
						// You can only ever collide with one sausage from each wall.
						//

						goto try_the_next_wall;
					}
				}
			}
		}
		else
		{
			ASSERT(dz == 0);

			if (WITHIN(z2, wz1 - radius + 1, wz1 + radius - 1))
			{
				min = wx1;
				max = wx2;

				if (max < min) {SWAP(min, max);}

				min -= radius - 1;
				max += radius - 1;

				if (WITHIN(x2, min, max))
				{
					//
					// Make sure the doors are sorted.
					//

					ASSERT(iw->door[0] <= iw->door[1]);
					ASSERT(iw->door[1] <= iw->door[2]);

					//
					// Find each of the sausages along this wall.
					//

					start = wx1;
					end   = wx2;

					for (j = 0; j < 3; j++)
					{
						if (iw->door[j] == 255)
						{
							//
							// No more doors.
							//

							break;
						}

						//
						// The z-range of the doorway.
						//

						if (dx > 0)
						{
							doorway_start = wx1 + (iw->door[j] << ELE_SHIFT) + 0x30;
							doorway_end   = wx1 + (iw->door[j] << ELE_SHIFT) + 0xd0;
						}
						else
						{
							doorway_start = wx1 - (iw->door[j] << ELE_SHIFT) - 0x30;
							doorway_end   = wx1 - (iw->door[j] << ELE_SHIFT) - 0xd0;
						}

						//
						// Collide against a sausage.
						//

						if (collide_against_sausage(
								start,         wz1,
								doorway_start, wz1,
								radius,
								x1, z1,
								x2, z2,
								slide_x,
								slide_z))
						{
							collided = TRUE;

							x2 = *slide_x;
							z2 = *slide_z;

							//
							// You can only ever collide with one sausage from each wall.
							//

							goto try_the_next_wall;
						}

						start = doorway_end;
					}

					if (collide_against_sausage(
							start, wz1,
							end,   wz1,
							radius,
							x1, z1,
							x2, z2,
							slide_x,
							slide_z))
					{
						collided = TRUE;

						x2 = *slide_x;
						z2 = *slide_z;

						//
						// You can only ever collide with one sausage from each wall.
						//

						goto try_the_next_wall;
					}
				}
			}
		}

	  try_the_next_wall:;
	}

	return collided;
}



SLONG ID_collide_2d_old(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG *slide_x,
		SLONG *slide_z)
{
	SLONG i;
	SLONG j;

	SLONG dx;
	SLONG dz;

	SLONG wx1;
	SLONG wz1;
	SLONG wx2;
	SLONG wz2;

	SLONG dx1;
	SLONG dz1;
	SLONG dx2;
	SLONG dz2;

	SLONG sx;
	SLONG sz;

	SLONG pushx;
	SLONG pushz;

	SLONG dprod;
	SLONG length;
	SLONG collided;
	SLONG through;

	SLONG bot;
	SLONG top;

	ID_Wall *iw;

	//
	// Collide with all the walls.
	//

	collided = FALSE;

	for (i = 0; i < ID_wall_upto; i++)
	{
		iw = &ID_wall[i];

		dx = iw->x2 - iw->x1;
		dz = iw->z2 - iw->z1;

		wx1 = iw->x1 << ELE_SHIFT;
		wz1 = iw->z1 << ELE_SHIFT;
		wx2 = iw->x2 << ELE_SHIFT;
		wz2 = iw->z2 << ELE_SHIFT;

		if (dx && dz)
		{
			//
			// This must be a wonky outside wall. Do the two line segments
			// intersect?
			//

			if (MATHS_seg_intersect(wx1, wz1, wx2, wz2, x1, z1, x2, z2))
			{
				//
				// The lines intersect. Slide along.
				//

				dx1 = wx2 - wx1;
				dz1 = wz2 - wz1;
				
				dx2 = x2 - x1;
				dz2 = z2 - z1;
				
				dprod  = dx1*dx2 + dz1*dz2;
				length = QDIST2(abs(dx1), abs(dz1));

				sx = dx1 * dprod / length;
				sz = dz1 * dprod / length;

				sx >>= 8;
				sz >>= 8;

			   *slide_x = x1 + sx;
			   *slide_z = z1 + sz;

				x2 = *slide_x;
				z2 = *slide_z;

				collided = TRUE;

				//
				// Go through all the walls again!
				//

				i = -1;
			}
		}
		else
		{
			//
			// This is easier because the wall is orthogonal.
			//

			if (dx == 0)
			{
				through = FALSE;

				//
				// Collide with the wall if we start and end on
				// either side of it or we end on it.
				//

				if (x2 == wx1)
				{
					if (x1 != wx1)
					{
						//
						// Starting and ending on the wall doesn't count.
						//

						through = TRUE;
					}
				}
				else
				if ((x1 < wx1 && x2 > wx1) ||
					(x1 > wx1 && x2 < wx1))
				{
					through = TRUE;
				}

				if (through)
				{
					//
					// The line crosses the line defined the wall. Does it cross
					// at the wall? We can use this approximation if we assume that
					// the line segments we deal with are smallish.
					//

					//
					// The top and bottom of the wall.
					//

					top = wz1;
					bot = wz2;

					if (top < bot) {SWAP(top, bot);}

					//
					// Don't bother working out the z position of the line segment
					// at wx1.
					//

					if (WITHIN(z1, bot, top) || WITHIN(z2, bot, top))
					{
						//
						// Check for doors.
						//

						for (j = 0; j < 3; j++)
						{
							if (iw->door[j] != 255)
							{
								bot   = iw->z1;
								bot  += iw->door[j] * SIGN(dz);
								bot <<= ELE_SHIFT;
								top   = bot;
								
								bot  += 0x30 * SIGN(dz);
								top  += 0xd0 * SIGN(dz);

								if (top < bot) {SWAP(top, bot);}

								if (WITHIN(z1, bot, top) && WITHIN(z2, bot, top))
								{
									//
									// We have walked through a doorframe.
									//

									goto passed_through_door;
								}
							}
						}

						//
						// This is an intersection.. take out the x component from
						// the movement vector.
						//

					   *slide_x = x1;
					   *slide_z = z2;

					   *slide_x += (x2 < x1) ? 0x1f : -0x1f;

					    x2 = *slide_x;
						z2 = *slide_z;

						collided = TRUE;

						//
						// Go through all the walls again!
						//

						i = -1;
					}
				}
			}
			else
			{
				ASSERT(dz == 0);

				through = FALSE;

				//
				// Collide with the wall if we start and end on
				// either side of it or we end on it.
				//

				if (z2 == wz1)
				{
					if (z1 != wz1)
					{
						//
						// Starting and ending on the wall doesn't count.
						//

						through = TRUE;
					}
				}
				else
				if ((z1 < wz1 && z2 > wz1) ||
					(z1 > wz1 && z2 < wz1))
				{
					through = TRUE;
				}

				if (through)
				{
					//
					// The line crosses the line defined the wall. Does it cross
					// at the wall? We can use this approximation if we assume that
					// the line segments we deal with are smallish.
					//

					//
					// The top and bottom of the wall.
					//

					top = wx1;
					bot = wx2;

					if (top < bot) {SWAP(top, bot);}

					//
					// Don't bother working out the z position of the line segment
					// at wx1.
					//

					if (WITHIN(x1, bot, top) || WITHIN(x2, bot, top))
					{
						//
						// Check for doors.
						//

						for (j = 0; j < 3; j++)
						{
							if (iw->door[j] != 255)
							{
								bot   = iw->x1;
								bot  += iw->door[j] * SIGN(dx);
								bot <<= ELE_SHIFT;
								top   = bot;
								
								bot  += 0x30 * SIGN(dx);
								top  += 0xd0 * SIGN(dx);

								if (top < bot) {SWAP(top, bot);}

								if (WITHIN(x1, bot, top) && WITHIN(x2, bot, top))
								{
									//
									// We have walked through a doorframe.
									//

									goto passed_through_door;
								}
							}
						}

						//
						// This is an intersection.. take out the x component from
						// the movement vector.
						//

					   *slide_x = x2;
					   *slide_z = z1;

					   //*slide_z += (z2 < z1) ? 0xf : -0xf;

					    x2 = *slide_x;
						z2 = *slide_z;

						collided = TRUE;

						//
						// Go through all the walls again!
						//

						i = -1;
					}
				}
			}
		}

	  passed_through_door:;
	}

	return collided;
}



SLONG ID_editor_get_room_upto;
SLONG ID_editor_get_wall_upto;
SLONG ID_editor_get_stair_upto;

void ID_editor_start_get_rooms () {ID_editor_get_room_upto  = 1;}
void ID_editor_start_get_walls () {ID_editor_get_wall_upto  = 0;}
void ID_editor_start_get_stairs() {ID_editor_get_stair_upto = 0;}

//
// These functions return FALSE if there are no more rooms, walls
// or stairs, otherwise they fill out the given structure with
// info describing the next room, wall or staircase.
//

#define ID_MAX_ROOM_NAME 32

CBYTE ID_room_name[ID_MAX_ROOM_NAME];

SLONG ID_editor_get_room(ID_Roominfo *ans)
{
	SLONG room;

	if (ID_editor_get_room_upto >= ID_room_upto)
	{
		return FALSE;
	}
	else
	{
		ASSERT(WITHIN(ID_editor_get_room_upto, 1, ID_room_upto - 1));

		SLONG x;
		SLONG z;

		ID_Room *ir = &ID_room[ID_editor_get_room_upto];

		for (x = ir->x1; x <= ir->x2; x++)
		for (z = ir->z1; z <= ir->z2; z++)
		{
			room = ID_FLOOR(x, z)->room;

			if (room == ID_editor_get_room_upto)
			{
				ans->x = x;
				ans->z = z;

				switch(ir->type)
				{
					case ID_ROOM_TYPE_LOO:       ans->what = "Loo";          break;
					case ID_ROOM_TYPE_KITCHEN:   ans->what = "Kitchen";      break;
					case ID_ROOM_TYPE_LOUNGE:    ans->what = "Lounge";       break;
					case ID_ROOM_TYPE_LOBBY:     ans->what = "Lobby";        break;
					case ID_ROOM_TYPE_DINING:    ans->what = "Dining";       break;
					case ID_ROOM_TYPE_WAREHOUSE: ans->what = "Warehouse";    break;
					case ID_ROOM_TYPE_OFFICE:    ans->what = "Office";       break;
					case ID_ROOM_TYPE_MEETING:   ans->what = "Meeting room"; break;
					case ID_ROOM_TYPE_BEDROOM:   ans->what = "Bedroom";		 break;
					case ID_ROOM_TYPE_CORRIDOR:  ans->what = "Corridor";     break;

					default:
						ASSERT(0);
						break;
				}

				//
				// Get the next room next time.
				//

				ID_editor_get_room_upto += 1;

				if (ID_room[room].flat)
				{
					sprintf(ID_room_name, "%d:%s", ID_room[room].flat, ans->what);

					ans->what = ID_room_name;
				}

				return TRUE;
			}
		}

		//
		// A bit of error recovery...
		//

		ans->x    = ir->x1;
		ans->z    = ir->z1;
		ans->what = "ERROR";

		ID_editor_get_room_upto += 1;

		//
		// No room square in the bounding box of the room!
		//

		return TRUE;
	}
}

SLONG ID_editor_get_wall(ID_Wallinfo *ans)
{
	ID_Wall *iw;

	while(1)
	{
		if (ID_editor_get_wall_upto >= ID_wall_upto)
		{
			return FALSE;
		}
		else
		{
			ASSERT(WITHIN(ID_editor_get_wall_upto, 0, ID_wall_upto - 1));

			iw = &ID_wall[ID_editor_get_wall_upto++];

			if (iw->type == ID_WALL_T_OUTSIDE)
			{
				//
				// Skip outside walls.
				//
			}
			else
			{
				ans->x1 = iw->x1;
				ans->z1 = iw->z1;
				ans->x2 = iw->x2;
				ans->z2 = iw->z2;

				ans->door[0] = iw->door[0];
				ans->door[1] = iw->door[1];
				ans->door[2] = iw->door[2];
				ans->door[3] = 255;		// Never a fourth door...

				return TRUE;
			}
		}
	}
}

SLONG ID_editor_get_stair(ID_Stairinfo *ans)
{
	if (ID_editor_get_stair_upto >= ID_num_stairs)
	{
		return FALSE;
	}
	else
	{
		ASSERT(WITHIN(ID_editor_get_stair_upto, 0, ID_num_stairs - 1));

		ID_Stair *it = &ID_stair[ID_editor_get_stair_upto];

		ans->x1 = it->x1;
		ans->z1 = it->z1;
		ans->x2 = it->x2;
		ans->z2 = it->z2;

		//
		// Get the next staircase next time.
		//

		ID_editor_get_stair_upto += 1;

		return TRUE;
	}
}

ID_Finfo ID_get_finfo;

SLONG ID_get_num_furn()
{
	return ID_furn_upto;
}

ID_Finfo *ID_get_furn(SLONG number)
{
	ASSERT(WITHIN(number, 0, ID_furn_upto - 1));

	ID_get_finfo.x    = ID_furn[number].x;
	ID_get_finfo.y    = INDOORS_HEIGHT_FLOOR;
	ID_get_finfo.z    = ID_furn[number].z;
	ID_get_finfo.prim = ID_furn[number].prim;
	ID_get_finfo.yaw  = ID_furn[number].yaw << 3;

	return &ID_get_finfo;
}

#else
//
// no need to do psx version of this unused stuff, but lets get it compiling
//

SLONG ID_calc_height_at(SLONG x, SLONG z)
{
}

SLONG ID_get_num_furn()
{
	return	(0);
}

ID_Finfo *ID_get_furn(SLONG number)
{
	return(NULL);
}

SLONG ID_collide_2d(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG  radius,
		SLONG *slide_x,
		SLONG *slide_z)
{
	return(0);
}

void ID_this_is_where_i_am(SLONG x, SLONG z)
{
}

void ID_get_floorplan_bounding_box(
		SLONG *x1,
		SLONG *z1,
		SLONG *x2,
		SLONG *z2)
{
	*x1=0;
	*z1=0;
	*x2=0;
	*z2=0;
}

#endif
