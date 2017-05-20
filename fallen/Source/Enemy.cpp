// Enemy.cpp
// Guy Simmons, 6th December 1997.


#include	"Game.h"
#include	"person.h"
#include	"animate.h"
#include	"..\Editor\Headers\Thing.h"

//extern KeyFrame			*darci_array[100],*roper_array[100];

void	process_enemy(Thing *e_thing);
void	process_enemy2(Thing *e_thing);
SLONG	calc_height_at(SLONG x,SLONG z);

extern Thing			*darci_thing;

//---------------------------------------------------------------

void	init_enemy(Thing *e_thing)
{
	e_thing->DrawType			=	DT_ROT_MULTI;

	e_thing->Draw.Tweened->Angle				=	0;
	e_thing->Draw.Tweened->Roll				=	0;
	e_thing->Draw.Tweened->Tilt				=	0;
	e_thing->Draw.Tweened->AnimTween			=	0;
	e_thing->Draw.Tweened->TweenStage		=	0;
	e_thing->Draw.Tweened->TheChunk			=	&game_chunk[0];

//	e_thing->StateFn				=	(void(*)(Thing*))process_enemy;

	set_anim(e_thing,ANIM_STAND_READY);
	add_thing_to_map(e_thing);
}

//---------------------------------------------------------------



