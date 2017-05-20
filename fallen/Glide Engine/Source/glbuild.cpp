//
// Draws buildings.
//

#include "game.h"
#include "aeng.h"
#include "glpoly.h"
#include "light.h"
#include "build.h"

void BUILD_draw(Thing *p_thing)
{
	SLONG i;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	SLONG page;
	SLONG backface_cull;
	ULONG shadow;
	ULONG face_colour;
	ULONG face_specular;

	float bx = float(p_thing->WorldPos.X >> 8);
	float by = float(p_thing->WorldPos.Y >> 8);
	float bz = float(p_thing->WorldPos.Z >> 8);

	SLONG bo_index;
	SLONG bf_index;

	BuildingFacet  *bf;
	BuildingObject *bo;
	
	bo_index =  p_thing->Index;
	bo       = &building_objects[bo_index];

	//
	// Points out of the ambient light.
	//

	shadow =
		((LIGHT_amb_colour.red   >> 1) << 16) |
		((LIGHT_amb_colour.green >> 1) <<  8) |
		((LIGHT_amb_colour.blue  >> 1) <<  0);

	//
	// The ambient light colour.
	//

	ULONG colour;

	colour = LIGHT_get_glide_colour(LIGHT_amb_colour);

	//
	// ONLY TRIANGLES USE THESE VALUES. MAKE THEM STAND OUT.
	// 

	colour = 0xffffffff;

	//
	// Draw each facet.
	//

	bf_index = bo->FacetHead;

	while(bf_index)
	{
		bf = &building_facets[bf_index];

		//
		// Rotate all the points.
		//

		sp = bf->StartPoint;
		ep = bf->EndPoint;

		POLY_buffer_upto = 0;

		for (i = sp; i < ep; i++)
		{
			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			pp = &POLY_buffer[POLY_buffer_upto++];

			POLY_transform(
				AENG_dx_prim_points[i].X + bx,
				AENG_dx_prim_points[i].Y + by,
				AENG_dx_prim_points[i].Z + bz,
				pp);

			if (pp->clip & POLY_CLIP_TRANSFORMED)
			{
				pp->colour = LIGHT_get_glide_colour(LIGHT_building_point[i]);

				POLY_fadeout_point(pp);
			}
		}

		//
		// Draw all the quads.
		//

		for (i = bf->StartFace4; i < bf->EndFace4; i++)
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
				//
				// Should this poly be backface culled?
				//

				backface_cull = !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED);

				//
				// The texture page to use.
				//

				page = p_f4->TexturePage | ((p_f4->UV[0][0] << 2) & ~0xff);

				//
				// Texture the quad.
				// 

				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * 8.0F;
				quad[0]->v = float(p_f4->UV[0][1]       ) * 8.0F;

				quad[1]->u = float(p_f4->UV[1][0]       ) * 8.0F;
				quad[1]->v = float(p_f4->UV[1][1]       ) * 8.0F;

				quad[2]->u = float(p_f4->UV[2][0]       ) * 8.0F;
				quad[2]->v = float(p_f4->UV[2][1]       ) * 8.0F;

				quad[3]->u = float(p_f4->UV[3][0]       ) * 8.0F;
				quad[3]->v = float(p_f4->UV[3][1]       ) * 8.0F;

				if (p_f4->FaceFlags & (FACE_FLAG_SHADOW_1 | FACE_FLAG_SHADOW_2))
				{
					POLY_Point ps[4];

					//
					// Create four darkened points.
					// 

					ps[0] = *(quad[0]);
					ps[1] = *(quad[1]);
					ps[2] = *(quad[2]);
					ps[3] = *(quad[3]);

					//
					// Darken the points.
					//

					ps[0].colour >>= 1;
					ps[1].colour >>= 1;
					ps[2].colour >>= 1;
					ps[3].colour >>= 1;

					ps[0].colour &= 0xff7f7f7f;
					ps[1].colour &= 0xff7f7f7f;
					ps[2].colour &= 0xff7f7f7f;
					ps[3].colour &= 0xff7f7f7f;
	
					if ((p_f4->FaceFlags & FACE_FLAG_SHADOW_1) &&
						(p_f4->FaceFlags & FACE_FLAG_SHADOW_2))
					{
						quad[0] = &ps[0];
						quad[1] = &ps[1];
						quad[2] = &ps[2];
						quad[3] = &ps[3];

						POLY_add_quad(quad, page, backface_cull);
					}
					else
					{
						if (p_f4->FaceFlags & FACE_FLAG_SHADOW_2)
						{
							tri[0] = quad[0];
							tri[1] = quad[1];
							tri[2] = quad[2];

							POLY_add_triangle(tri, page, backface_cull);

							tri[0] = &ps[1];
							tri[1] = &ps[3];
							tri[2] = &ps[2];

							POLY_add_triangle(tri, page, backface_cull);
						}
						else
						{
							tri[0] = quad[1];
							tri[1] = quad[3];
							tri[2] = quad[2];

							POLY_add_triangle(tri, page, backface_cull);

							tri[0] = &ps[0];
							tri[1] = &ps[1];
							tri[2] = &ps[2];

							POLY_add_triangle(tri, page, backface_cull);
						}
					}
				}
				else
				{
					POLY_add_quad(quad, page, backface_cull);
				}
			}
		}

		//
		// Draw all the triangles.
		//

		for (i = bf->StartFace3; i < bf->EndFace3; i++)
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
				//
				// Should this poly be backface culled?
				//

				backface_cull = !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED);

				//
				// The texture page to use.
				//

				page = p_f3->TexturePage | ((p_f3->UV[0][0] << 2) & ~0xff);

				//
				// Texture the triangle.
				// 

				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * 8.0F;
				tri[0]->v = float(p_f3->UV[0][1]       ) * 8.0F;

				tri[1]->u = float(p_f3->UV[1][0]       ) * 8.0F;
				tri[1]->v = float(p_f3->UV[1][1]       ) * 8.0F;

				tri[2]->u = float(p_f3->UV[2][0]       ) * 8.0F;
				tri[2]->v = float(p_f3->UV[2][1]       ) * 8.0F;

				POLY_add_triangle(tri, page, backface_cull);
			}
		}
		
		bf_index = bf->NextFacet;
	}
}

#if DONT_WORRY_ABOUT_INSIDES_FOR_NOW

void BUILD_draw_inside()
{
	Thing *p_thing = TO_THING(INDOORS_THING);

	SLONG i;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	
	float max_height;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	ULONG amb_colour;
	ULONG amb_specular;

	float bx = float(p_thing->WorldPos.X >> 8);
	float by = float(p_thing->WorldPos.Y >> 8);
	float bz = float(p_thing->WorldPos.Z >> 8);

	SLONG bo_index;
	SLONG bf_index;

	BuildingFacet  *bf;
	BuildingObject *bo;
	
	bo_index =  p_thing->Index;
	bo       = &building_objects[bo_index];

	//
	// The ambient light colour.
	//

	ULONG colour;
	ULONG specular;

	LIGHT_get_d3d_colour(
		LIGHT_amb_colour,
	   &colour,
	   &specular);

	//
	// Draw each facet.
	//

	bf_index = bo->FacetHead;

	while(bf_index)
	{
		bf = &building_facets[bf_index];

		if (bf->FacetFlags & FACET_FLAG_ROOF)
		{
			max_height = float(INDOORS_HEIGHT_FLOOR + 32);
		}
		else
		{
			if (building_list[p_thing->BuildingList].BuildingType == BUILDING_TYPE_WAREHOUSE)
			{
				max_height = float(INDOORS_HEIGHT_CEILING + 256 + 32);
			}
			else
			{
				max_height = float(INDOORS_HEIGHT_CEILING + 32);
			}
		}

		//
		// Rotate all the points.
		//

		sp = bf->StartPoint;
		ep = bf->EndPoint;

		POLY_buffer_upto = 0;

		for (i = sp; i < ep; i++)
		{
			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			pp = &POLY_buffer[POLY_buffer_upto++];

			if (AENG_dx_prim_points[i].Y + by <= max_height)
			{
				POLY_transform(
					AENG_dx_prim_points[i].X + bx,
					AENG_dx_prim_points[i].Y + by,
					AENG_dx_prim_points[i].Z + bz,
					pp);
				
				if (pp->clip & POLY_CLIP_TRANSFORMED)
				{
					pp->colour   = colour;
					pp->specular = specular;

					POLY_fadeout_point(pp);
				}
			}
			else
			{
				pp->clip = 0;
			}
		}

		//
		// Draw all the quads.
		//

		for (i = bf->StartFace4; i < bf->EndFace4; i++)
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
				quad[0]->u = float(p_f4->UV[0][0]) * (1.0F / 256.0F);
				quad[0]->v = float(p_f4->UV[0][1]) * (1.0F / 256.0F);

				quad[1]->u = float(p_f4->UV[1][0]) * (1.0F / 256.0F);
				quad[1]->v = float(p_f4->UV[1][1]) * (1.0F / 256.0F);

				quad[2]->u = float(p_f4->UV[2][0]) * (1.0F / 256.0F);
				quad[2]->v = float(p_f4->UV[2][1]) * (1.0F / 256.0F);

				quad[3]->u = float(p_f4->UV[3][0]) * (1.0F / 256.0F);
				quad[3]->v = float(p_f4->UV[3][1]) * (1.0F / 256.0F);

				POLY_add_quad(quad, p_f4->TexturePage, TRUE);
			}
		}


		//
		// Draw all the quads.
		//

		for (i = bf->StartFace3; i < bf->EndFace3; i++)
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
				tri[0]->u = float(p_f3->UV[0][0]) * (1.0F / 256.0F);
				tri[0]->v = float(p_f3->UV[0][1]) * (1.0F / 256.0F);

				tri[1]->u = float(p_f3->UV[1][0]) * (1.0F / 256.0F);
				tri[1]->v = float(p_f3->UV[1][1]) * (1.0F / 256.0F);

				tri[2]->u = float(p_f3->UV[2][0]) * (1.0F / 256.0F);
				tri[2]->v = float(p_f3->UV[2][1]) * (1.0F / 256.0F);

				POLY_add_triangle(tri, p_f3->TexturePage, TRUE);
			}
		}
		
		bf_index = bf->NextFacet;
	}
}

#endif






