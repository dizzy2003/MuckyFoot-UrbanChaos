//
// For doing network game stuff...
//

#ifndef CNET_H
#define CNET_H


//
// The configuring loop.  
//

extern UBYTE CNET_network_game;
extern UBYTE CNET_i_am_host;
extern UBYTE CNET_num_players;
extern UBYTE CNET_player_id;

SLONG CNET_configure(void);





#endif
