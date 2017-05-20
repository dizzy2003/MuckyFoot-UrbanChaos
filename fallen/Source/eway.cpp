//
// Ugh... the whole game is based on waypoints.
//
#include	"demo.h"
#include "game.h"
#include "cam.h"
#include "eway.h"
#include "bang.h"
#include "ob.h"
#include "fmatrix.h"
#include "trip.h"
#include "dirt.h"
#include "thing.h"
#include "pow.h"
#include "c:\fallen\headers\music.h"
#ifndef	PSX
#include "font2d.h"
#include "C:\fallen\DDEngine\Headers\console.h"
#include "C:\fallen\DDEngine\Headers\map.h"
#endif
#include "chopper.h"
#include "animal.h"
#ifndef PSX
#include "drive.h"
#endif
#include "pcom.h"
#include "supermap.h"
#include "sound.h"
#include "statedef.h"
#include "wmove.h"
#ifdef		PSX 
#include "c:\fallen\psxeng\headers\psxeng.h"
#include "c:\fallen\psxeng\headers\panel.h"
#else
#include "aeng.h"
#include "panel.h"
#endif
#include "memory.h"
#include "dirt.h"
#include "cnet.h"
#include "mfx.h"
#include "fc.h"
#include "ware.h"
#include "statedef.h"
#include "door.h"
#include "interfac.h"
#include "psystem.h"
#include "poly.h"
#include "playcuts.h"
#include "xlat_str.h"
#include "mist.h"
#include "gamemenu.h"
#include	"c:\fallen\headers\env.h"


#ifdef TARGET_DC
#include "target.h"
#endif


extern	SLONG person_ok_for_conversation(Thing *p_person);
extern	ULONG	timer_bored; // I don't care I'm making the game better not the code!
//
// The conditions that are part of boolean operations.
//
extern	SLONG save_psx;
SLONG     EWAY_cond_upto;

EWAY_Cond	*EWAY_cond;//[EWAY_MAX_CONDS];
EWAY_Way	*EWAY_way; //[EWAY_MAX_WAYS];
EWAY_Edef	*EWAY_edef; //[EWAY_MAX_EDEFS];
CBYTE		**EWAY_mess; //[EWAY_MAX_MESSES];
CBYTE		*EWAY_mess_buffer; //[EWAY_MESS_BUFFER_SIZE];











#if 0
#define ANNOYINGSCRIBBLECHECK ScribbleCheck()
static void ScribbleCheck ( void )
{
	ASSERT ( prim_faces4[1].Points[0] >= 48 );
	ASSERT ( prim_faces4[1].Points[0] < 62 );
	ASSERT ( prim_faces4[1].Points[1] >= 48 );
	ASSERT ( prim_faces4[1].Points[1] < 62 );
	ASSERT ( prim_faces4[1].Points[2] >= 48 );
	ASSERT ( prim_faces4[1].Points[2] < 62 );
	ASSERT ( prim_faces4[1].Points[3] >= 48 );
	ASSERT ( prim_faces4[1].Points[3] < 62 );
}

#else
#define ANNOYINGSCRIBBLECHECK
#endif






//
// The waypoints.
//

#ifdef	PSX
//#define	ASSERT(x)	
extern	char *PANEL_wide_cont;
extern  char PANEL_wide_text[];

#endif


SLONG    EWAY_way_upto;

//
// The definitions of people we create.
//


SLONG     EWAY_edef_upto;


//
// Magic dodgy flagness

EWAY_Way *EWAY_magic_radius_flag;

//
// The messages.
//

SLONG EWAY_mess_buffer_upto;
SLONG EWAY_mess_upto;

//
// The time.
// 

SLONG EWAY_time_accurate;	 // 1600 ticks per second
SLONG EWAY_time;			 // 100  ticks per second
SLONG EWAY_tick;			 // The amount of time since the last process waypoints: (100 ticks per sec.)
SLONG EWAY_count_up;		 // The visible count-up timer...
UBYTE EWAY_count_up_visible; // Onscreen at the moment.

//
// The penalties incurred for the count-up timer.
// 

UBYTE EWAY_count_up_add_penalties;
SWORD EWAY_count_up_num_penalties;
UWORD EWAY_count_up_penalty_timer;

SLONG	EWAY_cam_jumped=0;

//
// The timers are indexed by the bottom byte of the arg of COUNTDOWN conditions.
//

UWORD *EWAY_timer;

//
// The counters.
//

UBYTE *EWAY_counter;

//
// The fake wandering people messages.
//

UWORD EWAY_fake_wander_text_normal_index;
UWORD EWAY_fake_wander_text_normal_number;
UWORD EWAY_fake_wander_text_guilty_index;
UWORD EWAY_fake_wander_text_guilty_number;
UWORD EWAY_fake_wander_text_annoyed_index;
UWORD EWAY_fake_wander_text_annoyed_number;

//
// The tutorial string.
//

CBYTE *EWAY_tutorial_string;
SLONG  EWAY_tutorial_counter;

//
// How the waypoint system knows when Darci pulls off a particular
// combat move.
//

UBYTE EWAY_darci_move;

//
// If non-NULL then this is the person who say any message!
//

UWORD EWAY_used_thing;	

//
// The cut-scene camera.
//

SLONG EWAY_cam_active;
SLONG EWAY_cam_goinactive;
SLONG EWAY_cam_x;		// Big coordinates...
SLONG EWAY_cam_y;
SLONG EWAY_cam_z;
SLONG EWAY_cam_dx;
SLONG EWAY_cam_dy;
SLONG EWAY_cam_dz;
SLONG EWAY_cam_yaw;
SLONG EWAY_cam_pitch;
SLONG EWAY_cam_want_yaw;
SLONG EWAY_cam_want_pitch;
SLONG EWAY_cam_waypoint;
SLONG EWAY_cam_target;
SLONG EWAY_cam_delay;
SLONG EWAY_cam_speed;
SLONG EWAY_cam_freeze;	// Stop the player moving.
SLONG EWAY_cam_cant_interrupt;
UWORD EWAY_cam_thing;
SLONG EWAY_cam_targx;
SLONG EWAY_cam_targy;
SLONG EWAY_cam_targz;
SLONG EWAY_cam_lens;	// 16-bit fixed point
SLONG EWAY_cam_warehouse;
SLONG EWAY_cam_lock;
SLONG EWAY_cam_last_yaw;
SLONG EWAY_cam_last_x;
SLONG EWAY_cam_last_y;
SLONG EWAY_cam_last_z;
SLONG EWAY_cam_skip;
SLONG EWAY_cam_last_dyaw;

UBYTE  GAME_cut_scene = 0;
CBYTE  EWAY_message[128];

//
// A conversation.
//

#define EWAY_CONV_TALK_A 0
#define EWAY_CONV_TALK_B 1

UBYTE		EWAY_conv_active;		// Bool. Is there a conversation going on?
UWORD		EWAY_conv_waypoint;		// The waypoint that set the conversation going.
THING_INDEX EWAY_conv_person_a;
THING_INDEX EWAY_conv_person_b;
UWORD       EWAY_conv_str;			// Index into the EWAY_mess_buffer
UWORD       EWAY_conv_str_count;	// 0 for first line etc
SLONG       EWAY_conv_timer;		// How long until the next person talks.
SLONG       EWAY_conv_skip;			// How long until the user is allowed to skip the conversation.
SLONG		EWAY_conv_ambient;		// TRUE => Don't control the camera or go into widescreen mode.
SLONG		EWAY_conv_talk=0;

#ifndef	PSX
extern	CBYTE ELEV_fname_level[_MAX_PATH];

void	get_level_word(CBYTE *str)
{
	SLONG	c0=0,c1=0;


	
	while(ELEV_fname_level[c0]!='\\' && c0<101)
	{
		c0++;
	}
	ASSERT(c0<99);
	c0++;
	while(ELEV_fname_level[c0]!='.' && c0<151)
	{
		str[c1++]=ELEV_fname_level[c0];
		c0++;
	}
	str[c1]=0;

}

SLONG	playing_combat_tutorial(void)
{
#ifdef VERSION_DEMO
//	return 0;	
#endif

 	SLONG	c0=0,c1=0;
//	if(save_psx)
//		return(0);
	
//	ASSERT(0);
	while(ELEV_fname_level[c0]!='\\' && c0<101)
	{
		c0++;
	}
	if(c0>99)
		return(0);
	ASSERT(c0<99);
	c0++;

	if(!strcmp(&ELEV_fname_level[c0],"FTutor1.ucm"))
	{
		return(1);
	}
	else
	if(!strcmp(&ELEV_fname_level[c0],"fight1.ucm"))
	{
		return(1);
	}
	else
	if(!strcmp(&ELEV_fname_level[c0],"fight2.ucm"))
	{
		return(1);
	}
	else
	{
		return(0);
	}

}

SLONG	playing_level(const CBYTE *name)
{
 	SLONG	c0=0,c1=0;
#ifdef VERSION_DEMO
//	return 0;	
#endif


#ifdef	PSX
	return(0);
#endif



//	if(save_psx)
//		return(0);
	
//	ASSERT(0);
	while(ELEV_fname_level[c0]!='\\' && c0<101)
	{
		c0++;
	}
	if(c0>99)
		return(0);
	ASSERT(c0<99);
	c0++;

	if(!strcmp(&ELEV_fname_level[c0],name))
	{
		return(1);
	}
	else
	{
		return(0);
	}

}

CBYTE	*crap_levels[]=
{
	"FTutor1.ucm",
	"Assault1.ucm",
	"testdrive1a.ucm",
	"fight1.ucm",
	"testdrive2.ucm",
	"fight2.ucm",
	"testdrive3.ucm",
	// Er... why can't people have times for these?
	//"freecd1.ucm",
	//"Album1.ucm",
	0



};
SLONG	playing_real_mission(void)
{


 	SLONG	c0=0,c1=0;
	
	while(ELEV_fname_level[c0]!='\\' && c0<101)
	{
		c0++;
	}
	if(c0>99)
		return(0);
	ASSERT(c0<99);
	c0++;

	while(crap_levels[c1])
	{
		if(!strcmp(&ELEV_fname_level[c0],crap_levels[c1]))
		{
			return(0);
		}
		c1++;
	}
	return(1);

}
#else
SLONG	playing_combat_tutorial(void)
{
	return (wad_level==1);
}

UBYTE PSX_real[]={0,0,0,0,1,0,0,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0};

SLONG	playing_real_mission(void)
{
	return PSX_real[wad_level-1];
}
#endif
Thing	*talk_thing;

//
// Play the wav for the current mission with waypoint number waypoint
//
void	EWAY_talk(ULONG	waypoint)
{

ANNOYINGSCRIBBLECHECK;

#ifndef	PSX
	if(save_psx)
		return;
	CBYTE	str[100];
	CBYTE	level[100];

	//
	// generate a filename for the wav,  filename is    missionname_waypointnumber.wav
	//


	get_level_word(level);
#ifdef TARGET_DC
extern char *pcSpeechLanguageDir;
	sprintf(str,"%s%s%s.ucm%d.wav",GetSpeechPath(),pcSpeechLanguageDir,level,waypoint);
#else
	sprintf(str,"%stalk2\\%s.ucm%d.wav",GetSpeechPath(),level,waypoint);
#endif
/*
	if(!FileExists(str))
	{
		sprintf(str,"talk\\%s_%02d.wav",level,waypoint); //old method
	}
*/
	if (MUSIC_is_playing())
	{
		MFX_QUICK_stop();
	}

	MFX_QUICK_wait();

	extern void  DCLL_stream_volume(float volume);	// 0.0F <= volume <= 1.0F

	DCLL_stream_volume(1.0F);

	MFX_QUICK_play(str, FALSE);
	EWAY_conv_talk=0;
#else
	MFX_Conv_wait();
	MFX_Conv_play(waypoint,0,0);
	EWAY_conv_talk=0;

#endif

ANNOYINGSCRIBBLECHECK;

}

void	check_eway_talk(SLONG stop)
{
	if(!MFX_QUICK_still_playing())
		talk_thing=0;

	if(talk_thing)
	{
		if(stop||talk_thing->State==STATE_HIT_RECOIL||talk_thing->State==STATE_DEAD||talk_thing->State==STATE_DYING)
		{
			MFX_QUICK_stop();
			talk_thing=NULL;
		}

	}
}

//
// Play a conversation wav  (these have conversation sub parts in the files as a tag of A,B,C,D etc
//
void	EWAY_talk_conv(ULONG	waypoint,SLONG conversation)
{

ANNOYINGSCRIBBLECHECK;


#ifndef	PSX
	if(save_psx)
		return;
	CBYTE	str[100];
	CBYTE	level[100];

	//
	// generate a filename for the wav, filename is    missionname_waypointnumber_SUBPART.wav   S
	//
	talk_thing=NULL;
	get_level_word(level);
#ifdef TARGET_DC
extern char *pcSpeechLanguageDir;
	sprintf(str,"%s%s%s.ucm%d%c.wav",GetSpeechPath(),pcSpeechLanguageDir,level,waypoint,65+conversation);
#else
	sprintf(str,"%stalk2\\%s.ucm%d%c.wav",GetSpeechPath(),level,waypoint,65+conversation);
#endif
/*
	if(!FileExists(str))
	{
		sprintf(str,"talk\\%s_%02d_%c.wav",level,waypoint,65+conversation);

	}
*/

	if (MUSIC_is_playing())
	{
		MFX_QUICK_stop();
	}

	MFX_QUICK_wait();

	extern void  DCLL_stream_volume(float volume);	// 0.0F <= volume <= 1.0F

	DCLL_stream_volume(1.0F);

	Thing* speaker = TO_THING(EWAY_conv_person_a);
	EWAY_conv_talk=MFX_QUICK_play(str, FALSE, speaker->WorldPos.X >> 8, speaker->WorldPos.Y >> 8, speaker->WorldPos.Z >> 8);
#else
	MFX_Conv_wait();
	EWAY_conv_talk=MFX_Conv_play(waypoint,1,conversation);
#endif


ANNOYINGSCRIBBLECHECK;

}
#ifndef	FINAL
//#define	ASSERT(x) if(!(x)){asm("break 0");}
#endif

CBYTE	*EWAY_get_mess(SLONG index)
{
	ASSERT(index<EWAY_mess_upto);
	ASSERT(EWAY_mess[index]>=&EWAY_mess_buffer[0] && EWAY_mess[index]<&EWAY_mess_buffer[EWAY_mess_buffer_upto]);
	ASSERT(EWAY_mess[index]!=(CBYTE*)0x3c003c00);
	ASSERT(index>=0);
	return(EWAY_mess[index]);
}
#ifndef	PSX
#ifndef TARGET_DC
void EWAY_init()
{

ANNOYINGSCRIBBLECHECK;


	//
	// Clear all the waypoints.
	// 
	talk_thing=0;

	memset((UBYTE*)EWAY_way, 0, sizeof(EWAY_Way)*EWAY_MAX_WAYS);

	EWAY_way_upto = 1;

	//
	// Clear the people definitions.
	//

	memset((UBYTE *)EWAY_edef, 0, sizeof(EWAY_Edef)*EWAY_MAX_EDEFS);

	EWAY_edef_upto = 1;

	//
	// Clear the messages.
	//

	EWAY_mess_buffer_upto = 0; //EWAY_mess_buffer;
	EWAY_mess_upto        = 0;

	memset((UBYTE*)EWAY_mess, 0, sizeof(CBYTE *)*EWAY_MAX_MESSES);

	//
	// Clear the conditions.
	//

	memset((UBYTE*)EWAY_cond, 0, sizeof(EWAY_Cond)*EWAY_MAX_CONDS);

	EWAY_cond_upto = 1;

	//
	// Clear the timers.
	//

	memset((UBYTE *)EWAY_timer, 0, sizeof(UWORD)*EWAY_MAX_TIMERS);

	//
	// Clear camera and conversation stuff.
	//

	EWAY_cam_active  = FALSE;
	EWAY_conv_active = FALSE;

	//
	// Clear the counters.
	//

	memset(EWAY_counter, 0, sizeof(UBYTE) * EWAY_MAX_COUNTERS);

ANNOYINGSCRIBBLECHECK;


}
#endif
#endif

//
// Create a player.
//

UWORD EWAY_create_player(
		UBYTE subtype,
		UBYTE yaw,
		SLONG has,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z)
{
	UWORD p_index;

	//
	// Make sure we haven't got too many players.
	// 

	ASSERT(WITHIN(NO_PLAYERS, 0, 1));

	//
	// Make sure the player is on the ground.
	//

	switch(subtype&7)
	{
		case PLAYER_DARCI:

			p_index = PCOM_create_player(
						PLAYER_DARCI,
						has,
						world_x,
						world_y,
						world_z,
						NO_PLAYERS,
						yaw << 3);

			NET_PERSON(NO_PLAYERS) = TO_THING(p_index);

			NET_PLAYER(NO_PLAYERS)->Genus.Player->Strength	  =the_game.DarciStrength;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Constitution=the_game.DarciConstitution;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Stamina	  =the_game.DarciStamina;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Skill		  =the_game.DarciSkill;

			break;

		case PLAYER_ROPER:
			
			p_index = PCOM_create_player(
						PLAYER_ROPER,
						has,
						world_x,
						world_y,
						world_z,
						NO_PLAYERS,
						yaw << 3);

			NET_PERSON(NO_PLAYERS) = TO_THING(p_index);

			NET_PLAYER(NO_PLAYERS)->Genus.Player->Strength	  =the_game.DarciStrength;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Constitution=the_game.DarciConstitution;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Stamina	  =the_game.DarciStamina;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Skill		  =the_game.DarciSkill;
/*
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Strength	  =the_game.RoperStrength;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Constitution=the_game.RoperConstitution;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Stamina	  =the_game.RoperStamina;
			NET_PLAYER(NO_PLAYERS)->Genus.Player->Skill		  =the_game.RoperSkill;
*/

			break;

		case PLAYER_COP: 
			
			p_index = PCOM_create_player(
						PLAYER_COP,
						has,
						world_x,
						world_y,
						world_z,
						NO_PLAYERS,
						yaw << 3);

			NET_PERSON(NO_PLAYERS) = TO_THING(p_index);

			break;

		case PLAYER_THUG:
			
			p_index = PCOM_create_player(
						PLAYER_THUG,
						has,
						world_x,
						world_y,
						world_z,
						NO_PLAYERS,
						yaw << 3);

			NET_PERSON(NO_PLAYERS) = TO_THING(p_index);

			break;

		default:
			ASSERT(0);
			break;
	}

	NO_PLAYERS += 1;

	return p_index;
}


//
// Creates an enemy.
// 

UWORD EWAY_create_enemy(
		UBYTE subtype,
		UBYTE yaw,
		UBYTE colour,
		UBYTE group,
		UBYTE pcom_ai,
		UBYTE pcom_bent,
		UBYTE pcom_move,
		UBYTE drop,
		UBYTE zone,
		SLONG skill,
		UWORD follow,
		UWORD other,
		SLONG has,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG random)
{
	THING_INDEX p_index;
	Thing      *p_person;
	ULONG	f1=0,f2=0;

ANNOYINGSCRIBBLECHECK;


	if (zone & (1 << 4)) {f2 |= FLAG2_PERSON_INVULNERABLE;}
	if (zone & (1 << 5)) {f2 |= FLAG2_PERSON_GUILTY;      }
	if (zone & (1 << 6)) 
	{
		f2 |= FLAG2_PERSON_FAKE_WANDER; 
	}

//	group_count[group]++;
	zone &= 0xf;

	p_index = PCOM_create_person(
				subtype,
				colour,
				group,
				pcom_ai,
				other,
				skill,
				pcom_move,
				follow,
				pcom_bent,
				has,
				drop,
				zone,				// We don't have zone information nowadays.
				world_x << 8,
				world_y << 8,
				world_z << 8,
				yaw << 3,
				random,f1,f2);

	p_person = TO_THING(p_index);

ANNOYINGSCRIBBLECHECK;

	return p_index;
}

UWORD EWAY_create_animal(
		UBYTE subtype,
		UBYTE yaw,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z)
{
	UWORD p_index;

	switch(subtype)
	{
		case EWAY_SUBTYPE_ANIMAL_BAT:
#ifndef	PSX
#ifdef	UNUSED
			if(!save_psx)
			{
				p_index = BAT_create(
							BAT_TYPE_BAT,
							world_x,
							world_z,
							yaw << 3);
			}
#endif
#endif
			break;

		case EWAY_SUBTYPE_ANIMAL_GARGOYLE:
			p_index = BAT_create(
						BAT_TYPE_GARGOYLE,
						world_x,
						world_z,
						yaw << 3);
			break;

		case EWAY_SUBTYPE_ANIMAL_BALROG:
			p_index = BAT_create(
						BAT_TYPE_BALROG,
						world_x,
						world_z,
						yaw << 3);
			break;

		case EWAY_SUBTYPE_ANIMAL_BANE:
			p_index = BAT_create(
						BAT_TYPE_BANE,
						world_x,
						world_z,
						yaw << 3);
			break;

		default:
			ASSERT(0);
			break;
	}

	return p_index;
}

//
// Creates an item of the given subtype.
//

UWORD EWAY_create_item(
		UBYTE subtype,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		EWAY_Way *ew)
{
	SLONG way_index;
	Thing	*p_thing;

ANNOYINGSCRIBBLECHECK;

	if (ew == NULL)
	{
		way_index = NULL;
	}
	else
	{
		way_index = ew - EWAY_way;
	}

	if (subtype == SPECIAL_MINE)
	{
		world_y = PAP_calc_map_height_at(world_x, world_z);
	}
	else
	{
		//
		// Don't create specials under ground...
		//
		
		SLONG ground = PAP_calc_map_height_at(world_x, world_z);

		if (world_y < ground + 0x40 && world_y > ground - 0xc0)

		if (WITHIN(world_y, ground - 0xc0, ground + 0x40))
		{
			world_y = ground + 0x40;
		}
	}

	p_thing=alloc_special(
		subtype,
		SPECIAL_SUBSTATE_NONE,
		world_x,
		world_y + 0x10,
		world_z,
		way_index);


	ASSERT(p_thing);
	if(p_thing)
	if(ew->ed.arg1&EWAY_ARG_ITEM_STASHED_IN_PRIM)
	{
		SLONG	ob_index;
		OB_Info *oi;
		oi=OB_find_index(world_x,world_y,world_z,2048,FALSE);
		if(oi)
		{
			ob_index=oi->index;

			OB_ob[ob_index].flags|=OB_FLAG_HIDDEN_ITEM|OB_FLAG_SEARCHABLE;
			p_thing->Genus.Special->counter=ob_index;

			remove_thing_from_map(p_thing);
			p_thing->Flags|=FLAG_SPECIAL_HIDDEN;
		}
	}

ANNOYINGSCRIBBLECHECK;

	return THING_NUMBER(p_thing);
}

//
// Creates a vehicle of the given subtype.
//

UWORD EWAY_create_vehicle(
		UBYTE subtype,
		UBYTE key,
		UWORD arg,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG yaw)
{
	Thing      *p_thing = NULL;
	THING_INDEX p_index = NULL;
//	ASSERT(0);

ANNOYINGSCRIBBLECHECK;

	switch(subtype)
	{
		case EWAY_SUBTYPE_VEHICLE_VAN:

			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,
						0,
						0,
						VEH_TYPE_VAN,
						key,
						Random());

//			if(!save_psx)
  			WMOVE_create(TO_THING(p_index));

			break;

		case EWAY_SUBTYPE_VEHICLE_HELICOPTER:

			{
				GameCoord posn;

				posn.X = world_x       << 8;
				posn.Z = world_z       << 8;
				posn.Y = world_y + 265 << 8;

				p_thing = CHOPPER_create(
							posn,
							CHOPPER_CIVILIAN);

				if (arg)
				{
					p_thing->Genus.Chopper->victim = arg;
				}

				p_index = THING_NUMBER(p_thing);
			}

			break;

		case EWAY_SUBTYPE_VEHICLE_BIKE:

#ifdef BIKE
#ifndef PSX
			p_index = BIKE_create(
						world_x,
						world_z,
						yaw);
#endif
#endif
			break;

		case EWAY_SUBTYPE_VEHICLE_CAR:

			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_CAR,
						key,
						Random());
#ifndef PSX
			if(!save_psx)
				WMOVE_create(TO_THING(p_index));
#endif
			break;

		case EWAY_SUBTYPE_VEHICLE_TAXI:

			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_TAXI,
						key,
						Random());
#ifndef PSX
			if(!save_psx)
				WMOVE_create(TO_THING(p_index));
#endif
			break;

		case EWAY_SUBTYPE_VEHICLE_POLICE:
			
			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_POLICE,
						key,
						Random());
#ifndef PSX
			if(!save_psx)
				WMOVE_create(TO_THING(p_index));
#endif
			break;

		case EWAY_SUBTYPE_VEHICLE_AMBULANCE:
			
			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_AMBULANCE,
						key,
						Random());

//			if(!save_psx)
				WMOVE_create(TO_THING(p_index));

			break;

		case EWAY_SUBTYPE_VEHICLE_JEEP:
			
			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_JEEP,
						key,
						Random());
#ifndef PSX
			if(!save_psx)
				WMOVE_create(TO_THING(p_index));
#endif
			break;

		case EWAY_SUBTYPE_VEHICLE_MEATWAGON:
			
			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_MEATWAGON,
						key,
						Random());
#ifndef PSX
			if(!save_psx)
				WMOVE_create(TO_THING(p_index));
#endif
			break;

		case EWAY_SUBTYPE_VEHICLE_SEDAN:

			ANNOYINGSCRIBBLECHECK;

			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,0,0,
						VEH_TYPE_SEDAN,
						key,
						Random());

			ANNOYINGSCRIBBLECHECK;


#ifndef PSX
			if(!save_psx)
				WMOVE_create(TO_THING(p_index));
#endif

			ANNOYINGSCRIBBLECHECK;


			break;

		case EWAY_SUBTYPE_VEHICLE_WILDCATVAN:

			p_index = VEH_create(
						world_x         << 8,
						world_y + 0x100 << 8,	// Add a mapsquare to make it fall to the ground.
						world_z         << 8,
						yaw,
						0,
						0,
						VEH_TYPE_WILDCATVAN,
						key,
						Random());



//			if(!save_psx)
  			WMOVE_create(TO_THING(p_index));

			break;

		default:
			ASSERT(0);
			break;
	}

ANNOYINGSCRIBBLECHECK;

	return p_index;
}

#ifndef PSX
#ifndef TARGET_DC

//
// Converts a condition definition into a EWAY_Cond.
//

SLONG ob_x;
SLONG ob_y;
SLONG ob_z;
SLONG ob_yaw;
SLONG ob_prim;
SLONG ob_index;

EWAY_Cond EWAY_create_cond(
			EWAY_Way     *ew,
			EWAY_Conddef *ecd)
{
	EWAY_Cond ans;

	ans.type   = ecd->type;
	ans.negate = ecd->negate;
	ans.arg1   = ecd->arg1;
	ans.arg2   = ecd->arg2;

	//
	// We might have to do some extra processing.
	//

	switch(ans.type)
	{
		case EWAY_COND_TRIPWIRE:

			{
				SLONG x1;
				SLONG y1;
				SLONG z1;

				SLONG x2;
				SLONG y2;
				SLONG z2;

				SLONG vector[3];

				PrimInfo *pi;

				//
				// Look for a nearby tripwire.
				//

				if (OB_find_type(
						ew->x,
						ew->y,
						ew->z,
						0x200,
						FIND_OB_TRIPWIRE,
					   &ob_x,
					   &ob_y,
					   &ob_z,
					   &ob_yaw,
					   &ob_prim,
					   &ob_index))
				{
					pi = get_prim_info(ob_prim);

					//
					// The direction the prim is facing.
					// 

					FMATRIX_vector(
						vector,
						ob_yaw,
						0);

					//
					// The start coordinate of the tripwire.
					//

					x1 = ob_x - MUL64(pi->radius, vector[0]);
					z1 = ob_z - MUL64(pi->radius, vector[2]);
					y1 = ob_y + (pi->miny + pi->maxy >> 1);

					//
					// The end coordinate of the tripwire.
					// 

					x2 = x1 - MUL64(vector[0], 0x500);
					y2 = y1;
					z2 = z1 - MUL64(vector[2], 0x500);

					//
					// If the tripwire hits something...
					//

					if (!there_is_a_los(
							x1, y1, z1,
							x2, y2, z2,
							LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
					{
						x2 = los_failure_x;
						y2 = los_failure_y;
						z2 = los_failure_z;
					}

					//
					// Create the tripwire.
					// 

					ans.arg1 = TRIP_create(
										y1,
										x1, z1,
										x2, z2);
				}
				else
				{
					ans.arg1 = NULL;
				}
			}

			break;

		case EWAY_COND_SWITCH:

			//
			// Find a nearby switch anim-prim.
			//

			ans.arg1 = find_anim_prim(
							ew->x,
							ew->y,
							ew->z,
							0x200,
							(1 << ANIM_PRIM_TYPE_SWITCH));

			break;

		case EWAY_COND_BOOL_AND:
		case EWAY_COND_BOOL_OR:

			ASSERT(EWAY_cond_upto + 2 <= EWAY_MAX_CONDS);

			ans.arg1 = EWAY_cond_upto++;
			ans.arg2 = EWAY_cond_upto++;

			EWAY_cond[ans.arg1] = EWAY_create_cond(ew, ecd->bool_arg1);
			EWAY_cond[ans.arg2] = EWAY_create_cond(ew, ecd->bool_arg2);

			break;

		case EWAY_COND_PRIM_DAMAGED:

			//
			// Look for the nearest damagable OB
			//

			{
				SWORD best_dist  = 0x7800;
				UWORD best_index = NULL;
				UWORD best_x;
				UWORD best_z;
				SWORD dx;
				SWORD dy;
				SWORD dz;
				SWORD dist;

				OB_Info *oi = OB_find(
								ew->x >> PAP_SHIFT_LO,
								ew->z >> PAP_SHIFT_LO);

				while(oi->prim)
				{
					dx = ew->x - oi->x;
					dy = ew->y - oi->y;
					dz = ew->z - oi->z;

					dist = abs(dx) + abs(dy) + abs(dz);

					if (best_dist > dist)
					{
						best_dist  = dist;
						best_index = oi->index;
						best_x     = oi->x;
						best_z     = oi->z;
					}

					oi += 1;
				}

				if (best_index)
				{
					//
					// Move to the position of the object... for some reason!
					//

					ew->x    = best_x;
					ew->z    = best_z;
					ans.arg1 = best_index;
				}
			}

			break;

		case EWAY_COND_RADIUS_USED:
			
			//
			// Is there a nearby switch or valve OB?
			//

			if (OB_find_type(
					ew->x,
					ew->y,
					ew->z,
					ecd->arg1 * 4,
					FIND_OB_SWITCH_OR_VALVE,
				   &ob_x,
				   &ob_y,
				   &ob_z,
				   &ob_yaw,
				   &ob_prim,
				   &ob_index))
			{
				//
				// Convert to a EWAY_COND_PRIM_ACTIVATED...
				// 

				ans.type = EWAY_COND_PRIM_ACTIVATED;
				ans.arg1 = ob_index;
			}

			break;

		case EWAY_COND_PRIM_ACTIVATED:

			ans.arg1 = NULL;

			if (OB_find_type(
					ew->x,
					ew->y,
					ew->z,
					ecd->arg1,
					FIND_OB_SWITCH_OR_VALVE,
				   &ob_x,
				   &ob_y,
				   &ob_z,
				   &ob_yaw,
				   &ob_prim,
				   &ob_index))
			{
				ans.arg1 = ob_index;
			}

			break;
	}

	return ans;
}

#endif
#endif

#ifndef PSX
#ifndef TARGET_DC
void EWAY_create(
		SLONG identifier,	// A unique number that names the waypoint- same as arg for DEPENDENT conditions.
		SLONG colour,
		SLONG group,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z,
		SLONG yaw,
		EWAY_Conddef *ecd,
		EWAY_Do      *ed,
		EWAY_Stay    *es,
		EWAY_Edef    *ee,
		SLONG         unreferenced,
		SLONG         kludge_index,
		UWORD	magic_index)
{
	EWAY_Way *ew;

ANNOYINGSCRIBBLECHECK;

	ASSERT(WITHIN(EWAY_way_upto, 1, EWAY_MAX_WAYS - 1));

	ew = &EWAY_way[EWAY_way_upto];

	//
	// Set values.
	//

	ew->id     = identifier;
	ew->colour = colour;
	ew->group  = group;
	ew->flag   = 0;
	ew->timer  = 0;
	ew->x      = world_x;
	ew->y      = world_y;
	ew->z      = world_z;
	ew->yaw    = yaw >> 3;
	ew->index  = kludge_index;

	ew->ec     =  EWAY_create_cond(ew, ecd);
	ew->es     = *es;
	ew->ed     = *ed;

	if (ew->ed.type == EWAY_DO_MESSAGE      ||
		ew->ed.type == EWAY_DO_CONVERSATION ||
		ew->ed.type == EWAY_DO_AMBIENT_CONV)
	{
		//
		// encode the magic number somewhere unused for messages
		//

		ew->yaw=magic_index>>8;
		ew->index=magic_index&0xff;


	}
	if (ew->ed.type == EWAY_DO_CREATE_PLAYER  ||
		ew->ed.type == EWAY_DO_CREATE_ANIMAL  ||
		ew->ed.type == EWAY_DO_CREATE_ENEMY   ||
//		ew->ed.type == EWAY_DO_CREATE_VEHICLE ||	// For vehicles arg2 means something so it can't be cleared yet.
		ew->ed.type == EWAY_DO_CAMERA_TARGET)
	{
		//
		// For these waypoint types, arg is the index of the last Thing
		// to be created by it.  NULL => it hasn't created anybody yet.
		//
		// (For a camera target with subtype THING has the waypoint that creates
		//  the thing in arg)
		//

		ew->ed.arg1 = NULL;
		ew->ed.arg2 = NULL;
	}

	//
	// When we create an enemy or adjust an enemies AI,
	// we need a load of extra info stored in the edef_upto[]
	// array.
	//

	if (ew->ed.type == EWAY_DO_CREATE_ENEMY ||
		ew->ed.type == EWAY_DO_CHANGE_ENEMY)
	{
		ASSERT(WITHIN(EWAY_edef_upto, 0, EWAY_MAX_EDEFS - 1));

		//
		// We have to get remember our person definition structure.
		//

		EWAY_edef[EWAY_edef_upto] = *ee;
		ew->index                 =  EWAY_edef_upto;
		EWAY_edef_upto           +=  1;
	}

	//
	// If this waypoint needs to be attached to anything, then
	// find that thing!
	//

	if (ew->ed.type == EWAY_DO_CONTROL_DOOR)
	{
		//
		// This is done using DOOR.CPP code nowadays.
		//

		/*

		//
		// Find a nearby door anim-prim.
		//

		ew->ed.arg = find_anim_prim(
						world_x,
						world_y,
						world_z,
						512,
						(1 << ANIM_PRIM_TYPE_DOOR));

		if (ew->ed.arg == NULL)
		{
#ifndef	PSX
			sprintf(EWAY_message, "Waypoint %d could not find a door", ew->id);

			CONSOLE_text(EWAY_message, 8000);
#else
			sprintf("Waypoint %d could not find a door\n",ew->id);
#endif
		}

		*/
	}

	if (ew->ed.type == EWAY_DO_ACTIVATE_PRIM)
	{
		//
		// Find a nearby anim-prim.
		//

		ew->ed.arg1 = find_anim_prim(
						world_x,
						world_y,
						world_z,
						512,
						0xffff);

		if (ew->ed.arg1 == NULL)
		{
#ifndef PSX
			sprintf(EWAY_message, "Waypoint %d could not find an anim-prim to activate", ew->id);

			CONSOLE_text(EWAY_message, 8000);
#else
			printf("Waypoint %d could not find an anim-prim to activate", ew->id);
#endif
		}
	}

	if (ew->ed.type == EWAY_DO_ELECTRIFY_FENCE)
	{
		//
		// Look for a nearby electric fence.
		//

		ew->ed.arg1 = find_electric_fence_dbuilding(
						world_x,
						world_y,
						world_z,
						256);
	}

	if (ew->ed.type == EWAY_DO_CREATE_BOMB)
	{
		//
		// Create a bomb special.
		//

		alloc_special(
			SPECIAL_BOMB,
			SPECIAL_SUBSTATE_ACTIVATED,
			ew->x,
			ew->y + 0x60,
			ew->z,
			EWAY_way_upto);
	}

	if (ew->ec.type == EWAY_COND_TRUE)
	{
		if (unreferenced)
		{
			//
			// Certain waypoints need not take up a EWAY_way. We set set them active
			// now and reuse the EWAY_way.
			//

			if (ew->ed.type    == EWAY_DO_CREATE_VEHICLE &&
				ew->ed.subtype != EWAY_SUBTYPE_VEHICLE_HELICOPTER)
			{
				void EWAY_set_active(EWAY_Way *ew);

				EWAY_set_active(ew);

				memset(ew, 0, sizeof(EWAY_Way));

ANNOYINGSCRIBBLECHECK;

				return;
			}
		}
	}

	EWAY_way_upto += 1;

ANNOYINGSCRIBBLECHECK;

	return;
}
#endif
#endif

#ifndef PSX
#ifndef TARGET_DC
SLONG EWAY_set_message(
		UBYTE  number,
		CBYTE *message)
{
	SLONG len = strlen(message) + 1;

	if (!WITHIN(number, 0, EWAY_MAX_MESSES - 1))
	{
		//
		// Out of range message.
		//
//		ASSERT(0);

		return(0);
	}

#ifndef NDEBUG

	//
	// Make sure the string has a terminating byte!
	//

	if (message[len - 1] != '\000')
	{	
		message[len - 1] = '\000';
#ifndef	FINAL
#ifndef	PSX
		PANEL_new_text(NULL, 5000, "Message was not terminated: \"%s\"", message);
#endif
#endif
	}

	#endif

	if (EWAY_mess_buffer_upto + len > EWAY_MESS_BUFFER_SIZE)
	{
		//
		// Not enough memory in the buffer.
		//

		return(0);
	}

	EWAY_mess[number] = &EWAY_mess_buffer[EWAY_mess_buffer_upto];
	if(number>=EWAY_mess_upto)
		EWAY_mess_upto=number+1;

	strcpy(&EWAY_mess_buffer[EWAY_mess_buffer_upto], message);

	EWAY_mess_buffer_upto += len;
	return(1);
}
#endif
#endif



#ifndef PSX
#ifndef TARGET_DC

//
// Returns the index of the waypoint with the given identifier.
//

SLONG EWAY_find_id(SLONG id)
{
	SLONG i;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];

		if (ew->id == id)
		{
			return i;
		}
	}

	return NULL;
}
#endif
#endif

#ifndef PSX
#ifndef TARGET_DC

void EWAY_fix_cond(EWAY_Cond *ec)
{
	SLONG id;
	SLONG id1;
	SLONG id2;

	if (ec->type == EWAY_COND_DEPENDENT           ||
		ec->type == EWAY_COND_PERSON_DEAD         ||
		ec->type == EWAY_COND_HALF_DEAD           ||
		ec->type == EWAY_COND_PERSON_USED         ||
		ec->type == EWAY_COND_ITEM_HELD           ||
		ec->type == EWAY_COND_CONVERSE_END        ||
		ec->type == EWAY_COND_PERSON_ARRESTED     ||
		ec->type == EWAY_COND_COUNTDOWN           ||
		ec->type == EWAY_COND_COUNTDOWN_SEE       ||
		ec->type == EWAY_COND_PERSON_NEAR         ||
		ec->type == EWAY_COND_IS_MURDERER	      ||
		ec->type == EWAY_COND_KILLED_NOT_ARRESTED ||
		ec->type == EWAY_COND_THING_RADIUS_DIR    ||
		ec->type == EWAY_COND_SPECIFIC_ITEM_HELD  ||
		ec->type == EWAY_COND_MOVE_RADIUS_DIR     ||
		ec->type == EWAY_COND_RANDOM)
	{
		ec->arg1 = EWAY_find_id(ec->arg1);

		if (ec->type == EWAY_COND_ITEM_HELD)
		{
			ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

			//
			// Work out which item we should check the person for having.
			//

			if (EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_ITEM)
			{
				ec->arg1 = EWAY_way[ec->arg1].ed.subtype;
			}
			else
			if (EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_ENEMY)
			{
				EWAY_Edef *ee;
				EWAY_Way  *ew = &EWAY_way[ec->arg1];

				ASSERT(WITHIN(ew->index, 1, EWAY_edef_upto - 1));

				ee = &EWAY_edef[ew->index];
				
				ec->arg1 = ee->drop;
			}
		}
	}
	else
	if (ec->type == EWAY_COND_A_SEE_B ||
		ec->type == EWAY_COND_PERSON_IN_VEHICLE)
	{	
		//
		// Two ids.
		//

		ec->arg1 = EWAY_find_id(ec->arg1);
		ec->arg2 = EWAY_find_id(ec->arg2);
	}
}
#endif
#endif

#ifndef PSX
#ifndef TARGET_DC
void EWAY_fix_do(EWAY_Do *ed, EWAY_Way *ew)
{
	SLONG id;
	SLONG id1;
	SLONG id2;

	if (ed->type == EWAY_DO_CHANGE_ENEMY     ||
		ed->type == EWAY_DO_KILL_WAYPOINT    ||
		ed->type == EWAY_DO_LOCK_VEHICLE     ||
		ed->type == EWAY_DO_CHANGE_ENEMY_FLG ||
		ed->type == EWAY_DO_STALL_CAR        ||
		ed->type == EWAY_DO_EXTEND_COUNTDOWN ||
		ed->type == EWAY_DO_TRANSFER_PLAYER  ||
		ed->type == EWAY_DO_MAKE_PERSON_PEE  ||
		ed->type == EWAY_DO_MOVE_THING)
	{
		ed->arg1 = EWAY_find_id(ed->arg1);
	}
	else
	if (ed->type == EWAY_DO_CONVERSATION ||
		ed->type == EWAY_DO_AMBIENT_CONV)
	{

		ed->arg1 = EWAY_find_id(ed->arg1);
		ed->arg2 = EWAY_find_id(ed->arg2);
	}
	else
	if (ed->type == EWAY_DO_NAV_BEACON)
	{
		if (ed->arg2)
		{
			ed->arg2 = EWAY_find_id(ed->arg2);

			if (EWAY_way[ed->arg2].ed.type == EWAY_DO_CREATE_PLAYER  ||
				EWAY_way[ed->arg2].ed.type == EWAY_DO_CREATE_ENEMY   ||
				EWAY_way[ed->arg2].ed.type == EWAY_DO_CREATE_ANIMAL  ||
				EWAY_way[ed->arg2].ed.type == EWAY_DO_CREATE_VEHICLE ||
				EWAY_way[ed->arg2].ed.type == EWAY_DO_CREATE_ITEM)
			{
				//
				// Okay to track these waypoints...
				//
			}
			else
			{
				//
				// Can't track these... its a location.
				//

				ed->arg2 = NULL;
			}
		}
	}
	else
	if (ed->type == EWAY_DO_CREATE_VEHICLE)
	{
		if (ed->arg2)
		{
			ed->arg2 = EWAY_find_id(ed->arg2);
		}
	}
	else
	if (ed->type == EWAY_DO_MESSAGE)
	{
		if (ed->arg2                                &&
			ed->arg2 != EWAY_MESSAGE_WHO_STREETNAME &&
			ed->arg2 != EWAY_MESSAGE_WHO_TUTORIAL)
		{
			ed->arg2 = EWAY_find_id(ed->arg2);
		}
	}
}

void EWAY_fix_edef(EWAY_Edef *ee)
{
	if (ee->pcom_ai == PCOM_AI_BODYGUARD  ||
		ee->pcom_ai == PCOM_AI_ASSASIN    ||
		ee->pcom_ai == PCOM_AI_SHOOT_DEAD ||
		ee->pcom_ai == PCOM_AI_KILL_COLOUR)
	{
		//
		// ed->number is the ID of the waypoint that creates the person
		// we want to guard.
		//

		ee->ai_other = EWAY_find_id(ee->ai_other);
	}

	if (ee->pcom_move == PCOM_MOVE_FOLLOW)
	{
		ee->follow = EWAY_find_id(ee->follow);
	}
}
#endif
#endif

#ifndef PSX
#ifndef TARGET_DC
SLONG EWAY_load_message_file(CBYTE *fname, UWORD *index, UWORD *number)
{
	FILE *handle = MF_Fopen(fname, "rb");

	if (handle)
	{
		CBYTE  line   [512];
		CBYTE  message[512];
		CBYTE *ch;
		CBYTE *start;

		SLONG match;
		SLONG upto;

	   *index  = EWAY_mess_upto;
	   *number = 0;

		while(fgets(line, 512, handle))
		{
			match = sscanf(line, "%d. %s", &upto, &message);

			if (match == 2)
			{
				//
				// Find the start of the message.
				//

				for (ch = line; *ch != '.'; ch++);
				while(isspace(*++ch));
				start = ch;

				//
				// Get rid of white space at the end of the message.
				// 

				while(*ch) ch++;
				for (; isspace(*--ch); *ch = '\000');

				if(EWAY_set_message(*index + *number,start))
				{
					// only increase the count if succesfully added
					*number += 1;
				}
			}
		}

		MF_Fclose(handle);

		return TRUE;
	}

	return FALSE;
}



void EWAY_load_fake_wander_text(CBYTE *fname)
{
#ifndef PSX
	CBYTE name_buffer[_MAX_PATH];
	CBYTE	str[100];

	EWAY_fake_wander_text_normal_index	 = NULL;
	EWAY_fake_wander_text_normal_number	 = 0;
	EWAY_fake_wander_text_guilty_index	 = NULL;
	EWAY_fake_wander_text_guilty_number	 = 0;
	EWAY_fake_wander_text_annoyed_index	 = NULL;
	EWAY_fake_wander_text_annoyed_number = 0;

	XLAT_str(X_THIS_LANGUAGE_IS,name_buffer);

	if (!stricmp(name_buffer,"Italian"))
	{
		fname = "text\\citsez_ita.txt";
	}
	if (!stricmp(name_buffer,"French"))
	{
		fname = "text\\citsez_french.txt";
	}
	if (!stricmp(name_buffer,"Deutsch")) // everyone else just put the english word for their lang in
	{
		fname = "text\\citsez_german.txt";
	}
	if (!stricmp(name_buffer,"Spanish"))
	{
		fname = "text\\citsez_spa.txt"; // must change if/when we get citsez for spanish...
	}

	if(save_psx)
	{
		sprintf(str,"%s",ENV_get_value_string("level_dir","Secret"));

		if (!stricmp(str,"glevels"))
		{
			fname = "text\\citsez_german.txt";
		}
		if (!stricmp(str,"flevels"))
		{
			fname = "text\\citsez_french.txt";
		}
		if (!stricmp(str,"ilevels"))
		{
			fname = "text\\citsez_ita.txt";
		}
		if (!stricmp(str,"slevels"))
		{
			fname = "text\\citsez_spa.txt";
		}


	}

	if (fname == NULL)
	{
		fname = "text\\citsez.txt";
	}

	if (!EWAY_load_message_file(
			fname,
		   &EWAY_fake_wander_text_normal_index,
		   &EWAY_fake_wander_text_normal_number))
	{
		if(save_psx)
		{
			if (!stricmp(str,"glevels"))
			{
				ASSERT(0);
			}
			if (!stricmp(str,"flevels"))
			{
				ASSERT(0);
			}
			if (!stricmp(str,"ilevels"))
			{
				ASSERT(0);
			}
			if (!stricmp(str,"slevels"))
			{
				ASSERT(0);
			}
		}

		EWAY_load_message_file(
			"text\\citsez.txt",
		   &EWAY_fake_wander_text_normal_index,
		   &EWAY_fake_wander_text_normal_number);
	}

	EWAY_load_message_file(
		"text\\guilty.txt",
	   &EWAY_fake_wander_text_guilty_index,
	   &EWAY_fake_wander_text_guilty_number);

	if(save_psx)
	{

		if (!stricmp(str,"levels"))
		{
		}
		else
		{
			EWAY_load_message_file(
				"text\\annoyed.txt",
			   &EWAY_fake_wander_text_annoyed_index,
			   &EWAY_fake_wander_text_annoyed_number);
		}

	}
	else
	{
		EWAY_load_message_file(
			"text\\annoyed.txt",
		   &EWAY_fake_wander_text_annoyed_index,
		   &EWAY_fake_wander_text_annoyed_number);
	}

//	ASSERT(index+number<EWAY_mess_upto);
	ASSERT(EWAY_fake_wander_text_annoyed_index+EWAY_fake_wander_text_annoyed_number<=EWAY_mess_upto);
	ASSERT(EWAY_fake_wander_text_guilty_index+EWAY_fake_wander_text_guilty_number<=EWAY_mess_upto);
	ASSERT(EWAY_fake_wander_text_normal_index+EWAY_fake_wander_text_normal_number<=EWAY_mess_upto);

	// If this trips, you don't have c:\fallen\text\citsez.txt (or the foreign equivalent).
	ASSERT ( EWAY_fake_wander_text_normal_number > 0 );
#endif
	return;
}
#endif
#endif


CBYTE *EWAY_get_fake_wander_message(SLONG type)
{
#ifdef TARGET_DC
	// We only have normal text on DC (not sure why not).
	UWORD index  = EWAY_fake_wander_text_normal_index;
	UWORD number = EWAY_fake_wander_text_normal_number;

#else //#ifdef TARGET_DC


	UWORD index;
	UWORD number;
#ifndef PSX
	static SBYTE this_is_english = -1;

	if (this_is_english<0)
	{
		this_is_english = !stricmp(XLAT_str(X_THIS_LANGUAGE_IS),"English");
	}

	if (!this_is_english) type=EWAY_FAKE_MESSAGE_NORMAL;
#else
	extern UBYTE IsEnglish;

	if (!IsEnglish)
		type=EWAY_FAKE_MESSAGE_NORMAL;
#endif
	switch(type)
	{
		case EWAY_FAKE_MESSAGE_NORMAL:
			index  = EWAY_fake_wander_text_normal_index;
			number = EWAY_fake_wander_text_normal_number;
			break;

		case EWAY_FAKE_MESSAGE_ANNOYED:
			index  = EWAY_fake_wander_text_annoyed_index;
			number = EWAY_fake_wander_text_annoyed_number;
			break;

		case EWAY_FAKE_MESSAGE_GUILTY:
			index  = EWAY_fake_wander_text_guilty_index;
			number = EWAY_fake_wander_text_guilty_number;
			break;

		default:
			ASSERT(0);
			break;
	}
#endif //#else //#ifdef TARGET_DC

	ASSERT(index+number<=EWAY_mess_upto);

	if (number == 0)
	{
		// return "I have nothing to say to you."; // not only does this look rubbish, it's not been translated.
		ASSERT ( FALSE );
		return 0; // this gets passed to PANEL_new_text() which early-out's on nulls, so we're safe.
	}
	else
	{
		SLONG which = Random() % number;

		ASSERT(WITHIN(which + index, 0, EWAY_mess_upto - 1));

		if(which + index>=EWAY_mess_upto)
		{
			which=0;
			index=0;
		}

		return EWAY_mess[which + index];
	}
}


#ifndef PSX
#ifndef TARGET_DC
void EWAY_created_last_waypoint()
{
	SLONG i;
	SLONG id;
	SLONG points;
	SLONG total_points;
	SLONG num_guilty_people;

	EWAY_Way  *ew;
	EWAY_Cond *ec;
	EWAY_Do   *ed;
	EWAY_Edef *ee;

	//
	// Go through all the data and remap the dependent
	// identifiers to be waypoint indices.  Count the total
	// of objective points and the number of thugs/guilty people.
	//

	total_points      = 0;
	num_guilty_people = 0;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];

		ec = &ew->ec;
		ed = &ew->ed;

		EWAY_fix_cond(ec);
		EWAY_fix_do  (ed,ew);

		if (ed->type    == EWAY_DO_CAMERA_TARGET &&
			ed->subtype == EWAY_SUBTYPE_CAMERA_TARGET_THING)
		{	
			//
			// Look for the nearest CREATE_* waypoint and attach to it.
			//

			SLONG j;
			SLONG dx;
			SLONG dy;
			SLONG dz;
			SLONG dist;
			SLONG best_dist = 0x300;
			SLONG best_way  = NULL;

			EWAY_Way *ew_near;

			for (j = 1; j < EWAY_way_upto; j++)
			{
				ew_near = &EWAY_way[j];

				switch(ew_near->ed.type)
				{
					case EWAY_DO_CREATE_PLAYER:
					case EWAY_DO_CREATE_ANIMAL:
					case EWAY_DO_CREATE_ENEMY:
					case EWAY_DO_CREATE_ITEM:
					case EWAY_DO_CREATE_VEHICLE:
					case EWAY_DO_CREATE_PLATFORM:

						dx = abs(ew_near->x - ew->x);
						dy = abs(ew_near->y - ew->y);
						dz = abs(ew_near->z - ew->z);

						dist = QDIST3(dx,dy,dz);

						if (dist < best_dist)
						{
							best_dist = dist;
							best_way  = j;
						}

						break;

					default:
						break;
				}
			}
#ifndef PSX
			if (best_way == NULL)
			{
				sprintf(EWAY_message, "Camera target %d couldn't attach", i);

				CONSOLE_text(EWAY_message, 8000);
			}
#endif
			ew->ed.arg1 = best_way;
		}
		else
		if (ew->ed.type == EWAY_DO_OBJECTIVE)
		{
			points = ew->ed.arg2;

			switch (ew->ed.subtype) 
			{
				case EWAY_SUBTYPE_OBJECTIVE_MAIN:

					//
					// The main objective doesn't contribute to the crime rate.
					//

					break;

				case EWAY_SUBTYPE_OBJECTIVE_SUB:
					total_points += points;
					break;

				case EWAY_SUBTYPE_OBJECTIVE_ADDITIONAL:
					total_points += points;
					break;
			}
		}
		else
		if (ew->ed.type == EWAY_DO_CREATE_ENEMY)
		{
			EWAY_Edef *ee;

			ASSERT(WITHIN(ew->index, 1, EWAY_edef_upto - 1));

			ee = &EWAY_edef[ew->index];

			if (ee->pcom_ai == PCOM_AI_GANG /* || The person is guilty... */)
			{
				num_guilty_people += 1;
			}
		}
	}

	for (i = 1; i < EWAY_cond_upto; i++)
	{
		ec = &EWAY_cond[i];

		EWAY_fix_cond(ec);
	}

	for (i = 1; i < EWAY_edef_upto; i++)
	{
		ee = &EWAY_edef[i];

		EWAY_fix_edef(ee);
	}

	//
	// Reset the time.
	//

	EWAY_time_accurate = 0;
	EWAY_time          = 0;
	EWAY_count_up      = 0;

	//
	// The penalties on the count-up timer.
	//

	EWAY_count_up_add_penalties = 0;
	EWAY_count_up_num_penalties = 0;
	EWAY_count_up_penalty_timer = 0;

	//
	// Reset the tutorial string.
	//
	
	EWAY_tutorial_string = NULL;

	//
	// Work out the crime rate score multiplier.
	//

	if (total_points == 0)
	{
		CRIME_RATE_SCORE_MUL = 0;
	}
	else
	{
		//
		// 4 percent per guilty person... but no more than 50% of the
		// original CRIME_RATE is allocated to guilty people.
		//
		
		SLONG for_guilty = num_guilty_people * 4;	
		
		SATURATE(for_guilty, 0, CRIME_RATE >> 1);

		CRIME_RATE_SCORE_MUL = ((CRIME_RATE - for_guilty) << 8) / total_points;
	}

	//
	// Look for level lost waypoints dependent on a message and make that
	// message flagged as 'EWAY_FLAG_LOST_MESS'.
	//

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];

		if (ew->ed.type == EWAY_DO_MISSION_FAIL)
		{
			if (ew->ec.type == EWAY_COND_DEPENDENT)
			{
				if (WITHIN(ew->ec.arg1, 1, EWAY_way_upto - 1))
				{
					EWAY_way[ew->ec.arg1].flag |= EWAY_FLAG_WHY_LOST;
				}
			}
		}

		if (ew->ed.type == EWAY_DO_MESSAGE)
		{
			if (ew->ec.type == EWAY_COND_DEPENDENT)
			{
				if (WITHIN(ew->ec.arg1, 1, EWAY_way_upto - 1))
				{
					if (EWAY_way[ew->ec.arg1].ed.type == EWAY_DO_MISSION_FAIL)
					{
						ew->flag |= EWAY_FLAG_WHY_LOST;
					}
				}
			}
		}
	}

	#if 0

	// THIS IS CAUSING BUGS! IM GOING TO MAKE ALL MESSAGES AND CONVERSATIONS
	// NEVER TRIGGER UNTIL AFTER THE FIRST GAMETURN NOT JUST IF THEY ARE SET
	// TO EWAY COND TRUE

	//
	// Make all message and conversation waypoint that have a
	// condition TRUE have a condtion AFTER_FIRST_GAMETURN instead. So
	// they have a chance of triggering on the PSX.
	//

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];

		if (ew->ec.type == EWAY_COND_TRUE)
		{
			if (ew->ed.type == EWAY_DO_CONVERSATION ||
				ew->ed.type == EWAY_DO_AMBIENT_CONV ||
				ew->ed.type == EWAY_DO_MESSAGE)
			{
				ew->ec.type = EWAY_COND_AFTER_FIRST_GAMETURN;
			}
		}
	}

	#endif
}
#endif
#endif

/*
SLONG	global_write=0;
SLONG	global_write1=0;
SLONG	global_write2=0;
SLONG	global_write3=0;
SLONG	global_write4=0;
SLONG	global_write5=0;
SLONG	global_write6=0;
SLONG	global_write7=0;
SLONG	global_write8=0;
SLONG	global_write9=0;
SLONG	global_write10=0;
SLONG	global_write11=0;
SLONG	global_write12=0;
*/



SLONG EWAY_evaluate_condition(EWAY_Way *ew, EWAY_Cond *ec, SLONG EWAY_sub_condition_of_a_boolean = FALSE)
{
	SLONG ans = FALSE;

ANNOYINGSCRIBBLECHECK;

	switch(ec->type)
	{
		case EWAY_COND_FALSE:
			ans = FALSE;
			break;

		case EWAY_COND_TRUE: 
			ans = TRUE;
			break;

		case EWAY_COND_PROXIMITY:

			{
				Thing *darci = NET_PERSON(0);

				if (darci)
				{
					{
						SLONG dx = abs(ew->x - (darci->WorldPos.X >> 8));
						SLONG dy = abs(ew->y - (darci->WorldPos.Y >> 8));
						SLONG dz = abs(ew->z - (darci->WorldPos.Z >> 8));

						SLONG dist = QDIST3(dx,dy,dz);

						ans = (dist < ec->arg1);
					}
				}
				else
				{
					ans = FALSE;
				}
			}

			break;

		case EWAY_COND_TRIPWIRE:
			ans = TRIP_activated(ec->arg1);
			break;

		case EWAY_COND_PRESSURE:
			ans = FALSE;
			break;

		case EWAY_COND_CAMERA:
			ans = FALSE;
			break;

		case EWAY_COND_SWITCH:
			ans = (TO_THING(ec->arg1)->Flags & FLAGS_SWITCHED_ON);
			break;

		case EWAY_COND_TIME:
			ans = (EWAY_time >= ec->arg1);
			break;

		case EWAY_COND_DEPENDENT:

			if (ec->arg1 == NULL)
			{
#ifndef PSX
				sprintf(EWAY_message, "Waypoint %d has a NULL dependency", ew->id);

				CONSOLE_text(EWAY_message, 8000);
#endif
				ans = FALSE;

				//
				// Don't print the message again.
				//

				ec->type = EWAY_COND_FALSE;
			}
			else
			{
				ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

				ans = (EWAY_way[ec->arg1].flag & EWAY_FLAG_ACTIVE);
			}

			break;

		case EWAY_COND_BOOL_AND:
			
			{
				EWAY_Cond *ec1 = &EWAY_cond[ec->arg1];
				EWAY_Cond *ec2 = &EWAY_cond[ec->arg2];

				ans = EWAY_evaluate_condition(ew, ec1, TRUE) && EWAY_evaluate_condition(ew, ec2, TRUE);
			}

			break;

		case EWAY_COND_BOOL_OR:

			{
				EWAY_Cond *ec1 = &EWAY_cond[ec->arg1];
				EWAY_Cond *ec2 = &EWAY_cond[ec->arg2];

				ans = EWAY_evaluate_condition(ew, ec1, TRUE) || EWAY_evaluate_condition(ew, ec2, TRUE);
			}

			break;

		case EWAY_COND_COUNTDOWN:
		case EWAY_COND_COUNTDOWN_SEE:

			{
				SLONG depend = ec->arg1;
				SLONG timer  = ec->arg2;

				ASSERT(depend == 0 || WITHIN(depend, 1, EWAY_way_upto - 1));

				if (!depend || (EWAY_way[depend].flag & EWAY_FLAG_ACTIVE))
				{
					if (EWAY_conv_active)
					{
						//
						// Timers stop during conversations.
						//
					}
					else
					{
						//
						// Tick down the timer.
						//
	#ifndef PSX
						if (ec->arg2 <= EWAY_tick)
						{
							ec->arg2 = 0;
							ans      = TRUE;
						}
						else
						{
							ec->arg2 -= EWAY_tick;
							ans       = FALSE;
						}
	#else
						if (ec->arg2 <= EWAY_tick*2)
						{
							ec->arg2 = 0;
							ans		= TRUE;
						}
						else
						{
							ec->arg2 -= EWAY_tick*2;
							ans		= FALSE;
						}
	#endif			
					}

					if (ec->type == EWAY_COND_COUNTDOWN_SEE)
					{
						if (!(GAME_STATE&(GS_LEVEL_LOST|GS_LEVEL_WON))) {
							if (ec->arg2<3000) // 30 seconds... theoretically
//								MUSIC_play(S_TUNE_TIMER1,MUSIC_FLAG_OVERRIDE);
								music_mode_override = MUSIC_MODE_TIMER;
							else // maybe we were below 30 seconds but time got added
//								if (MUSIC_wave()==S_TUNE_TIMER1) MUSIC_stop(1);
							{
								if (music_mode_override) 
								{
									// this is a little awkward but override must be cleared before the call or it fails
									music_mode_override = 0;
									MUSIC_mode(0);
								} else music_mode_override = 0;
							}
						}
						PANEL_draw_timer(ec->arg2, 320, 50);
					}
				}
				else
				{
					//
					// Not counting down.
					//

					ans = FALSE;
				}
			}

			break;

		case EWAY_COND_PERSON_DEAD:

			ans = FALSE; // By default.

			{
				SLONG waypoint;
				SLONG index;

				Thing    *p_thing;
				EWAY_Way *ew_dead;

				waypoint = ec->arg1;

				if (waypoint == 0)
				{
#ifndef PSX
					CBYTE mess[128];

					sprintf(mess, "Waypoint %d: cond person dead has NULL dependency", ew->id);

					CONSOLE_text(mess, 8000);
#endif
					ew->flag |= EWAY_FLAG_DEAD;
				}
				else
				{
					ew_dead = &EWAY_way[waypoint];

					switch(ew_dead->ed.type)
					{
						case EWAY_DO_CREATE_PLAYER:
						case EWAY_DO_CREATE_VEHICLE:
						case EWAY_DO_CREATE_ANIMAL:
						case EWAY_DO_CREATE_ENEMY:

							index = ew_dead->ed.arg1;

							if (index)
							{
								p_thing = TO_THING(index);

								ans = (p_thing->State == STATE_DEAD);

/*
								if(ans)
								{
									printf(" cond deadwp %d\n",ew-EWAY_way);
								}
*/

								extern BOOL PersonIsMIB(Thing* p_person);

								if (p_thing->Class == CLASS_PERSON && PersonIsMIB(p_thing))
								{
									if (p_thing->Flags & FLAGS_ON_MAPWHO)
									{
										ans = FALSE;
									}
								}
							}

							break;

						case EWAY_DO_CREATE_ITEM:
							ans = (ew_dead->flag & EWAY_FLAG_GOTITEM);
							break;

						case EWAY_DO_CREATE_BARREL:
							
							{
								ans = FALSE;

								if (ew_dead->flag & EWAY_FLAG_ACTIVE)
								{	
									if (ew_dead->ed.arg1 == NULL)
									{
										//
										// The barrel has been created and destroyed.
										//

										ans = TRUE;
									}
								}
							}

							break;

						default:
							{
#ifndef PSX
								CBYTE mess[128];

								sprintf(mess, "Waypoint %d: cond person dead dependent on odd waypoint", ew->id);

								CONSOLE_text(mess, 8000);
#endif
								ew->flag |= EWAY_FLAG_DEAD;
							}

							break;
					}
				}
			}

			break;

		case EWAY_COND_PERSON_NEAR:
			
			{
				Thing *p_person;

				SLONG dx;
				SLONG dy;
				SLONG dz;
				SLONG dist;
				SLONG waypoint;
				SLONG index;
				SLONG radius;

				waypoint = ec->arg1;
				radius   = ec->arg2 * 64;	// Radius in quarter map-squares.

				if (waypoint == NULL)
				{
					//
					// Just check for anybody being near.
					//

					ans = THING_find_nearest(
							ew->x,
							ew->y,
							ew->z,
							radius,
							THING_FIND_PEOPLE);
				}
				else
				{
					index = EWAY_get_person(waypoint);

					if (index)
					{
						p_person = TO_THING(index);

						dx = abs((p_person->WorldPos.X >> 8) - ew->x);
						dy = abs((p_person->WorldPos.Y >> 8) - ew->y);
						dz = abs((p_person->WorldPos.Z >> 8) - ew->z);

						dist = QDIST3(dx,dy,dz);

						ans = dist < radius;
					}
					else
					{
						ans = FALSE;	// Person hasn't been created yet.
					}
				}
			}

			break;

		case EWAY_COND_CAMERA_AT:

			//
			// This condition is perversely set in EWAY_process_camera().
			//

			ans = FALSE;

			break;

		case EWAY_COND_PLAYER_CUBE:
			
			{
				Thing *darci = NET_PERSON(0);

				ans = FALSE; // By default...

				if (darci)
				{
					SLONG dx = ec->arg1;
					SLONG dz = ec->arg2;

					SLONG minx = ew->x - dx;
					SLONG minz = ew->z - dz;
					SLONG maxx = ew->x + dx;
					SLONG maxz = ew->z + dz;
					SLONG maxy = ew->y;

					if (darci->WorldPos.Y < maxy)
					{
						if (WITHIN(darci->WorldPos.X >> 8, minx, maxx) &&
							WITHIN(darci->WorldPos.Z >> 8, minz, maxz))
						{
							ans = TRUE;
						}
					}
				}
			}

			break;

		case EWAY_COND_A_SEE_B:

			ans = FALSE; // by default...

			{
				SLONG i_a = ec->arg1;
				SLONG i_b = ec->arg2;

				SLONG person_a = EWAY_get_person(i_a);
				SLONG person_b = EWAY_get_person(i_b);

				if (person_a &&
					person_b)
				{
					if (TO_THING(person_a)->State != STATE_DEAD)
					{
						ans = can_a_see_b(
								TO_THING(person_a),
								TO_THING(person_b));
					}
				}
			}

			break;

		case EWAY_COND_GROUP_DEAD:

			//
			// Boy, this is slow!
			//

			ans = FALSE;

			{
				SLONG i;
				SLONG found_at_least_one_person;
				SLONG	eway_max=EWAY_way_upto;

				EWAY_Way *ew2;

				ew2 = &EWAY_way[0];
				for (i = 0; i < eway_max; i++)
				{
//					global_write=i;

					//
					// On Simons, request. This only checks for people in the 
					// same colour. It ignores their group.
					//

					if (ew2->colour == ew->colour)
					{
						//
						// This waypoint has our group and colour.
						//

						if (ew2->ed.type == EWAY_DO_CREATE_ENEMY)
						{
							if (ew2->ed.arg1)
							{
								extern SLONG is_person_dead(Thing *p_person);

								if (!is_person_dead(TO_THING(ew2->ed.arg1)))
								{	
									//
									// Found someone alive.
									//

									ans = FALSE;

									break;
								}
								else
								{
									//
									// Found at least one dead person- so make the default be TRUE.
									//

									ans = TRUE;
								}
							}
						}
					}
					ew2++;
				}
			}

			break;

		case EWAY_COND_HALF_DEAD:

			ans = FALSE;

			{
				SLONG i_person = EWAY_get_person(ec->arg1);

				if (i_person)
				{
					Thing *p_person = TO_THING(i_person);

					//
					// Is this person half dead?
					//

					if (WITHIN(p_person->Genus.Person->Health, 10, 100))
					{
						//return TRUE;
						ans = TRUE;
					}
				}
			}

			break;

		case EWAY_COND_ITEM_HELD:

			ans = FALSE;

			//
			// Is the player carrying our item?
			//

			if (ec->arg1 == SPECIAL_GUN)
			{
				if (NET_PERSON(0)->Flags & FLAGS_HAS_GUN)
				{
					ans = TRUE;
				}
			}
			else
			{
				if (person_has_special(
						NET_PERSON(0),
						ec->arg1))
				{
					ans = TRUE;
				}
			}

			break;

		case EWAY_COND_PERSON_USED:

			if (ec->arg1)
			{
				ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

				if (EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_ENEMY)
				{
					EWAY_Way *ew2 = &EWAY_way[ec->arg1];

					if (ew2->ed.arg1)
					{
						//
						// Set the PERSON_FLAG_USEABLE so that when the player
						// presses use near this person the waypoint system
						// will be alerted.
						//

						ASSERT(TO_THING(ew2->ed.arg1)->Class == CLASS_PERSON);

						TO_THING(ew2->ed.arg1)->Genus.Person->Flags |= FLAG_PERSON_USEABLE;
					}
				}
			}

			ans = FALSE;

			if (ew->es.type == EWAY_STAY_ALWAYS)
			{
				ans = !!(ew->flag & EWAY_FLAG_ACTIVE);
			}

			break;

		case EWAY_COND_RADIUS_USED:

			{
				Thing *darci = NET_PERSON(0);

				if (darci)
				{
					SLONG dx = abs(ew->x - (darci->WorldPos.X >> 8));
					SLONG dy = abs(ew->y - (darci->WorldPos.Y >> 8));
					SLONG dz = abs(ew->z - (darci->WorldPos.Z >> 8));

					SLONG dist = QDIST3(dx,dy,dz);

					if (dist < ec->arg1) EWAY_magic_radius_flag = ew;
				}

				//
				// Always return FALSE, this waypoint will be set active in 
				// do_an_action() in interfac.cpp.
				//

				ans = FALSE;
			}

			break;

		case EWAY_COND_PRIM_DAMAGED:

			ans = FALSE;
			
			if (ec->arg1)
			{
				ASSERT(WITHIN(ec->arg1, 1, OB_ob_upto - 1));

				if (OB_ob[ec->arg1].flags & OB_FLAG_DAMAGED)
				{
					ans = TRUE;
				}
			}

			break;

		case EWAY_COND_CONVERSE_END:

			ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));
			
			ans = FALSE;

			if (EWAY_way[ec->arg1].flag & EWAY_FLAG_FINISHED)
			{
				ans = TRUE;
			}

			break;

		case EWAY_COND_COUNTER_GTEQ:
			
			ASSERT(WITHIN(ec->arg1, 0, EWAY_MAX_COUNTERS - 1));

			ans = FALSE;

			if (EWAY_counter[ec->arg1] >= ec->arg2)
			{
				ans = TRUE;
			}

			break;

		case EWAY_COND_PERSON_ARRESTED:

			ans = FALSE; // By default.

			if (ec->arg1 == 0)
			{
				//
				// Hmm... person waypoint not set.
				//
#ifndef	FINAL
#ifndef	PSX
#ifndef TARGET_DC
				PANEL_new_text(NULL, 4000, "Waypoint %d (arrest) is invalid", ew->id);
#endif
#endif
#endif

				ew->flag |= EWAY_FLAG_DEAD;
			}
			else
			{
				ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));
				ASSERT(
					EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_ENEMY ||
					EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_PLAYER);

				UWORD index = EWAY_way[ec->arg1].ed.arg1;

				if (index)
				{
					Thing *p_person = TO_THING(index);

					ASSERT(p_person->Class == CLASS_PERSON);

					ans = !!(p_person->Genus.Person->Flags & FLAG_PERSON_ARRESTED) && (p_person->SubState==SUB_STATE_DEAD_ARRESTED);
				}
			}

			break;

		case EWAY_COND_PLAYER_CUBOID:
#ifdef	PSX
			if((ew-EWAY_way)==124 && wad_level==25)		 //miked remove wetback part
				ans=FALSE;
			else
#else
extern	UBYTE	is_semtex;
			if((ew-EWAY_way)==124 && is_semtex)		 //miked remove wetback part for PC/Dreamcast
				ans=FALSE;
			else

#endif
			{
				SLONG x1;
				SLONG z1;

				SLONG x2;
				SLONG z2;
				
				SLONG dx = ec->arg1;
				SLONG dz = ec->arg2;

				Thing *darci = NET_PERSON(0);

				{
					x1 = ew->x - dx;
					x2 = ew->x + dx;

					z1 = ew->z - dz;
					z2 = ew->z + dz;

					if (WITHIN(darci->WorldPos.X >> 8, x1, x2) &&
						WITHIN(darci->WorldPos.Z >> 8, z1, z2))
					{
//						ASSERT(0);
						ans = TRUE;
					}
					else
					{
						ans = FALSE;
					}
				}
			}

			break;

		case EWAY_COND_KILLED_NOT_ARRESTED:

			ans = FALSE; // By default.

			{
				SLONG waypoint;
				SLONG index;

				Thing    *p_thing;
				EWAY_Way *ew_dead;

				waypoint = ec->arg1;

				if (waypoint == 0)
				{
#ifndef PSX
					CBYTE mess[128];

					sprintf(mess, "Waypoint %d: cond person dead has NULL dependency", ew->id);

					CONSOLE_text(mess, 8000);
#endif
					ew->flag |= EWAY_FLAG_DEAD;
				}
				else
				{
					ew_dead = &EWAY_way[waypoint];

					switch(ew_dead->ed.type)
					{
						case EWAY_DO_CREATE_PLAYER:
						case EWAY_DO_CREATE_ENEMY:

							index = ew_dead->ed.arg1;

							if (index)
							{
								p_thing = TO_THING(index);

								ASSERT(p_thing->Class == CLASS_PERSON);

								if (p_thing->State == STATE_DEAD)
								{
									//
									// Not if they're arrested.
									//

									if (!(p_thing->Genus.Person->Flags & FLAG_PERSON_ARRESTED))
									{
										ans = TRUE;
									}
								}
							}

							break;

						default:


							{
							#ifndef PSX
								CBYTE mess[128];

								sprintf(mess, "Waypoint %d: killed not arrested dependent on odd waypoint", ew->id);

								CONSOLE_text(mess, 8000);
							#endif

								ew->flag |= EWAY_FLAG_DEAD;
							}


							break;
					}
				}
			}

			break;

		case EWAY_COND_CRIME_RATE_GTEQ: ans = (CRIME_RATE >= ec->arg1); break;
		case EWAY_COND_CRIME_RATE_LTEQ:	ans = (CRIME_RATE <= ec->arg1); break;

		case EWAY_COND_IS_MURDERER:

			{
				ans = FALSE;

				UWORD person = EWAY_get_person(ec->arg1);

				if (person)
				{
					Thing *p_person = TO_THING(person);

					if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_IS_MURDERER)
					{
						ans = TRUE;
					}
				}
			}

			break;

		case EWAY_COND_PERSON_IN_VEHICLE:

			ans = FALSE;

			{
				UWORD person;
				UWORD vehicle;

				person = EWAY_get_person(ec->arg1);

				if (person)
				{
					Thing *p_person = TO_THING(person);

					ASSERT(p_person->Class == CLASS_PERSON);

					if (p_person->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_BIKING))
					{
						if (ec->arg2)
						{
							//
							// They must be driving a specific vehicle!
							//

							ASSERT(WITHIN(ec->arg2, 1, EWAY_way_upto - 1));
							ASSERT(EWAY_way[ec->arg2].ed.type == EWAY_DO_CREATE_VEHICLE);

							if (p_person->Genus.Person->InCar == EWAY_way[ec->arg2].ed.arg1)
							{
								ans = TRUE;
							}
						}
						else
						{	
							ans = TRUE;
						}
					}
				}
			}

			break;

		case EWAY_COND_THING_RADIUS_DIR:
		case EWAY_COND_MOVE_RADIUS_DIR:
			
			ans = FALSE;

			{
				UWORD thing = EWAY_get_person(ec->arg1);

				if (thing == NULL)
				{
#ifndef	FINAL
#ifndef TARGET_DC
					#ifndef PSX
					PANEL_new_text(NULL, 10000, "Waypoint %d (in radius direction) bad", ew->id);
					#endif
#endif
#endif
				}
				else
				{
					Thing *p_thing = TO_THING(thing);
					SLONG  radius  = ec->arg2 * 64;	// Radius in quarter map-squares.

					SLONG dx = abs((p_thing->WorldPos.X >> 8) - ew->x);
					SLONG dy = abs((p_thing->WorldPos.Y >> 8) - ew->y);
					SLONG dz = abs((p_thing->WorldPos.Z >> 8) - ew->z);

					SLONG dist = QDIST3(dx,dy,dz);

					if (dist < radius)
					{
						SLONG dangle;
						SLONG angle;
						SLONG speed;

						//
						// Do the angles match up?
						//

						switch(p_thing->Class)
						{
							case CLASS_PERSON:
								angle = p_thing->Draw.Tweened->Angle;
								speed = p_thing->Velocity;
								break;

							case CLASS_VEHICLE:
								angle = p_thing->Genus.Vehicle->Angle;
								speed = p_thing->Velocity;
								break;
#ifndef PSX
#ifdef BIKE
							case CLASS_BIKE:
								angle = p_thing->Genus.Bike->yaw;
								speed = BIKE_get_speed(p_thing) >> 4;
								break;
#endif
#endif
							default:
								ASSERT(0);
								break;
						}

						if (ec->type == EWAY_COND_MOVE_RADIUS_DIR || abs(speed < 16))
						{
							dangle = angle - (ew->yaw << 3);

							if (dangle > +1024) {dangle -= 2048;}
							if (dangle < -1024) {dangle += 2048;}

							if (WITHIN(dangle, -128, +128))
							{
								//
								// Angle near enough too...
								//

								ans = TRUE;
							}
						}
					}
				}
			}

			break;

		case EWAY_COND_SPECIFIC_ITEM_HELD:

			ans = FALSE;

			{
				EWAY_Way *ew_other;

				ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

				ew_other = &EWAY_way[ec->arg1];

				ASSERT(ew_other->ed.type == EWAY_DO_CREATE_ITEM);

				if (ew_other->ed.arg2)
				{
					//
					// Has this special been picked up by Darci?
					//

					if (ew_other->flag & EWAY_FLAG_GOTITEM)
					{
						ans = TRUE;
					}
				}
			}

			break;

		case EWAY_COND_RANDOM:

			#ifndef NDEBUG
			if (ec->arg1 == NULL)
			{
				sprintf(EWAY_message, "Waypoint %d has a NULL dependency", ew->id);

				CONSOLE_text(EWAY_message, 8000);

				//
				// Don't print the message again.
				//

				ec->type = EWAY_COND_FALSE;
			}
			else
			#endif
			{
				ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

				if (EWAY_way[ec->arg1].flag & EWAY_FLAG_ACTIVE)
				{
					//
					// Look for all other EWAY_COND_RANDOM waypoints that are dependent
					// on the same waypoint as us.
					//

					#define EWAY_MAX_SAME 8

					SLONG i;
					UWORD same[EWAY_MAX_SAME];
					SLONG same_upto = 0;
					SLONG which;

					for (i = 1; i < EWAY_way_upto; i++)
					{
//					global_write1=i;
							if (EWAY_way[i].ec.type == EWAY_COND_RANDOM &&
							EWAY_way[i].ec.arg1 == ec->arg1)
						{
							ASSERT(WITHIN(same_upto, 0, EWAY_MAX_SAME - 1));

							same[same_upto++] = i;
						}
					}

					which = Random() % (same_upto + 1);

					if (which == same_upto)
					{
						//
						// None of the Random waypoint go active.
						//
					}
					else
					{
						//
						// Set active the Random waypoint that we've chosen.
						//

						void EWAY_set_active(EWAY_Way *ew);

						EWAY_set_active(&EWAY_way[same[which]]);
					}

					//
					// Kill all the random waypoints.
					//

					for (i = 0; i < same_upto; i++)
					{
						EWAY_way[same[i]].flag |= EWAY_FLAG_DEAD;
					}
				}
			}

			ans = FALSE;

			break;

		case EWAY_COND_PLAYER_FIRED_GUN:
			ans = !!(GAME_FLAGS & GF_PLAYER_FIRED_GUN);
			break;

		case EWAY_COND_PRIM_ACTIVATED:
			ans = !!(ew->flag & EWAY_FLAG_TRIGGERED);
			break;

		case EWAY_COND_DARCI_GRABBED:
			ans = (NET_PERSON(0)->State == STATE_FIGHTING && NET_PERSON(0)->SubState == SUB_STATE_GRAPPLE_HOLD);
			break;

		case EWAY_COND_PUNCHED_AND_KICKED:

			ans = FALSE;
			
			if (ec->arg1 == NULL)
			{
				#ifndef NDEBUG

				PANEL_new_text(NULL, 8000, "Waypoint %d has NULL dependency", ew->id);

				#endif
			}
			else
			{
				ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

				if (EWAY_way[ec->arg1].flag & EWAY_FLAG_ACTIVE)
				{
					if (ew->flag & EWAY_FLAG_CLEARED)
					{
						if (EWAY_darci_move & EWAY_DARCI_MOVE_PUNCH)
						{
							ec->arg2 += 1;

							EWAY_darci_move &= ~EWAY_DARCI_MOVE_PUNCH;
						}
						
						if (EWAY_darci_move & EWAY_DARCI_MOVE_KICK)
						{
							ec->arg2 += 0x100;

							EWAY_darci_move &= ~EWAY_DARCI_MOVE_KICK;
						}

						if ((ec->arg2 & 0xff) >= 2 &&
							(ec->arg2 >> 8)   >= 2)
						{
							ans = TRUE;
						}
					}
					else
					{
						//
						// The first process where this waypoint has gone active.
						//

						EWAY_darci_move = 0;

						ew->flag |= EWAY_FLAG_CLEARED;
					}
				}
			}

			break;

		case EWAY_COND_AFTER_FIRST_GAMETURN:
			ans = (GAME_TURN > 0);
			break;

		default:
			ASSERT(0);
			break;
	}

	if (ec->negate)
	{
		ans = !ans;
	}

	//
	// Skip all this falsifying code if we don't have
	// the final answer it.
	//

	if (EWAY_sub_condition_of_a_boolean)
	{
ANNOYINGSCRIBBLECHECK;

		return ans;
	}

	if (ew->ed.type == EWAY_DO_CONVERSATION ||
		ew->ed.type == EWAY_DO_AMBIENT_CONV ||
		ew->ed.type == EWAY_DO_MESSAGE)
	{
		if (GAME_TURN < 1)
		{
			ans = FALSE;
		}
	}

	if (ew->ed.type == EWAY_DO_MESSAGE && ans && !(ew->flag & EWAY_FLAG_ACTIVE))
	{
		//
		// If there is a voice playing, then delay the message triggering.
		// 

		if (!WITHIN(ew->ed.arg1, 0, EWAY_MAX_MESSES - 1))
		{
			//
			// Bad message...
			//
		}
		else
		{
			if (EWAY_used_thing)
			{
				if (MFX_QUICK_still_playing())
				{
					if (MUSIC_is_playing())
					{
						//
						// Just stop the music...
						//

						MFX_QUICK_stop();
						MFX_QUICK_wait();
					}
					else
					{
						ans = FALSE;
					}

					/*

					if (ew->es.type == EWAY_STAY_ALWAYS ||
						ew->es.type == EWAY_STAY_DIE)
					{
						ew->ec.type = EWAY_COND_TRUE;
					}

					*/
				}
			}
			else
			{
				if (ew->ed.arg2 == EWAY_MESSAGE_WHO_STREETNAME)
				{
				}
				else
				if (ew->ed.arg2 == EWAY_MESSAGE_WHO_TUTORIAL)
				{
				}
				else
				{
					Thing *who_says = NULL;

					if (ew->ed.arg2)
					{
						SLONG who = EWAY_get_person(ew->ed.arg2);

						if (who)
						{
							who_says = TO_THING(who);
						}
					}

					if (who_says && who_says->State == STATE_DEAD)
					{
						//
						// Dead people can't talk.
						//
					}
					else
					{
						if (MFX_QUICK_still_playing())
						{
							if (MUSIC_is_playing())
							{
								//
								// Just stop the music...
								//

								MFX_QUICK_stop();
								MFX_QUICK_wait();
							}
							else
							{
								ans = FALSE;
							}

							/*

							if (ew->es.type == EWAY_STAY_ALWAYS ||
								ew->es.type == EWAY_STAY_DIE)
							{
								ew->ec.type = EWAY_COND_TRUE;
							}

							*/
						}
					}
				}
			}
		}
	}

	if (ew->ed.type == EWAY_DO_CAMERA_CREATE && ans)
	{
		if (EWAY_stop_player_moving() && !EWAY_cam_goinactive)
		{
			ans = FALSE;
		}
	}

	if ((ew->ed.type == EWAY_DO_CONVERSATION || ew->ed.type == EWAY_DO_AMBIENT_CONV) && ans)
	{
		Thing *darci = NET_PERSON(0);

		//
		// Make sure that Darci isn't jumping or falling through the air!
		//

		if (!person_ok_for_conversation(darci))
		{
			ans = FALSE;
		}

		//
		// Make sure each person in the conversation is okay...
		//

		UWORD person1 = EWAY_get_person(ew->ed.arg1);
		UWORD person2 = EWAY_get_person(ew->ed.arg2);

		if (person1 == NULL ||
			person2 == NULL)
		{
			ans = FALSE;
		}
		else
		{
			Thing *p_person1 = TO_THING(person1);
			Thing *p_person2 = TO_THING(person2);

			if (!person_ok_for_conversation(p_person1) ||
			    !person_ok_for_conversation(p_person2))
			{
				ans = FALSE;
			}
			else
			{
				//
				// If there isn't a line of sight between the two people...
				//

				SLONG x1 = (p_person1->WorldPos.X >> 8);
				SLONG y1 = (p_person1->WorldPos.Y >> 8) + 0xa0;
				SLONG z1 = (p_person1->WorldPos.Z >> 8);

				SLONG x2 = (p_person2->WorldPos.X >> 8);
				SLONG y2 = (p_person2->WorldPos.Y >> 8) + 0xa0;
				SLONG z2 = (p_person2->WorldPos.Z >> 8);

				if (!there_is_a_los(
						x1, y1, z1,
						x2, y2, z2,
						LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
				{
					ans = FALSE;
				}
			}
		}
	}

	if(ans)
	{
#ifndef TARGET_DC
		// Bloody annoying on DC, and slows everything to a crawl.
//		TRACE("EVAL TRUE type %d way %d \n",ec->type,ew-EWAY_way);
#endif
	}

ANNOYINGSCRIBBLECHECK;

	return ans;
}



//
// Sets off a new cut-scene camera starting from the given waypoint.
//

void EWAY_create_camera(SLONG waypoint)
{
	SLONG i;


ANNOYINGSCRIBBLECHECK;


	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	EWAY_Way *ew;
	EWAY_Way *ew_target;
	
	ew = &EWAY_way[waypoint];

	EWAY_cam_active         = TRUE;
	EWAY_cam_goinactive     = FALSE;
	EWAY_cam_x              = ew->x << 8;
	EWAY_cam_y              = ew->y << 8;
	EWAY_cam_z              = ew->z << 8;
 	EWAY_cam_dx             = 0;
	EWAY_cam_dy             = 0;
	EWAY_cam_dz             = 0;
	EWAY_cam_yaw            = 0;
	EWAY_cam_pitch          = 0;
	EWAY_cam_speed          = ew->ed.arg1;
	EWAY_cam_delay          = ew->ed.arg2 * 10;
	EWAY_cam_waypoint       = waypoint;
	EWAY_cam_freeze         = !!(ew->ed.subtype & EWAY_SUBTYPE_CAMERA_LOCK_PLAYER);
	EWAY_cam_thing          = NULL;
	EWAY_cam_lens           = 0x28000;
	EWAY_cam_lock           = !!(ew->ed.subtype & EWAY_SUBTYPE_CAMERA_LOCK_DIRECTION);
	EWAY_cam_cant_interrupt = !!(ew->ed.subtype & EWAY_SUBTYPE_CAMERA_CANT_INTERRUPT);

	// This is benign
	ASSERT(EWAY_cam_cant_interrupt==0);
	EWAY_cam_last_yaw       = ew->yaw << 11;
	EWAY_cam_last_x         = ew->x << 8;
	EWAY_cam_last_y         = ew->y << 8;
	EWAY_cam_last_z         = ew->z << 8;
	EWAY_cam_skip           = 100;			// Can't skip for 1 second.
	EWAY_cam_last_dyaw      = 0;
	EWAY_conv_ambient		= 0;

	//
	// Find the lowest numbered camera-target waypoint.
	//

	EWAY_cam_target = EWAY_find_waypoint(1, EWAY_DO_CAMERA_TARGET, ew->colour, ew->group, FALSE);

	if (EWAY_cam_target == EWAY_NO_MATCH)
	{
		//
		// There is nothing for the camera to look at!
		//

		CONSOLE_text("No target for camera", 8000);

		EWAY_cam_active = FALSE;
	}

	MFX_stop(THING_NUMBER(NET_PERSON(0)),S_SEARCH_END);
	MFX_stop(THING_NUMBER(NET_PERSON(0)),S_SLIDE_START);

ANNOYINGSCRIBBLECHECK;

}



void EWAY_process_camera(void)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;
	SLONG next;
	SLONG speed;
	SLONG wspeed;

	SLONG target_stationary = FALSE;

	SLONG look_x;
	SLONG look_y;
	SLONG look_z;
	SLONG look_yaw = 0;

	EWAY_Way *ew_go;
	EWAY_Way *ew_look;
	EWAY_Way *ew_next;

ANNOYINGSCRIBBLECHECK;

	EWAY_cam_warehouse = NULL;

	if (!EWAY_cam_active)
	{
ANNOYINGSCRIBBLECHECK;
		return;
	}

	if (EWAY_cam_goinactive)
	{
		EWAY_cam_goinactive--;
		if (EWAY_cam_goinactive==0)
		{
			EWAY_cam_jumped=10;
			EWAY_cam_active     = FALSE;
			EWAY_cam_goinactive	= FALSE;
		}
ANNOYINGSCRIBBLECHECK;
		return;

	}
	timer_bored=0;

	EWAY_cam_skip -= EWAY_tick;

	//
	// Work out what we should look at.
	//

	if (EWAY_cam_thing)
	{
		Thing *p_thing = TO_THING(EWAY_cam_thing);

		look_x = (p_thing->WorldPos.X >> 8);
		look_y = (p_thing->WorldPos.Y >> 8) + 0xa0;
		look_z = (p_thing->WorldPos.Z >> 8);

		if (p_thing->Class == CLASS_PERSON)
		{
			EWAY_cam_warehouse = p_thing->Genus.Person->Ware;

			if (p_thing->State == STATE_IDLE)
			{
				target_stationary = TRUE;
			}
			if (p_thing->State == STATE_MOVEING)
			{
				if (p_thing->SubState == SUB_STATE_SIMPLE_ANIM ||
					p_thing->SubState == SUB_STATE_SIMPLE_ANIM_OVER)
					
				{
					target_stationary = TRUE;
				}
			}
		}
	}
	else if (!EWAY_cam_target)
	{
		// No waypoint _or_ thing -- so we must be forcing raw coordinates
		look_x = EWAY_cam_targx >>8;
		look_y = EWAY_cam_targy >>8;
		look_z = EWAY_cam_targz >>8;
	}
	else
	{
		//
		// The waypoint  we are looking at.
		//

		ASSERT(WITHIN(EWAY_cam_target, 1, EWAY_way_upto - 1));

		ew_look = &EWAY_way[EWAY_cam_target];

		//
		// Just look at the waypoint by default...
		//

		look_x   = ew_look->x;
		look_y   = ew_look->y;
		look_z   = ew_look->z;

		EWAY_cam_warehouse = ew_look->ware;

		switch(ew_look->ed.subtype)
		{
			case EWAY_SUBTYPE_CAMERA_TARGET_PLACE:
				target_stationary = TRUE;
				break;

			case EWAY_SUBTYPE_CAMERA_TARGET_THING:

				if (ew_look->ed.arg1)
				{
					ASSERT(WITHIN(ew_look->ed.arg1, 1, EWAY_way_upto - 1));

					EWAY_Way *ew_thing;

					ew_thing = &EWAY_way[ew_look->ed.arg1];

					//
					// Has this waypoint created a thing yet?
					//

					if (ew_thing->ed.arg1)
					{
						//
						// Yes! Look at the thing.
						//

						Thing *p_thing = TO_THING(ew_thing->ed.arg1);

						look_x = (p_thing->WorldPos.X >> 8);
						look_y = (p_thing->WorldPos.Y >> 8) + 0xa0;
						look_z = (p_thing->WorldPos.Z >> 8);

						if (p_thing->Class == CLASS_PERSON)
						{
							EWAY_cam_warehouse = p_thing->Genus.Person->Ware;

							if (p_thing->State == STATE_IDLE)
							{
								target_stationary = TRUE;
							}
							if (p_thing->State == STATE_MOVEING)
							{
								if (p_thing->SubState == SUB_STATE_SIMPLE_ANIM ||
									p_thing->SubState == SUB_STATE_SIMPLE_ANIM_OVER)
									
								{
									target_stationary = TRUE;
								}
							}
						}
					}
				}

				break;

			case EWAY_SUBTYPE_CAMERA_TARGET_NEAR:
				
				//
				// Search around for the nearest living thing.
				//

				{
					UWORD look_index = THING_find_nearest(
											ew_look->x,
											ew_look->y,
											ew_look->z,
											0x500,
											THING_FIND_LIVING);

					if (look_index)
					{
						Thing *p_look = TO_THING(look_index);

						look_x = p_look->WorldPos.X >> 8;
						look_y = p_look->WorldPos.Y >> 8;
						look_z = p_look->WorldPos.Z >> 8;

						if (p_look->Class == CLASS_PERSON)
						{
							EWAY_cam_warehouse = p_look->Genus.Person->Ware;

							if (p_look->State == STATE_IDLE)
							{
								target_stationary = TRUE;
							}
							if (p_look->State == STATE_MOVEING)
							{
								if (p_look->SubState == SUB_STATE_SIMPLE_ANIM ||
									p_look->SubState == SUB_STATE_SIMPLE_ANIM_OVER)
									
								{
									target_stationary = TRUE;
								}
							}
						}
					}
					else
					{
						//
						// If there is nothing to look at nearby- then just look
						// at the waypoint itself.
						//

						look_x            = ew_look->x;
						look_y            = ew_look->y;
						look_z            = ew_look->z;
						look_yaw          = ew_look->yaw << 11;
						target_stationary = TRUE;
					}
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}

	if (EWAY_cam_waypoint == NULL)
	{
		//
		// No camera movement required.
		//
	}
	else
	{
		ASSERT(WITHIN(EWAY_cam_waypoint, 1, EWAY_way_upto - 1));

		ew_go = &EWAY_way[EWAY_cam_waypoint];

		//
		// The direction locked cameras look in.
		//

		look_yaw = ew_go->yaw << 11;

		if (EWAY_stop_player_moving())
		{
			if (!EWAY_cam_cant_interrupt && EWAY_cam_skip <= 0)
			{
				if (!EWAY_conv_ambient)			 
				if (NET_PLAYER(0)->Genus.Player->Pressed & (INPUT_MASK_JUMP | INPUT_MASK_KICK | INPUT_MASK_ACTION | INPUT_MASK_PUNCH))
				{
#ifndef PSX
					MFX_QUICK_stop();
#else
					if (PANEL_wide_cont)
					{
						strcpy(PANEL_wide_text,PANEL_wide_cont);
						PANEL_wide_cont=0;
						goto skip_camera;
					} 
					else
					{
						MFX_QUICK_stop();
					}
#endif
					//
					// Mark all waypoints that this camera _would_ go to as arrived at.
					//

					while(1)
					{
						//
						// Mark this waypoint as active... if it is a CAMERA_WAYPOINT.
						//

						if (ew_go->ec.type == EWAY_COND_CAMERA_AT)
						{
							ew_go->flag |= EWAY_FLAG_ACTIVE | EWAY_FLAG_DEAD;
						}

						next = EWAY_find_waypoint(EWAY_cam_waypoint + 1, EWAY_DO_CAMERA_WAYPOINT, ew_go->colour, ew_go->group, FALSE);

						if (next == EWAY_NO_MATCH || next <= EWAY_cam_waypoint)
						{
							//
							// The search wrapped... there are no more waypoints for
							// the camera to use. Go inactive NEXT GAME TURN!
							//

							//if (EWAY_cam_goinactive==0)
								EWAY_cam_goinactive = 2;//TRUE;

ANNOYINGSCRIBBLECHECK;

							return;
						}

						EWAY_cam_waypoint = next;

						ASSERT(WITHIN(EWAY_cam_waypoint, 1, EWAY_way_upto - 1));

						ew_go = &EWAY_way[EWAY_cam_waypoint];
					}
				}
			}
			else
			{
				//
				// Pressing the key twice skips!
				//


				if (!EWAY_conv_ambient)
				if (NET_PLAYER(0)->Genus.Player->Pressed & (INPUT_MASK_JUMP | INPUT_MASK_KICK | INPUT_MASK_ACTION | INPUT_MASK_PUNCH))
				{
#ifdef VERSION_PSX
					if ((EWAY_cam_skip<=0)&&PANEL_wide_cont)
					{
						strcpy(PANEL_wide_text,PANEL_wide_cont);
						PANEL_wide_cont=0;
					}
					else
#endif
						EWAY_cam_skip = 0;
				}
			}
		}
#ifdef PSX
skip_camera:;
#endif
		//
		// How far to our next waypoint?
		//

		dx = ew_go->x - (EWAY_cam_x >> 8);
		dy = ew_go->y - (EWAY_cam_y >> 8);
		dz = ew_go->z - (EWAY_cam_z >> 8);

		dist = QDIST3(abs(dx),abs(dy),abs(dz)) + 1;

		if (dist < 0x80)
		{
			EWAY_cam_last_x   = EWAY_cam_x;
			EWAY_cam_last_y   = EWAY_cam_y;
			EWAY_cam_last_z   = EWAY_cam_z;
			EWAY_cam_last_yaw = EWAY_cam_yaw;

			if (EWAY_cam_delay == 10 * 100)
			{
				//
				// Maximum delay really means that it stays here while
				// the waypoint is active.
				//

				if (ew_go->flag & EWAY_FLAG_ACTIVE)
				{
					//
					// Just stay here...
					//	
				}
				else
				{
					//
					// Move onto the next waypoint.
					//

					EWAY_cam_delay = 0;
				}
			}
			else
			{
				//
				// Count down the delay.
				//

				EWAY_cam_delay -= EWAY_tick;

				if (EWAY_cam_delay < 0)
				{
					//
					// Mark this waypoint as active... if it is a CAMERA_WAYPOINT.
					//

					if (ew_go->ec.type == EWAY_COND_CAMERA_AT)
					{
						ew_go->flag |= EWAY_FLAG_ACTIVE | EWAY_FLAG_DEAD;
					}

					//
					// Move onto the next waypoint.
					//

					next = EWAY_find_waypoint(EWAY_cam_waypoint + 1, EWAY_DO_CAMERA_WAYPOINT, ew_go->colour, ew_go->group, FALSE);

					if (next == EWAY_NO_MATCH || next <= EWAY_cam_waypoint)
					{
						//
						// The search wrapped... there are no more waypoints for
						// the camera to use. Go inactive NEXT GAME TURN!
						//

						//if (EWAY_cam_goinactive==0)
							EWAY_cam_goinactive = 2;//TRUE;

						//
						// Make the camera look at Darci properly.
						// 
	#ifdef OLD_CAM
						CAM_look_at_thing(FALSE);
	#endif
ANNOYINGSCRIBBLECHECK;
						return;
					}

					ASSERT(WITHIN(next, 1, EWAY_way_upto - 1));

					ew_next = &EWAY_way[next];

					EWAY_cam_speed = ew_next->ed.arg1 << 8;
					EWAY_cam_delay = ew_next->ed.arg2 * 10;

					EWAY_cam_waypoint = next;
				}
			}
		}

		//
		// Accelerate towards our next waypoint.
		//

		#define EWAY_CAM_ACCEL_SPEED (EWAY_cam_speed >> 3)

		dx = (dx * EWAY_CAM_ACCEL_SPEED) / dist;
		dy = (dy * EWAY_CAM_ACCEL_SPEED) / dist;
		dz = (dz * EWAY_CAM_ACCEL_SPEED) / dist;

#if 0
		EWAY_cam_dx += dx;
		EWAY_cam_dy += dy;
		EWAY_cam_dz += dz;
#else
		EWAY_cam_dx += dx * TICK_RATIO >> TICK_SHIFT;
		EWAY_cam_dy += dy * TICK_RATIO >> TICK_SHIFT;
		EWAY_cam_dz += dz * TICK_RATIO >> TICK_SHIFT;
#endif

		//
		// Keep our speed at what we want.
		//

		speed = QDIST3(abs(EWAY_cam_dx),abs(EWAY_cam_dy),abs(EWAY_cam_dz)) + 1;

		if (dist >= 0x80)
		{
			wspeed = EWAY_cam_speed;
		}
		else
		{
			wspeed = (dist * EWAY_cam_speed) / 0x80;
		}

		if (speed > wspeed)
		{
			EWAY_cam_dx = (EWAY_cam_dx * wspeed) / speed;
			EWAY_cam_dy = (EWAY_cam_dy * wspeed) / speed;
			EWAY_cam_dz = (EWAY_cam_dz * wspeed) / speed;
		}

		//
		// Move the camera.
		//

#if 0
		EWAY_cam_x += EWAY_cam_dx;
		EWAY_cam_y += EWAY_cam_dy;
		EWAY_cam_z += EWAY_cam_dz;
#else
		EWAY_cam_x += EWAY_cam_dx * TICK_RATIO >> TICK_SHIFT;
		EWAY_cam_y += EWAY_cam_dy * TICK_RATIO >> TICK_SHIFT;
		EWAY_cam_z += EWAY_cam_dz * TICK_RATIO >> TICK_SHIFT;
#endif
	}

	//
	// Look at our target.
	//

	dx = look_x - (EWAY_cam_x >> 8) >> 1;
	dy = look_y - (EWAY_cam_y >> 8) >> 1;
	dz = look_z - (EWAY_cam_z >> 8) >> 1;

	SLONG dxz = QDIST2(abs(dx),abs(dz));

	//
	// Look at the right part of the thing.
	//
						
	EWAY_cam_want_yaw    = -Arctan(dx,dz);
	EWAY_cam_want_pitch  =  Arctan(dy,dxz);

	EWAY_cam_want_yaw    &= 2047;
	EWAY_cam_want_pitch  &= 2047;

	EWAY_cam_want_yaw   <<= 8;
	EWAY_cam_want_pitch <<= 8;

	SLONG dyaw   = EWAY_cam_want_yaw   - EWAY_cam_yaw;
	SLONG dpitch = EWAY_cam_want_pitch - EWAY_cam_pitch;

	dyaw   &= (2048 << 8) - 1;
	dpitch &= (2048 << 8) - 1;

	if (dyaw   > (1024 << 8)) {dyaw   -= (2048 << 8);}
	if (dpitch > (1024 << 8)) {dpitch -= (2048 << 8);}

	if (EWAY_cam_lock)
	{
		if (EWAY_cam_lock != INFINITY)
		{
			EWAY_cam_pitch = EWAY_cam_want_pitch;

			EWAY_cam_lock = INFINITY;
		}

		#if 1

		dyaw  = look_yaw - EWAY_cam_yaw;
		dyaw &= (2048 << 8) - 1;

		if (dyaw > (1024 << 8)) {dyaw -= 2048 << 8;}

		dyaw = (EWAY_cam_last_dyaw * 3) + dyaw >> 2;

		EWAY_cam_yaw      += dyaw >> 5;
		EWAY_cam_last_dyaw = dyaw;

		#else

		//
		// The yaw is interpolated.
		//

		SLONG along;
		SLONG whole_dist;
		SLONG along_dist;

		dx = (EWAY_cam_x >> 8) - (EWAY_cam_last_x >> 8);
		dz = (EWAY_cam_y >> 8) - (EWAY_cam_last_y >> 8);
		dz = (EWAY_cam_z >> 8) - (EWAY_cam_last_z >> 8);

		along_dist = QDIST3(abs(dx),abs(dy),abs(dz));

		dx = ew_go->x - (EWAY_cam_last_x >> 8);
		dy = ew_go->y - (EWAY_cam_last_y >> 8);
		dz = ew_go->z - (EWAY_cam_last_z >> 8);

		whole_dist = QDIST3(abs(dx),abs(dy),abs(dz));

		if (whole_dist <= 0x140)
		{
			along = 0;
		}
		else
		if (along_dist < 0xa0)
		{
			along = 0;
		}
		else
		if (along_dist > whole_dist - 0xa0)
		{
			along = 2048;
		}
		else
		{
			along = (along_dist - 0xa0 << 11) / (whole_dist - 0x140);
		}

		dyaw  = look_yaw - EWAY_cam_last_yaw;
		dyaw &= (2048 << 8) - 1;

		if (dyaw > (1024 << 8)) {dyaw -= 2048 << 8;}

		EWAY_cam_yaw  = EWAY_cam_last_yaw + (dyaw * along >> 11);
		EWAY_cam_yaw &= (2048 << 8) - 1;

		#endif
	}
	else
	{
		SLONG max_dangle = 200 << 8;

		if (target_stationary)
		{
			max_dangle = 100 << 8;
		}

		if (abs(dyaw  ) > max_dangle ||
			abs(dpitch) > max_dangle)
		{
/*
			if(dyaw>0)
			{
				EWAY_cam_yaw   += MAX(dyaw   >> 2,dyaw-max_dangle);
			}
			else
			{
				EWAY_cam_yaw   -= MAX(-dyaw   >> 2,-dyaw-max_dangle);
			}
			if(dpitch>0)
			{
				EWAY_cam_pitch   += MAX(dpitch   >> 2,dpitch-max_dangle);
			}
			else
			{
				EWAY_cam_pitch   -= MAX(-dpitch   >> 2,-dpitch-max_dangle);
			}
*/

			EWAY_cam_yaw   = EWAY_cam_want_yaw;
			EWAY_cam_pitch = EWAY_cam_want_pitch;
//			EWAY_cam_yaw   += dyaw   >> 1;
//			EWAY_cam_pitch += dpitch >> 1;
		}

		else

		{
			EWAY_cam_yaw   += dyaw   >> 1;
			EWAY_cam_pitch += dpitch >> 1;
		}
	}

ANNOYINGSCRIBBLECHECK;

}

//
// Finishes a conversation.
// 

void EWAY_finish_conversation(void)
{
	EWAY_conv_active = FALSE;

ANNOYINGSCRIBBLECHECK;

	if (!EWAY_conv_ambient)
	{
		EWAY_cam_relinquish();
	}

	ASSERT(WITHIN(EWAY_conv_waypoint, 1, EWAY_way_upto - 1));

	EWAY_way[EWAY_conv_waypoint].flag |= EWAY_FLAG_FINISHED;

	PCOM_stop_people_talking_to_eachother(
		TO_THING(EWAY_conv_person_a),
		TO_THING(EWAY_conv_person_b));

	extern THING_INDEX PANEL_wide_top_person;
	extern THING_INDEX PANEL_wide_bot_person;

	PANEL_wide_top_person = NULL;
	PANEL_wide_bot_person = NULL;

ANNOYINGSCRIBBLECHECK;

}

//
// Processes a conversation.
//

void EWAY_process_conversation(void)
{
	CBYTE *ch;
	CBYTE *str;

ANNOYINGSCRIBBLECHECK;

	check_eway_talk(0);
	if (!EWAY_conv_active)
	{
		return;
	}
	timer_bored=0;

//	ASSERT(GAME_TURN != 12313);

	//
	// Time for the next person to talk?
	//

	EWAY_conv_timer -= EWAY_tick;
	EWAY_conv_skip  -= EWAY_tick;

	if (EWAY_conv_skip <= 0)
	{
//		if(0)
		if (!EWAY_conv_ambient)
		{
			if (NET_PLAYER(0)->Genus.Player->Pressed & (INPUT_MASK_JUMP | INPUT_MASK_KICK | INPUT_MASK_ACTION | INPUT_MASK_PUNCH))
			{
				//
				// cut off the voice-overs
				//
#ifndef PSX
				MFX_QUICK_stop();

				EWAY_conv_timer = 0;
#else
				if (PANEL_wide_cont)
				{
					strcpy(PANEL_wide_text,PANEL_wide_cont);
					PANEL_wide_cont=0;
				}
				else
				{
					MFX_QUICK_stop();
					EWAY_conv_timer=0;
				}
#endif
			}
		}
#ifdef PSX
		else
		{
			if (NET_PLAYER(0)->Genus.Player->Pressed & (INPUT_MASK_JUMP | INPUT_MASK_KICK | INPUT_MASK_ACTION | INPUT_MASK_PUNCH))
			{
				if (PANEL_wide_cont)
				{
					strcpy(PANEL_wide_text,PANEL_wide_cont);
					PANEL_wide_cont=0;
				}
			}
		}
#endif
	}
	else
	{
//		if(0)
		if (!EWAY_conv_ambient)
		if (NET_PLAYER(0)->Genus.Player->Pressed & (INPUT_MASK_JUMP | INPUT_MASK_KICK | INPUT_MASK_ACTION | INPUT_MASK_PUNCH))
		{
			//
			// Two presses in a row skips...
			//

			EWAY_conv_skip = 0;
		}
	}

	if (EWAY_conv_timer <= 0 || (EWAY_conv_talk && MFX_QUICK_still_playing()==0))
	{
		str = &EWAY_mess_buffer[EWAY_conv_str];

		//
		// Reached the end of the conversation?
		//

		if (*str == 0)
		{
			EWAY_finish_conversation();

ANNOYINGSCRIBBLECHECK;

			return;
		}

		EWAY_talk_conv((EWAY_way[EWAY_conv_waypoint].yaw<<8)+(EWAY_way[EWAY_conv_waypoint].index),EWAY_conv_str_count);

		//
		// What are they saying? Create a NULL terminated string.  Work out
		// how long the string is and how much time wait before the next string.
		//
//		ASSERT(0);
		for (ch = str, EWAY_conv_timer = 200; *ch != '\000' && *ch != '|'; ch++, EWAY_conv_str += 1, EWAY_conv_timer += 4);

		if (ch[0] == '|')
		{
			EWAY_conv_str += 1;
			EWAY_conv_str_count++;

		    ch[0]          = '\000';
		}
		
		EWAY_conv_skip = 50;

#if 0

		PANEL_new_text(
			TO_THING(EWAY_conv_person_a),
			EWAY_conv_timer * 10 - 500,
			"(%d) %s",
			(EWAY_way[EWAY_conv_waypoint].yaw<<8)+(EWAY_way[EWAY_conv_waypoint].index),
			str);

#else

		PANEL_new_text(
			TO_THING(EWAY_conv_person_a),
			EWAY_conv_timer * 10 - 500,
			str);

#endif


		//
		// Make our two people say stuff.
		//

		PCOM_make_people_talk_to_eachother(
			TO_THING(EWAY_conv_person_a),
			TO_THING(EWAY_conv_person_b),
			ch[-1] == '?' || ch[-2] == '?',
			TRUE);

		if (!EWAY_conv_ambient)
		{
			//
			// Look at who ever is talking.
			//

			EWAY_cam_converse(TO_THING(EWAY_conv_person_a),TO_THING(EWAY_conv_person_b));
		}

		//
		// Swap whos talking.
		//

		SWAP(EWAY_conv_person_a, EWAY_conv_person_b);
		EWAY_cam_jumped=10;
	}

	//
	// If either of the people in the conversation are attacked, run-over or if anything
	// unusual happens to them, then stop the conversation.
	//

	{
		Thing *p_person_a = TO_THING(EWAY_conv_person_a);
		Thing *p_person_b = TO_THING(EWAY_conv_person_b);

		if ((p_person_a->SubState != SUB_STATE_SIMPLE_ANIM && p_person_a->SubState != SUB_STATE_SIMPLE_ANIM_OVER && p_person_a->State != STATE_IDLE) ||
			(p_person_b->SubState != SUB_STATE_SIMPLE_ANIM && p_person_b->SubState != SUB_STATE_SIMPLE_ANIM_OVER && p_person_b->State != STATE_IDLE))
		{
			EWAY_finish_conversation();

ANNOYINGSCRIBBLECHECK;

			return;
		}
	}

ANNOYINGSCRIBBLECHECK;

}


//
// Processes a waypoint that is emitting steam.
//

void EWAY_process_emit_steam(EWAY_Way *ew)
{
	SLONG tick;

	SLONG speed;
	SLONG steps;
	SLONG range;
	SLONG choreography;

ANNOYINGSCRIBBLECHECK;

	ASSERT(ew->flag & EWAY_FLAG_ACTIVE);
	ASSERT(ew->ed.type == EWAY_DO_EMIT_STEAM);

	speed = (ew->ed.arg2 >> 10) & 0x3f;
	steps = (ew->ed.arg2 >>  6) & 0x0f;
	range = (ew->ed.arg2 >>  0) & 0x3f;

	tick  = EWAY_time * speed >> 7;
	tick %= steps;

	choreography = ew->ed.arg1;

	if (choreography & (1 << tick))
	{
		SLONG dx;
		SLONG dy;
		SLONG dz;

		switch(ew->ed.subtype)
		{
			case EWAY_SUBTYPE_STEAM_FORWARD:
				dx = -SIN(ew->yaw << 3) * range >> 8;
				dy = 0;                         
				dz = -COS(ew->yaw << 3) * range >> 8;
				break;

			case EWAY_SUBTYPE_STEAM_UP:
				dx = 0;
				dy = range << 8;
				dz = 0;
				break;

			case EWAY_SUBTYPE_STEAM_DOWN:
				dx =  0;
				dy = -range << 8;
				dz =  0;
				break;

			default:
				ASSERT(0);
				break;
		}

		dx += (Random() & 0xff) - 0x7f;
		dy += (Random() & 0xff) - 0x7f;
		dz += (Random() & 0xff) - 0x7f;
	
		PARTICLE_Add(
			ew->x + (Random() & 0x7) - 0x3 << 8,
			ew->y + (Random() & 0x7) - 0x3 << 8,
			ew->z + (Random() & 0x7) - 0x3 << 8,
			dx,
			dy,
			dz,
			POLY_PAGE_SMOKECLOUD2,
			2 + ((Random() & 0x3) << 2),
			0x7fe8ffd0,
			PFLAG_SPRITEANI | PFLAG_SPRITELOOP | PFLAG_FADE2 | PFLAG_RESIZE | PFLAG_HURTPEOPLE,
			100,
			40,
			1,
			1,
			1);
	}

ANNOYINGSCRIBBLECHECK;

}


//
// Does what has to be done when a waypoint goes active.
//

void EWAY_set_active(EWAY_Way *ew)
{
	SLONG has;

ANNOYINGSCRIBBLECHECK;

	ew->flag |= EWAY_FLAG_ACTIVE;

	switch(ew->ed.type)
	{
		case EWAY_DO_NOTHING:
			break;

		case EWAY_DO_CREATE_PLAYER:

			has=(ew->ed.subtype&8) ? 0 : PCOM_HAS_GUN;

			ew->ed.arg1 = EWAY_create_player(
							ew->ed.subtype,
							ew->yaw,
							has,
							ew->x,
							ew->y,
							ew->z);
			break;

		case EWAY_DO_CREATE_ANIMAL:
			ew->ed.arg1 = EWAY_create_animal(
							ew->ed.subtype,
							ew->yaw,
							ew->x,
							ew->y,
							ew->z);
			break;

		case EWAY_DO_CREATE_ENEMY:
//#ifndef PSX
			{
				EWAY_Edef *ee;

				ASSERT(WITHIN(ew->index, 1, EWAY_edef_upto - 1));

				ee = &EWAY_edef[ew->index];

				ew->ed.arg1 = EWAY_create_enemy(
								ew->ed.subtype,
								ew->yaw,
								ew->colour,
								ew->group,
								ee->pcom_ai,
								ee->pcom_bent,
								ee->pcom_move,
								ee->drop,
								ee->zone,
								ee->ai_skill,
								ee->follow,
								ee->ai_other,
								ee->pcom_has,
								ew->x,
								ew->y,
								ew->z,
								ew - EWAY_way);
			}
//#endif
			break;

		case EWAY_DO_CREATE_ITEM:

			extern void find_nice_place_near_person(
							Thing *p_person,
							SLONG *nice_x,	// 8-bits per mapsquare
							SLONG *nice_y,
							SLONG *nice_z);


			if ((ew->ed.arg1&EWAY_ARG_ITEM_FOLLOW_PERSON)&&
			    (
				(ew->ec.type==EWAY_COND_KILLED_NOT_ARRESTED)||
				(ew->ec.type==EWAY_COND_PERSON_ARRESTED)||
				(ew->ec.type==EWAY_COND_PERSON_DEAD)||
				(ew->ec.type==EWAY_COND_HALF_DEAD)||
				(ew->ec.type=EWAY_COND_PERSON_USED)
				) 
			   )
			{
				ASSERT(ew->ec.arg1);

				if (ew->ec.arg1)
				{
					SLONG person = EWAY_get_person(ew->ec.arg1);

					ASSERT(person);

					if (person)
					{
						SLONG item_x;
						SLONG item_y;
						SLONG item_z;

						Thing *p_bloke = TO_THING(person);

						find_nice_place_near_person(
							p_bloke,
						   &item_x,
						   &item_y,
						   &item_z);

						ew->x = item_x;
						ew->y = item_y;
						ew->z = item_z;
#ifndef	FINAL
//						PANEL_new_text(p_bloke, 4000, "Creating item from me");
#endif
					}
				}
			}

			ew->ed.arg2 = EWAY_create_item(
							ew->ed.subtype,
							ew->x,
							ew->y,
							ew->z,
							ew);

			break;

		case EWAY_DO_CREATE_VEHICLE:
			ew->ed.arg1 = EWAY_create_vehicle(
							ew->ed.subtype,
							ew->ed.arg1,
							ew->ed.arg2,
							ew->x,
							ew->y,
							ew->z,
							((ew->yaw << 3) + 1024) & 2047);

			break;
		case EWAY_DO_SOUND_ALARM:

			MFX_play_xyz(0,S_KICK_CAN,MFX_OVERLAP,ew->x<<8,ew->y<<8,ew->z<<8);
/*			play_quick_wave_xyz(
				ew->x << 8,
				ew->y << 8,
				ew->z << 8,
				S_SIREN_START,
				0,
				WAVE_PLAY_INTERUPT);*/

			PCOM_oscillate_tympanum(
				PCOM_SOUND_ALARM,
				NULL,
				ew->x,
				ew->y,
				ew->z);

			break;

		case EWAY_DO_SOUND_EFFECT:
			play_glue_wave(ew->ed.subtype,ew->ed.arg1,ew->x<<8, ew->y<<8, ew->z<<8);
			break;

		case EWAY_DO_CONTROL_DOOR:

			DOOR_open(
				ew->x,
				ew->z);

			break;

		case EWAY_DO_EXPLODE:

			//
			// Bang!
			// 
/*
			POW_create(
				POW_CREATE_LARGE_SEMI,
				ew->x << 8,
				ew->y << 8,
				ew->z << 8);
*/

			{
				GameCoord posn;

				posn.X = ew->x << 8;
				posn.Y = ew->y << 8;
				posn.Z = ew->z << 8;

				PYRO_construct(
					posn,
					ew->ed.subtype,
					ew->ed.arg1);
			}

			if (ew->ed.subtype=!16) {

				//
				// 16 is fire on its own...
				//

				create_shockwave(
					ew->x,
					ew->y,
					ew->z,
					0x400,
					100,
					NULL);
			}

			break;

		case EWAY_DO_SPOT_FX:

			// water and similar stuff
			GameCoord posn;
			Thing *pyro;

			posn.X = ew->x << 8;
			posn.Y = ew->y << 8;
			posn.Z = ew->z << 8;
			pyro=PYRO_create(posn,PYRO_IRONICWATERFALL);
			if(pyro)
			{
				IRONICWATERFALL_state_function[STATE_INIT].StateFn(pyro);
	//			PYRO_fn_init(pyro);
				pyro->Genus.Pyro->radius=ew->ed.subtype;
				pyro->Genus.Pyro->scale =ew->ed.arg1;
			}
		break;

		case EWAY_DO_MESSAGE:

			//
			// Add a message to the console.
			// 

			if (!WITHIN(ew->ed.arg1, 0, EWAY_MAX_MESSES - 1))
			{
				CONSOLE_text("Too many messages for the waypoint system! Tell Mark!", 8000);
			}
			else
			{
				SLONG time = ew->ed.subtype;

				if (time)
				{
					time *= 1000;
				}
				else
				{
					time = 8000;
				}

				if (EWAY_mess[ew->ed.arg1] == NULL)
				{
					#ifndef NDEBUG
					CONSOLE_text("Bug! No message defined");
					#endif
				}
				else
				{
					if (EWAY_used_thing)
					{

						talk_thing=TO_THING(EWAY_used_thing);
						EWAY_talk((ew->yaw<<8)+ew->index);

						// HACK: JDW - Add waypoint number to start of messages

						#if 0

						PANEL_new_text(
							TO_THING(EWAY_used_thing),
							time,
							"(%d) %s",
							(ew->yaw << 8) + ew->index,
							EWAY_mess[ew->ed.arg1]);
  
						#else

						PANEL_new_text(
							TO_THING(EWAY_used_thing),
							time,
							EWAY_mess[ew->ed.arg1]);

						#endif


						if (person_ok_for_conversation(TO_THING(EWAY_used_thing)))
						{
							PCOM_make_people_talk_to_eachother(
								TO_THING(EWAY_used_thing),
								NET_PERSON(0),
								FALSE,
								FALSE);
						}
					}
					else
					{
						if (ew->ed.arg2 == EWAY_MESSAGE_WHO_STREETNAME)
						{
							PANEL_new_help_message(EWAY_mess[ew->ed.arg1]);
						}
						else
						if (ew->ed.arg2 == EWAY_MESSAGE_WHO_TUTORIAL)
						{
							EWAY_tutorial_string  = EWAY_mess[ew->ed.arg1];
							EWAY_tutorial_counter = 0;

							MFX_stop(THING_NUMBER(NET_PERSON(0)),S_SEARCH_END);
							MFX_stop(THING_NUMBER(NET_PERSON(0)),S_SLIDE_START);


							#ifndef PSX
							LastKey = 0;
							#endif
						}
						else
						{
							Thing *who_says = NULL;

							if (ew->ed.arg2)
							{
								SLONG who = EWAY_get_person(ew->ed.arg2);

								if (who)
								{
									who_says = TO_THING(who);
								}
							}

							if (who_says && who_says->State == STATE_DEAD)
							{
								//
								// Dead people can't talk.
								//
							}
							else
							{
								talk_thing=who_says;
								EWAY_talk((ew->yaw<<8)+ew->index);

								#if 0

								PANEL_new_text(
									who_says,
									time,
									"(%d) %s",
									(ew->yaw << 8) + ew->index,
									EWAY_mess[ew->ed.arg1]);

  
								#else

								PANEL_new_text(
									who_says,
									time,
									EWAY_mess[ew->ed.arg1]);

								#endif

#ifdef TARGET_DC
								extern int g_iLevelNumber;
								if ( g_iLevelNumber == 4)
								{
									// Spot them driving the wrong way round Driving Bronze
									// These magic numbers are the messages for "1/2/3 laps complete".

									SLONG yaw_car;
									if ( NET_PERSON(0)->Genus.Person->Flags & FLAG_PERSON_DRIVING )
									{
										Thing *p_vehicle = TO_THING(NET_PERSON(0)->Genus.Person->InCar);
										ASSERT ( p_vehicle != NULL );
										yaw_car = p_vehicle->Genus.Vehicle->Angle;
										// 2048 in a ful circle. and the car normally crosses at 1024,
										// so we want the range around 0.
										bool bCarGoingBackwards = FALSE;
										if ( ( yaw_car > ( 1024+512 ) ) || ( yaw_car < 512 ) )
										{
											bCarGoingBackwards = TRUE;
										}

										// Stores the info between laps.
										static m_bBackwardsCarCheat = FALSE;

										if ( who_says == 0 )
										{
											switch ( ew->ed.arg1 )
											{
											case 6:
												// 1 LAP COMPLETE
												m_bBackwardsCarCheat = bCarGoingBackwards;
												TRACE ( "1: Car yaw: %i\n", yaw_car );
												TRACE ( "Cheating: %i\n", (int)m_bBackwardsCarCheat );
												if ( bCarGoingBackwards )
												{
													if ( 0 == ENV_get_value_number ( "lang_num", 0, "" ) )
													{
														CONSOLE_text ( "D'arci, stop muckin' around!", 8000 );
													}
													else
													{
														CONSOLE_text ( "D'arci arrête de jouer !", 8000 );
													}
												}
												break;
											case 5:
												// 2 LAPS COMPLETE
												if ( !bCarGoingBackwards )
												{
													m_bBackwardsCarCheat = FALSE;
												}
												else
												{
													if ( 0 == ENV_get_value_number ( "lang_num", 0, "" ) )
													{
														CONSOLE_text ( "That's not good for the gearbox!", 8000 );
													}
													else
													{
														CONSOLE_text ( "La boîte de vitesse D'arci !", 8000 );
													}
												}
												TRACE ( "2: Car yaw: %i\n", yaw_car );
												TRACE ( "Cheating: %i\n", (int)m_bBackwardsCarCheat );
												break;
											case 4:
												// 3 LAPS COMPLETE
												if ( !bCarGoingBackwards )
												{
													m_bBackwardsCarCheat = FALSE;
												}
												TRACE ( "3: Car yaw: %i\n", yaw_car );
												TRACE ( "Cheating: %i\n", (int)m_bBackwardsCarCheat );
												if ( m_bBackwardsCarCheat )
												{
													// Did all three laps going backwards - activate the cheat.
													TRACE ( "Cheat activated!\n" );
													ENV_set_value_number ( "cheat_driving_bronze", 1, "" );
													if ( 0 == ENV_get_value_number ( "lang_num", 0, "" ) )
													{
														CONSOLE_text ( "Well done D'arci - that car's going to need a full servicing now.", 8000 );
													}
													else
													{
														CONSOLE_text ( "Bien joué D'arci. La bagnole est bonne pour la casse maintenant !", 8000 );
													}
												}
												break;
											default:
												// Do nothing.
												break;
											}
										}
									}
								}
#endif


								if (who_says && who_says != NET_PERSON(0))
								{
									if (person_ok_for_conversation(who_says))
									{
										PCOM_make_people_talk_to_eachother(
											who_says,
											NET_PERSON(0),
											FALSE,
											FALSE,
											FALSE);
									}
								}
							}
						}
					}
#ifndef PSX
					if (ew->flag & EWAY_FLAG_WHY_LOST)
					{
						//
						// This is a message that comes up when you've lost the level.
						//

						GAMEMENU_set_level_lost_reason(EWAY_mess[ew->ed.arg1]);
					}
#endif
				}
			}

			break;

		case EWAY_DO_NAV_BEACON:


			//
			// Add a nav beacon to the map.
			// 

			{
				CBYTE *str;
				UWORD  track_thing = NULL;

				if (!WITHIN(ew->ed.arg1, 0, EWAY_MAX_MESSES - 1))
				{
					CONSOLE_text("Too many navbeacon messages for the waypoint system! Tell Mark!");
				}
				else
				{
					str = EWAY_mess[ew->ed.arg1];
				}

				track_thing = EWAY_get_person(ew->ed.arg2);

				if(ew->ed.arg2 && track_thing==0)
				{
					//
					// Person not created yet, maybe next time!
					//
					ew->flag &= ~EWAY_FLAG_ACTIVE;
ANNOYINGSCRIBBLECHECK;

					return;

				}
				else
				{

					ew->ed.subtype = MAP_beacon_create(ew->x,ew->z, ew->ed.arg1, track_thing);
					ASSERT(ew->ed.subtype);

					if (GAME_TURN < 16)
					{
						MFX_play_stereo(0,S_MENU_END,MFX_REPLACE);
					}

				}

//

//				CBYTE msg[300];
//				strcpy(msg,"if it was finished yet, beacon \"");
//				if (EWAY_mess[ew->ed.arg])
//					strcat(msg,EWAY_mess[ew->ed.arg]);
//				else
//					strcat(msg,"default");
//				strcat(msg,"\" would be active.");
//				CONSOLE_text(msg,4000);
//

//void	set_beacon(SLONG bx,SLONG by,SLONG bz);
//				set_beacon(ew->x,ew->y,ew->z);

			}


			break;

		case EWAY_DO_ELECTRIFY_FENCE:

			//
			// Turn on the electric fence.
			//

			if (ew->ed.arg1)
			{
				set_electric_fence_state(ew->ed.arg1, TRUE);
			}

			break;

		case EWAY_DO_CAMERA_CREATE:
 			EWAY_create_camera(ew - EWAY_way);
			if (EWAY_cam_freeze) MFX_play_stereo(0,S_CUTSCENE_STING,0);
			break;

		case EWAY_DO_CAMERA_WAYPOINT:
			break;

		case EWAY_DO_CAMERA_TARGET:
			break;

		case EWAY_DO_MISSION_FAIL:
			if (GAME_STATE != GS_LEVEL_WON)
			{
				GAME_STATE = GS_LEVEL_LOST;
			}
			break;

		case EWAY_DO_MISSION_COMPLETE:
			if (GAME_STATE != GS_LEVEL_LOST)
			{
				//
				// Make sure Darci doesn't die!
				//

				NET_PERSON(0)->Genus.Person->Flags2 |= FLAG2_PERSON_INVULNERABLE;

				GAME_STATE = GS_LEVEL_WON;
extern	void	set_stats(void);
				set_stats();
			}
			break;

		case EWAY_DO_CHANGE_ENEMY:

//			ASSERT(WITHIN(ew->ed.arg1, 1, EWAY_way_upto  - 1));
			ASSERT(WITHIN(ew->index,   1, EWAY_edef_upto - 1));

			{
				EWAY_Edef *ee = &EWAY_edef[ew->index];

				SLONG change = EWAY_get_person(ew->ed.arg1);

				if (change == NULL)
				{
#ifdef	MIKE
//					ASSERT(0);
					PANEL_new_text(NULL, 5000, "Adjust enemy waypoint %d: No enemy to change", ew->id);
#endif
#ifndef NDEBUG
#ifndef	PSX
					PANEL_new_text(NULL, 5000, "Adjust enemy waypoint %d: No enemy to change", ew->id);
#endif
#endif
				}
				else
				{
					Thing *p_change = TO_THING(change);
					
					PCOM_change_person_attributes(
						p_change,
						ew->colour,
						ew->group,
						ee->pcom_ai,
						ee->ai_other,
						ee->pcom_move,
						ee->follow,
						ee->pcom_bent,
						ew->yaw << 3);
				}
			}

			break;

		case EWAY_DO_CHANGE_ENEMY_FLG:
			{
				SLONG change = EWAY_get_person(ew->ed.arg1);

				if (change == NULL)
				{
#ifdef	MIKE
//					ASSERT(0);
					PANEL_new_text(NULL, 5000, "Adjust enemy flags waypoint %d: No enemy to change", ew->id);
#endif
#ifndef NDEBUG
#ifndef	PSX
					PANEL_new_text(NULL, 5000, "Adjust enemy flags waypoint %d: No enemy to change", ew->id);
#endif
#endif
				}
				else
				{
					Thing *p_change = TO_THING(change);
					
					p_change->Genus.Person->pcom_bent   = ew->ed.arg2;
					p_change->Genus.Person->pcom_zone	= ew->ed.arg2>>8;
extern void PCOM_set_person_ai_normal(Thing *p_person);
					PCOM_set_person_ai_normal(p_change);
				}
			}
			break;

		case EWAY_DO_CREATE_PLATFORM:
			ew->ed.arg1 = PLAT_create(
							ew->colour,
							ew->group,
							PLAT_MOVE_PATROL,
							ew->ed.arg2,
							ew->ed.arg1,
							ew->x,
							ew->y,
							ew->z);
			break;
		case EWAY_DO_CREATE_BOMB:
			
			//
			// The 'special' bomb monitors the waypoint that created it and
			// explodes when the waypoint goes active.
			//

			break;
#ifdef	ANIM_PRIM
		case EWAY_DO_ACTIVATE_PRIM:

			if (ew->ed.arg1)
			{
				if (ew->ed.subtype == 0)
				{
#ifndef NDEBUG
#ifndef PSX
					sprintf(EWAY_message, "Waypoint %d has null anim-prim anim", ew->id);

					CONSOLE_text(EWAY_message, 8000);
#endif
#endif
					ew->ed.subtype = 1;
				}

				set_anim_prim_anim(ew->ed.arg1, ew->ed.subtype);
			}

			break;
#endif

		case EWAY_DO_WATER_SPOUT:

			//
			// This waypoint spouts out water while active. It is done inside EWAY_process()
			//

			break;

		case EWAY_DO_KILL_WAYPOINT:

			if (!(WITHIN(ew->ed.arg1, 1, EWAY_way_upto - 1)))
			{
#ifndef NDEBUG
#ifndef	PSX
				PANEL_new_text(NULL, 10000, "Waypoint %d (kill waypoint) refers to bad waypoint", ew->id);
#endif
#endif
			}
			else
			{
				EWAY_Way *ewk = &EWAY_way[ew->ed.arg1];

				ewk->flag |= EWAY_FLAG_DEAD;

				if (ewk->ec.type == EWAY_COND_COUNTDOWN_SEE)
				{
					music_mode_override=0;
				}

				if (ewk->ed.type == EWAY_DO_CREATE_ENEMY)
				{
					if (ewk->ed.arg1)
					{
						Thing *p_person = TO_THING(ewk->ed.arg1);

						ASSERT(p_person->Class == CLASS_PERSON);

						//
						// Make this person take a holiday in another dimension!
						//

						p_person->Genus.Person->pcom_ai   = PCOM_AI_NONE;
						p_person->Genus.Person->pcom_bent = PCOM_BENT_ROBOT;

						remove_thing_from_map(p_person);

						p_person->WorldPos.X = 0x8000;
						p_person->WorldPos.Z = 0x8000;

						p_person->WorldPos.Y = PAP_calc_map_height_at(0x80,0x80);

						set_person_idle(p_person);

						//
						// As if this person has never been created.
						// 

						ewk->ed.arg1 = NULL;
					}
				}
				else
				if (ewk->ed.type == EWAY_DO_CREATE_VEHICLE)
				{
					if (ewk->ed.arg1)
					{
						Thing *p_vehicle = TO_THING(ewk->ed.arg1);

						if (p_vehicle->Class == CLASS_VEHICLE)
						{
							//
							// Make the vehicle blow up!
							// 

							extern void VEH_reduce_health(Thing *p_car, Thing *p_person, SLONG damage);

							VEH_reduce_health(
								p_vehicle,
								NULL,
								1050);
extern	UBYTE	hit_player;
							hit_player=1;
							create_shockwave(
								p_vehicle->WorldPos.X >> 8,
								p_vehicle->WorldPos.Y >> 8,
								p_vehicle->WorldPos.Z >> 8,
								0x300,
								350,
								NULL,1);
							hit_player=0;



							//
							// As if this vehicle has never been created.
							// 

							ewk->ed.arg1 = NULL;

						}
					}
				}
				else
				if (ewk->ed.type == EWAY_DO_NAV_BEACON)
				{
					if (ewk->ed.subtype)
					{
						MAP_beacon_remove(ewk->ed.subtype);

						ewk->ed.subtype = NULL;
					}
				}
				else
				if (ewk->ed.type == EWAY_DO_ELECTRIFY_FENCE)
				{
					//
					// Turn off the electric fence.
					//

					if (ew->ed.arg1)
					{
						set_electric_fence_state(ew->ed.arg1, FALSE);
					}
				}
				else
				if (ewk->ed.type == EWAY_DO_MAKE_PERSON_PEE)
				{
					SLONG person = EWAY_get_person(ewk->ed.arg1);

					if (person)
					{
						Thing *p_person = TO_THING(person);

						ASSERT(p_person->Class == CLASS_PERSON);

						p_person->Genus.Person->Flags &= ~FLAG_PERSON_PEEING;
					}
				}
				else
				if (ewk->ec.type == EWAY_COND_TRIPWIRE)
				{
					if (ewk->ec.arg1)
					{
						TRIP_deactivate(ewk->ec.arg1);
					}
				}
				else
				if (ewk->ed.type == EWAY_DO_CREATE_BARREL)
				{
					//
					// Make the barrel dissapear!
					//

					if (ewk->ed.arg1)
					{
						Thing *p_barrel = TO_THING(ewk->ed.arg1);

						if (p_barrel->Class == CLASS_BARREL)
						{
							void BARREL_dissapear(Thing *p_barrel);

							BARREL_dissapear(p_barrel);
						}

						ewk->ed.arg1 = 0;
					}
				}
			}

			break;

		case EWAY_DO_OBJECTIVE:

			{
				UWORD mess    = ew->ed.arg1;
				SLONG percent = ew->ed.arg2 * CRIME_RATE_SCORE_MUL >> 8;

				CBYTE *str;

				if (!WITHIN(mess, 0, EWAY_MAX_MESSES - 1))
				{
					str = "Too many messages for the waypoint system! Tell Mark!";
				}
				else
				{
					if (EWAY_mess[mess] == NULL)
					{
						str = "No objective message";
					}
					else
					{
						str = EWAY_mess[mess];
					}
				}

				CRIME_RATE -= percent;

				SATURATE(CRIME_RATE, 0, 100);

				//
				// If its a Roper mission then the crime rate isn't reduced.
				//
#ifndef	FINAL
				//PANEL_new_text(NULL, 4000, "%s Crime rate reduced by %d%%.", str, percent);
#endif
			}

			break;

		case EWAY_DO_GROUP_LIFE:

			//
			// Make all waypoints in our colour/group active
			//

			{
				SLONG i;

				for (i = 1; i < EWAY_way_upto; i++)
				{
//					global_write2=i;
					if (EWAY_way[i].group  == ew->group &&
						EWAY_way[i].colour == ew->colour)
					{
						if (EWAY_way[i].ed.type == EWAY_DO_GROUP_LIFE ||
							EWAY_way[i].ed.type == EWAY_DO_GROUP_DEATH)
						{
							//
							// These waypoints are immune.
							//
						}
						else
						{
							EWAY_way[i].flag &= ~EWAY_FLAG_DEAD;
						}
					}
				}
			}

			break;

		case EWAY_DO_GROUP_DEATH:

			//
			// Make all waypoints in our colour/group dead
			//

			{
				SLONG i;

				for (i = 1; i < EWAY_way_upto; i++)
				{
//					global_write3=i;
					if (EWAY_way[i].group  == ew->group &&
						EWAY_way[i].colour == ew->colour)
					{
						if (EWAY_way[i].ed.type == EWAY_DO_GROUP_LIFE ||
							EWAY_way[i].ed.type == EWAY_DO_GROUP_DEATH)
						{
							//
							// These waypoints are immune.
							//
						}
						else
						{
							EWAY_way[i].flag |= EWAY_FLAG_DEAD;
						}
					}
				}
			}

			break;

		case EWAY_DO_CONVERSATION:
		case EWAY_DO_AMBIENT_CONV:

			{
				UWORD person_a = EWAY_get_person(ew->ed.arg1);
				UWORD person_b = EWAY_get_person(ew->ed.arg2);

				if (person_a &&
					person_b)
				{
					if (person_ok_for_conversation(TO_THING(person_a)) &&
						person_ok_for_conversation(TO_THING(person_b)))
					{
						if (EWAY_conv_active)
						{
							//
							// End the current converstaion.
							// 

							EWAY_finish_conversation();
						}

						//
						// Make sure the people aren't too close together.
						//

						{
							Thing *p_person_a = TO_THING(person_a);
							Thing *p_person_b = TO_THING(person_b);

							extern void push_people_apart(Thing *p_person, Thing *p_avoid);

							push_people_apart(
								p_person_a,
								p_person_b);
						}

						//
						// Start a converstaion.
						//

						EWAY_conv_active    = TRUE;
						EWAY_conv_waypoint  = ew - EWAY_way;
						EWAY_conv_person_a  = person_a;
						EWAY_conv_person_b  = person_b;
						EWAY_conv_timer     = 0;
						EWAY_conv_str_count = 0;
						EWAY_conv_skip      = 100;
						EWAY_conv_ambient   = (ew->ed.type == EWAY_DO_AMBIENT_CONV);

						if (!WITHIN(ew->ed.subtype, 0, EWAY_MAX_MESSES - 1))
						{
							CONSOLE_text("Too many messages for the waypoint system! Tell Mark!", 8000);
						}
						else
						{
							if (EWAY_mess[ew->ed.subtype] == NULL)
							{
								EWAY_conv_str = 0;
							}
							else
							{
								EWAY_conv_str = EWAY_mess[ew->ed.subtype] - EWAY_mess_buffer;
							}
						}
					}
				}
			}

			break;

		case EWAY_DO_INCREASE_COUNTER:
			
			ASSERT(WITHIN(ew->ed.subtype, 0, EWAY_MAX_COUNTERS - 1));

			EWAY_counter[ew->ed.subtype] += 1;

			break;

		case EWAY_DO_EMIT_STEAM:

			//
			// The code for doing this is in EWAY_process_emit_steam() and is
			// called _while_ the waypoint is active.
			//

			break;

		case EWAY_DO_TRANSFER_PLAYER:

			{
				THING_INDEX i_player = EWAY_get_person(ew->ed.arg1);

				if (i_player == NULL)
				{
#ifndef	FINAL
#ifndef	PSX
					//PANEL_new_text(NULL, 8000, "Waypoint %d: Transfer player didn't have person.", ew->id);
#endif
#endif
				}
				else
				{
					Thing *p_person = TO_THING(i_player);

					//
					// first, preserve roper/darci's stats in the_game for saving later
					//
/*
					if (NET_PERSON(0)->Genus.Person->PersonType==PERSON_DARCI) 
					{
						the_game.DarciConstitution=NET_PLAYER(0)->Genus.Player->Constitution;
						the_game.DarciStrength=NET_PLAYER(0)->Genus.Player->Strength;
						the_game.DarciStamina=NET_PLAYER(0)->Genus.Player->Stamina;
						the_game.DarciSkill=NET_PLAYER(0)->Genus.Player->Skill;
					}
					else
					{
						the_game.RoperConstitution=NET_PLAYER(0)->Genus.Player->Constitution;
						the_game.RoperStrength=NET_PLAYER(0)->Genus.Player->Strength;
						the_game.RoperStamina=NET_PLAYER(0)->Genus.Player->Stamina;
						the_game.RoperSkill=NET_PLAYER(0)->Genus.Player->Skill;
					}
*/

					//
					// Stop the old player being a player.
					//

					NET_PERSON(0)->Genus.Person->PlayerID  = 0;
					NET_PERSON(0)->Genus.Person->pcom_ai   = PCOM_AI_CIV;
					NET_PERSON(0)->Genus.Person->pcom_bent = PCOM_BENT_ROBOT | PCOM_BENT_FIGHT_BACK;
					NET_PERSON(0)->Genus.Person->Flags2   |= FLAG2_PERSON_INVULNERABLE;
					NET_PERSON(0)->Genus.Person->HomeX     = NET_PERSON(0)->WorldPos.X >> 8;
					NET_PERSON(0)->Genus.Person->HomeZ     = NET_PERSON(0)->WorldPos.Z >> 8;
					NET_PERSON(0)->Genus.Person->HomeYaw   = NET_PERSON(0)->Draw.Tweened->Angle >> 3;

					SET_SKILL(NET_PERSON(0),15);

					set_person_idle(NET_PERSON(0));

					//
					// Make the new player be a player.
					//

					NET_PERSON(0)                             =  p_person;
					p_person->Genus.Person->PlayerID          =  1;
					NET_PLAYER(0)->Genus.Player->PlayerPerson =  p_person;
					NET_PERSON(0)->Genus.Person->pcom_ai      =  PCOM_AI_NONE;
					NET_PERSON(0)->Genus.Person->Flags2      &= ~FLAG2_PERSON_INVULNERABLE;
					NET_PERSON(0)->Genus.Person->pcom_bent	 &= ~PCOM_BENT_PLAYERKILL;//PCOM_BENT_ROBOT | PCOM_BENT_FIGHT_BACK;

					SET_SKILL(NET_PERSON(0),0);

					FC_look_at(0, i_player);
					FC_setup_initial_camera(0);

					if (NET_PERSON(0)->State == STATE_MOVEING)
					{
						if (NET_PERSON(0)->SubState == SUB_STATE_SIMPLE_ANIM ||
							NET_PERSON(0)->SubState == SUB_STATE_SIMPLE_ANIM_OVER)
						{
							set_person_idle(NET_PERSON(0));
						}
					}
				}
			}
			break;

		case EWAY_DO_AUTOSAVE:
			{

				/*

				// -- PLACE AUTOSAVE CODE HERE --
#ifndef	PSX
extern	SLONG	SAVE_ingame(CBYTE *fname);
				SAVE_ingame("");
				CONSOLE_text("GAME AUTOSAVED");
#endif
				*/

			}
			break;

		case EWAY_DO_CREATE_BARREL:

			ew->ed.arg1 = BARREL_alloc(
							ew->ed.subtype,
							//(ew->ed.subtype == BARREL_TYPE_CONE) ? PRIM_OBJ_TRAFFIC_CONE : PRIM_OBJ_BARREL,
							ew->ed.arg2,
							ew->x,
							ew->z,
							ew - EWAY_way);

			break;

		case EWAY_DO_LOCK_VEHICLE:
			
			{
				Thing *p_vehicle;

				if (!WITHIN(ew->ed.arg1, 1, EWAY_way_upto - 1))
				{
#ifndef	FINAL
#ifndef	PSX
					//PANEL_new_text(NULL, 10000, "Lock vehicle waypoint %d points nowhere", ew->id);
#endif
#endif
				}
				else
				{
					EWAY_Way *ewv = &EWAY_way[ew->ed.arg1];

					if (ewv->ed.arg1)
					{
						Thing *p_vehicle = TO_THING(ewv->ed.arg1);

						if (p_vehicle->Class != CLASS_VEHICLE)
						{
#ifndef	FINAL
#ifndef	PSX
							//PANEL_new_text(NULL, 10000, "Lock vehicle waypoint %d trying to unlock non-vehicle", ew->id);
#endif
#endif
						}
						else
						{
							switch(ew->ed.subtype)
							{
								case EWAY_SUBTYPE_VEHICLE_LOCK:
									p_vehicle->Genus.Vehicle->key = SPECIAL_NUM_TYPES;	// Non-existent special.
									break;

								case EWAY_SUBTYPE_VEHICLE_UNLOCK:
									p_vehicle->Genus.Vehicle->key = SPECIAL_NONE;
									break;

								default:
									ASSERT(0);
									break;
							}
						}
					}
				}
			}

			break;

		case EWAY_DO_CUTSCENE:
#ifndef PSX
			PLAYCUTS_Play(PLAYCUTS_cutscenes+ew->ed.arg1);
#endif
			break;

		case EWAY_DO_GROUP_RESET:
	
			{
				SLONG i;

				EWAY_Way *ewr;

				for (i = 1; i < EWAY_way_upto; i++)
				{
//					global_write4=i;
					ewr = &EWAY_way[i];

					if (ewr->colour == ew->colour &&
						ewr->group  == ew->group)
					{
						if (ewr->ed.type == EWAY_DO_GROUP_RESET)
						{
							//
							// Immune!
							//
						}
						else
						{
							if (ewr->flag & EWAY_FLAG_ACTIVE)
							{
								void EWAY_set_inactive(EWAY_Way *ew);

								EWAY_set_inactive(ewr);
							}

							ewr->flag &= ~(EWAY_FLAG_ACTIVE|EWAY_FLAG_DEAD|EWAY_FLAG_FINISHED);
						}
					}
				}
			}

			break;

		case EWAY_DO_VISIBLE_COUNT_UP:

			//
			// The code for doing this is in EWAY_process() and is
			// called _while_ the waypoint is active.
			//

			EWAY_count_up               = 0;
			EWAY_count_up_add_penalties = FALSE;
			EWAY_count_up_num_penalties = 0;
			EWAY_count_up_penalty_timer = 0;
			EWAY_counter[3]             = 0;

			break;

		case EWAY_DO_RESET_COUNTER:

			if (ew->ed.subtype == 0)
			{
				UBYTE i;

				for (i = 0; i < EWAY_MAX_COUNTERS; i++)
				{
					EWAY_counter[i] = 0;
				}
			}
			else
			{
				ASSERT(WITHIN(ew->ed.subtype, 0, EWAY_MAX_COUNTERS - 1));

				EWAY_counter[ew->ed.subtype] = 0;
			}

			break;

		case EWAY_DO_CREATE_MIST:
#ifndef PSX
			{
				static SLONG last_detail = 17;
				static SLONG last_height = 84;

				#define MIST_SIZE 0x800

				SLONG x1 = (ew->x) - MIST_SIZE;
				SLONG z1 = (ew->z) - MIST_SIZE;
				SLONG x2 = (ew->x) + MIST_SIZE;
				SLONG z2 = (ew->z) + MIST_SIZE;

				MIST_create(
					last_detail,
					last_height,
					x1, z1,
					x2, z2);

				last_height += (last_height & 0x1) ? -11 : +11;
			}
#endif
			break;

		case EWAY_DO_STALL_CAR:

			{
				UWORD veh = EWAY_get_person(ew->ed.arg1);

				if (veh)
				{
					Thing *p_vehicle = TO_THING(veh);

					ASSERT(p_vehicle->Class == CLASS_VEHICLE);

					p_vehicle->Genus.Vehicle->Flags |= FLAG_VEH_STALLED;
				}
			}

			break;

		case EWAY_DO_EXTEND_COUNTDOWN:

			if (WITHIN(ew->ed.arg1, 1, EWAY_way_upto - 1))
			{
				EWAY_Way *ew_other = &EWAY_way[ew->ed.arg1];

				if (ew_other->ec.type == EWAY_COND_COUNTDOWN_SEE ||
					ew_other->ec.type == EWAY_COND_COUNTDOWN)
				{
					ew_other->ec.arg2 += ew->ed.arg2 * 100;
				}
				#ifndef NDEBUG
				else
				{
					PANEL_new_text(NULL, 4000, "Extend time (waypoint %d) points to bad waypoint.", ew->id);
				}
				#endif
			}

			break;

		case EWAY_DO_MOVE_THING:

			{
				UWORD i_thing = EWAY_get_person(ew->ed.arg1);

				if (i_thing)
				{
					Thing *p_thing = TO_THING(i_thing);

					if (p_thing->State == STATE_DEAD)
					{
						//
						// Ignore...
						//
					}
					else
					{
						GameCoord newpos;

						newpos.X = ew->x << 8;
						newpos.Y = ew->y << 8;
						newpos.Z = ew->z << 8;

						if (p_thing->Flags & FLAGS_ON_MAPWHO)
						{
							move_thing_on_map(p_thing, &newpos);
						}
						else
						{
							p_thing->WorldPos = newpos;
						}

						if (p_thing->Class == CLASS_PERSON)
						{	
							plant_feet(p_thing);
							p_thing->Draw.Tweened->Angle   = ew->yaw << 3;
							p_thing->Genus.Person->HomeX   = ew->x;
							p_thing->Genus.Person->HomeZ   = ew->z;
							p_thing->Genus.Person->HomeYaw = ew->yaw;
						}
					}
				}
			}

			break;

		case EWAY_DO_MAKE_PERSON_PEE:

			{
				SLONG person = EWAY_get_person(ew->ed.arg1);

				if (person)
				{
					Thing *p_person = TO_THING(person);

					ASSERT(p_person->Class == CLASS_PERSON);

					p_person->Genus.Person->Flags |= FLAG_PERSON_PEEING;
				}
			}

			break;

		case EWAY_DO_CONE_PENALTIES:
			GAME_FLAGS |= GF_CONE_PENALTIES;
			break;

		case EWAY_DO_SIGN:
			PANEL_flash_sign(ew->ed.arg1, ew->ed.arg2);
			break;

		case EWAY_DO_WAREFX:
			{
				SLONG ware=WARE_which_contains(ew->x>>8,ew->z>>8);
				if (ware)
					WARE_ware[ware].ambience=ew->ed.arg1;
				else
					TRACE("MISSION ERROR -- Warehouse Ambience waypoint is not inside a warehouse\n");
			}
			break;
/*
		case EWAY_DO_WAREFX:
			{
				SLONG ware=WARE_which_contains(ew->x>>8,ew->z>>8);
				if (ware)
					WARE_ware[ware].ambience=ew->ed.arg1;
				else
					TRACE("MISSION ERROR -- Warehouse Ambience waypoint is not inside a warehouse\n");
			}
			break;
*/

		case EWAY_DO_NO_FLOOR:
			GAME_FLAGS |= GF_NO_FLOOR;
			break;

		case EWAY_DO_SHAKE_CAMERA:
			break;

		case EWAY_DO_END_OF_WORLD:
			PYRO_create(NET_PERSON(0)->WorldPos, PYRO_GAMEOVER);
			break;

		default:
			ASSERT(0);
			break;
	}

	switch(ew->es.type)
	{
		case EWAY_STAY_TIME:

			//
			// Start the countdown to going inactive.
			// 

			{
				UBYTE i;

				for (i = 0; i < EWAY_MAX_TIMERS; i++)
				{
					if (EWAY_timer[i] == 0)
					{
						EWAY_timer[i] = ew->es.arg * 10;	// ew->es.arg is in 10th of a second.
						ew->timer     = i;
						ew->flag     |= EWAY_FLAG_COUNTDOWN;

						break;
					}
				}
				if(i>=EWAY_MAX_TIMERS)
				{
					ASSERT(0);
				}
			}

			break;

		case EWAY_STAY_ALWAYS:
		
			//
			// Never need to process this waypoint anymore.
			//

			ew->flag |= EWAY_FLAG_DEAD;

			break;

		case EWAY_STAY_DIE:

			//
			// Never need to process this waypoint anymore.
			//

			ew->flag |=  EWAY_FLAG_DEAD;
			ew->flag &= ~EWAY_FLAG_ACTIVE;

			break;

		default:
			break;
	}

ANNOYINGSCRIBBLECHECK;


}

//
// Does what has to be done when a waypoint goes inactive.
//

void EWAY_set_inactive(EWAY_Way *ew)
{
ANNOYINGSCRIBBLECHECK;

	ew->flag &= ~EWAY_FLAG_ACTIVE;

	if (ew->ed.type == EWAY_DO_CONTROL_DOOR)
	{
		//
		// Close a door!
		//

		DOOR_shut(
			ew->x,
			ew->z);
	}

	if (ew->ed.type == EWAY_DO_ELECTRIFY_FENCE)
	{
		//
		// Turn off the electric fence.
		//

		if (ew->ed.arg1)
		{
			set_electric_fence_state(ew->ed.arg1, FALSE);
		}
	}

	/*

	if (ew->ed.type == EWAY_DO_VISIBLE_COUNT_UP)
	{
		//
		// We have to add on all the penalty waypoints visibly on-screen.
		//

		EWAY_count_up_add_penalties = TRUE;
		EWAY_count_up_penalty_timer = 0;

		//
		// Work out how long the count up took and put it into counter 3.
		//

		SLONG secs = (EWAY_count_up + (EWAY_count_up_num_penalties * 500)) / 1000;

		if (secs > 255)
		{
			secs = 255;
		}

		EWAY_counter[3] = secs;
	}

	*/

	if (ew->ed.type == EWAY_DO_NAV_BEACON)
	{
		MAP_beacon_remove(ew->ed.subtype);
	}

ANNOYINGSCRIBBLECHECK;


}


SLONG EWAY_stop_player_moving(void)
{
	return (EWAY_cam_active && EWAY_cam_freeze)||GAME_cut_scene;
}

//
// Showing the player the number of penalties incurred!
//

void EWAY_process_penalties()
{
	if (!EWAY_count_up_add_penalties)
	{
		return;
	}

	EWAY_count_up_penalty_timer += EWAY_tick;

	//
	// Show the time achieved.
	// 

	PANEL_draw_timer(EWAY_count_up / 10, 320, 50);

	if (EWAY_count_up_penalty_timer > 100)
	{
		//
		// Start showing all the penalty points.
		//

		CBYTE str[64];

		if (EWAY_count_up_num_penalties == 0)
		{
			strcpy(str,XLAT_str(X_NO_CONES_HIT));
		}
		else
		{
			sprintf(str, "%s = %d", XLAT_str(X_NUM_CONES_HIT),(EWAY_count_up_num_penalties > 0) ? EWAY_count_up_num_penalties : 0);
		}

		FONT2D_DrawStringCentred(
			str,
			320,
			100,
			0x00ff00);

		if (EWAY_count_up_penalty_timer >= 150)
		{
			if (EWAY_count_up_num_penalties == 0)
			{
				if (EWAY_count_up_penalty_timer >= 400)
				{
					EWAY_count_up_add_penalties = FALSE;
				}
			}
			else
			if (EWAY_count_up_num_penalties == -1)	// -1 => We've finished add up penalities...
			{
				if (EWAY_count_up_penalty_timer >= 400)
				{
					EWAY_count_up_add_penalties = FALSE;
				}
			}
			else
			{
				//
				// Add the penalties up. 3 per second.
				//

				while(EWAY_count_up_penalty_timer >= 200)
				{
					EWAY_count_up_penalty_timer -= 10;
					EWAY_count_up_num_penalties -= 1;
					EWAY_count_up               += 500;	// 500 milliseconds per penalty.

					if (EWAY_count_up_num_penalties == 0)
					{
						EWAY_count_up_num_penalties = -1;	// -1 => Finished adding up the penalties.
					}
				}
			}
		}
	}
}

#ifndef	PSX
#ifndef TARGET_DC
extern	SWORD	people_types[50];

void	count_people_types(void)
{		 
	return;

	EWAY_Way *ew;
	SLONG	i;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
		if(ew->ed.type == EWAY_DO_CREATE_ENEMY)
		{
			people_types[ew->ed.subtype]++;
		}
	}

}
#endif
#endif

void EWAY_process()
{
	SLONG i;
	SLONG on;
	SLONG timer;

ANNOYINGSCRIBBLECHECK;


	EWAY_Way *ew;
	SLONG	offset,step;//,start,end;
	SLONG	eway_max=EWAY_way_upto;


#ifdef PSX	

	if(GAME_TURN==0)
	{
		step=1;
		offset=0;
//		start=1;
//		end=eway_max;
	}
	else
	{
		step=2;
		offset=GAME_TURN&1;
/*
		if(GAME_TURN&1)
		{
			start=1;
			end=eway_max>>1;
		}
		else
		{
			start=eway_max>>1;
			end=eway_max;
		}
*/
	}
#else
		step=1;
		offset=0;

//		start=1;
//		end=eway_max;
#endif

//		step=1;
//		offset=0;
//	step=2;
//	offset=GAME_TURN&1;

	//
	// This will be set to TRUE if we draw a the count-up timer.
	//

	EWAY_count_up_visible = FALSE;

	//
	// Update the time.
	//

	EWAY_time_accurate += 80 * TICK_RATIO >> TICK_SHIFT;
	EWAY_time           = EWAY_time_accurate >> 4;
	EWAY_tick			= 5 * TICK_RATIO >> TICK_SHIFT;


ANNOYINGSCRIBBLECHECK;


	if (GAME_STATE & (GS_LEVEL_LOST | GS_LEVEL_WON))
	{
		//
		// Don't process the waypoint after the game is lost or won.
		//
	}
	else
	{
		//
		// Process the waypoints.
		//

		for (i = 1+(offset); i < eway_max; i+=step)
		{

ANNOYINGSCRIBBLECHECK;

			ew = &EWAY_way[i];

			if (ew->flag & EWAY_FLAG_DEAD)
			{
				continue;
			}

			if (ew->flag & EWAY_FLAG_ACTIVE)
			{
				if (ew->ed.type == EWAY_DO_EMIT_STEAM)
				{
					//
					// This is a waypoint type that does stuff _while_ it
					// is active not just when it goes active or inactive.
					//

					EWAY_process_emit_steam(ew);
				}
				else
				if (ew->ed.type == EWAY_DO_SHAKE_CAMERA)
				{
					if (FC_cam[0].shake < 100)
					{
						#if PSX
						FC_cam[0].shake += 6;
						#else
						FC_cam[0].shake += 3;
						#endif
					}
				}
				else
				if (ew->ed.type == EWAY_DO_VISIBLE_COUNT_UP)
				{
					//
					// Increase the timer.
					//

					EWAY_count_up += (50 * TICK_RATIO >> TICK_SHIFT) * step;	// 50 * 20 ticks per second. i.e. 1000

					//
					// Print the time.
					//

					PANEL_draw_timer(EWAY_count_up / 10, 320, 50);

					EWAY_count_up_visible = TRUE;

					{
						SLONG secs = EWAY_count_up / 1000;

						if (secs > 255)
						{
							secs = 255;
						}

						EWAY_counter[3] = secs;
					}
				}

				if (ew->flag & EWAY_FLAG_COUNTDOWN)
				{
					//
					// The countdown to going inactive.
					//

					timer  = EWAY_timer[ew->timer];
					timer -= EWAY_tick * step;

					if (timer <= 0)
					{
						timer = 0;

						EWAY_set_inactive(ew);
					}

					EWAY_timer[ew->timer] = timer;
				}
				else
				{
					//
					// Do we have to check for going inactive?
					//

					switch(ew->es.type)
					{
						case EWAY_STAY_ALWAYS:

							ew->flag |= EWAY_FLAG_DEAD;

							break;

						case EWAY_STAY_WHILE:

							on = EWAY_evaluate_condition(ew, &ew->ec);

							if (!on)
							{
								EWAY_set_inactive(ew);
							}

							break;

						case EWAY_STAY_WHILE_TIME:

							on = EWAY_evaluate_condition(ew, &ew->ec);

							if (!on)
							{
								//
								// The countdown to going inactive.
								// 

								ew->timer = ew->es.arg;
								ew->flag |= EWAY_FLAG_COUNTDOWN;
							}

							break;

						//
						// There is no case for EWAY_STAY_TIME because ew->timer
						// should always be set in this case.
						// 

						default:
							ASSERT(0);
							break;
					}
				}

				if (ew->ed.type == EWAY_DO_WATER_SPOUT)
				{
					//
					// While this waypoint is active, it spouts out water.
					//

					DIRT_new_water(ew->x + 2, ew->y, ew->z,     -1, 28,  0);
					DIRT_new_water(ew->x    , ew->y, ew->z + 2,  0, 29, -1);
					DIRT_new_water(ew->x    , ew->y, ew->z - 2,  0, 28, +1);
					DIRT_new_water(ew->x - 2, ew->y, ew->z,     +1, 29,  0);
				}
			}
			else
			{
				//
				// Check for going active.
				//

				on = EWAY_evaluate_condition(ew, &ew->ec);

				if (on)
				{
					EWAY_set_active(ew);
				}


			}
		}
	}

	//
	// Process any conversation that might be going on.
	//

ANNOYINGSCRIBBLECHECK;


	EWAY_process_conversation();

	//
	// Show the player the count-up timer penalties after the count_up timer has
	// gone inactive.
	//

ANNOYINGSCRIBBLECHECK;


	EWAY_process_penalties();

	//
	// Process the camera.
	//

ANNOYINGSCRIBBLECHECK;


	EWAY_process_camera();

ANNOYINGSCRIBBLECHECK;


}



void EWAY_get_position(
		SLONG  waypoint,
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z)
{
	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

   *world_x = EWAY_way[waypoint].x;
   *world_y = EWAY_way[waypoint].y;
   *world_z = EWAY_way[waypoint].z;
}

UWORD EWAY_get_angle(SLONG waypoint)
{
	UWORD ans;

	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	ans = EWAY_way[waypoint].yaw << 3;

	return ans;
}

UWORD EWAY_get_person(SLONG waypoint)
{
	UWORD ans;
	
	if (waypoint == NULL)
	{
		return NULL;
	}

	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	EWAY_Way *ew;

	ew = &EWAY_way[waypoint];

	if (ew->ed.type == EWAY_DO_CREATE_ENEMY   ||
		ew->ed.type == EWAY_DO_CREATE_PLAYER  ||
		ew->ed.type == EWAY_DO_CREATE_VEHICLE ||
		ew->ed.type == EWAY_DO_CREATE_ANIMAL)
	{
		ans = ew->ed.arg1;
	}
	else
	{
		ans = NULL;
	}

	return ans;
}



SLONG EWAY_find_waypoint(
		SLONG index,
		SLONG whatdo,	// or EWAY_DONT_CARE
		SLONG colour,	// or EWAY_DONT_CARE
		SLONG group,	// or EWAY_DONT_CARE
		UBYTE only_active)
{
	SLONG i;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
//		global_write6=i;
		if (index >= EWAY_way_upto)
		{
			index = 1;
		}

		ASSERT(WITHIN(index, 0, EWAY_way_upto - 1));

		ew = &EWAY_way[index];

		if ((colour == EWAY_DONT_CARE || colour == ew->colour) &&
			(group  == EWAY_DONT_CARE || group  == ew->group ) &&
			(whatdo == EWAY_DONT_CARE || whatdo == ew->ed.type))
		{
			//
			// Found a matching waypoint
			//

			if (!only_active || (ew->flag & EWAY_FLAG_ACTIVE))
			{
				return index;
			}
		}

		index += 1;
	}

	return EWAY_NO_MATCH;
}

SLONG EWAY_find_waypoint_rand(
		SLONG not_this_index,
		SLONG colour,
		SLONG group,
		UBYTE only_active)
{
	SLONG i;
	SLONG ans;

	EWAY_Way *ew;

	#define EWAY_FIND_RAND_MAX 32

	UWORD choice[EWAY_FIND_RAND_MAX];
	SLONG choice_upto = 0;

	//
	// Find all the suitable waypoints.
	//

	for (i = 1; i < EWAY_way_upto; i++)
	{
//		global_write7=i;
		if (i == not_this_index)
		{
			//
			// Ignore this one.
			//

			continue;
		}

		ew = &EWAY_way[i];

		if ((colour == EWAY_DONT_CARE || colour == ew->colour) &&
			(group  == EWAY_DONT_CARE || group  == ew->group))
		{
			ASSERT(WITHIN(choice_upto, 0, EWAY_FIND_RAND_MAX - 1));

			//
			// Found a matching waypoint
			//

			if (!only_active || (ew->flag & EWAY_FLAG_ACTIVE))
			{
				choice[choice_upto++] = i;

				if (choice_upto >= EWAY_FIND_RAND_MAX)
				{
					break;
				}
			}
		}
	}

	if (choice_upto == 0)
	{
		ans = EWAY_NO_MATCH;
	}
	else
	{
		ans = choice[Random() % choice_upto];
	}

	return ans;
}



SLONG EWAY_find_nearest_waypoint(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG colour,	// or EWAY_DONT_CARE
		SLONG group)	// or EWAY_DONT_CARE
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;

	SLONG best_dist     = INFINITY;
	SLONG best_waypoint = EWAY_NO_MATCH;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
//		global_write8=i;

		if ((colour == EWAY_DONT_CARE || colour == ew->colour) &&
			(group  == EWAY_DONT_CARE || group  == ew->group))
		{
			//
			// Found a matching waypoint
			//

			if (ew->flag & EWAY_FLAG_ACTIVE)
			{
				dx = abs(ew->x - x);
				dy = abs(ew->y - y);
				dz = abs(ew->z - z);

				dist = QDIST3(dx,dy,dz);

				if (dist < best_dist)
				{
					best_dist     = dist;
					best_waypoint = i;
				}
			}
		}
	}

	return best_waypoint;
}


SLONG EWAY_grab_camera(
		SLONG *cam_x,
		SLONG *cam_y,
		SLONG *cam_z,
		SLONG *cam_yaw,
		SLONG *cam_pitch,
		SLONG *cam_roll,
		SLONG *cam_lens)
{
	if (EWAY_cam_active)
	{
		//
		// If there is analogue control and the player is still moving
		// then we can't grab the camera because it'll confuse the
		// player control.
		//

		extern SLONG analogue;

#ifndef PSX
		if (analogue && !EWAY_stop_player_moving())
#else
		if (!EWAY_stop_player_moving())
#endif
		{
			return FALSE;
		}
		else
		{
			*cam_x     = EWAY_cam_x;
			*cam_y     = EWAY_cam_y;
			*cam_z     = EWAY_cam_z;
			*cam_yaw   = EWAY_cam_yaw;
			*cam_pitch = EWAY_cam_pitch;
			*cam_roll  = 1024 << 8;
			*cam_lens  = EWAY_cam_lens;
		}
	}

	return EWAY_cam_active;
}

UBYTE EWAY_camera_warehouse()
{
	if (EWAY_cam_active)
	{
		if (EWAY_conv_active)
		{
			if (EWAY_conv_person_a)
			{
				ASSERT(TO_THING(EWAY_conv_person_a)->Class == CLASS_PERSON);

				return TO_THING(EWAY_conv_person_a)->Genus.Person->Ware;
			}
		}
		else
		{
			return EWAY_cam_warehouse;
		}
	}

	return NULL;
}


void EWAY_item_pickedup(SLONG waypoint)
{
	EWAY_Way *ew;

	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	ew = &EWAY_way[waypoint];

	ASSERT(ew->ed.type == EWAY_DO_CREATE_ITEM);

	ew->flag |= EWAY_FLAG_GOTITEM;
}


SLONG EWAY_get_delay(SLONG waypoint, SLONG default_delay)
{
	EWAY_Way *ew;

	if (waypoint == 0)
	{
		return default_delay;
	}

	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	ew = &EWAY_way[waypoint];

	switch(ew->ed.type)
	{
		case EWAY_DO_NOTHING:
			return ew->ed.arg1 * 100;
			break;

		default:
			return default_delay;
			break;
	}
}

SLONG EWAY_is_active(SLONG waypoint)
{
	EWAY_Way *ew;

	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	ew = &EWAY_way[waypoint];

	if (ew->flag & EWAY_FLAG_ACTIVE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


SLONG EWAY_used_person(UWORD t_index)
{
	UWORD i;
	SLONG ans = FALSE;
	
	EWAY_Way *ew;

	//
	// Go through EVERY WAYPOINT looking for a waypoint with a condition
	// for person used.
	//

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
//		global_write9=i;

		if (ew->flag & EWAY_FLAG_DEAD)
		{
			continue;
		}

		if (ew->ec.type == EWAY_COND_PERSON_USED)
		{
			//
			// Is this waypoint looking at our person?
			//
			
			ASSERT(WITHIN(ew->ec.arg1, 1, EWAY_way_upto - 1));

			if (EWAY_way[ew->ec.arg1].ed.type == EWAY_DO_CREATE_ENEMY)
			{
				EWAY_Way *ew2 = &EWAY_way[ew->ec.arg1];

				if (ew2->ed.arg1 == t_index)
				{
					//
					// Cool! Set this waypoint active.
					//

					EWAY_used_thing = t_index;

					EWAY_set_active(ew);

					EWAY_used_thing = NULL;

					//
					// Don't mark this person as useable any more.
					//
					
					ASSERT(TO_THING(t_index)->Class == CLASS_PERSON);

					TO_THING(t_index)->Genus.Person->Flags &= ~FLAG_PERSON_USEABLE;

					if (ew->ed.type == EWAY_DO_MESSAGE)
					{
						ans = TRUE;
					}
				}
			}
		}
	}

	return ans;
}


UBYTE EWAY_get_warehouse(SLONG waypoint)
{
	UBYTE ans;

	ASSERT(WITHIN(waypoint, 1, EWAY_way_upto - 1));

	ans = EWAY_way[waypoint].ware;

	return ans;
}


#ifndef	PSX
#ifndef TARGET_DC
void EWAY_work_out_which_ones_are_in_warehouses()
{
	SLONG i;
	SLONG id;

	EWAY_Way  *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
//		global_write10=i;
		ew = &EWAY_way[i];

		//
		// Is this waypoint in a warehouse?
		//

		if (PAP_2HI(ew->x >> 8, ew->z >> 8).Flags & PAP_FLAG_HIDDEN)
		{
			SLONG ware_top;

			ware_top = PAP_calc_map_height_at(
						ew->x,
						ew->z);

			if (ew->y < ware_top - 0x80)
			{
				ew->ware = WARE_which_contains(
								ew->x >> 8,
								ew->z >> 8);
			}
		}
	}
}


#define EWAY_CAM_VIEW_ANGLES 16		// Power of 2 please

void EWAY_cam_get_position_for_angle(
			Thing *p_thing,
			SLONG  angle,
			SLONG *vx,
			SLONG *vy,
			SLONG *vz)
{
	SLONG dx;
	SLONG dz;

	ASSERT(WITHIN(angle, 0, EWAY_CAM_VIEW_ANGLES - 1));

	angle  = angle * (2048 / EWAY_CAM_VIEW_ANGLES) + 100;
	angle &= 2047;

	dx = SIN(angle);
	dz = COS(angle);

   *vx = p_thing->WorldPos.X >> 8;
   *vy = p_thing->WorldPos.Y >> 8;
   *vz = p_thing->WorldPos.Z >> 8;

   *vx += dx >> 7;
   *vz += dz >> 7;
   *vy += 0x140;
}
#endif
#endif

void EWAY_cam_converse(Thing *p_thing, Thing *p_listener)
{
	SLONG i;
	SLONG j;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;

	SLONG cam_dist;

	SLONG best_angle;
	SLONG best_score;
	
	SLONG side1;
	SLONG side2;

	SLONG angle1;
	SLONG angle2;
	SLONG dangle;

	#define EWAY_CONVERSE_ANGLES 16	// Power of 2 please.

	UBYTE score[EWAY_CONVERSE_ANGLES];

	EWAY_cam_thing      = THING_NUMBER(p_thing);
	EWAY_cam_waypoint   = NULL;
	EWAY_cam_lens       = 0x28000;
	EWAY_cam_freeze     = TRUE;
	EWAY_cam_lock       = FALSE;
	EWAY_cam_goinactive = FALSE;

	//
	// If the current camera can see the listener and the thing, then
	// don't make it move needlessly.
	//

	if (EWAY_cam_active)
	{
		x = EWAY_cam_x;
		y = EWAY_cam_y;
		z = EWAY_cam_z;
	}
	else
	{
		x = FC_cam[0].x;
		y = FC_cam[0].y;
		z = FC_cam[0].z;
	}

	dx = abs(p_thing->WorldPos.X - x);
	dy = abs(p_thing->WorldPos.Y - y);
	dz = abs(p_thing->WorldPos.Z - z);

	dist = QDIST3(dx,dy,dz) >> 8;

	if (WITHIN(dist, 0x180, 0x700) && 0)
	{
		dx = abs(p_listener->WorldPos.X - x);
		dy = abs(p_listener->WorldPos.Y - y);
		dz = abs(p_listener->WorldPos.Z - z);

		dist = QDIST3(dx,dy,dz) >> 8;
	
		if (WITHIN(dist, 0x200, 0x700))
		{
			x >>= 8;
			y >>= 8;
			z >>= 8;

			if (there_is_a_los(
					x, y, z,
					p_thing->WorldPos.X          >> 8,
					p_thing->WorldPos.Y + 0x6000 >> 8,
					p_thing->WorldPos.Z          >> 8,
					LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
			{
				if (there_is_a_los(
						x, y, z,
						p_listener->WorldPos.X          >> 8,
						p_listener->WorldPos.Y + 0x6000 >> 8,
						p_listener->WorldPos.Z          >> 8,
						LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
				{
					EWAY_cam_x = x << 8;
					EWAY_cam_y = y << 8;
					EWAY_cam_z = z << 8;

					EWAY_cam_active = TRUE;

					return;
				}
			}
		}
	}

	//
	// How far the camera should be from the talking thing.
	//
	
	dx = p_listener->WorldPos.X - p_thing->WorldPos.X >> 8;
	dz = p_listener->WorldPos.Z - p_thing->WorldPos.Z >> 8;

	cam_dist = QDIST2(abs(dx),abs(dz));

	//
	// Use the distance between the two people... but not too extreme.
	//
			 
	SATURATE(cam_dist, 0x180, 0x300);

	for (i = 0; i < EWAY_CONVERSE_ANGLES; i++)
	{
		score[i] = 0;

		dx = SIN(i * (2048 / EWAY_CONVERSE_ANGLES)) * cam_dist >> 16;
		dz = COS(i * (2048 / EWAY_CONVERSE_ANGLES)) * cam_dist >> 16;

		x = (p_thing->WorldPos.X >> 8) + dx;
		y = (p_thing->WorldPos.Y >> 8) + 0x100;
		z = (p_thing->WorldPos.Z >> 8) + dz;

		if (!there_is_a_los(
				x, y, z,
				p_thing->WorldPos.X          >> 8,
				p_thing->WorldPos.Y + 0x6000 >> 8,
				p_thing->WorldPos.Z          >> 8,
				LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG |
				LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
		{
			//
			// The camera can't see the person who is talking.
			//

			continue;
		}

		score[i] += 40;

		if (!there_is_a_los(
				x, y, z,
				p_thing->WorldPos.X          >> 8,
				p_thing->WorldPos.Y + 0x6000 >> 8,
				p_thing->WorldPos.Z          >> 8,
				LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG |
				LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
		{
			//
			// The camera can see the person who is listening. That
			// is a good thing.
			//

			score[i] += 25;
		}

		//
		// Is the listener obscuring the person who is talking.
		//

		dx = (p_thing->WorldPos.X >> 8) - x;
		dz = (p_thing->WorldPos.Z >> 8) - z;

		angle1 = calc_angle(dx,dz);

		dx = (p_listener->WorldPos.X >> 8) - x;
		dz = (p_listener->WorldPos.Z >> 8) - z;

		angle2 = calc_angle(dx,dz);

		dangle = angle2 - angle1;

		if (dangle < -1024) {dangle += 2048;}
		if (dangle > +1024) {dangle -= 2048;}

		if (abs(dangle) > 100)
		{
			//
			// The two people aren't obscuring eachother. That is
			// a good thing.
			//

			score[i] += 15;
		}

		//
		// Is the camera looking at the front of the listener?
		//

		dangle = angle1 - p_thing->Draw.Tweened->Angle;

		if (dangle < -1024) {dangle += 2048;}
		if (dangle > +1024) {dangle -= 2048;}

		if (abs(dangle) < 400)
		{
			//
			// We can see the speakers face. Another good thing.
			//

			score[i] += 10;
		}

		//
		// Avoid prims.
		//

		dx = (p_thing->WorldPos.X >> 8) - x >> 3;
		dy = (p_thing->WorldPos.Y >> 8) - y >> 3;
		dz = (p_thing->WorldPos.Z >> 8) - z >> 3;

		for (j = 0; j < 8; j++)
		{
			if (OB_inside_prim(x,y,z))
			{
				score[i] -= 5;
			}

			x += dx;
			y += dy;
			z += dz;
		}
	}

	//
	// Stop the camera being too near a wall.
	//

	for (i = 0; i < EWAY_CONVERSE_ANGLES; i++)
	{
		side1 = (i - 1) & (EWAY_CONVERSE_ANGLES - 1);
		side2 = (i + 1) & (EWAY_CONVERSE_ANGLES - 1);

		if (side1 == 0 ||
			side2 == 0)
		{
			score[i] >>= 1;
		}
	}

	best_angle = -1;
	best_score =  0;

	for (i = 0; i < EWAY_CONVERSE_ANGLES; i++)
	{
		if (score[i] > best_score)
		{
			best_score = score[i];
			best_angle = i;
		}
	}

	if (best_score == 0)
	{
		//
		// Emergency!
		//

		EWAY_cam_x = p_thing->WorldPos.X + p_listener->WorldPos.X >> 1;
		EWAY_cam_y = p_thing->WorldPos.Y + p_listener->WorldPos.Y >> 1;
		EWAY_cam_z = p_thing->WorldPos.Z + p_listener->WorldPos.Z >> 1;

		EWAY_cam_y += 0x30000;
	}
	else
	{
		SLONG	division = 2048 / EWAY_CONVERSE_ANGLES;
		SLONG	angle = best_angle * division;

		// modulate it randomly
		angle += (Random() & (division - 1)) - (division / 2);
		angle &= 2047;

		dx = SIN(angle) * cam_dist >> 8;
		dz = COS(angle) * cam_dist >> 8;

		// find a vector 0 to 50% from speaker to listener
		SLONG	rnd = Random() & 127;

		x = ((p_listener->WorldPos.X >> 8) - (p_thing->WorldPos.X >> 8)) * rnd;
		y = ((p_listener->WorldPos.Y >> 8) - (p_thing->WorldPos.Y >> 8)) * rnd;
		z = ((p_listener->WorldPos.Z >> 8) - (p_thing->WorldPos.Z >> 8)) * rnd;

		// add in the origin coordinates
		x += p_thing->WorldPos.X;
		y += p_thing->WorldPos.Y;
		z += p_thing->WorldPos.Z;

		EWAY_cam_targx = x;
		EWAY_cam_targy = y + 0xA000;
		EWAY_cam_targz = z;

		// add the vector to the camera
		EWAY_cam_x = x + dx;
		EWAY_cam_y = y + 0x10000;
		EWAY_cam_z = z + dz;

		EWAY_cam_target = 0;
		EWAY_cam_thing = 0;
	}

	EWAY_cam_active = TRUE;
}




#ifdef MATTS_BUGGY_VERSION

void EWAY_cam_converse(Thing *p_thing, Thing *p_listener, UBYTE cam_flags) {
	SLONG thing_yaw, yaw;
	SLONG dx,dy,dz;
	SLONG cx,cy,cz;
	GameCoord targ;
	static UBYTE lineside=0;
	static UWORD tick = 0;
	static Thing *p_last1=0;
	static Thing *p_last2=0;
	static UBYTE lastflags=0;

	// debug force:
	//cam_flags=32|3|16;
	cam_flags=1;

	if ((p_thing==0)&&(p_listener==0)) {
		p_thing=p_last1;
		p_listener=p_last2;
		cam_flags=lastflags;
	} else {
		if ((p_thing!=p_last1)||(p_listener!=p_last2)||(cam_flags!=lastflags)) { // new scene


			
			if (!(lastflags&16)) lineside^=1;
			p_last1=p_thing;
			p_last2=p_listener;
			lastflags=cam_flags;
			tick=0;
		}
	}

	// get round to making some #defs soon
	// for the moment, bits 0-2 (values 0-7) indicate which shot to use
	// bit 3 (8) is zoom flag
	// bit 4 (16) is cross-line flag
	// bit 5 (32) is drift flag
	// bit 6 (64) isn't used yet
	// bit 7 (128) inverts the meaning of movement options


	//
	// What direction is the thing facing in?
	//

	dy=140<<8;

	switch(p_thing->Class)
	{
		case CLASS_PERSON:
			thing_yaw = p_thing->Draw.Tweened->Angle;
			break;

		case CLASS_VEHICLE:
			thing_yaw = p_thing->Genus.Vehicle->Angle;
			break;

		default:
			ASSERT(0);
			break;
	}

	EWAY_cam_thing    = 0;
	EWAY_cam_waypoint = NULL;
	EWAY_cam_target   = NULL;

	switch (cam_flags&7) {
	case 0: // 1-shot closeup
		dx=-SIN(thing_yaw&2047);
		dz=-COS(thing_yaw&2047);
		targ=p_thing->WorldPos;
		EWAY_cam_targx	  = p_thing->WorldPos.X;
		EWAY_cam_targy	  = p_thing->WorldPos.Y+(0x80<<8);
		EWAY_cam_targz	  = p_thing->WorldPos.Z;

		break;
	case 1: // 1-shot medium
		yaw=thing_yaw;
		if (lineside) yaw+=260; else yaw-=260;
		dx=-SIN(yaw&2047); dx+=(dx>>1);
		dz=-COS(yaw&2047); dz+=(dz>>1);
		targ=p_thing->WorldPos;
		EWAY_cam_targx	  = p_thing->WorldPos.X;
		EWAY_cam_targy	  = p_thing->WorldPos.Y+(0x60<<8);
		EWAY_cam_targz	  = p_thing->WorldPos.Z;

		break;
	case 2: // 2-shot medium (actually, it's a long shot right now)
		yaw=thing_yaw;
		if (lineside) yaw+=512; else yaw-=512;
		dx=-SIN(yaw&2047)<<2;
		dz=-COS(yaw&2047)<<2;
		EWAY_cam_targx	  = targ.X = (p_thing->WorldPos.X+p_listener->WorldPos.X)>>1;
		EWAY_cam_targy	  = targ.Y = ((p_thing->WorldPos.Y+p_listener->WorldPos.Y)>>1)+(0x80<<8);
		EWAY_cam_targz	  = targ.Z = (p_thing->WorldPos.Z+p_listener->WorldPos.Z)>>1;
		break;
	case 3: // 2-shot shoulder
/*		EWAY_cam_targx	  = (p_thing->WorldPos.X+p_listener->WorldPos.X)>>1;
		EWAY_cam_targy	  = ((p_thing->WorldPos.Y+p_listener->WorldPos.Y)>>1)+(0x80<<8);
		EWAY_cam_targz	  = (p_thing->WorldPos.Z+p_listener->WorldPos.Z)>>1;*/
		yaw=thing_yaw; // heh.
		dy=160<<8;
		targ=p_listener->WorldPos;
		if (lineside) yaw-=200; else yaw+=200;
		dx=-SIN(yaw&2047);
		dz=-COS(yaw&2047);
		EWAY_cam_targx	  = p_thing->WorldPos.X+dx;
		EWAY_cam_targy	  = p_thing->WorldPos.Y+dy;
		EWAY_cam_targz	  = p_thing->WorldPos.Z+dz;
		yaw=thing_yaw;
		if (cam_flags&16) {
//			yaw=(SIN(tick)>>7)-256;
			yaw=(COS(tick<<2)>>8);
			if (lineside) yaw=-yaw;
			yaw+=thing_yaw;
		} else {
		  if (lineside) yaw+=200; else yaw-=200;
		}
		dx=(SIN(yaw&2047)>>2)-SIN(yaw&2047);
		dz=(COS(yaw&2047)>>2)-COS(yaw&2047);
		break;
	}

	EWAY_cam_x = targ.X + dx;
	EWAY_cam_y = targ.Y + dy;
	EWAY_cam_z = targ.Z + dz;
	EWAY_cam_active   = TRUE;
	EWAY_cam_freeze   = (cam_flags&32) ? 0 : 1;

	FC_move_to(0,EWAY_cam_x>>8,EWAY_cam_y>>8,EWAY_cam_z>>8);
	if (tick<512) tick+=4;

}

#endif
#ifndef	PSX
#ifndef TARGET_DC
void EWAY_cam_look_at(Thing *p_thing)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG cx;
	SLONG cz;

	SLONG dprod;

	SLONG vx;
	SLONG vy;
	SLONG vz;
	SLONG score;

	SLONG best_angle;
	SLONG best_score;

	UWORD thing_yaw;
	UWORD angle;

	SLONG view[EWAY_CAM_VIEW_ANGLES];

	//
	// What direction is the thing facing in?
	//

	switch(p_thing->Class)
	{
		case CLASS_PERSON:
			thing_yaw = p_thing->Draw.Tweened->Angle;
			break;

		case CLASS_VEHICLE:
			thing_yaw = p_thing->Genus.Vehicle->Angle;
			break;

		default:
			ASSERT(0);
			break;
	}
	
	dx = -SIN(thing_yaw);
	dz = -COS(thing_yaw);

	//
	// Which angles could we view the thing from?
	//

	for (i = 0; i < EWAY_CAM_VIEW_ANGLES; i++)
	{
		EWAY_cam_get_position_for_angle(
			p_thing,
			i,
		   &vx,
		   &vy,
		   &vz);

		if (there_is_a_los(
				vx,	vy, vz,
				(p_thing->WorldPos.X >> 8),
				(p_thing->WorldPos.Y >> 8) + 0x80,
				(p_thing->WorldPos.Z >> 8),
				0))
		{
			cx = vx - (p_thing->WorldPos.X >> 8);
			cz = vz - (p_thing->WorldPos.Z >> 8);

			dprod = dx*cx + dz*cz;
			
			if (dprod < 1000)
			{
				dprod = 1000;
			}

			view[i] = dprod;
		}
		else
		{
			view[i] = FALSE;
		}
	}

	//
	// Pick the best view.
	//

	best_score = 0;

	ULONG seed = p_thing->WorldPos.X ^ p_thing->WorldPos.Z;

	for (i = 0; i < EWAY_CAM_VIEW_ANGLES; i++)
	{
		seed *= 69069;
		seed += 1;

		if (!view[i])
		{
			continue;
		}

		score  = seed & 0x3ffff;
		score += 1;		// Incase (seed & 0xfff) is zero. It could happen!
		score += view[i];

		if (view[(i - 1) & (EWAY_CAM_VIEW_ANGLES - 1)]) {score += seed & 0x3ffff;}
		if (view[(i + 1) & (EWAY_CAM_VIEW_ANGLES - 1)]) {score += seed & 0x3ffff;}

		if (score > best_score)
		{
			EWAY_cam_get_position_for_angle(
				p_thing,
				i,
			   &vx,
			   &vy,
			   &vz);

			best_score = score;
			best_angle = i;
		}
	}

	if (best_score)
	{
		EWAY_cam_get_position_for_angle(
			p_thing,
			best_angle,
		   &EWAY_cam_x,
		   &EWAY_cam_y,
		   &EWAY_cam_z);
	}
	else
	{
		//
		// Find a default position to view the thing from.
		//

		EWAY_cam_x = p_thing->WorldPos.X >> 8;
		EWAY_cam_y = p_thing->WorldPos.Y >> 8;
		EWAY_cam_z = p_thing->WorldPos.Z >> 8;

		EWAY_cam_x -= dx >> 8;
		EWAY_cam_y += 0x140;
		EWAY_cam_z -= dz >> 8;
	}

	EWAY_cam_active   = TRUE;
	EWAY_cam_thing    = THING_NUMBER(p_thing);
	EWAY_cam_waypoint = NULL;
	EWAY_cam_freeze   = TRUE;

	EWAY_cam_x <<= 8;
	EWAY_cam_y <<= 8;
	EWAY_cam_z <<= 8;
}
#endif
#endif

void EWAY_cam_relinquish()
{
	EWAY_cam_goinactive = 2;

	/*

	EWAY_cam_jumped=1;
	EWAY_cam_active   = FALSE;
	EWAY_cam_thing    = NULL;
	EWAY_cam_waypoint = NULL;
	EWAY_cam_freeze   = FALSE;

	*/
}


SLONG EWAY_find_or_create_waypoint_that_created_person(Thing *p_person)
{
	SLONG i;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{	
//		global_write11=i;
		ew = &EWAY_way[i];

		if (EWAY_get_person(i) == THING_NUMBER(p_person))
		{
			return i;
		}
	}

	ASSERT(WITHIN(EWAY_way_upto, 1, EWAY_MAX_WAYS - 1));

	ew = &EWAY_way[EWAY_way_upto];

	ew->flag    = EWAY_FLAG_ACTIVE | EWAY_FLAG_DEAD;
	ew->ed.type = EWAY_DO_CREATE_ENEMY;
	ew->ed.arg1 = THING_NUMBER(p_person);

	return EWAY_way_upto++;
}


SLONG EWAY_conversation_happening(
		THING_INDEX *person_a,
		THING_INDEX *person_b)
{
	if (EWAY_conv_active)
	{
	   *person_a = EWAY_conv_person_a;
	   *person_b = EWAY_conv_person_b;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void EWAY_prim_activated(SLONG ob_index)
{
	SLONG i;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
//		global_write12=i;
		ew = &EWAY_way[i];

		if (ew->ec.type == EWAY_COND_PRIM_ACTIVATED && ew->ec.arg1 == ob_index)
		{
			ew->flag |= EWAY_FLAG_TRIGGERED;

			return;
		}
	}
}


void EWAY_deduct_time_penalty(SLONG time_to_deduct_in_hundreths_of_a_second)
{
	SLONG i;

	EWAY_Way *ew;

	for (i = 0; i < EWAY_MAX_WAYS; i++)
	{
		ew = &EWAY_way[i];

		if (ew->ec.type == EWAY_COND_COUNTDOWN_SEE)
		{
			SLONG depend = ew->ec.arg1;
			SLONG timer  = ew->ec.arg2;

			ASSERT(depend == 0 || WITHIN(depend, 1, EWAY_way_upto - 1));

			if (!depend || (EWAY_way[depend].flag & EWAY_FLAG_ACTIVE))
			{
				if (ew->ec.arg2 < time_to_deduct_in_hundreths_of_a_second)
				{
					ew->ec.arg2 = 0;
				}
				else
				{
					ew->ec.arg2 -= time_to_deduct_in_hundreths_of_a_second;
				}
			}
		}
	}
}

#ifdef	UNFINISHED
void	flag_undeletable_people(void)
{
	EWAY_Way *ew;
	EWAY_Cond *ec;
	SLONG	i;

	for (i = 1; i < eway_max; i++)
	{
		ew = &EWAY_way[i];

		switch(ew->type)
		{
			
		}
	}

	for (i = 1; i < eway_cond_upto; i++)
	{
		ec = &EWAY_cond[i];

		switch(ew->type)
		{
			case EWAY_COND_HALF_DEAD:
			case EWAY_COND_IS_MURDERER:
				SLONG i_person = EWAY_get_person(ec->arg1);

			case EWAY_COND_PERSON_USED:
				if (ec->arg1)
				{
					ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));

					if (EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_ENEMY)
					{
						EWAY_Way *ew2 = &EWAY_way[ec->arg1];

						if (ew2->ed.arg1)
						{
							//
							// Set the PERSON_FLAG_USEABLE so that when the player
							// presses use near this person the waypoint system
							// will be alerted.
							//

							ASSERT(TO_THING(ew2->ed.arg1)->Class == CLASS_PERSON);

							TO_THING(ew2->ed.arg1)->Genus.Person->Flags |= FLAG_PERSON_USEABLE;
						}
					}
				}
				break;

			case EWAY_COND_PERSON_ARRESTED:
				ans = FALSE; // By default.

				if (ec->arg1 == 0)
				{
					//
					// Hmm... person waypoint not set.
					//
	#ifndef	FINAL
	#ifndef	PSX
					//PANEL_new_text(NULL, 4000, "Waypoint %d (arrest) is invalid", ew->id);
	#endif
	#endif

					ew->flag |= EWAY_FLAG_DEAD;
				}
				else
				{
					ASSERT(WITHIN(ec->arg1, 1, EWAY_way_upto - 1));
					ASSERT(
						EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_ENEMY ||
						EWAY_way[ec->arg1].ed.type == EWAY_DO_CREATE_PLAYER);

					UWORD index = EWAY_way[ec->arg1].ed.arg1;

					if (index)
					{
						Thing *p_person = TO_THING(index);

						ASSERT(p_person->Class == CLASS_PERSON);

						ans = !!(p_person->Genus.Person->Flags & FLAG_PERSON_ARRESTED) && (p_person->SubState==SUB_STATE_DEAD_ARRESTED);
					}
				}

			case EWAY_COND_KILLED_NOT_ARRESTED:
			{
				SLONG waypoint;
				SLONG index;

				Thing    *p_thing;
				EWAY_Way *ew_dead;

				waypoint = ec->arg1;

				if (waypoint == 0)
				{
#ifndef PSX
					CBYTE mess[128];

					sprintf(mess, "Waypoint %d: cond person dead has NULL dependency", ew->id);

					CONSOLE_text(mess, 8000);
#endif
					ew->flag |= EWAY_FLAG_DEAD;
				}
				else
				{
					ew_dead = &EWAY_way[waypoint];

					switch(ew_dead->ed.type)
					{
						case EWAY_DO_CREATE_PLAYER:
						case EWAY_DO_CREATE_ENEMY:

							index = ew_dead->ed.arg1;

							if (index)
							{
								p_thing = TO_THING(index);

								ASSERT(p_thing->Class == CLASS_PERSON);

								if (p_thing->State == STATE_DEAD)
								{
									//
									// Not if they're arrested.
									//

									if (!(p_thing->Genus.Person->Flags & FLAG_PERSON_ARRESTED))
									{
										ans = TRUE;
									}
								}
							}

							break;

						default:


							{
							#ifndef PSX
								CBYTE mess[128];

								sprintf(mess, "Waypoint %d: killed not arrested dependent on odd waypoint", ew->id);

								CONSOLE_text(mess, 8000);
							#endif

								ew->flag |= EWAY_FLAG_DEAD;
							}


							break;
					}
				}
			}


			
		}
	}
}
#endif



#ifdef TARGET_DC

// Goes through the waypoints that set up stuff at start of day,
// that will already have been activated, but the state they set up wasn't
// saved in the DAD file.
void EWAY_reactivate_waypoints_that_arent_in_the_dad_file ( void )
{
	for ( int i = 0; i < EWAY_way_upto; i++ )
	{
		EWAY_Way *ew = &EWAY_way[i];

		switch ( ew->ed.type )
		{
		case EWAY_DO_CREATE_MIST:
			EWAY_set_active ( ew );
			break;

		default:
			// Nothing to do.
			break;
		}
	}
}


#endif



