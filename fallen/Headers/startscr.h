#ifndef	START_SCR_H
#define	START_SCR_H	1


#define	STARTS_START	1
#define	STARTS_EDITOR	2
#define	STARTS_LOAD		3	
#define	STARTS_EXIT		4	
#define	STARTS_HOST		5	
#define	STARTS_JOIN		6
#define	STARTS_PLAYBACK	7
#define	STARTS_PSX		8
#define	STARTS_MULTI	9
#define STARTS_LANGUAGE_CHANGE 10

struct	StartMenu
{
	UBYTE	StartIndex;
	UBYTE	Count;
	UBYTE	Current;
	UWORD	Type;
};

struct	StartMenuItemSimple
{
	CBYTE	*Str;
	SLONG	NextMenu;
	SLONG	Action;
	SLONG	Dummy1;
	SLONG	Dummy2;
};

struct	StartMenuItemComplex
{
	CBYTE	*Str;
	CBYTE	*Strb[3];
	SLONG	NextMenu;
	SLONG	Action;
	SLONG	Item;
	SLONG	Dummy2;
};

void STARTSCR_notify_gameover(BOOL won);

typedef void (*MISSION_callback)(CBYTE *filename);
void	MissionListCallback(CBYTE *script, MISSION_callback cb);



#endif