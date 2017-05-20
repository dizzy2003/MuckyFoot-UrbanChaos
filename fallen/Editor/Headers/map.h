#ifndef	EMAP_H
#define	EMAP_H	1

#undef ELE_SIZE

//defs
#define	ELE_SHIFT				(8)
#define	ELE_SIZE				(1<<ELE_SHIFT)
#define	HALF_ELE_SIZE			((ELE_SIZE)>>1)
#define	ELE_AND					(0xffffff00)

#define EDIT_MAP_WIDTH	128
//1024
#define EDIT_MAP_HEIGHT	1
#define EDIT_MAP_DEPTH	128
//4

//structs


struct	DepthStrip
{
	UWORD	MapThingIndex;
//	UWORD	Depth[EDIT_MAP_DEPTH];
	UWORD	ColVectHead;
//	UWORD	Dummy1;
	UWORD	Texture;
	SWORD	Bright;
	UBYTE	Flags;
	SBYTE	Y;
	SWORD	Walkable;
};

//data
extern	struct	DepthStrip	edit_map[EDIT_MAP_WIDTH][EDIT_MAP_DEPTH];  //2meg
extern	SBYTE	edit_map_roof_height[EDIT_MAP_WIDTH][EDIT_MAP_DEPTH];
extern	UWORD	tex_map[EDIT_MAP_WIDTH][EDIT_MAP_DEPTH];


//code

#endif