//
// Sewers.. yuck!
//

#include "game.h"
#include "sewer.h"



//
// The sewage system.
//

#define SEWER_TYPE_OFF			0
#define SEWER_TYPE_ON			1
#define SEWER_TYPE_LADDER_XS	2
#define SEWER_TYPE_LADDER_XL	3
#define SEWER_TYPE_LADDER_ZS	4
#define SEWER_TYPE_LADDER_ZL	5
#define SEWER_TYPE_PILLAR		6
#define SEWER_TYPE_WALL_XS		7
#define SEWER_TYPE_WALL_XL		8
#define SEWER_TYPE_WALL_ZS		9
#define SEWER_TYPE_WALL_ZL		10
#define SEWER_TYPE_WALL_CSS		11
#define SEWER_TYPE_WALL_CSL		12
#define SEWER_TYPE_WALL_CLS		13
#define SEWER_TYPE_WALL_CLL		14
#define SEWER_TYPE_WALL_PSS		15
#define SEWER_TYPE_WALL_PSL		16
#define SEWER_TYPE_WALL_PLS		17
#define SEWER_TYPE_WALL_PLL		18


UBYTE SEWER_square[SEWER_SIZE][SEWER_SIZE];


//
// Returns TRUE if the sewer square is walkable.
//

#define SEWER_TYPE_WALKABLE(type) (WITHIN((type), SEWER_TYPE_ON, SEWER_TYPE_LADDER_ZL))


//
// Creating the sewage system.
//

void SEWER_init()
{
	memset((UBYTE*)SEWER_square, SEWER_TYPE_OFF, sizeof(SEWER_square));
}

void SEWER_square_on(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, SEWER_SIZE - 1));
	ASSERT(WITHIN(z, 0, SEWER_SIZE - 1));

	SEWER_square[x][z] = SEWER_TYPE_ON;
}

void SEWER_ladder_on(SLONG x, SLONG z, SLONG edge)
{
	ASSERT(WITHIN(x,    0, SEWER_SIZE - 1));
	ASSERT(WITHIN(z,    0, SEWER_SIZE - 1));
	ASSERT(WITHIN(edge, 0, 3));

	SEWER_square[x][z] = SEWER_TYPE_LADDER_XS + edge;
}

void SEWER_pillar_on(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, SEWER_SIZE - 1));
	ASSERT(WITHIN(z, 0, SEWER_SIZE - 1));

	SEWER_square[x][z] = SEWER_TYPE_PILLAR;
}

void SEWER_precalc()
{
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG nx;
	SLONG nz;

	UBYTE on;

	for (x = 0; x < SEWER_SIZE; x++)
	for (z = 0; z < SEWER_SIZE; z++)
	{
		if (SEWER_square[x][z] == SEWER_TYPE_OFF)
		{
			i  = 0;
			on = 0;
				
			for (i = 0; i < 4; i++)
			{
				dx = 0;
				dz = 0;

				switch(i)
				{
					case 0: dx = -1; break;
					case 1: dx = +1; break;
					case 2: dz = -1; break;
					case 3: dz = +1; break;
				}

				nx = x + dx;
				nz = z + dz;

				if (WITHIN(nx, 0, SEWER_SIZE - 1) &&
					WITHIN(nz, 0, SEWER_SIZE - 1))
				{
					if (WITHIN(SEWER_square[nx][nz], SEWER_TYPE_ON, SEWER_TYPE_PILLAR))
					{
						on |= 1 << i;
					}
				}
			}

			switch(on)
			{
				case 0:	  SEWER_square[x][z] = SEWER_TYPE_OFF;      break;
				case 1:	  SEWER_square[x][z] = SEWER_TYPE_WALL_XS;  break;
				case 2:	  SEWER_square[x][z] = SEWER_TYPE_WALL_XL;  break;
				case 4:	  SEWER_square[x][z] = SEWER_TYPE_WALL_ZS;  break;
				case 8:	  SEWER_square[x][z] = SEWER_TYPE_WALL_ZL;  break;
				case 1|4: SEWER_square[x][z] = SEWER_TYPE_WALL_CSS; break;
				case 1|8: SEWER_square[x][z] = SEWER_TYPE_WALL_CSL; break;
				case 2|4: SEWER_square[x][z] = SEWER_TYPE_WALL_CLS; break;
				case 2|8: SEWER_square[x][z] = SEWER_TYPE_WALL_CLL; break;
			}
		}
	}
 
	//
	// The corner points.
	//

	for (x = 0; x < SEWER_SIZE; x++)
	for (z = 0; z < SEWER_SIZE; z++)
	{
		if (SEWER_square[x][z] == SEWER_TYPE_OFF)
		{
			i  = 0;
			on = 0;
				
			for (i = 0; i < 4; i++)
			{
				switch(i)
				{
					case 0: dx = -1; dz = -1; break;
					case 1: dx = +1; dz = -1; break;
					case 2: dx = -1; dz = +1; break;
					case 3: dx = +1; dz = +1; break;
				}

				nx = x + dx;
				nz = z + dz;

				if (WITHIN(nx, 0, SEWER_SIZE - 1) &&
					WITHIN(nz, 0, SEWER_SIZE - 1))
				{
					if (WITHIN(SEWER_square[nx][nz], SEWER_TYPE_ON, SEWER_TYPE_PILLAR))
					{
						on |= 1 << i;
					}
				}
			}

			switch(on)
			{
				case 1: SEWER_square[x][z] = SEWER_TYPE_WALL_PSS; break;
				case 2: SEWER_square[x][z] = SEWER_TYPE_WALL_PLS; break;
				case 4: SEWER_square[x][z] = SEWER_TYPE_WALL_PSL; break;
				case 8: SEWER_square[x][z] = SEWER_TYPE_WALL_PLL; break;
			}
		}
	}
}

void SEWER_save(CBYTE *filename)
{
#ifndef	PSX
	FILE *handle = MF_Fopen(filename, "wb");

	if (handle == NULL)
	{
		TRACE("Could not open sewage file %s\n", filename);
	}
	else
	{
		if (fwrite(SEWER_square, 1, sizeof(SEWER_square), handle) == sizeof(SEWER_square))
		{
			//
			// Write ok.
			//
		}
		else
		{
			TRACE("Error writing sewage file %s\n", filename);
		}

		MF_Fclose(handle);
	}
#endif
}

void SEWER_load(CBYTE *filename)
{
#ifndef	PSX
	FILE *handle = MF_Fopen(filename, "rb");

	SEWER_init();

	if (handle == NULL)
	{
		TRACE("Could not open sewage file %s\n", filename);
	}
	else
	{
		if (fread(SEWER_square, 1, sizeof(SEWER_square), handle) == sizeof(SEWER_square))
		{
			//
			// Read ok.
			//
		}
		else
		{
			TRACE("Error reading sewage file %s\n", filename);

			//
			// Repair the damage.
			//

			SEWER_init();
		}

		MF_Fclose(handle);
	}
#endif
}



SLONG SEWER_can_i_enter(UBYTE x, UBYTE z)
{
	if (WITHIN(x, 0, SEWER_SIZE - 1) &&
		WITHIN(z, 0, SEWER_SIZE - 1))
	{
		if (SEWER_TYPE_WALKABLE(SEWER_square[x][z]))
		{
			return TRUE;
		}
	}

	return FALSE;
}




SLONG SEWER_colvect_old_next_col_vect;
SLONG SEWER_colvect_old_next_col_vect_link;
SLONG SEWER_colvect_stuff_valid;


void SEWER_colvects_insert()
{
	SLONG x;
	SLONG z;

	SLONG started;
	SLONG need;
	SLONG walkable1;
	SLONG walkable2;
	SLONG swap;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	//
	// The old colvect values.
	//

	SEWER_colvect_old_next_col_vect      = next_col_vect;
	SEWER_colvect_old_next_col_vect_link = next_col_vect_link;
	SEWER_colvect_stuff_valid            = TRUE;

	//
	// Scan the map one way...
	//

	for (x = 1; x < SEWER_SIZE - 1; x++)
	{
		started = FALSE;

		for (z = 0; z < SEWER_SIZE; z++)
		{
			//
			// Do we need a colvect along this edge?
			//

			walkable1 = SEWER_TYPE_WALKABLE(SEWER_square[x - 0][z]);
			walkable2 = SEWER_TYPE_WALKABLE(SEWER_square[x - 1][z]);
			need      = walkable1 ^ walkable2;

			if (need)
			{
				if (!started)
				{
					x1 = x << 8;
					z1 = z << 8;

					started = TRUE;

					//
					// Get the orientation of the colvect correct.
					//

					swap = walkable2;
				}
			}
			else
			{
				if (started)
				{
					x2 = x << 8;
					z2 = z << 8;

					started = FALSE;

					//
					// Insert the colvect.
					//

					if (swap)
					{
						insert_collision_vect(
							x2, -SEWER_DEPTH, z2,
							x1, -SEWER_DEPTH, z1,
							STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_AGAIN_THIS_IS_THE_VALUE_PUT_INTO_PRIMTYPE_BY_THE_SEWERS,
							1, 0);
					}
					else
					{
						insert_collision_vect(
							x1, -SEWER_DEPTH, z1,
							x2, -SEWER_DEPTH, z2,
							STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_AGAIN_THIS_IS_THE_VALUE_PUT_INTO_PRIMTYPE_BY_THE_SEWERS,
							1, 0);
					}
				}
			}
		}
	}

	//
	// ...then scan the map the other way.
	//

	for (z = 1; z < SEWER_SIZE - 1; z++)
	{
		started = FALSE;

		for (x = 0; x < SEWER_SIZE; x++)
		{
			//
			// Do we need a colvect along this edge?
			//

			walkable1 = SEWER_TYPE_WALKABLE(SEWER_square[x][z - 0]);
			walkable2 = SEWER_TYPE_WALKABLE(SEWER_square[x][z - 1]);
			need      = walkable1 ^ walkable2;

			if (need)
			{
				if (!started)
				{
					x1 = x << 8;
					z1 = z << 8;

					started = TRUE;

					//
					// Get the orientation of the colvect correct.
					//

					swap = walkable1;
				}
			}
			else
			{
				if (started)
				{
					x2 = x << 8;
					z2 = z << 8;

					started = FALSE;

					//
					// Insert the colvect.
					//

					if (swap)
					{
						insert_collision_vect(
							x2, -SEWER_DEPTH, z2,
							x1, -SEWER_DEPTH, z1,
							STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_AGAIN_THIS_IS_THE_VALUE_PUT_INTO_PRIMTYPE_BY_THE_SEWERS,
							1, 0);
					}
					else
					{
						insert_collision_vect(
							x1, -SEWER_DEPTH, z1,
							x2, -SEWER_DEPTH, z2,
							STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_AGAIN_THIS_IS_THE_VALUE_PUT_INTO_PRIMTYPE_BY_THE_SEWERS,
							1, 0);
					}
				}
			}
		}
	}
}

void SEWER_colvects_remove()
{
	SLONG i;

	ASSERT(SEWER_colvect_stuff_valid);

	for (i = SEWER_colvect_old_next_col_vect; i < next_col_vect; i++)
	{
		remove_collision_vect(i);
	}

	//
	// We are not using these any more...
	//

	next_col_vect             = SEWER_colvect_old_next_col_vect;
	next_col_vect_link        = SEWER_colvect_old_next_col_vect_link;
	SEWER_colvect_stuff_valid = FALSE;
}



SLONG SEWER_calc_height_at(SLONG x, SLONG z)
{
	return -SEWER_DEPTH;
}


// ########################################################
// ========================================================
//
// DRAWING THE SEWER SYSTEM
//
// ========================================================
// ########################################################

//
// Creating sewer faces.
//

#define SEWER_LO_RES 1

#if SEWER_LO_RES

#define SEWER_CURVE_STEPS		4
#define SEWER_CURVE_STEPS_SHIFT	2

UBYTE SEWER_curve[SEWER_CURVE_STEPS + 1] =	// i.e. a curve
{
	0,
	181,
	255,
	181,
	0
};

#else

#define SEWER_CURVE_STEPS		8
#define SEWER_CURVE_STEPS_SHIFT	3

UBYTE SEWER_curve[SEWER_CURVE_STEPS + 1] =	// i.e. a curve
{
	0,
	98,
	181,
	236,
	255,
	236,
	181,
	98,
	0
};

#endif

//
// SEWER_create_face put its stuff in here.
//

SEWER_Face SEWER_face;

//
// These functions let you draw a curved surface between the four
// control points given in the order...
//
//		 1----3
// Norm1 |	  | Norm2
//		 |	  |
//		 0----2
//

#define SEWER_INT_X		0
#define SEWER_INT_Y		1
#define SEWER_INT_Z		2
#define SEWER_INT_U		3
#define SEWER_INT_V		4
#define SEWER_INT_R		5
#define SEWER_INT_G		6
#define SEWER_INT_B		7
#define SEWER_INT_NUM	8

SLONG SEWER_fi[SEWER_INT_NUM][2];
SLONG SEWER_fd[SEWER_INT_NUM][2];

SLONG SEWER_norm1_x;
SLONG SEWER_norm1_y;
SLONG SEWER_norm1_z;

SLONG SEWER_norm2_x;
SLONG SEWER_norm2_y;
SLONG SEWER_norm2_z;

SLONG SEWER_created;

void SEWER_set_control_points(
		SLONG x[4],
		SLONG y[4],
		SLONG z[4],
		SLONG u[4],
		SLONG v[4],
		SLONG r[4],
		SLONG g[4],
		SLONG b[4],
		SLONG page,
		SLONG n1x,
		SLONG n1y,
		SLONG n1z,
		SLONG n2x,
		SLONG n2y,
		SLONG n2z)
{
	SLONG i;

	SLONG p0;
	SLONG p1;

	SLONG len;

	//
	// Create the interpolants.
	//

	for (i = 0; i < 2; i++)
	{
		p0 = i * 2 + 0;
		p1 = i * 2 + 1;

		SEWER_fi[SEWER_INT_X][i] = x[p0];
		SEWER_fd[SEWER_INT_X][i] = x[p1] - x[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_Y][i] = y[p0];
		SEWER_fd[SEWER_INT_Y][i] = y[p1] - y[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_Z][i] = z[p0];
		SEWER_fd[SEWER_INT_Z][i] = z[p1] - z[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_U][i] = u[p0];
		SEWER_fd[SEWER_INT_U][i] = u[p1] - v[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_V][i] = v[p0];
		SEWER_fd[SEWER_INT_V][i] = v[p1] - v[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_R][i] = r[p0];
		SEWER_fd[SEWER_INT_R][i] = r[p1] - r[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_G][i] = g[p0];
		SEWER_fd[SEWER_INT_G][i] = g[p1] - g[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_fi[SEWER_INT_B][i] = b[p0];
		SEWER_fd[SEWER_INT_B][i] = b[p1] - b[p0] >> SEWER_CURVE_STEPS_SHIFT;
	}

	SEWER_face.page = page;

	SEWER_norm1_x = n1x;
	SEWER_norm1_y = n1y;
	SEWER_norm1_z = n1z;
	
	SEWER_norm2_x = n2x;
	SEWER_norm2_y = n2y;
	SEWER_norm2_z = n2z;

	SEWER_created = 0;
}

//
// Creates an interpolated face. Call SEWER_CURVE_STEPS times for each call
// to SEWER_set_control_points().
//

void SEWER_create_face()
{
	SLONG i;
	SLONG j;

	SLONG r[4];
	SLONG g[4];
	SLONG b[4];

	SEWER_face.x[0] = SEWER_fi[SEWER_INT_X][0];
	SEWER_face.x[1] = SEWER_fi[SEWER_INT_X][1];
	SEWER_face.x[2] = SEWER_fi[SEWER_INT_X][0] + SEWER_fd[SEWER_INT_X][0];
	SEWER_face.x[3] = SEWER_fi[SEWER_INT_X][1] + SEWER_fd[SEWER_INT_X][1];

	SEWER_face.y[0] = SEWER_fi[SEWER_INT_Y][0];
	SEWER_face.y[1] = SEWER_fi[SEWER_INT_Y][1];
	SEWER_face.y[2] = SEWER_fi[SEWER_INT_Y][0] + SEWER_fd[SEWER_INT_Y][0];
	SEWER_face.y[3] = SEWER_fi[SEWER_INT_Y][1] + SEWER_fd[SEWER_INT_Y][1];

	SEWER_face.z[0] = SEWER_fi[SEWER_INT_Z][0];
	SEWER_face.z[1] = SEWER_fi[SEWER_INT_Z][1];
	SEWER_face.z[2] = SEWER_fi[SEWER_INT_Z][0] + SEWER_fd[SEWER_INT_Z][0];
	SEWER_face.z[3] = SEWER_fi[SEWER_INT_Z][1] + SEWER_fd[SEWER_INT_Z][1];

	SEWER_face.u[0] = SEWER_fi[SEWER_INT_U][0];
	SEWER_face.u[1] = SEWER_fi[SEWER_INT_U][1];
	SEWER_face.u[2] = SEWER_fi[SEWER_INT_U][0] + SEWER_fd[SEWER_INT_U][0];
	SEWER_face.u[3] = SEWER_fi[SEWER_INT_U][1] + SEWER_fd[SEWER_INT_U][1];

	SEWER_face.v[0] = SEWER_fi[SEWER_INT_V][0];
	SEWER_face.v[1] = SEWER_fi[SEWER_INT_V][1];
	SEWER_face.v[2] = SEWER_fi[SEWER_INT_V][0] + SEWER_fd[SEWER_INT_V][0];
	SEWER_face.v[3] = SEWER_fi[SEWER_INT_V][1] + SEWER_fd[SEWER_INT_V][1];

	r[0] = SEWER_fi[SEWER_INT_R][0];
	r[1] = SEWER_fi[SEWER_INT_R][1];
	r[2] = SEWER_fi[SEWER_INT_R][0] + SEWER_fd[SEWER_INT_R][0];
	r[3] = SEWER_fi[SEWER_INT_R][1] + SEWER_fd[SEWER_INT_R][1];

	g[0] = SEWER_fi[SEWER_INT_G][0];
	g[1] = SEWER_fi[SEWER_INT_G][1];
	g[2] = SEWER_fi[SEWER_INT_G][0] + SEWER_fd[SEWER_INT_G][0];
	g[3] = SEWER_fi[SEWER_INT_G][1] + SEWER_fd[SEWER_INT_G][1];

	b[0] = SEWER_fi[SEWER_INT_B][0];
	b[1] = SEWER_fi[SEWER_INT_B][1];
	b[2] = SEWER_fi[SEWER_INT_B][0] + SEWER_fd[SEWER_INT_B][0];
	b[3] = SEWER_fi[SEWER_INT_B][1] + SEWER_fd[SEWER_INT_B][1];

	SEWER_face.c[0] = (r[0] << 16) | (g[0] << 8) | (b[0] << 0);
	SEWER_face.c[1] = (r[1] << 16) | (g[1] << 8) | (b[1] << 0);
	SEWER_face.c[2] = (r[2] << 16) | (g[2] << 8) | (b[2] << 0);
	SEWER_face.c[3] = (r[3] << 16) | (g[3] << 8) | (b[3] << 0);

	//
	// Add the curviness.
	//

	ASSERT(WITHIN(SEWER_created, 0, SEWER_CURVE_STEPS - 1));

	SEWER_face.x[0] += SEWER_curve[SEWER_created + 0] * SEWER_norm1_x >> 8;
	SEWER_face.y[0] += SEWER_curve[SEWER_created + 0] * SEWER_norm1_y >> 8;
	SEWER_face.z[0] += SEWER_curve[SEWER_created + 0] * SEWER_norm1_z >> 8;

	SEWER_face.x[1] += SEWER_curve[SEWER_created + 0] * SEWER_norm2_x >> 8;
	SEWER_face.y[1] += SEWER_curve[SEWER_created + 0] * SEWER_norm2_y >> 8;
	SEWER_face.z[1] += SEWER_curve[SEWER_created + 0] * SEWER_norm2_z >> 8;

	SEWER_face.x[2] += SEWER_curve[SEWER_created + 1] * SEWER_norm1_x >> 8;
	SEWER_face.y[2] += SEWER_curve[SEWER_created + 1] * SEWER_norm1_y >> 8;
	SEWER_face.z[2] += SEWER_curve[SEWER_created + 1] * SEWER_norm1_z >> 8;

	SEWER_face.x[3] += SEWER_curve[SEWER_created + 1] * SEWER_norm2_x >> 8;
	SEWER_face.y[3] += SEWER_curve[SEWER_created + 1] * SEWER_norm2_y >> 8;
	SEWER_face.z[3] += SEWER_curve[SEWER_created + 1] * SEWER_norm2_z >> 8;

	//
	// Move on the interpolants.
	//

	for (i = 0; i < SEWER_INT_NUM; i++)
	{
		SEWER_fi[i][0] += SEWER_fd[i][0];
		SEWER_fi[i][1] += SEWER_fd[i][1];
	}

	SEWER_created += 1;
}

//
// A higher level over and above SEWER_set_control_points.
//
//         norm3
//
//		2---------3
//		 \ 		 /
//		  \	    /
//	norm1  \   /  norm2
//          \ /
//			0-1
//

#define SEWER_SUPER_X	0
#define SEWER_SUPER_Y	1
#define SEWER_SUPER_Z	2
#define SEWER_SUPER_U	3
#define SEWER_SUPER_V	4
#define SEWER_SUPER_R	5
#define SEWER_SUPER_G	6
#define SEWER_SUPER_B	7
#define SEWER_SUPER_NUM	8

SLONG SEWER_si[SEWER_SUPER_NUM][2];
SLONG SEWER_sd[SEWER_SUPER_NUM][2];
SLONG SEWER_norm_xz_len;
SLONG SEWER_norm_y;
SLONG SEWER_norm3_x;
SLONG SEWER_norm3_y;
SLONG SEWER_norm3_z;
SLONG SEWER_corner_dx;
SLONG SEWER_corner_dy;
SLONG SEWER_corner_dz;
SLONG SEWER_super_page;
SLONG SEWER_super_done;
SLONG SEWER_turn_corner;

void SEWER_super_curve(
		SLONG x[4],
		SLONG y[4],
		SLONG z[4],
		SLONG u[4],
		SLONG v[4],
		SLONG r[4],
		SLONG g[4],
		SLONG b[4],
		SLONG page,
		SLONG nxzlen,
		SLONG ny,
		SLONG n3x,
		SLONG n3y,
		SLONG n3z)
{
	SLONG i;
	SLONG p0;
	SLONG p1;

	//
	// Create the interpolants.
	//

	for (i = 0; i < 2; i++)
	{
		p0 = i * 2 + 0;
		p1 = i * 2 + 1;

		SEWER_si[SEWER_SUPER_X][i] = x[p0];
		SEWER_sd[SEWER_SUPER_X][i] = x[p1] - x[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_Y][i] = y[p0];
		SEWER_sd[SEWER_SUPER_Y][i] = y[p1] - y[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_Z][i] = z[p0];
		SEWER_sd[SEWER_SUPER_Z][i] = z[p1] - z[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_U][i] = u[p0];
		SEWER_sd[SEWER_SUPER_U][i] = u[p1] - v[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_V][i] = v[p0];
		SEWER_sd[SEWER_SUPER_V][i] = v[p1] - v[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_R][i] = r[p0];
		SEWER_sd[SEWER_SUPER_R][i] = r[p1] - r[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_G][i] = g[p0];
		SEWER_sd[SEWER_SUPER_G][i] = g[p1] - g[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_B][i] = b[p0];
		SEWER_sd[SEWER_SUPER_B][i] = b[p1] - b[p0] >> SEWER_CURVE_STEPS_SHIFT;
	}

	//
	// Stuff...
	//

	SEWER_super_page  = page;
	SEWER_super_done  = 0;
	SEWER_norm_xz_len = nxzlen;
	SEWER_norm_y      = ny;
	SEWER_norm3_x     = n3x;
	SEWER_norm3_y     = n3y;
	SEWER_norm3_z     = n3z;
	SEWER_turn_corner = FALSE;
}

//     	 norm3
//     	2------3
//     	|	   |
// norm1|	   |norm2
//     	0	   4
//     	 \	  /
//     	  \	1/
//     	   \/
//

void SEWER_super_curve_corner(
		SLONG x[5],
		SLONG y[5],
		SLONG z[5],
		SLONG u[4],
		SLONG v[4],
		SLONG r[4],
		SLONG g[4],
		SLONG b[4],
		SLONG page,
		SLONG nxzlen,
		SLONG ny,
		SLONG n3x,
		SLONG n3y,
		SLONG n3z)
{
	SLONG i;
	SLONG p0;
	SLONG p1;

	//
	// Create the interpolants.
	//

	for (i = 0; i < 2; i++)
	{
		p0 = i * 2 + 0;
		p1 = i * 2 + 1;

		if (i == 1)
		{
			SEWER_si[SEWER_SUPER_X][i] = x[p0];
			SEWER_sd[SEWER_SUPER_X][i] = x[p1] - x[p0] >> SEWER_CURVE_STEPS_SHIFT;

			SEWER_si[SEWER_SUPER_Y][i] = y[p0];
			SEWER_sd[SEWER_SUPER_Y][i] = y[p1] - y[p0] >> SEWER_CURVE_STEPS_SHIFT;

			SEWER_si[SEWER_SUPER_Z][i] = z[p0];
			SEWER_sd[SEWER_SUPER_Z][i] = z[p1] - z[p0] >> SEWER_CURVE_STEPS_SHIFT;
		}
		else
		{
			SEWER_si[SEWER_SUPER_X][i] = x[p0];
			SEWER_sd[SEWER_SUPER_X][i] = x[p1] - x[p0] >> (SEWER_CURVE_STEPS_SHIFT - 1);

			SEWER_si[SEWER_SUPER_Y][i] = y[p0];
			SEWER_sd[SEWER_SUPER_Y][i] = y[p1] - y[p0] >> (SEWER_CURVE_STEPS_SHIFT - 1);

			SEWER_si[SEWER_SUPER_Z][i] = z[p0];
			SEWER_sd[SEWER_SUPER_Z][i] = z[p1] - z[p0] >> (SEWER_CURVE_STEPS_SHIFT - 1);
		}

		SEWER_si[SEWER_SUPER_U][i] = u[p0];
		SEWER_sd[SEWER_SUPER_U][i] = u[p1] - v[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_V][i] = v[p0];
		SEWER_sd[SEWER_SUPER_V][i] = v[p1] - v[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_R][i] = r[p0];
		SEWER_sd[SEWER_SUPER_R][i] = r[p1] - r[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_G][i] = g[p0];
		SEWER_sd[SEWER_SUPER_G][i] = g[p1] - g[p0] >> SEWER_CURVE_STEPS_SHIFT;

		SEWER_si[SEWER_SUPER_B][i] = b[p0];
		SEWER_sd[SEWER_SUPER_B][i] = b[p1] - b[p0] >> SEWER_CURVE_STEPS_SHIFT;
	}

	//
	// The corner
	//

	SEWER_corner_dx = x[4] - x[1] >> (SEWER_CURVE_STEPS_SHIFT - 1);
	SEWER_corner_dy = y[4] - y[1] >> (SEWER_CURVE_STEPS_SHIFT - 1);
	SEWER_corner_dz = z[4] - z[1] >> (SEWER_CURVE_STEPS_SHIFT - 1);

	//
	// Stuff...
	//

	SEWER_super_page  = page;
	SEWER_super_done  = 0;
	SEWER_norm_xz_len = nxzlen;
	SEWER_norm_y      = ny;
	SEWER_norm3_x     = n3x;
	SEWER_norm3_y     = n3y;
	SEWER_norm3_z     = n3z;
	SEWER_turn_corner = TRUE;
}

void SEWER_super_set()	// Dont call more than SEWER_CURVE_STEPS times for each call to SEWER_super_curve*()
{
	SLONG i;

	SLONG val[SEWER_SUPER_NUM][4];

	SLONG nx1;
	SLONG ny1;
	SLONG nz1;
	SLONG nx2;
	SLONG ny2;
	SLONG nz2;
	SLONG page;

	//
	// Interpolate the values.
	//
	
	for (i = 0; i < SEWER_SUPER_NUM; i++)
	{
		val[i][0] = SEWER_si[i][0];
		val[i][1] = SEWER_si[i][1];
		val[i][2] = val[i][0] + SEWER_sd[i][0];
		val[i][3] = val[i][1] + SEWER_sd[i][1];
	}

	//
	// Add the curviness
	//
	
	ASSERT(WITHIN(SEWER_super_done, 0, SEWER_CURVE_STEPS - 1));

	val[SEWER_SUPER_X][1] += SEWER_norm3_x * SEWER_curve[SEWER_super_done + 0] >> 8;
	val[SEWER_SUPER_Y][1] += SEWER_norm3_y * SEWER_curve[SEWER_super_done + 0] >> 8;
	val[SEWER_SUPER_Z][1] += SEWER_norm3_z * SEWER_curve[SEWER_super_done + 0] >> 8;

	val[SEWER_SUPER_X][3] += SEWER_norm3_x * SEWER_curve[SEWER_super_done + 1] >> 8;
	val[SEWER_SUPER_Y][3] += SEWER_norm3_y * SEWER_curve[SEWER_super_done + 1] >> 8;
	val[SEWER_SUPER_Z][3] += SEWER_norm3_z * SEWER_curve[SEWER_super_done + 1] >> 8;

	SEWER_super_done += 1;

	//
	// Work out the normals.
	//

	SLONG len1;
	SLONG len2;

	ny1 = SEWER_norm_y;
	ny2 = SEWER_norm_y;

	nx1 = val[SEWER_SUPER_X][1] - val[SEWER_SUPER_X][0];
	nz1 = val[SEWER_SUPER_Z][1] - val[SEWER_SUPER_Z][0];

	nx2 = val[SEWER_SUPER_X][3] - val[SEWER_SUPER_X][2];
	nz2 = val[SEWER_SUPER_Z][3] - val[SEWER_SUPER_Z][2];

	len1 = QDIST2(abs(nx1), abs(nz1));
	len2 = QDIST2(abs(nx2), abs(nz2));

	nx1 *= SEWER_norm_xz_len;
	nz1 *= SEWER_norm_xz_len;

	nx2 *= SEWER_norm_xz_len;
	nz2 *= SEWER_norm_xz_len;

	nx1 /= len1;
	nz1 /= len1;

	nx2 /= len2;
	nz2 /= len2;

	//
	// Pass them on as control points.
	//

	SEWER_set_control_points(
		val[SEWER_SUPER_X],
		val[SEWER_SUPER_Y],
		val[SEWER_SUPER_Z],
		val[SEWER_SUPER_U],
		val[SEWER_SUPER_V],
		val[SEWER_SUPER_R],
		val[SEWER_SUPER_G],
		val[SEWER_SUPER_B],
		SEWER_super_page,
		nx1, ny1, nz1,
		nx2, ny2, nz2);

	//
	// Move on the interpolants.
	//
	
	for (i = 0; i < SEWER_SUPER_NUM; i++)
	{
		SEWER_si[i][0] += SEWER_sd[i][0];
		SEWER_si[i][1] += SEWER_sd[i][1];
	}

	if (SEWER_super_done == SEWER_CURVE_STEPS / 2)
	{
		if (SEWER_turn_corner)
		{
			//
			// Turn the corner.
			//

			SEWER_sd[SEWER_SUPER_X][0] = SEWER_corner_dx;
			SEWER_sd[SEWER_SUPER_Y][0] = SEWER_corner_dy;
			SEWER_sd[SEWER_SUPER_Z][0] = SEWER_corner_dz;
		}
	}
}



//
// Getting mapsquare faces.
//

#define SEWER_COLOUR_FLOOR (0x00bbccdd)
#define SEWER_COLOUR_ROOF  (0x00080808)

UBYTE SEWER_get_type;
SLONG SEWER_get_x;
SLONG SEWER_get_z;
SLONG SEWER_got_floor;
SLONG SEWER_got_wall;
SLONG SEWER_got_wall1_count;
SLONG SEWER_got_wall2_count;
SLONG SEWER_got_pillar;
SLONG SEWER_got_pillar_count;
SLONG SEWER_got_pillar_wall_count;
SLONG SEWER_got_water_count;

void SEWER_get_start(SLONG x, SLONG z)
{
	ASSERT(WITHIN(x, 0, SEWER_SIZE - 1));
	ASSERT(WITHIN(z, 0, SEWER_SIZE - 1));

	SEWER_get_type              = SEWER_square[x][z];
	SEWER_get_x                 = x;
	SEWER_get_z                 = z;
	SEWER_got_floor             = FALSE;
	SEWER_got_wall              = FALSE;
	SEWER_got_pillar            = (SEWER_get_type != SEWER_TYPE_PILLAR);
	SEWER_got_wall1_count       = 0;
	SEWER_got_wall2_count       = 0;
	SEWER_got_pillar_count      = 0;
	SEWER_got_pillar_wall_count = 0;
	SEWER_got_water_count       = 0;

	//
	// Which groups of faces do we need?
	//

	if (SEWER_get_type == SEWER_TYPE_OFF)
	{
		SEWER_got_floor  = TRUE;
		SEWER_got_wall   = TRUE;
		SEWER_got_pillar = TRUE;
	}
	else
	if (WITHIN(SEWER_get_type, SEWER_TYPE_ON, SEWER_TYPE_PILLAR))
	{
		SEWER_got_wall = TRUE;
	}
	else
	{
		SEWER_got_floor = TRUE;
	}
}

SEWER_Face *SEWER_get_next()
{
	if (!SEWER_got_floor)
	{
		SEWER_face.x[0] = SEWER_get_x + 0 << 8;
		SEWER_face.z[0] = SEWER_get_z + 0 << 8;
		SEWER_face.x[1] = SEWER_get_x + 1 << 8;
		SEWER_face.z[1] = SEWER_get_z + 0 << 8;
		SEWER_face.x[2] = SEWER_get_x + 0 << 8;
		SEWER_face.z[2] = SEWER_get_z + 1 << 8;
		SEWER_face.x[3] = SEWER_get_x + 1 << 8;
		SEWER_face.z[3] = SEWER_get_z + 1 << 8;

		SEWER_face.y[0] = -SEWER_DEPTH;
		SEWER_face.y[1] = -SEWER_DEPTH;
		SEWER_face.y[2] = -SEWER_DEPTH;
		SEWER_face.y[3] = -SEWER_DEPTH;

		SEWER_face.u[0] = 0x00000000;
		SEWER_face.v[0] = 0x00000000;
		SEWER_face.u[1] = 0x01000000;
		SEWER_face.v[1] = 0x00000000;
		SEWER_face.u[2] = 0x00000000;
		SEWER_face.v[2] = 0x01000000;
		SEWER_face.u[3] = 0x01000000;
		SEWER_face.v[3] = 0x01000000;

		SEWER_face.c[0] = SEWER_COLOUR_FLOOR;
		SEWER_face.c[1] = SEWER_COLOUR_FLOOR;
		SEWER_face.c[2] = SEWER_COLOUR_FLOOR;
		SEWER_face.c[3] = SEWER_COLOUR_FLOOR;

		SEWER_face.page = SEWER_PAGE_FLOOR;

		SEWER_got_floor = TRUE;

		return &SEWER_face;
	}

	#define SEWER_WALL_HEIGHT      0x100
	#define SEWER_WALL_WIDTH       0xa0
	#define SEWER_WALL_CURVE       (SEWER_WALL_WIDTH * 75 >> 8)
	#define SEWER_WALL_PCURVE      (SEWER_WALL_CURVE >> 1)
	#define SEWER_WALL_CCURVE      ((0x100 - SEWER_WALL_WIDTH >> 1) * 75 >> 8)
	#define SEWER_PILLAR_WIDTH_BOT 0x50
	#define SEWER_PILLAR_WIDTH_TOP 0x30
	#define SEWER_PILLAR_CURVE     0x20

	if (!SEWER_got_wall)
	{
		if (SEWER_got_wall1_count == 0 &&
			SEWER_got_wall2_count == 0)
		{
			SLONG x[5];
			SLONG y[5];
			SLONG z[5];
			SLONG u[4];
			SLONG v[4];
			SLONG r[4];
			SLONG g[4];
			SLONG b[4];

			//
			// These are common to all wall types.
			//

			y[0] = -SEWER_DEPTH;
			y[1] = -SEWER_DEPTH + SEWER_WALL_HEIGHT;
			y[2] = -SEWER_DEPTH;
			y[3] = -SEWER_DEPTH + SEWER_WALL_HEIGHT;
			y[4] = -SEWER_DEPTH;

			u[0] = 0x00000000;
			v[0] = 0x00000000;
			u[1] = 0x01000000;
			v[1] = 0x00000000;
			u[2] = 0x00000000;
			v[2] = 0x01000000;
			u[3] = 0x01000000;
			v[3] = 0x01000000;

			r[0] = (SEWER_COLOUR_FLOOR >> 16) & 0xff;
			r[1] = (SEWER_COLOUR_ROOF  >> 16) & 0xff;
			r[2] = (SEWER_COLOUR_FLOOR >> 16) & 0xff;
			r[3] = (SEWER_COLOUR_ROOF  >> 16) & 0xff;

			g[0] = (SEWER_COLOUR_FLOOR >>  8) & 0xff;
			g[1] = (SEWER_COLOUR_ROOF  >>  8) & 0xff;
			g[2] = (SEWER_COLOUR_FLOOR >>  8) & 0xff;
			g[3] = (SEWER_COLOUR_ROOF  >>  8) & 0xff;

			b[0] = (SEWER_COLOUR_FLOOR >>  0) & 0xff;
			b[1] = (SEWER_COLOUR_ROOF  >>  0) & 0xff;
			b[2] = (SEWER_COLOUR_FLOOR >>  0) & 0xff;
			b[3] = (SEWER_COLOUR_ROOF  >>  0) & 0xff;

			if (!WITHIN(SEWER_get_type, SEWER_TYPE_WALL_XS, SEWER_TYPE_WALL_ZL))
			{
				//
				// The super control set takes these values in a different order.
				//
				
				SWAP(y[1], y[2]);
				SWAP(u[1], u[2]);
				SWAP(v[1], v[2]);
				SWAP(r[1], r[2]);
				SWAP(g[1], g[2]);
				SWAP(b[1], b[2]);
			}

			switch(SEWER_get_type)
			{
				//
				// Initialise the simple 
				//

				case SEWER_TYPE_WALL_XS:

					x[0] = SEWER_get_x << 8;
					x[2] = SEWER_get_x << 8;
					x[1] = x[0] + SEWER_WALL_WIDTH;
					x[3] = x[2] + SEWER_WALL_WIDTH;

					z[0] = SEWER_get_z + 1 << 8;
					z[1] = SEWER_get_z + 1 << 8;
					z[2] = SEWER_get_z + 0 << 8;
					z[3] = SEWER_get_z + 0 << 8;

					SEWER_set_control_points(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
						0,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
						0);

					SEWER_got_wall2_count = SEWER_CURVE_STEPS;

					break;
					
				case SEWER_TYPE_WALL_XL:

					x[0] = SEWER_get_x + 1 << 8;
					x[2] = SEWER_get_x + 1 << 8;
					x[1] = x[0] - SEWER_WALL_WIDTH;
					x[3] = x[2] - SEWER_WALL_WIDTH;

					z[0] = SEWER_get_z + 1 << 8;
					z[1] = SEWER_get_z + 1 << 8;
					z[2] = SEWER_get_z + 0 << 8;
					z[3] = SEWER_get_z + 0 << 8;

					SEWER_set_control_points(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
						0,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
						0);

					SEWER_got_wall2_count = SEWER_CURVE_STEPS;

					break;
					
				case SEWER_TYPE_WALL_ZS:

					x[0] = SEWER_get_x + 1 << 8;
					x[1] = SEWER_get_x + 1 << 8;
					x[2] = SEWER_get_x + 0 << 8;
					x[3] = SEWER_get_x + 0 << 8;

					z[0] = SEWER_get_z + 0 << 8;
					z[2] = SEWER_get_z + 0 << 8;
					z[1] = z[0] + SEWER_WALL_WIDTH;
					z[3] = z[2] + SEWER_WALL_WIDTH;

					SEWER_set_control_points(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						0,
					   -SEWER_WALL_CURVE,
						SEWER_WALL_CURVE,
						0,
					   -SEWER_WALL_CURVE,
						SEWER_WALL_CURVE);

					SEWER_got_wall2_count = SEWER_CURVE_STEPS;

					break;

				case SEWER_TYPE_WALL_ZL:

					x[0] = SEWER_get_x + 1 << 8;
					x[1] = SEWER_get_x + 1 << 8;
					x[2] = SEWER_get_x + 0 << 8;
					x[3] = SEWER_get_x + 0 << 8;

					z[0] = SEWER_get_z + 1 << 8;
					z[2] = SEWER_get_z + 1 << 8;
					z[1] = z[0] - SEWER_WALL_WIDTH;
					z[3] = z[2] - SEWER_WALL_WIDTH;

					SEWER_set_control_points(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						0,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
						0,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE);

					SEWER_got_wall2_count = SEWER_CURVE_STEPS;

					break;

				case SEWER_TYPE_WALL_CSS:

					x[1] = SEWER_get_x + 0 << 8;
					z[1] = SEWER_get_z + 0 << 8;
					x[0] = SEWER_get_x + 0 << 8;
					z[0] = SEWER_get_z + 1 << 8;
					x[4] = SEWER_get_x + 1 << 8;
					z[4] = SEWER_get_z + 0 << 8;

					x[2] = x[0] + SEWER_WALL_WIDTH;
					z[2] = z[0];
					x[3] = x[4];
					z[3] = z[4] + SEWER_WALL_WIDTH;

					SEWER_super_curve_corner(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_CCURVE,
						0,
					   -SEWER_WALL_CCURVE);
					
					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_CSL:

					x[1] = SEWER_get_x + 0 << 8;
					z[1] = SEWER_get_z + 1 << 8;
					x[0] = SEWER_get_x + 1 << 8;
					z[0] = SEWER_get_z + 1 << 8;
					x[4] = SEWER_get_x + 0 << 8;
					z[4] = SEWER_get_z + 0 << 8;

					x[2] = x[0];
					z[2] = z[0] - SEWER_WALL_WIDTH;
					x[3] = x[4] + SEWER_WALL_WIDTH;
					z[3] = z[4];

					SEWER_super_curve_corner(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_CCURVE,
						0,
					    SEWER_WALL_CCURVE);
					
					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_CLS:

					x[1] = SEWER_get_x + 1 << 8;
					z[1] = SEWER_get_z + 0 << 8;
					x[0] = SEWER_get_x + 0 << 8;
					z[0] = SEWER_get_z + 0 << 8;
					x[4] = SEWER_get_x + 1 << 8;
					z[4] = SEWER_get_z + 1 << 8;

					x[2] = x[0];
					z[2] = z[0] + SEWER_WALL_WIDTH;
					x[3] = x[4] - SEWER_WALL_WIDTH;
					z[3] = z[4];

					SEWER_super_curve_corner(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					   +SEWER_WALL_CCURVE,
						0,
					   -SEWER_WALL_CCURVE);
					
					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_CLL:

					x[1] = SEWER_get_x + 1 << 8;
					z[1] = SEWER_get_z + 1 << 8;
					x[0] = SEWER_get_x + 0 << 8;
					z[0] = SEWER_get_z + 1 << 8;
					x[4] = SEWER_get_x + 1 << 8;
					z[4] = SEWER_get_z + 0 << 8;

					x[2] = x[0];
					z[2] = z[0] - SEWER_WALL_WIDTH;
					x[3] = x[4] - SEWER_WALL_WIDTH;
					z[3] = z[4];

					SEWER_super_curve_corner(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					   +SEWER_WALL_CCURVE,
						0,
					   +SEWER_WALL_CCURVE);
					
					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_PSS:

					x[0] = SEWER_get_x + 0 << 8;
					z[0] = SEWER_get_z + 0 << 8;
					x[1] = SEWER_get_x + 0 << 8;
					z[1] = SEWER_get_z + 0 << 8;
					x[2] = x[0];
					z[2] = z[0] + SEWER_WALL_WIDTH;
					x[3] = x[1] + SEWER_WALL_WIDTH;
					z[3] = z[1];

					SEWER_super_curve(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
						SEWER_WALL_PCURVE,
						0,
						SEWER_WALL_PCURVE);

					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_PLS:

					x[0] = SEWER_get_x + 1 << 8;
					z[0] = SEWER_get_z + 0 << 8;
					x[1] = SEWER_get_x + 1 << 8;
					z[1] = SEWER_get_z + 0 << 8;
					x[2] = x[0] - SEWER_WALL_WIDTH;
					z[2] = z[0];
					x[3] = x[1];                   
					z[3] = z[1] + SEWER_WALL_WIDTH;

					SEWER_super_curve(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_PCURVE,
						0,
						SEWER_WALL_PCURVE);

					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_PSL:

					x[0] = SEWER_get_x + 0 << 8;
					z[0] = SEWER_get_z + 1 << 8;
					x[1] = SEWER_get_x + 0 << 8;
					z[1] = SEWER_get_z + 1 << 8;
					x[2] = x[0] + SEWER_WALL_WIDTH;
					z[2] = z[0];
					x[3] = x[1];                   
					z[3] = z[1] - SEWER_WALL_WIDTH;

					SEWER_super_curve(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					    SEWER_WALL_PCURVE,
						0,
					   -SEWER_WALL_PCURVE);

					SEWER_super_set();

					break;

				case SEWER_TYPE_WALL_PLL:

					x[0] = SEWER_get_x + 1 << 8;
					z[0] = SEWER_get_z + 1 << 8;
					x[1] = SEWER_get_x + 1 << 8;
					z[1] = SEWER_get_z + 1 << 8;
					x[2] = x[0] - SEWER_WALL_WIDTH;
					z[2] = z[0];
					x[3] = x[1];                   
					z[3] = z[1] - SEWER_WALL_WIDTH;

					SEWER_super_curve(
						x, y, z, u, v, r, g, b,
						SEWER_PAGE_WALL,
						SEWER_WALL_CURVE,
					   -SEWER_WALL_CURVE,
					   -SEWER_WALL_PCURVE,
						0,
					   -SEWER_WALL_PCURVE);

					SEWER_super_set();

					break;

				default:
					ASSERT(0);
					break;
			}
		}

		SEWER_got_wall1_count += 1;

		if (SEWER_got_wall1_count > SEWER_CURVE_STEPS)
		{
			SEWER_got_wall1_count  = 1;
			SEWER_got_wall2_count += 1;

			if (SEWER_got_wall2_count >= SEWER_CURVE_STEPS)
			{
				//
				// Finished with getting walls.
				//

				SEWER_got_wall = TRUE;
			}
			else
			{
				//
				// Initialise the next set of wall1 stuff...
				//

				SEWER_super_set();
			}
		}

		if (!SEWER_got_wall)
		{
			SEWER_create_face();

			return &SEWER_face;
		}
	}

	if (!SEWER_got_pillar)
	{
		if (SEWER_got_pillar_wall_count == 0)
		{
			if (SEWER_got_pillar_count >= 4)
			{
				SEWER_got_pillar = TRUE;
			}
			else
			{
				SLONG x[5];
				SLONG y[5];
				SLONG z[5];
				SLONG u[4];
				SLONG v[4];
				SLONG r[4];
				SLONG g[4];
				SLONG b[4];
				SLONG xm;
				SLONG zm;
				SLONG curve_x1;
				SLONG curve_y1;
				SLONG curve_z1;
				SLONG curve_x2;
				SLONG curve_y2;
				SLONG curve_z2;

				//
				// These are common to all the walls.
				//

				y[0] = -SEWER_DEPTH;
				y[1] = -SEWER_DEPTH + SEWER_WALL_HEIGHT;
				y[2] = -SEWER_DEPTH;
				y[3] = -SEWER_DEPTH + SEWER_WALL_HEIGHT;
				y[4] = -SEWER_DEPTH;

				u[0] = 0x00000000;
				v[0] = 0x00000000;
				u[1] = 0x01000000;
				v[1] = 0x00000000;
				u[2] = 0x00000000;
				v[2] = 0x01000000;
				u[3] = 0x01000000;
				v[3] = 0x01000000;

				r[0] = (SEWER_COLOUR_FLOOR >> 16) & 0xff;
				r[1] = (SEWER_COLOUR_ROOF  >> 16) & 0xff;
				r[2] = (SEWER_COLOUR_FLOOR >> 16) & 0xff;
				r[3] = (SEWER_COLOUR_ROOF  >> 16) & 0xff;

				g[0] = (SEWER_COLOUR_FLOOR >>  8) & 0xff;
				g[1] = (SEWER_COLOUR_ROOF  >>  8) & 0xff;
				g[2] = (SEWER_COLOUR_FLOOR >>  8) & 0xff;
				g[3] = (SEWER_COLOUR_ROOF  >>  8) & 0xff;

				b[0] = (SEWER_COLOUR_FLOOR >>  0) & 0xff;
				b[1] = (SEWER_COLOUR_ROOF  >>  0) & 0xff;
				b[2] = (SEWER_COLOUR_FLOOR >>  0) & 0xff;
				b[3] = (SEWER_COLOUR_ROOF  >>  0) & 0xff;

				xm = SEWER_get_x + SEWER_get_x + 1 << 7;
				zm = SEWER_get_z + SEWER_get_z + 1 << 7;

				switch(SEWER_got_pillar_count)
				{
					case 0:

						x[0] = xm - SEWER_PILLAR_WIDTH_BOT;
						z[0] = zm - SEWER_PILLAR_WIDTH_BOT;
						x[1] = xm - SEWER_PILLAR_WIDTH_TOP;
						z[1] = zm - SEWER_PILLAR_WIDTH_TOP;

						x[2] = xm - SEWER_PILLAR_WIDTH_BOT;
						z[2] = zm + SEWER_PILLAR_WIDTH_BOT;
						x[3] = xm - SEWER_PILLAR_WIDTH_TOP;
						z[3] = zm + SEWER_PILLAR_WIDTH_TOP;

						curve_x1 =  SEWER_PILLAR_CURVE;
						curve_y1 = -SEWER_PILLAR_CURVE;
						curve_z1 =  SEWER_PILLAR_CURVE;

						curve_x2 =  SEWER_PILLAR_CURVE;
						curve_y2 = -SEWER_PILLAR_CURVE;
						curve_z2 = -SEWER_PILLAR_CURVE;

						break;

					case 1:

						x[0] = xm - SEWER_PILLAR_WIDTH_BOT;
						z[0] = zm + SEWER_PILLAR_WIDTH_BOT;
						x[1] = xm - SEWER_PILLAR_WIDTH_TOP;
						z[1] = zm + SEWER_PILLAR_WIDTH_TOP;

						x[2] = xm + SEWER_PILLAR_WIDTH_BOT;
						z[2] = zm + SEWER_PILLAR_WIDTH_BOT;
						x[3] = xm + SEWER_PILLAR_WIDTH_TOP;
						z[3] = zm + SEWER_PILLAR_WIDTH_TOP;

						curve_x1 =  SEWER_PILLAR_CURVE;
						curve_y1 = -SEWER_PILLAR_CURVE;
						curve_z1 = -SEWER_PILLAR_CURVE;

						curve_x2 = -SEWER_PILLAR_CURVE;
						curve_y2 = -SEWER_PILLAR_CURVE;
						curve_z2 = -SEWER_PILLAR_CURVE;

						break;

					case 2:

						x[0] = xm + SEWER_PILLAR_WIDTH_BOT;
						z[0] = zm + SEWER_PILLAR_WIDTH_BOT;
						x[1] = xm + SEWER_PILLAR_WIDTH_TOP;
						z[1] = zm + SEWER_PILLAR_WIDTH_TOP;

						x[2] = xm + SEWER_PILLAR_WIDTH_BOT;
						z[2] = zm - SEWER_PILLAR_WIDTH_BOT;
						x[3] = xm + SEWER_PILLAR_WIDTH_TOP;
						z[3] = zm - SEWER_PILLAR_WIDTH_TOP;

						curve_x1 = -SEWER_PILLAR_CURVE;
						curve_y1 = -SEWER_PILLAR_CURVE;
						curve_z1 = -SEWER_PILLAR_CURVE;

						curve_x2 = -SEWER_PILLAR_CURVE;
						curve_y2 = -SEWER_PILLAR_CURVE;
						curve_z2 =  SEWER_PILLAR_CURVE;

						break;

					case 3:

						x[0] = xm + SEWER_PILLAR_WIDTH_BOT;
						z[0] = zm - SEWER_PILLAR_WIDTH_BOT;
						x[1] = xm + SEWER_PILLAR_WIDTH_TOP;
						z[1] = zm - SEWER_PILLAR_WIDTH_TOP;

						x[2] = xm - SEWER_PILLAR_WIDTH_BOT;
						z[2] = zm - SEWER_PILLAR_WIDTH_BOT;
						x[3] = xm - SEWER_PILLAR_WIDTH_TOP;
						z[3] = zm - SEWER_PILLAR_WIDTH_TOP;

						curve_x1 = -SEWER_PILLAR_CURVE;
						curve_y1 = -SEWER_PILLAR_CURVE;
						curve_z1 =  SEWER_PILLAR_CURVE;

						curve_x2 =  SEWER_PILLAR_CURVE;
						curve_y2 = -SEWER_PILLAR_CURVE;
						curve_z2 =  SEWER_PILLAR_CURVE;

						break;

					default:
						ASSERT(0);
						break;
				}

				SEWER_set_control_points(
					x, y, z, u, v, r, g, b,
					SEWER_PAGE_WALL,
					curve_x1,
					curve_y1,
					curve_z1,
					curve_x2,
					curve_y2,
					curve_z2);
			}
		}

		if (!SEWER_got_pillar)
		{
			SEWER_create_face();

			SEWER_got_pillar_wall_count += 1;

			if (SEWER_got_pillar_wall_count >= SEWER_CURVE_STEPS)
			{
				SEWER_got_pillar_wall_count = 0;
				SEWER_got_pillar_count     += 1;
			}

			return &SEWER_face;
		}
	}

	return NULL;
}


SEWER_Face *SEWER_get_water()
{
	if (SEWER_get_type == SEWER_TYPE_OFF)
	{
		return NULL;
	}

	#define SEWER_WATER_HEIGHT   (-SEWER_DEPTH + 0x40)
	#define SEWER_WATER_WALL     (0x60)
	#define SEWER_WATER_COLOUR   (0xffffffff)
	#define SEWER_WATER_WALL_UV1 (SEWER_WATER_WALL << 16)
	#define SEWER_WATER_WALL_UV2 ((0x100 - SEWER_WATER_WALL) << 16)

	//
	// Common to all types...
	//

	SEWER_face.y[0] = SEWER_WATER_HEIGHT;
	SEWER_face.y[1] = SEWER_WATER_HEIGHT;
	SEWER_face.y[2] = SEWER_WATER_HEIGHT;
	SEWER_face.y[3] = SEWER_WATER_HEIGHT;
	SEWER_face.c[0] = SEWER_WATER_COLOUR;
	SEWER_face.c[1] = SEWER_WATER_COLOUR;
	SEWER_face.c[2] = SEWER_WATER_COLOUR;
	SEWER_face.c[3] = SEWER_WATER_COLOUR;

	switch(SEWER_get_type)
	{
		case SEWER_TYPE_ON:
		case SEWER_TYPE_LADDER_XS:
		case SEWER_TYPE_LADDER_XL:
		case SEWER_TYPE_LADDER_ZS:
		case SEWER_TYPE_LADDER_ZL:
		case SEWER_TYPE_PILLAR:

			if (SEWER_got_water_count == 0)
			{
				SEWER_face.x[0] = SEWER_get_x + 0 << 8;
				SEWER_face.z[0] = SEWER_get_z + 0 << 8;
				SEWER_face.x[1] = SEWER_get_x + 1 << 8;
				SEWER_face.z[1] = SEWER_get_z + 0 << 8;
				SEWER_face.x[2] = SEWER_get_x + 0 << 8;
				SEWER_face.z[2] = SEWER_get_z + 1 << 8;
				SEWER_face.x[3] = SEWER_get_x + 1 << 8;
				SEWER_face.z[3] = SEWER_get_z + 1 << 8;
				SEWER_face.u[0] = 0x00000000;
				SEWER_face.v[0] = 0x00000000;
				SEWER_face.u[1] = 0x01000000;
				SEWER_face.v[1] = 0x00000000;
				SEWER_face.u[2] = 0x00000000;
				SEWER_face.v[2] = 0x01000000;
				SEWER_face.u[3] = 0x01000000;
				SEWER_face.v[3] = 0x01000000;

				return &SEWER_face;
			}
			
			break;

		case SEWER_TYPE_WALL_XS:

			if (SEWER_got_water_count == 0)
			{
				SEWER_face.x[0]  = SEWER_get_x + 0 << 8;
				SEWER_face.z[0]  = SEWER_get_z + 0 << 8;
				SEWER_face.x[1]  = SEWER_get_x + 0 << 8;
				SEWER_face.x[1] += SEWER_WATER_WALL;
				SEWER_face.z[1]  = SEWER_get_z + 0 << 8;
				SEWER_face.x[2]  = SEWER_get_x + 0 << 8;
				SEWER_face.z[2]  = SEWER_get_z + 1 << 8;
				SEWER_face.x[3]  = SEWER_get_x + 0 << 8;
				SEWER_face.x[3] += SEWER_WATER_WALL;
				SEWER_face.z[3]  = SEWER_get_z + 1 << 8;
				SEWER_face.u[0]  = 0x00000000;
				SEWER_face.v[0]  = 0x00000000;
				SEWER_face.u[1]  = SEWER_WATER_WALL_UV1;
				SEWER_face.v[1]  = 0x00000000;
				SEWER_face.u[2]  = 0x00000000;
				SEWER_face.v[2]  = 0x01000000;
				SEWER_face.u[3]  = SEWER_WATER_WALL_UV1;
				SEWER_face.v[3]  = 0x01000000;

				return &SEWER_face;
			}

			break;

		case SEWER_TYPE_WALL_XL:

			if (SEWER_got_water_count == 0)
			{
				SEWER_face.x[0]  = SEWER_get_x + 1 << 8;
				SEWER_face.x[0] -= SEWER_WATER_WALL;
				SEWER_face.z[0]  = SEWER_get_z + 0 << 8;
				SEWER_face.x[1]  = SEWER_get_x + 1 << 8;
				SEWER_face.z[1]  = SEWER_get_z + 0 << 8;
				SEWER_face.x[2]  = SEWER_get_x + 1 << 8;
				SEWER_face.x[2] -= SEWER_WATER_WALL;
				SEWER_face.z[2]  = SEWER_get_z + 1 << 8;
				SEWER_face.x[3]  = SEWER_get_x + 1 << 8;
				SEWER_face.z[3]  = SEWER_get_z + 1 << 8;
				SEWER_face.u[0]  = SEWER_WATER_WALL_UV2;
				SEWER_face.v[0]  = 0x00000000;
				SEWER_face.u[1]  = 0x01000000;
				SEWER_face.v[1]  = 0x00000000;
				SEWER_face.u[2]  = SEWER_WATER_WALL_UV2;
				SEWER_face.v[2]  = 0x01000000;
				SEWER_face.u[3]  = 0x01000000;
				SEWER_face.v[3]  = 0x01000000;

				return &SEWER_face;
			}

			break;

		case SEWER_TYPE_WALL_ZS:

			if (SEWER_got_water_count == 0)
			{
				SEWER_face.x[0]  = SEWER_get_x + 0 << 8;
				SEWER_face.z[0]  = SEWER_get_z + 0 << 8;
				SEWER_face.x[1]  = SEWER_get_x + 1 << 8;
				SEWER_face.z[1]  = SEWER_get_z + 0 << 8;
				SEWER_face.x[2]  = SEWER_get_x + 0 << 8;
				SEWER_face.z[2]  = SEWER_get_z + 0 << 8;
				SEWER_face.z[2] += SEWER_WATER_WALL;
				SEWER_face.x[3]  = SEWER_get_x + 1 << 8;
				SEWER_face.z[3]  = SEWER_get_z + 0 << 8;
				SEWER_face.z[3] += SEWER_WATER_WALL;
				SEWER_face.u[0]  = 0x00000000;
				SEWER_face.v[0]  = 0x00000000;
				SEWER_face.u[1]  = 0x01000000;
				SEWER_face.v[1]  = 0x00000000;
				SEWER_face.u[2]  = 0x00000000;
				SEWER_face.v[2]  = SEWER_WATER_WALL_UV1;
				SEWER_face.u[3]  = 0x01000000;
				SEWER_face.v[3]  = SEWER_WATER_WALL_UV1;

				return &SEWER_face;
			}

			break;

		case SEWER_TYPE_WALL_ZL:

			if (SEWER_got_water_count == 0)
			{
				SEWER_face.x[0]  = SEWER_get_x + 0 << 8;
				SEWER_face.z[0]  = SEWER_get_z + 1 << 8;
				SEWER_face.z[0] -= SEWER_WATER_WALL;
				SEWER_face.x[1]  = SEWER_get_x + 1 << 8;
				SEWER_face.z[1]  = SEWER_get_z + 1 << 8;
				SEWER_face.z[1] -= SEWER_WATER_WALL;
				SEWER_face.x[2]  = SEWER_get_x + 0 << 8;
				SEWER_face.z[2]  = SEWER_get_z + 1 << 8;
				SEWER_face.x[3]  = SEWER_get_x + 1 << 8;
				SEWER_face.z[3]  = SEWER_get_z + 1 << 8;
				SEWER_face.u[0]  = 0x00000000;
				SEWER_face.v[0]  = SEWER_WATER_WALL_UV2;
				SEWER_face.u[1]  = 0x01000000;
				SEWER_face.v[1]  = SEWER_WATER_WALL_UV2;
				SEWER_face.u[2]  = 0x00000000;
				SEWER_face.v[2]  = 0x01000000;
				SEWER_face.u[3]  = 0x01000000;
				SEWER_face.v[3]  = 0x01000000;

				return &SEWER_face;
			}

			break;

		default:
			return NULL;
			break;
	}

	return NULL;
}
