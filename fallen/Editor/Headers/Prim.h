#ifndef	PRIM_H
#define	PRIM_H			1

//DEFINES


// Set this to 1 to enable TomF's new D3D-friendly engine.
// 0 enables the old engine again.
// NOTE! There are other versions of this define dotted around in other header
// files! Make sure they all agree or you'll have grief.
#ifdef TARGET_DC
#define USE_TOMS_ENGINE_PLEASE_BOB 1
#else
#define USE_TOMS_ENGINE_PLEASE_BOB 1
#endif


//
// face savetype for primpoint size change
//
#define 	PRIM_START_SAVE_TYPE	5793
#define	PRIM_END_SAVE_TYPE		5800

//
// roof face flags
//

#define RFACE_FLAG_SHADOW_1		(1<<0)		// Form a number from 0-7
#define RFACE_FLAG_SHADOW_2		(1<<1)
#define RFACE_FLAG_SHADOW_3		(1<<2)
#define RFACE_FLAG_SLIDE_EDGE	(1 << 3)
#define RFACE_FLAG_SLIDE_EDGE_0	(1 << 3)
#define RFACE_FLAG_SLIDE_EDGE_1	(1 << 4)
#define RFACE_FLAG_SLIDE_EDGE_2	(1 << 5)
#define RFACE_FLAG_SLIDE_EDGE_3	(1 << 6)

#define	RFACE_FLAG_SPLIT		(1 << 7)	// Seeing as we aren't using this...
#define RFACE_FLAG_NODRAW		(1 << 7)	// ...I'll sneak in this flag! Its temporary.



#define	GAME_SCALE			(256.0f) //(1660.0+200)   //3660
#define	GAME_SCALE_DIV		(100.0f) //170.0 //100.0

#define FACE_FLAG_WMOVE			(1<<0)	// A moving walkable face- ThingIndex in an index into the WMOVE_face array.
#define	FACE_FLAG_SMOOTH		(1<<0)
#define	FACE_FLAG_OUTLINE		(1<<1)
#define FACE_FLAG_SHADOW_1		(1<<2)	// Form a number from 0-7
#define FACE_FLAG_SHADOW_2		(1<<3)
#define FACE_FLAG_SHADOW_3		(1<<4)
#define	FACE_FLAG_ROOF			(1<<5)
#define	FACE_FLAG_WALKABLE		(1<<6)
#define FACE_FLAG_ENVMAP		(1<<7)
#define FACE_FLAG_VERTICAL		(1<<7)

#define FACE_FLAG_TINT			(1<<8)
#define	FACE_FLAG_ANIMATE		(1<<9)
#define	FACE_FLAG_OTHER_SPLIT	(1<<10)
#define	FACE_FLAG_METAL			(1<<10)
#define	FACE_FLAG_NON_PLANAR	(1<<11)
#define	FACE_FLAG_TEX2			(1<<12)

#define	FACE_FLAG_FIRE_ESCAPE	(1<<13)
#define FACE_FLAG_PRIM			(1<<14)
#define FACE_FLAG_THUG_JACKET	(1<<15)

//
// The face has been fixed- the page changed by TEXTURE_fix_prim_textures()
//

#define FACE_FLAG_FIXED			(1 << 5)

//
// An edge of a walkable quad that does not lead onto a
// connecting one.
//

#define FACE_FLAG_SLIDE_EDGE	(1 << 1)
#define FACE_FLAG_SLIDE_EDGE_0	(1 << 1)
#define FACE_FLAG_SLIDE_EDGE_1	(1 << 2)
#define FACE_FLAG_SLIDE_EDGE_2	(1 << 3)
#define FACE_FLAG_SLIDE_EDGE_3	(1 << 4)

#define FACE_FLAG_SLIDE_EDGE_ALL	(0xf << 1)
//
// Used only while loading in a SEX object...
//

#define FACE_FLAG_EDGE_VISIBLE   (1 << 13)
#define FACE_FLAG_EDGE_VISIBLE_A (FACE_FLAG_EDGE_VISIBLE << 0)
#define FACE_FLAG_EDGE_VISIBLE_B (FACE_FLAG_EDGE_VISIBLE << 1)
#define FACE_FLAG_EDGE_VISIBLE_C (FACE_FLAG_EDGE_VISIBLE << 2)



//#define	FACE_TYPE_FIRE_ESCAPE	(1<<0)
#define	FACE_TYPE_FENCE			(1<<1)
#define	FACE_TYPE_CABLE			(1<<2)
#define	FACE_TYPE_SKYLIGHT		(1<<3)
#define FACE_TYPE_PRIM			(1<<4)

#define	GET_SORT_LEVEL(f)		(((f)>>2)&7)
#define	OR_SORT_LEVEL(u,f)		u|=((f)<<2)
#define	SET_SORT_LEVEL(u,f)	 	u&=~(7<<2);u|=((f)<<2)


//
// What each prim object is.
//

#define PRIM_OBJ_LAMPPOST			1
#define PRIM_OBJ_TRAFFIC_LIGHT		2
#define PRIM_OBJ_WALK_DONT_WALK		3
#define PRIM_OBJ_PETROL_PUMP		4
#define PRIM_OBJ_DOUBLE_PETROL_PUMP	5
#define PRIM_OBJ_GAS_STATION_SIGN	6
#define PRIM_OBJ_BILLBOARD			7
#define PRIM_OBJ_GAS_STATION_BBOARD	8
#define PRIM_OBJ_HOTEL_SIGN			9
#define PRIM_OBJ_GAS_SIGN			10
#define PRIM_OBJ_GAS_STATION_LIGHT	11
#define PRIM_OBJ_FIRE_ESCAPE1		12
#define PRIM_OBJ_FIRE_ESCAPE2		13
#define PRIM_OBJ_FIRE_ESCAPE3		14
#define PRIM_OBJ_FIRE_ESCAPE4		15
#define PRIM_OBJ_FIRE_ESCAPE5		16
#define PRIM_OBJ_FIRE_ESCAPE6		17
#define PRIM_OBJ_TYRE_SIGN			18
#define PRIM_OBJ_CANOPY1			19
#define PRIM_OBJ_CANOPY2			20
#define PRIM_OBJ_CANOPY3			21
#define PRIM_OBJ_CANOPY4			22
#define PRIM_OBJ_DINER_SIGN			23
#define PRIM_OBJ_MOTEL_SIGN1		25
#define PRIM_OBJ_MOTEL_SIGN2		26
#define PRIM_OBJ_WILDCATVAN_BODY	27
#define PRIM_OBJ_INTERIOR_STAIRS2	28
#define PRIM_OBJ_INTERIOR_STAIRS3	29

//
// Im giving up typing in the right names now....
//

#define PRIM_OBJ_RADIATOR			30
#define PRIM_OBJ_MOTORBIKE			31
#define PRIM_OBJ_BIN            	33
#define PRIM_OBJ_ITEM_KEY			36
#define PRIM_OBJ_LION2				46
#define PRIM_OBJ_BIKE				47
#define PRIM_OBJ_VAN				48
#define PRIM_OBJ_BOAT				49
#define PRIM_OBJ_CAR				50
#define PRIM_OBJ_STROBE				51
#define PRIM_OBJ_DOOR				52
#define PRIM_OBJ_LAMP				54
#define PRIM_OBJ_SIGN				60
#define PRIM_OBJ_TRAFFIC_CONE		67
#define PRIM_OBJ_CHOPPER			74
#define	PRIM_OBJ_CHOPPER_BLADES		75
#define PRIM_OBJ_CAN				76
#define PRIM_OBJ_CANOPY				81
#define PRIM_OBJ_ITEM_TREASURE		84
#define PRIM_OBJ_HOOK				86
#define PRIM_OBJ_VAN_WHEEL			87
#define PRIM_OBJ_VAN_BODY			88
#define PRIM_OBJ_PARK_BENCH			89
#define PRIM_OBJ_JEEP_BODY			90
#define PRIM_OBJ_MEATWAGON_BODY		91
#define PRIM_OBJ_ARMCHAIR			101
#define PRIM_OBJ_COFFEE_TABLE		102
#define PRIM_OBJ_SOFA				105
#define PRIM_OBJ_CAR_BODY			108
#define PRIM_OBJ_WOODEN_TABLE		110
#define PRIM_OBJ_ITEM_AK47			120
#define PRIM_OBJ_ITEM_MAGNUM		121
#define PRIM_OBJ_CHAIR				126
#define PRIM_OBJ_ITEM_SHOTGUN     	127
#define PRIM_OBJ_SEDAN_BODY			131
#define PRIM_OBJ_BALLOON			132
#define PRIM_OBJ_BIKE_STEER			137		// The steering column of the bike.
#define PRIM_OBJ_BIKE_BWHEEL		138
#define PRIM_OBJ_BIKE_FWHEEL		139
#define PRIM_OBJ_BIKE_FRAME			140
#define PRIM_OBJ_BARREL				141
#define PRIM_OBJ_ITEM_HEALTH		142
#define PRIM_OBJ_ITEM_GUN			143
#define PRIM_OBJ_ITEM_KEYCARD		144
#define PRIM_OBJ_ITEM_BOMB      	145
#define PRIM_OBJ_POLICE_BODY		150
#define PRIM_OBJ_TAXI_BODY			155
#define PRIM_OBJ_AMBULANCE_BODY 	159
#define PRIM_OBJ_ITEM_KNIFE     	166
#define PRIM_OBJ_ITEM_EXPLOSIVES	167
#define PRIM_OBJ_ITEM_GRENADE		168
#define PRIM_OBJ_TRIPWIRE			178
#define PRIM_OBJ_MINE				188
#define PRIM_OBJ_SPIKE				189
#define PRIM_OBJ_VALVE				196
#define PRIM_OBJ_THERMODROID		201
#define PRIM_OBJ_ITEM_BASEBALLBAT   205
#define PRIM_OBJ_CAR_WHEEL			220
#define PRIM_OBJ_POLICE_TARGET		147
#define PRIM_OBJ_SWITCH_OFF			216
#define PRIM_OBJ_SWITCH_ON			157
#define PRIM_OBJ_HYDRANT			228
#define PRIM_OBJ_ITEM_FILE          232
#define PRIM_OBJ_ITEM_FLOPPY_DISK   233
#define PRIM_OBJ_ITEM_CROWBAR       234
#define PRIM_OBJ_ITEM_GASMASK       235
#define PRIM_OBJ_ITEM_WRENCH        236
#define PRIM_OBJ_ITEM_VIDEO         237
#define PRIM_OBJ_ITEM_GLOVES        238
#define PRIM_OBJ_ITEM_WEEDKILLER	239
#define PRIM_OBJ_ITEM_AMMO_SHOTGUN  253
#define PRIM_OBJ_ITEM_AMMO_AK47     254
#define PRIM_OBJ_ITEM_AMMO_PISTOL   255
#define PRIM_OBJ_NUMBER				256
#define PRIM_OBJ_WEAPON_GUN			256			   
#define PRIM_OBJ_WEAPON_KNIFE		257			   
#define PRIM_OBJ_WEAPON_SHOTGUN		258			   
#define PRIM_OBJ_WEAPON_BAT			259
#define PRIM_OBJ_WEAPON_AK47		260
#define PRIM_OBJ_WEAPON_GUN_FLASH	261 
#define PRIM_OBJ_WEAPON_SHOTGUN_FLASH 262
#define PRIM_OBJ_WEAPON_AK47_FLASH		263



//
// The "Fun Stuff" day (14th Jan 1999)
//



//
// Returns the collision model to use for each prim.
//

#define PRIM_COLLIDE_BOX		0	// As a rotated bounding box
#define PRIM_COLLIDE_NONE		1	// Just walk through the prim
#define PRIM_COLLIDE_CYLINDER	2	// As a cylinder
#define PRIM_COLLIDE_SMALLBOX	3	// A bounding box smaller than the prim

UBYTE prim_get_collision_model(SLONG prim);

//
// The type of shadow to draw under the prim.
//

#define PRIM_SHADOW_NONE		0
#define PRIM_SHADOW_BOXEDGE		1
#define PRIM_SHADOW_CYLINDER	2
#define PRIM_SHADOW_FOURLEGS	3
#define PRIM_SHADOW_FULLBOX		4

UBYTE prim_get_shadow_type(SLONG prim);

//
// Prim flags.
//

#define PRIM_FLAG_LAMPOST					(1 << 0)
#define PRIM_FLAG_CONTAINS_WALKABLE_FACES	(1 << 1)
#define PRIM_FLAG_GLARE						(1 << 2)
#define PRIM_FLAG_ITEM						(1 << 3)
#define PRIM_FLAG_TREE						(1 << 4)
#define PRIM_FLAG_ENVMAPPED					(1 << 5)
#define PRIM_FLAG_JUST_FLOOR				(1 << 6)
#define PRIM_FLAG_ON_FLOOR					(1 << 7)

#define PRIM_DAMAGE_DAMAGABLE	(1 << 0)
#define PRIM_DAMAGE_EXPLODES	(1 << 1)
#define PRIM_DAMAGE_CRUMPLE		(1 << 2)
#define PRIM_DAMAGE_LEAN		(1 << 3)
#define PRIM_DAMAGE_NOLOS		(1 << 4)	// You can't see through this prim (included in LOS calculation)

#define	SMAT_SHIFT0	(2)
#define	SMAT_SHIFT1	(12)
#define	SMAT_SHIFT2	(22)
#define	SMAT_SHIFTD	(22)
#define	CMAT0_MASK	(0x3ff00000)
#define	CMAT1_MASK	(0x000ffc00)
#define	CMAT2_MASK	(0x000003ff)


// STRUCTS
struct 	CMatrix33
{
	SLONG	M[3];
};

struct 	Matrix33
{
	SLONG	M[3][3];
};

struct 	Matrix31
{
	SLONG	M[3];
};

struct 	SMatrix31
{
	SWORD	M[3];
};

struct	OldPrimPoint
{
	SLONG	X,Y,Z;
};

struct	PrimPoint
{
	SWORD	X,Y,Z;
};



#define	ROOF_SHIFT	3
struct	RoofFace4
{
//	UWORD	TexturePage; //could use the texture on the floor
	SWORD	Y;
	SBYTE	DY[3];
	UBYTE	DrawFlags;
	UBYTE	RX;
	UBYTE	RZ;
	SWORD	Next; //link list of walkables off floor 

};

#ifdef	PSX
#define	WALKABLE	TexturePage

struct	PrimFace4
{
	SWORD	TexturePage;
	UBYTE	AltPal;
	UBYTE	DrawFlags;
	UWORD	Points[4];
	UBYTE	UV[4][2];
	SWORD	ThingIndex;
	UWORD	FaceFlags;
};

struct	PrimFace3
{
	SWORD	TexturePage;
	UBYTE	AltPal;
	UBYTE	DrawFlags;
	UWORD	Points[3];
	UBYTE	UV[3][2];
	SWORD	ThingIndex;
	UWORD	FaceFlags;
};

#else
#define	WALKABLE	Col2
struct	PrimFace3
{
	UBYTE	TexturePage;
	UBYTE	DrawFlags;
	UWORD	Points[3];
	UBYTE	UV[3][2];
	SWORD	Bright[3]; //make into byte
	SWORD	ThingIndex;
	UWORD	Col2;
	UWORD	FaceFlags;
	UBYTE	Type;      // move after bright
	SBYTE	ID;        // delete 
};

struct	PrimFace4
{
	UBYTE	TexturePage;
	UBYTE	DrawFlags;
	UWORD	Points[4];
	UBYTE	UV[4][2];

	union
	{
		SWORD	    Bright[4];	// Used for people.

		struct	// We cant use a LIGHT_Col because of circluar #include problems :-(
		{
			UBYTE red;
			UBYTE green;
			UBYTE blue;

		} col;		// Used for building faces...
	};


	SWORD	ThingIndex;
	SWORD	Col2;
	UWORD	FaceFlags;
	UBYTE	Type;      // move after bright
	SBYTE	ID;
};
#endif

struct	PrimFace4PSX
{
	SWORD	TexturePage;
	UBYTE	AltPal;
	UBYTE	DrawFlags;
	UWORD	Points[4];
	UBYTE	UV[4][2];
	SWORD	ThingIndex;
	UWORD	FaceFlags;
};

struct	PrimFace3PSX
{
	SWORD	TexturePage;
	UBYTE	AltPal;
	UBYTE	DrawFlags;
	UWORD	Points[3];
	UBYTE	UV[3][2];
	SWORD	ThingIndex;
	UWORD	FaceFlags;
};




struct	PrimObject
{
	UWORD	StartPoint;
	UWORD	EndPoint;
	UWORD	StartFace4;
	UWORD	EndFace4;
	SWORD	StartFace3;
	SWORD	EndFace3;

	UBYTE   coltype;
	UBYTE   damage;		// How this prim gets damaged
	UBYTE   shadowtype;
	UBYTE   flag;
};


#if USE_TOMS_ENGINE_PLEASE_BOB

// A flag that lives in the top bit of wTexturePage.
#define TEXTURE_PAGE_FLAG_JACKET			(1<<15)
#define TEXTURE_PAGE_FLAG_OFFSET			(1<<14)
#define TEXTURE_PAGE_FLAG_TINT				(1<<13)
#define TEXTURE_PAGE_FLAG_NOT_TEXTURED		(1<<12)
#define TEXTURE_PAGE_MASK					(0x0fff)

// A prim's material.
// Note that the indices are listed by first the low-quality ones, then the high-quality ones.
// Also, the material uses the number of vertices specified by wNumHiVertices, of which the
// first wNumVertices are used. There are no vertices used by the low-quality mesh that are
// not also used by the high-quality one.
struct PrimObjectMaterial
{
	UWORD	wTexturePage;				// The texture page, maybe with some flags in the top few bits.
	UWORD	wNumListIndices;			// How many list indices there are.
	UWORD	wNumStripIndices;			// How many interrupted strip indices there are.
	UWORD	wNumVertices;				// Number of vertices used.
	// For the low-quality models.
	UWORD	wNumLoListIndices;			// How many list indices there are.
	UWORD	wNumLoStripIndices;			// How many interrupted strip indices there are.
	UWORD	wNumLoVertices;				// Number of vertices used.
};

// My version of an object - an addition to the one above.
// (can't change the size of PrimObject or the loader gets kersplat.

// Object has no alpha polys in it - don't bother checking.
//#define D3DOBJECT_FLAG_NOALPHA				(1<<0)

struct	TomsPrimObject
{
	UWORD	wFlags;						// D3DOBJECT_FLAG_*** flags.
	UWORD	wNumMaterials;				// Number of materials.
	UWORD	wTotalSizeOfObj;			// Number of vertices used by object.
	UBYTE	bLRUQueueNumber;			// Position in the LRU queue.
	UBYTE	bPadding;
	PrimObjectMaterial	*pMaterials;	// Pointer to the materials. Can MemFree this.
	void	*pD3DVertices;				// Pointer to the D3DVERTEX list. DONT MEMFREE THIS
	UWORD	*pwListIndices;				// Pointer to the indices in list form. Can MemFree this.
	UWORD	*pwStripIndices;			// Pointer to the indices in interrupted strip form. DONT MEMFREE THIS
	float	fBoundingSphereRadius;		// Guess!
};
#endif



struct	PrimObjectOld
{
	CBYTE	ObjectName[32];
	UWORD	StartPoint;
	UWORD	EndPoint;
	UWORD	StartFace4;
	UWORD	EndFace4;
	SWORD	StartFace3;
	SWORD	EndFace3;

	UBYTE   coltype;
	UBYTE   damage;		// How this prim gets damaged
	UBYTE   shadowtype;
	UBYTE   flag;

	UWORD	Dummy[4];
};

struct	PrimMultiObject
{
	UWORD	StartObject;
	UWORD	EndObject;
	SWORD	Tween;
	SWORD	Frame;
};


//data
extern	CBYTE	prim_names[2000][32];
extern	struct	SVector			global_res[]; //max points per object?
extern	SLONG	global_flags[];
extern	UWORD	global_bright[];

extern	UWORD	background_prim;

// FUNCTIONS

//extern	void	draw_a_rot_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct	PrimMultiAnim *anim_info);
extern	void	test_poly(void);

extern	SLONG	load_all_prims(CBYTE	*name);
extern	SLONG	save_all_prims(CBYTE	*name);

extern	SLONG	copy_prim_to_end(UWORD prim,UWORD direct,SWORD thing);
extern	void	delete_a_prim(UWORD prim);
extern	void	delete_last_prim(void);
extern	void	calc_normal(SWORD	face,struct SVector *p_normal);
extern	UWORD	apply_ambient_light_to_object(UWORD object,SLONG lnx,SLONG lny,SLONG lnz,UWORD intense);


//
// Initialises all the prim data.
//

void clear_prims(void);

//
// Calculates the FACE_FLAG_SLIDE_EDGE flags in the walkable faces.
//

void calc_slide_edges(void);

//
// Calculates the normals for each prim. These aren't saved
// or loaded in. The length of each normal is 256.
//

void calc_prim_normals(void);

//
// Calculates the PRIM_Infos for all the prims and sets the
// PRIM_FLAG_ENVMAP_OR_TINTED flag for prims that contain environment
// mapped or tinted faces.
//

void calc_prim_info(void);


//
// Puts all the array indices back to what they were at
// some point.
//

void record_prim_status   (void);
void revert_to_prim_status(void);



//
// Returns the info for each prim.
//

typedef struct
{
	SLONG minx;		// The bounding rectangle of the prim.
	SLONG miny;
	SLONG minz;

	SLONG maxx;
	SLONG maxy;
	SLONG maxz;

	SLONG cogx;		// The centre of gravity of the prim.
	SLONG cogy;
	SLONG cogz;

	SLONG radius;	// The bounding sphere about the origin.

} PrimInfo;

PrimInfo *get_prim_info(SLONG prim);


//
// ...
//

void compress_prims(void);

//
// Returns the position of the given point of the prim.
// If the point is -1 then a random point is returned.
//

void get_rotated_point_world_pos(
		SLONG  point,				// -1 => A random point.
		SLONG  prim,
		SLONG  prim_x,
		SLONG  prim_y,
		SLONG  prim_z,
		SLONG  prim_yaw,
		SLONG  prim_pitch,
		SLONG  prim_roll,
		SLONG *px,
		SLONG *py,
		SLONG *pz);

//
// Collides the a movement vector with the bounding-box of the given prim.
// Returns TRUE if a collision occured.
//

SLONG slide_along_prim(
		SLONG  prim,
		SLONG  prim_x,
		SLONG  prim_y,
		SLONG  prim_z,
		SLONG  prim_yaw,
		SLONG  x1, SLONG  y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  radius,
		SLONG  shrink,	// Makes the bounding box of the prim much shorter and smaller.
		SLONG  dont_slide);	// TRUE => Don't move if the vector collides with the prim.

//
// Sets the animation used by the given anim_prim. Anims start
// from 1... anim 0 is not used.
// 

void set_anim_prim_anim(SLONG anim_prim_thing_index, SLONG anim);


//
// Returns the type of the given anim prim.
//

#define ANIM_PRIM_TYPE_NORMAL	0
#define ANIM_PRIM_TYPE_DOOR		1
#define ANIM_PRIM_TYPE_SWITCH	2

SLONG get_anim_prim_type(SLONG anim_prim);


//
// Returns the THING_INDEX of the nearest anim prim to the given point of
// one of the given types within the maximum range.
//

SLONG find_anim_prim(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG range,
		ULONG type_bit_field);	// i.e (1 << ANIM_PRIM_TYPE_DOOR) | (1 << ANIM_PRIM_TYPE_NORMAL)

//
// Toggles the state of the given switch prim.
//

void toggle_anim_prim_switch_state(SLONG anim_prim_thing_index);

//
// Find the bounding box of each anim-prim. The positions and rotations
// are taken from the initial position of the first frame.
//

void find_anim_prim_bboxes(void);


//
// Does nothing to the prim_points and prim_faces, but marks
// each of the prim_objects as not loaded.  You won't be able
// to draw them, but next time you call load_prim_object()
// it will load in the prim.
//

void mark_prim_objects_as_unloaded(void);


//
// Goes through all the prim faces and makes sure that none of them have
// their WMOVE flag set.
//

void clear_all_wmove_flags(void);


//
// Returns TRUE if a face lies along this line. Coordinates are in world coordinates (8-bits per mapsquare)
// The line must lie completely within the fence.
//

SLONG does_fence_lie_along_line(SLONG x1, SLONG z1, SLONG x2, SLONG z2);



#endif





