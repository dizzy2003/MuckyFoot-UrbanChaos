#include	"game.h"
#include	"combat.h"
#include	"animate.h"
#include	"statedef.h"
//#include	"sample.h"
#include	"Sound.h"
#include	"pap.h"
#include	"pcom.h"
#include	"overlay.h"
#include	"mfx.h"
#include	"eway.h"
#include	"psystem.h"
#include	"poly.h"
#include	"dirt.h"

#ifdef TARGET_DC
#include "DIManager.h"
#endif

SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor);

extern	SLONG	set_face_thing(Thing *p_person,Thing *p_target);
extern	void	e_draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
extern	void	e_draw_3d_line_col(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
SLONG	check_combat_hit_with_person(Thing	*p_victim,SLONG x,SLONG y,SLONG z,struct GameFightCol *fight,Thing *p_agressor,SLONG *ret_angle);
SLONG	check_combat_grapple_with_person(Thing	*p_victim,MAPCO16 x,MAPCO16 y,MAPCO16 z,struct GameFightCol *fight,Thing *p_agressor,SLONG *ret_angle,SLONG grapple);
extern	void	reset_gang_attack(Thing *p_target);
extern	void	scare_gang_attack(Thing *p_target);
extern	SLONG person_has_gun_out(Thing *p_person);
extern	void	drop_current_gun(Thing *p_person,SLONG change_anim);
extern	SLONG	is_person_dead(Thing *p_person);
extern	SLONG	is_person_ko(Thing *p_person);
extern	void	add_damage_value_thing(Thing *p_thing,SLONG value);
extern	SLONG	set_person_kick_near(Thing *p_person,SLONG dist);
extern	SLONG dist_to_target(Thing *p_person_a,Thing *p_person_b);
extern	SLONG	is_person_ko_and_lay_down(Thing *p_person);

extern	SLONG	person_on_floor(Thing *p_person);

extern	SLONG is_there_room_behind_person(Thing *p_person, SLONG hit_from_behind);

#define	FIGHT_ANGLE_RANGE	(400)

extern	SLONG	set_person_stomp(Thing *p_person);

#define STANCE_MAX_FIND 8
#define STANCE_RADIUS	0x200

THING_INDEX found[16];

#ifdef	UNUSED
struct ComboHistory combo_histories[MAX_HISTORY];
struct BlockingHistory block_histories[MAX_HISTORY];
#endif
struct GangAttack gang_attacks[MAX_HISTORY];

//
// I've changed this to a 2d array, but may change back to structure if one element becomes > byte
//
struct	FightTree
{
	UBYTE	Anim;
	UBYTE	Finish;
	UBYTE	NextPunch1;
	UBYTE	NextPunch2;
	UBYTE	NextKick1;
	UBYTE	NextKick2;
	UBYTE	NextJump;
	UBYTE	NextBlock;
	UBYTE	Damage;
	UBYTE	HitType;
};

#define	FIGHT_TREE_DAMAGE	8
#define	FIGHT_TREE_HIT_TYPE	9


//struct	FighTree fight_tree[]=
SWORD	fight_tree[][10]=
{
	{0						,0 ,1 ,1 ,6 ,6 ,0 ,0 ,0 ,COMBAT_NONE},
	{ANIM_PUNCH_COMBO1		,2 ,3 ,0 ,11,11,0 ,0 ,10,COMBAT_PUNCH },  // 1
	{ANIM_PUNCH_RETURN1		,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,COMBAT_NONE},  // 2
	{ANIM_PUNCH_COMBO2		,4 ,5 ,1 ,12,12,0 ,0 ,30,COMBAT_PUNCH},  // 3
	{ANIM_PUNCH_RETURN2		,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,COMBAT_NONE},  // 4
	{ANIM_PUNCH_COMBO3		,0 ,0 ,0 ,0 ,0 ,0 ,0 ,60,COMBAT_PUNCH},  // 5
						  	  			       
	{ANIM_KICK_COMBO1		,7 ,7 ,7 ,8 ,0 ,0 ,0 ,10,COMBAT_KICK},  // 6
	{ANIM_KICK_RETURN1		,0 ,1 ,0 ,6 ,0 ,0 ,0 ,0 ,COMBAT_NONE},  // 7
	{ANIM_KICK_COMBO2		,9 ,13,13,10,0 ,0 ,0 ,30,COMBAT_KICK},  // 8
	{ANIM_KICK_RETURN2		,0 ,1 ,0 ,6 ,0 ,0 ,0 ,0 ,COMBAT_NONE},  // 9
	{ANIM_KICK_COMBO3		,0 ,0 ,0 ,6 ,0 ,0 ,0 ,60,COMBAT_KICK},  // 10

	{ANIM_PUNCH_COMBO2b		,0 ,0 ,0 ,0 ,0 ,0 ,0 ,30,COMBAT_KICK},  // 11
	{ANIM_PUNCH_COMBO3b		,0 ,0 ,0 ,0 ,0 ,0 ,0 ,80,COMBAT_KICK},  // 12
	{ANIM_KICK_COMBO3b		,0 ,0 ,0 ,0 ,0 ,0 ,0 ,80,COMBAT_PUNCH},  // 13

	{ANIM_KNIFE_ATTACK1		,15,16,0 ,0 ,0 ,0 ,0 ,30,COMBAT_KNIFE },  // 14
	{ANIM_KNIFE_ATTACK1_RET	,0 ,14,14,0 ,0 ,0 ,0 ,0 ,COMBAT_NONE},  // 15
	{ANIM_KNIFE_ATTACK2		,17,18,14,0 ,0 ,0 ,0 ,60,COMBAT_KNIFE},  // 16
	{ANIM_KNIFE_ATTACK2_RET	,0 ,14,14,0 ,0 ,0 ,0 ,0 ,COMBAT_NONE},  // 17
	{ANIM_KNIFE_ATTACK3		,0 ,0 ,0 ,0 ,0 ,0 ,0 ,80,COMBAT_KNIFE},  // 18

	{ANIM_BAT_HIT1			,20,21,0 ,0 ,0 ,0 ,0 ,60,COMBAT_KNIFE },  // 19
	{ANIM_BAT_HIT1_RET		,0,19,0 ,0 ,0 ,0 ,0 ,0,COMBAT_NONE },  // 20
	{ANIM_BAT_HIT2			,0,0,0 ,0 ,0 ,0 ,0 ,90,COMBAT_KNIFE },  // 21
};


// front  upper
// front  middle
// front lower

// behind  upper
// behind  middle
// behind lower

UBYTE	take_hit[7][2]=
{
	{ANIM_KD_FRONT_LOW,1},
	{ANIM_HIT_FRONT_MID,0},
	{ANIM_HIT_FRONT_HI,0},
	{ANIM_KD_BACK_LOW,1},
	{ANIM_HIT_BACK_MID,0},
	{ANIM_HIT_BACK_HI,0},
	{0,0}
};



//
// Data Hiding
//
SLONG	get_anim_and_node_for_action(UBYTE current_node,UBYTE action,UWORD *new_anim)
{
	SLONG	new_node;

	new_node=fight_tree[current_node][action];
	*new_anim=fight_tree[new_node][0];
	return(new_node);

}


SLONG	get_combat_type_for_node(UBYTE current_node)
{
	return(fight_tree[current_node][9]);
}



SWORD	punches[4][5]=
{
	{ANIM_PUNCH_COMBO1,ANIM_PUNCH_COMBO2,ANIM_PUNCH_COMBO3,0,0},
	{ANIM_PUNCH3,ANIM_PUNCH1,0,0,0},
	{ANIM_PUNCH2,0,0,0,0},
	{0,0,0,0,0}
};

SWORD	kicks[4][5]=
{
	{ANIM_KICK_COMBO1,ANIM_KICK_COMBO2,ANIM_KICK_COMBO3,0,0},
	{ANIM_KICK2,ANIM_KICK_ROUND1,0,0,0},
	{ANIM_KICK3,0,0,0,0},
	{0,0,0,0,0}
};


struct	Grapples
{
	UWORD	Anim;
	SWORD	Dist;
	SWORD	Range;
	SWORD	Angle;
	SWORD	DAngle;
	SWORD	Peep;
};

struct	Grapples	grapples[]=
{
	{ANIM_PISTOL_WHIP,75,65,1024,0,1},
	{ANIM_STRANGLE,  50,20,   0,0,2},
	{ANIM_HEADBUTT,  65,20,   0,0,2},
	{ANIM_GRAB_ARM,  60,20,   0,0,1},
//	{ANIM_SNAP_KNECK,75,65,1024,0,1},
	{0,0,0,0,0},
};

SWORD	grapple[]=
{
	ANIM_SNAP_KNECK,
	0
};

void	init_gangattack(void)
{
	memset((UBYTE*)gang_attacks,0,sizeof(GangAttack)*MAX_HISTORY);
}

SLONG	get_gangattack(Thing *p_person)
{
	UWORD	c0;
	SLONG	oldest=0,best=0;
	for(c0=1;c0<MAX_HISTORY;c0++)
	{
		SLONG	ticks;

		ticks=((UWORD)GAME_TURN)-gang_attacks[c0].LastUsed;

		if( ticks>oldest)
		{
			oldest=ticks;
			best=c0;
		}
		if(gang_attacks[c0].Owner==0)
		{
			p_person->Genus.Person->GangAttack=c0;
			gang_attacks[c0].Owner=THING_NUMBER(p_person);
			gang_attacks[c0].Index=0;
			gang_attacks[c0].Count=0;
			gang_attacks[c0].LastUsed=(UWORD)GAME_TURN;
			memset(&gang_attacks[c0].Perp[0],0,16);

			return(c0);
		}
	}

	if(best)
	{
		Thing	*p_owner;

		
		p_owner=TO_THING(gang_attacks[best].Owner);
		p_owner->Genus.Person->GangAttack=0;
		p_person->Genus.Person->GangAttack=best;
		gang_attacks[best].Owner=THING_NUMBER(p_person);
		memset(&gang_attacks[best].Perp[0],0,16);
//		memset(&gang_attacks[best].PerpDiag[0],0,8);
		gang_attacks[best].Index=0;
		gang_attacks[best].Count=0;
		gang_attacks[best].LastUsed=(UWORD)GAME_TURN;
	}
	ASSERT(best);
	return(best);
}

#ifdef	UNUSED
SLONG	get_history(Thing *p_person)
{
	UWORD	c0;
	SLONG	oldest=0,best=0;
	for(c0=1;c0<MAX_HISTORY;c0++)
	{
		SLONG	ticks;

		ticks=((UWORD)GAME_TURN)-combo_histories[c0].LastUsed;

		if( ticks>oldest)
		{
			oldest=ticks;
			best=c0;
		}
		if(combo_histories[c0].Owner==0)
		{
			p_person->Genus.Person->ComboHistory=c0;
			combo_histories[c0].Owner=THING_NUMBER(p_person);
			combo_histories[c0].Index=0;
			combo_histories[c0].Count=0;

			return(c0);
		}
	}

	if(best)
	{
		Thing	*p_owner;
		p_owner=TO_THING(combo_histories[best].Owner);
		p_owner->Genus.Person->ComboHistory=0;
		p_person->Genus.Person->ComboHistory=best;
		combo_histories[best].Owner=THING_NUMBER(p_person);
		combo_histories[best].Index=0;
		combo_histories[best].Count=0;
	}
	return(best);
}

SLONG	get_block_history(Thing *p_person)
{
	UWORD	c0;
	SLONG	oldest=0,best=0;
	for(c0=1;c0<MAX_HISTORY;c0++)
	{
		SLONG	ticks;

		ticks=((UWORD)GAME_TURN)-block_histories[c0].LastUsed;

		if( ticks>oldest)
		{
			oldest=ticks;
			best=c0;
		}
		if(block_histories[c0].Owner==0)
		{
			p_person->Genus.Person->BlockHistory=c0;
			block_histories[c0].Owner=THING_NUMBER(p_person);
			block_histories[c0].Index=0;
			block_histories[c0].Count=0;

			return(c0);
		}
	}

	if(best)
	{
		Thing	*p_owner;
		p_owner=TO_THING(block_histories[best].Owner);

		p_owner->Genus.Person->BlockHistory=0;
		p_person->Genus.Person->BlockHistory=best;

		block_histories[best].Owner=THING_NUMBER(p_person);
		block_histories[best].Index=0;
		block_histories[best].Count=0;
	}
	return(best);
}
#endif

SLONG	find_possible_combat_target(struct GameFightCol *p_fight,SLONG x,SLONG y,SLONG z,Thing *p_thing,Thing **p_victim)
{
	SLONG i;
	SLONG hit;
	SLONG angle;

	Thing *t_thing;

	//
	// Find all the people near us.
	//

//	THING_INDEX found[16];
	SLONG       found_upto;

	found_upto = THING_find_sphere(x, y, z, 0x280, found, 16, (1 << CLASS_PERSON));

	//
	// Check each person in turn.
	//

	hit = 0;

	for (i = 0; i < found_upto; i++)
	{
		t_thing = TO_THING(found[i]);

		if (t_thing != p_thing)
		{
			if (check_combat_hit_with_person(
					t_thing,
					x, y, z,
					p_fight,
					p_thing,
				   &angle) > 0)
			{
				hit     += p_fight->Damage+5;
			   *p_victim = t_thing;
			}
		}
	}
	
	return hit;

	/*

	SLONG	dx,dz;
	SLONG	index;
	Thing	*t_thing;
	SLONG	hit=0;
	SLONG	angle;

	for(dx=-2;dx<3;dx++)
	for(dz=-2;dz<3;dz++)
	{
		SLONG	mx,mz;
		mx=(x>>ELE_SHIFT)+dx;
		mz=(z>>ELE_SHIFT)+dz;
		index=MAP[MAP_INDEX(mx,mz)].MapWho;
		while(index)
		{
			t_thing	= TO_THING(index);
			if(t_thing!=p_thing)
			{
				LogText(" found a thing at dx dz %d %d of type %d\n",dx,dz,t_thing->DrawType);
				switch(t_thing->DrawType)
				{
					case	DT_ROT_MULTI:
						LogText(" its a figure \n");
						if(check_combat_hit_with_person(t_thing,x,y,z,p_fight,p_thing,&angle)>0)
						{
							hit+=p_fight->Damage;
							*p_victim=t_thing;
							
						}

						break;

				}
			}
			index	=	t_thing->Child;
		}
	}

	return(hit);

	*/
}

SLONG	find_possible_grapple_target(struct GameFightCol *p_fight,SLONG x,SLONG y,SLONG z,Thing *p_thing,Thing **p_victim,SLONG grapple)
{
	SLONG i;
	SLONG hit;
	SLONG angle;

	Thing *t_thing;

	//
	// Find all the people near us.
	//

//	THING_INDEX found[16];
	SLONG       found_upto;

	found_upto = THING_find_sphere(x, y, z, 0x280, found, 16, (1 << CLASS_PERSON));

	//
	// Check each person in turn.
	//

	hit = 0;

	for (i = 0; i < found_upto; i++)
	{
		t_thing = TO_THING(found[i]);
		if(p_thing!=t_thing)
		if(people_allowed_to_hit_each_other(t_thing,p_thing))
		{
			if (t_thing != p_thing && t_thing->SubState!=SUB_STATE_GRAPPLE_HOLD &&t_thing->SubState!=SUB_STATE_GRAPPLE_HELD&&t_thing->SubState!=SUB_STATE_GRAPPLE_ATTACK && t_thing->SubState!=SUB_STATE_ESCAPE && t_thing->State!=STATE_DEAD && !is_person_ko(t_thing) && t_thing->Draw.Tweened->CurrentAnim!=ANIM_PISTOL_WHIP_TAKE)
			{
				if (check_combat_grapple_with_person(
						t_thing,
						x, y, z,
						p_fight,
						p_thing,
					   &angle,grapple) > 0)
				{
					hit     += 1000;
				   *p_victim = t_thing;
				}
			}
		}
	}
	
	return hit;

	/*

	SLONG	dx,dz;
	SLONG	index;
	Thing	*t_thing;
	SLONG	hit=0;
	SLONG	angle;


	for(dx=-2;dx<3;dx++)
	for(dz=-2;dz<3;dz++)
	{
		SLONG	mx,mz;
		mx=(x>>ELE_SHIFT)+dx;
		mz=(z>>ELE_SHIFT)+dz;
		index=MAP[MAP_INDEX(mx,mz)].MapWho;
		while(index)
		{
			t_thing	= TO_THING(index);
			if(t_thing!=p_thing)
			{
				LogText(" found a thing at dx dz %d %d of type %d\n",dx,dz,t_thing->DrawType);
				switch(t_thing->DrawType)
				{
					case	DT_ROT_MULTI:
						LogText(" its a figure \n");
						if(check_combat_grapple_with_person(t_thing,x,y,z,p_fight,p_thing,&angle)>0)
						{
							hit+=1000;
							*p_victim=t_thing;
							
						}

						break;

				}
			}
			index	=	t_thing->Child;
		}
	}

	return(hit);

	*/
}

SLONG	find_hit_value(Thing *p_person,SLONG anim,Thing **p_victim)
{
	GameKeyFrame	*current;
	struct	GameFightCol	*fight_info;
	SLONG	x,y,z;
	SLONG	dx,dy,dz;
	SLONG	hits=0;

	current=global_anim_array[p_person->Genus.Person->AnimType][anim];

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,0,&x,&y,&z);
	x+=p_person->WorldPos.X>>8;
	y+=p_person->WorldPos.Y>>8;
	z+=p_person->WorldPos.Z>>8;
//	calc_sub_objects_position_keys(p_person,p_person->Draw.Tweened->AnimTween,0,&dx,&dy,&dz,anim_array[anim],anim_array[anim]->NextFrame);

	while(current)
	{
		if(current->Fight)
		{
			hits+=find_possible_combat_target(current->Fight,x,y,z,p_person,p_victim);
		}
		current=current->NextFrame;
	}
	return(hits);
}

SLONG	find_grapple_value(Thing *p_person,SLONG anim,Thing **p_victim,SLONG grapple)
{
	GameKeyFrame	*current;
	struct	GameFightCol	*fight_info;
	SLONG	x,y,z;
	SLONG	dx,dy,dz;
	SLONG	hits=0;

	current=global_anim_array[p_person->Genus.Person->AnimType][anim];

//	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&x,&y,&z);
	x=p_person->WorldPos.X>>8;
	y=p_person->WorldPos.Y>>8;
	z=p_person->WorldPos.Z>>8;
//	calc_sub_objects_position_keys(p_person,p_person->Draw.Tweened->AnimTween,0,&dx,&dy,&dz,anim_array[anim],anim_array[anim]->NextFrame);

	hits+=find_possible_grapple_target(0,x,y,z,p_person,p_victim,grapple);
	return(hits);
}

void	set_grapple_pos(Thing *p_person,Thing *p_victim,SLONG dist,SLONG anim,SLONG grapple)
{
	SLONG	dx,dy,dz,len;
	SLONG	angle;

	GameCoord new_position;

	dx	=-	(SIN(p_person->Draw.Tweened->Angle)*dist)>>16;
	dz	=-	(COS(p_person->Draw.Tweened->Angle)*dist)>>16;

	new_position.X = p_person->WorldPos.X+(dx<<8);
	new_position.Y = p_person->WorldPos.Y;
	new_position.Z = p_person->WorldPos.Z+(dz<<8);

	move_thing_on_map(p_victim,&new_position);

	p_victim->Draw.Tweened->Angle=(p_person->Draw.Tweened->Angle+grapples[grapple].Angle+1024)&2047;


}


SLONG	set_grapple(Thing *p_person,Thing *p_victim,SLONG anim,SLONG grapple)
{
	UBYTE taunt_prob = 0;
	set_face_thing(p_person,p_victim);
	set_anim(p_person,anim);
	p_person->Genus.Person->Target=THING_NUMBER(p_victim);
	ASSERT(p_person->Genus.Person->Target !=THING_NUMBER(p_person));
	ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
																		
	p_victim->Genus.Person->Target=THING_NUMBER(p_person);
	ASSERT(p_victim->Genus.Person->Target !=THING_NUMBER(p_victim));
	ASSERT(TO_THING(p_victim->Genus.Person->Target)->Class==CLASS_PERSON);

	switch(anim)
	{
		case	ANIM_SNAP_KNECK:
//			if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
			set_anim(p_victim,ANIM_DIE_KNECK);
			
			set_grapple_pos(p_person,p_victim,70,anim,grapple);
			p_victim->SubState=SUB_STATE_GRAPPLEE;
			p_person->SubState=SUB_STATE_GRAPPLE;

			break;
		case	ANIM_PISTOL_WHIP:
			set_anim(p_victim,ANIM_PISTOL_WHIP_TAKE);
			set_grapple_pos(p_person,p_victim,70,anim,grapple);
			p_victim->SubState=SUB_STATE_GRAPPLEE;
			p_person->SubState=SUB_STATE_GRAPPLE;
			break;

		case	ANIM_GRAB_ARM:
			set_grapple_pos(p_person,p_victim,90,anim,grapple);

			set_anim(p_victim,ANIM_GRAB_ARMV);
			p_victim->Genus.Person->Action=ACTION_GRAPPLEE;
			p_victim->Genus.Person->Escape=0;
			p_victim->SubState=SUB_STATE_GRAPPLE_HELD;
			p_person->SubState=SUB_STATE_GRAPPLE;
			break;
		case	ANIM_STRANGLE:
			set_grapple_pos(p_person,p_victim,100,anim,grapple);

			set_anim(p_victim,ANIM_STRANGLE_VICTIM);
			p_victim->Genus.Person->Action=ACTION_GRAPPLEE;
			p_victim->Genus.Person->Escape=0;
			p_victim->SubState=SUB_STATE_STRANGLEV;
			p_person->SubState=SUB_STATE_STRANGLE;
			taunt_prob=50;
			break;
		case	ANIM_HEADBUTT:
			set_grapple_pos(p_person,p_victim,100,anim,grapple);

			set_anim(p_victim,ANIM_HEADBUTT_VICTIM);
			p_victim->Genus.Person->Action=ACTION_GRAPPLEE;
			p_victim->Genus.Person->Escape=0;
			p_victim->SubState=SUB_STATE_HEADBUTTV;
			p_person->SubState=SUB_STATE_HEADBUTT;
			taunt_prob=50;
			break;
	}
	set_generic_person_state_function(p_person,STATE_FIGHTING);

	set_generic_person_state_function(p_victim,STATE_FIGHTING);
	p_victim->Genus.Person->Flags |= FLAG_PERSON_HELPLESS;
	p_person->Genus.Person->Action=ACTION_GRAPPLE;
	scare_gang_attack(p_person);

#ifndef PSX
	if ((Random()&0xff)<taunt_prob) // not too often or it gets annoying
	{
		SLONG sound=0;

		switch (p_person->Genus.Person->PersonType)
		{
/*		case PERSON_DARCI: 
			if (Random()&3)
				sound=SOUND_Range(S_DARCI_FIGHT_TAUNT_START,S_DARCI_FIGHT_TAUNT_END);
			else
				sound=SOUND_Range(S_DARCI_DEADSAFE_TAUNT_START,S_DARCI_DEADSAFE_TAUNT_END);
			break;*/
		case PERSON_ROPER:// Only Roper moves currently have a taunt_prob greater than zero...
			if (Random()&2)
				sound=SOUND_Range(S_ROPER_FIGHT_TAUNT_START,S_ROPER_FIGHT_TAUNT_END);
			else
				sound=SOUND_Range(S_ROPER_DEADSAFE_TAUNT_START,S_ROPER_DEADSAFE_TAUNT_END);
			break;
		}
		if (sound) MFX_play_thing(THING_NUMBER(p_person),sound,MFX_QUEUED|MFX_SHORT_QUEUE,p_person);
	}
#endif
	//
	// The sound of a breaking neck!
	//
/*
	PCOM_oscillate_tympanum(
		PCOM_SOUND_FIGHT,
		p_person,
		p_person->WorldPos.X >> 8,
		p_person->WorldPos.Y >> 8,
		p_person->WorldPos.Z >> 8);
*/

	return(0);
}

SLONG	find_best_grapple(Thing *p_person)
{
	SLONG	c0=0;
	SLONG	best=-1,best_hit=0,hit;
	Thing	*p_target=0,*best_target=0;
	SLONG	only_kneck=0;
	SLONG	iam=1;
#ifndef	PSX
	if(p_person->Genus.Person->AnimType==ANIM_TYPE_ROPER)
	{

		iam=2;
	}
#endif

/*
	if (p_person->Genus.Person->PersonType != PERSON_DARCI)
	{
		//
		// Only let Darci grapple because otherwise set_anim() crashes
		//

		return NULL;
	}
*/
	// don't let roper grapple, cos junior asked for him not to
//	if (p_person->Genus.Person->AnimType == ANIM_TYPE_ROPER) return NULL;
	//

	if(!p_person->Genus.Person->PlayerID || p_person->Genus.Person->Mode==PERSON_MODE_FIGHT)
	{
		//
		// skip neck snap for non players
		//
		c0=1;
	}
	if((p_person->Genus.Person->PlayerID && p_person->SubState==SUB_STATE_STEP_FORWARD && p_person->Draw.Tweened->FrameIndex<3) || ((!p_person->Genus.Person->PlayerID) && ((GAME_TURN+THING_NUMBER(p_person))&0x3)==0))
	{
		only_kneck=0;
	}
	else
	{
		only_kneck=1;
		if(c0==1)
		{
			//
			//kneck snap has been skipped
			//
			return(0);
		}

	}

		
	while(grapples[c0].Anim)
	{
		if(grapples[c0].Peep&iam)
		{
			hit=find_grapple_value(p_person,grapples[c0].Anim,&p_target,c0);
			if(hit>best_hit)
			{
				best=c0;
				best_hit=hit;
				best_target=p_target;
			}
		}

		c0++;
		if(only_kneck && c0==1)
		{
			break;
		}

	}

	if(best>=0)
	{
//		if(best_target)
		if(best_target&&(best_target->Genus.Person->AnimType!=ANIM_TYPE_ROPER)) // junior doesn't want darci grappling roper.
		{
			if (best_target->Genus.Person->PersonType == PERSON_MIB1 ||
				best_target->Genus.Person->PersonType == PERSON_MIB2 ||
				best_target->Genus.Person->PersonType == PERSON_MIB3)
			{
				//
				// You can't throw MIB
				// 

				return FALSE;
			}

			if (grapples[best].Anim == ANIM_PISTOL_WHIP)
			{
				//
				// You can't pistol whip people the combat training people.
				//

				if (best_target->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
				{
					return FALSE;
				}
			}

			if (grapples[best].Anim == ANIM_STRANGLE ||
				grapples[best].Anim == ANIM_HEADBUTT)
			{
				//
				// The victim is thrown forawrd at the end of some grapples. If
				// they are too near a wall or a fence, they will go through.
				//

				extern SLONG is_there_room_in_front_of_me(Thing *p_person, SLONG how_much_room);

				if (!is_there_room_in_front_of_me(p_person, 150))
				{
/*
					extern void add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text);

					add_damage_text(
						p_person->WorldPos.X          >> 8,
						p_person->WorldPos.Y + 0x6000 >> 8,
						p_person->WorldPos.Z          >> 8,
						"Better not grapple!");
*/

					return FALSE;
				}
			}

			set_grapple(p_person,best_target,grapples[best].Anim,best);
			return(1);
		}
		return(0); //grapple[best]);
	}
	else
		return(0);
}

#ifdef	UNUSED
void	set_combo_history(SLONG history,SLONG power,SLONG index)
{
	SLONG	pos;
	pos=combo_histories[history].Index;

	combo_histories[history].Power[pos]=power;
	combo_histories[history].Moves[pos]=index;
	combo_histories[history].Times[pos]=(UWORD)GAME_TURN;
	combo_histories[history].Index=(++pos)%MAX_MOVES;
	if(combo_histories[history].Count<MAX_MOVES)
		combo_histories[history].Count++;
	combo_histories[history].LastUsed=(UWORD)GAME_TURN;


}

void	set_combo_history_result(Thing *p_person,SLONG res)
{
	SLONG	pos;
	SLONG	history;

	history=p_person->Genus.Person->ComboHistory;

	pos=combo_histories[history].Index;
	pos--;
	if(pos<0)
		pos=MAX_MOVES-1;

	combo_histories[history].Result[pos]=res;

}

void	set_block_history_result(UBYTE history,SLONG perp,SLONG res,SLONG height)
{
	SLONG	pos;
	pos=block_histories[history].Index;

	block_histories[history].Attack[pos]=height;
	block_histories[history].Times[pos]=(UWORD)GAME_TURN;
	block_histories[history].Perp[pos]=(UWORD)perp;
	block_histories[history].Flags[pos]=res;
	block_histories[history].Index=(++pos)%MAX_MOVES;
	if(block_histories[history].Count<MAX_MOVES)
		block_histories[history].Count++;
	block_histories[history].LastUsed=(UWORD)GAME_TURN;


}
#endif
/*
void	set_block_history(SLONG history,SLONG attack,SLONG perp)
{
	SLONG	pos;
	pos=block_histories[history].Index;

	block_histories[history].Attack[pos]=attack;
	block_histories[history].Times[pos]=(UWORD)GAME_TURN;
	block_histories[history].Perp[pos]=(UWORD)perp;
	block_histories[history].Flags[pos]=0;
	block_histories[history].Index=(++pos)%MAX_MOVES;
	if(block_histories[history].Count<MAX_MOVES)
		block_histories[history].Count++;
	block_histories[history].LastUsed=(UWORD)GAME_TURN;


}

void	set_block_history_result(Thing *p_person,SLONG attack,SLONG perp,SLONG res,SLONG height)
{
	SLONG	pos;
	SLONG	history;

	history=p_person->Genus.Person->BlockHistory;

	pos=block_histories[history].Index;
	count=combo_histories[history].Count;
	while(count)
	{
		pos--;
		if(pos<0)
			pos=MAX_MOVES-1;
		if(block_histories[pos].Perp==perp && block_histories[pos].Attack==attack)
		{
			block_histories[history].Flags[pos]=res;
			block_histories[history].Attack[pos]=height;
			return;
		}
		count--;
	}
}
*/

void func(void)
{
//	printf("hello");
}

SLONG find_anim_fight_height(SLONG anim,SLONG person)
{
	struct GameKeyFrame		*f;
	
	f=global_anim_array[person][anim];
	while(f)
	{
		if(f->Fight)
		{
			return(f->Fight->Height);
		}
		if(f->Flags&ANIM_FLAG_LAST_FRAME)
			return(0);
		f=f->NextFrame;
	}
	return(0);
}

SLONG	should_i_block(Thing *p_person,Thing *p_agressor,SLONG anim)
{
	SLONG	pos;
	SLONG	history;
	SLONG	count;
	SLONG	perp;
	SLONG	block_prob=(20*256)/100; //20%  <<8
	SLONG	fheight;
	SLONG	same=0,other=0;
	SLONG	can_see=0;

	if(p_person->Genus.Person->PlayerID)
	{
		if(p_person->SubState==SUB_STATE_STEP_FORWARD)
		{
			//
			// player moving backwards is autoblock
			//
			if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STEP_S)
			{
				return(1);
			}


		}
		return(0);
	}

	fheight=find_anim_fight_height(anim,p_agressor->Genus.Person->AnimType);

	perp=THING_NUMBER(p_agressor);
	/*
	history=p_person->Genus.Person->BlockHistory;
	if(history==0)
	{
		block_prob-=20;
	}
	else
	{

		//
		// the prob of blocking is equal to the probability of that person doing that move
		//
		pos=block_histories[history].Index;
		count=combo_histories[history].Count;
		while(count&& (same+other)<4)
		{
			pos--;
			if(pos<0)
				pos=MAX_MOVES-1;
			// the right chap
			if(block_histories[history].Perp[pos]==perp)// && block_histories[pos].Attack==attack)
			{

				//
				// count the number of times that move has been done and not done
				//
				if(block_histories[history].Attack[pos]==fheight)
					same++;
				else
					other++;
			}
			count--;
		}							 
		if(same+other>0)
		{

			block_prob+=(same*256)>>2;///(same+other);
		}
	}
	block_prob+=GET_SKILL(p_person)*12;
	*/

	block_prob=60+GET_SKILL(p_person)*12;

	if(!can_a_see_b(p_person,p_agressor))
	{
		//
		// if you can't see the enemy then reduce chances of blocking
		//
		block_prob>>=1;

	}

	//
	// must never be impossible to hit clipped to a skill related value
	//
	if(block_prob>150+GET_SKILL(p_person)*5)
		block_prob=150+GET_SKILL(p_person)*5;

#ifndef	PSX
	{
		CBYTE	str[100];
		sprintf(str,"block prob %d    same %d other %d \n",(block_prob*100)>>8,same,other);
		//CONSOLE_text(str);
	}
#endif

	
	if((rand()&255)<block_prob)
		return(1);
	else
		return(0);

}

#ifdef	UNUSED
void	calc_combo_power(Thing *p_person)
{
	SLONG	history,pos;
	SLONG	count,moves=0;
	history=p_person->Genus.Person->ComboHistory;
	pos=combo_histories[history].Index;
	pos--;
	count=combo_histories[history].Count;
	while(count)
	{
		if(pos<0)
			pos=MAX_MOVES-1;

		if((UWORD)GAME_TURN-combo_histories[history].Times[pos]>70)
			break;
		if(combo_histories[history].Result[pos]==1)
		{
			moves++;
		}
		else
			moves--;

		pos--;
		count--;
	}
	if(moves<0)
		moves=0;
	p_person->Genus.Person->Power=moves;
	
}


SLONG	find_best_punch(Thing *p_person, ULONG flag)
{
	SLONG	c0=0;
	SLONG	best=-1,best_hit=0,hit;
	Thing	*p_target=0,*best_target=0;
	SLONG	power,best_power;
	SLONG	history;


	calc_combo_power(p_person);

	power=p_person->Genus.Person->Power;
	history=p_person->Genus.Person->ComboHistory;
	if(!history)
		history=get_history(p_person);

	power>>=1;
	best_power=power;
	while(power>=0)
	{
		c0=0;
		if(best_power==0&&punches[power][0])
			best_power=power;

		while(punches[power][c0])
		{
			hit=find_hit_value(p_person,punches[power][c0],&p_target);
			if(hit>best_hit)
			{
				best=c0;
				best_hit=hit;
				best_target=p_target;
			}
			c0++;
		}
		if(best>=0)
			break;
		power--;
	}

	if(best>=0)
	{
extern	SLONG	set_face_thing(Thing *p_person,Thing *p_target);
		if(best_target)
			set_face_thing(p_person,best_target);
		set_combo_history(history,power,best);
		ASSERT(punches[power][best]);
		return(punches[power][best]);
	}
	else
	{
		if (flag & FIND_BEST_USE_DEFAULT)
		{
			ASSERT(punches[best_power][0]);
			return(punches[best_power][0]);
		}
		else
		{
			return(0);
		}
	}
}

SLONG	find_best_kick(Thing *p_person, ULONG flag)
{
	SLONG	c0=0;
	SLONG	best=-1,best_hit=-1,hit;
	Thing	*p_target;
	SLONG	power,best_power=0;
	SLONG	history;

	calc_combo_power(p_person);
	power=p_person->Genus.Person->Power;
	history=p_person->Genus.Person->ComboHistory;
	if(!history)
		history=get_history(p_person);

	power>>=1;
	while(power>=0)
	{
		c0=0;
		if(best_power==0&&punches[power][0])
			best_power=power;

		while(kicks[power][c0])
		{
			hit=find_hit_value(p_person,kicks[power][c0],&p_target);
			if(hit>best_hit)
			{
				best=c0;
				best_hit=hit;
			}


			c0++;
		}
		if(best>=0)
			break;

		power--;
	}

	if(best>=0)
	{

		set_combo_history(history,power,best);
		return(kicks[power][best]);
	}
	else
	{
		//
		// Did not find a connecting kick.
		//

		if (flag & FIND_BEST_USE_DEFAULT)
		{
			return kicks[best_power][0];
		}
		else
		{
			return NULL;
		}
	}
}
#endif

#ifndef PSX
#ifndef TARGET_DC
void	show_fight_range(Thing	*p_thing)
{
	SLONG	temp_angle,temp_angle2;
	SLONG	x,z;
	struct	GameFightCol	*fight;
	SLONG	dist;

	x=p_thing->WorldPos.X>>8;
	z=p_thing->WorldPos.Z>>8;


	fight=p_thing->Draw.Tweened->CurrentFrame->Fight;
	temp_angle=+(fight->Angle<<3)-p_thing->Draw.Tweened->Angle;
	temp_angle-=(FIGHT_ANGLE_RANGE>>1);
	if(temp_angle<0)
		temp_angle=2048+temp_angle;
	temp_angle=temp_angle&2047;

	e_draw_3d_line(x,0,z,x+(COS(temp_angle)>>8),0,z+(SIN(temp_angle)>>8) );

	temp_angle2=temp_angle+FIGHT_ANGLE_RANGE;
	temp_angle2=temp_angle2&2047;
	e_draw_3d_line(x,0,z,x+(COS(temp_angle2)>>8),0,z+(SIN(temp_angle2)>>8) );
/*
	dist=fight->Dist1;
	e_draw_3d_line(x+((SIN(temp_angle)*dist)>>16),0,z+((COS(temp_angle)*dist)>>16),x+((SIN(temp_angle2)*dist)>>16),0,z+((COS(temp_angle2)*dist)>>16) );

	dist=fight->Dist2;
	e_draw_3d_line(x+((SIN(temp_angle)*dist)>>16),0,z+((COS(temp_angle)*dist)>>16),x+((SIN(temp_angle2)*dist)>>16),0,z+((COS(temp_angle2)*dist)>>16) );
*/
}
#endif
#endif


//
// using the combat distances
//
SLONG	check_combat_hit_with_person(Thing	*p_victim,MAPCO16 x,MAPCO16 y,MAPCO16 z,struct GameFightCol *fight,Thing *p_agressor,SLONG *ret_angle)
{
	SLONG	dist,dx,dy,dz,adx,adz;
	SLONG	mx,my,mz;
	SLONG	victim_add_y=0;

	if(p_agressor->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STOMP) 
	{
	//	if((p_victim->Genus.Person->Flags&FLAG_PERSON_KO)==0)
		if(!is_person_ko_and_lay_down(p_victim))
			return(0);
		victim_add_y=100;
	}
	if(p_victim->Genus.Person->Flags&FLAG_PERSON_KO)
	{
		if(p_agressor->Draw.Tweened->CurrentAnim!=ANIM_FIGHT_STOMP) 
		{
			return(0);
		}
	}
	else
		if(is_person_dead(p_victim))
			return(0);
	

	calc_sub_objects_position(p_victim,p_victim->Draw.Tweened->AnimTween,0,&mx,&my,&mz);

	mx+=p_victim->WorldPos.X>>8;
	my+=p_victim->WorldPos.Y>>8;
	mz+=p_victim->WorldPos.Z>>8;

	LogText(" thing pos %d %d %d \n",mx,my,mz);

	dx=mx-x;
	dy=my-y;
	dz=mz-z;

	LogText(" dx %d dz %d \n",dx,dz);

	adx=abs(dx);
	adz=abs(dz);

	//
	// compares my pelvis with his, but what if he is lay down and i am stood up
	//
	if(abs(dy+victim_add_y)>100)
	{
//		ASSERT(0);	// Just for now!!!

		return(0); // height wrong
	}

	if (p_agressor->Draw.Tweened->CurrentAnim == ANIM_FIGHT_STOMP)
	{
		//
		// Some special stomp hit code! Find out where the agressors foot is.
		// 

		SLONG fx;
		SLONG fy;
		SLONG fz;
		SLONG	pass=0;

		calc_sub_objects_position(
			p_agressor,
			p_agressor->Draw.Tweened->AnimTween,
			SUB_OBJECT_RIGHT_FOOT,
		   &fx,
		   &fy,
		   &fz);

		fx += p_agressor->WorldPos.X >> 8;
		fy += p_agressor->WorldPos.Y >> 8;
		fz += p_agressor->WorldPos.Z >> 8;


	
		for(pass=0;pass<3;pass++)
		{

			if(pass>0)
			{
				SLONG	ob=SUB_OBJECT_HEAD;
				if(pass==2)
					ob=SUB_OBJECT_RIGHT_TIBIA;


				calc_sub_objects_position(p_victim,p_victim->Draw.Tweened->AnimTween,ob,&mx,&my,&mz);

				mx+=p_victim->WorldPos.X>>8;
				my+=p_victim->WorldPos.Y>>8;
				mz+=p_victim->WorldPos.Z>>8;


			}

			dx = abs(mx - fx);
			dz = abs(mz - fz);

			dist = QDIST2(dx,dz);

			if (dist < 0x53)
			{
				//
				// Near enough to pelvis!
				//

				return TRUE;
			}
		}
		return FALSE;
	}


	dist=QDIST2(adx,adz);

	LogText(" dist %d range %d-%d \n",dist,fight->Dist1,fight->Dist2);

	#define COMBAT_HIT_DIST_LEEWAY 0x30

//	if (dist > fight->Dist1 && dist < fight->Dist2)

//	if (WITHIN(dist, fight->Dist1 - COMBAT_HIT_DIST_LEEWAY, fight->Dist2 + COMBAT_HIT_DIST_LEEWAY))
	//
	// if distance pretty near do an angle check
	//
	if (WITHIN(dist, 0, fight->Dist2 + COMBAT_HIT_DIST_LEEWAY+128))
	{
		SLONG	angle,temp_angle;

		if(fight->Dist1<40)
		{
			//
			// we are intersecting the enemy
			//
			if(dist<35)
				return(1);
		}

		LogText(" dist ok \n");

		angle=-Arctan(-dx,dz)-512;
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		*ret_angle=angle;


		angle-=((fight->Angle<<3)-p_agressor->Draw.Tweened->Angle);

		LogText(" angle diff %d \n",angle);



		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		if(angle<(FIGHT_ANGLE_RANGE>>1)||angle>2048-(FIGHT_ANGLE_RANGE>>1))
		{
			//
			// angle is good check the distance exactly
			//
	//		if (WITHIN(dist, fight->Dist1 - COMBAT_HIT_DIST_LEEWAY, fight->Dist2 + COMBAT_HIT_DIST_LEEWAY))
			if (WITHIN(dist, 0, fight->Dist2 + COMBAT_HIT_DIST_LEEWAY))
			{
				return(1);
			}
			else
			{
				//
				// near miss
				//
				PCOM_attack_happened_but_missed(p_victim,p_agressor);

				return(0);
			}
		}
		else
		{
//			ANGLE WRONG 
			if(angle<(FIGHT_ANGLE_RANGE>>0)||angle>2048-(FIGHT_ANGLE_RANGE>>0))
			{
				//
				// near miss
				//
				PCOM_attack_happened_but_missed(p_victim,p_agressor);

			}
			return(0);
		}
	}
	else
	{
		//
		// Distance Wrong
		//
		return(0);
	}

}

SLONG	check_combat_grapple_with_person(Thing	*p_victim,MAPCO16 x,MAPCO16 y,MAPCO16 z,struct GameFightCol *fight,Thing *p_agressor,SLONG *ret_angle,SLONG grapple)
{
	SLONG	dist,dx,dy,dz,adx,adz;
	SLONG	mx,my,mz;
	struct	Grapples	*pg;

	pg=&grapples[grapple];

//	calc_sub_objects_position(p_victim,p_victim->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&mx,&my,&mz);
	mx=p_victim->WorldPos.X>>8;
	my=p_victim->WorldPos.Y>>8;
	mz=p_victim->WorldPos.Z>>8;



	dx=mx-x;
	dy=my-y;
	dz=mz-z;



	adx=abs(dx);
	adz=abs(dz);

	if(abs(dy)>50)
		return(0); // height wrong

	dist=QDIST2(adx,adz);
/*
#ifndef	PSX
	{
		CBYTE	str[100];
		sprintf(str,"grapple dist %d  dx %d dz %d\n",dist,dx,dz);
		CONSOLE_text(str);
	}
#endif
*/



	if(abs(dist-pg->Dist)<pg->Range) //if(dist>10 && dist<140) 
	{
		SLONG	angle,temp_angle;

		LogText(" dist ok \n");

		//
		// Angle I'm looking at relative to him
		// I'e for a kneck snap should be looking in same dir

		angle=((p_victim->Draw.Tweened->Angle+2048)&2047)-((p_agressor->Draw.Tweened->Angle+2048)&2047);
		angle=(angle+2048)&2047;

//		MSG_add(" his %d mine %d diff %d \n",p_victim->Draw.Tweened->Angle,p_agressor->Draw.Tweened->Angle,angle);

		angle+=(pg->Angle-1024)&2047;

		if(angle>160&&angle< 2048-160)
			return(0);


		angle=Arctan(-dx,dz);
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		*ret_angle=angle;

//		e_draw_3d_line_col(x,y,z,x+(COS(angle)>>8),y,z+(SIN(angle)>>8) ,255,0,0);


		MSG_add(" angle to him %d myangle %d\n",angle,p_agressor->Draw.Tweened->Angle);

		//
		// victim should be in the direction I'm facing
		//
		angle-=(p_agressor->Draw.Tweened->Angle);




		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		if(angle<(FIGHT_ANGLE_RANGE>>1)||angle>2048-(FIGHT_ANGLE_RANGE>>1))
		{
			SLONG dx,dz;
			LogText(" angle ok \n");
			return(1);
		}

		else
		{
//			MSG_add(" ANGLE WRONG  angle dif %d",angle);
			return(0);
		}
		
	}
	else
	{
		return(0);
	}

}


//
// Returns TRUE if he is behind me.
// 

SLONG check_hit_from_behind(Thing *p_me, Thing *p_him)
{
	SLONG mx;
	SLONG my;
	SLONG mz;

	SLONG hx;
	SLONG hy;
	SLONG hz;

	SLONG dx;
	SLONG dz;

	SLONG dprod;

	ASSERT(p_me ->Class == CLASS_PERSON);
	ASSERT(p_him->Class == CLASS_PERSON);

	//
	// My pelvis.
	//

	calc_sub_objects_position(
		p_me,
		p_me->Draw.Tweened->AnimTween,
		0,
	   &mx,
	   &my,
	   &mz);

	mx += p_me->WorldPos.X >> 8;
	mz += p_me->WorldPos.Z >> 8;

	//
	// His pelvis.
	//

	calc_sub_objects_position(
		p_him,
		p_him->Draw.Tweened->AnimTween,
		0,
	   &hx,
	   &hy,
	   &hz);

	hx += p_him->WorldPos.X >> 8;
	hz += p_him->WorldPos.Z >> 8;

	//
	// The direction I am facing in.
	//

	dx = -SIN(p_me->Draw.Tweened->Angle);
	dz = -COS(p_me->Draw.Tweened->Angle);

	//
	// The vector from me to him.
	//

	SLONG vx = hx - mx;
	SLONG vz = hz - mz;

	dprod = dx*vx + dz*vz;

	if (dprod < 0)
	{
		//
		// He is behind me.
		//

		return TRUE;
	}
	else
	{
		//
		// He is in front of me.
		//

		return FALSE;
	}
}




#if 0

SLONG	check_hit_from_behind(Thing	*p_victim,struct GameFightCol *fight,Thing *p_agressor,SLONG *ret_angle)
{
	SLONG	dist,dx,dy,dz,adx,adz;
	SLONG	mx,my,mz;
	SLONG	x,y,z;

	calc_sub_objects_position(p_agressor,p_agressor->Draw.Tweened->AnimTween,0,&x,&y,&z);

	x+=p_agressor->WorldPos.X>>8;
	y+=p_agressor->WorldPos.Y>>8;
	z+=p_agressor->WorldPos.Z>>8;

	calc_sub_objects_position(p_victim,p_victim->Draw.Tweened->AnimTween,0,&mx,&my,&mz);

	mx+=p_victim->WorldPos.X>>8;
	my+=p_victim->WorldPos.Y>>8;
	mz+=p_victim->WorldPos.Z>>8;

	LogText(" thing pos %d %d %d \n",mx,my,mz);

	dx=mx-x;
	dy=my-y;
	dz=mz-z;

	LogText(" dx %d dz %d \n",dx,dz);


	if(abs(dy)>150)
		return(0); // height wrong


	{
		SLONG	angle,temp_angle;


		angle=Arctan(-dx,dz);
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		*ret_angle=angle;

		MSG_add(" angle to him %d hisangle %d\n",angle,p_victim->Draw.Tweened->Angle);
		angle-=(p_victim->Draw.Tweened->Angle);



		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;

		MSG_add("RESULT angle %d \n",angle);

		if(angle>400 && angle<1024+600)
		{
			return(0);
		}
		else
		{
			return(1);

		}
/*
		if(angle<(FIGHT_ANGLE_RANGE>>1)||angle>2048-(FIGHT_ANGLE_RANGE>>1))
		{
			return(-2);
		}

		else
		{
			SLONG dx,dz;
			LogText(" angle ok \n");
			return(1);
		}
*/		
	}

}

#endif


/*

//
// behind, where they hit from behind?
// height 0,1,2  (feet, torso head)
//

void set_person_dead_combat(Thing *p_thing,Thing *p_aggressor,SLONG death_type,SLONG behind,SLONG height)
{
	DrawTween  *draw_info;
	WaveParams  die;
	UWORD		frame;
	
	ASSERT(WITHIN(behind, 0, 1));
	ASSERT(WITHIN(height, 0, 2));

	//
	// Make the person do the correct animation for the height
	// at which the attack occured.
	//
	
	draw_info               = p_thing->Draw.Tweened;
	frame                   = knock_down[behind*3+height][0];
	draw_info->CurrentFrame	= global_anim_array[p_thing->Genus.Person->AnimType][frame];
	draw_info->CurrentAnim	= frame;
	draw_info->NextFrame	= draw_info->CurrentFrame->NextFrame;
	draw_info->QueuedFrame	= 0;
	draw_info->AnimTween	= 0;

	//
	// Create a sound effect.
	// 

	die.Priority				=	0;
	die.Flags					=	WAVE_CARTESIAN;
	die.Mode.Cartesian.Scale	=	(128<<8);
	die.Mode.Cartesian.X		=	p_thing->WorldPos.X;
	die.Mode.Cartesian.Y		=	p_thing->WorldPos.Y;
	die.Mode.Cartesian.Z		=	p_thing->WorldPos.Z;

	switch(p_thing->Genus.Person->PersonType)
	{
		case	PERSON_COP:
			play_quick_wave_old(&die,S_MALE_DIE_2,0,0);
			break;
		case	PERSON_DARCI:
			switch(death_type)
			{
				case HIT_TYPE_GUN_SHOT_H:
				case HIT_TYPE_GUN_SHOT_M:
				case HIT_TYPE_GUN_SHOT_L:
				default:
					play_quick_wave_old(&die,S_FEMALE_DIE_2,0,0);
			}
			break;
	}

	//
	// Make the person die.
	//

	set_person_dying(p_thing);

	//
	// If we know who killed us...
	//

	if (p_aggressor)
	{
		p_aggressor->Genus.Person->Target = NULL;
		p_aggressor->Genus.Person->InWay  = NULL;

		if(p_aggressor->Genus.Person->PlayerID)
		{
			//
			// Its a player so keep score
			//

			GAME_SCORE(p_aggressor->Genus.Person->PlayerID-1) += 50;
		}
	}
}

*/

SLONG is_combo_anim(SLONG anim)
{
	return
		anim == ANIM_PUNCH_COMBO3  ||
		anim == ANIM_PUNCH_COMBO3b ||
		anim == ANIM_KICK_COMBO3   ||
		anim == ANIM_KICK_COMBO3b;
}



extern	Thing	*talk_thing;
void	check_eway_talk(SLONG stop);

SLONG	apply_hit_to_person(Thing *p_thing,SLONG angle,SLONG type,SLONG damage,Thing *p_aggressor,struct GameFightCol *fight)
{
	SLONG		hit_wave;
	DrawTween	*draw_info;
//	WaveParams	hit;
	SLONG		behind;
	SLONG		block=0;
	UBYTE		player_hit=0;
	UBYTE		skill=1,shot=0,stomped=0,ko=0,ko_ed=0,batted=0,knifed=0;
	GameCoord	vec;
	SLONG	nad=0, taunt_prob=15;

	switch(type)
	{
		case	HIT_TYPE_GUN_SHOT_H:
		case	HIT_TYPE_GUN_SHOT_M:
		case	HIT_TYPE_GUN_SHOT_L:

			
		case	HIT_TYPE_GUN_SHOT_PISTOL:
		case	HIT_TYPE_GUN_SHOT_SHOTGUN:
		case	HIT_TYPE_GUN_SHOT_AK47:
			shot=1;
			if(!p_thing->Genus.Person->PlayerID)
				p_thing->Flags|=FLAGS_PERSON_BEEN_SHOT;
	}						

	if(p_thing->Genus.Person->PlayerID)
	{
		player_hit=1;
	}

	if(p_aggressor)
	{
		SLONG	node;
		SLONG	hit_anim,hit;
		ASSERT(p_aggressor->Class==CLASS_PERSON);


		if(player_hit||p_aggressor->Genus.Person->PlayerID)
		{
			//
			// one of them is a player
			//
			if(!(p_thing->Genus.Person->Flags&FLAG_PERSON_KO)) //target not lay down
			{
				//
				// can our fist physically reach them?
				//
				if(!shot)
				if(!there_is_a_los_things(p_thing,p_aggressor,LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG|LOS_FLAG_IGNORE_PRIMS|LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
				{
					return(0);
				}
			}
		}

		node=p_aggressor->Genus.Person->CombatNode;
		if(node<0)
			node=0;
//		ASSERT(node>=0);

//		damage=0;

		hit_anim=p_aggressor->Draw.Tweened->CurrentAnim;
		switch(hit_anim)
		{
			case	ANIM_KICK_NAD:
				if (SOUND_Gender(p_thing)==1)
				{
					// has nads
					damage+=50;
					nad=1;
					taunt_prob=75; // a lot more common for comedic effect
				}
				else
				{
					damage+=10;
				}
				break;
			case	ANIM_BAT_HIT1:
			case	ANIM_BAT_HIT2:
				batted=1;   //fall through
			case	ANIM_FLYKICK_START:
			case	ANIM_FLYKICK_LAND:
			case	ANIM_FLYKICK_FALL:
				// these are all fancy moves so let's taunt more often
				taunt_prob+=10;	
							// fall through
			case	ANIM_KICK_BEHIND:
				damage+=20; //fall through
			case	ANIM_KICK_RIGHT:
			case	ANIM_KICK_LEFT:
				damage+=30; //fall through
			case	ANIM_PUNCH_COMBO3:
			case	ANIM_PUNCH_COMBO3b:
			case	ANIM_KICK_COMBO3:
			case	ANIM_KICK_COMBO3b:

				if (p_thing->Genus.Person->PersonType == PERSON_MIB1 ||
					p_thing->Genus.Person->PersonType == PERSON_MIB2 ||
					p_thing->Genus.Person->PersonType == PERSON_MIB3)
				{
					//
					// You can't knock out MIB! They're too hard.
					//
				}
				else
				if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
				{
					//
					// Invulnerable people can't be knocked out!
					//
				}
				else
				{
					ko = TRUE;
				}

				if (p_thing->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
				{
					//
					// Fight test dummys can die from special moves. Even if they
					// are invulnerable.
					//

					if (((p_thing->Genus.Person->pcom_ai_other & PCOM_COMBAT_COMBO_PPP) && (hit_anim == ANIM_PUNCH_COMBO3)                            ) ||
						((p_thing->Genus.Person->pcom_ai_other & PCOM_COMBAT_COMBO_KKK) && (hit_anim == ANIM_KICK_COMBO3)                             ) ||
						((p_thing->Genus.Person->pcom_ai_other & PCOM_COMBAT_COMBO_ANY) && is_combo_anim(hit_anim)                                    ) ||
						((p_thing->Genus.Person->pcom_ai_other & PCOM_COMBAT_SIDE_KICK) && (hit_anim == ANIM_KICK_RIGHT || hit_anim == ANIM_KICK_LEFT)) ||
						((p_thing->Genus.Person->pcom_ai_other & PCOM_COMBAT_BACK_KICK) && (hit_anim == ANIM_KICK_BEHIND)                             ))
					{
						ko = TRUE;

						p_thing->Genus.Person->Health = 0;
					}
				}

//				damage+=4;
				break;
			case	ANIM_FIGHT_STOMP:
				damage+=50;
				stomped=1;
				break;
			case	ANIM_KICK_NEAR:
				damage=40;
				break;
			case	ANIM_KNIFE_ATTACK1:
				damage=30;
				knifed=1;
				break;
			case	ANIM_KNIFE_ATTACK2:
				damage=50;
				knifed=1;
				break;
			case	ANIM_KNIFE_ATTACK3:
				damage=70;
				knifed=1;
				break;

		}

		if ((Random()&0xff)<taunt_prob) // not too often or it gets annoying
		{
			SLONG sound=0;

			switch (p_aggressor->Genus.Person->PersonType)
			{
			case PERSON_DARCI:
				if (Random()&3)
					sound=SOUND_Range(S_DARCI_FIGHT_TAUNT_START,S_DARCI_FIGHT_TAUNT_END);
				else
					sound=SOUND_Range(S_DARCI_DEADSAFE_TAUNT_START,S_DARCI_DEADSAFE_TAUNT_END);
				break;
			case PERSON_ROPER:
				if (Random()&2)
					sound=SOUND_Range(S_ROPER_FIGHT_TAUNT_START,S_ROPER_FIGHT_TAUNT_END);
				else
					sound=SOUND_Range(S_ROPER_DEADSAFE_TAUNT_START,S_ROPER_DEADSAFE_TAUNT_END);
				break;
			case PERSON_COP:
				sound=SOUND_Range(S_COP_TAUNT_START,S_COP_TAUNT_END);
				break;
			case PERSON_CIV:
				break;
			case PERSON_THUG_RASTA:
				sound=SOUND_Range(S_RASTA_TAUNT_START,S_RASTA_TAUNT_END);
				break;
			case PERSON_THUG_GREY:
				sound=SOUND_Range(S_GGREY_TAUNT_START,S_GGREY_TAUNT_END);
				break;
			case PERSON_THUG_RED:
				if (THING_NUMBER(p_aggressor)&1)
					sound=SOUND_Range(S_GRED_TAUNT_START,S_GRED_TAUNT_END);
				else
					sound=SOUND_Range(S_GRED2_TAUNT_START,S_GRED2_TAUNT_END);
				break;
			}
			if (sound) MFX_play_thing(THING_NUMBER(p_aggressor),sound,MFX_QUEUED|MFX_SHORT_QUEUE,p_aggressor);
		}


		if(hit=fight_tree[node][FIGHT_TREE_DAMAGE])
		{
			damage+=hit;
		}

//		ASSERT(damage);

	}
	else
	if(damage<30)
	{
		damage=30;
	}
	if(is_person_ko_and_lay_down(p_thing))
	{
		//
		// checked all through this function, so may as well have a local flag
		//

		//
		// victim is unconscious  
		//
		ko_ed=1;
	}



	if(p_aggressor->Genus.Person->PersonType==PERSON_ROPER)
	{
		if(shot)
			damage<<=1;
		else
			damage+=20;

	}
	if(p_aggressor->Genus.Person->PlayerID)
	{
		//
		// more skill better aim with guns, more strength harder you hit 
		//
		if(shot)
			damage+=NET_PLAYER(p_aggressor->Genus.Person->PlayerID-1)->Genus.Player->Skill;
		else
			damage+=NET_PLAYER(p_aggressor->Genus.Person->PlayerID-1)->Genus.Player->Strength;
	}

	if(player_hit)
	{

		//
		// player hit by player
		//
void	PCOM_new_gang_attack(Thing *p_person, Thing *p_target);
		if(shot==0)
		if(p_aggressor->Genus.Person->PlayerID)
		{
			//
			// give victim a gang attack group
			//
			PCOM_new_gang_attack(p_aggressor, p_thing);
		}

		damage>>=1;
	}
	else
	{
		//
		// skillful people get hurt less
		//
		skill=GET_SKILL(p_thing);
		damage-=skill;

		if(damage<=0)
		{
			damage=2;
		}

	}

	//
	//skillful people hurt you more
	//
	damage+=GET_SKILL(p_aggressor);

	{
		extern void emergency_uncarry(Thing *p_person);

		ASSERT(p_thing->Class == CLASS_PERSON);

		if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING)
		{
			emergency_uncarry(p_thing);
		}
	}

	if (p_thing->SubState==SUB_STATE_BLOCK && (type==0 || type > HIT_TYPE_GUN_SHOT_L))
	{
		block = 1;
	}
	else
	{
		p_thing->Genus.Person->Agression-=damage;
		if(p_thing->Genus.Person->Agression>0)
			p_thing->Genus.Person->Agression=0;

		
		p_aggressor->Genus.Person->Agression+=damage;
		if(p_aggressor->Genus.Person->Agression<0)
			p_aggressor->Genus.Person->Agression=0;
	}

	
//	if((p_thing->Genus.Person->Flags&FLAG_PERSON_KO))

	if(ko_ed)
	{
		//
		// you are knocked out, and the bad guy is not stomping you
		//
		if(!stomped && !shot)
		{
			return(0);
		}
		else
		{
			// you are stomped so leave stomped true
		}

	}
	else
	{
		//
		// can'y be stomped if your not ko'ed
		//
		stomped=0;
	}

	if(is_person_dead(p_thing))
			return(0);

	if (p_thing->SubState==SUB_STATE_GRAPPLE_HELD||p_thing->SubState==SUB_STATE_GRAPPLE_ATTACK||p_thing->SubState==SUB_STATE_ESCAPE)
		return(0);	
	
	behind=check_hit_from_behind(p_thing,p_aggressor);
	if(behind>0)
		behind=1;
	else
		behind=0;

		//
		// if victim is player then set victim to attack aggressor
		//
	if(shot==0)
	{
		if(p_thing->Genus.Person->PlayerID)
		{
			if(p_thing->Genus.Person->Target==0)
			{
extern	void person_enter_fight_mode(Thing *p_person);
				person_enter_fight_mode(p_thing);
				p_thing->Genus.Person->Target=THING_NUMBER(p_aggressor);
				ASSERT(p_thing->Genus.Person->Target !=THING_NUMBER(p_thing));
				ASSERT(TO_THING(p_thing->Genus.Person->Target)->Class==CLASS_PERSON);
			}
		}
	}

	//should look up in a table what anim to go into given the type of hit
	//and their current state

	// for now just bodge a hit anim

	draw_info=p_thing->Draw.Tweened;

	MFX_stop(THING_NUMBER(p_thing),S_SEARCH_END);

	if(block==0)
	{
		SLONG	pain=0;
		if(p_thing==talk_thing)
		{
			PainSound(p_thing);
			pain=1;
			check_eway_talk(1);
		}

		//
		// The sound of the hit
		// 

		switch(type)
		{
			case	HIT_TYPE_GUN_SHOT_H:
			case	HIT_TYPE_GUN_SHOT_M:
			case	HIT_TYPE_GUN_SHOT_L:
		case	HIT_TYPE_GUN_SHOT_PISTOL:
		case	HIT_TYPE_GUN_SHOT_SHOTGUN:
		case	HIT_TYPE_GUN_SHOT_AK47:
/*				hit_wave	=	((rand()*(S_PUNCH_END-S_PUNCH_START))>>16)+S_PUNCH_START;
				hit_wave = S_MALE_DIE_1;
				MFX_play_xyz(0,hit_wave,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);*/

				break;
			default:
				// a droplet of blood
				calc_sub_objects_position(
					p_aggressor,
					p_aggressor->Draw.Tweened->AnimTween,
					SUB_OBJECT_LEFT_HAND,
				   &vec.X,
				   &vec.Y,
				   &vec.Z);
				vec.X+=(Random()&0x1f);
				vec.Y+=(Random()&0x1f);
				vec.Z+=(Random()&0x1f);
				vec.X<<=8; vec.Y<<=8; vec.Z<<=8;
				vec.X+=p_thing->WorldPos.X;
				vec.Y+=p_thing->WorldPos.Y;
				vec.Z+=p_thing->WorldPos.Z;


//#ifndef VERSION_GERMAN
				if(VIOLENCE)
				DIRT_new_water(vec.X>>8, vec.Y>>8, vec.Z>>8,     (Random()&0xf)-7, 0,  (Random()&0xf)-7, DIRT_TYPE_BLOOD);
//#endif
				if (knifed)
				{
//#ifndef	PSX
					hit_wave = SOUND_Range(S_STAB_START,S_STAB_END);
					MFX_play_xyz(0,hit_wave,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
//#else

#ifdef	PSX
				    MFX_play_thing(THING_NUMBER(p_aggressor),S_KNIFE_START+(Random()&1),0,p_aggressor);
#endif
//#endif

//#ifndef VERSION_GERMAN
					if(VIOLENCE)
					for (hit_wave=0;hit_wave<5;hit_wave++)
						DIRT_new_water(vec.X>>8, vec.Y>>8, vec.Z>>8,     (Random()&0xf)-7, 0,  (Random()&0xf)-7, DIRT_TYPE_BLOOD);
//#endif
				}
				else 
					if (batted)
					{
//#ifndef	PSX
						if (SOUND_Gender(p_thing)==1)
							hit_wave = SOUND_Range(S_BAT_MALE_START,S_BAT_MALE_END);
						else
							hit_wave = SOUND_Range(S_BAT_FEMALE_START,S_BAT_FEMALE_END);

//#else
//							hit_wave = S_BAT_MALE_START;
//#endif
						MFX_play_xyz(0,hit_wave,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);

					}
					else
					if (nad)
						MFX_play_thing(THING_NUMBER(p_thing),SOUND_Range(S_KICK_IN_THE_NUTS_START,S_KICK_IN_THE_NUTS_END),0,p_thing);
					else
					{
						hit_wave = S_PUNCH_START+(GAME_TURN&3);
						MFX_play_xyz(0,hit_wave,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
						if(!pain)
							PainSound(p_thing);
					}
				// do a lil bloodsplat

				if(VIOLENCE)
				{
					calc_sub_objects_position(
						p_thing,
						p_thing->Draw.Tweened->AnimTween,
						SUB_OBJECT_LEFT_HAND,
					   &vec.X,
					   &vec.Y,
					   &vec.Z);
					vec.X+=(Random()&0x1f);
					vec.Y+=(Random()&0x1f);
					vec.Z+=(Random()&0x1f);
					vec.X<<=8; vec.Y<<=8; vec.Z<<=8;
					vec.X+=p_thing->WorldPos.X;
					vec.Y+=p_thing->WorldPos.Y;
					vec.Z+=p_thing->WorldPos.Z;
	//#ifndef VERSION_GERMAN
					PARTICLE_Add(vec.X,vec.Y,vec.Z,0,0,0,
						POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7FFF0000,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE,10,75,1,20,5);
	//#endif
				}
		}

		if ((p_thing->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL) && (!p_aggressor || !p_aggressor->Genus.Person->PlayerID))
		{
			//
			// Only the player can hurt this person.
			//
		}
		else
		if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
		{
			//
			// Nothing hurts this person.
			//
		}
		else
		{
			//
			// Actually hurt the person.
			//

			p_thing->Genus.Person->Health -= damage;
#ifdef PSX
			// Shock the player dependant on damage
			if (p_thing->Genus.Person->PlayerID)
			{
				SLONG dam=(damage<<4)+96;
				if (dam>255) dam=255;
				PSX_SetShock(1,dam);
			}
#endif
#ifdef TARGET_DC
			if ( p_thing->Genus.Person->PlayerID )
			{
				Vibrate ( 5.0f, (float)( (damage<<4)+94 ) * 0.002f, 0.0f );
			}
			if ( p_aggressor->Genus.Person->PlayerID )
			{
				switch(type)
				{
					case	HIT_TYPE_GUN_SHOT_H:
					case	HIT_TYPE_GUN_SHOT_M:
					case	HIT_TYPE_GUN_SHOT_L:
					case	HIT_TYPE_GUN_SHOT_PISTOL:
					case	HIT_TYPE_GUN_SHOT_SHOTGUN:
					case	HIT_TYPE_GUN_SHOT_AK47:
						// You don't get a vibro from hitting someone with a gun (just from shooting it, done elsewhere).
						break;
					default:
						// Hand-to-hand combat. Feel the knuckles crunch into their nose.
						// Less damage-dependant jolt this time.
						Vibrate ( 5.0f, (float)( (damage<<2)+120 ) * 0.004f, 0.0f );
						break;
				}
			}
#endif

		}

		//
		// make damage value display on screen and drift upwards
		//

//		add_damage_value_thing(p_thing,damage>>1);

		if(p_thing->Genus.Person->Health<=0)
		{
			if (p_aggressor)
			{
				p_aggressor->Genus.Person->Flags2 |= FLAG2_PERSON_IS_MURDERER;
			}

			//
			// death type will one day be hit from behind or shot from front or ...
			//

			p_thing->Genus.Person->Health = 0;

			if(stomped && ko_ed==0)
			{
				ASSERT(0);
			}
			if (!is_there_room_behind_person(p_thing, behind))
			{
				behind = !behind;
			}

			if(ko_ed) //(p_thing->Genus.Person->Flags&FLAG_PERSON_KO))
			{
				p_thing->Genus.Person->Flags &= ~(FLAG_PERSON_KO | FLAG_PERSON_HELPLESS);

				set_person_dead(
					p_thing,
					p_aggressor,
					PERSON_DEATH_TYPE_COMBAT_PRONE,
					behind,
					0);
			}
			else
			if(fight)
			{
				set_person_dead(
					p_thing,
					p_aggressor,
					PERSON_DEATH_TYPE_COMBAT,
					behind,
					fight->Height & 0x3);
			}
			else
			{
				SLONG	death_type=PERSON_DEATH_TYPE_COMBAT;
extern	SLONG	really_on_floor(Thing *p_person);

				if(shot)
				{
					if(person_on_floor(p_thing)&&p_thing->OnFace==0&&really_on_floor(p_thing))
					{
						switch(type)
						{
							case	HIT_TYPE_GUN_SHOT_PISTOL:
								death_type=PERSON_DEATH_TYPE_SHOT_PISTOL;
								break;
							case	HIT_TYPE_GUN_SHOT_SHOTGUN:
								death_type=PERSON_DEATH_TYPE_SHOT_SHOTGUN;
								break;
							case	HIT_TYPE_GUN_SHOT_AK47:
								death_type=PERSON_DEATH_TYPE_SHOT_AK47;
								break;
						}
					}
					else
					{
//						ASSERT(0);
						death_type=PERSON_DEATH_TYPE_COMBAT;

					}

				}
				set_person_dead(
					p_thing,
					p_aggressor,
					death_type,
					behind,
					0);
			}

			return(damage);
		}

		if(!shot)
		{
			if(person_has_gun_out(p_thing))
			{
//				ASSERT(0);
				drop_current_gun(p_thing,0);
			}
		}

			
		if(p_aggressor->Genus.Person->PlayerID)
		{
			//its a player so keep score

			track_enemy(p_thing);

			GAME_SCORE(p_aggressor->Genus.Person->PlayerID-1)+=10;
		}

		//
		// Turn to face aggressor. unless you are a player
		// 

		if(!p_thing->Genus.Person->PlayerID)
//		if((p_thing->Genus.Person->Flags&FLAG_PERSON_KO)==0)
		if(!ko_ed)
			if(!behind)
				set_face_thing(p_thing, p_aggressor);

	//	switch(fight->Height)
		//
		// Tell the AI module that you've been hit.  (only by a player for now)
		//

//		if(p_aggressor->Genus.Person->PlayerID)
		if( !ko_ed && p_thing->Genus.Person->PlayerID==0)
		{
			PCOM_attack_happened(
				p_thing,
				p_aggressor);
		}
	}
	
//	if((p_thing->Genus.Person->Flags&FLAG_PERSON_KO)==0)
	if(!ko_ed)
	{
		SLONG	anim,flag;
		SLONG	height;
		UBYTE	history;

		if(fight)
		{
			height=fight->Height;
			height&=3;
		}
		else
		{
			height=2;
		}

		MSG_add(" take hit height %d behind %d \n",height,behind);
		ASSERT(height+behind*3<=5);
		anim = take_hit[height+behind*3][0];
		flag = take_hit[height+behind*3][1];

#ifdef	UNUSED
		history=p_thing->Genus.Person->BlockHistory;
		if(history==0)
			history=get_block_history(p_thing);
		if(block&&height!=0)
		{
			set_block_history_result(history,THING_NUMBER(p_aggressor),1,height); //1 is blocked

		}
		else
#endif

		if(block&&height!=0)
		{
		}
		else
		{
			
#ifdef	UNUSED
			set_block_history_result(history,THING_NUMBER(p_aggressor),0,height); //0 is not blocked
#endif

			

//			if(hit_anim==ANIM_KICK_RIGHT || hit_anim==ANIM_KICK_LEFT || hit_anim==ANIM_KICK_BEHIND || hit_anim==ANIM_PUNCH_COMBO3 || hit_anim==ANIM_KICK_COMBO3|| hit_anim==ANIM_KICK_COMBO3b|| hit_anim==ANIM_PUNCH_COMBO3b)
			if(ko)
			{
				//
				// not really dead, just knocked out
				//
				p_thing->Genus.Person->Agression=-60; //make them backoff

				if (!is_there_room_behind_person(p_thing, behind))
				{
					behind = !behind;
				}

				set_person_dead(
					p_thing,
					0,
					PERSON_DEATH_TYPE_STAY_ALIVE,
					behind,
					0);
			}
			else
			{
				if(height==0)
				{
					extern	void sweep_feet(Thing *p_person,Thing *p_aggressor,SLONG  death_type);

					sweep_feet(p_thing,p_aggressor,0);
				}
				else
				{
					if(shot && p_thing->Genus.Person->PlayerID && p_thing->State==STATE_MOVEING)
					{
						//
						// players don't recoil when shot if moveing
						//
						return(damage);
					}
					else
					{
						
						if(nad)
						{
							set_person_recoil(p_thing,ANIM_KICK_NAD_TAKE,flag);
						}
						else
						{
							set_person_recoil(p_thing,anim,flag);
						}
					}
				}
			}
		}
	}
	else
	{
		SLONG anim;

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
				// headbutted people cause a crash
				ASSERT(0);
				break;
		}

		set_person_ko_recoil(p_thing,anim,0);
	}

	//
	// Make sure that others can hear the hoo-har.
	//

	PCOM_oscillate_tympanum(
		PCOM_SOUND_FIGHT,
		p_aggressor,
		p_aggressor->WorldPos.X >> 8,
		p_aggressor->WorldPos.Y >> 8,
		p_aggressor->WorldPos.Z >> 8);


	if(block)
		return(0);
	else
		return(damage);
}

extern	UBYTE	semtex;

SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor)
{
	ULONG	will_hit=0xffffffff;

	if(p_agressor->Genus.Person->PlayerID==0)
	if(p_agressor->Genus.Person->Target&&p_agressor->Genus.Person->Target==THING_NUMBER(p_victim))
	{


		if(p_agressor->Genus.Person->PersonType==PERSON_COP)
			ASSERT(p_victim->Genus.Person->PersonType!=PERSON_COP);  //cop deliberatly hitting other cop
		if(p_agressor->Genus.Person->PersonType==PERSON_ROPER)
		{
			if(p_victim->Genus.Person->PersonType==PERSON_TRAMP)
			{
extern	 UBYTE	estate;
				if(estate)
					return(0);

			}

		}

		//
		// deliberate aimed blow
		//
		return(1);
	}

	//
	// Bodyguards aren't allowed to hit the person they're protecting.
	//

	if (p_agressor->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD)
	{
		UWORD i_client = EWAY_get_person(p_agressor->Genus.Person->pcom_ai_other);

		if (i_client == THING_NUMBER(p_victim))
		{
			return FALSE;
		}
	}

	//
	// People in the same gang like eachother.
	//

	if (p_victim->Genus.Person->pcom_bent & p_agressor->Genus.Person->pcom_bent & PCOM_BENT_GANG)
	{
		if (p_victim  ->Genus.Person->pcom_colour ==
			p_agressor->Genus.Person->pcom_colour)
		{
			return FALSE;
		}
	}

	switch(p_agressor->Genus.Person->PersonType)
	{
		case	PERSON_DARCI:
			//
			// won't hit cops or roper
			//

			if(p_victim->Genus.Person->Flags2&FLAG2_PERSON_GUILTY)
				return(1);

			if(VIOLENCE==0)
			{
				if(p_victim->Genus.Person->PersonType==PERSON_CIV&& (p_victim->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER))
				{
					return(0);
				}
				if((p_victim->Genus.Person->PersonType==PERSON_CIV && p_victim->Genus.Person->pcom_ai == PCOM_AI_CIV)||p_victim->Genus.Person->PersonType==PERSON_HOSTAGE)
				{
					return(0);
				}

			}

			#if DARCI_HITS_COPS
			will_hit^=(/*(1<<PERSON_COP)|*/(1<<PERSON_ROPER));
			#else
			will_hit ^= (1<<PERSON_COP)| (1<<PERSON_ROPER);
			#endif

			if(semtex)
				will_hit ^= (1<<PERSON_TRAMP);

			break;

		case	PERSON_TRAMP:
			if(semtex)
			{
				will_hit ^= (1<<PERSON_DARCI);
				will_hit^=((1<<PERSON_THUG_GREY)|(1<<PERSON_THUG_RASTA)|(1<<PERSON_THUG_RED));
			}
			
			break;

		case	PERSON_ROPER:

			if(p_agressor->Genus.Person->PlayerID==0)
			if(p_victim->Genus.Person->PersonType==PERSON_TRAMP)
			{

extern	 UBYTE	estate;
				if(estate)
				{
					return(0);
				}
			}


			if(VIOLENCE==0)
			{
				if(p_victim->Genus.Person->PersonType==PERSON_CIV&& (p_victim->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER))
				{
					return(0);
				}
				if((p_victim->Genus.Person->PersonType==PERSON_CIV && p_victim->Genus.Person->pcom_ai == PCOM_AI_CIV)||p_victim->Genus.Person->PersonType==PERSON_HOSTAGE)
				{
					return(0);
				}

			}

			//
			// won't hit darci or cops
			//
			#if DARCI_HITS_COPS
			will_hit^=(/*(1<<PERSON_COP)|*/(1<<PERSON_DARCI));
			#else
			will_hit^=((1<<PERSON_COP)|(1<<PERSON_DARCI));
			#endif
			break;

		case	PERSON_COP:
			//
			// will hit any fellon
			//
//			ASSERT(p_victim->Genus.Person->PersonType!=PERSON_COP);
			if(p_victim->Genus.Person->Flags&FLAG_PERSON_FELON)
			{
				ASSERT(p_victim->Genus.Person->PersonType!=PERSON_COP);
				return(1);
			}
			if(p_victim->Genus.Person->Flags2&FLAG2_PERSON_GUILTY)
			{
				ASSERT(p_victim->Genus.Person->PersonType!=PERSON_COP);
					return(1);
			}

			//
			// won't hit darci, other cops,roper or civilians
			//
			will_hit^=((1<<PERSON_COP)|(1<<PERSON_DARCI)|(1<<PERSON_ROPER));
/*
			#if DARCI_HITS_COPS
			will_hit^=((1<<PERSON_COP));
			#else
			will_hit^=((1<<PERSON_COP)|(1<<PERSON_DARCI)|(1<<PERSON_ROPER)|(1<<PERSON_CIV));
			#endif
*/
			break;
		case	PERSON_THUG_GREY:
		case	PERSON_THUG_RASTA:
		case	PERSON_THUG_RED:
			if(p_agressor->Genus.Person->PlayerID)
			{
				//
				// player is a thug, they can hit anyone
				//

				return(1);
			}
			//
			// can't hit other thugs or MIB's
			//
			will_hit^=((1<<PERSON_THUG_GREY)|(1<<PERSON_THUG_RASTA)|(1<<PERSON_THUG_RED));
			will_hit^=((1<<PERSON_MIB1)|(1<<PERSON_MIB2)|(1<<PERSON_MIB3));
			if(semtex)
				will_hit ^= (1<<PERSON_TRAMP);


			break;
		case	PERSON_MIB1:
		case	PERSON_MIB2:
		case	PERSON_MIB3:


			//
			// won't hit someone of same group && colour, unless it's deliberate see above
			//

/*
			if(p_victim->Genus.Person->pcom_colour==p_agressor->Genus.Person->pcom_colour &&
				p_victim->Genus.Person->pcom_group==p_agressor->Genus.Person->pcom_group)
			{
				return(0);
			}
*/

			//
			// won't hit other MIB's
			//
			will_hit^=((1<<PERSON_MIB1)|(1<<PERSON_MIB2)|(1<<PERSON_MIB3));
			break;

	}
	if(will_hit & (1<<p_victim->Genus.Person->PersonType))
	{
		//
		// The flags have spoken and they have said yes, strike him...
		//

		if ((p_victim  ->Genus.Person->pcom_bent & PCOM_BENT_GANG) &&
			(p_agressor->Genus.Person->pcom_bent & PCOM_BENT_GANG) &&
			(p_victim  ->Genus.Person->pcom_colour == p_agressor->Genus.Person->pcom_colour))
		{
			//
			// Don't hit someone in your own gang.
			//

			return(0);
		}
			
		if(p_agressor->Genus.Person->PersonType==PERSON_COP)
			ASSERT(p_victim->Genus.Person->PersonType!=PERSON_COP);  //cop deliberatly hitting other cop
		return(1);
	}
	else
	{
		return(0);
	}

}
SLONG	find_combat_target(struct GameFightCol *p_fight,SLONG x,SLONG y,SLONG z,Thing *p_thing)
{
	SLONG i;
	SLONG hit;
	SLONG angle;

	Thing *t_thing;

	//
	// Find all the people near us.
	//

//	THING_INDEX found[16];
	SLONG       found_upto;

	found_upto = THING_find_sphere(x, y, z, 0x280, found, 16, (1 << CLASS_PERSON));

	//
	// Check each person in turn.
	//

	hit = 0;

	for (i = 0; i < found_upto; i++)
	{
		t_thing = TO_THING(found[i]);

		if (t_thing != p_thing) //&& t_thing->SubState!=SUB_STATE_BLOCK)
		{
			if(people_allowed_to_hit_each_other(t_thing,p_thing))
			{

				if(t_thing->SubState!=SUB_STATE_FLIPING)
				if(t_thing->Draw.Tweened->CurrentAnim!=ANIM_SLIDER_HOLD)
				if(t_thing->Draw.Tweened->CurrentAnim!=ANIM_KICK_NS)
				if(t_thing->Draw.Tweened->CurrentAnim!=ANIM_LEG_SWEEP)
				{
					if (check_combat_hit_with_person(
							t_thing,
							x, y, z,
							p_fight,
							p_thing,
						   &angle) > 0)
					{
						hit += apply_hit_to_person(t_thing,0,0,0,p_thing,p_fight);
					}
				}
			}
		}
	}
	
	return hit;

	/*


	SLONG	dx,dz;
	SLONG	index;
	Thing	*t_thing;
	SLONG	hit=0;
	SLONG	angle;

	for(dx=-2;dx<3;dx++)
	for(dz=-2;dz<3;dz++)
	{
		SLONG	mx,mz;
		mx=(x>>ELE_SHIFT)+dx;
		mz=(z>>ELE_SHIFT)+dz;
		index=MAP[MAP_INDEX(mx,mz)].MapWho;
		while(index)
		{
			t_thing	= TO_THING(index);
			if(t_thing!=p_thing)
			{
				LogText(" found a thing at dx dz %d %d of type %d\n",dx,dz,t_thing->DrawType);
				switch(t_thing->DrawType)
				{
					case	DT_ROT_MULTI:
						LogText(" its a figure \n");
						if(check_combat_hit_with_person(t_thing,x,y,z,p_fight,p_thing,&angle)>0)
						{
							hit+=apply_hit_to_person(t_thing,0,0,0,p_thing,p_fight);
							
						}

						break;

				}
			}
			index	=	t_thing->Child;
		}
	}

	return(hit);

	*/
}


SLONG	apply_violence(Thing *p_thing)
{

	struct	GameFightCol	*fight_info;
	SLONG	hits=0,count=0;
	SLONG	x,y,z;
//		show_fight_range(p_thing);
	calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,0,&x,&y,&z);
	x+=p_thing->WorldPos.X>>8;
	y+=p_thing->WorldPos.Y>>8;
	z+=p_thing->WorldPos.Z>>8;

	LogText(" thing apply's violence\n");
	fight_info=p_thing->Draw.Tweened->CurrentFrame->Fight;

	while(fight_info&&count++<5)
	{

		LogText(" my pos %d %d %d \n",x,y,z);

		hits+=find_combat_target(fight_info,x,y,z,p_thing);
		fight_info=fight_info->Next;
	}
	ASSERT(count<5);
	return(hits);
  
}

//
// changed to return if it found someone to hit
//
SLONG find_attack_stance(
		Thing     *p_person,
		SLONG      attack_direction,
		SLONG      attack_distance,
		SLONG		attack_range,
		Thing    **stance_target,
		GameCoord *stance_position,
		SLONG     *stance_angle)
{
	SLONG  i;
	SLONG  dx;
	SLONG  dy;
	SLONG  dz;
	SLONG  angle;
	SLONG  dist;
	SLONG  dangle;
	SLONG  wangle;
	SLONG  px;
	SLONG  py;
	SLONG  pz;
	SLONG  score;
	SLONG  min_dist;
	SLONG  max_dist;

	Thing *p_target;

	SLONG  best_score;
	SLONG  best_angle;
	Thing *best_target;
	SLONG  best_dist;
	SLONG  best_dx;
	SLONG  best_dz;
	SLONG  ox,oy,oz;

	SLONG       found_upto;

	//
	// Find all nearby people.
	//

	found_upto = THING_find_sphere(
					p_person->WorldPos.X >> 8,
					p_person->WorldPos.Y >> 8,
					p_person->WorldPos.Z >> 8,
					STANCE_RADIUS,
				    found,
					STANCE_MAX_FIND,
					THING_FIND_PEOPLE);

	//
	// Initialise stance.
	//

	best_target   =  NULL;
	best_dist     =  0;
	best_dx       =  0;
	best_dz       =  0;
	best_angle    =  p_person->Draw.Tweened->Angle;
	best_score    = -INFINITY;
	
	//
	// The distance people are away who we consider.
	//

	#define STANCE_DIST_LEEWAY	0x60

	min_dist = attack_distance - attack_range; //STANCE_DIST_LEEWAY;
	max_dist = attack_distance + attack_range; //STANCE_DIST_LEEWAY;

	//
	// The angle that dangle is calculated relative to.
	//

	wangle = p_person->Draw.Tweened->Angle;

	switch(attack_direction)
	{
		case FIND_DIR_FRONT:	  wangle += 0;    break;
		case FIND_DIR_BACK:       wangle += 1024; break;
		case FIND_DIR_LEFT:		  wangle += 512;  break;
		case FIND_DIR_RIGHT:	  wangle -= 512;  break;
		case FIND_DIR_TURN_LEFT:  wangle += 256;  break;
		case FIND_DIR_TURN_RIGHT: wangle -= 256;  break;

		default:
			ASSERT(0);
			break;
	}

	wangle &= 2047;

	//
	// Look for the best person.
	//

	for (i = 0; i < found_upto; i++)
	{
		p_target = TO_THING(found[i]);

		if (p_target == p_person)
		{
			//
			// Ignore yourself!
			//

			continue;
		}

//		if ((p_target->State == STATE_DEAD || p_target->State == STATE_DYING) && !(p_target->Genus.Person->Flags&FLAG_PERSON_KO))
		if(is_person_dead(p_target))
		{
			//
			// Ignore dead people, but not knocked out people.
			//

			continue;
		}

		calc_sub_objects_position(p_target,p_target->Draw.Tweened->AnimTween,SUB_OBJECT_PELVIS,&ox,&oy,&oz);
		dx   = ox+(p_target->WorldPos.X - p_person->WorldPos.X >> 8);
		dy   = oy+(p_target->WorldPos.Y - p_person->WorldPos.Y >> 8);
		dz   = oz+(p_target->WorldPos.Z - p_person->WorldPos.Z >> 8);

		dist = QDIST2(abs(dx),abs(dz));

		if (!WITHIN(dist, min_dist, max_dist))
		{
			//
			// Distance out of range.
			//

			continue;
		}

		angle  = calc_angle(dx,dz) + 1024;
		angle &= 2047;
		dangle = angle_diff(angle, wangle);

		//
		// How important dangle is compared to distance.
		//

		#define STANCE_DANGLE_IMPORTANCE 4

		//
		// Score this dist and dangle.
		//

		switch(attack_direction)
		{
			case FIND_DIR_FRONT:
			case FIND_DIR_BACK:
			case FIND_DIR_LEFT:
			case FIND_DIR_RIGHT:

				if (abs(dangle) > 300)
				{
					//
					// Out of range dangle.
					//

					score = -INFINITY;
				}
				else
				{
					score  = 0;
					score -= dist;
					score -= abs(dangle) << STANCE_DANGLE_IMPORTANCE;
				}

				break;

			case FIND_DIR_TURN_LEFT:

				if (0 && !WITHIN(dangle, -512, 256))
				{
					//
					// Out of range dangle.
					//

					score = -INFINITY;
				}
				else
				{
					score  = 0;
					score -= dist;
					score -= abs(dangle) << STANCE_DANGLE_IMPORTANCE;
				}

				break;

			case FIND_DIR_TURN_RIGHT:

				if (0 && !WITHIN(dangle, -256, +512))
				{
					//
					// Out of range dangle.
					//

					score = -INFINITY;
				}
				else
				{
					score  = 0;
					score -= dist;
					score -= abs(dangle) << STANCE_DANGLE_IMPORTANCE;
				}

				break;

			default:
				ASSERT(0);
				break;
		}

		//
		// Is this a better person to hit?
		//

		if (score > best_score)
		{
			best_score  = score;
			best_target = p_target;
			best_angle  = angle;
			best_dx     = ox;
			best_dz     = oz;
			best_dist   = dist;
		}
	}

	if (best_target)
	{
		//
		// Turn to the correct dangle.
		//

		switch(attack_direction)
		{
			case FIND_DIR_FRONT:	  dangle =  0;    break;
			case FIND_DIR_BACK:       dangle =  1024; break;
			case FIND_DIR_LEFT:		  dangle = -512;  break;
			case FIND_DIR_RIGHT:	  dangle = +512;  break;
			case FIND_DIR_TURN_LEFT:  dangle =  0;    break;
			case FIND_DIR_TURN_RIGHT: dangle =  0;    break;

			default:
				ASSERT(0);
				break;
		}

		angle  = best_angle + dangle;
		angle &= 2047;

		//
		// Move the correct distance from the person.
		//

		dx = SIN(best_angle) * attack_distance >> 16;
		dz = COS(best_angle) * attack_distance >> 16;

		px = (best_target->WorldPos.X >> 8) + dx+best_dx;
		pz = (best_target->WorldPos.Z >> 8) + dz+best_dz;

		py = PAP_calc_height_at_thing(p_person,px,pz);

		ASSERT(WITHIN(px, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1));
		ASSERT(WITHIN(pz, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1));

	   *stance_target      = best_target;
	   *stance_angle       = angle;
	    stance_position->X = px << 8;
	    stance_position->Y = py << 8;
	    stance_position->Z = pz << 8;
		return(1);

	}
	else
	{
	   *stance_target   = NULL;
	   *stance_angle    = p_person->Draw.Tweened->Angle;
	   *stance_position = p_person->WorldPos;
		return(0);
	}
}

//
// returns true if its hitting a human
//
SLONG turn_to_target(
		Thing *p_person,
		SLONG  find_dir)
{
	Thing    *stance_target;
	GameCoord stance_position;
	SLONG     stance_angle;
	SLONG	ret;
	
	ret=find_attack_stance(
		p_person,
		find_dir&FIND_DIR_MASK,
		0x80,			// Hard-code the fight distance.
		0x60,           // hard code the fight range
	   &stance_target,
	   &stance_position,
	   &stance_angle);

	//
	// Move to the new position.
	//

// actually no	move_thing_on_map(p_person, &stance_position);

	//
	// Turn to face the target.
	//

	if(ret)
	{
/*
		if(find_dir&FIND_DIR_DONT_TURN)
		{
			switch(find_dir&FIND_DIR_MASK)
			{
				case	FIND_DIR_LEFT:
					stance_angle+=512;
					break;
				case	FIND_DIR_RIGHT:
					stance_angle-=512;
					break;
				case	FIND_DIR_BACK:
					stance_angle+=1024;
					break;

			}

		}
*/
	}
//	reset_gang_attack(p_person);
	p_person->Draw.Tweened->Angle = stance_angle&2047;
	if(stance_target)
	{
		if(p_person->Genus.Person->Target==0)
		{
			p_person->Genus.Person->Target=THING_NUMBER(stance_target);
			ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
			ASSERT(p_person->Genus.Person->Target !=THING_NUMBER(p_person));
		}
	}

	if(stance_target)
		return(THING_NUMBER(stance_target));
	return(-ret);
}

SLONG turn_to_direction_and_find_target(Thing *p_person,SLONG  find_dir)
{
	Thing    *stance_target;
	GameCoord stance_position;
	SLONG     stance_angle;
	SLONG	ret;
	
	ret=find_attack_stance(
		p_person,
		find_dir&FIND_DIR_MASK,
		0xf0,			// Hard-code the fight distance.
		0xd0,           // hard code the fight range
	   &stance_target,
	   &stance_position,
	   &stance_angle);



	//
	// Turn to face the target.
	//

	switch(find_dir&FIND_DIR_MASK)
	{
		case	FIND_DIR_LEFT:
			stance_angle+=512;
			break;
		case	FIND_DIR_RIGHT:
			stance_angle-=512;
			break;
		case	FIND_DIR_BACK:
			stance_angle+=1024;
			break;

	}

//	reset_gang_attack(p_person);
	p_person->Draw.Tweened->Angle = stance_angle&2047;
	if(stance_target)
	{
		p_person->Genus.Person->Target=THING_NUMBER(stance_target);
		ASSERT(TO_THING(p_person->Genus.Person->Target)->Class==CLASS_PERSON);
		ASSERT(p_person->Genus.Person->Target !=THING_NUMBER(p_person));

		return(THING_NUMBER(stance_target));
	}
	else
	{
		//
		// no target in this direction
		//
		p_person->Genus.Person->Target=0;
		return(0);

	}

}


SLONG turn_to_target_and_punch(Thing *p_person)
{
	SLONG	ret;
	SLONG	anim;
	//
	// Turn...
	//

	ret=turn_to_target(
		p_person,
		FIND_DIR_FRONT);

	//
	// ... and punch!
	//

	anim=set_person_punch(p_person);
	if(ret>0&& anim)
	{
		Thing	*p_target;
		p_target=TO_THING(ret);

		if(should_i_block(p_target,p_person,anim))
			p_target->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;

	}

	return(ret);
}

SLONG turn_to_target_and_kick(Thing *p_person)
{
	SLONG	ret;
	SLONG	anim;
	Thing	*p_target;
	//
	// Turn...
	//

	ret=turn_to_target(
		p_person,
		FIND_DIR_FRONT);

	//
	// ... and kick!
	//

	p_target=TO_THING(ret);
	if(ret&&is_person_ko_and_lay_down(p_target)) //(p_target->Genus.Person->Flags&FLAG_PERSON_KO))
	{
		set_person_stomp(p_person);
	}
	else
	{
		SLONG dist=0;
		if(ret &&  (dist=dist_to_target(p_person,TO_THING(ret)))<120)
			anim=set_person_kick_near(p_person,dist);
		else
			anim=set_person_kick(p_person);
		if(ret>0 && anim)
		{
			Thing	*p_target;
			p_target=TO_THING(ret);
			if(should_i_block(p_target,p_person,anim))
				p_target->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;

		}
	}

	return(ret);
}


Thing *is_person_under_attack_low_level(Thing *p_person,SLONG any_state,SLONG radius)
{
	SLONG  i;
//	UWORD  found[8];
	SLONG  num;
	Thing *p_found;

	SLONG  dx;
	SLONG  dz;
	SLONG  dist;

	SLONG  best_dist   = INFINITY;
	Thing *best_person = NULL;

	//
	// any_state is old!
	//

	ASSERT(any_state == FALSE);

	num = THING_find_sphere(
				p_person->WorldPos.X >> 8,
				p_person->WorldPos.Y >> 8,
				p_person->WorldPos.Z >> 8,
				radius,
				found,
				8,
				1 << CLASS_PERSON);

	for (i = 0; i < num; i++)
	{
		p_found = TO_THING(found[i]);

		if (p_found == p_person)
		{
			//
			// Ignore yourself.
			//
		}
		else
		{
//			if (p_found->State == STATE_DEAD ||
//				p_found->State == STATE_DYING&&!(p_found->Genus.Person->Flags&FLAG_PERSON_KO))
			if(is_person_dead(p_found))
			{
				//
				// Ignore dead or dying people, but not ko'ed
				//
			}
			else
			if (p_found->Genus.Person->Flags & FLAG_PERSON_ARRESTED)
			{
				//
				// Ignore arrested people.
				//
			}
			else
			if (abs(p_found->WorldPos.Y - p_person->WorldPos.Y) < 0x4000)
			{
				if (p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_KILLING   ||
					p_found->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NAVTOKILL)
				{
					//
					// Is he attacking me?
					//

					Thing *p_attacking = NULL;

					if (p_found->Genus.Person->pcom_ai_arg)
					{
						p_attacking = TO_THING(p_found->Genus.Person->pcom_ai_arg);
					}

					if (p_attacking == p_person || (p_attacking && p_attacking->Genus.Person->pcom_ai == PCOM_AI_BODYGUARD && p_attacking->Genus.Person->pcom_ai_other == THING_NUMBER(p_person)))
					{
						//
						// Yes! He is a candidate for being an answer.
						//

						dx = p_found->WorldPos.X - p_person->WorldPos.X >> 8;
						dz = p_found->WorldPos.Z - p_person->WorldPos.Z >> 8;

						dist = abs(dx) + abs(dz);

						if (dist < best_dist)
						{
							best_dist   = dist;
							best_person = p_found;
						}
					}
				}
			}
		}
	}

	return best_person;
}

Thing *is_person_under_attack(Thing *p_person)
{
	return(is_person_under_attack_low_level(p_person,0,0x800));
}

