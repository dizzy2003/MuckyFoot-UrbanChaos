//
// A cache designed for storing light info.
//

#ifndef CACHE_H
#define CACHE_H


typedef UBYTE CACHE_Index;


//
// Initialises all the cache elements.
//

void CACHE_init(void);

//
// Creates a cache entry for the given thing. It saves all the
// light info.  If it returns NULL, then it couldn't cache the
// info.
//

CACHE_Index CACHE_create(
				SLONG key,
				void *data,
				UWORD num_bytes);

//
// Returns TRUE if the given CACHE_Index contains valid info.
//

SLONG CACHE_is_valid(CACHE_Index c_index);

//
// Returns the data associated with the cache_index.
//

typedef struct
{
	SLONG key;
	void *data;
	SLONG num_bytes;

} CACHE_Info;

CACHE_Info CACHE_get_info(CACHE_Index c_index);


//
// Invalidates the given cache entry.
//

void CACHE_invalidate(CACHE_Index c_index);
void CACHE_invalidate_all(void);


//
// You can flag cache entries. All cache entries are created
// already with their flag set.
//

void CACHE_flag_clear_all(void);
void CACHE_flag_set      (CACHE_Index c_index);
void CACHE_flag_clear    (CACHE_Index c_index);

//
// Invalidates all cache entries that don't have their flag set.
//

void CACHE_invalidate_unflagged(void);



#endif
