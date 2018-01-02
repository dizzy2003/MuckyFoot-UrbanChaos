// Person.cpp
// Guy Simmons, 12th January 1998.

#include	"Game.h"
#include	"Cop.h"
#include	"cam.h"
#include	"Darci.h"
#include	"Roper.h"
#include	"Thug.h"
#include	"id.h"
#include	"statedef.h"
#include	"animate.h"
#include	"combat.h"
#include	"sample.h"
#include	"guns.h"
#include	"cnet.h"
#include	"interfac.h"
//#include	"command.h"
#include	"sewer.h"
#include	"mav.h"
#include	"Sound.h"
#include	"eway.h"
#include	"spark.h"
#include	"drip.h"
#include	"puddle.h"
#include	"pap.h"
#include	"supermap.h"
#include	"ns.h"
#include	"dirt.h"
#include	"hook.h"
#include	"pcom.h"
#include	"tracks.h"
#include	"Matrix.h"
#include	"ob.h"
#include	"wmove.h"
#include	"balloon.h"
#include	"inside2.h"
#include	"walkable.h"
#include	"overlay.h"
#include	"psystem.h"
#include	"poly.h"
#include	"memory.h"
#include	"fmatrix.h"
#include	"fc.h"
#include	"mfx.h"
#include	"night.h"
#include	"ware.h"
#include	"xlat_str.h"
#include	"pow.h"
#include	"frontend.h"
#ifdef		PSX 
#include "c:\fallen\psxeng\headers\psxeng.h"
#include "c:\fallen\psxeng\headers\panel.h"
#else
#include "aeng.h"
#include "panel.h"
#endif

#ifndef PSX
UBYTE	player_visited[16][128];
extern	SLONG	save_psx;
#else
#define save_psx (1)
#endif


#ifdef TARGET_DC
#include "DIManager.h"
#endif


#define MAX_COL_WITH 16


extern Thing *is_person_under_attack_low_level(
				Thing *p_person,
				SLONG  any_state,
				SLONG  radius);


/*
//MF_SOUND_API


//
// flags = MOVING/LOOPED/INTERRUPT/RESTART/???
//

  //
  // id is a general identification you give the sample so you can alter the playing sample later
  // e.g a thing may use THING_NUMBER(p_thing) to generate a unique ID for the thing generating the sound effect

  //
  // the sound system will have to handle this stuff internally
  //

MF_sfx_play_thing(UWORD id,Thing *p,UWORD sample,UWORD pitch,ULONG flags)
MF_sfx_play_pos(UWORD id,SLONG x,SLONG y,SLONg z,UWORD sample,UWORD pitch,ULONG flags)
MF_sfx_play_wpos(UWORD id,struct WorldPos *pos,UWORD sample,UWORD pitch,ULONG flags)
MF_sfx_play(UWORD id,UWORD sample,UWORD vol,UWORD pitch,ULONG flags);

//
// these functions alter samples allready playing, the sfx system will search for a playing sample with matching id and sample, to alter
//
MF_sfx_change_pitch_thing(UWORD id,UWORD sample)
MF_sfx_change_sample(UWORD id,UWORD old_sample,UWORD new_sample,ULONG flags)
MF_sfx_change_pos(UWORD id,UWORD sample,SLONG x,SLONG y,SLONG z)

MF_sfx_stop_sample(UWORD id,UWORD sample);
MF_sfx_stop_all();




*/




// 
// From Jan...
// 
// 
// There was a young actress from Crewe,
// Who remarked as the vicar withdrew,
// The Bishop was quicker
// and thicker and slicker,
// and two inches longer than you.
// 
// There was a young vampire called mable,
// whose periods were always quite stable,
// at every full moon
// she took out a spoon,
// and drank herself under the table.
// 
// There was a young plumber from Lee,
// who was plumbing his girl with great glee,
// she said stop your plumbing,
// I think someones coming,
// said the plumber still plumbing "its me"!
// 
// A kinky young girl from Coleshill,
// Tried a dynamite stick for a thrill,
// They found her vagina,
// in North Carolina,
// and bits of her tits in Brazil.
// 
// There was a young man from Pitlocherie,
// making love to his girl in the rockery,
// she said look you've cum,
// all over my bum,
// This isn't a shag it's a mockery.
// 
// There was a young lassie from Morton,
// who had one long tit and one short 'en,
// on top of all that,
// a great hairy twat,
// and a fart like a six fifty Norton.
// 
// There was a young man from Nantucket,
// Who's appendage was so long he could suck it,
// He was heard to allude,
// if I may be so crude,
// If my ear was a c*nt I could f*ck it.
// 


// by mike

//	 There was a young stripper from London
//	 whos dress was easily undone
//	 with a quick flash 
//   we'd all see her Gash 
//	 and shout out quite loudly, well done.
//
// ^^^^ this is rubbish, it doesn't fit the meter

//	 A girl named Jackie from Vancouver
//	 Liked to have sex with a hoover
//	 Until she met Chris
//   While out on the piss
//	 Now she's a real sexy groover

#ifndef PSX
extern BOOL allow_debug_keys;
#endif

extern	SLONG	person_holding_2handed(Thing *p_person);
extern	SLONG	continue_dir(Thing *p_person,SLONG dir);
extern	SLONG	should_i_sneak(Thing *p_person);
extern	void	change_velocity_to(Thing *p_person,SWORD velocity);
extern	void	change_velocity_to_slow(Thing *p_person,SWORD velocity);
extern	SLONG	PAP_on_slope(SLONG x,SLONG z,SLONG *angle);
extern	SLONG	RFACE_on_slope(SLONG face,SLONG x,SLONG z,SLONG *angle);
extern	void	process_gang_attack(Thing *p_person,Thing *p_target);
extern	void	reset_gang_attack(Thing *p_target);
extern	void drop_on_heads(Thing *p_thing);
extern	UWORD	count_gang(Thing *p_target);
extern	void person_enter_fight_mode(Thing *p_person);
extern	UWORD	get_any_gang_member(Thing *p_target);
extern	void	add_damage_value_thing(Thing *p_thing,SLONG value);
extern	void	set_person_locked_idle_ready(Thing *p_person);
extern	SLONG	remove_from_gang_attack(Thing *p_person,Thing *p_target);
extern	SLONG continue_pressing_action(Thing *p_person);
extern	void	set_action_used(Thing *p_person);
extern	void	carry_running(Thing *p_person);
extern	void	set_person_stand_carry(Thing *p_person);


extern UBYTE stealth_debug;


extern	SLONG	plant_feet(Thing *p_person);
extern	SLONG	calc_angle(SLONG dx,SLONG dz);
extern	SLONG	set_person_sidle(struct Thing *p_person);
extern	SLONG	slide_ladder;

extern	SLONG	yomp_speed;
extern	SLONG	sprint_speed;

UWORD	player_dlight=0;

void	aim_at_victim(Thing *p_person,SLONG count=1);
void	set_anim_idle(Thing *p_person);
SLONG dist_to_target_pelvis(Thing *p_person_a,Thing *p_person_b);
UWORD	find_arrestee(Thing *p_person);
void	set_person_fight_idle(Thing *p_person);
SLONG	set_person_pos_for_fence_vault(Thing *p_person,SLONG col);

SLONG	check_near_facet(Thing *p_person,SLONG max_dist,SLONG max_end_dist,SLONG px,SLONG pz);

SLONG	find_idle_fight_stance(Thing *p_person);
SLONG	person_holding_special(Thing* p_person, UBYTE special);
SLONG	person_holding_bat(Thing *p_person)	{ return person_holding_special(p_person, SPECIAL_BASEBALLBAT); }
void	set_person_running(Thing *p_person);

void	fn_person_moveing(Thing *p_person);
void	fn_person_idle(Thing *p_person);
void	fn_person_jumping(Thing *p_person);
void	fn_person_dangling(Thing	*p_person);
void	fn_person_laddering(Thing	*p_person);
void	fn_person_climbing(Thing	*p_person);
void	fn_person_fighting(Thing *p_person);
void	fn_person_recoil(Thing *p_person);
void	fn_person_dying(Thing *p_person);
void	fn_person_dead(Thing *p_person);
void	fn_person_gun(Thing *p_person);
void	fn_person_wait(Thing *p_person);
void	fn_person_navigate(Thing *p_person);
void	fn_person_fight(Thing *p_person);
void	fn_person_stand_up(Thing *p_person);
void	fn_person_mavigate(Thing *p_person);
void	fn_person_grapple(Thing *p_person);
void	fn_person_goto(Thing *p_person);
void	fn_person_can(Thing *p_person);
void	fn_person_circle(Thing *p_person);
void	fn_person_hug_wall(Thing *p_person);
void	fn_person_search(Thing *p_person);
void	fn_person_carry(Thing *p_person);
void	fn_person_float(Thing *p_person);

SLONG	find_anim_fall_dir(SLONG	anim);

SLONG	set_person_pos_for_fence(Thing *p_person,SLONG col,SLONG set_pos,SLONG req_dist);
void	locked_anim_change(Thing *p_person,UWORD locked_object,UWORD anim,SLONG dangle=0);
void	do_person_on_cable(Thing *p_person);
SLONG	fight_any_gang_attacker(Thing *p_person);

extern	SLONG	projectile_move_thing(Thing *p_thing,SLONG flag);
extern	SLONG	calc_height_at(SLONG x,SLONG z);
extern	SLONG	find_best_grapple(Thing *p_person);

SLONG	set_person_pos_for_half_step(Thing *p_person,SLONG col);
void	set_person_sneaking(Thing *p_person);
void	set_person_mav_to_xz(Thing *p_person,SLONG x,SLONG z);
SLONG	turn_to_face_thing(Thing *p_person,Thing *p_target,SLONG slow);
SLONG	set_face_thing(Thing *p_person,Thing *p_target);
void	set_person_mav_to_thing(Thing *p_person,Thing *p_target);
SLONG	get_pitch_to_thing_quick(Thing *p_person,Thing *p_target);
void	turn_to_face_thing_quick(Thing *p_person,Thing *p_target);
SLONG	get_yomp_anim(Thing *p_person);
SLONG	person_holding_2handed(Thing *p_person);
void	drop_all_items(Thing *p_person, UBYTE is_being_searched);
void	set_person_croutch(Thing *p_person);
void	drop_current_gun(Thing *p_person,SLONG change_anim);
SLONG	player_running_aim_gun(Thing *p_person);
SLONG	might_i_be_a_villain(Thing *p_person);
void	highlight_gun_target(Thing *p_person,Thing *p_target);
void	locked_anim_change_of_type(Thing *p_person,UWORD locked_object,UWORD anim,SLONG type);
void	set_anim_running(Thing *p_person);

extern	void	move_thing_on_map_dxdydz(Thing *t_thing,SLONG dx,SLONG dy,SLONG dz);

#if !defined(PSX) && !defined(TARGET_DC)
SLONG	person_is_on_sewer(Thing *p_person);
#endif


GenusFunctions	people_functions[]	=
{
	{	PERSON_DARCI,		darci_states	},
	{	PERSON_ROPER,		roper_states	},//roper
	{	PERSON_COP,			cop_states		},
	{	PERSON_CIV,			cop_states		},
	{	PERSON_THUG_RASTA,	cop_states		},
	{	PERSON_THUG_GREY,	cop_states		},
	{	PERSON_THUG_RED,	cop_states		},
	{	PERSON_SLAG_TART,	cop_states		},
	{	PERSON_SLAG_FATUGLY,cop_states		},
	{	PERSON_HOSTAGE,		cop_states		},
	{	PERSON_MECHANIC,	cop_states		},
	{	PERSON_TRAMP,	cop_states		},
	{	PERSON_MIB1,	cop_states		},
	{	PERSON_MIB2,	cop_states		},
	{	PERSON_MIB3,	cop_states		},



};


StateFunction	generic_people_functions[]	=
{
	{	STATE_INIT,				NULL				},
	{	STATE_NORMAL,			NULL				},
	{	STATE_HIT,				NULL				},
	{	STATE_ABOUT_TO_REMOVE,	NULL				},
	{	STATE_REMOVE_ME,		NULL				},

	{	STATE_MOVEING,			fn_person_moveing	},
	{	STATE_IDLE,				fn_person_idle		},
	{	STATE_LANDING,			NULL				},
	{	STATE_JUMPING,			fn_person_jumping	},
	{	STATE_FIGHTING,			fn_person_fighting	},
	{	STATE_FALLING,			NULL				},
	{	STATE_USE_SCENERY,		NULL				},
	{	STATE_DOWN,				NULL				},
	{	STATE_HIT,				NULL				},
	{	STATE_CHANGE_LOCATION,	NULL				},
	{	STATE_DRIVING,			NULL				},
	{	STATE_DYING,			fn_person_dying		},
	{	STATE_DEAD,				fn_person_dead		},
	{	STATE_DANGLING,			fn_person_dangling  },
	{	STATE_CLIMB_LADDER,		fn_person_laddering },
	{	STATE_HIT_RECOIL,		fn_person_recoil	},
	{	STATE_CLIMBING,			fn_person_climbing	},
	{	STATE_GUN,				fn_person_gun		},
	{	0,						NULL				},
	{	0,						NULL				},
	{	STATE_NAVIGATING,		fn_person_navigate	},
	{	STATE_WAIT,				fn_person_wait	},
	{	STATE_FIGHT,			fn_person_fight	},
	{	0,						NULL				},  //stand up?
	{	STATE_MAVIGATING,		fn_person_mavigate	},
	{	STATE_GRAPPLING,		fn_person_grapple	},
	{	STATE_GOTOING,			fn_person_goto		},
	{	STATE_CANNING,			fn_person_can		},
	{	STATE_CIRCLING,			fn_person_circle	},
	{	STATE_HUG_WALL,			fn_person_hug_wall	},
	{	STATE_SEARCH,			fn_person_search	},
	{	STATE_CARRY,			fn_person_carry		},
	{	STATE_FLOAT,			fn_person_float		},
	{	0,						NULL				}
};

CBYTE *PERSON_mode_name[PERSON_MODE_NUMBER] =
{
	"Run",
	"Walk",
	"Sneak",
	"Fight"
};

SLONG	stat_killed_thug;
SLONG	stat_killed_innocent;
SLONG	stat_arrested_thug;
SLONG	stat_arrested_innocent;
SLONG	stat_count_bonus;
SLONG	stat_start_time,stat_game_time;

void	set_stats(void)
{
	stat_game_time=GetTickCount()-stat_start_time;
}

void	init_stats(void)
{
	stat_killed_thug=0;
	stat_killed_innocent=0;
	stat_arrested_thug=0;
	stat_arrested_innocent=0;
	stat_count_bonus=0;
	stat_start_time=GetTickCount(); //I believe this is in PSX
}


/*
#define PERSON_NONE			100	// For debug...
#define PERSON_DARCI		0
#define PERSON_ROPER		1
#define PERSON_COP			2
#define PERSON_CIV			3
#define PERSON_THUG_RASTA	4
#define PERSON_THUG_GREY	5
#define PERSON_THUG_RED		6
#define PERSON_SLAG_TART	7
#define PERSON_SLAG_FATUGLY	8
#define PERSON_HOSTAGE		9
#define PERSON_MECHANIC		10
#define PERSON_NUMBER		11	// Number of people types.
*/

UBYTE anim_type[PERSON_NUM_TYPES] =
{
	ANIM_TYPE_DARCI,
	ANIM_TYPE_ROPER,
	ANIM_TYPE_CIV,
	ANIM_TYPE_CIV,
	ANIM_TYPE_CIV,
	ANIM_TYPE_CIV,
	ANIM_TYPE_CIV,
	ANIM_TYPE_DARCI,
	ANIM_TYPE_DARCI,
	ANIM_TYPE_DARCI,
	ANIM_TYPE_CIV, //10
	ANIM_TYPE_CIV, //11 tramp
	ANIM_TYPE_CIV, //11  mib
	ANIM_TYPE_CIV, //12 
	ANIM_TYPE_CIV //13 
};

UBYTE mesh_type[PERSON_NUM_TYPES] = 
{
	0, //darci
	0, //roper
	4, //cop
	7, //civ
	0, //rasta
	1, //thug grey
	2, //thug red
	1, //slag tart
	2, //slag fatugly
	3, //hostage
	3, // ed miller , mechanic
	6, //tramp
	5, //mib
	5,
	5
};

SWORD health[PERSON_NUM_TYPES]=
{
	200,
	400,
	200,
	130,
	200,
	200,
	200,
	130,
	130,
	200,
	200,
	200,
	700,
	700,
	700
};



#ifndef PSX
void	set_player_visited(UBYTE x,UBYTE z)
{
	UWORD	bit;
	ASSERT(WITHIN(x, 0, 127));
	ASSERT(WITHIN(z, 0, 127));
	bit=x&7;
	x=x>>3;
	player_visited[x][z]|=1<<bit;

}
#endif

inline BOOL	MagicFrameCheck(Thing *p_person, UBYTE frameindex) {
	if (p_person->Draw.Tweened->FrameIndex>=frameindex) {
		if (!(p_person->Genus.Person->Flags2&FLAG2_SYNC_SOUNDFX)) {
			p_person->Genus.Person->Flags2|=FLAG2_SYNC_SOUNDFX;
			return TRUE;
		}
	} else {
		p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	}
	return FALSE;

}

BOOL PersonIsMIB(Thing* p_person)
{
	return (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3);
}

void SlideSoundCheck(Thing* p_person, BOOL force=0)
{
	if (p_person->Genus.Person->Flags&FLAG_PERSON_SLIDING)
	{
		MFX_stop(THING_NUMBER(p_person),S_SEARCH_END);
		if (force||!((p_person->State==STATE_MOVEING)&&(p_person->SubState==SUB_STATE_RUNNING_SKID_STOP))) 
		{
			MFX_stop(THING_NUMBER(p_person),S_SLIDE_START);
			p_person->Genus.Person->Flags&=~FLAG_PERSON_SLIDING;
		}
	}
	else
	{
		if (p_person->SubState==SUB_STATE_RUNNING_SKID_STOP)
		{
#ifndef PSX
			MFX_play_thing(THING_NUMBER(p_person),S_SLIDE_START,MFX_LOOPED,p_person);
#else
			MFX_play_thing(THING_NUMBER(p_person),S_SLIDE_START,MFX_LOOPED|MFX_FLAG_SLIDER,p_person);
#endif
			p_person->Genus.Person->Flags |= FLAG_PERSON_SLIDING;
		}

	}

}

//---------------------------------------------------------------


#ifndef PSX
void	init_persons(void)
{
	SLONG	c0;
	memset((UBYTE*)PEOPLE,0,sizeof(Person)*MAX_PEOPLE);
	for(c0=0;c0<MAX_PEOPLE;c0++)
	{
		PEOPLE[c0].AnimType=PERSON_NONE;
	}
	PERSON_COUNT	=	0;
}
#endif

//---------------------------------------------------------------

Thing	*alloc_person(UBYTE type, UBYTE random_number)
{
	SLONG			c0;
	Person			*new_person;
	Thing			*person_thing	=	NULL;

//	if(type==1)
//		type=4;

	// Run through the people array & find an unused one.
	for(c0=0;c0<MAX_PEOPLE;c0++)
	{
		if(PEOPLE[c0].AnimType==PERSON_NONE)
		{
			person_thing	=	alloc_thing(CLASS_PERSON);
			if(person_thing)
			{
				new_person					=	TO_PERSON(c0);
				new_person->PersonType		=	type;
				
				new_person->AnimType		=	anim_type[type];
				new_person->Thing			=	THING_NUMBER(person_thing);
				new_person->PlayerID		=	0;
				new_person->Ammo			=	15;
				new_person->SpecialList		=	0;
				new_person->SpecialUse      =   0;
				new_person->Stamina			=	128;
				person_thing->Genus.Person	=	new_person;
				person_thing->Draw.Tweened	=	alloc_draw_tween(DT_ROT_MULTI);

				person_thing->OnFace        =	NULL;

				person_thing->Draw.Tweened->TheChunk			=	&game_chunk[anim_type[type]];
/*
				switch(anim_type[type])
				{
					case	ANIM_TYPE_DARCI:
						person_thing->Draw.Tweened->TheChunk			=	&game_chunk[anim_type[type]];
						break;
					case	ANIM_TYPE_ROPER:
						person_thing->Draw.Tweened->TheChunk			=	&game_chunk[2];
						break;
					case	ANIM_TYPE_CIV:
						person_thing->Draw.Tweened->TheChunk			=	&game_chunk[3];
						break;
//					case	ANIM_TYPE_COP:
//						person_thing->Draw.Tweened->TheChunk			=	&game_chunk[1];
						break;
				}
*/

				person_thing->Draw.Tweened->MeshID	=	mesh_type[type];
#ifdef	PSX
//				ASSERT(person_thing->Draw.Tweened->MeshID||type==4);
#endif
/*
// aint true no more
				if(save_psx)
				{
					//
					// PSX don't have ugly whore 
					//
					if(person_thing->Draw.Tweened->MeshID==6)
						person_thing->Draw.Tweened->MeshID=5;
					else
					if(person_thing->Draw.Tweened->MeshID==8)
						person_thing->Draw.Tweened->MeshID=6;
				}
*/


				/*

				//if(new_person->AnimType==ANIM_TYPE_CIV && person_thing->Draw.Tweened->MeshID==0)
				if(type==PERSON_THUG)
				{
					SLONG	mesh=Random()%2;

					person_thing->Draw.Tweened->MeshID	=	mesh+1; //normal thug


				}
				else
				if(type==PERSON_THUG2)
				{
					SLONG	mesh=Random()%5;
					switch(mesh)
					{
						case	0:
						case	1:
						case	2:
							person_thing->Draw.Tweened->MeshID	=	mesh; //normal thug
							break;
						case	3:
							person_thing->Draw.Tweened->MeshID	=	8; // mechanic
							break;
						case	4:
							person_thing->Draw.Tweened->MeshID	=	5; //slag1
							break;
					}

				}

				if(type==PERSON_CIV || type==PERSON_CIV2)
				{
					person_thing->Draw.Tweened->PersonID=6+Random()%6;
					if(person_thing->Draw.Tweened->PersonID==10)
					{
						person_thing->Draw.Tweened->PersonID=0;
						person_thing->Draw.Tweened->MeshID	=	7; //hostage
					}
					if(person_thing->Draw.Tweened->PersonID==11)
					{
						person_thing->Draw.Tweened->PersonID=0;
						person_thing->Draw.Tweened->MeshID	=	6; //ugly slag (real ugly)
					}
				}

				*/

				if (type == PERSON_CIV)
				{
					//
					// A civ's PersonID makes him look different.
					//

					person_thing->Draw.Tweened->PersonID = 6 + random_number % 4;
//					if(Random()&1)
//						person_thing->Draw.Tweened->MeshID	++; //make it a bloke
				}
/*
				if (type == PERSON_MIB2)
				{
					person_thing->Draw.Tweened->PersonID = 1;
				}
				if (type == PERSON_MIB3)
				{
					person_thing->Draw.Tweened->PersonID = 2;
				}
*/

				set_state_function(person_thing,STATE_INIT);
				person_thing->Genus.Person->Health=health[type];
				return(person_thing);

			}
			else
			{
				ASSERT(0);
			}

			break;
		}
	}
	ASSERT(0);
	return	person_thing;
}

//---------------------------------------------------------------

void	free_person(Thing *person_thing)
{
	// Set the person type to none & free the thing.
	person_thing->Genus.Person->AnimType	=	PERSON_NONE;
	free_draw_tween(person_thing->Draw.Tweened);
	person_thing->Draw.Tweened=0; // clear the pointer just incase

	free_thing(person_thing);
}

//---------------------------------------------------------------

UBYTE	global_person=0;

THING_INDEX	create_person(
				SLONG type,
				SLONG random_number,
				SLONG x,
				SLONG y,
				SLONG z)
{
	Thing *p_person = alloc_person(type, random_number);
	global_person++;

	

	if (p_person)
	{
#ifndef	PSX
#ifndef TARGET_DC

extern	SWORD	people_types[50];
		people_types[type]++;
#endif
#endif
//		ASSERT(THING_NUMBER(p_person)!=195);
		p_person->WorldPos.X = x;
		p_person->WorldPos.Y = y;
		p_person->WorldPos.Z = z;

//		ASSERT(!(MAV_SPARE(x>>16,z>>16) & MAV_SPARE_FLAG_WATER));

		p_person->Genus.Person->HomeX = x >> 8;
		p_person->Genus.Person->HomeZ = z >> 8;


		//
		// Is this person on a building?
		//

		SLONG new_y;
		SLONG face = find_face_for_this_pos(
						x >> 8,
						y >> 8,
						z >> 8,
					   &new_y,
						NULL,0);

//		ASSERT(type!=PERSON_DARCI);

		if (face)
		{
			if (face == GRAB_FLOOR)
			{

				p_person->WorldPos.Y = new_y << 8;
				//
				// No problem.
				//
			}
			else
			{
				//
				// Put the person on this face.
				//

				p_person->OnFace     = face;
				p_person->WorldPos.Y = new_y << 8;
			}
		}

		// plant_feet(p_person);	// ?

		//
		// Is this person inside a warehouse?
		//

		if (PAP_2HI(
				p_person->WorldPos.X >> 16,
				p_person->WorldPos.Z >> 16).Flags & PAP_FLAG_HIDDEN)
		{
			SLONG ware_top;

			ware_top = PAP_calc_map_height_at(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Z >> 8);

			if (p_person->WorldPos.Y < (ware_top - 0x80 << 8))
			{
				p_person->Genus.Person->Ware = WARE_which_contains(
													p_person->WorldPos.X >> 16,
													p_person->WorldPos.Z >> 16);
				
				if (p_person->Genus.Person->Ware)
				{
					p_person->Genus.Person->Flags  |= FLAG_PERSON_WAREHOUSE;
					p_person->Genus.Person->Flags2 |= FLAG2_PERSON_HOME_IN_WAREHOUSE;
				}
			}
		}

		//
		// Initialise the person straight away!
		//

		PTIME(p_person)=(UBYTE)Random();
		p_person->StateFn(p_person);

		return THING_NUMBER(p_person);
	}
	else
	{
		ASSERT(0);
	}

	return NULL;
}


//
// Returns TRUE if there is enough room in front of the given person.
// 

SLONG is_there_room_in_front_of_me(Thing *p_person, SLONG how_much_room)
{
	SLONG x1;
	SLONG y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG dx;
	SLONG dz;

	x1 = p_person->WorldPos.X          >> 8;
	y1 = p_person->WorldPos.Y + 0x6000 >> 8;
	z1 = p_person->WorldPos.Z          >> 8;

	dx = -SIN(p_person->Draw.Tweened->Angle) * how_much_room >> 16;
	dz = -COS(p_person->Draw.Tweened->Angle) * how_much_room >> 16;

	x2 = x1 + dx;
	z2 = z1 + dz;

	y2 = PAP_calc_map_height_at(x2,z2) + 0x60;

	if (abs(y1 - y2) > 0x40)
	{
		//
		// Too much height difference.
		//

		return FALSE;
	}

	if (!there_is_a_los(
			x1, y1, z1, 
			x2, y2, z2,
			LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG))
	{
		return FALSE;
	}

	return TRUE;
}



//---------------------------------------------------------------
extern	THING_INDEX col_with[];

SLONG find_searchable_person(Thing *p_person)
{
	SLONG       col_with_upto;
	SLONG		collide_types = (1 << CLASS_PERSON);
	Thing		*col_thing;
	SLONG		i;
	SLONG		best_dist=INFINITY,best_index=0,dist;

	col_with_upto = THING_find_sphere(
					    p_person->WorldPos.X>>8,
					    p_person->WorldPos.Y>>8,
					    p_person->WorldPos.Z>>8,
						256,
						col_with,
						MAX_COL_WITH,
						collide_types);

	for (i = 0; i < col_with_upto; i++)
	{
		col_thing = TO_THING(col_with[i]);

		if (col_thing->State == STATE_DEAD && (col_thing->SubState==SUB_STATE_DEAD_ARRESTED||col_thing->SubState==0) && !(col_thing->Genus.Person->Flags&FLAG_PERSON_SEARCHED))
		{
			dist=dist_to_target_pelvis(p_person,col_thing);
			if(dist<best_dist)
			{
				best_dist=dist;
				best_index=col_with[i];
			}
		}
	}

	if(best_dist<64)
	{
		return(best_index);

	}
	else
		return(0);

}


SLONG set_person_search(Thing *p_person, SLONG ob_index,SLONG ox,SLONG oy,SLONG oz)
{
	SLONG	dx,dz,angle,dangle;

	dx=(p_person->WorldPos.X>>8)-ox;
	dz=(p_person->WorldPos.Z>>8)-oz;

	angle=calc_angle(dx,dz);
	dangle=angle-p_person->Draw.Tweened->Angle;

	if(abs(dangle)<256 || angle>2048-256)
	{

		p_person->Draw.Tweened->Angle=angle;
		set_person_locked_idle_ready(p_person);
		set_generic_person_state_function(p_person,STATE_SEARCH);
		p_person->SubState=SUB_STATE_SEARCH_PRIM;
		p_person->Genus.Person->Timer1=0;
		p_person->Genus.Person->Flags |= (FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

		p_person->Genus.Person->Action=ACTION_NONE;
		p_person->Genus.Person->Target=ob_index;
		return(1);
	}
	else
		return(0);

}

SLONG set_person_search_corpse(Thing *p_person, Thing *p_dead)
{

//	set_person_locked_idle_ready(p_person);

	set_anim(p_person, ANIM_CROUTCH_DOWN);

	set_generic_person_state_function(p_person,STATE_SEARCH);
	p_person->SubState=SUB_STATE_SEARCH_CORPSE;
	p_person->Genus.Person->Timer1=0;
	p_person->Genus.Person->Flags |= (FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	p_person->Genus.Person->Action=ACTION_NONE;
	p_person->Genus.Person->Target=THING_NUMBER(p_dead);
#ifndef PSX
	MFX_play_ambient(THING_NUMBER(p_person),S_SEARCH_END,MFX_LOOPED);
#else
	MFX_play_ambient(THING_NUMBER(p_person),S_SEARCH_END,MFX_LOOPED|MFX_FLAG_SEARCHER);
#endif
	p_person->Genus.Person->Flags|=FLAG_PERSON_SLIDING; // cunning lie.
	return(1);
}

void	release_searched_item(Thing *p_person)
{
	SLONG	index;
	Thing	*p_thing;
	SLONG	ob_index;

	MFX_stop(THING_NUMBER(p_person),S_SEARCH_END);
	switch(p_person->SubState)
	{
		case	SUB_STATE_SEARCH_CORPSE:
			drop_all_items(TO_THING(p_person->Genus.Person->Target),1);
			ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
			TO_THING(p_person->Genus.Person->Target)->Genus.Person->Flags|=FLAG_PERSON_SEARCHED;
			MFX_play_ambient(THING_NUMBER(p_person),S_HIDDENITEM,0);
			break;
		case	SUB_STATE_SEARCH_PRIM:
			ob_index=p_person->Genus.Person->Target;
			p_person->Genus.Person->Target=0;

			if(!(OB_ob[ob_index].flags&OB_FLAG_HIDDEN_ITEM))
			{
				return;
			}

			MFX_play_ambient(THING_NUMBER(p_person),S_HIDDENITEM,0);
			index=thing_class_head[CLASS_SPECIAL];
			while(index)
			{

				p_thing=TO_THING(index);
				if(p_thing->Flags&FLAG_SPECIAL_HIDDEN)
				{
					if(p_thing->Genus.Special->counter==ob_index)
					{
						add_thing_to_map(p_thing);
						p_thing->Flags&=~FLAG_SPECIAL_HIDDEN;
						p_thing->Genus.Special->counter = 0;
						OB_ob[ob_index].flags&=~OB_FLAG_HIDDEN_ITEM;
					}
				}

				index=p_thing->NextLink;
			}
			break;
	}
	
}

//
// Returns a place on the ground near the given person.
//

void find_nice_place_near_person(
		Thing *p_person,
		SLONG *nice_x,	// 8-bits per mapsquare
		SLONG *nice_y,
		SLONG *nice_z)
{
	SLONG gx;
	SLONG gy;
	SLONG gz;

	SLONG	px,py,pz;

	//
	// Anywhere on the same mapsquare as the person should do!
	//

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_PELVIS,
	   &px,
	   &py,
	   &pz);

	px = px + (p_person->WorldPos.X >> 8);
	py = py + (p_person->WorldPos.Y >> 8);
	pz = pz + (p_person->WorldPos.Z >> 8);
	
	gx = (((px) & 0xff00) | (Random() & 0x1f)) + 0x3f;
	gz = (((pz) & 0xff00) | (Random() & 0x1f)) + 0x3f;

	gy = PAP_calc_height_at_thing(p_person,gx,gz);

	//
	// If there is a huge difference in y, then it ain't a nice place!
	//

	if (abs(gy - (p_person->WorldPos.Y >> 8)) > 0x50)
	{
		gx = p_person->WorldPos.X >> 8;
		gz = p_person->WorldPos.Z >> 8;

		gy = PAP_calc_height_at_thing(p_person,gx,gz);
	}

   *nice_x = gx;
   *nice_y = gy + 0x30;	// above the ground a little bit..
   *nice_z = gz;
}



//
// Valid substates are
//
//	SUB_STATE_DYING_INITIAL_ANI
//	SUB_STATE_DYING_FINAL_ANI
//	SUB_STATE_DYING_KNOCK_DOWN	- means they play a get up animation after their current animation is over and don't die.
//

void set_person_dying(Thing *p_person, UBYTE substate)
{
	//
	// Valid substate?
	// 

	ASSERT(
		substate == SUB_STATE_DYING_ACTUALLY_DIE ||
		substate == SUB_STATE_DYING_INITIAL_ANI ||
		substate == SUB_STATE_DYING_FINAL_ANI   ||
		substate == SUB_STATE_DYING_KNOCK_DOWN  ||
		substate == SUB_STATE_DYING_KNOCK_DOWN_WAIT);

	//
	// Let go of any balloon you may have.
	//

#if !defined(PSX) && !defined(TARGET_DC)
	if (p_person->Genus.Person->Balloon)
	{
		BALLOON_release(p_person->Genus.Person->Balloon);
	}
#endif

	set_generic_person_state_function(p_person,STATE_DYING);

	p_person->Velocity             = 0;
	p_person->Genus.Person->Action = ACTION_DYING;
	p_person->SubState             = substate;
	p_person->Genus.Person->Flags |= (FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	//
	// The pcom ai counter is used to count how long somebody is dead.
	// 

	p_person->Genus.Person->pcom_ai_counter = 0;

	if ((substate == SUB_STATE_DYING_KNOCK_DOWN)||(substate == SUB_STATE_DYING_KNOCK_DOWN_WAIT))
		return; // only continue if they're actually _dying_.

	if (PersonIsMIB(p_person))
	{
//				MFX_play_ambient(0,S_THUNDER_START+(Random()&1),0);
		MFX_play_thing(THING_NUMBER(p_person),S_MIB_LEVITATE,MFX_LOOPED,p_person);
	}

}


//
// Returns if the person is lying on his front or back
// 

#define PERSON_ON_HIS_FRONT		0
#define PERSON_ON_HIS_BACK		1
#define PERSON_ON_HIS_SOMETHING 2

SLONG person_is_lying_on_what(Thing *p_person)
{
	ASSERT(p_person->Class == CLASS_PERSON);

	switch(p_person->Draw.Tweened->CurrentAnim)
	{
		case ANIM_KO_BEHIND_BIG:
		case ANIM_KO_BEHIND_BIG_GU:
		case ANIM_KD_FRONT_LOW:
		case ANIM_KD_FRONT_MID:
		case ANIM_KD_FRONT_HI:
		case ANIM_FIGHT_STOMPED_BACK:
		case ANIM_KD_FRONT_LAND:
		case ANIM_HANDS_UP_LIE:
		case ANIM_PISTOL_WHIP_TAKE:
			return PERSON_ON_HIS_FRONT;

		case ANIM_KO_BACK:
		case ANIM_KO_BACK_GU:
		case ANIM_KD_BACK_LOW:
		case ANIM_KD_BACK_MID:
		case ANIM_KD_BACK_HI:
		case ANIM_FIGHT_STOMPED_FRONT:
		case ANIM_KD_BACK_LAND:
		case ANIM_GRAB_ARM_THROWV:
		case ANIM_STRANGLE_VICTIM:
		case ANIM_HEADBUTT_VICTIM:
			return(PERSON_ON_HIS_BACK);

		default:
#ifndef	NDEBUG
			return PERSON_ON_HIS_SOMETHING;
#else
			return(PERSON_ON_HIS_BACK);
#endif
	}
}


SLONG	footstep_wave(Thing *p_person)
{
	//
	// Which one out of a choice of sounds we play.
	// 

//	static SLONG which;

	SLONG start;
	SLONG end;
	SLONG num;

	if (p_person->Genus.Person->Ware)
	{
		// in a warehouse. generic texture.
#ifndef PSX
		return SOUND_Range(S_SOFT_STEP_START,S_SOFT_STEP_END);
#else
		return S_FOOTS_ROAD_START;
#endif
	}

	switch(num=person_is_on(p_person))
	{

		case PERSON_ON_DUNNO:
		case PERSON_ON_PRIM:
			start = S_FOOTS_ROAD_START;
			end   = S_FOOTS_ROAD_END;
			break;
#ifndef	PSX
		case PERSON_ON_WATER:
			start = S_FOOTS_PUDDLE_START;
			end   = S_FOOTS_PUDDLE_END;
			break;

		case PERSON_ON_SEWATER:
			start = S_FOOTS_SEWER_START;
			end   = S_FOOTS_SEWER_END;
			break;
#endif
		case PERSON_ON_METAL:
			start = S_FOOTS_RUNG_START;
			end	  = S_FOOTS_RUNG_END;
			break;
/*
		case PERSON_ON_PRIM:
			start = S_FOOTS_CAR_START;
			end   = S_FOOTS_CAR_END;
			break;
*/
#ifndef	PSX
		case PERSON_ON_GRAVEL:
//		case PERSON_ON_PRIM:
			start = S_FOOTS_GRAVEL_START;
			end   = S_FOOTS_GRAVEL_END;
			break;

		case PERSON_ON_WOOD:
			start = S_FOOTS_WOOD_START;
			end   = S_FOOTS_WOOD_END;
			break;

		case PERSON_ON_GRASS:
			start = S_FOOTS_GRASS_START;
			end   = S_FOOTS_GRASS_END;
			break;
#endif

		default:
#ifndef PSX
			if (num==0xff) num=0;
			if (num<=0) { // we have a specific index. woo
			  start = SOUND_FXGroups[-num][0];
			  end   = SOUND_FXGroups[-num][1];
			  //TRACE("footstep range %d to %d, num was %d\n",start,end,num);
			} else {
				ASSERT(0);
			}
#else
			start = S_FOOTS_ROAD_START;
			end   = S_FOOTS_ROAD_END;
#endif
			break;
	}
#ifndef PSX
	return SOUND_Range(start,end);
#else
	return start;
#endif

/*	num = end - start + 1;

	return (which++ % num) + start;*/
}

//
// Returns the relative angle of the target from the person.
//

SLONG get_dangle(Thing *p_person, Thing *p_target)
{
	SLONG dx;
	SLONG dz;
	SLONG angle;
	SLONG dangle;

	dx = p_target->WorldPos.X - p_person->WorldPos.X >> 8;
	dz = p_target->WorldPos.Z - p_person->WorldPos.Z >> 8;

	angle   = Arctan(dx,-dz) + 1024;
	angle  &= 2047;
	dangle  = angle - p_person->Draw.Tweened->Angle;
	dangle &= 2047;

	return dangle;
}



//---------------------------------------------------------------

SLONG get_fence_bottom(SLONG x, SLONG z, SLONG col)
{
	SLONG fy;

	if (dfacets[col].FacetFlags & FACET_FLAG_ONBUILDING||dfacets[col].FacetType==STOREY_TYPE_NORMAL)
	{
		fy = dfacets[col].Y[0];
	}
	else
	{
		fy = PAP_calc_height_at(x,z);
	}

	return fy;	// + 20?
}

SLONG get_fence_top(SLONG x, SLONG z, SLONG col)
{
	SLONG fy;
	SLONG fheight;
	SLONG ftop;

	if (dfacets[col].FacetFlags & FACET_FLAG_ONBUILDING ||dfacets[col].FacetType==STOREY_TYPE_NORMAL)
	{
		fy = dfacets[col].Y[0];
	}
	else
	{
		fy = PAP_calc_height_at(x,z);
	}

	fheight = dfacets[col].Height * 64;
	ftop    = fy + fheight;

	if (fheight == 0x100)
	{
		//
		// You can jump over 1 high unclimbable fences. Pretend they
		// are slightly higher!
		//

		if (dfacets[col].FacetFlags & FACET_FLAG_UNCLIMBABLE)
		{
			ftop += 0x40;
		}
	}

	return ftop;
}



//
// Makes a person create a splash.
//

void person_splash(
		Thing *p_person,
		SLONG  limb)	// -1 => Splash at the centre of the person.
{

#ifndef	PSX

	SLONG i,type;

	SLONG foot_x;
	SLONG foot_y;
	SLONG foot_z;

	SLONG track_x,track_y,track_z,dx,dy,dz;

	SLONG splash_x;
	SLONG splash_y;
	SLONG splash_z;


	if (limb == -1)
	{
		track_x = 0;
		track_y = 0;
		track_z = 0;
	}
	else
	{
		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			limb,
		   &track_x,
		   &track_y,
		   &track_z);
	}

	foot_x = track_x + (p_person->WorldPos.X >> 8);
	foot_y = track_y + (p_person->WorldPos.Y >> 8);
	foot_z = track_z + (p_person->WorldPos.Z >> 8);

#ifndef TARGET_DC
	for (i = 0; i < 1; i++)
	{
		splash_x = foot_x + (Random() & 0x1f) - 0xf;
		splash_y = foot_y;
		splash_z = foot_z + (Random() & 0x1f) - 0xf;

		DRIP_create_if_in_puddle(
			splash_x,
			splash_y,
			splash_z);
	}

	PUDDLE_splash(
		foot_x,
		foot_y,
		foot_z);
#endif

	/*
	if (GAME_FLAGS & GF_SEWERS) {
		if ((p_person->Flags & FLAGS_IN_SEWERS) && (p_person->State != STATE_IDLE)) {
			if (person_is_on(p_person)==PERSON_ON_WATER)
			  DRIP_create(foot_x,foot_y,foot_z);
		}
	}
	*/


	// Add footprints
	float yaw,pitch,roll;
	float matrix[9], vector[3];
	static int heh_heh_heh=-1;


    if (limb==heh_heh_heh) return;
	heh_heh_heh=limb;

	yaw   = -float(p_person->Draw.Tweened->Angle) * (2.0F * PI / 2048.0F);
	pitch = -float(p_person->Draw.Tweened->Tilt)  * (2.0F * PI / 2048.0F);
	roll  = -float(p_person->Draw.Tweened->Roll)  * (2.0F * PI / 2048.0F);

	MATRIX_calc(matrix, yaw, pitch, roll);
	vector[2]=20; vector[1]=0; vector[0]=0; 
	MATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
	dx=vector[0]; dy=vector[1]; dz=vector[2];

	if ((limb==SUB_OBJECT_RIGHT_FOOT)||(limb==-1)) {
		type=TRACK_TYPE_RIGHT_PRINT;

		// don't ask...
		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			SUB_OBJECT_RIGHT_FOOT,
		   &track_x,
		   &track_y,
		   &track_z);
		track_x<<=8;
		track_y<<=8;
		track_z<<=8;
		track_x+=p_person->WorldPos.X;
		track_z+=p_person->WorldPos.Z;

		track_y=(PAP_calc_height_at_thing(p_person,track_x>>8,track_z>>8)<<8)+0x180;
		
		{
			SLONG	mx,mz;

			mx=(track_x-(dx*0x1ff))>>16;
			mz=(track_z-(dz*0x1ff))>>16;
			if(mx>=0 && mz<128 && mz>=0 && mz<128)
				if(!(MAV_SPARE(mx ,mz) & MAV_SPARE_FLAG_WATER))
					p_person->Genus.Person->muckyfootprint=TRACKS_Add(track_x-(dx*0x1ff),track_y-(dy*0x1ff),track_z-(dz*0x1ff),dx*2,dy*2,dz*2,type,p_person->Genus.Person->muckyfootprint);
		}
	}
	if ((limb==SUB_OBJECT_LEFT_FOOT)||(limb==-1)) {
		type=TRACK_TYPE_LEFT_PRINT;

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			SUB_OBJECT_LEFT_FOOT,
		   &track_x,
		   &track_y,
		   &track_z);
		track_x<<=8;
		track_y<<=8;
		track_z<<=8;
		track_x+=p_person->WorldPos.X;
		track_z+=p_person->WorldPos.Z;
		track_y=(PAP_calc_height_at_thing(p_person,track_x>>8,track_z>>8)<<8)+0x180;

		{
			SLONG	mx,mz;

			mx=(track_x-(dx*0x1ff))>>16;
			mz=(track_z-(dz*0x1ff))>>16;
			if(mx>=0 && mz<128 && mz>=0 && mz<128)
				if(!(MAV_SPARE(mx ,mz) & MAV_SPARE_FLAG_WATER))
					p_person->Genus.Person->muckyfootprint=TRACKS_Add(track_x-(dx*0x1ff),track_y-(dy*0x1ff),track_z-(dz*0x1ff),dx*2,dy*2,dz*2,type,p_person->Genus.Person->muckyfootprint);
		}
	}
#endif
}


//
// Sets a person PersonID to the correct value for that person.  It looks
// to see if they have a gun/item in use etc...
//

void set_persons_personid(Thing *p_person)
{
	Thing *p_special;

	if(p_person->Genus.Person->PersonType==PERSON_CIV)
		return;

	p_person->Draw.Tweened->PersonID &= 0x1f;

	//
	// Has this person got a pistol out?
	//

	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		p_person->Draw.Tweened->PersonID |= 1<<5;

		return;
	}

	//
	// Using an item?
	//

	if (p_person->Genus.Person->SpecialUse)
	{
		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch(p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_NONE:
			case SPECIAL_KEY:
				break;

			case SPECIAL_GUN:
				p_person->Draw.Tweened->PersonID |= 1<<5;
				break;

			case SPECIAL_HEALTH:
			case SPECIAL_BOMB:
				break;

			case SPECIAL_SHOTGUN:
				p_person->Draw.Tweened->PersonID |= 3<<5;
				break;

			case SPECIAL_KNIFE:
				p_person->Draw.Tweened->PersonID |= 2<<5;
				break;

			case SPECIAL_EXPLOSIVES:
			case SPECIAL_GRENADE:
				break;

			case SPECIAL_AK47:
				p_person->Draw.Tweened->PersonID |= 5<<5;
				break;

			case SPECIAL_BASEBALLBAT:
				p_person->Draw.Tweened->PersonID |= 4<<5;
				break;

			default:
				break;
		}
	}
}


//---------------------------------------------------------------
//---------------------------------------------------------------

//#define SHOW_ANIM_NUMS

// hopefully the compiler's smart enough (heh) to completely optimise out an empty inline function... :P

inline void ShowAnimNumber(SLONG anim) {

#ifdef SHOW_ANIM_NUMS

	CBYTE str[100];

	sprintf(str,"Anim %d",anim);

	CONSOLE_text(str,4000);

#endif
}

void	queue_anim(Thing *p_person,SLONG anim)
{
	ASSERT(anim!=1);
//	ASSERT(anim!=54);
	ASSERT(((ULONG)global_anim_array[p_person->Genus.Person->AnimType][anim])>1000);
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	if(p_person->Genus.Person->Flags&FLAG_PERSON_LOCK_ANIM_CHANGE)
	{
		locked_anim_change(p_person,0,anim);
		p_person->Genus.Person->Flags&=~FLAG_PERSON_LOCK_ANIM_CHANGE;
	}
	else
	{
		p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][anim];
	}
	p_person->Draw.Tweened->CurrentAnim	=	anim;
	p_person->Draw.Tweened->FrameIndex=0;
	ASSERT(p_person->Draw.Tweened->CurrentFrame->FirstElement);
	if(p_person->Draw.Tweened->NextFrame)
		ASSERT(p_person->Draw.Tweened->NextFrame->FirstElement);

	ShowAnimNumber(anim);
}

void	tween_to_anim(Thing *p_person,SLONG anim)
{
//	ASSERT(anim!=54);
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	if(p_person->Genus.Person->Flags&FLAG_PERSON_LOCK_ANIM_CHANGE)
	{
		locked_anim_change(p_person,0,anim);
		p_person->Genus.Person->Flags&=~FLAG_PERSON_LOCK_ANIM_CHANGE;
	}
	else
	{
		p_person->Draw.Tweened->NextFrame	=	global_anim_array[p_person->Genus.Person->AnimType][anim];
		p_person->Draw.Tweened->QueuedFrame	=	0;
	}

	p_person->Draw.Tweened->CurrentAnim	=	anim;
	p_person->Draw.Tweened->FrameIndex=0;
	ASSERT(p_person->Draw.Tweened->CurrentFrame->FirstElement);
	if(p_person->Draw.Tweened->NextFrame)
		ASSERT(p_person->Draw.Tweened->NextFrame->FirstElement);

	ShowAnimNumber(anim);
}

void	set_anim(Thing *p_person,SLONG anim)
{
	ASSERT(anim!=1);
//	ASSERT(anim!=54);
/*
	if(p_person->Genus.Person->PlayerID==0)
	{
		CBYTE	str[100];
		sprintf(str," fight %d 
	}
*/
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;

	if(p_person->Genus.Person->Flags&FLAG_PERSON_LOCK_ANIM_CHANGE)
	{
		locked_anim_change(p_person,0,anim);
		p_person->Genus.Person->Flags&=~FLAG_PERSON_LOCK_ANIM_CHANGE;
	}
	else
	{
		p_person->Draw.Tweened->AnimTween	 =	0;
		p_person->Draw.Tweened->QueuedFrame	 =	0;
		p_person->Draw.Tweened->CurrentFrame =	global_anim_array[p_person->Genus.Person->AnimType][anim];
		p_person->Draw.Tweened->NextFrame    =	global_anim_array[p_person->Genus.Person->AnimType][anim]->NextFrame;
		p_person->Draw.Tweened->Locked       =  0;
	}
	p_person->Draw.Tweened->FrameIndex=0;
	p_person->Draw.Tweened->CurrentAnim	=	anim;
	ASSERT(p_person->Draw.Tweened->CurrentFrame->FirstElement);
	if(p_person->Draw.Tweened->NextFrame)
		ASSERT(p_person->Draw.Tweened->NextFrame->FirstElement);

	ShowAnimNumber(anim);

}
//				set_anim_of_type(p_person,CIV_M_ANIM_WALK,ANIM_TYPE_CIV);
void	set_anim_of_type(Thing *p_person,SLONG anim,SLONG type)
{
	ASSERT(anim!=1);
//	ASSERT(anim!=54);
/*
	if(p_person->Genus.Person->PlayerID==0)
	{
		CBYTE	str[100];
		sprintf(str," fight %d 
	}
*/
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	p_person->Genus.Person->Flags&=~FLAG_PERSON_LOCK_ANIM_CHANGE;
/*
	if(p_person->Genus.Person->Flags&FLAG_PERSON_LOCK_ANIM_CHANGE)
	{
		locked_anim_change(p_person,0,anim);
		p_person->Genus.Person->Flags&=~FLAG_PERSON_LOCK_ANIM_CHANGE;
	}
	else
*/
	{
		p_person->Draw.Tweened->AnimTween	 =	0;
		p_person->Draw.Tweened->QueuedFrame	 =	0;
//		p_person->Draw.Tweened->CurrentFrame =	global_anim_array[type][anim];
//		p_person->Draw.Tweened->NextFrame    =	global_anim_array[type][anim]->NextFrame;
		p_person->Draw.Tweened->CurrentFrame =	game_chunk[type].AnimList[anim];
		p_person->Draw.Tweened->NextFrame    =	game_chunk[type].AnimList[anim]->NextFrame;
	}
	p_person->Draw.Tweened->FrameIndex=0;
	p_person->Draw.Tweened->CurrentAnim	=	anim;
	ASSERT(p_person->Draw.Tweened->CurrentFrame->FirstElement);
	if(p_person->Draw.Tweened->NextFrame)
		ASSERT(p_person->Draw.Tweened->NextFrame->FirstElement);

	ShowAnimNumber(anim);

}

void	set_locked_anim(Thing *p_person,SLONG anim,SLONG sub_object)
{
	ASSERT(anim!=1);
	ASSERT(anim);
//	ASSERT(anim!=54);
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	locked_anim_change(p_person,sub_object,anim);
	p_person->Draw.Tweened->AnimTween	 =	0;
	p_person->Draw.Tweened->QueuedFrame	 =	0;
	p_person->Draw.Tweened->CurrentFrame =	global_anim_array[p_person->Genus.Person->AnimType][anim];
	p_person->Draw.Tweened->NextFrame    =	global_anim_array[p_person->Genus.Person->AnimType][anim]->NextFrame;
	p_person->Draw.Tweened->CurrentAnim	=	anim;
	p_person->Draw.Tweened->FrameIndex=0;

	ShowAnimNumber(anim);

}
void	set_locked_anim_angle(Thing *p_person,SLONG anim,SLONG sub_object,SLONG dangle)
{
	ASSERT(anim!=1);
	ASSERT(anim);
//	ASSERT(anim!=54);
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	locked_anim_change(p_person,sub_object,anim,dangle);
	p_person->Draw.Tweened->AnimTween	 =	0;
	p_person->Draw.Tweened->QueuedFrame	 =	0;
	p_person->Draw.Tweened->CurrentFrame =	global_anim_array[p_person->Genus.Person->AnimType][anim];
	p_person->Draw.Tweened->NextFrame    =	global_anim_array[p_person->Genus.Person->AnimType][anim]->NextFrame;
	p_person->Draw.Tweened->CurrentAnim	=	anim;
	p_person->Draw.Tweened->FrameIndex=0;

	ShowAnimNumber(anim);

}
void	set_locked_anim_of_type(Thing *p_person,SLONG anim,SLONG sub_object)
{
	ASSERT(anim!=1);
	ASSERT(anim);
//	ASSERT(anim!=54);
	p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	locked_anim_change(p_person,sub_object,anim);
	p_person->Draw.Tweened->AnimTween	 =	0;
	p_person->Draw.Tweened->QueuedFrame	 =	0;
	p_person->Draw.Tweened->CurrentFrame =	game_chunk[p_person->Genus.Person->AnimType].AnimList[anim];
	p_person->Draw.Tweened->NextFrame    =	game_chunk[p_person->Genus.Person->AnimType].AnimList[anim]->NextFrame;
	p_person->Draw.Tweened->CurrentAnim	=	anim;
	p_person->Draw.Tweened->FrameIndex=0;

	ShowAnimNumber(anim);

}

SLONG get_cable_sdist_from_end(SLONG facet,SLONG ax,SLONG az)
{
	SLONG dx;
	SLONG dz;

	SLONG x1;
	SLONG z1;


	SLONG	d1;

	struct	DFacet	*p_facet;

	p_facet=&dfacets[facet];




	if(p_facet->Y[0] >p_facet->Y[1])
	{
		x1=p_facet->x[1] << 8;
		z1=p_facet->z[1] << 8;

		dx = x1 - ax;
		dz = z1 - az;

		d1=dx*dx+dz*dz;
	}
	else
	{
		x1=p_facet->x[0] << 8;
		z1=p_facet->z[0] << 8;

		dx = x1 - ax;
		dz = z1 - az;

		d1=dx*dx+dz*dz;
	}

	return(d1);
}

void	person_death_slide(Thing *p_person)
{
	SLONG	dx,dy=0,dz;
	SLONG	along;
	SLONG	dist,sdist;

	GameCoord new_position = p_person->WorldPos;

	dist=p_person->Velocity >> 2;

	dx	=-	(SIN(p_person->Draw.Tweened->Angle)*dist)>>8;
	dz	=-	(COS(p_person->Draw.Tweened->Angle)*dist)>>8;
	dx = (dx*TICK_RATIO)>>TICK_SHIFT;
	dz = (dz*TICK_RATIO)>>TICK_SHIFT;

	along = get_cable_along(p_person->Genus.Person->OnFacet,(p_person->WorldPos.X>>8)+(dx>>8),(p_person->WorldPos.Z>>8)+(dz>>8));
	sdist = get_cable_sdist_from_end(p_person->Genus.Person->OnFacet,(p_person->WorldPos.X>>8)+(dx>>8),(p_person->WorldPos.Z>>8)+(dz>>8));

	#define CABLE_START (1   * CABLE_ALONG_MAX >> 8)
	#define CABLE_END   (245 * CABLE_ALONG_MAX >> 8)

	//
	// only really needs the new sdist method, but keep old one for backwards compatability
	//
	if (WITHIN(along, CABLE_START, CABLE_END) && sdist>64*64)
	{
		//
		// Still on the cable.
		//

		MFX_play_ambient(THING_NUMBER(p_person), S_ZIPWIRE, MFX_LOOPED | MFX_NEVER_OVERLAP);
		MFX_set_pitch(THING_NUMBER(p_person), S_ZIPWIRE, 224 + (p_person->Velocity / 4));

		new_position.X += dx;
		new_position.Z += dz;
		move_thing_on_map(p_person, &new_position);

		do_person_on_cable(p_person);
	}
	else
	{
		MFX_stop(THING_NUMBER(p_person), S_ZIPWIRE);

		set_person_drop_down(p_person,PERSON_DROP_DOWN_OFF_FACE);
		
		return;

		//p_person->SubState=SUB_STATE_DANGLING_CABLE;
		//p_person->Velocity=0;
		//p_person->DeltaVelocity=0;
		//p_person->Genus.Person->Action=ACTION_DANGLING_CABLE;
		//locked_anim_change(p_person,SUB_OBJECT_LEFT_HAND,ANIM_HAND_OVER_HAND);
	}
}

/*

//
// big drop
//
void	set_person_dead_land(Thing *p_thing)
{
	DrawTween	*draw_info;
	WaveParams	die;

	
	draw_info=p_thing->Draw.Tweened;
	draw_info->CurrentFrame		=	global_anim_array[p_thing->Genus.Person->AnimType][ANIM_FATAL_FALL];

	//	Set up the wave params.
	die.Priority				=	0;
	die.Flags					=	WAVE_CARTESIAN;
	die.Mode.Cartesian.Scale	=	(128<<8);
	die.Mode.Cartesian.X		=	p_thing->WorldPos.X;
	die.Mode.Cartesian.Y		=	p_thing->WorldPos.Y;
	die.Mode.Cartesian.Z		=	p_thing->WorldPos.Z;

	switch(p_thing->Genus.Person->AnimType)
	{
		case	PERSON_COP:
//			PlayWave(THING_NUMBER(p_thing),S_MALE_DIE_2,WAVE_PLAY_NO_INTERUPT,&die);
			play_quick_wave_xyz(p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z,
				S_MALE_DIE_2,0,0);
			break;
		case	PERSON_DARCI:
//			PlayWave(THING_NUMBER(p_thing),S_FEMALE_DIE_2,WAVE_PLAY_NO_INTERUPT,&die);
			play_quick_wave_xyz(p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z,
				S_FEMALE_DIE_2,0,0);
			break;
	}
	set_person_dying(p_thing);
	draw_info->NextFrame		=	draw_info->CurrentFrame->NextFrame;
	draw_info->QueuedFrame		=	0; //draw_info->CurrentFrame->NextFrame;
	draw_info->AnimTween		=	0;
	draw_info->CurrentAnim	=	ANIM_KO_BACK;
}

*/


void emergency_uncarry(Thing *p_person);
SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor);

void sweep_feet(Thing *p_person,Thing *p_aggressor,SLONG  death_type)
{
	SlideSoundCheck(p_person,1);

	if (p_person->State == STATE_JUMPING)
	{
		return;
	}

	if (p_person->State == STATE_DANGLING)
	{
		if (p_person->SubState == SUB_STATE_DROP_DOWN)
		{
			return;
		}
	}

	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		emergency_uncarry(p_person);
	}

	if(!people_allowed_to_hit_each_other(p_person,p_aggressor))
	{
		//
		// Your chums jump to avoid your attack
		//

		set_person_standing_jump(p_person);

		return;

	}

	if (PersonIsMIB(p_person))
	{
		//
		// Men In Black jump to avoid the attack like ninjas!
		//

		set_person_standing_jump(p_person);

		return;
	}

	if (PersonIsMIB(p_person))
	{

	}


	if (p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST && (p_person->Genus.Person->pcom_ai_other & PCOM_COMBAT_SLIDE))
	{
		//
		// Fight test dummies who are set to die when their feet are
		// swept die even if they are invulnerable...
		//

		p_person->Genus.Person->Health = 0;
	}
	else
	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
	{
		//
		// Invulnerable people jump.
		//

		set_person_standing_jump(p_person);

		return;
	}

	{
		SLONG	r,prob;
		prob=40+(GET_SKILL(p_person)<<3);
		if((r=(Random()%160))<prob)
		{
			//
			//
			// Somone tried to slide attack you
			if(can_a_see_b(p_person,p_aggressor,-1,1))
			{
				PCOM_attack_happened(p_person, p_aggressor);
				set_person_standing_jump(p_person);

				return;
			}

		}
/*		else
		{
			TRACE(" %d !<%d\n",r,prob);
		}
*/
	}



	if ((p_person->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL) && !p_aggressor->Genus.Person->PlayerID)
	{
		//
		// Only the player can hurt this person
		//
	}
	else
	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
	{
		//
		// Nothing hurts this person.
		//
	}
	else
	{
		if(!p_person->Genus.Person->PlayerID)
			p_person->Genus.Person->Health -= 49;
#ifdef PSX
		if (p_person->Genus.Person->PlayerID)
		{
			PSX_SetShock(1,128);
		}
#endif
#ifdef TARGET_DC
		if (p_person->Genus.Person->PlayerID)
		{
			Vibrate ( 20.0f, 1.0f, 0.25f );
		}
#endif
//		add_damage_value_thing(p_person,49>>1);
	}

	if (p_person->Genus.Person->Health <= 0)
	{
		p_person->Genus.Person->Health = 0;
		death_type                     = PERSON_DEATH_TYPE_OTHER;

		if (p_aggressor)
		{
			p_aggressor->Genus.Person->Flags2 |= FLAG2_PERSON_IS_MURDERER;
		}
	}
	else
	{
		death_type = PERSON_DEATH_TYPE_LEG_SWEEP;
	}

	//
	// Tell the AI system whats going on.
	//

	PCOM_attack_happened(p_person, p_aggressor);

	set_person_dead(p_person,p_aggressor,death_type,0,0);
}

//
// Returns TRUE if there is room behind the person for him to die
// in the given direction.
// 

SLONG is_there_room_behind_person(Thing *p_person, SLONG hit_from_behind)
{
	ULONG los_flag;

	//
	// A vector in the direction this person is going to die in.
	// 

	SLONG dx = SIN(p_person->Draw.Tweened->Angle);
	SLONG dz = COS(p_person->Draw.Tweened->Angle);

	if (hit_from_behind)
	{
		dx = -dx;
		dz = -dz;
	}

	SLONG h1 = PAP_calc_map_height_at(
					p_person->WorldPos.X >> 8,
					p_person->WorldPos.Z >> 8);

	SLONG h2 = PAP_calc_map_height_at(
					p_person->WorldPos.X + dx >> 8,
					p_person->WorldPos.Z + dz >> 8);

	if (abs(h2 - h1) > 0x40)
	{
		//
		// No room to die this way round.
		//

		return FALSE;
	}

	//
	// Make sure there is enough room in this direction.
	//

	los_flag = LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG | LOS_FLAG_INCLUDE_CARS;

	if (p_person->Genus.Person->Ware)
	{
		//
		// Don't do the underground check if the person is in a warehouse.
		//

		los_flag |= LOS_FLAG_IGNORE_UNDERGROUND_CHECK;
	}

	/*

	AENG_world_line(
			(p_person->WorldPos.X          >> 8),
			(p_person->WorldPos.Y + 0x3000 >> 8),
			(p_person->WorldPos.Z          >> 8),
			20,
			0xffffff,
			(p_person->WorldPos.X + dx     >> 8),
			(p_person->WorldPos.Y + 0x3000 >> 8),
			(p_person->WorldPos.Z + dz     >> 8),
			0,
			0,
			TRUE);
	
	*/

	if (!there_is_a_los(
			(p_person->WorldPos.X          >> 8),
			(p_person->WorldPos.Y + 0x3000 >> 8),
			(p_person->WorldPos.Z          >> 8),
			(p_person->WorldPos.X + dx     >> 8),
			(p_person->WorldPos.Y + 0x3000 >> 8),
			(p_person->WorldPos.Z + dz     >> 8),
			los_flag))	// No underground check incase this is inside a warehouse...
	{
		//
		// No room to die this way round.
		//

		return FALSE;
	}

	return TRUE;
}


extern	SLONG slide_along(SLONG  x1, SLONG  y1, SLONG  z1,SLONG *x2, SLONG *y2, SLONG *z2,SLONG  extra_wall_height,SLONG  radius,ULONG  flags);
void	set_fence_hole(struct DFacet *p_facet,SLONG pos);

SLONG	get_along_facet(SLONG	x,SLONG z,SLONG colvect)
{
	DFacet	*p_facet;
	SLONG	dx,dz;
	SLONG	along;
	
	p_facet=&dfacets[colvect];

	dx=p_facet->x[1]-p_facet->x[0];
	dz=p_facet->z[1]-p_facet->z[0];

	if(dx)
	{
		along=x-(p_facet->x[0]<<8);
		along=along/dx;    
	}
	else
	{
		along=z-(p_facet->z[0]<<8);
		along=along/dz;    
	}

	return(along);
}
#ifdef	UNUSED_WIRECUTTERS
extern	UWORD	fence_colvect;
SLONG	set_person_cut_fence(Thing *p_person)
{
	SLONG	x1,y1,z1,x2,y2,z2,dx,dz;
	SLONG	along;


	dx	=-	(SIN(p_person->Draw.Tweened->Angle)*50)>>8;
	dz	=-	(COS(p_person->Draw.Tweened->Angle)*50)>>8;

	x1=p_person->WorldPos.X;
	y1=p_person->WorldPos.Y;
	z1=p_person->WorldPos.Z;

	x2=x1+dx;
	z2=z1+dz;
	y2=y1;


	slide_along(x1,y1,z1,&x2,&y2,&z2,0,50,0);

	if(fence_colvect)
	{
		// we have hit a fence colvect

		along=get_along_facet(x1>>8,z1>>8,fence_colvect);

		if(along>0&&along<255)
		{

			set_fence_hole(&dfacets[fence_colvect],along);
			set_person_croutch(p_person);
			return(1);

		}
	}
	return(0);
}
#endif
void set_person_dead(
		Thing *p_thing,
		Thing *p_aggressor,
		SLONG  death_type,
		SLONG  behind,
		SLONG  height)
{
	DrawTween  *draw_info;
#ifndef PSX
	WaveParams  die;
#endif
	SLONG		anim;
	SLONG       substate;
	SLONG       quick  = FALSE;
	SLONG       locked = FALSE;

	ASSERT(p_thing->Class == CLASS_PERSON);
	p_thing->Draw.Tweened->Roll	 = 0;
	p_thing->Draw.Tweened->DRoll = 0;

	p_thing->Genus.Person->InsideRoom=0;

	if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		emergency_uncarry(p_thing);
	}


//	ASSERT(is_there_room_behind_person(p_thing, behind));

	#ifdef BIKE

	//
	// If you've come off a bike...
	//

	if (p_thing->Genus.Person->Flags & FLAG_PERSON_BIKING)
	{
		//
		// Knocked off your bike...
		//
	
		BIKE_set_parked(TO_THING(p_thing->Genus.Person->InCar));

		p_thing->Genus.Person->Flags &= ~FLAG_PERSON_BIKING;
		p_thing->Genus.Person->InCar  =  0;
	}

	#endif

	//
	// Tell the PCOM brain what has happened.
	//

	if (p_thing->Genus.Person->PlayerID==0)
	{
		//
		// Non players
		//

		PCOM_knockdown_happened(p_thing);
	}

	if (p_aggressor && p_aggressor->Genus.Person->PlayerID)
	{
		if (death_type != PERSON_DEATH_TYPE_LEG_SWEEP  &&
			death_type != PERSON_DEATH_TYPE_STAY_ALIVE &&
			death_type != PERSON_DEATH_TYPE_GET_DOWN &&
			death_type != PERSON_DEATH_TYPE_STAY_ALIVE_PRONE)
		{
			//
			// The player caused the death...
			//

				switch(p_thing->Genus.Person->PersonType)
				{
					case PERSON_THUG_RASTA:
					case PERSON_THUG_GREY:
					case PERSON_THUG_RED:
					case PERSON_MIB1:
					case PERSON_MIB2:
					case PERSON_MIB3:
						stat_killed_thug++;
						break;
					default:
						stat_killed_innocent++;
						break;

				}


			if ((p_thing->Genus.Person->pcom_ai == PCOM_AI_CIV && p_thing->Genus.Person->pcom_move == PCOM_MOVE_WANDER) ||
				(p_thing->Genus.Person->PersonType == PERSON_COP))
			{
				if 	(!(p_thing->Genus.Person->Flags2 & FLAG2_PERSON_GUILTY))
				{
					//
					// ...of an innocent!
					//

					NET_PLAYER(0)->Genus.Player->RedMarks += 1;
				}
			}
		}
	}

	if (p_thing->SubState == SUB_STATE_GRAPPLE_HELD)
	{
		//
		// set person holding me to fight idle
		//

		if(TO_THING(p_thing->Genus.Person->Target)->SubState!=SUB_STATE_GRAPPLE_ATTACK)
		{
			set_person_fight_idle(TO_THING(p_thing->Genus.Person->Target));
		}
	}

	if(death_type!=PERSON_DEATH_TYPE_STAY_ALIVE && death_type!=PERSON_DEATH_TYPE_STAY_ALIVE_PRONE &&death_type!=PERSON_DEATH_TYPE_LEG_SWEEP)
	{
		if(p_thing->Genus.Person->Target)
		{
			remove_from_gang_attack(p_thing,TO_THING(p_thing->Genus.Person->Target));
		}
		if(p_aggressor)
		{
			remove_from_gang_attack(p_thing,p_aggressor);
			if(p_aggressor->Genus.Person->Target==THING_NUMBER(p_thing))
			{
				//ASSERT(0);
				extern	UWORD	find_target_from_gang(Thing *p_target);
				p_aggressor->Genus.Person->Target=find_target_from_gang(p_aggressor);
				if(p_aggressor->Genus.Person->PlayerID)
				if(p_aggressor->Genus.Person->Target==0)
				{
					//
					// nobody is attacking me so leave fight mode
					//
					p_aggressor->Genus.Person->Mode=PERSON_MODE_RUN;
				}

			}
		}
	}

	if(p_thing->SubState==SUB_STATE_GRAPPLE_HOLD||p_thing->SubState==SUB_STATE_GRAPPLE_ATTACK)
	{
		//
		// die while grappleing, hmmmmmm
		//

		//
		// better let go of person we are grappelling
		//

		set_person_fight_idle(TO_THING(p_thing->Genus.Person->Target));
//		return;
	}

	#define MAX_KNOCK_DOWN 6

	static UBYTE knock_down[MAX_KNOCK_DOWN] =
	{
		ANIM_KD_FRONT_LOW,
		ANIM_KD_FRONT_MID,
		ANIM_KD_FRONT_HI,
		ANIM_KD_BACK_LOW,
		ANIM_KD_BACK_MID,
		ANIM_KD_BACK_HI
	};
	
	ASSERT(p_thing->Class == CLASS_PERSON);

	//
	// Ignore people who are already dead.
	//

	if(death_type!=PERSON_DEATH_TYPE_PRONE && death_type!=PERSON_DEATH_TYPE_COMBAT_PRONE)
	{
		if (p_thing->State == STATE_DEAD ||
			p_thing->State == STATE_DYING)
		{
			return;
		}
	}

	if (p_thing->State    == STATE_DYING &&
		p_thing->SubState == SUB_STATE_DYING_FINAL_ANI)
	{
		//
		// As good as dead already...
		//

		return;
	}

	//
	// Which anim shall we do?
	//

	switch(death_type)
	{
		case PERSON_DEATH_TYPE_PRONE:

			//
			// Already lay on floor.
			//

			substate                      =   SUB_STATE_DYING_ACTUALLY_DIE;
			p_thing->Genus.Person->Flags &= ~(FLAG_PERSON_KO | FLAG_PERSON_HELPLESS);

			//
			// If (quick) then we don't set an anim or play
			// a sound effect.
			// 

			quick = TRUE;

			break;

		case PERSON_DEATH_TYPE_COMBAT_PRONE:

			//
			// Person already lay down 
			//

			substate                      =   SUB_STATE_DYING_FINAL_ANI;
			p_thing->Genus.Person->Flags &= ~(FLAG_PERSON_KO | FLAG_PERSON_HELPLESS);

			//
			// We must do a locked anim change.
			//

			locked = TRUE;

			//
			// Is this person on their front or back?
			//

			switch(person_is_lying_on_what(p_thing))
			{
				case PERSON_ON_HIS_FRONT:
					anim  =  ANIM_FIGHT_STOMPED_BACK;
					break;

				case PERSON_ON_HIS_BACK:
					anim  =  ANIM_FIGHT_STOMPED_FRONT;
					break;

				default:
					ASSERT(0);
					break;
			}

			break;

		case PERSON_DEATH_TYPE_SHOT_PISTOL:
		case PERSON_DEATH_TYPE_SHOT_SHOTGUN:
		case PERSON_DEATH_TYPE_SHOT_AK47:

			{
				static UWORD shoot_dead_anim[3] =
				{
					ANIM_SHOT_DEAD_HEAD,
					ANIM_SHOT_DEAD_GUT,
					ANIM_SHOT_DEAD_CHEST,
				};
/*				
				if (!is_there_room_behind_person(p_thing, behind))
				{
					behind = !behind;
				}
*/
				if (!behind)
				{
					anim = ANIM_SHOT_DEAD_BACK;
				}
				else
				{
					anim = shoot_dead_anim[Random() % 3];
				}

				substate = SUB_STATE_DYING_FINAL_ANI;
				p_thing->Genus.Person->Flags&=~FLAG_PERSON_KO;
			}
			
			break;

		case PERSON_DEATH_TYPE_COMBAT:

			ASSERT(WITHIN(behind, 0, 1));
			ASSERT(WITHIN(height, 0, 2));

			ASSERT(WITHIN(behind*3+height, 0, MAX_KNOCK_DOWN - 1));

			//
			// Make sure that this person is not going to die 'through' a
			// building or a fence.
			//
/*
			if (!is_there_room_behind_person(p_thing, behind))
			{
				behind = !behind;
			}
*/

			anim     = knock_down[behind*3+height];
			substate = SUB_STATE_DYING_INITIAL_ANI;

			break;

		case PERSON_DEATH_TYPE_LAND:

			switch(p_thing->Draw.Tweened->CurrentAnim)
			{
				case ANIM_PLUNGE_START:
				case ANIM_PLUNGE_FORWARDS:
					anim                 = ANIM_PLUNGE_FRONT_SLAM;
//#ifndef VERSION_GERMAN
					if(VIOLENCE)
						PYRO_create(p_thing->WorldPos,PYRO_SPLATTERY);
//#endif
//					p_thing->WorldPos.Y += 0x8; // !
					break;

				default:
//					anim = ANIM_FATAL_FALL;
					anim = ANIM_BIGLAND_DIE;
					break;
			}

			StopScreamFallSound(p_thing); // just in case
			locked   = TRUE;
			substate = SUB_STATE_DYING_FINAL_ANI;

			break;

		case PERSON_DEATH_TYPE_OTHER:

			//
			// Is this person on the ground?
			//

			{
				SLONG ground;
				SLONG dy;
				
				ground = PAP_calc_height_at_thing(p_thing,p_thing->WorldPos.X >> 8, p_thing->WorldPos.Z >> 8);
				dy     = (p_thing->WorldPos.Y >> 8) - ground;

				if (dy < 0x20)
				{
					//
					// On the ground.
					//

					anim     = (behind) ? ANIM_KO_BEHIND_BIG : ANIM_KO_BACK;
					substate = SUB_STATE_DYING_FINAL_ANI;
				}
				else
				{
					//
					// In the air.
					// 

					anim     = ANIM_KD_FRONT_HI;
					substate = SUB_STATE_DYING_INITIAL_ANI;
				}
			}

			break;

		case PERSON_DEATH_TYPE_GET_DOWN:
			set_anim(p_thing,ANIM_HANDS_UP_LIE);
			p_thing->Genus.Person->Flags |= FLAG_PERSON_KO | FLAG_PERSON_HELPLESS;
			substate                      = SUB_STATE_DYING_KNOCK_DOWN;
//			substate                      = SUB_STATE_DYING_INITIAL_ANI;
			p_thing->Genus.Person->Timer1 = 0;

			//
			// If (quick) then we don't set an animation going or
			// play a sound effect
			//

			quick = TRUE;
			break;

		case PERSON_DEATH_TYPE_STAY_ALIVE_PRONE:

			p_thing->Genus.Person->Flags |= FLAG_PERSON_KO | FLAG_PERSON_HELPLESS;
			substate                      = SUB_STATE_DYING_KNOCK_DOWN;
			p_thing->Genus.Person->Timer1 = 0;

			//
			// If (quick) then we don't set an animation going or
			// play a sound effect
			//

			quick = TRUE;

			break;

		case PERSON_DEATH_TYPE_STAY_ALIVE:


			//
			// Is this person on the ground?
			//

			{
				SLONG ground;
				SLONG dy;
				
				ground = PAP_calc_height_at_thing(p_thing,p_thing->WorldPos.X >> 8, p_thing->WorldPos.Z >> 8);
				dy     = (p_thing->WorldPos.Y >> 8) - ground;

				p_thing->Genus.Person->Flags |= FLAG_PERSON_KO | FLAG_PERSON_HELPLESS;

				if (dy < 0x20)
				{
					//
					// On the ground.
					//

					substate = SUB_STATE_DYING_KNOCK_DOWN;
					anim     = (behind) ? ANIM_KO_BEHIND_BIG : ANIM_KO_BACK;
				}
				else
				{
					//
					// In the air.
					// 

					anim     = ANIM_KD_FRONT_HI;
					substate = SUB_STATE_DYING_INITIAL_ANI;
				}
			}


			break;

		case PERSON_DEATH_TYPE_LEG_SWEEP:
			anim                          = ANIM_KD_BACK_LOW;
			substate                      = SUB_STATE_DYING_INITIAL_ANI;
			p_thing->Genus.Person->Flags |= FLAG_PERSON_KO | FLAG_PERSON_HELPLESS;
			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Make the person do the correct animation for the height
	// at which the attack occured.
	//

	if(!quick)
	{
		if(locked)
		{
			set_locked_anim(p_thing,anim,0);
		}
		else
		{
			set_anim(p_thing,anim);
		}

		switch(p_thing->Genus.Person->PersonType)
		{
			case	PERSON_COP:
				MFX_play_xyz(0,S_MALE_DIE_2,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
				break;
			case	PERSON_DARCI:
				switch(death_type)
				{
					case HIT_TYPE_GUN_SHOT_H:
					case HIT_TYPE_GUN_SHOT_M:
					case HIT_TYPE_GUN_SHOT_L:
					default:
						MFX_play_xyz(0,S_FEMALE_DIE_2,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
						break;
				}
				break;
		}
	}

	//
	// Make the person die.
	//

	set_person_dying(p_thing, substate);

	//
	// If we know who killed us...
	//

	if (p_aggressor)
	{
		p_aggressor->Genus.Person->InWay = NULL;

		if(p_aggressor->Genus.Person->PlayerID)
		{
			//
			// Its a player so keep score
			//

			GAME_SCORE(p_aggressor->Genus.Person->PlayerID-1) += 50;
		}
	}
}

//
// 0 is not guilty, 1 is a bit guilty, 2 is very guilty , 3 is top ten most wanted
//
SLONG	is_person_guilty(Thing *p_person)
{
	if(p_person->Genus.Person->Flags2&FLAG2_PERSON_GUILTY)
		return(1);

	if(p_person->Genus.Person->PersonType == PERSON_THUG_RASTA ||
		p_person->Genus.Person->PersonType == PERSON_THUG_RED  ||
//		p_person->Genus.Person->PersonType == PERSON_TRAMP  ||
		p_person->Genus.Person->PersonType == PERSON_THUG_GREY)
	{
		return(2);
	}

/*	if(p_person->Genus.Person->PersonType == PERSON_MIB1)
		return(3);

	if(p_person->Genus.Person->PersonType == PERSON_MIB2)
		return(3);
	if(p_person->Genus.Person->PersonType == PERSON_MIB3)
		return(3);*/
	if (PersonIsMIB(p_person)) return 3;

	if (p_person->Genus.Person->pcom_ai == PCOM_AI_GUARD ||
		p_person->Genus.Person->pcom_ai == PCOM_AI_GANG)
	{
		return 1;
	}

	return(0);
}

SLONG	person_on_floor(Thing *p_person)
{
	if(p_person->SubState==SUB_STATE_RUNNING||p_person->SubState==SUB_STATE_WALKING||p_person->State==STATE_IDLE||p_person->State==STATE_CIRCLING||p_person->State==STATE_GUN||p_person->State==STATE_HIT_RECOIL)
	{
		return(1);
	}
	else
	{
		//
		// climbing ladders, jumping, doing stuff that may take you off the floor
		//
		return(0);
	}
}

SLONG	really_on_floor(Thing *p_person)
{
	SLONG foot_x;
	SLONG foot_y;
	SLONG foot_z;

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		SUB_OBJECT_LEFT_FOOT,	// 0 => foot
	   &foot_x,
	   &foot_y,
	   &foot_z);

	foot_x += p_person->WorldPos.X >> 8;
	foot_y += p_person->WorldPos.Y >> 8;
	foot_z += p_person->WorldPos.Z >> 8;

	SLONG ground = PAP_calc_map_height_at(foot_x, foot_z);

	if(foot_y>ground+32)
		return(0);
	else
		return(1);
	

}


SLONG	is_person_dead(Thing *p_person)
{
	if(p_person->State==STATE_DEAD || (p_person->State==STATE_DYING && (p_person->Genus.Person->Flags&FLAG_PERSON_KO)==0)) 
		return(1);
	else
		return(0);

}

SLONG	is_person_ko(Thing *p_person)
{
	if (p_person->State == STATE_DYING)
	{
		switch(p_person->SubState)
		{
			case SUB_STATE_DYING_KNOCK_DOWN_WAIT:
			case SUB_STATE_DYING_KNOCK_DOWN:	// An extra substate compared to the old version...
				return TRUE;
		}
	}

	return FALSE;
}

SLONG	is_person_ko_and_lay_down(Thing *p_person)
{
	if (p_person->State == STATE_DYING)
	{
		switch(p_person->SubState)
		{
			case SUB_STATE_DYING_KNOCK_DOWN_WAIT:
				return TRUE;
		}
	}

	return FALSE;
}

void knock_person_down(
		Thing *p_person,
		SLONG  hitpoints,
		SLONG  origin_x,
		SLONG  origin_z,
		Thing *p_aggressor)
{
	SLONG death_type;
	SLONG behind;

	//
	// Ignore other agreossors... the Balrog for instance!
	//

	if (p_aggressor && p_aggressor->Class != CLASS_PERSON)
	{
		p_aggressor = NULL;
	}

	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		//
		// being knocked down better drop who we are carrying
		//
		emergency_uncarry(p_person);
	}


	//
	// Look at what caused you to fall over.
	//

	set_face_pos(
		p_person,
		origin_x,
		origin_z);

	behind = FALSE;

	if ((p_person->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL) && (!p_aggressor || !p_aggressor->Genus.Person->PlayerID))
	{
		//
		// Only the player can hurt this person...
		//
	}
	else
	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
	{
		//
		// Nothing hurts this person.
		//
	}
	else
	{
		//
		// Hurt the person.
		//

		p_person->Genus.Person->Health -= hitpoints;
#ifdef PSX
		if (p_person->Genus.Person->PlayerID)
			PSX_SetShock(1,hitpoints+56);
#endif
#ifdef TARGET_DC
		if (p_person->Genus.Person->PlayerID)
		{
			Vibrate ( 4.0f, (float)( hitpoints+56 ) * 0.003f, 0.25f );
		}
#endif
//		add_damage_value_thing(p_person,hitpoints>>1);
	}

	if (p_person->Genus.Person->Health <= 0)
	{
		p_person->Genus.Person->Health = 0;
		death_type                     = PERSON_DEATH_TYPE_OTHER;

		if (p_aggressor)
		{
			p_aggressor->Genus.Person->Flags2 |= FLAG2_PERSON_IS_MURDERER;
		}
	}
	else
	{
		death_type = PERSON_DEATH_TYPE_STAY_ALIVE;
	}

	//
	// Knock the person to the floor.
	//
	if (!is_there_room_behind_person(p_person, behind))
	{
		behind = !behind;
	}

	set_person_dead(
		p_person,
		p_aggressor,
		death_type,
		behind,
		0);
}


void	person_bodge_forward(Thing *p_person,SLONG dist)
{
	SLONG	dx,dy=0,dz;
	GameCoord new_position = p_person->WorldPos;

	dx	=-	(SIN(p_person->Draw.Tweened->Angle)*dist)>>8;
	dz	=-	(COS(p_person->Draw.Tweened->Angle)*dist)>>8;

	new_position.X += dx;
	new_position.Z += dz;

	move_thing_on_map(p_person, &new_position);

}
/*
#define	ON_MAP(x,z)	( ((x)>0) && ((x)<MAP_WIDTH) && ((z)>0) && ((z)<MAP_DEPTH) )

SLONG	find_an_enemy_within_dist(SLONG x,SLONG y,SLONG z,SLONG dist,SLONG group)
{
	SLONG	dx,dz,blocks;
	SLONG	mx,mz;
	SLONG	index;

	blocks=dist>>ELE_SHIFT;

	mx=x>>ELE_SHIFT;
	mz=z>>ELE_SHIFT;

	for(dx=-blocks;dx<blocks;dx++)
	for(dz=-blocks;dz<blocks;dz++)
	{

		if(ON_MAP(mx+dx,mz+dz))
		{
			index=MAP2(mx+dx,mz+dz).MapWho;
			while(index)
			{
				Thing	*p_thing;
				p_thing=TO_THING(index);

				switch(p_thing->Class)
				{
					case	CLASS_PERSON:

						break;
				}

				index=p_thing->Child;
			}
		}
	}
}
*/


//
// Returns TRUE if there is a LOS between the two people's heads.
//

SLONG los_between_heads(
		Thing *person_1,
		Thing *person_2)
{
	SLONG x1 = person_1->WorldPos.X >> 8;
	SLONG y1 = person_1->WorldPos.Y >> 8;
	SLONG z1 = person_1->WorldPos.Z >> 8;

	SLONG x2 = person_2->WorldPos.X >> 8;
	SLONG y2 = person_2->WorldPos.Y >> 8;
	SLONG z2 = person_2->WorldPos.Z >> 8;

	y1 += 0x70;
	y2 += 0x70;

	return there_is_a_los(
				x1, y1, z1,
				x2, y2, z2,
				0);
}


//
// for stack reasons, should make barrel use same memory array
//

extern	THING_INDEX col_with[];

#ifndef PSX
void	oscilate_tinpanum(SLONG x,SLONG y,SLONG z,Thing *p_thing,SLONG vol)
{
	SLONG       col_with_upto;
	SLONG		collide_types = (1 << CLASS_PERSON);
	Thing		*col_thing;
	SLONG		i;

	col_with_upto = THING_find_sphere(
					    x,
						y,
						z,
						5*256,
						col_with,
						MAX_COL_WITH,
						collide_types);

	for (i = 0; i < col_with_upto; i++)
	{
		col_thing = TO_THING(col_with[i]);

		if (col_thing->State == STATE_DEAD || 
		    col_thing->State == STATE_DYING)
		{
			//
			// Dead or dying things are of no interest
			//

			continue;
		}

		switch(col_thing->Class)
		{
			case CLASS_PERSON:

				if (col_thing == p_thing)
				{
					//
					// Don't influence yourself!
					//
				}
				else
				{
					if ((col_thing->Genus.Person->Target   != THING_NUMBER(p_thing)) &&
						(col_thing->Genus.Person->PlayerID == 0))
					{
						//
						// This person doesn't have me as his target and he is not another player.
						//
						
						if (col_thing->State == STATE_IDLE)
						{
							//
							// People only listen while they're doing nothing.
							//

							if (los_between_heads(col_thing, p_thing))
							{
								set_face_thing(col_thing, p_thing);
							}
							else
							{
								turn_to_face_thing(col_thing,p_thing,2); 
							}
						}
					}
				}

				break;
		}
	}
}
#endif

SLONG dist_to_target(Thing *p_person_a,Thing *p_person_b)
{
	SLONG	dx,dz;

	dx=abs(p_person_a->WorldPos.X-p_person_b->WorldPos.X)>>8;
	dz=abs(p_person_a->WorldPos.Z-p_person_b->WorldPos.Z)>>8;
	return(QDIST2(dx,dz));
}

SLONG dist_to_target_pelvis(Thing *p_person_a,Thing *p_person_b)
{
	SLONG	dx,dz;
	SLONG	ax,ay,az;
	SLONG	bx,by,bz;

	calc_sub_objects_position(p_person_a,p_person_a->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&ax,&ay,&az);
	calc_sub_objects_position(p_person_b,p_person_b->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&bx,&by,&bz);


	dx=(p_person_a->WorldPos.X-p_person_b->WorldPos.X)>>8;
	dz=(p_person_a->WorldPos.Z-p_person_b->WorldPos.Z)>>8;

	dx+=ax-bx;
	dz+=az-bz;

	dx=abs(dx);
	dz=abs(dz);

	return(QDIST2(dx,dz));
}

//
// Returns TRUE if a person is crouching
//

SLONG is_person_crouching(Thing *p_person)
{
	ASSERT(p_person->Class == CLASS_PERSON);

	return
		p_person->SubState == SUB_STATE_CRAWLING     ||
		p_person->SubState == SUB_STATE_STOP_CRAWL   ||
		p_person->SubState == SUB_STATE_IDLE_CROUTCH ||
		p_person->SubState == SUB_STATE_IDLE_CROUTCHING;
}


//
// Returns TRUE if person_a can see person_b
//

//
// range is a theoretical maximum range of sight for person_a   default==0    
//        ==0 use default range 8<<8
//        <0 means forget view conditions and just use max range (useful for player)
//        >0 clip calculated view distance to this value

SLONG can_a_see_b(
		Thing *p_person_a,
		Thing *p_person_b,SLONG range,SLONG no_los)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG view;
	SLONG dist;
	SLONG angle;
	SLONG dangle;
	SLONG p_person_b_moving;


	if(p_person_a->Flags&FLAGS_PERSON_BEEN_SHOT)
	{
		range=-(9<<8);
	}



	/*
	
	// Makes people go berserk! I'm gonna stop them shooting/punching/kicking instead.

	if(p_person_b->Class==CLASS_PERSON && p_person_b->Genus.Person->PlayerID && EWAY_stop_player_moving())
	{
		//
		// can't see player during cut scene
		//
		return FALSE;
	}

	*/

	if (p_person_a->Genus.Person->Ware !=
		p_person_b->Genus.Person->Ware)
	{
		//
		// These two people are in different warehouses or one is in a warehouse
		// while the other isn't.
		//

		return FALSE;
	}

	if(range==0)
	{
		range=8<<8;
	}

	//
	// Is darci moving?
	//

	p_person_b_moving = TRUE;

	if (p_person_b->State == STATE_IDLE || (p_person_b->State == STATE_GUN && p_person_b->SubState == SUB_STATE_AIM_GUN))
	{
		p_person_b_moving = FALSE;
	}

	dx = p_person_b->WorldPos.X - p_person_a->WorldPos.X;
	dy = p_person_b->WorldPos.Y - p_person_a->WorldPos.Y;
	dz = p_person_b->WorldPos.Z - p_person_a->WorldPos.Z;

	dx >>= 8;
	dy >>= 8;
	dz >>= 8;

	dist = QDIST2(abs(dx),abs(dz));

	//
	// How far a can see b from depends on the amount of light falling on b.
	//

	if(range<0)
	{
		//
		// -ve range so ignore view conditions
		//
		view=-range;
	}
	else
	{
		if (p_person_a->Genus.Person->PlayerID)
		{
			view = 256 << 8;
		}
		else
		{		
			NIGHT_Colour col;

			col = NIGHT_get_light_at(
					p_person_b->WorldPos.X >> 8,
					p_person_b->WorldPos.Y >> 8,
					p_person_b->WorldPos.Z >> 8);

			//
			// See further in good light.
			//

			view  = col.red + col.green + col.blue;
			view += view << 3;
			view += view >> 2;
			view += 256;
		}

		if (is_person_crouching(p_person_b))
		{
			view >>= 1;
		}

		if (p_person_b_moving)
		{
			view += 256;
		}

		if (view > range) //256 * 8)
		{
			view = range; //256 * 8;
		}
	}


	if (dist > view)	  // this should be an #define or a table lookup for people types
	{
		//
		// Too far away.
		//

		return FALSE;
	}

	{
		//
		// Distance and dy is ok.
		//

		angle   = Arctan(dx,-dz) + 1024;
		angle  &= 2047;
		dangle  = angle - p_person_a->Draw.Tweened->Angle;
		dangle &= 2047;

		if (dist < 0xc0)
		{
			//
			// Very large field of view at this low range!
			//

			if (dangle < 700 || dangle > 2048 - 700)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}

		#define PEOPLE_FOV 420 //was 320

		if (dangle < PEOPLE_FOV || dangle > 2048 - PEOPLE_FOV)
		{
			//
			// Seeing someone out of the corner of your eye reduces your
			// viewing distance... if they are not moving.
			//

			if (!p_person_b_moving)
			{
				#define CORNER_OF_EYE_FOV 250
				
				if (WITHIN(dangle, CORNER_OF_EYE_FOV, 2048 - CORNER_OF_EYE_FOV))
				{
					if (dist > (view >> 1))
					{
						//
						// Too far away to see out of the corner of your eye.
						//

						return FALSE;
					}
				}
			}

			//
			// The height of each person's head.
			//

			UBYTE ahead = (is_person_crouching(p_person_a)) ? 0x20 : 0x60;
			UBYTE bhead = (is_person_crouching(p_person_b)) ? 0x20 : 0x60;

			//
			// Angle is valid
			//

			if(no_los)
				return(TRUE);

			if (p_person_b->Genus.Person->Ware)
			{
				//
				// there_is_a_los() works as long as you don't do the underground check.
				//

				if (there_is_a_los(
						(p_person_a->WorldPos.X >> 8),
						(p_person_a->WorldPos.Y >> 8) + ahead,
						(p_person_a->WorldPos.Z >> 8),
						(p_person_b->WorldPos.X >> 8),
						(p_person_b->WorldPos.Y >> 8) + bhead,
						(p_person_b->WorldPos.Z >> 8),
						LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
				{
					return TRUE;
				}
			}
			else
			{
				if (there_is_a_los(
						(p_person_a->WorldPos.X >> 8),
						(p_person_a->WorldPos.Y >> 8) + ahead,
						(p_person_a->WorldPos.Z >> 8),
						(p_person_b->WorldPos.X >> 8),
						(p_person_b->WorldPos.Y >> 8) + bhead,
						(p_person_b->WorldPos.Z >> 8),
						0))
				{
					//
					// No building in the way
					//

					/*

					AENG_world_line(
						(p_person_a->WorldPos.X >> 8),
						(p_person_a->WorldPos.Y >> 8) + 0x60,
						(p_person_a->WorldPos.Z >> 8),
						32,
						0x00ffffff,
						(p_person_b->WorldPos.X >> 8),
						(p_person_b->WorldPos.Y >> 8) + 0x60,
						(p_person_b->WorldPos.Z >> 8),
						0,
						0x00123456,
						TRUE);

					*/

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}


SLONG can_i_see_place(Thing *p_person, SLONG x, SLONG y, SLONG z)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG view;
	SLONG dist;
	SLONG angle;
	SLONG dangle;

	dx = x - (p_person->WorldPos.X >> 8);
	dy = y - (p_person->WorldPos.Y >> 8);
	dz = z - (p_person->WorldPos.Z >> 8);

	dist = QDIST2(abs(dx),abs(dz));

	if (dist > 0x600)
	{
		//
		// Too far away.
		//

		return FALSE;
	}

	if (abs(dy) > abs(dist >> 1))
	{
		//
		// Too high up.
		//

		return FALSE;
	}

	//
	// Distance and dy is ok.
	//

	angle   = Arctan(dx,-dz) + 1024;
	angle  &= 2047;
	dangle  = angle - p_person->Draw.Tweened->Angle;
	dangle &= 2047;

	#define PEOPLE_FOV 420 //was 320

	if (dangle < PEOPLE_FOV || dangle > 2048 - PEOPLE_FOV)
	{
		//
		// The height of each person's head.
		//

		UBYTE ahead = (is_person_crouching(p_person)) ? 0x20 : 0x60;

		//
		// Angle is valid
		//

		if (there_is_a_los(
				(p_person->WorldPos.X >> 8),
				(p_person->WorldPos.Y >> 8) + ahead,
				(p_person->WorldPos.Z >> 8),
				x,
				y + 0x20,	// So we can see things on the ground.
				z,
				0))
		{
			//
			// No building in the way
			//

			return TRUE;
		}
	}

	return FALSE;
}





void set_person_sliding_tackle(Thing *p_person, Thing *p_target)
{
	if(p_person->SubState!=SUB_STATE_RUNNING_SKID_STOP)
	{
		set_face_thing(p_person, p_target);
		set_generic_person_state_function(p_person, STATE_MOVEING);
		set_anim(p_person, ANIM_SLIDER_START);
		p_person->SubState = SUB_STATE_RUNNING_SKID_STOP;
	}
}

//
// Returns TRUE if the person vaults...
//

SLONG set_person_vault(Thing *p_person, SLONG facet)
{
	if(set_person_pos_for_fence_vault(p_person,facet))
	{
		set_anim(p_person,ANIM_VAULT);
		p_person->SubState = SUB_STATE_RUNNING_VAULT;
		p_person->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;

		set_generic_person_state_function(p_person, STATE_MOVEING);

		return TRUE;
	}

	return FALSE;
}

SLONG set_person_climb_half(Thing *p_person, SLONG facet)
{
	if(set_person_pos_for_half_step(p_person,facet))
	{
		set_anim(p_person,ANIM_GET_UP_HALF_BLOCK);
		p_person->SubState = SUB_STATE_RUNNING_HALF_BLOCK;
		p_person->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;

		set_generic_person_state_function(p_person, STATE_MOVEING);

		return TRUE;
	}

	return FALSE;
}

//
// can person see player
//

SLONG can_i_see_player(Thing *p_person)
{
	//
	// THERE MIGHT BE TWO PLAYERS NOW REMEMBER!
	//


	return can_a_see_b(p_person, NET_PERSON(0));
}

void	do_look_for_enemies(Thing *p_person)
{
	//
	// THERE MIGHT BE TWO PLAYERS NOW REMEMBER!
	//


	//
	// for now the player is the only enemy
	//

	if (can_i_see_player(p_person))
	{
		//
		// Navigate to the player.
		//

		p_person->Genus.Person->Target = THING_NUMBER(NET_PERSON(0)); 
		ASSERT(p_person->Genus.Person->Target != THING_NUMBER(p_person));
	ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
		p_person->Genus.Person->Flags |= FLAG_PERSON_NAV_TO_KILL;

		set_person_mav_to_thing(p_person,NET_PERSON(0));
	}
}

//
// If the person is too far from home or Darci sets of home again.
//
#ifdef	UNUSED
void show_me_the_way_to_go_home(Thing *p_person)
{
	SLONG dx;
	SLONG dz;

	SLONG home_x = p_person->Genus.Person->HomeX << 8;
	SLONG home_z = p_person->Genus.Person->HomeZ << 8;

	SLONG home_sickness;
	SLONG darci_distance;

	//
	// Far from home?
	//

	dx = home_x - p_person->WorldPos.X;
	dz = home_z - p_person->WorldPos.Z;

	home_sickness = abs(dx) + abs(dz);	// i.e. Distance from home!

	//
	// Far from Darci?
	//

	dx = NET_PERSON(0)->WorldPos.X - p_person->WorldPos.X;
	dz = NET_PERSON(0)->WorldPos.Z - p_person->WorldPos.Z;

	darci_distance = abs(dx) + abs(dz);

	//
	// Go home?
	//

	if (home_sickness > 0x100000 || (home_sickness > 0x018000 && darci_distance > 0x80000))
	{
		p_person->Genus.Person->Target = NULL;

		set_person_mav_to_xz(
			p_person,
			home_x >> 8,
			home_z >> 8);
	}
}
#endif

extern void	play_music(UWORD id, UBYTE track);


ULONG	timer_bored=0;
void	general_process_player(Thing *p_person)
{
	DrawTween  *dt;

	if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		if (p_person->State    == STATE_MOVEING &&
			p_person->SubState == SUB_STATE_RUNNING)
		{
			p_person->Genus.Person->Mode = PERSON_MODE_RUN;
		}
	}

	#if PSX

	if(player_dlight==0)
	{
		player_dlight = NIGHT_dlight_create( p_person->WorldPos.X>>8, (p_person->WorldPos.Y>>8)+128, p_person->WorldPos.Z>>8, 200, 4, 4, 4);

	}
	else
	{
		NIGHT_dlight_move(player_dlight,p_person->WorldPos.X>>8, (p_person->WorldPos.Y>>8)+64, p_person->WorldPos.Z>>8);
	}

	#endif

	if(p_person->Genus.Person->GangAttack)
	{
extern	void	check_players_gang(Thing *p_target);
		check_players_gang(p_person);
	}
/*
	if(p_person->Genus.Person->Mode==PERSON_MODE_SPRINT)
	{
		AENG_world_line(
			p_person->WorldPos.X         >> 8,
			p_person->WorldPos.Y + 0 >> 8,
			p_person->WorldPos.Z         >> 8,
			16,
			0xff0000,
			p_person->WorldPos.X                  >> 8,
			p_person->WorldPos.Y + 0 + 0x1000 >> 8,
			p_person->WorldPos.Z                  >> 8,
			0,
			0x330088,
			FALSE);
	}
*/


	if(p_person->Genus.Person->Mode==PERSON_MODE_FIGHT)
	{

		if(p_person->Genus.Person->Target)
		{
			Thing	*p_target;

			p_target=TO_THING(p_person->Genus.Person->Target);

			//
			// player has a target, lets see if we should bin it
			//

//			if((p_target->State==STATE_DEAD) ||(p_target->State==STATE_DYING && (p_target->Genus.Person->Flags&FLAG_PERSON_KO)==0))
			if(is_person_dead(p_target))
			{
				//
				// find a better target
				//

				fight_any_gang_attacker(p_person);

			}
		}
		else
			if(count_gang(p_person))
			{
				fight_any_gang_attacker(p_person);
			}

		timer_bored=0;
	}
	else
	{
		if(!EWAY_stop_player_moving())
			timer_bored+=TICK_TOCK;
	}


	dt=p_person->Draw.Tweened;
	if(p_person->SubState==SUB_STATE_RUNNING)
	{
		if(dt->Roll||dt->DRoll)
		{
			//
			// This is a really crap place to put this, but where else can it go?
			//

			if(dt->DRoll)
			{
				if(abs(dt->Roll)<70)
					dt->Roll-=dt->DRoll;

				//
				// when you swap direction need to clear roll
				//
				if(dt->Roll>0 && dt->DRoll>0)
					dt->Roll=0;
				else
				if(dt->Roll<0 && dt->DRoll<0)
					dt->Roll=0;
				dt->DRoll=0;

			}
			else
			{
				//
				// uses 2's complement maths shift feature
				//
				if(dt->Roll<0)
					dt->Roll-=(dt->Roll>>1);
				if(dt->Roll>0)
					dt->Roll+=(-dt->Roll)>>1;

			}

		}
		if(p_person->Genus.Person->Flags2&FLAG2_PERSON_CARRYING)
		{
			TO_THING(p_person->Genus.Person->Target)->Draw.Tweened->Roll=(2048-dt->Roll)&2047;
		}

	}
	else
	{
		if(p_person->SubState==SUB_STATE_WALKING)
		{
/*
			if((GAME_TURN&3)==0)
			{
				if(should_i_sneak(p_person))
				{
					if(p_person->Genus.Person->Mode!=PERSON_MODE_SNEAK)
					{
						p_person->Genus.Person->Mode=PERSON_MODE_SNEAK;
						set_person_sneaking(p_person);
					}

				}
				else
				{
					if(p_person->Genus.Person->Mode!=PERSON_MODE_WALK)
					{
						p_person->Genus.Person->Mode=PERSON_MODE_WALK;
						set_person_walking(p_person);
					}

				}
			}
*/
		}

		if (p_person->SubState != SUB_STATE_RIDING_BIKE)
		{
			if(dt->Roll<0)
				dt->Roll-=(dt->Roll>>2);
			if(dt->Roll>0)
				dt->Roll+=(-dt->Roll)>>2;
		}
	}
}


void person_pick_best_target(Thing *p_person,SLONG dir)
{
	SLONG i;

	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG best_dist = INFINITY;

	UWORD lowest_person = 0xffff;
	UWORD highest_person = 0;
	UWORD next_person   = 0xffff;
	UWORD prev_person   = 0;

	SLONG num_found = THING_find_sphere(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							0x300,
							THING_array,
							THING_ARRAY_SIZE,
							1 << CLASS_PERSON);

	for (i = 0; i < num_found; i++)
	{
		Thing *p_found = TO_THING(THING_array[i]);

		if (p_found == p_person)
		{
			continue;
		}

		if (p_found->State == STATE_DEAD)
		{
			//
			// Ignore dead and arrested people.
			//

			continue;
		}

		if (PCOM_person_wants_to_kill(p_found) == THING_NUMBER(p_person))
		{
			if (p_person->Genus.Person->Target == NULL)
			{
				//
				// Pick the nearest person.
				//

				dx = abs(p_found->WorldPos.X - p_person->WorldPos.X);
				dz = abs(p_found->WorldPos.Z - p_person->WorldPos.Z);

				dist = QDIST2(dx,dz);

				if (dist < best_dist)
				{
					best_dist   = dist;
					next_person = THING_array[i];
				}
			}
			else
			{
				//
				// Found someone who wants to kill me.
				//

				if (THING_array[i] < lowest_person)
				{
					lowest_person = THING_array[i];
				}
				if (THING_array[i] > highest_person)
				{
					highest_person = THING_array[i];
				}

				if (THING_array[i] > p_person->Genus.Person->Target)
				{
					if (THING_array[i] < next_person)
					{
						next_person = THING_array[i];
					}
				}
				else
				if (THING_array[i] < p_person->Genus.Person->Target)
				{
					if (THING_array[i] > prev_person)
					{
						prev_person = THING_array[i];
					}
				}
			}
		}
	}

	if(dir==1)
	{
		if (next_person != 0xffff)
		{
			p_person->Genus.Person->Target = next_person;
		}
		else
		if (lowest_person != 0xffff)
		{
			p_person->Genus.Person->Target = lowest_person;
		}
	}
	else
	{
		if (prev_person != 0xffff)
		{
			p_person->Genus.Person->Target = prev_person;
		}
		else
		if (highest_person != 0xffff)
		{
			p_person->Genus.Person->Target = highest_person;
		}
	}

	if (p_person->Genus.Person->Target)
	{
		turn_to_face_thing(p_person,TO_THING(p_person->Genus.Person->Target),0);
	}

	/*

	UWORD	target;
extern	UWORD	get_nearest_gang_member(Thing *p_target);
	target=get_nearest_gang_member(p_person);
	if(target)
	{
		turn_to_face_thing(p_person,TO_THING(target),0);
		p_person->Genus.Person->Target=target;
	}

	*/
}


void	general_process_person(Thing *p_person)
{
/*
	if(p_person->State!=STATE_DEAD && p_person->State!=STATE_DYING)
	{
		if(p_person->Genus.Person->PersonType==PERSON_CIV)
		if(p_person->Genus.Person->PersonType!=PERSON_ROPER)
		if(p_person->Genus.Person->PersonType!=PERSON_HOSTAGE)
		if(p_person->Genus.Person->PlayerID==0)
		if(p_person->Genus.Person->InCar==0)
		{
			if((PTIME(p_person)&63)==0)
			{
				set_person_dead(p_person, NULL, PERSON_DEATH_TYPE_PRONE, 0, 0);
	//			set_person_dead(p_person);
			}
		}
	}
*/


/*	
#ifdef	PSX
//	ASSERT(p_person->Draw.Tweened!=TO_DRAW_TWEEN(0));

	if(p_person->State!=STATE_DEAD)
	switch(p_person->Genus.Person->PersonType)
	{
		case	PERSON_CIV:
			ASSERT(p_person->Draw.Tweened->MeshID>6 &&p_person->Draw.Tweened->MeshID<10);
			break;
	}
#endif
*/

/*
	if(p_person->Genus.Person->InCar)
		AENG_world_line(
			p_person->WorldPos.X         >> 8,
			p_person->WorldPos.Y + 0 >> 8,
			p_person->WorldPos.Z         >> 8,
			16,
			0xff0000,
			p_person->WorldPos.X                  >> 8,
			p_person->WorldPos.Y + 0 + 0x21000 >> 8,
			p_person->WorldPos.Z                  >> 8,
			0,
			0x330088,
			FALSE);
*/
	//
	// every 64 turns clear the flag been shot, this flag gives you super vision
	//
//	if((GAME_TURN&63)==(THING_NUMBER(p_person)&63))
	if((PTIME(p_person)&63)==0)
	{
		p_person->Flags&=~FLAGS_PERSON_BEEN_SHOT;
	}

//	if(Keys[KB_F])
//		p_person->WorldPos.Y=0;


#ifndef	NDEBUG
//	if(p_person->Genus.Person->Flags2&FLAG2_PERSON_GUILTY)
/*
	if(p_person->Genus.Person->OnFacet)
	{
		AENG_world_line(
			p_person->WorldPos.X         >> 8,
			p_person->WorldPos.Y + 0 >> 8,
			p_person->WorldPos.Z         >> 8,
			16,
			0xff0000,
			p_person->WorldPos.X                  >> 8,
			p_person->WorldPos.Y + 0 + 0x21000 >> 8,
			p_person->WorldPos.Z                  >> 8,
			0,
			0x330088,
			FALSE);
	}
	if(p_person->Genus.Person->Flags&FLAG_PERSON_FELON)
	{
		AENG_world_line(
			p_person->WorldPos.X         >> 8,
			p_person->WorldPos.Y + 0 >> 8,
			p_person->WorldPos.Z         >> 8,
			16,
			0x00ffff,
			p_person->WorldPos.X                  >> 8,
			p_person->WorldPos.Y + 0 + 0x21000 >> 8,
			p_person->WorldPos.Z                  >> 8,
			0,
			0x33ff88,
			FALSE);
	}
*/
	if(ControlFlag)
	if(p_person->Genus.Person->Target)
	{
		Thing	*p_target;

		p_target=TO_THING(p_person->Genus.Person->Target);

		AENG_world_line(
			p_person->WorldPos.X         >> 8,
			p_person->WorldPos.Y + 0x1000 >> 8,
			p_person->WorldPos.Z         >> 8,
			16,
			0x00ffff,
			p_target->WorldPos.X                  >> 8,
			p_target->WorldPos.Y + 0 + 0x1000 >> 8,
			p_target->WorldPos.Z                  >> 8,
			0,
			0x33ff88,
			FALSE);
	}
#endif

#if 0
	if (p_person->OnFace && p_person->OnFace < 0 && p_person->State != STATE_DEAD)
	{
		if (IS_ROOF_HIDDEN_FACE(p_person->OnFace))
		{
			SLONG mx = ROOF_HIDDEN_X(p_person->OnFace);
			SLONG mz = ROOF_HIDDEN_Z(p_person->OnFace);

			ASSERT(abs(mx - (p_person->WorldPos.X >> 16)) <= 1);
			ASSERT(abs(mz - (p_person->WorldPos.Z >> 16)) <= 1);
		}
	}
#endif


#ifndef PSX
/*
	if(p_person->Genus.Person->PlayerID==0)
	{
		if(!is_person_dead(p_person))
		if(can_a_see_b(p_person,NET_PERSON(0)))
		{
			Thing	*p_target=NET_PERSON(0);
			AENG_world_line_infinite(p_target->WorldPos.X>>8,p_target->WorldPos.Y>>8,p_target->WorldPos.Z>>8,1,0x00ff00,p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,5,0x00ff00,1);
		}
	}
*/
#endif
/*
	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
	{
		DebugText(" %d %d %d \n",p_person->State,p_person->SubState,p_person->Genus.Person->pcom_ai);
	}
*/
//	p_person->Genus.Person->Flags &=~ FLAG_PERSON_DID_ANIMATE;
/*
	if(p_person->Draw.Tweened->CurrentFrame==p_person->Draw.Tweened->NextFrame)
	{
		AENG_world_line(p_person->WorldPos.X          >> 8,p_person->WorldPos.Y + 0x0000 >> 8,p_person->WorldPos.Z  >> 8,
						32,0x00ff00,p_person->WorldPos.X >> 8,p_person->WorldPos.Y + 0x1000 >> 8,p_person->WorldPos.Z >> 8,
						0,
						0x000000,
						TRUE);
	}
*/
	#ifndef NDEBUG
/*
	{
		Thing *p_target;

		if (WITHIN(p_person->Genus.Person->Target, 1, MAX_THINGS - 1))
		{
			p_target = TO_THING(p_person->Genus.Person->Target);

			if (p_target->Class == CLASS_PERSON)
			{
				AENG_world_line(
					p_person->WorldPos.X          >> 8,
					p_person->WorldPos.Y + 0x1000 >> 8,
					p_person->WorldPos.Z          >> 8,
					32,
					0xff0000,
					p_person->WorldPos.X          >> 8,
					p_person->WorldPos.Y + 0x1000 >> 8,
					p_person->WorldPos.Z          >> 8,
					0,
					0x000000,
					TRUE);
			}
		}
	}

	//
	// Draw an arrow over helpless people!
	//

	if (p_person->Genus.Person->Flags & FLAG_PERSON_HELPLESS)
	{
		SLONG px;
		SLONG py;
		SLONG pz;

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			SUB_OBJECT_HEAD,
		   &px,
		   &py,
		   &pz);

		px += p_person->WorldPos.X >> 8;
		py += p_person->WorldPos.Y >> 8;
		pz += p_person->WorldPos.Z >> 8;

		AENG_world_line(
			px, py, pz, 0, 0xffffff,
			px, py + 20, pz, 32, 0xccccff,
			FALSE);
	}
*/

	#endif

//	ASSERT(p_person->WorldPos.X>>16<128);
//	ASSERT(p_person->WorldPos.Z>>16<128);


	if (p_person->OnFace>0)
	{
		ASSERT(WITHIN(p_person->OnFace, 1, next_prim_face4 - 1));

		PrimFace4 *f4 = &prim_faces4[p_person->OnFace];

		ASSERT(f4->FaceFlags & FACE_FLAG_WALKABLE);

		if (f4->FaceFlags & FACE_FLAG_WMOVE)
		{
			SLONG now_x;
			SLONG now_y;
			SLONG now_z;
			SLONG now_dangle;
			SLONG	wmove_index;

			wmove_index=f4->ThingIndex;
			ASSERT(WITHIN(wmove_index, 1, WMOVE_face_upto - 1));

			WMOVE_relative_pos(
				f4->ThingIndex,
				p_person->WorldPos.X,
				p_person->WorldPos.Y,
				p_person->WorldPos.Z,
			   &now_x,
			   &now_y,
			   &now_z,
			   &now_dangle);

			p_person->Draw.Tweened->Angle += now_dangle;
			p_person->Draw.Tweened->Angle &= 2047;

			//
			// Make sure you don't end up too near the edge of the map!
			//

			GameCoord newpos;

			newpos.X = now_x;
			newpos.Y = now_y;
			newpos.Z = now_z;

			if (WITHIN(newpos.X, 2 << 16, (PAP_SIZE_HI - 3) << 16) &&
				WITHIN(newpos.Z, 2 << 16, (PAP_SIZE_HI - 3) << 16))
			{

				move_thing_on_map(p_person, &newpos);
			}
			else
			{
				//
				// Too close to the edge of the map! Fall off.
				// 

				p_person->OnFace = 0;

				set_person_dead(
					p_person,
					NULL,
					PERSON_DEATH_TYPE_STAY_ALIVE,
					0,
					0);
			}
		}
	}


	if (p_person->State == STATE_DEAD || 
		p_person->State == STATE_DYING)
	{
		//
		// Dead or dying things are of no interest
		//

		return;
	}

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		if (p_person->WorldPos.Y < -0x180000)
		{
			//
			// This person is dead!
			//

			p_person->Genus.Person->Health = 0;

			set_person_dead(p_person, NULL, PERSON_DEATH_TYPE_PRONE, 0, 0);

			remove_thing_from_map(p_person);

			return;
		}
	}

	{
		UWORD	max_stamina=128;
		if(p_person->Genus.Person->PlayerID)
		{
			max_stamina+=NET_PLAYER(p_person->Genus.Person->PlayerID-1)->Genus.Player->Stamina;
			if(!continue_pressing_action(p_person))
			{
				if (p_person->Genus.Person->Stamina < max_stamina)
				{
					p_person->Genus.Person->Stamina++;

					if (p_person->Genus.Person->Stamina == 5)
					{
						p_person->Genus.Person->Stamina = 20;
					}
				}
			}
		}
		else
		if (p_person->Genus.Person->Stamina < max_stamina)
		{
			p_person->Genus.Person->Stamina++;
		}
	}

	// check for burning flags

	SLONG get_person_radius(SLONG type);

	if (p_person->Flags & FLAGS_BURNING) {
		SLONG x2,y2,z2,ndx;
		Thing *thing;
		Pyro  *pyro;
		TRACE("the pain the pain\n");

		ndx=p_person->Genus.Person->BurnIndex;
		if ((!ndx)||((pyro=TO_PYRO(ndx-1))->PyroType==PYRO_NONE)) {
			thing=PYRO_create(p_person->WorldPos,PYRO_IMMOLATE);
			if (thing)
			{
				pyro=thing->Genus.Pyro;
				pyro->victim=p_person;
				pyro->Flags=PYRO_FLAGS_FLICKER; // immolate faces
				p_person->Genus.Person->BurnIndex=PYRO_NUMBER(pyro)+1;
				thing->StateFn(thing);
			}
		} else {
			// add to it's life theoretically... unless in water
			if (PAP_2HI(p_person->WorldPos.X>>16,p_person->WorldPos.Z>>16).Flags & PAP_FLAG_WATER) {
				if (pyro->PyroType==PYRO_IMMOLATE) {
					pyro->Dummy=2;
					pyro->radius=290;
				}
			}
		}

		if (p_person->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL)
		{
			//
			// Only the player can hurt this person.
			//
		}
		else
		if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
		{
			//
			// Nothing hurts this person.
			//
		}
		else
		{
			p_person->Genus.Person->Health-=30;
//			add_damage_value_thing(p_person,30>>1);
		}

		if(p_person->Genus.Person->Health<=0)
		{
			p_person->Genus.Person->Health=0;

			set_person_dead(
				p_person,
				NULL,
				PERSON_DEATH_TYPE_OTHER,
				FALSE,
				0);

		} else {

			// later it'd be nice to only turn this off after a delay
			p_person->Flags &= ~FLAGS_BURNING;
			collide_against_things(
				p_person,
				get_person_radius(p_person->Genus.Person->PersonType),
				p_person->WorldPos.X,p_person->WorldPos.Y,p_person->WorldPos.Z,
				&x2,&y2,&z2);

			if (!(p_person->Flags & FLAGS_BURNING))
				TRACE("oh that's much better thankyou\n");
		}
	}

	// used to have something somewhere else to do this, but it appears to have vanished.
	// so here's something to do it. :P

	if (p_person->Genus.Person->BurnIndex && (p_person->Genus.Person->Health>0))
		p_person->Genus.Person->Health--;


	//
	// If urinating... then pee!
	//

#ifndef PSX	// save Eidos/Sony the trouble of instructing us to remove this
	if (p_person->Genus.Person->Flags & FLAG_PERSON_PEEING)
	{
		if (p_person->Flags & FLAGS_IN_VIEW)
		{
			SLONG penis_x;
			SLONG penis_y;
			SLONG penis_z;

			SLONG dx;
			SLONG dz;

			calc_sub_objects_position(
				p_person,
				p_person->Draw.Tweened->AnimTween,
				SUB_OBJECT_PELVIS,
			   &penis_x,
			   &penis_y,
			   &penis_z);

			penis_x += p_person->WorldPos.X >> 8;
			penis_y += p_person->WorldPos.Y >> 8;
			penis_z += p_person->WorldPos.Z >> 8;

			penis_y -= 0x10;

			dx = -SIN(p_person->Draw.Tweened->Angle) >> 13;
			dz = -COS(p_person->Draw.Tweened->Angle) >> 13;

			DIRT_new_water(
				penis_x,
				penis_y,
				penis_z,
				dx, -2, dz,
				DIRT_TYPE_URINE);
		}
	}
#endif

	// if dying, leave blood trails
	if (p_person->Genus.Person->Health<health[p_person->Genus.Person->PersonType]>>2) 
	{
		if(p_person->Genus.Person->Stamina>50)
			p_person->Genus.Person->Stamina-=1; //bleeding people lose stamina all the time

		if (!p_person->Genus.Person->InCar)
		{
			if (((Random()&0x7f)>p_person->Genus.Person->Health)&&(Random()&1)) TRACKS_Bleed(p_person);
		}
	}


	#if DARCI_HITS_COPS

	if (p_person->Genus.Person->UnderAttack)
	{	
		SLONG ticks = 256 * TICK_RATIO >> TICK_SHIFT;

		if (p_person->Genus.Person->UnderAttack <= ticks)
		{	
			p_person->Genus.Person->UnderAttack = 0;
		}
		else
		{
			SLONG last = p_person->Genus.Person->UnderAttack;

			p_person->Genus.Person->UnderAttack -= ticks;

			if (last         >  0xffff - (256 * 20 * 2) &&
				last - ticks <= 0xffff - (256 * 20 * 2))
			{
				if (p_person->Genus.Person->pcom_ai       == PCOM_AI_COP &&
					p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
				{
					PCOM_set_person_ai_talk_to(
						p_person,
						NET_PERSON(0),
						PCOM_AI_SUBSTATE_TALK_ASK,
						FALSE);


//					PANEL_new_text(p_person, 4000, "Hey! Why are you hitting me, Darci?");

				}
			}
		}
	}

	#endif

	//
	// Does this person have the grappling hook?
	//
#ifndef PSX
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GRAPPLING)
	{
		SLONG percent;
		SLONG pitch;

		if (p_person->State == STATE_GRAPPLING &&
			p_person->SubState == SUB_STATE_GRAPPLING_WINDUP)
		{
			//
			// How far through the animation is Darci?
			//

			percent  = p_person->Draw.Tweened->FrameIndex << 8;
			percent |= p_person->Draw.Tweened->AnimTween;
			pitch    = (-percent * 2048) / 0x500;
			pitch   &= 2047;
		}
		else
		{
			//
			// Make the hook hang down.
			//

			pitch = 1536;
		}

		//
		// Make sure the hook is connected to Darci's right hand.
		//

		SLONG px;
		SLONG py;
		SLONG pz;

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			SUB_OBJECT_RIGHT_HAND,
		   &px,
		   &py,
		   &pz);

		px += p_person->WorldPos.X >> 8;
		py += p_person->WorldPos.Y >> 8;
		pz += p_person->WorldPos.Z >> 8;

		HOOK_spin(
			px,
			py,
			pz,
			p_person->Draw.Tweened->Angle,
		   -pitch);
	}
#endif
	/*

	//if( (THING_NUMBER(p_person)&0x7) == (GAME_TURN&0x7) )
	if(p_person->Genus.Person->PlayerID==0)
	{
		if (p_person->State == STATE_IDLE)
		{
			//
			// Follow the player?
			//

			do_look_for_enemies(p_person);

			if (ControlFlag)
			{
				TRACE("Looking for enemies\n");
			}
		}

		//
		// Too far from home?
		//

		show_me_the_way_to_go_home(p_person);
	}

	*/

	if( (p_person->Genus.Person->Action!=ACTION_RUN_JUMP) && (p_person->Genus.Person->Action!=ACTION_FIGHT_PUNCH) && (p_person->Genus.Person->Action!=ACTION_GRAPPLE))
	{
		p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_KICK;
	}


/*

#ifndef PSX
	if (GAME_FLAGS & GF_SEWERS)
	{
		if ((p_person->Flags & FLAGS_IN_SEWERS) && p_person->State != STATE_IDLE)
		{
			SLONG i;
			SLONG j;
			SLONG angle;

			ULONG bit;
			ULONG newsewerbits;
			ULONG oldsewerbits = p_person->Genus.Person->sewerbits;

			SLONG partx;
			SLONG party;
			SLONG partz;

			SLONG sheight = NS_calc_splash_height_at(
								p_person->WorldPos.X >> 8,
								p_person->WorldPos.Z >> 8);

			//
			// Work out which body parts of the person are under water.
			//

			bit          = 1;
			newsewerbits = 0;

			for (i = 0; i < 15; i++)
			{
				calc_sub_objects_position(
					p_person,
					p_person->Draw.Tweened->AnimTween,
					i,
				   &partx,
				   &party,
				   &partz);

				partx += p_person->WorldPos.X >> 8;
				party += p_person->WorldPos.Y >> 8;
				partz += p_person->WorldPos.Z >> 8;

				if (party < sheight)
				{
					newsewerbits |= bit;

					if (oldsewerbits & bit)
					{
						if (((GAME_TURN + i) & 0x3) == 2)
						{
							DRIP_create(
								partx,
								sheight,
								partz);
						}
					}
					else
					{
						//
						// This limb is entering the water for the first time.
						// make a splash.
						//

						for (j = 0; j < 4; j++)
						{
							angle = rand() & 2047;

							DIRT_new_water(
								partx,
								sheight,
								partz,
								SIN(angle) >> 14,
								14,
								COS(angle) >> 14);
						}

						DRIP_create(
							partx,
							sheight,
							partz);
					}
				}

				bit <<= 1;
			}

			p_person->Genus.Person->sewerbits = newsewerbits;
		}
	}
#endif

	*/

	/*
	if (!(p_person->Flags & FLAGS_IN_SEWERS))
	{
		SLONG sx = p_person->WorldPos.X >> 16;
		SLONG sz = p_person->WorldPos.Z >> 16;

		ASSERT(WITHIN(sx, 0, PAP_SIZE_HI - 1));
		ASSERT(WITHIN(sz, 0, PAP_SIZE_HI - 1));

		if (p_person->State == STATE_CLIMB_LADDER)
		{
			//
			// Let the climb ladder function handle this eventuality.
			//
		}
		else
		{
			if (PAP_2HI(sx,sz).Flags & PAP_FLAG_SEWER_SQUARE)
			{
				SLONG bottom = PAP_calc_height_at(
									p_person->WorldPos.X >> 8,
									p_person->WorldPos.Z >> 8) << 8;
				
				if (p_person->WorldPos.Y < bottom + 0x6000)
				{
					//
					// This person is entering the sewers.
					//

					p_person->Flags |= FLAGS_IN_SEWERS;
				}
			}
		}
	}
	*/

	if(p_person->Genus.Person->PlayerID)
	{
		general_process_player(p_person);
	}

}

SLONG	check_on_slippy_slope(Thing *p_person)
{
	SLONG	slope,angle;
	SLONG	size=50;

		if(p_person->Genus.Person->InsideIndex)
		{
			slope=0;
		}
		else
		{
			if(p_person->OnFace<0)
			{
				slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
			}
			else
			{

				slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle)>>1;
			}
		}

		
	if(slope>size)	// Change 50 here and change the 50 in MAV_precalculate...
	{
		switch(p_person->SubState)
		{
			default:
			case	SUB_STATE_RUNNING_JUMP:
			case	SUB_STATE_RUNNING_JUMP_LAND_FAST:
			case	SUB_STATE_RUNNING_JUMP_LAND:
				set_generic_person_state_function(p_person,STATE_MOVEING);
			case	SUB_STATE_RUNNING:
			case	SUB_STATE_WALKING:
			case	SUB_STATE_WALKING_BACKWARDS:
			case	SUB_STATE_FLIPING:
			case	SUB_STATE_RUNNING_SKID_STOP:

				set_anim(p_person,ANIM_FALLING);

			case	SUB_STATE_SLIPPING:
				p_person->SubState=SUB_STATE_SLIPPING;
				p_person->Draw.Tweened->AngleTo=angle;

				slope=MIN(slope-size,10);
				slope=MAX(slope,size);


				change_velocity_to(p_person,slope);

//					p_person->Velocity=MIN(slope-50,40);
				break;
		}

		if(p_person->OnFace==0 &&p_person->Genus.Person->Ware==0)
	 	if (PAP_2HI((p_person->WorldPos.X >> 16)&127, (p_person->WorldPos.Z >> 16)&127).Flags & PAP_FLAG_NOGO)
		{
			//
			// shit its a nogo, so lets ping to nearest normal space
			//
			SLONG	angle,step;
			for(step=64;step<512;step+=64)
			{
				for(angle=0;angle<2048;angle+=256)
				{
					SLONG	dx,dz;
					dx=(COS(angle)*step)>>8;
					dz=(SIN(angle)*step)>>8;
				 	if (!(PAP_2HI(((p_person->WorldPos.X+dx) >> 16)&127, ((p_person->WorldPos.Z+dz) >> 16)&127).Flags & PAP_FLAG_NOGO))
					{
						GameCoord newpos;

						slope=PAP_on_slope((p_person->WorldPos.X+dx)>>8,(p_person->WorldPos.Z+dz)>>8,&angle)>>1;
						if(slope<40)
						{


							newpos.X = p_person->WorldPos.X+dx;
							newpos.Z = p_person->WorldPos.Z+dz;
							newpos.Y = PAP_calc_map_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8);

							move_thing_on_map(p_person, &newpos);

							step=50000;
							break;
						}
					}

				}
			}

		}

		return(1);
	}
	else
	if(p_person->SubState==SUB_STATE_SLIPPING)
	{
		p_person->SubState=SUB_STATE_SLIPPING_END;
		//set_person_idle(p_person);
	}
	return(0);

}
SLONG	slope_ahead(Thing *p_person,SLONG dist)
{
	SLONG dx;
	SLONG dz;
	SLONG	slippy;

	dx = -(SIN(p_person->Draw.Tweened->Angle) * dist) >> 8;
	dz = -(COS(p_person->Draw.Tweened->Angle) * dist) >> 8;

	p_person->WorldPos.X+=dx;
	p_person->WorldPos.Z+=dz;

	slippy=check_on_slippy_slope(p_person);

	p_person->WorldPos.X-=dx;
	p_person->WorldPos.Z-=dz;

	return(slippy);

}
void person_normal_move_dxdz(Thing *p_person,SLONG dx,SLONG dz)
{
//	SLONG i;

	SLONG dy;
	SLONG new_y;
	SLONG on_face;

//	SLONG x1, y1, z1;
//	SLONG x2, y2, z2;

//	GameCoord new_position;

//	Thing *col_thing;
	slide_ladder=0;

	if (p_person->Draw.Tweened->Locked)
	{
		//
		// The movement is part of the animation.
		//

		return;
	}

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_HIT_WALL;

	dy = 0;


	//
	// Move the person slower if he is at the wrong angle.
	//

	SLONG dangle = p_person->Draw.Tweened->AngleTo - p_person->Draw.Tweened->Angle;
	SLONG dspeed = 300 - abs(dangle);

	SATURATE(dspeed, 0, 256);

	SLONG ratio = TICK_RATIO * dspeed >> 8;

	//
	// Don't do this ratio thing!
	//

	ratio = TICK_RATIO;

	dx = dx * ratio >> TICK_SHIFT;
	dz = dz * ratio >> TICK_SHIFT;

#ifndef PSX
	if(allow_debug_keys)
	if (ShiftFlag && Keys[KB_Q])
	{
		dx <<= 2;
		dz <<= 2;
	}
#endif
	//
	// Work out our new y-position.
	//

	if(p_person->OnFace)
	{
		if(p_person->OnFace>0)
		{
			on_face = calc_height_on_face(
						(p_person->WorldPos.X + dx) >> 8,
						(p_person->WorldPos.Z + dz) >> 8,
						p_person->OnFace,
					   &new_y);
		}
		else
		{
			on_face = calc_height_on_rface(
						(p_person->WorldPos.X + dx) >> 8,
						(p_person->WorldPos.Z + dz) >> 8,
						-p_person->OnFace,
					   &new_y);

		}

		if (!on_face)
		{
			//
			// we have walked off the face we were on, so we leave it to move thing to sort out what happens next
			//

			new_y=p_person->WorldPos.Y;
		}
		else
		{
			MSG_add("normal move height %d \n",new_y);
			new_y = (new_y ) <<8; //remove +4
		}
	}
	else
	{
/*
		if (p_person->Genus.Person->InsideIndex)
		{
//			ASSERT(p_person->Genus.Person->InsideIndex==1);
			new_y = get_inside_alt(p_person->Genus.Person->InsideIndex)<<8;
		}
		else
		if (p_person->Flags & FLAGS_IN_SEWERS)
		{
			new_y = NS_calc_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;
		}
		else
		{
			new_y = PAP_calc_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;
		}
*/
		{
			SLONG	mx,mz;

			mx=(p_person->WorldPos.X+dx) >> 16;
			mz=(p_person->WorldPos.Z+dz) >> 16;
			mx&=127;
			mz&=127;


			if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
				new_y = PAP_calc_height_at_thing(p_person,(p_person->WorldPos.X+0*dx)>>8,(p_person->WorldPos.Z+0*dz)>>8)<<8; //shit miked aug 2000 wasnt adding dx dz
			else
				new_y = PAP_calc_height_at_thing(p_person,(p_person->WorldPos.X+dx)>>8,(p_person->WorldPos.Z+dz)>>8)<<8; //shit miked aug 2000 wasnt adding dx dz
		}
	}

	//
	// Work out the change in our height.
	//

	dy = new_y - p_person->WorldPos.Y;

//	if(p_person->SubState!=SUB_STATE_WALKING)
	if(p_person->SubState!=SUB_STATE_SLIPPING)
	if(dy<-60<<8)
	{
		
//		p_person->Velocity=20;
		set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL);
		return;
	}
/*
	if(dy>16<<8)
	{
		if(p_person->SubState==SUB_STATE_WALKING||p_person->SubState==SUB_STATE_RUNNING)
		{
			//
			// don't ping up big steps, take them with style
			//
			dx=0;
			dz=0;
			dy=16<<8;
		}
	}
*/

//	if(abs(dy)>60<<8)
//		return;

//	ASSERT(abs(dy>>8)<256);
/*
	//
	// If our change-in-height is too big, then don't move.
	// 

	#define PERSON_MAX_HEIGHT_CHANGE 40

	if (!(p_person->Genus.Person->InsideIndex))
	if (dy > (PERSON_MAX_HEIGHT_CHANGE << 8))
	{
		return;
	}
*/
	//
	// Actually move the person.
	//

	if (dx || dz)
	{
/*
		SLONG	slope=0,angle;
		if(p_person->SubState!=SUB_STATE_SLIPPING && dy)
		{
			
			if(p_person->OnFace<0)
			{
			}
			else
			{

				slope=PAP_on_slope((p_person->WorldPos.X+dx)>>8,(p_person->WorldPos.Z+dz)>>8,&angle)>>1;
			}
		}
		if(slope<50)
*/
			move_thing(dx,dy,dz,p_person);
	}

	if(dy||p_person->Genus.Person->PlayerID)
	{
		check_on_slippy_slope(p_person);

/*
		SLONG	slope,angle;

		if(p_person->Genus.Person->InsideIndex)
		{
			slope=0;
		}
		else
		{
			if(p_person->OnFace<0)
			{
				slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
			}
			else
			{

				slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
			}
		}

		if(slope>50)
		{
			switch(p_person->SubState)
			{
				case	SUB_STATE_RUNNING:
				case	SUB_STATE_WALKING:
				case	SUB_STATE_WALKING_BACKWARDS:
				case	SUB_STATE_RUNNING_JUMP_LAND_FAST:
				case	SUB_STATE_RUNNING_JUMP_LAND:
					set_anim(p_person,ANIM_FALLING);
				case	SUB_STATE_SLIPPING:
					p_person->SubState=SUB_STATE_SLIPPING;
					p_person->Draw.Tweened->AngleTo=angle;

					slope=MIN(slope-50,40);
					slope=MAX(slope,10);


					change_velocity_to(p_person,slope);
					set_generic_person_state_function(p_person,STATE_MOVEING);

//					p_person->Velocity=MIN(slope-50,40);
					break;
			}
		}
		else
		if(p_person->SubState==SUB_STATE_SLIPPING)
		{
			p_person->SubState=SUB_STATE_SLIPPING_END;
			//set_person_idle(p_person);
		}
*/
	}
	else
	{
		if(p_person->SubState==SUB_STATE_SLIPPING)
		{
			SLONG	slope,angle;
			if(p_person->Genus.Person->InsideIndex)
			{
				slope=0;
			}
			else
			{

				if(p_person->OnFace<0)
				{
					slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
				}
				else
				{

					slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle)>>1;
				}
			}

			if(slope<=50)
				p_person->SubState=SUB_STATE_SLIPPING_END;
				//set_person_idle(p_person);

		}
	}

}

void person_normal_move(Thing *p_person)
{
	SLONG dx;
	SLONG dz;

	dx = -(SIN(p_person->Draw.Tweened->Angle) * p_person->Velocity) >> 8;
	dz = -(COS(p_person->Draw.Tweened->Angle) * p_person->Velocity) >> 8;
	person_normal_move_dxdz(p_person,dx,dz);
}

void person_normal_move_check(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG	x,z;

	dx = -(SIN(p_person->Draw.Tweened->Angle) * p_person->Velocity) >> 8;
	dz = -(COS(p_person->Draw.Tweened->Angle) * p_person->Velocity) >> 8;
	x=dx+p_person->WorldPos.X;
	z=dz+p_person->WorldPos.Z;

	if(x<0||z<0||x>=(128<<16)||z>=(128<<16))
		return;

	person_normal_move_dxdz(p_person,dx,dz);
}

SLONG	advance_keyframe(DrawTween *draw_info)
{
	SLONG	ret=0;
	draw_info->CurrentFrame	=	draw_info->NextFrame;
	if(draw_info->QueuedFrame)
	{
		draw_info->NextFrame	=	draw_info->QueuedFrame;
		draw_info->QueuedFrame	=	0;
		draw_info->FrameIndex	=	0;	 //if we are leaping into the middle of an anim then this should have a value
		ret=2; //anim has ended
		ASSERT(draw_info->CurrentFrame->FirstElement);
		ASSERT(draw_info->NextFrame->FirstElement);

	}
	else
	{
		if(draw_info->NextFrame->NextFrame)
		{
//			LogText(" do next frame \n");
			draw_info->NextFrame		=	draw_info->NextFrame->NextFrame;
			if(draw_info->CurrentFrame->Flags&ANIM_FLAG_LAST_FRAME)
				ret=1; // hopefully escape funny lock out situations by saying a looped anim has ended
			
		}
		else
			ret=1; //anim has ended
	}
	return(ret);
}

SLONG	retreat_keyframe(DrawTween *draw_info)
{
	SLONG	ret=0;
	draw_info->NextFrame	=	draw_info->CurrentFrame;
	if(draw_info->QueuedFrame)
	{
//		draw_info->NextFrame	=	draw_info->QueuedFrame;
//		draw_info->QueuedFrame	=	0;
//		draw_info->FrameIndex	=	0;	 //if we are leaping into the middle of an anim then this should have a value
	}
	else
	{
		if(draw_info->CurrentFrame->PrevFrame)
		{
//			LogText(" do next frame \n");
			draw_info->CurrentFrame		=	draw_info->CurrentFrame->PrevFrame;
		}
		else
		{
			ret=1; //anim has ended
//			MSG_add(" backwards anim ends \n");
		}
	}
	return(ret);
}
extern	void	calc_sub_objects_position_fix8(Thing *p_mthing,SLONG tween,UWORD object,SLONG *x,SLONG *y,SLONG *z);

void	move_locked_tween(Thing *p_person,DrawTween *dt,SLONG t1,SLONG t2)
{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;
		SLONG	dx,dy,dz;

//		calc_sub_objects_position(p_person,tween1,locked,&lock_x1,&lock_y1,&lock_z1);
//		calc_sub_objects_position(p_person,tween1,locked,&lock_x1,&lock_y1,&lock_z1);

		calc_sub_objects_position_fix8(p_person,   t1, abs(dt->Locked), &x1, &y1, &z1);
		calc_sub_objects_position_fix8(p_person,   t2, abs(dt->Locked), &x2, &y2, &z2);

//		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   t1, abs(dt->Locked), &x1, &y1, &z1);
//		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, t2, abs(dt->Locked), &x2, &y2, &z2);

		dx = (x1 - x2); //<<8;
		dy = (y1 - y2); //<<8;
		dz = (z1 - z2); //<<8;

		if(abs(dy)>600<<8)
		{
			ASSERT(0);
			calc_sub_objects_position_fix8(p_person,   t1, abs(dt->Locked), &x1, &y1, &z1);
			calc_sub_objects_position_fix8(p_person,   t2, abs(dt->Locked), &x2, &y2, &z2);

		}
		if(p_person->State==STATE_DANGLING)
		{
			move_thing_quick(dx,dy,dz,p_person);
			MSG_add(" mtq dx %d dy %d dz %d \n",dx,dy,dz);
		}
		else
		{
			move_thing(dx,dy,dz,p_person);
			MSG_add(" mt dx %d dy %d dz %d \n",dx,dy,dz);
		}
}


SLONG	person_normal_animate_speed(Thing *p_person,SLONG speed)
{
	SLONG	ret=0;
	DrawTween		*draw_info;
	SLONG	old_tween;
	SLONG	dx,dy,dz;
	SLONG	tween1,tween2;

//	speed=256;


	draw_info		=	p_person->Draw.Tweened;

	if(draw_info->CurrentFrame==0 || draw_info->NextFrame==0)
	{
		MSG_add(" !!!!!!!!!!!!!!!!!!!!!!error2 animate 0 frames \n");
		return(1);
	}
	old_tween		=	draw_info->AnimTween;

	{
		//SLONG	tween_step=speed/(draw_info->CurrentFrame->TweenStep+1);
		SLONG	tween_step=draw_info->CurrentFrame->TweenStep<<1;

		tween1=draw_info->AnimTween;
		tween_step  = (tween_step*TICK_RATIO)>>TICK_SHIFT;
		if(tween_step<=0)
			tween_step=1;
		draw_info->AnimTween += tween_step; //256/(draw_info->CurrentFrame->TweenStep+1);
		tween2=draw_info->AnimTween;
		if(draw_info->Locked&&draw_info->AnimTween<256)
		{
			move_locked_tween(p_person,draw_info,tween1,tween2);
		}
	}


	//
	// If anim has tweened over to the next frame
	//
//	while(draw_info->AnimTween>=256)
	while(tween2>=256)
	{
		SLONG	lock_x1,lock_y1,lock_z1,lock_x2,lock_y2,lock_z2;
//		MSG_add(" next frame \n");

//		draw_info->AnimTween	-=	256;
		tween2-=256;

		if(draw_info->NextFrame)
			tween2=(tween2*draw_info->NextFrame->TweenStep)/draw_info->CurrentFrame->TweenStep;
		draw_info->AnimTween	=	tween2;

		if(draw_info->CurrentFrame->Flags&ANIM_FLAG_LAST_FRAME)
		{
//			LogText(" at frame %d flagged as last frame \n",draw_info->FrameIndex);
			draw_info->FrameIndex=0;
		}
		else
		{
			draw_info->FrameIndex++;
		}

		if(draw_info->Locked)
		{
			GameCoord	temp_pos;
			SLONG	locked;

			locked=abs(draw_info->Locked);

			calc_sub_objects_position_fix8(p_person,tween1,locked,&lock_x1,&lock_y1,&lock_z1);
			ret|=advance_keyframe(draw_info);
			calc_sub_objects_position_fix8(p_person,tween2,draw_info->Locked,&lock_x2,&lock_y2,&lock_z2);

			dx=(+lock_x1-lock_x2);//<<8;
			dy=(+lock_x1-lock_x2);//<<8;
			dz=(+lock_x1-lock_x2);//<<8;

			if(p_person->State==STATE_DANGLING)
			{
				MSG_add("MOVE THING QUICK dx %d dy %d dz %d \n",dx,dy,dz);
				move_thing_quick(dx,dy,dz,p_person);
			}
			else
			{
				move_thing(dx,dy,dz,p_person);
				MSG_add("MOVE THING dx %d dy %d dz %d \n",dx,dy,dz);
			}

		}
		else
		{
			ret|=advance_keyframe(draw_info);
		}

		if(draw_info->CurrentFrame->Fight)
		{
//			PlaySample(THING_NUMBER(p_person),SAMPLE_HIT_MISS1,SAMPLE_VOL_MAX,SAMPLE_PAN_CENTER,SAMPLE_FREQ_ORIG+(GAME_TURN&0xfff),0);
			if(apply_violence(p_person)==0)
			{
#ifndef PSX
extern BOOL PLAYCUTS_playing;
#ifdef EDITOR
extern HWND			CUTSCENE_edit_wnd;
				if (!(CUTSCENE_edit_wnd||PLAYCUTS_playing))
#else
				if (!PLAYCUTS_playing)
#endif
#endif
			    MFX_play_thing(THING_NUMBER(p_person),S_KNIFE_START+(Random()&1),0,p_person);
				//
				// we missed
				//
//				p_person->Genus.Person->CombatNode=-p_person->Genus.Person->CombatNode;

			}

			//
			// Knock over barrels!
			//

			BARREL_hit_with_sphere(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Y >> 8,
				p_person->WorldPos.Z >> 8,
				0xa0);	// Bigger than normal
		}
	}

	if (ret == 1)
	{
	}



	return(ret);
}


SLONG	person_normal_animate(Thing *p_person)
{
	return(person_normal_animate_speed(p_person,256));
}

SLONG	person_backwards_animate(Thing *p_person)
{
	SLONG	ret=0;
	DrawTween		*draw_info;
	SLONG	old_tween;
	SLONG	tween_step;
	
	draw_info		=	p_person->Draw.Tweened;

	if(draw_info->CurrentFrame==NULL)
	{
		MSG_add(" backwards anim crash");
		return(1);

	}
	//tween_step=256/(draw_info->CurrentFrame->TweenStep+1);
	tween_step=draw_info->CurrentFrame->TweenStep<<1;

	tween_step=(tween_step*TICK_RATIO)>>TICK_SHIFT;



	old_tween		=	draw_info->AnimTween;

	draw_info->AnimTween	-=	tween_step; //256/(draw_info->CurrentFrame->TweenStep+1);

	//LogText(" animate person tween %d  current_anim %d currentframe %x\n",draw_info->AnimTween,draw_info->CurrentAnim,draw_info->CurrentFrame);

	//
	// If anim has tweened over to the next frame
	//
	while(draw_info->AnimTween<0)
	{
		SLONG	lock_x1,lock_y1,lock_z1,lock_x2,lock_y2,lock_z2;

		draw_info->AnimTween	+=	256;

		if(draw_info->CurrentFrame->Flags&ANIM_FLAG_LAST_FRAME)
		{
			draw_info->FrameIndex=0;
		}
		else
		{
			draw_info->FrameIndex++;
		}

		if(draw_info->Locked)
		{
			GameCoord	temp_pos;
			SLONG	locked;
			//
			// Part of body is locked in place so across anim maintain its location
			//

			locked=abs(draw_info->Locked);

			calc_sub_objects_position(p_person,0,locked,&lock_x1,&lock_y1,&lock_z1);
			calc_sub_objects_position(p_person,256,locked,&lock_x2,&lock_y2,&lock_z2);
			ret=retreat_keyframe(draw_info);

			//
			// difference in lock co-ord is ammount to move by to maintain same position for limb
			// 

			temp_pos.X=((+lock_x1-lock_x2)<<8)+p_person->WorldPos.X;
			temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
			temp_pos.Z=((+lock_z1-lock_z2)<<8)+p_person->WorldPos.Z;

			//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
			//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);
		
			move_thing_on_map(p_person,&temp_pos);
		}
		else
			ret=retreat_keyframe(draw_info);

		if(draw_info->CurrentFrame->Fight)
		{
			apply_violence(p_person);
		}

	}
//	MSG_add(" backwards returns %d \n",ret);
	return(ret);
}

void	camera_shoot(void)
{
	/*

	Thing *darci;

	SLONG map_x;
	SLONG map_z;

	darci = NET_PERSON(0);

	map_x = darci->WorldPos.X >> 16;
	map_z = darci->WorldPos.Z >> 16;

	if (PAP_2HI(map_x, map_z).Flags & PAP_FLAG_NAUGHTY)
	{
		//
		// No point doing this indoors...
		//
	}
	else
	{
		CAM_set_mode(CAM_MODE_SHOOT_NORMAL);
		CAM_set_zoom(0x320);
		CAM_set_behind_up(0x30000);
		CAM_lens = CAM_LENS_NORMAL;
	}
	*/
}

void	camera_fight(void)
{
	
/*
	Thing *darci;

	SLONG map_x;
	SLONG map_z;

	darci = NET_PERSON(0);

	map_x = darci->WorldPos.X >> 16;
	map_z = darci->WorldPos.Z >> 16;

	if (PAP_2HI(map_x, map_z).Flags & PAP_FLAG_NAUGHTY)
	{
		//
		// No point doing this indoors...
		//
	}
	else
	{
		CAM_set_mode(CAM_MODE_FIGHT_NORMAL);
		CAM_set_zoom(0x320);
		CAM_set_behind_up(0x70000);
		CAM_lens = CAM_LENS_NORMAL;
	}
*/
	
}

void	camera_normal(void)
{
/*	
#ifndef PSX
	Thing *darci;

	SLONG map_x;
	SLONG map_z;

	darci = NET_PERSON(0);

	map_x = darci->WorldPos.X >> 16;
	map_z = darci->WorldPos.Z >> 16;

	if (PAP_2HI(map_x, map_z).Flags & PAP_FLAG_NAUGHTY)
	{
		CAM_set_mode(CAM_MODE_BEHIND);
		CAM_set_zoom(0x340 * 180 / 256);
		CAM_set_behind_up(0x18000 * 100 / 256);
		CAM_lens = CAM_LENS_WIDEANGLE;
		CAM_set_collision(FALSE);
	}
	else
	{
		CAM_set_type(CAM_get_type());
	}
#endif
*/	
}


void set_person_aim(Thing *p_person,SLONG locked=0)
{
	SLONG  anim;
	Thing *p_special;

	if (p_person->Genus.Person->SpecialUse)
	{
		//
		// It must be a two-handed shotgun weapon.
		//
/*
		if ((p_person->Genus.Person->PersonType==PERSON_ROPER)&&((p_person->Draw.Tweened->PersonID>>5)==5)) {
			// ...but roper's so fucking 'ard he uses an ak47 one-handed.
			// of course, it should break his arm when he fires, but hey.
			anim = ANIM_AK_AIM;
		} else {
			anim = ANIM_SHOTGUN_AIM;
		}
*/
		anim = ANIM_SHOTGUN_AIM;

	}
	else
	{
		anim = ANIM_PISTOL_AIM_AHEAD;
	}

	set_generic_person_state_function(p_person,STATE_GUN);
	if(locked)
		set_locked_anim(p_person,anim,locked-1);
	else
		set_anim(p_person,anim);

	p_person->Genus.Person->Action  =   ACTION_AIM_GUN;
	p_person->SubState              =   SUB_STATE_AIM_GUN;
	p_person->Genus.Person->Flags  &= ~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
//	p_person->Genus.Person->Flags  |=  (FLAG_PERSON_NON_INT_C);
}

inline	SLONG	weapon_accuracy_at_dist(Thing *p_person,SLONG dist)
{
	if(dist<0)
		return(dist);

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		//
		// Using a special.
		//

		switch (p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_AK47:
				if(!p_person->Genus.Person->PlayerID)
				{
					//
					// AI needs high skill to use this well
					//

					return((dist*(280-(GET_SKILL(p_person)<<2)))>>8);
				}
				else
				{
					return((dist*200)>>8);
				}
				break;

			case SPECIAL_SHOTGUN:
				return(dist>>2);
				break;

			default:
				ASSERT(0);
				return 0;
		}

	}
	else
	{
		//
		// This must be the gun.
		//

		return(dist);
	}
}

//
// Returns the damage a person should do when he shoots his target.
// A damage of zero means that he missed the person.
//

UBYTE VehicleBelongsToMIB(Thing *p_target)
{
	Vehicle *veh = p_target->Genus.Vehicle;
	Thing *thing;
	SWORD passenger;

	if ((p_target->Class!=CLASS_VEHICLE)||!veh) return 0; // it's not even a vehicle, so it can't possibly be...

	if (veh->Driver)
	{
		thing=TO_THING(veh->Driver);
		if (PersonIsMIB(thing)) return 1;				  // it's being driven by a MIB
	}
	passenger=veh->Passenger;
	while (passenger)
	{
		thing=TO_THING(passenger);
		if (PersonIsMIB(thing)) return 1;				 // a MIB is riding shotgun
	    passenger=thing->Genus.Person->Passenger;
	}
	return 0;											 // nope, no MIBs
}


SLONG get_shoot_damage(Thing *p_person, Thing *p_target,SLONG *gun_type)
{
	SLONG damage;

	SLONG dx   = abs(p_target->WorldPos.X - p_person->WorldPos.X >> 8);
	SLONG dz   = abs(p_target->WorldPos.Z - p_person->WorldPos.Z >> 8);
	SLONG dist = QDIST2(dx,dz);

	//
	// Work out the chance of hitting our target.
	//

	{
		SLONG chance;	// 0 => Never hit, (>= 256) => Definitely hit

		if (p_target->Class == CLASS_VEHICLE)
		{
			//
			// Vehicle are too big not to miss!
			// 

/*			apparently, we don't want this. cool, that way I don't have to test it... :}

			if (VehicleBelongsToMIB(p_target))
			{
				chance=0;
				// do some special effect here...
			}
			else*/
				chance = 256;
		}
		else
		if (p_target->Class == CLASS_BAT)
		{
			//
			// Bats are tricky to hit!
			//

//			chance = 128;
			chance = 200; //bats are just an annoyance, kill em easy just for fun, don't want to watse your ammo on these damn things
		}
		else
		if (p_target->Class == CLASS_BARREL)
		{
			//
			// Barrels tend to not move! So they are easy to hit.
			//

			chance = 200;
		}
		else
		if (p_target->Class == CLASS_SPECIAL)
		{
			//
			// Mines are really small!
			//

//			chance = 128;
			chance = 250;   //but you get to take a long time aiming sng get a clear shot against a stationary target
		}
		else
		if (p_target->Class == CLASS_PERSON)
		{
			//
			// People are trickier to hit if they are turning. A players
			// Roll maxes out at about 100.
			//

			chance = 230 - (abs(p_target->Draw.Tweened->Roll)>>1);	  //180 for full turning person

			//
			//Moving person harder to hit
			//
			chance -=p_target->Velocity;                          //150 for run or down to 130 with sprint


			//
			// Non-player characters are a bad aim!
			//

			if (!p_person->Genus.Person->PlayerID)
			{
				if(p_target->Genus.Person->PlayerID)
				{
					chance -= 64;  // enemies accuracy reduced for shooting player 
				}
				else
				{
					chance+=100; //AI is very good at shooting other AI
				}
				chance +=GET_SKILL(p_person)<<3;  // add on their skill factor

			}
			else
			{
				chance+=64; // player accuracy should be better than it currently is
			}
			if(p_target->SubState==SUB_STATE_FLIPING)
			{
				chance-=96;
			}
		}
		else
		{
			//
			// This is an invalid target! It could be that the target was a
			// barrel or a mine that was freed up before the person noticed.
			//

			p_person->Genus.Person->Target = NULL;

			return 0;
		}


		//
		// Make the chance of hitting the target depend on distance.
		//

		if(p_target->Genus.Person->PlayerID)
		{
			dist  -= 0x2a0;
		}
		else
		{
			dist  -= 0x400;
		}
		if(dist<0)
		{
			//
			// -ve values are subtracted directly from chance
			// 	so if you are at zero distance then chance will be 100%
			//	2 blocks awayt is a 50% bonus
			//  3 blocks is a 25% bonus
			//  4 blocks break even
			dist >>= 2;
		}
		else
		{
			dist >>= 3;
		}

		// dist is 0->8 blocks say  so this is 0->(256*8-300)/8
		// dist =0 ->218

		//
		// At 0x300 distance chance is unchanged. At 0x600 distance, chance is reduced
		// by 37.5%
		// 

		{
			SLONG	dchance;
			dchance=chance;
			chance -= weapon_accuracy_at_dist(p_person,dist);
//			PANEL_new_text(p_person, 2000, "Chance of hitting you is%d%% with dist %d%% at dist %d",dchance * 100 >> 8, chance * 100 >> 8,dist);
		}

		if (p_target->State    == STATE_MOVEING &&
			p_target->SubState == SUB_STATE_FLIPING)
		{
			chance >>= 1;
		}

		SATURATE(chance, 20, 256);  //always have a slim hope surely
	
		//
		// There is a chance the person will miss
		// 

		if (chance < (Random() & 0xff))
		{
			//
			// A miss!
			//

			return 0;
		}
	}

	if(p_target->Genus.Player->PlayerID)
	{
		dist-=(300>>3);
	}


	//
	// A hit. The damage depends on the gun.
	// 

	if (PersonIsMIB(p_person))
	{
		//
		// Men In Black all have AK47s
		//

	   *gun_type = HIT_TYPE_GUN_SHOT_AK47;
		damage   = 40;	// Rapid fire...
	}
	else
	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		//
		// Using a special.
		//

		switch (p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_AK47:
			   *gun_type=HIT_TYPE_GUN_SHOT_AK47;

				if (p_person->Genus.Person->PlayerID)
				{
					damage = 100;
				}
				else
				{
					damage = 40;	// Rapid fire...
				}
				break;

			case SPECIAL_SHOTGUN:
			   *gun_type=HIT_TYPE_GUN_SHOT_SHOTGUN;
			   //
			   // shot gun depends on distance, lethal up close
			   //

			   // max range = (0x600-0x400)>>3 = 0x40   0x40<<2= 100
				if(p_target->Genus.Player->PlayerID || p_person->Genus.Person->PlayerID)
				{
					dist<<=2;
				}
				else
				{
					//
					// AI characters shooting each other can do lots of damage
					//
				}
				damage = 300-(dist);//<<2);	// One shot kills. was 250
				SATURATE(damage,0,250);

				break;

			default:
				ASSERT(0);
				return 0;
		}

	}
	else
	{
		//
		// This must be the gun.
		//

		ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT);

	   *gun_type = HIT_TYPE_GUN_SHOT_PISTOL;
	    damage   = 70;	// Three shots kill...
	}

	if (p_target->Genus.Person->PlayerID)
	{
		damage >>= 1;
	}

	return damage;
}


#define NOT_A_GUN_YOU_SHOOT (-1)
#define HAD_TO_CHANGE_CLIP  (-2)

SLONG shoot_get_ammo_sound_anim_time(Thing *p_person,SLONG *sound,SLONG *anim,SLONG *time)
{
	SLONG ammo = FALSE;
	SLONG ammo_in_clip;

	if (PersonIsMIB(p_person))
	{
		//
		// MIB all have ak47s with unlimited ammo!
		//

       *anim  = ANIM_AK_FIRE;
	    ammo  = 50;
	   *time  = 64;
       *sound = S_MIB_GUN_WDOWN;

		return ammo;
	}

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		//
		// All specials use the shotgun sound?
		//

       *sound = S_SHOTGUN_SHOT;

		//
		// Using a special.
		//

		switch (p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_AK47:

				if (p_person->Genus.Person->PlayerID == 0)
				{
					//
					// Enemies never run out of AK ammo.
					//

					p_special->Genus.Special->ammo ++;

				}

				*anim = ANIM_AK_FIRE;
				*time = 64;
//				*sound = S_AK47_SHOT; // <- more 'accurate' but not as 'fun'
				*sound = S_AK47_BURST;// <- sounds better overall
				break;

			case SPECIAL_SHOTGUN:
				*anim = ANIM_SHOTGUN_FIRE;
				*time = 400;
#ifndef BUILD_PSX
				DIRT_new_sparks(p_person->Genus.Person->GunMuzzle.X>>8,p_person->Genus.Person->GunMuzzle.Y>>8,p_person->Genus.Person->GunMuzzle.Z>>8,2|32);
#endif
				break;

			case SPECIAL_GRENADE:
				
				//
				// This is the way non-players throw grenades...
				//

				SPECIAL_prime_grenade(p_special);

				set_person_can_release(p_person, 128);

				return NOT_A_GUN_YOU_SHOOT; 

			default:

				//
				// This isn't a weapon you shoot!
				//

				return NOT_A_GUN_YOU_SHOOT;
		}

		//
		// Any ammo?
		//

		if (p_special->Genus.Special->ammo)
		{
			p_special->Genus.Special->ammo -= 1;

			//
			// We have ammo.
			//

			ammo = TRUE;
		}
		else
		{
			switch (p_special->Genus.Special->SpecialType)
			{
				case SPECIAL_AK47:
					if(p_person->Genus.Person->ammo_packs_ak47)
					{
						ammo_in_clip=p_person->Genus.Person->ammo_packs_ak47;
						ammo_in_clip-=SPECIAL_AMMO_IN_A_AK47;
						if (ammo_in_clip<0)
						{
							p_special->Genus.Special->ammo = p_person->Genus.Person->ammo_packs_ak47;
							p_person->Genus.Person->ammo_packs_ak47 = 0;
						} 
						else
						{
							p_special->Genus.Special->ammo = SPECIAL_AMMO_IN_A_AK47;
							p_person->Genus.Person->ammo_packs_ak47 = ammo_in_clip;
						}
						
						ammo = HAD_TO_CHANGE_CLIP;

					}
					break;
				case SPECIAL_SHOTGUN:
					if(p_person->Genus.Person->ammo_packs_shotgun)
					{
						ammo_in_clip=p_person->Genus.Person->ammo_packs_shotgun;
						ammo_in_clip-=SPECIAL_AMMO_IN_A_SHOTGUN;
						if (ammo_in_clip<0)
						{
							p_special->Genus.Special->ammo = p_person->Genus.Person->ammo_packs_shotgun;
							p_person->Genus.Person->ammo_packs_shotgun = 0;
						} 
						else
						{
							p_special->Genus.Special->ammo = SPECIAL_AMMO_IN_A_SHOTGUN;
							p_person->Genus.Person->ammo_packs_shotgun = ammo_in_clip;
						}
						
						ammo = HAD_TO_CHANGE_CLIP;

					}
					break;
			}
		}
	}
	else
	{
		//
		// Using the pistol.
		//

	   *sound = SOUND_Range(S_PISTOL_SHOT,S_PISTOL_SHOT_END);
	   *time  = 140;

		if (p_person->Genus.Person->Ammo)
		{	
			p_person->Genus.Person->Ammo--;
			
			//
			// We have ammo.
			//

			ammo  = TRUE;
		   *anim  = ANIM_PISTOL_SHOOT;
		}
		else
		{
/*			if(p_person->Genus.Person->ammo_packs_pistol)
			{
				p_person->Genus.Person->ammo_packs_pistol--;
				p_person->Genus.Person->Ammo = SPECIAL_AMMO_IN_A_PISTOL;
				ammo = HAD_TO_CHANGE_CLIP;
			}*/
			if(p_person->Genus.Person->ammo_packs_pistol)
			{
				ammo_in_clip=p_person->Genus.Person->ammo_packs_pistol;
				ammo_in_clip-=SPECIAL_AMMO_IN_A_PISTOL;
				if (ammo_in_clip<0)
				{
					p_person->Genus.Person->Ammo = p_person->Genus.Person->ammo_packs_pistol;
					p_person->Genus.Person->ammo_packs_pistol = 0;
				} 
				else
				{
					p_person->Genus.Person->Ammo = SPECIAL_AMMO_IN_A_PISTOL;
					p_person->Genus.Person->ammo_packs_pistol = ammo_in_clip;
				}
				
				ammo = HAD_TO_CHANGE_CLIP;

			}

		}
	}

	return ammo;
}

void	actually_fire_gun(Thing *p_person)
{
	WaveParams	shot;
	SLONG	rico_id;
	//	Set up the wave params.

	shot.Priority				=	0;
	shot.Flags					=	WAVE_CARTESIAN;
	shot.Mode.Cartesian.Scale	=	(128<<8);
	shot.Mode.Cartesian.X		=	p_person->WorldPos.X;
	shot.Mode.Cartesian.Y		=	p_person->WorldPos.Y;
	shot.Mode.Cartesian.Z		=	p_person->WorldPos.Z;

	PCOM_oscillate_tympanum(
		PCOM_SOUND_GUNSHOT,
		p_person,
		p_person->WorldPos.X >> 8,
		p_person->WorldPos.Y >> 8,
		p_person->WorldPos.Z >> 8);

	if (p_person->Genus.Person->PlayerID)
	{
		GAME_FLAGS |= GF_PLAYER_FIRED_GUN;
		if (p_person->Genus.Person->Target)
			timer_bored=0;
	}

	{
		//
		// A dynamic light muzzle flash that only lasts one frame.
		//

		UBYTE dlight;

		dlight = NIGHT_dlight_create(
					(p_person->WorldPos.X >> 8) - (SIN(p_person->Draw.Tweened->Angle) >> 9),
					(p_person->WorldPos.Y >> 8) + 0x60,
					(p_person->WorldPos.Z >> 8) - (COS(p_person->Draw.Tweened->Angle) >> 9),
					100,
					30,
					25,
					5);
		
		if (dlight)
		{
			NIGHT_dlight[dlight].flag |= NIGHT_DLIGHT_FLAG_REMOVE;
		}
	}

#ifndef BUILD_PSX

extern void DIRT_create_brass(SLONG x,SLONG y,SLONG z,SLONG angle);

	{
		GameCoord vec;

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			SUB_OBJECT_LEFT_HAND,
		   &vec.X,
		   &vec.Y,
		   &vec.Z);
		vec.X+=p_person->WorldPos.X>>8;
		vec.Y+=p_person->WorldPos.Y>>8;
		vec.Z+=p_person->WorldPos.Z>>8;
		DIRT_create_brass(vec.X,vec.Y,vec.Z,(p_person->Draw.Tweened->Angle+512)&2047);
		if (p_person->Genus.Person->SpecialUse||PersonIsMIB(p_person))
		{
			Thing* p_special = TO_THING(p_person->Genus.Person->SpecialUse);
		    if ((!p_person->Genus.Person->SpecialUse)||(p_special->Genus.Special->SpecialType==SPECIAL_AK47))
			{
				// go OTT with AK...
				DIRT_create_brass(vec.X,vec.Y,vec.Z,(p_person->Draw.Tweened->Angle+512)&2047);
				DIRT_create_brass(vec.X,vec.Y,vec.Z,(p_person->Draw.Tweened->Angle+512)&2047);
			}
		}
	}
#else
#ifdef	PSX
	if (p_person->Genus.Person->PlayerID)
		PSX_SetShock(1,0);	// Just shock the fast one
#endif
#endif
#ifdef TARGET_DC
	if (p_person->Genus.Person->PlayerID)
	{

		if (p_person->Genus.Person->SpecialUse)
		{
			Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);
			switch (p_special->Genus.Special->SpecialType)
			{
				case SPECIAL_AK47:
					Vibrate ( 4.0f, 1.0f, 0.0f );
					break;
				case SPECIAL_SHOTGUN:
					Vibrate ( 6.5f, 1.0f, 0.0f );
					break;
			}
		}
		else
		{
			// It's a pistol.
			Vibrate ( 8.0f, 1.0f, 0.0f );
		}
	}
#endif


	if (p_person->Genus.Person->Target)
	{
		SLONG  damage;
		Thing *p_target = TO_THING(p_person->Genus.Person->Target);
		GameCoord vec;
		SLONG	gun_type;
/*
		if (p_target->Class == CLASS_PERSON && is_person_ko(p_target))
		{
			return;
		}
*/

		if (p_target->Class == CLASS_PERSON && (p_target->Genus.Person->Flags & (FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER)))
		{
			//
			// Shoot the car this person is driving.
			// 

			p_person->Genus.Person->Target = p_target->Genus.Person->InCar;
			p_target                       = TO_THING(p_target->Genus.Person->InCar);
		}

		damage = get_shoot_damage(p_person, p_target, &gun_type);

		if (damage)
		{
			//
			// We have hit our target.
			//

			PYRO_hitspang(p_person, p_target);
			timer_bored=0;

			if (p_target->Class == CLASS_PERSON )
			{
				if (!p_target->Genus.Person->PlayerID)
				{

					if (p_target->Genus.Person->Health > 0 && !is_person_ko(p_target))
					{
						SLONG skill = GET_SKILL(p_target);

						if (PersonIsMIB(p_person))
						{
							//
							// MIB are expert bullet dodgers!
							//

							skill += 5;
						}

						//
						// High skill level people can dodge bullets!
						//

						if (skill >= 7)
						{
							skill -= 5;

							if (p_person->Genus.Person->SpecialUse)
							{
								Thing* p_special = TO_THING(p_person->Genus.Person->SpecialUse);
								if (p_special->Genus.Special->SpecialType==SPECIAL_SHOTGUN)
								{
									skill-=5; // you have to be at least level 10 to dodge a wide shotgun spread, mike.
								}
							}

							if ((Random() & 0x1f) < skill )
							{
								PCOM_attack_happened(p_target, p_person);

								set_person_flip(p_target, Random() & 0x1);

								return;
							}
						}
					}
				}
/*
				else
				{
					timer_bored=0;
				}
*/				
				// do a lil bloodsplat
#ifndef PSX
				if(VIOLENCE)
				{
					calc_sub_objects_position(
						p_target,
						p_target->Draw.Tweened->AnimTween,
						SUB_OBJECT_LEFT_HAND,
					   &vec.X,
					   &vec.Y,
					   &vec.Z);
					vec.X+=(Random()&0x1f);
					vec.Y+=(Random()&0x1f);
					vec.Z+=(Random()&0x1f);
					vec.X<<=8; vec.Y<<=8; vec.Z<<=8;
					vec.X+=p_target->WorldPos.X;
					vec.Y+=p_target->WorldPos.Y;
					vec.Z+=p_target->WorldPos.Z;
					PARTICLE_Add(vec.X,vec.Y,vec.Z,0,0,0,
						POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7FFF0000,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE,10,75,1,20,5);
				}
#endif
				apply_hit_to_person(
					p_target,
					0,	// angle
					gun_type,
					damage,
					p_person,
					NULL);	// Fight frame

				if (p_person->Genus.Person->PlayerID)
				{
					//
					// Target someone else once you've killed someone.
					//
#ifdef PSX
					PSX_SetShock(1,damage+32);
#endif
#ifdef TARGET_DC
					// Eh? Why should this vibrate? I've already vibrated from the gunshot -
					// just because you hit or missed the person, or did them more or less
					// damage, doesn't mean you should vibrate as well.
					// Madness.
					//Vibrate ( 5.0f, (float)(damage+32) * 0.01f, 0.0f );
#endif

					if (p_target->Genus.Person->Health <= 0)
					{
						p_person->Genus.Person->Target = 0;
					}
				}
			}
			else
			if (p_target->Class == CLASS_BAT)
			{
				BAT_apply_hit(
					p_target,
					p_person,
					damage);
			}
			else
			if (p_target->Class == CLASS_VEHICLE)
			{
				//
				// Not another full recompile for me!
				//

				extern void VEH_reduce_health(Thing *p_car, Thing *p_person, SLONG damage);

				VEH_reduce_health(
					p_target,
					p_person,
					damage);

				p_target->Genus.Vehicle->Flags |= FLAG_VEH_SHOT_AT;	// Tell the vehicle it is being attacked.
			}
			else
			if (p_target->Class == CLASS_SPECIAL)
			{
				//
				// This must be a mine!
				//

				void special_activate_mine(Thing *p_mine);

				special_activate_mine(p_target);
			}
			else
			{
				ASSERT(p_target->Class == CLASS_BARREL);

				BARREL_shoot(
					p_target,
					p_person);
			}
		}
		else
		{
			//
			// Miss!
			//

			shot.Mode.Cartesian.X -= (SIN(p_person->Draw.Tweened->Angle)*1024)>>8;
			shot.Mode.Cartesian.Z -= (COS(p_person->Draw.Tweened->Angle)*1024)>>8;

			rico_id	= ((Random()*(S_RICOCHET_END-S_RICOCHET_START))>>16)+S_RICOCHET_START;

			MFX_play_xyz(0,rico_id,0,shot.Mode.Cartesian.X,shot.Mode.Cartesian.Y,shot.Mode.Cartesian.Z);

			//
			// Where did the bullet hit if it didn't hit the target?
			//

			SLONG hitx;
			SLONG hity;
			SLONG hitz;

			SLONG b_index = THING_find_nearest(
								p_target->WorldPos.X >> 8,
								p_target->WorldPos.Y >> 8,
								p_target->WorldPos.Z >> 8,
								0xa0,
								1 << CLASS_BARREL);

			if (b_index)
			{
				Thing *p_barrel = TO_THING(b_index);

				hitx = p_barrel->WorldPos.X >> 8;
				hity = p_barrel->WorldPos.Y >> 8;
				hitz = p_barrel->WorldPos.Z >> 8;

				BARREL_shoot(
					p_barrel,
					p_person);
			}
			else
			{
				hitx = p_target->WorldPos.X + (Random() & 0x1fff) - 0xfff;
				hitz = p_target->WorldPos.Z + (Random() & 0x1fff) - 0xfff;

				hity = PAP_calc_map_height_at(hitx >> 8, hitz >> 8) + 0x1000;
			}

			PYRO_hitspang(
				p_person,
				hitx, 
				hity,
				hitz);
		}
	}
	else
	{
		//
		// Shoot a coke can?
		//

		if (DIRT_shoot(p_person))
		{
			shot.Mode.Cartesian.X	-=	(SIN(p_person->Draw.Tweened->Angle)*1024)>>8;
			shot.Mode.Cartesian.Z	-=	(COS(p_person->Draw.Tweened->Angle)*1024)>>8;

//				PlayWave(THING_NUMBER(p_person),S_PISTOL_SHOT,WAVE_PLAY_QUEUE,&shot);
//				play_quick_wave_old(&shot,S_PISTOL_SHOT,0,0);
			MFX_play_xyz(0,S_PISTOL_SHOT,0,shot.Mode.Cartesian.X,shot.Mode.Cartesian.Y,shot.Mode.Cartesian.Z);

		}
		else
		{
			shot.Mode.Cartesian.X	-=	(SIN(p_person->Draw.Tweened->Angle)*1024)>>8;
			shot.Mode.Cartesian.Z	-=	(COS(p_person->Draw.Tweened->Angle)*1024)>>8;

			rico_id	=	((Random()*(S_RICOCHET_END-S_RICOCHET_START))>>16)+S_RICOCHET_START;
//				PlayWave(THING_NUMBER(p_person),rico_id,WAVE_PLAY_QUEUE,&shot);
			MFX_play_xyz(0,rico_id,0,shot.Mode.Cartesian.X,shot.Mode.Cartesian.Y,shot.Mode.Cartesian.Z);
//				play_quick_wave_old(&shot,rico_id,0,0);

			//
			// Find where the bullet hit...
			//

			{
				SLONG endx = p_person->WorldPos.X - (SIN(p_person->Draw.Tweened->Angle) << 2) >> 8;
				SLONG endy = p_person->WorldPos.Y + 0x6000                                    >> 8;
				SLONG endz = p_person->WorldPos.Z - (COS(p_person->Draw.Tweened->Angle) << 2) >> 8;

				if (there_is_a_los(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						endx,
						endy,
						endz,
						0))
				{
					PYRO_hitspang(
						p_person,
						endx << 8,
						endy << 8,
						endz << 8);
				}
				else
				{
					PYRO_hitspang(
						p_person,
						los_failure_x << 8,
						los_failure_y << 8,
						los_failure_z << 8);
				}
			}
		}
	}

}
										    
void	set_person_running_shoot(Thing *p_person)
{
	SLONG ammo,sound,anim,time;

	if (p_person->Genus.Person->Timer1)
	{
		//
		// Too little time since the last shot.
		//

		return;
	}

	ammo = shoot_get_ammo_sound_anim_time(p_person,&sound,&anim,&time);

	if (ammo == NOT_A_GUN_YOU_SHOOT)
	{
		return;
	}

	if (!ammo || ammo == HAD_TO_CHANGE_CLIP)
	{
		MFX_play_thing(THING_NUMBER(p_person),S_PISTOL_DRY,MFX_REPLACE,p_person);
		return;
	}
	
	MFX_play_thing(THING_NUMBER(p_person),sound,MFX_REPLACE,p_person);

	actually_fire_gun(p_person);

	//
	// Make sure this person can't fire the gun again for a while...
	//

	p_person->Genus.Person->Timer1 = time;
}


//
// Returns the best type of special a person has that's got ammo.
//

SLONG get_persons_best_weapon_with_ammo(Thing *p_person)
{
	Thing *p_special;

	//
	// The weapon order is:
	//
	//   AK47
	//   shotgun
	//   gun
	// 	 baseball bat
	// 	 knife
	//

	static UBYTE weapon_order[5] =
	{
		SPECIAL_AK47,
		SPECIAL_SHOTGUN,
		SPECIAL_GUN,
		SPECIAL_BASEBALLBAT,
		SPECIAL_KNIFE		
	};

	SLONG i;

	for (i = 0; i < 5; i++)
	{
		if (i == 2)
		{
			if (p_person->Flags & FLAGS_HAS_GUN)
			{
				if (p_person->Genus.Person->Ammo)
				{
					return SPECIAL_GUN;
				}
			}
		}
		else
		{
			if (p_special = person_has_special(p_person, weapon_order[i]))
			{
				if (i < 2)
				{
					if (p_special->Genus.Special->ammo)
					{
						return weapon_order[i];
					}
				}
				else
				{
					return weapon_order[i];
				}
			}
		}
	}

	return SPECIAL_NONE;
}


//
// Returns TRUE if you shouldn't hit this person due to
// a cutscene playing.
//

SLONG dont_hurt_target_during_cutscene(Thing *p_person, Thing *p_target)
{
	if (!p_person->Genus.Person->PlayerID)
	{
		if (p_target->Class == CLASS_PERSON)
		{
			SLONG dont_shoot_in_a_cutscene = FALSE;

			if (p_target->Genus.Person->PlayerID)
			{
				dont_shoot_in_a_cutscene = TRUE;
			}
			else
			if (p_target->Genus.Person->pcom_move == PCOM_MOVE_FOLLOW)
			{
				//
				// Dont shoot people in a cutscene who are following the player.
				//

				UWORD i_follow = EWAY_get_person(p_person->Genus.Person->pcom_move_follow);

				if (i_follow)
				{
					Thing *p_follow = TO_THING(i_follow);

					if (p_follow->Class == CLASS_PERSON && p_follow->Genus.Person->PlayerID)
					{
						dont_shoot_in_a_cutscene = TRUE;
					}
				}
			}

			if (dont_shoot_in_a_cutscene && EWAY_stop_player_moving())
			{
				//
				// Don't shoot.
				//

				return TRUE;
			}
		}			
	}

	return FALSE;
}









void	set_person_shoot(Thing *p_person,UWORD shoot_target)
{
	SLONG		dx,dz;
	SLONG		anim = ANIM_PISTOL_SHOOT;
	SLONG       ammo = FALSE;
	SLONG		sound;
	SLONG		time;

	if (p_person->State == STATE_CARRY)
	{
		//
		// Can't shoot while carrying someone.
		//

		return;
	}

	if(might_i_be_a_villain(p_person))
	{
		//
		// arrest anyone who might be a villain
		//
		PCOM_call_cop_to_arrest_me(p_person,1);
	}

	if(p_person->SubState==SUB_STATE_RUNNING)
	{
		set_person_running_shoot(p_person);
		return;
	}

	p_person->Genus.Person->Timer1=0;  // used for ak47 to limit continuos fire

	if (p_person->Genus.Person->PlayerID)
	{
		//
		// For players...
		//

		if (p_person->Genus.Person->Target)
		{
			Thing *p_target;

			p_target = TO_THING(p_person->Genus.Person->Target);

			//
			// Players twist their body towards their target- but because they
			// can't shoot and twist at the same time, turn them to face their
			// target.
			//

//			set_face_thing(p_person, TO_THING(p_person->Genus.Person->Target));

			if (p_target->Class == CLASS_PERSON)
			{
				//
				// You get punished for killing innocents nowadays...   
				//
				
				// PANEL_new_help_message("I, Mark Adami, think you should be able to shoot unarmed people.");
				if(p_person->Genus.Person->PersonType==PERSON_DARCI)
				{
					if(p_target->Draw.Tweened->CurrentAnim==ANIM_HANDS_UP||p_target->Draw.Tweened->CurrentAnim==ANIM_HANDS_UP_LOOP)
					{
//						PANEL_new_text(p_person,8000,"Get Down.");
						PANEL_new_text(p_person,8000,XLAT_str(X_GET_DOWN));
						set_person_dead(p_target,p_person,PERSON_DEATH_TYPE_GET_DOWN,0,0);

						//
						// Look for a new target now...
						//

						p_person->Genus.Person->Target = NULL;

					//	set_person_get_down(p_target);
						return; //leave this here its good for you.
					}
					if(p_target->Genus.Person->PersonType==PERSON_COP && !(p_target->Genus.Person->Flags2&FLAG2_PERSON_GUILTY))
					{
						
//						PANEL_new_text(p_person,8000,"I can't shoot a Cop.");
						PANEL_new_text(p_person,8000,XLAT_str(X_CANT_SHOOT_COP));

						return; //leave this here its good for you.

					}
				}

				
			}
		}
	}
	else
	{
		//
		// For non-player characters. If the player is in a cut scene then
		// it is a bit unfair to shoot them!
		//

		if (p_person->Genus.Person->Target)
		{
			Thing *p_target;

			p_target = TO_THING(p_person->Genus.Person->Target);

			if (p_target->Class == CLASS_VEHICLE)
			{
				if (p_target->State == STATE_DEAD)
				{
					p_person->Genus.Person->Target = NULL;
				}
			}
			else
			{
				if (dont_hurt_target_during_cutscene(p_person, p_target))
				{
					return;
				}
			}
		}
	}


	ammo=shoot_get_ammo_sound_anim_time(p_person,&sound,&anim,&time);

	if (ammo == NOT_A_GUN_YOU_SHOOT)
	{
		return;
	}

	if (!ammo || ammo == HAD_TO_CHANGE_CLIP)
	{
		MFX_play_thing(THING_NUMBER(p_person),S_PISTOL_DRY,MFX_REPLACE,p_person);
		
		if (p_person->Genus.Person->PlayerID && ammo != HAD_TO_CHANGE_CLIP)
		{
			//
			// Change Darci's weapon.
			//

			SLONG special = get_persons_best_weapon_with_ammo(p_person);

			if (special)
			{
				if (special == SPECIAL_GUN)
				{
					set_person_draw_gun(p_person);
				}
				else
				{
					set_person_draw_item(p_person, special);
				}
			}
			else
			{
				if (p_person->Genus.Person->SpecialUse)
				{
					set_person_item_away(p_person);
				}
				else
				{
					set_person_gun_away(p_person);
				}
			}
		}

		return;
	}

	set_anim(p_person,anim);
	p_person->Genus.Person->Action=ACTION_SHOOT;

	set_generic_person_state_function(p_person,STATE_GUN);

	p_person->SubState=SUB_STATE_SHOOT_GUN;
//	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	p_person->Genus.Person->Flags|=(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	p_person->Draw.Tweened->Flags|=DT_FLAG_GUNFLASH;

	{
		

		// ideally the 0 in the above line would be fixed up on some kind of
		// person-by-person basis. ask me about it later. :P
		// hey look! it got fixed up on a person-by-person basis! ;)

		if (!shoot_target || p_person->Genus.Person->Target==0)
		{
			//
			// Find someone to shoot
			//

			p_person->Genus.Person->Target = find_target_new(p_person);

			//
			// check it's not a cut scene again
			//
			if (p_person->Genus.Person->Target)
			{
				Thing *p_target;

				p_target = TO_THING(p_person->Genus.Person->Target);

				
				set_face_thing(p_person, p_target);

				if (p_target->Class == CLASS_PERSON)
				{
					//
					// You get punished for killing innocents nowadays...
					//
					
					if(p_person->Genus.Person->PersonType==PERSON_DARCI)
					{
						if(p_target->Draw.Tweened->CurrentAnim==ANIM_HANDS_UP||p_target->Draw.Tweened->CurrentAnim==ANIM_HANDS_UP_LOOP)
						{
							PANEL_new_text(p_person,8000,XLAT_str(X_GET_DOWN));
	//						PANEL_new_text(p_person,8000,"Get Down.");
							set_person_dead(p_target,p_person,PERSON_DEATH_TYPE_GET_DOWN,0,0);
						//	set_person_get_down(p_target);
							return; //leave this here its good for you.
						}

						if (p_target->Draw.Tweened->CurrentAnim == ANIM_HANDS_UP_LIE)
						{
							//
							// Can't shoot civs while they are 'getting down'.
							//

							return;
						}

						if(p_target->Genus.Person->PersonType==PERSON_COP && !(p_target->Genus.Person->Flags2&FLAG2_PERSON_GUILTY))
						{
							PANEL_new_text(p_person,8000,XLAT_str(X_CANT_SHOOT_COP));
							return; //leave this here its good for you.

						}
					}
/*
					if(p_target->Draw.Tweened->CurrentAnim==ANIM_HANDS_UP)
					{
						PANEL_new_text(p_person,8000,"I can't shoot an Unarmed Person.");
						return; //leave this here its good for you.
					}
					if(p_target->Genus.Person->PersonType==PERSON_COP)
					{
						PANEL_new_text(p_person,8000,"I can't shoot a Cop.");
						return; //leave this here its good for you.

					}
*/

					if (p_target->Genus.Person->PlayerID)
					{
						if (EWAY_stop_player_moving())
						{
							//
							// Don't shoot.
							//

							return;
						}
					}
				}			
			}

		}

		MFX_play_thing(THING_NUMBER(p_person),sound,MFX_REPLACE,p_person);

		ASSERT(p_person->Genus.Person->Target != THING_NUMBER(p_person));
//		ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);

		//
		// Guards can hear gunshots.
		//


		actually_fire_gun(p_person);
	}
}

#ifndef PSX
void	set_person_grapple_windup(Thing *p_person)
{
	//
	// Make sure she has the grappling hook.
	//

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_GRAPPLING);

	//
	// Set the animation.
	//

	set_anim(p_person, ANIM_GRAPPLING_HOOK_WINDUP);

	//
	// Set the state.
	//

	set_generic_person_state_function(p_person, STATE_GRAPPLING);

	p_person->SubState = SUB_STATE_GRAPPLING_WINDUP;
}


void	set_person_grappling_hook_release(Thing *p_person)
{
	//
	// Make sure she has the grappling hook.
	//

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_GRAPPLING);

	//
	// Set the animation.
	//

	set_anim(p_person, ANIM_GRAPPLING_HOOK_RELEASE);

	//
	// Set the state.
	//

	set_generic_person_state_function(p_person, STATE_GRAPPLING);

	p_person->SubState = SUB_STATE_GRAPPLING_RELEASE;
}
#endif
//
// Returns SPECIAL_TYPE if the given person is holding a gun.
//

SLONG person_has_gun_out(Thing *p_person)
{
	if (p_person->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)
	{
		return(SPECIAL_GUN);
	}

	if (!p_person->Genus.Person->SpecialUse)
	{
		return FALSE;
	}

	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);


		if (p_special->Genus.Special->SpecialType == SPECIAL_SHOTGUN ||
			p_special->Genus.Special->SpecialType == SPECIAL_AK47)
		{
			return(p_special->Genus.Special->SpecialType);
		}
	}

	return FALSE;
}


void	drop_current_gun(Thing *p_person,SLONG change_anim)
{
	SLONG gx;
	SLONG gy;
	SLONG gz;

	Thing *p_special;

	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		Thing *p_gun;

		//
		// Create a gun on the ground.
		//

		find_nice_place_near_person(
			p_person,
		   &gx,
		   &gy,
		   &gz);

		p_gun = alloc_special(
					SPECIAL_GUN,
					SPECIAL_SUBSTATE_NONE,
					gx,
					gy,
					gz,
					NULL);

		if (p_gun)
		{
			if (p_person->Genus.Person->PlayerID)
			{
				p_gun->Genus.Special->ammo = p_person->Genus.Person->Ammo;
			}
			else
			{
				p_gun->Genus.Special->ammo = (Random() & 0x3) + 3;
			}
		}

		//
		// p_person doesn't have the gun any more.
		//
		
		p_person->Draw.Tweened->PersonID&=  ~0xe0;
		p_person->Genus.Person->Flags   &= ~FLAG_PERSON_GUN_OUT;
		p_person->Flags                 &= ~FLAGS_HAS_GUN;
	}
	else
	if (p_person->Genus.Person->SpecialUse)
	{
		//
		// Drop the special that p_person is using.
		//

		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		special_drop(p_special, p_person);

		//
		// Not using anything any more.
		//

		p_person->Genus.Person->SpecialUse = NULL;
		p_person->Draw.Tweened->PersonID&=  ~0xe0;
		//p_person->Draw.Tweened->PersonID   = 0;

		if(change_anim)
			set_person_idle(p_person);
	}
}

void drop_all_items(Thing *p_person, UBYTE is_being_searched)
{
	SLONG gx;
	SLONG gy;
	SLONG gz;
	UBYTE found_something=0;

	//
	// If he has a gun...
	//

	if (p_person->Flags & FLAGS_HAS_GUN)
	{
		find_nice_place_near_person(
			p_person,
		   &gx,
		   &gy,
		   &gz);

		{
			Thing *p_gun = alloc_special(
								SPECIAL_GUN,
								SPECIAL_SUBSTATE_NONE,
								gx,
								gy,
								gz,
								NULL);

			if (p_gun)
			{
				if (p_person->Genus.Person->PlayerID)
				{
					p_gun->Genus.Special->ammo = p_person->Genus.Person->Ammo;
				}
				else
				{
					p_gun->Genus.Special->ammo = (Random() & 0x3) + 3;
				}
				found_something=1;
			}
		}

		p_person->Flags &= ~FLAGS_HAS_GUN;
	}

	//
	// Make the person drop all the specials he is carrying.
	//

	while(p_person->Genus.Person->SpecialList)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialList);

		if (p_person->Genus.Person->drop == p_special->Genus.Special->SpecialType)
		{
			//
			// Already dropping his bounty...
			//

			p_person->Genus.Person->drop = NULL;
		}

		special_drop(p_special, p_person);
		found_something=1;
	}

	//
	// Make the person drop his bounty.
	//

	if (p_person->Genus.Person->drop)
	{
		find_nice_place_near_person(
			p_person,
		   &gx,
		   &gy,
		   &gz);

		alloc_special(
			p_person->Genus.Person->drop,
			SPECIAL_SUBSTATE_NONE,
			gx,
			gy,
			gz,
			NULL);

		p_person->Genus.Person->drop = NULL;
		found_something=1;
	}
	if (is_being_searched&&found_something) MFX_play_ambient(THING_NUMBER(p_person),S_ITEM_REVEALED,0);

}


void	set_person_idle_uncroutch(Thing *p_person)
{
	SLONG	anim;
	set_generic_person_state_function(p_person,STATE_IDLE);
	p_person->Genus.Person->Action=ACTION_IDLE;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;
	p_person->SubState = SUB_STATE_IDLE_UNCROUCH;

	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		anim=ANIM_UNCROUTCH_PISTOL;
	}
	else
	if(person_holding_2handed(p_person))
	{
		anim=ANIM_UNCROUTCH_AK;
	}
	else
	{
		anim=ANIM_CROUTCH_GETUP;
	}

	set_anim(p_person, anim);

	return;



}



void	set_person_turn_to_hug_wall(Thing *p_person)
{
	SLONG	anim;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		anim=ANIM_PRESS_WALL_TURN_PISTOL;
	}
	else
	if(person_holding_2handed(p_person))
	{
		anim=ANIM_PRESS_WALL_TURN_AK;
	}
	else
	{
		anim=ANIM_PRESS_WALL_TURN;
	}
	set_generic_person_state_function(p_person,STATE_HUG_WALL);
	p_person->SubState=SUB_STATE_HUG_WALL_TURN;
	set_anim(p_person,anim);
	p_person->Genus.Person->Action=ACTION_HUG_WALL;
}

void	set_person_hug_wall_dir(Thing *p_person,SLONG dir)
{
	SLONG	gun;
	SLONG	anim;

	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
		gun=2;
	else
		gun=person_holding_2handed(p_person);
	if(dir==1)
	{
		//
		//left
		//
		if(gun==2)
			anim=ANIM_PRESS_WALL_SIDLE_R_PISTOL;
		else
		if(gun==1)
			anim=ANIM_PRESS_WALL_SIDLE_R_AK;
		else
			anim=ANIM_PRESS_WALL_SIDLE_R;

		set_anim(p_person,anim);
		p_person->SubState=SUB_STATE_HUG_WALL_STEP_LEFT;
	}
	else
	{
		//
		//right
		//
		if(gun==2)
			anim=ANIM_PRESS_WALL_SIDLE_L_PISTOL;
		else
		if(gun==1)
			anim=ANIM_PRESS_WALL_SIDLE_L_AK;
		else
			anim=ANIM_PRESS_WALL_SIDLE_L;

		set_anim(p_person,anim);
		p_person->SubState=SUB_STATE_HUG_WALL_STEP_RIGHT;

	}

}
void	set_person_hug_wall_look(Thing *p_person,SLONG dir)
{
	SLONG	gun;
	SLONG	anim;

	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	p_person->Genus.Person->InsideRoom=0;

	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
		gun=2;
	else
		gun=person_holding_2handed(p_person);
	if(dir==1)
	{
		//
		//left
		//
		if(gun==2)
			anim=ANIM_PRESS_WALL_LOOK_L_PISTOL;
		else
		if(gun==1)
			anim=ANIM_PRESS_WALL_LOOK_L_AK;
		else
			anim=ANIM_PRESS_WALL_LOOK_L;

		set_anim(p_person,anim);
		p_person->SubState=SUB_STATE_HUG_WALL_LOOK_L;
	}
	else
	{
		//
		//right
		//
		if(gun==2)
			anim=ANIM_PRESS_WALL_LOOK_R_PISTOL;
		else
		if(gun==1)
			anim=ANIM_PRESS_WALL_LOOK_R_AK;
		else
			anim=ANIM_PRESS_WALL_LOOK_R;

		set_anim(p_person,anim);
		p_person->SubState=SUB_STATE_HUG_WALL_LOOK_R;

	}


}

void	set_person_idle(Thing *p_person)
{
	SLONG anim;
	p_person->Genus.Person->Flags&=~FLAG_PERSON_KO;

	if(check_on_slippy_slope(p_person))
		return;


	if(p_person->Genus.Person->Flags2&FLAG2_PERSON_CARRYING)
	{
		set_person_stand_carry(p_person);
		return;
	}


	if(p_person->Genus.Person->PlayerID)
	{
		if (p_person->Genus.Person->PlayerID && !person_has_gun_out(p_person))
		{
			//
			// You are a player and you're not in fight mode.
			//

			Thing *p_attacker = is_person_under_attack_low_level(p_person, FALSE, 0x200);

			if (p_attacker)
			{
				SLONG anim;

				//
				// You are under attack by someone.
				//

				ASSERT(p_attacker->Class==CLASS_PERSON);

				p_person->Genus.Person->Target = THING_NUMBER(p_attacker);
				person_enter_fight_mode(p_person);		//for fuck's sake get in fight mode
				set_person_fight_idle(p_person);
				return;
			}
			else
			{
				p_person->Genus.Person->Target = 0;
				p_person->Genus.Person->Mode = PERSON_MODE_RUN;
				p_person->Genus.Person->Agression=0;
			}
			GAME_FLAGS&=~GF_SIDE_ON_COMBAT;
		}





		if (p_person->Genus.Person->Mode != PERSON_MODE_FIGHT)

		{
			if(continue_moveing(p_person))
			{
				set_person_running(p_person);
				return;
			}
		}
	}

	p_person->SubState             =  0;
	p_person->Genus.Person->Flags &= ~FLAG_PERSON_HELPLESS;
//	p_person->Genus.Person->Target =  NULL;

#ifndef PSX
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GRAPPLING)
	{
		set_person_grapple_windup(p_person);

		return;
	}
#endif
	if (person_has_gun_out(p_person))
	{
		set_person_aim(p_person);

		return;
	}

	p_person->Genus.Person->Timer1=(Random()&0x1ff)+400;
	set_generic_person_state_function(p_person,STATE_IDLE);
	p_person->Genus.Person->Action=ACTION_IDLE;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	p_person->Velocity=0;

	//
	// What animation should we use?
	//

	set_anim_idle(p_person);
/*
	if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		anim=find_idle_fight_stance(p_person);

	}
	else
	{
		if(person_holding_2handed(p_person) && p_person->Genus.Person->PersonType!=PERSON_ROPER)
			anim=ANIM_SHOTGUN_IDLE;
		else
			anim = ANIM_STAND_READY;
	}
	set_anim(p_person,anim);
*/

/*
	if(p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special;

		//
		// This person might be using the base ball bat. This weapon has
		// its own special idle stance.
		//

		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch(p_special->Genus.Special->SpecialType)
		{
			case	SPECIAL_BASEBALLBAT:
				anim = ANIM_BAT_IDLE;
				break;
			case	SPECIAL_SHOTGUN:
			case	SPECIAL_AK47:
				anim = ANIM_SHOTGUN_STAND;
				break;
		}
	}
*/


//	queue_anim(p_person,anim);
/*
	if(p_person->Genus.Person->Flags&FLAG_PERSON_LOCK_ANIM_CHANGE)
	{
		locked_anim_change(p_person,0,ANIM_STAND_READY);
		p_person->Genus.Person->Flags&=~FLAG_PERSON_LOCK_ANIM_CHANGE;
	}
	else
	{
		p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_STAND_HIP];
		p_person->Draw.Tweened->CurrentAnim	=	ANIM_STAND_HIP;
	}
*/
	set_person_sidle(p_person);

}


void	set_person_locked_idle_ready(Thing *p_person)
{
	SLONG	anim=ANIM_STAND_READY;

	if(p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special;

		//
		// This person might be using the base ball bat. This weapon has
		// its own special idle stance.
		//

		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch(p_special->Genus.Special->SpecialType)
		{
			case	SPECIAL_GUN:
				anim = ANIM_PISTOL_AIM_AHEAD;
				break;

			case	SPECIAL_BASEBALLBAT:
				anim = ANIM_BAT_IDLE;
				break;
			case	SPECIAL_SHOTGUN:
			case	SPECIAL_AK47:
				anim = ANIM_SHOTGUN_STAND;
				break;
		}
	}


	p_person->SubState=0;
#ifndef PSX
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GRAPPLING)
	{
		set_person_grapple_windup(p_person);

		return;
	}
#endif

	p_person->Genus.Person->Timer1=(Random()&0x1ff)+400;
	if(p_person->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)
	{
		if(p_person->Genus.Person->PlayerID)
		{
			camera_shoot();
		}


		locked_anim_change(p_person,0,anim);
		set_person_aim(p_person);
/*
		p_person->Genus.Person->Action=ACTION_AIM_GUN;
		set_generic_person_state_function(p_person,STATE_GUN);
		p_person->SubState=SUB_STATE_AIM_GUN;
		p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
		p_person->Genus.Person->Flags|=(FLAG_PERSON_NON_INT_C);
*/
		return;
	}
	if(p_person->Genus.Person->PlayerID)
	{
		camera_normal();
	}




	p_person->SubState = 0;
	set_generic_person_state_function(p_person,STATE_IDLE);
	p_person->SubState=0;

	set_locked_anim(p_person,anim,0);

/*
	locked_anim_change(p_person,0,anim);
	p_person->Draw.Tweened->AnimTween=0;
	p_person->Draw.Tweened->CurrentAnim	=	ANIM_STAND_READY;
*/
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_IDLE;//fight_idle
//	p_person->WorldPos.Y = calc_height_at(p_person->WorldPos.X,p_person->WorldPos.Z);
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	if (person_has_gun_out(p_person))
	{
		set_person_aim(p_person);

		return;
	}

	set_person_sidle(p_person);

}

//
//	Check if back against a wall, for sneak along mode
//

SLONG	set_person_sidle(struct Thing *p_person)
{
	SLONG	dx,dz;
	SLONG	index,facet;
	SLONG	dist;
	SLONG	px,pz,mx,mz;
	SLONG	angle;
	SLONG	ret_x,ret_z;
	SLONG	mdx,mdz;
	return(0);
	/*

	px=p_person->WorldPos.X>>8;
	pz=p_person->WorldPos.Z>>8;

	mx=px>>PAP_SHIFT_LO;
	mz=pz>>PAP_SHIFT_LO;

	for(mdx=-1;mdx<=1;mdx++)
	for(mdz=-1;mdz<=1;mdz++)
	{
		SLONG	mpx,mpz;
		SLONG	exit=0;

		mpx=(mdx*80+px)>>PAP_SHIFT_LO;
		mpz=(mdz*80+pz)>>PAP_SHIFT_LO;

		if(mpx>=0 && mpx<PAP_SIZE_LO && mpz>=0 && mpz<PAP_SIZE_LO)
		if((mdx==0&&mdz==0)||(mpx!=mx||mpz!=mz))
		{
			index=PAP_2LO(mpx,mpz).ColVectHead;

			if(index)
			while(!exit)
			{
				struct	DFacet	*p_facet;
				facet=facet_links[index];
				if(facet<0)
				{
					facet=-facet;
					exit=1;

				}

				p_facet=&dfacets[facet];

		SLONG nearest_point_on_line_and_dist(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z);

				if(p_facet->FacetType==STOREY_TYPE_NORMAL)
				{
					dist = nearest_point_on_line_and_dist(
								p_facet->x[0] << 8, p_facet->z[0] << 8,
								p_facet->x[1] << 8, p_facet->z[1] << 8,
								px, pz,
							   &ret_x,
							   &ret_z);

					if(dist>0&&dist<40)
					{
						SLONG	dx,dz;

						dx=(px)-ret_x;
						dz=(pz)-ret_z;

						angle=calc_angle(dx,dz);
						angle-=p_person->Draw.Tweened->Angle;

						if((angle>-1024-100 && angle<-1024+100)|| (angle>1024-100 && angle<1024+100))
						{
							//
							// valid wall to sidle along
							//
	 					//	p_person->Genus.Person->Action=ACTION_SIDE_STEP;
	 						p_person->SubState=SUB_STATE_SIDLE;
							p_person->Draw.Tweened->Angle=(calc_angle(dx,dz)+1024)&2047;
							return(1);
						}
					}
				}
				index++;
			}
		}
	}
	return(0);
	*/

}

/*
void	set_person_idle_ready(Thing *p_person)
{
	p_person->SubState=0;
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GRAPPLING)
	{
		set_person_grapple_windup(p_person);

		return;
	}
	p_person->Genus.Person->Timer1=(Random()&0xff)+100;
	if(p_person->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)
	{
		set_person_aim(p_person);
		return;
	}
	set_generic_person_state_function(p_person,STATE_IDLE);
	p_person->SubState=0;

//	queue_anim(p_person,ANIM_STAND_READY);
	set_anim(p_person,ANIM_STAND_READY);
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_IDLE;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	set_person_sidle(p_person);
}
*/

//
// See if this location contains a viable platform for a fott or a hand or anything
// has an offset from the player, that is rotated by the players angle
//
#ifdef	UNUSED
SLONG	foot_hold_available(Thing *p_person,SLONG dx,SLONG dy,SLONG dz)
{
	SLONG	rdx,rdz;
	SLONG	angle;

	SLONG	px,py,pz,ret_y,face;

	angle=p_person->Draw.Tweened->Angle;

	rdx=(dx*COS(angle)-dz*SIN(angle))>>16;
	rdz=(dx*SIN(angle)+dz*COS(angle))>>16;

	px=(p_person->WorldPos.X>>8)+rdx;
	py=(p_person->WorldPos.Y>>8);
	pz=(p_person->WorldPos.Z>>8)+rdz;

	face=find_face_for_this_pos(px,py,pz,&ret_y,0,0);

	if(face)
		return(1);
	else
		return(0);
}

SLONG	foot_hold_available_obj(Thing *p_person,SLONG dx,SLONG dy,SLONG dz,SLONG obj)
{
	SLONG	rdx,rdz;
	SLONG	angle;
	SLONG	wx,wy,wz;
	SLONG	px,py,pz,ret_y,face;

	angle=p_person->Draw.Tweened->Angle;

	rdx=(dx*COS(angle)-dz*SIN(angle))>>16;
	rdz=(dx*SIN(angle)+dz*COS(angle))>>16;

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,obj,&wx,&wy,&wz);

	px=(p_person->WorldPos.X>>8)+rdx+wx;
	py=(p_person->WorldPos.Y>>8)+wy;
	pz=(p_person->WorldPos.Z>>8)+rdz+wz;

	face=find_face_for_this_pos(px,py,pz,&ret_y,0,0);

	if(face)
		return(1);
	else
		return(0);
}
#endif
void	set_person_flip(Thing *p_person,SLONG dir)
{
	MSG_add(" start flipping");
	switch(dir)
	{	
		case	0:
//			if(!foot_hold_available(p_person,100,0,0))
//				return;

			set_anim(p_person,ANIM_FLIP_LEFT);
			p_person->Genus.Person->Action = ACTION_FLIP_LEFT;
			break;

		case	1:
//			if(!foot_hold_available(p_person,-100,0,0))
//				return;

			set_anim(p_person,ANIM_FLIP_RIGHT);
			p_person->Genus.Person->Action = ACTION_FLIP_RIGHT;
			break;

	}
//	set_thing_velocity(p_person,40);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	p_person->SubState = SUB_STATE_FLIPING;
	p_person->Genus.Person->Flags|=(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
}

void	set_person_running(Thing *p_person)
{

	p_person->Draw.Tweened->Locked = 0;
	p_person->Genus.Person->Timer1 = 0;

	if(p_person->Genus.Person->PlayerID)
	{
		camera_normal();
	}

	if(p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		Thing	*p_target;
		p_target=TO_THING(p_person->Genus.Person->Target);

		set_anim(p_person,ANIM_START_WALK_CARRY);
		set_anim(p_target,ANIM_START_WALK_CARRY_V);
		p_target->SubState=SUB_STATE_CARRY_MOVE_V;

		goto	run;

	}


	switch(p_person->Genus.Person->Mode)
	{
		case PERSON_MODE_WALK:
			set_person_walking(p_person);
			break;
		case PERSON_MODE_SPRINT:
			if(p_person->Genus.Person->PersonType==PERSON_ROPER)
				goto	yomp;

			set_anim_running(p_person);
			goto	run;
		case PERSON_MODE_RUN:
		case PERSON_MODE_FIGHT:
//			if(p_person->Genus.Person->AnimType!=ANIM_TYPE_ROPER)  //I don't like yomp_start
yomp:;
//			if(person_holding_2handed(p_person))

			if(person_holding_bat(p_person))
			{
				set_anim(p_person,ANIM_YOMP_START_BAT);

			}
			else
			if(person_holding_2handed(p_person))
			{
				set_anim(p_person,ANIM_YOMP_START_AK);

				//
				// de-target
				//

			}
			else
			{
				if(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
				{
					set_anim(p_person,ANIM_YOMP_START_PISTOL);

				}
				else
					set_anim(p_person,ANIM_YOMP_START);
			}
			if (p_person->Genus.Person->PlayerID)
				p_person->Genus.Person->Target=0;
//			else
//				set_anim(p_person,ANIM_YOMP);
run:;
//			set_thing_velocity(p_person,15);
			set_generic_person_state_function(p_person,STATE_MOVEING);
			p_person->SubState = SUB_STATE_RUNNING;
			if(p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
				p_person->Genus.Person->Action = ACTION_NONE;
			else
				p_person->Genus.Person->Action = ACTION_RUN;
			p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
			p_person->Genus.Person->Mode = PERSON_MODE_RUN;
			break;
		case PERSON_MODE_SNEAK:
			set_person_sneaking(p_person);
			break;
		default:
			ASSERT(0);
	}
}

void	set_person_running_frame(Thing *p_person,SLONG frame)
{
	switch(p_person->Genus.Person->Mode)
	{
		case	0:
			MSG_add(" start running");
			queue_anim(p_person,ANIM_RUN);

//			set_thing_velocity(p_person,15);
			set_generic_person_state_function(p_person,STATE_MOVEING);
			p_person->SubState = SUB_STATE_RUNNING;
			p_person->Genus.Person->Action = ACTION_RUN;
			p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
			while(frame)
			{
				p_person->Draw.Tweened->QueuedFrame=p_person->Draw.Tweened->QueuedFrame->NextFrame;
				frame--;
			}
			break;
		case	1:
			set_person_walking(p_person);
			break;
		case	2:
			set_person_sneaking(p_person);
			break;
	}


}

void	set_person_draw_special(Thing *p_person)
{
	Thing *p_special = NULL;
/*
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		//
		// Put away your gun.
		//

		set_person_gun_away(p_person);
	}
	else
*/
	{
		//
		// Make p_person use an item.
		//
		
		if (p_person->Genus.Person->SpecialUse == NULL)
		{
			//
			// Use her first item.
			//

			if (p_person->Genus.Person->SpecialList)
			{
				p_special = TO_THING(p_person->Genus.Person->SpecialList);
			}
		}
		else
		{
			p_special = TO_THING(p_person->Genus.Person->SpecialUse);

			if (p_special->Genus.Special->NextSpecial)
			{
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			}
			else
			{
				p_special = NULL;
			}
		}

		if (p_special)
		{
			set_person_draw_item(p_person, p_special->Genus.Special->SpecialType);
		}
		else
		{
/*
			p_person->Genus.Person->SpecialUse = NULL;
			p_person->Draw.Tweened->PersonID   = 0;

			//
			// If p_person has a gun then get it out.
			//

			set_person_draw_gun(p_person);
*/
		}
	}

	//
	// Make people notice.
	//

	PCOM_oscillate_tympanum(
		PCOM_SOUND_DRAW_GUN,
		p_person,
		p_person->WorldPos.X >> 8,
		p_person->WorldPos.Y >> 8,
		p_person->WorldPos.Z >> 8);
}

void	set_person_draw_gun(Thing *p_person)
{
	if(p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		return;
	}

	if (!(p_person->Flags & FLAGS_HAS_GUN))
	{
		return;
	}

	MSG_add(" start draw gun");
	p_person->Genus.Person->Mode = PERSON_MODE_RUN;
	set_anim(p_person,ANIM_PISTOL_DRAW);
	set_thing_velocity(p_person,0);
	set_generic_person_state_function(p_person,STATE_GUN);

	p_person->SubState                  = SUB_STATE_DRAW_GUN;
	p_person->Genus.Person->Action      = ACTION_DRAW_SPECIAL;
	p_person->Genus.Person->SpecialDraw = NULL;
	p_person->Genus.Person->SpecialUse  = NULL;
	p_person->Genus.Person->Flags      |= FLAG_PERSON_NON_INT_C | FLAG_PERSON_NON_INT_M;
	p_person->Genus.Person->Target      = NULL;

	if(p_person->Genus.Person->PlayerID)
	{
		camera_shoot();
	}

	//
	// You can't be in fight mode with a gun out!
	//

	if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		p_person->Genus.Person->Mode =  PERSON_MODE_RUN;
	}
	
	//
	// Make people notice.
	//

	PCOM_oscillate_tympanum(
		PCOM_SOUND_DRAW_GUN,
		p_person,
		p_person->WorldPos.X >> 8,
		p_person->WorldPos.Y >> 8,
		p_person->WorldPos.Z >> 8);
}

void	set_person_gun_away(Thing *p_person)
{
/*
	if (!(p_person->Flags & FLAGS_HAS_GUN))
	{
		return;
	}
*/

	p_person->Genus.Person->SpecialUse = NULL;
	p_person->Draw.Tweened->PersonID&=  ~0xe0;
//	p_person->Draw.Tweened->PersonID=0;
	p_person->Genus.Person->Flags&=~FLAG_PERSON_GUN_OUT;
	set_person_idle(p_person);

	if(p_person->Genus.Person->PlayerID)
	{
		camera_normal();
	}
}


void	set_person_step_left(Thing *p_person)
{
	MSG_add(" start step_left");
	if(p_person->SubState == SUB_STATE_STEP_LEFT)
		return;
	set_anim(p_person,ANIM_STEP_LEFT);
	set_thing_velocity(p_person,0);
	set_generic_person_state_function(p_person,STATE_MOVEING);
//			p_person->State=STATE_MOVEING;
	p_person->SubState = SUB_STATE_STEP_LEFT;
	p_person->Genus.Person->Action = ACTION_SIDE_STEP;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

//	p_person->Genus.Person->Flags|=FLAG_PERSON_LOCK_ANIM_CHANGE;
}

void	set_person_step_right(Thing *p_person)
{
	MSG_add(" start step_left");
	if(p_person->SubState == SUB_STATE_STEP_RIGHT)
		return;
	set_anim(p_person,ANIM_STEP_RIGHT);
	set_thing_velocity(p_person,0);
	set_generic_person_state_function(p_person,STATE_MOVEING);
//			p_person->State=STATE_MOVEING;
	p_person->SubState = SUB_STATE_STEP_RIGHT;
	p_person->Genus.Person->Action = ACTION_SIDE_STEP;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

//	p_person->Genus.Person->Flags|=FLAG_PERSON_LOCK_ANIM_CHANGE;
}

void	set_vehicle_anim(Thing *p_vehicle,SLONG anim)
{
	// 1 is still
	// 2 is open/close
	ASSERT(0);
	return;
	p_vehicle->Genus.Vehicle->Draw.CurrentFrame=game_chunk[5].AnimList[anim];
	p_vehicle->Genus.Vehicle->Draw.AnimTween=0;

	if(anim==2)
	{
		p_vehicle->Genus.Vehicle->Draw.NextFrame=p_vehicle->Genus.Vehicle->Draw.CurrentFrame->NextFrame;
	}
	else
	{
		p_vehicle->Genus.Vehicle->Draw.NextFrame=p_vehicle->Genus.Vehicle->Draw.CurrentFrame;
	}
}

void	get_car_enter_xz(Thing *p_vehicle,SLONG door,SLONG *cx,SLONG *cz);

void	position_person_for_vehicle(Thing *p_person,Thing *p_vehicle,SLONG door)
{
	SLONG	ix,iz;
	GameCoord	new_position;

	ASSERT(door == 0 || door == 1);

	get_car_enter_xz(p_vehicle,door,&ix,&iz);

	new_position.X=ix<<8;
	new_position.Z=iz<<8;

	new_position.Y=p_person->WorldPos.Y;//PAP_calc_map_height_at(ix,iz)<<8;
	move_thing_on_map(p_person,&new_position);

	p_person->Draw.Tweened->Angle = p_vehicle->Genus.Vehicle->Angle;

	if (door)
	{
		p_person->Draw.Tweened->Angle += 512;
	}
	else
	{
		p_person->Draw.Tweened->Angle -= 512;
	}

	p_person->Draw.Tweened->Angle &= 2047;

}

void	set_person_enter_vehicle(Thing *p_person,Thing *p_vehicle, SLONG door)
{
	ASSERT(door == 0 || door == 1);

	if (p_vehicle->Genus.Vehicle->Driver)
	{
		//
		// This vehicle is already occupied.
		//

		p_person->Genus.Person->InCar = 0;
		return;
	}

	if (p_vehicle->State == STATE_DEAD)
	{
		//
		// Broken car!
		// 

		return;
	}

	if (p_vehicle->Genus.Vehicle->key != SPECIAL_NONE)
	{
		//
		// We can only get into this car if we have the right key.
		// 

		if (!person_has_special(p_person, p_vehicle->Genus.Vehicle->key))
		{

			if (p_person->Genus.Person->PlayerID)
			{
				//CONSOLE_text("The car is locked");
/*
				extern void add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text);

				add_damage_text(
					p_person->WorldPos.X          >> 8,
					p_person->WorldPos.Y + 0x6000 >> 8,
					p_person->WorldPos.Z          >> 8,
					XLAT_str(X_CAR_LOCKED));
*/
				PANEL_new_info_message(XLAT_str(X_CAR_LOCKED));
			}

			p_person->Genus.Person->InCar = 0;
			return;
		}
	}

	set_thing_velocity(p_person,0);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	p_person->SubState = SUB_STATE_ENTERING_VEHICLE;
	p_person->Genus.Person->Action = ACTION_ENTER_VEHICLE;
	p_person->Genus.Person->Flags|=(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	{
		// NASTY HACK! This extern changes the behaviour of get_car_door_offsets()
		// in vehicle.cpp!

		extern UBYTE sneaky_do_it_for_positioning_a_person_to_do_the_enter_anim;

		sneaky_do_it_for_positioning_a_person_to_do_the_enter_anim = TRUE;

		position_person_for_vehicle(p_person,p_vehicle,door);

		sneaky_do_it_for_positioning_a_person_to_do_the_enter_anim = FALSE;
	}

//	set_locked_anim(p_person,ANIM_DANCE_BOOGIE,SUB_OBJECT_LEFT_FOOT);
	set_locked_anim(p_person,(door) ? ANIM_ENTER_TAXI : ANIM_ENTER_CAR,SUB_OBJECT_LEFT_FOOT);
	p_person->Genus.Person->InCar = THING_NUMBER(p_vehicle);
//	set_vehicle_anim(p_vehicle,2);
	p_vehicle->Genus.Vehicle->Flags|=FLAG_VEH_ANIMATING;


	MFX_play_thing(THING_NUMBER(p_vehicle),S_CAR_DOOR,0,p_vehicle);

}


//
// Adds a person to the linked list of people who are passengers in
// the given vehicle.
// 

void add_person_to_passenger_list(Thing *p_person, Thing *p_vehicle)
{
	ASSERT(p_person->Class  == CLASS_PERSON);
	ASSERT(p_vehicle->Class == CLASS_VEHICLE);

	p_person ->Genus.Person ->Passenger = p_vehicle->Genus.Vehicle->Passenger;
	p_vehicle->Genus.Vehicle->Passenger	= THING_NUMBER(p_person);
}

//
// Removes a person from the passenger list of the given vehicle.
//

void remove_person_from_passenger_list(Thing *p_person, Thing *p_vehicle)
{
	Thing *p_passenger;

	ASSERT(p_person->Class  == CLASS_PERSON);
	ASSERT(p_vehicle->Class == CLASS_VEHICLE);

	ASSERT(p_person->Genus.Person->Flags & FLAG_PERSON_PASSENGER);

	UWORD  next;
	UWORD *prev;

	prev = &p_vehicle->Genus.Vehicle->Passenger;
	next =  p_vehicle->Genus.Vehicle->Passenger;

	while(1)
	{
		if (next == NULL)
		{
			ASSERT(0);

			//
			// The person wasn't a passenger in this vehicle!
			//

			return;
		}

		p_passenger = TO_THING(next);

		ASSERT(p_passenger->Class == CLASS_PERSON);

		if (p_passenger == p_person)
		{
			//
			// We have found who to delete.
			//

		   *prev = p_person->Genus.Person->Passenger;

			return;
		}

		prev = &p_passenger->Genus.Person->Passenger;
		next =  p_passenger->Genus.Person->Passenger;
	}
}




void set_person_passenger_in_vehicle(Thing *p_person, Thing *p_vehicle, SLONG door)
{
	//
	// No animation. Just pop in!
	//

	set_thing_velocity(p_person,0);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	remove_thing_from_map(p_person);
	add_person_to_passenger_list(p_person, p_vehicle);

	p_person->SubState             = SUB_STATE_INSIDE_VEHICLE;
	p_person->Genus.Person->Action = ACTION_INSIDE_VEHICLE;
	p_person->Genus.Person->Flags |= (FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C|FLAG_PERSON_PASSENGER);
	p_person->Genus.Person->InCar  = THING_NUMBER(p_vehicle);
}


void	set_person_exit_vehicle(Thing *p_person)
{
	Thing *p_vehicle;

	p_vehicle = TO_THING(p_person->Genus.Person->InCar);

	//
	// Where do we want this person to appear?
	//

	SLONG door_x;
	SLONG door_y;
	SLONG door_z;
	SLONG side;
	SLONG otherside = FALSE;
	SLONG	dx,dz;

	GameCoord newpos;

	side = (p_person->Genus.Person->Flags & FLAG_PERSON_PASSENGER);

  try_again:;


	VEH_find_door(
		p_vehicle,
		side,
	   &door_x,
	   &door_z);

	//
	// Is there a fence or a wall in the way?
	//

	dx = 0;//(door_x - (p_vehicle->WorldPos.X >> 8)) >> 2;
	dz = 0;//(door_z - (p_vehicle->WorldPos.Z >> 8)) >> 2;

	SLONG mx = door_x + dx >> 8;
	SLONG mz = door_z + dz >> 8;

	door_y = PAP_calc_map_height_at(door_x + dx,door_z + dz);

#ifndef TARGET_DC
	#ifndef NDEBUG

	AENG_world_line(
		p_vehicle->WorldPos.X >> 8,
		p_vehicle->WorldPos.Y >> 8,
		p_vehicle->WorldPos.Z >> 8,
		32,
		0xffffff,
		door_x+dx,
		door_y,
		door_z+dz,
		0,
		0x03f008,
		TRUE);

	#endif
#endif

	if (abs(door_y-(p_person->WorldPos.Y>>8))>150 || (PAP_2HI(mx,mz).Flags & PAP_FLAG_NOGO) || !there_is_a_los(
														p_vehicle->WorldPos.X          >> 8,
														p_vehicle->WorldPos.Y + 0x6000 >> 8,
														p_vehicle->WorldPos.Z          >> 8,
														door_x + dx,
														door_y + 0x60,
														door_z + dz,
														LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG |
														LOS_FLAG_IGNORE_PRIMS                 |
														LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
	{
		//
		// There is something in the way- bin this message because sometimes
		// you simply can't move the car.
		//

		/*

		if (p_person->Genus.Person->PlayerID)
		{
			//
			// Tell the player that he can't get out of the car there.
			//

			PANEL_new_help_message("You can't open the door to get out of the vehicle!");

			return;
		}
	
		*/

		if (!otherside)
		{
			//
			// Try getting out the passenger side.
			//

			side      = !side;
			otherside =  TRUE;

			goto try_again;
		}
		else
		{
			//
			// This is unfortunate! There is a person who wants to get out of the vehicle
			// but he can't open any of the doors... Just make him exit _inside_ the
			// vehicle!
			// 

			door_x = p_vehicle->WorldPos.X >> 8;
			door_z = p_vehicle->WorldPos.Z >> 8;
		}
	}


	newpos.X = door_x << 8;
	newpos.Y = PAP_calc_map_height_at(door_x,door_z);
	newpos.Z = door_z << 8;

	move_thing_on_map(p_person, &newpos);

	if (p_person->Genus.Person->Flags & FLAG_PERSON_PASSENGER)
	{
		//
		// Get out of the car.
		//

		remove_person_from_passenger_list(p_person, p_vehicle);

		p_person->Genus.Person->Flags    &= ~(FLAG_PERSON_PASSENGER|FLAG_PERSON_DRIVING);
		p_person->Genus.Person->InCar     =  NULL;
		p_person->Genus.Person->Passenger =  NULL;
	}
	else
	{
		p_person->Genus.Person->Flags   &= ~(FLAG_PERSON_PASSENGER|FLAG_PERSON_DRIVING);
		p_person->Genus.Person->InCar    =  NULL;
		p_vehicle->Genus.Vehicle->Flags &= ~FLAG_FURN_DRIVING;
		p_vehicle->Genus.Vehicle->Driver =  NULL;

#ifndef PSX
		MFX_stop(THING_NUMBER(p_vehicle),S_CARX_START);
		MFX_stop(THING_NUMBER(p_vehicle),S_CARX_CRUISE);
		MFX_stop(THING_NUMBER(p_vehicle),S_CARX_IDLE);
#endif
		if (p_vehicle->Genus.Vehicle->Flags&FLAG_VEH_FX_STATE) {
//			MFX_play_thing(THING_NUMBER(p_vehicle),S_CARX_DECEL,MFX_MOVING|MFX_EARLY_OUT,p_vehicle);
			p_vehicle->Genus.Vehicle->Flags&=~FLAG_VEH_FX_STATE;
		}
		MFX_stop(THING_NUMBER(p_vehicle),MFX_WAVE_ALL);
#ifndef PSX
		MFX_play_thing(THING_NUMBER(p_vehicle),S_CARX_END,0,p_vehicle);
#endif
	}

	add_thing_to_map(p_person);
	set_person_idle(p_person);
	plant_feet(p_person);
}

#ifdef	BIKE
void position_person_for_mounting_bike(Thing *p_person, Thing *p_bike)
{
	GameCoord newpos;
	SLONG     vector[3];

	FMATRIX_vector(
		vector,
	   (p_bike->Draw.Tweened->Angle + 1024) & 2047,
		0);

	newpos    = p_bike->WorldPos;

	switch(p_person->Genus.Person->AnimType) {
/*
	case ANIM_TYPE_ROPER:
		newpos.X += vector[2] * 30 >> 8;
		newpos.Y -= 0x1800;
		newpos.Z -= vector[0] * 30 >> 8;

		newpos.X += vector[0] * 50 >> 8;
		newpos.Z += vector[2] * 50 >> 8;

		break;
*/
	default:

		newpos.X += vector[2] * 60 >> 8;
		newpos.Y -= 0x2800;
		newpos.Z -= vector[0] * 60 >> 8;

		newpos.X += vector[0] * 30 >> 8;
		newpos.Z += vector[2] * 30 >> 8;

		break;
	}

	move_thing_on_map(p_person, &newpos);

	p_person->Draw.Tweened->Angle = (p_bike->Draw.Tweened->Angle - 512) & 2047;
}


void set_person_mount_bike(Thing *p_person, Thing *p_bike)
{
	set_generic_person_state_function(p_person,STATE_MOVEING);

	set_anim(p_person, ANIM_BIKE_MOUNT);

	//
	// Position the person for getting on the bike.
	//

	position_person_for_mounting_bike(p_person, p_bike);

	p_person->SubState             = SUB_STATE_MOUNTING_BIKE;
	p_person->Genus.Person->Flags |= FLAG_PERSON_BIKING | FLAG_PERSON_NON_INT_M | FLAG_PERSON_NON_INT_C;
	p_person->Genus.Person->InCar  = THING_NUMBER(p_bike);
	p_person->Genus.Person->Action = ACTION_ENTER_VEHICLE;

	//
	// Tell the bike that somebody is getting on it.
	//

	BIKE_set_mounting(p_bike, p_person);
}

void set_person_dismount_bike(Thing *p_person)
{
	//
	// Tell the bike to play its dismount anim.
	//

	void BIKE_set_dismounting(Thing *p_bike);

	Thing *p_bike = TO_THING(p_person->Genus.Person->InCar);

	BIKE_set_dismounting(p_bike);

	set_generic_person_state_function(p_person,STATE_MOVEING);

	set_anim(p_person, ANIM_BIKE_DISMOUNT);

	// position_person_for_mounting_bike(p_person, p_bike);

	p_person->SubState             = SUB_STATE_DISMOUNTING_BIKE;
	p_person->Genus.Person->Flags |= FLAG_PERSON_BIKING | FLAG_PERSON_NON_INT_M | FLAG_PERSON_NON_INT_C;
	p_person->Genus.Person->Action = ACTION_ENTER_VEHICLE;	// Enter.. exit.. whats the difference!

	/*

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_BIKING;
	p_person->Genus.Person->InCar  =  0;

	p_person->Draw.Tweened->Tilt=0;
	set_person_idle(p_person);

	*/

	//
	// Position the person for getting on the bike.
	//

	position_person_for_mounting_bike(p_person, p_bike);
}

#endif


void	set_anim_walking(Thing *p_person)
{
	SLONG	anim;

	if(person_holding_2handed(p_person))
	{
		if(p_person->Genus.Person->PersonType!=PERSON_DARCI && p_person->Genus.Person->PersonType!=PERSON_ROPER)
		{
			set_anim(p_person,ANIM_THUG_WALK_SHOTGUN);
			return;
		}

	}
	if(p_person->Genus.Person->PersonType == PERSON_THUG_RASTA ||
		p_person->Genus.Person->PersonType == PERSON_THUG_RED   ||
		p_person->Genus.Person->PersonType == PERSON_THUG_GREY)
	{
			set_anim(p_person,ANIM_THUG_WALK);
	}
	else

	if(p_person->Genus.Person->PersonType==PERSON_COP)
	{
		SLONG	old;
		old=p_person->Genus.Person->AnimType;
		set_anim_of_type(p_person,COP_ROPER_ANIM_WALK,ANIM_TYPE_ROPER);
		p_person->Genus.Person->AnimType=old;
	}
	else
	if(p_person->Genus.Person->PersonType==PERSON_CIV)
	{
		switch(p_person->Draw.Tweened->MeshID)
		{
			case	7:
			case	8:
				set_anim_of_type(p_person,CIV_M_ANIM_WALK,ANIM_TYPE_CIV);
				break;
			case	9:
				set_anim_of_type(p_person,CIV_F_ANIM_WALK,ANIM_TYPE_CIV);
				break;
			default:
				ASSERT(0);
				break;

		}
	}
	else
	{
		
		set_anim(p_person,ANIM_WALK);
	}

}

void	set_anim_running(Thing *p_person)
{
	SLONG	anim;

	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
	{
		set_anim(p_person,ANIM_YOMP); //roper dont have a sprint
	}
	else
	if(p_person->Genus.Person->PersonType==PERSON_COP)
	{
		SLONG	old;
		old=p_person->Genus.Person->AnimType;
		set_anim_of_type(p_person,COP_ROPER_ANIM_RUN,ANIM_TYPE_ROPER);
		p_person->Genus.Person->AnimType=old;
	}
	else
	if(p_person->Genus.Person->PersonType==PERSON_CIV)
	{
		SLONG	old;
		switch(p_person->Draw.Tweened->MeshID)
		{
			case	7:
			case	8:
				set_anim_of_type(p_person,CIV_M_ANIM_RUN,ANIM_TYPE_CIV);
				break;
			case	9:
				set_anim_of_type(p_person,CIV_F_ANIM_RUN,ANIM_TYPE_CIV);
				break;
			default:
				ASSERT(0);
				break;

		}
	}
	else
	{

		if (person_has_gun_out(p_person))
		{
			set_anim(p_person, ANIM_AK_JOG);
		}
		else
		{
			set_anim(p_person,ANIM_RUN);
		}
	}

}
void	set_anim_idle(Thing *p_person)
{
	SLONG	anim;

	if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		anim=find_idle_fight_stance(p_person);

	}
	else
	{
		if(person_holding_2handed(p_person) && p_person->Genus.Person->PersonType!=PERSON_ROPER)
		{
			anim=ANIM_SHOTGUN_IDLE;
			set_anim(p_person,anim);
		}
		else
		{

			switch(p_person->Genus.Person->PersonType)
			{
				case	PERSON_SLAG_TART:
				case	PERSON_SLAG_FATUGLY:
					set_anim(p_person,ANIM_STAND_HIP);
					break;
				case	PERSON_CIV:
					switch(p_person->Draw.Tweened->MeshID)
					{
						case	7:
						case	8:
							set_anim_of_type(p_person,CIV_M_ANIM_STAND,ANIM_TYPE_CIV);
							break;
						case	9:
							set_anim(p_person,ANIM_STAND_HIP);

							break;
						default:
							ASSERT(0);
							break;

					}
					break;
				case	PERSON_COP:
					set_anim_of_type(p_person,COP_ROPER_ANIM_READY,ANIM_TYPE_ROPER);
					break;
				default:
					anim = ANIM_STAND_READY;
					set_anim(p_person,anim);
					break;
			}
		}
	}
}

void	set_person_walking(Thing *p_person)
{
	set_anim_walking(p_person);

	set_thing_velocity(p_person,5);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	p_person->SubState = SUB_STATE_WALKING;
	p_person->Genus.Person->Action = ACTION_WALK;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
}
void	set_person_walk_backwards(Thing *p_person)
{
	if(person_holding_2handed(p_person))
	{
		set_anim(p_person,ANIM_BACK_WALK_AK);
	}
	else
	{
		set_anim(p_person,ANIM_BACK_WALK);
	}


	set_thing_velocity(p_person,-5);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	p_person->SubState = SUB_STATE_WALKING_BACKWARDS;
	p_person->Genus.Person->Action = ACTION_WALK;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
}

void	set_person_sneaking(Thing *p_person)
{
	set_anim(p_person,ANIM_SNEAK);

	set_thing_velocity(p_person,5);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	p_person->SubState = SUB_STATE_WALKING;
	p_person->Genus.Person->Action = ACTION_WALK;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
}

void	set_person_hop_back(Thing *p_person)
{
//	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_WALK];
//	p_person->Draw.Tweened->CurrentAnim	=	ANIM_WALK;
	set_thing_velocity(p_person,0);
	set_generic_person_state_function(p_person,STATE_MOVEING);
	p_person->SubState = SUB_STATE_HOP_BACK;
	p_person->Genus.Person->Action = ACTION_HOP_BACK;
	p_person->Genus.Person->Timer1=3;
	p_person->Genus.Person->Flags|=(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
//	queue_anim(p_person,ANIM_BSTEP);
//	set_locked_anim(p_person,ANIM_WALK,0);
	set_anim(p_person,ANIM_BACK_HOP);
}

SLONG	find_idle_fight_stance(Thing *p_person)
{
	SLONG	anim=ANIM_FIGHT;

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special;

		//
		// This person is using an item.
		//

		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch(p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_KNIFE:
				anim=ANIM_KNIFE_FIGHT_READY;
				break;

			case SPECIAL_BASEBALLBAT:
				anim=ANIM_BAT_IDLE;

				break;
			case SPECIAL_SHOTGUN:
			case SPECIAL_AK47:
				break;
		}
	}
	return(anim);
}

void	set_person_fight_idle(Thing *p_person)
{
	SLONG	anim; //=ANIM_FIGHT;

	p_person->Genus.Person->Flags&=~FLAG_PERSON_KO; //bodge

	if(p_person->Genus.Person->PlayerID)
	{
		Thing *p_attacker = is_person_under_attack_low_level(p_person, FALSE, 0x200);

		if (p_attacker == NULL)
		{
			//
			// nobody to fight, then exit fight mode
			//
			p_person->Genus.Person->Agression=0;
			p_person->Genus.Person->Mode=PERSON_MODE_RUN;
			p_person->Genus.Person->Target=0;
			set_person_idle(p_person);
			if(p_person->State==STATE_IDLE)
				set_anim(p_person,ANIM_UNFIGHT);
			GAME_FLAGS&=~GF_SIDE_ON_COMBAT;
			return;
		}
	}


	person_enter_fight_mode(p_person);  //for fuck's sake get in fight mode

	set_generic_person_state_function(p_person,STATE_IDLE);

	anim=find_idle_fight_stance(p_person);
	if(person_on_floor(p_person))
	{
		set_anim(p_person,anim);
	}
	else
	{
		locked_anim_change(p_person,SUB_OBJECT_RIGHT_FOOT,anim); //LEFT or right?
	}

	p_person->Draw.Tweened->AnimTween=0;
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_IDLE;//fight_idle
	p_person->SubState=0;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	plant_feet(p_person);

	//set_person_sidle(p_person);
}

void	set_person_fight_step(Thing *p_person,SLONG dir)
{
	SLONG	anim;
	p_person->Genus.Person->Timer1=0;
	if(person_holding_bat(p_person))
	{
		switch(dir)
		{
			case	0:
				anim=ANIM_FIGHT_STEP_N_BAT;
				break;
			case	1:
				anim=ANIM_FIGHT_STEP_E_BAT;
				break;
			case	2:
				anim=ANIM_FIGHT_STEP_S_BAT;
				break;
			case	3:
				anim=ANIM_FIGHT_STEP_W_BAT;
				break;

		}
	}
	else
	{
		switch(dir)
		{
			case	0:
				anim=ANIM_FIGHT_STEP_N;
				break;
			case	1:
				anim=ANIM_FIGHT_STEP_E;
				break;
			case	2:
				anim=ANIM_FIGHT_STEP_S;
				break;
			case	3:
				anim=ANIM_FIGHT_STEP_W;
				break;

		}
	}
	if(anim==p_person->Draw.Tweened->CurrentAnim)
		return;
	set_locked_anim(p_person, anim,SUB_OBJECT_LEFT_FOOT);
	set_generic_person_state_function(p_person,STATE_FIGHTING);
	p_person->SubState=SUB_STATE_STEP_FORWARD;
	p_person->Velocity=(p_person->Genus.Person->AnimType==ANIM_TYPE_ROPER) ? 10 : 20;
	p_person->Genus.Person->Timer1=0;

}

void	set_person_fight_step_forward(Thing *p_person)
{
	if(p_person->Draw.Tweened->CurrentAnim	!=ANIM_FIGHT_STEP_N)
	{
		set_anim(p_person,ANIM_FIGHT_STEP_N);
		set_generic_person_state_function(p_person,STATE_FIGHTING);
		p_person->SubState=SUB_STATE_STEP_FORWARD;
		p_person->Velocity=10;
	}
}

void	set_person_block(Thing *p_person)
{
	SLONG anim = ANIM_BLOCK_HIGH;

	set_generic_person_state_function(p_person,STATE_FIGHTING);

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special;

		//
		// This person might be using a weapon with its own special block anim.
		//

		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch (p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_BASEBALLBAT: anim = ANIM_BAT_BLOCK;     break;
			case SPECIAL_SHOTGUN:     anim = ANIM_SHOTGUN_BLOCK; break;
		}
	}

	if (anim)
	{
		p_person->Draw.Tweened->Locked=0;
		locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); //was pelvis, but pelvis is bobbing in fight idle
		p_person->Genus.Person->CombatNode=0;
		p_person->Velocity=0;
		p_person->Genus.Person->Action=ACTION_FIGHT_PUNCH;
		p_person->SubState=SUB_STATE_BLOCK;
		p_person->Draw.Tweened->Locked=0; //-SUB_OBJECT_LEFT_FOOT;
		p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	}
}

void	set_person_idle_croutch(Thing *p_person)
{
	SLONG	anim;
	set_generic_person_state_function(p_person,STATE_IDLE);
	anim=ANIM_IDLE_CROUTCH;

	if(anim)
	{
		p_person->Draw.Tweened->Locked=0;
		set_anim(p_person,anim); 

		p_person->Genus.Person->CombatNode=0;
		p_person->Velocity=0;
		p_person->Genus.Person->Action=ACTION_CROUTCH;
		p_person->SubState=SUB_STATE_IDLE_CROUTCHING;

		p_person->Draw.Tweened->Locked=0; //-SUB_OBJECT_LEFT_FOOT;
	}
}

//
// Drops the person you are carrying in an emergency. e.g. if you just
// been hit or shot.
//

void emergency_uncarry(Thing *p_person)
{
	ASSERT(p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING);

	Thing *p_target = TO_THING(p_person->Genus.Person->Target);

	//
	// Make the target plunge down.
	//

	p_target->DY       = -(4 << 8);
	p_target->OnFace   = 0;
	p_target->SubState = SUB_STATE_DYING_PRONE;

	set_generic_person_state_function(p_target, STATE_DYING);

	locked_anim_change(p_target,0,ANIM_PLUNGE_FORWARD);

	//
	// Stop carrying.
	//

	p_person->Genus.Person->Flags2 &= ~FLAG2_PERSON_CARRYING;
}


void	carry_running(Thing *p_person)
{
	Thing	*p_target;

	SLONG dx;
	SLONG dz;
	GameCoord new_position = p_person->WorldPos;
	p_target=TO_THING(p_person->Genus.Person->Target);


	dx = -(SIN(p_person->Draw.Tweened->Angle) * 90) >> 8;
	dz = -(COS(p_person->Draw.Tweened->Angle) * 90) >> 8;
	new_position.X+=dx;
	new_position.Z+=dz;

	move_thing_on_map(p_target, &new_position);
	p_target->Draw.Tweened->Angle=p_person->Draw.Tweened->Angle+1024;
	p_target->Draw.Tweened->Angle&=2047;
}
void	set_person_carry(Thing *p_person,SLONG s_index)
{
	Thing	*p_target;
	SLONG dx;
	SLONG dz;
	GameCoord new_position = p_person->WorldPos;

	p_person->Genus.Person->Mode = PERSON_MODE_RUN;
	p_person->Genus.Person->Flags2 |= FLAG2_PERSON_CARRYING;

	dx = -(SIN(p_person->Draw.Tweened->Angle) * 90) >> 8;
	dz = -(COS(p_person->Draw.Tweened->Angle) * 90) >> 8;
	new_position.X+=dx;
	new_position.Z+=dz;

	p_person->Genus.Person->Target=s_index;
	p_target=TO_THING(s_index);
	set_generic_person_state_function(p_person,STATE_CARRY);
	set_anim(p_person,ANIM_PICKUP_CARRY);
	p_person->SubState=SUB_STATE_PICKUP_CARRY;

//	set_generic_person_state_function(p_target,STATE_CARRY);
	set_anim(p_target,ANIM_PICKUP_CARRY_V);
	p_target->SubState=SUB_STATE_PICKUP_CARRY_V;

	move_thing_on_map(p_target, &new_position);
	p_target->Draw.Tweened->Angle=p_person->Draw.Tweened->Angle+1024;
	p_target->Draw.Tweened->Angle&=2047;
	if(p_target->Genus.Person->Target)
	{
		remove_from_gang_attack(p_target,TO_THING(p_target->Genus.Person->Target));
	}
}

void	set_person_uncarry(Thing *p_person)
{
	Thing	*p_target;
	GameCoord new_position = p_person->WorldPos;

	p_target=TO_THING(p_person->Genus.Person->Target);
	set_generic_person_state_function(p_person,STATE_CARRY);
	set_anim(p_person,ANIM_PUTDOWN_CARRY);
	p_person->SubState=SUB_STATE_DROP_CARRY;

//	set_generic_person_state_function(p_target,STATE_CARRY);
	set_anim(p_target,ANIM_PUTDOWN_CARRY_V);
	p_target->SubState=SUB_STATE_DROP_CARRY_V;

//	move_thing_on_map(p_target, &new_position);
//	p_target->Draw.Tweened->Angle=p_person->Draw.Tweened->Angle;

}

void	set_person_stand_carry(Thing *p_person)
{
	Thing	*p_target;
	p_target=TO_THING(p_person->Genus.Person->Target);

	set_generic_person_state_function(p_person,STATE_CARRY);

	set_anim(p_person,ANIM_STAND_CARRY);
	p_person->SubState=SUB_STATE_STAND_CARRY;

	set_anim(p_target,ANIM_STAND_CARRY_V);
	p_target->SubState=SUB_STATE_STAND_CARRY_V;
}

void	fn_person_carry(Thing *p_person)
{
	SLONG	end;
	Thing	*p_target;

	p_target=TO_THING(p_person->Genus.Person->Target);
	switch(p_person->SubState)
	{
		case	SUB_STATE_PICKUP_CARRY:
				end=person_normal_animate(p_person);
				if(end)
				{
					set_person_stand_carry(p_person);
				}
				break;
		case	SUB_STATE_DROP_CARRY:
				end=person_normal_animate(p_person);
				if(end)
				{
					p_person->Genus.Person->Flags2&=~FLAG2_PERSON_CARRYING;
					set_person_idle(p_person);
				}
				break;
		case	SUB_STATE_STAND_CARRY:
			{
				SLONG dx;
				SLONG dz;
				GameCoord new_position = p_person->WorldPos;


				dx = -(SIN(p_person->Draw.Tweened->Angle) * 90) >> 8;
				dz = -(COS(p_person->Draw.Tweened->Angle) * 90) >> 8;
				new_position.X+=dx;
				new_position.Z+=dz;
				move_thing_on_map(p_target, &new_position);
				p_target->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle+1024)&2047;
				p_target->Draw.Tweened->Roll =(2048-p_person->Draw.Tweened->Roll)&2047;
			}
			break;
	}
}

void	set_person_arrest(Thing *p_person,SLONG s_index)
{
	SLONG	anim;
	ASSERT(s_index);

	set_generic_person_state_function(p_person,STATE_IDLE);
	anim=ANIM_ARREST_CROUTCH;//_DOWN;

	p_person->Draw.Tweened->Locked=0;
	locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); 

	p_person->Genus.Person->CombatNode=0;
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_CROUTCH;
	p_person->SubState=SUB_STATE_IDLE_CROUTCH_ARREST;

	p_person->Draw.Tweened->Locked=0; //-SUB_OBJECT_LEFT_FOOT;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	p_person->Genus.Person->Target=s_index;

//	p_person->Genus.Person->Mode = PERSON_MODE_RUN;

	if(s_index)
	{
		SLONG	ax,ay,az;
		Thing	*p_target;
		SLONG	dx,dz;
		SLONG	on_what_side;
		GameCoord new_position;

		p_target=TO_THING(s_index);
		p_target->Genus.Person->Flags|=FLAG_PERSON_ARRESTED;

		switch(p_target->Genus.Person->PersonType)
		{
			case PERSON_THUG_RASTA:
			case PERSON_THUG_GREY:
			case PERSON_THUG_RED:
			case PERSON_MIB1:
			case PERSON_MIB2:
			case PERSON_MIB3:
				stat_arrested_thug++;
				break;
			default:
				stat_arrested_innocent++;
				break;

		}


		on_what_side=person_is_lying_on_what(p_target);


		if(on_what_side==PERSON_ON_HIS_BACK)
		{
			set_locked_anim(p_target,ANIM_ARREST_ROLL,0);
			p_target->SubState=SUB_STATE_DEAD_ARREST_TURN_OVER;
		}
		else
		{
			SLONG	angle;

//			angle=lie_down_angle(p_target->Draw.Tweened->CurrentAnim);
			set_locked_anim(p_target,ANIM_ARREST_BE_CUFFED,0);
			p_target->SubState=SUB_STATE_DEAD_CUFFED;
//			p_target->Draw.Tweened->Angle+=1024;
			p_target->Draw.Tweened->Angle&=2047;

		}

		calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&ax,&ay,&az);

		ax+=p_target->WorldPos.X>>8;
		ay+=p_target->WorldPos.Y>>8;
		az+=p_target->WorldPos.Z>>8;

		dx	=	(SIN(p_target->Draw.Tweened->Angle)*60)>>16;
		dz	=	(COS(p_target->Draw.Tweened->Angle)*60)>>16;

		ax-=dx;
		az-=dz;

		dx=((p_person->WorldPos.X>>8)-ax)<<8;
		dz=((p_person->WorldPos.Z>>8)-az)<<8;

		new_position.X = p_target->WorldPos.X+dx;
		new_position.Y = p_target->WorldPos.Y;
		new_position.Z = p_target->WorldPos.Z+dz;

		move_thing_on_map(p_target, &new_position);



/*
		new_position.X = ax<<8;
		new_position.Y = p_person->WorldPos.Y;
		new_position.Z = az<<8;

		move_thing_on_map(p_person, &new_position);
*/


		p_person->Draw.Tweened->Angle=(p_target->Draw.Tweened->Angle+1024)&2047;
		


		if(!remove_from_gang_attack(p_target,p_person))
		{
			Thing	*p_victim;
			if(p_target->Genus.Person->Target)
			{
				
				p_victim=TO_THING(p_target->Genus.Person->Target);
				if(p_victim->Class==CLASS_PERSON)
					remove_from_gang_attack(p_target,p_victim);
			}
		}

	//
	// set them to dead so they can't get upto any funny business
	//
		set_generic_person_state_function(p_target,STATE_DEAD);
		p_target->Genus.Person->Action=ACTION_DEAD;
		p_target->Genus.Person->Timer1 = 0;

		switch(p_person->Genus.Person->PersonType) 
		{
			case PERSON_DARCI:
#ifndef PSX
				MFX_play_thing(THING_NUMBER(p_person),SOUND_Range(S_DARCI_ARREST_START,S_DARCI_ARREST_END),MFX_MOVING|MFX_OVERLAP,p_person);
#else
				MFX_play_thing(THING_NUMBER(p_person),S_DARCI_ARREST,MFX_MOVING|MFX_OVERLAP,p_person);
#endif
				break;
#ifdef ROPER_EVER_ARRESTS_AGAIN
			case PERSON_ROPER:
				MFX_play_thing(THING_NUMBER(p_person),S_ROPER_ARREST,MFX_MOVING|MFX_OVERLAP,p_person);
				break;
#endif
			case PERSON_COP:
#ifndef PSX
				MFX_play_thing(THING_NUMBER(p_person),SOUND_Range(S_COP_ARREST_START,S_COP_ARREST_END),MFX_MOVING|MFX_OVERLAP,p_person);
#else
//				MFX_play_thing(THING_NUMBER(p_person),S_COP_ARREST_START,MFX_MOVING|MFX_OVERLAP,p_person);
#endif
				break;
		}

		//
		// Alert the world.
		//

		PCOM_oscillate_tympanum(
			PCOM_SOUND_HEY,
			p_person,
			p_person->WorldPos.X >> 8,
			p_person->WorldPos.Y >> 8,
			p_person->WorldPos.Z >> 8);
	}

}

void	set_person_croutch(Thing *p_person)
{
	SLONG	anim;
	SLONG	index;

	if(p_person->Genus.Person->PersonType==PERSON_DARCI && (index=find_arrestee(p_person)))
	{
//		ASSERT(0);
		set_person_arrest(p_person,index);
		return;
	}

	set_generic_person_state_function(p_person,STATE_IDLE);
	if(person_has_gun_out(p_person))
	{
		if(person_holding_2handed(p_person))
		{
			anim = ANIM_SHOTGUN_DUCK;
		}
		else
		{
			anim = ANIM_PISTOL_DUCK;
		}
	}
	else
	{
		anim = ANIM_CROUTCH_DOWN;
	}


	if(anim)
	{
		//
		// Doing a locked anim change here makes Roper move backwards.
		//

		p_person->Draw.Tweened->Locked=0;

		//locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); 
	
		set_anim(p_person, anim);

		p_person->Genus.Person->CombatNode=0;
		p_person->Velocity=0;
		p_person->Genus.Person->Action=ACTION_CROUTCH;
		p_person->SubState=SUB_STATE_IDLE_CROUTCH;

		p_person->Draw.Tweened->Locked=0; //-SUB_OBJECT_LEFT_FOOT;
//		p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	}
}


void	set_person_crawling(Thing *p_person)
{
	set_generic_person_state_function(p_person,STATE_MOVEING);

	if(person_has_gun_out(p_person))
	{
		//
		// Make sure this person isn't too near a fence.
		//


		SLONG x1 = p_person->WorldPos.X;
		SLONG y1 = p_person->WorldPos.Y;
		SLONG z1 = p_person->WorldPos.Z;

		SLONG x2 = p_person->WorldPos.X;
		SLONG y2 = p_person->WorldPos.Y;
		SLONG z2 = p_person->WorldPos.Z;

		slide_along(
			x1, y1, z2,
			&x2, &y2, &z2,
			0,
			50,
			0);

		GameCoord newpos;

		newpos.X = x2;
		newpos.Y = y2;
		newpos.Z = z2;

		move_thing_on_map(p_person, &newpos);

		//
		// Sniper crawl anim.
		//

		p_person->Draw.Tweened->Locked=0;

		locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_SNIPER_CRAWL);
	}
	else
	{
		//
		// Normal crawl anim.
		//

		p_person->Draw.Tweened->Locked=0;

		locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_CRAWL);
	}

	p_person->Genus.Person->CombatNode=0;
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_CRAWLING;
	p_person->SubState=SUB_STATE_CRAWLING; //_CROUTCH;

	p_person->Draw.Tweened->Locked=0; //-SUB_OBJECT_LEFT_FOOT;
}

SLONG	set_person_leg_sweep(Thing *p_person)
{
	set_generic_person_state_function(p_person,STATE_FIGHTING);
	set_anim(p_person,ANIM_LEG_SWEEP);

	p_person->Genus.Person->CombatNode = 0;
	p_person->SubState                 = SUB_STATE_KICK;
	return(0);
}

//
// this should only happen from fight idle or stand idle?
//
SLONG set_person_punch(Thing *p_person)
{
	SLONG anim;
	SLONG node = 1;
//	ASSERT(p_person->Genus.Person->PersonType!=PERSON_DARCI);

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_PUNCH;

	if (!p_person->Genus.Person->PlayerID)
	{
		//
		// For non-player characters. If the player is in a cut scene then
		// it is a bit unfair to punch them!
		//

		UWORD i_target = PCOM_person_wants_to_kill(p_person);

		if (i_target)
		{
			Thing *p_target = TO_THING(i_target);

			if (dont_hurt_target_during_cutscene(p_person, p_target))
			{
				//
				// Don't punch.
				//

				return 0;
			}
		}
	}

/*
	if(p_person->Genus.Person->PlayerID)
	{
		camera_fight();
	}
*/
	//
	// if players punching while stepping forward or AI 1 in 4
	//

//	if((p_person->Genus.Person->PlayerID && p_person->SubState==SUB_STATE_STEP_FORWARD && p_person->Draw.Tweened->FrameIndex<2) || ((!p_person->Genus.Person->PlayerID) && ((GAME_TURN+THING_NUMBER(p_person))&0x3)==0))
	if(find_best_grapple(p_person))
	{
		return(0);
	}


	set_generic_person_state_function(p_person,STATE_FIGHTING);

//	anim=find_best_punch(p_person,FIND_BEST_USE_DEFAULT);
//	anim=ANIM_PUNCH_COMBO1;
//	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][anim];
//	p_person->Draw.Tweened->CurrentAnim	=	anim;

	//
	// What anim shall we use?
	//

	anim = ANIM_PUNCH_COMBO1;

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special;

		//
		// This person is using an item.
		//

		p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		switch(p_special->Genus.Special->SpecialType)
		{
			case SPECIAL_KNIFE:
				anim=ANIM_KNIFE_ATTACK1;
				node=14;

				break;

			case SPECIAL_BASEBALLBAT:

				anim = ANIM_BAT_HIT1; break;
				node=19;
/*
				switch((Random() >> 4) & 1)
				{
					case 0: anim = ANIM_BAT_HIT1; break;
					case 1: anim = ANIM_BAT_HIT2; break;
				}
*/

				break;
			case SPECIAL_SHOTGUN:
			case SPECIAL_AK47:
				switch((Random() >> 4) & 1)
				{
					case 0: anim = ANIM_SHOTGUN_WHIP1; break;
					case 1: anim = ANIM_SHOTGUN_WHIP2; break;
				}
				break;
		}
	}

	if (anim)
	{
		if (anim == ANIM_PUNCH_COMBO1 && p_person == NET_PERSON(0))
		{
			//
			// Darci has done a punch
			//

			EWAY_darci_move |= EWAY_DARCI_MOVE_PUNCH;
		}

		p_person->Draw.Tweened->Locked = 0;

		//locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); //was pelvis, but pelvis is bobbing in fight idle

		set_anim(p_person, anim);

		p_person->Genus.Person->CombatNode = node;
		p_person->Velocity                 = 0;
		p_person->Genus.Person->Action     = ACTION_FIGHT_PUNCH;
		p_person->SubState                 = SUB_STATE_PUNCH;

		p_person->Draw.Tweened->Locked = 0; //-SUB_OBJECT_LEFT_FOOT;
		p_person->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	}

//	PlaySample(6);
	return(anim);
}

#ifdef	UNUSED
SLONG	set_person_punch_if_can(Thing *p_person)
{
	SLONG	anim;
	/*
	if(find_best_grapple(p_person))
	{
		return(0);
	}
	*/
	anim=find_best_punch(p_person,0);
	if(anim)
	{
		set_generic_person_state_function(p_person,STATE_FIGHTING);
	//	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][anim];
	//	p_person->Draw.Tweened->CurrentAnim	=	anim;
		locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim);
		
		p_person->Velocity=0;
		p_person->Genus.Person->Action=ACTION_FIGHT_PUNCH;
		p_person->SubState=SUB_STATE_PUNCH;

		p_person->Draw.Tweened->Locked=0; //-SUB_OBJECT_LEFT_FOOT;
		p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
		return(1);
	}
	return(0);
//	PlaySample(6);
}
#endif
SLONG set_person_kick_dir(Thing *p_person,SLONG dir)
{
	SLONG	anim;
	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_KICK;

	if(p_person->Genus.Person->PlayerID)
	{
		camera_fight();
	}

	set_generic_person_state_function(p_person,STATE_FIGHTING);


	//
	// What anim shall we use?
	//
	anim=ANIM_KICK_COMBO1;

	switch(dir)
	{
		case	0: //n
			break;
		case	1: //e
			anim=ANIM_KICK_RIGHT;
			break;
		case	2: //s
			anim=ANIM_KICK_BEHIND;
			break;

		case	3: //w
			anim=ANIM_KICK_LEFT;
			break;

	}

	if (p_person == NET_PERSON(0))
	{
		EWAY_darci_move |= EWAY_DARCI_MOVE_KICK;
	}

	if (anim)
	{
		p_person->Draw.Tweened->Locked = 0;

		locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); //was pelvis, but pelvis is bobbing in fight idle

		p_person->Genus.Person->CombatNode = 1;
		p_person->Velocity                 = 0;
		p_person->Genus.Person->Action     = ACTION_FIGHT_PUNCH;
		p_person->SubState                 = SUB_STATE_KICK;

		p_person->Draw.Tweened->Locked = 0; //-SUB_OBJECT_LEFT_FOOT;
		p_person->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	}

	return(anim);
}
void	set_person_fight_anim(Thing *p_person,SLONG anim)
{
	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_KICK;
	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_PUNCH;


	set_generic_person_state_function(p_person,STATE_FIGHTING);

	if (anim)
	{
		p_person->Draw.Tweened->Locked = 0;

		locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); //was pelvis, but pelvis is bobbing in fight idle

		p_person->Genus.Person->CombatNode = 1;
		p_person->Velocity                 = 0;
		p_person->Genus.Person->Action     = ACTION_FIGHT_PUNCH;
		p_person->SubState                 = SUB_STATE_KICK;

		p_person->Draw.Tweened->Locked = 0; //-SUB_OBJECT_LEFT_FOOT;
		p_person->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	}

}
/*
void	set_person_alive_alive_o(Thing *p_person)
{
	{
		SLONG ndx=p_person->Genus.Person->BurnIndex;
		Pyro  *pyro;

		set_person_idle(p_person);
		plant_feet(p_person);

		p_person->Genus.Person->Health=health[p_person->Genus.Person->PersonType];
		if (ndx&&((pyro=TO_PYRO(ndx-1))->PyroType==PYRO_IMMOLATE)) {
			pyro->Dummy=2;
			pyro->radius=290;
		}

		p_person->Genus.Person->Flags&=~(FLAG_PERSON_KO | FLAG_PERSON_HELPLESS);
		if(!CNET_network_game)
		{
		extern	void	FC_unkill_player_cam(Thing *p_thing);
			FC_unkill_player_cam(p_person);
		}
	//	set_anim(p_person,ANIM_STAND_HIP);
	}
}
*/
SLONG	set_person_kick(Thing *p_person)
{
	SLONG	anim;

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_KICK;

	if (!p_person->Genus.Person->PlayerID)
	{
		//
		// For non-player characters. If the player is in a cut scene then
		// it is a bit unfair to punch them!
		//

		UWORD i_target = PCOM_person_wants_to_kill(p_person);

		if (i_target)
		{
			Thing *p_target = TO_THING(i_target);

			if (dont_hurt_target_during_cutscene(p_person, p_target))
			{
				//
				// Don't kick.
				//

				return 0;
			}
		}
	}

	if (p_person == NET_PERSON(0))
	{
		EWAY_darci_move |= EWAY_DARCI_MOVE_KICK;
	}

	set_generic_person_state_function(p_person,STATE_FIGHTING);
//	anim=find_best_kick(p_person, FIND_BEST_USE_DEFAULT);

	anim=ANIM_KICK_COMBO1;
	p_person->Draw.Tweened->Locked=0;
	p_person->Genus.Person->CombatNode=6;

	//locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim);
	set_anim(p_person,anim);

	/*
	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_KICK_ROUND1];
	p_person->Draw.Tweened->CurrentAnim	=	ANIM_KICK_ROUND1;
	*/
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_FIGHT_PUNCH;
	p_person->SubState=SUB_STATE_KICK;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	return(anim);

}

SLONG	set_person_kick_near(Thing *p_person,SLONG dist)
{
	SLONG	anim;
	SLONG	not_nad=0;

	if (p_person == NET_PERSON(0))
	{
		EWAY_darci_move |= EWAY_DARCI_MOVE_KICK;
	}

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_KICK;

	set_generic_person_state_function(p_person,STATE_FIGHTING);

	if(p_person->Genus.Person->Target)
	{
		Thing	*p_victim;
		p_victim=TO_THING(p_person->Genus.Person->Target);

		if(p_victim->Draw.Tweened->CurrentAnim==ANIM_KICK_NAD_TAKE || p_victim->Draw.Tweened->CurrentAnim==ANIM_KICK_NAD_STUNNED || p_victim->Draw.Tweened->CurrentAnim==ANIM_KICK_NAD_RECOVER)
			not_nad=1;

	}
	if(p_person->Genus.Person->PersonType==PERSON_DARCI && dist<100 && p_person->SubState==SUB_STATE_STEP_FORWARD && !not_nad)
		anim=ANIM_KICK_NAD;
	else
		anim=ANIM_KICK_NEAR;
	p_person->Draw.Tweened->Locked=0;
	p_person->Genus.Person->CombatNode=0;

	if(p_person->SubState==SUB_STATE_STEP_FORWARD)
		set_anim(p_person,anim);
	else
		locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim);
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_FIGHT_PUNCH;
	p_person->SubState=SUB_STATE_KICK;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	return(anim);

}

SLONG	set_person_stomp(Thing *p_person)
{
	SLONG	anim;

	p_person->Genus.Person->Flags &= ~FLAG_PERSON_REQUEST_KICK;
	if(p_person->Genus.Person->PlayerID)
	{
		camera_fight();
	}

	set_generic_person_state_function(p_person,STATE_FIGHTING);

	anim=ANIM_FIGHT_STOMP;
	p_person->Draw.Tweened->Locked=0;
	p_person->Genus.Person->CombatNode=0;


	locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim);
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_FIGHT_PUNCH;
	p_person->SubState=SUB_STATE_KICK;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
	return(anim);

}

void	set_person_position_for_ladder(Thing *p_person,UWORD facet)
{
	SLONG	angle,px,pz;
	GameCoord	new_position;
	SLONG	scale=256;

	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
		scale=320;


	correct_pos_for_ladder(&dfacets[facet],&px,&pz,&angle,scale);

	new_position.X  =px<<8;
	new_position.Y  =p_person->WorldPos.Y;
	new_position.Z  =pz<<8;

	p_person->Draw.Tweened->Angle=angle;

	move_thing_on_map(p_person,&new_position);

	/*

	if(p_person->Genus.Person->PlayerID==PLAYER_ID+1)
	{
		correct_pos_for_ladder(storey,&px,&pz,&angle,15);

		new_position.X  =px<<8;
		new_position.Y  =(p_person->WorldPos.Y+(200<<8));
		new_position.Z  =pz<<8;
	}
	*/
}

inline void	play_jump_sound(Thing *p_person) {
	static SLONG jump_chan=0;
	SLONG  jump_snd=0;
#if !defined(PSX) && !defined(TARGET_DC)
	if (p_person->Flags & FLAGS_IN_SEWERS) {
		switch (person_is_on_sewer(p_person)) {
		case PERSON_ON_SEWATER:
			jump_snd=S_CLIMB_SEWER; break;
		case PERSON_ON_WATER:
			jump_snd=S_FOOTS_PUDDLE_START; break;
		}
	}
#endif
//	if (jump_snd) jump_chan=play_quick_wave_xyz(p_person->WorldPos.X,p_person->WorldPos.Y,p_person->WorldPos.Z,jump_snd,jump_chan,WAVE_PLAY_NO_INTERRUPT);
//	if (jump_snd) play_quick_wave_xyz(p_person->WorldPos.X,p_person->WorldPos.Y,p_person->WorldPos.Z,jump_snd,jump_chan,WAVE_PLAY_NO_INTERRUPT);
	if (jump_snd) MFX_play_thing(THING_NUMBER(p_person),jump_snd,0,p_person);
}


void	set_person_climb_ladder(Thing *p_person,UWORD storey)
{
//	MSG_add(" set person climb ladder \n");
	set_generic_person_state_function(p_person,STATE_CLIMB_LADDER);
	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
	{
		set_anim_of_type(p_person,COP_ROPER_ANIM_LADDER_START,ANIM_TYPE_ROPER);
	}
	else
//	if(p_person->Genus.Person->PersonType==PERSON_COP)
	if(p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY||p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||p_person->Genus.Person->PersonType==PERSON_THUG_RED)
	{
		set_anim_of_type(p_person,COP_ROPER_ANIM_LADDER_START,ANIM_TYPE_ROPER);
	}
	else
	{
		set_anim(p_person,ANIM_MOUNT_LADDER);
	}

/*
	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_MOUNT_LADDER];
	p_person->Draw.Tweened->CurrentAnim	=	ANIM_MOUNT_LADDER;
*/
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_CLIMBING;
	p_person->SubState=SUB_STATE_MOUNT_LADDER;
	p_person->Genus.Person->OnFacet=storey;
	set_person_position_for_ladder(p_person,storey);
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;

	play_jump_sound(p_person);

}
void	set_person_on_ladder(Thing *p_person)
{
//	set_generic_person_state_function(p_person,STATE_CLIMB_LADDER);
//	MSG_add(" set person ON ladder \n");
//	set_anim(p_person,ANIM_ON_LADDER);
	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
	{
		locked_anim_change_of_type(p_person,0,COP_ROPER_ANIM_LADDER_LOOP,ANIM_TYPE_ROPER);
	}
	else
//	if(p_person->Genus.Person->PersonType==PERSON_COP)
	if(p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY||p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||p_person->Genus.Person->PersonType==PERSON_THUG_RED)
	{
		locked_anim_change_of_type(p_person,0,COP_ROPER_ANIM_LADDER_LOOP,ANIM_TYPE_ROPER);
	}
	else
	{
		locked_anim_change(p_person,0,ANIM_ON_LADDER);
	}

	//
	// Make sure they aren't doing fighting controls.
	//

	p_person->Genus.Person->Mode = PERSON_MODE_RUN;

	/*
	p_person->Draw.Tweened->CurrentFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_ON_LADDER];
	p_person->Draw.Tweened->NextFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_ON_LADDER]->NextFrame;
	*/
	p_person->Draw.Tweened->AnimTween	=	0; //global_anim_array[p_person->Genus.Person->AnimType][ANIM_ON_LADDER]->NextFrame;
	p_person->Velocity=0;
	p_person->SubState=SUB_STATE_ON_LADDER;
	p_person->Genus.Person->Action=ACTION_CLIMBING;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M);
	p_person->OnFace = NULL;

}

void	set_person_on_fence(Thing *p_person)
{

//	set_generic_person_state_function(p_person,STATE_CLIMB_LADDER);
//	MSG_add(" set person ON ladder \n");
	set_anim(p_person,ANIM_CLIMB_UP_FENCE);
/*	p_person->Draw.Tweened->CurrentFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_CLIMB_UP_FENCE];
	p_person->Draw.Tweened->NextFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_CLIMB_UP_FENCE]->NextFrame;
	*/
	p_person->Draw.Tweened->AnimTween	=	0; //global_anim_array[p_person->Genus.Person->AnimType][ANIM_ON_LADDER]->NextFrame;
	p_person->Velocity=0;
	p_person->SubState=SUB_STATE_CLIMB_AROUND_WALL;
	p_person->Genus.Person->Action=ACTION_CLIMBING;
	p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	p_person->OnFace = NULL;
}


void	set_person_standing_jump(Thing *p_person)
{
	SLONG	anim;
 	if(p_person->Genus.Person->InsideIndex)
		return;
	if(p_person->SubState==SUB_STATE_SLIPPING)
		return;

	//
	// Is this person standing in front of a climbable fence? If so
	// don't do a standing jump. Climb onto the fence.
	//

	{
		SLONG x1;
		SLONG y1;
		SLONG z1;

		SLONG x2;
		SLONG y2;
		SLONG z2;

		SLONG dx = -SIN(p_person->Draw.Tweened->Angle) >> 2;
		SLONG dz = -COS(p_person->Draw.Tweened->Angle) >> 2;

		x1 = p_person->WorldPos.X          >> 8;
		y1 = p_person->WorldPos.Y + 0xa000 >> 8;
		z1 = p_person->WorldPos.Z          >> 8;

		x2 = p_person->WorldPos.X +     dx >> 8;
		y2 = p_person->WorldPos.Y + 0xa000 >> 8;
		z2 = p_person->WorldPos.Z +     dz >> 8;

		if (!there_is_a_los(
				x1, y1, z1,
				x2, y2, z2,
				LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG |
				LOS_FLAG_IGNORE_PRIMS				  |
				LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
		{
			if (los_failure_dfacet)
			{
				DFacet *df;

				df = &dfacets[los_failure_dfacet];

				if (df->FacetType == STOREY_TYPE_FENCE      ||
					df->FacetType == STOREY_TYPE_FENCE_FLAT ||
					df->FacetType == STOREY_TYPE_FENCE_BRICK)
				{
					if (!(df->FacetFlags & FACET_FLAG_UNCLIMBABLE))
					{
						//
						// The person is standing in front of a climbable fence.
						//

						set_person_land_on_fence(p_person, los_failure_dfacet, 1);

						return;
					}
				}
			}
		}
	}

	if(person_holding_2handed(p_person))
		anim=ANIM_STANDING_JUMP_AK;
	else
		anim=ANIM_STANDING_JUMP;

	set_generic_person_state_function(p_person,STATE_JUMPING);
	set_anim(p_person,anim);
	/*
	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_STANDING_JUMP];
	p_person->Draw.Tweened->CurrentAnim	=	ANIM_STANDING_JUMP;
	*/
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_STANDING_JUMP;
	p_person->SubState=SUB_STATE_STANDING_JUMP;
//	PlaySample(THING_NUMBER(p_person),SAMPLE_WOMAN_JUMP1,SAMPLE_VOL_MAX,SAMPLE_PAN_CENTER,SAMPLE_FREQ_ORIG+(GAME_TURN&0xfff),0);
	play_jump_sound(p_person);
	p_person->OnFace = 0;
}

void	set_person_standing_jump_forwards(Thing *p_person)
{
	SLONG	slope;
	SLONG	angle;
	if(p_person->OnFace<0)
	{
		slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
	}
	else
	{

		slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle)>>1;
	}
	if(slope>50)
		return;
		

	if(p_person->Genus.Person->InsideIndex)
		return;
	if(p_person->SubState==SUB_STATE_SLIPPING)
		return;
	set_person_running_jump(p_person);
/*
	set_generic_person_state_function(p_person,STATE_JUMPING);
	set_anim(p_person,ANIM_FORWARD_JUMP);
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_STANDING_JUMP;
	p_person->SubState=SUB_STATE_STANDING_JUMP_FORWARDS;
//	PlaySample(THING_NUMBER(p_person),SAMPLE_WOMAN_JUMP1,SAMPLE_VOL_MAX,SAMPLE_PAN_CENTER,SAMPLE_FREQ_ORIG+(GAME_TURN&0xfff),0);
	play_jump_sound(p_person);
	p_person->OnFace = 0;
*/
}

void	set_person_standing_jump_backwards(Thing *p_person)
{
	SLONG	slope;
	SLONG	angle;
	if(p_person->OnFace<0)
	{
		slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
	}
	else
	{

		slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle)>>1;
	}
	if(slope>50)
		return;

	if(p_person->Genus.Person->InsideIndex)
		return;
	if(p_person->SubState==SUB_STATE_SLIPPING)
		return;
	set_person_running_jump(p_person);

	set_generic_person_state_function(p_person,STATE_JUMPING);
	set_anim(p_person,ANIM_BACK_FLIP);
	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_STANDING_JUMP;
	p_person->SubState=SUB_STATE_STANDING_JUMP_BACKWARDS;
//	PlaySample(THING_NUMBER(p_person),SAMPLE_WOMAN_JUMP1,SAMPLE_VOL_MAX,SAMPLE_PAN_CENTER,SAMPLE_FREQ_ORIG+(GAME_TURN&0xfff),0);
	play_jump_sound(p_person);
	p_person->OnFace = 0;

}

void	set_person_running_jump_lr(Thing *p_person,SLONG dir);

void	set_person_running_jump(Thing *p_person)
{
	SLONG	slope;
	SLONG	angle;
	if(p_person->OnFace<0)
	{
		slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
	}
	else
	{

		slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle)>>1;
	}
	if(slope>50)
		return;

	if(p_person->SubState==SUB_STATE_SLIPPING || p_person->Genus.Person->InsideIndex)
		return;

/*
	p_person->SubState=SUB_STATE_RUNNING_THEN_JUMP;

	set_person_running_jump_lr(p_person,1);
	switch(p_person->Draw.Tweened->FrameIndex)
	{
		case	0:
		case	1:
			set_person_running_jump_lr(p_person,1);
			break;
		case	3:
		case	4:
			set_person_running_jump_lr(p_person,0);
			break;
	}
*/

	set_generic_person_state_function(p_person,STATE_JUMPING);
	if(person_has_gun_out(p_person))
	{
		set_anim(p_person,ANIM_RUN_JUMP_LEFT_AK);
	}
	else
	{
		set_anim(p_person,ANIM_RUN_JUMP_LEFT);
	}

	if (p_person->Genus.Person->PlayerID)
	{
		if (p_person->Velocity < 16)
		{
			p_person->Velocity = 16;
		}
	}
	else
	{
		p_person->Velocity = PCOM_if_i_wanted_to_jump_how_fast_should_i_do_it(p_person);
	}

	p_person->DY=0;
	p_person->DY=10<<8;
	p_person->Genus.Person->Action=ACTION_RUNNING_JUMP;
	p_person->SubState=SUB_STATE_RUNNING_JUMP;
	p_person->OnFace = 0;

	play_jump_sound(p_person);

}

void	set_person_running_jump_lr(Thing *p_person,SLONG dir)
{
}

SLONG	traverse_pos(Thing	*p_person,SLONG right)
{
	SLONG	lhx,lhy,lhz;
//	SLONG	rhx,rhy,rhz;
	SLONG	angle;
	SLONG	grab_x,grab_y,grab_z,type;
	SLONG	x,y,z;
	SLONG	face,grab_angle;

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_HAND, &lhx,&lhy,&lhz);
//	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_RIGHT_HAND,&rhx,&rhy,&rhz);

	x = lhx + (p_person->WorldPos.X >> 8);
	y = lhy + (p_person->WorldPos.Y >> 8);
	z = lhz + (p_person->WorldPos.Z >> 8);

	if(right)
	{
		angle=(p_person->Draw.Tweened->Angle-512)&2047;

	}
	else
	{
		angle=(p_person->Draw.Tweened->Angle+512)&2047;

	}

	x+=-(SIN(angle)*74)>>16; //precit hand pos at end of sideways movement
	z+=-(COS(angle)*74)>>16;

	//
	// If this square is too near a nogo zone or underground...
	//

	{
		SLONG dx;
		SLONG dz;
		SLONG cx;
		SLONG cz;

		for (dx = -32; dx <= 32; dx += 32)
		for (dz = -32; dz <= 32; dz += 32)
		{
			cx = x + dx;
			cz = z + dz;

			if (!WITHIN(cx >> 8, 0, PAP_SIZE_HI - 1) ||
				!WITHIN(cz >> 8, 0, PAP_SIZE_HI - 1))
			{
				return FALSE;
			}

			//
			// if this place is underground...
			//

			if (PAP_calc_map_height_at(cx,cz) > y + 0x30)
			{
				return FALSE;
			}

			cx >>= 8;
			cz >>= 8;

			if (PAP_2HI(cx,cz).Flags & PAP_FLAG_NOGO)
			{
				return FALSE;
			}
		}
	}

	//
	// check to see if face available for hands to grab at end of sideways movement
	//
	face = find_grab_face(
				x,y,z,
				20,40, //strict on radius
				p_person->Draw.Tweened->Angle,
			   &grab_x,
			   &grab_y,
			   &grab_z,
			   &grab_angle,
				0,
				0,&type,
				p_person);

	if(face==0)
		return(0);

	if(type!=0)
		return(0); //ladder or cable, or other odd thing found

	{
		SLONG	new_x,new_y,new_z;
		GameCoord	temp_pos;

		x-=-(SIN(angle)*74)>>16; //get hand pos back to start of traverse
		z-=-(COS(angle)*74)>>16;

		//
		// reposition self as repeating traverse anim is not good enough to make you move along straight line
		//
		face = find_grab_face(
					x,y,z,
					20,40,
					p_person->Draw.Tweened->Angle,
				   &grab_x,
				   &grab_y,
				   &grab_z,
				   &grab_angle,
					0,
					0,&type,
					p_person);

		if(face==0)
		{
			return(1); //dont repos
			ASSERT(0);
		}



		calc_sub_objects_position(p_person,0,SUB_OBJECT_LEFT_HAND,&new_x,&new_y,&new_z);

		new_x+=p_person->WorldPos.X>>8;
		new_y+=p_person->WorldPos.Y>>8;
		new_z+=p_person->WorldPos.Z>>8;

//		MSG_add(" left hand y %d grab_y %d tween %d\n",new_y,grab_y,draw_info->AnimTween);

		//LogText(" left hand pos for grab face after anim change %d %d %d \n",new_x,new_y,new_z);

		//LogText(" found a ledge offset %d %d %d to it \n",grab_x-new_x,grab_y-new_y,grab_z-new_z);

		temp_pos.X=((grab_x-new_x)<<8)+p_person->WorldPos.X;
		temp_pos.Y=((grab_y-new_y)<<8)+p_person->WorldPos.Y;
		temp_pos.Z=((grab_z-new_z)<<8)+p_person->WorldPos.Z;
		
		move_thing_on_map(p_person,&temp_pos);
	}
	return(1);
}


void	set_person_traverse(Thing	*p_person,SLONG right)
{
	if(traverse_pos(p_person,right)==0)
		return;

	set_generic_person_state_function(p_person,STATE_DANGLING); //should allready be in this state
	if(right)
	{

		p_person->SubState=SUB_STATE_TRAVERSE_RIGHT;
		p_person->Genus.Person->Action=ACTION_TRAVERSE_RIGHT;
		set_locked_anim(p_person,ANIM_TRAVERSE_RIGHT,0);
	}
	else
	{
		p_person->SubState=SUB_STATE_TRAVERSE_LEFT;
		p_person->Genus.Person->Action=ACTION_TRAVERSE_LEFT;
		set_locked_anim(p_person,ANIM_TRAVERSE_LEFT,0);
	}
	p_person->Draw.Tweened->Locked=0;

}
void	set_person_pulling_up(Thing	*p_person)
{
	//
	// Make sure the person won't pull-up into a nogo zone.
	//

	{
		SLONG dx = -SIN(p_person->Draw.Tweened->Angle) >> 2;
		SLONG dz = -COS(p_person->Draw.Tweened->Angle) >> 2;

		SLONG mx = p_person->WorldPos.X + dx >> 16;
		SLONG mz = p_person->WorldPos.Z + dz >> 16;

		if (!WITHIN(mx, 0, PAP_SIZE_HI - 1) ||
			!WITHIN(mz, 0, PAP_SIZE_HI - 1))
		{
			//
			// Pull up off the map!
			//

			return;
		}

		if (PAP_2HI(mx,mz).Flags & PAP_FLAG_NOGO)
		{
			//
			// Pull up into a nogo area!
			//

			return;
		}
	}


	set_generic_person_state_function(p_person,STATE_DANGLING); //should allready be in this state
	p_person->SubState=SUB_STATE_PULL_UP;
//	queue_anim(p_person,ANIM_PULL_UP_NEW);
	set_locked_anim(p_person,ANIM_PULL_UP_NEW,0);
	/*
	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_PULL_UP];
	p_person->Draw.Tweened->CurrentAnim	=	ANIM_PULL_UP;*/
	p_person->Genus.Person->Action=ACTION_PULL_UP;
	p_person->Draw.Tweened->Locked=0;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;

}

extern	void make_cable_flabby(SLONG building);

//
// Make a person start falling as a projectile...
//

void	set_person_drop_down(Thing	*p_person,SLONG flag)
{
	SLONG	dv=-2;
	SLONG	shotgun=0;

	SlideSoundCheck(p_person,1);

	if(person_holding_2handed(p_person))
		shotgun=1;
	MSG_add(" set person drop");
	p_person->Genus.Person->Flags&=~FLAG_PERSON_ON_CABLE;
	MFX_stop(THING_NUMBER(p_person), S_ZIPWIRE);
	if( p_person->State==STATE_DANGLING && (p_person->SubState==SUB_STATE_DANGLING_CABLE||p_person->SubState==SUB_STATE_DANGLING_CABLE_FORWARD||p_person->SubState==SUB_STATE_DANGLING_CABLE_BACKWARD) )
	{
		if(p_person->OnFace)
		{
/*
			if(prim_faces4[p_person->OnFace].Type==FACE_TYPE_CABLE)
			{
				SLONG	wall,storey,building;
				wall=-prim_faces4[p_person->OnFace].ThingIndex;
				storey=wall_list[wall].StoreyHead;
				
				building=storey_list[storey].BuildingHead;
				 make_cable_flabby(building);
				 dv=0;

			}
*/
			if(p_person->Genus.Person->Flags&FLAG_PERSON_ON_CABLE)
			{
//				 make_cable_flabby(building);
				 dv=0;

			}

		}
	}
	if( p_person->SubState==SUB_STATE_SLIPPING || p_person->SubState==SUB_STATE_SLIPPING_END)
	{
		p_person->Genus.Person->Flags|=FLAG_PERSON_MOVE_ANGLETO;

	}
	else
	{
		p_person->Genus.Person->Flags&=~FLAG_PERSON_MOVE_ANGLETO;
	}

	set_generic_person_state_function(p_person,STATE_DANGLING);

	if (flag & PERSON_DROP_DOWN_OFF_FACE)
	{
		p_person->SubState = SUB_STATE_DROP_DOWN_OFF_FACE;
	}
	else
	{
		p_person->SubState = SUB_STATE_DROP_DOWN;
	}

//	p_person->Draw.Tweened->CurrentFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_FALLING];
//	p_person->Draw.Tweened->NextFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_FALLING]->NextFrame;

	if (!(flag & PERSON_DROP_DOWN_QUEUED))
	{
		if(shotgun)
			locked_anim_change(p_person,0,ANIM_FALLING_AK);
		else
			locked_anim_change(p_person,0,ANIM_FALLING);
	}
	else
	{
		if(shotgun)
			queue_anim(p_person,ANIM_FALLING_QUEUED_AK);
		else
			queue_anim(p_person,ANIM_FALLING_QUEUED);

	}
//	p_person->Draw.Tweened->CurrentAnim	=	ANIM_FALLING;
	p_person->Genus.Person->Action=ACTION_DROP_DOWN;
	p_person->Draw.Tweened->AnimTween=0;
	p_person->Draw.Tweened->Locked=0;

//	if((flag&1)==0)
//		p_person->Velocity=-8;

	if (!(flag & PERSON_DROP_DOWN_KEEP_VEL))
	{
		p_person->Velocity = -8;
	}
	else
	{
		if(abs(p_person->Velocity)<8)
		{
			if(p_person->Velocity<0)
				p_person->Velocity=-8;
			else
				p_person->Velocity=8;
		}
	}

	p_person->DeltaVelocity=0;//dv;

//	if((flag&4)==0)
//		p_person->DY=-(4<<8);

	if (!(flag & PERSON_DROP_DOWN_KEEP_DY))
	{
		p_person->DY = -(4 << 8);
	}

	p_person->OnFace=0;

	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M;

}

void	set_person_locked_drop_down(Thing	*p_person,SLONG vely)
{
	MSG_add(" set person drop locked");
	set_generic_person_state_function(p_person,STATE_DANGLING);
	p_person->SubState=SUB_STATE_DROP_DOWN;
	locked_anim_change(p_person,0,ANIM_FALLING);
	p_person->Genus.Person->Action=ACTION_DROP_DOWN;
	p_person->Draw.Tweened->Locked=0;
	p_person->Velocity=0;
	p_person->DeltaVelocity=0;
	p_person->DY=(vely<<8);
	p_person->OnFace=0;
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M;

}

extern	SLONG nearest_point_on_line_and_dist(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z);


SLONG	is_wall_good_for_bump_and_turn(Thing *p_person,SLONG col)
{
	struct	DFacet	*p_facet;


	SLONG	dist,angle,wy,wx,wz;
	SLONG	mx,my,mz,dx,dz;

	angle=p_person->Draw.Tweened->Angle;

	wx=p_person->WorldPos.X>>8;
	wy=p_person->WorldPos.Y>>8;
	wz=p_person->WorldPos.Z>>8;

	dx = -(SIN(angle)) >> 9;
	dz = -(COS(angle)) >> 9;

	mx=(wx+dx)>>8;
	mz=(wz+dz)>>8;


	my=MAVHEIGHT(mx,mz)<<6;

	if(wy>my-196)
		return(0);

	return(1);


/*
	p_facet=&dfacets[col];

	if(p_facet->BlockHeight>8)
		return(1);
	else
		return(0);
*/

}

//#define	VAULT_DA 128
SLONG	am_i_facing_wall(Thing *p_person,SLONG col,SLONG *wall_angle,SLONG vault_da=128)
{
	SLONG	mdx,mdz,dx,dz,len,dist;
	SLONG	near_x,near_z;
	SLONG	wx,wy,wz;
	SLONG	angle;
	GameCoord	new_position;
	SLONG	on,norm_x,norm_z;
	struct	DFacet	*p_facet;
	SLONG	req_dist=50;
	SLONG	side;

	p_facet=&dfacets[col];

	dx = p_facet->x[1] - p_facet->x[0] << 8;
	dz = p_facet->z[1] - p_facet->z[0] << 8;
/*
		AENG_world_line(
			p_facet->x[0]<<8,p_facet->Y[0],p_facet->z[0]<<8,
			16,
			0xff0000,
			p_facet->x[1]<<8,p_facet->Y[0],p_facet->z[1]<<8,
			0,
			0x330088,
			TRUE);
*/

	mdx=abs(dx);
	mdz=abs(dz);


	{
		SLONG da;
		angle=Arctan(-dx,dz)-512; //+512;
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;
		*wall_angle=angle;

		da = abs(angle - p_person->Draw.Tweened->Angle);
//		PANEL_new_text(p_person,10000,"da %d",da);

//		if((da>VAULT_DA && da <1024-VAULT_DA )||(da>1024+VAULT_DA && da <2048-VAULT_DA ))

		if(p_facet->FacetType==STOREY_TYPE_FENCE ||p_facet->FacetType==STOREY_TYPE_FENCE_FLAT)
		{
extern			SLONG which_side(SLONG  x1, SLONG z1,SLONG  x2, SLONG z2,SLONG  a,  SLONG b);

			if(which_side(p_facet->x[0]<<8,p_facet->z[0]<<8,p_facet->x[1]<<8,p_facet->z[1]<<8,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<0)
			{
				// on other side
				da+=1024;

//				da&=2047;

			}
		}
		if((da<vault_da && da>-vault_da) || (da>2048-vault_da && da<2048+vault_da))
			return(1);
	}
	return(0);
}


//
// hopefully this will work with diagonal fences, should they exist
//
SLONG	along_middle_of_facet(Thing *p_person,SLONG col)
{
	SLONG	wx,wy,wz;
	SLONG	on,norm_x,norm_z,dist;
	struct	DFacet	*p_facet;

	//
	// Along the middle of the facet?
	//

	p_facet=&dfacets[col];

	wx = p_person->WorldPos.X >> 8;
	wy = p_person->WorldPos.Y >> 8;
	wz = p_person->WorldPos.Z >> 8;

	signed_dist_to_line_with_normal(
		p_facet->x[0] << 8, p_facet->z[0] << 8,
		p_facet->x[1] << 8, p_facet->z[1] << 8,
		wx, wz,
	   &dist,
	   &norm_x,
	   &norm_z,
	   &on);

	if(on)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

SLONG set_person_pos_for_fence_vault(Thing *p_person,SLONG col)
{
	SLONG	mdx,mdz,dx,dz,len,dist;
	SLONG	near_x,near_z;
	SLONG	wx,wy,wz;
	SLONG	angle;
	GameCoord	new_position;
	SLONG	on,norm_x,norm_z;
	struct	DFacet	*p_facet;
	SLONG	req_dist=50;
	SLONG	side;
	SLONG	bot;
	SLONG	top;

	//
	// Is this person in the correct y for the fence.
	//

	bot = get_fence_bottom(p_person->WorldPos.X >> 8, p_person->WorldPos.Z >> 8, col);
	top = get_fence_top   (p_person->WorldPos.X >> 8, p_person->WorldPos.Z >> 8, col);

	if (!WITHIN(p_person->WorldPos.Y >> 8, bot - 30, top))
	{
		return FALSE;
	}

	p_facet = &dfacets[col];

	dx = p_facet->x[1] - p_facet->x[0] << 8;
	dz = p_facet->z[1] - p_facet->z[0] << 8;

	mdx=abs(dx);
	mdz=abs(dz);

#undef VAULT_DA
#define	VAULT_DA (p_person->Genus.Person->PlayerID ? 128 : 256)

	{
		SLONG da;
		angle=Arctan(-mdx,mdz)+1024+512;
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		da = abs(angle - p_person->Draw.Tweened->Angle);

		if((da>VAULT_DA && da <1024-VAULT_DA )||(da>1024+VAULT_DA && da <2048-VAULT_DA ))
			return(0);

		if (da>512&& da<2048-512)
		{
			angle += 1024;
			angle &= 2047;
		}

	}

	//
	// Along the middle of the fence?
	//

	wx = p_person->WorldPos.X >> 8;
	wy = p_person->WorldPos.Y >> 8;
	wz = p_person->WorldPos.Z >> 8;

	signed_dist_to_line_with_normal(
		p_facet->x[0] << 8, p_facet->z[0] << 8,
		p_facet->x[1] << 8, p_facet->z[1] << 8,
		wx, wz,
	   &dist,
	   &norm_x,
	   &norm_z,
	   &on);

	if(!on)
		return(0);

	//
	// Make sure you are looking at the fence rather than away from it.
	//

	SLONG ldx = -SIN(angle) >> 8;
	SLONG ldz = -COS(angle) >> 8;

	if (!two4_line_intersection(
			p_facet->x[0] << 8, p_facet->z[0] << 8,
			p_facet->x[1] << 8, p_facet->z[1] << 8,
			wx,       wz,
			wx + ldx, wz + ldz))
	{
		return 0;
	}

	//
	// If the other side of the fence is a no-go square...
	//

	if (PAP_2HI(wx + ldx >> 8, wz + ldz >> 8).Flags & PAP_FLAG_NOGO)
	{
		p_person->Velocity=0;
		return 0;
	}

	//
	// Make sure we're not going to vault into a building.
	//

	{
		SLONG flx = wx + ldx + (-ldz >> 3) >> 8;
		SLONG fly = wz + ldz + (+ldx >> 3) >> 8;

		SLONG frx = wx + ldx - (-ldz >> 3) >> 8;
		SLONG fry = wz + ldz - (+ldx >> 3) >> 8;

		SLONG fl_height = MAVHEIGHT(flx,fly) << 6;
		SLONG fr_height = MAVHEIGHT(frx,fry) << 6;

		if (fl_height - (p_person->WorldPos.Y >> 8) > 0x50 ||
			fr_height - (p_person->WorldPos.Y >> 8) > 0x50)
		{
			//
			// Vaulting into a wall!
			//

			return 0;
		}
	}

	//
	// Set angle for fence.
	//

	p_person->Draw.Tweened->Angle = angle;

	//
	// Set position for fence.
	//

	{
		SLONG	len;
		SLONG	adx,adz;
		SLONG	odx,odz;
		if(dist<0)
		{
			norm_x=-norm_x;
			norm_z=-norm_z;
		}
		dist=abs(dist);

		adx=abs(norm_x);
		adz=abs(norm_z);

		len=QDIST2(adx,adz);
		if(len==0)
			len=1;

		mdx=((norm_x*(req_dist-dist))<<8)/len;
		mdz=((norm_z*(req_dist-dist))<<8)/len;


		new_position.X=p_person->WorldPos.X+(mdx);
		new_position.Z=p_person->WorldPos.Z+(mdz);

		new_position.Y=p_person->WorldPos.Y;
		move_thing_on_map(p_person,&new_position);
	}



	return(1);
}

SLONG	set_person_pos_for_fence(Thing *p_person,SLONG col,SLONG set_pos,SLONG req_dist)
{
	SLONG	mdx,mdz,dx,dz,len,dist;
	SLONG	near_x,near_z;
	SLONG	wx,wy,wz;
	SLONG	angle;
	GameCoord	new_position;
	SLONG	on,norm_x,norm_z;
	struct	DFacet	*p_facet;

	p_facet=&dfacets[col];

	dx = p_facet->x[1] - p_facet->x[0] << 8;
	dz = p_facet->z[1] - p_facet->z[0] << 8;

	mdx=abs(dx);
	mdz=abs(dz);

	{
		angle=Arctan(-mdx,mdz)+1024+512;
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		//
		// The angle is never out of range anymore.
		//
		SLONG da = angle - p_person->Draw.Tweened->Angle;

		//
		// and now it is out of range again
		//

		if ((abs(da)>112&& abs(da)<1024-112) || (abs(da)>1024+112 && abs(da)<2048-112))
			return(1);



		if (abs(da)>512&&abs(da)<2048-512)
		{
			angle += 1024;
			angle &= 2047;
		}


		MSG_add(" SET LAND ON FENCE angle %d person angle %d  \n",angle,p_person->Draw.Tweened->Angle);

	}

	//
	// Check to see if there is a nogo zone in front of the person.
	// If so, then she shouldn't grab.
	//

	{
		SLONG dax = -SIN(angle);
		SLONG daz = -COS(angle);

		SLONG mx = p_person->WorldPos.X + dax >> 16;
		SLONG mz = p_person->WorldPos.Z + daz >> 16;

		if (!WITHIN(mx, 0, PAP_SIZE_HI - 1) ||
			!WITHIN(mz, 0, PAP_SIZE_HI - 1))
		{
			return 1;
		}

		if (PAP_2HI(mx,mz).Flags & PAP_FLAG_NOGO)
		{
			return 1;
		}
	}


	//
	// set the angle we want them at before we find out the arse position
	//

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,0,&wx,&wy,&wz);

	wx += p_person->WorldPos.X >> 8;
	wy += p_person->WorldPos.Y >> 8;
	wz += p_person->WorldPos.Z >> 8;

	//
	// check if person can actually grab the fence (i.e their hands are in range
	//

	{
		SLONG fence_top    = get_fence_top   (wx,wz, col);
		SLONG fence_bottom = get_fence_bottom(wx,wz, col);

		if (WITHIN(wy, fence_bottom + 64, fence_top - 64))
		{
			//
			// Okay to grab.
			//
		}
		else
		{
			//
			// Wrong y-position to grab fence.
			//

			return 1;
		}
	}

	//
	// Along the middle of the fence?
	//

	signed_dist_to_line_with_normal(
		p_facet->x[0] << 8, p_facet->z[0] << 8,
		p_facet->x[1] << 8, p_facet->z[1] << 8,
		wx, wz,
	   &dist,
	   &norm_x,
	   &norm_z,
	   &on);

	if(!on)
		return(1);

	//
	// Set position for fence.
	//
		p_person->Draw.Tweened->Angle=angle;

	{
		SLONG	len;
		SLONG	adx,adz;
		if(dist<0)
		{
			norm_x=-norm_x;
			norm_z=-norm_z;
		}
		dist=abs(dist);

		adx=abs(norm_x);
		adz=abs(norm_z);

		len=QDIST2(adx,adz);
		if(len==0)
			len=1;

		mdx=((norm_x*(req_dist-dist))<<8)/len;
		mdz=((norm_z*(req_dist-dist))<<8)/len;

		new_position.X=p_person->WorldPos.X+(mdx);
		new_position.Z=p_person->WorldPos.Z+(mdz);

		new_position.Y=p_person->WorldPos.Y;
		move_thing_on_map(p_person,&new_position);
	}


	return(0);
}

SLONG	set_person_pos_for_half_step(Thing *p_person,SLONG col)
{
	SLONG	dx,dz;
	SLONG	wall_angle;
	struct	DFacet	*p_facet;
	p_facet=&dfacets[col];
	SLONG	on,norm_x,norm_z;

	SLONG	wx,wy,wz;


	if(am_i_facing_wall(p_person,col,&wall_angle))
	{
		SLONG	dist,angle,wy;
		SLONG	mx,my,mz;

		wx=p_person->WorldPos.X>>8;
		wy=p_person->WorldPos.Y>>8;
		wz=p_person->WorldPos.Z>>8;

		dx = -(SIN(wall_angle)) >> 9;
		dz = -(COS(wall_angle)) >> 9;

		mx=(wx+dx)>>8;
		mz=(wz+dz)>>8;

		if (p_person->Genus.Person->Ware)
		{
			//
			// Would this take Darci out of her warehouse?
			//

			if (!WARE_in_floorplan(p_person->Genus.Person->Ware, mx, mz))
			{
				return 0;
			}

			my = WARE_calc_height_at(p_person->Genus.Person->Ware, (mx << 8) + 0x80, (mz << 8) + 0x80);
		}
		else
		{
			my = MAVHEIGHT(mx,mz) << 6;

		}

		if(wy<my-196 || wy>my-64)
			return(0);


		signed_dist_to_line_with_normal(
			p_facet->x[0] << 8, p_facet->z[0] << 8,
			p_facet->x[1] << 8, p_facet->z[1] << 8,
			wx, wz,
		   &dist,
		   &norm_x,
		   &norm_z,
		   &on);

		if(!on)
			return(0);

		{
			SLONG ax1;
			SLONG az1;
			SLONG ax2;
			SLONG az2;

			if (p_facet->x[0] == p_facet->x[1])
			{
				ax1 = p_facet->x[0] << 8;
				ax2 = p_facet->x[0] << 8;

				az1 = wz & ~0xff;
				az2 = az1 + 256;
			}
			else
			{
				ax1 = wx & ~0xff;
				ax2 = ax1 + 256;

				az1 = p_facet->z[0] << 8;
				az2 = p_facet->z[0] << 8;

			}

			//
			// If there is a fence lying along this facet- then we
			// can't step up 'through' the fence.
			//

			if (does_fence_lie_along_line(
					ax1, az1,
					ax2, az2))
			{
				return FALSE;
			}
		}

		if(abs(dist)<60)
		{
			dist=abs(dist);
			p_person->Draw.Tweened->Angle=wall_angle;
			angle=(p_person->Draw.Tweened->Angle)&2047;

			dx = (SIN(angle) * (40-dist)) >> 8;
			dz = (COS(angle) * (40-dist)) >> 8;
			person_normal_move_dxdz(p_person,dx,dz);
		//	move_thing_on_map_dxdydz(p_person,dx,0,dz);
			return(1);
		}
	}
	return(0);

}

inline	SLONG	is_facet_vaultable(SLONG facet)
{
	struct DFacet	*p_facet;

	p_facet=&dfacets[facet];
	if(p_facet->FacetType==STOREY_TYPE_FENCE ||p_facet->FacetType==STOREY_TYPE_FENCE_FLAT)
//	if((p_facet->FacetFlags & FACET_FLAG_ONBUILDING) ==0)
	if(p_facet->Height==2)
	if(p_facet->BlockHeight==16)
	{
		return(1);
	}

	return(0);
}

inline	SLONG	is_facet_half_step(SLONG facet)
{
	struct DFacet	*p_facet;

	p_facet=&dfacets[facet];

	if(p_facet->FacetType==STOREY_TYPE_NORMAL)
	if((p_facet->Height * p_facet->BlockHeight << 2)==128)
	{
		return(1);
	}

	return(0);
}

SLONG	set_person_land_on_fence(Thing	*p_person,SLONG col,SLONG set_pos,SLONG while_walking)
{
	SLONG	ret;
	SLONG	dist=45;

	SLONG	wall_angle;

	if(!am_i_facing_wall(p_person,col,&wall_angle,256))
	{
		return(0);
	}

	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
		dist=60;

	MSG_add(" set person land on fence");
	
	if(while_walking==0)
	if (is_facet_vaultable(col))
	{
//		return;
		
		if (set_person_vault(p_person, col))
		{
			return 1;
		}
	}

	//
	// Leave fight mode when you grab something.
	//

	p_person->Genus.Person->Mode = PERSON_MODE_RUN;

	if(ret=set_person_pos_for_fence(p_person,col,set_pos,dist))
	{
		switch(ret)
		{
			case	1:
				//
				// hands above it
				//

//				if(p_person->SubState == SUB_STATE_DROP_DOWN ||p_person->SubState == SUB_STATE_DROP_DOWN_OFF_FACE)
					return(0);
				if(while_walking==0)
					set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL|PERSON_DROP_DOWN_KEEP_DY);
				p_person->Velocity=1;
				break;

		}
		return(1);
	}
	set_generic_person_state_function(p_person,STATE_CLIMBING);
	p_person->SubState=SUB_STATE_CLIMB_LANDING;
	queue_anim(p_person,ANIM_LAND_ON_FENCE);
	/*
	p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_LAND_ON_FENCE];
	p_person->Draw.Tweened->CurrentAnim	=	ANIM_LAND_ON_FENCE;
	*/
	p_person->Genus.Person->Action=ACTION_CLIMBING;
	p_person->Draw.Tweened->Locked=0;
	p_person->Velocity=0;
	p_person->DeltaVelocity=-2;
	p_person->DY=0;
	p_person->Genus.Person->OnFacet=col; //_vects[col].Face;   //wall
	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;
	return(1);

}

SLONG	set_person_kick_off_wall(Thing	*p_person,SLONG col,SLONG set_pos)
{
	SLONG	dist=100;
	MSG_add(" set person land on fence");
	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
		dist=110;
	
	if(set_person_pos_for_fence(p_person,col,set_pos,dist)==-1)
		return(0);
	set_generic_person_state_function(p_person,STATE_FIGHTING);
	p_person->SubState=SUB_STATE_WALL_KICK;
	queue_anim(p_person,ANIM_WALL_KICK);

	p_person->Genus.Person->Action=ACTION_FIGHT_KICK;
	p_person->Draw.Tweened->Locked=0;
	p_person->Velocity=0;
	p_person->DeltaVelocity=0;
	p_person->DY=0;
//	p_person->OnFace=-col_vects[col].Face;   //wall
//	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;
	return(1);

}

SLONG	fight_any_gang_attacker(Thing *p_person)
{
	if(p_person->SubState!=SUB_STATE_IDLE_CROUTCH_ARREST)
	if(p_person->Genus.Person->GangAttack)
	{
		UWORD	attacker;
		if(attacker=get_any_gang_member(p_person))
		{
			// you are under attack by someone
			ASSERT(TO_THING(attacker)->Class==CLASS_PERSON);
			ASSERT(TO_THING(attacker)->State!=STATE_DEAD);
			p_person->Genus.Person->Target=attacker; 
			if(p_person->Genus.Person->Mode!=PERSON_MODE_FIGHT)
			{
				person_enter_fight_mode(p_person);
				set_anim(p_person,ANIM_FIGHT_READY);
			}
		}
	}

	return(0);
}

UWORD find_arrestee(Thing *p_person)
{
	SLONG i;
	SLONG dist;
	SLONG score;
	SLONG best_score;
	SLONG best_answer;

	best_score  = 0;
	best_answer = NULL;

	extern THING_INDEX col_with[];

	SLONG col_with_upto = THING_find_sphere(
							p_person->WorldPos.X>>8,
							p_person->WorldPos.Y>>8,
							p_person->WorldPos.Z>>8,
							0x280,
							col_with,
							MAX_COL_WITH,
							1 << CLASS_PERSON);

	for (i = 0; i < col_with_upto; i++)
	{
		Thing *p_found = TO_THING(col_with[i]);

		if (p_found->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
		{
			//
			// You can't arrest invulnerable people.
			//

			if (p_found->Genus.Person->pcom_ai != PCOM_AI_FIGHT_TEST)
			{
				//
				// But you can arrest FIGHT_TEST DUMMIES!
				//

				continue;
			}
		}

		dist  = dist_to_target_pelvis(p_person, p_found);
		score = 0;

		if (dist < 128)
		{
			if (is_person_ko_and_lay_down(p_found))
			{
				score = 500 - dist;

				if (p_found->Genus.Person->PersonType == PERSON_MIB1 ||
					p_found->Genus.Person->PersonType == PERSON_MIB2 ||
					p_found->Genus.Person->PersonType == PERSON_MIB3 )
				{
					score=0;
				}

				if (p_found->Genus.Person->PersonType == PERSON_COP)
				{
					if 	(!(p_found->Genus.Person->Flags2 & FLAG2_PERSON_GUILTY))
					{
						//
						// cant arrest cops!
						//
						score=0;
					}
				}

//				if (p_found->Genus.Person->PersonType == PERSON_COP   || 
				if(	p_found->Genus.Person->PersonType == PERSON_ROPER ||
					p_found->Genus.Person->PersonType == PERSON_HOSTAGE)
				{
					score -= 250;
				}

				if (p_found->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD && EWAY_get_person(p_found->Genus.Person->pcom_ai_other) == THING_NUMBER(p_person))
				{
					//
					// Don't arrest your own bodyguards!
					//

					score = 0;
				}

			}
			else
			if (p_found->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
			{
				if (p_found->State == STATE_DEAD && p_found->SubState == SUB_STATE_DEAD_INJURED)
				{
					score = 500 - dist;
				}
			}
		}

		if (score > 0)
		{
			if (best_score < score)
			{
				best_score  = score;
				best_answer = col_with[i];
			}
		}
	}

	return best_answer;

	/*

	UWORD	s_index;
	Thing	*p_target;
extern	SLONG THING_find_nearest_person(Thing *p_person,SLONG radius,ULONG classes);
	s_index = THING_find_nearest_person(p_person,256,1<<CLASS_PERSON);

	if(s_index)
	{
		p_target=TO_THING(s_index);


		if(dist_to_target_pelvis(p_person,p_target)<128 && is_person_ko(p_target))	// 128 was 64
		{
			return(s_index);
		}
	}

	return(0);

	*/
}

UWORD	find_corpse(Thing *p_person)
{
	UWORD	s_index;
	Thing	*p_target;
extern	SLONG THING_find_nearest_person(Thing *p_person,SLONG radius,ULONG classes);
	s_index = THING_find_nearest_person(p_person,256,1<<CLASS_PERSON);

	if(s_index)
	{
		p_target=TO_THING(s_index);


		if(dist_to_target_pelvis(p_person,p_target)<128 && p_target->State == STATE_DEAD && p_target->SubState == SUB_STATE_DEAD_INJURED)	// 128 was 64
		{
			return(s_index);
		}
	}

	return(0);
}

UWORD	perform_arrest(Thing *p_person,UWORD s_index)
{
	Thing	*p_target;

	p_target=TO_THING(s_index);


	if(s_index)
	{
		SLONG	ax,ay,az;
/*
		calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&ax,&ay,&az);
		ax+=p_person->WorldPos.X>>8;
		ay+=p_person->WorldPos.Y>>8;
		az+=p_person->WorldPos.Z>>8;

extern	void	add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text);
		add_damage_text(ax,ay,az,"Arrested");
*/
		PANEL_new_info_message(XLAT_str(X_ARRESTED));

		return(1);
	}
	return(0);
	
}

void	lock_to_compass(Thing *p_thing);

void	fn_person_search(Thing *p_person)
{
	UWORD last_timer;

	switch(p_person->SubState)
	{
		case	SUB_STATE_SEARCH_PRIM:
		case	SUB_STATE_SEARCH_CORPSE:

			person_normal_animate(p_person);

			if(!continue_pressing_action(p_person))
			{
				MFX_stop(THING_NUMBER(p_person),S_SEARCH_END);

				if (p_person->SubState == SUB_STATE_SEARCH_CORPSE)
				{
					set_anim(p_person, ANIM_CROUTCH_GETUP);
					p_person->SubState = SUB_STATE_SEARCH_GETUP;
				}
				else
				{
					set_person_idle(p_person);
				}

				return;
			}

			last_timer = p_person->Genus.Person->Timer1;

			p_person->Genus.Person->Timer1+=400 * TICK_RATIO >> TICK_SHIFT;

			if(p_person->Genus.Person->Timer1>=(125<<8))	// Stays at 100 for a while!
			{
				set_action_used(p_person);
				set_person_idle(p_person);

				return;
			}

			if (last_timer < (100 << 8) &&
				p_person->Genus.Person->Timer1 >= (100 << 8))
			{
				release_searched_item(p_person);


				if (p_person->SubState == SUB_STATE_SEARCH_CORPSE)
				{
					set_anim(p_person, ANIM_CROUTCH_GETUP);
					p_person->SubState = SUB_STATE_SEARCH_GETUP;
					p_person->Genus.Person->Target=0;
				}
			}

			break;
		
		case SUB_STATE_SEARCH_GETUP:
			
			{
				SLONG end = person_normal_animate(p_person);

				if (end)
				{
					set_action_used(p_person);
					set_person_idle(p_person);
				}
			}

			break;

	}
	
}

void	set_person_random_idle(Thing *p_person)
{
	SLONG	anim=Random();

	if(person_holding_2handed(p_person))
	{
		queue_anim(p_person,anim);
	}

	p_person->Genus.Person->Timer1=(Random()&0xff)+400;

	switch(p_person->Genus.Person->PersonType)
	{
		case	PERSON_ROPER:
				queue_anim(p_person,anim&1?ANIM_BREATHE:ANIM_IDLE_SCRATCH1);
				break;

		case	PERSON_COP:
			{
				switch(anim&3)
				{
					case	0:
						anim=COP_ROPER_ANIM_IDLE1;
						break;
					case	1:
						anim=COP_ROPER_ANIM_IDLE2;
						break;
					case	2:
						anim=COP_ROPER_ANIM_IDLE3;
						break;
					case	3:
						anim=COP_ROPER_ANIM_IDLE4;
						break;
				}
				p_person->Draw.Tweened->QueuedFrame	= game_chunk[ANIM_TYPE_ROPER].AnimList[anim]; 
				p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
				p_person->Draw.Tweened->CurrentAnim	=	anim;
				p_person->Draw.Tweened->FrameIndex=0;
			}
				
			break;
/*
		case	PERSON_CIV:
				switch(p_person->Draw.Tweened->MeshID)
				{
					case	7:
					case	8:
						//
						// male idle's
						//
						set_anim_of_type(p_person,CIV_M_ANIM_STAND,ANIM_TYPE_CIV);
						break;
					case	9:
						//
						// female idle's
						//
						set_anim(p_person,ANIM_STAND_HIP);

						break;
					default:
						ASSERT(0);
						break;

				}
				break;
*/
		default:
				switch(anim%3)
				{
					case	0:
						anim=ANIM_IDLE_SCRATCH1;
						break;
					case	1:
						anim=ANIM_IDLE_SCRATCH2;
						break;
					case	2:
						anim=ANIM_BREATHE;
						break;
				}

				queue_anim(p_person,anim);
			break;
	}

}

void	fn_person_idle(Thing *p_person)
{
	SLONG end;

#ifndef NDEBUG
/*
	if(person_holding_2handed(p_person))
	{
				AENG_world_line(
					(p_person->WorldPos.X >> 8),
					(p_person->WorldPos.Y >> 8) + 0x60,
					(p_person->WorldPos.Z >> 8),
					32,
					0x00ffffff,
					(p_person->WorldPos.X >> 8),
					(p_person->WorldPos.Y >> 8) + 0x160,
					(p_person->WorldPos.Z >> 8),
					0,
					0x00123456,
					TRUE);
	}
*/
#endif

/*
	if(Keys[KB_7]&&ShiftFlag)
	{
		plant_feet(p_person);

	}
	if(Keys[KB_7]&&!ShiftFlag)
	{
		p_person->WorldPos.Y=0;
	}
*/

	if ((world_type==WORLD_TYPE_SNOW)&&!PersonIsMIB(p_person))
	{
		// breathe
		if (MagicFrameCheck(p_person,5))
		{
			SLONG x,y,z;
			calc_sub_objects_position(
				p_person, p_person->Draw.Tweened->AnimTween,
				SUB_OBJECT_HEAD,
				&x, &y, &z);
			x<<=8; y<<=8; z<<=8;
			x+=p_person->WorldPos.X;
			y+=p_person->WorldPos.Y;
			z+=p_person->WorldPos.Z;
			PARTICLE_Add(x,y,z,256,10,256,POLY_PAGE_SMOKECLOUD2,
				2+((Random()&3)<<2),0x7FFFFFFF,PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE,
				30,30,1,10,1);
		}
	}

	SlideSoundCheck(p_person);

	if(p_person->Genus.Person->Mode==PERSON_MODE_FIGHT)
	{
		if(p_person->SubState!=SUB_STATE_IDLE_CROUTCH_ARREST)
		{
			if(p_person->Genus.Person->PlayerID)
			{
				if(p_person->Genus.Person->Target )
				{
	//				ASSERT(TO_THING(p_person->Genus.Person->Target)->State!=STATE_DEAD);

				}
				else
				{
					//ASSERT(count_gang(p_person)==0);
				}

			}
			if(p_person->Genus.Person->Target )
				if(TO_THING(p_person->Genus.Person->Target)->State!=STATE_DEAD)
					turn_to_face_thing(p_person,TO_THING(p_person->Genus.Person->Target),0);

			//if(count_gang(p_person)==0)
			{
				Thing *p_attacker = is_person_under_attack_low_level(p_person, FALSE, 0x200);

				if (p_attacker == NULL)
				{
					//
					// nobody to fight, wait a while then exit fight mode
					//
	//				if(p_person->Genus.Person->Agression++>100)
					{
						p_person->Genus.Person->Agression=0;
						p_person->Genus.Person->Mode=PERSON_MODE_RUN;
						p_person->Genus.Person->Target=0;
						set_person_idle(p_person);
						if(p_person->State==STATE_IDLE)
							set_anim(p_person,ANIM_UNFIGHT);
					}
				}
				else
				{
					if (p_person->Genus.Person->Target == NULL)
					{
						p_person->Genus.Person->Target = THING_NUMBER(p_attacker);
					}
				}
			}
			/*
			else
			{
				//
				// I'm fighting but I'm idle, my agression must be 0
				//
//				p_person->Genus.Person->Agression=0;
			}
			*/
		}
		

	}
	else
	{
		
		if (p_person->OnFace>0)
		{
			ASSERT(WITHIN(p_person->OnFace, 1, next_prim_face4 - 1));

			PrimFace4 *f4 = &prim_faces4[p_person->OnFace];

			ASSERT(f4->FaceFlags & FACE_FLAG_WALKABLE);

			//
			// Take out the surfing for now...
			//

			#if WE_WANT_THE_SURFING_THAT_BREAKS_WHEN_YOU_PRESS_C

			if (f4->FaceFlags & FACE_FLAG_WMOVE)
			{
				SLONG	wmove_index;

				WMOVE_Face *wf;
				Thing      *p_thing;

				wmove_index=f4->ThingIndex;

				wf = &WMOVE_face[wmove_index];

				if(wf->thing)
				{
					p_thing = TO_THING(wf->thing);
					if(p_thing->Class==CLASS_VEHICLE)
					{
						if(p_thing->Velocity>100)
						{
							if(person_holding_2handed(p_person))
							{
								if(p_person->Draw.Tweened->CurrentAnim!=ANIM_SURF_AK)
									set_anim(p_person,ANIM_SURF_AK);
							}
							else
							{
								if(p_person->Draw.Tweened->CurrentAnim!=ANIM_SURF)
									set_anim(p_person,ANIM_SURF);
							}

						}
						else
						{
							if(p_person->Draw.Tweened->CurrentAnim!=ANIM_STAND_READY)
								set_anim(p_person,ANIM_STAND_READY);

						}
					}
					person_normal_animate(p_person);
					return;
				}
			}
			
			#endif
		}


		if (p_person->Genus.Person->PlayerID &&	p_person->Genus.Person->Mode != PERSON_MODE_FIGHT && !person_has_gun_out(p_person))
		{
			//
			// You are a player and you're not in fight mode.
			//

			if (p_person->SubState!=SUB_STATE_IDLE_CROUTCH_ARREST)
			{
				Thing *p_attacker = is_person_under_attack_low_level(p_person, FALSE, 0x100);


				if(p_person->Genus.Person->pcom_colour)
				{
					p_person->Genus.Person->pcom_colour--;
					

				}
				else
				if (p_attacker)
				{
					SLONG anim;

					//
					// You are under attack by someone.
					//

					ASSERT(p_attacker->Class==CLASS_PERSON);

					p_person->Genus.Person->Target = THING_NUMBER(p_attacker);
					person_enter_fight_mode(p_person);		//for fuck's sake get in fight mode
					anim = find_idle_fight_stance(p_person);
					set_anim(p_person,anim);
				}
			}

			/*
			if(p_person->Genus.Person->GangAttack)
			{
				UWORD	attacker;
				if(attacker=get_any_gang_member(p_person))
				{
					SLONG	anim;
					// you are under attack by someone
					ASSERT(TO_THING(attacker)->Class==CLASS_PERSON);
					p_person->Genus.Person->Target=attacker; //get_any_gang_member(p_person);
					person_enter_fight_mode(p_person);		//for fuck's sake get in fight mode

					anim=find_idle_fight_stance(p_person);
					set_anim(p_person,anim);
//					CONSOLE_text("doing change 2",1000);
					
				}
			}
			*/
		}
	}

	switch(p_person->SubState)
	{
		case	0:
			//
			// check to see if you should slip down the slope your idle on
			//
			{
				SLONG	slope,angle;
				if(p_person->Genus.Person->InsideIndex)
				{
					slope=0;
				}
				else
				{

					if(p_person->OnFace<0)
					{
						slope=RFACE_on_slope(-p_person->OnFace,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle);
					}
					else
					{

						slope=PAP_on_slope(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,&angle)>>1;
					}

					if(slope>50)
					{
						set_anim(p_person,ANIM_FALLING);
						set_generic_person_state_function(p_person,STATE_MOVEING);
						p_person->SubState=SUB_STATE_SLIPPING;
						p_person->Draw.Tweened->AngleTo=angle;
						slope=MIN(slope-50,10);
						slope=MAX(slope,50);
						change_velocity_to(p_person,slope);

						return;
					}
				}
			}


//				if(p_thing->Genus.Person->AnimType!=2)

				
				if(p_person->Draw.Tweened->DRoll && p_person->Draw.Tweened->FrameIndex==2) //&& (p_person->Genus.Person->AnimType!=2))
				{
					end=0;
				}
				else
				{
					end = person_normal_animate(p_person);
/*
					if(p_person->Genus.Person->PlayerID)
					if(p_person->Genus.Person->Flags2&FLAG2_PERSON_LOOK)
						lock_to_compass(p_person);	
*/

				}
				p_person->Draw.Tweened->DRoll=0;

				if(end&&(p_person->Draw.Tweened->CurrentAnim!=ANIM_IDLE_SCRATCH1))
				{
					p_person->Draw.Tweened->CurrentAnim	=	ANIM_STAND_READY;

				}

				//
				// Is this person holding a baseball bat?
				//
/*
				if (p_person->Genus.Person->SpecialUse &&
					TO_THING(p_person->Genus.Person->SpecialUse)->Genus.Special->SpecialType == SPECIAL_BASEBALLBAT)
				{
				}
				else
				if (p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT)
				{
				}
				else
*/
				if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
				{
					if (end == 1)
					{	
						SLONG	anim;
						anim=find_idle_fight_stance(p_person);

						queue_anim(p_person, anim);
					}
				}
				else
				{
					if(p_person->Genus.Person->SpecialUse)
					{

					}
					else
					{
						
#ifndef	PSX
						if(p_person->Genus.Person->Timer1--==0)
						{
							set_person_random_idle(p_person);
/*
							SWORD the_anim=ANIM_IDLE_SCRATCH1+(p_person->Genus.Person->Timer1&1);
							p_person->Genus.Person->Timer1=(Random()&0xff)+400;


							if (p_person->Genus.Person->PersonType==PERSON_ROPER) // naff hack
								the_anim=ANIM_IDLE_SCRATCH1;

//							p_person->Draw.Tweened->QueuedFrame	= global_anim_array[p_person->Genus.Person->AnimType][the_anim]; //teeter
							queue_anim(p_person,the_anim);							
*/
						}

						// "It says here that 'Drinky Winky' of the teletubbies is obviously
						//  an abusive drunk because of the bottle of booze he carries..."
						// "THAT'S NOT A BOTTLE OF BOOZE! It's his magic bottle that makes
						//  his problems go away! ... ... ... ... um ... ... nevermind."
						if ((p_person->Genus.Person->PersonType==PERSON_ROPER)&&(p_person->Draw.Tweened->CurrentAnim==ANIM_IDLE_SCRATCH1)) {
							if (p_person->Draw.Tweened->FrameIndex>=3) {
								if (p_person->Draw.Tweened->FrameIndex>=17)
							//		p_person->Draw.Tweened->PersonID = 0; // back to normal hand
									p_person->Draw.Tweened->PersonID&=  ~0xe0;
								else
								{
									p_person->Draw.Tweened->PersonID&=  ~0xe0;
									p_person->Draw.Tweened->PersonID|= 7<<5; // roper's magic bottle
								}
							}
						}
#endif

						/*

						if(Keys[KB_0])
						{
							p_person->Draw.Tweened->QueuedFrame	= global_anim_array[p_person->Genus.Person->AnimType][61]; //teeter
						}

						*/

						/*
						if(0 && end==1)
						{
							switch (p_person->Genus.Person->AnimType) 
							{// darci
//							case ANIM_TYPE_ROPER: 
//								set_locked_anim(p_person,ANIM_BREATHE,SUB_OBJECT_LEFT_FOOT);
//								break;
							default:
//								p_person->Draw.Tweened->QueuedFrame	= global_anim_array[p_person->Genus.Person->AnimType][ANIM_BREATHE]; 
								break;

							}
							//p_person->Draw.Tweened->PersonID    = 0;

						}
						*/
					}
				}

				break;

		case	SUB_STATE_IDLE_CROUTCH:
				end = person_normal_animate(p_person);
				
				if(end==1)
				{
					p_person->SubState=SUB_STATE_IDLE_CROUTCHING;
					p_person->Genus.Person->Flags&=~FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
/*
					if(perform_arrest(p_person))
					{
						set_person_idle(p_person);
					}
*/
				}
				break;
		case	SUB_STATE_IDLE_CROUTCH_ARREST:
			end = person_normal_animate(p_person);
			
			if(end==1)
			{
				//p_person->SubState=SUB_STATE_IDLE_CROUTCHING;
				p_person->Genus.Person->Flags&=~FLAG_PERSON_NON_INT_M; //|FLAG_PERSON_NON_INT_C;
				perform_arrest(p_person,p_person->Genus.Person->Target);



				set_person_idle(p_person);
			}
			break;
		case	SUB_STATE_IDLE_CROUTCHING:
			break;

		case	SUB_STATE_IDLE_UNCROUCH:

			end = person_normal_animate(p_person);

			if(p_person->Genus.Person->PersonType==PERSON_ROPER)
			{
				if(p_person->Draw.Tweened->FrameIndex>1)
					end=1;
			}

			if (end)
			{
				set_person_idle(p_person);

				/*
				SLONG anim;
				p_person->SubState=0;

				if (person_has_gun_out(p_person))
				{
					set_person_aim(p_person);

					return;
				}

				p_person->Genus.Person->Timer1=(Random()&0xff)+100;
				set_generic_person_state_function(p_person,STATE_IDLE);
				p_person->Genus.Person->Action=ACTION_IDLE;
				p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

				p_person->Velocity=0;

				if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
				{
					anim = ANIM_FIGHT;
				}
				else
				{
					anim = ANIM_STAND_READY;
				}

				set_anim(p_person,anim);
				locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,anim); 
				set_person_sidle(p_person);
				*/
			}

			break;
	}

}

void PCOM_set_person_ai_flee_person(Thing *p_person,Thing *p_scary);
void PCOM_make_driver_run_away(Thing *p_driver, Thing *p_scary);

void	set_person_in_vehicle(Thing *p_person,Thing *p_car)
{
	SLONG sample;

	if(p_car->Genus.Vehicle->Driver)
	{
		Thing	*other_driver;

//		ASSERT(0); // lets check it works
		//
		// this car has a driver allready, we must have both got in at the same time!
		//

		other_driver=TO_THING(p_car->Genus.Vehicle->Driver);

		if(other_driver->Genus.Person->PlayerID)
		{
			//
			// theres a player in the car, let them drive, we will simply run away
			//
			p_person->Genus.Person->InCar = 0;
			PCOM_set_person_ai_flee_person(p_person, other_driver);
		}
		else
		if(p_person->Genus.Person->PlayerID)
		{
			//
			// player getting in a car that allready has a driver
			//
			p_car->Velocity=0; //make sure car is stopped dead so driver can get out

			PCOM_make_driver_run_away(other_driver, p_person); // make current driver run away

		}


		





	}

	ASSERT(p_car->Class==CLASS_VEHICLE);
	p_person->Genus.Person->Flags|=FLAG_PERSON_DRIVING;
	p_person->Genus.Person->InCar = THING_NUMBER(p_car);
	p_person->SubState = SUB_STATE_INSIDE_VEHICLE;
	remove_thing_from_map(p_person);
	p_car->Genus.Vehicle->Flags|=FLAG_FURN_DRIVING;
	p_car->Genus.Vehicle->Flags&=~FLAG_VEH_ANIMATING;
	p_car->Genus.Vehicle->Driver=THING_NUMBER(p_person);
	set_state_function(p_car, STATE_FDRIVING);

	switch(p_car->Genus.Vehicle->Type)
	{
	case VEH_TYPE_AMBULANCE:
	case VEH_TYPE_VAN:
	case VEH_TYPE_JEEP:
	case VEH_TYPE_MEATWAGON:
	case VEH_TYPE_WILDCATVAN:
		sample=S_VAN_START;
		break;

	case VEH_TYPE_CAR:
	case VEH_TYPE_TAXI:
	case VEH_TYPE_POLICE:
	case VEH_TYPE_SEDAN:
		sample=S_CAR_START;
		break;

	default:
		ASSERT(0);
	}
//	p_car->Genus.Vehicle->SoundFX=play_quick_wave_xyz(p_car->WorldPos.X,p_car->WorldPos.Y,p_car->WorldPos.Z,sample,p_car->Genus.Vehicle->SoundFX,0);

	// only darci's vehicle makes sound...
//	if (p_person==NET_PERSON(0))
//	  MFX_play_thing(THING_NUMBER(p_car),sample,MFX_REPLACE|MFX_MOVING,p_car);
	switch(p_car->Genus.Vehicle->Type)
	{
	case VEH_TYPE_AMBULANCE:
	case VEH_TYPE_VAN:
	case VEH_TYPE_JEEP:
	case VEH_TYPE_MEATWAGON:
	case VEH_TYPE_WILDCATVAN:
		sample=S_VAN_IDLE;
		break;

	case VEH_TYPE_CAR:
	case VEH_TYPE_TAXI:
	case VEH_TYPE_POLICE:
	case VEH_TYPE_SEDAN:
		sample=S_CAR_IDLE;
		break;

	default:
		ASSERT(0);
	}
//	play_quick_wave_xyz(p_car->WorldPos.X,p_car->WorldPos.Y,p_car->WorldPos.Z,sample,p_car->Genus.Vehicle->SoundFX,WAVE_PLAY_QUEUE|WAVE_LOOP);

	//add this back in when queueing's done.
//	if (p_person==NET_PERSON(0))
//	  MFX_play_thing(THING_NUMBER(p_car),sample,MFX_QUEUED|MFX_LOOPED|MFX_MOVING,p_car);

//	set_vehicle_anim(p_car,1);
}

void	set_person_out_of_vehicle(Thing *p_person)
{
	Thing	*p_car;

	ASSERT(p_person->Genus.Person->InCar);

	p_car=TO_THING(p_person->Genus.Person->InCar);

	ASSERT(p_car->Class==CLASS_VEHICLE);

	p_person->Genus.Person->Flags&=~(FLAG_PERSON_DRIVING|FLAG_PERSON_PASSENGER);
	p_person->Genus.Person->InCar = 0;
//	add_thing_to_map(p_person);
	set_person_locked_idle_ready(p_person);
	plant_feet(p_person);
	p_car->Genus.Vehicle->Flags&=~FLAG_FURN_DRIVING;
//	set_vehicle_anim(p_car,1);
/*
	MFX_stop(THING_NUMBER(p_car),S_CARX_START);
	MFX_stop(THING_NUMBER(p_car),S_CARX_START+1);
	MFX_play_thing(THING_NUMBER(p_car),S_CARX_END,MFX_MOVING|MFX_REPLACE,p_car);
	MFX_play_thing(THING_NUMBER(p_car),S_NULL,MFX_QUEUED|MFX_SHORT_QUEUE,p_car);
	p_car->Genus.Vehicle->Flags^=FLAG_VEH_FX_STATE;
*/
}


//
// Change from current anim status to a totally fresh one, 
// keeping a limb in the same place
//

void	locked_anim_change(Thing *p_person,UWORD locked_object,UWORD anim,SLONG dangle)
{
	SLONG	lock_x1,lock_y1,lock_z1;
	SLONG	lock_x2,lock_y2,lock_z2;
	DrawTween	*draw_info;
	GameCoord	temp_pos;

	ASSERT(anim);

	ASSERT(global_anim_array[p_person->Genus.Person->AnimType][anim]);

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x1,&lock_y1,&lock_z1);

	draw_info               =   p_person->Draw.Tweened;
	draw_info->CurrentFrame	=	global_anim_array[p_person->Genus.Person->AnimType][anim];
	draw_info->QueuedFrame	=	0; 
	draw_info->NextFrame	=	draw_info->CurrentFrame->NextFrame;
	draw_info->AnimTween    =   0;
	draw_info->CurrentAnim	=	anim;
	draw_info->FrameIndex	=	0;

	draw_info->Angle+=dangle;
	draw_info->Angle&=2047;

	calc_sub_objects_position(p_person,0,locked_object,&lock_x2,&lock_y2,&lock_z2);
	//
	// difference in lock co-ord is ammount to move by to maintain same position for limb
	// 

	temp_pos.X=((+lock_x1-lock_x2)<<8)+p_person->WorldPos.X;
	temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
	temp_pos.Z=((+lock_z1-lock_z2)<<8)+p_person->WorldPos.Z;

	//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
	//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);

	//LogText(" locked anim change dx dy dz %d %d %d \n",lock_x1-lock_x2,lock_y1-lock_y2,lock_z1-lock_z2);

	move_thing_on_map(p_person,&temp_pos);
}
void	locked_anim_change_of_type(Thing *p_person,UWORD locked_object,UWORD anim,SLONG type)
{
	SLONG	lock_x1,lock_y1,lock_z1;
	SLONG	lock_x2,lock_y2,lock_z2;
	DrawTween	*draw_info;
	GameCoord	temp_pos;

	ASSERT(anim);

	ASSERT(global_anim_array[type][anim]);

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x1,&lock_y1,&lock_z1);

	draw_info               =   p_person->Draw.Tweened;
	draw_info->CurrentFrame	=	game_chunk[type].AnimList[anim];//global_anim_array[type][anim];
	draw_info->QueuedFrame	=	0; 
	draw_info->NextFrame	=	draw_info->CurrentFrame->NextFrame;
	draw_info->AnimTween    =   0;
	draw_info->CurrentAnim	=	anim;

	calc_sub_objects_position(p_person,0,locked_object,&lock_x2,&lock_y2,&lock_z2);
	//
	// difference in lock co-ord is ammount to move by to maintain same position for limb
	// 

	temp_pos.X=((+lock_x1-lock_x2)<<8)+p_person->WorldPos.X;
	temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
	temp_pos.Z=((+lock_z1-lock_z2)<<8)+p_person->WorldPos.Z;

	//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
	//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);

	//LogText(" locked anim change dx dy dz %d %d %d \n",lock_x1-lock_x2,lock_y1-lock_y2,lock_z1-lock_z2);

	move_thing_on_map(p_person,&temp_pos);
}

void	locked_anim_change_height_type(Thing *p_person,UWORD locked_object,UWORD anim,SLONG type)
{
	SLONG	lock_x1,lock_y1,lock_z1;
	SLONG	lock_x2,lock_y2,lock_z2;
	DrawTween	*draw_info;
	GameCoord	temp_pos;

//	ASSERT(global_anim_array[type][anim]);

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x1,&lock_y1,&lock_z1);

	draw_info               =   p_person->Draw.Tweened;
	draw_info->CurrentFrame	=	game_chunk[type].AnimList[anim]; //global_anim_array[type][anim];
	draw_info->QueuedFrame	=	0; 
	draw_info->NextFrame	=	draw_info->CurrentFrame->NextFrame;
	draw_info->AnimTween    =   0;
	draw_info->CurrentAnim	=	anim;

	calc_sub_objects_position(p_person,0,locked_object,&lock_x2,&lock_y2,&lock_z2);
	//
	// difference in lock co-ord is ammount to move by to maintain same position for limb
	// 

	temp_pos.X=p_person->WorldPos.X;
	temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
	temp_pos.Z=p_person->WorldPos.Z;

	//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
	//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);

	//LogText(" locked anim change dx dy dz %d %d %d \n",lock_x1-lock_x2,lock_y1-lock_y2,lock_z1-lock_z2);

	move_thing_on_map(p_person,&temp_pos);
}

SLONG	set_limb_to_y(Thing *p_person,SLONG obj,SLONG y)
{
	SLONG	x1,y1,z1;
	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,obj,&x1,&y1,&z1);
	y1+=(p_person->WorldPos.Y>>8);

	y-=y1;

	p_person->WorldPos.Y+=y<<8;
	return(0);
}


void	locked_next_anim_change(Thing *p_person,UWORD locked_object,GameKeyFrame *queued_frame)
{
	SLONG	lock_x1,lock_y1,lock_z1;
	SLONG	lock_x2,lock_y2,lock_z2;
	DrawTween	*draw_info;
	GameCoord	temp_pos;

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x1,&lock_y1,&lock_z1);

	draw_info=p_person->Draw.Tweened;
	draw_info->NextFrame	=	queued_frame;
//	draw_info->AnimTween=0;
//	draw_info->CurrentAnim	=	anim;

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x2,&lock_y2,&lock_z2);
	//
	// difference in lock co-ord is ammount to move by to maintain same position for limb
	// 

	temp_pos.X=((+lock_x1-lock_x2)<<8)+p_person->WorldPos.X;
	temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
	temp_pos.Z=((+lock_z1-lock_z2)<<8)+p_person->WorldPos.Z;

	//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
	//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);

	//LogText(" locked anim change dx dy dz %d %d %d \n",lock_x1-lock_x2,lock_y1-lock_y2,lock_z1-lock_z2);

	move_thing_on_map(p_person,&temp_pos);
}

void	locked_anim_change_end_type(Thing *p_person,UWORD locked_object,UWORD anim,SLONG type)
{
	SLONG	lock_x1,lock_y1,lock_z1;
	SLONG	lock_x2,lock_y2,lock_z2;
	DrawTween	*draw_info;
	GameCoord	temp_pos;
	
	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x1,&lock_y1,&lock_z1);

	draw_info=p_person->Draw.Tweened;
	draw_info->CurrentFrame	=	game_chunk[type].AnimList[anim];//global_anim_array[type][anim];
	draw_info->QueuedFrame	=	0;
	draw_info->NextFrame	=	draw_info->CurrentFrame->NextFrame;

	while(draw_info->NextFrame->NextFrame)
	{
		draw_info->CurrentFrame = draw_info->NextFrame;
		draw_info->NextFrame	= draw_info->NextFrame->NextFrame;


	}
	draw_info->AnimTween=255;
	draw_info->CurrentAnim	=	anim;

	calc_sub_objects_position(p_person,draw_info->AnimTween,locked_object,&lock_x2,&lock_y2,&lock_z2);
	//
	// difference in lock co-ord is ammount to move by to maintain same position for limb
	// 

	temp_pos.X=((+lock_x1-lock_x2)<<8)+p_person->WorldPos.X;
	temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
	temp_pos.Z=((+lock_z1-lock_z2)<<8)+p_person->WorldPos.Z;

	//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
	//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);

	//LogText(" locked anim change dx dy dz %d %d %d \n",lock_x1-lock_x2,lock_y1-lock_y2,lock_z1-lock_z2);

	move_thing_on_map(p_person,&temp_pos);
}

void	locked_anim_change_end(Thing *p_person,UWORD locked_object,UWORD anim)
{
	SLONG	lock_x1,lock_y1,lock_z1;
	SLONG	lock_x2,lock_y2,lock_z2;
	DrawTween	*draw_info;
	GameCoord	temp_pos;
	
	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,locked_object,&lock_x1,&lock_y1,&lock_z1);

	draw_info=p_person->Draw.Tweened;
	draw_info->CurrentFrame	=	global_anim_array[p_person->Genus.Person->AnimType][anim];
	draw_info->QueuedFrame	=	0;
	draw_info->NextFrame	=	draw_info->CurrentFrame->NextFrame;

	while(draw_info->NextFrame->NextFrame)
	{
		draw_info->CurrentFrame = draw_info->NextFrame;
		draw_info->NextFrame	= draw_info->NextFrame->NextFrame;


	}
	draw_info->AnimTween=255;
	draw_info->CurrentAnim	=	anim;

	calc_sub_objects_position(p_person,draw_info->AnimTween,locked_object,&lock_x2,&lock_y2,&lock_z2);
	//
	// difference in lock co-ord is ammount to move by to maintain same position for limb
	// 

	temp_pos.X=((+lock_x1-lock_x2)<<8)+p_person->WorldPos.X;
	temp_pos.Y=((+lock_y1-lock_y2)<<8)+p_person->WorldPos.Y;
	temp_pos.Z=((+lock_z1-lock_z2)<<8)+p_person->WorldPos.Z;

	//LogText(" old lock %d %d %d \n",lock_x1,lock_y1,lock_z1);
	//LogText(" new lock %d %d %d \n",lock_x2,lock_y2,lock_z2);

	//LogText(" locked anim change dx dy dz %d %d %d \n",lock_x1-lock_x2,lock_y1-lock_y2,lock_z1-lock_z2);

	move_thing_on_map(p_person,&temp_pos);
}

SLONG	steep_cable(SLONG facet)
{
	struct	DFacet	*p_facet;
	SLONG	dx,dy,dz,len,m;
	p_facet=&dfacets[facet];

	dx = abs(p_facet->x[1] - p_facet->x[0] << 8);
	dz = abs(p_facet->z[1] - p_facet->z[0] << 8);

	len=QDIST2(dx,dz);

	dy=p_facet->Y[1]-p_facet->Y[0];

	if(len==0)
		len=1;

	m=(dy<<8)/len;


	return(m);
}

void	face_down_cable(Thing *p_person,SLONG facet)
{
	struct	DFacet	*p_facet;
	SLONG	dx,dy,dz,len,m;
	p_facet=&dfacets[facet];

	dx = (p_facet->x[1] - p_facet->x[0] << 8);
	dy = (p_facet->Y[1] - p_facet->Y[0]);
	dz = (p_facet->z[1] - p_facet->z[0] << 8);

	if(dy>0)
	{
		p_person->Draw.Tweened->Angle=calc_angle(dx,dz);
	}
	else
	{
		p_person->Draw.Tweened->Angle=calc_angle(-dx,-dz);

	}
}

SLONG	find_best_cable_angle(Thing *p_person,SLONG facet)
{
	struct	DFacet	*p_facet;
	SLONG	dx,dy,dz,len,m;
	SLONG	dangle,cable_angle;

	p_facet=&dfacets[facet];

	dx = (p_facet->x[1] - p_facet->x[0]) << 8;
	dz = (p_facet->z[1] - p_facet->z[0]) << 8;

	cable_angle=calc_angle(dx,dz);

	dangle=p_person->Draw.Tweened->Angle-cable_angle;

	if(abs(dangle)>1024)
		return((cable_angle+1024)&2047);
	else
		return(cable_angle);
						  
	
}

//
// see if persons hands are near to grabbing a ledge 
//
SLONG	grab_ledge(Thing *p_person)
{
	SLONG	x,y,z;
	SLONG	grab_x,grab_y,grab_z,type;
	SLONG	grab_angle;
	SLONG	face;

	SLONG ignore_building;

	if(p_person->Genus.Person->InsideIndex)
		return(0);

	//
	// While we're grabbing a ledge- try grabbing a balloon!
	// 

#if !defined(PSX) && !defined(TARGET_DC)
	BALLOON_find_grab(THING_NUMBER(p_person));
#endif
	//
	// Dont grab a ledge whilst falling backwards...
	//
/*
	if (p_person->SubState == SUB_STATE_DROP_DOWN && p_person->Velocity < 0)
	{
		return 0;
	}
*/
	if (p_person->Flags & FLAGS_IN_BUILDING)
	{
		ignore_building = INDOORS_DBUILDING;
	}
	else
	{
		ignore_building = 0;
	}

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_HAND,&x,&y,&z);

	{
		SLONG	rx,ry,rz;

		calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_RIGHT_HAND,&rx,&ry,&rz);

		x+=rx;
		y+=ry;
		z+=rz;

		x>>=1;
		y>>=1;
		z>>=1;

	}
	x+=p_person->WorldPos.X>>8;
	y+=p_person->WorldPos.Y>>8;
	z+=p_person->WorldPos.Z>>8;

	//
	// We grab the sewer floor in the sewers.
	//
#if !defined(PSX) && !defined(TARGET_DC)
	if (p_person->Flags & FLAGS_IN_SEWERS)
	{
		face = find_grab_face_in_sewers(
					x,y,z,
					50,40,
					p_person->Draw.Tweened->Angle,
				   &grab_x,
				   &grab_y,
				   &grab_z,
				   &grab_angle);
	}
	else
#endif
	{
		SLONG	radius=80;
		if(p_person->State==STATE_CLIMBING)
			radius=100;
		face = find_grab_face(
					x,y,z,
					radius,40,
					p_person->Draw.Tweened->Angle,
				   &grab_x,
				   &grab_y,
				   &grab_z,
				   &grab_angle,
				    ignore_building,
					FALSE,&type,
					p_person);
	}

	
	if(face)
	{
		//
		// Leave fight mode when you grab something.
		//

		p_person->Genus.Person->Mode = PERSON_MODE_RUN;

		SLONG		new_x,new_y,new_z, old_substate;
		GameCoord	temp_pos;
		DrawTween	*draw_info;

		if(type==2)
		{
			if(p_person->SubState!=SUB_STATE_RUNNING_JUMP_FLY && p_person->SubState!=SUB_STATE_DROP_DOWN)
				return(0);
			if(p_person->DY>0)
				return(0);

			//grab_ladder
/*
			//
			// move person so new hand position is at grab_x,grab_y,grab_z
			//

			calc_sub_objects_position(p_person,0,SUB_OBJECT_LEFT_HAND,&new_x,&new_y,&new_z);

			new_x+=p_person->WorldPos.X>>8;
			new_y+=p_person->WorldPos.Y>>8;
			new_z+=p_person->WorldPos.Z>>8;

			//LogText(" left hand pos for grab face after anim change %d %d %d \n",new_x,new_y,new_z);

			//LogText(" found a ledge offset %d %d %d to it \n",grab_x-new_x,grab_y-new_y,grab_z-new_z);

*/
			temp_pos.X=grab_x<<8;
			temp_pos.Y=p_person->WorldPos.Y;
			temp_pos.Z=grab_z<<8;
			
			move_thing_on_map(p_person,&temp_pos);
			// grabbed a ladder
			p_person->Genus.Person->OnFacet=face;
			p_person->Draw.Tweened->Angle = grab_angle;
			set_generic_person_state_function(p_person,STATE_CLIMB_LADDER);

			set_person_on_ladder(p_person);
			return(1);

		}

		draw_info=p_person->Draw.Tweened;

		draw_info->AnimTween=0;
		p_person->Genus.Person->Action = ACTION_GRABBING_LEDGE;
		set_generic_person_state_function(p_person,STATE_DANGLING);
		old_substate=p_person->SubState;
		p_person->SubState=SUB_STATE_GRAB_TO_DANGLE;
//		p_person->Draw.Tweened->Angle = grab_angle;

		if(type==1)
		{
			SLONG	grad;
			//
			// grabbed a cable
			//
			p_person->Genus.Person->Flags|=FLAG_PERSON_ON_CABLE;
			set_anim(p_person,ANIM_DEATH_SLIDE);
			
			MinorEffortSound(p_person);

			grad=steep_cable(face);
			if(abs(grad)>10)
			{
				p_person->SubState=SUB_STATE_DEATH_SLIDE;
				p_person->Velocity=0;
				grad>>=6;
				if(grad>16)
					grad=16;
				if(grad<2)
					grad=2;

				p_person->DeltaVelocity=abs(grad);
				face_down_cable(p_person,face);
				p_person->Genus.Person->Action = ACTION_DEATH_SLIDE;
				p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
			}
		}
		else
		{
			switch (old_substate) {
			case SUB_STATE_STANDING_JUMP:
				MinorEffortSound(p_person);
				TRACE("minoreffort(stand jump grab) %d\n",p_person->Draw.Tweened->CurrentAnim);
				break;
			case SUB_STATE_RUNNING_JUMP:
				MinorEffortSound(p_person);
				TRACE("minoreffort(running jump grab) %d\n",p_person->Draw.Tweened->CurrentAnim);
				break;
			case SUB_STATE_DROP_DOWN:
				// just in case...
				StopScreamFallSound(p_person);
				// ideally check speed... 
				if (p_person->Genus.Person->PersonType==PERSON_ROPER)
					MFX_play_thing(THING_NUMBER(p_person),S_ROPER_HIT_END-1,0,p_person);
				else
				    MinorEffortSound(p_person);
				TRACE("minoreffort(falling grab) %d\n",p_person->Draw.Tweened->CurrentAnim);
				break;
			case SUB_STATE_RUNNING_JUMP_FLY:
				TRACE("oof(run jump fly grab) %d\n",p_person->Draw.Tweened->CurrentAnim);
				switch(p_person->Genus.Person->PersonType) {
				case PERSON_DARCI:
					MFX_play_thing(THING_NUMBER(p_person),S_DARCI_HIT_END,0,p_person);
					break;
				case PERSON_ROPER:
					MFX_play_thing(THING_NUMBER(p_person),S_ROPER_HIT_END-1,0,p_person);
					break;
				}
				break;
			default:
				TRACE("unknown grab substate: %d\n",old_substate);

			}

			set_anim(p_person,ANIM_FLY_GRABBING_LEDGE);
			p_person->Draw.Tweened->Angle = grab_angle;
		}

		//
		// We have grabbed a face, so change to grab anim
		//


		//
		// move person so new hand position is at grab_x,grab_y,grab_z
		//

		{
			SLONG lx;
			SLONG ly;
			SLONG lz;

			calc_sub_objects_position(p_person,0,SUB_OBJECT_LEFT_HAND,&lx,&ly,&lz);

			SLONG rx;
			SLONG ry;
			SLONG rz;

			calc_sub_objects_position(p_person,0,SUB_OBJECT_RIGHT_HAND,&rx,&ry,&rz);

			new_x = lx + rx >> 1;
			new_y = ly + ry >> 1;
			new_z = lz + rz >> 1;
		}

		new_x+=p_person->WorldPos.X>>8;
		new_y+=p_person->WorldPos.Y>>8;
		new_z+=p_person->WorldPos.Z>>8;

		MSG_add(" left hand y %d grab_y %d tween %d\n",new_y,grab_y,draw_info->AnimTween);

		//LogText(" left hand pos for grab face after anim change %d %d %d \n",new_x,new_y,new_z);

		//LogText(" found a ledge offset %d %d %d to it \n",grab_x-new_x,grab_y-new_y,grab_z-new_z);

		temp_pos.X=((grab_x-new_x)<<8)+p_person->WorldPos.X;
		temp_pos.Y=((grab_y-new_y)<<8)+p_person->WorldPos.Y;
		temp_pos.Z=((grab_z-new_z)<<8)+p_person->WorldPos.Z;
		
		if (face > 0 && (prim_faces4[face].FaceFlags & (FACE_FLAG_WMOVE|FACE_FLAG_PRIM)))
		{
			//
			// For some mad reason- if you pull yourself up on wmove faces at
			// the top of the anim you are not properly on the face...
			//

			temp_pos.X += -SIN(p_person->Draw.Tweened->Angle) >> 5;
			temp_pos.Z += -COS(p_person->Draw.Tweened->Angle) >> 5;
		}

		move_thing_on_map(p_person,&temp_pos);

		//
		// Set the person's angle to line up with the grabbed edge.
		//


//		calc_sub_objects_position(p_person,0,SUB_OBJECT_LEFT_HAND,&new_x,&new_y,&new_z);
//		new_x+=p_person->WorldPos.X;
//		new_y+=p_person->WorldPos.Y;
//		new_z+=p_person->WorldPos.Z;
		//LogText(" left hand pos for grab face after anim change and adjust %d %d %d \n",new_x,new_y,new_z);

		// this is perfect

		//
		// lock hand in place for drawing and animating
		//

		draw_info->Locked=SUB_OBJECT_LEFT_HAND;
//		draw_info->AnimTween	=	C0;
		if(face==GRAB_FLOOR || face == GRAB_SEWERS)
		{
			MSG_add("grab floor or sewers\n");
			p_person->OnFace=0;
		}
		else
		{
			if(type==1)
			{
				p_person->Genus.Person->OnFacet=face;
				p_person->OnFace=0;
			}
			else
			{
				p_person->OnFace=face;
			}
		}

		//
		// Automatically pull yourself up if you're getting onto a moving walkable face.
		//

		if (face > 0 && (prim_faces4[face].FaceFlags & (FACE_FLAG_WMOVE|FACE_FLAG_PRIM)))
		{
			//
			// Don't automatically pullup on crates.
			//

			if (prim_faces4[face].FaceFlags & FACE_FLAG_PRIM)
			{
				SLONG ob_index = -prim_faces4[face].ThingIndex;

				ASSERT(WITHIN(ob_index, 1, OB_ob_upto - 1));

				if (OB_ob[ob_index].prim == 129)
				{
					//
					// Don't pull up
					//

					goto dont_pull_up;
				}
			}

			//
			// Automatically do a pullup.
			//

			set_person_pulling_up(p_person);

		  dont_pull_up:;
		}

		return(1);
	}

	return(0);
}

void	set_tween_for_dy(Thing *p_person,SLONG dy)
{
	MSG_add(" tween dy %d \n",dy);

	
	dy-=28;

	if(dy>128)
		dy=0;
	else
	if(dy<0)
		dy=255;
	else
		dy=(128-dy)<<1;
		
	


//	MSG_add("DY %d dy %d pre_shift %d \n",p_person->DY>>8,dy,y-floor_y);
	p_person->Draw.Tweened->AnimTween=dy;

}

void	set_tween_for_height(Thing *p_person)
{
	SLONG	x,y,z;
	SLONG	floor_y,dy;

	if(p_person->DY>0)
		return;

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,3,&x,&y,&z);
	y+=(p_person->WorldPos.Y)>>8;

#if !defined(PSX) && !defined(TARGET_DC)
	if (p_person->Flags & FLAGS_IN_SEWERS)
	{
		floor_y	= NS_calc_height_at(x,z);
	}
	else
#endif
	{
		floor_y = PAP_calc_height_at_thing(p_person,x,z);
	}

	dy=y-floor_y;
	set_tween_for_dy(p_person,256+(p_person->DY>>5));

}

SLONG	over_nogo(Thing *p_person)
{
	SLONG	mx,mz;

	if(p_person->Genus.Person->Ware)
		return(0);
	mx=p_person->WorldPos.X>>16;
	mz=p_person->WorldPos.Z>>16;
	if(mx>=0&&mx<128 && mz>=0 && mz<=128)
	{
	 	if (PAP_2HI((mx)&127, (mz)&127).Flags & PAP_FLAG_NOGO)
		{
			return	1;
		}

	}
	return(0);
}


void	fn_person_jumping(Thing *p_person)
{
	SLONG		end		=	0,
				grab	=	0,
				wave_id1,
				wave_id2;
	GameCoord	new_position;
	SLONG		old_frame,frame;
//	LogText(" person jumping  substate %d\n",p_person->SubState);
//	MSG_add(" JUMPING ");	
	//return;

	SlideSoundCheck(p_person);

	switch(p_person->SubState)
     	{
		case	SUB_STATE_STANDING_JUMP_FORWARDS:
				change_velocity_to(p_person,32); //was 25

				if(over_nogo(p_person))
					p_person->Velocity=0;

				p_person->DeltaVelocity=0;
				p_person->DY=0;
				end=projectile_move_thing(p_person,1);
				if(end==100)
				{
					//
					// dead
					//

					return;
				}


				if(!end)
				{
					end=person_normal_animate(p_person);
					if(end)
					{
						p_person->DeltaVelocity=0;
						p_person->DY=10<<8;
						p_person->SubState=SUB_STATE_RUNNING_JUMP_FLY;
						p_person->Genus.Person->Action=ACTION_RUN_JUMP;
						if(person_has_gun_out(p_person))
						{
							set_locked_anim(p_person,ANIM_MID_AIR_TWEEN_LEFT_AK,0);
						}
						else
						{
							set_locked_anim(p_person,ANIM_MID_AIR_TWEEN_LEFT,0);
						}
						p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_MID_AIR_TWEEN_LEFT];

					}
				}

				grab=grab_ledge(p_person);
/*
				if((end==1)&&!grab)
				{
					set_person_idle(p_person);
//					plant_feet(p_person);
				}
*/

			break;
		case	SUB_STATE_STANDING_JUMP_BACKWARDS:
				change_velocity_to(p_person,-32); //was 25
				if(over_nogo(p_person))
					p_person->Velocity=0;

				p_person->DeltaVelocity=0;
				p_person->DY=0;
				if(p_person->Draw.Tweened->FrameIndex>2)
				{
					if(p_person->Draw.Tweened->FrameIndex>4)
						end=projectile_move_thing(p_person,3);
					else
						end=projectile_move_thing(p_person,1);
					if(end==100)
					{
						//
						// dead
						//

						return;
					}
					if(end==1)
					{
						set_person_idle(p_person);
	//					plant_feet(p_person);
						return;
					}
					else
					if(end==2)
					{
						set_person_drop_down(p_person,0);
						p_person->Velocity = 0;
						return;
					}
				}
				else
				{
					end=0;
				}

				if(!end)
					end=person_normal_animate(p_person);

//				grab=grab_ledge(p_person);

				if(end==1)
				{
					set_person_drop_down(p_person,0);
					p_person->DY=-20<<8;
				}


			break;

		case	SUB_STATE_STANDING_JUMP:
				MSG_add(" standing jump ");
				end=person_normal_animate(p_person);
				grab=grab_ledge(p_person);
				SLONG pinnacle_frame;
		
				switch(p_person->Genus.Person->AnimType) 
				{
				case ANIM_TYPE_ROPER:
					pinnacle_frame=5;
					break;
				default:	// darci, others
					pinnacle_frame=3;
				}
//				if(p_person->Draw.Tweened->FrameIndex>pinnacle_frame && !grab)
				if(end && !grab)
				{
					set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL|PERSON_DROP_DOWN_QUEUED);
				}
				else
				{
					if(end==1)
					{
						MSG_add("end standing jump anim");
					}
					if(grab) 
					{
						MSG_add("grab standing jump");

					}
					
					if((end==1)&&!grab)
					{
						set_person_idle(p_person);
						person_splash(p_person, -1);
					}

					if(p_person->Draw.Tweened->FrameIndex==4)
					{
						wave_id1	=	footstep_wave(p_person);
						wave_id2	=	footstep_wave(p_person);
//						play_quick_wave(p_person,wave_id1,WAVE_PLAY_INTERUPT);
//						play_quick_wave(p_person,wave_id2,WAVE_PLAY_OVERLAP);
						MFX_play_thing(THING_NUMBER(p_person),wave_id1,MFX_REPLACE,p_person);
						MFX_play_thing(THING_NUMBER(p_person),wave_id2,MFX_REPLACE,p_person);
					}
				}

			break;
		case	SUB_STATE_RUNNING_JUMP:
				if(p_person->Genus.Person->PlayerID)
				{
					SLONG	reqd_vel=(50*15)/20;
					if(p_person->Velocity<(50*15)/20)
						p_person->Velocity=(50*15)/20;
					else
						if(p_person->Velocity>(60*15)/20)
							p_person->Velocity=(60*15)/20;


					//set_thing_velocity(p_person,50); //32); //was 25
				}
				if(over_nogo(p_person))
					p_person->Velocity=0;

				p_person->DeltaVelocity=0;
				end=projectile_move_thing(p_person,1);
				if(end==100)
				{
					//
					// dead
					//

					return;
				}

				old_frame=p_person->Draw.Tweened->FrameIndex;

				if(old_frame<3)
				{
					if(check_on_slippy_slope(p_person))
						return;
				}
				if(!end)
					end=person_normal_animate(p_person);
				frame=p_person->Draw.Tweened->FrameIndex;

				p_person->DY=0;

				grab=grab_ledge(p_person);
				if(grab)
				{
					MSG_add(" running jump grabbed 1\n");
				}
				if(!grab)
				{
					if(end==1) //phase 1 anim end
					{
							p_person->Draw.Tweened->Locked=0;//

						p_person->Flags|=FLAGS_PROJECTILE_MOVEMENT;
						p_person->DeltaVelocity=0;
						p_person->DY=10<<8;
						p_person->SubState=SUB_STATE_RUNNING_JUMP_FLY;
						p_person->Genus.Person->Action=ACTION_RUN_JUMP;
//						PlaySample(THING_NUMBER(p_person),SAMPLE_WOMAN_JUMP1,SAMPLE_VOL_MAX,SAMPLE_PAN_CENTER,SAMPLE_FREQ_ORIG,0);
						if(person_has_gun_out(p_person))
						{
							p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_MID_AIR_TWEEN_LEFT_AK];
						}
						else
						{
							p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_MID_AIR_TWEEN_LEFT];
						}

/*
						switch(p_person->Draw.Tweened->CurrentAnim)
						{
							case	ANIM_RUN_JUMP_LEFT:
								p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_MID_AIR_TWEEN_LEFT];
								break;

							case	ANIM_RUN_JUMP_RIGHT:
								p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_MID_AIR_TWEEN_RIGHT];
								break;

							default:
								p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_MID_AIR_LAND];
								break;


						}
*/
						p_person->Draw.Tweened->AnimTween=0;

					}
				}
				break;
		case	SUB_STATE_RUNNING_JUMP_FLY_STOP:
				change_velocity_to(p_person,0); //25
				goto	jump_fly;

		case	SUB_STATE_RUNNING_JUMP_FLY:
				if(continue_moveing(p_person)&&!over_nogo(p_person))
				{
				//	change_velocity_to(p_person,50); //32); //25
				}
				else
				{
					set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL|PERSON_DROP_DOWN_KEEP_DY|PERSON_DROP_DOWN_QUEUED);
					p_person->Velocity>>=1;
					return;
				}
					//change_velocity_to(p_person,0); //25

				//	set_thing_velocity(p_person,32); //25
jump_fly:;

				if (p_person->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_START ||
					p_person->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_FORWARDS)
				{
					end = person_normal_animate(p_person);

					if (end == 1)
					{
						if (p_person->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_START)
						{
							locked_anim_change(p_person, SUB_OBJECT_PELVIS, ANIM_PLUNGE_FORWARDS);
						}
					}
				}
				else
				{
					set_tween_for_height(p_person);
				}

				end = projectile_move_thing(p_person,3);

				if(end==100)
				{
					//
					// dead
					//

					return;
				}
//				if(!end)
//					person_normal_animate(p_person);

				grab=grab_ledge(p_person);
				if(grab)
				{
					MSG_add("grab face in running jump fly \n");
					if(end==2)
					{
						MSG_add(" hit fence, but grabbed something as well");
					}

//					p_person->SubState=0;
				}
				else
				if(end==1)
				{
					p_person->Genus.Person->Action=ACTION_LANDING;
					person_splash(p_person, -1);

					//
					// If end == 1, then we are about to land on the floor or on a
					// face... it depends on whether the p_person's OnFace field is set or not.
					//

					DrawTween	*draw_info;

					draw_info = p_person->Draw.Tweened;
					if(continue_moveing(p_person))
					{
						//
						// player wishes to land and continue running
						//
						if(person_has_gun_out(p_person))
						{
							set_locked_anim(p_person,ANIM_LAND_RIGHT_AK,SUB_OBJECT_RIGHT_FOOT);
						}
						else
						{
							set_locked_anim(p_person,ANIM_LAND_RIGHT,SUB_OBJECT_RIGHT_FOOT);
						}
/*
						switch(p_person->Draw.Tweened->CurrentAnim)
						{
							case	ANIM_RUN_JUMP_LEFT:
								set_locked_anim(p_person,ANIM_LAND_RIGHT,SUB_OBJECT_RIGHT_FOOT);
								break;

							case	ANIM_RUN_JUMP_RIGHT:
								set_locked_anim(p_person,ANIM_LAND_LEFT,SUB_OBJECT_LEFT_FOOT);
								break;
						}
*/
//						set_locked_anim(p_person,ANIM_LAND_RUN,SUB_OBJECT_LEFT_FOOT);
//						plant_feet(p_person);

						if(p_person->OnFace==0)
						{
/*
							if (p_person->Flags & FLAGS_IN_SEWERS)
							{
								p_person->WorldPos.Y=NS_calc_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;
							}
							else
							{
								p_person->WorldPos.Y=PAP_calc_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;
							}
*/
//already set
//							p_person->WorldPos.Y=PAP_calc_height_at_thing(p_person,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;

						}
						else
						{

#ifdef	NOT_SETUP_IN_PROJECTILE
							some old tat
							SLONG	new_y,face;
/*
							calc_sub_objects_position(
								p_thing,
								255, //p_thing->Draw.Tweened->AnimTween,
								pelvis?0:SUB_OBJECT_LEFT_FOOT,
							   &fx,
							   &fy,
							   &fz);
*/


							MSG_add("wp y %d \n",p_person->WorldPos.Y>>8);
							face = find_face_for_this_pos(p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,&new_y,0,0);

							if(face==0)
							{
								ASSERT(0);
								face = find_face_for_this_pos(p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,&new_y,0,0);

							}
							MSG_add(" hit face while jumping, try to find face= %d newy %d \n",face,new_y);
							//ASSERT(face!=GRAB_FACE);
/*
							if(new_y==1000000)
							{
								ASSERT(0);
								p_person->OnFace=0;
							}
							else
*/
							{
// 								ASSERT(face);//triggered
								p_person->OnFace=face;
								p_person->WorldPos.Y=new_y<<8;
							}
#endif


						}

						p_person->SubState		=	SUB_STATE_RUNNING_JUMP_LAND_FAST;
						p_person->Genus.Person->Action = ACTION_RUN;

						LogText("cm1 calc landing y again %d \n",p_person->WorldPos.Y);
					}
					else
					{
						//
						// player has stopped pressing forward and wants to just land and be idle
						//
						set_locked_anim(p_person,ANIM_LAND_STAND,0);
						
						if(p_person->OnFace==0)
						{
							/*
							if (p_person->Flags & FLAGS_IN_SEWERS)
							{
								p_person->WorldPos.Y=NS_calc_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;
							}
							else
							{
								p_person->WorldPos.Y=PAP_calc_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;
							}
							*/
							// already set
							//p_person->WorldPos.Y=PAP_calc_height_at_thing(p_person,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8; //md021199
						}
						else
						{

							SLONG	new_y,face;
							face = find_face_for_this_pos(p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,&new_y,0,0);
//							ASSERT(face!=GRAB_FACE)
/*
							if(new_y==1000000)
							{
								ASSERT(0);
								p_person->OnFace=0;
							}
							else
*/
							{
								ASSERT(face);
								p_person->OnFace=face;
								//alreadyset
								//p_person->WorldPos.Y=new_y<<8; //md021199
							}

						}

//						p_person->SubState		=	SUB_STATE_RUNNING_JUMP_LAND;

						set_person_idle(p_person);
//						plant_feet(p_person);
						LogText("cm2 calc landing y again %d \n",p_person->WorldPos.Y);
					}

					draw_info->AnimTween	=	0;
//					PlaySample(THING_NUMBER(p_person),SAMPLE_WOMAN_LAND1,SAMPLE_VOL_MAX,SAMPLE_PAN_CENTER,SAMPLE_FREQ_ORIG+(GAME_TURN&0xfff),0);
				}
				else
				if(end==2)
				{
					MSG_add(" hit fence, so hold on");
				}
				else
				{

					//if(p_person->Draw.Tweened->AnimTween<64)
					if(p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_KICK)
					{
						p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_KICK;
						set_anim(p_person,ANIM_FLYKICK_START);
						p_person->SubState=SUB_STATE_FLYING_KICK;

					}

				}
				break;
		case	SUB_STATE_FLYING_KICK:
				end=person_normal_animate(p_person);
				if(end)
				{
					queue_anim(p_person,ANIM_FLYKICK_FALL);
					p_person->SubState=SUB_STATE_FLYING_KICK_FALL;
				}
				//
				// fall through into next case
				//
		case	SUB_STATE_FLYING_KICK_FALL:
				end=person_normal_animate(p_person);
				if(over_nogo(p_person))
					p_person->Velocity=0;
				end=projectile_move_thing(p_person,3);
				if(end==100)
				{
					//
					// dead
					//

					return;
				}
				else
				if(end==1)
				{
					p_person->Genus.Person->Action=ACTION_LANDING;
					person_splash(p_person, -1);

					//
					// If end == 1, then we are about to land on the floor or on a
					// face... it depends on whether the p_person's OnFace field is set or not.
					//

					DrawTween	*draw_info;

					draw_info = p_person->Draw.Tweened;
					{
						//
						// player wishes to land and continue running
						//
						set_locked_anim(p_person,ANIM_FLYKICK_LAND,SUB_OBJECT_RIGHT_FOOT);

						if(p_person->OnFace==0)
						{
							p_person->WorldPos.Y=PAP_calc_height_at_thing(p_person,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8)<<8;

						}
						else
						{



						}

						p_person->SubState		=	SUB_STATE_RUNNING_JUMP_LAND_FAST;
						LogText("cm1 calc landing y again %d \n",p_person->WorldPos.Y);
					}

					draw_info->AnimTween	=	0;
				}
				break;


			break;
		case	SUB_STATE_RUNNING_JUMP_LAND:
				MSG_add("RJ        LANDING ");	
				change_velocity_to(p_person,yomp_speed); //32); //25
				if(over_nogo(p_person))
					p_person->Velocity=0;
				end=projectile_move_thing(p_person,3);
				if(end==100)
				{
					//
					// dead
					//

					return;
				}
//				person_normal_move(p_person);
				if(end)
				{
					set_person_idle(p_person);
//					plant_feet(p_person);
//					calc_camera_pos(p_person);
				}
			break;
		case	SUB_STATE_RUNNING_JUMP_LAND_FAST:

//				while(!Keys[KB_E]&&SHELL_ACTIVE);
				MSG_add("RJ        LANDING FAST");	
//				change_velocity_to(p_person,5);
//				person_normal_move(p_person);
				change_velocity_to(p_person,yomp_speed); //32); //25
				if(over_nogo(p_person))
					p_person->Velocity=0;
				person_normal_move(p_person);

				if(p_person->SubState!=SUB_STATE_RUNNING_JUMP_LAND_FAST)
				{
					p_person->DY=-4001;
					return;

				}
				end=person_normal_animate(p_person);

//				if(p_person->Draw.Tweened->NextFrame)
//					if(p_person->Draw.Tweened->NextFrame->NextFrame==0)
				if(end||!continue_moveing(p_person))
					{
						SLONG	anim;

						anim=get_yomp_anim(p_person);

						set_anim(p_person,anim); //was locked	   //Miked aug 2000
						set_generic_person_state_function(p_person,STATE_MOVEING);
						p_person->SubState = SUB_STATE_RUNNING;
						p_person->Genus.Person->Action = ACTION_RUN;
						p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

						/*
						switch(p_person->Draw.Tweened->CurrentAnim)
						{
							case	ANIM_LAND_LEFT:
								set_person_running(p_person);
								break;
							case	ANIM_LAND_RIGHT:
								set_person_running_frame(p_person,3);
								break;


						}
						*/

						wave_id1	=	footstep_wave(p_person);
						wave_id2	=	footstep_wave(p_person);
//						play_quick_wave(p_person,wave_id1,WAVE_PLAY_INTERUPT);
//						play_quick_wave(p_person,wave_id2,WAVE_PLAY_OVERLAP);
						MFX_play_thing(THING_NUMBER(p_person),wave_id1,MFX_REPLACE,p_person);
						MFX_play_thing(THING_NUMBER(p_person),wave_id2,MFX_REPLACE,p_person);

//						set_thing_velocity(p_person,48); //25
					}
						//p_person->Draw.Tweened->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_RUN];
/*
				if(end)
				{
					set_person_running(p_person);
					p_person->Draw.Tweened->QueuedFrame	=	0; //global_anim_array[p_person->Genus.Person->AnimType][ANIM_RUN];
					set_thing_velocity(p_person,25);
//					calc_camera_pos(p_person);
				}
				*/
			break;
		default:
			MSG_add("JUMPING unknow substate %d \n",p_person->SubState);
			break;

	}
}

void	position_person_at_ladder_top(Thing *p_person,SLONG limb)
{
	SLONG x1,y1,z1;
	SLONG	x[2],z[2],y,wall;
	struct	DFacet	*p_facet;
	SLONG	top;

	p_facet=&dfacets[p_person->Genus.Person->OnFacet];

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,limb,&x1,&y1,&z1);

	y1+=p_person->WorldPos.Y>>8;

	top = p_facet->Y[0] + p_facet->Height * 64;
	y1-=top-20;

	//
	// y1 is difference between position of Right Hand on ladder, and top of ladder.
	//

//	y1=0; 


	p_person->WorldPos.Y-=y1<<8;
}

void	position_person_at_ladder_bot(Thing *p_person,SLONG limb)
{
	SLONG x1,y1,z1;
	SLONG	x[2],z[2],y,wall;
	struct	DFacet	*p_facet;
	SLONG	bot;

	p_facet=&dfacets[p_person->Genus.Person->OnFacet];

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,limb,&x1,&y1,&z1);

	y1+=p_person->WorldPos.Y>>8;

	bot = p_facet->Y[0]; // + p_facet->Height * 64;
	y1-=bot;

	//
	// y1 is difference between position of Right Hand on ladder, and top of ladder.
	//

//	y1=0; 


	p_person->WorldPos.Y=bot<<8; //y1<<8;
}

//
// Returns a bit field to indicate where on the ladder the sub-part is.
//

#define PERSON_LIMB_ON_LADDER	(1 << 0)
#define PERSON_LIMB_TOP_BLOCK	(1 << 1)
#define PERSON_LIMB_BOT_BLOCK	(1 << 2)
#define PERSON_LIMB_OFF_TOP		(1 << 3)
#define PERSON_LIMB_OFF_BOT		(1 << 4)

ULONG check_limb_pos_on_ladder(Thing *p_person,SLONG sub_part, SLONG i_am_going_down)
{
	SLONG   x1,y1,z1;
	UWORD	facet;
	SLONG	x[2],z[2],y,wall;
	SLONG   top;
	SLONG   bot;
	ULONG	ans;

	struct	DFacet	*p_facet;

	facet=p_person->Genus.Person->OnFacet;

	p_facet=&dfacets[facet];

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,sub_part,&x1,&y1,&z1);

	x1 += p_person->WorldPos.X>>8;
	y1 += p_person->WorldPos.Y>>8;
	z1 += p_person->WorldPos.Z>>8;

	x[0] = p_facet->x[0] << 8;
	z[0] = p_facet->z[0] << 8;

	x[1] = p_facet->x[1] << 8;
	z[1] = p_facet->z[1] << 8;

	bot = p_facet->Y[0];

	top = bot + p_facet->Height * 64;

	ans = 0;

	if (WITHIN(y1, bot + ((i_am_going_down) ? 20 : 0), top - 20))
	{
		ans |= PERSON_LIMB_ON_LADDER;

		if (y1 > top - 256) {ans |= PERSON_LIMB_TOP_BLOCK;}
		if (y1 < bot + 256) {ans |= PERSON_LIMB_BOT_BLOCK;}
	}
	else
	{
		if (y1 < bot + 20) {ans |= PERSON_LIMB_OFF_BOT;}
		if (y1 > top + 20) {ans |= PERSON_LIMB_OFF_TOP;}
	}

	return ans;
}



SLONG	check_limb_pos_on_fence(Thing *p_person,SLONG sub_part)
{
	SLONG	x1,y1,z1;
	SLONG	col;
	SLONG	top,bottom;
	SLONG	x,z;

	//
	// The top and bottom of the fence.
	//

	x   = p_person->WorldPos.X >> 8;
	z   = p_person->WorldPos.Z >> 8;
	col = p_person->Genus.Person->OnFacet;

	top    = get_fence_top   (x,z,col);
	bottom = get_fence_bottom(x,z,col);

	//
	// The position of our limb.
	//

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,sub_part,&x1,&y1,&z1);

	y1 += p_person->WorldPos.Y>>8;

	//
	// The limb counts as being on the fence only if it is not too near the bottom.
	//

	if (WITHIN(y1, bottom + 20, top))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

SLONG	check_limb_pos_on_fence_sideways(Thing *p_person,SLONG sub_part)
{
	SLONG	x1,y1,z1;
	SLONG	col;
	SLONG	along;

	col=p_person->Genus.Person->OnFacet;

SLONG calc_along_vect(SLONG ax,SLONG az,struct DFacet *p_vect);

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,sub_part,&x1,&y1,&z1);

	x1+=p_person->WorldPos.X>>8;
	z1+=p_person->WorldPos.Z>>8;

	along=calc_along_vect(x1,z1,&dfacets[col]);

	if(along<=0 || along>=255)
	{
		return(0);
	}
	else
	{
		return(1);
	}

}


void	fn_person_laddering(Thing	*p_person)
{
	BOOL		play_it	=	FALSE;
	UBYTE		last_frame;
	SLONG		end=0,hit,
				foot_step_wave;
	WaveParams	foot_step;
	ULONG       on_ladder,on_ladder_left,on_ladder_right;

	switch(p_person->SubState)
	{
		case	SUB_STATE_MOUNT_LADDER:
				last_frame	=	p_person->Draw.Tweened->FrameIndex;
				end			=	person_normal_animate(p_person);
				if(p_person->Draw.Tweened->FrameIndex!=last_frame && p_person->Draw.Tweened->FrameIndex==3)
					play_it	=	TRUE;

				if(end==1)
				{
					set_person_on_ladder(p_person);
				}
				break;
		case	SUB_STATE_ON_LADDER:
		case	SUB_STATE_STOPPING:
			break;
		case	SUB_STATE_CLIMB_UP_LADDER:
	
				on_ladder_left =	check_limb_pos_on_ladder(p_person,SUB_OBJECT_LEFT_HAND,  FALSE);
				on_ladder_right=    check_limb_pos_on_ladder(p_person,SUB_OBJECT_RIGHT_HAND, FALSE);
					
				if ((on_ladder_left&on_ladder_right) & PERSON_LIMB_ON_LADDER)
				{
					last_frame	=	p_person->Draw.Tweened->FrameIndex;
					end			=	person_normal_animate(p_person);
					if(p_person->Draw.Tweened->FrameIndex!=last_frame && ((p_person->Draw.Tweened->FrameIndex%3==0) || (p_person->Draw.Tweened->FrameIndex%6==0)))
						play_it	=	TRUE;

					if(end==1)
					{
						if(p_person->Genus.Person->PersonType==PERSON_ROPER)
 							locked_anim_change_height_type(p_person,SUB_OBJECT_LEFT_HAND,COP_ROPER_ANIM_LADDER_LOOP,p_person->Genus.Person->AnimType);
						else
//						if(p_person->Genus.Person->PersonType==PERSON_COP)
						if(p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY||p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||p_person->Genus.Person->PersonType==PERSON_THUG_RED)
							locked_anim_change_height_type(p_person,SUB_OBJECT_LEFT_HAND,COP_ROPER_ANIM_LADDER_LOOP,ANIM_TYPE_ROPER);
						else
 							locked_anim_change_height_type(p_person,SUB_OBJECT_LEFT_HAND,ANIM_ON_LADDER,ANIM_TYPE_DARCI);

						if (p_person->Genus.Person->PlayerID)
						{
							p_person->SubState=SUB_STATE_STOPPING;
						}
					}

					/*

					if (p_person->Flags & FLAGS_IN_SEWERS)
					{
						SLONG   ladder = p_person->Genus.Person->OnFacet;
						DFacet *df;

						ASSERT(WITHIN(ladder, 1, next_dfacet - 1));
						ASSERT(dfacets[ladder].FacetType == STOREY_TYPE_LADDER);

						//
						// If this person is climbing out of a sewer...
						//

						df = &dfacets[ladder];

						if (df->FacetFlags & FACET_FLAG_LADDER_LINK)
						{
							if ((on_ladder_left|on_ladder_right) & PERSON_LIMB_TOP_BLOCK)
							{
								//
								// Make the person leave the sewers.
								//

								p_person->Flags &= ~FLAGS_IN_SEWERS;
							}
						}
					}

					*/
				}
				else
				{
					//
					// Climb off top of ladder
					//
					p_person->SubState=SUB_STATE_CLIMB_OFF_LADDER_TOP;
					if(p_person->Genus.Person->PersonType==PERSON_ROPER)
 						set_anim_of_type(p_person,COP_ROPER_ANIM_LADDER_END_L,ANIM_TYPE_ROPER);
					else
					if(p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY||p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||p_person->Genus.Person->PersonType==PERSON_THUG_RED)
 						set_anim_of_type(p_person,COP_ROPER_ANIM_LADDER_END_L,ANIM_TYPE_ROPER);
					else
						set_anim(p_person,ANIM_OFF_LADDER_TOP);

					if(on_ladder_left)
						position_person_at_ladder_top(p_person,SUB_OBJECT_RIGHT_HAND);
					else
						position_person_at_ladder_top(p_person,SUB_OBJECT_LEFT_HAND);
					p_person->Genus.Person->Action=ACTION_CLIMBING;
				}
/*
				if(--p_person->Genus.Person->Timer1==0)
				{
					p_person->SubState=SUB_STATE_STOPPING;
				}
*/

			break;
		case	SUB_STATE_CLIMB_DOWN_ONTO_LADDER:
				end			=	person_backwards_animate(p_person);
				if(end==1)
				{
					set_person_on_ladder(p_person);
				}
			break;
		case	SUB_STATE_CLIMB_DOWN_LADDER:

				on_ladder_left =check_limb_pos_on_ladder(p_person,SUB_OBJECT_LEFT_FOOT,  TRUE);
				on_ladder_right=check_limb_pos_on_ladder(p_person,SUB_OBJECT_RIGHT_FOOT, TRUE);
					
				if ((on_ladder_left&on_ladder_right) & PERSON_LIMB_ON_LADDER)
				{
					last_frame	=	p_person->Draw.Tweened->FrameIndex;
					end			=	person_backwards_animate(p_person);
					if(p_person->Draw.Tweened->FrameIndex!=last_frame && ((p_person->Draw.Tweened->FrameIndex%3==0) || (p_person->Draw.Tweened->FrameIndex%6==0)))
						play_it	=	TRUE;

					if(end==1)
					{
						MSG_add(" LOCKED CHANGE END \n");
						p_person->Draw.Tweened->AnimTween=0;
						if(p_person->Genus.Person->PersonType==PERSON_ROPER)
 							locked_anim_change_end_type(p_person,SUB_OBJECT_LEFT_HAND,COP_ROPER_ANIM_LADDER_LOOP,p_person->Genus.Person->AnimType);
						else
	//					if(p_person->Genus.Person->PersonType==PERSON_COP)
						if(p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY||p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||p_person->Genus.Person->PersonType==PERSON_THUG_RED)
 							locked_anim_change_end_type(p_person,SUB_OBJECT_LEFT_HAND,COP_ROPER_ANIM_LADDER_LOOP,ANIM_TYPE_ROPER);
						else
 							locked_anim_change_end_type(p_person,SUB_OBJECT_LEFT_HAND,ANIM_ON_LADDER,p_person->Genus.Person->AnimType);

//						locked_anim_change_end(p_person,SUB_OBJECT_LEFT_HAND,ANIM_ON_LADDER);
						p_person->SubState=SUB_STATE_STOPPING;
					}

					/*
					if (p_person->Flags & FLAGS_IN_SEWERS)
					{
						SLONG   ladder = p_person->Genus.Person->OnFacet;
						DFacet *df;

						ASSERT(WITHIN(ladder, 1, next_dfacet - 1));
						ASSERT(dfacets[ladder].FacetType == STOREY_TYPE_LADDER);

						//
						// If this person is climbing into a sewer...
						//

						df = &dfacets[ladder];

						if (df->FacetFlags & FACET_FLAG_LADDER_LINK)
						{
							if (!(on_ladder_left & PERSON_LIMB_TOP_BLOCK))
							{
								//
								// Make the person enter the sewers.
								//

								p_person->Flags |= FLAGS_IN_SEWERS;
							}
						}
					}
					*/
				}
				else
				{
					//
					// climb off bottom of ladder
					//
					p_person->SubState=SUB_STATE_CLIMB_OFF_LADDER_BOT;
					set_anim(p_person,ANIM_OFF_LADDER_BOT);
					position_person_at_ladder_bot(p_person,SUB_OBJECT_LEFT_FOOT);

					p_person->Genus.Person->Action=ACTION_CLIMBING;
				}
/*
				if(--p_person->Genus.Person->Timer1==0)
				{
					p_person->SubState=SUB_STATE_STOPPING;
				}
*/
			break;
		case	SUB_STATE_CLIMB_OFF_LADDER_BOT:
				end=person_normal_animate(p_person);
				if(end==1)
				{
					//p_person->OnFace=0;
					set_person_idle(p_person);
					//set_anim(p_person,ANIM_OFF_LADDER_TOP);
					p_person->Draw.Tweened->CurrentFrame=p_person->Draw.Tweened->QueuedFrame;
					p_person->Draw.Tweened->NextFrame=p_person->Draw.Tweened->QueuedFrame;
					p_person->Draw.Tweened->QueuedFrame=0;
					locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_STAND_READY);
					plant_feet(p_person);
				}

			break;
		case	SUB_STATE_CLIMB_OFF_LADDER_TOP:
				last_frame	=	p_person->Draw.Tweened->FrameIndex;
				end			=	person_normal_animate(p_person);
				if(p_person->Draw.Tweened->FrameIndex!=last_frame && (p_person->Draw.Tweened->FrameIndex==1 || p_person->Draw.Tweened->FrameIndex==3))
					play_it	=	TRUE;

				if(end==1)
				{
					person_bodge_forward(p_person,40);
					locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_STAND_READY);
					plant_feet(p_person);
					if(p_person->State!=STATE_DANGLING)
						set_person_idle(p_person);
				}

			break;
		default:
			MSG_add("LADDERING unknow substate %d \n",p_person->SubState);
			break;
	}
	if(play_it)
	{
		foot_step_wave	=	((Random()*(S_FOOTS_RUNG_END-S_FOOTS_RUNG_START))>>16)+S_FOOTS_RUNG_START;
		//play_quick_wave(p_person,foot_step_wave,WAVE_PLAY_NO_INTERUPT);
		MFX_play_thing(THING_NUMBER(p_person),foot_step_wave,MFX_OVERLAP,p_person);
	}
}

void	fn_person_climbing(Thing	*p_person)
{
	SLONG	end,hit;
	SLONG	left_foot,right_foot;

	switch(p_person->SubState)
	{
		case	SUB_STATE_CLIMB_LANDING:
			//LogText(" stand to mount ladder \n");

				end=person_normal_animate(p_person);

				if (end==1)
				{
				    if (p_person->Genus.Person->AnimType!=ANIM_TYPE_ROPER) 
						queue_anim(p_person,ANIM_LANDED_ON_FENCE);
					p_person->SubState=SUB_STATE_CLIMB_LANDING2;

					{
						//
						// If this is an electric fence... then fall off the fence and
						// create electricity between the fence and the person...
						//

						if (p_person->Genus.Person->OnFacet > 0)
						{
							SLONG col = p_person->Genus.Person->OnFacet;

							if (col > 0)
							{
								if (dfacets[col].FacetFlags & FACET_FLAG_ELECTRIFIED)
								{
									SLONG i;

									SLONG px;
									SLONG py;
									SLONG pz;
#ifndef PSX
									SPARK_Pinfo p1;
									SPARK_Pinfo p2;
									
									UBYTE limb[4] =
									{
										SUB_OBJECT_LEFT_FOOT,
										SUB_OBJECT_LEFT_HAND,
										SUB_OBJECT_RIGHT_FOOT,
										SUB_OBJECT_RIGHT_HAND
									};

									for (i = 0; i < 4; i++)
									{
										calc_sub_objects_position(
											p_person,
											p_person->Draw.Tweened->AnimTween,
											limb[i],
											&px,
											&py,
											&pz);

										px += p_person->WorldPos.X >> 8;
										py += p_person->WorldPos.Y >> 8;
										pz += p_person->WorldPos.Z >> 8;

										p1.type   = SPARK_TYPE_LIMB;
										p1.flag   = 0;
										p1.person = THING_NUMBER(p_person);
										p1.limb   = limb[i];

										p2.type   = SPARK_TYPE_POINT;
										p2.flag   = SPARK_FLAG_STILL;
										p2.x      = px;
										p2.y      = py;
										p2.z      = pz;

										SPARK_create(
											&p1,
											&p2,
											20);
									}
#endif

									{
										SLONG origin_x = p_person->WorldPos.X - SIN(p_person->Draw.Tweened->Angle) >> 8;
										SLONG origin_z = p_person->WorldPos.Z - COS(p_person->Draw.Tweened->Angle) >> 8;


										knock_person_down(
											p_person,
											30,
											origin_x,
											origin_z,
											NULL);

										// set_person_drop_down(p_person, 0); // 0 => Fall off backwards...
									}

									return;
								}
							}
						}
					}
				}

				break;

		case	SUB_STATE_CLIMB_LANDING2:
				end=person_normal_animate(p_person);
				if(end==1)
				{
					set_person_on_fence(p_person);
				}
				break;

		case	SUB_STATE_CLIMB_AROUND_WALL:
			break;
#ifdef	UNUSED
		case	SUB_STATE_CLIMB_LEFT_WALL:
				if(!check_limb_pos_on_fence_sideways(p_person, SUB_OBJECT_LEFT_HAND))
				{
					locked_anim_change(p_person,0,ANIM_CLIMB_UP_FENCE);
					p_person->SubState=SUB_STATE_CLIMB_AROUND_WALL;
				}
				else
				{
					end=person_normal_animate(p_person);
					if(end==1)
					{
						locked_anim_change(p_person,0,ANIM_CLIMB_UP_FENCE);
						p_person->SubState=SUB_STATE_CLIMB_AROUND_WALL;
					}
				}

			break;
		case	SUB_STATE_CLIMB_RIGHT_WALL:
				if(!check_limb_pos_on_fence_sideways(p_person, SUB_OBJECT_LEFT_HAND))
				{
					locked_anim_change(p_person,0,ANIM_CLIMB_UP_FENCE);
					p_person->SubState=SUB_STATE_CLIMB_AROUND_WALL;
				}

				if(check_limb_pos_on_fence(p_person,SUB_OBJECT_HEAD)) //&&check_limb_pos_on_fence(p_person,SUB_OBJECT_RIGHT_HAND))
				{
					end=person_normal_animate(p_person);
					if(end==1)
					{
						locked_anim_change(p_person,0,ANIM_CLIMB_UP_FENCE);
						p_person->SubState=SUB_STATE_CLIMB_AROUND_WALL;
					}
				}
				break;
#endif


		case	SUB_STATE_CLIMB_UP_WALL:

				//
				// If our hand touches the top of a barbed-wire or angle-top fence, then
				// fall off the fence.
				//

				if (dfacets[p_person->Genus.Person->OnFacet].FacetType == STOREY_TYPE_FENCE_BRICK ||
					dfacets[p_person->Genus.Person->OnFacet].FacetType == STOREY_TYPE_FENCE)
				{
					if (!check_limb_pos_on_fence(p_person, SUB_OBJECT_RIGHT_HAND) ||
						!check_limb_pos_on_fence(p_person, SUB_OBJECT_LEFT_HAND))
					{
						set_person_drop_down(p_person, 0);

						goto dont_check_climbing_over;
					}
				}

				if(check_limb_pos_on_fence(p_person,SUB_OBJECT_HEAD)) //&&check_limb_pos_on_fence(p_person,SUB_OBJECT_RIGHT_HAND))
				{
					end=person_normal_animate(p_person);
					if(end==1)
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_HAND,ANIM_CLIMB_UP_FENCE);

						if (p_person->Genus.Person->PlayerID)
						{
							p_person->SubState=SUB_STATE_STOPPING;
						}
					}
				}
				else
				{
					if (dfacets[p_person->Genus.Person->OnFacet].FacetType==STOREY_TYPE_NORMAL) 
					{
						grab_ledge(p_person);
					}
					else
					if (dfacets[p_person->Genus.Person->OnFacet].FacetType!=STOREY_TYPE_FENCE_BRICK) 
					{
						//
						// Climb off top of fence
						//
						p_person->SubState=SUB_STATE_CLIMB_OVER_WALL;
						locked_anim_change(p_person,0,ANIM_CLIMB_OVER_FENCE);
						set_limb_to_y(p_person,SUB_OBJECT_LEFT_HAND,get_fence_top(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,p_person->Genus.Person->OnFacet));
	//					p_person->Draw.Tweened->QueuedFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_CLIMB_OVER_FENCE];
	//					p_person->Genus.Person->Action=ACTION_CLIMBING;
					} // else blocked from climbing by barbed wire
				}
/*
				if(--p_person->Genus.Person->Timer1==0)
				{
					p_person->SubState=SUB_STATE_STOPPING;
				}
*/

			  dont_check_climbing_over:;

			break;

		case	SUB_STATE_CLIMB_DOWN_WALL:

			left_foot  = check_limb_pos_on_fence(p_person,SUB_OBJECT_LEFT_FOOT);
			right_foot = check_limb_pos_on_fence(p_person,SUB_OBJECT_RIGHT_FOOT);

			if (left_foot && right_foot)
			{
				end = person_backwards_animate(p_person);

				if (end == 1)
				{
					locked_anim_change_end(
						p_person,
						SUB_OBJECT_LEFT_HAND,
						ANIM_CLIMB_UP_FENCE);//,p_person->Genus.Person->AnimType);

					if (p_person->Genus.Person->PlayerID)
					{
						p_person->SubState=SUB_STATE_STOPPING;
					}
				}
			}
			else
			{
				//
				// The bottom of this fence might not be on the ground.
				//

				{
					SLONG ground_y = PAP_calc_height_at_thing(
										p_person,
										p_person->WorldPos.X >> 8,
										p_person->WorldPos.Z >> 8);

					if ((p_person->WorldPos.Y >> 8) <= ground_y + 0x40)
					{
						//
						// Climb off onto the ground.
						//

						p_person->SubState=SUB_STATE_CLIMB_OFF_BOT_WALL;
						set_anim(p_person,ANIM_OFF_LADDER_BOT);
						set_limb_to_y(p_person,SUB_OBJECT_RIGHT_FOOT,get_fence_bottom(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8,p_person->Genus.Person->OnFacet));
					}
					else
					{
						//
						// Fall off- this fence is in the air.
						//

						set_person_drop_down(p_person, 0);	// 0 => Fall off backwards...
					}
				}
			}
/*
			if(--p_person->Genus.Person->Timer1==0)
			{
				p_person->SubState=SUB_STATE_STOPPING;
			}
*/

			break;
		case	SUB_STATE_CLIMB_OFF_BOT_WALL:
				end=person_normal_animate(p_person);
				if(end==1)
				{
					set_person_idle(p_person);
					//set_anim(p_person,ANIM_OFF_LADDER_TOP);
					p_person->Draw.Tweened->CurrentFrame=p_person->Draw.Tweened->QueuedFrame;
					p_person->Draw.Tweened->NextFrame=p_person->Draw.Tweened->QueuedFrame;
					p_person->Draw.Tweened->QueuedFrame=0;
					plant_feet(p_person);
					if(p_person->State!=STATE_DANGLING)
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_STAND_READY);
				}
			break;
		case	SUB_STATE_CLIMB_OVER_WALL:
				end=person_normal_animate(p_person);
				if(end==1)
				{
//					MSG_add(" drop down over fence \n");
					set_person_locked_drop_down(p_person,-32);
					p_person->WorldPos.Y+=p_person->DY;
//					p_person->DY=-16;
					//set_person_falling(p_person);
				}
				break;
		case	SUB_STATE_STOPPING:
			break;
		default:
//			ASSERT(0);
			MSG_add("CLIMBING unknow substate %d \n",p_person->SubState);
			break;


	}
}


void	set_cable_angle(Thing *p_person)
{
	SLONG	dx,dz;
	struct	DFacet	*p_facet;
	SLONG	angle;

	ASSERT(p_person->Genus.Person->Flags&FLAG_PERSON_ON_CABLE);
	ASSERT(dfacets[p_person->Genus.Person->OnFacet].FacetType==STOREY_TYPE_CABLE);

	p_facet=&dfacets[p_person->Genus.Person->OnFacet];

	dx = p_facet->x[1] - p_facet->x[0] << 8;
	dz = p_facet->z[1] - p_facet->z[0] << 8;

	angle=calc_angle(dx,dz);

	p_person->Draw.Tweened->Angle=angle;
}



void	do_person_on_cable(Thing *p_person)
{
	SLONG	along;
	SLONG	mx,my,mz;

	SLONG hx;
	SLONG hy;
	SLONG hz;

	SLONG	lhx,lhy,lhz;
	SLONG	rhx,rhy,rhz;

	if(p_person->Genus.Person->Flags&FLAG_PERSON_ON_CABLE)
	{
		calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_HAND,&lhx,&lhy,&lhz);
		calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_HAND,&rhx,&rhy,&rhz);

		hx = lhx + rhx >> 1;
		hy = lhy + rhy >> 1;
		hz = lhz + rhz >> 1;

		along=get_cable_along(p_person->Genus.Person->OnFacet,(p_person->WorldPos.X>>8)+hx,(p_person->WorldPos.Z>>8)+hz);

		my=find_cable_y_along(&dfacets[p_person->Genus.Person->OnFacet],along);
		MSG_add(" death slide along %d my %d\n",along,my);

		p_person->WorldPos.Y=(my-hy)<<8;
	}
	else
	{
		//
		// Why aren't we on a cable?
		//

		ASSERT(0);
	}
}

void	fn_person_dangling(Thing	*p_person)
{
	SLONG ignore_building;
	SLONG end;
	SLONG hit;
	SLONG	grab;
	SLONG		wave_id1,
				wave_id2;

	switch(p_person->SubState)
	{
		case	SUB_STATE_STOPPING:
			if(p_person->Genus.Person->OnFacet>0 && (p_person->Genus.Person->Flags&FLAG_PERSON_ON_CABLE) )
				p_person->Genus.Person->Action=ACTION_DANGLING_CABLE;

			break;

		case	SUB_STATE_GRAB_TO_DANGLE:
			MSG_add("grab to dangle \n");
				end=person_normal_animate(p_person);
				if(end==1)
				{
					MSG_add("anim end \n");
					p_person->Draw.Tweened->Locked=0;

					if(p_person->Genus.Person->Flags&FLAG_PERSON_ON_CABLE) //p_person->OnFace>0 && prim_faces4[p_person->OnFace].Type==FACE_TYPE_CABLE)
					{
						
						set_person_drop_down(p_person,PERSON_DROP_DOWN_OFF_FACE);
/*

						p_person->SubState=SUB_STATE_DANGLING_CABLE;
						MSG_add(" its a cable grab face %d",p_person->Genus.Person->OnFacet);
						p_person->Draw.Tweened->Angle=find_best_cable_angle(p_person,p_person->Genus.Person->OnFacet);

						p_person->Genus.Person->Action=ACTION_DANGLING_CABLE;
						locked_anim_change(p_person,SUB_OBJECT_LEFT_HAND,ANIM_HAND_OVER_HAND);
*/
						
					}
					else


					{
						p_person->SubState=SUB_STATE_DANGLING;
//						MSG_add(" ledge dangle");
						p_person->Genus.Person->Action=ACTION_DANGLING;
						p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
						
					}
					//LogText(" set action dangling \n");
				}
			break;
		case	SUB_STATE_DEATH_SLIDE:
			end=person_normal_animate(p_person);
			if(end)
			{
				if(p_person->Draw.Tweened->CurrentAnim==ANIM_DEATH_SLIDE)
				{
					set_anim(p_person,ANIM_WIRE_SLIDE_PULLUP);
				}
				else
				if(p_person->Draw.Tweened->CurrentAnim==ANIM_WIRE_SLIDE_PULLUP)
				{
					set_anim(p_person,ANIM_WIRE_SLIDE_HANG);
				}

			}
			p_person->Velocity+=p_person->DeltaVelocity;
			if(p_person->Velocity>256)
				p_person->Velocity=256;
			person_death_slide(p_person);
			break;


		case	SUB_STATE_DANGLING:
			MSG_add(" dangling NORMAL ");
			//LogText(" dangling action %d\n",p_person->Genus.Person->Action);

			/*

			if(Keys[KB_0])
				set_person_idle(p_person);

			*/

			break;
		case	SUB_STATE_DANGLING_CABLE:
//			MSG_add(" dangling cable ");

			break;
		case	SUB_STATE_DANGLING_CABLE_FORWARD:
			p_person->Draw.Tweened->Locked=0;
			end=person_normal_animate(p_person);
			if(end==1)
			{
//				MSG_add(" LOCKED CHANGE END FORWARD\n");
					
				set_person_drop_down(p_person,PERSON_DROP_DOWN_OFF_FACE);
				return;
//				locked_anim_change(p_person,SUB_OBJECT_LEFT_HAND,ANIM_HAND_OVER_HAND);
			}
			do_person_on_cable(p_person);

			break;
		case	SUB_STATE_DANGLING_CABLE_BACKWARD:
					end=person_backwards_animate(p_person);
					if(end==1)
					{
//						MSG_add(" LOCKED CHANGE END \n");
						set_person_drop_down(p_person,PERSON_DROP_DOWN_OFF_FACE);
						return;
						//locked_anim_change_end(p_person,SUB_OBJECT_LEFT_HAND,ANIM_HAND_OVER_HAND);
					}
					do_person_on_cable(p_person);

			break;


		case SUB_STATE_DROP_DOWN_OFF_FACE:
		case SUB_STATE_DROP_DOWN:

				end = person_normal_animate(p_person);

				if (end == 1)
				{
					if (p_person->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_START)
					{
						locked_anim_change(p_person, SUB_OBJECT_PELVIS, ANIM_PLUNGE_FORWARDS);
					}
				}

				//
				// Knock people over when you jump on them
				//

				drop_on_heads(p_person);

				{
					SLONG temp_angle;

					//
					// If you slip of a face then your velocity is not necessarily in the direction your facing
					// (while slipping angleto is your movement angle), and it continues with dropdown
					//

					if (p_person->Genus.Person->Flags & FLAG_PERSON_MOVE_ANGLETO)
					{
						temp_angle                    = p_person->Draw.Tweened->Angle;
						p_person->Draw.Tweened->Angle = p_person->Draw.Tweened->AngleTo;
					}
					if(over_nogo(p_person))
						p_person->Velocity=0;

					if (p_person->DY<-4000)
					{
						if (p_person->Velocity>0)
						{
							hit=projectile_move_thing(p_person,3);
						}
						else
						{
							hit=projectile_move_thing(p_person,2|8|1); //2
						}
					}
					else
					{
						if (p_person->Velocity>0)
						{
							hit=projectile_move_thing(p_person,1);
						}
						else
						{
							hit=projectile_move_thing(p_person,1|8); //2
						}
					}


						if(p_person->Genus.Person->Flags & FLAG_PERSON_MOVE_ANGLETO)
						{
							p_person->Draw.Tweened->Angle = temp_angle;

							if(hit)
							{
								p_person->Genus.Person->Flags &= ~FLAG_PERSON_MOVE_ANGLETO;
							}
						}
				}

				if (hit == 100)
				{
					//
					// Projectile move thing killed the person!
					//

					return;
				}

				if (p_person->SubState == SUB_STATE_DROP_DOWN_OFF_FACE)
				{
					//
					// We don't grab faces when falling (backwards) off a face.
					//

					grab = FALSE;
				}
				else
				{
					grab = grab_ledge(p_person);
				}

				if (!grab && hit == 1)
				{
					//
					// projectile move thing sets person foot to floor height so need to
					// maintain foot pos in world across anims
					//

					if (p_person->DY < -15000)
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_BIG_LAND);
					}
					else
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_LAND_VERT);
					}

					p_person->SubState				=	SUB_STATE_DROP_DOWN_LAND;
					p_person->Genus.Person->Action	=	ACTION_LANDING;

					person_splash(p_person, -1);

					{
						wave_id1 = footstep_wave(p_person);
						wave_id2 = footstep_wave(p_person);

						MFX_play_thing(THING_NUMBER(p_person),wave_id1,MFX_REPLACE,p_person);
						MFX_play_thing(THING_NUMBER(p_person),wave_id2,MFX_REPLACE,p_person);
					}
				}

				break;

/*

		case	SUB_STATE_DROP_DOWN_OFF_FACE:
				{
					SLONG	temp_angle;


//If you slip of a face then your velocity is not necessarily in the direction your facing
//(while slipping angleto is your movement angle), and it continues with dropdown


					if(p_person->Genus.Person->Flags&FLAG_PERSON_MOVE_ANGLETO)
					{
						temp_angle=p_person->Draw.Tweened->Angle;
						p_person->Draw.Tweened->Angle=p_person->Draw.Tweened->AngleTo;
					}

					if(p_person->Velocity>0)
						hit=projectile_move_thing(p_person,3);
					else
						hit=projectile_move_thing(p_person,2); //2

					if(p_person->Genus.Person->Flags&FLAG_PERSON_MOVE_ANGLETO)
					{
						p_person->Draw.Tweened->Angle=temp_angle;
						if(hit)
							p_person->Genus.Person->Flags&=~FLAG_PERSON_MOVE_ANGLETO;

					}
				}
				drop_on_heads(p_person);
				if(hit==100)
				{
					//
					// dead
					//

					return;
				}
				if(hit==1)
				{
//					MSG_add(" hit something while falling ");
					if (p_person->DY < -15000)
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_BIG_LAND);
					}
					else
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_LAND_VERT);

					}
//
//					if (p_thing->DY < -5000)
//					{
//						queue_anim(p_person,ANIM_BIG_LAND);
//					}
//					else
//					{
//						queue_anim(p_person,ANIM_LAND_VERT);
//
//					}
//

					p_person->SubState		=	SUB_STATE_DROP_DOWN_LAND;
					p_person->Genus.Person->Action		=	ACTION_LANDING;

					person_splash(p_person, -1);

//					if(p_person->Draw.Tweened->FrameIndex==1)
					{
						wave_id1	=	footstep_wave(p_person);
						wave_id2	=	footstep_wave(p_person);
//						play_quick_wave(p_person,wave_id1,WAVE_PLAY_INTERUPT);
//						play_quick_wave(p_person,wave_id2,WAVE_PLAY_OVERLAP);
						MFX_play_thing(THING_NUMBER(p_person),wave_id1,MFX_REPLACE,p_person);
						MFX_play_thing(THING_NUMBER(p_person),wave_id2,MFX_REPLACE,p_person);
					}
				}

			break;

		case	SUB_STATE_DROP_DOWN:

//				MSG_add(" Do DROP DOWN");

				person_normal_animate(p_person);
				
				{
					SLONG	temp_angle;

//
//If you slip of a face then your velocity is not necessarily in the direction your facing
//(while slipping angleto is your movement angle), and it continues with dropdown
//

					if(p_person->Genus.Person->Flags&FLAG_PERSON_MOVE_ANGLETO)
					{
						temp_angle=p_person->Draw.Tweened->Angle;
						p_person->Draw.Tweened->Angle=p_person->Draw.Tweened->AngleTo;
					}

					if(p_person->Velocity>0)
						hit=projectile_move_thing(p_person,3);
					else
						hit=projectile_move_thing(p_person,2);

					if(p_person->Genus.Person->Flags&FLAG_PERSON_MOVE_ANGLETO)
					{
						p_person->Draw.Tweened->Angle=temp_angle;
						if(hit)
							p_person->Genus.Person->Flags&=~FLAG_PERSON_MOVE_ANGLETO;

					}
				}
				drop_on_heads(p_person);
				if(hit==100)
				{
					//
					// dead
					//

					return;
				}
				//person_normal_move(p_person);
				grab=grab_ledge(p_person);
				if(grab)
					MSG_add(" running jump grabbed \n");
				if(!grab)
				if(hit==1)
				{
//					MSG_add(" hit something while falling ");
//					queue_anim(p_person,ANIM_LAND_VERT);
					//
					// projectile move thing sets person foot to floor height so need to maintain foot pos in world across anims
					//
					if (p_person->DY < -15000)
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_BIG_LAND);
					}
					else
					{
						locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_LAND_VERT);

					}
//					plant_feet(p_person);

					//DrawTween	*draw_info;
					//draw_info=p_person->Draw.Tweened;
					//draw_info->QueuedFrame	=	global_anim_array[p_person->Genus.Person->AnimType][ANIM_LAND_VERT];
					//draw_info->CurrentAnim	=	ANIM_LAND_VERT;
					p_person->SubState		=	SUB_STATE_DROP_DOWN_LAND;
					p_person->Genus.Person->Action		=	ACTION_LANDING;

					person_splash(p_person, -1);

//					if(p_person->Draw.Tweened->FrameIndex==1)
					{
						wave_id1	=	footstep_wave(p_person);
						wave_id2	=	footstep_wave(p_person);
//						play_quick_wave(p_person,wave_id1,WAVE_PLAY_INTERUPT);
//						play_quick_wave(p_person,wave_id2,WAVE_PLAY_OVERLAP);
						MFX_play_thing(THING_NUMBER(p_person),wave_id1,MFX_REPLACE,p_person);
						MFX_play_thing(THING_NUMBER(p_person),wave_id2,MFX_REPLACE,p_person);
					}
		
				}
			break;
*/

		case	SUB_STATE_DROP_DOWN_LAND:
				change_velocity_to(p_person,0);
				if(over_nogo(p_person))
					p_person->Velocity=0;
				person_normal_move(p_person);
				end=person_normal_animate(p_person);
				if(end==1)
				{
					set_person_idle(p_person);		
					if(p_person->SubState!=SUB_STATE_RUNNING)
						plant_feet(p_person);
				}
			break;
		case	SUB_STATE_TRAVERSE_LEFT:
		case	SUB_STATE_TRAVERSE_RIGHT:
			end = person_normal_animate(p_person);
			if(end)
			{
				SLONG	dist,dx,dz;
				locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_DANGLE);

				p_person->SubState		=	SUB_STATE_DANGLING;
				p_person->Genus.Person->Action=ACTION_DANGLING;
/*
				if((dist=abs(check_near_facet(p_person,64,64,(p_person->WorldPos.X)>>8,(p_person->WorldPos.Z)>>8)))==0)
				{
					return;
				}

				if(abs(32-dist)<32)
				{
					SLONG	angle;
					angle=(p_person->Draw.Tweened->Angle+1024)&2047;

					dx = -(SIN(angle) * (32-dist)) >> 8;
					dz = -(COS(angle) * (32-dist)) >> 8;

				}

				person_normal_move_dxdz(p_person,dx,dz);
*/
				#if 0

				//
				// How far is Darci from the facet she is hanging onto?
				//

				{
					SLONG dist;

					SLONG dx = -SIN(p_person->Draw.Tweened->Angle);
					SLONG dz = -COS(p_person->Draw.Tweened->Angle);

					if (abs(dx) > abs(dz))
					{
						if (dx > 0)
						{
							dist = 0x10000 - (p_person->WorldPos.X & 0xffff);
						}
						else
						{
							dist = p_person->WorldPos.X & 0xffff;
						}
					}
					else
					{
						if (dz > 0)
						{
							dist = 0x10000 - (p_person->WorldPos.Z & 0xffff);
						}
						else
						{
							dist = p_person->WorldPos.Z & 0xffff;
						}
					}
#ifndef	FINAL
					PANEL_new_text(NULL, 4000, "Distance from facet = 0x%x", dist);
#endif

				}

				#endif
			}

			break;
		case	SUB_STATE_PULL_UP:

				end = person_normal_animate(p_person);

				if (MagicFrameCheck(p_person,2)) EffortSound(p_person);

				if(end==1)
				{
					//
					// Finished pulling ourselves up.
					//

					SLONG	face,new_y;
					//p_person->Draw.Tweened->Locked=0;
					//p_person->SubState=SUB_STATE_DANGLING;
					p_person->Draw.Tweened->AnimTween=0;		
					locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_STAND_READY);

					if (p_person->Flags & FLAGS_IN_BUILDING)
					{
						ignore_building = INDOORS_DBUILDING;
					}
					else
					{
						ignore_building = NULL;
					}

					//
					// Find what face we are stood on now
					//
#if !defined(PSX) && !defined(TARGET_DC)
					if (p_person->Flags & FLAGS_IN_SEWERS)
					{
						p_person->OnFace     = 0;
						p_person->WorldPos.Y = NS_calc_height_at(p_person->WorldPos.X >> 8, p_person->WorldPos.Z >> 8) << 8;
					}
					else
#endif
					{
						face = find_face_for_this_pos(p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,&new_y,ignore_building,0);

						if(face==GRAB_FLOOR)
						{
							ASSERT(0);

							p_person->OnFace=0;
							p_person->WorldPos.Y=new_y<<8;
						}
						else
						if(face)
						{
							p_person->OnFace=face;
	//						new_y=calc_height_on_face(p_person->WorldPos.X,p_person->WorldPos.Z,face);
							p_person->WorldPos.Y=new_y<<8;
						}
						else
						{
							ASSERT(0);

							p_person->WorldPos.Y=0;
						}
					}

					//
					// This is a subset of set person idle (the set anim stuff is removed
					//

					set_person_idle(p_person);

					/*
					set_generic_person_state_function(p_person,STATE_IDLE);
					p_person->SubState=0;
					p_person->Genus.Person->Action=ACTION_IDLE;
					p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
					*/
					
				}
			break;

			default:
				set_person_drop_down(p_person, 0);	// EMERGENCY!
				break;
	}
	
}

extern	void	trickle_velocity_to(Thing *p_thing,SWORD velocity);


void	set_person_running_stop(Thing *p_person,SLONG leg)
{
	
//	tween_to_anim(p_person,ANIM_STOP_RUN_L+leg);
	p_person->SubState=SUB_STATE_STOPPING;
}

WaveParams	foot_step;


SLONG should_person_automatically_land_on_fence(Thing *p_person, SLONG facet)
{
	SLONG dx;
	SLONG dz;
	SLONG da;
	SLONG mdx;
	SLONG mdz;
	SLONG angle;

	DFacet *df;

	df = &dfacets[facet];

	//
	// Only players...
	//

	if (p_person->Genus.Person->PlayerID == 0)
	{
		return FALSE;
	}

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		if (p_special->Genus.Special->SpecialType == SPECIAL_WIRE_CUTTER)
		{
			//
			// While holding wire cutters don't auto climb fences, because you might want to cut it
			//
			return(FALSE);

		}
	}


	//
	// Is this a climbable fence facet?
	// 

	if (df->FacetType == STOREY_TYPE_FENCE      ||
		df->FacetType == STOREY_TYPE_NORMAL		||
		df->FacetType == STOREY_TYPE_FENCE_FLAT ||
		df->FacetType == STOREY_TYPE_FENCE_BRICK)
	{
		if (!(df->FacetFlags & FACET_FLAG_UNCLIMBABLE))
		{
			//
			// Is the person's angle correct?
			//

			dx = abs(df->x[1] - df->x[0] << 8);
			dz = abs(df->z[1] - df->z[0] << 8);

			angle  = Arctan(-dx,dz) + 1536;
			angle &= 2047;

			da = abs(angle - p_person->Draw.Tweened->Angle);

			#define FENCE_DA 128

			if ((da > FENCE_DA        && da < 1024 - FENCE_DA ) ||
				(da > FENCE_DA + 1024 && da < 2048 - FENCE_DA ))
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
	}

	/*

	if (da > 512 && da < 2048 - 512)
	{
		angle += 1024;
		angle &= 2047;
	}

	*/

	return FALSE;
}

void process_a_vaulting_person(Thing *p_person)
{
	SLONG end;

	end = person_normal_animate(p_person);

	if(p_person->Draw.Tweened->FrameIndex==6)
	{
		SLONG	wx,wy,wz,fy;

		calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_FOOT,&wx,&wy,&wz);

		wx += p_person->WorldPos.X >> 8;
		wy += p_person->WorldPos.Y >> 8;
		wz += p_person->WorldPos.Z >> 8;

		fy=PAP_calc_height_at(wx,wz);

		if(abs(fy-(wy-12))>8)
		{
			p_person->Velocity = 6;
			p_person->DY = -(10 << 8);
			set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL|PERSON_DROP_DOWN_KEEP_DY);//|PERSON_DROP_DOWN_OFF_FACE);
		}
	}

	if(end==1)
	{
		p_person->Genus.Person->Flags &= ~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

		locked_anim_change(p_person,0,ANIM_STAND_READY);

		//
		// Only non-player characters can continue running.
		//

		if (p_person->Genus.Person->PlayerID && continue_moveing(p_person))
		{
			set_person_running(p_person);
		}
		else
		{
			set_person_idle(p_person);
		}
	}
}



void set_person_sit_down(Thing *p_person)
{
	set_generic_person_state_function(p_person, STATE_MOVEING);

	set_person_do_a_simple_anim(p_person, ANIM_SIT_DOWN);

	p_person->SubState               = SUB_STATE_SIMPLE_ANIM;	
	p_person->Genus.Person->Flags   |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
	p_person->Genus.Person->Action   = ACTION_SIT_BENCH;

	if (GAME_FLAGS & GF_DISABLE_BENCH_HEALTH)
	{
		p_person->Genus.Person->Timer1 = 100;
	}
	else
	{
		p_person->Genus.Person->Timer1 = 0;
		GAME_FLAGS                    |= GF_DISABLE_BENCH_HEALTH;
	}
}

void set_person_unsit(Thing *p_person)
{
	set_generic_person_state_function(p_person, STATE_MOVEING);

	set_person_do_a_simple_anim(p_person, ANIM_SIT_TO_STAND);

	p_person->SubState               =  SUB_STATE_SIMPLE_ANIM;	
	p_person->Genus.Person->Flags   &= ~FLAG_PERSON_NO_RETURN_TO_NORMAL;
	p_person->Genus.Person->Action   =  ACTION_UNSIT;
}


SLONG	person_holding_2handed(Thing *p_person)
{
	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		//
		// The shotgun and ak47 have their own special shotgun yomp.
		// 

		if (p_special->Genus.Special->SpecialType == SPECIAL_SHOTGUN ||
//			p_special->Genus.Special->SpecialType == SPECIAL_BASEBALLBAT   ||
			p_special->Genus.Special->SpecialType == SPECIAL_AK47)
		{
			return(1);
		}	
	}
	return(0);
}

SLONG	person_holding_special(Thing* p_person, UBYTE special)
{
	if (!p_person->Genus.Person->SpecialUse)	return 0;

	Thing*	p_special = TO_THING(p_person->Genus.Person->SpecialUse);

	return (p_special->Genus.Special->SpecialType == special) ? 1 : 0;
}

SLONG	get_yomp_anim(Thing *p_person)
{


	if(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
		return(ANIM_PISTOL_JOG);

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		//
		// The shotgun and ak47 have their own special shotgun yomp.
		// 

		if (p_special->Genus.Special->SpecialType == SPECIAL_BASEBALLBAT)
		{
			return(ANIM_YOMP_BAT);
		}

		if (p_special->Genus.Special->SpecialType == SPECIAL_SHOTGUN ||
			p_special->Genus.Special->SpecialType == SPECIAL_AK47)
		{
			return(ANIM_AK_JOG);
		}	
	}
	if(p_person->Genus.Person->PersonType==PERSON_COP)
	{
		return(COP_ROPER_ANIM_RUN);
	}
	else
	return(ANIM_YOMP);
}

void	fn_person_moveing(Thing *p_person)
{
	UBYTE		last_frame;
	SLONG		end,
				foot_step_wave;
	SLONG		yomp,sprint,change,stamina_used;

	MSG_add(" state %d substate %d vel %d \n",p_person->State,p_person->SubState,p_person->Velocity);

	if(p_person->Genus.Person->PlayerID)
	{
		camera_normal();
	}
	
	//LogText(" person moveing  substate %d\n",p_person->SubState);

	//return;
	if (p_person->SubState == SUB_STATE_RUNNING           ||
	    p_person->SubState == SUB_STATE_WALKING			  )
		{
		//
		//
		//
			switch(p_person->Draw.Tweened->CurrentAnim)
			{
				case	ANIM_STAND_HIP:
				case	ANIM_STAND_READY:
				case	ANIM_SHOTGUN_IDLE:
				case	COP_ROPER_ANIM_READY:
				case	ANIM_FIGHT:
					{
						SLONG	anim;
						anim=get_yomp_anim(p_person);
						if(anim==COP_ROPER_ANIM_RUN)
						{
							set_anim_of_type(p_person,anim,ANIM_TYPE_ROPER);
						}
						else
							set_anim(p_person,anim);
					}
					break;


			}
		}

	if (p_person->SubState == SUB_STATE_RUNNING           ||
	    p_person->SubState == SUB_STATE_WALKING			  ||
		p_person->SubState == SUB_STATE_SNEAKING		  ||
		p_person->SubState == SUB_STATE_RUNNING_SKID_STOP ||
		p_person->SubState == SUB_STATE_RUNNING_THEN_JUMP)
	{
		if (p_person->Draw.Tweened->FrameIndex == 1)
		{
			person_splash(p_person, SUB_OBJECT_RIGHT_FOOT);
		}
		else
		if (p_person->Draw.Tweened->FrameIndex == 5)
		{
			person_splash(p_person, SUB_OBJECT_LEFT_FOOT);
		}
	}
	SlideSoundCheck(p_person);
	switch(p_person->SubState)
	{
		case	SUB_STATE_ENTERING_VEHICLE:
			end=person_normal_animate(p_person);
			if(end)
			{
				set_person_in_vehicle(p_person,TO_THING(p_person->Genus.Person->InCar));
			}
			break;

		case	SUB_STATE_INSIDE_VEHICLE:
			
			//
			// We are not on the mapwho at this point- so we just track the position of
			// the vehicle we are in and teleport to that place.
			//

			p_person->WorldPos            = TO_THING(p_person->Genus.Person->InCar)->WorldPos;
			p_person->Draw.Tweened->Angle = TO_THING(p_person->Genus.Person->InCar)->Genus.Vehicle->Angle;

			//
			// Vehicles can drive off the map!
			//

			SATURATE(p_person->WorldPos.X, 0, (PAP_SIZE_HI << 16) - 1);
			SATURATE(p_person->WorldPos.Z, 0, (PAP_SIZE_HI << 16) - 1);

			break;

		case	SUB_STATE_EXITING_VEHICLE:
			end=person_normal_animate(p_person);
			if(end)
			{
				set_person_out_of_vehicle(p_person);
			}
			break;

		case	SUB_STATE_SLIPPING_END:
			change_velocity_to(p_person,0);
			if(p_person->Velocity==0)
			{
				set_person_idle(p_person);
				return;
			}

		case	SUB_STATE_SLIPPING:
			{
				SLONG	angle;
				angle=p_person->Draw.Tweened->Angle;
				p_person->Draw.Tweened->Angle=p_person->Draw.Tweened->AngleTo;

				person_normal_move(p_person);

				p_person->Draw.Tweened->Angle=angle;
				
			}

			{
				SLONG fx,fy,fz,px,pz,rgb,sz,i,j;
				calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,
					SUB_OBJECT_LEFT_FOOT, &fx, &fy, &fz);
				fx=(fx<<8) + p_person->WorldPos.X;
				fy=(fy<<8) + p_person->WorldPos.Y;
				fz=(fz<<8) + p_person->WorldPos.Z;
//				rgb=0xFF-(p_person->Velocity<<2);
				rgb=0x7F-(p_person->Velocity<<1);
				if (rgb<0) 
				{
					rgb=0;
				}
				rgb=(rgb<<24)|0xc9b7a3;
				j=(p_person->Velocity>40) ? 7 : 3;
				for (i=0;i<j;i++) 
				{
				  px=fx+(((Random()&0x7ff)-0x3ff)<<2);
				  pz=fz+(((Random()&0x7ff)-0x3ff)<<2);
				  PARTICLE_Add(px,fy,pz,(Random()&7)-3,20,(Random()&7)-3,POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),rgb,PFLAG_FADE|PFLAG_RESIZE,40,25,1,10,4);
				}
			}


			break;


				break;

		case	SUB_STATE_RUNNING_HALF_BLOCK:
				end=person_normal_animate(p_person);
				if(end)
				{
					locked_anim_change(p_person,0,ANIM_STAND_READY);
					plant_feet(p_person);
					if(p_person->State!=STATE_DANGLING)
						set_person_idle(p_person);
				}
			break;
		case	SUB_STATE_RUNNING_VAULT:
			process_a_vaulting_person(p_person);
			break;

		case	SUB_STATE_RUNNING_HIT_WALL:
				end=person_normal_animate(p_person);
				if(end)
				{
					set_person_idle(p_person);

					//set_person_turn_to_hug_wall(p_person);
				}
				break;
		case	SUB_STATE_RUNNING:
			//LogText(" running \n");

				last_frame	=	p_person->Draw.Tweened->FrameIndex;
				last_slide_colvect=0;

//				p_person->Flags&=~FLAG_PERSON_AIM_AND_RUN;
				if(!(p_person->Genus.Person->Flags2&FLAG2_PERSON_CARRYING))
				if(p_person->Genus.Person->PlayerID)// && p_person->Draw.Tweened->CurrentAnim==ANIM_YOMP)
				{
					if (p_person->Genus.Person->Timer1)
					{
						SLONG ticks = 16 * TICK_RATIO >> TICK_SHIFT;

						//
						// Countdown since te person last fired his gun.
						//

						if (p_person->Genus.Person->Timer1 <= ticks)
						{
							p_person->Genus.Person->Timer1 = 0;

							if (p_person->Genus.Person->SpecialUse)
							{
								Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

								if (p_special->Genus.Special->SpecialType == SPECIAL_AK47)
								{
									if (continue_firing(p_person))
									{
										set_person_running_shoot(p_person);
									}
								}
							}
						}
						else
						{
							p_person->Genus.Person->Timer1 -= ticks;
						}
					}

					//if(p_person->Draw.Tweened->CurrentAnim==ANIM_AK_JOG)
					if(person_has_gun_out(p_person))
					{
						//
						// we are jogging along with a shotgun or ak47 or pistol lets aim at people
						//

						player_running_aim_gun(p_person);

					}

				}
				end=person_normal_animate(p_person);

				p_person->DY=0;


				if(end==1)
				{
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_HIT_WALL:
						case	ANIM_YOMP_START:
						case	ANIM_YOMP_START_PISTOL:
						case	ANIM_YOMP_START_AK:
						case	ANIM_YOMP_START_BAT:
							
							{
								UWORD anim;

								anim=get_yomp_anim(p_person);
								if(anim==COP_ROPER_ANIM_RUN)
								{
									set_anim_of_type(p_person,anim,ANIM_TYPE_ROPER);
								}
								else
									set_anim(p_person,anim);
							}

							break;
						case	ANIM_START_WALK_CARRY:
							{
								Thing	*p_target;
								p_target=TO_THING(p_person->Genus.Person->Target);
								set_anim(p_target,ANIM_WALK_CARRY_V);
								set_anim(p_person,ANIM_WALK_CARRY);
							}

					}

				}
				

/*
				switch(p_person->Genus.Person->AnimType) 
				{
					case ANIM_TYPE_ROPER:
						sprint=60;
						yomp=34;
//						change=15;
						change=40; // immediately :}
//						stamina_used=1;
						stamina_used=4; // since roper runs all the time...
						break;
					default:	// darci, others
						sprint=sprint_speed;
						yomp=yomp_speed;
						change=50;
						stamina_used=2;
						break;
				}
*/

				sprint=sprint_speed;
				yomp=yomp_speed;
				change=50;
				stamina_used = ((PTIME(p_person) & 0x3) == 0);

				if(p_person->Genus.Person->AnimType==ANIM_TYPE_ROPER)
				{
					stamina_used=4;

				}

				{
					SLONG	run_speed=yomp; //_speed;
					if(p_person->Genus.Person->Mode==PERSON_MODE_SPRINT)
					{
						run_speed=sprint; //_speed;

						if (p_person->Genus.Person->Stamina < 10)
						{
							p_person->Genus.Person->Stamina=0;
							p_person->Genus.Person->Mode=PERSON_MODE_RUN;
							switch(p_person->Genus.Person->PersonType) {
							case PERSON_DARCI:
								MFX_play_thing(THING_NUMBER(p_person),S_DARCI_OUTOFBREATH,MFX_MOVING,p_person);
								break;
							case PERSON_ROPER:
								MFX_play_thing(THING_NUMBER(p_person),S_ROPER_OUTOFBREATH,MFX_MOVING,p_person);
								break;
							}
						}
						else
						{
							p_person->Genus.Person->Stamina-=stamina_used;
						}

						if(p_person->Draw.Tweened->CurrentAnim	== ANIM_YOMP)
						{
							if(end||(change==1))
								if(p_person->Velocity>change)
									set_anim(p_person,ANIM_RUN);
						}

						//
						// accelerate twice in sprint mode
						//

						change_velocity_to(p_person,run_speed);
						
					}
					else
					{
						if(p_person->Draw.Tweened->CurrentAnim	== ANIM_RUN)
						{
							if(end)
								if(p_person->Velocity<change-5)
									set_anim(p_person,ANIM_YOMP);
						}
					}

					change_velocity_to(p_person,run_speed);
extern	UBYTE	cheat;
//					if(cheat==1)
//						change_velocity_to(p_person,run_speed*2);


				}


				if(p_person->Draw.Tweened->CurrentAnim==ANIM_YOMP_START || p_person->Draw.Tweened->CurrentAnim==ANIM_YOMP_START_AK|| p_person->Draw.Tweened->CurrentAnim==ANIM_YOMP_START_PISTOL|| p_person->Draw.Tweened->CurrentAnim==ANIM_YOMP_START_BAT)
				{
					if(p_person->Draw.Tweened->FrameIndex<2)
						p_person->Velocity=0;
				}

				last_slide_colvect=0;
				person_normal_move(p_person);

//SLONG	along_middle_of_facet(Thing *p_person,SLONG col)
				if(p_person->Genus.Person->Flags2&FLAG2_PERSON_CARRYING)
				{
					carry_running(p_person);
				}
				else
				{
					if(last_slide_colvect&&along_middle_of_facet(p_person,last_slide_colvect))
					{
	//					p_person->Velocity>>=1;
						if (is_facet_vaultable(last_slide_colvect))
						{
							if(set_person_vault(p_person,last_slide_colvect))
							{
								return;
							}
						}
						if (is_facet_half_step(last_slide_colvect))
						{
							if(set_person_climb_half(p_person,last_slide_colvect))
							{
								return;
							}
						}

						if (should_person_automatically_land_on_fence(p_person,last_slide_colvect))
						{	
							set_person_land_on_fence(p_person, last_slide_colvect, 1,1);
							return;
						}

	//#define WE_WANT_TO_TURN_AND_PUT_OUR_BACK_TO_THE_WALL	77179
	#ifdef WE_WANT_TO_TURN_AND_PUT_OUR_BACK_TO_THE_WALL

						if(p_person->Velocity>35)
						{
							SLONG	wall_angle;
							if(is_wall_good_for_bump_and_turn(p_person,last_slide_colvect))
							if(am_i_facing_wall(p_person,last_slide_colvect,&wall_angle))
							{
								p_person->Velocity=0;
								ASSERT((wall_angle&0x1ff)==0);
								p_person->Draw.Tweened->Angle=(wall_angle)&2047;
								set_person_turn_to_hug_wall(p_person);

						//							set_person_idle(p_person);
	//							set_anim(p_person,ANIM_HIT_WALL);
	//							p_person->SubState=SUB_STATE_RUNNING_HIT_WALL;
								return;
							}
						}

	#endif
						if(0)
						if(p_person->Velocity>35)
						{
							SLONG	wall_angle;
							if(is_wall_good_for_bump_and_turn(p_person,last_slide_colvect))
							if(am_i_facing_wall(p_person,last_slide_colvect,&wall_angle))
							{
								p_person->Velocity=0;
								ASSERT((wall_angle&0x1ff)==0);
								p_person->Draw.Tweened->Angle=(wall_angle)&2047;

//								set_person_idle(p_person);
								set_anim(p_person,ANIM_HIT_WALL);
								p_person->SubState=SUB_STATE_RUNNING_HIT_WALL;
								if(p_person->Genus.Person->PersonType==PERSON_DARCI)
									MFX_play_thing(THING_NUMBER(p_person),SOUND_Range(S_DARCI_EFFORT_START,S_DARCI_EFFORT_END),0,p_person);
								return;
							}
						}
					}

					if (p_person->Genus.Person->PlayerID)
					if(slide_ladder)
					{
						if(mount_ladder(p_person, slide_ladder))
							return;


					}
				}
//				if(p_person->Draw.Tweened->FrameIndex==0 || p_person->Draw.Tweened->FrameIndex==3)
//				if(p_person->Draw.Tweened->FrameIndex==1 || p_person->Draw.Tweened->FrameIndex==5)
				if(p_person->Draw.Tweened->FrameIndex==1 || p_person->Draw.Tweened->FrameIndex==(4+(p_person->Genus.Person->Mode==PERSON_MODE_RUN)))
				{
					if (p_person->Flags & FLAGS_PLAYED_FOOTSTEP)
					{
						//
						// Don't play twice!
						//
					}
					else
					{
						foot_step_wave = footstep_wave(p_person);

						/*

						oscilate_tinpanum(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							p_person,
							64);

						*/

						PCOM_oscillate_tympanum(
							PCOM_SOUND_FOOTSTEP,
							p_person,
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8);

						foot_step.Flags					=	WAVE_CARTESIAN;
						foot_step.Priority				=	0;
						foot_step.Mode.Cartesian.Scale	=	(128<<8);
						foot_step.Mode.Cartesian.X		=	p_person->WorldPos.X;
						foot_step.Mode.Cartesian.Y		=	p_person->WorldPos.Y;
						foot_step.Mode.Cartesian.Z		=	p_person->WorldPos.Z;

						/*
						if(GAME_FLAGS&GF_SEWERS && p_person->Draw.Tweened->FrameIndex!=last_frame)
						{
							foot_step_wave	=	((Random()*(S_FOOTS_SEWER_END-S_FOOTS_SEWER_START))>>16)+S_FOOTS_SEWER_START;;
//							PlayWave(THING_NUMBER(p_person),foot_step_wave,WAVE_PLAY_OVERLAP,&foot_step);
							play_quick_wave_old(&foot_step,foot_step_wave,0,0);
						}
						else
						{
//							PlayWave(THING_NUMBER(p_person),foot_step_wave,WAVE_PLAY_INTERUPT,&foot_step);
							play_quick_wave_old(&foot_step,foot_step_wave,0,0);
						}
						*/
//						play_quick_wave_old(&foot_step,foot_step_wave,0,0);
						MFX_play_thing(THING_NUMBER(p_person),foot_step_wave,MFX_REPLACE,p_person);

						p_person->Flags |= FLAGS_PLAYED_FOOTSTEP;
					}
				}
				else
				{
					p_person->Flags &= ~FLAGS_PLAYED_FOOTSTEP;
				}
				


//				MSG_add(" velocity %d \n",p_person->Velocity);

			break;
		case	SUB_STATE_CRAWLING:
				if(slope_ahead(p_person,100))
					return;

				/*
				switch(p_person->Genus.Person->AnimType) 
				{
//					case ANIM_TYPE_ROPER:
//						change_velocity_to(p_person,7);
//						break;
					default:	// darci, others
				*/
						change_velocity_to(p_person,15);
				/*
						break;
				}
				*/
				person_normal_move(p_person);
				person_normal_animate(p_person);
			break;
		case	SUB_STATE_WALKING:
				
				if(p_person->Genus.Person->PersonType==PERSON_CIV)
				{
					SLONG	old;
					if(!(p_person->Draw.Tweened->MeshID&1))
						change_velocity_to(p_person,14);
					else
						change_velocity_to(p_person,10); //bloody women, walking round slow
				}
				else
					change_velocity_to(p_person,16);
				person_normal_move(p_person);

				//
				// Make walking people able to vault fences too.
				//

				if(last_slide_colvect && is_facet_vaultable(last_slide_colvect))
				{
					set_person_vault(p_person,last_slide_colvect);
					return;
					
				}

				if(p_person->Genus.Person->PlayerID)
				if(slide_ladder)
				{
					if(mount_ladder(p_person, slide_ladder))
						return;
				}

				person_normal_animate(p_person);
				if(p_person->Draw.Tweened->FrameIndex==1 || p_person->Draw.Tweened->FrameIndex==5)
				{
					if (p_person->Flags & FLAGS_PLAYED_FOOTSTEP)
					{
						//
						// Don't play twice!
						//
					}
					else
					{
						foot_step_wave = footstep_wave(p_person);

/* //nah were only walking
						PCOM_oscillate_tympanum(
							PCOM_SOUND_FOOTSTEP,
							p_person,
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8);
*/

						foot_step.Flags					=	WAVE_CARTESIAN;
						foot_step.Priority				=	0;
						foot_step.Mode.Cartesian.Scale	=	(128<<8);
						foot_step.Mode.Cartesian.X		=	p_person->WorldPos.X;
						foot_step.Mode.Cartesian.Y		=	p_person->WorldPos.Y;
						foot_step.Mode.Cartesian.Z		=	p_person->WorldPos.Z;

						MFX_play_thing(THING_NUMBER(p_person),foot_step_wave,MFX_REPLACE,p_person);

						p_person->Flags |= FLAGS_PLAYED_FOOTSTEP;
					}
				}
				else
				{
					p_person->Flags &= ~FLAGS_PLAYED_FOOTSTEP;
				}


			break;
		case	SUB_STATE_WALKING_BACKWARDS:
				change_velocity_to(p_person,-16);
				person_normal_move(p_person);
				person_normal_animate(p_person);
				//person_backwards_animate(p_person);
			break;
/*
		case	SUB_STATE_SNEAKING:
				change_velocity_to(p_person,16);
				person_normal_move(p_person);
				person_normal_animate(p_person);
			break;
*/
		case	SUB_STATE_RUN_STOP:
				person_normal_move(p_person);
				end=person_normal_animate(p_person);
				change_velocity_to(p_person,0);

				if(end)
				{
     				p_person->SubState=SUB_STATE_STOPPING;
				}
			break;
		case	SUB_STATE_RUNNING_SKID_STOP:
				person_normal_move(p_person);
				if(check_on_slippy_slope(p_person))
					return;
				if(slope_ahead(p_person,50))
				{
					return;

				}

				end=person_normal_animate(p_person);
				switch(p_person->Draw.Tweened->CurrentAnim)
				{
					case	ANIM_SLIDER_START:
						if(p_person->Velocity>42)
							p_person->Velocity=42;
//						change_velocity_to_slow(p_person,40);
						if(end)
						{
							set_anim(p_person,ANIM_SLIDER_HOLD);
						}
						break;
					case	ANIM_SLIDER_HOLD:

						{
							SLONG fx,fy,fz,px,pz,rgb,sz,i,j;
							calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,
								SUB_OBJECT_LEFT_FOOT, &fx, &fy, &fz);
							fx=(fx<<8) + p_person->WorldPos.X;
							fy=(fy<<8) + p_person->WorldPos.Y;
							fz=(fz<<8) + p_person->WorldPos.Z;
#ifdef	PSX
							rgb=0x20505050;
#else
							rgb=0xFF-(p_person->Velocity<<2);
							if (rgb<0) rgb=0;
							rgb=(rgb<<24)|0xc9b7a3;
							j=(p_person->Velocity>40) ? 7 : 3;
							for (i=0;i<j;i++) 
#endif
							{
							  px=fx+(((Random()&0x7ff)-0x3ff)<<2);
							  pz=fz+(((Random()&0x7ff)-0x3ff)<<2);
							  PARTICLE_Add(px,fy,pz,(Random()&7)-3,20,(Random()&7)-3,POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),rgb,PFLAG_FADE|PFLAG_RESIZE|PFLAG_SPRITEANI|PFLAG_SPRITELOOP,40,25,1,10,4);

							}
						}
						change_velocity_to_slow(p_person,0);
						if(p_person->Velocity<25)
						{
							set_anim(p_person,ANIM_SLIDER_END);
						}
						break;
					case	ANIM_SLIDER_END:
						MFX_stop(THING_NUMBER(p_person),S_SLIDE_START);
						if (p_person->Velocity>5)
						{
							SLONG px,py,pz,rgb,sz;
							calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,
								SUB_OBJECT_LEFT_FOOT, &px, &py, &pz);
							px=(px<<8) + p_person->WorldPos.X;
							py=(py<<8) + p_person->WorldPos.Y;
							pz=(pz<<8) + p_person->WorldPos.Z;
#ifdef	PSX
							rgb=0x20505050;
#else
							rgb=0xFF-(p_person->Velocity<<2);
							if (rgb<0) rgb=0;
							rgb=(rgb<<24)|0xc9b7a3;
#endif
							PARTICLE_Add(px,py,pz,(Random()&7)-3,20,(Random()&7)-3,POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),rgb,PFLAG_FADE|PFLAG_RESIZE|PFLAG_SPRITEANI|PFLAG_SPRITELOOP,40,25,1,10,4);
						}
						change_velocity_to(p_person,0);
						if(end)
						{
							set_person_locked_idle_ready(p_person);
							p_person->Velocity=1;

							person_normal_move(p_person);

						}
						break;
				}

				//
				// Put some leeway in the jump frames so she jumps
				// more responsively.
				//
/*
				if (WITHIN(p_person->Draw.Tweened->FrameIndex, 0, 0))
				{
					set_person_running_stop(p_person,0);
					p_person->SubState=SUB_STATE_STOPPING_OT;
				}
				else
				if (WITHIN(p_person->Draw.Tweened->FrameIndex, 3, 3))
				{
					// jump frame found
					set_person_running_stop(p_person,1);
				}
*/

			break;

		case	SUB_STATE_RUNNING_THEN_JUMP:
			//LogText(" running \n");
				
				if(check_on_slippy_slope(p_person))
					return;
				change_velocity_to(p_person,32);
				person_normal_move(p_person);
				person_normal_animate(p_person);

				//
				// Put some leeway in the jump frames so she jumps
				// more responsively.
				//

				if (WITHIN(p_person->Draw.Tweened->FrameIndex, 0, 1))
				{
					set_person_running_jump_lr(p_person,0);
				}
				else
				if (WITHIN(p_person->Draw.Tweened->FrameIndex, 3, 4))
				{
					// jump frame found
					set_person_running_jump_lr(p_person,1);
				}

				/*
				if (WITHIN(p_person->Draw.Tweened->FrameIndex, 3, 3))
				{
					set_person_running_jump_lr(p_person,2);
				}
				else
				if (WITHIN(p_person->Draw.Tweened->FrameIndex, 7, 7))
				{
					// jump frame found
					set_person_running_jump_lr(p_person,3);
				}
				*/

			break;

		case	SUB_STATE_STOPPING:
		case	SUB_STATE_STOPPING_DEAD:
				if(check_on_slippy_slope(p_person))
					return;
				end=person_normal_animate(p_person);
				change_velocity_to(p_person,0);

				if(p_person->Velocity<5)
				{
//					set_person_locked_idle_ready(p_person);
					set_person_idle(p_person);
					plant_feet(p_person);
				}

			break;
			break;


		case	SUB_STATE_STOPPING_OT:
//		case	SUB_STATE_STOPPING:
				end=person_normal_animate(p_person);
				//trickle_velocity_to(p_person,0);
				//change_velocity_to(p_person,0);

				if( (p_person->Draw.Tweened->FrameIndex<2))
					person_normal_move(p_person);
				else
				if( (p_person->Draw.Tweened->FrameIndex==2))
				{
					ASSERT(0);
					if(p_person->SubState==SUB_STATE_STOPPING_OT)
						p_person->Draw.Tweened->Locked=SUB_OBJECT_RIGHT_FOOT;
					else
						p_person->Draw.Tweened->Locked=SUB_OBJECT_LEFT_FOOT;
					p_person->Velocity=0;
				}
/*
			MSG_add("stopping vel %d",p_person->Velocity);
				if( (p_person->Draw.Tweened->FrameIndex==0))
					end=person_normal_animate(p_person);
				else
				if( (p_person->Draw.Tweened->FrameIndex==1))
					end=person_normal_animate_tween(p_person,64);
				else
					end=person_normal_animate(p_person);

				if( (p_person->Draw.Tweened->FrameIndex<2))
					trickle_velocity_to(p_person,0);
				else
					change_velocity_to(p_person,0);
				//	p_person->Velocity=0;

					person_normal_move(p_person);

				if(end==1)
					p_person->Velocity=0;
				else
					trickle_velocity_to(p_person,0);

//					change_velocity_to(p_person,0);
*/
				if(end) //p_person->Velocity<1)
				{
					set_person_locked_idle_ready(p_person);
					plant_feet(p_person);
					p_person->Draw.Tweened->Locked=0; //SUB_OBJECT_LEFT_FOOT;
				}
			break;
		case	SUB_STATE_HOP_BACK:

				//change_velocity_to(p_person,-10);
				//person_normal_move(p_person);
				//end			=	person_backwards_animate(p_person);

				end=person_normal_animate(p_person);
				person_normal_animate(p_person);
				if(end==1&&p_person->SubState==SUB_STATE_HOP_BACK)
					set_person_locked_idle_ready(p_person);

			break;
		case	SUB_STATE_STEP_LEFT:
				end=person_normal_animate(p_person);

				if(end==1)
				{
					set_person_locked_idle_ready(p_person);
				}

			break;
		case	SUB_STATE_STEP_RIGHT:
				end=person_normal_animate(p_person);
				if(end==1)
				{
					set_person_locked_idle_ready(p_person);
				}
			break;
		case	SUB_STATE_FLIPING:
				if(p_person->Draw.Tweened->FrameIndex>5)
//					set_thing_velocity(p_person,24);
					change_velocity_to_slow(p_person,20);
				else
				if(p_person->Draw.Tweened->FrameIndex>1)
					set_thing_velocity(p_person,50);
				//	change_velocity_to(p_person,40);


				if(p_person->Draw.Tweened->FrameIndex<11)
				{
					SLONG dx;
					SLONG dz;
					SLONG	angle;
					angle=p_person->Draw.Tweened->Angle;
					if(p_person->Genus.Person->Action==ACTION_FLIP_LEFT)
					{
						angle+=512;
					}
					else
						angle-=512;

					angle&=2047;
					dx = -(SIN(angle) * p_person->Velocity) >> 8;
					dz = -(COS(angle) * p_person->Velocity) >> 8;
					person_normal_move_dxdz(p_person,dx,dz);

					if(check_on_slippy_slope(p_person))
						return;
				}


				end=person_normal_animate(p_person);
				if(end==1)
				{
/*					
					if(continue_action(p_person))
					{
						switch(p_person->Genus.Person->Action)
						{
							case	ACTION_FLIP_LEFT:
//								set_person_locked_idle_ready(p_person);
								locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_FLIP_LEFT_CONT); //,SUB_OBJECT_LEFT_FOOT);
								p_person->Draw.Tweened->FrameIndex=0;
								if(!foot_hold_available(p_person,100,0,0))
								{
									set_person_locked_idle_ready(p_person);
								}
//								p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_FLIP_LEFT_CONT];
								break;
							case	ACTION_FLIP_RIGHT:
								locked_anim_change(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_FLIP_RIGHT_CONT); //,SUB_OBJECT_LEFT_FOOT);
								p_person->Draw.Tweened->FrameIndex=0;
								if(!foot_hold_available(p_person,-100,0,0))
								{
									set_person_locked_idle_ready(p_person);
								}
//								set_person_locked_idle_ready(p_person);
//								set_anim(p_person,ANIM_FLIP_RIGHT_CONT); //,SUB_OBJECT_LEFT_FOOT);
//								p_person->Draw.Tweened->NextFrame=global_anim_array[p_person->Genus.Person->AnimType][ANIM_FLIP_RIGHT_CONT];
								break;
						}
					}
					else
*/
					set_locked_anim(p_person,ANIM_STAND_READY,SUB_OBJECT_LEFT_FOOT);
					plant_feet(p_person);
					if(p_person->State!=STATE_DANGLING)
						set_person_locked_idle_ready(p_person);

					if(p_person->Genus.Person->PlayerID) 
					{
						p_person->Genus.Person->pcom_colour=20; // 50 game turns of not going into fight mode, pcom_colour reuse GOD SAVE US ALL
						p_person->Genus.Person->Mode = PERSON_MODE_RUN;
					}
					p_person->Velocity=0;
				}
			break;

		//
		// Bike riding substates...
		//

		case SUB_STATE_MOUNTING_BIKE:

			end = person_normal_animate(p_person);

/*
//dog poo
		if(!p_person->Genus.Person->Channel) 
			p_person->Genus.Person->Channel=play_object_wave(p_person->Genus.Person->Channel,p_person,S_BIKE_IDLE,WAVE_PLAY_NO_INTERUPT|WAVE_LOOP);
#ifdef USE_A3D
//		if(p_person->Genus.Person->Channel) 
//			A3DPosition(person->Genus.Person->Channel,p_person->WorldPos.X,p_person->WorldPos.Y,p_person->WorldPos.Z);
#endif
*/
			if (end == 1)
			{
				p_person->SubState = SUB_STATE_RIDING_BIKE;
			}

			break;

		case SUB_STATE_RIDING_BIKE:

//			person_normal_animate(p_person);

			{
				Thing *p_bike = TO_THING(p_person->Genus.Person->InCar);

				//
				// Move to the same position above the bike.
				//

				GameCoord newpos = p_bike->WorldPos;

				move_thing_on_map(p_person,&newpos);

//				newpos.Y += 0x1000;

				//
				// The code to animate the person it all done in the engine nowaday...
				// just before the person is drawn.
				//
			}

			break;

		case SUB_STATE_DISMOUNTING_BIKE:

			end = person_normal_animate(p_person);

			if (end == 1)
			{
				p_person->Genus.Person->Flags &= ~FLAG_PERSON_BIKING;
				p_person->Genus.Person->InCar  =  0;
				p_person->Draw.Tweened->Tilt   = 0;
				p_person->Draw.Tweened->Roll   = 0;

				set_person_idle(p_person);
			}

			break;

		case SUB_STATE_SIMPLE_ANIM:
			
			//
			// A simple animation (maybe a taunt) set of by PCOM_set_person_move_animation()
			//

			end = person_normal_animate(p_person);

			

			if (end == 1)
			{
				if (p_person->Genus.Person->Flags & FLAG_PERSON_NO_RETURN_TO_NORMAL)
				{
					p_person->Genus.Person->Flags &= ~FLAG_PERSON_NO_RETURN_TO_NORMAL;

					p_person->SubState = SUB_STATE_SIMPLE_ANIM_OVER;
				}
				else
				{
					if(p_person->Draw.Tweened->CurrentAnim==ANIM_VALVE_LOOP)
					{
						p_person->Genus.Person->Timer1--;
						if(p_person->Genus.Person->Timer1<=0)
							set_anim(p_person,ANIM_VALVE_END);
					}
					else
					{
						if(p_person->Draw.Tweened->CurrentAnim==ANIM_VALVE_END)
							set_person_locked_idle_ready(p_person);
//							set_person_idle(p_person);
						else
							set_person_idle(p_person);
					}
				}
			}

			break;

		case SUB_STATE_SIMPLE_ANIM_OVER:

			if (p_person->Draw.Tweened->CurrentAnim == ANIM_SIT_DOWN ||
				p_person->Draw.Tweened->CurrentAnim == ANIM_SIT_IDLE)
			{
				//
				// Every once in a while do an idle anim...
				//

				if (p_person->Genus.Person->PlayerID)
				{
					if ((PTIME(p_person) & 0x3) == 0)
					{
						p_person->Genus.Person->Timer1 += 1;

						if (p_person->Genus.Person->Timer1 < 40)
						{
							p_person->Genus.Person->Health += 1;

							if (p_person->Genus.Person->PersonType == PERSON_ROPER)
							{
								SATURATE(p_person->Genus.Person->Health, 0, 400);
							}
							else
							{
								SATURATE(p_person->Genus.Person->Health, 0, 200);
							}
						}
					}
				}


				if (((GAME_TURN + THING_NUMBER(p_person) * 8) & 0x3f) < 4)
				{
					set_person_do_a_simple_anim(p_person, ANIM_SIT_IDLE);

					p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
				}
			}
			else
			if (p_person->Draw.Tweened->CurrentAnim == ANIM_HANDS_UP ||
				p_person->Draw.Tweened->CurrentAnim == ANIM_HANDS_UP_LOOP)
			{
				//
				// Every once in a while do an idle anim...
				//

				if (((GAME_TURN + THING_NUMBER(p_person) * 8) & 0x3f) < 4)
				{
					set_person_do_a_simple_anim(p_person, ANIM_HANDS_UP_LOOP);

					p_person->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
				}
			}
			else
			{
				person_normal_animate(p_person);
			}

			break;

		default:
			MSG_add("MOVEING unknow substate %d \n",p_person->SubState);
			set_person_idle(p_person);
			break;

	}
}

void	set_person_ko_recoil(Thing *p_person,SLONG anim,UBYTE flags)
{
	if(p_person->Draw.Tweened->CurrentAnim==anim && p_person->Draw.Tweened->FrameIndex<5)
	{
//		if(p_person->Draw.Tweened->CurrentFrame!=p_person->Draw.Tweened->NextFrame)
		if(!(p_person->Draw.Tweened->CurrentFrame->Flags&ANIM_FLAG_LAST_FRAME))
			return;
	}
	if(p_person->SubState == SUB_STATE_DYING_GET_UP_AGAIN)
	{
		set_anim(p_person, anim);
		p_person->SubState = SUB_STATE_DYING_KNOCK_DOWN_WAIT;
	}
	else
	{
		SLONG last_anim = p_person->Draw.Tweened->CurrentAnim;

		set_locked_anim(p_person, anim,SUB_OBJECT_PELVIS);

		if (last_anim == ANIM_GRAB_ARM_THROWV)
		{
			//
			// This person was not lying in the direction of
			// his angle. When he played the stomped on anim
			// he flipped to a new angle.  We change his angle here
			// so it doesn't look like he moves.
			//

			// p_person->Draw.Tweened->Angle += 400;

			//
			// MY GOD! This doesn't work- we're going to need a rotate
			// around pelvis function!
			//
		}
	}
}

void	set_person_recoil(Thing *p_person,SLONG anim,UBYTE flags)
{
	if(p_person->SubState == SUB_STATE_DRAW_ITEM)
		return;

	if(p_person->State==STATE_JUMPING)
		return;

	if(p_person->SubState==SUB_STATE_CANNING_RELEASE)
	{
		return;
	}

	if(p_person->SubState == SUB_STATE_DYING_GET_UP_AGAIN)
	{
		return;

	}
	if(p_person->State==STATE_DYING || p_person->State==STATE_DEAD)
	{
		return;
	}

	if(p_person->State==STATE_DANGLING)
	{
		if(p_person->SubState!=SUB_STATE_DROP_DOWN && p_person->SubState!=SUB_STATE_DROP_DOWN_LAND)
		{
			MFX_stop(THING_NUMBER(p_person), S_ZIPWIRE); //just incase we are on zipwire, dont bother with condition it will just bloat code
			set_person_drop_down(p_person,0);
		}
		return;
	}
	if(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING)
	{
		//throw person off bike because he's been hit/shot
		set_person_dead(p_person,0,PERSON_DEATH_TYPE_STAY_ALIVE,0,0);
		return;


	}
	if(p_person->State==STATE_CLIMB_LADDER)
	{
		set_person_drop_down(p_person,0);
		return;
	}
	if(p_person->SubState==SUB_STATE_GRAPPLE_HOLD||p_person->SubState==SUB_STATE_GRAPPLE_ATTACK)
	{
		//
		// take hit while grappleing, hmmmmmm
		//

		//
		// better let go of person we are grappelling
		//


		set_person_fight_idle(TO_THING(p_person->Genus.Person->Target));
//		return;
	}
	if(p_person->SubState==SUB_STATE_GRAPPLE_HELD)
	{
		//
		// don't really have any sensible options here
		//
		return;
	}

	if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		emergency_uncarry(p_person);
	}

	if(p_person->State==STATE_HIT_RECOIL && p_person->Draw.Tweened->CurrentAnim==anim && p_person->Draw.Tweened->FrameIndex<2)
	{
		return;
	}


	p_person->OldState = p_person->State;
	set_anim(p_person, anim); //,SUB_OBJECT_PELVIS);
	p_person->SubState = 0;
	set_generic_person_state_function(p_person, STATE_HIT_RECOIL);

	//
	// Remember the state this person was in before we recoil them,
	// so that at the end of the recoil we can try to carry on what
	// we were doing.
	//

//	p_person->Genus.Person->Mode = PERSON_MODE_FIGHT;

	//
	// Everything else sets these flags so why dont we...
	//

//	p_person->Genus.Person->Flags|=(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
}

void	fn_person_recoil(Thing *p_person)
{
	SLONG end;

	SlideSoundCheck(p_person);

	end = person_normal_animate(p_person);
	switch(p_person->Draw.Tweened->CurrentAnim)
	{
		case	ANIM_KICK_NAD_TAKE:
			if(end)
				set_anim(p_person,ANIM_KICK_NAD_STUNNED);
			return;
			break;
		case	ANIM_KICK_NAD_STUNNED:
			if(end)
				set_anim(p_person,ANIM_KICK_NAD_RECOVER);
			return;
		case	ANIM_KICK_NAD_RECOVER:
			if(end)
				goto	finish;
			return;
	}

	if(p_person->Draw.Tweened->FrameIndex==0)
	{
		SLONG	old_velocity;
		//
		// little nudge back
		//
		old_velocity=p_person->Velocity;
		p_person->Velocity=-4;
		person_normal_move(p_person);
		p_person->Velocity=old_velocity;

	}
	
	else
	if(p_person->Draw.Tweened->FrameIndex==1)
	{
		//
		// fair enough you can continue moveing now if you want
		//
		p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
		if(person_has_gun_out(p_person))
		{
			p_person->Genus.Person->Action  =   ACTION_AIM_GUN;
		}
		else
		{
			p_person->Genus.Person->Action=ACTION_IDLE;
		}
	}
	
	


	if (end == 1)
	{
finish:;
		if(p_person->Genus.Person->Mode==PERSON_MODE_FIGHT)
			set_person_fight_idle(p_person);
		else
			set_person_idle(p_person);
		//plant_feet(p_person);
	}
}

#define FALL_DIR_STRANGE_LAND  (-1)
#define FALL_DIR_LAND_ON_FRONT ( 0)
#define FALL_DIR_LAND_ON_BACK  ( 1)

SLONG	find_anim_fall_dir(SLONG	anim)
{
	switch(anim)
	{
		case	ANIM_PLUNGE_BACKWARD://128	
		case	ANIM_KD_FRONT_MID:	//170
		case	ANIM_KD_FRONT_HI:	//171
		case	ANIM_KD_BACK_HI:	//173
		case	ANIM_KD_BACK_LOW:	//174
			return FALL_DIR_LAND_ON_BACK; //land on back

		case	ANIM_PLUNGE_START:
		case	ANIM_PLUNGE_FORWARD://129	
		case	ANIM_KD_FRONT_LOW:	//175
		case	ANIM_KD_BACK_MID:	//172
		case	ANIM_HANDS_UP_LIE:	 //18 civ male
			return FALL_DIR_LAND_ON_FRONT; //land on front

		default:
			return FALL_DIR_STRANGE_LAND; //strange land
	}
}


void	move_away_from_wall(Thing *p_person)
{
	person_normal_move_dxdz(p_person,1,1);
}

void	generate_bonus_item(Thing *p_person)
{
	if(GET_SKILL(p_person)>=12 || ((p_person->Genus.Person->pcom_ai!=PCOM_AI_CIV) && (p_person->Genus.Person->Flags2&FLAG2_PERSON_FAKE_WANDER)&& (Random()&0xff)<34))
//	if( ((p_person->Genus.Person->pcom_ai!=PCOM_AI_CIV) && (p_person->Genus.Person->Flags2&FLAG2_PERSON_FAKE_WANDER)&& (Random()&0xff)<34))
	{
		SLONG	gx,gy,gz;
		SLONG	special=SPECIAL_HEALTH;

		if(!(p_person->Genus.Person->Flags2&FLAG2_PERSON_FAKE_WANDER))
		switch(Random()&3)
		{
			case	0:
				special=SPECIAL_HEALTH;
				break;
			case	1:
				special=SPECIAL_AMMO_PISTOL;

				break;
			case	2:
				special=SPECIAL_AMMO_SHOTGUN;

				break;
			case	3:
				special=SPECIAL_AMMO_AK47;
				break;

		}
		find_nice_place_near_person(
			p_person,
		   &gx,
		   &gy,
		   &gz);

		Thing *p_gun = alloc_special(
							special,
							SPECIAL_SUBSTATE_NONE,
							gx,
							gy,
							gz,
							NULL);
	}
}

SLONG	part_bad(Thing *p_person,SLONG part)
{
	SLONG	track_x,track_y,track_z;
	SLONG	fy,dy;


	if(p_person->Genus.Person->Ware)
		return(0);

	calc_sub_objects_position(
		p_person,
		p_person->Draw.Tweened->AnimTween,
		part,
	   &track_x,
	   &track_y,
	   &track_z);

	track_x+=p_person->WorldPos.X>>8;
	track_y+=p_person->WorldPos.Y>>8;
	track_z+=p_person->WorldPos.Z>>8;

	fy=PAP_calc_map_height_at(track_x,track_z);

	dy=fy-track_y;

	if(abs(dy)>256)
	{
		return(1);
	}
	return(0);


}

void	fn_person_dying(Thing *p_person)
{
	SLONG hit;
	SLONG end;
	SLONG backwards;
	SWORD on_face;
	SLONG	height;

	//
	// Is this person's pelvis is below the ground, then
	// raise him up!
	//

	if (p_person->Genus.Person->Ware)
	{
		//
		// Don't do this inside warehouses!
		//
	}
	else
	{
		SLONG pelvis_x;
		SLONG pelvis_y;
		SLONG pelvis_z;

		calc_sub_objects_position(
			p_person,
			p_person->Draw.Tweened->AnimTween,
			0,	// 0 => pelvis
		   &pelvis_x,
		   &pelvis_y,
		   &pelvis_z);

		pelvis_x += p_person->WorldPos.X >> 8;
		pelvis_y += p_person->WorldPos.Y >> 8;
		pelvis_z += p_person->WorldPos.Z >> 8;

		SLONG ground = PAP_calc_map_height_at(pelvis_x, pelvis_z);

		if (pelvis_y < ground + 8)
		{
			//
			// The pelvis is below ground level. Move the person up.
			//

			if (ground > pelvis_y + 0x80)
			{
				//
				// This is too much to move the person by... something bad
				// could be hapenning!
				//
			}
			else
			{
				p_person->WorldPos.Y += ground + 8 - pelvis_y << 8;
			}
		}
	}

	switch(p_person->SubState)
	{
		case SUB_STATE_DYING_INITIAL_ANI:

			//
			// This person is doing the first bit of their die animation.
			// 

			end = person_normal_animate(p_person);

			
			if(part_bad(p_person,SUB_OBJECT_LEFT_FOOT)||part_bad(p_person,SUB_OBJECT_HEAD))
			{
				p_person->Draw.Tweened->Angle+=100;
				p_person->Draw.Tweened->Angle&=2047;
			}

			if (end == 1)
			{
				backwards = find_anim_fall_dir(p_person->Draw.Tweened->CurrentAnim);

				if (backwards == FALL_DIR_STRANGE_LAND)
				{
					//
					// This is not an animation we recognise- kill them straight away!
					//

					p_person->SubState = SUB_STATE_DYING_ACTUALLY_DIE; //dead
				}

				if ((height=height_above_anything(p_person,SUB_OBJECT_PELVIS,&on_face)) > 8)
				{
					//
					// Pelvis a bit in the air so set person death fall
					//
//					ASSERT(0)

					if (on_face == GRAB_FLOOR)
					{
						on_face = 0;
					}

					p_person->DY       = -(4 << 8);
					p_person->OnFace   = on_face;
					p_person->SubState = SUB_STATE_DYING_PRONE;
					if(height>128)
					{
						switch(backwards)
						{
							case FALL_DIR_LAND_ON_BACK:
								locked_anim_change(p_person,0,ANIM_PLUNGE_BACKWARD);
								break;

							case FALL_DIR_LAND_ON_FRONT:
								locked_anim_change(p_person,0,ANIM_PLUNGE_FORWARD);
								break;

							default:
								ASSERT(0);
								break;
						}

					}

					return;
				}
				else
				{
					MSG_add(" pretty near floor backwards %d\n",backwards);

					//
					// This person is on the floor- so do the final death animation.
					//

//					p_person->WorldPos.Y-=(height-9)<<8; // mistake?
					p_person->WorldPos.Y-=(height)<<8; // mistake?
//					ASSERT(0);

					switch(backwards)
					{
						case FALL_DIR_LAND_ON_BACK:
							locked_anim_change(p_person,0,ANIM_KD_BACK_LAND);
							break;

						case FALL_DIR_LAND_ON_FRONT:
							locked_anim_change(p_person,0,ANIM_KD_FRONT_LAND);
							break;

						default:
							ASSERT(0);
							break;
					}
					drop_current_gun(p_person,0);

					p_person->SubState = SUB_STATE_DYING_FINAL_ANI;

					return;
				}
			}

			break;

		case SUB_STATE_DYING_FINAL_ANI:

			end = person_normal_animate(p_person);

			ASSERT(end == 0 || end == 1);

			if (end == 1)
			{
				if (p_person->Genus.Person->pcom_ai == PCOM_AI_HYPOCHONDRIA ||
					p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
				{
					set_generic_person_state_function(p_person, STATE_DEAD);

					p_person->Genus.Person->Timer1 = 0;
					p_person->SubState             = SUB_STATE_DEAD_INJURED;
				}
				else
				{
					if(p_person->Genus.Person->Flags&FLAG_PERSON_KO)
					{
						p_person->SubState = SUB_STATE_DYING_KNOCK_DOWN; // be unconscious
					}
					else
					{
						p_person->SubState = SUB_STATE_DYING_ACTUALLY_DIE; // go dead
					}
				}
			}

			break;

		case SUB_STATE_DYING_ACTUALLY_DIE:

			if (p_person->Genus.Person->pcom_ai == PCOM_AI_HYPOCHONDRIA ||
				p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
			{
				//
				// These people don't actually die...
				//

				set_generic_person_state_function(p_person, STATE_DEAD);

				p_person->Genus.Person->Timer1 = 0;
				p_person->SubState             = SUB_STATE_DEAD_INJURED;
			}
			else
			{
				//
				// Kill the person.
				//

				generate_bonus_item(p_person);


				set_generic_person_state_function(p_person,STATE_DEAD);

				p_person->Genus.Person->Timer1 = 0;
				p_person->Genus.Person->Action = ACTION_DEAD;

				//
				// The blood that appears under dead people.
				//

				drop_current_gun(p_person,0);

				TRACKS_Bloodpool(p_person);
				TRACKS_Bloodpool(p_person);

				//
				// A counter for the number of cops killed.
				//

				if (p_person->Genus.Person->PersonType == PERSON_COP)
				{
					extern UBYTE *EWAY_counter;

					if (EWAY_counter[7] < 255)
					{
						EWAY_counter[7] += 1;
					}
				}

				//
				// Drop all weapons
				//

	//			drop_all_items(p_person);
				p_person->SubState=0;
				if(p_person->Genus.Person->pcom_ai!=PCOM_AI_SUICIDE)
				{
					if (is_person_guilty(p_person))
					{
						//
						// killing guilty people decreases the crime rate.
						//

						CRIME_RATE -= 2;

						SATURATE(CRIME_RATE, 0, 100);
					}
					else
					if (p_person->Genus.Person->pcom_ai   == PCOM_AI_CIV &&
						p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
					{
						//
						// Killing wandering civs increases the crime rate.
						//

						CRIME_RATE += 5;

						SATURATE(CRIME_RATE, 0, 100);
					}
				}

				p_person->Genus.Person->Timer1 = 0;
			}

			break;

		case SUB_STATE_DYING_KNOCK_DOWN:

			end = person_normal_animate(p_person);

			if (end == 1)
			{
				//
				// He's going to be ok!
				//

				move_away_from_wall(p_person);
				p_person->SubState             = SUB_STATE_DYING_KNOCK_DOWN_WAIT;
				p_person->Genus.Person->Timer1 = 0;
				drop_current_gun(p_person,0);
			}

			break;

		case SUB_STATE_DYING_KNOCK_DOWN_WAIT:

			//
			// Animate so we can take hits.
			//

			end = person_normal_animate(p_person);

			//
			// The count up to the person getting up again.
			// 

			if(p_person->Genus.Person->PlayerID)
			{
				if((p_person->Genus.Person->Flags&(FLAG_PERSON_REQUEST_BLOCK|FLAG_PERSON_REQUEST_KICK|FLAG_PERSON_REQUEST_PUNCH))) //|| (GAME_TURN&3)==THING_NUMBER(p_person))
				{
					p_person->Genus.Person->Flags&=~(FLAG_PERSON_REQUEST_BLOCK|FLAG_PERSON_REQUEST_KICK|FLAG_PERSON_REQUEST_PUNCH);

					p_person->Genus.Person->Timer1+=20;
				}
				else
					p_person->Genus.Person->Timer1+=5;
			}
			else
			{
				p_person->Genus.Person->Timer1+=2;
//				p_person->Genus.Person->Timer1=0;
			}

			if(p_person->Genus.Person->Timer1++>200-(GET_SKILL(p_person)*8))
			{
				p_person->SubState = SUB_STATE_DYING_GET_UP_AGAIN;

				//
				// Which get up animation shall we use?
				//

				switch(person_is_lying_on_what(p_person))
				{
					case PERSON_ON_HIS_FRONT:
/*
						if(p_person->Genus.Person->PlayerID)
						{
							PANEL_new_text(NULL,4000,"get up anim %d",p_person->Draw.Tweened->CurrentAnim);
						}
*/
						locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_KO_BEHIND_BIG_GU);
						break;

					case PERSON_ON_HIS_BACK:

						if (part_bad(p_person,SUB_OBJECT_LEFT_FOOT))
						{
							locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_QUICK_GET_UP);
						}
						else
						if(part_bad(p_person,SUB_OBJECT_HEAD))
						{
							locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_KO_BACK_GU);
						}
						else
						if (p_person->Genus.Person->PersonType == PERSON_DARCI ||   //&& count_gang(p_person))||
							p_person->Genus.Person->PersonType == PERSON_THUG_RASTA ||
							p_person->Genus.Person->PersonType == PERSON_THUG_RED   ||
							p_person->Genus.Person->PersonType == PERSON_THUG_GREY)
						{
							//
							// A fast acrobatic animation.
							//

							
							locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_QUICK_GET_UP);
						}
						else
						{
/*
							if(p_person->Genus.Person->PlayerID)
							{
								PANEL_new_text(NULL,4000,"get up anim %d",p_person->Draw.Tweened->CurrentAnim);
							}
*/

							locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_KO_BACK_GU);
						}

						break;

					default:
						ASSERT(0);
						break;
				}

			}
		
			break;

		case SUB_STATE_DYING_GET_UP_AGAIN:

			end = person_normal_animate(p_person);

//			if(p_person->Draw.Tweened->FrameIndex>2)

			if (end == 1)
			{
				p_person->Genus.Person->Flags &= ~(FLAG_PERSON_KO | FLAG_PERSON_HELPLESS);

				//
				// He is okay now.
				//

				if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STOMPED_FRONT ||p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STOMPED_BACK)
				{
					locked_anim_change(p_person,SUB_OBJECT_PELVIS,ANIM_KO_BACK_GU);
				}
				else
				{
//					set_person_idle(p_person);
					set_person_locked_idle_ready(p_person);
					plant_feet(p_person);
					if(p_person->State==STATE_DANGLING)
						return;
				}

				if(p_person->Genus.Person->pcom_ai==PCOM_AI_CIV)
				{
					void PCOM_set_person_ai_flee_place(Thing *p_person,SLONG  scary_x,SLONG  scary_z);

					if (p_person->Genus.Person->pcom_bent & PCOM_BENT_ROBOT)
					{
						//
						// Robotic people never run away.
						//
					}
					else
					{
						PCOM_set_person_ai_flee_place(p_person, p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8);
					}
				}
			}

			break;

		case SUB_STATE_DYING_PRONE:
			end = person_normal_animate(p_person);

			//
			// The person is falling in the middle of the dying animation.
			// 

			if (p_person->Velocity > 0)
			{
				hit = projectile_move_thing(p_person,3+4); //+4 sets pelvis flag
			}
			else
			{
				hit = projectile_move_thing(p_person,2+4);
			}

			if (hit == 1 || hit == 100)
			{
				drop_current_gun(p_person,0);

				//
				// The person has hit the floor
				//

				backwards = find_anim_fall_dir(p_person->Draw.Tweened->CurrentAnim);

				switch(backwards)
				{
					case FALL_DIR_STRANGE_LAND:
						ASSERT(0);
						break;

					case FALL_DIR_LAND_ON_FRONT:
						locked_anim_change(p_person,0,ANIM_KD_FRONT_LAND);
						break;

					case FALL_DIR_LAND_ON_BACK:
						locked_anim_change(p_person,0,ANIM_KD_BACK_LAND);
						break;

					default:
						ASSERT(0);
						break;
				}

				//
				// The last animation before the person dies.
				// 

				p_person->SubState = SUB_STATE_DYING_FINAL_ANI;
				{
					SLONG	new_y,on_face;

					if (p_person->Genus.Person->pcom_ai == PCOM_AI_HYPOCHONDRIA)
					{
						//
						// These people always end up on the floor!
						// 

						p_person->WorldPos.Y = PAP_calc_map_height_at(p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8) << 8;
						p_person->OnFace     = NULL;
					}
					else
					{
						on_face = find_face_for_this_pos(p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,&new_y,0,0);
						if(on_face==GRAB_FLOOR)
						{
						}
						else
						if(on_face)
						{
						}
						else
						{
							new_y=PAP_calc_height_at_thing(p_person,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8);
						}

						if(abs((p_person->WorldPos.Y>>8)-new_y)<128)
						{
							p_person->WorldPos.Y=new_y<<8;
						}
						else
						{
	//						ASSERT(0); //Tell MikeD
						}
					}

				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}


DrawTween	dead_tween;
void	init_dead_tween(void)
{
	memset(&dead_tween,0,sizeof(DrawTween));
}

void	fn_person_dead(Thing *p_person)
{
	SLONG	end;
	UWORD	try_respawn=0;
/*
	if(CNET_network_game)
	if(p_person->Genus.Person->PlayerID)
	{
		set_person_alive_alive_o(p_person);
		return;
	}
*/
	SlideSoundCheck(p_person);
	switch(p_person->SubState)
	{

		case	SUB_STATE_DEAD_ARREST_TURN_OVER:
			end=person_normal_animate(p_person);
			if(end)
			{
				SLONG	c0;
				set_anim(p_person,ANIM_ARREST_BE_CUFFED);
				p_person->Draw.Tweened->Angle+=1024;
				p_person->Draw.Tweened->Angle&=2047;

				p_person->SubState=SUB_STATE_DEAD_CUFFED;
				for(c0=0;c0<3;c0++)
				{
					advance_keyframe(p_person->Draw.Tweened);
				}

			}

			break;
		case	SUB_STATE_DEAD_CUFFED:
			end=person_normal_animate(p_person);
			if(end)
			{
				p_person->SubState=SUB_STATE_DEAD_ARRESTED;
//				drop_all_items(p_person);

				if (is_person_guilty(p_person))
				{
					//
					// Arresting guilty people decreases the crime rate.
					//

					CRIME_RATE -= 4;

					SATURATE(CRIME_RATE, 0, 100);
				}
				generate_bonus_item(p_person);

			}

			break;
		case	SUB_STATE_DEAD_ARRESTED:
			try_respawn=1;
			end=person_normal_animate(p_person);
			if(end)
			{
				if(PTIME(p_person)==0) //once every 256 game turns
					set_anim(p_person,ANIM_ARREST_STRUGGLE);


			}
			break;
		case	SUB_STATE_DEAD_RESPAWN:
extern	SLONG	PCOM_do_regen(Thing *p_person);
				PCOM_do_regen(p_person);
			break;

		case	SUB_STATE_PICKUP_CARRY_V:
				end=person_normal_animate(p_person);
				if(end)
				{
					p_person->SubState = SUB_STATE_STAND_CARRY_V;
				}
				break;
		case	SUB_STATE_DROP_CARRY_V:
				end=person_normal_animate(p_person);
				if(end)
				{
					p_person->SubState=SUB_STATE_DEAD_INJURED;
				}
				break;
		case	SUB_STATE_STAND_CARRY_V:
			//
			//
			// If you ever add anything here then my sloppy coding down in default will fuck up
				
			break;
		case	SUB_STATE_CARRY_MOVE_V:
				end=person_normal_animate(p_person);
				break;

		case SUB_STATE_DEAD_INJURED:

			//
			// Do a groan once in a while...
			//

			if ((PTIME(p_person) & 0x1f) == 0 && (Random() & 0xff) < 50)
			{
				if (p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST && (p_person->Flags & FLAGS_IN_VIEW) && person_is_lying_on_what(p_person) == PERSON_ON_HIS_BACK)
				{
					//
					// This person is lying on his back and is in view- he had
					// better not do his anim or he'll visibly flip.
					//
				}
				else
				{
					set_anim(p_person, ANIM_INJURED_LOOP_STRUGGLE);
				}
			}

			person_normal_animate(p_person);

			break;

		default:

			if (p_person->Genus.Person->pcom_ai == PCOM_AI_SUICIDE)
			{
				//
				// Suicide people are important to the level so can't be deleted.
				//
			}
			else
			{
				if(!p_person->Genus.Person->PlayerID)
					try_respawn=1;

				//
				// dont respawn if we are doing something with them ,like searching them
				//
				if(NET_PERSON(0)->Genus.Person->Target==THING_NUMBER(p_person))
					try_respawn=0;

/*
				if(!(p_person->Flags & FLAGS_IN_VIEW))
				{
					p_person->Genus.Person->Timer1 = 0;
				}
*/
			}
			break;
	}


	if (PersonIsMIB(p_person))
	{
		if (p_person->Genus.Person->Timer1 >= 20 * 20 * 5)		// Was 32 * 20 * 5 for the PC, less time for the DC...
		{
			if (p_person->Genus.Person->Timer1 != 0xffff)
			{
				SLONG px;
				SLONG py;
				SLONG pz;

				calc_sub_objects_position(
					p_person,
					p_person->Draw.Tweened->AnimTween,
					SUB_OBJECT_PELVIS,
				   &px,
				   &py,
				   &pz);

				px <<= 8;
				py <<= 8;
				pz <<= 8;

				px += p_person->WorldPos.X;
				py += p_person->WorldPos.Y;
				pz += p_person->WorldPos.Z;

				//
				// Make this person blow up!
				//

/*				POW_create(
					POW_CREATE_LARGE_SEMI,
					px,
					py,
					pz,0,0,0);*/

				//
				// Massive lightning strike
				//

				SPARK_Pinfo p1;
				SPARK_Pinfo p2;
				UBYTE i;

				PARTICLE_Add(px,py,pz,0,0,0,POLY_PAGE_BLOOM1,0,0xffFFFFFF,PFLAG_FADE,40,255,1,-5,0);

				for (i=0;i<20;i++)
				{
					PARTICLE_Add(px+((Random()&0x9ff)-0x4ff),
								 py+((Random()&0x9ff)-0x4ff),
								 pz+((Random()&0x9ff)-0x4ff),
						(Random()&0x1ff)-0xff,256+(Random()&0x3ff),(Random()&0x1ff)-0x7f,
						POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7F7fFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE|PFLAG_RESIZE,
						300,70,1,2,2);
					DIRT_new_sparks(px>>8,py>>8,pz>>8,2);
				}

//				MFX_play_ambient(0,S_THUNDER_END-(Random()&1),0);
				MFX_stop(THING_NUMBER(p_person),S_MIB_LEVITATE);
				MFX_play_thing(THING_NUMBER(p_person),S_MIB_EXPLODE,0,p_person);

				px>>=8; py>>=8; pz>>=8;


				for (i=0;i<10;i++)
				{
					p1.type   = SPARK_TYPE_POINT;
					p1.flag   = 0;
					p1.person = 0;
					p1.limb   = 0;
					p1.x=px+(Random()&0x3f)-0x1f;
					p1.y=py+1000;
					p1.z=pz+(Random()&0x3f)-0x1f;
					p2.type   = SPARK_TYPE_POINT;
					p2.flag   = 0;
					p2.person = 0;
					p2.limb   = 0;
					p2.x=px+(Random()&0x3f)-0x1f;
					p2.y=PAP_calc_map_height_at(px,pz);
					p2.z=pz+(Random()&0x3f)-0x1f;

					SPARK_create(
						&p1,
						&p2,
						25+(Random()&0xf));
					DIRT_new_sparks(p2.x,p2.y,p2.z,2);

				}

				create_shockwave(
					px,
					py,
					pz,
					0x300,
					200,
					p_person);

				DIRT_gust(p_person,px,pz,px+400,pz);

				remove_thing_from_map(p_person);
			}

			p_person->Genus.Person->Timer1 = 0xffff;
		}
		else
		{
			p_person->Genus.Person->Timer1 += 16 * TICK_RATIO >> TICK_SHIFT;
		}
	}
	else
	if(try_respawn)   //MIB's dont respawn
	{
		SLONG	vanish_time;
#ifdef	PSX
#define	TIMER_VANISH	0	
#else
#define	TIMER_VANISH	100
#endif

		vanish_time=TIMER_VANISH;

		if(VIOLENCE==0)
			vanish_time=30;

#ifdef PSX
extern	UBYTE	remove_dead_people;
			if(remove_dead_people)
			{
				goto	remove_them;
			}
#endif

		if( ( p_person->Flags & FLAGS_IN_VIEW) && ( VIOLENCE!=0 ) )
		{
			// Person is still in view, and we're allowed violence,
			// so reset the timer.
			p_person->Genus.Person->InsideRoom = 0;
		}


		if((!(p_person->Flags & FLAGS_IN_VIEW)) ||VIOLENCE==0)
		{
//			if(VIOLENCE==0)
//				p_person->WorldPos.Y-=256;
			p_person->Genus.Person->InsideRoom++; //timer
//#ifndef	PSX
			if(p_person->Genus.Person->InsideRoom>vanish_time)//TIMER_VANISH)
//#endif
			{
#ifdef PSX
remove_them:;
#endif
				if(p_person->Genus.Person->Flags2&FLAG2_PERSON_FAKE_WANDER )
				{
					drop_all_items(p_person,0);
					remove_thing_from_map(p_person);

					p_person->SubState=SUB_STATE_DEAD_RESPAWN;
				}
				else
				if(p_person->Genus.Person->pcom_ai   == PCOM_AI_CIV && p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
				{
					remove_thing_from_map(p_person);
					p_person->SubState=SUB_STATE_STAND_CARRY_V; // do nowt from now on
//					p_person->SubState=SUB_STATE_DEAD_RESPAWN;
				}
				else
				{

					//
					// gone vanished kaput
#ifdef	PSX						//
					drop_all_items(p_person,0);
					remove_thing_from_map(p_person);
						
					p_person->SubState=SUB_STATE_STAND_CARRY_V; // do nowt from now on

					free_draw_tween(p_person->Draw.Tweened);
					p_person->Draw.Tweened=&dead_tween;//&DRAW_TWEENS[0]; // lets see if this crashes it
#else
					if(VIOLENCE==0)
					{
						drop_all_items(p_person,0);
						remove_thing_from_map(p_person);
					}
#endif

				}

				return;
			}
//#endif
		}
		else
		{
			p_person->Genus.Person->InsideRoom=0; //timer

		}

	}

}

SLONG	dist_from_a_to_b(Thing *a,Thing *b)
{
	SLONG	dx,dz,dist;

	dx=(a->WorldPos.X-b->WorldPos.X)>>8;
	dz=(a->WorldPos.Z-b->WorldPos.Z)>>8;

	dx=abs(dx);
	dz=abs(dz);

	dist=QDIST2(dx,dz);
	return(dist);
}


void	player_aim_at_new_person(Thing *p_person,UWORD new_target)
{
	SLONG	pitch;
	Thing	*p_target;

	ASSERT(p_person->Genus.Person->PlayerID);

	p_target=TO_THING(new_target);

	pitch=get_pitch_to_thing_quick(p_person,p_target);
	pitch-=1024;
	pitch&=2047;

	turn_to_face_thing_quick(p_person,p_target);

//extern	SLONG  FC_focus_yaw;

//	FC_focus_yaw=p_person->Draw.Tweened->Angle;
//	FC_position_for_lookaround(pitch);
extern	void	set_look_pitch(SLONG p);
	set_look_pitch(pitch);


	
}

SLONG	get_angle_to_target(Thing *p_person)
{
	Thing *p_target = TO_THING(p_person->Genus.Person->Target);
	SLONG	dx,dz;
	SLONG	angle;

//	if(p_target->Class==CLASS_PERSON)
	{
		dx=(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
		dz=(p_target->WorldPos.Z-p_person->WorldPos.Z)>>8;
		angle	=	(Arctan(dx,-dz)+1024+2048)&2047;
		return(angle);
	}
//	return(-1);
}

SLONG	player_running_aim_gun(Thing *p_person)
{
	SLONG	old_target=p_person->Genus.Person->Target;
	if (p_person->Genus.Person->Target = find_target_new(p_person))
	{
		Thing *p_target = TO_THING(p_person->Genus.Person->Target);

		highlight_gun_target(p_person,p_target);

		p_person->Flags|=FLAGS_PERSON_AIM_AND_RUN;
		p_person->Draw.Tweened->AngleTo = get_angle_to_target(p_person);

		if(old_target!=p_person->Genus.Person->Target)
		{
			if(p_person->Genus.Person->PersonType==PERSON_DARCI)
			{



				p_target=TO_THING(p_person->Genus.Person->Target);

				//
				// You can target bats/barrels/cars now aswell...
				//

				if (p_target->Class == CLASS_PERSON)
				{
					if(might_i_be_a_villain(p_target))
					{
						PCOM_cop_aiming_at_you(p_target,p_person);
					}
				}

			}
		}
	}
	return(0);
}

void twist_darci_body_to_angle(Thing *p_person, SLONG twist)
{
	UWORD ahead;
	UWORD left;
	UWORD right;

	DrawTween *dt = p_person->Draw.Tweened;

	if (twist > 1024)
	{
		twist -= 2048;
	}

	twist -= twist >> 3;

	//
	// The anim to use depends on the gun.
	// 

	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		//
		// The pistol.
		//

		ahead = ANIM_PISTOL_AIM_AHEAD;
		left  = ANIM_POINT_GUN_LEFT;
		right = ANIM_POINT_GUN_RIGHT;
	}
	else
	{
		//
		// Some other type of gun.
		//
/*
		if ((p_person->Genus.Person->PersonType==PERSON_ROPER)&&((p_person->Draw.Tweened->PersonID>>5)==5)) {
			ahead = ANIM_AK_AIM;
			left  = ANIM_AK_AIM_L;
			right = ANIM_AK_AIM_R;
		} else 
*/
		{
			ahead = ANIM_POINT_SHOTGUN_AHEAD;
			left  = ANIM_POINT_SHOTGUN_RIGHT;
			right = ANIM_POINT_SHOTGUN_LEFT;
		}

		//twist = SIGN(twist) * 255;
	}

	if (twist == 0)
	{
		dt->CurrentFrame = global_anim_array[p_person->Genus.Person->AnimType][ahead];
		dt->NextFrame    = global_anim_array[p_person->Genus.Person->AnimType][ahead];
		dt->AnimTween    = 0;
	}
	else
	if (twist < 0)
	{
		if (twist < -255)
		{
			twist = -255;
		}

		dt->CurrentFrame =  global_anim_array[p_person->Genus.Person->AnimType][ahead];
		dt->NextFrame    =  global_anim_array[p_person->Genus.Person->AnimType][left ];
		dt->AnimTween    = -twist;
	}
	else
	{
		if (twist > 255)
		{
			twist = 255;
		}

		dt->CurrentFrame = global_anim_array[p_person->Genus.Person->AnimType][ahead];
		dt->NextFrame    = global_anim_array[p_person->Genus.Person->AnimType][right];
		dt->AnimTween    = twist;
	}
}

SLONG	might_i_be_a_villain(Thing *p_person)
{
	if(p_person->Genus.Person->PersonType==PERSON_DARCI||
		p_person->Genus.Person->PersonType==PERSON_ROPER||
		p_person->Genus.Person->PersonType==PERSON_COP||
		p_person->Genus.Person->PersonType==PERSON_HOSTAGE)
		return(0);
	else
		return(1);

}

SLONG	am_i_a_thug(Thing *p_person)
{
	if(p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||
		p_person->Genus.Person->PersonType==PERSON_THUG_GREY||
		p_person->Genus.Person->PersonType==PERSON_THUG_RED||
		p_person->Genus.Person->PersonType==PERSON_TRAMP)
		return(1);
	else
		return(0);

}


//
// This needs to take into account the type of weapon, sniper is accurate at a distance etc
//
SLONG calc_dist_benefit_to_gun(Thing *p_person,SLONG dist)
{
	SLONG benefit; //in thousandths of a second
	
	

/*
//
// I can't work out how these aim differently at distance
//
	if(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		


	}
	else
	if (p_person->Genus.Person->SpecialUse)
	{
		p_special = TO_THING(p_person->Genus.Person->SpecialUse);
		switch (p_special->Genus.Special->SpecialType)
		{
			case	SPECIAL_SHOTGUN:

				break;
			case	SPECIAL_AK47:


				break;

			default:

				//
				// This isn't a weapon you shoot!
				//

				benefit=0;
				break;
		}
	}
*/

	if(dist<512)
	{
		benefit=(1024+300-(dist<<1));

	}
	else
	{
		benefit=500-(dist>>3);
	}


	if (benefit < 0)
	{
		return 0;
	}
	else
	{
		return benefit * TICK_RATIO >> TICK_SHIFT;
	}
}

extern	SLONG	look_pitch;
void	highlight_gun_target(Thing *p_person,Thing *p_target)
{
	if(p_person->Genus.Person->PlayerID)
	{
		SLONG	angle,dx,dy,dz;

		if(dist_to_target(p_person,p_target)< (10<<8) )
			track_gun_sight(p_target,256);
	//	ASSERT(dist_to_target(p_person,p_target)< (10<<8) );

#define	MAKE_FPM_TRACK_ENEMY	1

#ifdef	MAKE_FPM_TRACK_ENEMY
		if(p_person->Genus.Person->Flags2&FLAG2_PERSON_LOOK)
		{
			//
			// player is in first person look round mode, so take global look_pitch into account
			//

			//
			// look_pitch is a bit odd, its 0 directly infront of you 1900 when looking up and 50 when looking down
			//

			dx = p_target->WorldPos.X - p_person->WorldPos.X >> 8;
			dy = p_target->WorldPos.Y - p_person->WorldPos.Y >> 8;
			dz = p_target->WorldPos.Z - p_person->WorldPos.Z >> 8;

			angle   = Arctan(dx*dx+dz*dz,dy*abs(dy)<<1);
			angle=((-(angle-512))+2048)&2047;


			angle-=look_pitch;
			if(angle>1024)
				angle-=2048;
			if(angle<-1024)
				angle+=2048;

			if(angle<-8)
				look_pitch-=8;
			else
			if(angle>8)
				look_pitch+=8;


		}
#endif



	}
	else
	{
//		track_gun_sight(p_target,p_target->);
	}
	return;
/*
	if (p_person->Genus.Person->PlayerID ||
		p_target->Genus.Person->PlayerID)
	{
		SLONG above;

		//
		// Draw a little mark above our targets head.
		//

		if (p_target->Class == CLASS_PERSON ||
			p_target->Class == CLASS_VEHICLE)
		{
			above = 0xb000;
		}
		else
		{
			//
			// A bat/barrel is not as tall as a person...
			//

			above = 0x4000;
		}

		AENG_world_line(
			p_target->WorldPos.X         >> 8,
			p_target->WorldPos.Y + above >> 8,
			p_target->WorldPos.Z         >> 8,
			16,
			0xff0000,
			p_target->WorldPos.X                  >> 8,
			p_target->WorldPos.Y + above + 0x1000 >> 8,
			p_target->WorldPos.Z                  >> 8,
			0,
			0x330088,
			FALSE);


		{
		}
	}
*/
}

void	fn_person_gun(Thing *p_person)
{
	SLONG	end;
	switch(p_person->SubState)
	{
		case	SUB_STATE_DRAW_GUN:

				end=person_normal_animate(p_person);

				if( (p_person->Draw.Tweened->FrameIndex==3) || (p_person->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)==0 )
				{
					p_person->Draw.Tweened->PersonID&=  ~0xe0;
					p_person->Draw.Tweened->PersonID|= 1<<5;
					p_person->Genus.Person->Flags   |= FLAG_PERSON_GUN_OUT;
				}

				if (end==1)
				{
					set_person_aim(p_person);

					//p_person->SubState=SUB_STATE_AIM_GUN;
					//set_anim(p_person,ANIM_GUN_AIM);
					//p_person->Genus.Person->Action=ACTION_AIM_GUN;

					p_person->Genus.Person->Flags &= ~(FLAG_PERSON_NON_INT_C | FLAG_PERSON_NON_INT_M);
				}

				break;

		case	SUB_STATE_AIM_GUN:

				if (p_person->Genus.Person->PlayerID)
				{
					SLONG twist;
					SLONG	old_target;

					if (p_person->Genus.Person->SpecialUse)
					{
						Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

						//
						// Using a special.
						//

						
						if (p_special->Genus.Special->SpecialType == SPECIAL_AK47)
						{
							p_person->Genus.Person->Timer1+=TICK_TOCK;
							if(p_person->Genus.Person->Timer1>100)  // 1/10th of a second
							if(continue_firing(p_person))
							{
								p_person->Genus.Person->Timer1=0;  // 1/5th of a second

								set_person_shoot(p_person, TRUE);

								return;
							}
						}
					}
					old_target=p_person->Genus.Person->Target;

					if (p_person->Genus.Person->Target = find_target_new(p_person))
					{
						Thing *p_target = TO_THING(p_person->Genus.Person->Target);
						if(p_target->SubState==SUB_STATE_DYING_KNOCK_DOWN_WAIT)
						{
							if(p_target->Genus.Person->pcom_ai==PCOM_AI_CIV)
								p_target->Genus.Person->Timer1=0;

						}

						if(old_target!=p_person->Genus.Person->Target)
						{
							if(p_person->Genus.Person->PersonType==PERSON_DARCI)
							{
								Thing	*p_target;
								p_target=TO_THING(p_person->Genus.Person->Target);


//								if(p_person->Draw.Tweened->DRoll==0) //not spinning
//#ifndef PSX

								//
								// You can target bats/barrels/cars now aswell...
								//

								if (p_target->Class == CLASS_PERSON)
								{
									if(might_i_be_a_villain(p_target))
									{
										PCOM_cop_aiming_at_you(p_target,p_person);
										//
										//
										//
										if ((p_target->State != STATE_DEAD)&&(p_target->State != STATE_DYING))
										{
											SLONG	sample;
											switch(Random()&3)
											{
												case	0:
													sample=S_DARCI_FREEZE_START;
													break;
												case	1:
													sample=S_DARCI_FREEZE_END;
													break;
												case	2:
												case	3:
													sample=S_DARCI_STOP_POLICE;
													break;
											}
											if (IsEnglish)
												MFX_play_thing(THING_NUMBER(p_person),sample,0,p_person);
										}
									}
								}
//#endif
							}
						}

						twist = get_dangle(p_person, p_target);

						twist_darci_body_to_angle(p_person, twist);
					}
					else
					{
						//
						// No target so keep current twist!
						//
					}
				}
				else
				{
					person_normal_animate(p_person);
				}

				if (p_person->Genus.Person->Target)
				{
					Thing *p_target = TO_THING(p_person->Genus.Person->Target);
					highlight_gun_target(p_person,p_target);

				}

				/*

				{
					THING_INDEX	target;
					SLONG	target_good=0;
					end=person_normal_animate(p_person);

					//
					// don't need to do this every game turn
					//

					if (p_person->Genus.Person->PlayerID)
					{
						UWORD old_target;;
						UWORD new_target;

						old_target = p_person->Genus.Person->Target;
						new_target = find_target(p_person);

						if (new_target && old_target != new_target)
						{
							//if(old_target==0)
							{
								player_aim_at_new_person(p_person,new_target);

							}
							//
							// Just targeted a new person.
							// 

							p_person->Genus.Person->Target = new_target;
							p_person->Genus.Person->Gunaim = MAX_GUN_AIM;
							ASSERT(p_person->Genus.Person->Target != THING_NUMBER(p_person));
							ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
						}

						if(new_target==0)
						{
							p_person->Genus.Person->Target=0;
							
						}
						else
						{
							target_good=1;
						}
					}

					if (p_person->Genus.Person->Target)
					{
						Thing *p_target;

						p_target = TO_THING(p_person->Genus.Person->Target);

						//
						// Improve this person's aim the longer he can see his target.
						//

						if (target_good || can_a_see_b(p_person, p_target))
						{
							SLONG	dist,aimratio;

							if (p_person->Genus.Person->PlayerID ||
								p_target->Genus.Person->PlayerID)
							{
								//
								// This shoot-out involves the player- so put it on screen.
								//

								track_gun_sight(p_target,(p_person->Genus.Person->Gunaim*100)/MAX_GUN_AIM);
							}

							//
							// How quickly the aim improves depends on the distance to the target.
							//

							dist     = dist_from_a_to_b(p_person, p_target);
							aimratio = calc_dist_benefit_to_gun(p_person,dist);

							p_person->Genus.Person->Gunaim -= aimratio; // relate this to distance to target

							//
							// If the person is moving/turning then we can't get a good aim...
							//

							{
								SLONG min_gunaim = abs(p_target->Velocity << 5) + abs(p_target->Draw.Tweened->Roll << 4);

								if (p_person->Genus.Person->Gunaim < min_gunaim)
								{
									p_person->Genus.Person->Gunaim = min_gunaim;
								}
							}
						}
						else
						{
							p_person->Genus.Person->Gunaim = MAX_GUN_AIM;
						}
					}
				}
				*/

				break;
		case	SUB_STATE_SHOOT_GUN:

				{
					UWORD anim_speed = 256;
					
					//
					// We want the gun shooting to be faster.
					//

					if (p_person->Genus.Person->SpecialUse == NULL)
					{
						anim_speed = 400;
					}

					end=person_normal_animate_speed(p_person, anim_speed);
				}

				{
					//
					// Let's try some gun smoke
					//
					SLONG px,py,pz, alpha;
/*
					calc_sub_objects_position(
						p_person,
						p_person->Draw.Tweened->AnimTween,
						SUB_OBJECT_RIGHT_HAND,
					   &px,
					   &py,
					   &pz);

					px<<=8; py<<=8; pz<<=8;
					px += p_person->WorldPos.X; 
					py += p_person->WorldPos.Y;
					pz += p_person->WorldPos.Z;
*/
#ifndef BUILD_PSX
					alpha=(0x6f-(p_person->Draw.Tweened->FrameIndex*4));
					if (alpha>=0) 
					{
						alpha<<=24;

						PARTICLE_Add(
							p_person->Genus.Person->GunMuzzle.X,
							p_person->Genus.Person->GunMuzzle.Y,
							p_person->Genus.Person->GunMuzzle.Z,
							(Random()&0xff)-0x7f,0xff,(Random()&0xff)-0x7f,
							POLY_PAGE_SMOKECLOUD2, 2+((Random()&3)<<2),
							alpha|0x00FFFFFF,
							PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE|PFLAG_RESIZE,
							150,28,1,8,1);
					}
#endif
					
				}

				if (end == 1)
				{
					SLONG firing_an_ak = FALSE;
					SLONG fire_length  = 4;
					
					if (!p_person->Genus.Person->PlayerID && p_person->Genus.Person->SpecialUse && TO_THING(p_person->Genus.Person->SpecialUse)->Genus.Special->SpecialType == SPECIAL_AK47)
					{
						//
						// This person is firing the AK47!
						//

						firing_an_ak = TRUE;
					}

					if (PersonIsMIB(p_person))
					{
						firing_an_ak = TRUE;
						fire_length  = 6;
					}

					if (firing_an_ak && p_person->Genus.Person->Timer1 < fire_length)
					{
						SLONG old_timer = p_person->Genus.Person->Timer1;

						set_person_shoot(p_person, TRUE);

						p_person->Genus.Person->Timer1 = old_timer + 1;

						return;
					}
					else
					{
						p_person->Genus.Person->Flags&=~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);
	/*
						if (p_person->Genus.Person->SpecialUse)
						{
							Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

							//
							// Using a special.
							//

							
							if (p_special->Genus.Special->SpecialType == SPECIAL_AK47)
							{
								p_person->Genus.Person->Timer1+=TICK_TOCK;
								if(p_person->Genus.Person->Timer1>200)  // 1/5th of a second
								if(continue_firing(p_person))
								{
									p_person->Genus.Person->Timer1=0;  // 1/5th of a second

									set_person_shoot(p_person, TRUE);

									return;
								}
							}
						}
	*/

						set_person_aim(p_person);
					}
				}

				break;
		case	SUB_STATE_GUN_AWAY:
			break;
		case	SUB_STATE_STOPPING:
			p_person->SubState=SUB_STATE_AIM_GUN;
			p_person->Genus.Person->Action=ACTION_AIM_GUN;
			break;											  

		case SUB_STATE_DRAW_ITEM:

			end = person_normal_animate(p_person);

			if (p_person->Draw.Tweened->FrameIndex == 3 &&
				p_person->Genus.Person->SpecialDraw     &&
				p_person->Genus.Person->SpecialUse != p_person->Genus.Person->SpecialDraw)
			{
				//
				// Use the special we are drawing.
				//

				if (!person_has_special(p_person, p_person->Genus.Person->SpecialDraw))

				p_person->Genus.Person->SpecialUse = p_person->Genus.Person->SpecialDraw;

				//
				// Draw the special in this person's hand.
				//

				set_persons_personid(p_person);
			}

			if (end == 1)
			{
				if(p_person->Genus.Person->PlayerID && continue_moveing(p_person))
				{
					set_person_running(p_person);
					return;
				}
				else
				{
					SLONG anim;
					Thing *p_special;

					if (p_person->Genus.Person->SpecialUse == 0)
					{
						//
						// Gun might have been knocked out of hand while
						// drawing it...
						//

						set_person_idle(p_person);

						return;
					}

					ASSERT(p_person->Genus.Person->SpecialUse);

					p_special = TO_THING(p_person->Genus.Person->SpecialUse);

					switch(p_special->Genus.Special->SpecialType)
					{
						case	SPECIAL_GUN:
							anim=ANIM_PISTOL_AIM_AHEAD;
							break;
						case	SPECIAL_SHOTGUN:
							anim = ANIM_SHOTGUN_AIM;
							break;
						case	SPECIAL_AK47:
	//						if (p_person->Genus.Person->PersonType==PERSON_ROPER) 
	//							anim = ANIM_AK_AIM;
	//						else
							anim = ANIM_SHOTGUN_AIM;
							break;
						default:
							set_person_idle(p_person);
							return;
					}
					p_person->SubState=SUB_STATE_AIM_GUN;
					set_anim(p_person,anim);
					p_person->Genus.Person->Action=ACTION_AIM_GUN;
					p_person->Genus.Person->Flags &= ~(FLAG_PERSON_NON_INT_C | FLAG_PERSON_NON_INT_M);
				}
			}

			break;

		case SUB_STATE_ITEM_AWAY:
			p_person->Genus.Person->SpecialUse = NULL;
			set_persons_personid(p_person);
			set_person_idle(p_person);
			break;
	}
}


#define	COMBO_ACCURACY 300		// Was 190

SLONG	person_new_combat_node(Thing *p_person)
{
	SBYTE	node=0;
	UBYTE	new_node=0;
	UWORD	anim;
	MSG_add(" new node  y %d \n",p_person->WorldPos.Y>>8);
	
	node=p_person->Genus.Person->CombatNode;

	if(node>0)
	if(get_combat_type_for_node(node)!=COMBAT_NONE)
	if(p_person->Genus.Person->PlayerID)
	{
		if(p_person->Genus.Person->pcom_ai_counter>COMBO_ACCURACY)
		{
			//
			// don't continue combo
			//
			p_person->Genus.Person->CombatNode=-p_person->Genus.Person->CombatNode;
			p_person->Genus.Person->Flags&=~(FLAG_PERSON_REQUEST_KICK|FLAG_PERSON_REQUEST_PUNCH);
#ifndef	PSX

			{
//				CBYTE	str[100];
//				sprintf(str,"MISS TIMED COMBO %d GT %d",p_person->Genus.Person->pcom_ai_counter,COMBO_ACCURACY);
//				CONSOLE_text(str);
			}
#endif
		}

		p_person->Genus.Person->pcom_ai_counter=0;

	}

	if(node)
	{
		if(node>0)
		{
			if(!p_person->Genus.Person->PlayerID)
			{
				//
				// non players must now decide wether to continue the combo
				//

				if((Random()&255)<GET_SKILL(p_person)*10)
				{
					switch(node)
					{
						case	1:	
						case	3:	
						case	5:
							p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_PUNCH;
							break;
						case	6:	
						case	8:	
						case	9:
							p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_KICK;
							break;
					}
				}
			}
		}

		if((p_person->Genus.Person->Flags&(FLAG_PERSON_REQUEST_PUNCH|FLAG_PERSON_REQUEST_KICK)) && node>0)
		{
			if(p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_PUNCH) //punch
			{
				new_node=get_anim_and_node_for_action(node,2,&anim);
				p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_PUNCH;
			}
			
			if(!new_node)
			{
				if(p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_KICK) //kick
				{
					p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_KICK;
					new_node=get_anim_and_node_for_action(node,4,&anim);
				}

			}
		}
		else
		{
			//
			// if node is negative then we missed so can't continue the combo
			//
			new_node=get_anim_and_node_for_action(abs(node),1,&anim); //finished
		}


		if(new_node&&anim)
		{
			p_person->Genus.Person->CombatNode=new_node;
			queue_anim(p_person,anim);
			person_normal_animate(p_person); // to get the queued anim into the nextframe field
			MSG_add(" new node  && anim y %d \n",p_person->WorldPos.Y>>8);
			return(1);
		}

	}
	return(0);
}
extern	SLONG	should_i_block(Thing *p_person,Thing *p_agressor,SLONG anim);

void	aim_at_victim(Thing *p_person,SLONG count)
{
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_KICK_COMBO3b)
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_FIGHT_STOMP)
	if(p_person->Genus.Person->Target)
	{
		Thing	*p_target;

		p_target=TO_THING(p_person->Genus.Person->Target);

		if(p_person->Genus.Person->PlayerID)
		{

			if(count_gang(p_person)<=count)
			{
				turn_to_face_thing(p_person,p_target,0);
			}
		}
	}
}

UBYTE	combo_display=0;
void	fn_person_fighting(Thing *p_person)
{
	Thing	*p_target;
	SLONG	end=0;
	//return;
//	MSG_add(" fighting substate %d action %d",p_person->SubState,p_person->Genus.Person->Action);

	SlideSoundCheck(p_person);

	switch(p_person->SubState)
	{
		case	SUB_STATE_WALL_KICK:
				end=person_normal_animate(p_person);
				if(end==1)
				{
					//MSG_add(" finish punch");
					p_person->Draw.Tweened->Angle+=1024;
					p_person->Draw.Tweened->Angle&=2047;

					set_person_fight_idle(p_person);
				}
				break;

		case	SUB_STATE_BLOCK:


				if(p_person->Draw.Tweened->FrameIndex==2)
				{
					if(p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_KICK)
					{
						p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_KICK;
						set_person_leg_sweep(p_person);
						return;

					}

					if(p_person->Genus.Person->PlayerID)
					{
extern	SLONG	continue_blocking(Thing *p_person);
						if(continue_blocking(p_person))
						{
							goto	skip_animate;
						}

					}
				}
				end = person_normal_animate(p_person);
skip_animate:
				if (end==1)
				{
					{
						set_person_fight_idle(p_person);
					}
				}

				break;

		case	SUB_STATE_PUNCH:
//				person_normal_move(p_person);
				//MSG_add(" do punch");
				Thing	*p_target;
				if(p_person->Genus.Person->Target)
				{

					p_target=TO_THING(p_person->Genus.Person->Target);
					aim_at_victim(p_person);
/*
					if(p_person->Genus.Person->PlayerID)
					{

						if(count_gang(p_person)<=1)
						{
							turn_to_face_thing(p_person,TO_THING(p_person->Genus.Person->Target),0);
						}
					}
*/
/*
					if(count_gang(p_person)<=1)
					{
						turn_to_face_thing(p_person,TO_THING(p_person->Genus.Person->Target),0);
					}
*/

					if(p_target->Class==CLASS_PERSON)
					if(p_target->Genus.Person->PlayerID)
					{
						//
						// punch player , so check over and over if they want to block
						//

						if(should_i_block(p_target,p_person,0))
							p_target->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;

					}
				}

				switch(p_person->Draw.Tweened->CurrentAnim)
				{
					case	ANIM_PUNCH_COMBO2:
					//case	ANIM_PUNCH_COMBO3:
						if(p_person->Draw.Tweened->FrameIndex<2)
						{
							p_person->Velocity=6;
							person_normal_move(p_person);
						}

						break;
				}
				if(p_person->Genus.Person->Flags&(FLAG_PERSON_REQUEST_PUNCH|FLAG_PERSON_REQUEST_KICK))
				{
					if(p_person->Genus.Person->PlayerID)
					{
						p_person->Genus.Person->pcom_ai_counter+=TICK_TOCK;
					}

				}
				end=person_normal_animate(p_person);

				if(0)
				if(p_person->Genus.Person->PlayerID)
				{
					SLONG	index1=0,index2;
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_PUNCH_COMBO1:
							index1=3;
							index2=4;
							break;
						case	ANIM_PUNCH_COMBO2:
							index1=3;
							index2=4;
							break;
//						case	ANIM_PUNCH_COMBO3:
//							index=4;
//							break;
					}

					if(index1)
						if(p_person->Draw.Tweened->FrameIndex>=index1&&p_person->Draw.Tweened->FrameIndex<=index2)
						{
							combo_display=1;
						}
				}
				if(end==1)
				{
					if(person_new_combat_node(p_person))
					{
					}
					else
					{
						MSG_add(" finish punch");
						set_person_fight_idle(p_person);
					}
				}

			break;
		case	SUB_STATE_KICK:
//				change_velocity_to(p_person,16);
//				person_normal_move(p_person);

				if(p_person->Genus.Person->Target)
				{
					aim_at_victim(p_person);
/*
				if(p_person->Genus.Person->PlayerID)
				{

					if(count_gang(p_person)<=1)
					{
						turn_to_face_thing(p_person,TO_THING(p_person->Genus.Person->Target),0);
					}
				}
*/
/*
					if(count_gang(p_person)<=1)
					{
						turn_to_face_thing(p_person,TO_THING(p_person->Genus.Person->Target),0);
					}
*/
					p_target=TO_THING(p_person->Genus.Person->Target);
					if(p_target->Class==CLASS_PERSON)
					if(p_target->Genus.Person->PlayerID)
					{
						//
						// punch player , so check over and over if they want to block
						//

						if(should_i_block(p_target,p_person,0))
							p_target->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;

					}
				}


				if(p_person->Genus.Person->Flags&(FLAG_PERSON_REQUEST_KICK|FLAG_PERSON_REQUEST_PUNCH))
				{
					if(p_person->Genus.Person->PlayerID)
					{
						p_person->Genus.Person->pcom_ai_counter+=TICK_TOCK;
					}

				}
				end=person_normal_animate(p_person);

				if(0)
				if(p_person->Genus.Person->PlayerID)
				{
					SLONG	index1=0,index2;
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_KICK_COMBO1:
							index1=3;
							index2=4;
							break;
						case	ANIM_KICK_COMBO2:
							index1=1;
							index2=4;
							break;
					}

					if(index1)
						if(p_person->Draw.Tweened->FrameIndex>=index1&&p_person->Draw.Tweened->FrameIndex<=index2)
						{
							combo_display=2;
						}
				}

				if(end==1)
				{
					if(person_new_combat_node(p_person))
					{
					}
					else
					{
						set_person_fight_idle(p_person);
					}
				}
			break;
		case	SUB_STATE_STRANGLE:
		case	SUB_STATE_HEADBUTT:

				{
					static UBYTE last_frame = 0;

					end=person_normal_animate_speed(p_person,256);

					if (last_frame != p_person->Draw.Tweened->FrameIndex)
					{
						last_frame  = p_person->Draw.Tweened->FrameIndex;

						Thing *p_target = TO_THING(p_person->Genus.Person->Target);

						ASSERT(p_target->Class == CLASS_PERSON);

						//PANEL_new_text(NULL, 400, "Frame = %d", last_frame);

						//
						// Damage our victim.
						//

						if ((p_target->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL) && !p_person->Genus.Person->PlayerID)
						{
							//
							// Only the player can hurt this person
							//
						}
						else
						if (p_target->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
						{
							//
							// Nothing hurts this person.
							//
						}
						else
						{
							if (p_person->SubState == SUB_STATE_STRANGLE)
							{
								if (last_frame == 27 || ((last_frame & 1) == 0 && last_frame >= 14 && last_frame <= 24))
								{
									SLONG damage;

									//
									// A strangulation frame.
									//

									damage  = 40 - GET_SKILL(p_target) >> 3;
									damage += 1;

									p_target->Genus.Person->Health -= damage;
								}
							}
							else
							{
								if (last_frame == 11)
								{
									//
									// This is the headbutting frame!
									//

									Thing *p_target = TO_THING(p_person->Genus.Person->Target);
									SWORD hit_wave;

									ASSERT(p_target->Class == CLASS_PERSON);

									p_target->Genus.Person->Health -= 40 - GET_SKILL(p_target);

									if (SOUND_Gender(p_target)==1)
										hit_wave = SOUND_Range(S_BAT_MALE_START,S_BAT_MALE_END);
									else
										hit_wave = SOUND_Range(S_BAT_FEMALE_START,S_BAT_FEMALE_END);
									MFX_play_thing(THING_NUMBER(p_target),hit_wave,0,p_target);

								}
							}
						}
					}

					if(end)
					{
						set_person_fight_idle(p_person);
					}
				}

			break;
		case	SUB_STATE_HEADBUTTV:
		case	SUB_STATE_STRANGLEV:

				end=person_normal_animate_speed(p_person,256);

				if(end)
				{
					if (p_person->Genus.Person->Health <= 0)
					{
						//
						// This person is dead- make him die using the same
						// bit of code that everyone else uses.
						//

						remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));

						set_generic_person_state_function(p_person, STATE_DYING);

						p_person->SubState = SUB_STATE_DYING_ACTUALLY_DIE;
					}
					else
					{
						//
						// This person is alive.
						//

						set_person_dead(
							p_person,
							0,
							PERSON_DEATH_TYPE_STAY_ALIVE_PRONE,
							0,
							0);
					}
				}
			break;


		case	SUB_STATE_GRAPPLE:
				end=person_normal_animate_speed(p_person,256);
				if((p_person->Draw.Tweened->FrameIndex==8)&&(p_person->Draw.Tweened->CurrentAnim==ANIM_NECK_SNAP))
//					play_quick_wave(p_person,S_NECK_BREAK,WAVE_PLAY_NO_INTERUPT);
					MFX_play_thing(THING_NUMBER(p_person),S_NECK_BREAK,0,p_person);

				if(end==1)
				{
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_GRAB_ARM:
						case	ANIM_GRAB_ARM_KNEE1:
							p_person->SubState=SUB_STATE_GRAPPLE_HOLD;
							break;
						default:
							set_person_fight_idle(p_person);
							break;

					}
				}

			break;
		case	SUB_STATE_GRAPPLEE:
				if (  (p_person->Draw.Tweened->CurrentAnim==ANIM_PISTOL_WHIP_TAKE)
					&&(MagicFrameCheck(p_person,4))
				   )
						MFX_play_thing(THING_NUMBER(p_person),S_JUDO_CHOP,0,p_person);
				end=person_normal_animate_speed(p_person,256);
				if(end==1)
				{
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_GRAB_ARMV:
							p_person->SubState=SUB_STATE_GRAPPLE_HELD;
							break;
						case	ANIM_PISTOL_WHIP_TAKE:
							set_person_dead(p_person,TO_THING(p_person->Genus.Person->Target),PERSON_DEATH_TYPE_STAY_ALIVE_PRONE,0,0);
							break;
						default:
							//
							// knee to death
							//
//							ASSERT(0);
							// kneck snap
/*
							if(p_person->Genus.Person->Target)
							{
								remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
							}

							set_generic_person_state_function(p_person,STATE_DEAD);
							p_person->Genus.Person->Action = ACTION_DEAD;
							p_person->Genus.Person->Timer1 = 0;
							p_person->SubState=0;
*/
							set_person_dead(p_person,TO_THING(p_person->Genus.Person->Target),PERSON_DEATH_TYPE_PRONE,0,0);
							break;

					}
				}
			break;
		case	SUB_STATE_GRAPPLE_ATTACK:	
				end=person_normal_animate(p_person);
				if(end==1)
				{
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_GRAB_ARM_THROW: //victim dead
						case	ANIM_GRAB_ARM_KNEE2: //victim dead
							set_person_fight_idle(p_person);
							break;
						default:
							p_person->SubState=SUB_STATE_GRAPPLE_HOLD;
							break;
					}
				}
				else
				{
					//
					// nobodys escaping during the attack
					//
					//TO_THING(p_person->Genus.Person->Target)->Genus.Person->Escape=0;
				}
				break;

		case	SUB_STATE_GRAPPLE_HOLD:

				{
					ASSERT(p_person->Genus.Person->Target);

					Thing *p_target = TO_THING(p_person->Genus.Person->Target);

					ASSERT(p_target->Class == CLASS_PERSON);

					if(p_person->Genus.Person->PlayerID)
					{
						track_enemy(TO_THING(p_person->Genus.Person->Target));
					}

					if (p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_KICK)
					{
						p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_KICK;

						if ((p_target->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL) && !p_person->Genus.Person->PlayerID)
						{
							//
							// Only the player can hurt this person.
							// 
						}
						else
						if (p_target->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST && (p_target->Genus.Person->pcom_ai_other & PCOM_COMBAT_GRAPPLE_ATTACK))
						{
							//
							// This fight test dummy dies through a grapple attack- even when marked
							// as invulnerable.
							//

							p_target->Genus.Person->Health = 0;
						}
						else
						if (p_target->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
						{
							//
							// Nothing hurts this person.
							//
						}
						else
						{
							p_target->Genus.Person->Health -= 45 - GET_SKILL(p_target) >> 1;
#ifdef PSX
							if (p_target->Genus.Person->PlayerID)
								PSX_SetShock(0,128);
#endif
#ifdef TARGET_DC
							if (p_target->Genus.Person->PlayerID)
							{
								Vibrate ( 20.0f, 1.0f, 0.25f );
							}
#endif
//							add_damage_value_thing(p_person,20>>1);
						}

						if (p_target->Genus.Person->Health<=0)
						{
							p_target->Genus.Person->Health=0;
							set_anim(p_person,ANIM_GRAB_ARM_KNEE2);
							set_anim(p_target,ANIM_GRAB_ARM_KNEE2V);

							if (p_person)
							{
								p_person->Genus.Person->Flags2 |= FLAG2_PERSON_IS_MURDERER;
							}
						}
						else
						{
 							set_anim(p_person,ANIM_GRAB_ARM_KNEE1);
							set_anim(p_target,ANIM_GRAB_ARM_KNEE1V);
						}
						p_person->SubState=SUB_STATE_GRAPPLE_ATTACK;

					}
					else
					if(p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_PUNCH)
					{
						p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_PUNCH;
						set_anim(p_person,ANIM_GRAB_ARM_THROW);
						set_anim(TO_THING(p_person->Genus.Person->Target),ANIM_GRAB_ARM_THROWV);
	//					TO_THING(p_person->Genus.Person->Target)->Genus.Person->Escape=0;

						p_person->SubState=SUB_STATE_GRAPPLE_ATTACK;
					}
				}


			break;
		case	SUB_STATE_GRAPPLE_HELD:

				{
					Thing *p_attacker = TO_THING(p_person->Genus.Person->Target);

					if((p_person->Genus.Person->Flags&(FLAG_PERSON_REQUEST_BLOCK|FLAG_PERSON_REQUEST_KICK|FLAG_PERSON_REQUEST_PUNCH)) || (PTIME(p_person)&3)==0)
					{
	/*
	#ifndef	PSX
						CBYTE	str[100];
						sprintf(str," press c on turn %d escape %d\n",GAME_TURN,p_person->Genus.Person->Escape);
						CONSOLE_text(str,8000);
	#endif
	*/
						p_person->Genus.Person->Escape++;
	//					p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_BLOCK;
						p_person->Genus.Person->Flags&=~(FLAG_PERSON_REQUEST_BLOCK|FLAG_PERSON_REQUEST_KICK|FLAG_PERSON_REQUEST_PUNCH);
					}

					if (p_attacker->SubState!=SUB_STATE_GRAPPLE_ATTACK)
					{
						SLONG escape_count = 23;

						if(p_person->Genus.Person->PlayerID)
						{
							escape_count = 15;
						}
						else
						if (p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST && !(p_person->Genus.Person->pcom_ai_other & PCOM_COMBAT_GRAPPLE_ATTACK))
						{
							escape_count = 0;
						}
						else
						{
							escape_count-=(GET_SKILL(p_person)>>1);

//							escape_count=10000; //for debugging grapple
						}

						if 	(p_attacker->SubState != SUB_STATE_GRAPPLE      &&
							 p_attacker->SubState != SUB_STATE_GRAPPLE_HOLD &&
							 p_attacker->SubState != SUB_STATE_GRAPPLE_ATTACK)
						{
							//
							// Our attacker is no longer holding us...
							//

							p_person->Genus.Person->Escape=0;
							p_person->SubState=SUB_STATE_ESCAPE;
							set_anim(p_person,ANIM_GRAB_ARM_ESCAPEV);

							return;
						}
						else
						if (p_person->Genus.Person->Escape > escape_count)
						{
							p_person->Genus.Person->Escape=0;
							set_anim(p_person,ANIM_GRAB_ARM_ESCAPEV);
							p_person->SubState=SUB_STATE_ESCAPE;
							set_anim(p_attacker,ANIM_GRAB_ARM_ESCAPE);
							p_attacker->SubState=SUB_STATE_ESCAPE;
							return;
						}

					}
				}

				end=person_normal_animate(p_person);
				switch(p_person->Draw.Tweened->CurrentAnim)
				{

					case	ANIM_GRAB_ARM_THROWV:

						if (end)
						{
							if (p_person->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST && (p_person->Genus.Person->pcom_ai_other & PCOM_COMBAT_GRAPPLE_ATTACK))
							{
								//
								// This fight test dummy dies through a grapple attack- even when marked
								// as invulnerable.
								//

								p_person->Genus.Person->Health = 0;

								set_person_dead(p_person,TO_THING(p_person->Genus.Person->Target),PERSON_DEATH_TYPE_PRONE,0,0);
							}
							else
							{
								//
								// Just knocked out.
								//

								set_person_dead(p_person,TO_THING(p_person->Genus.Person->Target),PERSON_DEATH_TYPE_STAY_ALIVE_PRONE,0,0);
							}
						}

						if (MagicFrameCheck(p_person,8)) {
//							if (p_person->Genus.Person->PersonType==PERSON_DARCI)
							if (SOUND_Gender(p_person)==2)
							  MFX_play_thing(THING_NUMBER(p_person),S_FEMALE_DIE_2,0,p_person);
							else
							  MFX_play_thing(THING_NUMBER(p_person),S_MALE_DIE_2,0,p_person);
						}

						break;
					case	ANIM_GRAB_ARM_KNEE1V:
						if (MagicFrameCheck(p_person,2)) {
							MFX_play_thing(THING_NUMBER(p_person),S_PUNCH_START+(GAME_TURN&3),0,p_person);
							PainSound(p_person);
						}
						break;
					case	ANIM_GRAB_ARM_KNEE2V:
						if (MagicFrameCheck(p_person,2)) {
							MFX_play_thing(THING_NUMBER(p_person),S_PUNCH_START+(GAME_TURN&3),0,p_person);
							PainSound(p_person);
						}
						if(end)
						{
//							ASSERT(0);
							set_person_dead(p_person,TO_THING(p_person->Genus.Person->Target),PERSON_DEATH_TYPE_PRONE,0,0);
/*
							if(p_person->Genus.Person->Target)
							{
								remove_from_gang_attack(p_person,TO_THING(p_person->Genus.Person->Target));
							}

							set_generic_person_state_function(p_person,STATE_DEAD);
							p_person->Genus.Person->Timer1 = 0;
							p_person->Genus.Person->Action = ACTION_DEAD;
							p_person->SubState=0;
*/
						}
						break;

					default:
						break;

				}
			break;
		case	SUB_STATE_ESCAPE:
				end=person_normal_animate(p_person);
				if(end)
				{
					set_person_idle(p_person);
					if(!p_person->Genus.Person->PlayerID)
					{
						PCOM_attack_happened(p_person,TO_THING(p_person->Genus.Person->Target));
					}

					p_person->Genus.Person->Flags &= ~FLAG_PERSON_HELPLESS;
				}
				break;



		case	SUB_STATE_STEP:
				end=person_normal_animate(p_person);
				plant_feet(p_person); //gasp
				if(p_person->State==STATE_DANGLING)
					return;

				if(end==1)
				{
//					set_person_locked_idle_ready(p_person);
					set_person_fight_idle(p_person);
				}

			break;
		case	SUB_STATE_STEP_FORWARD:


				p_person->Genus.Person->Timer1++;
				if(p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_BLOCK)
				{
					if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STEP_S)
					{
						p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_BLOCK;
						set_person_block(p_person);
						return;
					}
				}
//				change_velocity_to(p_person,30);
//				p_person->Velocity=(p_person->Genus.Person->AnimType==ANIM_TYPE_ROPER) ? 15 : 30; //15:30
				p_person->Velocity= 30; //15:30
				if(p_person->Draw.Tweened->FrameIndex<2)
				{
					SLONG	old_angle;
					old_angle=p_person->Draw.Tweened->Angle;
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_FIGHT_STEP_N:
						case	ANIM_FIGHT_STEP_N_BAT:
								break;
						case	ANIM_FIGHT_STEP_E:
						case	ANIM_FIGHT_STEP_E_BAT:
							p_person->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle-512)&2047;

								break;
						case	ANIM_FIGHT_STEP_W:
						case	ANIM_FIGHT_STEP_W_BAT:
							p_person->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle+512)&2047;
								break;
						case	ANIM_FIGHT_STEP_S:
						case	ANIM_FIGHT_STEP_S_BAT:
							p_person->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle+1024)&2047;
								break;
					}
					person_normal_move(p_person);
					p_person->Draw.Tweened->Angle=old_angle;
				}
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_FIGHT_STEP_N:
						case	ANIM_FIGHT_STEP_N_BAT:
						case	ANIM_FIGHT_STEP_S:
						case	ANIM_FIGHT_STEP_S_BAT:
							aim_at_victim(p_person,1000);
								break;
						default:
							aim_at_victim(p_person,1);
								break;

					}
				end=person_normal_animate(p_person);

				if(end)
				{
					reset_gang_attack(p_person);

					set_person_fight_idle(p_person);

				}
				break;

		default:
			MSG_add("FIGHTING unknow substate %d \n",p_person->SubState);
			break;
	}
}

void	fn_person_wait(Thing *p_person)
{
	/*

	struct	Command	*com;
	SLONG	end;
	end=person_normal_animate(p_person);
	if(end==1)
	{

	}
	com=TO_COMMAND(p_person->Genus.Person->Command);

	switch(com->CommandType)
	{
		case	COM_WAIT_FOR_TRIGGER:
			if(TO_THING(com->Data1)->Genus.Switch->Flags&SWITCH_FLAGS_TRIGGERED)
			{
				advance_person_command(p_person);

			}
			break;
		case	COM_WAIT_FOR_CLIST:
			if(TO_CONLIST(com->Data1)->Flags&LIST_TRUE)
			{
				advance_person_command(p_person);

			}
			else
			{
				if(p_person->Genus.Person->AnimType==4)
				if((GAME_TURN&0x3f)==0)
				{
					queue_anim(p_person,ANIM_DESPAIR);
				}
			}
			break;

	}
	*/
}


SLONG	turn_to_face_thing(Thing *p_person,Thing *p_target,SLONG slow)
{
	SLONG	dx,dz;
	SLONG	angle,pangle;
	SLONG	angle_diff;
	SLONG	dist;
	SLONG	ax,ay,az,bx,by,bz;

	ASSERT(p_person->Genus.Person->Target);
	if(p_target->Class!=CLASS_PERSON)
		return(200);
	ASSERT(p_target->Class==CLASS_PERSON);
	ASSERT(p_target->Draw.Tweened);

//	p_target=TO_THING(p_person->Genus.Person->Target);

//	e_draw_3d_line(p_person->WorldPos.X,0,p_person->WorldPos.Z,p_target->WorldPos.X,0,p_target->WorldPos.Z);
	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&ax,&ay,&az);
	ax+=p_person->WorldPos.X>>8;
	ay+=p_person->WorldPos.Y>>8;
	az+=p_person->WorldPos.Z>>8;
	calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&bx,&by,&bz);
	bx+=p_target->WorldPos.X>>8;
	by+=p_target->WorldPos.Y>>8;
	bz+=p_target->WorldPos.Z>>8;

	dx=bx-ax; //(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
	dz=bz-az;//(p_target->WorldPos.Z-p_person->WorldPos.Z)>>8;

	dist=QDIST2(abs(dx),abs(dz));

	if (dist > 0x30)
	{
		angle	=	(Arctan(dx,-dz)+1024+2048)&2047;
	//	MSG_add(" cop turn to reqd %d ",angle);

		pangle = p_person->Draw.Tweened->Angle;
		angle_diff=angle-pangle;

		if(angle_diff>1024)
			angle_diff-=2048;
		if(angle_diff<-1024)
			angle_diff+=2048;

		angle_diff>>=slow;

		angle=(pangle+angle_diff+2048)&2047;

		p_person->Draw.Tweened->Angle=angle;
	}

	return(dist);
}

void	turn_to_face_thing_quick(Thing *p_person,Thing *p_target)
{
	SLONG	dx,dz;
	SLONG	angle,pangle;
	SLONG	angle_diff;
	SLONG	dist;
	SLONG	ax,ay,az,bx,by,bz;

//	ASSERT(p_person->Genus.Person->Target);
	ASSERT(p_target->Class==CLASS_PERSON);
	ASSERT(p_target->Draw.Tweened);
	ax=p_person->WorldPos.X>>8;
	ay=p_person->WorldPos.Y>>8;
	az=p_person->WorldPos.Z>>8;

	bx=p_target->WorldPos.X>>8;
	by=p_target->WorldPos.Y>>8;
	bz=p_target->WorldPos.Z>>8;

	dx=bx-ax; //(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
	dz=bz-az;//(p_target->WorldPos.Z-p_person->WorldPos.Z)>>8;
	dist=QDIST2(abs(dx),abs(dz));

	if (dist > 0x30)
	{
		angle	=	(Arctan(dx,-dz)+1024+2048)&2047;


		p_person->Draw.Tweened->Angle=angle;
	}

}

SLONG	get_pitch_to_thing_quick(Thing *p_person,Thing *p_target)
{
	SLONG	dx,dy,dz,dxz;
	SLONG	angle,pangle;
	SLONG	angle_diff;
	SLONG	dist;
	SLONG	ax,ay,az,bx,by,bz;

//	ASSERT(p_person->Genus.Person->Target);
	ASSERT(p_target->Class==CLASS_PERSON);
	ASSERT(p_target->Draw.Tweened);
	ax=p_person->WorldPos.X>>8;
	ay=p_person->WorldPos.Y>>8;
	az=p_person->WorldPos.Z>>8;

	bx=p_target->WorldPos.X>>8;
	by=p_target->WorldPos.Y>>8;
	bz=p_target->WorldPos.Z>>8;

	dx=abs(bx-ax); //(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
	dy=(by-ay); //(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
	dz=abs(bz-az);//(p_target->WorldPos.Z-p_person->WorldPos.Z)>>8;

	dxz=QDIST2(dx,dz);

	angle   =  Arctan(dy,dxz);

	return(angle&2047);
}

void set_person_draw_item(Thing *p_person, SLONG special_type)
{
	SLONG  anim;
	Thing *p_special;

	if(p_person->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
	{
		return;
	}

	//
	// Does this person have an item of that type?
	//

	p_special = person_has_special(p_person, special_type);

	if (p_special == NULL)
	{
		//
		// The person cannot draw an item if it doesn't have it!
		//

		return;
	}

	//
	// clear old weapon
	//
	p_person->Genus.Person->SpecialUse = 0;
	p_person->Draw.Tweened->PersonID &= 0x1f;
	p_person->Genus.Person->Flags     &= ~FLAG_PERSON_GUN_OUT;


	set_persons_personid(p_person);

	//
	// Which animation shall we use?
	//

	anim = ANIM_PISTOL_DRAW;
/*
	if (special_type == SPECIAL_SHOTGUN ||
		special_type == SPECIAL_AK47)
	{
		//
		// These are two-handed weapons- so use a different animation.
		//

		anim = ANIM_SHOTGUN_DRAW;
	}*/
	switch (special_type) 
	{
		case SPECIAL_SHOTGUN:
			anim = ANIM_SHOTGUN_DRAW;
			p_person->Genus.Person->Mode = PERSON_MODE_RUN;
			break;
		case SPECIAL_KNIFE:
			anim = ANIM_KNIFE_DRAW;
			break;
		case SPECIAL_BASEBALLBAT:
			anim = ANIM_BAT_DRAW;
			break;
		case SPECIAL_AK47:
			p_person->Genus.Person->Mode = PERSON_MODE_RUN;
//			if (p_person->Genus.Person->PersonType==PERSON_ROPER)
//				anim = ANIM_AK_DRAW;
//			else
				anim = ANIM_SHOTGUN_DRAW;
			break;
	}

	//
	// Stand still and do the draw item anim.
	//

	set_anim                         (p_person, anim);
	set_thing_velocity               (p_person, 0);
	set_generic_person_state_function(p_person, STATE_GUN);

	p_person->SubState                  =  SUB_STATE_DRAW_ITEM;
	p_person->Genus.Person->SpecialDraw =  THING_NUMBER(p_special);
	p_person->Genus.Person->Action      =  ACTION_DRAW_SPECIAL;
	p_person->Genus.Person->Flags      |= FLAG_PERSON_NON_INT_C | FLAG_PERSON_NON_INT_M;
//	p_person->Genus.Person->Flags      |=  FLAG_PERSON_LOCK_ANIM_CHANGE;
}

void set_person_item_away(Thing *p_person)
{
	//
	// No anim for putting away an item?
	//

	p_person->Genus.Person->SpecialUse =  NULL;
	p_person->Genus.Person->Flags     &= ~FLAG_PERSON_GUN_OUT;

	set_persons_personid(p_person);
	set_person_idle     (p_person);
}



void set_face_pos(
		Thing *p_person,
		SLONG  world_x,
		SLONG  world_z)
{
	SLONG dx = world_x - (p_person->WorldPos.X >> 8);
	SLONG dz = world_z - (p_person->WorldPos.Z >> 8);

	SLONG angle;
	
	angle  = calc_angle(dx,dz);
	angle += 1024;				// People walk and face backwards!
	angle &= 2047;

	p_person->Draw.Tweened->Angle = angle;

	return;
}


SLONG set_face_thing(Thing *p_person,Thing *p_target)
{
	SLONG	dx,dz;
	SLONG	angle,pangle;
	SLONG	angle_diff;
	SLONG	dist;

	dx=(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
	dz=(p_target->WorldPos.Z-p_person->WorldPos.Z)>>8;
	angle	=	(Arctan(dx,-dz)+1024+2048)&2047;

	p_person->Draw.Tweened->Angle=angle;

	dist=QDIST2(abs(dx),abs(dz));

	return(dist);
}

void turn_towards_thing(Thing *p_person,Thing *p_target)
{
	SLONG	dx,dz;
	SLONG	angle,dangle;
	SLONG	angle_diff;
	SLONG	dist;

	dx=(p_target->WorldPos.X-p_person->WorldPos.X)>>8;
	dz=(p_target->WorldPos.Z-p_person->WorldPos.Z)>>8;
	angle	=	(Arctan(dx,-dz)+1024+2048)&2047;

	dangle=p_person->Draw.Tweened->Angle-angle;

	if(dangle<-1024)
		dangle+=2048;
	if(dangle>1024)
		dangle-=2048;
	if(dangle<-32)
		p_person->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle+32)&2047;
	else
	if(dangle>32)
		p_person->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle-32)&2047;
}

void	fn_person_stand_up(Thing *p_person)
{
	/*

	SLONG	end;
	end=person_normal_animate(p_person);
	if(end==1)
	{
		struct	Command	*com;
		com=TO_COMMAND(p_person->Genus.Person->Command);

		set_person_walking(p_person);
		set_generic_person_state_function(p_person,STATE_NAVIGATING);
		p_person->Genus.Person->NavIndex=com->Data1;

	}
	*/

}
void	fn_person_fight(Thing *p_person)
{
	Thing	*p_target;
	SLONG	dist;
	SLONG	end;

	p_target=TO_THING(p_person->Genus.Person->Target);
	ASSERT(0);

	/*

	if(p_target->State==STATE_DEAD||p_target->State==STATE_DYING)
	{
		advance_person_command(p_person);
		return;
	}
	*/
	SlideSoundCheck(p_person);

	switch(p_person->Draw.Tweened->CurrentAnim)
	{
		case	ANIM_PUNCH1:
		case	ANIM_PUNCH2:
		case	ANIM_KICK_ROUND1:
		case	ANIM_KICK2:
			dist=turn_to_face_thing(p_person,p_target,0);
			if(dist>170)
			{
				set_anim(p_person,ANIM_WALK);
			}
			else
			{
				end=person_normal_animate(p_person);
				if(end==1)
				{
					queue_anim(p_person,ANIM_WALK);
				}
			}
			break;
		default:

/*
			dist=turn_to_face_thing(p_person,p_target);

			


			if(dist>256*12)
			{
				if(p_person->Draw.Tweened->CurrentAnim!=ANIM_WALK)
					queue_anim(p_person,ANIM_WALK);
				p_person->Genus.Person->Target=0;
			}
			else if(dist>256*2)
			{
				if(p_person->Draw.Tweened->CurrentAnim!=ANIM_WALK)
					queue_anim(p_person,ANIM_WALK);
				change_velocity_to(p_person,32);
				person_normal_move(p_person);
			}
			else if(dist>160)
			{
				if(p_person->Draw.Tweened->CurrentAnim!=ANIM_WALK)
					queue_anim(p_person,ANIM_WALK);
				change_velocity_to(p_person,16);
				person_normal_move(p_person);
			}
			else
			{
				if(dist>100)
				{
					if(dist&1)
						queue_anim(p_person,ANIM_PUNCH1);
					else
						queue_anim(p_person,ANIM_KICK_ROUND1);
				}
				else
					queue_anim(p_person,ANIM_STAND_HIP);
			}

			person_normal_animate(p_person);
*/
			break;
	}

}


void set_person_goto_xz(Thing *p_person, SLONG x, SLONG z, SLONG speed)
{
	SLONG velocity;

	if (p_person->SubState == SUB_STATE_SLIPPING)
	{
		//
		// Wait till you've reached the bottom of the slope...
		// 

		return;
	}

	if (PersonIsMIB(p_person))
	{
		//
		// MIB are too cool to run anywhere.
		//

		speed = PERSON_SPEED_WALK;
	}	

	//
	// Remember where we are going.
	//

	p_person->Genus.Person->GotoSpeed = speed;
	p_person->Genus.Person->GotoX     = x;
	p_person->Genus.Person->GotoZ     = z;

	SLONG dx = abs(x - (p_person->WorldPos.X >> 8));
	SLONG dz = abs(z - (p_person->WorldPos.Z >> 8));

	if (QDIST2(dx,dz) < 0x40)
	{
		//
		// Barely worth moving!
		//

		if (p_person->State != STATE_IDLE &&
			p_person->State != STATE_GUN)
		{
			set_person_idle(p_person);
		}

		return;
	}

	//
	// Correct speed
	//

	switch(speed)
	{
		case PERSON_SPEED_WALK:

			if (p_person->SubState != SUB_STATE_WALKING)
			{
				set_anim_walking(p_person);
			//	set_anim(p_person,ANIM_WALK);

				p_person->SubState             =  SUB_STATE_WALKING;
				p_person->Genus.Person->Action =  ACTION_WALK;
			}

			break;

		default:
			
			//
			// Sometimes speed is not set properly...
			//

			speed = PERSON_SPEED_RUN;

			p_person->Genus.Person->GotoSpeed = speed;

			// FALLTHROUGH!

		case PERSON_SPEED_SPRINT:
		case PERSON_SPEED_RUN:

			if (p_person->SubState != SUB_STATE_RUNNING)
			{
				set_anim_running(p_person);
				//set_anim(p_person,ANIM_RUN);

				p_person->SubState             =  SUB_STATE_RUNNING;
				p_person->Genus.Person->Action =  ACTION_RUN;
			}

			break;

		case PERSON_SPEED_SNEAK:

			if (p_person->SubState != SUB_STATE_SNEAKING)
			{
				set_anim(p_person,ANIM_SNEAK);

				p_person->SubState             =  SUB_STATE_SNEAKING;
				p_person->Genus.Person->Action =  ACTION_WALK;
			}

			break;

		case PERSON_SPEED_YOMP:
			
			if (p_person->SubState != SUB_STATE_RUNNING)
			{
			if(person_has_special(p_person, SPECIAL_BASEBALLBAT))
			{
				set_anim(p_person,ANIM_YOMP_START_BAT);

			}
			else
				if(person_holding_2handed(p_person))
				{
					set_anim(p_person,ANIM_YOMP_START_AK);

				}
				else
				{
					if(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
					{
						set_anim(p_person,ANIM_YOMP_START_PISTOL);

					}
					else
						set_anim(p_person,ANIM_YOMP_START);
				}


				p_person->SubState             =  SUB_STATE_RUNNING;
				p_person->Genus.Person->Action =  ACTION_RUN;
			}

			break;

		case PERSON_SPEED_CRAWL:
			
			if (p_person->SubState != SUB_STATE_CRAWLING)
			{
				locked_anim_change(
					p_person,
					SUB_OBJECT_LEFT_FOOT,
					ANIM_CRAWL); 

				p_person->SubState             = SUB_STATE_CRAWLING;
				p_person->Genus.Person->Action = ACTION_CRAWLING;
			}

			break;
	}

	p_person->Genus.Person->Flags &= ~(FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C);

	//
	// How fast?
	//

	velocity = 50;

	switch(speed)
	{
		case PERSON_SPEED_WALK:
			if(p_person->Genus.Person->PersonType==PERSON_CIV)
			{
				if(!(p_person->Draw.Tweened->MeshID&1))
					velocity=14;
				else
					velocity=10;
			}
			else
				velocity=16;
			break;

		case PERSON_SPEED_SNEAK:
		case PERSON_SPEED_CRAWL:
			velocity = 13;

			break;

		case PERSON_SPEED_YOMP:
			if(p_person->Genus.Person->PersonType==PERSON_THUG_RASTA)
			{
				//
				// can't out yomp a rasta, can out sprint though
				//
//				velocity +=10;
			}
			else
				velocity -= velocity >> 2;
			break;

		case PERSON_SPEED_RUN:
		case PERSON_SPEED_SPRINT:

			if (p_person->Genus.Person->pcom_ai       == PCOM_AI_CIV ||
				p_person->Genus.Person->pcom_ai_state != PCOM_AI_STATE_FOLLOWING)
			{
				//
				// Make civs run slower.
				//

				velocity = 30;
			}
			else
  			if(p_person->Genus.Person->PersonType==PERSON_THUG_RASTA)
			{
				//
				// can't out yomp a rasta, can out sprint though
				//
				velocity +=10;
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	set_thing_velocity(p_person, velocity);

	//
	// Change state.
	//

	set_generic_person_state_function(p_person, STATE_GOTOING);
}


void fn_person_goto(Thing *p_person)
{
	SLONG end;

	SLONG dx;
	SLONG dz;
	SLONG dist;

	SLONG cangle;
	SLONG wangle;
	SLONG dangle;

	if (p_person->SubState == SUB_STATE_RUNNING_VAULT)
	{
		process_a_vaulting_person(p_person);

		return;
	}

	//
	// Animate the person.
	//

	end = person_normal_animate(p_person);

	if (end == 1)
	{
		switch(p_person->Draw.Tweened->CurrentAnim)
		{
			case	ANIM_HIT_WALL:
			case	ANIM_YOMP_START:
			case	ANIM_YOMP_START_PISTOL:
			case	ANIM_YOMP_START_AK:
			case	ANIM_YOMP_START_BAT:
				
				{
					UWORD anim;

					anim=get_yomp_anim(p_person);
					if(anim==COP_ROPER_ANIM_RUN)
					{
						set_anim_of_type(p_person,anim,ANIM_TYPE_ROPER);
					}
					else
						set_anim(p_person,anim);
				}

				break;
		}
/*
		if (p_person->Draw.Tweened->CurrentAnim == ANIM_YOMP_START)
		{
			set_anim(p_person,ANIM_YOMP);
		}
		if (p_person->Draw.Tweened->CurrentAnim == ANIM_YOMP_START_AK)
		{
			set_anim(p_person,ANIM_AK_JOG);
		}
*/
	}

	//
	// Point the person towards where he wants to go.
	//

	dx = p_person->Genus.Person->GotoX - (p_person->WorldPos.X >> 8);
	dz = p_person->Genus.Person->GotoZ - (p_person->WorldPos.Z >> 8);

	cangle  = p_person->Draw.Tweened->Angle;
	wangle  = Arctan(dx,-dz) - 1024;
	wangle &= 2047;
	dangle  = wangle - cangle;
	dist    = abs(dx) + abs(dz);

	if (dist < 0x100)
	{
		p_person->Draw.Tweened->Angle = wangle;
	}
	else
	{
		if (dangle >  1024) {dangle -= 2048;}
		if (dangle < -1024) {dangle += 2048;}

		if (dangle >  500) {dangle =  500;}
		if (dangle < -500) {dangle = -500;}

		p_person->Draw.Tweened->AngleTo = wangle;
		p_person->Draw.Tweened->Angle   = cangle + (dangle >> 1);
		p_person->Draw.Tweened->Angle  &= 2047;
	}

	//
	// Move the person.
	//

	person_normal_move_check(p_person);

	if (p_person->SubState == SUB_STATE_SLIPPING)
	{
		set_generic_person_state_function(p_person, STATE_MOVEING);
		
		return;		
	}

	//
	// Make navigating people able to vault fences because the navigation
	// system ignores these little vaultable fences.
	//

	if (last_slide_colvect && is_facet_vaultable(last_slide_colvect))
	{
		set_person_vault(
			p_person,
			last_slide_colvect);

		return;
	}

	//
	// If this person wants to do a running jump...
	//

	if (p_person->SubState == SUB_STATE_RUNNING_THEN_JUMP)
	{
		switch(p_person->Draw.Tweened->FrameIndex)
		{
			case	0:
			case	1:
				set_person_running_jump_lr(p_person, 1);
				break;
			case	3:
			case	4:
				set_person_running_jump_lr(p_person, 0);
				break;
		}
	}
/*
	AENG_world_line(
		(p_person->WorldPos.X >> 8),
		(p_person->WorldPos.Y >> 8),
		(p_person->WorldPos.Z >> 8),
		0x10,
		0x00ffffff,
		p_person->Genus.Person->GotoX,
		PAP_calc_map_height_at(p_person->Genus.Person->GotoX, p_person->Genus.Person->GotoZ),
		p_person->Genus.Person->GotoZ,
		0x0,
		0x00ff0000,
		TRUE);
*/
}

#ifdef	UNUSED
SLONG	hit_enemy(Thing *p_person,SLONG dx,SLONG dz)
{

	SLONG	strike;
//	return(0);
//	if(GAME_TURN&1)
	strike=set_person_punch_if_can(p_person);
	if(strike)
	{
		MSG_add(" apply hit \n");
	}
	p_person->StateFn	=	fn_person_mavigate;
//	set_generic_person_just_function(p_person,STATE_MAVIGATING);
	return(strike);
}
#endif
//
// Returns TRUE if you've reached your destination.
//

SLONG process_person_goto_xz(Thing *p_person,SLONG x,SLONG z,SLONG dist)
{
	ASSERT(0);

	return 0;
}

void fn_person_navigate(Thing *p_person)
{
	ASSERT(0);
}

void init_person_command(Thing *p_person)
{
	ASSERT(0);
}

SLONG mav_arrived(Thing *p_person)
{
	ASSERT(0);

	return 0;
}


SLONG person_mav_again(Thing *p_person)
{
	ASSERT(0);

	return 0;
}

UWORD dir_to_angle[]=
{
	512,
	512+1024,
	0,
	1024
};

void get_dx_dz_for_dir(SLONG dir,SLONG *dx,SLONG *dz)
{
	ASSERT(0);
}

void init_new_mav(Thing *p_person)
{
	ASSERT(0);
}

void fn_person_mavigate_action(Thing *p_person)
{
	ASSERT(0);
}

void fn_person_mavigate(Thing *p_person)
{
	ASSERT(0);
}

void set_person_grappling_hook_pickup(Thing *p_person)
{
	//
	// Do the pickup hook animation.
	//

	set_anim(p_person, ANIM_GRAPPLING_HOOK_PICKUP);

	//
	// Enter the pickup state.
	//

	set_generic_person_state_function(p_person, STATE_GRAPPLING);

	p_person->SubState = SUB_STATE_GRAPPLING_PICKUP;
}



void fn_person_grapple(Thing *p_person)
{
	SLONG end;
#ifndef PSX
	switch(p_person->SubState)
	{
		case SUB_STATE_GRAPPLING_PICKUP:
			
			if (p_person->Draw.Tweened->FrameIndex == 2) // i.e. if you have reached the bottom of the pickup animation
			{
				SLONG px;
				SLONG py;
				SLONG pz;

				//
				// Pickup the grappling hook.
				//

				calc_sub_objects_position(
					p_person,
					p_person->Draw.Tweened->AnimTween,
					SUB_OBJECT_RIGHT_HAND,
				   &px,
				   &py,
				   &pz);

				px += p_person->WorldPos.X >> 8;
				py += p_person->WorldPos.Y >> 8;
				pz += p_person->WorldPos.Z >> 8;

				HOOK_spin(
					px,
					py,
					pz,
					p_person->Draw.Tweened->Angle,
					50);

				//
				// Mark the person as having the grappling hook.
				//

				p_person->Genus.Person->Flags |= FLAG_PERSON_GRAPPLING;
			}

			end = person_normal_animate(p_person);

			if (end)
			{
				//
				// Start spinning it.
				//

				set_person_grapple_windup(p_person);
			}

			break;

		case SUB_STATE_GRAPPLING_WINDUP:

			person_normal_animate_speed(p_person, 256);

			break;

		case SUB_STATE_GRAPPLING_RELEASE:

			//
			// If you have reached the bottom of the pickup animation for
			// the first time.
			//
						
			if (/*p_person->Draw.Tweened->FrameIndex == 2 &&*/ HOOK_get_state() == HOOK_STATE_SPINNING)
			{
				//
				// Release the grappling hook.
				//
	
				HOOK_release();

				//
				// We don't have it anymore.
				//

				p_person->Genus.Person->Flags &= ~FLAG_PERSON_GRAPPLING;
			}

			end = person_normal_animate_speed(p_person, 512);

			if (end)
			{
				//
				// Nothing more to do now.
				//

				set_person_idle(p_person);
			}

			break;

		default:
			ASSERT(0);
			break;
	}
#endif
}



void set_person_mav_to_xz(Thing *p_person,SLONG x,SLONG z)
{
	ASSERT(0);
}

void set_person_mav_to_thing(Thing *p_person,Thing *p_target)
{
	ASSERT(0);
}

#if !defined(TARGET_DC) && !defined(PSX)
SLONG person_is_on_sewer(Thing *p_person) {
	NS_Hi *ns;
	ns=&NS_hi[p_person->WorldPos.X>>(8+PAP_SHIFT_HI)][p_person->WorldPos.Z>>(8+PAP_SHIFT_HI)];
	if (ns->packed&NS_HI_FLAG_GRATE) return PERSON_ON_METAL;
	if (ns->water) return PERSON_ON_SEWATER;
	return PERSON_ON_WATER;
}
#endif

SLONG person_is_on(Thing *p_person)
{
#if !defined(PSX) && !defined(TARGET_DC)
	if (GAME_FLAGS & GF_SEWERS)
	{
		if (p_person->Flags & FLAGS_IN_SEWERS) {
			return person_is_on_sewer(p_person);
		}
	}
#endif
	if (p_person->OnFace)
	{
		//
		// Does this face belong to a prim?
		//

		if (p_person->OnFace > 0)
		{
			if(prim_faces4[p_person->OnFace].FaceFlags&FACE_FLAG_METAL)
			{
				return PERSON_ON_METAL;
			}
//			if (prim_faces4[p_person->OnFace].ThingIndex > 0)
			{
				return PERSON_ON_PRIM;
			}
		}
	}
//	else
	{
		//
		// Standing in a puddle?
		// 
#ifndef PSX
#ifndef TARGET_DC
		if (PUDDLE_in(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Z >> 8))
		{
			return PERSON_ON_WATER;
		}
#endif
#endif
		//
		// Check for special floor textures...
		//

		SLONG mx = p_person->WorldPos.X >> 16;
		SLONG mz = p_person->WorldPos.Z >> 16;

		if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
			WITHIN(mz, 0, MAP_HEIGHT - 1))
		{
#ifdef	PSX
//			page=255;
			return(-255);
#else

			SLONG page = PAP_2HI(mx,mz).Texture & 0x3ff;

			page=SOUND_FXMapping[page];

			if (page==255) page=5;

			if (page==0) return 255; // god this SUCKS SO HARD! sorry

			return -page;
#endif

			//return -SOUND_FXMapping[page];

/*			if (page == 65 ||
				page == 66 ||
				page == 143)
			{
				return PERSON_ON_WOOD;
			}

			if (page >= 69 && page <= 74)
			{
				return PERSON_ON_GRASS;
			}

			if (page == 68 || (page >= 106 && page <= 111))
			{
				return PERSON_ON_GRAVEL;
			}*/
		}
	}

	return PERSON_ON_DUNNO;
}


void set_person_can_pickup(Thing *p_person)
{
	//
	// Do the pickup can animation.
	// 

	set_anim(p_person, ANIM_CAN_PICKUP);

	//
	// Enter the pickup state.
	//

	set_generic_person_state_function(p_person, STATE_CANNING);

	p_person->SubState = SUB_STATE_CANNING_PICKUP;
}

void set_person_can_release(Thing *p_person, SLONG power)
{
	Thing *p_special;

	if (p_person->Genus.Person->Flags & FLAG_PERSON_CANNING)
	{
		goto something_to_throw;
	}

	if (p_person->Genus.Person->SpecialUse)
	{
		p_special = TO_THING(p_person->Genus.Person->SpecialUse);
		
		if (p_special->Genus.Special->SpecialType == SPECIAL_GRENADE)
		{
			//
			// We have a grenade to throw
			//

			goto something_to_throw;
		}
	}

	p_special = person_has_special(p_person, SPECIAL_MINE);

	if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
	{
		//
		// We have an activated mine to throw.
		//

		goto something_to_throw;
	}

	return;

  something_to_throw:;

	//
	// Do the throw can animation.
	// 

	set_anim(p_person, ANIM_CAN_RELEASE);

	//
	// Enter the throw a can state.
	//

	set_generic_person_state_function(p_person, STATE_CANNING);

	p_person->SubState = SUB_STATE_CANNING_RELEASE;
}

/*
void set_person_barrel_pickup(Thing *p_person)
{
	//
	// Do the pickup barrel animation.
	//

	set_anim(p_person, ANIM_LOB2);

	//
	// Enter the throw a can state.
	//

	set_generic_person_state_function(p_person, STATE_CANNING);

	p_person->SubState = SUB_STATE_CANNING_GET_BARREL;
}
*/

void set_person_special_pickup(Thing *p_person)
{
	//
	// Do the pickup can animation.
	//

	set_anim(p_person, ANIM_CAN_PICKUP);

	//
	// Enter the pickup special state.
	//

	set_generic_person_state_function(p_person, STATE_CANNING);

	p_person->SubState = SUB_STATE_CANNING_GET_SPECIAL;
}

void fn_person_can(Thing *p_person)
{
	SLONG end;

	switch(p_person->SubState)
	{
		case SUB_STATE_CANNING_PICKUP:

//			if (p_person->Draw.Tweened->FrameIndex == 4)	// i.e. if you have reached the bottom of the pickup animation

			// cunnnnning
			if (p_person->Draw.Tweened->FrameIndex == (3+p_person->Genus.Person->AnimType))
			{
				if (p_person->Genus.Person->Flags & FLAG_PERSON_CANNING)
				{
					//
					// Already got a can.
					// 
				}
				else
				{
					//
					// Find the position of your right hand.
					//

					MFX_play_thing(THING_NUMBER(p_person),S_PICKUP_SWISH,0,p_person);

					SLONG px;
					SLONG py;
					SLONG pz;

					calc_sub_objects_position(
						p_person,
						p_person->Draw.Tweened->AnimTween,
						SUB_OBJECT_PREFERRED_HAND,
					   &px,
					   &py,
					   &pz);

					px += p_person->WorldPos.X >> 8;
					py += p_person->WorldPos.Y >> 8;
					pz += p_person->WorldPos.Z >> 8;

					//
					// Pickup the nearest coke can or head
					//

					DIRT_pick_up_can_or_head(p_person);
				}
			}

			end = person_normal_animate(p_person);

			if (end)
			{
				//
				// Finished picking-up the coke can or head
				//

				set_person_idle(p_person);
			}

			break;

		case SUB_STATE_CANNING_RELEASE:

			{
				// SLONG when = (p_person->Genus.Person->AnimType==ANIM_TYPE_ROPER) ? 7 : 3;

				if (p_person->Draw.Tweened->FrameIndex == 3) // i.e. reached the release part of the anim.
				{
					if (p_person->Genus.Person->Flags & FLAG_PERSON_CANNING)
					{
						//
						// Throwing a coke can or a head.
						//

						DIRT_release_can_or_head(p_person, 128);
					}
					else
					{
						Thing *p_special;

						//
						// Throwing a grenade or a mine.
						//

						if (p_person->Genus.Person->SpecialUse &&
							TO_THING(p_person->Genus.Person->SpecialUse)->Genus.Special->SpecialType == SPECIAL_GRENADE)
						{
							p_special = TO_THING(p_person->Genus.Person->SpecialUse);

							if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
							{
								SPECIAL_throw_grenade(TO_THING(p_person->Genus.Person->SpecialUse));
							}
						}
						/*

						//
						// You can't throw mines any more.
						// 

						else
						{
							p_special = person_has_special(p_person, SPECIAL_MINE);

							if (p_special)
							{
								if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
								{
									SPECIAL_throw_mine(p_special);
								}
							}
						}

						*/
					}
				}

				end = person_normal_animate(p_person);

				if (end)
				{
					//
					// Finished throwing the can or head.
					//

					set_person_idle(p_person);
				}
			}

			break;

		case SUB_STATE_CANNING_GET_BARREL:

			ASSERT(0);

			/*

			end = person_normal_animate(p_person);

			if (end)
			{
				//
				// Finished throwing the barrel.
				//

				set_person_idle(p_person);

				return;
			}

			if (p_person->Draw.Tweened->FrameIndex == 2)
			{
				if (!(p_person->Genus.Person->Flags & FLAG_PERSON_BARRELING))
				{
					//
					// Find a barrel to pickup.
					//

					p_person->Genus.Person->Hold = THING_find_nearest(
														p_person->WorldPos.X >> 8,
														p_person->WorldPos.Y >> 8,
														p_person->WorldPos.Z >> 8,
														0x80,
														(1 << CLASS_BARREL));

					if (p_person->Genus.Person->Hold == NULL)
					{
						//
						// No barrel to pickup.
						//

						set_person_idle(p_person);
					}
					else
					{
						p_person->Genus.Person->Flags |= FLAG_PERSON_BARRELING;
					}
				}
			}

			if (p_person->Genus.Person->Flags & FLAG_PERSON_BARRELING)

			{
				BARREL_position_on_hands(TO_THING(p_person->Genus.Person->Hold), p_person);
			}

			{
				if (p_person->Draw.Tweened->FrameIndex == 9 || (p_person->Draw.Tweened->FrameIndex == 8 && p_person->Draw.Tweened->AnimTween >= 128))
				{
					

					if (p_person->Genus.Person->Flags & FLAG_PERSON_BARRELING)
					{
						//
						// Release the barrel.
						//

						BARREL_throw(TO_THING(p_person->Genus.Person->Hold));

						p_person->Genus.Person->Flags &= ~FLAG_PERSON_BARRELING;
					}
				}
			}

			*/

			break;

		case SUB_STATE_CANNING_GET_SPECIAL:

			if (p_person->Draw.Tweened->FrameIndex == (3+p_person->Genus.Person->AnimType))
			{
				UWORD  s_index;
				Thing *s_thing;

				MFX_play_thing(THING_NUMBER(p_person),S_PICKUP_SWISH,0,p_person);
				s_index = THING_find_nearest(
									p_person->WorldPos.X >> 8,
									p_person->WorldPos.Y >> 8,
									p_person->WorldPos.Z >> 8,
									0xa0,
									1 << CLASS_SPECIAL);

				if (s_index)
				{
					s_thing = TO_THING(s_index);

					if (should_person_get_item(p_person, s_thing))
					{
						person_get_item(p_person, s_thing);

						//
						// Don't check for specials anymore.
						//

						p_person->SubState = SUB_STATE_CANNING_GOT_SPECIAL;
					}
				}		
				
			}

			//
			// FALLTHROUGH!
			// 

		case SUB_STATE_CANNING_GOT_SPECIAL:

			end = person_normal_animate(p_person);

			if (end)
			{
				//
				// Finished picking up the special.
				//

				set_person_idle(p_person);
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}


void set_person_do_a_simple_anim(Thing *p_person, SLONG anim)
{
	set_anim(p_person, anim);
	set_generic_person_state_function(p_person, STATE_MOVEING);
	p_person->SubState = SUB_STATE_SIMPLE_ANIM;
	p_person->Genus.Person->Flags |= FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;
}



void set_person_recircle(Thing *p_person)
{
	SLONG	anim;
	ASSERT(p_person->Genus.Person->PlayerID==0);

	if (p_person->Genus.Person->Target == NULL)
	{	
		//
		// This person has no target... lets find him one!
		//

		p_person->Genus.Person->Target = PCOM_person_wants_to_kill(p_person);
	}

	if(p_person->SubState==SUB_STATE_BLOCK)
		return;
	if (p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_BLOCK)
	{
		p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_BLOCK;

		//
		// person has decided to do a block so, do a block
		//

		set_person_block(p_person);
		return;
	}


	anim=find_idle_fight_stance(p_person);
	set_anim(p_person, anim);
	p_person->Genus.Person->Timer1=0;
	set_generic_person_state_function(p_person, STATE_CIRCLING);
	if(p_person->Genus.Person->Agression>-10)
		p_person->SubState = SUB_STATE_CIRCLING_CIRCLE;
	else
		p_person->SubState = SUB_STATE_CIRCLING_BACK_OFF;
	p_person->Genus.Person->Mode=PERSON_MODE_FIGHT;
}

void set_person_circle(Thing *p_person, Thing *p_target)
{
	SLONG	dx,dy,dz;
	SLONG	anim;
	//
	// Remember who we are circling.
	//
	ASSERT(p_person->Genus.Person->PlayerID==0);

	p_person->Genus.Person->Target = THING_NUMBER(p_target);
	ASSERT(p_person->Genus.Person->Target !=THING_NUMBER(p_person));
	ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
	p_person->Genus.Person->Timer1=0;
	ASSERT(p_person->Genus.Person->Target != THING_NUMBER(p_person));

	//
	// Do the fighting animation.
	// 
	anim=find_idle_fight_stance(p_person);

	set_anim(p_person, anim);

	//
	// Enter the cirling state.
	//

	set_generic_person_state_function(p_person, STATE_CIRCLING);

	p_person->SubState = SUB_STATE_CIRCLING_BACK_OFF;
	//
	// STACK  collide_against_things/sweep_feet/attack_happened/set_person_ai_kill/pcomset_move_circle
	//
//	calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&dx,&dy,&dz);
	p_person->Genus.Person->TargetX=(p_target->WorldPos.X>>8);//+dx;
	p_person->Genus.Person->TargetZ=(p_target->WorldPos.Z>>8);//+dz;
	p_person->Genus.Person->Agression=-21;
	p_person->Genus.Person->Mode=PERSON_MODE_FIGHT;

}

void	set_person_hug_wall_leap_out(Thing *p_person,SLONG dir)
{
	SLONG	anim;
	p_person->SubState=SUB_STATE_HUG_WALL_LEAP_OUT;
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		anim=ANIM_PRESS_WALL_FREEZE_L_PISTOL;
	}
	else
	if(person_holding_2handed(p_person))
	{
		anim=ANIM_PRESS_WALL_FREEZE_L_AK;
	}
	else
	{
		anim=ANIM_PRESS_WALL_FREEZE_L;
	}
	if(dir==1)
		anim++;
//	set_locked_anim(p_person,anim,SUB_OBJECT_LEFT_FOOT);
	set_anim(p_person,anim);
	p_person->Genus.Person->Action=ACTION_NONE;

}
void	set_person_hug_wall_stand(Thing *p_person,SLONG dangle=0,SLONG locked=1)
{
	SLONG	anim;
	SLONG	dist,dx,dz;
	p_person->SubState=SUB_STATE_HUG_WALL_STAND;
	if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		anim=ANIM_PRESS_WALL_STAND_PISTOL;
	}
	else
	if(person_holding_2handed(p_person))
	{
		anim=ANIM_PRESS_WALL_STAND_AK;
	}
	else
	{
		anim=ANIM_PRESS_WALL_STAND;
	}
	if(!locked)
	{
		set_anim(p_person,anim);
	}
	else
	{
		set_locked_anim_angle(p_person,anim,SUB_OBJECT_PELVIS,dangle);
	}
/*
	if((dist=abs(check_near_facet(p_person,64,64,(p_person->WorldPos.X)>>8,(p_person->WorldPos.Z)>>8)))==0)
	{
		return;
	}
	else
	if(abs(32-dist)<32)
	{
		SLONG	angle;
		angle=(p_person->Draw.Tweened->Angle)&2047;

		dx = -(SIN(angle) * (32-dist)) >> 8;
		dz = -(COS(angle) * (32-dist)) >> 8;
		person_normal_move_dxdz(p_person,dx,dz);

	}
*/


//	set_anim(p_person,anim);

}

//
// returns true if person is within max_dist of a facet
//
extern	SLONG	global_on;
UWORD	near_facet=0;
SLONG	check_near_facet(Thing *p_person,SLONG max_dist,SLONG max_end_dist,SLONG px,SLONG pz)
{
	SLONG	loop=4;
	SLONG	mx,mz;
	SLONG	c0,exit,f_list,i_facet;
	struct	DFacet	*df;
	SLONG	y_top,y_bot,pers_y;
	SLONG	best_dist=99999;
	SLONG	person_north_south=0;

	SUPERMAP_counter_increase(0);
	near_facet=0;
	/*
					AENG_world_line(
						(p_person->WorldPos.X >> 8),
						(p_person->WorldPos.Y >> 8),
						(p_person->WorldPos.Z >> 8),
						3,
						0x00ffffff,
						px,
						(p_person->WorldPos.Y)>>8,
						pz,
						0,
						0x00123456,
						TRUE);
	  */

	{
		SLONG a;
		a=p_person->Draw.Tweened->Angle&2047;
		if(a<200 || a>2048-200 || (a>1024-200 && a<1024+200))
		{
			person_north_south=1;
		}

	}




	pers_y=(p_person->WorldPos.Y>>8)+128;

	while(loop>=0)
	{
		mx=px>>10;
		mz=pz>>10;

		switch(loop)
		{
			case	4:
				break;
			case	3:
				mx++;
				break;
			case	2:
				mx--;
				break;
			case	1:
				mz++;
				break;
			case	0:
				mz--;
				break;
		}
		loop--;
		exit   = FALSE;
		if(mx>=0 && mz>=0 && mx<32 && mz<32)
		{
			f_list = PAP_2LO(mx,mz).ColVectHead;

			if (!f_list)
			{
				//
				// No facets on this square.
				//

				exit=1;
			}

			while(!exit)
			{

				i_facet = facet_links[f_list++];
				ASSERT(i_facet<next_dfacet);

				if (i_facet < 0)
				{
					i_facet = -i_facet;
					exit   =   TRUE;
				}

				df = &dfacets[i_facet];

				//
				// Done this one already?
				// 

				if (df->Counter[0] == SUPERMAP_counter[0])
				{
					//
					// We've done this facet already.
					// 

					continue;
				}

				//
				// Mark this facet as already done.
				//

				df->Counter[0] = SUPERMAP_counter[0];

				if (df->FacetType == STOREY_TYPE_CABLE)
				{
					//
					// Ignore cables in collision.
					//

					continue;
				}

				if (df->FacetType == STOREY_TYPE_OUTSIDE_DOOR && (df->FacetFlags & FACET_FLAG_OPEN))
				{
					//
					// Ignore open doors.
					//

					continue;
				}

				if((person_north_south && (df->x[0]==df->x[1]) ) ||( (!person_north_south) && (df->z[0]==df->z[1]) ) )
				{
					//
					// looking for facets in other plane
					//
					continue;

				}

				//
				// Is the 'y' component in range?
				//
				
				if (df->FacetType == STOREY_TYPE_FENCE_FLAT  ||
					df->FacetType == STOREY_TYPE_FENCE       ||
					df->FacetType == STOREY_TYPE_FENCE_BRICK ||
					df->FacetType == STOREY_TYPE_OUTSIDE_DOOR)
				{
					y_top = get_fence_top   (px , pz, i_facet);
					y_bot = get_fence_bottom(px, pz, i_facet) - 30;

				}
				else
				{
					y_bot = df->Y[0] - 64;
					y_top = df->Y[0] + (df->Height * df->BlockHeight << 2);

					if (df->FHeight)
					{
						//
						// You can never walk under foundations.
						//

						y_bot = -0x7fff;
					}
				}
				if (WITHIN(pers_y, y_bot, y_top))
				{
					SLONG	dist;

					dist=distance_to_line(df->x[0]<<8,df->z[0]<<8,df->x[1]<<8,df->z[1]<<8,px,pz);
	//				df->FacetFlags|=FACET_FLAG_IN_SEWERS;
					if(global_on)
					if(abs(dist)<abs(best_dist))
					{
						best_dist=dist;
						near_facet=i_facet;
					}


				}
			}
		}
//		PANEL_new_text(p_person,100," best_dist %d dist %d",best_dist,max_dist);

		if(abs(best_dist)<max_dist)
		{
			return(best_dist);
		}
/*
		{
			SLONG	angle;
			SLONG	dx,dz,nx,nz;
			angle=(p_person->Draw.Tweened->Angle+1024)&2047;
			dx = -(SIN(angle) * 128) >> 16;
			dz = -(COS(angle) * 128) >> 16;

			nx=(px+dx)>>10;
			nz=(pz+dz)>>10;

			if(nx==mx && nz==mz)
			{
				if(abs(best_dist)<max_dist)
				{
					return(best_dist);
				}
				else
				{
					near_facet=0;
		PANEL_new_text(p_person,3000,"NO HUGb");
					return(0);
				}
			}
			else
			{
				mx=nx;
				mz=nz;

			}
		}
*/

	}

	if(abs(best_dist)<max_dist)
	{
		return(best_dist);
	}
	else
	{
		near_facet=0;
//		PANEL_new_text(p_person,3000,"NO HUG");
		return(0);
	}

}
// return angle+1
SLONG	can_i_hug_wall(Thing *p_person)
{
	if (p_person->OnFace>0)
	{
		ASSERT(WITHIN(p_person->OnFace, 1, next_prim_face4 - 1));

		PrimFace4 *f4 = &prim_faces4[p_person->OnFace];

		ASSERT(f4->FaceFlags & FACE_FLAG_WALKABLE);

		if (f4->FaceFlags & FACE_FLAG_WMOVE)
		{
			return(0); // not while stood on a car etc
		}
	}
	if(p_person->Genus.Person->Ware==0)
	if(check_near_facet(p_person,64,64,p_person->WorldPos.X>>8,p_person->WorldPos.Z>>8))
	{
		SLONG	wall_angle,ft;
		ft=dfacets[near_facet].FacetType;
		if(ft==STOREY_TYPE_NORMAL)
		if(am_i_facing_wall(p_person,near_facet,&wall_angle,200))
		{
			SLONG	wx,wy,wz;
			SLONG	mx,my,mz;
			SLONG	dx,dz;

			wx=p_person->WorldPos.X>>8;
			wy=p_person->WorldPos.Y>>8;
			wz=p_person->WorldPos.Z>>8;

			dx = -(SIN(wall_angle)) >> 9;
			dz = -(COS(wall_angle)) >> 9;

			mx=(wx+dx)>>8;
			mz=(wz+dz)>>8;

			my=MAVHEIGHT(mx,mz)<<6;

//			if(wy<my-196 || wy>my-64)
			if(wy>my-196)
				return(0);

			return((wall_angle&2047)+1);
		}


	}
	return(0);

}

SLONG	move_ok(Thing *p_person,SLONG dx,SLONG dz)
{
	SLONG	y,dy;
	SLONG	nx,nz;

	nx=p_person->WorldPos.X;
	nz=p_person->WorldPos.Z;

	nx+=dx;
	nz+=dz;

	nx>>=8;
	nz>>=8;


	y=MAVHEIGHT(nx>>8,nz>>8)<<6;

	dy=y-(p_person->WorldPos.Y>>8);
	if(abs(dy)>60)
		return(0);

	if(PAP_2HI(nx >> 8, nz >> 8).Flags & PAP_FLAG_NOGO)
	{
		return(0);
	}

	return(1);

}

void fn_person_hug_wall(Thing *p_person)
{
	SLONG	end;
	SLONG dx;
	SLONG dz;
	SLONG	angle;
	SLONG	dist;


	switch(p_person->SubState)
	{
		case	SUB_STATE_HUG_WALL_TURN:
			end = person_normal_animate(p_person);
			if(end)
			{
				set_person_hug_wall_stand(p_person,1024);
//				p_person->Draw.Tweened->Angle+=1024;
//				p_person->Draw.Tweened->Angle&=2047;
			}
			break;
		case	SUB_STATE_HUG_WALL_STAND:

			break;
		case	SUB_STATE_HUG_WALL_STEP_LEFT:
			if(!continue_dir(p_person,1))
			{
				set_person_hug_wall_stand(p_person);
				return;
			}

			angle=(p_person->Draw.Tweened->Angle+512)&2047;

			dx = -(SIN(angle) * 5) >> 8;
			dz = -(COS(angle) * 5) >> 8;

			if(!move_ok(p_person,dx<<4,dz<<4))
			{

				set_person_hug_wall_stand(p_person,0,0);
				return;
			}




			if((dist=abs(check_near_facet(p_person,64,64,(p_person->WorldPos.X+dx*8)>>8,(p_person->WorldPos.Z+dz*8)>>8)))==0)
			{
				set_person_hug_wall_look(p_person,0);
//				set_person_hug_wall_stand(p_person);
				return;
			}

			if(abs(32-dist)<32)
			{
				angle=(p_person->Draw.Tweened->Angle)&2047;

				dx += -(SIN(angle) * (32-dist)) >> 8;
				dz += -(COS(angle) * (32-dist)) >> 8;

			}

			person_normal_move_dxdz(p_person,dx,dz);
			end = person_normal_animate(p_person);
/*
			if(end)
			{
				set_person_hug_wall_stand(p_person);
			}
*/
			
			break;
		case	SUB_STATE_HUG_WALL_LEAP_OUT:
			end = person_normal_animate(p_person);
			if(end)
			{
				if(person_has_gun_out(p_person))
				{
					set_person_aim(p_person,SUB_OBJECT_PELVIS+1);
				}
				else
				{
					set_person_locked_idle_ready(p_person);
				}
				p_person->Draw.Tweened->Angle+=1024;
				p_person->Draw.Tweened->Angle&=2047;

				plant_feet(p_person);

//				set_person_idle(p_person);
				
			}

			break;
		case	SUB_STATE_HUG_WALL_LOOK_L:
			//if(continue_dir(p_person,0))
			if(++p_person->Genus.Person->InsideRoom>16)
				p_person->Genus.Person->InsideRoom=16;

				end = person_normal_animate(p_person);
/*			else
			{
				set_person_hug_wall_stand(p_person);
			}
*/
			break;
		case	SUB_STATE_HUG_WALL_LOOK_R:
//			if(continue_dir(p_person,1))
			if(++p_person->Genus.Person->InsideRoom>16)
				p_person->Genus.Person->InsideRoom=16;
				end = person_normal_animate(p_person);
/*
			else
			{
				set_person_hug_wall_stand(p_person);
			}
*/


			break;
		case	SUB_STATE_HUG_WALL_STEP_RIGHT:
			if(!continue_dir(p_person,0))
			{
				set_person_hug_wall_stand(p_person);
				return;
			}
			angle=(p_person->Draw.Tweened->Angle-512)&2047;

			dx = -(SIN(angle) * 5) >> 8;
			dz = -(COS(angle) * 5) >> 8;

			if(!move_ok(p_person,dx<<4,dz<<4))
			{

				set_person_hug_wall_stand(p_person,0,0);
				return;
			}

			if((dist=abs(check_near_facet(p_person,64,64,(p_person->WorldPos.X+dx*8)>>8,(p_person->WorldPos.Z+dz*8)>>8)))==0)
			{
				set_person_hug_wall_look(p_person,1);
//				set_person_hug_wall_stand(p_person);
				return;
			}
			if(abs(32-dist)<32)
			{
				angle=(p_person->Draw.Tweened->Angle)&2047;

				dx += -(SIN(angle) * (32-dist)) >> 8;
				dz += -(COS(angle) * (32-dist)) >> 8;

			}

			person_normal_move_dxdz(p_person,dx,dz);
			end = person_normal_animate(p_person);
/*
			if(end)
			{
				set_person_hug_wall_stand(p_person);
			}
*/
			break;
	}
}


SLONG kick_or_punch = 0;

void fn_person_circle(Thing *p_person)
{
	//
	// start simple orbit the enemy
	//

	SLONG dx=0;
	SLONG	dy;
	SLONG dz=0;
	SLONG end;
	SLONG dist;
	SLONG angle;
	SLONG shove;
	SLONG dangle;
	SLONG random;
	SLONG ddist;

	SLONG vx = 0;
	SLONG vz = 0;
	UBYTE	renav=0;
	UWORD	reqd_anim=0;

	BOOL poo;
	
	Thing *p_target = TO_THING(p_person->Genus.Person->Target);
	SLONG	hit_distance=140;
	SLONG	bat=0;
	SLONG	stamp=0;
	if(person_holding_bat(p_person))
		bat=1;

	if(p_person->Genus.Person->Target==0)
	{
		set_person_idle(p_person);
		return;
	}

	if (p_person->Genus.Person->Flags&FLAG_PERSON_REQUEST_BLOCK)
	{
		p_person->Genus.Person->Flags&=~FLAG_PERSON_REQUEST_BLOCK;

		//
		// person has decided to do a block so, do a block
		//

		set_person_block(p_person);
		return;
	}
	if((p_target->Genus.Person->PlayerID==0) ||p_person->Genus.Person->PersonType==PERSON_COP)
		p_person->Genus.Person->Agression=100;

	ASSERT(p_target!=p_person);

	end = person_normal_animate(p_person);

	if (is_person_ko_and_lay_down(p_target))
	{
		//
		// Close enough to stamp!
		//
		stamp=4; //renav every 4 turns

		hit_distance = 60;

		if (!(p_target->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL))
		{
			if ( p_person->Genus.Person->PersonType == PERSON_DARCI      ||
				 (p_person->Genus.Person->pcom_ai    == PCOM_AI_COP && p_person->Genus.Person->PersonType==PERSON_COP)      ||
				 p_person->Genus.Person->pcom_ai    == PCOM_AI_COP_DRIVER||
				(p_person->Genus.Person->pcom_ai    == PCOM_AI_BODYGUARD && p_person->Genus.Person->PersonType == PERSON_COP))

//			if (p_person->Genus.Person->pcom_ai    == PCOM_AI_COP ||
//				p_person->Genus.Person->pcom_ai    == PCOM_AI_COP_DRIVER ||
//				p_person->Genus.Person->PersonType == PERSON_DARCI)
			{
				//
				// Cops get in close enough to arrest!
				//

				hit_distance = 30;
				p_person->Genus.Person->Agression =0;
				stamp=1; //renav every turn to ensure we get to a good arrest position

			}
		}

		p_person->SubState = SUB_STATE_CIRCLING_CIRCLE;
	}

	if (end == 1)
	{

		SLONG	anim;
		//
		// Carry on with your fighting stance.
		//
		anim=find_idle_fight_stance(p_person);

		set_anim(p_person, anim);
	}

	if (p_person->Genus.Person->Agression < -50)
	{
		//
		// back off
		//

		p_person->SubState=SUB_STATE_CIRCLING_BACK_OFF;
	}

	/*

	if(p_person->Genus.Person->Agression>0)
	{
		hit_distance=140;
	}

	*/

	switch(p_person->SubState)
	{
		case	SUB_STATE_CIRCLING_BACK_OFF:
				hit_distance=280;
				if(p_person->Genus.Person->Agression>-10)
				{
					p_person->SubState=SUB_STATE_CIRCLING_CIRCLE;
					//p_person->Genus.Person->Timer1=-5; // make sure I don't push myself 

					//
					// set everyone else to backoff
					//
					if(p_target->Genus.Person->PlayerID)
						process_gang_attack(p_person,p_target);

				}

				// FALLTHROUGH!

		case	SUB_STATE_CIRCLING_CIRCLE:

				if (p_target->Genus.Person->Flags & FLAG_PERSON_DRIVING)
				{
					//
					// Back off from people in cars! If we have a gun then PCOM_process_killing()
					// will make us draw a gun and shoot the car.
					//

					hit_distance *= 2;
				}

				if((PTIME(p_person)&1)==0)
				{
					if(p_person->Genus.Person->Agression<0)
						p_person->Genus.Person->Agression+=(GET_SKILL(p_person)>>2)+1;
					else
						p_person->Genus.Person->Agression--;
				}


//				if(p_person->Genus.Person->Agression>=0)
				{
					SLONG	renav_how_often;
					renav_how_often=(40-(GET_SKILL(p_person)<<1));
					if(!p_target->Genus.Person->PlayerID)
					{
						renav_how_often>>=1;
						if(p_person->Genus.Person->PersonType==PERSON_COP)
							renav_how_often-=5;

						if(renav_how_often<5)
							renav_how_often=5;

					}
					if(stamp)
						renav_how_often=stamp;
					if((PTIME(p_person)%renav_how_often)==0)
					{
						if(p_target->Genus.Person->PlayerID)
						{
							//its a player so do the panel stuff

							track_enemy(p_person);
						}

						calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&dx,&dy,&dz);
						p_person->Genus.Person->TargetX=(p_target->WorldPos.X>>8)+dx;
						p_person->Genus.Person->TargetZ=(p_target->WorldPos.Z>>8)+dz;


						if(p_person->SubState==SUB_STATE_CIRCLING_CIRCLE)
						{
							//
							// set everyone else to backoff
							//
							if(p_target->Genus.Person->PlayerID)
								process_gang_attack(p_person,p_target);
						}
						renav=1;
					}
				}

				//
				// Where is my target relative to me?
				//

					dx = p_person->Genus.Person->TargetX - (p_person->WorldPos.X >> 8);
					dz = p_person->Genus.Person->TargetZ - (p_person->WorldPos.Z >> 8);

					dist   = QDIST2(abs(dx),abs(dz)) + 1;
/*
				{
					CBYTE	str[100];
					sprintf(str," combat dist %d hit_dist %d \n",dist,hit_distance);
					CONSOLE_text(str);
				}
*/
				angle  = (calc_angle(dx,dz))&2047;

				dx=0;
				dz=0;

				//
				// angle relative to targets viewpoint
				//

				if(dist<hit_distance-15)
				{
					// forewards
					if(bat)
						reqd_anim=ANIM_FIGHT_STEP_N_BAT;
					else
						reqd_anim=ANIM_FIGHT_STEP_N;
					dx=-SIN(angle)>>4;
					dz=-COS(angle)>>4;
				}
				else
				if(dist>hit_distance+15)
				{
					//backwards
					if(bat)
						reqd_anim=ANIM_FIGHT_STEP_S_BAT;
					else
						reqd_anim=ANIM_FIGHT_STEP_S;
					dx=SIN(angle)>>4;
					dz=COS(angle)>>4;
				}
				else
				{
					dx=0;
					dz=0;
				}


//				if(dist<hit_distance-20 || dist>hit_distance+20)
				{
					if(p_person->Genus.Person->Timer1++>10)
					{
						SLONG	gang;
				void	push_into_attack_group_at_angle(Thing *p_person,SLONG gang,SLONG reqd_angle);
						gang=p_target->Genus.Person->GangAttack;
						if(gang==0)
						{
							//CONSOLE_text(" Circling but have no gang");
							set_person_idle(p_person);
							return;
						}

						remove_from_gang_attack(p_person,p_target);
						push_into_attack_group_at_angle(p_person,gang,(angle+256)>>9);
						p_person->Genus.Person->Timer1=0;
					}
				}
/*
				else
				if(p_person->SubState==SUB_STATE_CIRCLING_CIRCLE)
				{

					if(p_person->Genus.Person->Timer1++==15)
					{
						process_gang_attack(p_person,p_target);
						p_person->Genus.Person->Timer1=0;
					}
				}
*/

				if (dist > 80)
				{
					//
					// add 1024 to make you look at darci rather than away from darci
					//

					p_person->Draw.Tweened->Angle = (angle+1024)&2047;
				}

				dangle=p_person->Genus.Person->AttackAngle<<9;
			//	ASSERT(p_person->Genus.Person->AttackAngle==0);

				dangle-=angle;

				if(abs(dangle)>30)
				{
					SLONG	shift=4;
					if (dangle >  1024) {dangle -= 2048;}
					if (dangle < -1024) {dangle += 2048;}



					if(dangle>0)
					{
						if(reqd_anim==0)
						{
							if(bat)
								reqd_anim=ANIM_FIGHT_STEP_E_BAT;
							else
								reqd_anim=ANIM_FIGHT_STEP_E;
						}

						angle-=512;
					}
					else
					{
						if(reqd_anim==0)
						{
							if(bat)
								reqd_anim=ANIM_FIGHT_STEP_W_BAT;
							else
								reqd_anim=ANIM_FIGHT_STEP_W;
						}
						angle+=512;
					}

					angle&=2047;
					if(abs(dangle)<60)
						shift=5;
					dx+=SIN(angle)>>shift;
					dz+=COS(angle)>>shift;
				}

				if(dx||dz)
				{
					person_normal_move_dxdz(p_person,dx,dz);
/*
					move_thing(
						dx ,
						 0 << 8,
						dz ,
						p_person);
*/

					if(reqd_anim)
						if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT || p_person->Draw.Tweened->CurrentAnim==ANIM_BAT_IDLE ||p_person->Draw.Tweened->CurrentAnim==ANIM_KNIFE_FIGHT_READY)
							set_anim(p_person,reqd_anim);
				}
				else
				{
					//if(p_person->Draw.Tweened->CurrentAnim!=ANIM_FIGHT || )
					if(p_person->Draw.Tweened->CurrentAnim!=ANIM_FIGHT && p_person->Draw.Tweened->CurrentAnim!=ANIM_BAT_IDLE && p_person->Draw.Tweened->CurrentAnim!=ANIM_KNIFE_FIGHT_READY)
					{
						SLONG	anim;
						anim=find_idle_fight_stance(p_person);
						set_anim(p_person,anim);
					}

				}

				if(renav)
				{
					if(am_i_a_thug(p_person))
					{
						//
						// only arrest thugs?
						//
						PCOM_call_cop_to_arrest_me(p_person,1);
					}

					if (p_target->Genus.Person->PlayerID && EWAY_stop_player_moving())
					{
						//
						// Don't the player while there are stationary in a cutscene.
						//
					}
					else
					{
						if (p_person->Genus.Person->pcom_ai== PCOM_AI_FIGHT_TEST)
						{
							//
							// Fight test dummies never throw a punch.
							//
						}
						else
						{
							extern	void PCOM_set_person_move_punch (Thing *p_person);
							extern	void PCOM_set_person_move_kick  (Thing *p_person);
							extern	void PCOM_set_person_move_arrest(Thing *p_person);

							kick_or_punch++;
		//					if(p_target->Genus.Person->Flags&FLAG_PERSON_KO)
							if(is_person_ko_and_lay_down(p_target))
							{
								if (!(p_target->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL))
								{
									if ( p_person->Genus.Person->PersonType == PERSON_DARCI      ||
										 (p_person->Genus.Person->pcom_ai    == PCOM_AI_COP && p_person->Genus.Person->PersonType==PERSON_COP)      ||
										 p_person->Genus.Person->pcom_ai    == PCOM_AI_COP_DRIVER||
										(p_person->Genus.Person->pcom_ai    == PCOM_AI_BODYGUARD && p_person->Genus.Person->PersonType == PERSON_COP))
									{
										if(dist<50)
											PCOM_set_person_move_arrest(p_person);
									}
									else
									{
										PCOM_set_person_move_kick(p_person);
									}
								}
							}
							/*
							else
							if (p_target->SubState              == SUB_STATE_GRAPPLE_HELD &&
								p_person->Genus.Person->pcom_ai == PCOM_AI_COP)
							{
								//
								// Cops always throw their grapplee's so they can arrest them!
								//

								PCOM_set_person_move_punch(p_person);
							}
							*/
							else
							if(dist<160 && dist>90)
							{

								if(kick_or_punch&1)
								{
									PCOM_set_person_move_punch(p_person);
								}
								else
								{
									PCOM_set_person_move_kick(p_person);
								}

							}
							else
							if(dist<90)
							{
								PCOM_set_person_move_punch(p_person);
							}
						}
					}
				}
			break;

	}

}

#ifndef PSX
void fn_person_circle_old(Thing *p_person)
{
	SLONG dx;
	SLONG dz;
	SLONG end;
	SLONG dist;
	SLONG angle;
	SLONG shove;
	SLONG dangle;
	SLONG random;
	SLONG ddist;

	SLONG vx = 0;
	SLONG vz = 0;
	
	Thing *p_target = TO_THING(p_person->Genus.Person->Target);

	end = person_normal_animate(p_person);

	if (end == 1)
	{
		//
		// Carry on with your fighting stance.
		//

		set_anim(p_person, ANIM_FIGHT);
	}

	//
	// Where is our target relative to us?
	//

	dx = p_target->WorldPos.X - p_person->WorldPos.X >> 8;
	dz = p_target->WorldPos.Z - p_person->WorldPos.Z >> 8;

	dist   = QDIST2(abs(dx),abs(dz)) + 1;
	angle  = calc_angle(dx,dz) + 1024;
	angle &= 2047;
	dangle = angle_diff(angle, p_person->Draw.Tweened->Angle);

	//
	// Trickle round to 90 degree relative angle.
	//

	dangle += 256;
	dangle &= 0xff;

	shove  = p_person->Genus.Person->Shove;
	shove += dangle - 256 >> 4;

	SATURATE(shove, -127, +127);

	if (shove > 0) {shove -= 16;} else {shove += 16;}

	p_person->Genus.Person->Shove = shove;

	//
	// The distance we hop by.
	//

	dx   = ((0x10 * TICK_RATIO) * dx) / dist;
	dz   = ((0x10 * TICK_RATIO) * dz) / dist;

	dx >>= TICK_SHIFT;
	dz >>= TICK_SHIFT;

	//
	// Should we hop to the left or right?
	//

	if (shove > 0)
	{
		random = Random() & 0x7f;

		if (shove > random)
		{
			//
			// Hop to the left.
			//

			vx += -dz;
			vz += +dx;
		}
	}
	else
	{
		random =  Random() & 0x7f;
		random = -random;

		if (shove < random)
		{
			//
			// Hop to the right.
			//

			vx += +dz;
			vz += -dx;
		}
	}

	//
	// Should we hop in or out?
	//

	ddist = dist - 0x80;

	if (ddist > 0)
	{
		random = Random() & 0x3f;

		if (ddist > random)
		{
			vx += dx;
			vz += dz;
		}
	}
	else
	{
		random =  Random() & 0x3f;
		random = -random;

		if (ddist < random)
		{
			vx += -dx;
			vz += -dz;
		}
	}

	//
	// Move the person.
	//

	move_thing(
		vx << 8,
		 0 << 8,
		vz << 8,
		p_person);
}
#endif



//*************************************************************************************************************
//** (JCL)  - get person scale

SLONG	person_get_scale(Thing *t)
{
	// ASSERT( t is actually a person.  probably is, cause this gets called from FIGURE_draw_blah() );

	// fixed point 8.  (ie 256 = normal, 512 = double size, 128 = half size.
	// I wonder what happens if you use negative values!

	if(t->Class==CLASS_PERSON && t->Genus.Person->PersonType==PERSON_ROPER)
		return(276);
	if(t->Class==CLASS_PERSON && t->Genus.Person->PlayerID)
	{
		return(256);
	}
	else
	{
//		if(t->Class==CLASS_BAT && t->Genus.Bat->type==BAT_TYPE_BALROG)
//			return(256);
//		else
//			return 256;//56;

		return(256);//250+(THING_NUMBER(t)&0x1f));
	}
  
	// this essentially generates random heights per person
	//return ((SLONG(t) >> 3) & 7) * (384 / 7) + 128;

	// PS - this function should be reasonably quick.  it's called 15 times per character per frame.
}


//
// returns how many game_turns an anim takes, a game_turn is assumed to be 1/20th of a second
//
#ifndef	PSX
SLONG	how_long_is_anim(SLONG anim)
{
	GameKeyFrame	*frame;
	SLONG	total=0;

	frame=global_anim_array[0][anim];


	while(frame)
	{
		SLONG	step;

		step=frame->TweenStep;
		if(!step)
			step=1;


		total+=256/step;
		if(frame->Flags&ANIM_FLAG_LAST_FRAME)
			break;

		frame	=	frame->NextFrame;
	}
	return(total);

}
#endif

SLONG person_ok_for_conversation(Thing *p_person)
{
	if (p_person->Class != CLASS_PERSON)
	{
		return 0;
	}

	if(is_person_ko(p_person))
		return(0);

	if(p_person->State==STATE_DEAD)
		return(0);
	if(p_person->State==STATE_DANGLING)
		return(0);
	if(p_person->State==STATE_DYING)
		return(0);
	if(p_person->State==STATE_JUMPING)
		return(0);

	if (p_person->State == STATE_CLIMB_LADDER)
	{
		return 0;
	}

	if ((p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING) ||
		(p_person->Genus.Person->Flags & FLAG_PERSON_BIKING))
	{
		return 0;
	}

	return(1);
}

void set_person_float_up(Thing *p_person)
{
	set_generic_person_state_function(p_person, STATE_FLOAT);

	p_person->SubState = SUB_STATE_FLOAT_UP;

//	set_anim(p_person, ANIM_DANCE_HEADBANG);
}

void set_person_float_down(Thing *p_person)
{
	ASSERT(p_person->State == STATE_FLOAT);

	p_person->SubState = SUB_STATE_FLOAT_DOWN;
}


void fn_person_float(Thing *p_person)
{
	SLONG ground;

	ground = PAP_calc_map_height_at(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Z >> 8);

	switch(p_person->SubState)
	{
		case SUB_STATE_FLOAT_UP:

			p_person->WorldPos.Y        += 64 * TICK_RATIO >> TICK_SHIFT;
			p_person->Draw.Tweened->Tilt = (-p_person->WorldPos.Y >> 8) - ground;

			if (p_person->Draw.Tweened->Tilt < -200)
			{
				p_person->SubState = SUB_STATE_FLOAT_BOB;
			}

			break;

		case SUB_STATE_FLOAT_BOB:

			//
			// Do nowt while bobbing...
			//

			break;

		case SUB_STATE_FLOAT_DOWN:

			p_person->WorldPos.Y        -= 64 * TICK_RATIO >> TICK_SHIFT;
			p_person->Draw.Tweened->Tilt = (-p_person->WorldPos.Y >> 8) - ground;

			if (p_person->Draw.Tweened->Tilt > -4)
			{
				set_person_idle(p_person);
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}

void set_person_injured(Thing *p_person)
{
	set_generic_person_state_function(p_person, STATE_DEAD);
	set_anim(p_person, ANIM_INJURED_LOOP);
	p_person->SubState = SUB_STATE_DEAD_INJURED;
	p_person->Genus.Person->Timer1 = 0;
}


//
// Makes sure that person 'a' is at least half a block from person 'b'.
//

void push_people_apart(Thing *p_person, Thing *p_avoid)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;

	if (p_person->Genus.Person->Ware ||
		p_avoid ->Genus.Person->Ware)
	{
		//
		// This function doesn't work for people in warehouses.
		//

		return;
	}

	dx = p_person->WorldPos.X - p_avoid->WorldPos.X >> 8;
	dz = p_person->WorldPos.Z - p_avoid->WorldPos.Z >> 8;

	dist = QDIST2(abs(dx),abs(dz)) + 1;

	if (dist >= 0x80)
	{
		//
		// Already far enough apart.
		//
		
		return;
	}

	SLONG new_x = (p_person->WorldPos.X >> 8) + dx * (0x80 - dist) / dist;
	SLONG new_z = (p_person->WorldPos.Z >> 8) + dz * (0x80 - dist) / dist;

	SLONG old_y = PAP_calc_map_height_at(p_person->WorldPos.X >> 8, p_person->WorldPos.Z >> 8);
	SLONG new_y = PAP_calc_map_height_at(new_x, new_z);

	if (abs(old_y - new_y) < 0x50)
	{
		//
		// Ok to move here...
		// 

		GameCoord newpos;

		newpos.X = new_x << 8;
		newpos.Y = new_y << 8;
		newpos.Z = new_z << 8;

		move_thing_on_map(p_person, &newpos);
	}
	else
	{
		//
		// Bugger it! Doesn't have to work all the time...
		//
	}
}
