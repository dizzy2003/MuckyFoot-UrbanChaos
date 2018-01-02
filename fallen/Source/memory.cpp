#include	"game.h"
#include	"ob.h"
#include	"memory.h"
#include	"fc.h"
#include	"wmove.h"
#include	"supermap.h"
#include	"night.h"
#include	"barrel.h"
#include	"bike.h"
#include	"eway.h"
#include	"pap.h"
#include	"ob.h"
#include	"mav.h"
#include	"road.h"
#include	"balloon.h"
#include	"tracks.h"
#include	"ware.h"
#include	"trip.h"
#include	"psystem.h"
#include	"env.h"
#include	"bat.h"
#include	"door.h"
#include	"spark.h"
#ifndef PSX
#include	"playcuts.h"
#endif
#include	"eway.h"
#include	"statedef.h"
#ifndef PSX
#include	"poly.h"
#include	"sound.h"
#endif

#ifdef PSX
#include <libsn.h>
#include <libcd.h>
#include <ctype.h>
#include "c:\fallen\psxlib\headers\myheap.h"
#endif

#ifdef PSX
#ifdef VERSION_PAL

#if	!defined(MIKE)&&!defined(VERSION_DEMO)
#include "libcrypt.h"
UBYTE do_decrypt[]={0,0,0,0,0,0,0,1,0,1,
					1,1,1,0,0,0,0,1,0,0,
					1,0,0,1,0,0,0,0,0,0,
					0,0,1,1,1,0};
#endif
#endif
#endif

#ifndef	PSX
#define	NEW_LEVELS	1
#endif

//#define	NEW_LEVELS	1



#ifdef TARGET_DC
#include "target.h"
#endif


extern	ULONG	level_index;
/*

  //
  // e3.ucm 16 aug 99
  //	 

 MOSTspecialS 49 
 MOSTmeshS 138 
 MOSTtweenS 66 
 MOSTCARS 29 
 MOSTPEOPLE 66 

 SAVE INGAME 
 store data 4 
 Pap_Hi -> 98304   tot 98304 (16384)
 Pap_Lo -> 8192   tot 106496 (1024)
 net_peep -> 40   tot 106536 (10)
 net_plyr -> 40   tot 106576 (10)
 f-links -> 10666   tot 117242 (5333/32000)
 dbuildings -> 4728   tot 121970 (197/1024)
 dfacets -> 71968   tot 193938 (2768/16384)
 dwalkables -> 5874   tot 199812 (267/2048)
 dstyles -> 8746   tot 208558 (4373/10000)
 dstoreys -> 3192   tot 211750 (532/10000)
 paintmem -> 2379   tot 214129 (2379/64000)
 insideStoreys -> 22   tot 214151 (1/2000)
 insideStairs -> 10   tot 214161 (1/8000)
 insideblock -> 1   tot 214162 (1/64000)
 roof bounds -> 6   tot 214168 (1/2000)
 prim_points -> 64170   tot 278338 (10695/65000)
 prim face 4 psx = 98568 was 139638
 prim_faces4 -> 98568   tot 376906 (4107/32760)
 prim face3 psx = 131460 was 184044
 prim_faces3 -> 131460   tot 508366 (6573/32000)
 prim_objects -> 8928   tot 517294 (558/2000)
 prim_Mobjects -> 136   tot 517430 (17/100)
 ob_ob -> 5776   tot 523206 (722/2048)
 ob_ mapwho -> 2048   tot 525254 (1024)
 EWAY_mess -> 280   tot 525534 (70/128)
 EWAY_mess buf -> 4651   tot 530185 (4651/8192)
 EWAY_timer -> 0   tot 530185 (32)
 EWAY_cond -> 114   tot 530299 (19/128)
 EWAY_way -> 10272   tot 540571 (321/512)
 EWAY_edef -> 900   tot 541471 (75/150)
 EWAY_counter -> 10   tot 541481 (10)
 vehicles -> 6120   tot 547601 (34)
 people -> 11008   tot 558609 (86)
 animals -> 120   tot 558729 (6)
 choppers -> 336   tot 559065 (4)
 pyro -> 2688   tot 561753 (32)
 players -> 272   tot 562025 (2)
 projectiles -> 80   tot 562105 (10)
 special -> 1980   tot 564085 (99)
 switches -> 56   tot 564141 (2)
 bats -> 320   tot 564461 (10)
 thing -> 23460   tot 587921 (391)
 drawtween -> 4472   tot 592393 (86)
 drawmesh -> 1680   tot 594073 (168)
 barrelsphere -> 2240   tot 596313 (80)
 barrels -> 354   tot 596667 (59/300)
 plat -> 12   tot 596679 (1/32)
 wmove -> 4608   tot 601287 (192)
 mav_opt -> 3220   tot 604507 (805/1024)
 mav_nav -> 32768   tot 637275 (16384)
 road_noads -> 150   tot 637425 (25/256)
 balloons -> 104   tot 637529 (1/32)
 tracks -> 1600   tot 639129 (50)
 roofface4 -> 31960   tot 671089 (3196/10000)
 fastnav -> 2048   tot 673137 (2048)
 night_slight -> 2048   tot 675185 (256/256)
 night_smap -> 2048   tot 677233 (1024)
 night_dlight -> 768   tot 678001 (64)
 WARE_ware -> 32   tot 678033 (1/32)
 WARE_nav -> 0   tot 678033 (0/4096)
 WARE_height -> 0   tot 678033 (0/8192)
 WARE_rooftex -> 0   tot 678033 (0/4096)
 Trip_Wire -> 14   tot 678047 (1/32)
 Road_edges -> 0   tot 678047 (0/8)
 Thing_heads -> 38   tot 678085 (19)
 psx_remap -> 256   tot 678341 (128)
 psx_tex_xy -> 2000   tot 680341 (1000)
 map_beacon -> 512   tot 680853 (32)
 cutscene_data -> 0   tot 680853 (0/20)
 cutscene_trks -> 0   tot 680853 (0/300)
 cutscene_pkts -> 0   tot 680853 (0/38400)
 cutscene_text -> 0   tot 680853 (0/4096)
 darci normal -> 482   tot 681335 (241/1200)

*/
extern	void BAT_normal(Thing *p_thing);

#ifndef PSX
#define M_(x) x
#else
#ifndef FS_ISO9660
#define M_(x) x
#else
#define M_(x) NULL
#endif
#endif

/*

extern	SLONG FC_x;
extern	SLONG FC_y;
extern	SLONG FC_z;
extern	SLONG FC_want_x;
extern	SLONG FC_want_y;
extern	SLONG FC_want_z;
extern	SLONG FC_dx;
extern	SLONG FC_dy;
extern	SLONG FC_dz;
extern	SLONG FC_yaw;
extern	SLONG FC_pitch;
extern	SLONG FC_roll;
extern	SLONG FC_want_yaw;
extern	SLONG FC_want_pitch;
extern	SLONG FC_want_roll;
extern	SLONG FC_lens ;	// Initialise this here because of the game editor!
extern	SLONG FC_toonear;
extern	SLONG FC_rotate;
extern	SLONG FC_nobehind;
extern	SLONG FC_lookabove;
extern	UBYTE FC_shake;
*/

extern	ULONG        NIGHT_amb_d3d_colour;
extern	ULONG        NIGHT_amb_d3d_specular;
extern	SLONG        NIGHT_amb_red;
extern	SLONG        NIGHT_amb_green;
extern	SLONG        NIGHT_amb_blue;
extern	SLONG        NIGHT_amb_norm_x;
extern	SLONG        NIGHT_amb_norm_y;
extern	SLONG	     NIGHT_amb_norm_z;

extern	UBYTE        NIGHT_dlight_free;
extern	UBYTE        NIGHT_dlight_used;
extern	ULONG        NIGHT_flag;
extern	UBYTE        NIGHT_lampost_radius;
extern	SBYTE        NIGHT_lampost_red;
extern	SBYTE        NIGHT_lampost_green;
extern	SBYTE        NIGHT_lampost_blue;
extern	NIGHT_Colour NIGHT_sky_colour;

extern UWORD EWAY_fake_wander_text_normal_index;
extern UWORD EWAY_fake_wander_text_normal_number;
extern UWORD EWAY_fake_wander_text_guilty_index;
extern UWORD EWAY_fake_wander_text_guilty_number;
extern UWORD EWAY_fake_wander_text_annoyed_index;
extern UWORD EWAY_fake_wander_text_annoyed_number;

extern UBYTE semtex;
extern UBYTE estate;

extern	UWORD	*thing_class_head;
extern  SWORD	world_type;

extern	void PLAT_process(Thing *p_thing);
extern	SLONG	TEXTURE_set;
//extern	Thing *CAM_focus;

void	convert_index_to_pointers(void);

SLONG load_anim_prim_object(SLONG prim);

MAP_Beacon *MAP_beacon; //[MAP_MAX_BEACONS];

PSX_TEX *psx_textures_xy; //[200][5];

#ifdef TARGET_DC
// Don't allocate/free each time, just allocate it statically.
#define LAZY_LOADING_MEMORY_ON_DC_PLEASE_BOB
#endif

void	*mem_all=0;
ULONG	mem_all_size=0;

UWORD	*psx_remap;

//
// supermap stuff to do with ingame facets facet textures inside buildings and walkable rooftops
//

SWORD	*facet_links; //[MAX_FACET_LINK];

struct DBuilding	*dbuildings;//[MAX_DBUILDINGS];
struct DFacet		*dfacets;   //[MAX_DFACETS	 ];
struct	DWalkable	*dwalkables;//[MAX_DWALKABLES];
SWORD				*dstyles;   //[MAX_DSTYLES	 ];
struct	DStorey		*dstoreys;  //[MAX_DSTOREYS];

UBYTE	*paint_mem; //[MAX_PAINTMEM];


//
// for compressed anims
//

struct	PrimPoint	*anim_mids;//[256];
ULONG	next_anim_mids=0;

//
// from inside2
//

struct	InsideStorey	*inside_storeys;//[MAX_INSIDE_RECT];
struct	Staircase		*inside_stairs;//[MAX_INSIDE_STAIRS];
UBYTE	*inside_block;//[MAX_INSIDE_MEM];
UBYTE	inside_tex[64][16];

#ifdef PSX
UWORD	next_inside_storey=1;
UWORD	next_inside_stair=1;
SLONG	next_inside_block=1;
#endif


//
// from building.cpp
//

struct	BoundBox	*roof_bounds;//[MAX_ROOF_BOUND];
struct	PrimPoint *prim_points;//[MAX_PRIM_POINTS];
struct	PrimFace4 *prim_faces4;//[MAX_PRIM_FACES4];
struct	PrimFace3 *prim_faces3;//[MAX_PRIM_FACES3];
struct	PrimObject	*prim_objects;//[MAX_PRIM_OBJECTS];
struct	PrimMultiObject	*prim_multi_objects;//[MAX_PRIM_MOBJECTS];
PrimNormal *prim_normal;//[MAX_PRIM_POINTS];


UWORD	next_roof_face4=1;
struct	RoofFace4	*roof_faces4;



extern	SLONG EWAY_time_accurate;	// 1600 ticks per second
extern	SLONG EWAY_time;			// 100  ticks per second
extern	SLONG EWAY_tick;			// The amount of time since the last process waypoints: (100 ticks per sec.)



//
// The cut-scene camera.
//
			   
extern	SLONG  EWAY_cam_active;
extern	SLONG  EWAY_cam_x;		// Big coordinates...
extern	SLONG  EWAY_cam_y;
extern	SLONG  EWAY_cam_z;
extern	SLONG  EWAY_cam_dx;
extern	SLONG  EWAY_cam_dy;
extern	SLONG  EWAY_cam_dz;
extern	SLONG  EWAY_cam_yaw;
extern	SLONG  EWAY_cam_pitch;
extern	SLONG  EWAY_cam_waypoint;
extern	SLONG  EWAY_cam_target;
extern	SLONG  EWAY_cam_delay;
extern	SLONG  EWAY_cam_speed;
extern	SLONG  EWAY_cam_freeze;	// Stop the player moving.
extern	UBYTE *EWAY_counter;
extern  SLONG  EWAY_cam_cant_interrupt;

extern SLONG EWAY_cam_active;
extern SLONG EWAY_cam_goinactive;
extern SLONG EWAY_cam_x;		// Big coordinates...
extern SLONG EWAY_cam_y;
extern SLONG EWAY_cam_z;
extern SLONG EWAY_cam_dx;
extern SLONG EWAY_cam_dy;
extern SLONG EWAY_cam_dz;
extern SLONG EWAY_cam_yaw;
extern SLONG EWAY_cam_pitch;
extern SLONG EWAY_cam_want_yaw;
extern SLONG EWAY_cam_want_pitch;
extern SLONG EWAY_cam_waypoint;
extern SLONG EWAY_cam_target;
extern SLONG EWAY_cam_delay;
extern SLONG EWAY_cam_speed;
extern SLONG EWAY_cam_freeze;	// Stop the player moving.
extern SLONG EWAY_cam_cant_interrupt;
extern UWORD EWAY_cam_thing;
extern SLONG EWAY_cam_targx;
extern SLONG EWAY_cam_targy;
extern SLONG EWAY_cam_targz;
extern SLONG EWAY_cam_lens;	// 16-bit fixed point
extern SLONG EWAY_cam_warehouse;
extern SLONG EWAY_cam_lock;
extern SLONG EWAY_cam_last_yaw;
extern SLONG EWAY_cam_last_x;
extern SLONG EWAY_cam_last_y;
extern SLONG EWAY_cam_last_z;
extern SLONG EWAY_cam_skip;
extern SLONG EWAY_cam_last_dyaw;


UWORD	*darci_normal;
UWORD	darci_normal_count=0;

void	release_memory(void)
{

}



#define MEM_DYNAMIC 1
#define MEM_STATIC  2

struct MemTable save_table[]=
{

	{M_("Pap_Hi")		,(void**)&PAP_hi				,MEM_STATIC, 0							,0							,PAP_SIZE_HI*PAP_SIZE_HI	,sizeof(PAP_Hi)					,0}, //0
	{M_("Pap_Lo")		,(void**)&PAP_lo				,MEM_STATIC, 0							,0							,PAP_SIZE_LO*PAP_SIZE_LO	,sizeof(PAP_Lo)					,0}, //1
	{M_("net_peep")		,(void**)&NETPERSON				,MEM_STATIC, 0							,0							,10							,sizeof(Thing*)					,0}, //2
	{M_("net_plyr")		,(void**)&NETPLAYERS			,MEM_STATIC, 0							,0							,10							,sizeof(Thing*)					,0}, //3
	{M_("f-links")		,(void**)&facet_links			,MEM_DYNAMIC,0							,(UWORD*)&next_facet_link	,MAX_FACET_LINK				,sizeof(SWORD)					,0}, //4
	{M_("dbuildings")	,(void**)&dbuildings			,MEM_DYNAMIC,&next_dbuilding			,0							,MAX_DBUILDINGS				,sizeof(struct DBuilding)		,0}, //5
	{M_("dfacets")		,(void**)&dfacets				,MEM_DYNAMIC,&next_dfacet				,0							,MAX_DFACETS				,sizeof(struct DFacet)			,0}, //6
	{M_("dwalkables")	,(void**)&dwalkables			,MEM_DYNAMIC,&next_dwalkable			,0							,MAX_DWALKABLES				,sizeof(struct DWalkable)		,0}, //7
	{M_("dstyles")		,(void**)&dstyles				,MEM_DYNAMIC,&next_dstyle				,0							,MAX_DSTYLES				,sizeof(SWORD)					,0}, //8
	{M_("dstoreys")		,(void**)&dstoreys				,MEM_DYNAMIC,0							,(UWORD*)&next_dstorey		,MAX_DSTOREYS				,sizeof(struct DStorey )		,0}, //9
	{M_("paintmem")		,(void**)&paint_mem				,MEM_DYNAMIC,0							,(UWORD*)&next_paint_mem	,MAX_PAINTMEM				,sizeof(UBYTE)					,0}, //10
	{M_("insideStoreys"),(void**)&inside_storeys		,MEM_DYNAMIC,0							,(UWORD*)&next_inside_storey,MAX_INSIDE_RECT			,sizeof(struct InsideStorey)	,0}, //11
	{M_("insideStairs")	,(void**)&inside_stairs			,MEM_DYNAMIC,0							,&next_inside_stair			,MAX_INSIDE_STAIRS			,sizeof(struct Staircase)		,0}, //12
	{M_("insideblock")	,(void**)&inside_block			,MEM_DYNAMIC,&next_inside_block			,0							,MAX_INSIDE_MEM				,sizeof(UBYTE)					,0}, //13
	{M_("roof bounds")	,(void**)&roof_bounds			,MEM_DYNAMIC,0							,&next_roof_bound			,MAX_ROOF_BOUND				,sizeof(struct	BoundBox)		,0}, //14
	{M_("prim_points")	,(void**)&prim_points			,MEM_DYNAMIC,0							,&next_prim_point			,RMAX_PRIM_POINTS			,sizeof(struct PrimPoint)		,256}, //15
	{M_("prim_faces4")	,(void**)&prim_faces4			,MEM_DYNAMIC,0							,&next_prim_face4			,RMAX_PRIM_FACES4			,sizeof(struct PrimFace4)		,64}, //16
	{M_("prim_faces3")	,(void**)&prim_faces3			,MEM_DYNAMIC,0							,&next_prim_face3			,MAX_PRIM_FACES3			,sizeof(struct PrimFace3)		,0}, //17
	{M_("prim_objects")	,(void**)&prim_objects			,MEM_DYNAMIC,0							,&next_prim_object			,MAX_PRIM_OBJECTS			,sizeof(struct PrimObject)		,0}, //18
	{M_("prim_Mobjects"),(void**)&prim_multi_objects	,MEM_DYNAMIC,0							,&next_prim_multi_object	,MAX_PRIM_MOBJECTS			,sizeof(struct PrimMultiObject)	,0}, //19

/*
#ifdef TEST_DC
	{M_("prim normal")	,(void**)&prim_normal			,MEM_DYNAMIC,0							,&next_prim_point			,MAX_PRIM_POINTS			,sizeof(PrimNormal)				},
#endif
*/
																																										
	{M_("ob_ob")		,(void**)&OB_ob					,MEM_DYNAMIC,&OB_ob_upto				,0							,OB_MAX_OBS					,sizeof(OB_Ob)					,0}, //20				
	{M_("ob_ mapwho")	,(void**)&OB_mapwho				,MEM_STATIC, 0							,0							,OB_SIZE*OB_SIZE			,sizeof(OB_Mapwho)				,0}, //21
	{M_("EWAY_mess")	,(void**)&EWAY_mess				,MEM_DYNAMIC,&EWAY_mess_upto			,0							,EWAY_MAX_MESSES			,sizeof(CBYTE*)					,0}, //22
	{M_("EWAY_mess buf"),(void**)&EWAY_mess_buffer		,MEM_DYNAMIC,&EWAY_mess_buffer_upto		,0							,EWAY_MESS_BUFFER_SIZE		,sizeof(CBYTE)					,0}, //23
#ifdef	NEW_LEVELS
	{M_("EWAY_timer")	,(void**)&EWAY_timer			,MEM_STATIC,0							,0							,EWAY_MAX_TIMERS			,sizeof(UWORD)					,0}, //24
#else
	{M_("EWAY_timer")	,(void**)&EWAY_timer			,MEM_DYNAMIC,0							,0							,EWAY_MAX_TIMERS			,sizeof(UWORD)					,0}, //24
#endif
	{M_("EWAY_cond")	,(void**)&EWAY_cond				,MEM_DYNAMIC,&EWAY_cond_upto			,0							,EWAY_MAX_CONDS				,sizeof(EWAY_Cond)				,0}, //25
	{M_("EWAY_way")		,(void**)&EWAY_way				,MEM_DYNAMIC,&EWAY_way_upto				,0							,EWAY_MAX_WAYS				,sizeof(EWAY_Way)				,0}, //26
	{M_("EWAY_edef")	,(void**)&EWAY_edef				,MEM_DYNAMIC,&EWAY_edef_upto			,0							,EWAY_MAX_EDEFS				,sizeof(EWAY_Edef)				,0}, //27
	{M_("EWAY_counter") ,(void**)&EWAY_counter			,MEM_STATIC	,0							,0							,EWAY_MAX_COUNTERS			,sizeof(UBYTE)					,0}, //28
																																										
	{M_("vehicles")		,(void**)&VEHICLES				,MEM_STATIC, 0							,0							,RMAX_VEHICLES				,sizeof(Vehicle)				,32}, //29
	{M_("people")		,(void**)&PEOPLE				,MEM_STATIC, 0							,0							,RMAX_PEOPLE				,sizeof(Person)					,128}, //30
	{M_("animals")		,(void**)&ANIMALS				,MEM_STATIC, 0							,0							,MAX_ANIMALS				,sizeof(Animal)					,0}, //31
	{M_("choppers")		,(void**)&CHOPPERS				,MEM_STATIC, 0							,0							,MAX_CHOPPERS				,sizeof(Chopper)				,0}, //32
	{M_("pyro")			,(void**)&PYROS					,MEM_STATIC, 0							,0							,MAX_PYROS					,sizeof(Pyro)					,0}, //33
	{M_("players")		,(void**)&PLAYERS				,MEM_STATIC, 0							,0							,MAX_PLAYERS				,sizeof(Player)					,0}, //34
	{M_("projectiles")	,(void**)&PROJECTILES			,MEM_STATIC, 0							,0							,MAX_PROJECTILES			,sizeof(Projectile)				,0}, //35
	{M_("special")		,(void**)&SPECIALS				,MEM_STATIC, 0							,0							,RMAX_SPECIALS				,sizeof(Special)				,128}, //36
	{M_("switches")		,(void**)&SWITCHES				,MEM_STATIC, 0							,0							,MAX_SWITCHES				,sizeof(Switch)					,0}, //37
	{M_("bats")			,(void**)&BATS					,MEM_STATIC, 0							,0							,RBAT_MAX_BATS				,sizeof(Bat)					,32}, //38
	{M_("thing")		,(void**)&THINGS				,MEM_STATIC, 0							,0							,MAX_THINGS					,sizeof(Thing)					,0}, //39
	{M_("drawtween")	,(void**)&DRAW_TWEENS			,MEM_STATIC, 0							,0							,RMAX_DRAW_TWEENS			,sizeof(DrawTween)				,128}, //40
	{M_("drawmesh")		,(void**)&DRAW_MESHES			,MEM_STATIC, 0							,0							,RMAX_DRAW_MESHES			,sizeof(DrawMesh)				,128}, //41
	#ifdef BIKE
	{M_("bike")			,(void**)&BIKE_bike				,MEM_STATIC, 0							,0							,BIKE_MAX_BIKES				,sizeof(BIKE_Bike)				,0}, //42
	#endif
	{M_("barrelsphere")	,(void**)&BARREL_sphere			,MEM_STATIC, 0							,0							,BARREL_MAX_SPHERES			,sizeof(BARREL_Sphere)			,0}, //43
	{M_("barrels")		,(void**)&BARREL_barrel			,MEM_DYNAMIC,&BARREL_barrel_upto		,0							,BARREL_MAX_BARRELS			,sizeof(Barrel)					,0}, //44			
	{M_("plat")			,(void**)&PLAT_plat				,MEM_DYNAMIC,&PLAT_plat_upto			,0							,RPLAT_MAX_PLATS			,sizeof(Plat)					,2}, //45			
	{M_("wmove")		,(void**)&WMOVE_face			,MEM_DYNAMIC,&WMOVE_face_upto			,0							,RWMOVE_MAX_FACES			,sizeof(WMOVE_Face)				,64}, //46			
	{M_("mav_opt")		,(void**)&MAV_opt				,MEM_DYNAMIC,&MAV_opt_upto				,0							,MAV_MAX_OPTS				,sizeof(MAV_Opt)				,0}, //47			

	//{M_("mav height")	,(void**)&MAV_height			,MEM_STATIC, 0							,0							,PAP_SIZE_HI*PAP_SIZE_HI	,sizeof(SBYTE)					},			

	{M_("mav_nav")		,(void**)&MAV_nav				,MEM_STATIC, 0							,0							,PAP_SIZE_HI*PAP_SIZE_HI	,sizeof(UWORD)					,0}, //48			
	{M_("road_noads")	,(void**)&ROAD_node				,MEM_DYNAMIC,&ROAD_node_upto			,0							,ROAD_MAX_NODES				,sizeof(ROAD_Node)				,0}, //49			
	{M_("balloons")		,(void**)&BALLOON_balloon		,MEM_DYNAMIC,&BALLOON_balloon_upto		,0							,BALLOON_MAX_BALLOONS		,sizeof(BALLOON_Balloon)		,0}, //50			
	{M_("tracks")		,(void**)&tracks				,MEM_STATIC, 0							,0							,TRACK_BUFFER_LENGTH		,sizeof(Track)					,0}, //51			
	{M_("roofface4")	,(void**)&roof_faces4			,MEM_DYNAMIC,0							,&next_roof_face4			,MAX_ROOF_FACE4				,sizeof(struct RoofFace4)		,0}, //52			
	{M_("fastnav")		,(void**)&COLLIDE_fastnav		,MEM_STATIC, 0							,0							,PAP_SIZE_HI*PAP_SIZE_HI>>3 ,sizeof(UBYTE)					,0}, //53
	{M_("night_slight")	,(void**)&NIGHT_slight			,MEM_DYNAMIC,&NIGHT_slight_upto			,0							,NIGHT_MAX_SLIGHTS			,sizeof(NIGHT_Slight)			,0}, //54
	{M_("night_smap")	,(void**)&NIGHT_smap			,MEM_STATIC	,0							,0							,PAP_SIZE_LO*PAP_SIZE_LO	,sizeof(NIGHT_Smap)				,0}, //55
	{M_("night_dlight")	,(void**)&NIGHT_dlight			,MEM_STATIC	,0							,0							,NIGHT_MAX_DLIGHTS			,sizeof(NIGHT_Dlight)			,0}, //56
	{M_("WARE_ware")	,(void**)&WARE_ware				,MEM_DYNAMIC,0							,&WARE_ware_upto			,WARE_MAX_WARES				,sizeof(WARE_Ware)				,0}, //57
	{M_("WARE_nav")		,(void**)&WARE_nav				,MEM_DYNAMIC,0							,&WARE_nav_upto				,WARE_MAX_NAVS				,sizeof(UWORD)					,0}, //58
	{M_("WARE_height")	,(void**)&WARE_height			,MEM_DYNAMIC,0							,&WARE_height_upto			,WARE_MAX_HEIGHTS			,sizeof(SBYTE)					,0}, //59
	{M_("WARE_rooftex") ,(void**)&WARE_rooftex          ,MEM_DYNAMIC,0                          ,&WARE_rooftex_upto         ,WARE_MAX_ROOFTEXES         ,sizeof(UWORD)                  ,0}, //60
	{M_("Trip_Wire")	,(void**)&TRIP_wire				,MEM_DYNAMIC,&TRIP_wire_upto			,0							,TRIP_MAX_WIRES				,sizeof(TRIP_Wire)				,0}, //61
	{M_("Road_edges")	,(void**)&ROAD_edge				,MEM_DYNAMIC,0							,&ROAD_edge_upto			,ROAD_MAX_EDGES				,sizeof(UBYTE)					,0}, //62
	{M_("Thing_heads")	,(void**)&thing_class_head		,MEM_STATIC ,0							,0							,CLASS_END					,sizeof(UWORD)					,0}, //63
	{M_("psx_remap")	,(void**)&psx_remap				,MEM_STATIC ,0							,0							,128						,sizeof(UWORD)					,0}, //64
	{M_("psx_tex_xy")	,(void**)&psx_textures_xy		,MEM_STATIC ,0							,0							,200*5						,sizeof(UWORD)					,0}, //65
	{M_("map_beacon")	,(void**)&MAP_beacon			,MEM_STATIC ,0							,0							,MAP_MAX_BEACONS			,sizeof(MAP_Beacon)				,0}, //66
//	{"anim_mids"	,(void**)&anim_mids				,MEM_STATIC ,0							,&next_anim_mids			,256						,sizeof(PrimPoint)				},
// cutscene memory
#ifndef BUILD_PSX
	{M_("cutscene_data"),(void**)&PLAYCUTS_cutscenes	,MEM_DYNAMIC,0							,&PLAYCUTS_cutscene_ctr		,MAX_CUTSCENES				,sizeof(CPData)					,0},
	{M_("cutscene_trks"),(void**)&PLAYCUTS_tracks   	,MEM_DYNAMIC,0							,&PLAYCUTS_track_ctr    	,MAX_CUTSCENE_TRACKS		,sizeof(CPChannel)				,0},
	{M_("cutscene_pkts"),(void**)&PLAYCUTS_packets		,MEM_DYNAMIC,0							,&PLAYCUTS_packet_ctr		,MAX_CUTSCENE_PACKETS		,sizeof(CPPacket)				,0},
	{M_("cutscene_text"),(void**)&PLAYCUTS_text_data	,MEM_DYNAMIC,0							,&PLAYCUTS_text_ctr			,MAX_CUTSCENE_TEXT			,sizeof(CBYTE)					,0},
#endif
	{M_("darci normal") ,(void**)&darci_normal			,MEM_DYNAMIC,0							,&darci_normal_count		,12000						,sizeof(UWORD)					,0},
	{M_("prim info")	,(void**)&prim_info				,MEM_STATIC ,0							,0							,256						,sizeof(PrimInfo)				,0},

	{M_("Doors-gates")	,(void**)&DOOR_door				,MEM_STATIC	,0							,0							,DOOR_MAX_DOORS				,sizeof(DOOR_Door)				,0},

//
// new ones added by MikeD for footstep surfaces
//
	{M_("soundfxmap")		,(void**)&SOUND_FXMapping	,MEM_STATIC, 0							,0							,1024						,sizeof(UBYTE)					,0}, //48			
	{M_("soundfxgroup")		,(void**)&SOUND_FXGroups	,MEM_STATIC, 0							,0							,128*2						,sizeof(UWORD)					,0}, //48			


	{0,0,0,0,0,0,0}
//	{""		,(void**)		,1,0					,&		,MAX_PAINTMEM			,sizeof(struct )	},


};


void	init_memory(void)
{
	SLONG	c0=0;
	SLONG	mem_size,mem_cumlative=0;
	struct	MemTable	*p_tab;
	UBYTE	*p_all;
	SLONG	temp;



	// FIXME FUDGE!
#ifdef TARGET_DC
#endif



#ifndef PSX
#ifndef TARGET_DC
	save_table[SAVE_TABLE_PEOPLE].Maximum=RMAX_PEOPLE;
	save_table[SAVE_TABLE_VEHICLE].Maximum=RMAX_VEHICLES;
	save_table[SAVE_TABLE_SPECIAL].Maximum=RMAX_SPECIALS;
	save_table[SAVE_TABLE_BAT].Maximum=RBAT_MAX_BATS;
	save_table[SAVE_TABLE_DTWEEN].Maximum=RMAX_DRAW_TWEENS;
	save_table[SAVE_TABLE_DMESH].Maximum=RMAX_DRAW_MESHES;

extern	UBYTE	music_max_gain;

//	 temp= ENV_get_value_number("music_vol", 128, "Audio");
//	 SATURATE(temp,0,255);
//	 music_max_gain=temp;


extern	SLONG	save_psx;
extern	SLONG	build_psx;

	if((save_psx=ENV_get_value_number("psx", 0, "Secret")))
	{
//		save_psx=1;
		build_psx=1;
	}
#endif
#endif
	while(save_table[c0].Point)
	{
		void*	ptr;
		p_tab=&save_table[c0];
		mem_size=p_tab->StructSize*p_tab->Maximum;
		mem_cumlative+=mem_size;
		DebugText(" %s = %d  cuml %d \n",p_tab->Name,mem_size,mem_cumlative);


		c0++;
	}

	c0=0;
	mem_cumlative+=1024;

	if(mem_all)
	{
		MemFree(mem_all);
		mem_all = NULL;
	}


#ifndef LAZY_LOADING_MEMORY_ON_DC_PLEASE_BOB
	// Not used on DC.

	mem_all=MemAlloc(mem_cumlative);
	ASSERT(mem_all);
	mem_all_size=mem_cumlative;


	p_all=(UBYTE*)mem_all;

	while(save_table[c0].Point)
	{
		void*	ptr;
		p_tab=&save_table[c0];
		mem_size=p_tab->StructSize*p_tab->Maximum;

		mem_size+=3;
		mem_size&=0xfffffffc;
		*p_tab->Point=p_all;
		p_all+=mem_size;
		c0++;
	}

#endif


	anim_mids=(PrimPoint*)MemAlloc(256*sizeof(PrimPoint));
// because furniture isnt saved at the moment, so isnt in the table
	the_game.Furnitures=(Furniture*)MemAlloc(sizeof(Furniture)*MAX_FURNITURE);
//	{"prim normal"	,(void**)&prim_normal			,1,0							,&next_prim_point			,MAX_PRIM_POINTS		,sizeof(PrimNormal)				},
	prim_normal=(PrimNormal*)MemAlloc(sizeof(PrimNormal)*MAX_PRIM_POINTS);



}
#ifndef	PSX
void	set_darci_normals(void)
{
	SLONG	count_vertex;
	SLONG	c0,c1,index;
	SLONG	sp,ep;
	SLONG	last_point=0,first_point=0x7fffffff;
	SLONG	start_object;
	for(c0=1;c0<darci_normal_count;c0++)
	{
		SLONG	nx,ny,nz,c;

		nx=prim_normal[c0].X;
		ny=prim_normal[c0].Y;
		nz=prim_normal[c0].Z;
		SATURATE(nx,-256,255);
		SATURATE(ny,-256,255);
		SATURATE(nz,-256,255);

		nx>>=4;
		nx+=16;
		ASSERT(nx>=0 &&nx<=31);

		ny>>=4;
		ny+=16;
		ASSERT(ny>=0 && ny<=31);

		nz>>=4;
		nz+=16;
		ASSERT(nz>=0 && nz<=31);

		nx&=31;
		ny&=31;
		nz&=31;

		c=nx<<10;
		c|=ny<<5;
		c|=nz;
		darci_normal[c0]=c;

	}
	return;
/*
	start_object = prim_multi_objects[game_chunk[0].MultiObject[0]].StartObject;
	for(c0=0;c0<15;c0++)
	{
		index = start_object+game_chunk[0].PeopleTypes[0].BodyPart[c0];

		sp=prim_objects[index].StartPoint;
		ep=prim_objects[index].EndPoint;

		if(sp<first_point)
			first_point=sp;
		if(ep>last_point)
			last_point=ep;


	}
	ASSERT(last_point-first_point>0 &&last_point-first_point<1000);

//	darci_normal=(UWORD*)MemAlloc((last_point-first_point+2)*2);
	darci_normal[0]=first_point-1;

	for(c0=0;c0<15;c0++)
	{
		index = start_object+game_chunk[0].PeopleTypes[0].BodyPart[c0];
		sp=prim_objects[index].StartPoint;
		ep=prim_objects[index].EndPoint;
		for(c1=sp;c1<ep;c1++)
		{
			SLONG	nx,ny,nz,c;

			nx=prim_normal[c1].X;
			ny=prim_normal[c1].Y;
			nz=prim_normal[c1].Z;
			SATURATE(nx,-256,255);
			SATURATE(ny,-256,255);
			SATURATE(nz,-256,255);
//			>>1 =-127 ->127
//			>>1 =-63 -> 63
//			>>1 =-31 -> 31
//			>>1 =-15 -> 15
			nx>>=4;
			nx+=16;
			ASSERT(nx>=0 &&nx<=31);

			ny>>=4;
			ny+=16;
			ASSERT(ny>=0 && ny<=31);

			nz>>=4;
			nz+=16;
			ASSERT(nz>=0 && nz<=31);

			nx&=31;
			ny&=31;
			nz&=31;

			c=nx<<10;
			c|=ny<<5;
			c|=nz;
			darci_normal[c1-first_point+1]=c;


		}
	}
	darci_normal_count=last_point-first_point+1;
*/

}
#endif

#ifndef	PSX
void	convert_drawtype_to_index(Thing *p_thing,SLONG meshtype)
{
	switch(meshtype)
	{
		case	DT_MESH:
			if(p_thing->Draw.Mesh)
			{
				ULONG	drawtype;
				drawtype=(p_thing->Draw.Mesh-DRAW_MESHES);
				p_thing->Draw.Mesh=(DrawMesh*)drawtype;
			}
		break;
		case	DT_ROT_MULTI:
		case	DT_ANIM_PRIM:
		case	DT_BIKE:
			if(p_thing->Draw.Tweened)
			{
				ULONG	drawtype;
				SLONG	chunk;

				switch(p_thing->Class)
				{
					case	CLASS_BIKE:
					case	CLASS_PERSON:
					case	CLASS_VEHICLE:
					case	CLASS_ANIMAL:
						chunk=(ULONG)(p_thing->Draw.Tweened->TheChunk-&game_chunk[0]);
						ASSERT(chunk>=0);
						chunk|=1<<16;
						p_thing->Draw.Tweened->TheChunk=(GameKeyFrameChunk*)chunk;
						break;
					case	CLASS_BAT:
					case	CLASS_ANIM_PRIM:
						chunk=(ULONG)(p_thing->Draw.Tweened->TheChunk-&anim_chunk[0]);
						ASSERT(chunk>=0);
						chunk|=2<<16;
						p_thing->Draw.Tweened->TheChunk=(GameKeyFrameChunk*)chunk;
						break;
				}

				drawtype=(p_thing->Draw.Tweened-DRAW_TWEENS);
				p_thing->Draw.Tweened=(DrawTween*)drawtype;
			}
			break;
	}
}


void	convert_thing_to_index(Thing *p_thing)
{
//	ASSERT(THING_NUMBER(p_thing)!=94);
	switch(p_thing->DrawType)
	{
		case	DT_MESH:
		case	DT_CHOPPER:
			convert_drawtype_to_index(p_thing,DT_MESH);
			break;
		case	DT_ROT_MULTI:
			convert_drawtype_to_index(p_thing,DT_ROT_MULTI);
			break;

		case	DT_ANIM_PRIM:
		case	DT_BIKE:
			convert_drawtype_to_index(p_thing,DT_ANIM_PRIM);
			break;
	}

	switch(p_thing->Class)
	{

		case	CLASS_NONE:
			break;
		case	CLASS_PLAYER:
			p_thing->Genus.Player->PlayerPerson=(Thing*)THING_NUMBER(p_thing->Genus.Player->PlayerPerson);
			p_thing->Genus.Player=(Player*)PLAYER_NUMBER(p_thing->Genus.Player);

			break;
		case	CLASS_CAMERA:
			break;
		case	CLASS_PROJECTILE:
			p_thing->Genus.Projectile=(Projectile *)PROJECTILE_NUMBER(p_thing->Genus.Projectile);
			break;
		case	CLASS_BUILDING:
			break;
		case	CLASS_PERSON:
			p_thing->Genus.Person=(Person *)PERSON_NUMBER(p_thing->Genus.Person);
			
			break;
		case	CLASS_ANIMAL:
			p_thing->Genus.Animal=(Animal*)ANIMAL_NUMBERb(p_thing->Genus.Animal);

			break;
		case	CLASS_FURNITURE:
			p_thing->Genus.Furniture=(Furniture*)FURNITURE_NUMBER(p_thing->Genus.Furniture);
			break;
		case	CLASS_SWITCH:
			p_thing->Genus.Switch=(Switch*)SWITCH_NUMBER(p_thing->Genus.Switch);
			break;
		case	CLASS_VEHICLE:
			p_thing->Genus.Vehicle=(Vehicle*)VEHICLE_NUMBER(p_thing->Genus.Vehicle);
			break;
		case	CLASS_SPECIAL:
			p_thing->Genus.Special=(Special*)SPECIAL_NUMBER(p_thing->Genus.Special);
			break;
		case	CLASS_ANIM_PRIM:
			break;
		case	CLASS_CHOPPER:
			p_thing->Genus.Chopper=(Chopper*)CHOPPER_NUMBER(p_thing->Genus.Chopper);

			break;
		case	CLASS_PYRO:
			p_thing->Genus.Pyro=(Pyro*)PYRO_NUMBER(p_thing->Genus.Pyro);
			break;
		case	CLASS_TRACK:
			if (p_thing->Genus.Track->flags==TRACK_FLAGS_SPLUTTING)
				p_thing->Genus.Track->page=POLY_PAGE_BLOODSPLAT;
			p_thing->Genus.Track=(Track*)TRACK_NUMBER(p_thing->Genus.Track);
			break;
		case	CLASS_PLAT:
			p_thing->Genus.Plat=(Plat*)PLAT_NUMBER(p_thing->Genus.Plat);

			break;
		case	CLASS_BARREL:
			p_thing->Genus.Barrel=(Barrel*)BARREL_NUMBER(p_thing->Genus.Barrel);

			break;
		#ifdef BIKE
		case	CLASS_BIKE:
			p_thing->Genus.Bike=(BIKE_Bike*)BIKE_NUMBER(p_thing->Genus.Bike);
			break;
		#endif
		case	CLASS_BAT:
			p_thing->Genus.Bat=(Bat*)BAT_NUMBER(p_thing->Genus.Bat);
			break;

		default:
			ASSERT(0);
			break;
	}

}

void	convert_pointers_to_index(void)
{
	SLONG	c0,i;
	static	SLONG max_people=0,max_car=0,max_mesh=0,max_tween=0,max_anim=0,max_special=0,max_bat=0;
	SLONG	count_people=0,count_car=0,count_mesh=0,count_tween=0,count_anim=0,count_special=0,count_bat=0;
	SLONG	gap=0;

	for(c0=0;c0<MAX_THINGS;c0++)
	{
		convert_thing_to_index(TO_THING(c0));
/*
		switch(TO_THING(c0)->Class)
		{
			case	CLASS_PERSON:
				count_people++;
				break;
			case	CLASS_VEHICLE:
				count_car++;
				break;
			case	CLASS_SPECIAL:
				count_special++;
				break;
			case	CLASS_BAT:
				count_bat++;
				break;
		}
		switch(TO_THING(c0)->DrawType)
		{
			case	DT_MESH:
			case	DT_CHOPPER:
				count_mesh++;
				break;
			case	DT_ROT_MULTI:
				count_tween++;
				break;

			case	DT_ANIM_PRIM:
			case	DT_BIKE:
				count_anim++;
				break;
		}
*/
	}
	for (i = 0; i < RMAX_DRAW_MESHES; i++)
	{
		if (DRAW_MESHES[i].Angle != 0xfafa)
		{
			if(i>count_mesh)
			{
				count_mesh=i;
			}
		} 
	}

	for(c0=0;c0<RMAX_DRAW_TWEENS;c0++)
	{
		if(!(DRAW_TWEENS[c0].Flags&DT_FLAG_UNUSED))
		{
			if(c0>count_tween)
				count_tween=c0;
		}
	}
	for (i = 0; i < RMAX_VEHICLES; i++)
	{
		if (TO_VEHICLE(i)->Spring[0].Compression != VEH_NULL)
		{
			if(i>count_car)
				count_car=i;

		}
	}
	for(c0=0;c0<RMAX_PEOPLE;c0++)
	{
		if(PEOPLE[c0].AnimType!=PERSON_NONE)
		{
			if(c0>count_people)
				count_people=c0;

		}
	}
	for(c0=1;c0<RMAX_SPECIALS;c0++)
	{
		if(SPECIALS[c0].SpecialType!=SPECIAL_NONE)
		{	
			if(c0>count_special)
				count_special=c0;
		}
	}

	for (i = 0; i < RBAT_MAX_BATS; i++)
	{
		if (TO_BAT(i)->type != BAT_TYPE_UNUSED)
		{
			if(i>count_bat)
				count_bat=i;
		}
	}

	

	save_table[SAVE_TABLE_PEOPLE].Maximum  =MIN(save_table[SAVE_TABLE_PEOPLE].Extra+count_people,RMAX_PEOPLE);
	save_table[SAVE_TABLE_VEHICLE].Maximum =MIN(save_table[SAVE_TABLE_VEHICLE].Extra+count_car,RMAX_VEHICLES);
	save_table[SAVE_TABLE_SPECIAL].Maximum =MIN(save_table[SAVE_TABLE_SPECIAL].Extra+count_special,RMAX_SPECIALS);
	save_table[SAVE_TABLE_BAT].Maximum     =MIN(save_table[SAVE_TABLE_BAT].Extra+count_bat,RBAT_MAX_BATS);
	save_table[SAVE_TABLE_DTWEEN].Maximum  =MIN(save_table[SAVE_TABLE_DTWEEN].Extra+count_tween,RMAX_DRAW_TWEENS);
	save_table[SAVE_TABLE_DMESH].Maximum   =MIN(save_table[SAVE_TABLE_DMESH].Extra+count_mesh,RMAX_DRAW_MESHES);

extern CBYTE ELEV_fname_level[];
	if ( level_index==0 )
	{
		// This is probably Breakout! - in which case, we need the fudge in.
		// Make sure you manually step into the fudge code.
		if (strstr(ELEV_fname_level, "FTutor"))
		{
		}
		else
		{
			//ASSERT ( strstr ( ELEV_fname_level, "Album1" ) );
			//ASSERT ( FALSE );
		}
	}
	if(level_index==20 || level_index==19 || level_index==	26|| level_index==	24 || strstr ( ELEV_fname_level, "Album1" ) )
	{
		//cop killers or semtex or estate map or stern revenge

		save_table[SAVE_TABLE_PEOPLE].Maximum  =MIN(save_table[SAVE_TABLE_PEOPLE].Extra+count_people+30,RMAX_PEOPLE);
		save_table[SAVE_TABLE_DTWEEN].Maximum  =MIN(save_table[SAVE_TABLE_DTWEEN].Extra+count_tween+30,RMAX_DRAW_TWEENS);
		save_table[SAVE_TABLE_DMESH].Maximum   =MIN(save_table[SAVE_TABLE_DMESH].Extra+count_mesh+30,RMAX_DRAW_MESHES);
	}


	if(count_special>max_special)
	{
		max_special=count_special;
		DebugText(" MOSTspecialS %d \n",max_special);
	}
	if(count_bat>max_bat)
	{
		max_bat=count_bat;
		DebugText(" MOSTbatS %d \n",max_bat);
	}
	if(count_mesh>max_mesh)
	{
		max_mesh=count_mesh;
		DebugText(" MOSTmeshS %d \n",max_mesh);
	}
	if(count_tween>max_tween)
	{
		max_tween=count_tween;
		DebugText(" MOSTtweenS %d \n",max_tween);
	}
	if(count_anim>max_anim)
	{
		max_anim=count_anim;
		DebugText(" MOSTanimS %d \n",max_anim);
	}

	if(count_car>max_car)
	{
		max_car=count_car;
		DebugText(" MOSTCARS %d \n",max_car);
	}
	if(count_people>max_people)
	{
		max_people=count_people;
		DebugText(" MOSTPEOPLE %d \n",max_people);
	}

	for(c0=0;c0<MAX_PLAYERS;c0++)
	{
		NET_PERSON(c0)=(Thing*)THING_NUMBER(NET_PERSON(c0));
		NET_PLAYER(c0)=(Thing*)THING_NUMBER(NET_PLAYER(c0));
	}

	for(c0=0;c0<EWAY_mess_upto;c0++)
	{
		EWAY_mess[c0]=(CBYTE*)((SLONG)(EWAY_mess[c0]-EWAY_mess_buffer));
	}

	for(c0=0;c0<PYRO_COUNT;c0++)
	{
		if(TO_PYRO(c0)->thing)
			TO_PYRO(c0)->thing=(Thing*)THING_NUMBER(TO_PYRO(c0)->thing);

		if(TO_PYRO(c0)->victim)
			TO_PYRO(c0)->victim=(Thing*)THING_NUMBER(TO_PYRO(c0)->victim);
	}

#ifndef BUILD_PSX
	// cutscene stuff. convert the track pointers first:
	for(c0=0;c0<PLAYCUTS_cutscene_ctr;c0++)
	{
		PLAYCUTS_cutscenes[c0].channels=(CPChannel*)(PLAYCUTS_cutscenes[c0].channels-PLAYCUTS_tracks);
	}
	for(c0=0;c0<PLAYCUTS_track_ctr;c0++)
	{
		PLAYCUTS_tracks[c0].packets=(CPPacket*)(PLAYCUTS_tracks[c0].packets-PLAYCUTS_packets);
	}
	for(c0=0;c0<PLAYCUTS_packet_ctr;c0++)
	{
//		if (PLAYCUTS_packets[c0].type==PT_TEXT) PLAYCUTS_packets[c0].pos.X-=PLAYCUTS_text_data;
		if (PLAYCUTS_packets[c0].type==5) PLAYCUTS_packets[c0].pos.X-=(ULONG)PLAYCUTS_text_data;
	}
#endif
}

#define	STORE_DATA(a)	FileWrite(handle,(UBYTE*)&a,sizeof(a));DebugText(" store data %d \n",sizeof(a))

void	convert_keyframe_to_index(GameKeyFrame *p,GameKeyFrameElement *p_ele,GameFightCol *p_fight,SLONG count)
{
	SLONG	c0;
	for(c0=0;c0<count;c0++)
	{
		p[c0].FirstElement=(GameKeyFrameElement *)((SLONG)(p[c0].FirstElement-p_ele));
		p[c0].PrevFrame=(GameKeyFrame*)((SLONG)(p[c0].PrevFrame-p));
		p[c0].NextFrame=(GameKeyFrame*)((SLONG)(p[c0].NextFrame-p));
		p[c0].Fight=(GameFightCol*)((SLONG)(p[c0].Fight-p_fight));
	}
}

void	convert_animlist_to_index(GameKeyFrame **p,GameKeyFrame *p_anim,SLONG count)
{
	SLONG	c0;
	for(c0=0;c0<count;c0++)
	{
		p[c0]=(GameKeyFrame*)((SLONG)(p[c0]-p_anim));
	}
}

void	convert_fightcol_to_index(GameFightCol *p,GameFightCol *p_fight,SLONG count)
{
	SLONG	c0;
	for(c0=0;c0<count;c0++)
	{
		p[c0].Next=(GameFightCol*)((SLONG)(p[c0].Next-p_fight));
	}

}
#endif

void	convert_keyframe_to_pointer(GameKeyFrame *p,GameKeyFrameElement *p_ele,GameFightCol *p_fight,SLONG count)
{
	SLONG	c0;


	ASSERT(( ((ULONG)p)&3)==0);
	ASSERT(( ((ULONG)p_ele)&3)==0);
	ASSERT(( ((ULONG)p_fight)&3)==0);

	for(c0=0;c0<count;c0++)
	{
		if(((SLONG)p[c0].FirstElement)<0)
			p[c0].FirstElement=NULL;
		else
			p[c0].FirstElement=&p_ele[(SLONG)p[c0].FirstElement];

		if(((SLONG)p[c0].PrevFrame)<0)
			p[c0].PrevFrame=NULL;
		else
			p[c0].PrevFrame=&p[(SLONG)p[c0].PrevFrame];

		if(((SLONG)p[c0].NextFrame)<0)
			p[c0].NextFrame=NULL;
		else
			p[c0].NextFrame=&p[(SLONG)p[c0].NextFrame];

		if(((SLONG)p[c0].Fight)<0)
			p[c0].Fight=NULL;
		else
			p[c0].Fight=&p_fight[(SLONG)p[c0].Fight];
	}
}

void	convert_animlist_to_pointer(GameKeyFrame **p,GameKeyFrame *p_anim,SLONG count)
{
	SLONG	c0;
	for(c0=0;c0<count;c0++)
	{
		p[c0]=&p_anim[(SLONG)p[c0]];
	}
}

void	convert_fightcol_to_pointer(GameFightCol *p,GameFightCol *p_fight,SLONG count)
{
	SLONG	c0;
	for(c0=0;c0<count;c0++)
	{

		if(((SLONG)p[c0].Next)<0)
			p[c0].Next=0;
		else
			p[c0].Next=&p_fight[(SLONG)p[c0].Next];
	}

}

#ifndef PSX

void	save_whole_anims(MFFileHandle handle)
{
	SLONG	c0,c1;
	SLONG	blank=-1;
	SLONG	check=666;


	STORE_DATA(next_game_chunk);
	STORE_DATA(next_anim_chunk);

	for(c0=0;c0<next_game_chunk;c0++)
	{
		if(game_chunk[c0].MultiObject[0])
		{
			struct	 GameKeyFrameChunk	*gc;
			gc=&game_chunk[c0];
			STORE_DATA(c0);							//4
			STORE_DATA(gc->MaxPeopleTypes);         //2
			STORE_DATA(gc->MaxKeyFrames);			//2
			STORE_DATA(gc->MaxAnimFrames);			//2
			STORE_DATA(gc->MaxFightCols);			//2
			STORE_DATA(gc->MaxElements);			//4
			STORE_DATA(gc->ElementCount);			//4

			//
			// Convert the pointers to indexes
			//
				
			convert_keyframe_to_index(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_index(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_index(gc->FightCols,gc->FightCols,gc->MaxFightCols);

			//
			// now save the data blocks
			//
			STORE_DATA(check);
			for(c1=0;c1<10;c1++)
			{
				STORE_DATA(gc->MultiObject[c1]);
			}

			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->PeopleTypes,gc->MaxPeopleTypes*sizeof(struct BodyDef));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->AnimKeyFrames,gc->MaxKeyFrames*sizeof(GameKeyFrame));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->AnimList,gc->MaxAnimFrames*sizeof(GameKeyFrame*));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->TheElements,gc->MaxElements*sizeof(GameKeyFrameElement));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->FightCols,gc->MaxFightCols*sizeof(GameFightCol));
			STORE_DATA(check);

			//
			// Now convert the indexes back to pointers
			//

			convert_keyframe_to_pointer(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_pointer(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_pointer(gc->FightCols,gc->FightCols,gc->MaxFightCols);


		}
		else
			STORE_DATA(blank);

	}

	for(c0=0;c0<next_anim_chunk;c0++)
	{
		if(anim_chunk[c0].MultiObject[0])
		{
			struct	 GameKeyFrameChunk	*gc;
			gc=&anim_chunk[c0];
			STORE_DATA(c0);							//4 
			STORE_DATA(gc->MaxPeopleTypes);	        //2 
			STORE_DATA(gc->MaxKeyFrames);			//2 
			STORE_DATA(gc->MaxAnimFrames);			//2 
			STORE_DATA(gc->MaxFightCols);			//2 
			STORE_DATA(gc->MaxElements);			//4 
			STORE_DATA(gc->ElementCount);			//4 

			//
			// Convert the pointers to indexes
			//
				
			convert_keyframe_to_index(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_index(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_index(gc->FightCols,gc->FightCols,gc->MaxFightCols);

			//
			// now save the data blocks
			//
			for(c1=0;c1<10;c1++)
			{
				STORE_DATA(gc->MultiObject[c1]);
			}

//			FileWrite(handle,(UBYTE*)&gc->MultiObject[0],10*sizeof(UWORD));
			FileWrite(handle,(UBYTE*)gc->PeopleTypes,gc->MaxPeopleTypes*sizeof(struct BodyDef));
			FileWrite(handle,(UBYTE*)gc->AnimKeyFrames,gc->MaxKeyFrames*sizeof(GameKeyFrame));
			FileWrite(handle,(UBYTE*)gc->AnimList,gc->MaxAnimFrames*sizeof(GameKeyFrame*));
			FileWrite(handle,(UBYTE*)gc->TheElements,gc->MaxElements*sizeof(GameKeyFrameElement));
			FileWrite(handle,(UBYTE*)gc->FightCols,gc->MaxFightCols*sizeof(GameFightCol));

			//
			// Now convert the indexes back to pointers
			//

			convert_keyframe_to_pointer(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_pointer(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_pointer(gc->FightCols,gc->FightCols,gc->MaxFightCols);


		}
		else
			STORE_DATA(blank);
	}
}



SLONG find_best_anim_offset_old(SLONG mx,SLONG my,SLONG mz,SLONG bdist)
{
	SLONG	c0,dist;

	bdist=128-bdist;
	ASSERT(bdist>0);

	for(c0=0;(unsigned)c0<next_anim_mids;c0++)
	{
		SLONG	dx,dy,dz;
		dx=anim_mids[c0].X-mx;
		dy=anim_mids[c0].Y-my;
		dz=anim_mids[c0].Z-mz;

		dist=Root(dx*dx+dy*dy+dz*dz);
		if(dist<bdist)
			return(c0);
	}

	if(next_anim_mids<256)
	{
		anim_mids[next_anim_mids].X=mx;
		anim_mids[next_anim_mids].Y=my;
		anim_mids[next_anim_mids].Z=mz;
		next_anim_mids++;
		return(next_anim_mids-1);
	}
	ASSERT(0);
	return 0;
}



SLONG find_best_anim_offset(SLONG mx,SLONG my,SLONG mz,SLONG anim,struct GameKeyFrameChunk *gc)
{
	SLONG	c0,dist,bdist;


//	if(anim<1565 || anim>1570)
	if(1)
	{
		for(c0=0;(unsigned)c0<next_anim_mids;c0++)
		{
			SLONG	cx,cy,cz;
			SLONG	dx,dy,dz;
			SLONG ele;

			bdist=0;
			cx=anim_mids[c0].X;
			cy=anim_mids[c0].Y;
			cz=anim_mids[c0].Z;

			for(ele=0;ele<gc->ElementCount;ele++)
			{
				dx=gc->AnimKeyFrames[anim].FirstElement[ele].OffsetX+mx-cx;
				dy=gc->AnimKeyFrames[anim].FirstElement[ele].OffsetY+my-cy;
				dz=gc->AnimKeyFrames[anim].FirstElement[ele].OffsetZ+mz-cz;
				dist=Root(dx*dx+dy*dy+dz*dz);
				if(dist>bdist)
					bdist=dist;
			}
			if(bdist<128)
				return(c0);
		}
	}
	else
	{
		ASSERT(0);

	}

	if(next_anim_mids<256)
	{
		DebugText(" added a new anim_mid %d  (%d,%d,%d)\n",next_anim_mids,mx,my,mz);
		anim_mids[next_anim_mids].X=mx;
		anim_mids[next_anim_mids].Y=my;
		anim_mids[next_anim_mids].Z=mz;
		next_anim_mids++;
		return(next_anim_mids-1);
	}
	ASSERT(0);
	return 0;
}


extern void convert_to_psx_gke(GameKeyFrameElementComp *to, GameKeyFrameElement *from);
#ifndef	ULTRA_COMPRESSED_ANIMATIONS


void	fix_psxed_anims(void)
{
	SLONG	c0,c1,ele,index;
	struct	GameKeyFrameElementComp	*p;
	UWORD	*bits;

	for(c0=0;c0<next_game_chunk;c0++)
	{
		if(game_chunk[c0].MultiObject[0])
		{
			struct	 GameKeyFrameChunk	*gc;

			gc=&game_chunk[c0];

			bits=(UWORD *)MemAlloc(sizeof(UWORD)*gc->MaxElements);
			memset((UBYTE*)bits,0,gc->MaxElements*2);

			if(c0!=5)
			for(c1=0;c1<gc->MaxKeyFrames;c1++)
			{

				if(gc->AnimKeyFrames[c1].FirstElement)
				{
					SLONG	ele_index;
					ele_index=(ULONG)(gc->AnimKeyFrames[c1].FirstElement-gc->TheElements);

					  //
					  // don't want to do a keyframe twice
					  //
					if(bits[ele_index])
					{

					}
					else
					{
						SLONG	bdist=0;
						SLONG	mx,my,mz;
						mx=anim_mids[gc->AnimKeyFrames[c1].XYZIndex].X;
						my=anim_mids[gc->AnimKeyFrames[c1].XYZIndex].Y;
						mz=anim_mids[gc->AnimKeyFrames[c1].XYZIndex].Z;

						for(ele=0;ele<gc->ElementCount;ele++)
						{
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX+=mx;
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY+=my;
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ+=mz;

						}

						bits[ele_index]=1; //done
					}
				}
				gc->AnimKeyFrames[c1].XYZIndex=0;
			}
			MemFree(bits);
		}

	}
	next_anim_mids=0;

}

void	save_whole_anims_psx(MFFileHandle handle)
{
	SLONG	c0,c1;
	SLONG	blank=-1;
	SLONG	check=666;
	struct	GameKeyFrameElementComp	*p;
	UWORD	*bits;
	ULONG	ele_index,ele;
	SLONG	big_x=0,big_y=0,big_z=0;

	next_anim_mids=0;

	STORE_DATA(next_game_chunk);
	STORE_DATA(next_anim_chunk);

	for(c0=0;c0<next_game_chunk;c0++)
	{
		if(game_chunk[c0].MultiObject[0])
		{
			struct	 GameKeyFrameChunk	*gc;

			gc=&game_chunk[c0];

			p=(struct	 GameKeyFrameElementComp	*)MemAlloc(sizeof(struct GameKeyFrameElementComp)*gc->MaxElements);
			bits=(UWORD *)MemAlloc(sizeof(UWORD)*gc->MaxElements);
			memset((UBYTE*)bits,0,gc->MaxElements*2);

			if(c0!=5)
			for(c1=0;c1<gc->MaxKeyFrames;c1++)
			{
				SLONG	bx=0,by=0,bz=0;//999999,by=9999999,bz=9999999,ele;
				UWORD	off_bits=0;

//				ASSERT(c1!=1565);
				if(gc->AnimKeyFrames[c1].FirstElement)
				{
					ele_index=(ULONG)(gc->AnimKeyFrames[c1].FirstElement-gc->TheElements);

  //
  // don't want to do a keyframe twice
  //
					if(bits[ele_index])
					{
						gc->AnimKeyFrames[c1].XYZIndex=bits[ele_index]-1;

					}
					else
					{
						SLONG	bdist=0;
						for(ele=0;(signed)ele<gc->ElementCount;ele++)
						{
							
							//if(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX<bx)
								bx+=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX);

							//if(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY<by)
								by+=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY);

							//if(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ<bz)
								bz+=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ);

						}
						bx/=gc->ElementCount;
						by/=gc->ElementCount;
						bz/=gc->ElementCount;
/*
						bx+=128;
						by+=128;
						bz+=128;
						bx&=0xffffff00;
						by&=0xffffff00;
						bz&=0xffffff00;
*/

						for(ele=0;(signed)ele<gc->ElementCount;ele++)
						{
							SLONG	dx,dy,dz,dist;

							
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX-=bx;
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY-=by;
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ-=bz;

							dx=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX);
							dy=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY);
							dz=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ);
							dist=Root(dx*dx+dy*dy+dz*dz);

							if(dist>bdist)
								bdist=dist;


							if(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX)>abs(big_x))
								big_x=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX);

							if(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY)>abs(big_y))
								big_y=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY);

							if(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ)>abs(big_z))
								big_z=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ);


							

//							ASSERT(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX>=-128 && gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX<128);
//							ASSERT(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY>=-128 && gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY<128);
//							ASSERT(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ>=-128 && gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ<128);

						}
						ASSERT(bdist<128);
						ASSERT(abs(big_x)<128);
						ASSERT(abs(big_y)<128);
						ASSERT(abs(big_z)<128);

						{
							SLONG	dx,dy,dz;
							SLONG	index;

							index=find_best_anim_offset(bx,by,bz,c1,gc);

							dx=bx-anim_mids[index].X;
							dy=by-anim_mids[index].Y;
							dz=bz-anim_mids[index].Z;

							for(ele=0;(signed)ele<gc->ElementCount;ele++)
							{
								gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX+=dx;
								gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY+=dy;
								gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ+=dz;
								ASSERT(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX)<128);
								ASSERT(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY)<128);
								ASSERT(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ)<128);

							}
							gc->AnimKeyFrames[c1].XYZIndex=index;
							bits[ele_index]=index+1; //done

						}

/*
						off_bits=(bx&0x700)>>7;
						off_bits|=(bz&0x700)>>4;
						off_bits|=(by&0xff00);
						bits[ele_index]=off_bits|1;
*/
//						bits[ele_index]=1; //done
					}
					gc->AnimKeyFrames[c1].Flags=(gc->AnimKeyFrames[c1].Flags&1);//|off_bits;
				}

			}
			DebugText(" game chunk %d  bigx %d bigy %d bigz %d \n",c0,big_x,big_y,big_z);

			big_x=0;
			big_y=0;
			big_z=0;



			for(c1=0;c1<gc->MaxElements;c1++)
			{
				convert_to_psx_gke(&p[c1],&gc->TheElements[c1]);
			}


			STORE_DATA(c0);							//4
			STORE_DATA(gc->MaxPeopleTypes);         //2
			STORE_DATA(gc->MaxKeyFrames);			//2
			STORE_DATA(gc->MaxAnimFrames);			//2
			STORE_DATA(gc->MaxFightCols);			//2
			STORE_DATA(gc->MaxElements);			//4
			STORE_DATA(gc->ElementCount);			//4

			//
			// Convert the pointers to indexes
			//
				
			convert_keyframe_to_index(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_index(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_index(gc->FightCols,gc->FightCols,gc->MaxFightCols);

			//
			// now save the data blocks
			//
			STORE_DATA(check);
			for(c1=0;c1<10;c1++)
			{
				STORE_DATA(gc->MultiObject[c1]);
			}


			STORE_DATA(check);
			DebugText(" peep_types -> %d  tot %d\n",gc->MaxPeopleTypes,gc->MaxPeopleTypes*sizeof(struct BodyDef));
			DebugText(" keyframes  -> %d  tot %d\n",gc->MaxKeyFrames,gc->MaxKeyFrames*sizeof(GameKeyFrame));
			DebugText(" animlist   -> %d  tot %d\n",gc->MaxAnimFrames,gc->MaxAnimFrames*sizeof(GameKeyFrame*));
			DebugText(" elements   -> %d  tot %d\n",gc->MaxElements,gc->MaxElements*sizeof(GameKeyFrameElementComp));
			DebugText(" fightcols  -> %d  tot %d\n",gc->MaxFightCols,gc->MaxFightCols*sizeof(GameFightCol));




			FileWrite(handle,(UBYTE*)gc->PeopleTypes,gc->MaxPeopleTypes*sizeof(struct BodyDef));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->AnimKeyFrames,gc->MaxKeyFrames*sizeof(GameKeyFrame));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->AnimList,gc->MaxAnimFrames*sizeof(GameKeyFrame*));
			STORE_DATA(check);
//			FileWrite(handle,(UBYTE*)gc->TheElements,gc->MaxElements*sizeof(GameKeyFrameElement));
			FileWrite(handle,(UBYTE*)p,gc->MaxElements*sizeof(GameKeyFrameElementComp));
			STORE_DATA(check);
			FileWrite(handle,(UBYTE*)gc->FightCols,gc->MaxFightCols*sizeof(GameFightCol));
			STORE_DATA(check);

			//
			// Now convert the indexes back to pointers
			//

			convert_keyframe_to_pointer(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_pointer(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_pointer(gc->FightCols,gc->FightCols,gc->MaxFightCols);

			MemFree(p);
			MemFree(bits);


		}
		else
			STORE_DATA(blank);

	}

	for(c0=0;c0<next_anim_chunk;c0++)
	{
		if(anim_chunk[c0].MultiObject[0])
		{
			struct	 GameKeyFrameChunk	*gc;
			UWORD	ele_index;

			gc=&anim_chunk[c0];



			p=(struct	 GameKeyFrameElementComp	*)MemAlloc(sizeof(struct GameKeyFrameElementComp)*gc->MaxElements);
			bits=(UWORD *)MemAlloc(sizeof(UWORD)*gc->MaxElements);
			memset((UBYTE*)bits,0,gc->MaxElements*2);

  /*
			for(c1=0;c1<gc->MaxKeyFrames;c1++)
			{
				SLONG	bx=0,by=0,bz=0;//999999,by=9999999,bz=9999999,ele;
				UWORD	off_bits=0;
				if(gc->AnimKeyFrames[c1].FirstElement)
				{
					ele_index=(ULONG)(gc->AnimKeyFrames[c1].FirstElement-gc->TheElements);
					if(bits[ele_index])
					{
						off_bits=bits[ele_index]&0xfffe;
					}
					else
					{
						for(ele=0;ele<gc->ElementCount;ele++)
						{
							
							//if(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX<bx)
								bx+=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX);

							//if(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY<by)
								by+=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY);

							//if(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ<bz)
								bz+=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ);

						}
						bx/=gc->ElementCount;
						by/=gc->ElementCount;
						bz/=gc->ElementCount;

						bx+=128;
						by+=128;
						bz+=128;
						bx&=0xffffff00;
						by&=0xffffff00;
						bz&=0xffffff00;

						for(ele=0;ele<gc->ElementCount;ele++)
						{
							
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX-=bx;
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY-=by;
							gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ-=bz;

							if(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX)>abs(big_x))
								big_x=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX);

							if(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY)>abs(big_y))
								big_y=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY);

							if(abs(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ)>abs(big_z))
								big_z=(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ);


//							ASSERT(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX>=-128 && gc->AnimKeyFrames[c1].FirstElement[ele].OffsetX<128);
//							ASSERT(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY>=-128 && gc->AnimKeyFrames[c1].FirstElement[ele].OffsetY<128);
//							ASSERT(gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ>=-128 && gc->AnimKeyFrames[c1].FirstElement[ele].OffsetZ<128);

						}


						off_bits=(bx&0x700)>>7;
						off_bits|=(bz&0x700)>>4;
						off_bits|=(by&0xff00);
						bits[ele_index]=off_bits|1;
					}
					gc->AnimKeyFrames[c1].Flags=(gc->AnimKeyFrames[c1].Flags&1)|off_bits;
				}


			}
*/


			for(c1=0;c1<gc->MaxElements;c1++)
			{
				convert_to_psx_gke(&p[c1],&gc->TheElements[c1]);
			}


			STORE_DATA(c0);							//4 
			STORE_DATA(gc->MaxPeopleTypes);	        //2 
			STORE_DATA(gc->MaxKeyFrames);			//2 
			STORE_DATA(gc->MaxAnimFrames);			//2 
			STORE_DATA(gc->MaxFightCols);			//2 
			STORE_DATA(gc->MaxElements);			//4 
			STORE_DATA(gc->ElementCount);			//4 

			//
			// Convert the pointers to indexes
			//
				
			convert_keyframe_to_index(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_index(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_index(gc->FightCols,gc->FightCols,gc->MaxFightCols);

			//
			// now save the data blocks
			//
			for(c1=0;c1<10;c1++)
			{
				STORE_DATA(gc->MultiObject[c1]);
			}

			DebugText(" peep_types -> %d  tot %d\n",gc->MaxPeopleTypes,gc->MaxPeopleTypes*sizeof(struct BodyDef));
			DebugText(" keyframes  -> %d  tot %d\n",gc->MaxKeyFrames,gc->MaxKeyFrames*sizeof(GameKeyFrame));
			DebugText(" animlist   -> %d  tot %d\n",gc->MaxAnimFrames,gc->MaxAnimFrames*sizeof(GameKeyFrame*));
			DebugText(" elements   -> %d  tot %d\n",gc->MaxElements,gc->MaxElements*sizeof(GameKeyFrameElementComp));
			DebugText(" fightcols  -> %d  tot %d\n",gc->MaxFightCols,gc->MaxFightCols*sizeof(GameFightCol));

//			FileWrite(handle,(UBYTE*)&gc->MultiObject[0],10*sizeof(UWORD));
			FileWrite(handle,(UBYTE*)gc->PeopleTypes,gc->MaxPeopleTypes*sizeof(struct BodyDef));
			FileWrite(handle,(UBYTE*)gc->AnimKeyFrames,gc->MaxKeyFrames*sizeof(GameKeyFrame));
			FileWrite(handle,(UBYTE*)gc->AnimList,gc->MaxAnimFrames*sizeof(GameKeyFrame*));
//			FileWrite(handle,(UBYTE*)gc->TheElements,gc->MaxElements*sizeof(GameKeyFrameElement));
			FileWrite(handle,(UBYTE*)p,gc->MaxElements*sizeof(GameKeyFrameElementComp));
			FileWrite(handle,(UBYTE*)gc->FightCols,gc->MaxFightCols*sizeof(GameFightCol));

			//
			// Now convert the indexes back to pointers
			//

			convert_keyframe_to_pointer(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_pointer(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_pointer(gc->FightCols,gc->FightCols,gc->MaxFightCols);

			DebugText(" anim chunk %d  bigx %d bigy %d bigz %d \n",c0,big_x,big_y,big_z);

			big_x=0;
			big_y=0;
			big_z=0;
			MemFree(p);
			MemFree(bits);

		}
		else
			STORE_DATA(blank);

	}
	STORE_DATA(next_anim_mids);
	FileWrite(handle,(UBYTE*)anim_mids,next_anim_mids*sizeof(PrimPoint));

}
#endif

#define getPSXU(page)		(((page)&7)<<5)
#define getPSXV(page)		(((page)&0x38)<<2)

//#ifdef PSX
extern	UWORD	psx_start_page;
//#endif
void	fix_psx_face3(struct	PrimFace3PSX	*p2)
{
//#ifdef PSX
	ULONG	page;
	SLONG	x;
	SLONG	u,v;

	page=p2->TexturePage;//|((p2->UV[0][0]&0xc0)<<2)+(8<<6);
	p2->TexturePage+=(psx_start_page>>6)<<6;

	p2->UV[0][0]&=0x3f;

	u=getPSXU(page);
	v=getPSXV(page);

	for(x=0;x<3;x++)
	{
		if (p2->UV[x][0]==32) p2->UV[x][0]=31;
			p2->UV[x][0]+=u;
		if (p2->UV[x][1]==32) p2->UV[x][1]=31;
			p2->UV[x][1]+=v;
	}
//#endif
}

void	fix_psx_face4(struct	PrimFace4PSX	*p)
{
//#ifdef PSX
	ULONG	page;
	SLONG	x;
	SLONG	u,v;

	page=p->TexturePage;
	if (!(p->FaceFlags & FACE_FLAG_WALKABLE))
		p->TexturePage+=(psx_start_page>>6)<<6;

	p->UV[0][0]&=0x3f;
	u=getPSXU(page);
	v=getPSXV(page);

	for(x=0;x<4;x++)
	{
		if (p->UV[x][0]==32) p->UV[x][0]=31;
			p->UV[x][0]+=u;
		if (p->UV[x][1]==32) p->UV[x][1]=31;
			p->UV[x][1]+=v;
	}
//#endif
}
void	save_whole_wad(CBYTE	*gamename,UBYTE type)
{

//	return;

	SLONG	c0=0;
	SLONG	*p_slong;
	UWORD	*p_uword;
	UBYTE	*p_mem;
	SLONG	mem_size,mem_cumlative=0;
	struct	MemTable *ptab;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	SLONG	count;
	SLONG	save_type=0;
	SLONG	check=666;
	UBYTE	padding_byte;
	UWORD	padding_word;
	ULONG	struct_size;


	set_darci_normals();
	//
	// remove anything that can't survive the reload (transient effects)
	//


	handle=FileCreate(gamename,1);
	if(handle!=FILE_CREATION_ERROR) 
	{
// get rid of the nasty pointers
// by converting to indexes
		convert_pointers_to_index();
		DebugText("\n SAVE INGAME \n");

		STORE_DATA(save_type);



		while(save_table[c0].Point)
		{
			mem_size=0;
			count=0;
//			ASSERT(c0!=29);
			ptab=&save_table[c0];
			switch(ptab->Type)
			{
				case	2:
					mem_size=ptab->Maximum*ptab->StructSize;
					count=ptab->Maximum;
					break;

				case	1:
					if(ptab->CountL)
					{
						count=*ptab->CountL;
					}
					else
					{
						if(ptab->CountW)
							count=*ptab->CountW;
						else
							ASSERT(0);
					}
					if(ptab->Extra)		      //redundant but more readable!
						count+=ptab->Extra;   // we have some extra ones so increase how many we create, this is also saved into the next_blah_blah filed but the loader will subtract this off for next_blah_blah

					mem_size=count*ptab->StructSize;
					break;
			}

			if(c0==16)
			{
				struct	PrimFace4PSX	*block;
				SLONG	index;
				// special PSX primface4
				DebugText(" prim face 4 psx = %d was %d\n",count*sizeof(struct PrimFace4PSX),mem_size);
				mem_size=count*sizeof(struct PrimFace4PSX);
				block=(struct PrimFace4PSX*)MemAlloc(mem_size);

				for(index=0;index<*ptab->CountW;index++)
				{
					SLONG	page;
					SLONG	i,j;
					block[index].Points[0]=prim_faces4[index].Points[0];
					block[index].Points[1]=prim_faces4[index].Points[1];
					block[index].Points[2]=prim_faces4[index].Points[2];
					block[index].Points[3]=prim_faces4[index].Points[3];

					for(j=0;j<4;j++)
					for(i=0;i<2;i++)
					{
						block[index].UV[j][i]=prim_faces4[index].UV[j][i];
					}
					block[index].FaceFlags=prim_faces4[index].FaceFlags;
					block[index].DrawFlags=prim_faces4[index].DrawFlags;

					page   = prim_faces4[index].UV[0][0] & 0xc0;
					page <<= 2;
					page  |= prim_faces4[index].TexturePage;

					block[index].ThingIndex=prim_faces4[index].ThingIndex;
					if (prim_faces4[index].FaceFlags & FACE_FLAG_WALKABLE)
					{
						block[index].TexturePage=prim_faces4[index].WALKABLE;
					}
					else
					{
						block[index].TexturePage=page;
					}
					block[index].AltPal=0;
					fix_psx_face4(&block[index]);

				}
				// chunk id
				FileWrite(handle,(UBYTE*)&c0,4);
				FileWrite(handle,(UBYTE*)&count,4);




				{
					ULONG	size=sizeof(struct PrimFace4PSX);
					FileWrite(handle,(UBYTE*)&size,4);
				}
				FileWrite(handle,(UBYTE*)&mem_size,4);
				if(mem_size&3)
					FileWrite(handle,(UBYTE*)&mem_size,4-(mem_size&3));


				FileWrite(handle,(UBYTE*)block,mem_size);
				MemFree(block);
				


			}
			else
			if(c0==17)
			{
				struct	PrimFace3PSX	*block;
				SLONG	index;
				// special PSX primface4
				DebugText(" prim face3 psx = %d was %d\n",count*sizeof(struct PrimFace3PSX),mem_size);
				mem_size=count*sizeof(struct PrimFace3PSX);
				block=(struct PrimFace3PSX*)MemAlloc(mem_size);

				for(index=0;index<*ptab->CountW;index++)
				{
					SLONG	page;
					SLONG	i,j;
					block[index].Points[0]=prim_faces3[index].Points[0];
					block[index].Points[1]=prim_faces3[index].Points[1];
					block[index].Points[2]=prim_faces3[index].Points[2];

					for(j=0;j<3;j++)
					for(i=0;i<2;i++)
					{
						block[index].UV[j][i]=prim_faces3[index].UV[j][i];
					}
					block[index].FaceFlags=prim_faces3[index].FaceFlags;
					block[index].DrawFlags=prim_faces3[index].DrawFlags;

					page   = prim_faces3[index].UV[0][0] & 0xc0;
					page <<= 2;
					page  |= prim_faces3[index].TexturePage;

					block[index].ThingIndex=prim_faces3[index].ThingIndex;
					if (prim_faces3[index].FaceFlags & FACE_FLAG_WALKABLE)
					{
						block[index].TexturePage=prim_faces3[index].WALKABLE;
					}
					else
						block[index].TexturePage=page;

					block[index].AltPal=0;
					fix_psx_face3(&block[index]);

				}
				// chunk id
				FileWrite(handle,(UBYTE*)&c0,4);
				FileWrite(handle,(UBYTE*)&count,4);
				{
					ULONG	size=sizeof(struct PrimFace3PSX);
					FileWrite(handle,(UBYTE*)&size,4);
				}
				FileWrite(handle,(UBYTE*)&mem_size,4);
				if(mem_size&3)
					FileWrite(handle,(UBYTE*)&mem_size,4-(mem_size&3));


				FileWrite(handle,(UBYTE*)block,mem_size);
				MemFree(block);
			}
			else
			{
				p_mem=(UBYTE*)*ptab->Point;
				
				// chunk id
				FileWrite(handle,(UBYTE*)&c0,4);
				FileWrite(handle,(UBYTE*)&count,4);
				struct_size=ptab->StructSize;
				FileWrite(handle,(UBYTE*)&struct_size,4);
				FileWrite(handle,(UBYTE*)&mem_size,4);
				if(mem_size&3)
					FileWrite(handle,(UBYTE*)&mem_size,4-(mem_size&3));


				FileWrite(handle,(UBYTE*)p_mem,mem_size);
			}

			mem_cumlative+=mem_size;
			if(ptab->CountL)
				DebugText(" %s -> %d   tot %d (%d/%d)\n",ptab->Name,mem_size,mem_cumlative,*ptab->CountL,ptab->Maximum);
			else
			if(ptab->CountW)
				DebugText(" %s -> %d   tot %d (%d/%d)\n",ptab->Name,mem_size,mem_cumlative,*ptab->CountW,ptab->Maximum);
			else
				DebugText(" %s -> %d   tot %d (%d)\n",ptab->Name,mem_size,mem_cumlative,ptab->Maximum);

			c0++;
		}
		DebugText("\n");


		STORE_DATA(PRIMARY_USED);								   
		STORE_DATA(PRIMARY_UNUSED);								   
		STORE_DATA(SECONDARY_USED);								   
		STORE_DATA(SECONDARY_UNUSED);							   
		STORE_DATA(PERSON_COUNT);								   
		STORE_DATA(PERSON_COUNT);								   
		STORE_DATA(ANIMAL_COUNT);								   
		STORE_DATA(CHOPPER_COUNT);								   
		STORE_DATA(PYRO_COUNT);									   
		STORE_DATA(PLAYER_COUNT);								   
		STORE_DATA(PROJECTILE_COUNT);							   
		STORE_DATA(SPECIAL_COUNT);								   
		STORE_DATA(SWITCH_COUNT);								   
		STORE_DATA(PRIMARY_COUNT);								   
		STORE_DATA(SECONDARY_COUNT);							   
		STORE_DATA(GAME_STATE);									   
		STORE_DATA(GAME_TURN);									   
		STORE_DATA(GAME_FLAGS);									   
		STORE_DATA(TEXTURE_set);								   
		STORE_DATA(WMOVE_face_upto);							   
		STORE_DATA(first_walkable_prim_point);					   
		STORE_DATA(number_of_walkable_prim_points);				   
		STORE_DATA(first_walkable_prim_face4);					   
		STORE_DATA(number_of_walkable_prim_faces4);				   
		STORE_DATA(BARREL_sphere_last);							   
		STORE_DATA(track_head);									   
		STORE_DATA(track_tail);									   
		STORE_DATA(track_eob);									   
		STORE_DATA( NIGHT_dlight_free);							   
		STORE_DATA( NIGHT_dlight_used);							   


		STORE_DATA(EWAY_time_accurate);							   
		STORE_DATA(EWAY_time);									   
		STORE_DATA(EWAY_tick);									   
		STORE_DATA(EWAY_cam_active);							   
		STORE_DATA(EWAY_cam_x);									   
		STORE_DATA(EWAY_cam_y);									   
		STORE_DATA(EWAY_cam_z);									   
		STORE_DATA(EWAY_cam_dx);								   
		STORE_DATA(EWAY_cam_dy);								   
		STORE_DATA(EWAY_cam_dz);								   
		STORE_DATA(EWAY_cam_yaw);								   
		STORE_DATA(EWAY_cam_pitch);								   
		STORE_DATA(EWAY_cam_waypoint);							   
		STORE_DATA(EWAY_cam_target);							   
		STORE_DATA(EWAY_cam_delay);								   
		STORE_DATA(EWAY_cam_speed);								   
		STORE_DATA(EWAY_cam_freeze);							   
		STORE_DATA(EWAY_cam_cant_interrupt);
		STORE_DATA( NIGHT_amb_d3d_colour);						   
		STORE_DATA( NIGHT_amb_d3d_specular);					   
		STORE_DATA( NIGHT_amb_red);								   
		STORE_DATA( NIGHT_amb_green);							   
		STORE_DATA( NIGHT_amb_blue);							   
		STORE_DATA( NIGHT_amb_norm_x);							   
		STORE_DATA( NIGHT_amb_norm_y);							   
		STORE_DATA( NIGHT_amb_norm_z);							   
		STORE_DATA( NIGHT_flag);								   
		STORE_DATA( NIGHT_lampost_radius);						   
		STORE_DATA( NIGHT_lampost_red);							   
		STORE_DATA( NIGHT_lampost_green);						   
		STORE_DATA( NIGHT_lampost_blue);						   
		STORE_DATA( NIGHT_sky_colour);						
		STORE_DATA(padding_byte);
		STORE_DATA(check);					
		STORE_DATA(CRIME_RATE);					
		STORE_DATA(CRIME_RATE_SCORE_MUL);					
		STORE_DATA(MUSIC_WORLD);
		STORE_DATA(BOREDOM_RATE);
		STORE_DATA(world_type);

		STORE_DATA(EWAY_fake_wander_text_normal_index  );
		STORE_DATA(EWAY_fake_wander_text_normal_number );
		STORE_DATA(EWAY_fake_wander_text_guilty_index  );
		STORE_DATA(EWAY_fake_wander_text_guilty_number );
		STORE_DATA(EWAY_fake_wander_text_annoyed_index );
		STORE_DATA(EWAY_fake_wander_text_annoyed_number);
		
		STORE_DATA(GAME_FLAGS);
		
		STORE_DATA(semtex);
		STORE_DATA(estate);
		STORE_DATA(padding_word);


#ifdef	OLD_CAM													  
																  
		CAM_focus=(Thing*)THING_NUMBER(CAM_focus);				  
		STORE_DATA(CAM_focus);									  
		CAM_focus=TO_THING((SLONG)CAM_focus);					  
#endif															  

		switch(type)
		{
			case	0:
				save_whole_anims(handle);								  
				break;

			case	1:
#ifndef	ULTRA_COMPRESSED_ANIMATIONS
				save_whole_anims_psx(handle);								  
#endif
				break;


		}

			
																  
																  
//		STORE_DATA(GAME_TIME,sizeof								  
//		STORE_DATA(GAME_SEASON,size

		FileClose(handle);

// convert back again
		convert_index_to_pointers();
	}


}
void	save_whole_game(CBYTE	*gamename)
{
	SLONG	level;
	CBYTE	name[30];
//	return;
extern	SLONG	save_psx;

#ifndef	ULTRA_COMPRESSED_ANIMATIONS
	if(save_psx)
	{
#if 0
void WMOVE_remove(UBYTE which_class);
		WMOVE_remove(CLASS_VEHICLE);
#endif
//		save_whole_wad(gamename,0);

		//
		// creates texture pages for playstation with just the prim/people textures that are used
		//

		//
		// Also adjusts the face3 and face4 texturepage numbers to the new location
		//
SLONG	build_tims_ingame(CBYTE *name);

		level=build_tims_ingame(gamename);

		sprintf(name,"c:\\levels\\%d\\%d.nad",level,level);


		gamename[strlen(gamename)-3]='n';
		save_whole_wad(name,1);
#ifdef TARGET_DC
		// Hmmmm.. this has trouble copiling on the DC.
		// shouldn't be using it, anyway.
		ASSERT(FALSE);
#else
		CopyFile(name,gamename,0);
#endif

		fix_psxed_anims();
	}
#endif

}

#endif

extern	SLONG	person_normal_animate(Thing *p_person);

void	convert_drawtype_to_pointer(Thing *p_thing,SLONG meshtype)
{
	switch(meshtype)
	{
		case	DT_MESH:
//			if(p_thing->Draw.Mesh)
			{
				DrawMesh	*drawtype;
				drawtype=TO_DRAW_MESH((ULONG)p_thing->Draw.Mesh);
				p_thing->Draw.Mesh=drawtype;
				drawtype->Cache=0;

			}
		break;
		case	DT_ROT_MULTI:
		case	DT_ANIM_PRIM:
		case	DT_BIKE:
//			if(p_thing->Draw.Tweened)
			{
				DrawTween	*drawtype;
				SLONG	chunk;

				drawtype=TO_DRAW_TWEEN((ULONG)p_thing->Draw.Tweened);
				ASSERT((ULONG)(p_thing->Draw.Tweened)<RMAX_DRAW_TWEENS);
				p_thing->Draw.Tweened=drawtype;

				chunk=(SLONG)drawtype->TheChunk;
				switch(chunk>>16)
				{
					case	1:
						drawtype->TheChunk=&game_chunk[chunk&0xffff];
						break;
					case	2:
						drawtype->TheChunk=&anim_chunk[chunk&0xffff];
						break;
					default:
						ASSERT(0);
						break;
				}
				
				switch(p_thing->Class)
				{
					case	CLASS_PERSON:
						drawtype->QueuedFrame=0;
						drawtype->InterruptFrame=0;
						if(p_thing->Genus.Person->PersonType==PERSON_CIV && (drawtype->CurrentAnim>100 && drawtype->CurrentAnim<140)&&drawtype->CurrentAnim!=109)
						{
							drawtype->CurrentFrame=game_chunk[ANIM_TYPE_CIV].AnimList[drawtype->CurrentAnim];
						}
						else
						if(p_thing->Genus.Person->PersonType==PERSON_COP && (drawtype->CurrentAnim>200 && drawtype->CurrentAnim<220))
						{
							drawtype->CurrentFrame=game_chunk[ANIM_TYPE_ROPER].AnimList[drawtype->CurrentAnim];
						}
						else
							drawtype->CurrentFrame=global_anim_array[p_thing->Genus.Person->AnimType][drawtype->CurrentAnim];


						if(drawtype->CurrentFrame)
							drawtype->NextFrame=drawtype->CurrentFrame->NextFrame;
						else
							drawtype->NextFrame=0;

						if(p_thing->State==STATE_DEAD)
						{
							SLONG	c0;
							SLONG	old_tick;

							old_tick=TICK_RATIO;

							the_game.TickRatio=1<<TICK_SHIFT;
							for(c0=0;c0<40;c0++)
								person_normal_animate(p_thing);

							the_game.TickRatio=old_tick;
						}
						break;
					case	CLASS_BAT:

					case	CLASS_BIKE:
					case	CLASS_ANIM_PRIM:
						drawtype->QueuedFrame=0;
						drawtype->InterruptFrame=0;

//						load_anim_prim_object(p_thing->Index); // make sure the anim prim is loaded, wont load it if it's already in
//
						//
						// bat's dont set up index right
						//
						drawtype->CurrentFrame		=	anim_chunk[p_thing->Index].AnimList[drawtype->CurrentAnim];
						if(drawtype->CurrentFrame)
							drawtype->NextFrame=drawtype->CurrentFrame->NextFrame;
						else
							drawtype->NextFrame=0;

						break;

					case	CLASS_ANIMAL:
						drawtype->QueuedFrame=0;
						drawtype->InterruptFrame=0;
						drawtype->CurrentFrame		=	game_chunk[6].AnimList[1];
						if(drawtype->CurrentFrame)
							drawtype->NextFrame=drawtype->CurrentFrame->NextFrame;
						else
							drawtype->NextFrame=0;
						break;

					default:
						ASSERT(0);
						drawtype->QueuedFrame=0;
						drawtype->InterruptFrame=0;
						drawtype->CurrentFrame=0;
						drawtype->NextFrame=0;
						break;

				}
			}
			break;
	}
}


//
// what's left is function pointers
// and 


extern	void	process_hardware_level_input_for_player(Thing *p_thing);
extern	void	fn_anim_prim_normal(Thing *p_thing);

void	convert_thing_to_pointer(Thing *p_thing)
{

	switch(p_thing->Class)
	{

		case	CLASS_NONE:
			break;
		case	CLASS_PLAYER:
			p_thing->Genus.Player=(Player*)TO_PLAYER((SLONG)p_thing->Genus.Player);
			p_thing->Genus.Player->PlayerPerson=(Thing*)TO_THING((SLONG)p_thing->Genus.Player->PlayerPerson);
			p_thing->StateFn		=	process_hardware_level_input_for_player; // Bodge for now

			break;
		case	CLASS_CAMERA:
			break;
		case	CLASS_PROJECTILE:
			p_thing->Genus.Projectile=(Projectile *)TO_PROJECTILE((SLONG)p_thing->Genus.Projectile);
			break;
		case	CLASS_BUILDING:
			break;
		case	CLASS_PERSON:
			p_thing->Genus.Person=(Person *)TO_PERSON((SLONG)p_thing->Genus.Person);
			set_generic_person_state_function(p_thing,p_thing->State);
			break;
		case	CLASS_ANIMAL:
			p_thing->Genus.Animal=(Animal*)TO_ANIMAL((SLONG)p_thing->Genus.Animal);

			set_state_function(p_thing, p_thing->State);

			break;
		case	CLASS_FURNITURE:
			p_thing->Genus.Furniture=(Furniture*)TO_FURNITURE((SLONG)p_thing->Genus.Furniture);
			break;
		case	CLASS_SWITCH:
			p_thing->Genus.Switch=(Switch*)TO_SWITCH((SLONG)p_thing->Genus.Switch);
			break;
		case	CLASS_VEHICLE:
			p_thing->Genus.Vehicle=(Vehicle*)TO_VEHICLE((SLONG)p_thing->Genus.Vehicle);
			set_state_function(p_thing, p_thing->State);
			break;
		case	CLASS_SPECIAL:
			p_thing->Genus.Special=(Special*)TO_SPECIAL((SLONG)p_thing->Genus.Special);
void special_normal(Thing *s_thing);
			p_thing->StateFn		 = special_normal;

			break;
#ifdef	ANIM_PRIM
		case	CLASS_ANIM_PRIM:
			if(p_thing->StateFn)
			{
				p_thing->StateFn		 = fn_anim_prim_normal;

			}
			break;
#endif
		case	CLASS_CHOPPER:
void CHOPPER_fn_normal(Thing *);
			p_thing->Genus.Chopper=(Chopper*)TO_CHOPPER((SLONG)p_thing->Genus.Chopper);
			p_thing->StateFn    = CHOPPER_fn_normal;
	
			break;
		case	CLASS_PYRO:
			p_thing->Genus.Pyro=(Pyro*)TO_PYRO((SLONG)p_thing->Genus.Pyro);
			set_state_function(p_thing, p_thing->State);
			break;
		case	CLASS_TRACK:
			p_thing->Genus.Track=(Track*)TO_TRACK((SLONG)p_thing->Genus.Track);
			break;
		case	CLASS_PLAT:
			p_thing->Genus.Plat=(Plat*)TO_PLAT((SLONG)p_thing->Genus.Plat);
			p_thing->StateFn    = PLAT_process;
			break;
		case	CLASS_BARREL:
void BARREL_process_normal(Thing *p_barrel);
			p_thing->Genus.Barrel=(Barrel*)TO_BARREL((SLONG)p_thing->Genus.Barrel);
			p_thing->StateFn      = BARREL_process_normal;

			break;

		#ifdef BIKE
		case	CLASS_BIKE:
			void BIKE_process_normal(Thing *p_bike);
			p_thing->StateFn      = BIKE_process_normal;
			p_thing->Genus.Bike=(BIKE_Bike*)TO_BIKE((SLONG)p_thing->Genus.Bike);
			break;
		#endif

		case	CLASS_BAT:
			p_thing->StateFn      = BAT_normal;
			p_thing->Genus.Bat=(Bat*)TO_BAT((SLONG)p_thing->Genus.Bat);
			break;

		default:
			ASSERT(0);
			break;
	}
	switch(p_thing->DrawType)
	{
		case	DT_MESH:
		case	DT_CHOPPER:
			convert_drawtype_to_pointer(p_thing,DT_MESH);
			break;
		case	DT_ROT_MULTI:
			convert_drawtype_to_pointer(p_thing,DT_ROT_MULTI);
			break;
		case	DT_ANIM_PRIM:
		case	DT_BIKE:
			convert_drawtype_to_pointer(p_thing,DT_ANIM_PRIM);
			break;
	}



}


void	convert_index_to_pointers(void)
{
	SLONG	c0;
	for(c0=0;c0<MAX_THINGS;c0++)
	{
		convert_thing_to_pointer(TO_THING(c0));
	}

	for(c0=0;c0<MAX_PLAYERS;c0++)
	{
		NET_PERSON(c0)=TO_THING((SLONG)NET_PERSON(c0));
		NET_PLAYER(c0)=TO_THING((SLONG)NET_PLAYER(c0));
	}

	for(c0=0;c0<EWAY_mess_upto;c0++)
	{
		EWAY_mess[c0]=(CBYTE*)&EWAY_mess_buffer[(SLONG)EWAY_mess[c0]];
	}

	for(c0=0;c0<MAX_PYROS;c0++)
	{
		if(TO_PYRO(c0)->thing)
			TO_PYRO(c0)->thing=TO_THING((ULONG)TO_PYRO(c0)->thing);

		if(TO_PYRO(c0)->victim)
			TO_PYRO(c0)->victim=TO_THING((ULONG)TO_PYRO(c0)->victim);
	}
#ifndef PSX
	// cutscene stuff. convert the track pointers back first:
	for(c0=0;c0<PLAYCUTS_cutscene_ctr;c0++)
	{
		PLAYCUTS_cutscenes[c0].channels=PLAYCUTS_tracks+(SLONG)PLAYCUTS_cutscenes[c0].channels;
	}
	for(c0=0;c0<PLAYCUTS_track_ctr;c0++)
	{
		PLAYCUTS_tracks[c0].packets=PLAYCUTS_packets+(SLONG)PLAYCUTS_tracks[c0].packets;
	}
	for(c0=0;c0<PLAYCUTS_packet_ctr;c0++)
	{
//		if (PLAYCUTS_packets[c0].type==PT_TEXT) PLAYCUTS_packets[c0].pos.X+=PLAYCUTS_text_data;
		if (PLAYCUTS_packets[c0].type==5) PLAYCUTS_packets[c0].pos.X+=(ULONG)PLAYCUTS_text_data;
	}
#endif
}

void	uncache(void)
{
	SLONG	c0;
void NIGHT_destroy_all_cached_info();
	NIGHT_destroy_all_cached_info();

	for(c0=0;c0<next_dfacet;c0++)
	{
		dfacets[c0].Dfcache=0;
	}
}

#define	GET_DATA(d)	memcpy((UBYTE*)&d,p_all,sizeof(d));p_all+=sizeof(d)

void	load_whole_anims(UBYTE	*p_all)
{
	SLONG	c0,c1;
	SLONG	dummy;
	SLONG	check;


	GET_DATA(next_game_chunk);
	GET_DATA(next_anim_chunk);

	for(c0=0;c0<next_game_chunk;c0++)
	{
		GET_DATA(dummy);
		if(dummy>=0)
			ASSERT(dummy==c0);

		if(dummy==c0)
		{
			struct	 GameKeyFrameChunk	*gc;
			gc=&game_chunk[c0];
			GET_DATA(gc->MaxPeopleTypes);
			GET_DATA(gc->MaxKeyFrames);
			GET_DATA(gc->MaxAnimFrames);
			GET_DATA(gc->MaxFightCols);
			GET_DATA(gc->MaxElements);
			GET_DATA(gc->ElementCount);

			GET_DATA(check);
			ASSERT(check==666);

			for(c1=0;c1<10;c1++)
			{
				GET_DATA(gc->MultiObject[c1]);
			}

			GET_DATA(check);
			ASSERT(check==666);
			gc->PeopleTypes=(struct BodyDef*)p_all;
			p_all+=gc->MaxPeopleTypes*sizeof(struct BodyDef);

			GET_DATA(check);
			ASSERT(check==666);
			gc->AnimKeyFrames=(GameKeyFrame*)p_all;
			p_all+=gc->MaxKeyFrames*sizeof(GameKeyFrame);


			GET_DATA(check);
			ASSERT(check==666);
			gc->AnimList=(GameKeyFrame**)p_all;
			p_all+=gc->MaxAnimFrames*sizeof(GameKeyFrame*);


			GET_DATA(check);
			ASSERT(check==666);
			gc->TheElements=(GameKeyFrameElement*)p_all;
			p_all+=gc->MaxElements*sizeof(GameKeyFrameElement);

			GET_DATA(check);
			ASSERT(check==666);
			gc->FightCols=(GameFightCol*)p_all;
			p_all+=gc->MaxFightCols*sizeof(GameFightCol);

			GET_DATA(check);
			ASSERT(check==666);

			convert_keyframe_to_pointer(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_pointer(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_pointer(gc->FightCols,gc->FightCols,gc->MaxFightCols);


		}

	}

	for(c0=0;c0<next_anim_chunk;c0++)
	{
		GET_DATA(dummy);
		if(dummy>=0)
			ASSERT(dummy==c0);

		if(dummy==c0)
		{
			struct	 GameKeyFrameChunk	*gc;
			gc=&anim_chunk[c0];
			GET_DATA(gc->MaxPeopleTypes);
			GET_DATA(gc->MaxKeyFrames);
			GET_DATA(gc->MaxAnimFrames);
			GET_DATA(gc->MaxFightCols);
			GET_DATA(gc->MaxElements);
			GET_DATA(gc->ElementCount);

			for(c1=0;c1<10;c1++)
			{
				GET_DATA(gc->MultiObject[c1]);
			}

			gc->PeopleTypes=(struct BodyDef*)p_all;
			p_all+=gc->MaxPeopleTypes*sizeof(struct BodyDef);

			gc->AnimKeyFrames=(GameKeyFrame*)p_all;
			p_all+=gc->MaxKeyFrames*sizeof(GameKeyFrame);


			gc->AnimList=(GameKeyFrame**)p_all;
			p_all+=gc->MaxAnimFrames*sizeof(GameKeyFrame*);

			gc->TheElements=(GameKeyFrameElement*)p_all;
			p_all+=gc->MaxElements*sizeof(GameKeyFrameElement);

			gc->FightCols=(GameFightCol*)p_all;
			p_all+=gc->MaxFightCols*sizeof(GameFightCol);

			convert_keyframe_to_pointer(gc->AnimKeyFrames,gc->TheElements,gc->FightCols,gc->MaxKeyFrames);
			convert_animlist_to_pointer(gc->AnimList,gc->AnimKeyFrames,gc->MaxAnimFrames);
			convert_fightcol_to_pointer(gc->FightCols,gc->FightCols,gc->MaxFightCols);


		}
	}
	GET_DATA(next_anim_mids);
	anim_mids=(PrimPoint *)p_all;
	p_all+=next_anim_mids*sizeof(PrimPoint);

}

extern SLONG EWAY_count_up;		 // The visible count-up timer...

//
// The penalties incurred for the count-up timer.
// 

extern UBYTE EWAY_count_up_add_penalties;
extern SWORD EWAY_count_up_num_penalties;
extern UWORD EWAY_count_up_penalty_timer;

void	flag_v_faces(void)
{
	SLONG	c0;
	struct	PrimFace4 *p4;
	struct	PrimFace3 *p3;
	SLONG	vx,vz,wx,wz,ny;

	p4=&prim_faces4[0];
	for(c0=0;c0<next_prim_face4;c0++)
	{
		vx=prim_points[p4->Points[1]].X-prim_points[p4->Points[0]].X;
		vz=prim_points[p4->Points[1]].Z-prim_points[p4->Points[0]].Z;

		wx=prim_points[p4->Points[0]].X-prim_points[p4->Points[2]].X;
		wz=prim_points[p4->Points[0]].Z-prim_points[p4->Points[2]].Z;


		ny=((vz)*(wx))-(vx*wz)>>8;  //perform cross product on vect V & W

		if(ny<2)
			p4->FaceFlags|=FACE_FLAG_VERTICAL;

		p4++;
	}

	p3=&prim_faces3[0];
	for(c0=0;c0<next_prim_face3;c0++)
	{
		vx=prim_points[p3->Points[1]].X-prim_points[p3->Points[0]].X;
		vz=prim_points[p3->Points[1]].Z-prim_points[p3->Points[0]].Z;

		wx=prim_points[p3->Points[0]].X-prim_points[p3->Points[2]].X;
		wz=prim_points[p3->Points[0]].Z-prim_points[p3->Points[2]].Z;


		ny=((vz)*(wx))-(vx*wz)>>8;  //perform cross product on vect V & W

		if(ny<2)
			p3->FaceFlags|=FACE_FLAG_VERTICAL;

		p3++;
	}
}
#ifndef	NEW_LEVELS
UWORD	EWAY_timer_bodge[EWAY_MAX_TIMERS];
#endif

void	load_whole_game(CBYTE	*gamename)
{
//	return;

	SLONG	c0=0,id;
	SLONG	*p_slong;
	UWORD	*p_uword;
	UBYTE	*p_mem,*p_all;
	SLONG	mem_size;
	struct	MemTable *ptab;
#ifndef PSX
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
#else
	SLONG			handle;
#endif
	SLONG	count;
	SLONG	texture_set_local;
	SLONG	save_type;

	SLONG	check;
	UBYTE	padding_byte;
	UWORD	padding_word;

	SetSeed(1234567);
	srand(1234567);
#ifndef	NEW_LEVELS
	memset((UBYTE*)EWAY_timer_bodge,0,EWAY_MAX_TIMERS*2);
#endif

#ifdef PSX

extern void SetDisplayClear(SLONG clear);
extern void AENG_loading_bar(SLONG percent);
extern	void OB_init();
	OB_init(); //ob_hydrant now cleared

	SetDisplayClear(0);

extern int wad_level;
extern	UBYTE	roper_pickup;

	if(wad_level==21)
	{
		roper_pickup=2;
	}
	else
	if(wad_level==33)
	{
		roper_pickup=1;
	}
	else
	{
		roper_pickup=0;
	}

	//
	// Make sure when we go into the game, the draw stuff is set up
	//
#if 1
	GDisp_SetupBucketMem(&my_heap[8],16384);
#endif
void AENG_flip_init(void);
	AENG_flip_init();
#endif

extern	UWORD	player_dlight;
	player_dlight=0;

#ifdef	ULTRA_COMPRESSED_ANIMATIONS
	gamename[strlen(gamename)-3]='n';
#endif

	if(mem_all)
		MemFree(mem_all);

#ifndef PSX
	handle=FileOpen(gamename);
	if(handle==FILE_OPEN_ERROR)
	{
		ASSERT(0);
		return;
	}

	mem_size=FileSize(handle);
	p_all=(UBYTE*)MemAlloc(mem_size);
	ASSERT(p_all);
	FileRead(handle,p_all,mem_size);
	FileClose(handle);
#else

	TEXTURE_choose_set(wad_level);

	AENG_loading_bar(20);

#ifndef FS_ISO9660
	handle=PCopen(gamename,0,0);
	if (handle<0)
	{
		ASSERT(0);
		return;
	}
	mem_size=PClseek(handle,0,SEEK_END);
	PClseek(handle,0,SEEK_SET);
	p_all=(UBYTE*)MemAlloc(mem_size);
	PCread(handle,p_all,mem_size);
	PCclose(handle);
	// Calculate how much bucket space is available
#if 1
	BUCKET_MEM=(total_mem_size-(mem_size+128))&0xfffffffc;
#endif

	AENG_loading_bar(40);
#else
extern char cd_file_buffer[];

	CdlFILE cfile;
	char *p=gamename;
	while(*p)
		*p++=toupper(*p);
	sprintf(cd_file_buffer,"\\%s;1",gamename);

	if (CdSearchFile(&cfile,cd_file_buffer)==0)
	{
//		printf("Error: Cannot find level file '%s'\n",cd_file_buffer);
	}
	mem_size=(cfile.size+2047)&0x7ffff800;
	p_all=(UBYTE*)MemAlloc(mem_size);
	CdReadFile(cd_file_buffer,(ULONG*)p_all,mem_size);
	CdReadSync(0,cd_file_buffer);
#if 1
	BUCKET_MEM=(total_mem_size-(mem_size+128))&0xfffffffc;
#endif
	AENG_loading_bar(40);

#ifdef VERSION_PAL
#if !defined(MIKE)&&!defined(VERSION_DEMO)
	if (do_decrypt[wad_level-1])
		Decrypt(p_all);
#endif
#endif

#endif	
#endif

	mem_all=p_all;
	mem_all_size=mem_size;



	GET_DATA(save_type);



//	printf("Save Type: %d\n",save_type);
	while(save_table[c0].Point)
	{
		SLONG	struct_size;
//		printf("Loading: %s\n",save_table[c0].Name);
		id=*((SLONG*)p_all);
//		printf(" id %d ",id);
		ASSERT(id==c0);
		p_all+=4;

		count=*((SLONG*)p_all);
//		printf(" count %d",count);
		p_all+=4;

		struct_size=*((SLONG*)p_all);
		p_all+=4;
//		printf(" struct size %d shouldbe %d memsize %d\n",struct_size,save_table[c0].StructSize,*(SLONG*)p_all);
		ASSERT(struct_size==save_table[c0].StructSize);

		mem_size=*((SLONG*)p_all);
		p_all+=4;

		if(mem_size&3)
			p_all+=4-(mem_size&3);

		ptab=&save_table[c0];

		*ptab->Point=p_all;

		switch(ptab->Type)
		{
			case	2:
				if(ptab->Extra)
					ptab->Maximum=count;
				
				break;

			case	1:
				if(ptab->Extra)
					count-=ptab->Extra;

				if(ptab->CountL)
				{
					*ptab->CountL=count;
				}
				else
				{
					if(ptab->CountW)
						*ptab->CountW=count;
				}
				break;
		}

		p_all+=mem_size;

		c0++;
	}

	for(c0=0;c0<EWAY_mess_buffer_upto;c0++)
		if (EWAY_mess_buffer[c0]==160) EWAY_mess_buffer[c0]=32;

#ifdef PSX
	AENG_loading_bar(60);
#endif

		GET_DATA(PRIMARY_USED);								   
		GET_DATA(PRIMARY_UNUSED);								   
		GET_DATA(SECONDARY_USED);								   
		GET_DATA(SECONDARY_UNUSED);							   
		GET_DATA(PERSON_COUNT);								   
		GET_DATA(PERSON_COUNT);								   
		GET_DATA(ANIMAL_COUNT);								   
		GET_DATA(CHOPPER_COUNT);								   
		GET_DATA(PYRO_COUNT);									   
		GET_DATA(PLAYER_COUNT);								   
		GET_DATA(PROJECTILE_COUNT);							   
		GET_DATA(SPECIAL_COUNT);								   
		GET_DATA(SWITCH_COUNT);								   
		GET_DATA(PRIMARY_COUNT);								   
		GET_DATA(SECONDARY_COUNT);							   
		GET_DATA(GAME_STATE);									   

#ifdef TARGET_DC
		GAME_STATE &= ~(GS_RECORD|GS_PLAYBACK|GS_EDITOR);
#endif

		GET_DATA(GAME_TURN);									   
		GET_DATA(GAME_FLAGS);									   
		GET_DATA(texture_set_local);								   
		GET_DATA(WMOVE_face_upto);							   
		GET_DATA(first_walkable_prim_point);					   
		GET_DATA(number_of_walkable_prim_points);				   
		GET_DATA(first_walkable_prim_face4);					   
		GET_DATA(number_of_walkable_prim_faces4);				   
		GET_DATA(BARREL_sphere_last);							   
		GET_DATA(track_head);									   
		GET_DATA(track_tail);									   
		GET_DATA(track_eob);									   
		GET_DATA( NIGHT_dlight_free);							   
		GET_DATA( NIGHT_dlight_used);							   


		GET_DATA(EWAY_time_accurate);							   
		GET_DATA(EWAY_time);									   
		GET_DATA(EWAY_tick);									   
		GET_DATA(EWAY_cam_active);							   
		GET_DATA(EWAY_cam_x);									   
		GET_DATA(EWAY_cam_y);									   
		GET_DATA(EWAY_cam_z);									   
		GET_DATA(EWAY_cam_dx);								   
		GET_DATA(EWAY_cam_dy);								   
		GET_DATA(EWAY_cam_dz);								   
		GET_DATA(EWAY_cam_yaw);								   
		GET_DATA(EWAY_cam_pitch);								   
		GET_DATA(EWAY_cam_waypoint);							   
		GET_DATA(EWAY_cam_target);							   
		GET_DATA(EWAY_cam_delay);								   
		GET_DATA(EWAY_cam_speed);								   
		GET_DATA(EWAY_cam_freeze);							   
		GET_DATA(EWAY_cam_cant_interrupt);
		GET_DATA(NIGHT_amb_d3d_colour);
		GET_DATA(NIGHT_amb_d3d_specular);
		GET_DATA(NIGHT_amb_red);
		GET_DATA(NIGHT_amb_green);
		GET_DATA(NIGHT_amb_blue);
		GET_DATA(NIGHT_amb_norm_x);							   
		GET_DATA(NIGHT_amb_norm_y);							   
		GET_DATA(NIGHT_amb_norm_z);							   
		GET_DATA(NIGHT_flag);								   
		GET_DATA(NIGHT_lampost_radius);						   
		GET_DATA(NIGHT_lampost_red);							   
		GET_DATA(NIGHT_lampost_green);						   
		GET_DATA(NIGHT_lampost_blue);						   
		GET_DATA(NIGHT_sky_colour);						
		GET_DATA(padding_byte);
		GET_DATA(check);										  
		GET_DATA(CRIME_RATE);					
		GET_DATA(CRIME_RATE_SCORE_MUL);					
		GET_DATA(MUSIC_WORLD);
		GET_DATA(BOREDOM_RATE);
		GET_DATA(world_type);

		GET_DATA(EWAY_fake_wander_text_normal_index  );
		GET_DATA(EWAY_fake_wander_text_normal_number );
		GET_DATA(EWAY_fake_wander_text_guilty_index  );
		GET_DATA(EWAY_fake_wander_text_guilty_number );
		GET_DATA(EWAY_fake_wander_text_annoyed_index );
		GET_DATA(EWAY_fake_wander_text_annoyed_number);

		GET_DATA(GAME_FLAGS);

		GET_DATA(semtex);
		GET_DATA(estate);
		GET_DATA(padding_word);


	ASSERT(check==666);

	//NIGHT_flag&=~NIGHT_FLAG_DAYTIME;

	// This needs to go here, if it doesn't then the later levels will
	// automatically fail because the timer will be over the time limit.

	EWAY_time_accurate = 0;
	EWAY_time          = 0;
	EWAY_count_up      = 0;

	//
	// The penalties on the count-up timer.
	//

	EWAY_count_up_add_penalties = 0;
	EWAY_count_up_num_penalties = 0;
	EWAY_count_up_penalty_timer = 0;

//	NIGHT_amb_red*=2;
//	NIGHT_amb_green*=2;
//	NIGHT_amb_blue*=2;
#ifdef VERSION_USA
	SATURATE(NIGHT_amb_red,25,127);
	SATURATE(NIGHT_amb_green,25,127);



#endif

	SATURATE(NIGHT_amb_blue,25,127);

	
#ifdef VERSION_USA
	{
		SLONG	bright;
		SATURATE(NIGHT_amb_red,25,127);
		SATURATE(NIGHT_amb_green,25,127);

		bright=Root(NIGHT_amb_red*NIGHT_amb_red + NIGHT_amb_green*NIGHT_amb_green+NIGHT_amb_blue*NIGHT_amb_blue);


		if(bright==0)
			bright=1;
#define	MIN_BRIGHT	75
		if(bright<MIN_BRIGHT)
		{
			NIGHT_amb_red=(NIGHT_amb_red*MIN_BRIGHT)/bright;
			NIGHT_amb_green=(NIGHT_amb_green*MIN_BRIGHT)/bright;
			NIGHT_amb_blue=(NIGHT_amb_blue*MIN_BRIGHT)/bright;
		}
	}
#endif



#ifdef	OLD_CAM
	GET_DATA(CAM_focus);
	CAM_focus=TO_THING((SLONG)CAM_focus);
#endif

	//
	// this needs to be before convert_index_to_pointers (because c_i_t_p needs the anims in place)
	//
	load_whole_anims(p_all);

#ifdef PSX
	AENG_loading_bar(80);
	printf("BUCKET_MEM = %d\n",BUCKET_MEM);
#endif

void	setup_global_anim_array(void);
	setup_global_anim_array();
void	init_dead_tween(void);
	init_dead_tween();


	convert_index_to_pointers();

	FC_look_at(0, THING_NUMBER(NET_PERSON(0)));
	FC_force_camera_behind(0);

	uncache();

//	if(texture_set_local!=TEXTURE_set)
	{
#ifndef	PSX
		TEXTURE_choose_set(texture_set_local);
		ASSERT(0);	// you need a filename for TEXTURE_load_needed()
//		TEXTURE_load_needed();
#endif
	}

extern void PARTICLE_Reset();
	PARTICLE_Reset();

extern void POW_init();
	POW_init();

extern	void PCOM_init(void);
	PCOM_init();

#ifdef	PSX
void	init_punch_kick(void);
	init_punch_kick();

#ifndef	NEW_LEVELS
	EWAY_timer=&EWAY_timer_bodge[0];
#endif

#endif

//	calc_prim_info();
	VEH_init_vehinfo();

extern	void	init_noises(void);
extern	void	init_arrest(void);
		init_noises();
		init_arrest();

#ifndef	PSX
	calc_prim_normals();
#else
extern void SetDisplayFade(void);
	SetDisplayFade();
#endif
//	find_anim_prim_bboxes();

#ifndef	PSX
//	TEXTURE_fix_prim_textures();
//	TEXTURE_fix_texture_styles();
	AENG_create_dx_prim_points();
	NIGHT_generate_walkable_lighting();
#else
	AENG_loading_bar(100);
	SetDisplayClear(1);
	AENG_create_dx_prim_points();
	NIGHT_ambient(NIGHT_amb_red,NIGHT_amb_green,NIGHT_amb_blue,NIGHT_amb_norm_x,NIGHT_amb_norm_y,NIGHT_amb_norm_z);
void	init_gangattack(void);

	init_gangattack(); //probably should save these

SBYTE	global_spang_count;
	global_spang_count=0;


extern void Wadmenu_PutStats();
	Wadmenu_PutStats();

extern void Wadmenu_PutStats();
	Wadmenu_PutStats();

#if 0

extern void AENG_ambient_editor(int store);
	AENG_ambient_editor(1);
#endif

	{
		SLONG	c0;
		for(c0=0;c0<100;c0++)
		{
			FC_process();
		}
	}


	#endif

	flag_v_faces();

#ifdef	PSX
	if(wad_level==25)
	{

		TO_THING(193)->Genus.Person->Flags &= ~FLAG_PERSON_USEABLE;
	}
#endif


}




#ifndef PSX
#ifndef TARGET_DC

//
// Quick load\save filename.
// The value we save out at the end to make sure we're okay.
//

#define MEMORY_QUICK_FNAME "data\\quicksave.dat"
#define MEMORY_QUICK_CHECK 314159265


SLONG MEMORY_quick_avaliable;


void MEMORY_quick_init()
{
	FileDelete(MEMORY_QUICK_FNAME);

	MEMORY_quick_avaliable = FALSE;
}

void MEMORY_quick_save()
{
	SLONG i;
	UBYTE padding_byte;
	SLONG check = 666;
	SLONG checksum;

	FILE *handle = MF_Fopen(MEMORY_QUICK_FNAME, "wb");

	if (!handle)
	{
		return;
	}

	//
	// Save out a version number.
	//

	SLONG version = 1;

	if (fwrite(&version, sizeof(SLONG), 1, handle) != 1) goto file_error;

	//
	// Go through the memory table and save all arrays.
	//

	MemTable *mt;

	for (i = 0; save_table[i].Point; i++)
	{
		mt = &save_table[i];

		if (fwrite(mt->Point, mt->StructSize, mt->Maximum, handle) != (unsigned)mt->Maximum) goto file_error;
	}

	//
	// Save out the extra data we might need!
	//

	#define QSTORE_DATA(x) if (fwrite(&(x), sizeof(x), 1, handle) != 1) goto file_error;

	QSTORE_DATA(PRIMARY_USED);								   
	QSTORE_DATA(PRIMARY_UNUSED);								   
	QSTORE_DATA(SECONDARY_USED);								   
	QSTORE_DATA(SECONDARY_UNUSED);							   
	QSTORE_DATA(PERSON_COUNT);								   
	QSTORE_DATA(PERSON_COUNT);								   
	QSTORE_DATA(ANIMAL_COUNT);								   
	QSTORE_DATA(CHOPPER_COUNT);								   
	QSTORE_DATA(PYRO_COUNT);									   
	QSTORE_DATA(PLAYER_COUNT);								   
	QSTORE_DATA(PROJECTILE_COUNT);							   
	QSTORE_DATA(SPECIAL_COUNT);								   
	QSTORE_DATA(SWITCH_COUNT);								   
	QSTORE_DATA(PRIMARY_COUNT);								   
	QSTORE_DATA(SECONDARY_COUNT);							   
	QSTORE_DATA(GAME_STATE);	   
	QSTORE_DATA(GAME_TURN);	
	QSTORE_DATA(GAME_FLAGS);	   
	QSTORE_DATA(TEXTURE_set);
	QSTORE_DATA(WMOVE_face_upto)
	QSTORE_DATA(first_walkable_prim_point);
	QSTORE_DATA(number_of_walkable_prim_points);
	QSTORE_DATA(first_walkable_prim_face4);
	QSTORE_DATA(number_of_walkable_prim_faces4);
	QSTORE_DATA(BARREL_sphere_last);
	QSTORE_DATA(track_head);
	QSTORE_DATA(track_tail);
	QSTORE_DATA(track_eob);
	QSTORE_DATA( NIGHT_dlight_free);
	QSTORE_DATA( NIGHT_dlight_used);

	QSTORE_DATA(EWAY_time_accurate);							   
	QSTORE_DATA(EWAY_time);									   
	QSTORE_DATA(EWAY_tick);									   
	QSTORE_DATA(EWAY_cam_active);							   
	QSTORE_DATA(EWAY_cam_x);									   
	QSTORE_DATA(EWAY_cam_y);									   
	QSTORE_DATA(EWAY_cam_z);									   
	QSTORE_DATA(EWAY_cam_dx);								   
	QSTORE_DATA(EWAY_cam_dy);								   
	QSTORE_DATA(EWAY_cam_dz);								   
	QSTORE_DATA(EWAY_cam_yaw);								   
	QSTORE_DATA(EWAY_cam_pitch);								   
	QSTORE_DATA(EWAY_cam_waypoint);							   
	QSTORE_DATA(EWAY_cam_target);							   
	QSTORE_DATA(EWAY_cam_delay);								   
	QSTORE_DATA(EWAY_cam_speed);								   
	QSTORE_DATA(EWAY_cam_freeze);							   
	QSTORE_DATA(EWAY_cam_cant_interrupt);

	QSTORE_DATA( NIGHT_amb_d3d_colour);						   
	QSTORE_DATA( NIGHT_amb_d3d_specular);					   
	QSTORE_DATA( NIGHT_amb_red);								   
	QSTORE_DATA( NIGHT_amb_green);							   
	QSTORE_DATA( NIGHT_amb_blue);							   
	QSTORE_DATA( NIGHT_amb_norm_x);							   
	QSTORE_DATA( NIGHT_amb_norm_y);							   
	QSTORE_DATA( NIGHT_amb_norm_z);							   
	QSTORE_DATA( NIGHT_flag);								   
	QSTORE_DATA( NIGHT_lampost_radius);						   
	QSTORE_DATA( NIGHT_lampost_red);							   
	QSTORE_DATA( NIGHT_lampost_green);						   
	QSTORE_DATA( NIGHT_lampost_blue);						   
	QSTORE_DATA( NIGHT_sky_colour);						
	QSTORE_DATA(padding_byte);
	QSTORE_DATA(check);					
	QSTORE_DATA(CRIME_RATE);					
	QSTORE_DATA(CRIME_RATE_SCORE_MUL);					
	QSTORE_DATA(MUSIC_WORLD);
	QSTORE_DATA(BOREDOM_RATE);
	QSTORE_DATA(world_type);

	QSTORE_DATA(EWAY_fake_wander_text_normal_index  );
	QSTORE_DATA(EWAY_fake_wander_text_normal_number );
	QSTORE_DATA(EWAY_fake_wander_text_guilty_index  );
	QSTORE_DATA(EWAY_fake_wander_text_guilty_number );
	QSTORE_DATA(EWAY_fake_wander_text_annoyed_index );
	QSTORE_DATA(EWAY_fake_wander_text_annoyed_number);
	
	QSTORE_DATA(GAME_FLAGS);
	
	//
	// Lighting stuff that isn't saved on the PSX, so it isn't in the save table.
	//

	if (fwrite( NIGHT_square,       sizeof(NIGHT_Square),      NIGHT_MAX_SQUARES        , handle) != NIGHT_MAX_SQUARES        ) goto file_error;
	if (fwrite(&NIGHT_square_free,  sizeof(NIGHT_square_free), 1                        , handle) != 1                        ) goto file_error;
	if (fwrite( NIGHT_cache,        sizeof(UBYTE),             PAP_SIZE_LO * PAP_SIZE_LO, handle) != PAP_SIZE_LO * PAP_SIZE_LO) goto file_error;

	if (fwrite( NIGHT_dfcache,      sizeof(NIGHT_Dfcache),     NIGHT_MAX_DFCACHES       , handle) != NIGHT_MAX_DFCACHES       ) goto file_error;
	if (fwrite(&NIGHT_dfcache_used, sizeof(UBYTE),             1                        , handle) != 1                        ) goto file_error;
	if (fwrite(&NIGHT_dfcache_free, sizeof(UBYTE),             1                        , handle) != 1                        ) goto file_error;

	//
	// Make sure we're in the right place.
	//

	checksum = MEMORY_QUICK_CHECK;

	QSTORE_DATA(checksum);

	MF_Fclose(handle);

	MEMORY_quick_avaliable = TRUE;

	return;

  file_error:;

	MF_Fclose(handle);

	FileDelete(MEMORY_QUICK_FNAME);

	MEMORY_quick_avaliable = FALSE;

	return;
}

SLONG MEMORY_quick_load_available(void)
{
	return MEMORY_quick_avaliable;
}


SLONG MEMORY_quick_load()
{
	SLONG i;
	UBYTE padding_byte;
	SLONG check = 666;
	SLONG checksum;

	FILE *handle = MF_Fopen(MEMORY_QUICK_FNAME, "rb");

	if (!handle)
	{
		return FALSE;
	}

	//
	// Save out a version number.
	//

	SLONG version;

	if (fread(&version, sizeof(SLONG), 1, handle) != 1) goto file_error;

	ASSERT(version == 1);

	//
	// Go through the memory table and load all arrays.
	//

	MemTable *mt;

	for (i = 0; save_table[i].Point; i++)
	{
		mt = &save_table[i];

		if (fread(mt->Point, mt->StructSize, mt->Maximum, handle) != (unsigned)mt->Maximum) goto file_error;
	}

	//
	// Save out the extra data we might need!
	//

	#define QREAD_DATA(x) if (fread(&(x), sizeof(x), 1, handle) != 1) goto file_error;

	QREAD_DATA(PRIMARY_USED);								   
	QREAD_DATA(PRIMARY_UNUSED);								   
	QREAD_DATA(SECONDARY_USED);								   
	QREAD_DATA(SECONDARY_UNUSED);							   
	QREAD_DATA(PERSON_COUNT);								   
	QREAD_DATA(PERSON_COUNT);								   
	QREAD_DATA(ANIMAL_COUNT);								   
	QREAD_DATA(CHOPPER_COUNT);								   
	QREAD_DATA(PYRO_COUNT);									   
	QREAD_DATA(PLAYER_COUNT);								   
	QREAD_DATA(PROJECTILE_COUNT);							   
	QREAD_DATA(SPECIAL_COUNT);								   
	QREAD_DATA(SWITCH_COUNT);								   
	QREAD_DATA(PRIMARY_COUNT);								   
	QREAD_DATA(SECONDARY_COUNT);							   
	QREAD_DATA(GAME_STATE);	   

#ifdef TARGET_DC
		GAME_STATE &= ~(GS_RECORD|GS_PLAYBACK|GS_EDITOR);
#endif

	QREAD_DATA(GAME_TURN);	
	QREAD_DATA(GAME_FLAGS);	   
	QREAD_DATA(TEXTURE_set);
	QREAD_DATA(WMOVE_face_upto)
	QREAD_DATA(first_walkable_prim_point);
	QREAD_DATA(number_of_walkable_prim_points);
	QREAD_DATA(first_walkable_prim_face4);
	QREAD_DATA(number_of_walkable_prim_faces4);
	QREAD_DATA(BARREL_sphere_last);
	QREAD_DATA(track_head);
	QREAD_DATA(track_tail);
	QREAD_DATA(track_eob);
	QREAD_DATA( NIGHT_dlight_free);
	QREAD_DATA( NIGHT_dlight_used);

	QREAD_DATA(EWAY_time_accurate);							   
	QREAD_DATA(EWAY_time);									   
	QREAD_DATA(EWAY_tick);									   
	QREAD_DATA(EWAY_cam_active);							   
	QREAD_DATA(EWAY_cam_x);									   
	QREAD_DATA(EWAY_cam_y);									   
	QREAD_DATA(EWAY_cam_z);									   
	QREAD_DATA(EWAY_cam_dx);								   
	QREAD_DATA(EWAY_cam_dy);								   
	QREAD_DATA(EWAY_cam_dz);								   
	QREAD_DATA(EWAY_cam_yaw);								   
	QREAD_DATA(EWAY_cam_pitch);								   
	QREAD_DATA(EWAY_cam_waypoint);							   
	QREAD_DATA(EWAY_cam_target);							   
	QREAD_DATA(EWAY_cam_delay);								   
	QREAD_DATA(EWAY_cam_speed);								   
	QREAD_DATA(EWAY_cam_freeze);							   
	QREAD_DATA(EWAY_cam_cant_interrupt);
	QREAD_DATA( NIGHT_amb_d3d_colour);						   
	QREAD_DATA( NIGHT_amb_d3d_specular);					   
	QREAD_DATA( NIGHT_amb_red);								   
	QREAD_DATA( NIGHT_amb_green);							   
	QREAD_DATA( NIGHT_amb_blue);							   
	QREAD_DATA( NIGHT_amb_norm_x);							   
	QREAD_DATA( NIGHT_amb_norm_y);							   
	QREAD_DATA( NIGHT_amb_norm_z);							   
	QREAD_DATA( NIGHT_flag);								   
	QREAD_DATA( NIGHT_lampost_radius);						   
	QREAD_DATA( NIGHT_lampost_red);							   
	QREAD_DATA( NIGHT_lampost_green);						   
	QREAD_DATA( NIGHT_lampost_blue);						   
	QREAD_DATA( NIGHT_sky_colour);						
	QREAD_DATA(padding_byte);
	QREAD_DATA(check);					
	QREAD_DATA(CRIME_RATE);					
	QREAD_DATA(CRIME_RATE_SCORE_MUL);					
	QREAD_DATA(MUSIC_WORLD);
	QREAD_DATA(BOREDOM_RATE);
	QREAD_DATA(world_type);

	QREAD_DATA(EWAY_fake_wander_text_normal_index  );
	QREAD_DATA(EWAY_fake_wander_text_normal_number );
	QREAD_DATA(EWAY_fake_wander_text_guilty_index  );
	QREAD_DATA(EWAY_fake_wander_text_guilty_number );
	QREAD_DATA(EWAY_fake_wander_text_annoyed_index );
	QREAD_DATA(EWAY_fake_wander_text_annoyed_number);
	
	QREAD_DATA(GAME_FLAGS);

	//
	// Lighting stuff that isn't saved on the PSX, so it isn't in the save table.
	//

	if (fread( NIGHT_square,       sizeof(NIGHT_Square),      NIGHT_MAX_SQUARES        , handle) != NIGHT_MAX_SQUARES        ) goto file_error;
	if (fread(&NIGHT_square_free,  sizeof(NIGHT_square_free), 1                        , handle) != 1                        ) goto file_error;
	if (fread( NIGHT_cache,        sizeof(UBYTE),             PAP_SIZE_LO * PAP_SIZE_LO, handle) != PAP_SIZE_LO * PAP_SIZE_LO) goto file_error;

	if (fread( NIGHT_dfcache,      sizeof(NIGHT_Dfcache),     NIGHT_MAX_DFCACHES       , handle) != NIGHT_MAX_DFCACHES       ) goto file_error;
	if (fread(&NIGHT_dfcache_used, sizeof(UBYTE),             1                        , handle) != 1                        ) goto file_error;
	if (fread(&NIGHT_dfcache_free, sizeof(UBYTE),             1                        , handle) != 1                        ) goto file_error;

	//
	// Make sure we're in the right place.
	//

	QREAD_DATA(checksum);

	if (checksum != MEMORY_QUICK_CHECK)
	{
		goto file_error;
	}

	MF_Fclose(handle);

	NIGHT_cache_recalc();
	NIGHT_dfcache_recalc();
	NIGHT_generate_walkable_lighting();

	return TRUE;

  file_error:;

	MF_Fclose(handle);

	return FALSE;
}


#endif
#endif


#ifndef TARGET_DC
#if 1 // TEST_DC

//
// Dreamcast load/save games...
//

void save_dreamcast_wad(CBYTE *fname)
{

	SLONG	c0=0;
	SLONG	*p_slong;
	UWORD	*p_uword;
	UBYTE	*p_mem;
	SLONG	mem_size,mem_cumlative=0;
	struct	MemTable *ptab;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	SLONG	count;
	SLONG	save_type=0;
	SLONG	check=666;
	UBYTE	padding_byte;
	UWORD	padding_word;
	ULONG	struct_size;

	//
	// The dreamcast doesn't want darci_normals...
	//

	darci_normal_count = 0;

	//
	// remove anything that can't survive the reload (transient effects)
	//


	// Sets up level_index for special fudges.
extern SLONG get_level_no ( CBYTE *name );
	get_level_no(fname);


	// Add a full pathname.

	char cFullName[100] = "c:\\fallen\\";
	strcat ( cFullName, fname );

	// Change the extension to ".dad"
	int iLastChar = strlen ( cFullName );
	ASSERT ( cFullName[iLastChar-4] == '.' );
	ASSERT ( cFullName[iLastChar-3] == 'u' );
	ASSERT ( cFullName[iLastChar-2] == 'c' );
	ASSERT ( cFullName[iLastChar-1] == 'm' );

	cFullName[iLastChar-3] = 'd';
	cFullName[iLastChar-2] = 'a';
	cFullName[iLastChar-1] = 'd';

	TRACE ( "Writing out level <%s>", cFullName );

	handle = FileCreate(cFullName, 1);

	if(handle!=FILE_CREATION_ERROR) 
	{
		// get rid of the nasty pointers
		// by converting to indexes

		convert_pointers_to_index();

		DebugText("\n SAVE INGAME \n");

		STORE_DATA(save_type);

		while(save_table[c0].Point)
		{
			mem_size=0;
			count=0;
//			ASSERT(c0!=29);
			ptab=&save_table[c0];
			switch(ptab->Type)
			{
				case	MEM_STATIC:
					mem_size=ptab->Maximum*ptab->StructSize;
					count=ptab->Maximum;
					break;

				case	MEM_DYNAMIC:
					if(ptab->CountL)
					{
						count=*ptab->CountL;
					}
					else
					{
						if(ptab->CountW)
				
							count=*ptab->CountW;
					}
					if(ptab->Extra)				// redundant but more readable!
						count+=ptab->Extra;		// we have some extra ones so increase how many we create,
												// this is also saved into the next_blah_blah filed but the
												// loader will subtract this off for next_blah_blah

					mem_size=count*ptab->StructSize;
					break;

				default:
					ASSERT(0);
					break;
			}


			p_mem=(UBYTE*)*ptab->Point;
			
			// chunk id
			FileWrite(handle,(UBYTE*)&c0,4);
			FileWrite(handle,(UBYTE*)&count,4);
			struct_size=ptab->StructSize;
			FileWrite(handle,(UBYTE*)&struct_size,4);
			FileWrite(handle,(UBYTE*)&mem_size,4);
			if(mem_size&3)
				FileWrite(handle,(UBYTE*)&mem_size,4-(mem_size&3));

			FileWrite(handle,(UBYTE*)p_mem,mem_size);

			mem_cumlative+=mem_size;

			if(ptab->CountL)
				DebugText(" %s -> %d   tot %d (%d/%d)\n",ptab->Name,mem_size,mem_cumlative,*ptab->CountL,ptab->Maximum);
			else
			if(ptab->CountW)
				DebugText(" %s -> %d   tot %d (%d/%d)\n",ptab->Name,mem_size,mem_cumlative,*ptab->CountW,ptab->Maximum);
			else
				DebugText(" %s -> %d   tot %d (%d)\n",ptab->Name,mem_size,mem_cumlative,ptab->Maximum);

			c0++;
		}
		DebugText("\n");

		STORE_DATA(PRIMARY_USED);								   
		STORE_DATA(PRIMARY_UNUSED);								   
		STORE_DATA(SECONDARY_USED);								   
		STORE_DATA(SECONDARY_UNUSED);							   
		STORE_DATA(PERSON_COUNT);								   
		STORE_DATA(PERSON_COUNT);								   
		STORE_DATA(ANIMAL_COUNT);								   
		STORE_DATA(CHOPPER_COUNT);								   
		STORE_DATA(PYRO_COUNT);									   
		STORE_DATA(PLAYER_COUNT);								   
		STORE_DATA(PROJECTILE_COUNT);							   
		STORE_DATA(SPECIAL_COUNT);								   
		STORE_DATA(SWITCH_COUNT);								   
		STORE_DATA(PRIMARY_COUNT);								   
		STORE_DATA(SECONDARY_COUNT);							   
		STORE_DATA(GAME_STATE);									   
		STORE_DATA(GAME_TURN);									   
		STORE_DATA(GAME_FLAGS);									   
		STORE_DATA(TEXTURE_set);								   
		STORE_DATA(WMOVE_face_upto);							   
		STORE_DATA(first_walkable_prim_point);					   
		STORE_DATA(number_of_walkable_prim_points);				   
		STORE_DATA(first_walkable_prim_face4);					   
		STORE_DATA(number_of_walkable_prim_faces4);				   
		STORE_DATA(BARREL_sphere_last);							   
		STORE_DATA(track_head);									   
		STORE_DATA(track_tail);									   
		STORE_DATA(track_eob);									   
		STORE_DATA( NIGHT_dlight_free);							   
		STORE_DATA( NIGHT_dlight_used);							   


		STORE_DATA(EWAY_time_accurate);							   
		STORE_DATA(EWAY_time);									   
		STORE_DATA(EWAY_tick);									   



		STORE_DATA(EWAY_cam_active);
		STORE_DATA(EWAY_cam_goinactive);
		STORE_DATA(EWAY_cam_x);		// Big coordinates...
		STORE_DATA(EWAY_cam_y);
		STORE_DATA(EWAY_cam_z);
		STORE_DATA(EWAY_cam_dx);
		STORE_DATA(EWAY_cam_dy);
		STORE_DATA(EWAY_cam_dz);
		STORE_DATA(EWAY_cam_yaw);
		STORE_DATA(EWAY_cam_pitch);
		STORE_DATA(EWAY_cam_want_yaw);
		STORE_DATA(EWAY_cam_want_pitch);
		STORE_DATA(EWAY_cam_waypoint);
		STORE_DATA(EWAY_cam_target);
		STORE_DATA(EWAY_cam_delay);
		STORE_DATA(EWAY_cam_speed);
		STORE_DATA(EWAY_cam_freeze);	// Stop the player moving.
		STORE_DATA(EWAY_cam_cant_interrupt);
		STORE_DATA(EWAY_cam_thing);
		STORE_DATA(EWAY_cam_thing);
		STORE_DATA(EWAY_cam_targx);
		STORE_DATA(EWAY_cam_targy);
		STORE_DATA(EWAY_cam_targz);
		STORE_DATA(EWAY_cam_lens);	// 16-bit fixed point
		STORE_DATA(EWAY_cam_warehouse);
		STORE_DATA(EWAY_cam_lock);
		STORE_DATA(EWAY_cam_last_yaw);
		STORE_DATA(EWAY_cam_last_x);
		STORE_DATA(EWAY_cam_last_y);
		STORE_DATA(EWAY_cam_last_z);
		STORE_DATA(EWAY_cam_skip);
		STORE_DATA(EWAY_cam_last_dyaw);



		STORE_DATA( NIGHT_amb_d3d_colour);						   
		STORE_DATA( NIGHT_amb_d3d_specular);					   
		STORE_DATA( NIGHT_amb_red);								   
		STORE_DATA( NIGHT_amb_green);							   
		STORE_DATA( NIGHT_amb_blue);							   
		STORE_DATA( NIGHT_amb_norm_x);							   
		STORE_DATA( NIGHT_amb_norm_y);							   
		STORE_DATA( NIGHT_amb_norm_z);							   
		STORE_DATA( NIGHT_flag);								   
		STORE_DATA( NIGHT_lampost_radius);						   
		STORE_DATA( NIGHT_lampost_red);							   
		STORE_DATA( NIGHT_lampost_green);						   
		STORE_DATA( NIGHT_lampost_blue);						   
		STORE_DATA( NIGHT_sky_colour);						
		STORE_DATA(padding_byte);
		STORE_DATA(check);					
		STORE_DATA(CRIME_RATE);					
		STORE_DATA(CRIME_RATE_SCORE_MUL);					
		STORE_DATA(MUSIC_WORLD);
		STORE_DATA(BOREDOM_RATE);
		STORE_DATA(world_type);
		STORE_DATA(TEXTURE_SET);
		STORE_DATA(padding_word);

		STORE_DATA(EWAY_fake_wander_text_normal_index  );
		STORE_DATA(EWAY_fake_wander_text_normal_number );
		STORE_DATA(EWAY_fake_wander_text_guilty_index  );
		STORE_DATA(EWAY_fake_wander_text_guilty_number );
		STORE_DATA(EWAY_fake_wander_text_annoyed_index );
		STORE_DATA(EWAY_fake_wander_text_annoyed_number);
		
		STORE_DATA(GAME_FLAGS);
		
		STORE_DATA(semtex);
		STORE_DATA(estate);
		STORE_DATA(padding_word);


#ifdef	OLD_CAM													  
																  
		CAM_focus=(Thing*)THING_NUMBER(CAM_focus);				  
		STORE_DATA(CAM_focus);									  
		CAM_focus=TO_THING((SLONG)CAM_focus);					  
#endif															  

		save_whole_anims(handle);								  

		FileClose(handle);

		//
		// convert back again
		//

		convert_index_to_pointers();
	}
}

#endif // TEST_DC
#endif // not TARGET_DC


//#ifdef TARGET_DC
#if TEST_DC


void load_dreamcast_wad(CBYTE *fname)
{
	SLONG	c0=0,id;
	SLONG	*p_slong;
	UWORD	*p_uword;
	UBYTE	*p_mem,*p_all;
	ULONG	mem_size;
	struct	MemTable *ptab;
#ifndef PSX
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
#else
	SLONG			handle;
#endif
	SLONG	count;
	SLONG	texture_set_local;
	SLONG	save_type;

	SLONG	check;
	UBYTE	padding_byte;
	UWORD	padding_word;

	SetSeed(1234567);
	srand(1234567);



	// Clear out all the rendering pages' VBs and IBs.
extern void POLY_ClearAllPages ( void );
	POLY_ClearAllPages();



	extern	UWORD	player_dlight;
	player_dlight=0;

	//
	// Load in the wad file into memory.
	//

#ifndef LAZY_LOADING_MEMORY_ON_DC_PLEASE_BOB
	if(mem_all)
		MemFree(mem_all);
#endif


	// Add a full pathname.
	char cFullName[64] = "";
	strcat ( cFullName, fname );

	// Change the extension to ".dad"
	int iLastChar = strlen ( cFullName );
	ASSERT ( cFullName[iLastChar-4] == '.' );
	ASSERT ( cFullName[iLastChar-3] == 'u' );
	ASSERT ( cFullName[iLastChar-2] == 'c' );
	ASSERT ( cFullName[iLastChar-1] == 'm' );

	cFullName[iLastChar-3] = 'd';
	cFullName[iLastChar-2] = 'a';
	cFullName[iLastChar-1] = 'd';

	TRACE ( "Reading level <%s>", cFullName );




	//strcpy ( cFullName, "dream.dad" );

	handle=FileOpen(cFullName);
	if(handle==FILE_OPEN_ERROR)
	{
		ASSERT(0);
		return;
	}

	mem_size=FileSize(handle);

#ifdef LAZY_LOADING_MEMORY_ON_DC_PLEASE_BOB
	if ( mem_size > mem_all_size )
	{
		if ( mem_all != NULL )
		{
			MemFree ( mem_all );
		}
		mem_all = MemAlloc ( mem_size );
		ASSERT ( mem_all != NULL );
		mem_all_size = mem_size;
	}
#else

	mem_all=(UBYTE*)MemAlloc(mem_size);
	mem_all_size=mem_size;
	ASSERT(mem_all);
#endif

	ASSERT ( mem_all != NULL );
	ASSERT ( mem_size <= mem_all_size );
	FileRead(handle,mem_all,mem_size);
	FileClose(handle);
	p_all = (UBYTE *)mem_all;


extern UBYTE loading_screen_active;
	loading_screen_active = TRUE;

	//
	// Assign the data...
	//

	GET_DATA(save_type);

//	printf("Save Type: %d\n",save_type);
	while(save_table[c0].Point)
	{
		SLONG	struct_size;
//		printf("Loading: %s\n",save_table[c0].Name);
		id=*((SLONG*)p_all);
//		printf(" id %d ",id);
		ASSERT(id==c0);
		p_all+=4;

		count=*((SLONG*)p_all);
//		printf(" count %d",count);
		p_all+=4;

		struct_size=*((SLONG*)p_all);
		p_all+=4;
//		printf(" struct size %d shouldbe %d memsize %d\n",struct_size,save_table[c0].StructSize,*(SLONG*)p_all);
		ASSERT(struct_size==save_table[c0].StructSize);

		mem_size=*((SLONG*)p_all);
		p_all+=4;

		if(mem_size&3)
			p_all+=4-(mem_size&3);

		ptab=&save_table[c0];

		*ptab->Point=p_all;

		switch(ptab->Type)
		{
			case	2:
				if(ptab->Extra)
					ptab->Maximum=count;
				
				break;

			case	1:
				if(ptab->Extra)
					count-=ptab->Extra;

				if(ptab->CountL)
				{
					*ptab->CountL=count;
				}
				else
				{
					if(ptab->CountW)
						*ptab->CountW=count;
				}
				break;
		}

		p_all+=mem_size;

		c0++;
	}

	//
	// I wonder what this does!
	//

	for(c0=0;c0<EWAY_mess_buffer_upto;c0++)
	{
		if (EWAY_mess_buffer[c0]==160)
		{
			EWAY_mess_buffer[c0]=32;
		}
	}

	GET_DATA(PRIMARY_USED);								   
	GET_DATA(PRIMARY_UNUSED);								   
	GET_DATA(SECONDARY_USED);								   
	GET_DATA(SECONDARY_UNUSED);							   
	GET_DATA(PERSON_COUNT);								   
	GET_DATA(PERSON_COUNT);								   
	GET_DATA(ANIMAL_COUNT);								   
	GET_DATA(CHOPPER_COUNT);								   
	GET_DATA(PYRO_COUNT);									   
	GET_DATA(PLAYER_COUNT);								   
	GET_DATA(PROJECTILE_COUNT);							   
	GET_DATA(SPECIAL_COUNT);								   
	GET_DATA(SWITCH_COUNT);								   
	GET_DATA(PRIMARY_COUNT);								   
	GET_DATA(SECONDARY_COUNT);							   
	// Don't load this from the WAD!
	//GET_DATA(GAME_STATE);									   
	p_all+=sizeof(GAME_STATE);

#ifdef TARGET_DC
		GAME_STATE &= ~(GS_RECORD|GS_PLAYBACK|GS_EDITOR);
#endif

	GET_DATA(GAME_TURN);									   
	GET_DATA(GAME_FLAGS);									   
	GET_DATA(texture_set_local);								   
	GET_DATA(WMOVE_face_upto);							   
	GET_DATA(first_walkable_prim_point);					   
	GET_DATA(number_of_walkable_prim_points);				   
	GET_DATA(first_walkable_prim_face4);					   
	GET_DATA(number_of_walkable_prim_faces4);				   
	GET_DATA(BARREL_sphere_last);							   
	GET_DATA(track_head);									   
	GET_DATA(track_tail);									   
	GET_DATA(track_eob);									   
	GET_DATA( NIGHT_dlight_free);							   
	GET_DATA( NIGHT_dlight_used);							   


	GET_DATA(EWAY_time_accurate);							   
	GET_DATA(EWAY_time);									   
	GET_DATA(EWAY_tick);									   

	GET_DATA(EWAY_cam_active);
	GET_DATA(EWAY_cam_goinactive);
	GET_DATA(EWAY_cam_x);		// Big coordinates...
	GET_DATA(EWAY_cam_y);
	GET_DATA(EWAY_cam_z);
	GET_DATA(EWAY_cam_dx);
	GET_DATA(EWAY_cam_dy);
	GET_DATA(EWAY_cam_dz);
	GET_DATA(EWAY_cam_yaw);
	GET_DATA(EWAY_cam_pitch);
	GET_DATA(EWAY_cam_want_yaw);
	GET_DATA(EWAY_cam_want_pitch);
	GET_DATA(EWAY_cam_waypoint);
	GET_DATA(EWAY_cam_target);
	GET_DATA(EWAY_cam_delay);
	GET_DATA(EWAY_cam_speed);
	GET_DATA(EWAY_cam_freeze);	// Stop the player moving.
	GET_DATA(EWAY_cam_cant_interrupt);
	GET_DATA(EWAY_cam_thing);
	GET_DATA(EWAY_cam_thing);	// Twice because EWAY_cam_thing is a UWORD
	GET_DATA(EWAY_cam_targx);
	GET_DATA(EWAY_cam_targy);
	GET_DATA(EWAY_cam_targz);
	GET_DATA(EWAY_cam_lens);	// 16-bit fixed point
	GET_DATA(EWAY_cam_warehouse);
	GET_DATA(EWAY_cam_lock);
	GET_DATA(EWAY_cam_last_yaw);
	GET_DATA(EWAY_cam_last_x);
	GET_DATA(EWAY_cam_last_y);
	GET_DATA(EWAY_cam_last_z);
	GET_DATA(EWAY_cam_skip);
	GET_DATA(EWAY_cam_last_dyaw);


	GET_DATA(NIGHT_amb_d3d_colour);
	GET_DATA(NIGHT_amb_d3d_specular);
	GET_DATA(NIGHT_amb_red);
	GET_DATA(NIGHT_amb_green);
	GET_DATA(NIGHT_amb_blue);
	GET_DATA(NIGHT_amb_norm_x);							   
	GET_DATA(NIGHT_amb_norm_y);							   
	GET_DATA(NIGHT_amb_norm_z);							   
	GET_DATA(NIGHT_flag);								   
	GET_DATA(NIGHT_lampost_radius);						   
	GET_DATA(NIGHT_lampost_red);							   
	GET_DATA(NIGHT_lampost_green);						   
	GET_DATA(NIGHT_lampost_blue);						   
	GET_DATA(NIGHT_sky_colour);						
	GET_DATA(padding_byte);
	GET_DATA(check);										  
	GET_DATA(CRIME_RATE);					
	GET_DATA(CRIME_RATE_SCORE_MUL);					
	GET_DATA(MUSIC_WORLD);
	GET_DATA(BOREDOM_RATE);
	GET_DATA(world_type);
	GET_DATA(TEXTURE_SET);
	GET_DATA(padding_word);

	GET_DATA(EWAY_fake_wander_text_normal_index  );
	GET_DATA(EWAY_fake_wander_text_normal_number );
	GET_DATA(EWAY_fake_wander_text_guilty_index  );
	GET_DATA(EWAY_fake_wander_text_guilty_number );
	GET_DATA(EWAY_fake_wander_text_annoyed_index );
	GET_DATA(EWAY_fake_wander_text_annoyed_number);

	GET_DATA(GAME_FLAGS);

	GET_DATA(semtex);
	GET_DATA(estate);
	GET_DATA(padding_word);

	ASSERT(check==666);

	//NIGHT_flag&=~NIGHT_FLAG_DAYTIME;

	// This needs to go here, if it doesn't then the later levels will
	// automatically fail because the timer will be over the time limit.

	EWAY_time_accurate = 0;
	EWAY_time          = 0;
	EWAY_count_up      = 0;

	//
	// The penalties on the count-up timer.
	//

	EWAY_count_up_add_penalties = 0;
	EWAY_count_up_num_penalties = 0;
	EWAY_count_up_penalty_timer = 0;

//	NIGHT_amb_red*=2;
//	NIGHT_amb_green*=2;
//	NIGHT_amb_blue*=2;
	SATURATE(NIGHT_amb_red,16,127);
	SATURATE(NIGHT_amb_green,16,127);
	SATURATE(NIGHT_amb_blue,25,127);


	//
	// this needs to be before convert_index_to_pointers (because c_i_t_p needs the anims in place)
	//
	load_whole_anims(p_all);

	void setup_global_anim_array(void);
	setup_global_anim_array();
	void init_dead_tween(void);
	init_dead_tween();

	convert_index_to_pointers();

	// Reset camera mode.
extern int g_iPlayerCameraMode;
	g_iPlayerCameraMode = 0;
	FC_change_camera_type(0,0);
	FC_look_at(0, THING_NUMBER(NET_PERSON(0)));
	FC_force_camera_behind(0);

	uncache();

	//
	// Pass a NULL filename to TEXTURE_load_needed(). This is okay if
	// we aren't using texture clumping.
	//
	// Er.... no it isn't - I need that name! ATF
	//

	if ( ( GAME_STATE & GS_REPLAY ) == 0 )
	{
		TEXTURE_choose_set(texture_set_local);
		//TEXTURE_load_needed(NULL);
		TEXTURE_load_needed(fname, 0, 256, 900);
	}

extern void PARTICLE_Reset();
	PARTICLE_Reset();

extern void POW_init();
	POW_init();

//	calc_prim_info();
	VEH_init_vehinfo();

extern void DIRT_init(SLONG,SLONG,SLONG,SLONG,SLONG,SLONG,SLONG);
	DIRT_init(100, 3, 3, INFINITY, INFINITY, INFINITY, INFINITY);

extern	void	init_noises(void);
extern	void	init_arrest(void);
	init_noises();
	init_arrest();
void MIST_init();
	MIST_init();
extern void PANEL_new_text_init();
	PANEL_new_text_init();
	SPARK_init();


	// Set Darci's powerups.
	NET_PLAYER(0)->Genus.Player->Strength	  =the_game.DarciStrength;
	NET_PLAYER(0)->Genus.Player->Constitution=the_game.DarciConstitution;
	NET_PLAYER(0)->Genus.Player->Stamina	  =the_game.DarciStamina;
	NET_PLAYER(0)->Genus.Player->Skill		  =the_game.DarciSkill;

	// Clear the cheat status.
extern bool g_bPunishMePleaseICheatedOnThisLevel;
	g_bPunishMePleaseICheatedOnThisLevel = FALSE;

	calc_prim_normals();
//	find_anim_prim_bboxes();

//	TEXTURE_fix_prim_textures();
//	TEXTURE_fix_texture_styles();
	AENG_create_dx_prim_points();
	NIGHT_generate_walkable_lighting();

	flag_v_faces();

	//
	// Stop it getting too dark, bloody artists!
	//
void	init_gangattack(void);

	init_gangattack(); //probably should save these

extern	SBYTE	global_spang_count;
	global_spang_count=0;


extern UBYTE EWAY_conv_active;
	EWAY_conv_active=0; //fixes semtex restart crash during cut scene

#ifdef TARGET_DC
	// Reactivates mist.
extern void EWAY_reactivate_waypoints_that_arent_in_the_dad_file ( void );
	EWAY_reactivate_waypoints_that_arent_in_the_dad_file ();
#endif
	
	loading_screen_active = FALSE;

}

#endif // TARGET_DC