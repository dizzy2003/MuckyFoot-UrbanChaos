//
// For establishing a common time with the server.
//

#ifndef _PING_
#define _PING_



//
// Sends pings to the server for a while until a common time has
// been established with the server.  
//
// After calling this function GAME_turn, GAME_process and GAME_tick are
// all set to the same values are the server (theoretically).
//
// Returns FALSE on failure.
//

SLONG PING_do(void);




#endif
