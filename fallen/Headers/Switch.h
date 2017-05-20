// Switch.h
// Guy Simmons, 9th March 1998.

#ifndef	SWITCH_H
#define	SWITCH_H

//---------------------------------------------------------------

#define	MAX_SWITCHES	2 //10

#define	SWITCH_NONE		0
#define	SWITCH_PLAYER	1
#define	SWITCH_THING	2
#define	SWITCH_GROUP	3
#define	SWITCH_CLASS	4

//---------------------------------------------------------------

#define	SWITCH_FLAGS_TRIGGERED	(1<<0)
#define	SWITCH_FLAGS_RESET		(1<<1)

#define	SCAN_MODE_SPHERE		0
#define	SCAN_MODE_RECT			1

//---------------------------------------------------------------

typedef struct
{
	COMMON(SwitchType)

	UBYTE		ScanMode;
	UBYTE		padtoword;
	UWORD		Scanee;

	SLONG		Depth,
				Height,
				Radius,
				Width;
}Switch;

typedef Switch* SwitchPtr;

//---------------------------------------------------------------

void		init_switches(void);
Thing		*alloc_switch(UBYTE type);
void		free_switch(Thing *person_thing);
THING_INDEX	create_switch(void);

void	fn_switch_player(Thing *s_thing);
void	fn_switch_thing(Thing *s_thing);
void	fn_switch_group(Thing *s_thing);
void	fn_switch_class(Thing *s_thing);

//---------------------------------------------------------------

#endif
