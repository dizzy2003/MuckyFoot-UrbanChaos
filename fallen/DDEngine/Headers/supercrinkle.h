//
// SUPERCRINKLES!
//

#ifndef _SUPERCRINKLE_
#define _SUPERCRINKLE_


// Set to 0 to remove all crinkle stuff.
#define SUPERCRINKLES_ENABLED 0




#if SUPERCRINKLES_ENABLED

//
// Loads all the crinkles for the current level and creates
// their lighting textures.
//

void SUPERCRINKLE_init(void);


//
// Does this page have a crinkle?
//

extern UBYTE SUPERCRINKLE_is_crinkled[512];

#define SUPERCRINKLE_IS_CRINKLED(num) SUPERCRINKLE_is_crinkled[num]


//
// Draws the given crinkle. It assumes the POLY_local_rotation
// has been setup properly already.  Returns TRUE if it drew
// a SUPERCRINKLE.
//

SLONG SUPERCRINKLE_draw(SLONG page, ULONG colour[4], ULONG specular[4]);


#else //#if SUPERCRINKLES_ENABLED


// Dummy routs.
inline void SUPERCRINKLE_init(void) {}

#define SUPERCRINKLE_IS_CRINKLED(num) FALSE

inline SLONG SUPERCRINKLE_draw(SLONG page, ULONG colour[4], ULONG specular[4]){ return FALSE; }


#endif //#else //#if SUPERCRINKLES_ENABLED


#endif
