//
// Draw a person.
// 

#include "game.h"
#include "aeng.h"
#include "poly.h"
#include "light.h"
#include "figure.h"
#include "c:\fallen\headers\fmatrix.h"
#include "c:\fallen\headers\interact.h"

void FIGURE_rotate_obj(SLONG xangle,SLONG yangle,SLONG zangle, Matrix33 *r3) 
{
	SLONG	sinx, cosx, siny, cosy, sinz, cosz;
 	SLONG	cxcz,sysz,sxsycz,sxsysz,sysx,cxczsy,sxsz,cxsysz,czsx,cxsy,sycz,cxsz;

	sinx = SIN(xangle & (2048-1)) >>1;  	// 15's
	cosx = COS(xangle & (2048-1)) >>1;
	siny = SIN(yangle & (2048-1)) >>1;
	cosy = COS(yangle & (2048-1)) >>1;
	sinz = SIN(zangle & (2048-1)) >>1;
	cosz = COS(zangle & (2048-1)) >>1;

	cxsy    = (cosx*cosy);				  		// 30's
	sycz    = (cosy*cosz);
	cxcz	= (cosx*cosz);
	cxsz	= (cosx*sinz);
	czsx	= (cosz*sinx);
	sysx    = (cosy*sinx);
	sysz	= (cosy*sinz);
	sxsz 	= (sinx*sinz);
	sxsysz  = (sxsz>>15)*siny;
	cxczsy	= (cxcz>>15)*siny;
	cxsysz  = ((cosx*siny)>>15)*sinz;
	sxsycz  = (czsx>>15)*siny;

	// Define rotation matrix r3.

	r3->M[0][0] = (sycz)>>15;						// 14's
	r3->M[0][1] = (-sysz)>>15;
	r3->M[0][2] = siny;
	r3->M[1][0] = (sxsycz+cxsz)>>15;
	r3->M[1][1] = (cxcz-sxsysz)>>15;
	r3->M[1][2] = (-sysx)>>15;
	r3->M[2][0] = (sxsz-cxczsy)>>15;
	r3->M[2][1] = (cxsysz+czsx)>>15;
	r3->M[2][2] = (cxsy)>>15;
}


void FIGURE_draw_prim_tween(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG tween,
		struct GameKeyFrameElement *anim_info,
		struct GameKeyFrameElement *anim_info_next,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		SLONG backwards)	// TRUE => The faces are drawn in the wrong order.
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
	ULONG colour;
	ULONG red;
	ULONG green;
	ULONG blue;
	ULONG r;
	ULONG g;
	ULONG b;
	ULONG face_colour;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;

	SVECTOR temp;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	colour = LIGHT_get_glide_colour(LIGHT_get_point(x, y, z));

	red   = (colour >> 16) & 0xff;
	green = (colour >>  8) & 0xff;
	blue  = (colour >>  0) & 0xff;

	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	offset.M[0] = anim_info->OffsetX + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8);
	offset.M[1] = anim_info->OffsetY + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8);
	offset.M[2] = anim_info->OffsetZ + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8);

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);

	x += temp.X;
	y += temp.Y;
	z += temp.Z;	

	//
	// Create a temporary "tween" matrix between current and next
	//

	build_tween_matrix(&mat2,&anim_info->CMatrix,&anim_info_next->CMatrix,tween);

	normalise_matrix(&mat2);
	
	//
	// Apply local rotation matrix to get mat_final that rotates
	// the point into world space.
	//

	matrix_mult33(&mat_final, rot_mat, &mat2);

	//
	// Do everything in floats.
	//

	float off_x;
	float off_y;
	float off_z;
	float fmatrix[9];

	fmatrix[0] = float(mat_final.M[0][0]) * (1.0F / 32768.0F);
	fmatrix[1] = float(mat_final.M[0][1]) * (1.0F / 32768.0F);
	fmatrix[2] = float(mat_final.M[0][2]) * (1.0F / 32768.0F);
	fmatrix[3] = float(mat_final.M[1][0]) * (1.0F / 32768.0F);
	fmatrix[4] = float(mat_final.M[1][1]) * (1.0F / 32768.0F);
	fmatrix[5] = float(mat_final.M[1][2]) * (1.0F / 32768.0F);
	fmatrix[6] = float(mat_final.M[2][0]) * (1.0F / 32768.0F);
	fmatrix[7] = float(mat_final.M[2][1]) * (1.0F / 32768.0F);
	fmatrix[8] = float(mat_final.M[2][2]) * (1.0F / 32768.0F);

	off_x = float(x);
	off_y = float(y);
	off_z = float(z);

	POLY_set_local_rotation(
		off_x,
		off_y,
		off_z,
		fmatrix);

	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;

	for (i = sp; i < ep; i++)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform_using_local_rotation(
			AENG_dx_prim_points[i].X,
			AENG_dx_prim_points[i].Y,
			AENG_dx_prim_points[i].Z,
			pp);

		pp->colour = colour;
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

		if (backwards)
		{
			quad[0] = &POLY_buffer[p0];
			quad[2] = &POLY_buffer[p1];
			quad[1] = &POLY_buffer[p2];
			quad[3] = &POLY_buffer[p3];
		}
		else
		{
			quad[0] = &POLY_buffer[p0];
			quad[1] = &POLY_buffer[p1];
			quad[2] = &POLY_buffer[p2];
			quad[3] = &POLY_buffer[p3];
		}

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

				page   = p_f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f4->TexturePage;

				POLY_add_quad(quad, page, TRUE);
			}
			else
			{
				/*

				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f4->Col].red;
				g = ENGINE_palette[p_f4->Col].green;
				b = ENGINE_palette[p_f4->Col].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

				quad[0]->colour = face_colour;
				quad[1]->colour = face_colour;
				quad[2]->colour = face_colour;
				quad[3]->colour = face_colour;

				POLY_add_quad(quad, POLY_PAGE_COLOUR, TRUE);

				*/
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

		if (backwards)
		{
			tri[0] = &POLY_buffer[p0];
			tri[2] = &POLY_buffer[p1];
			tri[1] = &POLY_buffer[p2];
		}
		else
		{
			tri[0] = &POLY_buffer[p0];
			tri[1] = &POLY_buffer[p1];
			tri[2] = &POLY_buffer[p2];
		}

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

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;

				POLY_add_triangle(tri, page, TRUE);
			}
			else
			{
				/*

				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f3->Col].red;
				g = ENGINE_palette[p_f3->Col].green;
				b = ENGINE_palette[p_f3->Col].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

				tri[0]->colour = face_colour;
				tri[1]->colour = face_colour;
				tri[2]->colour = face_colour;

				POLY_add_triangle(tri, POLY_PAGE_COLOUR, TRUE);

				*/
			}
		}
	}
}




void FIGURE_draw(Thing *p_thing)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;

	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		return;
	}

	//
	// Find the lighting context for this thing.  The FIGURE_draw_prim_tween() function
	// calls LIGHT_get_point(). LIGHT_get_point returns the light on the given
	// point in this context.
	//

	LIGHT_get_context(THING_NUMBER(p_thing));

	//
	// The offset to keep the locked limb in the same place.
	//

	if (dt->Locked)
	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		//
		// Taken from temp.cpp
		//


		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   0, dt->Locked, &x1, &y1, &z1);
		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, 256, dt->Locked, &x2, &y2, &z2);

		dx = x1 - x2;
		dy = y1 - y2;
		dz = z1 - z2;
	}
	else
	{
		dx = 0;
		dy = 0;
		dz = 0;
	}

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		dt->Roll,
	   &r_matrix);

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];

		FIGURE_draw_prim_tween(
			start_object + object_offset,
			p_thing->WorldPos.X >> 8,
			(p_thing->WorldPos.Y >> 8)+5,
			p_thing->WorldPos.Z >> 8,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			FALSE);
	}
}




void FIGURE_draw_reflection(Thing *p_thing, SLONG height)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;

	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR FIGURE_draw_reflection");
		return;
	}

	//
	// Find the lighting context for this thing.  The FIGURE_draw_prim_tween() function
	// calls LIGHT_get_point(). LIGHT_get_point returns the light on the given
	// point in this context.
	//

	LIGHT_get_context(THING_NUMBER(p_thing));

	//
	// The offset to keep the locked limb in the same place.
	//

	if (dt->Locked)
	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		//
		// Taken from temp.cpp
		//


		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   0, dt->Locked, &x1, &y1, &z1);
		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, 256, dt->Locked, &x2, &y2, &z2);

		dx = x1 - x2;
		dy = y1 - y2;
		dz = z1 - z2;
	}
	else
	{
		dx = 0;
		dy = 0;
		dz = 0;
	}

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		dt->Roll,
	   &r_matrix);

	SLONG posx = p_thing->WorldPos.X >> 8;
	SLONG posy = p_thing->WorldPos.Y >> 8;
	SLONG posz = p_thing->WorldPos.Z >> 8;

	//
	// Reflect about y = height.
	//

	posy =  height - (posy - height);
	dy   = -dy;

	r_matrix.M[0][1] = -r_matrix.M[0][1];
	r_matrix.M[1][1] = -r_matrix.M[1][1];
	r_matrix.M[2][1] = -r_matrix.M[2][1];

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];

		FIGURE_draw_prim_tween(
			start_object + object_offset,
			posx,
			posy,
			posz,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			TRUE);
	}
}



























