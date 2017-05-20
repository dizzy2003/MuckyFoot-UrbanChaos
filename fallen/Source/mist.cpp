//
// Mist.
//

#include "game.h"
#include <MFStdLib.h>
#ifndef	PSX
#include <math.h>
#endif
#include "mist.h"
#include "pap.h"


//
// Messages drawn straight to the screen.
//

//void MSG_add(CBYTE *message, ...);

//
// This function returns the height of the floor at (x,z).
// It is defined in collide.cpp. We don't #include collide.h
// because then we would have to include thing.h and eventaully
// we'd have to include everything!
//

SLONG calc_height_at(SLONG x, SLONG z);


typedef struct
{
	float u;	// Offset from base u.
	float v;	// Offset from base v.
	float du;
	float dv;

} MIST_Point;

#ifdef	PSX
#define MIST_MAX_POINTS 4096
#else
#define MIST_MAX_POINTS 4096
#endif

MIST_Point MIST_point[MIST_MAX_POINTS];
SLONG      MIST_point_upto;


typedef struct
{
	UBYTE type;
	UBYTE detail;
	UWORD p_index;
	SLONG height;
	SLONG x1, z1;
	SLONG x2, z2;

} MIST_Mist;

#define MIST_MAX_MIST 8

MIST_Mist MIST_mist[MIST_MAX_MIST];
SLONG     MIST_mist_upto;




void MIST_init()
{
	//
	// Initialise all the points and mist.
	//

	MIST_point_upto = 0;
	MIST_mist_upto  = 0;
}

void MIST_create(
		SLONG detail,			// The number of quads-per-row on the quad patch.
		SLONG height,			// Above the ground.
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2)
{
	SLONG i;
	static SLONG type_cycle = 0;

#ifdef	PSX
	return;
#endif
	ASSERT(WITHIN(MIST_mist_upto, 0, MIST_MAX_MIST - 1));

	MIST_Mist *mm = &MIST_mist[MIST_mist_upto++];

	//
	// No too detailed and not to simple.
	//

	SATURATE(detail, 3, 255);

	//
	// Do we have enough points?
	//

	ASSERT(MIST_point_upto + detail*detail <= MIST_MAX_POINTS);

	//
	// Initialise the new mist.
	//

	mm->type    = type_cycle++ & 0x3;
	mm->height  = height;
	mm->detail  = detail;
	mm->p_index = MIST_point_upto;
	mm->x1      = x1;
	mm->z1      = z1;
	mm->x2      = x2;
	mm->z2      = z2;

	//
	// Initialise all the points.
	//

	for (i = 0; i < detail * detail; i++)
	{
		MIST_point[MIST_point_upto].u  = 0;
		MIST_point[MIST_point_upto].v  = 0;
		MIST_point[MIST_point_upto].du = 0;
		MIST_point[MIST_point_upto].dv = 0;

		MIST_point_upto += 1;
	}
}


void MIST_gust(
		SLONG gx1, SLONG gz1,
		SLONG gx2, SLONG gz2)
{
#ifndef	PSX
	SLONG i;
	SLONG j;

	SLONG sx;
	SLONG sz;

	SLONG sizex;
	SLONG sizez;

	SLONG x1, z1;
	SLONG x2, z2;

	SLONG x;
	SLONG z;

	SLONG wx;
	SLONG wz;

	SLONG dx;
	SLONG dz;

	SLONG dgx;
	SLONG dgz;

	SLONG dist;
	SLONG push;
	SLONG dprod;
	SLONG strength;

	SLONG p_index;

	float ddu;
	float ddv;

	MIST_Mist  *mm;
	MIST_Point *mp;

	//
	// The stength (length) of the gust.  Change the length
	// of the gust depending on the TICK_RATIO, otherwise with
	// fast frame rates, the mist never moves.
	//

	dgx = gx2 - gx1;
	dgz = gz2 - gz1;

	dgx = dgx * TICK_INV_RATIO >> TICK_SHIFT;
	dgz = dgz * TICK_INV_RATIO >> TICK_SHIFT;

	gx2 = gx1 + dgx;
	gz2 = gz1 + dgz;

	strength = QDIST2(abs(dgx), abs(dgz));

	//
	// Cars go too fast...
	//

	#define MIST_MAX_STRENGTH 48

	if (strength > MIST_MAX_STRENGTH)
	{
		gx2 = gx1 + (dgx * MIST_MAX_STRENGTH) / strength;
		gz2 = gz1 + (dgz * MIST_MAX_STRENGTH) / strength;

		strength = MIST_MAX_STRENGTH;
	}

	strength <<= 4;

	if (strength == 0)
	{
		//
		// Ignore this gust.
		//
	}

	for (i = 0; i < MIST_mist_upto; i++)
	{
		mm = &MIST_mist[i];

		sizex = (mm->x2 - mm->x1) / (mm->detail - 1);
		sizez = (mm->z2 - mm->z1) / (mm->detail - 1);

		//
		// Which square of the mist is the gust located in?
		//

		sx = (gx1 - mm->x1) / sizex;
		sz = (gz2 - mm->z1) / sizez;

		if (sx < -1 || sx >= mm->detail ||
			sz < -1 || sz >= mm->detail)
		{
			//
			// The gust is outside the mist.
			//
		}
		else
		{
			//
			// The bounding box of points to consider.
			//

			x1 = sx - 1;
			z1 = sz - 1;
			x2 = sx + 2;
			z2 = sz + 2;

			if (x1 < 0) {x1 = 0;}
			if (z1 < 0) {z1 = 0;}

			if (x2 > mm->detail - 1) {x2 = mm->detail - 1;}
			if (z2 > mm->detail - 1) {z2 = mm->detail - 1;}

			for (x = x1; x <= x2; x++)
			for (z = z1; z <= z2; z++)
			{
				//
				// The world position of this point.
				//

				wx = mm->x1 + x * sizex;
				wz = mm->z1 + z * sizez;

				//
				// How far from the origin of the gust?
				//

				dx = wx - gx1;
				dz = wz - gz1;

				dist = QDIST2(abs(dx), abs(dz));

				if (dist == 0)
				{
					//
					// Ignore this point!
					//

					continue;
				}

				//
				// Do we effect this point?
				//

				if (strength > dist)
				{
					//
					// Yes we do! Depending on whether the point is in front of
					// the gust or behind the gust, it moves towards or away from
					// the origin.
					//

					dprod  = dx * dgx + dz * dgz;

					push  = -dprod;
					push /=  dist;
					push *=  strength - dist;
					push /=  strength;

					ddu = float(dx) * float(push) * 0.0000005F;
					ddv = float(dz) * float(push) * 0.0000005F;

					if (push > 0)
					{
						//
						// Make it follow you more.
						//

						ddu += ddu;
						ddv += ddv;
					}

					//
					// Effect the point.
					//

					p_index  = mm->p_index;
					p_index += x + z * mm->detail;

					ASSERT(WITHIN(p_index, 0, MIST_point_upto - 1));

					mp = &MIST_point[p_index];

					mp->du += ddu;
					mp->dv += ddv;
				}
			}
		}
	}
#endif
}



void MIST_process()
{
	SLONG i;
#ifndef	PSX
	MIST_Point *mp;

	for (i = 0; i < MIST_point_upto; i++)
	{
		mp = &MIST_point[i];

		mp->u += mp->du;
		mp->v += mp->dv;

		mp->dv -= mp->dv * 0.25F;
		mp->du -= mp->du * 0.25F;

		mp->du -= mp->u * 0.005F;
		mp->dv -= mp->v * 0.005F;
	}
#endif
}


SLONG MIST_get_upto;
SLONG MIST_get_dx;
SLONG MIST_get_dz;
float MIST_get_du;
float MIST_get_dv;
SLONG MIST_get_turn;
float MIST_get_base_u;
float MIST_get_base_v;
float MIST_off_u_odd;
float MIST_off_v_odd;
float MIST_off_u_even;
float MIST_off_v_even;

MIST_Mist *MIST_get_mist;


void MIST_get_start()
{
	MIST_get_upto  = 0;
	MIST_get_turn += 1;
}

#ifndef	PSX
SLONG MIST_get_detail()
{
	float radius;
	float yaw_odd;
	float yaw_even;

	MIST_Mist *mm;

	if (!WITHIN(MIST_get_upto, 0, MIST_mist_upto - 1))
	{
		//
		// No more mist left.
		//

		return NULL;
	}
	else
	{
		mm = &MIST_mist[MIST_get_upto++];

		//
		// Remember the layer of mist whose points we are getting.
		//

		MIST_get_mist = mm;

		//
		// Precalculate stuff to make returning the point info faster.
		//

		MIST_get_dx = (mm->x2 - mm->x1) /      (mm->detail - 1);
		MIST_get_dz = (mm->z2 - mm->z1) /      (mm->detail - 1);
		MIST_get_du = 0.5F              / float(mm->detail - 1);
		MIST_get_dv = 0.5F              / float(mm->detail - 1);

		MIST_get_base_u = float(mm->type &  1) * 0.5f;
		MIST_get_base_v = float(mm->type >> 1) * 0.5f;

		//
		// The rotatey wibble on the even and odd points
		//

		yaw_odd  = float(MIST_get_turn)  / (float(mm->type + 1) * 15.0F);
		yaw_odd += MIST_get_upto;
		yaw_even = yaw_odd + (PI / 1.75F);

		//
		// The radius of the rotatey wibble.
		//

		radius = 0.1F / float(mm->detail - 1);

		//
		// The wibble offsets.
		//

		MIST_off_u_odd  = sin(yaw_odd)  * radius;
		MIST_off_v_odd  = cos(yaw_odd)  * radius;

		MIST_off_u_even = sin(yaw_even) * radius;
		MIST_off_v_even = cos(yaw_even) * radius;

		//
		// The texture offsets.
		//

		return mm->detail;
	}
}
void MIST_get_point(SLONG px, SLONG pz,
		SLONG *x,
		SLONG *y,
		SLONG *z)
{
	MIST_Mist *mm = MIST_get_mist;

	ASSERT(WITHIN(mm, &MIST_mist[0], &MIST_mist[MIST_mist_upto - 1]));
	ASSERT(WITHIN(px, 0, mm->detail - 1));
	ASSERT(WITHIN(pz, 0, mm->detail - 1));
	
	*x = mm->x1 + px * MIST_get_dx;
	*z = mm->z1 + pz * MIST_get_dz;
	*y = /*PAP_calc_height_at(*x,*z) +*/ mm->height;
}

void  MIST_get_texture(SLONG px, SLONG pz,
		float *u,
		float *v)
{
	SLONG p_index;

	MIST_Point *mp;
	MIST_Mist  *mm = MIST_get_mist;

	ASSERT(WITHIN(mm, &MIST_mist[0], &MIST_mist[MIST_mist_upto - 1]));
	ASSERT(WITHIN(px, 0, mm->detail - 1));
	ASSERT(WITHIN(pz, 0, mm->detail - 1));

	*u = MIST_get_base_u;
	*v = MIST_get_base_v;

	*u += float(px) * MIST_get_du;
	*v += float(pz) * MIST_get_dv;

	if (px == 0 || px == mm->detail - 1 ||
		pz == 0 || pz == mm->detail - 1)
	{
		//
		// Dont wibble the outside points.
		//

		return;
	}

	//
	// Odd or even?
	//

	if ((px + pz) & 0x1)
	{
		*u += MIST_off_u_odd;
		*v += MIST_off_v_odd;
	}
	else
	{
		*u += MIST_off_u_even;
		*v += MIST_off_v_even;
	}

	p_index  = mm->p_index;
	p_index += px + pz * mm->detail;

	ASSERT(WITHIN(p_index, 0, MIST_point_upto - 1));

	mp = &MIST_point[p_index];

	*u += mp->u;
	*v += mp->v;
}
#endif