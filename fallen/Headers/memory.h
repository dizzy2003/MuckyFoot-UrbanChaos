#ifndef	MEMORY_H
#define	MEMORY_H

#include	"supermap.h"
#include	"inside2.h"
#include	"building.h"

//
// The map beacons.
//

typedef struct
{
	UBYTE  used;
	UBYTE  counter;
	UWORD  track_thing;
	UWORD	index;
	UWORD	pad;
//	CBYTE *str;
	SLONG  wx;
	SLONG  wz;
	ULONG  ticks;	// The GetTickCount when we created the beacon. LAZY!

} MAP_Beacon;

#define MAP_MAX_BEACONS 32
extern	MAP_Beacon *MAP_beacon; //[MAP_MAX_BEACONS];


typedef	UWORD  PSX_TEX[5];

extern	PSX_TEX *psx_textures_xy; //[PAP_SIZE_LO][PAP_SIZE_LO];

extern	SWORD	*facet_links; //[MAX_FACET_LINK];

extern	struct DBuilding	*dbuildings;//[MAX_DBUILDINGS];
extern	struct DFacet		*dfacets;   //[MAX_DFACETS	 ];
extern	struct	DWalkable	*dwalkables;//[MAX_DWALKABLES];
extern	SWORD				*dstyles;   //[MAX_DSTYLES	 ];
extern	struct	DStorey		*dstoreys;  //[MAX_DSTOREYS];

extern	UBYTE	*paint_mem; //[MAX_PAINTMEM];

//
// from inside2
//

extern	struct	InsideStorey	*inside_storeys;//[MAX_INSIDE_RECT];
extern	struct	Staircase		*inside_stairs;//[MAX_INSIDE_STAIRS];
extern	UBYTE	*inside_block;//[MAX_INSIDE_MEM];
extern	UBYTE	inside_tex[64][16];


//
// from building.cpp
//

extern	struct	BoundBox	*roof_bounds;//[MAX_ROOF_BOUND];
extern	struct	PrimPoint *prim_points;//[MAX_PRIM_POINTS];
extern	struct	PrimFace4 *prim_faces4;//[MAX_PRIM_FACES4];
extern	struct	PrimFace3 *prim_faces3;//[MAX_PRIM_FACES3];
extern	struct	PrimObject	*prim_objects;//[MAX_PRIM_OBJECTS];
extern	struct	PrimMultiObject	*prim_multi_objects;//[MAX_PRIM_MOBJECTS];
extern	PrimNormal *prim_normal;//[MAX_PRIM_POINTS];

extern	PrimInfo *prim_info;//[256];//MAX_PRIM_OBJECTS];



// functions
extern	void	init_memory(void);



//extern OB_Mapwho OB_mapwho;//[OB_SIZE][OB_SIZE];



#define	MAX_ROOF_FACE4	10000
extern	UWORD	next_roof_face4;
extern	struct	RoofFace4	*roof_faces4;



#ifndef PSX

//
// Quick load\save
//

void  MEMORY_quick_init(void);

void  MEMORY_quick_save(void);

SLONG MEMORY_quick_load_available(void);
SLONG MEMORY_quick_load          (void);

#endif



#endif

