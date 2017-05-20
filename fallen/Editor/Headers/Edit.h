#ifndef	EDIT_H
#define	EDIT_H			1

#include	"primativ.hpp"
#include	"Anim.h"
//#include	"Engine.h"
//DEFINES



#define	MAX_D_LIGHTS				10

#define	CUBE_INDEX_FRONT	(0)
#define	CUBE_INDEX_TOP		(1)
#define	CUBE_INDEX_BOTTOM	(2)
#define	CUBE_INDEX_LEFT		(3)
#define	CUBE_INDEX_RIGHT	(4)
#define	CUBE_INDEX_BACK		(5)


#define	CUBE_FLAG_FRONT		(1<<0)
#define	CUBE_FLAG_TOP		(1<<1)
#define	CUBE_FLAG_BOTTOM	(1<<2)
#define	CUBE_FLAG_LEFT		(1<<3)
#define	CUBE_FLAG_RIGHT		(1<<4)
#define	CUBE_FLAG_BACK		(1<<5)
#define CUBE_FLAG_ALL		(0x3f)
#define CUBE_FLAG_MULTI		(1<<6)


#define	CUBE_TYPE_FULL			(1)
#define	CUBE_TYPE_SLOPE_LR		(2)
#define	CUBE_TYPE_STEPS_LR		(3)
#define	CUBE_TYPE_LEDGE1		(4)
#define	CUBE_TYPE_END			(5)


#define	TEXTURE_DEFAULT_ROCKS		(3+(6<<3)+(1<<6))
#define	TEXTURE_DEFAULT_BEIGEWALL	(1+(6<<3)+(1<<6))
#define	TEXTURE_DEFAULT_BEIGESLATS	(1+(6<<3)+(1<<6))

#define	MAX_EDIT_FACE_LIST	3000


/*********************************************
// STRUCTS
**********************************************/
struct	EditInfo
{
	EdRect		SelectRect;
	SWORD		SelectFlag;
	CBYTE		MapName[30];
	SLONG		amb_dx,amb_dy,amb_dz,amb_bright,amb_offset,amb_flags;
	UWORD		GridOn;
	SLONG		TileFlag,TileScale;
	SLONG		DX,DY,DZ;
	SLONG		OX,OY,OZ;
	UWORD		FlatShade;
	UWORD		NoHidden;
	SLONG		MinX,MinZ,MaxX,MaxZ;
	UWORD		Clipped;
	UWORD		RoofTex;
	UWORD		Inside;
	UWORD		InsideHeight;
	UWORD		MapID;
	UWORD		HideMap;
};

struct	Light
{
	SWORD	Intensity;
	SWORD	R;

	SWORD	G;
	SWORD	B;

	SLONG	X;
	SLONG	Y;
	SLONG	Z;
};



struct	OldDepthStrip
{
	UWORD	MapThingIndex;
	UWORD	Depth[4];
	UWORD	ColVectHead;
	UWORD	Dummy1;
	UWORD	Dummy2;
};


//
// These are mirrored in MAP.H in the FALLEN project.
// 

//#define	FLOOR_FLIP_FLAG		(1<<0)
//#define	FLOOR_BRIGHT1		(1<<1)
//#define	FLOOR_BRIGHT2		(1<<2)
//#define	FLOOR_HIDDEN		(1<<3)
//#define FLOOR_SINK_SQUARE	(1<<4)	// Lowers the floorsquare to create a curb.
//#define FLOOR_SINK_POINT	(1<<5)	// Transform the point on the lower level.

#define	FLOOR_HEIGHT_SHIFT	(3)


struct	CubeBits
{
	UBYTE	Prim:3;
   	UBYTE	Rot:3;
   	UBYTE	Spare:2;
};

struct	TextureBits
{
	ULONG 	X:5;
	ULONG 	Y:5;
	ULONG	Page:4;
	ULONG	Rot:2;
	ULONG	Flip:2;
	ULONG	DrawFlags:8;
	ULONG	Width:3;
	ULONG	Height:3;
};
/*
struct	MiniTextureBits
{
	UWORD 	X:3;
	UWORD 	Y:3;
	UWORD	Page:4;
	UWORD	Rot:2;
	UWORD	Flip:2;
	UWORD	Size:2;
};
*/
struct	EditMapElement
{
	struct	CubeBits	CubeType;
	UBYTE	CubeFlags;
	struct	TextureBits	Textures[5];  //can never see 6th face
};
			
struct	EditFace
{
	SLONG	MapX;
	SLONG	MapY;
	SLONG	MapZ;
	SLONG	Z;
	SWORD	Face;
	UWORD	Flag;
	struct	EditMapElement	*PEle;
	SLONG	EditTurn;
	struct BucketHead		*Bucket;
};



/*********************************************
// DATA
**********************************************/
extern struct SVector	selected_prim_xyz;
extern	UBYTE	texture_sizes[];

extern	struct	Light	d_lights[];
extern	UWORD	next_d_light;


extern ULONG		editor_turn;
// extern EditFace		edit_face;
extern EditFace		hilited_face,
					selected_face;


//extern	struct	EditMapElement	edit_map_eles[65000];


extern	SWORD	face_selected_list[MAX_EDIT_FACE_LIST];
extern	SWORD	next_face_selected;

extern	struct	EditInfo	edit_info;	

/*********************************************
// FUNCTIONS
**********************************************/
extern	UWORD	is_it_clockwise_xy(SLONG x1,SLONG y1,SLONG x2,SLONG y2,SLONG x3,SLONG y3);


extern	SLONG	add_a_light(SWORD i,SLONG x,SLONG y,SLONG z);
extern	void	clear_lights(void);

extern	void	calc_things_screen_box(SLONG	map_thing,EdRect *rect);
extern	SLONG	hilight_map_backgrounds(UWORD type);
extern	SLONG	hilight_map_things(UWORD type);
extern	SLONG	select_map_things(MFPoint *mouse,UWORD type);
extern	SLONG	select_map_backgrounds(MFPoint *mouse,UWORD type);
				
extern	void	init_editor(void);
extern	void	draw_editor_map(ULONG);
extern	UBYTE	check_big_point_triangle(SLONG x,SLONG y,SLONG ux,SLONG uy,SLONG vx,SLONG vy,SLONG wx,SLONG wy);
extern	BOOL	check_mouse_over_prim_quad(struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG p4,SLONG face);
extern	BOOL	check_mouse_over_prim_tri(struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG face);
extern	void	set_quad_buckets_texture(struct	BucketQuad	*p_bucket,struct	TextureBits *texture);
extern	ULONG	editor_user_interface(UBYTE type);
extern	SLONG	place_prim_at(UWORD prim,SLONG x,SLONG y,SLONG z);
extern	SLONG	load_map(CBYTE	*name);
extern	void	save_map(CBYTE	*name,SLONG quick);
extern	ULONG	engine_keys_scroll(void);
extern	ULONG	engine_keys_zoom(void);
extern	ULONG	engine_keys_spin(void);
extern	void	set_things_faces(SWORD thing);
extern	void	add_face_to_list(SWORD face);
extern	SLONG	face_is_in_list(SWORD face);
extern	void	gamut_fiddle(void);
extern	void	clear_map(void);
extern	void	draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col);
extern	void	set_screen_box(SLONG x,SLONG y,SLONG z,EdRect *rect,SLONG w,SLONG h);
extern	void	build_radius_info(void);
extern	void	draw_quick_map(void);
extern	ULONG	engine_keys_scroll_game(void);
extern	SLONG	add_floor_face_to_bucket(SLONG	x1,SLONG	y1,SLONG	z1,SLONG	x2,SLONG	y2,SLONG	z2,SLONG	x3,SLONG	y3,SLONG	z3,SLONG	x4,SLONG	y4,SLONG	z4,struct	DepthStrip	*p_map,SLONG b1,SLONG b2,SLONG b3,SLONG b4,UWORD tex);


#endif
