//
// Simple ovals underneath people, barrels etc...
//

#include "game.h"
#include "ddlib.h"
#include "mav.h"
#include "poly.h"
#include "oval.h"

#include <math.h>


//
// The current oval.
//

float OVAL_mid_x;
float OVAL_mid_y;
float OVAL_mid_z;
float OVAL_dudx;
float OVAL_dvdx;
float OVAL_dudz;
float OVAL_dvdz;


//
// Returns the oval (u,v) at the given position.
//

void OVAL_get_uv(
		float  world_x,
		float  world_z,
		float *u,
		float *v)
{
	float dx;
	float dz;

	dx = world_x - OVAL_mid_x;
	dz = world_z - OVAL_mid_z;

	float ans_u;
	float ans_v;

	ans_u = 0.5F;
	ans_v = 0.5F;

	ans_u += dx * OVAL_dudx;
	ans_u += dz * OVAL_dudz;

	ans_v += dx * OVAL_dvdx;
	ans_v += dz * OVAL_dvdz;

   *u = ans_u;
   *v = ans_v;
}


//
// Projects the oval onto the given mapsquare and adds it
// to the POLY stuff.
//

void OVAL_project_onto_mapsquare(UBYTE map_x, UBYTE map_z, SLONG page)
{
	SLONG i;

	PAP_Hi *ph = &PAP_2HI(map_x,map_z);

	if (!WITHIN(map_x, 1, 126) ||
		!WITHIN(map_z, 1, 126))
	{
		//
		// Out of bounds...
		//

		return;
	}

	//
	// Work out the height at the four corners of this square.
	//

	float world_y[4];

	if (ph->Flags & PAP_FLAG_ROOF_EXISTS)
	{
		world_y[0] = 
		world_y[1] = 
		world_y[2] = 
		world_y[3] = MAVHEIGHT(map_x,map_z) << 6;
	}
	else
	if (ph->Flags & PAP_FLAG_HIDDEN)
	{	
		//
		// Damn... this'll be annoying!
		//

		return;
	}
	else
	{
		world_y[0] = ph[0              ].Alt << PAP_ALT_SHIFT;
		world_y[1] = ph[0 + PAP_SIZE_HI].Alt << PAP_ALT_SHIFT;
		world_y[2] = ph[1              ].Alt << PAP_ALT_SHIFT;
		world_y[3] = ph[1 + PAP_SIZE_HI].Alt << PAP_ALT_SHIFT;

		if (ph->Flags & PAP_FLAG_SINK_SQUARE)
		{
			world_y[0] -= 32.0F;
			world_y[1] -= 32.0F;
			world_y[2] -= 32.0F;
			world_y[3] -= 32.0F;
		}
	}

	//
	// The average y coordinate...
	//

	float av_y;

	av_y  = world_y[0];
	av_y += world_y[1];
	av_y += world_y[2];
	av_y += world_y[3];

	av_y *= 0.25F;

	//
	// How dark we make the shadow.
	//

	float dy = OVAL_mid_y - av_y;

	if (dy < -48.0F)
	{
		//
		// This oval is above the person causing the shadow!
		//

		return;
	}

	float dark;

	if (dy < 128.0F)
	{
		dark = 1.0F;	// Maximum darkness...
	}
	else
	{
		dark = 1.0F - (dy - 128) * (1.0F / 128.0F);
	}

	if (dark <= 0.0F)
	{
		//
		// Too faded out.
		// 

		return;
	}

	//
	// Convert the darkness into a colour.
	//

	SLONG colour;
	
	colour  = ftol(dark * 128.0F);
	colour |= colour << 8;
	colour |= colour << 16;

	//
	// Create the quad.
	//

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	// The old order, that doesn't match the landscape.
	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	for (i = 0; i < 4; i++)
	{
		POLY_transform(
			float(map_x + (i &  1) << 8),
			world_y[i],
			float(map_z + (i >> 1) << 8),
			quad[i],
			TRUE);

#ifdef TARGET_DC
		// "Z-bias" bodge to stop Z-fighting.
		quad[i]->Z += 0.0001f;
#endif
	}

	if (POLY_valid_quad(quad))
	{
		//
		// Add in the uv coords and colour.
		//

		for (i = 0; i < 4; i++)
		{
			OVAL_get_uv(
				float(map_x + (i &  1) << 8),
				float(map_z + (i >> 1) << 8),
			   &pp[i].u,
			   &pp[i].v);

			pp[i].colour   = colour;
			pp[i].specular = 0xff000000;
		}

#if 1
		// A reordering that matches the way the land is drawn, and avoids Z-fights.
		quad[0] = &pp[1];
		quad[1] = &pp[3];
		quad[2] = &pp[0];
		quad[3] = &pp[2];
#endif


		POLY_add_quad(quad, page, FALSE);
	}
}


void OVAL_add(
		SLONG x,	// 8 bits per mapsquare
		SLONG y,
		SLONG z,
		SLONG size,
		float elongate,
		float angle,
		SLONG type)
{
	SLONG mx;
	SLONG mz;

	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	//
	// Work out roughly the bounding box we need to project the oval into/
	//

	SLONG msize = ftol(float(size) * elongate + 8.0F);

	mx1 = x - msize >> PAP_SHIFT_HI;
	mz1 = z - msize >> PAP_SHIFT_HI;
	mx2 = x + msize >> PAP_SHIFT_HI;
	mz2 = z + msize >> PAP_SHIFT_HI;

	SATURATE(mx1, 1, 126);
	SATURATE(mz1, 1, 126);

	SATURATE(mx2, 1, 126);
	SATURATE(mz2, 1, 126);

	//
	// Work out the oval mapping.
	//

	OVAL_mid_x = float(x);
	OVAL_mid_y = float(y);
	OVAL_mid_z = float(z);

	OVAL_dudx = (sin(angle) / float(size)) / elongate;
	OVAL_dvdx = (cos(angle) / float(size));

	OVAL_dudz = (sin(angle + PI / 2) / float(size)) / elongate;
	OVAL_dvdz = (cos(angle + PI / 2) / float(size));

	//
	// Which page are we using?
	//

	SLONG page;

	switch(type)
	{
		case OVAL_TYPE_OVAL:    page = POLY_PAGE_SHADOW_OVAL;   break;
		case OVAL_TYPE_SQUARE: page = POLY_PAGE_SHADOW_SQUARE; break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Project the oval onto these squares.
	//

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		OVAL_project_onto_mapsquare(mx,mz,page);
	}
}
