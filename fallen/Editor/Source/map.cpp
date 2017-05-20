#include	"Editor.hpp"

#include	"map.h"

struct	DepthStrip	edit_map[EDIT_MAP_WIDTH][EDIT_MAP_DEPTH];  //2meg
UWORD	tex_map[EDIT_MAP_WIDTH][EDIT_MAP_DEPTH];

SBYTE	edit_map_roof_height[EDIT_MAP_WIDTH][EDIT_MAP_DEPTH];

