//
// A cache designed for storing light info.
//

#include "game.h"
#include <MFStdLib.h>

#include "cache.h"
#include "heap.h"
#include "c:/fallen/ddengine/headers/message.h"




//
// All the cache entries.
//

#define CACHE_FLAG_VALID	(1 << 0)
#define CACHE_FLAG_FLAGGED	(1 << 1)

typedef struct
{
	void       *data;
	SLONG       key;
	UWORD       num_bytes;
	UBYTE       flag;
	CACHE_Index next;
	
} CACHE_Entry;

#define CACHE_MAX_ENTRIES 128

CACHE_Entry  CACHE_entry[CACHE_MAX_ENTRIES];
CACHE_Index  CACHE_free;




void CACHE_init()
{
	SLONG i;

	//
	// Build the free list.
	//

	CACHE_free = 1;

	for (i = 1; i < CACHE_MAX_ENTRIES - 1; i++)
	{
		CACHE_entry[i].flag = 0;
		CACHE_entry[i].next = i + 1;
	}

	CACHE_entry[CACHE_MAX_ENTRIES - 1].flag = 0;
	CACHE_entry[CACHE_MAX_ENTRIES - 1].next = 0;
}


CACHE_Index CACHE_create(
				SLONG key,
				void *data,
				UWORD num_bytes)
{
	void *heap;

	CACHE_Index  c_index;
	CACHE_Entry *ce;

	if (CACHE_free == NULL)
	{
		//
		// Oh dear! Pick a random cache entry and overwrite that!
		//

		c_index = Random() & (CACHE_MAX_ENTRIES - 1);
	}
	else
	{
		ASSERT(WITHIN(CACHE_free, 1, CACHE_MAX_ENTRIES - 1));

		//
		// Take a cache element of the top of the linked list.
		//

		c_index    = CACHE_free;
		CACHE_free = CACHE_entry[CACHE_free].next;
	}

	ce = &CACHE_entry[c_index];

	//
	// Get some memory from the heap.
	//

	heap = HEAP_get(num_bytes);

	if (heap == NULL)
	{
		//
		// We can't cache the light info.
		//

//		MSG_add("No more heap!");

		return NULL;
	}
	else
	{
		ce->flag      = CACHE_FLAG_VALID | CACHE_FLAG_FLAGGED;
		ce->key       = key;
		ce->data      = heap;
		ce->num_bytes = num_bytes;
		ce->next      = NULL;

		//
		// Copy the data into the heap.
		//

		memcpy(heap, data, num_bytes);

		return c_index;
	}
}



SLONG CACHE_is_valid(CACHE_Index c_index)
{
	if (WITHIN(c_index, 1, CACHE_MAX_ENTRIES - 1))
	{
		if (CACHE_entry[c_index].flag & CACHE_FLAG_VALID)
		{
			return TRUE;
		}
	}

	return FALSE;
}


CACHE_Info CACHE_get_info(CACHE_Index c_index)
{
	CACHE_Info ans;

	ASSERT(WITHIN(c_index, 1, CACHE_MAX_ENTRIES - 1));

	ans.key       = CACHE_entry[c_index].key;
	ans.data      = CACHE_entry[c_index].data;
	ans.num_bytes = CACHE_entry[c_index].num_bytes;

	return ans;
}


void CACHE_invalidate(CACHE_Index c_index)
{
	CACHE_Entry *ce;

	ASSERT(WITHIN(c_index, 1, CACHE_MAX_ENTRIES - 1));

	ce = &CACHE_entry[c_index];

	if (ce->flag & CACHE_FLAG_VALID)
	{
		//
		// Invalidate it.
		//

		ce->flag &= ~CACHE_FLAG_VALID;

		//
		// Return the heap memory.
		//

		HEAP_give(ce->data, ce->num_bytes);

		//
		// Add it to the free list.
		//

		ce->next   = CACHE_free;
		CACHE_free = c_index;
	}
}

void CACHE_invalidate_all()
{
	SLONG i;

	CACHE_Entry *ce;

	for (i = 1; i < CACHE_MAX_ENTRIES; i++)
	{
		ce = &CACHE_entry[i];

		if (ce->flag & CACHE_FLAG_VALID)
		{
			//
			// Invalidate it.
			//

			ce->flag &= ~CACHE_FLAG_VALID;

			//
			// Return the heap memory.
			//

			HEAP_give(ce->data, ce->num_bytes);

			//
			// Add it to the free list.
			//

			ce->next   = CACHE_free;
			CACHE_free = i;
		}
	}
}


void CACHE_flag_clear_all()
{
	SLONG i;

	for (i = 1; i < CACHE_MAX_ENTRIES; i++)
	{
		CACHE_entry[i].flag &= ~CACHE_FLAG_FLAGGED;
	}
}

void CACHE_flag_set(CACHE_Index c_index)
{
	ASSERT(WITHIN(c_index, 1, CACHE_MAX_ENTRIES - 1));

	CACHE_entry[c_index].flag |= CACHE_FLAG_FLAGGED;
}

void CACHE_flag_clear(CACHE_Index c_index)
{
	ASSERT(WITHIN(c_index, 1, CACHE_MAX_ENTRIES - 1));

	CACHE_entry[c_index].flag &= ~CACHE_FLAG_FLAGGED;
}

void CACHE_invalidate_unflagged()
{
	SLONG i;

	for (i = 1; i < CACHE_MAX_ENTRIES; i++)
	{
		if (!(CACHE_entry[i].flag & CACHE_FLAG_FLAGGED) && (CACHE_entry[i].flag & CACHE_FLAG_VALID))
		{
			//
			// Invalidate this cache entry.
			// 

//			MSG_add("Invalidating an unflagged cache element.");

			CACHE_invalidate(i);
		}
	}
}
