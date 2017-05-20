//Cop.h
// Guy Simmons, 12th January 1998.

#ifndef	COP_H
#define	COP_H

#define	COP_ANIM_WALK			(1)
#define	COP_ANIM_STAND			(2)
#define	COP_ANIM_HIT			(3)
#define	COP_ANIM_IDLE			(4)
#define	COP_ANIM_KICK			(5)
#define	COP_ANIM_HEAD_HIT_SMALL	(6)
#define	COP_ANIM_GUT_HIT_SMALL	(7)
#define	COP_ANIM_GUT_DEATH		(8)
#define	COP_ANIM_DEATH			(9)
#define	COP_ANIM_JAB			(10)
#define	COP_ANIM_DIE_KNECK		(12)
#define	COP_ANIM_BLOCK			(23)
#define	COP_ANIM_JUMP_UP_GRAB	(17)
#define	COP_ANIM_PULL_UP		(18)
#define	COP_ANIM_JAB2			(25)
#define	COP_ANIM_IDLE			(4)
#define	COP_ANIM_IDLE2			(14)
#define	COP_ANIM_IDLE3			(15)
#define	COP_ANIM_BREATHE		(11)



//---------------------------------------------------------------

extern	StateFunction	cop_states[10];

void	fn_cop_init(Thing *t_thing);
void	fn_cop_normal(Thing *t_thing);

//---------------------------------------------------------------

#endif

