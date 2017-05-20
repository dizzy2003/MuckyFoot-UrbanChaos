//
// Another engine.
//

#define __MSC__

#include <MFStdLib.h>
#include "game.h"
#include <glide.h>
#include "glpoly.h"
#include "ngamut.h"
#include "light.h"
#include "gltexture.h"
#include "matrix.h"
#include "figure.h"
#include "shadow.h"
#include "build.h"
#include "mesh.h"
#include "mist.h"

//
// The shift of the floor...
//

#define ALT_SHIFT (3)

//
// The maximum draw distance.
//

float AENG_lens = 3.3F;

#define AENG_DRAW_DIST 22
#define AENG_LENS      (AENG_lens)

//
// The camera.
// 

float AENG_cam_x;
float AENG_cam_y;
float AENG_cam_z;

float AENG_cam_yaw;
float AENG_cam_pitch;
float AENG_cam_roll;

//
// Clearing the screen to the correct value.
//

FxI32 AENG_wrange[2];

//
// Call once at the start of the whole program.
//

void AENG_init()
{
	POLY_init();
	TEXTURE_load();

	//
	// What is the wbuffer range?
	//

    grGet(GR_WDEPTH_MIN_MAX, sizeof(AENG_wrange), AENG_wrange);
}

//
// This function makes a local copy of the prim points for
// the engine's own use.
//

SVECTOR_F AENG_dx_prim_points[MAX_PRIM_POINTS];

void AENG_create_dx_prim_points()
{
	SLONG i;

	for (i = 0; i < MAX_PRIM_POINTS; i++)
	{
		AENG_dx_prim_points[i].X = float(prim_points[i].X);
		AENG_dx_prim_points[i].Y = float(prim_points[i].Y);
		AENG_dx_prim_points[i].Z = float(prim_points[i].Z);
	}
}

//
// Initialises a new frame.
// Draws the engine.
//

void AENG_set_camera(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll)
{
	float radians_yaw   = float(yaw  ) * (2.0F * PI / 2048.0F);
	float radians_pitch = float(pitch) * (2.0F * PI / 2048.0F);
	float radians_roll  = float(roll ) * (2.0F * PI / 2048.0F);

	AENG_set_camera_radians(
		world_x,
		world_y,
		world_z,
		radians_yaw,
		radians_pitch,
		radians_roll);
}

void AENG_set_camera_radians(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		float yaw,
		float pitch,
		float roll)
{
	AENG_cam_x = float(world_x);
	AENG_cam_y = float(world_y);
	AENG_cam_z = float(world_z);

	AENG_cam_yaw   = yaw;
	AENG_cam_pitch = pitch;
	AENG_cam_roll  = roll;

	POLY_camera_set(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		float(AENG_DRAW_DIST) * 256.0F,
		AENG_LENS);
}

//
// Draws world lines.
//

void AENG_world_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front)
{
	POLY_Point p1;
	POLY_Point p2;

	POLY_transform(float(x1), float(y1), float(z1), &p1);
	POLY_transform(float(x2), float(y2), float(z2), &p2);

	if (POLY_valid_line(&p1, &p2))
	{
		p1.colour   = colour1;
		p2.colour   = colour2;

		POLY_add_line(&p1, &p2, float(width1), float(width2), sort_to_front);
	}
}

//
// Older engine compatability.
//

void AENG_e_draw_3d_line           (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2) {}
void AENG_e_draw_3d_line_dir       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2) {}
void AENG_e_draw_3d_line_col       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b) {}
void AENG_e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b) {}
void AENG_e_draw_3d_mapwho         (SLONG x1,SLONG z1) {}
void AENG_e_draw_3d_mapwho_y       (SLONG x1,SLONG y1,SLONG z1) {}




void AENG_calc_gamut()
{
	float width;
	float height;
	float depth;
	float aspect;
	float matrix[9];

	struct
	{
		float x;
		float z;

	} cone[5];
	
	MATRIX_calc(
		matrix,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll);

	//
	// The dimensions of the view pyramid.
	//

	width  = AENG_DRAW_DIST;
	height = AENG_DRAW_DIST;
	depth  = AENG_DRAW_DIST;

	width *= 640.0F;
	width /= 480.0F;

	width  /= AENG_LENS;
	height /= AENG_LENS;

	//
	// Finds the points of the cone in world space
	//

	cone[3].x = cone[4].x = AENG_cam_x / 256.0;
	cone[3].z = cone[4].z = AENG_cam_z / 256.0;

	cone[3].x += depth * matrix[6];
	cone[3].z += depth * matrix[8];

	//
	// cone[0] is the top left corner...
	//

	cone[0].x = cone[3].x + height * matrix[3];
	cone[0].z = cone[3].z + height * matrix[5];

	cone[0].x = cone[0].x - width *  matrix[0];
	cone[0].z = cone[0].z - width *  matrix[2];

	//
	// cone[1] is the top right corner...
	//

	cone[1].x = cone[3].x + height * matrix[3];
	cone[1].z = cone[3].z + height * matrix[5];

	cone[1].x = cone[1].x + width *  matrix[0];
	cone[1].z = cone[1].z + width *  matrix[2];

	//
	// cone[2] is the bottom right corner...
	//

	cone[2].x = cone[3].x - height * matrix[3];
	cone[2].z = cone[3].z - height * matrix[5];

	cone[2].x = cone[2].x + width *  matrix[0];
	cone[2].z = cone[2].z + width *  matrix[2];

	//
	// cone[3] is the bottom left corner...
	//

	cone[3].x = cone[3].x - height * matrix[3];
	cone[3].z = cone[3].z - height * matrix[5];

	cone[3].x = cone[3].x - width *  matrix[0];
	cone[3].z = cone[3].z - width *  matrix[2];

	//
	// Create the gamut.
	//

	NGAMUT_init();

	NGAMUT_add_line(cone[4].x, cone[4].z, cone[0].x, cone[0].z);
	NGAMUT_add_line(cone[4].x, cone[4].z, cone[1].x, cone[1].z);
	NGAMUT_add_line(cone[4].x, cone[4].z, cone[2].x, cone[2].z);
	NGAMUT_add_line(cone[4].x, cone[4].z, cone[3].x, cone[3].z);

	NGAMUT_add_line(cone[0].x, cone[0].z, cone[1].x, cone[1].z);
	NGAMUT_add_line(cone[1].x, cone[1].z, cone[2].x, cone[2].z);
	NGAMUT_add_line(cone[2].x, cone[2].z, cone[3].x, cone[3].z);
	NGAMUT_add_line(cone[3].x, cone[3].z, cone[0].x, cone[0].z);

	NGAMUT_calculate_point_gamut();
}



//
// The new engines.
// 

POLY_Point AENG_upper[MAP_WIDTH][MAP_HEIGHT];
POLY_Point AENG_lower[MAP_WIDTH][MAP_HEIGHT];

void AENG_draw()
{
	SLONG i;

	SLONG x;
	SLONG z;
	
	SLONG nx;
	SLONG nz;

	SLONG page;
	SLONG shadow;

	float world_x;
	float world_y;
	float world_z;

	POLY_Point *pp;
	POLY_Point *ppl;
	MapElement *me;

	THING_INDEX t_index;
	Thing      *p_thing;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	if (Keys[KB_Y]) {Keys[KB_Y] = 0; AENG_lens -= 0.1F;}
	if (Keys[KB_U]) {Keys[KB_U] = 0; AENG_lens += 0.1F;}

	//
	// In case we've fiddled with the renderstates ourselves.
	//

	TEXTURE_init_states();

	//
	// Create the gamut
	//

	AENG_calc_gamut();

	//
	// Points out of the ambient light.
	//

	shadow =
		((LIGHT_amb_colour.red   >> 1) << 16) |
		((LIGHT_amb_colour.green >> 1) <<  8) |
		((LIGHT_amb_colour.blue  >> 1) <<  0);

	//
	// Rotate all the points.
	//

	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		for (x = NGAMUT_point_gamut[z].xmin; x <= NGAMUT_point_gamut[z].xmax; x++)
		{
			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

			me = &MAP[MAP_INDEX(x, z)];

			//
			// The upper point.
			//

			world_x = x       * 256.0F;
			world_y = me->Alt * float(1 << ALT_SHIFT);
			world_z = z       * 256.0F;

			if (!(me->Flags & FLOOR_NOUPPER))
			{
				pp = &AENG_upper[x][z];

				POLY_transform(world_x, world_y, world_z, pp);

				if (pp->clip & POLY_CLIP_TRANSFORMED)
				{
					pp->colour = LIGHT_get_glide_colour(me->Colour);
					
					POLY_fadeout_point(pp);
				}
			}

			//
			// The lower point.
			//

			if (me->Flags & FLOOR_SINK_POINT)
			{
				world_y -= 32.0F;

				ppl = &AENG_lower[x][z];

				POLY_transform(world_x, world_y, world_z, ppl);

				if (ppl->clip & POLY_CLIP_TRANSFORMED)
				{
					ppl->colour = LIGHT_get_glide_colour(me->Colour);

					POLY_fadeout_point(ppl);
				}
			}
		}
	}

	#ifdef DONT_IGNORE_SHADOWS

	//
	// Draw the players shadow map.
	//

	{
		Thing *darci = NET_PERSON(0);
		UBYTE *bitmap;
		UBYTE  u_res;
		UBYTE  v_res;

		bitmap = (UBYTE *) AENG_aa_buffer;
		u_res  = AENG_AA_BUF_SIZE;
		v_res  = AENG_AA_BUF_SIZE;

		memset(AENG_aa_buffer, 0, sizeof(AENG_aa_buffer));

		SMAP_person(
			darci,
			bitmap,
			u_res,
			v_res,
		    147,
		   -148,
		   -147);

		//
		// Plonk it into the top left of the shadow texture page.
		//

		if (TEXTURE_shadow_lock())
		{
			SLONG  x;
			SLONG  y;
			UWORD *line;
			UBYTE *buf = (UBYTE *) AENG_aa_buffer;

			for (y = 0; y < AENG_AA_BUF_SIZE; y++)
			{
				line = &TEXTURE_shadow_bitmap[y * TEXTURE_shadow_pitch >> 1];

				for (x = AENG_AA_BUF_SIZE - 1; x >= 0; x--)
				{
					#if WE_WANT_A_WHITE_SHADOW
					*line  =  0xffff;
					*line &= ~((0xff >> TEXTURE_shadow_mask_alpha)      << TEXTURE_shadow_shift_alpha);
					#else
					*line  = 0;
					#endif
					*line |=  (*buf >> (TEXTURE_shadow_mask_alpha + 1)) << TEXTURE_shadow_shift_alpha;

					*line++;
					*buf++;
				}
			}

			TEXTURE_shadow_unlock();
		}

		//
		// How we map floating points coordinates from 0 to 1 onto
		// where we plonked the shadow map in the texture page.
		//

		AENG_project_offset_u = 0.0F;
		AENG_project_offset_v = 0.0F;
		AENG_project_mul_u    = float(u_res) / float(TEXTURE_SHADOW_SIZE);
		AENG_project_mul_v    = float(v_res) / float(TEXTURE_SHADOW_SIZE);

		//
		// Map this poly onto the mapsquares surrounding darci.
		//

		SLONG i;

		SLONG mx;
		SLONG mz;
		SLONG dx;
		SLONG dz;

		MapElement *me[4];

		SVECTOR_F  poly[4];
		SMAP_Link *projected;

		SLONG v_list;
		SLONG i_vect;

		CollisionVect *p_vect;

		SLONG w_list;
		SLONG w_face;

		PrimFace4 *p_f4;
		PrimPoint *pp;

		SLONG wall;
		SLONG storey;
		SLONG building;
		SLONG thing;
		SLONG face_height;
		UBYTE face_order[4] = {0,1,3,2};

		Thing *p_fthing;

		//
		// Colvect we have already done.
		// 

		#define AENG_MAX_DONE 8

		SLONG done[AENG_MAX_DONE];
		SLONG done_upto = 0;

		for (dx = -1; dx <= 1; dx++)
		for (dz = -1; dz <= 1; dz++)
		{
			mx = (darci->WorldPos.X >> 16) + dx;
			mz = (darci->WorldPos.Z >> 16) + dz;

			if (WITHIN(mx, 0, MAP_WIDTH  - 2) &&
				WITHIN(mz, 0, MAP_HEIGHT - 2))
			{
				me[0] = &MAP[MAP_INDEX(mx + 0, mz + 0)];
				me[1] = &MAP[MAP_INDEX(mx + 1, mz + 0)];
				me[2] = &MAP[MAP_INDEX(mx + 1, mz + 1)];
				me[3] = &MAP[MAP_INDEX(mx + 0, mz + 1)];

				if (!(me[0]->Flags & FLOOR_HIDDEN))
				{
					poly[3].X = float(mx << 8);
					poly[3].Y = float(me[0]->Alt << ALT_SHIFT);
					poly[3].Z = float(mz << 8);

					poly[0].X = poly[3].X + 256.0F;
					poly[0].Y = float(me[1]->Alt << ALT_SHIFT);
					poly[0].Z = poly[3].Z;

					poly[1].X = poly[3].X + 256.0F;
					poly[1].Y = float(me[2]->Alt << ALT_SHIFT);
					poly[1].Z = poly[3].Z + 256.0F;

					poly[2].X = poly[3].X;
					poly[2].Y = float(me[3]->Alt << ALT_SHIFT);
					poly[2].Z = poly[3].Z + 256.0F;

					if (me[0]->Flags & FLOOR_SINK_SQUARE)
					{
						poly[0].Y -= 32.0F;
						poly[1].Y -= 32.0F;
						poly[2].Y -= 32.0F;
						poly[3].Y -= 32.0F;
					}

					if (me[0]->Alt == me[1]->Alt &&
						me[0]->Alt == me[2]->Alt &&
						me[0]->Alt == me[3]->Alt)
					{
						//
						// This quad is coplanar.
						//

						projected = SMAP_project_onto_poly(poly, 4);

						if (projected)
						{
							AENG_add_projected_shadow_poly(projected);
						}
					}
					else
					{
						//
						// Do each triangle separatly.
						//

						projected = SMAP_project_onto_poly(poly, 3);

						if (projected)
						{
							AENG_add_projected_shadow_poly(projected);
						}

						//
						// The triangles are 0,1,2 and 0,2,3.
						//

						poly[1] = poly[0];

						projected = SMAP_project_onto_poly(poly + 1, 3);

						if (projected)
						{
							AENG_add_projected_shadow_poly(projected);
						}
					}
				}

				//
				// Project onto nearby colvects...
				//

				v_list = me[0]->ColVectHead;

				while(v_list)
				{
					i_vect =  col_vects_links[v_list].VectIndex;
					p_vect = &col_vects[i_vect];

					if (p_vect->PrimType == STOREY_TYPE_NORMAL ||
						p_vect->PrimType == STOREY_TYPE_FENCE)
					{
						for (i = 0; i < done_upto; i++)
						{
							if (done[i] == i_vect)
							{
								//
								// Dont do this colvect again
								//

								goto ignore_this_colvect;
							}
						}

						poly[0].X = float(p_vect->X[1]);
						poly[0].Y = float(p_vect->Y[1]);
						poly[0].Z = float(p_vect->Z[1]);

						poly[1].X = float(p_vect->X[1]);
						poly[1].Y = float(p_vect->Y[1] + BLOCK_SIZE * 4 * p_vect->PrimExtra);
						poly[1].Z = float(p_vect->Z[1]);

						poly[2].X = float(p_vect->X[0]);
						poly[2].Y = float(p_vect->Y[0] + BLOCK_SIZE * 4 * p_vect->PrimExtra);
						poly[2].Z = float(p_vect->Z[0]);

						poly[3].X = float(p_vect->X[0]);
						poly[3].Y = float(p_vect->Y[0]);
						poly[3].Z = float(p_vect->Z[0]);

						projected = SMAP_project_onto_poly(poly, 4);

						if (projected)
						{
							AENG_add_projected_shadow_poly(projected);
						}

						//
						// We've already done this colvect.
						//

						if (done_upto < AENG_MAX_DONE)
						{
							done[done_upto++] = i_vect;
						}
					}

				  ignore_this_colvect:;

					v_list = col_vects_links[v_list].Next;
				}

				if (darci->OnFace)
				{
					//
					// Cast shadow on walkable faces.
					//

					w_list = me[0]->Walkable;

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

								face_height  = p_fthing->WorldPos.Y >> 8;
								face_height += prim_points[p_f4->Points[0]].Y;

								if (face_height > ((darci->WorldPos.Y + 0x8000) >> 8))
								{
									//
									// This face is above Darci, so don't put her shadow
									// on it.
									//
								}
								else
								{
									poly[0].X  = float(p_fthing->WorldPos.X >> 8);
									poly[0].Y  = float(p_fthing->WorldPos.Y >> 8);
									poly[0].Z  = float(p_fthing->WorldPos.Z >> 8);

									poly[1]    = poly[0];
									poly[2]    = poly[0];
									poly[3]    = poly[0];

									for (i = 0; i < 4; i++)
									{
										pp = &prim_points[p_f4->Points[face_order[i]]];

										poly[i].X += float(pp->X);
										poly[i].Y += float(pp->Y);
										poly[i].Z += float(pp->Z);
									}

									projected = SMAP_project_onto_poly(poly, 4);

									if (projected)
									{
										AENG_add_projected_shadow_poly(projected);
									}
								}
							}
						}

						w_list = walk_links[w_list].Next;
					}
				}
			}
		}

		TEXTURE_shadow_update();
	}

	#endif

	#ifdef DONT_IGNORE_STARS

	//
	// Draw the stars...
	//

	if (the_display.screen_lock())
	{
		SKY_draw_stars(
			AENG_cam_x,
			AENG_cam_y,
			AENG_cam_z,
			AENG_DRAW_DIST * 256.0F);

		the_display.screen_unlock();
	}

	#endif

	#ifdef DONT_IGNORE_REFLECTIONS

	//
	// Draw the reflections of people.
	//

	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{
		for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
		{
			me = &MAP[MAP_INDEX(x, z)];

			if (me->Flags & FLOOR_REFLECTIVE)
			{
				t_index = me->MapWho;

				while(t_index)
				{
					p_thing = TO_THING(t_index);

					if (p_thing->DrawType == DT_ROT_MULTI)
					{
						if (POLY_sphere_visible(
								float(p_thing->WorldPos.X >> 8),
								float(p_thing->WorldPos.Y >> 8) + 64.0F,
								float(p_thing->WorldPos.Z >> 8),
								128.0F / (AENG_DRAW_DIST * 256.0F)))
						{
							FIGURE_draw_reflection(p_thing, p_thing->WorldPos.Y >> 8);
						}
					}

					t_index = p_thing->Child;
				}
			}
		}
	}

	#endif

	#ifdef DONT_IGNORE_PUDDLES

	//
	// Draw the puddles.
	//

	{
		SLONG i;

		UWORD puddle_x;
		SWORD puddle_y;
		UWORD puddle_z;
		UWORD puddle_radius;
		UWORD puddle_angle;

		float mid_x;
		float mid_y;
		float mid_z;
		float radius;
		float angle;

		float world_x;
		float world_y;
		float world_z;

		static struct
		{
			float u;
			float v;

		} texture_coords[4] =
		{
			{0.0F,0.0F},
			{1.0F,0.0F},
			{1.0F,1.0F},
			{0.0F,1.0F}
		};

		POLY_Point  pp  [4];
		POLY_Point *quad[4];

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[3];
		quad[3] = &pp[2];

		for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
		{
			PUDDLE_get_start(z, NGAMUT_gamut[z].xmin, NGAMUT_gamut[z].xmax);

			while(PUDDLE_get(
					&puddle_x,
					&puddle_y,
					&puddle_z,
					&puddle_radius,
					&puddle_angle))
			{
				mid_x  = float(puddle_x);
				mid_y  = float(puddle_y);
				mid_z  = float(puddle_z);
				radius = float(puddle_radius);
				angle  = float(puddle_angle) * (2.0F * PI / 2048.0F);

				for (i = 0; i < 4; i++)
				{
					world_x = mid_x + sin(angle) * radius;
					world_y = mid_y;
					world_z = mid_z + cos(angle) * radius;

					angle += PI / 2.0F;

					POLY_transform(
						 world_x,
						 world_y,
						 world_z,
						&pp[i]);

					if (pp[i].clip & POLY_CLIP_TRANSFORMED)
					{
						pp[i].u        = texture_coords[i].u;
						pp[i].v        = texture_coords[i].v;
						pp[i].colour   = 0xffffffff;
						pp[i].specular = 0x00000000;
					}
					else
					{
						goto ignore_this_puddle;
					}
				}

				if (POLY_valid_quad(quad))
				{
					POLY_add_quad(quad, POLY_PAGE_PUDDLE, FALSE);
				}

			  ignore_this_puddle:;
			}
		}	
	}

	//
	// Draw the reflections and the puddles and clear the poly lists.
	//

	POLY_frame_draw(FALSE);
	POLY_frame_init(TRUE);

	#endif

	//
	// Create all the squares.
	//

	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{
		for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
		{
			me = &MAP[MAP_INDEX(x, z)];

			if (me->Flags & FLOOR_HIDDEN)
			{
				continue;
			}

			//
			// The four points of the quad.
			//

			if (me->Flags & FLOOR_SINK_SQUARE)
			{
				quad[0] = &AENG_lower[x + 0][z + 0];
				quad[1] = &AENG_lower[x + 1][z + 0];
				quad[2] = &AENG_lower[x + 0][z + 1];
				quad[3] = &AENG_lower[x + 1][z + 1];
			}
			else
			{
				quad[0] = &AENG_upper[x + 0][z + 0];
				quad[1] = &AENG_upper[x + 1][z + 0];
				quad[2] = &AENG_upper[x + 0][z + 1];
				quad[3] = &AENG_upper[x + 1][z + 1];
			}
			
			if (POLY_valid_quad(quad))
			{
				// 
				// Texture the quad.
				//

				TEXTURE_get_minitexturebits_uvs(
						me->Texture,
					   &page,
					   &quad[0]->u,
					   &quad[0]->v,
					   &quad[1]->u,
					   &quad[1]->v,
					   &quad[2]->u,
					   &quad[2]->v,
					   &quad[3]->u,
					   &quad[3]->v);
				
				if (me->Flags & (FLOOR_SHADOW_1 | FLOOR_SHADOW_2))
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

					if ((me->Flags & FLOOR_SHADOW_1) &&
						(me->Flags & FLOOR_SHADOW_2))
					{
						quad[0] = &ps[0];
						quad[1] = &ps[1];
						quad[2] = &ps[2];
						quad[3] = &ps[3];

						POLY_add_quad(quad, page, TRUE);					
					}
					else
					{
						if (me->Flags & FLOOR_SHADOW_2)
						{
							tri[0] = quad[0];
							tri[1] = quad[1];
							tri[2] = quad[2];

							POLY_add_triangle(tri, page, TRUE);

							tri[0] = &ps[1];
							tri[1] = &ps[3];
							tri[2] = &ps[2];

							POLY_add_triangle(tri, page, TRUE);
						}
						else
						{
							tri[0] = quad[1];
							tri[1] = quad[3];
							tri[2] = quad[2];

							POLY_add_triangle(tri, page, TRUE);

							tri[0] = &ps[0];
							tri[1] = &ps[1];
							tri[2] = &ps[2];

							POLY_add_triangle(tri, page, TRUE);
						}
					}
				}
				else
				{
					POLY_add_quad(quad, page, TRUE);
				}
			}

			if (me->Flags & FLOOR_SINK_SQUARE)
			{
				//
				// Do the curbs now.
				//

				struct
				{
					SLONG dpx1;
					SLONG dpz1;
					SLONG dpx2;
					SLONG dpz2;

					SLONG dsx;
					SLONG dsz;

				} curb[4] = 
				{
					{0,0,0,1,-1,0},
					{0,1,1,1,0,+1},
					{1,1,1,0,+1,0},
					{1,0,0,0,0,-1}
				};

				for (i = 0; i < 4; i++)
				{
					nx = x + curb[i].dsx;
					nz = z + curb[i].dsz;

					if (WITHIN(nx, 0, MAP_WIDTH  - 1) &&
						WITHIN(nz, 0, MAP_HEIGHT - 1))
					{
						if (MAP[MAP_INDEX(nx, nz)].Flags & FLOOR_SINK_SQUARE)
						{
							//
							// No need for a curb here.
							//
						}
						else
						{
							quad[0] = &AENG_lower[x + curb[i].dpx1][z + curb[i].dpz1];
							quad[1] = &AENG_lower[x + curb[i].dpx2][z + curb[i].dpz2];
							quad[2] = &AENG_upper[x + curb[i].dpx1][z + curb[i].dpz1];
							quad[3] = &AENG_upper[x + curb[i].dpx2][z + curb[i].dpz2];

							if (POLY_valid_quad(quad))
							{
								TEXTURE_get_minitexturebits_uvs(
									0,
								   &page,
								   &quad[0]->u,
								   &quad[0]->v,
								   &quad[1]->u,
								   &quad[1]->v,
								   &quad[2]->u,
								   &quad[2]->v,
								   &quad[3]->u,
								   &quad[3]->v);

								//
								// Add the poly.
								//

								POLY_add_quad(quad, page, TRUE);
							}
						}
					}
				}
			}
		}
	}

	for (z = NGAMUT_zmin; z <= NGAMUT_zmax; z++)
	{
		for (x = NGAMUT_gamut[z].xmin; x <= NGAMUT_gamut[z].xmax; x++)
		{
			me = &MAP[MAP_INDEX(x, z)];

			t_index = me->MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				if (p_thing->Flags & FLAGS_IN_BUILDING)
				{
					//
					// Dont draw things inside buildings when we are outdoors.
					//
				}
				else
				{
					switch(p_thing->DrawType)
					{
						case DT_NONE:
							break;

						case DT_BUILDING:

							//
							// Draw cables from their mapwho link. All other buildings are
							// drawn from their colvects.
							//

							if (p_thing->Flags & FLAGS_CABLE_BUILDING)
							{
								BUILD_draw(p_thing);
							}

							break;

						case DT_PRIM:
							break;

						case DT_ROT_MULTI:
							
							p_thing->DrawType = DT_ROT_MULTI;


							if (POLY_sphere_visible(
									float(p_thing->WorldPos.X >> 8),
									float(p_thing->WorldPos.Y >> 8) + 64.0F,
									float(p_thing->WorldPos.Z >> 8),
									128.0F / (AENG_DRAW_DIST * 256.0F)))
							{
								FIGURE_draw(p_thing);
							}

							break;

						case DT_EFFECT:
							break;

						case DT_MESH:
							
							//						
							// Light the mesh.
							//

							LIGHT_prim_use_normals(t_index);

							MESH_draw_poly(
								p_thing->Draw.Mesh->ObjectId,
								p_thing->WorldPos.X >> 8,
								p_thing->WorldPos.Y >> 8,
								p_thing->WorldPos.Z >> 8,
								p_thing->Draw.Mesh->Angle,
								p_thing->Draw.Mesh->Tilt,
								p_thing->Draw.Mesh->Roll,
								LIGHT_point_colour);

							break;

						default:
							ASSERT(0);
							break;
					}
				}

				t_index = p_thing->Child;
			}

			//
			// Look at the first colvect on this square and draw the building
			// it is attached to.
			//

			{
				SLONG building;
				SLONG storey;
				SLONG wall;

				SLONG v_list;
				SLONG i_vect;

				CollisionVect *p_vect;

				v_list = me->ColVectHead;

				if (v_list)
				{
					i_vect =  col_vects_links[v_list].VectIndex;
					p_vect = &col_vects[i_vect];

					wall     = -p_vect->Face;
					storey   =  wall_list[wall].StoreyHead;
					building =  storey_list[storey].BuildingHead;

					if (building_list[building].BuildingType == BUILDING_TYPE_CRATE_IN)
					{
						//
						// Dont draw crates inside buildings.
						// 
					}
					else
					{
						if (building_list[building].LastDrawn != UWORD(GAME_TURN))
						{
							//
							// We haven't already drawn this building.
							//

							t_index = building_list[building].ThingIndex;
							p_thing = TO_THING(t_index);

							BUILD_draw(p_thing);

							//
							// Mark this building as drawn.
							//

							building_list[building].LastDrawn = UWORD(GAME_TURN);
						}
					}

					#if WE_WANT_TO_TEST_THE_WORLD_LINE_DRAW_BY_DRAWING_THE_COLVECTS

					//
					// Draw the colvect.
					//

					{
						void AENG_world_line(
								SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
								SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
								SLONG sort_to_front);

						AENG_world_line(
							p_vect->X[0], p_vect->Y[0], p_vect->Z[0], 32, 0x00ff0000,
							p_vect->X[1], p_vect->Y[1], p_vect->Z[1], 0,  0x000000ff,
							TRUE);
					}

					#endif
				}
			}
		}
	}

	#ifdef DONT_IGNORE_FANCY_STUFF

	//
	// The dirt.
	//

	float leaf_centre_u;
	float leaf_centre_v;

	TEXTURE_get_fiddled_position(6, 0, 0,
		&leaf_centre_u,
		&leaf_centre_v);

	leaf_centre_u += 16.0F / 256.0F;
	leaf_centre_v += 16.0F / 256.0F;

	#define LEAF_PAGE		(POLY_PAGE_MASKED)
	#define LEAF_CENTRE_U	(leaf_centre_u)
	#define LEAF_CENTRE_V	(leaf_centre_v)
	#define LEAF_RADIUS		(16.00F / 256.0F)
	#define LEAF_U(a)		(LEAF_CENTRE_U + LEAF_RADIUS * sin(a))
	#define LEAF_V(a)		(LEAF_CENTRE_V + LEAF_RADIUS * cos(a))
	#define LEAF_UP			8
	#define LEAF_SIZE       (20.0F+(float)(i&15))

	{
		SLONG j,falling;

		DIRT_Info di;

		float fyaw;
		float fpitch;
		float froll;

		float       matrix[9];
		float       angle;
		SVECTOR_F   temp[3];
		POLY_Point  pp[3];
		POLY_Point *tri[3];

		ULONG leaf_colour_choice[4] =
		{
			0x332d1d,
			0x243224,
			0x123320,
			0x332f07
		};

		ULONG flag[4];
		ULONG leaf_colour;
		ULONG leaf_specular;

		tri[0] = &pp[0];
		tri[1] = &pp[1];
		tri[2] = &pp[2];

		//
		// Draw the dirt.
		//

		for (i = 0; i < DIRT_MAX_DIRT; i++)
		{
			falling = FALSE;

			di = DIRT_get_info(i);

			if ((di.yaw | di.pitch | di.roll) == 0)
			{
				//
				// This happens often... so we optimise it out.
				//

				temp[0].X = float(di.x);
				temp[0].Y = float(di.y + LEAF_UP);
				temp[0].Z = float(di.z + LEAF_SIZE);

				temp[1].X = float(di.x + LEAF_SIZE);
				temp[1].Y = float(di.y + LEAF_UP);
				temp[1].Z = float(di.z - LEAF_SIZE);

				temp[2].X = float(di.x - LEAF_SIZE);
				temp[2].Y = float(di.y + LEAF_UP);
				temp[2].Z = float(di.z - LEAF_SIZE);
			}
			else
			{
				//
				// The rotation matrix of this bit of dirt.
				//

				fyaw   = float(di.yaw)   * (PI / 1024.0F);
				fpitch = float(di.pitch) * (PI / 1024.0F);
				froll  = float(di.roll)  * (PI / 1024.0F);

				MATRIX_calc(matrix, fyaw, fpitch, froll);

				//
				// Work out the position of the points.																													 				//

				for (j = 0; j < 3; j++)
				{

					temp[j].X  = float(di.x);
					temp[j].Y  = float(di.y);
					temp[j].Z  = float(di.z);

					temp[j].Y += float(LEAF_UP);
				}


				temp[0].X += matrix[6] * LEAF_SIZE;
				temp[0].Y += matrix[7] * LEAF_SIZE;
				temp[0].Z += matrix[8] * LEAF_SIZE;

				temp[1].X -= matrix[6] * LEAF_SIZE;
				temp[1].Y -= matrix[7] * LEAF_SIZE;
				temp[1].Z -= matrix[8] * LEAF_SIZE;

				temp[2].X -= matrix[6] * LEAF_SIZE;
				temp[2].Y -= matrix[7] * LEAF_SIZE;
				temp[2].Z -= matrix[8] * LEAF_SIZE;

				temp[1].X += matrix[0] * LEAF_SIZE;
				temp[1].Y += matrix[1] * LEAF_SIZE;
				temp[1].Z += matrix[2] * LEAF_SIZE;

				temp[2].X -= matrix[0] * LEAF_SIZE;
				temp[2].Y -= matrix[1] * LEAF_SIZE;
				temp[2].Z -= matrix[2] * LEAF_SIZE;

				falling = TRUE;
			}

			//
			// Transform the points.
			//

			for (j = 0; j < 3; j++)
			{
				POLY_transform(
					temp[j].X,
					temp[j].Y,
					temp[j].Z,
				   &pp[j]);

				if (!(pp[j].clip & POLY_CLIP_TRANSFORMED))
				{
					//
					// Don't bother transforming the other points.
					//

					goto do_next_dirt;
				}
			}

			if (POLY_valid_triangle(tri))
			{
				//
				// The colour and texture of the leaf.
				//

				leaf_colour = leaf_colour_choice[i & 0x3];

				angle = float(i);

				for (j = 0; j < 3; j++)
				{
					pp[j].colour   = leaf_colour * (j + 3);
					pp[j].specular = 0;
					pp[j].u        = LEAF_U(angle);
					pp[j].v        = LEAF_V(angle);

					angle += 2.0F * PI / 3.0F;
				}

				POLY_add_triangle(tri, POLY_PAGE_MASKED, FALSE);
			}

		  do_next_dirt:;

		}
	}

	//
	// The sky.
	// 

	SKY_draw_poly(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_DRAW_DIST * 256.0F);

	#endif

	//
	// Draw the mist.
	//

	{
		SLONG i;

		SLONG sx;
		SLONG sz;

		SLONG px;
		SLONG pz;
		SLONG detail;

		SLONG wx;
		SLONG wy;
		SLONG wz;

		POLY_Point *pp;
		POLY_Point *quad[4];

		#define MAX_MIST_DETAIL 32

		POLY_Point mist_pp[MAX_MIST_DETAIL][MAX_MIST_DETAIL];

		MIST_get_start();

		while(detail = MIST_get_detail())
		{
			//
			// Only draw as much as we can!
			//

			if (detail > MAX_MIST_DETAIL)
			{
				detail = MAX_MIST_DETAIL;
			}

			for (px = 0; px < detail; px++)
			for (pz = 0; pz < detail; pz++)
			{
				pp = &mist_pp[px][pz];

				MIST_get_point(px, pz,
				   &wx,
				   &wy,
				   &wz);

				POLY_transform(
					float(wx),
					float(wy),
					float(wz),
				    pp);

				if (pp->clip & POLY_CLIP_TRANSFORMED)
				{
					pp->colour = 0x00ffffff;

					MIST_get_texture(
						px,
						pz,
					   &pp->u,
					   &pp->v);

					pp->u *= 256.0F;
					pp->v *= 256.0F;
				}
			}

			for (sx = 0; sx < detail - 1; sx++)
			for (sz = 0; sz < detail - 1; sz++)
			{
				quad[0] = &mist_pp[sx + 0][sz + 0];
				quad[1] = &mist_pp[sx + 1][sz + 0];
				quad[2] = &mist_pp[sx + 0][sz + 1];
				quad[3] = &mist_pp[sx + 1][sz + 1];

				if (POLY_valid_quad(quad))
				{
					POLY_add_quad(quad, TEXTURE_page_fog, FALSE);
				}
			}
		}
	}
}

void AENG_draw_inside(void) {}

void AENG_draw_sewer(void)
{
}

//
// The scanner.
// 

void AENG_draw_scanner(
		SLONG screen_x1,
		SLONG screen_y1,
		SLONG screen_x2,
		SLONG screen_y2,
		SLONG map_x,
		SLONG map_z,
		SLONG map_zoom,		// The number of pixels per mapsquare in fixed-point 8.
		SLONG map_angle)
{
}

//
// Draws the messages and the FPS stuff to the screen.
//

void ANEG_draw_messages(void)
{
}

//
// Flips/blits the back-buffer.
//

void AENG_flip(void) {grBufferSwap(1);}
void AENG_blit(void) {grBufferSwap(0);grDepthMask(FXTRUE);grBufferClear(0x00000000, 0, (FxU16) AENG_wrange[1]);}
	


//
// Adds a message to the message system.
//

void MSG_add(CBYTE *message, ...)
{
}

//
// Drawing stuff straight to the screen...
//

void  AENG_clear_screen(void) {}
SLONG AENG_lock(void)         {return 0;}
SLONG FONT_draw(SLONG x, SLONG y, CBYTE *text, ...) {return 0;}
void  AENG_unlock(void) {}


//
// Call once at the end of the whole program.
// 

void AENG_fini(void)
{
}


