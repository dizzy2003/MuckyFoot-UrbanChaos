//
// Drawing rotating prims.
//

#include "game.h"
#include "aeng.h"
#include "matrix.h"
#include "mesh.h"
#include "c:\fallen\headers\night.h"
#include "c:\fallen\headers\light.h"
#include "poly.h"
#include "c:\fallen\headers\animtmap.h"
#include "c:\fallen\headers\morph.h"
#include <math.h>

#define	POLY_FLAG_GOURAD		(1<<0)
#define	POLY_FLAG_TEXTURED		(1<<1)
#define	POLY_FLAG_MASKED		(1<<2)
#define	POLY_FLAG_SEMI_TRANS	(1<<3)
#define	POLY_FLAG_ALPHA			(1<<4)
#define	POLY_FLAG_TILED			(1<<5)
#define	POLY_FLAG_DOUBLESIDED	(1<<6)
#define	POLY_FLAG_CLIPPED		(1<<7)


NIGHT_Colour *MESH_draw_guts(
				SLONG         prim,
				MAPCO16	      at_x,
				MAPCO16       at_y,
				MAPCO16	      at_z,
				float         matrix[9],
				NIGHT_Colour *lpc)
{
	SLONG i;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG page;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	POLY_set_local_rotation(
		float(at_x),
		float(at_y),
		float(at_z),
		matrix);

	//
	// Rotate all the points into the POLY_buffer and all the
	// shadow points in the POLY_shadow.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;
	POLY_shadow_upto = 0;

	for (i = sp; i < ep; i++)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform_using_local_rotation(
			AENG_dx_prim_points[i].X,
			AENG_dx_prim_points[i].Y,
			AENG_dx_prim_points[i].Z,
			pp);

		if (lpc)
		{
			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
				NIGHT_get_d3d_colour(
				   *lpc,
				   &pp->colour,
				   &pp->specular);
			}

			lpc += 1;
		}
		else
		{
		    pp->colour   = NIGHT_amb_d3d_colour;
		    pp->specular = NIGHT_amb_d3d_specular;
		}

		POLY_fadeout_point(pp);
	}

	//
	// The quads.
	//

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - sp;
		p1 = p_f4->Points[1] - sp;
		p2 = p_f4->Points[2] - sp;
		p3 = p_f4->Points[3] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

		quad[0] = &POLY_buffer[p0];
		quad[1] = &POLY_buffer[p1];
		quad[2] = &POLY_buffer[p2];
		quad[3] = &POLY_buffer[p3];

		if (POLY_valid_quad(quad))
		{
			if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
			{
				if(p_f4->FaceFlags&FACE_FLAG_ANIMATE)
				{
					struct	AnimTmap	*p_a;
					SLONG	cur;
					p_a=&anim_tmaps[p_f4->TexturePage];
					cur=p_a->Current;


					quad[0]->u = float(p_a->UV[cur][0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_a->UV[cur][0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_a->UV[cur][1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_a->UV[cur][1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_a->UV[cur][2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_a->UV[cur][2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_a->UV[cur][3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_a->UV[cur][3][1]       ) * (1.0F / 32.0F);

					page   = p_a->UV[cur][0][0] & 0xc0;
					page <<= 2;
					page  |= p_a->Page[cur];
				}
				else
				{

					quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

					page   = p_f4->UV[0][0] & 0xc0;
					page <<= 2;
					page  |= p_f4->TexturePage;
				}

				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}
	}

	//
	// The triangles.
	//

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - sp;
		p1 = p_f3->Points[1] - sp;
		p2 = p_f3->Points[2] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

		tri[0] = &POLY_buffer[p0];
		tri[1] = &POLY_buffer[p1];
		tri[2] = &POLY_buffer[p2];

		if (POLY_valid_triangle(tri))
		{
			if (p_f3->DrawFlags & POLY_FLAG_TEXTURED)
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												       
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												       
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}
	}

	if (prim == 122)
	{
		//
		// The cinema screen. Find the screen and draw it backwards
		// faded out to white
		//

		for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
		{
			p_f4 = &prim_faces4[i];

			page   = p_f4->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= p_f4->TexturePage;

			if (page == 86)
			{
				p0 = p_f4->Points[2] - sp;
				p1 = p_f4->Points[3] - sp;
				p2 = p_f4->Points[0] - sp;
				p3 = p_f4->Points[1] - sp;
				
				ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

				quad[0] = &POLY_buffer[p0];
				quad[1] = &POLY_buffer[p1];
				quad[2] = &POLY_buffer[p2];
				quad[3] = &POLY_buffer[p3];

				if (POLY_valid_quad(quad))
				{
					quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

					quad[0]->specular |= (0x00888888 & ~POLY_colour_restrict);
					quad[1]->specular |= (0x00888888 & ~POLY_colour_restrict);
					quad[2]->specular |= (0x00888888 & ~POLY_colour_restrict);
					quad[3]->specular |= (0x00888888 & ~POLY_colour_restrict);

					POLY_add_quad(quad, 86, TRUE);
				}
			}
		}
	}

	return lpc;
}


NIGHT_Colour *MESH_draw_poly(
				SLONG         prim,
				MAPCO16	      at_x,
				MAPCO16       at_y,
				MAPCO16	      at_z,
				SLONG         i_yaw,
				SLONG         i_pitch,
				SLONG         i_roll,
				NIGHT_Colour *lpc)
{
	float matrix[9];
	float yaw;
	float pitch;
	float roll;

	yaw   = float(i_yaw)   * (2.0F * PI / 2048.0F);
	pitch = float(i_pitch) * (2.0F * PI / 2048.0F);
	roll  = float(i_roll)  * (2.0F * PI / 2048.0F);
	
	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	MATRIX_TRANSPOSE(matrix);

	POLY_set_local_rotation(
		float(at_x),
		float(at_y),
		float(at_z),
		matrix);

	return MESH_draw_guts(prim, at_x, at_y, at_z, matrix, lpc);
}

NIGHT_Colour *MESH_draw_poly_inv_matrix(
				SLONG         prim,
				MAPCO16	      at_x,
				MAPCO16       at_y,
				MAPCO16	      at_z,
				SLONG         i_yaw,
				SLONG         i_pitch,
				SLONG         i_roll,
				NIGHT_Colour *lpc)
{
	float matrix[9];
	float yaw;
	float pitch;
	float roll;

	yaw   = float(i_yaw)   * (2.0F * PI / 2048.0F);
	pitch = float(i_pitch) * (2.0F * PI / 2048.0F);
	roll  = float(i_roll)  * (2.0F * PI / 2048.0F);
	
	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

//	MATRIX_TRANSPOSE(matrix);

	POLY_set_local_rotation(
		float(at_x),
		float(at_y),
		float(at_z),
		matrix);

	return MESH_draw_guts(prim, at_x, at_y, at_z, matrix, lpc);
}


/*
NIGHT_Colour *MESH_draw_poly(
				SLONG         prim,
				MAPCO16	      at_x,
				MAPCO16       at_y,
				MAPCO16	      at_z,
				SLONG         i_yaw,
				SLONG         i_pitch,
				SLONG         i_roll,
				NIGHT_Colour *lpc)
{
	SLONG i;
//	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG page;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
//	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	float matrix[9];
	float yaw;
	float pitch;
	float roll;

	yaw   = float(i_yaw)   * (2.0F * PI / 2048.0F);
	pitch = float(i_pitch) * (2.0F * PI / 2048.0F);
	roll  = float(i_roll)  * (2.0F * PI / 2048.0F);
	
	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	MATRIX_TRANSPOSE(matrix);

	POLY_set_local_rotation(
		float(at_x),
		float(at_y),
		float(at_z),
		matrix);

	//
	// Rotate all the points into the POLY_buffer and all the
	// shadow points in the POLY_shadow.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;
	POLY_shadow_upto = 0;

	for (i = sp; i < ep; i++)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform_using_local_rotation(
			AENG_dx_prim_points[i].X,
			AENG_dx_prim_points[i].Y,
			AENG_dx_prim_points[i].Z,
			pp);

		if (lpc)
		{
			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
				NIGHT_get_d3d_colour(
				   *lpc,
				   &pp->colour,
				   &pp->specular);
			}

			lpc += 1;
		}
		else
		{
		    pp->colour   = NIGHT_amb_d3d_colour;
		    pp->specular = NIGHT_amb_d3d_specular;
		}

		POLY_fadeout_point(pp);
	}

	//
	// The quads.
	//

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - sp;
		p1 = p_f4->Points[1] - sp;
		p2 = p_f4->Points[2] - sp;
		p3 = p_f4->Points[3] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

		quad[0] = &POLY_buffer[p0];
		quad[1] = &POLY_buffer[p1];
		quad[2] = &POLY_buffer[p2];
		quad[3] = &POLY_buffer[p3];

		if (POLY_valid_quad(quad))
		{
			if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
			{
				if(p_f4->FaceFlags&FACE_FLAG_ANIMATE)
				{
					struct	AnimTmap	*p_a;
					SLONG	cur;
					p_a=&anim_tmaps[p_f4->TexturePage];
					cur=p_a->Current;


					quad[0]->u = float(p_a->UV[cur][0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_a->UV[cur][0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_a->UV[cur][1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_a->UV[cur][1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_a->UV[cur][2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_a->UV[cur][2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_a->UV[cur][3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_a->UV[cur][3][1]       ) * (1.0F / 32.0F);

					page   = p_a->UV[cur][0][0] & 0xc0;
					page <<= 2;
					page  |= p_a->Page[cur];
				}
				else
				{

					quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

					page   = p_f4->UV[0][0] & 0xc0;
					page <<= 2;
					page  |= p_f4->TexturePage;
				}

				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}
	}

	//
	// The triangles.
	//

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - sp;
		p1 = p_f3->Points[1] - sp;
		p2 = p_f3->Points[2] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

		tri[0] = &POLY_buffer[p0];
		tri[1] = &POLY_buffer[p1];
		tri[2] = &POLY_buffer[p2];

		if (POLY_valid_triangle(tri))
		{
			if (p_f3->DrawFlags & POLY_FLAG_TEXTURED)
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												       
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												       
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}
	}

	return lpc;
}
*/





void MESH_draw_morph(
		SLONG         prim,
		UBYTE         morph1,
		UBYTE         morph2,
		UWORD		  tween,		// 0 - 256         
		MAPCO16	      at_x,
		MAPCO16       at_y,
		MAPCO16	      at_z,
		SLONG         i_yaw,
		SLONG         i_pitch,
		SLONG         i_roll,
		NIGHT_Colour *lpc)
{
	SLONG i;
//	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	float px;
	float py;
	float pz;

	float px1, px2;
	float py1, py2;
	float pz1, pz2;

	float ptween;

	SLONG page;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	MORPH_Point *mp1 = MORPH_get_points(morph1);
	MORPH_Point *mp2 = MORPH_get_points(morph2);

	SLONG num_points;

	POLY_Point *pp;
//	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	float matrix[9];
	float yaw;
	float pitch;
	float roll;

	yaw   = float(i_yaw)   * (2.0F * PI / 2048.0F);
	pitch = float(i_pitch) * (2.0F * PI / 2048.0F);
	roll  = float(i_roll)  * (2.0F * PI / 2048.0F);
	
	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	MATRIX_TRANSPOSE(matrix);

	POLY_set_local_rotation(
		float(at_x),
		float(at_y),
		float(at_z),
		matrix);

	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;
	POLY_shadow_upto = 0;

	ASSERT(
		MORPH_get_num_points(morph1) == 
		MORPH_get_num_points(morph2));

	num_points = MORPH_get_num_points(morph1);
	ptween     = float(tween) * (1.0F / 256.0F);

	for (i = 0; i < num_points; i++)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		px1 = float(mp1->x);
		py1 = float(mp1->y);
		pz1 = float(mp1->z);
		
		px2 = float(mp2->x);
		py2 = float(mp2->y);
		pz2 = float(mp2->z);

		px  = px1 + (px2 - px1) * ptween;
		py  = py1 + (py2 - py1) * ptween;
		pz  = pz1 + (pz2 - pz1) * ptween;

		POLY_transform_using_local_rotation(px, py, pz, pp);

		if (lpc)
		{
			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
				NIGHT_get_d3d_colour(
				   *lpc,
				   &pp->colour,
				   &pp->specular);
			}

			lpc += 1;
		}
		else
		{
			pp->colour   = NIGHT_amb_d3d_colour;
			pp->specular = NIGHT_amb_d3d_specular;

			pp->colour   &= ~POLY_colour_restrict;
			pp->specular &= ~POLY_colour_restrict;
		}

		mp1 += 1;
		mp2 += 1;
	}

	//
	// The quads.
	//

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - sp;
		p1 = p_f4->Points[1] - sp;
		p2 = p_f4->Points[2] - sp;
		p3 = p_f4->Points[3] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

		quad[0] = &POLY_buffer[p0];
		quad[1] = &POLY_buffer[p1];
		quad[2] = &POLY_buffer[p2];
		quad[3] = &POLY_buffer[p3];

		if (POLY_valid_quad(quad))
		{
			if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
			{
				if(p_f4->FaceFlags&FACE_FLAG_ANIMATE)
				{
					struct	AnimTmap	*p_a;
					SLONG	cur;
					p_a=&anim_tmaps[p_f4->TexturePage];
					cur=p_a->Current;


					quad[0]->u = float(p_a->UV[cur][0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_a->UV[cur][0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_a->UV[cur][1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_a->UV[cur][1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_a->UV[cur][2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_a->UV[cur][2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_a->UV[cur][3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_a->UV[cur][3][1]       ) * (1.0F / 32.0F);

					page   = p_a->UV[cur][0][0] & 0xc0;
					page <<= 2;
					page  |= p_a->Page[cur];



				}
				else
				{

					quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
					quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);

					quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
					quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);

					quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
					quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

					quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
					quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

					page   = p_f4->UV[0][0] & 0xc0;
					page <<= 2;
					page  |= p_f4->TexturePage;
				}

				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}
	}

	//
	// The triangles.
	//

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - sp;
		p1 = p_f3->Points[1] - sp;
		p2 = p_f3->Points[2] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

		tri[0] = &POLY_buffer[p0];
		tri[1] = &POLY_buffer[p1];
		tri[2] = &POLY_buffer[p2];

		if (POLY_valid_triangle(tri))
		{
			if (p_f3->DrawFlags & POLY_FLAG_TEXTURED)
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												       
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												       
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}
	}
}


//
// The reflected-object data structures.
//

typedef struct
{
	float x;
	float y;
	float z;
	SLONG fade;

} MESH_Point;

typedef struct
{
	UWORD p[3];
	float u[3];
	float v[3];
	UWORD page;
	UWORD padding;

} MESH_Face;

typedef struct
{
	SLONG calculated;

	SLONG max_points;
	SLONG max_faces;

	SLONG num_points;
	SLONG num_faces;

	MESH_Point *mp;
	MESH_Face  *mf;

} MESH_Reflection;

#define MESH_MAX_REFLECTIONS 256

MESH_Reflection MESH_reflection[MESH_MAX_REFLECTIONS];

void MESH_init_reflections()
{
	SLONG i;
	
	MESH_Reflection *mr;

	for (i = 0; i < MESH_MAX_REFLECTIONS; i++)
	{
		mr = &MESH_reflection[i];

		if (mr->calculated)
		{
			mr->calculated = FALSE;

			MemFree(mr->mp);
			MemFree(mr->mf);

			mr->num_points = 0;
			mr->num_faces  = 0;

			mr->max_points = 0;
			mr->max_faces  = 0;
		}
	}
}


//
// Grows the points or faces arrays.
//

void MESH_grow_points(MESH_Reflection *mr)
{
	mr->max_points *= 2;
	mr->mp          = (MESH_Point *) realloc(mr->mp, sizeof(MESH_Point) * mr->max_points);
}

void MESH_grow_faces(MESH_Reflection *mr)
{
	mr->max_faces *= 2;
	mr->mf          = (MESH_Face *) realloc(mr->mf, sizeof(MESH_Face) * mr->max_faces);
}

//
// Returns the index of a point with the given position.
//

SLONG MESH_add_point(
		MESH_Reflection *mr,
		float x,
		float y,
		float z,
		float fade)
{
	SLONG i;

	float dx;
	float dy;
	float dz;
	float dist;

	SLONG ans;

	for (i = mr->num_points - 1; i >= 0; i--)
	{
		dx = fabs(mr->mp[i].x - x);
		dy = fabs(mr->mp[i].y - y);
		dz = fabs(mr->mp[i].z - z);

		dist = dx + dy + dz;

		if (dist < 0.1F)
		{
			//
			// These points are just about the same!
			//

			return i;
		}
	}

	//
	// We must create a new point.
	// 

	if (mr->num_points >= mr->max_points)
	{
		MESH_grow_points(mr);
	}

	ans = mr->num_points;

	mr->mp[ans].x    = x;
	mr->mp[ans].y    = y;
	mr->mp[ans].z    = z;
	mr->mp[ans].fade = MIN(255,(SLONG)fade);

	mr->num_points += 1;

	return ans;
}

//
// Adds a face.
//

void MESH_add_face(
		MESH_Reflection *mr,
		SLONG p1,
		float u1,
		float v1,
		SLONG p2,
		float u2,
		float v2,
		SLONG p3,
		float u3,
		float v3,
		SLONG page)
{
	if (mr->num_faces >= mr->max_faces)
	{
		MESH_grow_faces(mr);
	}

	mr->mf[mr->num_faces].p[0] = p1;
	mr->mf[mr->num_faces].p[1] = p2;
	mr->mf[mr->num_faces].p[2] = p3;

	mr->mf[mr->num_faces].u[0] = u1;
	mr->mf[mr->num_faces].u[1] = u2;
	mr->mf[mr->num_faces].u[2] = u3;

	mr->mf[mr->num_faces].v[0] = v1;
	mr->mf[mr->num_faces].v[1] = v2;
	mr->mf[mr->num_faces].v[2] = v3;

	mr->mf[mr->num_faces].page = page;

	mr->num_faces += 1;
}


//
// Adds a polygon to the given reflection. Give the points in
// clockwise (or anti-clockwise) order.
//

typedef struct
{
	float x;
	float y;
	float z;
	float u;
	float v;
	float fade;		// 0 - 255 indicating how deep the point is under water.
	SLONG index;	// Dont worry about this field when calling MESH_add_poly()

} MESH_Add;

#define MESH_MAX_ADD 256

MESH_Add MESH_add[MESH_MAX_ADD];
SLONG    MESH_add_upto;

void MESH_add_poly(MESH_Reflection *mr, MESH_Add poly[], SLONG num_points, SLONG page)
{
	SLONG i;
	SLONG bot;
	SLONG top;
	SLONG p1;
	SLONG p2;
	SLONG last;
	SLONG count;

	float f;
	float tween;
	float top_height;
	float bot_height;
	float dx;
	float dy;
	float dz;
	float du;
	float dv;
	float dfade;
	float x;
	float y;
	float z;
	float u;
	float v;
	float fade;
	float mul;

	MESH_Add *ma;
	MESH_Add *mp1;
	MESH_Add *mp2;
	MESH_Add *mpl;

	ASSERT(num_points > 2);

	//
	// Work out all the points going around the poly.
	//

	MESH_add_upto = 0;

	for (i = 0; i < num_points; i++)
	{
		p1 = i + 0;
		p2 = i + 1;

		if (p2 >= num_points) {p2 = 0;}

		mp1 = &poly[p1];
		mp2 = &poly[p2];

		dx    = mp2->x    - mp1->x;
		dy    = mp2->y    - mp1->y;
		dz    = mp2->z    - mp1->z;
		du    = mp2->u    - mp1->u;
		dv    = mp2->v    - mp1->v;
		dfade = mp2->fade - mp1->fade;

		//
		// Make that each point is not too far in y from the previous one.
		//

		tween = fabs(dy * (1.0F / 8.0F));
		tween = floor(tween);

		if (tween < 1.0F) {tween = 1.0F;}

		for (f = 0.0F; f < tween; f += 1.0F)
		{
			mul = f * (1.0F / tween);

			x    = mp1->x    + mul * dx;
			y    = mp1->y    + mul * dy;
			z    = mp1->z    + mul * dz;
			u    = mp1->u    + mul * du;
			v    = mp1->v    + mul * dv;
			fade = mp1->fade + mul * dfade;

			ASSERT(WITHIN(MESH_add_upto, 0, MESH_MAX_ADD - 1));

			ma = &MESH_add[MESH_add_upto++];

			ma->x     = x;
			ma->y     = y;
			ma->z     = z;
			ma->u     = u;
			ma->v     = v;
			ma->fade  = fade;
			ma->index = INFINITY;	// Mark as not having an index.
		}
	}

	//
	// Which is the highest and lowest point?
	// 

	top        = 0;
	bot        = 0;
	top_height = MESH_add[0].y;
	bot_height = MESH_add[0].y;

	for (i = 1; i < MESH_add_upto; i++)
	{
		if (MESH_add[i].y > top_height)
		{
			top        = i;
			top_height = MESH_add[i].y;
		}

		if (MESH_add[i].y < bot_height)
		{
			bot        = i;
			bot_height = MESH_add[i].y;
		}
	}

	#define MESH_NEXT_CLOCK(p) (((p) + 1 >= MESH_add_upto) ?       0 : (p) + 1)
	#define MESH_NEXT_ANTI(p)  (((p) - 1 >=             0) ? (p) - 1 : MESH_add_upto - 1)

	//
	// Create triangles from the points.
	//

	p1 = top;
	p2 = MESH_NEXT_ANTI(top);

	count = 0;

	while(1)
	{
		ASSERT(count++ < 2048);

		ASSERT(WITHIN(p1, 0, MESH_add_upto - 1));
		ASSERT(WITHIN(p2, 0, MESH_add_upto - 1));
	
		mp1 = &MESH_add[p1];
		mp2 = &MESH_add[p2];

		//
		// Move on the highest point.
		//

		if (p1 != bot && mp1->y >= mp2->y)
		{
			last = p1;
			p1   = MESH_NEXT_CLOCK(p1);
		}
		else
		{
			last = p2;
			p2   = MESH_NEXT_ANTI(p2);
		}

		ASSERT(WITHIN(p1,   0, MESH_add_upto - 1));
		ASSERT(WITHIN(p2,   0, MESH_add_upto - 1));
		ASSERT(WITHIN(last, 0, MESH_add_upto - 1));
	
		mp1 = &MESH_add[p1];
		mp2 = &MESH_add[p2];
		mpl = &MESH_add[last];

		if (mp1->fade >= 255.0F &&
			mp2->fade >= 255.0F &&
			mpl->fade >= 255.0F)
		{
			//
			// This face is completely faded out- and since we are moving
			// deeper into the reflection, there is no need to fade out
			// any other points.
			//

			break;
		}
			

		//
		// Make sure each of the points has been added to the object.
		//

		if (mp1->index == INFINITY) {mp1->index = MESH_add_point(mr, mp1->x, mp1->y, mp1->z, mp1->fade);}
		if (mp2->index == INFINITY) {mp2->index = MESH_add_point(mr, mp2->x, mp2->y, mp2->z, mp2->fade);}
		if (mpl->index == INFINITY) {mpl->index = MESH_add_point(mr, mpl->x, mpl->y, mpl->z, mpl->fade);}

		//
		// Create a new triangle.
		//

		MESH_add_face(
			mr,
			mpl->index,
			mpl->u,
			mpl->v,
			mp1->index,
			mp1->u,
			mp1->v,
			mp2->index,
			mp2->u,
			mp2->v,
			page);

		if (p1 == bot &&
			p2 == bot)
		{
			break;
		}
	}
}



//
// Creates a reflection for the given mesh prim.
//

void MESH_create_reflection(SLONG prim)
{
	SLONG i;
	SLONG j;
	SLONG pt;
	SLONG page;

	UBYTE order3[3] = {0, 2, 1};
	UBYTE order4[4] = {0, 2, 3, 1};

	float dy;
	float fade;
	float reflect_height;

	PrimInfo        *pi;
	PrimObject      *po;
	PrimFace3       *f3;
	PrimFace4       *f4;
	MESH_Reflection *mr;

	MESH_Add ma[4];

	ASSERT(WITHIN(prim, 0, MESH_MAX_REFLECTIONS - 1));

	mr = &MESH_reflection[prim];

	ASSERT(!mr->calculated);

	po = &prim_objects[prim];
	pi =  get_prim_info(prim);
	
	//
	// Initialise it.
	//

	mr->max_points = 32;
	mr->max_faces  = 32;

	mr->num_points = 0;
	mr->num_faces  = 0;

	mr->mp = (MESH_Point *) MemAlloc(mr->max_points * sizeof(MESH_Point));
	mr->mf = (MESH_Face  *) MemAlloc(mr->max_faces  * sizeof(MESH_Face ));

	//
	// The height about which the reflection takes place.
	// 

	reflect_height = float(pi->miny);

	//
	// The maximum height of a reflection.
	//

	#define MESH_MAX_DY					(128.0F)
	#define MESH_255_DIVIDED_BY_MAX_DY	(255.0F / MESH_MAX_DY)

	//
	// Triangles.
	//

	for (i = po->StartFace3; i < po->EndFace3; i++)
	{
		f3 = &prim_faces3[i];

		for (j = 0; j < 3; j++)
		{
			pt = order3[j];

			ma[j].x    = AENG_dx_prim_points[f3->Points[pt]].X;
			ma[j].y    = AENG_dx_prim_points[f3->Points[pt]].Y;
			ma[j].z    = AENG_dx_prim_points[f3->Points[pt]].Z;
			ma[j].u    = float(f3->UV[pt][0] & 0x3f) * (1.0F / 32.0F);
			ma[j].v    = float(f3->UV[pt][1]       ) * (1.0F / 32.0F);
			dy         = ma[j].y - reflect_height;
			fade       = dy * MESH_255_DIVIDED_BY_MAX_DY;
			ma[j].fade = fade;
			ma[j].y    = reflect_height - dy;

			ASSERT(ma[j].fade >= 0.0F);
		}

		if (ma[0].fade >= 255.0F &&
			ma[1].fade >= 255.0F &&
			ma[2].fade >= 255.0F)
		{
			//
			// This whole poly is faded out.
			// 
		}
		else
		{
			page   = f3->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= f3->TexturePage;

			MESH_add_poly(mr, ma, 3, page);
		}
	}

	//
	// Quads.
	//

	for (i = po->StartFace4; i < po->EndFace4; i++)
	{
		f4 = &prim_faces4[i];

		for (j = 0; j < 4; j++)
		{
			pt = order4[j];

			ma[j].x    = AENG_dx_prim_points[f4->Points[pt]].X;
			ma[j].y    = AENG_dx_prim_points[f4->Points[pt]].Y;
			ma[j].z    = AENG_dx_prim_points[f4->Points[pt]].Z;
			ma[j].u    = float(f4->UV[pt][0] & 0x3f) * (1.0F / 32.0F);
			ma[j].v    = float(f4->UV[pt][1]       ) * (1.0F / 32.0F);
			dy         = ma[j].y - reflect_height;
			fade       = dy * MESH_255_DIVIDED_BY_MAX_DY;
			ma[j].fade = fade;
			ma[j].y    = reflect_height - dy;

			ASSERT(ma[j].fade >= 0.0F);
		}

		if (ma[0].fade >= 255.0F &&
			ma[1].fade >= 255.0F &&
			ma[2].fade >= 255.0F &&
			ma[3].fade >= 255.0F)
		{
			//
			// This whole poly is faded out.
			// 
		}
		else
		{
			page   = f4->UV[0][0] & 0xc0;
			page <<= 2;
			page  |= f4->TexturePage;

			MESH_add_poly(mr, ma, 4, page);
		}
	}

	mr->calculated = TRUE;
}



void MESH_draw_reflection(
		SLONG         prim,
		SLONG         at_x,
		SLONG         at_y,
		SLONG         at_z,
		SLONG         yaw,
		NIGHT_Colour *lpc)
{
	SLONG i;
//	SLONG j;
	SLONG fog;

	float px;
	float py;
	float pz;

	float matrix[9];

	POLY_Point      *pp;
	POLY_Point      *tri[3];
	MESH_Face       *mf;
	MESH_Reflection *mr;
	
	if(prim==83)
	{
		return;
	}

	ASSERT(WITHIN(prim, 0, MESH_MAX_REFLECTIONS - 1));

	mr = &MESH_reflection[prim];

	//
	// If we haven't worked out the reflection mesh,
	// then do so now.
	//

	if (!mr->calculated)
	{
		MESH_create_reflection(prim);
	}

	//
	// This mesh's rotation matrix.
	// 

	MATRIX_calc(
		matrix,
		float(yaw) * (2.0F * PI / 2048.0F),
		0,
		0);

	MATRIX_TRANSPOSE(matrix);

	POLY_set_local_rotation(
		float(at_x),
		float(at_y),
		float(at_z),
		matrix);

	//
	// Rotate all the points into the POLY_buffer.
	//

	for (i = 0; i < mr->num_points; i++)
	{
		pp = &POLY_buffer[i];

		px = mr->mp[i].x;
		py = mr->mp[i].y;
		pz = mr->mp[i].z;

		POLY_transform_using_local_rotation_and_wibble(
			px,
			py,
			pz,
			pp,
			mr->mp[i].fade);

		if (lpc)
		{
			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
				NIGHT_get_d3d_colour(
				   *lpc,
				   &pp->colour,
				   &pp->specular);
			}

			lpc += 1;
		}
		else
		{
		    pp->colour   = NIGHT_amb_d3d_colour;
		    pp->specular = NIGHT_amb_d3d_specular;
		}

		if (pp->clip & POLY_CLIP_TRANSFORMED)
		{
			POLY_fadeout_point(pp);

			//
			// Fade out with depth too.
			// 

			fog   = pp->specular >> 24;
			fog  *= 255 - mr->mp[i].fade;
			fog >>= 8;

			pp->specular &= 0x00ffffff;
			pp->specular |= fog << 24;
		}
	}

	//
	// Add all the faces.
	//

	for (i = 0; i < mr->num_faces; i++)
	{
		mf = &mr->mf[i];

		ASSERT(WITHIN(mf->p[0], 0, mr->num_points - 1));
		ASSERT(WITHIN(mf->p[1], 0, mr->num_points - 1));
		ASSERT(WITHIN(mf->p[2], 0, mr->num_points - 1));

		tri[0] = &POLY_buffer[mf->p[0]];
		tri[1] = &POLY_buffer[mf->p[1]];
		tri[2] = &POLY_buffer[mf->p[2]];

		if (POLY_valid_triangle(tri))
		{
			tri[0]->u = mf->u[0];
			tri[0]->v = mf->v[0];

			tri[1]->u = mf->u[1];
			tri[1]->v = mf->v[1];

			tri[2]->u = mf->u[2];
			tri[2]->v = mf->v[2];

			POLY_add_triangle(tri, mf->page, TRUE);
		}
	}
}





