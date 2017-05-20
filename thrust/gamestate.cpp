//
// For storing and restoring gamestate
//

#include "always.h"
#include "game.h"
#include "gamestate.h"
#include "orb.h"
#include "ship.h"
#include "tb.h"





void GAMESTATE_store(GAMESTATE_State *gs)
{
	//
	// Copy over data.
	//

	memcpy(gs->ship_ship, SHIP_ship, sizeof(SHIP_ship));
	memcpy(gs->orb_orb,   ORB_orb,   sizeof(ORB_orb  ));
	memcpy(gs->tb_tb,     TB_tb,     sizeof(TB_tb    ));
}


void GAMESTATE_restore(GAMESTATE_State *gs)
{
	//
	// copy over data.
	//

	memcpy(SHIP_ship, gs->ship_ship, sizeof(SHIP_ship));
	memcpy(ORB_orb,   gs->orb_orb,   sizeof(ORB_orb  ));
	memcpy(TB_tb,     gs->tb_tb,     sizeof(TB_tb    ));
}
