//
// Image compression.
//

#include <MFStdLib.h>
#include "ic.h"
#include "tga.h"


//
// Converts 24-bit RGB into 5:6:5
//

ULONG IC_convert(UBYTE r, UBYTE g, UBYTE b)
{
	SLONG nr = r;
	SLONG ng = g;
	SLONG nb = b;
	
	ULONG ans;

	nr = (nr + 4) >> 3;
	ng = (ng + 2) >> 2;
	nb = (nb + 4) >> 3;

	if (nr > 31) {nr = 31;}
	if (ng > 63) {ng = 63;}
	if (nb > 31) {nb = 31;}

	ans = (nr << 11) | (ng << 5) | (nb);

	return ans;
}


IC_Packet IC_pack(
			TGA_Pixel *tga, 
			SLONG      tga_width,
			SLONG      tga_height,
			SLONG      px,
			SLONG      py)
{
	SLONG i;
	SLONG j;
	SLONG k;
	SLONG l;

	SLONG x;
	SLONG y;

	SLONG x1;
	SLONG y1;

	SLONG x2;
	SLONG y2;

	SLONG r1,g1,b1;
	SLONG r2,g2,b2;

	SLONG dr;
	SLONG dg;
	SLONG db;

	SLONG r[4];
	SLONG g[4];
	SLONG b[4];

	TGA_Pixel *tp;
	TGA_Pixel *tp1;
	TGA_Pixel *tp2;

	SLONG     error;
	ULONG     bit;
	SLONG     dist;
	SLONG     best_dist;
	SLONG     best_bit;
	SLONG     best_error = INFINITY;
	IC_Packet best_ans;

	//
	// Try lines between points i and j.
	//

	for (i = 1; i < 16; i++)
	for (j = 0; j <  i; j++)
	{
		if (i == j) {continue;}

		x1 = px + (i  & 0x3);
		y1 = py + (i >> 0x2);

		x2 = px + (j  & 0x3);
		y2 = py + (j >> 0x2);

		tp1 = &tga[x1 + y1 * tga_width];
		tp2 = &tga[x2 + y2 * tga_width];

		r1 = tp1->red;
		g1 = tp1->green;
		b1 = tp1->blue;

		r2 = tp2->red;
		g2 = tp2->green;
		b2 = tp2->blue;

		if (r1 == r2 &&
			g1 == g2 &&
			b1 == b2)
		{
			//
			// Not a valid line.
			//

			continue;
		}

		//
		// Work out the four colours we get from the line.
		//

		r[0] = r1;
		g[0] = g1;
		b[0] = b1;

		dr = r2 - r1;
		dg = g2 - g1;
		db = b2 - b1;

		r[1] = r1 + (dr *  85 >> 8);
		g[1] = g1 + (dg *  85 >> 8);
		b[1] = b1 + (db *  85 >> 8);

		r[2] = r1 + (dr * 170 >> 8);
		g[2] = g1 + (dg * 170 >> 8);
		b[2] = b1 + (db * 170 >> 8);

		r[3] = r2;
		g[3] = g2;
		b[3] = b2;

		//
		// Work out the score of the line- i.e. the amount of
		// error and the answer bits while we are at it.
		//

		bit   = 0;
		error = 0;

		for (k = 0; k < 16; k++)
		{
			x = px + (k  & 0x3);
			y = py + (k >> 0x2);

			tp = &tga[x + y * tga_width];

			best_dist = INFINITY;
			best_bit  = 0;

			for (l = 0; l < 4; l++)
			{
				dr = r[l] - tp->red;
				dg = g[l] - tp->green;
				db = b[l] - tp->blue;

				dist = dr * dr * 3 + dg * dg * 2 + db * db;

				if (dist < best_dist)
				{
					best_dist = dist;
					best_bit  = l;
				}
			}

			error += best_dist;
			bit  <<= 2;
			bit   |= best_bit;
		}

		if (error < best_error)
		{
			//
			// Found a better line.
			// 

			best_error       = error;
			best_ans.colour1 = IC_convert(r1,g1,b1);
			best_ans.colour2 = IC_convert(r2,g2,b2);
			best_ans.bit     = bit;
		}
	}

	if (best_error == INFINITY)
	{
		//
		// There are no lines! All the pixels must be the same colour.
		//

		tp = &tga[px + py * tga_width];

		best_ans.colour1 = IC_convert(tp->red,tp->green,tp->blue);
		best_ans.colour2 = IC_convert(tp->red,tp->green,tp->blue);
		best_ans.bit     = 0;
	}

	return best_ans;
}



void IC_unpack(
			IC_Packet  ip,
			TGA_Pixel *tga, 
			SLONG      tga_width,
			SLONG      tga_height,
			SLONG      px,
			SLONG      py)
{
	SLONG i;
	SLONG bit;

	SLONG r[4];
	SLONG g[4];
	SLONG b[4];

	TGA_Pixel *tp;
	
	r[0] = (((ip.colour1 >> 11)       ) << 3) + 4;
	g[0] = (((ip.colour1 >>  5) & 0x3f) << 2) + 2;
	b[0] = (((ip.colour1      ) & 0x1f) << 3) + 4;

	r[3] = (((ip.colour2 >> 11)       ) << 3) + 4;
	g[3] = (((ip.colour2 >>  5) & 0x3f) << 2) + 2;
	b[3] = (((ip.colour2      ) & 0x1f) << 3) + 4;

	SLONG dr = (r[3] - r[0]) * 85 >> 8;
	SLONG dg = (g[3] - g[0]) * 85 >> 8;
	SLONG db = (b[3] - b[0]) * 85 >> 8;

	r[1] = r[0] + dr;
	g[1] = g[0] + dg;
	b[1] = b[0] + db;

	r[2] = r[1] + dr;
	g[2] = g[1] + dg;
	b[2] = b[1] + db;

	for (i = 0; i < 4; i++)
	{
		tp = &tga[px + (py + i) * tga_width];

		// 
		// Unwind the loop a bit.
		//

		#define IC_DECOMPRESS_A_PIXEL()	\
		{								\
			bit      = ip.bit >> 30;	\
			ip.bit <<= 2;				\
										\
			tp->red   = r[bit];			\
			tp->green = g[bit];			\
			tp->blue  = b[bit];			\
										\
			tp += 1;					\
		}

		IC_DECOMPRESS_A_PIXEL();
		IC_DECOMPRESS_A_PIXEL();
		IC_DECOMPRESS_A_PIXEL();
		IC_DECOMPRESS_A_PIXEL();
	}	
}


#ifndef TARGET_DC
TGA_Pixel test[256 * 256];

void IC_test()
{
	SLONG x;
	SLONG y;

	TGA_Info  ti;
	IC_Packet ip;

	ti = TGA_load("test.tga", 256, 256, test, -1);

	if (!ti.valid)
	{
		return;
	}

	ASSERT((ti.width  & 0x3) == 0);
	ASSERT((ti.height & 0x3) == 0);

	for (x = 0; x < ti.width;  x += 4)
	for (y = 0; y < ti.height; y += 4)
	{
		ip = IC_pack(
				test,
				ti.width,
				ti.height,
				x,
				y);

		IC_unpack(
			ip,
			test,
			ti.width,
			ti.height,
			x,
			y);
	}

	TGA_save("testcomp.tga", ti.width, ti.height, test, FALSE);
}

#endif

