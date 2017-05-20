#ifndef	BUILDING_H
#define	BUILDING_H			1

//#include	"c:\fallen\headers\memory.h"

//DEFINES

//#define	TEXTURE_PIECE_TOP_LEFT		(0)
//#define	TEXTURE_PIECE_TOP_MIDDLE	(1)
//#define	TEXTURE_PIECE_TOP_RIGHT		(2)

#define	TEXTURE_PIECE_LEFT			(0)
#define	TEXTURE_PIECE_MIDDLE		(1)
#define	TEXTURE_PIECE_RIGHT			(2)
#define	TEXTURE_PIECE_MIDDLE1		(3)
#define	TEXTURE_PIECE_MIDDLE2		(4)
#define TEXTURE_PIECE_NUMBER		(5)



#if defined(PSX) || defined(TARGET_DC)
#define	RMAX_PRIM_POINTS		15000
#define	MAX_PRIM_FACES3		10000
#define	RMAX_PRIM_FACES4		10000
#define	MAX_PRIM_OBJECTS	2000
#define	MAX_PRIM_MOBJECTS	100



#else
#define	RMAX_PRIM_POINTS		65000
#define	MAX_PRIM_FACES3		32000
#define	RMAX_PRIM_FACES4		32760
#define	MAX_PRIM_OBJECTS	2000
#define	MAX_PRIM_MOBJECTS	100
#endif


#define	MAX_PRIM_POINTS		(save_table[SAVE_TABLE_POINTS].Maximum)
#define	MAX_PRIM_FACES4		(save_table[SAVE_TABLE_FACE4].Maximum)


#define	GRAB_FLOOR		(MAX_PRIM_FACES4+1)
#define GRAB_SEWERS		(MAX_PRIM_FACES4+2)

#define	MAX_ROOF_BOUND		2000  //(8k)


#define	BLOCK_SHIFT	(6)
#define	BLOCK_SIZE	(1<<BLOCK_SHIFT)


#define	BUILD_MODE_EDITOR	(1)
#define	BUILD_MODE_DX		(2)
#define	BUILD_MODE_SOFT		(3)
#define	BUILD_MODE_PSX		(4)
#define	BUILD_MODE_OTHER	(5)

#define BUILDING_TYPE_HOUSE			0
#define BUILDING_TYPE_WAREHOUSE		1
#define BUILDING_TYPE_OFFICE		2
#define BUILDING_TYPE_APARTEMENT	3
#define BUILDING_TYPE_CRATE_IN		4
#define BUILDING_TYPE_CRATE_OUT		5

#if defined(PSX)||defined(TARGET_DC)
#define	MAX_WINDOWS	1
#define	MAX_WALLS	1
#define	MAX_STOREYS	1
#define	MAX_BUILDINGS	1
#define	MAX_BUILDING_FACETS	1
#define	MAX_BUILDING_OBJECTS	1
#define	MAX_INSIDE_STOREYS	(512)

#else
#define	MAX_BUILDINGS	(500)
#define	MAX_STOREYS		(MAX_BUILDINGS*5)
#define	MAX_WALLS		(MAX_STOREYS*6)
#define	MAX_WINDOWS		(50)
#define	MAX_INSIDE_STOREYS	(1024)


#define	MAX_BUILDING_FACETS		4000
#define	MAX_BUILDING_OBJECTS	400
#endif


#define	FACET_FLAG_NEAR_SORT	(1<<0)
#define	FACET_FLAG_ROOF			(1<<1)
#define	FACET_FLAG_CABLE		(1<<2)
#define	FACET_FLAG_NON_SORT		(1<<3)

#define	FACET_FLAG_DONT_CULL	(1<<3)

#define FACET_FLAG_IN_SEWERS	(1<<4)
#define FACET_FLAG_LADDER_LINK	(1<<5)	// A ladder you use to go in and out of the sewers.





#define	FACET_FLAG_INVISIBLE	(1<<0)  // facet is duplicate so mark invisible
#define	FACET_FLAG_INSIDE		(1<<3)
#define FACET_FLAG_DLIT			(1<<4)	// Lit with a dynamic light.
#define FACET_FLAG_HUG_FLOOR	(1<<5)	// For fake fences that are normal walls
#define FACET_FLAG_ELECTRIFIED	(1<<6)	// For fences...
#define	FACET_FLAG_2SIDED		(1<<7)
#define FACET_FLAG_UNCLIMBABLE	(1<<8)
#define FACET_FLAG_ONBUILDING	(1<<9)
#define FACET_FLAG_BARB_TOP		(1<<10)
#define FACET_FLAG_SEETHROUGH	(1<<11)
#define FACET_FLAG_OPEN			(1<<12)	// For OUTSIDE_DOOR facets...
#define FACET_FLAG_90DEGREE		(1<<13)	// Some OUTSIDE_DOOR facets open only by 90 degrees...
#define	FACET_FLAG_2TEXTURED	(1<<14)
#define	FACET_FLAG_FENCE_CUT	(1<<15)

#define	FLAG_ROOF_FLAT			 (1<<1)
#define	FLAG_ROOF_OVERLAP_SMALL	 (1<<2)
#define	FLAG_ROOF_OVERLAP_MEDIUM (1<<3)
#define	FLAG_ROOF_WALLED		 (1<<4)
#define	FLAG_ROOF_RECESSED		 (1<<5)


#define	FLAG_WALL_USED			(1<<0)
#define	FLAG_WALL_AUTO_WINDOWS	(1<<1)
#define	FLAG_WALL_FACET_LINKED	(1<<2)
#define	FLAG_WALL_FENCE_POST1	(1<<3)
#define	FLAG_WALL_FENCE_POST2	(1<<4)
//#define	FLAG_WALL_ROOF_RIM		(1<<5)
#define	FLAG_WALL_RECESSED		(1<<5)
#define	FLAG_WALL_CLIMBABLE		(1<<6)
#define	FLAG_WALL_ARCH_TOP		(1<<7)
#define	FLAG_WALL_BARB_TOP		(1<<8)

#define	FLAG_STOREY_USED			(1<<0)
#define	FLAG_STOREY_FLAT_TILED_ROOF	(1<<1)
#define	FLAG_STOREY_LEDGE			(1<<2)
#define	FLAG_STOREY_ROOF			(1<<3)

#define	FLAG_STOREY_FACET_LINKED	(1<<4)
#define	FLAG_STOREY_TILED_ROOF		(1<<5)
#define	FLAG_STOREY_ROOF_RIM		(1<<6)
#define	FLAG_STOREY_ROOF_RIM2		(1<<7)

#define FLAG_STOREY_UNCLIMBABLE		(1<<10)
#define FLAG_STOREY_ONBUILDING		(1<<11)

#define FLAG_STOREY_EXTRA_UNCLIMBABLE	(1 << 0)
#define FLAG_STOREY_EXTRA_ONBUILDING	(1 << 1)
#define FLAG_STOREY_EXTRA_90DEGREE		(1 << 2)


#define	FLAG_ISTOREY_DOOR	(1<<1)

//#define	FLAG_STOREY_ROOF_RIM2		(1<<7)


#define	BROWN_BRICK1		1
#define	BROWN_BRICK2		2
#define	GREY_RIM1			3
#define	GREY_RIM2			4
#define	RED_WINDOW			5
#define	GREY_CORIGATED		6
#define	CRATES_SMALL_BROWN	7
#define	GREY_POSH			8
#define	HOTEL_SIGN1			9
#define	HOTEL_SIGN2			10

#define	STOREY_TYPE_NORMAL			(1)
#define	STOREY_TYPE_ROOF			(2)
#define	STOREY_TYPE_WALL			(3)
#define	STOREY_TYPE_ROOF_QUAD		(4)
#define	STOREY_TYPE_FLOOR_POINTS	(5)
#define	STOREY_TYPE_FIRE_ESCAPE		(6)
#define	STOREY_TYPE_STAIRCASE		(7)
#define	STOREY_TYPE_SKYLIGHT		(8)
#define	STOREY_TYPE_CABLE			(9)
#define	STOREY_TYPE_FENCE			(10)
#define	STOREY_TYPE_FENCE_BRICK		(11)
#define	STOREY_TYPE_LADDER			(12)
#define	STOREY_TYPE_FENCE_FLAT		(13)
#define	STOREY_TYPE_TRENCH			(14)
#define STOREY_TYPE_JUST_COLLISION	(15)
#define	STOREY_TYPE_PARTITION		(16)
#define	STOREY_TYPE_INSIDE			(17)
#define	STOREY_TYPE_DOOR			(18)
#define	STOREY_TYPE_INSIDE_DOOR		(19)
#define	STOREY_TYPE_OINSIDE			(20)
#define STOREY_TYPE_OUTSIDE_DOOR	(21)

#define	STOREY_TYPE_NORMAL_FOUNDATION	(100)

#define STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_BUT_A_VALUE_TO_PUT_IN_THE_PRIM_TYPE_FIELD_OF_COLVECTS_GENERATED_BY_INSIDE_BUILDINGS (254)
#define STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_AGAIN_THIS_IS_THE_VALUE_PUT_INTO_PRIMTYPE_BY_THE_SEWERS                             (255)


// STRUCTS

struct	BuildingObject
{
	SWORD	FacetHead;
	UWORD	StartFace4;
	UWORD	EndFace4;	 
	SWORD	StartFace3;
	SWORD	EndFace3;
	UWORD	StartPoint;
	UWORD	EndPoint;
};


struct	BuildingFacet
{
	UWORD	StartFace4;
	UWORD	MidFace4;
	UWORD	EndFace4;
	SWORD	StartFace3;
	SWORD	EndFace3;
	UWORD	StartPoint;
	UWORD	MidPoint;
	UWORD	EndPoint;
	SWORD	NextFacet;
	UWORD	FacetFlags;
	SWORD	ColVect;
};

struct	BoundBox
{
	UBYTE	MinX;
	UBYTE	MaxX;
	UBYTE	MinZ;
	UBYTE	MaxZ;
	SWORD	Y;
};

struct	TempBuilding
{
	UWORD	FacetHead;
	UWORD	FacetCount;

};
//4 bytes

struct	TempFacet
{
	SLONG	x1;
	SLONG 	z1;
	SLONG 	x2;
	SLONG 	z2;

	SWORD	Y;
	UWORD	PrevFacet;

	UWORD	NextFacet;
	UWORD	RoofType;

	UWORD	StoreyHead;
	UWORD	StoreyCount;

};
//28 bytes

struct	TempStorey
{
	UWORD	StoreyFlags;
	UBYTE	WallStyle;
	UBYTE	WindowStyle;
	SWORD	Height;
	SWORD	Next;
	SWORD	Count;
};
//10 bytes

struct	TXTY
{
	UBYTE	Page,Tx,Ty,Flip;

};

struct	DXTXTY
{
	UWORD	Page;
	UWORD	Flip;

};

struct	TextureInfo
{
	UBYTE	Type;
	UBYTE	SubType;
};


struct	FWindow
{
	UWORD	Dist;
	UWORD	Height;
	UWORD	WindowFlags;
	UWORD	WindowWidth;
	UWORD	WindowHeight;
	SWORD	Next;
	UWORD	Dummy[6];
};

struct	FWall
{
	SWORD	DX,DZ;
	UWORD	WallFlags;

	SWORD	TextureStyle2;
	UWORD	TextureStyle;

	UWORD	Tcount2;
	SWORD	Next;

	UWORD	DY;
	UWORD	StoreyHead;

	UBYTE	*Textures;
	UWORD	Tcount;
	UBYTE	*Textures2;
	
	UWORD	Dummy[1];
};

struct	FStorey
{
	SWORD	DX,DY,DZ;
	UBYTE	StoreyType;
	UBYTE	StoreyFlags;

	UWORD	Height;
	SWORD	WallHead;

//	UWORD	WallCount;
//	UWORD	Roof;

	UWORD	ExtraFlags;

	UWORD	InsideIDIndex; //*Textures;

	SWORD	Next;
	SWORD	Prev;
	SWORD	Info1;
	SWORD	Inside;
	UWORD	BuildingHead;

	UWORD	InsideStorey;
};

#define	MAX_ROOMS_PER_FLOOR		16
#define	MAX_STAIRS_PER_FLOOR	10


struct	RoomID
{
	UBYTE	X[MAX_ROOMS_PER_FLOOR],Y[MAX_ROOMS_PER_FLOOR];
	UBYTE	Flag[MAX_ROOMS_PER_FLOOR];

	UBYTE	StairsX[MAX_STAIRS_PER_FLOOR];
	UBYTE	StairsY[MAX_STAIRS_PER_FLOOR];
	UBYTE	StairFlags[MAX_STAIRS_PER_FLOOR];

	UBYTE	FloorType;

	UBYTE	Dummy[5*4];

};


#define	STAIR_FLAG_UP		(1<<0)
#define	STAIR_FLAG_DOWN	(1<<1)
#define	STAIR_FLAGS_DIR		((1<<2)|(1<<3))
#define	SET_STAIR_DIR(x,dir)	((x)=((x)&~STAIR_FLAGS_DIR)|(dir<<2))
#define	GET_STAIR_DIR(x)	(((x)&STAIR_FLAGS_DIR)>>2)
struct	FBuilding
{
	UWORD	ThingIndex;
	UWORD	LastDrawn;	// The GAME_TURN this building was last drawn at.
	UBYTE	Dummy2;
	UBYTE	Foundation;
	SWORD	OffsetY;
	UWORD   InsideSeed;
	UBYTE	MinFoundation;
	UBYTE	ExtraFoundation;
	UWORD	BuildingFlags;
	SWORD	StoreyHead;
	SWORD	Angle;
	UWORD	StoreyCount;
	CBYTE	str[20];
	UBYTE	StairSeed;
	UBYTE	BuildingType;
	UWORD	Walkable;
	UWORD	Dummy[4];
};

typedef SVector PrimNormal;


//data
#ifndef PSX
extern	struct	DXTXTY	dx_textures_xy[200][5];
#endif

extern	UWORD	next_roof_bound;


extern	UWORD	next_prim_point;
extern	UWORD	next_prim_face4;
extern	UWORD	next_prim_face3;
extern	UWORD	next_prim_object;
extern	UWORD	next_prim_multi_object;

extern	UWORD	end_prim_point;
extern	UWORD	end_prim_face4;
extern	UWORD	end_prim_face3;
extern	UWORD	end_prim_object;
extern	UWORD	end_prim_multi_object;


extern	UWORD	next_building_object;
extern	UWORD	end_building_object;

extern	UWORD	next_building_facet;
extern	UWORD	end_building_facet;



extern	struct	BuildingObject	building_objects[];
extern	struct	BuildingFacet	building_facets[];

extern	struct	FWindow		window_list[MAX_WINDOWS];
extern	struct	FWall		wall_list[MAX_WALLS];
extern	struct	FStorey		storey_list[MAX_STOREYS];
extern	struct	FBuilding	building_list[MAX_BUILDINGS];
extern	struct	TXTY	textures_xy[200][5];
extern			UBYTE	textures_flags[200][5];
extern	struct	TextureInfo texture_info[];

extern	struct	RoomID	room_ids[MAX_INSIDE_STOREYS];
extern	SLONG	next_inside;

// functions

extern	SLONG	place_building_at(UWORD prim,SLONG x,SLONG y,SLONG z);
extern	void	offset_buildings(SLONG x,SLONG y,SLONG z);
//extern	void	calc_buildings_world_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);
//extern	void	calc_buildings_screen_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);
extern	void	draw_a_building_at(UWORD building,SLONG x,SLONG y,SLONG z);
extern	void	create_city(UBYTE flag);
extern	SLONG	create_building_prim(UWORD building,SLONG	*small_y);
extern	SLONG	next_connected_face(SLONG type,SLONG id,SLONG count);
extern	SLONG	is_storey_circular(SLONG storey);

//
// Returns the start and end of the given wall.
//

void get_wall_start_and_end(
		SLONG wall,

		//
		// These are 16-bit map coordinates...
		//

		SLONG *x1, SLONG *z1,	
		SLONG *x2, SLONG *z2);


//
// Returns how far (x,z) is along a cable.  Value goes from 0 to
// (1 << CABLE_ALONG_SHIFT)
//

#define CABLE_ALONG_SHIFT 12
#define CABLE_ALONG_MAX   (1 << CABLE_ALONG_SHIFT)

SLONG get_cable_along(
		SLONG storey,
		SLONG x,
		SLONG z);


//
// Make a cable taut, as if someone is dangling at some point along it.
// It returns the height of the point she is dangling at.
//

//
// If you know how far she is along the cable. This is the percentage
// straight-line distance from (x1,y1,z1) of the cable to (x2,y2,z2).
// 'along' goes from 0 to 256.
//

void make_cable_taut_along(
		SLONG          along,	// 0 - 256
		SLONG          building,
		SLONG         *x_hanging_point,
		SLONG         *y_hanging_point,
		SLONG         *z_hanging_point);

//
// Returns the cable to being flabby.
//

void make_cable_flabby(SLONG building);



#endif