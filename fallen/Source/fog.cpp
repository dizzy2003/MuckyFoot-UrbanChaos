//
// Fog.
//

#include "game.h"
#include <MFStdLib.h>
#include "fog.h"
#include "pap.h"

//
// Messages drawn straight to the screen.
//

void MSG_add(CBYTE *message, ...);

//
// This function returns the height of the floor at (x,z).
// It is defined in collide.cpp. We don't #include collide.h
// because then we would have to include thing.h and eventaully
// we'd have to include everything!
//

SLONG calc_height_at(SLONG x, SLONG z);



typedef struct
{
	UBYTE type;
	SBYTE dyaw;
	SWORD yaw;
	UWORD size;
	UWORD shit;
	SLONG x;
	SLONG y;
	SLONG z;

} FOG_Fog;

#ifdef	PSX
#define FOG_MAX_FOG 204
#else
#define FOG_MAX_FOG 2048
#endif

FOG_Fog FOG_fog[FOG_MAX_FOG];

//
// The current focus.
//

SLONG FOG_focus_x;
SLONG FOG_focus_z;
SLONG FOG_focus_radius;


void FOG_init()
{
	SLONG i;

	//
	// Mark all the fog as unused.
	//

	for (i = 0; i < FOG_MAX_FOG; i++)
	{
		FOG_fog[i].type = FOG_TYPE_UNUSED;
	}
}

//
// Returns how fast the given fog index should be turning.
// 

inline SLONG FOG_get_dyaw(SLONG f_index) {return (f_index & 15) - 7;}


//
// Initialises a new bit of fog on the edge of the current focus
//

void FOG_create(SLONG f_index)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG yaw;
	SLONG radius;

	ASSERT(WITHIN(f_index, 0, FOG_MAX_FOG - 1));

	FOG_Fog *ff = &FOG_fog[f_index];

	yaw     = rand() & 2047;
	radius  = 192;
	radius += rand() & 63;

	dx  = SIN(yaw) * radius >> 16;
	dz  = COS(yaw) * radius >> 16;
	dy  = 16;
	dy += rand() & 127;

	dx = dx * FOG_focus_radius >> 8;
	dz = dz * FOG_focus_radius >> 8;

	ff->type  = rand() & 3;
	ff->yaw   = rand() & 2047;
	ff->dyaw  = FOG_get_dyaw(f_index);
	ff->size  = rand() & 0x63;
	ff->size += 64;
	ff->x     = FOG_focus_x                      + dx;
	ff->z     = FOG_focus_z                      + dz;
	ff->y     = PAP_calc_height_at(ff->x, ff->z) + dy;
}


void FOG_set_focus(
  		SLONG x,
		SLONG z,
		SLONG radius)
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	FOG_Fog *ff;

	//
	// Remember the new focus.
	//

	FOG_focus_x      = x;
	FOG_focus_z      = z;
	FOG_focus_radius = radius;

	//
	// For now always have fog around the player...
	//

	for (i = 0; i < FOG_MAX_FOG; i++)
	{
		ff = &FOG_fog[i];

		if (ff->type == FOG_TYPE_UNUSED)
		{
			//
			// Generate a new bit of fog.
			//

			FOG_create(i);
		}
		else
		{
			//
			// Is this bit of fog within the focus?
			//

			dx = abs(ff->x - FOG_focus_x);
			dz = abs(ff->z - FOG_focus_z);

			dist = QDIST2(dx,dz);

			if (dist > FOG_focus_radius)
			{
				//
				// This bit of fog is outside the radius, we
				// must re-create it at the edge of the focus.
				//

				FOG_create(i);
			}
		}
	}
}


void FOG_gust(
		SLONG gx1, SLONG gz1,
		SLONG gx2, SLONG gz2)
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG dgx;
	SLONG dgz;
	SLONG dist;
	SLONG push;
	SLONG cross;
	SLONG strength;
	SLONG dyaw;
	SLONG ddyaw;

	FOG_Fog *ff;

	//
	// How strong (long) is the gust?
	//

	dgx = gx2 - gx1;
	dgz = gz2 - gz1;

	strength = QDIST2(abs(dgx), abs(dgz));

	if (strength == 0)
	{
		//
		// Not worth bothering with.
		//

		return;
	}

	//
	// Look for fog that the gust effects.
	//

	for (i = 0; i < FOG_MAX_FOG; i++)
	{
		ff = &FOG_fog[i];

		if (ff->type == FOG_TYPE_UNUSED)
		{
			continue;
		}

		//
		// How far is the gust from this fog?
		//
		
		dx = gx1 - ff->x;
		dz = gz1 - ff->z;

		dist = QDIST2(abs(dx),abs(dz));

		if (dist == 0)
		{
			//
			// Right on top of the fog.
			//

			continue;
		}

		if (dist > (ff->size * 300 >> 8))
		{
			//
			// Out of range of the fog...
			//

			continue;
		}

		//
		// How much of a push does the fog get?
		//

		push = strength - (abs(dist - ff->size) >> 5);

		if (push > 0)
		{
			//
			// Cross the vectors to find out how much of a
			// rotation we should set the fog spinning by.
			//

			cross  = dgx * dz - dgz * dx;

			ddyaw  = cross;
			ddyaw /= dist;
			ddyaw *= push;
			ddyaw /= strength;
			ddyaw <<= 1;

			SATURATE(ddyaw, -40, +40);

			dyaw  = ff->dyaw;
			dyaw += ddyaw;

			SATURATE(dyaw, -127, +127);

			ff->dyaw = dyaw;
		}
	}
}



void FOG_process()
{
	SLONG i;
	SLONG dyaw;
	SLONG wantdyaw;
	SLONG ddyaw;

	FOG_Fog *ff;

	//
	// Go through all the fog.
	//

	for (i = 0; i < FOG_MAX_FOG; i++)
	{
		ff = &FOG_fog[i];

		//
		// Set it turning...
		//

		ff->yaw += ff->dyaw;

		//
		// How fast should it be turning and how fast is
		// it turning now?
		//

		wantdyaw = FOG_get_dyaw(i);
		dyaw     = ff->dyaw;

		ddyaw = dyaw - wantdyaw;
		dyaw -= SIGN(ddyaw);
		dyaw -= dyaw / 16;

		ff->dyaw = dyaw;
	}
}

SLONG FOG_get_upto;

void     FOG_get_start() {FOG_get_upto = 0;}
FOG_Info FOG_get_info()
{
	SLONG trans;

	FOG_Fog *ff;
	FOG_Info ans;

	if (FOG_get_upto >= FOG_MAX_FOG)
	{
		ans.type = FOG_TYPE_NO_MORE;
	}
	else
	{
		ASSERT(WITHIN(FOG_get_upto, 0, FOG_MAX_FOG - 1));

		ff = &FOG_fog[FOG_get_upto];

		trans = 32 - (abs(ff->dyaw) >> 2);

		if (trans < 8) {trans = 8;}

		ans.type  = ff->type;
		ans.size  = ff->size;
		ans.yaw   = ff->yaw & 2047;
		ans.trans = trans;
		ans.x     = ff->x;
		ans.y     = ff->y;
		ans.z     = ff->z;

		FOG_get_upto += 1;
	}

	return ans;
}


