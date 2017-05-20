//
// An anti-aliased triangle draw.
//

#include <MFStdLib.h>

//
// The internal fixed point we used.
// 

#define AA_FIX 10
#define AA_PIX (1 << AA_FIX)
#define AA_PMN (AA_PIX - 1)


//
// The spans.
//

//#define AA_MAX_SPAN_X 256
//#define AA_MAX_SPAN_Y 256
#define AA_MAX_SPAN_X 33
#define AA_MAX_SPAN_Y 33

typedef struct
{
	SLONG rhs_min;
	SLONG rhs_max;

	SLONG lhs_min;
	SLONG lhs_max;

	SLONG pixel[AA_MAX_SPAN_X];

} AA_Span;

AA_Span AA_span[AA_MAX_SPAN_Y];



//
// A pixel on the rhs of a triangle filled in by 'frac' amount.
// frac is an AA_FIX-bit fraction.
//

void AA_pixel_rhs(
		SLONG x,
		SLONG y,
		SLONG frac)
{
	ASSERT(WITHIN(x, 0, AA_MAX_SPAN_X - 1));
	ASSERT(WITHIN(y, 0, AA_MAX_SPAN_Y - 1));

	frac >>= AA_FIX - 8;

	AA_Span *as = &AA_span[y];

	if (WITHIN(x, as->rhs_min, as->rhs_max))
	{
		as->pixel[x] += frac;
	}
	else
	{
		as->pixel[x] = frac;

		if (x < as->rhs_min) {as->rhs_min = x;}
		if (x > as->rhs_max) {as->rhs_max = x;}
	}
}

//
// A pixel on the lhs of a triangle where there is frac_left
// free space on the left of the pixel and frac_right filled
// space on the right of the pixel.
//

void AA_pixel_lhs(
		SLONG x,
		SLONG y,
		SLONG frac_left,
		SLONG frac_right)
{
	ASSERT(WITHIN(x, 0, AA_MAX_SPAN_X - 1));
	ASSERT(WITHIN(y, 0, AA_MAX_SPAN_Y - 1));

	frac_left  >>= AA_FIX - 8;
	frac_right >>= AA_FIX - 8;

	//ASSERT(WITHIN(frac_left,  0, 256));
	//ASSERT(WITHIN(frac_right, 0, 256));

	AA_Span *as = &AA_span[y];

	if (WITHIN(x, as->rhs_min, as->rhs_max))
	{
		//
		// Take a chunk out of this pixel.
		//

		as->pixel[x] -= frac_left;

		if (x < as->lhs_min) {as->lhs_min = x;}
		if (x > as->lhs_max) {as->lhs_max = x;}
	}
	else
	{
		if (WITHIN(x, as->lhs_min, as->lhs_max))
		{
			as->pixel[x] += frac_right;
		}
		else
		{
			as->pixel[x] = frac_right;

			if (x < as->lhs_min) {as->lhs_min = x;}
			if (x > as->lhs_max) {as->lhs_max = x;}
		}
	}
}


//
// An rhs span on one line. All values are in AA_FIX-bit fixed point.
//

void AA_span_rhs(
		SLONG dydx,
		SLONG x1,
		SLONG y1,
		SLONG x2,
		SLONG y2)
{
	SLONG dx;
	SLONG dy;

	SLONG xleft;
	SLONG xright;
	SLONG ytop;
	SLONG ybot;

	SLONG frac;

	if (x2 == x1)
	{
		//
		// Ignore dydx!
		//

		frac = (x1 & AA_PMN) * (y2 - y1) >> AA_FIX;	// Fraction of this pixel covered.

		AA_pixel_rhs(
			x1 >> AA_FIX,
			y1 >> AA_FIX,
			frac);

		return;
	}

	if (x2 > x1)
	{
		xleft = x1;
		ytop  = y1;

		while(1)
		{
			xright  =  xleft + AA_PIX;
			xright &= ~AA_PMN;

			if (xright > x2)
			{
				xright = x2;
			}

			dx   = xright - xleft;
			dy   = dx * dydx >> AA_FIX;
			ybot = ytop + dy;

			frac  = dx * dy			 >> (AA_FIX + 1);
			frac += dx * (y2 - ybot) >>  AA_FIX;

			if (xleft & AA_PMN)
			{
				frac += dy          * (xleft & AA_PMN) >>  AA_FIX;
				frac += (y2 - ybot) * (xleft & AA_PMN) >>  AA_FIX;
			}
		
			AA_pixel_rhs(
				xleft >> AA_FIX,
				y1    >> AA_FIX,
				frac);

			if (xright >= x2)
			{
				return;
			}

			xleft = xright;
			ytop  = ybot;
		}
	}
	else
	{
		xright = x1;
		ytop   = y1;

		while(1)
		{
			xleft  =  xright - 1;
			xleft &= ~AA_PMN;

			if (xleft < x2)
			{
				xleft = x2;
			}

			dx   =  xright - xleft;
			dy   = -dx * dydx >> AA_FIX;
			ybot =  ytop + dy;

			frac  = dx * dy                      >> (AA_FIX + 1);
			frac += dx * (ytop - y1)             >> AA_FIX;

			if (xleft & AA_PMN)
			{
				frac += dy          * (xleft & AA_PMN) >> AA_FIX;
				frac += (ytop - y1) * (xleft & AA_PMN) >> AA_FIX;
			}

			AA_pixel_rhs(
				xleft >> AA_FIX,
				y1    >> AA_FIX,
				frac);

			if (xleft <= x2)
			{
				return;
			}

			xright = xleft;
			ytop   = ybot;
		}
	}
}




//
// An lhs span on one line. All values are in AA_FIX-bit fixed point.
//

void AA_span_lhs(
		SLONG dydx,
		SLONG x1,
		SLONG y1,
		SLONG x2,
		SLONG y2)
{
	SLONG dx;
	SLONG dy;

	SLONG xleft;
	SLONG xright;
	SLONG xpixel;
	SLONG ytop;
	SLONG ybot;

	SLONG frac_left;
	SLONG frac_right;

	if (x2 == x1)
	{
		//
		// Ignore dydx!
		//

		frac_left  = (x1 & AA_PMN)          * (y2 - y1) >> AA_FIX;
		frac_right = (AA_PMN - (x1 & AA_PMN)) * (y2 - y1) >> AA_FIX;

		AA_pixel_lhs(
			x1 >> AA_FIX,
			y1 >> AA_FIX,
			frac_left,
			frac_right);

		return;
	}

	if (x2 > x1)
	{
		xleft = x1;
		ytop  = y1;

		while(1)
		{
			xright  =  xleft + AA_PIX;
			xright &= ~AA_PMN;
			xpixel  = xright;

			if (xright > x2)
			{
				xright = x2;
			}

			dx   = xright - xleft;
			dy   = dx * dydx >> AA_FIX;
			ybot = ytop + dy;

			frac_right  = dx * dy          >> (AA_FIX + 1);
			frac_right += dx * (ytop - y1) >> AA_FIX;

			if (xpixel - xright)
			{
				frac_right += dy          * (xpixel - xright) >> AA_FIX;
				frac_right += (ytop - y1) * (xpixel - xright) >> AA_FIX;
			}

			frac_left  = dx * dy          >> (AA_FIX + 1);
			frac_left += dx * (y2 - ybot) >> AA_FIX;

			if (xleft & AA_PMN)
			{
				frac_left += dy          * (xleft & AA_PMN)	>> AA_FIX;
				frac_left += (y2 - ybot) * (xleft & AA_PMN) >> AA_FIX;
			}
			
			AA_pixel_lhs(
				xleft >> AA_FIX,
				y1    >> AA_FIX,
				frac_left,
				frac_right);

			if (xright >= x2)
			{
				return;
			}

			xleft = xright;
			ytop  = ybot;
		}
	}
	else
	{
		xright = x1;
		ytop   = y1;

		while(1)
		{
			xleft  =  xright - 1;
			xleft &= ~AA_PMN;
			xpixel = xleft + AA_PIX;

			if (xleft < x2)
			{
				xleft = x2;
			}

			dx   = xright - xleft;
			dy   = dx * -dydx >> AA_FIX;
			ybot = ytop + dy;

			frac_right  = dx * dy          >> (AA_FIX + 1);
			frac_right += dx * (y2 - ybot) >> AA_FIX;

			if (xpixel - xright)
			{
				frac_right += dy          * (xpixel - xright) >> AA_FIX;
				frac_right += (y2 - ybot) * (xpixel - xright) >> AA_FIX;
			}

			frac_left  = dx * dy                      >> (AA_FIX + 1);
			frac_left += dx * (ytop - y1)             >> AA_FIX;
			frac_left += dy * (xleft & AA_PMN)          >> AA_FIX;
			frac_left += (ytop - y1) * (xleft & AA_PMN) >> AA_FIX;

			AA_pixel_lhs(
				xleft >> AA_FIX,
				y1    >> AA_FIX,
				frac_left,
				frac_right);

			if (xleft <= x2)
			{
				return;
			}

			xright = xleft;
			ytop   = ybot;
		}
	}
}




//
// A line on the rhs of the triangle. py1 <= py2
//

void AA_line_rhs(SLONG px1, SLONG py1, SLONG px2, SLONG py2)
{
	if (py1 == py2)
	{
		return;
	}

	SLONG dy;
	SLONG dx;

	SLONG dydx;
	SLONG dxdy;

	SLONG x1, x2;
	SLONG y1, y2;
	
	dx   =  px2 - px1;
	dy   =  py2 - py1;
	dxdy = (dx << AA_FIX) / dy;

	if (dx)
	{
		dydx  = dy << AA_FIX;
		dydx /= dx;
	}
	else
	{
		//
		// dydx will never be referenced... honest!
		//

		dydx = 0;
	}

	x1 = px1;
	y1 = py1;

	while(1)
	{
		y2  =  y1 + AA_PIX;
		y2 &= ~AA_PMN;

		if (y2 > py2)
		{
			y2 = py2;
		}

		dy = y2 - y1;

		if (dy == 0)
		{
			//
			// Hmm... do nowt.
			//
		}
		else
		{
			x2 = x1 + (dy * dxdy >> AA_FIX);

			AA_span_rhs(dydx, x1, y1, x2, y2);
		}

		if (y2 >= py2)
		{
			return;
		}

		x1 = x2;
		y1 = y2;
	}
}

//
// A line on the lhs of the triangle py1 <= py2
//

void AA_line_lhs(SLONG px1, SLONG py1, SLONG px2, SLONG py2)
{
	if (py1 == py2)
	{
		return;
	}

	SLONG dy;
	SLONG dx;

	SLONG dydx;
	SLONG dxdy;

	SLONG x1, x2;
	SLONG y1, y2;
	
	dx   =  px2 - px1;
	dy   =  py2 - py1;
	dxdy = (dx << AA_FIX) / dy;

	if (dx)
	{
		dydx  = dy << AA_FIX;
		dydx /= dx;
	}
	else
	{
		//
		// dydx will never be referenced... honest!
		//

		dydx = 0;
	}

	x1 = px1;
	y1 = py1;

	while(1)
	{
		y2  =  y1 + AA_PIX;
		y2 &= ~AA_PMN;

		if (y2 > py2)
		{
			y2 = py2;
		}

		dy = y2 - y1;

		if (dy == 0)
		{
			//
			// Hmm... do nowt.
			//
		}
		else
		{
			x2 = x1 + (dy * dxdy >> AA_FIX);

			AA_span_lhs(dydx, x1, y1, x2, y2);
		}

		if (y2 >= py2)
		{
			return;
		}

		y1 = y2;
		x1 = x2;
	}
}



void AA_draw_do(
		UBYTE *bitmap,
		UBYTE  x_res,
		UBYTE  y_res,
		SLONG  pitch,

		//
		// In clockwise order.
		//

		SLONG  px[3],
		SLONG  py[3])
{
	SLONG i;

	SLONG miny;
	SLONG maxy;

	//SLONG minp;
	SLONG p1;
	SLONG p2;

	SLONG x;
	SLONG y;

	SLONG x1, y1;
	SLONG x2, y2;

	SLONG nextp[3] = {1,2,0};

	SLONG val;

	SLONG  fill_top;
	SLONG  fill_bot;

	SLONG fill;
	UBYTE *line;

	//
	// The bounds of the triangle.
	//

	miny = py[0];
	maxy = py[0];

	if (py[1] < miny) {miny = py[1];}
	if (py[1] > maxy) {maxy = py[1];}

	if (py[2] < miny) {miny = py[2];}
	if (py[2] > maxy) {maxy = py[2];}

	fill_top = AA_PIX - (miny & AA_PMN);
	fill_bot = ((maxy - 1) & AA_PMN);

	miny >>= AA_FIX;
	maxy  -= 1;
	maxy >>= AA_FIX;

	//
	// Initialise these guys.
	//

	for (i = miny; i <= maxy; i++)
	{
		ASSERT(WITHIN(i, 0, AA_MAX_SPAN_Y - 1));

		AA_span[i].lhs_min = +INFINITY;
		AA_span[i].lhs_max = -INFINITY;

		AA_span[i].rhs_min = +INFINITY;
		AA_span[i].rhs_max = -INFINITY;
	}

	//
	// Go through each line.
	//

	p1 = 0;

	for (i = 0; i < 3; i++)
	{
		p2 = nextp[p1];

		x1 = px[p1];
		y1 = py[p1];

		x2 = px[p2];
		y2 = py[p2];

		if (y1 < y2)
		{
			//
			// Running along the RHS of the triangle.
			//

			AA_line_rhs(x1, y1, x2, y2);
		}
		else
		{
			//
			// Running along the LHS of the triangle.
			//

			AA_line_lhs(x2, y2, x1, y1);
		}

		p1 = p2;
	}

	//
	// Draw the bitmap from the info in the spans.
	//

	fill = fill_top;

	for (y = miny; y <= maxy; y++)
	{
		if (y == maxy)
		{
			fill = fill_bot;
		}

		ASSERT(WITHIN(y, 0, AA_MAX_SPAN_Y - 1));

		AA_Span *as = &AA_span[y];

		line = &bitmap[y * pitch];

		x = as->lhs_min;

		while(x <= as->lhs_max)
		{
			ASSERT(WITHIN(x, 0, AA_MAX_SPAN_X - 1));


			val  = line[x];
			val += as->pixel[x];

			SATURATE(val, 0, 255);
			
			line[x] = (UBYTE)val;

			x += 1;
		}

		while (x < as->rhs_min)
		{
			ASSERT(WITHIN(x, 0, x_res - 1));

			val  = line[x];
			val += fill;

			SATURATE(val, 0, 255);

			line[x] = (UBYTE)val;

			x += 1;
		}

		while (x <= as->rhs_max)
		{
			ASSERT(WITHIN(x, 0, AA_MAX_SPAN_X - 1));

			val  = line[x];
			val += as->pixel[x];

			SATURATE(val, 0, 255);
			
			line[x] = (UBYTE)val;

			x += 1;
		}

		fill = 255;
	}

}





//
// The value generate for 
// 

void AA_draw(
		UBYTE *bitmap,
		UBYTE  x_res,
		UBYTE  y_res,
		SLONG  pitch,
		SLONG  p1x, SLONG p1y,	// 16-bit fixed point.
		SLONG  p2x, SLONG p2y,
		SLONG  p3x, SLONG p3y)
{
#ifndef NDEBUG
	SLONG i;
#endif

	//SLONG miny;
	//SLONG maxy;

	SLONG minp;

	SLONG p1;
	SLONG p2;
	SLONG p3;

	//SLONG x;
	//SLONG y;

	SLONG px[3];
	SLONG py[3];

	//SLONG val;

	//SLONG  fill_top;
	//SLONG  fill_bot;

	//UBYTE  fill;
	//UBYTE *line;

	SLONG nextp[3] = {1,2,0};

	//
	// Work in AA_FIX-bit internally.
	//

	px[0] = p1x >> (16 - AA_FIX);
	py[0] = p1y >> (16 - AA_FIX);

	px[1] = p2x >> (16 - AA_FIX);
	py[1] = p2y >> (16 - AA_FIX);

	px[2] = p3x >> (16 - AA_FIX);
	py[2] = p3y >> (16 - AA_FIX);

	//
	// Lose the bottom few bits to preserve accuracy in
	// the triangle draws...
	//

	px[0] &= ~0xf;
	py[0] &= ~0xf;

	px[1] &= ~0xf;
	py[1] &= ~0xf;

	px[2] &= ~0xf;
	py[2] &= ~0xf;

	#ifndef NDEBUG

	for (i = 0; i < 3; i++)
	{
		ASSERT(WITHIN(px[i], 0, AA_MAX_SPAN_X << AA_FIX));
		ASSERT(WITHIN(py[i], 0, AA_MAX_SPAN_Y << AA_FIX));
	}

	#endif

	//
	// The maximum span
	// 

	if (px[0] == AA_MAX_SPAN_X << AA_FIX) {px[0] -= 1;}
	if (px[1] == AA_MAX_SPAN_X << AA_FIX) {px[1] -= 1;}
	if (px[2] == AA_MAX_SPAN_X << AA_FIX) {px[2] -= 1;}

	if (py[0] == AA_MAX_SPAN_Y << AA_FIX) {py[0] -= 1;}
	if (py[1] == AA_MAX_SPAN_Y << AA_FIX) {py[1] -= 1;}
	if (py[2] == AA_MAX_SPAN_Y << AA_FIX) {py[2] -= 1;}

	//
	// What is the top point?
	//

	minp = 0;

	if (py[1] < py[   0]) {minp = 1;}
	if (py[2] < py[minp]) {minp = 2;}

	p1 = minp;
	p2 = nextp[p1];
	p3 = nextp[p2];

	if (py[0] == py[1] ||
		py[1] == py[2] ||
		py[2] == py[0])
	{
		//
		// This triangle is already flat along one edge!
		//

		SLONG tx[3];
		SLONG ty[3];

		tx[0] = px[p1];
		ty[0] = py[p1];

		tx[1] = px[p2];
		ty[1] = py[p2];

		tx[2] = px[p3];
		ty[2] = py[p3];

		AA_draw_do(
			bitmap,
			x_res,
			y_res,
			pitch,
			tx,
			ty);
	}
	else
	{
		if (py[p2] > py[p3])
		{
			//
			// Pointy left triangle. Find the new point along the edge from p1 to p2.
			//

			SLONG dx = px[p2] - px[p1];
			SLONG dy = py[p2] - py[p1];

			SLONG nx;
			SLONG ny;

			nx = px[p1] + (dx * (py[p3] - py[p1])) / dy;
			ny = py[p3];

			//
			// Two new triangles.
			//

			SLONG tx[3];
			SLONG ty[3];

			tx[0] = px[p1];
			ty[0] = py[p1];

			tx[1] = nx;
			ty[1] = ny;

			tx[2] = px[p3];
			ty[2] = py[p3];

			AA_draw_do(
				bitmap,
				x_res,
				y_res,
				pitch,
				tx,
				ty);


			tx[0] = px[p3];
			ty[0] = py[p3];

			tx[1] = nx;
			ty[1] = ny;

			tx[2] = px[p2];
			ty[2] = py[p2];

			AA_draw_do(
				bitmap,
				x_res,
				y_res,
				pitch,
				tx,
				ty);
		}
		else
		{
			//
			// Pointy right triangle. Find the new point along the edge from p1 to p3.
			//

			SLONG dx = px[p3] - px[p1];
			SLONG dy = py[p3] - py[p1];

			SLONG nx;
			SLONG ny;

			nx = px[p1] + (dx * (py[p2] - py[p1])) / dy;
			ny = py[p2];

			//
			// Two new triangles.
			//

			SLONG tx[3];
			SLONG ty[3];

			tx[0] = px[p1];
			ty[0] = py[p1];

			tx[1] = px[p2];
			ty[1] = py[p2];

			tx[2] = nx;
			ty[2] = ny;

			AA_draw_do(
				bitmap,
				x_res,
				y_res,
				pitch,
				tx,
				ty);

			tx[0] = nx;
			ty[0] = ny;

			tx[1] = px[p2];
			ty[1] = py[p2];

			tx[2] = px[p3];
			ty[2] = py[p3];

			AA_draw_do(
				bitmap,
				x_res,
				y_res,
				pitch,
				tx,
				ty);
		}
	}
}







