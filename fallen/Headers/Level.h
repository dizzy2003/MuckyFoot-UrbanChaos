// Level.h
// Guy Simmons, 26th January 1998.

#ifndef	LEVEL_H
#define	LEVEL_H

//---------------------------------------------------------------

struct	ThingDef
{
	UBYTE		Version;

	SLONG		Class,
				Genus,
				X,Y,Z;
	UWORD		CommandRef;
	SLONG		Data[10];
	UWORD		EdThingRef;
};

//---------------------------------------------------------------

struct	WaypointDef
{
	UBYTE		Version;

	UWORD		Next,
				Prev;
	SLONG		X,Y,Z;
	UWORD		EdWaypointRef;
};

//---------------------------------------------------------------

struct	ConditionListDef
{
	UBYTE		Version;

	CBYTE		ListName[32];
	ULONG		ConditionCount;
	UWORD		EdConListRef;
};

//---------------------------------------------------------------

struct	ConditionDef
{
	UBYTE		Version;

	UWORD		Flags,
				ConditionType,
				GroupRef;
	SLONG		Data1,
				Data2,
				Data3;
};

//---------------------------------------------------------------

struct	CommandListDef
{
	UBYTE		Version;

	CBYTE		ListName[32];
	ULONG		CommandCount;
	UWORD		EdComListRef;
};

//---------------------------------------------------------------

struct	CommandDef
{
	UBYTE		Version;

	UWORD		Flags,
				CommandType,
				GroupRef;
	SLONG		Data1,
				Data2,
				Data3;
};

//---------------------------------------------------------------
// Level format.

// VERSION 0

//	UBYTE		Version
//	ULONG		ThingCount
//	ThingDef	ThingDefs * ThingCount


// VERSION 1

//	UBYTE		Version
//	ULONG		ThingCount
//	ThingDef	ThingDefs * ThingCount
//	ULONG		WaypointCount
//	WaypointDef	WaypointDefs * WaypointCount

// VERSION 2

//	UBYTE		Version
//	ULONG		ThingCount
//	ThingDef	ThingDefs * ThingCount
//	ULONG		WaypointCount
//	WaypointDef	WaypointDefs * WaypointCount
//	ULONG		ConditionListCount
//	(
//	ConditionListDef
//	ConditionDef * ConditionListDef.ConditionCount
//	)	*	ConditionListCount

// VERSION 3

//	UBYTE		Version
//	ULONG		ThingCount
//	ThingDef	ThingDefs * ThingCount
//	ULONG		WaypointCount
//	WaypointDef	WaypointDefs * WaypointCount
//	ULONG		ConditionListCount
//	(
//	ConditionListDef
//	ConditionDef * ConditionListDef.ConditionCount
//	)	*	ConditionListCount
//	ULONG		CommandListCount
//	(
//	CommandListDef
//	ConmmandDef * CommandListDef.CommandCount
//	)	*	CommandListCount

//---------------------------------------------------------------

BOOL	load_level(ULONG level);

//---------------------------------------------------------------

#endif

