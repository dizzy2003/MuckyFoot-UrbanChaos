#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "poly.h"
#include "shape.h"
#include "night.h"
#include "matrix.h"
#include "c:\fallen\editor\headers\prim.h"


//
// Multiplies the two colours together.
//

ULONG SHAPE_colour_mult(ULONG c1, ULONG c2)
{
	SLONG r1;
	SLONG g1;
	SLONG b1;

	SLONG r2;
	SLONG g2;
	SLONG b2;

	SLONG r;
	SLONG g;
	SLONG b;

	ULONG ans;

	r1 = (c1 >> 16) & 0xff;
	g1 = (c1 >>  8) & 0xff;
	b1 = (c1 >>  0) & 0xff;

	r2 = (c2 >> 16) & 0xff;
	g2 = (c2 >>  8) & 0xff;
	b2 = (c2 >>  0) & 0xff;

	r = r1 * r2 >> 8;
	g = g1 * g2 >> 8;
	b = b1 * b2 >> 8;

	ans = (r << 16) | (g << 8) | (b << 0);

	return ans;
}




void SHAPE_semisphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,	// Gives the and direction of the semi-sphere.
		SLONG dy,
		SLONG dz,
		SLONG radius,
		SLONG page,
		UBYTE red,
		UBYTE green,
		UBYTE blue)
{
	SLONG i;
	SLONG j;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG ax;
	SLONG ay;
	SLONG az;

	SLONG bx;
	SLONG by;
	SLONG bz;

	SLONG ox;
	SLONG oy;
	SLONG oz;

	SLONG c_num;
	SLONG c_points;
	SLONG c_red;
	SLONG c_green;
	SLONG c_blue;
	ULONG c_colour;

	SLONG angle;
	SLONG width;
	SLONG height;
	SLONG elevation;
	SLONG along_a;
	SLONG along_b;

	SLONG p1;
	SLONG p2;

	POLY_Point *pp;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

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

	ax = dy*pz - dz*py >> 8;
	ay = dz*px - dx*pz >> 8;
	az = dx*py - dy*px >> 8;

	//
	// Create a vector parallel to both a and d.
	//

	bx = dy*az - dz*ay >> 8;
	by = dz*ax - dx*az >> 8;
	bz = dx*ay - dy*ax >> 8;

	//
	// Make the three vectors 'radius' long.
	//

	bx = bx * radius >> 8;
	by = by * radius >> 8;
	bz = bz * radius >> 8;

	ax = ax * radius >> 8;
	ay = ay * radius >> 8;
	az = az * radius >> 8;

	dx = dx * radius >> 8;
	dy = dy * radius >> 8;
	dz = dz * radius >> 8;

	//
	// Decide how detailed the semi-sphere should be. We need the number
	// of concentric circles and the number of points per circle.
	//

	c_points = 6;	// Constants for now!
	c_num    = 4;

	//
	// Build the points in the POLY_buffer.
	//

	ASSERT(WITHIN(c_points * c_num + 1, 0, POLY_BUFFER_SIZE));

	for (i = 0; i < c_num; i++)
	{
		//
		// The width and height of this circle.
		//

		elevation = i * (512 / c_num);
		width     = COS(elevation) >> 8;
		height    = SIN(elevation) >> 8;

		//
		// The centre of the circle.
		//

		ox = x + (dx * height >> 8);
		oy = y + (dy * height >> 8);
		oz = z + (dz * height >> 8);

		//
		// The colour of the points along this circle.
		// 

		c_red   = red   * height >> 8;
		c_green = green * height >> 8;
		c_blue  = blue  * height >> 8;

		c_colour = (c_red << 16) | (c_green << 8) | (c_blue << 0);

		for (j = 0; j < c_points; j++)
		{
			angle = j * (2048 / c_points);

			along_a = SIN(angle) * width >> 8;
			along_b = COS(angle) * width >> 8;

			px  = ox;
			py  = oy;
			pz  = oz;

			px += ax * along_a >> 16;
			py += ay * along_a >> 16;
			pz += az * along_a >> 16;

			px += bx * along_b >> 16;
			py += by * along_b >> 16;
			pz += bz * along_b >> 16;

			//
			// Build the point.
			//

			pp = &POLY_buffer[i * c_points + j];

			POLY_transform(
				(float) px,
				(float) py,
				(float) pz,
				pp);

			if (!(pp->clip & POLY_CLIP_TRANSFORMED))
			{
				//
				// Abandon the whole thing if one of the points is behind us!
				//

				return;
			}
			
			pp->colour   = c_colour;
			pp->specular = 0;
			pp->u        = 0.0F;
			pp->v        = 0.0F;
		}
	}

	//
	// Finally the uppermost point.
	//

	px = x + dx;
	py = y + dy;
	pz = z + dz;

	pp = &POLY_buffer[c_num * c_points];

	POLY_transform(
		(float) px,
		(float) py,
		(float) pz,
		pp);

	if (!(pp->clip & POLY_CLIP_TRANSFORMED))
	{
		//
		// Abandon the whole thing if one of the points is behind us!
		//

		return;
	}
	
	pp->colour   = (red << 16) | (green << 8) | (blue << 0);
	pp->specular = 0;
	pp->u        = 0.0F;
	pp->v        = 0.0F;

	//
	// Now create the polygons.
	//

	for (i = 0; i < c_num - 1; i++)
	{
		if (i == c_num - 2)
		{
			//
			// Triangles connecting to the uppermost point.
			//

			tri[2] = &POLY_buffer[c_num * c_points];	// Constant.

			for (j = 0; j < c_points; j++)
			{
				p1 = j + 0;
				p2 = j + 1;

				if (p2 == c_points) {p2 = 0;}

				tri[0] = &POLY_buffer[(i + 0) * c_points + p1];
				tri[1] = &POLY_buffer[(i + 0) * c_points + p2];

				if (POLY_valid_triangle(tri))
				{
					POLY_add_triangle(tri, page, TRUE);
				}
			}
		}
		else
		{
			//
			// Quads between this level and the one above.
			//

			for (j = 0; j < c_points; j++)
			{
				p1 = j + 0;
				p2 = j + 1;

				if (p2 == c_points) {p2 = 0;}

				quad[0] = &POLY_buffer[(i + 0) * c_points + p1];
				quad[1] = &POLY_buffer[(i + 0) * c_points + p2];
				quad[2] = &POLY_buffer[(i + 1) * c_points + p1];
				quad[3] = &POLY_buffer[(i + 1) * c_points + p2];

				if (POLY_valid_quad(quad))
				{
					POLY_add_quad(quad, page, TRUE);
				}
			}
		}
	}
}



void SHAPE_semisphere_textured(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,	// Gives the and direction of the semi-sphere.
		SLONG dy,
		SLONG dz,
		SLONG radius,
		float u_mid,
		float v_mid,
		float uv_radius,
		SLONG page,
		UBYTE red,
		UBYTE green,
		UBYTE blue)
{
	SLONG i;
	SLONG j;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG ax;
	SLONG ay;
	SLONG az;

	SLONG bx;
	SLONG by;
	SLONG bz;

	SLONG ox;
	SLONG oy;
	SLONG oz;

	SLONG c_num;
	SLONG c_points;
	SLONG c_red;
	SLONG c_green;
	SLONG c_blue;
	ULONG c_colour;
	float f_angle;
	float uv_width;

	SLONG angle;
	SLONG width;
	SLONG height;
	SLONG elevation;
	SLONG along_a;
	SLONG along_b;

	SLONG p1;
	SLONG p2;

	POLY_Point *pp;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

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

	ax = dy*pz - dz*py >> 8;
	ay = dz*px - dx*pz >> 8;
	az = dx*py - dy*px >> 8;

	//
	// Create a vector parallel to both a and d.
	//

	bx = dy*az - dz*ay >> 8;
	by = dz*ax - dx*az >> 8;
	bz = dx*ay - dy*ax >> 8;

	//
	// Make the three vectors 'radius' long.
	//

	bx = bx * radius >> 8;
	by = by * radius >> 8;
	bz = bz * radius >> 8;

	ax = ax * radius >> 8;
	ay = ay * radius >> 8;
	az = az * radius >> 8;

	dx = dx * radius >> 8;
	dy = dy * radius >> 8;
	dz = dz * radius >> 8;

	//
	// Decide how detailed the semi-sphere should be. We need the number
	// of concentric circles and the number of points per circle.
	//

	c_points = 9;	// Constants for now!
	c_num    = 5;

	//
	// Build the points in the POLY_buffer.
	//

	ASSERT(WITHIN(c_points * c_num + 1, 0, POLY_BUFFER_SIZE));

	for (i = 0; i < c_num; i++)
	{
		//
		// The width and height of this circle.
		//

		elevation = i * (512 / c_num);
		width     = COS(elevation) >> 8;
		height    = SIN(elevation) >> 8;

		//
		// The radius of the circle in uv space.
		//

		uv_width = float(width) * uv_radius * (1.0F / 256.0F);

		//
		// The centre of the circle.
		//

		ox = x + (dx * height >> 8);
		oy = y + (dy * height >> 8);
		oz = z + (dz * height >> 8);

		//
		// The colour of the points along this circle.
		// 

		c_red   = red   * height >> 8;
		c_green = green * height >> 8;
		c_blue  = blue  * height >> 8;

		c_colour = (c_red << 16) | (c_green << 8) | (c_blue << 0);

		for (j = 0; j < c_points; j++)
		{
			angle   = j * (2048 / c_points);
			f_angle = float(angle) * (2.0F * PI / 2048.0F);

			along_a = SIN(angle) * width >> 8;
			along_b = COS(angle) * width >> 8;

			px  = ox;
			py  = oy;
			pz  = oz;

			px += ax * along_a >> 16;
			py += ay * along_a >> 16;
			pz += az * along_a >> 16;

			px += bx * along_b >> 16;
			py += by * along_b >> 16;
			pz += bz * along_b >> 16;

			//
			// Build the point.
			//

			pp = &POLY_buffer[i * c_points + j];

			POLY_transform(
				(float) px,
				(float) py,
				(float) pz,
				pp);

			if (!(pp->clip & POLY_CLIP_TRANSFORMED))
			{
				//
				// Abandon the whole thing if one of the points is behind us!
				//

				return;
			}
			
			pp->colour   = c_colour;
			pp->specular = 0;
			pp->u        = (u_mid + sin(f_angle) * uv_width);
			pp->v        = (v_mid + cos(f_angle) * uv_width);
		}
	}

	//
	// Finally the uppermost point.
	//

	px = x + dx;
	py = y + dy;
	pz = z + dz;

	pp = &POLY_buffer[c_num * c_points];

	POLY_transform(
		(float) px,
		(float) py,
		(float) pz,
		pp);

	if (!(pp->clip & POLY_CLIP_TRANSFORMED))
	{
		//
		// Abandon the whole thing if one of the points is behind us!
		//

		return;
	}
	
	pp->colour   = (red << 16) | (green << 8) | (blue << 0);
	pp->specular = 0;
	pp->u        = u_mid;
	pp->v        = v_mid;

	//
	// Now create the polygons.
	//

	for (i = 0; i < c_num; i++)
	{
		if (i == c_num - 1)
		{
			//
			// Triangles connecting to the uppermost point.
			//

			tri[2] = &POLY_buffer[c_num * c_points];	// Constant.

			for (j = 0; j < c_points; j++)
			{
				p1 = j + 0;
				p2 = j + 1;

				if (p2 == c_points) {p2 = 0;}

				tri[0] = &POLY_buffer[(i + 0) * c_points + p1];
				tri[1] = &POLY_buffer[(i + 0) * c_points + p2];

				if (POLY_valid_triangle(tri))
				{
					POLY_add_triangle(tri, page, TRUE);
				}
			}
		}
		else
		{
			//
			// Quads between this level and the one above.
			//

			for (j = 0; j < c_points; j++)
			{
				p1 = j + 0;
				p2 = j + 1;

				if (p2 == c_points) {p2 = 0;}

				quad[0] = &POLY_buffer[(i + 0) * c_points + p1];
				quad[1] = &POLY_buffer[(i + 0) * c_points + p2];
				quad[2] = &POLY_buffer[(i + 1) * c_points + p1];
				quad[3] = &POLY_buffer[(i + 1) * c_points + p2];

				if (POLY_valid_quad(quad))
				{
					POLY_add_quad(quad, page, TRUE);
				}
			}
		}
	}
}


void SHAPE_sphere(
		SLONG ix,
		SLONG iy,
		SLONG iz,
		SLONG iradius,
		ULONG colour)
{
	SLONG i;
	SLONG j;

	SLONG p1;
	SLONG p2;

	SLONG index1;
	SLONG index2;
	SLONG index3;
	SLONG index4;

	SLONG line1;
	SLONG line2;

	float px;
	float py;
	float pz;

	float pitch;
	float yaw;

	float sx = float(ix);
	float sy = float(iy);
	float sz = float(iz);

	float sradius = float(iradius);

	float vector[3];

	POLY_Point  pp_top;
	POLY_Point  pp_bot;
	POLY_Point *pp;
	POLY_Point *quad[4];
	POLY_Point *tri [3];

	//
	// The top and bottom points.
	// 

	POLY_transform(sx, sy + sradius, sz, &pp_top);
	POLY_transform(sx, sy - sradius, sz, &pp_bot);

	if (!(pp_top.clip & POLY_CLIP_TRANSFORMED)) {return;}
	if (!(pp_bot.clip & POLY_CLIP_TRANSFORMED)) {return;}

	NIGHT_get_d3d_colour(
		NIGHT_ambient_at_point(0, +256, 0),
	   &pp_top.colour,
	   &pp_top.specular);

	NIGHT_get_d3d_colour(
		NIGHT_ambient_at_point(0, -256, 0),
	   &pp_bot.colour,
	   &pp_bot.specular);

	pp_top.colour = SHAPE_colour_mult(pp_top.colour, colour);
	pp_bot.colour = SHAPE_colour_mult(pp_bot.colour, colour);

	//
	// All the points in the middle.
	//

	#define SHAPE_SPHERE_NUM_UPDOWN 8
	#define SHAPE_SPHERE_NUM_AROUND 8

	pp = &POLY_buffer[0];

	pitch = PI / 2.0F;

	for (i = 1; i < SHAPE_SPHERE_NUM_UPDOWN; i++)
	{
		pitch -= PI / float(SHAPE_SPHERE_NUM_UPDOWN);
		yaw    = 0.0F;
 
		for (j = 0; j < SHAPE_SPHERE_NUM_AROUND; j++)
		{
			MATRIX_vector(
				vector,
				yaw,
				pitch);

			px = sx + vector[0] * sradius;
			py = sy + vector[1] * sradius;
			pz = sz + vector[2] * sradius;

			ASSERT(WITHIN(pp, &POLY_buffer[0], &POLY_buffer[POLY_BUFFER_SIZE - 1]));

			POLY_transform(
				px,
				py,
				pz,
				pp);

			if (!(pp->clip & POLY_CLIP_TRANSFORMED))
			{
				return;
			}

			NIGHT_get_d3d_colour(
				NIGHT_ambient_at_point(
					SLONG(vector[0] * 256.0F),
					SLONG(vector[1] * 256.0F),
					SLONG(vector[2] * 256.0F)),
			   &pp->colour,
			   &pp->specular);

			pp->colour = SHAPE_colour_mult(pp->colour, colour);

			yaw += 2.0F * PI / float(SHAPE_SPHERE_NUM_AROUND);
			pp  += 1;
		}
	}

	//
	// The triangles at the top and bottom.
	//

	for (i = 0; i < SHAPE_SPHERE_NUM_AROUND; i++)
	{
		p1 = i;
		p2 = i + 1;

		if (p2 == SHAPE_SPHERE_NUM_AROUND) {p2 = 0;}
		
		//
		// Top...
		//

		index1 = p1;
		index2 = p2;

		tri[0] = &pp_top;
		tri[1] = &POLY_buffer[index1];
		tri[2] = &POLY_buffer[index2];

		if (POLY_valid_triangle(tri))
		{
			POLY_add_triangle(tri, POLY_PAGE_COLOUR, FALSE);
		}

		//
		// Bottom...
		//

		index1 = p1 + SHAPE_SPHERE_NUM_AROUND * (SHAPE_SPHERE_NUM_UPDOWN - 2);
		index2 = p2 + SHAPE_SPHERE_NUM_AROUND * (SHAPE_SPHERE_NUM_UPDOWN - 2);

		tri[0] = &pp_bot;
		tri[1] = &POLY_buffer[index1];
		tri[2] = &POLY_buffer[index2];

		if (POLY_valid_triangle(tri))
		{
			POLY_add_triangle(tri, POLY_PAGE_COLOUR, FALSE);
		}
	}

	//
	// The quads in the middle.
	//

	for (i = 1; i < SHAPE_SPHERE_NUM_UPDOWN - 1; i++)
	{
		line1 = (i - 1) * SHAPE_SPHERE_NUM_AROUND;
		line2 = (i - 0) * SHAPE_SPHERE_NUM_AROUND;

		for (j = 0; j < SHAPE_SPHERE_NUM_AROUND; j++)
		{
			p1 = j;
			p2 = j + 1;

			if (p2 == SHAPE_SPHERE_NUM_AROUND) {p2 = 0;}
			
			quad[0] = &POLY_buffer[line1 + p1];
			quad[1] = &POLY_buffer[line1 + p2];
			quad[2] = &POLY_buffer[line2 + p1];
			quad[3] = &POLY_buffer[line2 + p2];

			if (POLY_valid_quad(quad))
			{
				POLY_add_quad(quad, POLY_PAGE_COLOUR, FALSE);
			}
		}
	}
}


void SHAPE_sparky_line(
		SLONG num_points,
		SLONG px[],
		SLONG py[],
		SLONG pz[],
		ULONG colour,
		float width)
{
	ASSERT(WITHIN(num_points, 2, SHAPE_MAX_SPARKY_POINTS));

	SLONG i;

	SLONG p1;
	SLONG p2;

	float dx;
	float dy;
	float len;
	float overlen;
	float size;

	float pnx;
	float pny;
	SLONG n1_valid;
	SLONG n2_valid;

	POLY_Point pp1;
	POLY_Point pp2;	

	float nx[SHAPE_MAX_SPARKY_POINTS];
	float ny[SHAPE_MAX_SPARKY_POINTS];

	POLY_Point pp[SHAPE_MAX_SPARKY_POINTS];

	POLY_Point *quad[4];

	//
	// Transform all the points along the middle of the line.
	//

	for (i = 0; i < num_points; i++)
	{
		POLY_transform(
			float(px[i]),
			float(py[i]),
			float(pz[i]),
		   &pp[i]);

		pp[i].colour   = colour | 0xff000000;
		pp[i].specular = 0x00000000;
		pp[i].u        = 0.0F;
		pp[i].v        = 0.0F;
	}

	//
	// Work out the 2d point 'normals'
	//

	for (i = 0; i < num_points; i++)
	{
		if (pp[i].clip & POLY_CLIP_TRANSFORMED)
		{
			if (i == 0 && width == 20.0F)
			{ 
				size = POLY_world_length_to_screen(width * 2.0F) * pp[i].Z;
			}
			else
			if (i == num_points - 1)
			{
				size = 2.0F;
			}
			else
			{
				size = POLY_world_length_to_screen(width) * pp[i].Z;
			}

			p1 = i - 1;
			p2 = i + 1;

			n1_valid = FALSE;
			n2_valid = FALSE;

			if (p1 > 0 && (pp[p1].clip & POLY_CLIP_TRANSFORMED))
			{
				dx = pp[i].X - pp[p1].X;
				dy = pp[i].Y - pp[p1].Y;

				//
				// Hmm... I guess that .414F is better than 0.500F
				//

				len     = (fabs(dx) > fabs(dy)) ? fabs(dx) + 0.414F * fabs(dy) : fabs(dy) + 0.414F * fabs(dx);
				overlen = size / len;
				
				dx *= overlen;
				dy *= overlen;

				pnx = -dy;
				pny =  dx;

				n1_valid = TRUE;
			}

			if (p2 < num_points && (pp[p2].clip & POLY_CLIP_TRANSFORMED))
			{
				dx = pp[p2].X - pp[i].X;
				dy = pp[p2].Y - pp[i].Y;

				//
				// Hmm... I guess that .414F is better than 0.500F
				//

				len     = (fabs(dx) > fabs(dy)) ? fabs(dx) + 0.414F * fabs(dy) : fabs(dy) + 0.414F * fabs(dx);
				overlen = size / len;
				
				dx *= overlen;
				dy *= overlen;

				if (n1_valid)
				{
					pnx  = (pnx + -dy) * 0.5F;
					pny  = (pny +  dx) * 0.5F;

					//
					// Normlise (pnx,pny);
					//

					len     = (fabs(pnx) > fabs(pny)) ? fabs(pnx) + 0.414F * fabs(pny) : fabs(pny) + 0.414F * fabs(pnx);
					overlen = size / len;
					
					pnx *= overlen;
					pny *= overlen;
				}
				else
				{
					pnx = -dy;
					pny =  dx;
				}

				n2_valid = TRUE;
			}

			nx[i] = pnx;
			ny[i] = pny;
		}
	}

	//
	// The quads...
	//

	for (i = 0; i < num_points - 1; i++)
	{
		if (pp[i + 0].clip & pp[i + 1].clip & POLY_CLIP_TRANSFORMED)
		{
			quad[0] = &pp[i + 0];
			quad[1] = &pp[i + 1];
			quad[2] = &pp1;
			quad[3] = &pp2;

			//
			// The two auxillary points.
			//

			pp1 = pp[i + 0];
			pp2 = pp[i + 1];

			if (!POLY_valid_quad(quad))
			{
				continue;
			}

			pp1.X += nx[i + 0];
			pp1.Y += ny[i + 0];

			pp2.X += nx[i + 1];
			pp2.Y += ny[i + 1];

			pp[i + 0].colour = colour | 0xff000000;
			pp[i + 1].colour = colour | 0xff000000;
			pp1.colour       = colour & 0x00ffffff;
			pp2.colour       = colour & 0x00ffffff;

			POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE);

			pp1.X = pp[i + 0].X + (nx[i + 0] * 0.25F);
			pp1.Y = pp[i + 0].Y + (ny[i + 0] * 0.25F);

			pp2.X = pp[i + 1].X + (nx[i + 1] * 0.25F);
			pp2.Y = pp[i + 1].Y + (ny[i + 1] * 0.25F);

			pp[i + 0].colour = 0xffffffff;
			pp[i + 1].colour = 0xffffffff;
			pp1.colour       = colour;
			pp2.colour       = colour;

			POLY_add_quad(quad, POLY_PAGE_ADDITIVE, FALSE);

			pp1.X = pp[i + 0].X - nx[i + 0];
			pp1.Y = pp[i + 0].Y - ny[i + 0];
							   
			pp2.X = pp[i + 1].X - nx[i + 1];
			pp2.Y = pp[i + 1].Y - ny[i + 1];

			pp[i + 0].colour = colour | 0xff000000;
			pp[i + 1].colour = colour | 0xff000000;
			pp1.colour       = colour & 0x00ffffff;
			pp2.colour       = colour & 0x00ffffff;

			POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE);

			pp1.X = pp[i + 0].X - (nx[i + 0] * 0.25F);
			pp1.Y = pp[i + 0].Y - (ny[i + 0] * 0.25F);
							   
			pp2.X = pp[i + 1].X - (nx[i + 1] * 0.25F);
			pp2.Y = pp[i + 1].Y - (ny[i + 1] * 0.25F);

			pp[i + 0].colour = 0xffffffff;
			pp[i + 1].colour = 0xffffffff;
			pp1.colour       = colour;
			pp2.colour       = colour;

			POLY_add_quad(quad, POLY_PAGE_ADDITIVE, FALSE);
		}
	}
}

void SHAPE_glitter(
		SLONG x1,
		SLONG y1,
		SLONG z1,
		SLONG x2,
		SLONG y2,
		SLONG z2,
		ULONG colour)
{
	float dpx;
	float dpy;

	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;

	float adpx;
	float adpy;

	float len;
	float mul;

	float size;

	POLY_Point pp1;
	POLY_Point pp2;

	POLY_Point top;
	POLY_Point bot;

	POLY_Point *tri[3];

	POLY_transform(
		float(x1),
		float(y1),
		float(z1),
	   &pp1);

	if (!(pp1.clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	POLY_transform(
		float(x2 + dx),
		float(y2 + dy),
		float(z2 + dz),
	   &pp2);

	if (!(pp2.clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	dpx = pp1.X - pp2.X;
	dpy = pp1.Y - pp2.Y;

	adpx = fabs(dpx);
	adpy = fabs(dpy);

	if (adpx > adpy)
	{
		len = adpx + (adpy * 0.5F);
	}
	else
	{
		len = adpy + (adpx * 0.5F);
	}	

	size = POLY_world_length_to_screen(30.0F) * pp1.Z;
	mul  = size / len;

	dpx *= mul;
	dpy *= mul;

	top = pp1;
	bot = pp1;

	top.X += dpy;
	top.Y -= dpx;

	bot.X -= dpy;
	bot.Y += dpx;

	tri[0] = &top;
	tri[1] = &bot;
	tri[2] = &pp2;

	if (POLY_valid_triangle(tri))
	{
		pp2.colour   = colour;
		pp2.specular = 0xff000000;
		pp2.u        = 1.0F;
		pp2.v        = 0.5F;

		top.colour   = colour;
		top.specular = 0xff000000;
		top.u        = 0.0F;
		top.v        = 0.0F;

		bot.colour   = colour;
		bot.specular = 0xff000000;
		bot.u        = 0.0F;
		bot.v        = 1.0F;

		POLY_add_triangle(tri, POLY_PAGE_SPARKLE, FALSE);
	}
}

//
// Sets uv coordinates depending on the frame counter.
//

void SHAPE_tripwire_uvs(
		UWORD  counter,
		float *u1,
		float *v1,
		float *u2,
		float *v2,
		float *u3,
		float *v3,
		float *u4,
		float *v4)
{
	float v = float(counter & 0x7fff) * (1.0F / (128.0F * 256.0F));

	if (counter & 0x8000)
	{
		*u1 = 0.6F;
		*u2 = 0.6F;
		*u3 = 0.9F;
		*u4 = 0.9F;
	}
	else
	{
		*u1 = 0.1F;
		*u2 = 0.1F;
		*u3 = 0.4F;
		*u4 = 0.4F;
	}

	*v1 = *v3 = v;
	*v2 = *v4 = v + (16.0F / 256.0F);
}

void SHAPE_tripwire(
		SLONG ix1,
		SLONG iy1,
		SLONG iz1,
		SLONG ix2,
		SLONG iy2,
		SLONG iz2,
		SLONG width,
		ULONG colour,
		UWORD counter,
		UBYTE along)
{
	float x1 = float(ix1);
	float y1 = float(iy1);
	float z1 = float(iz1);
	float x2 = float(ix2);
	float y2 = float(iy2);
	float z2 = float(iz2);

	POLY_Point  pp[4];
	POLY_Point *quad[4];

	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;

	if (along == 0)
	{
		return;
	}
	else
	if (along != 255)
	{
		float falong = float(along) * (1.0F / 256.0F);

		//
		// Change (x2,y2,z2).
		//

		x2 = x1 + dx * falong;
		y2 = y1 + dy * falong;
		z2 = z1 + dz * falong;
	}

	float len;

	if (fabs(dx) > fabs(dz))
	{
		len = fabs(dx) + 0.5F * fabs(dz);
	}
	else
	{
		len = fabs(dz) + 0.5F * fabs(dx);
	}

	float overlen = float(width) / len;

	dx *= overlen;
	dz *= overlen;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// Transform the four point. If any of them fail abandon
	// the whole line.
	//

	POLY_transform(
		x1 + dz,
		y1,
		z1 - dx,
	   &pp[0]);
	
	if (!(pp[0].clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	POLY_transform(
		x1 - dz,
		y1,
		z1 + dx,
	   &pp[1]);
	
	if (!(pp[1].clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	POLY_transform(
		x2 + dz,
		y2,
		z2 - dx,
	   &pp[2]);
	
	if (!(pp[2].clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	POLY_transform(
		x2 - dz,
		y2,
		z2 + dx,
	   &pp[3]);
	
	if (!(pp[3].clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	if (POLY_valid_quad(quad))
	{
		//
		// Draw two overlapping lines.
		//

		pp[0].colour = colour;
		pp[1].colour = colour;
		pp[2].colour = colour;
		pp[3].colour = colour;

		pp[0].specular = 0x00000000;
		pp[1].specular = 0x00000000;
		pp[2].specular = 0x00000000;
		pp[3].specular = 0x00000000;

		SHAPE_tripwire_uvs(
			counter,
		   &pp[0].u, &pp[0].v,
		   &pp[1].u, &pp[1].v,
		   &pp[2].u, &pp[2].v,
		   &pp[3].u, &pp[3].v);

		POLY_add_quad(quad, POLY_PAGE_FOG, FALSE);

		SHAPE_tripwire_uvs(
			0x2345 - counter,
		   &pp[0].u, &pp[0].v,
		   &pp[1].u, &pp[1].v,
		   &pp[2].u, &pp[2].v,
		   &pp[3].u, &pp[3].v);

		POLY_add_quad(quad, POLY_PAGE_FOG, FALSE);
	}
}




void SHAPE_waterfall(
		SLONG map_x,
		SLONG map_z,
		SLONG dx,
		SLONG dz,
		SLONG top,
		SLONG bot)
{
	SLONG i;
	SLONG y;
	ULONG colour;

	SLONG mid_x = (map_x << 8) + 0x80;
	SLONG mid_z = (map_z << 8) + 0x80;

	SLONG px1 = mid_x + (dx << 7) + (-dz << 7);
	SLONG pz1 = mid_z + (dz << 7) + (+dx << 7);

	SLONG px2 = mid_x + (dx << 7) + (+dz << 7);
	SLONG pz2 = mid_z + (dz << 7) + (-dx << 7);

	POLY_Point  pp  [6];
	POLY_Point *quad[4];

	for (i = 0; i < 3; i++)
	{
		switch(i)
		{	
			case 0: y = top;        colour = 0xaa4488aa; break;
			case 1: y = top - 0x10; colour = 0x884488aa; break;
			case 2: y = top - 0x40;	colour = 0x004488aa; break;
			default:
				ASSERT(0);
				break;
		}

		POLY_transform(
			float(px1),
			float(y),
			float(pz1),
		   &pp[0 + i]);

		pp[0 + i].colour   = colour;
		pp[0 + i].specular = 0xff000000;
		pp[0 + i].u        = 0.0F;
		pp[0 + i].v        = 0.0F;

		POLY_transform(
			float(px2),
			float(y),
			float(pz2),
		   &pp[3 + i]);

		pp[3 + i].colour   = colour;
		pp[3 + i].specular = 0xff000000;
		pp[3 + i].u        = 0.0F;
		pp[3 + i].v        = 0.0F;

		px1 -= dx << 4;
		pz1 -= dz << 4;

		px2 -= dx << 4;
		pz2 -= dz << 4;
	}

	quad[0] = &pp[3];
	quad[1] = &pp[0];
	quad[2] = &pp[4];
	quad[3] = &pp[1];

	if (POLY_valid_quad(quad))
	{
		POLY_add_quad(quad, POLY_PAGE_SEWATER, TRUE);
	}

	quad[0] = &pp[4];
	quad[1] = &pp[1];
	quad[2] = &pp[5];
	quad[3] = &pp[2];

	if (POLY_valid_quad(quad))
	{
		POLY_add_quad(quad, POLY_PAGE_SEWATER, TRUE);
	}
}



void SHAPE_droplet(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,
		SLONG dy,
		SLONG dz,
		ULONG colour)
{
	float dpx;
	float dpy;

	float adpx;
	float adpy;

	float len;
	float mul;

	float size;

	POLY_Point pp1;
	POLY_Point pp2;

	POLY_Point top;
	POLY_Point bot;

	POLY_Point *tri[3];

	POLY_transform(
		float(x),
		float(y),
		float(z),
	   &pp1);

	if (!(pp1.clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	POLY_transform(
		float(x - dx),
		float(y - dy),
		float(z - dz),
	   &pp2);

	if (!(pp2.clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	dpx = pp1.X - pp2.X;
	dpy = pp1.Y - pp2.Y;

	adpx = fabs(dpx);
	adpy = fabs(dpy);

	if (adpx > adpy)
	{
		len = adpx + (adpy * 0.5F);
	}
	else
	{
		len = adpy + (adpx * 0.5F);
	}	

	size = POLY_world_length_to_screen(20.0F) * pp1.Z;
	mul  = size / len;

	dpx *= mul;
	dpy *= mul;

	top = pp1;
	bot = pp1;

	top.X += dpy;
	top.Y -= dpx;

	bot.X -= dpy;
	bot.Y += dpx;

	tri[0] = &top;
	tri[1] = &bot;
	tri[2] = &pp2;

	if (POLY_valid_triangle(tri))
	{
		pp2.colour   = colour;
		pp2.specular = 0xff000000;
		pp2.u        = 1.0F;
		pp2.v        = 0.5F;

		top.colour   = colour;
		top.specular = 0xff000000;
		top.u        = 0.0F;
		top.v        = 0.0F;

		bot.colour   = colour;
		bot.specular = 0xff000000;
		bot.u        = 0.0F;
		bot.v        = 1.0F;

		POLY_add_triangle(tri, POLY_PAGE_DROPLET, FALSE);
	}
}



void SHAPE_prim_shadow(OB_Info *oi)
{
	POLY_Point  pp  [6];
	POLY_Point *quad[4];

	#define SHAPE_PRIM_SHADOW_LENGTH (128.0F)

	float px = float(oi->x);
	float py = float(oi->y);
	float pz = float(oi->z);

	SLONG i;

	PrimInfo *pi;

	float angle;

	float sin_yaw;
	float cos_yaw;

	float matrix[4];

	float wx;
	float wz;

	struct
	{
		float x;
		float z;

	} world[4];

	UBYTE order[3];

	POLY_Point *pp_upto;

	switch(prim_get_collision_model(oi->prim))
	{
		case PRIM_COLLIDE_CYLINDER:

			//
			// Circular shadow.
			//

			#define SHAPE_PRIM_SHADOW_RADIUS (24.0F)

			POLY_transform(
				px - SHAPE_PRIM_SHADOW_RADIUS * -0.707F,
				py + 2.0F,
				pz - SHAPE_PRIM_SHADOW_RADIUS * -0.707F,
			   &pp[0]);

			if (!(pp[0].clip & POLY_CLIP_TRANSFORMED))
			{
				return;
			}

			POLY_transform(
				px - SHAPE_PRIM_SHADOW_RADIUS * +0.707F,
				py + 2.0F,
				pz - SHAPE_PRIM_SHADOW_RADIUS * +0.707F,
			   &pp[1]);

			if (!(pp[1].clip & POLY_CLIP_TRANSFORMED))
			{
				return;
			}

			POLY_transform(
				px - SHAPE_PRIM_SHADOW_RADIUS * -0.707F + SHAPE_PRIM_SHADOW_LENGTH * +0.707F,
				py + 2.0F,
				pz - SHAPE_PRIM_SHADOW_RADIUS * -0.707F + SHAPE_PRIM_SHADOW_LENGTH * -0.707F,
			   &pp[2]);

			if (!(pp[2].clip & POLY_CLIP_TRANSFORMED))
			{
				return;
			}

			POLY_transform(
				px - SHAPE_PRIM_SHADOW_RADIUS * +0.707F + SHAPE_PRIM_SHADOW_LENGTH * +0.707F,
				py + 2.0F,
				pz - SHAPE_PRIM_SHADOW_RADIUS * +0.707F + SHAPE_PRIM_SHADOW_LENGTH * -0.707F,
			   &pp[3]);

			if (!(pp[3].clip & POLY_CLIP_TRANSFORMED))
			{
				return;
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			if (POLY_valid_quad(quad))
			{
				pp[0].colour   = 0x88000000;
				pp[0].specular = 0xff000000;
				pp[0].u        = 0.0F;
				pp[0].v        = 0.0F;

				pp[1].colour   = 0x88000000;
				pp[1].specular = 0xff000000;
				pp[1].u        = 0.0F;
				pp[1].v        = 0.0F;

				pp[2].colour   = 0x00000000;
				pp[2].specular = 0xff000000;
				pp[2].u        = 0.0F;
				pp[2].v        = 0.0F;

				pp[3].colour   = 0x00000000;
				pp[3].specular = 0xff000000;
				pp[3].u        = 0.0F;
				pp[3].v        = 0.0F;

				POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE);
			}

			break;

		case PRIM_COLLIDE_BOX:

			//
			// Bounding box shadow.
			//

			pi = get_prim_info(oi->prim);

			angle = float(-oi->yaw) * (2.0F * PI / 2048.0F);

			sin_yaw = sin(angle);
			cos_yaw = cos(angle);

			matrix[0] =  cos_yaw;
			matrix[1] = -sin_yaw;
			matrix[2] =  sin_yaw;
			matrix[3] =  cos_yaw;

			for (i = 0; i < 4; i++)
			{
				wx = (i & 0x1) ? float(pi->maxx) : float(pi->minx);
				wz = (i & 0x2) ? float(pi->maxz) : float(pi->minz);

				world[i].x  = wx * matrix[0] + wz * matrix[1];
				world[i].z  = wx * matrix[2] + wz * matrix[3];

				world[i].x += px;
				world[i].z += pz;
			}

			switch(((oi->yaw + 256) & 2047) >> 9)
			{
				case 0: order[0] = 0; order[1] = 1; order[2] = 3; break;
				case 1: order[0] = 1; order[1] = 3; order[2] = 2; break;
				case 2: order[0] = 3; order[1] = 2; order[2] = 0; break;
				case 3: order[0] = 2; order[1] = 0; order[2] = 1; break;

				default:
					ASSERT(0);
					break;
			}

			pp_upto = &pp[0];
		
			for (i = 0; i < 3; i++)
			{
				POLY_transform(
					world[order[i]].x,
					py + 2.0F,
					world[order[i]].z,
					pp_upto);

				if (!(pp_upto->clip & POLY_CLIP_TRANSFORMED))
				{
					return;
				}

				pp_upto->colour   = 0x88000000;
				pp_upto->specular = 0xff000000;
				pp_upto->u        = 0.0F;
				pp_upto->v        = 0.0F;

				pp_upto++;

				POLY_transform(
					world[order[i]].x + SHAPE_PRIM_SHADOW_LENGTH * +0.707F,
					py + 2.0F,
					world[order[i]].z + SHAPE_PRIM_SHADOW_LENGTH * -0.707F,
					pp_upto);

				if (!(pp_upto->clip & POLY_CLIP_TRANSFORMED))
				{
					return;
				}

				pp_upto->colour   = 0x00000000;
				pp_upto->specular = 0xff000000;
				pp_upto->u        = 0.0F;
				pp_upto->v        = 0.0F;

				pp_upto++;
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			if (POLY_valid_quad(quad))
			{
				POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE);
			}

			quad[0] = &pp[2];
			quad[1] = &pp[3];
			quad[2] = &pp[4];
			quad[3] = &pp[5];

			if (POLY_valid_quad(quad))
			{
				POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE);
			}

			break;

		case PRIM_COLLIDE_NONE:
			return;

		default:
			ASSERT(0);
			break;
	}
}







