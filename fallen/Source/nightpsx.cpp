//
// Cached lighting.
//

#include "game.h"
#include "heap.h"
#include "night.h"
#include "ob.h"
#include "pap.h"
#include "fmatrix.h"
#include "heap.h"
#include "shadow.h"
#include "supermap.h"
#include "ware.h"
#include "c:\fallen\ledit\headers\ed.h"

#ifdef	PSX


// file access stuff

//
// PSX include
//
#include "libsn.h"
extern	void			TEXTURE_choose_set(SLONG number);

#define	MAX_PATH	128
#define	FILE	SLONG

#define	MFFileHandle	SLONG
#define	FILE_OPEN_ERROR	(-1)
#define	SEEK_MODE_CURRENT	(1)

extern	SLONG	SpecialOpen(CBYTE *name);
extern	SLONG	SpecialRead(SLONG handle,UBYTE *ptr,SLONG s1);
extern	SLONG	SpecialSeek(SLONG handle,SLONG mode,SLONG size);
extern	SLONG	SpecialClose(SLONG handle);

#define	FileOpen(x)		SpecialOpen(x)
#define	FileClose(x)	SpecialClose(x)
#define	FileCreate(x,y)	ASSERT(0)
#define	FileRead(h,a,s) SpecialRead(h,(UBYTE*)a,s)
#define	FileWrite(h,a,s) ASSERT(0)
#define	FileSeek(h,m,o) SpecialSeek(h,m,o)

#define	MF_Fopen(x,y)		SpecialOpen(x)
#define	MF_Fclose(x)		SpecialClose(x)
#define	fread(a,s1,s2,h) SpecialRead(h,(UBYTE*)a,s1*s2)



//
// The static lights.
// 
/*
typedef struct
{
	SWORD y;
	UBYTE x;
	UBYTE z;
	SBYTE red;
	SBYTE green;
	SBYTE blue;
	UBYTE radius;

} NIGHT_Slight;
*/
//#define NIGHT_MAX_SLIGHTS 128

NIGHT_Slight *NIGHT_slight;//[NIGHT_MAX_SLIGHTS];
SLONG        NIGHT_slight_upto;

//
// The mapwho for static lights.
//
/*
typedef struct
{
	UBYTE index;
	UBYTE number;

} NIGHT_Smap;
*/
//typedef NIGHT_Smap NIGHT_smap_2d[PAP_SIZE_LO];//[PAP_SIZE_LO];

NIGHT_Smap_2d *NIGHT_smap;

UWORD	floor_psx_col[PAP_SIZE_HI][PAP_SIZE_HI];
UBYTE	floor_lum[32][32];
SBYTE	lum_off_x;
SBYTE	lum_off_z;

//
// The dynamic lights.
//
/*
typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;
	UBYTE red;
	UBYTE green;
	UBYTE blue;
	UBYTE radius;
	UBYTE type;
	UBYTE next;

} NIGHT_Dlight;
*/
#define NIGHT_MAX_DLIGHTS 64

NIGHT_Dlight *NIGHT_dlight;//[NIGHT_MAX_DLIGHTS];
UBYTE        NIGHT_dlight_free;
UBYTE        NIGHT_dlight_used;

//
// 1D mapwho for dynamic lights.
//

UBYTE NIGHT_dlight_mapwho[PAP_SIZE_LO];


//
// The cached lighting.
//

//NIGHT_Square  NIGHT_square[NIGHT_MAX_SQUARES];
//UBYTE         NIGHT_square_free;
//SLONG         NIGHT_square_num_used;
//UBYTE         NIGHT_cache[PAP_SIZE_LO][PAP_SIZE_LO];

NIGHT_Dfcache NIGHT_dfcache[NIGHT_MAX_DFCACHES];

ULONG        NIGHT_flag;
UBYTE        NIGHT_lampost_radius;
SBYTE        NIGHT_lampost_red;
SBYTE        NIGHT_lampost_green;
SBYTE        NIGHT_lampost_blue;
NIGHT_Colour NIGHT_sky_colour;


// ========================================================
//
// AMBIENT LIGHT
//
// ========================================================

ULONG        NIGHT_amb_d3d_colour;
ULONG        NIGHT_amb_d3d_specular;
SLONG        NIGHT_amb_red;
SLONG        NIGHT_amb_green;
SLONG        NIGHT_amb_blue;
SLONG        NIGHT_amb_norm_x;
SLONG        NIGHT_amb_norm_y;
SLONG        NIGHT_amb_norm_z;
UBYTE         NIGHT_dfcache_free;
UBYTE		  NIGHT_dfcache_used;


//
// The normal should be normalised to 256.
//

void NIGHT_ambient(
		UBYTE red,
		UBYTE green,
		UBYTE blue,
		SLONG norm_x,
		SLONG norm_y,
		SLONG norm_z)
{
	NIGHT_Colour amb_colour;

	NIGHT_amb_red    = (red);
	NIGHT_amb_green  = (green);
	NIGHT_amb_blue   = (blue);

	NIGHT_amb_norm_x = norm_x;
	NIGHT_amb_norm_y = norm_y;
	NIGHT_amb_norm_z = norm_z;

//	SATURATE(NIGHT_amb_red,32,192);
//	SATURATE(NIGHT_amb_green,32,192);
//	SATURATE(NIGHT_amb_blue,32,192);

//	SATURATE(NIGHT_amb_red,32,84);//,192);
//	SATURATE(NIGHT_amb_green,32,84);
//	SATURATE(NIGHT_amb_blue,32,84);

//	SATURATE(NIGHT_amb_red,16,192);
//	SATURATE(NIGHT_amb_green,16,192);
//	SATURATE(NIGHT_amb_blue,16,192);

	amb_colour.red   = NIGHT_amb_red;
	amb_colour.green = NIGHT_amb_green;
	amb_colour.blue  = NIGHT_amb_blue;

	NIGHT_get_d3d_colour(
		amb_colour,
	   &NIGHT_amb_d3d_colour,
	   &NIGHT_amb_d3d_specular);
}

NIGHT_Colour NIGHT_ambient_at_point(
				SLONG norm_x,
				SLONG norm_y,
				SLONG norm_z)
{
	NIGHT_Colour ans;

	SLONG dprod;

	dprod =
		norm_x * NIGHT_amb_norm_x +
		norm_y * NIGHT_amb_norm_y + 
		norm_z * NIGHT_amb_norm_z >> 10;

	dprod += 192; //what the buggery bollox
//	dprod += 128; //what the buggery bollox
/*
	set_red(ans.col,NIGHT_amb_red * dprod >> 8);
	or_green(ans.green , NIGHT_amb_green * dprod >> 8);
	or_blue(ans.blue  , NIGHT_amb_blue * dprod >> 8);
*/

	ans.red = NIGHT_amb_red * dprod >> 8;
	ans.green = NIGHT_amb_green * dprod >> 8;
	ans.blue  = NIGHT_amb_blue * dprod >> 8;

	return ans;
}


// ========================================================
//
// STATIC LIGHTS
//
// ========================================================
#ifndef	PSX
void NIGHT_slight_compress()
{
	
	SLONG x;
	SLONG z;

	NIGHT_Smap *ns;

	NIGHT_Slight *comp;
	SLONG         comp_upto;

	comp      = (NIGHT_Slight *) MemAlloc(sizeof(NIGHT_slight));
	comp_upto = 0;

	if (comp)
	{
		for (x = 0; x < PAP_SIZE_LO; x++)
		for (z = 0; z < PAP_SIZE_LO; z++)
		{
			ns = &NIGHT_smap[x][z];

			ASSERT(comp_upto + ns->number <= NIGHT_MAX_SLIGHTS);

			memcpy(&comp[comp_upto], &NIGHT_slight[ns->index], sizeof(NIGHT_Slight) * ns->number);

			ns->index  = comp_upto;
			comp_upto += ns->number;
		}

		//
		// Copy the compressed array over the old array.
		//

		memcpy(NIGHT_slight, comp, sizeof(NIGHT_slight));
		NIGHT_slight_upto = comp_upto;

		MemFree(comp);
	}
}
#endif
void NIGHT_slight_init()
{
	memset((UBYTE*)NIGHT_smap, 0, sizeof(NIGHT_smap));

	NIGHT_slight_upto = 0;
}
#ifndef	PSX
SLONG NIGHT_slight_create(
		SLONG x,
		SLONG y,
		SLONG z,
		UBYTE radius,
		SBYTE red,
		SBYTE green,
		SBYTE blue)
{
	NIGHT_Smap   *ns;
	NIGHT_Slight *nsl;

	SLONG map_x = x >> PAP_SHIFT_LO;
	SLONG map_z = z >> PAP_SHIFT_LO;

	ASSERT(WITHIN(map_x, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(map_z, 0, PAP_SIZE_LO - 1));

	ns = &NIGHT_smap[map_x][map_z];

	if (ns->index + ns->number == NIGHT_slight_upto && NIGHT_slight_upto < NIGHT_MAX_SLIGHTS)
	{
		//
		// This squares static lights are already at the end of the array.
		// And there enough memory to add another one...
		//
	}
	else
	{
		//
		// We must copy this squares lights to the end of the array. Is
		// there enough room?
		//

		if (NIGHT_slight_upto + ns->number + 1 > NIGHT_MAX_SLIGHTS)
		{
			//
			// Not enough room- try compressing the static lights.
			//
			
			NIGHT_slight_compress();

			if (NIGHT_slight_upto + ns->number + 1 > NIGHT_MAX_SLIGHTS)
			{
				//
				// Still not enough room!
				//

				return FALSE;
			}
		}

		//
		// Copy the squares to the end of the array.
		//

		memcpy(&NIGHT_slight[NIGHT_slight_upto], &NIGHT_slight[ns->index], sizeof(NIGHT_Slight) * ns->number);

		ns->index          = NIGHT_slight_upto;
		NIGHT_slight_upto += ns->number;
	}

	//
	// Put the new light at the end of the array.
	//

	ASSERT(WITHIN(NIGHT_slight_upto, 0, NIGHT_MAX_SLIGHTS - 1));

	nsl = &NIGHT_slight[NIGHT_slight_upto];

	//
	// (x,z) of the light is relative to the mapsquare the light is in.
	//

	nsl->x      = x >> 2;
	nsl->y      = y;
	nsl->z      = z >> 2;
	nsl->red    = red   >> 1;
	nsl->green  = green >> 1;
	nsl->blue   = blue  >> 1;
	nsl->radius = radius;

	ns->number        += 1;
	NIGHT_slight_upto += 1;

	return TRUE;
}

void  NIGHT_slight_delete(
		SLONG x,
		SLONG y,
		SLONG z,
		UBYTE radius,
		SBYTE red,
		SBYTE green,
		SBYTE blue)
{
	SLONG i;

	SLONG mx;
	SLONG mz;

	SLONG map_x;
	SLONG map_z;

	SLONG lx;
	SLONG lz;

	NIGHT_Smap   *ns;
	NIGHT_Slight *nsl;

	mx = x >> PAP_SHIFT_LO;
	mz = z >> PAP_SHIFT_LO;

	ASSERT(WITHIN(mx, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(mz, 0, PAP_SIZE_LO - 1));

	ns = &NIGHT_smap[mx][mz];

	map_x = mx << PAP_SHIFT_LO;
	map_z = mz << PAP_SHIFT_LO;

	for (i = 0; i < ns->number; i++)
	{
		ASSERT(WITHIN(ns->index + i, 0, NIGHT_MAX_SLIGHTS - 1));

		nsl = &NIGHT_slight[ns->index + i];

		if ((nsl->x << 2) + map_x == (x & ~3)     &&
			(nsl->z << 2) + map_z == (z & ~3)     &&
			 nsl->y               ==  y           &&
			 nsl->red             == (red   >> 1) &&
			 nsl->green           == (green >> 1) &&
			 nsl->blue            == (blue  >> 1) &&
			 nsl->radius          ==  radius)
		{
			//
			// This is the light we need to delete.
			//

			NIGHT_slight[ns->index + i] = NIGHT_slight[ns->index + ns->number - 1];

			ns->number -= 1;

			return;
		}
	}

	//
	// Trying to delete a light that doesn't exist.
	//

	ASSERT(0);
	
	return;
}

void  NIGHT_slight_delete_all()
{
	memset((UBYTE*)NIGHT_smap, 0, sizeof(NIGHT_smap));

	NIGHT_slight_upto = 0;
}
#endif

//
// Lampost lights
//

typedef struct
{
	UWORD x;
	SWORD y;
	UWORD z;

} NIGHT_Llight;

#define NIGHT_MAX_LLIGHTS 16

NIGHT_Llight NIGHT_llight[NIGHT_MAX_LLIGHTS];
SLONG        NIGHT_llight_upto;


//
// Lights the given lo-res mapsquare.
//

typedef struct
{
	SLONG nx;
	SLONG ny;
	SLONG nz;
	SLONG height;

} NIGHT_Precalc;

void NIGHT_get_lampost(SLONG x,SLONG y,SLONG z,SLONG *r,SLONG *g,SLONG *b);

void NIGHT_light_the_map(void)
{
	SLONG lo_map_x;
	SLONG lo_map_z;
	SLONG i;

	SLONG x;
	SLONG z;

	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG mx;
	SLONG mz;

	SLONG map_x;
	SLONG map_z;

	SLONG hi_map_x;
	SLONG hi_map_z;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG h0;
	SLONG h1;
	SLONG h2;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;

	SLONG lx;
	SLONG ly;
	SLONG lz;
	SLONG lradius;

	SLONG red;
	SLONG green;
	SLONG blue;
	
	SLONG dprod;
	SLONG bright;

	NIGHT_Smap   *ns;
	NIGHT_Slight *nsl;
	
	//
	// Allocate a 2d array on the scratchpad.
	//

	NIGHT_Precalc precalc;


	//
	// Random lighting fluctuations over the map...
	//

	ULONG seed = 0;//hi_map_x + (hi_map_z << 5);

	seed *= 328573;
	seed += 123456789;

//	ASSERT(WITHIN(lo_map_x, 0, PAP_SIZE_LO - 1));
//	ASSERT(WITHIN(lo_map_z, 0, PAP_SIZE_LO - 1));



	for (z = 0; z < PAP_SIZE_HI; z++)
	for (x = 0; x < PAP_SIZE_HI; x++)
	{
		lo_map_x=x>>2;
		lo_map_z=z>>2;

		mx = x;
		mz = z;

		//
		// Work out the normal at this point.
		//

		find_face_for_this_pos(mx<<8,INFINITY,mz<<8,&h0,0,1);
		find_face_for_this_pos((mx+1)<<8,INFINITY,mz<<8,&h1,0,1);
		find_face_for_this_pos((mx-1)<<8,INFINITY,mz<<8,&h2,0,1);
//		h0 = PAP_calc_height_at_point(mx    , mz);
//		h1 = PAP_calc_height_at_point(mx + 1, mz);
//		h2 = PAP_calc_height_at_point(mx - 1, mz);

		nx   = h2 - h0;
		nx  -= h1 - h0;
		nx <<= 2;
		//nx  += (seed >> 9) & 0x3f;
		//nx  -= 0x1f;

		//seed *= 328573;
		//seed += 123456789;

		find_face_for_this_pos(mx<<8,INFINITY,(mz+1)<<8,&h1,0,1);
		find_face_for_this_pos(mx<<8,INFINITY,(mz-1)<<8,&h2,0,1);

//		h1 = PAP_calc_height_at_point(mx, mz + 1);
//		h2 = PAP_calc_height_at_point(mx, mz - 1);

		nz   = h2 - h0;
		nz  -= h1 - h0;
		nz <<= 2;
		//nz  += (seed >> 9) & 0x3f;
		//nz  -= 0x1f;

		//seed *= 328573;
		//seed += 123456789;

		if (abs(nx) > 120) {nx = (nx > 0) ? 100 : -100;}
		if (abs(nz) > 120) {nz = (nz > 0) ? 100 : -100;}

		ny = 256 - QDIST2(abs(nx),abs(nz));

		//
		// Store the info.
		//

		precalc.nx     = nx;
		precalc.ny     = ny;
		precalc.nz     = nz;
		precalc.height = h0;

		//
		// Light the point from the ambient light.
		//

		dprod =
			NIGHT_amb_norm_x * nx + 
			NIGHT_amb_norm_y * ny + 
			NIGHT_amb_norm_z * nz;

		dprod = -dprod;

		SATURATE(dprod, 0, 65536);

		dprod >>= 9;
		dprod  += 128;

		red   = NIGHT_amb_red* dprod >> 7;
		green = NIGHT_amb_green* dprod >> 7;
		blue  = NIGHT_amb_blue* dprod >> 7;

		if(0)
		if (NIGHT_flag & NIGHT_FLAG_DARKEN_BUILDING_POINTS)
		{
			//
			// Darken points that lie on the edges of buildings.
			//

			for (dx = -1; dx <= 0; dx++)
			for (dz = -1; dz <= 0; dz++)
			{
				px = mx + dx;
				pz = mz + dz;

				if (px >= 0 && pz >= 0)
				{
					ASSERT(WITHIN(px, 0, PAP_SIZE_HI - 1));
					ASSERT(WITHIN(pz, 0, PAP_SIZE_HI - 1));

					if (PAP_2HI(px,pz).Flags & PAP_FLAG_HIDDEN)
					{
						#define NIGHT_DARKEN_GROUND 16

						if (red   <= NIGHT_DARKEN_GROUND) {red   = 0;} else {red   -= NIGHT_DARKEN_GROUND;}
						if (green <= NIGHT_DARKEN_GROUND) {green = 0;} else {green -= NIGHT_DARKEN_GROUND;}
						if (blue  <= NIGHT_DARKEN_GROUND) {blue  = 0;} else {blue  -= NIGHT_DARKEN_GROUND;}
						
						goto darkened_this_point_once;
					}
				}
			}
		}

	  darkened_this_point_once:;

	//	col_upto += 1;
//		SATURATE(red,0,255);
//		SATURATE(green,0,255);
//		SATURATE(blue,0,255);

//		floor_psx_col[x][z]=((red>>2)<<15) ||((green>>3)<<10)||((blue>>3)<<0);
//	close curly was here

	//
	// Light the map from the static lights and the lamposts.
	//

//#ifdef	NO_LIGHTS
		mx1 = lo_map_x - 1;
		mz1 = lo_map_z - 1;

		mx2 = lo_map_x + 1;
		mz2 = lo_map_z + 1;

		if (mx1 < 0) {mx1 = 0;}
		if (mz1 < 0) {mz1 = 0;}

		if (mx2 >= PAP_SIZE_LO - 1) {mx2 = PAP_SIZE_LO - 1;}
		if (mz2 >= PAP_SIZE_LO - 1) {mz2 = PAP_SIZE_LO - 1;}

		for (mx = mx1; mx <= mx2; mx++)
		for (mz = mz1; mz <= mz2; mz++)
		{
			map_x = mx << PAP_SHIFT_LO;
			map_z = mz << PAP_SHIFT_LO;

			ASSERT(WITHIN(mx, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(mz, 0, PAP_SIZE_LO - 1));

			ns = &NIGHT_smap[mx][mz];

			for (i = 0; i < ns->number; i++)
			{
				ASSERT(WITHIN(ns->index + i, 0, NIGHT_slight_upto - 1));

				nsl = &NIGHT_slight[ns->index + i];

				lx = (nsl->x << 2) + map_x;
				ly =  nsl->y;
				lz = (nsl->z << 2) + map_z;

				lradius = nsl->radius << 2;

				//
				// Work out the range of the map affected by the light
				//

				x1 = lx - lradius >> 8;
				z1 = lz - lradius >> 8;

				x2 = lx + lradius >> 8;
				z2 = lz + lradius >> 8;


				if(x<x1) continue;
				if(z<z1) continue;
				if(x>x2) continue;
				if(z>z2) continue;

				//
				// Light the affected region.
				//

	//			for (x = x1; x <= x2; x++)
	//			for (z = z1; z <= z2; z++)
				{
					//
					// The position of this point.
					//

					px = x<<PAP_SHIFT_HI;// + hi_map_x << 8;
					pz = z<<PAP_SHIFT_HI;// + hi_map_z << 8;

					py = precalc.height;

					//
					// How far is this point from the light?
					//

					dx = lx - px;
					dy = ly - py;
					dz = lz - pz;

					dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

					if (dist >= lradius)
					{
						//
						// Out of range.
						//
					}
					else
					{
						//
						// The normal of this point.
						//

						nx = precalc.nx;
						ny = precalc.ny;
						nz = precalc.nz;

						dprod = dx*nx + dy*ny + dz*nx;

						if (dprod <= 0)
						{
							//
							// Facing away from the light.
							//
						}
						else
						{
							dprod /= dist;
							bright = 256 - (dist << 8) / lradius;
							bright = bright * dprod >> 8;

//							red   = colour[x + z * PAP_BLOCKS].red;
//							green = colour[x + z * PAP_BLOCKS].green;
//							blue  = colour[x + z * PAP_BLOCKS].blue;

							red   += nsl->red   * bright >> 7;
							green += nsl->green * bright >> 7;
							blue  += nsl->blue  * bright >> 7;

						}
					}
				}
			}
		}
//#endif
//		red<<=2;
//		green<<=2;
//		blue<<=2;
		
		{	SLONG r,g,b,h0;
			h0 = PAP_calc_height_at_point(x,z);

			NIGHT_get_lampost(x<<8,h0,z<<8,&r,&g,&b);
			red+=r;
			green+=g;
			blue+=b;

		}
		SATURATE(red,   0, 255);
		SATURATE(green, 0, 255);
		SATURATE(blue,  0, 255);
		floor_psx_col[x][z]=((red>>2)<<10) |((green>>3)<<5)|((blue>>3)<<0);

	}
#ifdef	MORE_LIGHTS
	for (i = 0; i < NIGHT_llight_upto; i++)
	{
		//
		// There is a light under this prim... but where?
		//

		lx = NIGHT_llight[i].x;
		ly = NIGHT_llight[i].y;
		lz = NIGHT_llight[i].z;

		lradius = NIGHT_lampost_radius << 2;

		//
		// Work out the range of the map affected by the light
		//

		x1 = lx - lradius >> 8;
		z1 = lz - lradius >> 8;

		x2 = lx + lradius >> 8;
		z2 = lz + lradius >> 8;

		x1 -= hi_map_x;
		z1 -= hi_map_z;

		x2 -= hi_map_x;
		z2 -= hi_map_z;

		//
		// If the light does not affect any of our map points,
		// then ignore it.
		//

		if (x1 > PAP_BLOCKS - 1) {continue;}
		if (z1 > PAP_BLOCKS - 1) {continue;}

		if (x2 < 0) {continue;}
		if (z2 < 0) {continue;}

		//
		// Clip to our map points.
		//

		if (x1 < 0) {x1 = 0;}
		if (z1 < 0) {z1 = 0;}

		if (x2 > PAP_BLOCKS - 1) {x2 = PAP_BLOCKS - 1;}
		if (z2 > PAP_BLOCKS - 1) {z2 = PAP_BLOCKS - 1;}

		//
		// Light the affected region.
		//

		for (x = x1; x <= x2; x++)
		for (z = z1; z <= z2; z++)
		{
			//
			// The position of this point.
			//

			px = x + hi_map_x << 8;
			pz = z + hi_map_z << 8;

			py = precalc[x][z].height;

			//
			// How far is this point from the light?
			//

			dx = lx - px;
			dy = ly - py;
			dz = lz - pz;

			dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

			if (dist >= lradius)
			{
				//
				// Out of range.
				//
			}
			else
			{
				//
				// The normal of this point.
				//

				nx = precalc[x][z].nx;
				ny = precalc[x][z].ny;
				nz = precalc[x][z].nz;

				dprod = dx*nx + dy*ny + dz*nx;

				if (dprod <= 0)
				{
					//
					// Facing away from the light.
					//
				}
				else
				{
					dprod /= dist;
					bright = 256 - (dist << 8) / lradius;
					bright = bright * dprod >> 8;

					red   = colour[x + z * PAP_BLOCKS].red;
					green = colour[x + z * PAP_BLOCKS].green;
					blue  = colour[x + z * PAP_BLOCKS].blue;

					red   += NIGHT_lampost_red   * bright >> 8;
					green += NIGHT_lampost_green * bright >> 8;
					blue  += NIGHT_lampost_blue  * bright >> 8;

					SATURATE(red,   0, 255);
					SATURATE(green, 0, 255);
					SATURATE(blue,  0, 255);

					colour[x + z * PAP_BLOCKS].red   = red;
					colour[x + z * PAP_BLOCKS].green = green;
					colour[x + z * PAP_BLOCKS].blue  = blue;
				}
			}
		}
	}
#endif
}

#ifndef PSX
//
// Lights the given prim.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG nx;
	SLONG ny;
	SLONG nz;
	
} NIGHT_Point;

#define NIGHT_MAX_POINTS (HEAP_PAD_SIZE / sizeof(NIGHT_Point))

void NIGHT_light_prim(
		SLONG prim,
		SLONG prim_x,
		SLONG prim_y,
		SLONG prim_z,
		SLONG prim_yaw,
		SLONG prim_pitch,
		SLONG prim_roll,
		NIGHT_Colour *colour)
{
	SLONG i;
	SLONG j;

	SLONG mx;
	SLONG mz;

	SLONG xmid;
	SLONG zmid;

	SLONG xmin, xmax;
	SLONG zmin, zmax;

	SLONG map_x;
	SLONG map_z;

	SLONG amb_x;
	SLONG amb_y;
	SLONG amb_z;

	SLONG lx;
	SLONG ly;
	SLONG lz;
	SLONG lradius;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG dprod;
	SLONG bright;

	SLONG red;
	SLONG green;
	SLONG blue;

	SLONG num_points;

	SLONG point;
	SLONG matrix[9];
	SLONG rotate;

	PrimObject *p_obj  = &prim_objects[prim];
	PrimInfo   *p_info =  get_prim_info(prim);

	NIGHT_Point *np;
	NIGHT_Point *np_base = (NIGHT_Point *) HEAP_pad;

	NIGHT_Smap   *ns;
	NIGHT_Slight *nsl;

	//
	// Quite a few prims won't have any rotation to them... I expect.
	//

	if (prim_yaw   == 0 &&
		prim_pitch == 0 &&
		prim_roll  == 0)
	{
		rotate = FALSE;
	}
	else
	{
		rotate = TRUE;

		FMATRIX_calc(
			matrix,
			prim_yaw,
			prim_pitch,
			prim_roll);
	}

	//
	// Rotate the ambient light into the space of the prim.
	//

	amb_x = NIGHT_amb_norm_x;
	amb_y = NIGHT_amb_norm_y;
	amb_z = NIGHT_amb_norm_z;

	if (rotate)
	{
		FMATRIX_MUL(
			matrix,
			amb_x,
			amb_y,
			amb_z);
	}

	//
	// Initialise all the point colours to the ambient light.
	//

	num_points = p_obj->EndPoint - p_obj->StartPoint;

	for (i = 0; i < num_points; i++)
	{
		dprod =
			amb_x * prim_normal[p_obj->StartPoint + i].X + 
			amb_y * prim_normal[p_obj->StartPoint + i].Y + 
			amb_z * prim_normal[p_obj->StartPoint + i].Z;

		dprod >>= 9;
		dprod  += 128;

		colour[i].red   = NIGHT_amb_red   * dprod >> 8;
		colour[i].green = NIGHT_amb_green * dprod >> 8;
		colour[i].blue  = NIGHT_amb_blue  * dprod >> 8;
	}

	//
	// The bounding box we search for lights effecting this prim.
	//

	xmid = prim_x >> PAP_SHIFT_LO;
	zmid = prim_z >> PAP_SHIFT_LO;

	ASSERT(WITHIN(xmid, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(zmid, 0, PAP_SIZE_LO - 1));

	xmin = xmid - 1;
	zmin = zmid - 1;

	xmax = xmid + 1;
	zmax = zmid + 1;

	if (xmin < 0) {xmin = 0;}
	if (zmin < 0) {zmin = 0;}
	
	if (xmax > PAP_SIZE_LO - 1) {xmax = PAP_SIZE_LO - 1;}
	if (zmax > PAP_SIZE_LO - 1) {zmax = PAP_SIZE_LO - 1;}

	for (mz = zmin; mz <= zmax; mz++)
	{
		for (mx = xmin; mx <= xmax; mx++)
		{
			ns = &NIGHT_smap[mx][mz];

			map_x = mx << PAP_SHIFT_LO;
			map_z = mz << PAP_SHIFT_LO;

			for (i = 0; i < ns->number; i++)
			{
				nsl = &NIGHT_slight[ns->index + i];

				//
				// Rotate the light into the space of the prim.
				//

				lx  = (nsl->x << 2) + map_x;
				ly  =  nsl->y;
				lz  = (nsl->z << 2) + map_z;

				lx -= prim_x;
				ly -= prim_y;
				lz -= prim_z;

				if (rotate)
				{
					FMATRIX_MUL(
						matrix,
						lx,
						ly,
						lz);
				}

				lradius = nsl->radius << 2;

				//
				// Is the light in range of the prim?
				//

				if (lx - lradius > p_info->maxx ||
					lx + lradius < p_info->minx ||
					ly - lradius > p_info->maxy ||
					ly + lradius < p_info->miny ||
					lz - lradius > p_info->maxz ||
					lz + lradius < p_info->minz)
				{
					//
					// We can ignore the light.
					//
				}
				else
				{
					for (j = 0; j < num_points; j++)
					{
						point = p_obj->StartPoint + j;

						dx = prim_points[point].X - lx;
						dy = prim_points[point].Y - ly;
						dz = prim_points[point].Z - lz;

						dprod =
							dx * prim_normal[point].X + 
							dy * prim_normal[point].Y + 
							dz * prim_normal[point].Z;

						if (dprod <= 0)
						{
							//
							// The point is facing away from the light.
							//
						}
						else
						{
							//
							// Distance from the light.
							//

							dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

							if (dist < lradius)
							{
								dprod /= dist;
								bright = 256 - (dist << 8) / lradius;
								bright = bright * dprod >> 8;

								red   = colour[j].red;
								green = colour[j].green;
								blue  = colour[j].blue;

								red   += nsl->red   * bright >> 8;
								green += nsl->green * bright >> 8;
								blue  += nsl->blue  * bright >> 8;

								SATURATE(red,   0, 255);
								SATURATE(green, 0, 255);
								SATURATE(blue,  0, 255);

								colour[j].red   = red;
								colour[j].green = green;
								colour[j].blue  = blue;
							}
						}
					}
				}
			}
		}

		//
		// Dynamic lights?
		//
	}

	//
	// Lampost lights.
	//

	for (i = 0; i < NIGHT_llight_upto; i++)
	{
		//
		// Rotate the light into the space of the prim.
		//

		lx  = NIGHT_llight[i].x;
		ly  = NIGHT_llight[i].y;
		lz  = NIGHT_llight[i].z;

		lx -= prim_x;
		ly -= prim_y;
		lz -= prim_z;

		if (rotate)
		{
			FMATRIX_MUL(
				matrix,
				lx,
				ly,
				lz);
		}

		lradius = NIGHT_lampost_radius << 2;

		//
		// Is the light in range of the prim?
		//

		if (lx - lradius > p_info->maxx ||
			lx + lradius < p_info->minx ||
			ly - lradius > p_info->maxy ||
			ly + lradius < p_info->miny ||
			lz - lradius > p_info->maxz ||
			lz + lradius < p_info->minz)
		{
			//
			// We can ignore the light.
			//
		}
		else
		{
			for (j = 0; j < num_points; j++)
			{
				point = p_obj->StartPoint + j;

				dx = prim_points[point].X - lx;
				dy = prim_points[point].Y - ly;
				dz = prim_points[point].Z - lz;

				dprod =
					dx * prim_normal[point].X + 
					dy * prim_normal[point].Y + 
					dz * prim_normal[point].Z;

				if (dprod <= 0)
				{
					//
					// The point is facing away from the light.
					//
				}
				else
				{
					//
					// Distance from the light.
					//

					dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

					if (dist < lradius)
					{
						dprod /= dist;
						bright = 256 - (dist << 8) / lradius;
						bright = bright * dprod >> 8;

						red   = colour[j].red;
						green = colour[j].green;
						blue  = colour[j].blue;

						red   += NIGHT_lampost_red   * bright >> 8;
						green += NIGHT_lampost_green * bright >> 8;
						blue  += NIGHT_lampost_blue  * bright >> 8;

						SATURATE(red,   0, 255);
						SATURATE(green, 0, 255);
						SATURATE(blue,  0, 255);

						colour[j].red   = red;
						colour[j].green = green;
						colour[j].blue  = blue;
					}
				}
			}
		}
	}
}

#endif

// ========================================================
// 
// DYNAMIC LIGHTS
//
// ========================================================

void  NIGHT_dlight_init()
{
}

UBYTE NIGHT_dlight_create(
		SLONG x,
		SLONG y,
		SLONG z,
		UBYTE radius,
		UBYTE red,
		UBYTE green,
		UBYTE blue)
{
	NIGHT_Dlight *ndl;
	UBYTE ans;

	if (NIGHT_dlight_free==0)
		return NULL;

	ndl= &NIGHT_dlight[NIGHT_dlight_free];
	ndl->x=x;
	ndl->y=y;
	ndl->z=z;
	ndl->radius=radius;
//	ndl->used=TRUE;

	ans=NIGHT_dlight_free;
	NIGHT_dlight_free=ndl->next;
	ndl->next=NIGHT_dlight_used;
	NIGHT_dlight_used=ans;
	
	return NIGHT_dlight_used;
}

void NIGHT_dlight_move(UBYTE dlight_index, SLONG x, SLONG y, SLONG z)
{

	NIGHT_Dlight *ndl;
	if(dlight_index)
	{
		ASSERT(WITHIN(dlight_index, 1, NIGHT_MAX_DLIGHTS - 1));

		ndl = &NIGHT_dlight[dlight_index];

		ndl->x = x;
		ndl->y = y;
		ndl->z = z;
	}

}

void NIGHT_dlight_colour(UBYTE dlight_index, UBYTE red, UBYTE green, UBYTE blue)
{
	/*
	NIGHT_Dlight *ndl;

	ASSERT(WITHIN(dlight_index, 1, NIGHT_MAX_DLIGHTS - 1));

	ndl = &NIGHT_dlight[dlight_index];

	ndl->red   = red;
	ndl->green = green;
	ndl->blue  = blue;
	*/
}



void  NIGHT_dlight_destroy(UBYTE dlight_index)
{
	SLONG  map;
	UBYTE  next;
	UBYTE *prev;

	NIGHT_Dlight *ndl;
	if(dlight_index==0)
		return;

	ASSERT(WITHIN(dlight_index, 1, NIGHT_MAX_DLIGHTS - 1));

	//
	// Take this light out of the used list. I hate doubley linked
	// lists.  I'd rather do this!
	//

	next =  NIGHT_dlight_used;
	prev = &NIGHT_dlight_used;

	while(1)
	{
		if (next == NULL)
		{
			//
			// The light wasn't in the linked list!
			//

			ASSERT(0);
		}

		ASSERT(WITHIN(next, 1, NIGHT_MAX_DLIGHTS - 1));

		ndl = &NIGHT_dlight[next];

		if (next == dlight_index)
		{
			//
			// This is the light to take out of the list.
			//

		   *prev = ndl->next;
			
			break;
		}

		prev = &ndl->next;
		next =  ndl->next;
	}

	//
	// Add the light to the free list.
	//

	ndl->next         = NIGHT_dlight_free;
	NIGHT_dlight_free = dlight_index;
}
extern SLONG			NGAMUT_zmin;
extern SLONG			NGAMUT_xmin;

void NIGHT_dlight_render()
{
	NIGHT_Dlight *ndl;

	lum_off_x=NGAMUT_xmin;
	SATURATE(lum_off_x,0,96);
	lum_off_z=NGAMUT_zmin;
	SATURATE(lum_off_z,0,96);

//	memset((void*)floor_lum,0,sizeof(floor_lum));
	{
		//
		// I don't trust memset to be as efficient as this following code MikeD
		//
		SLONG	c0;
		ULONG	*ptr;
		ptr=(ULONG*)floor_lum;
		for(c0=0;c0<32*8;c0++)
		{
			*ptr++=0; // no point optimising the loop as the memory write is the slow thing
		}

	}

	for(ndl=&NIGHT_dlight[NIGHT_dlight_used];ndl!=NIGHT_dlight;ndl=&NIGHT_dlight[ndl->next])
	{
		SLONG x,z,r,minx,minz,maxx,maxz,x0,z0;

		x=ndl->x-(lum_off_x<<8);	// Shift into lum coords
		z=ndl->z-(lum_off_z<<8);	// Shift into lum coords
		r=(ndl->radius<<1)+128;		// Expand radius to reasonable range

		minx=(x-r)>>8;
		minz=(z-r)>>8;
		maxx=(x+r)>>8;
		maxz=(z+r)>>8;

		r-=128;						// Return to normal radius

		if (minx>31) continue;
		if (maxx<0) continue;
		if (minz>31) continue;
		if (maxz<0) continue;

		if (minx<0) minx=0;
		if (maxx>31) maxx=31;
		if (minz<0) minz=0;
		if (maxz>31) maxz=31;

		for(x0=minx;x0<=maxx;x0++)
			for(z0=minz;z0<=maxz;z0++)
			{
				SLONG lum=((r-(QDIST2(abs(x-(x0<<8)),abs(z-(z0<<8)))>>0))>>1);//+ndl->red-128;   // so we can have a brightness

				if(lum>0)
				{
					if(ndl->red<10)
					{
						lum>>=2;
					}
					lum=floor_lum[x0][z0]+lum;
					SATURATE(lum,0,128); //was 0,255
					floor_lum[x0][z0]=lum;
				}
//				SetLumi(x0,z0,lum);
			}
	}
}


// ========================================================
//
// CACHED LIGHTING OF THE MAP AND PRIMS
//
// ========================================================

void NIGHT_cache_init()
{
#ifndef PSX
	SLONG i;

	//
	// Build the free list of cache squares.
	//

	NIGHT_square_free     = 1;
	NIGHT_square_num_used = 0;

	for (i = 1; i < NIGHT_MAX_SQUARES - 1; i++)
	{
		NIGHT_square[i].next = i + 1;
		NIGHT_square[i].flag = 0;
	}

	NIGHT_square[NIGHT_MAX_SQUARES - 1].next = 0;

	//
	// Clear the cache entries.
	//

	memset((UBYTE*)NIGHT_cache, 0, sizeof(NIGHT_cache));
#endif
}

void NIGHT_cache_create(SLONG lo_map_x, SLONG lo_map_z)
{
	// this used to be fore cached floor lighting, but we now have a word per 
	// hi-res map cell
	//
}

void NIGHT_cache_recalc(void)
{
#ifndef PSX
	SLONG i;

	SLONG lo_map_x;
	SLONG lo_map_z;
	SLONG ware;

	for (i = 1; i < NIGHT_MAX_SQUARES; i++)
	{
   		if (NIGHT_square[i].flag & NIGHT_SQUARE_FLAG_USED)
		{
			ware = NIGHT_square[i].flag & NIGHT_SQUARE_FLAG_USED;

			lo_map_x = NIGHT_square[i].lo_map_x;
			lo_map_z = NIGHT_square[i].lo_map_z;

			NIGHT_cache_destroy(i);
			NIGHT_cache_create (lo_map_x, lo_map_z);//, ware);
		}
	}
#endif
}

void NIGHT_cache_destroy(UBYTE square_index)
{
#ifndef PSX
	NIGHT_Square *nq;

	ASSERT(WITHIN(square_index, 0, NIGHT_MAX_SQUARES - 1));

	nq = &NIGHT_square[square_index];

	//
	// Make sure this square is in use.
	//

	ASSERT(nq->flag & NIGHT_SQUARE_FLAG_USED);

	//
	// Return the memory.
	//

	HEAP_give(nq->colour, nq->sizeof_colour);

	//
	// Insert into the free list.
	//

	nq->next          = NIGHT_square_free;
	NIGHT_square_free = square_index;

	//
	// Clear the cache mapwho.
	//

	NIGHT_cache[nq->lo_map_x][nq->lo_map_z] = NULL;

	//
	// Mark as unused.
	//

	nq->flag = 0;

	NIGHT_square_num_used -= 1;
#endif
}



// ========================================================
//
// CACHED LIGHTING OF DFACETS ON THE SUPERMAP
//
// ========================================================

void NIGHT_dfcache_init(void)
{
	SLONG i;

	//
	// Build the free list of dfcache squares.
	//

	NIGHT_dfcache_free = 1;
	NIGHT_dfcache_used = 0;

	for (i = 1; i < NIGHT_MAX_DFCACHES - 1; i++)
	{
		NIGHT_dfcache[i].next = i + 1;
//		NIGHT_dfcache[i].used = FALSE;
	}

	NIGHT_dfcache[NIGHT_MAX_DFCACHES - 1].next = 0;
//	NIGHT_dfcache[NIGHT_MAX_DFCACHES - 1].used = FALSE;

	//
	// Clear all the cached lighting from the DFacets.
	//

	for (i = 1; i < next_dfacet; i++)
	{
		dfacets[i].Dfcache = 0;
	}
}

void NIGHT_dfcache_recalc()
{
	SLONG dfcache;
	SLONG next;
	SLONG dfacet;

	for (dfcache = NIGHT_dfcache_used; dfcache; dfcache = next)
	{
		ASSERT(WITHIN(dfcache, 1, NIGHT_MAX_DFCACHES - 1));

		dfacet = NIGHT_dfcache[dfcache].dfacet;
		next   = NIGHT_dfcache[dfcache].next;

		ASSERT(WITHIN(dfacet, 1, next_dfacet - 1));

		NIGHT_dfcache_destroy(dfcache);

		dfacets[dfacet].Dfcache = NIGHT_dfcache_create(dfacet);
	}
}

struct
{
	UBYTE index;
	UBYTE padding;
	UWORD x;
	UWORD z;

}     slight[16];

// Moved to reduce stack overhead

SLONG length;
SLONG height;
SLONG num_points;
SLONG num_faces;
SLONG num_bytes;
SLONG no_shadows;


UBYTE NIGHT_dfcache_create(UWORD dfacet_index)
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG lx;
	SLONG ly;
	SLONG lz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG dsx1;
	SLONG dsy1;
	SLONG dsz1;

	SLONG dsx2;
	SLONG dsy2;
	SLONG dsz2;

	SLONG dlx;
	SLONG dly;
	SLONG dlz;

	#ifndef NDEBUG
	void *min_address;
	void *max_address;
	#endif

	// Moved from here. ^ ^ ^ ^
	SLONG dprod;

	SLONG base_red;
	SLONG base_green;
	SLONG base_blue;

	SLONG red;
	SLONG green;
	SLONG blue;

	SLONG dist;
	SLONG bright;

	SLONG  shadow_shift;
	UBYTE *shadow_byte;
	UBYTE  shadow;

	UBYTE dfcache_index;
	UBYTE darken;

	NIGHT_Dfcache *nd;
	NIGHT_Colour  *nc;
	NIGHT_Smap    *ns;
	NIGHT_Slight  *nsl;

	#define NIGHT_MAX_SLIGHTS_PER_FACET 16

	SLONG slight_upto = 0;

	DFacet *df;

	ASSERT(WITHIN(dfacet_index, 1, next_dfacet - 1));

	df = &dfacets[dfacet_index];

	//
	// Calculate facet info.
	//

	dx = (df->x[1] - df->x[0])<<8;
	dz = (df->z[1] - df->z[0])<<8;

	if (dx && dz)
	{
		//
		// We can't handle diagonal facets.
		//
		if(df->FacetType==STOREY_TYPE_NORMAL)
			ASSERT(0);
	}

	if (dx)
	{
		length  = abs(dx >> 8);
		length += 1;

		dx = (dx > 0) ? +256 : -256;
		dz = 0;
	}
	else
	{
		ASSERT(dz);

		length  = abs(dz >> 8);
		length += 1;

		dx = 0;
		dz = (dz > 0) ? +256 : -256;
	}

	if (df->FacetType == STOREY_TYPE_FENCE ||
		df->FacetType == STOREY_TYPE_FENCE_FLAT)
	{
		height = 2;
		dy     = df->Height * 64;
	}
	else
	{
		height  = df->Height >> 2;
		height += 1;
		dy      = 256;
	}

	ASSERT(height >= 2);

	num_points = length * height;
	num_faces  = num_points - length - height + 1;
	num_bytes  = num_points * sizeof(NIGHT_Colour);

	//
	// Get a new cache element.
	//

 	ASSERT(WITHIN(NIGHT_dfcache_free, 1, NIGHT_MAX_DFCACHES - 1));

	dfcache_index      =  NIGHT_dfcache_free;
	nd                 = &NIGHT_dfcache[dfcache_index];

	nd->colour        = (NIGHT_Colour *) HEAP_get(num_bytes);
	if(nd->colour==0)
	{
		nd->dfacet = 0;
		return(0);
	}

	NIGHT_dfcache_free =  nd->next;

	//ASSERT(!nd->used);

	//nd->used   = TRUE;

	nd->dfacet = dfacet_index;
	nd->next           = NIGHT_dfcache_used;
	NIGHT_dfcache_used = dfcache_index;


	//
	// Allocate memory. How many bytes to we need?
	//

//	num_bytes += num_faces + 3 >> 2; //I think this is for shadow stuff

	nd->sizeof_colour = num_bytes;
//	nd->shadow_bytes  = num_faces + 3 >> 2;

//	ASSERT(nd->colour != NULL);

	#ifndef NDEBUG
	min_address = nd->colour;
	max_address = ((UBYTE *) nd->colour) + (num_bytes - 1);
	#endif

	//
	// The normal of the facet
	// 

	nx = -dz;
	ny =  0;
	nz =  dx;

	dprod =
		NIGHT_amb_norm_x * nx +
		NIGHT_amb_norm_z * nz >> 8;

	//
	// If the facet is facing away from the ambient light then
	// we don't need to check for shadows...
	//

	if (WARE_in)
	{
		//
		// Inside a warehouse we don't want directional light but we still want a
		// slight variation in hue.
		//

		dprod >>= 3;
		dprod  += 128 + 64 + 32;

		//
		// The walls of a warehouse are seen from the other side.
		//

		nx = -nx;
		nz = -nz;
	}
	else
	{

		dprod >>= 2;
		dprod  += 192;
	}

	base_red   = NIGHT_amb_red   * dprod >> 7;
	base_green = NIGHT_amb_green * dprod >> 7;
	base_blue  = NIGHT_amb_blue  * dprod >> 7;

	//
	// ambient goes from 0->128 as PC goes from 0->64 
	//

	//
	// Find all the static lights effecting this facet.
	//

	{
		SLONG mx;
		SLONG mz;
		SLONG mx1;
		SLONG mz1;
		SLONG mx2;
		SLONG mz2;
		SLONG map_x;
		SLONG map_z;

		mx1 = df->x[0]<<8;
		mx2 = df->x[1]<<8;

		mz1 = df->z[0]<<8;
		mz2 = df->z[1]<<8;

		if (mx1 > mx2) {SWAP(mx1, mx2);}
		if (mz1 > mz2) {SWAP(mz1, mz2);}

		mx1 -= 0x400;
		mz1 -= 0x400;

		mx2 += 0x400;
		mz2 += 0x400;

		mx1 >>= PAP_SHIFT_LO;
		mz1 >>= PAP_SHIFT_LO;
		mx2 >>= PAP_SHIFT_LO;
		mz2 >>= PAP_SHIFT_LO;

		SATURATE(mx1, 0, PAP_SIZE_LO - 1);
		SATURATE(mz1, 0, PAP_SIZE_LO - 1);
		SATURATE(mx2, 0, PAP_SIZE_LO - 1);
		SATURATE(mz2, 0, PAP_SIZE_LO - 1);

		for (mx = mx1; mx <= mx2; mx++)
		for (mz = mz1; mz <= mz2; mz++)
		{
			map_x = mx << PAP_SHIFT_LO;
			map_z = mz << PAP_SHIFT_LO;

			ASSERT(WITHIN(mx, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(mz, 0, PAP_SIZE_LO - 1));

			ns = &NIGHT_smap[mx][mz];

			for (i = 0; i < ns->number; i++)
			{
				ASSERT(WITHIN(ns->index + i, 0, NIGHT_slight_upto - 1));

				nsl = &NIGHT_slight[ns->index + i];

				lx = (nsl->x << 2) + map_x;
				lz = (nsl->z << 2) + map_z;

				//
				// Is the facet facing away from the light?
				//

				dlx = (df->x[0]<<8) - lx;
				dlz = (df->z[0]<<8) - lz;

				dprod = dlx*nx + dlz*nz;

				if (dprod > 0)
				{
					//
					// This light might light our facet up.
					//

					if (WITHIN(slight_upto, 0, NIGHT_MAX_SLIGHTS_PER_FACET - 1))
					{
						slight[slight_upto].index = ns->index + i;
						slight[slight_upto].x     = lx;
						slight[slight_upto].z     = lz;

						slight_upto += 1;
					}
				}
			}
		}
	}

	//
	// Now light the points.
	//

	lx = df->x[0]<<8;
	ly = df->Y[0];
	lz = df->z[0]<<8;

	nc = nd->colour;

	for (i = 0; i < height; i++)
	{
		x = lx;
		y = ly;
		z = lz;

		darken = 0;//(NIGHT_flag & NIGHT_FLAG_DARKEN_BUILDING_POINTS) && (ly == PAP_calc_height_at_point(lx >> 8, ly >> 8));

		for (j = 0; j < length; j++)
		{
			red   = base_red;
			green = base_green;
			blue  = base_blue;

			if(0)
			if (darken)
			{
				#define NIGHT_DARKEN_FACET 16

				red   -= NIGHT_DARKEN_FACET;
				green -= NIGHT_DARKEN_FACET;
				blue  -= NIGHT_DARKEN_FACET;
			}

			//
			// Static lighting.
			//

			for (k = 0; k < slight_upto; k++)
			{
				ASSERT(WITHIN(slight[k].index, 0, NIGHT_slight_upto - 1));

				nsl = &NIGHT_slight[slight[k].index];

				dlx = x - slight[k].x;
				dlz = z - slight[k].z;
				dly = y - nsl->y;

				dist = QDIST3(abs(dlx),abs(dly),abs(dlz)) + 1;

				if (dist < (nsl->radius << 2))
				{
					dprod = dlx*nx + dlz*nz;

					ASSERT(dprod >= 0);	// We only consider lights facing the facet.

					dprod /= dist;
					bright = 256 - (dist << 8)  / (nsl->radius << 2);
					bright = bright * dprod >> 8;

					red   += nsl->red   * bright >> 8;
					green += nsl->green * bright >> 8;
					blue  += nsl->blue  * bright >> 8;
				}
			}

//			red<<=2;
//			green<<=2;
//			blue<<=2;
			SATURATE(red,   0, 255);
			SATURATE(green, 0, 255);
			SATURATE(blue,  0, 255);

			nc->red   = red;
			nc->green = green;
			nc->blue  = blue;

		    x += dx;
			z += dz;

			nc += 1;
		}

		ly += dy;
	}

	ASSERT(nc == nd->colour + num_points);

	/*
	if (no_shadows)
	{
		//
		// Just set all the shadow bits to zero.
		//

		shadow_byte = (UBYTE *) nc;

		for (i = nd->shadow_bytes; i > 0; i--)
		{
			ASSERT(WITHIN(shadow_byte, min_address, max_address));

			*shadow_byte++ = 0;
		}
	}
	else
	{
		//
		// The vectors to the points within each face where we do
		// the shadow los from.
		//

		dsx1 = dx >> 2;
		dsy1 = 0x40;
		dsz1 = dz >> 2;

		dsx2 = dx * 3 >> 2;
		dsy2 = 0xc0;
		dsz2 = dz * 3 >> 2;

		//
		// Set the shadow bits.
		//			  

		lx = df->x[0]<<8;
		ly = df->Y[0];
		lz = df->z[0]<<8;

		shadow_shift = 0;
		shadow_byte  = (UBYTE *) nc;
	   *shadow_byte  = 0;
		
		for (i = 0; i < height - 1; i++)
		{
			x = lx;
			y = ly;
			z = lz;

			for (j = 0; j < length - 1; j++)
			{
				shadow = 0;

				//
				// Do the shadow calculation.
				//

				if (SHADOW_in(x + dsx1, y + dsy1, z + dsz1)) {shadow |= 1;}
				if (SHADOW_in(x + dsx2, y + dsy2, z + dsz2)) {shadow |= 2;}

				ASSERT(WITHIN(shadow_byte, min_address, max_address));

			   *shadow_byte  &= ~(0x3    << shadow_shift);
			   *shadow_byte  |=  (shadow << shadow_shift);
				shadow_shift += 2;
				
				if (shadow_shift >= 8)
				{
					shadow_shift = 0;
					shadow_byte += 1;
				}

				x += dx;
				z += dz;
			}

			ly += 256;
		}
	}
	*/
	return dfcache_index;
}

void NIGHT_dfcache_destroy(UBYTE dfcache_index)
{
	NIGHT_Dfcache *nd;

	ASSERT(WITHIN(dfcache_index, 1, NIGHT_MAX_DFCACHES - 1));

	//
	// Take out of the used list.
	//

	UBYTE  next =  NIGHT_dfcache_used;
	UBYTE *prev = &NIGHT_dfcache_used;

	while(1)
	{
		if (next == NULL)
		{
			//
			// It wasn't in the used list! 
			//

			ASSERT(0);

			break;
		}

		ASSERT(WITHIN(next, 1, NIGHT_MAX_DFCACHES - 1));

		nd = &NIGHT_dfcache[next];

		if (next == dfcache_index)
		{
			//
			// This is the one to take out of the used list.
			//

		   *prev = nd->next;
			
			break;
		}

		prev = &nd->next;
		next =  nd->next;
	}

	//
	// Return the memory.
	//

	nd = &NIGHT_dfcache[dfcache_index];

	HEAP_give(nd->colour, nd->sizeof_colour);

	//
	// Insert into the free list.
	//

	nd->next           = NIGHT_dfcache_free;
	NIGHT_dfcache_free = dfcache_index;
	nd->dfacet         = NULL;
}

//
// Light at a point.
//
NIGHT_Colour NIGHT_get_light_at(SLONG x,SLONG y,SLONG z)
{
	NIGHT_Colour ans;
	UWORD	col;

	col=floor_psx_col[x>>8][z>>8];


	ans.red  =((col>>10)&0x3f)<<2;
	ans.green=((col>>5)&0x2f)<<3;
	ans.blue =((col>>0)&0x2f)<<3;

	return ans;
}

void NIGHT_get_lampost(SLONG x,SLONG y,SLONG z,SLONG *r,SLONG *g,SLONG *b)
{
	SLONG mx;
	SLONG mz;

	SLONG xmid;
	SLONG zmid;

	SLONG xmin;
	SLONG zmin;

	SLONG xmax;
	SLONG zmax;

	SLONG map_x;
	SLONG map_z;

	SLONG lx;
	SLONG ly;
	SLONG lz;
	SLONG lradius;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG bright;

	SLONG red=0;
	SLONG green=0;
	SLONG blue=0;

	SLONG vector[3];


	NIGHT_Colour ans;

	OB_Info *oi;


	//
	// Look for static lights effecting this point.
	//

	xmid = x >> PAP_SHIFT_LO;
	zmid = z >> PAP_SHIFT_LO;

	xmin = xmid - 1;
	zmin = zmid - 1;

	xmax = xmid + 1;
	zmax = zmid + 1;

	SATURATE(xmin, 0, PAP_SIZE_LO - 1);
	SATURATE(zmin, 0, PAP_SIZE_LO - 1);
	SATURATE(xmax, 0, PAP_SIZE_LO - 1);
	SATURATE(zmax, 0, PAP_SIZE_LO - 1);

	for (mz = zmin; mz <= zmax; mz++)
	for (mx = xmin; mx <= xmax; mx++)
	{
		if (NIGHT_flag & NIGHT_FLAG_LIGHTS_UNDER_LAMPOSTS)
		{
			//
			// Lampost lights.
			//

			for (oi = OB_find(mx,mz); oi->prim; oi++)
			{
				if (oi->prim < 5)
				{
					FMATRIX_vector(
						vector,
						oi->yaw,
						oi->pitch);

					//
					// The position of the static light is relative
					// to the mapwho it is in.
					//

					lx = (-vector[0] >> 9) + oi->x;
					ly = (-vector[1] >> 9) + oi->y + 0x100;
					lz = (-vector[2] >> 9) + oi->z;

					dx = abs(lx - x);
					dy = abs(ly - y);
					dz = abs(lz - z);

					lradius = NIGHT_lampost_radius << 2;

					//
					// Distance from the light.
					//

					dist = QDIST3(abs(dx),abs(dy),abs(dz));

					if (dist > lradius)
					{
						//
						// The point is too far from this light.
						//
					}
					else
					{
						bright = 128 - (dist * 128) / lradius;

						red   += NIGHT_lampost_red   * bright >> 7;
						green += NIGHT_lampost_green * bright >> 7;
						blue  += NIGHT_lampost_blue  * bright >> 7;
					}
				}
			}
		}
	}


	*r= red;
	*g= green;
	*b= blue;

}

#ifndef	PSX
NIGHT_Colour NIGHT_get_light_at(
				SLONG x,
				SLONG y,
				SLONG z)
{
	SLONG i;

	SLONG mx;
	SLONG mz;

	SLONG xmid;
	SLONG zmid;

	SLONG xmin;
	SLONG zmin;

	SLONG xmax;
	SLONG zmax;

	SLONG map_x;
	SLONG map_z;

	SLONG lx;
	SLONG ly;
	SLONG lz;
	SLONG lradius;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG bright;

	SLONG red;
	SLONG green;
	SLONG blue;

	SLONG vector[3];

	NIGHT_Smap   *ns;
	NIGHT_Slight *nsl;

	NIGHT_Colour ans;

	OB_Info *oi;

	//
	// Initialise to the ambient light.
	//

	red   = NIGHT_amb_red   * 3 >> 2;
	green = NIGHT_amb_green * 3 >> 2;
	blue  = NIGHT_amb_blue  * 3 >> 2;

	//
	// Look for static lights effecting this point.
	//

	xmid = x >> PAP_SHIFT_LO;
	zmid = z >> PAP_SHIFT_LO;

	xmin = xmid - 1;
	zmin = zmid - 1;

	xmax = xmid + 1;
	zmax = zmid + 1;

	SATURATE(xmin, 0, PAP_SIZE_LO - 1);
	SATURATE(zmin, 0, PAP_SIZE_LO - 1);
	SATURATE(xmax, 0, PAP_SIZE_LO - 1);
	SATURATE(zmax, 0, PAP_SIZE_LO - 1);

	for (mz = zmin; mz <= zmax; mz++)
	for (mx = xmin; mx <= xmax; mx++)
	{
		ns = &NIGHT_smap[mx][mz];

		map_x = mx << PAP_SHIFT_LO;
		map_z = mz << PAP_SHIFT_LO;

		for (i = 0; i < ns->number; i++)
		{
			nsl = &NIGHT_slight[ns->index + i];

			//
			// The position of the static light is relative
			// to the mapwho it is in.
			//

			lx = (nsl->x << 2) + map_x;
			ly =  nsl->y;
			lz = (nsl->z << 2) + map_z;

			dx = abs(lx - x);
			dy = abs(ly - y);
			dz = abs(lz - z);

			lradius = nsl->radius << 2;

			//
			// Distance from the light.
			//

			dist = QDIST3(abs(dx),abs(dy),abs(dz));

			if (dist > lradius)
			{
				//
				// The point is too far from this light.
				//
			}
			else
			{
				bright = 128 - (dist * 128) / lradius;

				red   += nsl->red   * bright >> 8;
				green += nsl->green * bright >> 8;
				blue  += nsl->blue  * bright >> 8;
			}
		}

		if (NIGHT_flag & NIGHT_FLAG_LIGHTS_UNDER_LAMPOSTS)
		{
			//
			// Lampost lights.
			//

			for (oi = OB_find(mx,mz); oi->prim; oi++)
			{
				if (oi->prim < 5)
				{
					FMATRIX_vector(
						vector,
						oi->yaw,
						oi->pitch);

					//
					// The position of the static light is relative
					// to the mapwho it is in.
					//

					lx = (-vector[0] >> 9) + oi->x;
					ly = (-vector[1] >> 9) + oi->y + 0x100;
					lz = (-vector[2] >> 9) + oi->z;

					dx = abs(lx - x);
					dy = abs(ly - y);
					dz = abs(lz - z);

					lradius = NIGHT_lampost_radius << 2;

					//
					// Distance from the light.
					//

					dist = QDIST3(abs(dx),abs(dy),abs(dz));

					if (dist > lradius)
					{
						//
						// The point is too far from this light.
						//
					}
					else
					{
						bright = 128 - (dist * 128) / lradius;

						red   += NIGHT_lampost_red   * bright >> 8;
						green += NIGHT_lampost_green * bright >> 8;
						blue  += NIGHT_lampost_blue  * bright >> 8;
					}
				}
			}
		}
	}

	SATURATE(red,   0, 255);
	SATURATE(green, 0, 255);
	SATURATE(blue,  0, 255);

	ans.red   = red;
	ans.green = green;
	ans.blue  = blue;

	return ans;
}
#endif

void NIGHT_lum_init()
{
	lum_off_x=lum_off_z=0;
	memset((void*)floor_lum,0,sizeof(floor_lum));
}

void NIGHT_init()
{
	//
	// Initialise the heap where all the cached memory lives.
	//

	HEAP_init();

	//
	// Initialises everything else.
	//
	
	NIGHT_cache_init();
	NIGHT_slight_init();
	NIGHT_dlight_init();
	NIGHT_dfcache_init();
	NIGHT_lum_init();
	//
	// Sensible defaults.
	//

	NIGHT_sky_colour.red   = 210;
	NIGHT_sky_colour.green = 200;
	NIGHT_sky_colour.blue  = 240;

	NIGHT_lampost_red   = 70;
	NIGHT_lampost_green = 70;
	NIGHT_lampost_blue  = 36;

	NIGHT_lampost_radius = 255;

	NIGHT_flag = NIGHT_FLAG_DAYTIME | NIGHT_FLAG_DARKEN_BUILDING_POINTS;

	NIGHT_ambient(255, 255, 255, 110, -148, -177);
}


// ========================================================
//
// WALKABLE POINT LIGHTING
//
// ========================================================

#ifndef NDEBUG

SLONG NIGHT_check_index(SLONG walkable_prim_point_index)
{
	ASSERT(WITHIN(
			walkable_prim_point_index,
			first_walkable_prim_point, 
			first_walkable_prim_point + number_of_walkable_prim_points - 1));

	return TRUE;
}

#endif

/*
SLONG        NIGHT_first_walkable_prim_point;
NIGHT_Colour NIGHT_walkable[NIGHT_MAX_WALKABLE];
*/
void NIGHT_generate_walkable_lighting()
{
	/*
	SLONG i;
	SLONG j;

	SLONG mx;
	SLONG mz;

	SLONG mx1;
	SLONG mz1;
	SLONG mx2;
	SLONG mz2;

	SLONG map_x;
	SLONG map_z;

	SLONG lradius;
	SLONG lx;
	SLONG ly;
	SLONG lz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;
	SLONG dprod;
	SLONG bright;

	SLONG amb_amount;
	SLONG amb_red;
	SLONG amb_green;
	SLONG amb_blue;

	SLONG red;
	SLONG green;
	SLONG blue;

	PrimPoint    *pp;
	NIGHT_Smap   *ns;
	NIGHT_Slight *nsl;
	NIGHT_Colour *nc;

	//
	// The first walkable prim-point...
	//

	NIGHT_first_walkable_prim_point = first_walkable_prim_point;
	
	//
	// The ambient colour of the walkable prim points.
	// 

	amb_amount  =  128;
	amb_amount += -NIGHT_amb_norm_y >> 1;

	amb_red   = NIGHT_amb_red   * amb_amount >> 8;
	amb_green = NIGHT_amb_green * amb_amount >> 8;
	amb_blue  = NIGHT_amb_blue  * amb_amount >> 8;

	//
	// The lighting for all walkable the prim points.
	//

	for (i = 0; i < number_of_walkable_prim_points; i++)
	{
		ASSERT(WITHIN(i, 0, NIGHT_MAX_WALKABLE));
		ASSERT(WITHIN(first_walkable_prim_point + i, 1, next_prim_point - 1));

		pp = &prim_points[first_walkable_prim_point + i];

		//
		// Initialise the points to almost all the ambient colour.
		//

		nc = &NIGHT_walkable[i];

		nc->red   = amb_red;
		nc->green = amb_green;
		nc->blue  = amb_blue;

		//
		// The bounding box in which we search for static lighting affecting this point.
		//

		mx1 = pp->X - 0x400 >> PAP_SHIFT_LO;
		mz1 = pp->Z - 0x400 >> PAP_SHIFT_LO;

		mx2 = pp->X + 0x400 >> PAP_SHIFT_LO;
		mz2 = pp->Z + 0x400 >> PAP_SHIFT_LO;

		SATURATE(mx1, 0, PAP_SIZE_LO - 1);
		SATURATE(mz1, 0, PAP_SIZE_LO - 1);

		SATURATE(mx2, 0, PAP_SIZE_LO - 1);
		SATURATE(mz2, 0, PAP_SIZE_LO - 1);

		for (mx = mx1; mx <= mx2; mx++)
		for (mz = mz1; mz <= mz2; mz++)
		{
			map_x = mx << PAP_SHIFT_LO;
			map_z = mz << PAP_SHIFT_LO;

			ASSERT(WITHIN(mx, 0, PAP_SIZE_LO - 1));
			ASSERT(WITHIN(mz, 0, PAP_SIZE_LO - 1));

			ns = &NIGHT_smap[mx][mz];

			for (j = 0; j < ns->number; j++)
			{
				ASSERT(WITHIN(ns->index + j, 0, NIGHT_MAX_SLIGHTS - 1));

				nsl = &NIGHT_slight[ns->index + j];

				lx = (nsl->x << 2) + map_x;
				ly =  nsl->y;
				lz = (nsl->z << 2) + map_z;

				lradius = nsl->radius << 2;

				if (ly <= pp->Y)
				{
					//
					// We assume that the normals of the walkable points all point
					// up.  So to be lit up the light must be higher than the point.
					//

					continue;
				}

				//
				// Light this point...
				//

				dx = pp->X - lx;
				dy = pp->Y - ly;
				dz = pp->Z - lz;

				dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

				if (dist > lradius)
				{
					//
					// Too far away.
					//

					continue;
				}

				//
				// Lighting dot product... but with a hardcoded normal
				// of (0,1,0)
				//

				dprod  = -256 * dy;
				dprod /=  dist;
				bright =  256 - (dist << 8) / lradius;
				bright =  bright * dprod >> 8;

				red   = nc->red;
				green = nc->green;
				blue  = nc->blue;

				red   += nsl->red   * bright >> 8;
				green += nsl->green * bright >> 8;
				blue  += nsl->blue  * bright >> 8;

				SATURATE(red,   0, 255);
				SATURATE(green, 0, 255);
				SATURATE(blue,  0, 255);

				nc->red   = red;
				nc->green = green;
				nc->blue  = blue;
			}
		}
	}
	*/
}



void NIGHT_destroy_all_cached_info()
{
	HEAP_init();
	NIGHT_cache_init();
	NIGHT_dfcache_init();
}


#ifndef PSX
SLONG NIGHT_load_ed_file(CBYTE *name)
{
//#ifndef	PSX
	SLONG i;

	SLONG sizeof_ed_light;
	SLONG ed_max_lights;
	SLONG sizeof_night_colour;
	SLONG ed_light_free;

	ED_Light el;

	NIGHT_Colour col;

	MFFileHandle handle = NULL;

	handle = FileOpen(name);

	if (handle==FILE_OPEN_ERROR)
	{
		return FALSE;
	}

	//
	// The header.
	//



	if (FileRead(handle,&sizeof_ed_light,     sizeof(SLONG)) != sizeof(SLONG)) goto file_error;
	if (FileRead(handle,&ed_max_lights,     sizeof(SLONG)) != sizeof(SLONG)) goto file_error;
	if (FileRead(handle,&sizeof_night_colour,     sizeof(SLONG)) != sizeof(SLONG)) goto file_error;


	if (sizeof_ed_light     != sizeof(ED_Light) ||
		sizeof_night_colour != sizeof(NIGHT_Colour))
	{
		//
		// Incompatable files.
		//

		FileClose(handle);

		return FALSE;
	}

	//
	// The lights.
	//

	for (i = 0; i < ed_max_lights; i++)
	{
		if (FileRead(handle,&el, sizeof(ED_Light)) != sizeof(ED_Light)) goto file_error;

		if (el.used)
		{
			NIGHT_slight_create(
				el.x,
				el.y,
				el.z,
				el.range,
				el.red,
				el.green,
				el.blue);
		}
	}

	//
	// We can ignore this.
	// 
	
	if (FileRead(handle,&ed_light_free, sizeof(SLONG)) != sizeof(SLONG)) goto file_error;

	//
	// Other stuff.
	//

	ULONG        flag;
	ULONG        amb_d3d_colour;
	ULONG        amb_d3d_specular;
	SLONG        amb_red;
	SLONG        amb_green;
	SLONG        amb_blue;
	SBYTE		 lampost_red;
	SBYTE		 lampost_green;
	SBYTE		 lampost_blue;
	UBYTE        padding;
	SLONG        lampost_radius;
	NIGHT_Colour sky_colour;

	if (FileRead(handle,&flag,             sizeof(ULONG)) !=sizeof(ULONG)) goto file_error;
	if (FileRead(handle,&amb_d3d_colour,   sizeof(ULONG)) !=sizeof(ULONG))  goto file_error;
	if (FileRead(handle,&amb_d3d_specular, sizeof(ULONG)) !=sizeof(ULONG) ) goto file_error;
	if (FileRead(handle,&amb_red,          sizeof(ULONG)) !=sizeof(ULONG) ) goto file_error;
	if (FileRead(handle,&amb_green,        sizeof(ULONG)) !=sizeof(ULONG)  ) goto file_error;
	if (FileRead(handle,&amb_blue,         sizeof(ULONG)) !=sizeof(ULONG)  ) goto file_error;
	if (FileRead(handle,&lampost_red,      sizeof(SBYTE)) !=sizeof(SBYTE)  ) goto file_error;
	if (FileRead(handle,&lampost_green,    sizeof(SBYTE)) !=sizeof(SBYTE)  ) goto file_error;
	if (FileRead(handle,&lampost_blue,     sizeof(SBYTE)) !=sizeof(SBYTE)  ) goto file_error;
	if (FileRead(handle,&padding,          sizeof(UBYTE)) !=sizeof(UBYTE)  ) goto file_error;
	if (FileRead(handle,&lampost_radius,   sizeof(SLONG)) !=sizeof(SLONG)  ) goto file_error;
	if (FileRead(handle,&sky_colour,       sizeof(NIGHT_Colour)) !=sizeof(NIGHT_Colour) ) goto file_error;

	NIGHT_ambient(
		amb_red   * 820 >> 8,
		amb_green * 820 >> 8,
		amb_blue  * 820 >> 8,
		110, -148, -177);	// These constants crop up everywhere!

	NIGHT_flag           = flag;
	NIGHT_sky_colour     = sky_colour;
	NIGHT_lampost_radius = lampost_radius;
	NIGHT_lampost_red    = lampost_red;
	NIGHT_lampost_green  = lampost_green;
	NIGHT_lampost_blue   = lampost_blue;

	FileClose(handle);

	return TRUE;

  file_error:;

	FileClose(handle);
	
//#endif
	return FALSE;	
}

#endif

#endif