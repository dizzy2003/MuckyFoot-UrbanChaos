//
// Crinkles!
//

#include "game.h"
#include "poly.h"
#include "crinkle.h"

#include <math.h>

#ifdef TARGET_DC
// intrinsic maths
#include <shsgintr.h>
#endif


//
// Temporary storage for transformed crinkle points.
//

#ifdef TARGET_DC
// Only temporary - got no memory for them.
#define DISABLE_CRINKLES 1
#else
#define DISABLE_CRINKLES 0
#endif


#if DISABLE_CRINKLES
#define CRINKLE_MAX_POINTS_PER_CRINKLE 1
#else
#define CRINKLE_MAX_POINTS_PER_CRINKLE 700
#endif

POLY_Point CRINKLE_pp[CRINKLE_MAX_POINTS_PER_CRINKLE];




// Bogus rout - just needs to be out of the way somewhere.
// Just stops the compiler optimising out entire loops!
void DontDoAnythingWithThis ( DWORD blibble )
{
	// Don't do anything.
}



//
// The points.
//

typedef struct
{
	float vec1;		// From point 0 to point 1
	float vec2;		// From point 0 to point 2
	float vec3;		// 0-1 cross 0-2
	UBYTE c[4];		// How much of each points corner's colour this point takes.

} CRINKLE_Point;

#if DISABLE_CRINKLES
#define CRINKLE_MAX_POINTS 1
#else
#define CRINKLE_MAX_POINTS 8192
#endif

CRINKLE_Point CRINKLE_point[CRINKLE_MAX_POINTS];
SLONG         CRINKLE_point_upto;

//
// The faces.
//

typedef struct
{
	UWORD point[3];	// Index in the CRINKLE_point array

} CRINKLE_Face;

#ifdef TARGET_DC
#define CRINKLE_MAX_FACES 1
#else
#define CRINKLE_MAX_FACES 8192
#endif

CRINKLE_Face CRINKLE_face[CRINKLE_MAX_FACES];
SLONG        CRINKLE_face_upto;

//
// The crinkles.
//

typedef struct
{
	SLONG num_points;
	SLONG num_faces;

	CRINKLE_Point *point;
	CRINKLE_Face  *face;

} CRINKLE_Crinkle;

#ifdef TARGET_DC
#define CRINKLE_MAX_CRINKLES 1
#else
#define CRINKLE_MAX_CRINKLES 256
#endif

CRINKLE_Crinkle CRINKLE_crinkle[CRINKLE_MAX_CRINKLES];
SLONG           CRINKLE_crinkle_upto;



void CRINKLE_init(void)
{
	CRINKLE_crinkle_upto = 1;
	CRINKLE_point_upto   = 0;
	CRINKLE_face_upto    = 0;
}



//
// The longest a line will be in an asc...
//

#define CRINKLE_MAX_LINE 256

CRINKLE_Handle CRINKLE_load(CBYTE *asc_filename)
{
	SLONG i;

	SLONG o;
	SLONG f;

	float x;
	float y;
	float z;

	float dx;
	float dy;
	float dz;

	float size;
	float oversize;

	SLONG a;
	SLONG b;
	SLONG c;

	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG match;
	SLONG index;

	CBYTE line[CRINKLE_MAX_LINE];

	CRINKLE_Handle   ans;
	CRINKLE_Crinkle *cc;

	FILE *handle;

	//
	// Open the file.
	//

	return NULL;


#if DISABLE_CRINKLES
	return NULL;
#endif

	handle = MF_Fopen(asc_filename, "rb");

	if (!handle)
	{
		TRACE("Could not open crinkle file \"%s\"\n", asc_filename);

		return NULL;
	}
	else
	{
		TRACE("crinkle = %s\n", asc_filename);
	}

	ASSERT(WITHIN(CRINKLE_crinkle_upto, 1, CRINKLE_MAX_CRINKLES - 1));

	//
	// The new crinkle.
	//

	ans =  CRINKLE_crinkle_upto;
	cc  = &CRINKLE_crinkle[CRINKLE_crinkle_upto++];

	cc->num_points =  0;
	cc->num_faces  =  0;
	cc->point      = &CRINKLE_point[CRINKLE_point_upto];
	cc->face       = &CRINKLE_face [CRINKLE_face_upto];

	index = 0;

	//
	// Load the asc. Put the points into the buffer and the faces
	// into the CRINKLE_face array.
	//

	while(fgets(line, CRINKLE_MAX_LINE, handle))
	{
		match = sscanf(line, "Vertex: (%f, %f, %f)", &x, &y, &z);

		if (match == 3)
		{
			ASSERT(WITHIN(CRINKLE_point_upto, 0, CRINKLE_MAX_POINTS - 1));

			//
			// Found a point. Add it to the buffer.
			//

			SWAP_FL(y, z);
			x = -x;

			CRINKLE_point[CRINKLE_point_upto].vec1  = x;
			CRINKLE_point[CRINKLE_point_upto].vec2  = y;
			CRINKLE_point[CRINKLE_point_upto].vec3  = z;

			CRINKLE_point_upto += 1;
			cc->num_points     += 1;

			continue;
		}

		match = sscanf(line, "Face: Material %d xyz (%d, %d, %d)", &f, &a, &b, &c);

		if (match == 4)
		{
			ASSERT(WITHIN(CRINKLE_face_upto, 0, CRINKLE_MAX_FACES - 1));

			cc->face[cc->num_faces].point[0] = a;
			cc->face[cc->num_faces].point[1] = b;
			cc->face[cc->num_faces].point[2] = c;

			cc->num_faces     += 1;
			CRINKLE_face_upto += 1;

			continue;
		}
	}

	//
	// Finished with the file.
	//

	MF_Fclose(handle);

	//
	// What is the bounding rectangle?
	//

	float minx = +float(INFINITY);
	float miny = +float(INFINITY);
	float minz = +float(INFINITY);

	float maxx = -float(INFINITY);
	float maxy = -float(INFINITY);
	float maxz = -float(INFINITY);

	for (i = 0; i < cc->num_points; i++)
	{
		if (cc->point[i].vec1 < minx) {minx = cc->point[i].vec1;}
		if (cc->point[i].vec2 < miny) {miny = cc->point[i].vec2;}
		if (cc->point[i].vec3 < minz) {minz = cc->point[i].vec3;}

		if (cc->point[i].vec1 > maxx) {maxx = cc->point[i].vec1;}
		if (cc->point[i].vec2 > maxy) {maxy = cc->point[i].vec2;}
		if (cc->point[i].vec3 > maxz) {maxz = cc->point[i].vec3;}
	}

	float sizex = maxx - minx;
	float sizey = maxy - miny;
	float sizez = maxz - minz;

	if (sizey < sizex && sizey < sizez)
	{
		//
		// This is a ground crinkle.
		//

		for (i = 0; i < cc->num_points; i++)
		{
			x = cc->point[i].vec1;
			y = cc->point[i].vec2;
			z = cc->point[i].vec3;

			x -= minx;
			z -= minz;

			x *= 1.0F / 100.0F;
			y *= 1.0F / 100.0F;
			z *= 1.0F / 100.0F;

			SATURATE(x, 0.0F, 1.0F);
			SATURATE(z, 0.0F, 1.0F);

			cc->point[i].vec1 = x;
			cc->point[i].vec2 = z;
			cc->point[i].vec3 = y;			// y is the extrusion coordinate...
		}
	}
	else
	{
		//
		// This is a wall crinkle.
		//

		for (i = 0; i < cc->num_points; i++)
		{
			x = cc->point[i].vec1;
			y = cc->point[i].vec2;
			z = cc->point[i].vec3;

			x -= minx;
			y -= miny;

			x *= 1.0F / 100.0F;
			y *= 1.0F / 100.0F;
			z *= 1.0F / 100.0F;

			SATURATE(x, 0.0F, 1.0F);
			SATURATE(y, 0.0F, 1.0F);

			cc->point[i].vec1 = 1.0F - x;	// They are flipped in x and y apparently!
			cc->point[i].vec2 = 1.0F - y;
			cc->point[i].vec3 = z;			// z is the extrusion coordinate...
		}
	}

	//
	// Set the colour interpolations.
	//

	float v[4];

	for (i = 0; i < cc->num_points; i++)
	{
		v[0] = (1.0F - cc->point[i].vec1) * (1.0F - cc->point[i].vec2);
		v[1] = (       cc->point[i].vec1) * (1.0F - cc->point[i].vec2);
		v[2] = (1.0F - cc->point[i].vec1) * (       cc->point[i].vec2);
		v[3] = (       cc->point[i].vec1) * (       cc->point[i].vec2);

		ASSERT(WITHIN(v[0] + v[1] + v[2] + v[3], 0.9F, 1.1F));

		cc->point[i].c[0] = v[0] * 128.0F;
		cc->point[i].c[1] = v[1] * 128.0F;
		cc->point[i].c[2] = v[2] * 128.0F;
		cc->point[i].c[3] = v[3] * 128.0F;
	}

	return ans;
}

CRINKLE_Handle CRINKLE_read_bin(FileClump* tclump, int id)
{
	int	ii;

	// read data

	UBYTE*	buffer = tclump->Read(id);
	if (!buffer)	return NULL;

	UBYTE*	bptr = buffer;

	ASSERT(WITHIN(CRINKLE_crinkle_upto, 1, CRINKLE_MAX_CRINKLES - 1));

	//
	// The new crinkle.
	//

	CRINKLE_Handle		ans = CRINKLE_crinkle_upto;
	CRINKLE_Crinkle*	cc  = &CRINKLE_crinkle[CRINKLE_crinkle_upto];
	CRINKLE_crinkle_upto++;

	// do the header
	memcpy(cc, bptr, sizeof(*cc));
	bptr += sizeof(*cc);

	CRINKLE_Point*	cp = &CRINKLE_point[CRINKLE_point_upto];
	CRINKLE_Face*	cf = &CRINKLE_face [CRINKLE_face_upto];

	cc->point      = cp;
	cc->face       = cf;

	CRINKLE_point_upto += cc->num_points;
	CRINKLE_face_upto += cc->num_faces;

	for (ii = 0; ii < cc->num_points; ii++)
	{
		memcpy(cp, bptr, sizeof(*cp));
		cp++;
		bptr += sizeof(*cp);
	}

	for (ii = 0; ii < cc->num_faces; ii++)
	{
		memcpy(cf, bptr, sizeof(*cf));
		cf++;
		bptr += sizeof(*cf);
	}

	return ans;
}


#ifndef TARGET_DC
void CRINKLE_write_bin(FileClump* tclump, CRINKLE_Handle hnd, int id)
{
	CRINKLE_Crinkle*	cc = &CRINKLE_crinkle[hnd];

	int	size = sizeof(CRINKLE_Crinkle) + cc->num_points * sizeof(CRINKLE_Point) + cc->num_faces * sizeof(CRINKLE_Face);

	UBYTE*	buffer = new UBYTE[size];
	ASSERT(buffer);
	UBYTE*	bptr = buffer;

	memcpy(bptr, cc, sizeof(*cc));
	bptr += sizeof(*cc);

	CRINKLE_Point*	cp = cc->point;
	CRINKLE_Face*	cf = cc->face;

	for (int ii = 0; ii < cc->num_points; ii++)
	{
		memcpy(bptr, cp, sizeof(*cp));
		cp++;
		bptr += sizeof(*cp);
	}

	for (int ii = 0; ii < cc->num_faces; ii++)
	{
		memcpy(bptr, cf, sizeof(*cf));
		cf++;
		bptr += sizeof(*cf);
	}

	ASSERT(bptr - buffer == size);

	tclump->Write(buffer, size, id);
}
#endif //#ifndef TARGET_DC


float CRINKLE_light_x;
float CRINKLE_light_y;
float CRINKLE_light_z;

float CRINKLE_mul_x;
float CRINKLE_mul_y;
float CRINKLE_mul_recip_x;
float CRINKLE_mul_recip_y;

void CRINKLE_skew(float aspect, float lens)
{
	CRINKLE_mul_x       = aspect * lens;
	CRINKLE_mul_y       = lens;
	CRINKLE_mul_recip_x = 1.0F / CRINKLE_mul_x;
	CRINKLE_mul_recip_y = 1.0F / CRINKLE_mul_y;
}

void CRINKLE_light(float dx, float dy, float dz)
{
#ifdef TARGET_DC
	// USe the intrinsic one.
	float len     = dx*dx + dy*dy + dz*dz;
	float overlen = _InvSqrtA(len);
#else
	float len     = sqrt(dx*dx + dy*dy + dz*dz);
	float overlen = 1.0F / len;
#endif

	dx *= overlen;
	dy *= overlen;
	dz *= overlen;

	CRINKLE_light_x = dx;
	CRINKLE_light_y = dy;
	CRINKLE_light_z = dz;
}


void CRINKLE_do(
		CRINKLE_Handle crinkle,
		SLONG          page,
		float          extrude,
		POLY_Point    *pp[4],
		SLONG          flip)
{
	SLONG i;

	CRINKLE_Crinkle *cc;

	ASSERT(WITHIN(crinkle, 0, CRINKLE_crinkle_upto - 1));

	cc = &CRINKLE_crinkle[crinkle];
#ifndef	FINAL
#ifndef TARGET_DC
	if (Keys[KB_RSHIFT])
	{
		flip = TRUE;
	}
#endif
#endif
	if (flip)
	{
		#define PPSWAP(a,b) {POLY_Point *pp_spare = (a); (a) = (b); (b) = pp_spare;}

		PPSWAP(pp[0],pp[1]);
		PPSWAP(pp[2],pp[3]);
	}

	//
	// Un-warp viewspace...
	//

	pp[0]->x *= CRINKLE_mul_recip_x;
	pp[1]->x *= CRINKLE_mul_recip_x;
	pp[2]->x *= CRINKLE_mul_recip_x;
	pp[3]->x *= CRINKLE_mul_recip_x;

	pp[0]->y *= CRINKLE_mul_recip_y;
	pp[1]->y *= CRINKLE_mul_recip_y;
	pp[2]->y *= CRINKLE_mul_recip_y;
	pp[3]->y *= CRINKLE_mul_recip_y;

	//
	// The base vectors of the crinkle.
	//

	float ox = pp[0]->x;
	float oy = pp[0]->y;
	float oz = pp[0]->z;
	float ou = pp[0]->u;
	float ov = pp[0]->v;

	float ax = pp[1]->x - ox;
	float ay = pp[1]->y - oy;
	float az = pp[1]->z - oz;
	float au = pp[1]->u - ou;
	float av = pp[1]->v - ov;

	float bx = pp[2]->x - ox;
	float by = pp[2]->y - oy;
	float bz = pp[2]->z - oz;
	float bu = pp[2]->u - ou;
	float bv = pp[2]->v - ov;

	float cx = (ay*bz - az*by) * (1.0F / 256.0F);
	float cy = (az*bx - ax*bz) * (1.0F / 256.0F);
	float cz = (ax*by - ay*bx) * (1.0F / 256.0F);

#ifdef TARGET_DC
	float len     = (cx*cx + cy*cy + cz*cz);
	float overlen = 0.05F * _InvSqrtA ( len );
#else
	float len     = sqrt(cx*cx + cy*cy + cz*cz);
	float overlen = 0.05F / len;
#endif

	if (flip)
	{
		overlen = -overlen;
	}

	SLONG pr[4];
	SLONG pg[4];
	SLONG pb[4];
	SLONG pa[4];

	for (i = 0; i < 4; i++)
	{
		pa[i] = (pp[i]->colour >> 24) & 0xff;
		pr[i] = (pp[i]->colour >> 16) & 0xff;
		pg[i] = (pp[i]->colour >>  8) & 0xff;
		pb[i] = (pp[i]->colour >>  0) & 0xff;
	}

	cx *= overlen;
	cy *= overlen;
	cz *= overlen;

	//
	// Warp viewspace again...
	//

	pp[0]->x *= CRINKLE_mul_x;
	pp[1]->x *= CRINKLE_mul_x;
	pp[2]->x *= CRINKLE_mul_x;
	pp[3]->x *= CRINKLE_mul_x;

	pp[0]->y *= CRINKLE_mul_y;
	pp[1]->y *= CRINKLE_mul_y;
	pp[2]->y *= CRINKLE_mul_y;
	pp[3]->y *= CRINKLE_mul_y;

	//
	// Transform the points.
	//

	float x;
	float y;
	float z;

	float u;
	float v;

	SLONG r;
	SLONG g;
	SLONG b;
	SLONG a;

	CRINKLE_Point *cp;
	POLY_Point    *pt;

	ASSERT(WITHIN(cc->num_points, 4, CRINKLE_MAX_POINTS_PER_CRINKLE));

	for (i = 0; i < cc->num_points; i++)
	{
		cp = &cc->point[i];
		pt = &CRINKLE_pp[i];

		pt->x = ox + cp->vec1 * ax + cp->vec2 * bx + cp->vec3 * cx * extrude;
		pt->y = oy + cp->vec1 * ay + cp->vec2 * by + cp->vec3 * cy * extrude;
		pt->z = oz + cp->vec1 * az + cp->vec2 * bz + cp->vec3 * cz * extrude;
		pt->u = ou + cp->vec1 * au + cp->vec2 * bu;
		pt->v = ov + cp->vec1 * av + cp->vec2 * bv;

		r = pr[0] * cp->c[0] + pr[1] * cp->c[1] + pr[2] * cp->c[2] + pr[3] * cp->c[3] >> 7;
		g = pg[0] * cp->c[0] + pg[1] * cp->c[1] + pg[2] * cp->c[2] + pg[3] * cp->c[3] >> 7;
		b = pb[0] * cp->c[0] + pb[1] * cp->c[1] + pb[2] * cp->c[2] + pb[3] * cp->c[3] >> 7;
		a = pa[0] * cp->c[0] + pa[1] * cp->c[1] + pa[2] * cp->c[2] + pa[3] * cp->c[3] >> 7;

		ASSERT(WITHIN(r, 0, 255));
		ASSERT(WITHIN(g, 0, 255));
		ASSERT(WITHIN(b, 0, 255));
		ASSERT(WITHIN(a, 0, 255));

#ifdef TARGET_DC
		pt->specular = 0x00000000;
		pt->colour   = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
#else
		pt->colour   = (a << 24) | (r << 16) | (g << 8) | (b << 0);
		pt->specular = 0xff000000;
#endif


		// Warp viewspace...

		pt->x *= CRINKLE_mul_x;
		pt->y *= CRINKLE_mul_y;

		POLY_transform_from_view_space(pt);
	}

	//
	// The faces.
	//

	POLY_Point   *tri[3];
	CRINKLE_Face *cf;

	for (i = 0; i < cc->num_faces; i++)
	{
		cf = &cc->face[i];

		if (flip)
		{
			tri[0] = &CRINKLE_pp[cf->point[0]];
			tri[1] = &CRINKLE_pp[cf->point[2]];
			tri[2] = &CRINKLE_pp[cf->point[1]];
		}
		else
		{
			tri[0] = &CRINKLE_pp[cf->point[0]];
			tri[1] = &CRINKLE_pp[cf->point[1]];
			tri[2] = &CRINKLE_pp[cf->point[2]];
		}

		if (POLY_valid_triangle(tri))
		{
			//
			// A face that isn't extruded doesn't have it lighting changed...
			//

			if (fabsf(cc->point[cf->point[0]].vec3) + fabsf(cc->point[cf->point[1]].vec3) + fabsf(cc->point[cf->point[2]].vec3) > 0.001F)
			{
				//
				// Find the normal :(
				//

				float ax = tri[1]->x - tri[0]->x;
				float ay = tri[1]->y - tri[0]->y;
				float az = tri[1]->z - tri[0]->z;

				float bx = tri[2]->x - tri[0]->x;
				float by = tri[2]->y - tri[0]->y;
				float bz = tri[2]->z - tri[0]->z;

				float nx = ay*bz - az*by;
				float ny = az*bx - ax*bz;
				float nz = ax*by - ay*bx;

#ifdef TARGET_DC
				float len     = (nx*nx + ny*ny + nz*nz);
				float overlen = _InvSqrtA ( len );
#else
				float len     = sqrt(nx*nx + ny*ny + nz*nz);
				float overlen = 1.0F / len;
#endif

				nx *= overlen;
				ny *= overlen;
				nz *= overlen;

				//
				// Work out how we vary the lighting of the crinkle because of
				// the light vector.
				//

				float dprod   = nx*CRINKLE_light_x + ny*CRINKLE_light_y + nz*CRINKLE_light_z;
				float dbright = dprod * extrude;
				SLONG drgb    = dbright * 64.0F;

				//
				// Backup the old lighting.
				//

				ULONG c0 = tri[0]->colour;
				ULONG c1 = tri[1]->colour;
				ULONG c2 = tri[2]->colour;

				SLONG r;
				SLONG g;
				SLONG b;
				SLONG a;

				//
				// Each colour in turn...
				//

				r = ((c0 >> 16) & 0xff) + drgb;
				g = ((c0 >>  8) & 0xff) + drgb;
				b = ((c0 >>  0) & 0xff) + drgb;
				a = ((c0 >> 24) & 0xff) + drgb;

				SATURATE(r, 0, 255);
				SATURATE(g, 0, 255);
				SATURATE(b, 0, 255);
				SATURATE(a, 0, 255);

#ifdef TARGET_DC
				tri[0]->colour = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
#else
				tri[0]->colour = (a << 24) | (r << 16) | (g << 8) | (b << 0);
#endif

				r = ((c1 >> 16) & 0xff) + drgb;
				g = ((c1 >>  8) & 0xff) + drgb;
				b = ((c1 >>  0) & 0xff) + drgb;
				a = ((c1 >> 24) & 0xff) + drgb;

				SATURATE(r, 0, 255);
				SATURATE(g, 0, 255);
				SATURATE(b, 0, 255);
				SATURATE(a, 0, 255);

#ifdef TARGET_DC
				tri[1]->colour = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
#else
				tri[1]->colour = (a << 24) | (r << 16) | (g << 8) | (b << 0);
#endif

				r = ((c2 >> 16) & 0xff) + drgb;
				g = ((c2 >>  8) & 0xff) + drgb;
				b = ((c2 >>  0) & 0xff) + drgb;
				a = ((c2 >> 24) & 0xff) + drgb;

				SATURATE(r, 0, 255);
				SATURATE(g, 0, 255);
				SATURATE(b, 0, 255);
				SATURATE(a, 0, 255);

#ifdef TARGET_DC
				tri[2]->colour = (0xff << 24) | (r << 16) | (g << 8) | (b << 0);
#else
				tri[2]->colour = (a << 24) | (r << 16) | (g << 8) | (b << 0);
#endif

				//
				// Draw the triangle.
				//

				POLY_add_triangle(tri, page, TRUE);

				//
				// Restore the old colours.
				//

				tri[0]->colour = c0;
				tri[1]->colour = c1;
				tri[2]->colour = c2;
			}
			else
			{
				POLY_add_triangle(tri, page, TRUE);
			}
		}
	}
}


void CRINKLE_project(
		CRINKLE_Handle crinkle,
		float          extrude,
		SVector_F      pp[4],
		SLONG          flip)
{
	SLONG i;

	CRINKLE_Crinkle *cc;

	ASSERT(WITHIN(crinkle, 0, CRINKLE_crinkle_upto - 1));

	cc = &CRINKLE_crinkle[crinkle];

#ifndef TARGET_DC
	if (Keys[KB_RSHIFT])
	{
		flip = TRUE;
	}
#endif

	if (flip)
	{
		#define SVSWAP(a,b) {SVector_F sv_spare = (a); (a) = (b); (b) = sv_spare;}

		SVSWAP(pp[0],pp[1]);
		SVSWAP(pp[2],pp[3]);
	}

	//
	// The base vectors of the crinkle.
	//

	float ox = pp[0].X;
	float oy = pp[0].Y;
	float oz = pp[0].Z;

	float ax = pp[1].X - ox;
	float ay = pp[1].Y - oy;
	float az = pp[1].Z - oz;

	float bx = pp[2].X - ox;
	float by = pp[2].Y - oy;
	float bz = pp[2].Z - oz;

	float cx = (ay*bz - az*by) * (1.0F / 256.0F);
	float cy = (az*bx - ax*bz) * (1.0F / 256.0F);
	float cz = (ax*by - ay*bx) * (1.0F / 256.0F);

#ifdef TARGET_DC
	float len     = (cx*cx + cy*cy + cz*cz);
	float overlen = 0.05F * _InvSqrtA ( len );
#else
	float len     = sqrt(cx*cx + cy*cy + cz*cz);
	float overlen = 0.05F / len;
#endif

	if (flip)
	{
		overlen = -overlen;
	}

	cx *= overlen;
	cy *= overlen;
	cz *= overlen;

	//
	// Find the points.
	//

	float x;
	float y;
	float z;

	float u;
	float v;

	SLONG r;
	SLONG g;
	SLONG b;
	SLONG a;

	CRINKLE_Point *cp;
	POLY_Point    *pt;

	ASSERT(WITHIN(cc->num_points, 4, CRINKLE_MAX_POINTS_PER_CRINKLE));

	for (i = 0; i < cc->num_points; i++)
	{
		cp = &cc->point[i];
		pt = &CRINKLE_pp[i];

		pt->x = ox + cp->vec1 * ax + cp->vec2 * bx + cp->vec3 * cx * extrude;
		pt->y = oy + cp->vec1 * ay + cp->vec2 * by + cp->vec3 * cy * extrude;
		pt->z = oz + cp->vec1 * az + cp->vec2 * bz + cp->vec3 * cz * extrude;
	}

	//
	// The faces.
	//

	POLY_Point   *tri[3];
	CRINKLE_Face *cf;

	for (i = 0; i < cc->num_faces; i++)
	{
		cf = &cc->face[i];

		if (flip)
		{
			tri[0] = &CRINKLE_pp[cf->point[0]];
			tri[1] = &CRINKLE_pp[cf->point[2]];
			tri[2] = &CRINKLE_pp[cf->point[1]];
		}
		else
		{
			tri[0] = &CRINKLE_pp[cf->point[0]];
			tri[1] = &CRINKLE_pp[cf->point[1]];
			tri[2] = &CRINKLE_pp[cf->point[2]];
		}

		AENG_world_line(
			tri[0]->x,
			tri[0]->y,
			tri[0]->z,
			4,
			0xffffff,
			tri[1]->x,
			tri[1]->y,
			tri[1]->z,
			3,
			0xffffff,
			TRUE);

		AENG_world_line(
			tri[1]->x,
			tri[1]->y,
			tri[1]->z,
			4,
			0xffffff,
			tri[2]->x,
			tri[2]->y,
			tri[2]->z,
			3,
			0xffffff,
			TRUE);

		AENG_world_line(
			tri[2]->x,
			tri[2]->y,
			tri[2]->z,
			4,
			0xffffff,
			tri[0]->x,
			tri[0]->y,
			tri[0]->z,
			3,
			0xffffff,
			TRUE);
	}
}

