#ifndef	SUPERMAP_H
#define	SUPERMAP_H	1

#ifdef	PSX
//
// PSX include
//
#include "libsn.h"
/*
#define	MFFileHandle	SLONG
#define	FILE_OPEN_ERROR	(-1)
#define	SEEK_MODE_CURRENT	(1)

#define	FileOpen(x)		PCopen(x,0,0)
#define	FileClose(x)	PCclose(x)
#define	FileCreate(x,y)	PCopen(x,1,0)
#define	FileRead(h,a,s) PCread(h,(char*)a,s)
#define	FileWrite(h,a,s) PCwrite(h,(char*)a,s)
#define	FileSeek(h,m,o) PClseek(h,o,m)

#endif
*/



#define	MFFileHandle	SLONG
#define	FILE_OPEN_ERROR	(-1)
#define	SEEK_MODE_CURRENT	(1)

extern	SLONG	SpecialOpen(CBYTE *name);
extern	SLONG	SpecialRead(SLONG handle,UBYTE *ptr,SLONG s1);
extern	SLONG	SpecialSeek(SLONG handle,SLONG mode,SLONG size);

#define	FileOpen(x)		SpecialOpen(x)
#define	FileClose(x)	SpecialClose(x)
#define	FileCreate(x,y)	ASSERT(0)
#define	FileRead(h,a,s) SpecialRead(h,(char*)a,s)
#define	FileWrite(h,a,s) ASSERT(0)
#define	FileSeek(h,m,o) SpecialSeek(h,m,o)


#endif



#define	MAX_FACET_LINK	32000


struct	DStorey
{
	UWORD	Style; //replacement style           // maybe this could be a byte
	UWORD	Index; //Index to painted info
	SBYTE	Count; //+ve is a style  //-ve is a  //get rid of this
	UBYTE	BloodyPadding;
};

struct DFacet
{
	UBYTE	FacetType;
	UBYTE	Height;
	UBYTE	x[2];		// these are bytes because they are grid based 
	SWORD	Y[2];
	UBYTE	z[2];		// these are bytes because they are grid based 
	UWORD	FacetFlags;
	UWORD	StyleIndex;
	UWORD	Building;
	UWORD	DStorey;
	UBYTE	FHeight;
	UBYTE	BlockHeight;
	UBYTE	Open;				// How open or closed a STOREY_TYPE_OUTSIDE_DOOR is.
	UBYTE   Dfcache;			// Index into NIGHT_dfcache[] or NULL...
	UBYTE	Shake;				// When a fence has been hit hard by something.
	UBYTE	CutHole;			
	UBYTE	Counter[2];
};

struct DBuilding
{
	SLONG	X,Y,Z;
	UWORD	StartFacet;
	UWORD	EndFacet;
	UWORD	Walkable;
	UBYTE	Counter[2];
	UWORD	Padding;
	UBYTE	Ware;		// If this building is a warehouse, this is an index into the WARE_ware[] array
	UBYTE	Type;
};

struct	DWalkable
{
	UWORD	StartPoint;	// Unused nowadays
	UWORD	EndPoint;  	// Unused nowadays
	UWORD	StartFace3;	// Unused nowadays
	UWORD	EndFace3;  	// Unused nowadays

	UWORD	StartFace4;	// These are indices into the roof faces
	UWORD	EndFace4;

	UBYTE	X1;
	UBYTE	Z1;
	UBYTE	X2;
	UBYTE	Z2;
	UBYTE	Y;
	UBYTE	StoreyY;
	UWORD	Next;
	UWORD	Building;
};

struct	DInsideRect
{
	UBYTE	MapX;
	UBYTE	MapZ;
	UBYTE	Width;
	UBYTE	Depth;
	UBYTE	StoreyY;
	UBYTE	Flags;      // bound to need flags plus it pads us out nicely
	UWORD	BitIndex;   // index to block of data for inside buildings
};


// bits 0->3      16 room ID's   0== no entry
// bits 4->5      Direction
// bits 6->7      Type

#define	CALC_INSIDE_ID(id)			(id&3)
#define	CALC_INSIDE_DIRECTION(id)	((id&3)<<4)
#define	CALC_INSIDE_TYPE(id)		((id&3)<<6)


#define	GET_INSIDE_ID(id)			(id&3)
#define	GET_INSIDE_DIRECTION(id)	((id>>4)&3)
#define	GET_INSIDE_TYPE(id)			((id>>6)&3)


extern	SWORD	next_paint_mem;
extern	SWORD	next_dstorey;
extern	SLONG	next_inside_mem;


extern	SWORD	next_facet_link;
extern	SWORD	facet_link_count;

#define MAX_DBUILDINGS	1024
#define MAX_DFACETS		16384
#define MAX_DWALKABLES	2048
#define MAX_DSTYLES		10000
#define MAX_DSTOREYS	10000
#define MAX_PAINTMEM	64000




extern	SLONG	next_dwalkable;
extern	SLONG	next_dbuilding;
extern	SLONG	next_dfacet;
extern	SLONG	next_dstyle;


void	save_super_map(MFFileHandle	handle);
void	load_super_map(MFFileHandle	handle,SLONG st);

//
// Identifies the subset of the primpoints that are used
// by the walkable faces.
//

extern SLONG first_walkable_prim_point;
extern SLONG number_of_walkable_prim_points;

extern SLONG first_walkable_prim_face4;
extern SLONG number_of_walkable_prim_faces4;

//
// Adds a sewer ladder facet.  If (link) then this is a ladder that
// links the sewers to the outside world.
//

void add_sewer_ladder(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2,
		SLONG bottom,
		SLONG height,
		SLONG link);

//
// Finds the nearest electric fence DBUILDING to the given point.
// Returns NULL if it couldn't find one withing the given range.
//

SLONG find_electric_fence_dbuilding(
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG range);

//
// Sets the state of the given electric fence dbuilding. It sets the
// flags in all the facets of the dbuilding.
//

void set_electric_fence_state(SLONG dbuilding, SLONG onoroff);

// ========================================================
//
// TO THINGS TO DFACETS ONLY ONCE
//
// ========================================================

extern UBYTE SUPERMAP_counter[2];	// One for each camera...

//
// This function increases the SUPERMAP_counter to a value that it guarantees
// will not be present in any of the dfacets or dbuildings Counter[] arrays.
//

void SUPERMAP_counter_increase(UBYTE which);



// ========================================================
//
// EDITOR HACK
//
//
// This is a hack put it to let the game ENTER, STAIR and ID modules
// work on buildings in the editor.
//
// It converts just the given building into the supermap structure
// and puts it into dbulding[1]
//

void create_super_dbuilding(SLONG building);

//
// ========================================================



#endif


