//
// The light editor.
//

#include <MFStdLib.h>
#include "ed.h"
#include "night.h"


//
// Our lights.
// 

ED_Light ED_light[ED_MAX_LIGHTS];
SLONG    ED_light_free;




//
// Encapsulates all editor data.
//

typedef struct
{
	ED_Light     ed_light[ED_MAX_LIGHTS];
	SLONG        ed_light_free;
	ULONG        night_flag;
	ULONG        night_amb_d3d_colour;
	ULONG        night_amb_d3d_specular;
	SLONG        night_amb_red;
	SLONG        night_amb_green;
	SLONG        night_amb_blue;
	SBYTE        night_lampost_red;
	SBYTE        night_lampost_green;
	SBYTE        night_lampost_blue;
	UBYTE        padding;
	SLONG        night_lampost_radius;
	NIGHT_Colour night_sky_colour;

} ED_Undo;

#define ED_MAX_UNDO 64

ED_Undo ED_undo[ED_MAX_UNDO];
SLONG   ED_undo_top;	// A circular system: Access these values MOD ED_MAX_UNDO
SLONG   ED_undo_bot;
SLONG   ED_undo_stage;



void ED_init()
{
	SLONG i;

	//
	// Clear all lights.
	//

	ED_light_free = 1;

	for (i = 1; i < ED_MAX_LIGHTS - 1; i++)
	{
		ED_light[i].next = i + 1;
		ED_light[i].used = FALSE;
	}

	ED_light[ED_MAX_LIGHTS - 1].next = NULL;

	//
	// Initialise game lighting too.
	//

	NIGHT_init();

	//
	// Lose all undo info except the initial state.
	//

	ED_undo_bot   =  0;
	ED_undo_top   =  0;
	ED_undo_stage = -1;

	ED_undo_store();
}


SLONG ED_create(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG range,
		SLONG red,
		SLONG green,
		SLONG blue)
{
	SLONG ans;

	ED_Light *el;

	if (ED_light_free == NULL)
	{
		return NULL;
	}

	//
	// Make sure the values are in the correct range.
	//

	SATURATE(range,    1, 255);
	SATURATE(red,   -127, 127);
	SATURATE(green, -127, 127);
	SATURATE(blue,  -127, 127);
	
	//
	// Create the new edit-light structure.
	//

	ASSERT(WITHIN(ED_light_free, 1, ED_MAX_LIGHTS - 1));

	ans           =  ED_light_free;
	el            = &ED_light[ED_light_free];
	ED_light_free =  el->next;

	el->used  = TRUE;
	el->x     = x;
	el->y     = y;
	el->z     = z;
	el->range = range;
	el->red   = red;
	el->green = green;
	el->blue  = blue;

	//
	// Add the light to the map.
	//

	NIGHT_slight_create(
		x, y, z,
		range,
		red,
		green,
		blue);

	//
	// Make sure we can see it.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
	
	return ans;
}


void ED_light_move(
		SLONG light,
		SLONG x,
		SLONG y,
		SLONG z)
{
	ED_Light *el;

	ASSERT(WITHIN(light, 1, ED_MAX_LIGHTS - 1));

	el = &ED_light[light];

	ASSERT(el->used);

	//
	// Delete the light from the game map.
	//

	NIGHT_slight_delete(
		el->x,
		el->y,
		el->z,
		el->range,
		el->red,
		el->green,
		el->blue);

	//
	// Move it and re-insert it.
	//

	el->x = x;
	el->y = y;
	el->z = z;

	NIGHT_slight_create(
		el->x,
		el->y,
		el->z,
		el->range,
		el->red,
		el->green,
		el->blue);

	//
	// Make sure we can see the change.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}


void ED_light_change(
		SLONG light,
		SLONG range,
		SLONG red,
		SLONG green,
		SLONG blue)
{
	ED_Light *el;

	ASSERT(WITHIN(light, 1, ED_MAX_LIGHTS - 1));

	el = &ED_light[light];

	ASSERT(el->used);

	//
	// Delete the light from the game map.
	//

	NIGHT_slight_delete(
		el->x,
		el->y,
		el->z,
		el->range,
		el->red,
		el->green,
		el->blue);

	//
	// Change it and re-insert it.
	//

	el->range = range;
	el->red   = red;
	el->green = green;
	el->blue  = blue;

	NIGHT_slight_create(
		el->x,
		el->y,
		el->z,
		el->range,
		el->red,
		el->green,
		el->blue);

	//
	// Make sure we can see the change.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}




void ED_amb_get(
		SLONG *red,
		SLONG *green,
		SLONG *blue)
{
	//
	// This is insider information!
	//

	*red   = NIGHT_amb_red   * 820 >> 8;
	*green = NIGHT_amb_green * 820 >> 8;
	*blue  = NIGHT_amb_blue  * 820 >> 8;
}

void ED_amb_set(
		SLONG red,
		SLONG green,
		SLONG blue)
{
	NIGHT_ambient(
		red,
		green,
		blue,
		110, -148, -177); // These number are taken from game.cpp

	//
	// Make sure we can see it.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}


SLONG ED_lampost_on_get()
{
	return NIGHT_flag & NIGHT_FLAG_LIGHTS_UNDER_LAMPOSTS;
}

void ED_lampost_on_set(SLONG b)
{
	if (b)
	{
		NIGHT_flag |= NIGHT_FLAG_LIGHTS_UNDER_LAMPOSTS;
	}
	else
	{
		NIGHT_flag &= ~NIGHT_FLAG_LIGHTS_UNDER_LAMPOSTS;
	}

	//
	// Recalculate everything.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}

SLONG ED_night_get(void)
{
	return !(NIGHT_flag & NIGHT_FLAG_DAYTIME);
}

void ED_night_set(SLONG b)
{
	if (!b)
	{
		NIGHT_flag |=  NIGHT_FLAG_DAYTIME;
	}
	else
	{
		NIGHT_flag &= ~NIGHT_FLAG_DAYTIME;
	}
}

SLONG ED_darken_bottoms_on_get()
{
	return (NIGHT_flag & NIGHT_FLAG_DARKEN_BUILDING_POINTS);
	
}

void ED_darken_bottoms_on_set(SLONG b)
{
	if (b)
	{
		NIGHT_flag |=  NIGHT_FLAG_DARKEN_BUILDING_POINTS;
	}
	else
	{
		NIGHT_flag &= ~NIGHT_FLAG_DARKEN_BUILDING_POINTS;
	}

	//
	// Recalc game lighting.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}


void ED_lampost_get(
		SLONG *range,
		SLONG *red,
		SLONG *green,
		SLONG *blue)
{
	*range = NIGHT_lampost_radius;
	*red   = NIGHT_lampost_red;
	*green = NIGHT_lampost_green;
	*blue  = NIGHT_lampost_blue;
}

void ED_lampost_set(
		SLONG range,
		SLONG red,
		SLONG green,
		SLONG blue)
{
	NIGHT_lampost_radius = range;
	NIGHT_lampost_red    = red;
	NIGHT_lampost_green  = green;
	NIGHT_lampost_blue   = blue;

	//
	// Recalc game lighting.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}

//
// Get/Set the sky light colour.  Values go from 0 - 255
//

void ED_sky_get(
		SLONG *red,
		SLONG *green,
		SLONG *blue)
{
	*red   = NIGHT_sky_colour.red;
	*green = NIGHT_sky_colour.green;
	*blue  = NIGHT_sky_colour.blue;
}

void ED_sky_set(
		SLONG red,
		SLONG green,
		SLONG blue)
{
	NIGHT_sky_colour.red   = red;
	NIGHT_sky_colour.green = green;
	NIGHT_sky_colour.blue  = blue;
}




//
// Fills out the given undo structure with the current state.
//

void ED_undo_create(ED_Undo *eu)
{
	memcpy(eu->ed_light, ED_light, sizeof(ED_light));

	eu->ed_light_free		   = ED_light_free;
	eu->night_flag			   = NIGHT_flag;
	eu->night_amb_d3d_colour   = NIGHT_amb_d3d_colour;
	eu->night_amb_d3d_specular = NIGHT_amb_d3d_specular;
	eu->night_amb_red		   = NIGHT_amb_red;
	eu->night_amb_green		   = NIGHT_amb_green;
	eu->night_amb_blue		   = NIGHT_amb_blue;
	eu->night_lampost_red      = NIGHT_lampost_red;
	eu->night_lampost_green    = NIGHT_lampost_green;
	eu->night_lampost_blue     = NIGHT_lampost_blue;
	eu->night_lampost_radius   = NIGHT_lampost_radius;
	eu->night_sky_colour	   = NIGHT_sky_colour;
}

//
// Restores the lighting state from the given undo structure.
//

void ED_undo_restore(ED_Undo *eu)
{
	SLONG i;

	memcpy(ED_light, eu->ed_light, sizeof(ED_light));

	ED_light_free		   = eu->ed_light_free;
	NIGHT_flag			   = eu->night_flag;
	NIGHT_amb_d3d_colour   = eu->night_amb_d3d_colour;
	NIGHT_amb_d3d_specular = eu->night_amb_d3d_specular;
	NIGHT_amb_red		   = eu->night_amb_red;
	NIGHT_amb_green		   = eu->night_amb_green;
	NIGHT_amb_blue		   = eu->night_amb_blue;
	NIGHT_lampost_red      = eu->night_lampost_red;
	NIGHT_lampost_green    = eu->night_lampost_green;
	NIGHT_lampost_blue     = eu->night_lampost_blue;
	NIGHT_lampost_radius   = eu->night_lampost_radius;
	NIGHT_sky_colour	   = eu->night_sky_colour;

	//
	// Re-add all the static lights.
	//

	NIGHT_slight_delete_all();

	for (i = 0; i < ED_MAX_LIGHTS; i++)
	{
		if (ED_light[i].used)
		{
			NIGHT_slight_create(
				ED_light[i].x,    
				ED_light[i].y,    
				ED_light[i].z,    
				ED_light[i].range,
				ED_light[i].red,  
				ED_light[i].green,
				ED_light[i].blue);
		}
	}

	//
	// Recalculate everything.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}


void ED_undo_store()
{
	ED_Undo *eu;

	ED_undo_stage = ED_undo_stage + 1;
	ED_undo_top   = ED_undo_stage;

	eu = &ED_undo[ED_undo_top & (ED_MAX_UNDO - 1)];

	ED_undo_create(eu);

	if (ED_undo_bot < ED_undo_top - 31)
	{
		ED_undo_bot = ED_undo_top - 31;
	}
}

void ED_undo_undo()
{
	ASSERT(WITHIN(ED_undo_stage, ED_undo_bot, ED_undo_top));

	if (ED_undo_stage == ED_undo_bot)
	{
		//
		// No undo info.
		//

		return;
	}

	ED_undo_stage -= 1;

	ED_undo_restore(&ED_undo[ED_undo_stage & (ED_MAX_UNDO - 1)]);
}

void ED_undo_redo()
{
	ASSERT(WITHIN(ED_undo_stage, ED_undo_bot, ED_undo_top));

	if (ED_undo_stage == ED_undo_top)
	{
		//
		// No redo info.
		//

		return;
	}

	ED_undo_stage += 1;

	ED_undo_restore(&ED_undo[ED_undo_stage & (ED_MAX_UNDO - 1)]);
}

SLONG ED_undo_undo_valid()
{
	return ED_undo_stage > ED_undo_bot;
}

SLONG ED_undo_redo_valid()
{
	return ED_undo_stage < ED_undo_top;
}

//
// The load/save scratchpad.
//

ED_Undo ED_loadsave;

typedef struct
{
	SLONG sizeof_ed_light;
	SLONG ed_max_lights;
	SLONG sizeof_night_colour;

} ED_Header;

SLONG ED_load(CBYTE *name)
{
	SLONG i,data_left;
	UBYTE version = 0;

	ED_Header eh;

//	FILE *handle;
	MFFileHandle handle;

//	handle = fopen(name, "rb");
	handle = FileOpen(name);

	if (handle)
	{
		//
		// Read in the header.
		//

//		if (fread(&eh, sizeof(eh), 1, handle) != 1) goto file_error;
		if (FileRead(handle,&eh,sizeof(eh))<0) goto file_error;

		// math time
/*		data_left=FileSize(handle)-12;
		if (data_left!=(eh.sizeof_ed_light*eh.ed_max_lights)+40) {
			FileRead(handle,&version,1);
		}*/

		version=eh.sizeof_ed_light>>16;
		eh.sizeof_ed_light&=0xFFFF;

		//
		// Check compatability.
		//

		if (eh.sizeof_ed_light     != sizeof(ED_Light) ||
			eh.sizeof_night_colour != sizeof(NIGHT_Colour))
		{
			goto file_error;
		}

		if (eh.ed_max_lights != ED_MAX_LIGHTS)
		{
			if (eh.ed_max_lights > ED_MAX_LIGHTS)
			{
				//
				// Too many lights!
				//

				goto file_error;
			}

			//
			// Load in the lights we have.
			//

			for (i = 0; i < eh.ed_max_lights; i++)
			{
//				if (fread(&ED_loadsave.ed_light[i], sizeof(ED_Light), 1, handle) != 1) goto file_error;
				if (FileRead(handle,&ED_loadsave.ed_light[i], sizeof(ED_Light)) < 0) goto file_error;
			}

			//
			// Load in the rest of the structure.
			//

//			if (fread(&ED_loadsave.ed_light_free, sizeof(ED_loadsave) - sizeof(ED_loadsave.ed_light), 1, handle) != 1) goto file_error;
			if (FileRead(handle,&ED_loadsave.ed_light_free, sizeof(ED_loadsave) - sizeof(ED_loadsave.ed_light)) < 0) goto file_error;

			//
			// Add the remaining lights to the free list.
			//

			for (i = eh.ed_max_lights; i < ED_MAX_LIGHTS; i++)
			{
				memset(&ED_loadsave.ed_light[i], 0, sizeof(ED_Light));

				ED_loadsave.ed_light[i].next = ED_loadsave.ed_light_free;
				ED_loadsave.ed_light_free    = i;
			}
		}
		else
		{
//			if (fread(&ED_loadsave, sizeof(ED_loadsave), 1, handle) != 1) goto file_error;
			if (FileRead(handle, &ED_loadsave, sizeof(ED_loadsave)) < 0) goto file_error;
		}

//		fclose(handle);
		FileClose(handle);

		//
		// Restore state from the save structure.
		//

		ED_undo_restore(&ED_loadsave);

		//
		// Lose all undo info except the initial state.
		//

		ED_undo_bot   =  0;
		ED_undo_top   =  0;
		ED_undo_stage = -1;

		ED_undo_store();

		return TRUE;
	
	  file_error:;

//		fclose(handle);
		FileClose(handle);

		return FALSE;

	}

	return FALSE;
}

SLONG ED_save(CBYTE *name)
{
	SLONG version = 1;
	SLONG c0;

	ED_Header eh;

	FILE *handle;

	handle = fopen(name, "wb");

	if (handle)
	{
		//
		// Fill out our undo structure.
		//

		ED_undo_create(&ED_loadsave);

		for (c0=0;c0<ED_MAX_LIGHTS;c0++)
		  ED_loadsave.ed_light[c0].blue&=~1;

		//
		// Fill out the header.
		//

		eh.sizeof_ed_light     = sizeof(ED_Light) | (version<<16);
		eh.ed_max_lights       = ED_MAX_LIGHTS;
		eh.sizeof_night_colour = sizeof(NIGHT_Colour);

		//
		// Save everything out.
		//

		if (fwrite(&eh,          sizeof(eh),          1, handle) != 1) goto file_error;


		// sticking in an extra bit of version info here...
//		if (fwrite(&version, sizeof(version), 1, handle) !=1) goto file_error;


		if (fwrite(&ED_loadsave, sizeof(ED_loadsave), 1, handle) != 1) goto file_error;

		fclose(handle);

		//
		// Lose all undo info except the initial state.
		//

		ED_undo_bot   =  0;
		ED_undo_top   =  0;
		ED_undo_stage = -1;

		ED_undo_store();

		return TRUE;

	  file_error:;

		fclose(handle);

		return FALSE;
	}

	return FALSE;
}


void ED_delete(SLONG light)
{
	ASSERT(WITHIN(light, 0, ED_MAX_LIGHTS - 1));

	ED_Light *el = &ED_light[light];

	ASSERT(el->used);

	//
	// Delete the light from the game map.
	//

	NIGHT_slight_delete(
		el->x,
		el->y,
		el->z,
		el->range,
		el->red,
		el->green,
		el->blue);

	//
	// Add to the free list.
	//

	el->next      = ED_light_free;
	ED_light_free = light;

	//
	// Mark as unused.
	//

	el->used = FALSE;

	//
	// Recalc game lighting.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();
}

void ED_delete_all(void)
{
	SLONG i;

	//
	// Clear all lights.
	//

	ED_light_free = 1;

	for (i = 1; i < ED_MAX_LIGHTS - 1; i++)
	{
		ED_light[i].next = i + 1;
		ED_light[i].used = FALSE;
	}

	ED_light[ED_MAX_LIGHTS - 1].next = NULL;

	//
	// Remove all static lights.
	// 

	NIGHT_slight_delete_all();

	//
	// Recalc game lighting.
	//

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();

}

