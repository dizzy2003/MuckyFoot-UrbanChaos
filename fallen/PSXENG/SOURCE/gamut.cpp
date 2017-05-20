		//
// The gamut.
//

#include <MFStdLib.h>
#include "c:\fallen\psxeng\headers\Gamut.h"


NGAMUT_Gamut	NGAMUT_gamut[NGAMUT_SIZE];
NGAMUT_Gamut2	NGAMUT_gamut2[NGAMUT_SIZE];
SLONG			NGAMUT_zmin;
SLONG			NGAMUT_zmax;
SLONG			NGAMUT_Ymin,NGAMUT_Ymax;
SLONG			NGAMUT_xmin,NGAMUT_xmax;

//
// Initialises the gamut.
//

void NGAMUT_init(void)
{
	SLONG i;

	//
	// This could be the first time we have called NGAMUT_init.
	// We have to initialise the whole array.
	//

	for (i = 0; i < NGAMUT_SIZE; i++)
	{
		NGAMUT_gamut[i].xmin =  255;//INFINITY;
		NGAMUT_gamut[i].xmax = 0;//-INFINITY;

		NGAMUT_gamut2[i].zmin =  255;//INFINITY;
		NGAMUT_gamut2[i].zmax = 0;//-INFINITY;
	}

	NGAMUT_zmin =  INFINITY;
	NGAMUT_zmax = -INFINITY;
//	NGAMUT_Ymin =  INFINITY;
//	NGAMUT_Ymax = -INFINITY;
	NGAMUT_xmin =  INFINITY;
	NGAMUT_xmax = -INFINITY;
}

//
// Pushes out the gamut so it includes the given square.
//

inline void NGAMUT_add_square(SLONG x, SLONG z)
{
	if (!WITHIN(z, 0, NGAMUT_SIZE - 2))
	{
		//
		// The zline is off the map.
		//

		return;
	}

	//
	// Make sure that the x-coord is in range.
	//

	SATURATE(x, 0, NGAMUT_SIZE - 2);

	//
	// Push out the gamut.
	//
/*
	if (z < NGAMUT_zmin) {NGAMUT_zmin = z;}
	if (z > NGAMUT_zmax) {NGAMUT_zmax = z;}
*/

	if (x < NGAMUT_gamut[z].xmin) {NGAMUT_gamut[z].xmin = x;}
	if (x > NGAMUT_gamut[z].xmax) {NGAMUT_gamut[z].xmax = x;}


}

inline void NGAMUT_add_square_z(SLONG x, SLONG z)
{
	if (!WITHIN(x, 0, NGAMUT_SIZE - 2))
	{
		//
		// The zline is off the map.
		//

		return;
	}

	//
	// Make sure that the x-coord is in range.
	//

	SATURATE(z, 0, NGAMUT_SIZE - 2);

	//
	// Push out the gamut.
	//
	if (z < NGAMUT_gamut2[x].zmin) {NGAMUT_gamut2[x].zmin = z;}
	if (z > NGAMUT_gamut2[x].zmax) {NGAMUT_gamut2[x].zmax = z;}
}

//
// Pushes out the gamut so it includes the given line.
//

void NGAMUT_add_line(SLONG p1x, SLONG p1z, SLONG p2x, SLONG p2z)
{
	SLONG x;
	SLONG z;

	SLONG m1x;
	SLONG m1z;
	
	SLONG m2x;
	SLONG m2z;

	SLONG xfrac;
	SLONG zfrac;

	SLONG dx;
	SLONG dz;
	SLONG dxdz;

	//
	// Sort the points by z.
	//

	if(p1x<NGAMUT_xmin)
		NGAMUT_xmin=MAX(0,p1x);

	if(p2x>NGAMUT_xmax)
		NGAMUT_xmax=MIN(126,p2x);

	if(p1x>NGAMUT_xmax)
		NGAMUT_xmax=MIN(126,p1x);

	if(p2x<NGAMUT_xmin)
		NGAMUT_xmin=MAX(0,p2x);

	if(p1z<NGAMUT_zmin)
		NGAMUT_zmin=MAX(0,p1z);

	if(p2z>NGAMUT_zmax)
		NGAMUT_zmax=MIN(126,p2z);

	if(p1z>NGAMUT_zmax)
		NGAMUT_zmax=MIN(126,p1z);

	if(p2z<NGAMUT_zmin)
		NGAMUT_zmin=MAX(0,p2z);



	if (p2z < p1z)
	{
		SWAP(p1x, p2x);
		SWAP(p1z, p2z);
	}

	//
	// Add the end points of the line.
	//

	m1x = SLONG(p1x);
	m1z = SLONG(p1z);

	NGAMUT_add_square(m1x, m1z);
	NGAMUT_add_square_z(m1x, m1z);

	m2x = SLONG(p2x);
	m2z = SLONG(p2z);

	NGAMUT_add_square(m2x, m2z);
	NGAMUT_add_square_z(m2x, m2z);

	//
	// Go along the line.
	//

	dx = p2x - p1x;
	dz = p2z - p1z;

	if (dz == 0)
	{

		//
		// No z to traverse.
		//
	}
	else
	{
		dxdz = (dx<<10) / dz;

		//
		// Move down to the next zline.
		//

		zfrac = 1 - (p1z - m1z);
		xfrac = zfrac * dxdz;

		x = (p1x<<10) + xfrac;
		z = m1z + 1;

		while(z <= m2z)
		{
			NGAMUT_add_square(SLONG(x>>10), SLONG(z    ));
			NGAMUT_add_square(SLONG(x>>10), SLONG(z - 1));

			x += dxdz;
			z += 1;
		}
	}



	//
	// added by MikeD for stepping along x
	//
		

	if (p2x < p1x)
	{
		SWAP(p1x, p2x);
		SWAP(p1z, p2z);
	}

	//
	// Add the end points of the line.
	//

	m1x = SLONG(p1x);
	m1z = SLONG(p1z);


	m2x = SLONG(p2x);
	m2z = SLONG(p2z);


	//
	// Go along the line.
	//

	dx = p2x - p1x;
	dz = p2z - p1z;

	if (dx == 0)
	{

		//
		// No x to traverse.
		//
	}
	else
	{
		dxdz = (dz<<10) / dx;

		//
		// Move down to the next zline.
		//

		xfrac = 1 - (p1x - m1x);
		zfrac = xfrac * dxdz;

		z = (p1z<<10) + zfrac;
		x = m1x + 1;

		while(x <= m2x)
		{
			NGAMUT_add_square_z(SLONG(x), SLONG(z>>10    ));
			NGAMUT_add_square_z(SLONG(x-1), SLONG(z>>10));

			z += dxdz;
			x += 1;
		}
	}


}

#ifdef	POO
void NGAMUT_view_square(SLONG mid_x, SLONG mid_z, SLONG radius)
{
	SLONG i;
	SLONG z;

	SLONG zmin;
	SLONG zmax;

	SLONG xmin;
	SLONG xmax;

	zmin = SLONG(mid_z - radius);
	zmax = SLONG(mid_z + radius);

	xmin = SLONG(mid_x - radius);
	xmax = SLONG(mid_x + radius);

	SATURATE(zmin, 0, NGAMUT_SIZE - 1);
	SATURATE(zmax, 0, NGAMUT_SIZE - 1);

	SATURATE(xmin, 0, NGAMUT_SIZE - 1);
	SATURATE(xmax, 0, NGAMUT_SIZE - 1);

	NGAMUT_zmin = zmin;
	NGAMUT_zmax = zmax;

	if(xmin<NGAMUT_xmin)
		NGAMUT_xmin=xmin;

	if(xmin>NGAMUT_xmax)
		NGAMUT_xmax=xmin;

	if(xmax<NGAMUT_xmin)
		NGAMUT_xmin=xmax;

	if(xmax>NGAMUT_xmax)
		NGAMUT_xmax=xmax;


	//
	// Initialise the rest of the gamut...
	//

	for (i = 0; i < NGAMUT_SIZE; i++)
	{
		NGAMUT_gamut[i].xmin =  255;//INFINITY;
		NGAMUT_gamut[i].xmax = 0;//-INFINITY;
	}

	//
	// Mark the square of the gamut.
	//

	for (z = zmin; z <= zmax; z++)
	{
		NGAMUT_gamut[z].xmin = xmin;
		NGAMUT_gamut[z].xmax = xmax;
	}
}
#endif

NGAMUT_Gamut NGAMUT_point_gamut[NGAMUT_SIZE];
SLONG        NGAMUT_point_zmin;
SLONG        NGAMUT_point_zmax;

void NGAMUT_calculate_point_gamut(void)
{

	SLONG i;
	SLONG	temp;
	NGAMUT_point_zmin = NGAMUT_zmin;
	NGAMUT_point_zmax = NGAMUT_zmax + 1;

	for (i = NGAMUT_zmin + 1; i <= NGAMUT_zmax; i++)
	{
		NGAMUT_point_gamut[i].xmin = MIN(NGAMUT_gamut[i].xmin,     NGAMUT_gamut[i - 1].xmin);
		temp=MAX(NGAMUT_gamut[i].xmax + 1, NGAMUT_gamut[i - 1].xmax + 1);
		SATURATE(temp,0,255);
		NGAMUT_point_gamut[i].xmax = temp;
	}

	NGAMUT_point_gamut[NGAMUT_point_zmin].xmin = NGAMUT_gamut[NGAMUT_zmin].xmin;

	temp= NGAMUT_gamut[NGAMUT_zmin].xmax + 1;
	SATURATE(temp,0,255);
	NGAMUT_point_gamut[NGAMUT_point_zmin].xmax = temp;


	NGAMUT_point_gamut[NGAMUT_point_zmax].xmin = NGAMUT_gamut[NGAMUT_zmax].xmin;

	temp = NGAMUT_gamut[NGAMUT_zmax].xmax + 1;
	SATURATE(temp,0,255);
	NGAMUT_point_gamut[NGAMUT_point_zmax].xmax = temp;



	//
	//do gamut2
	//
	for (i = NGAMUT_xmax ; i > NGAMUT_xmin; i--)
	{
		NGAMUT_gamut2[i].zmin = MIN(NGAMUT_gamut2[i].zmin,     NGAMUT_gamut2[i - 1].zmin);
		NGAMUT_gamut2[i].zmax = MAX(NGAMUT_gamut2[i].zmax + 1, NGAMUT_gamut2[i - 1].zmax + 1);
	}


	temp   = NGAMUT_gamut2[NGAMUT_zmin].zmin - 1;
	SATURATE(temp,0,255);
	NGAMUT_gamut2[NGAMUT_zmin].zmin   = temp;

	temp   = NGAMUT_gamut2[NGAMUT_zmin].zmax + 1;
	SATURATE(temp,0,255);
	NGAMUT_gamut2[NGAMUT_zmin].zmax   = temp;


	temp = NGAMUT_gamut2[NGAMUT_zmax].zmin - 1;
	SATURATE(temp,0,255);
	NGAMUT_gamut2[NGAMUT_zmax+1].zmin = temp;

	temp = NGAMUT_gamut2[NGAMUT_zmax].zmax + 1;
	SATURATE(temp,0,255);
	NGAMUT_gamut2[NGAMUT_zmax+1].zmax = temp;

}

NGAMUT_Gamut NGAMUT_lo_gamut[NGAMUT_SIZE_LO];
SLONG        NGAMUT_lo_zmin;
SLONG        NGAMUT_lo_zmax;

void NGAMUT_calculate_lo_gamut()
{
	SLONG i;
	SLONG j;

	SLONG z;
	
	SLONG xmin;
	SLONG xmax;

	SLONG out_xmin;
	SLONG out_xmax;

	NGAMUT_lo_zmin = (NGAMUT_point_zmin - 1)>>2; // / (NGAMUT_SIZE / NGAMUT_SIZE_LO);
	NGAMUT_lo_zmax = (NGAMUT_point_zmax + 1)>>2; // / (NGAMUT_SIZE / NGAMUT_SIZE_LO);

	SATURATE(NGAMUT_lo_zmin, 0, NGAMUT_SIZE_LO - 1);
	SATURATE(NGAMUT_lo_zmax, 0, NGAMUT_SIZE_LO - 1);

	for (i = NGAMUT_lo_zmin; i <= NGAMUT_lo_zmax; i++)
	{
		xmin = +INFINITY;
		xmax = -INFINITY;

		for (j = 0; j < (NGAMUT_SIZE / NGAMUT_SIZE_LO); j++)
		{
			z  = i<<2; // * (NGAMUT_SIZE / NGAMUT_SIZE_LO);
			z += j;

			if (WITHIN(z, NGAMUT_point_zmin, NGAMUT_point_zmax))
			{
				out_xmin = (NGAMUT_point_gamut[z].xmin - 1)>>2; // / (NGAMUT_SIZE / NGAMUT_SIZE_LO);
				out_xmax = (NGAMUT_point_gamut[z].xmax + 1)>>2; // / (NGAMUT_SIZE / NGAMUT_SIZE_LO);

				if (out_xmin < xmin) {xmin = out_xmin;}
				if (out_xmax > xmax) {xmax = out_xmax;}
			}
		}

		SATURATE(xmin, 0, NGAMUT_SIZE_LO - 1);
		SATURATE(xmax, 0, NGAMUT_SIZE_LO - 1);

		NGAMUT_lo_gamut[i].xmin = xmin;
		NGAMUT_lo_gamut[i].xmax = xmax;
	}
}





