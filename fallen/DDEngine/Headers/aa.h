//
// An anti-aliased triangle draw.
//

#ifndef _AA_
#define _AA_


//
// The value generated for each pixel is the percentage area of
// that pixel covered by the triangle. This value is added with
// saturate to the value already in the pixel.
//
// The points must be given in clockwise order.
// 

void AA_draw(
		UBYTE *bitmap,
		UBYTE  x_res,
		UBYTE  y_res,
		SLONG  pitch,
		SLONG  p1x, SLONG p1y,	// 16-bit fixed point.
		SLONG  p2x, SLONG p2y,
		SLONG  p3x, SLONG p3y);


#endif
