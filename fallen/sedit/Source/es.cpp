//
// The editor for the sewers.
//

#include <MFStdLib.h>
#include "structs.h"
#include "building.h"
#include "supermap.h"
#include "es.h"
#include "ns.h"
#include "c:\fallen\ddengine\headers\aeng.h"
#include "ob.h"
#include	"c:\fallen\headers\memory.h"



//
// The map.
// 

ES_Hi ES_hi[PAP_SIZE_HI][PAP_SIZE_HI];
ES_Lo ES_lo[PAP_SIZE_LO][PAP_SIZE_LO];

ES_Thing ES_thing[ES_MAX_THINGS];


//
// Water in the city.
//

UBYTE ES_city_water_on   [PAP_SIZE_HI][PAP_SIZE_HI];
SBYTE ES_city_water_level[PAP_SIZE_LO][PAP_SIZE_LO];


//
// Encapsulates the state of the editor.
//

typedef struct
{
	ES_Hi    es_hi              [PAP_SIZE_HI][PAP_SIZE_HI];
	ES_Lo    es_lo              [PAP_SIZE_LO][PAP_SIZE_LO];
	ES_Thing es_thing           [ES_MAX_THINGS];
	UBYTE    es_city_water_on   [PAP_SIZE_HI][PAP_SIZE_HI];
	SBYTE    es_city_water_level[PAP_SIZE_LO][PAP_SIZE_LO];

} ES_Undo;

#define ES_MAX_UNDO 64

ES_Undo ES_undo[ES_MAX_UNDO];
SLONG   ES_undo_top;	// A circular system: Access these values MOD ES_MAX_UNDO
SLONG   ES_undo_bot;
SLONG   ES_undo_stage;




void ES_init()
{
	SLONG i;

	SLONG x;
	SLONG z;

	ES_Hi *eh;
	ES_Lo *el;

	//
	// All things are unused.
	//

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		ES_thing[i].type = ES_THING_TYPE_UNUSED;
	}

	//
	// Make the map full of rock.
	//

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		eh = &ES_hi[x][z];

		eh->type    = ES_TYPE_ROCK;
		eh->height  = 224;
		eh->flag    = 0;
		eh->water   = 0;
	}

	//
	// Turn off all lights.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		el = &ES_lo[x][z];

		el->light_x = 64;
		el->light_z = 64;
		el->light_y = 0;
	}

	//
	// No water in the city.
	// 

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		ES_city_water_on[x][z] = FALSE;
	}

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		ES_city_water_level[x][z] = -127;
	}

	//
	// Lose all undo info except the initial state.
	//

	ES_undo_bot   =  0;
	ES_undo_top   =  0;
	ES_undo_stage = -1;

	ES_undo_store();
}



void ES_change_height(
		SLONG map_x,
		SLONG map_z,
		SLONG dheight)
{
	SLONG i;
	SLONG dx;
	SLONG dz;
	SLONG nx;
	SLONG nz;

	SLONG height;
	SLONG match;
	SLONG count;

	ES_Hi *eh;

	ASSERT(WITHIN(map_x, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(map_z, 0, PAP_SIZE_HI - 1));

	height  = ES_hi[map_x][map_z].height;
	match   = height;
	height += dheight;

	SATURATE(height, 8, 248);

	typedef struct
	{
		UBYTE x;
		UBYTE z;
		
	} Queue;

	#define QUEUE_SIZE 512

	Queue queue[QUEUE_SIZE];

	SLONG queue_start = 0;	// Access MOD QUEUE_SIZE
	SLONG queue_end   = 0;

	Queue *qs;
	Queue *qe;

	queue[0].x = map_x;
	queue[0].z = map_z;

	queue_start = 0;
	queue_end   = 1;
	
	count = 0;

	while(queue_start < queue_end)
	{
		count += 1;

		if (count > 500)
		{
			//
			// There is the possibility of a infinite loop here.
			//

			return;
		}

		qs = &queue[queue_start & (QUEUE_SIZE - 1)];

		ASSERT(WITHIN(qs->x, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(qs->z, 0, PAP_SIZE_HI - 1));

		eh = &ES_hi[qs->x][qs->z];
		
		eh->height = height;

		if (eh->type == ES_TYPE_SEWER || ShiftFlag)
		{
			//
			// Look for neigbouring sewers.
			//

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
					default:
						ASSERT(0);
						break;
				}

				nx = qs->x + dx;
				nz = qs->z + dz;

				if (WITHIN(nx, 0, PAP_SIZE_HI - 1) &&
					WITHIN(nx, 0, PAP_SIZE_HI - 1))
				{
					if (ES_hi[nx][nz].type   == eh->type &&
						ES_hi[nx][nz].height == match)
					{
						qe = &queue[queue_end & (QUEUE_SIZE - 1)];

						qe->x = nx;
						qe->z = nz;

						queue_end += 1;
					}
				}
			}
		}

		queue_start += 1;
	}
}


//
// Sets the water-related flags on the PAP map from the ES_city_water maps
// 

void ES_city_water_build(void)
{
	SLONG i;

	SLONG x;
	SLONG z;

	OB_Info *oi;

	//
	// Clear the maps of all old water.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		PAP_2LO(x,z).water = PAP_LO_NO_WATER;
	}

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		if (PAP_2HI(x,z).Flags & PAP_FLAG_WATER)
		{
			PAP_2HI(x,z).Flags &= ~(PAP_FLAG_WATER); 
		}
	}

	//
	// Get the water level from the bottom of a prim 13 Ob.
	// 

	SLONG water_level = 0;

	for (i = 1; i < OB_ob_upto; i++)
	{
		if (OB_ob[i].prim == 13)
		{
			water_level = OB_ob[i].y + get_prim_info(13)->miny;
		}
	}

	//
	// Put water on the map where there is texture '1'.
	//

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		ES_city_water_on[x][z] = ((PAP_2HI(x,z).Texture & 0x3ff) == 1);
	}

	/*

	//
	// Put in the new water
	//

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		if (ES_city_water_on[x][z])
		{
			PAP_2HI(x + 0, z + 0).Flags |= PAP_FLAG_SEWER_POINT| PAP_FLAG_WATER | PAP_FLAG_TRENCH;
			PAP_2HI(x + 1, z + 0).Flags |= PAP_FLAG_SEWER_POINT;
			PAP_2HI(x + 0, z + 1).Flags |= PAP_FLAG_SEWER_POINT;
			PAP_2HI(x + 1, z + 1).Flags |= PAP_FLAG_SEWER_POINT;

			//PAP_2HI(x - 1, z - 1).Flags |= PAP_FLAG_REFLECTIVE;
			//PAP_2HI(x + 0, z - 1).Flags |= PAP_FLAG_REFLECTIVE;
			//PAP_2HI(x + 1, z - 1).Flags |= PAP_FLAG_REFLECTIVE;
			//PAP_2HI(x - 1, z + 0).Flags |= PAP_FLAG_REFLECTIVE;
			//PAP_2HI(x - 1, z + 1).Flags |= PAP_FLAG_REFLECTIVE;

			PAP_2LO(x >> 2, z >> 2).water = ES_city_water_level[x >> 2][z >> 2];
		}
	}

	*/

	for (x = 0; x < PAP_SIZE_LO - 1; x++)
	for (z = 0; z < PAP_SIZE_LO - 1; z++)
	{
		PAP_2LO(x,z).water = ES_city_water_level[x][z] = water_level >> 3;

		//
		// All watery obs at the water level.
		//

		for (oi= OB_find(x,z); oi->prim; oi += 1)
		{
			if (PAP_2HI(oi->x >> 8, oi->z >> 8).Flags & PAP_FLAG_WATER)
			{
				oi->y = water_level - get_prim_info(oi->prim)->miny;
			}
		}
	}
}



void ES_build_sewers()
{
	SLONG i ;

	SLONG x;
	SLONG z;

	SLONG f_list;
	SLONG facet;
	SLONG exit;
	SLONG num_links;
	SLONG num_facets;
	SLONG last_top;
	SLONG this_top;
	SLONG this_top_count;
	SLONG bottom;
	SLONG dx;
	SLONG dz;
	SLONG mx;
	SLONG mz;

	ES_Hi *eh;
	NS_Hi *nh;

	ES_Lo *el;
	NS_Lo *nl;

	ES_Thing *et;

	PAP_Lo *pl;

	SLONG nh_type;
	SLONG nh_flag;

	//
	// Initialise the NS map.
	//

	NS_init();
	NS_cache_init();

	//
	// Create the NS map from our ES map.
	//

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		eh = &ES_hi[x][z];
		nh = &NS_hi[x][z];

		switch(eh->type)
		{
			case ES_TYPE_ROCK:   nh_type = NS_HI_TYPE_ROCK;  break;
			case ES_TYPE_SEWER:	 nh_type = NS_HI_TYPE_SEWER; break;
			case ES_TYPE_GROUND: nh_type = NS_HI_TYPE_STONE; break;
			case ES_TYPE_HOLE:   nh_type = NS_HI_TYPE_STONE; break;
			default:
				ASSERT(0);
				break;
		}

		if (eh->flag & ES_FLAG_GRATING)
		{
			nh_flag = NS_HI_FLAG_GRATE;
		}
		else
		{
			nh_flag = 0;
		}

		nh->bot    = eh->height;
		nh->top    = eh->height;
		nh->water  = eh->water;
		nh->packed = nh_type | nh_flag;

		if (eh->type == ES_TYPE_HOLE)
		{
			//
			// Holes are given by making 'bot' zero.
			//

			nh->bot = 0;
		}
	}

	//
	// Create the top heights from the rocks squares.
	//

	last_top = 232;

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		eh = &ES_hi[x][z];
		nh = &NS_hi[x][z];

		this_top       = 0;
		this_top_count = 0;

		if (eh[PAP_SIZE_HI *  0 - 0].type == ES_TYPE_ROCK) {this_top_count += 1; this_top += eh[PAP_SIZE_HI *  0 - 0].height;}
		if (eh[PAP_SIZE_HI *  0 - 1].type == ES_TYPE_ROCK) {this_top_count += 1; this_top += eh[PAP_SIZE_HI *  0 - 1].height;}
		if (eh[PAP_SIZE_HI * -1 - 0].type == ES_TYPE_ROCK) {this_top_count += 1; this_top += eh[PAP_SIZE_HI * -1 - 0].height;}
		if (eh[PAP_SIZE_HI * -1 - 1].type == ES_TYPE_ROCK) {this_top_count += 1; this_top += eh[PAP_SIZE_HI * -1 - 1].height;}

		if (this_top_count)
		{
			this_top /= this_top_count;

			nh->top  = this_top;
			last_top = this_top;
		}
		else
		{
			nh->top = last_top;
		}
	}

	//
	// Put down the lights.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		el = &ES_lo[x][z];
		nl = &NS_lo[x][z];

		nl->light_x = el->light_x;
		nl->light_y = el->light_y;
		nl->light_z = el->light_z;
	}

	//
	// Get rid of all sewer ladders.
	//

	for (x = 0; x < PAP_SIZE_LO; x++)
	for (z = 0; z < PAP_SIZE_LO; z++)
	{
		pl = &PAP_2LO(x,z);

		//
		// Go through all the dfacets above this square.
		//

		if (pl->ColVectHead)
		{
			num_links  = 0;
			num_facets = 0;

			for (f_list = pl->ColVectHead, exit = FALSE; !exit; f_list += 1)
			{
				facet = facet_links[f_list];

				if (facet < 0)
				{
					//
					// This is the last facet.
					//

					facet = -facet;
					exit  =  TRUE;
				}

				num_facets += 1;

				ASSERT(WITHIN(facet, 1, next_dfacet - 1));

				if (dfacets[facet].FacetType == STOREY_TYPE_LADDER &&
				    dfacets[facet].FacetType == FACET_FLAG_IN_SEWERS)
				{
					//
					// Take out this facet.
					//

					facet_links[f_list] = 0;
				}
				else
				{
					num_links += 1;
				}

				if (exit)
				{
					if (num_links == 0)
					{
						//
						// No more colvects on this square- zero out any that
						// we once had.
						//

						f_list = pl->ColVectHead;

						for (i = 0; i < num_facets; i++)
						{
							facet_links[f_list++] = 0;
						}

						pl->ColVectHead = 0;
					}
					else
					{
						if (num_links == num_facets)
						{
							//
							// We didn't get rid of any ladders.
							//
						}
						else
						{
							//
							// This is the last link. squash up all the links we have.
							//

							f_list = pl->ColVectHead;

							for (i = 0; i < num_links; i++)
							{
								//
								// The next non-zero facet index.
								//

								while(facet_links[f_list] == 0)
								{
									f_list += 1;
								}

								facet_links[pl->ColVectHead + i] = facet_links[f_list];

								f_list += 1;

								if (i == num_links - 1)
								{
									//
									// Make the last link negative.
									//

									facet_links[pl->ColVectHead + i] = -abs(facet_links[pl->ColVectHead + i]);
								}
							}

							//
							// Zero out the remaning facet links.
							//

							for (; i < num_facets; i++)
							{
								facet_links[pl->ColVectHead + i] = 0;
							}
						}
					}
				}
			}
		}
	}

	//
	// Mark all the sewer ladder dfacets as unused.
	//

	for (i = 1; i < next_dfacet; i++)
	{
		if (dfacets[i].FacetType == STOREY_TYPE_LADDER &&
			dfacets[i].FacetType == FACET_FLAG_IN_SEWERS)
		{
			//
			// Take out this facet.
			//

			dfacets[i].FacetType = 0;
		}
	}

	//
	// Reduce next_dfacet if we can.
	//

	while(next_dfacet > 0 && dfacets[next_dfacet - 1].FacetType == 0)
	{
		next_dfacet -= 1;
	}

	//
	// Put down the things.
	//

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		switch(et->type)
		{
			case ES_THING_TYPE_UNUSED:
				break;

			case ES_THING_TYPE_LADDER:

				//
				// Find the middle square of this ladder.
				//

				mx = (et->x1 + et->x2 << 7) + ((et->z2 - et->z1) << 2) >> 8;
				mz = (et->z1 + et->z2 << 7) - ((et->x2 - et->x1) << 2) >> 8;

				ASSERT(WITHIN(mx, 0, PAP_SIZE_HI - 1));
				ASSERT(WITHIN(mz, 0, PAP_SIZE_HI - 1));

				bottom = (ES_hi[mx][mz].height << 5) + (32 * -0x100);

				add_sewer_ladder(
					et->x1 << 8, et->z1 << 8,
					et->x2 << 8, et->z2 << 8,
					bottom,
					et->height,
					ES_hi[mx][mz].flag & ES_FLAG_ENTRANCE);

				break;

			case ES_THING_TYPE_PRIM:
				
				NS_add_prim(
					et->prim,
					et->yaw,
					et->x,
					et->y,
					et->z);

				break;

			default:
				ASSERT(0);
				break;
		}
	}


	/*
	//
	// Put in the entrances.
	//

	for (x = 0; x < PAP_SIZE_HI; x++)
	for (z = 0; z < PAP_SIZE_HI; z++)
	{
		eh = &ES_hi[x][z];

		eh->flag &= ~(PAP_FLAG_SEWER_SQUARE | PAP_FLAG_SEWER_POINT | PAP_FLAG_NOCURBS);
	}

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		eh = &ES_hi[x][z];

		if (eh->flag & ES_FLAG_ENTRANCE)
		{
			PAP_hi[x][z].Flags |= PAP_FLAG_SEWER_SQUARE;

			PAP_hi[x + 0][z + 0].Flags |= PAP_FLAG_SEWER_POINT;
			PAP_hi[x + 1][z + 0].Flags |= PAP_FLAG_SEWER_POINT;
			PAP_hi[x + 0][z + 1].Flags |= PAP_FLAG_SEWER_POINT;
			PAP_hi[x + 1][z + 1].Flags |= PAP_FLAG_SEWER_POINT;

			if (eh->flag & ES_FLAG_NOCURBS)
			{
				PAP_hi[x][z].Flags |= PAP_FLAG_NOCURBS;
			}
		}
	}

	*/

	NS_precalculate();

	//
	// Put the the water above the city.
	// 

	ES_city_water_build();
}


//
// Fills out the given undo structure with the current state.
//

void ES_undo_create(ES_Undo *eu)
{
	memcpy(eu->es_hi,               ES_hi,               sizeof(ES_hi));
	memcpy(eu->es_lo,               ES_lo,               sizeof(ES_lo));
	memcpy(eu->es_thing,            ES_thing,            sizeof(ES_thing));
	memcpy(eu->es_city_water_on,    ES_city_water_on,    sizeof(ES_city_water_on));
	memcpy(eu->es_city_water_level, ES_city_water_level, sizeof(ES_city_water_level));
}

//
// Restores the editor state from the given undo structure.
//

void ES_undo_restore(ES_Undo *eu)
{
	memcpy(ES_hi,               eu->es_hi,               sizeof(ES_hi));
	memcpy(ES_lo,               eu->es_lo,               sizeof(ES_lo));
	memcpy(ES_thing,            eu->es_thing,            sizeof(ES_thing));
	memcpy(ES_city_water_on,    eu->es_city_water_on,    sizeof(ES_city_water_on));
	memcpy(ES_city_water_level, eu->es_city_water_level, sizeof(ES_city_water_level));
}

void ES_undo_store()
{
	ES_Undo *eu;

	ES_undo_stage = ES_undo_stage + 1;
	ES_undo_top   = ES_undo_stage;

	eu = &ES_undo[ES_undo_top & (ES_MAX_UNDO - 1)];

	ES_undo_create(eu);

	if (ES_undo_bot < ES_undo_top - 31)
	{
		ES_undo_bot = ES_undo_top - 31;
	}
}

void ES_undo_undo()
{
	ASSERT(WITHIN(ES_undo_stage, ES_undo_bot, ES_undo_top));

	if (ES_undo_stage == ES_undo_bot)
	{
		//
		// No undo info.
		//

		return;
	}

	ES_undo_stage -= 1;

	ES_undo_restore(&ES_undo[ES_undo_stage & (ES_MAX_UNDO - 1)]);
}

void ES_undo_redo()
{
	ASSERT(WITHIN(ES_undo_stage, ES_undo_bot, ES_undo_top));

	if (ES_undo_stage == ES_undo_top)
	{
		//
		// No redo info.
		//

		return;
	}

	ES_undo_stage += 1;

	ES_undo_restore(&ES_undo[ES_undo_stage & (ES_MAX_UNDO - 1)]);
}

SLONG ES_undo_undo_valid()
{
	return ES_undo_stage > ES_undo_bot;
}

SLONG ES_undo_redo_valid()
{
	return ES_undo_stage < ES_undo_top;
}



//
// The load/save scratchpad.
//

ES_Undo ES_loadsave;

//
// The header at the start of the files.
//

typedef struct
{
	SLONG version;
	SLONG sizeof_es_undo;

} ES_Header;


SLONG ES_save(CBYTE *filename)
{
	ES_Header ed;

	FILE *handle;

	handle = fopen(filename, "wb");

	if (handle == NULL)
	{
		return FALSE;
	}

	//
	// Build the info.
	//

	ed.version        = 2;
	ed.sizeof_es_undo = sizeof(ES_Undo);

	ES_undo_create(&ES_loadsave);

	//
	// Save out the info.
	//

	if (fwrite(&ed,          sizeof(ES_Header), 1, handle) != 1) goto file_error;
	if (fwrite(&ES_loadsave, sizeof(ES_Undo),   1, handle) != 1) goto file_error;

	fclose(handle);

	//
	// Lose all undo info except the initial state.
	//

	ES_undo_bot   =  0;
	ES_undo_top   =  0;
	ES_undo_stage = -1;

	ES_undo_store();

	return TRUE;

  file_error:;

	fclose(handle);

	return FALSE;
}

SLONG ES_load(CBYTE *filename)
{
	ES_Header ed;

	FILE *handle;

	handle = fopen(filename, "rb");

	if (handle == NULL)
	{
		return FALSE;
	}

	//
	// Load in the header.
	//

	if (fread(&ed, sizeof(ES_Header), 1, handle) != 1) goto file_error;

	//
	// Check compatability.
	//

	if (ed.version        != 2 ||
		ed.sizeof_es_undo != sizeof(ES_Undo))
	{
		//
		// Incompatible!
		//

		fclose(handle);

		return FALSE;
	}

	//
	// Load in the undo info.
	//

	if (fread(&ES_loadsave, sizeof(ES_Undo), 1, handle) != 1) goto file_error;

	fclose(handle);

	//
	// Restore the state.
	//

	ES_undo_restore(&ES_loadsave);

	//
	// Lose all undo info except the initial state.
	//

	ES_undo_bot   =  0;
	ES_undo_top   =  0;
	ES_undo_stage = -1;

	ES_undo_store();

	return TRUE;

  file_error:;

	fclose(handle);

	return FALSE;
}



void ES_draw_editor(
		SLONG  cam_x,
		SLONG  cam_y,
		SLONG  cam_z,
		SLONG  cam_yaw,
		SLONG  cam_pitch,
		SLONG  cam_roll,
		SLONG  mouse_x,
		SLONG  mouse_y,
		SLONG *mouse_over_valid,
		SLONG *mouse_over_x,
		SLONG *mouse_over_y,
		SLONG *mouse_over_z,
		SLONG  draw_prim_at_mouse,
		SLONG  prim_object,
		SLONG  prim_yaw)
{
	if (!WITHIN(mouse_x, 0, 640) ||
		!WITHIN(mouse_y, 0, 480))
	{
		mouse_x = INFINITY;
		mouse_y = INFINITY;
	}

	if (draw_prim_at_mouse)
	{
		TEXTURE_load_needed_object(prim_object);
	}

#ifdef SEWERS
	AENG_draw_sewer_editor(
		cam_x,
		cam_y,
		cam_z,
		cam_yaw,
		cam_pitch,
		cam_roll,
		mouse_x,
		mouse_y,
	    mouse_over_valid,
	    mouse_over_x,
	    mouse_over_y,
	    mouse_over_z,
		draw_prim_at_mouse,
		prim_object,
		prim_yaw);
#endif
}


void ES_light_move(SLONG x, SLONG z)
{
	SLONG lo_map_x = x >> PAP_SHIFT_LO;
	SLONG lo_map_z = z >> PAP_SHIFT_LO;

	if (WITHIN(lo_map_x, 0, PAP_SIZE_LO - 1) &&
		WITHIN(lo_map_z, 0, PAP_SIZE_LO - 1))
	{
		//
		// Move the light.
		//

		SLONG mx = x - (lo_map_x << PAP_SHIFT_LO) >> 3;
		SLONG mz = z - (lo_map_z << PAP_SHIFT_LO) >> 3;

		ES_lo[lo_map_x][lo_map_z].light_x = mx;
		ES_lo[lo_map_x][lo_map_z].light_z = mz;

		//
		// Make sure the light isn't underground.
		//

		SLONG height = ES_hi[x >> PAP_SHIFT_HI][z >> PAP_SHIFT_HI].height;

		if (ES_lo[lo_map_x][lo_map_z].light_y < height + 4)
		{
			ES_lo[lo_map_x][lo_map_z].light_y = height + 4;
		}
	}
}

void ES_light_dheight(SLONG x, SLONG z, SLONG dheight)
{
	SLONG lo_map_x = x >> PAP_SHIFT_LO;
	SLONG lo_map_z = z >> PAP_SHIFT_LO;

	if (WITHIN(lo_map_x, 0, PAP_SIZE_LO - 1) &&
		WITHIN(lo_map_z, 0, PAP_SIZE_LO - 1))
	{
		//
		// Raise/lower the light.
		//

		SLONG lheight;
		
		lheight  = ES_lo[lo_map_x][lo_map_z].light_y;
		lheight += dheight;

		SATURATE(lheight, 1, 255);

		ES_lo[lo_map_x][lo_map_z].light_y = lheight;

		//
		// Make sure the light isn't underground.
		//

		SLONG height = ES_hi[x >> PAP_SHIFT_HI][z >> PAP_SHIFT_HI].height;

		if (ES_lo[lo_map_x][lo_map_z].light_y < height + 4)
		{
			ES_lo[lo_map_x][lo_map_z].light_y = height + 4;
		}
	}
}

void ES_light_delete(SLONG x, SLONG z)
{
	SLONG lo_map_x = x >> PAP_SHIFT_LO;
	SLONG lo_map_z = z >> PAP_SHIFT_LO;

	if (WITHIN(lo_map_x, 0, PAP_SIZE_LO - 1) &&
		WITHIN(lo_map_z, 0, PAP_SIZE_LO - 1))
	{
		//
		// A height of zero => unused.
		//

		ES_lo[lo_map_x][lo_map_z].light_y = 0;
	}
}



// ========================================================
//
// LADDER FUNCTIONS
//
// ========================================================

void ES_ladder_create(
		SLONG ax,
		SLONG az,
		SLONG bx,
		SLONG bz)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG mx;
	SLONG mz;

	SLONG map_x = ax >> PAP_SHIFT_HI;
	SLONG map_z = az >> PAP_SHIFT_HI;

	ES_Thing *et;

	if (!WITHIN(map_x, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(map_z, 0, PAP_SIZE_HI - 1))
	{
		//
		// Ladder is off the map!
		//

		return;
	}

	//
	// Don't put two ladders on the same mapsquare- that way lies
	// madness!
	//

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_LADDER)
		{
			mx = (et->x1 + et->x2 << 7) + ((et->z2 - et->z1) << 2) >> 8;
			mz = (et->z1 + et->z2 << 7) - ((et->x2 - et->x1) << 2) >> 8;

			if (mx == map_x &&
				mz == map_z)
			{
				return;
			}
		}
	}

	//
	// Look for a spare thing structure.
	//

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_UNUSED)
		{
			//
			// Work out the coordinates of the ladder.
			//

			dx = bx - ax;
			dz = bz - az;

			if (abs(dx) > abs(dz))
			{
				if (dx > 0)
				{
					x1 = map_x + 1;
					z1 = map_z + 1;
					x2 = map_x + 1;
					z2 = map_z + 0;
				}
				else
				{
					x1 = map_x + 0;
					z1 = map_z + 0;
					x2 = map_x + 0;
					z2 = map_z + 1;
				}
			}
			else
			{
				if (dz > 0)
				{
					x1 = map_x + 0;
					z1 = map_z + 1;
					x2 = map_x + 1;
					z2 = map_z + 1;
				}
				else
				{
					x1 = map_x + 1;
					z1 = map_z + 0;
					x2 = map_x + 0;
					z2 = map_z + 0;
				}
			}

			et->type   = ES_THING_TYPE_LADDER;
			et->x1     = x1;
			et->z1     = z1;
			et->x2     = x2;
			et->z2     = z2;

			if (ES_hi[map_x][map_z].flag & ES_FLAG_ENTRANCE)
			{
				SLONG floor  = PAP_calc_height_at((ax << 8) + 0x80, (ax << 8) + 0x80);
				SLONG bottom = (ES_hi[map_x][map_z].height << 5) + -32 * 0x100;

				//
				// Reach to the ground.
				//

				et->height = ((floor - bottom) * 4) / 0x100;
			}
			else
			{
				et->height = 4;
			}

			return;
		}
	}
}

void ES_ladder_dheight(
		SLONG x,
		SLONG z,
		SLONG dheight)
{
	SLONG i;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG best_thing = 0;
	SLONG best_dist  = INFINITY;

	ES_Thing *et;

	x >>= PAP_SHIFT_HI;
	z >>= PAP_SHIFT_HI;

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_LADDER)
		{
			dx = et->x1 - x + et->x2 - x;
			dz = et->z1 - z + et->z2 - z;

			dist = abs(dx) + abs(dz);

			if (dist < best_dist)
			{	
				best_thing = i;
				best_dist  = dist;
			}
		}
	}

	if (best_dist < 5)
	{
		ASSERT(WITHIN(best_thing, 0, ES_MAX_THINGS - 1));

		et = &ES_thing[best_thing];

		//
		// Work out the new height.
		//

		SLONG height;
		
		height  = et->height;
		height += dheight;

		SATURATE(height, 4, 128);

		et->height = height;
	}

	return;
}

void ES_ladder_delete(
		SLONG x,
		SLONG z)
{
	SLONG i;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG best_thing = 0;
	SLONG best_dist  = INFINITY;

	x >>= PAP_SHIFT_HI;
	z >>= PAP_SHIFT_HI;

	ES_Thing *et;

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_LADDER)
		{
			dx = et->x1 - x + et->x2 - x;
			dz = et->z1 - z + et->z2 - z;

			dist = abs(dx) + abs(dz);

			if (dist < best_dist)
			{	
				best_thing = i;
				best_dist  = dist;
			}
		}
	}

	if (best_dist < 5)
	{
		ASSERT(WITHIN(best_thing, 0, ES_MAX_THINGS - 1));

		et = &ES_thing[best_thing];

		//
		// Delete this ladder.
		//

		et->type = ES_THING_TYPE_UNUSED;
	}

	return;
}

// ========================================================
//
// WATER FUNCTIONS
//
// ========================================================

void ES_sewer_water_dheight(SLONG x, SLONG z, SLONG dheight)
{
	SLONG i;

	SLONG count;
	SLONG match;
	SLONG height;

	SLONG dx;
	SLONG dz;

	SLONG nx;
	SLONG nz;

	ES_Hi *eh;

	SLONG map_x = x >> PAP_SHIFT_HI;
	SLONG map_z = z >> PAP_SHIFT_HI;

	ASSERT(WITHIN(map_x, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(map_z, 0, PAP_SIZE_HI - 1));

	eh = &ES_hi[map_x][map_z];

	if (eh->water == 0)
	{
		//
		// No water to change the height of!
		//

		return;
	}
	else
	{
		match   = eh->water;
		height  = eh->water;
		height += dheight;
		
		SATURATE(height, 9, 255);
	}

	typedef struct
	{
		UBYTE x;
		UBYTE z;
		
	} Queue;

	#define QUEUE_SIZE 512

	Queue queue[QUEUE_SIZE];

	SLONG queue_start = 0;	// Access MOD QUEUE_SIZE
	SLONG queue_end   = 0;

	Queue *qs;
	Queue *qe;

	queue[0].x = map_x;
	queue[0].z = map_z;

	queue_start = 0;
	queue_end   = 1;

	count = 0;

	while(queue_start < queue_end)
	{
		count += 1;

		if (count > 500)
		{
			//
			// There is the possibility of a infinite loop here.
			//

			return;
		}

		qs = &queue[queue_start & (QUEUE_SIZE - 1)];

		ASSERT(WITHIN(qs->x, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(qs->z, 0, PAP_SIZE_HI - 1));
		
		eh = &ES_hi[qs->x][qs->z];

		eh->water = height;

		if (eh->water <= eh->height)
		{
			eh->water  = eh->height + 1;
		}

		//
		// Look for neigbouring sewers.
		//

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
				default:
					ASSERT(0);
					break;
			}

			nx = qs->x + dx;
			nz = qs->z + dz;

			if (WITHIN(nx, 0, PAP_SIZE_HI - 1) &&
				WITHIN(nz, 0, PAP_SIZE_HI - 1))
			{
				if (ES_hi[nx][nz].water == match)
				{
					qe = &queue[queue_end & (QUEUE_SIZE - 1)];

					qe->x = nx;
					qe->z = nz;

					queue_end += 1;
				}
			}
		}

		queue_start += 1;
	}
}


//
// Set/get the city water 'on' status of the city water at (x,z)
//

SLONG ES_city_water_get(SLONG x, SLONG z)
{
	SLONG map_x = x >> PAP_SHIFT_HI;
	SLONG map_z = z >> PAP_SHIFT_HI;

	ASSERT(WITHIN(map_x, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(map_z, 0, PAP_SIZE_HI - 1));

	if (ES_city_water_on[map_x][map_z])
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void  ES_city_water_set(SLONG x, SLONG z, SLONG on_or_not)
{
	SLONG ground;
	SLONG wlevel;

	SLONG map_x = x >> PAP_SHIFT_HI;
	SLONG map_z = z >> PAP_SHIFT_HI;

	ASSERT(WITHIN(map_x, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(map_z, 0, PAP_SIZE_HI - 1));

	if (on_or_not)
	{
		if (!ES_city_water_on[map_x][map_z])
		{
			ES_city_water_on[map_x][map_z] = TRUE;

			//
			// Make sure the water level here is not underground.
			//

			SLONG lo_map_x = x >> PAP_SHIFT_LO;
			SLONG lo_map_z = z >> PAP_SHIFT_LO;

			ground = PAP_calc_height_at(x,z);
			wlevel = ES_city_water_level[lo_map_x][lo_map_z] << PAP_ALT_SHIFT;

			if (wlevel <= ground)
			{
				wlevel   = ((ground >> PAP_ALT_SHIFT) << PAP_ALT_SHIFT) + (1 << PAP_ALT_SHIFT);
				wlevel >>= PAP_ALT_SHIFT;

				ES_city_water_level[lo_map_x][lo_map_z] = wlevel;
			}

			//
			// Put the the water above the city.
			// 

			ES_city_water_build();

		}
	}
	else
	{
		if (ES_city_water_on[map_x][map_z])
		{
			ES_city_water_on[map_x][map_z] = FALSE;

			//
			// Put the the water above the city.
			// 

			ES_city_water_build();
		}
	}
}


void ES_city_water_dlevel(SLONG x, SLONG z, SLONG dlevel)
{
	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG ground;
	SLONG maxg;
	SLONG level;

	SLONG lo_map_x = x >> PAP_SHIFT_LO;
	SLONG lo_map_z = z >> PAP_SHIFT_LO;

	ASSERT(WITHIN(lo_map_x, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(lo_map_z, 0, PAP_SIZE_LO - 1));

	//
	// The water level we want.
	//

	level  = ES_city_water_level[lo_map_x][lo_map_z] << PAP_ALT_SHIFT;
	level += dlevel;

	//
	// The height ground point under water in this lo-res mapsquare.
	//

	maxg = -INFINITY;

	for (dx = 0; dx < 4; dx++)
	for (dz = 0; dz < 4; dz++)
	{
		mx = (lo_map_x << 2) + dx;
		mz = (lo_map_z << 2) + dz;

		if (ES_city_water_on[mx][mz])
		{
			ground = PAP_calc_height_at((mx << 8) + 0x80, (mz << 8) + 0x80);

			if (maxg < ground)
			{
				maxg = ground;
			}
		}
	}

	if (maxg == -INFINITY)
	{
		//
		// No water here to raise or lower!
		//

		return;
	}

	//
	// Round off to the nearest representable value.
	//

	maxg >>= PAP_ALT_SHIFT;
	maxg <<= PAP_ALT_SHIFT;

	//
	// Make sure no water goes underground.
	//

	if (level < maxg)
	{
		level = maxg + (1 << PAP_ALT_SHIFT);
	}
	
	//
	// Set the new water level FOR THE WHOLE CITY FOR NOW!
	//

	for (lo_map_x = 0; lo_map_x < PAP_SIZE_LO; lo_map_x++)
	for (lo_map_z = 0; lo_map_z < PAP_SIZE_LO; lo_map_z++)
	{
		ES_city_water_level[lo_map_x][lo_map_z] = level >> PAP_ALT_SHIFT;
	}

	//
	// Put the the water above the city.
	// 

	ES_city_water_build();
}



// ========================================================
//
// PRIM FUNCTIONS
//
// ========================================================

void ES_prim_create(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw)
{
	SLONG i;

	SLONG map_x = x >> PAP_SHIFT_HI;
	SLONG map_z = z >> PAP_SHIFT_HI;

	ES_Thing *et;

	if (!WITHIN(map_x, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(map_z, 0, PAP_SIZE_HI - 1))
	{
		//
		// Prim is off the map!
		//

		return;
	}

	//
	// Look for a spare thing structure.
	//

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_UNUSED)
		{
			//
			// Create the prim.
			//

			et->prim = prim;
			et->x    = x;
			et->y    = y;
			et->z    = z;
			et->yaw  = yaw;

			return;
		}
	}
}

void ES_prim_delete(
		SLONG x,
		SLONG y,
		SLONG z)
{
	SLONG i;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG best_thing = 0;
	SLONG best_dist  = INFINITY;

	x >>= PAP_SHIFT_HI;
	z >>= PAP_SHIFT_HI;

	ES_Thing *et;

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_PRIM)
		{
			dx = et->x - x;
			dz = et->z - z;

			dist = abs(dx) + abs(dz);

			if (dist < best_dist)
			{	
				best_thing = i;
				best_dist  = dist;
			}
		}
	}

	if (best_dist < 600)
	{
		ASSERT(WITHIN(best_thing, 0, ES_MAX_THINGS - 1));

		et = &ES_thing[best_thing];

		//
		// Delete this prim.
		//

		et->type = ES_THING_TYPE_UNUSED;
	}

	return;
}


void ES_prim_dheight(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dheight)
{
	SLONG i;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG best_thing = 0;
	SLONG best_dist  = INFINITY;

	x >>= PAP_SHIFT_HI;
	z >>= PAP_SHIFT_HI;

	ES_Thing *et;

	for (i = 0; i < ES_MAX_THINGS; i++)
	{
		et = &ES_thing[i];

		if (et->type == ES_THING_TYPE_PRIM)
		{
			dx = et->x - x;
			dz = et->z - z;

			dist = abs(dx) + abs(dz);

			if (dist < best_dist)
			{	
				best_thing = i;
				best_dist  = dist;
			}
		}
	}

	if (best_dist < 600)
	{
		ASSERT(WITHIN(best_thing, 0, ES_MAX_THINGS - 1));

		et = &ES_thing[best_thing];

		//
		// Change the height of this prim.
		//

		et->y += dheight;
	}

	return;
}

