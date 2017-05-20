//
// Road stuff...
//

#ifndef _ROAD_
#define _ROAD_



//
// The wander system for vehicles.
// 

typedef struct
{
	UBYTE	x;
	UBYTE	z;

	UBYTE	c[4];	// Other nodes connected to this one.

} ROAD_Node;

#define ROAD_MAX_NODES 256

extern	ROAD_Node *ROAD_node;
extern	SLONG      ROAD_node_upto;

//
// The indices of nodes that lead off (or onto) the map.
//

#define ROAD_MAX_EDGES 8

extern UBYTE *ROAD_edge;//[ROAD_MAX_EDGES];
extern UWORD ROAD_edge_upto;

//
// The distance from the middle of the road that you want to drive along.
// 

#define ROAD_LANE (0x100)


//
// Creates curbs and cambers.
//

void ROAD_sink(void);




//
// Returns TRUE if the given square is part of a road.
// Returns TRUE if the given square is on a zebra crossing.
// 

SLONG ROAD_is_road (SLONG map_x, SLONG map_z);
SLONG ROAD_is_zebra(SLONG map_x, SLONG map_z);


//
// Calculates the type of each mqpsquare texture on the map. This
// is really slow!
//

void ROAD_calc_mapsquare_type(void);


//
// Returns the type of each mapsquare.
// 

#define ROAD_TYPE_TARMAC   0
#define ROAD_TYPE_GRASS    1
#define ROAD_TYPE_DIRT     2
#define ROAD_TYPE_SLIPPERY 3

SLONG ROAD_get_mapsquare_type(SLONG map_x, SLONG map_z);

//
// Calculates the nodes of the vehicle wandering system.
//

void ROAD_wander_calc(void);


//
// Returns the road nearest to the given point.
//

void ROAD_find(
		SLONG world_x,
		SLONG world_z,

		SLONG *n1,
		SLONG *n2);

//
// Returns the position of the given node.
//

void ROAD_node_pos(
		SLONG  node,
		SLONG *world_x,
		SLONG *world_z);

//
// Returns the degree (number of connections) to a given node
//

SLONG ROAD_node_degree(SLONG node);

//
// Returns the nearer of rn1 and rn2, and the distance squared
//

#define	JN_RADIUS_IN	0x300
#define JN_RADIUS_OUT	0x500
#define AT_JUNCTION		(JN_RADIUS_IN * JN_RADIUS_IN)		// square of distance from car to junction to count as being at / on the junction
#define NEAR_JUNCTION	(JN_RADIUS_OUT * JN_RADIUS_OUT)		// square of distance from car to junction to count as approaching the junction

SLONG ROAD_nearest_node(SLONG rn1, SLONG rn2, SLONG wx, SLONG wz, SLONG* nnd);

//
// Returns another road to drive down after reaching node n2.
//

void ROAD_whereto_now(
		SLONG  n1,
		SLONG  n2,
		SLONG *wtn1,
		SLONG *wtn2);

//
// Returns the destination for a length of road.
// 

void ROAD_get_dest(
		SLONG  n1,
		SLONG  n2,
		SLONG *world_x,
		SLONG *world_z);

//
// Returns TRUE if the given node is the end of the line! i.e. it
// leads you off the edge of the map.
//

SLONG ROAD_is_end_of_the_line(SLONG n);

//
// Returns <0 for a left-hand bend, >0 for a right-hand bend and 0 for no bend (or a u-turn)
//

SLONG ROAD_bend(SLONG n1, SLONG n2, SLONG n3);

//
// If you've driven off the map and you want to reappear in a new place
// call this function!  Fill in (*world_x,*world_z) with your current
// position (so you won't get anywhere too nearby),
//

void ROAD_find_me_somewhere_to_appear(
		SLONG *world_x,		// Current position on calling, new position on return.
		SLONG *world_z,
		SLONG *nrn1,		// The new road you are on.
		SLONG *nrn2,
		SLONG *ryaw);		// The new yaw you should be at.

//
// Signed distance from the middle of road.  A negative distance
// means that you are on the wrong side of the road.
//

SLONG ROAD_signed_dist(
		SLONG n1,	// Going from
		SLONG n2,	// Going to
		SLONG world_x,
		SLONG world_z);

//
// Draws a debug display of the road system.
//

void ROAD_debug(void);


#endif
