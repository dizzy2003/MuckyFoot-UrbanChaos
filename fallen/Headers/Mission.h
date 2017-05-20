//	Mission.h
//	Guy Simmons, 2nd August 1998.

#ifndef	MISSION_H
#define	MISSION_H

//---------------------------------------------------------------

#define WPT_FLAGS_SUCKS		  1
#define WPT_FLAGS_INVERSE	  2
#define WPT_FLAGS_INSIDE	  4
#define WPT_FLAGS_WARE		  8
#define WPT_FLAGS_REFERENCED 16	// => Another event point references this one. Set in export_mission()
#define WPT_FLAGS_OPTIONAL	 32 // can be twatted on the PSX if necessary

#define	MAX_EVENTPOINTS		512

struct	EventPoint
{
	UBYTE		Colour,				// (the colour and group (A-Z) combine to make a set of 
				Group,				//  events that are linked, eg the waypoints along a patrol)
				WaypointType,		// WPT_* -- what the event does when triggered
				Used,				// bool, whether the event is in use

				TriggeredBy,		// TT_*  -- what triggers the event
				OnTrigger,			// OT_*  -- how the trigger should continue/reset 
				Direction,			// Angle in degrees, scaled to fit a byte
				Flags;				// New and improved! Now contains sucks, inverse and inside info too!

	UWORD		EPRef,				// Dependency index for TT_DEPENDENCY and TT_BOOLEAN (first input)
				EPRefBool,			// index for TT_BOOLEAN (second input)
				AfterTimer;			// for OT_ACTIVE_TIME, how long to wait before resetting
//fuck	UWORD		MorePadding;        // PSX requires this
	SLONG		Data[10],
				Radius,				// for TT_RADIUS; used as time argument for TT_TIMER or pointer for TT_SHOUT (eek)
				X,Y,Z;
	

	UWORD		Next,
				Prev;
};

extern	EventPoint		*current_ep;

//---------------------------------------------------------------

#define	MAX_MISSIONS		100

#define MISSION_FLAG_USED		          (1 << 0)
#define MISSION_FLAG_SHOW_CRIMERATE       (1 << 1)
#define MISSION_FLAG_CARS_WITH_ROAD_PRIMS (1 << 2)

struct	Mission
{
	CBYTE			Flags;
	CBYTE			BriefName[_MAX_PATH],
					LightMapName[_MAX_PATH],
					MapName[_MAX_PATH],
					MissionName[_MAX_PATH],
					CitSezMapName[_MAX_PATH];	// Was the sewer file.
	UWORD			MapIndex,
					FreeEPoints,
					UsedEPoints;
	UBYTE			CrimeRate,
					CivsRate;
	EventPoint		EventPoints[MAX_EVENTPOINTS];
	UBYTE			SkillLevels[254]; // up to 254 AI types supported
	UBYTE			BoredomRate;
	UBYTE			CarsRate,
					MusicWorld;
};

struct	OldMissionB
{
	BOOL			Used;
	CBYTE			BriefName[_MAX_PATH],
					LightMapName[_MAX_PATH],
					MapName[_MAX_PATH],
					MissionName[_MAX_PATH],
					CitSezMapName[_MAX_PATH];	// Was the sewer file.
	UWORD			MapIndex,
					FreeEPoints,
					UsedEPoints;
	UBYTE			CrimeRate,
					CivsRate;
	EventPoint		EventPoints[MAX_EVENTPOINTS];
	UBYTE			SkillLevels[255]; // up to 255 AI types supported

};

struct	OldMission
{
	BOOL			Used;
	CBYTE			BriefName[_MAX_PATH],
					LightMapName[_MAX_PATH],
					MapName[_MAX_PATH],
					MissionName[_MAX_PATH],
					SewerMapName[_MAX_PATH];
	UWORD			MapIndex,
					FreeEPoints,
					UsedEPoints,
					padding;
	EventPoint		EventPoints[MAX_EVENTPOINTS];
};

extern Mission		mission_pool[MAX_MISSIONS],
					*current_mission;

extern CBYTE MissionZones[MAX_MISSIONS][128][128];

//---------------------------------------------------------------

#define ZF_NONE				0
#define ZF_INSIDE			1
#define ZF_REVERB			2
#define ZF_NO_WANDER		4
#define ZF_ZONE1			8
#define	ZF_ZONE2			16
#define	ZF_ZONE3			32
#define	ZF_ZONE4			64
#define ZF_NO_GO			128
#define ZF_NUM				8

//---------------------------------------------------------------

#define	MAX_MAPS			20

struct	GameMap
{
	BOOL		Used;
	CBYTE		MapName[_MAX_PATH];
	UWORD		Missions[MAX_MISSIONS];
};

extern GameMap		*current_map,
					game_maps[MAX_MAPS];

//---------------------------------------------------------------
//	Access Defines.

#define	TO_MAP(m)				(&game_maps[m])
#define	TO_MISSION(m)			(&mission_pool[m])
#define	TO_EVENTPOINT(b,e)		(&b[e])
#define	MAP_NUMBER(m)			(m-TO_MAP(0))
#define	MISSION_NUMBER(m)		(m-TO_MISSION(0))
//#define	EVENTPOINT_NUMBER(b,e)	(e-TO_EVENTPOINT(b,0))
#define	EVENTPOINT_NUMBER(b,e)	(e-b)

//---------------------------------------------------------------
//	Waypoint types.
	
#define	WPT_NONE			0
#define	WPT_SIMPLE			1
#define	WPT_CREATE_PLAYER	2
#define	WPT_CREATE_ENEMIES	3
#define	WPT_CREATE_VEHICLE	4
#define	WPT_CREATE_ITEM		5
#define	WPT_CREATE_CREATURE	6
#define	WPT_CREATE_CAMERA	7
#define	WPT_CREATE_TARGET	8
#define	WPT_CREATE_MAP_EXIT	9
#define	WPT_CAMERA_WAYPOINT	10
#define WPT_TARGET_WAYPOINT	11
#define	WPT_MESSAGE			12
#define	WPT_SOUND_EFFECT	13
#define	WPT_VISUAL_EFFECT	14
#define	WPT_CUT_SCENE		15
#define	WPT_TELEPORT		16
#define	WPT_TELEPORT_TARGET	17
#define WPT_END_GAME_LOSE	18
#define WPT_SHOUT			19
#define	WPT_ACTIVATE_PRIM	20
#define	WPT_CREATE_TRAP		21
#define	WPT_ADJUST_ENEMY	22
#define WPT_LINK_PLATFORM	23
#define	WPT_CREATE_BOMB		24
#define	WPT_BURN_PRIM		25
#define WPT_END_GAME_WIN	26
#define WPT_NAV_BEACON		27
#define WPT_SPOT_EFFECT		28
#define WPT_CREATE_BARREL	29
#define	WPT_KILL_WAYPOINT	30
#define WPT_CREATE_TREASURE	31
#define WPT_BONUS_POINTS	32
#define WPT_GROUP_LIFE		33
#define WPT_GROUP_DEATH		34
#define WPT_CONVERSATION	35
#define WPT_INTERESTING		36
#define	WPT_INCREMENT		37
#define WPT_DYNAMIC_LIGHT	38
#define WPT_GOTHERE_DOTHIS	39
#define WPT_TRANSFER_PLAYER 40
#define WPT_AUTOSAVE		41
#define WPT_MAKE_SEARCHABLE 42
#define WPT_LOCK_VEHICLE    43
#define WPT_GROUP_RESET		44
#define WPT_COUNT_UP_TIMER	45
#define WPT_RESET_COUNTER	46
#define WPT_CREATE_MIST		47
#define WPT_ENEMY_FLAGS		48
#define WPT_STALL_CAR		49
#define WPT_EXTEND			50
#define WPT_MOVE_THING		51
#define WPT_MAKE_PERSON_PEE 52
#define WPT_CONE_PENALTIES  53
#define WPT_SIGN			54
#define WPT_WAREFX			55
#define WPT_NO_FLOOR        56
#define WPT_SHAKE_CAMERA    57

// not used any more, kept so elev.cpp doesn't break...
#define WPT_TRIGGER			-1
#define WPT_END_GAME		WPT_END_GAME_LOSE

//---------------------------------------------------------------
//	Player defines.

#define	PT_NONE				0
#define	PT_DARCI			1
#define	PT_ROPER			2
#define PT_COP				3
#define PT_GANG				4

//---------------------------------------------------------------
//	Enemy defines.

#define ET_NONE					0
#define ET_CIV					1
#define ET_CIV_BALLOON			2
#define ET_SLAG					3
#define ET_UGLY_FAT_SLAG		4
#define ET_WORKMAN				5
#define ET_GANG_RASTA			6
#define ET_GANG_RED				7
#define ET_GANG_GREY			8
#define ET_GANG_RASTA_PISTOL	9
#define ET_GANG_RED_SHOTGUN		10
#define ET_GANG_GREY_AK47		11
#define ET_COP					12
#define ET_COP_PISTOL			13
#define ET_COP_SHOTGUN			14
#define ET_COP_AK47				15
#define ET_HOSTAGE				16
#define ET_WORKMAN_GRENADE		17
#define ET_TRAMP				18
#define ET_MIB1					19
#define ET_MIB2					20
#define ET_MIB3					21
#define ET_DARCI				22
#define ET_ROPER				23

/*

#define ET_NONE					0
#define ET_CIVILLIAN			1
#define ET_CIVILLIAN_CLOWN		2
#define	ET_CIVILLIAN_TRAMP		3
#define ET_CIVILLIAN_OFFICE		4
#define ET_CIVILLIAN_INDUSTRY	5
#define ET_GANG_UNARMED			6
#define ET_GANG_BASEBALL		7
#define ET_GANG_KNIFE			8
#define ET_GANG_PISTOL			9
#define ET_GANG_CAPTAIN			10
#define ET_GANG_BOSS			11
#define ET_COP_UNARMED			12
#define ET_COP_ARMED			13
#define ET_COP_SWAT				14
#define ET_SECURITY_GUARD		15
#define ET_ARMY_SOLDIER			16
#define ET_ARMY_CAPTAIN			17
#define ET_ARMY_GUARD			18
#define ET_CULT_UNARMED			19
#define ET_CULT_CAPTAIN			20
#define ET_CULT_BOSS			21
#define ET_DARCI				22
#define ET_ROPER				23

// obsolete redefinitions...
#define ET_CIVILIAN			ET_CIVILLIAN
#define	ET_GANGMEMBER		ET_GANG_UNARMED
#define	ET_CULTMEMBER		ET_CULT_UNARMED
#define	ET_COP				ET_COP_UNARMED
#define	ET_SOLDIER			ET_ARMY_SOLDIER

*/


//---------------------------------------------------------------
//	Vehicle define.

#define	VT_NONE				0
#define	VT_CAR				1
#define	VT_VAN				2
#define	VT_TAXI 			3
#define	VT_HELICOPTER		4
#define VT_BIKE				5
#define VT_POLICE			6
#define VT_AMBULANCE		7
#define VT_MEATWAGON		8
#define VT_JEEP				9
#define VT_SEDAN			10
#define VT_WILDCATVAN       11

#define VMT_PLAYER_DRIVES	0
#define VMT_PATROL_WPTS		1
#define VMT_GUARD_AREA		2
#define VMT_TRACK_TARGET	3

#define VK_UNLOCKED	0
#define VK_RED		1
#define VK_BLUE		2
#define VK_GREEN	3
#define VK_BLACK	4
#define VK_WHITE	5
#define VK_LOCKED	6

//---------------------------------------------------------------
//	Item defines.

#define IT_NONE		    0
#define IT_KEY		    1
#define IT_PISTOL	    2
#define IT_HEALTH	    3
#define IT_SHOTGUN	    4
#define IT_KNIFE	    5
#define IT_AK47		    6
#define IT_MINE		    7
#define IT_BASEBALLBAT  8
#define IT_BARREL       9
#define IT_AMMO_PISTOL  10
#define IT_AMMO_SHOTGUN 11
#define IT_AMMO_AK47    12
#define IT_KEYCARD		13
#define IT_FILE			14
#define IT_FLOPPY_DISK	15
#define IT_CROWBAR		16
#define IT_GASMASK		17
#define IT_WRENCH		18
#define IT_VIDEO		19
#define IT_GLOVES		20
#define IT_WEEDAWAY		21
#define IT_GRENADE		22
#define IT_EXPLOSIVES   23
#define	IT_SILENCER		24
#define	IT_WIRE_CUTTER	25


/*
#define	IT_NONE					0
#define	IT_KNIFE				1
#define	IT_BASEBALL_BAT			2
#define	IT_SLEDGE_HAMMER		3
#define	IT_MACHETE				4
#define	IT_IRON_BAR				5
#define	IT_AXE					6
#define	IT_GRENADE_HAND			7
#define	IT_GRENADE_FIRE			8
#define	IT_GRENADE_SMOKE		9
#define	IT_DYNAMITE				10
#define	IT_CROSSBOW				11
#define	IT_COLT45				12
#define	IT_UZI9MM				13
#define	IT_H_AND_K_MP5			14
#define	IT_RIFLES				15
#define	IT_SAUTO_RIFLE			16
#define	IT_SAUTO_RIFLE_SNIPER	17
#define	IT_AUTO_KALASHNIKOV		18
#define	IT_AUTO_M16A2			19
#define	IT_SAWN_OFF				20
#define	IT_PUMP_ACTION			21
#define	IT_M61_LIGHT_MG			22
#define	IT_CHAIN_GUN			23
#define	IT_ANTI_TANK_WEAPON		24
#define	IT_ANTI_TANK_RIFLE		25
#define	IT_GRENADE_LAUNCHER		26
#define	IT_M78_GL				27
#define	IT_FLAMETHROWER			28
#define	IT_HOME_MADE_FLAMER		29
#define	IT_FLAK_JACKET			30	// The second ULONG starts here!
#define	IT_PETROL_LIGHTER		31
#define	IT_SPRAY_CAN			32
#define	IT_RED_KEY_CARD			33
#define	IT_BLUE_KEY_CARD		34
#define	IT_GREEN_KEY_CARD		35
#define	IT_CROW_BAR				36
#define	IT_LOCK_PICKER			37
#define	IT_SCANNER				38
#define	IT_AMMUNITION_CLIP		39
#define	IT_AMMUNITION_FUEL		40
#define	IT_MEDICAL_KIT			41
#define	IT_STIM_PILL			42
#define	IT_GRAPPLING_HOOK		43
*/

//---------------------------------------------------------------
// Items an enemy can have

#define HAS_PISTOL		(1 << 0)
#define HAS_SHOTGUN		(1 << 1)
#define HAS_AK47		(1 << 2)
#define HAS_GRENADE		(1 << 3)
#define HAS_BALLOON		(1 << 4)
#define HAS_KNIFE		(1 << 5)
#define HAS_BAT			(1 << 6)


//---------------------------------------------------------------
//	Visual Effect defines.

#define	VFX_NONE				0
#define	VFX_EXPLOSION			1
#define	VFX_FIRE				2
#define	VFX_WATER_JET			3

//---------------------------------------------------------------
//	Creature Types.

#define	CT_NONE					0
#define	CT_DOG					1
#define	CT_PIGEON				2

//---------------------------------------------------------------
//  Activate prim types.

#define AP_DOOR					0
#define AP_ELECTRIC_FENCE		1
#define AP_SECURITY_CAMERA		2

//---------------------------------------------------------------
//	Trigger Types.

#define	TT_NONE					0
#define	TT_DEPENDENCY			1
#define	TT_RADIUS				2
#define	TT_DOOR					3
#define	TT_TRIPWIRE				4
#define	TT_PRESSURE_PAD			5
#define	TT_ELECTRIC_FENCE		6
#define	TT_WATER_LEVEL			7
#define	TT_SECURITY_CAMERA		8
#define	TT_SWITCH				9
#define	TT_ANIM_PRIM			10
#define	TT_TIMER				11
#define	TT_SHOUT_ALL			12
#define	TT_BOOLEANAND			13
#define	TT_BOOLEANOR			14
#define	TT_ITEM_HELD			15
#define	TT_ITEM_SEEN			16
#define	TT_KILLED				17
#define	TT_SHOUT_ANY			18
#define	TT_COUNTDOWN			19
#define TT_ENEMYRADIUS			20
#define TT_VISIBLECOUNTDOWN		21
#define TT_CUBOID				22
#define TT_HALFDEAD				23
#define TT_GROUPDEAD			24
#define TT_PERSON_SEEN			25
#define TT_PERSON_USED			26
#define TT_PLAYER_USES_RADIUS	27
#define TT_PRIM_DAMAGED     	28
#define TT_PERSON_ARRESTED		29
#define TT_CONVERSATION_OVER	30
#define TT_COUNTER				31
#define TT_KILLED_NOT_ARRESTED	32
#define TT_CRIME_RATE_ABOVE		33
#define TT_CRIME_RATE_BELOW		34
#define TT_PERSON_IS_MURDERER	35
#define TT_PERSON_IN_VEHICLE    36
#define TT_THING_RADIUS_DIR		37
#define TT_PLAYER_CARRY_PERSON  38
#define TT_SPECIFIC_ITEM_HELD	39
#define TT_RANDOM				40
#define TT_PLAYER_FIRES_GUN		41
#define TT_DARCI_GRABBED		42
#define TT_PUNCHED_AND_KICKED   43
#define TT_MOVE_RADIUS_DIR      44
// how many?
#define TT_NUMBER				45

/*// not used any more, kept so elev.cpp doesn't break...
#define TT_NORMAL 			-1

//	Triggered By (obsolete)
#define	TB_NONE					0
#define	TB_PROXIMITY			1
*/
//	On Trigger.
#define	OT_NONE					0
#define	OT_ACTIVE				1
#define	OT_ACTIVE_WHILE			2
#define	OT_ACTIVE_TIME			3
#define OT_ACTIVE_DIE			4

//	Camera target type.
#define CT_NORMAL				1
#define CT_ATTACHED				2
#define CT_NEAREST_LIVING		3

//	Link platform flags
#define LP_LOCK_TO_AXIS			(1 << 0)
#define LP_LOCK_ROTATION		(1 << 1)
#define LP_BODGE_ROCKET 		(1 << 2)

//
// Barrel types.
// 

#define BT_BARREL			0
#define BT_TRAFFIC_CONE		1
#define BT_BIN				2
#define BT_BURNING_BARREL	3
#define BT_BURNING_BIN		4
#define BT_OIL_DRUM			5
#define BT_LOX_DRUM			6


//---------------------------------------------------------------
// Masks for what's used; makes handling properties easier

#define WPU_DEPEND				1
#define	WPU_BOOLEAN				2
#define	WPU_TIME				4
#define	WPU_RADIUS				8
#define	WPU_RADTEXT				16
#define WPU_RADBOX				32
#define WPU_COUNTER				64

extern CBYTE WaypointUses[TT_NUMBER];


//---------------------------------------------------------------

void		MISSION_init(void);

UWORD		alloc_map(void);
void		free_map(UWORD map);
UWORD		alloc_mission(UWORD	map_ref);
void		free_mission(UWORD mission);
void		init_mission(UWORD mission_ref,CBYTE *mission_name);
EventPoint	*alloc_eventpoint(void);
void		free_eventpoint(EventPoint *the_ep);
#ifndef		PSX
void		write_event_extra(FILE *file_handle, EventPoint *ep);
void		read_event_extra(FILE *file_handle, EventPoint *ep, EventPoint *base, SLONG ver=0);
#endif
BOOL		export_mission(void);
void		import_mission(void);
BOOL		valid_mission(void);
void		ResetFreepoint(Mission *mission);
void		ResetUsedpoint(Mission *mission);
void		ResetFreelist(Mission *mission);
void		ResetUsedlist(Mission *mission);
BOOL		HasText(EventPoint *ep);
UWORD		GetTextID(CBYTE *msg);
UWORD		GetEPTextID(EventPoint *ep);
//void		SetTextID(CBYTE *msg, SLONG value=-1);
void		SetEPTextID(EventPoint *ep, SLONG value=-1);
CBYTE	   *GetEPText(EventPoint *ep);


//---------------------------------------------------------------

#endif
