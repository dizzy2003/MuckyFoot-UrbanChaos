//
// Anti-alaised triangle draw mark 2
//

#ifndef _AB_
#define _AB_


//
// A clipped anti-aliased triangle draw, where each
// pixel is drawn additive.
//

void AB_draw(
		UBYTE *bitmap,
		SLONG  xres,	// Power of 2
		SLONG  yres,	// Power of 2
		SLONG  num_points,
		SLONG  px[],	// 8-bit fixed point
		SLONG  py[]);	// 8-bit fixed point








#endif
