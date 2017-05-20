// Player.h
// Guy Simmons, 2nd January 1998.

#ifndef	PLAYER_H
#define	PLAYER_H

//---------------------------------------------------------------

#ifndef PSX
#define	MAX_PLAYERS			2
#else
#define MAX_PLAYERS			2
#endif

#define	PLAYER_NONE			0
#define	PLAYER_DARCI		1
#define	PLAYER_ROPER		2
#define	PLAYER_COP  		3
#define PLAYER_THUG			4

//---------------------------------------------------------------

typedef struct
{
	COMMON(PlayerType)

	ULONG			Input;
	ULONG			InputDone;
	UWORD			PlayerID;
	UBYTE			Stamina;
	UBYTE			Constitution;

	ULONG			LastInput;			// The input last gameturn
	ULONG			ThisInput;			// The input this gameturn
	ULONG			Pressed;			// The keys pressed  this gameturn
	ULONG			Released;			// The keys released this gameturn
	ULONG			DoneSomething;		// Flag so you know when you've pressed left or done a left-punch.
	SLONG			LastReleased[16];	// The GetTickCount() of when each key was last released
	UBYTE			DoubleClick[16];	// The double-click count for each key.

	UBYTE			Strength;
	UBYTE			RedMarks;
	UBYTE			TrafficViolations;
	UBYTE			Danger;				// How far from Danger is Darci? 0 => No danger, 1 = Max danger, 3 = min danger
	// temporarily marked out from the psx until it gets ported across
	UBYTE			PopupFade;			// Bringing the pop-up inventory in and out. Part of player cos of splitscreen mode
	SBYTE			ItemFocus;			// In the inventory, the item you're about to select when you let go
	UBYTE			ItemCount;			// Number of valid inventory items currently held
	UBYTE			Skill;

	struct Thing	*CameraThing,
					*PlayerPerson;
	
}Player;

typedef Player*		PlayerPtr;

//---------------------------------------------------------------

extern	GenusFunctions		player_functions[];

void	init_players(void);
Thing	*alloc_player(UBYTE type);
void	free_player(Thing *player_thing);
Thing	*create_player(UBYTE type,SLONG x,SLONG y,SLONG z,SLONG id);

//---------------------------------------------------------------

//
// Call when the player gains or looses a red mark.
//

void PLAYER_redmark(SLONG playerid, SLONG dredmarks);


#endif
