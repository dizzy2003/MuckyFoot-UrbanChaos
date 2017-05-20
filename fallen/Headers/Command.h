// Command.h
// Guy Simmons, 8th February 1998.

#ifndef	COMMAND_H
#define	COMMAND_H

//---------------------------------------------------------------

#ifdef	PSX
#define	MAX_WAYPOINTS		100
#else
#define	MAX_WAYPOINTS		1000
#endif

struct Waypoint
{
	BOOL		Used;
	UWORD		Next,
				Prev;
	SLONG		X,Y,Z;
};

extern	ULONG				waypoint_count;
extern	Waypoint			waypoints[MAX_WAYPOINTS];

//---------------------------------------------------------------

void	init_waypoints(void);
UWORD	alloc_waypoint(void);
void	free_waypoint(UWORD wp_index);

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
// Condition types.

#define	CON_NONE				0
#define	CON_THING_DEAD			1
#define	CON_ALL_GROUP_DEAD		2
#define	CON_PERCENT_GROUP_DEAD	3
#define	CON_THING_NEAR_PLAYER	4
#define	CON_GROUP_NEAR_PLAYER	5
#define	CON_CLASS_NEAR_PLAYER	6
#define	CON_THING_NEAR_THING	7
#define	CON_GROUP_NEAR_THING	8
#define	CON_CLASS_NEAR_THING	9
#define	CON_CLASS_COUNT			10
#define	CON_GROUP_COUNT			11
#define	CON_SWITCH_TRIGGERED	12
#define	CON_TIME				13
#define	CON_CLIST_FULFILLED		14


//---------------------------------------------------------------

#ifdef	PSX
#define	MAX_CONDITIONS	100
#else
#define	MAX_CONDITIONS	1000
#endif

#define	CONDITION_TRUE	(1<<0)

struct Condition
{
	BOOL		Used;
	UWORD		Flags,
				ConditionType,
				GroupRef;
	SLONG		Data1,
				Data2,
				Data3;
	Condition	*Next,
				*Prev;
};

extern ULONG			condition_count;
extern Condition		conditions[MAX_CONDITIONS];

//---------------------------------------------------------------


void		init_conditions(void);
Condition	*alloc_condition(void);

//---------------------------------------------------------------

#define	MAX_CLISTS		100

#define	LIST_TRUE		(1<<0)

struct	ConditionList
{
	BOOL		Used;
	SLONG		ConditionCount,
				Flags;
	Condition	*TheList,
				*TheListEnd;
};

extern ULONG			con_list_count;
extern ConditionList	con_lists[MAX_CLISTS];

#define	CONLIST_NUMBER(c)	(UWORD)(c-con_lists)
#define TO_CONLIST(c)		(&con_lists[c])

//---------------------------------------------------------------

void			init_clists(void);
ConditionList	*alloc_clist(void);
void			add_condition(ConditionList *the_list,Condition *the_condition);

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------
// Primary Command types.

#define	COM_NONE				0
#define	COM_ATTACK_PLAYER		1
#define	COM_ATTACK_THING		2
#define	COM_ATTACK_GROUP		3
#define	COM_ATTACK_CLASS		4
#define	COM_DEFEND_PLAYER		5
#define	COM_DEFEND_THING		6	
#define	COM_DEFEND_GROUP		7
#define	COM_DEFEND_CLASS		8
#define	COM_PATROL_WAYPOINT		9
#define	COM_START_TIMER			10
#define	COM_WAIT_FOR_TRIGGER	11
#define	COM_WAIT_FOR_CLIST		12
#define	COM_FOLLOW_PLAYER		13

// Secondary Command types.
#define	COM_S_NONE				0
#define	COM_S_UNTIL_TRIGGER		1
#define	COM_S_UNTIL_CLIST		2
#define	COM_S_WHILE_TRIGGER		3
#define	COM_S_WHILE_CLIST		4

//---------------------------------------------------------------
#ifdef	PSX
#define	MAX_COMMANDS	200
#else
#define	MAX_COMMANDS	2000
#endif

struct Command
{
	BOOL		Used;
	UWORD		Flags,
				CommandType,
				GroupRef;
	SLONG		Data1,
				Data2,
				Data3;
	Command		*Next,
				*Prev;
};

extern ULONG			command_count;
extern Command			commands[MAX_COMMANDS];

#define	COMMAND_NUMBER(x)	  (ULONG)(x-&commands[0])
#define TO_COMMAND(x)		   (&commands[x])

//---------------------------------------------------------------


void		init_commands(void);
Command		*alloc_command(void);

//---------------------------------------------------------------

#define	MAX_COMLISTS	200

struct	CommandList
{
	BOOL		Used;
	SLONG		CommandCount,
				Flags;
	Command		*TheList,
				*TheListEnd;
};

extern ULONG			com_list_count;
extern CommandList		com_lists[MAX_COMLISTS];

#define	COMLIST_NUMBER(c)	(UWORD)(c-com_lists)

//---------------------------------------------------------------

void			init_comlists(void);
CommandList		*alloc_comlist(void);
void			add_command(CommandList *the_list,Command *the_command);

//---------------------------------------------------------------

void	process_condition_lists(void);

//---------------------------------------------------------------

#endif
