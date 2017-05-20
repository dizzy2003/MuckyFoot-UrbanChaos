//
// Cones clipped by planar polygons.
//

#include "game.h"
#include "poly.h"
#include "cone.h"
#include "pap.h"
#include "supermap.h"
#include <math.h>
#include "memory.h"


#ifdef TARGET_DC
// intrinsic maths
#include <shsgintr.h>
#endif

//
// The origin of the cone.
//

float CONE_origin_x;
float CONE_origin_y;
float CONE_origin_z;
ULONG CONE_origin_colour;
float CONE_end_x;
float CONE_end_y;
float CONE_end_z;

//
// The points around the base of the cone.
//

typedef struct
{
	float      x;
	float      y;
	float      z;
	ULONG      colour;
	POLY_Point pp;

} CONE_Point;

#define CONE_MAX_POINTS 64

CONE_Point CONE_point[CONE_MAX_POINTS];
SLONG      CONE_point_upto;

void CONE_create(
		float x,
		float y,
		float z,
		float dx,
		float dy,
		float dz,
		float length,
		float radius,
		ULONG colour_start,
		ULONG colour_end,
		SLONG detail)
{
	SLONG i;

	float ax;
	float ay;
	float az;

	float bx;
	float by;
	float bz;

	float px;
	float py;
	float pz;

	float dist;

	//
	// The origin of the cone.
	//

	CONE_origin_x      = x;
	CONE_origin_y      = y;
	CONE_origin_z      = z;
	CONE_origin_colour = colour_start;

	//
	// Normalise (dx,dy,dz).
	//

#ifdef TARGET_DC
	dist = dx*dx + dy*dy + dz*dz;
	dist = _InvSqrtA(dist);

	dx = dx * dist;
	dy = dy * dist;
	dz = dz * dist;
#else
	dist = dx*dx + dy*dy + dz*dz;
	dist = sqrt(dist);

	dx = dx * (1.0F / dist);
	dy = dy * (1.0F / dist);
	dz = dz * (1.0F / dist);
#endif

	//
	// Construct two vectors orthogonal to (dx,dy,dz) and to eachother.
	//

	px = dy;
	py = dz;
	pz = dx;

	//
	// (px,py,pz) is not paralell to (dx,dy,dz) so (d x p) will be
	// orthogonal to (dx,dy,dz)
	//

	ax = dy*pz - dz*py;
	ay = dz*px - dx*pz;
	az = dx*py - dy*px;

	//
	// Create a vector parallel to both a and d.
	//

	bx = dy*az - dz*ay;
	by = dz*ax - dx*az;
	bz = dx*ay - dy*ax;

	//
	// The number of points around the edge of the base depends on 'detail'...
	// Always use at least 4 points.
	//

	ASSERT(WITHIN(detail, 0, 256));

	CONE_point_upto  = 4;
	CONE_point_upto += detail * (CONE_MAX_POINTS - 4) >> 8;

	//
	// The midpoints of the circle at the end of the cone.
	//

	CONE_end_x = CONE_origin_x + dx * length;
	CONE_end_y = CONE_origin_y + dy * length;
	CONE_end_z = CONE_origin_z + dz * length;

	//
	// Build the bottom points of the cone.
	//

	float angle;
	float along_a;
	float along_b;

	CONE_Point *cp;

	for (i = 0; i < CONE_point_upto; i++)
	{
		cp = &CONE_point[i];

		angle   = float(i) * (2.0F * PI / float(CONE_point_upto));
		along_a = cos(angle) * radius;
		along_b = sin(angle) * radius;

		cp->x      = CONE_end_x + along_a * ax + along_b * bx;
		cp->y      = CONE_end_y + along_a * ay + along_b * by;
		cp->z      = CONE_end_z + along_a * az + along_b * bz;
		cp->colour = colour_end;
	}
}



ULONG CONE_interpolate_colour(float v, ULONG colour1, ULONG colour2)
{
	SLONG rb1, rb2, drb, rba;
	SLONG ga1, ga2, dga, gaa;

	union
	{
		float vfloat;
		ULONG vfixed8;
	};

	SLONG answer;

	//
	// Early outs.
	//

	if (colour1 == colour2) {return colour1;}
	
	if (v < 0.01F) {return colour1;}
	if (v > 0.99F) {return colour2;}

	//
	// Work out how much to interpolate along in fixed point 8.
	//

	vfloat   = 32768.0F + v;
	vfixed8 &= 0xff;

	//
	// Red and blue.
	//

	rb1 = colour1 & (0x00ff00ff);
	rb2 = colour2 & (0x00ff00ff);

	if (rb1 != rb2)
	{
		//
		// Do interpolation of red and blue simultaneously.
		//

		drb   = rb2 - rb1;
		drb  *= vfixed8;
		drb >>= 8;
		rba   = rb1 + drb;
		rba  &= 0x00ff00ff;
	}
	else
	{
		//
		// No need to interpolated red and blue.
		//

		rba = rb1;
	}

	//
	// Green and alpha.
	//
	
	ga1 = colour1 >> 8;
	ga2 = colour2 >> 8;

	ga1 &= 0x00ff00ff;
	ga2 &= 0x00ff00ff;

	if (ga1 != ga2)
	{
		//
		// Do interpolationg of red and blue simultaneously.
		//

		dga  = ga2 - ga1;
		dga *= vfixed8;
		gaa  = (ga1 << 8) + dga;
		gaa &= 0xff00ff00;
	}
	else
	{
		//
		// No need to interpolate green and alpha.
		//

		gaa = ga1 << 8;
	}

	answer = rba | gaa;

	return answer;
}


//
// Inserts two new points in the cone after the given point at the given
// position.
//

void CONE_insert_points(
		SLONG point,
		float x1,
		float y1,
		float z1,
		ULONG colour1,
		float x2,
		float y2,
		float z2,
		ULONG colour2)
{
	SLONG i;

	ASSERT(WITHIN(point, 0, CONE_point_upto - 1));

	if (CONE_point_upto >= CONE_MAX_POINTS - 1)
	{
		//
		// We already have the maximum number of point.
		//

		return;
	}

	for (i = CONE_point_upto + 1; i - 2 >= point; i--)
	{
		CONE_point[i] = CONE_point[i - 2];
	}

	CONE_point[point + 0].x      = x1;
	CONE_point[point + 0].y      = y1;
	CONE_point[point + 0].z      = z1;
	CONE_point[point + 0].colour = colour1;

	CONE_point[point + 1].x      = x2;
	CONE_point[point + 1].y      = y2;
	CONE_point[point + 1].z      = z2;
	CONE_point[point + 1].colour = colour2;

	CONE_point_upto += 2;
}



void CONE_clip(
		CONE_Poly p[],
		SLONG     num_points)
{
	SLONG i;
	SLONG j;

	SLONG p1;
	SLONG p2;

	float av;
	float aw;

	float bv;
	float bw;

	float along;

	float along_v;
	float along_w;

	float len_v;
	float len_w;

	float overlen_v;
	float overlen_w;

	float dpc;

	float px;
	float py;
	float pz;

	float dx;
	float dy;
	float dz;
	float dist;
	float dist_want;
	float dist_mul;

	float ix;
	float iy;
	float iz;
	ULONG icolour;

	float insert_x1;
	float insert_y1;
	float insert_z1;
	ULONG insert_colour1;

	float insert_x2;
	float insert_y2;
	float insert_z2;
	ULONG insert_colour2;

	float dprod;

	CONE_Poly  *pp;
	CONE_Point *cp;

	#define CONE_MAX_POLY_POINTS 16

	struct
	{
		float along_v;
		float along_w;

	} along_p[CONE_MAX_POLY_POINTS];

	struct
	{
		UBYTE failed_side;
		UBYTE failed_dprod;
		UWORD shit;
		float ix;
		float iy;
		float iz;
		ULONG icolour;
		float dprod[CONE_MAX_POLY_POINTS];
		
	} point_info[2],
	 *point_info_now,
	 *point_info_then,
	 *point_info_spare;

	#define CONE_SWAP_POINT_INFO(a,b) {point_info_spare = (a); (a) = (b); (b) = point_info_spare;}
	
	point_info_now  = &point_info[0];
	point_info_then = &point_info[1];

	ASSERT(WITHIN(num_points, 3, CONE_MAX_POLY_POINTS));

	//
	// Find the plane of the polygon. Defined by the position of point 0, and
	// two vectors from point 0 to point 1 and point 0 to point (num_points - 1).
	//

	CONE_Poly *vp = &p[1];
	CONE_Poly *wp = &p[num_points - 1];

	float vx = vp->x - p[0].x;
	float vy = vp->y - p[0].y;
	float vz = vp->z - p[0].z;

	float wx = wp->x - p[0].x;
	float wy = wp->y - p[0].y;
	float wz = wp->z - p[0].z;
	
	len_v = sqrt(vx*vx + vy*vy + vz*vz);
	len_w = sqrt(wx*wx + wy*wy + wz*wz);

	overlen_v = 1.0F / len_v;
	overlen_w = 1.0F / len_w;

	//
	// The normal of the plane.
	// 

	float nx = vy * wz - vz * wy;
	float ny = vz * wx - vx * wz;
	float nz = vx * wy - vy * wx;

	//
	// Which side of the plane is the origin of the cone?
	//

	float ox  = CONE_origin_x - p[0].x;
	float oy  = CONE_origin_y - p[0].y;
	float oz  = CONE_origin_z - p[0].z;

	float dpo = ox*nx + oy*ny + oz*nz;

	if (dpo >= 0.0F)
	{
		//
		// The origin is on the back side of the quad.
		//

		return;
	}

	//
	// Find all the points of the polyon in terms of the
	// two vectors v and w.
	//

	along_p[0].along_v = 0.0F;
	along_p[0].along_w = 0.0F;

	along_p[1].along_v = len_v;
	along_p[1].along_w = 0.0F;

	along_p[num_points - 1].along_v = 0.0F;
	along_p[num_points - 1].along_w = len_w;

	for (i = 2; i < num_points - 1; i++)
	{
		pp = &p[i];

		px = pp->x - p[0].x;
		py = pp->y - p[0].y;
		pz = pp->z - p[0].z;

		along_p[i].along_v = (px*vx + py*vy + pz*vz) * overlen_v;
		along_p[i].along_w = (px*wx + py*wy + pz*wz) * overlen_w;
	}

	//
	// Check all the points of the cone.  If the line connecting two points
	// crosses over the edge of a polygon, then create a point at the intersection.
	//

	point_info_then->failed_side = TRUE;

	for (i = 0; i < CONE_point_upto; i++)
	{
		cp = &CONE_point[i];

		px = cp->x - p[0].x;
		py = cp->y - p[0].y;
		pz = cp->z - p[0].z;

		dpc = px*nx + py*ny + pz*nz;

		if (dpc <= 0.0F)
		{
			//
			// This point is on the front side of the quad.
			//

			point_info_now->failed_side = TRUE;
		}
		else
		{
			point_info_now->failed_side = FALSE;

			//
			// The origin of the cone is on the front of the quad and this
			// point is on the back of the quad.  How far along the line
			// is the intersection with the plane?
			//

			along = dpo / (dpo - dpc);

			ASSERT(WITHIN(along, 0.0F, 1.0F));

			//
			// The intersection point.
			//

			ix      = CONE_origin_x + along * (cp->x - CONE_origin_x);
			iy      = CONE_origin_y + along * (cp->y - CONE_origin_y);
			iz      = CONE_origin_z + along * (cp->z - CONE_origin_z);
			icolour = CONE_interpolate_colour(along, CONE_origin_colour, cp->colour);

			point_info_now->ix      = ix;
			point_info_now->iy      = iy;
			point_info_now->iz      = iz;
			point_info_now->icolour = icolour;

			//
			// Find the intersection point in terms of the two vectors
			// v and w.
			//

			px = ix - p[0].x;
			py = iy - p[0].y;
			pz = iz - p[0].z;

			along_v = (px*vx + py*vy + pz*vz) * overlen_v;
			along_w = (px*wx + py*wy + pz*wz) * overlen_w;

			//
			// Is this point inside the polygon? We can optimise here because
			// the first and last points (av,aw)s are easy constants. (0s and 1s)
			//

			for (j = 0; j < num_points; j++)
			{
				p1 = j + 0;
				p2 = j + 1;

				if (p2 == num_points) {p2 = 0;}

				av = along_p[p2].along_v - along_p[p1].along_v;
				aw = along_p[p2].along_w - along_p[p1].along_w;

				bv = along_v - along_p[p1].along_v;
				bw = along_w - along_p[p1].along_w;

				dprod = av*bv + aw*bw;

				//
				// Remember the dprods WRT each line of the poly so we can create
				// a point on the intersection of each line.
				//

				point_info_now->dprod[j] = dprod;

				if (dprod < 0.0F)
				{
					//
					// This ray does not intersect the polygon.
					//

					point_info_now->failed_dprod = j;

					if (point_info_then->failed_side  == FALSE &&
						point_info_then->failed_dprod == num_points)
					{
						//
						// The last point did intersect the polygon. We must
						// create a point along the edge of the polygon.
						//

						along = point_info_then->dprod[j] / (point_info_then->dprod[j] - dprod);

						insert_x1      = point_info_then->ix + along * (ix - point_info_then->ix);
						insert_y1      = point_info_then->iy + along * (iy - point_info_then->iy);
						insert_z1      = point_info_then->iz + along * (iz - point_info_then->iz);
						insert_colour1 = CONE_interpolate_colour(along, point_info_then->icolour, icolour);

						//
						// Create another point at the end of the ray...
						//

						dx = cp->x - CONE_origin_x;
						dy = cp->y - CONE_origin_y;
						dz = cp->z - CONE_origin_z;

						dist_want = sqrt(dx*dx + dy*dy + dz*dz);

						dx = insert_x1 - CONE_origin_x;
						dy = insert_y1 - CONE_origin_y;
						dz = insert_z1 - CONE_origin_z;

						dist      = sqrt(dx*dx + dy*dy + dz*dz);

						dist_mul = dist_want / dist;

						dx *= dist_mul;
						dy *= dist_mul;
						dz *= dist_mul;

						insert_x2      = CONE_origin_x + dx;
						insert_y2      = CONE_origin_y + dy;
						insert_z2      = CONE_origin_z + dz;
						insert_colour2 = cp->colour;

						CONE_insert_points(
							i,
							insert_x1,
							insert_y1,
							insert_z1,
							insert_colour1,
							insert_x2,
							insert_y2,
							insert_z2,
							insert_colour2);

						//
						// Skip over the inserted points...
						//

						i += 2;
					}

					goto point_outside_polygon;
				}
			}

			//
			// Make this ray finish at the intersection with this polygon.
			//

			point_info_now->failed_dprod = num_points;

			cp->x      = ix;
			cp->y      = iy;
			cp->z      = iz;
			cp->colour = icolour;

			if (point_info_then->failed_side  == FALSE &&
				point_info_then->failed_dprod  < num_points)
			{
				//
				// The last point was not in the polygon, so create an intersection
				// point between this point and the last one.
				//

				along =	point_info_then->dprod[point_info_then->failed_dprod] / (point_info_then->dprod[point_info_then->failed_dprod] - point_info_now->dprod[point_info_then->failed_dprod]);

				insert_x2      = point_info_then->ix + along * (ix - point_info_then->ix);
				insert_y2      = point_info_then->iy + along * (iy - point_info_then->iy);
				insert_z2      = point_info_then->iz + along * (iz - point_info_then->iz);
				insert_colour2 = CONE_interpolate_colour(along, point_info_then->icolour, icolour);

				//
				// Create another point at the end of the ray...
				//

				ASSERT(WITHIN(i, 1, CONE_point_upto));

				CONE_Point *lp = &CONE_point[i - 1];

				dx = lp->x - CONE_origin_x;
				dy = lp->y - CONE_origin_y;
				dz = lp->z - CONE_origin_z;

				dist_want = sqrt(dx*dx + dy*dy + dz*dz);

				dx = insert_x2 - CONE_origin_x;
				dy = insert_y2 - CONE_origin_y;
				dz = insert_z2 - CONE_origin_z;

				dist      = sqrt(dx*dx + dy*dy + dz*dz);

				dist_mul = dist_want / dist;

				dx *= dist_mul;
				dy *= dist_mul;
				dz *= dist_mul;

				insert_x1      = CONE_origin_x + dx;
				insert_y1      = CONE_origin_y + dy;
				insert_z1      = CONE_origin_z + dz;
				insert_colour1 = lp->colour;

				CONE_insert_points(
					i,
					insert_x1,
					insert_y1,
					insert_z1,
					insert_colour1,
					insert_x2,
					insert_y2,
					insert_z2,
					insert_colour2);

				//
				// Skip over the inserted points.
				//

				i += 2;
			}

		  point_outside_polygon:;
		}

		CONE_SWAP_POINT_INFO(
			point_info_now,
			point_info_then);
	}
}

//
// Intersects the cone with colvects and walkable faces on the
// given square.
//

#define CONE_COLVECT_DONE 4

SLONG CONE_colvect_done[CONE_COLVECT_DONE];
SLONG CONE_colvect_done_upto;

void CONE_intersect_square(
		SLONG mx,
		SLONG mz)
{
	SLONG i;

	SLONG x1;
	SLONG z1;

	SLONG w_list;
	SLONG w_face;

	PrimFace4 *p_f4;
	PrimPoint *pp;

	SLONG f_list;
	SLONG exit;
	SLONG facet;
	SLONG build;

	DFacet *df;

	SLONG face_height;
	UBYTE face_order[4] = {0,1,3,2};

	Thing *p_fthing;

	CONE_Poly poly[4];

	if (!WITHIN(mx, 0, PAP_SIZE_LO - 1) ||
		!WITHIN(mz, 0, PAP_SIZE_LO - 1))
	{
		return;
	}

	PAP_Lo *pl = &PAP_2LO(mx,mz);

	f_list = pl->ColVectHead;

	if (f_list)
	{
		exit = FALSE;

		while(!exit)
		{
			facet = facet_links[f_list];

			ASSERT(facet);

			if (facet < 0)
			{
				facet = -facet;
				exit  =  TRUE;
			}
			
			//
			// Have we done this facet already?
			//

			for (i = 0; i < CONE_COLVECT_DONE; i++)
			{
				if (CONE_colvect_done[i] == facet)
				{
					//
					// Dont do this facet again
					//

					goto ignore_this_facet;
				}
			}

			//
			// Remember this facet.
			//

			ASSERT(WITHIN(CONE_colvect_done_upto, 0, CONE_COLVECT_DONE - 1));
			ASSERT(CONE_COLVECT_DONE == 4 || CONE_COLVECT_DONE == 8);

			CONE_colvect_done[CONE_colvect_done_upto] = facet;
			CONE_colvect_done_upto += 1;
			CONE_colvect_done_upto &= CONE_COLVECT_DONE - 1;

			//
			// The poly covering the facet.
			//

			df = &dfacets[facet];

			poly[0].x = float(df->x[1] << 8);
			poly[0].y = float(df->Y[1]     );
			poly[0].z = float(df->z[1] << 8);

			poly[1].x = float(df->x[1] << 8);
			poly[1].y = float(df->Y[1]      + BLOCK_SIZE * df->Height);
			poly[1].z = float(df->z[1] << 8);

			poly[2].x = float(df->x[0] << 8);
			poly[2].y = float(df->Y[0]      + BLOCK_SIZE * df->Height);
			poly[2].z = float(df->z[0] << 8);

			poly[3].x = float(df->x[0] << 8);
			poly[3].y = float(df->Y[0]     );
			poly[3].z = float(df->z[0] << 8);

			if (df->FacetType == STOREY_TYPE_NORMAL_FOUNDATION)
			{
				//
				// Foundations go down deep into the ground...
				//

				poly[0].y -= 256.0F;
				poly[3].y -= 256.0F;
			}

			//
			// Clip the cone to this colvect.
			//

			CONE_clip(poly, 4);


		  ignore_this_facet:;

			f_list++;
		}
	}



	/*

	//
	// The walkable faces.
	//

	w_list = me->Walkable;

	while(w_list)
	{
		w_face = walk_links[w_list].Face;

		if (w_face > 0)
		{
			p_f4 = &prim_faces4[w_face];

			//
			// Find the thing this face is a part of!!!
			//

			wall = p_f4->ThingIndex;

			if (wall < 0)
			{
				storey   = wall_list[-wall].StoreyHead;
				building = storey_list[storey].BuildingHead;
				thing    = building_list[building].ThingIndex;
				p_fthing = TO_THING(thing);

				poly[0].x  = float(p_fthing->WorldPos.X >> 8);
				poly[0].y  = float(p_fthing->WorldPos.Y >> 8);
				poly[0].z  = float(p_fthing->WorldPos.Z >> 8);

				poly[1]    = poly[0];
				poly[2]    = poly[0];
				poly[3]    = poly[0];

				for (i = 0; i < 4; i++)
				{
					pp = &prim_points[p_f4->Points[face_order[i]]];

					poly[i].x += float(pp->X);
					poly[i].y += float(pp->Y);
					poly[i].z += float(pp->Z);
				}

				//
				// Clip the cone to this walkable face.
				//

				CONE_clip(poly, 4);
			}
		}

		w_list = walk_links[w_list].Next;
	}

	*/
}

void CONE_intersect_with_map()
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG dx;
	SLONG dz;

	SLONG lx;
	SLONG lz;

	SLONG mx;
	SLONG mz;

	SLONG len;
	SLONG steps;

	//
	// Make sure we dont do the same mapsquare more than once.
	//

	#define CONE_MAX_DONE 4

	struct
	{
		SLONG x;
		SLONG z;

	}     done[CONE_MAX_DONE] = {{-1,-1},{-1,-1},{-1,-1},{-1,-1}};
	SLONG done_upto = 0;

	//
	// Make sure we dont do a colvect more than once.
	//

	for (i = 0; i < CONE_COLVECT_DONE; i++)
	{
		CONE_colvect_done[i] = -1;
	}

	CONE_colvect_done_upto = 0;

	x1 = SLONG(CONE_origin_x);
	z1 = SLONG(CONE_origin_z);

	x2 = SLONG(CONE_end_x);
	z2 = SLONG(CONE_end_z);

	dx = x2 - x1;
	dz = z2 - z1;

	len = QDIST2(abs(dx),abs(dz)) + 1;

	//
	// Four steps per lo-res mapsquare.
	// 

	dx = (dx << 8) / len;
	dz = (dz << 8) / len;

	steps  = len >> 8;
	steps += 1;

	x  =  x1;
	z  =  z1;

	lx = -dz;
	lz =  dx;

	for (i = 0; i < steps; i++)
	{
		for (j = 0; j < 3; j++)
		{
			mx = x - lx + lx * j >> PAP_SHIFT_LO;
			mz = z - lz + lz * j >> PAP_SHIFT_LO;

			//
			// Have we done this mapsquare before?
			//

			for (k = 0; k < CONE_MAX_DONE; k++)
			{
				if (done[k].x == mx &&
					done[k].z == mz)
				{
					goto already_done_this_square;
				}
			}

			//
			// Remember this square.
			//

			ASSERT(WITHIN(done_upto, 0, CONE_MAX_DONE - 1));

			done[done_upto].x = mx;
			done[done_upto].z = mz;

			ASSERT(CONE_MAX_DONE == 4 || CONE_MAX_DONE == 8);

			done_upto += 1;
			done_upto &= CONE_MAX_DONE - 1;

			//
			// Intersect the cone with stuff one this mapsquare.
			//

			CONE_intersect_square(mx,mz);

		  already_done_this_square:;
		}

		x += dx;
		z += dz;
	}
}


void CONE_draw()
{
	SLONG i;

	SLONG p1;
	SLONG p2;

	POLY_Point ppo;

	POLY_Point *tri[3];

	CONE_Point *cp;
	CONE_Point *cp1;
	CONE_Point *cp2;

	//
	// Rotate the origin of the cone.
	//

	POLY_transform(
		CONE_origin_x,
		CONE_origin_y,
		CONE_origin_z,
	   &ppo);

	if (!ppo.IsValid())
	{
		//
		// If the origin of the cone is behind us, then we are doomed!
		//

		return;
	}

	ppo.u        = 0.0F;
	ppo.v        = 0.0F;
	ppo.colour   = CONE_origin_colour;
	ppo.specular = 0x00000000;

	//
	// Rotate all the points in the cone.
	//

	for (i = 0; i < CONE_point_upto; i++)
	{
		cp = &CONE_point[i];

		POLY_transform(
			cp->x,
			cp->y,
			cp->z,
		   &cp->pp);

		cp->pp.colour   = cp->colour;
		cp->pp.specular = 0x00000000;
		cp->pp.u        = 0.0F;
		cp->pp.v        = 0.0F;
	}

	//
	// Generate the triangles.
	//

	tri[0] = &ppo;

	for (i = 0; i < CONE_point_upto; i++)
	{
		p1 = i + 0;
		p2 = i + 1;

		if (p2 == CONE_point_upto) {p2 = 0;}

		tri[1] = &CONE_point[p1].pp;
		tri[2] = &CONE_point[p2].pp;

		if (POLY_valid_triangle(tri))
		{
			POLY_add_triangle(tri, POLY_PAGE_ADDITIVE, FALSE);
		}
	}
}

