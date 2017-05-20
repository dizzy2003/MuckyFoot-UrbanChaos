//
// For storing and restoring gamestate
//

#ifndef _GAMESTATE_
#define _GAMESTATE_

#include "orb.h"
#include "ship.h"
#include "tb.h"


//
// The structure that holds our game state.
//

typedef struct gamestate_state
{
	SHIP_Ship  ship_ship[SHIP_MAX_SHIPS];
	ORB_Orb    orb_orb  [ORB_MAX_ORBS  ];
	TB_Tb      tb_tb    [TB_MAX_TBS    ];

} GAMESTATE_State;





//
// Returns a snapshot of the current game state.
//

void GAMESTATE_store(GAMESTATE_State *fill_me_in_please);



//
// Restores the given game state.
//

void GAMESTATE_restore(GAMESTATE_State *old_game_state);





#endif
