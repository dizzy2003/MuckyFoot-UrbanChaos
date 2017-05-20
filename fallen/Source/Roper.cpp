// Roper.cpp
// Guy Simmons, 4th January 1998

#include	"Game.h"
#include	"Roper.h"
#include	"statedef.h"
#include	"animate.h"

//---------------------------------------------------------------
// All temp stuff, botched in for demo.

#include	"..\Editor\Headers\Thing.h"

ULONG	move_thing(SLONG m_dx,SLONG m_dy,SLONG m_dz,struct MapThing *p_thing);

SLONG	calc_height_at(SLONG x,SLONG z);

extern Thing			*darci_thing;

//---------------------------------------------------------------

StateFunction	roper_states[]	=
{
	{	STATE_INIT,				fn_roper_init	},
	{	STATE_NORMAL,			fn_roper_normal	},
	{	STATE_HIT,				NULL			},
	{	STATE_ABOUT_TO_REMOVE,	NULL			},
	{	STATE_REMOVE_ME,		NULL			}
};

//---------------------------------------------------------------

void	fn_roper_init(Thing *t_thing)
{
//	t_thing->WorldPos.X	=	16896;
//	t_thing->WorldPos.Y	=	0;
//	t_thing->WorldPos.Z	=	16896;

	t_thing->DrawType						=	DT_ROT_MULTI;
//	t_thing->Draw.Tweened->Angle			=	0;
	t_thing->Draw.Tweened->Roll				=	0;
	t_thing->Draw.Tweened->Tilt				=	0;
	t_thing->Draw.Tweened->AnimTween		=	0;
	t_thing->Draw.Tweened->TweenStage		=	0;
	t_thing->Draw.Tweened->QueuedFrame		=	NULL;
//	t_thing->Draw.Tweened->AnimElements		=	t_thing->Draw.Tweened->CurrentFrame->FirstElement;
//	t_thing->Draw.Tweened->NextAnimElements	=	t_thing->Draw.Tweened->AnimElements;

//	t_thing->StateFn				=	(void(*)(Thing*))process_enemy2;

	t_thing->Genus.Person->Health		=	health[t_thing->Genus.Person->PersonType];

	add_thing_to_map(t_thing);
	set_person_idle(t_thing);
}

//---------------------------------------------------------------

void	fn_roper_normal(Thing *t_thing)
{

	return;
}

//---------------------------------------------------------------
