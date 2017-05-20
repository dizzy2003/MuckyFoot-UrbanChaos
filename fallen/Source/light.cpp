//
// Lights...
//

#include "game.h"

#ifndef	PSX

#include "light.h"
#include "fmatrix.h"
#include	"memory.h"


#define IMPLIES(a,b) (!(a) || (b))

typedef struct
{
	GameCoord    pos;
	LIGHT_Colour colour;
	UBYTE        range;
	UBYTE        type;
	UBYTE        param;
	UBYTE        counter;
	UBYTE        next;	// Free list or mapwho list. 0 is the NULL index.

} LIGHT_Light;

#define LIGHT_MAX_LIGHTS 128

LIGHT_Light LIGHT_light[LIGHT_MAX_LIGHTS];
UBYTE       LIGHT_free;

//
// The light mapwho
//

typedef struct
{
	UBYTE next;

} LIGHT_Square;

#define LIGHT_MAP_SIZE (MAP_WIDTH / 2)

LIGHT_Square LIGHT_map[LIGHT_MAP_SIZE][LIGHT_MAP_SIZE];

//
// Converts a world map coordinate to a mapwho square coordinate.
//

#define LIGHT_TO_MAP(x) ((x) >> 9)


//
// The ambient light. The length of the light normal is 255.
//

LIGHT_Colour LIGHT_amb_colour;
SLONG        LIGHT_amb_norm_x;
SLONG        LIGHT_amb_norm_y;
SLONG        LIGHT_amb_norm_z;

//
// The height field lit by the lights.
//

LIGHT_Map LIGHT_hf;

//
// Lighting on buildings.
//

LIGHT_Colour LIGHT_building_point[MAX_PRIM_POINTS];


//
// The cache of light values.
//

#define LIGHT_PER_SLOT 16

typedef struct
{
	LIGHT_Colour colour[LIGHT_PER_SLOT];
	UWORD        next;

} LIGHT_Slot;

#ifdef	PSX
#define LIGHT_MAX_SLOTS 128
#else
#define LIGHT_MAX_SLOTS 1280
#endif

LIGHT_Slot LIGHT_slot[LIGHT_MAX_SLOTS];
UWORD      LIGHT_slot_free;

//
// The cache elements.
//

typedef struct
{
	THING_INDEX me;			// The thing whose lighting this is for.
	LIGHT_Index light[3];	// The lights shining on this thing.
	UBYTE       num_lights;
	UWORD       next;

} LIGHT_Cache;

#ifdef	PSX
#define LIGHT_MAX_CACHES 128
#else
#define LIGHT_MAX_CACHES 1280
#endif

LIGHT_Cache LIGHT_cache[LIGHT_MAX_CACHES];
UBYTE       LIGHT_cache_free;


//
// The lit point colours.
//

LIGHT_Colour LIGHT_point_colour[LIGHT_MAX_POINTS];
SLONG        LIGHT_point_colour_upto;



//
// Builds the free list of lights.
//

void LIGHT_build_free_list(void)
{
	SLONG i;

	LIGHT_free = 1;

	for (i = 1; i < LIGHT_MAX_LIGHTS - 1; i++)
	{
		LIGHT_light[i].next = i + 1;
	}

	LIGHT_light[LIGHT_MAX_LIGHTS - 1].next = 0;
}

//
// Gives and gets lights.
//

UBYTE LIGHT_get(void)
{
	if (LIGHT_free == NULL)
	{
		return NULL;
	}
	else
	{
		UBYTE ans = LIGHT_free;

		ASSERT(WITHIN(LIGHT_free, 1, LIGHT_MAX_LIGHTS - 1));

		LIGHT_free = LIGHT_light[LIGHT_free].next;

		return ans;
	}
}

void LIGHT_give(UBYTE l_index)
{
	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	LIGHT_light[l_index].next = LIGHT_free;
	LIGHT_free                = l_index;
}


//
// Places, removes and moves a light on the light mapwho.
//

void LIGHT_map_place(LIGHT_Index l_index)
{
	SLONG map_x;
	SLONG map_z;

	LIGHT_Light  *ll;
	LIGHT_Square *ls;

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// The light must be on the map!
	//

	map_x = LIGHT_TO_MAP(ll->pos.X);
	map_z = LIGHT_TO_MAP(ll->pos.Z);

	ASSERT(WITHIN(map_x, 0, LIGHT_MAP_SIZE - 1));
	ASSERT(WITHIN(map_z, 0, LIGHT_MAP_SIZE - 1));

	//
	// The map square.
	// 

	ls = &LIGHT_map[map_x][map_z];
	
	//
	// Place it on the map.
	//

	ll->next = ls->next;
	ls->next = l_index;
}

void LIGHT_map_remove(LIGHT_Index l_index)
{
	SLONG map_x;
	SLONG map_z;

	LIGHT_Light  *ll;
	LIGHT_Square *ls;
	UBYTE         next;
	UBYTE        *prev;

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// The light must be on the map!
	//

	map_x = LIGHT_TO_MAP(ll->pos.X);
	map_z = LIGHT_TO_MAP(ll->pos.Z);

	ASSERT(WITHIN(map_x, 0, LIGHT_MAP_SIZE - 1));
	ASSERT(WITHIN(map_z, 0, LIGHT_MAP_SIZE - 1));

	//
	// The map square.
	// 

	ls = &LIGHT_map[map_x][map_z];
	
	//
	// Look for this light in the linked list above this square.
	//

	prev = &ls->next;
	next =  ls->next;

	while(1)
	{
		//
		// This ASSERT catches the end of the linked list.
		//

		ASSERT(WITHIN(next, 1, LIGHT_MAX_LIGHTS - 1));

		ll = &LIGHT_light[next];

		if (next == l_index)
		{
			//
			// Found the light to take out of the linked list.
			//

		   *prev = ll->next;

		    return;
		}

		prev = &ll->next;
		next =  ll->next;
	}
}

void LIGHT_map_move(UBYTE l_index, GameCoord newpos)
{
	SLONG map_x_old;
	SLONG map_z_old;
	SLONG map_x_new;
	SLONG map_z_new;

	LIGHT_Light *ll;

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// Does its mapwho change?
	//

	map_x_old = LIGHT_TO_MAP(ll->pos.X);
	map_z_old = LIGHT_TO_MAP(ll->pos.Z);

	map_x_new = LIGHT_TO_MAP(newpos.X);
	map_z_new = LIGHT_TO_MAP(newpos.Z);

	if (map_x_old == map_x_new &&
		map_z_old == map_z_new)
	{
		//
		// This is an easy function!
		//

		ll->pos = newpos;
	}
	else
	{
		//
		// Still pretty easy!
		//

		LIGHT_map_remove(l_index);
		ll->pos = newpos;
		LIGHT_map_place(l_index);
	}
}

//
// Clears the map.
//

void LIGHT_map_clear(void)
{
	SLONG x;
	SLONG z;

	for (x = 0; x < LIGHT_MAP_SIZE; x++)
	for (z = 0; z < LIGHT_MAP_SIZE; z++)
	{
		LIGHT_map[x][z].next = 0;
	}
}

//
// Makes up the free list of slots.
//

void LIGHT_slot_build_free_list(void)
{
	SLONG i;

	LIGHT_slot_free = 1;

	for (i = 1; i < LIGHT_MAX_SLOTS - 1; i++)
	{
		LIGHT_slot[i].next = i + 1;
	}

	LIGHT_slot[LIGHT_MAX_SLOTS - 1].next = 0;
}

//
// Gets an unused slot.
//

UBYTE LIGHT_slot_get(void)
{
	UBYTE ans = LIGHT_slot_free;

	if (ans == NULL)
	{
		return NULL;
	}
	else
	{
		ASSERT(WITHIN(ans, 1, LIGHT_MAX_SLOTS - 1));

		LIGHT_slot_free = LIGHT_slot[ans].next;

		return ans;
	}
}
	
//
// Gives up a slot.
//

void LIGHT_slot_give(UBYTE s_index)
{
	ASSERT(WITHIN(s_index, 1, LIGHT_MAX_SLOTS - 1));

	LIGHT_slot[s_index].next = LIGHT_slot_free;
	LIGHT_slot_free          = s_index;
}


//
// Makes up the free list of cache entries.
//

void LIGHT_cache_build_free_list(void)
{
	SLONG i;

	LIGHT_cache_free = 1;

	for (i = 1; i < LIGHT_MAX_CACHES - 1; i++)
	{
		LIGHT_cache[i].next = i + 1;
	}

	LIGHT_cache[LIGHT_MAX_CACHES - 1].next = 0;
}

//
// Gets an unused cache entry.
//

UBYTE LIGHT_cache_get(UBYTE c_index)
{
	UBYTE ans;

	ans = LIGHT_cache_free;

	if (ans == NULL)
	{
		return NULL;
	}
	else
	{
		ASSERT(WITHIN(ans, 1, LIGHT_MAX_CACHES - 1));

		LIGHT_cache_free = LIGHT_cache[ans].next;

		return ans;
	}
}

//
// Gives back a dead cache entry.
//

void LIGHT_cache_give(UBYTE c_index)
{
	ASSERT(WITHIN(c_index, 1, LIGHT_MAX_CACHES - 1));

	LIGHT_cache[c_index].next = LIGHT_cache_free;
	LIGHT_cache_free          = c_index;
}


void LIGHT_set_hf(LIGHT_Map *map)
{
	LIGHT_hf = *map;
}


void LIGHT_set_ambient(
		LIGHT_Colour amb_colour,
		SLONG        amb_norm_x,
		SLONG        amb_norm_y,
		SLONG        amb_norm_z)
{
	LIGHT_amb_colour     = amb_colour;
	LIGHT_amb_norm_x     = amb_norm_x;
	LIGHT_amb_norm_y     = amb_norm_y;
	LIGHT_amb_norm_z     = amb_norm_z;
}

//
// Lights up/down the given building.
//

void LIGHT_building_up(LIGHT_Index l_index, THING_INDEX t_index)
{
	SLONG i;
	SLONG facet;
	SLONG point;

	SLONG dpx;
	SLONG dpy;
	SLONG dpz;

	SLONG lposx;
	SLONG lposy;
	SLONG lposz;

	SLONG dprod;
	SLONG dist;
	SLONG range;

	LIGHT_Light *ll;

	Thing *p_thing = TO_THING(t_index);

	PrimPoint    *pp;
	SVector      *pn;
	LIGHT_Colour *pc;

	BuildingObject *bo;
	BuildingFacet  *bf;

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// The range of the light.
	//

	range = ll->range * LIGHT_MAX_RANGE >> 8;

	//
	// The light position in relative to the building.
	//

	lposx = ll->pos.X - (p_thing->WorldPos.X >> 8);
	lposy = ll->pos.Y - (p_thing->WorldPos.Y >> 8);
	lposz = ll->pos.Z - (p_thing->WorldPos.Z >> 8);

	bo = &building_objects[p_thing->BuildingList];

	for (facet = bo->FacetHead; facet; facet = bf->NextFacet)
	{
		bf = &building_facets[facet];

		//
		// Light each point.
		//

		for (point = bf->StartPoint; point < bf->EndPoint; point++)
		{
			pp = &prim_points         [point];
			pn = &prim_normal         [point];
			pc = &LIGHT_building_point[point];

			//
			// Do the lighting.
			//

			dpx = pp->X - lposx;
			dpy = pp->Y - lposy;
			dpz = pp->Z - lposz;

			dist = QDIST3(abs(dpx), abs(dpy), abs(dpz));

			if (dist < range)
			{
				//
				// The angle the light hits.
				//

				dprod   =  pn->X*dpx + pn->Y*dpy + pn->Z*dpz;
				dprod  /=  dist;
				dprod   =  dprod * (256 - ((dist * 256) / range)) >> 8;
				dprod   = -dprod;

				if (dprod > 0)
				{
					#if LIGHT_COLOURED
					pc->red   += ll->colour.red   * dprod >> 8;
					pc->green += ll->colour.green * dprod >> 8;
					pc->blue  += ll->colour.blue  * dprod >> 8;
					#else
				   *pc        += ll->colour       * dprod >> 8;
					#endif
				}
			}
		}
	}
}


void LIGHT_building_down(LIGHT_Index l_index, THING_INDEX t_index)
{
	SLONG i;
	SLONG facet;
	SLONG point;

	SLONG dpx;
	SLONG dpy;
	SLONG dpz;

	SLONG lposx;
	SLONG lposy;
	SLONG lposz;

	SLONG dprod;
	SLONG dist;
	SLONG range;

	LIGHT_Light *ll;

	Thing *p_thing = TO_THING(t_index);

	PrimPoint    *pp;
	SVector      *pn;
	LIGHT_Colour *pc;

	BuildingObject *bo;
	BuildingFacet  *bf;

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// The range of the light.
	//

	range = ll->range * LIGHT_MAX_RANGE >> 8;

	//
	// The light position in relative to the building.
	//

	lposx = ll->pos.X - (p_thing->WorldPos.X >> 8);
	lposy = ll->pos.Y - (p_thing->WorldPos.Y >> 8);
	lposz = ll->pos.Z - (p_thing->WorldPos.Z >> 8);

	bo = &building_objects[p_thing->BuildingList];

	for (facet = bo->FacetHead; facet; facet = bf->NextFacet)
	{
		bf = &building_facets[facet];

		//
		// Light each point.
		//

		for (point = bf->StartPoint; point < bf->EndPoint; point++)
		{
			pp = &prim_points         [point];
			pn = &prim_normal         [point];
			pc = &LIGHT_building_point[point];

			//
			// Do the lighting.
			//

			dpx = pp->X - lposx;
			dpy = pp->Y - lposy;
			dpz = pp->Z - lposz;

			dist = QDIST3(abs(dpx), abs(dpy), abs(dpz));

			if (dist < range)
			{
				//
				// The angle the light hits.
				//

				dprod   =  pn->X*dpx + pn->Y*dpy + pn->Z*dpz;
				dprod  /=  dist;
				dprod   =  dprod * (256 - ((dist * 256) / range)) >> 8;
				dprod   = -dprod;

				if (dprod > 0)
				{
					#if LIGHT_COLOURED
					pc->red   -= ll->colour.red   * dprod >> 8;
					pc->green -= ll->colour.green * dprod >> 8;
					pc->blue  -= ll->colour.blue  * dprod >> 8;
					#else
				   *pc        -= ll->colour       * dprod >> 8;
					#endif
				}
			}
		}
	}
}


//
// Removes a light from the hf
//

void LIGHT_hf_light_up(LIGHT_Index l_index)
{
#ifdef TARGET_DC
	// Shouldn't be using this, apparently.
	ASSERT ( FALSE );
#endif
	SLONG x;
	SLONG y;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG building;
	SLONG storey;
	SLONG wall;

	SLONG v_list;
	SLONG i_vect;

	SLONG dist;
	SLONG range;
	SLONG brightness;
	UWORD litkey;

	LIGHT_Light   *ll;
	LIGHT_Colour   col;
	CollisionVect *p_vect;

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// When we light up a building, we put this value in 'LastDrawn'
	// to mark the building as lit.
	//

	litkey = rand();

	//
	// The range of the light.
	//

	range = ll->range * LIGHT_MAX_RANGE >> 8;

	x1 = ll->pos.X - range;
	z1 = ll->pos.Z - range;
	x2 = ll->pos.X + range;
	z2 = ll->pos.Z + range;

	x1 >>= 8;
	z1 >>= 8;

	x2 = (x2 + 0xff) >> 8;
	z2 = (z2 + 0xff) >> 8;

	for (mx = x1; mx <= x2; mx += 1)
	for (mz = z1; mz <= z2; mz += 1)
	{
		x = mx << 8;
		z = mz << 8;
		y = LIGHT_hf.get_height(mx, mz);

		dx = abs(ll->pos.X - x);
		dy = abs(ll->pos.Y - y);
		dz = abs(ll->pos.Z - z);

		dist = QDIST3(dx, dy, dz);

		brightness = 256 - ((dist * 256) / range);	// Shouldn't we convert this to a multiply?

		if (brightness > 0)
		{
			col = LIGHT_hf.get_light(mx, mz);

			#if LIGHT_COLOURED

			col.red   += ll->colour.red   * brightness >> 8;
			col.green += ll->colour.green * brightness >> 8;
			col.blue  += ll->colour.blue  * brightness >> 8;

			#else

			col += ll->colour * brightness >> 8;

			#endif

			LIGHT_hf.set_light(mx, mz, col);
		}

		//
		// Look for a building we might have to light.
		//

		v_list = MAP[MAP_INDEX(mx,mz)].ColVectHead;

		if (v_list)
		{
			i_vect =  col_vects_links[v_list].VectIndex;
			p_vect = &col_vects[i_vect];

			wall     = -p_vect->Face;
			storey   =  wall_list[wall].StoreyHead;
			building =  storey_list[storey].BuildingHead;

			if (building_list[building].LastDrawn != litkey)
			{
				//
				// Light up this building.
				//

				LIGHT_building_up(l_index, building_list[building].ThingIndex);

				//
				// Mark the building as list.
				//

				building_list[building].LastDrawn = litkey;
			}
		}
	}
}

void LIGHT_hf_light_down(LIGHT_Index l_index)
{
#ifdef TARGET_DC
	// Shouldn't be using this, apparently.
	ASSERT ( FALSE );
#endif
	SLONG x;
	SLONG y;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG building;
	SLONG storey;
	SLONG wall;

	SLONG v_list;
	SLONG i_vect;

	SLONG dist;
	SLONG range;
	SLONG brightness;
	UWORD litkey;

	LIGHT_Colour   col;
	LIGHT_Light   *ll;
	CollisionVect *p_vect;

	//
	// When we light up a building, we put this value in 
	//

	litkey = rand();

	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	ll = &LIGHT_light[l_index];

	//
	// The range of the light.
	//

	range = ll->range * LIGHT_MAX_RANGE >> 8;

	x1 = ll->pos.X - range;
	z1 = ll->pos.Z - range;
	x2 = ll->pos.X + range;
	z2 = ll->pos.Z + range;

	x1 >>= 8;
	z1 >>= 8;

	x2 = (x2 + 0xff) >> 8;
	z2 = (z2 + 0xff) >> 8;

	for (mx = x1; mx <= x2; mx += 1)
	for (mz = z1; mz <= z2; mz += 1)
	{
		x = mx << 8;
		z = mz << 8;
		y = LIGHT_hf.get_height(mx, mz);

		dx = abs(ll->pos.X - x);
		dy = abs(ll->pos.Y - y);
		dz = abs(ll->pos.Z - z);

		dist = QDIST3(dx, dy, dz);

		brightness = 256 - ((dist * 256) / range);	// Shouldn't we convert this to a multiply?

		if (brightness > 0)
		{
			col = LIGHT_hf.get_light(mx, mz);

			#if LIGHT_COLOURED

			col.red   -= ll->colour.red   * brightness >> 8;
			col.green -= ll->colour.green * brightness >> 8;
			col.blue  -= ll->colour.blue  * brightness >> 8;

			#else

			col -= ll->colour * brightness >> 8;

			#endif

			LIGHT_hf.set_light(mx, mz, col);
		}

		//
		// Look for a building we might have to light down.
		//

		v_list = MAP[MAP_INDEX(mx,mz)].ColVectHead;

		if (v_list)
		{
			i_vect =  col_vects_links[v_list].VectIndex;
			p_vect = &col_vects[i_vect];

			wall     = -p_vect->Face;
			storey   =  wall_list[wall].StoreyHead;
			building =  storey_list[storey].BuildingHead;

			if (building_list[building].LastDrawn != litkey)
			{
				//
				// Light down this building.
				//

				LIGHT_building_down(l_index, building_list[building].ThingIndex);

				//
				// Mark the building as list.
				//

				building_list[building].LastDrawn = litkey;
			}
		}

	}
}

void LIGHT_recalc_hf(void)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG ho;
	SLONG h1;
	SLONG h2;

	SLONG a;
	SLONG ao;
	SLONG a1;
	SLONG a2;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG ny2;
	SLONG dprod;

	LIGHT_Colour col;

	ASSERT(LIGHT_hf.get_height);
	ASSERT(LIGHT_hf.get_light);
	ASSERT(LIGHT_hf.set_light);

	//
	// Clear the ambient light on the hf.
	//

	#if LIGHT_COLOURED
	col.red   = 0;
	col.green = 0;
	col.blue  = 0;
	#else
	col       = 0;
	#endif

	for (x = 0; x < LIGHT_hf.width;  x++)
	for (z = 0; z < LIGHT_hf.height; z++)
	{
		LIGHT_hf.set_light(x, z, col);
	}

	//
	// Puts the ambient light on the map.
	//

	for (x = 1; x < LIGHT_hf.width  - 1; x++)
	for (z = 1; z < LIGHT_hf.height - 1; z++)
	{
		//
		// The height of this point.
		// 

		ho = LIGHT_hf.get_height(x,z);

		//
		// Find an approximate normal for this point.
		//

		//
		// The x-component of the normal.
		//

		h1 = LIGHT_hf.get_height(x + 1, z) - ho;
		h2 = LIGHT_hf.get_height(x - 1, z) - ho;

		a1 = Arctan(h1, -0x100);
		a2 = Arctan(h2, -0x100);

		a  = 1024 - a1 - a2;
		ao = a2 + (a >> 1);

		nx = COS(ao) >> 8;

		//
		// The z-component of the normal.
		//

		h1 = LIGHT_hf.get_height(x, z + 1) - ho;
		h2 = LIGHT_hf.get_height(x, z - 1) - ho;

		a1 = Arctan(h1, -0x100);
		a2 = Arctan(h2, -0x100);

		a  = 1024 - a1 - a2;
		ao = a2 + (a >> 1);

		nz = COS(ao) >> 8;

		//
		// Set the y-component of the normal so that
		// (nx,ny,nz) has a length of 0x100.
		//

		ny2 = 0x10000 - nx*nx - nz*nz;

		if (ny2 <= 0)
		{
			TRACE("ny * ny < 0\n");

			ny2 = 0;
		}
		else
		{
			ny = Root(ny2);
		}

		//
		// Find the intensity of ambient light on this point.
		//

		dprod   =  nx*LIGHT_amb_norm_x + ny*LIGHT_amb_norm_y + nz*LIGHT_amb_norm_z;
		dprod >>=  9;
		dprod   = -dprod;
		dprod  +=  128;

		SATURATE(dprod, 0, 256);

		#if LIGHT_COLOURED
		col.red   = LIGHT_amb_colour.red   * dprod >> 8;
		col.green = LIGHT_amb_colour.green * dprod >> 8;
		col.blue  = LIGHT_amb_colour.blue  * dprod >> 8;
		#else
		col       = LIGHT_amb_colour       * dprod >> 8;
		#endif

		LIGHT_hf.set_light(x, z, col);
	}

	{
		//
		// The ambient light on all the building objects.
		//

		SLONG facet;
		SLONG point;

		SVector normal;

		BuildingFacet  *bf;
		BuildingObject *bo;

		PrimPoint    *pp;
		SVector      *pn;
		LIGHT_Colour *pc;

		for (i = 1; i < next_building_object; i++)
		{
			bo = &building_objects[i];

			for (facet = bo->FacetHead; facet; facet = bf->NextFacet)
			{
				bf = &building_facets[facet];

				//
				// Light each point.
				//

				for (point = bf->StartPoint; point < bf->EndPoint; point++)
				{
					pn = &prim_normal         [point];
					pc = &LIGHT_building_point[point];

					//
					// Do the lighting.
					//

					dprod   =  pn->X*LIGHT_amb_norm_x + pn->Y*LIGHT_amb_norm_y + pn->Z*LIGHT_amb_norm_z;
					dprod >>=  9;
					dprod   = -dprod;
					dprod  +=  128;

					SATURATE(dprod, 0, 256);

					#if LIGHT_COLOURED

					pc->red   = LIGHT_amb_colour.red   * dprod >> 8;
					pc->green = LIGHT_amb_colour.green * dprod >> 8;
					pc->blue  = LIGHT_amb_colour.blue  * dprod >> 8;

					#else

					pc->red   = LIGHT_amb_colour       * dprod >> 8;

					#endif
				}
				
				#if WE_WANT_LIGHTING_ON_A_PER_FACE_INSTEAD_OF_PER_POINT_BASIS

				//
				// Only do face4s...
				//

				for (face = bf->StartFace4; face < bf->EndFace4; face++)
				{
					f4 = &prim_faces4[face];

					//
					// Calculate the face normal.
					//

					calc_normal(face, &normal);

					//
					// Do the lighting.
					//

					dprod   =  normal.X*LIGHT_amb_norm_x + normal.Y*LIGHT_amb_norm_y + normal.Z*LIGHT_amb_norm_z;
					dprod >>=  9;
					dprod   = -dprod;
					dprod  +=  128;

					SATURATE(dprod, 0, 256);

					#if LIGHT_COLOURED

					f4->col.red   = LIGHT_amb_colour.red   * dprod >> 8;
					f4->col.green = LIGHT_amb_colour.green * dprod >> 8;
					f4->col.blue  = LIGHT_amb_colour.blue  * dprod >> 8;

					#else

					f4->col.red   = LIGHT_amb_colour       * dprod >> 8;

					#endif
				}

				#endif
			}
		}
	}

	for (i = 0; i < LIGHT_MAX_LIGHTS; i++)
	{
		if (LIGHT_light[i].type)
		{
			//
			// Place this light on the map.
			//

			LIGHT_hf_light_up(i);
		}
	}
}


void LIGHT_init()
{
	SLONG i;

	//
	// Mark all lights as unused.
	// 

	for (i = 0; i < LIGHT_MAX_LIGHTS; i++)
	{
		LIGHT_light[i].type = 0;
	}

	//
	// Create the free lists...
	//

	LIGHT_build_free_list      ();
	LIGHT_slot_build_free_list ();
	LIGHT_cache_build_free_list();

	//
	// Clears the mapwho.
	//

	LIGHT_map_clear();
}



LIGHT_Index LIGHT_create(
				GameCoord    where, //fix 8
				LIGHT_Colour colour,
				UBYTE        range,
				UBYTE        type,
				UBYTE        param)
{

	UBYTE l_index;

	LIGHT_Light *ll;

	//
	// Get a new light.
	//

	l_index = LIGHT_get();

	if (l_index == NULL)
	{
		//
		// No more lights left!
		//

		return NULL;
	}
	else
	{
		ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

		ll = &LIGHT_light[l_index];

		ll->type    = type;
		ll->param   = param;
		ll->range   = range;
		ll->colour  = colour;
		ll->pos.X   = where.X;
		ll->pos.Y   = where.Y;
		ll->pos.Z   = where.Z;
		ll->counter = 0;

		if (ll->type == LIGHT_TYPE_BROKEN)
		{
			//
			// Mark the light as being on.
			//

			ll->counter |= 0x80;	// Top bit set => light is on.
		}

		//
		// Place the light on the light mapwho.
		//

		LIGHT_map_place(l_index);

		//
		// Put the light on the hf.
		//

		LIGHT_hf_light_up(l_index);

		return l_index;
	}
}


void LIGHT_destroy(LIGHT_Index l_index)
{
	LIGHT_hf_light_down(l_index);
	LIGHT_map_remove(l_index);
	LIGHT_give(l_index);
}


//
// Gets/sets a light's position.
//

GameCoord LIGHT_pos_get(LIGHT_Index l_index)
{
	ASSERT(WITHIN(l_index, 1, LIGHT_MAX_LIGHTS - 1));

	return LIGHT_light[l_index].pos;
}

void LIGHT_pos_set(LIGHT_Index l_index, GameCoord newpos)
{
	ASSERT(WITHIN(l_index, 0, LIGHT_MAX_LIGHTS - 1));

	LIGHT_Light *ll = &LIGHT_light[l_index];

	if (IMPLIES(ll->type == LIGHT_TYPE_BROKEN, ll->counter & 0x80) &&
		IMPLIES(ll->type == LIGHT_TYPE_PULSE,  ll->counter > (ll->param >> 1)))
	{
		//
		// The light is on.
		//

		LIGHT_hf_light_down(l_index);
		LIGHT_map_move(l_index, newpos);
		LIGHT_hf_light_up(l_index);
	}
	else
	{
		//
		// The light is off.
		//

		LIGHT_map_move(l_index, newpos);
	}
}

//
// Processes all the lights.
//

void LIGHT_process()
{
	SLONG i;
	UBYTE just_on;
	UBYTE just_off;

	LIGHT_Light *ll;

	for (i = 0; i < LIGHT_MAX_LIGHTS; i++)
	{
		ll = &LIGHT_light[i];

		if (ll->type)
		{
			just_on  = FALSE;
			just_off = FALSE;

			if (ll->type == LIGHT_TYPE_PULSE)
			{
				//
				// If the counter > (ll->param / 2) then 
				// the light is off, otherwise it is on.
				//

				ll->counter += 1;

				if (ll->counter == (ll->param >> 1))
				{
					just_off = TRUE;
				}
				
				if (ll->counter >= ll->param)
				{
					ll->counter = 0;
					just_on     = TRUE;
				}
			}

			if (ll->type == LIGHT_TYPE_BROKEN)
			{
				//
				// The top bit of counter is whether the light is on or off.
				//

				SLONG countdown = ll->counter & 0x7f;

				if (countdown == 0)
				{
					countdown = rand() % ((ll->param >> 1) + 1);

					ll->counter ^= 0x80;
					ll->counter &= 0x80;
					ll->counter |= countdown;

					if (ll->counter & 0x80)
					{
						just_on = TRUE;
					}
					else
					{
						just_off = TRUE;
					}
				}
				else
				{
					countdown   -= 1;
					ll->counter &= 0x80;
					ll->counter |= countdown;
				}
			}

			if (just_on)
			{
				LIGHT_hf_light_up(i);
			}
			
			if (just_off)
			{
				LIGHT_hf_light_down(i);
			}
		}
	}
}



//
// Remember the light context, incase we need that info
// shortly afterwards in a call to LIGHT_prim.
//

#define LIGHT_MAX_PER_PRIM 8

THING_INDEX LIGHT_context_t_index;
LIGHT_Index LIGHT_context_l_index[LIGHT_MAX_PER_PRIM];
SLONG       LIGHT_context_l_num;
SLONG       LIGHT_context_gameturn;
SLONG       LIGHT_context_context;

SLONG LIGHT_get_context(THING_INDEX t_index)
{
	SLONG i;
	SLONG x;
	SLONG z;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG range;

	SLONG x1, z1;
	SLONG x2, z2;

	SLONG context;

	LIGHT_Index  next;
	LIGHT_Light *ll;

	Thing *p_thing = TO_THING(t_index);

	//
	// Find all the lights acting on this prim.
	//

	#define LIGHT_THING_SIZE   (256)
	#define LIGHT_SEARCH_RANGE (LIGHT_MAX_RANGE + LIGHT_THING_SIZE)

	x1 = LIGHT_TO_MAP((p_thing->WorldPos.X >> 8) - LIGHT_SEARCH_RANGE);
	z1 = LIGHT_TO_MAP((p_thing->WorldPos.Z >> 8) - LIGHT_SEARCH_RANGE);

	x2 = LIGHT_TO_MAP((p_thing->WorldPos.X >> 8) + LIGHT_SEARCH_RANGE);
	z2 = LIGHT_TO_MAP((p_thing->WorldPos.Z >> 8) + LIGHT_SEARCH_RANGE);

	SATURATE(x1, 0, LIGHT_MAP_SIZE - 1);
	SATURATE(z1, 0, LIGHT_MAP_SIZE - 1);
	SATURATE(x2, 0, LIGHT_MAP_SIZE - 1);
	SATURATE(z2, 0, LIGHT_MAP_SIZE - 1);

	LIGHT_context_t_index  = t_index;
	LIGHT_context_l_num    = 0;
	LIGHT_context_gameturn = GAME_TURN;

	for (x = x1; x <= x2; x++)
	for (z = z1; z <= z2; z++)
	{
		ASSERT(WITHIN(x, 0, LIGHT_MAP_SIZE - 1));
		ASSERT(WITHIN(z, 0, LIGHT_MAP_SIZE - 1));

		next = LIGHT_map[x][z].next;

		while(next)
		{
			ASSERT(WITHIN(next, 1, LIGHT_MAX_LIGHTS - 1));

			ll = &LIGHT_light[next];
			
			if (IMPLIES(ll->type == LIGHT_TYPE_BROKEN, ll->counter & 0x80) &&
				IMPLIES(ll->type == LIGHT_TYPE_PULSE,  ll->counter > (ll->param >> 1)))
			{
				//
				// What is the range of this light?
				//

				range = LIGHT_MAX_RANGE * ll->range >> 8;

				//
				// How far is the prim from the light?
				//

				dx = abs(ll->pos.X - (p_thing->WorldPos.X >> 8));
				dy = abs(ll->pos.Y - (p_thing->WorldPos.Y >> 8));
				dz = abs(ll->pos.Z - (p_thing->WorldPos.Z >> 8));

				dist = QDIST3(dx, dy, dz);

				if (dist <= range)
				{
					//
					// This light is acting on the prim.
					//

					if (LIGHT_context_l_num < LIGHT_MAX_PER_PRIM)
					{
						LIGHT_context_l_index[LIGHT_context_l_num++] = next;
					}
				}
			}

			next = ll->next;
		}
	}

	//
	// Work out the context.
	//

	context  = 0;
	context ^= LIGHT_context_t_index << 5;
	context ^= LIGHT_context_l_num   << 13;
	context ^= p_thing->WorldPos.X;
	context ^= p_thing->WorldPos.Z;

	if (p_thing->DrawType == DT_MESH)
	{
		context ^= p_thing->Draw.Mesh->Angle << 2;
		context ^= p_thing->Draw.Mesh->Tilt  << 5;
		context ^= p_thing->Draw.Mesh->Roll  << 9;
	}

	for (i = 0; i < LIGHT_context_l_num; i++)
	{
		ASSERT(WITHIN(LIGHT_context_l_index[i], 1, LIGHT_MAX_LIGHTS - 1));

		ll = &LIGHT_light[LIGHT_context_l_index[i]];

		context ^= ll->pos.X      << (i + 0);
		context ^= ll->pos.Y      << (i + 5);
		context ^= ll->pos.Z      << (i + 10);
		context ^= ll->range      << (i + 11);

		#if LIGHT_COLOURED
		context ^= ll->colour.red   << (8  - i);
		context ^= ll->colour.green << (16 - i);
		context ^= ll->colour.blue  << (24 - i);
		#else
		context ^= ll->colour       << (24 - i);
		#endif
	}

	LIGHT_context_context = context;

	return context;
}


LIGHT_Colour LIGHT_get_point(SLONG x, SLONG y, SLONG z)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;
	SLONG range;
	SLONG brightness;

	LIGHT_Light *ll;
	LIGHT_Colour ans;
	
	//
	// Since we have no normal info, assume that only
	// have the ambient light shines off this point.
	//

	#if LIGHT_COLOURED
	ans.red   = LIGHT_amb_colour.red   >> 1;
	ans.green = LIGHT_amb_colour.green >> 1;
	ans.blue  = LIGHT_amb_colour.blue  >> 1;
	#else
	ans = LIGHT_amb_colour;
	#endif

	//
	// Add the light from all the lights.
	//

	for (i = 0; i < LIGHT_context_l_num; i++)
	{
		ASSERT(WITHIN(LIGHT_context_l_index[i], 1, LIGHT_MAX_LIGHTS - 1));

		ll = &LIGHT_light[LIGHT_context_l_index[i]];

		//
		// What is the range of this light?
		//

		range = LIGHT_MAX_RANGE * ll->range >> 8;

		//
		// Light each point.
		//

		dx = abs(x - ll->pos.X);
		dy = abs(y - ll->pos.Y);
		dz = abs(z - ll->pos.Z);

		dist = QDIST3(dx, dy, dz);

		brightness = 256 - ((dist * 256) / range);	// Shouldn't we convert this to a multiply?

		if (brightness > 0)
		{
			#if LIGHT_COLOURED

			ans.red   += ll->colour.red   * brightness >> 8;
			ans.green += ll->colour.green * brightness >> 8;
			ans.blue  += ll->colour.blue  * brightness >> 8;

			#else

			ans += ll->colour * brightness >> 8;

			#endif
		}
	}

	return ans;
}




void LIGHT_prim(THING_INDEX t_index)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;
	SLONG range;
	SLONG brightness;

	SLONG x1;
	SLONG x2;
	SLONG z1;
	SLONG z2;

	UBYTE next;

	SLONG prim;
	SLONG num_points;

	SLONG matrix[9];

	GameCoord    lp;

	DrawMesh    *dm;
	Thing       *p_thing = TO_THING(t_index);
	LIGHT_Light *ll;
	PrimObject  *p_obj;

	//
	// This must have a DT_MESH drawtype...
	//

	ASSERT(p_thing->DrawType == DT_MESH);
	ASSERT(WITHIN(p_thing->Draw.Mesh, &DRAW_MESHES[0], &DRAW_MESHES[MAX_DRAW_MESHES]));

	dm = p_thing->Draw.Mesh;

	//
	// Find out the lighting context of the thing, if we have to...
	//

	if (LIGHT_context_t_index  == t_index &&
		LIGHT_context_gameturn == GAME_TURN)
	{
		//
		// It is already worked out!
		//
	}
	else
	{
		LIGHT_get_context(t_index);
	}

	//
	// What is the prim?
	//

	ASSERT(WITHIN(dm->ObjectId, 1, next_prim_object - 1));

	p_obj = &prim_objects[dm->ObjectId];

	//
	// What is the rotation matrix of the prim?
	//

	FMATRIX_calc(matrix, dm->Angle, dm->Tilt, dm->Roll);

	//
	// Initialise all the point colours to the ambient light colour.
	//

	num_points = p_obj->EndPoint - p_obj->StartPoint;

	for (i = 0; i < num_points; i++)
	{
		LIGHT_point_colour[i] = LIGHT_amb_colour;
	}

	//
	// Remember how many points we have.
	//

	LIGHT_point_colour_upto = num_points;

	//
	// Go through all the lights that shine on this prim.
	//

	for (i = 0; i < LIGHT_context_l_num; i++)
	{
		ASSERT(WITHIN(LIGHT_context_l_index[i], 1, LIGHT_MAX_LIGHTS - 1));

		ll = &LIGHT_light[LIGHT_context_l_index[i]];

		//
		// What is the range of this light?
		//

		range = LIGHT_MAX_RANGE * ll->range >> 8;

		//
		// Rotate this light into the space of the prim.
		//

		lp.X = ll->pos.X - (p_thing->WorldPos.X >> 8);
		lp.Y = ll->pos.Y - (p_thing->WorldPos.Y >> 8);
		lp.Z = ll->pos.Z - (p_thing->WorldPos.Z >> 8);

		FMATRIX_MUL(
			matrix,
			lp.X,
			lp.Y,
			lp.Z);

		//
		// Light each point.
		//

		for (j = 0; j < num_points; j++)
		{
			dx = abs(prim_points[i + p_obj->StartPoint].X - lp.X);
			dy = abs(prim_points[i + p_obj->StartPoint].Y - lp.Y);
			dz = abs(prim_points[i + p_obj->StartPoint].Z - lp.Z);

			dist = QDIST3(dx, dy, dz);

			brightness = 256 - ((dist * 256) / range);	// Shouldn't we convert this to a multiply?

			if (brightness > 0)
			{
				#if LIGHT_COLOURED

				LIGHT_point_colour[j].red   += ll->colour.red   * brightness >> 8;
				LIGHT_point_colour[j].green += ll->colour.green * brightness >> 8;
				LIGHT_point_colour[j].blue  += ll->colour.blue  * brightness >> 8;

				#else

				LIGHT_point_colour[j] += ll->colour * brightness >> 8;

				#endif
			}
		}
	}
}


void LIGHT_prim_use_normals(THING_INDEX t_index)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;
	SLONG range;
	SLONG brightness;

	SLONG amb_x;
	SLONG amb_y;
	SLONG amb_z;

	SLONG x1;
	SLONG x2;
	SLONG z1;
	SLONG z2;

	SLONG dprod;

	UBYTE next;

	SLONG prim;
	SLONG num_points;

	SLONG matrix[9];

	GameCoord    lp;

	DrawMesh    *dm;
	Thing       *p_thing = TO_THING(t_index);
	LIGHT_Light *ll;
	PrimObject  *p_obj;
	PrimNormal  *p_normal;
	PrimPoint   *p_point;
	SVector      l_normal;

	//
	// This must have a DT_MESH drawtype...
	//

	ASSERT(p_thing->DrawType == DT_MESH);
	ASSERT(WITHIN(p_thing->Draw.Mesh, &DRAW_MESHES[0], &DRAW_MESHES[MAX_DRAW_MESHES]));

	dm = p_thing->Draw.Mesh;

	//
	// Find out the lighting context of the thing, if we have to...
	//

	if (LIGHT_context_t_index  == t_index &&
		LIGHT_context_gameturn == GAME_TURN)
	{
		//
		// It is already worked out!
		//
	}
	else
	{
		LIGHT_get_context(t_index);
	}

	//
	// What is the prim?
	//

	ASSERT(WITHIN(dm->ObjectId, 1, next_prim_object - 1));

	p_obj = &prim_objects[dm->ObjectId];

	//
	// What is the rotation matrix of the prim?
	//

	FMATRIX_calc(matrix, dm->Angle, dm->Tilt, dm->Roll);

	//
	// Rotate the ambient light into the space of the prim.
	//

	amb_x = LIGHT_amb_norm_x;
	amb_y = LIGHT_amb_norm_y;
	amb_z = LIGHT_amb_norm_z;

	FMATRIX_MUL(
		matrix,
		amb_x,
		amb_y,
		amb_z);

	//
	// Initialise all the point colours to the ambient light colour.
	//

	num_points = p_obj->EndPoint - p_obj->StartPoint;

	for (i = 0; i < num_points; i++)
	{
		dprod =
			amb_x * prim_normal[p_obj->StartPoint + i].X + 
			amb_y * prim_normal[p_obj->StartPoint + i].Y + 
			amb_z * prim_normal[p_obj->StartPoint + i].Z;

		dprod >>= 9;
		dprod   = -dprod;
		dprod  +=  128;

		#if LIGHT_COLOURED
		LIGHT_point_colour[i].red   = LIGHT_amb_colour.red   * dprod >> 8;
		LIGHT_point_colour[i].green = LIGHT_amb_colour.green * dprod >> 8;
		LIGHT_point_colour[i].blue  = LIGHT_amb_colour.blue  * dprod >> 8;
		#else
		LIGHT_point_colour[i] = LIGHT_amb_colour * dprod >> 8;
		#endif
	}

	//
	// Remember how many points we have.
	//

	LIGHT_point_colour_upto = num_points;

	//
	// Go through all the lights that shine on this prim.
	//

	for (i = 0; i < LIGHT_context_l_num; i++)
	{
		ASSERT(WITHIN(LIGHT_context_l_index[i], 1, LIGHT_MAX_LIGHTS - 1));

		ll = &LIGHT_light[LIGHT_context_l_index[i]];

		//
		// What is the range of this light?
		//

		range = LIGHT_MAX_RANGE * ll->range >> 8;

		//
		// Rotate this light into the space of the prim.
		//

		lp.X = ll->pos.X - (p_thing->WorldPos.X >> 8);
		lp.Y = ll->pos.Y - (p_thing->WorldPos.Y >> 8);
		lp.Z = ll->pos.Z - (p_thing->WorldPos.Z >> 8);

		FMATRIX_MUL_BY_TRANSPOSE(
			matrix,
			lp.X,
			lp.Y,
			lp.Z);

		//
		// The normalised vector from the light to the centre of the prim.
		//

		l_normal.X = -lp.X;
		l_normal.Y = -lp.Y;
		l_normal.Z = -lp.Z;

		dx = abs(l_normal.X);
		dy = abs(l_normal.Y);
		dz = abs(l_normal.Z);

		dist = QDIST3(dx, dy, dz);

		l_normal.X <<= 8;
		l_normal.Y <<= 8;
		l_normal.Z <<= 8;

		l_normal.X /= dist;
		l_normal.Y /= dist;
		l_normal.Z /= dist;

		//
		// Light each point.
		//

		for (j = 0; j < num_points; j++)
		{
			p_point  = &prim_points[j + p_obj->StartPoint];
			p_normal = &prim_normal[j + p_obj->StartPoint];

			//
			// Take the light vector and the point normal into account.
			//

			dprod = l_normal.X * p_normal->X +  l_normal.Y * p_normal->Y + l_normal.Z * p_normal->Z >> 8;

			if (dprod > 0)
			{
				//
				// Distance from the light.
				//

				dx = lp.X - p_point->X;
				dy = lp.Y - p_point->Y;
				dz = lp.Z - p_point->Z;

				dx = abs(dx);
				dy = abs(dy);
				dz = abs(dz);

				dist = QDIST3(dx, dy, dz);

				brightness = 256 - ((dist * 256) / range);	// Shouldn't we convert this to a multiply?
				brightness = brightness * dprod >> 8;

				if (brightness > 0)
				{
					#if LIGHT_COLOURED

					LIGHT_point_colour[j].red   += ll->colour.red   * brightness >> 8;
					LIGHT_point_colour[j].green += ll->colour.green * brightness >> 8;
					LIGHT_point_colour[j].blue  += ll->colour.blue  * brightness >> 8;

					#else

					LIGHT_point_colour[j] += ll->colour * brightness >> 8;

					#endif
				}
			}
		}
	}
}



// ########################################################
// ========================================================
// """"""""""""""""""""""""""""""""""""""""""""""""""""""""
// OLD LIGHT STUFF
// ........................................................
// ========================================================
// ########################################################


#ifdef	EDITOR
#include	"c:\fallen\editor\headers\scan.h"
extern	void	scan_undo_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);
extern	void	apply_ambient_to_floor(void);
extern	void	remove_ambient_from_floor(void);
extern	void	scan_apply_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);
#endif

void	apply_global_amb_to_map(void)
{
#ifdef	EDITOR

	scan_function=scan_undo_ambient;
	scan_map();	
	remove_ambient_from_floor();
extern	void	setup_ambient(SLONG dx,SLONG dy,SLONG dz,SLONG bright,SLONG flags);
// 	setup_ambient(100,100,-70,1024,2);
 	setup_ambient(90,-100,-90,655,2);
	scan_function=scan_apply_ambient;
	scan_map();	
	apply_ambient_to_floor();
#endif
	
}
#endif