//
// Shadow maps!
//

#include "game.h"
#include "smap.h"
#include "aa.h"
#include "person.h"
#include "fmatrix.h"
#include "matrix.h"
#include "memory.h"
#include <math.h>

// ========================================================
//
// THE SHADOW MAPPER PROTOTYPE
//
// ========================================================


//
// The shadow-mapper.
//
// First initialise the shadow mapper by giving it the direction of
// the parallel light you want to shadow and the bitmap the shadow
// map is going to be drawn into.
//

void SMAP_init(
		float  light_dx,	// The light vector doesn't have to be normalised
		float  light_dy,
		float  light_dz,
		UBYTE *bitmap,
		UBYTE  res_u,
		UBYTE  res_v);

//
// Add world space points. Each point is given an index that is used
// to identify it when you add triangles.  When there are no more points
// left, call SMAP_point_finished().  The first point you add has index
// 0, then they go up by one each time.
//

SLONG SMAP_point_add(
		float world_x,
		float world_y,
		float world_z);

void SMAP_point_finished(void);

//
// Add each triangle. Each triangle is rendered into the current shadow map.
//

void SMAP_tri_add(
		SLONG p1,
		SLONG p2,
		SLONG p3);

//
// After you have called SMAP_point_finished(), you can get the shadow maps
// position in world space.
//

void SMAP_get_world_pos(

		//
		// Position of (0,0) of the shadow map.
		// 

		float *world_x,
		float *world_y,
		float *world_z,

		//
		// The vector along on pixel in the u and v direction.
		//
		
		float *world_dxdu,
		float *world_dydu,
		float *world_dzdu,

		float *world_dxdv,
		float *world_dydv,
		float *world_dzdv);


// ========================================================
//
// THE SHADOW MAPPER CODE
//
// ========================================================

//
// While adding points, we project them on the plane whose normal is
// the light vector and which goes through the origin of the world.  It
// point is given in terms of two vectors, plane_u and plane_v.
//

//
// The plane, passing through the origin of the world.
// 

float SMAP_plane_ux;
float SMAP_plane_uy;
float SMAP_plane_uz;

float SMAP_plane_vx;
float SMAP_plane_vy;
float SMAP_plane_vz;

float SMAP_plane_nx;
float SMAP_plane_ny;
float SMAP_plane_nz;

//
// The bounding values in all the points.
// 


float SMAP_u_min;
float SMAP_u_max;

float SMAP_v_min;
float SMAP_v_max;

float SMAP_n_min;
float SMAP_n_max;

float SMAP_u_map_mul_float;
float SMAP_v_map_mul_float;

float SMAP_u_map_add_float;
float SMAP_v_map_add_float;

float SMAP_u_map_mul_slong;
float SMAP_v_map_mul_slong;

//
// The bitmap.
// 

UBYTE *SMAP_bitmap;
SLONG  SMAP_res_u;
SLONG  SMAP_res_v;


//
// The points.
// 

typedef struct
{
	float along_u;
	float along_v;
	float along_n;

	float world_x;
	float world_y;
	float world_z;

	SLONG u;	// Coordinates on the bitmap in fixed-point 16.
	SLONG v;

} SMAP_Point;

#define SMAP_MAX_POINTS 2048

SMAP_Point SMAP_point[SMAP_MAX_POINTS];
SLONG      SMAP_point_upto;


//
// Normlises the given vector.
//

void inline SMAP_vector_normalise(
					float *x,
					float *y,
					float *z)	// First time I have ever used references.
								// You haven't used references, you've used pointers.
{
	float len2 = *x**x + *y**y + *z**z;
	float len  = sqrt(len2);
	float lenr = 1.0F / len;

   *x *= lenr;
   *y *= lenr;
   *z *= lenr;
}



void SMAP_init(
		float  light_dx,
		float  light_dy,
		float  light_dz,
		UBYTE *bitmap,
		UBYTE  res_u,
		UBYTE  res_v)
{
	float len;
	float overlen;

	//
	// Store the bitmap.
	//

	SMAP_bitmap = bitmap;
	SMAP_res_u  = res_u;
	SMAP_res_v  = res_v;

	memset(bitmap, 0, res_u * res_v);

	//
	// Calculate the plane definition.
	//

	SMAP_plane_nx = -light_dx;	// I like to think of the plane as 'facing' the light!
	SMAP_plane_ny = -light_dy;
	SMAP_plane_nz = -light_dz;

	SMAP_vector_normalise(
		&SMAP_plane_nx,
		&SMAP_plane_ny,
		&SMAP_plane_nz);

	//
	// Work out the u-vector from the light vector.
	//

	if (fabsf(light_dx) + fabsf(light_dz) < 0.001F)
	{
		//
		// An arbitrary direction...
		//

		SMAP_plane_ux = 1.0F;
		SMAP_plane_uy = 0.0F;
		SMAP_plane_uz = 0.0F;
	}
	else
	{
		SMAP_plane_ux = -light_dz;
		SMAP_plane_uy =  0;
		SMAP_plane_uz =  light_dx;

		SMAP_vector_normalise(
			&SMAP_plane_ux,
			&SMAP_plane_uy,
			&SMAP_plane_uz);
	}

	//
	// The third vector is just the cross-product.
	//

	SMAP_plane_vx = SMAP_plane_ny*SMAP_plane_uz - SMAP_plane_nz*SMAP_plane_uy;
	SMAP_plane_vy = SMAP_plane_nz*SMAP_plane_ux - SMAP_plane_nx*SMAP_plane_uz;
	SMAP_plane_vz = SMAP_plane_nx*SMAP_plane_uy - SMAP_plane_ny*SMAP_plane_ux;

	//
	// Clear all the points and initialise the bounding values.
	//

	#define FINFINITY ((float) INFINITY)

	SMAP_point_upto = 0;

	SMAP_u_min = +FINFINITY;
	SMAP_u_max = -FINFINITY;

	SMAP_v_min = +FINFINITY;
	SMAP_v_max = -FINFINITY;

	SMAP_n_min = +FINFINITY;
	SMAP_n_max = -FINFINITY;
}


SLONG SMAP_point_add(
		float world_x,
		float world_y,
		float world_z)
{
	SMAP_Point *sp;

	ASSERT(WITHIN(SMAP_point_upto, 0, SMAP_MAX_POINTS - 1));

	sp = &SMAP_point[SMAP_point_upto];

	sp->world_x = world_x;
	sp->world_y = world_y;
	sp->world_z = world_z;

	sp->along_n = world_x*SMAP_plane_nx + world_y*SMAP_plane_ny + world_z*SMAP_plane_nz;
	sp->along_u = world_x*SMAP_plane_ux + world_y*SMAP_plane_uy + world_z*SMAP_plane_uz;
	sp->along_v = world_x*SMAP_plane_vx + world_y*SMAP_plane_vy + world_z*SMAP_plane_vz;

	//
	// Update the bounding values.
	// 

	if (sp->along_n < SMAP_n_min) {SMAP_n_min = sp->along_n;}
	if (sp->along_n > SMAP_n_max) {SMAP_n_max = sp->along_n;}

	if (sp->along_u < SMAP_u_min) {SMAP_u_min = sp->along_u;}
	if (sp->along_u > SMAP_u_max) {SMAP_u_max = sp->along_u;}

	if (sp->along_v < SMAP_v_min) {SMAP_v_min = sp->along_v;}
	if (sp->along_v > SMAP_v_max) {SMAP_v_max = sp->along_v;}

	return SMAP_point_upto++;
}


void SMAP_point_finished()
{
	SLONG i;

	SMAP_Point *sp;

	float along_bu;
	float along_bv;

	float res_u = float(SMAP_res_u);
	float res_v = float(SMAP_res_v);

	//
	// Push the image a pixel in from the edge of the bitmap.
	//

	SMAP_u_map_mul_float = (res_u - 2.0F) / (SMAP_u_max - SMAP_u_min);
	SMAP_v_map_mul_float = (res_v - 2.0F) / (SMAP_v_max - SMAP_v_min);

	SMAP_u_map_mul_slong = SMAP_u_map_mul_float * 65536.0F;
	SMAP_v_map_mul_slong = SMAP_v_map_mul_float * 65536.0F;

	SMAP_u_map_add_float = 1.0F / res_u;
	SMAP_v_map_add_float = 1.0F / res_v;

	SMAP_u_map_mul_float *= SMAP_u_map_add_float;
	SMAP_v_map_mul_float *= SMAP_v_map_add_float;

	//
	// Calculate the positions of the points on the bitmap.
	//

	for (i = 0; i < SMAP_point_upto; i++)
	{
		sp = &SMAP_point[i];

		along_bu = 65536.0F + (sp->along_u - SMAP_u_min) * SMAP_u_map_mul_slong;
		along_bv = 65536.0F + (sp->along_v - SMAP_v_min) * SMAP_v_map_mul_slong;

		sp->u = SLONG(along_bu);
		sp->v = SLONG(along_bv);

		ASSERT(WITHIN(sp->u, 0, SMAP_res_u << 16));
		ASSERT(WITHIN(sp->v, 0, SMAP_res_v << 16));
	}
}

void SMAP_tri_add(
		SLONG p1,
		SLONG p2,
		SLONG p3)
{
	SLONG du1;
	SLONG dv1;

	SLONG du2;
	SLONG dv2;

	ASSERT(WITHIN(p1, 0, SMAP_point_upto - 1));
	ASSERT(WITHIN(p2, 0, SMAP_point_upto - 1));
	ASSERT(WITHIN(p3, 0, SMAP_point_upto - 1));

	//
	// Backface cull...
	//

	du1 = SMAP_point[p2].u - SMAP_point[p1].u;
	dv1 = SMAP_point[p2].v - SMAP_point[p1].v;

	du2 = SMAP_point[p3].u - SMAP_point[p1].u;
	dv2 = SMAP_point[p3].v - SMAP_point[p1].v;

	if (MUL64(du1,dv2) - MUL64(dv1,du2) <= 0)
	{
		//
		// Backface culled.
		//
	}
	else
	{
		AA_draw(
			SMAP_bitmap,
			SMAP_res_u,
			SMAP_res_v,
			SMAP_res_v,
			SMAP_point[p1].u,
			SMAP_point[p1].v,
			SMAP_point[p2].u,
			SMAP_point[p2].v,
			SMAP_point[p3].u,
			SMAP_point[p3].v);
	}
}




// ========================================================
//
// FEEDING PRIMS AND BIKES TO THE SHADOW MAPPER
//
// ========================================================

//
// Add the triangles and quads of the given prim to the shadow mapper given the
// index of its first point.
//

void SMAP_add_prim_triangles(
		SLONG prim,
		SLONG index)
{
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	p_obj = &prim_objects[prim];

	SLONG i;

	index -= p_obj->StartPoint;

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		SMAP_tri_add(
			p_f3->Points[0] + index,
			p_f3->Points[1] + index,
			p_f3->Points[2] + index);
	}

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		SMAP_tri_add(
			p_f4->Points[0] + index,
			p_f4->Points[1] + index,
			p_f4->Points[2] + index);

		SMAP_tri_add(
			p_f4->Points[1] + index,
			p_f4->Points[3] + index,
			p_f4->Points[2] + index);
	}
}

//
// Adds a prims points to the shadow mapper and returns the index of the points.
//

SLONG SMAP_prim_points(
		SLONG prim,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll)
{
	SLONG i;
	float px;
	float py;
	float pz;
	float ox = float(world_x);
	float oy = float(world_y);
	float oz = float(world_z);
	SLONG base = -1;
	SLONG index;
	float matrix[9];

	PrimObject *p_obj = &prim_objects[prim];

	//
	// Calculate the objects rotation matrix.
	// 

	MATRIX_calc(
		matrix,
		float(yaw)   * (2.0F * PI / 2048.0F),
		float(pitch) * (2.0F * PI / 2048.0F),
		float(roll)  * (2.0F * PI / 2048.0F));

	//
	// Add all the points to the shadow mapper.
	//

	for (i = p_obj->StartPoint; i < p_obj->EndPoint; i++)
	{
		px = AENG_dx_prim_points[i].X;
		py = AENG_dx_prim_points[i].Y;
		pz = AENG_dx_prim_points[i].Z;

		MATRIX_MUL_BY_TRANSPOSE(
			matrix,
			px,
			py,
			pz);

		px += world_x;
		py += world_y;
		pz += world_z;

		index = SMAP_point_add(
					px,
					py,
					pz);

		if (base == -1)
		{
			base = index;
		}
	}

	return base;
}

#ifdef BIKE

void SMAP_bike(
		Thing *p_bike, 
		UBYTE *bitmap,	// 0 => transparent 255 => opaque
		UBYTE  u_res,
		UBYTE  v_res,
		SLONG  light_dx, // This vector need not be normalised
		SLONG  light_dy,
		SLONG  light_dz)
{
	SLONG i_frame;
	SLONG i_steer;
	SLONG i_fwheel;
	SLONG i_bwheel;

	BIKE_Drawinfo bdi = BIKE_get_drawinfo(p_bike);

	//
	// Initialise the shadow mapper.
	//

	SMAP_init(
		float(light_dx),
		float(light_dy),
		float(light_dz),
		bitmap,
		u_res,
		v_res);

	//
	// Add the points for each bit of the bike.
	//

	i_frame = SMAP_prim_points(
				PRIM_OBJ_BIKE_FRAME,
				p_bike->WorldPos.X >> 8,
				p_bike->WorldPos.Y >> 8,
				p_bike->WorldPos.Z >> 8,
				bdi.yaw,
				bdi.pitch,
				bdi.roll);

	/*

	//
	// The front of the bike is where we physically model the wheel
	// it is not at the correct place for the pivot point of the front wheel
	// or the steering column.
	//

	i_steer = SMAP_prim_points(
				PRIM_OBJ_BIKE_STEER,
				bdi.steer_x,
				bdi.steer_y,
				bdi.steer_z,
				bdi.steer,
				bdi.pitch,
				bdi.roll);

	i_fwheel = SMAP_prim_points(
				PRIM_OBJ_BIKE_FWHEEL,
				bdi.front_x,
				bdi.front_y,
				bdi.front_z,
				bdi.steer,
				bdi.pitch,
				bdi.roll);

	i_bwheel = SMAP_prim_points(
				PRIM_OBJ_BIKE_BWHEEL,
				bdi.back_x,
				bdi.back_y,
				bdi.back_z,
				bdi.yaw,
				0,
				bdi.roll);

	*/

	//
	// Finished adding the points.
	//

	SMAP_point_finished();

	//
	// Add the triangles and quads for each bit of the bike.
	// 

	SMAP_add_prim_triangles(PRIM_OBJ_BIKE_FRAME,  i_frame);

	/*

	SMAP_add_prim_triangles(PRIM_OBJ_BIKE_STEER,  i_steer);
	SMAP_add_prim_triangles(PRIM_OBJ_BIKE_FWHEEL, i_fwheel);
	SMAP_add_prim_triangles(PRIM_OBJ_BIKE_BWHEEL, i_bwheel);

	*/
}

#endif



// ========================================================
//
// FEEDING PERSON COORDINATES TO THE SHADOW MAPPER
//
// ========================================================


//
// Rotates all the points of the prim into world space and adds them
// to the Shadow mapper. Returns the index of the last point it
// added to the shadow mapper.
//

UWORD SMAP_add_tweened_points(
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
		Thing *p_thing)
{
	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;
	Matrix33 *mat;
	Matrix33 *mat_next;

	UWORD ans;

	SVector temp;

	PrimObject *p_obj;

	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
//	mat		 = &anim_info     ->Matrix;
//	mat_next = &anim_info_next->Matrix;

	offset.M[0] = anim_info->OffsetX + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8);
	offset.M[1] = anim_info->OffsetY + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8);
	offset.M[2] = anim_info->OffsetZ + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8);

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);

	SLONG	character_scale  = person_get_scale(p_thing);

	temp.X = (temp.X * character_scale) / 256;
	temp.Y = (temp.Y * character_scale) / 256;
	temp.Z = (temp.Z * character_scale) / 256;

	x += temp.X;
	y += temp.Y;
	z += temp.Z;	

	//
	// Create a temporary "tween" matrix between current and next
	//
/*
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			mat2.M[i][j] = mat->M[i][j] + ((mat_next->M[i][j] - mat->M[i][j]) * tween >> 8);
		}
	}
 */
	//
	// Stop the matrix flattening... do we have to bother?
	//

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	build_tween_matrix(&mat2 ,&m1, &m2, tween);


	normalise_matrix(&mat2);

	mat2.M[0][0] = (mat2.M[0][0] * character_scale) / 256;
	mat2.M[0][1] = (mat2.M[0][1] * character_scale) / 256;
	mat2.M[0][2] = (mat2.M[0][2] * character_scale) / 256;
	mat2.M[1][0] = (mat2.M[1][0] * character_scale) / 256;
	mat2.M[1][1] = (mat2.M[1][1] * character_scale) / 256;
	mat2.M[1][2] = (mat2.M[1][2] * character_scale) / 256;
	mat2.M[2][0] = (mat2.M[2][0] * character_scale) / 256;
	mat2.M[2][1] = (mat2.M[2][1] * character_scale) / 256;
	mat2.M[2][2] = (mat2.M[2][2] * character_scale) / 256;
	
	//
	// Apply local rotation matrix to get mat_final that rotates
	// the point into world space.
	//

	matrix_mult33(&mat_final, rot_mat, &mat2);

	//
	// Put all the points into the shadow mapper.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	//
	// Transform the shadow coordinates.
	//

	for (i = sp; i < ep; i++)
	{
		//
		// Find the world-space position of this point.
		//

		matrix_transform_small((struct Matrix31*)&temp, &mat_final, (struct SMatrix31*)&prim_points[i]);

		temp.X += x;
		temp.Y += y;
		temp.Z += z;

		ans = SMAP_point_add(
				float(temp.X),
				float(temp.Y),
				float(temp.Z));
	}

	return ans;
}


void SMAP_person(
		Thing     *p_thing, 
		UBYTE     *bitmap,	// 0 => transparent 255 => opaque
		UBYTE      u_res,
		UBYTE      v_res,
		SLONG      light_dx, 
		SLONG      light_dy,
		SLONG      light_dz)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG i_frame;
	SLONG i_steer;
	SLONG i_fwheel;
	SLONG i_bwheel;

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

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR SMAP_person");
		return;
	}

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
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR SMAP_person has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	void FIGURE_rotate_obj(
			SLONG xangle,
			SLONG yangle,
			SLONG zangle,
			Matrix33 *r3);

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		dt->Roll,
	   &r_matrix);

	//
	// Initialise the shadow mapper.
	//

	SMAP_init(
		float(light_dx),
		float(light_dy),
		float(light_dz),
		bitmap,
		u_res,
		v_res);

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	//
	// Add all the points to the shadow mapper.
	//

	#define SMAP_MAX_PARTS 20

	SLONG indices[SMAP_MAX_PARTS];

	for (i = 0; i < ele_count; i++)
	{
		ASSERT(WITHIN(i, 0, SMAP_MAX_PARTS - 1));

		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID&0x1f].BodyPart[i];

		indices[i] = SMAP_add_tweened_points(
							start_object + object_offset,
							p_thing->WorldPos.X >> 8,
							p_thing->WorldPos.Y >> 8,
							p_thing->WorldPos.Z >> 8,
							dt->AnimTween,
						   &ae1[i],
						   &ae2[i],
						   &r_matrix,
							dx,dy,dz, p_thing);
	}

	#ifdef BIKE

	if (p_thing->Genus.Person->Flags & FLAG_PERSON_BIKING)
	{
		Thing        *p_bike = TO_THING(p_thing->Genus.Person->InCar);
		BIKE_Drawinfo bdi    = BIKE_get_drawinfo(p_bike);

		//
		// Draw the shadow of the bike too!
		//

		/*

		i_frame = SMAP_prim_points(
					PRIM_OBJ_BIKE_FRAME,
					p_bike->WorldPos.X >> 8,
					p_bike->WorldPos.Y >> 8,
					p_bike->WorldPos.Z >> 8,
					bdi.yaw,
					bdi.pitch,
					bdi.roll);

		//
		// The front of the bike is where we physically model the wheel
		// it is not at the correct place for the pivot point of the front wheel
		// or the steering column.
		//

		i_steer = SMAP_prim_points(
					PRIM_OBJ_BIKE_STEER,
					bdi.steer_x,
					bdi.steer_y,
					bdi.steer_z,
					bdi.steer,
					bdi.pitch,
					bdi.roll);

		*/

		i_fwheel = SMAP_prim_points(
					PRIM_OBJ_BIKE_FWHEEL,
					bdi.front_x,
					bdi.front_y,
					bdi.front_z,
				    bdi.steer,
					bdi.pitch,
					bdi.roll);

		i_bwheel = SMAP_prim_points(
					PRIM_OBJ_BIKE_BWHEEL,
					bdi.back_x,
					bdi.back_y,
					bdi.back_z,
				    bdi.yaw,
					0,
					bdi.roll);
	}

	#endif

	SMAP_point_finished();

	//
	// Add all the triangles to the shadow mapper.
	//

	SLONG index = 0;

	for (i = 0; i < ele_count; i++)
	{
		ASSERT(WITHIN(i, 0, SMAP_MAX_PARTS - 1));


		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID&0x1f].BodyPart[i];
//		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];
//		object_offset = dt->TheChunk->PeopleTypes[0].BodyPart[i];

		SMAP_add_prim_triangles(
			start_object + object_offset,
			index);

		index = indices[i] + 1;
	}

	#ifdef BIKE

	if (p_thing->Genus.Person->Flags & FLAG_PERSON_BIKING)
	{
		//
		// Add the triangles and quads for each bit of the bike.
		// 

		/*
		SMAP_add_prim_triangles(PRIM_OBJ_BIKE_FRAME,  i_frame);
		SMAP_add_prim_triangles(PRIM_OBJ_BIKE_STEER,  i_steer);
		*/
		SMAP_add_prim_triangles(PRIM_OBJ_BIKE_BWHEEL, i_fwheel);
		SMAP_add_prim_triangles(PRIM_OBJ_BIKE_BWHEEL, i_bwheel);
	}
	
	#endif
}





// ========================================================
//
// PROJECTING THE SHADOW MAP ONTO A QUAD IN WORLD SPACE
//
// ========================================================

#define SMAP_CLIP_U_LESS	(1 << 0)
#define SMAP_CLIP_U_MORE	(1 << 1)
#define SMAP_CLIP_V_LESS	(1 << 2)
#define SMAP_CLIP_V_MORE	(1 << 3)

#define SMAP_MAX_LINKS 16

SMAP_Link SMAP_link[SMAP_MAX_LINKS];
SLONG     SMAP_link_upto;

//
// Returns TRUE if the poly is the wrong side of the
// shadow map... i.e. nearer to the light.
//

SLONG SMAP_wrong_side(SMAP_Link *sl)
{
	float order = 0.0F;
	float overorder;

	float av_wx = 0.0F;
	float av_wy = 0.0F;
	float av_wz = 0.0F;

	float av_n;

	while(sl)
	{
		av_wx += sl->wx;
		av_wy += sl->wy;
		av_wz += sl->wz;

		order += 1.0F;

		sl = sl->next;
	}

	overorder = 1.0F / order;

	av_wx *= overorder;
	av_wy *= overorder;
	av_wz *= overorder;

	av_n   = av_wx*SMAP_plane_nx + av_wy*SMAP_plane_ny + av_wz*SMAP_plane_nz;

	if (av_n >= SMAP_n_min + (SMAP_n_max - SMAP_n_min) * 0.75F)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//
// Converts the u,v in the polys to the texture uvs.
//

void SMAP_convert_uvs(SMAP_Link *sl)
{
	while(sl)
	{
		sl->u -= SMAP_u_min;
		sl->v -= SMAP_v_min;

		sl->u *= SMAP_u_map_mul_float;
		sl->v *= SMAP_v_map_mul_float;

		sl->u += SMAP_u_map_add_float;
		sl->v += SMAP_v_map_add_float;

		sl = sl->next;
	}
}



SMAP_Link *SMAP_project_onto_poly(SVector_F quad[], SLONG num_points)
{
	SLONG i;
	ULONG clip_and;
	ULONG clip_or;

	float along;

	SMAP_Link *poly;
	SMAP_Link *sl;
	SMAP_Link *sl1;
	SMAP_Link *sl2;
	SMAP_Link *sc;

	SMAP_Link **prev;
	SMAP_Link  *next;

	//
	// Clear the old points.
	// 

	SMAP_link_upto = 0;

	//
	// Create the un-clipped poly.
	//

	clip_or  = 0;
	clip_and = 0xffffffff;

	for (i = 0; i < num_points; i++)
	{
		sl = &SMAP_link[SMAP_link_upto++];

		sl->wx = quad[i].X;
		sl->wy = quad[i].Y;
		sl->wz = quad[i].Z;

		sl->u = sl->wx*SMAP_plane_ux + sl->wy*SMAP_plane_uy + sl->wz*SMAP_plane_uz;
		sl->v = sl->wx*SMAP_plane_vx + sl->wy*SMAP_plane_vy + sl->wz*SMAP_plane_vz;

		sl->next = &SMAP_link[SMAP_link_upto];

		if (SMAP_link_upto == num_points)
		{
			sl->next = NULL;
		}

		sl->clip = 0;

		if (sl->u < SMAP_u_min) {sl->clip |= SMAP_CLIP_U_LESS;}
		if (sl->u > SMAP_u_max) {sl->clip |= SMAP_CLIP_U_MORE;}
		if (sl->v < SMAP_v_min) {sl->clip |= SMAP_CLIP_V_LESS;}
		if (sl->v > SMAP_v_max) {sl->clip |= SMAP_CLIP_V_MORE;}

		clip_or  |= sl->clip;
		clip_and &= sl->clip;
	}

	if (clip_and)
	{
		//
		// No part of the shadow map falls on this poly.
		//

		return NULL;
	}

	//
	// Backface cull?
	//

	{
		float vec1u;
		float vec1v;
		float vec2u;
		float vec2v;

		float cross;

		SMAP_Link *p1 = &SMAP_link[0];
		SMAP_Link *p2 = &SMAP_link[1];
		SMAP_Link *p3 = &SMAP_link[2];

		vec1u = p2->u - p1->u;
		vec1v = p2->v - p1->v;
		vec2u = p3->u - p1->u;
		vec2v = p3->v - p1->v;

		cross = vec1u*vec2v - vec1v*vec2u;

		if (cross >= 0)
		{
			return NULL;
		}
	}

	poly = &SMAP_link[0];

	if (clip_or == 0)
	{
		//
		// We dont have to do any clipping!
		//

		if (SMAP_wrong_side(poly))
		{
			return NULL;
		}
		else
		{
			SMAP_convert_uvs(poly);

			return poly;
		}
	}

	// ======================
	//
	// CLIP TO SMAP_u_min
	//
	// ======================

	for (sl = poly; sl; sl = sl->next)
	{
		sl1 = sl;
		sl2 = sl->next;

		if (sl2 == NULL) {sl2 = poly;}

		if ((sl1->clip ^ sl2->clip) & SMAP_CLIP_U_LESS)
		{
			along = (SMAP_u_min - sl1->u) / (sl2->u - sl1->u);

			//
			// Create a new point on the line u = SMAP_u_min
			//

			ASSERT(WITHIN(SMAP_link_upto, 0, SMAP_MAX_LINKS - 1));

			sc = &SMAP_link[SMAP_link_upto++];

			sc->wx = sl1->wx + along * (sl2->wx - sl1->wx);
			sc->wy = sl1->wy + along * (sl2->wy - sl1->wy);
			sc->wz = sl1->wz + along * (sl2->wz - sl1->wz);

			sc->u  = SMAP_u_min;
			sc->v  = sl1->v  + along * (sl2->v  - sl1->v);

			//
			// Set its clipping flags.
			//

			sc->clip = 0;

			if (sc->u < SMAP_u_min) {sc->clip |= SMAP_CLIP_U_LESS;}
			if (sc->u > SMAP_u_max) {sc->clip |= SMAP_CLIP_U_MORE;}
			if (sc->v < SMAP_v_min) {sc->clip |= SMAP_CLIP_V_LESS;}
			if (sc->v > SMAP_v_max) {sc->clip |= SMAP_CLIP_V_MORE;}

			//
			// Insert it in the linked list.
			//

			sc->next = sl->next;
			sl->next = sc;

			//
			// Don't clip this point again...
			//

			sl = sl->next;
		}
	}

	//
	// Take out all points in the linked list outside the SMAP_u_min boundary.
	//

	prev = &poly;
	next =  poly;

	clip_and = 0xffffffff;
	clip_or  = 0;

	while(1)
	{
		if (next == NULL)
		{
			break;
		}

		if (next->clip & SMAP_CLIP_U_LESS)
		{
			//
			// Get rid of this point.
			//

		   *prev = next->next;
		    next = next->next;
		}
		else
		{
			//
			// Keep this point.
			//

			clip_and &= next->clip;
			clip_or  |= next->clip;

			prev = &next->next;
			next =  next->next;
		}
	}

	//
	// Early outs?
	// 

	if ( clip_and) {return NULL;}
	if (!clip_or)  {if (SMAP_wrong_side(poly)) {return NULL;} else {SMAP_convert_uvs(poly); return poly;}}

	// ======================
	//
	// CLIP TO SMAP_u_max
	//
	// ======================

	for (sl = poly; sl; sl = sl->next)
	{
		sl1 = sl;
		sl2 = sl->next;

		if (sl2 == NULL) {sl2 = poly;}

		if ((sl1->clip ^ sl2->clip) & SMAP_CLIP_U_MORE)
		{
			along = (SMAP_u_max - sl1->u) / (sl2->u - sl1->u);

			//
			// Create a new point on the line u = SMAP_u_max
			//

			ASSERT(WITHIN(SMAP_link_upto, 0, SMAP_MAX_LINKS - 1));

			sc = &SMAP_link[SMAP_link_upto++];

			sc->wx = sl1->wx + along * (sl2->wx - sl1->wx);
			sc->wy = sl1->wy + along * (sl2->wy - sl1->wy);
			sc->wz = sl1->wz + along * (sl2->wz - sl1->wz);

			sc->u  = SMAP_u_max;
			sc->v  = sl1->v  + along * (sl2->v  - sl1->v);

			//
			// Set its clipping flags.
			//

			sc->clip = 0;

			if (sc->u < SMAP_u_min) {sc->clip |= SMAP_CLIP_U_LESS;}
			if (sc->u > SMAP_u_max) {sc->clip |= SMAP_CLIP_U_MORE;}
			if (sc->v < SMAP_v_min) {sc->clip |= SMAP_CLIP_V_LESS;}
			if (sc->v > SMAP_v_max) {sc->clip |= SMAP_CLIP_V_MORE;}

			//
			// Insert it in the linked list.
			//

			sc->next = sl->next;
			sl->next = sc;

			//
			// Don't clip this point again...
			//

			sl = sl->next;
		}
	}

	//
	// Take out all points in the linked list outside the SMAP_u_max boundary.
	//

	prev = &poly;
	next =  poly;

	clip_and = 0xffffffff;
	clip_or  = 0;

	while(1)
	{
		if (next == NULL)
		{
			break;
		}

		if (next->clip & SMAP_CLIP_U_MORE)
		{
			//
			// Get rid of this point.
			//

		   *prev = next->next;
		    next = next->next;
		}
		else
		{
			//
			// Keep this point.
			//

			clip_and &= next->clip;
			clip_or  |= next->clip;

			prev = &next->next;
			next =  next->next;
		}
	}

	//
	// Early outs?
	// 

	if ( clip_and) {return NULL;}
	if (!clip_or)  {if (SMAP_wrong_side(poly)) {return NULL;} else {SMAP_convert_uvs(poly); return poly;}}

	// ======================
	//
	// CLIP TO SMAP_v_min
	//
	// ======================

	for (sl = poly; sl; sl = sl->next)
	{
		sl1 = sl;
		sl2 = sl->next;

		if (sl2 == NULL) {sl2 = poly;}

		if ((sl1->clip ^ sl2->clip) & SMAP_CLIP_V_LESS)
		{
			along = (SMAP_v_min - sl1->v) / (sl2->v - sl1->v);

			//
			// Create a new point on the line v = SMAP_v_min
			//

			ASSERT(WITHIN(SMAP_link_upto, 0, SMAP_MAX_LINKS - 1));

			sc = &SMAP_link[SMAP_link_upto++];

			sc->wx = sl1->wx + along * (sl2->wx - sl1->wx);
			sc->wy = sl1->wy + along * (sl2->wy - sl1->wy);
			sc->wz = sl1->wz + along * (sl2->wz - sl1->wz);

			sc->u  = sl1->u  + along * (sl2->u  - sl1->u);
			sc->v  = SMAP_v_min;

			//
			// Set its clipping flags.
			//

			sc->clip = 0;

			if (sc->u < SMAP_u_min) {sc->clip |= SMAP_CLIP_U_LESS;}
			if (sc->u > SMAP_u_max) {sc->clip |= SMAP_CLIP_U_MORE;}
			if (sc->v < SMAP_v_min) {sc->clip |= SMAP_CLIP_V_LESS;}
			if (sc->v > SMAP_v_max) {sc->clip |= SMAP_CLIP_V_MORE;}

			//
			// Insert it in the linked list.
			//

			sc->next = sl->next;
			sl->next = sc;

			//
			// Don't clip this point again...
			//

			sl = sl->next;
		}
	}

	//
	// Take out all points in the linked list outside the SMAP_v_min boundary.
	//

	prev = &poly;
	next =  poly;

	clip_and = 0xffffffff;
	clip_or  = 0;

	while(1)
	{
		if (next == NULL)
		{
			break;
		}

		if (next->clip & SMAP_CLIP_V_LESS)
		{
			//
			// Get rid of this point.
			//

		   *prev = next->next;
		    next = next->next;
		}
		else
		{
			//
			// Keep this point.
			//

			clip_and &= next->clip;
			clip_or  |= next->clip;

			prev = &next->next;
			next =  next->next;
		}
	}

	//
	// Early outs?
	// 

	if ( clip_and) {return NULL;}
	if (!clip_or)  {if (SMAP_wrong_side(poly)) {return NULL;} else {SMAP_convert_uvs(poly); return poly;}}

	// ======================
	//
	// CLIP TO SMAP_v_max
	//
	// ======================

	for (sl = poly; sl; sl = sl->next)
	{
		sl1 = sl;
		sl2 = sl->next;

		if (sl2 == NULL) {sl2 = poly;}

		if ((sl1->clip ^ sl2->clip) & SMAP_CLIP_V_MORE)
		{
			along = (SMAP_v_max - sl1->v) / (sl2->v - sl1->v);

			//
			// Create a new point on the line v = SMAP_v_max
			//

			ASSERT(WITHIN(SMAP_link_upto, 0, SMAP_MAX_LINKS - 1));

			sc = &SMAP_link[SMAP_link_upto++];

			sc->wx = sl1->wx + along * (sl2->wx - sl1->wx);
			sc->wy = sl1->wy + along * (sl2->wy - sl1->wy);
			sc->wz = sl1->wz + along * (sl2->wz - sl1->wz);

			sc->u  = sl1->u  + along * (sl2->u  - sl1->u);
			sc->v  = SMAP_v_max;

			//
			// Set its clipping flags.
			//

			sc->clip = 0;

			if (sc->u < SMAP_u_min) {sc->clip |= SMAP_CLIP_U_LESS;}
			if (sc->u > SMAP_u_max) {sc->clip |= SMAP_CLIP_U_MORE;}
			if (sc->v < SMAP_v_min) {sc->clip |= SMAP_CLIP_V_LESS;}
			if (sc->v > SMAP_v_max) {sc->clip |= SMAP_CLIP_V_MORE;}

			//
			// Insert it in the linked list.
			//

			sc->next = sl->next;
			sl->next = sc;

			//
			// Don't clip this point again...
			//

			sl = sl->next;
		}
	}

	//
	// Take out all points in the linked list outside the SMAP_v_max boundary.
	//

	prev = &poly;
	next =  poly;

	clip_and = 0xffffffff;
	clip_or  = 0;

	while(1)
	{
		if (next == NULL)
		{
			break;
		}

		if (next->clip & SMAP_CLIP_V_MORE)
		{
			//
			// Get rid of this point.
			//

		   *prev = next->next;
		    next = next->next;
		}
		else
		{
			//
			// Keep this point.
			//

			clip_and &= next->clip;
			clip_or  |= next->clip;

			prev = &next->next;
			next =  next->next;
		}
	}

	//
	// Early outs?
	// 

	if ( clip_and) {return NULL;}
	if (!clip_or)  {if (SMAP_wrong_side(poly)) {return NULL;} else {SMAP_convert_uvs(poly); return poly;}}

	//
	// Hmm! Done clipping and still not clipped!
	//

	ASSERT(0);

	return NULL;
}
