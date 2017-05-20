//
// Draped cloth and flags.
//
#include "game.h"
#ifndef	PSX
#include <MFStdLib.h>
#include <math.h>
#include "cloth.h"
#include "pap.h"


//
// The cloth
// 

typedef struct
{
	float x;
	float y;
	float z;
	float dx;
	float dy;
	float dz;
	
} CLOTH_Point;

typedef struct
{
	UBYTE type;
	UBYTE next;
	UBYTE padding;
	ULONG colour;
	ULONG lock;	// One bit per point.

	float offset_x;
	float offset_y;
	float offset_z;

	float dist[2];	// [0] = The distance the points want to be from eachother SQUARED
					// [1] = The distance * 1.41421 ... root2				   SQUARED
					// Index by the type of link.

	CLOTH_Point p[CLOTH_WIDTH * CLOTH_HEIGHT];

} CLOTH_Cloth;

#define CLOTH_MAX_CLOTH 16

CLOTH_Cloth CLOTH_cloth[CLOTH_MAX_CLOTH];
SLONG       CLOTH_cloth_last;

//
// The links between the different cloth.
// 

#define CLOTH_LINK_TYPE_ONE	0
#define CLOTH_LINK_TYPE_HYP	1	// A diagonal connection.

typedef struct
{
	UBYTE p1;
	UBYTE p2;
	UBYTE type;
	
} CLOTH_Link;

#define CLOTH_MAX_LINKS 256

CLOTH_Link CLOTH_link[CLOTH_MAX_LINKS];
SLONG      CLOTH_link_upto;

//
// How we calculate the normal of each point.
//

typedef struct
{
	ULONG num;
	UBYTE p1[4];
	UBYTE p2[4];
	float overnum;

} CLOTH_Normal;

#define CLOTH_MAX_NORMALS (CLOTH_WIDTH * CLOTH_HEIGHT)

CLOTH_Normal CLOTH_normal[CLOTH_MAX_NORMALS];


//
// The mapwho.
//

UBYTE CLOTH_mapwho[PAP_SIZE_LO][PAP_SIZE_LO];


//
// The global elasticity of cloth.
// The global damping.
//

float CLOTH_elasticity =  0.0003F;
float CLOTH_damping    =  0.95F;
float CLOTH_gravity    = -0.15F;


//
// Fast approximation to vector length in 3d.
//

static inline float qdist(float x, float y, float z)
{
	float ans;

	ASSERT(x >= 0.0F);
	ASSERT(y >= 0.0F);
	ASSERT(z >= 0.0F);

	if (x > y)
	{
		if (x > z)
		{
			//
			// x is the biggeset.
			//

			ans = x + (y + z) * 0.2941F;

			return ans;
		}
	}
	else
	{
		if (y > z)
		{
			//
			// y is the biggeset.
			//

			ans = y + (x + z) * 0.2941F;

			return ans;
		}
	}

	//
	// z is the biggeset.
	//

	ans = z + (x + y) * 0.2941F;

	return ans;
}



void CLOTH_init()
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG y;
	SLONG dx;
	SLONG dy;
	SLONG x1;
	SLONG y1;
	SLONG x2;
	SLONG y2;
	SLONG o1;
	SLONG o2;

	const struct {SBYTE dx; SBYTE dy;} order[4] =
	{
		{+1, 0},
		{0, -1},
		{-1, 0},
		{0, +1}
	};

	CLOTH_Normal *cn;

	//
	// Mark all the cloth as unused.
	//

	for (i = 0; i < CLOTH_MAX_CLOTH; i++)
	{
		CLOTH_cloth[i].type = CLOTH_TYPE_UNUSED;
	}

	CLOTH_cloth_last = 1;

	//
	// Clear the mapwho.
	//

	memset(CLOTH_mapwho, 0, sizeof(CLOTH_mapwho));

	//
	// Create the link array.
	//

	CLOTH_link_upto = 0;

	for (x = 0; x < CLOTH_WIDTH;  x++)
	for (y = 0; y < CLOTH_HEIGHT; y++)
	{
		for (dx = -1; dx <= 1; dx++)
		for (dy = -1; dy <= 1; dy++)
		{
			if (dx | dy)
			{
				x1 = x;
				y1 = y;

				x2 = x + dx;
				y2 = y + dy;

				if (WITHIN(x2, 0, CLOTH_WIDTH  - 1) &&
					WITHIN(y2, 0, CLOTH_HEIGHT - 1))
				{
					ASSERT(WITHIN(CLOTH_link_upto, 0, CLOTH_MAX_LINKS - 1));

					CLOTH_link[CLOTH_link_upto].p1   = CLOTH_INDEX(x1,y1);
					CLOTH_link[CLOTH_link_upto].p2   = CLOTH_INDEX(x2,y2);
					CLOTH_link[CLOTH_link_upto].type = (dx == 0 || dy == 0) ? CLOTH_LINK_TYPE_ONE : CLOTH_LINK_TYPE_HYP;

					CLOTH_link_upto += 1;
				}
			}
		}
	}

	//
	// Create the normal array.
	//

	cn = &CLOTH_normal[0];

	for (y = 0; y < CLOTH_HEIGHT; y++)
	for (x = 0; x < CLOTH_WIDTH;  x++)
	{
		cn->num = 0;
		
		for (i = 0; i < 4; i++)
		{
			o1 = (i + 0) & 3;
			o2 = (i + 1) & 3;

			x1 = x + order[o1].dx;
			y1 = y + order[o1].dy;

			x2 = x + order[o2].dx;
			y2 = y + order[o2].dy;

			if (WITHIN(x1, 0, CLOTH_WIDTH  - 1) &&
				WITHIN(y1, 0, CLOTH_HEIGHT - 1) &&
				WITHIN(x2, 0, CLOTH_WIDTH  - 1) &&
				WITHIN(y2, 0, CLOTH_HEIGHT - 1))
			{
				cn->p1[cn->num] = CLOTH_INDEX(x1,y1);
				cn->p2[cn->num] = CLOTH_INDEX(x2,y2);

				cn->num += 1;
			}
		}

		ASSERT(WITHIN(cn->num, 1, 4));

		cn->overnum = 1.0F / float(cn->num);

		cn += 1;
	}
}


UBYTE CLOTH_create(
		UBYTE type,
		SLONG ox,
		SLONG oy,
		SLONG oz,
		SLONG iwdx, SLONG iwdy, SLONG iwdz,
		SLONG ihdx, SLONG ihdy, SLONG ihdz,
		SLONG dist,
		ULONG colour)
{
	SLONG i;
	SLONG j;

	float x;
	float y;
	float z;

	float lx;
	float ly;
	float lz;

	float wdx;
	float wdy;
	float wdz;

	float hdx;
	float hdy;
	float hdz;

	UBYTE map_x;
	UBYTE map_z;

	CLOTH_Cloth *cc;
	CLOTH_Point *cp;

	//
	// Look for an unused cloth.
	//

	for (i = 0; i < CLOTH_MAX_CLOTH; i++)
	{
		CLOTH_cloth_last += 1;

		if (CLOTH_cloth_last >= CLOTH_MAX_CLOTH)
		{
			CLOTH_cloth_last = 1;
		}

		ASSERT(WITHIN(CLOTH_cloth_last, 1, CLOTH_MAX_CLOTH - 1));

		if (CLOTH_cloth[CLOTH_cloth_last].type == CLOTH_TYPE_UNUSED)
		{	
			goto found_unused_cloth;
		}
	}

	//
	// No unused cloth structure :o(
	//

	return NULL;

  found_unused_cloth:;

	ASSERT(WITHIN(CLOTH_cloth_last, 1, CLOTH_MAX_CLOTH - 1));

	cc = &CLOTH_cloth[CLOTH_cloth_last];

	cc->type     = type;
	cc->next     = 0;
	cc->colour   = colour;
	cc->lock     = 0;
	cc->offset_x = 0.0F;
	cc->offset_y = 0.0F;
	cc->offset_z = 0.0F;
	cc->dist[0]  = float(dist) * float(dist);
	cc->dist[1]  = float(dist) * float(dist) * 2.0F;

	//
	// Initialise the points.
	//

	lx  = float(ox);
	ly  = float(oy);
	lz  = float(oz);

	wdx = float(iwdx);
	wdy = float(iwdy);
	wdz = float(iwdz);
	   
	hdx = float(ihdx);
	hdy = float(ihdy);
	hdz = float(ihdz);

	cp = &cc->p[0];

	for (i = 0; i < CLOTH_HEIGHT; i++)
	{
		x = lx;
		y = ly;
		z = lz;

		for (j = 0; j < CLOTH_WIDTH; j++)
		{
			cp->x  = x;
			cp->y  = y;
			cp->z  = z;
			cp->dx = 0.0F;
			cp->dy = 0.0F;
			cp->dz = 0.0F;
			
			cp += 1;

			x += wdx;
			y += wdy;
			z += wdz;
		}

		lx += hdx;
		ly += hdy;
		lz += hdz;
	}

	//
	// Insert on the mapwho.
	// 

	map_x = ox >> PAP_SHIFT_LO;
	map_z = oz >> PAP_SHIFT_LO;

	if (WITHIN(map_x, 0, PAP_SIZE_LO - 1) &&
		WITHIN(map_z, 0, PAP_SIZE_LO - 1))
	{
		cc->next                   = CLOTH_mapwho[map_x][map_z];
		CLOTH_mapwho[map_x][map_z] = CLOTH_cloth_last;
	}

	return CLOTH_cloth_last;
}


void CLOTH_point_lock(UBYTE cloth, UBYTE w, UBYTE h)
{
	CLOTH_Cloth *cc;

	ASSERT(WITHIN(cloth, 1, CLOTH_MAX_CLOTH - 1));
	ASSERT(WITHIN(w,     0, CLOTH_WIDTH     - 1));
	ASSERT(WITHIN(h,     0, CLOTH_HEIGHT    - 1));

	cc = &CLOTH_cloth[cloth];

	cc->lock |= 1 << CLOTH_INDEX(w,h);
}


void CLOTH_point_move(UBYTE cloth, UBYTE w, UBYTE h, SLONG x, SLONG y, SLONG z)
{
	CLOTH_Cloth *cc;
	CLOTH_Point *cp;

	ASSERT(WITHIN(cloth, 1, CLOTH_MAX_CLOTH - 1));
	ASSERT(WITHIN(w,     0, CLOTH_WIDTH     - 1));
	ASSERT(WITHIN(h,     0, CLOTH_HEIGHT    - 1));

	cc = &CLOTH_cloth[cloth];
	cp = &cc->p[CLOTH_INDEX(w,h)];

	cp->x = float(x);
	cp->y = float(y);
	cp->z = float(z);
}

void CLOTH_process()
{
#if 0
	SLONG i;
	SLONG j;

	ULONG bit;

	float dx;
	float dy;
	float dz;

	float wantdist;
	float ddist;
	float dist;

	float force;
	float fx;
	float fy;
	float fz;

	SLONG times;

	static SLONG GAME_TURN = 0;

	CLOTH_Cloth *cc;
	CLOTH_Point *cp;
	CLOTH_Point *cp1;
	CLOTH_Point *cp2;
	CLOTH_Link  *cl;

	for (i = 1; i < CLOTH_MAX_CLOTH; i++)
	{
		cc = &CLOTH_cloth[i];

		if (cc->type != CLOTH_TYPE_UNUSED)
		{
			//
			// Lets go for it! Process the cloth a link at a time.
			//

			for (times = 0; times < 3; times += 1)
			{
				GAME_TURN += 1;

				if (!ShiftFlag)
				{
					//
					// Make flags flap in the wind.
					//

					float wind_x = -0.2F;
					float wind_y =  0.0F;
					float wind_z =  0.25F;

					float upndown;
					
					upndown  = fabs(sin(float(GAME_TURN) * 0.05F)) * (fabs(sin(float(GAME_TURN) * 0.023F)) + 0.5F);
					upndown += 2.5F;
					upndown *= 0.2F;

					if (cc->type == CLOTH_TYPE_FLAG)
					{
						bit = 1 << 0;

						for (j = 0; j < CLOTH_WIDTH * CLOTH_HEIGHT; j++)
						{
							if (!(cc->lock & bit))
							{
								float change;
								
								change  = float(rand()) * (0.25F / float(RAND_MAX));
								change += 0.75F;
								change *= upndown;

								cc->p[j].dx += wind_x * change;
								cc->p[j].dy += wind_y * change;
								cc->p[j].dz += wind_z * change;
							}

							bit <<= 1;
						}

						//cc->offset_x -= wind_x * change;
						//cc->offset_y -= wind_y * change;
						//cc->offset_z -= wind_z * change;
					}
				}

				//
				// Forces between the points.
				//

				for (j = 0; j < CLOTH_link_upto; j++)
				{
					cl = &CLOTH_link[j];

					ASSERT(WITHIN(cl->p1, 0, CLOTH_WIDTH * CLOTH_HEIGHT - 1));
					ASSERT(WITHIN(cl->p2, 0, CLOTH_WIDTH * CLOTH_HEIGHT - 1));
					ASSERT(WITHIN(cl->type, 0, 1));

					cp1 = &cc->p[cl->p1];
					cp2 = &cc->p[cl->p2];

					wantdist = cc->dist[cl->type];

					dx = cp2->x - cp1->x;
					dy = cp2->y - cp1->y;
					dz = cp2->z - cp1->z;

					dist = dx*dx + dy*dy + dz*dz;

					if (dist > wantdist)
					{
						ddist = dist - wantdist;
						force = ddist * CLOTH_elasticity;

						fx = dx * force;
						fy = dy * force;
						fz = dz * force;

						cp1->dx += fx;
						cp1->dy += fy;
						cp1->dz += fz;

						cp2->dx -= fx;
						cp2->dy -= fy;
						cp2->dz -= fz;
					}
				}

				//
				// Damp all the points.
				//

				for (j = 0; j < CLOTH_WIDTH * CLOTH_HEIGHT; j++)
				{
					cp = &cc->p[j];

					cp->dx *= CLOTH_damping;
					cp->dy *= CLOTH_damping;
					cp->dz *= CLOTH_damping;
				}

				//
				// Make sure the locked points don't move.
				//

				bit = 1 << 0;

				for (j = 0; j < CLOTH_WIDTH * CLOTH_HEIGHT; j++)
				{
					if (cc->lock & bit)
					{
						cc->p[j].dx = 0.0F;
						cc->p[j].dy = 0.0F;
						cc->p[j].dz = 0.0F;
					}

					bit <<= 1;
				}

				//
				// Do the movement of the points and gravity.
				//

				for (j = 0; j < CLOTH_WIDTH * CLOTH_HEIGHT; j++)
				{
					cp = &cc->p[j];

					cp->x  += cp->dx;
					cp->y  += cp->dy;
					cp->z  += cp->dz;

					cp->dy += CLOTH_gravity;

					//
					// Make sure the points don't go underground? Yes for the
					// time being.
					//

					if (cp->y < 2.0F)
					{
						cp->y  = 2.0F;
						cp->dy = 0.0F;
					}
				}
			}
		}
	}
#endif
}


UBYTE CLOTH_get_first(UBYTE lo_map_x, UBYTE lo_map_z)
{
	ASSERT(WITHIN(lo_map_x, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(lo_map_z, 0, PAP_SIZE_LO - 1));

	return CLOTH_mapwho[lo_map_x][lo_map_z];
}


CLOTH_Info CLOTH_info;

CLOTH_Info *CLOTH_get_info(UBYTE cloth)
{
	SLONG i;
	SLONG j;

	float nx;
	float ny;
	float nz;
	float len;
	float overlen;

	float vx1;
	float vy1;
	float vz1;

	float vx2;
	float vy2;
	float vz2;

	float cx;
	float cy;
	float cz;

	CLOTH_Cloth  *cc;
	CLOTH_Point  *cp;
	CLOTH_Point  *cp1;
	CLOTH_Point  *cp2;
	CLOTH_Drawp  *cd;
	CLOTH_Normal *cn;

	ASSERT(WITHIN(cloth, 1, CLOTH_MAX_CLOTH - 1));

	cc = &CLOTH_cloth[cloth];

	CLOTH_info.type   = cc->type;
	CLOTH_info.next   = cc->next;
	CLOTH_info.colour = cc->colour;
	
	//
	// Fill in the point positions and normals.
	//

	cd = &CLOTH_info.p[0];
	cp = &cc->p[0];
	cn = &CLOTH_normal[0];
	
	for (i = 0; i < CLOTH_WIDTH * CLOTH_HEIGHT; i++)
	{
		cd->x = cp->x + cc->offset_x;
		cd->y = cp->y + cc->offset_y;
		cd->z = cp->z + cc->offset_z;

		//
		// Calculate the normal
		//

		nx = 0.0F;
		ny = 0.0F;
		nz = 0.0F;

		for (j = 0; j < cn->num; j++)
		{
			cp1 = &cc->p[cn->p1[j]];
			cp2 = &cc->p[cn->p2[j]];

			vx1 = cp1->x - cp->x;
			vy1 = cp1->y - cp->y;
			vz1 = cp1->z - cp->z;

			vx2 = cp2->x - cp->x;
			vy2 = cp2->y - cp->y;
			vz2 = cp2->z - cp->z;

			cx  = vy1 * vz2 - vz1 * vy2;
			cy  = vz1 * vx2 - vx1 * vz2;
			cz  = vx1 * vy2 - vy1 * vx2;

			nx += cx;
			ny += cy;
			nz += cz;
		}

		nx *= cn->overnum;
		ny *= cn->overnum;
		nz *= cn->overnum;

		//
		// Normalise it.
		//

		len     = qdist(fabs(nx),fabs(ny),fabs(nz));
		overlen = 1.0F / len;

		nx *= overlen;
		ny *= overlen;
		nz *= overlen;

		cd->nx = nx;
		cd->ny = ny;
		cd->nz = nz;

		cd += 1;
		cp += 1;
		cn += 1;
	}

	return &CLOTH_info;
}

#endif