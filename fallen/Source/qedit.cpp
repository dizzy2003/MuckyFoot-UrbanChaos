//
// An editor for the QMAP...
//

#ifndef TARGET_DC


#include "game.h"
#include <MFStdLib.h>
#include "qedit.h"
#include "qmap.h"
#include "fmatrix.h"
#include "fallen/ddengine/headers/qeng.h"



//
// The focus on the map.
//

SLONG QEDIT_focus_x;
SLONG QEDIT_focus_y;
SLONG QEDIT_focus_z;
SLONG QEDIT_cam_yaw;
SLONG QEDIT_cam_pitch;
SLONG QEDIT_cam_roll;
SLONG QEDIT_cam_zoom;
SLONG QEDIT_cam_x;
SLONG QEDIT_cam_y;
SLONG QEDIT_cam_z;
SLONG QEDIT_cam_matrix[9];


//
// Draws the QMAP around the given location in wire-frame.
//

void QEDIT_draw_wireframe(
		SLONG world_x,
		SLONG world_z)
{
	SLONG i;

	SLONG mx;
	SLONG mz;

	SLONG mx1;
	SLONG mz1;

	SLONG mx2;
	SLONG mz2;

	SLONG bx1;
	SLONG bz1;

	SLONG bx2;
	SLONG bz2;

	SLONG base;
	SLONG index;
	SLONG cube;

	SLONG x;
	SLONG z;

	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	QMAP_Map  *qm;
	QMAP_Cube *qc;

	mx = world_x >> 13;
	mz = world_z >> 13;

	mx1 = mx - 1;
	mz1 = mz - 1;

	mx2 = mx + 1;
	mz2 = mz + 1;

	SATURATE(mx1, 0, QMAP_MAPSIZE - 1);
	SATURATE(mz1, 0, QMAP_MAPSIZE - 1);

	SATURATE(mx2, 0, QMAP_MAPSIZE - 1);
	SATURATE(mz2, 0, QMAP_MAPSIZE - 1);

	//
	// Draw all the cubes.
	//

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		qm = &QMAP_map[mx][mz];

		base = QMAP_ALL_INDEX_CUBES(qm);

		for (i = 0; i < qm->num_cubes; i++)
		{
			index =  base + i;
			cube  =  QMAP_all[index];
			qc    = &QMAP_cube[cube];

			QMAP_get_cube_coords(
				 cube,
				&x1, &y1, &z1,
				&x2, &y2, &z2);

			x1 <<= 8;
			y1 <<= 8;
			z1 <<= 8;

			x2 <<= 8;
			y2 <<= 8;
			z2 <<= 8;

			#define QEDIT_CUBE_RADIUS 0x40
			#define QEDIT_CUBE_COLOUR 0x00880022

			QENG_world_line(
				x1, y1, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x2, y1, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x2, y1, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x2, y1, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x2, y1, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x1, y1, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x1, y1, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x1, y1, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);


			QENG_world_line(
				x1, y1, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x1, y2, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x2, y1, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x2, y2, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x1, y1, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x1, y2, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x2, y1, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x2, y2, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);


			QENG_world_line(
				x1, y2, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x2, y2, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x2, y2, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x2, y2, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x2, y2, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x1, y2, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);

			QENG_world_line(
				x1, y2, z2, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				x1, y2, z1, QEDIT_CUBE_RADIUS, QEDIT_CUBE_COLOUR,
				FALSE);
		}
	}

	//
	// Draw the ground as a grid.
	//

	bx1 = mx1 << 5;
	bz1 = mz1 << 5;

	bx2 = mx2 + 1 << 5;
	bz2 = mz2 + 1 << 5;

	for (x = bx1; x <= bx2; x++)
	for (z = bz1; z <  bz2; z++)
	{
		x1 = x << 8;
		x2 = x << 8;

		z1 = z + 0 << 8;
		z2 = z + 1 << 8;

		y1 = QMAP_calc_height_at(x1, z1);
		y2 = QMAP_calc_height_at(x2, z2);

		#define QEDIT_GROUND_RADIUS 0x20
		#define QEDIT_GROUND_COLOUR 0x0044444a

		QENG_world_line(
			x1, y1, z1, QEDIT_GROUND_RADIUS, (x & 0x1f) ? QEDIT_GROUND_COLOUR : QEDIT_GROUND_COLOUR * 2,
			x2, y2, z2, QEDIT_GROUND_RADIUS, (x & 0x1f) ? QEDIT_GROUND_COLOUR : QEDIT_GROUND_COLOUR * 2,
			FALSE);
	}

	for (z = bz1; z <= bz2; z++)
	for (x = bx1; x <  bx2; x++)
	{
		x1 = x + 0 << 8;
		x2 = x + 1 << 8;

		z1 = z << 8;
		z2 = z << 8;

		y1 = QMAP_calc_height_at(x1, z1);
		y2 = QMAP_calc_height_at(x2, z2);

		#define QEDIT_GROUND_RADIUS 0x20
		#define QEDIT_GROUND_COLOUR 0x0044444a

		QENG_world_line(
			x1, y1, z1, QEDIT_GROUND_RADIUS, (z & 0x1f) ? QEDIT_GROUND_COLOUR : QEDIT_GROUND_COLOUR * 2,
			x2, y2, z2, QEDIT_GROUND_RADIUS, (z & 0x1f) ? QEDIT_GROUND_COLOUR : QEDIT_GROUND_COLOUR * 2,
			FALSE);
	}
}

QMAP_Draw QEDIT_qmap_draw;

void QEDIT_draw_mapsquare(UBYTE map_x, UBYTE map_z)
{
	QMAP_create(&QEDIT_qmap_draw, map_x, map_z);
	QENG_draw(&QEDIT_qmap_draw);
	QMAP_free(&QEDIT_qmap_draw);
}

void QEDIT_init(void)
{
	QMAP_init();

	QMAP_add_cube(520, 520, 540, 526, 12);
	QMAP_add_cube(515, 503, 525, 522,  6);
	QMAP_add_cube(515, 505, 520, 515, 10);

	QEDIT_focus_x = 512 << 8;
	QEDIT_focus_z = 512 << 8;
	QEDIT_focus_y = QMAP_calc_height_at(QEDIT_focus_x, QEDIT_focus_y);

	QEDIT_cam_yaw   =  0;
	QEDIT_cam_pitch = -400;
	QEDIT_cam_roll  =  0;

	QEDIT_cam_zoom  = 0x4000;
}

void QEDIT_control(void)
{
	SLONG len_x;
	SLONG len_z;

	//
	// The camera matrix.
	//

	FMATRIX_calc(
		QEDIT_cam_matrix,
		QEDIT_cam_yaw,
		QEDIT_cam_pitch,
		QEDIT_cam_roll);

	#define QEDIT_SPEED_MOVE	((ShiftFlag) ? 0x400 :  0x80)
	#define QEDIT_SPEED_ROTATE	((ShiftFlag) ?    40 :    20)
	#define QEDIT_SPEED_ZOOM    ((ShiftFlag) ? 0x600 : 0x300)

	len_x = QDIST2(abs(QEDIT_cam_matrix[0]),abs(QEDIT_cam_matrix[2])) + 1;
	len_z = QDIST2(abs(QEDIT_cam_matrix[6]),abs(QEDIT_cam_matrix[8])) + 1;

	QEDIT_cam_matrix[0] = QEDIT_cam_matrix[0] * QEDIT_SPEED_MOVE / len_x;
	QEDIT_cam_matrix[2] = QEDIT_cam_matrix[2] * QEDIT_SPEED_MOVE / len_x;

	QEDIT_cam_matrix[6] = QEDIT_cam_matrix[6] * QEDIT_SPEED_MOVE / len_z;
	QEDIT_cam_matrix[8] = QEDIT_cam_matrix[8] * QEDIT_SPEED_MOVE / len_z;

	if (Keys[KB_UP   ]) {QEDIT_focus_x += QEDIT_cam_matrix[6]; QEDIT_focus_z += QEDIT_cam_matrix[8];}
	if (Keys[KB_DOWN ]) {QEDIT_focus_x -= QEDIT_cam_matrix[6]; QEDIT_focus_z -= QEDIT_cam_matrix[8];}

	if (Keys[KB_LEFT ]) {QEDIT_focus_x += QEDIT_cam_matrix[0]; QEDIT_focus_z += QEDIT_cam_matrix[2];}
	if (Keys[KB_RIGHT]) {QEDIT_focus_x -= QEDIT_cam_matrix[0]; QEDIT_focus_z -= QEDIT_cam_matrix[2];}

	if (Keys[KB_DEL ]) {QEDIT_cam_yaw -= QEDIT_SPEED_ROTATE;}
	if (Keys[KB_PGDN]) {QEDIT_cam_yaw += QEDIT_SPEED_ROTATE;}

	if (Keys[KB_INS ]) {QEDIT_cam_pitch -= QEDIT_SPEED_ROTATE;}
	if (Keys[KB_PGUP]) {QEDIT_cam_pitch += QEDIT_SPEED_ROTATE;}

	if (Keys[KB_HOME]) {QEDIT_cam_zoom -= QEDIT_SPEED_ZOOM;}
	if (Keys[KB_END] ) {QEDIT_cam_zoom += QEDIT_SPEED_ZOOM;}

	SATURATE(QEDIT_focus_x, 0, (QMAP_SIZE << 8) - 1);
	SATURATE(QEDIT_focus_z, 0, (QMAP_SIZE << 8) - 1);

	SATURATE(QEDIT_cam_zoom, 0x100, 0x7800);

	QEDIT_cam_yaw   &= 2047;
	QEDIT_cam_pitch &= 2047;

	//
	// Follow the contour of the landscape.
	//

	QEDIT_focus_y = QMAP_calc_height_at(QEDIT_focus_x, QEDIT_focus_z);

	//
	// Work out the position of the camera.
	//

	FMATRIX_calc(
		QEDIT_cam_matrix,
		QEDIT_cam_yaw,
		QEDIT_cam_pitch,
		QEDIT_cam_roll);

	QEDIT_cam_x = QEDIT_focus_x - (QEDIT_cam_matrix[6] * QEDIT_cam_zoom >> 16);
	QEDIT_cam_y = QEDIT_focus_y - (QEDIT_cam_matrix[7] * QEDIT_cam_zoom >> 16);
	QEDIT_cam_z = QEDIT_focus_z - (QEDIT_cam_matrix[8] * QEDIT_cam_zoom >> 16);
}

extern BOOL  text_fudge;
extern ULONG text_colour;
extern void  draw_centre_text_at(float x,float y,CBYTE *message,SLONG font_id);
extern void  draw_text_at(float x,float y,CBYTE *message,SLONG font_id);

void QEDIT_draw(void)
{
	float world_x = float(QEDIT_cam_x);
	float world_y = float(QEDIT_cam_y);
	float world_z = float(QEDIT_cam_z);

	float radians_yaw   = float(QEDIT_cam_yaw)   * (2.0f * PI / 2048.0F);
	float radians_pitch = float(QEDIT_cam_pitch) * (2.0f * PI / 2048.0F);
	float radians_roll  = float(QEDIT_cam_roll)  * (2.0f * PI / 2048.0F);

	QENG_clear_screen();

	QENG_set_camera(
		world_x,
		world_y,
		world_z,
		radians_yaw,
		radians_pitch,
		radians_roll);

	QEDIT_draw_wireframe(
		QEDIT_focus_x,
		QEDIT_focus_z);

	QEDIT_draw_mapsquare(
		QEDIT_focus_x >> 13,
		QEDIT_focus_z >> 13);

	text_fudge  = FALSE;
	text_colour = 0x00ffeeee;
	draw_text_at(20, 20, "Hello", 0);

	QENG_render();

	QENG_flip();
}



void QEDIT_loop()
{
	SLONG quit = FALSE;

	//
	// Open the display.
	//

	if (OpenDisplay(640,480,16,FLAGS_USE_3D|FLAGS_USE_WORKSCREEN) != 0)
	{
		return;
	}

	QENG_init();
	QEDIT_init();

	while(SHELL_ACTIVE && !quit)
	{
		QEDIT_control();
		QEDIT_draw();

		//
		// Control + Q quits.
		//

		if (Keys[KB_Q] && ControlFlag)
		{
			Keys[KB_Q] = 0;

			quit = TRUE;
		}
	}

	QENG_fini();

	//
	// Close the display.
	//

	CloseDisplay();
}




#endif //#indef TARGET_DC



