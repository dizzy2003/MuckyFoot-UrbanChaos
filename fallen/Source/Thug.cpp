// Thug.cpp
// Guy Simmons, 8th February 1998.

#include	"Game.h"
//#include	"Command.h"
#include	"Thug.h"
#include	"statedef.h"
#include	"animate.h"
#include	"pap.h"

SLONG	calc_height_at(SLONG x,SLONG z);
SLONG	person_normal_animate(Thing *p_person);


#define	THUG_IDLE		1

#undef THUG_ANIM_WALK
#define	THUG_ANIM_WALK	1
#define	THUG_ANIM_IDLE	0


//---------------------------------------------------------------

StateFunction	thug_states[]	=
{
	{	STATE_INIT,				fn_thug_init	},
	{	STATE_NORMAL,			fn_thug_normal	},
	{	STATE_HIT,				NULL			},
	{	STATE_ABOUT_TO_REMOVE,	NULL			},
	{	STATE_REMOVE_ME,		NULL			}
};

//---------------------------------------------------------------

void	fn_thug_init(Thing *t_thing)
{
	ASSERT(0);
	t_thing->DrawType						=	DT_ROT_MULTI;
	t_thing->Draw.Tweened->Angle			=	0;
	t_thing->Draw.Tweened->Roll				=	0;
	t_thing->Draw.Tweened->Tilt				=	0;
	t_thing->Draw.Tweened->AnimTween		=	0;
	t_thing->Draw.Tweened->TweenStage		=	0;
	t_thing->Draw.Tweened->QueuedFrame		=	NULL;
	t_thing->Draw.Tweened->TheChunk			=	&game_chunk[1];
//	t_thing->Genus.Person->Health		=	200;
	t_thing->Genus.Person->Health = health[t_thing->Genus.Person->PersonType];


	// Initialise the cop to be walking.
//	t_thing->Genus.Person->State		=	0;
	set_thing_velocity(t_thing,10);

	// Set up the command index;
//	t_thing->Genus.Person->Command		=	t_thing->Genus.Person->CommandRef;

	set_anim(t_thing,ANIM_STAND_READY);

	set_state_function(t_thing,STATE_NORMAL);
//	set_person_idle(t_thing);
	add_thing_to_map(t_thing);
}

//---------------------------------------------------------------

void	fn_thug_normal(Thing *t_thing)
{
	#if 0

	SWORD			angle_diff;
	SLONG			distance,
					dx,dy,dz;
	DrawTween		*t_draw;
	GameCoord		dest_position,
					new_position;
	Person			*t_person;


	t_draw		=	t_thing->Draw.Tweened;
	t_person	=	t_thing->Genus.Person;
	if(t_person->Command)
	{
		if(t_person->State==THUG_IDLE)
		{
			if(person_normal_animate(t_thing))
			{
				t_person->State		=	0;
				t_person->Velocity	=	10;
				queue_anim(t_thing,ANIM_WALK);
//				t_draw->NextFrame	=	cop_array[THUG_ANIM_WALK];
			}
		}
		else
		{
			person_normal_animate(t_thing);

			dest_position.X	=	waypoints[t_person->Command].X;
			dest_position.Y	=	waypoints[t_person->Command].Y;
			dest_position.Z	=	waypoints[t_person->Command].Z;

			dx				=	dest_position.X-t_thing->WorldPos.X;
			dy				=	dest_position.Y-t_thing->WorldPos.Y;
			dz				=	dest_position.Z-t_thing->WorldPos.Z;

//			distance		=	(dx*dx)+(dy*dy)+(dz*dz);
			distance		=	(dx*dx)+(dz*dz);

			if(distance<(64*64))
			{
				t_person->Command	=	waypoints[t_person->Command].Next;
				t_draw->AngleTo		=	Arctan(dx,-dz)-1024;
/*
				if(Random()&0x0100)
				{
					t_person->State		=	THUG_IDLE;
					t_person->Velocity	=	0;
					t_draw->NextFrame	=	cop_array[THUG_ANIM_IDLE];
				}
*/
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
			new_position	=	t_thing->WorldPos;
			new_position.X	-=	(SIN(t_draw->Angle)*t_person->Velocity)>>16;
			new_position.Z	-=	(COS(t_draw->Angle)*t_person->Velocity)>>16;
			new_position.Y	=	PAP_calc_height_at(new_position.X,new_position.Z);
			move_thing_on_map(t_thing,&new_position);
		}
	}
	else
	{
		if(person_normal_animate(t_thing))
		{
			/*
			if(Random()&0x0080)
				t_thing->Draw.Tweened->NextFrame	=	thug_array[THUG_ANIM_IDLE];
				*/
		}
	}

	#endif
}

//---------------------------------------------------------------
