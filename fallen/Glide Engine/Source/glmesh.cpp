//
// Drawing rotating prims.
//

#include "game.h"
#include "aeng.h"
#include "matrix.h"
#include "mesh.h"
#include "c:\fallen\headers\light.h"
#include "glpoly.h"


#define	POLY_FLAG_GOURAD		(1<<0)
#define	POLY_FLAG_TEXTURED		(1<<1)
#define	POLY_FLAG_MASKED		(1<<2)
#define	POLY_FLAG_SEMI_TRANS	(1<<3)
#define	POLY_FLAG_ALPHA			(1<<4)
#define	POLY_FLAG_TILED			(1<<5)
#define	POLY_FLAG_DOUBLESIDED	(1<<6)
#define	POLY_FLAG_CLIPPED		(1<<7)


void MESH_draw_poly(
		SLONG         prim,
		MAPCO16	      at_x,
		MAPCO16       at_y,
		MAPCO16	      at_z,
		SLONG         i_yaw,
		SLONG         i_pitch,
		SLONG         i_roll,
		LIGHT_Colour *lpc)
{
	SLONG i;
	SLONG j;

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
	POLY_Point *ps;

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

		if (pp->clip & POLY_CLIP_TRANSFORMED)
		{
			pp->colour = LIGHT_get_glide_colour(*lpc);
		}

		lpc += 1;
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
				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * 8.0F;
				quad[0]->v = float(p_f4->UV[0][1]       ) * 8.0F;

				quad[1]->u = float(p_f4->UV[1][0]       ) * 8.0F;
				quad[1]->v = float(p_f4->UV[1][1]       ) * 8.0F;

				quad[2]->u = float(p_f4->UV[2][0]       ) * 8.0F;
				quad[2]->v = float(p_f4->UV[2][1]       ) * 8.0F;

				quad[3]->u = float(p_f4->UV[3][0]       ) * 8.0F;
				quad[3]->v = float(p_f4->UV[3][1]       ) * 8.0F;

				page  =  p_f4->TexturePage;
				page |= (p_f4->UV[0][0] << 2) & ~0xff;

				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			//else
			//{
			//	POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			//}
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
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * 8.0F;
				tri[0]->v = float(p_f3->UV[0][1]       ) * 8.0F;

				tri[1]->u = float(p_f3->UV[1][0]       ) * 8.0F;
				tri[1]->v = float(p_f3->UV[1][1]       ) * 8.0F;

				tri[2]->u = float(p_f3->UV[2][0]       ) * 8.0F;
				tri[2]->v = float(p_f3->UV[2][1]       ) * 8.0F;

				page  =  p_f3->TexturePage;
				page |= (p_f3->UV[0][0] << 2) & ~0xff;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			//else
			//{
			//	POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			//}
		}
	}
}






