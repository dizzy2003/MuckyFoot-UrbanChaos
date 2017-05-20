//
// The wire-frame spinny objects
//

#include "always.h"
#include "key.h"
#include "imp.h"
#include "matrix.h"
#include "mf.h"
#include "os.h"
#include "wire.h"



//
// Our meshes.
//

#define WIRE_NUM_MESHES 4

IMP_Mesh WIRE_mesh[WIRE_NUM_MESHES];


//
// The textures.
//

OS_Texture *WIRE_ot_line;
OS_Texture *WIRE_ot_dot;



SLONG WIRE_last;
SLONG WIRE_now;


//
// The box we use as the zbuffer hack.
// 

typedef struct
{
	float x;
	float y;
	float z;

	OS_Vert ov;
 
} WIRE_Point;

#define WIRE_NUM_POINTS 8

WIRE_Point WIRE_point[WIRE_NUM_POINTS];



//
// The current mesh we are drawing and how we are drawing it...
//

#define WIRE_MODE_NONE_WIRE			0
#define WIRE_MODE_WIRE_BRIGHT		1
#define WIRE_MODE_BRIGHT_TEXTURE	2
#define WIRE_MODE_TEXTURE_NONE		3

SLONG WIRE_current_mesh;
SLONG WIRE_current_mode;
SLONG WIRE_current_countdown;





//
// Initialises the box so it will encompass the given mesh, no
// matter how it is rotated.
//

void WIRE_plane_init(IMP_Mesh *im)
{
	SLONG i;

	for (i = 0; i < WIRE_NUM_POINTS; i++)	
	{
		WIRE_point[i].x = (i & 1) ? im->max_x + 1.0F : im->min_x - 1.0F;
		WIRE_point[i].y = (i & 2) ? im->max_y + 5.0F : im->min_y - 5.0F;
		WIRE_point[i].z = (i & 4) ? im->max_z + 1.0F : im->min_z - 1.0F;
	}
}

void WIRE_plane_rotate(IMP_Mesh *im, float angle)
{
	SLONG i;

	float matrix[9];

	MATRIX_calc(
		matrix,
		angle,
		0.0F,
		0.0F);

	for (i = 0; i < WIRE_NUM_POINTS; i++)	
	{
		WIRE_point[i].x = (i & 1) ? im->max_x + 5.0F : im->min_x - 5.0F;
		WIRE_point[i].z = (i & 4) ? im->max_z + 5.0F : im->min_z - 5.0F;

		MATRIX_MUL(
			matrix,
			WIRE_point[i].x,
			WIRE_point[i].y,	// y will be unchanged.
			WIRE_point[i].z);
	}
}


//
// Makes the plane sink down.
//

void WIRE_plane_process()
{
	SLONG i;


	WIRE_now = OS_ticks();

	if (WIRE_last < WIRE_now - 500)
	{
		WIRE_last = WIRE_now - 500;
	}

	if (KEY_on[KEY_S])
	{
		WIRE_last -= 100;
	}

	while(WIRE_last < WIRE_now)
	{
		WIRE_last += 1000 / 50;	// Process 20 times a second...

		if (WIRE_current_mode == WIRE_MODE_TEXTURE_NONE)
		{
			if (WIRE_current_countdown == 0)
			{
				for (i = 0; i < WIRE_NUM_POINTS; i++)
				{
					if (i &  2)
					{
						WIRE_point[i].y += 0.3F;
					}
				}
			}
		}
		else
		{
			for (i = 0; i < WIRE_NUM_POINTS; i++)
			{
				if (i &  2)
				{
					WIRE_point[i].y -= 0.2F;
				}
			}
		}
	}
}


void WIRE_plane_draw()
{
	SLONG i;
	SLONG px;
	SLONG pz;
	SLONG index;
	
	OS_Buffer  *ob = OS_buffer_new();
	OS_Vert    *ov;
	WIRE_Point *wp;

	//
	// Rotate all the points.
	//

	for (i = 0; i < WIRE_NUM_POINTS; i++)
	{
		wp = &WIRE_point[i];

		OS_transform(
			wp->x,
			wp->y,
			wp->z,
		   &OS_trans[i]);

		ov = &wp->ov;
	
		ov->trans    = i;
		ov->index    = NULL;
		ov->u1       = 0.0F;
		ov->v1       = 0.0F;
		ov->u2       = 0.0F;
		ov->v2       = 0.0F;
		ov->colour   = 0x44444444;
		ov->specular = 0x00000000;
	}

	//
	// Add each face to our buffer.
	//

	struct
	{
		UBYTE p1;
		UBYTE p2;
		UBYTE p3;
		UBYTE p4;

	} face[5] =
	{
		{0,1,2,3},
		{2,3,6,7},
		{6,7,4,5},
		{1,5,3,7},
		{4,0,6,2}
	};


	for (i = 0; i < 5; i++)
	{
		OS_buffer_add_triangle(
			ob,
		   &WIRE_point[face[i].p3].ov,
		   &WIRE_point[face[i].p2].ov,
		   &WIRE_point[face[i].p1].ov);

		OS_buffer_add_triangle(
			ob,
		   &WIRE_point[face[i].p2].ov,
		   &WIRE_point[face[i].p3].ov,
		   &WIRE_point[face[i].p4].ov);
	}

	OS_buffer_draw(ob, NULL, NULL, OS_DRAW_TRANSPARENT | OS_DRAW_ZALWAYS);
}



void WIRE_init()
{
	SLONG i;

	static SLONG done;

	if (!done)
	{
		/*
		WIRE_mesh[0] = IMP_load("Meshes\\coinlogo.sex", 0.80F);
		WIRE_mesh[1] = IMP_load("Meshes\\tallpot.sex",  0.25F);
		WIRE_mesh[2] = IMP_load("Meshes\\capsule.sex",  0.25F);
		*/

		WIRE_mesh[0] = IMP_binary_load("Meshes\\heart.imp");
		WIRE_mesh[1] = IMP_binary_load("Meshes\\coinlogo.imp");
		WIRE_mesh[2] = IMP_binary_load("Meshes\\tallpot.imp");
		WIRE_mesh[3] = IMP_binary_load("Meshes\\capsule.imp");

		//
		// Process each mesh.
		// 

		for (i = 0; i < WIRE_NUM_MESHES; i++)
		{
			//
			// Load the textures.
			//

			MF_load_textures(&WIRE_mesh[i]);

			//
			// So we can rotate it later...
			//

			MF_backup(&WIRE_mesh[i]);
		}

		//
		// The wireframe textures.
		//

		WIRE_ot_line = OS_texture_create("line.tga");
		WIRE_ot_dot  = OS_texture_create("dot.tga");

		done = TRUE;
	}

	//
	// The plane.
	//

	WIRE_plane_init(&WIRE_mesh[0]);

	WIRE_current_mesh = 0;
	WIRE_current_mode = 0;

	WIRE_now  = 0;
	WIRE_last = 0;
}






void WIRE_draw()
{
	IMP_Mesh *im = &WIRE_mesh[WIRE_current_mesh];

	SLONG mode_over;

	if (OS_ticks() < 7000)
	{
		//
		// Nothing for a while...
		//

//		return;
	}

	WIRE_plane_process();

	switch(WIRE_current_mode)
	{
		case WIRE_MODE_NONE_WIRE:
		case WIRE_MODE_WIRE_BRIGHT:
			mode_over = (WIRE_point[2].y < im->min_y - 5.0F);
			break;

		case WIRE_MODE_BRIGHT_TEXTURE:
			mode_over = (WIRE_point[2].y < im->min_y - 55.0F);
			break;

		case WIRE_MODE_TEXTURE_NONE:

			if (WIRE_point[2].y >= im->max_y + 5.0F)
			{
				if (WIRE_current_countdown == 0)
				{
					WIRE_current_countdown = OS_ticks();
				}
			}

			mode_over = WIRE_current_countdown && (OS_ticks() > WIRE_current_countdown + 4096);

			break;

		default:
			ASSERT(0);
			break;
	}

	if (mode_over)
	{
		//
		// Start another sweep.
		//

		WIRE_current_mode += 1;

		if (WIRE_current_mode > WIRE_MODE_TEXTURE_NONE)
		{
			//
			// Start a new mesh.
			//

			WIRE_current_mode  = 0;
			WIRE_current_mesh += 1;

			if (WIRE_current_mesh >= WIRE_NUM_MESHES)
			{
				WIRE_current_mesh = 0;
			}
		}
	
		if (WIRE_current_mode == WIRE_MODE_TEXTURE_NONE)
		{
			WIRE_current_countdown = 0;
		}
		else
		{
			WIRE_plane_init(im);
		}
	}

	if (KEY_on[KEY_P])
	{
		WIRE_plane_init(im);
	}

	//
	// Rotate the mesh and plane.
	//

	float angle = OS_ticks() * 0.001F;

	MF_rotate_mesh   (im, angle);
	WIRE_plane_rotate(im, angle);

	//
	// Work out where the light source is.
	//

	#define WIRE_LIGHT_YAW   ((KEY_on[KEY_J]) ? (130 * 180 / PI) : (190 * 180 / PI))
	#define WIRE_LIGHT_PITCH (-PI / 4)

	float light_x;
	float light_y;
	float light_z;
	float light_matrix[9];

	MATRIX_calc(
		light_matrix,
		WIRE_LIGHT_YAW,
		WIRE_LIGHT_PITCH,
		0.0F);

	light_x = 0.0F - light_matrix[6] * 512.0F;
	light_y = 0.0F - light_matrix[7] * 512.0F;
	light_z = 0.0F - light_matrix[8] * 512.0F;

	switch(WIRE_current_mode)
	{
		case WIRE_MODE_NONE_WIRE:
			WIRE_plane_draw();
			MF_transform_points(im);
			MF_add_wireframe(im, WIRE_ot_line, 0x114439, 0.002F, OS_DRAW_ADD | OS_DRAW_NOZWRITE);
			break;

		case WIRE_MODE_WIRE_BRIGHT:
			WIRE_plane_draw();
			MF_transform_points(im);
			MF_add_wireframe(im, WIRE_ot_line, 0x114439, 0.002F, OS_DRAW_ADD | OS_DRAW_NOZWRITE | OS_DRAW_ZREVERSE);
			MF_transform_points(im);
			MF_add_wireframe(im, WIRE_ot_line, 0x336611, 0.004F, OS_DRAW_ADD | OS_DRAW_NOZWRITE);
			break;

		case WIRE_MODE_BRIGHT_TEXTURE:

			WIRE_plane_draw();
			MF_transform_points(im);
			MF_add_wireframe(im, WIRE_ot_line, 0x336611, 0.004F, OS_DRAW_ADD | OS_DRAW_NOZWRITE | OS_DRAW_ZREVERSE);


			MF_diffuse_spotlight(im, light_x, light_y, light_z, light_matrix, 1.5F);
			MF_add_triangles_bumpmapped_pass(im, 0, OS_DRAW_NORMAL);
			MF_add_triangles_bumpmapped_pass(im, 1, OS_DRAW_ADD);
			MF_add_triangles_texture_after_bumpmap(im);
			MF_specular_spotlight(im, light_x, light_y, light_z, light_matrix, 1.5F);
			MF_add_triangles_specular_bumpmapped(im, WIRE_ot_dot);

			break;

		case WIRE_MODE_TEXTURE_NONE:

			{
				SLONG bright;
				ULONG colour;

				if (WIRE_current_countdown == 0)
				{
					bright = 255;
				}
				else
				{
					bright = OS_ticks() - WIRE_current_countdown >> 4;
					bright = 255 - bright;
				}

				SATURATE(bright, 0, 255);
				
				colour  = bright;
				colour |= (colour >> 3) << 16;

				WIRE_plane_draw();
				MF_transform_points(im);
				MF_add_wireframe(im, WIRE_ot_line, colour, 0.006F, OS_DRAW_ADD | OS_DRAW_NOZWRITE | OS_DRAW_ZREVERSE);



				MF_diffuse_spotlight(im, light_x, light_y, light_z, light_matrix, 1.5F);
				MF_add_triangles_bumpmapped_pass(im, 0, OS_DRAW_NORMAL);
				MF_add_triangles_bumpmapped_pass(im, 1, OS_DRAW_ADD);
				MF_add_triangles_texture_after_bumpmap(im);
				MF_specular_spotlight(im, light_x, light_y, light_z, light_matrix, 1.5F);
				MF_add_triangles_specular_bumpmapped(im, WIRE_ot_dot);
			}

			break;

		default:
			ASSERT(0);
	}
}

