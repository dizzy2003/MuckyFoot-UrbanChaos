//
// The landscape- its made up of lots of lines.
//

#ifndef _LAND_
#define _LAND_




//
// Initialises the landscape to nothing.
//

void LAND_init(void);



//
// Adds a line to the landscape. It makes a different whether the lines
// are given in clockwise or anticlockwise order.
//

void LAND_add_line(float x1, float y1, float x2, float y2);
void LAND_add_line(SLONG num_points, Point2d p[]);

//
// Calculates the normals for the land. You must call this function before
// the collision routines work.
//

void LAND_calc_normals(void);


//
// Checks to see if the given circle has collided with the landscape.
// If it the circle does collide, then if gives back the normal of the
// point in the landscape where the the circle touched and returns TRUE.
//

SLONG LAND_collide_sphere(
		float  x,
		float  y,
		float  radius,
		float *nx,
		float *ny);


//
// Draws the landscape centred at (x,y) with the given zoom factor.
//

void LAND_draw_all(float x, float y, float zoom);



#endif

