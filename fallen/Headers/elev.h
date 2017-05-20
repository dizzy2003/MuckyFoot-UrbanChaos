//
// The new ECTS level stuff!
// 

#ifndef _ELEV_
#define _ELEV_



#ifndef PSX

//
// This are the last ones to be loaded...
// 

extern CBYTE ELEV_last_map_loaded[MAX_PATH];

extern CBYTE ELEV_fname_map     [_MAX_PATH];
extern CBYTE ELEV_fname_lighting[_MAX_PATH];
extern CBYTE ELEV_fname_citsez  [_MAX_PATH];
extern CBYTE ELEV_fname_level   [_MAX_PATH];


#endif



//
// Prompts for a level and loads it. Returns FALSE on failure.
// Loads the given level in. Returns FALSE on failure.
//

SLONG ELEV_load_user(SLONG game);
SLONG ELEV_load_name(CBYTE *fname);
SLONG ELEV_game_init(
		CBYTE *fname_map,
		CBYTE *fname_lighting,
		CBYTE *fname_sewers,
		CBYTE *fname_level);



#endif
