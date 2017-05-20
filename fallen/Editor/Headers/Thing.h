// Thing.h
// Guy Simmons, 15th October 1997.

#ifndef	ETHING_H
#define	ETHING_H

#include	"anim.h"
#include	"DarkCity.h"


//---------------------------------------------------------------
// old style

//defs
#define	MAX_MAP_THINGS	2000


#define	MAP_THING_TYPE_PRIM				1
#define	MAP_THING_TYPE_MULTI_PRIM		2
#define	MAP_THING_TYPE_ROT_MULTI		3
#define	MAP_THING_TYPE_SPRITE			4
#define	MAP_THING_TYPE_AGENT			5
#define	MAP_THING_TYPE_LIGHT			6
#define	MAP_THING_TYPE_BUILDING			7
#define	MAP_THING_TYPE_ANIM_PRIM		8

// Game editor stuff.
#define	MAP_THING_TYPE_ED_THING			8

#define	TO_MTHING(m)	&map_things[(m)]

//structs
struct	MapThing
{
	SLONG	X;	
	SLONG	Y;	
	SLONG	Z;	
	UWORD	MapChild;   //mapwho 2 way linked list
	UWORD	MapParent;
	UBYTE	Type;
	UBYTE	SubType;	// Type for lights...
	ULONG	Flags;

	union
	{
		struct
		{
			SWORD	IndexOther;		// Brightness for lights...
			UWORD	Width;
			UWORD	Height;
			UWORD	IndexOrig;		// param for lights...
			UWORD	AngleX;			// (R,G,B) for lights...
			UWORD	AngleY;
			UWORD	AngleZ;
			UWORD	IndexNext;
			SWORD	LinkSame;
			SWORD	OnFace;
			SWORD	State;
			SWORD	SubState;
			SLONG	BuildingList;
			ULONG	EditorFlags,
					EditorData;
			ULONG	DummyArea[3];
			SLONG						TweenStage;
			//struct KeyFrameElement		*AnimElements,
			//								*NextAnimElements;
			KeyFrame	*CurrentFrame;
			KeyFrame	*NextFrame;
		};

		struct
		{
			UWORD		CommandRef;
			SLONG		Class,
						Genus;
			SLONG		Data[6];
		};
	};
};

struct	MapThingPSX
{
	SLONG	X;	
	SLONG	Y;	
	SLONG	Z;	
	UWORD	MapChild;   //mapwho 2 way linked list
	UWORD	MapParent;
	UBYTE	Type;
	UBYTE	SubType;	// Type for lights...
	ULONG	Flags;

			SWORD	IndexOther;		// Brightness for lights...
			UWORD	Width;
			UWORD	Height;
			UWORD	IndexOrig;		// param for lights...
			UWORD	AngleX;			// (R,G,B) for lights...
			UWORD	AngleY;
			UWORD	AngleZ;
			UWORD	IndexNext;
			SWORD	LinkSame;
			SWORD	OnFace;
			SWORD	State;
			SWORD	SubState;
			SLONG	BuildingList;
			ULONG	EditorFlags,
					EditorData;
			ULONG	DummyArea[3];
			SLONG						TweenStage;
			//struct KeyFrameElement		*AnimElements,
			//								*NextAnimElements;
			KeyFrame	*CurrentFrame;
			KeyFrame	*NextFrame;

};


//data
extern	struct	MapThing	map_things[MAX_MAP_THINGS];

//code
extern	UWORD	find_empty_map_thing(void);
extern	void	delete_thing_from_edit_map(SLONG x,SLONG y,UWORD	thing);
extern	void	add_thing_to_edit_map(SLONG x,SLONG y,UWORD	thing);
extern	SLONG	move_thing_on_cells(UWORD thing,SLONG x,SLONG y,SLONG z);
extern	void	delete_thing(SWORD index);
//---------------------------------------------------------------
/*
void			init_things(void);
THING_INDEX		alloc_primary_thing(void);
void			free_primary_thing(THING_INDEX thing);
THING_INDEX		alloc_secondary_thing(void);
void		free_secondary_thing(THING_INDEX thing);
*/
//---------------------------------------------------------------

#endif
