//
// Movie compression.
//

#include <MFStdlib.h>
#include "comp.h"
#include "ic.h"
#include "tga.h"



//
// COMP_Delta data structures.  There are as many pan structures as you need
// followed by at least one COMP_Update structure.  If (last) then that
// COMP_Update is the last one.
//

typedef struct
{
	UBYTE num;
	UBYTE pan;

} COMP_Pan;

typedef struct
{
	UBYTE     x;
	UBYTE     y;
	UBYTE     num;
	UBYTE     last;	// TRUE => there are no more update structures.
	IC_Packet ip[];

} COMP_Update;


//
// Memory for a tga.
//

#ifndef TARGET_DC
TGA_Pixel COMP_tga_data[COMP_TGA_MAX_WIDTH * COMP_TGA_MAX_HEIGHT];
#endif
TGA_Info  COMP_tga_info;


//
// Returns the colour of the virtual pixel at the given location where
// x and y go from 0.0 to 1.0F
//

TGA_Pixel COMP_tga_colour(float x, float y)
{
	TGA_Pixel ans;
#ifdef TARGET_DC
	// This has been spoofed to save memory -
	// hopefully we won't need it.
	ASSERT(FALSE);
	return ans;
#else
	SLONG px;
	SLONG py;


	px = SLONG(float(COMP_tga_info.width)  * x);
	py = SLONG(float(COMP_tga_info.height) * y);

	SATURATE(px, 0, COMP_tga_info.width  - 1);
	SATURATE(py, 0, COMP_tga_info.height - 1);

	ans = COMP_tga_data[px + py * COMP_tga_info.width];

	return ans;

#endif
}



SLONG COMP_load(CBYTE *filename, COMP_Frame *cf)
{
#ifdef TARGET_DC
	// This has been spoofed to save memory -
	// hopefully we won't need it.
	ASSERT(FALSE);
	return TRUE;
#else

	SLONG px;
	SLONG py;

	SLONG dx;
	SLONG dy;

	SLONG r;
	SLONG g;
	SLONG b;

	TGA_Pixel tp;

	float x;
	float y;

	COMP_tga_info = TGA_load(
						filename,
						COMP_TGA_MAX_WIDTH,
						COMP_TGA_MAX_HEIGHT,
						COMP_tga_data,-1);

	if (!COMP_tga_info.valid)
	{
		return FALSE;
	}

	//
	// Sample the tga to fit into our frame.
	//

	#define COMP_SAMPLES_PER_PIXEL 4

	for (px = 0; px < COMP_SIZE; px++)
	for (py = 0; py < COMP_SIZE; py++)
	{
		r = 0;
		g = 0;
		b = 0;

		for (dx = 0; dx < COMP_SAMPLES_PER_PIXEL; dx++)
		for (dy = 0; dy < COMP_SAMPLES_PER_PIXEL; dy++)
		{
			x = float((px * COMP_SAMPLES_PER_PIXEL) + dx) * (1.0F / float(COMP_SIZE * COMP_SAMPLES_PER_PIXEL));
			y = float((py * COMP_SAMPLES_PER_PIXEL) + dy) * (1.0F / float(COMP_SIZE * COMP_SAMPLES_PER_PIXEL));

			tp = COMP_tga_colour(x,y);

			r += tp.red;
			g += tp.green;
			b += tp.blue;
		}

		r /= COMP_SAMPLES_PER_PIXEL * COMP_SAMPLES_PER_PIXEL;
		g /= COMP_SAMPLES_PER_PIXEL * COMP_SAMPLES_PER_PIXEL;
		b /= COMP_SAMPLES_PER_PIXEL * COMP_SAMPLES_PER_PIXEL;

		cf->p[py][px].red   = r;
		cf->p[py][px].green = g;
		cf->p[py][px].blue  = b;
	}

	return TRUE;
#endif
}



#ifndef TARGET_DC
//
// Returns the difference between the two squares. The square coordinates
// clamp to be inside the frame.
//

SLONG COMP_square_error(
		COMP_Frame *f1,
		SLONG       sx1,
		SLONG       sy1,
		COMP_Frame *f2,
		SLONG       sx2,
		SLONG       sy2,
		SLONG       size)
{
	SLONG dx;
	SLONG dy;

	SLONG x1;
	SLONG y1;

	SLONG x2;
	SLONG y2;

	TGA_Pixel *tp1;
	TGA_Pixel *tp2;

	SLONG error = 0;

	for (dx = 0; dx < size; dx++)
	for (dy = 0; dy < size; dy++)
	{
		x1 = sx1 + dx;
		y1 = sy1 + dy;

		x2 = sx2 + dx;
		y2 = sy2 + dy;

		SATURATE(x1, 0, COMP_SIZE - 1);
		SATURATE(y1, 0, COMP_SIZE - 1);

		SATURATE(x2, 0, COMP_SIZE - 1);
		SATURATE(y2, 0, COMP_SIZE - 1);

		tp1 = &f1->p[y1][x1];
		tp2 = &f2->p[y2][x2];

		error += abs(tp1->red   - tp2->red  );
		error += abs(tp1->green - tp2->green);
		error += abs(tp1->blue  - tp2->blue );
	}

	return error;
}

//
// Copies one square onto another.
//

void COMP_square_copy(
		COMP_Frame *f1,
		SLONG       sx1,
		SLONG       sy1,
		COMP_Frame *f2,
		SLONG       sx2,
		SLONG       sy2,
		SLONG       size)
{
	SLONG dx;
	SLONG dy;

	SLONG x1;
	SLONG y1;

	SLONG x2;
	SLONG y2;

	TGA_Pixel *tp1;
	TGA_Pixel *tp2;

	for (dx = 0; dx < size; dx++)
	for (dy = 0; dy < size; dy++)
	{
		x1 = sx1 + dx;
		y1 = sy1 + dy;

		x2 = sx2 + dx;
		y2 = sy2 + dy;

		SATURATE(x1, 0, COMP_SIZE - 1);
		SATURATE(y1, 0, COMP_SIZE - 1);

		SATURATE(x2, 0, COMP_SIZE - 1);
		SATURATE(y2, 0, COMP_SIZE - 1);

		tp1 = &f1->p[y1][x1];
		tp2 = &f2->p[y2][x2];

		*tp2 = *tp1;
	}
}


//
// The compression data.
//

#define COMP_MAX_DATA (1024 * 16)

struct
{
	SLONG size;
	UBYTE data[COMP_MAX_DATA];

} COMP_data;

COMP_Frame COMP_frame;

COMP_Delta *COMP_calc(COMP_Frame *f1, COMP_Frame *f2, COMP_Frame *ans)
{
	SLONG i;

	SLONG sx;
	SLONG sy;

	SLONG dx;
	SLONG dy;

	SLONG sx1;
	SLONG sy1;

	SLONG sx2;
	SLONG sy2;

	SLONG error;

	SLONG best_error;
	SLONG best_dx;
	SLONG best_dy;
	SLONG best_pan;

	UBYTE pan[COMP_SNUM * COMP_SNUM];
	SLONG pan_upto = 0;
	SLONG pan_index;

	UBYTE diff[COMP_SIZE * COMP_SIZE / 16];
	SLONG diff_upto = 0;

	COMP_Pan    *cp;
	COMP_Update *cu;
	IC_Packet   *ip;

	SLONG cu_valid;
	SLONG cu_num;

	//
	// Work out the best pan values to get from frame one to frame two.
	//

	#define COMP_MAX_PAN 5

	for (sx = 0; sx < COMP_SNUM; sx++)
	for (sy = 0; sy < COMP_SNUM; sy++)
	{
		sx2 = sx * COMP_SSIZE;
		sy2 = sy * COMP_SSIZE;

		best_error = INFINITY;
		best_dx    = 0;
		best_dy    = 0;

		for (dx = -COMP_MAX_PAN; dx <= +COMP_MAX_PAN; dx++)
		for (dy = -COMP_MAX_PAN; dy <= +COMP_MAX_PAN; dy++)
		{
			sx1 = sx2 + dx;
			sy1 = sy2 + dy;

			error = COMP_square_error(
						f1, sx1, sy1,
						f2, sx2, sy2,
						COMP_SSIZE);

			if (error < best_error)
			{
				best_error = error;
				best_dx    = dx;
				best_dy    = dy;

				if (best_error == 0)
				{
					//
					// We're not going to get any better than this!
					//

					goto found_best_pan;
				}
			}
		}

	  found_best_pan:;

		//
		// The maximum error per-pixel we can live with.
		// 

		#define COMP_MAX_ERROR_PER_PIXEL 32

		if (best_error < (COMP_SSIZE * COMP_SSIZE * COMP_MAX_ERROR_PER_PIXEL))
		{
			pan[pan_upto++] = (best_dx + COMP_MAX_PAN) | ((best_dy + COMP_MAX_PAN) << 4);
		}
		else
		{
			//
			// Mark as undefined.
			//

			pan[pan_upto++] = 255;
		}
	}

	//
	// RLE the pan values
	//

	cp = (COMP_Pan *) COMP_data.data;

	cp->num = 1;
	cp->pan = pan[0];
	
	for (i = 1; i < COMP_SNUM * COMP_SNUM; i++)
	{
		if (pan[i] == cp->pan && cp->num != 255)
		{
			cp->num += 1;
		}
		else
		{
			cp += 1;

			cp->num = 1;
			cp->pan = pan[i];
		}
	}

	//
	// Build the frame from just the pan info.
	// 

	pan_upto = 0;

	for (sx = 0; sx < COMP_SNUM; sx++)
	for (sy = 0; sy < COMP_SNUM; sy++)
	{
		sx2 = sx * COMP_SSIZE;
		sy2 = sy * COMP_SSIZE;

		if (pan[pan_upto] != 255)
		{
			dx = pan[pan_upto] & 0xf;
			dy = pan[pan_upto] >> 4;

			dx -= COMP_MAX_PAN;
			dy -= COMP_MAX_PAN;

			sx1 = sx2 + dx;
			sy1 = sy2 + dy;

			COMP_square_copy(
				f1,  sx1, sy1,
				ans, sx2, sy2,
				COMP_SSIZE);
		}

		pan_upto += 1;
	}

	//
	// Work out which packets we have to store in full.
	//

	cu       = (COMP_Update *) (cp + 1);
	ip       = (IC_Packet   *) (cp + 1);
	cu_valid = FALSE;
	cu_num   = 0;

	for (sx = 0; sx < COMP_SIZE; sx += 4)
	for (sy = 0; sy < COMP_SIZE; sy += 4)
	{
		pan_index = (sx / COMP_SSIZE) * COMP_SNUM + (sy / COMP_SSIZE);

		if (pan[pan_index] == 255)
		{
			//
			// Always update undefined pans.
			//

			error = INFINITY;
		}
		else
		{
			error = COMP_square_error(
						f1, sx, sy,
						f2, sx, sy,
						4);
		}

		if (error >= 16 * COMP_MAX_ERROR_PER_PIXEL)
		{
			cu_num += 1;

			if (cu_valid)
			{
				//
				// Add another packet.
				//

				*ip = IC_pack(
						(TGA_Pixel *) f2->p,
						COMP_SIZE,
						COMP_SIZE,
						sx,
						sy);

				cu->num += 1;

				if (cu->num == 255)
				{
					//
					// We have to start another run!
					//

					cu_valid = FALSE;
				}
			}
			else
			{
				//
				// Create a new packet.
				//

				cu = (COMP_Update *) ip;

				ASSERT((sx & 0x3) == 0);
				ASSERT((sy & 0x3) == 0);

				cu->x    = sx;
				cu->y    = sy;
				cu->num  = 1;
				cu->last = FALSE;
				ip       = cu->ip;

				*ip = IC_pack(
						(TGA_Pixel *) f2->p,
						COMP_SIZE,
						COMP_SIZE,
						sx,
						sy);

				cu_valid = TRUE;
			}

			//
			// Build the frame.
			//

			IC_unpack(
				*ip,
				(TGA_Pixel *) ans->p,
				COMP_SIZE,
				COMP_SIZE,
				sx,
				sy);
			
			ip += 1;
		}
		else
		{
			//
			// We'll have to start another RLE run next time we find a 
			// square to update.
			//

			cu_valid = FALSE;
		}
	}

	if (cu_num == 0)
	{
		//
		// Create a dud last packet.
		// 

		cu->x    = 0;
		cu->y    = 0;
		cu->num  = 0;
		cu->last = 1;
		ip       = cu->ip;
	}
	else
	{
		//
		// Mark the last packet as the last one.
		//

		cu->last = TRUE;
	}

	UBYTE *data_start = COMP_data.data;
	UBYTE *data_end   = (UBYTE *) ip;

	COMP_data.size = data_end - data_start;

	//
	// Done!
	//

	return (COMP_Delta *) &COMP_data;
}



void COMP_decomp(
		COMP_Frame *base,
		COMP_Delta *delta,
		COMP_Frame *result)
{
	SLONG i;
	SLONG sx;
	SLONG sy;
	SLONG dx;
	SLONG dy;

	COMP_Pan    *cp;
	COMP_Update *cu;
	IC_Packet   *ip;

	//
	// Use the pan info to copy over data.
	//

	cp = (COMP_Pan *) delta->data;

	sx = 0;
	sy = 0;

	while(1)
	{
		dx = cp->pan & 0xf;
		dy = cp->pan >> 4;

		dx -= COMP_MAX_PAN;
		dy -= COMP_MAX_PAN;

		for (i = 0; i < cp->num; i++)
		{
			if (cp->pan != 255)
			{
				COMP_square_copy(
					base,
					sx + dx,
					sy + dy,
					result,
					sx,
					sy,
					COMP_SSIZE);
			}

			sy += COMP_SSIZE;

			if (sy >= COMP_SIZE)
			{
				sy = 0;

				sx += COMP_SSIZE;

				if (sx >= COMP_SIZE)
				{
					//
					// Finished doing pan copying.
					//

					goto finished_panning;
				}
			}
		}

		cp += 1;
	}

  finished_panning:;

	//
	// Put in all the packets.
	//

	cu = (COMP_Update *) (cp + 1);

	while(1)
	{
		sx = cu->x;
		sy = cu->y;
		ip = cu->ip;

		ASSERT((sx & 0x3) == 0);
		ASSERT((sy & 0x3) == 0);

		for (i = 0; i < cu->num; i++)
		{
			ASSERT(WITHIN(sx, 0, COMP_SIZE - 4));
			ASSERT(WITHIN(sy, 0, COMP_SIZE - 4));

			IC_unpack(
				*ip,
				(TGA_Pixel *) result->p,
				COMP_SIZE,
				COMP_SIZE,
				sx,
				sy);

			sy += 4;

			if (sy >= COMP_SIZE)
			{
				sy = 0;

				sx += 4;
			}

			ip += 1;
		}

		if (cu->last)
		{
			break;
		}

		cu = (COMP_Update *) ip;
	}
}


#else //#ifndef TARGET_DC

// Spoof em, Danno.
void COMP_decomp(
		COMP_Frame *base,
		COMP_Delta *delta,
		COMP_Frame *result)
{
	ASSERT(FALSE);
}

COMP_Delta *COMP_calc(COMP_Frame *f1, COMP_Frame *f2, COMP_Frame *ans)
{
	ASSERT(FALSE);
	return ( NULL );
}

#endif //#else //#ifndef TARGET_DC

