//
// Menus from within the game.
//

#ifndef _GAMEMENU_
#define _GAMEMENU_





//
// Initialises all the gamemenu stuff. Call at the start of the
// game loop.
//

void GAMEMENU_init(void);


//
// Processes the game menu keypresses.  Returns what action should
// be taken.
//

#define GAMEMENU_DO_NOTHING            0
#define GAMEMENU_DO_RESTART            1
#define GAMEMENU_DO_CHOOSE_NEW_MISSION 2
#define GAMEMENU_DO_NEXT_LEVEL         3

SLONG GAMEMENU_process(void);


//
// Returns TRUE if the game is paused.
//

SLONG GAMEMENU_is_paused(void);


//
// Returns a multiplier to be used to slow down the game. It returns
// an 8-bit fixed point number. 0 means that the game should not be
// processed at all.
//

SLONG GAMEMENU_slowdown_mul(void);



//
// When you lose the level, this is the message that will be displayed.
//

void GAMEMENU_set_level_lost_reason(CBYTE *reason);


//
// Draws all the game menu stuff.
//

void GAMEMENU_draw(void);




#endif
