//
// Road stuff...
//

#include "game.h"
#include "road.h"
#include "pap.h"
#include "mav.h"
#include "elev.h"
#ifndef PSX
#include "texture.h"
#endif

#include "memory.h"

extern	SLONG	build_psx;
//
// The width of the roads.
//

#define ROAD_WIDTH 5


//
// The wander system for vehicles.
// 

ROAD_Node *ROAD_node;
SLONG      ROAD_node_upto;

//
// The nodes at the edge of the map.
//

UBYTE *ROAD_edge;//[ROAD_MAX_EDGES];
UWORD ROAD_edge_upto;


//
// Returns the position of a node.
//

void ROAD_node_pos(
		SLONG  node,
		SLONG *node_x,
		SLONG *node_z)
{
	ROAD_Node *rn;

	ASSERT(WITHIN(node, 1, ROAD_node_upto - 1));

	rn = &ROAD_node[node];

   *node_x = (rn->x << 8) + 0x80;
   *node_z = (rn->z << 8) + 0x80;
}

// the bendiness of a junction n1 -> n2 -> n3

SLONG ROAD_bend(SLONG n1, SLONG n2, SLONG n3)
{
	ASSERT(WITHIN(n1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n2, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n3, 1, ROAD_node_upto - 1));

	ROAD_Node*	rn1 = &ROAD_node[n1];
	ROAD_Node*	rn2 = &ROAD_node[n2];
	ROAD_Node*	rn3 = &ROAD_node[n3];

	SLONG	x12 = rn2->x - rn1->x;
	SLONG	z12 = rn2->z - rn1->z;

	SLONG	x23 = rn3->x - rn2->x;
	SLONG	z23 = rn3->z - rn2->z;

	return x12 * z23 - z12 * x23;
}

// the degree of a road node

SLONG ROAD_node_degree(SLONG node)
{
	ASSERT(WITHIN(node, 1, ROAD_node_upto - 1));

	ROAD_Node* rn = &ROAD_node[node];

	SLONG	degree = 0;

	if (rn->c[0])	degree++;
	if (rn->c[1])	degree++;
	if (rn->c[2])	degree++;
	if (rn->c[3])	degree++;

	return degree;
}

// ROAD_nearest_node
//
// return the nearer of rn1 and rn2 in nn, and the distance squared in nnd; wx,wz = WorldPos >> 8

SLONG ROAD_nearest_node(SLONG rn1, SLONG rn2, SLONG wx, SLONG wz, SLONG* nnd)
{
	ASSERT(WITHIN(rn1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(rn2, 1, ROAD_node_upto - 1));

	ROAD_Node*	prn1 = &ROAD_node[rn1];
	ROAD_Node*	prn2 = &ROAD_node[rn2];

	SLONG x1 = (prn1->x << 8) + 0x80;
 	SLONG z1 = (prn1->z << 8) + 0x80;
	SLONG x2 = (prn2->x << 8) + 0x80;
 	SLONG z2 = (prn2->z << 8) + 0x80;

	SLONG	d1 = (wx - x1)*(wx - x1) + (wz - z1)*(wz - z1);
	SLONG	d2 = (wx - x2)*(wx - x2) + (wz - z2)*(wz - z2);
	
	if (d1 < d2)
	{
		*nnd = d1;
		return rn1;
	}

	*nnd = d2;
	return rn2;
}


#if !defined(PSX) && !defined(TARGET_DC)
//
// Creates curbs and cambers.
//

void ROAD_sink()
{
	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;
	
	SLONG dist;
	SLONG best_dist;

	SLONG page;
	
	PAP_Hi *ph;
	MapElement *me;

	for (mx = 0; mx < MAP_WIDTH  - 1; mx++)
	for (mz = 0; mz < MAP_HEIGHT - 1; mz++)
	{
		if (ROAD_is_road(mx,mz))
		{
			//
			// This is a road texture...
			//

			PAP_2HI(mx+0,mz+0).Flags |= PAP_FLAG_SINK_SQUARE;

			PAP_2HI(mx+0,mz+0).Flags |= PAP_FLAG_SINK_POINT;
			PAP_2HI(mx+1,mz+0).Flags |= PAP_FLAG_SINK_POINT;
			PAP_2HI(mx+0,mz+1).Flags |= PAP_FLAG_SINK_POINT;
			PAP_2HI(mx+1,mz+1).Flags |= PAP_FLAG_SINK_POINT;
		}
	}

	//
	// Which points don't need to be transformed on the upper level?
	//

	for (mx = 0; mx < MAP_WIDTH;  mx++)
	for (mz = 0; mz < MAP_HEIGHT; mz++)
	{
		PAP_2HI(mx,mz).Flags |= PAP_FLAG_NOUPPER;
	}

	for (mx = 0; mx < MAP_WIDTH  - 1; mx++)
	for (mz = 0; mz < MAP_HEIGHT - 1; mz++)
	{
		if (PAP_2HI(mx,mz).Flags & PAP_FLAG_SINK_SQUARE)
		{
		}
		else
		{
			//
			// We need the upper level.
			//

			PAP_2HI(mx + 0, mz + 0).Flags &= ~PAP_FLAG_NOUPPER;
			PAP_2HI(mx + 1, mz + 0).Flags &= ~PAP_FLAG_NOUPPER;
			PAP_2HI(mx + 0, mz + 1).Flags &= ~PAP_FLAG_NOUPPER;
			PAP_2HI(mx + 1, mz + 1).Flags &= ~PAP_FLAG_NOUPPER;
		}
	}

	//
	// Give the roads a camber.
	//

	for (mx = 0; mx < MAP_WIDTH  - 1; mx++)
	for (mz = 0; mz < MAP_HEIGHT - 1; mz++)
	{
		ph = &PAP_2HI(mx,mz);

		if (PAP_2HI(mx,mz).Flags & PAP_FLAG_SINK_POINT)
		{
			//
			// How close is this point to the edge of a road?
			//

			best_dist = 2;

			for (dx = -1; dx <= +1; dx++)
			for (dz = -1; dz <= +1; dz++)
			{
				if (WITHIN(mx + dx, 0, MAP_WIDTH  - 1) &&
					WITHIN(mz + dz, 0, MAP_HEIGHT - 1))

				if (PAP_on_map_hi(mx + dx, mz + dz))
				{
					if (PAP_2HI(mx + dx, mz + dz).Flags & PAP_FLAG_NOUPPER)
					{
						//
						// This is not the edge of a road.
						//
					}
					else
					{
						dist = MAX(abs(dx),abs(dz));

						if (dist < best_dist)
						{
							best_dist = dist;
						}
					}
				}
			}

			if (best_dist == 2) {ph->Alt += 3;}
			if (best_dist == 1) {ph->Alt += 2;}
		}
	}
}
#endif

SLONG ROAD_is_road(SLONG map_x, SLONG map_z)
{
	PAP_Hi *ph;
	SLONG   num;

	if (!WITHIN(map_x, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(map_z, 0, PAP_SIZE_HI - 1))
	{
		return FALSE;
	}

	ph = &PAP_2HI(map_x,map_z);

	if (ph->Flags & PAP_FLAG_HIDDEN)
	{
		return FALSE;
	}

	num = ph->Texture & 0x3ff;

	extern SLONG TEXTURE_set;
#ifdef	PSX
	if (WITHIN(num, 256, 256+22))
//	if (WITHIN(num, 256, 306))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else
	if (build_psx)
	{
		if(WITHIN(num, 256, 256+22))
			return(TRUE);
	}
	else
	{

		if (WITHIN(num, 323, 356) || ((TEXTURE_set == 7 || TEXTURE_set == 8) && (num == 35 || num == 36 || num == 39)))
		{
			return TRUE;
		}
	}

	return FALSE;
#endif
}

SLONG ROAD_is_zebra(SLONG map_x, SLONG map_z)
{
	PAP_Hi *ph;
	SLONG   num;

	if (!WITHIN(map_x, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(map_z, 0, PAP_SIZE_HI - 1))
	{
		return FALSE;
	}

	ph = &PAP_2HI(map_x,map_z);

	num = ph->Texture & 0x3ff;

	if (num == 333 || num == 334)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



//
// Returns the index of a node at (x,z).  Create one if it has to.
//

SLONG ROAD_find_node(SLONG x, SLONG z)
{
	SLONG i;

	//
	// Look for a node that already exists.
	//

	for (i = 1; i < ROAD_node_upto; i++)
	{
		if (ROAD_node[i].x == x &&
			ROAD_node[i].z == z)
		{
			return i;
		}
	}

	//
	// We must create a new node.
	//

	ASSERT(WITHIN(ROAD_node_upto, 1, ROAD_MAX_NODES - 1));

	ROAD_node[ROAD_node_upto].x = x;
	ROAD_node[ROAD_node_upto].z = z;

	return ROAD_node_upto++;
}

//
// Connects the two nodes together.
//

#if !defined(PSX) && !defined(TARGET_DC)

void ROAD_connect(SLONG n1, SLONG n2)
{
	SLONG i;

	ROAD_Node *rn1;
	ROAD_Node *rn2;

	ASSERT(WITHIN(n1, 1, ROAD_MAX_NODES - 1));
	ASSERT(WITHIN(n2, 1, ROAD_MAX_NODES - 1));

	rn1 = &ROAD_node[n1];
	rn2 = &ROAD_node[n2];

	for (i = 0; i < 4; i++)
	{
		if (rn1->c[i] == n2)
		{
			//
			// Already connected
			//

			goto connected_2_to_1;
		}

		if (rn1->c[i] == 0)
		{
			//
			// Found a spare slot.
			//

			rn1->c[i] = n2;

			goto connected_2_to_1;
		}
	}

	//
	// No spare slots!
	//

	ASSERT(0);

  connected_2_to_1:;

	for (i = 0; i < 4; i++)
	{
		if (rn2->c[i] == n1)
		{
			//
			// Already connected
			//

			return;
		}

		if (rn2->c[i] == 0)
		{
			//
			// Found a spare slot.
			//

			rn2->c[i] = n1;

			return;
		}
	}

	//
	// No spare slots!
	//

	ASSERT(0);
}

//
// Disconects the two nodes.
// 

void ROAD_disconnect(SLONG n1, SLONG n2)
{
	SLONG i;
	SLONG j;

	ASSERT(WITHIN(n1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n2, 1, ROAD_node_upto - 1));

	ROAD_Node *rn1 = &ROAD_node[n1];
	ROAD_Node *rn2 = &ROAD_node[n2];

	for (i = 0; i < 4; i++)
	{
		if (rn1->c[i] == n2)
		{
			for (j = i; j < 3; j++)
			{
				rn1->c[j] = rn1->c[j + 1];
			}

			rn1->c[3] = 0;

			goto found_2_from_1;
		}
	}

	//
	// These two nodes aren't connected!
	//

	ASSERT(0);

  found_2_from_1:;

	for (i = 0; i < 4; i++)
	{
		if (rn2->c[i] == n1)
		{
			for (j = i; j < 3; j++)
			{
				rn2->c[j] = rn2->c[j + 1];
			}

			rn2->c[3] = 0;

			goto found_1_from_2;
		}
	}

	//
	// These two nodes aren't connected!
	//

	ASSERT(0);

  found_1_from_2:;

	return;
}


//
// Fills in the indices of a road that intersects a road from (x1,z1) to (x2,z2).
// This function returns TRUE if it found an intersecting road or FALSE if it didn't.
// 

SLONG ROAD_intersect(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,

		SLONG *in1,
		SLONG *in2,
		SLONG *ix,	// Intersection point
		SLONG *iz)
{
	SLONG i;
	SLONG j;

	ROAD_Node *rn;
	ROAD_Node *rm;

	SLONG minx;
	SLONG maxx;

	SLONG minz;
	SLONG maxz;

	for (i = 1; i < ROAD_node_upto; i++)
	{
		rn = &ROAD_node[i];

		//
		// Two roads don't count as intersecting if they share an end point.
		//

		if ((x1 == rn->x && z1 == rn->z) ||
			(x2 == rn->x && z2 == rn->z))
		{
			continue;
		}

		for (j = 0; j < 4; j++)
		{
			if (rn->c[j] == 0)
			{
				break;
			}

			if (rn->c[j] < i)
			{
				rm = &ROAD_node[rn->c[j]];

				//
				// Two roads don't count as intersecting if they share an end point.
				//

				if ((x1 == rm->x && z1 == rm->z) ||
					(x2 == rm->x && z2 == rm->z))
				{
					continue;
				}

				if (x1 == x2)
				{
					if (rn->x == rm->x)
					{
						//
						// These roads are parallel so they can't intersect.
						//
					}
					else
					{
						minx = rn->x;
						maxx = rm->x;

						if (minx > maxx) {SWAP(minx,maxx);}

						if (WITHIN(x1, minx, maxx))
						{
							minz = z1;
							maxz = z2;

							if (minz > maxz) {SWAP(minz,maxz);}
							
							if (WITHIN(rn->z, minz, maxz))
							{
								//
								// These roads intersect.
								//

							   *in1 = i;
							   *in2 = rn->c[j];
							   *ix  = x1;
							   *iz  = rn->z;

								return TRUE;
							}
						}
					}
				}
				else
				{
					ASSERT(z1 == z2);

					if (rn->z == rm->z)
					{
						//
						// These roads are parallel so they can't intersect.
						//
					}
					else
					{
						minz = rn->z;
						maxz = rm->z;

						if (minz > maxz) {SWAP(minz,maxz);}

						if (WITHIN(z1, minz, maxz))
						{
							minx = x1;
							maxx = x2;

							if (minx > maxx) {SWAP(minx,maxx);}
							
							if (WITHIN(rn->x, minx, maxx))
							{
								//
								// These roads intersect.
								//

							   *in1 = i;
							   *in2 = rn->c[j];
							   *ix  = rn->x;
							   *iz  = z1;

								return TRUE;
							}
						}
					}
				}
			}
		}
	}

	//
	// Found no intersecting roads.
	//

	return FALSE;
}



//
// Inserts an extra point in the given road.
//

void ROAD_split(SLONG n1, SLONG n2, SLONG splitx, SLONG splitz)
{
	SLONG sn = ROAD_find_node(splitx, splitz);

	if (sn == n1 || sn == n2)
	{
		//
		// Splitting a road at one of its end-points does nothing.
		//

		return;
	}

	ASSERT(WITHIN(n1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n2, 1, ROAD_node_upto - 1));

	ROAD_Node *rn1 = &ROAD_node[n1];
	ROAD_Node *rn2 = &ROAD_node[n2];

	//
	// Make sure this split point lies on the road and it in the
	// middle of it.
	//

	#ifndef NDEBUG

	if (rn1->x == rn2->x)
	{
		SLONG minz;
		SLONG maxz;

		ASSERT(splitx == rn1->x);
		
		minz = rn1->z;
		maxz = rn2->z;

		if (minz > maxz) {SWAP(minz,maxz);}

		ASSERT(WITHIN(splitz, minz + 1, maxz - 1));
	}
	else
	{
		ASSERT(rn1->z == rn2->z);

		SLONG minx;
		SLONG maxx;

		ASSERT(splitz == rn1->z);
		
		minx = rn1->x;
		maxx = rn2->x;

		if (minx > maxx) {SWAP(minx,maxx);}

		ASSERT(WITHIN(splitx, minx + 1, maxx - 1));
	}

	#endif

	//
	// Disconnect the two road nodes.
	//

	ROAD_disconnect(n1,n2);

	//
	// Connect the two new roads.
	//

	ROAD_connect(n1, sn);
	ROAD_connect(n2, sn);
}


//
// Adds a new road.
//

void ROAD_add(SLONG x1, SLONG z1, SLONG x2, SLONG z2)
{
	SLONG n1;
	SLONG n2;

	SLONG in1;
	SLONG in2;

	SLONG ix;
	SLONG iz;

	if (ROAD_node_upto >= ROAD_MAX_NODES - 4)
	{
		//
		// There aren't enough road nodes left for this to be a safe operation.
		//

		return;
	}

	//
	// Don't add any silly roads.
	//

	if (x1 == x2 && z1 == z2)
	{
		return;
	}

	if ((x1 == 121 && z1 == 33) ||
		(x2 == 121 && z2 == 33))
	{
		//
		// Don't add this road if we are on the "Cop Killers" or 
		// "Arms Breaker" mission.
		//

		if (strstr(ELEV_fname_map, "gpost3.iam"))
		{
			return;
		}
	}

	//
	// The nodes at our start and end.
	//

	n1 = ROAD_find_node(x1,z1);
	n2 = ROAD_find_node(x2,z2);

	//
	// We must create nodes at the intersection point of this road
	// with any other one.
	//

	if (!ROAD_intersect(x1,z1, x2,z2, &in1, &in2, &ix, &iz))
	{
		//
		// We are done.
		//

		ROAD_connect(n1,n2);
	}
	else
	{
		//
		// Split up the road we intersect.
		//

		ROAD_split(in1, in2, ix, iz);

		//
		// Split ourselves up and insert each bit.
		//

		ROAD_add(x1, z1, ix, iz);
		ROAD_add(x2, z2, ix, iz);
	}
}

#endif


//
// Returns TRUE if the given square lies along the middle of a road.
//

SLONG ROAD_is_middle(SLONG map_x, SLONG map_z)
{
	SLONG dx;
	SLONG dz;
	
	SLONG mx;
	SLONG mz;

	if (!WITHIN(map_x, 2, PAP_SIZE_HI - 3) ||
		!WITHIN(map_z, 2, PAP_SIZE_HI - 3))
	{
		return FALSE;
	}

	//
	// Do some quick rejection first.
	//

	if (!ROAD_is_road(map_x,map_z))
	{
		return FALSE;
	}

	if (!ROAD_is_road(map_x + 2, map_z + 2))
	{
		return FALSE;
	}

	for (dx = -2; dx <= +2; dx++)
	for (dz = -2; dz <= +2; dz++)
	{
		mx = map_x + dx;
		mz = map_z + dz;

		if (!ROAD_is_road(mx,mz))
		{
			return FALSE;
		}
	}

	return TRUE;
}


SLONG ROAD_is_end_of_the_line(SLONG n)
{
	ROAD_Node *rn;

	ASSERT(WITHIN(n, 1, ROAD_node_upto - 1));

	rn = &ROAD_node[n];

	if (rn->c[0] && rn->c[1] == NULL)
	{
		//
		// This node is only connected to one other. It is a canditate for
		// leading of the edge of the map.
		//

		if (rn->x == 0               ||
		    rn->z == 0               ||
			rn->x == PAP_SIZE_HI - 1 ||
			rn->z == PAP_SIZE_HI - 1)
		{
			return TRUE;
		}
	}

	return FALSE;
}


#if !defined(PSX) && !defined(TARGET_DC)
void ROAD_wander_calc()
{
	SLONG x;
	SLONG z;

	SLONG i;
	SLONG onedge;

	SLONG p1;
	SLONG p2;
	SLONG p1valid;
	SLONG p2valid;

	ROAD_Node *rn;

	//
	// Clear all road info.
	//

	ROAD_node_upto = 1;

	memset(ROAD_node, 0, sizeof(ROAD_Node) * ROAD_MAX_NODES);

	ROAD_edge_upto = 0;

	memset(ROAD_edge, 0, sizeof(UBYTE) * ROAD_MAX_EDGES);

	//
	// Find all roads parallel to the z-axis.
	//

	for (x = 2; x < PAP_SIZE_HI - 2; x++)
	{
		p1valid = FALSE;
		p2valid = FALSE;

		for (z = 1; z < PAP_SIZE_HI - 1; z++)
		{
			if (ROAD_is_middle(x,z))
			{
				if (!p1valid)
				{
					p1valid = TRUE;
					p1      = z;
				}
				else
				{
					p2valid = TRUE;
					p2      = z;
				}
			}
			else
			{
				if (p1valid && p2valid)
				{
					//
					// Create a new road if it long enough.
					//

					if (abs(p2 - p1) > 2)
					{
						ROAD_add(x,p1, x,p2);
					}
				}

				p1valid = FALSE;
				p2valid = FALSE;
			}
		}
	}

	//
	// Find all roads parallel to the x-axis.
	//

	for (z = 2; z < PAP_SIZE_HI - 2; z++)
	{
		p1valid = FALSE;
		p2valid = FALSE;

		for (x = 1; x < PAP_SIZE_HI - 1; x++)
		{
			if (ROAD_is_middle(x,z))
			{
				if (!p1valid)
				{
					p1valid = TRUE;
					p1      = x;
				}
				else
				{
					p2valid = TRUE;
					p2      = x;
				}
			}
			else
			{
				if (p1valid && p2valid)
				{
					//
					// Create a new road if it is long enough.
					//

					if (abs(p2 - p1) > 2)
					{
						ROAD_add(p1,z, p2,z);
					}
				}

				p1valid = FALSE;
				p2valid = FALSE;
			}
		}
	}

	//
	// Find all nodes that lead off the map and put them right at
	// the edge of the map.  Make a note of them all somewhere.
	//

	for (i = 1; i < ROAD_node_upto; i++)
	{
		rn = &ROAD_node[i];

		if (rn->c[0] && rn->c[1] == NULL)
		{
			onedge = FALSE;

			//
			// This node is only connected to one other. It is a canditate for
			// leading of the edge of the map.
			//

			if (rn->x == 2) {rn->x = 0; onedge = TRUE;}
			if (rn->z == 2) {rn->z = 0; onedge = TRUE;}

			if (rn->x == PAP_SIZE_HI - 3) {rn->x = PAP_SIZE_HI - 1; onedge = TRUE;}
			if (rn->z == PAP_SIZE_HI - 3) {rn->z = PAP_SIZE_HI - 1; onedge = TRUE;}

			if (onedge)
			{
				if (ROAD_edge_upto < ROAD_MAX_EDGES)
				{
					ROAD_edge[ROAD_edge_upto++] = i;
				}

				#ifndef NDEBUG

				{
					ROAD_Node *rn1 =  rn;
					ROAD_Node *rn2 = &ROAD_node[rn->c[0]];

					ASSERT(
						rn1->x == rn2->x ||
						rn1->z == rn2->z);
				}

				#endif
			}
		}
	}
}
#endif

void ROAD_find_me_somewhere_to_appear(
		SLONG *world_x,		// Current position on calling, new position on return.
		SLONG *world_z,
		SLONG *nrn1,		// The new road you are on.
		SLONG *nrn2,
		SLONG *nyaw)
{
	UBYTE i;
	UBYTE e;
	SLONG dx;
	SLONG dz;

	ROAD_Node *rn;

   *nrn1 = 0;
   *nrn2 = 0;

	for (i = 0; i < ROAD_edge_upto; i++)
	{
		ASSERT(WITHIN(ROAD_edge[i],                 1, ROAD_node_upto - 1));
		ASSERT(WITHIN(ROAD_node[ROAD_edge[i]].c[0], 1, ROAD_node_upto - 1));

		ROAD_Node *rn1 = &ROAD_node[ROAD_edge[i]];
		ROAD_Node *rn2 = &ROAD_node[rn1->c[0]];

		ASSERT(
			rn1->x == rn2->x ||
			rn1->z == rn2->z);
	}

	if (ROAD_edge_upto)
	{
		//
		// A bit of random number coding!
		// 

		for (i = 0; i < 32; i++)
		{
			e = Random() & (ROAD_MAX_EDGES - 1);

			if (e >= ROAD_edge_upto)
			{
				continue;
			}

			ASSERT(WITHIN(ROAD_edge[e], 1, ROAD_node_upto - 1));

			rn = &ROAD_node[ROAD_edge[e]];

			dx = abs((rn->x << 8) - *world_x);
			dz = abs((rn->z << 8) - *world_z);

			if (dx + dz > 0x1000)
			{
				//
				// Found a good node- i.e. its far enough away.  Is there somebody
				// too close to the node?
				//

				if (THING_find_nearest(
						rn->x << 8,
						MAVHEIGHT(rn->x,rn->z) << 6,
						rn->z << 8,
						0x400,
						1 << CLASS_VEHICLE))
				{
					//
					// There is another vehicle too close to the node.
					//
				}
				else
				{

				   *nrn1 = ROAD_edge[e];
				   *nrn2 = rn->c[0];

					break;
				}
			}
		}

		if (*nrn1 == 0)
		{
#if !defined(PSX) && !defined(TARGET_DC)
			CONSOLE_text("Road node alert!");
#endif
			//
			// Oh dear! Use the first road edge.
			//

			ASSERT(WITHIN(ROAD_edge[0], 1, ROAD_node_upto - 1));

			rn = &ROAD_node[ROAD_edge[0]];

		   *nrn1 = ROAD_edge[0];
		   *nrn2 = rn->c[0];
		}

		//
		// Position the vehicle for being on the road... badly for now!
		// 

	   *world_x  = rn->x << 8;
	   *world_z  = rn->z << 8;

	   *world_x += 0x80;
	   *world_z += 0x80;

		//
		// Our new yaw.
		//

		dx = ROAD_node[*nrn2].x - ROAD_node[*nrn1].x;
		dz = ROAD_node[*nrn2].z - ROAD_node[*nrn1].z;

	   *nyaw  = -Arctan(dx,dz);
	   *nyaw &=  2047;

	   *world_x -= SIGN(dz) << 8;
	   *world_z += SIGN(dx) << 8;

		ASSERT(
			ROAD_node[*nrn1].x == ROAD_node[*nrn2].x ||
			ROAD_node[*nrn1].z == ROAD_node[*nrn2].z);
	}
	else
	{
		//
		// Oh dear... what now?  Appear somewhere random or assert?
		//

		ASSERT(0);
	}
}



void ROAD_debug()
{
#if !defined(PSX) && !defined(TARGET_DC)

	SLONG i;
	SLONG j;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG mx;
	SLONG my;
	SLONG mz;

	ROAD_Node *rn;

	//
	// Draw all the nodes.
	//

	for (i = 1; i < ROAD_node_upto; i++)
	{
		rn = &ROAD_node[i];

		ROAD_node_pos(i, &nx, &nz);

		ny = PAP_calc_map_height_at(nx,nz);

		//
		// The node.
		//

		AENG_world_line(
			nx, ny, nz,
			32,
			0xffffff,
			nx, ny + 0x60, nz,
			0,
			0x000000,
			FALSE);
		
		for (j = 0; j < 4; j++)
		{	
			if (rn->c[j])
			{
				ROAD_node_pos(rn->c[j], &mx, &mz);

				my = PAP_calc_map_height_at(mx,mz);

				AENG_world_line_infinite(
					nx, ny + 0x10, nz,
					32,
					0x8888ff,
					mx, my + 0x10, mz,
					0,
					0x008800,
					FALSE);
			}
		}
	}
#endif
}



SLONG ROAD_signed_dist(
		SLONG n1,
		SLONG n2,
		SLONG world_x,
		SLONG world_z)
{
	SLONG dist = 0x10000;	// Very far away... but not unreasonable.

	ASSERT(WITHIN(n1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n2, 1, ROAD_node_upto - 1));

	ROAD_Node *rn1 = &ROAD_node[n1];
	ROAD_Node *rn2 = &ROAD_node[n2];

	if (rn1->x == rn2->x)
	{
		SLONG minz = rn1->z;
		SLONG maxz = rn2->z;

		if (minz > maxz) {SWAP(minz,maxz);}

		if (WITHIN(world_z >> 8, minz - 2, maxz + 2))
		{
			dist = world_x - ((rn1->x << 8) + 0x80);

			if (rn1->z < rn2->z)
			{
				dist = -dist;
			}
		}
	}
	else
	{
		ASSERT(rn1->z == rn2->z);

		SLONG minx = rn1->x;
		SLONG maxx = rn2->x;

		if (minx > maxx) {SWAP(minx,maxx);}

		if (WITHIN(world_x >> 8, minx - 2, maxx + 2))
		{
			dist = world_z - ((rn1->z << 8) + 0x80);

			if (rn1->x > rn2->x)
			{
				dist = -dist;
			}
		}
	}

	return dist;
}



void ROAD_find(
		SLONG world_x,
		SLONG world_z,

		SLONG *n1,
		SLONG *n2)
{
	SLONG i;
	SLONG j;

	ROAD_Node *rn;

	SLONG dist;

	SLONG best_dist = INFINITY;
	SLONG best_n1   = NULL;
	SLONG best_n2   = NULL;

	for (i = 1; i < ROAD_node_upto; i++)
	{
		rn = &ROAD_node[i];

		for (j = 0; j < 4; j++)
		{
			if (rn->c[j] == 0)
			{
				break;
			}

			if (rn->c[j] < i)
			{
				dist = abs(0x100 - ROAD_signed_dist(i, rn->c[j], world_x, world_z));

				if (dist < best_dist)
				{
					best_dist = dist;
					best_n1   = i;
					best_n2   = rn->c[j];
				}
			}
		}
	}

   *n1 = best_n1;
   *n2 = best_n2;
}



void ROAD_whereto_now(
		SLONG  n1,
		SLONG  n2,
		SLONG *wtn1,
		SLONG *wtn2)
{
	SLONG i;
	SLONG score;

	SLONG best_node  = NULL;
	SLONG best_score = 0;

	ASSERT(WITHIN(n1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n2, 1, ROAD_node_upto - 1));

	ROAD_Node *rn = &ROAD_node[n2];

	for (i = 0; i < 4; i++)
	{
		if (rn->c[i])
		{
			if (rn->c[i] == n1)
			{
				score = 10;
			}
			else
			{
				score  = Random() & 0xff;
				score += 20;
			}

			if (score > best_score)
			{
				best_score = score;
				best_node  = rn->c[i];
			}
		}
	}

   *wtn1 = n2;
   *wtn2 = best_node;
}


void ROAD_get_dest(
		SLONG  n1,
		SLONG  n2,
		SLONG *world_x,
		SLONG *world_z)
{
	SLONG dx;
	SLONG dz;

	ASSERT(WITHIN(n1, 1, ROAD_node_upto - 1));
	ASSERT(WITHIN(n2, 1, ROAD_node_upto - 1));

	ROAD_Node *rn1 = &ROAD_node[n1];
	ROAD_Node *rn2 = &ROAD_node[n2];

	dx = SIGN(rn2->x - rn1->x) * ROAD_LANE;
	dz = SIGN(rn2->z - rn1->z) * ROAD_LANE;

   *world_x = ((rn2->x << 8) + 0x80) - dz;
   *world_z = ((rn2->z << 8) + 0x80) + dx;

	if (rn2->x == 0) {*world_x -= 0x400;}
	if (rn2->z == 0) {*world_z -= 0x400;}

	if (rn2->x == PAP_SIZE_HI - 1) {*world_x += 0x400;}
	if (rn2->z == PAP_SIZE_HI - 1) {*world_z += 0x400;}
}


//
// There are 512 textures on the map and 2 bits for each texture.
// 

UBYTE ROAD_mapsquare_type[512 / 4];

#if !defined(PSX) && !defined(TARGET_DC)

void ROAD_calc_mapsquare_type()
{
	SLONG mx;
	SLONG mz;
	SLONG page;
	SLONG offset;
	SLONG index;
	SLONG look;

	PAP_Hi *ph;

	UBYTE done[512];

	memset(done, 0, sizeof(done));

	for (mx = 0; mx < PAP_SIZE_HI; mx++)
	for (mz = 0; mz < PAP_SIZE_HI; mz++)
	{
		ph = &PAP_2HI(mx,mz);

		//
		// The texture page on this mapsquare.
		// 

		page = ph->Texture & 0x3ff;

		if (!WITHIN(page, 0, 511))
		{
			continue;
		}

		if (!done[page])
		{
			done[page] = TRUE;

			if (ROAD_is_road(mx,mz))
			{
				look = TEXTURE_LOOK_ROAD;
			}
			else
			{
				look = TEXTURE_looks_like(page);
			}

			switch(look)
			{
				case TEXTURE_LOOK_ROAD:
					look = ROAD_TYPE_TARMAC;
					break;

				case TEXTURE_LOOK_GRASS:
					look = ROAD_TYPE_GRASS;
					break;

				case TEXTURE_LOOK_DIRT:
					look = ROAD_TYPE_DIRT;
					break;

				case TEXTURE_LOOK_SLIPPERY:
					look = ROAD_TYPE_SLIPPERY;
					break;

				default:
					ASSERT(0);
					look = ROAD_TYPE_TARMAC;
					break;
			}

			offset = (page & 0x3) << 1;
			index  = page >> 2;

			ROAD_mapsquare_type[index] &= ~(0x3  << offset);
			ROAD_mapsquare_type[index] |=  (look << offset);
		}
	}
}

#endif

SLONG ROAD_get_mapsquare_type(SLONG mx, SLONG mz)
{
	SLONG page;
	SLONG look;
	SLONG offset;
	SLONG index;

	PAP_Hi *ph;

	ph = &PAP_2HI(mx,mz);

	//
	// The texture page on this mapsquare.
	// 

	page = ph->Texture & 0x3ff;


	if (WITHIN(page, 0, 511))
	{
		offset = (page & 0x3) << 1;
		index  = page >> 2;

		look  = ROAD_mapsquare_type[index] >> offset;
		look &= 0x3;
	}
	else
	{
		look = ROAD_TYPE_TARMAC;
	}

	return look;
}




