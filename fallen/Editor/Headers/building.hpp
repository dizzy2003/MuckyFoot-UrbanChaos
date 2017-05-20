#ifndef	BUILDING_H
#define	BUILDING_H			1

//DEFINES
#define	MAX_BUILDINGS	(500)
#define	MAX_STOREYS		(MAX_BUILDINGS*5)
#define	MAX_WALLS		(MAX_STOREYS*6)
#define	MAX_WINDOWS		(MAX_WALLS*4)


#define	MAX_BUILDING_FACETS		2000
#define	MAX_BUILDING_OBJECTS	200


#define	FACET_FLAG_NEAR_SORT	(1<<0)
#define	FACET_FLAG_ROOF			(1<<1)
#define	FACET_FLAG_CABLE		(1<<2)

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
#define	FLAG_WALL_ROOF_RIM		(1<<5)
#define	FLAG_WALL_ARCH_SIDE		(1<<6)
#define	FLAG_WALL_ARCH_TOP		(1<<7)

#define	FLAG_STOREY_USED		(1<<0)
#define	FLAG_STOREY_NORMAL		(1<<1)
#define	FLAG_STOREY_LEDGE		(1<<2)
#define	FLAG_STOREY_ROOF		(1<<3)

#define	FLAG_STOREY_FACET_LINKED	(1<<4)
#define	FLAG_STOREY_TILED_ROOF		(1<<5)
#define	FLAG_STOREY_ROOF_RIM		(1<<6)


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
#define	STOREY_TYPE_DRAINPIPE		(8)
#define	STOREY_TYPE_CABLE			(9)
#define	STOREY_TYPE_FENCE			(10)

#define	FACE_TYPE_FIRE_ESCAPE	1

// STRUCTS

struct	BuildingObject
{
	SWORD	FacetHead;
	SWORD	StartFace4;
	SWORD	EndFace4;	 
	SWORD	StartFace3;
	SWORD	EndFace3;
	SWORD	StartPoint;
	SWORD	EndPoint;
};


struct	BuildingFacet
{
	SWORD	StartFace4;
	SWORD	MidFace4;
	SWORD	EndFace4;
	SWORD	StartFace3;
	SWORD	EndFace3;
	SWORD	StartPoint;
	SWORD	MidPoint;
	SWORD	EndPoint;
	SWORD	NextFacet;
	UWORD	FacetFlags;
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
	SWORD	WindowHead;
	UWORD	TextureStyle;
	UWORD	WindowCount;
	SWORD	Next;
	UWORD	DY;
	UWORD	StoreyHead;
	UWORD	Dummy[6];
};

struct	FStorey
{
	SWORD	DX,DY,DZ;
	UBYTE	StoreyType;
	UBYTE	StoreyFlags;

	UWORD	Height;
	SWORD	WallHead;

	UWORD	WallCount;
	UWORD	Roof;

	SWORD	Next;
	SWORD	Prev;
	SWORD	Info1;
	SWORD	Info2;
	UWORD	BuildingHead;

	UWORD	Dummy[1];
};

struct	FBuilding
{
	SLONG	X,Y,Z;
	UWORD	BuildingFlags;
	SWORD	StoreyHead;
	SWORD	Angle;
	UWORD	StoreyCount;
	CBYTE	str[20];
	UWORD	Dummy[6];
};



//data

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
extern	struct	TXTY	textures_xy[200][8];
extern			UBYTE	textures_flags[200][8];
extern	struct	TextureInfo texture_info[];


// functions

extern	SLONG	place_building_at(UWORD prim,SLONG x,SLONG y,SLONG z);
extern	void	offset_buildings(SLONG x,SLONG y,SLONG z);
//extern	void	calc_buildings_world_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);
//extern	void	calc_buildings_screen_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);
extern	void	draw_a_building_at(UWORD building,SLONG x,SLONG y,SLONG z);
extern	void	create_city(void);
extern	SLONG	create_building_prim(UWORD building,SLONG	*small_y);
extern	SLONG	next_connected_face(SLONG type,SLONG id,SLONG count);
extern	SLONG	is_storey_circular(SLONG storey);

#endif