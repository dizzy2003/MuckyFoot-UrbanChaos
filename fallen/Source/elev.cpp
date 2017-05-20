//
// The new ECTS level stuff!
// 

//
// Richard Reed's phone number in London, Hilton. 0171 636 1000 (room 343)
// 482874 for Bjarne at EA.
//
// Jez - 07771 780 630,   01483 573 849
// Darran - 01483 453085  07788 747 600
// Glenn - 07887 515577
//


#include "game.h"

#ifndef	PSX
#include "ddlib.h"
#else
#define ZeroMemory(a,s) memset((UBYTE*)a,0,s)
SLONG PSX_eog_timer;
extern SLONG MFX_OnKey,MFX_OffKey;
#endif



#include "eway.h"
#include "mission.h"
#include "night.h"
#include "ob.h"
#include "trip.h"
#include "music.h"
#include "dirt.h"
#include "fog.h"
#include "hook.h"
#include "mist.h"
#include "water.h"
#include "puddle.h"
#include "az.h"
#include "drip.h"
#include "bang.h"
#include "glitter.h"
#include "spark.h"
#include "io.h"
#include "pow.h"
#include "build2.h"
#ifndef TARGET_DC
#include "es.h"
#endif
#include "ns.h"
#include "road.h"
#include "mav.h"
#include "cnet.h"
#include "interfac.h"
#include "animtmap.h"
#include "shadow.h"
#include "attract.h"
#include "cam.h"
#include "psystem.h"
#include "tracks.h"
#include "pcom.h"
#include "wmove.h"
#include "balloon.h"
#include "wand.h"
#include "ribbon.h"
#include "barrel.h"
#include "fc.h"
#include "briefing.h"
#include "ware.h"
#include "memory.h"
#include "playcuts.h"
#include "grenade.h"
#include "env.h"
#ifndef PSX
#include "panel.h"
#endif

#include "sound.h"
#ifdef USE_A3D
#include "soundenv.h"
#endif

#include "DCLowLevel.h"



#ifdef PSX

//
// PSX include
//
#include "libsn.h"
#include "psxeng.h"

#define	MFFileHandle	SLONG
#define	FILE_OPEN_ERROR	(-1)
#define	SEEK_MODE_CURRENT	(1)

extern	SLONG	SpecialOpen(CBYTE *name);
extern	SLONG	SpecialRead(SLONG handle,UBYTE *ptr,SLONG s1);
extern	SLONG	SpecialSeek(SLONG handle,SLONG mode,SLONG size);
extern	SLONG	SpecialClose(SLONG handle);

#define	FileOpen(x)		SpecialOpen(x)
#define	FileClose(x)	SpecialClose(x)
#define	FileCreate(x,y)	ASSERT(0)
#define	FileRead(h,a,s) SpecialRead(h,(char*)a,s)
#define	FileWrite(h,a,s) ASSERT(0)
#define	FileSeek(h,m,o) SpecialSeek(h,m,o)

#ifdef PSX
CBYTE *psx_game_name;
#endif

#define	FILE_CLOSE_ERROR		((MFFileHandle)-101)
#define	FILE_CREATION_ERROR		((MFFileHandle)-102)
#define	FILE_SIZE_ERROR			((SLONG)-103)
#define	FILE_READ_ERROR			((SLONG)-104)
#define	FILE_WRITE_ERROR		((SLONG)-105)
#define	FILE_SEEK_ERROR			((SLONG)-106)
#define	FILE_LOAD_AT_ERROR		((SLONG)-107)


//
// psx has no sewers at the moment
//
SLONG ES_load(CBYTE *filename)
{
	return(0);
}

void ES_build_sewers(void)
{
}


#endif


#ifndef PSX

//
// This is the last map to be loaded.
// 

CBYTE ELEV_last_map_loaded[MAX_PATH];


#ifdef	MIKE

MFFileHandle			llog_handle		=	NULL;
extern	TCHAR *witem_strings[];
void		TesterText(CBYTE *error, ...)
{
	CBYTE 			buf[512];
	va_list 		argptr;
	if(!llog_handle)
	{
		llog_handle	=	FileCreate("tester.log",1);
		if(llog_handle==FILE_CREATION_ERROR)
			llog_handle	=	NULL;			
	}

	if(llog_handle)
	{
		va_start(argptr,error); 
		vsprintf(buf, error,argptr); 
		va_end(argptr);
		
		FileWrite(llog_handle,buf,strlen(buf));
	}
}

#endif

#endif


//
// to stop psx stack overflow
//

#ifndef PSX
#ifndef TARGET_DC
extern	UBYTE	vehicle_random[];

CBYTE      junk[2048];
EventPoint event_point;

extern	SLONG save_psx;
SLONG	iamapsx=0;

void ELEV_load_level(CBYTE *fname_level)
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;
	SLONG tx;
	SLONG ty;
	SLONG tz;
	SLONG angle;
	SLONG version;
	SLONG load_ok = FALSE;
	SLONG flag;
	
	SLONG ew_id;
	SLONG ew_type;
	SLONG ew_subtype;
	SLONG ew_world_x;
	SLONG ew_world_y;
	SLONG ew_world_z;
	SLONG ew_active;
	SLONG ew_active_arg;
	SLONG ew_stay;
	SLONG ew_stay_arg;
	SLONG ew_colour;
	SLONG ew_group;
	SLONG ew_yaw;
	SLONG ew_speed;
	SLONG ew_delay;

	SLONG mess_count  =  0;
	SLONG cutscene_count = 0;
	SLONG water_level = -0x80;
	
	MFFileHandle handle = NULL;
	CBYTE       *error;

	SLONG enemy_type;
	SLONG enemy_count;
	SLONG follow;
	SLONG kludge_index;

	EWAY_Conddef ecd;
	EWAY_Conddef ecd1;
	EWAY_Conddef ecd2;
	EWAY_Do      ed;
	EWAY_Stay    es;
	EWAY_Edef    ee;

	OB_Info *oi;
	UBYTE	FAKE_CARS = 0;

	//
	// Clear all waypoint info.
	// 

	GAME_TURN=0;
	SAVE_VALID=0;

	EWAY_init();	

	iamapsx=ENV_get_value_number("iamapsx", FALSE);

	//
	// Start off with no players.
	//
	if(!CNET_network_game)
		NO_PLAYERS = 0;

	//
	// Load the level.
	//

	load_ok = TRUE;

	if (fname_level != NULL)
	{
		handle = FileOpen(fname_level);

		if (handle == FILE_OPEN_ERROR)
		{
			//
			// Couldn't open file.
			//

			load_ok = FALSE;
		}
		else
		{
			//
			// Load in the mission file.
			//

			if (FileRead(handle, &version, sizeof(SLONG)) == FILE_READ_ERROR) goto file_error;	// Version
			if (FileRead(handle, &flag,    sizeof(SLONG)) == FILE_READ_ERROR) goto file_error;	// Used
			if (FileRead(handle,  junk,    _MAX_PATH)	  == FILE_READ_ERROR) goto file_error;	// BriefName
			if (FileRead(handle,  junk,    _MAX_PATH)	  == FILE_READ_ERROR) goto file_error;	// LightMapName
			if (FileRead(handle,  junk,    _MAX_PATH)	  == FILE_READ_ERROR) goto file_error;	// MapName
			if (FileRead(handle,  junk,    _MAX_PATH)	  == FILE_READ_ERROR) goto file_error;	// MissionName
			if (FileRead(handle,  junk,    _MAX_PATH)	  == FILE_READ_ERROR) goto file_error;	// SewerMapName
			if (FileRead(handle,  junk,    sizeof(UWORD)) == FILE_READ_ERROR) goto file_error;	// MapIndex... what's this?
			if (FileRead(handle,  junk,    sizeof(UWORD)) == FILE_READ_ERROR) goto file_error;	// Used
			if (FileRead(handle,  junk,    sizeof(UWORD)) == FILE_READ_ERROR) goto file_error;	// Free

			GAME_FLAGS &= ~GF_SHOW_CRIMERATE;
			GAME_FLAGS &= ~GF_CARS_WITH_ROAD_PRIMS;

			if (flag & MISSION_FLAG_SHOW_CRIMERATE      ) {GAME_FLAGS |= GF_SHOW_CRIMERATE      ;}
			if (flag & MISSION_FLAG_CARS_WITH_ROAD_PRIMS) {GAME_FLAGS |= GF_CARS_WITH_ROAD_PRIMS;}

			//
			// Two bytes of crime rate and padding...
			//

			if (FileRead(handle, junk, sizeof(UWORD)) == FILE_READ_ERROR) goto file_error;	// Crime rate byte / Padding byte

			CRIME_RATE = junk[0];

			if (CRIME_RATE == 0)
			{	
				//
				// If the mission hasn't been given a crime rate.
				//

				CRIME_RATE = 50;
			}
//#ifdef	EIDOS
			FAKE_CIVS = junk[1];// = 0;

			extern UBYTE build_dc;

			if(save_psx)
			{
				FAKE_CIVS >>=1;
				if(FAKE_CIVS>3)
					FAKE_CIVS=3;

			}
			else
			if (build_dc)
			{
				FAKE_CIVS *= 2;
				FAKE_CIVS /= 3;
			}
			else
			{
				if(!the_display.CurrDevice->IsHardware())
				{
					if(FAKE_CIVS>5)
						FAKE_CIVS=5;

				}
			}

			
			for(i=0;i<FAKE_CIVS;i++)

			{
				SLONG	index;
				SLONG	mx,mz;
				//
				// lets hope the player isnt looking at the edge of the map
				//

				SLONG max = 0;

				while(max++ < 128 * 128)
				{
SLONG	WAND_find_good_start_point_near(SLONG *mapx,SLONG *mapz);

					mx=(Random()%100)+14;
					mz=(Random()%100)+14;



					if(WAND_find_good_start_point_near(&mx,&mz))
						break;
				}

				index=PCOM_create_person(
					PERSON_CIV,
					0,
					0,
					PCOM_AI_CIV,
					0,
					0,
					PCOM_MOVE_WANDER,
					0,
					0,		// bent
					0,		// has
					0,		// drop
					0,		// zone
					mx<<8,//0x10000,//darci->WorldPos.X + 0x4000,
					0,//0x10000,//darci->WorldPos.Y,
					mz<<8,//0x10000,//darci->WorldPos.Z,
					0,
					0,0,FLAG2_PERSON_FAKE_WANDER);

				TO_THING(index)->Genus.Person->InsideRoom=(Random()%20)+20;
			}
//#endif
			//
			// Load in all the eventpoints.
			//

			for (i = 0; i < MAX_EVENTPOINTS; i++)
			{
				if (FileRead(handle, &event_point,         14)     == FILE_READ_ERROR) goto file_error;
				if (FileRead(handle, &event_point.Data[0], 14*4+4) == FILE_READ_ERROR) goto file_error;

				if (event_point.Used)
				{
					//
					// Create a blank definitions to start off with.
					//

					ecd.type      = EWAY_COND_TRUE;
					ecd.negate    = (event_point.Flags>>1)&1;
					ecd.arg1      = 0;
					ecd.arg2      = 0;
					ecd.bool_arg1 = NULL;
					ecd.bool_arg2 = NULL;

					es.type = EWAY_STAY_ALWAYS;
					es.arg  = 0;

					ed.type      = EWAY_DO_NOTHING;
					ed.subtype   = 0;
					ed.arg1      = 0;
					ed.arg2      = 0;

					ee.pcom_ai   = 0;
					ee.pcom_bent = 0;
					ee.pcom_move = 0;
					ee.pcom_has  = 0;
					ee.ai_other  = 0;
					ee.zone      = 0;

					kludge_index = 0;

					//
					// Convert from the mission editor eventpoint definition to the
					// game waypoint stuff.
					//

					ew_colour  = event_point.Colour;
					ew_group   = event_point.Group;
					ew_world_x = event_point.X;
					if (event_point.Flags&WPT_FLAGS_INSIDE) {
						ew_world_y = get_inside_alt(event_point.Y);
					} else {
						ew_world_y = event_point.Y;
					}
					ew_world_z = event_point.Z;
					ew_yaw     = ((128+event_point.Direction) << 3) & 2047;

					//
					// This is how a waypoint is activated.
					//

					switch(event_point.TriggeredBy)
					{
						case TT_NONE:
							break;

						case TT_DEPENDENCY:
							ecd.type  = EWAY_COND_DEPENDENT;
							ecd.arg1  = event_point.EPRef;
							break;

						case TT_RADIUS:
							ecd.type  = EWAY_COND_PROXIMITY;
							ecd.arg1  = event_point.Radius;
							break;

						case TT_DOOR:
							break;

						case TT_TRIPWIRE:
							ecd.type = EWAY_COND_TRIPWIRE;
							break;

						case TT_PRESSURE_PAD:
							ecd.type = EWAY_COND_PRESSURE;
							break;

						case TT_ELECTRIC_FENCE:
							break;

						case TT_WATER_LEVEL:
							break;

						case TT_SECURITY_CAMERA:
							ecd.type = EWAY_COND_CAMERA;
							break;

						case TT_SWITCH:
							ecd.type = EWAY_COND_SWITCH;
							break;

						case TT_ANIM_PRIM:
							break;

						case TT_TIMER:
							ecd.type  = EWAY_COND_TIME;
							ecd.arg1  = event_point.Radius;		// Radius is the time for time triggers.
							break;

						case TT_SHOUT_ALL:
							break;

						case TT_BOOLEANAND:
						
							//
							// Booleans are always dependencies.
							//

							ecd1.type   = EWAY_COND_DEPENDENT;
							ecd1.arg1   = event_point.EPRef;
							ecd1.negate = FALSE;

							ecd2.type   = EWAY_COND_DEPENDENT;
							ecd2.arg1   = event_point.EPRefBool;
							ecd2.negate = FALSE;

							ecd.type      =  EWAY_COND_BOOL_AND;
							ecd.bool_arg1 = &ecd1;
							ecd.bool_arg2 = &ecd2;

							break;

						case TT_BOOLEANOR:
							
							//
							// Booleans are always dependencies.
							//

							ecd1.type   = EWAY_COND_DEPENDENT;
							ecd1.arg1   = event_point.EPRef;
							ecd1.negate = FALSE;

							ecd2.type   = EWAY_COND_DEPENDENT;
							ecd2.arg1   = event_point.EPRefBool;
							ecd2.negate = FALSE;

							ecd.type      =  EWAY_COND_BOOL_OR;
							ecd.bool_arg1 = &ecd1;
							ecd.bool_arg2 = &ecd2;

							break;

						case TT_ITEM_HELD:
							ecd.type = EWAY_COND_ITEM_HELD;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_ITEM_SEEN:
							break;

						case TT_KILLED:
							ecd.type = EWAY_COND_PERSON_DEAD;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_SHOUT_ANY:
							break;

						case TT_COUNTDOWN:
							ecd.type = EWAY_COND_COUNTDOWN;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.Radius;
							break;

						case TT_ENEMYRADIUS:
							ecd.type = EWAY_COND_PERSON_NEAR;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.Radius / 64;
							break;

						case TT_VISIBLECOUNTDOWN:
							ecd.type = EWAY_COND_COUNTDOWN_SEE;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.Radius;
							break;

						case TT_CUBOID:
							ecd.type = EWAY_COND_PLAYER_CUBOID;
							ecd.arg1 = event_point.Radius & 0xffff;
							ecd.arg2 = event_point.Radius >> 16;
							break;

						case TT_HALFDEAD:
							ecd.type = EWAY_COND_HALF_DEAD;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_PERSON_SEEN:
							ecd.type = EWAY_COND_A_SEE_B;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.EPRefBool;
							break;

						case TT_PERSON_USED:
							ecd.type = EWAY_COND_PERSON_USED;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_PLAYER_USES_RADIUS:
							ecd.type = EWAY_COND_RADIUS_USED;
							ecd.arg1 = event_point.Radius;
							break;

						case TT_GROUPDEAD:
							ecd.type = EWAY_COND_GROUP_DEAD;
							break;

						case TT_PRIM_DAMAGED:
							ecd.type = EWAY_COND_PRIM_DAMAGED;
							break;

						case TT_PERSON_ARRESTED:
							ecd.type = EWAY_COND_PERSON_ARRESTED;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_CONVERSATION_OVER:
							ecd.type = EWAY_COND_CONVERSE_END;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_COUNTER:
							ecd.type = EWAY_COND_COUNTER_GTEQ;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.Radius;
							break;

						case TT_KILLED_NOT_ARRESTED:
							ecd.type = EWAY_COND_KILLED_NOT_ARRESTED;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_CRIME_RATE_ABOVE:
							ecd.type = EWAY_COND_CRIME_RATE_GTEQ;
							ecd.arg1 = event_point.Radius / 100;
							break;

						case TT_CRIME_RATE_BELOW:
							ecd.type = EWAY_COND_CRIME_RATE_LTEQ;
							ecd.arg1 = event_point.Radius / 100;
							break;

						case TT_PERSON_IS_MURDERER:
							ecd.type = EWAY_COND_IS_MURDERER;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_PERSON_IN_VEHICLE:
							ecd.type = EWAY_COND_PERSON_IN_VEHICLE;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.EPRefBool;
							break;

						case TT_THING_RADIUS_DIR:
							ecd.type = EWAY_COND_THING_RADIUS_DIR;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.Radius / 64;
							break;

						case TT_SPECIFIC_ITEM_HELD:
							ecd.type = EWAY_COND_SPECIFIC_ITEM_HELD;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_RANDOM:
							ecd.type  = EWAY_COND_RANDOM;
							ecd.arg1  = event_point.EPRef;
							break;

						case TT_PLAYER_FIRES_GUN:
							ecd.type = EWAY_COND_PLAYER_FIRED_GUN;
							break;

						case TT_DARCI_GRABBED:
							ecd.type = EWAY_COND_DARCI_GRABBED;
							break;

						case TT_PUNCHED_AND_KICKED:
							ecd.type = EWAY_COND_PUNCHED_AND_KICKED;
							ecd.arg1 = event_point.EPRef;
							break;

						case TT_MOVE_RADIUS_DIR:
							ecd.type = EWAY_COND_MOVE_RADIUS_DIR;
							ecd.arg1 = event_point.EPRef;
							ecd.arg2 = event_point.Radius / 64;
							break;

						default:
							ASSERT(0);
							break;
					}

					//
					// This is what the waypoint does when its activated.
					//

					switch(event_point.WaypointType)
					{
						case WPT_NONE:
							break;

						case WPT_SIMPLE:
							ed.type = EWAY_DO_NOTHING;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_CREATE_PLAYER:

							ed.type = EWAY_DO_CREATE_PLAYER;

							switch(event_point.Data[0])
							{
								default:
								case PT_DARCI:
									ed.subtype = PLAYER_DARCI; 
									break;
								case PT_ROPER: 
									ed.subtype = PLAYER_ROPER;
									break;
								case PT_COP:
									ed.subtype = PLAYER_COP;
									break;
								case PT_GANG:
									ed.subtype = PLAYER_THUG;
									break;
							}

							ed.subtype |= (event_point.Data[1] ? 8 : 0);

							break;

						case WPT_CREATE_ENEMIES:

							ed.type = EWAY_DO_CREATE_ENEMY;

							if (version >= 3)
							{
								enemy_type  = event_point.Data[0] & 0xffff;
								enemy_count = event_point.Data[0] >> 16;
								follow      = event_point.Data[1];
							}
							else
							{
								enemy_type  = event_point.Data[0];
								enemy_count = event_point.Data[1];
								follow      = NULL;
							}

							//
							// The HIWORD of Data[2] contains the HAS_* flags.
							// 

							ee.pcom_has = 0;

							if ((event_point.Data[2] >> 16) & HAS_PISTOL)	{ee.pcom_has |= PCOM_HAS_GUN;}
							if ((event_point.Data[2] >> 16) & HAS_SHOTGUN)	{ee.pcom_has |= PCOM_HAS_SHOTGUN;}
							if ((event_point.Data[2] >> 16) & HAS_AK47)		{ee.pcom_has |= PCOM_HAS_AK47;}
							if ((event_point.Data[2] >> 16) & HAS_GRENADE)	{ee.pcom_has |= PCOM_HAS_GRENADE;}
							if ((event_point.Data[2] >> 16) & HAS_BALLOON)	{ee.pcom_has |= PCOM_HAS_BALLOON;}
							if ((event_point.Data[2] >> 16) & HAS_KNIFE)	{ee.pcom_has |= PCOM_HAS_KNIFE;}
							if ((event_point.Data[2] >> 16) & HAS_BAT)		{ee.pcom_has |= PCOM_HAS_BASEBALLBAT;}

							switch(enemy_type)
							{
								case ET_CIV:
									ed.subtype = PERSON_CIV;
									break;

								case ET_CIV_BALLOON:
									ed.subtype   = PERSON_CIV;
									ee.pcom_has |= PCOM_HAS_BALLOON;
									break;

								case ET_SLAG:
									ed.subtype = PERSON_SLAG_TART;
									break;

								case ET_UGLY_FAT_SLAG:
									ed.subtype = PERSON_SLAG_FATUGLY;
									break;

								case ET_WORKMAN:
									ed.subtype = PERSON_MECHANIC;
									break;

								case ET_GANG_RASTA:
									ed.subtype = PERSON_THUG_RASTA;
									break;

								case ET_GANG_RED:
									ed.subtype = PERSON_THUG_RED;
									break;

								case ET_GANG_GREY:
									ed.subtype = PERSON_THUG_GREY;
									break;

								case ET_GANG_RASTA_PISTOL:
									ed.subtype   = PERSON_THUG_RASTA;
									ee.pcom_has |= PCOM_HAS_GUN;
									break;

								case ET_GANG_RED_SHOTGUN:
									ed.subtype   = PERSON_THUG_RED;
									ee.pcom_has |= PCOM_HAS_SHOTGUN;
									break;

								case ET_GANG_GREY_AK47:
									ed.subtype   = PERSON_THUG_GREY;
									ee.pcom_has |= PCOM_HAS_AK47;
									break;

								case ET_COP:
									ed.subtype = PERSON_COP;
									break;

								case ET_COP_PISTOL:
									ed.subtype   = PERSON_COP;
									ee.pcom_has |= PCOM_HAS_GUN;
									break;

								case ET_COP_SHOTGUN:
									ed.subtype   = PERSON_COP;
									ee.pcom_has |= PCOM_HAS_SHOTGUN;
									break;

								case ET_COP_AK47:
									ed.subtype   = PERSON_COP;
									ee.pcom_has |= PCOM_HAS_AK47;
									break;

								case ET_HOSTAGE:
									ed.subtype = PERSON_HOSTAGE;
									break;

								case ET_WORKMAN_GRENADE:
									ed.subtype   = PERSON_MECHANIC;
									ee.pcom_has |= PCOM_HAS_GRENADE;
									break;
								case ET_TRAMP:
									ed.subtype   = PERSON_TRAMP;
									break;

								case ET_MIB1:
									ed.subtype   = PERSON_MIB1;
									break;
								case ET_MIB2:
									ed.subtype   = PERSON_MIB2;
									break;
								case ET_MIB3:
									ed.subtype   = PERSON_MIB3;
									break;

								case ET_NONE:
									ed.subtype = PERSON_CIV;
									break;

								case ET_DARCI:
									ed.subtype = PERSON_DARCI;
									break;

								case ET_ROPER:
									ed.subtype = PERSON_ROPER;
									break;

								default:
									ASSERT(0);
									break;
							}

							ee.pcom_ai   = LOWORD(event_point.Data[5]);
							ee.ai_skill  = HIWORD(event_point.Data[5]);
							ee.pcom_bent = event_point.Data[4];
							ee.pcom_move = event_point.Data[3] + 1;
							ee.ai_other  = event_point.Data[7] & 0xffff;	// For PCOM_AI_BODYGUARD and PCOM_AI_ASSASIN, the ID of the waypoint that creates the person you guard/kill
							ee.follow    = follow;				// For PCOM_MOVE_FOLLOW
							ee.zone      = event_point.Data[4] >> 8;

extern	SWORD	people_types[50];

							people_types[ed.subtype]++;

							if (ed.subtype == PERSON_MIB1 ||
								ed.subtype == PERSON_MIB2 ||
								ed.subtype == PERSON_MIB3)
							{
								//
								// MIB never have a low skill...
								// 

								ee.ai_skill = 8 + (ee.ai_skill >> 1);
							}

							if (ee.pcom_ai == PCOM_AI_FIGHT_TEST)
							{
								//
								// The other is the combat move that knocks out this person.
								//

								ee.ai_other = event_point.Data[7] >> 16;

								//
								// Fight test dummies are always invulnerable.
								// 

								ee.zone |= 1 << 4; // ugh!
							}

							//
							// What does this person drop when he dies?
							//

							ee.drop = 0;							

							for (j = 0; j < 32; j++)
							{
								if (event_point.Data[8] & (1 << j))
								{
									ee.drop = j + 1;
								}
							}



							break;

						case WPT_CREATE_VEHICLE:

							ed.type = EWAY_DO_CREATE_VEHICLE;
							ed.arg1 = 0;

							//
							// What key to unlock the vehicle?
							// 

							switch(event_point.Data[3])
							{
								case VK_UNLOCKED: ed.arg1 = SPECIAL_NONE;      break;
								case VK_RED:	  ed.arg1 = SPECIAL_KEY;	   break;
								case VK_BLUE:	  ed.arg1 = SPECIAL_KEY;	   break;
								case VK_GREEN:	  ed.arg1 = SPECIAL_KEY;	   break;
								case VK_BLACK:	  ed.arg1 = SPECIAL_KEY;	   break;
								case VK_WHITE:	  ed.arg1 = SPECIAL_KEY;	   break;
								case VK_LOCKED:	  ed.arg1 = SPECIAL_NUM_TYPES; break;	// A special that doesn't exist

								default:
									ASSERT(0);
									break;
							}

							switch(event_point.Data[0])
							{
								case VT_NONE:
								case VT_CAR:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_CAR;
									break;

								case VT_VAN:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_VAN;
									break;

								case VT_TAXI:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_TAXI;
									break;

								case VT_HELICOPTER:

									ed.subtype = EWAY_SUBTYPE_VEHICLE_HELICOPTER;

									if (event_point.Data[1] == VMT_TRACK_TARGET)
									{
										//
										// arg2 is who to track... for a helicopter.
										//

										ed.arg2 = event_point.Data[2];
									}

									break;

								case VT_BIKE:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_BIKE;
									break;

								case VT_POLICE:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_POLICE;
									break;

								case VT_AMBULANCE:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_AMBULANCE;
									break;

								case VT_JEEP:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_JEEP;
									break;

								case VT_MEATWAGON:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_MEATWAGON;
									break;

								case VT_SEDAN:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_SEDAN;
									break;

								case VT_WILDCATVAN:
									ed.subtype = EWAY_SUBTYPE_VEHICLE_WILDCATVAN;
									break;

								default:
									ASSERT(0);
									break;
							}

							break;

						case WPT_CREATE_ITEM:

							if (version < 4)
							{	
								//
								// This is using the old list of hundreds of items.
								//

								event_point.Data[0] = IT_PISTOL;
							}

							ed.type = EWAY_DO_CREATE_ITEM;
							ed.arg1 = event_point.Data[2]; // flags

#ifdef	MIKE
							if(!(ed.arg1&EWAY_ARG_ITEM_FOLLOW_PERSON))
							{
								switch(event_point.Data[0])
								{
									case IT_BARREL:
										break;
									case	0:
										break;

									default:
										TesterText("(WP)%d item %s at %d,%d,%d \n",i,witem_strings[event_point.Data[0]-1],event_point.X>>8,event_point.Y>>8,event_point.Z>>8);
										break;
								}
							}
#endif							

							switch(event_point.Data[0])
							{
								default:
								case IT_NONE:

								case IT_KEY:          ed.subtype = SPECIAL_KEY;          break;
								case IT_PISTOL:	      ed.subtype = SPECIAL_GUN;          break;
								case IT_HEALTH:       ed.subtype = SPECIAL_HEALTH;       break;
								case IT_SHOTGUN:      ed.subtype = SPECIAL_SHOTGUN;      break;
								case IT_KNIFE:        ed.subtype = SPECIAL_KNIFE;        break;
								case IT_AK47:         ed.subtype = SPECIAL_AK47;	     break;
								case IT_MINE:         ed.subtype = SPECIAL_MINE;         break;
								case IT_BASEBALLBAT:  ed.subtype = SPECIAL_BASEBALLBAT;  break;
								case IT_AMMO_SHOTGUN: ed.subtype = SPECIAL_AMMO_SHOTGUN; break;
								case IT_AMMO_AK47:    ed.subtype = SPECIAL_AMMO_AK47;    break;
								case IT_AMMO_PISTOL:  ed.subtype = SPECIAL_AMMO_PISTOL;  break;
								case IT_KEYCARD:	  ed.subtype = SPECIAL_KEYCARD;		 break;
								case IT_FILE:		  ed.subtype = SPECIAL_FILE;		 break;
								case IT_FLOPPY_DISK:  ed.subtype = SPECIAL_FLOPPY_DISK;  break;
								case IT_CROWBAR:	  ed.subtype = SPECIAL_CROWBAR;      break;
								case IT_VIDEO:	      ed.subtype = SPECIAL_VIDEO;        break;
								case IT_GLOVES:	      ed.subtype = SPECIAL_GLOVES;       break;
								case IT_WEEDAWAY:     ed.subtype = SPECIAL_WEEDAWAY;     break;
								case IT_GRENADE:      ed.subtype = SPECIAL_GRENADE;		 break;
								case IT_EXPLOSIVES:   ed.subtype = SPECIAL_EXPLOSIVES;   break;
								case IT_WIRE_CUTTER:
									ed.type = EWAY_DO_END_OF_WORLD;
									break;

								case IT_BARREL:

									//
									// Don't create a waypoint- just put down some barrels.
									//

									for (j = 0; j < event_point.Data[1]; j++)
									{
										

										BARREL_alloc(
											BARREL_TYPE_NORMAL,
											PRIM_OBJ_BARREL,
											ew_world_x + (Random() & 0xf) - 0x7,
											ew_world_z + (Random() & 0xf) - 0x7,
											NULL);
									}

									goto dont_create_a_waypoint;
							}

							break;

						case WPT_CREATE_CREATURE:
							ed.type    = EWAY_DO_CREATE_ANIMAL;
							ed.subtype = event_point.Data[0];
							break;

						case WPT_CREATE_CAMERA:

							ed.type  = EWAY_DO_CAMERA_CREATE;

							ew_speed = event_point.Data[2] >> 2;
							ew_delay = event_point.Data[3];

							SATURATE(ew_speed, 4, 255);

							ed.arg1 = ew_speed;
							ed.arg2 = ew_delay;

							ed.subtype = 0;
							
							if (event_point.Data[4]) {ed.subtype |= EWAY_SUBTYPE_CAMERA_LOCK_PLAYER;}
							if (event_point.Data[5]) {ed.subtype |= EWAY_SUBTYPE_CAMERA_LOCK_DIRECTION;}
							if (event_point.Data[6]) {ed.subtype |= EWAY_SUBTYPE_CAMERA_CANT_INTERRUPT;}

							break;

						case WPT_CREATE_TARGET:

							ed.type = EWAY_DO_CAMERA_TARGET;

							switch(event_point.Data[1])
							{
								case NULL:
								case CT_NORMAL:
									ed.subtype = EWAY_SUBTYPE_CAMERA_TARGET_PLACE;
									break;

								case CT_ATTACHED:
									ed.subtype = EWAY_SUBTYPE_CAMERA_TARGET_THING;
									break;

								case CT_NEAREST_LIVING:
									ed.subtype = EWAY_SUBTYPE_CAMERA_TARGET_NEAR;
									break;

								default:
									ASSERT(0);
									break;
							}

							break;

						case WPT_CREATE_MAP_EXIT:
							break;

						case WPT_CAMERA_WAYPOINT:

							ed.type = EWAY_DO_CAMERA_WAYPOINT;

							ew_speed = event_point.Data[2] >> 2;
							ew_delay = event_point.Data[3];

							if (ew_speed < 4) {ew_speed = 4;}
							
							ed.arg1 = ew_speed;
							ed.arg2 = ew_delay;

							//
							// 'Camera waypoint' waypoints must have this condition.
							//

							ecd.type   = EWAY_COND_CAMERA_AT;
							ecd.arg1   = NULL;
							ecd.arg2   = NULL;
							ecd.negate = FALSE;

							break;

						case WPT_TARGET_WAYPOINT:
							break;

						case WPT_MESSAGE:
							ed.type    = EWAY_DO_MESSAGE;
							ed.arg1    = mess_count++;
							ed.arg2    = event_point.Data[2];
							ed.subtype = event_point.Data[1];	// The time the message lasts for.
							break;

						case WPT_NAV_BEACON:
							ed.type	   = EWAY_DO_NAV_BEACON;
							ed.arg1    = mess_count++;
							ed.arg2    = event_point.Data[1];
							break;
							
						case WPT_SOUND_EFFECT:
							ed.type    = EWAY_DO_SOUND_EFFECT;
							ed.subtype = event_point.Data[0];
							ed.arg1    = event_point.Data[1];
							break;

						case WPT_VISUAL_EFFECT:
							ed.type    = EWAY_DO_EXPLODE;
							ed.subtype = event_point.Data[0];
							ed.arg1    = event_point.Data[1];
							break;

						case WPT_SPOT_EFFECT:

							goto dont_create_a_waypoint;

							/*

							//
							// These are automatically generated nowadays...
							//

							ed.type	   = EWAY_DO_SPOT_FX;
							ed.subtype = event_point.Data[0];
							ed.arg1    = event_point.Data[1];

							*/

							break;

						case WPT_TELEPORT:
							break;

						case WPT_TELEPORT_TARGET:
							break;

						case WPT_END_GAME_LOSE:
							ed.type = EWAY_DO_MISSION_FAIL;
							break;

						case WPT_END_GAME_WIN:
							ed.type = EWAY_DO_MISSION_COMPLETE;
							break;

						case WPT_SHOUT:
							break;

						case WPT_ACTIVATE_PRIM:
							
							switch(event_point.Data[0])
							{
								case AP_DOOR:
									ed.type = EWAY_DO_CONTROL_DOOR;
									break;

								case AP_ELECTRIC_FENCE:
									ed.type = EWAY_DO_ELECTRIFY_FENCE;
									break;

								case AP_SECURITY_CAMERA:
									break;

								default:
									ASSERT(0);
									break;
							}

							break;

						case WPT_CREATE_TRAP:

							ed.type    = EWAY_DO_EMIT_STEAM;
							ed.subtype = event_point.Data[4];
							ed.arg1    = event_point.Data[3];
							ed.arg2    = 0;
							ed.arg2   |= (event_point.Data[1] & 0x3f) << 10;
							ed.arg2   |= (event_point.Data[2] & 0x0f) <<  6;
							ed.arg2   |= (event_point.Data[5] & 0x3f) <<  0;

							break;

						case WPT_ADJUST_ENEMY:

							ed.type      = EWAY_DO_CHANGE_ENEMY;
							ed.arg1      = event_point.Data[6];	// ID of waypoint that creates the person to adjust
							ee.pcom_ai   = event_point.Data[5];
							ee.pcom_bent = event_point.Data[4];
							ee.pcom_move = event_point.Data[3] + 1;
							ee.ai_other  = event_point.Data[7];	// For PCOM_AI_BODYGUARD/PCOM_AI_ASSASIN, the ID of the waypoint that creates the person you guard.

							if (ee.pcom_ai == PCOM_AI_FIGHT_TEST)
							{
								//
								// The other is the combat move that knocks out this person.
								//

								ee.ai_other = event_point.Data[7] >> 16;

								//
								// Fight test dummies are always invulnerable.
								// 

								ee.zone |= 1 << 4; // ugh!
							}

							if (version >= 3)
							{
								ee.follow = event_point.Data[1];
							}
							else
							{
								ee.follow = NULL;
							}

							break;

						case WPT_ENEMY_FLAGS:
							ed.type      = EWAY_DO_CHANGE_ENEMY_FLG;
							ed.arg1      = event_point.Data[0];	// ID of waypoint that creates the person to adjust
							ed.arg2		 = event_point.Data[4];
							break;

						case WPT_LINK_PLATFORM:

							ed.type = EWAY_DO_CREATE_PLATFORM;

							if (event_point.Data[0] == 0)
							{
								//
								// Use default speed.
								//

								ed.arg1 = 50;
							}
							else
							{
								ed.arg1 = event_point.Data[0];
							}

							if (event_point.Data[1] & LP_LOCK_TO_AXIS)  {ed.arg2 = PLAT_FLAG_LOCK_MOVE;}
							if (event_point.Data[1] & LP_LOCK_ROTATION) {ed.arg2 = PLAT_FLAG_LOCK_ROT;}
							if (event_point.Data[1] & LP_BODGE_ROCKET)  {ed.arg2 = PLAT_FLAG_BODGE_ROCKET;}

							break;

						case WPT_CREATE_BOMB:
							ed.type = EWAY_DO_CREATE_BOMB;
							break;

						case WPT_CREATE_BARREL:
							
							{
								UWORD barrel_type;
								UWORD prim;
							

								switch(event_point.Data[0])
								{
									case BT_OIL_DRUM:

										extern SLONG playing_level(const CBYTE *name);

										if (playing_level("Semtex.ucm"))
										{
											barrel_type = BARREL_TYPE_NORMAL;
											prim        = 145;

											break;
										}
										else
										{
											//
											// Fallthrough!
											//
										}


									case BT_BARREL:
									case BT_LOX_DRUM:
										barrel_type = BARREL_TYPE_NORMAL;
										prim        = PRIM_OBJ_BARREL;
										break;

									case BT_TRAFFIC_CONE:
										barrel_type = BARREL_TYPE_CONE;
										prim        = PRIM_OBJ_TRAFFIC_CONE;
										break;

									case BT_BURNING_BARREL:
										barrel_type = BARREL_TYPE_BURNING;
										prim        = PRIM_OBJ_BARREL;
										break;

									case BT_BURNING_BIN:
									case BT_BIN:
										barrel_type = BARREL_TYPE_BIN;
										prim        = PRIM_OBJ_BIN;
										break;

									default:
										ASSERT(0);
										break;
								}

								if (!(event_point.Flags & WPT_FLAGS_REFERENCED) && ecd.type == EWAY_COND_TRUE)
								{
									BARREL_alloc(
										barrel_type,
										prim,
										ew_world_x,
										ew_world_z,
										NULL);

									goto dont_create_a_waypoint;
								}
								else
								{
									ed.type    = EWAY_DO_CREATE_BARREL;
									ed.subtype = barrel_type;
									ed.arg2    = prim;
								}
							}

							break;

						case WPT_KILL_WAYPOINT:
							ed.type = EWAY_DO_KILL_WAYPOINT;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_CREATE_TREASURE:
							ed.type    = EWAY_DO_CREATE_ITEM;
							ed.subtype = SPECIAL_TREASURE;
#ifdef	MIKE
							TesterText(" POWERUP at %d,%d,%d \n",event_point.X>>8,event_point.Y>>8,event_point.Z>>8);
#endif
							break;

						case WPT_BONUS_POINTS:

							if (0)
							{
								//
								// Bonus points are just messages nowadays.
								//

								ed.type    = EWAY_DO_OBJECTIVE;
								ed.subtype = EWAY_SUBTYPE_OBJECTIVE_SUB;
								ed.arg1    = mess_count++;
								ed.arg2    = event_point.Data[1] / 10;
							}
							else
							{
								ed.type    = EWAY_DO_MESSAGE;
								ed.arg1    = mess_count++;
								ed.arg2    = NULL;	// Who says the message
								ed.subtype = 0;		// The time the message lasts for.
							}

							break;

						case WPT_GROUP_LIFE:
							ed.type = EWAY_DO_GROUP_LIFE;
							break;

						case WPT_GROUP_DEATH:
							ed.type = EWAY_DO_GROUP_DEATH;
							break;

						case WPT_CONVERSATION:

							if (event_point.Data[3])
							{
								ed.type = EWAY_DO_AMBIENT_CONV;
							}
							else
							{
								ed.type = EWAY_DO_CONVERSATION;
							}

							ed.subtype = mess_count++;
							ed.arg1    = event_point.Data[1];
							ed.arg2    = event_point.Data[2];
							break;

						case WPT_INCREMENT:
							ed.type    = EWAY_DO_INCREASE_COUNTER;
							ed.subtype = event_point.Data[1];
							break;

						case WPT_TRANSFER_PLAYER:
							ed.type = EWAY_DO_TRANSFER_PLAYER;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_AUTOSAVE:
							ed.type = EWAY_DO_AUTOSAVE;
							break;

						case WPT_MAKE_SEARCHABLE:
							
							//
							// Look for the nearest ob and flag it as searchable.
							//

							{
								OB_Info *oi = OB_find_index(
												ew_world_x,
												ew_world_y,
												ew_world_z,
												0x100,
												FALSE);

								if (oi)
								{
									OB_ob[oi->index].flags |= OB_FLAG_SEARCHABLE;
								}
							}

							goto dont_create_a_waypoint;

						case WPT_LOCK_VEHICLE:
							ed.type    = EWAY_DO_LOCK_VEHICLE;
							ed.arg1    = event_point.Data[0];

							if (event_point.Data[1])
							{
								ed.subtype = EWAY_SUBTYPE_VEHICLE_LOCK;
							}
							else
							{
								ed.subtype = EWAY_SUBTYPE_VEHICLE_UNLOCK;
							}

							break;

						case WPT_CUT_SCENE:
							{
								ed.type    = EWAY_DO_CUTSCENE;
								ed.arg1    = cutscene_count++;
							}
							break;
						case WPT_GROUP_RESET:
							ed.type = EWAY_DO_GROUP_RESET;
							break;

						case WPT_COUNT_UP_TIMER:
							ed.type    = EWAY_DO_VISIBLE_COUNT_UP;
							break;

						case WPT_RESET_COUNTER:
							ed.type    = EWAY_DO_RESET_COUNTER;
							ed.subtype = event_point.Data[0];
							break;

						case WPT_CREATE_MIST:
							ed.type = EWAY_DO_CREATE_MIST;
							break;

						case WPT_STALL_CAR:
							ed.type = EWAY_DO_STALL_CAR;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_EXTEND:
							ed.type = EWAY_DO_EXTEND_COUNTDOWN;
							ed.arg1 = event_point.Data[0];
							ed.arg2 = event_point.Data[1];
							break;

						case WPT_MOVE_THING:
							ed.type = EWAY_DO_MOVE_THING;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_MAKE_PERSON_PEE:
							ed.type = EWAY_DO_MAKE_PERSON_PEE;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_CONE_PENALTIES:
							ed.type = EWAY_DO_CONE_PENALTIES;
							break;

						case WPT_SIGN:
							ed.type = EWAY_DO_SIGN;
							ed.arg1 = event_point.Data[0];
							ed.arg2 = event_point.Data[1];
							break;

						case WPT_WAREFX:
							ed.type = EWAY_DO_WAREFX;
							ed.arg1 = event_point.Data[0];
							break;

						case WPT_NO_FLOOR:
							ed.type = EWAY_DO_NO_FLOOR;
							break;

						case WPT_SHAKE_CAMERA:
							ed.type = EWAY_DO_SHAKE_CAMERA;
							break;

						default:
							ASSERT(0);
							break;
					}

					switch(event_point.OnTrigger)
					{
						case OT_NONE:
						case OT_ACTIVE:
							es.type = EWAY_STAY_ALWAYS;
							break;

						case OT_ACTIVE_WHILE:
							es.type = EWAY_STAY_WHILE;
							break;

						case OT_ACTIVE_TIME:
							es.type = EWAY_STAY_TIME;
							es.arg  = event_point.AfterTimer;
							break;

						case OT_ACTIVE_DIE:
							es.type = EWAY_STAY_DIE;
							break;

						default:
							ASSERT(0);
							break;
					}

					//
					// Unreferenced when COND_PERSON DO_MESSAGE waypoints should
					// be set to whenever triggered.
					//

					if (ecd.type == EWAY_COND_PERSON_USED &&
						ed.type  == EWAY_DO_MESSAGE)
					{
						if (!(event_point.Flags & WPT_FLAGS_REFERENCED))
						{
							es.type = EWAY_STAY_WHILE;
						}
					}
//					if(0)

					if (save_psx || ENV_get_value_number("iamapsx", FALSE))
					{
						//
						// Skip waypoints marked as optional. We do it here so that
						// the message numbers set properly.
						//

						if (event_point.Flags & WPT_FLAGS_OPTIONAL)
						{
							continue;
						}
					}


					//
					// Create the waypoint.
					// 


					EWAY_create(
						i,
						ew_colour,
						ew_group,
						ew_world_x,
						ew_world_y,
						ew_world_z,
						ew_yaw,
					   &ecd,
					   &ed,
					   &es,
					   &ee,
						!(event_point.Flags & WPT_FLAGS_REFERENCED),
						kludge_index,
						event_point.Data[9]);

				  dont_create_a_waypoint:;

					continue;
				}
				else
				{
					continue;
				}

//			  abandon_waypoint:;
#ifndef	PSX
#ifndef TARGET_DC
				{
					CBYTE title[256];

					//
					// Tell the user about not loading the waypoint.
					//

					sprintf(title, "Error loading waypoint %d", i);

					MessageBox(
						hDDLibWindow,
						error,
						title,
						MB_OK | MB_ICONERROR | MB_APPLMODAL);
				}
#endif
#endif
			}
		}


		// skip the skill level junk
		if (version>5)
			FileSeek(handle,SEEK_MODE_CURRENT,254); // last one is now boredom rate...
		FileRead(handle,&BOREDOM_RATE,1);
		if (version<10) BOREDOM_RATE=4;

		if(BOREDOM_RATE<4)
			BOREDOM_RATE=4;

		if (version>8) 
		{
//			FileSeek(handle,SEEK_MODE_CURRENT,1);
			FileRead(handle,&FAKE_CARS,1);
			FileRead(handle,&MUSIC_WORLD,1);
			if (MUSIC_WORLD<1) MUSIC_WORLD=1;
		}

		//
		// Load in the messages and other extra data
		//

		if (version<8) { // only messages

			for (i = 0; i < mess_count; i++)
			{
				SLONG l;

				ZeroMemory(junk,sizeof(junk));

				if (version>4) FileRead(handle,&l,4); else l=_MAX_PATH;
				if (FileRead(handle, junk, l) == FILE_READ_ERROR) goto file_error;
				
				//
				// Tell the EWAY module what each message is.
				//

				EWAY_set_message(i, junk);
			}

		} else {

			for (i = 0; i < mess_count+cutscene_count; i++)
			{
				SLONG l;
				UBYTE what;

				FileRead(handle,&what,1);

				switch (what) {
				case 1: // message
					ZeroMemory(junk,sizeof(junk));
					FileRead(handle,&l,4); 
					if (FileRead(handle, junk, l) == FILE_READ_ERROR) goto file_error;
					//
					// Tell the EWAY module what each message is.
					//
					EWAY_set_message(i, junk);
					break;
				case 2: // cutscene
					//PLAYCUTS_cutscenes[PLAYCUTS_cutscene_ctr++]=PLAYCUTS_Read(handle);
					PLAYCUTS_Read(handle); // don't care bout result -- static alloc
					break;
				}
			}

		}

		if (version >= 2)
		{
			//
			// Load in the zone squares.
			//

			for (x = 0; x < 128; x++)
			{
				if (FileRead(handle, junk, 128) == FILE_READ_ERROR) goto file_error;

				for (z = 0; z < 128; z++)
				{
					PAP_2HI(x,z).Flags &= ~PAP_FLAG_ZONE1;
					PAP_2HI(x,z).Flags &= ~PAP_FLAG_ZONE2;
					PAP_2HI(x,z).Flags &= ~PAP_FLAG_ZONE3;
					PAP_2HI(x,z).Flags &= ~PAP_FLAG_ZONE4;

					if (junk[z] & ZF_ZONE1) {PAP_2HI(x,z).Flags |= PAP_FLAG_ZONE1;}
					if (junk[z] & ZF_ZONE2) {PAP_2HI(x,z).Flags |= PAP_FLAG_ZONE2;}
					if (junk[z] & ZF_ZONE3) {PAP_2HI(x,z).Flags |= PAP_FLAG_ZONE3;}
					if (junk[z] & ZF_ZONE4) {PAP_2HI(x,z).Flags |= PAP_FLAG_ZONE4;}

					if (junk[z] & ZF_NO_WANDER)
					{
						if (PAP_2HI(x,z).Flags & PAP_FLAG_HIDDEN)
						{
							//
							// For hidden squares, the PAP_FLAG_WANDER flag means something else!
							//
						}
						else
						{
							PAP_2HI(x,z).Flags &= ~PAP_FLAG_WANDER;
						}
					}

					if (junk[z] & ZF_NO_GO)
					{
						void MAV_turn_off_whole_square(
								SLONG x,
								SLONG z);

						void MAV_turn_off_whole_square_car(
								SLONG x,
								SLONG z);

						MAV_turn_off_whole_square(x,z);
						MAV_turn_off_whole_square_car(x,z);

						PAP_2HI(x,z).Flags |= PAP_FLAG_NOGO;
					}
					else
					{
						PAP_2HI(x,z).Flags &= ~PAP_FLAG_NOGO;
					}
				}
			}
		}

		//
		// Finish with the file.
		//

		FileClose(handle);

		//
		// The music world is taken from this file now!
		//

		{
			FILE *handle = MF_Fopen("levels\\mworlds.txt", "rb");

			if (handle)
			{
				CBYTE line[256];
				CBYTE levname[64];
				SLONG match;
				SLONG mworld;

				CBYTE *ch;
				CBYTE *blah;

				for (ch = fname_level; *ch && *ch != '\\'; ch++);

				ch += 1;
				blah = levname;

				while(*ch != '.')
				{
					*blah++ = *ch++;
				}

				*blah++ = ':';
				*blah++ = ' ';
				*blah++ = '%';
				*blah++ = 'd';
				*blah++ = '\000';

				_strlwr(levname);

				while(fgets(line, 256, handle))
				{
					_strlwr(line);

					match = sscanf(line, levname, &mworld);

					if (match == 1)
					{
						MUSIC_WORLD = mworld;

						break;
					}
				}

				MF_Fclose(handle);
			}
		}


		//
		// No more waypoints to be created.
		//

		EWAY_created_last_waypoint();
		EWAY_work_out_which_ones_are_in_warehouses();

		//
		// create fake cars (must do this after the "real" cars are created)
		//

extern SLONG WAND_find_good_start_point_for_car(SLONG* posx, SLONG* posz, SLONG* yaw, SLONG anywhere);
	
		if(FAKE_CARS && save_psx)
		{
			FAKE_CARS=(FAKE_CARS+1)>>1;
		}
		else
		if(FAKE_CARS)
		if(!the_display.CurrDevice->IsHardware())
		{
			FAKE_CARS=(FAKE_CARS+1)>>1;
		}

		for (i = 0; i < FAKE_CARS; i++)
		{
			SLONG	x,z,yaw;
			SLONG	watchdog;


			watchdog = 16;
			while (!WAND_find_good_start_point_for_car(&x, &z, &yaw, 1))
			{
				if (!--watchdog)	break;
			}
			if (!watchdog)	continue;

			SLONG	ix = PCOM_create_person(
				PERSON_CIV, 
				0, 
				0, 
				PCOM_AI_DRIVER, 
				0, 
				0, 
				PCOM_MOVE_WANDER, 
				0, 
				0, 
				0, 
				0, 
				0, 
				x << 8, 
				0, 
				z << 8, 
				0, 
				0, 
				0, 
				FLAG2_PERSON_FAKE_WANDER);

			TO_THING(ix)->Genus.Person->sewerbits = (Random()%20)+20;

			SLONG	type = (Random() >> 4) &15;//% (sizeof(vehicles) / sizeof(vehicles[0]));

			SLONG p_index = VEH_create(
					x << 8,
					0,
					z << 8,
					yaw,
					0,
					0,
					vehicle_random[type],
					0,
					Random());

  			WMOVE_create(TO_THING(p_index));

		}

		//
		// Sort out the water-related flags.
		//

		for (x = 0; x < PAP_SIZE_LO; x++)
		for (z = 0; z < PAP_SIZE_LO; z++)
		{
			PAP_2LO(x,z).water = PAP_LO_NO_WATER;

			for (x = 1; x < PAP_SIZE_HI - 1; x++)
			for (z = 1; z < PAP_SIZE_HI - 1; z++)
			{
				if (PAP_2HI(x,z).Flags & PAP_FLAG_WATER)
				{
					PAP_2HI(x + 0, z + 0).Flags |= PAP_FLAG_SINK_POINT| PAP_FLAG_WATER;
					PAP_2HI(x + 1, z + 0).Flags |= PAP_FLAG_SINK_POINT;
					PAP_2HI(x + 0, z + 1).Flags |= PAP_FLAG_SINK_POINT;
					PAP_2HI(x + 1, z + 1).Flags |= PAP_FLAG_SINK_POINT;

					//PAP_2HI(x - 1, z - 1).Flags |= PAP_FLAG_REFLECTIVE;
					//PAP_2HI(x + 0, z - 1).Flags |= PAP_FLAG_REFLECTIVE;
					//PAP_2HI(x + 1, z - 1).Flags |= PAP_FLAG_REFLECTIVE;
					//PAP_2HI(x - 1, z + 0).Flags |= PAP_FLAG_REFLECTIVE;
					//PAP_2HI(x - 1, z + 1).Flags |= PAP_FLAG_REFLECTIVE;

					PAP_2LO(x >> 2, z >> 2).water = water_level >> 3;	// Hard-coded water level.
				}
			}

			for (x = 0; x < PAP_SIZE_LO - 1; x++)
			for (z = 0; z < PAP_SIZE_LO - 1; z++)
			{
				//
				// All watery obs at the water level.
				//

				for (oi = OB_find(x,z); oi->prim; oi += 1)
				{
					if (PAP_2HI(oi->x >> 8, oi->z >> 8).Flags & PAP_FLAG_WATER)
					{
						oi->y = water_level - get_prim_info(oi->prim)->miny;
					}
				}
			}
		}
	}
	else
	{
	  file_error:;

		load_ok = FALSE;

		//
		// Finish with the file.
		//

		if (handle)
		{
			FileClose(handle);
		}
	}

	//
	// If there was an error loading the level, then we just
	// put down the player on a map.
	//

	if (!load_ok)
	{
		if(NO_PLAYERS==0)
			NO_PLAYERS = 1;


/*
		if(ShiftFlag)
			NO_PLAYERS = 2;  // sneak in an extra player
		else
			NO_PLAYERS = 1;
*/

		//
		// Plonk down the players.
		//

		x = 64 << 8;
		z = 64 << 8;

		//
		// Initilaises the players.
		//

		for (i = 0; i < NO_PLAYERS; i++)
		{
			angle = (2048 * i) / NO_PLAYERS;

			tx = x + (SIN(angle) >> 9);
			tz = z + (COS(angle) >> 9);
			
			ty = PAP_calc_height_at(tx,tz);

			NET_PERSON(i) = create_player(
								PLAYER_DARCI,
								tx,ty,tz,
								i);
		}
	}
	else
	{
void load_level_anim_prims(void);
		load_level_anim_prims();
	}



	//
	// the CRIME_RATE stuff is now done be EWAY_created_last_waypoint()
	//
}
#endif
#endif


void save_dreamcast_wad(CBYTE *fname);
void load_dreamcast_wad(CBYTE *fname);


SLONG	quick_load=0;
UBYTE loading_screen_active;
//
// Initialises the game using the given files.
// Any of the files can be NULL except the map file.
//



// The stuff that is common to DC and PC.
void ELEV_game_init_common(
		CBYTE *fname_map,
		CBYTE *fname_lighting,
		CBYTE *fname_citsez,
		CBYTE *fname_level)
{
extern void SND_BeginAmbient();
	MFX_load_wave_list();
	SND_BeginAmbient();
}


#ifdef TARGET_DC
//#if 1


SLONG ELEV_game_init(
		CBYTE *fname_map,
		CBYTE *fname_lighting,
		CBYTE *fname_citsez,
		CBYTE *fname_level)
{
	SLONG i;

	// Fix the language-specific stuff by loading the correct DAD file.
	char cTempName[50];
	ASSERT ( fname_level[0] == 'l' );
	ASSERT ( fname_level[1] == 'e' );
	ASSERT ( fname_level[2] == 'v' );
	ASSERT ( fname_level[3] == 'e' );
	ASSERT ( fname_level[4] == 'l' );
	ASSERT ( fname_level[5] == 's' );
	ASSERT ( fname_level[6] == '\\' );

	switch ( ENV_get_value_number ( "lang_num", 0, "" ) )
	{
	case 0:
		// English.
		strcpy ( cTempName, "levels\\" );
		break;
	case 1:
		// French.
		strcpy ( cTempName, "levels_french\\" );
		break;
	default:
		ASSERT ( FALSE );
		break;
	}

	ASSERT ( strlen ( cTempName ) + strlen ( &(fname_level[7]) ) < 45 );

	strcat ( cTempName, &(fname_level[7]) );

	load_dreamcast_wad(cTempName);

	init_user_interface();

	ELEV_game_init_common ( fname_map, fname_lighting, fname_citsez, fname_level );

	return TRUE;
}



#else



#if !defined(PSX)
SLONG ELEV_game_init(
		CBYTE *fname_map,
		CBYTE *fname_lighting,
		CBYTE *fname_citsez,
		CBYTE *fname_level)
{
	SLONG i;

	/*

	extern UWORD darci_dlight;

	darci_dlight = 0;

	*/

	ATTRACT_loadscreen_draw(10 * 256 / 100);

	//
	// Remember the last map loaded.
	//

	strcpy(ELEV_last_map_loaded, fname_map);

	//
	// Sets the prim arrays to be just after all the person points
	// and faces.
	//

//	revert_to_prim_status();

	//
	// Marks all the prim_objects as unloaded.
	//

	mark_prim_objects_as_unloaded();

	//
	// Added by MikeD, anything could happen from here on in
	//
void global_load(void);
	global_load();

	ATTRACT_loadscreen_draw(15 * 256 / 100);

	//
	// So we stay in synch!
	//

	extern SLONG kick_or_punch;	// This is in person.cpp

	kick_or_punch = 0;

	//
	// Initialise everything.
	//

	void init_anim_prims(void);
	void	init_overlay(void);

	init_overlay();
	WMOVE_init();
	PCOM_init();
	init_things();
	init_draw_meshes();
//	init_draw_tweens(); // this is in global_load()

	init_anim_prims();
	init_persons();
	init_choppers();
	init_pyros();
//	init_furniture();
	init_vehicles();
	init_projectiles();
	init_specials();
	BAT_init();
void	init_gangattack(void);
	init_gangattack();
	init_map();
	init_user_interface();

	ATTRACT_loadscreen_draw(20 * 256 / 100);

	SOUND_reset();
	PARTICLE_Reset();
//	TRACKS_Reset();
	TRACKS_InitOnce();
	RIBBON_init();
#ifndef	PSX
	load_palette("data\\tex01.pal");

	//
	// The PC panel widescreen text...
	//

	extern CBYTE PANEL_wide_text[256];
	extern THING_INDEX PANEL_wide_top_person;
	extern THING_INDEX PANEL_wide_bot_person;

	PANEL_wide_text[0]    = 0;
	PANEL_wide_top_person = NULL;
	PANEL_wide_bot_person = NULL;

#endif
	load_animtmaps();

	ATTRACT_loadscreen_draw(25 * 256 / 100);

	FC_init();
	#ifdef BIKE
	BIKE_init();
	#endif
	BARREL_init();
	BALLOON_init();
	NIGHT_init();
	OB_init();
	TRIP_init();
	FOG_init();
	MIST_init();
//	WATER_init();
	PUDDLE_init();
//	AZ_init();
	DRIP_init();
//	BANG_init();
	GLITTER_init();
	POW_init();
	SPARK_init();
	CONSOLE_clear();
	PLAT_init();
	MAV_init();
	PANEL_new_text_init();

	ATTRACT_loadscreen_draw(35 * 256 / 100);

	//
	// static's in sound.cpp
	//
void	init_ambient(void);
	init_ambient();
	
extern void MAP_beacon_init();
extern void MAP_pulse_init();

	MAP_pulse_init();
	MAP_beacon_init();

	//
	// Load the map.
	// 

	load_game_map(fname_map);

	calc_prim_info();

	ATTRACT_loadscreen_draw(50 * 256 / 100);

	//
	// Add the facets to the mapwho and walkable faces to the mapwho.
	//

	build_quick_city();

	// tum te tum
	init_animals();

	// what? function call? i din... oh, _that_ function call

	//
	// Process the prim data.
	//

	TRACE("Q1\n");

	//
	// We have to call DIRT_init() after we've loaded the map because
	// it looks for trees to put clumps of leaves around.
	// 

	DIRT_init(100, 1, 0, INFINITY, INFINITY, INFINITY, INFINITY);

	//
	// Map processing.
	//

	OB_convert_dustbins_to_barrels();
	ROAD_sink();
	ROAD_wander_calc();
	WAND_init();
	WARE_init();
	MAV_precalculate();
	extern void BUILD_car_facets();
	BUILD_car_facets();
	SHADOW_do();
//	AZ_create_lines();
	COLLIDE_calc_fastnav_bits();
	COLLIDE_find_seethrough_fences();
	clear_all_wmove_flags();
	InitGrenades();

	// now the map's loaded we can precalc audio polys, if req'd
#ifdef USE_A3D
	SOUNDENV_precalc();
#endif
	SOUND_SewerPrecalc();

	//
	// Load in the lighting.
	//

	if (fname_lighting == NULL || !NIGHT_load_ed_file(fname_lighting))
	{
		//
		// Default lighting.
		//

		NIGHT_ambient(255, 255, 255, 110, -148, -177);
	}

	TRACE("Q2\n");

	NIGHT_generate_walkable_lighting();

	//
	// Always rain at night.
	//

	if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
	{
		GAME_FLAGS |= GF_RAINING;
	}

	ATTRACT_loadscreen_draw(60 * 256 / 100);

	//
	// Load the level.
	// 

	ELEV_load_level(fname_level);

	ATTRACT_loadscreen_draw(65 * 256 / 100);

#ifndef	PSX
	TEXTURE_fix_prim_textures();
//	TEXTURE_fix_texture_styles();

	AENG_create_dx_prim_points();

	extern void envmap_specials(void);

	envmap_specials();

#endif

	calc_prim_info();
#ifndef PSX
	calc_prim_normals();
	find_anim_prim_bboxes();
#endif

	loading_screen_active = TRUE;

	if(!quick_load)
	{
		TEXTURE_load_needed(fname_level,  0, 256, 400);

		extern void PACK_do(void);

		// PACK_do();
	}

	loading_screen_active = FALSE;

	EWAY_process(); //pre process map, stick it here Or we get stack overflow
//	MUSIC_WORLD=(Random()%6)+1;



// MARK! THIS CODE ISN'T CALLED FOR THE DC!!!!
#if 0

extern void SND_BeginAmbient();

	MFX_load_wave_list();
	SND_BeginAmbient();

#else
	ELEV_game_init_common ( fname_map, fname_lighting, fname_citsez, fname_level );
#endif

	ATTRACT_loadscreen_draw(95 * 256 / 100);

	//
	// Load what the citizen's say.
	//

	EWAY_load_fake_wander_text(fname_citsez);

	//
	// Add the walkable faces in the objects to the mapwho. We do this after ELEV_load_level()
	// because some of the objects are converted to moving platforms when we load a level.
	//

	OB_make_all_the_switches_be_at_the_proper_height();

	OB_add_walkable_faces();

	//
	// Work out which edges of the walkable faces we can grab.
	//

	calc_slide_edges();

	ATTRACT_loadscreen_draw(100 * 256 / 100);

	//
	// Set the camera to point at the player.
	//
#ifdef OLD_CAM

	CAM_set_focus(NET_PERSON(PLAYER_ID));


	//CAM_set_mode     (CAM_MODE_NORMAL);
	//CAM_set_zoom     (0x320);
	//CAM_set_behind_up(0x38000);
	//CAM_lens = CAM_LENS_WIDEANGLE;

	CAM_set_type(CAM_TYPE_WIDE);

	CAM_set_pos(
		NET_PERSON(PLAYER_ID)->WorldPos.X + 0x10000 >> 8,
		NET_PERSON(PLAYER_ID)->WorldPos.Y + 0x40000 >> 8,
		NET_PERSON(PLAYER_ID)->WorldPos.Z + 0x10000 >> 8);
#endif

	/*

	HOOK_init(
		NET_PERSON(PLAYER_ID)->WorldPos.X - 0x20000 >> 8,
		NET_PERSON(PLAYER_ID)->WorldPos.Z - 0x20000 >> 8);

	*/

	//
	// New camera.
	//
	if(CNET_network_game)
	{
				FC_look_at(0, THING_NUMBER(NET_PERSON(PLAYER_ID)));
	}
	else
	{

		for (i = 0; i < NO_PLAYERS; i++)
		{
			if (NET_PERSON(i))
			{
				FC_look_at(i, THING_NUMBER(NET_PERSON(i)));
				FC_setup_initial_camera(i);
			}
		}
	}

	/*
	{
		Thing *darci = NET_PERSON(0);

		darci_dlight = NIGHT_dlight_create(
							(darci->WorldPos.X >> 8),
							(darci->WorldPos.Y >> 8) + 0x80,
							(darci->WorldPos.Z >> 8),
							200,
							30,
							40,
							50);
	}
	*/

// debug camera
/*
	FC_look_at(1, 195);
	FC_setup_initial_camera(1);
*/


	// set boomboxes playing

/*	for (i = 1; i < OB_ob_upto; i++)
	{
		if (OB_ob[i].prim==61)
			MFX_play_xyz(0,S_TUNE_CLUB,MFX_LOOPED,OB_ob[i].x,OB_ob[i].y,OB_ob[i].z);
	}*/

	OB_Mapwho *om;
	OB_Ob     *oo;
	SLONG num;
	SLONG index;
	SLONG j;

	for (i=0;i<OB_SIZE-1;i++)
		for (j=0;j<OB_SIZE-1;j++)
		{
			om    = &OB_mapwho[i][j];
			index =  om->index;
			num   =  om->num;
			while(num--)
			{
				ASSERT(WITHIN(index, 1, OB_ob_upto - 1));
				oo = &OB_ob[index];
				if (oo->prim==61)
					MFX_play_xyz(0,S_ACIEEED,MFX_LOOPED|MFX_OVERLAP,((i << 10) + (oo->x << 2))<<8, oo->y<<8, ((j << 10) + (oo->z << 2))<<8);
				index++;
			}
			
		}


	//
	// Put puddles randomly around the city when its raining.
	//

	if (GAME_FLAGS & GF_RAINING)
	{
		PUDDLE_precalculate();
	}

	//
	// Insert colvects on the edge of water and the edges of sewer
	// entrances.
	//

	insert_collision_facets();

	#if GET_RID_OF_THE_BLOODY_MIST

	//
	// Put down some mist.
	//

	MIST_create(
		17,
		84,
		14914,
		13655,
		19010,
		17751);


	MIST_create(
		17,
		84,
		8787,
		4000,
		12883,
		8096);

	#endif

	TRACE("Q4\n");

	//
	// Clear all the keys!
	//

	Keys[KB_SPACE] = 0;
	Keys[KB_ENTER] = 0;
	Keys[KB_A]     = 0;
	Keys[KB_Z]     = 0;
	Keys[KB_X]     = 0;
	Keys[KB_C]     = 0;
	Keys[KB_V]     = 0;
	Keys[KB_LEFT]  = 0;
	Keys[KB_RIGHT] = 0;
	Keys[KB_UP]    = 0;
	Keys[KB_DOWN]  = 0;

	ATTRACT_loadscreen_draw(100 * 256 / 100);

	//save_dreamcast_wad(fname_level);

	#if TEST_DC

	extern UBYTE build_dc;

	if (build_dc)
	{
		extern void PACK_do(void);

		save_dreamcast_wad(fname_level);

#ifdef SAVE_MY_VQ_TEXTURES_PLEASE_BOB
		//PACK_do();
#endif
	}
	else
	{
		save_dreamcast_wad(fname_level);
	}

	#endif // TEST_DC

	return TRUE;

}
#endif
#endif


#ifndef PSX
//
// Fills the destination with the same filename (not the full path) of
// the source, except it has the given extension. (The extension does
// not include the '.')
//

void ELEV_create_similar_name(
		CBYTE *dest,
		CBYTE *src,
		CBYTE *ext)
{
	CBYTE *ch;
	CBYTE *ci;

	//
	// Find the start of the source filename.
	//

	for (ch = src; *ch; ch++);

	while(1)
	{
		if (ch == src)
		{
			break;
		}
		
		if (*ch == '\\')
		{
			ch++;

			break;
		}

		ch--;
	}

	//
	// Copy over the filename.
	//

	ci = dest;

	while(1)
	{
		*ci++ = *ch++;

		if (*ch == '.' || *ch == '\000')
		{
			break;
		}
	}

	//
	// Add the extension.
	// 

	*ci++ = '.';

	strcpy(ci, ext);
}




CBYTE ELEV_fname_map     [_MAX_PATH];
CBYTE ELEV_fname_lighting[_MAX_PATH];
CBYTE ELEV_fname_citsez  [_MAX_PATH];
CBYTE ELEV_fname_level   [_MAX_PATH];


SLONG ELEV_load_name(CBYTE *fname_level)
{
	SLONG ans;

	CBYTE *fname_map;
	CBYTE *fname_lighting;
	CBYTE *fname_citsez;

	MFFileHandle handle;



	// Play FMV now, when we have enough memory to do so!
	if (strstr(ELEV_fname_level, "Finale1.ucm"))
	{
		if (GAME_STATE & GS_REPLAY)
		{
			//
			// Don't play cutscenes on replay.
			//

		}
		else
		{
			stop_all_fx_and_music();
#ifdef TARGET_DC
			the_display.RunCutscene( 2, ENV_get_value_number("lang_num", 0, "" ) );
#else
			the_display.RunCutscene(2);
#endif

			// Reshow the "loading" screen.
			ATTRACT_loadscreen_init();

			// And play the loading music, coz it's all in memory.
			DCLL_memstream_play();

		}
	}



	//
	// Extract map, sewer and lighting filenames from the level file.
	//

	if (fname_level == NULL)
	{
		return FALSE;
	}

	handle = FileOpen(fname_level);

	if (handle == FILE_OPEN_ERROR)
	{
		//
		// Couldn't open file.
		//

		ASSERT ( FALSE );
		return FALSE;
	}

	strcpy(ELEV_fname_level, fname_level);	// I hope this is OK

	//
	// Load in the mission file.
	//

	char junk[1000];

	if (FileRead(handle, junk, sizeof(SLONG))  == FILE_READ_ERROR) goto file_error;	// Version number
	if (FileRead(handle, junk, sizeof(SLONG))  == FILE_READ_ERROR) goto file_error;	// Used
	if (FileRead(handle, junk, _MAX_PATH)	  == FILE_READ_ERROR) goto file_error;	// BriefName

	if (FileRead(handle,  ELEV_fname_lighting, _MAX_PATH) == FILE_READ_ERROR) goto file_error;	// LightMapName
	if (FileRead(handle,  ELEV_fname_map,      _MAX_PATH) == FILE_READ_ERROR) goto file_error;	// MapName
	if (FileRead(handle,  junk,                _MAX_PATH) == FILE_READ_ERROR) goto file_error;	// MissionName
	if (FileRead(handle,  ELEV_fname_citsez,   _MAX_PATH) == FILE_READ_ERROR) goto file_error;	// CitSezName

	FileClose(handle);

	//
	// Sort out NULL filenames.
	//

	fname_map      = (ELEV_fname_map     [0]) ? ELEV_fname_map      : NULL;
	fname_lighting = (ELEV_fname_lighting[0]) ? ELEV_fname_lighting : NULL;
	fname_citsez   = (ELEV_fname_citsez  [0]) ? ELEV_fname_citsez   : NULL;

	//
	// Do the load.
	//
#ifdef PSX
	printf("Map Name:%s\nLighting:%s\nCitsez:  %s\nLevel:   %s\n",fname_map,fname_lighting,fname_citsez,fname_level);
#endif
	ans = ELEV_game_init(
					fname_map,
					fname_lighting,
					fname_citsez,
					fname_level);


	return ans;

  file_error:;

	FileClose(handle);

	return TRUE;	
}

extern MFFileHandle	playback_file;

extern	CBYTE	tab_map_name[];

#endif

//extern SLONG EWAY_conv_active;
extern SLONG PSX_inv_open;

SLONG ELEV_load_user(SLONG mission)
{
#ifndef	PSX
	CBYTE *fname_map;
	CBYTE *fname_lighting;
	CBYTE *fname_citsez;
	CBYTE *fname_level;
	CBYTE  curr_directory[_MAX_PATH];

#ifndef TARGET_DC
	OPENFILENAME ofn;
#endif

	MFX_QUICK_stop(TRUE);
	MUSIC_mode(0);
	MUSIC_mode_process();




#ifndef TARGET_DC
try_again:;
#endif

/*
	if(mission<0)
	{
		//
		// bodge for publishing meeting
		//
			{
				SLONG	c0;
				strcpy(tab_map_name,my_mission_names[-mission]);
				for(c0=0;c0<strlen(tab_map_name);c0++)
				{
					if(tab_map_name[c0]=='.')
					{
						tab_map_name[c0+1]='t';
						tab_map_name[c0+2]='g';
						tab_map_name[c0+3]='a';

						break;
					}
				}
			}
		return ELEV_load_name(my_mission_names[-mission]);

	}
*/

	//
	// Using the GetOpenFileName() function changes the current directory,
	// so we must save and restore it.
	//

#ifdef TARGET_DC
	curr_directory[0] = '\0';
#else
	GetCurrentDirectory(_MAX_PATH, curr_directory);
#endif

	if(GAME_STATE&GS_PLAYBACK) {
		UWORD c;

		// marker to indicate level name is included
		FileRead(playback_file,&c,2);
		if (c==1) {
			CBYTE temp[_MAX_PATH];
			// restore string
			FileRead(playback_file,&c,2);
			FileRead(playback_file,temp,c);

			strcpy(ELEV_fname_level,curr_directory);
			if (ELEV_fname_level[strlen(ELEV_fname_level)-1]!='\\') strcat(ELEV_fname_level,"\\");
			strcat(ELEV_fname_level,temp);
			return ELEV_load_name(ELEV_fname_level);

			/*
			strcpy(ELEV_fname_level,temp);
			return ELEV_load_name(&ELEV_fname_level[23]);  // The stupid thing saves the absolute address
			*/
		}
	}





#ifdef OBEY_SCRIPT
	return ELEV_load_name(BRIEFING_mission_filename);
#endif

//extern CBYTE* STARTSCR_mission;
extern CBYTE STARTSCR_mission[_MAX_PATH];
	if (*STARTSCR_mission) 
	{
		//
		// need to record level name for Restart
		//
		strcpy(ELEV_fname_level,STARTSCR_mission);

		if(GAME_STATE&GS_RECORD)
		{
			UWORD c=1;
			CBYTE fname[_MAX_PATH],*cname;

			// marker to indicate level name is included
			FileWrite(playback_file,&c,2);
			// store string
			strcpy(fname,ELEV_fname_level);
			cname=fname;
			if (strnicmp(fname,curr_directory,strlen(curr_directory))==0) 
			{
			  cname+=strlen(curr_directory);
			  if (*cname='\\') cname++;
			}
			c=strlen(cname)+1; // +1 is to include terminating zero
			FileWrite(playback_file,&c,2);
			FileWrite(playback_file,cname,c);
		}

//		MemFree(STARTSCR_mission);
		SLONG res=ELEV_load_name(STARTSCR_mission);
		*STARTSCR_mission=0;
		return res;
	}

	
#ifdef TARGET_DC
	// Should never get here on DC.
	ASSERT ( FALSE );

#else //#ifdef TARGET_DC

	
	//
	// So we can see the dialog boxes!
	// 

	the_display.toGDI();

	//
	// Are we loading a combined level file or a bunch of separate files?
	//

	SLONG ans = MessageBox(
					hDDLibWindow,
					"Do you want to load a level file?",
					"Load a (map + lighting + citsez) file or a single level file",
					MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONQUESTION);

	switch(ans)
	{
		case IDYES:

			//
			// Get a level filename.
			//

			ELEV_fname_level[0] = 0;

			ofn.lStructSize       = sizeof(OPENFILENAME);
			ofn.hwndOwner         = hDDLibWindow;
			ofn.hInstance         = NULL;
			ofn.lpstrFilter       = "Level files\0*.ucm\0Wad files\0*.wad\0\0";
			ofn.lpstrCustomFilter = NULL;
			ofn.nMaxCustFilter    = 0;
			ofn.nFilterIndex      = 0;
			ofn.lpstrFile         = ELEV_fname_level;
			ofn.nMaxFile          = _MAX_PATH;
			ofn.lpstrFileTitle    = NULL;
			ofn.nMaxFileTitle     = 0;
			ofn.lpstrInitialDir   = "Levels";
			ofn.lpstrTitle        = "Load a level";
			ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nFileOffset       = 0;
			ofn.nFileExtension    = 0;
			ofn.lpstrDefExt       = "ucm";
			ofn.lCustData         = NULL;
			ofn.lpfnHook          = NULL;
			ofn.lpTemplateName    = NULL;

			if (!GetOpenFileName(&ofn))
			{
				return FALSE;
			}

			//
			// Restore our current directory.
			// 
			
			SetCurrentDirectory(curr_directory);

				
			if(GAME_STATE&GS_RECORD)
			{
				UWORD c=1;
				CBYTE fname[_MAX_PATH],*cname;

				// marker to indicate level name is included
				FileWrite(playback_file,&c,2);
				// store string
				strcpy(fname,ELEV_fname_level);
				cname=fname;
				if (strnicmp(fname,curr_directory,strlen(curr_directory))==0) 
				{
				  cname+=strlen(curr_directory);
				  if (*cname='\\') cname++;
				}
				c=strlen(cname)+1; // +1 is to include terminating zero
				FileWrite(playback_file,&c,2);
				FileWrite(playback_file,cname,c);
			}

			/*

			//
			// Load the level.
			//

			{
				SLONG	c0;
				strcpy(tab_map_name,ELEV_fname_level);
				for(c0=0;c0<strlen(tab_map_name);c0++)
				{
					if(tab_map_name[c0]=='.')
					{
						tab_map_name[c0+1]='t';
						tab_map_name[c0+2]='g';
						tab_map_name[c0+3]='a';

						break;
					}
				}
			}

			*/

			if(ELEV_fname_level[strlen(ELEV_fname_level)-3]=='w')
			{
				//
				// oh my god, crazy shit, it's a long shot but it just might work
				//
extern	void	load_whole_game(CBYTE	*gamename);

				load_whole_game(ELEV_fname_level);
				return(4);
			}
			else
			{
				if(ELEV_load_name(ELEV_fname_level))
					return(5);
				else
					return(0);
			}

			return (ELEV_load_name(ELEV_fname_level)?5:0);

		case IDNO:

			//
			// We have to get the map/lighting/and citsez files separately.
			//

			ELEV_fname_map[0] = 0;

			ofn.lStructSize       = sizeof(OPENFILENAME);
			ofn.hwndOwner         = hDDLibWindow;
			ofn.hInstance         = NULL;
			ofn.lpstrFilter       = "Game map files\0*.iam\0\0";
			ofn.lpstrCustomFilter = NULL;
			ofn.nMaxCustFilter    = 0;
			ofn.nFilterIndex      = 0;
			ofn.lpstrFile         = ELEV_fname_map;
			ofn.nMaxFile          = _MAX_PATH;
			ofn.lpstrFileTitle    = NULL;
			ofn.nMaxFileTitle     = 0;
			ofn.lpstrInitialDir   = "data";
			ofn.lpstrTitle        = "Load a game map";
			ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nFileOffset       = 0;
			ofn.nFileExtension    = 0;
			ofn.lpstrDefExt       = "iam";
			ofn.lCustData         = NULL;
			ofn.lpfnHook          = NULL;
			ofn.lpTemplateName    = NULL;

			if (!GetOpenFileName(&ofn))
			{
				goto	try_again;
//				return FALSE;
			}
			else
			{
				fname_map = ELEV_fname_map;
			}

			//
			// Create the default lighting filename.
			// 

			ELEV_create_similar_name(
				ELEV_fname_lighting,
				ELEV_fname_map,
				"lgt");

			//
			// Restore our current directory.
			// 
			
			SetCurrentDirectory(curr_directory);

			//
			// Get the lighting filename.
			// 
	

			ofn.lStructSize       = sizeof(OPENFILENAME);
			ofn.hwndOwner         = hDDLibWindow;
			ofn.hInstance         = NULL;
			ofn.lpstrFilter       = "Lighting files\0*.lgt\0\0";
			ofn.lpstrCustomFilter = NULL;
			ofn.nMaxCustFilter    = 0;
			ofn.nFilterIndex      = 0;
			ofn.lpstrFile         = ELEV_fname_lighting;
			ofn.nMaxFile          = _MAX_PATH;
			ofn.lpstrFileTitle    = NULL;
			ofn.nMaxFileTitle     = 0;
			ofn.lpstrInitialDir   = "data\\lighting";
			ofn.lpstrTitle        = "Load a lighting file";
			ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nFileOffset       = 0;
			ofn.nFileExtension    = 0;
			ofn.lpstrDefExt       = "lgt";
			ofn.lCustData         = NULL;
			ofn.lpfnHook          = NULL;
			ofn.lpTemplateName    = NULL;

			if (!GetOpenFileName(&ofn))
			{
				fname_lighting = NULL;
			}
			else
			{
				fname_lighting = ELEV_fname_lighting;
			}
			
			//
			// Create the default citsez filename.
			// 

			ELEV_create_similar_name(
				ELEV_fname_citsez,
				ELEV_fname_map,
				"txt");

			//
			// Restore our current directory.
			// 
			
			SetCurrentDirectory(curr_directory);

			//
			//  Get the sewer filename.
			//

			ofn.lStructSize       = sizeof(OPENFILENAME);
			ofn.hwndOwner         = hDDLibWindow;
			ofn.hInstance         = NULL;
			ofn.lpstrFilter       = "Text files\0*.txt\0\0";
			ofn.lpstrCustomFilter = NULL;
			ofn.nMaxCustFilter    = 0;
			ofn.nFilterIndex      = 0;
			ofn.lpstrFile         = ELEV_fname_citsez;
			ofn.nMaxFile          = _MAX_PATH;
			ofn.lpstrFileTitle    = NULL;
			ofn.nMaxFileTitle     = 0;
			ofn.lpstrInitialDir   = "\\text";
			ofn.lpstrTitle        = "Load a Citizen-sez text file";
			ofn.Flags             = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nFileOffset       = 0;
			ofn.nFileExtension    = 0;
			ofn.lpstrDefExt       = "txt";
			ofn.lCustData         = NULL;
			ofn.lpfnHook          = NULL;
			ofn.lpTemplateName    = NULL;

			if (!GetOpenFileName(&ofn))
			{
				fname_citsez = NULL;
			}
			else
			{
				fname_citsez = ELEV_fname_citsez;
			}

			//
			// We don't have a level.
			//
			
			fname_level = NULL;

			//
			// Restore our current directory.
			// 
			
			SetCurrentDirectory(curr_directory);

			//
			// Do the load.
			//

			return ELEV_game_init(
						fname_map,
						fname_lighting,
						fname_citsez,
						fname_level);

		case IDCANCEL:
			return FALSE;

		default:
			return FALSE;
	}

#endif //#else //#ifdef TARGET_DC




#else


#if 1

extern	void	load_whole_game(CBYTE	*gamename);
extern char *Wadmenu_AttractMenu(void);
extern UBYTE Wadmenu_Video;
extern int Wadmenu_Levelwon;
extern UBYTE Eidos_Played;
extern void Wadmenu_Introduction();

#ifndef VERSION_DEMO
	do
	{
#endif
		// Stop all sound effects playing
		MFX_OffKey=-1;
		MFX_render();
		DrawSync(0);
		PSXOverLay(OVERLAY_NAME,OVERLAY_SIZE);
		if (!Eidos_Played)
		{
			Wadmenu_Introduction();
		}

		if ((wad_level==33)&&Wadmenu_Levelwon)
			Wadmenu_Video=4;
		psx_game_name=Wadmenu_AttractMenu();
//		psx_game_name="levels\\level07\\level.nad";
		if (strcmp(psx_game_name,"MDEC")==0)
			psx_game_name=NULL;
#ifndef VERSION_DEMO
	}
	while(!psx_game_name);
#else
	if (psx_game_name==NULL)
		return 0;
#endif

	load_whole_game(psx_game_name);
//	load_whole_game("levels\\botanic1.wad");
	DIRT_init(100, 1, 0, INFINITY, INFINITY, INFINITY, INFINITY);
	InitGrenades();
	SPARK_init();

extern void PANEL_new_text_init();
extern void PANEL_new_widescreen_init();
	PANEL_new_text_init();
	PANEL_new_widescreen_init();
	MUSIC_init_level(wad_level);
	MFX_Init_Speech(wad_level);
	PSX_eog_timer=60;
	EWAY_conv_active=0;
	PSX_inv_open=0;

extern	void	init_record(SLONG level);

#ifndef FS_ISO9660
	init_record(wad_level);
#endif

	return(4);

//	return ELEV_load_name("levels\\psx_test.ucp");
#else
	CBYTE *fname_map="data\\jumper1.iam";
	CBYTE *fname_lighting="data\\lighting\\jumper1.lgt";
	CBYTE *fname_citsez=NULL;//"data\\gptest1.sew";
	CBYTE *fname_level=NULL; //"data\\gptest1.ucm";
	return ELEV_game_init(
				fname_map,
				fname_lighting,
				fname_citsez,
				fname_level);
#endif


#endif
}

void	reload_level(void)
{
#ifndef PSX
	 ELEV_load_name(ELEV_fname_level);
#else
extern	void	load_whole_game(CBYTE	*gamename);
extern  void Wadmenu_LoadingScreen(TIM_IMAGE *tim);
extern  void *mem_all;
extern  void setup_textures(int world);

	 TIM_IMAGE tim;

extern	void	end_record(void);

#ifndef FS_ISO9660
	end_record();
#endif

	 SetupMemory();
	 mem_all=0;
	 MFX_OffKey=-1;
	 MFX_render();
	 
	 ClearOTag(the_display.CurrentDisplayBuffer->ot,OTSIZE);
	 PSXOverLay(OVERLAY_NAME,OVERLAY_SIZE);
//	 setup_textures(0);
	 Wadmenu_LoadingScreen(&tim);
	 load_whole_game(psx_game_name);
	 DIRT_init(100, 1, 0, INFINITY, INFINITY, INFINITY, INFINITY);
	 InitGrenades();
	 SPARK_init();
extern void PANEL_new_text_init();
extern void PANEL_new_widescreen_init();
	 MUSIC_init_level(wad_level);
	 MFX_Init_Speech(wad_level);
	 PANEL_new_text_init();
	 PANEL_new_widescreen_init();
	 PSX_eog_timer=60;
	 EWAY_conv_active=0;
	 PSX_inv_open=0;

extern	void	init_playback(void);

//	 init_playback();
#endif

}
