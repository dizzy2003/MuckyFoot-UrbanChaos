//
// Functions that act on the imported meshes
//

#include "always.h"
#include "imp.h"
#include "matrix.h"
#include "mf.h"
#include "os.h"


void MF_load_textures(IMP_Mesh *im)
{
	SLONG i;

	IMP_Mat *it;

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		//
		// Load the texture if appropriate.
		//

		if (it->has_texture)
		{
			it->ot_tex = OS_texture_create(it->tname);
		}
		else
		{
			it->ot_tex = NULL;
		}

		//
		// Load the bumpmap if this material has one.
		//

		if (it->has_bumpmap)
		{
			it->ot_bpos = OS_texture_create(it->bname, FALSE);
			it->ot_bneg = OS_texture_create(it->bname, TRUE );
		}
		else
		{
			it->ot_bpos = NULL;
			it->ot_bneg = NULL;
		}
	}
}

void MF_backup(IMP_Mesh *im)
{
	im->old_vert  = (IMP_Vert  *) malloc(sizeof(IMP_Vert)  * im->num_verts );
	im->old_svert = (IMP_Svert *) malloc(sizeof(IMP_Svert) * im->num_sverts);

	memcpy(im->old_vert,  im->vert,  sizeof(IMP_Vert)  * im->num_verts );
	memcpy(im->old_svert, im->svert, sizeof(IMP_Svert) * im->num_sverts);
}

void MF_rotate_mesh(
		IMP_Mesh *im,
		float     yaw,
		float     pitch,
		float     roll,
		float     scale,
		float     pos_x,
		float     pos_y,
		float     pos_z)
{
	SLONG i;

	float matrix[9];

	MATRIX_calc(
		matrix,
		yaw,
		pitch,
		roll);

	//
	// In the sverts...
	//

	for (i = 0; i < im->num_sverts; i++)
	{
		im->svert[i] = im->old_svert[i];

		MATRIX_MUL(
			matrix,
			im->svert[i].nx,
			im->svert[i].ny,
			im->svert[i].nz);

		MATRIX_MUL(
			matrix,
			im->svert[i].dxdu,
			im->svert[i].dydu,
			im->svert[i].dzdu);

		MATRIX_MUL(
			matrix,
			im->svert[i].dxdv,
			im->svert[i].dydv,
			im->svert[i].dzdv);
	}

	//
	// Only scale the vertices- not the normals!
	//

	MATRIX_scale(
		matrix,
		scale);

	for (i = 0; i < im->num_verts; i++)
	{
		im->vert[i] = im->old_vert[i];

		MATRIX_MUL(
			matrix,
			im->vert[i].x,
			im->vert[i].y,
			im->vert[i].z);

		im->vert[i].x += pos_x;
		im->vert[i].y += pos_y;
		im->vert[i].z += pos_z;
	}
}


void MF_rotate_mesh(
		IMP_Mesh *im,
		float     pos_x,
		float     pos_y,
		float     pos_z,
		float     matrix[9])
{
	SLONG i;

	//
	// In the sverts...
	//

	for (i = 0; i < im->num_sverts; i++)
	{
		im->svert[i] = im->old_svert[i];

		MATRIX_MUL(
			matrix,
			im->svert[i].nx,
			im->svert[i].ny,
			im->svert[i].nz);

		MATRIX_MUL(
			matrix,
			im->svert[i].dxdu,
			im->svert[i].dydu,
			im->svert[i].dzdu);

		MATRIX_MUL(
			matrix,
			im->svert[i].dxdv,
			im->svert[i].dydv,
			im->svert[i].dzdv);
	}

	//
	// The verts...
	//

	for (i = 0; i < im->num_verts; i++)
	{
		im->vert[i] = im->old_vert[i];

		MATRIX_MUL(
			matrix,
			im->vert[i].x,
			im->vert[i].y,
			im->vert[i].z);

		im->vert[i].x += pos_x;
		im->vert[i].y += pos_y;
		im->vert[i].z += pos_z;
	}
}

void MF_transform_points(IMP_Mesh *im)
{
	SLONG i;

	IMP_Vert *iv;

	ASSERT(im->num_verts <= OS_MAX_TRANS);

	for (i = 0; i < im->num_verts; i++)
	{
		iv = &im->vert[i];

		OS_transform(
			iv->x,
			iv->y,
			iv->z,
		   &OS_trans[i]);
	}
}





void MF_diffuse_spotlight(
		IMP_Mesh *im,
		float     light_x,
		float     light_y,
		float     light_z,
		float     light_matrix[9],
		float     light_lens)		// The bigger the lens the smaller the spotlight.
{
	SLONG i;

	float x;
	float y;
	float z;
	float X;
	float Y;
	float Z;

	float dprod;

	IMP_Vert  *iv;
	IMP_Svert *is;

	//
	// The position of the light map on the mesh.
	//

	for (i = 0; i < im->num_verts; i++)
	{
		iv = &im->vert[i];

		//
		// Rotate the point into light-source space.
		//

		x = iv->x - light_x;
		y = iv->y - light_y;
		z = iv->z - light_z;

		MATRIX_MUL(
			light_matrix,
			x,
			y,
			z);

		if (z > 0.0F)	// No zclip plane!
		{
			//
			// Perspective transform.
			//

			Z = light_lens / z;

			X = 0.5F + x * Z;
			Y = 0.5F + y * Z;

			iv->lu = X;
			iv->lv = Y;
		}
		else
		{
			//
			// This point is behind the light!
			//

			ASSERT(0);

			iv->lu = 0.0F;
			iv->lv = 0.0F;
		}
	}

	//
	// The gouraud shade value and the bump map offsets.
	//

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];

		//
		// Gouraud.
		//

		dprod =
			is->nx * light_matrix[6] + 
			is->ny * light_matrix[7] + 
			is->nz * light_matrix[8];

		if (dprod < 0)
		{
			SLONG bright = ftol(dprod * -255.0F);

			ASSERT(WITHIN(bright, 0, 255));

			is->colour = bright | (bright << 8) | (bright << 16);
		}
		else
		{
			is->colour = 0x00000000;
		}

		//
		// Bumpmap offsets.
		//

		dprod =
			is->dxdu * light_matrix[6] + 
			is->dydu * light_matrix[7] + 
			is->dzdu * light_matrix[8];

		is->du = dprod * 0.005F;

		dprod =
			is->dxdv * light_matrix[6] + 
			is->dydv * light_matrix[7] + 
			is->dzdv * light_matrix[8];

		is->dv = dprod * 0.005F;
	}
}



void MF_specular_spotlight(
		IMP_Mesh *im,
		float     light_x,
		float     light_y,
		float     light_z,
		float     light_matrix[9],
		float     light_lens)
{
	SLONG i;

	float x;
	float y;
	float z;
	float X;
	float Y;
	float Z;

	float dx;
	float dy;
	float dz;

	float along_x;
	float along_y;
	float along_z;

	float dprod;
	float along;

	float len;
	float overlen;

	float rx;
	float ry;
	float rz;

	float bright;

	IMP_Vert  *iv;
	IMP_Svert *is;
	IMP_Mat   *it;

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		iv = &im->vert [is->vert];
		it = &im->mat  [is->mat ];

		//
		// Rotate the point into light-source space.
		//

		x = iv->x - light_x;
		y = iv->y - light_y;
		z = iv->z - light_z;

		MATRIX_MUL(
			light_matrix,
			x,
			y,
			z);

		if (z > 0.0F)	// No zclip plane!
		{
			//
			// Perspective transform.
			//

			Z = light_lens / z;

			X = 0.5F + x * Z;
			Y = 0.5F + y * Z;

			//
			// Work out the reflected ray from the camera through the point.
			//

			//
			// The ray from the camera to the point.
			//

			dx = iv->x - OS_cam_x;
			dy = iv->y - OS_cam_y;
			dz = iv->z - OS_cam_z;

			//
			// The ray reflected from the normal.
			//

			dprod  = dx*is->nx + dy*is->ny + dz*is->nz;
			dprod *= 2.0F;

			rx = dx - is->nx * dprod;
			ry = dy - is->ny * dprod;
			rz = dz - is->nz * dprod;

			//
			// Normalise the ray.
			//

			{
				float overlen = 1.0F / sqrt(rx*rx + ry*ry + rz*rz);

				rx *= overlen;
				ry *= overlen;
				rz *= overlen;
			}

			//
			// Work out how parallel this ray is to each axis.
			//

			along_x =
				rx * light_matrix[0] +
				ry * light_matrix[1] +
				rz * light_matrix[2];

			along_y =
				rx * light_matrix[3] +
				ry * light_matrix[4] +
				rz * light_matrix[5];

			along_z =
				rx * light_matrix[6] +
				ry * light_matrix[7] +
				rz * light_matrix[8];

			//
			// How bright is the hightlight?
			// 

			bright = -along_z;

			if (bright <= 0.0F)
			{
				is->colour = 0x00000000;
			}
			else
			{
				//
				// Darker the further from the middle of the spotlight.
				//

				len  = (X-0.5F)*(X-0.5F) + (Y-0.5F)*(Y-0.5F);

				if (len > 0.25F)
				{
					is->colour = 0x00000000;
				}
				else
				{
					len *= 4.0F;
					len *= len;
					len *= len;
					len  = 1.0F - len;

					bright *= len;

					is->colour  = ftol(bright * (255.0F * it->shinstr));
					is->colour |= is->colour << 8;
					is->colour |= is->colour << 8;
				}
			}

			//
			// If the reflected ray and the direction of the light do not match,
			// then make the uv coordinates leave the specular highlight.
			//

			X += along_x * it->shininess;
			Y += along_y * it->shininess;

			is->lu = X;
			is->lv = Y;
		}
		else
		{
			//
			// This point is behind the light!
			//

			ASSERT(0);

			is->lu = 0.0F;
			is->lv = 0.0F;
		}
	}
}



//
// The array we build before we start adding triangles.
// 

#define MF_MAX_SVERTS 16384

OS_Vert MF_vert[MF_MAX_SVERTS];

void MF_add_triangles_normal(IMP_Mesh *im, ULONG draw)
{
	SLONG i;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;

	//
	// Build an array of OS_Verts using the shared vertices in the mesh.
	//

	ASSERT(im->num_sverts <= MF_MAX_SVERTS);

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		ov = &MF_vert  [i];

		ov->trans    = is->vert;
		ov->index    = NULL;
		ov->colour   = is->colour;
		ov->specular = 0x00000000;
		ov->u1       = is->u;
		ov->v1       = is->v;
		ov->u2       = 0.0F;
		ov->v2       = 0.0F;
	}

	//
	// Get a buffers for each texture.
	//

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		it->ob = OS_buffer_new();
	}

	//
	// Add all the faces.
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];
		it = &im->mat [ic->mat];

		OS_buffer_add_triangle(
			it->ob,
		   &MF_vert[ic->s[0]],
		   &MF_vert[ic->s[1]],
		   &MF_vert[ic->s[2]]);
	}

	//
	// Render all the buffers.
	//

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		OS_buffer_draw(
			it->ob,
			it->ot_tex,
			NULL,
			draw | ((it->sided == IMP_SIDED_DOUBLE) ? OS_DRAW_DOUBLESIDED : 0));
	}
}


void MF_add_triangles_normal_colour(IMP_Mesh *im, ULONG draw, ULONG colour)
{
	SLONG i;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;

	//
	// Build an array of OS_Verts using the shared vertices in the mesh.
	//

	ASSERT(im->num_sverts <= MF_MAX_SVERTS);

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		ov = &MF_vert  [i];

		ov->trans    = is->vert;
		ov->index    = NULL;
		ov->colour   = colour;
		ov->specular = 0x00000000;
		ov->u1       = is->u;
		ov->v1       = is->v;
		ov->u2       = 0.0F;
		ov->v2       = 0.0F;
	}

	//
	// Get a buffers for each texture.
	//

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		it->ob = OS_buffer_new();
	}

	//
	// Add all the faces.
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];
		it = &im->mat [ic->mat];

		OS_buffer_add_triangle(
			it->ob,
		   &MF_vert[ic->s[0]],
		   &MF_vert[ic->s[1]],
		   &MF_vert[ic->s[2]]);
	}

	//
	// Render all the buffers.
	//

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		OS_buffer_draw(
			it->ob,
			it->ot_tex,
			NULL,
			draw | ((it->sided == IMP_SIDED_DOUBLE) ? OS_DRAW_DOUBLESIDED : 0));
	}
}




void MF_add_triangles_light(IMP_Mesh *im, OS_Texture *ot, ULONG draw)
{
	SLONG i;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;
	IMP_Vert  *iv;
	OS_Buffer *ob;

	//
	// Build the array of OS_Verts.
	//

	ASSERT(im->num_sverts <= MF_MAX_SVERTS);

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		ov = &MF_vert  [i];
		iv = &im->vert [is->vert];

		ov->trans    = is->vert;
		ov->index    = NULL;
		ov->colour   = is->colour;
		ov->specular = 0x00000000;
		ov->u1       = iv->lu;
		ov->v1       = iv->lv;
		ov->u2       = 0.0F;
		ov->v2       = 0.0F;
	}

	//
	// Get a buffers for our texture.
	//

	ob = OS_buffer_new();

	//
	// Add all the faces.
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];

		OS_buffer_add_triangle(
			ob,
		   &MF_vert[ic->s[0]],
		   &MF_vert[ic->s[1]],
		   &MF_vert[ic->s[2]]);
	}

	//
	// Render the buffer.
	//

	OS_buffer_draw(
		ob,
		ot,
		NULL,
		draw);
}

void MF_add_triangles_light_bumpmapped(IMP_Mesh *im, OS_Texture *ot, ULONG draw)
{
	SLONG i;
	SLONG pass;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;
	IMP_Vert  *iv;
	OS_Buffer *ob;

	for (pass = 0; pass < 2; pass++)
	{
		//
		// Build the array of OS_Verts.
		//

		if (pass == 0)
		{
			//
			// All the data the first time around.
			//

			ASSERT(im->num_sverts <= MF_MAX_SVERTS);

			for (i = 0; i < im->num_sverts; i++)
			{
				is = &im->svert[i];
				ov = &MF_vert  [i];
				iv = &im->vert [is->vert];

				ov->trans    = is->vert;
				ov->index    = NULL;
				ov->colour   = is->colour;
				ov->specular = 0x00000000;
				ov->u1       = iv->lu;
				ov->v1       = iv->lv;
				ov->u2       = is->u - is->du;
				ov->v2       = is->v - is->dv;
			}
		}
		else
		{
			//
			// Clear the index and update the shifted texture coordinates for the bumpmap pass.
			//

			for (i = 0; i < im->num_sverts; i++)
			{
				ov = &MF_vert  [i];
				is = &im->svert[i];

				ov->index = NULL;
				ov->u2    = is->u + is->du;
				ov->v2    = is->v + is->dv;
			}
		}

		//
		// Get a buffers for each texture.
		//

		for (i = 0; i < im->num_mats; i++)
		{
			it = &im->mat[i];

			if (it->has_bumpmap || pass == 0)
			{
				it->ob = OS_buffer_new();
			}
		}

		//
		// Add all the faces.
		//

		for (i = 0; i < im->num_faces; i++)
		{
			ic = &im->face[i];
			it = &im->mat [ic->mat];

			if (it->has_bumpmap || pass == 0)
			{
				OS_buffer_add_triangle(
					it->ob,
				   &MF_vert[ic->s[0]],
				   &MF_vert[ic->s[1]],
				   &MF_vert[ic->s[2]]);
			}
		}

		//
		// Render all the buffers.
		//

		for (i = 0; i < im->num_mats; i++)
		{
			it = &im->mat[i];

			if (it->has_bumpmap)
			{
				OS_buffer_draw(
					it->ob,
					ot,
					(pass == 0) ? it->ot_bpos : it->ot_bneg,
					draw | OS_DRAW_TEX_MUL);
			}
			else
			{
				if (pass == 0)
				{
					OS_buffer_draw(
						it->ob,
						ot,
						NULL,
						draw);
				}
			}
		}

		draw |= OS_DRAW_ADD;	// The second pass is always additive.
	}
}


void MF_add_triangles_specular(IMP_Mesh *im, OS_Texture *ot, ULONG draw)
{
	SLONG i;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Face  *ic;
	IMP_Vert  *iv;
	OS_Buffer *ob;
	OS_Vert   *ov1;
	OS_Vert   *ov2;
	OS_Vert   *ov3;

	//
	// Build the array of OS_Verts.
	//

	ASSERT(im->num_sverts <= MF_MAX_SVERTS);

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		ov = &MF_vert  [i];

		ov->trans    = is->vert;
		ov->index    = NULL;
		ov->colour   = is->colour;
		ov->specular = 0x000000;
		ov->u1       = is->lu;
		ov->v1       = is->lv;
		ov->u2       = 0.0F;
		ov->v2       = 0.0F;
	}

	//
	// Get a buffers for our texture.
	//

	ob = OS_buffer_new();

	//
	// Add all the faces.
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];
		
		//
		// The three points of this face.
		// 

		ov1 = &MF_vert[ic->s[0]];
		ov2 = &MF_vert[ic->s[1]];
		ov3 = &MF_vert[ic->s[2]];

		OS_buffer_add_triangle(
			ob,
			ov1,
			ov2,
			ov3);
	}

	//
	// Render the buffer.
	//

	OS_buffer_draw(
		ob,
		ot,
		NULL,
		draw);
}


void MF_add_triangles_specular_bumpmapped(IMP_Mesh *im, OS_Texture *ot, ULONG draw)
{
	SLONG i;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;
	IMP_Vert  *iv;
	OS_Buffer *ob;

	//
	// Build the array of OS_Verts.
	//

	ASSERT(im->num_sverts <= MF_MAX_SVERTS);

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		ov = &MF_vert  [i];
		iv = &im->vert [is->vert];

		ov->trans    = is->vert;
		ov->index    = NULL;
		ov->colour   = is->colour;
		ov->specular = 0x00000000;
		ov->u1       = is->lu;
		ov->v1       = is->lv;
		ov->u2       = is->u;
		ov->v2       = is->v;
	}

	//
	// Get a buffers for each texture.
	//

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		it->ob = OS_buffer_new();
	}

	//
	// Add all the faces.
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];
		it = &im->mat [ic->mat];

		OS_buffer_add_triangle(
			it->ob,
		   &MF_vert[ic->s[0]],
		   &MF_vert[ic->s[1]],
		   &MF_vert[ic->s[2]]);
	}

	//
	// Render all the buffers.
	//

	for (i = 0; i < im->num_mats; i++)
	{
		it = &im->mat[i];

		if (it->has_bumpmap)
		{
			OS_buffer_draw(
				it->ob,
				ot,
				it->ot_bpos,
				draw | OS_DRAW_TEX_MUL);
		}
		else
		{
			OS_buffer_draw(
				it->ob,
				ot,
				NULL,
				draw);
		}
	}
}


/*
void MF_add_triangles_specular_bumpmapped(IMP_Mesh *im, OS_Texture *ot, ULONG draw)
{
	SLONG i;
	SLONG pass;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;
	IMP_Vert  *iv;
	OS_Buffer *ob;

	for (pass = 0; pass < 2; pass++)
	{
		//
		// Build the array of OS_Verts.
		//

		if (pass == 0)
		{
			//
			// All the data the first time around.
			//

			ASSERT(im->num_sverts <= MF_MAX_SVERTS);

			for (i = 0; i < im->num_sverts; i++)
			{
				is = &im->svert[i];
				ov = &MF_vert  [i];
				iv = &im->vert [is->vert];

				ov->trans    = is->vert;
				ov->index    = NULL;
				ov->colour   = is->colour;
				ov->specular = 0x00000000;
				ov->u1       = is->lu;
				ov->v1       = is->lv;
				ov->u2       = is->u;
				ov->v2       = is->v;
			}
		}
		else
		{
			//
			// Clear the index and update the shifted texture coordinates for the bumpmap pass.
			//

			for (i = 0; i < im->num_sverts; i++)
			{
				ov = &MF_vert  [i];
				is = &im->svert[i];

				ov->index = NULL;
				ov->u2    = is->u + is->du;
				ov->v2    = is->v + is->dv;
			}
		}

		//
		// Get a buffers for each texture.
		//

		for (i = 0; i < im->num_mats; i++)
		{
			it = &im->mat[i];

			it->ob = OS_buffer_new();
		}

		//
		// Add all the faces.
		//

		for (i = 0; i < im->num_faces; i++)
		{
			ic = &im->face[i];
			it = &im->mat [ic->mat];

			OS_buffer_add_triangle(
				it->ob,
			   &MF_vert[ic->s[0]],
			   &MF_vert[ic->s[1]],
			   &MF_vert[ic->s[2]]);
		}

		//
		// Render all the buffers.
		//

		for (i = 0; i < im->num_mats; i++)
		{
			it = &im->mat[i];

			OS_buffer_draw(
				it->ob,
				ot,
				(pass == 0) ? it->ot_bpos : it->ot_bneg,
				draw | OS_DRAW_TEX_MUL);
		}
	}
}

*/

void MF_add_triangles_specular_shadowed(IMP_Mesh *im, OS_Texture *ot_specdot, OS_Texture *ot_diffdot, ULONG draw)
{
	SLONG i;

	IMP_Svert *is;
	OS_Vert   *ov;
	IMP_Mat   *it;
	IMP_Face  *ic;
	IMP_Vert  *iv;
	OS_Buffer *ob;

	//
	// Build the array of OS_Verts.
	//

	ASSERT(im->num_sverts <= MF_MAX_SVERTS);

	for (i = 0; i < im->num_sverts; i++)
	{
		is = &im->svert[i];
		ov = &MF_vert  [i];
		iv = &im->vert [is->vert];

		ov->trans    = is->vert;
		ov->index    = NULL;
		ov->colour   = is->colour;
		ov->specular = 0x00000000;
		ov->u1       = is->lu;
		ov->v1       = is->lv;
		ov->u2       = iv->lu;
		ov->v2       = iv->lv;
	}

	//
	// Get a buffers for our texture.
	//

	ob = OS_buffer_new();

	//
	// Add all the faces.
	//

	for (i = 0; i < im->num_faces; i++)
	{
		ic = &im->face[i];

		OS_buffer_add_triangle(
			ob,
		   &MF_vert[ic->s[0]],
		   &MF_vert[ic->s[1]],
		   &MF_vert[ic->s[2]]);
	}

	//
	// Render the buffer.
	//

	OS_buffer_draw(
		ob,
		ot_specdot,
		ot_diffdot,
		draw);
}

