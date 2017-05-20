#ifdef	PSX
//
// Hypermatter!
//


#include <MFStdLib.h>
#include <stdlib.h>
#include <hm.h>
#include "c:\fallen\ddengine\headers\message.h"
#include "game.h"
#include "c:\fallen\ddengine\headers\matrix.h"

#include "libmath.h"

//
// How to draw a line in the world.
//

void e_draw_3d_line           (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2, SLONG r, SLONG g, SLONG b);

//
// Fast approximation to vector length in 3d.
//

#define float SLONG
#define float(x) SLONG(x)

static inline float qdist(float x, float y, float z)
{
	float ans;

	ASSERT(x >= 0.0F);
	ASSERT(y >= 0.0F);
	ASSERT(z >= 0.0F);

	if (x > y)
	{
		if (x > z)
		{
			//
			// x is the biggeset.
			//

			ans = x + (y + z) * 0.2941F;

			return ans;
		}
	}
	else
	{
		if (y > z)
		{
			//
			// y is the biggeset.
			//

			ans = y + (x + z) * 0.2941F;

			return ans;
		}
	}

	//
	// z is the biggeset.
	//

	ans = z + (x + y) * 0.2941F;

	return ans;
}


//
// Each hypermatter point.
//

typedef struct
{
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
	float mass;	// How much gravity we apply to this point.

} HM_Point;

//
// An edge connecting two points for fast force calculations
// between pairs of points.
//

typedef struct
{
	UWORD p1;
	UWORD p2;
	UBYTE len;	// Index into the length squared array...
	UBYTE shit;

} HM_Edge;

typedef UWORD HM_Index;	// Index into the point[] array or NULL if there is no point here.

//
// Each point of the original prim is given in terms of the
// position of three prim points relative to an origin.
//

typedef struct
{
	UWORD origin;
	UWORD p[3];
	float along[3];

} HM_Mesh;

//
// When a point enters a cube of hypermatter belonging to a different object,
// one of these structures is created and tagged onto the linked list for
// each point.
//

typedef struct hm_bump
{
	//
	// The point that has entered the other object.
	// 

	UWORD point;
	UWORD shit;

	// 
	// The other object the point is inside, and the cube
	// of that object the point entered.
	//

	HM_Index hm_index;
	UBYTE    cube_x;
	UBYTE    cube_y;
	UBYTE    cube_z;

	//
	// The point where the point entered the object, relative to
	// the cube.
	//

	float rel_x;
	float rel_y;
	float rel_z;
	
	//
	// When this point recieves a force that tries to make it leave the
	// cube, an equal and opposite force must be applied to the cube.
	// The force is applied to these eight points. The proportions should
	// all add up to one.
	//

	#define HM_NUM_OPP_POINTS 8

	UWORD  opp_point[HM_NUM_OPP_POINTS];
	float  opp_prop [HM_NUM_OPP_POINTS];

	//
	// The next structure in the linked list
	// 

	struct hm_bump *next;
} HM_Bump;




//
// An edge that a hypermatter object collides with.
//

typedef struct
{
	ULONG consider;	// 1 bit for each point, on if that point is worth considering...

	float x1, z1;
	float x2, z2;
	float len;
	
} HM_Col;


//
// Each hypermatter object.
//

#define HM_MAX_SIZES		256
#define HM_COLS_PER_OBJECT	64

typedef struct
{
	UBYTE     used;
	UBYTE     shit;
	UWORD     prim;
	SLONG     x_res;
	SLONG     y_res;
	SLONG     z_res;
	SLONG     x_res_times_y_res;
	SLONG     num_indices;
	SLONG     num_points;
	SLONG     num_edges;
	SLONG     num_sizes;
	SLONG     num_meshes;
	SLONG     num_cols;
	HM_Index *index;
	HM_Point *point;
	HM_Edge  *edge;
	HM_Mesh  *mesh;
	HM_Col    col[HM_COLS_PER_OBJECT];
	HM_Bump  *bump;

	//
	// To recreate the (x,y,z) and matrix where we could
	// draw the prim. to fit as best as it can inside the hypermatter.
	//

	SLONG x_index;		// Indices into the point array.
	SLONG y_index;
	SLONG z_index;
	SLONG o_index;
	float o_prim_x;
	float o_prim_y;
	float o_prim_z;
	float cog_x;		// The point about which the prim object rotates.
	float cog_y;
	float cog_z;

	float size    [HM_MAX_SIZES];
	float oversize[HM_MAX_SIZES];	// The reciprocal of the size array.
	float elasticity;
	float bounciness;
	float friction;
	float damping;

} HM_Object;

#define HM_MAX_OBJECTS 8

HM_Object HM_object[HM_MAX_OBJECTS];
SLONG     HM_object_upto;


//
// Returns the index of the given point.
// 

inline SLONG HM_index(HM_Object *ho, SLONG x, SLONG y, SLONG z)
{
	SLONG ans;

	ASSERT(WITHIN(x, 0, ho->x_res - 1));
	ASSERT(WITHIN(y, 0, ho->y_res - 1));
	ASSERT(WITHIN(z, 0, ho->z_res - 1));

	ans  = x;
	ans += y * ho->x_res;
	ans += z * ho->x_res_times_y_res;

	return ans;
}


void HM_init()
{
	SLONG i;

	for (i = 0; i < HM_MAX_OBJECTS; i++)
	{
		HM_object[i].used = FALSE;
	}
}


#define HM_MAX_PRIMGRIDS 64

HM_Primgrid HM_primgrid[HM_MAX_PRIMGRIDS];
SLONG       HM_primgrid_upto;


void HM_load(CBYTE *fname)
{
	return;
}

HM_Primgrid HM_default_primgrid =
{
	0,
	2,
	2,
	2,

	{0, 0x10000},
	{0, 0x10000},
	{0, 0x10000},

	 0.00F,
	 0.00F,
	-0.25F
};


HM_Primgrid *HM_get_primgrid(SLONG prim)
{
	SLONG i;

	for (i = 0; i < HM_primgrid_upto; i++)
	{
		if (HM_primgrid[i].prim == prim)
		{
			return &HM_primgrid[i];
		}
	}

	return &HM_default_primgrid;
}





HM_Mesh HM_find_point_inside_cube(
			HM_Object *ho,
			SLONG      x,
			SLONG      y,
			SLONG      z,
			float      ppx,
			float      ppy,
			float      ppz)
{
	SLONG index_o;
	SLONG index_x;
	SLONG index_y;
	SLONG index_z;

	HM_Point *p_o;
	HM_Point *p_x;
	HM_Point *p_y;
	HM_Point *p_z;

	float along_x;
	float along_y;
	float along_z;

	HM_Mesh ans;

	//
	// Find the origin of the cube.
	//

	index_o = ho->index[HM_index(ho, x + 0, y + 0, z + 0)];
	index_x = ho->index[HM_index(ho, x + 1, y + 0, z + 0)];
	index_y = ho->index[HM_index(ho, x + 0, y + 1, z + 0)];
	index_z = ho->index[HM_index(ho, x + 0, y + 0, z + 1)];

	ASSERT(WITHIN(index_o, 1, ho->num_points));
	ASSERT(WITHIN(index_x, 1, ho->num_points));
	ASSERT(WITHIN(index_y, 1, ho->num_points));
	ASSERT(WITHIN(index_z, 1, ho->num_points));

	p_o = &ho->point[index_o];
	p_x = &ho->point[index_x];
	p_y = &ho->point[index_y];
	p_z = &ho->point[index_z];

	//
	// Find the amount you are along each edge.
	//

	along_x = (ppx - p_o->x) / (p_x->x - p_o->x);
	along_y = (ppy - p_o->y) / (p_y->y - p_o->y);
	along_z = (ppz - p_o->z) / (p_z->z - p_o->z);

	ASSERT(WITHIN(along_x, 0.0F, 1.0F));
	ASSERT(WITHIN(along_y, 0.0F, 1.0F));
	ASSERT(WITHIN(along_z, 0.0F, 1.0F));

	if (along_x + along_y + along_z > 1.5F)
	{
		//
		// We use the other corner of cube because it iss nearer.
		//

		index_o = ho->index[HM_index(ho, x + 1, y + 1, z + 1)];
		index_x = ho->index[HM_index(ho, x + 0, y + 1, z + 1)];
		index_y = ho->index[HM_index(ho, x + 1, y + 0, z + 1)];
		index_z = ho->index[HM_index(ho, x + 1, y + 1, z + 0)];

		ASSERT(WITHIN(index_o, 1, ho->num_points));
		ASSERT(WITHIN(index_x, 1, ho->num_points));
		ASSERT(WITHIN(index_y, 1, ho->num_points));
		ASSERT(WITHIN(index_z, 1, ho->num_points));

		along_x = 1.0F - along_x;
		along_y = 1.0F - along_y;
		along_z = 1.0F - along_z;
	}

	ans.origin   = index_o;
	ans.p[0]     = index_x;
	ans.p[1]     = index_y;
	ans.p[2]     = index_z;
	ans.along[0] = along_x;
	ans.along[1] = along_y;
	ans.along[2] = along_z;

	return ans;
}


UBYTE HM_create(

		SLONG prim,

		SLONG pos_x,
		SLONG pos_y,
		SLONG pos_z,

		SLONG yaw,
		SLONG pitch,
		SLONG roll,

		SLONG vel_x,
		SLONG vel_y,
		SLONG vel_z,

		SLONG x_res,		// The number of points along the x-axis
		SLONG y_res,		// The number of points along the y-axis
		SLONG z_res,		// The number of points along the z-axis

		SLONG x_point[],	// The position of each point along the x-axis, 0 => The bounding box min, 0x10000 => the bb max.
		SLONG y_point[],
		SLONG z_point[],

		float x_dgrav,
		float y_dgrav,
		float z_dgrav,

		//
		// These go from 0 to 1.
		// 

		float elasticity,
		float bounciness,
		float friction,
		float damping)
{
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG index;
	SLONG index1;
	SLONG index2;

	SLONG num_points;
	SLONG num_edges;

	SLONG edge_upto;
	SLONG point_upto;

	SLONG ans;

	float dpx;
	float dpy;
	float dpz;
	float size;

	HM_Object *ho;
	HM_Point  *hp;
	HM_Point  *hp1;
	HM_Point  *hp2;
	HM_Edge   *he;

	//
	// Look for an unused HM_Object...
	//

	for (i = 0; i < HM_MAX_OBJECTS; i++)
	{
		ho = &HM_object[i];

		if (!ho->used)
		{
			//
			// Found one!
			//

			ans = i;

			goto found_unused_hm_object;
		}
	}

	//
	// Oh dear!
	//

	return HM_NO_MORE_OBJECTS;

  found_unused_hm_object:;

	ho->used  = TRUE;
	ho->prim  = prim;
	ho->x_res = x_res;
	ho->y_res = y_res;
	ho->z_res = z_res;

	ho->x_res_times_y_res = ho->x_res * ho->y_res;

	ho->elasticity = elasticity;
	ho->bounciness = bounciness;
	ho->friction   = friction;
	ho->damping    = damping;
	ho->bump       = NULL;

	//
	// The bounding box of the prim.
	//

	ASSERT(WITHIN(prim, 1, next_prim_object - 1));

	PrimObject *po = &prim_objects[prim];
	PrimInfo   *pi =  get_prim_info(prim);
	PrimPoint  *pp;
	PrimFace3  *f3;
	PrimFace4  *f4;

	ASSERT(WITHIN(x_res, 2, HM_MAX_RES));
	ASSERT(WITHIN(y_res, 2, HM_MAX_RES));
	ASSERT(WITHIN(z_res, 2, HM_MAX_RES));

	UBYTE empty[HM_MAX_RES][HM_MAX_RES][HM_MAX_RES];
	SLONG cubex[HM_MAX_RES];
	SLONG cubey[HM_MAX_RES];
	SLONG cubez[HM_MAX_RES];

	//
	// Work out the positions of all the unrotated points of
	// the new hypermatter object.
	//

	for (i = 0; i < x_res; i++) {cubex[i] = pi->minx + MUL64(x_point[i], pi->maxx - pi->minx);}
	for (i = 0; i < y_res; i++) {cubey[i] = pi->miny + MUL64(y_point[i], pi->maxy - pi->miny);}
	for (i = 0; i < z_res; i++) {cubez[i] = pi->minz + MUL64(z_point[i], pi->maxz - pi->minz);}

	//
	// Mark all the cubes as empty.
	//

	for (x = 0; x < x_res - 1; x++)
	for (y = 0; y < y_res - 1; y++)
	for (z = 0; z < z_res - 1; z++)
	{
		empty[x][y][z] = TRUE;
	}

	//
	// Go through all the points. Mark the cube a point is in as
	// not empty.
	//

	for (i = po->StartPoint; i < po->EndPoint; i++)
	{
		pp = &prim_points[i];

		for (x = 0; x < x_res - 1; x++)
		{
			if (!WITHIN(pp->X, cubex[x], cubex[x + 1]))
			{
				continue;
			}

			for (y = 0; y < y_res - 1; y++)
			{
				if (!WITHIN(pp->Y, cubey[y], cubey[y + 1]))
				{
					continue;
				}

				for (z = 0; z < z_res - 1; z++)
				{
					if (WITHIN(pp->Z, cubez[z], cubez[z + 1]))
					{
						empty[x][y][z] = FALSE;

						goto do_next_point;
					}
				}
			}
		}

	  do_next_point:;
	}

	//
	// Allocate the indices.
	//

	ho->num_indices = ho->x_res * ho->y_res * ho->z_res;
	ho->index       = (HM_Index *) malloc(ho->num_indices * sizeof(HM_Index));

	ASSERT(ho->index != NULL);

	//
	// Clear out the indices.
	//

	memset((UBYTE*)ho->index, 0, ho->num_indices * sizeof(HM_Index));

	//
	// Go through all the active cubes and mark the points they used as alive.
	//

	for (x = 0; x < x_res - 1; x++)
	for (y = 0; y < y_res - 1; y++)
	for (z = 0; z < z_res - 1; z++)
	{
		if (!empty[x][y][z])
		{
			ho->index[HM_index(ho, x + 0, y + 0, z + 0)] = 0xffff;
			ho->index[HM_index(ho, x + 1, y + 0, z + 0)] = 0xffff;
			ho->index[HM_index(ho, x + 0, y + 1, z + 0)] = 0xffff;
			ho->index[HM_index(ho, x + 1, y + 1, z + 0)] = 0xffff;

			ho->index[HM_index(ho, x + 0, y + 0, z + 1)] = 0xffff;
			ho->index[HM_index(ho, x + 1, y + 0, z + 1)] = 0xffff;
			ho->index[HM_index(ho, x + 0, y + 1, z + 1)] = 0xffff;
			ho->index[HM_index(ho, x + 1, y + 1, z + 1)] = 0xffff;
		}
	}

	//
	// How many points do we need?
	//

	num_points = 1;	// point 0 is the NULL point, so we always need one point.

	for (x = 0; x < x_res; x++)
	for (y = 0; y < y_res; y++)
	for (z = 0; z < z_res; z++)
	{
		if (ho->index[HM_index(ho, x, y, z)] != NULL)
		{
			num_points += 1;
		}
	}
	
	//
	// Create the points array.
	// 

	ho->point = (HM_Point *) malloc(num_points * sizeof(HM_Point));

	ASSERT(ho->point != NULL);

	//
	// Put in the correct indices into the point array.
	//

	float grav_mid_x = float(x_res) * 0.5F;
	float grav_mid_y = float(y_res) * 0.5F;
	float grav_mid_z = float(z_res) * 0.5F;

	point_upto = 1;

	for (x = 0; x < x_res; x++)
	for (y = 0; y < y_res; y++)
	for (z = 0; z < z_res; z++)
	{
		if (ho->index[HM_index(ho, x, y, z)] != NULL)
		{
			ASSERT(WITHIN(point_upto, 1, num_points - 1));

			ho->index[HM_index(ho, x, y, z)] = point_upto;

			ho->point[point_upto].x = float(cubex[x]);
			ho->point[point_upto].y = float(cubey[y]);
			ho->point[point_upto].z = float(cubez[z]);

			ho->point[point_upto].dx = float(vel_x);
			ho->point[point_upto].dy = float(vel_y);
			ho->point[point_upto].dz = float(vel_z);

			ho->point[point_upto].mass  = 1.0F;
			ho->point[point_upto].mass += (float(x) - grav_mid_x) * x_dgrav;
			ho->point[point_upto].mass += (float(y) - grav_mid_y) * y_dgrav;
			ho->point[point_upto].mass += (float(z) - grav_mid_z) * z_dgrav;

			point_upto += 1;
		}
	}

	ASSERT(point_upto == num_points);

	ho->num_points = num_points;

	//
	// Adjust the gravity of the points so that all objects have the
	// same force of gravity acting on them!
	//
	
	float grav_av = 0.0F;
	float grav_adjust;

	for (i = 1; i < ho->num_points; i++)
	{
		grav_av += ho->point[i].mass;
	}

	grav_av /= float(ho->num_points - 1);

	if (grav_av < 0.01F)
	{
		//
		// Give up!
		//
	}
	else
	{
		grav_adjust = 1.0F / grav_av;

		for (i = 1; i < ho->num_points; i++)
		{
			ho->point[i].mass *= grav_adjust;
		}
	}

	//
	// Count how many edges we need.
	//

	num_edges = 0;

	for (z = 0; z < ho->z_res; z++)
	for (y = 0; y < ho->y_res; y++)
	for (x = 0; x < ho->x_res; x++)
	{
		index1 = HM_index(ho, x, y, z);

		if (ho->index[index1] == NULL)
		{
			continue;
		}

		for (dz =  0; dz <= 1; dz++)
		for (dy = -1; dy <= 1; dy++)
		for (dx = -1; dx <= 1; dx++)
		{
			if ((dz ==  1)            ||
				(dx == -1 && dy == 1) ||
				(dx ==  0 && dy == 1) ||
				(dx ==  1 && dy == 1) ||
				(dx ==  1 && dy == 0))
			{
				if (WITHIN(x + dx, 0, ho->x_res - 1) &&
					WITHIN(y + dy, 0, ho->y_res - 1) &&
					WITHIN(z + dz, 0, ho->z_res - 1))
				{
					index2 = HM_index(ho, x + dx, y + dy, z + dz);

					if (ho->index[index2] != NULL)
					{
						num_edges += 1;
					}
					else
					{
						num_edges += 0;
					}
				}
			}
		}
	}

	ho->num_edges = num_edges;

	//
	// Reserve enough memory for them.
	//

	ho->edge = (HM_Edge *) malloc(num_edges * sizeof(HM_Edge));

	//
	// Build the edges.
	//

	edge_upto     = 0;
	ho->num_sizes = 0;

	for (z = 0; z < ho->z_res; z++)
	for (y = 0; y < ho->y_res; y++)
	for (x = 0; x < ho->x_res; x++)
	{
		index1 = HM_index(ho, x, y, z);

		if (ho->index[index1] == NULL)
		{
			continue;
		}

		for (dz =  0; dz <= 1; dz++)
		for (dy = -1; dy <= 1; dy++)
		for (dx = -1; dx <= 1; dx++)
		{
			if ((dz ==  1)            ||
				(dx == -1 && dy == 1) ||
				(dx ==  0 && dy == 1) ||
				(dx ==  1 && dy == 1) ||
				(dx ==  1 && dy == 0))
			{
				if (WITHIN(x + dx, 0, ho->x_res - 1) &&
					WITHIN(y + dy, 0, ho->y_res - 1) &&
					WITHIN(z + dz, 0, ho->z_res - 1))
				{
					index2 = HM_index(ho, x + dx, y + dy, z + dz);

					if (ho->index[index2] != NULL)
					{
						ASSERT(WITHIN(edge_upto, 0, ho->num_edges - 1));

						ho->edge[edge_upto].p1  = ho->index[index1];
						ho->edge[edge_upto].p2  = ho->index[index2];

						hp1 = &ho->point[ho->index[index1]];
						hp2 = &ho->point[ho->index[index2]];

						dpx = hp2->x - hp1->x;
						dpy = hp2->y - hp1->y;
						dpz = hp2->z - hp1->z;

						size = dpx*dpx + dpy*dpy + dpz*dpz;

						for (i = 0; i < ho->num_sizes; i++)
						{
							if (ho->size[i] == size)
							{
								ho->edge[edge_upto].len = i;

								goto found_size;
							}
						}
						
						ASSERT(WITHIN(ho->num_sizes, 0, HM_MAX_SIZES - 1));

						//
						// Create a new element in the size array.
						//

						ho->size    [ho->num_sizes] = size;
						ho->oversize[ho->num_sizes] = 1.0F / size;

						ho->edge[edge_upto].len = ho->num_sizes;

						ho->num_sizes += 1;

					  found_size:;

						edge_upto += 1;
					}
				}
			}
		}
	}

	ASSERT(edge_upto == num_edges);

	//
	// Create the HM_mesh...
	//

	ho->num_meshes = po->EndPoint - po->StartPoint;
	ho->mesh       = (HM_Mesh *) malloc(ho->num_meshes * sizeof(HM_Mesh));

	ASSERT(ho->mesh != NULL);

	for (i = 0; i < ho->num_meshes; i++)
	{
		//
		// Which cube is this point in?
		//

		pp = &prim_points[po->StartPoint + i];

		for (x = 0; x < x_res - 1; x++)
		{
			if (!WITHIN(pp->X, cubex[x], cubex[x + 1]))
			{
				continue;
			}

			for (y = 0; y < y_res - 1; y++)
			{
				if (!WITHIN(pp->Y, cubey[y], cubey[y + 1]))
				{
					continue;
				}

				for (z = 0; z < z_res - 1; z++)
				{
					if (WITHIN(pp->Z, cubez[z], cubez[z + 1]))
					{
						//
						// This point is in cube (x,y,z)
						//

						ho->mesh[i] = HM_find_point_inside_cube(
										ho,
										x, y, z,
										float(pp->X),
										float(pp->Y),
										float(pp->Z));

						goto done_this_point;
					}
				}
			}
		}

		ASSERT(0);

	  done_this_point:;

	}

	//
	// Find the best three vectors to use for calculating the
	// position and orientation of the original prim.
	//

	float best_score  = float(INFINITY);
	SLONG best_origin = -1;
	SLONG best_x = -1;
	SLONG best_y = -1;
	SLONG best_z = -1;

	SLONG index_o;
	SLONG index_x;
	SLONG index_y;
	SLONG index_z;

	float score;

	for (x = 0; x < ho->x_res - 1; x++)
	for (y = 0; y < ho->y_res - 1; y++)
	for (z = 0; z < ho->z_res - 1; z++)
	{
		index_o = ho->index[HM_index(ho, x,     y,     z    )];
		index_x = ho->index[HM_index(ho, x + 1, y,     z    )];
		index_y = ho->index[HM_index(ho, x,     y + 1, z    )];
		index_z = ho->index[HM_index(ho, x,     y,     z + 1)];

		if (index_o != NULL &&
			index_x != NULL &&
			index_y != NULL &&
			index_z != NULL)
		{
			//
			// Points closer to the centre of gravity and origin of
			// the prim are better.
			//

			score  = 0;

			score += fabs(ho->point[index_o].x - float(pi->cogx));
			score += fabs(ho->point[index_o].y - float(pi->cogy));
			score += fabs(ho->point[index_o].z - float(pi->cogz));

			score += fabs(ho->point[index_o].x);
			score += fabs(ho->point[index_o].y);
			score += fabs(ho->point[index_o].z);

			if (score < best_score)
			{
				best_score  = score;
				best_origin = index_o;
				best_x      = index_x;
				best_y      = index_y;
				best_z      = index_z;
			}
		}
	}

	if (best_score >= INFINITY)
	{
		//
		// Oh hell! We couldn't find and decent vectors.
		//

		free(ho->index);
		free(ho->point);
		free(ho->edge);
		free(ho->mesh);

		return HM_NO_MORE_OBJECTS;
	}
	else
	{

		ho->x_index  = best_x;
		ho->y_index  = best_y;
		ho->z_index  = best_z;
		ho->o_index  = best_origin;
		ho->o_prim_x = ho->point[best_origin].x;
		ho->o_prim_y = ho->point[best_origin].y;
		ho->o_prim_z = ho->point[best_origin].z;
		ho->cog_x    = float(pi->cogx);
		ho->cog_y    = float(pi->cogy);
		ho->cog_z    = float(pi->cogz);
	}

	//
	// The rotation matrix of the prim.
	//

	float f_yaw   = float(yaw)   * 2.0F * PI / 2048.0F;
	float f_pitch = float(pitch) * 2.0F * PI / 2048.0F;
	float f_roll  = float(roll)  * 2.0F * PI / 2048.0F;

	float matrix[9];

//mdPSX 	MATRIX_calc(matrix, f_yaw, f_pitch, f_roll);

	//
	// Rotate the points about the centre of gravity.
	//

	float cogx = float(pi->cogx);
	float cogy = float(pi->cogy);
	float cogz = float(pi->cogz);

	for (i = 1; i < ho->num_points; i++)
	{
		ho->point[i].x -= cogx;
		ho->point[i].y -= cogy;
		ho->point[i].z -= cogz;
/*MDPSX
		MATRIX_MUL_BY_TRANSPOSE(
			matrix,
			ho->point[i].x,
			ho->point[i].y,
			ho->point[i].z);
			*/

		ho->point[i].x += cogx;
		ho->point[i].y += cogy;
		ho->point[i].z += cogz;
	}

	//
	// Put the points at the correct place
	//

	float fposx = float(pos_x);
	float fposy = float(pos_y);
	float fposz = float(pos_z);

	for (i = 1; i < ho->num_points; i++)
	{
		ho->point[i].x += fposx;
		ho->point[i].y += fposy;
		ho->point[i].z += fposz;
	}

	return ans;
}


void HM_destroy(UBYTE hm_index)
{
	ASSERT(WITHIN(hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Object *ho = &HM_object[hm_index];

	if (ho->used)
	{
		ho->used = FALSE;

		//
		// Free up malloc'ed memory.
		//

		free(ho->index);
		free(ho->point);
		free(ho->edge);
		free(ho->mesh);
	}
	else
	{
		//
		// Lets just pretend it was okay...
		//
	}
}


void HM_find_cog(
		UBYTE  hm_index,
		float *x,
		float *y,
		float *z)
{
	SLONG i;

	float ans_x = 0.0F;
	float ans_y = 0.0F;
	float ans_z = 0.0F;

	float tot_mass = 0.0F;

	ASSERT(WITHIN(hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Object *ho = &HM_object[hm_index];
	HM_Point  *hp;

	ASSERT(ho->used);

	for (i = 1; i < ho->num_points; i++)
	{
		hp = &ho->point[i];

		ans_x += hp->x * hp->mass;
		ans_y += hp->y * hp->mass;
		ans_z += hp->z * hp->mass;

		tot_mass += hp->mass;
	}

	ans_x /= tot_mass;
	ans_y /= tot_mass;
	ans_z /= tot_mass;

	*x = ans_x;
	*y = ans_y;
	*z = ans_z;
}



void HM_colvect_clear(UBYTE hm)
{
	ASSERT(WITHIN(hm, 0, HM_MAX_OBJECTS - 1));

	HM_object[hm].num_cols = 0;
}

void HM_colvect_add(
		UBYTE hm,
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG i;

	float dx;
	float dz;

	ASSERT(WITHIN(hm, 0, HM_MAX_OBJECTS - 1));

	HM_Object *ho = &HM_object[hm];
	HM_Col    *hc;

	ASSERT(ho->used);
	ASSERT(WITHIN(ho->num_cols, 0, HM_COLS_PER_OBJECT - 1));

	//
	// Do we already have a colvect like this?
	// 

	for (i = 0; i < ho->num_cols; i++)
	{
		hc = &ho->col[i];

		if (hc->x1 == float(x1) &&
			hc->z1 == float(z1) &&
			hc->x2 == float(x2) &&
			hc->z2 == float(z2))
		{
			return;
		}
	}

	//
	// Create a new colvect for this object to collide with.
	//

	hc = &ho->col[ho->num_cols++];

	hc->x1 = float(x1);
	hc->z1 = float(z1);
	hc->x2 = float(x2);
	hc->z2 = float(z2);

	dx = hc->x2 - hc->x1;
	dz = hc->z2 - hc->z1;

	hc->len = sqrt(dx*dx + dz*dz);

	//
	// Which points of this object are worth considering?
	// All of them for now!
	//

	hc->consider = 0xffffffff;
}

//
// Hmm...
//

//
// This function returns the height of the floor at (x,z).
// It is defined in collide.cpp. We don't #include collide.h
// because then we would have to include thing.h and eventaully
// we'd have to include everything!
//

SLONG calc_height_at(SLONG x, SLONG z);


float HM_height_at(float x, float z)
{
	SLONG ans = calc_height_at(
					SLONG(x),
					SLONG(z));

	return float(ans);
}



//
// Returns the position of the given mesh-point of the HM_object.
//

void HM_find_mesh_point(
		UBYTE  hm_index,
		SLONG  point,
		float *x,
		float *y,
		float *z)
{
	float ansx;
	float ansy;
	float ansz;

	HM_Point *p_o;
	HM_Point *p_x;
	HM_Point *p_y;
	HM_Point *p_z;

	ASSERT(WITHIN(hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Object *ho = &HM_object[hm_index];
	HM_Mesh   *hm;

	ASSERT(WITHIN(point, 0, ho->num_meshes - 1));

	hm = &ho->mesh[point];

	ASSERT(WITHIN(hm->origin, 1, ho->num_points - 1));
	ASSERT(WITHIN(hm->p[0],   1, ho->num_points - 1));
	ASSERT(WITHIN(hm->p[1],   1, ho->num_points - 1));
	ASSERT(WITHIN(hm->p[2],   1, ho->num_points - 1));

	p_o = &ho->point[hm->origin];
	p_x = &ho->point[hm->p[0]];
	p_y = &ho->point[hm->p[1]];
	p_z = &ho->point[hm->p[2]];

	ansx  = p_o->x;
	ansy  = p_o->y;
	ansz  = p_o->z;

	ansx += hm->along[0] * (p_x->x - p_o->x);
	ansy += hm->along[0] * (p_x->y - p_o->y);
	ansz += hm->along[0] * (p_x->z - p_o->z);

	ansx += hm->along[1] * (p_y->x - p_o->x);
	ansy += hm->along[1] * (p_y->y - p_o->y);
	ansz += hm->along[1] * (p_y->z - p_o->z);

	ansx += hm->along[2] * (p_z->x - p_o->x);
	ansy += hm->along[2] * (p_z->y - p_o->y);
	ansz += hm->along[2] * (p_z->z - p_o->z);

	*x = ansx;
	*y = ansy;
	*z = ansz;
}

//
// Returnstrue if the cube of the hypermatter object exists. If you pass
// an out-of-bounds cube, then it just returns FALSE.
//

SLONG HM_cube_exists(
		HM_Object *ho,
		SLONG      x_cube,
		SLONG      y_cube,
		SLONG      z_cube)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG index;

	if (!WITHIN(x_cube, 0, ho->x_res - 2) ||
		!WITHIN(y_cube, 0, ho->y_res - 2) ||
		!WITHIN(z_cube, 0, ho->z_res - 2))
	{
		return FALSE;
	}

	for (i = 0; i < 8; i++)
	{
		dx = (i >> 0) & 1;
		dy = (i >> 1) & 1;
		dz = (i >> 2) & 1;

		px = x_cube + dx;
		py = y_cube + dy;
		pz = z_cube + dz;

		if (!WITHIN(px, 0, ho->x_res - 1) ||
			!WITHIN(py, 0, ho->y_res - 1) ||
			!WITHIN(pz, 0, ho->z_res - 1))
		{
			return FALSE;
		}

		index = HM_index(ho, px, py, pz);

		if (ho->index[index] == NULL)
		{
			//
			// No point here.
			//

			return FALSE;
		}
	}

	return TRUE;
}


//
// Returns true if 'p' is inside the given cube of the hypermatter object.
// If it is it gives a point on an outside surface of the cube that 'p' entered by.
// The point is given in terms of the three vectors that go out of the origin of the
// cube.
//

SLONG HM_is_point_in_cube(
		HM_Object *ho,		// The object the cube is a part of...
		HM_Point  *p,
		SLONG      x_cube,
		SLONG      y_cube,
		SLONG      z_cube,
		float     *rel_x,
		float     *rel_y,
		float     *rel_z)
{
	HM_Point *hp_o;
	HM_Point *hp_x;
	HM_Point *hp_y;
	HM_Point *hp_z;

	float rx;
	float ry;
	float rz;
	float len;
	float matrix[9];

	SLONG index_o;
	SLONG index_x;
	SLONG index_y;
	SLONG index_z;

	ASSERT(WITHIN(x_cube, 0, ho->x_res - 2));
	ASSERT(WITHIN(y_cube, 0, ho->y_res - 2));
	ASSERT(WITHIN(z_cube, 0, ho->z_res - 2));

	//
	// Make sure this cube exists...
	//

	if (!HM_cube_exists(ho, x_cube, y_cube, z_cube))
	{
		return FALSE;
	}

	//
	// ASSUME THAT THE CUBE IS IN A RIGID SHAPE.
	//

	index_o = HM_index(ho, x_cube + 0, y_cube + 0, z_cube + 0);
	index_x = HM_index(ho, x_cube + 1, y_cube + 0, z_cube + 0);
	index_y = HM_index(ho, x_cube + 0, y_cube + 1, z_cube + 0);
	index_z = HM_index(ho, x_cube + 0, y_cube + 0, z_cube + 1);

	ASSERT(WITHIN(ho->index[index_o], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_x], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_y], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_z], 1, ho->num_points - 1));

	hp_o = &ho->point[ho->index[index_o]];
	hp_x = &ho->point[ho->index[index_x]];
	hp_y = &ho->point[ho->index[index_y]];
	hp_z = &ho->point[ho->index[index_z]];

	//
	// Create a rotation matrix that rotates point 'p' into the space of the cube.
	//

	matrix[0] = hp_x->x - hp_o->x;
	matrix[1] = hp_x->y - hp_o->y;
	matrix[2] = hp_x->z - hp_o->z;

	matrix[3] = hp_y->x - hp_o->x;
	matrix[4] = hp_y->y - hp_o->y;
	matrix[5] = hp_y->z - hp_o->z;

	matrix[6] = hp_z->x - hp_o->x;
	matrix[7] = hp_z->y - hp_o->y;
	matrix[8] = hp_z->z - hp_o->z;

	//
	// ASSUME THAT THE MATRIX IS ORTHOGONAL!  Normalise each vector of the matrix
	// 'doubley' so that the space of the box goes from 0 to 1 in each axis..
	//

	len = matrix[0]*matrix[0] + matrix[1]*matrix[1] + matrix[2]*matrix[2];
	len = 1.0F / len;

	matrix[0] *= len;
	matrix[1] *= len;
	matrix[2] *= len;

	len = matrix[3]*matrix[3] + matrix[4]*matrix[4] + matrix[5]*matrix[5];
	len = 1.0F / len;

	matrix[3] *= len;
	matrix[4] *= len;
	matrix[5] *= len;

	len = matrix[6]*matrix[6] + matrix[7]*matrix[7] + matrix[8]*matrix[8];
	len = 1.0F / len;

	matrix[6] *= len;
	matrix[7] *= len;
	matrix[8] *= len;

	//
	// Find the coordinates of point 'p' in the space of this cube.
	//

	rx = p->x - hp_o->x;
	ry = p->y - hp_o->y;
	rz = p->z - hp_o->z;

//MDPSX	MATRIX_MUL(matrix, rx, ry, rz);

	//
	// Is the point inside the cube?
	//

	if (WITHIN(rx, 0.0F, 1.0F) &&
		WITHIN(ry, 0.0F, 1.0F) &&
		WITHIN(rz, 0.0F, 1.0F))
	{
		//
		// This point is inside the cube.
		//

		*rel_x = rx;
		*rel_y = ry;
		*rel_z = rz;

		return TRUE;
	}

	return FALSE;
}

//
// Find the last position of the point relative to the last position of the cube.
//

void HM_last_point_in_last_cube(
		HM_Object *ho,		// The object the cube is a part of...
		HM_Point  *p,
		SLONG      x_cube,
		SLONG      y_cube,
		SLONG      z_cube,
		float     *last_rel_x,
		float     *last_rel_y,
		float     *last_rel_z)
{
	HM_Point *hp_o;
	HM_Point *hp_x;
	HM_Point *hp_y;
	HM_Point *hp_z;

	float rx;
	float ry;
	float rz;
	float len;
	float matrix[9];

	SLONG index_o;
	SLONG index_x;
	SLONG index_y;
	SLONG index_z;

	ASSERT(WITHIN(x_cube, 0, ho->x_res - 2));
	ASSERT(WITHIN(y_cube, 0, ho->y_res - 2));
	ASSERT(WITHIN(z_cube, 0, ho->z_res - 2));

	//
	// This cube must exist.
	//

	ASSERT(HM_cube_exists(ho, x_cube, y_cube, z_cube));

	//
	// ASSUME THAT THE CUBE IS IN A RIGID SHAPE.
	//

	index_o = HM_index(ho, x_cube + 0, y_cube + 0, z_cube + 0);
	index_x = HM_index(ho, x_cube + 1, y_cube + 0, z_cube + 0);
	index_y = HM_index(ho, x_cube + 0, y_cube + 1, z_cube + 0);
	index_z = HM_index(ho, x_cube + 0, y_cube + 0, z_cube + 1);

	ASSERT(WITHIN(ho->index[index_o], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_x], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_y], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_z], 1, ho->num_points - 1));

	hp_o = &ho->point[ho->index[index_o]];
	hp_x = &ho->point[ho->index[index_x]];
	hp_y = &ho->point[ho->index[index_y]];
	hp_z = &ho->point[ho->index[index_z]];

	//
	// Create a rotation matrix that rotates point 'p' into the space of the cube.
	//

	matrix[0] = (hp_x->x - hp_x->dx) - (hp_o->x - hp_o->dx);
	matrix[1] = (hp_x->y - hp_x->dy) - (hp_o->y - hp_o->dy);
	matrix[2] = (hp_x->z - hp_x->dz) - (hp_o->z - hp_o->dz);

	matrix[3] = (hp_y->x - hp_y->dx) - (hp_o->x - hp_o->dx);
	matrix[4] = (hp_y->y - hp_y->dy) - (hp_o->y - hp_o->dy);
	matrix[5] = (hp_y->z - hp_y->dz) - (hp_o->z - hp_o->dz);

	matrix[6] = (hp_z->x - hp_z->dx) - (hp_o->x - hp_o->dx);
	matrix[7] = (hp_z->y - hp_z->dy) - (hp_o->y - hp_o->dy);
	matrix[8] = (hp_z->z - hp_z->dz) - (hp_o->z - hp_o->dz);

	//
	// ASSUME THAT THE MATRIX IS ORTHOGONAL!  Normalise each vector of the matrix
	// 'doubley' so that the space of the box goes from 0 to 1 in each axis..
	//

	len = matrix[0]*matrix[0] + matrix[1]*matrix[1] + matrix[2]*matrix[2];
	len = 1.0F / len;

	matrix[0] *= len;
	matrix[1] *= len;
	matrix[2] *= len;

	len = matrix[3]*matrix[3] + matrix[4]*matrix[4] + matrix[5]*matrix[5];
	len = 1.0F / len;

	matrix[3] *= len;
	matrix[4] *= len;
	matrix[5] *= len;

	len = matrix[6]*matrix[6] + matrix[7]*matrix[7] + matrix[8]*matrix[8];
	len = 1.0F / len;

	matrix[6] *= len;
	matrix[7] *= len;
	matrix[8] *= len;

	//
	// Find the coordinates of point 'p' in the space of this cube.
	//

	rx = (p->x - p->dx) - (hp_o->x - hp_o->dx);
	ry = (p->y - p->dy) - (hp_o->y - hp_o->dy);
	rz = (p->z - p->dz) - (hp_o->z - hp_o->dz);

//MDPSX	MATRIX_MUL(matrix, rx, ry, rz);

	*last_rel_x = rx;
	*last_rel_y = ry;
	*last_rel_z = rz;
}



void HM_collide_all()
{
	SLONG i;
	SLONG j;

	for (i = 0; i < HM_MAX_OBJECTS; i++)
	{
		if (!HM_object[i].used)
		{
			continue;
		}

		for (j = i + 1;	j < HM_MAX_OBJECTS; j++)
		{
			if (!HM_object[i].used)
			{
				continue;
			}

			HM_collide(i,j);
			HM_collide(j,i);
		}
	}
}

//
// Given a point relative to a cube, this function returns the 3d
// coordinates of that point.
//

void HM_rel_cube_to_world(
		HM_Object *ho,		// The object the cube is a part of...
		SLONG      x_cube,
		SLONG      y_cube,
		SLONG      z_cube,
		float      rel_x,
		float      rel_y,
		float      rel_z,
		float     *world_x,
		float     *world_y,
		float     *world_z)
{
	float wx;
	float wy;
	float wz;

	float len;
	float matrix[9];

	HM_Point *hp_o;
	HM_Point *hp_x;
	HM_Point *hp_y;
	HM_Point *hp_z;

	SLONG index_o;
	SLONG index_x;
	SLONG index_y;
	SLONG index_z;

	ASSERT(WITHIN(x_cube, 0, ho->x_res - 2));
	ASSERT(WITHIN(y_cube, 0, ho->y_res - 2));
	ASSERT(WITHIN(z_cube, 0, ho->z_res - 2));

	//
	// This cube must exist.
	//

	ASSERT(HM_cube_exists(ho, x_cube, y_cube, z_cube));

	index_o = HM_index(ho, x_cube + 0, y_cube + 0, z_cube + 0);
	index_x = HM_index(ho, x_cube + 1, y_cube + 0, z_cube + 0);
	index_y = HM_index(ho, x_cube + 0, y_cube + 1, z_cube + 0);
	index_z = HM_index(ho, x_cube + 0, y_cube + 0, z_cube + 1);

	ASSERT(WITHIN(ho->index[index_o], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_x], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_y], 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->index[index_z], 1, ho->num_points - 1));

	hp_o = &ho->point[ho->index[index_o]];
	hp_x = &ho->point[ho->index[index_x]];
	hp_y = &ho->point[ho->index[index_y]];
	hp_z = &ho->point[ho->index[index_z]];

	//
	// Create a rotation matrix that rotates point 'p' into the space of the cube.
	//

	matrix[0] = hp_x->x - hp_o->x;
	matrix[1] = hp_x->y - hp_o->y;
	matrix[2] = hp_x->z - hp_o->z;

	matrix[3] = hp_y->x - hp_o->x;
	matrix[4] = hp_y->y - hp_o->y;
	matrix[5] = hp_y->z - hp_o->z;

	matrix[6] = hp_z->x - hp_o->x;
	matrix[7] = hp_z->y - hp_o->y;
	matrix[8] = hp_z->z - hp_o->z;

	#if WE_WANT_TO_NORMALISE_THE_MATRIX

	//
	// ASSUME THAT THE MATRIX IS ORTHOGONAL!  Normalise each vector of the matrix
	// 'doubley' so that the space of the box goes from 0 to 1 in each axis..
	//

	len = matrix[0]*matrix[0] + matrix[1]*matrix[1] + matrix[2]*matrix[2];
	len = 1.0F / len;

	matrix[0] *= len;
	matrix[1] *= len;
	matrix[2] *= len;

	len = matrix[3]*matrix[3] + matrix[4]*matrix[4] + matrix[5]*matrix[5];
	len = 1.0F / len;

	matrix[3] *= len;
	matrix[4] *= len;
	matrix[5] *= len;

	len = matrix[6]*matrix[6] + matrix[7]*matrix[7] + matrix[8]*matrix[8];
	len = 1.0F / len;

	matrix[6] *= len;
	matrix[7] *= len;
	matrix[8] *= len;

	#endif

	//
	// Convert from cube-space to world space.
	//
	
	wx = rel_x;
	wy = rel_y;
	wz = rel_z;

/*MDPSX
	MATRIX_MUL_BY_TRANSPOSE(
		matrix,
		wx,
		wy,
		wz);
		*/

	wx += hp_o->x;
	wy += hp_o->y;
	wz += hp_o->z;

	*world_x = wx;
	*world_y = wy;
	*world_z = wz;
}

//
// Does the processing for the enter structure that it part of
// the given HM_Object.
//

void HM_process_bump(HM_Object *ho, HM_Bump *hb)
{
	SLONG i;

	float out_x;
	float out_y;
	float out_z;

	float squash;

	float dx;
	float dy;
	float dz;

	float fx;
	float fy;
	float fz;

	float dist;

	ASSERT(WITHIN(hb->point,    1, ho->num_points - 1));
	ASSERT(WITHIN(hb->hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Point  *hp  = &ho->point[hb->point];
	HM_Object *ho2 = &HM_object[hb->hm_index];
	HM_Point  *hp2;

	MSG_add("Bumping!");

	//
	// Where did this point enter the object?
	//

	HM_rel_cube_to_world(
		ho2,
		hb->cube_x,
		hb->cube_y,
		hb->cube_z,
		hb->rel_x,
		hb->rel_y,
		hb->rel_z,
	   &out_x,
	   &out_y,
	   &out_z);

	e_draw_3d_line(
		hp->x,
		hp->y,
		hp->z,
		out_x,
		out_y,
		out_z);

	//
	// Work out the force on the point.
	//

	dx = out_x - hp->x;
	dy = out_y - hp->y;
	dz = out_z - hp->z;

	dist = dx*dx + dy*dy + dz*dz;

	squash = ho2->elasticity * dist * 0.0003F;

	fx = squash * dx;
	fy = squash * dy;
	fz = squash * dz;

	//
	// Apply the force to the point.
	//

	hp->dx += fx;
	hp->dy += fy;
	hp->dz += fz;

	//
	// Apply an equal but opposite force to the cube.
	//

	for (i = 0; i < HM_NUM_OPP_POINTS; i++)
	{
		ASSERT(WITHIN(hb->opp_point[i], 1, ho2->num_points - 1));

		hp2 = &ho2->point[hb->opp_point[i]];

		hp2->dx -= fx * hb->opp_prop[i];
		hp2->dy -= fy * hb->opp_prop[i];
		hp2->dz -= fz * hb->opp_prop[i];
	}
}

//
// Returns TRUE if the given bump structure is no longer relavent- i.e.
// the bumping point is no longer bumping.
//

SLONG HM_bump_dead(HM_Object *ho, HM_Bump *hb)
{
	SLONG sx;
	SLONG sy;
	SLONG sz;

	float rel_x;
	float rel_y;
	float rel_z;

	ASSERT(WITHIN(hb->point,    1, ho->num_points - 1));
	ASSERT(WITHIN(hb->hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Point  *hp  = &ho->point[hb->point];
	HM_Object *ho2 = &HM_object[hb->hm_index];

	//
	// Check the cube the point entered originally first, as 
	// an optimisation.
	//

	if (HM_is_point_in_cube(
			ho2,
			hp,
			hb->cube_x,
			hb->cube_y,
			hb->cube_z,
		   &rel_x,
		   &rel_y,
		   &rel_z))
	{
		return FALSE;
	}
			

	for (sx = 0; sx < ho2->x_res - 1; sx++)
	for (sy = 0; sy < ho2->y_res - 1; sy++)
	for (sz = 0; sz < ho2->z_res - 1; sz++)
	{
		if (sx == hb->cube_x &&
			sy == hb->cube_y &&
			sz == hb->cube_z)
		{
			//
			// Already checked this cube.
			//
		}
		else
		{
			if (HM_is_point_in_cube(
					ho2,
					hp,
					sx,
					sy,
					sz,
				   &rel_x,
				   &rel_y,
				   &rel_z))
			{
				return FALSE;
			}
		}
	}

	//
	// The point is not inside the object... don't collide any more.
	//

	return TRUE;
}

void HM_collide(UBYTE hm_index1, UBYTE hm_index2)
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG index;

	SLONG sx;
	SLONG sy;
	SLONG sz;

	float dpx;
	float dpy;
	float dpz;
		   
	float fx;
	float fy;
	float fz;

	float rel_x;
	float rel_y;
	float rel_z;

	float last_rel_x;
	float last_rel_y;
	float last_rel_z;

	float along_x;
	float along_y;
	float along_z;

	float along_enter;

	float enter_rel_x;
	float enter_rel_y;
	float enter_rel_z;

	float out_x;
	float out_y;
	float out_z;

	float total_dist;
	float dist;
	float ddist;
	float pdist;
	float squash;
	float wantdist;
	float squaredist;

	SLONG byte;
	SLONG bit;

	HM_Object *ho1;
	HM_Object *ho2;

	HM_Point *hp;
	HM_Point *hp2;
	HM_Bump  *hb;

	#define HM_ALREADY_BYTES 16

	UBYTE already_bumped[HM_ALREADY_BYTES];

	ASSERT(WITHIN(hm_index1, 0, HM_MAX_OBJECTS - 1));
	ASSERT(WITHIN(hm_index2, 0, HM_MAX_OBJECTS - 1));

	ho1 = &HM_object[hm_index1];
	ho2 = &HM_object[hm_index2];

	//
	// Find any points of hypermatter object 1 that are already inside one of
	// the cubes of hypermatter object 2.
	//

	memset((UBYTE*)already_bumped, 0, sizeof(already_bumped));

	for (hb = ho1->bump; hb; hb = hb->next)
	{
		if (hb->hm_index == hm_index2)
		{
			byte = hb->point >> 3;
			bit  = hb->point  & 7;

			ASSERT(WITHIN(byte, 0, HM_ALREADY_BYTES - 1));

			already_bumped[byte] |= 1 << bit;
		}
	}

	//
	// Are any of the points of hypermatter object1 inside one of
	// the cubes of hypermatter object 2?
	//

	for (i = 1; i < ho1->num_points; i++)
	{
		hp = &ho1->point[i];

		if (already_bumped[i >> 3] & (1 << (i & 7)))
		{
			//
			// This point already has a bumped structure for it.
			//

			continue;
		}

		//
		// Go through each square in object 2.
		//

		for (sx = 0; sx < ho2->x_res - 1; sx++)
		for (sy = 0; sy < ho2->y_res - 1; sy++)
		for (sz = 0; sz < ho2->z_res - 1; sz++)
		{
			if (HM_is_point_in_cube(
					ho2,
					hp,
					sx,
					sy,
					sz,
				   &rel_x,
				   &rel_y,
				   &rel_z))
			{
				//
				// Find the last relative position of the point.
				//

				HM_last_point_in_last_cube(
					ho2,
					hp,
					sx,
					sy,
					sz,
				   &last_rel_x,
				   &last_rel_y,
				   &last_rel_z);

				//
				// So where did the point enter the cube?
				//
				
				along_x = float(INFINITY);
				along_y = float(INFINITY);
				along_z = float(INFINITY);

				//
				// Along one of the x-edges?
				//

				if (last_rel_x > 1.0F && rel_x <= 1.0F && !HM_cube_exists(ho2, sx + 1, sy, sz))
				{
					along_x = (1.0F - last_rel_x) / (rel_x - last_rel_x);
				}
				else
				if (last_rel_x < 0.0F && rel_x >= 0.0F && !HM_cube_exists(ho2, sx - 1, sy, sz))
				{
					along_x = (0.0F - last_rel_x) / (rel_x - last_rel_x);
				}

				//
				// Along one of the y-edges?
				//

				if (last_rel_y > 1.0F && rel_y <= 1.0F && !HM_cube_exists(ho2, sx, sy + 1, sz))
				{
					along_y = (1.0F - last_rel_y) / (rel_y - last_rel_y);
				}
				else
				if (last_rel_y < 0.0F && rel_y >= 0.0F && !HM_cube_exists(ho2, sx, sy - 1, sz))
				{
					along_y = (0.0F - last_rel_y) / (rel_y - last_rel_y);
				}

				//
				// Along one of the z-edges?
				//

				if (last_rel_z > 1.0F && rel_z <= 1.0F && !HM_cube_exists(ho2, sx, sy, sz + 1))
				{																		     
					along_z = (1.0F - last_rel_z) / (rel_z - last_rel_z);				     
				}																		     
				else																	     
				if (last_rel_z < 0.0F && rel_z >= 0.0F && !HM_cube_exists(ho2, sx, sy, sz - 1))
				{
					along_z = (0.0F - last_rel_z) / (rel_z - last_rel_z);
				}

				along_enter = float(INFINITY);

				if (along_x < along_enter) {along_enter = along_x;}
				if (along_y < along_enter) {along_enter = along_y;}
				if (along_z < along_enter) {along_enter = along_z;}

				//
				// 'along_enter' is how far along the line from (last_rel to rel) the point
				// entered the cube.
				//

				enter_rel_x = last_rel_x + along_enter * (rel_x - last_rel_x);
				enter_rel_y = last_rel_y + along_enter * (rel_y - last_rel_y);
				enter_rel_z = last_rel_z + along_enter * (rel_z - last_rel_z);
				
				//
				// The point outside the cube.
				//

				HM_rel_cube_to_world(
					ho2,
					sx,
					sy,
					sz,
					enter_rel_x,
					enter_rel_y,
					enter_rel_z,
				   &out_x,
				   &out_y,
				   &out_z);

				//
				// Find the HM_NUM_OPP_POINTS points of 'ho2' that are nearest (out_x,out_y,out_z).
				//

				SLONG opp_num;
				SLONG opp_point[HM_NUM_OPP_POINTS];
				float opp_dist [HM_NUM_OPP_POINTS];

				dpx = ho2->point[1].x - out_x;
				dpy = ho2->point[1].y - out_y;
				dpz = ho2->point[1].z - out_z;

				dist = dpx*dpx + dpy*dpy + dpz*dpz;

				opp_point[0] = 1;
				opp_dist [0] = dist;
				opp_num      = 1;

				for (j = 2; j < ho2->num_points; j++)
				{
					dpx = ho2->point[j].x - out_x;
					dpy = ho2->point[j].y - out_y;
					dpz = ho2->point[j].z - out_z;

					dist = dpx*dpx + dpy*dpy + dpz*dpz;

					for (k = opp_num - 1; k >= 0; k--)
					{
						if (opp_dist[k] > dist)
						{
							if (k + 1 < HM_NUM_OPP_POINTS)
							{
								opp_point[k + 1] = opp_point[k];
								opp_dist [k + 1] = opp_dist [k];
							}
						}
						else
						{
							if (k + 1 < HM_NUM_OPP_POINTS)
							{
								opp_point[k + 1] = j;
								opp_dist [k + 1] = dist;

								break;
							}
						}
					}

					opp_num += 1;

					if (opp_num > HM_NUM_OPP_POINTS)
					{
						opp_num = HM_NUM_OPP_POINTS;
					}
				}

				//
				// Create a new HM_Bump structure for this point.
				//
				
				hb = (HM_Bump *) malloc(sizeof(HM_Bump));

				hb->point    = i;
				hb->hm_index = hm_index2;
				hb->cube_x   = sx;
				hb->cube_y   = sy;
				hb->cube_z   = sz;
				hb->rel_x    = enter_rel_x;
				hb->rel_y    = enter_rel_y;
				hb->rel_z    = enter_rel_z;

				total_dist = 0;

				for (j = 0; j < HM_NUM_OPP_POINTS; j++)
				{
					hb->opp_point[j] = opp_point[j];
					hb->opp_prop [j] = 1.0F / opp_dist [j];

					total_dist += hb->opp_prop[j];
				}

				for (j = 0; j < HM_NUM_OPP_POINTS; j++)
				{
					hb->opp_prop[j] *= (1.0F / total_dist);
				}

				{
					HM_Bump *bb;

					for (bb = ho1->bump; bb; bb = bb->next)
					{
						if (bb->hm_index == hm_index2)
						{
							if (bb->point == i)
							{
								MSG_add("Two that are the same.");
							}
						}
					}
				}

				//
				// Add the structure to the linked list for this object.
				//

				hb->next  = ho1->bump;
				ho1->bump = hb;
			}
			else
			{
			}
		}
	}
}


//
// The gravty...
//

#ifdef	PSX
#define	MATHS_seg_intersect(a,b,c,d,e,f,h,i)	(0)
#endif

#define HM_GRAVITY (-0.01F)

void HM_process()
{
	SLONG i;
	SLONG j;
	SLONG k;

	float ddx;
	float ddy;
	float ddz;

	float squaredist;
	float dpx;
	float dpy;
	float dpz;
	float pdist;
	float wantdist;
	float wantdistx;
	float wantdisty;
	float wantdistz;
	float ddist;
	float ddistx;
	float ddisty;
	float ddistz;
	float squash;

	float fx;
	float fy;
	float fz;

	float gy;
	float av_div;

	HM_Object *ho;
	HM_Point  *hp;
	HM_Point  *hp1;
	HM_Point  *hp2;
	HM_Edge   *he;
	HM_Col    *hc;
	HM_Bump   *hb;
	HM_Bump  **prev;
	HM_Bump   *dead;

	for (i = 0; i < HM_MAX_OBJECTS; i++)
	{
		ho = &HM_object[i];

		if (!ho->used)
		{
			continue;
		}

		//
		// Keyboard control...
		//

		if (Keys[KB_P3] ||
			Keys[KB_P2])
		{
			if (Keys[KB_P3]) {Keys[KB_P3] = 0; ho->elasticity *= 1.05F;}
			if (Keys[KB_P2]) {Keys[KB_P2] = 0; ho->elasticity *= 0.95F;}

			MSG_add("Elasticity = %f", ho->elasticity);
		}

		if (Keys[KB_P6] ||
			Keys[KB_P5])
		{
			if (Keys[KB_P6]) {Keys[KB_P6] = 0; ho->damping += 0.0001F;}
			if (Keys[KB_P5]) {Keys[KB_P5] = 0; ho->damping -= 0.0001F;}

			MSG_add("Damping = %f", ho->damping);
		}

		if (Keys[KB_P1])
		{
			Keys[KB_P1] = 0;
		}

		if (Keys[KB_ASTERISK])
		{
			Keys[KB_ASTERISK] = 0;

			if (ho->friction < 0.5F)
			{
				ho->friction = 0.9F;
			}
			else
			{
				ho->friction = 0.1F;
			}

			MSG_add("Friction = %f", ho->friction);
		}

		if (Keys[KB_PSLASH])
		{
			Keys[KB_PSLASH] = 0;

			if (ho->bounciness > 0.75F)
			{
				ho->bounciness = 0.0F;
			}
			else
			if (ho->bounciness > 0.25F)
			{
				ho->bounciness = 1.0F;
			}
			else
			{
				ho->bounciness = 0.5F;
			}

			MSG_add("Bounciness = %f", ho->bounciness);
		}

		if (Keys[KB_P7])
		{
			ho->point[1].dy = 40.0F * -HM_GRAVITY * ho->x_res;
		}

		if (Keys[KB_P4])
		{
			ho->point[2].dx += 25.0F * -HM_GRAVITY * ho->x_res;
		}

		//
		// Fast processing via the edge list.
		//

		for (j = 0; j < ho->num_edges; j++)
		{
			he = &ho->edge[j];

			ASSERT(WITHIN(he->p1, 0, ho->num_points - 1));
			ASSERT(WITHIN(he->p2, 0, ho->num_points - 1));

			hp1 = &ho->point[he->p1];
			hp2 = &ho->point[he->p2];

			dpx = hp2->x - hp1->x;
			dpy = hp2->y - hp1->y;
			dpz = hp2->z - hp1->z;

			squaredist  = dpx*dpx;
			squaredist += dpy*dpy;
			squaredist += dpz*dpz;

			if (squaredist > 0.001F)
			{
				ASSERT(WITHIN(he->len, 0, ho->num_sizes - 1));

				pdist    = squaredist;											
				wantdist = ho->size[he->len];
				ddist    = pdist - wantdist;
				squash   = ddist * ho->elasticity * ho->oversize[he->len];

				//
				// Equal and opposite force between the two points.. what if they
				// have different mass, though?
				//

				fx = dpx * squash;
				fy = dpy * squash;
				fz = dpz * squash;

				hp1->dx += fx;
				hp1->dy += fy;
				hp1->dz += fz;

				hp2->dx -= fx;
				hp2->dy -= fy;
				hp2->dz -= fz;
			}
			else
			{
				MSG_add("Points too close together.");
			}
		}

		//
		// Collision processing of the bump list.
		//

		for (hb = ho->bump; hb; hb = hb->next)
		{
			HM_process_bump(ho, hb);
		}

		//
		// Do the movement of all the points
		//

		for (j = 1; j < ho->num_points; j++)
		{
			hp = &ho->point[j];

			//
			// Damping.
			//

			hp->dx *= ho->damping;
			hp->dy *= ho->damping;
			hp->dz *= ho->damping;

			//
			// Gravity should constant on all points... surely!
			//

			hp->dy += HM_GRAVITY * hp->mass;	// WE CAN PRECALCULATE THIS MULTIPLY!!!

			//
			// If the point is underground, then bring it to the surface.
			//

			{
				gy = 0;

				if (hp->y < gy)
				{
					//
					// Bounciness and friction.
					//

					hp->dy  = -hp->dy * ho->bounciness;
					hp->dx *=  ho->friction;
					hp->dz *=  ho->friction;
					hp->y   =  gy - hp->y;
				}
			}

			//
			// Does this point collide?
			//

			for (k = 0; k < ho->num_cols; k++)
			{
				hc = &ho->col[k];

				if (MATHS_seg_intersect(
						SLONG(hc->x1), SLONG(hc->z1),
						SLONG(hc->x2), SLONG(hc->z2),
						SLONG(hp->x),
						SLONG(hp->z),
						SLONG(hp->x + hp->dx + hp->dx),
						SLONG(hp->z + hp->dz + hp->dz)))
				{
					if (hc->x1 == hc->x2)
					{
						hp->dx = -hp->dx;
					}
					else
					if (hc->z1 == hc->z2)
					{
						hp->dz = -hp->dz;
					}
					else
					{
						hp->dx = -hp->dx;
						hp->dz = -hp->dz;
					}

					MSG_add("Point collided.");

					goto dont_move_this_point;
				}
			}

			hp->x  += hp->dx;
			hp->y  += hp->dy;
			hp->z  += hp->dz;

		  dont_move_this_point:;
		}

		//
		// Can we get rid of any of the bumps?
		//

		prev = &ho->bump;
		hb   =  ho->bump;

		while(hb)
		{
			if (HM_bump_dead(ho, hb))
			{
				dead = hb;

				//
				// Take this bump out of the linked list.
				//

			   *prev = hb->next;
			    hb   = hb->next;

				free(dead);
			}
			else
			{
				//
				// Go onto the next one.
				//

				prev = &hb->next;
				hb   =  hb->next;
			}
		}
	}
}


void HM_draw(void)
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG  index;
	SLONG nindex;

	SLONG r;
	SLONG g;
	SLONG b;

	float px[4];
	float py[4];
	float pz[4];

	HM_Object *ho;
	HM_Point  *hp;
	HM_Point  *np;

	PrimObject *po;
	PrimFace4  *f4;

	for (i = 0; i < HM_MAX_OBJECTS; i++)
	{
		ho = &HM_object[i];

		if (!ho->used)
		{
			continue;
		}

		if (Keys[KB_5])
		{
			//
			// Draw the actual mesh.
			//

			ASSERT(WITHIN(ho->prim, 1, next_prim_object - 1));

			po = &prim_objects[ho->prim];

			for (j = po->StartFace4; j < po->EndFace4; j++)
			{
				f4 = &prim_faces4[j];

				for (k = 0; k < 4; k++)
				{
					HM_find_mesh_point(i, f4->Points[k] - po->StartPoint, &px[k], &py[k], &pz[k]);
				}

				e_draw_3d_line_col_sorted((SLONG) px[0], (SLONG) py[0], (SLONG) pz[0], (SLONG) px[1], (SLONG) py[1], (SLONG) pz[1], 255, 255, 255);
				e_draw_3d_line_col_sorted((SLONG) px[1], (SLONG) py[1], (SLONG) pz[1], (SLONG) px[3], (SLONG) py[3], (SLONG) pz[3], 255, 255, 255);
				e_draw_3d_line_col_sorted((SLONG) px[3], (SLONG) py[3], (SLONG) pz[3], (SLONG) px[2], (SLONG) py[2], (SLONG) pz[2], 255, 255, 255);
				e_draw_3d_line_col_sorted((SLONG) px[2], (SLONG) py[2], (SLONG) pz[2], (SLONG) px[0], (SLONG) py[0], (SLONG) pz[0], 255, 255, 255);
			}
		}

		index = 0;

		for (z = 0; z < ho->z_res; z++)
		{
			for (y = 0; y < ho->y_res; y++)
			{
				for (x = 0; x < ho->x_res; x++)
				{
					if (ho->index[index])
					{
						ASSERT(WITHIN(ho->index[index], 1, ho->num_points - 1));

						hp = &ho->point[ho->index[index]];

						x1 = SLONG(hp->x - hp->dx * 0.5F);
						y1 = SLONG(hp->y - hp->dy * 0.5F);
						z1 = SLONG(hp->z - hp->dz * 0.5F);

						if (z && (x == 0 || x == ho->x_res - 1 || y == 0 || y == ho->y_res - 1))
						{
							r = 32;
							g = 32;
							b = 32;

							if (x == 0 || x == ho->x_res - 1) {r = 244 - x * 24;}
							else
							if (y == 0 || y == ho->y_res - 1) {g = 244 - y * 24;}

							nindex = index - ho->x_res_times_y_res;

							ASSERT(WITHIN(nindex, 0, ho->num_indices - 1));

							if (ho->index[nindex])
							{
								ASSERT(WITHIN(ho->index[nindex], 1, ho->num_points - 1));

								np = &ho->point[ho->index[nindex]];

								x2 = SLONG(np->x - np->dx * 0.5F);
								y2 = SLONG(np->y - np->dy * 0.5F);
								z2 = SLONG(np->z - np->dz * 0.5F);

								e_draw_3d_line_col_sorted(
									x1, y1, z1,
									x2, y2, z2,
									r, g, b);
							}
						}

						if (y && (x == 0 || x == ho->x_res - 1 || z == 0 || z == ho->y_res - 1))
						{
							r = 32;
							g = 32;
							b = 32;

							if (x == 0 || x == ho->x_res - 1) {r = 244 - x * 24;}
							else
							if (z == 0 || z == ho->z_res - 1) {b = 244 - z * 24;}

							nindex = index - ho->x_res;

							ASSERT(WITHIN(nindex, 0, ho->num_indices - 1));

							if (ho->index[nindex])
							{
								ASSERT(WITHIN(ho->index[nindex], 1, ho->num_points - 1));

								np = &ho->point[ho->index[nindex]];

								x2 = SLONG(np->x - np->dx * 0.5F);
								y2 = SLONG(np->y - np->dy * 0.5F);
								z2 = SLONG(np->z - np->dz * 0.5F);

								e_draw_3d_line_col_sorted(
									x1, y1, z1,
									x2, y2, z2,
									r, g, b);
							}
						}

						if (x && (y == 0 || y == ho->x_res - 1 || z == 0 || z == ho->y_res - 1))
						{
							r = 32;
							g = 32;
							b = 32;

 							if (y == 0 || y == ho->y_res - 1) {g = 244 - y * 24;}
							else
							if (z == 0 || z == ho->z_res - 1) {b = 244 - z * 24;}

							nindex = index - 1;

							ASSERT(WITHIN(nindex, 0, ho->num_indices - 1));

							if (ho->index[nindex])
							{
								ASSERT(WITHIN(ho->index[nindex], 1, ho->num_points - 1));

								np = &ho->point[ho->index[nindex]];

								x2 = SLONG(np->x - np->dx * 0.5F);
								y2 = SLONG(np->y - np->dy * 0.5F);
								z2 = SLONG(np->z - np->dz * 0.5F);

								e_draw_3d_line_col_sorted(
									x1, y1, z1,
									x2, y2, z2,
									r, g, b);
							}
						}
					}

					index++;
				}
			}
		}
	}
}


void HM_find_mesh_pos(
		UBYTE  hm_index,
		SLONG *x,
		SLONG *y,
		SLONG *z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll)
{
	float len;
	float overlen;

	float matrix[9];

	float ans_x;
	float ans_y;
	float ans_z;

	ASSERT(WITHIN(hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Object *ho = &HM_object[hm_index];
	HM_Point  *hp_o;
	HM_Point  *hp_x;
	HM_Point  *hp_y;
	HM_Point  *hp_z;

	//
	// The points we use...
	//

	ASSERT(WITHIN(ho->o_index, 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->x_index, 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->y_index, 1, ho->num_points - 1));
	ASSERT(WITHIN(ho->z_index, 1, ho->num_points - 1));

	hp_o = &ho->point[ho->o_index];
	hp_x = &ho->point[ho->x_index];
	hp_y = &ho->point[ho->y_index];
	hp_z = &ho->point[ho->z_index];

	//
	// The rows of the matrix.
	//

	matrix[0] = hp_x->x - hp_o->x;
	matrix[1] = hp_x->y - hp_o->y;
	matrix[2] = hp_x->z - hp_o->z;

	matrix[3] = hp_y->x - hp_o->x;
	matrix[4] = hp_y->y - hp_o->y;
	matrix[5] = hp_y->z - hp_o->z;

	matrix[6] = hp_z->x - hp_o->x;
	matrix[7] = hp_z->y - hp_o->y;
	matrix[8] = hp_z->z - hp_o->z;

	//
	// Assume they are orthogonal for now!
	//

	len = matrix[0]*matrix[0] + matrix[1]*matrix[1] + matrix[2]*matrix[2];
	len = sqrt(len);

	overlen = 1.0F / len;

	matrix[0] *= overlen;
	matrix[1] *= overlen;
	matrix[2] *= overlen;

	len = matrix[3]*matrix[3] + matrix[4]*matrix[4] + matrix[5]*matrix[5];
	len = sqrt(len);

	overlen = 1.0F / len;

	matrix[3] *= overlen;
	matrix[4] *= overlen;
	matrix[5] *= overlen;

	len = matrix[6]*matrix[6] + matrix[7]*matrix[7] + matrix[8]*matrix[8];
	len = sqrt(len);

	overlen = 1.0F / len;

	matrix[6] *= overlen;
	matrix[7] *= overlen;
	matrix[8] *= overlen;

	//
	// This matrix in terms of three angles.
	//

	Direction rot;
//MDPSX	Direction rot = MATRIX_find_angles(matrix);

	rot.yaw   *= 2048.0F / (2.0F * PI);
	rot.pitch *= 2048.0F / (2.0F * PI);
	rot.roll  *= 2048.0F / (2.0F * PI);

	*yaw   = SLONG(rot.yaw);
	*pitch = SLONG(rot.pitch);
	*roll  = SLONG(rot.roll);

	//
	// Unrotate the origin into prim space.
	//

	ans_x  = ho->o_prim_x;
	ans_y  = ho->o_prim_y;
	ans_z  = ho->o_prim_z;

	ans_x -= ho->cog_x;
	ans_y -= ho->cog_y;
	ans_z -= ho->cog_z;

/*
//MDPSX
	MATRIX_MUL_BY_TRANSPOSE(
		matrix,
		ans_x,
		ans_y,
		ans_z);
		*/

	ans_x += ho->cog_x;
	ans_y += ho->cog_y;
	ans_z += ho->cog_z;

	*x = SLONG(hp_o->x - ans_x);
	*y = SLONG(hp_o->y - ans_y);
	*z = SLONG(hp_o->z - ans_z);
}



SLONG HM_stationary(UBYTE hm_index)
{
	SLONG i;

	float dx = 0;
	float dy = 0;
	float dz = 0;

	ASSERT(WITHIN(hm_index, 0, HM_MAX_OBJECTS - 1));

	HM_Object *ho = &HM_object[hm_index];

	for (i = 1; i < ho->num_points; i++)
	{
		dx += fabs(ho->point[i].dx);
		dy += fabs(ho->point[i].dy);
		dz += fabs(ho->point[i].dz);
	}

	dx /= ho->num_points;
	dy /= ho->num_points;
	dz /= ho->num_points;

	if (dx + dy + dz < 0.1F)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#endif
