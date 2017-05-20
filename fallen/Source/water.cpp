//
// Water.
//

#include "game.h"
#include <MFStdLib.h>
#include "water.h"
//#include <math.h>

//
// The size of the water map.
//

#define WATER_SIZE 128


//
// The water points.
//

typedef struct
{
	UBYTE x;		// 1-bit fixed point!
	SBYTE y;
	UBYTE z;		// 1-bit fixed point!
	SBYTE ddyaw;
	SWORD dyaw;		// Over and above the normal uv rotation.
	UWORD index;

} WATER_Point;

#ifdef	PSX
#define WATER_MAX_POINTS 102
#else
#define WATER_MAX_POINTS 1024
#endif

WATER_Point WATER_point[WATER_MAX_POINTS];
SLONG       WATER_point_upto;


//
// The water faces.
//

typedef struct
{
	UWORD p_index[4];
	UWORD next;

} WATER_Face;

#define WATER_MAX_FACES 512

WATER_Face WATER_face[WATER_MAX_FACES];
SLONG      WATER_face_upto;


//
// The water mapwho.
//

typedef struct
{
	UWORD next;

} WATER_Map;

WATER_Map WATER_map[WATER_SIZE][WATER_SIZE];


//
// The game turn for the water module.
//

SLONG WATER_turn;


//
// The wibble and uv offsets for each type.
//

#define WATER_NUM_TYPES 4

typedef struct
{
	float dy;
	float du;
	float dv;

} WATER_Wibble;

WATER_Wibble WATER_wibble[WATER_NUM_TYPES];



void WATER_init()
{
	SLONG x;
	SLONG z;

	for (x = 0; x < WATER_SIZE; x++)
	for (z = 0; z < WATER_SIZE; z++)
	{
		WATER_map[x][z].next = 0;
	}

	WATER_point_upto = 0;
	WATER_face_upto  = 1;
	WATER_turn       = rand();
}

//
// Returns the index of a point at (x,y,z) or WATER_NO_MORE_POINTS if there
// are no more points left.
//

#define WATER_NO_MORE_POINTS 0xffff

UWORD WATER_get_point(UBYTE x, SBYTE y, UBYTE z)
{
	SLONG i;
	UWORD ans;

	WATER_Point *wp;

	for (i = WATER_point_upto - 1; i >= 0; i--)
	{
		wp = &WATER_point[i];

		if (wp->x == x &&
			wp->y == y &&
			wp->z == z)
		{
			return i;
		}
	}

	//
	// We have to create a new point.
	//

	if (WATER_point_upto >= WATER_MAX_POINTS)
	{
		//
		// No more point memory.
		//

		return WATER_NO_MORE_POINTS;
	}
	else
	{
		ASSERT(WITHIN(WATER_point_upto, 0, WATER_MAX_POINTS - 1));

		ans = WATER_point_upto++;

		wp = &WATER_point[ans];

		wp->x     = x;
		wp->y     = y;
		wp->z     = z;
		wp->dyaw  = 0;
		wp->ddyaw = 0;

		return ans;
	}
}


//
// Adds a face to a mapsquare- does nothing if there are no more
// faces left.
//

void WATER_add_face(
		SLONG map_x,
		SLONG map_z,
		UWORD p_index[4])
{
	WATER_Map  *wm;
	WATER_Face *wf;

	ASSERT(WITHIN(p_index[0], 0, WATER_point_upto - 1));
	ASSERT(WITHIN(p_index[1], 0, WATER_point_upto - 1));
	ASSERT(WITHIN(p_index[2], 0, WATER_point_upto - 1));
	ASSERT(WITHIN(p_index[3], 0, WATER_point_upto - 1));

	ASSERT(WITHIN(map_x, 0, WATER_SIZE - 1));
	ASSERT(WITHIN(map_z, 0, WATER_SIZE - 1));

	if (WATER_face_upto >= WATER_MAX_FACES)
	{
		//
		// No more faces left!
		//

		return;
	}

	//
	// The mapsquare we are creating the face in.
	//

	wm = &WATER_map[map_x][map_z];

	//
	// Create the new face.
	//

	wf = &WATER_face[WATER_face_upto];

	wf->p_index[0] = p_index[0];
	wf->p_index[1] = p_index[1];
	wf->p_index[2] = p_index[2];
	wf->p_index[3] = p_index[3];

	//
	// Insert it in the linked list.
	//

	wf->next = wm->next;
	wm->next = WATER_face_upto;

	WATER_face_upto += 1;
}



void WATER_add(SLONG map_x, SLONG map_z, SLONG height)
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;

	SLONG fx;
	SLONG fz;
	SLONG px;
	SLONG pz;
	SLONG index;

	UWORD p_index[9];
	UWORD f_point[4];
	UBYTE wcx;
	UBYTE wcz;

	ASSERT(WITHIN(map_x, 0, WATER_SIZE - 1));
	ASSERT(WITHIN(map_z, 0, WATER_SIZE - 1));

	if (WATER_map[map_x][map_z].next)
	{
		//
		// Do nothing because there is already water on this square.
		//

		return;
	}

	//
	// The 1-bit fixed point water coordinates
	//

	wcx = map_x << 1;
	wcz = map_z << 1;

	//
	// Create 9 points. One at each corner, one in the middle
	// of each edge and on in the middle of the square...
	//

	i = 0;

	for (z = 0; z < 3; z++)
	for (x = 0; x < 3; x++)
	{
		p_index[i++] = WATER_get_point(wcx + x, height, wcz + z);
	}

	//
	// Create four faces.
	//

	for (i = 0; i < 4; i++)
	{
		fx = i  & 1;
		fz = i >> 1;

		for (j = 0; j < 4; j++)
		{
			px = fx + (j  & 1);
			pz = fz + (j >> 1);

			index = px + pz * 3;

			ASSERT(WITHIN(index, 0, 8));

			f_point[j] = p_index[index];

			if (f_point[j] == WATER_NO_MORE_POINTS)
			{
				//
				// Abandon the face.
				//

				goto abandon_face;
			}
		}

		//
		// Swap the sense of some of the faces...
		//

		if ((fx ^ fz))
		{
			SWAP(f_point[1], f_point[0]);
			SWAP(f_point[3], f_point[0]);
			SWAP(f_point[2], f_point[0]);
		}

		//
		// Add the face to the water map.
		//

		WATER_add_face(map_x, map_z, f_point);

      abandon_face:;
	}
}



void WATER_gush(SLONG gx1, SLONG gz1, SLONG gx2, SLONG gz2)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG dgx;
	SLONG dgz;

	SLONG strength;
	SLONG push;
	SLONG dist;

	SLONG dyaw;
	SLONG ddyaw;

	SLONG wx;
	SLONG wz;

	SLONG px;
	SLONG pz;

	UWORD next;

	WATER_Face  *wf;
	WATER_Point *wp;
	WATER_Map   *wm;

	#define WATER_MAX_DONE 16

	UWORD done[WATER_MAX_DONE];
	SLONG done_upto;

	//
	// What water square is the gush in?
	//

	wx = gx1 >> 8;
	wz = gz1 >> 8;

	if (!WITHIN(wx, 0, WATER_SIZE - 1) ||
		!WITHIN(wz, 0, WATER_SIZE - 1))
	{
		//
		// No water outside the map!
		//

		return;
	}

	wm = &WATER_map[wx][wz];

	if (wm->next == NULL)
	{
		//
		// No water on this square.
		//

		return;
	}

	//
	// The stength (length) of the gust.
	//

	dgx = gx2 - gx1;
	dgz = gz2 - gz1;

	strength   = QDIST2(abs(dgx), abs(dgz));
	strength <<= 2;

	if (strength == 0)
	{
		//
		// No gust!
		//

		return;
	}

	//
	// Remember the points we have done already.
	//

	done_upto = 0;

	//
	// Go through all the faces on this mapsquare to find
	// points we can wibble.
	//

	next = wm->next;

	while(next)
	{
		ASSERT(WITHIN(next, 1, WATER_face_upto - 1));

		wf = &WATER_face[next];

		for (i = 0; i < 4; i++)
		{
			ASSERT(WITHIN(wf->p_index[i], 0, WATER_point_upto - 1));

			//
			// Have we done this point already?
			//

			for (j = done_upto - 1; j >= 0; j--)
			{
				if (done[j] == wf->p_index[i])
				{
					//
					// Don't do this point twice.
					//

					goto next_point;
				}
			}

			ASSERT(WITHIN(done_upto, 0, WATER_MAX_DONE - 1));

			done[done_upto++] = wf->p_index[i];

			//
			// This point.
			//

			wp = &WATER_point[wf->p_index[i]];

			px = wp->x << 7;
			pz = wp->z << 7;

			//
			// How far is this point from the origin of the gust?
			//

			dx = px - gx1;
			dz = pz - gz1;

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
				push   = strength - dist;
				push <<= 1;

				if ((wp->x ^ wp->z) & 0x1)
				{
					push = -push;
				}

				ddyaw  = wp->ddyaw;
				ddyaw += push;

				SATURATE(ddyaw, -127, +127);

				wp->ddyaw = ddyaw;

				/*

				//
				// Yes we do! Depending on whether the point is in front of
				// the gust or behind the gust, it moves up or down.
				//

				dprod  = dx * dgx + dz * dgz;

				push   = -dprod;
				push  /=  dist;
				push  *=  strength - dist;
				push  /=  strength;

				ddy  = wp->ddy;
				ddy += push;
				
				SATURATE(ddy, -35, +35);

				wp->ddy = ddy;
				
				TRACE("push = %d\n", push);

				*/
			}

		  next_point:;
		}

		next = wf->next;
	}
}


void WATER_process()
{
	SLONG i;

	SLONG dy;
	SLONG ddy;

	float yaw_dy;
	float yaw_uv;

	WATER_Point *wp;

	for (i = 0; i < WATER_point_upto; i++)
	{
		wp = &WATER_point[i];

		wp->dyaw  += wp->ddyaw;
		wp->dyaw  &= 2047;

		wp->ddyaw -= SIGN(wp->ddyaw);
	}

	//
	// Calculate the wibbles for this game turn.
	//

	WATER_turn += 1;
/*
	static float wibble_period_dy[WATER_NUM_TYPES] = {0.054F, 0.063F, 0.047F, 0.059F};
	static float wibble_period_uv[WATER_NUM_TYPES] = {0.044F, 0.033F, 0.023F, 0.037F};

	for (i = 0; i < WATER_NUM_TYPES; i++)
	{
		yaw_dy = float(WATER_turn) * wibble_period_dy[i];
		yaw_uv = float(WATER_turn) * wibble_period_uv[i];

		WATER_wibble[i].dy = sin(yaw_dy) * 16.0F;

		WATER_wibble[i].du = sin(yaw_uv) * 0.08;
		WATER_wibble[i].dv = cos(yaw_uv) * 0.08;
	}
*/
}


void WATER_point_index_clear_all()
{
	SLONG i;

	for (i = 0; i < WATER_point_upto; i++)
	{
		WATER_point[i].index = 0;
	}
}

UWORD WATER_point_index_get(UWORD p_index)
{
	UWORD ans;

	ASSERT(WITHIN(p_index, 0, WATER_point_upto - 1));

	ans = WATER_point[p_index].index;

	return ans;
}

void WATER_point_index_set(UWORD p_index, UWORD index)
{
	ASSERT(WITHIN(p_index, 0, WATER_point_upto - 1));

	WATER_point[p_index].index = index;
}


UWORD WATER_get_first_face(SLONG x, SLONG z)
{
	UWORD ans;

	ASSERT(WITHIN(x, 0, WATER_SIZE - 1));
	ASSERT(WITHIN(z, 0, WATER_SIZE - 1));

	ans = WATER_map[x][z].next;

	return ans;
}

UWORD WATER_get_next_face (UWORD f_index)
{
	UWORD ans;

	ASSERT(WITHIN(f_index, 0, WATER_face_upto - 1));

	ans = WATER_face[f_index].next;

	return ans;
}

void WATER_get_face_points(UWORD f_index, UWORD p_index[4])
{
	ASSERT(WITHIN(f_index, 0, WATER_face_upto - 1));

	p_index[0] = WATER_face[f_index].p_index[0];
	p_index[1] = WATER_face[f_index].p_index[1];
	p_index[2] = WATER_face[f_index].p_index[2];
	p_index[3] = WATER_face[f_index].p_index[3];
}

void WATER_get_point_pos(UWORD p_index, float *x, float *y, float *z)
{
	SLONG height;
	SLONG type;

	WATER_Point *wp;

	ASSERT(WITHIN(p_index, 0, WATER_point_upto - 1));
	
	wp = &WATER_point[p_index];

	//
	// The height without wibbling.
	//

	height = wp->y << 5;

	*x = float(wp->x) * 128.0F;
	*z = float(wp->z) * 128.0F;

	*y = float(height);

	//
	// Add the wibbling.
	//

	type = (wp->x ^ wp->z) & 0x3;

	ASSERT(WITHIN(type, 0, WATER_NUM_TYPES - 1));

	*y += WATER_wibble[type].dy;
}

void WATER_get_point_uvs(UWORD p_index, float *u, float *v, ULONG *colour)
{
	SLONG type;

	WATER_Point *wp;

	ASSERT(WITHIN(p_index, 0, WATER_point_upto - 1));

	wp = &WATER_point[p_index];

	//
	// The uv coordinates without wibbling.
	//

	*u = float(wp->x) * 0.19F;
	*v = float(wp->z) * 0.19F;

	//
	// Add the wibbling.
	// 

	type = (wp->x ^ wp->z) & 0x3;

	ASSERT(WITHIN(type, 0, WATER_NUM_TYPES - 1));

	*u += WATER_wibble[type].du;
	*v += WATER_wibble[type].dv;

	//
	// Add the gushing.
	//

	if (wp->dyaw)
	{
		TRACE("wp->dyaw = %d, ddyaw = %d\n", wp->dyaw, wp->ddyaw);
	}

	float du = float(SIN(wp->dyaw) * abs(wp->ddyaw) >> 16);
	float dv = float(COS(wp->dyaw) * abs(wp->ddyaw) >> 16);

	ASSERT(WITHIN(du, -128.0F, 128.0F));
	ASSERT(WITHIN(dv, -128.0F, 128.0F));

	du *= (0.1F / 128.0F);
	dv *= (0.1F / 128.0F);

	*u += du;
	*v += dv;

	//
	// Change the colour...
	//

	*colour  = 0xffbfbfbf;
	*colour += 0x00010101 * abs(SLONG(WATER_wibble[type].dy * 4.0F));
}










