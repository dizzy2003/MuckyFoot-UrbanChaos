//
// Sky.
//

#ifndef SKY_H
#define SKY_H


//
// Needs the screen to have been initialised.
// if 'star_file' is NULL, then stars are placed randomly, otherwise
// it should be an ascii file of the form...
//
//
// # These are comment lines
// # The values are Yaw, Pitch, Brightness
// # Yaw and Pitch are in degrees and brightness goes from 0 to 255.
// 
// Star: 230, 45, 205
// Star: 125, 70, 100
//

void SKY_init(CBYTE *star_file);



// ========================================================
//
// THE SKY AT NIGHT
//
// ========================================================

//
// Draws the stars to a LOCKED screen!
// This should be done before drawing any polys.
//

void SKY_draw_stars(
		float world_camera_x,
		float world_camera_y,
		float world_camera_z,
		float max_dist);

void SKY_draw_poly_clouds(
		float world_camera_x,
		float world_camera_y,
		float world_camera_z,
		float max_dist);

void SKY_draw_poly_moon(
		float world_camera_x,
		float world_camera_y,
		float world_camera_z,
		float max_dist);

//
// Draws the reflection of the moon and returns its bounding box on screen.
// Returns FALSE if the moon was off-screen.
//

SLONG SKY_draw_moon_reflection(
		float  world_camera_x,
		float  world_camera_y,
		float  world_camera_z,
		float  max_dist,
		float *moon_screen_x1,
		float *moon_screen_y1,
		float *moon_screen_x2,
		float *moon_screen_y2);


// ========================================================
//
// THE SKY DURING THE DAY.
//
// ========================================================

void SKY_draw_poly_sky(
		float world_camera_x,
		float world_camera_y,
		float world_camera_z,
		float world_camera_yaw,
		float max_dist,
		ULONG bot_colour,
		ULONG top_colour);


#endif
