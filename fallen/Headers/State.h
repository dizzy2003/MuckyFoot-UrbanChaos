// State.h
// Guy Simmons, 4th January 1998.

#ifndef	STATE_H
#define	STATE_H



//---------------------------------------------------------------

typedef struct	
{
	UBYTE			State;
	void			(*StateFn)(Thing*);
}StateFunction;

//---------------------------------------------------------------

typedef struct 
{
	UBYTE			Genus;
	StateFunction	*StateFunctions;
}GenusFunctions;

//---------------------------------------------------------------

extern	void	set_state_function(Thing *t_thing,UBYTE state);
extern	void	set_generic_person_state_function(Thing *t_thing,UBYTE state);
extern	void	set_generic_person_just_function(Thing *t_thing,UBYTE state);

//---------------------------------------------------------------

#endif
