// Cop.cpp
// Guy Simmons, 12th January 1998.

#include	"Game.h"
//#include	"Command.h"
#include	"Cop.h"
#include	"statedef.h"
#include	"animate.h"



extern	SLONG	calc_height_at(SLONG x,SLONG z);
extern	SLONG	person_normal_animate(Thing *p_person);
extern	void	change_velocity_to(Thing *p_person,SWORD velocity);
extern	void	person_normal_move(Thing *p_person);
extern	void	fn_cop_fight(Thing *p_person);


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

#define	COP_IDLE		1

//---------------------------------------------------------------

StateFunction	cop_states[10]	=
{
	{	STATE_INIT,				fn_cop_init		},
	{	STATE_NORMAL,			fn_cop_normal	},
	{	STATE_HIT,				NULL			},
	{	STATE_ABOUT_TO_REMOVE,	NULL			},
	{	STATE_REMOVE_ME,		NULL			}
};

/*

StateFunction	cop_states[]	=
{
	{	STATE_INIT,				fn_cop_init				},
	{	STATE_NORMAL,			fn_cop_normal				},
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
	{	0,						NULL				}
};

*/
//---------------------------------------------------------------

void	fn_cop_init(Thing *t_thing)
{
	t_thing->DrawType						=	DT_ROT_MULTI;
	// Angle already set when the person was created.
	t_thing->Draw.Tweened->Roll				=	0;
	t_thing->Draw.Tweened->Tilt				=	0;
	t_thing->Draw.Tweened->AnimTween		=	0;
	t_thing->Draw.Tweened->TweenStage		=	0;
	t_thing->Draw.Tweened->CurrentFrame	    =	global_anim_array[t_thing->Genus.Person->AnimType][ANIM_STAND_READY];
	t_thing->Draw.Tweened->NextFrame		=	t_thing->Draw.Tweened->CurrentFrame->NextFrame;
	t_thing->Draw.Tweened->QueuedFrame		=	NULL;
/*
	switch(t_thing->Genus.Person->AnimType)
	{
		case	1: //darci
			t_thing->Draw.Tweened->TheChunk			=	&game_chunk[0];
			break;
		case	2: //roper
			t_thing->Draw.Tweened->TheChunk			=	&game_chunk[2];
			break;
		case	3: //cop
			t_thing->Draw.Tweened->TheChunk			=	&game_chunk[1];
			break;
		case	4: //thug
			t_thing->Draw.Tweened->TheChunk			=	&game_chunk[3];
			break;
		case	5: //hostage
			t_thing->Draw.Tweened->TheChunk			=	&game_chunk[0];
			break;
	}
*/
	// Initialise the cop to be walking.
//	t_thing->Genus.Person->State		=	0;
	t_thing->Genus.Person->Health		=	health[t_thing->Genus.Person->PersonType];

	set_thing_velocity(t_thing,10);

	// Set up the command index;
//	t_thing->Genus.Person->Command		=	t_thing->Genus.Person->CommandRef;

	set_person_idle(t_thing);
	add_thing_to_map(t_thing);
}

//---------------------------------------------------------------

//
void	fn_cop_normal(Thing *t_thing)
{
	#if 0

	SWORD			angle_diff;
	SLONG			distance,
					dx,dy,dz;
	DrawTween		*t_draw;
	GameCoord		dest_position,
					new_position;
	Person			*t_person;


	return;

	t_draw		=	t_thing->Draw.Tweened;
	t_person	=	t_thing->Genus.Person;

	if(t_person->Target)
	{
		fn_cop_fight(t_thing);
		return;
	}


	if(t_person->Command)
	{
		if(t_person->State==COP_IDLE)
		{
			if(person_normal_animate(t_thing))
			{
				t_person->State		=	0;
				set_thing_velocity(t_thing,15);
				t_draw->NextFrame	=	global_anim_array[t_person->AnimType][ANIM_WALK];
			}
		}
		else
		{
			person_normal_animate(t_thing);
//			MSG_add(" cop animtween %d \n",t_draw->AnimTween);

			dest_position.X	=	waypoints[t_person->Command].X;
			dest_position.Y	=	waypoints[t_person->Command].Y;
			dest_position.Z	=	waypoints[t_person->Command].Z;

			dx				=	dest_position.X-(t_thing->WorldPos.X>>8);
			dy				=	dest_position.Y-(t_thing->WorldPos.Y>>8);
			dz				=	dest_position.Z-(t_thing->WorldPos.Z>>8);

			distance		=	(dx*dx)+(dy*dy)+(dz*dz);

			if(distance<(64*64))
			{
				t_person->Command	=	waypoints[t_person->Command].Next;
				t_draw->AngleTo		=	Arctan(dx,-dz)-1024;
				if(Random()&0x0100)
				{
					t_person->State		=	COP_IDLE;
					t_person->Velocity	=	0;
					t_draw->NextFrame	=	global_anim_array[t_person->AnimType][ANIM_IDLE_SCRATCH1];
//					t_draw->NextFrame	=	cop_array[COP_ANIM_IDLE];
				}
			}
			else// if(t_draw->Angle!=t_draw->AngleTo)
			{
				// Recalibrate angle.
				t_draw->AngleTo		=	(Arctan(dx,-dz)-1024)&2047;

				// Interpolate the angle.
				angle_diff	=	(t_draw->AngleTo-t_draw->Angle)&2047;
				if(angle_diff>1024)
					angle_diff	-=	2048;
				t_draw->Angle	=	(t_draw->Angle+(angle_diff>>4))&2047;
			}

			// Move the bugger.
			
/*			dx = (SIN(t_draw->Angle)*t_thing->Velocity)>>16;
			dz = (COS(t_draw->Angle)*t_thing->Velocity)>>16;
			dy = t_thing->DY;

			dx = (dx*TICK_RATIO)>>TICK_SHIFT;
			dy = (dy*TICK_RATIO)>>TICK_SHIFT;
			dz = (dz*TICK_RATIO)>>TICK_SHIFT;
			
			new_position	=	t_thing->WorldPos;
			new_position.X	-=	dx; //(SIN(t_draw->Angle)*t_person->Velocity)>>16;
			new_position.Z	-=	dz; //(COS(t_draw->Angle)*t_person->Velocity)>>16;
			new_position.Y	=	dy; //calc_height_at(new_position.X,new_position.Z);
			move_thing_on_map(t_thing,&new_position);
*/
			person_normal_move(t_thing);
			if(t_thing->Genus.Person->InWay)
			{
				//
				// some fuckers in my way, so bash them in
				//
				t_thing->Genus.Person->Target=t_thing->Genus.Person->InWay; 
			ASSERT(t_thing->Genus.Person->Target !=THING_NUMBER(t_thing));

			}
		}
	}
	else
	{
		if(person_normal_animate(t_thing))
		{
			if(Random()&0x0080)
				t_thing->Draw.Tweened->NextFrame	=	global_anim_array[t_person->AnimType][ANIM_IDLE_SCRATCH2];
				//t_thing->Draw.Tweened->NextFrame	=	cop_array[4];
		}
	}

	#endif
}




void	fn_cop_fight(Thing *p_person)
{
/*
	Thing	*p_target;
	SLONG	dist;
	SLONG	end;

	p_target=TO_THING(p_person->Genus.Person->Target);

	if(p_target->State==STATE_DEAD||p_target->State==STATE_DYING)
	{
		p_person->Genus.Person->Target=0;
		p_person->Genus.Person->InWay=0;
		return;
	}

	switch(p_person->Draw.Tweened->CurrentAnim)
	{
		case	COP_ANIM_HIT:
			dist=turn_to_face_thing(p_person,p_target);
			if(dist>170)
			{
				cop_set_anim(p_person,COP_ANIM_WALK);
			}
			else
			{
				end=person_normal_animate(p_person);
				if(end)
				{
					cop_queue_anim(p_person,COP_ANIM_WALK);
				}
			}
			break;
		default:

			dist=turn_to_face_thing(p_person,p_target);

			if(dist>256*12)
			{
				cop_queue_anim(p_person,COP_ANIM_WALK);
				p_person->Genus.Person->Target=0;
			}
			else if(dist>256*2)
			{
				cop_queue_anim(p_person,COP_ANIM_WALK);
				change_velocity_to(p_person,32);
				person_normal_move(p_person);
			}
			else if(dist>160)
			{
				change_velocity_to(p_person,16);
				cop_queue_anim(p_person,COP_ANIM_WALK);
				person_normal_move(p_person);
			}
			else
			{
				if(dist>100)
				{
					cop_queue_anim(p_person,COP_ANIM_HIT);
				}
				else
					cop_queue_anim(p_person,COP_ANIM_STAND);
			}

			person_normal_animate(p_person);

			break;
	}
	*/

}


//---------------------------------------------------------------
