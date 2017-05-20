//
// The landscape- its made up of lots of lines.
//

#include "always.h"
#include "land.h"
#include "log.h"
#include "os.h"



//
// The points of the landscape.
//

typedef struct
{
	float x;
	float y;
	float nx;
	float ny;

} LAND_Point;

#define LAND_MAX_POINTS 256

LAND_Point LAND_point[LAND_MAX_POINTS];
SLONG      LAND_point_upto;



//
// The lines between the points.
//

typedef struct
{
	SLONG p1;
	SLONG p2;
	float nx;
	float ny;

} LAND_Line;

#define LAND_MAX_LINES 256

LAND_Line LAND_line[LAND_MAX_LINES];
SLONG     LAND_line_upto;


//
// The texture we use to draw the landscape.
//

OS_Texture *LAND_ot;



void LAND_init()
{
	//
	// Clear the landscape.
	//

	memset(LAND_point, 0, sizeof(LAND_point));
	memset(LAND_line,  0, sizeof(LAND_line ));

	LAND_point_upto = 0;
	LAND_line_upto  = 0;

	//
	// Load in our texture.
	//

	LAND_ot = OS_texture_create("line.tga");
}


//
// Returns the index of a point at (x1,y1)
//

SLONG LAND_add_point(float x, float y)
{
	SLONG i;

	LAND_Point *lp;

	for (i = 0; i < LAND_point_upto; i++)
	{
		lp = &LAND_point[i];

		if (lp->x == x &&
			lp->y == y)
		{
			return i;
		}
	}

	ASSERT(WITHIN(LAND_point_upto, 0, LAND_MAX_POINTS - 1));

	lp = &LAND_point[LAND_point_upto];

	lp->x = x;
	lp->y = y;

	return LAND_point_upto++;
}


void LAND_add_line(float x1, float y1, float x2, float y2)
{
	LAND_Line *ll;

	ASSERT(WITHIN(LAND_line_upto, 0, LAND_MAX_LINES - 1));

	ll = &LAND_line[LAND_line_upto++];

	ll->p1 = LAND_add_point(x1, y1);
	ll->p2 = LAND_add_point(x2, y2);
}


void LAND_add_line(SLONG num_points, Point2d p[])
{
	SLONG i;

	for (i = 0; i < num_points - 1; i++)
	{
		LAND_add_line(
			p[i + 0].x,
			p[i + 0].y,
			p[i + 1].x,
			p[i + 1].y);
	}
}


void LAND_calc_normals()
{
	SLONG i;
	SLONG j;

	float nx;
	float ny;
	float len;
	float overlen;

	LAND_Line  *ll;
	LAND_Point *lp;
	LAND_Point *lp1;
	LAND_Point *lp2;

	//
	// Work out the normals of all the lines.
	//

	for (i = 0; i < LAND_line_upto; i++)
	{
		ll = &LAND_line[i];

		ASSERT(WITHIN(ll->p1, 0, LAND_point_upto - 1));
		ASSERT(WITHIN(ll->p2, 0, LAND_point_upto - 1));

		lp1 = &LAND_point[ll->p1];
		lp2 = &LAND_point[ll->p2];

		nx = -(lp1->y - lp2->y);
		ny =  (lp1->x - lp2->x);

		len     = sqrt(nx*nx + ny*ny);
		overlen = 1.0F / len;

		ll->nx = nx * overlen;
		ll->ny = ny * overlen;
	}

	//
	// Work out the normals of all the points.
	//

	for (i = 0; i < LAND_point_upto; i++)
	{
		lp = &LAND_point[i];

		//
		// Initialise the normal.
		//

		nx = 0.0F;
		ny = 0.0F;

		for (j = 0; j < LAND_line_upto; j++)
		{
			ll = &LAND_line[j];

			//
			// Does this line use our point?
			//

			if (ll->p1 == i ||
				ll->p2 == i)
			{
				nx += ll->nx;
				ny += ll->ny;
			}
		}

		//
		// Normalised...
		//

		len = sqrt(nx*nx + ny*ny);

		if (len < 0.01F)
		{
			//
			// Oh dear!
			//

			ASSERT(0);

			lp->nx = 1.0F;
			lp->ny = 0.0F;
		}
		else
		{
			overlen = 1.0F / len;

			lp->nx = nx * overlen;
			lp->ny = ny * overlen;
		}
	}
}


SLONG LAND_collide_sphere(
		float  x,
		float  y,
		float  radius,
		float *nx,
		float *ny)
{
	SLONG i;

	float dx;
	float dy;
	float dist;
	float dprod;

	float radius2 = radius * radius;

	LAND_Line  *ll;
	LAND_Point *lp;
	LAND_Point *lp1;
	LAND_Point *lp2;

	SLONG ans = FALSE;

	//
	// Initialise the normal.
	//

   *nx = 0.0F;
   *ny = 0.0F;

	//
	// Check all the points first...
	//

	for (i = 0; i < LAND_point_upto; i++)
	{
		lp = &LAND_point[i];

		dx = lp->x - x;
		dy = lp->y - y;

		dist = dx*dx + dy*dy;

		if (dist < radius2)
		{
			//
			// Collision!
			//

		   *nx += lp->nx;
		   *ny += lp->ny;

			LOG_file("    Collision with point %d normal (%f,%f)\n", i, lp->nx, lp->ny);

			ans = TRUE;
		}
	}

	//
	// Now check all the lines.
	//

	for (i = 0; i < LAND_line_upto; i++)
	{
		ll = &LAND_line[i];

		ASSERT(WITHIN(ll->p1, 0, LAND_point_upto - 1));
		ASSERT(WITHIN(ll->p2, 0, LAND_point_upto - 1));

		lp1 = &LAND_point[ll->p1];
		lp2 = &LAND_point[ll->p2];
		
		dx = x - lp1->x;
		dy = y - lp1->y;

		dist = dx*ll->nx + dy*ll->ny;

		if (fabs(dist) < radius)
		{
			//
			// The circle intersects the infinite line defined by ll->(x1,y1) ll->(x2,y2).
			// Now we must check that it lies upon the line segment.
			//

			dprod = dx * (lp2->x - lp1->x) + dy * (lp2->y - lp1->y);

			if (dprod < 0.0F)
			{
				//
				// Not on the line segment.
				//
			}
			else
			{
				dx = x - lp2->x;
				dy = y - lp2->y;

				dprod = dx * (lp1->x - lp2->x) + dy * (lp1->y - lp2->y);

				if (dprod < 0.0F)
				{
					//
					// Not on the line segment.
					//
				}
				else
				{
					//
					// Collision!
					//

				   *nx += ll->nx;
				   *ny += ll->ny;

					{
						union
						{
							float f;
							int   i;

						} v[2];

						v[0].f = *nx;
						v[1].f = *ny;

						LOG_file("    0: Collision normal = (%f,%f) (%d,%d)\n", v[0].f,v[1].f, v[0].i,v[1].i);
					}

					ans = TRUE;
				}
			}
		}
	}

	if (ans)
	{
		{
			union
			{
				float f;
				int   i;

			} v[2];

			v[0].f = *nx;
			v[1].f = *ny;

			LOG_file("    1: Collision normal = (%f,%f) (%d,%d)\n", v[0].f,v[1].f, v[0].i,v[1].i);
		}


		//
		// Normalise the normal.
		//

		float len     = sqrt(*nx * *nx + *ny * *ny);
		float overlen = 1.0F / len;

	   *nx *= overlen;
	   *ny *= overlen;

 		{
			union
			{
				float f;
				int   i;

			} v[2];

			v[0].f = *nx;
			v[1].f = *ny;

			LOG_file("    2: Collision normal = (%f,%f) (%d,%d)\n", v[0].f,v[1].f, v[0].i,v[1].i);
		}


		LOG_file("    Collision normal = (%f,%f)\n", *nx, *ny);
	}

	return ans;
}


void LAND_draw_all(float mid_x, float mid_y, float zoom)
{
	SLONG i;

	LAND_Line  *ll;
	LAND_Point *lp1;
	LAND_Point *lp2;

	OS_Buffer *ob = OS_buffer_new();

	for (i = 0; i < LAND_line_upto; i++)
	{
		ll = &LAND_line[i];

		ASSERT(WITHIN(ll->p1, 0, LAND_point_upto - 1));
		ASSERT(WITHIN(ll->p2, 0, LAND_point_upto - 1));

		lp1 = &LAND_point[ll->p1];
		lp2 = &LAND_point[ll->p2];

		OS_buffer_add_line_2d(
			ob,
			0.5F + (lp1->x - mid_x) * zoom,
			0.5F - (lp1->y - mid_y) * zoom * 1.33F,
			0.5F + (lp2->x - mid_x) * zoom,
			0.5F - (lp2->y - mid_y) * zoom * 1.33F,
			0.2F * zoom,
			0.0F, 0.0F,
			1.0F, 1.0F,
			0.0F,
			0x00aaaaff);
	}

	OS_buffer_draw(ob, LAND_ot, NULL, OS_DRAW_ADD | OS_DRAW_DOUBLESIDED);
}




